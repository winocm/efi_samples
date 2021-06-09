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

  CommPs2.c
    
Abstract:   

  PS2 Mouse Communication Interface 


Revision History

--*/

#include "Ps2Mouse.h"
#include "CommPs2.h"

EFI_STATUS
PS2MouseRead (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  OUT VOID                *Buffer,
  IN OUT UINTN            *BufSize,
  IN  UINTN               State
  );

// 8042 I/O function
EFI_STATUS
Out8042Command (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,
  IN UINT8                Command
  );

EFI_STATUS
In8042Data (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN OUT UINT8            *Data
  );

EFI_STATUS
Out8042Data (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN UINT8                Data
  );

EFI_STATUS
Out8042AuxCommand (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN UINT8                Command,
  IN BOOLEAN              Resend
  );

EFI_STATUS
In8042AuxData (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN OUT UINT8            *Data
  );

EFI_STATUS
Out8042AuxData (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN UINT8                Data
  );

EFI_STATUS
CheckForInput (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo
  );

EFI_STATUS
WaitInputEmpty (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN UINTN                Timeout
  );

EFI_STATUS
WaitOutputFull (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN UINTN                Timeout
  );

UINT8 SampleRateTbl[MAX_SR] = { 0xa, 0x14, 0x28, 0x3c, 0x50, 0x64, 0xc8 } ; 

UINT8 ResolutionTbl[MAX_CMR] = { 0, 1, 2, 3 };

EFI_STATUS
KbcSelfTest (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo
  )
{
  EFI_STATUS  Status ;
  UINT8       Data ;
  
  //
  // Keyboard controller self test
  //
  Status = Out8042Command(IsaIo, SELF_TEST);
  if (EFI_ERROR(Status)) {
    return Status ;
  }
  
  //
  // Read return code
  //
  Status = In8042Data ( IsaIo, &Data ) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }
  
  if ( Data != 0x55 ) {
    return EFI_DEVICE_ERROR ;
  } 
  
  //
  // Set system flag
  //
  Status = Out8042Command(IsaIo, READ_CMD_BYTE);
  if (EFI_ERROR(Status)) {
    return Status ;
  }

  Status = In8042Data( IsaIo, &Data ) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }

  Status = Out8042Command(IsaIo, WRITE_CMD_BYTE);
  if (EFI_ERROR(Status)) {
    return Status ;
  }

  Data |= CMD_SYS_FLAG;
  Status = Out8042Data( IsaIo, Data ) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }
  
  return EFI_SUCCESS ;
}

EFI_STATUS
KbcEnableAux (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo
  )
{
  //
  // Send 8042 enable mouse command
  //
  return Out8042Command(IsaIo, ENABLE_AUX);
}

EFI_STATUS
KbcDisableAux (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo
  )
{
  //
  // Send 8042 disable mouse command
  //
  return Out8042Command(IsaIo, DISABLE_AUX);
}

EFI_STATUS
KbcEnableKb (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo
  )
{
  //
  // Send 8042 enable keyboard command
  //
  return Out8042Command(IsaIo, ENABLE_KB);
}

EFI_STATUS
KbcDisableKb (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo
  )
{
  //
  // Send 8042 disable keyboard command
  //
  return Out8042Command(IsaIo, DISABLE_KB);
}

EFI_STATUS
CheckKbStatus (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,
  OUT BOOLEAN             *KeyboardEnable
  )
{
  EFI_STATUS  Status;
  UINT8       Data;
  
  //
  // Send command to read KBC command byte
  //
  Status = Out8042Command(IsaIo, READ_CMD_BYTE);
  if (EFI_ERROR(Status)) {
    return Status ;
  }

  Status = In8042Data( IsaIo, &Data ) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }

  //
  // Check keyboard enable or not
  //
  if ( (Data & CMD_KB_STS) == CMD_KB_DIS ) {
    *KeyboardEnable = FALSE;
  } else {
    *KeyboardEnable = TRUE;
  }
  
  return EFI_SUCCESS;  
}

EFI_STATUS
PS2MouseReset (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo  
  )
{
  EFI_STATUS  Status;
  UINT8       Data;

  Status = Out8042AuxCommand ( IsaIo, RESET_CMD, FALSE ) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }
  
  Status = In8042AuxData ( IsaIo, &Data ) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }

  //
  // Check BAT Complete Code
  //
  if ( Data != PS2MOUSE_BAT1 ) {
    return EFI_DEVICE_ERROR ;
  }
  
  Status = In8042AuxData ( IsaIo, &Data ) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }
   
  //
  // Check BAT Complete Code
  //
  if ( Data != PS2MOUSE_BAT2 ) {
    return EFI_DEVICE_ERROR ;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PS2MouseSetSampleRate ( 
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN MOUSE_SR             SampleRate 
  )
{
  EFI_STATUS Status;
  
  //
  // Send auxiliary command to set mouse sample rate
  //
  Status = Out8042AuxCommand ( IsaIo, SETSR_CMD, FALSE ) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }

  Status = Out8042AuxData ( IsaIo, SampleRateTbl[SampleRate] ) ;
  
  return Status ;
}
  
EFI_STATUS
PS2MouseSetResolution ( 
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN MOUSE_RE             Resolution
  )
{
  EFI_STATUS Status ; 
  
  //
  // Send auxiliary command to set mouse resolution
  //
  Status = Out8042AuxCommand ( IsaIo, SETRE_CMD, FALSE ) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }
  
  Status = Out8042AuxData ( IsaIo, ResolutionTbl[Resolution] ) ;
    
  return Status ;
}

EFI_STATUS
PS2MouseSetScaling ( 
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN MOUSE_SF             Scaling
  )
{
  UINT8 Command ;
    
  Command = (UINT8)(Scaling == SF1 ? SETSF1_CMD : SETSF2_CMD) ;
  
  //
  // Send auxiliary command to set mouse scaling data
  //
  return Out8042AuxCommand ( IsaIo, Command, FALSE );  
}

EFI_STATUS
PS2MouseEnable(
  IN EFI_ISA_IO_PROTOCOL  *IsaIo
  )
{
  //
  // Send auxiliary command to enable mouse
  //
  return Out8042AuxCommand ( IsaIo, ENABLE_CMD, FALSE );
}

EFI_STATUS
PS2MouseGetPacket (
  PS2_MOUSE_DEV     *MouseDev
  )
{
  EFI_STATUS        Status ;
  BOOLEAN           KeyboardEnable;
  UINT8             Packet[PS2_PACKET_LENGTH];
  UINT8             Data;
  UINTN             Count ;
  UINTN             State ;
  INT8              RelativeMovementX;
  INT8              RelativeMovementY; 
  BOOLEAN           LButton;
  BOOLEAN           RButton;

  KeyboardEnable = FALSE;
  Count = 1;
  State = PS2_READ_BYTE_ONE;

  //
  // State machine to get mouse packet 
  //
  while( 1 )  {

    switch( State )
    {
      case PS2_READ_BYTE_ONE: 
          //
          // Read mouse first byte data, if failed, immediately return
          //
          KbcDisableAux(MouseDev->IsaIo);
          Status = PS2MouseRead( MouseDev->IsaIo, &Data, &Count, State );
          if ( EFI_ERROR( Status ) ) {
            KbcEnableAux(MouseDev->IsaIo);
            return EFI_NOT_READY;
          }

          if( Count != 1 ) {
            KbcEnableAux(MouseDev->IsaIo);
            return EFI_NOT_READY;
          }

          if( IS_PS2_SYNC_BYTE( Data ) )  {
            Packet[0] = Data;
            State = PS2_READ_DATA_BYTE;
            
            CheckKbStatus(MouseDev->IsaIo, &KeyboardEnable);
            KbcDisableKb(MouseDev->IsaIo);
            KbcEnableAux(MouseDev->IsaIo);
          }
          break ;

      case PS2_READ_DATA_BYTE: 
          Count = 2;
          Status = PS2MouseRead( MouseDev->IsaIo, (Packet + 1), &Count, State );
          if ( EFI_ERROR( Status ) ) {
            if (KeyboardEnable) {
              KbcEnableKb(MouseDev->IsaIo);
            }
            return EFI_NOT_READY;
          }

          if( Count != 2 ) {
            if (KeyboardEnable) {
              KbcEnableKb(MouseDev->IsaIo);
            }
            return EFI_NOT_READY;
          }
          
          State = PS2_PROCESS_PACKET;
          break;

      case PS2_PROCESS_PACKET: 
          if (KeyboardEnable) {
            KbcEnableKb(MouseDev->IsaIo);
          }
          //
          // Decode the packet 
          //
          RelativeMovementX = Packet[1];
          RelativeMovementY = Packet[2];
          RButton = (BOOLEAN)( ( Packet[0] & 0x2 ) ? TRUE : FALSE );
		  LButton = (BOOLEAN)( ( Packet[0] & 0x1 ) ? TRUE : FALSE );

          //
          // Update mouse state 
          //
		  if (RelativeMovementX != 0 ||
			  RelativeMovementY != 0 ||
			  MouseDev->State.LeftButton != LButton ||
			  MouseDev->State.RightButton != RButton) {
            MouseDev->State.RelativeMovementX += RelativeMovementX; 
            MouseDev->State.RelativeMovementY -= RelativeMovementY;
            MouseDev->State.LeftButton = LButton;
            MouseDev->State.RightButton = RButton;
            MouseDev->StateChanged = TRUE;
		  }
          
          return EFI_SUCCESS;
    }
  }
}

EFI_STATUS
PS2MouseRead (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  OUT VOID                *Buffer,
  IN OUT UINTN            *BufSize,
  IN  UINTN               State
  )
{
  EFI_STATUS  Status;
  UINTN       BytesRead;
  UINT8       Data;
  
  Status = EFI_SUCCESS;
  BytesRead = 0;

  if (State == PS2_READ_BYTE_ONE)
  {
    //
    // Check input for mouse
    //
    Status = CheckForInput(IsaIo);
    
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }

  Data = (UINT8)(*BufSize);
  while( BytesRead < *BufSize ) {
    
    Status = WaitOutputFull (IsaIo, TIMEOUT) ;
    if ( EFI_ERROR( Status ) ) {
      break ;
    }
    
    IsaIo->Io.Read(IsaIo, EfiIsaIoWidthUint8, KBC_DATA_PORT, 1, Buffer);

    BytesRead++;
    Buffer = (UINT8*)Buffer + 1 ;
  }

  //
  // Verify the correct number of bytes read 
  //
  if (BytesRead == 0 || BytesRead != *BufSize) {
    Status = EFI_NOT_FOUND;
  }

  *BufSize = BytesRead;
  return Status;  
}

//
// 8042 I/O function
//
EFI_STATUS
Out8042Command (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,
  IN UINT8                Command
  )
{
  EFI_STATUS  Status ;
  UINT8       Data ;

  //
  // Wait keyboard controller input buffer empty
  //
  Status = WaitInputEmpty (IsaIo, TIMEOUT) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }
  
  //
  // Send command
  //
  Data = Command;
  IsaIo->Io.Write( IsaIo, EfiIsaIoWidthUint8, KBC_CMD_STS_PORT, 1, &Data ) ;
  
  Status = WaitInputEmpty (IsaIo, TIMEOUT) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }

  return  EFI_SUCCESS;
}  

EFI_STATUS
Out8042Data (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN UINT8                Data
  )
{
  EFI_STATUS  Status;
  UINT8       temp;
  //
  // Wait keyboard controller input buffer empty
  //
  Status = WaitInputEmpty (IsaIo, TIMEOUT) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }
  
  temp = Data;
  IsaIo->Io.Write( IsaIo, EfiIsaIoWidthUint8, KBC_DATA_PORT, 1, &temp ) ;
  
  Status = WaitInputEmpty (IsaIo, TIMEOUT) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }

  return  EFI_SUCCESS;
}

EFI_STATUS
In8042Data (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN OUT UINT8            *Data
  )
{
  UINTN   Delay ;
  UINT8   temp ;
  
  Delay = TIMEOUT / 50 ;
  
  do {
    IsaIo->Io.Read (IsaIo, EfiIsaIoWidthUint8, KBC_CMD_STS_PORT, 1, &temp);
    
    //
    // Check keyboard controller status bit 0(output buffer status) 
    //
    if ( (temp & KBC_OUTB) == KBC_OUTB ) {
      break;
    }
    gBS->Stall(50);
    Delay --;
  } while (Delay);
  
  if (Delay == 0) {
    return EFI_TIMEOUT ;
  }
  
  IsaIo->Io.Read (IsaIo, EfiIsaIoWidthUint8, KBC_DATA_PORT, 1, Data);

  return EFI_SUCCESS ;
}

EFI_STATUS
Out8042AuxCommand (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN UINT8                Command,
  IN BOOLEAN              Resend
  )
{
  EFI_STATUS  Status ;
  UINT8       Data ;
  
  //
  // Wait keyboard controller input buffer empty
  //
  Status = WaitInputEmpty (IsaIo, TIMEOUT) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }
  
  //
  // Send write to auxiliary device command
  //
  Data = WRITE_AUX_DEV;
  IsaIo->Io.Write( IsaIo, EfiIsaIoWidthUint8, KBC_CMD_STS_PORT, 1, &Data ) ;
  
  Status = WaitInputEmpty (IsaIo, TIMEOUT) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }
  
  //
  // Send auxiliary device command
  //
  IsaIo->Io.Write( IsaIo, EfiIsaIoWidthUint8, KBC_DATA_PORT, 1, &Command ) ;
  
  //
  // Read return code
  //
  Status = In8042AuxData ( IsaIo, &Data ) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }
  
  if ( Data == PS2_ACK ) {
    
    // Receive mouse acknowledge, command send success
    return EFI_SUCCESS ;
    
  } else if ( Resend ) {
    
    // Resend fail
    return EFI_DEVICE_ERROR ;
  
  } else if ( Data == PS2_RESEND ) {
  
    // Resend command
    Out8042AuxCommand ( IsaIo, Command, TRUE ) ;
  
  } else {
    
    // Invalid return code
    return EFI_DEVICE_ERROR ;
  
  }

  return EFI_SUCCESS ;  
}

EFI_STATUS
Out8042AuxData (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN UINT8                Data
  )
{
  EFI_STATUS  Status;
  UINT8       Temp;
  //
  // Wait keyboard controller input buffer empty
  //
  Status = WaitInputEmpty (IsaIo, TIMEOUT) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }
  
  //
  // Send write to auxiliary device command
  //
  Temp = WRITE_AUX_DEV;
  IsaIo->Io.Write( IsaIo, EfiIsaIoWidthUint8, KBC_CMD_STS_PORT, 1, &Temp ) ;
  
  Status = WaitInputEmpty (IsaIo, TIMEOUT) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }

  Temp = Data;
  IsaIo->Io.Write( IsaIo, EfiIsaIoWidthUint8, KBC_DATA_PORT, 1, &Temp ) ;
  
  Status = WaitInputEmpty (IsaIo, TIMEOUT) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }

  return  EFI_SUCCESS;
}

EFI_STATUS
In8042AuxData (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN OUT UINT8            *Data
  )
{
  EFI_STATUS Status ;
  
  //
  // wait for output data
  //
  Status = WaitOutputFull (IsaIo, BAT_TIMEOUT) ;
  if ( EFI_ERROR( Status ) ) {
    return Status ;
  }
  
  IsaIo->Io.Read (IsaIo, EfiIsaIoWidthUint8, KBC_DATA_PORT, 1, Data);

  return EFI_SUCCESS ;
}

EFI_STATUS
CheckForInput (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo
  )
{
  UINT8   Data;
  
  IsaIo->Io.Read(IsaIo, EfiIsaIoWidthUint8, KBC_CMD_STS_PORT, 1, &Data);
  
  //
  // Check keyboard controller status, if it is output buffer full and for auxiliary device
  //
  if ( (Data & (KBC_OUTB | KBC_AUXB)) != (KBC_OUTB | KBC_AUXB) ) {
    return EFI_NOT_READY;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
WaitInputEmpty (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN UINTN                Timeout
  )
{
  UINTN   Delay ; 
  UINT8   Data ;
  
  Delay = Timeout / 50 ;
  
  do {
    IsaIo->Io.Read (IsaIo, EfiIsaIoWidthUint8, KBC_CMD_STS_PORT, 1, &Data);
  
    //
    // Check keyboard controller status bit 1(input buffer status) 
    //
    if ( (Data & KBC_INPB) == 0 ) {
      break; 
    }
    gBS->Stall(50);
    Delay --;
  } while (Delay);
  
  if (Delay == 0) {
    return EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}

EFI_STATUS
WaitOutputFull (
  IN EFI_ISA_IO_PROTOCOL  *IsaIo,  
  IN UINTN                Timeout
  )
{
  UINTN   Delay ;
  UINT8   Data ;
  
  Delay = Timeout / 50 ;
  
  do {
    IsaIo->Io.Read (IsaIo, EfiIsaIoWidthUint8, KBC_CMD_STS_PORT, 1, &Data);
    
    //
    // Check keyboard controller status bit 0(output buffer status) 
    //  & bit5(output buffer for auxiliary device)
    //
    if ( (Data & (KBC_OUTB | KBC_AUXB)) == (KBC_OUTB | KBC_AUXB) ) {
      break;
    }
    gBS->Stall(50);
    Delay --;
  } while (Delay);
  
  if (Delay == 0) {
    return EFI_TIMEOUT ;
  }
  
  return EFI_SUCCESS;  
}
