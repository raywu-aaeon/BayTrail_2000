/** @file
  Measure TrEE required variable.

Copyright (c) 2012, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>
#include <Guid/ImageAuthentication.h>
#include <IndustryStandard/UefiTcgPlatform.h>
#include <Protocol/TrEEProtocol.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/TpmMeasurementLib.h>

typedef struct {
  CHAR16                                 *VariableName;
  EFI_GUID                               *VendorGuid;
} VARIABLE_TYPE;

VARIABLE_TYPE  mVariableType[] = {
  {EFI_SECURE_BOOT_MODE_NAME,    &gEfiGlobalVariableGuid},
  {EFI_PLATFORM_KEY_NAME,        &gEfiGlobalVariableGuid},
  {EFI_KEY_EXCHANGE_KEY_NAME,    &gEfiGlobalVariableGuid},
  {EFI_IMAGE_SECURITY_DATABASE,  &gEfiImageSecurityDatabaseGuid},
  {EFI_IMAGE_SECURITY_DATABASE1, &gEfiImageSecurityDatabaseGuid},
};

/**
  This function will return if this variable is SecureBootPolicy Variable.

  @param[in]  VariableName      A Null-terminated string that is the name of the vendor's variable.
  @param[in]  VendorGuid        A unique identifier for the vendor.

  @retval TRUE  This is SecureBootPolicy Variable
  @retval FALSE This is not SecureBootPolicy Variable
**/
BOOLEAN
IsSecureBootPolicyVariable (
  IN CHAR16                                 *VariableName,
  IN EFI_GUID                               *VendorGuid
  )
{
  UINTN   Index;

  for (Index = 0; Index < sizeof(mVariableType)/sizeof(mVariableType[0]); Index++) {
    if ((StrCmp (VariableName, mVariableType[Index].VariableName) == 0) && 
        (CompareGuid (VendorGuid, mVariableType[Index].VendorGuid))) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
  Measure and log an EFI variable, and extend the measurement result into a specific PCR.

  @param[in]  VarName           A Null-terminated string that is the name of the vendor's variable.
  @param[in]  VendorGuid        A unique identifier for the vendor.
  @param[in]  VarData           The content of the variable data.  
  @param[in]  VarSize           The size of the variable data.  
 
  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.
  @retval EFI_DEVICE_ERROR      The operation was unsuccessful.

**/
EFI_STATUS
EFIAPI
MeasureVariable (
  IN      CHAR16                    *VarName,
  IN      EFI_GUID                  *VendorGuid,
  IN      VOID                      *VarData,
  IN      UINTN                     VarSize
  )
{
  EFI_STATUS                        Status;
  UINTN                             VarNameLength;
  EFI_VARIABLE_DATA_TREE            *VarLog;
  UINT32                            VarLogSize;

  VarNameLength      = StrLen (VarName);
  VarLogSize = (UINT32)(sizeof (*VarLog) + VarNameLength * sizeof (*VarName) + VarSize
                        - sizeof (VarLog->UnicodeName) - sizeof (VarLog->VariableData));

  VarLog = (EFI_VARIABLE_DATA_TREE *) AllocateZeroPool (VarLogSize);
  if (VarLog == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (&VarLog->VariableName, VendorGuid, sizeof(VarLog->VariableName));
  VarLog->UnicodeNameLength  = VarNameLength;
  VarLog->VariableDataLength = VarSize;
  CopyMem (
     VarLog->UnicodeName,
     VarName,
     VarNameLength * sizeof (*VarName)
     );
  CopyMem (
     (CHAR16 *)VarLog->UnicodeName + VarNameLength,
     VarData,
     VarSize
     );

  DEBUG ((EFI_D_ERROR, "AuthVariableDxe: MeasureVariable (Pcr - %x, EventType - %x, ", (UINTN)7, (UINTN)EV_EFI_VARIABLE_AUTHORITY));
  DEBUG ((EFI_D_ERROR, "VariableName - %s, VendorGuid - %g)\n", VarName, VendorGuid));

  Status = MeasureAndLogData (
             7,
             EV_EFI_VARIABLE_DRIVER_CONFIG,
             VarLog,
             VarLogSize,
             VarLog,
             VarLogSize,
             TPM_MEASUREMENT_LOG_FLAGS_TPM20
             );
  FreePool (VarLog);
  return Status;
}

/**
  SecureBoot Hook for SetVariable.

  @param[in] VariableName                 Name of Variable to be found.
  @param[in] VendorGuid                   Variable vendor GUID.
  @param[in] DataSize                     Size of Data found. If size is less than the
                                          data, this value contains the required size.
  @param[in] Data                         Data pointer.

**/
VOID
EFIAPI
SecureBootHook (
  IN CHAR16                                 *VariableName,
  IN EFI_GUID                               *VendorGuid,
  IN UINTN                                  DataSize,
  IN VOID                                   *Data
  )
{
  EFI_STATUS                        Status;

  if (!IsSecureBootPolicyVariable (VariableName, VendorGuid)) {
    return ;
  }

  Status = MeasureVariable (
             VariableName,
             VendorGuid,
             Data,
             DataSize
             );
  DEBUG ((EFI_D_ERROR, "MeasureBootPolicyVariable - %r\n", Status));

  return ;
}
