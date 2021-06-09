# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=isr_icl - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to isr_icl - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "isr_icl - Win32 Release" && "$(CFG)" !=\
 "isr_icl - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "isr_icl.mak" CFG="isr_icl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "isr_icl - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "isr_icl - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "isr_icl - Win32 Debug"
CPP=cl.exe

!IF  "$(CFG)" == "isr_icl - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "isrRelease"
# PROP Intermediate_Dir "isrRelease"
# PROP Target_Dir ""
OUTDIR=.\isrRelease
INTDIR=.\isrRelease

ALL : "$(OUTDIR)\icl.lib" "..\build\release\lib\icl.lib"

CLEAN : 
	-@erase "$(INTDIR)\add.obj"
	-@erase "$(INTDIR)\bytes.obj"
	-@erase "$(INTDIR)\compare.obj"
	-@erase "$(INTDIR)\div.obj"
	-@erase "$(INTDIR)\eea.obj"
	-@erase "$(INTDIR)\modexp.obj"
	-@erase "$(INTDIR)\modinv.obj"
	-@erase "$(INTDIR)\modmul.obj"
	-@erase "$(INTDIR)\monpro.obj"
	-@erase "$(INTDIR)\monsqu.obj"
	-@erase "$(INTDIR)\move.obj"
	-@erase "$(INTDIR)\mul.obj"
	-@erase "$(INTDIR)\n0prime.obj"
	-@erase "$(INTDIR)\rem.obj"
	-@erase "$(INTDIR)\shabegin.obj"
	-@erase "$(INTDIR)\shades.obj"
	-@erase "$(INTDIR)\shaend.obj"
	-@erase "$(INTDIR)\shaproc.obj"
	-@erase "$(INTDIR)\shatrans.obj"
	-@erase "$(INTDIR)\subtract.obj"
	-@erase "$(INTDIR)\verifymd.obj"
	-@erase "$(OUTDIR)\icl.lib"
	-@erase "..\build\release\lib\icl.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c 
# ADD CPP /nologo /G5 /W4 /GX /O1 /I ".\Include" /D "NDEBUG" /D "SPACE_REDUCED" /D "OASIS" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /G5 /ML /W4 /GX /O1 /I ".\Include" /D "NDEBUG" /D\
 "SPACE_REDUCED" /D "OASIS" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\isrRelease/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/isr_icl.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"isrRelease\icl.lib"
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/icl.lib" 
LIB32_OBJS= \
	"$(INTDIR)\add.obj" \
	"$(INTDIR)\bytes.obj" \
	"$(INTDIR)\compare.obj" \
	"$(INTDIR)\div.obj" \
	"$(INTDIR)\eea.obj" \
	"$(INTDIR)\modexp.obj" \
	"$(INTDIR)\modinv.obj" \
	"$(INTDIR)\modmul.obj" \
	"$(INTDIR)\monpro.obj" \
	"$(INTDIR)\monsqu.obj" \
	"$(INTDIR)\move.obj" \
	"$(INTDIR)\mul.obj" \
	"$(INTDIR)\n0prime.obj" \
	"$(INTDIR)\rem.obj" \
	"$(INTDIR)\shabegin.obj" \
	"$(INTDIR)\shades.obj" \
	"$(INTDIR)\shaend.obj" \
	"$(INTDIR)\shaproc.obj" \
	"$(INTDIR)\shatrans.obj" \
	"$(INTDIR)\subtract.obj" \
	"$(INTDIR)\verifymd.obj"  \
        #"C:\MSDEV\lib\libc.lib"

"$(OUTDIR)\icl.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

# Begin Custom Build
InputPath=.\isrRelease\icl.lib
SOURCE=$(InputPath)

"..\build\release\lib\icl.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   copy           .\isrRelease\icl.lib           ..\build\release\lib\ 

# End Custom Build

!ELSEIF  "$(CFG)" == "isr_icl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "isrDebug"
# PROP Intermediate_Dir "isrDebug"
# PROP Target_Dir ""
OUTDIR=.\isrDebug
INTDIR=.\isrDebug

ALL : "$(OUTDIR)\icl.lib" "..\build\debug\lib\icl.lib"

CLEAN : 
	-@erase "$(INTDIR)\add.obj"
	-@erase "$(INTDIR)\bytes.obj"
	-@erase "$(INTDIR)\compare.obj"
	-@erase "$(INTDIR)\div.obj"
	-@erase "$(INTDIR)\eea.obj"
	-@erase "$(INTDIR)\modexp.obj"
	-@erase "$(INTDIR)\modinv.obj"
	-@erase "$(INTDIR)\modmul.obj"
	-@erase "$(INTDIR)\monpro.obj"
	-@erase "$(INTDIR)\monsqu.obj"
	-@erase "$(INTDIR)\move.obj"
	-@erase "$(INTDIR)\mul.obj"
	-@erase "$(INTDIR)\n0prime.obj"
	-@erase "$(INTDIR)\rem.obj"
	-@erase "$(INTDIR)\shabegin.obj"
	-@erase "$(INTDIR)\shades.obj"
	-@erase "$(INTDIR)\shaend.obj"
	-@erase "$(INTDIR)\shaproc.obj"
	-@erase "$(INTDIR)\shatrans.obj"
	-@erase "$(INTDIR)\subtract.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\verifymd.obj"
	-@erase "$(OUTDIR)\icl.lib"
	-@erase "..\build\debug\lib\icl.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c 
# ADD CPP /nologo /G5 /W4 /Gm /GX /Zi /Od /I ".\Include" /D "_DEBUG" /D "SPACE_REDUCED" /D "OASIS" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /G5 /MLd /W4 /Gm /GX /Zi /Od /I ".\Include" /D "_DEBUG" /D\
 "SPACE_REDUCED" /D "OASIS" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\isrDebug/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/isr_icl.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"isrDebug\icl.lib"
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/icl.lib" 
LIB32_OBJS= \
	"$(INTDIR)\add.obj" \
	"$(INTDIR)\bytes.obj" \
	"$(INTDIR)\compare.obj" \
	"$(INTDIR)\div.obj" \
	"$(INTDIR)\eea.obj" \
	"$(INTDIR)\modexp.obj" \
	"$(INTDIR)\modinv.obj" \
	"$(INTDIR)\modmul.obj" \
	"$(INTDIR)\monpro.obj" \
	"$(INTDIR)\monsqu.obj" \
	"$(INTDIR)\move.obj" \
	"$(INTDIR)\mul.obj" \
	"$(INTDIR)\n0prime.obj" \
	"$(INTDIR)\rem.obj" \
	"$(INTDIR)\shabegin.obj" \
	"$(INTDIR)\shades.obj" \
	"$(INTDIR)\shaend.obj" \
	"$(INTDIR)\shaproc.obj" \
	"$(INTDIR)\shatrans.obj" \
	"$(INTDIR)\subtract.obj" \
	"$(INTDIR)\verifymd.obj" \
        #"C:\MSDEV\lib\libc.lib"

"$(OUTDIR)\icl.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

# Begin Custom Build
InputPath=.\isrDebug\icl.lib
SOURCE=$(InputPath)

"..\build\debug\lib\icl.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   copy           .\isrDebug\icl.lib          ..\build\debug\lib\ 

# End Custom Build

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "isr_icl - Win32 Release"
# Name "isr_icl - Win32 Debug"

!IF  "$(CFG)" == "isr_icl - Win32 Release"

!ELSEIF  "$(CFG)" == "isr_icl - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\C\iclproc\move.c
DEP_CPP_MOVE_=\
	".\include\iclproc.h"\
	".\include\rsa.h"\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\move.obj" : $(SOURCE) $(DEP_CPP_MOVE_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\sha\shatrans.c

"$(INTDIR)\shatrans.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\sha\shaend.c
DEP_CPP_SHAEN=\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\shaend.obj" : $(SOURCE) $(DEP_CPP_SHAEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\sha\shaproc.c
DEP_CPP_SHAPR=\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\shaproc.obj" : $(SOURCE) $(DEP_CPP_SHAPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\sha\shabegin.c
DEP_CPP_SHABE=\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\shabegin.obj" : $(SOURCE) $(DEP_CPP_SHABE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\pbe\shades.c
DEP_CPP_SHADE=\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\shades.obj" : $(SOURCE) $(DEP_CPP_SHADE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\dss\verifymd.c
DEP_CPP_VERIF=\
	".\include\iclproc.h"\
	".\include\rsa.h"\
	".\include\rsakg.h"\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\verifymd.obj" : $(SOURCE) $(DEP_CPP_VERIF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\rsa1024\compare.c
DEP_CPP_COMPA=\
	".\include\rsa.h"\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\compare.obj" : $(SOURCE) $(DEP_CPP_COMPA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\rsa1024\rem.c
DEP_CPP_REM_C=\
	".\include\rsa.h"\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\rem.obj" : $(SOURCE) $(DEP_CPP_REM_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\rsa1024\modmul.c
DEP_CPP_MODMU=\
	".\include\rsa.h"\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\modmul.obj" : $(SOURCE) $(DEP_CPP_MODMU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\rsa1024\modexp.c
DEP_CPP_MODEX=\
	".\include\rsa.h"\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\modexp.obj" : $(SOURCE) $(DEP_CPP_MODEX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\iclproc\bytes.c
DEP_CPP_BYTES=\
	".\include\iclproc.h"\
	".\include\rsa.h"\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\bytes.obj" : $(SOURCE) $(DEP_CPP_BYTES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\rsakg\modinv.c
DEP_CPP_MODIN=\
	".\include\iclproc.h"\
	".\include\rsa.h"\
	".\include\rsakg.h"\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\modinv.obj" : $(SOURCE) $(DEP_CPP_MODIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\rsa1024\subtract.c
DEP_CPP_SUBTR=\
	".\include\rsa.h"\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\subtract.obj" : $(SOURCE) $(DEP_CPP_SUBTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\rsa1024\monpro.c
DEP_CPP_MONPR=\
	".\include\rsa.h"\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\monpro.obj" : $(SOURCE) $(DEP_CPP_MONPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\rsa1024\monsqu.c
DEP_CPP_MONSQ=\
	".\include\rsa.h"\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\monsqu.obj" : $(SOURCE) $(DEP_CPP_MONSQ) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\rsa1024\n0prime.c
DEP_CPP_N0PRI=\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\n0prime.obj" : $(SOURCE) $(DEP_CPP_N0PRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\rsakg\eea.c
DEP_CPP_EEA_C=\
	".\include\iclproc.h"\
	".\include\rsa.h"\
	".\include\rsakg.h"\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\eea.obj" : $(SOURCE) $(DEP_CPP_EEA_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\rsa1024\add.c
DEP_CPP_ADD_C=\
	".\include\rsa.h"\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\add.obj" : $(SOURCE) $(DEP_CPP_ADD_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\rsa1024\mul.c
DEP_CPP_MUL_C=\
	".\include\rsa.h"\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\mul.obj" : $(SOURCE) $(DEP_CPP_MUL_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\C\rsakg\div.c
DEP_CPP_DIV_C=\
	".\include\rsa.h"\
	".\include\rsakg.h"\
	".\C\sha\..\..\include\icl.h"\
	

"$(INTDIR)\div.obj" : $(SOURCE) $(DEP_CPP_DIV_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
