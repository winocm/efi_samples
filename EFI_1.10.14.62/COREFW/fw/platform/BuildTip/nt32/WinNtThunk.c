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

    WinNtThunk.c

Abstract:

    Build WinNtThunk instance for all WinNt Drivers

Revision History

--*/

#include "ntemul.h"

#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (WinNtThunk)

typedef struct {
  VENDOR_DEVICE_PATH            Vendor;
  EFI_DEVICE_PATH_PROTOCOL      EndDevicePath;
} WIN_NT_THUNK_DEVICE_PATH;

static WIN_NT_THUNK_DEVICE_PATH mWinNtThunkDevicePath = {
  {
    HARDWARE_DEVICE_PATH,
    HW_VENDOR_DP,
    (UINT8) (sizeof(VENDOR_DEVICE_PATH)),
    (UINT8) ((sizeof(VENDOR_DEVICE_PATH)) >> 8),
    EFI_WIN_NT_THUNK_PROTOCOL_GUID, 
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    END_DEVICE_PATH_LENGTH,
    0
  }
};

EFI_WIN_NT_THUNK_PROTOCOL  mWinNtThunkTable;

EFI_STATUS
PlInitializeWinNtThunkTable (
  )

{
  EFI_STATUS  Status;
  EFI_HANDLE  Handle;

  //
  // Initialize table header
  //
  mWinNtThunkTable.Signature = EFI_WIN_NT_THUNK_PROTOCOL_SIGNATURE;

  //
  // Win32 Process APIs
  //
  mWinNtThunkTable.GetCurrentProcess                = GetCurrentProcess;
  mWinNtThunkTable.GetCurrentThread                 = GetCurrentThread;
  mWinNtThunkTable.GetCurrentThreadId               = GetCurrentThreadId;
  mWinNtThunkTable.GetProcAddress                   = GetProcAddress;
  mWinNtThunkTable.GetTickCount                     = GetTickCount;
  mWinNtThunkTable.LoadLibraryEx                    = LoadLibraryEx;

  mWinNtThunkTable.SuspendThread                    = SuspendThread;
  mWinNtThunkTable.CreateThread                     = CreateThread;
  mWinNtThunkTable.TerminateThread                  = TerminateThread;
  mWinNtThunkTable.SendMessage                      = SendMessage;
  mWinNtThunkTable.ExitThread                       = ExitThread;
  mWinNtThunkTable.ResumeThread                     = ResumeThread;
  mWinNtThunkTable.DuplicateHandle                  = DuplicateHandle;

  mWinNtThunkTable.SetPriorityClass                 = SetPriorityClass;
  mWinNtThunkTable.SetThreadPriority                = SetThreadPriority;
  mWinNtThunkTable.Sleep                            = Sleep;

  mWinNtThunkTable.InitializeCriticalSection        = InitializeCriticalSection;
  mWinNtThunkTable.EnterCriticalSection             = EnterCriticalSection;
  mWinNtThunkTable.LeaveCriticalSection             = LeaveCriticalSection;
  mWinNtThunkTable.DeleteCriticalSection            = DeleteCriticalSection;
  mWinNtThunkTable.TlsAlloc                         = TlsAlloc;
  mWinNtThunkTable.TlsFree                          = TlsFree;
  mWinNtThunkTable.TlsSetValue                      = TlsSetValue;
  mWinNtThunkTable.TlsGetValue                      = TlsGetValue;
  mWinNtThunkTable.CreateSemaphore                  = CreateSemaphore;
  mWinNtThunkTable.WaitForSingleObject              = WaitForSingleObject;
  mWinNtThunkTable.ReleaseSemaphore                 = ReleaseSemaphore;


  //
  // Win32 Console APIs
  //
  mWinNtThunkTable.CreateConsoleScreenBuffer        = CreateConsoleScreenBuffer;
  mWinNtThunkTable.FillConsoleOutputAttribute       = FillConsoleOutputAttribute;
  mWinNtThunkTable.FillConsoleOutputCharacter       = FillConsoleOutputCharacter;
  mWinNtThunkTable.GetConsoleCursorInfo             = GetConsoleCursorInfo;
  mWinNtThunkTable.GetNumberOfConsoleInputEvents    = GetNumberOfConsoleInputEvents;
  mWinNtThunkTable.PeekConsoleInput                 = PeekConsoleInput;
  mWinNtThunkTable.ScrollConsoleScreenBuffer        = ScrollConsoleScreenBuffer;
  mWinNtThunkTable.ReadConsoleInput                 = ReadConsoleInput;

  mWinNtThunkTable.SetConsoleActiveScreenBuffer     = SetConsoleActiveScreenBuffer;
  mWinNtThunkTable.SetConsoleCursorInfo             = SetConsoleCursorInfo;
  mWinNtThunkTable.SetConsoleCursorPosition         = SetConsoleCursorPosition;
  mWinNtThunkTable.SetConsoleScreenBufferSize       = SetConsoleScreenBufferSize;
  mWinNtThunkTable.SetConsoleTitleW                 = SetConsoleTitleW;
  mWinNtThunkTable.WriteConsoleInput                = WriteConsoleInput;
  mWinNtThunkTable.WriteConsoleOutput               = WriteConsoleOutput;

  //
  // Win32 File APIs
  //
  mWinNtThunkTable.CreateFile                       = CreateFile;
  mWinNtThunkTable.CreateFileMapping                = CreateFileMapping;
  mWinNtThunkTable.CloseHandle                      = CloseHandle;
  mWinNtThunkTable.DeleteFile                       = DeleteFile;
  mWinNtThunkTable.FindFirstFile                    = FindFirstFile;
  mWinNtThunkTable.FindNextFile                     = FindNextFile;
  mWinNtThunkTable.FindClose                        = FindClose;
  mWinNtThunkTable.FlushFileBuffers                 = FlushFileBuffers;
  mWinNtThunkTable.GetEnvironmentVariable           = GetEnvironmentVariable;
  mWinNtThunkTable.GetLastError                     = GetLastError;
  mWinNtThunkTable.SetErrorMode                     = SetErrorMode;
  mWinNtThunkTable.GetStdHandle                     = GetStdHandle;
  mWinNtThunkTable.MapViewOfFileEx                  = MapViewOfFileEx;
  mWinNtThunkTable.ReadFile                         = ReadFile;
  mWinNtThunkTable.SetEndOfFile                     = SetEndOfFile;
  mWinNtThunkTable.SetFilePointer                   = SetFilePointer;
  mWinNtThunkTable.WriteFile                        = WriteFile;
  mWinNtThunkTable.GetFileInformationByHandle       = GetFileInformationByHandle;
  mWinNtThunkTable.GetDiskFreeSpace                 = GetDiskFreeSpace;
  mWinNtThunkTable.GetDiskFreeSpaceEx               = GetDiskFreeSpaceEx;
  mWinNtThunkTable.MoveFile                         = MoveFile;


  //
  // Win32 Time APIs
  //
  mWinNtThunkTable.FileTimeToLocalFileTime          = FileTimeToLocalFileTime;
  mWinNtThunkTable.FileTimeToSystemTime             = FileTimeToSystemTime;
  mWinNtThunkTable.GetSystemTime                    = GetSystemTime;
  mWinNtThunkTable.SetSystemTime                    = SetSystemTime;
  mWinNtThunkTable.GetLocalTime                     = GetLocalTime;
  mWinNtThunkTable.SetLocalTime                     = SetLocalTime;
  mWinNtThunkTable.GetTimeZoneInformation           = GetTimeZoneInformation;

  //
  // Win32 Serial APIs
  //
  mWinNtThunkTable.ClearCommError                   = ClearCommError;
  mWinNtThunkTable.EscapeCommFunction               = EscapeCommFunction;
  mWinNtThunkTable.GetCommModemStatus               = GetCommModemStatus;
  mWinNtThunkTable.GetCommState                     = GetCommState;
  mWinNtThunkTable.SetCommState                     = SetCommState;
  mWinNtThunkTable.PurgeComm                        = PurgeComm;
  mWinNtThunkTable.SetCommTimeouts                  = SetCommTimeouts;

  mWinNtThunkTable.ExitProcess                      = ExitProcess;

  mWinNtThunkTable.SPrintf                          = swprintf;
  
  mWinNtThunkTable.GetDesktopWindow                 = GetDesktopWindow;
  mWinNtThunkTable.GetForegroundWindow              = GetForegroundWindow;
  mWinNtThunkTable.CreateWindowEx                   = CreateWindowEx;
  mWinNtThunkTable.ShowWindow                       = ShowWindow;
  mWinNtThunkTable.UpdateWindow                     = UpdateWindow;
  mWinNtThunkTable.InvalidateRect                   = InvalidateRect;
  mWinNtThunkTable.DestroyWindow                    = DestroyWindow;
  mWinNtThunkTable.GetWindowDC                      = GetWindowDC;
  mWinNtThunkTable.InvalidateRect                   = InvalidateRect;
  mWinNtThunkTable.GetClientRect                    = GetClientRect;
  mWinNtThunkTable.AdjustWindowRect                 = AdjustWindowRect;
  mWinNtThunkTable.SetDIBitsToDevice                = SetDIBitsToDevice;
  mWinNtThunkTable.BitBlt                           = BitBlt;
  mWinNtThunkTable.GetDC                            = GetDC;
  mWinNtThunkTable.ReleaseDC                        = ReleaseDC;
  mWinNtThunkTable.RegisterClassEx                  = RegisterClassEx;
  mWinNtThunkTable.UnregisterClass                  = UnregisterClass;

  mWinNtThunkTable.BeginPaint                       = BeginPaint;
  mWinNtThunkTable.EndPaint                         = EndPaint;
  mWinNtThunkTable.PostQuitMessage                  = PostQuitMessage;
  mWinNtThunkTable.DefWindowProc                    = DefWindowProc;
  mWinNtThunkTable.LoadIcon                         = LoadIcon;
  mWinNtThunkTable.LoadCursor                       = LoadCursor;
  mWinNtThunkTable.GetStockObject                   = GetStockObject;
  mWinNtThunkTable.SetViewportOrgEx                 = SetViewportOrgEx;
  mWinNtThunkTable.SetWindowOrgEx                   = SetWindowOrgEx;

  mWinNtThunkTable.GetMessage                       = GetMessage;
  mWinNtThunkTable.TranslateMessage                 = TranslateMessage;
  mWinNtThunkTable.DispatchMessage                  = DispatchMessage;

  mWinNtThunkTable.GetProcessHeap                   = GetProcessHeap;
  mWinNtThunkTable.HeapAlloc                        = HeapAlloc;
  mWinNtThunkTable.HeapFree                         = HeapFree;

  Handle = NULL;
  Status = BS->InstallMultipleProtocolInterfaces (
                 &Handle,
                 &gEfiDevicePathProtocolGuid, &mWinNtThunkDevicePath,
                 &gEfiWinNtThunkProtocolGuid, &mWinNtThunkTable,
                 NULL
                 );

  return Status;
}

