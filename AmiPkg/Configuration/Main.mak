#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
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
# $Revision:  $
#
# $Date:  $
#**********************************************************************
#<AMI_FHDR_START>
#
# Name:	Main.mak
#
# Description:	Called by build.mak, this includes all of the makefiles
#  from module.mak, generates target.txt, then launches the EDK2 build.exe
#
#<AMI_FHDR_END>
#**********************************************************************
include $(UTILITIES_MAK)
include $(TOKEN_MAK)

ifeq ($(SILENT), 1) 
APTIO_MAKE+=-s
.SILENT :
endif

# Set target debug type based on SDL token DEBUG_MODE
ifeq ($(DEBUG_MODE), 1)
 TARGET	= DEBUG
else
 TARGET	= RELEASE
endif	#ifeq ($(DEBUG_MODE), 1)

#TODO: this should be tool chain based (not OS based)
#TODO: what about optimization flags for other tool chains?
ifeq ($(BUILD_OS), $(BUILD_OS_WINDOWS))
ifeq ($(OPTIMIZATION), 0)
EXTERNAL_CC_FLAGS +=  /Od 
else
EXTERNAL_CC_FLAGS +=  /O1ib2 
endif
endif
ifeq ($(DEBUG_CODE),  1)
EXTERNAL_CC_FLAGS +=  -DEFI_DEBUG 
else
EXTERNAL_CC_FLAGS +=  -DMDEPKG_NDEBUG 
endif
export EXTERNAL_CC_FLAGS

ifeq ($(RUN_VSVARS_BAT), 1)
EDKII_BUILD_COMMAND = $(TOOLS_DIR)\RunInVsEnv.bat $(EDII_BUILD)
else
EDKII_BUILD_COMMAND = $(EDII_BUILD)
endif	#ifeq (TOOL_CHAIN_TAG, MYTOOLS)

.PHONY : all run Prepare EdkII End ModuleEnd BuildTimeStamp

all: Prepare EdkII

Prepare: BuildTimeStamp

# Generate new timestamp files
BuildTimeStamp:
	$(APTIO_MAKE) -f $(CONFIGURATION_DIR)dater.mak TODAY=$(TODAY) NOW=$(NOW)

.PHONY : BeforeGenFds InitBeforeGenFds FontFile 
Prepare : BeforeGenFds
BeforeGenFds : InitBeforeGenFds FontFile 

ABS_BUILD_DIR:=$(WORKSPACE)$(PATH_SLASH)$(BUILD_DIR)
ABS_OUTPUT_DIR:=$(WORKSPACE)$(PATH_SLASH)$(OUTPUT_DIRECTORY)

InitBeforeGenFds :
	$(ECHO) \
".PHONY : all\
$(EOL)all:\
" > $(BUILD_DIR)/BeforeGenFds.mak
ifeq ($(call __gt, 21,$(BUILD_TOOLS_VERSION)),yes)
	$(ECHO) $(APTIO_MAKE) -f $(BUILD_DIR)/BeforeGenFds.mak > $(BUILD_DIR)/BeforeGenFds.bat
else
EDKII_FLAGS += --before-fv-command="$(APTIO_MAKE) -f $(BUILD_DIR)/BeforeGenFds.mak"
endif

export FONT_TOOL := FontTool -F 2.1 -C $(FONT_INI_FILE)
export FONT_TOOL_TMP_FILE:=$(ABS_OUTPUT_DIR)/$(TARGET)_$(TOOL_CHAIN_TAG)/font.tmp

FontFile : InitBeforeGenFds
	$(FONT_TOOL) -S -T $(FONT_TOOL_TMP_FILE)
	$(ECHO) \
"$(EOL).PHONY : FontFile$(EOL)all: FontFile\
$(EOL)FontFile : \
$(EOL)$(TAB)$(FONT_TOOL) -IL $(subst ;,$(SPACE),$(LANGUAGE_FONT_LIST)) -T $(FONT_TOOL_TMP_FILE)\
$(EOL)$(TAB)$(FONT_TOOL) -O $(ABS_OUTPUT_DIR)/$(TARGET)_$(TOOL_CHAIN_TAG)/font.out -T $(FONT_TOOL_TMP_FILE)\
" >> $(BUILD_DIR)/BeforeGenFds.mak

# Clear MAKEFLAGS during execution of the EdkII target. 
# We don't want to pass our options to EDKII build process.
run : MAKEFLAGS=
run : 
	$(EDKII_BUILD_COMMAND) run

# Create Conf/target.txt, based on SDL tokens
Conf/target.txt : $(TOKEN_MAK) $(MAIN_MAK) Conf
	@$(ECHO) \
"ACTIVE_PLATFORM	   = $(subst \,/, $(ACTIVE_PLATFORM))\
$(EOL)\
$(EOL)TARGET				= $(TARGET)\
$(EOL)\
$(EOL)TOOL_CHAIN_CONF	   = $(TOOL_DEFINITION_FILE)\
$(EOL)\
$(EOL)TOOL_CHAIN_TAG		= $(TOOL_CHAIN_TAG)\
$(EOL)\
$(EOL)MAX_CONCURRENT_THREAD_NUMBER = $(NUMBER_OF_BUILD_PROCESS_THREADS)\
$(EOL)\
$(EOL)BUILD_RULE_CONF = $(BUILD_RULE_FILE)" > Conf/target.txt

Conf: 
	mkdir Conf

# Make sure the variables required to launch EDKII build process are set
ifeq ("$(wildcard $(ACTIVE_PLATFORM))","")
  $(error Project description(.dsc) file "$(ACTIVE_PLATFORM)" defined by the ACTIVE_PLATFORM SDL token is not found.\
  This can happen when PLATFORM_DSC or <Arch-Type>_PLATFORM_DSC SDL output directive does not exist\
  or refers to a different file)
endif
ifeq ("$(PLATFORM_GUID)","INVALID")
  $(error PLATFORM_GUID SDL token must be cloned in every project)
endif
ifeq ("$(SUPPORTED_ARCHITECTURES)","INVALID")
  $(error The value of the SUPPORTED_ARCHITECTURES SDL token is invalid.\
  The token must be cloned by the CPU architecture module)
endif

ifeq ($(SILENT), 1) 
EDKII_FLAGS += -s
endif

####################################################
# Single Module Build Support.
# There are two ways to define a module to build:
# 1. Using VeB GUI. VeB will set VEB_BUILD_MODULE environment variable.
# 2. Using EDKII_FLGAS (-m <module-name>)
####################################################
# Extract module name from EDKII_FLAGS (Don't try this at home :)
EDKII_ACTIVE_MODULE_NAME:=$(strip $(subst ~~~~~,,$(filter ~~~~~%,$(subst -m$(SPACE),~~~~~,$(EDKII_FLAGS)))))
ifneq ("$(EDKII_ACTIVE_MODULE_NAME)","")
ifneq ("$(VEB_BUILD_MODULE)","")
$(error Ambiguous single component build parameters. VEB_BUILD_MODULE=$(VEB_BUILD_MODULE). EDKII_FLAGS module = $(EDKII_ACTIVE_MODULE_NAME).)
endif
endif
ifneq ("$(VEB_BUILD_MODULE)","")
EDKII_FLAGS += -m $(VEB_BUILD_MODULE)
EDKII_ACTIVE_MODULE_NAME:=$(VEB_BUILD_MODULE)
endif

ifeq ("$(EDKII_ACTIVE_MODULE_NAME)","")
all: End
else
all: ModuleEnd
endif

# Clear MAKEFLAGS during execution of the EdkII target. 
# We don't want to pass our options to EDKII build process.
EdkII: MAKEFLAGS=
# If using MYTOOLS, create a batch file that runs VSVARS32.BAT before build.exe
#  so that the build can inherit the newly set up env variables
EdkII: Conf/target.txt
	$(EDKII_BUILD_COMMAND) $(EDKII_FLAGS)

# Include all the module makefiles
include $(MODULE_MAK)
ifneq ($(LAST_MAKEFILE),)
 include $(LAST_MAKEFILE)
endif

ModuleEnd:
	$(ECHO) TARGET: $(TARGET). TOOL CHAIN: $(TOOL_CHAIN_TAG). ARCHITECTURE: "$(SUPPORTED_ARCHITECTURES)".
	$(ECHO) Build Tools: $(BUILD_TOOLS_VERSION). Single component build mode. ROM image generation is skipped.
	$(ECHO) Active module: $(EDKII_ACTIVE_MODULE_NAME)

End:
ifdef FWBUILD
  ifeq ($(SILENT), 0) 
	$(FWBUILD) $(ROM_FILE_NAME) /v
  else 
	$(FWBUILD) $(ROM_FILE_NAME) /s
 endif
endif
	$(ECHO) TARGET: $(TARGET). TOOL CHAIN: $(TOOL_CHAIN_TAG). ARCHITECTURE: "$(SUPPORTED_ARCHITECTURES)".
	$(ECHO) Build Tools: $(BUILD_TOOLS_VERSION). ROM IMAGE: $(AMI_ROM).
ifeq ($(PLATFORM_FLAVOR_SELECT), 1)
	$(ECHO) Platform: Notebook	
else
	$(ECHO) Platform: Desktop
endif

ifeq ($(SecureMod_SUPPORT), 1)
ifneq ($(CREATE_FWCAPSULE), 0)
ifneq ($(wildcard $(FWpriv)),$(FWpriv))
	@echo ---------------------------------------------------------------------------------------
	@echo ----- WARNING!!! Missing RSA private key FWpriv="$(FWpriv)" to verify signed BIOS updates.
	@echo ----- WARNING!!! AMI flash utilities will not work when Secure Flash is enabled and the BIOS is unsigned.
	@echo ---------------------------------------------------------------------------------------
endif
endif
endif
	$(ECHO) All output modules were successfully built.

#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
#**                                                                  **
#**                       All Rights Reserved.                       **
#**                                                                  **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
#**                                                                  **
#**                       Phone: (770)-246-8600                      **
#**                                                                  **
#**********************************************************************
#**********************************************************************