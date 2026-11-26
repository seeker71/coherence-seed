# The lens

A lens reads text into nodes and prints nodes back as text (graph hub 5.3). This is the first
one, and it is written in form: `lens/lens.ck` is its reader, its printer and its matcher,
`lens/words.ck` its words and `lens/rows.ck` its rows. The canonical print of
`spec/node-encoding.md` stays what is stored; the lens is how a stored node is read by a person
and how a new one is written.

It has three parts. Words and leaves turn text into nodes and back. Names stand for a recipe's
arguments and for the parts a `let` gives a place. Rows turn the surface a person writes into the
core the walker runs, and back.

## Words and leaves

```
node = "(" node { space node } ")"        a composite: its blueprint, then its children
     | "{" { name } "|" node "}"          a recipe, with a name for each of its arguments
     | "(" "let" { bind node } node ")"   parts given a name, then the body that uses them
     | "(" "if" { node node } node ")"    conditions and what each gives, then what is left
     | "(" node sign node ")"             an operation of two, written between them: (a + b)
     | "[" node [ space hex ] "]"         a leaf of any blueprint, its bytes in hex
     | '"' bytes '"'                      a text leaf; \" is a quote, \\ a backslash, \n a line end
     | [ "-" ] digits                     an int leaf
     | "0x" hex                           an int leaf, written in hexadecimal
     | digits "n"                         a nat leaf
     | "$" digits                         an argument by its number: $0 is (arg 0n)
     | name                               an argument by its name
     | name "." part                      a part of what a name of a known shape names: row.ints
     | "'" byte "'"                       the int of one byte: '(' is 40
     | "#" digits                         a catalogue entry by its number
     | word                               a catalogue entry by its word
bind = name [ ":" word ] | "$" digits        a name, and the shape of what it names
sign = "+" | "-" | "*" | "/" | "%" | "<" | "="     add sub mul div mod lt eq
space = one or more spaces, tabs or line ends
```

`(add 20 (mul 2 11))` is the node `(#46 [#4 14] (#48 [#4 02] [#4 0b]))`. A node written twice is
one node; nothing marks sharing, and nothing needs to.

Reading takes any of these forms. Showing chooses one for each node: the word when the entry has
one and `#N` when it has none, the number for an `int` of at most eight bytes, in hexadecimal
from 2 to the 28th up, and for a `nat` of at most seven, the name of an argument that has one and `$k` otherwise, braces for a recipe, the
quoted form for a text whose only control byte is the line end, the bracket form for every
other leaf, and the sign between its two children for an operation that has one. What fits in a hundred columns is shown on one line. So showing is total: every node
has a showing, and reading it gives back that node.

`lens/words.ck` binds `lens.words` to a list of rows, each a word and the entry it names. A word
is a row beside the node and no part of it: changing a word changes no id.

The signs are rows too: `lens/rows.ck` binds `lens.ops` to a list of them, each a sign and the
entry it stands for. `(n < 2)` is `(lt n 2)` and `((a + b) * c)` is `(mul (add a b) c)`. A sign
stands between exactly two, in brackets of its own, so no rule of precedence is needed and none
exists: `(a + b + c)` is refused.

## Names

An argument is a number in the node: `$0`, `$1`. A name is what a person calls it, and it is
kept beside the node (graph node 5.2.4).

- `{a b | (a + b)}` is the recipe `(recipe (add $0 $1))`. Inside the braces a is `$0` and b is
  `$1`. `{| x}` is a recipe whose arguments have no names.
- A recipe written inside another sees the outer names. `{x | (x + sum)}`, where sum is a name
  outside the braces, is the inner recipe closed over the outer arguments
  (`spec/core-operations.md`, entries 105 and 106): they come first among its arguments and x
  after them. It is closed only when it uses an outer name; `{x | (x * 2)}` in the same place
  is a plain recipe. A name of its own that repeats an outer one hides it, and so does writing
  an argument by its number: then the recipe is read by itself.
- `(let sum (add a b) twice (mul sum 2) (tuple sum twice))` gives two parts a name, each seen by
  what comes after it. It is two nodes of entry 100, one inside the other
  (`spec/core-operations.md`): a new name takes the next argument number, so sum is `$2` and
  twice `$3` when the recipe has two arguments. A name that is already there is given its place
  again: `(let n (add n 1) ..)` hides the old n inside the let and nowhere else.
- A name is any word that is not in the lexicon and does not begin like a number, `$`, `#`, `'`
  or `-`. Where a place has a name, its number is refused, so one text does not say one thing
  two ways.

The names of a recipe's arguments and the name a let gives are rows keyed by that node: a list
whose first item is the node's content id. A file of nodes holds them on a second line, after
the node's print. Reading a file learns its rows; storing writes the rows of the names met, in
the order the node holds them. A node is its first line alone: giving a name, or another name,
changes no id. Where a name cannot be read back as it stands, because it has become a word or
would land on another number, the number is shown.

## Shapes

A shape is an entry that is the blueprint of its instances, and it needs nothing new in the
core. `(bind emit.row {key body ints | (join emit.row (tuple key body ints))})` binds the entry
to a recipe that joins its arguments under it. The lens takes that recipe's names for the names
of the entry's parts.

A name may say what shape it names: `{mine:emit.row | ..}` or `(let mine:emit.row (child rows
k) ..)`. Then `mine.ints` is `(child mine 2)`: the part is found when the text is read, so
reading a part by its name costs nothing when the program runs, and the stored node holds no
names. Showing gives `mine.ints` back wherever a part of a name with a shape is read by a
number that the shape has a name for. The lexicon can name the parts of an entry as well: its
row for `let` is the word, the entry, and `place`, `value`, `body`, so `{x:let | x.value}`
reads the value of a let node. An entry that only names parts, with no meaning of its own, is
an interface: `emit.two` names `left` and `right` for any operation of two. The shape is written into the name as it is stored
beside the node (`mine:emit.row`), so the second line of the file carries it.

## Rows

A row is two shapes with holes: the surface a person writes and the core it stands for.
`lens/rows.ck` binds `lens.rows` to the list of them, written in this lens:

| surface | core |
| --- | --- |
| `(if c a b)` | `(choice (seq (cut (need c)) a) b)`; `(if c a d b e)` is `(if c a (if d b e))`, and so on for any number |
| `(first x)` | `(choice (cut x))` |
| `(when c x)` | `(seq (need c) x)` |
| `(not x)` | `(eq x no)` |
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
`odd-hex`, `too-large`, `number`, `char`, `unexpected`, `trailing`, `name` for a name that is a
word, is written twice or cannot be one, `named` for the number of a place that has a name, `let`
for a let with nothing to bind, `if` for an if of several conditions with nothing left at its end, `infix` for a sign between more than two, `row` for a row whose two shapes do not hold the same holes, and
`refused` when the encoding refuses the node that was written.

## Using it

```bash
./ck text lens/to-words.ck lens/all.ck < ground/check.ck
```

shows a stored node, with the names its file holds.

```bash
./ck text lens/to-nodes.ck lens/all.ck < words.txt > node.ck
```

reads what was written and gives what to store: the canonical print, and the names on a second
line when there are any. Write to a new file: the shell empties the file it writes before `ck`
reads anything. `lens/all.ck` evaluates the three files of the lens. Inside a program, after
`(step (read (file "lens/all.ck")))`, `(lens.read t)` answers the node a text leaf shows,
`(lens.show x)` the one-line text of x, `(lens.lower x)` and `(lens.raise x)` run the rows,
`(lens.stored raw)` is the node of a file's bytes with its names learned and `(lens.to-store x)`
the text of the file that stores x.

## How it is known

- The lens shows its own definition, 984 lines and 119 definitions, through its own rows and with its own names, and reads that
  text back to the file it is stored as, byte for byte, both lines. It was brought into being
  once by a substitution of numbers for words; since then its source is its own showing. Names
  were read by a second lens written in the first, and its own arguments were then named without
  one byte of its node changing.
- `ground/lens-round-trip.ck`, written in the lens, shows, reads and stores again every file of
  nodes in the repository: 121 files, none differs in either line, in 0.8 s.

## Limits

- It is not yet the surface of graph node 5.3.5. Every form is in brackets: a sign stands between
  two and no more, there is no precedence and no pattern over text.
- A part is read by name only from a name, not from an expression, and only where the shape's
  constructor was read before in the same reading. Nothing checks that what a name is given
  has the shape it says.
- A recipe closes over the outer names only where every outer argument has a name. In a recipe
  whose arguments have no names, or at the top of a program, the first name a let gives takes
  `$0`.
- Two recipes that are one node have one row of names. A name belongs to one lens; nothing yet
  keeps a second lens's names for the same node.
- A row's hole may stand only once on each side; a shape with the same hole twice is not checked.
- Every row is tried at every node. Words are found through cells, indexed once when the lens loads.
- The observer prints words from a table of its own in C, not from these rows, and shows no names.
- Which numbers a module's entries take is a convention of the lexicon: the engine's own
  definitions take package 16 (`spec/nums.md`). No row yet says which numbers a module may take.
