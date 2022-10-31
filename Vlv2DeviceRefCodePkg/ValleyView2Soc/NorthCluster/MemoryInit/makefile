#--------------------------------------------------------------------
# A dummy pseudo-target just so it is the first in the file.
# Now, NMAKE is the same as NMAKE ALL. This is just for developer
# convenience, such that all source and object files appear at the
# top of this file for easy modification.
#--------------------------------------------------------------------
DUMMY:  ALL

#--------------------------------------------------------------------
# Some basic definitions.
#--------------------------------------------------------------------
ROOTPATH                        = .
MEMREFCODEBINARYNAME            = MEMINIT
BIOSIMAGENAME                   = MINIBIOS

#--------------------------------------------------------------------
# Directory Declarations
#--------------------------------------------------------------------
SRC16DIR=$(ROOTPATH)\SRC16
SRC32DIR=$(ROOTPATH)\SRC32
OUT16DIR=$(ROOTPATH)\OUT16
OUT32DIR=$(ROOTPATH)\OUT32


#--------------------------------------------------------------------
# All the King's Targets...
#--------------------------------------------------------------------
ALL:    TARGETDIRECTORIES $(BIOSIMAGENAME).BIN

TARGETDIRECTORIES:
        @echo MKDIRing Target Directories...
        if not exist $(OUT32DIR) mkdir $(OUT32DIR)
        if not exist $(OUT16DIR) mkdir $(OUT16DIR)

BINS16BIT:
        cd $(SRC16DIR)
        nmake
        cd ..

MEMREFCODEBINARY:
        cd $(SRC32DIR)
        nmake
        cd ..

$(BIOSIMAGENAME).BIN:   MEMREFCODEBINARY BINS16BIT
	cls
        @echo Concatenating all binaries...
        copy /b $(ROOTPATH)\896kpad.bin+$(OUT16DIR)\mrc_64k.bin+$(OUT16DIR)\f000.bin $(BIOSIMAGENAME).BIN > nul
        @echo ----------------------------------------------------------
        @echo *** BUILD COMPLETE ***
        @echo MINI-BIOS Image File....................: $(BIOSIMAGENAME).BIN
        @echo Memory Reference Code Image File...: $(OUT32DIR)\$(MEMREFCODEBINARYNAME).BIN
        @echo ----------------------------------------------------------
	dir $(BIOSIMAGENAME).BIN
	dir $(OUT32DIR)\$(MEMREFCODEBINARYNAME).BIN

CLEAN:
	-@echo  Deleting $(BIOSIMAGENAME).BIN
	-if exist $(BIOSIMAGENAME).BIN erase $(BIOSIMAGENAME).BIN

        cd $(SRC16DIR)
        nmake clean
        cd ..

        cd $(SRC32DIR)
        nmake clean
        cd ..
