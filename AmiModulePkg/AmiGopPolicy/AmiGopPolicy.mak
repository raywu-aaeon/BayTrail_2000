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
# $Header: /Alaska/SOURCE/Modules/AmiGopPolicy/AmiGopPolicy.mak 4     7/26/12 7:30a Josephlin $
#
# $Revision: 4 $
#
# $Date: 7/26/12 7:30a $
#**********************************************************************
#<AMI_FHDR_START>
#
# Name:         AmiGopPolicy.mak
#
# Description:  Make file that builds AmiGopPolicy components and link
#               them to respective binary.
#
#<AMI_FHDR_END>
#**********************************************************************
Prepare : $(BUILD_DIR)/AmiGopPolicyElink.h

$(BUILD_DIR)/AmiGopPolicyElink.h :  $(BUILD_DIR)/token.mak
	$(ECHO) \
"#define OEM_GOP_DEVICE_CHECK_LIST $(OemGopDeviceCheckList) $(EOL)\
#define OEM_GOP_SWITCH_HOOK_LIST $(OemGopSwitchHookList) $(EOL)"\
 > $@
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