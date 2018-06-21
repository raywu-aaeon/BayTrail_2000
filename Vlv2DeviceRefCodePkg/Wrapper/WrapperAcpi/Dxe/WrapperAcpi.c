
//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2017, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
//
// $Header:  $
//
// $Revision:  $
//
// $Date:  $
//
//*****************************************************************************


//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        WrapperAcpi.c
//
// Description: This file contains code for Template Southbridge initialization
//              in the DXE stage
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

// Module specific Includes
#include <Library/UefiLib.h>
#include <Protocol/AcpiSupport.h>
#include <IndustryStandard/Acpi50.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <WrapperCSRT.h>

// Constant definitions
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//          Variable Declaration
//----------------------------------------------------------------------------
// Variable Declaration(s)

// GUID Definitions

//----------------------------------------------------------------------------

// Function Prototypes

/**
  Entry Point for this driver.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
WrapperAcpiDriverEntry (
  IN  EFI_HANDLE        ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
)
{
  EFI_STATUS                                Status;
  EFI_ACPI_SUPPORT_PROTOCOL                 *mAcpiSupport = NULL;
  UINTN                                     AcpiTableKey = 0;
  EFI_ACPI_TABLE_VERSION                    Version;

  // By default, a table belongs in all ACPI table versions published.
  Version = EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0 | EFI_ACPI_TABLE_VERSION_3_0;

  Status = gBS->LocateProtocol(&gEfiAcpiSupportProtocolGuid, NULL, &mAcpiSupport);
  if(EFI_ERROR(Status)) {
    DEBUG ((EFI_D_INFO, "Can't locate EfiAcpiSupportProtocol\n"));
  }   
  
  
  AcpiTableKey = 0;
  Status = mAcpiSupport->SetAcpiTable (mAcpiSupport, &Csrt, TRUE, Version, &AcpiTableKey);
  DEBUG ((EFI_D_ERROR, "Install CSRT ACPITABLE = %r\n", Status));

  return Status;
}
