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
# Name:	<ComponentName>.mak
#
# Description:	
#
#<AMI_FHDR_END>
#**********************************************************************
Prepare : $(BUILD_DIR)/SmmPlatformeLinks.h

$(BUILD_DIR)/SmmPlatformeLinks.h : $(BUILD_DIR)/Token.h $(AmiSmmPlatFormHook_DIR)/AmiSmmCorePlatformHookLib.mak
	$(ECHO) \
"#define PLATFORMHOOK_BEFORE_SMMDISPATCH $(PLATFORMHOOK_BEFORE_SMMDISPATCH)$(EOL)\
#define PLATFORMHOOK_AFTER_SMMDISPATCH $(PLATFORMHOOK_AFTER_SMMDISPATCH)$(EOL)"\
>$(BUILD_DIR)/SmmPlatformeLinks.h