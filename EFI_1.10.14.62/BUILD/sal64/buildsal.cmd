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
REM #      buildsal.cmd
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

set EFI_SOURCE=%EFI%
set EFI_DEBUG=YES
set EFI_DEBUG_CLEAR_MEMORY=NO
set EFI_BOOTSHELL=YES
set SOFT_SDV=NO
cd %BLD_EFI%
