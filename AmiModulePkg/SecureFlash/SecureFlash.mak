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
# Name: SecureMod.mak
#
# Description: Includes main build module for Secure sub-components
# 
#   1. Include FWkey ffs with Root Platform key into the BIOS FV_BB
#   2. Include FwCapsuleHdr ffs with Rom signing map into the BIOS FV_MAIN
#   3. Create signed BIOS image (Aptio FW Capsule)
#
#<AMI_FHDR_END>
#*************************************************************************
Prepare: SecureModule

.PHONY : SecureModule
.PHONY : CLEAN_FWCAPSULE_FILE AMI_MAKE_SIGN_ROM_BATCH_FILE $(FWCAPSULE_FILE_NAME)

#---------------------------------------------------------------------------
#        Prepare Signed Capsule : FWCAPSULE_FILE_NAME
#---------------------------------------------------------------------------
SecureModule: CLEAN_FWCAPSULE_FILES
CLEAN_FWCAPSULE_FILES:
	@if exist $(FWCAPSULE_FILE_NAME) @del $(FWCAPSULE_FILE_NAME)
	@if exist make_sign_capsule.bat @del make_sign_capsule.bat
	@if exist make_sign_capsule_readme.txt @del make_sign_capsule_readme.txt
		
#---------------------------------------------------------------------------
# Only for PKCS1v2.1 Key files: re-assign FWpub = FWpriv only if
# following codition is true: FWpub file not defined, FWpriv file is defined
#---------------------------------------------------------------------------
ifneq ($(FWKEY_FILE_FORMAT),2)
ifneq ($(wildcard $(FWpub)), $(FWpub))
ifeq ($(wildcard $(FWpriv)),$(FWpriv))
FWpub := $(FWpriv)
endif
endif
endif

#---------------------------------------------------------------------------
# Should be the last step after creating of the ROM image. All fixups to the .ROM must be made prior to this step.
# check END target in the MAIN.MAK and all .MAK files to make sure this step is not overriden
#---------------------------------------------------------------------------

#---------------------------------------------------------------------------
#Create make_sign_capsule.bat        - include cryptocon command prompt to sign input BIOS image
#Create make_sign_capsule_readme.txt - include instructions to run the .bat file
#---------------------------------------------------------------------------
End: AMI_MAKE_SIGN_ROM_BATCH_FILE
AMI_MAKE_SIGN_ROM_BATCH_FILE: $(UNSIGNED_BIOS_ROM)
	@$(ECHO) "$(CRYPTCON) $(CRYPTOCON_CMDLINE)" > make_sign_capsule.bat
	@$(ECHO) "======================================================================\
$(EOL)=== INSTRUCTIONS TO CREATE SIGNED APTIO BIOS IMAGE ===\
$(EOL)======================================================================"\
> make_sign_capsule_readme.txt
ifneq ($(wildcard $(FWpriv)),$(FWpriv))
	@$(ECHO) "\
$(EOL)BUILD ERROR!!!\
$(EOL)Missing RSA private key FWpriv=$(FWpriv) to sign BIOS image."\
>>make_sign_capsule_readme.txt
endif
ifneq ($(wildcard $(FWpub)),$(FWpub))
	@$(ECHO) "\
$(EOL)BUILD WARNING!!!\
$(EOL)Missing RSA public key FWpub=$(FWpub) to verify Signed BIOS updates.\
$(EOL)$(FWKEY_FILE_SIZE) byte dummy Key inserted into BIOS RTU (FV_BB).\
$(EOL)Flash Updates or Recovery will fail on $(UNSIGNED_BIOS_ROM) BIOS\
$(EOL)unless the dummy Key is replaced with a valid public key during BIOS signing."\
>> make_sign_capsule_readme.txt
endif
ifeq ($(FWCAPSULE_CERT_FORMAT),1)
ifeq ($(CONFIG_PEI_PKCS7),0)
	@$(ECHO) "$(EOL)BUILD WARNING!!!\
$(EOL)Flash update is disabled for PKCS#7 Signed Fw Capsules.\
$(EOL)Set SDL Token "CONFIG_PEI_PKCS7" to 1 to enable PKCS#7 verification in PEI."\
>> make_sign_capsule_readme.txt
endif
	@$(ECHO) "\
$(EOL)======================================================================\
$(EOL)1. Provide RSA private key in PKCS#12 PFX and public key in X.509 DER formats."\
>> make_sign_capsule_readme.txt
else
	@$(ECHO) "\
$(EOL)======================================================================\
$(EOL)1. Provide RSA 2048 bit key in PKCS#1v2.1 (BIN,PEM) format."\
>> make_sign_capsule_readme.txt
endif
	@$(ECHO) "\
$(EOL)   a. Use customer provided key files.\
$(EOL)   b. Use AMI provided Test key.\
$(EOL)   c. Generate a Test key using 3rd party tools:\
$(EOL)      Use AMI Keygen.exe to create PKCS#1v2.1 RSA Key pair,\
$(EOL)        e.g keygen privkey.bin pubkey.bin\
$(EOL)      Use openSSL.exe to create PKCS#1v2.1 RSA Key pair,\
$(EOL)        e.g openssl genrsa -out privkey.pem 2048\
$(EOL)            openssl rsa -in privkey.pem -pubout -out pubkey.pem\
$(EOL)      Or use MS WinSDK Makecert.exe to generate PKCS#12 PFX Key.\
$(EOL)\
$(EOL)2. Update 2 SDL Tokens with path to Sign key files:\
$(EOL)   FWpriv - file with Full RSA key\
$(EOL)   FWpub  - file with Public portion of RSA key\
$(EOL)3. Provide ready to be signed BIOS ROM file which was created with\
$(EOL)\
$(EOL)   Secure Flash support enabled and CREATE_FWCAPSULE set to 1 or 2.\
$(EOL)\
$(EOL)4. Create signed BIOS file.\
$(EOL)   Launch Cryptocon.exe with command line:\
$(EOL)\
$(EOL)   Cryptocon.exe $(CRYPTOCON_CMDLINE)\
$(EOL)\
$(EOL)======================================================================\
$(EOL)=== vital Cryptocon.exe input arguments ===\
$(EOL)======================================================================\
$(EOL) -f $(UNSIGNED_BIOS_ROM) - input, unsigned BIOS.ROM\
$(EOL) -o $(FWCAPSULE_FILE_NAME) - output, signed BIOS image\
$(EOL) -y embed FwCapsule Header inside signed BIOS image\
$(EOL) -l 'value' FwCapsule Header size/alignment\
$(EOL) -c'FWrootKey' -k'FWpriv'- Create PKCS1v1.5 signed FwCapsule\
$(EOL)     -c, -k list the file(s) with PKCS#1v2.1 RSA key to sign BIOS\
$(EOL) -c2 -x 'FWpriv','pswd' - Create PKCS#7 signed FwCapsule\
$(EOL) -c2 -x command will invoke signtool.exe as external process\
$(EOL) -n replace BIOS Platform Root key with a public portion of '-k' key\
$(EOL) -r2 or -r'rom.map' instruction to locate/embed rom map descriptor file\
$(EOL)"\
>>make_sign_capsule_readme.txt

#--------------------------------------------------------------------------
# 1. Creating Signing descriptor table (RomLayout map) file
#--------------------------------------------------------------------------
$(ROM_LAYOUT_EX): $(UNSIGNED_BIOS_ROM) 
	@if not exist $@ FWBUILD $< /s /m $@
	
#---------------------------------------------------------------------------
# 2. Embedding Signing descriptor table "$(ROM_LAYOUT_EX)" inside "$(UNSIGNED_BIOS_ROM)"
# 3. Invoke cryptocon.exe to create Signed FwCapsule if CREATE_FWCAPSULE == 1
#---------------------------------------------------------------------------
End: $(FWCAPSULE_FILE_NAME)
$(FWCAPSULE_FILE_NAME): $(UNSIGNED_BIOS_ROM) $(ROM_LAYOUT_EX)
	$(CRYPTCON) -c2 -y -r$(ROM_LAYOUT_EX) -l$(FWCAPSULE_MAX_HDR_SIZE) -f $(UNSIGNED_BIOS_ROM) -o $(UNSIGNED_BIOS_ROM)
ifeq ($(FWCAPSULE_CERT_FORMAT),1)
ifneq ($(CONFIG_PEI_PKCS7),)
ifeq ($(CONFIG_PEI_PKCS7),0)
	@echo -----
	@echo ----- WARNING!!! PKCS#7 verification is disabled in PEI phase of Flash update.
	@echo ----- Enable PKCS#7 support in PEI Crypto library via SDL Token "CONFIG_PEI_PKCS7"
endif
endif
endif
ifeq ($(CREATE_FWCAPSULE),1)
	-$(CRYPTCON) $(CRYPTOCON_CMDLINE)
ifneq ($(wildcard $(FWpriv)),$(FWpriv))
	@echo -----
	@echo ----- ERROR!!! Unable to create signed BIOS Image "$(FWCAPSULE_FILE_NAME)"
	@echo ----- Missing RSA private key FWpriv=$(FWpriv) to sign BIOS image.
ifeq ($(FWCAPSULE_CERT_FORMAT),0)
	@echo ----- Expected Key file format: PKCS#1v2.1 DER (BIN,PEM)
else
	@echo ----- Expected Key file format: PKCS#12 PFX
endif
	@echo ----- Follow instructions in make_sign_capsule_readme.txt.
	@echo -----
else
#ifeq ($(FWCAPSULE_FILE_FORMAT),0)
# Target FWCAPSULE_FILE_NAME file is an original unsigned .ROM with Fw Signature block embedded as Ffs file
# rename target file name to UNSIGNED_BIOS_ROM if this file will be used as input to Intel fitc or other Flash image packaging tool
#	$(CP) $(FWCAPSULE_FILE_NAME) $(UNSIGNED_BIOS_ROM)
#endif
endif # ($(wildcard $(FWpriv)),$(FWpriv))
endif #ifeq  ($(CREATE_FWCAPSULE),1)
ifneq ($(wildcard $(FWpub)), $(FWpub))
#---------------------------------------------------------------------------
#   Display Warning message at the end of Build log if FWpub key is missing
#---------------------------------------------------------------------------
	@echo ----- 
	@echo ----- WARNING!!! Missing RSA public key FWpub="$(FWpub)" to verify Signed BIOS updates.
ifeq ($(FWCAPSULE_CERT_FORMAT),0)
	@echo ----- Expected Key file format: PKCS#1v2.1 DER (BIN,PEM)
else
	@echo ----- Expected Key file format: X509 CER
endif
	@echo ----- $(FWKEY_FILE_SIZE) byte dummy Key inserted into BIOS RTU (FV_BB).
	@echo ----- WARNING!!! Flash Updates or Recovery will fail on "$(UNSIGNED_BIOS_ROM)" BIOS
	@echo ----- unless the dummy Key is replaced with a valid public key during BIOS signing.
	@echo ----- 
endif
#Example of using the msft signtool and ami cryptocon cmdline scripts at the back end of a remote signing server
#Make sure to use the version of Microsoft SignTool.exe that supports /p7 switch
#Latest tool version can be downloaded as part of Win8 SDK from http://msdn.microsoft.com/en-us/windows/hardware/hh852363.aspx. 
# 1. Create serialized data fwcap_serialized to be signed in step 2
#	$(CRYPTCON) -c2 -s -y -l $(FWCAPSULE_IMAGE_ALLIGN) -r2 -f $(UNSIGNED_BIOS_ROM) -o $(BUILD_DIR)\fwcap_serialized
# 2. Create .p7 certificate file by signing of Sign fwcap_serialized. May include invoking of a remote signing server
#	signtool sign /fd sha256 /p7 $(BUILD_DIR) /p7co 1.2.840.113549.1.7.1 /p7ce DetachedSignedData /a /f $(FWpriv) /p$(FW_PFX_Password) $(BUILD_DIR)\fwcap_serialized
# 3. Creating Fw Capsule image $(FWCAPSULE_FILE_NAME) with .p7 signature embedded into a FwCap header
#	$(CRYPTCON) -c2 -s -x $(BUILD_DIR)\fwcap_serialized.p7 -y -l $(FWCAPSULE_IMAGE_ALLIGN) -r2 -f $(UNSIGNED_BIOS_ROM) -o $(FWCAPSULE_FILE_NAME)
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
