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
# Revision History
# ----------------
# $Log: $
# 
# 
#*************************************************************************
#<AMI_FHDR_START>
#---------------------------------------------------------------------------
# Name:     Cpu.mak
#
# Description:  
#   Make file for the PlatformSecLib. "Prepare" target will make INF files 
#   generated to support SEC ELINK. 
#
#---------------------------------------------------------------------------
#<AMI_FHDR_END>

Prepare :  $(BUILD_DIR)\SecCoreHdr.txt AmiValleyViewCpuPkg\Library\PlatformSecLib\Ia32\SECCoreL.ASM

$(BUILD_DIR)\equates.equ:
	$(ECHO) ";This file is generated by Cpu.mak and expected to be used by SECCoreL.ASM "\
> $(BUILD_DIR)\equates.equ

$(BUILD_DIR)\SecCoreHdr.txt: $(BUILD_DIR)\equates.equ
	$(ECHO) ".686p$(EOL).xmm$(EOL).model small,c$(EOL)"\
> $(BUILD_DIR)\SecCoreHdr.txt

AmiValleyViewCpuPkg\Library\PlatformSecLib\Ia32\SECCoreL.ASM : $(BUILD_DIR)\SECCore.ASM $(BUILD_DIR)\SecCoreHdr.txt
	del /F /Q AmiValleyViewCpuPkg\Library\PlatformSecLib\Ia32\SECCoreL.ASM 
	$(CP) $(CP_OPTS) $(BUILD_DIR)$(PATH_SLASH)SecCoreHdr.txt + $(BUILD_DIR)$(PATH_SLASH)SECCore.ASM $@
  

