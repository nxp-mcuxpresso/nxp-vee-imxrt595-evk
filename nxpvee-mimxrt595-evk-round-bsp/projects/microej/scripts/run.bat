REM 'run.bat' implementation

SET CURRENT_DIRECTORY=%CD%
SET MAKEFILE_DIR="%~dp0\..\..\nxpvee-ui\sdk_makefile"

CD %MAKEFILE_DIR%

IF %ERRORLEVEL% NEQ 0 (
	ECHO "Can not open %MAKEFILE_DIR%"
	EXIT /B %ERRORLEVEL%
)

make RELEASE=1 flash_cmsisdap

IF %ERRORLEVEL% NEQ 0 (
	ECHO "Failed to flash the board"
	EXIT /B %ERRORLEVEL%
)

cd "%CURRENT_DIRECTORY%"
