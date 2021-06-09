@echo off

REM # Delegate control to ..\build-common.cmd
call "%~dp0..\build-common-ia32.cmd" Ia-32emb %1 %2
if errorlevel 1 exit /b 1

REM # Things specific to this tip go here

REM # Set the command window title
title EFI 1.1 - IA-32 Embedded Environment - No BIOS
