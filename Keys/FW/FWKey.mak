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
#<AMI_FHDR_START>
#
# Name: FWKey.mak
#
# Description: includes Root Platform key (PR) into the BIOS rom
# 
#<AMI_FHDR_END>
#*************************************************************************
#$(warning FWKey.mak evaluation......)
# empty
#ifeq ($(wildcard $(PROJECT_SETUP_HEADER)), "")
#file exists
#ifeq ($(wildcard $(PROJECT_LDL)), $(PROJECT_LDL))

#---------------------------------------------------------------------------
#   Include Platform Firmware Root Key (FWKey) .FFS
#---------------------------------------------------------------------------
Prepare: $(BUILD_DIR)\FWKEY.BIN

ifneq ($(wildcard $(FWpub)), $(FWpub))
#---------------------------------------------------------------------------
#   Create a scratchpad buff for a dummy key placeholder
#---------------------------------------------------------------------------
$(BUILD_DIR)\FWKEY.BIN: $(FWKey_DIR)\FWKey.mak
#Merge64 FwKey pad.
	$(ECHO)  \
"output$(EOL)\
    FWKEY_FILE($@)$(EOL)\
end$(EOL)\
group FWKEY_FILE$(EOL)\
    upper=0xffffffff$(EOL)\
components$(EOL)\
blank MICROCODE_PAD$(EOL)\
    size=$(FWKEY_FILE_SIZE)$(EOL)\
    pattern=(0xFF)$(EOL)\
end$(EOL)\
end end"\
>$(BUILD_DIR)\Fwkey.ini
	Merge64 /s $(BUILD_DIR)\Fwkey.ini

#---------------------------------------------------------------------------
else  # pub keys
#---------------------------------------------------------------------------
#   Prepare Platform Firmware Root Key (FWKey) bin file
#---------------------------------------------------------------------------
$(FWpub): $(FWKey_DIR)\FWKey.mak

$(BUILD_DIR)\FWKEY.BIN : $(FWpub)
ifeq ($(FWKEY_FILE_FORMAT),2)
#x509 Cert Key
	$(CP) $< $@
else
#Extract 256byte n-modulus from x509 DER or PKCS#1v2 ASN.1 encoded RSA Key
# n-modulus can be extracted either from Public Key FWpub or full RSA Key FWpriv files
	$(CRYPTCON) -w -k $< -o $@
ifeq ($(FWKEY_FILE_FORMAT),1)
#get SHA256 Hash of n-modulus of RSA Key
	$(CRYPTCON) -h2 -f $@ -o $@
endif
endif
endif #ifneq ($(wildcard $(FWpub)), $(FWpub))
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
