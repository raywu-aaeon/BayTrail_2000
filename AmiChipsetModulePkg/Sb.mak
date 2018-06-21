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
#<AMI_FHDR_START>
#
# Name:		SB.mak
#
# Description:	
#
#<AMI_FHDR_END>
#**********************************************************************
#CSP20140314_21 >>  
#CSP20130930>>
Prepare : $(BUILD_DIR)/SbDxeInitElink.h

$(BUILD_DIR)/SbDxeInitElink.h :  $(BUILD_DIR)\token.mak
	$(ECHO) \
"#define OEM_HDA_VERB_TABLE $(OEM_HDA_VERB_TABLE)$(EOL)\
#define ADJUST_EMMC_TABLE $(ADJUST_EMMC_TABLE)$(EOL)\
#define ADJUST_AUDIO_TABLE $(ADJUST_AUDIO_TABLE)$(EOL)\
#define ADJUST_OSSELECT_TABLE $(ADJUST_OSSELECT_TABLE)$(EOL)\
#define ADJUST_I2C0_TABLE $(ADJUST_I2C0_TABLE)$(EOL)\
#define ADJUST_I2C1_TABLE $(ADJUST_I2C1_TABLE)$(EOL)\
#define ADJUST_I2C2_TABLE $(ADJUST_I2C2_TABLE)$(EOL)\
#define ADJUST_I2C3_TABLE $(ADJUST_I2C3_TABLE)$(EOL)\
#define ADJUST_I2C4_TABLE $(ADJUST_I2C4_TABLE)$(EOL)\
#define ADJUST_I2C5_TABLE $(ADJUST_I2C5_TABLE)$(EOL)\
#define ADJUST_I2C6_TABLE $(ADJUST_I2C6_TABLE)$(EOL)\
#define ADJUST_UART0_TABLE $(ADJUST_UART0_TABLE)$(EOL)\
#define ADJUST_UART1_TABLE $(ADJUST_UART1_TABLE)$(EOL)\
#define ADJUST_MISC0_TABLE $(ADJUST_MISC0_TABLE)$(EOL)\
#define ADJUST_MISC1_TABLE $(ADJUST_MISC1_TABLE)$(EOL)\
#define SAVE_RESTORE_CALLBACK_LIST $(SbTimerSaveRestoreRegistersCallbacks)$(EOL)"\
> $@

AMI_CSP_LIB_INCLUDE_FILES := \
 $(SB_INCLUDE_DIR)$(PATH_SLASH)Sb.h \
 $(SB_LIBRARY_INCLUDE_DIR)$(PATH_SLASH)SbCspLib.h \
 $(AmiChipsetPkg_Include_Library_DIR)$(PATH_SLASH)AmiChipsetIoLib.h \
$(AMI_CSP_LIB_INCLUDE_FILES)

#CSP20130930<<
#CSP20140314_21 <<
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
