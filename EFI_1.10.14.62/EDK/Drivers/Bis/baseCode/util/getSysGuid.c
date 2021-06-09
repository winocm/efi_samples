///////////////////////////////////////////////////////////////////////////////
//
// This software is provided "as is" with no warranties, express or
// implied, including but not limited to any implied warranty of
// merchantability, fitness for a particular purpose, or freedom from
// infringement.
//
// Intel Corporation may have patents or pending patent applications,
// trademarks, copyrights, or other intellectual property rights that
// relate to this software.  The furnishing of this document does not
// provide any license, express or implied, by estoppel or otherwise,
// to any such patents, trademarks, copyrights, or other intellectual
// property rights.
//
// This software is furnished under license and may only be used or
// copied in accordance with the terms of the license. Except as
// permitted by such license, no part of this software may be reproduced,
// stored in a retrieval system, or transmitted in any form or by any
// means without the express written consent of Intel Corporation.
//
/*

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

*/

//************************************************************************************************//
// getSysGuid.c
//
// Description: contains function GetSystemGuid(), which
//  retreives the system's unique id from EFI lib function.
//
//
// GetSystemGuid( ) - and copy it into storage provided by caller parm.
 //
//************************************************************************************************//

#include <bisBaseCode.h>


CHAR8*
BisLibGetSmbiosString (
    IN  SMBIOS_STRUCTURE_POINTER    *Smbios,
    IN  INT16                       StringNumber
    )
/*++

    Return SMBIOS string given the string number.

    Arguments:
        Smbios - Pointer to SMBIOS structure
        StringNumber - String number to return. -1 is used to skip all strings and 
            point to the next SMBIOS structure.

    Returns:
        Pointer to string, or pointer to next SMBIOS strcuture if StringNumber == -1
--*/
{
    UINT16  Index;
    CHAR8   *String;

    //
    // Skip over formatted section
    //
    String = (CHAR8 *)(Smbios->Raw + Smbios->Hdr->Length);

    //
    // Look through unformated section
    //
    for (Index = 1; Index <= StringNumber; Index++) {
        if (StringNumber == Index) {
            return String;
        }

        //
        // Skip string
        //
        for (; *String != 0; String++);
        String++;

        if (*String == 0) {
            //
            // If double NULL then we are done.
            //  Retrun pointer to next structure in Smbios.
            //  if you pass in a -1 you will always get here
            //
            Smbios->Raw = (UINT8 *)++String;
            return NULL;        
        }
    }
    return NULL;        
}


EFI_STATUS
BisLibGetSmbiosSystemGuidAndSerialNumber (
    IN  EFI_GUID    *SystemGuid,
    OUT CHAR8       **SystemSerialNumber
    )
{
    EFI_STATUS                  Status;
    SMBIOS_STRUCTURE_TABLE      *SmbiosTable;
    SMBIOS_STRUCTURE_POINTER    Smbios;  
    SMBIOS_STRUCTURE_POINTER    SmbiosEnd;  
    UINT16                      Index;
    
    Status = EfiLibGetSystemConfigurationTable(&gEfiSmbiosTableGuid, &SmbiosTable);
    if (EFI_ERROR(Status)) {
        return EFI_NOT_FOUND;
    }

    Smbios.Hdr = (SMBIOS_HEADER *)SmbiosTable->TableAddress;
    SmbiosEnd.Raw = (UINT8 *)(SmbiosTable->TableAddress + SmbiosTable->TableLength);
    for (Index = 0; Index < SmbiosTable->TableLength ; Index++) {
        if (Smbios.Hdr->Type == 1) {
            if (Smbios.Hdr->Length < 0x19) {
                //
                // Older version did not support Guid and Serial number
                //
                continue;
            }

            //
            // SMBIOS tables are byte packed so we need to do a byte copy to
            //  prevend alignment faults on Itanium-based platform.
            
            EfiCopyMem (SystemGuid, &Smbios.Type1->Uuid, sizeof(EFI_GUID));
            *SystemSerialNumber = BisLibGetSmbiosString(&Smbios, Smbios.Type1->SerialNumber);
            return EFI_SUCCESS;
        }

        //
        // Make Smbios point to the next record
        //
        BisLibGetSmbiosString (&Smbios, -1);

        if (Smbios.Raw >= SmbiosEnd.Raw) {
            //
            // SMBIOS 2.1 incorrectly stated the length of SmbiosTable as 0x1e. 
            //  given this we must double check against the lenght of
            /// the structure. My home PC has this bug.ruthard
            //
            return EFI_SUCCESS;
        }
    }

    return EFI_SUCCESS;
}

BIS_STATUS
GetSystemGuid( CSSM_GUID_PTR theSystemGuid )
{
  EFI_GUID	guidStorage;
	BIS_STATUS  rv= 		 BIS_OK;
	EFI_STATUS  st;
	CHAR8		*sysSerialNbr;

    if (theSystemGuid == NULL)
    {
        rv= BIS_BAD_PARM;
    }
    else 
    {
		st= BisLibGetSmbiosSystemGuidAndSerialNumber (
	 		&guidStorage,
	    	&sysSerialNbr
	    	);

		if ( st == EFI_SUCCESS )
		{
			EfiCopyMem( (UINT8*)theSystemGuid, (UINT8*)&guidStorage, sizeof(CSSM_GUID) );
		}

		else 
		{
			DEBUG((EFI_D_ERROR, "EFIBIS_BaseCodeModeGetSystemGUID. Guid not set yet, using 0 value\n"));

			#ifdef EFI_NT_EMULATOR
			//We don't get the guid in NT emulator, but we use zero.
			EfiZeroMem(theSystemGuid, sizeof(CSSM_GUID));
			#else
			rv= BIS_GETGUID_ERROR;
			#endif
		}
		
	}


    return rv;
}



//HISTORY
// Archive: /SMA/Src/bis/util/getSysGuid.c
// Revision: 5
// Date: 10/07/98 1:58p


//eof
