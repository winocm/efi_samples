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

  Keyboard.c
    
Abstract:

  Helper functions for USB Keyboard Driver

Revision History

--*/
#include "keyboard.h"
#include "hid.h"

//
//USB Key Code to Efi key mapping table
//Format:<efi scan code>, <unicode without shift>, <unicode with shift>
//
static
UINT8 KeyConvertionTable[USB_KEYCODE_MAX_MAKE][3] = {
    SCAN_NULL,      'a',      'A',      // 0x04
    SCAN_NULL,      'b',      'B',      // 0x05
    SCAN_NULL,      'c',      'C',      // 0x06
    SCAN_NULL,      'd',      'D',      // 0x07
    SCAN_NULL,      'e',      'E',      // 0x08
    SCAN_NULL,      'f',      'F',      // 0x09
    SCAN_NULL,      'g',      'G',      // 0x0A
    SCAN_NULL,      'h',      'H',      // 0x0B
    SCAN_NULL,      'i',      'I',      // 0x0C
    SCAN_NULL,      'j',      'J',      // 0x0D
    SCAN_NULL,      'k',      'K',      // 0x0E
    SCAN_NULL,      'l',      'L',      // 0x0F
    SCAN_NULL,      'm',      'M',      // 0x10
    SCAN_NULL,      'n',      'N',      // 0x11
    SCAN_NULL,      'o',      'O',      // 0x12
    SCAN_NULL,      'p',      'P',      // 0x13
    SCAN_NULL,      'q',      'Q',      // 0x14
    SCAN_NULL,      'r',      'R',      // 0x15
    SCAN_NULL,      's',      'S',      // 0x16
    SCAN_NULL,      't',      'T',      // 0x17
    SCAN_NULL,      'u',      'U',      // 0x18
    SCAN_NULL,      'v',      'V',      // 0x19
    SCAN_NULL,      'w',      'W',      // 0x1A
    SCAN_NULL,      'x',      'X',      // 0x1B
    SCAN_NULL,      'y',      'Y',      // 0x1C
    SCAN_NULL,      'z',      'Z',      // 0x1D
    SCAN_NULL,      '1',      '!',      // 0x1E
    SCAN_NULL,      '2',      '@',      // 0x1F
    SCAN_NULL,      '3',      '#',      // 0x20
    SCAN_NULL,      '4',      '$',      // 0x21
    SCAN_NULL,      '5',      '%',      // 0x22
    SCAN_NULL,      '6',      '^',      // 0x23
    SCAN_NULL,      '7',      '&',      // 0x24
    SCAN_NULL,      '8',      '*',      // 0x25
    SCAN_NULL,      '9',      '(',      // 0x26
    SCAN_NULL,      '0',      ')',      // 0x27
    SCAN_NULL,      0x0d,     0x0d,     // 0x28   Enter
    SCAN_ESC,       0x00,     0x00,     // 0x29   Esc
    SCAN_NULL,      0x08,     0x08,     // 0x2A   Backspace
    SCAN_NULL,      0x09,     0x09,     // 0x2B   Tab
    SCAN_NULL,      ' ',      ' ',      // 0x2C   Spacebar
    SCAN_NULL,      '-',      '_',      // 0x2D
    SCAN_NULL,      '=',      '+',      // 0x2E
    SCAN_NULL,      '[',      '{',      // 0x2F
    SCAN_NULL,      ']',      '}',      // 0x30
    SCAN_NULL,      '\\',     '|',      // 0x31
    SCAN_NULL,      '\\',     '|',      // 0x32  Keyboard US \ and |
    SCAN_NULL,      ';',      ':',      // 0x33
    SCAN_NULL,      '\'',     '"',      // 0x34
    SCAN_NULL,      '`',      '~',      // 0x35  Keyboard Grave Accent and Tlide
    SCAN_NULL,      ',',      '<',      // 0x36
    SCAN_NULL,      '.',      '>',      // 0x37
    SCAN_NULL,      '/',      '?',      // 0x38
    SCAN_NULL,      0x00,     0x00,     // 0x39   CapsLock
    SCAN_F1,        0x00,     0x00,     // 0x3A
    SCAN_F2,        0x00,     0x00,     // 0x3B
    SCAN_F3,        0x00,     0x00,     // 0x3C  
    SCAN_F4,        0x00,     0x00,     // 0x3D  
    SCAN_F5,        0x00,     0x00,     // 0x3E
    SCAN_F6,        0x00,     0x00,     // 0x3F
    SCAN_F7,        0x00,     0x00,     // 0x40
    SCAN_F8,        0x00,     0x00,     // 0x41
    SCAN_F9,        0x00,     0x00,     // 0x42
    SCAN_F10,       0x00,     0x00,     // 0x43
    SCAN_NULL,      0x00,     0x00,     // 0x44   F11
    SCAN_NULL,      0x00,     0x00,     // 0x45   F12
    SCAN_NULL,      0x00,     0x00,     // 0x46   PrintScreen
    SCAN_NULL,      0x00,     0x00,     // 0x47   Scroll Lock
    SCAN_NULL,      0x00,     0x00,     // 0x48   Pause
    SCAN_INSERT,    0x00,     0x00,     // 0x49
    SCAN_HOME,      0x00,     0x00,     // 0x4A
    SCAN_PAGE_UP,   0x00,     0x00,     // 0x4B
    SCAN_DELETE,    0x00,     0x00,     // 0x4C
    SCAN_END,       0x00,     0x00,     // 0x4D
    SCAN_PAGE_DOWN, 0x00,     0x00,     // 0x4E
    SCAN_RIGHT,     0x00,     0x00,     // 0x4F
    SCAN_LEFT,      0x00,     0x00,     // 0x50
    SCAN_DOWN,      0x00,     0x00,     // 0x51
    SCAN_UP,        0x00,     0x00,     // 0x52
    SCAN_NULL,      0x00,     0x00,     // 0x53   NumLock
    SCAN_NULL,      '/',      '/',      // 0x54
    SCAN_NULL,      '*',      '*',      // 0x55
    SCAN_NULL,      '-',      '-',      // 0x56
    SCAN_NULL,      '+',      '+',      // 0x57
    SCAN_NULL,      0x0d,     0x0d,     // 0x58
    SCAN_END,       '1',      '1',      // 0x59
    SCAN_DOWN,      '2',      '2',      // 0x5A
    SCAN_PAGE_DOWN, '3',      '3',      // 0x5B
    SCAN_LEFT,      '4',      '4',      // 0x5C
    SCAN_NULL,      '5',      '5',      // 0x5D
    SCAN_RIGHT,     '6',      '6',      // 0x5E
    SCAN_HOME,      '7',      '7',      // 0x5F
    SCAN_UP,        '8',      '8',      // 0x60
    SCAN_PAGE_UP,   '9',      '9',      // 0x61
    SCAN_INSERT,    '0',      '0',      // 0x62
    SCAN_DELETE,    '.',      '.',      // 0x63
    SCAN_NULL,      '\\',     '|',      // 0x64 Keyboard Non-US \ and |
    SCAN_NULL,      0x00,     0x00,     // 0x65 Keyboard Application
    SCAN_NULL,      0x00,     0x00,     // 0x66 Keyboard Power
    SCAN_NULL,      '=' ,     '='      // 0x67 Keypad =
 }; 
    
STATIC KB_MODIFIER KB_Mod[8] = {
  { MOD_CONTROL_L,  0xe0 }, // 11100000 
  { MOD_CONTROL_R,  0xe4 }, // 11100100 
  { MOD_SHIFT_L,    0xe1 }, // 11100001 
  { MOD_SHIFT_R,    0xe5 }, // 11100101 
  { MOD_ALT_L,      0xe2 }, // 11100010 
  { MOD_ALT_R,      0xe6 }, // 11100110 
  { MOD_WIN_L,      0xe3 }, // 11100011 
  { MOD_WIN_R,      0xe7 }, // 11100111 
};


BOOLEAN
IsUSBKeyboard(
  IN  EFI_USB_IO_PROTOCOL       *UsbIo
  )
/*++
  
  Routine Description:
    Uses USB I/O to check whether the device is a USB Keyboard device.
  
  Arguments:
    UsbIo:    Points to a USB I/O protocol instance.
    
  Returns:
  
--*/  
{
  EFI_STATUS                      Status;
  EFI_USB_INTERFACE_DESCRIPTOR    InterfaceDescriptor;

  //
  // Get the Default interface descriptor, currently we 
  // assume it is interface 1
  //
  Status = UsbIo->UsbGetInterfaceDescriptor(
                        UsbIo,
                        &InterfaceDescriptor
                        );

  if(EFI_ERROR(Status)) {
    return FALSE;
  }    

  if (InterfaceDescriptor.InterfaceClass == CLASS_HID &&
    InterfaceDescriptor.InterfaceSubClass == SUBCLASS_BOOT &&
    InterfaceDescriptor.InterfaceProtocol == PROTOCOL_KEYBOARD) {
        
        return TRUE;
  }

  return FALSE; 
}


EFI_STATUS
InitUSBKeyboard(
  IN USB_KB_DEV   *UsbKeyboardDevice
  )
/*++
  
  Routine Description:
    Initialize USB Keyboard device and all private data structures.
    
  Arguments:
    UsbKeyboardDevice:    The USB_KB_DEV instance.
    
  Returns:
  
--*/
{
  UINT8           ConfigValue;
  UINT8           Protocol;
  UINT8           ReportId;
  UINT8           Duration;
  EFI_STATUS      Status;
  UINT32          TransferResult;

  
  InitUSBKeyBuffer(&(UsbKeyboardDevice->KeyboardBuffer));
  
  ConfigValue = 0x01;   // default configurations
  
  //
  // Uses default configuration to configure the USB Keyboard device.
  //
  Status = UsbSetDeviceConfiguration (
            UsbKeyboardDevice->UsbIo, 
            (UINT16)ConfigValue,
            &TransferResult
            );
  if(EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }
  
  UsbGetProtocolRequest(UsbKeyboardDevice->UsbIo,
                        UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
                        &Protocol
                        );
  //
  // Sets boot protocol for the USB Keyboard.
  // This driver only supports boot protocol.
  // !!BugBug: How about the device that does not support boot protocol?
  //
  if(Protocol != BOOT_PROTOCOL) {
    UsbSetProtocolRequest(
          UsbKeyboardDevice->UsbIo,
          UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
          BOOT_PROTOCOL
          );
  }   
  
  //
  // the duration is indefinite, so the endpoint will inhibit reporting forever,
  // and only reporting when a change is detected in the report data.
  //
  ReportId = 0;       // idle value for all report ID
  Duration = 0;   // idle forever until there is a key pressed and released.
  UsbSetIdleRequest(
                 UsbKeyboardDevice->UsbIo,
                 UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
                 ReportId,
                 Duration
                 );
  
  UsbKeyboardDevice->CtrlOn     = 0;
  UsbKeyboardDevice->AltOn      = 0;
  UsbKeyboardDevice->ShiftOn    = 0;
  UsbKeyboardDevice->NumLockOn  = 0 ;
  UsbKeyboardDevice->CapsOn     = 0 ;
  EfiZeroMem(UsbKeyboardDevice->LastKeyCodeArray, sizeof(UINT8)*8);
  
  //
  // Set a timer for repeat keys' generation.
  //
  if(UsbKeyboardDevice->RepeatTimer) {
    gBS->CloseEvent(UsbKeyboardDevice->RepeatTimer);
    UsbKeyboardDevice->RepeatTimer = 0;
  }  
  Status = gBS->CreateEvent (
            EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL, 
            EFI_TPL_NOTIFY, 
            USBKeyboardRepeatHandler, 
            UsbKeyboardDevice, 
            &UsbKeyboardDevice->RepeatTimer
            ) ;
 
  return EFI_SUCCESS;
} 


EFI_STATUS
KeyboardHandler(
  IN  VOID          *Data,
  IN  UINTN         DataLength,
  IN  VOID          *Context,  
  IN  UINT32        Result
  )
/*++
  
  Routine Description:
    Handler function for USB Keyboard's asynchronous interrupt transfer.
    
  Arguments:
    Data:       A pointer to a buffer that is filled with key data which is
                retrieved via asynchronous interrupt transfer.
    DataLength: Indicates the size of the data buffer.
    Context:    Pointing to USB_KB_DEV instance.
    Result:     Indicates the result of the asynchronous interrupt transfer.
    
  Returns:
  
--*/  
{
  USB_KB_DEV              *UsbKeyboardDevice;
  EFI_USB_IO_PROTOCOL     *UsbIo;
  UINT8                   *CurKeyCodeBuffer;
  UINT8                   *OldKeyCodeBuffer;
  UINT8                   CurModifierMap;
  UINT8                   OldModifierMap;
  UINT8                   i;
  UINT8                   j;
  BOOLEAN                 Down;
  EFI_STATUS              Status;
  BOOLEAN                 KeyRelease,KeyPress;
  UINT8                   SavedTail;
  USB_KEY                 UsbKey;  
  UINT8                   NewRepeatKey = 0;
  UINT32                  UsbStatus;
  UINT8                   PacketSize;
  
  UsbKeyboardDevice = (USB_KB_DEV*)Context;
  ASSERT(UsbKeyboardDevice);
  
  UsbIo = UsbKeyboardDevice->UsbIo;
  
  //
  // Analyzes the Result and performs corresponding action.
  //
  if (Result != EFI_USB_NOERROR) {
    //
    // stop the repeat key generation if any
    //
    UsbKeyboardDevice->RepeatKey = 0;
    
    gBS->SetTimer(UsbKeyboardDevice->RepeatTimer, 
                  TimerCancel, 
                  USBKBD_REPEAT_RATE
                  );
    
    if ((Result & EFI_USB_ERR_STALL) == EFI_USB_ERR_STALL) {
      UsbClearEndpointHalt (
                UsbIo,
                UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress,
                &UsbStatus
                );
    }
    
    //
    // Delete & Submit this interrupt again
    //
    
    Status = UsbIo->UsbAsyncInterruptTransfer(
             UsbIo,
             UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress,
             FALSE,
             0,
             0,
             NULL,
             NULL
           );
    
    PacketSize =  (UINT8)(UsbKeyboardDevice->IntEndpointDescriptor.MaxPacketSize);
      
    Status = UsbIo->UsbAsyncInterruptTransfer(
                      UsbIo,
                      UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress,
                      TRUE,
                      UsbKeyboardDevice->IntEndpointDescriptor.Interval,
                      PacketSize,
                      KeyboardHandler,
                      UsbKeyboardDevice
                );
    return EFI_DEVICE_ERROR;
  }
  
  if(DataLength == 0 || Data == NULL) {
    return EFI_SUCCESS;
  }
  
  CurKeyCodeBuffer = (UINT8*)Data;    
  OldKeyCodeBuffer = UsbKeyboardDevice->LastKeyCodeArray ;
  
  //
  // checks for new key stroke.
  // if no new key got, return immediately.
  //
  for (i = 0; i < 8; i ++) {
    if(OldKeyCodeBuffer[i] != CurKeyCodeBuffer[i]) {
      break;
    }
  }
  if(i == 8) {
    return EFI_SUCCESS;
  }
  
  //
  // Parse the modifier key
  //
  
  CurModifierMap = CurKeyCodeBuffer[0];
  OldModifierMap = OldKeyCodeBuffer[0];

  //
  // handle modifier key's pressing or releasing situation.
  //
  for (i = 0; i < 8; i ++) {

    if ((CurModifierMap & KB_Mod[i].Mask) != 
                      (OldModifierMap & KB_Mod[i].Mask)) {
      //
      // if current modifier key is up, then 
      // CurModifierMap & KB_Mod[i].Mask = 0;
      // otherwize it is a non-zero value.
      // Inserts the pressed modifier key into key buffer.
      //
      Down = (UINT8)(CurModifierMap & KB_Mod[i].Mask);
      InsertKeyCode(&(UsbKeyboardDevice->KeyboardBuffer),KB_Mod[i].Key,Down);
    } 
  }
  
  //
  // handle normal key's releasing situation
  //  
  KeyRelease = FALSE;
  for (i = 2; i < 8; i ++) {
    
    if(!USBKBD_VALID_KEYCODE(OldKeyCodeBuffer[i])) {
      continue;
    }
    
    KeyRelease = TRUE;
    for (j = 2; j < 8; j ++) {
      
      if(!USBKBD_VALID_KEYCODE(CurKeyCodeBuffer[j])) {
        continue;
      }
        
      if (OldKeyCodeBuffer[i] == CurKeyCodeBuffer[j]) {
        KeyRelease = FALSE;
        break;
      }
    }
    if (KeyRelease) {
      InsertKeyCode(&(UsbKeyboardDevice->KeyboardBuffer),OldKeyCodeBuffer[i],0);
      //
      // the original reapeat key is released.
      //
      if (OldKeyCodeBuffer[i] == UsbKeyboardDevice->RepeatKey) {
        UsbKeyboardDevice->RepeatKey = 0;
      }
    }
  }
  
  
  //
  // original repeat key is released, cancel the repeat timer
  //
  if (UsbKeyboardDevice->RepeatKey == 0) {
    gBS->SetTimer(UsbKeyboardDevice->RepeatTimer, 
                  TimerCancel, 
                  USBKBD_REPEAT_RATE
                  );
  }
  
  //
  // handle normal key's pressing situation
  //
  KeyPress = FALSE;
  for (i = 2; i < 8; i ++) {
    
    if(!USBKBD_VALID_KEYCODE(CurKeyCodeBuffer[i])) {
      continue;
    }
    
    KeyPress = TRUE;
    for (j = 2; j < 8; j ++) {
      
      if(!USBKBD_VALID_KEYCODE(OldKeyCodeBuffer[j])) {
        continue;
      }
        
      if (CurKeyCodeBuffer[i] == OldKeyCodeBuffer[j]) {
        KeyPress = FALSE;
        break;
      }
    }
    if(KeyPress) {    
      InsertKeyCode(&(UsbKeyboardDevice->KeyboardBuffer),CurKeyCodeBuffer[i],1);
      //
      // NumLock pressed or CapsLock pressed
      //
      if(CurKeyCodeBuffer[i] == 0x53 || CurKeyCodeBuffer[i] == 0x39) {
        UsbKeyboardDevice->RepeatKey = 0;
      } else {
        NewRepeatKey = CurKeyCodeBuffer[i];
        //
        // do not repeat the original repeated key
        //
        UsbKeyboardDevice->RepeatKey = 0;
      }
    }   
  } 
  
  //
  // Update LastKeycodeArray[] buffer in the 
  // Usb Keyboard Device data structure.
  //
  for (i = 0; i < 8; i ++) {
    UsbKeyboardDevice->LastKeyCodeArray[i] = CurKeyCodeBuffer[i];
  }
  
  //
  // pre-process KeyboardBuffer, pop out the ctrl,alt,del key in sequence
  // and judge whether it will invoke reset event.
  //
  SavedTail = UsbKeyboardDevice->KeyboardBuffer.bTail;
  i = UsbKeyboardDevice->KeyboardBuffer.bHead;
  while(i != SavedTail) {
    RemoveKeyCode(&(UsbKeyboardDevice->KeyboardBuffer),&UsbKey);
    
    switch(UsbKey.KeyCode) {
      case 0xe0:
      case 0xe4:
        if(UsbKey.Down) {
          UsbKeyboardDevice->CtrlOn = 1;
        } else {
          UsbKeyboardDevice->CtrlOn = 0;
        }
        break;
      
      case 0xe2:
      case 0xe6:
        if(UsbKey.Down) {
          UsbKeyboardDevice->AltOn = 1;
        } else {
          UsbKeyboardDevice->AltOn = 0;
        }
        break;
      
      case 0x4c:      // Del Key Code
      case 0x63:
        if(UsbKey.Down) {
          if(UsbKeyboardDevice->CtrlOn && UsbKeyboardDevice->AltOn) {
            gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);         
          } 
        }
        break;
        
      default:
        break;
    }
    
    //
    // insert the key back to the buffer. 
    // so the key sequence will not be destroyed.
    //
    InsertKeyCode(&(UsbKeyboardDevice->KeyboardBuffer),
                  UsbKey.KeyCode,
                  UsbKey.Down
                  );
    i = UsbKeyboardDevice->KeyboardBuffer.bHead;
     
  }
  
  //
  // If have new key pressed, update the RepeatKey value, and set the
  // timer to repeate delay timer
  //
  if(NewRepeatKey != 0) {    
    //
    // sets trigger time to "Repeat Delay Time",
    // to trigger the repeat timer when the key is hold long
    // enough time.
    //
    gBS->SetTimer(UsbKeyboardDevice->RepeatTimer, 
                  TimerRelative, 
                  USBKBD_REPEAT_DELAY
                  );
    UsbKeyboardDevice->RepeatKey = NewRepeatKey;
  } 

  return EFI_SUCCESS;
}   


EFI_STATUS
USBParseKey(
  IN OUT  USB_KB_DEV  *UsbKeyboardDevice,
  OUT     UINT8       *KeyChar
  )
/*++
  
  Routine Description:
    Retrieves a key character after parsing the raw data in keyboard buffer.
    
  Arguments:
    UsbKeyboardDevice:    The USB_KB_DEV instance.
    KeyChar:              Points to the Key character after key parsing.
    
  Returns:
  
--*/  
{
  USB_KEY   UsbKey;
  
  *KeyChar = 0;
  
  while (!IsUSBKeyboardBufferEmpty(UsbKeyboardDevice->KeyboardBuffer)) {
    //
    // pops one raw data off.
    //
    RemoveKeyCode(&(UsbKeyboardDevice->KeyboardBuffer),&UsbKey);
    
    if (!UsbKey.Down) {
      switch(UsbKey.KeyCode) {
        case 0xe0:  // fall through
        case 0xe4:
          UsbKeyboardDevice->CtrlOn = 0;
          break;
      
        case 0xe1:  // fall through
        case 0xe5:
          UsbKeyboardDevice->ShiftOn = 0;
          break;
        
        case 0xe2:  // fall through
        case 0xe6:
          UsbKeyboardDevice->AltOn = 0;
          break;
        
        default:          
          break;
      }
      
      continue;
    }
    
    //
    // Analyzes key pressing situation
    //  
    switch (UsbKey.KeyCode) {
      case 0xe0:  // fall through
      case 0xe4:
        UsbKeyboardDevice->CtrlOn = 1;
        continue;
        break;
      
      case 0xe1:  // fall through
      case 0xe5:
        UsbKeyboardDevice->ShiftOn = 1;
        continue;
        break;
        
      case 0xe2:  // fall through
      case 0xe6:
        UsbKeyboardDevice->AltOn = 1;
        continue;
        break;
        
      case 0xe3:  // fall through
      case 0xe7:
        continue;       
        break;
        
      case 0x53:
        UsbKeyboardDevice->NumLockOn ^= 1;
        SetKeyLED(UsbKeyboardDevice);
        continue;
        break;
        
      case 0x39:
        UsbKeyboardDevice->CapsOn ^= 1;        
        SetKeyLED(UsbKeyboardDevice);
        continue;
        break;
        
      //
      // F11,F12,PrintScreen,ScrollLock,Pause,Application,Power
      // keys are not valid EFI key
      //  
      case 0x44:  // fall through
      case 0x45:  // fall through
      case 0x46:  // fall through
      case 0x47:  // fall through
      case 0x48:  // fall through
      case 0x65:  // fall through
      case 0x66: 
        continue;
        break;
          
      default:
        break;
    }
    
    //
    // When encountered Del Key...
    //
    if (UsbKey.KeyCode == 0x4c || UsbKey.KeyCode == 0x63) {
      if(UsbKeyboardDevice->CtrlOn && UsbKeyboardDevice->AltOn) {
        gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL); 
      }
    }
    
    *KeyChar = UsbKey.KeyCode;
    return EFI_SUCCESS;   
  }
  
  return EFI_NOT_READY;
    
}


EFI_STATUS
USBKeyCodeToEFIScanCode(
  IN  USB_KB_DEV      *UsbKeyboardDevice, 
  IN  UINT8           KeyChar,
  OUT EFI_INPUT_KEY   *Key
  )
/*++
  
  Routine Description:
    Converts USB Keyboard code to EFI Scan Code.
    
  Arguments:  
    UsbKeyboardDevice:    The USB_KB_DEV instance.
    KeyChar:              Indicates the key code that will be interpreted.    
    Key:                  A pointer to a buffer that is filled in with 
                          the keystroke information for the key that 
                          was pressed.
  Returns:
  
--*/
{
  UINT8     Index;
  
  if (!USBKBD_VALID_KEYCODE(KeyChar)) {
    return EFI_NOT_READY;
  }
  
  //
  // valid USB Key Code starts from 4
  //
  Index = (UINT8)(KeyChar - 4);
  
  if (Index >= USB_KEYCODE_MAX_MAKE){
    return EFI_NOT_READY;
  }    
  
  Key->ScanCode = KeyConvertionTable[Index][0];
  
  if (UsbKeyboardDevice->ShiftOn) {
    
    Key->UnicodeChar = KeyConvertionTable[Index][2];
  
  } else {
    
    Key->UnicodeChar = KeyConvertionTable[Index][1];
  }
  
  if (UsbKeyboardDevice->CapsOn) {
    
    if (Key->UnicodeChar >= 'a' && Key->UnicodeChar <= 'z') {
    
      Key->UnicodeChar = KeyConvertionTable[Index][2];
    
    } else if (Key->UnicodeChar >= 'A' && Key->UnicodeChar <= 'Z') {
    
      Key->UnicodeChar = KeyConvertionTable[Index][1];
    
    }
  }
  
  if (KeyChar >= 0x59 && KeyChar <=0x63 && Key->ScanCode != SCAN_NULL) {
    
    if (UsbKeyboardDevice->NumLockOn && !UsbKeyboardDevice->ShiftOn) {
    
      Key->ScanCode = SCAN_NULL;
    
    } else {
      
      Key->UnicodeChar = 0x00;
    }
  }
  
  if(Key->UnicodeChar == 0 && Key->ScanCode == SCAN_NULL) {
    return EFI_NOT_READY;
  }
  
  return EFI_SUCCESS;    
  
}


EFI_STATUS
InitUSBKeyBuffer(
  IN OUT  USB_KB_BUFFER   *KeyboardBuffer
  )
/*++
  
  Routine Description:
    Resets USB Keyboard Buffer.
    
  Arguments:
    KeyboardBuffer:   Points to the USB Keyboard Buffer.
    
  Returns:
  
--*/  
{
  EfiZeroMem(KeyboardBuffer,sizeof(USB_KB_BUFFER));
  
  KeyboardBuffer->bHead = KeyboardBuffer->bTail;
  
  return EFI_SUCCESS;
}

BOOLEAN
IsUSBKeyboardBufferEmpty(
  IN  USB_KB_BUFFER   KeyboardBuffer
  )
/*++
  
  Routine Description:
    Check whether USB Keyboard buffer is empty.
    
  Arguments:
    KeyboardBuffer:   USB Keyboard Buffer.
    
  Returns:
  
--*/
{
  //
  // meet FIFO empty condition
  //
  return (BOOLEAN)(KeyboardBuffer.bHead == KeyboardBuffer.bTail);
}


BOOLEAN
IsUSBKeyboardBufferFull(
  IN  USB_KB_BUFFER   KeyboardBuffer
  )
/*++
  
  Routine Description:
    Check whether USB Keyboard buffer is full.
    
  Arguments:
    KeyboardBuffer:   USB Keyboard Buffer.
    
  Returns:
  
--*/
{
  return (BOOLEAN)(((KeyboardBuffer.bTail + 1) % (MAX_KEY_ALLOWED + 1)) == 
                                                        KeyboardBuffer.bHead);
}


EFI_STATUS
InsertKeyCode(
  IN OUT  USB_KB_BUFFER *KeyboardBuffer,
  IN      UINT8         Key,
  IN      UINT8         Down
  )
/*++
  
  Routine Description:
    Inserts a key code into keyboard buffer.
    
  Arguments:
    KeyboardBuffer:   Points to the USB Keyboard Buffer.
  
  Returns:
  
--*/
{
  USB_KEY         UsbKey;
  
  //
  // if keyboard buffer is full, throw the 
  // first key out of the keyboard buffer.
  //  
  if (IsUSBKeyboardBufferFull (*KeyboardBuffer)) {
    RemoveKeyCode(KeyboardBuffer,&UsbKey);
  }
    
  KeyboardBuffer->buffer[KeyboardBuffer->bTail].KeyCode = Key;
  KeyboardBuffer->buffer[KeyboardBuffer->bTail].Down = Down;
  
  //
  // adjust the tail pointer of the FIFO keyboard buffer.
  //
  KeyboardBuffer->bTail = (UINT8)((KeyboardBuffer->bTail + 1) 
                                          % (MAX_KEY_ALLOWED + 1)) ;
  
  return EFI_SUCCESS;
}

EFI_STATUS
RemoveKeyCode(
  IN OUT  USB_KB_BUFFER *KeyboardBuffer,
  OUT     USB_KEY       *UsbKey
  )
/*++
  
  Routine Description:
    Pops a key code off from keyboard buffer.
    
  Arguments:
    KeyboardBuffer:   Points to the USB Keyboard Buffer.
    UsbKey:           Points to the buffer that contains a usb key code.
  
  Returns:
  
--*/  
{
  if (IsUSBKeyboardBufferEmpty (*KeyboardBuffer)) {
    return EFI_DEVICE_ERROR;
  }
    
  UsbKey->KeyCode = KeyboardBuffer->buffer[KeyboardBuffer->bHead].KeyCode;
  UsbKey->Down = KeyboardBuffer->buffer[KeyboardBuffer->bHead].Down;
  
  //
  // adjust the head pointer of the FIFO keyboard buffer.
  //
  KeyboardBuffer->bHead = (UINT8)((KeyboardBuffer->bHead + 1) % 
                                                (MAX_KEY_ALLOWED + 1)) ;
  
  return EFI_SUCCESS;
}   

EFI_STATUS
SetKeyLED(
  IN  USB_KB_DEV    *UsbKeyboardDevice
  )
/*++
  
  Routine Description:
    Sets USB Keyboard LED state.
    
  Arguments:
    UsbKeyboardDevice:    The USB_KB_DEV instance.
  
  Returns:
  
--*/  
{
  LED_MAP     Led;
  UINT8       ReportId;
  
  //
  // Set each field in Led map.
  //  
  Led.NumLock = (UINT8)UsbKeyboardDevice->NumLockOn;
  Led.CapsLock = (UINT8)UsbKeyboardDevice->CapsOn;
  Led.Resrvd = 0;    
  
  ReportId = 0;
  //
  // call Set Report Request to lighten the LED.
  //
  UsbSetReportRequest (
    UsbKeyboardDevice->UsbIo,
    UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
    ReportId,
    HID_OUTPUT_REPORT,
    1,
    (CHAR8 *)&Led
    );
    
  return EFI_SUCCESS;
}   

VOID
USBKeyboardRepeatHandler(
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  )
/*++
  
  Routine Description:
    Timer handler for Repeat Key timer.
    
  Arguments:
    Event:    The Repeat Key event.
    Context:  Points to the USB_KB_DEV instance.
    
  Returns:
  
--*/    
{
  USB_KB_DEV  *UsbKeyboardDevice;
  
  UsbKeyboardDevice = (USB_KB_DEV*)Context;
  
  //
  // Do nothing when there is no repeat key.
  //
  if(UsbKeyboardDevice->RepeatKey != 0) {
    //
    // Inserts one Repeat key into keyboard buffer,
    //
    InsertKeyCode(
            &(UsbKeyboardDevice->KeyboardBuffer),
            UsbKeyboardDevice->RepeatKey,
            1
            );
    
    //
    // set repeate rate for repeat key generation.
    //
   gBS->SetTimer(UsbKeyboardDevice->RepeatTimer, 
                  TimerRelative, 
                  USBKBD_REPEAT_RATE
                  ) ; 
    
  }
}