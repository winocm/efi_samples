!IFNDEF EFI_SOURCE
!ERROR EFI_SOURCE must be defined
!ENDIF

SRC_DIR = $(EFI_SOURCE)\BUILD\Tools\src
BIN_DIR = $(EFI_SOURCE)\BUILD\Tools\bin

OBJ_DEPS =	\
	$(SRC_DIR)\common\compress.obj

BIN_TARGETS = \
	$(BIN_DIR)\col.exe          \
    $(BIN_DIR)\fwimage.exe      \
    $(BIN_DIR)\efildrimage.exe  \
    $(BIN_DIR)\genhelp.exe      \
    $(BIN_DIR)\genmake.exe      \
    $(BIN_DIR)\splitfile.exe    \
	$(BIN_DIR)\eficompress.exe  \
	$(BIN_DIR)\efirom.exe       \
	$(BIN_DIR)\dskimage			\
	$(BIN_DIR)\vccheck.exe

INC=-I $(EFI_SOURCE)\inc -I $(EFI_SOURCE)\inc\ia32 -I $(EFI_SOURCE)\Edk -I $(EFI_SOURCE)\Edk\Include -I $(EFI_SOURCE)\Edk\Include\ia32 -I $(EFI_SOURCE)\build\tools\src\common
C_PROJ_FLAGS=/DEFI32 /UEFI_DEBUG /DNDEBUG
C_BUILD_FLAGS=/W3 /WX /GX /O1 /GF /Gy /MD
L_BUILD_FLAGS=/SUBSYSTEM:CONSOLE /MACHINE:I386 /OPT:REF /LIBPATH:"$(EFI_LIBPATH)" /LIBPATH:"$(EFI_PSDKPATH)\Lib"
LIB_BUILD_FLAGS=

#
# Ia32 tools
#

CC=cl /nologo
LINK=link /NOLOGO
LIB=lib /NOLOGO
MAKE=$(MAKE) -nologo
FWIMAGE=$(BUILD_TOOL)\fwimage

LFLAGS=$(L_BUILD_FLAGS) $(L_PROJ_FLAGS)
CFLAGS=$(C_BUILD_FLAGS) $(C_PROJ_FLAGS) $(INC)

# PATH=$(PATH);$(EFI_BUILD_TOOLS);$(EFI_SOURCE)\tools\bin

all: $(BIN_TARGETS)

$(BIN_TARGETS): $(OBJ_DEPS)
	$(MAKE) -f <<$(SRC_DIR)\$(@B)\makefile
$@: $(SRC_DIR)\$(@B)\$(@B).obj $(OBJ_DEPS)
	$(CC) $(SRC_DIR)\$(@B)\$(@B).obj /Fe$@ /link $(OBJ_DEPS) $(LFLAGS)
$(SRC_DIR)\$(@B)\$(@B).obj: $(SRC_DIR)\$(@B)\$(@B).c
	$(CC) -c $(SRC_DIR)\$(@B)\$(@B).c /Fo$(SRC_DIR)\$(@B)\$(@B).obj $(CFLAGS)
<<NOKEEP

.c.obj:
	$(CC) -c $< /Fo$@ $(CFLAGS)
		
clean:
	-erase /s $(SRC_DIR)\*.obj
	-erase /s $(SRC_DIR)\*.idb
	-erase /s $(SRC_DIR)\*.pdb
	-erase /q $(BIN_DIR)\col.*
	-erase /q $(BIN_DIR)\fwimage.*
	-erase /q $(BIN_DIR)\efildrimage.*
	-erase /q $(BIN_DIR)\genhelp.*
	-erase /q $(BIN_DIR)\genmake.*
	-erase /q $(BIN_DIR)\splitfile.*
	-erase /q $(BIN_DIR)\eficompress.*
	-erase /q $(BIN_DIR)\efirom.*
	-erase /q $(BIN_DIR)\dskimage.*
	-erase /q $(BIN_DIR)\vccheck.*

rebuild: clean all
