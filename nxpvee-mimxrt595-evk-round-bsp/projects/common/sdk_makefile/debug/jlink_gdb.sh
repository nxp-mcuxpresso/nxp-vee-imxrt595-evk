#!/bin/bash

#
# Copyright 2022 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

JLINKK_PID=$(pgrep JLinkGDBServer)
if [ "${JLINKK_PID}" == "" ]
then
  JLinkGDBServer -device "${1}" -if swd -ir > /dev/null 2>&1 &
fi
