#*************************************************************************
#*************************************************************************
#**                                                                     **
#**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
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
# $Date: 11/28/2013 KimiWu $
#*************************************************************************
#<AMI_FHDR_START>
#
# Name: FOTA.mak
#
# Description: 
#
#<AMI_FHDR_END>
#*************************************************************************
.PHONY: FotaCapsuleBuild DeleteFotaCapsuleBuildFiles

Prepare: DeleteFotaCapsuleBuildFiles

DeleteFotaCapsuleBuildFiles:
	@if exist "tmp" @$(RMDIR) "tmp"
	@if exist dfu_log.txt @$(RM) dfu_log.txt
	@if exist BIOSUPDATE.fv @$(RM) BIOSUPDATE.fv
	@if exist $(FOTA_CAPSULE_NAME) @$(RM) $(FOTA_CAPSULE_NAME)

End: FotaCapsuleBuild

FotaCapsuleBuild: CopyRomFile

ifeq ($(SecureMod_SUPPORT), 1)
ifeq ($(FWCAPSULE_CERT_FORMAT), 0)
ifneq ($(wildcard $(FWpriv)),$(FWpriv))
    @if not exist $(AMI_ROM) @$(CP) $(BUILD_DIR)$(PATH_SLASH)$(PLATFORM_NAME)$(PATH_SLASH)$(TARGET)_$(TOOL_CHAIN_TAG)$(PATH_SLASH)FV$(PATH_SLASH)AMIROM.fd $(AMI_ROM) 
	"$(FOTA_CAPSULE_DIR)\FotaCapsuleBuild.exe" $(AMI_ROM) $(TXE_UPD_IMAGE) $(FW_VERSION) $(CAPSULE_DXE_DRIVER_IMAGE)
else
	"$(FOTA_CAPSULE_DIR)\FotaCapsuleBuild.exe" $(FWCAPSULE_FILE_NAME) $(TXE_UPD_IMAGE) $(FW_VERSION) $(CAPSULE_DXE_DRIVER_IMAGE)
endif
else
ifeq ($(FWCAPSULE_CERT_FORMAT), 1)
ifneq ($(wildcard $(FWpriv)),$(FWpriv))
    @if not exist $(AMI_ROM) @$(CP) $(BUILD_DIR)$(PATH_SLASH)$(PLATFORM_NAME)$(PATH_SLASH)$(TARGET)_$(TOOL_CHAIN_TAG)$(PATH_SLASH)FV$(PATH_SLASH)AMIROM.fd $(AMI_ROM) 
	"$(FOTA_CAPSULE_DIR)\FotaCapsuleBuild.exe" $(AMI_ROM) $(TXE_UPD_IMAGE) $(FW_VERSION) $(CAPSULE_DXE_DRIVER_IMAGE)
else
ifneq ($(wildcard $(FWpub)),$(FWpub))
    @if not exist $(AMI_ROM) @$(CP) $(BUILD_DIR)$(PATH_SLASH)$(PLATFORM_NAME)$(PATH_SLASH)$(TARGET)_$(TOOL_CHAIN_TAG)$(PATH_SLASH)FV$(PATH_SLASH)AMIROM.fd $(AMI_ROM) 
	"$(FOTA_CAPSULE_DIR)\FotaCapsuleBuild.exe" $(AMI_ROM) $(TXE_UPD_IMAGE) $(FW_VERSION) $(CAPSULE_DXE_DRIVER_IMAGE)
else
	"$(FOTA_CAPSULE_DIR)\FotaCapsuleBuild.exe" $(FWCAPSULE_FILE_NAME) $(TXE_UPD_IMAGE) $(FW_VERSION) $(CAPSULE_DXE_DRIVER_IMAGE)
endif
endif
endif
endif
else
    @if not exist $(AMI_ROM) @$(CP) $(BUILD_DIR)$(PATH_SLASH)$(PLATFORM_NAME)$(PATH_SLASH)$(TARGET)_$(TOOL_CHAIN_TAG)$(PATH_SLASH)FV$(PATH_SLASH)AMIROM.fd $(AMI_ROM) 
	"$(FOTA_CAPSULE_DIR)\FotaCapsuleBuild.exe" $(AMI_ROM) $(TXE_UPD_IMAGE) $(FW_VERSION) $(CAPSULE_DXE_DRIVER_IMAGE)
endif
	$(CP) BIOSUPDATE.fv $(FOTA_CAPSULE_NAME)
	$(AMIGCH) $(FOTA_CAPSULE_NAME)

#*************************************************************************
#*************************************************************************
#**                                                                     **
#**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
#**                                                                     **
#**                       All Rights Reserved.                          **
#**                                                                     **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
#**                                                                     **
#**                       Phone: (770)-246-8600                         **
#**                                                                     **
#*************************************************************************
#*************************************************************************