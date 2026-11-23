# Contributing to NexSolid Kernel

Thanks for considering a contribution. The project is at its foundation stage, so small changes that improve the build, tests, documentation, and starter geometry are especially useful.

## Before you start

1. Read the [Getting Started guide](docs/GETTING_STARTED.md).
2. Check [open issues](https://github.com/BrixLabs-Admin/NexSolid-Kernal/issues) and the [roadmap](ROADMAP.md).
3. For a larger change, open an issue first so the scope and design can be discussed.
4. Do not submit code copied or adapted from another project unless its origin and license are clear.

## Make a change

- Fork the repository and create a focused branch.
- Keep pull requests small and explain the problem they solve.
- Add or update tests for behavior changes.
- Update documentation when commands or user-facing behavior change.
- Use clear commit messages, for example: `Add box validity check`.

## Run the checks

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

All checks in the GitHub Actions workflow must pass before a pull request is ready to merge.

## Open a pull request

Use the pull request template. Include:

- What changed and why.
- How you built and tested it.
- Any follow-up work or limitations.

Be respectful and constructive in reviews. Maintainers may ask for revisions or a smaller scope.

## Licensing status

The repository owner has not selected a project license yet. No license file or contributor license agreement is currently in place. Please do not contribute code copied from third parties, and ask before submitting material that you do not own. The project license and contribution terms should be made clear before the first public release.
