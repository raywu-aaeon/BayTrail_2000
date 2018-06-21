#*************************************************************************
#*************************************************************************
#**                                                                     **
#**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
#**                                                                     **
#**                       All Rights Reserved.                          **
#**                                                                     **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
#**                                                                     **
#**                       Phone: (770)-246-8600                         **
#**                                                                     **
#*************************************************************************
#*************************************************************************

#*************************************************************************
# $Header: $
#
# $Revision: $
#
# $Date: $
#*************************************************************************
#<AMI_FHDR_START>
#
# Name: FastBoot.mak
#
# Description:
# Make file to build Fastboot library part
#
#<AMI_FHDR_END>
#*************************************************************************

#(EIP62683)(EIP63924)(EIP62845)>
#{$(FastBoot_DIR)}.c{$(BUILD_DIR)}.obj::
#	$(CC) $(CFLAGS) /I $(FastBoot_DIR)  \
#    /D\"IS_FAST_BOOT_LIST=$(IsFastBootList)\" \
#    /D\"AFTER_ALL_DRIVER_CONNECT_HOOK=$(FastBootAfterAllDriverConnctHook)\" \
#    /D\"FAST_BOOT_CHECK_MODE_CHANGE_HOOK=$(FastBootCheckModeChangeList)\" \
#    /D\"BEFORE_CONNECT_FAST_BOOT_DEVICE_HOOK=$(BeforeConnectFastBootDeviceHook)\" \
#    /Fo$(BUILD_DIR)\ $<
#<(EIP62683)(EIP63924)(EIP62845)

Prepare : FastBootFile

FastBootFile : $(BUILD_DIR)/FastBootLinks.h
	
$(BUILD_DIR)/FastBootLinks.h : $(BUILD_DIR)/Token.h $(FastBoot_DIR)/FastBoot.mak
	$(ECHO) \
"#define IS_FAST_BOOT_LIST $(IsFastBootList)$(EOL)\
#define AFTER_ALL_DRIVER_CONNECT_HOOK $(FastBootAfterAllDriverConnctHook)$(EOL)\
#define FAST_BOOT_CHECK_MODE_CHANGE_HOOK $(FastBootCheckModeChangeList)$(EOL)\
#define BEFORE_CONNECT_FAST_BOOT_DEVICE_HOOK $(BeforeConnectFastBootDeviceHook)$(EOL)\
#define RETURN_NORMAL_MODE_HOOK $(ReturnNormalMode)"\
>$(BUILD_DIR)/FastBootLinks.h

#*************************************************************************
#*************************************************************************
#**                                                                     **
#**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
#**                                                                     **
#**                       All Rights Reserved.                          **
#**                                                                     **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
#**                                                                     **
#**                       Phone: (770)-246-8600                         **
#**                                                                     **
#*************************************************************************
#*************************************************************************
