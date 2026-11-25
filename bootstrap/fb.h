/* fb.h - the framebuffer's layout and the catalogue's numbers, as the C compiler needs them.
 * Made by region/header.ck from lens/words.ck and region/layout.ck (spec/framebuffer.md).
 * It is not written by hand: change the nodes and run 'make header'. */
#ifndef CK_FB_H
#define CK_FB_H
#include <stdint.h>
typedef uint32_t H;                       /* a handle: an index into the node table; 0 is nothing */
#define ROOT 1u
#define E(n) ((H)((n) + 1))               /* handle of catalogue entry n, n below SEEDED */
#define SEEDED 97u
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
AT = 96, LAST = 96 };
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
[96] = "at", 
enum { A_NODES, A_LEAF_BYTES, A_CHILDREN, A_BY_CONTENT, A_BY_ID, A_FRAMES, A_LINKS, A_ACTIVATIONS, A_ARGUMENTS, A_CHOICE_POINTS, A_GATHERED, A_BORN_CODE, A_BINDINGS, A_BOUND_BY_NODE, A_CELLS, A_TRAIL, A_READER_OPENED, A_GATHERING_CHILDREN, A_GATHERING_BYTES, A_PRINT, A_PRINT_NUMBERS, A_TEXT, A_STACK, A_COUNT };
#define FB_AREA_NAMES "nodes", "leaf-bytes", "children", "by-content", "by-id", "frames", "links", "activations", "arguments", "choice-points", "gathered", "born-code", "bindings", "bound-by-node", "cells", "trail", "reader-opened", "gathering-children", "gathering-bytes", "print", "print-numbers", "text", "stack", 
enum { F_NONE, F_SEQ, F_ARGS, F_CUT, F_FAIL, F_ALL, F_NEED, F_RECOVER, F_HANDLER, F_OUTCOME, F_LAYER, F_COUNT };
#define FB_FRAME_NAMES "?", "seq", "args", "cut", "fail", "all", "need", "recover", "handler", "outcome", "layer", 
enum { M_EVAL, M_RET, M_FAILED, M_FAULT, M_DONE, M_COUNT };
#define FB_MODE_NAMES "eval", "ret", "failed", "fault", "done", 
typedef struct { H bp; H by; uint32_t mode; H at; H x; H v; H reason; uint32_t env; uint32_t cut; uint32_t k; uint32_t outcome; uint32_t gathers; uint32_t ids; uint32_t pcount; uint32_t spare; uint32_t ncode; uint32_t one; uint32_t pad; uint64_t step; uint64_t code; } Machine;
typedef struct { H bp; H by; H of; uint32_t size; uint32_t cap; uint32_t n; uint64_t off; } Area;
typedef struct { H bp; H by; uint32_t n; uint32_t off; uint8_t comp; uint8_t hasid; uint8_t pad[2]; uint8_t id[32]; } Node;
typedef struct { H bp; H by; uint32_t t; H node; uint32_t i; uint32_t env; uint32_t cut; uint32_t vals; uint32_t k; uint32_t aux; uint32_t mact; } Frame;
typedef struct { H bp; H by; H v; uint32_t next; } Link;
typedef struct { H bp; H by; H recipe; uint32_t off; uint32_t n; uint32_t lim; uint32_t up; uint32_t cut; } Activation;
typedef struct { H bp; H by; uint32_t all; H node; uint32_t i; uint32_t env; uint32_t k; uint32_t mfr; uint32_t mlink; uint32_t mact; uint32_t marg; uint32_t coll; uint32_t mbind; uint32_t mtrail; } ChoicePoint;
typedef struct { H bp; H by; uint32_t id; H v; } Gathered;
typedef struct { H bp; H by; H leaf; uint32_t off; } Code;
typedef struct { H bp; H by; H entry; H impl; uint32_t prev; uint32_t sees; } Binding;
typedef struct { H bp; H by; H key; H old; } Trail;
typedef struct { uint8_t magic[8]; H bp; H by; uint32_t version; uint32_t areas; uint32_t pid; uint32_t floor; uint64_t bytes; Machine m; Area a[A_COUNT]; } Head;
#define FB_MAGIC "CKFB1\n\0"
#define FB_PAGE 16384u
#endif
