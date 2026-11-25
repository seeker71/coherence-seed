# Core operations, floor version

What the bootstrap's walker runs. Each operation is a catalogue entry (`spec/node-encoding.md`,
section 4); a **composite** whose blueprint is one of these entries is executed, and every other
node answers itself. `bootstrap/ck.c` is the walker and `ground/` holds the programs that witness
it.

This page names 82 entries. 32 to 52 are operations and data, and 53 to 63 with 72 the kinds of
the floor's own records. 64 to 70 take a node apart and build one; 71 to 75 let a program replace
what runs; 76 to 80 are what the bootstrap does for itself. 81 is the walker's step and 82
observes an outcome. 83 and 84 divide, 85 is a program's own fault and 86 makes bindings one
layer. 87 and 88 are the encoding itself; 89 to 93 are cells, their trail, notes and unbinding;
94 to 97 read a region; 98 calls the host, 99 counts, 100 gives a part a place, 101 binds from
outside the run, 102 says what runs an entry, 103 holds a call back until a search has settled
104 answers nothing with a reason, 105 with 106 close a recipe over values, 107 to 111 work on
the bits of an integer, and 112 gives a binding a faster form. 128 is the first operation that
exists only in form: entries below 128 are the bootstrap's to name, and from 128 up they are
named by modules written in form.

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

A reason is a node, most often a text leaf. It is exempt from undo. `(fault r)`, entry 85, is a
program's own fault: it breaks off with the reason r exactly as the walker's faults do, through
every `all` and `choice` on the way to the nearest `recover` or `outcome`.

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


`(range a b)`, entry 99, is a choice whose options are the integers from a up to b: it answers
a, and each fail that returns to it gives the next, until the one before b. It keeps one choice
point and no activation however long it runs, so counting through a list or a leaf, the
commonest loop, is `(child x (range 0 (count x)))` and no recursion.
## Arithmetic

`add`, `sub`, `mul` and `lt` take `int` leaves. The floor computes in 64 bits. A result outside
that range answers nothing with the reason `range`: the bound is the machine's, not the
language's. An argument that is nothing answers nothing. An argument of another kind is a fault.

| N | name | children | what it does |
| --- | --- | --- | --- |
| 83 | div | a, b | the quotient of a by b, rounded toward zero |
| 84 | mod | a, b | what is left of a: a minus b times that quotient, so it has a's sign |

Dividing by zero answers nothing with the reason `zero`.

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
recipe), `not-a-recipe` (calling something else), `kind` (an operand of the wrong kind),
`irreversible` (a call of the host while a choice is open), and `crash`: the processor faulted
inside a code leaf or a host function a program called. A bad address, an instruction that is
none, a recursion in machine code that runs off its stack: each is a fault like the others,
caught by `recover` and seen by `outcome`, and the run goes on (`ground/crash-is-a-fault.ck`). A
fault of the processor anywhere else, in the bootstrap's own code, ends the process.

`full` is the fault of an area of the region that has no room left; its reason is
`(list "full" name)`, the area's name beside it. A recursion that never returns ends this way
and not with the process. Where a `recover` catches a fault, the frames, the gathered values and
the activations of the body that faulted are given back, so a program that ran out of frames goes
on (`ground/full-is-a-fault.ck` does it twice). Nodes are not given back: a store that is full
stays full, and what catches that fault can do little.

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
| 62 | word-table | a table of 32-bit words: handles or counts |
| 63 | byte-table | a table of bytes |
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
  A `lazy` bound while a node's children are already being evaluated leaves that node to what
  lies beneath it: it takes nodes, and there are only answers (`ground/bound-in-flight.ck`).
- **`(step x)`** evaluates the node x where the step stands, with that place's arguments,
  whoever computed x (`ground/step-stands-here.ck`).
- **`(up x)` and `(base x)`** (entries 73, 74) evaluate the node x where the user of the
  implementation stands, with its arguments and in its cut scope, from however deep inside the
  implementation they are used. The user is whoever stands at the node a `lazy` or a step was
  given; a recipe called with answers has no user of its own and passes on its caller's
  (`ground/user-kept.ck`). Under `base` the node's own entry means what it meant
  beneath this binding, and everything inside it is as the user sees it: a `lazy` that records
  something and ends in `(base (arg 0))` adds a diagnostic to every use of an operation.
- **An implementation runs on the bindings beneath its own.** Inside i, and inside whatever i
  calls, e means what it meant before this binding. So a new `add` can use the `add` it replaces,
  and `add` and `sub` can each be written with the other.
- **Bindings made together are one layer.** `(layer x)`, entry 86, evaluates x, and every binding
  x made then runs on all of them: its implementation sees itself and the others by name, with
  what was bound before the layer beneath. This is how a set of definitions that call each other
  is written; a single `bind` outside a layer keeps meaning what it meant before.
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

The whole of `spec/node-encoding.md` is written in form as well. `form/read.ck` and
`form/print.ck` are entries 76 and 77, the reader and the printer of the canonical print: the
reader goes through the text once, numbers each node it opens, checks each leaf's bytes itself
and names the first rule broken; the printer numbers what it opens and writes `^k` for what it
has closed. `form/id.ck` is entries 78 and 87: SHA-256 written over the bits of integers, the
canonical bytes of a node, and the id as the one of the other, kept once computed.
`form/decode.ck` is entry 88, the reader of bytes: it holds every node whose id it has computed
and names the first rule a node's bytes break. With all five bound, the 142 vectors hold, ids
among them; with the reader and the printer bound, the lens shows and reads every file here and
every ground program ends as it does on the bootstrap's own (`ground/encoding-in-form.ck`).

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
| 76 | read | t | the node whose canonical print is the first line of leaf t: what follows a line feed is not read; a refused print answers nothing with the refusal's word |
| 77 | print | x | a text leaf holding x's canonical print |
| 78 | id | x | a bytes leaf holding x's content id |
| 79 | file | p | a bytes leaf holding the file whose path is the bytes of leaf p; nothing with `no-file` when it cannot be read |
| 80 | clock | | nanoseconds on a clock that only moves forward |

Text comes in through `file`, which reads the standard input under its usual path, and goes out
at the end of a run: `./ck text F` runs F as `./ck run F` does and writes an answer that is a
leaf as its own bytes. Nothing is written while a program runs, so nothing written needs undoing.

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

## Entry 82: an outcome, observed

| N | name | children | what it does |
| --- | --- | --- | --- |
| 82 | outcome | x | evaluates x to its first outcome and answers it as a list of a word and a node |

The answer is `node` and x's answer, `nothing` and the reason, or `fault` and the reason: what
`ck run` prints, as a node a program can hold and compare. Everything x did is undone as by a
fail, a binding it made included; only the outcome and the nodes it names remain. A `cut` inside
x does not reach outside it, and a `recover` inside x still catches first.

So a program can run another and judge how it ended. `ground/check.ck` does that for every
program under `ground/`: it reads `ground/expect.ck`, a list of each program's path and expected
outcome, runs each one, and answers the list of those that ended otherwise, which is empty when
all is well. `ground/observe.ck` answers the same list filled with what the programs do now: the
node to store when an expected outcome is meant to change.

## Entries 87 and 88: the encoding as operations

| N | name | children | what it does |
| --- | --- | --- | --- |
| 87 | encode | x | a bytes leaf holding x's canonical bytes (`spec/node-encoding.md`, section 2) |
| 88 | decode | b | the node the bytes of leaf b are; refused bytes answer nothing with the refusal's word |

With `read`, `print` and `id` these are the whole of what the encoding's page fixes, so what the
bootstrap once did as commands is written in form:

- **The conformance check.** `ground/vectors.ck` reads the vectors, a list in
  `spec/node-vectors.ck`, and answers those the reader does not hold: for a good vector that its
  print reads to those bytes and that id, prints back unchanged and decodes to the same node; for
  a bad one that it is refused with the word the page names. It faults if it finds no vectors.
- **Ice.** `ice/ice.ck` binds `ice.freeze`, which answers a bytes leaf holding a node and
  everything under it, each node once and children before parents, every record four bytes of
  length and the canonical bytes; and `ice.thaw`, which decodes such bytes record by record and
  answers the last node. `ice/freeze.ck` and `ice/thaw.ck` run them on standard input. A file
  frozen by one process thaws in another to the same node.

`ck` itself has two commands, `run` and `text`. Everything else is a program they run.

## Entries 89 to 91: cells and their trail

| N | name | children | what it does |
| --- | --- | --- | --- |
| 89 | hold | k, v | the cell k now holds the node v; answers v |
| 90 | held | k | the node the cell k holds; nothing with the reason `empty` when it holds none |
| 91 | trail | | data: the kind of a trail record |

A cell is where a program keeps something it will change. Any node is a cell, and what tells one
use from another is the shape of the node: `(lens.find "add")`, built with `join`, is the cell
the lens keeps the entry for that word in. Holding costs one write.

**A change to a cell is undone like everything else.** While a choice is open, what a cell held
before each change goes on the trail, and a fail that returns to that choice puts it back; so do
the end of an `all` and of an `outcome`. A program that wants to try something, look at the
result and leave no trace wraps it in `outcome`. With no choice open there is nothing to return
to and nothing is kept on the trail.

What follows from that: a table built inside an option is gone when the option fails. A table
meant to last is built before the choices that will use it, as the lens indexes its words when
it loads. `ground/cells.ck` runs each kind of undo.

## Entries 92 and 93: what is seen of a run, and a binding released

| N | name | children | what it does |
| --- | --- | --- | --- |
| 92 | note | k, v | the cell k now holds v, and no fail undoes it; answers v |
| 93 | unbind | e | takes away e's newest binding and answers yes, or answers no when it has none |

A note is for what is observed about a run and is no part of it: a count of calls is true
whether or not the path that made them was kept. `held` reads it like any cell. `unbind` uncovers
the binding beneath, or the bootstrap's own code, and a fail restores what it took away. Mind
that `all`, and so `(tuple ..)`, fails between its answers: a binding or an unbinding made inside
one of them is gone before the next.

With these a run can watch itself and change what runs it. `form/heat.ck` binds `heat.watch`,
which puts a `lazy` on `call`: each call is counted in a note under its recipe, and at the
hundredth the watcher steps off `call`, asks the emitter to compile that recipe, and takes its
place again above the code that was bound. After ten thousand calls that bring nothing new it
steps off for good. `ground/warms-itself.ck` runs three recursions under it: the long one starts
on the walker and ends on machine code, a second is compiled the same way, and a third the
emitter does not know stays on the walker until the watcher rests. The answers are the
walker's, in 1.6 million walker steps where the first recursion alone takes 158.6 million
there. The observer shows the bindings afterwards: watched, released, compiled, watched again.

## Entries 94 to 97: the region, read by a program

| N | name | children | what it does |
| --- | --- | --- | --- |
| 94 | peek | o, n | a bytes leaf holding n bytes of the region from byte o; nothing with `range` outside it or on the page kept under the stack |
| 95 | handle | x | the number of x's place in the node table |
| 96 | at | h | the node at that place; nothing with `range` when there is none |
| 97 | look | name, o, n | n bytes from byte o of the region another process made under that name, mapped to be read only; nothing with `no-region` when there is none |

Diagnostics is reading the region, and these let a program do it for its own. What the bytes
mean is in `region/layout.ck`, a node (`spec/framebuffer.md`). A handle is a place in this
process's memory and means nothing in another or in a stored node. `look` reads a region that
is being written while it is read, and its writer neither knows nor waits.

## Entry 98: the host, called by name

| N | name | children | what it does |
| --- | --- | --- | --- |
| 98 | host | name, up to six arguments | looks the name up in the host and calls it; answers what it returns, as an integer |

One lookup and one raw call reach everything the host offers, the C library and a framework
alike (graph node 5.8.3). An `int` argument goes as it is; any other leaf goes as the address of
its bytes with a zero byte after them, good for the length of the call. A name the host does not
know answers nothing with the reason `no-symbol`.

What the host does cannot be undone, so `host` is a fault, `irreversible`, while any choice is
open, inside an `all` or an `outcome` included: effects happen where nothing can fail back over
them. With no choice open a program wrote a file through `creat`, `write` and `close`, and found
the host's `getpid` equal to the pid in its own region's head. `ground/host-refused.ck` is the
refusal. A search that must act on the world holds the act back with `after`, entry 103.

## Entry 100: let, a part given a place

`(let $k v body)` evaluates v, then runs body on the same arguments with argument k being v. The
first child is the argument's own node and is not evaluated. Nothing else changes: `self` is still
the recipe the let stands in, a `cut` in body commits what it would have committed outside, and
the bindings that body runs on are the same. Arguments above the ones passed, up to k, are
nothing.

A let is how a part is named and used twice. Its number is a level and not a distance: a let
inside another let, or inside a choice, leaves every argument where it was, so `$2` means the
same thing through all of a recipe's body. Giving an argument that already has a value hides the
old one inside body only; outside the let it is what it was, since the let makes a new activation
and the old one is never changed. `(let $0 5 $0)` at the top of a program answers 5.

A wrong shape, three children of which the first is an argument, is a fault, `arity`.

## Entry 101: outside, a scope above the program

`(outside x)` evaluates x as if it were asked from outside the run. What x binds is not one of
the program's own bindings:

- **Every layer sees it.** A program's own binding is seen by what stands above it and not by
  the implementations bound before it. One made outside is seen everywhere, inside every
  implementation, so a diagnostic bound there sits on every use of an operation and not only on
  the uses at the top.
- **No fail undoes it.** It was not part of the run, as a note is not. `(outside (unbind e))`
  takes the newest one for e away, and nothing brings it back.
- **It runs on what lies beneath it**: the outside bindings made before it, and the program as
  the place that used it sees the program. So `base` inside it gives exactly what would have run
  without it, at that layer. Under `(outside (layer x))` the bindings x makes see each other, as
  a layer's do.
- **It acts from outside.** What an implementation bound from outside binds or releases, however
  deep inside it, is bound and released outside too. So a whole module can be loaded from
  outside and manage its own bindings there, as the watcher and the emitter do when a running
  program is asked to load them (`spec/framebuffer.md`).

The limit an activation runs under holds both counts, the program's own bindings and the ones
from outside, and whether what is bound under it is the program's own. At most 127 can be bound
from outside at once; one more is a fault, `full`. `ground/from-outside.ck` holds a diagnostic
that sees one use of `add` when the program binds it and both when it is bound from outside, a
binding kept through a fail, a release, and two outside bindings of one entry with the newer
running on the older, a layer of two that call each other, and an implementation whose own
binding stays when the option that called it fails.

## Entry 102: bound, what runs an entry

`(bound e)` answers the binding that runs e, an entry or a recipe: the newest one, as the top of
the program sees it, wherever it is asked from. With none it answers nothing, `unbound`. The
answer is a node of entry 72, `(binding e i s)`: the entry, its implementation, and yes when the
implementation runs on its own binding, as one made in a layer does, so that its own name inside
it means itself. It is how a program reads what it or another has bound. The emitter takes an
entry's recipe from it, and compiles a call of the recipe by its own name only when s is yes:
outside a layer the name means what it meant before, and the code would not be the walker.
`(bound e f)` answers the binding of e as the implementation of f sees it, which is how the
emitter asks whether an operation it would replace by an instruction means, inside f, what the
bootstrap means by it.

## Entry 103: after, a call held back

`(after f a..)` is a call of the recipe f with the values a.., made later: when every choice
that is open now has been closed. It answers yes at once. If a fail returns past it first, the
call is never made; so is it when a fault leaves the `recover` it stands in. With no choice open
it is a call like any other.

So a search can act on the world and only the path that held does: the values are taken when
`after` is evaluated, and the act waits for the commit. Calls held back are made in the order
they were asked, each with nothing but its values. When a run ends with an answer while a choice
is still open, its choices are closed by that, and what waited is done before it ends. Inside an
`all` or an `outcome` nothing held back is ever done, since both end by failing; a held call's
own answer is not seen by anyone.

`ground/held-back.ck` looks for a number among five, holding back a line to a file at each try:
the file holds the one line of the try that held. A line held back before a fault is not written,
a direct `host` inside the search is refused, and a line still held when the program ends is
written before its outcome. It runs in the open, as a line of `make check`, since the checker's
own `outcome` would end by failing over it.

## Entry 104: nothing, with its reason

`(nothing r)` answers nothing, and r is the reason beside it: what an operation of the bootstrap
does when it has no answer, said by a program. It is not a fail and not a fault: the caller gets
nothing, and `outcome` shows `(list "nothing" r)`. The reader written in form refuses a print
this way, with the same word the bootstrap's reader gives.

## Entries 105 and 106: a recipe closed over values

`(close f a..)` answers the recipe f closed over the values a..: a node of entry 106,
`(closed f a..)`, which is data and can be called like a recipe. Called, it runs f with the values
it holds first among the arguments and the call's own after them. `(self)` inside it is the closed
node, so it can call itself; a closed node can be closed again, and the values of the outer one
come after those of the inner. `recover` takes one as its handler and `after` holds one back.
What is bound for f runs a closed f too, with the values as its first arguments.

This is how a recipe written inside another sees what the outer one holds: the lens writes
`{x | (x + sum)}` as the inner recipe closed over the outer arguments, when it uses one of them
(`spec/lens.md`). `ground/closes-over.ck` closes over two arguments and a part named by a let,
over nine at once, calls itself through a closed node, hands one to another definition and to
`recover`, and is refused a closing over what is no recipe.

## Entries 107 to 111: the bits of an integer

| N | name | children | what it does |
| --- | --- | --- | --- |
| 107, 108, 109 | band, bor, bxor | a, b | and, or and exclusive or of the 64 bits of two integers |
| 110, 111 | shl, shr | a, n | the bits of a moved n places up or down, zeros coming in; n from 0 to 63, otherwise nothing, `range` |

They work on the two's-complement bits and never overflow: what is moved out is lost.
SHA-256 in `form/id.ck` is written with them.

## Entry 112: fast, a faster form of what is bound

`(fast e code)` gives the newest binding of e, an entry or a recipe bound to a recipe, a faster
form: a code leaf that is tried first wherever that binding runs, on integers, and when it
declines the recipe runs. The binding stays what it was, where it was: every definition that
could see the recipe sees the code, the others of its own layer among them, and what is bound
later lies above both. It is a form of what is there and not a change to the run, so no fail
undoes it.

The code steps aside while something bound from outside sits on an operation of the bootstrap or
on an entry the program defines: a counter put on `add` from outside sees every addition, the
compiled ones too, and when it is taken off the code runs again.

## Entry 128: compile, which exists only in form

| N | name | children | what it does |
| --- | --- | --- | --- |
| 128 | compile | x | writes machine code for the recipe x and binds x to it, or for the recipe an entry x is bound to and gives that binding the code as its faster form; answers the code leaf, or no, changing nothing, when it cannot |

The bootstrap has no code for this entry. `form/compile.aarch64.ck` binds it to an emitter written
in form: a layer of definitions, one for each instruction it uses, which answers the
instruction's word as a number, and a few for the shapes of node it knows. It makes one pass over
the recipe's body, each node's instructions gathered as bytes by `all` and packed into one leaf
under entry 52. What it knows is numbers:

- an `int` leaf of any size, and an argument;
- `let`: the part is kept in the frame at its argument's place, and what was there is put back
  after the body;
- `add`, `sub`, `mul`, `div` and `mod`, with an overflow or a division by zero collected in one
  register, and the five operations on bits;
- `(if c a b)`, where c is an `lt` or an `eq` of numbers, `yes`, `no`, or an `if` of such tests.
  A nothing inside a test makes that test fail, as on the walker;
- a call of the recipe itself, by `(call (self) ..)` or by its own name when an entry was
  compiled, and a call of another definition by its name. Every definition reached that way is
  compiled into the same leaf, which begins with a table of branches, one for each, and calls
  go through the table: the leaf lies wherever it is put. A definition is taken in only as the
  caller's own implementation sees it, and only if it too is numbers. A call in the last
  position is a jump when nothing has overflowed before it, so a recursion written as a loop
  runs as a loop, to any depth.

For any other node the emitter emits a byte that `pack` refuses, so a recipe is compiled whole or
not at all, and what is not compiled stays on the walker.

**The leaf answers a number or declines.** It declines when an overflow or a division by zero is
left at its end, when it was given fewer arguments than the recipe reads, and when the machine
stack, whose floor it reads from the region's head, has no room for its frame
(`ground/deep-on-code.ck` goes 600,000 deep). A decline travels up through the calls above it,
and the walker then runs the whole call. What that call then calls does not try code again: a
called recipe runs beneath its declined code, and beneath a faster form that declined no faster
form is tried, since the reason to decline is still there. So wherever emitted code cannot be sure, the answer, the
reason and the fault are the walker's own: emitted code is faster or it is the walker, never
different. It keeps its arguments, fourteen places for them and for the parts a let names, and
the registers it uses in a frame of 208 bytes on the machine stack, and a node's result in a
register chosen by its depth, x19 to x25.

`ground/code-or-walker.ck` runs twenty-nine recipes on the walker and again after asking for
their code, and holds the two outcomes equal: every shape above, the largest and the most
negative numbers, a division by zero, a shift too far, tests with a nothing inside, and three
the emitter must refuse. Its last lines show an entry's code used by another definition of its layer, stepping
aside while `add` is watched from outside and back after; an entry bound outside a layer left on
the walker; two definitions that call each other and a loop that calls a third, compiled
together; that loop run for thirty million rounds; and a recursion deeper than the machine stack
answered by the walker. Nine faults planted in the emitter each turned it red. `ground/compiles-a-recipe.ck` loads the
emitter, runs a recursion on the walker, compiles it and runs a much longer one on the leaf.

Code for an entry is the faster form of the entry's own binding (entry 112), so it runs wherever
the recipe would have, for the other definitions of its layer too. The emitter refuses while any
operation it would replace by an instruction is bound otherwise where the recipe runs: its code
would not be what the walker does there.

## Limits

- A cell holds a node and has a trail, and that is all: it has no blueprint it is declared with,
  no access or constructor recipe and no view. Those belong to stage 5.
- A call or a `let` in the last position keeps no frame, and gives back the activations made
  since the frame it answers to unless an open choice, or the user of an implementation, could
  return to them. So a loop written as a recursion runs in the room of one round
  (`ground/loop-in-place.ck`). The nodes a run makes are not given back: a loop that counts to
  ten million fills the store with its integers.
- A recipe cannot name another recipe that names it back (question 6.1.14); `self` covers direct
  recursion only.
- A recipe node holds no values: one that uses the arguments around it is closed over them when
  it is made (entries 105 and 106), and each closing is a node. A `let` is not a recipe: its
  body keeps the arguments it stands among.
- Integer law is still question 6.1.11; the 64-bit bound is the floor's.
- The root cannot be built: its blueprint is nothing, and `pack` needs a blueprint.
- An operation that evaluates its children first takes at most eight, so a call passes at most
  seven arguments.
- A replaced step decides what each node means, and the bootstrap's walker still carries the run
  between nodes: the continuation, the choice points and the undo are its own and have no binding.
  The one step in machine code is written by hand and takes only the leaves.
- The emitter compiles numbers only: it calls only what it can compile with it, a test stands
  only as the condition of an `if`, there is no answer of yes or no, and an expression nested
  deeper than seven registers is not compiled. It cannot hand a node it does not know back to the
  walker: code may decline only before it has done anything, because a declined call is run
  again from its start. The watcher that offers recipes by how often they run sees only `call`.
- Where emitted code declines, the work is done twice: once until it declines, then on the
  walker. It goes on computing while it unwinds, so a recursion that branches can spend long on
  answers it will discard.
- Finding a leaf's runnable copy reads the born code from the oldest up, so it grows with how
  many leaves have been run. What runs a node is found through an index and does not.
- What is not an operation has no binding yet: the store's own making of nodes and the host
  functions that reserve memory and make code runnable. A raw call passes six arguments in
  registers, so a host function that takes a varying number of them, `open` or `printf`, does
  not get the ones after its fixed ones. A host function that faults is caught, but what it
  changed before it faulted stays changed.
- `sub` in form answers nothing when b is the least integer, where the bootstrap's `sub` may have
  an answer. `mul` in form takes one call for each unit of b. A form operation that meets nothing
  inside may name another reason than the bootstrap's.
- A binding has no name of its own: `unbind` takes the newest of an entry, not one in the middle.
- Nothing bounds a run but the region's capacities: a wrong recipe under a recursion gives no
  answer and goes on until an area is full, which is the fault `full`.
