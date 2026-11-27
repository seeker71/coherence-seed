# Core operations, floor version

What the bootstrap's walker runs. Each operation is a catalogue entry (`spec/node-encoding.md`,
section 4); a **composite** whose blueprint is one of these entries is executed, and every other
node answers itself. `bootstrap/ck.c` is the walker and `ground/` holds the programs that witness
it.

This page names 84 entries. 32 to 52 are operations and data, and 53 to 63 with 72 the kinds of
the floor's own records. 64 to 70 take a node apart and build one; 71 to 75 let a program replace
what runs; 76 to 80 are what the bootstrap does for itself. 81 is the walker's step and 82
observes an outcome. 83 and 84 divide, 85 is a program's own fault and 86 makes bindings one
layer. 87 and 88 are the encoding itself; 89 to 93 are cells, their trail, notes and unbinding;
94 to 97 read a region; 98 calls the host, 99 counts, 100 gives a part a place, 101 binds from
outside the run, 102 says what runs an entry, 103 holds a call back until a search has settled
104 answers nothing with a reason, 105 with 106 close a recipe over values, 107 to 111 work on
the bits of an integer, and 112 gives a binding a faster form. 126 and 127 are the kinds of the
substrate's head and of a lane's row; 124 names the running process's calling convention and
125 has no meaning yet. 128 is the first operation that
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

A reason is a node and is exempt from undo. On the numeric port the bootstrap's reasons and
the tags returned by `outcome` are catalogue entries in 16.2.3. The text-facing commands
supply the original text labels for their callers. The wire representation and image loader
are specified in [the NUMS specification](nums.md). `(fault r)`, entry 85, is a
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
| 51 | native | code, arguments | runs a CPU code leaf on up to six values, or a Metal map on two packed word vectors |
| 52 | code-aarch64 | | data: the blueprint of a leaf of AArch64 machine code |

## Fail, choice and cut

The definition (graph node 5.4.2): fail means backtrack, undo and reverse all changes
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

`(host-target)` (entry 124) returns the numeric ABI identity of the running process, or
`no` on a host with the generic runtime alone. A leaf with blueprint `(code abi)` carries
an executable payload for that ABI. The original bare `code` blueprint identifies AArch64.
`native` answers nothing with reason `no-native` when the identities differ. A binding
declines that implementation and can continue with its recipe.

`(has-target abi)` (entry 125) answers whether the process can use that target. It recognizes
the current CPU ABI and probes the default Metal device for the 64-bit integer arithmetic used by `metal-map64`.
For that ABI the leaf holds a compiled single-kernel Metal library. Its two arguments are
byte leaves of equal length containing little-endian 64-bit words. Dispatch completes before
the runtime returns a fresh byte leaf. Unequal lengths or partial words answer `range`.
A device or pipeline that cannot run the library answers `no-native`.

The CPU call has six value positions, followed by the framebuffer address and the argument-count
and handle-mask word. On AArch64 these are x0 through x7, with value and kind returned in
x0 and x1. On System V AMD64 the values occupy RDI, RSI, RDX, RCX, R8 and R9. The seventh
and eighth arguments are at entry RSP+8 and RSP+16. RAX and RDX return value and kind.
The generated x86-64 word backend preserves the callee-saved registers and restores RSP on
each return. [AMD64 calling convention](https://gitlab.com/x86-psABIs/x86-64-ABI/-/blob/master/x86-64-ABI/low-level-sys-info.tex)

An integer of up to eight bytes travels as its value. Every other argument travels as a
node handle. The low byte of the last argument gives the count, and the following bits
identify the positions containing handles. The return kind is 0 for an integer, 1 for a
Boolean, and 4 for a node handle. Kind 3 or a zero handle in kind 4 declines the call.
A malformed result answers nothing with reason `range`. An explicit `native` call that
the code declines answers nothing with reason `declined`. The runtime copies each local
leaf into executable memory on its first use.

### Code asks the bootstrap

The machine's `serve`, at byte 128 of the region, is the address of one function of the
bootstrap that does an operation on values in hand. Code calls it as it is called itself: the
values in x0 to x5, numbers as they are and nodes as handles, x7 saying how many and which; x6
holds the operation's number.

- `quote` is the value itself, so a number comes back as its node.
- `all` is the list of those values that are something.
- `node` takes the address of thirty-two bytes and answers the node whose id they are, or
  faults `no-node`. So code names a node by what it is and not by where one run keeps it.
- `gathered` with no value says, as a number, where the gathering of answers stands; with one
  value it adds that value, if it is something. `list` with that number answers the list of
  what was added since. What code gathered and did not take is given back when it returns.
- `note` is kept back: the cell is written when the code has answered, and not at all when it
  declines, since the walker then runs the call again and writes the note itself. `held` reads
  what was kept back before it reads the cell, so code reads its own notes.
- Any operation that works on values alone is itself: the parts of a node, building one,
  arithmetic, cells, the encoding. Control and the calling of recipes are not, and asking for
  them is the fault `kind`.

With 256 added to the operation the answer is asked as a number: x1 is 0 with the number in x0,
2 when the answer is nothing, 3 when it is a node that is no number. Otherwise x1 is 4 and x0
the handle, none for nothing. When the operation faults, the fault is the program's and the call
does not return: the walker goes on from the fault. So code reads and builds nodes without
knowing how the store is laid out, and what it gets is what the walker would have got.

## Faults the walker names

`arity` (wrong number of children), `arg` (no such argument), `no-recipe` (`self` outside a
recipe), `not-a-recipe` (calling something else), `kind` (an operand of the wrong kind),
`irreversible` (a call of the host while a choice is open), and `crash`: the processor faulted
inside a code leaf or a host function a program called. A bad address, an instruction that is
none, a recursion in machine code that runs off its stack: each is a fault like the others,
caught by `recover` and seen by `outcome`, and the run goes on (`ground/crash-is-a-fault.ck`). A
fault of the processor anywhere else, in the bootstrap's own code, ends the process.

`full` is the fault of an area of the region that has no room for what is asked of it; its reason
is `(list "full" name)`, the area's name beside it. All areas draw on the run's host memory allowance (see `spec/framebuffer.md`). Where a `recover` catches a fault, the frames,
the gathered values, the activations and the choices of the body that faulted are given back, and
the handler takes the frame of the `recover` itself, so the program goes on in the room it had
(`ground/full-is-a-fault.ck` asks a thousand calls deep for a room larger than memory, twice, and
finds its frames given back; `ground/loop-in-place.ck` catches a thousand faults in a loop and
holds as many frames as after one). A recover gives back no nodes, since the cells, bindings and
rooms of the body that faulted still stand: a store that is full stays full until a fail returns
past where its nodes were made, and what catches that fault can do little.

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
| 126 | substrate | the head of the substrate the node table lies in |
| 127 | lane | one lane of the substrate: whose it is, and how far its nodes are settled |

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
  control operation. A code leaf is called with the answers, numbers as they are and any other
  value as its handle (Native code, above). It may decline; then what lies beneath it runs, so
  code takes the cases it is sure of and leaves the rest. A recipe's call that its code declined runs
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
a `cut` handed through a helper recipe to the program that used it, which stops at a running
`outcome` (`cut-through.ck`), and a new
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
- **An AArch64 code leaf gets the node's handle in x0 and the number of the activation it stands in in
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
fail, a binding it made included, and the nodes it made are given back; only the outcome, the
nodes it names and what no fail undoes remain (What a fail gives back, below). A `cut` inside
x does not reach outside it, not even one run in the cut scope of the recipe that called x, and
a `recover` inside x still catches first.

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
  answers the last node, including `no`. A header without a record yields the `short` fault.
  `ice/freeze.ck` and `ice/thaw.ck` run them on standard input. A file
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
to and nothing is kept on the trail. A note, and a note written by code, go on the trail as kept
while a choice is open: a fail back past them leaves them standing, even where the same cell was
held since, and keeps what they hold.

What follows from that: a table built inside an option is gone when the option fails. A table
meant to last is built before the choices that will use it, as the lens indexes its words when
it loads. `ground/cells.ck` runs each kind of undo.

### What a fail gives back

A choice point marks where the nodes, their parts, what was gathered and the rooms stood when it
was made. A fail that returns to it gives back every node made since, as a machine for a
backtracking language gives back its heap, except what no fail undoes and what that holds:

- what an `all` around it has gathered, the answer and the reason in hand, an outcome's answer
  and a range's numbers;
- what notes hold and the cells they hold it in, a faster form, code that was born and the nodes
  its bytes name by id, and bindings made from outside;
- the blueprint, the parts and the room of anything kept.

What stays moves down in the order it was made, so its handle can change: a handle kept as a
number across a fail back past where its node was made may name another node afterwards, while
its id stays the same. A node made while no choice was open, and every node that stood when the
last open choice closed, is settled (`spec/framebuffer.md`, The substrate): it lies below every
choice point's mark, so no fail gives it back or moves it. A kept node whose writer was given back is written by the choice that
kept it. A choice that stays open begins again from where things stand after the fail. The area
`gives` counts the times nodes were given back and names the choice, the mark and the top of the
last time (`spec/framebuffer.md`). `ground/gives-back.ck` runs forty thousand outcomes of 64
lists each, more nodes than the area holds, and ends with a note made in a failed branch, the
answers gathered, an outcome's answer, a node made before the outcomes at its own handle and the
lane's settled watermark where it stood before them while they ran. Once they close, the watermark
is past their answer, which keeps its handle through the fails after it, and a note made after a
hold on a key that the fail moves onto a page of cells not used before still stands.

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
process's node table, which is lane 0 of a substrate inside its own region, and means nothing in
another process or in a stored node. `look` reads a region that
is being written while it is read, and its writer neither knows nor waits.

## Entry 98: the host, called by name or by address

| N | name | children | what it does |
| --- | --- | --- | --- |
| 98 | host | a name or an address, up to eight arguments | calls it: a name is looked up in the host, an `int` is the address itself; answers what it returns, as an integer |

One lookup and one raw call reach everything the host offers, the C library and a framework
alike (graph node 5.8.3). An `int` argument goes as it is; any other leaf goes as the address of
its bytes with a zero byte after them, good for the length of the call. A name the host does not
know answers nothing with the reason `no-symbol`, and so does the address 0. A call of an address
where no code lies is the fault `crash`, like a crash inside code.

The address of the host's own lookup, `dlsym`, is the word `symbol` of the region's head
(`spec/framebuffer.md`), so `(host symbol -2 name)` answers the address of any function and a
program that holds no name as text can still reach the host. On the numeric port a first value
that is not an `int` is, as before, a call of the entry bound to it; an `int` is an address there
too. This was read in the code and compiled into `seed-run`; it was not run on the port.

What the eight registers cannot carry is carried by the package `host/` (graph node 5.14.60): a
host function is a row with a slot for each value, and one machine-code leaf, the berth, lays
integers, floats, addresses and varying values where the host's calling convention wants them
and brings back a float or an integer result. `host/all.ck` loads it.

What the host does cannot be undone, so `host` is a fault, `irreversible`, while any choice is
open, inside an `all` or an `outcome` included: effects happen where nothing can fail back over
them. With no choice open a program wrote a file through `creat`, `write` and `close`, and found
the host's `getpid` equal to the pid in its own region's head. `ground/host-refused.ck` is the
refusal, by address as by name, when the ground catalogue runs it inside its search; run by
itself, by `make check-native`, it is the flow through the package `host/`. A search that must act on the world holds the act back with `after`, entry 103.

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
it is a call like any other. When the last open choice closes inside a `recover`'s body, what was
held back until then is made, and a fault in that body later drops only what it held since.

So a search can act on the world and only the path that held does: the values are taken when
`after` is evaluated, and the act waits for the commit. Calls held back are made in the order
they were asked, each with nothing but its values. When a run ends with an answer while a choice
is still open, its choices are closed by that, and what waited is done before it ends. Inside an
`all` or an `outcome` nothing held back is ever done, since both end by failing; a held call's
own answer is not seen by anyone.

`ground/held-back.ck` looks for a number among five, holding back a line to a file at each try:
the file holds the one line of the try that held. A line held back before a fault is not written,
a direct `host` inside the search is refused, and a line still held when the program ends is
written before its outcome. A hundred calls held back are made in the order they were held, and
a fault caught after a cut in its body closed every choice keeps the calls held outside it. It runs in the open under `make check-full`, since the checker's
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
form: a code leaf that is tried first wherever that binding runs, and when it declines the
recipe runs. The binding stays what it was, where it was: every definition that
could see the recipe sees the code, the others of its own layer among them, and what is bound
later lies above both. It is a form of what is there and not a change to the run, so no fail
undoes it.

The code steps aside while something bound from outside sits on an operation of the bootstrap or
on an entry the program defines: a counter put on `add` from outside sees every addition, the
compiled ones too, and when it is taken off the code runs again.

## Entries 113 to 117: rooms, bytes changed in place

| Entry | Name | Takes | Answers |
| --- | --- | --- | --- |
| 113 | room | | data: the kind of a room record, and the blueprint of the leaf that refers to one |
| 114 | room-trail | | data: the kind of a room-trail record |
| 115 | take | b, n | a new room of n bytes, all zero, under the blueprint b: a leaf of the room's place in the area `rooms`; nothing, `range`, for n below 0, and the fault `full` for more than the area can hold |
| 116 | load | r, o, w | the w bytes from byte o of the room r as an unsigned integer, little-endian; w is 1, 2, 4 or 8, and eight bytes give the integer's 64 bits; nothing, `range`, outside the room |
| 117 | store | r, o, w, v | writes the low w bytes of the integer v at byte o of the room r, and answers v |

A room is a record in the area `rooms`: its blueprint, the node that last stored into it, its
size and how many stores it has had, then its bytes. It is how a cell holds a value whose
blueprint the place fixes, as its bytes alone, with the blueprint once in the room's head
(`spec/nums.md`). While a choice is open a store first puts each four bytes it changes on the
room trail, with the room's writer and count, and a fail puts them back; with no choice open
nothing is kept. A room taken since a choice point is given back when a fail returns to it,
unless a node that stays names it. `ground/rooms.ck` stores, loads, has two
stores undone by a fail and is refused outside a room and for a width of three.

## Entries 118 to 123: integers as their 64 bits

| Entry | Name | Takes | Answers |
| --- | --- | --- | --- |
| 118, 119, 120 | wadd, wsub, wmul | a, b | the sum, difference and product of two integers modulo 2^64, as the 64 bits of an integer |
| 121 | ult | a, b | yes if a is less than b when both are read as unsigned, otherwise no |
| 122, 123 | udiv, umod | a, b | the quotient and the remainder of a by b, both read as unsigned; nothing, `zero`, for b zero |

`add`, `sub` and `mul` answer nothing when the true result does not fit in 64 bits. A number of a
fixed width wraps instead, so the engine's arithmetic on such numbers (`spec/nums.md`) uses
these and keeps the low bits its layout has. A 64-bit unsigned number is held in an integer's
64 bits, and `ult`, `udiv` and `umod` read it as such.

In the NUMS image, entry 119 calls the resident instance `(b.instance wsub int)`.
The recipe computes the complement of the wrapping sum of the complemented left word
and the right word. Resolved integer subtraction calls that instance directly. The
compiler can attach machine code to its binding while callers keep the same id.
The C implementation supplies subtraction during cold execution.

Entry 121 uses the same binary binding. Its instance `(b.instance ult int)` flips the
high bit of each operand and compares the resulting signed words. This maps the unsigned
sequence from zero through 2⁶⁴ − 1 onto signed order from −2⁶³ through 2⁶³ − 1.
The recipe returns a Boolean node. The C implementation serves cold callers.

## The operations describe themselves

`form/operations.ck` binds `operations` to a list with one node for each operation: the entry,
the kinds of what it takes, the kind of what it gives, and what it does to the world. A kind is
`int` for a number, `node` for any value, `quote` for a part that is taken as it stands and not
evaluated; `rest` after a kind means any further number of it, and `no` for what it gives means
that depends on its parts. What it does is one of six: `op.reads` (it changes nothing and can
be done again), `op.builds` (it makes a node; doing it again makes the same node),
`op.notes`, `op.marks` (it changes a cell, a binding or something outside the store),
`op.faults` and `op.controls`. `(op.of e)` is an entry's description, and `op.takes`,
`op.gives` and `op.effect` its parts.

`ground/operations-as-described.ck` holds the descriptions to the bootstrap for the 37
operations that read or build with a fixed number of parts: each takes the number its
description says and refuses one more. Two wrong descriptions planted were both named. The kinds
are not checked, and neither the walker nor the bootstrap reads the descriptions.

The emitter reads these and holds no list of its own: the kind of a value, which places of an
operation are numbers, and whether an operation can be asked of the bootstrap, which is so for
one that reads, builds, notes or faults.

## Entry 128: compile, which exists only in form

| N | name | children | what it does |
| --- | --- | --- | --- |
| 128 | compile | x | writes machine code for the recipe x and binds x to it, or for the recipe an entry x is bound to and gives that binding the code as its faster form; answers the code leaf, or no, changing nothing, when it cannot |

The bootstrap has no code for this entry. `form/compile.aarch64.ck` binds it to an emitter written
in form: a layer of definitions, one for each instruction it uses, which answers the
instruction's word as a number, and a few for the shapes of node it knows. It makes one pass over
the recipe's body, each node's instructions gathered as bytes by `all` and packed into one leaf
under entry 52. What it knows:

- an `int` leaf of any size and an argument;
- any other node as a constant, a leaf standing for itself or a node under `quote`: an entry of
  the bootstrap's catalogue by its handle, which is the same in every run, and any other by its
  id, written into the leaf and looked up by the bootstrap where it is used;
- `let`: the part is kept in the frame at its argument's place. What was there is put back
  after the body only where something outside the let could read it, which costs a register;
- `add`, `sub`, `mul`, `div` and `mod`, with an overflow or a division by zero collected in one
  register, and the five operations on bits;
- the parts of a node and the building of one: `blueprint`, `count`, `leaf`, `child`, `byte`,
  `held`, `join` and `pack`, and `(all (choice a b ..))` of up to six parts, which is what
  `tuple` stands for. Each is asked of the bootstrap (Native code, above), so the code holds
  handles and the store stays the bootstrap's;
- `(if c a b)`, where c is an `lt` of numbers, an `eq` of numbers or of nodes, anything that
  answers a node, which holds unless the node is `no` or there is none, `yes`, `no`, or an `if`
  of such tests. A nothing inside a test makes that test fail, as on the walker;
- an `lt` or an `eq` as a value: `yes`, `no`, or nothing when a part of it is nothing;
- control that does not search: `seq`, `need`, `fail`, and `(first (choice a b ..))` over
  alternatives that each answer once or fail; any of these as the condition of an `if`; a part
  of a row that fails is left out of it; `fault` and `nothing`;
- a search inside one definition: `(all x)` and `(first x)` where x is built of `range`,
  `choice`, `let`, `seq`, `need`, `fail`, `if` and operations whose parts are searches
  themselves;
- a definition that answers more than once, called from such a search;
- `note`, and what reads without leaving a mark: `bound`, `handle`, `id`, `print`, `encode`,
  `decode`, `read`, `file`, `at`, `peek`; `close`, a recipe as a value, and `self`;
- a call of the recipe itself, by `(call (self) ..)` or by its own name when an entry was
  compiled, and a call of another definition by its name. Every definition reached that way is
  compiled into the same leaf, which begins with a table of branches, one for each, and calls
  go through the table: the leaf lies wherever it is put. A definition is taken in only as the
  caller's own implementation sees it, and only if it too can be compiled. A call in the last
  position is a jump when nothing has overflowed before it, so a recursion written as a loop
  runs as a loop, to any depth.

For any other node the emitter emits a byte that `pack` refuses, so a recipe is compiled whole or
not at all, and what is not compiled stays on the walker.

**Every value has a kind, a number or a node**, and the emitter knows it as it writes. A
parameter that is used as a number, in arithmetic, in a comparison with a number or as an index,
is taken as a number, and the code declines a call that gives it anything else. Every other
parameter is taken as a node and takes anything: a number given for it is made a node on the
way in. Where a number is wanted and a node is in hand, or the other way round, the bootstrap is
asked for the one from the other. A definition answers a node if any of its ways out does. The
kinds decide how fast the code is and never what it answers.

**Once it has declined, code asks for nothing more.** After an overflow, a nothing where a
number was wanted or a decline from a call, no call is made, of the bootstrap or of another
definition: the code is marked declined instead. So the bootstrap is never handed a value that
means nothing, and a fault the code did not reach is the walker's to find.

**A definition may fail.** Inside code a failure is a quiet mark: what follows it is passed
over without a call being made, until something that takes a failure is reached, the condition
of an `if`, the next alternative of a `first` or a part of a row. What was collected inside the
failed stretch, an overflow or a nothing, goes with it; a decline does not. Between definitions
compiled together a failure passes as a failure. Where the walker called the code, a failure is
a decline: the walker runs the call again and fails in its own way, with its own reason.

**A search is a loop.** Inside an `all` or a `first` a `range` is a loop, the alternatives of a
`choice` follow one another, a `need` that does not hold jumps past what depends on it, and an
operation with a search among its parts is done once for each answer of that part. What comes
after an answer is written out at each place an answer arises. `all` gathers its answers in the
bootstrap and takes them as a list; `first` keeps its first answer and sets a third quiet mark,
stopped, under which loops end and no call is made until the `first` is left. What a search
keeps from one answer to the next lies in the frame, which is 336 bytes for a definition that
searches. An answer that is nothing is an answer, as on the walker.

**A definition that answers more than once hands its answers on.** It is compiled as a search
of its own. Where it has an answer it calls the code of the search that called it, which it was
given together with the caller's frame; that code takes the answer, does what follows in the
caller, and says whether to go on. A `first` that has its answer says stop, and so does
anything that declined. So a search crosses from one definition into another, and a definition
may call itself while answering: every node under a node, each handed up through the calls
above it. What a search keeps between answers lies in its frame for this reason: the code that
takes an answer runs beneath the definition that gave it. Such a definition is compiled only in
the leaf of one that searches it; asked for by itself it is refused, since the walker takes one
answer at a time.

**The leaf answers a number or a node, or declines.** It declines when an overflow or a division by zero is
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
negative numbers, a division by zero, a shift too far, tests with a nothing inside, and two
the emitter must refuse. Its last lines show an entry's code used by another definition of its layer, stepping
aside while `add` is watched from outside and back after; an entry bound outside a layer left on
the walker; two definitions that call each other and a loop that calls a third, compiled
together; that loop run for thirty million rounds; and a recursion deeper than the machine stack
answered by the walker. Nine faults planted in the emitter each turned it red.
`ground/code-on-nodes.ck` does the same for eighty recipes over nodes: reading, comparing
and building them, texts, lists and entries as constants, comparisons as values, notes written and
read, a nothing and a fault at each place one
can arise, a number where a node is wanted and the other way round, definitions that hand nodes
to each other, and a chain of a hundred thousand pairs built by one and walked by another. For
each it says whether the code answered or handed the call back: fifteen hand back, each one
that ends in nothing or in a fault, and the rest the code answers. Twelve faults planted in the
emitter and three in the bootstrap's keeping of notes each turned it red. `ground/code-without-search.ck` holds thirty-six recipes that test,
fail and choose a first alternative to the walker's outcome, a failure crossing between
definitions and into a row among them; the ten that end with no path left are handed back, and
ten more planted faults each turned it red. `ground/code-searches.ck` does it for thirty-three
searches: ranges within ranges, a search as a part of an operation, a `first` inside an `all`,
tests and failures between the answers, a nothing and a fault among them. Two are handed back,
one that ends with no path and one that divides by zero on its way; seven planted faults each
turned it red. The primes below 3,000 by trial division and the pairs below 400 were counted in
79 ms on the walker and 2 ms compiled. `ground/code-hands-answers.ck` does it for twenty-five
recipes that search definitions which answer more than once: one handing on another's answers,
one walking a tree by calling itself, answers of both kinds, a first that stops an endless one,
a fault inside one. Three are handed back and one, such a definition asked for by itself, is
refused. Five planted faults each turned it red, and a sixth made it never end. All 5,711
nodes of the lens's definition were handed up by a definition calling itself, counted the same
as on the walker.

`ground/compiles-its-own.ck` has the emitter compile the system and then itself. Walked, it
compiles the 228 definitions the other modules bind, and 112 of them give code. Then it
compiles its own 149: 95 give code, all 55 instruction builders among them. Then, running on
that code, it compiles the 228 again, and the program holds two things: every leaf is byte for
byte the one the walked emitter wrote, and the second pass took less than half the time of the
first (2.1 s and 0.5 s when measured). Each of the 207 also compiles the same alone in a fresh
process. That count once came out differently in two programs, which is how a wrong kind for
the root as a constant was found.
The reader of the print written in form (`form/read.ck`) read the lens's 19,780 bytes in 20 ms
on the walker and in 7 ms after this, to the same node; the lens showed its own definition and
read it back in 54 ms and then in 50 ms. The emitter compiles a definition again with each one
that calls it, and the rest of it, 30 definitions, is walked. `ground/compiles-a-recipe.ck` loads the
emitter, runs a recursion on the walker, compiles it and runs a much longer one on the leaf.

Code for an entry is the faster form of the entry's own binding (entry 112), so it runs wherever
the recipe would have, for the other definitions of its layer too. The emitter refuses a definition while
any operation it uses, or the step, is bound otherwise where the definition runs: its code would
not be what the walker does there.

## Limits

- A cell holds a node and has a trail, and that is all: it has no blueprint it is declared with,
  no access or constructor recipe and no view. Those belong to stage 5.
- A call or a `let` in the last position keeps no frame, and gives back the activations made
  since the frame it answers to unless an open choice, or the user of an implementation, could
  return to them. So a loop written as a recursion runs in the room of one round
  (`ground/loop-in-place.ck`). The nodes a run makes are given back only by a fail that returns
  past where they were made: a loop with no choice open that counts to ten million fills the
  store with its integers.
- A recipe cannot name another recipe that names it back (question 6.1.14); `self` covers direct
  recursion only.
- A recipe node holds no values: one that uses the arguments around it is closed over them when
  it is made (entries 105 and 106), and each closing is a node. A `let` is not a recipe: its
  body keeps the arguments it stands among.
- Integer law is still question 6.1.11; the 64-bit bound is the floor's.
- The root cannot be built: its blueprint is nothing, and `pack` needs a blueprint.
- An operation that evaluates its children first holds their values in hand, in the area
  `values`, so a call passes as many arguments as memory holds, and a recipe closes over as many.
  Code takes six values in registers: given more, it declines, and the recipe runs on the walker.
- A replaced step decides what each node means, and the bootstrap's walker still carries the run
  between nodes: the continuation, the choice points and the undo are its own and have no binding.
  The one step in machine code is written by hand and takes only the leaves.
- A definition that answers more than once is compiled only inside the leaf of one that
  searches it, and each answer passes through the code of every search above it, so answers
  from deep inside a recursion cost their depth. A search stands only under an `all` or a
  `first`, or as the whole of a definition: not as the condition of an `if`. No `cut` of its
  own, no `recover`, and no call of a recipe held as a value, which is how the emitter passes
  what follows an answer. What comes after an answer is written again for each alternative, so
  code for a wide search grows. No mark but a note: no `hold`, `bind` or written file, because a declined call is run again from
  its start, and building a node twice gives the same node while writing a cell twice does not.
  An expression nested deeper than seven registers is not compiled. So the emitter compiles
  part of itself and not the whole.
- A definition that fails where the walker called it is run again on the walker.
- Code for a recipe that is not an entry's is bound to the recipe as it read when the code was
  made. What the program binds afterwards, an operation or a definition the recipe calls, that
  code does not see, though the walker running the recipe where it is called would; and when
  such code declines, the recipe runs beneath it and misses those later bindings too. An entry's
  code has neither gap: an entry's recipe sees only what is older than its binding, with code
  or without. The watcher relies on the first: it binds `call` again after each compile.
- Each operation on a node is a call into the bootstrap; none is written out as instructions.
  A chain of a million pairs is built and summed in 0.13 s, against 0.53 s on the walker, and
  most of what is left is the store finding each node. A constant named by its id is looked up
  at each use: a loop of 300,000 rounds, each comparing a child with a text, took 2 ms against
  88 ms.
- The watcher that offers recipes by how often they run sees only `call`.
- Where emitted code declines, the work is done twice: once until it declines, then on the
  walker. It goes on computing while it unwinds, so a recursion that branches can spend long on
  answers it will discard.
- Finding a leaf's runnable copy reads the born code from the oldest up, so it grows with how
  many leaves have been run. What runs a node is found through an index and does not.
- What is not an operation has no binding yet: the store's own making of nodes and the host
  functions that reserve memory and make code runnable. A raw call passes eight arguments in
  the registers AArch64 has for them, so a host function that takes a varying number of them,
  `open` or `printf`, does not get the ones after its fixed ones through entry 98 alone; a row of
  `host/` run through the berth gives them. A host function that faults is caught, but what it
  changed before it faulted stays changed.
- `sub` in form answers nothing when b is the least integer, where the bootstrap's `sub` may have
  an answer. `mul` in form takes one call for each unit of b. A form operation that meets nothing
  inside may name another reason than the bootstrap's.
- A binding has no name of its own: `unbind` takes the newest of an entry, not one in the middle.
- A growing recursion shares the run's memory allowance with all other areas. Reaching it
  raises the area's existing `full` fault. A loop that reuses its pages can still run indefinitely.
