# Contributing

Keep changes focused on native document transaction integrity. New modeling
features and UI frameworks belong in separate projects.

For a bug report or patch, include:

- FreeCAD/OCCT versions, platform and a minimal reproducible command.
- Expected and observed object set, body tip, geometry and undo behavior.
- A regression that fails before the fix and passes after it.
- Results from `python3 scripts/check.py --freecad /path/to/FreeCADCmd`.

Do not count a mocked document as proof of native rollback. Fault injection is
appropriate for error paths, provided the injection boundary is identified.
Keep tests headless and use disposable documents. Never contribute private CAD
models, personal paths, connection tokens, generated checkpoints or unlicensed
third-party code. Preserve LGPL-2.1-or-later notices.

Do not add a poison-state reset without an independently verified recovery
protocol. Do not solve a rollback failure by blindly undoing an older command.
Document compatibility changes and callback preconditions explicitly.
