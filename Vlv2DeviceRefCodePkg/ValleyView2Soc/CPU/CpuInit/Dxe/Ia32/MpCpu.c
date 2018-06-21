/*++

Copyright (c) 1999 - 2007 Intel Corporation. All rights reserved
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
/*
#include "Tiano.h"
#include "EfiDriverLib.h"
#include "Pei.h"
#include "MpService.h"
#include "PeiLib.h"
#include "CpuDxe.h"
#include "ThermalMonitor.h"
#include "Peci.h"
#include "C1e.h"
#include "CState.h"
#include "Xd.h"
#include "MchkInit.h"
#include "FastString.h"
#include "Prefetcher.h"
#include "Gv3.h"
#include "Ferr.h"
#include "Smrr.h"
#include "vt.h"
//
// Consumered PPI and Protocol
//
#include EFI_ARCH_PROTOCOL_DEFINITION (Cpu)
#include EFI_PROTOCOL_DEFINITION (MpService)
#include EFI_PROTOCOL_DEFINITION (LoadPe32Image)
#include EFI_PPI_DEFINITION (Variable)
*/
#include "PlatformMpService.h"
extern EFI_CPU_MICROCODE_HEADER **mMicrocodePointerBuffer;


ACPI_CPU_DATA_COMPATIBILITY             *mAcpiCpuData;
MP_SYSTEM_DATA            *mMPSystemData;
#ifdef ECP_FLAG
EFI_DEVICE_PATH_PROTOCOL*
AppendDevicePath (
  IN CONST EFI_DEVICE_PATH_PROTOCOL *Src1,
  IN CONST EFI_DEVICE_PATH_PROTOCOL *Src2
  );
#else
EFI_DEVICE_PATH_PROTOCOL  *
AppendDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL  *Src1,
  IN EFI_DEVICE_PATH_PROTOCOL  *Src2
  );
#endif

UINTN
DevicePathSize (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  );

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
  //VOID                  *Registration;

  if (ImageHandle != NULL) {

    Status = AllocateReservedMemoryBelow4G (
               sizeof (MP_CPU_RESERVED_DATA),
               (VOID **) &MpCpuReservedData
               );

    if (EFI_ERROR (Status)) {
      return Status;
    }

    ZeroMem (MpCpuReservedData, sizeof (MP_CPU_RESERVED_DATA));

    AcpiVariableSet                       = &(MpCpuReservedData->AcpiVariableSet);
    mMPSystemData                         = &(MpCpuReservedData->MPSystemData);
    mAcpiCpuData                          = &(AcpiVariableSet->AcpiCpuData);

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
/*
    Status = gBS->HandleProtocol (
                    ImageHandle,
                    &gEfiLoadedImageProtocolGuid,
                    (VOID **) &LoadedImage
                    );
    ASSERT_EFI_ERROR (Status);

    Status = gBS->HandleProtocol (
                    LoadedImage->DeviceHandle,
                    &gEfiDevicePathProtocolGuid,
                    (VOID **) &ImageDevicePath
                    );
    ASSERT_EFI_ERROR (Status);

    CompleteFilePath = AppendDevicePath (
                         ImageDevicePath,
                         LoadedImage->FilePath
                         );

    Status = gBS->LocateProtocol (&gEfiLoadPeImageGuid, NULL, (VOID **) &LoadPeImageEx);
    ASSERT_EFI_ERROR (Status);

    DstPages = EFI_SIZE_TO_PAGES (((UINTN) (LoadedImage->ImageSize))) + 1;
    Status = gBS->AllocatePages (
                    AllocateAnyPages,
                    EfiACPIMemoryNVS,
                    DstPages,
                    &Dst
                    );
    ASSERT_EFI_ERROR (Status);

    Pages = DstPages;
    Status = LoadPeImageEx->LoadPeImage (
                              LoadPeImageEx,
                              ImageHandle,
                              CompleteFilePath,
                              NULL,
                              0,
                              Dst,
                              &Pages,
                              &NewCpuDriver,
                              &EntryPoint,
                              EFI_LOAD_PE_IMAGE_ATTRIBUTE_NONE
                              );
    ASSERT_EFI_ERROR (Status);

    INITIALIZE_SCRIPT (ImageHandle, SystemTable);

    SCRIPT_INFORMATION_ASCII_STRING (
      EFI_ACPI_S3_RESUME_SCRIPT_TABLE, 
      "MpCpuEntryBegin"
    );

    SCRIPT_DISPATCH (EFI_ACPI_S3_RESUME_SCRIPT_TABLE, EntryPoint);

    SCRIPT_INFORMATION_ASCII_STRING (
      EFI_ACPI_S3_RESUME_SCRIPT_TABLE, 
      "MpCpuEntryEnd"
    );    
*/
  } else {
/*
    PeiServices     = (EFI_PEI_SERVICES **) SystemTable;

    AcpiVariableSet = NULL;
    Status          = (*PeiServices)->GetHobList (PeiServices, &Hob.Raw);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    while (!END_OF_HOB_LIST (Hob)) {
      if ((Hob.Header->HobType == EFI_HOB_TYPE_GUID_EXTENSION) &&
          CompareGuid ( &Hob.Guid->Name, &gEfiAcpiVariableGuid)) {
        AcpiVariableSet = (ACPI_VARIABLE_SET *) (Hob.Raw   + 
                          sizeof (EFI_HOB_GENERIC_HEADER)  + 
                          sizeof (EFI_GUID));
        break;
      }
      Hob.Raw = GET_NEXT_HOB (Hob);
    }

    if (AcpiVariableSet == NULL) {
      return EFI_NOT_FOUND;
    }

    mAcpiCpuData             = (ACPI_CPU_DATA *) &(AcpiVariableSet->AcpiCpuData);
    mAcpiCpuData->S3BootPath  = TRUE;
    MPSystemData = (MP_SYSTEM_DATA *) (UINTN) mAcpiCpuData->CpuPrivateData;
    //
    // Update microcode for BSP
    //
    S3PathInitMicrocode ((EFI_CPU_MICROCODE_HEADER **) (UINTN) mAcpiCpuData->MicrocodePointerBuffer);
    InitializeThermalMonitor (MPSystemData->Tm2Core2BusRatio, MPSystemData->Tm2Vid);
    Gv3Initialization (MPSystemData->Gv3Enable);
    InitializePeci (MPSystemData->PeciEnable);
    InitializeFerr (MPSystemData->FerrInterruptReportingEnable);
    EnableCpuIdMaximumValueLimit (MPSystemData->LimitCpuidMaximumValue);
    EnableP4FastStringOperation (MPSystemData->FastString);
    SmrrInitialization(MPSystemData->SmrrEnable);
    S3PathVmxInitialization (MPSystemData->VtEnable, MPSystemData->LtEnable, MPSystemData->VtSupported, MPSystemData->LtSupported);

    //
    // Try to enabled Cache Prefetch if supported
    //
    ProcessorsPrefetcherInitialization (
      MPSystemData->HardwarePrefetcherEnable,
      MPSystemData->AdjacentCacheLinePrefetchEnable,
      MPSystemData->IPPrefetcherEnable,
      MPSystemData->DCUPrefetcherEnable
      );

    C2eInitialization (MPSystemData->C2eEnable);
    C3eInitialization (FALSE);
    C4eInitialization (FALSE);
        
    if (MPSystemData->IsC1eSupported && MPSystemData->C1eEnable) {
      EnableC1E ();
    }

    //
    // Saves the BSP Mtrr settings for sync on APs.
    //
    ReadMtrrRegisters ();
    
    WakeUpAPs (ImageHandle, SystemTable); */
  }

  return EFI_SUCCESS;
}


#if 0
UINTN
DevicePathSize (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
/*++

Routine Description:

  Function is used to get the DevicePath's szie.a Src1 and Src2 together.

Arguments:

  DevicePath  - A pointer to a device path data structure.

Returns:

  The device path's size.

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *Start;

  if (DevicePath == NULL) {
    return 0;
  }

  //
  // Search for the end of the device path structure
  //
  Start = DevicePath;
  while (!EfiIsDevicePathEnd (DevicePath)) {
    DevicePath = EfiNextDevicePathNode (DevicePath);
  }

  //
  // Compute the size and add back in the size of the end device path structure
  //
  return ((UINTN) DevicePath - (UINTN) Start) + sizeof (EFI_DEVICE_PATH_PROTOCOL);
}

EFI_DEVICE_PATH_PROTOCOL *
AppendDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL  *Src1,
  IN EFI_DEVICE_PATH_PROTOCOL  *Src2
  )
/*++

Routine Description:
  Function is used to append a Src1 and Src2 together.

Arguments:
  Src1  - A pointer to a device path data structure.

  Src2  - A pointer to a device path data structure.

Returns:

  A pointer to the new device path is returned.
  NULL is returned if space for the new device path could not be allocated from pool.
  It is up to the caller to free the memory used by Src1 and Src2 if they are no longer needed.

--*/
{
  UINTN                     Size;
  UINTN                     Size1;
  UINTN                     Size2;
  EFI_DEVICE_PATH_PROTOCOL  *NewDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *SecondDevicePath;

  //
  // Allocate space for the combined device path. It only has one end node of
  // length EFI_DEVICE_PATH_PROTOCOL
  //
  Size1 = DevicePathSize (Src1);
  Size2 = DevicePathSize (Src2);
  Size  = Size1 + Size2;

  if (Size1 != 0 && Size2 != 0) {
    Size -= sizeof (EFI_DEVICE_PATH_PROTOCOL);
  }

  NewDevicePath = EfiLibAllocatePool (Size);

  if (NewDevicePath == NULL) {
    return NULL;
  }
  gBS->CopyMem (NewDevicePath, Src1, Size1);

  //
  // Over write Src1 EndNode and do the copy
  //
  if (Size1 != 0) {
    SecondDevicePath = (EFI_DEVICE_PATH_PROTOCOL *) ((CHAR8 *) NewDevicePath + (Size1 - sizeof (EFI_DEVICE_PATH_PROTOCOL)));
  } else {
    SecondDevicePath = NewDevicePath;
  }
  gBS->CopyMem (SecondDevicePath, Src2, Size2);

  return NewDevicePath;
}
#endif
