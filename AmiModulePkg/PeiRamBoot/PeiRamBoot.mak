#*************************************************************************
#*************************************************************************
#**                                                                     **
#**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
#**                                                                     **
#**                       All Rights Reserved.                          **
#**                                                                     **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
#**                                                                     **
#**                       Phone: (770)-246-8600                         **
#**                                                                     **
#*************************************************************************
#*************************************************************************

#**********************************************************************
# $Header: /Alaska/SOURCE/Modules/PEI Ram Boot/PeiRamBoot.mak 11    7/18/12 3:41a Calvinchen $
#
# $Revision: 11 $
#
# $Date: 7/18/12 3:41a $
#**********************************************************************
# Revision History
# ----------------
# $Log: /Alaska/SOURCE/Modules/PEI Ram Boot/PeiRamBoot.mak $
# 
# 11    7/18/12 3:41a Calvinchen
# 
# 10    2/23/12 6:35a Calvinchen
# [TAG]  		EIP82264
# [Category]  	Improvement
# [Description]  	Need to do cold boot to get the correct data in rom
# hole when changing data in rom hole.
# [Files]  		PeiRamBoot.sdl
# PeiRamBoot.mak
# PeiRamBoot.h
# PeiRamBoot.c
# PeiRamBootDxe.c
# PeiRamBoot.chm
# PeiRamBoot.cif
# 
# 9     6/21/11 2:22a Calvinchen
# ¡P Bug Fixed: 
# Bug Fixed:
# 1. Build failed if Core 4.6.4.0. 
# 2. System could hang if SAVE_ENTIRE_FV_TO_MEM = 1with AMD platform. 
# 
# 8     4/22/11 1:27a Calvinchen
# 
# 7     2/11/11 3:16a Calvinchen
# Bug Fixed : System hangs after reflashed BIOS with warm reset if
# PEI_RAM_BOOT_S3_SUPPORT = 1 with fast warm boot support.
# 
# 6     12/21/10 2:24a Calvinchen
# Added an eLink "PeiRamBootObjectsList" for oem links their obj files
# for fast warm boot support. 
# 
# 5     12/14/10 2:25a Calvinchen
# Improvement : 
# 1. Added an eLink "PeiRamBootList" for fast warm boot support
# (PEI_RAM_BOOT_S3_SUPPORT = 1). If system boots in warm boot state, BIOS
# directly boot to previous copied ROM image in RAM to save time of
# copying ROM. 
# 2. Added "PEI_RAM_BOOT_S3_SUPPORT" = "2" for saving runtime memory, it
# only keep necessary PEIM FFS in runtime memory for S3 resume
# improvement. 
# 
# 4     12/02/10 6:18a Calvinchen
# Bug Fixed : Fixed Update SMBIOS Structures failed with DMI Utility.
# 
# 3     10/27/10 3:03a Calvinchen
# Initail check-in.
# 
#
#**********************************************************************
#<AMI_FHDR_START>
#
# Name: PeiRamBoot.mak 
#
# Description: Make file for PEI Ram Boot.
#
#<AMI_FHDR_END>
#**********************************************************************
Prepare : $(BUILD_DIR)/PeiRamBootElinks.h

$(BUILD_DIR)/PeiRamBootElinks.h : $(BUILD_DIR)/Token.mak
	$(ECHO) \
"#define PEI_RAM_BOOT_LIST $(PeiRamBootList)$(EOL)\
#define PEI_RAM_BOOT_FV_BOOTBLOCK_LIST $(PeiRamBootFvBootBlockList)$(EOL)\
#define PEI_RAM_BOOT_FFS_GUID_LIST $(PeiRamBootFfsGuidList)$(EOL)"\
> $@

#*************************************************************************
#*************************************************************************
#**                                                                     **
#**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
#**                                                                     **
#**                       All Rights Reserved.                          **
#**                                                                     **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
#**                                                                     **
#**                       Phone: (770)-246-8600                         **
#**                                                                     **
#*************************************************************************
#*************************************************************************
