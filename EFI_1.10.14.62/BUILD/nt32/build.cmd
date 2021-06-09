@echo off

REM # Delegate control to ..\build-common.cmd
call "%~dp0..\build-common-ia32.cmd" nt32 %1 %2
if errorlevel 1 exit /b 1

REM # Things specific to this tip go here

REM # Invoke emulation environment configuration script
call "%~dp0System.cmd"
if errorlevel 1 goto FAIL

REM # Set the command window title
title EFI 1.1 - NT Emulation Environment
goto :EOF

:FAIL
echo Unexpected
exit /B 1