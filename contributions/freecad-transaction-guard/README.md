# FreeCAD Transaction Guard

**One fix: prevent a failed application edit from surviving because a helper
committed its native transaction too early.**

This small Python package gives a cooperative FreeCAD operation exactly one
transaction owner. It checkpoints the document, executes the edit, recomputes,
checks geometry, runs a read-only application postcondition, then commits.
A failure before commit aborts the owned transaction. If ownership or rollback
is uncertain, it preserves the checkpoint and refuses further guarded edits.

This is a FreeCAD application-level transaction integrity fix and regression
kit. It is not a patch to OCCT's boolean/fillet algorithms or a claim of universal
rollback, topological naming, crash recovery, or commercial-kernel parity.

## The reproducible bug

The original application used this transaction structure:

```text
outer editor: open transaction
  geometry helper: open transaction, create fillet, commit transaction
outer editor: validate result -> raises an exception
outer editor: abort transaction
```

The helper committed before the editor's postcondition passed. A native fillet
could remain in the document even though the UI reported failure.

`tests/native_suite.py` includes that broken pattern as a **negative control**:
it must actually reproduce the leak on the test runtime. The corrected helper
does no transaction management. The guard owns open/commit/abort around the entire
edit. The same late failure then restores the object set, `Body.Tip`, volume and
undo count. Successful fillets and chamfers each have one native undo/redo step.

## Minimal use

Run in FreeCAD's Python environment on its main thread. Importing the package
itself does not load FreeCAD or launch any application.

```python
from pathlib import Path
import FreeCAD as App
from freecad_transaction_guard import Guard

doc = App.newDocument("GuardExample")
doc.UndoMode = 1
box = doc.addObject("Part::Box", "Box")
doc.recompute()

guard = Guard(App, Path.home() / ".freecad-transaction-guard" / "checkpoints")

def edit(document):
    document.getObject("Box").Length = 25

def require_one_solid(document):
    shape = document.getObject("Box").Shape
    if len(shape.Solids) != 1:
        raise ValueError("Expected exactly one solid")

guard.run(doc, "Resize box", edit, validate=require_one_solid)
```

The callback result is returned after a successful commit. Ordinary operation
exceptions, including `KeyboardInterrupt` and `SystemExit`, are re-raised after
successful rollback. `RecoveryRequired` exposes `checkpoint`, `primary` and
`rollback` attributes when native state cannot be trusted. Restart the process
and inspect/open the checkpoint explicitly; there is no reset that silently
declares an uncertain document safe.

For a source checkout, add its `src` directory to FreeCAD's `sys.path`. A Python
wheel can be built with standard setuptools tooling, but FreeCAD is a separately
installed native dependency—not a package this project downloads or bundles.

## Callback contract and limits

- The supplied document must be active, with `UndoMode=1`. The application and
  all open documents must have no existing native transaction. An automatic GUI
  command transaction is also an existing owner; the guard refuses to nest in it.
- The operation may edit **only that document**. It must not open, commit or
  abort transactions, close documents, switch active documents or mutate from
  another thread. This is a cooperative API, not a sandbox for arbitrary scripts.
- The optional validator is **read-only**. It runs after recompute and default
  checks. Those checks reject native invalid feature states, invalid nonempty
  shapes and nonfinite/nonpositive solid volumes; they are not an exhaustive
  self-intersection or design-intent verifier.
- Native transaction IDs are checked before commit/abort. Ownership violations
  may already have changed the document. They raise `RecoveryRequired`; the
  guard does not falsely report that those changes were rolled back.
- The guard cannot undo external file/network effects, unrelated-document edits,
  fatal native crashes or process termination. Checkpoints are recovery artifacts,
  not a guarantee against every storage failure.
- Checkpoints are retained on success and failure. Choose a writable persistent
  directory, manage retention explicitly, and do not upload user checkpoints.
- The process-wide busy/poison state covers guards using this imported module;
  it cannot govern arbitrary native calls elsewhere in an application.

## Native verification

Prerequisites: Python 3.10+ for the runner and a compatible native FreeCADCmd.
Verified here with **FreeCAD 1.1.4 and OCCT 7.8.1** on Apple Silicon macOS.
Other versions/platforms are unverified. Required APIs include
`App.getActiveTransaction()`, `App.closeActiveTransaction()`,
`Document.HasPendingTransaction` and `Document.saveCopy()`.

From this checkout:

```sh
python3 scripts/check.py --freecad /path/to/FreeCADCmd
```

For the standard macOS installation:

```sh
python3 scripts/check.py \
  --freecad /Applications/FreeCAD.app/Contents/Resources/bin/freecadcmd
```

The runner creates disposable documents and temporary checkpoints, imports no
`FreeCADGui`, sets `QT_QPA_PLATFORM=offscreen`, and checks an explicit JSON result.
FreeCADCmd can exit zero after a script error, so exit status alone is insufficient.
Output goes to ignored `.build/`; the uploadable report is in `evidence/`.

The native suite covers:

1. The original nested-helper early-commit leak (negative control).
2. Fillet and chamfer rollback after a late application failure.
3. `Body.Tip`, object names, volume, native base links and one-step undo/redo.
4. Checkpoint reopening and unchanged original FCStd bytes/save target.
5. Refusal of a foreign lazy transaction before its first edit.
6. Rollback for `KeyboardInterrupt` and `SystemExit`.
7. Refusal of nested guard instances and worker-thread entry.
8. Checkpoint I/O failure before callback execution.
9. No-op closure without an undo entry.
10. Detected ownership loss and process-wide refusal of subsequent guarded edits.
11. Injected abort failure with real native geometry: both errors and the
    checkpoint survive; a newly created guard is also blocked.

Tests use real FreeCAD documents and OCCT geometry. The final abort-failure test
injects only the abort call's failure, not a simulated CAD kernel. Evidence is
fixture-specific. No performance comparison or general validity claim is made.

## Upload and contribution

Upload this directory's source files, license files and sanitized `evidence/`.
Do not upload `.build/`, caches or generated FCStd files. A source-only ZIP is
provided alongside this checkout. This standalone package does not change the host repository’s C++ kernel build.

The code is **LGPL-2.1-or-later**, preserving the original wrapper's licensing.
See `NOTICE`, `COPYING.LESSER` and `COPYING`. See `CONTRIBUTING.md` for review and
reproduction requirements.
