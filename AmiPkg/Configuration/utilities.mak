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
# $Header: $
#
# $Revision:  $
#
# $Date:  $
#**********************************************************************
#<AMI_FHDR_START>
#
# Name:	utilities.mak
#
# Description:	Currently used by makefile and Main.mak, contains utility
#  definitions, autodetects OS, builds and exports path environment variables,
#  and some common character definitions (such as COMMA, SPACE, and EOL)
#
#<AMI_FHDR_END>
#**********************************************************************

# Makefile utilities

# Detect OS, based on Windows WINDIR env variable
# 0 - Windows OS
# 1 - Linux OS
BUILD_OS_WINDOWS := 0
BUILD_OS_LINUX := 1

ifndef BUILD_OS
ifeq ($(OS),Windows_NT)
 # Windows OS
 BUILD_OS := $(BUILD_OS_WINDOWS)
else
 # Linux OS
 BUILD_OS := $(BUILD_OS_LINUX)
endif	#ifdef (windir)
export BUILD_OS
endif	#ifndef BUILD_OS

# Check paths 
# Make sure TOOLS_DIR is defined, otherwise error out!
ifdef TOOLS_DIR
 TOOLS_DIR := $(subst \,/,$(TOOLS_DIR))
else
 $(error TOOLS_DIR is not defined. Set TOOLS_DIR to an absolute path of the build tools directory.)
endif #ifdef (TOOLS_DIR)

# If VEB_BUILD_DIR isn't defined by VeB, just assume it's Build
ifndef BUILD_DIR
# The code below is commented out because we currently don't support Build folder override.
# Use fixed BUILD_DIR name.
 BUILD_DIR = Build
#ifndef VEB_BUILD_DIR
# $(warning VEB_BUILD_DIR is not defined. Setting to 'Build'.)
# BUILD_DIR = Build
#else
# BUILD_DIR = $(VEB_BUILD_DIR)
#endif #ifndef VEB_BUILD_DIR
export BUILD_DIR
endif #ifndef BUILD_DIR
export BUILD_ROOT=$(BUILD_DIR)

# Set all of the paths correctly, based on OS
ifeq ($(BUILD_OS), $(BUILD_OS_WINDOWS))
  UNIX_UTILS_DIR := $(TOOLS_DIR)/
  EFI_TOOLS_DIR := $(TOOLS_DIR)/Bin/Win32
  # Windows GNU Make needs both of these to be exported for children to properly
  #  inherit the path
  export PATH := $(TOOLS_DIR);$(EDK_TOOLS_PATH)/Bin/Win32;$(PATH)
  export Path := $(PATH)
else
  UNIX_UTILS_DIR = 
  ifeq ("$(shell getconf LONG_BIT)","64")
        EFI_TOOLS_DIR = $(TOOLS_DIR)/Bin/Linux64
  	export PATH := $(TOOLS_DIR):$(EDK_TOOLS_PATH)/Bin/Linux64:$(PATH)
  else
  	EFI_TOOLS_DIR = $(TOOLS_DIR)/Bin/Linux32
  	export PATH := $(TOOLS_DIR):$(EDK_TOOLS_PATH)/Bin/Linux32:$(PATH)
  endif
  SHELL:=/bin/bash
endif	#ifeq($(BUILD_OS),$(BUILD_OS_WINDOWS))

# If VEB isn't defined by VeB (or shell), try to guess which one
ifndef VEB
  __TMP_VEB_SUFFIX__:= $(suffix $(wildcard *.veb))
  ifneq ($(__TMP_VEB_SUFFIX__), .veb)
    ifndef __TMP_VEB_SUFFIX__
      $(error No VeB file found! Set VEB to a base name(no extention) of the project .veb file)
    endif
    $(error Multiple project VeB files found! Set VEB to a base name(no extention) of the project .veb file)
  endif
  VEB := $(basename $(wildcard *.veb) )
  $(warning VEB is not defined, using $(VEB).veb)
  export VEB
endif #ifndef VEB

# Check for CCX86DIR and CCX64DIR
ifeq ($(TOOL_CHAIN_TAG), MYTOOLS)
 ifndef CCX86DIR
  $(warning 32-bit compiler CCX86DIR not defined!)
 endif #ifndef CCX86DIR
 ifndef CCX64DIR
  $(warning 64-bit compiler CCX86DIR not defined!)
 endif #ifndef CCX64DIR
endif #ifeq( $(TOOL_CHAIN_TAG), MYTOOLS)

# EDK2 variables
WORKSPACE =$(CURDIR)
EDK_TOOLS_PATH=$(TOOLS_DIR)
BASE_TOOLS_PATH=$(TOOLS_DIR)
export WORKSPACE EDK_TOOLS_PATH BASE_TOOLS_PATH

# Useful build utilities
JAVA = java
MAKE = make
EDII_BUILD = $(EFI_TOOLS_DIR)/build

# Define some general unix utilities
  # RM/MKDIR made OS specific for performance reasons
ifeq ($(BUILD_OS), $(BUILD_OS_WINDOWS))
  RM = del /F /Q
  RMDIR = cmd /C rd /S /Q
  CP = copy
# Can't use plain "PATH_SLASH=\" here because Make treats 
# back slash followed by the new line as a line continuation character.
  PATH_SLASH := $(subst /,\,/)
  PATH_SLASH_ESC = \\
  DOUBLEDOLLAR = $$
  FWBUILD = FwBuild
__gt = $(shell if $1 GTR $2 (echo yes) else (echo no) )
__ge = $(shell if $1 GEQ $2 (echo yes) else (echo no) )
else
  RM = rm -fr
  RMDIR = rm -fr
  CP = cp -f
  PATH_SLASH = /
  PATH_SLASH_ESC = /
  DOUBLEDOLLAR = \$$
__gt = $(shell if [ $(1) -gt $(2) ] ; then echo yes ; else echo no ; fi)
__ge = $(shell if [ $(1) -ge $(2) ] ; then echo yes ; else echo no ; fi)
endif #ifeq ($(BUILD_OS), $(BUILD_OS_WINDOWS))
MKDIR = mkdir
ECHO = @$(UNIX_UTILS_DIR)echo -e
ECHO_NO_ESC = @$(UNIX_UTILS_DIR)echo -E
DATE = $(UNIX_UTILS_DIR)date
CAT = $(UNIX_UTILS_DIR)cat
GAWK = $(UNIX_UTILS_DIR)gawk

# Useful definitions
EOL = \n
COMMA := ,
SPACE :=
SPACE +=
TAB := \t

ifneq ($(BUILD_OS), $(BUILD_OS_WINDOWS))
 # This needs to be defined on non Windows OSs, this might break under some circumstances
 NUMBER_OF_PROCESSORS = $(shell cat /proc/cpuinfo | grep processor | wc -l)
endif	#ifeq ($(BUILD_OS), 0)

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