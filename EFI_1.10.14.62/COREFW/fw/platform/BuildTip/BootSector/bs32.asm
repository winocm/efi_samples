;******************************************************************************
;*
;* Copyright (c)  2003 Intel Corporation. All rights reserved
;* This software and associated documentation (if any) is furnished
;* under a license and may only be used or copied in accordance
;* with the terms of the license. Except as permitted by such
;* license, no part of this software or documentation may be
;* reproduced, stored in a retrieval system, or transmitted in any
;* form or by any means without the express written consent of
;* Intel Corporation.
;*
;******************************************************************************

    .model  small
;   .dosseg
    .stack
    .486p
    .code

FAT_DIRECTORY_ENTRY_SIZE  EQU     020h
FAT_DIRECTORY_ENTRY_SHIFT EQU     5
BLOCK_SIZE                EQU     0200h
BLOCK_MASK                EQU     01ffh
BLOCK_SHIFT               EQU     9

;******************************************************************************
; Boot Sector and BPB (BIOS Parameter Block) structure
;******************************************************************************

        org 0h
Ia32Jump:
  jmp   BootSectorEntryPoint  ; JMP inst    - 3 bytes
  nop

OemId               db  "INTEL   "    ; OemId                           - 8 bytes
SectorSize          dw  0200h         ; Sector Size                     - 2 bytes
SectorsPerCluster   db  10h           ; Sector Per Cluster              - 1 byte
ReservedSectors     dw  0002h         ; Reserved Sectors                - 2 bytes
NoFats              db  02h           ; Number of FATs                  - 1 byte
RootEntries         dw  0000h         ; Root Entries                    - 2 bytes
Sectors             dw  0000h         ; Number of Sectors               - 2 bytes
Media               db  0f8h          ; Media                           - 1 byte
SectorsPerFat16     dw  0000h         ; Sectors Per FAT for FAT12/FAT16 - 2 byte
SectorsPerTrack     dw  0000h         ; Sectors Per Track               - 2 bytes
Heads               dw  0000h         ; Heads                           - 2 bytes
HiddenSectors       dd  00000000h     ; Hidden Sectors                  - 4 bytes
LargeSectors        dd  00000000h     ; Large Sectors                   - 4 bytes

;******************************************************************************
;
;The structure for FAT32 starting at offset 36 of the boot sector. (At this point, 
;the BPB/boot sector for FAT12 and FAT16 differs from the BPB/boot sector for FAT32.)
;
;******************************************************************************

SectorsPerFat32     dd  00000000h     ; Sectors Per FAT for FAT32       - 4 bytes
ExtFlags            dw  0000h         ; Mirror Flag                     - 2 bytes
FSVersion           dw  0000h         ; File System Version             - 2 bytes
RootCluster         dd  00000002h     ; 1st Cluster Number of Root Dir  - 4 bytes
FSInfo              dw  0001h         ; Sector Number of FSINFO         - 2 bytes
BkBootSector        dw  0006h         ; Sector Number of Bk BootSector  - 2 bytes
Reserved            db  12 dup(0)     ; Reserved Field                  - 12 bytes
PhysicalDrive       db  80h           ; Physical Drive Number           - 1 byte
Reserved1           db  00h           ; Reserved Field                  - 1 byte
Signature           db  29h           ; Extended Boot Signature         - 1 byte
VolId               db  "    "        ; Volume Serial Number            - 4 bytes
FatLabel            db  "EFI FAT32  " ; Volume Label                    - 11 bytes
FileSystemType      db  "FAT32   "    ; File System Type                - 8 bytes

BootSectorEntryPoint:
        ASSUME  ds:@code
        ASSUME  ss:@code

  mov   ax,cs         ; ax = 0
  mov   ss,ax         ; ss = 0
  add   ax,1000h
  mov   ds,ax

  mov   sp,07c00h     ; sp = 0x7c00
  mov   bp,sp         ; bp = 0x7c00

  mov   ah,8                                ; ah = 8 - Get Drive Parameters Function
  mov   dl,byte ptr [bp+PhysicalDrive]      ; dl = Drive Number
  int   13h                                 ; Get Drive Parameters
  xor   ax,ax                   ; ax = 0
  mov   al,dh                   ; al = dh
  inc   al                      ; MaxHead = al + 1
  push  ax                      ; 0000:7bfe = MaxHead
  mov   al,cl                   ; al = cl
  and   al,03fh                 ; MaxSector = al & 0x3f
  push  ax                      ; 0000:7bfc = MaxSector

  cmp   word ptr [bp+SectorSignature],0aa55h  ; Verify Boot Sector Signature
  jne   BadBootSector
  cmp   byte ptr [bp+Ia32Jump],0ebh       ; Verify that first byte is a jump inst
  jne   BadBootSector
  cmp   word ptr [bp+SectorSize],00200h     ; Verify Block Size == 0x200
  jne   BadBootSector
  cmp   byte ptr [bp+SectorsPerCluster],000h  ; Verify Sectors Per Cluster != 0
  je    BadBootSector


  cmp   word ptr [bp+FileSystemType],04146h   ; Check for "FA"
  jne   BadBootSector
  cmp   byte ptr [bp+FileSystemType+2],'T'
  jne   BadBootSector
  mov   ax,02020h
  cmp   word ptr [bp+FileSystemType+5],ax
  jne   BadBootSector
  cmp   word ptr [bp+FileSystemType+6],ax
  jne   BadBootSector
  cmp   word ptr [bp+FileSystemType+3],03233h   ; Check for "32"
  je    Fat32Found
  cmp   word ptr [bp+FileSystemType+3],03631h   ; Check for "16"
  je    Fat16Found
  cmp   word ptr [bp+FileSystemType+3],03231h   ; Check for "12"
  je    Fat12Found
  jmp   BadBootSector
Fat32Found:
  mov   cx,word ptr [bp+RootEntries]      ; cx = RootEntries
  shl   cx,FAT_DIRECTORY_ENTRY_SHIFT      ; cx = cx * 32 = cx * sizeof(FAT_DIRECTORY_ENTRY) = Size of Root Directory in bytes
  mov   bx,cx                             ; bx = size of the Root Directory in bytes
  and   bx,BLOCK_MASK                     ; See if it is an even number of sectors long
  jne   BadBootSector                     ; If is isn't, then the boot sector is bad.
  mov   bx,cx                             ; bx = size of the Root Directory in bytes
  shr   bx,BLOCK_SHIFT                    ; bx = size of Root Directory in sectors
  mov   al,byte ptr [bp+NoFats]           ; al = NoFats
  xor   ah,ah                             ; ah = 0  ==> ax = NoFats
  mul   word ptr [bp+SectorsPerFat32]     ; ax = NoFats * SectorsPerFat
  add   ax,word ptr [bp+ReservedSectors]  ; ax = NoFats * SectorsPerFat + ReservedSectors = RootLBA
  add   ax,bx                             ; ax = NoFats * SectorsPerFat + ReservedSectors + RootDirSectors = FirstClusterLBA
  mov   word ptr [bp],ax                  ; Save FirstClusterLBA for later use
  
  mov   ax,word ptr [bp+RootCluster]        ; ax = StartCluster of Root Directory
  sub   ax,2                                ; ax = StartCluster - 2
  xor   bh,bh                               
  mov   bl,byte ptr [bp+SectorsPerCluster]  ; bx = SectorsPerCluster
  mul   bx                                  ; ax = (StartCluster - 2) * SectorsPerCluster
  add   ax, word ptr [bp]                   ; ax = FirstClusterLBA + (StartCluster-2)*SectorsPerCluster
  push  ds
  pop   es
  push  es
  mov   di,0                                ; Store directory in es:di = 1000:0000
  call  ReadBlocks                          ; Read StartCluster of Root Directory
  pop   es

FindEFILDR:
  cmp   word ptr [di],04645h          ; Compare to "EF"
  jne   NotMatchingEFILDR
  cmp   word ptr [di+2],04c49h        ; Compare to "IL"
  jne   NotMatchingEFILDR
  cmp   word ptr [di+4],05244h        ; Compare to "DR"
  jne   NotMatchingEFILDR
  cmp   word ptr [di+6],03032h        ; Compare to "20"
  jne   NotMatchingEFILDR
  mov   ax,02020h                     ; ax = "  "
  cmp   word ptr [di+8],ax            ; Compare to "  "
  jne   NotMatchingEFILDR
  cmp   word ptr [di+9],ax            ; Compare to "  "
  jne   NotMatchingEFILDR
  mov   al, byte ptr [di+11]
  and   al,058h
  je    FoundEFILDR
NotMatchingEFILDR:
  add   di,FAT_DIRECTORY_ENTRY_SIZE       ; Increment di
  sub   cx,FAT_DIRECTORY_ENTRY_SIZE       ; Decrement cx
  jne   FindEFILDR
  jmp   NotFoundEFILDR

FoundEFILDR:
    mov     cx, word ptr [di+26]                ; cx = FileCluster for EFILDR
    mov     ax,cs                               ; Destination = 2000:0000
    add     ax,2000h
    mov     es,ax
    xor     di,di
ReadFirstClusterOfEFILDR:
    mov     ax,cx                               ; ax = StartCluster
    sub     ax,2                                ; ax = StartCluster - 2
    xor     bh,bh                               
    mov     bl,byte ptr [bp+SectorsPerCluster]  ; bx = SectorsPerCluster
    mul     bx                                  ; ax = (StartCluster - 2) * SectorsPerCluster
    add     ax, word ptr [bp]                   ; ax = FirstClusterLBA + (StartCluster-2)*SectorsPerCluster
    xor     bh,bh
    mov     bl,byte ptr [bp+SectorsPerCluster]  ; bx = Number of Sectors in a cluster
    push    es
    call    ReadBlocks
    pop     ax
JumpIntoFirstSectorOfEFILDR:
    mov     word ptr [bp+JumpSegment],ax
JumpFarInstruction:
    db      0eah
JumpOffset:
    dw      0000h
JumpSegment:
    dw      2000h

; ****************************************************************************
; ReadBlocks - Reads a set of blocks from a block device
;
; AX    = Start LBA
; BX    = Number of Blocks to Read
; ES:DI = Buffer to store sectors read from disk
; ****************************************************************************

; cx = Blocks
; bx = NumberOfBlocks
; si = StartLBA

ReadBlocks:
    pusha
    add     eax,dword ptr [bp+HiddenSectors]    ; Add HiddenSectors to Start LBA
    mov     esi,eax                             ; esi = Start LBA
    mov     cx,bx                               ; cx = Number of blocks to read
ReadCylinderLoop:
    mov     bp,07bfch                           ; bp = 0x7bfc
    mov     eax,esi                             ; eax = Start LBA
    xor     dx,dx                               ; dx = 0
    movzx   ebx,word ptr [bp]                   ; bx = MaxSector
    div     ebx                                 ; ax = StartLBA / MaxSector
    inc     dx                                  ; dx = (StartLBA % MaxSector) + 1

    sub     bx,dx                               ; bx = MaxSector - Sector
    inc     bx                                  ; bx = MaxSector - Sector + 1
    cmp     cx,bx                               ; Compare (Blocks) to (MaxSector - Sector + 1)
    jg      LimitTransfer
    mov     bx,cx                               ; bx = Blocks
LimitTransfer:
    push    cx
    mov     cl,dl                               ; cl = (StartLBA % MaxSector) + 1 = Sector
    xor     dx,dx                               ; dx = 0
    div     word ptr [bp+2]                     ; ax = ax / (MaxHead + 1) = Cylinder  
                                                ; dx = ax % (MaxHead + 1) = Head

    push    bx                                  ; Save number of blocks to transfer
    mov     dh,dl                               ; dh = Head
    mov     bp,07c00h                           ; bp = 0x7c00
    mov     dl,byte ptr [bp+PhysicalDrive]      ; dl = Drive Number
    mov     ch,al                               ; ch = Cylinder
    mov     al,bl                               ; al = Blocks
    mov     ah,2                                ; ah = Function 2
    mov     bx,di                               ; es:bx = Buffer address
    int     013h
    pop     bx
    pop     cx
    movzx   ebx,bx
    add     esi,ebx                             ; StartLBA = StartLBA + NumberOfBlocks
    sub     cx,bx                               ; Blocks = Blocks - NumberOfBlocks
    mov     ax,es
    shl     bx,(BLOCK_SHIFT-4)
    add     ax,bx
    mov     es,ax                               ; es:di = es:di + NumberOfBlocks*BLOCK_SIZE
    cmp     cx,0
    jne     ReadCylinderLoop
    popa
    ret

; ****************************************************************************
; ERROR Condition:
; ****************************************************************************

BadBootSector:
Fat16Found:
Fat12Found:
NotFoundEFILDR:
    int     3
  jmp   BadBootSector

; ****************************************************************************
; Sector Signature
; ****************************************************************************

  org 01feh
SectorSignature:
  dw        0aa55h      ; Boot Sector Signature

  end 
  
