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
# Name: MakeSignVars.mak
#
# Description:  
# 
#
#<AMI_FHDR_END>
#*************************************************************************
PK_ATTRIBS = 0x27
ifeq ($(KEK_Append),1)
KEK_ATTRIBS = 0x67
else
KEK_ATTRIBS = 0x27
endif
ifeq ($(db_Append),1)
db_ATTRIBS = 0x67
else
db_ATTRIBS = 0x27
endif
ifeq ($(dbt_Append),1)
dbt_ATTRIBS = 0x67
else
dbt_ATTRIBS = 0x27
endif
ifeq ($(dbx_Append),1)
dbx_ATTRIBS = 0x67
else
dbx_ATTRIBS = 0x27
endif

PKbin = $(PkVarName:.var=.bin)
KEKbin = $(KekVarName:.var=.bin)
dbBin = $(dbVarName:.var=.bin)
dbtBin = $(dbtVarName:.var=.bin)
dbxBin = $(dbxVarName:.var=.bin)

PKser = $(PkVarName:.var=_serialize.bin)
KEKser = $(KekVarName:.var=_serialize.bin)
dbSer = $(dbVarName:.var=_serialize.bin)
dbtSer = $(dbtVarName:.var=_serialize.bin)
dbxSer = $(dbxVarName:.var=_serialize.bin)
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
ifneq ($(CertList_PK),)
Prepare: $(PkVarName)
$(PkVarName): $(PKbin)
	@$(Certificates_DIR)\serialize.bat 1 $(PK_ATTRIBS) $(PK_signedBy) $(PKbin) $(PKser) $(PFX_Password)
	@$(Certificates_DIR)\authhdr.bat $(PKser).p7
	@copy /b $(BUILD_DIR)\authhdr.bin+$(PKbin) $@
else
PkVarName := $(PkVarFile)
endif

ifneq ($(CertList_KEK),)
Prepare: $(KekVarName)
$(KekVarName): $(KEKbin)
	@$(Certificates_DIR)\serialize.bat 2 $(KEK_ATTRIBS) $(KEK_signedBy) $(KEKbin) $(KEKser) $(PFX_Password)
	@$(Certificates_DIR)\authhdr.bat $(KEKser).p7
	@copy /b $(BUILD_DIR)\authhdr.bin+$(KEKbin) $@
else
KekVarName :=  $(KekVarFile)
endif

ifneq ($(CertList_db),)
Prepare: $(dbVarName)
$(dbVarName): $(dbBin)
	@$(Certificates_DIR)\serialize.bat 3 $(db_ATTRIBS) $(db_signedBy) $(dbBin) $(dbSer) $(PFX_Password)
	@$(Certificates_DIR)\authhdr.bat $(dbSer).p7
	@copy /b $(BUILD_DIR)\authhdr.bin+$(dbBin) $@
else
dbVarName := $(dbVarFile)
endif

ifneq ($(CertList_dbt),)
Prepare: $(dbtVarName)
$(dbtVarName): $(dbtBin)
	@$(Certificates_DIR)\serialize.bat 4 $(dbt_ATTRIBS) $(db_signedBy) $(dbtBin) $(dbtSer) $(PFX_Password)
	@$(Certificates_DIR)\authhdr.bat $(dbtSer).p7
	@copy /b $(BUILD_DIR)\authhdr.bin+$(dbtBin) $@
else
dbtVarName  := $(dbtVarFile)
endif

ifneq ($(CertList_dbx),)
Prepare: $(dbxVarName)
$(dbxVarName): $(dbxBin)
	@$(Certificates_DIR)\serialize.bat 5 $(dbx_ATTRIBS) $(db_signedBy) $(dbxBin) $(dbxSer) $(PFX_Password)
	@$(Certificates_DIR)\authhdr.bat $(dbxSer).p7
	@copy /b $(BUILD_DIR)\authhdr.bin+$(dbxBin) $@
else
dbxVarName := $(dbxVarFile)
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
