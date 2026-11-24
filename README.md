# NexSolid Kernel

![NexSolid Kernel — From open source to world-class CAD geometry](assets/nexsolid-banner.svg)


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
bash scripts/build-test.sh
```

The script configures CMake, compiles NexSolid, runs CTest and launches the CLI demo. For individual commands, Debug builds and isolated testing of a patched OCCT installation, see [Getting Started](docs/GETTING_STARTED.md). The current smoke test only checks basic box geometry; more extensive regression testing is tracked in [issue #19](https://github.com/BrixLabs-Admin/NexSolid-Kernal/issues/19).

See [Getting Started](docs/GETTING_STARTED.md) for platform notes and troubleshooting.

## Project layout

- `include/nexsolid/` — public starter headers
- `src/` — kernel implementation
- `app/` — small runnable example
- `tests/` — automated tests
- `docs/` — setup and dependency notes
- `ROADMAP.md` — staged project direction

## Contributing

New contributors are welcome. Start with [good first issues](https://github.com/BrixLabs-Admin/NexSolid-Kernal/issues?q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22), read [CONTRIBUTING.md](CONTRIBUTING.md), and follow [Getting Started](docs/GETTING_STARTED.md) and the [first contribution guide](docs/FIRST_CONTRIBUTION.md). Optional AI-assisted development guidance is available in [Using AI assistants](docs/AI_CONTRIBUTIONS.md).

Please report bugs through the bug report form and propose larger changes through an issue before opening a pull request.

## Roadmap

See [ROADMAP.md](ROADMAP.md). The first goal is to establish a reliable build and test baseline before adding larger geometry features.

## Dependencies

NexSolid Kernel uses OCCT APIs and does not bundle OCCT. See [Dependency and third-party notices](docs/DEPENDENCIES.md) before redistributing builds.

## License

Original NexSolid Kernel starter code is licensed under **LGPL 2.1 with the Open CASCADE additional exception**, except where file-specific notices say otherwise. Read [LICENSE](LICENSE), [the LGPL license text](LICENSE_LGPL_21.txt), and [the OCCT exception](OCCT_LGPL_EXCEPTION.txt). OCCT-derived source must preserve its original copyright/license notices, and third-party components may have separate terms. See [Dependency and third-party notices](docs/DEPENDENCIES.md) and [Licensing plan](docs/LICENSING_PLAN.md).