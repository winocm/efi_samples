#ifndef _PLSHELL_H
#define _PLSHELL_H
/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    plshell.h

Abstract:

    Included by platform code to link in the shell



Revision History

--*/

/* 
 *  Externs for shell apps to load as drivers
 */

EFI_STATUS
InitializeShellEnvironment (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeLS (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeMkDir (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeMode (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeCP (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeComp (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeRM (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeMemmap (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeType (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeDumpStore (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeLoad (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
LoadBmpInitialize (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeVer (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeError (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );


EFI_STATUS
InitializeAttrib (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeMv (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeTime (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeDate (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );


EFI_STATUS
InitializeMount (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeStall (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeReset (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeShell (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeVol (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeCls (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeGetMTC (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeSetSize (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeTouch (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeBCfg (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeEFIEditor (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
InitializeEFIHexEditor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeCompress (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeDecompress (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
InitializeLoadPciRom (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

//
// Debug commands for the shell
//
EFI_STATUS
DumpBlockDev (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    );

EFI_STATUS
InstallEddDebug (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    );

EFI_STATUS
DumpIoModify (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    );

EFI_STATUS
PciDump (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    );

EFI_STATUS
DumpMem (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    );

/* 
 *  To load the shell tools
 */

#if defined(EFI64)

#define PlLoadShellTools()                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"shellenv",                                \
        InitializeShellEnvironment                  \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"attrib",                                  \
        InitializeAttrib                            \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"ls",                                      \
        InitializeLS                                \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"mkdir",                                   \
        InitializeMkDir                             \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"mode",                                    \
        InitializeMode                              \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"cp",                                      \
        InitializeCP                                \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"mv",                                      \
        InitializeMv                                \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"comp",                                    \
        InitializeComp                              \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"rm",                                      \
        InitializeRM                                \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"memmap",                                  \
        InitializeMemmap                            \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"type",                                    \
        InitializeType                              \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"dmpstore",                                \
        InitializeDumpStore                         \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"load",                                    \
        InitializeLoad                              \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"loadbmp",                                 \
        LoadBmpInitialize                           \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"ver",                                     \
        InitializeVer                               \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"err",                                     \
        InitializeError                             \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"time",                                    \
        InitializeTime                              \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"date",                                    \
        InitializeDate                              \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"stall",                                   \
        InitializeStall                             \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"reset",                                   \
        InitializeReset                             \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"vol",                                     \
        InitializeVol                               \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"cls",                                     \
        InitializeCls                               \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"edit",                                    \
        InitializeEFIEditor                         \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"hexedit",                                 \
        InitializeEFIHexEditor                      \
        );                                          \


/* 
 *  Debug Shell Tools
 */

#define PlLoadShellDebugTools()                     \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"dblk",                                    \
        DumpBlockDev                                \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"mm",                                      \
        DumpIoModify                                \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"dmem",                                    \
        DumpMem                                     \
        );

/* 
 *  Additional Debug Shell Tools
 */

#define PlLoadShellAdditionalTools()                \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"pci",                                     \
        PciDump                                     \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"bcfg",                                    \
        InitializeBCfg                              \
        );                                          \
/*                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"eficompress",                             \
        InitializeCompress                          \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"efidecompress",                           \
        InitializeDecompress                        \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"loadpcirom",                              \
        InitializeLoadPciRom                        \
        );                                          \
*/                                                    \

/* 
 *  To load the shell
 */

#define PlLoadShell()                               \
/*                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"shellenv",                                \
        InitializeShellEnvironment                  \
        );                                          \
*/                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_APPLICATION,            \
        L"nshell",                                  \
        InitializeShell                             \
        );                                  
#else

#define PlLoadShellTools()                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"shellenv",                                \
        InitializeShellEnvironment                  \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"attrib",                                  \
        InitializeAttrib                            \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"ls",                                      \
        InitializeLS                                \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"mkdir",                                   \
        InitializeMkDir                             \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"mode",                                    \
        InitializeMode                              \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"cp",                                      \
        InitializeCP                                \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"mv",                                      \
        InitializeMv                                \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"comp",                                    \
        InitializeComp                              \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"rm",                                      \
        InitializeRM                                \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"memmap",                                  \
        InitializeMemmap                            \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"type",                                    \
        InitializeType                              \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"dmpstore",                                \
        InitializeDumpStore                         \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"load",                                    \
        InitializeLoad                              \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"loadbmp",                                 \
        LoadBmpInitialize                           \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"ver",                                     \
        InitializeVer                               \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"err",                                     \
        InitializeError                             \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"time",                                    \
        InitializeTime                              \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"date",                                    \
        InitializeDate                              \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"stall",                                   \
        InitializeStall                             \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"reset",                                   \
        InitializeReset                             \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"vol",                                     \
        InitializeVol                               \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"cls",                                     \
        InitializeCls                               \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"getmtc",                                  \
        InitializeGetMTC                            \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"setsize",                                 \
        InitializeSetSize                           \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"touch",                                   \
        InitializeTouch                             \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"edit",                                    \
        InitializeEFIEditor                         \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"hexedit",                                 \
        InitializeEFIHexEditor                      \
        );                                          \

/* 
 *  Debug Shell Tools
 */

#define PlLoadShellDebugTools()                     \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"dblk",                                    \
        DumpBlockDev                                \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"mm",                                      \
        DumpIoModify                                \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"dmem",                                    \
        DumpMem                                     \
        );

/* 
 *  Additional Debug Shell Tools
 */

#define PlLoadShellAdditionalTools()                \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"pci",                                     \
        PciDump                                     \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"bcfg",                                    \
        InitializeBCfg                              \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"eficompress",                             \
        InitializeCompress                          \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"efidecompress",                           \
        InitializeDecompress                        \
        );                                          \
                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"loadpcirom",                              \
        InitializeLoadPciRom                        \
        );                                          \
                                                    \

/* 
 *  To load the shell
 */

#define PlLoadShell()                               \
/*                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER,    \
        L"shellenv",                                \
        InitializeShellEnvironment                  \
        );                                          \
*/                                                    \
    LOAD_INTERNAL_DRIVER(                           \
        FW,                                         \
        IMAGE_SUBSYSTEM_EFI_APPLICATION,            \
        L"nshell",                                  \
        InitializeShell                             \
        );                                  

#endif

EFI_STATUS
PlInitializeInternalLoad (
    VOID
    );

VOID
PlStartShell (
    VOID
    );


#endif
