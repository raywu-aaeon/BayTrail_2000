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
# $Date: $
#*************************************************************************
#<AMI_FHDR_START>
#
# Name: PHashDPei.mak
#
# Description: 
#
#<AMI_FHDR_END>
#*************************************************************************
Prepare: PHashDPei

.PHONY : PHashDPei

#############################################################################
End: $(HASH_FV_SECOND_STAGE_KEY_FILE) $(HASH_FV_BB_KEY_FILE) $(BIOS_IMAGE_INFO)

$(HASH_FV_SECOND_STAGE_KEY_FILE): $(FD_FILE)
ifeq ($(SecureMod_SUPPORT), 1)
ifeq ($(FWCAPSULE_CERT_FORMAT), 0)
ifneq ($(wildcard $(FWpriv)),$(FWpriv))
	split -f $(FD_FILE) -s $(HASH_DATA_OFFSET_1) -o $(basename $(FD_FILE)).$(HASH_DATA_OFFSET_1) -t $(basename $(FD_FILE)).$(HASH_DATA_OFFSET_2)
	split -f $(basename $(FD_FILE)).$(HASH_DATA_OFFSET_2) -s $(HASH_DATA_SIZE) -o $(basename $(FD_FILE)).$(HASH_DATA_SIZE)
	$(CRYPTCON) -h2 -f $(basename $(FD_FILE)).$(HASH_DATA_SIZE) -o $@
else
	split -f $(FWCAPSULE_FILE_NAME) -s $(HASH_DATA_OFFSET_1) -o $(basename $(BUILD_DIR)$(PATH_SLASH)$(FWCAPSULE_FILE_NAME)).$(HASH_DATA_OFFSET_1) -t $(basename $(BUILD_DIR)$(PATH_SLASH)$(FWCAPSULE_FILE_NAME)).$(HASH_DATA_OFFSET_2)
	split -f $(basename $(BUILD_DIR)$(PATH_SLASH)$(FWCAPSULE_FILE_NAME)).$(HASH_DATA_OFFSET_2) -s $(HASH_DATA_SIZE) -o $(basename $(BUILD_DIR)$(PATH_SLASH)$(FWCAPSULE_FILE_NAME)).$(HASH_DATA_SIZE)
	$(CRYPTCON) -h2 -f $(basename $(BUILD_DIR)$(PATH_SLASH)$(FWCAPSULE_FILE_NAME)).$(HASH_DATA_SIZE) -o $@
endif
else
ifeq ($(FWCAPSULE_CERT_FORMAT), 1)
ifneq ($(wildcard $(FWpriv)),$(FWpriv))
	split -f $(FD_FILE) -s $(HASH_DATA_OFFSET_1) -o $(basename $(FD_FILE)).$(HASH_DATA_OFFSET_1) -t $(basename $(FD_FILE)).$(HASH_DATA_OFFSET_2)
	split -f $(basename $(FD_FILE)).$(HASH_DATA_OFFSET_2) -s $(HASH_DATA_SIZE) -o $(basename $(FD_FILE)).$(HASH_DATA_SIZE)
	$(CRYPTCON) -h2 -f $(basename $(FD_FILE)).$(HASH_DATA_SIZE) -o $@
else
ifneq ($(wildcard $(FWpub)),$(FWpub))
	split -f $(FD_FILE) -s $(HASH_DATA_OFFSET_1) -o $(basename $(FD_FILE)).$(HASH_DATA_OFFSET_1) -t $(basename $(FD_FILE)).$(HASH_DATA_OFFSET_2)
	split -f $(basename $(FD_FILE)).$(HASH_DATA_OFFSET_2) -s $(HASH_DATA_SIZE) -o $(basename $(FD_FILE)).$(HASH_DATA_SIZE)
	$(CRYPTCON) -h2 -f $(basename $(FD_FILE)).$(HASH_DATA_SIZE) -o $@
else
	split -f $(FWCAPSULE_FILE_NAME) -s $(HASH_DATA_OFFSET_1) -o $(basename $(BUILD_DIR)$(PATH_SLASH)$(FWCAPSULE_FILE_NAME)).$(HASH_DATA_OFFSET_1) -t $(basename $(BUILD_DIR)$(PATH_SLASH)$(FWCAPSULE_FILE_NAME)).$(HASH_DATA_OFFSET_2)
	split -f $(basename $(BUILD_DIR)$(PATH_SLASH)$(FWCAPSULE_FILE_NAME)).$(HASH_DATA_OFFSET_2) -s $(HASH_DATA_SIZE) -o $(basename $(BUILD_DIR)$(PATH_SLASH)$(FWCAPSULE_FILE_NAME)).$(HASH_DATA_SIZE)
	$(CRYPTCON) -h2 -f $(basename $(BUILD_DIR)$(PATH_SLASH)$(FWCAPSULE_FILE_NAME)).$(HASH_DATA_SIZE) -o $@
endif
endif
endif
endif
else
	split -f $(FD_FILE) -s $(HASH_DATA_OFFSET_1) -o $(basename $(FD_FILE)).$(HASH_DATA_OFFSET_1) -t $(basename $(FD_FILE)).$(HASH_DATA_OFFSET_2)
	split -f $(basename $(FD_FILE)).$(HASH_DATA_OFFSET_2) -s $(HASH_DATA_SIZE) -o $(basename $(FD_FILE)).$(HASH_DATA_SIZE)
	$(CRYPTCON) -h2 -f $(basename $(FD_FILE)).$(HASH_DATA_SIZE) -o $@
endif

$(HASH_FV_BB_KEY_FILE): $(FV_BB_FILE)
	$(CRYPTCON) -h2 -f $(FV_BB_FILE) -o $@

$(BIOS_IMAGE_INFO): $(HASH_FV_SECOND_STAGE_KEY_FILE) $(HASH_FV_BB_KEY_FILE)
	AmiModulePkg\HashDataPei\insert.exe CreateIncFromBin $(BUILD_DIR)\HashSecondStageKey.inc $(HASH_FV_SECOND_STAGE_KEY_FILE)
	AmiModulePkg\HashDataPei\insert.exe CreateIncFromBin $(BUILD_DIR)\HashFvBbKey.inc $(HASH_FV_BB_KEY_FILE)
	copy AmiModulePkg\HashDataPei\GenBiosImageInfo.asm $(BUILD_DIR)\GenBiosImageInfo.asm
	ml /c /nologo /Fo$(BUILD_DIR)\GenBiosImageInfo.obj $(BUILD_DIR)\GenBiosImageInfo.asm
	$(CCX86DIR)\link /NOENTRY /FIXED /DLL $(BUILD_DIR)\GenBiosImageInfo.obj /OUT:$(BUILD_DIR)\GenBiosImageInfo.dll
	genfw --exe2bin $(BUILD_DIR)\GenBiosImageInfo.dll -o $(BUILD_DIR)\GenBiosImageInfo.bin
	split -f $(BUILD_DIR)\GenBiosImageInfo.bin -s 0x64 -o $(BIOS_IMAGE_INFO)

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
