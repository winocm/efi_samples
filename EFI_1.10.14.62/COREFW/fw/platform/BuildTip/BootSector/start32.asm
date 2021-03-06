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
        .stack
        .486p
        .code

FAT_DIRECTORY_ENTRY_SIZE    EQU     020h
FAT_DIRECTORY_ENTRY_SHIFT   EQU     5
BLOCK_SIZE                  EQU     0200h
BLOCK_MASK                  EQU     01ffh
BLOCK_SHIFT                 EQU     9

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

; si      = NumberOfClusters
; cx      = ClusterNumber
; dx      = CachedFatSectorNumber
; ds:0000 = CacheFatSectorBuffer
; es:di   = Buffer to load file
; bx      = NextClusterNumber

        mov     si,0                                ; NumberOfClusters = 0 - Special case for first cluster
        xor     di,di                               ; di = 0
        mov     dx,0fffh                            ; CachedFatSectorNumber = 0xfff
        push    cx                                  ; Push StartCluster onto stack
FatChainLoop:
        mov     ax,cx                               ; ax = ClusterNumber    
        and     ax,0fff8h                           ; ax = ax & 0xfff8
        cmp     ax,0fff8h                           ; See if this is the last cluster
        je      FoundLastCluster                    ; Jump if last cluster found
        mov     ax,cx                               ; ax = ClusterNumber
        shl     ax,2                                ; FatOffset = ClusterNumber * 4
        push    si                                  ; Save si
        mov     si,ax                               ; si = FatOffset
        shr     ax,BLOCK_SHIFT                      ; ax = FatOffset >> BLOCK_SHIFT
        add     ax,word ptr [bp+ReservedSectors]    ; ax = FatSectorNumber = ReservedSectors + (FatOffset >> BLOCK_OFFSET)
        and     si,BLOCK_MASK                       ; si = FatOffset & BLOCK_MASK
        cmp     ax,dx                               ; Compare FatSectorNumber to CachedFatSectorNumber
        je      SkipFatRead
        mov     bx,2                                
        push    es
        push    ds
        pop     es
        call    ReadBlocks                          ; Read 2 blocks starting at AX storing at ES:DI
        pop     es
        mov     dx,ax                               ; CachedFatSectorNumber = FatSectorNumber
SkipFatRead:
        mov     bx,word ptr [si]                    ; bx = NextClusterNumber
        mov     ax,cx                               ; ax = ClusterNumber
        pop     si                                  ; Restore si

        cmp     si,0
        jne     NotFirstCluster
        mov     cx,bx
        pop     bx
        push    cx
        inc     si
        jmp     FatChainLoop
NotFirstCluster:

        dec     bx                                  ; bx = NextClusterNumber - 1
        cmp     bx,cx                               ; See if (NextClusterNumber-1)==ClusterNumber
        jne     ReadClusters
        inc     bx                                  ; bx = NextClusterNumber
        inc     si                                  ; NumberOfClusters++
        mov     cx,bx                               ; ClusterNumber = NextClusterNumber
        jmp     FatChainLoop
ReadClusters:
        inc     bx
        pop     ax                                  ; ax = StartCluster
        push    bx                                  ; StartCluster = NextClusterNumber
        mov     cx,bx                               ; ClusterNumber = NextClusterNumber
        sub     ax,2                                ; ax = StartCluster - 2
        xor     bh,bh                               
        mov     bl,byte ptr [bp+SectorsPerCluster]  ; bx = SectorsPerCluster
        mul     bx                                  ; ax = (StartCluster - 2) * SectorsPerCluster
        add     ax, word ptr [bp]                   ; ax = FirstClusterLBA + (StartCluster-2)*SectorsPerCluster
        push    ax                                  ; save start sector
        mov     ax,si                               ; ax = NumberOfClusters
        mul     bx                                  ; ax = NumberOfClusters * SectorsPerCluster
        mov     bx,ax                               ; bx = Number of Sectors
        pop     ax                                  ; ax = Start Sector
        call    ReadBlocks
        mov     si,1                                ; NumberOfClusters = 1
        jmp     FatChainLoop
FoundLastCluster:
        mov     ax,cs
        mov     word ptr cs:[JumpSegment],ax
JumpFarInstruction:
        db      0eah
JumpOffset:
        dw      0200h
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
        push    ax                                  ; save ax
        mov     ax,es                               ; ax = es
        shr     ax,(BLOCK_SHIFT-4)                  ; ax = Number of blocks into mem system
        and     ax,07fh                             ; ax = Number of blocks into current seg
        add     ax,bx                               ; ax = End Block number of transfer
        cmp     ax,080h                             ; See if it crosses a 64K boundry
        jle     NotCrossing64KBoundry               ; Branch if not crossing 64K boundry
        sub     ax,080h                             ; ax = Number of blocks past 64K boundry
        sub     bx,ax                               ; Decrease transfer size by block overage
NotCrossing64KBoundry:
        pop     ax                                  ; restore ax

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
        jc      DiskError
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

DiskError:
        mov     bx,000fh
        mov     al,ah
        mov     ah,0ah
        add     al,041h
        mov     cx,1
        int     10h
        xor     ah,ah
        int     16h
        int     19h
Halt:
        jmp     Halt


;******************************************************************************
;******************************************************************************
;******************************************************************************

DELAY_PORT           equ     0edh    ; Port to use for 1uS delay
KBD_CONTROL_PORT     equ     060h    ; 8042 control port     
KBD_STATUS_PORT      equ     064h    ; 8042 status port      
WRITE_DATA_PORT_CMD  equ     0d1h    ; 8042 command to write the data port
ENABLE_A20_CMD       equ     0dfh    ; 8042 command to enable A20

        org     200h
start:  
        mov ax,cs
        mov ds,ax
        mov es,ax
        mov ss,ax
        mov sp,MyStack

        mov ax,0b800h
        mov es,ax
        mov byte ptr es:[160],'a'
        mov ax,cs
        mov es,ax

        mov ebx,0
        lea edi,MemoryMap
MemMapLoop:
        mov eax,0e820h
        mov ecx,20
        mov edx,'SMAP'
        int 15h
        jc  MemMapDone
        add edi,20
        cmp ebx,0
        je  MemMapDone
        jmp MemMapLoop
MemMapDone:
        lea eax,MemoryMap
        sub edi,eax                         ; Get the address of the memory map
        mov dword ptr [MemoryMapSize],edi   ; Save the size of the memory map

        xor     ebx,ebx
        mov     bx,cs                       ; BX=segment
        shl     ebx,4                       ; BX="linear" address of segment base
        lea     eax,[GDT_BASE + ebx]        ; EAX=PHYSICAL address of gdt
        mov     dword ptr [gdtr + 2],eax    ; Put address of gdt into the gdtr
        lea     eax,[IDT_BASE + ebx]        ; EAX=PHYSICAL address of idt
        mov     dword ptr [idtr + 2],eax    ; Put address of idt into the idtr
        lea     edx,[MemoryMapSize + ebx]   ; Physical base address of the memory map

        add ebx,01000h                      ; Source of EFI32
        mov dword ptr [JUMP+2],ebx
        add ebx,01000h
        mov esi,ebx                         ; Source of EFILDR32

        mov ax,0b800h
        mov es,ax
        mov byte ptr es:[162],'b'
        mov ax,cs
        mov es,ax

;
; Enable A20 Gate 
;

        mov ax,2401h                        ; Enable A20 Gate
        int 15h
        jnc A20GateEnabled                  ; Jump if it suceeded

;
; If INT 15 Function 2401 is not supported, then attempt to Enable A20 manually.
;

        call    Empty8042InputBuffer        ; Empty the Input Buffer on the 8042 controller
        jnz     Timeout8042                 ; Jump if the 8042 timed out
        out     DELAY_PORT,ax               ; Delay 1 uS
        mov     al,WRITE_DATA_PORT_CMD      ; 8042 cmd to write output port
        out     KBD_STATUS_PORT,al          ; Send command to the 8042
        call    Empty8042InputBuffer        ; Empty the Input Buffer on the 8042 controller
        jnz     Timeout8042                 ; Jump if the 8042 timed out
        mov     al,ENABLE_A20_CMD           ; gate address bit 20 on
        out     KBD_CONTROL_PORT,al         ; Send command to thre 8042
        call    Empty8042InputBuffer        ; Empty the Input Buffer on the 8042 controller
        mov     cx,25                       ; Delay 25 uS for the command to complete on the 8042
Delay25uS:
        out     DELAY_PORT,ax               ; Delay 1 uS
        loop    Delay25uS                       
Timeout8042:


A20GateEnabled:

;
; DISABLE INTERRUPTS - Entering Protected Mode
;

        cli                             

        mov ax,0b800h
        mov es,ax
        mov byte ptr es:[164],'c'
        mov ax,cs
        mov es,ax

        db      66h     
        lgdt    fword ptr [gdtr]
        db      66h     
        lidt    fword ptr [idtr]

        mov     eax,cr0
        or      al,1
        mov     cr0,eax

        mov eax,0008h                       ; Flat data descriptor
        mov ebp,000400000h                  ; Destination of EFILDR32
        mov ebx,000070000h                  ; Length of copy
        
JUMP:
; jmp far 0010:00020000
        db  066h
        db  0eah
        dd  000020000h
        dw  00010h

Empty8042InputBuffer:
        mov cx,0
Empty8042Loop:
        out     DELAY_PORT,ax               ; Delay 1us
        in      al,KBD_STATUS_PORT          ; Read the 8042 Status Port
        and     al,02h                      ; Check the Input Buffer Full Flag
        loopnz  Empty8042Loop               ; Loop until the input buffer is empty or a timout of 65536 uS
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; data
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        align 02h

gdtr    dw GDT_END - GDT_BASE - 1   ; GDT limit
        dd 0                        ; (GDT base gets set above)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   global descriptor table (GDT)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        align 02h

public GDT_BASE
GDT_BASE:
; null descriptor
NULL_SEL            equ $-GDT_BASE
        dw 0            ; limit 15:0
        dw 0            ; base 15:0
        db 0            ; base 23:16
        db 0            ; type
        db 0            ; limit 19:16, flags
        db 0            ; base 31:24

; linear data segment descriptor
LINEAR_SEL      equ $-GDT_BASE
        dw 0FFFFh       ; limit 0xFFFFF
        dw 0            ; base 0
        db 0
        db 092h         ; present, ring 0, data, expand-up, writable
        db 0CFh                 ; page-granular, 32-bit
        db 0

; linear code segment descriptor
LINEAR_CODE_SEL equ $-GDT_BASE
        dw 0FFFFh       ; limit 0xFFFFF
        dw 0            ; base 0
        db 0
        db 09Ah         ; present, ring 0, data, expand-up, writable
        db 0CFh                 ; page-granular, 32-bit
        db 0

; system data segment descriptor
SYS_DATA_SEL    equ $-GDT_BASE
        dw 0FFFFh       ; limit 0xFFFFF
        dw 0            ; base 0
        db 0
        db 092h         ; present, ring 0, data, expand-up, writable
        db 0CFh                 ; page-granular, 32-bit
        db 0

; system code segment descriptor
SYS_CODE_SEL    equ $-GDT_BASE
        dw 0FFFFh       ; limit 0xFFFFF
        dw 0            ; base 0
        db 0
        db 09Ah         ; present, ring 0, data, expand-up, writable
        db 0CFh                 ; page-granular, 32-bit
        db 0

; spare segment descriptor
SPARE3_SEL  equ $-GDT_BASE
        dw 0            ; limit 0xFFFFF
        dw 0            ; base 0
        db 0
        db 0            ; present, ring 0, data, expand-up, writable
        db 0            ; page-granular, 32-bit
        db 0

; spare segment descriptor
SPARE4_SEL  equ $-GDT_BASE
        dw 0            ; limit 0xFFFFF
        dw 0            ; base 0
        db 0
        db 0            ; present, ring 0, data, expand-up, writable
        db 0            ; page-granular, 32-bit
        db 0

; spare segment descriptor
SPARE5_SEL  equ $-GDT_BASE
        dw 0            ; limit 0xFFFFF
        dw 0            ; base 0
        db 0
        db 0            ; present, ring 0, data, expand-up, writable
        db 0            ; page-granular, 32-bit
        db 0

GDT_END:

        align 02h



idtr            dw IDT_END - IDT_BASE - 1   ; IDT limit
        dd 0                        ; (IDT base gets set above)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   interrupt descriptor table (IDT)
;
;   Note: The hardware IRQ's specified in this table are the normal PC/AT IRQ
;       mappings.  This implementation only uses the system timer and all other
;       IRQs will remain masked.  The descriptors for vectors 33+ are provided
;       for convenience.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;idt_tag db "IDT",0     
        align 02h

public IDT_BASE
IDT_BASE:
; divide by zero (INT 0)
DIV_ZERO_SEL        equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; debug exception (INT 1)
DEBUG_EXCEPT_SEL    equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; NMI (INT 2)
NMI_SEL             equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; soft breakpoint (INT 3)
BREAKPOINT_SEL      equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; overflow (INT 4)
OVERFLOW_SEL        equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; bounds check (INT 5)
BOUNDS_CHECK_SEL    equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; invalid opcode (INT 6)
INVALID_OPCODE_SEL  equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; device not available (INT 7)
DEV_NOT_AVAIL_SEL   equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; double fault (INT 8)
DOUBLE_FAULT_SEL    equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; Coprocessor segment overrun - reserved (INT 9)
RSVD_INTR_SEL1      equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; invalid TSS (INT 0ah)
INVALID_TSS_SEL     equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; segment not present (INT 0bh)
SEG_NOT_PRESENT_SEL equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; stack fault (INT 0ch)
STACK_FAULT_SEL     equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; general protection (INT 0dh)
GP_FAULT_SEL        equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; page fault (INT 0eh)
PAGE_FAULT_SEL      equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; Intel reserved - do not use (INT 0fh)
RSVD_INTR_SEL2      equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; floating point error (INT 10h)
FLT_POINT_ERR_SEL   equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; alignment check (INT 11h)
ALIGNMENT_CHECK_SEL equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; machine check (INT 12h)
MACHINE_CHECK_SEL   equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; 86 unspecified descriptors, First 13 of them are reserved, the rest are avail
        db (86 * 8) dup(0)
        
; IRQ 0 (System timer) - (INT 68h)
IRQ0_SEL            equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; IRQ 1 (8042 Keyboard controller) - (INT 69h)
IRQ1_SEL            equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; Reserved - IRQ 2 redirect (IRQ 2) - DO NOT USE!!! - (INT 6ah)
IRQ2_SEL            equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; IRQ 3 (COM 2) - (INT 6bh)
IRQ3_SEL            equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; IRQ 4 (COM 1) - (INT 6ch)
IRQ4_SEL            equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; IRQ 5 (LPT 2) - (INT 6dh)
IRQ5_SEL            equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; IRQ 6 (Floppy controller) - (INT 6eh)
IRQ6_SEL            equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; IRQ 7 (LPT 1) - (INT 6fh)
IRQ7_SEL            equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; IRQ 8 (RTC Alarm) - (INT 70h)
IRQ8_SEL            equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; IRQ 9 - (INT 71h)
IRQ9_SEL            equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; IRQ 10 - (INT 72h)
IRQ10_SEL            equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; IRQ 11 - (INT 73h)
IRQ11_SEL            equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; IRQ 12 (PS/2 mouse) - (INT 74h)
IRQ12_SEL            equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; IRQ 13 (Floating point error) - (INT 75h)
IRQ13_SEL            equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; IRQ 14 (Secondary IDE) - (INT 76h)
IRQ14_SEL            equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

; IRQ 15 (Primary IDE) - (INT 77h)
IRQ15_SEL            equ $-IDT_BASE
        dw 0            ; offset 15:0
        dw SYS_CODE_SEL ; selector 15:0
        db 0            ; 0 for interrupt gate
        db 0eh OR 80h   ; (10001110)type = 386 interrupt gate, present
        dw 0            ; offset 31:16

IDT_END:

        align 02h

MemoryMapSize   dd  0
MemoryMap   dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0

        dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

        org 0fe0h
MyStack:    
        ; below is the pieces of the IVT that is used to redirect INT 68h - 6fh
        ;    back to INT 08h - 0fh  when in real mode...  It is 'org'ed to a
        ;    known low address (20f00) so it can be set up by PlMapIrqToVect in
        ;    8259.c
                
        int 8
        iret
        
        int 9
        iret
        
        int 10
        iret
        
        int 11
        iret
        
        int 12
        iret
        
        int 13
        iret
        
        int 14
        iret
        
        int 15
        iret
        
        
        org 0ffeh
BlockSignature:
        dw  0aa55h

        end 
