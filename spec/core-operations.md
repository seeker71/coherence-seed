# Core operations, floor version

What the bootstrap's walker runs. Each operation is a catalogue entry (`spec/node-encoding.md`,
section 4); a **composite** whose blueprint is one of these entries is executed, and every other
node answers itself. `bootstrap/ck.c` is the walker and `ground/` holds the programs that witness
it. This page names 51 entries: 32 to 52 are operations and data, 53 to 63 and 72 are the kinds of
the floor's own records, 64 to 70 take a node apart and build one, 71 to 75 let a program replace
what runs, 76 to 80 are what the bootstrap does for itself, 81 is the walker's step, and 82 is
the first operation that exists only in form. It does
not yet name operations that hold an offer or bind a cell; those come with the stage that first
needs them.

The bootstrap's C is one implementation of each operation, the one a cold start runs. Any
operation, and any recipe, can be given another while a program runs (entry 71): to add a
diagnostic, to try a faster or a gentler version, to run it elsewhere, and to go back.

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
| 72 | binding | one entry and the implementation that runs it |

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

## Entry 71: an operation's implementation

`(bind e i)` gives e the implementation i and answers yes. From then on i runs wherever e would
have.

- **e is any operation, an entry with no meaning yet, or a recipe.** Control operations included.
  A bound recipe is replaced wherever it is called, so one recipe can be swapped for a tracked or
  a compiled version and back. Only data entries keep their meaning.
- **i says how it wants to be used.** A `recipe` is called with the children's answers. A `lazy`
  (entry 75, shaped like a recipe) is given the node itself, unevaluated, and so can stand for a
  control operation. A code leaf is called with the answers as integers. It may decline, and it
  is not asked at all when an answer is not an integer; then what lies beneath it runs, so code
  takes the cases it is sure of and leaves the rest. A recipe's call that its code declined runs
  wholly on the bindings beneath that code. A
  leaf of any other blueprint is run by whatever that blueprint is bound to, which is how a new
  kind of implementation, another processor or a table, comes in without a change to the bootstrap.
- **`(up x)` and `(base x)`** (entries 73, 74) evaluate the node x where the user of the
  implementation stands, with its arguments and in its cut scope, from however deep inside the
  implementation they are used. Under `base` the node's own entry means what it meant
  beneath this binding, and everything inside it is as the user sees it: a `lazy` that records
  something and ends in `(base (arg 0))` adds a diagnostic to every use of an operation.
- **An implementation runs on the bindings beneath its own.** Inside i, and inside whatever i
  calls, e means what it meant before this binding. So a new `add` can use the `add` it replaces,
  and `add` and `sub` can each be written with the other.
- **A binding is undone like anything else.** A fail that returns to a choice entered before the
  binding takes the binding away.
- **A binding is a record in the region** (`spec/framebuffer.md`): the entry, the implementation,
  and the `bind` node that wrote it. The observer lists them, newest first: what runs, and for
  what.
- `./ck run F B..` evaluates each B first and leaves its bindings in place for F.

### The operations in form

Each file under `form/` is one `bind` node.

| entry | in form | as AArch64 code |
| --- | --- | --- |
| 46 add | `form/add.ck`: a minus (0 minus b) | `form/add.aarch64.ck` |
| 47 sub | `form/sub.ck`: a plus (b times -1) | `form/sub.aarch64.ck` |
| 48 mul | `form/mul.ck`: a added b times, by recursion | `form/mul.aarch64.ck` |
| 50 lt | `form/lt.ck`: when the signs differ, whether a is negative; otherwise whether a minus b is | `form/lt.aarch64.ck` |

`lt` in form reads the sign from the encoding: a byte from 128 up is an `int` leaf of two bytes, so
a number is negative when `(count (byte d last))` is 2. Each code version is one leaf bound
directly: it computes, and says in its second result whether the answer is a number, a yes or no,
or out of range.

Every operation can be replaced; the others are not yet written in form. `ground/` shows the
kinds of swap: a diagnostic on `choice` (`traced-choice.ck`), one recipe tracked on every call and
then released (`tracked-recipe.ck`), a control operation brought into being (`invented-control.ck`),
a `cut` handed through a helper recipe to the program that used it (`cut-through.ck`), and a new
kind of implementation (`new-kind.ck`).

## Entries 76 to 80: what the bootstrap does for itself

What the commands of `ck` do is also an operation, so a program can do it, and it can be replaced
like any other.

| N | name | children | what it does |
| --- | --- | --- | --- |
| 76 | read | t | the node whose canonical print is the bytes of leaf t, with or without the one line feed a print file ends in; a refused print answers nothing with the refusal's word |
| 77 | print | x | a text leaf holding x's canonical print |
| 78 | id | x | a bytes leaf holding x's content id |
| 79 | file | p | a bytes leaf holding the file whose path is the bytes of leaf p; nothing with `no-file` when it cannot be read |
| 80 | clock | | nanoseconds on a clock that only moves forward |

`ground/runs-a-file.ck` reads a program from a file, reads its print into a node and runs it: what
`ck run` does, written in form. `ground/keeps-the-faster.ck` times one workload on `add` in form
and on `add` as machine code, and binds the one that took less.

## Entry 81: the step

| N | name | children | what it does |
| --- | --- | --- | --- |
| 81 | step | x | evaluates x, then evaluates the node x answered, where this node stands |

Evaluating one node is the walker's step, and the step is an entry like the others. Used, it lets
a program run a node it holds. Bound, its implementation is given **every** node that is about to
be evaluated, leaves and data included, before the walker looks at it. An evaluator that traces,
that compiles, or that runs elsewhere is then one more implementation in the same table.

- **The newest binding that applies to a node runs it**: the one for its blueprint, or the step's.
  A step bound after an operation's binding is asked first for that operation's nodes.
- **A `lazy` or a `recipe` gets the node as its one argument.** `(base (arg 0))` hands it to what
  lies beneath: an earlier step, a binding of its blueprint, or the bootstrap's walker. The node's
  children are evaluated as the user sees them, so the step meets each of them in turn.
- **A code leaf gets the node's handle in x0 and the number of the activation it stands in in
  x1**, and reads what it needs from the region, whose address every call of code carries in x6
  (`spec/framebuffer.md`). It answers as any code does, or declines.
- **The implementation itself runs on the bindings beneath it**, so it is not given its own nodes.

In `ground/step-in-form.ck` a step reads every 2 as 3, inside the recipes the program calls too,
so the recursion answers 32; a fail takes the step away and it answers 55 again. In
`ground/step-in-code.ck` a step in form makes every leaf answer the
word "form", and a step of ten instructions laid over it reads each node's record in the
region, answers the leaves itself and declines the rest to the step beneath.
`form/step.traced.ck`, which leaves a mark for every node evaluated and changes no answer, and
`form/step.leaf.aarch64.ck` are bindings for `ck run`.

## Entry 82: compile, which exists only in form

| N | name | children | what it does |
| --- | --- | --- | --- |
| 82 | compile | r | writes machine code for the recipe r, binds r to it and answers the code leaf; answers no, and binds nothing, when it cannot |

The bootstrap has no code for this entry. `form/compile.aarch64.ck` binds it to an emitter written
in form: one pass over the recipe's body, each node's instructions gathered as bytes by `all` and
packed into one leaf under entry 52. It knows seven shapes of node:

- an `int` leaf from 0 to 65,535, and `arg`;
- `add`, `sub` and `mul`, with overflow collected in one register;
- `(choice (seq (cut (need c)) a) b)` where c is an `lt` or an `eq` of numbers, as a compare and
  two branches. An overflow inside c takes the second branch, as on the walker;
- a call of `self`, as a machine call of the leaf's own first instruction.

For any other node the emitter emits a byte that `pack` refuses, so a recipe is compiled whole or
not at all, and what is not compiled stays on the walker.

**The leaf answers a number or declines.** It declines when an overflow is left at its end, when
it was given fewer arguments than the recipe reads, and when the machine stack, whose end it reads
from the region, has no room for its frame. A decline travels up through the calls above it, and
the walker then runs the whole call. So wherever emitted code cannot be sure, the answer, the
reason and the fault are the walker's own: emitted code is faster or it is the walker, never
different. It keeps its arguments and the registers it uses in a frame on the machine stack, and
a node's result in a register chosen by its depth, x19 to x25.

`ground/compiles-a-recipe.ck` loads the emitter from its file, runs a recursion on the walker,
compiles it, runs a much longer one on the emitted leaf, and asks for a recipe the emitter does
not know.

## Fail, choice and cut

The guide's definition (graph node 5.4.2): fail means backtrack, undo and reverse all changes
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
integers in x0 to x5, finds the region's address in x6 and how many values it was given in x7,
and answers in x0 and x1. x1 says what x0 is: 0 a number, 1 a truth (yes unless x0 is zero), 4
the handle of a node in the region, 3 nothing at all: the code declines and what lies beneath its
binding runs. Anything else is no answer, which is nothing with the
reason `range`. So one leaf can be a whole operation, or the part of one it does well. `native`
copies it once into the process's code region and calls it; code that declines there answers
nothing with the reason `declined`. On a host that cannot run it, `native` answers nothing with
the reason `no-native`.

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
  no operation on the bits of a number.
- A replaced step decides what each node means, and the bootstrap's walker still carries the run
  between nodes: the continuation, the choice points and the undo are its own and have no binding.
  The one step in machine code is written by hand and takes only the leaves.
- The emitter compiles numbers only: no call of another recipe, no control beyond the one shape
  of condition, no answer of yes or no, no negative or large literal, and an expression nested
  deeper than seven registers is not compiled. It cannot hand a node it does not know back to the
  walker. A program asks for it; nothing offers a recipe by how often it runs.
- Where emitted code declines, the work is done twice: once until it declines, then on the
  walker. It goes on computing while it unwinds, so a recursion that branches can spend long on
  answers it will discard.
- Looking up what runs a node reads the bindings from the newest down, and finding a leaf's
  runnable copy reads the born code the same way: both grow with how many there are.
- What is not an operation has no binding yet: the store, freeze and thaw and the other host
  functions. The commands of `ck` still call the C directly.
- `sub` in form answers nothing when b is the least integer, where the bootstrap's `sub` may have
  an answer. `mul` in form takes one call for each unit of b. A form operation that meets nothing
  inside may name another reason than the bootstrap's.
- A binding has no name of its own and cannot be taken away except by a fail.
- Nothing bounds a run but the region's capacities: a wrong recipe under a recursion gives no
  answer and goes on until an area is full.
