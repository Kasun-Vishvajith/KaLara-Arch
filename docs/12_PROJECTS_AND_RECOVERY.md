# Project lifecycle, persistence, and crash recovery

## Native format

Final .kla is a ZIP container, not a DXF or screenshot:

- manifest.json: format ID, schema version, project ID, app version, entry hashes.
- project.json: authoritative semantic state.
- assets/: content-hashed custom SVG/raster assets and embedded library definitions.
- preview.png: optional nonauthoritative thumbnail.

Early steps may use versioned project JSON internally, but final native save must package all required local assets. JSON interchange remains a separate explicit export. Distinguish archived .kla from any legacy plain-JSON .kla through header detection and a migration path.

## Save algorithm

Capture committed snapshot R; serialize and package off the UI thread. Write to a sibling temporary file, flush, validate container entries/checksums, then atomically replace target using the platform adapter. On Qt platform layer, use QSaveFile without enabling unsafe direct-write fallback. Only a successful final commit updates the saved revision marker.

If state advances during save, R is saved but newer document stays dirty. If disk full, permission denied, path invalid, or commit fails, retain prior target, report failure, keep dirty state, and offer Save As. Never announce saved before final replacement.

Use UTF-8 metadata and path-safe archive entries. Document-scoped asset hashes avoid absolute-path dependence. Custom assets must work on another machine.

## Opening

Read into a candidate document, inspect archive limits, parse/validate schema, migrate supported old schema into a new candidate, resolve references/assets, then activate only after success. Existing open project remains available on failure. Unsupported major versions must not be overwritten through the current writer.

Reject archive traversal/absolute paths, duplicate entries, unsafe symlinks, invalid content types, and decompression bombs. Defaults: max 10,000 entries, 512 MiB total expanded assets, 64 MiB project JSON, and 100× compression ratio warning/rejection policy for suspicious input. Make exceptions explicit via a deliberate import setting, not an automatic retry without limits.

## Autosave and recovery

Default every 60 seconds when dirty, coalesced and from committed snapshot only. Use per-project ID plus session ID under writable user application data, never a single shared TEMP filename. Retain the latest five valid generations, write generation atomically, prune after successful replacement, and preserve the latest valid generation if newer write fails.

Recovery metadata includes original path, base saved revision/hash, generation revision, timestamp, and application version. On abnormal shutdown, recovery chooser offers preview/time/original file information, Open Recovered Copy, Open Original, and Discard Recovery. Do not overwrite original automatically. Saving recovered copy establishes a new known-safe target.

Handle two app instances: lock/advisory ownership for the same file, with read-only/open-copy choices; do not rely only on basename. A second document cannot erase another's recovery records.

## Templates and initialization

New project: Empty or Template. Empty needs only a title/default floor and display units. Template wizard: project type, optional site dimensions/shape, floor count, target area, room requirements. Each field has known/undecided/omitted distinction. Missing answers never become zero or invented design requirements.

Templates are versioned, shipped local, ordinary semantic geometry. Instantiate through a single validated command with fresh IDs. Starter templates: empty plan, rectangular room, small residential arrangement, site planning sheet. Preview and name chosen template; label examples as editable starts rather than approved architectural designs.

## Settings

User settings: theme, panels, shortcuts, recent paths, performance preference. Project settings: units display, precision, dimension/line styles, default drafting sizes, configured checks, named views. Don't autosave user machine paths into portable project metadata.

Recent files show unavailable status and Remove from Recent, without deleting the document.

## Tests

Round-trip semantic equivalence; non-ASCII filename; custom asset portability; truncation; unsupported version; missing host; disk-full fault injection; old file preserved on failed save; save during newer edit; crash during generation write; two instances; corrupt newest generation falls back to prior valid recovery; restored copy does not overwrite original.
