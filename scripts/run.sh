#!/bin/bash
set -e

debug=0
gdb=0

while test $# != 0
do
    case "$1" in
    -d|--debug) debug=1 ;;
    --gdb) gdb=1 ;;
    --) shift; break;;
    *)  break ;;
    esac
    shift
done

cd "$(dirname "$0")/.."

BUILD_DIR="${BUILD_DIR:-build}"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake ..
make
cd ..

if [ "$debug" -eq 1 ]; then
    QEMU_ARGS="-d int,cpu_reset $QEMU_ARGS"
fi

if [ "$gdb" -eq 1 ]; then
    QEMU_ARGS="-s -S $QEMU_ARGS"
fi

exec qemu-system-x86_64 \
    -cdrom "$BUILD_DIR/tetros.iso" \
    -net none \
    -serial mon:stdio \
    -m 4G \
    -audiodev pa,id=speaker \
    -machine pcspk-audiodev=speaker \
    -no-reboot \
    -no-shutdown \
    $QEMU_ARGS
