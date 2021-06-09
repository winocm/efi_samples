echo off
title EFI 1.1 - Itanium(TM) Drivers Environment
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
REM #      Initialize environment for EFI 1.1
REM #
REM #  Revision History
REM #
REM #########################################################################
REM # 
REM #  The following five environment variables must be set correctly for 
REM #  EFI to build correctly.
REM #
REM #  EFI_SOURCE            - The path to the root of the EFI source tree
REM #
REM #  EFI_DEBUG             - YES for debug version, NO for free version
REM #
REM #  EFI_DEBUG_CLEAR_MEMORY- YES for debug version that clears buffers, NO for free
REM #
REM #  EFI_BOOTSHELL         - YES for booting directly to the EFI Shell
REM #
REM #########################################################################

call "%~dp0..\build-common-ia64.cmd" IpfDrivers %1 %2
if errorlevel 1 exit /b 1

set EFI_DEBUG=YES
set EFI_DEBUG_CLEAR_MEMORY=NO
set EFI_BOOTSHELL=YES
set SOFT_SDV=NO

REM #########################################################################
REM # Echo settings to the screen
REM #########################################################################

cls
echo ************************************************************************
echo *                            E F I  1.1                                *
echo *                                                                      *
echo *                   Extensible Firmware Interface                      *
echo *                       Sample Implementation                          *
echo *                                                                      *
echo *                   Itanium(TM) Drivers Environment                    *
echo ************************************************************************
echo * Supported Build Commands                                             *
echo ************************************************************************
echo *     nmake                 - Incremental compile and link             *
echo *     nmake clean           - Remove all OBJ, LIB, EFI, and EXE files  *
echo ************************************************************************
echo EFI_SOURCE=%EFI_SOURCE%
echo EFI_DEBUG=%EFI_DEBUG%
echo EFI_DEBUG_CLEAR_MEMORY=%EFI_DEBUG_CLEAR_MEMORY%
echo EFI_BOOTSHELL=%EFI_BOOTSHELL%

REM #########################################################################
REM # Done
REM #########################################################################
