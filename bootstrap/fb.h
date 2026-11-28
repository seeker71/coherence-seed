/* fb.h - the framebuffer's layout and the catalogue's numbers, as the C compiler needs them.
 * Made by region/header.ck from lens/words.ck and region/layout.ck (spec/framebuffer.md).
 * It is not written by hand: change the nodes and run 'make header'. */
#ifndef CK_FB_H
#define CK_FB_H
#include <stdint.h>
typedef uint32_t H;                       /* a handle: an index into the node table; 0 is nothing */
#define ROOT 1u
#define E(n) ((H)((n) + 1))               /* handle of catalogue entry n, n below SEEDED */
#define SEEDED 128u
enum {
BYTES = 1, TEXT = 2, NAT = 3, INT = 4, U8 = 5, U16 = 6, U32 = 7,
U64 = 8, I8 = 9, I16 = 10, I32 = 11, I64 = 12, F32 = 13, F64 = 14, LIST = 16, NO = 32, YES = 33, QUOTE = 34, ARG = 35, RECIPE = 36, CALL = 37, SELF = 38, SEQ = 39,
CHOICE = 40, FAIL = 41, CUT = 42, ALL = 43, NEED = 44, RECOVER = 45, ADD = 46, SUB = 47,
MUL = 48, EQ = 49, LT = 50, NATIVE_OP = 51, CODE = 52, FRAMEBUFFER = 53, AREA = 54, MACHINE = 55,
NODE = 56, FRAME = 57, LINK = 58, ACTIVATION = 59, CHOICE_POINT = 60, GATHERED = 61, WORD_TABLE = 62, BYTE_TABLE = 63,
BLUEPRINT = 64, COUNT = 65, CHILD = 66, BYTE = 67, LEAF = 68, PACK = 69, JOIN = 70, BIND = 71,
BINDING = 72, UP = 73, BASE = 74, LAZY = 75, READ = 76, PRINT = 77, ID = 78, FILE_OP = 79,
CLOCK = 80, STEP = 81, OUTCOME = 82, DIV = 83, MOD = 84, FAULT = 85, LAYER = 86, ENCODE = 87,
DECODE = 88, HOLD = 89, HELD = 90, TRAIL = 91, NOTE = 92, UNBIND = 93, PEEK = 94, HANDLE = 95,
AT = 96, CAS = 97, HOST = 98, RANGE = 99, LET = 100, OUTSIDE = 101, BOUND = 102, AFTER = 103,
NOTHING = 104, CLOSE = 105, CLOSED = 106, BAND = 107, BOR = 108, BXOR = 109, SHL = 110, SHR = 111,
FAST = 112, ROOM = 113, ROOM_TRAIL = 114, TAKE = 115, LOAD = 116, STORE = 117, WADD = 118, WSUB = 119,
WMUL = 120, ULT = 121, UDIV = 122, UMOD = 123, HOST_TARGET = 124, HAS_TARGET = 125, SUBSTRATE = 126, LANE = 127,
LAST = 127 };
#define FB_ENTRY_NAMES \
[0] = "root", [1] = "bytes", [2] = "text", [3] = "nat", [4] = "int", [5] = "u8", \
[6] = "u16", [7] = "u32", [8] = "u64", [9] = "i8", [10] = "i16", [11] = "i32", \
[12] = "i64", [13] = "f32", [14] = "f64", [16] = "list", [32] = "no", [33] = "yes", [34] = "quote", [35] = "arg", \
[36] = "recipe", [37] = "call", [38] = "self", [39] = "seq", [40] = "choice", [41] = "fail", \
[42] = "cut", [43] = "all", [44] = "need", [45] = "recover", [46] = "add", [47] = "sub", \
[48] = "mul", [49] = "eq", [50] = "lt", [51] = "native", [52] = "code", [53] = "framebuffer", \
[54] = "area", [55] = "machine", [56] = "node", [57] = "frame", [58] = "link", [59] = "activation", \
[60] = "choice-point", [61] = "gathered", [62] = "word-table", [63] = "byte-table", [64] = "blueprint", [65] = "count", \
[66] = "child", [67] = "byte", [68] = "leaf", [69] = "pack", [70] = "join", [71] = "bind", \
[72] = "binding", [73] = "up", [74] = "base", [75] = "lazy", [76] = "read", [77] = "print", \
[78] = "id", [79] = "file", [80] = "clock", [81] = "step", [82] = "outcome", [83] = "div", \
[84] = "mod", [85] = "fault", [86] = "layer", [87] = "encode", [88] = "decode", [89] = "hold", \
[90] = "held", [91] = "trail", [92] = "note", [93] = "unbind", [94] = "peek", [95] = "handle", \
[96] = "at", [97] = "cas", [98] = "host", [99] = "range", [100] = "let", [101] = "outside", \
[102] = "bound", [103] = "after", [104] = "nothing", [105] = "close", [106] = "closed", [107] = "band", \
[108] = "bor", [109] = "bxor", [110] = "shl", [111] = "shr", [112] = "fast", [113] = "room", \
[114] = "room-trail", [115] = "take", [116] = "load", [117] = "store", [118] = "wadd", [119] = "wsub", \
[120] = "wmul", [121] = "ult", [122] = "udiv", [123] = "umod", [124] = "host-target", [125] = "has-target", \
[126] = "substrate", [127] = "lane",
#define TARGET_AAPCS64 UINT64_C(1297036695148953601)
#define TARGET_AMD64_SYSV UINT64_C(1297036695148953602)
#define E_MEMBER_INDEX UINT64_C(1153484467445170460)
#define E_MEMBER_SHAPE UINT64_C(1153484467445170461)
enum { A_NODES, A_LEAF_BYTES, A_CHILDREN, A_BY_CONTENT, A_BY_ID, A_FRAMES, A_LINKS, A_ACTIVATIONS, A_ARGUMENTS, A_CHOICE_POINTS, A_GATHERED, A_BORN_CODE, A_BINDINGS, A_BOUND_BY_NODE, A_OUTSIDE, A_CELLS, A_TRAIL, A_READER_OPENED, A_GATHERING_CHILDREN, A_GATHERING_BYTES, A_PRINT_NUMBERS, A_TEXT, A_STACK, A_ASKED, A_ANSWERED, A_AFTER, A_NOTED, A_ROOMS, A_ROOM_TRAIL, A_GIVES, A_IDS, A_VALUES, A_SEEN, A_TOUCHED, A_COUNT };
#define FB_AREA_NAMES "nodes", "leaf-bytes", "children", "by-content", "by-id", "frames", "links", "activations", "arguments", "choice-points", "gathered", "born-code", "bindings", "bound-by-node", "outside", "cells", "trail", "reader-opened", "gathering-children", "gathering-bytes", "print-numbers", "text", "stack", "asked", "answered", "after", "noted", "rooms", "room-trail", "gives", "ids", "values", "seen", "touched", 
enum { S_LANES, S_NODES, S_LEAF_BYTES, S_CHILDREN, S_IDS, S_BY_CONTENT, S_BY_ID, S_COUNT };
#define FB_SUBSTRATE_AREA_NAMES "lanes", "nodes", "leaf-bytes", "children", "ids", "by-content", "by-id", 
enum { F_NONE, F_SEQ, F_ARGS, F_CUT, F_FAIL, F_ALL, F_NEED, F_RECOVER, F_HANDLER, F_OUTCOME, F_LAYER, F_LET, F_ASIDE, F_COUNT };
#define FB_FRAME_NAMES "?", "seq", "args", "cut", "fail", "all", "need", "recover", "handler", "outcome", "layer", "let", "aside", 
enum { M_EVAL, M_RET, M_FAILED, M_FAULT, M_DONE, M_COUNT };
#define FB_MODE_NAMES "eval", "ret", "failed", "fault", "done", 
typedef struct { H bp; H by; uint32_t mode; H at; H x; H v; H reason; uint32_t env; uint32_t cut; uint32_t k; uint32_t outcome; uint32_t gathers; uint32_t ids; uint32_t one; uint32_t ncode; H handed; uint32_t handeds; uint32_t fresh; uint64_t step; uint64_t code; uint64_t serve; } Machine;
typedef struct { H bp; H by; H of; uint32_t size; uint64_t peak; uint64_t off; uint64_t cap; uint64_t n; } Area;
typedef struct { H bp; H by; uint32_t ida; uint8_t comp; uint8_t settled; uint8_t pad[2]; uint64_t n; uint64_t off; } Node;
typedef struct { H bp; H by; uint32_t t; H node; uint32_t i; uint32_t env; uint32_t cut; uint32_t vals; uint32_t k; uint32_t aux; uint32_t mact; uint32_t mlink; } Frame;
typedef struct { H bp; H by; H v; uint32_t next; } Link;
typedef struct { H bp; H by; H recipe; uint32_t off; uint32_t n; uint32_t lim; uint32_t up; uint32_t cut; } Activation;
typedef struct { H bp; H by; uint32_t all; H node; uint32_t i; uint32_t env; uint32_t k; uint32_t mfr; uint32_t mlink; uint32_t mact; uint32_t marg; uint32_t coll; uint32_t mbind; uint32_t mtrail; uint32_t mafter; uint32_t mroom; uint32_t mnode; uint32_t mcoll; uint64_t mkids; uint64_t mbytes; uint64_t mtake; uint64_t mids; } ChoicePoint;
typedef struct { H bp; H by; uint32_t id; H v; } Gathered;
typedef struct { H bp; H by; H leaf; uint32_t off; } Code;
typedef struct { H bp; H by; H entry; H impl; uint32_t prev; uint32_t sees; H fast; } Binding;
typedef struct { H bp; H by; H key; H old; } Trail;
typedef struct { H bp; H by; uint32_t writes; uint8_t pad[4]; uint64_t size; } Room;
typedef struct { H bp; H by; uint32_t old; uint8_t pad[4]; uint64_t at; } RoomTrail;
typedef struct { uint64_t symbol; uint64_t adopt; uint64_t region; uint8_t cells[512]; } Doors;
typedef struct { uint64_t share; uint64_t charged; uint32_t lane; H leave; uint8_t name[32]; uint64_t allowance; uint32_t taken; uint8_t pad[4]; } Member;
typedef struct { uint8_t magic[8]; H bp; H by; uint32_t version; uint32_t areas; uint32_t lanes; uint32_t pid; uint64_t share; uint64_t bytes; uint64_t ids; uint64_t start; uint64_t made; uint64_t allowance; uint64_t charged; uint64_t buried; uint64_t tide; uint64_t catalogue; Area a[S_COUNT]; } Substrate;
typedef struct { H bp; H by; uint64_t who; uint64_t start; uint32_t n; uint32_t sw; uint64_t charged; uint64_t forwarded; uint8_t pad[16]; } Lane;
typedef struct { uint8_t magic[8]; H bp; H by; uint32_t version; uint32_t areas; uint32_t pid; uint32_t floor; uint64_t bytes; Machine m; Area a[A_COUNT]; Doors d; Member me; } Head;
#define FB_MAGIC "CKFB1\n\0"
#define FB_SUBSTRATE_MAGIC "CKSUB1\n\0"
#define FB_VERSION 31u
#define FB_CATALOGUE UINT64_C(63731160999100191)
#define FB_PAGE 16384u
#define FB_HUE 128u
enum { FB_LANE, FB_HANDLES, FB_STACK, FB_BITS };   /* a reservation made by the bootstrap */
#define FB_AREA_ROWS {NODE, sizeof(Node), FB_LANE}, {BYTE_TABLE, 1, FB_LANE}, {WORD_TABLE, 4, FB_LANE}, {WORD_TABLE, 4, 1073741824ull}, {WORD_TABLE, 4, 1073741824ull}, {FRAME, sizeof(Frame), 1073741824ull}, {LINK, sizeof(Link), 1073741824ull}, {ACTIVATION, sizeof(Activation), 1073741824ull}, {WORD_TABLE, 4, 1073741824ull}, {CHOICE_POINT, sizeof(ChoicePoint), 1073741824ull}, {GATHERED, sizeof(Gathered), 1073741824ull}, {CODE, sizeof(Code), 268435456ull}, {BINDING, sizeof(Binding), 268435456ull}, {WORD_TABLE, 4, FB_HANDLES}, {BINDING, sizeof(Binding), 3556ull}, {WORD_TABLE, 4, FB_HANDLES}, {TRAIL, sizeof(Trail), 1073741824ull}, {WORD_TABLE, 4, 268435456ull}, {WORD_TABLE, 4, 1073741824ull}, {BYTE_TABLE, 1, 1073741824ull}, {WORD_TABLE, 4, FB_HANDLES}, {BYTE_TABLE, 1, 1073741824ull}, {BYTE_TABLE, 1, FB_STACK}, {BYTE_TABLE, 1, 268435456ull}, {BYTE_TABLE, 1, 268435456ull}, {WORD_TABLE, 4, 268435456ull}, {WORD_TABLE, 4, 268435456ull}, {BYTE_TABLE, 1, 4294967296ull}, {ROOM_TRAIL, sizeof(RoomTrail), 1073741824ull}, {WORD_TABLE, 4, 16ull}, {BYTE_TABLE, 1, FB_LANE}, {WORD_TABLE, 4, 1073741824ull}, {WORD_TABLE, 4, 1024ull}, {BYTE_TABLE, 1, FB_BITS}, {LANE, sizeof(Lane), 64ull}, {NODE, sizeof(Node), 4294967296ull}, {BYTE_TABLE, 1, 4294967296ull}, {WORD_TABLE, 4, 4294967296ull}, {BYTE_TABLE, 1, 4294967296ull}, {WORD_TABLE, 4, 4294967296ull}, {WORD_TABLE, 4, 4294967296ull}, 
#define FB_OWN(X) X(fb->pid) X(fb->bytes) X(W->code) X(W->serve) X(fb->d.symbol) X(fb->d.adopt) X(fb->d.region) X(fb->d.cells) X(fb->me.allowance) X(fb->a[A_STACK]) X(sub->pid) X(sub->allowance) X(sub->bytes) X(ln->who) 
#define FB_KEPT(X) X(fb->a[A_BINDINGS]) X(fb->a[A_BOUND_BY_NODE]) X(fb->a[A_CELLS]) X(fb->a[A_ROOMS]) X(fb->a[A_GIVES]) X(fb->a[A_TOUCHED]) X(sub->a[S_LANES]) X(sub->a[S_NODES]) X(sub->a[S_LEAF_BYTES]) X(sub->a[S_CHILDREN]) X(sub->a[S_IDS]) X(sub->a[S_BY_CONTENT]) X(sub->a[S_BY_ID]) 
#endif
