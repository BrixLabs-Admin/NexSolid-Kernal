#!/usr/bin/env bash
# One-command NexSolid configure, build, smoke-test, and demo run.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${NEXSOLID_BUILD_DIR:-$ROOT/build}"
BUILD_TYPE="${NEXSOLID_BUILD_TYPE:-Release}"

echo "==> Configuring NexSolid (${BUILD_TYPE})"
cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

echo "==> Building NexSolid"
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" --parallel

echo "==> Running automated tests"
ctest --test-dir "$BUILD_DIR" -C "$BUILD_TYPE" --output-on-failure

echo "==> Running CLI demo"
if [ -x "$BUILD_DIR/nexsolid_demo" ]; then
    "$BUILD_DIR/nexsolid_demo"
elif [ -x "$BUILD_DIR/$BUILD_TYPE/nexsolid_demo" ]; then
    "$BUILD_DIR/$BUILD_TYPE/nexsolid_demo"
else
    echo "Demo executable not found. Try your platform-specific build output." >&2
    exit 1
fi

echo "==> NexSolid starter build, tests and demo finished successfully."
