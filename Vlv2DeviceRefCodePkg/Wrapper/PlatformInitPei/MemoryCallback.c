/*++

Copyright (c)  1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  MemoryCallback.c

Abstract:

  EFI 2.0 PEIM to provide the platform support functionality on the Bridgeport.

--*/

#include "PlatformEarlyInit.h"
#include <Ppi/ReadOnlyVariable2.h>
#include <Setup.h>

#ifndef FLASH_BASE_ADDRESS
#define FLASH_BASE_ADDRESS 0xFFE00000
#endif

#ifndef FLASH_SIZE
#define FLASH_SIZE 0x200000
#endif

static EFI_GUID gSetupGuid = SETUP_GUID;    //(CSP20130313G+)

VOID
UpdateDefaultSetupValue(
    IN  EFI_PLATFORM_INFO_HOB       *PlatformInfo
)
{
    return;
}


EFI_STATUS
EndOfPeiPpiNotifyCallback(
    IN CONST EFI_PEI_SERVICES           **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
    IN VOID                       *Ppi
)
/*++

Routine Description:

  PEI termination callback.

Arguments:

  PeiServices       -  General purpose services available to every PEIM.
  NotifyDescriptor  -  Not uesed.
  Ppi               -  Not uesed.

Returns:

  EFI_SUCCESS  -  If the interface could be successfully
                  installed.

--*/
{
    EFI_STATUS                  Status;
//    UINT64                      MemoryTop;
//    UINT64                      LowUncableBase;
    EFI_PLATFORM_INFO_HOB       *PlatformInfo;
    UINT32                      HecBaseHigh;
    EFI_BOOT_MODE               BootMode;
    EFI_PEI_HOB_POINTERS        Hob;


    Status = (*PeiServices)->GetBootMode(PeiServices, &BootMode);

    ASSERT_EFI_ERROR(Status);

    //
    // Set the some PCI and chipset range as UC
    // And align to 1M at leaset
    //
    Hob.Raw = GetFirstGuidHob(&gEfiPlatformInfoGuid);
    ASSERT(Hob.Raw != NULL);
    PlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);

    UpdateDefaultSetupValue(PlatformInfo);

    DEBUG((EFI_D_ERROR, "Memory TOLM: %X\n", PlatformInfo->MemData.MemTolm));
    DEBUG((EFI_D_ERROR, "PCIE OSBASE: %lX\n", PlatformInfo->PciData.PciExpressBase));
    DEBUG(
        (EFI_D_ERROR,
         "PCIE   BASE: %lX     Size : %X\n",
         PlatformInfo->PciData.PciExpressBase,
         PlatformInfo->PciData.PciExpressSize)
    );
    DEBUG(
        (EFI_D_ERROR,
         "PCI32  BASE: %X     Limit: %X\n",
         PlatformInfo->PciData.PciResourceMem32Base,
         PlatformInfo->PciData.PciResourceMem32Limit)
    );
    DEBUG(
        (EFI_D_ERROR,
         "PCI64  BASE: %lX     Limit: %lX\n",
         PlatformInfo->PciData.PciResourceMem64Base,
         PlatformInfo->PciData.PciResourceMem64Limit)
    );
    DEBUG((EFI_D_ERROR, "UC    START: %lX     End  : %lX\n", PlatformInfo->MemData.MemMir0, PlatformInfo->MemData.MemMir1));

//    LowUncableBase = PlatformInfo->MemData.MemMaxTolm;
//    LowUncableBase &= (0x0FFF00000);
//    MemoryTop = (0x100000000);

    if(BootMode != BOOT_ON_S3_RESUME) {
        //
        // In BIOS, HECBASE will be always below 4GB
        //
        HecBaseHigh = (UINT32) RShiftU64(PlatformInfo->PciData.PciExpressBase, 28);
        ASSERT(HecBaseHigh < 16);

        //
        // Programe HECBASE for DXE phase
        //
        // PlatformInfo->PciData.PciExpressSize == 0x10000000
        //
    }

    return Status;
}

//(CSP20130313G+)>>
EFI_STATUS
FTPMPolicyInit(
    IN CONST EFI_PEI_SERVICES        **PeiServices
)
{
    EFI_STATUS                                  Status;
    EFI_PEI_PPI_DESCRIPTOR         *SECFTPMPolicyPpiDesc;
    SEC_FTPM_POLICY_PPI             *SECFTPMPolicyPpi;
    SETUP_DATA                      	    SetupData;
    UINTN                                             VariableSize = sizeof(SETUP_DATA);
    EFI_PEI_READ_ONLY_VARIABLE2_PPI *ReadOnlyVariable = NULL;

    Status = (*PeiServices)->AllocatePool(PeiServices, sizeof(EFI_PEI_PPI_DESCRIPTOR), &SECFTPMPolicyPpiDesc);
    ASSERT_EFI_ERROR(Status);

    Status = (*PeiServices)->AllocatePool(PeiServices, sizeof(SEC_FTPM_POLICY_PPI), &SECFTPMPolicyPpi);
    ASSERT_EFI_ERROR(Status);

    Status = (*PeiServices)->LocatePpi(PeiServices, \
                                       &gEfiPeiReadOnlyVariable2PpiGuid, \
                                       0, \
                                       NULL, \
                                       &ReadOnlyVariable);

    if(!EFI_ERROR(Status)) {
        Status = ReadOnlyVariable->GetVariable(ReadOnlyVariable, \
                                               L"Setup", \
                                               &gSetupGuid, \
                                               NULL, \
                                               &VariableSize, \
                                               &SetupData);
    }

    //
    // Initialize PPI
    //
    (*PeiServices)->SetMem((VOID *)SECFTPMPolicyPpi, sizeof(SEC_FTPM_POLICY_PPI), 0);
    SECFTPMPolicyPpiDesc->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
    SECFTPMPolicyPpiDesc->Guid = &gSeCfTPMPolicyPpiGuid;
    SECFTPMPolicyPpiDesc->Ppi = SECFTPMPolicyPpi;
    SECFTPMPolicyPpi->fTPMEnable = SetupData.fTPM;
    Status = (*PeiServices)->InstallPpi(PeiServices, SECFTPMPolicyPpiDesc);
    DEBUG((EFI_D_ERROR, "install FTPM ppi: %r\n", Status));
    ASSERT_EFI_ERROR(Status);

    return EFI_SUCCESS;
}
//(CSP20130313G+)<<

EFI_STATUS
EFIAPI
MemoryDiscoveredPpiNotifyCallback(
    IN CONST EFI_PEI_SERVICES           **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
    IN VOID                       *Ppi
)
/*++

Routine Description:

  Install Firmware Volume Hob's once there is main memory

Arguments:

  PeiServices       General purpose services available to every PEIM.
  NotifyDescriptor  Notify that this module published.
  Ppi               PPI that was installed.

Returns:

  EFI_SUCCESS     The function completed successfully.

--*/
{

    return EFI_SUCCESS;
}


EFI_STATUS
ValidateFvHeader(
    IN EFI_FIRMWARE_VOLUME_HEADER            *FwVolHeader
)
{
    UINT16  *Ptr;
    UINT16  HeaderLength;
    UINT16  Checksum;

    //
    // Verify the header revision, header signature, length
    // Length of FvBlock cannot be 2**64-1
    // HeaderLength cannot be an odd number
    //
    if((FwVolHeader->Revision != EFI_FVH_REVISION) ||
            (FwVolHeader->Signature != EFI_FVH_SIGNATURE) ||
            (FwVolHeader->FvLength == ((UINT64) -1)) ||
            ((FwVolHeader->HeaderLength & 0x01) != 0)
      ) {
        return EFI_NOT_FOUND;
    }
    //
    // Verify the header checksum
    //
    HeaderLength  = (UINT16)(FwVolHeader->HeaderLength / 2);
    Ptr           = (UINT16 *) FwVolHeader;
    Checksum      = 0;
    while(HeaderLength > 0) {
        Checksum = *Ptr++;
        HeaderLength--;
    }

    if(Checksum != 0) {
        return EFI_NOT_FOUND;
    }

    return EFI_SUCCESS;
}
