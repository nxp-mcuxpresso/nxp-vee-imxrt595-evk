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

cd "%~dp0"

"%JLINK_INSTALLATION_DIR%\Jlink.exe" -device MIMXRT555S_M33 -If SWD -Speed 20000 -CommanderScript Jlink.cmd

cd "%CURRENT_DIRECTORY%"