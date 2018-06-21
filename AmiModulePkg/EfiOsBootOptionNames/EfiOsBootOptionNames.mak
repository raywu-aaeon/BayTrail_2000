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
# $Header:  $
#
# $Revision: $
#
# $Date:  $
#**********************************************************************
#<AMI_FHDR_START>
#
# Name:	<ComponentName>.mak
#
# Description:	
#
#<AMI_FHDR_END>
#**********************************************************************
Prepare : $(BUILD_DIR)/EfiOsNamesFilePathMaps.h

EfiOsBootOptionNamesFilePathItem := $(subst \,\\,$(EfiOsBootOptionNamesFilePathItem)) #Replace \ to \\
EfiOsBootOptionNamesFilePathItem := $(subst <,L\",$(EfiOsBootOptionNamesFilePathItem)) #Replace < to L"
EfiOsBootOptionNamesFilePathItem := $(subst >,\",$(EfiOsBootOptionNamesFilePathItem)) #Replace > to "


$(BUILD_DIR)/EfiOsNamesFilePathMaps.h : 
	$(ECHO) \
"#define EfiOsFilePathMaps $(EfiOsBootOptionNamesFilePathItem)"\
>$(BUILD_DIR)/EfiOsNamesFilePathMaps.h
 
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