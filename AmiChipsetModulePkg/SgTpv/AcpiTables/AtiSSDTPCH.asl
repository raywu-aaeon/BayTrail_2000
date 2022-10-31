//****************************************************************************
//****************************************************************************
//**                                                                        **
//**         (C)Copyright 1985-2013, American Megatrends, Inc.              **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
//****************************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/SharkBayRefCodes/SwitchableGraphics/Sg TPV/Sg Acpi Tables/SgTpvPCH/AtiSSDTPCH.asl 1     1/15/13 6:02a Joshchou $
//
// $Revision: 1 $
//
// $Date: 1/15/13 6:02a $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/Modules/SharkBayRefCodes/SwitchableGraphics/Sg TPV/Sg Acpi Tables/SgTpvPCH/AtiSSDTPCH.asl $
// 
// 
// 
//**********************************************************************
DefinitionBlock (
        "Amd.aml",
        "SSDT",
        1,
        "AmdRef",
        "AmdPch",
        0x1000
        ){       

External(P8XH, MethodObj) 
#if defined(ASL_SGTPV_ASL_DEBUG) && (ASL_SGTPV_ASL_DEBUG ==1)
#define P8DB(arg0, arg1, arg2) P8XH (0, arg1) P8XH (1, arg0) sleep(arg2)
#else
#define P8DB(arg0, arg1, arg2) 
#endif

External(ASL_PCI_SCOPE, DeviceObj)
External(ASL_SG_ULT_RP_NUM, DeviceObj)
External(ASL_DGPUPCH_SCOPE, DeviceObj)
External(ASL_IGPU_SCOPE, DeviceObj)
External(ASL_DGPUPCH_SCOPE._ADR, DeviceObj)
External(ASL_DGPUPCH_SCOPE.SGST, MethodObj)
External(ASL_DGPUPCH_SCOPE.HGON, MethodObj)
External(ASL_DGPUPCH_SCOPE.HGOF, MethodObj)

#include <ATdGPUPCH.ASL>     // Include dGPU device namespace
#include <ATiGPUPCH.ASL>     // Include IGD _DSM and AMD ATIF/ATPM/ATRM methods
#include <ATiGDmiscPCH.ASL>  // Include misc event callback methods

} // end SSDT       
