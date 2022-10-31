/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c) 2011 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  PchSpi.c

Abstract:

  PCH SPI PEIM implements the SPI Host Controller Compatibility Interface.

--*/
#include "PchSpi.h"

#ifdef ECP_FLAG
EFI_GUID gPeiSpiPpiGuid = PEI_SPI_PPI_GUID;
#else
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#endif

EFI_STATUS
EFIAPI
InstallPchSpi (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
#ifdef ECP_FLAG
  IN       EFI_PEI_SERVICES     **PeiServices
#else
  IN CONST EFI_PEI_SERVICES     **PeiServices
#endif
  )
/*++

Routine Description:

  Installs PCH SPI PPI

Arguments:

  FfsHeader    - Not used.
  PeiServices  - General purpose services available to every PEIM.

Returns:

  EFI_SUCCESS  - PCH SPI PPI is installed successfully
  EFI_OUT_OF_RESOURCES - Can't allocate pool

--*/
{
  EFI_STATUS        Status;
  PEI_SPI_INSTANCE  *PeiSpiInstance;
  SPI_INSTANCE      *SpiInstance;

  DEBUG ((EFI_D_INFO, "InstallPchSpi() Start\n"));

  PeiSpiInstance = (PEI_SPI_INSTANCE *) AllocateZeroPool (sizeof (PEI_SPI_INSTANCE));
  if (NULL == PeiSpiInstance) {
    return EFI_OUT_OF_RESOURCES;
  }

  SpiInstance = &(PeiSpiInstance->SpiInstance);
  SpiProtocolConstructor (SpiInstance);

  PeiSpiInstance->PpiDescriptor.Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
  PeiSpiInstance->PpiDescriptor.Guid  = &gPeiSpiPpiGuid;
  PeiSpiInstance->PpiDescriptor.Ppi   = &(SpiInstance->SpiProtocol);

  //
  // Install the SPI PPI
  //
  Status = (**PeiServices).InstallPpi (PeiServices, &PeiSpiInstance->PpiDescriptor);
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "SPI PPI Installed\n"));

  DEBUG ((EFI_D_INFO, "InstallPchSpi() End\n"));

  return Status;
}

VOID
EFIAPI
SpiPhaseInit (
  VOID
  )
/*++
Routine Description:

  This function is a a hook for Spi Pei phase specific initialization

Arguments:

  None

Returns:

  None

--*/
{
  UINT32 PciD31F0RegBase;
  UINT32 SpiBase;

  PciD31F0RegBase = MmPciAddress (0,
                      DEFAULT_PCI_BUS_NUMBER_PCH,
                      PCI_DEVICE_NUMBER_PCH_LPC,
                      PCI_FUNCTION_NUMBER_PCH_LPC,
                      0
                    );
  SpiBase = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_SPI_BASE) & B_PCH_LPC_SPI_BASE_BAR;
  //
  // Disable SMM BIOS write protect if it's not a SMM protocol
  //
  MmioAnd8 (SpiBase + R_PCH_SPI_BCR,
    (UINT8) (~B_PCH_SPI_BCR_SMM_BWP)
    );
}
