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
#*      Makefile for Valleyview North Bridge                              *
#*      Memory Reference Code.                                            *
#*                                                                        *
#**************************************************************************

#--------------------------------------------------------------------
# A dummy pseudo-target just so it is the first in the file.
# Now, NMAKE is the same as NMAKE ALL. This is just for developer
# convenience, such that all source and object files appear at the
# top of this file for easy modification.
#--------------------------------------------------------------------
DUMMY: ALL

#--------------------------------------------------------------------
# Build-time platform options
# Switches:  1 = Include feature
#            0 = Exclude feature
#--------------------------------------------------------------------

OPTIMIZE        =    1
DEBUG           =    1


#--------------------------------------------------------------------
# Some basic definitions.
#--------------------------------------------------------------------
ROOTPATH                     = ..
MEMREFCODEBINARYNAME         = MEMINIT

#--------------------------------------------------------------------
# Compiler and Linker Flags
#--------------------------------------------------------------------

CL              = cl
ML              = ml
MLFLAGS         = /c /Zd /Fm$(OUT32DIR)\\ /Fl$(OUT32DIR)\\ /Fo$(OUT32DIR)\\
LINK32          = link
LINK32FLAGS     = /base:0x00000000 \
                  /fixed \
                  /nologo \
                  /incremental:no \
                  /align:32 \
                  /nodefaultlib \
                  /LTCG \
                  /subsystem:console \
                  /merge:.rdata=.data \
                  /pdb:$(OUT32DIR)\$(MEMREFCODEBINARYNAME).pdb \
                  /noentry \
                  /map:$(OUT32DIR)\$(MEMREFCODEBINARYNAME).MAP \
                  /out:$(OUT32DIR)\$(MEMREFCODEBINARYNAME).BIN \
                  /DLL

CLFLAGS         = /Oi /Zp1 /Gr /GL /W4 /WX /Gy /GS- /FAsc /c /Zi /Gm /X /Fd /Gs10240 /Fo$(OUT32DIR)\\ /Fa$(OUT32DIR)\\


!IF "$(DEBUG)" == "1"
LINK32FLAGS     = $(LINK32FLAGS) /debug
!ENDIF

!IFNDEF H2INCDIR
H2INC           = h2inc
!ELSE
H2INC           = $(H2INCDIR)\h2inc
!ENDIF
H2INCFLAGS      = /Zu /Zp1 /D_MSC_EXTENSIONS /X

#Optimize for small size on P4
!IF "$(OPTIMIZE)" == "1"
CLFLAGS          = $(CLFLAGS) /O1 /Osyb2
!ELSE
#Disable optimization for debugging
CLFLAGS          = $(CLFLAGS) /Od
!ENDIF

#--------------------------------------------------------------------
# Memory Reference Code Object Files.
#--------------------------------------------------------------------
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\McFunc.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\MrcFunc.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\OemHooks.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\DetectDimms.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\ConfigMem.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\ConfigMemData.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\IoAccess.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\MMRCLibraries.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\MMRCProjectLibraries.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\MMRCSelectLibraries.obj




#--------------------------------------------------------------------
# Directory Declarations
#--------------------------------------------------------------------
SRC32DIR=$(ROOTPATH)\SRC32
OUT32DIR=$(ROOTPATH)\OUT32

#--------------------------------------------------------------------
# Inference Rules
#--------------------------------------------------------------------
{$(SRC32DIR)}.c{$(OUT32DIR)}.obj:
        $(CL) $(CLFLAGS) $<

#--------------------------------------------------------------------
ALL:    CLEAN TARGETDIRECTORIES $(OUT32DIR)\$(MEMREFCODEBINARYNAME).BIN $(OUT32DIR)\$(MEMREFCODEBINARYNAME).DUMPBIN.TXT
TARGETDIRECTORIES:
  @echo MKDIRing Target Directories...
  if not exist $(OUT32DIR) mkdir $(OUT32DIR)


$(OUT32DIR)\Mrc.i: $(SRC32DIR)\Mrc.h
  $(H2INC) $(H2INCFLAGS) /Fa$(OUT32DIR)\Mrc.i $(SRC32DIR)\Mrc.h

$(OUT32DIR)\$(MEMREFCODEBINARYNAME).BIN: $(LINK32OBJS)
        @echo Linking 32-bit Components for POST Memory Init Module...
        $(LINK32) $(LINK32FLAGS) $(LINK32OBJS)

$(OUT32DIR)\$(MEMREFCODEBINARYNAME).DUMPBIN.TXT: $(OUT32DIR)\$(MEMREFCODEBINARYNAME).BIN
        @echo Generating dump file $(OUT32DIR)\$(MEMREFCODEBINARYNAME).DUMPBIN.TXT...
        dumpbin $(OUT32DIR)\$(MEMREFCODEBINARYNAME).BIN /headers > $(OUT32DIR)\$(MEMREFCODEBINARYNAME).DUMPBIN.TXT
        dumpbin $(OUT32DIR)\$(MEMREFCODEBINARYNAME).BIN /disasm >> $(OUT32DIR)\$(MEMREFCODEBINARYNAME).DUMPBIN.TXT
        @echo --------------------------------------------------------------
        @echo *** Memory Reference Code Build Complete ***
        @echo --------------------------------------------------------------
        -@if exist vc60.pdb  erase vc60.pdb
        -@if exist vc60.idb  erase vc60.idb
        -@if exist vc70.pdb  erase vc70.pdb
        -@if exist vc70.idb  erase vc70.idb
        -@if exist $(SRC32DIR)\vc60.pdb  erase $(SRC32DIR)\vc60.pdb
        -@if exist $(SRC32DIR)\vc60.idb  erase $(SRC32DIR)\vc60.idb
        -@if exist $(SRC32DIR)\vc70.pdb  erase $(SRC32DIR)\vc70.pdb
        -@if exist $(SRC32DIR)\vc70.idb  erase $(SRC32DIR)\vc70.idb

COPYSRC: $(OUT32DIR)\$(MEMREFCODEBINARYNAME).BIN
  @echo Copying $(SRC32DIR) to $(OUT32DIR)
  copy $(SRC32DIR) $(OUT32DIR) > nul

CLEAN:
  -@echo  Removing directory $(OUT32DIR)
  -@if exist $(OUT32DIR) rd $(OUT32DIR) /S /Q
