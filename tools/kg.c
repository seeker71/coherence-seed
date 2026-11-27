/* kg.c - reads the claim graph under graph/, checks it, prints its derived index.
 *
 *   cc -O2 -o kg tools/kg.c
 *   ./kg check [graph-dir]     every rule in graph/8.kg; exit 1 when a rule is broken
 *   ./kg index [graph-dir]     one line per node: address, digest, kind, status, review, name
 *   ./kg brief [graph-dir]     address, name and statement, for reading the whole at low cost
 *   ./kg disputed [graph-dir]  each claim its latest review does not confirm, with that review
 *
 * A graph file holds blocks separated by blank lines. A block is one node: lines of
 * "key: value". A line starting with '#' is a comment. A node's address (n) is its
 * stable handle; its digest (first 8 hex of SHA-256 over its lines without the n line)
 * is its present content. A review names the digest it saw, so a changed node shows
 * its review as stale. Nothing here is stored: the index is derived on every run.
 */
#ifdef __linux__
#define _POSIX_C_SOURCE 200809L
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>

#define MAXN 16384
#define MAXF 32

typedef struct { char *k, *v; } Field;
typedef struct Node {
  const char *file; int line;
  Field f[MAXF]; int nf;
  const char *n, *kind;
  char hex[9];
  const struct Node *review;      /* latest review of this node, if any */
} Node;

static Node nodes[MAXN]; static int nn;
static Node *byaddr[MAXN];
static int errors, warnings;

/* ---- SHA-256 (FIPS 180-4) ------------------------------------------------ */
static const uint32_t K[64] = {
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
#define ROR(x,r) (((x) >> (r)) | ((x) << (32 - (r))))
static void sha_block(uint32_t h[8], const uint8_t *p) {
  uint32_t w[64], a[8];
  for (int i = 0; i < 16; i++)
    w[i] = (uint32_t)p[4*i] << 24 | (uint32_t)p[4*i+1] << 16 | (uint32_t)p[4*i+2] << 8 | p[4*i+3];
  for (int i = 16; i < 64; i++) {
    uint32_t s0 = ROR(w[i-15],7) ^ ROR(w[i-15],18) ^ (w[i-15] >> 3);
    uint32_t s1 = ROR(w[i-2],17) ^ ROR(w[i-2],19) ^ (w[i-2] >> 10);
    w[i] = w[i-16] + s0 + w[i-7] + s1;
  }
  memcpy(a, h, sizeof a);
  for (int i = 0; i < 64; i++) {
    uint32_t s1 = ROR(a[4],6) ^ ROR(a[4],11) ^ ROR(a[4],25);
    uint32_t ch = (a[4] & a[5]) ^ (~a[4] & a[6]);
    uint32_t t1 = a[7] + s1 + ch + K[i] + w[i];
    uint32_t s0 = ROR(a[0],2) ^ ROR(a[0],13) ^ ROR(a[0],22);
    uint32_t mj = (a[0] & a[1]) ^ (a[0] & a[2]) ^ (a[1] & a[2]);
    uint32_t t2 = s0 + mj;
    a[7] = a[6]; a[6] = a[5]; a[5] = a[4]; a[4] = a[3] + t1;
    a[3] = a[2]; a[2] = a[1]; a[1] = a[0]; a[0] = t1 + t2;
  }
  for (int i = 0; i < 8; i++) h[i] += a[i];
}
static void sha256(const uint8_t *msg, size_t len, uint8_t out[32]) {
  uint32_t h[8] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
  size_t i = 0; uint8_t tail[128]; size_t r;
  for (; i + 64 <= len; i += 64) sha_block(h, msg + i);
  r = len - i; memcpy(tail, msg + i, r); tail[r++] = 0x80;
  size_t padded = r <= 56 ? 64 : 128;
  memset(tail + r, 0, padded - r);
  uint64_t bits = (uint64_t)len * 8;
  for (int b = 0; b < 8; b++) tail[padded - 1 - b] = (uint8_t)(bits >> (8 * b));
  sha_block(h, tail); if (padded == 128) sha_block(h, tail + 64);
  for (int j = 0; j < 8; j++) { out[4*j] = h[j] >> 24; out[4*j+1] = h[j] >> 16; out[4*j+2] = h[j] >> 8; out[4*j+3] = h[j]; }
}

/* ---- reading --------------------------------------------------------------- */
static void err(const Node *x, const char *msg, const char *arg) {
  fprintf(stderr, "%s:%d: %s%s%s%s\n", x->file, x->line, x->n ? x->n : "?", ": ", msg, arg ? arg : ""); errors++;
}
static void warn(const Node *x, const char *msg, const char *arg) {
  fprintf(stderr, "%s:%d: %s%s%s%s (warning)\n", x->file, x->line, x->n ? x->n : "?", ": ", msg, arg ? arg : ""); warnings++;
}
static const char *get(const Node *x, const char *k) {
  for (int i = 0; i < x->nf; i++) if (!strcmp(x->f[i].k, k)) return x->f[i].v;
  return NULL;
}
static int is_addr(const char *s, size_t len) {
  if (!len) return 0;
  for (size_t i = 0; i < len; i++) {
    if (s[i] == '.') { if (i == 0 || i == len - 1 || s[i-1] == '.') return 0; }
    else if (s[i] < '0' || s[i] > '9') return 0;
  }
  return 1;
}
static int cmp_addr(const char *a, const char *b) {   /* component-wise numeric order */
  for (;;) {
    long x = strtol(a, (char **)&a, 10), y = strtol(b, (char **)&b, 10);
    if (x != y) return x < y ? -1 : 1;
    if (*a != '.' || *b != '.') return (*a == '.') - (*b == '.');
    a++; b++;
  }
}
static int cmp_node(const void *a, const void *b) { return cmp_addr((*(Node **)a)->n, (*(Node **)b)->n); }
static Node *find(const char *addr, size_t len) {
  char buf[64]; if (len >= sizeof buf) return NULL;
  memcpy(buf, addr, len); buf[len] = 0;
  int lo = 0, hi = nn - 1;
  while (lo <= hi) { int m = (lo + hi) / 2, c = cmp_addr(buf, byaddr[m]->n);
    if (!c) return byaddr[m]; if (c < 0) hi = m - 1; else lo = m + 1; }
  return NULL;
}
static void digest(Node *x) {
  static char buf[1 << 16]; size_t len = 0; uint8_t out[32];
  for (int i = 0; i < x->nf; i++) {
    if (!strcmp(x->f[i].k, "n")) continue;
    int w = snprintf(buf + len, sizeof buf - len, "%s: %s\n", x->f[i].k, x->f[i].v);
    if (w < 0 || (size_t)w >= sizeof buf - len) { err(x, "node too large to digest", NULL); break; }
    len += (size_t)w;
  }
  sha256((const uint8_t *)buf, len, out);
  snprintf(x->hex, sizeof x->hex, "%02x%02x%02x%02x", out[0], out[1], out[2], out[3]);
}
static void read_file(const char *dir, const char *name) {
  char path[1024]; snprintf(path, sizeof path, "%s/%s", dir, name);
  FILE *fp = fopen(path, "rb"); if (!fp) { fprintf(stderr, "%s: cannot open\n", path); errors++; return; }
  fseek(fp, 0, SEEK_END); long size = ftell(fp); fseek(fp, 0, SEEK_SET);
  char *text = malloc((size_t)size + 2); size_t got = fread(text, 1, (size_t)size, fp); fclose(fp);
  text[got] = '\n'; text[got + 1] = 0;
  char *file = strdup(path);
  if (size > 48 * 1024) { fprintf(stderr, "%s: %ld bytes, over the 48 KB budget: give a child hub its own file (warning)\n", path, size); warnings++; }
  Node *cur = NULL; int line = 0;
  for (char *p = text; *p; ) {
    char *e = strchr(p, '\n'); *e = 0; line++;
    size_t len = (size_t)(e - p); while (len && (p[len-1] == ' ' || p[len-1] == '\r' || p[len-1] == '\t')) p[--len] = 0;
    if (!len) cur = NULL;
    else if (*p != '#') {
      if (!cur) { if (nn == MAXN) { fprintf(stderr, "too many nodes\n"); exit(2); } cur = &nodes[nn++]; cur->file = file; cur->line = line; }
      char *c = strstr(p, ": ");
      size_t klen = c ? (size_t)(c - p) : 0; int okkey = klen > 0;
      for (size_t i = 0; i < klen; i++) if (!((p[i] >= 'a' && p[i] <= 'z') || p[i] == '-')) okkey = 0;
      if (!okkey) { fprintf(stderr, "%s:%d: not a 'key: value' line\n", file, line); errors++; }
      else if (cur->nf == MAXF) { fprintf(stderr, "%s:%d: too many lines in one node\n", file, line); errors++; }
      else { *c = 0; cur->f[cur->nf].k = p; cur->f[cur->nf].v = c + 2; cur->nf++; }
    }
    p = e + 1;
  }
}

/* ---- rules ------------------------------------------------------------------- */
static int one_of(const char *v, const char *list) {      /* list: "a|b|c" */
  size_t len = strlen(v);
  for (const char *p = list; *p; ) { const char *e = strchr(p, '|'); size_t l = e ? (size_t)(e - p) : strlen(p);
    if (l == len && !memcmp(p, v, len)) return 1; p += l; if (*p == '|') p++; }
  return 0;
}
static int is_date(const char *v) {
  if (!v || strlen(v) != 10) return 0;
  for (int i = 0; i < 10; i++) if ((i == 4 || i == 7) ? v[i] != '-' : (v[i] < '0' || v[i] > '9')) return 0;
  return 1;
}
/* every token of a reference line is an address, optionally followed by #locator */
static void refs(const Node *x, const char *key, const char *want_kind) {
  for (int i = 0; i < x->nf; i++) {
    if (strcmp(x->f[i].k, key)) continue;
    for (const char *p = x->f[i].v; *p; ) {
      while (*p == ' ') p++;
      const char *e = p; while (*e && *e != ' ') e++;
      if (e == p) break;
      const char *h = memchr(p, '#', (size_t)(e - p)); size_t alen = (size_t)((h ? h : e) - p);
      char tok[96]; snprintf(tok, sizeof tok, " '%.*s' in %s", (int)(e - p), p, key);
      Node *t = is_addr(p, alen) ? find(p, alen) : NULL;
      if (!t) err(x, "reference does not resolve:", tok);
      else if (want_kind && strcmp(t->kind, want_kind)) err(x, "reference is not a source node:", tok);
      p = e;
    }
  }
}
static void need(const Node *x, const char *key) {
  const char *v = get(x, key);
  if (!v || !*v) err(x, "missing required line: ", key);
}
/* ---- sources resolve ------------------------------------------------------------
 * A 'path' starts with the name of a repository. That repository is itself a source
 * node whose path is the bare name. This repository's files must exist; any other
 * repository carries the 'url' and 'rev' its files are read at. */
#include <unistd.h>
#define HOME_ROOT "coherence-seed"
static const char *gdir = "graph";
static Node *root_of(const char *path) {
  size_t l = strcspn(path, "/");
  for (int i = 0; i < nn; i++) {
    const char *p = get(byaddr[i], "path");
    if (p && !strcmp(byaddr[i]->kind, "source") && strlen(p) == l && !memcmp(p, path, l)) return byaddr[i];
  }
  return NULL;
}
static void check_source(const Node *x) {
  const char *path = get(x, "path"); if (!path) return;
  Node *r = root_of(path); const char *rel = path + strcspn(path, "/");
  if (!r) { err(x, "no source node stands for the repository of path ", path); return; }
  if (x == r && !get(x, "url")) err(x, "a repository needs the 'url' it is read from", NULL);
  if (!strcmp(get(r, "path"), HOME_ROOT)) {
    char f[1024]; snprintf(f, sizeof f, "%s/..%s", gdir, rel);
    if (*rel && access(f, F_OK)) err(x, "path names a file that is not in this repository: ", path);
  } else if (x == r && !get(x, "rev")) err(x, "a repository other than this one needs the 'rev' it was read at", NULL);
}
static void link_of(const Node *s, const char *loc, size_t ll) {    /* where a source can be read */
  const char *path = get(s, "path"), *url = get(s, "url");
  if (!path) printf("%s\t%s", s->n, url ? url : "-");
  else {
    Node *r = root_of(path); const char *rel = path + strcspn(path, "/"); if (*rel) rel++;
    const char *ru = r ? get(r, "url") : NULL, *rev = get(s, "rev"); if (!rev && r) rev = get(r, "rev");
    if (r && !strcmp(get(r, "path"), HOME_ROOT)) printf("%s\t%s", s->n, *rel ? rel : ".");
    else if (ru && rev && !strcmp(ru, "unpublished")) printf("%s\t%s at %s (unpublished: read from a checkout on the first machine)", s->n, path, rev);
    else if (ru && rev) printf("%s\t%s/%s/%s/%s", s->n, ru, *rel ? "blob" : "tree", rev, rel);
    else printf("%s\t%s", s->n, path);
  }
  if (ll) printf("\t%.*s", (int)ll, loc);
  printf("\n");
}
static void check_node(Node *x) {
  static const char *KINDS = "hub|axiom|theorem|term|source|observation|finding|decision|question|assumption|limit|plan|review";
  const char *st = get(x, "status"), *is = get(x, "is");
  if (!one_of(x->kind, KINDS)) { err(x, "unknown kind: ", x->kind); return; }
  if (!is_date(get(x, "at"))) err(x, "needs 'at: YYYY-MM-DD'", NULL);
  if (!is && strcmp(x->kind, "review")) err(x, "needs an 'is' statement", NULL);
  if (is && strlen(is) > 600 && strcmp(x->kind, "axiom")) warn(x, "statement is over 600 bytes: split it", NULL);
  if (strcmp(x->kind, "review") && !get(x, "name")) err(x, "needs a 'name'", NULL);
  for (int i = 0; i < x->nf; i++)
    if (!one_of(x->f[i].k, "n|name|kind|is|status|conf|src|from|see|open|needs|at|by|why|alt|flip|limit|url|path|rev|date|got|re|saw|ack|ans|was"))
      err(x, "unknown key: ", x->f[i].k);
  refs(x, "from", NULL); refs(x, "see", NULL); refs(x, "open", NULL);
  refs(x, "ans", NULL); refs(x, "src", "source"); refs(x, "re", NULL);
  const char *conf = get(x, "conf");
  if (conf && !one_of(conf, "high|mid|low")) err(x, "conf is high, mid or low, not ", conf);

  if (!strcmp(x->kind, "hub")) { if (st) err(x, "a hub carries no status", NULL); return; }
  if (!strcmp(x->kind, "source")) {
    if (!get(x, "url") && !get(x, "path")) err(x, "a source needs 'url' or 'path'", NULL);
    if (!is_date(get(x, "got"))) err(x, "a source needs 'got: YYYY-MM-DD' (the day it was read)", NULL);
    if (get(x, "date") && strlen(get(x, "date")) < 4) err(x, "source 'date' is the source's own date", NULL);
    check_source(x);
    return;
  }
  if (!strcmp(x->kind, "review")) {
    const char *saw = get(x, "saw"), *ack = get(x, "ack");
    need(x, "re");
    if (!saw || strlen(saw) != 8) err(x, "a review needs 'saw: <8 hex digest of what it read>'", NULL);
    if (!ack || !one_of(ack, "1|0|node")) err(x, "a review needs 'ack: 1', 'ack: 0' or 'ack: node'", NULL);
    if (ack && strcmp(ack, "1") && !is) err(x, "a review that does not confirm says why in 'is'", NULL);
    return;
  }
  if (!st) { err(x, "needs a status", NULL); return; }
  if (!strcmp(x->kind, "axiom") || !strcmp(x->kind, "theorem")) {
    if (strcmp(st, "given")) err(x, "an axiom or theorem has status given, not ", st);
    need(x, "src");
  } else if (!strcmp(x->kind, "observation")) {
    if (!one_of(st, "observed|relayed")) err(x, "an observation is observed or relayed, not ", st);
    need(x, "src");
  } else if (!strcmp(x->kind, "question")) {
    if (!one_of(st, "open|answered")) err(x, "a question is open or answered, not ", st);
    if (!strcmp(st, "answered")) need(x, "ans");
  } else if (!strcmp(x->kind, "assumption")) {
    if (strcmp(st, "assumed")) err(x, "an assumption has status assumed, not ", st);
  } else {                                              /* term finding decision limit plan; given is guidance held together */
    if (!one_of(st, "given|observed|relayed|inferred|assumed")) err(x, "status is given, observed, relayed, inferred or assumed, not ", st);
    else if (!strcmp(st, "inferred")) need(x, "from");
    else if (!strcmp(st, "observed") || !strcmp(st, "relayed")) need(x, "src");
  }
}
static void check_place(const Node *x) {
  const char *base = strrchr(x->file, '/'); base = base ? base + 1 : x->file;
  size_t hl = strlen(base) - 3;                         /* strip ".kg" */
  size_t nl = strlen(x->n);
  int in0 = hl == 1 && base[0] == '0';
  if (!in0 && !(nl >= hl && !memcmp(x->n, base, hl) && (nl == hl || x->n[hl] == '.')))
    err(x, "lives in a file that is not one of its ancestors: ", base);
  const char *dot = strrchr(x->n, '.');
  if (dot) { if (!find(x->n, (size_t)(dot - x->n))) err(x, "parent node is missing", NULL); }
  else if (strcmp(x->n, "0") && !find("0", 1)) err(x, "root node 0 is missing", NULL);
}

/* ---- release ------------------------------------------------------------------
 * The graph holds only what is needed. The ground, the design, what is open, the path
 * and the graph's own rules (hubs 0, 1, 5, 6, 7, 8) are living. A source, a piece of
 * evidence or a review stays only while something living refers to it, directly or
 * through what it refers to; a review also has to confirm its claim's present wording.
 * Everything else is taken out of its file, and a file left without nodes is removed. */
#define LIVING "015678"
static char gone[MAXN], keep[MAXN];
static void mark(Node *x) {
  if (keep[x - nodes]) return;
  keep[x - nodes] = 1;
  for (const char *d = x->n; (d = strchr(d, '.')); d++) { Node *p = find(x->n, (size_t)(d - x->n)); if (p) mark(p); }
  const char *path = !strcmp(x->kind, "source") ? get(x, "path") : NULL;
  if (path) { Node *r = root_of(path); if (r) mark(r); }
  for (int i = 0; i < x->nf; i++) {
    if (!one_of(x->f[i].k, "from|see|open|ans|src")) continue;
    for (const char *p = x->f[i].v; *p; ) {
      while (*p == ' ') p++;
      const char *e = p; while (*e && *e != ' ') e++;
      const char *h = memchr(p, '#', (size_t)(e - p)); Node *t = e > p ? find(p, (size_t)((h ? h : e) - p)) : NULL;
      if (t) mark(t);
      p = e;
    }
  }
}
static int release_file(const char *path) {
  FILE *fp = fopen(path, "rb"); if (!fp) return 0;
  fseek(fp, 0, SEEK_END); long size = ftell(fp); fseek(fp, 0, SEEK_SET);
  char *text = malloc((size_t)size + 2), *out = malloc((size_t)size + 2);
  size_t got = fread(text, 1, (size_t)size, fp), on = 0; fclose(fp); text[got] = 0;
  char *bs = text; int skip = 0, dropped = 0;
  for (char *l = text; ; ) {                             /* a block is its lines and the blank line that closes it */
    char *nl = strchr(l, '\n'); size_t len = nl ? (size_t)(nl - l) : strlen(l);
    if (!len) {
      char *be = nl ? nl + 1 : l;
      if (skip) dropped++; else { memcpy(out + on, bs, (size_t)(be - bs)); on += (size_t)(be - bs); }
      bs = be; skip = 0; if (!nl) break; l = be; continue;
    }
    if (len > 3 && !memcmp(l, "n: ", 3)) { Node *x = find(l + 3, len - 3); if (x && gone[x - nodes]) skip = 1; }
    l = nl ? nl + 1 : l + len;
  }
  out[on] = 0;
  if (dropped && !strstr(out, "\nn: ") && (on < 3 || memcmp(out, "n: ", 3))) remove(path);   /* no node is left in it */
  else if (dropped) { fp = fopen(path, "wb"); if (!fp || fwrite(out, 1, on, fp) != on || fclose(fp)) { fprintf(stderr, "%s: cannot write\n", path); exit(2); } }
  return dropped;
}

int main(int argc, char **argv) {
  const char *cmd = argc > 1 ? argv[1] : "check", *want = NULL;
  if (!strcmp(cmd, "link") && argc > 2) { want = argv[2]; argv++; argc--; }
  const char *dir = argc > 2 ? argv[2] : "graph"; gdir = dir;
  if (!one_of(cmd, "check|index|brief|disputed|link|release") || (!strcmp(cmd, "link") && !want)) {
    fprintf(stderr, "usage: kg check|index|brief|disputed [graph-dir]\n       kg link N [graph-dir]   where the sources of node N can be read\n"
                    "       kg release [graph-dir]  take out what nothing living refers to, and reviews that no longer confirm\n"); return 2; }
  DIR *d = opendir(dir); if (!d) { fprintf(stderr, "%s: no such directory\n", dir); return 2; }
  int files = 0; struct dirent *de;
  while ((de = readdir(d))) {
    size_t l = strlen(de->d_name);
    if (l < 4 || strcmp(de->d_name + l - 3, ".kg")) continue;
    if (!is_addr(de->d_name, l - 3)) { fprintf(stderr, "%s/%s: a graph file is named by a node address\n", dir, de->d_name); errors++; continue; }
    read_file(dir, de->d_name); files++;
  }
  closedir(d);
  int kept = 0;
  for (int i = 0; i < nn; i++) {
    Node *x = &nodes[i]; x->n = get(x, "n"); x->kind = get(x, "kind");
    if (!x->n || !is_addr(x->n, strlen(x->n))) { err(x, "needs 'n: <address>' such as 5.2.1", NULL); continue; }
    if (!x->kind) { err(x, "needs a kind", NULL); continue; }
    digest(x); byaddr[kept++] = x;
  }
  nn = kept; qsort(byaddr, (size_t)nn, sizeof *byaddr, cmp_node);
  for (int i = 1; i < nn; i++)
    if (!cmp_addr(byaddr[i-1]->n, byaddr[i]->n)) err(byaddr[i], "address is used twice; first in ", byaddr[i-1]->file);
  for (int i = 0; i < nn; i++) { check_node(byaddr[i]); check_place(byaddr[i]); }
  if (want) {                                            /* a source's own link, or the links of a claim's sources */
    Node *x = is_addr(want, strlen(want)) ? find(want, strlen(want)) : NULL;
    if (!x) { fprintf(stderr, "kg: no node %s\n", want); return 1; }
    if (!strcmp(x->kind, "source")) link_of(x, NULL, 0);
    for (const char *p = get(x, "src"); p && *p; ) {
      while (*p == ' ') p++;
      const char *e = p; while (*e && *e != ' ') e++;
      if (e == p) break;
      const char *h = memchr(p, '#', (size_t)(e - p)); Node *t = find(p, (size_t)((h ? h : e) - p));
      if (t) link_of(t, h ? h : e, h ? (size_t)(e - h) : 0);
      p = e;
    }
    return 0;
  }

  int fresh = 0, stale = 0, disputed = 0, reviewed = 0, claims = 0;
  for (int i = 0; i < nn; i++) {                         /* latest review per target */
    Node *r = byaddr[i]; if (strcmp(r->kind, "review")) continue;
    const char *re = get(r, "re"); Node *t = re ? find(re, strcspn(re, " #")) : NULL; if (!t) continue;
    const char *a = get(r, "at"), *b = t->review ? get(t->review, "at") : NULL;
    if (!t->review || (a && b && strcmp(a, b) >= 0)) t->review = r;
  }
  if (!strcmp(cmd, "release")) {
    int out = 0; const char *last = NULL;
    for (int i = 0; i < nn; i++) {                       /* the living hubs, and the top hub of every other */
      Node *x = byaddr[i]; size_t top = strcspn(x->n, ".");
      if (!x->n[top] || (top == 1 && strchr(LIVING, x->n[0]))) mark(x);
    }
    for (int i = 0; i < nn; i++) {                       /* a review stays while its claim does and it still confirms it */
      Node *r = byaddr[i]; if (strcmp(r->kind, "review")) continue;
      const char *re = get(r, "re"), *saw = get(r, "saw"), *ack = get(r, "ack");
      Node *t = re ? find(re, strcspn(re, " #")) : NULL;
      if (t && keep[t - nodes] && saw && !strcmp(saw, t->hex) && ack && !strcmp(ack, "1")) mark(r);
    }
    for (int i = 0; i < nn; i++) if (!keep[byaddr[i] - nodes]) gone[byaddr[i] - nodes] = 1;
    for (int i = 0; i < nn; i++) if (gone[i] && nodes[i].file != last) { last = nodes[i].file; out += release_file(last); }
    printf("kg: released %d nodes that nothing living refers to or that no longer confirm their claim\n", out);
    return 0;
  }
  for (int i = 0; i < nn; i++) {
    Node *x = byaddr[i]; if (!strcmp(x->kind, "review") || !strcmp(x->kind, "hub")) continue;
    claims++; if (!x->review) continue;
    const char *saw = get(x->review, "saw"), *ack = get(x->review, "ack");
    reviewed++;
    if (!saw || strcmp(saw, x->hex)) { stale++; if (!strcmp(cmd, "check")) warn(x, "changed since its review ", x->review->n); }
    else if (ack && !strcmp(ack, "1")) fresh++;
    else { disputed++;
      if (!strcmp(cmd, "disputed")) {                    /* the claim, then what its review saw */
        printf("== %s (%s)\n", x->n, x->file);
        for (int j = 0; j < x->nf; j++) if (strcmp(x->f[j].k, "n")) printf("%s: %s\n", x->f[j].k, x->f[j].v);
        printf("-- %s\n", x->review->n);
        for (int j = 0; j < x->review->nf; j++) if (one_of(x->review->f[j].k, "is|why|limit")) printf("%s: %s\n", x->review->f[j].k, x->review->f[j].v);
        printf("\n"); } }
  }
  if (!strcmp(cmd, "index") || !strcmp(cmd, "brief"))
    for (int i = 0; i < nn; i++) {
      Node *x = byaddr[i]; const char *st = get(x, "status"), *nm = get(x, "name"), *is = get(x, "is");
      const char *rv = "-"; if (x->review) { const char *saw = get(x->review, "saw"); rv = (!saw || strcmp(saw, x->hex)) ? "stale" : get(x->review, "ack"); }
      if (!strcmp(cmd, "index")) printf("%s\t%s\t%s\t%s\t%s\t%s\n", x->n, x->hex, x->kind, st ? st : "-", rv ? rv : "-", nm ? nm : "-");
      else if (strcmp(x->kind, "review")) printf("%s %s [%s%s%s] %s\n", x->n, nm ? nm : "-", x->kind, st ? " " : "", st ? st : "", is ? is : "");
    }
  fprintf(stderr, "kg: %d nodes in %d files; %d errors, %d warnings; of %d claims: %d confirmed, %d disputed, %d stale, %d unreviewed\n",
          nn, files, errors, warnings, claims, fresh, disputed, stale, claims - reviewed);
  return errors ? 1 : 0;
}
