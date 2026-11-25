# NUMS.Go core package: study report

Study date: 2026-11-24. Subject: `NUMS.Go/nums` (every `.go` file, `go.mod`, `sample id space.txt`), `NUMS.Go/sample`, `NUMS.Go/test`, and the git history of that repository.

Status words used throughout:

- **observed**: I read the cited line or ran the cited command in this session.
- **inferred**: my reasoning from observed things (the basis is named).
- **assumed**: not verified.

Nothing in `NUMS.Go` was modified. `git status --short` before and after shows only two untracked `.DS_Store` files that were already there (observed, both runs). All experiments ran on a copy in the session scratchpad: `scratch/numscopy`.

---

## 0. The short version

1. `nums` is a **static representation** of Go source in a numeric space, plus a partial printer back to Go text. It does not execute anything. There is no evaluator, no interpreter, no code generator, no persistence. (observed: no such code exists in the 16 Go files; `Cell_t` at `nums/reflect.go:3-6` is a two-field struct that nothing uses.)
2. The numeric space is a four-part ID, `Package.Level.Type.Instance` (`nums/nums_nodes.go:18-24`), a fixed catalogue of leaf and operator IDs at levels 1 and 2 (`nums/nums_consts.go`), and hash-consed composite nodes at level 3 and above stored as strings like `0.2.3.1+0.1.11.1+0.1.11.2` (`nums/nums_db.go:55-80`, `nums/nums_nodes.go:866-886`).
3. Names and literal values are **not** in the tree. They live in side tables keyed by small ordinals (`nums/nums_symbols.go:9-15`, `:85-103`). I verified by experiment that renaming every identifier in a file leaves both node databases byte-identical (section 6.3).
4. It does **not build as checked in**: both `go.work` and `nums/go.mod` point at a sibling directory that is absent. With that one `replace` line removed and the fork-only `verilog` import removed (four lines, in a copy), it builds and vets clean against the pinned upstream binding found in the local module cache (section 7).
5. Once built, it **completes** on small Go files and **panics with a nil dereference on all three sample files shipped in the repository** (section 7.3).
6. The files named `emit*.go` are not text emitters. They are the builder that emits *into* the numeric space. The printer back *out* to Go text is `RecipeWriter_t` in `nums/recipe.go:88-351`, and its only call site in the package is a log line (`nums/emit.go:370`). (observed)

So the accurate reading is: **the idea is demonstrated; the implementation is a 2023 prototype that covers a core subset of Go**. The mission context calls NUMS.Go a "100%-working reference". For this package, at this commit, that is not what the code and the runs show. Section 8 separates proven from sketched precisely.

---

## 1. Provenance and dates

| Fact | Source | Status |
|---|---|---|
| 60 commits, first `bd100c6` on 2023-02-01, HEAD `9e6bbb4` on 2023-04-05 ("current state") | `git -C NUMS.Go log` | observed |
| The `nums/` directory was last changed in `ec1f742`, 2023-03-17 | `git log -- nums` | observed |
| The work after 2023-03-17 is all under `http/` (a second design: labels, id-scopes, a "new API"), about 15,000 lines | `git log --stat`; `wc -l http/model/*.go http/*.go web-api/*.go` gives 15059 | observed |
| The Go code is a port of an earlier C++ NUMS | commit `b508f08` 2023-02-08 "translate C++ NUMS Go handlers"; commented-out C++ at `nums/nums_db.go:224-272` | observed (commit subject and remnants); the C++ original itself was not seen |
| Go package import was marked non-functional by its author | commit `e2984e2` 2023-02-09 "adding import for Go (non-functional)"; stub at `nums/parser.go:505-525` | observed |
| `nums/` holds 7,045 lines of Go in 16 files; `go.go` is 1,502 of them, of which lines 1209-1423 are a table of grammar symbol numbers | `wc -l nums/*` | observed |
| No tests in `nums/` | no `_test.go` file present | observed |
| `http/` and `web-api/` do not import `[org]/nums` | grep over those trees returned nothing | observed |

The `http/` tree was outside the assigned scope. I looked only far enough to say what it is: a tree-sitter-symbol-to-label mapping driven by YAML (`http/config/Go.yaml`, 282 lines; `http/config/labels.yaml`, 298 lines), with its own model. It is the direction the author was moving after `nums/` stopped changing. (observed existence; contents not studied)

---

## 2. The numeric space

### 2.1 The ID

```
type NodeID_t struct { Package NodePackage; Level NodeLevel; Type NodeType; Instance NodeInstance }
```

`nums/nums_nodes.go:11-24`. Widths: `uint16`, `uint16`, `uint16`, `uint32`, so 80 bits. Text form is `P.L.T.I` (`:55-57`), parsed back by `NewNodeID` (`:26-53`). (observed)

The same four words appear in the fixed axioms: `(a-cell-is (package level type instance))` at `coherence-kernel/axioms/core-axioms.form:59`. (observed) That the axiom's shape descends from this struct is **inferred** from the identical field names and order.

There are two ID spaces with the same struct: `RecipeNodeID_t` (`:91`) and `BlueprintNodeID_t` (`:640`). The same numbers mean different things in each. `1.2.2` is "Bool" as a recipe (`nums_consts.go:60`) and "Numeric Integer" as a blueprint (`:538`). Which space an ID belongs to is carried by the Go type, not by the number. (observed)

### 2.2 Level

`nums/nums_consts.go:5-29`:

| Level | Name | Meaning |
|---|---|---|
| 0 | Undefined | `0.0.0.0` is "no node" (`nums_nodes.go:89`) |
| 1 | Trivial | leaves: literals, and references into symbol tables |
| 2 | Basic | operators and structural kinds, a fixed catalogue |
| 3 to 21 | Complex_1 to Complex_19 | composites, minted at run time |

For a composite, level is the height of the tree: one more than the largest of its category level and its children's levels (`get_level`, `nums_nodes.go:814-830`), with a floor of 3 for anything that has children (`nums_db.go:168-170`, `:184-186`). So `a + b` is level 3, `c = a + b` level 4, and the function body in the trace below is level 6. (observed in code and in the run)

One exception that matters: "reserved" nodes (function declarations, structs, interfaces, packages) get their ID while still empty, at level 3, and keep it when children are added, because a non-zero `Self.Level` wins (`nums_nodes.go:815-817`). So a function declaration is `1.3.0.N` no matter how deep its body is. For reserved nodes, level does not mean height. (observed)

### 2.3 The fixed catalogue, recipes

Level 1, type (`nums_consts.go:56-80`). Instance is either a fixed value or an ordinal into a symbol table:

| Type | Name | Instance means |
|---|---|---|
| 1 | Null | 0 |
| 2 | Bool | 0 false, 1 true (`nums_nodes.go:283-297`) |
| 3 | Integer | ordinal into the recipe's `Integers` table |
| 4 | Decimal | ordinal into `Decimals` |
| 5 | Imag | ordinal into `Decimals` (shared; `emit_recipe.go:331-335`) |
| 6 | String | ordinal into `Strings` |
| 7 | Literal | ordinal into `Literals` |
| 8 | BuiltIn | declared, unused |
| 9 | Local_Decl | ordinal into `Locals` |
| 10 | Local_Access | same ordinal as the declaration |
| 11 | Parameter | position + 1 |
| 12 | Member | module-wide member ordinal (`nums_symbols.go:185-195`) |
| 13 | Global | module-wide global ordinal (`:136-146`) |
| 14 | Blueprint | ordinal into the recipe's `RecipeBlueprints` |
| 15 | Label | declared, unused by the Go handlers |
| 16 | Empty | declared, unused |
| 17 | Jump | 1 return, 2 continue, 3 break, 4 yield (bare, no operand) |
| 18 | Exception | declared, unused |

Level 2, type (`nums_consts.go:125-150`) and instances:

| Type | Name | Instances | Lines |
|---|---|---|---|
| 1 | Compare | `==` `!=` `>` `<` `>=` `<=` `<=>` `===` `!==` `~==` | 192-205 |
| 2 | Logic | `!` `&&` `\|\|` | 221-227 |
| 3 | Math | `+` `-` `*` `/` `//` `%` | 236-245 |
| 4 | BitMath | `~` `&` `\|` `^` `<<` `>>` `<<<` `>>>` `&^` | 257-269 |
| 5 | TempWrite | same 21 instances as Write, target has no global or member in it | 402-426 |
| 6 | Write | `++x` `--x` `x++` `x--` `=` `+=` ... `<-` (send) | 402-426 |
| 7 | Access | `*` `.` `[]` `->` `&` `<-` (receive) `range` | 379-389 |
| 8 | Block | `()` `{}` comma newline | 307-314 |
| 9 | Sync | none defined | 138 |
| 10 | Cond | `?:` if, if-else, statement-switch, expression-switch, is, in | 284-294 |
| 11 | Loop | for, foreach, while, do | 506-513 |
| 12 | Jump | return, continue, break, yield, goto, case, label (with operand) | 324-334 |
| 13 | Flow | go, defer | 347-352 |
| 14 | Memory | new, make, delete, malloc, free | 360-368 |
| 15 | Transform | cast | 523-527 |
| 16 | Exception | try, catch, throw | 491-497 |
| 17 | Call | 1 | `nums_nodes.go:312` |
| 18 | FunctionDecl | 1 | `nums_nodes.go:311` |
| 19 | TypeDecl | declared, unused | 149 |

The catalogue is visibly drawn from many languages at once, not from Go: `<=>`, `===`, `//`, `>>>`, `->`, `?:`, `yield`, `malloc`, `try/catch` are not Go. The comments name C, C#, Python and C++ constructs (`:133-146`). (observed) That the catalogue was meant as a cross-language operator alphabet is **inferred** from that.

The split between Write (6) and TempWrite (5) is a small effect marker: an assignment is TempWrite when its target touches no global and no member (`emit_recipe.go:49-60`, `:392-397`, `go.go:627-630`). (observed)

### 2.4 The fixed catalogue, blueprints

Level 1 (`nums_consts.go:104-113`, `:534-548`): Void `1.1.0`; Numeric `1.2.x` with Bool 1, Integer 2, Decimal 3, Imag 4, Complex 5, Char 6, String 7. Control, Runtime, Resource and Variadic are named with a leading underscore and all their instances are commented out (`:565-608`). The name tables still list them (Mutex, Thread, GPU, Tensor, Pixel, and so on, `:550-620`), which shows the intended reach. (observed)

Level 2 (`:175-182`, `:622-701`): Reference `2.1.x` (Pointer 1, Reference 2); Container `2.2.x` (List 1, Dictionary 2, Object 3, Package 4, Interface 5, Any 6; Matrix, Location, Set, Template, Union commented out); Recipe `2.3.x` (Function 1, MemberFunction 2, Local 3, Parameter 4, Global 5); Data `2.4.x` (Channel 1). (observed)

### 2.5 Edges

A blueprint's children are not bare IDs. Each is an ID plus a bitmask, written `ID^mask` (`nums_nodes.go:743-764`). The mask has 23 bits (`nums_consts.go:781-807`): Const, Static, Public, Atomic, Operator, Virtual, Override, Abstract, Mutable, Input, Output, Attribute, Constructor, Destructor, Friend, Unsafe, RefCnt, Box, Size, Embed, Parent, Function, HasDefault. So "public", "embedded", "parent" are properties of the *reference to* a blueprint, not of the blueprint. Recipe children are bare IDs with no edge. (observed)

Only Public, Function, Abstract, HasDefault, Const, Static, Parent and Embed are ever set by this code (`emit_blueprint.go:23-35`, `emit.go:81-102`). The other fifteen are vocabulary. (observed)

### 2.6 What a composite node is

`RecipeNode_t { Self, Category RecipeNodeID_t; Nodes []RecipeNodeID_t }` (`nums_nodes.go:727-731`) and the blueprint twin with edges (`:780-785`). A composite is **a category from the catalogue plus an ordered list of child IDs**. Serialized (`:866-886`):

```
<category>+<child>+<child>...            recipes
<category>+<child>^<edge>+<child>^<edge> blueprints
```

Nothing else is in a node. No name, no source position, no literal value, no comment. (observed)

---

## 3. Storage: `nums_db.go`

The whole "database" is in memory, per module, and is never written anywhere. There is no `os.Create`, `WriteFile`, `json`, `gob` or `yaml` in `nums/`. (observed, grep)

```
Module_t { Name; Symbols; BlueprintDB TreeDB_t; RecipeDB TreeDB_t; ... }   module.go:6-17
TreeDB_t { LevelEntries map[NodeLevel]TreeLevelDB_t }                      nums_db.go:24-26
TreeLevelDB_t {
  Entries     map[string]string      "type:inst" -> serialized node
  RevEntries  map[string]string      serialized node -> "type:inst"
  EntryCounts map[string]float32     how many times this shape was recorded
  BlueprintNextID map[NodeType]NodeInstance   never read or written elsewhere
}                                                                          nums_db.go:8-13
```

`Record_Entry` (`nums_db.go:55-80`) is hash-consing:

- if the serialized string is already in `RevEntries`, return the existing ID and add to its count;
- otherwise the new instance is `len(Entries)+1` at that level, the type is whatever the caller passed, and the package is `DefaultPackage`, which is the constant 1 (`nums_nodes.go:16`).

Every caller passes type 0 (`nums_nodes.go:735`, `:789`). So **every minted ID has the form `1.<height>.0.<ordinal at that height>`**. Catalogue IDs are built with package 0 (`nums_nodes.go:290-294`, `:643-647`). Package 1 versus 0 is therefore the working distinction between "minted here" and "fixed catalogue". No other package value is ever produced. (observed)

A leaf-less node (a string with no `+`) is stored as `category=key` so it can never be found by value (`nums_db.go:69-72`). That is how reserved nodes get unique identities. `Update_Entry` (`:82-85`) then overwrites the entry in place when members or a body arrive, without touching `RevEntries`. So:

- **expressions, statements, blocks, parameter lists, function signatures, pointer/list/map types are structural**: same shape, same ID;
- **structs, interfaces, packages and function declarations are nominal**: each declaration gets its own ID, and that ID's content is mutated after it is handed out (`nums_nodes.go:719-721`, `:272-274`, `emit_module.go:265-281`, `nums_nodes.go:800-812`).

(observed) The second point conflicts with the fixed axiom 3, "a changed composition is already a new node id" (`core-axioms.form:68`). NUMS mutates reserved nodes under a stable ID. (inferred, from the two citations)

---

## 4. Symbols: how names attach, and why they are replaceable

Two tables (`nums_symbols.go`).

Per recipe, `RecipeSymbolTable_t` (`:9-15`):

- `Locals`, `Parameters`, `Integers`, `Decimals`, `Strings`, `Literals`, `Labels`: each `map[string]NodeInstance`, text to ordinal;
- `LocalBlueprints`, `ParameterBlueprints`: ordinal to blueprint ID with edge;
- `RecipeBlueprints`: blueprint ID to the ordinal used for it inside this recipe.

Per module, `ModuleSymbolTable_t` (`:85-103`):

- `BlueprintIDs`: name to blueprint ID with edge;
- `BlueprintMembers`: blueprint ID to (member name to member index);
- `BlueprintMemberIDs`: (blueprint ID, member index) to a module-wide member ordinal, which is the instance used in `1.12.N`;
- `GlobalIDs`, `GlobalBlueprints`, `GlobalCOTRs`: global name to ordinal, its blueprint, its initializer recipe;
- `RecipeIDs`, `RecipeBlueprints`, `RecipeSymbols`: function name to recipe ID, to its signature blueprint, to its recipe table.

Ordinals are assigned as `len(map)+1` on first sight (`emit.go:219-234`). Going from number back to name is a linear scan of the map (`emit.go:209-217`; `module.go:60-78`, `:162-169`; `nums_symbols.go:62-74`). The author marks this in comments: "Reverse lookup needed as well" (`nums_symbols.go:81`, `:86`, `:96`). (observed)

Consequences I verified by running it (section 6):

- **Renaming changes only these tables.** The node databases do not move.
- **Literal values are symbols too.** `return a + 1` and `return b + 2` are the same node. The `1` and the `2` are each "Integer ordinal 1" in their own recipe's table. A recipe's meaning is its tree **plus** its table.
- **Two names can share one ID.** `add` and `plus`, both `func(int, int) int`, map to the same signature blueprint. Number-to-name then returns whichever the map scan meets first (`module.go:162-169`).

Where the lens is *not* separated: the spellings `int`, `float`, `*T`, `[]T`, `map[K]V`, `struct`, `interface`, `func` are hard-coded in the core (`module.go:102-187`), and so are the Go operator spellings in the printer (`recipe.go:124-296`). There is an embryo of a table-driven lens in the constant names, which carry templates with placeholders, for example `"R(2.10.3)Condition if ({0}) {1} else {2}"` (`nums_consts.go:300`) and `"R(2.1.4)Compare {0} < {1}"` (`:212`), and the same template strings are used as lookup keys on the way in (`nums_nodes.go:516-535`). The printer does not use them. (observed)

---

## 5. Go text in, and back out

### 5.1 In: `parser.go` and `go.go`

1. **Tokenizing and parsing is delegated to tree-sitter**, through the `smacker/go-tree-sitter` cgo binding (`parser.go:11-24`). `nums` never tokenizes text itself. (observed)
2. Thirteen languages are enumerated and wired to their tree-sitter grammars (`parser.go:29-64`, `:122-152`), and file extensions map to them (`:66-98`). **Only Go has handlers.** Any other language returns the error "NUMS Language: '%s' not supported" (`:451-461`). (observed)
3. `FileParser_t` holds a dispatch table from tree-sitter symbol number to handler function (`parser.go:430-437`). `RegisterGo` fills 75 entries (`go.go:1425-1502`). The symbol numbers are a hand-copied table of 213 constants (`go.go:1209-1423`). I checked a sample of twelve against the pinned grammar in the local module cache (`.../go-tree-sitter@v0.0.0-20230113054119-af7e2ef5fed6/golang/parser.c`): `sym_identifier = 1`, `sym_iota = 90`, `sym_source_file = 92`, `sym_function_declaration = 103`, `sym_struct_type = 121`, `sym_block = 135`, `sym_call_expression = 169`, `sym_binary_expression = 184`, `sym_qualified_type = 185`, `sym_interpreted_string_literal = 186`, `alias_sym_field_identifier = 210`, `alias_sym_type_identifier = 213`. All twelve match. (observed) These numbers are only valid for that grammar version; a grammar update silently re-points every handler. (inferred)
4. `Process_AllNodes` walks the tree in document order; when a handler returns ok it skips to the end of that node (`parser.go:595-612`). An unregistered named node prints "not registered" and is dropped (`:614-629`). (observed)
5. Each handler turns a syntax node into calls on the builder (section 5.2). Each carries the grammar rule it implements as a comment, for example `go.go:185`, `:619`, `:709-712`. Three generic combinators exist: `Token`, `Sequence`, `Choice` (`parser.go:631-657`). (observed)
6. Typing is resolved while building. Every `EmitRecipe_t` carries a blueprint (`emit_recipe.go:9-18`). Identifier resolution order is local, parameter, member of self, function, global, blueprint (`emit_recipe.go:198-231`). **An identifier that resolves to nothing is auto-declared**: as a local in a `:=`, as a function if it is being called, otherwise as a global (`:232-246`); on a package or a struct it is auto-declared as a member (`:247-304`). So `fmt.Println` works without reading package `fmt`: the import creates an empty package blueprint (`parser.go:505-507`) and `Println` is added to it on first use. (observed)
7. All Go integer types collapse to one blueprint, all float types to another (`go.go:41-53`). Width and signedness are gone after parsing. Struct tags are ignored (`go.go:418`). Comments are skipped as "extra" nodes. (observed)

### 5.2 The builder: `emit*.go`

`EmitModule_t` (`emit_module.go:25-44`) holds stacks of open blueprints, open recipes, a "build mode" (declaration, body, parameter, return, call, init, var-decl, key; `:7-23`) and results. `EmitBlueprint_t` (`emit_blueprint.go:37-48`) collects fields and methods, then `End_Blueprint` writes them as child edges and records member names (`:164-191`). `EmitNamedRecipe_t` (`emit.go:178-191`) owns a function's symbol table and parameter list. `EmitRecipe_t` (`emit_recipe.go:9-18`) is one expression or statement under construction.

IDs are minted bottom-up by `EmitRecipe_t.Make_SelfID` (`emit_recipe.go:120-138`): mint the children, then record `category + children`. A function is recorded as two things (`emit_module.go:246-287`):

- a blueprint `Function[return, param, param, ...]`, structural;
- a recipe `FunctionDecl[parameter-list, body]`, with a forced unique ID.

A global with an initializer gets a synthetic recipe named `CTOR:<name>` (`emit_module.go:182-200`). A method is recorded as a member of its receiver's blueprint and then removed from the module's function table (`emit_blueprint.go:128-136`, `emit_module.go:237-244`). (observed)

### 5.3 Out: `recipe.go` and `module.go`

`RecipeWriter_t.Write` (`recipe.go:109-327`) has two modes.

- `asCode = false`: a labelled tree dump, any node.
- `asCode = true`: Go-like text. It has cases for compare, logic, math, bit-math, reference and dereference, increment and decrement, the assignment family, send, member access, subscript, range (operand only), the four block kinds, if and if-else, cast, return-with-value, go, defer, call (`:126-292`). Everything else falls to "Category=%s not handled" (`:293-296`).

Not handled when printing code: loops, function declarations, `new`/`make`, switch, labelled jumps, try/catch, the generic Literal leaf (`PrintGoCode`, `:353-416`, has no `IsLiteral` case, while `PrintString` at `:468-469` has one). (observed)

There is no printer for a function header, a file, a package clause, imports, or a type declaration as Go source. `Get_BlueprintStructLayout` (`module.go:102-139`) prints a struct-like layout for display. The only call to `PrintAsGo` in the package is `log.Printf("Body Go: %s\n", ...)` at `emit.go:370`. (observed)

So what round-trips is: **the body of a function, for the subset of constructs above, normalized**. It is not a text round trip and was not built as one. (inferred from the above)

---

## 6. One example, end to end, with the actual numbers

All of this section is **observed**: output of a small dump program I wrote in the scratch copy (`numscopy/trace/main.go`), linked against the unmodified `nums` sources (apart from the four build lines in section 7.2).

### 6.1 Source

`numscopy/tiny/tiny.go`:

```go
package main

type Point struct {
	x, y int
}

func add(a int, b int) int {
	c := a + b
	return c
}

func sum(p Point) int {
	return p.x + p.y
}
```

### 6.2 Numeric nodes

Blueprint database (key is `type:instance` within the level; the ID is `1.<level>.<key>`):

| ID | Stored value | Reading |
|---|---|---|
| `1.3.0.1` | `0.2.2.3+0.1.2.2^0+0.1.2.2^0` | Object [ Integer, Integer ]: the struct |
| `1.3.0.2` | `0.2.3.1+0.1.2.2^0+0.1.2.2^0+0.1.2.2^0` | Function [ returns Integer, Integer, Integer ]: signature of `add` |
| `1.4.0.1` | `0.2.3.1+0.1.2.2^0+1.3.0.1^4` | Function [ returns Integer, the struct with edge Public ]: signature of `sum` |

Recipe database:

| ID | Stored value | Reading |
|---|---|---|
| `1.3.0.1` | `0.2.8.3+0.1.11.1+0.1.11.2` | Block-comma [ Parameter 1, Parameter 2 ] |
| `1.3.0.2` | `0.2.3.1+0.1.11.1+0.1.11.2` | Math `+` [ Parameter 1, Parameter 2 ] |
| `1.4.0.1` | `0.2.5.5+0.1.10.1+1.3.0.2` | TempWrite `=` [ Local-access 1, `1.3.0.2` ] |
| `1.5.0.1` | `0.2.8.4+0.1.9.1+1.4.0.1` | Block-newline [ Local-decl 1, `1.4.0.1` ] |
| `1.3.0.3` | `0.2.12.1+0.1.10.1` | Jump return [ Local-access 1 ] |
| `1.6.0.1` | `0.2.8.2+1.5.0.1+1.3.0.3` | Block-compound [ `1.5.0.1`, `1.3.0.3` ]: body of `add` |
| `1.3.0.4` | `0.2.18.1+1.3.0.1+1.6.0.1` | FunctionDecl [ parameters, body ]: `add` |
| `1.3.0.5` | `0.2.8.3+0.1.11.1` | Block-comma [ Parameter 1 ] |
| `1.3.0.6` | `0.2.7.2+0.1.11.1+0.1.12.1` | Access `.` [ Parameter 1, Member 1 ] |
| `1.3.0.7` | `0.2.7.2+0.1.11.1+0.1.12.2` | Access `.` [ Parameter 1, Member 2 ] |
| `1.4.0.2` | `0.2.3.1+1.3.0.6+1.3.0.7` | Math `+` |
| `1.5.0.2` | `0.2.12.1+1.4.0.2` | Jump return |
| `1.6.0.2` | `0.2.8.2+1.5.0.2` | Block-compound: body of `sum` |
| `1.3.0.8` | `0.2.18.1+1.3.0.5+1.6.0.2` | FunctionDecl: `sum` |

The whole of `add` as one line, as printed by `Recipe_t.String()` (`recipe.go:49-67`):

```
1.3.0.4 <0.2.18.1> [1.3.0.1 <0.2.8.3> [0.1.11.1, 0.1.11.2],
                    1.6.0.1 <0.2.8.2> [1.5.0.1 <0.2.8.4> [0.1.9.1, 1.4.0.1 <0.2.5.5> [0.1.10.1, 1.3.0.2 <0.2.3.1> [0.1.11.1, 0.1.11.2]]],
                                       1.3.0.3 <0.2.12.1> [0.1.10.1]]]
```

Symbols:

```
BlueprintIDs        Point -> 1.3.0.1^4        add -> 1.3.0.2^2097156        sum -> 1.4.0.1^2097156
BlueprintMembers    1.3.0.1 -> { x:1, y:2 }
BlueprintMemberIDs  (1.3.0.1, 1) -> 1         (1.3.0.1, 2) -> 2
RecipeIDs           add -> 1.3.0.4            sum -> 1.3.0.8
add's table         Parameters { a:1, b:2 }   Locals { c:1, .c:1 }   LocalBlueprints { 1: 0.1.2.2^0 }
sum's table         Parameters { p:1 }        ParameterBlueprints { 1: 1.3.0.1^4 }
```

Edge `4` is the Public bit. Edge `2097156` is Function (bit 21) plus Public; `Declare_FunctionBlueprint` sets Public on every function regardless of its name (`emit_module.go:258`). The `.c` entry is an alias the builder adds for nested scopes (`emit.go:225-228`, `emit_recipe.go:157-159`).

### 6.3 Text back out

```
add  parameters : a, b
     body       : {
                    var c int
                    c = a + b
                    return c
                  }
sum  body       : {
                    return p.x + p.y
                  }
Point           : struct {
                    x int
                    y int
                  }
```

`c := a + b` came back as `var c int` followed by `c = a + b`. That is the builder lowering a short declaration into Local-decl plus TempWrite (`go.go:638-657`). The function header was assembled by my dump program; `nums` has no code to print one.

### 6.4 Three experiments on the same machinery

**Rename.** `numscopy/tiny/renamed.go` is the same program with every identifier replaced by a German word (`Punkt`, `links`, `rechts`, `addiere`, `erste`, `zweite`, `summe`, `gesamt`, `q`). Over 20 runs each, the blueprint and recipe databases of `tiny.go` and `renamed.go` fall into the **same two** byte-identical dumps (md5 `58b5f7f0...` and `97879dcb...`). The printed body of the renamed file is `var summe int / summe = erste + zweite / return summe`. So the names really are a lens over one numeric structure.

**Why two dumps and not one.** The two variants differ only in which field got member ordinal 1 and which got 2. Member ordinals are handed out inside `Fetch_Fields`, which ranges over a Go map (`blueprint.go:185-187`, calling `Emplace_MemberID` through `Fetch_Field` at `:74`). Go randomizes map iteration. `Fetch_Fields` is reached eagerly every time a blueprint handle is made, because `MakeBlueprint` fills a display-only layout string (`blueprint.go:14-26`, `module.go:122`). So a debugging convenience decides the numbering. For `tiny.go` the split over 20 runs was 19 to 1; for `renamed.go`, 16 to 4. **The numeric space is not deterministic from run to run.**

**Sharing.** `numscopy/tiny/shared.go` has `add(a, b)` returning `a + b`, `plus(x, y)` returning `x + y`, and `twice(n)` returning `n + n`. `add` and `plus` share the signature blueprint `1.3.0.1` (count 2), the parameter list `1.3.0.1`, the sum `1.3.0.2`, the return `1.4.0.1` and the body `1.5.0.1` (each count 2). Their FunctionDecl IDs differ (`1.3.0.3`, `1.3.0.4`) but hold the same string `0.2.18.1+1.3.0.1+1.5.0.1`. `twice` shares nothing above the leaves. Both `add` and `plus` appear in `BlueprintIDs` pointing at the same ID.

**Literals.** `numscopy/probe/p17_litshare.go`: `inc1(a)` returning `a + 1` and `inc2(b)` returning `b + 2` produce the identical tree, body `1.5.0.1` with count 2. Their tables are `Integers { "1": 1 }` and `Integers { "2": 1 }`. Literal values are stored as source text, per recipe.

---

## 7. Does it build and run today

Toolchain: `go version go1.27.2 darwin/arm64`. (observed)

### 7.1 As checked in: no

Run inside `NUMS.Go/nums`:

| Command | Result |
|---|---|
| `GOWORK=off GOFLAGS=-mod=readonly go build ./...` | exit 1. Fourteen lines of the form `parser.go:12:2: github.com/smacker/go-tree-sitter@v0.0.0-20230113054119-af7e2ef5fed6: replacement directory ../../[org]-go-tree-sitter does not exist` |
| `GOWORK=off GOFLAGS=-mod=readonly go vet ./...` | exit 1, the same fourteen lines |
| `go build ./...` (workspace on) | exit 1. `go: cannot load module ../../[org]-go-tree-sitter listed in go.work file: open ../../[org]-go-tree-sitter/go.mod: no such file or directory` |

Run inside `NUMS.Go/test` with `GOWORK=off GOPROXY=off GOFLAGS=-mod=readonly go build -o <scratch> .`: exit 1, `../nums/parser.go:24:2: cannot find module providing package github.com/smacker/go-tree-sitter/verilog`.

So `GOWORK=off` does not help, because the missing sibling is referenced twice: in `go.work:8` and in `nums/go.mod:5` (`replace github.com/smacker/go-tree-sitter => ../../[org]-go-tree-sitter`). The sibling is a private fork of the binding that added a Verilog grammar (commit `39e4b49`, 2023-02-27, "adding verilog from fork"). The upstream module at the pinned version has no `verilog` directory (observed in the module cache listing). `ls ~/source` shows no `[org]-go-tree-sitter` directory. (all observed)

`git status --short` afterwards: only the two pre-existing untracked `.DS_Store` files. (observed)

### 7.2 With four lines removed, in a copy: builds and vets clean

In `numscopy/` I copied `nums/`, `test/`, `sample/` and removed:

- `nums/go.mod` line 5, the `replace` to the missing fork;
- `nums/parser.go` line 24, the `verilog` import;
- `nums/parser.go` lines 148-149, `case Language_Verilog: return verilog.GetLanguage()`.

Then, with `GOWORK=off GOPROXY=off GOFLAGS=-mod=mod` (offline; the pinned upstream `go-tree-sitter@v0.0.0-20230113054119-af7e2ef5fed6` and `gopkg.in/guregu/null.v4@v4.0.0` were already in `~/go/pkg/mod`):

- `go build ./...`: exit 0;
- `go vet ./...`: exit 0, no findings.

(observed) It needs cgo and a C compiler, because the binding compiles tree-sitter's C runtime and each grammar's generated `parser.c`. (observed from the module's contents; that the build used them is inferred)

### 7.3 Running

| Input | Outcome |
|---|---|
| `numscopy/tiny/tiny.go` (section 6) | completes |
| 17 single-feature probes in `numscopy/probe/` | all complete; one prints "expression_switch_statement not registered" |
| `test/sample.go` (the repo's own, 41 lines) | **panic**, nil pointer dereference; top frames `emit_recipe.go:55`, `emit_recipe.go:393`, `go.go:1121` (a binary expression whose left side is nil) |
| `sample/test2.go` (the repo's own) | **panic**, nil pointer dereference at `go.go:1010` (`keyed_element`, `tm.Blueprint` with `tm` nil) |
| `sample/sample.go` (the repo's own, the one `test/test.go:13` loads) | prints `sample.go: 236: 162: type_switch_statement not registered`, records 36 functions and 547 nodes, then **panics** at `go.go:1010` while inside `NewStructTest`. The log before the panic holds 68 `ASSERT` lines (49 of them "debug any") and 47 `Break` lines |

(observed; line numbers are those of the original files, `go.go` being unchanged in the copy)

Two things make the crashes unsurprising. `[ORG]_ASSERT_` only logs and continues (`debugging.go:5-10`), and `DEBUGBREAK_` only logs (`:16-18`), so every failed precondition runs on into whatever comes next. And the author's own driver, `test/test.go`, is the eleven-line program that hits the panic.

### 7.4 What the probes show, feature by feature

"Recorded" means the construct is in the recipe database as numbers. "Printed" means `PrintAsGo` gave back correct Go for it. (all observed in `numscopy/probe/*.out`)

| Construct | Recorded | Printed |
|---|---|---|
| arithmetic, comparison, logic | yes | yes |
| `if` / `else` | yes | yes |
| `return x` | yes | yes |
| local `:=`, assignment, `+=`, `++` | yes | yes, with `:=` lowered to `var` plus `=` |
| int, float, string literals | yes (as text in tables) | yes; `var y float` for a `float64` |
| pointer `&a`, `*a` | yes | yes |
| struct, fields, `p.x` | yes | yes |
| call to an earlier function `g(a)` | yes | yes |
| `fmt.Println("x")` through an empty package | yes (member auto-declared) | yes |
| `for i := 0; i < n; i++ { }` | yes: `Loop for [init, cond, update, body]` | **no**: prints an empty line |
| `for _, x := range xs { }` | partly | **no**: prints `range _, x`, body lost |
| `m["a"] = 1`, `m["a"]` | **no**: `index_expression` is a stub (`go.go:916-922`) | no: `= 1` and `return` with nothing |
| `switch` | **no**: not registered | no |
| method with receiver | yes, moved under the struct | not reachable by name afterwards |
| interface with a method | yes | layout only |
| global `const` / `var` with initializer | yes, as `CTOR:` recipes | as `K, K = 7` |
| closure `func(a int) int { ... }` | yes, as recipe `$anom` | the assignment prints `g = ` with nothing after it |
| composite literal `P{x: 1, y: 2}` | yes, as writes to members | prints `return var  P, .x = 1, .y = 2` |
| `go g(c)`, `defer g(c)` with `g` declared later | yes | prints `go (c)`: the callee name is lost |
| generic `func min[T ~int \| ~float64]` | body yes; `T` becomes an auto-declared empty struct, constraint ignored | body yes |

Stub handlers that log and return false (`go.go`): `receive_statement` 597-603, `variadic_argument` 858-861, `index_expression` 916-922, `slice_expression` 931-939, `type_assertion_expression` 942-948, `qualified_type` 1125-1131. Not registered at all: both switch forms, `select`, labelled statements, `goto`, `fallthrough`, type alias, type parameters, variadic parameters. (observed)

---

## 8. Proven working versus sketched

**Proven by a run in this session**

- A four-number ID and a fixed catalogue of leaves and operators can carry a real subset of Go: declarations of structs, interfaces, functions, methods, globals; expressions; `if`; three-clause `for`; calls; member access; pointers.
- Hash-consing by serialized child list works and shares structure across functions.
- Names and literal text sit entirely outside the node databases; a full rename changes no node.
- For the printable subset, a function body comes back as equivalent Go.

**Present in code, wrong or incomplete when run**

- The three shipped samples crash.
- Loops, closures, composite literals, forward-referenced callees and map indexing are recorded (or half recorded) but do not print back.
- Node numbering depends on Go map iteration order.
- `Fetch_MapElementType` has its guard inverted (`blueprint.go:198-203`: returns empty when there *are* nodes, then indexes `Nodes[1]`).
- The level-1 recipe name table is two entries short, so Jump (type 17) is labelled by fallback and the Jump instance names are attached to type 15, Label (`nums_consts.go:739-757`).

**Sketched only**

- Any language other than Go (twelve grammars linked, zero handlers).
- Importing another Go package (`parser.go:482-485` "TODO: implement"; `:505-525` commented out).
- Cells: `Cell_t` (`reflect.go:3-6`) is unused. There is no value, no instance, no state.
- Execution of any kind.
- Persistence of any kind.
- Cross-module identity: the package field is always 1.
- `nums/sample id space.txt` (added 2023-02-22, commit `45322b7`): a 62-line outline of a module holding packages as blueprints plus a "blueprint-DB" and "recipe-DB". No code reads it. It lists a struct name twice (`:27` and `:39`), which the real sample does not. It is a design note.
- Fifteen of the 23 edge bits; the Control, Runtime, Resource, Variadic blueprint families; Matrix, Set, Template, Union containers; Sync, Exception, TypeDecl recipe kinds.
- The builder-style API written as comments beside the code in `test/sample.go:6-24` (`bp.declStruct("B")`, `rp.jump("Return", rp.call(...))`, `lang.Tuple()`), added in the last two commits (2023-04-02 and 2023-04-05). It reads as the next intended surface: a language-neutral way to say what each line declares. It exists only as comments in that file. (observed that it is comments; that it was the intended next surface is inferred)

---

## 9. What a language-neutral numeric root needs beyond what is here

All of this section is **inferred** from the observations above.

1. **Identity from content, not from arrival order.** Today an ID is "the Nth distinct shape of this height seen in this module". It is local to one in-memory module and one run. A root that many languages and many processes share needs the ID to be a function of the canonical serialization (a hash, or a number derived from one), so two parties that never met compute the same number. The serialization `category+child+child` is already a canonical form to hash.
2. **Determinism.** Remove every place where a Go map's iteration order can reach a number (member ordinals today).
3. **A decision on literals.** Today the tree is a *shape* and the values sit beside it as text. That is a real and useful split (shape versus binding), but then the identity of a runnable recipe is the pair. Either values enter the hash, or the design names both levels and gives each an ID.
4. **No mutation under a stable ID, or an explicit name layer that does it.** Reserved nodes are updated in place. Under axiom 3 the struct with one more field is a new node, and the *name* is what gets re-pointed. NUMS already has the two layers (node databases and symbol tables); the nominal identity belongs in the second.
5. **One ID space, or a tagged one.** Recipes and blueprints reuse the same numbers for different meanings, told apart by a Go type. A stored or transmitted number needs to say which it is.
6. **Blueprints precise enough to run.** One Integer and one Decimal cannot drive machine code. Width, signedness, representation and layout have to be in the blueprint (or in an edge) before a JIT can exist.
7. **Semantics.** NUMS has a vocabulary and a tree. It has no rule for what `2.3.1` *does*. An executable root needs each catalogue entry to have a meaning the bootstrap can run, and a way to define new entries in the grammar itself.
8. **Cells and state.** Blueprint and recipe are represented. Cell (an instance with a value and a life) is not. The ice/water/gas states of a node have nothing to attach to here.
9. **Lenses as data.** Reading in is 1,500 lines of hand-written handlers bound to one grammar version's symbol numbers. Writing out is a hard-coded switch with Go spellings. For "any language", both directions should be tables keyed by catalogue ID. The `{0} {1}` templates in the constant names are the seed of that. The later `http/config/Go.yaml` is the same idea for the inbound side.
10. **Its own reader.** The parser is a cgo dependency on a C library plus one generated C parser per language. A bootstrap that aims to be minimal and self-replacing cannot carry that; the one grammar needs a reader small enough to live in the bootstrap.
11. **Scopes.** Locals are one flat table per function with a `.` prefix as a nesting hint. Shadowing and block scope are not represented.
12. **The constructs BML proves and NUMS lacks.** Choice, fail, stop, store, templates and generics, multiple inheritance, objects as data pointer plus interface pointer. NUMS has `Template` and `Union` only as commented-out names, an unused `Virtual`/`Override`/`Parent` edge vocabulary, and `Interface` as a container kind.
13. **What is deliberately dropped.** Comments, layout, tags, numeric width. If the lens must reproduce a file, those need a side channel; if not, say so.
14. **Natural language.** Nothing in `nums/` touches natural language. The claim that the same space roots natural languages is not evidenced by this package. (observed absence; the `http/` tree has a `sentenceHistogram` and a label hierarchy that I did not study)

---

## 10. What to carry forward, and what to let go

Carry forward:

- The four-part address and the three-tier reading of it: fixed leaves, fixed operators, minted composites whose level is their height.
- A node is a category plus an ordered list of child IDs, and nothing else.
- Hash-consing with a use count per shape (`EntryCounts`): the count is a free signal of what is hot.
- Names, and optionally values, as a detachable table of small ordinals. Proven by the rename experiment.
- Edge attributes on the reference rather than on the referenced node.
- The blueprint/recipe pairing for a function: a structural signature and a body.
- Write versus TempWrite: effect visibility in the category.
- A catalogue drawn from many languages at once.
- The grammar rule kept as a comment above each handler: the map from surface rule to numeric construction, readable in one place.
- `Undefined 0.0.0.0` as a first-class "no node", distinct from Null and from false.

Let go:

- Ordinal, per-module, per-run IDs.
- In-place update of a node under a fixed ID.
- Assertions that log and continue.
- Auto-declaring whatever does not resolve. It makes single-file parsing possible and makes every typo a new global.
- Reverse lookups by scanning maps.
- Go spellings inside the core.
- Hand-copied grammar symbol numbers.
- A tree-sitter cgo dependency in anything called a bootstrap.
- Two ID spaces distinguished only by a host-language type.
- The word "emit" for both directions.

---

## 11. Open questions

1. Where is the C++ NUMS that this was translated from, and does it execute? If "fully executable" is true of NUMS anywhere, it is not true of this package.
2. Was the private tree-sitter fork changed beyond adding Verilog? If it changed the Go grammar, the symbol table at `go.go:1209-1423` might not match it; it does match upstream at the pinned version.
3. Did the three samples ever run to completion at some commit? I did not bisect.
4. Was the literal-outside-the-tree split intended as shape/binding, or is it a leftover of treating literals like identifiers? The code comment at `nums_symbols.go:11` only says "TODO: find a more generic way to store scalar literals".
5. Is `Type` meant to stay 0 for minted nodes? `Record_Entry` takes a type and `BlueprintNextID` is keyed by type, which suggests per-type numbering was planned and not finished.
6. What is the `Package` field for: a Go package, a module, a language, a tenant? Only 0 and 1 occur.
7. How far did the `http/` second design get, and does it supersede this one or sit beside it? Out of scope here.
8. For the new design: should the number of a node be a hash, or a dense ordinal plus a hash index? Dense ordinals are what make the tables small here; hashes are what make identity shareable.

---

## 12. Files produced by this study (all in the session scratchpad, none in the repository)

- `.../scratchpad/numscopy/nums`, `test`, `sample`: the copy, four lines removed as listed in 7.2.
- `.../scratchpad/numscopy/trace/main.go`: dump program (databases, symbol tables, numeric and labelled trees, printed bodies).
- `.../scratchpad/numscopy/crash/main.go`: the repo's `test/test.go` with the file name as an argument.
- `.../scratchpad/numscopy/tiny/{tiny,renamed,shared}.go` and `.out`: section 6.
- `.../scratchpad/numscopy/probe/p01..p17*.go` and `.out`: section 7.4.
- `.../scratchpad/numscopy/run-sample.log`: full log of the original driver on `sample/sample.go` (2,855 lines, ends in the panic).
