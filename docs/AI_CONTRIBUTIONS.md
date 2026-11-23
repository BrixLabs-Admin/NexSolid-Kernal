# AI-assisted development at NexSolid

**AI use is encouraged and recommended** for contributors at every level, from first-time C++ developers to expert computational-geometry engineers. We especially recommend **ChatGPT** for planning, understanding OCCT internals, generating test ideas, debugging CMake/compiler errors, reviewing patches and explaining numerical algorithms.

For demanding tasks, use the strongest reasoning-capable ChatGPT model **available in your account**. If a model labelled **Astra Ultra** is available to you, you are welcome to use it, but **we do not assume it is included with ChatGPT Plus or promise access to a particular model**. Free or local AI tools are equally welcome. No subscription is required to contribute.

## Recommended AI workflow
1. Ask the assistant to read the issue and propose a small, testable implementation plan.
2. Ask it to identify relevant OCCT APIs, assumptions, numerical tolerances, and likely edge cases.
3. Implement in small commits, with human understanding and review.
4. Ask AI to suggest adversarial regression tests and review the diff for bugs and license concerns.
5. Actually compile, run CTest, inspect geometry outputs and record performance measurements where relevant.
6. Explain in the pull request what was verified and any meaningful AI assistance.

## Example prompt
> I am contributing to NexSolid Kernel, a C++17 CAD geometry project using OCCT. Help me solve GitHub issue [link]. First propose a small design, identify relevant OCCT APIs and numerical edge cases, then suggest test-first changes. Do not invent APIs or benchmark results. Respect LGPL 2.1 and OCCT source notices. I will verify and test all code before submitting.

## Non-negotiable quality and safety
- Contributors are responsible for the code they submit, whether AI-assisted or handwritten.
- Do not submit untested generated code or claim an AI output proves geometry correctness.
- Preserve third-party copyrights, OCCT license notices, modification dates and attribution.
- Never paste secrets, private CAD models, personal information or proprietary code into external AI tools without authorization.
- Explain reproducible steps, compiler/OCCT versions and known limitations.
- Disclose material AI assistance when it helps reviewers assess generated code.
- Respect other contributors; AI use is a recommendation, **not a gatekeeping requirement**.

See [CONTRIBUTING](../CONTRIBUTING.md) and [first contribution](FIRST_CONTRIBUTION.md).
