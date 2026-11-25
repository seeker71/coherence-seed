# The word lens

A lens reads text into nodes and prints nodes back as text (graph hub 5.3). This is the first
one. It puts words beside the numbers of the catalogue and writes the common leaves as people
write them, and it is itself written in form: `lens/lens.ck` is its reader and its printer,
`lens/words.ck` its words. The canonical print of `spec/node-encoding.md` stays what is stored;
the lens is how a stored node is read by a person and how a new one is written.

## The notation

```
node = "(" node { space node } ")"    a composite: its blueprint, then its children
     | "[" node [ space hex ] "]"     a leaf of any blueprint, its bytes in hex
     | '"' bytes '"'                  a text leaf; the bytes hold no '"' and no control byte
     | [ "-" ] digits                 an int leaf
     | digits "n"                     a nat leaf
     | "#" digits                     a catalogue entry by its number
     | word                           a catalogue entry by its word
space = one or more spaces, tabs or line ends
```

`(add 20 (mul 2 11))` is the node `(#46 [#4 14] (#48 [#4 02] [#4 0b]))`. A node written twice is
one node; nothing marks sharing, and nothing needs to.

Reading takes any of these forms. Showing chooses one for each node, on one line with single
spaces: the word when the entry has one and `#N` when it has none, the number for an `int` of at
most eight bytes and a `nat` of at most seven, the quoted form for a text without `"` or control
bytes, and the bracket form for every other leaf. So showing is total: every node has a showing,
and reading it gives back that node.

Text the lens cannot read is a fault, and its reason says what and where: a word and the place as
a count of bytes from the start. `(add 1 (mull 2 3))` faults with
`(list "unknown-word" (list 8 "mull"))`. The words are `unknown-word`, `unclosed`, `empty`,
`odd-hex`, `too-large`, `number`, `unexpected`, `trailing`, and `refused` when the encoding
refuses the node that was written.

## The words

`lens/words.ck` binds entry 131, `lens.words`, to a recipe that answers a list of rows, each a
word and the entry it names. An entry's word is a row beside the node and no part of it: changing
a word changes no id. Entries 129 to 161 are the lens's own, named `lens.read`, `lens.show` and
so on; they are bound together as one layer (`spec/core-operations.md`, entry 86), so they call
each other by name.

## Using it

```bash
./ck text lens/to-words.ck lens/words.ck lens/lens.ck < ground/check.ck
```

shows a stored node in words.

```bash
./ck text lens/to-nodes.ck lens/words.ck lens/lens.ck < words.txt > node.ck
```

reads words and writes the canonical print to store. Inside a program, `(lens.read t)` answers the
node a text leaf shows and `(lens.show x)` the text leaf that shows x, once the two files have
been evaluated.

## How it is known

- The lens shows its own definition and reads that text back to the node it is stored as, byte
  for byte. It was brought into being once by a substitution of numbers for words; since then its
  source is its own showing.
- `ground/lens-round-trip.ck`, the first program written in words, does the same for every file
  of nodes in the repository: 50 files, none differs, 14 million walker steps in 0.08 s.

## Limits

- It is words and leaves, not yet the home lens of graph node 5.3.5: there are no rule rows of
  pattern and template, no operator table, no names for a recipe's arguments, and no way to name
  a part and use it twice, so a node with much sharing shows long.
- It shows on one line.
- A word is looked up by reading the rows in order.
- The observer prints the same notation from a table of its own in C, not from these rows.
- `lens/lens.ck` binds 32 entries by number; no row yet says in which module a number is free.
