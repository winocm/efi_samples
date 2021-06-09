@echo off

REM #
REM # Copyright (c) 1999, 2003
REM # Intel Corporation.
REM # All rights reserved.
REM # 

REM #
REM # This File set environment variable for EFI Toolkit build
REM #
REM 
REM # SDK_BUILD_ENV 
REM #        bios32 : bios32 environment
REM #	     nt32   : nt32 environment
REM #        sal64  : sal64 environment 
REM 
REM # SDK_INSTALL_DIR
REM #        EFI Toolkit source directory
REM 
REM # EFI_APPLICATION_COMPATIBILITY
REM #        EFI_APP_102        : Write application for EFI 1.02
REM #        EFI_APP_110        : Write application for EFI 1.10
REM #        EFI_APP_MULTIMODAL : Write application for multiple EFI version
REM # 

set SDK_BUILD_ENV=bios32
set SDK_INSTALL_DIR=c:\PROJECT\EFI_Toolkit_1.10
set EFI_APPLICATION_COMPATIBILITY=EFI_APP_102
set EFI_DEBUG=YES

echo.

if "%1"=="" (
  echo Usage: build [bios32^|nt32^|sal64]
  echo SDK_BUILD_ENV is set to bios32 by default
  echo. 
) ELSE (
  set SDK_BUILD_ENV=%1
)

echo SDK_BUILD_ENV=%SDK_BUILD_ENV%
echo SDK_INSTALL_DIR=%SDK_INSTALL_DIR%
echo EFI_APPLICATION_COMPATIBILITY=%EFI_APPLICATION_COMPATIBILITY%
echo EFI_DEBUG=%EFI_DEBUG%
echo.

