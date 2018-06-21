#**************************************************************************
#*                                                                        *
#*      Intel Restricted Secret                                           *
#*                                                                        *
#*      Valleyview Memory Reference Code                                   *
#*                                                                        *
#*      Copyright (c) 2012 Intel Corp.                                    *
#*                                                                        *
#*      This program has been developed by Intel Corporation.             *
#*      Licensee has Intel's permission to incorporate this source code   *
#*      into their product, royalty free.  This source code may NOT be    *
#*      redistributed to anyone without Intel's written permission.       *
#*                                                                        *
#*      Intel specifically disclaims all warranties, express or           *
#*      implied, and all liability, including consequential and other     *
#*      indirect damages, for the use of this code, including liability   *
#*      for infringement of any proprietary rights, and including the     *
#*      warranties of merchantability and fitness for a particular        *
#*      purpose.  Intel does not assume any responsibility for any        *
#*      errors which may appear in this code nor any responsibility to    *
#*      update it.                                                        *
#*                                                                        *
#**************************************************************************
#**************************************************************************
#*                                                                        *
#*  PURPOSE:                                                              *
#*                                                                        *
#*      Makefile for Bearlake   North Bridge                              *
#*      Memory Reference Code.                                            *
#*                                                                        *
#**************************************************************************

#--------------------------------------------------------------------
# A dummy pseudo-target just so it is the first in the file.
# Now, NMAKE is the same as NMAKE ALL. This is just for developer
# convenience, such that all source and object files appear at the
# top of this file for easy modification.
#--------------------------------------------------------------------
DUMMY:  ALL

#--------------------------------------------------------------------
# Build-time platform options
#--------------------------------------------------------------------
#//DT_PLAT_BEGIN
DT_PLAT		=		1
#//DT_DIMM_END
#//MB_PLAT_BEGIN
MB_PLAT		=		1
#//MB_DIMM_END
SDV		=		0


#--------------------------------------------------------------------
# Some basic definitions.
#--------------------------------------------------------------------
ROOTPATH                                = ..
BINARYNAME                              = PM32

#--------------------------------------------------------------------
# Directory Declarations
#--------------------------------------------------------------------
SRC16DIR=$(ROOTPATH)\SRC16
SRC32DIR=$(ROOTPATH)\SRC32
OUT32DIR=$(ROOTPATH)\OUT32
OUT16DIR=$(ROOTPATH)\OUT16


#--------------------------------------------------------------------
# Assembler, Compiler and Linker Flags
#--------------------------------------------------------------------
!IFNDEF MASMPATH
MASMPATH        = ..\..\Masm615
H2INC           = h2inc
ML              = ml
!ELSE
H2INC           = $(MASMPATH)\Bin\h2inc
ML              = $(MASMPATH)\Bin\ml
!ENDIF
LINK            = $(MASMPATH)\Bin\link

H2INCFLAGS      = /C /Zp1 /WIN32
MLFLAGS         = /nologo /Sa /c /I$(OUT16DIR) /Fo"$(OUT16DIR)\\" /Fl"$(OUT16DIR)\\"
LINKFLAGS       = /MAP

TOBINARY        = EXE2BIN


#--------------------------------------------------------------------
# Build-time platform options
#--------------------------------------------------------------------
MLFLAGS = $(MLFLAGS) /DMRC_ADDRESS=0E0000h

#//DT_PLAT_BEGIN
!IF "$(DT_PLAT)" == "1"
!MESSAGE --Support DT_PLAT
MLFLAGS = $(MLFLAGS) /DDT_PLAT=1
H2INCFLAGS = $(H2INCFLAGS) /DDT_PLAT=1
!ELSE
!MESSAGE --Disable DT_PLAT
MLFLAGS = $(MLFLAGS) /DDT_PLAT=0
!ENDIF
#//DT_PLAT_END

#//MB_PLAT_BEGIN
!IF "$(MB_PLAT)" == "1"
!MESSAGE --Support MB_PLAT
MLFLAGS = $(MLFLAGS) /DMB_PLAT=1
!ELSE
!MESSAGE --Disable MB_PLAT
MLFLAGS = $(MLFLAGS) /DMB_PLAT=0
!ENDIF
#//MB_PLAT_END

#//UB_DIMM_BEGIN
!IF "$(UB_DIMM)" == "1"
!MESSAGE --Support UB_DIMM
MLFLAGS = $(MLFLAGS) /DUB_DIMM=1
!ELSE
!MESSAGE --Disable UB_DIMM
MLFLAGS = $(MLFLAGS) /DUB_DIMM=0
!ENDIF
#//UB_DIMM_END

#//SO_DIMM_BEGIN
!IF "$(SO_DIMM)" == "1"
!MESSAGE --Support SO_DIMM
MLFLAGS = $(MLFLAGS) /DSO_DIMM=1
!ELSE
!MESSAGE --Disable SO_DIMM
MLFLAGS = $(MLFLAGS) /DSO_DIMM=0
!ENDIF
#//SO_DIMM_END

!IF "$(SDV)" == "1"
!MESSAGE --Support Soft SDV
MLFLAGS = $(MLFLAGS) /DSDV_BIOS=1
!ELSE
!MESSAGE --Disable Soft SDV
MLFLAGS = $(MLFLAGS) /DSDV_BIOS=0
!ENDIF


#--------------------------------------------------------------------
# Inference Rules
#--------------------------------------------------------------------
{$(SRC16DIR)}.asm{$(OUT16DIR)}.obj:
        $(ML) $(MLFLAGS) $<

{$(OUT16DIR)}.obj{$(OUT16DIR)}.exe:
        $(LINK) $(LINKFLAGS) $(LINK16OBJS), $(OUT16DIR)\$(BINARYNAME).EXE, $(OUT16DIR)\$(BINARYNAME).MAP;

{$(OUT16DIR)}.exe{$(OUT16DIR)}.bin:
        $(TOBINARY) $(OUT16DIR)\$(BINARYNAME).EXE $(OUT16DIR)\$(BINARYNAME).BIN


#--------------------------------------------------------------------
# Object File Dependencies List
#--------------------------------------------------------------------

LINK16OBJS      =       $(OUT16DIR)\pm32.obj    \
                        $(OUT16DIR)\carinit.obj

#--------------------------------------------------------------------
ALL:    TARGETDIRECTORIES mrc_64k.bin f000.bin

TARGETDIRECTORIES:
  @echo MKDIRing Target Directories...
  if not exist $(OUT16DIR) mkdir $(OUT16DIR)

$(OUT16DIR)\pm32.obj: $(SRC16DIR)\pm32.asm $(SRC16DIR)\mrc.i $(SRC16DIR)\mrcentrypoints.i

$(OUT16DIR)\$(BINARYNAME).EXE: $(LINK16OBJS)
  $(LINK) $(LINKFLAGS) $(LINK16OBJS), $(OUT16DIR)\$(BINARYNAME).EXE, $(OUT16DIR)\$(BINARYNAME).MAP;

$(SRC16DIR)\mrc.i: $(OUT32DIR)\Mrc.i
  copy /y $(OUT32DIR)\Mrc.i $(SRC16DIR)\mrc.i

$(SRC16DIR)\mrcentrypoints.i: $(OUT32DIR)\MrcEntrypoints.i
  copy /y $(OUT32DIR)\MrcEntrypoints.i $(SRC16DIR)\mrcentrypoints.i

$(OUT16DIR)\$(BINARYNAME).BIN: $(OUT16DIR)\$(BINARYNAME).EXE
    $(TOBINARY) $(OUT16DIR)\$(BINARYNAME).EXE $(OUT16DIR)\$(BINARYNAME).BIN
 
#--------------------------------------------------------------------
# Create the input file to DEBUG.EXE for padding the binary so
# the result is exactly 64KB, placed at (4GB - 64KB).
#--------------------------------------------------------------------
f000.bin: $(OUT16DIR)\$(BINARYNAME).EXE $(OUT16DIR)\$(BINARYNAME).BIN
	copy <<file1 $(OUT16DIR)\padin_f.txt
f ds:100,ffff aa
n ..\out16\pm32.bin
l
rip
0000
a 0000
mov ax, ds
add ax, 1000
mov ds, ax
sub ax, 1000
mov ds, ax

t3
f ds:0,200 99

e ds:f0 ea 00 00 00 f0

rbx
1
rcx
0
t2
n ..\out16\f000.bin
w cs:100
q
<<
	debug < $(OUT16DIR)\padin_f.txt

mrc_64k.bin: $(OUT32DIR)\meminit.bin
	copy <<file1 $(OUT16DIR)\padin_e.txt
f ds:100,ffff aa
n ..\out32\meminit.bin
l
rip
0000

rbx
1
rcx
0
n ..\out16\mrc_64k.bin
w cs:100
q
<<
	debug < $(OUT16DIR)\padin_e.txt

CLEAN:
	-@echo  Removing directory $(OUT16DIR)
	-if exist $(OUT16DIR) rd $(OUT16DIR) /S /Q



