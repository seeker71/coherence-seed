# Gap G6: is there a running witness for the object model?

Written 2026-11-24. Closes gap G6 of `critic.md` (multiple inheritance, objects as a data-pointer plus
interface-pointer pair, templates). Builds on `bml.md` 2.9 to 2.14 and `web-semantics-machines.md` 2.2 to 2.4;
does not repeat what they ground.

## 0. Paths, status words, what was run

- `K` = `coherence-kernel` (HEAD `b8ff978c9`, 2026-11-24)
- `T` = `Coherence-Network/docs/field/urs/artifacts/master-thesis-2000`
- `TT` = `T/backtracking-model-languages.txt` (footer July 5, 2000)
- `OBJ` = `T/companion/sgb-bml-objects.txt` (the companion object-model thesis, 2000; committed 2026-05-07)
- `A` = `~/Downloads/Angelic` (the 2000 archive, outside any repository; 2,776 files)
- `VM` = `A/BMCPU/CPU` (the 2000 virtual machine in C++; files dated 2000-04-13 to 2000-04-27)
- `JC` = `A/Java/jbml` (the 2000 BML compiler in Java; files dated 2000-01 to 2000-04-28)
- `BMA` = `S/pdftext/bma-reference.txt`, my `pdftotext -layout` copy of `A/SGB Documents/BMA Reference.pdf`
  (footer "May 22, 2000", file date 2000-05-26); line numbers below are lines of that text copy
- `S` = `scratch`
- `G` = `K/form/form-stdlib/grammars/bml.fk` (4,400 lines, 2026-11-19), `L` = `K/form/form-stdlib/bml/bml-form-lower.bml` (849 lines, 2026-10-11)

Status words: **observed** = I read the cited lines or ran the cited read-only command in this session;
**inferred** = my reasoning, basis named; **assumed** = not verified.

What I ran: no `fkwu`, no 2000 binary, nothing from the archive. Only readers: `git log / show / grep / ls-tree`
(history reads), `grep`, `sed -n`, `wc`, `ls`, `xxd -p` (hex dump of compiled class files), `unzip -p` (text of
`.docx` log containers), `pdftotext` (one PDF, output written into `S/pdftext/`). No repository file was modified.
One slip, repaired at once: a file list for the hex scan was first written to `/tmp` and removed in the same command.

"A record of a run" below means one of: a verdict pinned in a band head; a run log on disk; a compiled artefact that
only a compiler run could have produced. I did not re-run anything, so no claim here is "I saw it run".

## 1. Verdict in brief

Round one and the critic said: no running witness in either era. Read closer, that is half right, and the half that
is wrong matters.

1. **The reference pair ran in 2000.** The 2000 machine's source defines a reference as two 32-bit table indices
   (interface, object) with the native flag in the top bit of the interface index, and six run logs on disk count
   interface casts and interface-forwarded calls executed hundreds to thousands of times per run. observed (source,
   logs); the logs are undated containers of 2000-format output.
2. **The two-base rule is implemented in the 2000 machine and never appears exercised.** Every frame carries `self`
   and `this`; native methods receive both. But the only instructions that make `self` differ from `this`
   (`CALLD`, `FDC`) have no row in any of the six logs, and the logger prints every instruction that ran at least
   once. observed (source, logs), inferred (zero executions).
3. **Delegation was compiled in 2000, not seen executed.** The 2000 compiler turns every additional class parent into
   a hidden delegated field and emits forwarding stubs; 64 such stubs sit in five compiled class files dated
   2000-04-27 and 2000-05-01. The one 2000 test source with two class parents (`class C : A, B`) has no compiled
   output and no load record. observed.
4. **Delegation through a second class base has a 2026 record of a run, and it was deleted.** On 2026-09-15 the
   fixture `bml-class-delegation.bml` (a delegated method calling a method the derived class overrides; answer 47)
   stood under a band head pinning "Verdict 119" at a commit where exactly 119 fixtures declared an answer. The
   2026-09-29 cut removed that fixture and four sibling delegation fixtures. At HEAD nothing exercises delegation.
   observed (git history).
5. **But the 2026 "delegation" is flattening.** The delegate's fields ride in the one record; there is no separate
   base object, one receiver, no interface id in the reference. The thesis rule holds there only because `self` and
   `this` are the same record. For a field marked `delegate` (a genuinely separate object) the 2026 lowering
   re-sends to the field's value, which makes the delegate the receiver: `self` is lost. observed (source), inferred
   (behaviour; no fixture ever tested it).
6. **Templates by specialisation ran in both eras.** 2000: instance class files such as
   `Array[BML.lang.String].bmc` exist and are named in load logs. 2026: `bml-class-templates.bml` (answer 133) is
   among the 26 fixtures under the pinned "Verdict 26" at HEAD. Parameter bounds were parsed in 2000 and enforced in
   neither era. observed.
7. **Shared inheritance, tagging, detached interfaces: no implementation that I could find executed in either era.**
   Casts: implemented and executed in 2000; absent from the 2026 lowering. observed.

So critic contradiction C5 (delegation with two bases, or flattening) is not a choice between a proven thing and a
proposal. Each side has exactly one partial record: the pair and its casts from 2000, flattened multiple bases from
2026. The combination the request calls proven (separate base objects, `self` kept while `this` moves, running) has
none.

## 2. The reference: exact representation

### 2.1 As specified (2000)

- "The BML object model only prescribes one data-type: the reference. A reference is composed of three parts: the
  object identifier, the interface identifier, and the native flag. Both identifiers are indices into the object
  look-up table; the native flag indicates that the object identifier part actually holds the object's value"
  (`OBJ:486`). observed.
- Native reference: flag set, object part is the value, no descriptor, cannot be aliased; used for integers, floats,
  booleans (`OBJ:490`). Both kinds carry an interface identifier (`OBJ:492`). observed.
- "two object references with the same object identifier could use two different interface identifiers. Hence, the
  same object data would be used by different implementations" (`OBJ:493`). The facade alternative was dismissed
  "because it required the creation of many more instances" (`OBJ:493`). observed.
- Widths: 32-bit identifiers, 64-bit references, native values 32 bits, "the native flag is stored in the most
  significant bit of the interface identifier", at most 2^31 objects (`OBJ:495`). observed.
- Identity survives a change of view: "the same object can be referred to in other locations with different
  behaviors, yet the object identifier remains the same. This identity property is not upheld when using proxy
  classes" (`OBJ:701`). observed.
- The earlier proposal says the same with pointers: "combine the VMT reference and the data reference into a single
  object reference", and "32-bit integers are composed of the integer interface reference and the integer value"
  (`T/companion/sgb-thesis-another-object-model.txt:93`). observed.

### 2.2 As built (2000 machine source)

| Fact | Source | Status |
|---|---|---|
| `typedef struct _BMREF { INDEX ndxInterface; INDEX ndxObject; } BMREF;` with `INDEX` = `unsigned int` | `A/BMCPU/Include/bmvm.h:149,180-183` (2000-04-27) | observed |
| `#define BMNATIVE_FLAG 0x80000000`, `BMNATIVE_MASK 0x7FFFFFFF` | `bmvm.h:124-125` | observed |
| `struct REF { INDEX m_ndxInterface; union { INDEX m_ndxObject; FLOAT m_fValue; }; }` | `VM/descriptor.hpp:141-149` (2000-04-25) | observed |
| `IsNative()` tests the flag on the interface word; `Interface()` masks it off | `descriptor.hpp:165,181` | observed |
| Both halves index one table: `InterfaceDescriptor()` is `OLT[Interface()]`, `ObjectDescriptor()` is `OLT[Object()]` | `descriptor.hpp:884-895` | observed |
| Null is interface 0 and object 0 | `descriptor.hpp:164`; `bmvm.h:61` | observed |
| Descriptor as exported: index, counter, fixed count, dynamic count, binary size, instantiator reference, binary / dynamic / persistent flags | `bmvm.h:214-228`; matches `OBJ:506-526` | observed |

So an interface is itself an object in the same table: its dynamic part is the method list
(`sInterfaceDesc.DynamicAt( iIndex )`, `VM/instructions.hpp:144-154`), and its fixed field `INTERFACE_m_hBase` is the
base interface (`instructions.hpp:100-103`). An instantiator's dynamic part is the list of interfaces its instances
support (`VM/bmvm.cpp:970-972`; `OBJ:573`). observed.

Inferred, from the rows above: in today's words the 2000 reference is already "a pair of node ids in one node
space, plus an immediate mode". The object look-up table plays the part of the node table; the native flag plays the
part of the immediate-integer mode bit that `fkwu`'s node word has (`critic.md` R3).

## 3. The two-base call rule

### 3.1 As specified

- "Rather than having one base object reference, BML uses two. The first argument refers to the behavioral base and
  the second argument refers to the structural base. The behavioral base is always used to dispatch methods. The
  behavioral base is responsible for locating the appropriate structural base. The structural base is always used for
  structural access." (`OBJ:642-644`). observed.
- Rules of use and of update, from the proposal: "self is used when a method is invoked; this is used when data
  members are accessed ... calling a method on an object sets self and this to the foreign object's reference;
  forwarding a method to a delegation pointer sets only this to the delegation object reference"
  (`sgb-thesis-another-object-model.txt:103`). observed.
- Registers: `self` "contains the reference to the behavioral base object. It is used to invoke methods on the base
  object. Furthermore, the program can read its contents." `this` "contains the reference to the structural base
  object. It is used to access fields on the base object. However, the program cannot read its contents."
  (`BMA:528-541`). observed.
- Worked example `class A { void foo() { goo(); } void goo() {...} } class B {} class C : A, B {} class D {}
  class E : C, D { void goo() {...} }`: a call of `foo` on an `E` moves `this` to the C part, then to the A part,
  while `self` stays on E; `foo`'s call of `goo` goes through `self` and lands in E's `goo` (`OBJ:719-738`). observed.
- Dispatch is by index, possible because each interface keeps its own method table (`OBJ:640`); indices are bound
  at load time from symbolic names (`sgb-thesis-another-object-model.txt:95`). observed.

### 3.2 As built: the seven ways a frame is made

Every frame is `CONTEXT_FRAME { REF refSelf; REF refThis; REF refMethod; ... }` (`VM/cpu.hpp:35-46`), made by
`CreateContextFrameDo( refSelf, refThis, refMethod )` (`VM/cpu.cpp:719-743`). A native method has the signature
`( BMVM pCPU, CBMREF refSelf, CBMREF refThis )` (`bmvm.h:187`) and is entered with `Self()` and `This()`
(`cpu.cpp:864`). Field access reads `CPU.This()` and that reference's object descriptor
(`VM/instructions.hpp:191-220`). All observed.

| Form | New `self` | New `this` | Method found in | Source (all observed) |
|---|---|---|---|---|
| `CALL reg, m` (send to another object) | reg | reg | reg's interface table at index m | `instructions.hpp:1148-1162`, `1189-1203`, lookup `116-162` |
| `CALL self, m` | self | self | self's interface table | `instructions.hpp:1210-1223` |
| `CALL super, m` | unchanged | (base interface of `this`'s interface, same object) | that base interface; loops upward while it finds the running method again | `instructions.hpp:100-103`, `133-158`, `1169-1182`, `1230-1243` |
| `CALLD reg, m` (delegated call) | unchanged | reg | reg's interface table | `instructions.hpp:1250-1285`; `BMA:1423-1437` |
| `FDC field` + `FCX method` (forwarding stub) | unchanged | the reference stored in that fixed field of `this` | that reference's interface table; the stub's own frame is replaced | `instructions.hpp:1317-1392`; `BMA:1829-1845` |
| `FIC iface` + `FCX method` (interface stub) | `self` cast to iface | same as new self | the cast reference's interface table; frame replaced | `instructions.hpp:1398-1472` |
| host call `BMInvokeByIndexDo` | object | object | object's interface table | `VM/bmvm.cpp:1523-1545` |

Two precise readings follow, both inferred from the table:

1. "The behavioural base dispatches" is true of *sends from inside a method* (`CALL self` uses `self`'s table and
   resets `this` to `self`). The method index is always looked up in the interface table of the reference that becomes
   the target; for `CALLD` and `FDC` that is the delegate's table, with `self` carried along unchanged.
2. `super` is a change of the *interface half* of `this` with the object half kept. The pair is what makes `super`
   expressible as a plain reference.

A small disagreement between document and code: the reference manual says `FDC`'s constant is the method and `FCX`'s
is the field (`BMA:1833-1834`); the machine and the compiler use `FDC` field, `FCX` method
(`instructions.hpp:1330,1355`; `JC/ASM/Method.java:90-99`). observed.

### 3.3 A hazard in the 2000 machine where delegation meets backtracking (inferred)

On return, a frame is pushed on the backtrack stack from `refThis` onward, leaving out `refSelf`
(`cpu.cpp:771`, with `refSelf` first in the struct, `cpu.hpp:39`). When backtracking re-enters a finished call, the
frame is rebuilt by `DestroyContextFrameUndo( bDelegated )`, which runs `if( !bDelegated ) Self( This() );`
(`cpu.cpp:780-790`); the flag is `false` for `CALL` and `true` for `CALLD` (`instructions.hpp:1156-1157`,
`1258-1259`), and the stack's `Enter()` does not initialise the slot (`VM/stack.hpp:169-183`). `FDC` itself refuses
both redo and undo (`instructions.hpp:1381-1387`). observed.

Inferred from those lines: (a) re-entering a callee that was reached through a forwarding stub restores
`self := this`, the delegate, because the calling instruction was a plain `CALL`; (b) re-entering a `super` callee
restores `self` with the base interface's id; (c) re-entering a `CALLD` callee keeps whatever bytes the slot held.
The two-base rule and undo were therefore not jointly sound in the 2000 source as written. I did not run it; no log
shows the path taken.

## 4. Delegation, shared inheritance, tagging, detached interfaces, casts

### 4.1 Definitions (2000 text)

| Concept | Definition | Source | Status |
|---|---|---|---|
| Structural inheritance | Base fields then derived fields in one fixed part; single parent only | `OBJ:469-475`, `OBJ:706`; `TT:234` | observed |
| Behavioural inheritance | Merge of method definitions, most derived wins | `OBJ:708` | observed |
| Delegation inheritance | "does not include the fields defined by the base classes; rather it uses a reference to the instances defined by them"; two references let a call be delegated "yet retain the true object's scope" | `OBJ:711-713` | observed |
| Multiple inheritance | The first parent is structural; "each additional parent will generate a delegated field. Each delegated field will add all its interfaces to the class. For all the methods that are not implemented in the delegated interface, delegate deferred calls will be generated." | `TT:234`, `TT:722-723` | observed |
| `delegate` on an inheritance clause | derived class holds a hidden field instead of the base structure. "[Not supported yet]" | `TT:656` | observed |
| `shared` on an inheritance clause | "the reference to the base class is provided during creation. Therefore, multiple instances can share the same instance of the base class. [Not supported yet]" | `TT:657` | observed |
| `delegated` on a field | "this field delegates all method calls to the default interface" | `TT:661` | observed |
| Shared inheritance | "a byproduct of delegation": two true objects share one base object; "at this point, it lacks applications" | `OBJ:744-749` | observed |
| Common objects | Class-level state lives in a common object; a derived common object "always uses delegation inheritance to its base common object" | `OBJ:660-662` | observed |
| Tagging | An application of shared inheritance: `class ContainerItem : ITEM [tag]` adds information to an existing object; needs normalization (tags attach to the untagged base, one tag per type). "BML only supports at this point shared inheritance without the necessary normalization mechanism" | `OBJ:752-775` | observed |
| Detached interfaces | Interfaces defined on top of an existing class; at cast time, if prerequisites hold, the interface is added to the instantiator's list. "the current language implementation does not have the necessary constructs" | `OBJ:787-791` | observed |
| Interface cast | Walk the instantiator's interface list; if found, "a new object reference composed of the found interface and the original structure"; else a casting exception. Natives answer through an `Instantiator` method. Search is O(n) | `OBJ:649-653`, `OBJ:821` | observed |
| Cost | "considerable performance loss during runtime"; calls start at the true object and walk down; special instructions recycle the activation record | `OBJ:740`, `OBJ:796` | observed |

### 4.2 What the 2000 compiler and machine actually do

- **Additional class parent becomes a hidden delegated field.** For each named super that is a class other than
  the structural base: `m_hCompiledClass.AddDelegatedField( hClass )` (`JC/Components/Class.java:495-521`). The field
  is named `<Class>#m_h$Delegated$<Type>`, typed by the parent's default interface, marked `delegate` and `hidden`
  (`JC/ObjectModel/BMLInstanceDefinition.java:77-78,99-110`). observed.
- **Its interfaces join the class, marked `delegated`.** `AddAllInterfaces( ..., "delegated", hField.GUID() )`, and
  the field becomes a constructor parameter (`Class.java:524-532`). The default constructor takes one parameter per
  delegated parent field and passes them to `super` (`JC/Components/Method.java:406-430`). observed. Inferred: the
  2000 compiler therefore implements the *supplied-at-creation* form; a delegated base instance is always handed in,
  which is the thesis's `shared` meaning even though that attribute is marked unsupported. The test source agrees:
  `C hC = C( B() );` (`A/src/Test/ClassTest.bml:32`).
- **Forwarding stubs are generated.** For every method of a `delegated` interface that the class does not
  override: a two-instruction method `fdc <field>; fcx <method>`
  (`Class.java:311-337`; `JC/ASM/Method.java:90-99`). For interface-to-default-interface bridging: `fic <interface>;
  fcx <method>` (`Class.java:270-299`; `ASM/Method.java:80-89`). observed.
- **Common objects use the same mechanism.** A class whose base has class-level members gets a common object with a
  delegated field to the base's common object (`Class.java:477-489`). observed.
- **No inheritance attributes in the grammar.** `SuperClass ::= ClassDecl( 3 )` carries no property list; the only
  `delegate` word is a member property (`JC/JBML.BMF:43-44,149`). No rule spells `tag`. observed. This reconciles
  `TT:656` ("[Not supported yet]") with the code: the *attribute* is absent, the *mechanism* is present and automatic.
- **Cast as built.** `BMObjectCast` returns at once if the reference already carries the wanted interface, refuses a
  native, then loops over the instantiator's interfaces and each one's chain of base definitions comparing 128-bit
  GUIDs; on a match it rewrites only `pValue->ndxInterface`; otherwise `BM_NOTFOUND`, which the `CAST` instruction
  turns into an `INVALID_CAST` exception (`VM/bmvm.cpp:952-992`; `VM/instructions.hpp:235-282`). Nothing adds an
  interface to an instantiator at cast time. observed. So detached interfaces are not in the machine.
- **`CALLB`** (blocks) is a `HALT()` stub marked TODO (`instructions.hpp:1289-1310`). observed.

### 4.3 What the 2026 body does (thesis lane, reader "b" of `bml.md` 1.2)

- First super is the structural base unless it names an interface; later class supers are delegation bases
  (`G:4254-4257`, `G:219-220`). observed.
- One lookup order for members, fields, interfaces and subtyping: the class, then each super in declaration order,
  depth first, each class once (`G:3339-3347`). observed.
- **The record holds every field of that whole chain**: `bml-hati-class-fields-all` is the fields of the reversed
  depth-first chain, delegation bases included (`G:3348-3350`), and `new` builds one Form record with `bmlClass`,
  `bmlClassId` and those field rows (`L:639-645`). A name declared by two classes of one chain gets the key
  `Class.name` (`G:3352-3360`). From outside, a field resolves along the structural chain only, "never a delegate.
  Delegation lends methods and subtype, not fields" (`G:3363-3371`). observed.
- **One receiver.** A member is one function over the packed list `(self a0 a1 ...)`; `self`, `this` and `super`
  all lower to the head of that list (`L:598-604`, `L:610`); a member call is `(fn (list recv args...))` (`L:633`).
  observed.
- **Dispatch by class id, no interface id.** A signature that two classes answer differently gets a stub that reads
  `bmlClassId` from the head and walks an if-chain, one row per class id (`G:4279-4291`, `L:683-685`). Which
  implementations can be reached is narrowed by the receiver's *static* type (`G:3436-3440`). observed.
- **A field marked `delegate`** takes a call its class does not answer: the call is lowered again on
  `forward-get(receiver, field)` (`G:3634-3647`, `G:3679-3683`, `L:632`). observed.
- **A cast is read, never performed.** The word stands on 13 lines of `G` and on none of `L`. The thesis rulebook
  reads the operator as source text (`op-type-cast ::= "(" type-cast-op ")" => bml-emit-node-source`, `G:1057`;
  `TypeCastOp` in the list of reference rule names, `G:792`). A source-walk helper recognises the eight-token shape
  `T x = ( T ) y ;` and binds `x` to `y`'s value unchanged (`G:353-356`, used at `G:490-491`). No `bml-hati-*`
  lowering names a cast, and no reference changes its view, since a reference has none. observed. No `tag`, no
  detached interface, no base supplied at creation.
- A class-level model of the two-base rule also stands in `G`, over class *names*: `bml-method-binding(self-class,
  this-class, method)`, `bml-invoke-from-context` (looks up from the self class) and `bml-super-invoke`
  (`G:648-668`). At HEAD only their definitions mention them (`git grep`, observed); the band that called them,
  `tests/bml-class-inheritance-proof.fk`, left in the 2026-09-29 cut.
- The other two 2026 readers have no objects: reader "a" erases bases (`bml.md` 2.12); in reader "c" a class is a
  grouping and "the type parameter is a contract that erases" (`K/surface/core-class-surface.bml:4-10`, 2026-10-09,
  read in full: four generic classes `Num<T>`, `List<T>`, `Cell<T>`, `Task<T>` whose methods hoist to bare top-level
  definitions; no instance, no dispatch, no inheritance). observed.

### 4.4 The question asked: a method inherited through a delegation base in 2026

Two different things carry the word.

**(a) A second class base** (`class E : A, D`). `D`'s method is found on `E`'s chain and called with the same
receiver record; `D`'s fields are slots of that record; inside `D`'s code a call on `this` dispatches by the record's
class id, so `E`'s override answers. This is the behaviour the thesis prescribes, reached by flattening: there is no
`D` object, `self` and `this` cannot differ, a base cannot be shared between two objects or supplied at creation.
observed in source (`G:3343-3350`, `L:604,633,645,684-685`); the deleted receipt says the same in words: "The
delegate's fields sit in the delegating record, not in a separate delegate object"
(`git show 115fea55e^:receipts/2026-09-15-the-receiver-gets-its-own-slot.md`, line 234 of 676; line 237 reads
"Witnessed on fkwu (fixture `bml-class-delegation.bml`, 47)"). Record of a run: section 6, row R6.

**(b) A field marked `delegate`** (`Shape s [delegate];`). The call is re-sent to the field's value, so the
delegate object becomes the head of the packed list: inside the delegate's method, `this` *and* `self` are the
delegate. A method of the delegate that calls a method the outer class overrides will run the delegate's own. The
thesis rule (keep `self`, move `this`) is not preserved. observed in source (`G:3683`, `L:632`); the behaviour is
inferred, and no fixture in either the present tree or the pre-cut tree tests it: the two pre-cut fixtures for this
path forward a method that returns a constant
(`git show 115fea55e^:form/form-stdlib/tests/fixtures/bml-class-field-delegate.bml`, answer 10;
`bml-class-delegate-private.bml`, answer 10).

## 5. Templates and generics

### 5.1 2000

- Scope: "Templates are restricted to class and interface definitions" (`TT:234`, quoted in `bml.md` 2.9); a class
  template "defines the layout and operations for an unbounded set of related types" (`TT:755`). observed.
- Parameters: `TemplateParameter ::= InterfaceRef FieldName [ "=" InterfaceRef ] | Const | "interface" FieldName
  [ "=" InterfaceRef ]` (`TT:757`; same in `JC/Components/TemplateParameter.java:45-47`). So a parameter is a type
  bounded by an interface with an optional default, or a constant, or an unbounded interface name. observed.
- Bound enforcement: the bound is stored and compiled (`TemplateParameter.java:16,33`), and argument resolution
  does not consult it: `Resolve` returns the argument's reference or the default, with the note
  `/* TODO: get type from template */` (`TemplateParameter.java:21-29`). observed. Inferred: bounds were syntax, not
  a check. I did not read every caller.
- Specialisation: by substitution. `Template.Instantiate` builds a new template around `m_hClass.Instantiate(
  hArguments )`; `Arguments` maps parameter names to resolved arguments; `Signature` joins them with commas
  (`JC/Components/Template.java:30-55`); the instance is named `Name[arg,...]`
  (`JC/Components/FullClassName.java:86`). observed.
- Per-instance identity: `"<" Signature ">" "=" "[" Properties "]"` gives a chosen GUID to one instance, or to one
  instance's field (`TT:756`); used as `class Array [ dynamic=object, <Object>=[ guid={...} ], <Method>=[ guid={...}
  ], ... ]` (`A/src/Test/TemplateTest.bml:16-26`, 2000-04-26) and per field in
  `A/src/Test/SmallTemplateTest.bml:7-13` (2000-04-21). observed.
- Nested templates and inner classes of templates appear in the test source (`template<Object TYPE> class
  Enumerator : Enumeration<TYPE>` inside `HashT`, `SmallTemplateTest.bml:15`). observed.
- Arrays are template instances: "`int[]` creates an instance of `Array<int>`" (`bml.md` 2.10, citing
  `T/companion/bml-search-algorithms.txt:1004`); the test writes `ArrayTest[] hArray = Array<ArrayTest>();`
  (`A/src/Test/ArrayTest.bml:5`). observed (test source), cited (thesis line).

### 5.2 2026

- Thesis lane: a template is a node of parameter names plus a class or interface; "Every Name<args> a unit names
  instantiates it: type positions (field, parameter, const, local, new, base, super) are substituted, a generic
  method's own type parameters shadow the template's, and the instance enters the layout as a class named by the
  whole type" (`G:4095-4100`). Substitution is textual over identifier words (`G:4101-4110`). Wrong arity and
  unspecialised use are named refusals (`G:4160`; `L:643`). observed.
- Bounds: "template-kind-constraints" is in the unsupported list, with the pointer "which kinds a template may wrap
  (757)" (`K/grammars/bml-native-north-star.form:32-33`, the `current-unsupported` and `next-code-point classes` rows;
  2026-11-04). observed.
- Overloads are chosen per instance by a type score (same type 3, subtype or unknown 2, type parameter 1)
  (`G:3283-3288`). observed.
- Maintenance dialect: generic class headers are read and erased (4.3 last bullet).

## 6. Execution evidence, feature by feature

`R` rows are the records I found. "Not run by me" holds for every row.

### 6.1 Records

| # | Era | Record | What it shows | Source | Status |
|---|---|---|---|---|---|
| R1 | 2000 | Six instruction-count logs. Rows present in every one: `CastRegConstant` (426 to 1,909 per run), `ForwardInterfaceCallConstant` (92 to 2,322), `CallSelfConstant` (610 to 2,814), `CallSuperConstant` (44 to 281). Example: `CallRegConstant 12804`, `CastRegConstant 1867`, `ForwardInterfaceCallConstant 260`, total 66,206 instructions | `A/src/performance.log.docx`, `A/BMCPU/Performance/{performance,original,fast bit push,slow bit push}.log.docx`, `A/BMCPU/Debugger/performance.log.docx` (text read with `unzip -p ... word/document.xml`) | observed. Container files are dated 2024-09-19 (zip entries 2026-05-06); the content carries no date. The format is the one `VM/performance.cpp:8-51` (2000-04-15) prints |
| R2 | 2000 | The same logs have **no** `ForwardDelegatedCallConstant` row and **no** `CallDelegate...` row | the logger skips a timer only when its count is 0 (`performance.cpp:22`), and each instruction's timer is named after its handler struct (`VM/instruction.hpp:160-165`) | observed (absence, rule); inferred: `FDC` and `CALLD` executed zero times in every recorded run |
| R3 | 2000 | 64 `fdc`/`fcx` stub pairs in five compiled class files: `BML/io/FilterInputStream.bmc` 8, `FileInputStream.bmc` 8, `BufferedInputStream.bmc` 16, `LineNumberInputStream.bmc` 16 (all 2000-04-27), `Test/BMFTest.bmc` 16 (2000-05-01). 250 `fic`/`fcx` pairs in 33 files. 202 class files scanned | hex scan of `A/classes/**/*.bmc` for the little-endian words `....5xfc ....7xfc` at byte alignment; instruction encoding from `JC/ASM/Statement.java:42-53` and `JC/ASM/Instructions.java:62-64`; a match reads e.g. `020050fc 030070fc` (`fdc [2]; fcx [3]`), eight in a row 28 bytes apart | observed |
| R4 | 2000 | None of those five classes declares a second class parent or a `delegate` field (`A/src/BML/io/*.bml`, `A/src/Test/BMFTest.bml`, read in full); their roots declare class-level members (`A/src/BML/io/InputStream.bml:6`, `A/src/BMF/Syntax.bml:114`) | so the stubs are the common-object delegation of 4.2 | observed (sources); inferred (cause) |
| R5 | 2000 | `A/src/Test/ClassTest.bml` (2000-04-27): `class C : A, B`, `C hC = C( B() ); hC.Test();`. No `C.bmc`, `DTest.bmc` or `ClassTest*` under `A/classes`; no load line for it in any of the five `bml.log` files (`A/src`, `A/Java`, `A/Visual`, `A/BMCPU/Performance`, `A/BMCPU/Debugger`) (which do name `Test/BMFTest.bmc`, `Test/Ex.bmc`, `Test/Output.bmc`, `Test/TemplateTest.bmc`) | the one 2000 multiple-inheritance test has no record of having compiled or loaded | observed |
| R6 | 2026 | Commit `1044906a5` (2026-09-15 19:45): `tests/bml-native-fixtures.fk` head says "Verdict 119: every fixture answers what its head declares"; at that commit 139 `bml-*.bml` fixtures exist and exactly 119 declare `Answer:`; `bml-class-delegation.bml` is one of them, declaring 47 for `class E : A, D` where `D.DOnly()` returns `this.d + this.Who()` and `E` overrides `Who` | a pinned verdict covering a delegated method that reaches an override | `git show 1044906a5:...`, counts by `git ls-tree` plus `grep` (observed). The commit that added the fixture says "Fixture: bml-class-delegation.bml (47)" (`30cb80408`, 2026-09-15 16:47) |
| R7 | 2026 | Commit `115fea55e` (2026-09-29, "Cut the repo ... 14,729 files to 1,628") deletes `bml-class-delegation.bml`, `bml-class-delegate-slots.bml`, `bml-class-field-delegate.bml`, `bml-class-delegate-private.bml`, `bml-class-delegate-field-hidden.bml`, `bml-class-interfaces.bml`, `bml-class-interface-default.bml`, `bml-class-static-field.bml`, 30-odd other class fixtures, `tests/bml-class-inheritance-proof.fk` (164 lines) and `tests/bml-hati-native-class-dispatch-proof.fk` | the delegation witnesses left the tree | `git show --stat 115fea55e` (observed) |
| R8 | 2026 | Just before the cut the pin was stale: 173 fixtures, 152 declaring an answer, head still "Verdict 119" | the pin is evidence for 2026-09-15, not for 2026-09-29 | `git show 115fea55e^:...` (observed) |
| R9 | 2026 | HEAD: `tests/bml-native-fixtures.fk:11-12` (2026-10-03) "Verdict 26: ... this tree holds 26 of 26"; the tree holds 28 `bml-*.bml` fixtures, 26 declaring an answer. Among them `bml-class-dispatch.bml` 63 (three-level single inheritance, `this.Area()`, `super.Area()`), `bml-class-interface-dispatch.bml` 21, `bml-class-templates.bml` 133, `bml-class-construction.bml` 84, `bml-class-refusals.bml` | pinned verdict at HEAD for single inheritance, interface dispatch and template specialisation | observed. `K/form/band-verdicts.txt` (98 lines) has no row for this band; the pin is in the band head only |
| R10 | 2026 | `K/form/band-verdicts.txt:28`: `bml-full-class-model-proof fks 2147483647`. The band (`tests/bml-full-class-model-proof.fk`, 248 lines, 2026-11-17, read in full) builds AST components by hand and scores 31 bits, sum 2^31 - 1 | It checks flag readers and lookups over a class *model*: `bml-field-delegate?`, `bml-field-shared?`, abstract / final, access levels, structural constant and inner-class lookup, template kind validity, interface default lookup, `Application.Main`. It constructs no object and calls no method | observed (`:9-248`; the delegate bit is `:167`, the flags `:68-78`) |
| R11 | 2026 | `tests/bml-generics-band.fk` (81 lines, 2026-09-03): parses `class Map<K,V>`, a generic method, a nested instantiation, with reader "a"; "Expected total: 16777215" in a trailing comment | parse shape only; no verdict row, no `; Verdict` head | observed |
| R12 | 2000 | Template instances as compiled files: `A/classes/BML/lang/Array[BML.lang.String].bmc`, `HashTable[BML.lang.String,BMF.container.Rule].bmc`, `Dictionary[...]`, `Enumeration[...]`, `Stack[...]`, `Association[...]` (2000-04-27); load logs name them, e.g. `Loading 'BML/lang/HashTable[BML.lang.String,BML.lang.Attribute].bmc'` (`A/src/bml.log.docx`) and `Loading 'Test/HashT[BML.lang.Object,Test.TemplateTest].bmc'` (`A/Visual/bml.log.docx`) | specialisation by substitution compiled and loaded; the runs of R1 load them too | observed |

### 6.2 Summary matrix

| Feature | 2000 defined | 2000 built | 2000 record of execution | 2026 built (HEAD) | 2026 record of execution |
|---|---|---|---|---|---|
| Reference = (object id, interface id, native flag) | yes (`OBJ:486`) | yes (`bmvm.h:180-183`) | yes: R1 (casts and interface forwards executed) | no: one record with a class id inside (`L:645`) | n/a |
| Two bases, `self` dispatches, `this` reads | yes (`OBJ:642-644`) | yes (`cpu.hpp:35-46`) | only with `self` = `this`: R1, R2 | no: one receiver (`L:604,610`) | n/a |
| Interface cast | yes (`OBJ:649-651`) | yes (`bmvm.cpp:952-992`) | yes: R1 | no (4.3) | none |
| `super` as interface change | implied (`OBJ:611`) | yes (`instructions.hpp:100-103`) | yes: R1 (`CallSuperConstant`) | static choice from the base class (`L:631`) | R9 (`bml-class-dispatch` 63) |
| Single structural inheritance | yes | yes | yes (whole library; inferred from R1, R12) | yes | R9 |
| Multiple class bases by delegation to separate base objects | yes (`OBJ:711-738`, `TT:722-723`) | compiler and VM yes (4.2) | **none**: R2, R5 | no | none |
| Multiple class bases, flattened into one record | not in 2000 | no | n/a | yes (`G:3343-3350`) | R6 at `1044906a5` only; **none at HEAD** (R7) |
| Field marked `delegate` forwards calls | yes (`TT:661`) | yes (same `FDC` stub) | none: R2 | yes, as a re-send that loses `self` (`G:3683`) | R6 era: forward of a constant method only; none at HEAD |
| Common-object delegation (class members) | yes (`OBJ:662`) | yes (`Class.java:477-489`) | compiled (R3, R4), not executed (R2) | class field is a slot in program state (`L:652-657`) | pre-cut `bml-class-static-field.bml` (249) under R6's pin if present then; not at HEAD |
| Shared inheritance | yes (`OBJ:744`; `TT:657` unsupported) | base instance is a constructor parameter (4.2) | none (R5) | flag read only (`G:4057`) | R10 reads the flag |
| Tagging | yes, without normalization (`OBJ:752-775`) | not found in grammar | none | not found | none |
| Detached interfaces | yes, "by its architecture" (`OBJ:787-791`) | not in `BMObjectCast` | none | not found | none |
| Templates, specialisation by substitution | yes (`TT:755-757`) | yes (`Template.java:30-55`) | compiled and loaded: R12 | yes (`G:4095-4100`) | R9 (`bml-class-templates` 133) |
| Template parameter bounds | syntax (`TT:757`) | parsed, not checked (5.1) | n/a | unsupported (5.2) | none |
| Native values inside the reference | yes (`OBJ:490`) | yes (`descriptor.hpp:154-172`) | inferred from R1 (integer arithmetic ran) | host values | n/a |

## 7. Corrections to round one and to the critic

1. `critic.md` G6 bullet 1 and section 6 item 5 ("I found no run of either, in 2000 or in 2026"): the reference
   pair has 2000 run logs (R1); flattened multiple bases has a 2026 pinned verdict in history (R6). What has no run
   is the row of 6.2 named "Multiple class bases by delegation to separate base objects".
2. `bml.md` 2.12: "`delegate` ... marked [Not supported yet]" is true of the inheritance *attribute*; the automatic
   mechanism for additional class parents was implemented in the 2000 compiler and machine (4.2).
3. `bml.md` 2.12: "Not witnessed by a fixture I could find" is true at HEAD and false of history: five delegation
   fixtures lived from 2026-09-15 to 2026-09-29 (R6, R7).
4. `bml.md` 2.13: the inference that the 2026 delegate path loses `self` is right for a *delegate field* and does
   not apply to a *delegation base*, where no second object exists (4.4).
5. `K/grammars/bml-native-north-star.form` still lists `multiple-class-bases`, `delegate-field-slots` and
   `delegate-field-visibility` as supported and says `band-verdicts.txt` registers "the class-dispatch proof ... the
   class-model and inheritance proofs". At HEAD the verdict file has only the class-model row; the other two proofs
   and every fixture behind those three entries are gone (R7, R10). The compiler code is there; the claim has no
   standing witness. observed.
6. `T/README.md:125` (the lineage note that `NamedCell{Recipe access, Base blueprint, Name, CTOR recipe}` "is
   structurally a BML-style reference"): the struct is `NUMS.Go/nums/recipe.go:497-502`
   (2023-02-13), a declared slot, as the critic's G2 says. It pairs a base blueprint with an access recipe at
   *declaration*; nothing in NUMS executes, so it is a resemblance in shape, not a witness. observed (struct),
   inferred (assessment).

## 8. The object model in terms of blueprint, recipe and cell

Everything in this section is **inferred**: a formulation, built from sections 2 to 4, from
`web-semantics-machines.md` 2.2 to 2.4, and from the axiom text (`K/axioms/core-axioms.form:58,67,78,88`: a cell is a
present blueprint; a recipe is a cell that composes other cells; "a cell meets other cells only through an interface
it offers. The interface itself is a recipe or a blueprint of a recipe"; an offer carries a recipe or blueprint and
0..many argument cells). It is offered as the cleanest statement I can make, not as a decision.

### 8.1 Six kinds of node

| Kind | What it is | 2000 name | Notes |
|---|---|---|---|
| Structure blueprint | an ordered list of slots (each: a name-lens entry and the interface blueprint its content must offer), plus at most one parent whose slot list is a prefix | instance definition, fixed part | prefix rule = single structural inheritance (`OBJ:469`) |
| Interface blueprint | an ordered list of recipe signatures, plus at most one parent whose list is a prefix | interface definition | prefix rule makes indexed dispatch stable (`OBJ:640`) |
| Recipe | a method body; reads slots only through `this`, sends only through `self` or an explicit reference | method declaration / method | `do`, `redo`, `undo` for a native one (`OBJ:634`) |
| View | the table for one (interface blueprint, maker) pair: position i holds the id of the recipe that answers signature i | interface (the VMT-like object) | a derived, cacheable fact; this is `web-semantics` item 3, "witness" |
| Maker | a cell that mints cells: holds the structure blueprint, the list of views its cells offer, and the common cell | instantiator | `OBJ:569-577`; two makers of one blueprint may differ in common cell (`OBJ:669`) |
| Cell | state: one content node per slot; knows its maker | object | the only thing with identity over time |

### 8.2 The reference

A reference is a pair of node ids `(cell, view)`, or an immediate.

- `cell` names *whose state*. `view` names *through which interface*. Neither is stored inside the other.
- An immediate carries a value in the first word and a view id in the second; it names no cell, so it cannot be
  aliased and has no maker to ask. (2000: the native flag. Today: the node word's immediate mode.)
- Equality of references is equality of both ids; identity of the *object* is equality of the `cell` id alone
  (`OBJ:701`).
- The state's bytes hold no table pointer, so a cell's content is the same through every view. That is the property
  `web-semantics-machines.md` 2.2 names as the reason to prefer the pair in a content-addressed body, and it is the
  2000 design's stated advantage too (`OBJ:493`). Two independent sources, one conclusion.

### 8.3 The offer, with two bases

Every activation carries `(self, this, recipe, arguments)`, where `self` and `this` are references. Six rules, the
first four from the table in 3.2, the fifth from the field instructions cited above it, the sixth from 4.2:

1. **Send** to reference `r`, signature `i`: `self := r`, `this := r`, `recipe := view(r)[i]`.
2. **Self-send** of `i`: `this := self`, `recipe := view(self)[i]`.
3. **Super-send** of `i`: `this := (cell(this), parent(view(this)))`, `self` unchanged.
4. **Delegate** to reference `d`, signature `i`: `this := d`, `self` unchanged, `recipe := view(d)[i]`; the
   forwarding activation is replaced, not stacked.
5. **Slot read or write**: on `cell(this)` only. There is no other way to reach a slot.
6. **Cast** of `r` to interface blueprint `I`: find in `maker(cell(r))` a view whose interface is `I` or has `I` as an
   ancestor; answer `(cell(r), that view)`, or fail.

Rule 5 is what makes rule 4 safe: a delegate's recipes cannot touch the true object's slots, only send to it.

### 8.4 What each named feature becomes

- **Multiple inheritance**: a structure blueprint has one parent (prefix) and any number of *delegate slots*; each
  delegate slot's views are added to the maker's list, and every signature not answered locally gets a forwarding
  recipe that applies rule 4 to that slot.
- **Shared inheritance**: two cells whose delegate slots hold the same reference. Nothing more.
- **Supplied at creation**: the delegate slot is filled from an argument of the maker's offer (what the 2000
  compiler did, 4.2).
- **Tagging**: a new cell with one delegate slot holding an existing reference and extra slots of its own;
  normalization is the rule that the slot must hold an untagged reference and at most one tag per tag blueprint.
- **Detached interface**: rule 6 extended: when no view matches and `I` declares prerequisites that the maker's
  views satisfy, a view for `(I, maker)` is computed and added. Under content addressing this is a memo entry keyed
  by two ids, never an edit of the maker (`web-semantics-machines.md` R10, R11).
- **Common object**: a cell reached through the maker; its own maker's blueprint delegates to the base common cell.
- **Template**: a recipe from blueprints (and constants) to a blueprint. An instance's identity is the pair
  (template id, argument ids); the 2000 spelling `Name[args]` and the per-instance GUID table (`TT:756`) are two
  hand-kept forms of that pair. A bound is an interface blueprint each argument's maker must have a view for:
  checked by rule 6 at instantiation.
- **Flattening** (the 2026 lowering): the special case of rule 4 in which every delegate cell is private to one
  owner and never escapes. Then `cell(this)` is always a known region of the owner's own state, the forwarding
  recipes disappear, and `self` and `this` share one cell id. Read this way, 2026's one-record layout with
  `Class.name` slot keys is an *optimisation of private delegation*, and the 2000 rule stays the definition.

### 8.5 Where this formulation meets the open design questions

- It needs a `cell` id that stays the same while slot contents change. Axiom 3 makes identity the present content
  (`core-axioms.form:67`). The 2000 answer was an index into a table of mutable descriptors. So the first id of
  the pair cannot be a content hash of the state; it has to be a name-like handle whose present blueprint changes.
  This is the critic's N2 and C2 again, met from the object side.
- A view is fully determined by (interface id, maker id), so it can be content-addressed and shared.
- Cast failure needs an acknowledgment: `nothing`, `0`, or a fault. 2000 threw an exception. This is the critic's
  N1.
- A slot write under an open choice needs an undo entry; 3.3 shows the 2000 machine did not get `self` right there.

## 9. What remains unproven

Stated plainly, each with what would settle it.

1. **Separate base objects with `self` kept and `this` moved have never been recorded executing**, in 2000 or 2026.
   2000 has the machine code path and compiled stubs with zero recorded executions (R2, R3, R5); 2026 has no such
   representation. Settled by: one program in the new grammar equal to the thesis example `E : C, D` (`OBJ:719`)
   with a base object shared by two true objects, answering the value the walk at `OBJ:721-738` predicts.
2. **The 2026 flattened delegation is unwitnessed at HEAD.** The code stands; the five fixtures and the proof band
   are in history only (R6, R7). Settled, for the old body, by restoring `bml-class-delegation.bml` and running the
   fixture door; I did not, by instruction.
3. **A delegate field that calls back into an overridden method** has no test in any era. The 2026 source says it
   loses `self` (4.4 b); the 2000 source says it keeps it (rule 4), unexecuted.
4. **Delegation under backtracking** is unsound in the 2000 source as I read it (3.3), and untested in 2026 (the
   2026 journal is per field of one record; `bml-choose-fields.bml` covers single objects).
5. **The equivalence claimed in 8.4** (flattening is private delegation) is my inference. It needs a statement of
   when a delegate "never escapes" and a check that fails closed.
6. **Template parameter bounds** were never enforced anywhere (5.1, 5.2).
7. **Tagging, detached interfaces, shared inheritance as a language feature**: specified, never implemented
   (4.1, 4.2, 4.3).
8. **Casts** have no 2026 implementation; rule 6 rests on the 2000 code alone.
9. **The cost.** The 2000 logs put `CastRegConstant` at 0.095 to 0.303 ms and `ForwardInterfaceCallConstant` at 0.124
   to 0.172 ms per execution against 0.018 to 0.026 ms for `CallRegConstant` (R1, all six logs). Per log, a cast cost
   5.1 to 11.7 times a plain call and an interface forward 6.5 to 7.1 times; casts took 4.1 to 20.3 percent of
   instruction time (237.253 of 1,168.286 ms in `A/src/performance.log.docx`; 237.594 of 5,772.022 ms in
   `A/BMCPU/Performance/original.log.docx`). observed (numbers), computed by me (ratios). The thesis's own remedy
   (a search tree or a sparse table, `OBJ:821`) and the memoised view of 8.1 are untested here.
10. **The dates of the 2000 logs.** The containers were re-saved in 2024 and 2026; the content is undated. That they
    are 2000 runs is inferred from the format, the instruction names and the neighbouring build artefacts.

## 10. Carry forward, let go, open questions

**Carry forward** (inferred, each from the sections named):

1. The reference as two ids, cell and view, with an immediate mode; no table pointer inside state (2, 8.2).
2. The activation carrying `self` and `this`, with the six rules of 8.3 as the *definition* of dispatch (3.2).
3. "Slots are reachable only through `this`" as a hard rule; it is what makes delegation, tagging and views safe (8.3).
4. `super` as a change of the view half of `this` (3.2).
5. Views as derived, memoised facts keyed by (interface id, maker id); casts as lookups in that memo (4.2, 8.1).
6. The maker (instantiator) as an ordinary cell: it is where "which interfaces", "which common state" and
   "singleton / unique / many" live (`OBJ:569-577`, `OBJ:778-784`).
7. Flattening kept as an optimisation with a stated side condition, not as the meaning (8.4).
8. Templates as recipes from blueprints to blueprints, instance identity = (template id, argument ids);
   specialisation by substitution is the one mechanism both eras ran (5, R9, R12).
9. The fixture door's shape: every source declares its own answer in its head and one cell counts how many agree
   (`K/form/form-stdlib/bml/bml-native-fixtures.bml:1-13`). It is how R6 could be dated at all.
10. The 2000 test and library sources as a ready conformance corpus for the object model
    (`A/src/Test/*.bml`, `A/src/BML/lang`, `A/src/BML/io`; 243 `.bml` files).

**Let go**:

1. The claim "multiple inheritance and the data-plus-interface pair are proven working". Replace it with the matrix
   in 6.2.
2. Class id stored inside the record plus an if-chain per signature as the dispatch form (`L:645,684-685`): it is
   the embedded-table-pointer design the pair exists to avoid, and it ties a cell's content to its class.
3. A pinned count as a standing witness: the pin stayed at 119 while the tree grew to 152 (R8), then the fixtures
   left and a support list kept naming them (section 7 item 5).
4. `self` reconstructed instead of saved (3.3).
5. Bounds as unchecked syntax (5.1).
6. Three meanings of "class" and of "template" across three readers (`bml.md` 1.2, 2.9; 4.3 last bullet).
7. Forwarding stubs as real two-instruction methods stored per class (R3: 314 stubs in 202 files); with memoised
   views they are table entries.

**Open questions** (could not settle by reading):

1. What is the id of a mutable cell under axiom 3? (8.5)
2. Is a failed cast `nothing`, `0`, or a reported fault? (8.5)
3. Does the owner want separate, shareable base objects (2000) or composition into one shape (2026 and
   `web-semantics-machines.md` R12) as the *default*? Section 8.4 shows they can be one model; which the surface
   syntax means when it writes two parents is a choice.
4. Did `ClassTest.bml` ever compile in 2000? Nothing on disk says so (R5). Only the author can know.
5. Were the delegation fixtures cut on 2026-09-29 because they failed, or with the 13,000 other files? The cut's
   message gives a count, not a reason per file.
6. Does the present fixture door still read 26 of 26? Its head says so (2026-10-03); the compiler file changed last
   on 2026-11-19; I was told not to run it.
7. In 2000, is `self` readable and `this` unreadable by the program (`BMA:528-541`) a design rule to keep? It means
   a recipe can pass its true object on, never its current structure.

## 11. Surprise, and what stayed uncomfortable

Most surprising: the witness the critic could not find existed for two weeks. `bml-class-delegation.bml` tested
exactly the hard case (a delegated method reaching an override), a band head pinned the count that included it, and
then the cut removed it while the support list, the lookup functions and a class-level `self` / `this` model stayed
behind without a caller. And on the other side of twenty-six years, the 2000 machine really is a pair machine: its
logs count casts by the thousand, while the two instructions that would have shown `self` differing from `this` were
compiled into the standard library and never once counted.

Uncomfortable:

1. Every "ran" in this report is a record, not a run. The 2000 logs are undated text inside re-saved containers; the
   2026 pin is a sentence in a file head. I could have run the fixture door and was told not to.
2. The 2026 body satisfies the thesis rule for a second class base only by removing the second object, and nobody
   wrote that down as a change of meaning; the fixture comment calls it the thesis's model.
3. The two-base rule and undo do not compose in the 2000 source as I read it, and backtracking is the reason the
   whole system exists. I inferred that from about thirty lines and could be wrong about what the stack slot holds.
4. My hex scan finds stubs by a byte pattern. The eight-in-a-row regularity makes a false positive unlikely, but I
   did not parse the class-file format.

## Appendix: how the hex scan was done

For each of the 202 files under `A/classes` ending in `.bmc`: `xxd -p file | tr -d '\n'`, then
`grep -b -o -E '[0-9a-f]{4}5[0-9a-f]fc[0-9a-f]{4}7[0-9a-f]fc'` (for `fdc`+`fcx`) and the same with `6` (for
`fic`+`fcx`), keeping matches at even hex offsets. Basis: an instruction is one 32-bit word written low byte first
(`JC/ASM/Statement.java:47-50`), opcode in the high 12 bits (`fdc` 0xFC5, `fic` 0xFC6, `fcx` 0xFC7,
`JC/ASM/Instructions.java:62-64`; `VM/opcodes.h` `OP_FDCCST 0xFC500000`), constant index in the low 20 bits
(`JC/ASM/Instruction.java:10`). Output for `FileInputStream.bmc`: eight matches at hex offsets 10762, 10818, ...,
11154, each `020050fc NN0070fc` with NN = 03, 04, 05, 07, 08, 09, 0a, 0b.
