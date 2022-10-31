#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2011, American Megatrends, Inc.         **
#**                                                                  **
#**                       All Rights Reserved.                       **
#**                                                                  **
#**              5555 Oak brook Pkwy, Norcorss, GA 30093             **
#**                                                                  **
#**                       Phone: (770)-246-8600                      **
#**                                                                  **
#**********************************************************************
#**********************************************************************

#**********************************************************************
# $Header: /Alaska/BIN/Modules/AMITSE2_0/AMITSE/AMITSE.mak 20    7/21/11 2:14a Arunsb $
#
# $Revision: 20 $
#
# $Date: 7/21/11 2:14a $
#**********************************************************************
#<AMI_FHDR_START>
#
# Name:	AMITSE.mak
#
# Description:	AMI TSE BIN module makefile for Aptio.
#
#<AMI_FHDR_END>
#**********************************************************************

Prepare : $(BUILD_DIR)/AMITSEElinks.h $(BUILD_DIR)/HotKeyElinks.h $(BUILD_DIR)/HotClickElinks.h $(BUILD_DIR)/AMITSEOem.h $(BUILD_DIR)/AMITSEStrTokens.h

ifeq ($(TSE_GNUC_BUILD_SUPPORT), 1)
  AMITSE_TARGET_ARCH = AARCH64
else
  ifeq ($(X64_SUPPORT), 1)
    AMITSE_TARGET_ARCH = X64
  else
    AMITSE_TARGET_ARCH = IA32
  endif
endif

TSE_GC_MODE1 = $(subst {,,$(MAX_POST_GC_MODE))
TSE_GC_MODE2 = $(subst },,$(TSE_GC_MODE1))

$(BUILD_DIR)/AMITSEElinks.h :  $(BUILD_DIR)/Token.mak $(TSEBIN_DIR)/AMITSE.mak
	$(ECHO) \
"#define HOOK_LIST_FROM_ELINKS $(AMITSE_Hooks)$(EOL)\
#define CONTROL_KEY_MAP_LIST $(CONTROL_KEY_MAP_LIST)$(EOL)\
#define EXIT_PAGE_OPTIONS_LIST $(EXIT_PAGE_OPTIONS_LIST)$(EOL)\
#define OEM_KEY_CALLBACK_FN $(OEM_KEY_CALLBACK_LIST)$(EOL)\
#define AMITSE_HIDDEN_PAGE_LIST $(AMITSE_HIDDEN_PAGE_LIST)$(EOL)\
#define TSE_CALLBACK_SPEC_VERSION $(TSE_CALLBACK_SPEC_VERSION)$(EOL)\
#define BIOS_SIGNON_MESSAGE2_STRING	$(BIOS_SIGNON_MESSAGE2)$(EOL)\
#define TSE_MAX_POST_HRes(x, y)	(x)$(EOL)\
#define TSE_MAX_POST_VRes(x, y) (y)$(EOL)\
#define TSE_MAX_POST_X_RES TSE_MAX_POST_HRes($(MAX_POST_GC_MODE))$(EOL)\
#define TSE_MAX_POST_Y_RES TSE_MAX_POST_VRes($(MAX_POST_GC_MODE))$(EOL)\
#define AMITSE_SUBPAGE_AS_ROOT_PAGE_LIST $(AMITSE_SUBPAGE_AS_ROOT_PAGE_LIST)$(EOL)\
#define AMITSE_ROOT_PAGE_ORDER $(AMITSE_ROOT_PAGE_ORDER)$(EOL)\
#define PASSWORD_ENOCDE_LIST $(PASSWORD_ENOCDE_LIST)$(EOL)"\
>$(BUILD_DIR)/AMITSEElinks.h


$(BUILD_DIR)/AMITSEStrTokens.h : 
	$(ECHO) \
"#include <$(BUILD_DIR)$(PATH_SLASH)$(PLATFORM_NAME)$(PATH_SLASH)$(TARGET)_$(TOOL_CHAIN_TAG)$(PATH_SLASH)$(AMITSE_TARGET_ARCH)/Build/AMITSE/DEBUG/AMITSEStrDefs.h>$(EOL)" >$(BUILD_DIR)/AMITSEStrTokens.h
  
$(BUILD_DIR)/HotKeyElinks.h : $(BUILD_DIR)/Token.mak $(TSEBIN_DIR)/AMITSE.mak
	$(ECHO) \
"#define HOTKEY_LIST $(HOTKEY_LIST)$(EOL)"\
>$(BUILD_DIR)/HotKeyElinks.h

$(BUILD_DIR)/HotClickElinks.h : $(BUILD_DIR)/Token.mak $(TSEBIN_DIR)/AMITSE.mak
	$(ECHO) \
"#define HOTCLICK_LIST $(HOTCLICK_LIST)$(EOL)"\
>$(BUILD_DIR)/HotClickElinks.h

$(BUILD_DIR)/AMITSEOem.h : $(BUILD_DIR)/Token.mak $(TSEBIN_DIR)/AMITSE.mak
	$(ECHO) \
"$(if $(AMITSE_OEM_HEADER_LIST), $(foreach S_DEF, $(AMITSE_OEM_HEADER_LIST),#include<$(S_DEF)>$(EOL)))"\
>$(BUILD_DIR)/AMITSEOem.h

Prepare: BeforeGenFds $(BUILD_DIR)/AMITSE.inf
BeforeGenFds : HpkTool 
.PHONY : BeforeGenFds HpkTool

ABS_OUTPUT_DIR:=$(WORKSPACE)$(PATH_SLASH)$(OUTPUT_DIRECTORY)

ifeq ($(INI_FILE_PATH),)
  ifeq ($(wildcard $(TSEBIN_DIR)/uefisetup.ini), $(TSEBIN_DIR)/uefisetup.ini)
    HPKTOOL_INI_FILE_PATH:=$(WORKSPACE)$(PATH_SLASH)$(TSEBIN_DIR)$(PATH_SLASH)Uefisetup.ini
  else
    HPKTOOL_INI_FILE_PATH:=$(TOOLS_DIR)$(PATH_SLASH)DefaultHpkToolSetup.ini
  endif
else
HPKTOOL_INI_FILE_PATH:=$(WORKSPACE)$(PATH_SLASH)$(INI_FILE_PATH)
endif

HPKTOOL_COMMAND_LINE:=-i$(subst $(SPACE),$(SPACE)-i,$(IFR_DIR_LIST))\
-f$(HPKTOOL_INI_FILE_PATH)
HPKTOOL_COMMAND_LINE2:=$(HPKTOOL_COMMAND_LINE) -do$(ABS_OUTPUT_DIR)/Nvram.bin -o$(ABS_BUILD_DIR)/setupdata_asm.asm
HPKTOOL_COMMAND_LINE+=-do$(ABS_OUTPUT_DIR)/Defaults.bin -o$(ABS_BUILD_DIR)/tmp.asm
ifeq ($(MANUFACTURING_MODE_SUPPORT),1)
HPKTOOL_COMMAND_LINE+=-dm
HPKTOOL_COMMAND_LINE2+=-dm
endif
HPKTOOL_COMMAND_LINE+=-r -dn$(NVRAM_RECORD_CHECKSUM_SUPPORT) -de$(FLASH_ERASE_POLARITY) -vs$(EFI_SPECIFICATION_VERSION) -dl$(DEFAULT_LANGUAGE)\
-dg4599D26F-1A11-49b8-B91F-858745CFF824\
-h$(AMITSE_HPK_FILE_ORDER)
HPKTOOL_COMMAND_LINE2+=-dl$(DEFAULT_LANGUAGE)\
-ds$(RAW_NVRAM_SIZE) -dn$(NVRAM_RECORD_CHECKSUM_SUPPORT) -de$(FLASH_ERASE_POLARITY) -vs$(EFI_SPECIFICATION_VERSION) -h$(AMITSE_HPK_FILE_ORDER)
        
HpkTool :
ifeq ($(TSE_GNUC_BUILD_SUPPORT), 0)
	$(ECHO_NO_ESC) $(WORKSPACE)\$(TSEBIN_DIR)\UefiHpkTool.exe $(HPKTOOL_COMMAND_LINE) >> $(BUILD_DIR)/BeforeGenFds.bat
	$(ECHO_NO_ESC) $(WORKSPACE)\$(TSEBIN_DIR)\UefiHpkTool.exe $(HPKTOOL_COMMAND_LINE2) >> $(BUILD_DIR)/BeforeGenFds.bat
	$(ECHO_NO_ESC) ml /c /nologo /Fo$(ABS_BUILD_DIR)\ $(ABS_BUILD_DIR)\setupdata_asm.asm >> $(BUILD_DIR)/BeforeGenFds.bat
	$(ECHO_NO_ESC) $(CCX86DIR)\link /NOENTRY /FIXED /DLL $(ABS_BUILD_DIR)\setupdata_asm.obj /OUT:$(ABS_BUILD_DIR)\setupdata.dll >> $(BUILD_DIR)/BeforeGenFds.bat
	$(ECHO_NO_ESC) genfw --exe2bin $(ABS_BUILD_DIR)\setupdata.dll -o $(ABS_BUILD_DIR)\setupdata.bin >> $(BUILD_DIR)/BeforeGenFds.bat
else
	$(ECHO) chmod +x $(WORKSPACE)/$(TSEBIN_DIR)/HPKTool_Linux > $(BUILD_DIR)/BeforeGenFds.sh
	$(ECHO) $(WORKSPACE)/$(TSEBIN_DIR)/HPKTool_Linux  $(HPKTOOL_COMMAND_LINE) >> $(BUILD_DIR)/BeforeGenFds.sh
	$(ECHO) $(WORKSPACE)/$(TSEBIN_DIR)/HPKTool_Linux  $(HPKTOOL_COMMAND_LINE2) >> $(BUILD_DIR)/BeforeGenFds.sh
	$(ECHO) gcc -c -x assembler $(ABS_BUILD_DIR)/setupdata_asm.asm   -o$(ABS_BUILD_DIR)/setupdata_asm.o >> $(BUILD_DIR)/BeforeGenFds.sh
	$(ECHO) objcopy -O binary $(ABS_BUILD_DIR)/setupdata_asm.o $(ABS_BUILD_DIR)/setupdata.bin >> $(ABS_BUILD_DIR)/BeforeGenFds.sh
endif


ifneq ($(TSE_SOURCE_FILES),"")
TSE_STR_FILES += $(patsubst %,../%,$(subst $(SPACE),$(EOL)$(SPACE),$(TSE_SOURCE_FILES)))$(EOL)
endif
ifneq ($(TSE_STRING_FILES),"")
TSE_STR_FILES += $(patsubst %,../%,$(subst $(SPACE),$(EOL)$(SPACE),$(TSE_STRING_FILES)))$(EOL)
endif
ifneq ($(TSE_STRING_CONSUMERS_LIST),"")
TSE_STR_FILES += $(patsubst %,../%,$(subst $(SPACE),$(EOL)$(SPACE),$(TSE_STRING_CONSUMERS_LIST)))$(EOL)
endif


$(BUILD_DIR)/AMITSE.inf : $(BUILD_DIR)/Token.mak $(TSEBIN_DIR)/AMITSE.mak
	@$(ECHO) \
"## @file$(EOL)\
#   The [Sources] section for this file is auto-generated from ELINKs:$(EOL)\
#   SETUP_DEFINITIONS, SETUP_FORMSETS, SetupStringFiles, and SetupCallbackFiles$(EOL)\
##$(EOL)\
$(EOL)\
[Defines]$(EOL)\
  INF_VERSION                    = 0x00010005$(EOL)\
  BASE_NAME                      = AMITSE$(EOL)\
  FILE_GUID                      = B1DA0ADF-4F77-4070-A88E-BFFE1C60529A$(EOL)\
  MODULE_TYPE                    = DXE_DRIVER$(EOL)\
  VERSION_STRING                 = 1.0$(EOL)\
  ENTRY_POINT                    = MiniSetupApplication$(EOL)\
#$(EOL)\
#  This flag specifies whether HII resource section is generated into PE image.$(EOL)\
#$(EOL)\
  UEFI_HII_RESOURCE_SECTION      = TRUE$(EOL)\
  $(EOL)\
#$(EOL)\
# The following information is for reference only and not required by the build tools.$(EOL)\
#$(EOL)\
#  VALID_ARCHITECTURES           = IA32 X64 IPF EBC$(EOL)\
#$(EOL)\
$(EOL)\
[Sources]$(EOL)\
$(TSE_STR_FILES)$(EOL)\
../$(TSEBIN_DIR)/bootflow.c$(EOL)\
../$(TSEBIN_DIR)/bootflow.h$(EOL)\
../$(TSEBIN_DIR)/commonoem.c$(EOL)\
../$(TSEBIN_DIR)/commonoem.h$(EOL)\
../$(TSEBIN_DIR)/FakeTokens.c$(EOL)\
../$(TSEBIN_DIR)/Inc/boot.h$(EOL)\
../$(TSEBIN_DIR)/Inc/HiiLib.h$(EOL)\
../$(TSEBIN_DIR)/Inc/LogoLib.h$(EOL)\
../$(TSEBIN_DIR)/Inc/mem.h$(EOL)\
../$(TSEBIN_DIR)/Inc/PwdLib.h$(EOL)\
../$(TSEBIN_DIR)/Inc/variable.h$(EOL)\
../$(TSEBIN_DIR)/Inc/HookAnchor.h$(EOL)\
../$(TSEBIN_DIR)/HookList.c$(EOL)\
../$(TSEBIN_DIR)/CommonHelper.c$(EOL)\
../$(TSEBIN_DIR)/Keymon.c$(EOL)\
../$(TSEBIN_DIR)/Keymon.h$(EOL)\
../$(TSEBIN_DIR)/Inc/TseElinks.h$(EOL)\
../$(TSEBIN_DIR)/Inc/TseCommon.h$(EOL)\
../$(TSEBIN_DIR)/Inc/setupdata.h$(EOL)\
../$(TSEBIN_DIR)/TseDrvHealth.h$(EOL)\
../$(TSEBIN_DIR)/setupdbg.h$(EOL)\
$(EOL)\
[Packages]$(EOL)\
  AmiCompatibilityPkg/AmiCompatibilityPkg.dec$(EOL)\
  MdePkg/MdePkg.dec$(EOL)\
  AmiModulePkg/AmiModulePkg.dec$(EOL)\
  MdeModulePkg/MdeModulePkg.dec$(EOL)\
  IntelFrameworkModulePkg/IntelFrameworkModulePkg.dec$(EOL)\
  IntelFrameworkPkg/IntelFrameworkPkg.dec$(EOL)\
  AmiTsePkg/AmiTsePkg.dec$(EOL)\
  $(subst $(SPACE),$(EOL)$(SPACE),$(ADDITIONAL_AMITSE_Packages))$(EOL)\
  $(EOL)\
[LibraryClasses]$(EOL)\
  $(subst $(SPACE),$(EOL)$(SPACE),$(ADDITIONAL_AMITSE_LibraryClasses))$(EOL)\
  AmiDxeLib$(EOL)\
  MemoryAllocationLib$(EOL)\
  BaseLib$(EOL)\
  BaseMemoryLib$(EOL)\
  PerformanceLib$(EOL)\
  DevicePathLib$(EOL)\
  DebugLib$(EOL)\
  UefiLib$(EOL)\
  PrintLib$(EOL)\
  UefiDriverEntryPoint$(EOL)\
  ReportStatusCodeLib$(EOL)\
$(EOL)\
[Protocols]$(EOL)\
  gEfiConsoleControlProtocolGuid$(EOL)\
  gEfiFirmwareVolume2ProtocolGuid$(EOL)\
  gEfiUnicodeCollation2ProtocolGuid$(EOL)\
  gEfiUnicodeCollationProtocolGuid$(EOL)\
  gEfiOEMBadgingProtocolGuid$(EOL)\
  gEfiSimpleFileSystemProtocolGuid$(EOL)\
  gEfiUnicodeCollation2ProtocolGuid$(EOL)\
  gEfiUnicodeCollationProtocolGuid$(EOL)\
  gEfiBlockIoProtocolGuid$(EOL)\
  gEfiLegacyBiosProtocolGuid$(EOL)\
  gEfiFirmwareVolumeProtocolGuid$(EOL)\
  gEfiSimpleTextInProtocolGuid$(EOL)\
  gEfiLoadedImageProtocolGuid$(EOL)\
  gEfiFirmwareVolume2ProtocolGuid$(EOL)\
  gEfiConsoleControlProtocolGuid$(EOL)\
  gAmiPostManagerProtocolGuid$(EOL)\
  $(subst $(SPACE),$(EOL)$(SPACE),$(ADDITIONAL_AMITSE_Protocols))$(EOL)\
$(EOL)\
[Guids]$(EOL)\
  $(subst $(SPACE),$(EOL)$(SPACE),$(ADDITIONAL_AMITSE_Guids))$(EOL)\
[Depex]$(EOL)\
  TRUE$(EOL)\
$(EOL)\
[BuildOptions]$(EOL)\
  MSFT:*_*_*_CC_FLAGS = /DTSE_FOR_APTIO_4_50$(EOL)\
  GCC:*_*_*_CC_FLAGS = -DTSE_FOR_APTIO_4_50$(EOL)\
  *_*_*_BUILD_FLAGS=-s$(EOL)"\
> $(BUILD_DIR)/AMITSE.inf


#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2011, American Megatrends, Inc.         **
#**                                                                  **
#**                       All Rights Reserved.                       **
#**                                                                  **
#**             5555 Oakbrook Pkwy, Norcross, Georgia 30093          **
#**                                                                  **
#**                       Phone: (770)-246-8600                      **
#**                                                                  **
#**********************************************************************
#**********************************************************************
