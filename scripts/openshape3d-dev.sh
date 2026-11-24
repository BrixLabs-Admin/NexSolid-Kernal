#!/usr/bin/env bash
# NexSolid OpenShape3D development launcher (macOS + Xcode).
# This builds the OCCT source specified by OCCT_SRC, NOT the small NexSolid wrapper.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORKSPACE="${NEXSOLID_WORKSPACE:-$ROOT/.dev-openshape3d}"
APP="$WORKSPACE/openshape3d"
: "${OCCT_SRC:?Set OCCT_SRC to the full, patched OCCT source tree (not the NexSolid starter wrapper)}"
: "${IOS_TOOLCHAIN:?Set IOS_TOOLCHAIN to the ios-cmake toolchain file}"
command -v xcodebuild >/dev/null || { echo "Xcode required on macOS"; exit 1; }
command -v git >/dev/null || { echo "git required"; exit 1; }
command -v cmake >/dev/null || { echo "cmake required"; exit 1; }
command -v git-lfs >/dev/null || { echo "git-lfs required (brew install git-lfs)"; exit 1; }
test -f "$OCCT_SRC/CMakeLists.txt" || { echo "Invalid OCCT_SRC: $OCCT_SRC"; exit 1; }
test -f "$IOS_TOOLCHAIN" || { echo "Invalid IOS_TOOLCHAIN: $IOS_TOOLCHAIN"; exit 1; }
mkdir -p "$WORKSPACE"
if [ ! -d "$APP/.git" ]; then
  git clone https://github.com/laanlabs/openshape3d.git "$APP"
fi
(cd "$APP" && git lfs install && git lfs pull)
echo "Building patched OCCT from: $(cd "$OCCT_SRC" && pwd)"
echo "Source revision: $(git -C "$OCCT_SRC" rev-parse --short HEAD 2>/dev/null || echo uncommitted)"
(cd "$APP" && OCCT_SRC="$(cd "$OCCT_SRC" && pwd)" IOS_TOOLCHAIN="$(cd "$(dirname "$IOS_TOOLCHAIN")" && pwd)/$(basename "$IOS_TOOLCHAIN")" PLATFORMS="${PLATFORMS:-SIMULATORARM64 OS64}" bash scripts/build_occt_ios.sh)
echo "Patched OCCT.xcframework built. Opening OpenShape3D in Xcode."
PROJECT="$(find "$APP" -maxdepth 3 \( -name '*.xcworkspace' -o -name '*.xcodeproj' \) -print -quit)"
if [ -n "$PROJECT" ]; then open "$PROJECT"; else open "$APP"; fi
echo "IMPORTANT: Build/run the app from Xcode; do NOT use the App Store binary."
echo "Verify each tested operation uses the OCCT bridge, not the Euclid fallback."
