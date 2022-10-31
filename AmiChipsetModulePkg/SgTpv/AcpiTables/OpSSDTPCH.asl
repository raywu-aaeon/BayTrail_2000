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
// $Header: /Alaska/SOURCE/Modules/SharkBayRefCodes/SwitchableGraphics/Sg TPV/Sg Acpi Tables/SgTpvPCH/OpSSDTPCH.asl 3     2/21/13 5:42a Joshchou $
//
// $Revision: 3 $
//
// $Date: 2/21/13 5:42a $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/Modules/SharkBayRefCodes/SwitchableGraphics/Sg TPV/Sg Acpi Tables/SgTpvPCH/OpSSDTPCH.asl $
// 
// 
//**********************************************************************
DefinitionBlock (
        "NvOpt.aml",
        "SSDT",
        1,
        "OptRef",
        "NvdPch",
        0x1000
        ) {       

#define OPTIMUS_DSM_GUID 1
//#define NBCI_DSM_GUID 1

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
External(ASL_IGPU_SCOPE._DSM, MethodObj)
External(ASL_DGPUPCH_SCOPE.SGST, MethodObj)
External(ASL_DGPUPCH_SCOPE.HGON, MethodObj)
External(ASL_DGPUPCH_SCOPE.HGOF, MethodObj)

//External(\DSEL)
//External(\ESEL)
//External(\SSEL)
//External(\PSEL)
//External(\HLRS)
//External(\PWEN)
//External(\PWOK)
External(\SGMD)
External(\SGFL)
External(\SSMP)

#include <NVdGPUPCH.ASL>     // Include DGPU device namespace
#include <NViGPUPCH.ASL>     // Include NVHG DSM calls
//#include <NViGDmiscPCH.ASL>  // Include misc event callback methods

#if ASL_NV_VENTURA_SUPPORT == 1
#include <NvVenturaPCH.ASL>  // Include Ventura support
#endif
#if ASL_NV_GPS_SUPPORT == 1
#include <NvGPSPCH.ASL>  // Include GPS support
#endif

#if ASL_NV_GC6_SUPPORT == 1
#include <NvGC6PCH.ASL>  // Include GC6 support
#endif

#if ASL_NV_GC6_SUPPORT == 2
#include <NvGC6v2PCH.ASL>  // Include GC6 support
#endif

 Scope(ASL_PCI_SCOPE) 
 {
//<AMI_PHDR_START>
//------------------------------------------------------------------------
//
// Procedure:    WMI1
//
// Description:  WMI MXM Mapper. ASL Device is used to acccess Nv Optimus native method via WMI API
//
//-------------------------------------------------------------------------
//<AMI_PHDR_END> 
    Device(WMI1) // placed within PCI Bus scope parallel to iGPU 
    { 
        Name(_HID, "PNP0C14")
        Name(_UID, "OPT1")    
   
        Name(_WDG, Buffer() 
        {
            // Methods GUID {F6CB5C3C-9CAE-4ebd-B577-931EA32A2CC0}
            0x3C, 0x5C, 0xCB, 0xF6, 0xAE, 0x9C, 0xbd, 0x4e, 0xB5, 0x77, 0x93, 0x1E,
            0xA3, 0x2A, 0x2C, 0xC0,
            0x4D, 0x58, // Object ID "MX" = method "WMMX"
            1,          // Instance Count
            0x02,       // Flags (WMIACPI_REGFLAG_METHOD)        
        }) // End of _WDG

//<AMI_PHDR_START>
//------------------------------------------------------------------------
//
// Procedure:    WMMX
//
// Description:  WMI Method execution tunnel. MXM Native methods are called via WMMX index.
//
// Input:
//          Arg1:   Integer     GPU index. 0x10-iGPU, 0x100+PCIe Bus number for the GPU
//
//  Output:
//          Buffer      specific to the funcion being called
//-------------------------------------------------------------------------
//<AMI_PHDR_END> 
        Method(WMMX, 3)
        {

            //Arg1 = 0x10 indicates iGPU, 0x100+PCIe Bus number for the GPU
            // 
            CreateDwordField(Arg2, 0, FUNC)                 // Get the function name

            If (LEqual(FUNC, 0x534F525F))                   // "_ROM"
            {
                If (LGreaterEqual(SizeOf(Arg2), 8))
                {
                    CreateDwordField(Arg2, 4, ARGS)
                    CreateDwordField(Arg2, 8, XARG)
                    Return(ASL_DGPUPCH_SCOPE._ROM(ARGS, XARG))
                }
            }            

            If (LEqual(FUNC, 0x4D53445F))                   // "_DSM"
            {
                If (LGreaterEqual(SizeOf(Arg2), 28))
                {
                    CreateField(Arg2, 0, 128, MUID)
                    CreateDwordField(Arg2, 16, REVI)
                    CreateDwordField(Arg2, 20, SFNC)
                    CreateField(Arg2, 0xe0, 0x20, XRG0)
                    
//                    If(LNotEqual(Arg1,0x10))          
//                    {
                    If (CondRefOf(ASL_IGPU_SCOPE._DSM)) // common with dGPU DSM functions
                    {
                        Return(ASL_IGPU_SCOPE._DSM(MUID, REVI, SFNC, XRG0))
                    }
//                    }
                }
            }            
            Return(0)
        } // End of WMMX
   } // End of WMI1 Device
 } // end scope PCI0
} // end SSDT   
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
