#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
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
# $Header: /Alaska/BIN/Core/Modules/Recovery/ReFlash.mak 14    12/05/11 1:48p Artems $
#
# $Revision: 14 $
#
# $Date: 12/05/11 1:48p $
#**********************************************************************
#<AMI_FHDR_START>
#
# Name:	ReFlash.mak
#
# Description:	
#
#<AMI_FHDR_END>
#**********************************************************************
Prepare : $(BUILD_DIR)\ReflashDefinitions.h

$(BUILD_DIR)\ReflashDefinitions.h : $(ReFlash_DIR)\Reflash.mak
	@$(ECHO)\
"$(if $(REFLASH_DEFINITIONS) ,\
$(foreach S_DEF, $(REFLASH_DEFINITIONS),#include<$(S_DEF)>$(EOL)))\
$(EOL)\
#define REFLASH_FUNCTION_LIST $(ReflashFunctions)$(EOL)\
#define OEM_BEFORE_FLASH_UPDATE_CALLBACK_LIST $(OemBeforeFlashUpdateList)$(EOL)\
#define OEM_AFTER_FLASH_UPDATE_CALLBACK_LIST $(OemAfterFlashUpdateList)$(EOL)\
" > $(BUILD_DIR)\ReflashDefinitions.h


#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
#**                                                                  **
#**                       All Rights Reserved.                       **
#**                                                                  **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
#**                                                                  **
#**                       Phone: (770)-246-8600                      **
#**                                                                  **
#**********************************************************************
#**********************************************************************