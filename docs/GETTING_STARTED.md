# Getting Started

This guide builds the NexSolid C++ starter, runs its CLI demo, and checks its automated tests.

## Verified build environment

- Ubuntu **22.04**, CMake 3.20+, and a C++17 compiler
- OCCT **Foundation**, **Modeling Data** and **Modeling Algorithms** development libraries
- Git and a shell (Bash for the one-command helper)

The GitHub Actions build uses Ubuntu 22.04 and its OCCT development packages. macOS and Windows are not yet continuously tested.

### Ubuntu dependencies

```sh
sudo apt update
sudo apt install -y build-essential cmake git \
  libocct-foundation-dev libocct-modeling-data-dev libocct-modeling-algorithms-dev
```

The distribution's OCCT libraries may have no `OpenCASCADEConfig.cmake` file. NexSolid supports discovering the installed headers and toolkits directly.

## One command to build, test and run

From the repository root:

```sh
bash scripts/build-test.sh
```

This configures the project, compiles it, runs CTest and launches `nexsolid_demo`. A test failure causes the script to stop with a nonzero exit code.

To debug a change:

```sh
NEXSOLID_BUILD_TYPE=Debug bash scripts/build-test.sh
```

To run each stage separately:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/nexsolid_demo
```

The demo creates a 10 × 20 × 30 box through OCCT and prints its volume. The smoke test verifies shape validity, volume 6000, and rejection of one invalid dimension. **These tests do not yet cover Booleans, fillets, advanced topology, or the internals of modified OCCT**. Expanding CLI regression testing is tracked in [issue #19](https://github.com/BrixLabs-Admin/NexSolid-Kernal/issues/19).

## Testing a modified OCCT engine

**Important distinction:** NexSolid currently contains a small OCCT-backed wrapper and tests, **not** the entire OCCT source tree. Editing `src/Kernel.cpp` changes NexSolid's wrapper. To alter OCCT's own algorithms, obtain the full OCCT source separately, patch it, build it, and install the patched libraries to a dedicated directory.

To test the installed patched libraries, run:

```sh
NEXSOLID_OCCT_ROOT=/absolute/path/to/patched-occt-install bash scripts/build-test.sh
```

NexSolid will search for OCCT headers and toolkits **only inside that prefix**, rather than silently using system OCCT. Check the `NexSolid OCCT headers` and `NexSolid OCCT toolkits` lines during configuration to confirm the paths. **Use a fresh build directory when changing OCCT installations**, because CMake caches library paths:

```sh
NEXSOLID_BUILD_DIR="$PWD/build-patched" \
NEXSOLID_OCCT_ROOT=/absolute/path/to/patched-occt-install \
bash scripts/build-test.sh
```

Add a reproducible test for the behavior you changed. Passing the current box test alone does not demonstrate that an OCCT algorithm bug is fixed.

## Troubleshooting

- **OCCT headers/toolkits missing:** install the three development packages above or pass `NEXSOLID_OCCT_ROOT` to a complete installation.
- **Unexpected OCCT installation used:** review the CMake configuration output; use a fresh build directory and `NEXSOLID_OCCT_ROOT` for isolated patched builds.
- **Compilation or linker failure:** include the compiler error, platform, OCCT version, and the two printed OCCT paths in your issue.
- **Windows:** the demo commonly appears under `build\\Release\\nexsolid_demo.exe` with multi-configuration generators. Native Windows CI is not yet available.
- **CLI output not as expected:** run `ctest --test-dir build --output-on-failure` and include the failing test output in your bug report.
