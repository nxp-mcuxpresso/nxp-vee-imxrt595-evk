@echo off

REM Copyright 2021-2022 MicroEJ Corp. All rights reserved.
REM Use of this source code is governed by a BSD-style license that can be found with this software.

REM 'set_project_env.bat' implementation for GCC workbench, using JLINK

REM 'set_project_env' is responsible for
REM - checking the availability of required environment variables 
REM - setting project local variables for 'build.bat' and 'run.bat' 

REM Required Environment Variable
REM JLink installation directory 
REM JLINK_INSTALLATION_DIR
REM GNU ToolChain installation directory 
REM GNUCC_93x_INSTALLATION_DIR

IF "%JLINK_INSTALLATION_DIR%" == "" (
    ECHO Please set the environment variable 'JLINK_INSTALLATION_DIR'
    exit /B -1
)

IF "%GNUCC_93x_INSTALLATION_DIR%" == "" (
    ECHO Please set the environment variable 'GNUCC_93x_INSTALLATION_DIR'
    exit /B -1
)

SET PROJECT_DIR=%~dp0
SET PROJECT_NAME=application

ECHO JLINK_INSTALLATION_DIR=%JLINK_INSTALLATION_DIR%
ECHO GNUCC_93x_INSTALLATION_DIR=%JLINK_INSTALLATION_DIR%

SET PATH=%PATH%;%GNUCC_93x_INSTALLATION_DIR%

exit /B 0