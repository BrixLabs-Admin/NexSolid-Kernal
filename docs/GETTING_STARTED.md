# Getting Started

This guide builds the starter application and runs its smoke test.

## Requirements

- CMake 3.20 or newer.
- A C++17 compiler.
- Open CASCADE Technology (OCCT) development files for Foundation, Modeling Data, and Modeling Algorithms.
- Git.

The continuous integration workflow currently uses Ubuntu 24.04 packages. Other distributions and OCCT versions may need different package names or a `CMAKE_PREFIX_PATH`.

## Install dependencies

On Ubuntu 24.04:

```sh
sudo apt update
sudo apt install build-essential cmake git \
  libocct-foundation-dev libocct-modeling-data-dev libocct-modeling-algorithms-dev
```

If your OCCT installation is outside the system prefix, point CMake to the directory containing `OpenCASCADEConfig.cmake`:

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/occt/lib/cmake/opencascade
```

Adjust that path to match your installation.

## Build and test

From the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/nexsolid_demo
```

On Windows, the executable is usually `build\Release\nexsolid_demo.exe` when using a multi-configuration generator. Set `CMAKE_PREFIX_PATH` to your OCCT installation if CMake cannot find it.

## What the starter does

The demo creates a 10 × 20 × 30 box through OCCT and prints the resulting volume. The test checks that the shape is non-null, valid, and has volume 6000 within a small floating-point tolerance.

## Troubleshooting

### CMake cannot find OpenCASCADE

Install the OCCT development packages, then re-run CMake. If OCCT is in a custom location, set `CMAKE_PREFIX_PATH` to the directory that contains its package configuration file.

### The linker cannot find an OCCT toolkit

Confirm that your OCCT development installation contains the Foundation, Modeling Data, and Modeling Algorithms libraries, and that those libraries match the architecture and compiler used for this project.

### The demo runs but prints no expected volume

Run the test executable through CTest to see the assertion message, and include your platform, compiler, CMake version, and OCCT version in a bug report.
