/* fb.c - the observer: a second process that reads a framebuffer (bootstrap/fb.h).
 *
 *   fb show NAME [H]          one frame of a running or kept region; with H, that node and its writer
 *   fb record NAME FILE [MS]  copy a frame every MS milliseconds (default 5) until the writer is done
 *   fb play FILE              one line per recorded frame
 *   fb play FILE N [H]        frame N in detail (-1 is the last); with H, that node and its writer
 *   fb drop NAME              remove a kept region
 *
 * It maps the region read-only and never writes to it, signals its writer or holds a lock.
 * A frame is a copy of the head page and of the used part of every area. A recording keeps
 * only the pages that differ from the frame before. The names it prints for catalogue
 * entries are for reading here; they are not part of any node.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "../bootstrap/fb.h"

#define STACK_TOP (1u << 18)              /* how much of the machine stack a frame keeps: its newest end */
static const char *const AREA_NAME[A_COUNT] = { FB_AREA_NAMES };
static const char *const NAMES[SEEDED + 1] = { [1] = "bytes", "text", "nat", "int", [16] = "list",
  [64] = "blueprint", "count", "child", "byte", "leaf", "pack", "join", "bind", "binding", "up", "base", "lazy",
  "read", "print", "id", "file", "clock", "step", "outcome", "div", "mod", "fault", "layer", "encode", "decode",
  [32] = "no", "yes", "quote", "arg", "recipe", "call", "self", "seq", "choice", "fail", "cut", "all", "need",
  "recover", "add", "sub", "mul", "eq", "lt", "native", "code", "framebuffer", "area", "machine", "node",
  "frame", "link", "activation", "choice-point", "gathered", "word-table", "byte-table" };
static const char *const MODE[] = { "eval", "ret", "failed", "fault", "done" };
static const char *const FKIND[] = { "?", "seq", "args", "cut", "fail", "all", "need", "recover", "handler", "outcome", "layer" };

static uint8_t *img; static uint64_t size;                /* a private copy of one frame, laid out as the region is */
static Head *hd; static Node *nd; static uint8_t *bytes; static H *kids, *argp;
static Frame *fr; static Act *act; static Choice *cp; static Bind *bd;
static void stop(const char *m, const char *n) { fprintf(stderr, "fb: %s%s\n", m, n); exit(2); }
static void *at(int i) { return img + hd->a[i].off; }
static void bind(void) {
  hd = (Head *)img; nd = at(A_NODE); bytes = at(A_BYTES); kids = at(A_KIDS); argp = at(A_ARG);
  fr = at(A_FRAME); act = at(A_ACT); cp = at(A_CHOICE); bd = at(A_BIND);
}
static double now_ms(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec / 1e6; }

/* ---- reading a region ---------------------------------------------------------------- */
static const uint8_t *attach(const char *name, double wait_ms) {   /* waits for a region whose writer lives; takes a finished one last */
  const uint8_t *src = MAP_FAILED; double t0 = now_ms();
  for (;;) {
    int fd = shm_open(name, O_RDONLY, 0); struct stat s;
    if (fd >= 0 && !fstat(fd, &s) && s.st_size >= (off_t)FB_PAGE) {
      size = (uint64_t)s.st_size; src = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    }
    if (fd >= 0) close(fd);
    if (src != MAP_FAILED) {
      const Head *h = (const Head *)src; int live = h->pid && (!kill((pid_t)h->pid, 0) || errno == EPERM);
      if (!memcmp(h->magic, FB_MAGIC, 8) && (live || now_ms() - t0 >= wait_ms)) return src;
      munmap((void *)src, size); src = MAP_FAILED;
    }
    if (now_ms() - t0 >= wait_ms) stop("no framebuffer named ", name);
    usleep(200);
  }
}
/* Copies the pages of one frame that differ from the image; lists their offsets in *pg. */
static uint32_t grab(const uint8_t *src, uint64_t **pg, uint64_t *step_after) {
  static uint8_t head[FB_PAGE]; static uint64_t *list; static uint32_t cap; uint32_t n = 0;
  memcpy(head, src, FB_PAGE);                             /* the counts are read once, first */
  const Head *h = (const Head *)head;
  for (int i = -1; i < A_COUNT; i++) {
    uint64_t off = 0, len = FB_PAGE;
    if (i >= 0) {
      const Area *a = &h->a[i]; uint64_t full = (uint64_t)a->cap * a->size;
      len = (uint64_t)a->n * a->size; off = a->off;
      if (i == A_STACK) { len = STACK_TOP; off = a->off + full - len; }
      if (len > full) len = full;
    }
    for (uint64_t p = off; p < off + len; p += FB_PAGE) {
      const uint8_t *from = i < 0 ? head : src + p;
      if (!memcmp(img + p, from, FB_PAGE)) continue;
      memcpy(img + p, from, FB_PAGE);
      if (n == cap) { cap = cap ? cap * 2 : 1024; if (!(list = realloc(list, cap * sizeof *list))) stop("out of memory", ""); }
      list[n++] = p;
    }
  }
  *step_after = ((const Head *)src)->m.step; *pg = list; return n;
}

/* ---- showing a frame ------------------------------------------------------------------- */
static char sb[4096]; static size_t sn, slim; static int cut;
static void sp(const char *s) {           /* appends to the line being shown; a line too long ends in three dots */
  for (; *s && !cut; s++) { if (sn + 4 >= slim) { memcpy(sb + sn, "...", 3); sn += 3; cut = 1; } else sb[sn++] = *s; }
  sb[sn] = 0;
}
static long long catn(H h) {
  if (h >= 1 && h <= SEEDED) return h - 1;
  if (!h || h >= hd->a[A_NODE].n || nd[h].comp || nd[h].bp != ROOT) return -1;
  long long v = 0; for (uint32_t i = nd[h].n; i > 0; i--) v = v << 8 | bytes[nd[h].off + i - 1];
  return v;
}
static void shown(H h, int depth) {
  char t[40];
  if (!h) { sp("nothing"); return; }
  if (h >= hd->a[A_NODE].n) { sp("?"); return; }
  long long c = catn(h); const Node *x = &nd[h]; const uint8_t *b = bytes + x->off;
  if (c >= 0) { if (c <= SEEDED && NAMES[c]) sp(NAMES[c]); else { snprintf(t, sizeof t, "#%lld", c); sp(t); } return; }
  if (x->comp) {
    if (depth <= 0) { sp("(..)"); return; }
    sp("("); shown(x->bp, depth - 1);
    for (uint32_t i = 0; i < x->n; i++) { sp(" "); shown(kids[x->off + i], depth - 1); }
    sp(")"); return;
  }
  if ((x->bp == E(INT) || x->bp == E(NAT)) && x->n <= 8) {
    uint64_t u = (x->bp == E(INT) && x->n && (b[x->n-1] & 0x80)) ? ~0ull : 0;
    for (uint32_t i = x->n; i > 0; i--) u = u << 8 | b[i-1];
    if (x->bp == E(INT)) snprintf(t, sizeof t, "%lld", (long long)u); else snprintf(t, sizeof t, "%llun", (unsigned long long)u);
    sp(t); return;
  }
  if (x->bp == E(2)) { sp("\""); for (uint32_t i = 0; i < x->n; i++) { t[0] = (char)b[i]; t[1] = 0; sp(t); } sp("\""); return; }
  sp("["); shown(x->bp, depth - 1); sp(" ");
  for (uint32_t i = 0; i < x->n; i++) { snprintf(t, sizeof t, "%02x", b[i]); sp(t); }
  sp("]");
}
static const char *str(H h, size_t lim) { sn = 0; cut = 0; slim = lim < sizeof sb ? lim : sizeof sb; sb[0] = 0; shown(h, 8); return sb; }
static void writer(H by) { if (by) printf("%s", str(by, 90)); else printf("the bootstrap's own reader"); }

static void line(int i, double ms, uint64_t after, uint32_t pages) {
  const Machine *m = &hd->m;
  printf("frame %4d  %9.2f ms  step %11llu%s  %-6s  nodes %-7u frames %-5u links %-5u activations %-8u choices %-3u pages %-5u at %s\n",
         i, ms, (unsigned long long)m->step, after != m->step ? " moving" : "       ", MODE[m->mode <= M_DONE ? m->mode : 0],
         hd->a[A_NODE].n, hd->a[A_FRAME].n, hd->a[A_LINK].n, hd->a[A_ACT].n, hd->a[A_CHOICE].n, pages, str(m->at, 70));
}
static void node(H h) {
  if (!h || h >= hd->a[A_NODE].n) { printf("node %u: not in this frame\n", h); return; }
  printf("node %u\n  is         %s\n", h, str(h, 2000));
  printf("  blueprint  %s\n", str(nd[h].bp, 200));
  printf("  %s, %u %s\n  written by ", nd[h].comp ? "composite" : "leaf", nd[h].n, nd[h].comp ? "children" : "bytes");
  writer(nd[h].by); printf("\n");
}
static void detail(void) {
  const Machine *m = &hd->m;
  printf("framebuffer version %u, writer pid %u, %llu bytes reserved\n", hd->version, hd->pid, (unsigned long long)hd->bytes);
  printf("machine  mode %s  step %llu  env %u  cut %u  k %u\n", MODE[m->mode <= M_DONE ? m->mode : 0], (unsigned long long)m->step, m->env, m->cut, m->k);
  printf("  at      %s\n", str(m->at, 160));
  printf("  x       %s\n", str(m->x, 160));
  printf("  v       %s\n", str(m->v, 160));
  printf("  reason  %s\n", str(m->reason, 160));
  if (m->mode == M_DONE && m->outcome) {
    printf("outcome  %s %s", m->outcome == 'n' ? "node" : m->outcome == '0' ? "nothing" : "fault", str(m->outcome == 'n' ? m->v : m->reason, 400));
    if (m->outcome == 'n') { printf("   (node %u, written by ", m->v); writer(nd[m->v].by); printf(")"); }
    printf("\n");
  }
  printf("areas\n");
  for (int i = 0; i < A_COUNT; i++) { const Area *a = &hd->a[i];
    printf("  %-20s %-13s %10u in use %12llu bytes\n", AREA_NAME[i], str(a->of, 40), a->n, (unsigned long long)a->n * a->size); }
  if (m->env && m->env < hd->a[A_ACT].n) {
    const Act *e = &act[m->env];
    printf("activation %u  recipe %s\n  arguments", m->env, str(e->recipe, 120));
    for (uint32_t i = 0; i < e->n && i < 8; i++) printf("  %s", str(argp[e->off + i], 60));
    printf("\n");
  }
  printf("continuation, newest first\n");
  uint32_t k = m->k;
  for (int i = 0; k && k < hd->a[A_FRAME].n && i < 12; i++, k = fr[k].k) {
    const Frame *f = &fr[k];
    printf("  frame %-6u %-8s child %-2u of %s\n", k, FKIND[f->t <= F_LAYER ? f->t : 0], f->i, str(f->node, 100));
    printf("  %14s written by ", ""); writer(f->by); printf("\n");
  }
  if (k) printf("  ... and older frames\n");
  for (uint32_t i = hd->a[A_CHOICE].n, j = 0; i > 0 && j < 4; i--, j++)
    printf("choice point %u  next option %u of %s\n", i - 1, cp[i-1].i, str(cp[i-1].node, 100));
  for (uint32_t i = hd->a[A_BIND].n, j = 0; i > 0 && j < 12; i--, j++) {      /* what runs, newest first */
    printf("binding %u  %s is run by ", i, str(bd[i-1].entry, 60)); printf("%s\n", str(bd[i-1].impl, 100)); }
}

/* ---- commands ---------------------------------------------------------------------------- */
static const char RMAGIC[8] = "CKREC1\n";
static void image(void) {
  img = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
  if (img == MAP_FAILED) stop("cannot hold a frame", ""); bind();
}
static int record(const char *name, const char *path, double every) {
  const uint8_t *src = attach(name, 5000); image();
  FILE *f = fopen(path, "wb"); if (!f) stop("cannot write ", path);
  fwrite(RMAGIC, 1, 8, f); fwrite(&size, 8, 1, f);
  double t0 = now_ms(); uint32_t frames = 0; uint64_t all = 0;
  for (int last = 0; !last; frames++) {
    const Head *live = (const Head *)src;
    last = live->m.mode == M_DONE || (kill((pid_t)live->pid, 0) && errno != EPERM);
    uint64_t *pg, after; uint32_t n = grab(src, &pg, &after); double ms = now_ms() - t0;
    fwrite(&ms, 8, 1, f); fwrite(&after, 8, 1, f); fwrite(&n, 4, 1, f);
    for (uint32_t i = 0; i < n; i++) { fwrite(&pg[i], 8, 1, f); fwrite(img + pg[i], 1, FB_PAGE, f); }
    all += n;
    if (!last) usleep((useconds_t)(every * 1000));
  }
  if (fclose(f)) stop("cannot write ", path);
  bind();
  printf("fb: %u frames, %llu pages, %.1f ms; the last frame is %s at step %llu\n", frames, (unsigned long long)all, now_ms() - t0,
         MODE[hd->m.mode <= M_DONE ? hd->m.mode : 0], (unsigned long long)hd->m.step);
  return 0;
}
static int play(const char *path, int want, int detailed, H h) {
  FILE *f = fopen(path, "rb"); char mg[8]; double ms = 0; uint64_t after = 0; uint32_t n = 0; int i = 0, any = 0;
  if (!f || fread(mg, 1, 8, f) != 8 || memcmp(mg, RMAGIC, 8) || fread(&size, 8, 1, f) != 1) stop("not a recording: ", path);
  image();
  for (;; i++) {
    double m2; uint64_t a2, off; uint32_t n2;
    if (fread(&m2, 8, 1, f) != 1 || fread(&a2, 8, 1, f) != 1 || fread(&n2, 4, 1, f) != 1) break;
    for (uint32_t j = 0; j < n2; j++)
      if (fread(&off, 8, 1, f) != 1 || off > size - FB_PAGE || fread(img + off, 1, FB_PAGE, f) != FB_PAGE) stop("the recording is cut short: ", path);
    ms = m2; after = a2; n = n2; any = 1; bind();
    if (!detailed) line(i, ms, after, n);
    else if (i == want) break;
  }
  if (!any || (detailed && want >= 0 && i != want)) stop("no such frame in ", path);
  if (detailed) { line(want < 0 ? i - 1 : i, ms, after, n); if (h) node(h); else detail(); }
  return 0;
}
int main(int argc, char **argv) {
  const char *c = argc > 1 ? argv[1] : "";
  if ((argc == 3 || argc == 4) && !strcmp(c, "show")) {
    const uint8_t *src = attach(argv[2], 0); uint64_t *pg, after; image();
    uint32_t n = grab(src, &pg, &after); bind(); line(0, 0, after, n);
    if (argc == 4) node((H)strtoul(argv[3], NULL, 10)); else detail();
    return 0;
  }
  if ((argc == 4 || argc == 5) && !strcmp(c, "record")) return record(argv[2], argv[3], argc == 5 ? atof(argv[4]) : 5);
  if (argc == 3 && !strcmp(c, "play")) return play(argv[2], 0, 0, 0);
  if ((argc == 4 || argc == 5) && !strcmp(c, "play")) return play(argv[2], atoi(argv[3]), 1, argc == 5 ? (H)strtoul(argv[4], NULL, 10) : 0);
  if (argc == 3 && !strcmp(c, "drop")) { if (shm_unlink(argv[2])) stop("no framebuffer named ", argv[2]); return 0; }
  fprintf(stderr, "usage: fb show NAME [H] | record NAME FILE [MS] | play FILE [N [H]] | drop NAME\n");
  return 2;
}
