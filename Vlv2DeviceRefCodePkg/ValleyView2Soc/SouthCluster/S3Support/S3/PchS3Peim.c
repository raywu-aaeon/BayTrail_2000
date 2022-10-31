/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


  @file
  PchS3Peim.c

  @brief
  This is the PEIM that performs the S3 resume tasks.

--*/
#include "PchS3Peim.h"
#ifndef ECP_FLAG
#include <Base.h>
#endif

EFI_STATUS
EFIAPI
InitializePchS3Peim (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
#ifdef ECP_FLAG
  IN       EFI_PEI_SERVICES     **PeiServices
#else
  IN CONST EFI_PEI_SERVICES     **PeiServices
#endif
  )
/**

  @brief
  PCH S3 PEIM entry point

  @param[in] FfsHeader            Header for FFS
  @param[in] PeiServices          PEI Services table pointer

  @retval EFI_SUCCESS             Successfully completed

**/
{
  EFI_STATUS                        Status;
#ifdef ECP_FLAG
  PEI_READ_ONLY_VARIABLE_PPI        *PeiVar;
#else
  EFI_PEI_READ_ONLY_VARIABLE2_PPI    *PeiVar;
#endif
  UINTN                             VarSize;
  PCH_S3_PARAMETER_HEADER           *S3Parameter;
  EFI_BOOT_MODE                     BootMode;
  UINT32                            RootComplexBar;
  UINT32                            TypeSize;
  UINT32                            ParameterSize;
  UINT32                            Size;
  UINT8                             *CurrentPos;
  EFI_PCH_S3_DISPATCH_ITEM_TYPE     ItemType;
  UINT32                            HeaderSize;

  DEBUG ((EFI_D_INFO, "InitializePchS3Peim() Start\n"));

  ///
  /// If not in S3 boot path. do nothing
  ///
#ifndef ECP_FLAG
  PeiServices = GetPeiServicesTablePointer ();
#endif
  Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode);
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  if (BootMode != BOOT_ON_S3_RESUME) {
    return EFI_SUCCESS;
  }
  ///
  /// Get the Root Complex Bar
  ///
  RootComplexBar = MmioRead32 (
                     MmPciAddress (0,
                       DEFAULT_PCI_BUS_NUMBER_PCH,
                       PCI_DEVICE_NUMBER_PCH_LPC,
                       PCI_FUNCTION_NUMBER_PCH_LPC,
                       R_PCH_LPC_RCBA)
                   ) & B_PCH_LPC_RCBA_BAR;

  ///
  /// Get PCH S3 Parameters
  ///
  Status = (**PeiServices).LocatePpi (
                             PeiServices,
#ifdef ECP_FLAG
                             &gPeiReadOnlyVariablePpiGuid,
#else
                             &gEfiPeiReadOnlyVariable2PpiGuid,
#endif
                             0,
                             NULL,
                             (VOID **) &PeiVar
                             );
  ASSERT_EFI_ERROR (Status);
  VarSize = sizeof (UINT32);
#ifdef ECP_FLAG
  Status = PeiVar->PeiGetVariable (
                     PeiServices,
                     PCH_INIT_VARIABLE_NAME,
                     &gPchInitVariableGuid,
                     NULL,
                     &VarSize,
                     &S3Parameter
                     );
#else
  Status = PeiVar->GetVariable (
                     PeiVar,
                     PCH_INIT_VARIABLE_NAME,
                     &gPchInitVariableGuid,
                     NULL,
                     &VarSize,
                     &S3Parameter
                     );
#endif

  if (EFI_ERROR (Status)) {
    return Status;
  }
  ///
  /// Get the current Execute Position
  ///
  CurrentPos  = (UINT8 *) S3Parameter + S3Parameter->ExecutePosition;
  ItemType    = *(EFI_PCH_S3_DISPATCH_ITEM_TYPE *) CurrentPos;

  ///
  /// Round up TypeSize to be 8 byte aligned
  ///
  TypeSize  = sizeof (EFI_PCH_S3_DISPATCH_ITEM_TYPE);
  TypeSize  = (TypeSize + 7) / 8 * 8;
  CurrentPos += TypeSize;

  ///
  /// Calculate the size required;
  /// ** Always round up to be 8 byte aligned
  ///
  switch (ItemType) {
    case PchS3ItemTypeSendCodecCommand:
      ParameterSize = sizeof (EFI_PCH_S3_PARAMETER_SEND_CODEC_COMMAND);
      ParameterSize = (ParameterSize + 7) / 8 * 8;
      Status        = PchS3SendCodecCommand ((EFI_PCH_S3_PARAMETER_SEND_CODEC_COMMAND *) CurrentPos);
      PchPmTimerStall (60);
      break;
    case PchS3ItemTypePollStatus:

      ParameterSize = sizeof (EFI_PCH_S3_PARAMETER_POLL_STATUS);
      ParameterSize = (ParameterSize + 7) / 8 * 8;
      PchPmTimerStall (50);
      // Status        = CodecStatusPolling (S3ParameterPollStatus.MmioAddress, S3ParameterPollStatus, S3ParameterPollStatus.Value);
      break;
    case PchS3ItemTypeInitPcieRootPortDownstream:
      ParameterSize = sizeof (EFI_PCH_S3_PARAMETER_INIT_PCIE_ROOT_PORT_DOWNSTREAM);
      ParameterSize = (ParameterSize + 7) / 8 * 8;
      Status        = PchS3InitPcieRootPortDownstream ((EFI_PCH_S3_PARAMETER_INIT_PCIE_ROOT_PORT_DOWNSTREAM *) CurrentPos);
      if (Status == EFI_NOT_FOUND) {
        ///
        /// EFI_NOT_FOUND is not an error here
        ///
        Status = EFI_SUCCESS;
      }
      break;

    case PchS3ItemTypePcieSetPm:
      ParameterSize = sizeof (EFI_PCH_S3_PARAMETER_PCIE_SET_PM);
      ParameterSize = (ParameterSize + 7) / 8 * 8;
      Status        = PchS3PcieSetPm ((EFI_PCH_S3_PARAMETER_PCIE_SET_PM *) CurrentPos);
      if (Status == EFI_NOT_FOUND) {
        ///
        /// EFI_NOT_FOUND is not an error here
        ///
        Status = EFI_SUCCESS;
      }
      break;

    case PchS3ItemTypePmTimerStall:
      ParameterSize = sizeof (EFI_PCH_S3_PARAMETER_PM_TIMER_STALL);
      ParameterSize = (ParameterSize + 7) / 8 * 8;
      PchPmTimerStall (50);
      // Status        = CodecStatusPolling (S3ParameterPollStatus.MmioAddress, S3ParameterPollStatus, S3ParameterPollStatus.Value);
      break;


    default:
      ParameterSize = 0;
      ASSERT (FALSE);
      break;
  }
  ///
  /// Total size is TypeSize + ParameterSize
  ///
  Size = TypeSize + ParameterSize;

  ///
  /// Advance the Execution Position
  ///
  S3Parameter->ExecutePosition += Size;
  if (S3Parameter->ExecutePosition >= S3Parameter->StorePosition) {
    ///
    /// We are beyond end, wrap for the next S3 resume path
    ///
    HeaderSize                    = sizeof (PCH_S3_PARAMETER_HEADER);
    HeaderSize                    = (HeaderSize + 7) / 8 * 8;
    S3Parameter->ExecutePosition  = HeaderSize;
  }

  DEBUG ((EFI_D_INFO, "InitializePchS3Peim() End\n"));

  return Status;
}

#define AZALIA_MAX_LOOP_TIME  10
#define AZALIA_WAIT_PERIOD    100

static
EFI_STATUS
CodecStatusPolling (
  IN      UINT32          StatusReg,
  IN      UINT16          PollingBitMap,
  IN      UINT16          PollingData
  )
/**

  @brief
  Polling the Status bit

  @param[in] StatusReg            The regsiter address to read the status
  @param[in] PollingBitMap        The bit mapping for polling
  @param[in] PollingData          The Data for polling

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_TIMEOUT             Polling the bit map time out

**/
{
  UINT32  LoopTime;

  for (LoopTime = 0; LoopTime < AZALIA_MAX_LOOP_TIME; LoopTime++) {
    if ((MmioRead16 (StatusReg) & PollingBitMap) == PollingData) {
      break;
    } else {
      PchPmTimerStall (AZALIA_WAIT_PERIOD);
    }
  }

  if (LoopTime >= AZALIA_MAX_LOOP_TIME) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PchS3SendCodecCommand (
  EFI_PCH_S3_PARAMETER_SEND_CODEC_COMMAND      *Parameter
  )
/**

  @brief
  Send Codec command on S3 resume

  @param[in] Parameter            Parameters passed in from DXE

  @retval EFI_DEVICE_ERROR        Device status error, operation failed
  @retval EFI_SUCCESS             The function completed successfully

**/
{
  UINT32      HdaBar;
  UINT32      *CodecCommandData;
  EFI_STATUS  Status;

  HdaBar            = Parameter->HdaBar;
  CodecCommandData  = &Parameter->CodecCmdData;

  DEBUG ((EFI_D_INFO, "Going to SendCodecCommand: %08x! \n", *CodecCommandData));
  Status = CodecStatusPolling (HdaBar + R_HDA_ICS, (UINT16) B_HDA_ICS_ICB, (UINT16) 0);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "ICB bit is not zero before SendCodecCommand! \n"));
    return EFI_DEVICE_ERROR;
  }

  MmioWrite32 (HdaBar + R_HDA_IC, *CodecCommandData);
  MmioOr16 ((UINTN) (HdaBar + R_HDA_ICS), (UINT16) ((B_HDA_ICS_IRV | B_HDA_ICS_ICB)));

  Status = CodecStatusPolling (HdaBar + R_HDA_ICS, (UINT16) B_HDA_ICS_ICB, (UINT16) 0);
  if (EFI_ERROR (Status)) {
    MmioAnd16 ((UINTN) (HdaBar + R_HDA_ICS), (UINT16)~(B_HDA_ICS_ICB));
    DEBUG ((EFI_D_ERROR, "SendCodecCommand: SendCodecCommand:%08x fail! \n"));
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PchS3InitPcieRootPortDownstream (
  EFI_PCH_S3_PARAMETER_INIT_PCIE_ROOT_PORT_DOWNSTREAM     *Parameter
  )
/**

  @brief
  Perform Init Root Port Downstream devices on S3 resume

  @param[in] Parameter            Parameters passed in from DXE

  @retval EFI_STATUS

**/
{
  EFI_STATUS  Status;

  Status = PchPcieInitRootPortDownstreamDevices (
             Parameter->RootPortBus,
             Parameter->RootPortDevice,
             Parameter->RootPortFunc,
             Parameter->TempBusNumberMin,
             Parameter->TempBusNumberMax
             );
  ///
  /// Not checking the error status here - downstream device not present does not
  /// mean an error of this root port. Our return status of EFI_SUCCESS means this
  /// port is enabled and outer function depends on this return status to do
  /// subsequent initializations.
  ///
  return Status;
}

EFI_STATUS
PchS3PcieSetPm (
  EFI_PCH_S3_PARAMETER_PCIE_SET_PM    *Parameter
  )
/**

  @brief
  Perform Root Port Downstream devices PCIE ASPM on S3 resume

  @param[in] Parameter            Parameters passed in from DXE

  @retval EFI_STATUS

**/
{
  EFI_STATUS                    Status;
  PCH_PCIE_DEVICE_ASPM_OVERRIDE *DevAspmOverride;
  PCH_PCIE_DEVICE_LTR_OVERRIDE  *DevLtrOverride;

  DevAspmOverride = (PCH_PCIE_DEVICE_ASPM_OVERRIDE *) (UINTN) Parameter->DevAspmOverrideAddr;
  DevLtrOverride  = (PCH_PCIE_DEVICE_LTR_OVERRIDE *) (UINTN) Parameter->DevLtrOverrideAddr;

  Status = PcieSetPm (
             Parameter->RootPortBus,
             Parameter->RootPortDevice,
             Parameter->RootPortFunc,
             Parameter->RootPortAspm,
             Parameter->NumOfDevAspmOverride,
             DevAspmOverride,
             Parameter->TempBusNumberMin,
             Parameter->TempBusNumberMax,
             Parameter->NumOfDevLtrOverride,
             DevLtrOverride
             );
  ///
  /// Not checking the error status here - downstream device not present does not
  /// mean an error of this root port. Our return status of EFI_SUCCESS means this
  /// port is enabled and outer function depends on this return status to do
  /// subsequent initializations.
  ///
  return Status;
}
