# NexSolid Kernel

NexSolid Kernel is an early-stage C++ project for exploring a contributor-friendly CAD geometry kernel built on Open CASCADE Technology (OCCT).

> **Status:** starter repository. The project currently contains a minimal OCCT-backed box example and a smoke test. It is not yet a production CAD kernel.

## What this project is for

- A clear, buildable home for NexSolid Kernel development.
- Small, reviewable contributions around geometry operations, validation, and developer tooling.
- A documented foundation for future CAD kernel work.

This repository covers NexSolid Kernel only.

## Quick start

Requirements: CMake 3.20+, a C++17 compiler, and an OCCT development installation that includes Foundation, Modeling Data, and Modeling Algorithms.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/nexsolid_demo
```

See [Getting Started](docs/GETTING_STARTED.md) for platform notes and troubleshooting.

## Project layout

- `include/nexsolid/` — public starter headers
- `src/` — kernel implementation
- `app/` — small runnable example
- `tests/` — automated tests
- `docs/` — setup and dependency notes
- `ROADMAP.md` — staged project direction

## Contributing

New contributors are welcome. Start with [good first issues](https://github.com/BrixLabs-Admin/NexSolid-Kernal/issues?q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22), read [CONTRIBUTING.md](CONTRIBUTING.md), and follow [Getting Started](docs/GETTING_STARTED.md).

Please report bugs through the bug report form and propose larger changes through an issue before opening a pull request.

## Roadmap

See [ROADMAP.md](ROADMAP.md). The first goal is to establish a reliable build and test baseline before adding larger geometry features.

## Dependencies

NexSolid Kernel uses OCCT APIs and does not bundle OCCT. See [Dependency and third-party notices](docs/DEPENDENCIES.md) before redistributing builds.

## License

**The project's license has not been selected yet.** No license file is present. Until the project owner adds one, the source remains under default copyright and reuse permissions are not granted. Please do not redistribute or incorporate this project's code into other projects yet.

OCCT is a separate dependency with its own license terms; see [Dependency and third-party notices](docs/DEPENDENCIES.md).