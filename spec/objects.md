# Objects

Shapes, cells and references, written in form on top of cells and their trail
(`spec/core-operations.md`, entries 89 to 93). `objects/objects.ck` is all of it: one layer of
definitions and no operation of the bootstrap. Design nodes it follows: 5.5.1, 5.5.2, 5.5.4, 5.5.7.

## What there is

- **A shape** is a node: a name, its parents, the slots it adds and its methods, each a name and
  a recipe. `(obj.make name parents slots methods)` builds one. Being a node, a shape is its
  content: two shapes written alike are one.
- **Several parents are one composed shape.** `(obj.line s)` is s and all its ancestors, each
  once and ancestors first, and `(obj.slots s)` the slots of all of them: an ancestor reached
  through two parents gives its slots once. A slot name two shapes of the line both add is a
  fault, `ambiguous`.
- **A cell** is made by `(obj.new s)` and holds one value for each slot of the composed shape.
- **A reference is a cell and a view.** The view is the shape through which the cell is seen.
  `obj.new` answers the reference whose view is the whole shape.
- **Slots are reached only through the view.** `(obj.get r slot)` and `(obj.set r slot v)` fault
  with `no-slot` when the view's shape does not have that slot, even though the cell does.
- **Send.** `(obj.send r name x)` finds the method `name` in the cell's own shape: the one
  definition in its line that no other definition overrides. Two parents that both define it,
  with nothing below to choose, is `ambiguous`; the system does not guess. The method is called
  with the reference seen through the shape that defines it, and x.
- **Super moves the view.** `(obj.super r parent name x)` names a parent of the view's shape and
  runs that parent's method with the view moved to where it is defined.
- **Cast.** `(obj.as r s)` is the same cell seen through s, when s is in the cell's line.
- **A view by structure.** `(obj.through r face)` is the same cell seen through a shape it never
  named: it fits when it has every slot of face and knows every method face lists by name.
  Through it only face's slots are reached; a send still finds the cell's own method. A cell
  that does not fit is a fault, `no-fit`. So an interface can be written after the shapes that
  fit it (5.5.3).
- **A template is a recipe that answers a shape.** `{of | (obj.make ..)}` called twice with the
  same shape answers one node, since a shape is its content; nothing else is needed (5.5.5).

A method is a recipe of two arguments: `$0` the reference it runs on, `$1` what was sent.

## What undoes it

A slot is a cell of the walker, so a `set` inside an option that fails is put back, and so is the
making of a cell.

## How it is known

`ground/two-parents.ck`: a shape Tile with parents Named and Flat, which share the ancestor
Thing. Its composed slots are id, name, w, h, color, with id once. Its `describe` sends to both
parents through super. Asked for its view, a method reached through super answers the parent's
name. Read through the parent Named, the slot w is refused. A set inside a failed option is
undone. A shape with the same two parents and no `describe` of its own is refused as ambiguous.

Each of those was turned red by a fault planted in `objects/objects.ck` and taken out again: an
ancestor held twice, a super that keeps the caller's view, slots read past the view, a set no
fail undoes, a guess between two parents.

`ground/fits-by-structure.ck`: a cell of Named, seen through Labelled, which Named never named:
its name is read and its describe answered, its id is refused through that view, a view through
Sized, whose slot it lacks, is refused, and a template called twice answers one shape. A fit that
asks nothing of the slots turned it red.

## Limits

- The example is this repository's own. The thesis example the path names for stage 5 (graph node
  7.6) was not at hand.
- Attributes (5.5.6) are not built. A fit is remembered once found; a view is not kept apart
  from the reference that carries it.
- A method takes one thing sent, since a call cannot yet pass on a list of arguments.
- The activation does not carry the reference; a method is handed it as its first argument.
- A shape's line and its slots are composed the first time they are asked for and kept, not
  when the shape is made.
