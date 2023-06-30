#!/bin/sh

CURRENT_DIRECTORY=$(pwd)

# go to parent directory of script's location
SCRIPT_DIR=$(dirname "$(realpath "$0")")
cd "$SCRIPT_DIR/../../nxpvee-ui/sdk_makefile" || exit 1

make RELEASE=1 -j"$(nproc)" || exit 2

cp ../armgcc/release/mimxrt595_freertos-bsp.elf "$CURRENT_DIRECTORY"/application.out || exit 3
cp ../armgcc/release/mimxrt595_freertos-bsp.hex "$CURRENT_DIRECTORY"/application.hex || exit 3
cp ../armgcc/release/mimxrt595_freertos-bsp.bin "$CURRENT_DIRECTORY"/application.bin || exit 3

cd "$CURRENT_DIRECTORY" || exit 4
