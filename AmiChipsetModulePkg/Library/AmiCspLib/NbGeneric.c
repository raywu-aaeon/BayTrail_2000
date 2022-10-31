//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//*************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:        NbGeneric.C
//
// Description: This file contains generic NB code that is common between
//              various components such as NB PEI, DXE etc
//
// Notes:       MAKE SURE NO PEI OR DXE SPECIFIC CODE IS NEEDED
//
//
//*****************************************************************************


//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        NbGeneric.C
//
// Description: This file contains generic NB code that is common between
//              various components such as NB PEI, DXE etc
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>


// Module specific Includes
#include <VlvAccess.h>
#include "Efi.h"
#include "Token.h"
#include <AmiLib.h>
#include <AmiCspLib.h>
#include <AmiDxeLib.h>
#include <Protocol/PciRootBridgeIo.h>

#include "VlvAccess.h" //CSP20140329_22
#include "MchRegs.h" //CSP20140329_22

// Produced Protocols

// GUID Definitions

// Portable Constants

// Function Prototypes

// Function Definition

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#if     CSM_SUPPORT

EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *gPciRootBridgeIo;
//----------------------------------------------------------------------------
#define REGION_DECODE_ROM   0       //Read/Write ROM
#define REGION_LOCK     1       //Read Only
#define REGION_UNLOCK   3       //Read/Write Only
#define LEGACY_REGION_LOCK          0               // Read only Read to RAM, Write to ROM
#define LEGACY_REGION_BOOT_LOCK     1
#define LEGACY_REGION_UNLOCK        2               // Read/Write to RAM
#define LEGACY_REGION_DECODE_ROM    3               // Read/Write to ROM

typedef struct {
    UINT8   Register;
    UINT8   Mask;
    UINT32  StartAddress;
    UINT32  Length;
} PAM_STRUCT;

/** Porting required for the following structure **/
PAM_STRUCT gPamStruct[] = {
    {0x91, 0xfc, 0xc0000, 0x4000},
    {0x91, 0xcf, 0xc4000, 0x4000},
    {0x92, 0xfc, 0xc8000, 0x4000},
    {0x92, 0xcf, 0xcc000, 0x4000},
    {0x93, 0xfc, 0xd0000, 0x4000},
    {0x93, 0xcf, 0xd4000, 0x4000},
    {0x94, 0xfc, 0xd8000, 0x4000},
    {0x94, 0xcf, 0xdc000, 0x4000},
    {0x95, 0xfc, 0xe0000, 0x4000},
    {0x95, 0xcf, 0xe4000, 0x4000},
    {0x96, 0xcf, 0xec000, 0x4000},
    {0x96, 0xfc, 0xe8000, 0x4000},
    {0x90, 0xcf, 0xf0000, 0x10000}
};

#define NUM_PAM_ENTRIES (sizeof(gPamStruct) / sizeof(PAM_STRUCT))

typedef struct {
    UINT64                  Address;
    EFI_BOOT_SCRIPT_WIDTH   Width;
} BOOT_SCRIPT_PCI_REGISTER_SAVE;

BOOT_SCRIPT_PCI_REGISTER_SAVE gPciRegistersSave[] = {
    0x00000090,  EfiBootScriptWidthUint32,
    0x00000094,  EfiBootScriptWidthUint16,
    0x00000096,  EfiBootScriptWidthUint8
};

//----------------------------------------------------------------------------
// Start OF CSM Related Porting Hooks
//----------------------------------------------------------------------------

//
// The following data structure specifies the PCI device/function number of the root
// bridge(s). Number of entries in this table defined by ROOT_BRIDGE_COUNT
// This table is a missing link between RootBridgeIo and PciIo, which allows to update
// BusNumXlat table with actual bus numbers.
// Each entry in the table is a pair of RootBridge UID (UINT32), provided in RootBridge
// device path, and PCI Dev/Func number (UINT8) that can be used to access Root Bridge on
// PCI bus.
ROOT_BRIDGE_MAPPING_ENTRY   RbMap[ROOT_BRIDGE_COUNT] = {
//          RB ID           Device function number
    { 0,            0x00}       // PORTING PORTING - Include device function number of RB
};
UINTN                       RbCount = ROOT_BRIDGE_COUNT;

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NbGetPamStartEndIndex
//
// Description: Helper function to get the Start and End Index for
//              PAM register table.
//
// Input:       UINT32 StartAddress        Start address of the PAM area
//              UINT32 Length              Length of the PAM area
//              UINT32 *StartIndex         Start Index of the PAM (OUT)
//              UINT32 *EndIndex           End Index of the PAM (OUT)
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS NbGetPamStartEndIndex(
    IN  UINT32   StartAddress,
    IN  UINT32   Length,
    OUT UINT32  *StartIndex,
    OUT UINT32  *EndIndex
)
{
    UINT32 StartIdx;
    UINT32 EndIdx;
    UINT32 TotalLength = 0;

    if(StartAddress < gPamStruct[0].StartAddress) return EFI_INVALID_PARAMETER;

    for(StartIdx = 0; StartIdx < NUM_PAM_ENTRIES; ++StartIdx) {
        if(StartAddress <= gPamStruct[StartIdx].StartAddress) break;
    }
    if(StartAddress < gPamStruct[StartIdx].StartAddress) StartIdx--;

    if(StartIdx == NUM_PAM_ENTRIES) return EFI_INVALID_PARAMETER;

    // Adjust the length of the requested region if starting address is
    // out of bounds.
    Length += StartAddress - gPamStruct[StartIdx].StartAddress;

    for(EndIdx = StartIdx; EndIdx < NUM_PAM_ENTRIES; ++EndIdx) {
        TotalLength += gPamStruct[EndIdx].Length;
        if(TotalLength >= Length) break;
    }
    if(EndIdx == NUM_PAM_ENTRIES) return EFI_INVALID_PARAMETER;

    *StartIndex = StartIdx;
    *EndIndex = EndIdx;
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NbProgramPamRegisters
//
// Description: Program 0xc0000 - 0xfffff regions to Lock/Unlock.
//
// Input:       EFI_BOOT_SERVICES    *gBS            Pointer to the boot services table
//              EFI_RUNTIME_SERVICES *gRS            Pointer to the runtime services table
//              UINT32               StartAddress    Start address of the PAM area
//              UINT32               Length          Length of the PAM area
//              UINT8                Setting         Settings to be set for the above area
//              UINT32               *Granularity    Granularity of the above area (return value)
//
// Output:      EFI_STATUS
//
// Notes:       Here is the control flow of this function:
//              1. Search the structure for the first entry matching
//                 the StartAddress.
//              2. If not found, return EFI_INVALID_PARAMETER.
//              3. Find the last entry in structure for the region to program,
//                 by adding the lengths of the entries.
//              4. If not found, return EFI_INVALID_PARAMETER.
//              5. Read/Write each register for the entry to set region.
//              6. Return the Granularity for the region.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
NbProgramPamRegisters(
    IN EFI_BOOT_SERVICES    *gBS,
    IN EFI_RUNTIME_SERVICES *gRS,
    IN UINT32               StartAddress,
    IN UINT32               Length,
    IN UINT8                Setting,
    IN OUT UINT32           *Granularity
)
{
	// UNLOCK = normal operation (read/write to DRAM)
	// LOCK = reads to DRAM, writes to DMI
	// BOOTLOCK = LOCK = reads to DRAM, writes to DMI

	// Since there is no concept of write routing for legacy
	// regions in CDV, we will always assume that the caller
	// is requesting reads to be directed to DRAM.  The Mode parameter
	// is therefore ignored.

	UINT32 	Data = 0xFFFFFFFF;
	UINT32 	ZeroData = 0;

	if ((StartAddress < 0xF0000) && ((StartAddress + Length - 1) >= 0xE0000)) 
	{//F-segment are routed to DRAM
		if(Setting == LEGACY_REGION_LOCK || Setting == LEGACY_REGION_BOOT_LOCK) {
			MsgBus32And(VLV_BUNIT, BUNIT_BMISC, ZeroData, B_BMISC_RFSDRAM );
		} else if(Setting == LEGACY_REGION_UNLOCK) {
			MsgBus32Or(VLV_BUNIT, BUNIT_BMISC, Data, B_BMISC_RFSDRAM );
		}
	}

	if ((StartAddress < 0x100000) && ((StartAddress + Length - 1) >= 0xF0000)) 
	{//E-segment are routed to DRAM
		if(Setting == LEGACY_REGION_LOCK || Setting == LEGACY_REGION_BOOT_LOCK) {
			MsgBus32And(VLV_BUNIT, BUNIT_BMISC, ZeroData, B_BMISC_RESDRAM );
		} else if(Setting == LEGACY_REGION_UNLOCK) {
			MsgBus32Or(VLV_BUNIT, BUNIT_BMISC, Data, B_BMISC_RESDRAM );
		}
	}

	if (Granularity) {
		*Granularity = 64 * 1024;	// All regions are 64K.
	}

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NbPeiProgramPamRegisters
//
// Description: Program 0xc0000 - 0xfffff regions to Lock/Unlock during PEI stage
//
// Input:       EFI_PEI_SERVICES **PeiServices   Pointer to the PEI services table
//              UINT32           StartAddress    Start address of the PAM area
//              UINT32           Length          Length of the PAM area
//              UINT8            Setting         Settings to be set for the above area
//              UINT32           *Granularity    Granularity of the above area (return value)
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS NbPeiProgramPamRegisters(
    IN EFI_PEI_SERVICES **PeiServices,
    IN UINT32           StartAddress,
    IN UINT32           Length,
    IN UINT8            Setting,
    IN OUT UINT32       *Granularity OPTIONAL
)
{
//    EFI_PEI_PCI_CFG_PPI *PciCfg = (*PeiServices)->PciCfg;
    EFI_STATUS  Status;
//  UINT64      PciAddress;
    UINT32      StartIndex;
    UINT32      EndIndex;
    UINT32      i;
//  UINT8       Data;

    Status = NbGetPamStartEndIndex(
                 StartAddress,
                 Length,
                 &StartIndex,
                 &EndIndex
             );
    if(EFI_ERROR(Status)) return Status;

    if(Setting == LEGACY_REGION_LOCK) Setting = REGION_LOCK;
    else  if(Setting == LEGACY_REGION_UNLOCK) Setting = REGION_UNLOCK;
    else Setting = REGION_DECODE_ROM;

    for(i = StartIndex; i <= EndIndex; ++i) {
        /** CHIPSET PORTING
                //Bus 0, Device 0x10, Function 0
                PciAddress = (0 << 24) + (0x10 << 16) + (0 << 8) + gPamStruct[i].Register;
                PciCfg->Read(
                    PeiServices,
                    PciCfg,
                    EfiPeiPciCfgWidthUint8,
                    PciAddress,
                    &Data
                );

                Data &= gPamStruct[i].Mask;
                Data |= (gPamStruct[i].Mask == 0xfc) ? Setting : Setting << 4;

                PciCfg->Write(
                    PeiServices,
                    PciCfg,
                    EfiPeiPciCfgWidthUint8,
                    PciAddress,
                    &Data
                );
         ** CHIPSET PORTING **/
    }

    if(Granularity)
        *Granularity = ((StartAddress + Length) < 0xf0000) ? 0x4000 : 0x10000;


    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NbPamWriteBootScript
//
// Description: Writes the final settings of PAM registers to the BOOT Script
//
// Input:       *BootScriptSave - Pointer to S3 boot script save protocol
//
// Output:      EFI_STATUS
//
// Notes:       Here is the control flow of this function:
//              1. From the Pci register save table, read the pci register to save.
//              2. Write to the boot script the value.
//              3. Repeat 1 & 2 for all table entries.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS NbPamWriteBootScript(
    IN AMI_S3_SAVE_PROTOCOL     *BootScriptSave
)
{
    EFI_STATUS  Status = EFI_SUCCESS;
    UINTN       i;
    UINT32      Value;

    for(i = 0; i < sizeof(gPciRegistersSave) / sizeof(BOOT_SCRIPT_PCI_REGISTER_SAVE); ++i) {
        gPciRootBridgeIo->Pci.Read(
            gPciRootBridgeIo,
            gPciRegistersSave[i].Width,
            gPciRegistersSave[i].Address,
            1,
            &Value
        );
        BOOT_SCRIPT_S3_PCI_CONFIG_WRITE_MACRO(
            BootScriptSave,
            gPciRegistersSave[i].Width,
            gPciRegistersSave[i].Address,
            1,
            &Value
        );
    }

    return Status;
}


//----------------------------------------------------------------------------
#endif          // END OF CSM Related Porting Hooks
//----------------------------------------------------------------------------


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NbResetCpuOnly
//
// Description: This function issues a CPU only reset.
//
// Input:       None
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID NbResetCpuOnly(
    VOID
)
{
    // Porting Required.
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NbRuntimeShadowRamWrite
//
// Description: This function provides runtime interface to enable/disable
//              writing in E000-F000 segment
//
// Input:       IN BOOLEAN Enable - if TRUE - enable writing, if FALSE - disable
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID NbRuntimeShadowRamWrite(
    IN BOOLEAN Enable
)
{
    // Porting Required.
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NbRuntimeShadowRamWriteExt
//
// Description: This function provides runtime interface to enable/disable
//              writing in C000-F000 segment
//
// Input:       IN SHADOW_ARRTIBUTE   The shadowing attributes of the BIOS
//                                    range
//              IN SHADOW_BIOS_RANGE  The BIOS range which is going to be
//                                    shadowed
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
NbRuntimeShadowRamWriteExt(
    IN SHADOW_ARRTIBUTE   Attribute,
    IN SHADOW_BIOS_RANGE  Range
)
{
    /**** PORTING REQUIRED ****
    UINT8   PamData=0;
    UINT8   PamIndex=0;

    switch (Attribute) {
    case Disabled:
      PamData = 0x0;
      break;
    case ReadOnly:
      PamData = 0x1;
      break;
    case WriteOnly:
      PamData = 0x2;
      break;
    case ReadWrite:
      PamData = 0x3;
      break;
    default:
      return EFI_INVALID_PARAMETER;
    }

    switch (Range) {
    case C4000_16K:
      PamData <<= 4;
    case C0000_16K:
      PamIndex = PAM1;
      break;
    case CC000_16K:
      PamData <<= 4;
    case C8000_16K:
      PamIndex = PAM2;
      break;
    case D4000_16K:
      PamData <<= 4;
    case D0000_16K:
      PamIndex = PAM3;
      break;
    case DC000_16K:
      PamData <<= 4;
    case D8000_16K:
      PamIndex = PAM4;
      break;
    case E4000_16K:
      PamData <<= 4;
    case E0000_16K:
      PamIndex = PAM5;
      break;
    case EC000_16K:
      PamData <<= 4;
    case E8000_16K:
      PamIndex = PAM6;
      break;
    case F0000_64K:
      PamData <<= 4;
      PamIndex = PAM0;
      break;
    default:
      return EFI_INVALID_PARAMETER;
    }

    WRITE_PCI8_NB(PamIndex, PamData);
    **** PORTING REQUIRED ****/
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NBGetTsegBase
//
// Description: Returns the base address of TSEG.
//
// Input:       None
//
// Output:      UINT32 - The Base Address of TSEG.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32 NBGetTsegBase(VOID)
{
    /** Porting is required**/
//####return (MmioRead32 ((UINTN) NB_PCIE_CFG_ADDRESS(0, 0, 0, 0xB8)) & 0xFFF00000);
//CSP20140329_22 >>
	UINT32 	dTSegBase;
	UINT32	buffer32;
	  //BSMMRRL
	  //15:0 Lower Bound (SMMStart): These bits are compared with bits 35:20 of the incoming address 
	  //To determine the lower 1MB aligned value of the protected range.
	  MsgBus32Read(VLV_UNIT_BUNIT,BUNIT_BSMMRRL_OFFSET,buffer32);
	  dTSegBase =  (UINT32)(EFI_PHYSICAL_ADDRESS)(LShiftU64((buffer32&0x0000FFFF),20));
    return dTSegBase;
//CSP20140329_22 <<
}

// (EIP129308+)>>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: CheckPeiFvCopyToRam
//
// Description: Check system is cold or warm boot    
//
// Input:       PeiServices  - The PEI core services table.
//
// Output:      PeiFvCopyToRam - TRUE  for cold boot.
//                             - FALSE for warm boot.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN
CheckPeiFvCopyToRam (
  IN  EFI_PEI_SERVICES  **PeiServices
)
{

  UINT32        Buff32;
  BOOLEAN       PeiFvCopyToRam;

  Buff32 = MmioRead32 (PMC_BASE_ADDRESS + 0x20);

//  if (((Buff32 & BIT23) != 0) && ((Buff32 & BIT21) != 0)) 
  if ((Buff32 & BIT21) != 0) {
    PeiFvCopyToRam = FALSE;
  } else {
    PeiFvCopyToRam = TRUE;
  }

  return PeiFvCopyToRam;

}
// (EIP129308+)<<

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
