# Progress and reproducible evidence

**Updated: 2026-11-28 · NexSolid Kernel (CMake source version 1.1.0)**

This page documents what changed between the original small OCCT-backed box-demo starter and the current source. It provides source links and tests that contributors can reproduce.

**Important interpretation:** The earlier NexSolid starter lacked the features below. That is **not** proof that an earlier version of OCCT failed them or that NexSolid fixed OCCT's algorithms. An intentionally failing input is a test of error handling, not a formerly failing algorithm now repaired. The repository uses OCCT as an external dependency; the advanced capabilities here are utilities built around it.

## At a glance

| Example | Earlier NexSolid starter | Current source and test expectation | Evidence |
| --- | --- | --- | --- |
| STEP round-trip for a drilled box | No `readStep` / `writeStep` NexSolid API | Write/read a box cut by a cylindrical hole; valid result retains **7 faces**, **volume 1000 − 40π**, and bounds within test tolerances. | [STEP test](../tests/test_step_io.cpp), [integration commit](https://github.com/BrixLabs-Admin/NexSolid-Kernal/commit/c7ba1170c5e67cc7074078031703e3c98c0d83e0) |
| Multiple solids in STEP | No NexSolid STEP import/export | A two-solid compound round-trip retains **13 faces** and total volume of the drilled box plus **24**. | [STEP test](../tests/test_step_io.cpp) |
| Missing, corrupt or empty STEP input | No dedicated NexSolid file-I/O error handling | `readStep` **throws a STEP-related error** for missing, corrupt, empty and non-regular-file inputs; `writeStep` rejects null geometry. These are **correctly rejected failures**. | [STEP negative tests](../tests/test_step_io.cpp) |
| Shape report | Only `isValid` existed in the starter | A 10×20×30 box reports **1 solid, 1 shell, 6 faces, 12 edges, 8 vertices** with tolerance ranges. A cut-box fixture is expected to have **7 faces, 15 edges, 10 vertices**. | [Diagnostics test](../tests/test_diagnostics.cpp), [integration commit](https://github.com/BrixLabs-Admin/NexSolid-Kernal/commit/b2507ca344c57414cb8ffcd970fb99fe0629c296) |
| Safe repair call | No `repairShape` API | For valid input fixtures, returns a separate shape copy with valid geometry and preserved volume. Invalid repair tolerances fail explicitly. **No general recovery guarantee for damaged geometry.** | [Diagnostics test](../tests/test_diagnostics.cpp) |
| Analytical properties | Only basic `volume` available | A 10×20×30 box is checked for **volume 6000**, **area 2200**, **unique edge length 240**, **center of mass (5,10,15)** and unit-density principal moments **(250000, 500000, 650000)**. | [Properties test](../tests/test_properties.cpp), [integration commit](https://github.com/BrixLabs-Admin/NexSolid-Kernal/commit/5dae3513678a3731aecd64fc022930a6071f9e05) |
| Reproducible geometric indexing | No `indexTopology` API | Repeatedly building a box and reversing the insertion order of two disjoint boxes must produce matching ordered geometric keys. Overlapping geometrically identical keys are flagged `hasAmbiguousKeys`. **Not persistent topological naming.** | [Properties test](../tests/test_properties.cpp) |

## How to reproduce

In the repository root, install the documented OCCT development dependencies (including `libocct-data-exchange-dev`) and run:

```bash
bash scripts/build-test.sh
```

Alternatively, use CMake and run individual CTest cases:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
ctest --test-dir build --output-on-failure -R 'step_io|diagnostics|properties'
```

The four registered cases are `kernel_smoke`, `step_io`, `diagnostics` and `properties` ([CMake configuration](../CMakeLists.txt)). The scripts and cases are committed; **this document does not claim a fresh passing GitHub Actions run was independently observed on this update date**. Check the [live CI page](https://github.com/BrixLabs-Admin/NexSolid-Kernal/actions).

## What remains open

- Historical *failed* outputs and a matching *fixed* regression must be linked before describing any geometry problem as a proven bug fix. A reproducible test fixture and before/after commit results are required.
- Boolean cut is used to create **test fixtures through OCCT**. This does not yet prove NexSolid offers an independently improved or fully hardened Boolean algorithm.
- Shape repair is bounded and may fail on malformed B-reps; tests currently focus on validated inputs, copy semantics and tolerance rejection.
- A geometric ordering key is not a stable face identity after edits, Booleans or fillets.
- Claims of numerical precision, speed or superiority require representative benchmark datasets, stated tolerances and recorded baselines.

## Contribution acknowledgement

**ChatGPT Star (AI assistant)** has assisted the owner with NexSolid development and contributor-facing documentation through the owner's ChatGPT session. Repository commits performed using the **BrixLabs-Admin account** are credited on GitHub to that account, not to a separate human contributor named ChatGPT Star. All implementation and documentation remains subject to maintainers' review and validation. Other contributors and AI tools are welcome.

Start at [CONTRIBUTING.md](../CONTRIBUTING.md) or the [first contribution guide](FIRST_CONTRIBUTION.md).
