# NUMS.Go second generation: the `http/` model, its ID spaces, the label lens, and what is proven

Study date: 2026-11-24. Subject: `NUMS.Go` at HEAD `9e6bbb437aa419f4ea73807b2d3ea906e02f2ddf`
("current state", 2023-04-05). History: 60 commits, 2023-02-01 to 2023-04-05 (`git log`, observed).
The repository was not modified: `git status --short` before and after shows only two untracked `.DS_Store`
files that were already present. All experiments ran in copies under the session scratchpad.

Status words used below:
- **observed**: read in the source or run in this session.
- **observed (patched scratch)**: run in this session, in a scratch copy that carries the four build patches listed in
  section 9, against the **upstream** tree-sitter Go grammar (`github.com/smacker/go-tree-sitter` pinned
  `v0.0.0-20230113054119-af7e2ef5fed6`), not the author's fork, and against Go 1.27.2's standard library.
- **inferred**: my reasoning from observed things (the source is named).
- **assumed**: not verified.

Paths are relative to `NUMS.Go/` unless absolute. `file:line` refers to HEAD.

---

## 1. One-paragraph orientation

`http/` is a Go program (module `[org]/http`, `http/go.mod:1`) that parses Go source with tree-sitter and
re-describes every syntax node in a language-neutral vocabulary whose three roots are **Blueprint** (types, structure),
**Recipe** (behaviour: functions, statements, expressions, operators) and **Cell** (storage: globals, locals, parameters,
members). The neutral vocabulary is a YAML tree of 237 labels (`http/config/labels.yaml`); a second YAML file per
language (`http/config/Go.yaml`) maps tree-sitter symbol names to those labels and carries declarative rules for naming,
scoping and built-in matching. On top of the labelled tree it builds a nested identifier-scope tree with numeric IDs,
infers a blueprint (type) per identifier, scores nodes, compresses code to one-letter "sentences" and builds corpus
histograms, and serves all of it over a gin HTTP API and an HTML browser. The package contains **two generations side
by side**: the working "Cache_" model (`Node_t`, `IdScope_t`, global `cache_g`/`config`; `node.go`, `id_scope.go`,
`file.go`, `package.go`, `label.go`, `language.go`, `query.go`, `cache.go`, `html.go`) and a "new API"
(`interfaces.go`, `implementation.go`; `Node_i`, `BlueprintDeclaration_i`, `RecipeDeclaration_i`, `CellDeclaration_i`
and their `*Use_i` twins) started 2023-03-31 and left mid-refactor at HEAD.

It is an **analysis** system. Nothing in `http/` executes, emits, or translates code; "fully executable numeric space"
is not what this directory demonstrates (observed: no interpreter or emitter in `http/model/*.go`; the first-generation
`nums/` module has `emit*.go` files, outside this study's scope).

---

## 2. The node model

### 2.1 Old model (`Node_t`) — the one that works

`http/model/model.go:68-88` (observed):

| field | meaning |
|---|---|
| `ID NodeLocation_t` | `{Start byte, Level}` (`model.go:23-26`) |
| `File`, `Folder` | owning file and folder |
| `FieldName` | tree-sitter field name of this child in its parent (`node.go:101`) |
| `Level` | distance from the file root (root = 0); set in `file.go:224-237`, `node.go:94` |
| `Position` | index among the parent's children |
| `Parent` | parent node |
| `ResolvedLabels Labels_t` | the NUMS labels this node resolved to |
| `IdScope *IdScope_t` | set when this node **declares** an identifier / opens a scope |
| `IdRef *IdScope_t` | set when this node **refers** to a declared identifier |
| `Blueprint` | inferred type of the node |
| `sentence_`, `children_`, `depth_` | lazy caches (`depth_` = height above the leaves, `model.go:101-119`) |
| `tsnode *sitter.Node` | the tree-sitter node it wraps |

Note (observed): the comments on `model.go:73` ("`Level` // bottom-up") and `model.go:83` ("`depth_` // top-down") are
the reverse of what the code computes. `Level` is top-down (root 0, `cache.go:315-321`), `Depth` is bottom-up (leaf 0).
The trace in section 7 confirms it: the root is `1.0.0.0`, leaves have levels up to 10, and
`Cache_Get_DepthNodes(0,0)` returns leaves.

Everything is lazy and memoised behind `Cache_*` methods; the "Cache_" prefix marks every method that touches the
global cache (commit `2f9ab62`, 2023-03-30, "rename all methods touching global cache with Cache_").

### 2.2 New model (`Node_i` + three declaration/use families) — the intended shape

`http/model/interfaces.go` (590 lines, observed):

- `IdentifierNode_i` (`:254-261`): `UniqueID`, `Name`, `HasName`, `Declarations`, `Size`, `Blueprint`. Every named
  thing — package, blueprint, recipe, cell, and each *use* of one — satisfies it.
- `BlueprintDeclaration_i` (`:267-306`): kind, ordered `Members`, `MemberByName/Position`, `Base`, use/read/write maps
  keyed by use-kind and node location, methods, `isGeneric`, `genericInstance`.
- `RecipeDeclaration_i` (`:310-349`): `This`, `Input`, `Output` (all **blueprints**), parent/children recipes, calls,
  `IsLeaf`, `IsRecurive`, `MaxLoopLevel`, cell reads/writes.
- `CellDeclaration_i` (`:353-368`): kind (Global/Parameter/Local/Register/Resource/Sync/Literal, `:557-567`),
  `InitRecipe`, uses.
- `BlueprintUse_i`, `RecipeUse_i`, `CellUse_i` (`:371-423`): a declaration plus the node that uses it.
- `Node_i` (`:428-478`) with `NodeKind_e` as **bit flags** (`:569-590`):
  `{Delimitor, Whitespace, Comment, Decl, Use, Keyword} x {Blueprint, Recipe, Cell}`, e.g.
  `NodeKind_RecipeDecl = NodeKind_Decl | NodeKind_Recipe`.
- Time and authorship are in the contract but unimplemented: `Authors_i`, `Versions_i` (`:78-99`).

Two structural ideas in `implementation.go` are worth keeping (observed in source):

1. **A package is one blueprint plus one recipe.** `NewGoPackageI` creates a `BlueprintKind_Package` blueprint and a
   `name$global` block recipe (`implementation.go:885-889`). Types hang off the blueprint, functions and globals off the
   recipe.
2. **Every expression and statement is a use of a built-in recipe template.** `init()` makes 17 template
   registrations keyed by recipe kind, 16 distinct kinds since the two literal templates share one key (`$unary`, `$binary`, `$write`, `$call`, `[{0}]{1}`, `*{0}`, `$select`, `$read`, `$condition`, `$transform`, `$loop`,
   `$flow`, `$block`, `$jump`, literals, `implementation.go:246-262`) and each AST node instantiates one
   (`:296-355`, `:801-827`). A function's signature is itself a blueprint named `$func.<name>` with `$input`,
   `$output` and (for methods) `$this` children (`:596-618`, `:1677-1694`). Literals are uses of built-in recipes that
   return a primitive blueprint (`:219-244`, `:280-294`).

`implementation.go` has 118 `nums.DEBUGBREAK_()` calls, 73 of them one-line stub methods (counted with `grep`,
observed): `InfoAPI`, `Author_i`, version points, `UIInfo`, `Title`, `Calls`, `IsLeaf`, `CellWrites`, `Score`, and
`FolderByID`/`FileByID`/`PackageByFolder` are all stubs. `QueryAPI` is an empty interface (`interfaces.go:75-76`).

---

## 3. ID spaces: how numbers are allocated, nested and resolved

There are nine numeric spaces (rows below). None is content-addressed; apart from the two that come from outside
(the compiled language enum and the grammar's symbol numbers) all are **allocation-order counters or positions**.

| space | type | allocation | scope / lifetime | source |
|---|---|---|---|---|
| Label | `LabelID_t` | `len(c.Labels)+1` at first `Make_Label(name, create=true)` | process; **changes between runs** | `label.go:327-342` |
| Colour | `ColorID_t` | `len(c.Colors)+1` | process | `label.go:12-27` |
| Language | `nums.Language_e` | compiled `iota` enum, 13 languages | compile time | `nums/parser.go:29-47` |
| TS symbol | `sitter.Symbol` (uint16) | assigned by the generated tree-sitter grammar | grammar version | `language.go:226-236` |
| Folder | `FolderID_t` | `len(c.Folders)+1`, first-seen order | cache, until `Cache_Reset` | `cache.go:182-198` |
| File | `FileID_t` | `len(files)+1` per folder, in `os.ReadDir` order; `0` for in-memory content files | per folder | `folder.go:31-40`, `:68-75` |
| Node | `NodeLocation_t{Start, Level}` | byte offset + depth from root | per file version | `model.go:23-26`, `file.go:224-237` |
| Unique node | `UniqueNodeLocation_t` | `folder.file.start.level` | cache | `interfaces.go:141-144`, `node.go:1337-1339` |
| Identifier | `IdentifierID_t` | `len(c.IdScopes)+1`, one global counter | cache, until `Cache_Reset` | `model.go:687-704` |

### 3.1 Label IDs are not stable (observed, run three times)

`Load_LabelMap` walks the YAML as a Go `map[string]any` (`label.go:104-159`, `H` at `model.go:16`), and Go map iteration
order is randomised, so the order in which names reach `Make_Label` differs per process. Three runs of the same test
binary printed:

```
total=237 Blueprint=180 Recipe=2  Cell=155 BlueprintName=173 RecipeName=174 CellName=166 FunctionDecl=4   Comment=237
total=237 Blueprint=173 Recipe=20 Cell=227 BlueprintName=17  RecipeName=6   CellName=9   FunctionDecl=155 Comment=233
total=237 Blueprint=175 Recipe=7  Cell=229 BlueprintName=174 RecipeName=164 CellName=167 FunctionDecl=130 Comment=1
```

Consequences (observed in code): the well-known labels are Go package variables filled **by name** at start-up
(`config.go:28-140`, `setupLabels` `:142-293`), so in-process comparisons by ID are consistent; but any ID that leaves
the process is meaningless next time — `/api/label/:id` (`api.go:115-119`), and the language JSON cache, which serialises
`Label_t` copies with their IDs. In the language probe (section 8) a second process loaded `Python.json` and showed label
`identifier#243` while only 237 labels existed in that process.

So in this generation **the name is the identity and the number is a per-process handle** — the opposite of "a numeric
space with symbols attached as a replaceable lens". (The first-generation `nums/nums_consts.go` uses compiled `iota`
constants with names like `"(1.*.*)Trivial"`, i.e. fixed numeric coordinates; I read only its first 40 lines — observed,
shallow — and it belongs to the first-generation study.)

One more order dependence (inferred from `label.go:137-138` and `labels.yaml:180,193`): the label name `Yield` appears
under both `SyncKind` and `JumpKind`; `Make_Label` returns the existing label and `label.Parent = parent` is overwritten
by whichever is visited last, so `Yield`'s parent is run-dependent. `Yield` is the only duplicated name (checked with
`uniq -d`, observed).

### 3.2 Node IDs are positions

A node is addressed by `(start byte, level)` inside a file and `folder.file.start.level` globally; the HTML uses the
same string as `nid='1.0.8.2'` (`html.go:1036-1038`, asserted in `api_test.go:900`). It needs no table, and parent/child
chains that start at the same byte are separated by level. It is not stable under edits (any insertion earlier in the
file shifts `Start`) and it is not content-derived (inferred from the definition). Zero-width sibling nodes would
collide (assumed; not tested).

### 3.3 Identifier scopes: allocation and nesting

`IdScope_t` (`model.go:636-655`) is at once an identifier and the scope it opens:
`ID`, `Package`, `Name`, `Parent`, `Children map[name]*IdScope_t`, `Properties map[string]*Node_t` (named pointers back
into the syntax tree: `Blueprint`, `Value`, `Returns`, `Receiver`, `Body`, `Path`, `TypeParameters`, `_DeclNode`),
`Position`, `Kind *Label_t`, `Labels`, `Decl *Node_t`, `Blueprint`, `RecipeInfo`, and four use lists
`Calling/Reads/Writes/Uses` of `IdUse_t{ID, LoopLevel, Node}` (`model.go:670-675`).

- Root: one scope per package, named `Package:<name>` then renamed, kind `Package` (`package.go:23-24`, `:76-77`).
- Allocation: `cache_g.NewIdScope` under one mutex; `ID = len(c.IdScopes)+1`; `Position = len(parent.Children)` at
  creation (`model.go:687-704`). Reverse lookup `Cache_IdScopeById` backs `/api/id/:id` (`model.go:677-685`,
  `api.go:54-62`).
- Where a new identifier lands is chosen by the rule's `Scope` word, resolved by `Node_t.Get_IdScope`
  (`node.go:302-333`): `Top`/`Default` = the package scope; `Context` = nearest ancestor node that owns an `IdScope`,
  else the package; `TopContext` = the top of that chain.
- Anonymous scopes get synthetic names, all `$`-prefixed: a symbol listed under `Rules.Naming.Context` opens
  `$<symbol>` / `$<symbol>0`, `$<symbol>1`... (`node.go:910-928`, e.g. `$block`, `$if_statement`, `$for_statement0`);
  unnamed functions/structs become `$func`, `$func0`, `$struct` (`node.go:730-748`); unnamed parameters `$param<N>`,
  unnamed results `$ret`, `$ret<N>` (`node.go:713-728`); a shadowing name becomes `$unique[<name>]<N>` and gets the
  `Override` attribute label (`node.go:531-534`, `:564-567`).
- Methods are **re-parented**: `CheckReceiver` moves a method's scope from the package to the receiver's blueprint
  scope (`node.go:806-820`).
- For top-level declarations `SetDecl` also files the scope in the package's `Blueprints`, `Recipes` or `Cells` maps by
  its `BlueprintName`/`RecipeName`/`CellName` label (`id_scope.go:126-147`).

### 3.4 Resolution

`Cache_Lookup(sourceDir, id)` (`id_scope.go:193-273`): dotted names start again from the top; otherwise look in this
scope's children, then walk `Parent` upward. Import scopes and `.`-imports are loaded **on demand** through
`go/build.Default.Import` and the imported package's public names are copied in (`:212-239`, `:58-78`; "public" is "first
rune is not lower-case", a Go rule, `:61-67`). When a use resolves, `Node_t.Resolve` copies the declaration's labels and
rewrites the role: `BlueprintName -> BlueprintUse`, `RecipeName -> RecipeUse` (+ call edge), `CellName -> CellUse`
(+ read edge) (`node.go:976-996`). Member access resolves through the inferred blueprint of the base expression
(`node.go:998-1057`, `id_scope.go:411-433`). Call and write targets are recognised from position within a `Call` /
`AssignmentKind` parent (`node.go:1059-1104`), and each use records its loop nesting depth (`node.go:1593-1603`).

---

## 4. From language syntax to neutral labels

### 4.1 The label tree (`http/config/labels.yaml`, 298 lines, 237 labels — observed)

Top-level entries: `Comment`, `Delimiter`, `LeftDelimiter`, `RightDelimiter`, `WhiteSpace`, `SourceFile`,
`IdentifierKind`, `Recipe`, `Blueprint`, `Cell`, `Predefine`.

- `IdentifierKind` (`:8-24`): `BlueprintName`; `RecipeName {Function, Method}`; `CellName {Global, Member, Local,
  Parameter}`; attribute labels `ReturnParameter`, `Receiver`, `Generic`, `Override`, `UnresolvedKind`.
- `Recipe` (`:26-216`): `RecipeKeyword`, `Operator`, `MethodDecl`, `FunctionDecl`, `Literal {...}`,
  `TrivialRecipe {BuiltIn, Identifier {CellUse, RecipeUse, BlueprintUse, ...}, TrivialJump}`,
  `BasicRecipe {Expression {CompareKind, LogicKind, MathKind, BitMathKind, AssignmentKind, AccessKind,
  ConditionExpressionKind, MemoryKind, TransformKind, Call, BlockExpressionKind}, StatementKind {BlockStatementKind,
  SyncKind, ConditionStatementKind, JumpKind, LoopBody, LoopKind, FlowKind, ExceptionKind}}`, `FunctionBody`, `Lambda`,
  `TypeDecl`.
- `Blueprint` (`:218-285`): `BlueprintKeyword`, `BlueprintDecl {Alias, PackageDecl, ImportDecl, ObjectDecl,
  InterfaceDecl, MemberDecl}`, `BlueprintTrivial {Void, Numeric {Bool, Integer, Decimal, Imag, Complex, Char, String}}`,
  `BlueprintBasic {ReferenceKind {Pointer, Reference}, Container {List, Tuple, Dictionary, Object, Package, Import,
  Interface, Any, Matrix, Set, Template, Union}, Control {...}, BlueprintRecipe {FunctionSignature, ...}, Data {Channel,
  Event}}`.
- `Cell` (`:287-293`): `GlobalDecl`, `LocalDecl`, `ParameterDecl`, `Const`, `CellKeyword`.

Per-label metadata, inherited downward unless overridden (`label.go:104-159`): `_Score`, `_Level`
(Leaf 0 / Basic 1 / Category 2 / Group 3 / Top 4, `model.go:244-253`), `_ChildLevel`, `_Color`, `_Attribute`
(a modifier that does not count when choosing the root), and `_Op`, a **surface template with numbered holes** such as
`'{0} + {1}'`, `'if {0} {1} else {2}'`, `'for {0}; {1}; {2} {3}'` (`labels.yaml:79-207`). The hole count is used as an
arity to disambiguate operators (`label.go:573-587`, `node.go:1151-1168`).

The labels cover more than Go: `SpaceShip`, `StrictEqual`, `Ternary`, `While`, `Do`, `Try/Catch/Throw`, `Malloc/Free`,
`This/Super`, `AccessPointTo`, `Union`, `Template`, `DivideDivide` (`labels.yaml`, observed). The neutral vocabulary was
designed for a family of languages; only Go is mapped onto it.

The task brief names "declaration, implementation" as hierarchy members. `*Decl` labels exist under all three roots;
there is **no** `Implementation` label in `labels.yaml` (grep, observed). "implementation" appears only in the flat
wish-list of content labels in `query api and todo.txt:65-114`.

### 4.2 The language file (`http/config/Go.yaml`, 282 lines — observed)

Loaded by `Language_t.Load_Yaml` (`language.go:218-333`) into per-symbol entries `TSSymbolEntry_t`
(`model.go:741-752`): `PossibleLabels`, `Score` (default 1), `NamingRule`, `NamingContext`, `LabelMatchers`,
`LoopingField/Label`, `NameLookup`. Sections:

1. `Labels:` (`Go.yaml:84-231`, 112 label keys) — `NUMS label: [tree-sitter symbol names]`. Many-to-many: `'*'` is
   listed under `Operator`, `AccessDereference`, `Multiply`, `MathKind` and `Pointer`; `unary_expression` under four
   labels. The inverse table per symbol is its `PossibleLabels`.
2. `Rules.Naming.Labels:` (`:3-33`, 19 entries) — `DeclLabel: ts_symbol: { Name | Names | NoName, Kind, Match,
   Properties, Scope, Labels, DefaultName }`. Selectors are a tiny path language evaluated by regex in
   `Cache_Get_NodesFromRule` (`node.go:447-489`): `$Named[i]`, `$Child[i]`, `$Field[name]`, `$Parent`, plus
   `$FromPath()` (derive an import name from its path, `node.go:491-521`). `Match` supports only
   `<selector>.$Symbol == x`, `<selector> == x`, `!=` (`matchRule`, `node.go:768-804`).
   Example (`Go.yaml:19`): `method_declaration: { Name: "$Field[name]", Kind: Method, Properties: { Returns:
   "$Field[result]", Receiver: "$Field[receiver]", Body: "$Field[body]" }, Scope: Default, Labels: [RecipeName, Method,
   Function] }`.
3. `Rules.Naming.Context:` (`:35-43`) — symbols that open an anonymous scope (`block`, `if_statement`, `for_statement`,
   switch/select, case...).
4. `Rules.Matching:` (`:45-69`) — content matches for built-ins: `int`, `uint8`... -> `Integer`; `float32/64` ->
   `Decimal`; `string`, `bool`, `any`, `error`; `make/len/append/...` -> `BuiltInFunc`.
5. `Rules.Mapping.NameLookup:` (`:71-76`) — which symbols trigger an identifier lookup (`identifier`, `generic_type`).
   The nested `BlueprintUse: { Kind: Generic, Properties: ... }` payload does not match the Go struct
   (`language.go:69-73` has `Base`, `Name`, `Kind map`), so only the *presence* of the entry has effect (inferred from
   the struct and from `language.go:297-302`).
6. `Rules.Looping:` (`:80-82`) — `LoopBody: for_statement: "$Field[body]"`, tags loop bodies so uses can carry a loop
   level.
7. `Rules.Import:` (`:78`) — `{ Private: "HasCommonPath()", Friend: "IsFriend()", ... }`. No field in
   `LanguageYaml_Rules_t` (`language.go:79-84`) and no implementation (`grep` for `HasCommonPath`/`IsFriend` in
   `http/model` finds nothing). **Intended only.**
8. `Labels.Unmapped:` (`:233-282`) — explicit list of grammar-internal symbols deliberately left without a label.

### 4.3 Resolution order per node (old model, `node.go:873-970`, observed)

1. Resolve the parent first.
2. Try each naming rule of the node's symbol (`Cache_Apply`, `node.go:600-766`): check `Match`, pick the base scope by
   `Scope`, allocate identifier(s), bind properties, name them, attach kind and labels, label the declaration node with
   the rule's key (`ObjectDecl`, `MethodDecl`, ...). A `Local` declared directly in the package scope is rewritten to
   `Global`/`GlobalDecl` (`:629-636`).
3. Try content matchers (`node.go:822-835`).
4. If the symbol is a naming context and no child made a scope, open the anonymous `$symbol` scope.
5. If still unlabelled and the symbol has a `NameLookup`, look the identifier up through the scope chain.
6. Otherwise fall back to the symbol's possible labels; if more than one, **prune** (`Cache_Prune`,
   `node.go:1145-1209`): operator arity against the parent's named-child count, drop attribute labels, drop
   declaration labels, drop labels whose root disagrees with the parent's root (Blueprint vs Recipe), prefer labels
   matched by children. The remaining set may still hold several labels; there is no confidence value although
   `model.go:162` and `query api and todo.txt:14` plan "label + confidence factor".
7. Post-passes: loop label, member resolution through blueprints, call/write marking.

Package-level order (`package.go:162-195`): for all files, scan blueprint names; then blueprint bodies (members,
methods, parameters); then recipe names; then cells. Bodies resolve lazily when asked.

### 4.4 Blueprint (type) inference (old model)

`Blueprint` interface (`blueprint.go:24-30`) with `Primative`, `Alias`, `Interface`, `Struct`, `Function`, `Ptr`,
`List`, `Map`, `Channel`, `BlueprintList` (`:48-91`). `IdScope_t.Cache_Get_Blueprint` (`id_scope.go:275-401`) and
`Node_t.Cache_Get_Blueprint` (`:514-736`) infer from the `Blueprint`, `Value`, `Returns` properties and from label kind
(literals, `make`/`new`/`len`, pointers, lists, maps, channels, selects, subscripts, range targets). `BlueprintSize`
(`blueprint.go:11-22`) is a **shape histogram, not bytes**: counts of bools, integers, decimals, strings, pointers,
lists, maps; `Size() = primitives + 100*containers + pointers` (`:113`).

---

## 5. Generics and struct parsing

### 5.1 Old model
Structs: `type_spec` with `Match: "$Field[type].$Symbol == struct_type"` -> `Container` scope labelled
`[BlueprintName, Object]`; each `field_declaration` -> `Member` scopes labelled `[CellName, Member]` with a `Blueprint`
property pointing at the type node; embedded fields use `NoName: "$Field[type]"` (`Go.yaml:9-17`). Asserted by
`TestGo_Labels_Struct` (`api_test.go:502-537`) — **passes** (observed, patched scratch).

Generics: not resolved. Parsing Go 1.27's `net/http/mapping.go` during `Test_Query_File_api` printed
`Lookup .../mapping.go:25: <mapping[K, V]> Unresolved` repeatedly (observed, patched scratch). The `Generic` label
exists as an attribute (`labels.yaml:22`) and `Template` as a container kind (`:266`).

### 5.2 New model
- A parameter whose parent field is `type_parameters` becomes `BlueprintKind_GenericParameter` with the `Generic`
  attribute (`implementation.go:507-511`, `:639-641`); `Go.yaml:9,22` bind `TypeParameters: "$Field[type_parameters]"`;
  `Go.yaml:184` maps `type_parameter_list` to `ParameterDecl`.
- `isGeneric()` = "has a child of kind GenericParameter" (`implementation.go:1724-1731`).
- `genericInstance(arguments, node)` (`:1737-1762`) is a **shallow copy** of the declaration renamed
  `Name[Arg, ...]`, with each generic parameter child's `use` set to the matching argument. Two TODOs mark what is
  missing: `// TODO: lookup / store generic type in package for reuse and tracking` (`:1740`) and
  `// TODO: replace usage to argument` (`:1748`). Uses of `T` inside other members are not rewritten.
- A `generic_type` node (labelled `BlueprintDecl`) resolves its children to blueprint uses; if the first is generic the
  rest are taken as arguments (`:2549-2564`).

What ran (observed, patched scratch, per commit — see section 9 for method):

| commit | date | subject | `Test_API_Package` | `Test_API_Import` | printed package |
|---|---|---|---|---|---|
| `5da0eb9` | 2023-04-01 | basic struct parsing for new API | pass | pass | (trace only) |
| `3111bb4` | 2023-04-02 | more new API | pass | pass | `main { structA { a *int; b *int; m map[string][]*structA; }; test_f $func0; }` |
| `592a526` | 2023-04-02 | generic support | pass | pass | same line |
| `9e6bbb4` | 2023-04-05 | current state (HEAD) | pass | **panic** `implementation.go:1718` | none |

`Test_API_Import` has **no assertions**; it passes when nothing panics (`newapi_test.go:19-71`). The committed test at
`592a526` contains no generic code; the generic snippet exists only commented-out at HEAD (`newapi_test.go:45-50`).

My own probes on `592a526` (scratch file `gen_test.go`, observed):

```
type Stack_t[T any] struct { items []T } ; type StackUse struct { s Stack_t[int] }
  -> genericInstance: Stack_t[int] { T int; items []T; }
  -> main { Stack_t { T any; items []T; }; StackUse { s Stack_t[int]; }; }
type Pair_t[K any, V any] ... ; p Pair_t[string, int]
  -> nil-pointer panic inside String()
func add(a int, b int) int        -> "Not Resolved" for the function; package prints  main { a int; b int; }
method with receiver              -> package prints  main { structA { a int; }; rec structA; }
struct with nested anonymous struct -> main { structA { $struct0; ano $struct0; a int; f float32; }; }
```

So at its best commit the new model demonstrably handled named structs with pointer/map/slice members and a
**one-parameter** generic instantiation by shallow copy; parameters, receivers and nested members leaked into the wrong
blueprint, and two type parameters crashed.

At HEAD the new model regressed (observed, scratch file `trace_test.go`): every case prints `main {  }` or `main`,
because `namedNodeChildren(name, expand_leaves=true)` returns the named **children** of each matched node and so drops
matched nodes that are themselves leaves (`implementation.go:2465-2477`); every `$Field[name]` name is lost and
declarations fall into the anonymous `$struct0` path (`:549-556`, `:1630-1648`). The old model's equivalent keeps
leaves (`node.go:394-402`). At `592a526` the function had no expansion (`git show 592a526:http/model/implementation.go`,
line 1887). The method case panics at `:1718` because the receiver's type can no longer be found by name.

---

## 6. Query API, scoring, sentences, histograms

### 6.1 HTTP surface (`http/api.go:358-389`, observed in source; server not started — `gin` is not in the local module cache)

JSON: `/api/folders`, `/api/files`, `/api/file?folder&id|name|path`, `/api/nodes?...`, `/api/labels?level`,
`/api/languages`, `/api/language?id`, `/api/folder/:id`, `/api/id/:id` (identifier scope), `/api/func/:id`,
`/api/node/:id` (`folder.file.start.level`), `/api/label/:id`, `/api/file-types/:id`, `/api/file-functions/:id`.
HTML: `/`, `/file`, `/html/file/:id`, `/info_panel/:id`, `/breadcrumbs`, `/sort`, `/static`. Graph routes in
`histo.go:89`. `/api/nodes` builds a filter from 18 query parameters and returns **only the histogram** of the matching
nodes; returning the nodes is commented out (`api.go:194-222`).

### 6.2 Filter (`QueryFilter_t`, `model.go:879-901`; options `query.go:45-150`; predicate `query.go:201-246`)

Folder, files, languages, TS symbols, **labels (node must carry all requested labels)**, named/anonymous only,
max depth, byte range, row range, min children, label-count range, label-level range, score range (on summed symbol
score), and `Flat` (skip nodes nested inside an already-returned node). Options are functional
(`NewQueryFilter(With_Labels(...), With_MinChildrenCount("1"))`). `MaxLabelLevel`/`MinLabelLevel` are inert: the two
node methods return 0 (`query.go:152-153`).

Result projection: `Cache_NewResultWithFields(fields, childFilter, childFields)` (`node.go:1235-1320`) — the caller names
the fields it wants (`Content`, `Symbol`, `Row`, `ComputedLabels`, `SumLabelScore`, `Histogram`, `ChildResults`,
`ChildResultsHistogram`, ...) and a nested filter + field list for children. This matches the span model sketched in
`query api and todo.txt:4-22`.

Planned and absent (observed: `query api and todo.txt:52-58`; zero occurrences of "formula" in the code):
"named pre-compiled query selectors", "expression syntax (query-language + AST-node => bool)", `formula_label_score`,
`formula_score`, label confidence factors.

### 6.3 Scores
- **Symbol score**: `TSSymbolEntry_t.Score`, default 1, never set from config (`model.go:755`). The summed symbol score
  is therefore the node count: the trace has 103 nodes and `sumSymbolScore=103.00` (observed).
- **Label score**: `_Score` from `labels.yaml`, inherited. A node's label score is the **mean of its non-zero label
  scores** (`node.go:244-259`); the subtree score is the sum (`node.go:283-300`). Examples: `DecimalLiteral` 3, `Call`
  2, `For` 3, `LogicAnd` 2.5, `MemoryKind` 4, `GlobalDecl` 3, `Comment` -0.2.
- **HotScore**: placeholder text `HotScore: xx` in `html.go:200,439` and in `html design.md:21,27`. The raw material is
  recorded (`IdUse_t.LoopLevel`), the score is not computed. **Intended only.**
- **RecipeInfo** (`id_scope.go:835-896`): `IsLeaf`, `IsRecursive`, `HasExtern`, `Writes`, `DirectCalls`, `AllCalls`
  (transitive), `Blueprints`. `Test_Query_Function` printed for `fib`: `IsLeaf false, IsRecursive true, DirectCalls
  (main.fib)` (observed). `Writes` is always empty: it tests the *identifier's* labels for `IdentifierWrite`, which is a
  node label (`id_scope.go:859`); in the trace `Scale` writes `p.X` and reports `Writes ()` while scope `X` itself
  records `writes=1` (observed). `HasExtern` is always false (`IsExtern` returns false, `:831-833`).

### 6.4 Sentences
`Cache_Sentence` (`node.go:1356-1421`) maps each leaf to one character and concatenates: `F` function, `f` built-in
function, `L` local, `G` global, `P` parameter, `M` member, `B` blueprint name, `I` unresolved identifier, `r`/`b`/`c`
recipe/blueprint/cell keyword, `i` import, `p` package, `t` pointer, `O` operator, `N` nil, `d` number, `s` string,
delimiters as themselves, nothing for whitespace/comments, `?` otherwise. The trace file's sentence is
`bBbBb{M,MB}b(PtB)F(PB){P.MOP.MOP}cGBbF(){LOOB{M:d}L.F(d)}`; method `Scale` is `b(PtB)F(PB){P.MOP.MOP}` (observed).
It is a name-erased, language-neutral shape string.

`SentenceHistogram_t` (`model.go:940-949`) counts sentences per function, per statement, and per length bucket 2..20
(21 = longer) with up to 20 example locations each (`sentenceHistogram.go:34-76`, `node.go:1520-1576`). The rarity
("Anomaly") score of a sentence within its length bucket is
`(cumulative count of sentences at least as frequent / all) x (rank / distinct) x (1 - count / top count)`
(`sentenceHistogram.go:170-208`); a node blends its own score 50/50 with its parent's, recursively, and that drives
the background colour in the HTML view (`node.go:1432-1453`).

Corpus tooling: `repos.yaml` lists 53 repositories under Windows paths `D:\source\cncf.io\...` with instance stamps
2023-03-07 (observed); `repo scan|scanall|vendor` commands run 60 worker goroutines per repo (`repo.go:167-222`,
`api.go:679-749`). The committed `http/info/sentenseHistogram.json` (4.4 MB, last commit 2023-03-13) holds 3,745
statement sentences, 2,331 function sentences and 20 length buckets (counted with `jq`, observed) in an older flat
`sentence -> count` shape that no longer matches `SentenceCount_t{Map}` (inferred from `model.go:929-938`); also
`Load` reads `sentenseHistogram.json` from the working directory while `Save` writes `info/sentenseHistogram.json`
(`cache.go:377,397`).

### 6.5 Symbol/label histogram
`SymbolLabelHistogram_t` (`model.go:427-433`): counts per TS symbol and per label, distinct rows as LoC, summed scores;
the result groups label counts **by label level** (`model.go:468-494`). The trace: `LoC=17 SymScore=103.00
LabelScore=36.98`, top-level labels `GlobalDecl:1 LocalDecl:2 ParameterDecl:5` (observed).

---

## 7. One concrete example, traced with real IDs

Source (scratch `trace_test.go`; old model; folder ID forced to 1 as `TestGo_HTML_Comment` does, in-memory files have
file ID 0):

```go
package main

type Point struct {
	X, Y int
}

func (p *Point) Scale(k int) {
	p.X = p.X * k
}

var Origin Point

func main() {
	q := &Point{X: 1}
	q.Scale(2)
}
```

Observed output (patched scratch). `nid` = `folder.file.startByte.level`. `sym` = tree-sitter symbol number in the
pinned upstream Go grammar. Label numbers after `#` are from **this one run** and change next run (section 3.1); names
are stable. Identifier IDs were identical across four runs (hash of the scope dump with label numbers stripped:
`120b35220abd` each time).

Selected leaves:

| nid | sym | content | possible labels (from Go.yaml) | resolved labels | identifier |
|---|---|---|---|---|---|
| 1.0.8.2 | 212 package_identifier | `main` | Package | BlueprintName, Package | declares 1 |
| 1.0.19.3 | 213 type_identifier | `Point` | BlueprintUse | BlueprintName, Object | declares 2 |
| 1.0.35.6 | 210 field_identifier | `X` | Member | CellName, Member | declares 8 |
| 1.0.40.6 | 213 type_identifier | `int` | BlueprintUse | Integer (content matcher) | — |
| 1.0.53.4 | 1 identifier | `p` | Identifier | CellName, Parameter | declares 4 |
| 1.0.55.5 | 19 `*` | `*` | AccessDereference, MathKind, Multiply, Operator, Pointer | Pointer | — |
| 1.0.56.5 | 213 type_identifier | `Point` | BlueprintUse | BlueprintUse, Object | refers 2 |
| 1.0.63.2 | 210 field_identifier | `Scale` | Member | Function, Method, RecipeName | declares 3 |
| 1.0.79.6 | 1 identifier | `p` | Identifier | CellUse, Parameter, Receiver | refers 4 |
| 1.0.81.6 | 210 field_identifier | `X` | Member | IdentifierWrite, Member | refers 8 |
| 1.0.83.4 | 12 `=` | `=` | Assignment, AssignmentKind, Operator | same three | — |
| 1.0.89.6 | 19 `*` | `*` | (same five as above) | MathKind, Multiply, Operator | — |
| 1.0.100.3 | 1 identifier | `Origin` | Identifier | CellName, Global, StatementKind | declares 7 |
| 1.0.129.5 | 1 identifier | `q` | Identifier | CellName, Local, StatementKind | declares 12 |
| 1.0.134.6 | 65 `&` | `&` | AccessReference, BitAnd, BitMathKind, Operator | AccessReference, BitMathKind, Operator | — |
| 1.0.141.10 | 1 identifier | `X` (literal key) | Identifier | Member | refers 8 |
| 1.0.150.5 | 210 field_identifier | `Scale` | Member | Function | refers 3 |

The same symbol 19 (`*`) resolves to `Pointer` in a type position and to `Multiply` in an expression: the pruning by
parent root and arity at work. The same symbol 1 (`identifier`) becomes a parameter declaration, a receiver use, a
global declaration, a local declaration, or a member, depending on naming rules and lookup.

Identifier-scope tree (ID, position, kind, labels, declaring node, use counters):

```
1  "main"    pos0 Package   [BlueprintName,Package]          decl 1.0.8.2
  2  "Point"  pos0 Container [BlueprintName,Object]           decl 1.0.19.3  uses=3
    3  "Scale" pos1 Method   [Function,Method,RecipeName]     decl 1.0.63.2  calling=1  props Body@1.0.76.2 Receiver@1.0.52.2
      4  "p"      pos0 Parameter [CellName,Parameter,Receiver] decl 1.0.53.4  uses=2   props Blueprint@1.0.55.4
      5  "k"      pos1 Parameter [CellName,Parameter]          decl 1.0.69.4  uses=1   props Blueprint@1.0.71.4
      10 "$block" pos2 Container [CompoundBlock]               decl 1.0.76.2
    8  "X"     pos1 Member   [CellName,Member]                decl 1.0.35.6  reads=3 writes=1  props Blueprint@1.0.40.6
    9  "Y"     pos2 Member   [CellName,Member]                decl 1.0.38.6                    props Blueprint@1.0.40.6
  6  "main"   pos1 Function  [Function,RecipeName]            decl 1.0.119.2 props Body@1.0.126.2
    11 "$block" pos0 Container [CompoundBlock]                 decl 1.0.126.2
      12 "q"      pos0 Local   [CellName,Local,StatementKind]  decl 1.0.129.5 uses=1   props Value@1.0.134.4
  7  "Origin" pos2 Global    [CellName,Global,StatementKind]  decl 1.0.100.3 props Blueprint@1.0.107.3
```

What the numbers show:
- IDs follow **scan order**, not nesting: package 1; blueprint names (2); then bodies' methods and parameters (3, 4, 5);
  recipes (6); cells (7); then members (8, 9) and blocks (10, 11, 12) on demand. Compare `package.go:179-191`.
- `Scale` (3) was created in the package scope and then moved under `Point` (2) by `CheckReceiver`; it keeps
  `pos=1`, which now collides with member `X` (`pos=1`). Position is assigned once from the original parent
  (`model.go:698-700`).
- `X` and `Y` share one type node (`Blueprint@1.0.40.6`): `X, Y int` yields two identifiers from one declaration node,
  properties copied from the first (`node.go:758-762`).
- `X` counts `reads=3 writes=1` for three textual uses of which one is the write: the write is added to `Writes`, and
  its removal from `Reads` does not match (inferred from `id_scope.go:180-191`; counts observed).

Derived (observed): `BP Point => Point`, `BP Origin => Point`; `RECIPEINFO main: IsLeaf true, DirectCalls
(main.Point.Scale), Blueprints (main.Point)`; a query for label `FunctionDecl` with at least one child returns
`1.0.114.1` (`function_declaration`, rows 12..15) and not the method, which carries `MethodDecl`.

---

## 8. Plugging in another language "through config alone"

What is data (observed): the label tree with scores, levels, colours and `_Op` templates; the symbol-to-label table;
naming, context, matching, lookup and looping rules; the selector mini-language.

What exists to start a new language (observed, patched scratch, `lang_test.go`): `Language_t.Fill`
(`language.go:157-198`) falls back, when `<Lang>.yaml` and `<Lang>.json` are absent, to enumerating the grammar's symbol
table and regex-matching each symbol name against 18 patterns in `config/auto-labels.json` (`comment`, `expression`,
`literal`, `identifier`, `statement`, `function`, `type`, ...). Results on first run:

```
Python symbols=243 labeled=41 | TypeScript 356/125 | C 269/77 | CPP 395/129 | Java 292/99   (namingRules=0 for all)
def add(a, b): return a + b  ->  function_definition=function, identifier=identifier, parameters=parameter,
                                 return_statement=statement; binary_operator, block, def, return: no label
```

The run writes `<Lang>.json`; the next run converts it to a `<Lang>.yaml` skeleton (`Labels:` with lower-case auto
labels, `Operator`, `Unmapped`, empty `Rules`); the third run loads the YAML and logs `Label not found: comment`,
`... function`, `... identifier` and so on, because the auto labels are a separate lower-case vocabulary created with
`Make_Label(k, true)` (`config.go:456-462`) that does not exist in `labels.yaml`. After that only `Operator` survives
(and wrongly: `(`, `,`, `:` become operators). So the mechanism produces a **worksheet for a human**, not a mapping;
`Go.yaml` is that worksheet filled in by hand (commit `1e94aff`, 2023-02-14, "1st Attempt at Go Language Spec (yaml)").

What is compiled in and Go-specific (observed):
- the language set: `nums.Language_e` enum and the `TSLanguageFromNumsLanguage` switch (`nums/parser.go:29-47`,
  `:122-152`); Rust is not in it;
- `File_t.Get_Language` returns nil for anything but Go (`file.go:24-34`); folder/package scans filter on
  `Language_Go` (`package.go:149`, `repo.go:145`, `:323`);
- `GoPackage_t`, `go/build` import resolution, `_test.go` skipping, capital-letter visibility
  (`package.go`, `id_scope.go:35-78`);
- tree-sitter-go field and symbol names inside the engine: `"result"`, `"receiver"`, `"type_parameters"`, `"type"`,
  `"body"`, `"key"`, `"value"`, `"element"`, `"operand"`, `"identifier"`, `"struct_type"`, `"interface_type"`,
  `"type_identifier"` (`node.go:393`, `:673-689`, `:777-780`; `id_scope.go:549`, `:612-636`, `:716-719`;
  `implementation.go:631-641`);
- Go built-ins and idioms: `interface{}` -> `any`, `make`/`new`/`len`, `error` with an `Error` method, receivers
  (`id_scope.go:536-608`, `node.go:806-820`);
- the new model: `api_t.Language` accepts only `"Go"`, `LanguageFromExt` only `.go`, a `go_language` singleton with Go
  primitives (`implementation.go:60-77`, `:180-244`).

Conclusion: **classification** (symbol -> label) is config-only today and would carry to Python, TypeScript, C, C++,
Java for which grammars are already linked (observed up to the worksheet stage). **Naming, scoping, import and type
semantics** are partly declarative and partly Go code; a second programming language needs engine work (inferred from
the list above). Nothing was run for a second language beyond the auto-label probe. For a **natural language** there is
no parser, no label set and no rule in this directory; that part of the vision is not evidenced here (observed absence).

`config/id-sample.txt:13-52` sketches the target shape of an ID space as nested YAML, each name carrying `_Kind` and
`_Blueprint` as label paths (`Container->BlueprintBasic->Blueprint`) or references (`@model.Color_t`). It is a design
note; no code reads it (observed).

---

## 9. Does it build and run today?

Toolchain: `go version go1.27.2 darwin/arm64`. All commands ran in scratch copies with `GOPROXY=off` (no downloads).

### 9.1 As committed (observed)

| what | command | result |
|---|---|---|
| any module, workspace mode | `go build ./...` in `http/` | `go: cannot load module ../../[org]-go-tree-sitter listed in go.work file: ... no such file or directory` (`go.work:8`) |
| `nums/` | `GOWORK=off go build ./...` | every tree-sitter import fails: `replacement directory ../../[org]-go-tree-sitter does not exist` (`nums/go.mod:5`) |
| `http/model` | `GOWORK=off go build ./model/` | (a) `../nums/parser.go:24:2: cannot find module providing package github.com/smacker/go-tree-sitter/verilog`; (b) `go-echarts` not in the local module cache |
| `http/` (server + tests) | `GOWORK=off go build ./...` | the two above, plus `gin`, `gin-contrib/cors` not in the local module cache |
| `web-api/` | `GOWORK=off go build ./...` | `gin` not in the local module cache |
| `test/` | `GOWORK=off go build ./...` | same `verilog` failure through `nums` |

The `verilog` failure is structural, not a cache gap: the pinned upstream `go-tree-sitter` has no `verilog` directory
(checked in `~/go/pkg/mod/github.com/smacker/go-tree-sitter@v0.0.0-20230113054119-af7e2ef5fed6/`, and in the cached
2024-08-27 version). It came from the author's fork (commit `39e4b49`, 2023-02-27, "adding verilog from fork"). The
missing third-party modules (`gin`, `go-echarts`, `testify v1.8.1`) would need a network fetch, which I did not do.

**Net: nothing in the repository compiles on this machine as committed, and no committed test can run.**

### 9.2 In a patched scratch copy (observed)

Patches, scratch only: (1) drop the fork `replace` from `nums/go.mod`; (2) drop the `verilog` import and its `case` from
`nums/parser.go`; (3) cut the three `go-echarts` render functions from `model/sentenceHistogram.go`; (4) add
`//go:build ignore` to `api.go`, `histo.go`, `playground.go` (the gin server); `http/go.mod` rewritten to cached
versions (`testify v1.11.1`, `x/text v0.31.0`). With that, `nums`, `http/model`, `http/utils` and the test binary
compile.

Each of the 29 committed tests run on its own (a panic otherwise aborts the run):

- **23 pass**: `TestLabels`, `TestLanguageGo`, `TestGo_Labels_Package`, `_Import`, `_Struct`, `_Interface`, `_Mathods`,
  `_BuiltInFuncs`, `_Global`, `_Assignment`, `TestGo_IdScope_Context`, `_Locals`, `_Lambda`, `_Range`, `_Init`,
  `TestGo_NodeWalker`, `TestGo_HTML_Comment`, `TestLanguageCharp` (empty body), `Test_Query_File_api`,
  `Test_SentenceHistogram_Simple`, `Test_SentenceHistogram`, `Test_Query_Function`, `Test_API_Package`.
- **6 fail**:
  - `TestGo_Labels_Funcs`: expects `{19 2}=CommaBlock` for an empty parameter list, gets `ParameterDecl`. `Go.yaml:184`
    maps `parameter_list` to `ParameterDecl` and `:173` no longer lists it under `CommaBlock`: a stale expectation
    (inferred from the config; failure observed).
  - `Test_Query_Folder`: scans the repository root, which has no `.go` files; "No Nodes" (observed).
  - `TestGo_IdScope_BP2`: `readFileOS2` with unnamed results `(int, int, bool)` yields no blueprint ("Unhandled BP").
  - `TestGo_IdScope_BP`: **panic** `assignment to entry in nil map` at `blueprint.go:108` via `BlueprintSize.Add`
    (`id_scope.go:816-829` builds a zero `BlueprintSize` whose `Lists`/`Maps` are nil). It fires from the *argument* of
    a disabled debug print (`id_scope.go:368`), so any pointer-to-struct-with-a-list-or-map result triggers it.
  - `Test_Query_File_api_test`: nil-pointer panic in `BlueprintList.String` (`blueprint.go:463`), same pattern.
  - `Test_API_Import` (new model): nil-pointer panic at `implementation.go:1718` (section 5.2).

What the passing tests assert (`http/api_test.go`): for a Go snippet, the ordered list of leaf nodes as
`"{startByte level}=Label, Label"` strings (e.g. `"{18 3}=BlueprintName, Object"`, `"{221 6}=IdentifierWrite!A,
Member"`; `!A` = attribute label, `!G` = group label, `label.go:193-205`); the shape of the scope tree by name
(`Main` has children `a b c d e $block`; `$block` has `i` and `$if_statement`; ..., `:467-497`); inferred blueprint
names (`map[string]any`, `[]structA`, `*structA`, `complex128`, `:270-298`); method lookup through the receiver's
blueprint (`:638-643`); use counts on identifiers (`:776-799`); exact HTML spans with `nid` attributes (`:900-904`);
sentence-histogram cardinalities (`:1000-1002`, `:1027-1029`). `Test_Query_Function` and `Test_API_Import` assert
nothing.

Caveats on "pass": `[ORG]_ASSERT_` and `DEBUGBREAK_` only **log** (`nums/debugging.go`), so internal invariant failures
do not fail tests; passing runs still printed soft `ASSERT` lines (e.g. 25 in `TestGo_Labels_Import`, 3,613 in
`Test_Query_File_api` together with 477 "Unhandled BP" and 37 "Unresolved" lines while reading the Go 1.27 standard
library). `Test_Query_File_api` parsed the scratch `api.go`, which carries the added `//go:build ignore` line.
The byte-and-level expectations passing against the upstream grammar suggests the fork's Go grammar did not
differ for these snippets (inferred).

Logs kept in the scratchpad: `study/test-run-1.log`, `test-run-2.log`, `trace-old.log`, `trace-new.log`,
`probe-592a526.log`, `probe-lang.log` under
`scratch/`.

---

## 10. The other files in scope

- `query api and todo.txt` (115 lines): the span/label query design (section 6.2) and the principle
  "AST label: language + symbol; NUMS label: each AST label has a list of possible NUMS labels; leaf labels are most
  detailed, group labels provide a classification hierarchy" (`:60-62`).
- `http/html design.md` (34 lines): an ASCII wireframe of the browser (folder list, file view with Types/Functions/Code,
  info panel for node / identifier / function / label). Scores and ranks are `xx` placeholders.
- `http/playground.go`: scratch HTTP servers and the gin "albums" tutorial, plus `test_nums()` calling the
  first-generation parser. `web-api/main.go` (67 lines, module `example/web-service-gin`): the same gin tutorial; no
  NUMS content (observed).
- `http/repos.yaml`: 53 corpus repositories (section 6.4).

---

## 11. Proven versus intended

**Proven here (observed, patched scratch, upstream grammar):**
- One label tree + one per-language YAML turns a Go tree-sitter tree into Blueprint/Recipe/Cell-rooted labels with
  correct declaration/use/write roles on the tested snippets (23 passing tests, the section 7 trace).
- Nested identifier scopes with numeric IDs, anonymous `$` scopes, receiver re-parenting, on-demand import of real Go
  packages (the `log` package: 9 blueprints, 24 recipes, 10 cells), use/read/write/call edges with loop level.
- Type inference for the common Go shapes; recursion and call-set analysis per function.
- Name-erased sentences and corpus histograms; label and symbol histograms; filter + field-projection queries in-process.
- New model at `592a526`: named structs with members, single-parameter generic instantiation by shallow copy.

**Intended, not working or not present at HEAD:**
- The whole new API beyond package detection (regressed at HEAD; 73 stub methods); authors, versions, UI info.
- Generic substitution, multi-parameter generics, generic instance sharing.
- Hot score, formula scores, label confidence, query expression language, named selectors; `/api/nodes` returning nodes.
- Import classification rule (`Rules.Import`); `IsExtern`; recipe write sets.
- Any language other than Go beyond an auto-generated worksheet; any natural language.
- Stable identities: every numeric ID is an allocation counter or a position.
- A build on a clean machine: the tree-sitter fork is absent.

---

## 12. What the new design can take from this, and what it should leave

Keep:
1. Three roots — Blueprint, Recipe, Cell — crossed with Declaration / Use / Keyword as orthogonal bits
   (`interfaces.go:569-590`).
2. The lens as data: a neutral label tree plus a per-surface table from surface symbols to label sets, with the
   many-to-many reality acknowledged and resolved by context (`Go.yaml` `Labels`, `Cache_Prune`).
3. `_Op` templates with numbered holes: the same record names an operation and says how one surface prints it — the
   seed of a two-way lens (`labels.yaml:79-207`). Here it is one global surface and is used only for arity.
4. Declarative naming rules: `Name/Names/NoName`, `Kind`, `Scope`, `Properties`, `Match` over a small selector language.
5. "A package is a blueprint plus a recipe"; "every expression is a use of a built-in recipe"; "a recipe's signature is
   a blueprint with input/output/this".
6. Identifier = scope: one record that is both the name and the place its children live, with typed edges
   (uses, reads, writes, calls) carrying loop depth.
7. Name-erased sentences as a cheap structural fingerprint, and rarity against a corpus as a signal.
8. Field-projection queries: the caller lists the fields and the child filter.
9. Phased, lazy resolution: names of blueprints, then bodies, then recipes, then cells, then bodies on demand.

Leave:
1. Counter-allocated IDs and name-keyed identity; map-iteration-dependent numbering; process-local IDs written to disk.
2. Byte-offset node identity.
3. Global mutable singletons (`config`, `cache_g`, `go_language`) and `Cache_Reset` as the only isolation.
4. Language semantics inside the engine behind a config that looks general.
5. Assertions that only log; debug prints whose arguments run in production paths.
6. Two half-overlapping models in one package.
7. A build that depends on an unpublished sibling fork.
