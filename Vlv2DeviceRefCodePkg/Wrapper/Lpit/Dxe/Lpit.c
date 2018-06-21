#include <Lpit.h>

/**
  Entry Point for this driver.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
LpitDriverEntry(
  IN  EFI_HANDLE        ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
)
{
  EFI_STATUS                                Status;
  EFI_ACPI_TABLE_PROTOCOL                   *AcpiTable = NULL;
  UINTN                                     AcpiTableKey = 0;
  UINT8                                     OemId[6] = ACPI_OEM_ID_MAK;  //EIP134732
  UINT8                                     OemTblId[8] = ACPI_OEM_TBL_ID_MAK;  //EIP134732


  // Locate protocol.
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, &AcpiTable);
  ASSERT_EFI_ERROR (Status);
  
  CopyMem(&Lpit.Header.OemId, OemId, 6);  //EIP134732
  CopyMem(&Lpit.Header.OemTableId, OemTblId, 8);  //EIP134732
  Lpit.Header.OemRevision   = EFI_ACPI_OEM_REVISION;
  Lpit.Header.CreatorId   = EFI_ACPI_CREATOR_ID;
  Lpit.Header.CreatorRevision = EFI_ACPI_CREATOR_REVISION;
  
  Status = AcpiTable->InstallAcpiTable (AcpiTable, &Lpit, Lpit.Header.Length, &AcpiTableKey);  
  
  return Status;
}
