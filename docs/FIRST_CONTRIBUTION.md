# Your first NexSolid contribution

NexSolid is an early-stage C++17 project built against Open CASCADE Technology (OCCT).

## 1. Build it
Follow [Getting Started](GETTING_STARTED.md). After installing OCCT development packages:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## 2. Choose a small task
Start with an issue about documentation, reproducible test cases, or input validation. Comment on the issue before starting so work is coordinated.

## 3. Make a focused change
Fork the repository, create a branch, and change only what is needed. For geometry changes, include a small deterministic test. Do not introduce copied third-party code without checking its provenance and license.

## 4. Propose it for review
Open a pull request using the template. Explain the change, the OCCT/compiler versions you tested, and the command output.

## Important licensing note
The project owner's original NexSolid starter code is licensed under LGPL 2.1 with the Open CASCADE additional exception. Read [LICENSE](../LICENSE) and [CONTRIBUTING](../CONTRIBUTING.md) before contributing. The planned modified OCCT import still needs source and license review. Discuss substantial changes with the maintainer before opening a pull request.

Need help? Open a question issue describing where you got stuck. Be courteous and never post secrets or proprietary models.
