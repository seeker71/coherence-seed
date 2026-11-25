# Knowledge representation for the design graph — survey and recommendation

Study date: 2026-11-24. All URLs below were fetched on 2026-11-24 unless a line says otherwise.

## 0. How to read this report

Status words used on every claim:

- **observed** — read or run in this session. For web pages this means the WebFetch tool, which returns a
  small model's reading of the page rather than the raw bytes; short quotes are therefore "as relayed" and
  should be re-read before they are carved anywhere permanent. Two papers (Plate 1991, Kanerva 2009) were
  read directly as page images, and local files and the byte/hash probe were read or run directly.
- **inferred** — my reasoning from observed things; the premises are named.
- **assumed** — not verified in this session (model memory, or a search-result snippet that was not opened).

Limits of this study, stated once:

- No tokenizer was run. Token figures are the published ones; my own probe measures bytes only (section 5.3).
- Fetches that failed: `wiki.opencog.org` (DNS), `metta-lang.dev` (empty body), IEEE Xplore page for Plate
  1995 (empty), Springer page for Kanerva 2009 (login redirect; a course-hosted copy was read instead),
  `peerj.com/articles/cs-78` (403), `iso.org/standard/76120.html` (403), Unison codebase-format page (404).
  Anything about those topics that did not come from another fetched page is marked assumed.
- The new grammar does not exist yet, so "expressible in the system's own grammar" is argued from the five
  axioms, not demonstrated.

## 1. What the format has to do

From the task: (a) token-efficient for models to read and write; (b) expandable without restructuring;
(c) every part has the shape of the whole and can be understood alone; (d) every claim carries source, date,
status, confidence and can be superseded without being overwritten; (e) plain text under git, diffable and
mergeable; (f) later expressible as content-addressed nodes in the one grammar.

Requirement (d) and (f) are already stated by the fixed ground. Axiom 3: "nothing-referenced-is-overwritten
new-shape-mints-a-new-node-id", "undo-is-repoint a-name-at-a-still-referenced-blueprint", names are "free
query keys" (`coherence-kernel/axioms/core-axioms.form:66-75`,
last commit touching the file 7b0ff6bed, 2026-11-19; observed). Axiom 5 gives the acknowledgment values
"exactly one of nothing, 0, 1, or node" (same file, line 88; observed), and the theorem
`absence-is-not-a-verdict` separates "nobody answered" from "everybody said no" (lines 212-219; observed).
The survey below is read against those lines.

## 2. Survey

Each entry: what it is, the property worth keeping, the cost, and the source.

### 2.1 RDF, RDF 1.2 triple terms, named graphs, N-Quads

- RDF 1.2 Concepts is a W3C Candidate Recommendation Draft dated 16 November 2026 (latest CR snapshot
  7 April 2026); it is not yet a Recommendation. RDF 1.2 Turtle and N-Quads are still Working Drafts
  (11 and 20 November 2026). Sources: https://www.w3.org/TR/rdf12-concepts/ and
  https://www.w3.org/groups/wg/rdf-star/publications/ (observed).
- What used to be called RDF-star is now "triple terms": a triple used as the object of another triple;
  "A reifying triple is a triple where the predicate is `rdf:reifies` and the object is a triple term" (§1.5).
  A triple term is not asserted unless the same triple is also asserted. Triple terms cannot be subjects
  (N-Quads draft). The spec body no longer uses the word "RDF-star". (observed, same URLs plus
  https://www.w3.org/TR/rdf12-n-quads/)
- Named graphs: a dataset has one default graph and zero or more named graphs; "the graph name does not have
  to denote the graph" (§4 note). (observed)
- N-Quads is line-based: one statement per line, subject predicate object and an optional graph label, ended
  by a period; Canonical N-Quads gives each quad exactly one textual form. (observed)
- RDF Dataset Canonicalization (RDFC-1.0) is a W3C Recommendation of 21 May 2024: it labels blank nodes
  deterministically so that equal datasets serialise identically and can be hashed or signed; SHA-256 by
  default. https://www.w3.org/TR/rdf-canon/ (observed)
- SHACL (W3C Recommendation, 20 July 2017) validates graphs against shapes and returns a report with a
  boolean conformance result. https://www.w3.org/TR/shacl/ (observed)

Keep: one statement per line; a canonical text form so that a hash is meaningful; statements about
statements (provenance) as a first-class thing; shape validation as a separate mechanical pass.
Cost: verbose IRIs; the statement-about-statement machinery is still moving in 2026; measured token cost is
high (section 3.2).

### 2.2 PROV-O

W3C Recommendation, 30 April 2013 (https://www.w3.org/TR/prov-o/, observed). Three starting classes: Entity,
Activity, Agent. `wasDerivedFrom` has the sub-properties `wasRevisionOf`, `wasQuotedFrom` and
`hadPrimarySource`; `generatedAtTime` and `invalidatedAtTime` bound an entity's existence;
`wasInvalidatedBy` points at the activity that ended its usability; versions of one thing are linked by
`specializationOf` to a common entity.

Keep: the vocabulary split between *derived from another claim*, *revision of*, *quoted from* and *primary
source*; invalidation as a dated relation rather than a deletion; a review round as an Activity.

### 2.3 Nanopublications and trusty URIs

- A nanopublication is a small, self-contained, citable unit made of four named graphs: head, assertion,
  provenance, publication info. Well-formedness is a list of mechanical rules (exactly one assertion link,
  provenance must reference the assertion, publication info must reference the nanopublication, and so on).
  https://nanopub.net/guidelines/working_draft/ (observed; the document carries no date).
- Trusty URIs put a cryptographic hash of the content into the identifier, making artefacts "verifiable,
  immutable, and permanent", and can hash at the graph level so the id survives a change of serialisation.
  Kuhn and Dumontier, arXiv:1401.5775, submitted 16 January 2014. https://arxiv.org/abs/1401.5775 (observed)
- "A nanopublication is persistent, you can never edit nor delete it"; a retraction is a new
  nanopublication using `npx:retracts`, valid only if signed with the same key as the original, and
  retracted nanopublications are hidden from search by default.
  https://nanopub.readthedocs.io/en/latest/publishing/retraction.html (observed, undated page)
- New versions point back with `npx:supersedes` (assumed: search snippet from the nanopub registry FAQ and
  arXiv:2203.01608, not opened).

Keep: this is the closest existing match to requirement (d). Claim plus its provenance plus its publication
record is the unit; the unit is immutable and hash-named; change is a new unit that points at the old one;
"current" is a derived view.
Cost: four RDF graphs and a signature per claim is far heavier than a design graph needs.

### 2.4 Property graphs and GQL

ISO/IEC 39075:2024 (GQL) was published 12 April 2024; nodes and edges carry labels and properties; property
values cannot be graphs or elements, so graphs do not nest.
https://en.wikipedia.org/wiki/Graph_Query_Language (observed; the ISO page itself returned 403).

Keep: properties directly on nodes. Let go: the non-nesting rule conflicts with "every part has the shape of
the whole"; property graphs have no native statement-level provenance and no standard text form for git.

### 2.5 Datalog and Datomic datoms

- A datom is an immutable five-tuple: entity, attribute, value, transaction, and an added/retracted flag.
  A retraction is a new datom; the original assertion "remains unchanged". Transactions are totally ordered,
  are themselves entities ("transactions can have datoms that describe them"), and the database can be read
  "as of" any point in time. https://docs.datomic.com/whatis/data-model.html (observed, undated, build b391f42d)
- Four sort orders over the same datoms: EAVT (everything about an entity), AEVT (a column), AVET (lookup by
  value), VAET (reverse references). https://docs.datomic.com/indexes/index-model.html (observed)
- Datalog: facts and rules, evaluated bottom-up, pure Datalog always terminates.
  https://en.wikipedia.org/wiki/Datalog (observed)

Keep: an append-only log of facts where retraction is a fact; the writing transaction is itself a describable
entity (that is a review round); indexes are derived sort orders of one log, never hand-kept; derived views
(current, stale, contested) are terminating rules over facts.

### 2.6 Content-addressed Merkle DAGs: git, IPLD, Unison, prolly trees

- Git: "Git is a content-addressable filesystem"; an object's name is the SHA-1 of a `<type> <size>\0`
  header plus the content; a blob stores content without its filename; objects live at
  `.git/objects/<2 hex>/<38 hex>`. https://git-scm.com/book/en/v2/Git-Internals-Git-Objects (observed)
- Git's move to SHA-256 (after the SHAttered collision of 23 February 2017) needs a bidirectional mapping
  table between old and new names, because the hash is inside every reference.
  https://git-scm.com/docs/hash-function-transition (observed)
- IPLD/IPFS: a node's identifier is the hash of its payload plus its children's identifiers; any change
  changes the identifier of every ancestor; cycles are practically impossible; nodes can be shared by several
  parents. A CID carries a version, a content-type code and a self-describing multihash ("Avoid Lock-in").
  https://docs.ipfs.tech/concepts/merkle-dag/ , https://ipld.io/docs/intro/primer/ ,
  https://github.com/multiformats/cid (observed; the cid repository is archived and points to specs.ipfs.tech)
- Unison: "Unison code is content-addressed"; names are "separately stored metadata that don't affect the
  function's hash"; renames are non-breaking; two versions of a definition coexist; 512-bit SHA3.
  https://www.unison-lang.org/docs/the-big-idea/ (observed)
- Prolly trees (DoltHub, 3 March 2024): the same data gives the same tree whatever the order of operations;
  diff "scales with the size of the differences, not the size of the tree".
  https://www.dolthub.com/blog/2024-03-03-prolly-trees/ (observed)
- IPLD HAMT: every node has the same form (a bitmap and an array of buckets or links); the same key/value
  set gives the same root CID regardless of insertion order.
  https://ipld.io/specs/advanced-data-layouts/hamt/spec/ (observed, draft spec)

Keep: identity computed from content; names in a separate, mutable, tiny layer; the hash algorithm recorded
with the store; order-independent structure. Two warnings that shape the recommendation:
(1) In a Merkle DAG a node cannot point at something that will later point back at it, and changing a child
changes every ancestor's id. A design graph wants hubs whose membership grows. So membership has to be
declared by the child (child points at hub), not listed in the hub (inferred from the IPFS Merkle-DAG page).
(2) A full hash is long. References need a short unique prefix, as git abbreviations do (inferred).

### 2.7 Hypergraphs: OpenCog AtomSpace, MeTTa

The AtomSpace README describes "a kind of in-RAM generalized hypergraph (metagraph) database" that "stores
immutable, globally unique, typed s-expressions"; "a (mutable) key-value database is attached to each Atom";
TruthValues are on the deprecation list ("Just use FloatValue instead").
https://github.com/opencog/atomspace (observed). Hyperon/MeTTa is presented as "Atomese 2", in an early
experimental stage. https://github.com/trueagi-io/hyperon-experimental (observed). MeTTa's atom kinds and
non-deterministic matching: assumed (model memory; the documentation site returned an empty body).

Keep: immutable, globally unique atoms with a separate mutable value attachment is the same split as
git objects/refs and Unison hashes/names. A link is itself an atom, so a statement about a statement needs
no special machinery.

### 2.8 Conceptual graphs and Common Logic

Sowa introduced conceptual graphs in 1976 and developed them in a 1984 book; concepts and relations are both
nodes; CGIF is one of three dialects of ISO/IEC 24707 Common Logic (completed October 2007; a 2018 second
edition exists). https://en.wikipedia.org/wiki/Conceptual_graph , https://en.wikipedia.org/wiki/Common_Logic
(observed). Nested contexts in conceptual graphs: assumed (model memory).

Keep: a relation is a node. Nothing else here changes the recommendation.

### 2.9 Frames, schema.org, JSON-LD

JSON-LD 1.1 is a W3C Recommendation of 16 July 2020; `@context` maps short terms to IRIs; the same data has
expanded, compacted, flattened and framed forms. https://www.w3.org/TR/json-ld11/ (observed).
schema.org (V30.1, 2026-09-16) has a `Claim` type that "could be the itemReviewed in a ClaimReview", with
`firstAppearance` and `appearance`; it "does not currently define any types of relationship between claims".
https://schema.org/Claim (observed). Minsky's frames as the ancestor of slot-and-filler records: assumed
(model memory).

Keep: the `@context` idea is the lens idea — short local keys, with the mapping to global meaning stated once
at the root rather than in every node. Claim and review as separate things. Cost: section 3.2.

### 2.10 Zettelkasten and atomic notes

- A Zettel has a unique identifier, a body, and references; atomicity is "one knowledge building block per
  note"; with title-based ids "you cannot change the title unless you change any reference to it"; a link
  should state why it was made; a structure note "is a Zettel about other Zettel and their relationships".
  https://zettelkasten.de/introduction/ (27 October 2020; observed)
- Luhmann, "Communicating with Slip Boxes" (translation of the 1981 essay; the page shows no date):
  "we decide against the systematic ordering in accordance with topics and sub-topics"; "we never change this
  number and thus the fixed place of the slip"; "the fixed filing place needs no system"; a slip 57/12 can be
  continued by 57/13 and branched at any word by 57/12a; a keyword register is needed because memory cannot
  hold the numbers; the box develops clusters on its own.
  https://luhmann.surge.sh/communicating-with-slip-boxes (observed)

Keep: this is the oldest working answer to requirement (b). Fixed address, never a topical filing order;
structure is carried by links and by structure notes that are themselves notes; entry is through a small
register. A-MEM (arXiv:2502.12110, NeurIPS 2025) applies exactly this to agent memory.
https://arxiv.org/abs/2502.12110 (observed)

### 2.11 Architecture decision records, C4, arc42

- Nygard, 15 November 2011: Title, Context, Decision, Status, Consequences; "If a decision is reversed, we
  will keep the old one around, but mark it as superseded"; "numbered sequentially and monotonically. Numbers
  will not be reused"; one or two pages.
  https://cognitect.com/blog/2011/11/15/documenting-architecture-decisions (observed)
- MADR 4.0.0 (17 September 2024): adds considered options with pros and cons; front matter `status`, `date`,
  `decision-makers`; files `NNNN-title-with-dashes.md` under `docs/decisions/`; in subdirectories numbers are
  unique only per category. https://adr.github.io/madr/ (observed)
- C4: four hierarchical abstractions (software system, container, component, code), notation independent.
  https://c4model.com/ (observed)
- arc42: twelve fixed sections, including 9 Architectural Decisions, 11 Risks and Technical Debt,
  12 Glossary. https://arc42.org/overview (observed)

Keep: a decision is a record with context, options and consequences that is superseded, never rewritten; a
glossary is part of the architecture; zoom levels. Let go: sequential numbers as identity (they collide
between branches — Beads moved to hash ids for this reason, section 2.15); status edited in place inside the
old record; a fixed table of twelve sections (a fixed top-level taxonomy is the thing Luhmann decided
against).

### 2.12 Argument maps: IBIS and Toulmin

IBIS (Kunz and Rittel, working paper July 1970): issues, positions, arguments; positions answer issues,
arguments support or object to positions, issues can attach to anything.
https://en.wikipedia.org/wiki/Issue-based_information_system (observed).
Toulmin (The Uses of Argument, 1958): claim, grounds, warrant, backing, qualifier, rebuttal.
https://en.wikipedia.org/wiki/Toulmin_method (observed; redirects to the biography page).

Keep: an open question is a node, not an absence; a decision answers a question; arguments attach by typed
edges. Toulmin's parts map onto fields: grounds = source, warrant = the `from` edges of an inference,
qualifier = confidence, rebuttal = a stated limit.

### 2.13 "Holographic", literally: HRR and vector-symbolic architectures

- Plate, "Holographic Reduced Representations: Convolution Algebra for Compositional Distributed
  Representations", IJCAI-91, p. 30 (read directly): circular convolution associates vectors; bindings,
  sequences and frames "can be compressed into a fixed width vector. These representations are items in their
  own right, and can be used in constructing compositional structures. The noisy reconstructions given by
  convolution memories can be cleaned up by using a separate associative memory". The introduction restates
  Hinton's reduced description: the compressed vector "acts as a reduced description of the set of vectors
  and itself can be a member of another set of vectors"; the reduction must be reversible so one can move
  "in both directions in a part-whole hierarchy". https://www.ijcai.org/Proceedings/91-1/Papers/006.pdf (observed)
- Kanerva, "Hyperdimensional Computing", Cognitive Computation 1:139-159, published online 28 January 2009,
  p. 142 (read directly): "the information encoded into a representation should be distributed 'equally' over
  all the components ... When bits fail, the information degrades in relation to the number of failing bits
  irrespective of their position. This kind of representation is referred to as holographic or holistic. It
  is very different from the encoding of data in computers and databases where the bits are grouped into
  fields". Course copy at https://cs.uwaterloo.ca/~jhoey/teaching/cogsci600/papers/Kanerva09.pdf (observed)
- Gayler, "Vector Symbolic Architectures answer Jackendoff's challenges", conference July 2003,
  arXiv:cs/0412059 of 13 December 2004. https://arxiv.org/abs/cs/0412059 (observed)
- Operations (bind, bundle, permute) and the survey of models (TPR, HRR, MAP, BSC, SBDR):
  https://en.wikipedia.org/wiki/Hyperdimensional_computing and Kleyko et al., arXiv:2111.06077, ACM Computing
  Surveys 55(6). (observed)

What this means for a text graph (inferred from the two papers read directly):

1. The literal holographic property — information spread equally over all components, graceful degradation
   regardless of position — is a property of a vector, and Kanerva contrasts it explicitly with field-grouped
   records. A text file under git cannot have it. Claiming it for a text format would be untrue.
2. What a text graph *can* have is Plate's and Hinton's other property: the **reduced description**. A
   fixed-size summary of a set, which is itself an item that can be a member of a larger set, in a part-whole
   hierarchy that can be walked in both directions. That is a hub node: a bounded summary of its members that
   is a member of the hub above it.
3. HRR decoding is noisy and needs a clean-up memory of exact items. The exact text nodes are that clean-up
   memory. A hypervector or embedding per hub is therefore a possible *derived index* over the graph (a
   faster, lossy state of the same nodes), never the store of record.

### 2.14 Fractal and self-similar structures

- Holon (Koestler, 1967): "a whole in its own right that is also a part of a larger whole".
  https://en.wikipedia.org/wiki/Holon_(philosophy) (observed)
- GraphRAG: recursive Leiden clustering "until we reach a community-size threshold"; a report is generated for
  every community at every level; "if community A is the top-level community, we'll get a report about the
  entire graph". https://microsoft.github.io/graphrag/index/default_dataflow/ (observed)
- HAMT and prolly trees (section 2.6): one node shape at every depth; a node that overflows becomes a parent
  of nodes of the same shape. Fractal tree index (Bender, Farach-Colton, Kuszmaul, 2007): a B-tree variant
  with per-node buffers. https://en.wikipedia.org/wiki/Fractal_tree_index (observed)
- llms.txt (proposed 3 September 2024; page last modified 10 August 2026, "v2"): a small markdown file of
  links at the site root "or at any subpath", most specific file wins; an "Optional" section an agent can
  skip. https://llmstxt.org/ (observed)
- Agent Skills (16 October 2025): three levels of progressive disclosure; "the amount of context that can be
  bundled into a skill is effectively unbounded" because nothing is loaded until needed.
  https://www.anthropic.com/engineering/equipping-agents-for-the-real-world-with-agent-skills (observed)

Keep: one shape at every level; a size threshold that triggers a split into children of the same shape;
a summary at every level; an entry file at every level.

### 2.15 Agent memory designs, 2025-2026

- MCP knowledge-graph memory server: entities (unique name, type, observations), directed relations in active
  voice, observations "should be atomic (one fact per observation)", stored as a JSONL file.
  https://github.com/modelcontextprotocol/servers/tree/main/src/memory (observed)
- Zep/Graphiti: temporally aware knowledge graph (arXiv:2501.13956, 20 January 2025); README: "When
  information changes, old facts are invalidated — not deleted"; facts carry validity windows; every entity
  and relation traces back to the episode that produced it.
  https://arxiv.org/abs/2501.13956 , https://github.com/getzep/graphiti (observed)
- Letta, 12 August 2025: an agent with only file tools (grep, search, open) scored 74.0% on LoCoMo with
  GPT-4o mini, against 68.5% reported for Mem0's graph variant.
  https://www.letta.com/blog/benchmarking-ai-agent-memory (observed)
- Arize, October 2026: replaced a Graphiti design with one text file per user capped at 8,000 characters
  (about 2,000 tokens), loaded whole; reasons given: about 10 model calls per write in the graph design,
  retrieval misses, and "you open the file and read it" when memory is wrong.
  https://arize.com/blog/alyx-agent-long-term-memory-architecture/ (observed)
- MOOSEDev (arXiv:2608.13662, 13 August 2026): typed project-memory records with lifecycle status,
  provenance and supersession links; on supersession, set-completeness and negation questions it returned
  0.98-1.00 of the expected answers where a vector-memory baseline surfaced 6%-27%; relevance recall and
  token cost were about equal. https://arxiv.org/abs/2608.13662 (observed, abstract only)
- Beads: "Hash-based IDs (`bd-a1b2`) prevent merge collisions in multi-agent/multi-branch workflows";
  hierarchical child ids `bd-a3f8.1.1`; link types include `supersedes`; storage has moved to Dolt, with
  JSONL as an export only. https://github.com/steveyegge/beads (observed)
- Context cost: "the smallest possible set of high-signal tokens"; keep "file paths, stored queries, web
  links" and load on demand (Anthropic, 29 September 2025,
  https://www.anthropic.com/engineering/effective-context-engineering-for-ai-agents ). Performance becomes
  "increasingly unreliable as input length grows" across 18 models, and a focused prompt of about 300 tokens
  beat a full 113k-token prompt on LongMemEval (Chroma, 14 July 2025,
  https://www.trychroma.com/research/context-rot ). Position effects: arXiv:2307.03172. (observed)

Reading (inferred): plain files that an agent can grep are competitive with graph databases for recall, and
typed records with explicit supersession win on exactly the questions a design graph is asked ("what is
current", "what replaced this", "is the set complete"). The two are compatible: typed, superseding records
*in* plain files.

### 2.16 Stable numeric ids with names as a lens: Wikidata, NUMS, the present body

- Wikidata: an entity id is "a number prefixed by a letter" (Q, P, L); labels are per language and need not
  be unique; "there may be multiple items with the same label". Statement ranks are preferred, normal,
  deprecated; "The deprecated rank is used for statements that are known to include errors", and deprecating
  is preferred over deleting; a "reason for deprecated rank" qualifier is recommended; the best-rank rule
  ("preferred, if there is none: normal") defines what queries see by default.
  https://www.wikidata.org/wiki/Wikidata:Identifiers (edited 20 March 2025),
  https://www.wikidata.org/wiki/Help:Label (17 April 2026), https://www.wikidata.org/wiki/Help:Items
  (24 September 2026), https://www.wikidata.org/wiki/Help:Ranking (26 October 2026). (observed)
- NUMS.Go: a node id is four unsigned integers, `Package.Level.Type.Instance`, printed dotted
  (`NUMS.Go/nums/nums_nodes.go:11-24, 55-57`); names live in separate symbol tables
  mapping strings to instances and ids (`NUMS.Go/nums/nums_symbols.go:85-103`); a new
  instance number is the table's length plus one (`nums_symbols.go:141`). Last commit 9e6bbb4, 2023-04-05.
  (observed)
- The present body mints the same four coordinates with `make_nodeid p l t i` and, since 2026-09-14, equal
  coordinates answer the same cell (memory note
  `memory/reference-make-nodeid-mechanism.md`;
  observed as a note, not re-witnessed in code). Its memory index is derived from the notes, "never keep it
  by hand" (`reference-memory-index-is-derived-by-form-find.md`, 2026-10-08; observed as a note).
- KG-LLM-Bench found that replacing entity names with pseudonyms changed the overall score by 0.2 points
  (0.385 to 0.383). https://arxiv.org/html/2504.07087v1 Table 5 (observed)

Reading (inferred): models do not need meaningful names inside the graph to reason over it, so numeric ids
with a detachable name table cost little in accuracy. A per-table counter (NUMS, ADR numbers) is fine inside
one process and collides across git branches; a number derived from content does not.

### 2.17 Confidence

The Admiralty system rates source reliability (A-F) and information credibility (1-6) separately; "each
descriptor is considered in isolation"; both scales have an explicit "cannot be judged" value.
https://en.wikipedia.org/wiki/Admiralty_code (observed). Wikidata uses three ranks rather than numbers
(above). The AtomSpace is deprecating its dedicated truth-value type (2.7).

Reading (inferred): no surveyed system defends a numeric confidence on hand-written claims. A coarse ordinal,
kept separate from the status (how it is known), with an explicit "cannot judge", is the honest form.

## 3. Token efficiency: what is actually measured

### 3.1 Tabular and nested data

| Study (date) | Finding | Source |
|---|---|---|
| ImprovingAgents (30 Sep 2025), GPT-4.1-nano, 1,000 records, 11 formats | Markdown key-value blocks 60.7% at 52,104 tokens; INI 55.7% at 48,100; YAML 54.7% at 55,395; JSON 52.3% at 66,396; Markdown table 51.9% at 25,140; JSONL 45.0% at 54,407; CSV 44.3% at 19,524 | https://www.improvingagents.com/blog/best-input-data-format-for-llms |
| ImprovingAgents (14 Oct 2025), nested data, 3 models | YAML most accurate on two of three models (62.1% vs JSON 50.3% on GPT-5 Nano); Markdown cheapest everywhere, 34-38% fewer tokens than JSON; Llama 3.2 3B showed no significant format effect | https://www.improvingagents.com/blog/best-nested-data-format |
| TOON README (spec v4.4 of 8 Oct 2026), 4 models, 244 questions, o200k_base tokens | TOON 72.2% at 2,474 tokens; JSON compact 69.0% at 2,892; YAML 70.1% at 3,487; JSON 71.4% at 4,308; XML 70.7% at 4,909. On mixed-structure data TOON is 1.6% *larger* than compact JSON; on flat data CSV is 5.9% smaller than TOON; on gpt-5.4-nano TOON scored below XML and JSON | https://github.com/toon-format/toon , changelog https://raw.githubusercontent.com/toon-format/spec/main/CHANGELOG.md |
| He et al. (15 Nov 2024) | Same content as plain text, Markdown, JSON, YAML: up to 40% swing on GPT-3.5-turbo for code translation; GPT-4 more robust | https://arxiv.org/abs/2411.10541 |
| Sui et al., WSDM 2024 | Table format, order and partition marks change results; no single winner in the abstract | https://arxiv.org/abs/2305.13062 |

All observed. The TOON numbers are the project's own benchmark. TOON is a Working Draft whose version 4.0
(22 July 2026) was a breaking release.

### 3.2 Graphs specifically

KG-LLM-Bench (arXiv:2504.07087, 9 April 2025, seven models, five tasks), Tables 3 and 5 (observed):

| Textualisation | Mean input tokens | Overall score |
|---|---|---|
| List of edges | 2,644.8 | 0.412 |
| Structured YAML | 2,903.1 | 0.406 |
| Structured JSON | 4,504.7 | 0.419 |
| RDF Turtle | 8,171.1 | 0.349 |
| JSON-LD | 13,503.4 | 0.342 |

The same graphs cost three to five times the tokens in Turtle or JSON-LD and scored about seven points
lower. The cheapest form, one edge per line, was within a point of the best.

### 3.3 Reading across the studies (inferred)

1. There is no universally best syntax; results move with the model and the task. Weaker models are more
   sensitive, and the body's own voice is a local 27B model, so a familiar surface is the safe choice.
2. What is stable across the studies: self-describing `key: value` lines and one-edge-per-line lists are at
   or near the best accuracy while costing well under JSON; semantic-web serialisations cost several times
   more for less; a novel compact notation can lose on a small model.
3. Context length hurts more than syntax helps (Chroma, Anthropic, Arize in 2.15). The larger lever is
   never loading what is not needed: small files, a summary at every level, references by short id.

## 4. Comparison against the six requirements

Legend: + meets, o partly, - does not. Judgements are inferred from the sources in section 2.

| Approach | a tokens | b expand | c self-similar | d provenance, supersession | e git text | f content-addressed |
|---|---|---|---|---|---|---|
| RDF 1.2 / N-Quads / JSON-LD | - | + | o | + (triple terms, named graphs, PROV) | + (N-Quads) | o (needs canonicalisation) |
| Nanopublication | - | + | o | + | o | + (trusty URI) |
| Property graph | o | + | - (no nesting) | - | - | - |
| Datomic datoms | + | + | o | + | - (database) | - |
| git / IPLD / Unison objects | o | + | + | o (history, no claim fields) | o | + |
| AtomSpace / MeTTa | o | + | + | o | - | o |
| Zettelkasten | + | + | + (structure notes) | o (references only) | + | - |
| ADR / MADR | o | o (counter collides) | - | o (status edited in place) | + | - |
| IBIS / Toulmin | + | + | o | o | n/a | n/a |
| VSA hypervectors | + (fixed width) | - (capacity) | + | - (lossy) | - | - |
| GraphRAG hierarchy | o | o (regenerated) | + | o | - | - |
| Plain key-value claim blocks with content ids (section 5) | + | + | + | + | + | + |

No single prior system meets all six. The recommendation combines four of them: the nanopublication's unit
and immutability, the datom's append-only log with derived views, git/Unison's content id plus separate
names, and the Zettelkasten's fixed address with structure notes.

## 5. Recommendation

Everything in this section is a proposal (inferred) unless marked otherwise.

### 5.1 The model in seven sentences

1. The unit is a **node**: one statement plus how it is known, as a short block of `key: value` lines.
2. A node is **immutable**; its id is computed from its text.
3. Nodes connect by **typed edges that only point at nodes that already exist** (`part`, `from`,
   `supersedes`, `answers`, `supports`, `disputes`, `re`, `src`).
4. A **hub** is an ordinary node whose statement is a bounded summary; other nodes declare `part: <hub>`.
   Hubs are parts of hubs. The root hub is the whole design in one screen.
5. **Change is addition**: a correction is a new node with `supersedes: <old>`; a review is a new node with
   `re: <target>` and an acknowledgment; nothing is edited or deleted.
6. **Views are derived**: current, superseded, contested, stale, unreviewed, the reverse edges and the
   id-to-file index are computed from the nodes, never written by hand.
7. **Names are a lens**: a separate file maps ids to names in any language; renaming changes no id.

### 5.2 On-disk format

A file is UTF-8 text: blocks separated by exactly one blank line. Every line is `key: value` — lower-case
key, colon, one space, value to end of line. No nesting, no quoting, no continuation lines, no tabs, LF
endings. Keys appear in one fixed order. This is deliberately not YAML: the value is always raw text, so no
typing rules apply and a parser is a few lines in any language.

The example below is real: the ids were computed in this session with `shasum -a 256` over the block's lines
after the `n:` line (probe files under
`scratch/kgprobe/`; observed).

```
n: c808c09b
kind: claim
is: A cell is identified by its present blueprint: the same content-addressed shape is the same cell, and a new composition mints a new id.
status: observed
conf: high
src: file axioms/core-axioms.form:66-75 @7b0ff6bed
src-date: 2026-11-19
at: 2026-11-24
by: study/web-knowledge-representation
part: 91673052
```

An inference from it, and a review of it (ids also computed):

```
n: 25d9086d
kind: claim
is: A graph node is never edited in place: a revision is a new node that names the node it supersedes, and the current view is derived.
status: inferred
conf: high
from: c808c09b
at: 2026-11-24
by: study/web-knowledge-representation
part: 91673052

n: 7200e47a
kind: review
re: c808c09b
ack: 1
is: Re-read lines 66-75 at the pinned commit; the statement matches the source.
at: 2026-11-24
by: review/round-1
```

### 5.3 What the byte probe showed

The claim block above, in four renderings (observed, `wc -c`):

| Rendering | Bytes |
|---|---|
| key-value block | 333 |
| JSON, compact | 365 |
| JSON, pretty | 407 |
| Turtle with PROV terms, without the three prefix lines | 465 |
| Turtle including the prefix lines | 601 |
| the statement sentence alone | 135 |

Bytes are not tokens, so this is a proxy only. What it shows: for a prose-carrying node the syntax accounts
for roughly a tenth to a fifth of the size against JSON. The sentence is 40% of the block and the `src` and
`by` strings another quarter. The large savings are therefore structural, not syntactic: cite a shared
`source` node by id instead of repeating a URL, keep `by` short, keep the sentence to one sentence, and do not
load files that are not needed.

### 5.4 Fields

Required on every node:

| Key | Meaning | Rule |
|---|---|---|
| `n` | id | first 8 hex digits of SHA-256 over the block's remaining lines, each ended by LF; stamped by the tool, not hand-written; lengthened if the prefix is not unique |
| `kind` | `axiom`, `claim`, `decision`, `question`, `term`, `source`, `hub`, `round`, `review` | closed list, extended only by a `term` node defining the new kind |
| `is` | the statement | one line, at most 280 characters, understandable without its neighbours |
| `at` | date this node was written | ISO date, not in the future |
| `by` | the writing role or session | a role path, never a person's name or address |

Required by condition:

| Key | When | Meaning |
|---|---|---|
| `status` | all kinds except `round`, `review` | `observed`, `inferred`, `assumed` |
| `conf` | wherever `status` is present | `high`, `mid`, `low`, `unjudged` |
| `src` | `status: observed` | `file <path>:<lines> @<commit>` or `<source-node-id> <locator>`; for `assumed`, `src: model-memory` is allowed |
| `src-date` | when the source has its own date | ISO date, or `undated` |
| `from` | `status: inferred` | one line per premise id |
| `test` | `status: assumed` | how the assumption could be checked or moved |
| `part` | every node except the root hub | one line per hub; more than one is allowed |
| `re`, `ack` | `kind: review` | target id; `1` confirms, `0` disputes |
| `url`, `title` | `kind: source` | the address and the source's own title |

Optional: `limit` (a stated current limit), `supersedes`, `answers` (a decision or claim to a question),
`supports`, `disputes`, `see`.

Why `status` and `conf` are separate: status is how the writer knows it, confidence is how far to lean on it
(the Admiralty split, 2.17). Why an ordinal: 2.17.

### 5.5 Ids

Both kinds, each doing one job.

- **Identity is the content hash.** It is what axiom 3 already says, it is checkable by anyone, it needs no
  counter and so cannot collide between branches or agents, and it makes "same statement with the same
  provenance" the same node everywhere. It is written as a short number (8 hex digits, about 65,000 nodes
  before a prefix collision becomes likely; a collision is detected mechanically and the prefix lengthened,
  as git abbreviations are).
- **The stable handle of something that evolves is the first node of its supersession chain.** No second id
  scheme is needed: `head(x)` follows `supersedes` forward. A reference deliberately pins the version that was
  read; "x has a newer head" is then a computable signal, not a silent drift.
- **Names are a lens, stored apart.** `lens/en.names` holds `id name` lines; other languages sit beside it. A
  node may have no name, one, or many. Nothing resolves through a name.
- **Structured numeric coordinates** in the NUMS sense (`package.level.type.instance`) are the right identity
  for blueprints and recipes inside the running system, where one process allocates instances. For a
  hand- and model-written graph under git with several writers, a counter-allocated instance collides; I
  recommend the content number here and leave open whether the graph's nodes later receive coordinates when
  they are ingested as cells (open question 3).
- The hash algorithm and prefix length are stated once in the root `FORMAT` file, so they can change without
  ambiguity (the lesson of git's SHA-1 transition, 2.6).

Writing workflow (needed because a model cannot compute SHA-256): the writer leaves `n: ?label` and refers to
`?label` within the same batch; a mint step orders the batch, computes each id, and substitutes.

### 5.6 Directory layout

```
graph/
  FORMAT            the rules of section 5.2-5.5 as a dozen nodes: keys, kinds, hash, prefix length
  0.kg              the root hub and the nodes that summarise the whole design
  0/
    axioms.kg       one file per child hub: the hub node first, then its members
    grammar.kg
    grammar/        appears only when grammar.kg outgrows its budget; same shape inside
      choice.kg
    sources.kg      source nodes
    rounds/
      2026-11-24-r1.kg    a round node and its review nodes
  lens/
    en.names        id, space, name; one per line
  .index/           derived: id to file and line, reverse edges, heads, stale and due lists
```

Rules that make it scale without restructuring:

- A file holds one hub and the nodes that are `part` of it. Budget: 8 KB or 24 nodes, whichever comes first
  (8,000 characters is about 2,000 tokens in the Arize report, 2.15; GraphRAG chunks at 1,200 tokens, 2.14).
- When a file exceeds the budget, new child hubs are written and some members move to the child files. The
  moved blocks are byte-identical, so no id and no reference changes; only the derived index moves.
- File and directory names are lenses. Membership is the `part` edge. A node with two `part` lines lives in
  one file and is found from the other hub through the index.
- At every depth the reader meets the same thing: a summary sentence, then members, some of which are
  summaries of deeper files. Reading only `0.kg` gives the whole at low resolution.

### 5.7 Review rounds and supersession

- A **round** is a node (`kind: round`) stating scope, method and date. Each check made in the round is a
  `review` node that is `part` of the round and points at its target with `re`.
- The acknowledgment of a node takes the four values of axiom 5: **nothing** (no review exists yet — the node
  is `unreviewed`, which is not `refuted`), **1** (a review confirms), **0** (a review disputes, with its
  reason in `is`), **node** (a new node supersedes it).
- A superseding node repeats the full statement; it does not say "as before, except". Each node stays
  readable alone.
- Derived states, recomputed on every validation:
  - `current`: no node supersedes it and its latest review is not `0`.
  - `contested`: two current nodes joined by `disputes`, or two heads for one chain, or a latest review of `0`.
  - `stale`: a premise in `from`, or its `src` source node, has a newer head, and no review of this node is
    dated after that head.
  - `due`: observed from a web source whose latest confirming review is older than the chosen interval.
- Undo is a new node that supersedes the superseder and restates the earlier content (it will hash to a new
  id because its `supersedes` line differs); nothing is removed.

### 5.8 Keeping it honest: checks a machine can run

1. Shape: every non-blank line matches `^[a-z-]+: \S.*$`; a block starts with `n:`; keys are known and in
   canonical order; no tabs, no trailing spaces, LF only.
2. Identity: `n` equals the hash prefix of the block; all prefixes are unique.
3. Immutability: every block id present on the base branch is present, byte-identical, on the proposed
   branch. Additions and whole-block moves pass; an edit or a deletion fails.
4. References: every id in an edge exists; `supersedes` has no cycle; `re` targets exactly one node.
5. Status discipline: `observed` has `src`, and the source resolves — the file exists at the named commit and
   the line range is inside it, or the source node exists and has `url`, `at`; `inferred` has at least one
   `from`; `assumed` has `test` and no file or URL locator.
6. Confidence propagation: an inferred node's `conf` is no higher than the lowest `conf` among its premises;
   `assumed` is never `high`.
7. Rests-on-assumption: any node whose `from` closure contains an `assumed` node is listed as such.
8. Staleness and contest: the derived states of 5.7 are printed; `stale` and `contested` counts are the
   round's work list.
9. Reachability: every node reaches the root through `part`; every hub has members; a hub whose newest
   member is newer than the hub's latest head is listed as "summary behind".
10. Budget: files over budget are listed as "split due"; `is` over 280 characters fails.
11. Dates: ISO form; `at` is not in the future; `src-date` is not after `at`.
12. Privacy: `by` contains no `@` followed by a domain and matches the role-path pattern.
13. Coverage: the share of current nodes with a confirming review in the last N rounds, and the list of
    observed nodes never reviewed, are printed with every run.
14. Names: every id in a lens file exists; a name maps to at most one current head per lens.

Checks 1-6, 11, 12 and 14 are errors. Checks 7-10 and 13 are reports: they inform, they do not gate.

### 5.9 Merging

Blocks are only added. Two branches that both append to a file produce a textual conflict under git's default
driver. Git's built-in `union` driver "take[s] lines from both versions, instead of leaving conflict
markers", with the documented caveat that added lines may end up in arbitrary order
(https://git-scm.com/docs/gitattributes, observed). Because block order carries no meaning here and check 2
detects any block damaged by a merge, `*.kg merge=union` and `*.names merge=union` should make concurrent
additions merge without hand work. This is inferred and has not been tested; it is open question 2.

### 5.10 Later, in the one grammar

A node is already cell-shaped: a kind (blueprint), ordered children (the values), references to other cells
by content id, and an identity that is its present shape. The mapping to the axioms is direct: immutability
and supersession are axiom 3; the names lens is "names are free query keys"; a review is an offer answered by
nothing, 0, 1 or a node (axiom 5); an unreviewed node is not a refuted node (`absence-is-not-a-verdict`). When
the grammar exists, the same blocks can be read as cells and the validator's fourteen checks become recipes
over them; the text files then serve as the frozen, committed state of nodes whose working and in-flight
states live in the runtime. That last sentence is an assumption about how the ice, water and gas states will
be defined; I did not study them.

## 6. What I recommend against, and why

- **RDF or JSON-LD as the stored form.** Three to five times the tokens and lower scores on graph tasks
  (3.2); triple-term syntax still in draft (2.1). Keep a lossless export instead: one node becomes one named
  graph with PROV terms, so SHACL and SPARQL remain available.
- **TOON or CSV tables as the stored form.** They win only on uniform arrays; nodes are prose-carrying and
  heterogeneous; TOON is a moving draft (3.1).
- **JSONL.** Lowest tier of accuracy in the one lookup benchmark that included it (3.1), long lines diff
  poorly, and the project's own memory index records a direction away from it
  (`MEMORY.md` entry `project-jsonl-dissolution-layers-2026-11-20`; observed as an index line only).
- **Sequential numbers as identity** (ADR style). They collide between branches (2.11, 2.15).
- **A topical directory tree as identity.** Topics shift; Luhmann's fixed place "needs no system" (2.10).
- **A status field edited in place.** It overwrites what was referenced and makes merges conflict.
- **A numeric confidence.** Unsupported precision (2.17).
- **Calling the text store holographic in the vector sense.** It is not (2.13). The accurate claim is:
  self-similar, with a reduced description at every level.
- **A graph database or embedding store as the source of record.** Not diffable; writes are expensive; the
  2025-2026 evidence favours readable files with typed records (2.15). Both remain available as derived
  indexes.

## 7. Open questions

1. Token counts of the proposed block against JSON, YAML and TOON under the tokenizers that matter here (the
   rented model's and the local 27B voice's) were not measured; only bytes were.
2. Does `merge=union` behave as argued for blank-line-separated immutable blocks, including when two branches
   split the same file differently? Untested.
3. Should graph nodes later receive `package.level.type.instance` coordinates, and if so is the content number
   the instance, or a separate key into the numeric space?
4. Should the hash cover provenance (`at`, `by`) as proposed — making two independent observations of the same
   sentence two nodes that corroborate each other — or only kind and statement, making them one node with two
   provenance records?
5. Is 8 hex digits the right written length, and is hex or a denser base cheaper in tokens?
6. Can a local 27B model write these blocks reliably with `?label` placeholders, and read a 24-node file
   without loss? The format studies suggest yes for key-value lines but none tested this shape.
7. The interval after which a web-sourced observation is `due`, and the file budget, are first guesses.
8. Whether review nodes should be able to carry a narrower verdict than 0 or 1 (for example "statement right,
   locator wrong") or whether that is always a superseding node.
9. MeTTa's current documentation, the PeerJ nanopublication-network paper, ISO 39075 and Plate's 1995 journal
   paper could not be fetched; statements about them rest on secondary pages or memory.
10. How the ice, water and gas node states map onto committed text, derived index and in-context form.

## 8. Source list

Standards and specifications: https://www.w3.org/TR/rdf12-concepts/ ·
https://www.w3.org/groups/wg/rdf-star/publications/ · https://www.w3.org/TR/rdf12-n-quads/ ·
https://www.w3.org/TR/rdf-canon/ · https://www.w3.org/TR/prov-o/ · https://www.w3.org/TR/json-ld11/ ·
https://www.w3.org/TR/shacl/ · https://www.rfc-editor.org/rfc/rfc8785 (JSON Canonicalization Scheme, June
2020, Informational) · https://schema.org/Claim · https://ipld.io/docs/intro/primer/ ·
https://ipld.io/specs/advanced-data-layouts/hamt/spec/ · https://github.com/multiformats/cid ·
https://docs.ipfs.tech/concepts/merkle-dag/ · https://git-scm.com/book/en/v2/Git-Internals-Git-Objects ·
https://git-scm.com/docs/hash-function-transition · https://git-scm.com/docs/gitattributes ·
https://llmstxt.org/ · https://github.com/toon-format/toon · https://github.com/toon-format/spec ·
https://raw.githubusercontent.com/toon-format/spec/main/CHANGELOG.md

Papers: https://arxiv.org/abs/1401.5775 · https://www.ijcai.org/Proceedings/91-1/Papers/006.pdf ·
https://cs.uwaterloo.ca/~jhoey/teaching/cogsci600/papers/Kanerva09.pdf · https://arxiv.org/abs/cs/0412059 ·
https://arxiv.org/abs/2111.06077 · https://arxiv.org/abs/2504.07087 and https://arxiv.org/html/2504.07087v1 ·
https://arxiv.org/abs/2305.13062 · https://arxiv.org/abs/2411.10541 · https://arxiv.org/abs/2404.16130 ·
https://arxiv.org/abs/2501.13956 · https://arxiv.org/abs/2502.12110 · https://arxiv.org/abs/2608.13662 ·
https://arxiv.org/abs/2307.03172

Product and project documentation: https://nanopub.net/guidelines/working_draft/ ·
https://nanopub.readthedocs.io/en/latest/publishing/retraction.html ·
https://docs.datomic.com/whatis/data-model.html · https://docs.datomic.com/indexes/index-model.html ·
https://www.unison-lang.org/docs/the-big-idea/ · https://github.com/opencog/atomspace ·
https://github.com/trueagi-io/hyperon-experimental · https://adr.github.io/madr/ · https://c4model.com/ ·
https://arc42.org/overview · https://microsoft.github.io/graphrag/index/default_dataflow/ ·
https://github.com/getzep/graphiti · https://github.com/modelcontextprotocol/servers/tree/main/src/memory ·
https://github.com/steveyegge/beads · https://www.wikidata.org/wiki/Help:Ranking ·
https://www.wikidata.org/wiki/Wikidata:Identifiers · https://www.wikidata.org/wiki/Help:Label ·
https://www.wikidata.org/wiki/Help:Items

Articles and reports: https://cognitect.com/blog/2011/11/15/documenting-architecture-decisions ·
https://zettelkasten.de/introduction/ · https://zettelkasten.de/posts/luhmann-folgezettel-truth/ ·
https://luhmann.surge.sh/communicating-with-slip-boxes · https://www.dolthub.com/blog/2024-03-03-prolly-trees/ ·
https://www.improvingagents.com/blog/best-input-data-format-for-llms ·
https://www.improvingagents.com/blog/best-nested-data-format ·
https://www.letta.com/blog/benchmarking-ai-agent-memory ·
https://arize.com/blog/alyx-agent-long-term-memory-architecture/ ·
https://www.anthropic.com/engineering/effective-context-engineering-for-ai-agents ·
https://www.anthropic.com/engineering/equipping-agents-for-the-real-world-with-agent-skills ·
https://www.trychroma.com/research/context-rot

Encyclopedia pages (secondary): https://en.wikipedia.org/wiki/Graph_Query_Language ·
https://en.wikipedia.org/wiki/Datalog · https://en.wikipedia.org/wiki/Conceptual_graph ·
https://en.wikipedia.org/wiki/Common_Logic · https://en.wikipedia.org/wiki/Issue-based_information_system ·
https://en.wikipedia.org/wiki/Toulmin_method · https://en.wikipedia.org/wiki/Hyperdimensional_computing ·
https://en.wikipedia.org/wiki/Holon_(philosophy) · https://en.wikipedia.org/wiki/Fractal_tree_index ·
https://en.wikipedia.org/wiki/Admiralty_code

Local: `coherence-kernel/axioms/core-axioms.form` ·
`NUMS.Go/nums/nums_nodes.go` · `NUMS.Go/nums/nums_symbols.go` ·
memory notes under `memory/` ·
probe files under the scratchpad `study/kgprobe/` directory named in 5.2.
