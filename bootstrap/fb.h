/* fb.h - the framebuffer's layout (spec/framebuffer.md; graph 5.7.6 to 5.7.8).
 *
 * One shared region holds everything the bootstrap reads and writes: every node, every
 * frame, the walker's registers, the reader's working lists and the machine stack.
 * bootstrap/ck.c is its only writer. A second process (tools/fb.c) maps it read-only,
 * copies it as frames and replays them; the writer never calls or waits for a reader.
 *
 * Every record starts with two handles: its blueprint and the program node that last
 * wrote it (nothing when the bootstrap's own reader did). A reference is a handle or an
 * offset from the region's first byte, never an address, so a copy reads the same in
 * another process and at a later time. A record is complete before the count that makes
 * it reachable moves past it.
 */
#ifndef CK_FB_H
#define CK_FB_H
#include <stdint.h>

typedef uint32_t H;                       /* a handle: an index into the node table; 0 is nothing */
#define ROOT 1u
#define SEEDED 87u                 /* every entry this file names is in the store from the start */
#define E(n) ((H)((n) + 1))               /* handle of catalogue entry n, n below SEEDED */
enum { NAT = 3, INT = 4, LIST = 16,
       NO = 32, YES, QUOTE, ARG, RECIPE, CALL, SELF, SEQ, CHOICE, FAIL, CUT, ALL, NEED, RECOVER,
       ADD, SUB, MUL, EQ, LT, NATIVE_OP, CODE,                      /* 32 to 52: spec/core-operations.md */
       FB, AREA, MACHINE, NODE, FRAME, LINK, ACT, CHOICEPT, GATHERED, WORDS, BYTES,    /* 53 to 63 */
       BLUEPRINT, COUNT, CHILD, BYTE, LEAF, PACK, JOIN,             /* 64 to 70: a node's parts */
       BIND, BINDING,                         /* 71 gives a node an implementation; 72 is that record's kind */
       UP, BASE, LAZY,                        /* 73, 74: evaluate where the caller stands; 75: a recipe given the node */
       READ, PRINT, ID, FILEOP, CLOCK,        /* 76 to 80: what the bootstrap does for itself, as operations */
       STEP,                                  /* 81: a node is evaluated; what it is bound to is given every node */
       OUTCOME,                               /* 82: how a node's evaluation ended, as a node; what it did is undone */
       DIV, MOD,                              /* 83, 84: integer division and its remainder */
       RAISE,                                 /* 85, fault: a program breaks off with a reason, as the walker does */
       LAYER, LAST = LAYER };                 /* 86: the bindings made inside it are one layer and see each other */

typedef struct { H bp, by; uint32_t n, off; uint8_t comp, hasid, pad[2]; uint8_t id[32]; } Node;
enum { F_SEQ = 1, F_ARGS, F_CUT, F_FAILR, F_ALL, F_NEED, F_RECOVER, F_HANDLER, F_OUT, F_LAYER };
typedef struct { H bp, by; uint32_t t; H node; uint32_t i, env, cut, vals, k, aux, mact; } Frame;
typedef struct { H bp, by; H v; uint32_t next; } Link;
typedef struct { H bp, by; H recipe; uint32_t off, n, lim, up, cut; } Act;   /* lim: how many bindings it
                 runs on; up, cut: the activation and the cut scope of whoever used the implementation it belongs to */
typedef struct { H bp, by; uint32_t all; H node; uint32_t i, env, k, mfr, mlink, mact, marg, coll, mbind; } Choice;
                 /* all: 0 a choice, 1 an all (coll names its gathering), 2 an outcome (coll is what was seen) */
typedef struct { H bp, by; uint32_t id; H v; } Gathered;
typedef struct { H bp, by; H leaf; uint32_t off; } Born;      /* a runnable copy of a code leaf */
typedef struct { H bp, by; H entry, impl; uint32_t prev, sees; } Bind;   /* entry is run by impl. prev: the
                 binding of the same entry made before this one, counted from 1. sees: how many bindings
                 impl runs on: those beneath its own, or all of the layer it was made in */

/* An area is one table. "of" is the blueprint of its records, "size" one record's bytes,
 * "n" how many are in use and "cap" how many it can hold. "off" is where it starts. */
typedef struct { H bp, by; H of; uint32_t size, cap, n; uint64_t off; } Area;
enum { A_NODE, A_BYTES, A_KIDS, A_TAB, A_IDTAB, A_FRAME, A_LINK, A_ACT, A_ARG, A_CHOICE,
       A_GATHER, A_BORN, A_BIND, A_BOUND, A_OPEN, A_TK, A_TB, A_OUT, A_PNUM, A_SEEN, A_TEXT, A_STACK, A_COUNT };
#define FB_AREA_NAMES "nodes", "leaf-bytes", "children", "by-content", "by-id", "frames", "links", \
  "activations", "arguments", "choice-points", "gathered", "born-code", "bindings", "bound-by-node", "reader-opened", \
  "gathering-children", "gathering-bytes", "print", "print-numbers", "frozen-marks", "text", "stack"

/* The walker's registers. "at" is the program node being run: the writer stamped on
 * whatever is made next. "step" counts the walker's steps and is the region's clock.
 * "code" is the one address kept here: where this process mapped its executable pages. */
enum { M_EVAL, M_RET, M_FAILED, M_FAULT, M_DONE };
typedef struct { H bp, by; uint32_t mode; H at, x, v, reason; uint32_t env, cut, k;
                 uint32_t outcome, gathers, ids, pcount, loose, ncode, one, pad; uint64_t step, code; } Machine;

#define FB_MAGIC "CKFB1\n\0"
#define FB_PAGE 16384u
typedef struct { char magic[8]; H bp, by; uint32_t version, areas, pid, pad; uint64_t bytes;
                 Machine m; Area a[A_COUNT]; } Head;
#endif
