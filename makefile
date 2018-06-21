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
# Name:	build.mak
#
# Description:	First makefile to be called during build, calls ParseVeb,
#  AMISDL, and dater.mak to generate files needed by Main.mak, then launches 
#  Main.mak
#
#<AMI_FHDR_END>
#**********************************************************************
export CONFIGURATION_DIR:=AmiPkg/Configuration/
export UTILITIES_MAK:=$(CONFIGURATION_DIR)utilities.mak
include $(UTILITIES_MAK)
ifeq ($(wildcard $(TOOLS_DIR)/BuildToolsVersion.mak),$(TOOLS_DIR)/BuildToolsVersion.mak)
include $(TOOLS_DIR)/BuildToolsVersion.mak
else
export BUILD_TOOLS_VERSION=0
endif

export MAIN_MAK:=$(CONFIGURATION_DIR)Main.mak
export TOKEN_MAK:=Build/Token.mak
export MODULE_MAK:=Build/module.mak
export APTIO_MAKE:=$(MAKE) --no-print-directory
AMISDL_MAK:=$(CONFIGURATION_DIR)RunAmiSdl.mak

.PHONY : all clean rebuild run RunAmiSdl AptioV sdl

all: $(BUILD_DIR) RunAmiSdl AptioV

# Clean out the old files
clean:
	@$(ECHO) Deleting temporary files...
  ifeq ($(wildcard $(BUILD_DIR)), $(BUILD_DIR))
	-$(RMDIR) $(BUILD_DIR)
  endif	
  ifeq ($(wildcard Conf), Conf) 	
	-$(RMDIR) Conf
  endif	
	@$(APTIO_MAKE) -s -f $(AMISDL_MAK) clean
	@$(ECHO) Done.
	
rebuild: clean all

run:
	$(APTIO_MAKE) -f $(MAIN_MAK) run

sdl: $(BUILD_DIR) RunAmiSdl

# Check if Build exists, if not, then make it
$(BUILD_DIR): 
	mkdir $(BUILD_DIR)

RunAmiSdl:
	@$(APTIO_MAKE) -f $(AMISDL_MAK) all

# If TODAY/NOW are not defined on the command line, generate them automatically.
# These auto-generated values may be overridden with TODAY/NOW SDL tokens.
# TODAY/NOW defined on the command line have the highest priority (SDL tokens do not override them).
# TODAY format: mm/dd/yyyy 
# NOW format: hh:mm:ss
ifeq ($(TODAY),) 
export TODAY := $(shell $(DATE) +'%m/%d/%Y')
endif
ifeq ($(NOW),) 
export NOW := $(shell $(DATE) +%T)
endif

AptioV:
	$(APTIO_MAKE) -f $(MAIN_MAK) $(target)


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