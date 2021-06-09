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

  EfiBuild.h

Abstract:

  EFI build support macros and defines

  This file contians macros and defines used to automate the build process.
  The infrastructure includes support for including protocols and GUIDs in
  in builds

--*/

#ifndef _EFI_BUILD_H_
#define _EFI_BUILD_H_


//
// Mechanism to include protocols. The macros automatically carry the
// protocols subdirectory name. This prevents the need to add to the
// compiler include path every time a new protocol is included.  
//

#define EFI_STRINGIZE(a)                #a 

#define EFI_PROTOCOL_DEFINITION(a)      EFI_STRINGIZE(Protocol/a/a.h) 
#define EFI_ARCH_PROTOCOL_DEFINITION(a) EFI_STRINGIZE(ArchProtocol/a/a.h) 

//
//  BugBug: We need to add some dependency mechanism to this stuff.
//    I would suggest a new macro that also includes EFI_PROTOCOL_DEFINITION()
//
#define EFI_PROTOCOL_PRODUCER(a)    EFI_PROTOCOL_DEFINITION(a) 
#define EFI_PROTOCOL_CONSUMER(a)    EFI_PROTOCOL_DEFINITION(a) 
#define EFI_PROTOCOL_DEPENDENCY(a)  EFI_PROTOCOL_DEFINITION(a) 

#define EFI_ARCH_PROTOCOL_PRODUCER(a) EFI_ARCH_PROTOCOL_DEFINITION(a)
#define EFI_ARCH_PROTOCOL_CONSUMER(a) EFI_ARCH_PROTOCOL_DEFINITION(a)
#define EFI_ARCH_PROTOCOL_DEPENDENCY(a) EFI_ARCH_PROTOCOL_DEFINITION(a)


//
// Make GUIDs work the same way as protocol
//  
#define EFI_GUID_DEFINITION(a) EFI_STRINGIZE(Guid/a/a##.h) 

//
// Mechanism to associate a short and long ascii string with a GUID.
// For normal builds the strings are not included. A build utility
// can be constructed to extract the strings and build a table. It may
// be possible to add a build opption to automatically generate a GUID
// string table for a GUID to string utility build. 
//
#define EFI_GUID_STRING(guidpointer, shortstring, longstring)

//
// Needed to support ANSI compatible forward references. EfiBind.h contains
// compiler specific mechanism for implementing this feature.
//
//#define EFI_INTERFACE_DECL(type)  _EFI_INTERFACE_DECL(type)


#endif
