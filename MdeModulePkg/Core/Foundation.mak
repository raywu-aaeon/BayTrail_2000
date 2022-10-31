#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
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
#<AMI_FHDR_START>
#
# Name:	Foundation.mak
#
# Description:	Processes PeiCore and DxeCore eLinks
#
#<AMI_FHDR_END>
#**********************************************************************
Prepare : $(BUILD_DIR)/PeiCoreHooks.h $(BUILD_DIR)/DxeCoreHooks.h

$(BUILD_DIR)/PeiCoreHooks.h :  $(TOKEN_MAK)
	$(ECHO) \
"#define PEI_CORE_INITIALIZE_HOOKS $(PeiCoreInitialize)\
$(EOL)#define PEI_CORE_MEMORY_INSTALLED_HOOKS $(PeiCoreMemoryInstalled)$(EOL)\
"\
> $@

$(BUILD_DIR)/DxeCoreHooks.h : $(TOKEN_MAK)
	$(ECHO) "#define DXE_CORE_INITIALIZE_HOOKS $(DxeCoreInitialize)$(EOL)" > $@

#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
#**                                                                  **
#**                       All Rights Reserved.                       **
#**                                                                  **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
#**                                                                  **
#**                       Phone: (770)-246-8600                      **
#**                                                                  **
#**********************************************************************
#**********************************************************************