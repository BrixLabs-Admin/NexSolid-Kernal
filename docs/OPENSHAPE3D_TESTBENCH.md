# OpenShape3D experimental engine testbench (macOS)

**Status: integration scaffold, not yet verified end-to-end.** This is not an in-app source editor or a ready-made downloadable app. It rebuilds OpenShape3D's OCCT XCFramework from a contributor-provided **full modified OCCT source tree**, then opens the app project in Xcode.

## Prerequisites
- macOS, full Xcode and Apple SDKs, Homebrew, git, git-lfs, CMake
- A local **full OCCT source tree with your patch applied**; the current NexSolid `src/Kernel.cpp` wrapper is NOT that tree.
- The `ios.toolchain.cmake` file from the leetal ios-cmake toolchain project, compatible with OpenShape3D's script

## Build and launch
```bash
brew install cmake git-lfs
git lfs install
export OCCT_SRC="$HOME/dev/OCCT" # full OCCT source, patched
export IOS_TOOLCHAIN="$HOME/dev/ios-cmake/ios.toolchain.cmake"
bash scripts/openshape3d-dev.sh
```

The script clones OpenShape3D into `.dev-openshape3d/` (ignored by git), builds `ThirdParty/OCCT.xcframework` using its existing `scripts/build_occt_ios.sh`, and opens the Xcode project. Choose an available simulator/device and press Run in Xcode. To edit the engine, use your editor on `$OCCT_SRC`, then rerun the script. Builds may take significant time.

**Caveats:** OpenShape3D also has Euclid mesh-CSG operations; confirm the specific CAD feature is routed through OCCT before using it to validate a patch. The framework build and app launch are not tested by GitHub's Linux CI. This script does not yet automatically prove that a patch affected the operation. Future work: in-app engine provenance, reproducible failing models, one-button rebuild, and dedicated Mac test matrix. Respect OpenShape3D and OCCT licensing.
