tasklist /nh /fi "imagename eq JLinkGDBServerCL.exe" | find /i "JLinkGDBServerCL.exe"
if errorlevel 1 (
  Start "" "%JLINK_INSTALLATION_DIR%/JLinkGDBServerCL.exe" -device "%~1" -if swd -ir
)
