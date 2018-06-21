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
# Name: VerifyBoot.mak
#
# Description: 
#
#<AMI_FHDR_END>
#*************************************************************************
.PHONY: DeleteManifestGenerationFile $(FD_MANIFEST_FILE)

Prepare: DeleteManifestGenerationFile

DeleteManifestGenerationFile:
	@if exist "FD_PRE_BB_127K.bin" @$(RM) "FD_PRE_BB_127K.bin"
	@if exist "$(MANIFEST_NAME)_SB_*.*" @$(RM) "$(MANIFEST_NAME)_*.*"
	@if exist "$(notdir $(FD_MANIFEST_FILE))" @$(RM) "$(notdir $(FD_MANIFEST_FILE))"
	@if exist "$(ROM_IMAGE_DIR)$(PATH_SLASH)FpfMirrorNvarValues.txt" @$(RM) "$(ROM_IMAGE_DIR)$(PATH_SLASH)FpfMirrorNvarValues.txt"

End: $(FD_MANIFEST_FILE)

$(FD_MANIFEST_FILE): $(FD_FILE) $(BIOS_IMAGE_INFO)
ifeq ($(SecureMod_SUPPORT), 1)
ifeq ($(FWCAPSULE_CERT_FORMAT), 0)
ifneq ($(wildcard $(FWpriv)),$(FWpriv))
	split -f $(FD_FILE) -s $(FD_NON_PRE_BB_SIZE) -o $(FD_NON_PRE_BB_FILE) -t $(FD_PRE_BB_FILE)
else
	split -f $(FWCAPSULE_FILE_NAME) -s $(FD_NON_PRE_BB_SIZE) -o $(FD_NON_PRE_BB_FILE) -t $(FD_PRE_BB_FILE)
endif
else
ifeq ($(FWCAPSULE_CERT_FORMAT), 1)
ifneq ($(wildcard $(FWpriv)),$(FWpriv))
	split -f $(FD_FILE) -s $(FD_NON_PRE_BB_SIZE) -o $(FD_NON_PRE_BB_FILE) -t $(FD_PRE_BB_FILE)
else
ifneq ($(wildcard $(FWpub)),$(FWpub))
	split -f $(FD_FILE) -s $(FD_NON_PRE_BB_SIZE) -o $(FD_NON_PRE_BB_FILE) -t $(FD_PRE_BB_FILE)
else
	split -f $(FWCAPSULE_FILE_NAME) -s $(FD_NON_PRE_BB_SIZE) -o $(FD_NON_PRE_BB_FILE) -t $(FD_PRE_BB_FILE)
endif
endif
endif
endif
else
	split -f $(FD_FILE) -s $(FD_NON_PRE_BB_SIZE) -o $(FD_NON_PRE_BB_FILE) -t $(FD_PRE_BB_FILE)
endif
	split -f $(FD_PRE_BB_FILE) -s $(FD_PRE_BB_1K_SIZE) -o $(FD_PRE_BB_1K_FILE) -t $(FD_PRE_BB_127K_FILE)
	$(FLAMInGoKit) HashKey $(PUBLIC_KEY_FILE) $(BUILD_DIR)$(PATH_SLASH)$(addsuffix Hash.txt,$(basename $(notdir $(PUBLIC_KEY_FILE))))
	call $(VERIFY_BOOT_DIR)$(PATH_SLASH)FusesConfiguration.bat $(BUILD_DIR)$(PATH_SLASH)$(addsuffix Hash.txt,$(basename $(notdir $(PUBLIC_KEY_FILE)))) $(FUSE_CONFIG_FILE)
ifeq ($(PASS_FPFMIRROR_FILE_TO_FITC), 1)
	$(CP) /Y /B "$(BUILD_DIR)$(PATH_SLASH)FpfMirrorNvarValues.txt" "$(ROM_IMAGE_DIR)$(PATH_SLASH)FpfMirrorNvarValues.txt"
endif
	$(CP) /Y /B $(FD_PRE_BB_127K_FILE) FD_PRE_BB_127K.bin
ifeq ($(BIOS_IMAGE_INFO),)
	$(FLAMInGoKit) SBManCreate $(BUILD_DIR)$(PATH_SLASH)$(notdir $(FUSE_CONFIG_FILE)) $(MANIFEST_NAME) $(FD_PRE_BB_127K_FILE) $(SVN) $(PUBLIC_KEY_FILE)
else
	$(FLAMInGoKit) SBManCreate $(BUILD_DIR)$(PATH_SLASH)$(notdir $(FUSE_CONFIG_FILE)) $(MANIFEST_NAME) $(FD_PRE_BB_127K_FILE) $(SVN) $(PUBLIC_KEY_FILE) -OEMDataFile $(BIOS_IMAGE_INFO)
endif
	$(SimpleSignerKit) $(PRIVATE_KEY_FILE) $(MANIFEST_NAME)_SB_hash.bin $(MANIFEST_NAME)_SB_signature.bin
	$(FLAMInGoKit) SBManComplete $(BUILD_DIR)$(PATH_SLASH)$(notdir $(FUSE_CONFIG_FILE)) $(MANIFEST_NAME) $(MANIFEST_NAME)_SB_signature.bin
	$(CP) /Y /B $(FD_NON_PRE_BB_FILE)+$(MANIFEST_NAME)_SB_manifest.bin $@

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