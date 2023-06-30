#
# Copyright 2023 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
MODULE_REPOSITORY_SETTINGS_FILE?=$(MODULE_REPOSITORY_SETTINGS_FILE_VAR)
ECLIPSE_HOME=$(ECLIPSE_HOME_VAR)

ifeq ($(OS),Windows_NT)
MICROEJ_SEC=cmd /c "echo jdk.jar.disabledAlgorithms=MD2, MD5, RSA keySize ^< 1024, \
      DSA keySize ^< 1024, include jdk.disabled.namedCurves > javasec.prop"
else
MICROEJ_SEC=echo "jdk.jar.disabledAlgorithms=MD2, MD5, RSA keySize < 1024, \
      DSA keySize < 1024, include jdk.disabled.namedCurves" > javasec.prop
endif

S2S_PORT=5555
S2S_TTY ?= /dev/ttyACM0

ifeq ($(OS),Windows_NT)
BASE_DIR=${CURDIR}
else
BASE_DIR=$(shell pwd)
endif

define del_file
	$(if $(filter $(OS),Windows_NT),$(if $(wildcard $(1)),cmd /c DEL /f /q $(1),),rm -f $(1))
endef

define del_dir
	$(if $(filter $(OS),Windows_NT),$(if $(wildcard $(1)),cmd /c RD /s /q $(subst /,\\,$(1)),),rm -fr $(1))
endef

BSP_DIR=$(BASE_DIR)/nxpvee-mimxrt595-evk-round-bsp
PLAT_DIR=$(BASE_DIR)/MIMXRT595-evk_platform-CM4hardfp_GCC48-1.1.0
APP_DIR=$(BASE_DIR)/nxpvee-mimxrt595-evk-round-apps
FP_DIR=$(BASE_DIR)/nxpvee-mimxrt595-evk-round-fp
MOCK_DIR=$(BASE_DIR)/nxpvee-mimxrt595-evk-round-mock
CONF_DIR=$(BASE_DIR)/nxpvee-mimxrt595-evk-round-configuration
VAL_DIR=$(BASE_DIR)/nxpvee-mimxrt595-evk-round-validation

ifeq ($(ECLIPSE_HOME_VAR),)
$(error Define ECLIPSE_HOME_VAR to i.e. ~/MicroEJ/MicroEJ-SDK-21.11/rcp/)
endif

ifeq ($(strip $(VERBOSE)),1)
	JAVA_VERBOSE=-v
endif

USAGE?=eval

ifeq ($(USAGE),prod)

ifneq ($(MODULE_REPOSITORY_SETTINGS_FILE),)
	MODULE_REPOSITORY_SETTINGS=--module-repository-settings-file=$(MODULE_REPOSITORY_SETTINGS_FILE)
endif
endif

MAIN ?=

nxpvee-ui.prj:
nxpvee-validation.prj:

MAIN.nxpvee-ui.java.app=com.nxp.simpleGFX.SimpleGFX

PROJS = nxpvee-ui

include Makefile.inc
