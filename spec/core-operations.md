# Core operations, floor version

What the bootstrap's walker runs. Each operation is a catalogue entry (`spec/node-encoding.md`,
section 4); a **composite** whose blueprint is one of these entries is executed, and every other
node answers itself. `bootstrap/ck.c` is the walker and `ground/` holds the programs that witness
it. This page names 39 entries: 32 to 52 are operations and data, 53 to 63 are the kinds of the
floor's own records, and 64 to 70 take a node apart and build one. It does not yet name operations
that hold an offer or bind a cell; those come with the stage that first needs them.

## Outcomes

Evaluating a node is an offer. It ends in exactly one of:

- **a node**: the answer. `#33` is yes with nothing to carry and `#32` is an answer of no. Both
  are ordinary answers: neither moves a choice on.
- **nothing**, with a reason beside it: no answer stands. Either every path failed and was
  undone, or an operation had no answer to give (for example a sum outside the machine's range).
- **a fault**, with a reason: the program broke a promise of the system, such as a wrong number of
  children or arithmetic on something that is not a number. A fault unwinds to the nearest
  `recover`, or ends the run.

A reason is a text leaf. It is exempt from undo.

## Entries

| N | name | children | what it does |
| --- | --- | --- | --- |
| 32 | no | | data: the answer of no |
| 33 | yes | | data: yes with nothing to carry |
| 34 | quote | x | answers x itself, unevaluated |
| 35 | arg | k (a nat leaf) | answers argument k of the running recipe, counting from 0 |
| 36 | recipe | body | data: a recipe. Its body sees only its own arguments |
| 37 | call | f, arguments | evaluates f and the arguments once, left to right, then runs f's body |
| 38 | self | | answers the recipe now running |
| 39 | seq | steps | evaluates each in order and answers with the last; with no steps, yes |
| 40 | choice | options | runs the first option; a later fail returns here and runs the next |
| 41 | fail | optional reason | turns execution around (below) |
| 42 | cut | optional x | evaluates x, then commits (below), and answers with x's answer |
| 43 | all | x | answers a list of every answer x gives, in order |
| 44 | need | x | answers x's answer, but fails if it is no or nothing |
| 45 | recover | body, handler | answers with body's answer; on a fault inside it, calls handler with the reason |
| 46, 47, 48 | add, sub, mul | a, b | integer arithmetic on `int` leaves |
| 49 | eq | a, b | yes if a and b are the same node, otherwise no |
| 50 | lt | a, b | yes if integer a is less than integer b, otherwise no |
| 51 | native | code, arguments | runs a machine-code leaf with up to six integer arguments |
| 52 | code-aarch64 | | data: the blueprint of a leaf of AArch64 machine code |

## Entries 53 to 63: the floor's own records

Everything the floor keeps in memory is a record whose first word is one of these entries
(`spec/framebuffer.md`). They are data: a composite under one of them answers itself.

| N | name | a record of this kind is |
| --- | --- | --- |
| 53 | framebuffer | the head of the shared region |
| 54 | area | one table of the region: what its records are, how many, where |
| 55 | machine | the walker's registers |
| 56 | node | the kind of the node table; each record in it starts with that node's own blueprint |
| 57 | frame | a continuation: what to do with the answer now being computed |
| 58 | link | one gathered value and the link before it |
| 59 | activation | a running recipe and where its arguments are |
| 60 | choice-point | an open choice, its next option and the marks it returns to |
| 61 | gathered | one answer collected by `all` |
| 62 | words | a table of 32-bit words: handles or counts |
| 63 | bytes | a table of bytes |

The table of code leaves that have a runnable copy uses entry 52.

## Entries 64 to 70: a node's parts

With these a program reaches everything `spec/node-encoding.md` says a node is: its blueprint,
whether it is a leaf, and its bytes or its children. Each evaluates its children first, in order.
A number here is an `int` leaf; positions count from 0.

| N | name | children | what it does |
| --- | --- | --- | --- |
| 64 | blueprint | x | answers x's blueprint; for the root, nothing with the reason `root` |
| 65 | count | x | how many bytes a leaf has, or how many children a composite has |
| 66 | child | x, k | child k of the composite x |
| 67 | byte | x, k | byte k of the leaf x, as a number from 0 to 255 |
| 68 | leaf | x | yes if x is a leaf, no if it is a composite |
| 69 | pack | b, l | the leaf with blueprint b whose bytes are the numbers in the list l |
| 70 | join | b, l | the composite with blueprint b whose children are the children of the list l |

- `child` and `byte` answer nothing with the reason `range` when k is below 0 or not below the
  count. So does `pack` when a number in the list is outside 0 to 255.
- `pack` and `join` make the one node with that content: building a node that exists answers that
  same node, and `pack` under the root answers the catalogue entry of that number.
- A build the encoding refuses answers nothing, and the reason is the refusal's word from section 7
  of `spec/node-encoding.md`: `(pack #3 l)` with l holding 1 and 0 answers nothing with `minimal`.
- A node a program builds is stamped with the `pack` or `join` node that built it, as every record
  is with its writer (`spec/framebuffer.md`).
- An argument that is nothing answers nothing with the reason `nothing-in`. `child` on a leaf,
  `byte` on a composite, a position that is not an `int`, and an l that is not a list are the
  fault `kind`.

A list is built with `all`. The recipe below answers byte i of x and, when the program fails back
into it, each byte after it; `(all (call it x 0))` is the list of x's bytes. It is shown with the
names the observer prints; on file it is the canonical print:

```
(recipe (seq (need (lt (arg 1) (count (arg 0))))
             (choice (byte (arg 0) (arg 1))
                     (call (self) (arg 0) (add (arg 1) 1)))))
```

`ground/rebuild.ck` takes a nested node apart to its bytes with two such recipes and builds it
again; the answer is the node it was given. `ground/nat-rule.ck` is the first rule of the encoding
kept as nodes: given candidate bytes it answers the `nat` leaf they are, or the word `minimal`.

## Fail, choice and cut

The owner's definition (graph node 5.4.2): fail means backtrack, undo and reverse all changes
until a new choice is tried.

- `choice` remembers where it stands before running an option.
- `fail` returns to the most recent remembered choice that still has an option, drops everything
  made since, and runs that choice's next option from the state the choice was entered with.
- A choice stays open after it has answered. A fail later in the program re-enters it. This is
  what lets `all` collect every answer.
- `cut` closes the nearest enclosing `choice` in the same recipe body, together with every choice
  opened since it was entered. With no enclosing choice it closes the choices opened since the
  running recipe was called. `all` is a barrier: a cut inside it does not reach outside it.
- When no remembered choice is left, the run answers nothing.

Read with these: `if c then a else b` is `(choice (seq (cut (need c)) a) b)`, and "the first
answer only" is `(choice (cut x))`.

## Arithmetic

`add`, `sub`, `mul` and `lt` take `int` leaves. The floor computes in 64 bits. A result outside
that range answers nothing with the reason `range`: the bound is the machine's, not the
language's. An argument that is nothing answers nothing. An argument of another kind is a fault.

## Native code

A leaf under entry 52 holds AArch64 instructions for a function that takes up to six 64-bit
integers in x0 to x5 and returns one in x0. `native` copies it once into the process's code
region and calls it. On a host that cannot run it, `native` answers nothing with the reason
`no-native`.

## Faults the walker names

`arity` (wrong number of children), `arg` (no such argument), `no-recipe` (`self` outside a
recipe), `not-a-recipe` (calling something else), `kind` (an operand of the wrong kind).

## Limits

- The floor has no cell a program can change in place, so "undo" here is dropping what an option
  made. Journaled cells and their trail belong to stage 5.
- A call in the last position of a recipe keeps no frame. The activations and gathered values of
  calls that have returned are given back as soon as no open choice can return to them.
- A recipe cannot name another recipe that names it back (question 6.1.14); `self` covers direct
  recursion only.
- Recipes do not capture the arguments of an enclosing recipe.
- Integer law is still question 6.1.11; the 64-bit bound is the floor's.
- The root cannot be built: its blueprint is nothing, and `pack` needs a blueprint.
- An operation that evaluates its children first takes at most eight, so a call passes at most
  seven arguments.
- Programs are written by hand in the canonical print (`./ck canon` numbers the repeats). There is
  no operation on the bits of a number, which a content id needs.
