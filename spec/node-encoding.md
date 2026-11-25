# Node encoding, version 1

This fixes what a node is as bytes, what its identity is, how it is written as text, and which
nodes exist before any other. It is meant to be enough to write a reader from this page alone.
`spec/node-vectors.tsv` holds the conformance vectors and the bootstrap's reader holds them. Where
the page, the vectors and a reader disagree, this page is right and the others are wrong.

Design nodes this stands on: 5.1.1, 5.1.4, 5.1.5, 5.2.2, 5.2.4, 5.3.4. Choices made here that
those nodes left open are listed under "Choices" at the end.

## 1. A node

A node is one of two things:

- a **leaf**: a blueprint and a string of zero or more bytes;
- a **composite**: a blueprint and an ordered list of zero or more children.

A blueprint is a node. A child is a node. Nothing else is in a node: no name, no position, no
state, no length field.

## 2. Canonical bytes

```
leaf       00 | blueprint id (32 bytes) | the leaf's bytes
composite  01 | blueprint id (32 bytes) | child id (32 bytes), one per child, in order
```

The first byte is the kind. Any other first byte is refused; 02 to ff are kept for later versions.

## 3. Identity

A node's **content id** is the SHA-256 (FIPS 180-4) of its canonical bytes: 32 bytes.

The id of 32 zero bytes names **nothing**. It is not the id of a node. It may stand in the
blueprint position of exactly one node, the root (section 4), and never in a child position.

Two nodes are the same node exactly when their canonical bytes are equal.

## 4. The catalogue

The **root** is the leaf with blueprint nothing and no bytes: the 33 bytes `00` then 32 zero
bytes. It is catalogue entry 0.

**Catalogue entry N**, for N from 1 to 4,294,967,295, is the leaf whose blueprint is the root and
whose bytes are N as a `nat` (section 5), so one to four bytes. Any other leaf whose blueprint is
the root is refused. The root is the blueprint of no composite.

An entry's content is its number. Its name is a row in a lens table beside the nodes, so a name
can change, and a new entry can be named, without changing any id. The names of version 1:

| N | name | a node with this blueprint is |
| --- | --- | --- |
| 0 | root | a catalogue entry |
| 1 | bytes | a leaf of any bytes |
| 2 | text | a leaf of well-formed UTF-8 |
| 3 | nat | a leaf: an unsigned integer |
| 4 | int | a leaf: a signed integer |
| 5, 6, 7, 8 | u8, u16, u32, u64 | a leaf of exactly 1, 2, 4, 8 bytes |
| 9, 10, 11, 12 | i8, i16, i32, i64 | a leaf of exactly 1, 2, 4, 8 bytes |
| 13, 14 | f32, f64 | a leaf of exactly 4, 8 bytes |
| 16 | list | a composite of any children |
| 15, 17 to 31 | (kept) | refused in version 1 |
| 32 and above | (unnamed) | any leaf or any composite |

Entries 32 and above are where the core operations will be named. Naming one adds a lens row and
no rule, so no node written under version 1 becomes malformed.

A node whose blueprint is not a catalogue entry is any leaf or any composite.

## 5. Leaf rules

All integers and floats are little-endian.

- **nat**: the shortest little-endian bytes of the number. Zero is the empty string. The last
  byte is never `00`.
- **int**: the shortest little-endian two's-complement bytes. Zero is the empty string. Refused:
  a last byte of `00` when it is the only byte or the byte before it has its top bit clear; a last
  byte of `ff` when there is a byte before it with its top bit set.
- **u8 to i64**: exactly the stated width.
- **f32, f64**: IEEE 754 binary32 and binary64. Negative zero is its own value. The only NaN is
  the quiet one with a clear sign and an empty payload: `00 00 c0 7f` and
  `00 00 00 00 00 00 f8 7f`. Any other NaN is refused.
- **text**: UTF-8 with no overlong form, no surrogate (U+D800 to U+DFFF) and nothing above
  U+10FFFF. No normalisation is applied: two spellings of one visible character are two leaves.

## 6. The canonical print

Every node has exactly one printed form. It is ASCII.

```
node = "#" number                  catalogue entry
     | "^" number                  the node opened k-th in this print, counting from 0
     | "[" node [ " " hex ] "]"    leaf: blueprint, then bytes
     | "(" node *( " " node ) ")"  composite: blueprint, then children
number = "0" | a digit 1-9 followed by digits; at most 4294967295
hex    = one or more pairs of 0-9 a-f
```

Rules that make it canonical:

1. One space between items and no other white space. A file holds one node and one line feed.
2. A catalogue entry is always written `#N` and never as a leaf in brackets.
3. Each `[` and each `(` opens a node and gives it the next number, starting at 0. `#N` and `^k`
   open nothing.
4. A node that is not a catalogue entry is written in full the first time it appears, reading
   left to right, and as `^k` every time after, where k is the number it was given.
5. `^k` may name only a node whose closing bracket has already been read.
6. A leaf with no bytes is written with no space: `[#1]`.

A reader refuses text that breaks any of these, so reading and printing are inverse: a printed
node reads back to the same node, and accepted text prints back to the same text.

A reader goes through the text once, left to right, and stops at the first fault it meets. A
number is read to its last digit before it is used, so `^00` is outside the grammar whatever `^0`
would name. A node is checked when its closing bracket is read: first its bytes against section 7,
then rule 2, then rule 4. Text that leaves the grammar only after such a fault is refused for that
fault: `(#16 [#3 00] x)` is refused as `minimal`.

The print has no forms of its own beyond these four. Nobody writes in it except to bring the
first seed into being.

## 7. Refusals

A reader names the first rule broken. For a node's bytes that is the first in the order of this
table. The vectors use these words:

| word | meaning |
| --- | --- |
| `short` | fewer than 33 bytes |
| `kind` | first byte is not 00 or 01 |
| `nothing-blueprint` | blueprint is nothing and the node is not the root |
| `child-length` | a composite's bytes after the blueprint are not a multiple of 32 |
| `nothing-child` | a child id is nothing |
| `unknown` | the blueprint id or a child id is the id of no node the reader holds |
| `leaf-only` | a composite under the root or under entries 1 to 14 |
| `composite-only` | a leaf under entry 16 |
| `kept` | a node under entry 15 or 17 to 31 |
| `ordinal` | a leaf under the root that is not a nat from 1 to 4,294,967,295 |
| `width` | a fixed-width leaf of the wrong length |
| `minimal` | a nat or int that is not the shortest form |
| `utf8` | text that is not well-formed UTF-8 |
| `nan` | a NaN other than the one allowed |
| `syntax` | text outside the grammar of section 6 |
| `not-canonical` | text inside the grammar that breaks rule 2 or 4 |
| `forward` | `^k` naming a node not yet closed or never opened |

Sixteen of these depend only on the bytes or the text at hand. `unknown` depends on what the reader
holds: a reader that takes nodes in from bytes keeps each one it accepts, and can place a node
only under a blueprint and over children it already has. It holds the root and entries 1 to 31
from the start, and may hold more; the bootstrap starts with entries 0 to 82. A node read from
text is never `unknown`, because its blueprint and children are written before it closes.

## Choices

Made here, not given by the guide or measured. Each can be reopened; changing any changes ids.

- **SHA-256 as the content id.** Nobody asked for it. Any function that gives each node's
  canonical bytes one id would serve this page; which function, how many bits, and whether the id
  is a digest at all are questions 6.1.4, 6.1.18 and 6.1.31.
- **A kind byte and no length.** The hash covers the whole message, so a length adds nothing to
  identity. A store frames its records itself.
- **Child ids, not child bytes.** A node's bytes stay small whatever it holds, and a reader can
  check one node without its children.
- **Catalogue entries are numbers under one root.** The numeric space is the root; a symbol is a
  row beside it. The numbers are fixed by this page, not by arrival order.
- **Little-endian throughout.** Both first targets are little-endian and frozen nodes are read
  where they lie (5.7.1).
- **Both sized and unsized integers are seeded.** Which one arithmetic uses is question 6.1.11
  and is not decided here.
- **Sharing is written as `^k`.** Without it a node that names one child twice doubles its print
  at every level; the 2026-11-05 carrier measured that limit (3.13.2.11).
- **Text is not normalised.** Normalising would make the encoding depend on a Unicode version.

## Limits

- One reader holds the vectors: the bootstrap's, by the hand that wrote this page. The ids in the
  vectors were checked against the system's `shasum`.
- A second reader is owed: one written from this page alone, in the grammar, kept as nodes (graph
  nodes 7.2 and 7.13). One rule, the shortest form of a `nat`, runs as nodes in
  `ground/nat-rule.ck`; a hand that had read the bootstrap wrote it, so it shows that a rule can
  run as nodes and is no part of the second reader. A reader's rules and its print need nothing
  more than the walker has. The content id as section 3 writes it does: SHA-256 works on the bits
  of numbers, and the walker has no such operation.
- Definitions that refer to each other cannot be written as nodes yet (question 6.1.14).
- The core operations are not named. That belongs to stage 2, with the walker's laws.
