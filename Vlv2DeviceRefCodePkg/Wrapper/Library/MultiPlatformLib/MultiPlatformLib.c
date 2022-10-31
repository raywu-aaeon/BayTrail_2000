/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/** @file
  Multiplatform initialization.

  Copyright (c) 2010, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/

#include <MultiPlatformLib.h>

EFI_STATUS
MultiPlatformInfoInit(
    IN CONST EFI_PEI_SERVICES             **PeiServices,
    IN OUT EFI_PLATFORM_INFO_HOB          *PlatformInfoHob
//  IN OUT EFI_PLATFORM_CPU_INFO *PlatformCpuInfo
)
/*++
Routine Description:

  Platform Type detection. Because the PEI globle variable
  is in the flash, it could not change directly.So use
  2 PPIs to distinguish the platform type.

Arguments:

  FfsHeader    -  Pointer to Firmware File System file header.
  PeiServices  -  General purpose services available to every PEIM.

Returns:

  EFI_SUCCESS  -  Memory initialization completed successfully.
  Others       -  All other error conditions encountered result in an ASSERT.

--*/
{
//(EIP110662+)>>
#if !defined(PROGRAM_GPIO_SUPPORT)||(PROGRAM_GPIO_SUPPORT==0)
    EFI_STATUS      Status;
#endif
//(EIP110662+)<<

    //
    // Update IIO Type
    //
    //
    // Device ID
    //
    PlatformInfoHob->IohSku = MmPci16(0, MC_BUS, MC_DEV, MC_FUN, PCI_DEVICE_ID_OFFSET);

    //

    //
    PlatformInfoHob->IohRevision = MmPci8(0, MC_BUS, MC_DEV, MC_FUN, PCI_REVISION_ID_OFFSET);

    //
    // Update ICH Type
    //
    //
    // Device ID
    //
    PlatformInfoHob->IchSku = PchLpcPciCfg16(PCI_DEVICE_ID_OFFSET);

    //

    //
    PlatformInfoHob->IchRevision = PchLpcPciCfg8(PCI_REVISION_ID_OFFSET);

    //
    // Don't support BASE above 4GB currently
    //
    PlatformInfoHob->PciData.PciExpressSize     = PCIEX_LENGTH;
    PlatformInfoHob->PciData.PciExpressBase     = PCIEX_BASE_ADDRESS;

    PlatformInfoHob->PciData.PciResourceMem32Base  = (UINT32)(PlatformInfoHob->PciData.PciExpressBase - RES_MEM32_MIN_LEN);
    PlatformInfoHob->PciData.PciResourceMem32Limit = (UINT32)(PlatformInfoHob->PciData.PciExpressBase -1);

    PlatformInfoHob->PciData.PciResourceMem64Base   = RES_MEM64_36_BASE;
    PlatformInfoHob->PciData.PciResourceMem64Limit  = RES_MEM64_36_LIMIT;
    PlatformInfoHob->CpuData.CpuAddressWidth        = 36;

    PlatformInfoHob->MemData.MemMir0 = PlatformInfoHob->PciData.PciResourceMem64Base;
    PlatformInfoHob->MemData.MemMir1 = PlatformInfoHob->PciData.PciResourceMem64Limit + 1;

    PlatformInfoHob->PciData.PciResourceMinSecBus  = 1;  //can be changed by SystemConfiguration->PciMinSecondaryBus;

    //
    // Set MemMaxTolm to the lowest address between PCIe Base and PCI32 Base
    //
    if(PlatformInfoHob->PciData.PciExpressBase > PlatformInfoHob->PciData.PciResourceMem32Base) {
        PlatformInfoHob->MemData.MemMaxTolm = (UINT32) PlatformInfoHob->PciData.PciResourceMem32Base;
    } else {
        PlatformInfoHob->MemData.MemMaxTolm = (UINT32) PlatformInfoHob->PciData.PciExpressBase;
    }
    PlatformInfoHob->MemData.MemTolm = PlatformInfoHob->MemData.MemMaxTolm;

    //
    // Platform PCI MMIO Size in unit of 1MB
    //
    PlatformInfoHob->MemData.MmioSize = 0x1000 - (UINT16)(PlatformInfoHob->MemData.MemMaxTolm >> 20);

    //
    // Update Memory Config HOB size which will be used by SaveMemoryConfig
    //
    PlatformInfoHob->MemData.MemConfigSize = sizeof(MRC_PARAMS_SAVE_RESTORE);

    //
    // Enable ICH IOAPIC
    //
    PlatformInfoHob->SysData.SysIoApicEnable  = ICH_IOAPIC;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    DEBUG((EFI_D_ERROR,  "PlatformInfoHob->IohSku : %x\n", PlatformInfoHob->IohSku));
    DEBUG((EFI_D_ERROR,  "PlatformInfoHob->IohRevision : %x\n", PlatformInfoHob->IohRevision));
    DEBUG((EFI_D_ERROR,  "PlatformInfoHob->PciData.PciExpressSize : %x\n", PlatformInfoHob->PciData.PciExpressSize));
    DEBUG((EFI_D_ERROR,  "PlatformInfoHob->PciData.PciExpressBase : %x\n", PlatformInfoHob->PciData.PciExpressBase));
    DEBUG((EFI_D_ERROR,  "PlatformInfoHob->PciData.PciResourceMem32Base : %x\n", PlatformInfoHob->PciData.PciResourceMem32Base));
    DEBUG((EFI_D_ERROR,  "PlatformInfoHob->PciData.PciResourceMem32Limit : %x\n", PlatformInfoHob->PciData.PciResourceMem32Limit));
    DEBUG((EFI_D_ERROR,  "PlatformInfoHob->PciData.PciResourceMem64Base : %x\n", PlatformInfoHob->PciData.PciResourceMem64Base));
    DEBUG((EFI_D_ERROR,  "PlatformInfoHob->PciData.PciResourceMem64Limit : %x\n", PlatformInfoHob->PciData.PciResourceMem64Limit));
    DEBUG((EFI_D_ERROR,  "PlatformInfoHob->CpuData.CpuAddressWidth : %x\n", PlatformInfoHob->CpuData.CpuAddressWidth));
    DEBUG((EFI_D_ERROR,  "PlatformInfoHob->MemData.MemMir0 : %x\n", PlatformInfoHob->MemData.MemMir0));
    DEBUG((EFI_D_ERROR,  "PlatformInfoHob->MemData.MemMir1 : %x\n", PlatformInfoHob->MemData.MemMir1));
    DEBUG((EFI_D_ERROR,  "PlatformInfoHob->PciData.PciResourceMinSecBus : %x\n", PlatformInfoHob->PciData.PciResourceMinSecBus));
    DEBUG((EFI_D_ERROR,  "PlatformInfoHob->MemData.MemMaxTolm : %x\n", PlatformInfoHob->MemData.MemMaxTolm));
    DEBUG((EFI_D_ERROR,  "PlatformInfoHob->MemData.MemTolm : %x\n", PlatformInfoHob->MemData.MemTolm));
    DEBUG((EFI_D_ERROR,  "PlatformInfoHob->MemData.MmioSize : %x\n", PlatformInfoHob->MemData.MmioSize));
    DEBUG((EFI_D_ERROR,  "PlatformInfoHob->MemData.MemConfigSize : %x\n", PlatformInfoHob->MemData.MemConfigSize));
    DEBUG((EFI_D_ERROR,  "PlatformInfoHob->SysData.SysIoApicEnable : %x\n", PlatformInfoHob->SysData.SysIoApicEnable));

    DEBUG((EFI_D_ERROR,  "PlatformFlavor is %x (%x=tablet,%x=mobile,%x=desktop)\n", PlatformInfoHob->PlatformFlavor,FlavorTablet,FlavorMobile,FlavorDesktop));

    //
    // Get Platform Info and fill the Hob
    //
    PlatformInfoHob->RevisonId = PLATFORM_INFO_HOB_REVISION;

//(EIP110662+)>>
#if !defined(PROGRAM_GPIO_SUPPORT)||(PROGRAM_GPIO_SUPPORT==0)
    //
    // Get GPIO table
    //
    Status = MultiPlatformGpioTableInit(PeiServices, PlatformInfoHob);
    ASSERT_EFI_ERROR(Status);

    //
    // Program GPIO
    //
    Status = MultiPlatformGpioProgram(PeiServices, PlatformInfoHob);
    ASSERT_EFI_ERROR(Status);
#endif
//(EIP110662+)<<

    // Update OemId
//    Status = InitializeBoardOemId (PeiServices, PlatformInfoHob);
//    Status = InitializeBoardSsidSvid (PeiServices, PlatformInfoHob);

    return EFI_SUCCESS;
}
