#*************************************************************************
#*************************************************************************
#**                                                                     **
#**        (C)Copyright 1987-2011, American Megatrends, Inc.            **
#**                                                                     **
#**                       All Rights Reserved.                          **
#**                                                                     **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
#**                                                                     **
#**                       Phone: (770)-246-8600                         **
#**                                                                     **
#*************************************************************************
#*************************************************************************
#***********************************************************************
# $Header: /Alaska/SOURCE/CPU/Intel/Cedarview/PPM/AmiPpmPolicy/AmiPpmPolicy.mak 1     11/23/10 2:00a Davidhsieh $
#
# $Revision: 1 $
#
# $Date: 11/23/10 2:00a $
#**********************************************************************
# Revision History
# ----------------
# $Log: /Alaska/SOURCE/CPU/Intel/Cedarview/PPM/AmiPpmPolicy/AmiPpmPolicy.mak $
# 
# 1     11/23/10 2:00a Davidhsieh
#
#**********************************************************************
All : AmiPpmPolicyDxe

AmiPpmPolicyDxe : $(BUILD_DIR)\AmiPpmPolicy.mak AmiPpmPolicyBin

$(BUILD_DIR)\AmiPpmPolicy.mak : $(PpmPolicyInitDxe_DIR)\$(@B).cif $(PpmPolicyInitDxe_DIR)\$(@B).mak $(BUILD_RULES)
  $(CIF2MAK) $(PpmPolicyInitDxe_DIR)\$(@B).cif $(CIF2MAK_DEFAULTS)

PpmPolicyInitDxe_INCLUDES=\
    $(PowerManagement_INCLUDES)\
    /I$(PROJECT_DIR)\Include\
    /I$(PpmPolicyInitDxe_DIR)

PpmPolicyInitDxe_LIBS=\
    $(PpmProtocolLib_LIB)\
    $(ProtocolLib_LIB)\
    $(EFIGUIDLIB)\
    $(EFISCRIPTLIB)\
    $(EDKFRAMEWORKPROTOCOLLIB)\
    $(EdkIIGlueBaseLib_LIB)\
!IF "$(x64_BUILD)"=="1"
    $(EdkIIGlueBaseLibX64_LIB)\
!ELSE
    $(EdkIIGlueBaseLibIA32_LIB)\
!ENDIF
    $(EdkIIGlueBaseIoLibIntrinsic_LIB)\
    $(EdkIIGlueDxeReportStatusCodeLib_LIB)\
    $(EdkIIGluePeiDxeDebugLibReportStatusCode_LIB)\
    $(EdkIIGlueUefiBootServicesTableLib_LIB)\
    $(EdkIIGlueBasePciLibPciExpress_LIB)\
    $(EdkIIGlueDxeServicesTableLib_LIB)\
    $(CPUIA32LIB)\

PpmPolicyInitDxe_DEFINES = $(MY_DEFINES)\
            /D"__EDKII_GLUE_MODULE_ENTRY_POINT__=PpmDxePolicyInitEntryPoint"\
            /D __EDKII_GLUE_BASE_IO_LIB_INTRINSIC__ \
            /D __EDKII_GLUE_DXE_REPORT_STATUS_CODE_LIB__ \
            /D __EDKII_GLUE_PEI_DXE_DEBUG_LIB_REPORT_STATUS_CODE__ \
            /D __EDKII_GLUE_UEFI_BOOT_SERVICES_TABLE_LIB__ \
            /D __EDKII_GLUE_BASE_PCI_LIB_PCI_EXPRESS__ \
            /D __EDKII_GLUE_DXE_SERVICES_TABLE_LIB__

AmiPpmPolicyBin : $(AMICSPLib) $(AMIDXELIB)
  $(MAKE) /$(MAKEFLAGS) $(BUILD_DEFAULTS)\
    /f $(BUILD_DIR)\AmiPpmPolicy.mak all\
    GUID=1CE12314-AFBC-11F0-8A3E-AB44B8EE3120\
    "MY_INCLUDES = $(PpmPolicyInitDxe_INCLUDES)" \
    "MY_DEFINES = $(PpmPolicyInitDxe_DEFINES)" \
    ENTRY_POINT=PpmDxePolicyInitEntryPoint\
    DEPEX1=$(PpmPolicyInitDxe_DIR)\AmiPpmPolicy.dxs\
    DEPEX1_TYPE=EFI_SECTION_DXE_DEPEX\
    TYPE=BS_DRIVER\
    EDKIIModule=DXEDRIVER\
    COMPRESS=1

SetupSdbs : $(BUILD_DIR)\AmiPpmPolicy.Sdb

$(BUILD_DIR)\AmiPpmPolicy.sdb : $(PpmPolicyInitDxe_DIR)\AmiPpmPolicy.sd $(PpmPolicyInitDxe_DIR)\AmiPpmPolicy.uni
	$(STRGATHER) -i INCLUDE -parse -newdb -db $(BUILD_DIR)\AmiPpmPolicy.sdb $(PpmPolicyInitDxe_DIR)\AmiPpmPolicy.uni
	$(STRGATHER) -scan -db $(BUILD_DIR)\AmiPpmPolicy.sdb -od $(BUILD_DIR)\AmiPpmPolicy.sdb $(PpmPolicyInitDxe_DIR)\AmiPpmPolicy.sd
#*************************************************************************
#*************************************************************************
#**                                                                     **
#**        (C)Copyright 1987-2010, American Megatrends, Inc.            **
#**                                                                     **
#**                       All Rights Reserved.                          **
#**                                                                     **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
#**                                                                     **
#**                       Phone: (770)-246-8600                         **
#**                                                                     **
#*************************************************************************
#*************************************************************************
	