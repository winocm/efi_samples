@echo off
echo.
echo ***** %~nx0 *****
echo.

REM ##########################################################################
REM # Main
REM ##########################################################################

call %~dp0build-common.cmd %1
if errorlevel 1 exit /b 1
exit /b 0