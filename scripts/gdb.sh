#!/bin/bash
set -e

cd "$(dirname "$0")/.."

BUILD_DIR="${BUILD_DIR:-build}"

exec gdb "$BUILD_DIR/tetros" -ex "target remote localhost:1234"

