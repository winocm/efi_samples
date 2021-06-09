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
  
  efibis_persist.c



Abstract:


Revision History

--*/

#include <efi.h>
#include <efidriverlib.h>
//#include <efibis.h>
#include <bisbasecode.h>

static EFI_GUID	BISPersistProto= EFI_BIS_PERSISTENCE_PROTOCOL;

#define VarBISPersistentStorage             L"BISPersistentStorage"
#define VarBISPersistentStorageFragments    L"NumOfBISPersistentStorageFragments"
#define BISPersistentStorageSize            4096
#define EFI_MAXIMUM_VARIABLE_SIZE           1024
#define PERSIST_MAX_EFI_VAR_SIZE            EFI_MAXIMUM_VARIABLE_SIZE

BOOLEAN
BisGrowBuffer(
  IN OUT EFI_STATUS   *Status,
  IN OUT VOID         **Buffer,
  IN UINTN            BufferSize
  )
/*++

Routine Description:

    Helper function called as part of the code needed
    to allocate the proper sized buffer for various 
    EFI interfaces.

Arguments:

    Status      - Current status

    Buffer      - Current allocated buffer, or NULL

    BufferSize  - Current buffer size needed
    
Returns:
    
    TRUE - if the buffer was reallocated and the caller 
    should try the API again.

--*/
{
  BOOLEAN         TryAgain;

  //
  // If this is an initial request, buffer will be null with a new buffer size
  //

  if (!*Buffer && BufferSize) {
    *Status = EFI_BUFFER_TOO_SMALL;
  }

  //
  // If the status code is "buffer too small", resize the buffer
  //
      
  TryAgain = FALSE;
  if (*Status == EFI_BUFFER_TOO_SMALL) {

    if (*Buffer) {
      gBS->FreePool (*Buffer);
    }

//    *Buffer = AllocatePool (BufferSize);
  *Status = gBS->AllocatePool (
                  EfiBootServicesData, BufferSize, Buffer
                  );

    if (*Buffer) {
      TryAgain = TRUE;
    } else {    
      *Status = EFI_OUT_OF_RESOURCES;
    } 
  }

  //
  // If there's an error, free the buffer
  //

  if (!TryAgain && EFI_ERROR(*Status) && *Buffer) {
    gBS->FreePool (*Buffer);
    *Buffer = NULL;
  }

  return TryAgain;
}
VOID *
BisLibGetVariableAndSize (
    IN CHAR16               *Name,
    IN EFI_GUID             *VendorGuid,
    OUT UINTN               *VarSize
    )
/*++

Routine Description:
  Function returns the value of the specified variable and its size in bytes.

Arguments:
  Name                - A Null-terminated Unicode string that is 
                        the name of the vendor's variable.

  VendorGuid          - A unique identifier for the vendor.

  VarSize             - The size of the returned environment variable in bytes.

Returns:

  None

--*/
{
  EFI_STATUS              Status;
  VOID                    *Buffer;
  UINTN                   BufferSize;

  //
  // Initialize for BisGrowBuffer loop
  //

  Buffer = NULL;
  BufferSize = 100;

  //
  // Call the real function
  //

  while (BisGrowBuffer (&Status, &Buffer, BufferSize)) {
    Status = gRT->GetVariable (
                   Name,
                   VendorGuid,
                   NULL,
                   &BufferSize,
                   Buffer
                   );
  }
  if (Buffer) {
    *VarSize = BufferSize;
  } else {
    *VarSize = 0;
  }
  return Buffer;
}

    
VOID *
BisLibGetVariable (
  IN CHAR16               *Name,
  IN EFI_GUID             *VendorGuid
    )
/*++

Routine Description:
  Function returns the value of the specified variable.

Arguments:
  Name                - A Null-terminated Unicode string that is 
                        the name of the vendor's variable.

  VendorGuid          - A unique identifier for the vendor.

Returns:

  None

--*/
{
  UINTN   VarSize;

  return BisLibGetVariableAndSize (Name, VendorGuid, &VarSize);
}

UINTN
BisStrSize (
    IN CHAR16   *s1
    )
// string size
{
    UINTN        len;
    
    for (len=0; *s1; s1+=1, len+=1) ;
    return (len + 1) * sizeof(CHAR16);
}

UINTN
BisStrLen (
    IN CHAR16   *s1
    )
// string length
{
    UINTN        len;
    
    for (len=0; *s1; s1+=1, len+=1) ;
    return len;
}

///////////////////////////////////////////////////////////////////////////////
// Function: ValidateLength
//
// Purpose:  Verify the persistent storage data length is the length it was
//              initiallized to.  
//
// Parameters:			
//
// Function Returns:  EFI_SUCCESS if succefully returned, otherwise an error.
//
// Note:        It is caller's responsibility to free FragmentName.
///////////////////////////////////////////////////////////////////////////////
EFI_STATUS
ValidateLength(
     IN UINT32      Length)
{
    EFI_STATUS  retCode = EFI_SUCCESS;


    if (Length != BISPersistentStorageSize)
    {
        retCode = EFI_DEVICE_ERROR;
    }

    return retCode;
} // End of ValidateLength

///////////////////////////////////////////////////////////////////////////////
// Function: EnumerateFragmentNames
//
// Purpose:  Return the persistant storage variable names.  The usage model is
//				call EnumerateFramentNames with 0 as the fragment index.
//				It will return the first name if there is one and update
//				FragmentIndex with the next variable name.  It's value will be
//				zero if there are no more.  
//
// Parameters:			
//
// Function Returns:  EFI_SUCCESS if succefully returned, otherwise an error.
//
// Note:        It is caller's responsibility to free FragmentName.
///////////////////////////////////////////////////////////////////////////////
EFI_STATUS
EnumerateFragmentNames(
	OUT	UINT32    *FragmentIndex,
	OUT	CHAR16	  **FragmentName)
{
	EFI_STATUS      Status= EFI_SUCCESS;
    UINT32          *FragmentCount = 0;
    CHAR16          *LocalFragmentName;

	// Get the number of fragments that data was broken into

    FragmentCount = (UINT32*)BisLibGetVariable(
                        VarBISPersistentStorageFragments,   // Name
                        &BISPersistProto);                  // VendorGUID
    if (!FragmentCount)
    {
        DEBUG((EFI_D_ERROR,
            "PERSIST:  Missing Fragment Number variable.\n"));
        Status = EFI_NOT_FOUND;
    }

	if (EFI_SUCCESS == Status)
    {
        LocalFragmentName = x_malloc ((UINT32)(BisStrSize (VarBISPersistentStorage) + sizeof(UINT32)));
  
        // Get the variable name ready
        if (LocalFragmentName)
        {

            EfiCopyMem(LocalFragmentName,                  // Dest
                       VarBISPersistentStorage,            // Src
                       BisStrSize (VarBISPersistentStorage)); // len     

            LocalFragmentName[BisStrLen (VarBISPersistentStorage)]= (CHAR16)(*FragmentIndex + '0'); 

            if (*FragmentIndex < *FragmentCount)	
		    {										
			    (*FragmentIndex)++;
		    }
		    else
		    {
			    // That was the last fragment
			    *FragmentIndex = 0;
		    }

            *FragmentName = LocalFragmentName;

            if (FragmentCount)
            {
                gBS->FreePool(FragmentCount);
            }
        } // End of if (LocalFragmentName)
        else
        {
            // Failed to allocate pool
            DEBUG((EFI_D_ERROR,
                "PERSIST: Memory allocation failed\n"));
            Status = EFI_OUT_OF_RESOURCES;
        }
	}
	
	return Status;
} // End of EnumerateFragmentNames


///////////////////////////////////////////////////////////////////////////////
// Function: Persistentstorage_Read_Fn
//
// Purpose:  Return the data stored in persistent storage
//
// Parameters:  This            -
//              Buffer          - buffer to store the persistent data
//              Reserved        -
//
// Function Returns:  EFI_SUCCESS if succefully returned, otherwise an error.
//
// Note:        It is caller's responsibility to free Buffer.
///////////////////////////////////////////////////////////////////////////////
EFI_STATUS
Persistentstorage_Read_Fn(
	IN  EFI_BIS_PERSISTENCE_INTERFACE   *This,
	OUT UINT8						    *Buffer,
	IN  VOID                            *Reserved
	)
{
    EFI_STATUS              		Status= EFI_SUCCESS;
    UINT32                          BISPersistentStorageLen;
    UINT32                          TotalBISPersistentStorageLen = 0;
    UINT8                           *BISPersistentStorage;
    CHAR16                          *FragmentName;
	UINT32							FragmentIndex = 0;

    FragmentName = 0;

    // Check input parameters
    Status = EFI_INVALID_PARAMETER;     // assume failure till proven otherwise
    if (This)
    {
        if (Buffer)
        {
            if (Reserved == NULL)
            {
                Status = EFI_SUCCESS;
            }
        }
    } 

    if (EFI_SUCCESS == Status)
    {
		while (EFI_SUCCESS == Status)
		{
			Status = EnumerateFragmentNames(&FragmentIndex,
											&FragmentName);
            if (EFI_SUCCESS == Status)
			{	
				BISPersistentStorage = BisLibGetVariableAndSize (
										FragmentName,              // Name
										&BISPersistProto,          // VendorGUID
									   (UINTN*)&BISPersistentStorageLen); // var length
				if (BISPersistentStorage)
				{
					// Add this chunck to the big piece
                    EfiCopyMem (Buffer + TotalBISPersistentStorageLen ,  // Dest
                                BISPersistentStorage,                    // Src
                                BISPersistentStorageLen);                // len 
                    
                    // get the total space so far
					TotalBISPersistentStorageLen+= BISPersistentStorageLen;
                }
				else
				{
					 // Error in retrieving data from persistent storage
					DEBUG((EFI_D_ERROR,
				       	"PERSIST: Variable was not found.\n"));
					Status = EFI_NOT_FOUND;	
				}

				gBS->FreePool(FragmentName);
                if (EFI_SUCCESS == Status)
                {
                    gBS->FreePool(BISPersistentStorage);
                }

				// abort the cycling when the last one is reached
				if (FragmentIndex == 0)
					break;
			}
	    } // end of while
    }

    if (EFI_SUCCESS == Status)
    {
        // Verify data is the expected length
        Status = ValidateLength(TotalBISPersistentStorageLen);
    }

	return Status;
} // End of Persistentstorage_Read_Fn


///////////////////////////////////////////////////////////////////////////////
// Function: Persistentstorage_Write_Fn
//
// Purpose:  Update the certificate data in persistent storage.
//
// Parameters:  This            -
//              Buffer          - buffer of the data to write
//              Reserved        - 
//
// Function Returns:  EFI_SUCCESS if succefully wrote, otherwise an error.
//
// Note: Use 0 for BytesToWrite to clear the data
//
///////////////////////////////////////////////////////////////////////////////
EFI_STATUS
Persistentstorage_Write_Fn(
	IN EFI_BIS_PERSISTENCE_INTERFACE    *This,
	IN UINT8* 						    Buffer,
    IN VOID                             *Reserved
	)
{
    EFI_STATUS              		Status= EFI_SUCCESS;
    CHAR16                          *FragmentName;
    UINT32							            FragmentIndex;
    UINT32                          NumOfBytesToWrite;
    UINT32                          BufferIndex=0;
    BOOLEAN                         KeepWriting = TRUE;

    FragmentIndex = 0;
    FragmentName = 0;

    // Check input parameters
    Status = EFI_INVALID_PARAMETER;     // assume failure till proven otherwise
    if (This)
    {
        if (Buffer)
        {
            if (Reserved == NULL)
            {
                Status = EFI_SUCCESS;
            }
        }
    } 

    if (EFI_SUCCESS == Status)
    {
		// clear out the variables  (delete them)
        while (EFI_SUCCESS == Status)
		{
			Status = EnumerateFragmentNames(&FragmentIndex,
											&FragmentName);

			if (EFI_SUCCESS == Status)
			{	
				Status = gRT->SetVariable (
                              FragmentName,                     // VariableName
                              &BISPersistProto,                 // VendorGuid,
                              EFI_VARIABLE_NON_VOLATILE |
                              EFI_VARIABLE_BOOTSERVICE_ACCESS,  // Attributes
                              0,                                // DataSize
                              Buffer);                          // Data
    
                gBS->FreePool(FragmentName);
                
				// abort the cycling when the last one is reached
				if (FragmentIndex == 0)
					break;
			}	
		} // end of while
        
        // clear the number of fragments
        Status = gRT->SetVariable (
                      VarBISPersistentStorageFragments, // VariableName
                      &BISPersistProto,                 // VendorGuid,
                      EFI_VARIABLE_NON_VOLATILE |
                      EFI_VARIABLE_BOOTSERVICE_ACCESS,  // Attributes
                      0,                                // DataSize
                      Buffer);                          // Data
    
    } // End of if (EFI_SUCCESS == Status)

    FragmentIndex = 0;
    NumOfBytesToWrite = BISPersistentStorageSize;

    // write out the packets
    while (KeepWriting)
    {
        // Get the variable name ready
        FragmentName = x_malloc ((UINT32)(BisStrSize (VarBISPersistentStorage) + sizeof(UINT32)));
        if (FragmentName)
        {

            EfiCopyMem(FragmentName,                       // Dest
                       VarBISPersistentStorage,            // Src
                       BisStrSize (VarBISPersistentStorage)); // len     

            FragmentName[BisStrLen (VarBISPersistentStorage)]= 
                                                 (CHAR16)(FragmentIndex + '0');
        
            if (NumOfBytesToWrite > PERSIST_MAX_EFI_VAR_SIZE)
            {
            
                Status = gRT->SetVariable (
                              FragmentName,                     // VariableName
                              &BISPersistProto,                 // VendorGuid,
                              EFI_VARIABLE_NON_VOLATILE |
                              EFI_VARIABLE_BOOTSERVICE_ACCESS,  // Attributes
                              PERSIST_MAX_EFI_VAR_SIZE,         // DataSize
                              Buffer+ BufferIndex);             // Data
            
                NumOfBytesToWrite = NumOfBytesToWrite - PERSIST_MAX_EFI_VAR_SIZE;
                BufferIndex+=PERSIST_MAX_EFI_VAR_SIZE;
                FragmentIndex++;

            }
            else
            {
                Status = gRT->SetVariable (
                              FragmentName,                     // VariableName
                              &BISPersistProto,                 // VendorGuid,
                              EFI_VARIABLE_NON_VOLATILE |
                              EFI_VARIABLE_BOOTSERVICE_ACCESS,  // Attributes
                              NumOfBytesToWrite,                // DataSize
                              Buffer+ BufferIndex);             // Data
                KeepWriting = FALSE;
            }
        } // End of if (FragmentName)
        else
        {
            // Failed to allocate pool
            DEBUG((EFI_D_ERROR,
                "PERSIST: Memory allocation failed\n"));
            Status = EFI_OUT_OF_RESOURCES;
        }
        
        if (Status != EFI_SUCCESS)
        {
            DEBUG((EFI_D_ERROR,
                	    "PERSIST: Variable was not set. rc=%r\n", Status));
            KeepWriting = FALSE;
        }

        gBS->FreePool(FragmentName);
    } // End of while (KeepWriting)

    if (EFI_SUCCESS == Status)
    {
        // Updates the number of fragments variable
        Status = gRT->SetVariable (
                          VarBISPersistentStorageFragments, // VariableName
                          &BISPersistProto,                 // VendorGuid,
                          EFI_VARIABLE_NON_VOLATILE |
                          EFI_VARIABLE_BOOTSERVICE_ACCESS,  // Attributes
                          sizeof(UINT32),                   // DataSize
                          &FragmentIndex);                  // Data

        if (Status != EFI_SUCCESS)
        {
            DEBUG((EFI_D_ERROR,
                	    "PERSIST: Variable was not set. rc=%r\n", Status));
        }
    }
	return Status;
} // End of Persistentstorage_Write_Fn


///////////////////////////////////////////////////////////////////////////////
// Function: Persistentstorage_Getlength_Fn
//
// Purpose:  Return the size of the data in persistent storage
//
// Parameters:  This        -
//              Length      - Length of data stored in persistent storage
//              Reserved    - 
//
// Function Returns:  Size of data stored in persistent storage
//
// Note:
///////////////////////////////////////////////////////////////////////////////
EFI_STATUS
Persistentstorage_Getlength_Fn(
	IN      EFI_BIS_PERSISTENCE_INTERFACE   *This,
    OUT     UINT32                          *Length,
    IN      VOID                            *Reserved
	)
{
    EFI_STATUS              		Status= EFI_SUCCESS;
    UINT32                          BISPersistentStorageLen;
    UINT32                          TotalBISPersistentStorageLen = 0;
    UINT8                           *BISPersistentStorage;
    CHAR16                          *FragmentName;
	UINT32							FragmentIndex = 0;

    FragmentName = NULL;

    // Check input parameters
    Status = EFI_INVALID_PARAMETER;     // assume failure till proven otherwise
    if (This)
    {
        if (Length)
        {
            if (Reserved == NULL)
            {
                Status = EFI_SUCCESS;
            }
        }
    } 

    if (EFI_SUCCESS == Status)
    {
		while (EFI_SUCCESS == Status)
		{
			Status = EnumerateFragmentNames(&FragmentIndex,
											&FragmentName);

			if (EFI_SUCCESS == Status)
			{	
				BISPersistentStorage = BisLibGetVariableAndSize (
									    FragmentName,              // Name
										&BISPersistProto,          // VendorGUID
									    (UINTN*)&BISPersistentStorageLen); // var length
				if (BISPersistentStorage)
				{
					// get the total space so far
					TotalBISPersistentStorageLen+= BISPersistentStorageLen;
                    gBS->FreePool(BISPersistentStorage);
				}
				else
				{
					 // Error in retrieving data from persistent storage
					DEBUG((EFI_D_ERROR,
				       	"PERSIST: Variable was not found.\n"));
					Status = EFI_NOT_FOUND;	
				}

				gBS->FreePool(FragmentName);
                
				// abort the cycling when the last one is reached
				if (FragmentIndex == 0)
					break;
			}
		} // end of while
    } // End of if (EFI_SUCCESS == Status)

    if (EFI_SUCCESS == Status)
    {
        *Length = TotalBISPersistentStorageLen;
    }

    // Verify data is the expected length
    Status = ValidateLength(TotalBISPersistentStorageLen);    

    return Status;
} // End of Persistentstorage_Getlength_Fn()


///////////////////////////////////////////////////////////////////////////////
// Function: EFIBIS_InitPersistModule
//
// Purpose:  Initiallize the Persististent Module
//
// Parameters:  ImageHandle    -
//              SystemTable    -
//
// Function Returns:    -   EFI_SUCCESS - OK
//                          EFI_OUT_OF_RESOURCES - malloc failure
//                          BIS_PERSIST_MEM_SIZE_MISMATCH - persistent
//                              memory size was not expected size
//
// Note:
///////////////////////////////////////////////////////////////////////////////


EFI_STATUS
EFIBIS_InitPersistModule(
    EFI_BIS_PERSISTENCE_INTERFACE   **persistInterface
    )
{
    EFI_STATUS              		Status= EFI_SUCCESS;
    EFI_BIS_PERSISTENCE_INTERFACE   *persistI_F;
    UINT8				            *Buffer = 0;
    EFI_HANDLE                      tempHandle;

    tempHandle = NULL;

    persistI_F = x_malloc(sizeof(EFI_BIS_PERSISTENCE_INTERFACE));
	if ( persistI_F == NULL)
    {
        DEBUG((EFI_D_ERROR,
                	"PERSIST: Memory allocation failed\n"));
		Status= EFI_OUT_OF_RESOURCES;
	}

	//Fill out the interface structure
    if ( Status == EFI_SUCCESS)    
    {
		persistI_F->Read=		Persistentstorage_Read_Fn;
		persistI_F->Write=		Persistentstorage_Write_Fn;
		persistI_F->GetLength=	Persistentstorage_Getlength_Fn;

		//no real instance data defined
		persistI_F->InstanceData= (VOID*)persistI_F;
	}

    if (EFI_SUCCESS == Status)
    {
        // check if data is present
  Buffer = x_malloc(BISPersistentStorageSize);
        if (Buffer)
        {
            Status = Persistentstorage_Read_Fn(persistI_F,
                                               Buffer,
                                               0);
            gBS->FreePool(Buffer);
            Buffer = 0;
        }

        if (EFI_NOT_FOUND == Status)
        {
            // Variable is not found, allocate it and return
            Buffer = x_malloc(BISPersistentStorageSize);
            if (Buffer)
            {
                Status = Persistentstorage_Write_Fn(persistI_F,
                                                    Buffer,
                                                    0);
                if (EFI_SUCCESS == Status)
                {
                    gBS->FreePool(Buffer);
                    Buffer = 0;
                }
            }
            else
            {
                // Failed to allocate pool
                DEBUG((EFI_D_ERROR,
                	"PERSIST: Memory allocation failed\n"));
                Status = EFI_OUT_OF_RESOURCES;
            }
        } // End of if variable is not found
    } // End of if (EFI_SUCCESS == Status)

	//free mem if error
    if ( Status != EFI_SUCCESS)
    {
		gBS->FreePool( persistI_F );
	}

    if ( Status == EFI_SUCCESS)
    {
    	*persistInterface= persistI_F;

    }

	if ( Status != EFI_SUCCESS)
    {
		DEBUG((EFI_D_ERROR, "EFIBIS_InitPersistModule result %r\n", Status));
	}

    if(Buffer)
    {
        gBS->FreePool(Buffer);
    }

    return Status;
} // End of EFIBIS_InitPersistModule


//eof
