#!/bin/sh

FLAVOUR='Unix Makefiles'
CMD='make -j'

if [ "${1}" = "ninja" ]
then
  FLAVOUR='Ninja'
  CMD='ninja'
fi

if [ -d "CMakeFiles" ];then rm -rf CMakeFiles; fi
if [ -f "Makefile" ];then rm -f Makefile; fi
if [ -f "cmake_install.cmake" ];then rm -f cmake_install.cmake; fi
if [ -f "CMakeCache.txt" ];then rm -f CMakeCache.txt; fi
cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../mcux-sdk/core/tools/cmake_toolchain_files/armgcc.cmake" -G "${FLAVOUR}" -DCMAKE_BUILD_TYPE=debug .
${CMD}

if [ -d "CMakeFiles" ];then rm -rf CMakeFiles; fi
if [ -f "Makefile" ];then rm -f Makefile; fi
if [ -f "cmake_install.cmake" ];then rm -f cmake_install.cmake; fi
if [ -f "CMakeCache.txt" ];then rm -f CMakeCache.txt; fi
cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../mcux-sdk/core/tools/cmake_toolchain_files/armgcc.cmake" -G "${FLAVOUR}" -DCMAKE_BUILD_TYPE=release .
${CMD}
