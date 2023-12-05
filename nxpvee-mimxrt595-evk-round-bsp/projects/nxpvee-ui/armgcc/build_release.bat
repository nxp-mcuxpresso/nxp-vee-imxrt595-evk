SET FLAVOUR=Unix Makefiles
SET CMD=make -j
IF "%~1" == "ninja" (
  SET FLAVOUR=Ninja
  SET CMD=ninja
)

if exist CMakeFiles (RD /s /Q CMakeFiles)
if exist Makefile (DEL /s /Q /F Makefile)
if exist cmake_install.cmake (DEL /s /Q /F cmake_install.cmake)
if exist CMakeCache.txt (DEL /s /Q /F CMakeCache.txt)
cmake -DCMAKE_TOOLCHAIN_FILE="../../../mcux-sdk/core/tools/cmake_toolchain_files/armgcc.cmake" -G "%FLAVOUR%" -DCMAKE_BUILD_TYPE=release .
%CMD%
