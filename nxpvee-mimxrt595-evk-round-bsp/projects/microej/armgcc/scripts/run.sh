#!/bin/sh

CURRENT_DIRECTORY=$(pwd)

# go to directory of script's location
SCRIPT_DIR=$(dirname "$(realpath "$0")")
cd "$SCRIPT_DIR" || exit 1

# run JLink command to flash target
JLinkExe -device MIMXRT595S_M33 -If SWD -Speed 20000 -CommanderScript "$SCRIPT_DIR"/Jlink.cmd || exit 2

cd "$CURRENT_DIRECTORY" || exit 3
