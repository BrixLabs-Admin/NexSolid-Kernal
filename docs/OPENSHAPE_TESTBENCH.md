# Experimental OpenShape3D testbench (work in progress)

This branch adds a **macOS build helper**, not a finished embedded code editor or a verified running CAD testbench.

## Purpose

Edit a **full OCCT source checkout** containing your geometry patch, rebuild OpenShape3D's OCCT framework from that checkout, and open the app in Xcode for interactive tests.

**Important:** The current NexSolid repository is a small wrapper around system OCCT. Editing `src/Kernel.cpp` alone does not modify OpenShape3D's OCCT engine. Engine-level patches must live in a full OCCT source tree. A future step must integrate that source into NexSolid with provenance and license notices.

## Prerequisites

- macOS, Xcode and iOS Simulator
- Git LFS (`brew install git-lfs`)
- CMake
- A checkout of Open CASCADE source **containing your patch**
- An iOS CMake toolchain file, such as leetal/ios-cmake's `ios.toolchain.cmake`
- Enough disk space/time to compile OCCT from source

## Run

```bash
git clone https://github.com/BrixLabs-Admin/NexSolid-Kernal.git
cd NexSolid-Kernal
git switch feature/openshape3d-testbench

export OCCT_SRC="$HOME/dev/OCCT-patched"
export IOS_TOOLCHAIN="$HOME/dev/ios-cmake/ios.toolchain.cmake"
bash scripts/openshape-testbench.sh
```

The script clones OpenShape3D into `.nexsolid-testbench/openshape3d`, fetches Git LFS files, builds a replacement `ThirdParty/OCCT.xcframework` for arm64 iOS Simulator, and lists Xcode schemes. Open the Xcode project and Build & Run its app scheme on an arm64 simulator.

**Not yet implemented or verified:** automatic launch, Mac-native build, in-app editing, deterministic OCCT/Euclid routing, patch provenance indicator, and end-to-end CI. The script replaces generated framework files in a separate workspace, not upstream OpenShape3D. Do not commit generated binaries without reviewing licenses and repository rules.

## Validation

1. Test a known failing geometry operation using unpatched OCCT.
2. Build the patched OCCT with this script.
3. Launch the app and repeat the same operation.
4. Verify the operation routes through OCCT, not Euclid.
5. Compare shape validity and export, not only the viewport.
6. Save the original failing case as a regression test.

OpenShape3D: https://github.com/laanlabs/openshape3d
