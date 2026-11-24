#!/usr/bin/env bash
# NexSolid experimental OCCT -> OpenShape3D testbench (macOS only).
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORKSPACE="${NEXSOLID_WORKSPACE:-$ROOT/.nexsolid-testbench}"
OCCT_SRC="${OCCT_SRC:-}"
IOS_TOOLCHAIN="${IOS_TOOLCHAIN:-}"
if [[ "$(uname -s)" != Darwin ]]; then echo "Requires macOS and Xcode." >&2; exit 1; fi
for cmd in git cmake xcodebuild libtool; do command -v "$cmd" >/dev/null || { echo "Missing $cmd" >&2; exit 1; }; done
[[ -d "$OCCT_SRC" && -f "$OCCT_SRC/CMakeLists.txt" ]] || { echo "Set OCCT_SRC to your MODIFIED OCCT source checkout (not the NexSolid wrapper)." >&2; exit 1; }
[[ -f "$IOS_TOOLCHAIN" ]] || { echo "Set IOS_TOOLCHAIN to an ios.toolchain.cmake file." >&2; exit 1; }
mkdir -p "$WORKSPACE"
if [[ ! -d "$WORKSPACE/openshape3d/.git" ]]; then
  git clone https://github.com/laanlabs/openshape3d.git "$WORKSPACE/openshape3d"
fi
cd "$WORKSPACE/openshape3d"
git lfs install
git lfs pull
echo "Building experimental OCCT from $OCCT_SRC"
OCCT_SRC="$OCCT_SRC" IOS_TOOLCHAIN="$IOS_TOOLCHAIN" \
  WORK="$WORKSPACE/occt-build" \
  PLATFORMS="${PLATFORMS:-SIMULATORARM64}" \
  ./scripts/build_occt_ios.sh
echo "Experimental OCCT xcframework built. Source commit: $(git -C "$OCCT_SRC" rev-parse --short HEAD 2>/dev/null || echo unknown)"
echo "Building OpenShape3D for iOS Simulator..."
xcodebuild -list -project openshape3d.xcodeproj
echo "Open the Xcode project, choose its app scheme and iOS Simulator destination, then Build & Run."
echo "WARNING: Verify the app operation uses OCCT (not Euclid) before treating results as NexSolid engine tests."
