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

## Words and lenses

- The engine's own definitions are entries of package 16, numbered in the order they are
  named; 16.0.0.1 is the catalogue.
- A symbol table is rows keyed by node and lens: the name of a nominal blueprint, of a member
  (blueprint and place), of a function, of a global, of a local or parameter (function and
  place).
- A lens for a language is rows: for each catalogue entry a template with numbered places
  (`{0} == {1}`, `if {0} {1} else {2}`), the spelling of each blueprint, and the forms of a
  declaration. One small engine fills templates from nodes to write text. Reading text is the
  same rows run the other way and comes after writing.
- The first lens that reads is the bracket notation of `spec/lens.md`, with the catalogue's
  words.

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

The 69 operations of `spec/core-operations.md` are package 0. They are how the present lens,
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
- `nums/go.ck` and `nums/bml.ck` are rows for one reader and printer. Arithmetic has a direct
  spelling. The constructor spelling covers entries, leaves and composites, including a
  composite category. To run the reader, supply a row-file path on the first line of stdin
  and an expression on the following lines to `./ck text nums/read.ck`. The corresponding
  writer is `./ck text nums/show.ck`. Both return their output directly to the caller.
  The row files `nums/go-bytes.ck` and `nums/bml-bytes.ck` project any node as a native byte
  sequence literal carrying its frozen graph. This gives a language a direct way to produce
  nodes, including references to shared children. A Go program writing the generated literal
  for `(#9999 (#46 [#4 11] [#4 19]) [#1 ff00] ^1)` produced the same 510 bytes as
  `ice.freeze`. The repository checks the round trip in Form.

Full-language declaration and statement grammars, execution of the rest of the recipe
catalogue and regeneration of the bootstrap remain open.
