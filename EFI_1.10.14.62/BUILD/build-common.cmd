@echo off
echo.
echo *** %~nx0 ***
echo.

REM ##########################################################################
REM # Main
REM ##########################################################################

echo "%~dp0.."
call :SETEFISRC "%~dp0.."
exit /b 0

REM ##########################################################################
REM # Set EFI_SOURCE
REM ##########################################################################
:SETEFISRC

set EFI_SOURCE=%~f1
exit /b 0