#!/usr/bin/env python3

#
# Python
#
# Copyright 2022 MicroEJ Corp. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be found with this software.
# Copyright 2022 NXP
#

import platform
import os
import sys
import shutil

ret = 0
if (len(sys.argv) == 2 and sys.argv[1] == "makefile.gmake"):
    print("Compiling original makefile")
    ret = os.system("make -f makefile.gmake -R -j 8")
else:
    print("Compiling cmake makefile. For compiling original makefile, please execute 'prog.py makefile.gmake'")
    ret = os.system("make all -R -j 8")

if ret != 0:
    print('Compilation error.')
    os._exit(1)

# Check availability of arm-none-eabi-objdump and Jlink.exe
if platform.system() == 'Linux':
    cmds = [("JLinkExe", "JLINK_INSTALLATION_DIR")]
else:
    cmds = [("Jlink.exe", "JLINK_INSTALLATION_DIR")]

cmds_exe = [None]*len(cmds)
for i, (cmd, env_path) in enumerate(cmds):
    cmds_exe[i] = shutil.which(cmd)
    if None == cmds_exe[i] and env_path in os.environ.keys():
        cmds_exe[i] = shutil.which(cmd, os.F_OK, os.environ.get(env_path))
    if None == cmds_exe[i]:
        print("-E- Could not find %s. Update PATH environment variable or create a %s environment variable" % (cmd, env_path))
        sys.exit(-1)
    print("-I- %s: %s" % (cmd, cmds_exe[i]))

jlink = cmds_exe[0]

jlinkscript = "jlink.script"
device = "MIMXRT595S_M33"


with open(jlinkscript, "w") as fw:
    fw.write("r\n")
    fw.write("loadbin mimxrt595_freertos-bsp.bin 0x08000000\n")
    fw.write("r\ng\n\n")
    fw.write("qc\n")

cmd = '"%s"' % jlink + " -device %s -If SWD -Speed 10000 -CommanderScript %s" % (device, jlinkscript)
print("-I- cmd:" + cmd)
os.system(cmd)
