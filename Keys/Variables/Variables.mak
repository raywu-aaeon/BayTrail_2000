#*************************************************************************
#*************************************************************************
#**                                                                     **
#**        (C)Copyright 1985-2014, American Megatrends, Inc.            **
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
# Name: Variables.mak
#
# Description: includes Root Platform key (PR) into the BIOS rom
# 
#<AMI_FHDR_END>
#*************************************************************************
Prepare: \
$(BUILD_DIR)$(PATH_SLASH)PK \
$(BUILD_DIR)$(PATH_SLASH)KEK \
$(BUILD_DIR)$(PATH_SLASH)db

ifeq ($(DBT_include),1)
Prepare: $(BUILD_DIR)$(PATH_SLASH)dbt
endif

ifeq ($(DBX_include),1)
Prepare: $(BUILD_DIR)$(PATH_SLASH)dbx
endif

#---------------------------------------------------------------------------
#   Include Secure Variables as .FFS
#---------------------------------------------------------------------------
$(BUILD_DIR)$(PATH_SLASH)PK : $(PkVar)
	$(CP) $< $@
$(BUILD_DIR)$(PATH_SLASH)KEK: $(KekVar)
	 $(CP) $< $@
$(BUILD_DIR)$(PATH_SLASH)db: $(dbVar)
	 $(CP) $< $@
$(BUILD_DIR)$(PATH_SLASH)dbt: $(dbtVar)
	 $(CP) $< $@
$(BUILD_DIR)$(PATH_SLASH)dbx: $(dbxVar)
	 $(CP) $< $@
  
#*************************************************************************
#*************************************************************************
#**                                                                     **
#**        (C)Copyright 1985-2014, American Megatrends, Inc.            **
#**                                                                     **
#**                       All Rights Reserved.                          **
#**                                                                     **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
#**                                                                     **
#**                       Phone: (770)-246-8600                         **
#**                                                                     **
#*************************************************************************
#*************************************************************************
