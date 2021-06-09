#
# Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
# This software and associated documentation (if any) is furnished
# under a license and may only be used or copied in accordance
# with the terms of the license. Except as permitted by such
# license, no part of this software or documentation may be
# reproduced, stored in a retrieval system, or transmitted in any
# form or by any means without the express written consent of
# Intel Corporation.
#
# Module Name:
#
#    Master.mak
#    
# Abstract:
#
#    Master makefile 
#
# Revision History
#

.lib.efi:
   $(LINK) $(L_FLAGS) $(TARGET_LIB) /out $(BUILD_DIR)\$(*B).dll

all: $(BIN_TARGETS)