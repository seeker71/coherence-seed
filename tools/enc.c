/* enc.c - reference reader, printer and checker for spec/node-encoding.md (version 1).
 *
 *   cc -O2 -o enc tools/enc.c
 *   ./enc id '<print>'      read a canonical print; print a vector line: ok, print, bytes, id
 *   ./enc bytes <hex>       check one node's canonical bytes; print its id or the refusal
 *   ./enc vectors <file>    run a vector file; exit 1 when any line does not hold
 *
 * The page is the authority. A refusal is one of the words in its section 7.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

/* ---- nodes ------------------------------------------------------------------ */
#define MAXNODE 8192
#define MAXKID  1024
typedef struct {
  uint8_t id[32]; uint8_t *bytes; size_t len;
  int bp;             /* index of the blueprint node; -1 for the root */
  int *kid; int nkid; /* indices of children; a leaf has none */
  long long cat;      /* catalogue number, or -1 */
} Node;
static Node st[MAXNODE]; static int ns;
static const uint8_t NOTHING[32];
static uint8_t catid[32][32];                      /* ids of entries 0 to 31 */

static int utf8_ok(const uint8_t *s, size_t n) {
  for (size_t i = 0; i < n; ) {
    uint8_t c = s[i]; uint32_t cp; size_t k;
    if (c < 0x80) { i++; continue; }
    else if ((c & 0xe0) == 0xc0) { k = 1; cp = c & 0x1f; }
    else if ((c & 0xf0) == 0xe0) { k = 2; cp = c & 0x0f; }
    else if ((c & 0xf8) == 0xf0) { k = 3; cp = c & 0x07; }
    else return 0;
    if (i + k >= n) return 0;
    for (size_t j = 1; j <= k; j++) { if ((s[i+j] & 0xc0) != 0x80) return 0; cp = cp << 6 | (s[i+j] & 0x3f); }
    if ((k == 1 && cp < 0x80) || (k == 2 && cp < 0x800) || (k == 3 && cp < 0x10000)) return 0;
    if (cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff)) return 0;
    i += k + 1;
  }
  return 1;
}

/* Sections 2 to 5: the first rule a node's canonical bytes break, or NULL. */
static const char *refusal(const uint8_t *b, size_t len) {
  if (len < 33) return "short";
  if (b[0] > 1) return "kind";
  const uint8_t *bp = b + 1, *body = b + 33; size_t n = len - 33; int comp = b[0] == 1;
  if (!memcmp(bp, NOTHING, 32)) return (!comp && n == 0) ? NULL : "nothing-blueprint";
  if (comp) {
    if (n % 32) return "child-length";
    for (size_t i = 0; i < n; i += 32) if (!memcmp(body + i, NOTHING, 32)) return "nothing-child";
  }
  int c = -1;
  for (int i = 0; i < 32; i++) if (!memcmp(bp, catid[i], 32)) c = i;
  if (c < 0) return NULL;
  if (c == 15 || c >= 17) return "kept";
  if (c == 16) return comp ? NULL : "composite-only";
  if (comp) return "leaf-only";
  switch (c) {
  case 0: return (n >= 1 && n <= 4 && body[n-1] != 0) ? NULL : "ordinal";
  case 2: return utf8_ok(body, n) ? NULL : "utf8";
  case 3: return (n == 0 || body[n-1] != 0) ? NULL : "minimal";
  case 4:
    if (n == 0) return NULL;
    if (body[n-1] == 0x00 && (n == 1 || !(body[n-2] & 0x80))) return "minimal";
    if (body[n-1] == 0xff && n >= 2 && (body[n-2] & 0x80)) return "minimal";
    return NULL;
  case 5: case 9:  return n == 1 ? NULL : "width";
  case 6: case 10: return n == 2 ? NULL : "width";
  case 7: case 11: return n == 4 ? NULL : "width";
  case 8: case 12: return n == 8 ? NULL : "width";
  case 13: {
    if (n != 4) return "width";
    uint32_t u = (uint32_t)body[0] | (uint32_t)body[1] << 8 | (uint32_t)body[2] << 16 | (uint32_t)body[3] << 24;
    return ((u & 0x7f800000u) == 0x7f800000u && (u & 0x007fffffu) && u != 0x7fc00000u) ? "nan" : NULL; }
  case 14: {
    if (n != 8) return "width";
    uint64_t u = 0; for (int i = 7; i >= 0; i--) u = u << 8 | body[i];
    return ((u & 0x7ff0000000000000ull) == 0x7ff0000000000000ull && (u & 0x000fffffffffffffull)
            && u != 0x7ff8000000000000ull) ? "nan" : NULL; }
  }
  return NULL;                                            /* 1: bytes */
}

/* Checks the bytes, then returns the index of the one node with them. */
static int intern(uint8_t *b, size_t len, int bp, const int *kid, int nkid, const char **why) {
  if ((*why = refusal(b, len))) return -1;
  uint8_t id[32]; sha256(b, len, id);
  for (int i = 0; i < ns; i++) if (!memcmp(st[i].id, id, 32)) return i;
  if (ns == MAXNODE) { *why = "too-many-nodes"; return -1; }
  Node *x = &st[ns]; memcpy(x->id, id, 32);
  x->bytes = malloc(len); memcpy(x->bytes, b, len); x->len = len; x->bp = bp; x->nkid = nkid;
  x->kid = nkid ? malloc(sizeof(int) * (size_t)nkid) : NULL; if (nkid) memcpy(x->kid, kid, sizeof(int) * (size_t)nkid);
  x->cat = -1;
  if (bp < 0) x->cat = 0;
  else if (st[bp].cat == 0) { x->cat = 0; for (size_t i = len; i > 33; i--) x->cat = x->cat << 8 | b[i-1]; }
  return ns++;
}
static int entry(unsigned long n) {                       /* catalogue entry n */
  uint8_t b[37] = {0}; size_t len = 33; const char *why;
  if (n == 0) return intern(b, 33, -1, NULL, 0, &why);
  memcpy(b + 1, catid[0], 32);
  for (; n; n >>= 8) b[len++] = (uint8_t)n;
  return intern(b, len, 0, NULL, 0, &why);
}

/* ---- section 6: reading the print --------------------------------------------- */
static const char *p; static const char *err;
static int opened[MAXNODE], no;
static int fail(const char *w) { if (!err) err = w; return -1; }
static int number(unsigned long *out) {
  if (*p < '0' || *p > '9') return 0;
  if (*p == '0' && p[1] >= '0' && p[1] <= '9') return 0;
  unsigned long long v = 0;
  for (; *p >= '0' && *p <= '9'; p++) { v = v * 10 + (unsigned)(*p - '0'); if (v > 4294967295ull) return 0; }
  *out = (unsigned long)v; return 1;
}
static int hexval(char c) { return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1; }
static int read_node(void) {
  unsigned long n;
  if (*p == '#') { p++; if (!number(&n)) return fail("syntax"); return entry(n); }
  if (*p == '^') { p++; if (!number(&n)) return fail("syntax");
    if (n >= (unsigned long)no || opened[n] < 0) return fail("forward"); return opened[n]; }
  if (*p != '[' && *p != '(') return fail("syntax");
  int comp = *p++ == '(';
  if (no == MAXNODE) return fail("too-many-nodes");
  int me = no++; opened[me] = -1;
  int bp = read_node(); if (bp < 0) return -1;
  static uint8_t buf[33 + 32 * MAXKID]; uint8_t *b = malloc(sizeof buf); size_t len = 33;
  int kid[MAXKID], nkid = 0;
  b[0] = (uint8_t)comp; memcpy(b + 1, st[bp].id, 32);
  if (comp) {
    while (*p == ' ') { p++;
      if (nkid == MAXKID) { free(b); return fail("too-many-nodes"); }
      int k = read_node(); if (k < 0) { free(b); return -1; }
      kid[nkid++] = k; memcpy(b + len, st[k].id, 32); len += 32; }
    if (*p++ != ')') { free(b); return fail("syntax"); }
  } else {
    if (*p == ' ') { p++;
      if (hexval(*p) < 0) { free(b); return fail("syntax"); }
      while (hexval(*p) >= 0) {
        if (hexval(p[1]) < 0 || len == sizeof buf) { free(b); return fail("syntax"); }
        b[len++] = (uint8_t)(hexval(p[0]) << 4 | hexval(p[1])); p += 2; } }
    if (*p++ != ']') { free(b); return fail("syntax"); }
  }
  const char *why; int x = intern(b, len, bp, kid, nkid, &why); free(b);
  if (x < 0) return fail(why);
  if (st[x].cat >= 0) return fail("not-canonical");            /* rule 2 */
  for (int j = 0; j < no; j++) if (opened[j] == x) return fail("not-canonical");   /* rule 4 */
  opened[me] = x; return x;
}
static int read_print(const char *text, const char **why) {
  p = text; err = NULL; no = 0;
  int x = read_node();
  if (x >= 0 && *p) x = fail("syntax");
  *why = err; return x;
}

/* ---- section 6: printing -------------------------------------------------------- */
static void put(char **o, const char *s) { size_t l = strlen(s); memcpy(*o, s, l); *o += l; }
static void print_node(int x, int *seen, int *nseen, char **o) {
  char t[32];
  if (st[x].cat >= 0) { snprintf(t, sizeof t, "#%lld", st[x].cat); put(o, t); return; }
  for (int j = 0; j < *nseen; j++) if (seen[j] == x) { snprintf(t, sizeof t, "^%d", j); put(o, t); return; }
  seen[(*nseen)++] = x;
  int comp = st[x].bytes[0] == 1;
  put(o, comp ? "(" : "["); print_node(st[x].bp, seen, nseen, o);
  if (comp) for (int i = 0; i < st[x].nkid; i++) { put(o, " "); print_node(st[x].kid[i], seen, nseen, o); }
  else if (st[x].len > 33) { put(o, " ");
    for (size_t i = 33; i < st[x].len; i++) { snprintf(t, sizeof t, "%02x", st[x].bytes[i]); put(o, t); } }
  put(o, comp ? ")" : "]");
}
static void hex(const uint8_t *b, size_t n, char *o) { for (size_t i = 0; i < n; i++) sprintf(o + 2*i, "%02x", b[i]); o[2*n] = 0; }
static long unhex(const char *s, uint8_t *o, size_t max) {
  size_t n = 0;
  for (; s[0] && s[1]; s += 2) { if (hexval(s[0]) < 0 || hexval(s[1]) < 0 || n == max) return -1; o[n++] = (uint8_t)(hexval(s[0]) << 4 | hexval(s[1])); }
  return *s ? -1 : (long)n;
}

#define BIG (1 << 20)
int main(int argc, char **argv) {
  for (int i = 0; i < 32; i++) { int x = entry((unsigned long)i); memcpy(catid[i], st[x].id, 32); }
  static char out[BIG], h[BIG], line[BIG]; static uint8_t raw[BIG / 2]; static int seen[MAXNODE];
  const char *why; char idh[65];
  if (argc == 3 && !strcmp(argv[1], "id")) {
    int x = read_print(argv[2], &why); if (x < 0) { printf("refused\t%s\n", why); return 1; }
    hex(st[x].bytes, st[x].len, h); hex(st[x].id, 32, idh);
    printf("ok\t%s\t%s\t%s\n", argv[2], h, idh); return 0;
  }
  if (argc == 3 && !strcmp(argv[1], "bytes")) {
    long n = unhex(argv[2], raw, sizeof raw); if (n < 0) { printf("refused\tsyntax\n"); return 1; }
    if ((why = refusal(raw, (size_t)n))) { printf("refused\t%s\n", why); return 1; }
    uint8_t id[32]; sha256(raw, (size_t)n, id); hex(id, 32, idh); printf("%s\n", idh); return 0;
  }
  if (argc == 3 && !strcmp(argv[1], "vectors")) {
    FILE *f = fopen(argv[2], "r"); if (!f) { fprintf(stderr, "%s: cannot open\n", argv[2]); return 2; }
    int ok = 0, bad = 0, ln = 0;
    while (fgets(line, sizeof line, f)) {
      ln++; line[strcspn(line, "\n")] = 0;
      if (!line[0] || line[0] == '#') continue;
      char *a = line, *b = strchr(a, '\t'), *c = NULL, *d = NULL;
      if (b) { *b++ = 0; c = strchr(b, '\t'); if (c) { *c++ = 0; d = strchr(c, '\t'); if (d) *d++ = 0; } }
      const char *miss = NULL; char got[128] = "";
      if (!strcmp(a, "ok") && b && c && d) {
        int x = read_print(b, &why);
        if (x < 0) { miss = "refused"; snprintf(got, sizeof got, "%s", why); }
        else {
          hex(st[x].bytes, st[x].len, h); hex(st[x].id, 32, idh);
          char *o = out; int nseen = 0; print_node(x, seen, &nseen, &o); *o = 0;
          long n = unhex(c, raw, sizeof raw);
          if (strcmp(h, c)) miss = "bytes differ";
          else if (strcmp(idh, d)) { miss = "id differs"; snprintf(got, sizeof got, "%s", idh); }
          else if (strcmp(out, b)) miss = "prints back differently";
          else if (n < 0 || refusal(raw, (size_t)n)) miss = "its bytes are refused";
        }
      } else if (!strcmp(a, "bad-print") && b && c && !d) {
        int x = read_print(b, &why);
        if (x >= 0) miss = "accepted"; else if (strcmp(why, c)) { miss = "refused for another reason"; snprintf(got, sizeof got, "%s", why); }
      } else if (!strcmp(a, "bad-bytes") && b && c && !d) {
        long n = unhex(b, raw, sizeof raw); why = n < 0 ? "syntax" : refusal(raw, (size_t)n);
        if (!why) miss = "accepted"; else if (strcmp(why, c)) { miss = "refused for another reason"; snprintf(got, sizeof got, "%s", why); }
      } else miss = "not a vector line";
      if (miss) { bad++; printf("%s:%d: %s%s%s\n", argv[2], ln, miss, got[0] ? ": " : "", got); } else ok++;
    }
    fclose(f);
    printf("enc: %d vectors hold, %d do not\n", ok, bad);
    return bad ? 1 : 0;
  }
  fprintf(stderr, "usage: enc id '<print>' | enc bytes <hex> | enc vectors <file>\n");
  return 2;
}
