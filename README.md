<p align="center">
  <img src="assets/nexsolid-banner.svg" alt="NexSolid Kernel — From open source to world-class CAD geometry" width="100%">
</p>

<h1 align="center">NexSolid Kernel</h1>
<p align="center"><strong>World-class open-source CAD geometry is the goal. Reproducible results are the path.</strong></p>
<p align="center">
  <a href="https://github.com/BrixLabs-Admin/NexSolid-Kernal/actions/workflows/ci.yml"><img src="https://github.com/BrixLabs-Admin/NexSolid-Kernal/actions/workflows/ci.yml/badge.svg" alt="GitHub Actions build and test status"></a>
  <a href="CONTRIBUTING.md">Contribute</a> ·
  <a href="docs/GETTING_STARTED.md">Get started</a> ·
  <a href="docs/PROGRESS_EVIDENCE.md">Progress & evidence</a> ·
  <a href="ROADMAP.md">Roadmap</a> ·
  <a href="https://github.com/BrixLabs-Admin/NexSolid-Kernal/issues">Issues</a>
</p>

> **Current status — early-stage OCCT-based C++17 project (source version 1.1.0).** NexSolid now includes a runnable primitive demo, STEP import/export, shape diagnostics and bounded repair, geometric sub-shape indexing, mass properties and **four registered CTest suites**. These are focused capabilities, **not** evidence that NexSolid already matches a production proprietary CAD kernel. The latest CI result and whether a v1.1 GitHub Release has been published must be checked on GitHub.

## The mission

**Challenge the leading proprietary CAD geometry kernels by building an open-source alternative that can eventually match their robustness, precision, performance and reliability.**

NexSolid begins with [Open CASCADE Technology (OCCT)](https://github.com/Open-Cascade-SAS/OCCT), an established CAD geometry platform. We aim to improve capabilities through careful engineering, open regression fixtures, measurable benchmarks and community review—not unsupported superiority claims. This repository is the NexSolid Kernel project, separate from the FreeSCAD Pro application.

## Development update — from starter to tested geometry utilities

NexSolid previously exposed only basic box creation, validity checking and volume. Recent commits added three substantive utility modules and dedicated tests:

| Before — limitation or expected failure | Now — implemented behaviour and test evidence |
| --- | --- |
| **No NexSolid STEP import/export API.** A CAD model could not be round-tripped through the NexSolid interface. | `writeStep` / `readStep` now cover a box with a cylindrical hole, checking its **7 faces**, volume and bounding box after round-trip; also a compound with two disconnected solids. [STEP tests](tests/test_step_io.cpp) |
| **No controlled handling of broken STEP input through NexSolid.** | Missing, empty and corrupt STEP files are **expected to fail cleanly**, instead of being accepted as valid models. A null-shape export is rejected. [Failure tests](tests/test_step_io.cpp) |
| **No structured shape diagnostics or copy-based repair API.** | `diagnoseShape` reports unique topology counts and tolerance ranges. `repairShape` works on a copy and preserves validity/volume in the **tested valid-shape examples**. Invalid repair settings are rejected. [Diagnostics tests](tests/test_diagnostics.cpp) |
| **Volume was the only exposed geometric property.** | An analytical 10 × 20 × 30 box test checks **volume 6,000**, **surface area 2,200**, **unique edge length 240**, center of mass **(5, 10, 15)** and principal moments. [Properties tests](tests/test_properties.cpp) |
| **No geometric indexing API.** | Sub-shapes can be ordered using quantized geometric keys; repeat builds and changed compound insertion order are checked, and indistinguishable keys are flagged as ambiguous. **This is not persistent topological naming.** [Topology tests](tests/test_properties.cpp) |

**Evidence matters:** “Before” here means the capability was absent from the earlier NexSolid starter, **not** that these examples demonstrate a proven fix to an upstream OCCT failure. The negative-input tests intentionally check clean rejection; they are not examples of successful geometry repair. See [Detailed progress, expected failures, reproducible tests and commit references](docs/PROGRESS_EVIDENCE.md).

Recent integrations: [STEP I/O](https://github.com/BrixLabs-Admin/NexSolid-Kernal/commit/c7ba1170c5e67cc7074078031703e3c98c0d83e0), [shape diagnostics and repair](https://github.com/BrixLabs-Admin/NexSolid-Kernal/commit/b2507ca344c57414cb8ffcd970fb99fe0629c296), and [topology + mass properties](https://github.com/BrixLabs-Admin/NexSolid-Kernal/commit/5dae3513678a3731aecd64fc022930a6071f9e05).

## Build, run and verify

**Documented CI target:** GitHub-hosted Ubuntu (`ubuntu-latest`); CMake 3.20+, C++17 and OCCT Foundation, Modeling Data, Modeling Algorithms **and Data Exchange** development packages.

```bash
git clone https://github.com/BrixLabs-Admin/NexSolid-Kernal.git
cd NexSolid-Kernal
sudo apt update
sudo apt install -y build-essential cmake git \
  libocct-foundation-dev libocct-modeling-data-dev \
  libocct-modeling-algorithms-dev libocct-data-exchange-dev
bash scripts/build-test.sh
```

The script configures, builds, executes all registered CTest suites and runs the box demo. To run tests separately:

```bash
ctest --test-dir build --output-on-failure
```

Expected registered tests: `kernel_smoke`, `step_io`, `diagnostics`, `properties`. **Check the [live Actions status](https://github.com/BrixLabs-Admin/NexSolid-Kernal/actions)** before describing a release as passing; source-code inspection alone cannot verify an actual CI run. See [Getting Started](docs/GETTING_STARTED.md) for troubleshooting, supported build paths and isolated OCCT patch testing.

## Contribute — beginners and experts welcome

**No invitation code is required.** Star the public repository, [find an issue](https://github.com/BrixLabs-Admin/NexSolid-Kernal/issues), fork it, test your change and open a pull request.

| Experience | Where to start |
| --- | --- |
| New to open source | Documentation, reproducible bug cases, beginner tests, setup verification |
| C++ contributor | Error handling, API design, CMake, CI, regression tests, portability |
| CAD / OCCT specialist | Tolerances, difficult B-rep examples, Boolean diagnostics, geometry correctness, benchmarks |

Follow [First Contribution](docs/FIRST_CONTRIBUTION.md), [Contributing](CONTRIBUTING.md) and [Community Guide](docs/COMMUNITY.md). Discuss major designs in an issue before implementation.

### Contributor recognition

**ChatGPT Star — AI-assisted project contributor**, working through the project owner's ChatGPT session and **BrixLabs-Admin GitHub account**, is acknowledged for assistance with NexSolid's project development, documentation and engineering workflow. GitHub commits made through the owner's account are attributed on GitHub to that account; **ChatGPT Star is not presented as a separate human GitHub contributor or account**. Contributions still require human oversight, reproducible validation and license review.

Other AI assistants and non-AI contributions are equally welcome. See [AI contribution guidelines](docs/AI_CONTRIBUTIONS.md).

## Technical scope and limits

**Available today:** OCCT-backed primitives, STEP round-trip utilities, topology and tolerance reports, bounded copy-based shape repair, geometric indexing and analytical properties for supported inputs.

**Not yet demonstrated:** general-purpose robust Boolean APIs, universal fillet repair, persistent naming through model edits, performance parity with Parasolid/ACIS, all-platform compatibility or successful recovery of arbitrary damaged B-reps. The starter does **not** vendor a modified OCCT source tree.

See the [Roadmap](ROADMAP.md) and [Progress Evidence](docs/PROGRESS_EVIDENCE.md) for concrete next steps and verified source-level examples.

## Repository layout

`include/nexsolid/` public headers · `src/` implementations · `app/` runnable demo · `tests/` CTest cases · `scripts/` developer tools · `docs/` guides and evidence · `.github/` CI and contribution templates.

## License and dependency notices

Original NexSolid source made available by the repository owner is under **LGPL 2.1 with the Open CASCADE additional exception**, subject to individual file notices. See [LICENSE](LICENSE), [LGPL text](LICENSE_LGPL_21.txt) and [OCCT exception](OCCT_LGPL_EXCEPTION.txt). OCCT is a separately licensed external dependency; preserve upstream attribution and comply with [dependency notices](docs/DEPENDENCIES.md) before redistributing or importing modified OCCT code. See also the [licensing plan](docs/LICENSING_PLAN.md).

---

<p align="center"><strong>Open ambition. Reproducible evidence. Community-driven engineering.</strong></p>
