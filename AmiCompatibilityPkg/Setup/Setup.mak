Prepare : SetupFiles

SetupFiles : $(BUILD_DIR)/Setup.inf $(BUILD_DIR)/AutoId.h $(BUILD_DIR)/SetupCallbackList.h

$(BUILD_DIR)/SetupCallbackList.h : $(BUILD_DIR)/Token.mak
	@$(ECHO) \
"#define SETUP_ITEM_CALLBACK_LIST $(SetupItemCallbacks)$(EOL)\
#define SETUP_STRING_INIT_LIST $(SetupStringInit)$(EOL)" > $(BUILD_DIR)/SetupCallbackList.h

$(BUILD_DIR)/AutoId.h : $(Setup_DIR)/Setup.vfr $(SETUP_DEFINITIONS) $(BUILD_DIR)/Token.mak
	$(VFRID) /s$(THE_LOWEST_AUTOID_NUMBER) /o$(BUILD_DIR)/AutoId.h $(Setup_DIR)/Setup.vfr $(SETUP_DEFINITIONS)
	@$(ECHO) \
"#ifndef AUTO_ID$(EOL)\
#define AUTO_ID(x) x$(EOL)\
#endif$(EOL)" >> $(BUILD_DIR)/AutoId.h 

SETUP_DEFINITIONS := $(subst \,/,$(SETUP_DEFINITIONS))
SetupStringFiles := $(subst \,/,$(SetupStringFiles))
SetupCallbackFiles := $(subst \,/,$(SetupCallbackFiles))

ifneq ($(SETUP_DEFINITIONS),"")
SETUP_FILES += $(patsubst %,../%,$(subst $(SPACE),$(EOL)$(SPACE),$(SETUP_DEFINITIONS)))$(EOL)
endif
ifneq ($(SetupStringFiles),"")
SETUP_FILES += $(patsubst %,../%,$(subst $(SPACE),$(EOL)$(SPACE),$(SetupStringFiles)))$(EOL)
endif
ifneq ($(SetupCallbackFiles),"")
SETUP_FILES += $(patsubst %,../%,$(subst $(SPACE),$(EOL)$(SPACE),$(SetupCallbackFiles)))$(EOL)
endif


$(BUILD_DIR)/Setup.inf : $(BUILD_DIR)/Token.mak $(Setup_DIR)/Setup.mak
	@$(ECHO)\
"## @file$(EOL)\
#   The [Sources] section for this file is auto-generated from ELINKs:$(EOL)\
#   SETUP_DEFINITIONS, SETUP_FORMSETS, SetupStringFiles, and SetupCallbackFiles$(EOL)\
##$(EOL)\
$(EOL)\
[Defines]$(EOL)\
  INF_VERSION                    = 0x00010005$(EOL)\
  BASE_NAME                      = Setup$(EOL)\
  FILE_GUID                      = 899407D7-99FE-43d8-9A21-79EC328CAC21$(EOL)\
  MODULE_TYPE                    = DXE_DRIVER$(EOL)\
  VERSION_STRING                 = 1.0$(EOL)\
  ENTRY_POINT                    = SetupEntry$(EOL)\
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
  $(SETUP_FILES)$(EOL)\
  ../$(Setup_DIR)/Setup.c$(EOL)\
  ../$(Setup_DIR)/Setup.uni$(EOL)\
  ../$(Setup_DIR)/Setup.vfr$(EOL)\
  $(EOL)\
[Packages]$(EOL)\
  AmiCompatibilityPkg/AmiCompatibilityPkg.dec$(EOL)\
  AmiModulePkg/AmiModulePkg.dec$(EOL)\
  MdePkg/MdePkg.dec$(EOL)\
  MdeModulePkg/MdeModulePkg.dec$(EOL)\
  IntelFrameworkPkg/IntelFrameworkPkg.dec$(EOL)\
  $(subst $(SPACE),$(EOL)$(SPACE),$(AdditionalSetupPackages))$(EOL)\
  $(EOL)\
[LibraryClasses]$(EOL)\
  AmiDxeLib$(EOL)\
  UefiDriverEntryPoint$(EOL)\
  $(subst $(SPACE),$(EOL)$(SPACE),$(AdditionalSetupLibraryClasses))$(EOL)\
$(EOL)\
[Protocols]$(EOL)\
  gEfiHiiPackageListProtocolGuid    ## CONSUMES$(EOL)\
  gEfiHiiStringProtocolGuid    ## CONSUMES$(EOL)\
  gEfiHiiDatabaseProtocolGuid    ## CONSUMES$(EOL)\
  $(subst $(SPACE),$(EOL)$(SPACE),$(AdditionalSetupProtocols))$(EOL)\
$(EOL)\
[Guids]$(EOL)\
  $(subst $(SPACE),$(EOL)$(SPACE),$(AdditionalSetupGuids))$(EOL)\
$(EOL)\
[Depex]$(EOL)\
  gAmiConInStartedProtocolGuid$(EOL)\
$(EOL)\
[BuildOptions]$(EOL)\
  *_*_*_BUILD_FLAGS=-s$(EOL)"\
> $(BUILD_DIR)/Setup.inf
