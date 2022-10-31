/*++

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PlatformEarlyInit.c

Abstract:

  Do platform specific PEI stage initializations.

--*/

/*++
 This file contains an 'Intel Peripheral Driver' and is
 licensed for Intel CPUs and chipsets under the terms of your
 license agreement with Intel or your vendor.  This file may
 be modified by the user, subject to additional terms of the
 license agreement
--*/
#include "PlatformEarlyInit.h"

#pragma optimize ("", off)

extern EFI_GUID gEfiSetupVariableGuid;

//
// The reserved SMBus addresses are defined in PlatformDxe.h file.
//
static UINT8 mSmbusRsvdAddresses[] = PLATFORM_SMBUS_RSVD_ADDRESSES;

static PEI_SMBUS_POLICY_PPI         mSmbusPolicyPpi = {
  SMBUS_BASE_ADDRESS,
  SMBUS_BUS_DEV_FUNC,
  PLATFORM_NUM_SMBUS_RSVD_ADDRESSES,
  mSmbusRsvdAddresses
};

static EFI_PEI_PPI_DESCRIPTOR       mInstallSmbusPolicyPpi = {
  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gPeiSmbusPolicyPpiGuid,
  &mSmbusPolicyPpi
};

static EFI_PEI_NOTIFY_DESCRIPTOR    mNotifyList[] = {
    {
        EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK,
        &gEfiEndOfPeiSignalPpiGuid,
        EndOfPeiPpiNotifyCallback
    },
    {
        (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK| EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
        &gEfiPeiMemoryDiscoveredPpiGuid,
        MemoryDiscoveredPpiNotifyCallback
    }
};

EFI_STATUS
EFIAPI
GetWakeupEventAndSaveToHob(
    IN CONST EFI_PEI_SERVICES   **PeiServices
)
/*++

Routine Description:

  Parse the status registers for figuring out the wake-up event and save it into
  an GUID HOB which will be referenced later. However, modification is required
  to meet the chipset register definition and the practical hardware design. Thus,
  this is just an example.

Arguments:

  PeiServices   - pointer to the PEI Service Table
  EFI_SUCCESS   - Always return Success
Returns:

  None


--*/
{
    UINT16  Pm1Sts;
    UINTN   Gpe0Sts;
    UINTN   WakeEventData;

    //
    // Read the ACPI registers
    //
    Pm1Sts  = IoRead16(ACPI_BASE_ADDRESS + R_PCH_ACPI_PM1_STS);
    Gpe0Sts = IoRead32(ACPI_BASE_ADDRESS + R_PCH_ACPI_GPE0a_STS);

    //
    // Figure out the wake-up event
    //
    if((Pm1Sts & B_PCH_ACPI_PM1_STS_PWRBTN) != 0) {
        WakeEventData = SMBIOS_WAKEUP_TYPE_POWER_SWITCH;
    } else if(((Pm1Sts & B_PCH_ACPI_PM1_STS_WAK) != 0)) {
        WakeEventData = SMBIOS_WAKEUP_TYPE_PCI_PME;
    } else if(Gpe0Sts != 0) {
        WakeEventData = SMBIOS_WAKEUP_TYPE_OTHERS;
    } else {
        WakeEventData = SMBIOS_WAKEUP_TYPE_UNKNOWN;
    }
    DEBUG((EFI_D_ERROR, "WakeEventData: %04x\n", WakeEventData));
    DEBUG((EFI_D_ERROR, "ACPI Wake Status Register: %04x\n", Pm1Sts));

    return EFI_SUCCESS;
}

EFI_STATUS
GetSetupVariable(
    IN CONST EFI_PEI_SERVICES                **PeiServices,
    IN   SETUP_DATA          *SystemConfiguration
)
{
    UINTN                        VariableSize;
    EFI_STATUS                   Status;
    EFI_PEI_READ_ONLY_VARIABLE2_PPI   *Variable;

    ZeroMem(SystemConfiguration, sizeof(SETUP_DATA));

    Status = (*PeiServices)->LocatePpi(PeiServices,
                                       &gEfiPeiReadOnlyVariable2PpiGuid,
                                       0,
                                       NULL,
                                       &Variable
                                      );
    ASSERT_EFI_ERROR(Status);

    //
    // Use normal setup default from NVRAM variable,
    // the Platform Mode (manufacturing/safe/normal) is handle in PeiGetVariable.
    //
    VariableSize = sizeof(SETUP_DATA);
    Status = Variable->GetVariable(Variable,
                                   L"Setup",
                                   &gEfiSetupVariableGuid,
                                   NULL,
                                   &VariableSize,
                                   SystemConfiguration);
    ASSERT_EFI_ERROR(Status);
    return Status;
}

EFI_STATUS
EFIAPI
PlatformEarlyInitEntry(
    IN EFI_FFS_FILE_HEADER       *FfsHeader,
    IN CONST EFI_PEI_SERVICES    **PeiServices
)
/*++

Routine Description:

  Platform specific initializations in stage1.

Arguments:

  FfsHeader         Pointer to the PEIM FFS file header.
  PeiServices       General purpose services available to every PEIM.

Returns:

  EFI_SUCCESS       Operation completed successfully.
  Otherwise         Platform initialization failed.
--*/
{
    EFI_STATUS                  		Status;
    SETUP_DATA        			SystemConfiguration;
//  SB_SETUP_DATA               PchPolicyData;
//  NB_SETUP_DATA               VlvPolicyData;
    EFI_PLATFORM_INFO_HOB      *PlatformInfo;
//EFI_PEI_STALL_PPI           		*StallPpi;
    EFI_PEI_HOB_POINTERS        	Hob;
    EFI_PLATFORM_CPU_INFO       PlatformCpuInfo;

    //
    // Initialize SmbusPolicy PPI
    //
    Status = (*PeiServices)->InstallPpi(PeiServices, &mInstallSmbusPolicyPpi);
    ASSERT_EFI_ERROR (Status);

    //
    // Variable initialization
    //
    ZeroMem(&PlatformCpuInfo, sizeof(EFI_PLATFORM_CPU_INFO));

    //
    // Set the some PCI and chipset range as UC
    // And align to 1M at leaset
    //
    Hob.Raw = GetFirstGuidHob(&gEfiPlatformInfoGuid);
    ASSERT(Hob.Raw != NULL);
    PlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);

    //
    // Get setup variable. This can only be done after BootMode is updated
    //
    GetSetupVariable(PeiServices, &SystemConfiguration);

//  //
//  // Do basic PCH init
//  //
//  Status = PlatformPchInit (PeiServices, &PchPolicyData, PlatformInfo->PlatformType);
//  ASSERT_EFI_ERROR (Status);
//
//  //
//  // Initialize VlvPolicy PPI
//  //
//  Status = VlvPolicyInit (PeiServices, &VlvPolicyData);
//  ASSERT_EFI_ERROR (Status);

    //
    // Update PlatformInfo HOB according to setup variable
    //
    PlatformInfoUpdate(PeiServices, PlatformInfo, &SystemConfiguration);

    //
    // Do basic CPU init
    //
    PlatformCpuInit(PeiServices, &SystemConfiguration, &PlatformCpuInfo);
    //
    // Perform basic SSA related platform initialization
    //
//  PlatformSsaInit (PeiServices, &VlvPolicyData);

    //
    // Initialize platform PPIs
    //
//  Status = (*PeiServices)->InstallPpi (PeiServices, &mPpiList[0]);
//  ASSERT_EFI_ERROR (Status);


//  InstallPlatformClocksNotify (PeiServices);

//  InstallPlatformSysCtrlGPIONotify(PeiServices);
    //
    // Set LVDS_BKLT_CTRL to 50%.
    //
    MmPci8(0, 0, 2, 0, 0xF4) = 128;
    //
    // Initialize platform PPIs
    //
    Status = (*PeiServices)->NotifyPpi(PeiServices, &mNotifyList[0]);
    ASSERT_EFI_ERROR(Status);

    return Status;
}

EFI_STATUS
EFIAPI
CpuOnlyReset(
    IN CONST EFI_PEI_SERVICES   **PeiServices
)
{
//  MsgBus32Write(CDV_UNIT_PUNIT, PUNIT_CPU_RST, 0x01)

    _asm {
        xor   ecx, ecx
        HltLoop:
        hlt
        hlt
        hlt
        loop  HltLoop
    }

    //
    // If we get here we need to mark it as a failure.
    //
    return EFI_UNSUPPORTED;
}


#pragma optimize ("", on)

