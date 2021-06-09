@echo off
echo.
echo ***** %~nx0 *****
echo.

REM ##########################################################################
REM # Main
REM ##########################################################################

call "%~dp0build-common.cmd" %1
if errorlevel 1 goto USAGE

call :INIT %2 %3
call :VCVERSEL
if errorlevel 1 goto USAGE

call :BUILDCOMMON
if errorlevel 1 goto USAGE
echo.
echo !!! Current environment being used: VC %EFI_VCVER% !!!
echo.
goto END

:USAGE
echo Refer to Release Notes for detailed information
call :CLEANUP
set EFI_VCVER=
exit /b 1

:END
call :CLEANUP
exit /b 0

REM ##########################################################################
REM # Initialize temporary environment variables
REM ##########################################################################
:INIT

REM echo *** INIT ***

set COMPILERS=%ProgramFiles%
set VC6DIR=%COMPILERS%\Microsoft Visual Studio\VC98
set VC7DIR=%COMPILERS%\Microsoft Visual Studio .Net\VC7
set VC71DIR=%COMPILERS%\Microsoft Visual Studio .Net 2003\VC7

set EFI_VCVER=%EFI_VCVER_DEFAULT%
if not "%1" == "" set EFI_VCVER=%1
exit /b 0

REM ##########################################################################
REM # Cleanup temporary environment variables
REM ##########################################################################
:CLEANUP

REM echo *** CLEANUP ***

set VC6DIR=
set VC7DIR=
set VC71DIR=
set COMPILERS=
exit /b 0

REM ##########################################################################
REM # VC Version Select
REM ##########################################################################
:VCVERSEL

echo *** VCVERSEL ***

if "%EFI_VCVER%" == "" goto INVARG

if /I "%EFI_VCVER%" EQU "AUTO" (
	call :VCVERCHK
	if %EFI_VCVER% EQU 0 exit /b 1
	exit /b 0
)

if /I "%EFI_VCVER%" EQU "VC6" (
	set EFI_VCVER=6
	call :VCVARS32 "%VC6DIR%"
	if errorlevel 1 exit /b 1
	exit /b 0
)

if /I "%EFI_VCVER%" EQU "VC7" (
	set EFI_VCVER=7
	call :VCVARS32 "%VC7DIR%"
	if errorlevel 1 exit /b 1
	exit /b 0
)

if /I "%EFI_VCVER%" EQU "VC71" (
	set EFI_VCVER=7
	call :VCVARS32 "%VC71DIR%"
	if errorlevel 1 exit /b 1
	exit /b 0
)

:INVARG
echo Invalid Argument
echo You must supply `AUTO', `VC6', `VC7' or `VC71' as the first argument explicitly
echo Or set EFI_VCVER_DEFAULT to one of those values
exit /b 1

REM ##########################################################################
REM # Build Common Settings
REM ##########################################################################
:BUILDCOMMON

echo *** BUILDCOMMON ***

if %EFI_VCVER% LSS 6 goto UNKVCVER
if %EFI_VCVER% GTR 7 goto UNKVCVER

set EFI_MSVCTOOLPATH=%MSVCDir%
set EFI_MASMPATH=%EFI_SOURCE%\Tools\Ia32\MASM611
set EFI_DEBUG=YES
set EFI_DEBUG_CLEAR_MEMORY=YES
set EFI_BOOTSHELL=YES

set EFI_LIBPATH=%EFI_MSVCTOOLPATH%\lib
if %EFI_VCVER% EQU 6 set EFI_PSDKPATH=%EFI_MSVCTOOLPATH%
if %EFI_VCVER% EQU 7 set EFI_PSDKPATH=%EFI_MSVCTOOLPATH%\PlatformSDK

call :DISPSETTINGS
exit /b 0

:UNKVCVER
echo Logical error VCVER = %EFI_VCVER%
exit /b 1

REM ##########################################################################
REM # Display current settings
REM ##########################################################################
:DISPSETTINGS

cls
echo ************************************************************************
echo *                             E F I  1.1                               *
echo *                                                                      *
echo *                   Extensible Firmware Interface                      *
echo *                       Sample Implementation                          *
echo *                                                                      *
echo ************************************************************************
echo EFI_SOURCE=%EFI_SOURCE%
REM echo EFI_MSVCTOOLPATH=%EFI_MSVCTOOLPATH%
echo EFI_MASMPATH=%EFI_MASMPATH%
echo EFI_DEBUG=%EFI_DEBUG%
echo EFI_DEBUG_CLEAR_MEMORY=%EFI_DEBUG_CLEAR_MEMORY%
echo EFI_BOOTSHELL=%EFI_BOOTSHELL%
echo EFI_LIBPATH=%EFI_LIBPATH%
echo EFI_PSDKPATH=%EFI_PSDKPATH%
exit /b 0

REM ##########################################################################
REM # Call VCVARS32.BAT
REM ##########################################################################
:VCVARS32

echo *** VCVARS32 ***

echo Invoking %~1\Bin\VCVARS32.BAT ...
call "%~1\Bin\VCVARS32.BAT"
if errorlevel 1 exit /b 1
exit /b 0

REM ##########################################################################
REM # VC Version Check
REM ##########################################################################
:VCVERCHK

echo *** VCVERCHK ***

if "%MSVCDIR%" == "" (
	echo VCVARS32.BAT has not been run or is corrupted.
	exit /b 1
)

call :CLCHK 13.10 7
if not errorlevel 1 call :CLCHK 13.0 7
if not errorlevel 1 call :CLCHK 12.00 6
if not errorlevel 1 echo Unknown VC Version
set EFI_VCVER=%ERRORLEVEL%
exit /b %EFI_VCVER%

REM ##########################################################################
REM # CL Version Check
REM ##########################################################################
:CLCHK

echo *** CLCHK ***

"%MSVCDir%\Bin\cl.exe" 2>&1 | find "%1" > nul
if not errorlevel 1 exit /b %2
exit /b 0
