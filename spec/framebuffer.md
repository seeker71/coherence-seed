# The framebuffer, floor version

The definition (graph nodes 5.7.6 and 5.7.7): all memory of every object, node and stack
is read and written in one shared memory space, which can be recorded like video with no cost to
the runtime and played back to see what happened, when and where. The blueprint is stored next to
each object, and a record can name who last wrote it.

This page fixes how the bootstrap does that. `bootstrap/fb.h` is the layout in C, `bootstrap/ck.c`
is the only writer, and `tools/fb.c` is the observer.

The observer reads the layout version and area count before following any region offsets.
Live regions and recordings from another layout close with an incompatibility message.
The substrate header also identifies its version and extent within the region.

## The region

- The bootstrap reserves a shared region with stable addresses. An ordinary run uses an
  anonymous mapping, which the OS releases even if the process is killed. Set `CK_FB` to
  publish a run by name. A published run remains available after exit until `fb drop NAME`.
- Set `CK_SHARE` to a name and run after the prelude `share/all.ck`, and the process becomes a
  member of the substrate of that name (below). It starts alone, as any process does. The prelude
  maps the substrate at byte 2^43 of the region's addresses, as every member has it, by asking the
  host for that place and taking no other; nothing between is reserved. `CK_SHARE_BYTES` is that
  substrate's allowance, set by whoever makes it. With `CK_SHARE` set the bootstrap itself only
  gives the three tables keyed by handle room for every handle.
- The runtime gives all tables one allowance for touched memory. It charges their page-rounded
  high-water marks, the two heads, the thread's entire stack and the executable copy of code.
  Cells, bound-by-node and print-numbers are keyed by handle, so they are charged by the pages
  they use instead: a page is charged when it is first used, one bit for it in `touched`, and a
  page not yet used is never read, since reading it would take it.
  Backtracking can reuse charged pages. Each table retains space to grow into the common pool.
  `fb show NAME` reports both the virtual reservation and the shared memory allowance.
- Each area reserves what `region/layout.ck` says for it (`fb.reserve`), whatever the host or the
  allowance, so the layout is a fact of the nodes alone, the same on every host and at every
  allowance: 1 GiB for most of the walker's
  tables, 4 GiB for the store's and the rooms, 256 MiB for the small ones, one word for each
  handle a lane has for the three tables keyed by handle, and the stack last, as large as the
  host's, under a page nothing may touch. A process alone reserves about 49 GB of addresses. The
  allowance is a word of its own, `member.allowance`, not an area's capacity.
- At startup, available memory is divided by the number of online CPUs plus one, leaving room
  for other work. On macOS this uses free and inactive pages under normal pressure, free pages
  at warning, and zero at critical pressure. Other supported Unix builds use available physical
  pages. Linux also consults free backing space for a named region. A finite address-space
  limit lowers the allowance; one below the layout's reservation leaves the region unmade. `CK_MEMORY_BYTES` can lower the allowance in decimal bytes.
- A request past the allowance follows Form's existing `full` fault for the growing area.
  As an area's high-water mark crosses a MiB boundary, the runtime samples the host again
  before admitting growth. File input is read in bounded chunks and reports a full text area
  if the complete input will not fit.
- Each area but the stack begins `fb.hue` bytes (128, the longest line of a data cache among the
  hosts here) into its first page for each area before it, so the first records of no two areas
  fall on one set of a cache whose line is at most that. It is a row of the layout and not asked
  of the host, so a region is laid out alike on every host. A reader finds each area by its offset
  in the head.

## A region at rest

A start is what runs before the program: the files given after it (`ck run F B..`), or the image
`seed-run` opens with its first part. When the start has run and the region is at rest, it is a
function of what was read, and is saved once; every later start with the same name maps it.

- **At rest** means: no choice open, nothing held back (`after`), nothing asked from outside, no
  binding made from outside, no note of code kept back, not a member of a named substrate, and
  nothing taken from the host or the process since the start began. What a start may do and stay at
  rest is what is known to keep the region a function of the files it read: the walker's work on
  nodes, cells, rooms and bindings, reading a file with no choice open, and a `peek` of bytes an
  image keeps as they were, which are the two heads and the areas `fb.kept` names in
  `region/layout.ck`, but the words of `fb.own`. Anything else counts in `member.taken` and the
  start is not saved: a call of the host, `clock`, any other `peek`, any code a program
  runs (code reads what it likes, and is given the region's address), a read of a file not there,
  a read under a choice, more than 64 files. So does any fault whose coming follows the process
  and not the program, caught or not: `full` of any area, which follows the allowance, the stack
  and the host's free memory, and `crash`, which follows how deep the native stack goes. A fault
  the program alone decides, as `irreversible`, `arity` or its own, leaves the start at rest.
- **The name** is `region/rest.ck`'s, made from nothing read from a file: the id of a list of
  whether the port is numeric, the device and inode of the directory the start's paths are read
  from, and the paths as given. The file is `<name>.image` in a folder of the saver's own: `coherence-seed-images` in
  the folder `CK_IMAGES` names, or else `~/Library/Caches/coherence-seed/images` on macOS and
  `$XDG_CACHE_HOME` or `~/.cache`, then `coherence-seed/images`, elsewhere. `CK_IMAGES` set empty,
  or no `HOME` where it is not set, keeps no image, and so does a folder that is not this user's
  alone, or that lies in one another may write (made with mode 0700; the folder above it is
  checked too, before anything is made in it). Only macOS has
  it yet, since the executable's identity is its `LC_UUID`, the one fact the bootstrap gives.
- **The directory is held small.** After it saves an image, `region/rest.ck` lists its folder
  and removes the images read longest ago, never the one just saved, and only what it wrote: a
  name of 32 hex digits and `.image`, a regular file by `lstat`, this user's, beginning
  `CKREST1\n`; a builder's temporary file of that name older than ten seconds goes too. They are
  removed until they hold
  no more than 64 MB (`CK_IMAGES_BYTES`, in decimal digits, sets another; anything else is not
  read) and a sixteenth of the disk's free space.
  The host keeps when an image was last read: APFS moves the time when a start reads an image's
  head, and a Linux mount with `relatime` moves it at most once a day.
- **The image** is a head, its extents, the region's pages that hold anything, each at its place,
  and the rows of the files the start read, in the canonical print. The head repeats the whole key, the layout version and the
  executable's `LC_UUID`. The words of `fb.own` are saved as 0; the born-code table, the working
  indexes and the text past the guard's stack are saved empty, so code is born again at its first
  call.
- **The builder** takes the snapshot (The saver, below) and hands `region/rest.ck`, found beside
  the executable, every file the start read with the bytes it gave. `region/rest.ck` asks the
  host for each file's device, inode, size and both times, refuses one that is not a regular file
  or does not read the same bytes again, and makes a node that checks those signatures again.
  That node is the image's last part; the file is made by `mkstemp`, so it is this process's own and new,
  written, synced to the disk and then renamed over the name, all by `region/rest.ck`. The builder then puts the snapshot's pages back
  over its own region, so that it goes on exactly as a later start. The text area is saved as it
  was before the start, its count and its high mark, since how far it reached follows the command
  line.
- **Entering.** A later start runs `region/rest.ck` in a fresh region, which has nothing but
  the catalogue, given the executable's identity, the port, the start's paths and the layout's
  facts. For the start's files, then for fewer, it makes the name, and of an image there checks:
  owned by this user and not writable by others, and once opened the same file again by its
  device and inode (`fstat`), its length, the key, the version, the
  executable, every extent inside the region below the stack and apart from the text, in order
  and inside the file, every area laid out as this process lays it out (compared by the host,
  record by record), no code saved, and room in the allowance for what it charges; then it reads
  the rows the image keeps and asks one `stat` for each file. It answers the open file and the
  extents, and only then does the bootstrap make the region zero again and map the pages
  copy-on-write (`CK_FB`: read into the shared object instead), and the words of `fb.own` are
  this process's again. Any other answer makes the region what `fb_open` made, and the start
  runs cold and saves a new image.
- **What runs after.** A node a start leaves in the cell under the entry `after` runs once the
  start is over (when two of its files leave one each, the first, then the second), in every process, whether the start ran or was entered: a package's opening, the
  door, the device, the join, follows its loading and is not saved with it. `share/all.ck` and
  `host/metal-all.ck` leave theirs there, and so does the first part of a Metal image.
- **The saver.** When a start is judged at rest the bootstrap takes a copy-on-write snapshot of
  the region below the stack (`vm_copy`), with the words of `fb.own` as 0 and the text as before
  the start, and hands it to `region/rest.ck` with the page ranges the areas use. Form scans
  those pages for the ones that hold anything, writes them, the head and the check through the
  host's `creat`, `pwrite`, `fsync` and `rename`, and the bootstrap puts the snapshot's pages back
  over the region, so the builder goes on as a later start has it.
- **The part at rest.** A start saves itself after each of its files that leaves the region at
  rest, also after the part of it an image stood for, whose checks then go into the longer image, under the name of the files so far, and drops the image of the
  part before; so a start whose last file takes from the host keeps the image of the files before
  it. A later start tries the image of all its files, then of fewer, and runs the files after the
  part it entered. A start file is checked by the node its first line reads to, a file read with
  `file` and the image `seed-run` opens by their bytes.
- **Two builds of one start are the same bytes**, so two builders at once each write a file of
  their own and the second rename leaves what the first did.

## What is in it

The first page is the **head**: a magic word, the layout version (24), the writer's process id, the
walker's registers (the **machine**), one **area** entry per table, the doors, and the **member** part: where
the substrate lies (`share`), how many pages the tables keyed by handle have charged, the
process's lane, the node a member runs last (`leave`), and the name of the substrate it is a member of, empty for a process alone. An area entry says what its records are, how large one is, the most it has held,
where the table starts, how many fit and how many are in use; the last four are counted by 64 bits.

Machine code reads the region too, and three places in the head are kept for it whatever else
moves: at byte 28 the **floor**, the lowest page the machine stack may reach, counted in pages of
16,384 bytes from the region's start; at byte 128 the machine's `serve`, the address code calls
to have the bootstrap do an operation on values in hand (`spec/core-operations.md`, Native
code); and at byte 160 where the nodes area starts. The machine's word `unused`, where the
address of a watched region lay before another region was read in Form, holds the second and
third where code reads them. After the last area entry lie the **doors**: addresses the host
gave this process, for code that has no walker to ask. The first is `symbol`,
the address of the host's lookup of a name (`dlsym`); the second is `adopt`, the address of the
bootstrap's own function by which a process becomes a member (below). They come after the area table so that the
three places above do not move when a door is added; a reader finds them at the end of the area
table, by the layout's rows, and the member part after them. A leaf that reads any other place is tied
to one layout version: the step leaf of `ground/step-in-code.ck` and `form/step.leaf.aarch64.ck`
reads a node's record, 32 bytes with whether it is a composite at byte 12, as version 26 lays it
out.

| Area | Records | Holds |
| --- | --- | --- |
| nodes | node | every node: blueprint, writer, where its content id lies once asked for, whether it is a composite, whether it is settled, its child or byte count and where its payload is; the table lies in the process's lane of the substrate |
| leaf-bytes, children | bytes, words | the payloads of leaves and composites, in the lane |
| by-content, by-id | words | the working index: the two indexes over the nodes not yet settled; they can be rebuilt from the nodes |
| frames | frame | the walker's continuations |
| links | link | values gathered for an operation that takes several |
| activations, arguments | activation, words | each running recipe and its arguments; for an implementation, the limit it runs under, which counts the program's own bindings and those made from outside, and the activation and cut scope of whoever used it |
| choice-points | choice-point | each open choice and the marks it returns to, among them how many calls were held back |
| gathered | gathered | answers collected by `all` |
| born-code | code | which code leaves have a runnable copy, and where |
| bindings | binding | which entries a program has given a new implementation, oldest first, each naming the one before it for the same entry, and the code that is its faster form when it has one |
| bound-by-node | words | for each node, its newest binding; it can be rebuilt from the bindings |
| outside | binding | what was bound from outside the run: seen by every layer of the program, and not undone by a fail |
| cells | words | for each node used as a cell, the node it holds |
| trail | trail | while a choice is open, what a cell held before each change, so a fail puts it back; and, kept, each note, faster form and born code, so a fail leaves them and keeps what they hold |
| reader-opened, gathering-children, gathering-bytes, print, print-numbers, text | words, bytes | working lists of the reader and the printer, and every file read |
| stack | bytes | the machine stack of the C code and of code leaves |
| asked, answered | bytes | the print of a node another process asks the run to evaluate, and the print of its outcome |
| after | words | calls held back until every open choice is closed: for each, how many words follow, the recipe and its values |
| noted | words | notes written by code that has not answered yet: a cell and what it will hold, in pairs |
| rooms | bytes | rooms: each a head, its blueprint, the node that last stored into it, how many stores it has had and its size, then its bytes, changed in place (`spec/core-operations.md`, entries 113 to 117) |
| room-trail | room-trail | while a choice is open, the four bytes of a room as they were before each store, and its writer and count, so a fail puts them back |
| gives | words | how many times a fail gave nodes back, and the choice, the mark and the top of the last time |
| ids | bytes | each content id once asked for: the node's handle, then the id's 32 bytes; when a fail gives nodes back, the ids of those that stay move down with them; in the lane |
| values | words | the values in hand: those of a node's children while its operation runs, and what a call is given; all given back at the walker's next step |
| seen | words | for each lane, how far it was settled when the oldest open choice was made: what a search sees of it |
| touched | bytes | a bit for each page of cells, bound-by-node and print-numbers, set when the page is first used |

## The substrate

A node's handle is a place in a node table that lies in a **substrate**. A process alone makes its
own, a part of its region after its own areas, with one **lane**. A named substrate is an object
of that name which every **member** has mapped at the same offset `share`, 2^43, from its region's
first byte, so a place in it is the same offset in each member and rule 3 holds across members. A named substrate has 256 lanes that divide the handles between
them: lane k holds the handles from k·2^24 up to (k+1)·2^24, and a handle names the same node in
every member. Lane 0 belongs to whoever made the substrate and holds nothing at handle 0, the
seeded catalogue, and whatever else its maker had made before it became a member.

A member's nodes, their parts and their ids lie in its lane. Its own area entries for those four
tables point at the substrate's tables, with its lane's part as their count and their room, so the
head's byte 160 still says where node h lies: at that offset plus 32·h, in every member. The
substrate begins with its own head: a magic word, the layout version, how many areas and lanes it
has, its maker's process id, in `start` its maker's member word (below), the instant it was made,
`share`, its size, its allowance and how much of it is charged, `buried`, the `tide`, and `catalogue`,
a digest of the seeded entries' words and numbers that `region/header.ck` writes into `fb.h`; then one area entry
per table: lanes, nodes, leaf-bytes, children, ids, by-content and by-id.

- **A lane's row** is 64 bytes: `who`, one word that is the member: its process id from bit 42
  up, below that the low 40 bits of the instant the host started that process, in microseconds,
  and in its two lowest bits 2 live or 3 gone, 0 while the row is free; `start`, that instant in
  full, for a reader; `n`, how far the
  lane's records are made, stored with release as the member makes a node or a fail gives nodes
  back; `sw`; what the lane has charged; and how many of its nodes a commit forwarded.
- **Settled and working.** Every node of a lane below its `sw` is settled, and its record's
  `settled` byte is 1. A node made while no choice is open, when no working node
  stands, is settled as it is made. Any other is working: it is found by the process's own
  by-content and by-id, no other member can hold it, and a fail gives it back as it always did. A
  settled node lies below every choice point's mark, so no fail reaches it, and the substrate's
  by-content and by-id find it. That holds because no choice opens again once closed: a cut stops
  at a running outcome's choice point, which the outcome's end fails into. A fail that would reach
  a settled node ends the process.
- **Entering the index.** In a named substrate the two indexes are made once, for twice the nodes
  the allowance can hold, and never grow. A settled record is written whole above the lane's
  count, its `settled` byte first, and then one compare-and-swap puts its handle in an empty place
  of by-content. There is no lock and no place marked as being filled. A swap that finds the place
  taken looks at what took it: equal content answers instead, and the record just written is
  simply not counted, since nobody saw it; other content sends the probe on. Then `n` and `sw` rise
  past the record, stored with release. An index place is loaded as a whole, in sequence with the
  swaps. A node's id is a record appended whole to the lane of whoever asked for it, given to the
  node by one swap of its `ida`, and then found by one swap into by-id; whoever asks later makes
  sure of that last step, so a member that died between the two leaves nothing unfound.
- **The commit.** When no choice is open and working nodes stand, they are settled where they
  stand. A process alone does this at the top of the next evaluation step, the newest first as the
  working index forgets them, and ends if it finds equal content already settled, which nothing in
  it can cause. A member does it at once where its last choice closes, and oldest first, because
  another member may have settled equal content since the search began: such a node is forwarded
  to the settled one and gives its place back. The nodes made after it move down, as a fail's
  giving back moves what it keeps, with their parts, cells, bindings and ids, their blueprint,
  writer and children forwarded, and each is settled where it comes to lie; the forwarded node's
  cell, bindings and id go to the settled node; `gives` counts it, the lane's count drops before
  anything moves, and everything the run holds is forwarded: the machine's registers, frames, links, activations, arguments, what was gathered,
  bindings, born code, held-back calls, the heads of rooms and every cell. The lane's row counts
  the nodes forwarded.
- **What a search sees.** While a choice is open or working nodes stand, a member sees of another
  lane only what lay below that lane's `sw` when its oldest choice opened: `seen` keeps each lane's
  `sw` from that moment. It is one moment for all lanes: the substrate's `tide` is a count that is
  raised before any lane's `sw` is, by whoever raises it, and the opening reads the tide, every
  lane and the tide again, and reads again when it moved. A node's parts were below their lanes'
  `sw` before its own lane's tide moved for it, so a search never sees a node without its parts. A settled node of another lane beyond it is, for the maker of nodes and
  for finding by id, not there, so the search makes its own working node and the commit forwards
  it; `at` on its number is the fault `irreversible`. So going an option again meets the same
  nodes, and no run holds two handles for one content. A node in the index is settled together
  with everything below it in its lane, since a lane publishes in order. So whoever takes a node
  of another lane from the index outside a search first raises that lane's `sw` past it, by
  compare-and-swap, when the lane's member has not yet done so itself. A member killed between
  publishing a node and raising `sw` is finished that way by whoever next takes the node, and what
  a published node names is always below its lane's `sw`.
- **Members.** A member is a process id and the instant the host started that process, both in the
  one word a row is claimed by, so a row is never held in the name of an id alone; the word's
  fields are rows of the package (`share.member`). The host is asked through `sysctl` for the
  process of that id, which answers for any process: one that is not there, or is a zombie, or was
  started at another instant, is gone; only if the host cannot be asked does a process count as
  standing. Each process that joins, and each that leaves, looks at every row that was ever
  claimed and retires by compare-and-swap, once, a live row whose member is gone. When no row has
  a member the substrate ends: one process writes its own word in `buried` by compare-and-swap,
  in place of nothing or of the word of a process that is gone, and looks once more: at the rows,
  and at whether a maker that stands has begun the substrate and not finished. If neither, it
  opens the name anew, and unlinks it only when the object of that name holds its own word in
  `buried`; otherwise it takes `buried` back. So a process that still has an ended substrate mapped
  cannot unlink the name of the one made after it, and only one process at a time, the holder of
  `buried`, unlinks at all. An object left empty by a maker that died is given one page and buried
  the same way, by a joiner that has watched that same object stay empty, through its own
  descriptor of it, for twelve hundred looks half a millisecond apart: patience is counted of the
  object, not of the joiner. A maker writes its word in `start` first and, finding `buried` held,
  gives the object up and joins again; one that cannot size its object, or whose allowance is too
  small, or whom the bootstrap will not adopt, gives it up the same way, and never unlinks by
  the name alone. The words these decisions turn on are read and written by one
  compare-and-swap leaf, which orders them among all members. A process that claimed a row and
  then finds `buried` set gives the row up and joins again. So after every member was killed the
  object stays until the next process opens it, buries it and makes a new one. A process waits for
  nobody: it tries six thousand times, half a millisecond apart, and buries a substrate in whose
  head no maker's word appears while it looks twelve hundred times.
- **Becoming a member.** All of that is `share/share.ck`, a component package over the host's rows
  (`host/host.ck`): rows for the host functions it calls (`shm_open`, `ftruncate`, `fstat`,
  `proc_pidinfo`, `getenv` and the rest), rows for the host's constants and record offsets, a row
  for each table of a substrate and how its room is found, one machine-code leaf that compares and
  swaps a word, and functions that read a record's field by the name `region/layout.ck` gives it.
  `share.enter` reads `CK_SHARE` and makes or joins: the maker sizes and maps the object, writes
  its head, its area table and lane 0's row, adopts, and only then writes the magic word, so
  nobody takes a substrate half made; a joiner checks the magic word, then the version, then that the
  catalogue is its own, then whether it is being buried and whether any member lives, claims the first free row, asks whether
  the allowance has room for a lane's first pages, counts its lane among those ever claimed, looks
  at `buried` once more and adopts. Every one of these is a call of the host, which a search
  cannot undo, so it runs with no choice open.
- **Adopt** is the one part of this the bootstrap does, reached through its door with the
  substrate's address, a lane, the name and the node to run last: the process's entries for its four lane tables are
  pointed at that lane, and every node it had made while alone is made again there, oldest first,
  by the maker all a member's nodes come from, so content the substrate already holds answers with
  the handle every member knows it by; then everything the run holds is forwarded onto those, as a
  commit forwards. What it made as the first lane's maker did, in the same order, is first
  compared with that lane and keeps its handle without the index being asked. It is refused while
  a choice is open, from inside machine code, twice, in a process started without `CK_SHARE`, and,
  for a lane other than the first, in a process that has given out a handle above the catalogue
  other than the one of its leaving, since that number would name another node after;
  once begun, a fault or a full inside it ends the process. The lane-0 member's own nodes, the
  door's among them, are in the substrate like any other.
- **Leaving.** The head's member part holds the node adopt was given, where no program writes; the
  bootstrap runs it once on the walker's thread, after the run's own end or when it ends the
  process itself from inside a run (a program it cannot read, a refusal, an answer it cannot
  write), with the machine put back as the run left it. A member also leaves at a signal that
  would end it (`SIGTERM`, `SIGINT`, `SIGHUP`): the bootstrap's handler, set by adopt, ends the
  process there when no run is under way and otherwise marks `asked`, so that the walker ends it
  between two steps; this is C, since a signal cannot be handed to the walker as a call. The
  package's leaving retires the member's row and buries the substrate if no member is left; while
  it runs, a lane's growth is not refused for the allowance, so a member of a full substrate
  leaves too, a page or so above it. A process alone runs nothing last and keeps the host's own
  way with signals. A member the host kills outright does not leave: the next that looks retires it.
- **The door a member reads.** `share/all.ck` loads the two packages through `share/load.ck`, a
  start that is saved at rest, and holds the opening of the door and the joining in the cell under
  the entry `after`, which runs once the start is over (A region at rest, above). A member started
  with `CK_IMAGES` enters the loaded packages from their image; there is no second copy of their
  bindings to keep in step with them. `share/door.ck` loads and opens in one, for a program that
  needs the door and is not a member.
- **The allowance.** `CK_SHARE_BYTES`, or else what the host could spare when the substrate was
  made, is in its head and never changes. Each index has a place for twice the nodes the allowance
  can hold beside it, a thirty-second of the allowance in places, so the two take a quarter of
  it; they and the head are charged in full when it is made; a lane is charged nothing for being claimed, though one is not claimed where its first
  pages would have no room. A lane's growth is charged by whole
  pages: the head's count is read, and the growth is added by one compare-and-swap only when it
  fits, so a growth with no room never counts against another member's; it is the fault `full` with the area's name, and the host is asked again at each
  MiB step as for any area. A member's own allowance (`CK_MEMORY_BYTES`) no longer counts those
  four tables. Nothing a lane charged is given back while the substrate lives.
- `fb.check` reads the substrate's head, its areas and the member's own lane row through the
  description, and checks that `share` and the substrate's nodes area put the nodes where the head
  says. It runs inside a choice, so it also checks that `seen` holds the lane's `sw`, and that every
  node below `sw` has its `settled` byte at 1 and every other at 0.
  `fb.one-handle` walks every lane that was ever claimed, makes each settled node again from its
  blueprint and its parts, and answers how many it made and which were not answered their own
  handle: none, when each content has one handle.

## Rules

1. **Every record starts with two handles**: its blueprint, and the program node that last wrote it.
   A writer of nothing means the bootstrap's own reader wrote it, before any program ran.
2. **A record names its kind in the system's own numbers.** The kinds of the floor's records are
   catalogue entries 53 to 63, 72, 91, 113, 114, 126 and 127 (`spec/core-operations.md`), so a reader needs no table that
   is not itself in the catalogue.
3. **A reference is a handle or an offset from the region's first byte, never an address.** A copy
   of the region reads the same in another process and at a later time.
4. **A record is complete before the count that makes it reachable moves past it.** A reader that
   takes the counts first never sees a record half made. A count moves down before the records it
   leaves are changed: when a fail gives nodes back, the nodes count drops to the mark, what stays
   moves down, and the count rises past it. A place in the nodes can then hold another node, so a
   reader that keeps nodes by handle reads the count in `gives` and keeps them under it. Two
   working areas, `text` and `values`, take their room by moving the count first and are written
   after: what lies under their counts is whole only between two steps.
5. **The writer does nothing for a reader.** It makes no call, takes no lock and waits for no one.
   It cannot tell whether anyone is reading. What it does look at, once a step, is whether
   someone has asked it something (below).

The walker's registers are fields of the machine record: its mode, the node being evaluated, the
value in hand, the reason, the running activation, the continuation, and `at`, the program node now
running. `at` is the writer stamped on whatever is made next. `step` counts the walker's steps and
serves as the region's clock.

## The region describes itself

What each record looks like is a node, `region/layout.ck`: for the head, the machine, an area
entry and each kind of record, its fields in order with their widths and whether each holds a
handle, a number or bytes; and the areas by name in the order of the table. `bootstrap/fb.h` is
made from it and from the lexicon (`lens/words.ck`) by a program in form, `region/header.ck`:
`make header` writes it, and `make check-full` compares the stored header with what the nodes generate. So
the catalogue's numbers and the records' layouts are said once, in nodes, and the C follows.

A program reads its own region with `peek` (`spec/core-operations.md`, entries 94 to 96), and
`region/region.ck`, written in form, reads it through the description: `(fb.get "area" "n"
base)` is a field of a record, `(fb.show)` the text of a view of the region with every binding
in words, and `(fb.check)` the list of places where the description and the running region
disagree: the size of each kind of record against the size its area states, the count of areas,
the three places code relies on, the substrate's head and areas, the lane row, and where `share`
puts the nodes. `ground/region-describes-itself.ck` runs that check; a description with a field
widened or two fields changed over is reported.

The same program reads another process's region. `fb.watch` maps it to be read only through
`region/far.ck`, a package over the host's rows, and `region/watch.ck` shows it: every handle
it meets is a place in the other process's node table, so the node is built again here from its
record, its blueprint first, and what is shown is the node and not the number. The name is held
in the cell `fb.where` and the address of the mapping in `fb.mapped`.

## Where a run spends its time

The walker's registers are in the region, so looking at them at intervals is a profile.
`region/profile.ck`, written in form, looks at another process's region once a millisecond until
it ends, at most two thousand times, and counts two things: the node the walker was at, and the
recipe of the activation that was running. It then shows the most frequent of each, in the lens's
words and with the names the recipes' arguments have:

```bash
printf /ck.demo | ./ck text region/profile.ck
```

The run that is looked at does nothing for it. A look is a sample: what is met often is seen,
what is met rarely is not.

## Asking a running program

A second process can ask a running program to evaluate a node. It writes the node's print in the
area `asked` and then that area's count; between two steps the walker sees the count, evaluates
the node, writes the outcome's print in the area `answered` with its count, and sets the count
of `asked` back to nothing. The run goes on where it was: the node it was about to evaluate, its
activation, its cut scope and the reason in hand are kept in a frame and taken up again.

The node is evaluated as `(outcome (outside x))` (`spec/core-operations.md`, entries 82 and
101), with no arguments. So a request can read any cell and call anything the program has
bound; what it binds is bound from outside the run, seen at every layer and kept; and nothing
else it did stays, whether it answered, failed or faulted. A print that cannot be read is the
fault `syntax`. The answer is what `outcome` gives: `(list "node" v)`, `(list "nothing" reason)`
or `(list "fault" reason)`.

`region/ask.ck`, written in form, is the asker. Its first line of input is the region's name and
the rest a node in the lens's words:

```bash
printf '/ck.demo\n(bind add (lazy (seq (note #1002 (add 1 (if (held #1002) (held #1002) 0))) (base $0))))' | ./ck text region/ask.ck
```

puts a counter on every addition the program makes from then on, `(held #1002)` reads it while
the program runs, and `(unbind add)` takes it off. The asker maps the region through the host's
own `shm_open` and `mmap`, called by name, writes with `memcpy`, and waits by looking at the
count once a millisecond.

A request can load a module. Asked

```
(seq (step (read (file "form/compile.aarch64.ck"))) (step (read (file "form/heat.ck"))) (heat.watch))
```

a running program loads the emitter and the watcher from outside, and the watcher then compiles
what runs hot. A recursion that takes 14.3 s on the walker, asked this one second after it
began, ended at 1.3 s with the same answer.

The looking and the asking go together in `region/warm.ck`:

```bash
printf /ck.demo | ./ck text region/warm.ck
```

It asks the run to load the emitter, looks at it three hundred times, and asks it to compile the
three recipes it was in most, saying what became of each. Where a recipe is what an entry is
bound to, it asks for the entry by name, so the code becomes that binding's faster form and
every definition that calls the entry runs it. Nothing is put on the run's calls: what
is hot is seen from outside, and the run is only asked to do what the looking found. The same
recursion, looked at from half a second in, ended at 1.3 s.

## Frames, recording and replay

A **frame** is a copy of the head page and of the used part of every area, taken by a second
process that has mapped the region read-only.

- `fb show NAME` takes one frame of a running or kept region and prints it: the machine, and for
  each area how many records are in use, their bytes, and the most it has held. For a member of a
  named substrate it maps that object too and reads the nodes in place, the member's own and what
  any lane has settled.
- `./ck text share/show.ck`, given a substrate's name on its input, maps it alone through the
  same rows a member uses and prints its head and each lane that was ever claimed: its state, its
  process and whether that stands, its nodes, how far they are settled, how many a commit
  forwarded and what it charged. It fails when no substrate of that name stands.
- `fb record NAME FILE MS` takes a frame every MS milliseconds until the writer is done. The file
  keeps, for each frame, only the 16 KiB pages that differ from the frame before.
- `fb play FILE` prints one line per frame; `fb play FILE N` prints frame N in full; with a handle
  added it prints that node and the program node that wrote it.

A frame taken while the writer runs is marked `moving`: the step counter changed during the copy,
so its parts may belong to slightly different moments. The counts were read first, so every record
in it was complete when its count was read.

## Observed on the first machine

- 2026-11-27, version 25 against origin/main at 0c9f8c6 (version 24), under `/usr/bin/time -l`,
  instructions and peak footprint the medians of five runs, wall time the median of thirty from
  outside; a warm start has its image in the cache. A process that does nothing: main 14.7 M,
  1.54 MB, 1.43 ms (13.7 M, 1.49 MB, 1.29 ms with an allowance of 64 MiB); here 14.1 M, 1.28 MB
  and 1.25 ms (13.8 M, 1.26 MB, 1.11 ms at 64 MiB): 49 GB of addresses where main reserved 195 GB,
  2 GB at 64 MiB as on main; in alternating runs at 64 MiB 13.76 M against main's 13.59 M, the
  17 KB the executable grew by. The lens flow (`ck text lens/to-nodes.ck lens/all.ck` on
  `(add 1 2)`): main 269.4 M, 3.92 MB, 12.8 ms; cold 269.6 M, 3.67 MB, 11.3 ms; warm 18.0 M,
  2.13 MB, 1.53 ms. The image flow of `check-image`: main 946.5 M, 5.77 MB, 42.6 ms; cold 946.4 M,
  5.60 MB, 40.0 ms; warm 114.0 M, 2.59 MB, 5.6 ms; its start with no request 851.1 M and 38.1 ms on
  main, 15.7 M, 1.84 MB and 1.37 ms warm. The long programs run cold as on main, in instructions:
  `ground/gives-back.ck` 31.359 G against 31.356 G, the long recursion 26.947 G against 26.948 G,
  `ground/nums-compiled-agrees.ck` 206.411 G against 206.410 G; a lookup by id had cost 2.5% while
  it asked first whether the catalogue's ids were made. The lens image is 1.9 MB: 117 pages in 16 extents and a check of 2.7 kB; what a warm
  process maps it shares with every other until it writes a page. Starts that take from the host
  are cold as before: the Metal image's start 1,284 M and 51 ms on both, the maker of a substrate 109.0 M
  and 6.3 ms, a joiner 93.3 M and 6.1 ms.
- 2026-11-27, version 24, after the review, merged with origin/main at b7c76e5: the maker 107.6 M
  instructions, 6.4 ms and 12.7 MB, a joiner 92.3 M, 6.8 ms and 6.5 MB, the door read and nothing
  joined 55.6 M and 2.30 MB, a process alone 13.9 M and 1.48 MB against main's 13.7 M and 1.48 MB; the indexes at half their size, the member word
  read through its rows and the host asked by `sysctl`. With an allowance of 4 MiB the maker's
  footprint is 4.0 MB.
- 2026-11-27, version 23, the member's start in Form against the start in C of version 22 (the
  build of 4ea5ec2), each the answer to `#33`, with an allowance of 64 MiB for the process and for
  the substrate, under `/usr/bin/time -l`: instructions and peak footprint the medians of five
  runs, time the mean of thirty. A process alone: 13.8 M instructions, 1.7 ms, 1.50 MB, the same
  as the build of origin/main (8f3e52b), where Metal has left C; at 4ea5ec2 24.1 M, 2.4 ms, 2.05 MB.
  The door read and nothing joined: 43.7 M, 3.1 ms, 2.25 MB.
  The maker of a substrate: 78.4 M, 5.3 ms and 20.3 MB, against 134.8 M, 14 ms and 5.24 MB. A
  joiner: 63.0 M, 5.5 ms and 6.3 MB, against 133.3 M, 14 ms and 3.14 MB. What the start in C spent
  was the reservation: this host takes about 6.6 M instructions for each TiB of addresses a
  process reserves, whatever kind, and 0.78 ms (a process that reserves and ends: 10.2 M and 1.4 ms
  for 2^34 bytes, 11.6 M and 1.6 ms for 2^38, 16.1 M and 2.2 ms for 2^40, 117.8 M and 14.1 ms for
  2^44), and 16 TiB were reserved to keep byte 2^43 free. A member now maps only the object, 352 GiB
  of addresses whatever the allowance, since a lane's tables span its 2^24 handles. What rose is the
  footprint: the door's 3,200 nodes settle into lane 0, and each falls on a page of the
  by-content index of its own, since that index is made once for the whole allowance and is asked
  by a hash; the maker touches nearly all of its 16 MiB at once, where it used to touch them as
  its program made nodes. With an allowance of 4 MiB the maker's footprint is 5.2 MB.
- 2026-11-27, version 22 with the named substrate against version 21 (the build of 01db529), a
  process alone, in alternating rounds under `/usr/bin/time -l`, one process at a time, medians
  of seven, seven and three rounds: the long recursion 27.427 G instructions against 27.423 G
  (+0.01%) and 3.865 G cycles against 3.848 G (+0.43%); forty thousand outcomes given back 31.544 G
  against 31.485 G (+0.19%) and 5.076 G against 4.971 G (+2.12%; least of the rounds +1.42%); the
  compiled sweep 209.69 G against 209.54 G (+0.07%) and 34.32 G against 34.82 G (−1.44%). Getting
  there took three findings, each by building one change at a time. A commit at the top of every
  answer step cost the long recursion 1.6% of its instructions, so only a member commits where its
  last choice closes. One maker of nodes that tests whether the substrate is named cost 0.8%: it is
  called so often that a few instructions in it show, and carrying the place of a refusal across
  its exit to the member's maker alone cost 0.5%. A process alone now runs a copy of the maker with
  the test folded away, which a member leaves at a miss in a settled index of one empty place.
  Where the compiler puts the walker's loop moves the cycles by two to four percent between builds
  with equal instruction counts.
- The same day, two members making the same content at once, `ground/region-describes-itself.ck`,
  medians of five rounds: each took 6.28 G instructions and 1.01 G cycles; a process alone 6.88 G
  and 1.04 G; one member with nobody beside it 7.26 G and 1.20 G. A member alone pays for the
  commit's forwarding table, the index made once and the charge; two together make each node once
  between them, and 21,675 of their nodes were forwarded in the last round.
- The same day, what this Mac allows a member (two probes written and run in the session, not
  kept in the repository): a sparse object of 16 TiB, and a second process with another base that
  saw the first's bytes at the same offset and won a compare-and-swap there.
- 2026-11-27, version 21 against the bootstrap without the substrate (version 20), in three
  alternating rounds under `/usr/bin/time -l`, one process at a time, medians of cycles: the long
  recursion 3.85 G against 4.21 G (−8.6%; instructions −6.5%), forty thousand outcomes given back
  5.23 G against 5.51 G (−5.1%; −4.7%), and the compiled sweep of 37,822 cases 34.6 G against
  35.0 G (−1.2%; −2.0%). As first laid out the substrate took 4.45 G, 5.73 G and 40.4 G (+5.7%,
  +4.1%, +15.5%). Where those cycles went was found by building one change at a time and by
  sampling both programs every millisecond, and it was three choices of the compiler:
  - `room`, asked at every growth of a table, was a call. It now lies in the walker's loop. This
    gain does not come from the substrate: the same change alone gives the version without it
    −6.7%, −6.1% and −8.2% cycles.
  - The commit lay inside the walker's loop. It is now a call, made only when nodes settle.
  - The page-bit test lay inside the lookup of a binding, and the lookup then no longer lay in
    the walker's loop. The test is now a call, made only for a key past the dense prefix.

  What the substrate itself still costs is seen against version 20 with the same `room`: the long
  recursion +3.5% instructions and −2.0% cycles, the outcomes +4.2% and +1.1%, the sweep +7.3%
  and +7.6%. Of the sweep's instructions, 4.7 points go with the page-bit test, since the lookup
  of a binding is still a call from the walker's most used place (putting it in the loop gives
  back 3.6 points of instructions and no cycles); 1.1 with the second index and the test of
  what is settled, since of 81.7 million nodes made under a choice 52.7 million ask the settled
  index too and 44.3 million are found there; and 1.2 with the commit's test at each of 517
  million evaluation steps. Of its cycles, 2.5 points go with the second index and the commit's
  test and 1.4 with the page bits. Where the lane's areas start moves the sweep's cycles by up to
  3 points either way with the same instructions, so a smaller difference is not told apart from
  placement. The working index keeps its largest size beside the settled one: the peak footprint
  is 5.67 MB against 4.98 MB for the outcomes, 7.55 MB against 7.18 MB for the sweep and 1.93 MB
  against 1.87 MB for the long recursion.
- 2026-11-27, version 18 against version 17, in seven alternating rounds under `/usr/bin/time -l`,
  medians: the long recursion took 0.99 s of CPU against 1.01 s (cycles −1.6%, instructions
  +0.8%), three million steps of a choice 0.78 s against 0.83 s (cycles −5.9%), a million calls of
  a closure 0.23 s against 0.24 s (cycles −3.2%), and a loop of seven arguments 0.23 s against
  0.24 s (cycles −4.4%). The values in hand cost those instructions; each area beginning on its own
  set of the data cache gives back more cycles than they take, where with every area on a page the
  same walker took up to 11% more cycles.
- 2026-11-27, version 17 against version 16, which had fixed capacities, in alternating runs under
  `/usr/bin/time -l`: the region of 2.28 TiB is reserved in 1 to 6 µs, as the one of 2.5 GiB was.
  The long recursion in `ground/long-recursion.ck` (158.6 million walker steps) took 0.99 s of CPU
  against 0.97 s, with 1.2% more instructions, most of them the 64-bit hash's; its largest resident
  set was the same, and its page tables grew from 433 KiB to 673 KiB (`footprint`).
- 2026-11-25: recorded by a second process every 5 ms the long recursion took 0.71 to 0.76 s in
  three runs, and every 1 ms 0.72 to 0.75 s in five. The 1 ms recording held 481 frames.
- 2026-11-25: a recording of 117 frames was replayed. Its last frame holds the answer 2178309 and
  names the `add` node of the recipe as its writer.
- 2026-11-25: a build of the same walker with its registers in C locals and its tables on a private
  heap ran the long recursion in 0.56 s in two of three runs, where the region's took 0.71 s.

## Limits

- **The layout has a cost of its own.** Keeping the walker's registers in memory and stamping a
  writer on each record costs this run about a quarter of its speed (0.71 s against 0.56 s with
  registers in C locals and private tables). Reading costs the writer no instruction, but a reader that copies a page pulls
  its cache lines away from the writer; at 1 ms that was measured as up to about 4%.
- **Machine registers are not memory.** A value held only in a register of compiled code is seen
  once it is stored. The walker keeps its registers in the region for this reason.
- **Executable pages are outside.** macOS refused a shared executable mapping (`EINVAL`, tested).
  A code leaf's bytes are a node in the region and its runnable copy is in a private mapping; the
  region records the leaf, the offset of its copy and the address of that mapping. On macOS
  a process uses one `MAP_JIT` reservation. Linux x86-64 publishes code by changing the touched
  pages from writable to executable. Linux reserves anonymous framebuffer addresses with
  `MAP_NORESERVE` and charges touched pages to the runtime's shared allowance.
- **The C library's own memory is outside**: what the dynamic loader and the thread library keep.
  The process's arguments and its first thread's stack are outside until the move onto the
  region's stack.
- **A frame is a sample.** What a cell held between two frames is not kept, apart from its last
  writer. The journal that undo needs (stage 5) will keep old values inside the region.
- **The stack is recorded as raw bytes**, its newest 256 KiB, and the observer does not yet read it.
- **The allowance covers runtime-managed pages.** The live ground process runs with a 32 MiB
  allowance. It catches a room request that fits the room table's address range but exceeds the
  shared allowance, accepts the observer's binding, and completes its recursion. Multiple
  processes sample the same host independently. Raw host calls and other processes own their
  own allocations.

- **What is still counted by 32 bits or fewer**: a node's handle, so at most 2^32 − 1 nodes; the
  indexes of frames, links, activations, arguments, choice points, gathered values, born code,
  trails, room trails, held-back calls, notes, values in hand, and the nodes the reader opened and
  the children it gathers, so a composite has at most 2^32 − 1 children and a call as many values; the limit word, 2^23 − 1 of the program's
  own bindings and 127 from outside; the offset of born code, 4 GiB of it; and the floor, pages of
  16 KiB, so the stack lies within the region's first 64 TiB. The native stack is sized from
  host thread and stack limits and occupies at most half of the run's allowance.
- The runtime, live reader and observer were exercised together on macOS arm64.
- **A named substrate is not yet a channel.** Members share settled nodes and can name them by
  handle, but nothing carries a handle from one to another: shared rooms, `swap`, the inbox and the
  bell are not built, nor the side front. A lane is never used again, so a substrate has 256
  members in its life; and its indexes never grow. The id record of a forwarded node, and of a node given back under a
  search after a settled node's id was asked, keeps its 36 bytes.
  The numeric port does not answer by handle. A member is shown but not recorded. The counts of
  nodes settled in place and refused are not kept, only those forwarded. A commit that forwards
  charges the page of cells a cell moves onto without asking the allowance first, and forwards
  the writer of a node but not the writers stamped on frames and links. A
  process that dies while it buries a substrate is taken over by the next opener when its process
  id is gone; an id the host has given again meanwhile holds the name until `fb drop`.
- **Two statics lie outside the region**: where each keyed table's page bits begin and the key
  below which all its pages are used. Kept at the start of `touched` they cost the long recursion
  1.7% more instructions, measured, so they stay where they are.
- **One asker at a time.** Two processes asking at once can overwrite each other's request. The
  asker waits by looking, not by an event. A request is answered between two steps of the walker,
  so code that runs long without returning to it, emitted or the host's, delays the answer; a
  program that has ended answers nothing, and the asker gives up after five seconds.
- **Asking is not gated.** Any process that can map the region can ask it anything a program can
  do. Only the user that made the region can map it: it is created with mode 0600.
