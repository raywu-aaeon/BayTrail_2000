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
# $Header: SdioDriver.mak $
#
# $Revision: $
#
# $Date: $
#**********************************************************************
#<AMI_FHDR_START>
#
# Name:	SdioDriver.mak
#
# Description:	Make file for SdioDriver component.
#
#<AMI_FHDR_END>
#**********************************************************************
Prepare : $(BUILD_DIR)\SdioDriver.inf $(BUILD_DIR)/SdioElink.h

$(BUILD_DIR)/SdioElink.h : $(BUILD_DIR)/token.mak
	$(ECHO) \
"#define SDIO_MANUFACTURE_DEVICE_LIST $(SdIoManufactureDeviceList)$(EOL)" \
> $(BUILD_DIR)/SdioElink.h

$(BUILD_DIR)\SdioDriver.inf : $(BUILD_DIR)/token.mak $(BUILD_DIR)/token.h $(SDIO_DRIVER_DIR)/SdioDriver.mak
	@$(ECHO)\
"$(EOL)\
#**********************************************************************$(EOL)\
#**********************************************************************$(EOL)\
#**                                                                  **$(EOL)\
#**        (C)Copyright 1985-2012, American Megatrends, Inc.         **$(EOL)\
#**                                                                  **$(EOL)\
#**                       All Rights Reserved.                       **$(EOL)\
#**                                                                  **$(EOL)\
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **$(EOL)\
#**                                                                  **$(EOL)\
#**                       Phone: (770)-246-8600                      **$(EOL)\
#**                                                                  **$(EOL)\
#**********************************************************************$(EOL)\
#**********************************************************************$(EOL)\
$(EOL)\
## @file$(EOL)\
  #   SDIO module to support SD controllers and devices.$(EOL)\
  #   This driver installs gSdioBusInitProtocolGuid.$(EOL)\
##$(EOL)\
$(EOL)\
[Defines]$(EOL)\
  INF_VERSION                    = 0x00010015$(EOL)\
  BASE_NAME                      = SdioDriver$(EOL)\
  FILE_GUID                      = 2BA0D612-C3AD-4249-915D-AA0E8709485F$(EOL)\
  MODULE_TYPE                    = UEFI_DRIVER$(EOL)\
  VERSION_STRING                 = 1.0$(EOL)\
  ENTRY_POINT                    = SdioDriverEntryPoint$(EOL)\
$(EOL)\
#$(EOL)\
# The following information is for reference only and not required by the build tools.$(EOL)\
#$(EOL)\
#  VALID_ARCHITECTURES           = IA32 X64 IPF EBC$(EOL)\
#$(EOL)\
$(EOL)\
  #   if SDIO_SMM_SUPPORT token is disabled then this [Sources] section $(EOL)\
  #   includes SdioSmm module source files also else SdioSmm module $(EOL)\
  #   source files will be processed through SdioSmm.inf$(EOL)\
$(EOL)\
[Sources]$(EOL)\
  ../$(SDIO_DRIVER_DIR)/SdioDriver.c$(EOL)\
  ../$(SDIO_DRIVER_DIR)/SdioBlkIo.c$(EOL)\
  ../$(SDIO_DRIVER_DIR)/SdioDriver.h$(EOL)"\
> $(BUILD_DIR)\SdioDriver.inf

ifeq ($(SDIO_SMM_SUPPORT),0)
	@$(ECHO)\
 "../$(SDIO_SMM_DIR)/SdioSmm.c$(EOL)\
  ../$(SDIO_SMM_DIR)/SdioSmm.h$(EOL)\
  ../$(SDIO_SMM_DIR)/SdioSmmController.c$(EOL)\
  ../$(SDIO_SMM_DIR)/SdioSmmController.h$(EOL)"\
>> $(BUILD_DIR)\SdioDriver.inf
endif

	@$(ECHO)\
"[Packages]$(EOL)\
  AmiCompatibilityPkg/AmiCompatibilityPkg.dec$(EOL)\
  AmiModulePkg/AmiModulePkg.dec$(EOL)\
  MdePkg/MdePkg.dec$(EOL)\
  $(EOL)\
[LibraryClasses]$(EOL)\
  AmiDxeLib$(EOL)\
  UefiDriverEntryPoint$(EOL)\
  $(EOL)\
[Protocols]$(EOL)\
  gEfiDriverBindingProtocolGuid     # PROTOCOL ALWAYS_CONSUMED $(EOL)\
  gEfiPciIoProtocolGuid             # PROTOCOL ALWAYS_CONSUMED $(EOL)\
  gEfiDevicePathProtocolGuid        # PROTOCOL ALWAYS_CONSUMED $(EOL)\
  gEfiBlockIoProtocolGuid           # PROTOCOL ALWAYS_CONSUMED $(EOL)\
  gEfiDiskInfoProtocolGuid          # PROTOCOL ALWAYS_CONSUMED $(EOL)\
  gEfiComponentNameProtocolGuid     # PROTOCOL ALWAYS_CONSUMED $(EOL)\
  gEfiLegacyBiosExtProtocolGuid     # PROTOCOL ALWAYS_CONSUMED $(EOL)\
  gEfiSdioProtocolGuid              # PROTOCOL ALWAYS_CONSUMED $(EOL)\
  gEfiSmmControl2ProtocolGuid       # PROTOCOL ALWAYS_CONSUMED $(EOL)\
  gSdioBusInitProtocolGuid          # PROTOCOL ALWAYS_PRODUCED $(EOL)\
  gEfiComponentName2ProtocolGuid        # PROTOCOL ALWAYS_PRODUCED $(EOL)\
  $(EOL)\
[Depex]$(EOL)\
  TRUE$(EOL)\
$(EOL)\
#**********************************************************************$(EOL)\
#**********************************************************************$(EOL)\
#**                                                                  **$(EOL)\
#**        (C)Copyright 1985-2012, American Megatrends, Inc.         **$(EOL)\
#**                                                                  **$(EOL)\
#**                       All Rights Reserved.                       **$(EOL)\
#**                                                                  **$(EOL)\
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **$(EOL)\
#**                                                                  **$(EOL)\
#**                       Phone: (770)-246-8600                      **$(EOL)\
#**                                                                  **$(EOL)\
#**********************************************************************$(EOL)\
#**********************************************************************$(EOL)\
$(EOL)"\
>> $(BUILD_DIR)\SdioDriver.inf

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