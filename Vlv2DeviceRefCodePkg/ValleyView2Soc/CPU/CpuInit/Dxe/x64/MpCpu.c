/*++

Copyright (c) 2007 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  MpCpu.c

Abstract:

  MP Support driver

--*/

#include "CpuDxe.h"
#include "MpCommon.h"

#include "PlatformMpService.h"

extern EFI_CPU_MICROCODE_HEADER **mMicrocodePointerBuffer;

ACPI_CPU_DATA_COMPATIBILITY                   *mAcpiCpuData;
MP_SYSTEM_DATA                  *mMPSystemData;

//
// Function declarations
//
EFI_STATUS
InitializeMpSupport (
  IN EFI_HANDLE                       ImageHandle,
  IN EFI_SYSTEM_TABLE                 *SystemTable
  )
/*++

Routine Description:

  Initializes MP support in the system.

Arguments:
  ImageHandle - Image handle of the loaded driver
  SystemTable - Pointer to the System Table

Returns:

  EFI_SUCCESS          - Multiple processors are initialized successfully.
  EFI_NOT_FOUND        - The ACPI variable is not found in S3 boot path.
  EFI_OUT_OF_RESOURCES - No enough resoruces (such as out of memory).

--*/
{
  EFI_STATUS            Status;
  ACPI_VARIABLE_SET_COMPATIBILITY     *AcpiVariableSet;
  MP_CPU_RESERVED_DATA  *MpCpuReservedData;
// VOID                  *Registration;

  Status = AllocateReservedMemoryBelow4G (
            sizeof (MP_CPU_RESERVED_DATA),
            (VOID **) &MpCpuReservedData
            );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (MpCpuReservedData, sizeof (MP_CPU_RESERVED_DATA));

  AcpiVariableSet = &(MpCpuReservedData->AcpiVariableSet);
  mMPSystemData   = &(MpCpuReservedData->MPSystemData);
  mAcpiCpuData    = &(AcpiVariableSet->AcpiCpuData);

  CopyMem (
    MpCpuReservedData->MicrocodePointerBuffer,
    mMicrocodePointerBuffer,
    sizeof (EFI_CPU_MICROCODE_HEADER *) * (NUMBER_OF_MICROCODE_UPDATE + 1)
    );

  mAcpiCpuData->CpuPrivateData          = (EFI_PHYSICAL_ADDRESS)(UINTN)(&(mMPSystemData->S3DataPointer));
  mAcpiCpuData->S3BootPath              = FALSE;
  mAcpiCpuData->MicrocodePointerBuffer  = (EFI_PHYSICAL_ADDRESS)(UINTN) (MpCpuReservedData->MicrocodePointerBuffer);
  mAcpiCpuData->GdtrProfile             = (EFI_PHYSICAL_ADDRESS)(UINTN) (&(MpCpuReservedData->GdtrProfile));
  mAcpiCpuData->IdtrProfile             = (EFI_PHYSICAL_ADDRESS)(UINTN) (&(MpCpuReservedData->IdtrProfile));

  //EfiCreateProtocolNotifyEvent (
  //  &gEfiGenericMemTestProtocolGuid,
  //  TPL_CALLBACK,
  //  MpServiceInitialize,
  //  NULL,
  //  &Registration
  //  );

  MpServiceInitialize ();


  Status = gRT->SetVariable (
                  ACPI_GLOBAL_VARIABLE,
                  &gEfiAcpiVariableCompatiblityGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  sizeof (UINTN),
                  &AcpiVariableSet
                  );
  if (EFI_ERROR (Status)) {
#ifdef ECP_FLAG
    (gBS->FreePool) (AcpiVariableSet);
#else
    gBS->FreePool (AcpiVariableSet);
#endif
    return Status;
  }

  return EFI_SUCCESS;
}
