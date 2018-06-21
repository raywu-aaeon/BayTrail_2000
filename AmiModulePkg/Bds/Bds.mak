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
# $Header: /Alaska/Tools/template.mak 6     1/13/10 2:13p Felixp $
#
# $Revision: 6 $
#
# $Date: 1/13/10 2:13p $
#**********************************************************************
#<AMI_FHDR_START>
#
# Name:	<ComponentName>.mak
#
# Description:	
#
#<AMI_FHDR_END>
#**********************************************************************
Prepare : $(BUILD_DIR)/BootOptioneLinks.h

# We are trying to convert a list of comma separated functions to a list of comma 
# separated function name strings.
# The original list of function may contain C comment blocks /* */
BDS_CONTROL_FLOW0 = $(subst $(SPACE),,$(BDS_CONTROL_FLOW)) # remove the spaces
BDS_CONTROL_FLOW1 = $(subst */$(COMMA),*/,$(BDS_CONTROL_FLOW0)) # remove commas after the comments
BDS_CONTROL_FLOW2 = $(subst *//*,,$(BDS_CONTROL_FLOW1)) # merge the adjacent comments
BDS_CONTROL_FLOW3 = $(subst */,*/",$(BDS_CONTROL_FLOW2)) # add quote after the comment to begin the next string
BDS_CONTROL_FLOW4 = "$(subst $(COMMA),"$(COMMA)",$(BDS_CONTROL_FLOW3))", # add quotes before and after the commas
BDS_CONTROL_FLOW5 = $(subst "/*,/*,$(BDS_CONTROL_FLOW4)) # remove quote before the comments
BDS_CONTROL_FLOW_STRINGS = $(subst ",\",$(BDS_CONTROL_FLOW5)) # escape the quotes for echo

$(BUILD_DIR)/BootOptioneLinks.h : $(BUILD_DIR)/Token.h $(Bds_DIR)/Bds.mak
	$(ECHO) \
"#define BOOT_OPTION_DP_MATCHING_FUNCTIONS $(BootOptionDpMatchingFunctions)$(EOL)\
#define BOOT_OPTION_MATCHING_FUNCTIONS $(BootOptionMatchingFunctions)$(EOL)\
#define BOOT_OPTION_BUILD_NAME_FUNCTIONS $(BootOptionBuildNameFunctions)$(EOL)\
#define BOOT_OPTION_BUILD_FILE_PATH_FUNCTIONS $(BootOptionBuildFilePathFunctions)$(EOL)\
#define BOOT_OPTION_BOOT_DEVICE_FILTERING_FUNCTIONS $(BootOptionBootDeviceFilteringFunctions)$(EOL)\
$(EOL)\
#define BDS_CONTROL_FLOW $(BDS_CONTROL_FLOW)$(EOL)\
#define BDS_CONTROL_FLOW_NAMES $(BDS_CONTROL_FLOW_STRINGS)"\
>$(BUILD_DIR)/BootOptioneLinks.h
 
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