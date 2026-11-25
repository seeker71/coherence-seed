# The lens

A lens reads text into nodes and prints nodes back as text (graph hub 5.3). This is the first
one, and it is written in form: `lens/lens.ck` is its reader, its printer and its matcher,
`lens/words.ck` its words and `lens/rows.ck` its rows. The canonical print of
`spec/node-encoding.md` stays what is stored; the lens is how a stored node is read by a person
and how a new one is written.

It has two parts. Words and leaves turn text into nodes and back. Rows turn the surface a person
writes into the core the walker runs, and back.

## Words and leaves

```
node = "(" node { space node } ")"    a composite: its blueprint, then its children
     | "[" node [ space hex ] "]"     a leaf of any blueprint, its bytes in hex
     | '"' bytes '"'                  a text leaf; the bytes hold no '"' and no control byte
     | [ "-" ] digits                 an int leaf
     | digits "n"                     a nat leaf
     | "$" digits                     an argument: $0 is (arg 0n)
     | "'" byte "'"                   the int of one byte: '(' is 40
     | "#" digits                     a catalogue entry by its number
     | word                           a catalogue entry by its word
space = one or more spaces, tabs or line ends
```

`(add 20 (mul 2 11))` is the node `(#46 [#4 14] (#48 [#4 02] [#4 0b]))`. A node written twice is
one node; nothing marks sharing, and nothing needs to.

Reading takes any of these forms. Showing chooses one for each node: the word when the entry has
one and `#N` when it has none, the number for an `int` of at most eight bytes and a `nat` of at
most seven, `$k` for an argument, the quoted form for a text without `"` or control bytes, and the
bracket form for every other leaf. A composite that fits in a hundred columns is shown on one
line, a longer one with each child on a line of its own. So showing is total: every node has a
showing, and reading it gives back that node.

`lens/words.ck` binds `lens.words` to a list of rows, each a word and the entry it names. A word
is a row beside the node and no part of it: changing a word changes no id.

## Rows

A row is two shapes with holes: the surface a person writes and the core it stands for.
`lens/rows.ck` binds `lens.rows` to the list of them, written in this lens:

| surface | core |
| --- | --- |
| `(if c a b)` | `(choice (seq (cut (need c)) a) b)` |
| `(first x)` | `(choice (cut x))` |
| `(when c x)` | `(seq (need c) x)` |
| `(not x)` | `(eq x no)` |
| `(let v.. body)` | `(call (recipe body) v..)`, so inside body `$0` is the first v |
| `(def e body)` | `(bind e (recipe body))` |
| `(tuple a..)` | `(join list (all (choice a..)))`, a list of what a.. answer |

A hole is `(hole k)`; `(rest k)` holds every child from its place on, and a shape may have one.
One matcher runs a row both ways. Reading lowers: where a node has a row's surface shape, the core
shape is built from what the holes held, and those are lowered in turn. Showing raises, with the
two shapes changed over. The first row that fits is used, and what is under a `quote` is data and
is left as it is. No printer is written for a row: adding a row to the list is all it takes for
the lens to read it and to show it.

The surface words are catalogue entries. When the lens loads it checks that each row is two
shapes, and binds each surface word to lower itself where the walker meets it: a node that was
never lowered still runs, more slowly, and lowering ahead of time is a saving and not a need.
Without the lens loaded the walker answers such a node as data.

## Refusals

Text the lens cannot read is a fault, and its reason says what and where: a word and the place as
a count of bytes from the start. `(add 1 (mull 2 3))` faults with
`(list "unknown-word" (list 8 "mull"))`. The words are `unknown-word`, `unclosed`, `empty`,
`odd-hex`, `too-large`, `number`, `char`, `unexpected`, `trailing`, `row` for a row whose two
shapes do not hold the same holes, and `refused` when the encoding refuses the node that was
written.

## Using it

```bash
./ck text lens/to-words.ck lens/all.ck < ground/check.ck
```

shows a stored node.

```bash
./ck text lens/to-nodes.ck lens/all.ck < words.txt > node.ck
```

reads what was written and gives the canonical print to store. Write to a new file: the shell
empties the file it writes before `ck` reads anything. `lens/all.ck` evaluates the three files of
the lens. Inside a program, after `(step (read (file "lens/all.ck")))`, `(lens.read t)` answers the
node a text leaf shows, `(lens.show x)` the one-line text of x, and `(lens.lower x)` and
`(lens.raise x)` run the rows.

## How it is known

- The lens shows its own definition, through its own rows, and reads that text back to the node
  it is stored as, byte for byte. It was brought into being once by a substitution of numbers for
  words; since then its source is its own showing, and the rows and their matcher were written in
  it.
- `ground/lens-round-trip.ck`, written in the lens, raises, shows, reads and lowers every file of
  nodes in the repository: 59 files, none differs, 31 million walker steps in 0.2 s.

## Limits

- It is not yet the surface of graph node 5.3.5. Shapes are brackets only: there is no operator
  table and no pattern over text, so nothing is written infix. A recipe's arguments are numbered,
  not named. Nothing names a part to use it twice, so a node with much sharing shows long.
- A row's hole may stand only once on each side; a shape with the same hole twice is not checked.
- Every row is tried at every node. Words are found through cells, indexed once when the lens loads.
- The observer prints words from a table of its own in C, not from these rows.
- Entries 129 to 185 and 194 to 196 are the lens's and 186 to 193 the ice's by convention; no row yet says which numbers a
  module may take.
- Most stored programs were composed before the lens existed. They show through it; they were
  not rewritten in it.
