/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Mobile Silicon Support Module" and is
  licensed for Intel Mobile CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
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

  SmmAccessDriver.c

Abstract:

  This is the driver that publishes the SMM Access PPI
  instance for the Pineview chipset.

Revision History:

--*/

#include "SmmAccessDriver.h"

#ifdef ECP_FLAG
EFI_GUID gPeiSmmAccessPpiGuid = PEI_SMM_ACCESS_PPI_GUID;
#endif

EFI_STATUS
EFIAPI
SmmAccessDriverEntryPoint (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
#ifdef ECP_FLAG
  IN       EFI_PEI_SERVICES     **PeiServices
#else
  IN CONST EFI_PEI_SERVICES     **PeiServices
#endif
  )
/*++

  Routine Description:

    This is the constructor for the SMM Access Ppi

  Arguments:

    FfsHeader       - FfsHeader.
    PeiServices     - General purpose services available to every PEIM.

  Returns:
    EFI_SUCCESS     - Protocol successfully started and installed
    EFI_UNSUPPORTED - Protocol can't be started

--*/
{
  EFI_STATUS                      Status;
  UINTN                           Index;
  EFI_SMRAM_HOB_DESCRIPTOR_BLOCK  *DescriptorBlock;
  SMM_ACCESS_PRIVATE_DATA         *SmmAccessPrivate;
  EFI_PEI_PPI_DESCRIPTOR          *PpiList;
#ifdef ECP_FLAG
  EFI_HOB_GUID_TYPE               *GuidHob;
#else
  EFI_PEI_HOB_POINTERS            HobList;
#endif

  //
  // Initialize private data
  //
#ifdef ECP_FLAG
  Status = ((**PeiServices).AllocatePool) (
#else
  Status = (**PeiServices).AllocatePool (
#endif
                            PeiServices,
                            sizeof(*SmmAccessPrivate),
                            (VOID**)&SmmAccessPrivate
                            );
  ASSERT_EFI_ERROR (Status);

#ifdef ECP_FLAG
  Status = ((**PeiServices).AllocatePool) (
#else
  Status = (**PeiServices).AllocatePool (
#endif
                            PeiServices,
                            sizeof(*PpiList),
                            (VOID**)&PpiList
                            );
  ASSERT_EFI_ERROR (Status);

  //
  // Build SMM related information
  //
  SmmAccessPrivate->Signature          = SMM_ACCESS_PRIVATE_DATA_SIGNATURE;

  //
  // Get Hob list
  //
#ifdef ECP_FLAG
  GuidHob = GetFirstGuidHob (&gEfiSmmPeiSmramMemoryReserveGuid);
  ASSERT (GuidHob != NULL);
  {
    return EFI_UNSUPPORTED;
  }    
  DescriptorBlock = (EFI_SMRAM_HOB_DESCRIPTOR_BLOCK *)GET_GUID_HOB_DATA(GuidHob);
#else
  HobList.Raw = GetFirstGuidHob(&gEfiSmmPeiSmramMemoryReserveGuid);
  ASSERT(HobList.Raw != NULL);
  if(HobList.Raw  == NULL)
  {
    return EFI_UNSUPPORTED;
  }    
  DescriptorBlock = GET_GUID_HOB_DATA(HobList.Raw);
  ASSERT(DescriptorBlock != NULL);
#endif

  //
  // Use the hob to publish SMRAM capabilities
  //
  ASSERT (DescriptorBlock->NumberOfSmmReservedRegions <= MAX_SMRAM_RANGES);
  for (Index = 0; Index < DescriptorBlock->NumberOfSmmReservedRegions; Index++) {
    SmmAccessPrivate->SmramDesc[Index].PhysicalStart = DescriptorBlock->Descriptor[Index].PhysicalStart;
    SmmAccessPrivate->SmramDesc[Index].CpuStart      = DescriptorBlock->Descriptor[Index].CpuStart;
    SmmAccessPrivate->SmramDesc[Index].PhysicalSize  = DescriptorBlock->Descriptor[Index].PhysicalSize;
    SmmAccessPrivate->SmramDesc[Index].RegionState   = DescriptorBlock->Descriptor[Index].RegionState;
  }

  SmmAccessPrivate->NumberRegions              = Index;
  SmmAccessPrivate->SmmAccess.Open             = Open;
  SmmAccessPrivate->SmmAccess.Close            = Close;
  SmmAccessPrivate->SmmAccess.Lock             = Lock;
  SmmAccessPrivate->SmmAccess.GetCapabilities  = GetCapabilities;
  SmmAccessPrivate->SmmAccess.LockState        = FALSE;
  SmmAccessPrivate->SmmAccess.OpenState        = FALSE;

  PpiList->Flags = (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST);
  PpiList->Guid  = &gPeiSmmAccessPpiGuid;
  PpiList->Ppi   = &SmmAccessPrivate->SmmAccess;

  Status      = (**PeiServices).InstallPpi (PeiServices, PpiList);
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Open (
  IN EFI_PEI_SERVICES         **PeiServices,
  IN PEI_SMM_ACCESS_PPI       *This,
  IN UINTN                    DescriptorIndex
  )
/*++

  Routine Description:

    This routine accepts a request to "open" a region of SMRAM.  The
    region could be legacy ABSEG, HSEG, or TSEG near top of physical memory.
    The use of "open" means that the memory is visible from all PEIM
    and SMM agents.

  Arguments:

    PeiServices           - General purpose services available to every PEIM.
    This                  - Pointer to the SMM Access Interface.
    DescriptorIndex       - Region of SMRAM to Open.

  Returns:

    EFI_SUCCESS           - The region was successfully opened.
    EFI_DEVICE_ERROR      - The region could not be opened because locked by
                            chipset.
    EFI_INVALID_PARAMETER - The descriptor index was out of bounds.

--*/
{
  SMM_ACCESS_PRIVATE_DATA   *SmmAccess;
  SmmAccess = SMM_ACCESS_PRIVATE_DATA_FROM_THIS(This);

  if (DescriptorIndex >= SmmAccess->NumberRegions) {
    DEBUG (( EFI_D_ERROR, "SMRAM region out of range\n"));
    return EFI_INVALID_PARAMETER;
  } else if (SmmAccess->SmramDesc[DescriptorIndex].RegionState & EFI_SMRAM_LOCKED) {
    DEBUG ((EFI_D_ERROR, "Cannot open a locked SMRAM region\n"));
    return EFI_DEVICE_ERROR;
  }

  //BUnit SMM Range Read Access Control (BSMRRAC) - Allow access to all CPU HOST
  MsgBus32Write (VLV_BUNIT, BUNIT_BSMRRAC, BSMRRAC_SMM_WRITE_OPEN_FOR_ALL_CORE);
  //BUnit SMM Range Write Access Control (BSMRWAC) - - Allow access to all CPU HOST
  MsgBus32Write (VLV_BUNIT, BUNIT_BSMRWAC, BSMRWAC_SMM_WRITE_OPEN_FOR_ALL_CORE);

  // END CHIPSET SPECIFIC CODE

  SmmAccess->SmramDesc[DescriptorIndex].RegionState &= ~(EFI_SMRAM_CLOSED | EFI_ALLOCATED);
  SmmAccess->SmramDesc[DescriptorIndex].RegionState |= EFI_SMRAM_OPEN;
  SmmAccess->SmmAccess.OpenState = TRUE;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Close (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN PEI_SMM_ACCESS_PPI         *This,
  IN UINTN                      DescriptorIndex
  )
/*++

  Routine Description:

  This routine accepts a request to "close" a region of SMRAM.  This is valid for
  compatible SMRAM region.

  Arguments:

    PeiServices           - General purpose services available to every PEIM.
    This                  - Pointer to the SMM Access Interface.
    DescriptorIndex       - Region of SMRAM to Close.

  Returns:

    EFI_SUCCESS           - The region was successfully closed.
    EFI_DEVICE_ERROR      - The region could not be closed because locked by
                              chipset.
    EFI_INVALID_PARAMETER - The descriptor index was out of bounds.

--*/
{
  SMM_ACCESS_PRIVATE_DATA   *SmmAccess;
  SmmAccess = SMM_ACCESS_PRIVATE_DATA_FROM_THIS(This);

  if (DescriptorIndex >= SmmAccess->NumberRegions) {
    DEBUG ((EFI_D_ERROR, "SMRAM region out of range\n"));
    return EFI_INVALID_PARAMETER;
  } else if (SmmAccess->SmramDesc[DescriptorIndex].RegionState & EFI_SMRAM_LOCKED) {
    DEBUG ((EFI_D_ERROR, "Cannot close a locked SMRAM region\n"));
    return EFI_DEVICE_ERROR;
  }
  if (SmmAccess->SmramDesc[DescriptorIndex].RegionState & EFI_SMRAM_CLOSED) {
    return EFI_DEVICE_ERROR;
  }

  SmmAccess->SmramDesc[DescriptorIndex].RegionState &= ~EFI_SMRAM_OPEN;
  SmmAccess->SmramDesc[DescriptorIndex].RegionState |= (EFI_SMRAM_CLOSED | EFI_ALLOCATED);

  SmmAccess->SmmAccess.OpenState = FALSE;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Lock (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN PEI_SMM_ACCESS_PPI         *This,
  IN UINTN                      DescriptorIndex
  )
/*++

  Routine Description:

    This routine accepts a request to "lock" SMRAM.  The
    region could be legacy AB or TSEG near top of physical memory.
    The use of "lock" means that the memory can no longer be opened
    to PEIM.

  Arguments:

    PeiServices           - General purpose services available to every PEIM.
    This                  - Pointer to the SMM Access Interface.
    DescriptorIndex       - Region of SMRAM to Lock.

  Returns:

    EFI_SUCCESS           - The region was successfully locked.
    EFI_DEVICE_ERROR      - The region could not be locked because at least
                            one range is still open.
    EFI_INVALID_PARAMETER - The descriptor index was out of bounds.

--*/
{
  SMM_ACCESS_PRIVATE_DATA   *SmmAccess;
  SmmAccess = SMM_ACCESS_PRIVATE_DATA_FROM_THIS(This);

  if (DescriptorIndex >= SmmAccess->NumberRegions) {
    DEBUG ((EFI_D_ERROR, "SMRAM region out of range\n"));
    return EFI_INVALID_PARAMETER;
  } else if (SmmAccess->SmmAccess.OpenState) {
    DEBUG ((EFI_D_ERROR, "Cannot lock SMRAM when SMRAM regions are still open\n"));
    return EFI_DEVICE_ERROR;
  }

  MsgBus32Write (VLV_BUNIT, BUNIT_BSMRCP, BSMRCP_SMM_CTRL_REG_LOCK|BSMRCP_SMM_CTRL_REG_IA_SMM|BSMRCP_SMM_CTRL_REG_PUNIT);

  SmmAccess->SmramDesc[DescriptorIndex].RegionState |= EFI_SMRAM_LOCKED;
  SmmAccess->SmmAccess.LockState = TRUE;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GetCapabilities (
  IN EFI_PEI_SERVICES                  **PeiServices,
  IN PEI_SMM_ACCESS_PPI                *This,
  IN OUT   UINTN                       *SmramMapSize,
  IN OUT   EFI_SMRAM_DESCRIPTOR        *SmramMap
  )
/*++

  Routine Description:

    This routine services a user request to discover the SMRAM
    capabilities of this platform.  This will report the possible
    ranges that are possible for SMRAM access, based upon the
    memory controller capabilities.

  Arguments:

    PeiServices           - General purpose services available to every PEIM.
    This                  - Pointer to the SMRAM Access Interface.
    SmramMapSize          - Pointer to the variable containing size of the
                              buffer to contain the description information.
    SmramMap              - Buffer containing the data describing the Smram
                              region descriptors.
  Returns:

    EFI_BUFFER_TOO_SMALL  - The user did not provide a sufficient buffer.
    EFI_SUCCESS           - The user provided a sufficiently-sized buffer.

--*/
{
  EFI_STATUS                Status;
  SMM_ACCESS_PRIVATE_DATA   *SmmAccess;
  UINTN                     BufferSize;

  SmmAccess = SMM_ACCESS_PRIVATE_DATA_FROM_THIS(This);
  BufferSize = SmmAccess->NumberRegions * sizeof(EFI_SMRAM_DESCRIPTOR);

  if (*SmramMapSize < BufferSize) {
    DEBUG ((EFI_D_ERROR, "SMRAM Map Buffer too small\n"));
    Status = EFI_BUFFER_TOO_SMALL;
  } else {
    CopyMem (SmramMap, SmmAccess->SmramDesc, *SmramMapSize);
    Status = EFI_SUCCESS;
  }
  *SmramMapSize = BufferSize;

  return Status;
}
