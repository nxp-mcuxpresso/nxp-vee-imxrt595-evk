#
# Copyright 2023 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
TARGET=mimxrt595_freertos-bsp
DEVICE=MIMXRT595S_M33
ADDRESS=0x08000000
FLAVOUR=debug

ifeq ($(strip $(RELEASE)),1)
FLAVOUR=release
endif

include ../../common/sdk_makefile/Makefile
