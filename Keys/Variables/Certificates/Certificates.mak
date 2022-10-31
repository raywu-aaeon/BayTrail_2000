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
# Name: Certificates.mak
#
# Description:  
# 
#
#<AMI_FHDR_END>
#*************************************************************************
PKbin  := $(PkVarName:.var=.bin)
KEKbin := $(KekVarName:.var=.bin)
dbBin  := $(dbVarName:.var=.bin)
dbtBin := $(dbtVarName:.var=.bin)
dbxBin := $(dbxVarName:.var=.bin)
#---------------------------------------------------------------------------
#        Create Secure Variables from list of Certificates
#---------------------------------------------------------------------------
$(PKbin)  : $(CertList_PK)
$(KEKbin) : $(CertList_KEK)
$(dbBin)  : $(CertList_db)
$(dbtBin) : $(CertList_dbt)
$(dbxBin) : $(CertList_dbx)

$(PKbin) $(KEKbin) $(dbBin) $(dbtBin) $(dbxBin) :
	@if exist $(BUILD_DIR)\sig.dat del $(BUILD_DIR)\sig.dat
	@$(Certificates_DIR)\buildsig.bat $^
	@copy /b $(BUILD_DIR)\sig.dat $@

#---------------------------------------------------------------------------
$(BUILD_DIR)\authhdr.bin: $(Certificates_DIR)\authhdr.bat $(Certificates_DIR)\authhdr.c $(BUILD_DIR)\timestamp.h
	@if exist $(BUILD_DIR)\sig.dat del $(BUILD_DIR)\sig.dat
	@$(Certificates_DIR)\authhdr.bat $(BUILD_DIR)\sig.dat

#---------------------------------------------------------------------------
$(PkVarName) $(KekVarName) $(dbVarName) $(dbtVarName) $(dbxVarName): $(BUILD_DIR)\authhdr.bin
	@copy /b $(subst $(SPACE),+,$^) $@

#may not exist
ifneq ($(CertList_PK),)
Prepare: $(PkVarName)
$(PkVarName): $(PKbin)
else
PkVarName := $(PkVarFile)
endif

ifneq ($(CertList_KEK),)
Prepare: $(KekVarName)
$(KekVarName): $(KEKbin)
else
KekVarName :=  $(KekVarFile)
endif

#may not exist
ifneq ($(CertList_db),)
Prepare: $(dbVarName)
$(dbVarName): $(dbBin)
else
dbVarName := $(dbVarFile)
endif

ifneq ($(CertList_dbt),)
Prepare: $(dbtVarName)
$(dbtVarName): $(dbtBin)
else
dbtVarName  := $(dbtVarFile)
endif

ifneq ($(CertList_dbx),)
Prepare: $(dbxVarName)
$(dbxVarName): $(dbxBin)
else
dbxVarName :=  $(dbxVarFile)
endif

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
