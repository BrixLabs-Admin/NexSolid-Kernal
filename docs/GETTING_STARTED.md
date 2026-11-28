# Getting Started

Build the NexSolid C++17 OCCT-backed project, run its CLI demo and execute all four registered CTest suites. The project is early-stage; these focused tests are not broad certification of a CAD geometry engine.

## Build environment

GitHub Actions currently targets **`ubuntu-latest`** with CMake, a C++17 compiler and OCCT development packages. macOS and Windows are not currently covered by CI. The exact Ubuntu image can change; see [the workflow](../.github/workflows/ci.yml) for the current configuration.

Install dependencies on a supported Ubuntu environment:

```bash
sudo apt update
sudo apt install -y build-essential cmake git \
  libocct-foundation-dev libocct-modeling-data-dev \
  libocct-modeling-algorithms-dev libocct-data-exchange-dev
```

OCCT's STEP libraries are required for the new STEP tests. On newer OCCT builds the STEP toolkit is named `TKDESTEP` rather than `TKSTEP`; CMake searches for both. Some distributions do not provide `OpenCASCADEConfig.cmake`, so NexSolid can also find the libraries directly.

## Get the source and run everything

```bash
git clone https://github.com/BrixLabs-Admin/NexSolid-Kernal.git
cd NexSolid-Kernal
bash scripts/build-test.sh
```

This performs configuration, compilation, CTest and the `nexsolid_demo` CLI. A failing build/test stops the script with a nonzero exit code.

To build and test manually:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/nexsolid_demo
```

A Debug build:

```bash
NEXSOLID_BUILD_TYPE=Debug bash scripts/build-test.sh
```

## What gets tested?

| CTest case | What it covers |
| --- | --- |
| `kernel_smoke` | Basic primitive box, validity, volume and zero-dimension rejection |
| `step_io` | Drilled-box and compound STEP round-trips, volume/face/bounds invariants, missing/corrupt/empty inputs |
| `diagnostics` | Unique topology counts, tolerance ranges, safe copy repair of valid shapes and invalid repair-parameter rejection |
| `properties` | Analytical mass properties, geometric indexing consistency and ambiguous-key reporting |

Run one test group:

```bash
ctest --test-dir build --output-on-failure -R '^step_io$'
```

Read [Progress & evidence](PROGRESS_EVIDENCE.md) for precise before-and-after capability examples and code links. Note that the drilled-box Boolean is produced by OCCT for the test fixture; the test does not prove a new NexSolid Boolean algorithm or recovery of arbitrary invalid geometry.

## Testing separately built, modified OCCT

**NexSolid currently depends on OCCT and does not embed its complete source tree.** To test OCCT source patches, install a patched OCCT build to a dedicated prefix and point NexSolid to it:

```bash
NEXSOLID_BUILD_DIR="$PWD/build-patched" \
NEXSOLID_OCCT_ROOT=/absolute/path/to/patched-occt-install \
bash scripts/build-test.sh
```

When `NEXSOLID_OCCT_ROOT` is set, the build searches only inside that prefix. Check the `NexSolid OCCT headers` and `NexSolid OCCT toolkits` output to verify the dependency paths. Use a new build directory for each OCCT installation because CMake caches library selections.

## Troubleshooting

- **Missing OCCT components:** Install all four development packages listed above. STEP tests require Data Exchange.
- **Wrong OCCT version selected:** Use a fresh build directory and `NEXSOLID_OCCT_ROOT`, and inspect CMake's resolved dependency paths.
- **Compiler/linker errors:** Record OS, compiler, CMake and OCCT version, relevant terminal output and the exact toolkit found.
- **STEP test fails:** Keep the error output and the failing test name; do not assume this demonstrates an algorithm improvement or regression without a controlled comparison.
- **macOS/Windows:** Not verified in current CI. On multi-configuration Windows generators, the demo may be under `build/Release/nexsolid_demo.exe` and requires corresponding OCCT runtime libraries on the search path.

## Want to contribute?

Read [the first contribution guide](FIRST_CONTRIBUTION.md), [CONTRIBUTING](../CONTRIBUTING.md), and browse [issues](https://github.com/BrixLabs-Admin/NexSolid-Kernal/issues). AI development help is optional; see [AI contributor guidance](AI_CONTRIBUTIONS.md).
