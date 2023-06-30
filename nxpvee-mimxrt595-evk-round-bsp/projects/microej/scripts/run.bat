@echo off

REM Copyright 2022 MicroEJ Corp. All rights reserved.
REM Use of this source code is governed by a BSD-style license that can be found with this software.

REM 'run.bat' implementation for GCC workbench, using Jlink.

CALL "%~dp0\set_project_env.bat"
IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)


@echo on
SET CURRENT_DIRECTORY=%CD%
SET MAKEFILE_DIR="%~dp0\..\..\nxpvee-ui\sdk_makefile"

CD %MAKEFILE_DIR%
make RELEASE=1 flash_cmsisdap

cd "%CURRENT_DIRECTORY%"
