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
# $Header: /Alaska/SOURCE/Modules/SdioDriver/SdioRecovery.mak 1     7/18/12 4:49a Rajeshms $
#
# $Revision: 1 $
#
# $Date: 7/18/12 4:49a $
#**********************************************************************
# Revision History
# ----------------
# $Log: /Alaska/SOURCE/Modules/SdioDriver/SdioRecovery.mak $
# 
# 1     7/18/12 4:49a Rajeshms
# [TAG]  		EIP93345
# [Category]  	New Feature
# [Description]  	Create a PEI driver for Boot Block recovery from SD/MMC
# devices
# [Files]  		SdioRecovery.cif
# SdioRecovery.sdl
# SdioRecovery.mak
# SdioRecovery.c
# SdioFindRecoveryDevice.c
# SdioRecovery.h
# 
# 1     7/18/12 4:30a Rajeshms
# [TAG]  		EIP93345 
# [Category]  	New Feature
# [Description]  	Create a PEI driver for Boot Block recovery from SD/MMC
# devices
# [Files]  		Board\EM\SdioRecovery\SdioRecovery.cif
# Board\EM\SdioRecovery\SdioRecovery.sdl
# Board\EM\SdioRecovery\SdioRecovery.mak
# Board\EM\SdioRecovery\SdioRecovery.c
# Board\EM\SdioRecovery\SdioFindRecoveryDevice.c
# Board\EM\SdioRecovery\SdioRecovery.h
# 
# 
#**********************************************************************
#<AMI_FHDR_START>
#
# Name:           SdioRecovery.mak
#
# Description:    Make file to build SdioRecovery module
#
#<AMI_FHDR_END>
#**********************************************************************

Prepare : $(BUILD_DIR)/SdioRecoveryElink.h

$(BUILD_DIR)/SdioRecoveryElink.h :  $(BUILD_DIR)/token.mak $(SdioRecovery_DIR)/SdioRecovery.mak
	$(ECHO) \
"#define SD_ROOT_BRIDGE_LIST $(SD_ROOT_BRIDGE_LIST)$(EOL)"\
>$(BUILD_DIR)/SdioRecoveryElink.h

##*************************************************************************
##*************************************************************************
##**                                                                     **
##**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
##**                                                                     **
##**                       All Rights Reserved.                          **
##**                                                                     **
##**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
##**                                                                     **
##**                       Phone: (770)-246-8600                         **
##**                                                                     **
##*************************************************************************
##*************************************************************************