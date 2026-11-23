# Dependencies and third-party notices

## Open CASCADE Technology

NexSolid Kernel uses Open CASCADE Technology (OCCT) as an external dependency. The starter project calls OCCT APIs and does not copy or redistribute OCCT source files, headers, or binaries.

OCCT is distributed under GNU LGPL version 2.1 with the Open CASCADE exception. The exact terms and any distribution obligations are defined by the OCCT project:

- [OCCT license and exception](https://github.com/Open-Cascade-SAS/OCCT/wiki/license)
- [OCCT official repository](https://github.com/Open-Cascade-SAS/OCCT)

If a future NexSolid Kernel release distributes OCCT binaries or modified OCCT code, maintainers must review and meet the applicable OCCT terms, including notices and source availability. This note is informational and is not a substitute for the license texts.

## Dependency policy

- Prefer system-installed or separately packaged dependencies.
- Record new runtime and build dependencies here, including their versions and license links.
- Do not vendor third-party code without first reviewing its license and recording its source.
- Keep third-party notices with any future redistributable release.

The project owner's original NexSolid starter code is covered by the repository's [LICENSE](../LICENSE), with the full [LGPL 2.1 text](../LICENSE_LGPL_21.txt) and [OCCT exception](../OCCT_LGPL_EXCEPTION.txt). Imported OCCT source must retain its own original notices and attribution. See [Licensing plan](LICENSING_PLAN.md).
