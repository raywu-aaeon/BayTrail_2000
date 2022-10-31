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
# Revision History
# ----------------
# $Log: $
# 
#**********************************************************************
#<AMI_FHDR_START>
#
# Name:	<ComponentName>.mak
#
# Description:	
#
#<AMI_FHDR_END>
#**********************************************************************
End : CopyRomFile

.PHONY : CopyRomFile
CopyRomFile:
	$(RM) *.rom
#CSP20131203 - fix SecureMod_SUPPORT = 0 build error >>
ifeq ($(VERIFY_BOOT_SUPPORT), 1)
	$(CP) $(FD_MANIFEST_FILE) $(FWCAPSULE_FILE_NAME)
else
	@if exist "$(ROM_IMAGE_DIR)$(PATH_SLASH)FpfMirrorNvarValues.txt" @$(RM) "$(ROM_IMAGE_DIR)$(PATH_SLASH)FpfMirrorNvarValues.txt"
ifeq ($(SecureMod_SUPPORT), 1) 
ifeq ($(CREATE_FWCAPSULE), 0)
	$(CP) $(BUILD_DIR)$(PATH_SLASH)$(PLATFORM_NAME)$(PATH_SLASH)$(TARGET)_$(TOOL_CHAIN_TAG)$(PATH_SLASH)FV$(PATH_SLASH)AMIROM.fd $(AMI_ROM)  
else
ifneq ($(wildcard $(FWpriv)),$(FWpriv))
	$(CP) $(BUILD_DIR)$(PATH_SLASH)$(PLATFORM_NAME)$(PATH_SLASH)$(TARGET)_$(TOOL_CHAIN_TAG)$(PATH_SLASH)FV$(PATH_SLASH)AMIROM.fd $(AMI_ROM)  	
else
	make_sign_capsule.bat
endif
endif
else
	$(CP) $(BUILD_DIR)$(PATH_SLASH)$(PLATFORM_NAME)$(PATH_SLASH)$(TARGET)_$(TOOL_CHAIN_TAG)$(PATH_SLASH)FV$(PATH_SLASH)AMIROM.fd $(AMI_ROM)            
endif
endif	
#CSP20131203 - fix SecureMod_SUPPORT = 0 build error <<

ifeq ($(ROM_IMAGE_ME_MODE), 1)
End : BuildFullImage

.PHONY : BuildFullImage
BuildFullImage: 
#CSP20131203 - fix SecureMod_SUPPORT = 0 build error >>
ifeq ($(VERIFY_BOOT_SUPPORT), 1)
	$(CP) $(FD_MANIFEST_FILE) $(ROM_IMAGE_DIR)$(PATH_SLASH)BIOS.ROM
else
ifeq ($(SecureMod_SUPPORT), 1)
ifneq ($(CREATE_FWCAPSULE), 0)
ifneq ($(wildcard $(FWpriv)),$(FWpriv))
	$(CP) $(BUILD_DIR)$(PATH_SLASH)$(PLATFORM_NAME)$(PATH_SLASH)$(TARGET)_$(TOOL_CHAIN_TAG)$(PATH_SLASH)FV$(PATH_SLASH)AMIROM.fd $(ROM_IMAGE_DIR)$(PATH_SLASH)BIOS.ROM
else
	$(CP) $(FWCAPSULE_FILE_NAME) $(ROM_IMAGE_DIR)$(PATH_SLASH)BIOS.ROM
endif   
else
	$(CP) $(BUILD_DIR)$(PATH_SLASH)$(PLATFORM_NAME)$(PATH_SLASH)$(TARGET)_$(TOOL_CHAIN_TAG)$(PATH_SLASH)FV$(PATH_SLASH)AMIROM.fd $(ROM_IMAGE_DIR)$(PATH_SLASH)BIOS.ROM
endif
else
	$(CP) $(BUILD_DIR)$(PATH_SLASH)$(PLATFORM_NAME)$(PATH_SLASH)$(TARGET)_$(TOOL_CHAIN_TAG)$(PATH_SLASH)FV$(PATH_SLASH)AMIROM.fd $(ROM_IMAGE_DIR)$(PATH_SLASH)BIOS.ROM
endif
endif
#CSP20131203 - fix SecureMod_SUPPORT = 0 build error <<

	$(RM) $(ROM_IMAGE_DIR)$(PATH_SLASH)fitc.log

ifeq ($(PLATFORM_FLAVOR_SELECT), 2)
ifeq ($(TXE_1_25MB_SUPPORT), 1)
	$(ROM_IMAGE_DIR)$(PATH_SLASH)fitcbuildDesktop8MB_TXE1.25MB.bat
	$(CP) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin $(AMI_DESKTOP_ROM_8MB_TXE_1_25MB)
	$(RM) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin
endif
ifeq ($(TXE_1_375MB_SUPPORT), 1)
	$(ROM_IMAGE_DIR)$(PATH_SLASH)fitcbuildDesktop8MB_TXE1.375MB.bat
	$(CP) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin $(AMI_DESKTOP_ROM_8MB_TXE_1_375MB)
	$(RM) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin
endif
ifeq ($(TXE_1_5MB_SUPPORT), 1)
	$(ROM_IMAGE_DIR)$(PATH_SLASH)fitcbuildDesktop8MB_TXE1.5MB.bat
	$(CP) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin $(AMI_DESKTOP_ROM_8MB_TXE_1_5MB)
	$(RM) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin
endif
ifeq ($(TXE_3MB_SUPPORT), 1)
	$(ROM_IMAGE_DIR)$(PATH_SLASH)fitcbuildDesktop8MB_TXE3MB.bat
	$(CP) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin $(AMI_DESKTOP_ROM_8MB_TXE_3MB)
	$(RM) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin
endif
ifeq ($(BYTI_FEAT_TXE_SUPPORT), 1)
	$(ROM_IMAGE_DIR)$(PATH_SLASH)BYT-I_FEAT_TXE_KIT_RELEASE_1.0.2.1060v5.bat
	$(CP) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin $(AMI_BYTI_ROM_FEAT_TXE)
	$(RM) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin
endif
ifeq ($(BYTI_SLIM_TXE_SUPPORT), 1)
	$(ROM_IMAGE_DIR)$(PATH_SLASH)BYT-I_SLIM_TXE_KIT_RELEASE_1.0.2.1067.bat
	$(CP) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin $(AMI_BYTI_ROM_SLIM_TXE)
	$(RM) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin
endif
ifeq ($(BYTI_DUAL_BOOT_TXE_SUPPORT), 1)
	$(ROM_IMAGE_DIR)$(PATH_SLASH)BYT-I_DUAL_BOOT_TXE_KIT_RELEASE_1.1.0.1089.bat
	$(CP) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin $(AMI_BYTI_ROM_DUAL_BOOT_TXE)
	$(RM) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin
endif
else
ifeq ($(PLATFORM_FLAVOR_SELECT), 1)
ifeq ($(TXE_1_25MB_SUPPORT), 1)
	$(ROM_IMAGE_DIR)$(PATH_SLASH)fitcbuildNotebook8MB_TXE1.25MB.bat
	$(CP) $(BUILD_DIR)$(PATH_SLASH)Notebook.bin $(AMI_NOTEBOOK_ROM_8MB_TXE_1_25MB)
	$(RM) $(BUILD_DIR)$(PATH_SLASH)Notebook.bin
endif
ifeq ($(TXE_1_375MB_SUPPORT), 1)
	$(ROM_IMAGE_DIR)$(PATH_SLASH)fitcbuildNotebook8MB_TXE1.375MB.bat
	$(CP) $(BUILD_DIR)$(PATH_SLASH)Notebook.bin $(AMI_NOTEBOOK_ROM_8MB_TXE_1_375MB)
	$(RM) $(BUILD_DIR)$(PATH_SLASH)Notebook.bin
endif
ifeq ($(TXE_1_5MB_SUPPORT), 1)
	$(ROM_IMAGE_DIR)$(PATH_SLASH)fitcbuildNotebook8MB_TXE1.5MB.bat
	$(CP) $(BUILD_DIR)$(PATH_SLASH)Notebook.bin $(AMI_NOTEBOOK_ROM_8MB_TXE_1_5MB)
	$(RM) $(BUILD_DIR)$(PATH_SLASH)Notebook.bin
endif
ifeq ($(TXE_3MB_SUPPORT), 1)
	$(ROM_IMAGE_DIR)$(PATH_SLASH)fitcbuildNotebook8MB_TXE3MB.bat
	$(CP) $(BUILD_DIR)$(PATH_SLASH)Notebook.bin $(AMI_NOTEBOOK_ROM_8MB_TXE_3MB)
	$(RM) $(BUILD_DIR)$(PATH_SLASH)Notebook.bin
endif
else
ifeq ($(TXE_1_25MB_SUPPORT), 1)
	$(ROM_IMAGE_DIR)$(PATH_SLASH)fitcbuildDesktop8MB_TXE1.25MB.bat
	$(CP) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin $(AMI_DESKTOP_ROM_8MB_TXE_1_25MB)
	$(RM) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin
endif
ifeq ($(TXE_1_375MB_SUPPORT), 1)
	$(ROM_IMAGE_DIR)$(PATH_SLASH)fitcbuildDesktop8MB_TXE1.375MB.bat
	$(CP) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin $(AMI_DESKTOP_ROM_8MB_TXE_1_375MB)
	$(RM) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin
endif
ifeq ($(TXE_1_5MB_SUPPORT), 1)
	$(ROM_IMAGE_DIR)$(PATH_SLASH)fitcbuildDesktop8MB_TXE1.5MB.bat
	$(CP) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin $(AMI_DESKTOP_ROM_8MB_TXE_1_5MB)
	$(RM) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin
endif
ifeq ($(TXE_3MB_SUPPORT), 1)
	$(ROM_IMAGE_DIR)$(PATH_SLASH)fitcbuildDesktop8MB_TXE3MB.bat
	$(CP) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin $(AMI_DESKTOP_ROM_8MB_TXE_3MB)
	$(RM) $(BUILD_DIR)$(PATH_SLASH)Desktop.bin
endif
endif
endif

	$(RM) $(ROM_IMAGE_DIR)$(PATH_SLASH)BIOS.ROM
endif

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
