#*************************************************************************
#*************************************************************************
#**                                                                     **
#**        (C)Copyright 1985-2016, American Megatrends, Inc.            **
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
# Name: SecureMod.mak
#
# Description: Includes main build module for Secure sub-components
# 
#   1. Create error log file and a help file how to sign Fw Capsule using CryptoCon.exe
#   2. Create extended Rom layout map file RomLayoutEx.bin 
#   3. Update FwCapsuleHdr ffs file inside BIOS.ROM with RomLayoutEx map 
#   4. Create signed BIOS image (Aptio FW Capsule)
#
#<AMI_FHDR_END>
#*************************************************************************
Prepare: SecureModule

.PHONY : SecureModule
.PHONY : CLEAR_FWCAPSULE_FILES AMI_MAKE_SIGN_ROM_BATCH_FILE MOD_FWCAPSULE_HDR_FFS $(FWCAPSULE_FILE_NAME)

#---------------------------------------------------------------------------
# Only for PKCS1v2.1 Key files the FWpub(public key part) can be derived 
# from FWpriv(full RSA key) if FWpub file is not available
#---------------------------------------------------------------------------
ifeq ($(FWCAPSULE_CERT_FORMAT),0)
ifneq ($(wildcard $(FWpub)), $(FWpub))
ifeq ($(wildcard $(FWpriv)),$(FWpriv))
FWpub := $(FWpriv)
endif
endif
endif

#---------------------------------------------------------------------------
#        Prepare Signed Capsule : FWCAPSULE_FILE_NAME
#---------------------------------------------------------------------------
Prepare: CLEAR_FWCAPSULE_FILES MAKE_FWCAPSULE_HELP_FILES

CLEAR_FWCAPSULE_FILES:
ifeq ($(wildcard $(FWCAPSULE_FILE_NAME)), $(FWCAPSULE_FILE_NAME))
	-$(RM) $(FWCAPSULE_FILE_NAME)
endif
ifeq ($(wildcard make_sign_capsule.bat), make_sign_capsule.bat)
	-$(RM) make_sign_capsule*.*
endif

#---------------------------------------------------------------------------
#   1. Create error log and batch files with instructions to sign Fw Capsule 
#      using CryptoCon.exe
#---------------------------------------------------------------------------
#Create make_sign_capsule.bat - cryptocon command line to sign BIOS image
#Create make_sign_capsule_error.log - error log
#Create make_sign_capsule_readme.txt - simple instructions to create signed FwCapsule
#---------------------------------------------------------------------------
MAKE_FWCAPSULE_HELP_FILES: 
	@$(ECHO) "@echo ----run FWBUILD to build rom map file for this BIOS image\
$(EOL)@echo FWBUILD $(UNSIGNED_BIOS_ROM) -s -m $(ROM_LAYOUT_EX)\
$(EOL)@echo ----sign BIOS image using external rom map\
$(EOL)$(CRYPTCON) -r $(ROM_LAYOUT_EX) $(CRYPTOCON_CMDLINE_SIG) -f $(UNSIGNED_BIOS_ROM) -o $(FWCAPSULE_FILE_NAME)"\
> make_sign_capsule.bat
ifneq ($(wildcard $(FWpriv)),$(FWpriv))
	@$(ECHO) "\
$(EOL)WARNING!!! Missing RSA private key FWpriv=$(FWpriv) to sign BIOS image."\
>>make_sign_capsule_error.log
endif
ifneq ($(wildcard $(FWpub)),$(FWpub))
	@$(ECHO) "\
$(EOL)WARNING!!! Missing RSA public key FWpub=$(FWpub) to verify Signed BIOS updates.\
$(EOL)           Platform Key FFS file (FV_BB) is filled with the Dummy Key as a placeholder.\
$(EOL)WARNING!!! Flash updates are blocked in '$(notdir $(UNSIGNED_BIOS_ROM))' BIOS image\
$(EOL)           unless the Platform Key FFS(FV_BB) is updated with the valid Key."\
>> make_sign_capsule_error.log
endif
ifeq ($(FWCAPSULE_CERT_FORMAT),1)
ifeq ($(CONFIG_PEI_PKCS7),0)
	@$(ECHO) "\
$(EOL)WARNING!!! Flash update via Capsule or Recovery is disabled for PKCS#7 Signed Fw Capsules.\
$(EOL)           Set SDL Token CONFIG_PEI_PKCS7 to 1 to enable PKCS#7 verification in PEI."\
>> make_sign_capsule_error.log
endif
endif
	@$(ECHO) "=============================================================\
$(EOL)=== EXAMPLE OF SIGNING APTIO FW IMAGE USING CRYPTOCON.EXE ===\
$(EOL)=============================================================\
$(EOL)1. Use Aptio FW image '$(notdir $(UNSIGNED_BIOS_ROM))' created on\
$(EOL)    $(TODAY) at $(NOW) and including:\
$(EOL)    a)Platform Key ffs with a public key to verify signed BIOS updates;\
$(EOL)    b)FwCapsule Hdr ffs populated with Capsule signing parameters:\
$(EOL)      $(notdir $(CRYPTOCON_CMDLINE_MAP))\
$(EOL)"\
>> make_sign_capsule_readme.txt
ifeq ($(FWCAPSULE_CERT_FORMAT),1)
	@$(ECHO) "2. Provide RSA key in PKCS#12 PFX(private) and X.509 DER(public) file formats."\
>> make_sign_capsule_readme.txt
else
	@$(ECHO) "2. Provide 2048bit RSA Key file in PKCS#1v2.1 DER (PEM)format.\
$(EOL)    Root Key Certificate key (full RSA key)  - $(notdir $(FWrootKey))"\
>> make_sign_capsule_readme.txt
endif
	@$(ECHO) "    Signing Certificate key (full RSA key) - $(notdir $(FWpriv))\
$(EOL)    Signing Certificate key (public key part) - $(notdir $(FWpub))\
$(EOL)\
$(EOL)=============================================================\
$(EOL)=== Run Cryptocon.exe script to generate Signed FwCapsule\
$(EOL)=============================================================\
$(EOL)\
$(EOL) $(notdir $(CRYPTCON)) $(notdir $(CRYPTOCON_CMDLINE_SIG)) -f file_in -o file_out\
$(EOL)\
$(EOL)=============================================================\
$(EOL)=== Common Cryptocon FwCapsule build instructions \
$(EOL)=============================================================\
$(EOL) -c'FWrootPriv' -k'FWsignPriv' Create PKCS#1v1.5 signed FwCapsule (Note1)\
$(EOL) -c2 -x 'FWpriv'[,'pswd']      Create PKCS#7 signed FwCapsule (Note2, Note3)\
$(EOL) -f'file' input, un-signed BIOS image\
$(EOL) -o'file' output, signed FwCapsule image\
$(EOL) -y update an embedded FwCapsule Header, default-Hdr attached on top of BIOS\
$(EOL) -l'value' max size of a FwCapsule Header (file alignment)\
$(EOL) -n -k'key' insert Signer public 'key' into a signed image\
$(EOL) -r'rom.map' use a rom map from the external file\
$(EOL) -m embed the FwCapsule Sign parameters without creating a signed image\
$(EOL)\
$(EOL)Note1. -c'key1'-k'key2'    :take PKCS#1v2.1 DER(PEM) encoded RSA2048 keys\
$(EOL)Note2. -c2 -x'key1'-k'key2':key1-PKCS#12(PFX) with optional PFX password;\
$(EOL)                            key2-X.509(DER) with public 'key1'\
$(EOL)Note3. -c2 -x command invokes external Msft signtool.exe\
$(EOL)=============================================================\
$(EOL)=== Extended Cryptocon FwCapsule build instructions\
$(EOL)=============================================================\
$(EOL) -c2 -s Create serialized data block based on the rom map info\
$(EOL) -c2 -s -x'p7.sig' import PKCS#7 signed data from file into a FwCapsule\
$(EOL) -r2 use embedded rom map data"\
>> make_sign_capsule_readme.txt

#---------------------------------------------------------------------------
# Should be the last step after creating of the ROM image. All fixups to the .ROM must be made prior to this step.
# check END target in the MAIN.MAK and all .MAK files to make sure this step is not overriden
#---------------------------------------------------------------------------

#--------------------------------------------------------------------------
# 2. Creating Signing descriptor table (RomLayout map) file
#--------------------------------------------------------------------------
$(ROM_LAYOUT_EX): $(UNSIGNED_BIOS_ROM) 
ifeq ($(BUILD_OS), $(BUILD_OS_LINUX))
	FWBuild $< -v -m $@
else
	FWBuild $< /v /m $@
endif

#---------------------------------------------------------------------------
# 3. Embed Signing descriptor table "$(ROM_LAYOUT_EX)" inside "$(UNSIGNED_BIOS_ROM)"
#--------------------------------------------------------------------------
MOD_FWCAPSULE_HDR_FFS: $(ROM_LAYOUT_EX) $(UNSIGNED_BIOS_ROM)
#dbg
#	$(CP) $(UNSIGNED_BIOS_ROM) $(UNSIGNED_BIOS_ROM).ORG
#dbg	
	@echo ----Update "$(notdir $(UNSIGNED_BIOS_ROM))" with extended rom map "$(notdir $(ROM_LAYOUT_EX))" and Capsule signing parameters
	$(CRYPTCON) $(CRYPTOCON_CMDLINE_MAP) -f $(UNSIGNED_BIOS_ROM) -o $(UNSIGNED_BIOS_ROM)

#---------------------------------------------------------------------------
# 4. Invoke cryptocon.exe to create Signed FwCapsule if CREATE_FWCAPSULE == 1
#---------------------------------------------------------------------------
$(FWCAPSULE_BUILD_TARGET): $(FWCAPSULE_FILE_NAME)
$(FWCAPSULE_FILE_NAME): MOD_FWCAPSULE_HDR_FFS $(UNSIGNED_BIOS_ROM) $(ROM_LAYOUT_EX)
ifeq ($(CREATE_FWCAPSULE),1)
	@echo ----Create signed BIOS image "$(FWCAPSULE_FILE_NAME)"
	-$(CRYPTCON) $(CRYPTOCON_CMDLINE_SIG) -f $(UNSIGNED_BIOS_ROM) -o $(FWCAPSULE_FILE_NAME)
#####
#An example of alternative way of using cryptocon to build PKCS#7 signed Fw Capsule.
#Cryptocon is used for packaging the capsule while the signing is done by an external tool
#ifeq ($(FWCAPSULE_CERT_FORMAT),1)
#
#1. Optional step if new BIOS verification key $(FWpub) needs to be embedded into $(UNSIGNED_BIOS_ROM)
#	$(CRYPTCON) -y -c2 -n -k $(FWpub) -f $(UNSIGNED_BIOS_ROM) -o $(UNSIGNED_BIOS_ROM)
#
#2. Create serialized data stream "fwcap_serialized" to be signed in step 3
#	$(CRYPTCON) -y -c2 -s -f $(UNSIGNED_BIOS_ROM) -o fwcap_serialized
#
#3. Create .p7 detached certificate file by signing of "fwcap_serialized" using Microsoft Signtool.exe:
#   Make sure to use to use the version of Microsoft SignTool.exe that supports /p7 switch
#   Latest tool version can be downloaded as part of Win8 SDK from http://msdn.microsoft.com/en-us/windows/hardware/hh852363.aspx. 
#    3.1 sign using a certificate whose private key information is protected by a hardware cryptography module (e.g. HSM). 
#    SignTool sign /fd sha256 /p7 $(BUILD_DIR) /p7co 1.2.840.113549.1.7.1 /p7ce DetachedSignedData /csp "Hardware Cryptography Module" /f SigCert.cer /k KeyContainerName fwcap_serialized
#        A computer store is specified for the certification authority (CA) store; Certificate is identified by a Subject Name "My High Value Certificate" .
#    SignTool sign /fd sha256 /p7 $(BUILD_DIR) /p7co 1.2.840.113549.1.7.1 /p7ce DetachedSignedData /sm /n "My High Value Certificate" fwcap_serialized
#    3.2 sign using a certificate stored in a password-protected PFX file "$(FWpriv)"
#	SignTool sign /fd sha256 /p7 . /p7co 1.2.840.113549.1.7.1 /p7ce DetachedSignedData /p "$(FW_PFX_Password)" /f $(FWpriv) fwcap_serialized
#
#4. Creating Fw Capsule image $(FWCAPSULE_FILE_NAME) with .p7 signature embedded into a FwCap header
#	$(CRYPTCON) -y -c2 -s -x fwcap_serialized.p7 -f $(UNSIGNED_BIOS_ROM) -o $(FWCAPSULE_FILE_NAME)
#endif # ifeq ($(FWCAPSULE_CERT_FORMAT),1)
#####
#
ifneq ($(BUILD_OS), $(BUILD_OS_LINUX))
	@if not exist $@ @echo ERROR   !!!   Failed to create signed BIOS Image.
endif	
#ifeq ($(FWCAPSULE_FILE_FORMAT),0)
# Target FWCAPSULE_FILE_NAME file is an original unsigned UNSIGNED_BIOS_ROM with Fw Signature block embedded as Ffs file
# Replace original UNSIGNED_BIOS_ROM with the signed one if this file will be used as input to Intel fitc or other Flash image packaging tool
#	@if exist $@ $(CP) $@ $(UNSIGNED_BIOS_ROM)
#endif
endif #ifeq  ($(CREATE_FWCAPSULE),1)
#---------------------------------------------------------------------------
# Display warning
#---------------------------------------------------------------------------
ifneq ($(BUILD_OS), $(BUILD_OS_LINUX))
	@if not exist $@ @echo WARNING!!! Do not use un-signed "$(notdir $(UNSIGNED_BIOS_ROM))" file as a Recovery or an update BIOS image.
endif	

#*************************************************************************
#*************************************************************************
#**                                                                     **
#**        (C)Copyright 1985-2016, American Megatrends, Inc.            **
#**                                                                     **
#**                       All Rights Reserved.                          **
#**                                                                     **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
#**                                                                     **
#**                       Phone: (770)-246-8600                         **
#**                                                                     **
#*************************************************************************
#*************************************************************************
