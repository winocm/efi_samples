@echo off
REM #########################################################################
REM #
REM # Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
REM # This software and associated documentation (if any) is furnished
REM # under a license and may only be used or copied in accordance
REM # with the terms of the license. Except as permitted by such
REM # license, no part of this software or documentation may be
REM # reproduced, stored in a retrieval system, or transmitted in any
REM # form or by any means without the express written consent of
REM # Intel Corporation.
REM #
REM #  Module Name:
REM #
REM #      build.cmd
REM #
REM #  Abstract:
REM #
REM #      Initialize environment for building EBC drivers 
REM #
REM #########################################################################

REM # Delegate control to ..\build-common.cmd
call "%~dp0..\build-common.cmd"
if errorlevel 1 exit /b 1

REM # Things specific to this tip go here

:COMMEND
title EFI 1.1 - Byte Code Environment

REM #########################################################################
REM # 
REM #  The following environment variables must be set correctly to build
REM #  EFI EBC drivers.
REM #
REM #  EFI_EBC_TOOLS_PATH    - The path to where the EBC tools were installed
REM #
REM #########################################################################

set EFI_EBC_TOOLS_PATH=%ProgramFiles%\Intel\Ebc

REM #########################################################################
REM # Echo settings to the screen
REM #########################################################################

cls
echo ************************************************************************
echo *                             E F I  1.1                               *
echo *                                                                      *
echo *                   Extensible Firmware Interface                      *
echo *                       Sample Implementation                          *
echo *                                                                      *
echo ************************************************************************
echo * Supported Build Commands                                             *
echo ************************************************************************
echo *     nmake                 - Incremental compile and link             *
echo *     nmake clean           - Remove all OBJ, LIB, EFI, and EXE files  *
echo ************************************************************************
echo EFI_SOURCE=%EFI_SOURCE%
echo EFI_EBC_TOOLS_PATH=%EFI_EBC_TOOLS_PATH%
