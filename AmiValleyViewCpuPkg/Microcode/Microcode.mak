#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
#**                                                                  **
#**                       All Rights Reserved.                       **
#**                                                                  **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
#**                                                                  **
#**                       Phone: (770)-246-8600                      **
#**                                                                  **
#**********************************************************************
#**********************************************************************

#**********************************************************************
# $Header: $
#
# $Revision: $
#
# $Date: $
#**********************************************************************
# Revision History
# ----------------
# $Log: $
# 
#**********************************************************************
#<AMI_FHDR_START>
#
# Name:	Microcode.mak
#
# Description:	
#
#<AMI_FHDR_END>
#**********************************************************************
#List of merge64 formated microcode files separted by lines..
MERGE_64_MCODE_LIST = $(patsubst end$(SPACE), end$(EOL),$(MERGE_64_MCODE_STRING))

#List of merge64 formated microcode files separted by spaces.
MERGE_64_MCODE_STRING = $(foreach Microcode, $(MICROCODE_FILES_LIST), \
   file $(Microcode) binfile=$(Microcode) align=$(MICROCODE_ALIGNMENT) end)

#List of microcode files separated by spaces.
MICROCODE_FILES_LIST = $(strip $(MICROCODE_FILES))

#Merge64 microcode pad.
MCODE_PAD = blank MICROCODE_PAD$(EOL)\
    size= $(MICROCODE_PAD_SIZE)$(EOL)\
    pattern=(0xff)$(EOL)\
end

MCODE_PAD_ECHO = $(MCODE_PAD)

MCODE_TXT2PDB = $(MICROCODE_DIR)$(PATH_SLASH)$(MICROCODE_FILE_NAME)
   
TXT2PDB = $(MICROCODE_DIR)$(PATH_SLASH)CovMicroL.exe

#******************Prepare the target*******************************************
Prepare : GenerateMicrocode $(BUILD_DIR)$(PATH_SLASH)MICROCODE.BIN
.PHONY : GenerateMicrocode

GenerateMicrocode:
	$(TXT2PDB) $(join $(MCODE_TXT2PDB),.inc)

ifeq ("$(wildcard $(MCODE_TXT2PDB).inc)", "") 
    MCODE_TXT2PDB_STRING  =
   $(warning $(MCODE_TXT2PDB) - the uCode is not exist )
else 
    MCODE_TXT2PDB_STRING = $(foreach Microcode, $(MCODE_TXT2PDB), \
   file $(Microcode).PDB binfile=$(Microcode).PDB align=$(MICROCODE_ALIGNMENT) end)
endif

$(BUILD_DIR)$(PATH_SLASH)MICROCODE.BIN:
	$(ECHO)  \
"output$(EOL)\
    MICROCODE_FILES($(BUILD_DIR)$(PATH_SLASH)Microcode.bin)$(EOL)\
end$(EOL)\
group MICROCODE_FILES$(EOL)\
    upper=0xffffffff$(EOL)\
components$(EOL)\
$(MCODE_PAD_ECHO)$(EOL)\
$(MERGE_64_MCODE_LIST)$(EOL)\
$(MCODE_TXT2PDB_STRING)$(EOL)\
end end"\
>$(BUILD_DIR)$(PATH_SLASH)Microcode.ini

	Merge64 /s $(BUILD_DIR)\Microcode.ini

$(BUILD_DIR)$(PATH_SLASH)MICROCODE_BLANK.BIN : $(BUILD_DIR)$(PATH_SLASH)token.mak $(MICROCODE_DIR)$(PATH_SLASH)Microcode.mak
	$(ECHO)  \
"output$(EOL)\
    MICROCODE_FILES($(BUILD_DIR)$(PATH_SLASH)Microcode_Blank.bin)$(EOL)\
end$(EOL)\
group MICROCODE_FILES$(EOL)\
    upper=0xffffffff$(EOL)\
components$(EOL)\
    $(MCODE_PAD)$(EOL)\
end end"\
>$(BUILD_DIR)$(PATH_SLASH)MicrocodeBlank.ini

	Merge64 /s $(BUILD_DIR)$(PATH_SLASH)MicrocodeBlank.ini

#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
#**                                                                  **
#**                       All Rights Reserved.                       **
#**                                                                  **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
#**                                                                  **
#**                       Phone: (770)-246-8600                      **
#**                                                                  **
#**********************************************************************
#**********************************************************************
