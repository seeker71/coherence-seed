# The engine: blueprints, recipes and cells

This page fixes the design of what the seed stores and runs. It follows NUMS (`NUMS.Go`, read on
2026-11-26; graph hubs 3.1 and 3.2) and the five axioms. Of this page only what its last
section names is built; the rest is the design the work follows.

## What is stored, and what runs

- **Only nodes are stored.** A node is a category and ordered children, or a category and bytes
  (`spec/node-encoding.md`). Nothing else is in the repository beside the bootstrap.
- **Three roots.** A *blueprint* says what a thing is. A *recipe* says what is done. A *cell*
  holds a present value of a blueprint. Every node belongs to one of them.
- **At the altitude of languages.** The catalogue names what programs mean: a struct, a list, a
  template; an if, a loop, a write, a call, a new. A program in Go, BML or any other language
  becomes these nodes without loss, and is shown again in any of them.
- **Run as they stand.** The engine executes a recipe node on cells directly. There is no lower
  form that a recipe is first turned into.
- **Words are beside.** Names of blueprints, members, locals and functions, and the spelling of
  each catalogue entry in each language, are rows in tables keyed by node. They change no node.
- **Text is a view.** A lens writes a language's text from nodes. That text may be kept in a
  local cache; it is never stored here.
- **The runtime reads no words.** It loads nodes as their numbers and runs them by their ids:
  what a node is, is its blueprint's id, and what runs it is the binding of a recipe's id.
  Nothing is left to resolve, and no text is in it except what is shown to a person.
- **Two runtimes from one description.** The generic runtime runs the nodes as they stand; a
  compiler writes machine code for any recipe from the same nodes, for the CPU or the GPU, with
  or without tracking and tracing, boxed or unboxed, its stack and loops unrolled as the
  hardware wants. Either can stand in for the other while the system runs.

## Loading and calling a resolved image

`nums.image` is the entry at 16.2.4.1. Its first child initializes the bindings. Its second
child is a list that keeps the public recipes, blueprints and constants reachable. The packer
expands module imports once per image and freezes the resulting graph in dependency order;
a file that a statement of the initialization reads as data, such as a package handed to
`part.load`, is read when the image is packed, since the image runs where there are no files.
The export list can carry arbitrary user data. Argument names and the lexicon live in the
authoring lens. The generic NUMS library with its float definition, both AArch64 compilers and the example
exports pack together as 14,523 nodes containing zero text leaves.

```sh
make
./ck text nums/pack.ck < ground/nums-image.ck > /tmp/nums.ice
./ck text ground/nums-requests.ck | ./seed-run /tmp/nums.ice | ./ck run ground/nums-replies.ck
```

`seed-run` loads the canonical binary records from the image. Its input and output are byte
streams. It initializes once. Each request consists of one unsigned byte giving the number
of arguments, a 32-byte target id, and one 32-byte id per argument. The direct port accepts
as many arguments as its count byte numbers, zero to 255; a list node carries a longer vector.
A request that cannot be read to its end is answered `short`. A bound operation
receives its arguments as values. A recipe is called. A closed executable expression runs.
Each reply has a one-byte outcome (0 nothing, 1 node, 2 fault), the 32-byte id of its value
or reason, a four-byte little-endian archive length and that many bytes of frozen graph.
Further requests in the session can refer to returned ids, including a constructed cell with
mutable room storage. The returned graph preserves immutable structure. Resuming a live room
across processes also needs the state of its framebuffer.

Bootstrap status nodes belong to 16.2.3. The words `e.arity`, `e.range` and their peers
belong to the display lens. A numeric host call dispatches a bound capability by id. The
numeric observer submits and receives content ids. The `ck run` and `ck text` tools retain
their text-facing diagnostic convention.

## Execution and specialization

The generic evaluator defines the observable result, including the writes restored by a
failed choice. A machine implementation uses the same recipe id. A target specialization
also depends on the processor and instruction set, input layouts, calling convention and the
observations requested by the caller. A bare machine-code leaf is meaningful for the target
that produced it. The choice of boxed or unboxed values belongs to the calling convention.
Stack expansion and loop unrolling belong to a compilation decision for a particular shape
and target. A variant that omits observation points can run where those observations are
optional. The walker supplies the common, observable execution path.

A packed image carries the recipes and backend declarations. At startup, `host-target`
reports the calling convention of the running process. The runtime selects a `cpu.backend`
row by that numeric identity and invokes its setup recipe. The row provides a compiler for
declared NUMS functions and can supply a compiler for Form recipes. A compiler can decline
a body, and generated code can decline particular arguments. Frames evaluate either case.

| Process target | Native coverage |
| --- | --- |
| macOS AArch64 | The existing Form emitter and optimizing NUMS compiler, including calls, branches, typed integer operations, and floating point. |
| macOS and Linux x86-64, System V ABI | Up to six integer inputs, integer and Boolean literals, addition, subtraction, multiplication, bitwise operations and comparisons. A Boolean condition selects one of two integer or Boolean arms of matching kind. NUMS supports nested conditionals, two returning arms and a sequence of guarded returns ending in a return. Form also supplies checked arithmetic, wrapping arithmetic and unsigned less-than. Calls, loops and floating-point bodies use frames. |

`nums/word-target.ck` holds the expression compiler as a NUMS component. The target supplies
argument encodings, instruction and result-kind rows, literal and return encodings, and the
entry convention. The compiler recursively combines fragments and keeps the required argument
count and result kind alongside the bytes. Checked x86-64 arithmetic restores the entry stack
and declines on overflow, so the caller observes Form's original outcome.

Both Form and NUMS conditionals lower through the same branch component. The target describes
each forward jump by its opcode bytes and displacement width. The compiler measures the distance
from the end of that instruction, including the jump past the alternative when appropriate.
Only the selected arm executes.

To add a backend, assign its ABI a numeric node, list it in `fb.targets`, and select that
identity in the host port. Supply its setup and compilation recipes in a backend row in
`nums/run.ck`. A backend can reuse the word-expression compiler or provide its own lowering.
The OS port supplies executable memory and instruction-cache synchronization. Add the target
to the applicable ground witnesses as its capabilities run. Recipe IDs and image requests
stay portable.

`make check` sends the same recipe ID for `x + 2*y` before and after compilation and receives
42 both times. It also calls a generated NUMS function directly, and compiles the resident
word-subtraction instance by ID. A conditional recipe returns 2 and 42 for opposite orderings
both before and after compilation. A NUMS function with guarded returns also handles equality
and returns 0. Linux and macOS x86-64 returned identical binary replies from
one image packed on AArch64.

### Metal word maps

The macOS GPU runs a batch through one NUMS declaration with signature `(b.function int int int)`.
The Metal profile in `nums/metal.ck` lowers the two parameters, integer literals and nested
addition, subtraction, multiplication, AND, OR and XOR. It uses unsigned 64-bit Metal arithmetic
to preserve Seed's wrapping word results. It shares declaration and return-shape recognition with
the CPU compiler. Other expressions decline during lowering.

```sh
make metal-image METAL_RECIPE=ground/metal-polynomial.ck \
  METAL_INPUTS=ground/metal-vectors.ck METAL_IMAGE=/tmp/words.image
./ck text ground/metal-requests.ck | ./seed-run /tmp/words.image | ./ck run ground/metal-replies.ck
```

The builder projects the declaration to temporary Metal source and invokes Apple's Metal compiler
and library linker. The image holds the compiled library, the numeric map entry
`(b.instance b.gpu declaration)` and the supplied operands. A request addresses that entry
and two operands by content ID. Each operand packs one 64-bit word per lane. The entry ID
depends on the declaration and stays the same across builds of its Metal library.

The `metal-map64` ABI binds one shared word buffer at index 0 and the unsigned 32-bit lane count
at index 1. For `n` lanes the two inputs start at words 0 and `n`, and the output starts at word
`2*n`. The device must advertise Apple GPU family 3 or Metal family 3 for 64-bit
integer arithmetic. [Metal feature tables](https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf)

No C knows Metal. The component package `host/metal.ck` runs the map through the host door
(`spec/core-operations.md`, entry 98): the seven host functions it calls and the 28 messages it
sends are rows of `host/host.ck`, the two device families, the eight objects it holds and how
each is let go are rows of its own, and it reaches the host only through seven recipes it is
handed as a `metal.door`. `host/metal-bind.ck` makes that door, binds the blueprint
`(code metal-map64)` to `metal.map` and `has-target` to a recipe that asks `metal.ready` for
that target; `host/metal-all.ck` also reads where the door's words lie from the region's
layout and opens the door. An image carries `host/metal-bind.ck` linked in and those places as
numbers, with the layout version they were read under, and faults `metal.open` on another.

Every object a dispatch makes is written into a slot of one room, the bench, as it is made:
the autorelease pool, the device, the library's data, the library, the kernel, the pipeline,
the queue and the buffer. The bench is taken once per process. Whatever stands in a slot is
let go when the dispatch ends, when a fault leaves it, and before the next dispatch begins, so
a fault midway strands nothing. The two sizes, three words each, and the lane count lie in the
bench as well and are passed by their addresses. The output is copied from the shared buffer
into a second room, which grows to the largest output asked for, and leaves as a leaf of the
region's own bytes.

The threadgroup width comes from the pipeline, and the kernel checks its lane against the
supplied count. Before anything is made, a room of three times the input plus the library is
taken and given back by a fail: where the allowance or the host cannot hold it the map
answers `declined`. The buffer itself is the host's memory, checked against the device's
longest buffer and its reported working set. On a Mac with the compiler and a Metal device,
the image flow compares both orders of 65 inputs with their frame results, including
wraparound, the empty vector and two invalid shapes. The compiler is a build-time dependency.
Rows exist for Apple's arm64 convention only: under Rosetta and on Linux x86-64 the door does
not open, `has-target` answers no and a map answers `no-native` after the same checks of
shape. Floating-point layouts, reductions and pipeline reuse are open.

The word instance of subtraction is shared by signed and unsigned integer layouts. The layout
recipe reads the operands and packs the result at its own width. The word recipe works on the
64 bits in between. On the M4 Max the same call by instance id took 18 walker steps, then 5
with the 252-byte code leaf attached. Both returned the id of the maximum signed word for the
minimum signed word minus one.

The generic runtime shares one word instance across unsigned `<`, `>`, `<=` and `>=`.
Operand order and Boolean negation express the four relations. In the resolved image,
`r.gt` places 2⁶³ above 2⁶³ − 1 through that instance. Calling the instance by its own id
on the M4 Max took 21 walker steps, then 5 after Seed attached its 296-byte AArch64 body.

### Recipe tokens and live variants

`nums.token-request` takes a vocabulary, a token number and the argument ids. A vocabulary is
a list of target ids with `no` in ordinary token slots. A mapped token produces the same binary
request as a direct call. An ordinary token or an index outside the vocabulary answers nothing
with `e.unbound`. The vocabulary has its own content id, so two models can give the same token
number different meanings. In the image example, token 1 calls the word instance of subtraction.
A model decoder feeding the reply to its next prediction is still to be connected.

A recipe body has an immutable id. Its instance has a binding that a trial can replace.
A candidate genome can therefore hold recipes or typed subtrees. Development fills a template,
the evaluator calls the candidate on the task's cells, and `outcome` restores the cells and
bindings covered by the choice. The evaluator can compare the returned observations and select
a body for that instance. A compiled body runs under the same binding. For a stateful method,
the comparison includes the resulting cell state observed inside the trial.

## Coordinates

A catalogue entry is addressed by four numbers, `package.level.type.instance`, as axiom 2 and
NUMS have it. Packed into one number, which is the bytes of the entry's leaf:

| field | bits | meaning |
| --- | --- | --- |
| instance | 0 to 31 | the member of the family |
| type | 32 to 47 | the family |
| level | 48 to 55 | 1 trivial (a leaf), 2 basic (the fixed catalogue) |
| package | 56 to 63 | 0 the bootstrap's own kinds, 1 blueprints, 2 recipes, 3 cells, 16 the engine's own definitions, 17 and up one per language or module |

- The number is a catalogue entry (`spec/node-encoding.md`), which holds up to 2^63 − 1, so a
  package is 0 to 127. Instance 0 of a family is the family itself, as NUMS's `Undefined`, and
  the members follow from 1 in NUMS's order (`nums_consts.go`).
- NUMS tells its blueprint space from its recipe space only by a type of the host language. Here
  the root is the package, so one number says which it is.
- A composite has no coordinate of its own. Its identity is its content id; its level is one
  more than the highest level among its category and children, computed when asked.
- A language that has a meaning no neutral entry matches keeps it under its own package, and an
  equivalence to a neutral entry is a recorded relation, not an assumption.

## Blueprints (package 1)

The families are NUMS's, with three added (Layout, Part, FloatKind).

| level.type | family | members |
| --- | --- | --- |
| 1.1 | Void | |
| 1.2 | Numeric | Bool, Integer, Decimal, Imag, Complex, Char, String, Tensor, Time, Binary, Pixel |
| 1.3 | Control | Mutex, Lock, Semaphore, Wait, Barrier |
| 1.4 | Runtime | Thread, Process |
| 1.5 | Resource | Handle, Device, CPU, GPU, TPU, Network, Disk, IO |
| 1.6 | Variadic | |
| 2.1 | Reference | Pointer, Reference, Symbol, Member, Optional |
| 2.2 | Container | List, Dictionary, Object, Package, Interface, Any, Matrix, Location, Set, Template, Union, Tuple |
| 2.3 | Recipe | Function, MemberFunction, Local, Parameter, Global |
| 2.4 | Data | Channel, Event |
| 2.5 | Layout | Bits, TwosComplement, Float, Fixed, Affine, Block, Table, Utf8, CodePoint |
| 2.6 | Part | Member, Method, Nominal, Hole, Instance |
| 2.7 | FloatKind | Ieee, Fn, Fnuz, Finite, Scale |

A blueprint in use is a node over these:

- **A number with its layout.** `(Integer (TwosComplement 32))` is a signed 32-bit integer,
  `(Integer (Bits 8))` an unsigned byte. A float is `(Decimal (Float e m kind))`: a sign bit, e
  bits of exponent and m of fraction, and a kind that says what the highest exponent means.
  Ieee has infinities and NaN: binary16, 32 and 64, bfloat16 `(Float 8 7 Ieee)`, E5M2. Fn has
  no infinity and NaN only at all ones: E4M3. Fnuz has no negative zero and NaN where it would
  be. Finite makes every pattern a number: the 6- and 4-bit floats of microscaling. Scale is an
  exponent alone: E8M0. `(Fixed B k)` is B with k fraction bits, `(Affine B scale zero)` a
  quantized weight, `(Block B n scale)` n values sharing one scale, and `(Table B values)` a
  value chosen from a list by an index, as NF4. `Integer` or `Decimal` with no layout is the
  number of a language that does not say its width.
- **References.** `(Pointer B)`, `(Reference B)`, `(Optional B)`.
- **Containers.** `(List B)`, `(List B n)`, `(Tuple B..)`, `(Dictionary K V)`, `(Set B)`,
  `(Matrix B n..)`, `(Union B..)`, `Any`.
- **Objects.** `(Object part..)`. A part is `(Member B attributes init)` or `(Method signature
  body attributes)`. The attributes are on the reference to B, as in NUMS: public, static,
  const, embed, parent, has-default. A struct is an object of members; a class has methods and
  parents; a parent is a member with the parent attribute, and the parents' members are
  composed into one shape (graph 5.5.4). `(Interface signature..)` is a set of methods a shape
  may fit by structure.
- **Signatures.** `(Function result parameter..)`, each parameter a blueprint with its
  direction: in, out, receiver, variadic.
- **Nominal blueprints.** By axiom 3 two equal shapes are one blueprint. Where a language keeps
  two equal declarations apart, the blueprint is `(Nominal B mint)` with a leaf minted once.
- **Templates.** `(Template n body)` is a blueprint with n holes, written `(Hole k)` in its
  body; `(Instance template B..)` is it filled. An instance's identity is its content, so the
  same template with the same arguments is the same blueprint everywhere (graph 5.5.5).

## Recipes (package 2)

The catalogue is NUMS's levels 1 and 2, with the families of the axioms added (Search, Offer).

| level.type | family | members |
| --- | --- | --- |
| 1.1 to 1.7 | literals | Null, Bool, Integer, Decimal, Imag, String, Literal |
| 1.8 | BuiltIn | This, Super |
| 1.9, 1.10 | Local | declare, access |
| 1.11 to 1.14 | Parameter, Member, Global, Blueprint | |
| 1.15 to 1.18 | Label, Empty, Jump, Exception | |
| 2.1 | Compare | ==, !=, >, <, >=, <=, <=>, ===, !==, ~== |
| 2.2 | Logic | not, and, or |
| 2.3 | Math | +, -, *, /, //, % |
| 2.4 | BitMath | not, and, or, xor, shift left, shift right, signed shifts, and-not |
| 2.5, 2.6 | TempWrite, Write | =, the compound writes, pre and post increment and decrement, send |
| 2.7 | Access | dereference, select, subscript, point-to, reference, receive, range |
| 2.8 | Block | parenthesis, compound, comma, new line |
| 2.9 | Sync | lock, yield, wait |
| 2.10 | Cond | ternary, if, if-else, statement switch, expression switch, is, in |
| 2.11 | Loop | for, foreach, while, do |
| 2.12 | Jump | return, continue, break, yield, goto, case, label |
| 2.13 | Flow | go, defer |
| 2.14 | Memory | new, make, delete, malloc, free |
| 2.15 | Transform | cast |
| 2.16 | Exception | try, catch, throw |
| 2.17 to 2.19 | Call, FunctionDecl, TypeDecl | |
| 2.20 | Search | choice, fail, commit, all |
| 2.21 | Offer | outcome, recover, deadline |

- **An operator is a recipe.** The category of a recipe node is the recipe that is applied: a
  catalogue entry, an instance of one for given blueprints, or a declared function. `+` on two
  signed 32-bit integers is a node whose category is `(Instance Plus (Integer (TwosComplement
  32)))`. A call of a function is a node whose category is the function's declaration.
- **Every use has a blueprint.** A literal is a leaf under its blueprint. A local or a
  parameter is read by its place, and its blueprint is in the function's declaration. A member
  is `(Member B k)`: the blueprint and the place, so the node says of which shape it is a part.
  The blueprint of any recipe node follows from these without running it.
- **A function** is `(FunctionDecl signature locals body)`: the blueprints of its local cells in
  order, and a block. A method is a function whose first parameter is the receiver.
- **A generic recipe** is `(Template n body)` over recipes and blueprints with holes, and an
  instance of it is a recipe. The operators of the catalogue are generic in this sense.
- **Effect is in the category.** TempWrite changes a cell that no one outside the running
  function can see; Write changes one that others can. Which it is, is part of the node.

## Cells (package 3)

A cell is a present value of a blueprint, held somewhere. The kinds are Global, Local,
Parameter and Member. A declared cell has a blueprint, the recipe that reads it and the recipe
that constructs it.

- **Unboxed.** A value whose blueprint has a fixed layout is held as its bytes in the place
  that holds it: a local's place in a frame, a member's place in an object, an element's place
  in a list. The blueprint is not stored with it; it is that of the place.
- **Boxed.** A value whose blueprint the place does not fix (`Any`, an interface, a union) is
  held as a blueprint and a payload.
- **Objects.** An object's layout is computed from its blueprint: its members in order, each
  aligned, the parents' members composed in. An object that is allocated is a record in the
  region and starts, like every record there, with its blueprint and its last writer
  (`spec/framebuffer.md`), so it is seen from outside as what it is.
- **A reference** is a cell and a view of it (graph 5.5.1). A view through an interface is
  found once for a pair of interface and shape and remembered.
- **Making a cell.** `new B args` takes room for B's layout, runs the init recipe of each
  member that has one, then B's constructor on the new cell with the arguments. `make` does the
  same for a container.
- **Undo.** A write to a cell older than the newest open choice puts what it held on the trail,
  and a fail puts it back (graph 5.4.2, 5.4.9).

## Values and conversions

- `cast` between two numbers is one of five rules, named in the instance: exact, round to
  nearest even, truncate, saturate, wrap. A lens chooses the rule its language means.
- A conversion is one recipe over the layouts' parameters: the value is taken apart into sign,
  significand, power and special, and put together in the target layout by the cast's rule.
  Fixed, Affine, Block and Table are recipes over the layouts they name.
- A cast of a reference changes the view: up to a parent or an interface always, down to a
  shape only if the cell is one, and otherwise the answer is nothing.
- A cast to `Any` boxes; a cast from it unboxes if the blueprint is the one asked for.
- Arithmetic on floats is one recipe over the layouts' parameters, the same for every layout:
  the operands are taken apart into sign, significand, power and special, the result is
  computed exactly, kept to 62 bits with every bit beyond folded into the last one, and put
  together by the layout's encoder rounding to nearest even. An invalid operation answers the
  layout's quiet NaN, and a NaN operand passes on quieted, a signalling one first, as AArch64
  does. A comparison is the exact order of the two values; a NaN makes every comparison false
  but not-equal.
- An untyped number beside a typed one takes the typed one's blueprint, and an integer beside a
  float is converted by value. A value written to a typed place or returned from a typed
  function is converted the same way.

## Running

- A call makes a frame: a record in the region with the function, and its parameters and
  locals laid out by their blueprints.
- A recipe node is run by what its category says. Control is run as it is written: an if
  chooses, a loop repeats, a return leaves the frame. Nothing is rewritten first.
- An operator instance on fixed layouts is run on the unboxed values. An operator used without
  its blueprints is resolved from its operands when it runs, and the instance is remembered.
- The answer to running any node is the acknowledgment of axiom 5: nothing, 0, 1 or a node. A
  typed value is a node. A statement that completes answers 1.
- Choice, fail, commit and all are recipes of the catalogue and keep their meaning (graph 5.4).
- Each catalogue entry is bound to a recipe in form that runs it. A node whose category is a
  composite, such as an instance, runs that composite's own binding, so code specialised for
  an instance is bound to the instance node; with no binding it is data. Each binding can be
  replaced while the system runs, by machine code written from rows keyed by the instance.
  What form cannot yet do becomes an operation of the bootstrap.
- The engine is one layer. A recipe handed to a definition runs on the bindings that
  definition sees (`spec/core-operations.md`), so the parts that hand recipes to one another,
  the runtime, the float definition, the machine's rows and the compiler, are bound together.
  Each part's tables are written once the layer stands.

## Words and lenses

- The engine's own definitions are entries of package 16, numbered in the order they are
  named; 16.0.0.1 is the catalogue. The float definition is at 16.0.1, the machine's rows at
  16.0.2 and the compiler at 16.0.3.
- A symbol table is rows keyed by node and lens: the name of a nominal blueprint, of a member
  (blueprint and place), of a function, of a global, of a local or parameter (function and
  place).
- A lens for a language is rows: for each catalogue entry a template with numbered places
  (`{0} == {1}`, `if {0} {1} else {2}`), the spelling of each blueprint, and the forms of a
  declaration. One small engine fills templates from nodes to write text. Reading text is the
  same rows run the other way and comes after writing.
- The first lens that reads is the bracket notation of `spec/lens.md`, with the catalogue's
  words.

### Viewing a component declaration

The BML lexer calls the typed predicates in `nums/bml-alphabet.ck`. Both the lexer and
the BMF range matcher use `bmf.in-range` from `bmf/items.ck`. The interval has two
declared members and a positional constructor. The alphabet declares its three constant
intervals and composes the start and continuation predicates from them.

`lens/identifier.ck` prints this part of the installed model. Select Java, C#, TypeScript,
Python or BML with 0, 1, 2, 3 or 4 respectively. For example, this prints Python:

```sh
printf '3\n' | ./ck text lens/identifier.ck
```

`lens/identifier-model.ck` selects the range fields and constructor and the six function
declarations. The terminal's search method has a separate contract. In
`lens/identifier-views.ck` each row pairs a NUMS template with its five surface spellings.
The renderer in `nums/view-patterns.ck` extracts the template arguments and reapplies the
template to confirm the entire node matches. Reading the emitted spelling rebuilds the
declarations, including field order, types, formal positions and scoped template holes.
This inverse reads the emitted layouts. Each host's compiler or interpreter can also run the
host view. The TypeScript view uses `bigint` for Seed's signed integer arguments.

`ground/nums-portable.ck` round-trips the live declarations and a variant with a wider
uppercase range through all five views. It installs the variant read from BML, observes the
changed answer for U+005B, then observes the original answer after the trial.

## Templates, generics and variation

- A template is filled by putting blueprints or recipes in its holes. The result is a node, the
  same node wherever it is made, and code specialised for it is kept under its id.
- Genetic algorithms run on a population of immutable genomes. A genome may be a sequence of
  alleles or a recipe tree. A development recipe maps a genome to the value or recipe to be
  evaluated. A fitness recipe observes that phenotype on a particular task. A score belongs to
  the genome, evaluator and task together. Occurrence in the store alone gives no fitness.
- A generation selects parents, applies crossover and mutation, evaluates the offspring and
  chooses survivors. Random choices are explicit inputs so replay of the same generation gives
  the same offspring. Parent nodes persist. A typed replacement uses the required blueprint
  at the destination. A recipe with local references also needs a compatible scope.
- Selection, development, fitness and variation are ordinary replaceable recipes. A population
  can contain repeated references to one genome. This preserves selection pressure even when
  equal content shares one node.

The distinction between chromosome and expression tree follows Ferreira's gene expression
programming. DEAP's genetic programming model supplies the typed argument constraint. Its
variation operators also distinguish a changed individual from its parent's fitness.
[Ferreira](https://arxiv.org/abs/cs/0102027) ·
[Typed trees](https://deap.readthedocs.io/en/stable/tutorials/advanced/gp.html) ·
[Variation operators](https://deap.readthedocs.io/en/stable/api/tools.html)

## The bootstrap's earlier catalogue

The 75 operations of `spec/core-operations.md` are package 0. They are how the present lens,
emitter and tools are written. They stay while those are written again as recipes of this
catalogue, and go when nothing is left on them.

## What is built

The first of the steps of graph node 7.23 stands. `nums/catalogue.ck` is the catalogue as one node: every
entry of the tables above, family by family, as a catalogue entry whose number is its
coordinate, 282 in 54 families, each with its word in the lexicon. `nums/coordinate.ck` reads an
entry's number and makes the entry of a number. `ground/catalogue-at-coordinates.ck` holds
every family to one package, level and type with instances from 0, the families in order, seven
entries to the coordinates `nums_consts.go` gives them, and every entry to its number and back.
`ground/instance-runs-its-binding.ck` binds `(Instance Plus (Integer (TwosComplement 32)))` and
runs a node of it. An instance with no binding answers itself.

The following paths run on that catalogue. The ground programs with the prefix
`ground/nums-` observe their results.

- `nums/objects.ck` creates an object from its parents, members, methods and constructor.
  A field with a fixed layout occupies aligned bytes in one room labelled with the object declaration. Its declaration
  supplies the blueprint when read. An Any field holds the complete node. A cell reference
  carries the actual declaration, the visible view and the same room through each cast. Method calls, explicit parent
  calls, structural views and recipe specialization all operate on that reference.
- `nums/numbers.ck` implements the five cast policies in Form. It converts signed and unsigned
  integer layouts of 1 to 64 bits, the native fixed-width leaves and binary floating layouts
  specified by exponent width, fraction width and special-value convention. The witnesses
  include binary16/32/64, bfloat16, FP4, both FP6 formats, FP8, and unsigned E8M0. Exact casts can answer
  nothing. Integer wrap uses the target width. NaN payloads are canonicalized.
- `nums/quantized.ck` reads and writes a `Fixed` point as an integer times a power of two.
  An `Affine` layout rounds the quotient of the value and positive scale, adds its zero point
  and applies the integer policy. A `Table` layout stores an index into a numeric codebook.
  Magnitudes in `nums/magnitude.ck` are sequences of bytes. Integer products, sums and
  quotient/remainder can span multiple machine words before the target rounds.
- `nums.block-pack` takes the element values, a block blueprint, its shared E8M0 scale and
  the cast policy, with the blueprint first. The leaf holds the shared scale byte followed by
  the element bits from least significant upward. `nums.block-unpack` reads that leaf into
  the requested scalar layout. Three FP4 elements at scale two occupy the bytes `80 0a 05`
  for the values −2, 0 and 6. The caller selects the scale. The quantization and block
  conventions are described by [ONNX](https://onnx.ai/onnx/operators/onnx__QuantizeLinear.html)
  and [OCP](https://www.opencompute.org/documents/ocp-microscaling-formats-mx-v1-0-spec-final-pdf).
- `nums/genetics.ck` develops each genome and scores the resulting phenotype. A generation
  carries its specimens, random seed, development recipe and fitness recipe. Changing either
  recipe rescores the parents. The default is two-way tournament selection, a supplied
  variation recipe and generational replacement. A path can select a subtree for typed
  replacement. The caller supplies the type judgment for its program representation.
- `nums/read.ck` reads the lens path on the first line of stdin and the whole source after it.
  `nums/show.ck` takes the same path followed by a canonical node. The Go lens uses projection
  rows. The BML lens uses its BMF grammar for expressions and whole source files. Both read
  entries, leaves and composites through constructor expressions and print arithmetic directly.
  The row files `nums/go-bytes.ck` and `nums/bml-bytes.ck` project any node as a native byte
  sequence literal carrying its frozen graph. This gives a language a direct way to produce
  nodes, including references to shared children. A Go program writing the generated literal
  for `(#9999 (#46 [#4 11] [#4 19]) [#1 ff00] ^1)` produced the same 510 bytes as
  `ice.freeze`. The repository checks the round trip in Form.

- `nums/run.ck` is the generic runtime. The catalogue's recipes are bound to recipes in form,
  so a recipe node runs as it stands and the walker is at that node. A call of `(FunctionDecl
  signature locals body)` makes its frame, a room under the declaration laid out by the
  blueprints of its parameters and locals: a number of fixed layout as its bytes, a Bool as one
  byte, anything else as the handle of its node. What a slot holds for a value is one recipe,
  `run.slot-value`, which the machine's entry uses too: an integer slot holds the value's
  number, yes as 1, no as 0 and a typed integer by its own sign, kept to the slot's width; a
  Bool holds 0 or 1; a float slot holds the value converted by value. A value no slot of that
  layout takes fails the call. Block, if, the loops, return, break and continue run as written; a jump answers the
  node that says it, and a continue in a do loop goes to its test. An operator takes its
  instance from its operands' blueprint, makes the recipe for it once and keeps it under the
  instance node, and an integer of any Bits or TwosComplement width wraps at that width. An
  unsigned layout divides, shifts and compares as unsigned.
- On frames a nothing ends the call with no answer, wherever it stands: returned, passed,
  written, tested or left as a statement, a division by zero makes the call answer nothing
  with the reason `zero`. The arguments of a call are evaluated one after another and each
  keeps what it changed, so `g(i++)` leaves i one higher.
  `ground/nums-runs-as-written.ck` runs four functions written as nodes, a sum in a for loop, a
  recursion through Callee, a while that writes its parameters and a loop left by break, and two
  numbers that wrap. Its nodes hold no word and no text.
- `nums/float.ck` is the float definition above, over the magnitudes of `nums/magnitude.ck`
  and the encoder of `nums/numbers.ck`. Run in a session sweep over zeros, subnormals, the
  largest numbers, infinities and NaNs in both signs, it matched this machine's hardware bit
  for bit: 3,600 sums, differences, products and quotients each in binary16, 32 and 64, and
  6,264 comparisons. For bfloat16, E5M2, E4M3 and E4M3 with an unsigned zero it matched the
  hardware's binary32 result rounded back, except that it keeps a NaN's payload and names an
  overflow `range`.
- `nums/arm.ck` is the AArch64 encoder as a component package, loaded with `part.load`. A
  `Field` is an offset, a width and a kind, unsigned, signed or a condition taken inverted, and
  says whether a value `fits` and where it is `place`d; a `Format` holds its fields and `pack`s
  the operands into a base word, failing when one does not fit; an `Encoding` is an opcode, a
  base word and a format. The fields and formats the rows share are named (`rd`, `imm12`,
  `three-registers`, `register-pair`, ...). The encodings, the condition codes (`Condition`: a
  name, a number and the inverse) and the target's facts (`Fact`: a role and its value, such as
  which registers hold the region, the count, the answer and its kind, the arguments, the temps
  and the homes of each family, and how the stack floor is read) are rows, and each row keeps
  itself where the compiler reads it. `encode`, `bytes` and `pieces` are functions. The word an
  encoding gives an instruction is kept under the encoding and the instruction, so an
  instruction is packed once.
- `nums/compile.ck` writes AArch64 for a whole declared function from the same nodes the
  generic runtime runs. It is a component package, loaded with `part.load`:
  - A `Compilation` is what lowering a node carries: the `Routine` (the declaration, the homes
    of its parameters and locals, how many parameters there are, the declared result and the
    `Frame`), its `Exits` (the labels out, decline and early, where the stack floor is
    reached), the innermost `Loop` and the depth. Lowered code is a `Code`: the class of the
    value it leaves and its items, each a word, a `Mark` of a label or a `Jump` that names one.
  - Kinds vary behind interfaces. A `Lowering` for each kind of node is kept under its category
    by a table of `Rule` rows; the operators, compound writes and steps are built from the
    runtime's own tables. A `Family` of values, `Whole`, `Truth` or `Real`, loads a constant,
    answers a value and says where its temps spill. An `Operation` for each operator writes its
    words: an `Arithmetic` with its templates per column and its immediates, a `Relation` with
    its conditions. A `Crossing` for each pair of families converts and casts between them, and
    a `CalleeForm` finds the declaration a call names.
  - Knowledge is rows, kept where the compiler reads them: each family with its bank of
    registers, its rank and its templates; each operator with the families that admit it; the
    float formats, the cast rules, and the layouts of a function, the door, a call, a branch
    and each loop. A template is a list of `Piece`s: an `Instruction` whose holes are named
    roles, a `Mark`, a `Jump`, a `Splice` of items bound by name or a `Fragment` of a shared
    guard. When a template is kept it is specialized to the target's facts, and an instruction
    with no role left is kept as its word.
  - The encoder's `encode`, `pieces`, `invert` and `bytes`, the runtime's number of a literal
    and its float encoder reach the package as `Services`, which `nums/run.ck` hands to
    `jit.setup` with the runtime's tables of operators, compound writes and steps.
  - Records have named parts. A rule binds every part it builds before it puts them together,
    so a part that does not lower fails the whole function and never leaves a gap in its code.
    Code is laid out with fresh labels and jumps that name them, resolved once the words stand.
  - Where there is more than one way, ordered choice takes the first that holds: a constant is
    one move, an inverted move, or a move and up to three keeps; an immediate is tried before
    a register. An operand is compiled once and both forms are chosen over its code, so the
    time to compile grows with the size of a function and not with the depth of its nesting.
  - A lookup in a table fails, so a construct the rows do not cover refuses the function. A
    word that cannot be encoded, or a target fact, label or template that is not there, is the
    fault `jit.broken`, a mistake of the compiler. Under `(run.refused declaration)` is noted
    the category of a construct no rule takes, or, when the function was refused for another
    reason, only that it was refused; when compiling faulted, the fault's reason.
  - A node compiled at depth d leaves its value in the temp at d of its class and may use the
    temps above it; the temps below d hold values still waiting, and a call saves them beside
    the frame. Compiled code changes nothing outside its own frame and registers before it
    answers; the notes it writes are kept back until then, so a decline can run the whole call
    again on frames.
  Parameters and locals stay unboxed in registers for the whole call, integers and hardware
  floats each in their homes, values being computed in temps saved beside the frame across a
  call. Integers are kept at their width after every operation. Where the runtime would
  answer nothing or fault, a division by zero or by −1, a shift outside 0 to 63, a callee that
  declined, the code declines and the whole call runs on frames, so the machine's answer is
  the runtime's or there is none. A function's first call asks the compiler held in
  `run.compiler` and keeps what it gives under the declaration; a fault while compiling is
  caught, noted and answered by frames. A float instance's machine form is the compiler applied
  to the function that returns the operator on its two parameters, and it is given only what
  its form would take: numbers of its layout, ints and integer layouts.
- A function compiles together with every declaration it calls into one leaf: the callee is
  the running function (`r.callee`), a declaration written in the call, or an entry given one
  by `run.declare`. Each has its own frame, and a call is a branch with its arguments in x0 to
  x7 and its answer in x0 and x1, so two declarations that call each other stay in machine
  code. Between declarations an argument is converted to the callee's parameter as a slot
  converts it, a whole number extended by its own sign; a return between two different integer
  layouts, and a conversion into int, which frames keep as a node, are not compiled. A callee that
  does not compile fails the whole leaf. `run.declare` counts the declarations of entries, and
  code compiled before the newest one is compiled again at its next call.
- `(r.cast value B rule)` compiles between integer layouts and the hardware float layouts. Each
  of the five rules is a row of how it rounds and what it does beyond B's range: exact,
  nearest and truncate decline there, saturate clamps, wrap keeps the low bits. A float is taken
  to binary64, rounded and bounded there before it becomes an integer. Where the hardware would
  answer otherwise than the runtime, a NaN, a value beyond 64 bits, an inexact narrowing under
  exact or truncate, an infinity under saturate, the code declines. Wrap into a float, which the
  runtime faults, and an exact or truncating conversion into a float too narrow for every value
  of the integer layout, do not compile.
- With `run.reference` held, every call and every float operator runs on frames all the way
  down. `ground/nums-compiled-agrees.ck` runs each of its cases that way and as machine code
  and answers the cases whose outcomes differ:
  - every class, int and Bool among them, as result and as parameter, over six shapes: the
    parameter, a sum, an equality, a bit and, a ternary, and a local of the result's class;
  - each kind of value, a number, yes, no, a typed integer, a float and a NaN, passed to each
    class;
  - a literal right operand of every size for each operator, NaN, the infinities and −0, and a
    product nested twenty deep;
  - every operator on six integer and two float layouts through a call of a second function
    that takes its arguments swapped, over fourteen boundary values on both sides;
  - every cast between nine layouts by each rule over the same values;
  - thirteen edges, among them returns between widths, a continue in a do loop, a literal
    beyond 64 bits, an argument that changes a local, and a nothing as a condition and as an
    argument;
  - the encoder, against the rows and recipe it replaced, kept in the program as its
    reference: every instruction row with each field at its smallest and largest value, one
    past each, every condition and four operands that are no field's number, with one operand
    fewer and one more; every inversion, the pieces of eight constants, the bytes of two lists
    of words and the target's facts. Two encodings agree when they give the same word, both
    refuse or both fault for the same reason.
  Its expectation holds that none differ, that some compiled and some were answered by the
  machine, and that every planned encoding was compared; on 2026-11-26 that was 37,822 cases,
  30,269 compiled and 27,108 answered by the machine code, the rest declined, and on 2026-11-27
  3,742 encodings over the 60 rows, of which 2,436 gave the same word, 1,145 were refused by
  both and 161 faulted in both on an operand beyond 64 bits. It runs with the other ground
  programs and ends holding 57,464 nodes, since a fail gives back what it returns past.
  `ground/nums-compiled.ck` runs fib on frames
  to 15 and as machine code to 30, Newton's root of 2 in binary64, a harmonic sum to 1,000 in
  binary32, and the area under x² by 1,000 rectangles, each a call of a second declaration,
  cast to an integer by nearest. Beyond the 9 ms that loading and compiling take, a million
  rectangles took about 1 ms as machine code; a thousand took 159 ms on frames. It also calls
  a declaration quoted in the call, compiled into the same leaf, and a function that uses
  `r.callee` as a value, which no rule takes: it runs on frames and its refusal notes
  `r.callee`.

Lowering whole BML packages to executable declarations, a full Go grammar, execution of the
rest of the recipe catalogue and regeneration of the bootstrap remain open. Members and objects run neither on
frames nor in compiled code. The compiler takes no more than six parameters at a call from
outside and eight between declarations, and does not write GPU code, unroll loops or trace
compiled code; a function it refuses runs on frames.

## BML and its BMF character cursor

The BML productions are numeric nodes in `nums/bml.ck`. Each production refers to other
productions by number. The character matcher in `bmf/cursor.ck` serves both those productions
and the rules a BML program declares inside `syntax { … }`. The language-specific terminals
and the translation to NUMS live in `nums/bml-reader.ck`.

### Parse items and the search protocol

`bmf/items.ck` is a NUMS package of declarations. `bmf/cursor.ck` loads the package and
its execution support. The package runs independently of BML. The JBMF `ParseItem.match`,
`next` and `undo` protocol appears as one method, `bmf.match-item`. Its receiver is the
item. Its other parameters are the parser, the scalar vector, the cursor position and the
active rule path. Each answer is a pair of captured nodes and an end position. Asking for
the next answer resumes Form's choice point. Failing back over the answer restores trailed
writes. `bmf.match` exposes those ordered answers. `bmf.run-stream` selects the first
complete rule result.

A bounded repetition declares three members, an element and two integer bounds. Its
three-hole constructor template builds the value `(bmf.between element lower upper)`.
Its method invokes the repetition algorithm shared with unbounded repetition. The algorithm
tries another repetition before yielding the current match. It visits the child's answers
in their declared order and carries each answer's captures into the recursive call.
The range terminal declares its bounds and implements its own method. A range reads one
scalar and emits that scalar only when it falls within the closed interval.

A grammar can declare its own `b.object` with `(b.interface bmf.item)` and a typed
`b.method` for `bmf.match-item`. `nums.construct-value` constructs the immutable value
from the declared fields and constructor. Sequence, branch and exact-string nodes take
variadic fields. An operand can be an item, rule number, terminal word, literal or formal
slot. `bmf.external` passes an application's terminal value to the parser's lexer.
`ground/bmf-items.ck` gives a complete example of a grammar-owned class.

The NUMS reference in `ground/bmf-reference.ck` pairs a spelling with its capture tree.
For two items, sequence takes the ordered product of their paths and branch appends their
path lists. Filtering those spellings against an input prefix gives the expected captures
and end positions, including duplicate answers. `./ck run ground/bmf-sweep.ck` reports the
planned count, the count each evaluator completed, and each differing case with its expected
and actual answers. `make check-full` requires complete agreement for the generated cases.

### Execution of a search declaration

A package has the blueprint `(b.instance b.package r.search)`. Each `r.type` associates
a resolved identity with its declaration. Loading declares the whole package before
installing methods, so one declaration can refer to another by id. `nums.load-search` takes
the package and the evaluator for its methods. With `nums.search-direct`, the body stays
as NUMS. Form visits the selected expression at the point of use with the caller's slots
and cut scope. With `nums.search-function`, the loader specializes the same operations for
the whole body once. This is the default for a package loaded by `step`. Both modes share
the operation table in `nums/search.ck`. The signature and locals determine
the places available to `r.parameter` and `r.local`. A scoped `r.local-decl` binds a
value for its body. `r.choice`, `r.fail`, `r.commit` and `r.all` use Form's search.
`r.make` evaluates its fields as call arguments and constructs a value for each combination.
The grammar package and the class installer in `nums/classes.ck` use this same representation.
The ordinary NUMS frame engine serves procedural functions.

`nums/classes.ck` derives constructors and method dispatch from the declared shape.
A method must have the result and argument types of its interface with the concrete
receiver in the first parameter position. The declaration stays available for a lens after
installation. In a local Java projection, the range declaration produced a two-field record
implementing the projected interface. Its compiled method and Seed agreed on the capture
and end position for eleven boundary cases, including EOF, U+FFFF and U+1F600. That
projection covers the scalar terminal and its cursor accessor. A Java backend for the
whole search package remains open.

The cursor is a position between Unicode scalar values. The first position is zero. A choice
saves that position. Failure can return to it across a token, a comment or a newline. BML
terminals skip whitespace and comments, including nested block comments. An embedded BMF
rule sees each character, including spaces. `EndOfLine` consumes LF or CRLF. CRLF occupies
two cursor positions. `EndOfFile` succeeds at the end of the character stream.

| BMF element | What the matcher does |
| --- | --- |
| Sequence and ordered choice | Continue each match into the following element. Try the next alternative when a later element fails. |
| Optional and repetition | Optional offers the child's answers before the empty answer. Repetition extends each path first and can give characters back until the suffix matches or the minimum is reached. An empty match counts toward the minimum, carries its captures, and ends that path when the minimum is satisfied. |
| `Fail` | Resume the nearest available choice. Form restores trailed cell writes on that path. |
| `Cut` | Succeed going forward. A retry across this position ends the parse with the numeric cut reason and the saved position. |
| `Stop` | End the parse immediately with the numeric stop reason and the current position. |
| `MultiMatch(part, retries)` | Offer the first match and at most that many retries. Close the operand's choice points as its last allowed answer leaves. Enclosing alternatives remain available. |

Each match carries the characters or production nodes it captured and the position where it
ended. Sequence and repetition share immutable capture groups while a choice is open. They
deliver the groups in source order when they yield an answer. The active rule calls form a
separate immutable chain. Re-entering the same rule with the same arguments at the same
character position fails that branch. Recursion after consuming input and nested invocations
with different arguments remain available. Grammars that grow a result through left recursion
still need to express that growth as repetition or consuming recursion.

`bmf.on` associates a pattern kind with a numeric recipe in a trailed cell. The recipe takes
the parser, pattern, scalar stream, position and active rule chain, and yields matches in order.
A literal entry is registered as `(bmf.atom entry)`, so a terminal and a blueprint may
share the same number. An unregistered terminal goes to the parser's lexer. In
`ground/nums-lenses.ck`, an alternate range matcher accepts the character `2` through an existing word rule. Leaving the trial restores
the original range matcher and the same word rule declines that input.

Urs C. Muff describes these cursor, branch, repeat and cut semantics in *Backtracking Model
Languages* (2000), pp. 10–13. The separate immediate `Stop` follows the later kernel. Form's
scoped `first` closes local alternatives when a grammar production explicitly commits to its
first match. It also chooses the first complete parse at the public entry point.

The thesis and the original JBML grammar supply packages, interfaces, classes with multiple
parents, templates, sections, properties, declarations, expressions and statements. The
`section [form.bml]` productions cover the later kernel's blueprints, methods with contracts,
class members, collection literals, slices, lambdas, match arms and pipelines. The enclosing
grammar selects associativity. At the root, `10 - 3 - 2` follows the thesis's expression
example on pp. 23–25 and evaluates to 9. In a `form.bml` definition it evaluates to 5.

Reading a source file returns a syntax tree with numbered productions and typed token leaves.
Showing it emits normalized spacing. Reading that spelling reconstructs the tree.
`bml.recipe` turns an untyped, expression-bodied `form.bml` definition in the supported
operator subset into a recipe, resolving parameters to argument positions.

For a parsed `form.bml` blueprint, `bml.blueprint-decl(syntax, resolve)` produces an
`r.type` containing the ordered `b.member` declarations and a `b.template` constructor.
The resolver receives a scope and a spelling. Type names resolve at the root and members
resolve under their owner. Qualified names and nested type applications are supported.
An unresolved or ambiguous name, duplicate member identity or array suffix produces a
link fault. The runtime receives the resolved declaration.

`bml.class-decl(grammar, syntax, blueprint, resolve)` extends a resolved object declaration
with the methods of a parsed `class`. Existing members, interfaces, methods and the
constructor keep their order. For example, a class can add this method to the BMF
range blueprint and keep using the same range in a grammar.

```bml
class ScalarRange {
  def fits(n: Int): Bool = low <= n && n <= high;
}
```

A method declares its parameter and result types. Its body is an expression or a block
with sequential `let` bindings and a final expression, with an optional semicolon. A local can infer its type or
declare it. A use resolves first to a local, then a parameter, then a receiver field.
The result is a `b.method` with a typed `r.function`. The receiver occupies parameter zero.
`self.high` selects the receiver's field even when a parameter is also named `high`.
A parameter of the enclosing class type can supply the receiver as in `other.high`.
Field reads carry the resolved owner and field slot. Consistently renaming parameters and
locals gives the same numeric declaration. Applied types such as `Box<Int>` retain their
type arguments through linking and execution.

The method linker covers integer addition, subtraction, multiplication, bitwise AND and OR,
right shift, `<`, `<=`, same-type equality, and boolean AND and negation.
`ground/nums-portable.ck` runs the resulting methods directly and after specialization.
It projects linked classes through `write.text` with `nums/lang-bml.ck`, parses the output
and compares the entire linked declaration. The comparison includes a nominal receiver and
`Pair<Box<Int>, Int>`. Source spellings live beside the declarations.
Class type parameters, inheritance clauses, method calls in BML bodies and automatic
whole-package linking remain open.

`bml.bmf` links a syntax block's rule references and formal parameters to numbers. Its
linker indexes all rule signatures before resolving bodies. A reference carries its rule number
and a formal parameter is a NUMS template hole. A generic rule stores a
`b.template` over its production. Invocation applies the shared `nums.apply-template`. Repeated formal names have no unique slot
and produce a link fault. Given a start rule and source text, `bmf.run` executes the result.
For example,
`List(item, separator) ::= item { separator item };` accepts another rule and a literal as
arguments. The count syntax follows `Java/JBMF/library/BMFBasis.BMF` in the Angelic archive.
`2{ Blank }3` takes two to three matches. Either bound can be omitted. Character ranges,
literal strings, grouping and the control elements above execute.
`nums/bml-link.ck` resolves capture names while it links a syntax block. Each distinct
name gets its ordinal at first occurrence. Thus `$name:item` wraps the item's captures
under `(bmf.label k)`, and `:item` wraps them under `(bmf.label)`. An anonymous tag
uses no ordinal. Tags have their own scope alongside rule names and formal parameters.
Consistently renaming the tags gives identical linked rules.

```bml
syntax {
  Expr ::= Term { $op:('*' | '/') Term };
  Term ::= :< '0'..'9' >;
}
```

On `12*3/4`, the captures for `*` and `/` share label zero. The first term's anonymous
capture contains both digits of `12`. `ground/bml-captures.ck` runs this example and
compares the linked grammar with independently written numeric rules.

At the numeric API, `bmf.negated` performs zero-width negative lookahead,
`bmf.tagged` groups a match under a numeric label, and `bmf.action` calls a resolved
function from capture list to capture list. A transformation can offer several answers.
Source linking for capture actions and literal whitespace annotations remains open.
`ground/nums-lenses.ck` reads, shows, links and runs embedded rules in the same
process as a BML definition.

The numeric entry `bmf.run-stream` takes linked rules, a start rule and a vector of Unicode
scalars. A standalone image of the BMF package parsed `Alpha,B_e,C` with a parameterized
list in a directory containing only the image. Its reply carried the same tree id as the
authoring process. The image contains 1,747 records and zero text leaves.
A separate image linked from BML matched `aa` through a tagged, parameterized rule.
Its 1,727 records contained zero text leaves, and invoking `bmf.run-stream` by id returned
the tree id predicted by the numeric grammar. Under `make check` the numeric port constructs
a two-digit capture through a rule template. The arm64 requests also call the UTF-8 width function by id,
compile its recipe and call the same id on a four-byte lead.

The public lens also reads a thesis expression from stdin.

```sh
printf 'nums/bml.ck\n10 - 3 - 2\n' | ./ck text nums/read.ck
```

## Writing a package in a language

A component package is shown in a language by one writer and that language's rows. The writer
is itself a component package, `nums/write.ck`: classes for its context, rows and template
parts, and functions that answer the pieces of text in order. Each language is a file of rows,
`nums/lang-python.ck`, `nums/lang-csharp.ck`, `nums/lang-typescript.ck`, `nums/lang-java.ck`
and `nums/lang-bml.ck`: a template with numbered places for each operation, declaration and
statement, the spelling of each blueprint, a case for each kind of name, the reserved words and
how the language answers a search.

A typed byte leaf in a component projects as a call to `pack` with its blueprint and payload
bytes in order. For a composite literal, the writer reconstructs its blueprint and children
recursively. This preserves instruction bytes, including leading zeros, in every language view.

```sh
printf 'nums/write.ck\nnums/lang-python.ck\n' | ./ck text nums/write-text.ck
```

- **Names are beside.** A package is written with names (`write.named`, parameters and locals by
  name); `nums/resolve.ck` turns them into places and gives the names as rows keyed by declaration
  and method, kept in `nums/names.ck`, and puts them back (`write.named-package`). For each of
  `nums/write.ck`, `nums/part.ck` and `nums/resolve.ck`, putting the names back and resolving
  again gives the stored nodes, so a package is edited through its named view:

  ```sh
  printf 'nums/part.ck\n' | ./ck text nums/named-view.ck > part.txt      # edit part.txt
  printf 'part.txt\n' | ./ck text nums/resolved.ck > nums/part.ck
  printf 'part.txt\n' | ./ck text nums/names-merged.ck > names.ck && mv names.ck nums/names.ck
  ```
 A member's name is its `b.member` entry, or the
  part names the lexicon gives an entry (`r.type` has a name and a shape). A local that would take
  the name of a function of the package gets a suffix, as does a reserved word.
- **How often a recipe answers.** The writer finds for each function and method whether it answers
  exactly once, at most once, at least once or any number of times, two bits joined over the
  package until they stand. A function that answers exactly once is written as statements that
  return; any other is written as the language's search: generators in Python, C# and TypeScript
  (`yield`, `yield from`, `foreach … yield return`, `function*`), `Stream` with `flatMap` in Java,
  and Form's own choice in BML. `all` over a search is a comprehension in Python, a LINQ query in
  C#, a generator in TypeScript and a stream chain in Java. A first answer is `next(…, fallback)`,
  `FirstOrDefault`, `findFirst().orElseGet(…)`. An `if-else` commits to its condition's first
  truth; a condition that is not a comparison is tested by Form's truth, where only no and nothing
  are false.
- **BML** is written as `section [form.search]`, the old kernel's `[form.bml]` read as Form reads
  its nodes; `nums/bml.ck` takes both. Its first lines say so: `choice { a; b }` answers each
  answer of a and then of b, `fail` answers nothing, `let` binds each answer in turn, `first` and
  `all` keep the first answer or every one, `a ?? b` is `first(choice { a; b })`, a `match` is a
  ladder on one subject, and declared types are documentation. A blueprint holds a class's data
  and a class its methods, whose receiver is `self` and no parameter. A def that does not answer
  exactly once is preceded by a note saying how often it answers. The writer's own package and
  the encoder's, written as BML, read back into syntax trees with the BML reader; no reader yet
  lowers their searches to recipes.
- **A method is called on its receiver.** `part.load` (`nums/part.ck`) binds each method a package
  declares to a recipe that calls the receiver's own method (`nums.method-key`), in one layer with
  the package's own declarations, so that a method can call the package's functions and they its
  methods. A call `(r.call M receiver …)` dispatches by the receiver's blueprint and is written
  `receiver.m(…)`.
- **Constants and layout.** A function without parameters whose body is a literal is written as a
  constant, and a number it names is written by that name (`LOWER_A <= letter`). Each function
  numbers its own fresh names, and a loop over a list named in the plural takes the singular
  when it is free. A call or list whose flat text passes the language's width (`write.width`)
  puts its arguments one per line.

`ground/lens-round-trip.ck` writes the writer's package in all five languages and reads the BML
back, and the encoder's and the compiler's packages are written the same way. On this machine the
Python parses with Python's own parser; Deno, javac and Roslyn check the TypeScript, Java and C#
against stubs of the runtime and still refuse parts of them. A record takes as many fields as
memory holds: its fields are the arguments of one call, and the walker holds a call's values in
the region. The writer does not write a call of another package's function, since it knows how
often only its own package's functions and a table of operations answer (`e.unbound` of
`write.answers`); a package reaches another through recipes it is handed. Where an expression is
expected the writer takes no throw, Java no `let` and BML no sequence; the BML reader takes a
class of one interface only.
