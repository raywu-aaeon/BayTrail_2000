/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2008 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  fTPMInit.c

Abstract:

  Framework PEIM to Init fTPM.

--*/

#include "SeCAccess.h"
#include "HeciRegs.h"

#include <Library/DebugLib.h>
#include <Library/PciLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Ppi/fTPM.h>

UINT32
SeCSendfTPMSize (
  IN EFI_PEI_SERVICES **PeiServices
  )
/*++

Routine Description:

  This procedure will read and return the amount of SeC UMA requested
  by SeC ROM from the HECI device.

Arguments:

  PeiServices   - General purpose services available to every PEIM.

Returns:

  Return SeC UMA Size

--*/
{

  UINT32        Ptt_Ctrl_Reg;

  //  1.    BIOS reads the SATT_PTT_CTRL.PTT_DISABLED bit and finds that PTT is not disabled.
  Ptt_Ctrl_Reg = HeciPciRead32 (R_SATT_PTT_CTRL);

  if((Ptt_Ctrl_Reg & B_PTT_DISABLED) == B_PTT_DISABLED) {
    DEBUG((EFI_D_ERROR, "fTPM disabled \n"));
    return 0;
  }

  //  2.    BIOS creates the ACPI namespace for the PTT and fills out the TPM2.0 namespace table defined by Microsoft.

  // fTPM required 4k but MRC allocate 1MB for it.
  return 1;

}


EFI_STATUS
SeCConfigfTPM (
  IN EFI_PEI_SERVICES **PeiServices,
  UINT8                 MrcBootMode,
  UINT8                 InitStat,
  UINT32                SeCfTPMBase,
  UINT32                SeCfTPMSize
  )
/*++

Routine Description:

  This procedure will configure the SEC Host General Status register,
  indicating that DRAM Initialization is complete and SeC FW may
  begin using the allocated SeC UMA space.

Arguments:

  PeiServices - General purpose services available to every PEIM.
  MrcBootMode - MRC BootMode
  InitStat    - H_GS[27:24] Status
  SeCUmaBase  - LSB of base address
  SeCUmaBaseEx - MSB of base address
  SeCUmaSIze -  Allocated size of UMA

Returns:
  EFI_SUCCESS

--*/
{
  UINT32        fTPMSize;
  UINT32        lfTPMBase;

  if (SeCfTPMSize == 0) {
    fTPMSize = 0;
    lfTPMBase = 0;
  } else {
    fTPMSize = SEC_PTT_SAP_SIZE;
    lfTPMBase = SeCfTPMBase << 20;
  }


  // 3.1     SATT_PTT_SAP_SIZE.
  HeciPciWrite32(R_SATT_PTT_SAP_SIZE, fTPMSize);

  // 3.2     SATT_PTT_BRG_BA_LSB.
  HeciPciWrite32 (R_SATT_PTT_BRG_BA_LSB, lfTPMBase);


  // 3.3     SATT_PTT_CTRL.BRG_BA_MSB.
  // bit 8:11 is MSB for IOSF address
  // Here we only use the memory below 4G.
  // So bit 8:11 must be 0
  HeciPciOr32 (R_SATT_PTT_CTRL, ~0xF00);

  if (SeCfTPMSize == 0) {
    HeciPciAnd32(R_SATT_PTT_CTRL, ~B_ENTRY_VLD);
    return EFI_SUCCESS;
  }
  // 3.4     SATT_PTT_CTRL.ENTRY_VLD.
  HeciPciOr32(R_SATT_PTT_CTRL, B_ENTRY_VLD);

  DEBUG((EFI_D_ERROR, "******  fTPM  ************ \n"));
  DEBUG((EFI_D_ERROR, "UMA SIZE = 0x%x B regsVal %x \n", HeciPciRead32 (R_SATT_PTT_SAP_SIZE), R_SATT_PTT_SAP_SIZE));
  DEBUG((EFI_D_ERROR, "UMA LSB = 0x%x  regsVal %x\n", HeciPciRead32 (R_SATT_PTT_BRG_BA_LSB), R_SATT_PTT_BRG_BA_LSB));
  DEBUG((EFI_D_ERROR, "SATT PTT CTRL = 0x%x  regsVal %x\n", HeciPciRead32(R_SATT_PTT_CTRL), R_SATT_PTT_CTRL));

  SetMem((VOID*)(UINTN)lfTPMBase, SEC_PTT_SAP_SIZE, 0);
  return EFI_SUCCESS;
}


static SEC_FTPM_PPI         mSeCfTPMPpi = {
  SeCSendfTPMSize,
  SeCConfigfTPM
};

static EFI_PEI_PPI_DESCRIPTOR mSeCfTPMPpiList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gSeCfTPMPpiGuid,
    &mSeCfTPMPpi
  }
};

EFI_STATUS
PeimfTPMInit (
  IN EFI_PEI_FILE_HANDLE       *FfsHeader,
  IN CONST EFI_PEI_SERVICES    **PeiServices
  )
/*++

Routine Description:

  Initialize SEC after reset

Arguments:

  FfsHeader   - Not used.
  PeiServices - General purpose services available to every PEIM.

Returns:

  EFI_SUCCESS:  Heci initialization completed successfully.
  All other error conditions encountered result in an ASSERT.

--*/
{
  EFI_STATUS                        Status;


  DEBUG ((EFI_D_ERROR, "fTPM Initialization Start.\n"));
  Status  = (*PeiServices)->InstallPpi (PeiServices, mSeCfTPMPpiList);

  DEBUG ((EFI_D_ERROR, "fTPM Initialization Complete.\n"));

  return Status;
}



