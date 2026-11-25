/* second.c: a reader, printer and store of the node encoding, written from spec/node-encoding.md
 * by a hand that had not seen bootstrap/ck.c. It shares no line with the bootstrap, and it keeps
 * its nodes on the C heap: it is a witness beside the system, not a part of it.
 *
 *   second vectors FILE        hold every line of a vector file
 *   second read FILE           read one printed node; print its id and its print
 *   second freeze FILE OUT     read one printed node; write it as an ice file (graph node 5.14.8)
 *   second thaw FILE           read an ice file; print the top node's id and print
 *   second vary FILE N SEED    write N variations of FILE's vectors with this reader's verdict on each
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

static void die(const char *what, const char *arg) {
    fprintf(stderr, "second: %s %s\n", what, arg ? arg : "");
    exit(2);
}
static void *grow(void *p, size_t bytes) {
    p = realloc(p, bytes ? bytes : 1);
    if (!p) die("out of memory", NULL);
    return p;
}

/* ---- SHA-256, FIPS 180-4 ---- */
static const u32 K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

static u32 rotr(u32 x, int n) { return (x >> n) | (x << (32 - n)); }

static void sha_block(u32 h[8], const u8 *m) {
    u32 w[64], a[8];
    for (int t = 0; t < 16; t++)
        w[t] = (u32)m[4 * t] << 24 | (u32)m[4 * t + 1] << 16 | (u32)m[4 * t + 2] << 8 | m[4 * t + 3];
    for (int t = 16; t < 64; t++) {
        u32 s0 = rotr(w[t - 15], 7) ^ rotr(w[t - 15], 18) ^ (w[t - 15] >> 3);
        u32 s1 = rotr(w[t - 2], 17) ^ rotr(w[t - 2], 19) ^ (w[t - 2] >> 10);
        w[t] = w[t - 16] + s0 + w[t - 7] + s1;
    }
    memcpy(a, h, sizeof a);
    for (int t = 0; t < 64; t++) {
        u32 S1 = rotr(a[4], 6) ^ rotr(a[4], 11) ^ rotr(a[4], 25);
        u32 ch = (a[4] & a[5]) ^ (~a[4] & a[6]);
        u32 t1 = a[7] + S1 + ch + K[t] + w[t];
        u32 S0 = rotr(a[0], 2) ^ rotr(a[0], 13) ^ rotr(a[0], 22);
        u32 maj = (a[0] & a[1]) ^ (a[0] & a[2]) ^ (a[1] & a[2]);
        u32 t2 = S0 + maj;
        memmove(a + 1, a, 7 * sizeof a[0]);
        a[4] += t1;
        a[0] = t1 + t2;
    }
    for (int i = 0; i < 8; i++) h[i] += a[i];
}

static void sha256(const u8 *m, size_t n, u8 out[32]) {
    u32 h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    u8 last[128];
    size_t whole = n / 64, rest = n % 64, pad = rest < 56 ? 64 : 128;
    for (size_t i = 0; i < whole; i++) sha_block(h, m + 64 * i);
    memset(last, 0, sizeof last);
    memcpy(last, m + 64 * whole, rest);
    last[rest] = 0x80;
    for (int i = 0; i < 8; i++) last[pad - 1 - i] = (u8)(((u64)n * 8) >> (8 * i));
    sha_block(h, last);
    if (pad == 128) sha_block(h, last + 64);
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 4; j++) out[4 * i + j] = (u8)(h[i] >> (24 - 8 * j));
}

/* ---- sections 2 to 5: canonical bytes and what is refused ---- */
static u8 root_id[32];
static const u8 nothing[32];

static int is_nothing(const u8 *id) { return memcmp(id, nothing, 32) == 0; }

static size_t nat_bytes(u32 n, u8 out[4]) {
    size_t k = 0;
    for (; n; n >>= 8) out[k++] = (u8)n;
    return k;
}
static size_t entry_bytes(u32 n, u8 out[37]) { /* the canonical bytes of catalogue entry n */
    out[0] = 0;
    if (n == 0) { memset(out + 1, 0, 32); return 33; }
    memcpy(out + 1, root_id, 32);
    return 33 + nat_bytes(n, out + 33);
}

static int utf8_ok(const u8 *s, size_t n) {
    size_t i = 0;
    while (i < n) {
        u32 c = s[i], lo;
        size_t more;
        if (c < 0x80) { i++; continue; }
        if (c >= 0xc2 && c <= 0xdf) { more = 1; c &= 0x1f; lo = 0x80; }
        else if (c >= 0xe0 && c <= 0xef) { more = 2; c &= 0x0f; lo = 0x800; }
        else if (c >= 0xf0 && c <= 0xf4) { more = 3; c &= 0x07; lo = 0x10000; }
        else return 0;
        if (n - i <= more) return 0;
        for (size_t k = 1; k <= more; k++) {
            if ((s[i + k] & 0xc0) != 0x80) return 0;
            c = c << 6 | (s[i + k] & 0x3f);
        }
        if (c < lo || c > 0x10ffff || (c >= 0xd800 && c <= 0xdfff)) return 0;
        i += more + 1;
    }
    return 1;
}

/* ---- the nodes this run has met ---- */
typedef struct {
    u8 id[32];
    u8 *bytes;      /* canonical bytes */
    size_t len;
    int blueprint;  /* index, or -1 for the root's nothing */
    int *children;  /* indexes, for a composite */
    int entry;      /* 1 when this is a catalogue entry */
    u32 number;     /* which entry */
    long read_in;   /* the print that last opened it in full */
    long print_in;  /* the print that last wrote it in full, and the number it got there */
    int print_as;
    long froze_in;  /* the ice file that last took it */
} Node;

static Node *nodes;
static int nnodes, cap_nodes;
static int *slots; /* open-addressed index over ids */
static int nslots;

static u32 slot_of(const u8 *id) { return ((u32)id[0] << 24 | (u32)id[1] << 16 | (u32)id[2] << 8 | id[3]) & (u32)(nslots - 1); }

static int find(const u8 *id) {
    if (!nslots) return -1;
    for (u32 s = slot_of(id);; s = (s + 1) & (u32)(nslots - 1)) {
        if (slots[s] < 0) return -1;
        if (memcmp(nodes[slots[s]].id, id, 32) == 0) return slots[s];
    }
}
static void index_node(int x) {
    u32 s = slot_of(nodes[x].id);
    while (slots[s] >= 0) s = (s + 1) & (u32)(nslots - 1);
    slots[s] = x;
}

/* The first rule a node's canonical bytes break, in the order of section 7, or NULL. */
static const char *refusal(const u8 *b, size_t len) {
    static const size_t width[15] = {0, 0, 0, 0, 0, 1, 2, 4, 8, 1, 2, 4, 8, 4, 8};
    if (len < 33) return "short";
    if (b[0] > 1) return "kind";
    int composite = b[0];
    const u8 *p = b + 33;
    size_t n = len - 33;
    if (is_nothing(b + 1)) return !composite && n == 0 ? NULL : "nothing-blueprint";
    if (composite) {
        if (n % 32) return "child-length";
        for (size_t i = 0; i < n; i += 32)
            if (is_nothing(p + i)) return "nothing-child";
    }
    int bp = find(b + 1);
    if (bp < 0) return "unknown";
    for (size_t i = 0; composite && i < n; i += 32)
        if (find(p + i) < 0) return "unknown";
    if (!nodes[bp].entry || nodes[bp].number > 31) return NULL;
    int e = (int)nodes[bp].number;
    if (e <= 14 && composite) return "leaf-only";
    if (e == 16) return composite ? NULL : "composite-only";
    if (e >= 15) return "kept";
    if (e == 0) return n >= 1 && n <= 4 && p[n - 1] != 0 ? NULL : "ordinal";
    if (width[e] && n != width[e]) return "width";
    if (e == 3 && n && p[n - 1] == 0) return "minimal";
    if (e == 4 && n) {
        u8 last = p[n - 1];
        int before_top = n > 1 ? p[n - 2] >> 7 : -1;
        if (last == 0x00 && before_top != 1) return "minimal";
        if (last == 0xff && before_top == 1) return "minimal";
    }
    if (e == 2 && !utf8_ok(p, n)) return "utf8";
    if (e == 13 && (p[3] & 0x7f) == 0x7f && (p[2] & 0x80) && ((p[2] & 0x7f) | p[1] | p[0]))
        if (memcmp(p, "\x00\x00\xc0\x7f", 4)) return "nan";
    if (e == 14 && (p[7] & 0x7f) == 0x7f && (p[6] & 0xf0) == 0xf0 && ((p[6] & 0x0f) | p[5] | p[4] | p[3] | p[2] | p[1] | p[0]))
        if (memcmp(p, "\x00\x00\x00\x00\x00\x00\xf8\x7f", 8)) return "nan";
    return NULL;
}

/* Take canonical bytes in. Answers the node's index, or -1 with *why naming the refusal. */
static int admit(const u8 *b, size_t len, const char **why) {
    u8 id[32];
    if ((*why = refusal(b, len))) return -1;
    sha256(b, len, id);
    int x = find(id);
    if (x >= 0) return x;
    Node n;
    memset(&n, 0, sizeof n);
    n.read_in = n.print_in = n.froze_in = -1;
    n.blueprint = is_nothing(b + 1) ? -1 : find(b + 1);
    if (b[0] == 1) {
        size_t kids = (len - 33) / 32;
        n.children = grow(NULL, kids * sizeof(int));
        for (size_t i = 0; i < kids; i++) n.children[i] = find(b + 33 + 32 * i);
    }
    memcpy(n.id, id, 32);
    n.len = len;
    n.bytes = memcpy(grow(NULL, len), b, len);
    if (n.blueprint < 0) n.entry = 1;
    else if (b[0] == 0 && memcmp(b + 1, root_id, 32) == 0) {
        n.entry = 1;
        for (size_t i = len; i > 33; i--) n.number = n.number << 8 | b[i - 1];
    }
    if (nnodes == cap_nodes) nodes = grow(nodes, (size_t)(cap_nodes = cap_nodes ? 2 * cap_nodes : 256) * sizeof *nodes);
    nodes[nnodes] = n;
    if (2 * (nnodes + 1) > nslots) {
        nslots = nslots ? 2 * nslots : 1024;
        slots = grow(slots, (size_t)nslots * sizeof *slots);
        for (int i = 0; i < nslots; i++) slots[i] = -1;
        for (int i = 0; i < nnodes; i++) index_node(i);
    }
    index_node(nnodes);
    return nnodes++;
}

static int entry(u32 n) {
    u8 b[37];
    const char *why;
    return admit(b, entry_bytes(n, b), &why);
}

/* ---- section 6: reading the print ---- */
typedef struct {
    const char *s;
    size_t n, i;
    int *opened;        /* node index per number given, -1 while its bracket is open */
    int nopened, cap;
    long serial;
    const char *why;
} Reading;

static long serial;

static int stop(Reading *r, const char *why) {
    if (!r->why) r->why = why;
    return -1;
}
static int number(Reading *r, u32 *out) {
    size_t start = r->i;
    u64 v = 0;
    while (r->i < r->n && r->s[r->i] >= '0' && r->s[r->i] <= '9') {
        if (v <= 0xffffffffu) v = v * 10 + (u64)(r->s[r->i] - '0');
        r->i++;
    }
    size_t digits = r->i - start;
    if (!digits || (digits > 1 && r->s[start] == '0') || v > 0xffffffffu) return stop(r, "syntax");
    *out = (u32)v;
    return 0;
}
static int hexval(char c) { return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1; }

static int read_node(Reading *r) {
    if (r->i >= r->n) return stop(r, "syntax");
    char c = r->s[r->i++];
    u32 k;
    if (c == '#') return number(r, &k) ? -1 : entry(k);
    if (c == '^') {
        if (number(r, &k)) return -1;
        if (k >= (u32)r->nopened || r->opened[k] < 0) return stop(r, "forward");
        return r->opened[k];
    }
    if (c != '[' && c != '(') return stop(r, "syntax");
    if (r->nopened == r->cap) r->opened = grow(r->opened, (size_t)(r->cap = r->cap ? 2 * r->cap : 64) * sizeof(int));
    int mine = r->nopened++;
    r->opened[mine] = -1;
    int bp = read_node(r);
    if (bp < 0) return -1;
    size_t len = 33, room = 256;
    u8 *b = grow(NULL, room);
    b[0] = c == '(';
    memcpy(b + 1, nodes[bp].id, 32);
    if (c == '[') {
        if (r->i < r->n && r->s[r->i] == ' ') {
            r->i++;
            size_t pairs = 0;
            while (r->i + 1 < r->n && hexval(r->s[r->i]) >= 0 && hexval(r->s[r->i + 1]) >= 0) {
                if (len == room) b = grow(b, room *= 2);
                b[len++] = (u8)(hexval(r->s[r->i]) << 4 | hexval(r->s[r->i + 1]));
                r->i += 2;
                pairs++;
            }
            if (!pairs) { free(b); return stop(r, "syntax"); }
        }
        if (r->i >= r->n || r->s[r->i] != ']') { free(b); return stop(r, "syntax"); }
    } else {
        while (r->i < r->n && r->s[r->i] == ' ') {
            r->i++;
            int kid = read_node(r);
            if (kid < 0) { free(b); return -1; }
            if (len + 32 > room) b = grow(b, room *= 2);
            memcpy(b + len, nodes[kid].id, 32);
            len += 32;
        }
        if (r->i >= r->n || r->s[r->i] != ')') { free(b); return stop(r, "syntax"); }
    }
    r->i++;
    const char *why;
    int x = admit(b, len, &why);
    free(b);
    if (x < 0) return stop(r, why);
    if (nodes[x].entry) return stop(r, "not-canonical");              /* rule 2 */
    if (nodes[x].read_in == r->serial) return stop(r, "not-canonical"); /* rule 4 */
    nodes[x].read_in = r->serial;
    return r->opened[mine] = x;
}

/* Read text holding one printed node. Answers its index, or -1 with *why. */
static int read_print(const char *s, size_t n, const char **why) {
    Reading r = {s, n, 0, NULL, 0, 0, ++serial, NULL};
    int x = read_node(&r);
    if (x >= 0 && r.i != r.n) x = stop(&r, "syntax");
    free(r.opened);
    *why = r.why;
    return x;
}

/* ---- section 6: printing ---- */
typedef struct { char *s; size_t n, room; long serial; int given; } Print;

static void put(Print *p, const char *s) {
    size_t k = strlen(s);
    if (p->n + k + 1 > p->room) p->s = grow(p->s, p->room = 2 * (p->n + k + 1));
    memcpy(p->s + p->n, s, k + 1);
    p->n += k;
}
static void print_node(Print *p, int x) {
    Node *n = &nodes[x];
    char t[16];
    if (n->entry) { snprintf(t, sizeof t, "#%u", n->number); put(p, t); return; }
    if (n->print_in == p->serial) { snprintf(t, sizeof t, "^%d", n->print_as); put(p, t); return; }
    n->print_in = p->serial;
    n->print_as = p->given++;
    int composite = n->bytes[0];
    put(p, composite ? "(" : "[");
    print_node(p, n->blueprint);
    if (composite)
        for (size_t i = 0; i < (n->len - 33) / 32; i++) { put(p, " "); print_node(p, nodes[x].children[i]); }
    else if (n->len > 33) {
        put(p, " ");
        for (size_t i = 33; i < n->len; i++) { snprintf(t, sizeof t, "%02x", nodes[x].bytes[i]); put(p, t); }
    }
    put(p, composite ? ")" : "]");
}
static char *print_of(int x) {
    Print p = {NULL, 0, 0, ++serial, 0};
    put(&p, "");
    print_node(&p, x);
    return p.s;
}

/* ---- files, hex ---- */
static u8 *slurp(const char *path, size_t *n) {
    FILE *f = fopen(path, "rb");
    if (!f) die("cannot open", path);
    size_t room = 1 << 16;
    u8 *b = grow(NULL, room);
    *n = 0;
    for (size_t got; (got = fread(b + *n, 1, room - *n, f)) > 0;)
        if ((*n += got) == room) b = grow(b, room *= 2);
    fclose(f);
    return b;
}
static void hex(const u8 *b, size_t n, char *out) {
    for (size_t i = 0; i < n; i++) snprintf(out + 2 * i, 3, "%02x", b[i]);
    out[2 * n] = 0;
}
static long unhex(const char *s, size_t n, u8 *out) { /* -1 unless pairs of 0-9 a-f */
    if (n % 2) return -1;
    for (size_t i = 0; i < n; i += 2) {
        int a = hexval(s[i]), b = hexval(s[i + 1]);
        if (a < 0 || b < 0) return -1;
        out[i / 2] = (u8)(a << 4 | b);
    }
    return (long)(n / 2);
}

/* ---- vectors ---- */
/* Does one line hold? field[0] is the line's kind; the rest as the vector file's header says. */
static int holds(char **field, size_t *flen, int nfields, char *said, size_t room) {
    const char *why = NULL;
    said[0] = 0;
    if (!strcmp(field[0], "ok") && nfields == 4) {
        int x = read_print(field[1], flen[1], &why);
        if (x < 0) { snprintf(said, room, "refused as %s", why); return 0; }
        char *bytes = grow(NULL, 2 * nodes[x].len + 1), id[65], *again = print_of(x);
        hex(nodes[x].bytes, nodes[x].len, bytes);
        hex(nodes[x].id, 32, id);
        int ok = 1;
        if (strcmp(bytes, field[2])) { snprintf(said, room, "bytes %s", bytes); ok = 0; }
        else if (strcmp(id, field[3])) { snprintf(said, room, "id %s", id); ok = 0; }
        else if (strcmp(again, field[1])) { snprintf(said, room, "prints back as %s", again); ok = 0; }
        free(bytes);
        free(again);
        return ok;
    }
    if (!strcmp(field[0], "bad-print") && nfields == 3) {
        int x = read_print(field[1], flen[1], &why);
        snprintf(said, room, "%s", x < 0 ? why : "accepted");
        return x < 0 && !strcmp(why, field[2]);
    }
    if (!strcmp(field[0], "bad-bytes") && nfields == 3) {
        u8 *b = grow(NULL, flen[1] / 2 + 1);
        long n = unhex(field[1], flen[1], b);
        if (n < 0) why = "not hex";
        else admit(b, (size_t)n, &why);
        free(b);
        snprintf(said, room, "%s", why ? why : "accepted");
        return why && !strcmp(why, field[2]);
    }
    snprintf(said, room, "not a vector line");
    return 0;
}

static int split(char *line, char **field, size_t *flen, int max) {
    int k = 0;
    for (char *p = line; k < max; p++) {
        field[k] = p;
        p += strcspn(p, "\t");
        flen[k] = (size_t)(p - field[k]);
        k++;
        if (!*p) break;
        *p = 0;
    }
    return k;
}

static int vectors(const char *path) {
    size_t n;
    char *text = (char *)slurp(path, &n), said[256];
    text = grow(text, n + 1);
    text[n] = 0;
    int held = 0, not = 0, lineno = 0;
    for (char *line = text, *next; line < text + n; line = next) {
        next = line + strcspn(line, "\n");
        if (*next) *next++ = 0;
        lineno++;
        if (!*line || *line == '#') continue;
        char *field[5];
        size_t flen[5];
        int k = split(line, field, flen, 5);
        if (holds(field, flen, k, said, sizeof said)) held++;
        else { not++; printf("line %d does not hold: %s '%s': %s\n", lineno, field[0], k > 1 ? field[1] : "", said); }
    }
    int beyond = 0; /* what the bootstrap's region shows as node records beside its 64 entries and nothing */
    for (int i = 0; i < nnodes; i++) beyond += !(nodes[i].entry && nodes[i].number < 64);
    printf("second: %d vectors hold, %d do not; %d nodes held beyond entries 0 to 63\n", held, not, beyond);
    return not ? 1 : 0;
}

/* ---- one printed node from a file; one node from ice ---- */
static void show(int x) {
    char id[65], *p = print_of(x);
    hex(nodes[x].id, 32, id);
    printf("%s\n%s\n", id, p);
    free(p);
}
static int read_file(const char *path) {
    size_t n;
    const char *why;
    char *s = (char *)slurp(path, &n);
    if (!n || s[n - 1] != '\n') die("refused: a file ends with one line feed:", path);
    int x = read_print(s, n - 1, &why);
    if (x < 0) die("refused:", why);
    show(x);
    return 0;
}
/* Graph node 5.14.8: eight bytes of magic, then records of a 32-bit little-endian length and one
 * node's canonical bytes, children before parents, the top node last, the root left out. */
static const char magic[8] = "CKICE1\n";
static long freezing;

static void freeze_node(FILE *f, int x) {
    Node *n = &nodes[x];
    if (n->blueprint < 0 || n->froze_in == freezing) return;
    n->froze_in = freezing;
    freeze_node(f, n->blueprint);
    for (size_t i = 0; n->bytes[0] == 1 && i < (n->len - 33) / 32; i++) freeze_node(f, n->children[i]);
    u8 len[4] = {(u8)n->len, (u8)(n->len >> 8), (u8)(n->len >> 16), (u8)(n->len >> 24)};
    fwrite(len, 1, 4, f);
    fwrite(n->bytes, 1, n->len, f);
}
static int freeze(const char *path, const char *out) {
    size_t n;
    const char *why;
    char *s = (char *)slurp(path, &n);
    if (!n || s[n - 1] != '\n') die("refused: a file ends with one line feed:", path);
    int x = read_print(s, n - 1, &why);
    if (x < 0) die("refused:", why);
    FILE *f = fopen(out, "wb");
    if (!f) die("cannot write", out);
    freezing = ++serial;
    fwrite(magic, 1, 8, f);
    freeze_node(f, x);
    if (fclose(f)) die("cannot write", out);
    char id[65];
    hex(nodes[x].id, 32, id);
    printf("%s\n", id);
    return 0;
}
static int thaw(const char *path) {
    size_t n, i = 8;
    const char *why;
    u8 *b = slurp(path, &n);
    int top = -1;
    if (n < 8 || memcmp(b, magic, 8)) die("refused: no magic in", path);
    while (i < n) {
        if (n - i < 4) die("refused: a length cut short in", path);
        size_t len = (size_t)b[i] | (size_t)b[i + 1] << 8 | (size_t)b[i + 2] << 16 | (size_t)b[i + 3] << 24;
        i += 4;
        if (len > n - i) die("refused: a record cut short in", path);
        if ((top = admit(b + i, len, &why)) < 0) die("refused:", why);
        i += len;
    }
    if (top < 0) die("refused: no record in", path);
    show(top);
    return 0;
}

/* ---- variations: inputs neither hand chose, with this reader's verdict on each ---- */
static u64 seed;
static u32 rnd(u32 below) {
    seed ^= seed << 13;
    seed ^= seed >> 7;
    seed ^= seed << 17;
    return (u32)((seed >> 11) % below);
}
static size_t vary_text(const char *s, size_t n, char *out) {
    static const char alphabet[] = "#^[]() 0123456789abcdef";
    memcpy(out, s, n);
    for (int times = 1 + (int)rnd(3); times; times--) {
        size_t at = rnd((u32)n + 1);
        char c = alphabet[rnd(sizeof alphabet - 1)];
        switch (rnd(4)) {
        case 0: if (at < n) { memmove(out + at, out + at + 1, n - at - 1); n--; } break;
        case 1: memmove(out + at + 1, out + at, n - at); out[at] = c; n++; break;
        case 2: if (at < n) out[at] = c; break;
        default: /* say again a stretch that is already there */
            if (n) {
                size_t from = rnd((u32)n), len = 1 + rnd((u32)(n - from));
                if (len > 40) len = 40;
                memmove(out + at + len, out + at, n - at);
                memmove(out + at, at <= from ? out + from + len : out + from, len);
                n += len;
            }
        }
    }
    out[n] = 0;
    return n;
}
static size_t vary_bytes(const u8 *b, size_t n, u8 *out) {
    memcpy(out, b, n);
    switch (rnd(6)) {
    case 0: if (n) n -= 1 + rnd(n > 33 ? 33 : (u32)n); break;
    case 1: out[n++] = (u8)rnd(256); break;
    case 2: if (n) out[rnd((u32)n)] = (u8)rnd(256); break;
    case 3: if (n >= 33) memcpy(out + 1, nodes[entry(rnd(32))].id, 32); break;
    case 4: if (n) out[n - 1] = rnd(2) ? 0x00 : 0xff; break;
    default: if (n >= 33) { memcpy(out + 1, nodes[entry(rnd(15))].id, 32); n = 33 + rnd(10); for (size_t i = 33; i < n; i++) out[i] = (u8)(rnd(4) ? rnd(256) : rnd(2) ? 0xff : 0x00); }
    }
    return n;
}
static void say_ok(int x) { /* an ok line for a node this reader accepts */
    char *bytes = grow(NULL, 2 * nodes[x].len + 1), id[65], *p = print_of(x);
    hex(nodes[x].bytes, nodes[x].len, bytes);
    hex(nodes[x].id, 32, id);
    printf("ok\t%s\t%s\t%s\n", p, bytes, id);
    free(bytes);
    free(p);
}
static int vary(const char *path, long count, u64 from) {
    size_t n, nlines = 0, room = 0;
    char *text = (char *)slurp(path, &n), **lines = NULL;
    const char *why;
    text = grow(text, n + 1);
    text[n] = 0;
    seed = from * 0x9e3779b97f4a7c15ull + 1;
    for (char *line = text, *next; line < text + n; line = next) {
        next = line + strcspn(line, "\n");
        if (*next) *next++ = 0;
        if (!*line || *line == '#') continue;
        if (nlines == room) lines = grow(lines, (room = room ? 2 * room : 256) * sizeof *lines);
        lines[nlines++] = line;
    }
    if (!nlines) die("no vectors in", path);
    /* First the nodes the vectors accept, so that every reader of the output holds what this one does. */
    for (size_t i = 0; i < nlines; i++)
        if (!strncmp(lines[i], "ok\t", 3)) {
            size_t len = strcspn(lines[i] + 3, "\t");
            if (read_print(lines[i] + 3, len, &why) >= 0) printf("%s\n", lines[i]);
        }
    for (long made = 0; made < count; made++) {
        const char *pick = lines[rnd((u32)nlines)];
        char *copy = strcpy(grow(NULL, strlen(pick) + 1), pick), *field[5];
        size_t flen[5];
        int k = split(copy, field, flen, 5);
        if (k > 2 && strcmp(field[0], "bad-bytes") && (strcmp(field[0], "ok") || rnd(4))) {
            char *t = grow(NULL, 2 * flen[1] + 256);
            size_t tn = vary_text(field[1], flen[1], t);
            int x = read_print(t, tn, &why);
            if (x < 0) printf("bad-print\t%s\t%s\n", t, why);
            else say_ok(x);
            free(t);
        } else if (k > 2) {
            int col = strcmp(field[0], "ok") ? 1 : 2;
            u8 *b = grow(NULL, flen[col] / 2 + 64), *v = grow(NULL, flen[col] / 2 + 64);
            long bn = unhex(field[col], flen[col], b);
            size_t vn = vary_bytes(b, bn < 0 ? 0 : (size_t)bn, v);
            char *h = grow(NULL, 2 * vn + 1);
            hex(v, vn, h);
            int x = admit(v, vn, &why);
            if (x < 0) printf("bad-bytes\t%s\t%s\n", h, why);
            else say_ok(x);
            free(b);
            free(v);
            free(h);
        }
        free(copy);
    }
    return 0;
}

int main(int argc, char **argv) {
    u8 b[37];
    sha256(b, entry_bytes(0, b), root_id);
    for (u32 n = 0; n < 32; n++) entry(n); /* the entries that carry rules are held from the start */
    if (argc == 3 && !strcmp(argv[1], "vectors")) return vectors(argv[2]);
    if (argc == 3 && !strcmp(argv[1], "read")) return read_file(argv[2]);
    if (argc == 4 && !strcmp(argv[1], "freeze")) return freeze(argv[2], argv[3]);
    if (argc == 3 && !strcmp(argv[1], "thaw")) return thaw(argv[2]);
    if (argc == 5 && !strcmp(argv[1], "vary")) return vary(argv[2], atol(argv[3]), (u64)atol(argv[4]));
    fprintf(stderr, "usage: second vectors FILE | read FILE | freeze FILE OUT | thaw FILE | vary FILE N SEED\n");
    return 2;
}
