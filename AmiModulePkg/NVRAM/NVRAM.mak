#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2011, American Megatrends, Inc.         **
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
#-----------------------------------------------------------------------
#
# Name:			NVRAM.mak
#
# Description:	
#				
#
#-----------------------------------------------------------------------
#<AMI_FHDR_END>
#**********************************************************************
Prepare : $(BUILD_DIR)/NvramElink.h

$(BUILD_DIR)/NvramElink.h :  $(BUILD_DIR)/Token.mak
	$(ECHO) \
"#define PEI_GET_VAR_LIST $(PeiGetVariableHook)$(EOL)\
#define PEI_GET_NEXT_VAR_NAME_LIST $(PeiGetNextVariableNameHook)$(EOL)\
#define GET_VAR_LIST $(GetVariableHook)$(EOL)\
#define GET_NEXT_VAR_NAME_LIST $(GetNextVariableNameHook)$(EOL)\
#define SET_VAR_LIST $(SetVariableHook)$(EOL)\
#define IS_MFG_MODE_LIST $(IsMfgMode)$(EOL)\
#define IS_RESET_CONFIG_MODE_LIST $(IsResetConfigMode)$(EOL)\
#define IS_DEFAULT_CONFIG_MODE_LIST $(IsDefaultConfigMode)$(EOL)\
" > $(BUILD_DIR)/NvramElink.h

#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2011, American Megatrends, Inc.         **
#**                                                                  **
#**                       All Rights Reserved.                       **
#**                                                                  **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
#**                                                                  **
#**                       Phone: (770)-246-8600                      **
#**                                                                  **
#**********************************************************************
#**********************************************************************
