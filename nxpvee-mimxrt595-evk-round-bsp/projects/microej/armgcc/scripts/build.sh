#!/bin/sh

CURRENT_DIRECTORY=$(pwd)

# go to parent directory of script's location
SCRIPT_DIR=$(dirname "$(realpath "$0")")
cd "$SCRIPT_DIR"/.. || exit 1

make -j"$(nproc)" -R all || exit 2

cp mimxrt595_freertos-bsp.axf "$CURRENT_DIRECTORY"/application.out || exit 3
cp mimxrt595_freertos-bsp.hex "$CURRENT_DIRECTORY"/application.hex || exit 3
cp mimxrt595_freertos-bsp.bin "$CURRENT_DIRECTORY"/application.bin || exit 3

cd "$CURRENT_DIRECTORY" || exit 4
