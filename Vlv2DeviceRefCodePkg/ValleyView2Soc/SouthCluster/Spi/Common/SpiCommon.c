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

  SpiCommon.c

Abstract:

  VLV SPI Common Driver implements the SPI Host Controller Compatibility Interface.

--*/
#include "PchSpi.h"
#ifndef ECP_FLAG
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#endif

EFI_STATUS
SpiProtocolConstructor (
  SPI_INSTANCE          *SpiInstance
)
/*++

Routine Description:

  Initialize an SPI protocol instance.
  The function will assert in debug if SPI base address has not been initialized

Arguments:

  SpiInstance   - Pointer to SpiInstance to initialize

Returns:

  EFI_SUCCESS     The protocol instance was properly initialized
  EFI_UNSUPPORTED The PCH is not supported by this module

--*/
{
  UINT32 SpiBar;

  //
  // Check if the current VLV SC is known and supported by this code
  //
  if (!IsPchSupported ()) {
    DEBUG ((EFI_D_ERROR, "VLV SPI Protocol not supported due to no proper VLV PCU found!\n"));
    return EFI_UNSUPPORTED;
  }
  //
  // Initialize the SPI protocol instance
  //
  SpiInstance->Signature            = PCH_SPI_PRIVATE_DATA_SIGNATURE;
  SpiInstance->Handle               = NULL;
  SpiInstance->SpiProtocol.Init     = SpiProtocolInit;
  SpiInstance->SpiProtocol.Lock     = SpiProtocolLock;
  SpiInstance->SpiProtocol.Execute  = SpiProtocolExecute;

  SpiBar                            = MmioRead32 (
                                        MmPciAddress (
                                          0,
                                          DEFAULT_PCI_BUS_NUMBER_PCH,
                                          PCI_DEVICE_NUMBER_PCH_LPC,
                                          PCI_FUNCTION_NUMBER_PCH_LPC,
                                          R_PCH_LPC_SPI_BASE
                                          )
                                        );
  SpiInstance->SpiBase              = SpiBar & B_PCH_LPC_SPI_BASE_BAR;
  //
  // Let's perform a check if SpiBase is not programmed or is disabled.
  //
  ASSERT ((SpiInstance->SpiBase != 0) && (SpiInstance->SpiBase != B_PCH_LPC_SPI_BASE_BAR));

  //
  // Let's perform another check to see if SpiBase is enabled.
  //
  ASSERT ((SpiBar & B_PCH_LPC_SPI_BASE_EN) == B_PCH_LPC_SPI_BASE_EN);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GetDescriptorVsccValues (
  IN      EFI_SPI_PROTOCOL      *This,
  IN      UINT8                 ReadDataCmdOpcodeIndex,
  IN OUT  UINT32                *LowerVsccValue,
  IN OUT  UINT32                *UpperVsccValue
)
/*++

Routine Description:

  Get VSCC values from the Descriptor Region (VSCC Table).

Arguments:

  This                      A pointer to "EFI_SPI_PROTOCOL" for issuing commands
  ReadDataCmdOpcodeIndex    The index of the opcode - "PCH_SPI_COMMAND_READ_DATA"
  LowerVsccValue            Lower VSCC (Vendor Specific Component Capabilities) Value
  UpperVsccValue            Upper VSCC (Vendor Specific Component Capabilities) Value

Returns:

  EFI_SUCCESS               Found the VSCC values on Descriptor Region
  EFI_NOT_FOUND             Couldn't find the VSCC values on Descriptor Region
  EFI_UNSUPPORTED           ReadDataCmdOpcodeIndex is out of range

--*/
{
  UINT32        SpiDescFlashUpperMap1;
  UINT32        VsccTableBaseAddr;
  UINT32        VsccTableLength;
  UINT32        JedecIdRegIndex;
  EFI_STATUS    Status;
  UINT32        FlashDescriptor;
  SPI_INSTANCE  *SpiInstance;
  BOOLEAN       MatchedVtbEntryFound;

  if (ReadDataCmdOpcodeIndex >= SPI_NUM_OPCODE) {
    return EFI_UNSUPPORTED;
  }

  SpiInstance = SPI_INSTANCE_FROM_SPIPROTOCOL (This);

  Status = SpiProtocolExecute (
             This,
             ReadDataCmdOpcodeIndex,
             0,
             TRUE,
             TRUE,
             FALSE,
             (UINTN) R_PCH_SPI_FLASH_UMAP1,
             sizeof (SpiDescFlashUpperMap1),
             (UINT8 *) &SpiDescFlashUpperMap1,
             EnumSpiRegionDescriptor
             );
  if ((EFI_ERROR (Status)) || (SpiDescFlashUpperMap1 == 0xFFFFFFFF)) {
    return EFI_NOT_FOUND;
  }
  //
  // B_PCH_SPI_FLASH_UMAP1_VTBA represents address bits [11:4]
  //
  VsccTableBaseAddr = ((SpiDescFlashUpperMap1 & B_PCH_SPI_FLASH_UMAP1_VTBA) << 4);
  //
  // Multiplied by 4? B_PCH_SPI_FDBAR_VTL is the 1-based number of DWORDs.
  //
  VsccTableLength = (((SpiDescFlashUpperMap1 & B_PCH_SPI_FLASH_UMAP1_VTL) >> 8) << 2);
  if (VsccTableLength < SIZE_OF_SPI_VTBA_ENTRY) {
    //
    // Non-existent or invalid Vscc Table
    //
    return EFI_NOT_FOUND;
  }

  JedecIdRegIndex       = 0;
  MatchedVtbEntryFound  = FALSE;
  while (JedecIdRegIndex <= (VsccTableLength - SIZE_OF_SPI_VTBA_ENTRY)) {
    Status = SpiProtocolExecute (
               This,
               ReadDataCmdOpcodeIndex,
               0,
               TRUE,
               TRUE,
               FALSE,
               (UINTN) (VsccTableBaseAddr + JedecIdRegIndex),
               sizeof (UINT32),
               (UINT8 *) &FlashDescriptor,
               EnumSpiRegionDescriptor
               );

    if ((EFI_ERROR (Status)) || (FlashDescriptor == 0xFFFFFFFF)) {
      break;
    }

    if (((FlashDescriptor & B_PCH_SPI_VTBA_JID0_VID) != SpiInstance->SpiInitTable.VendorId) ||
        (((FlashDescriptor & B_PCH_SPI_VTBA_JID0_DID0) >> N_PCH_SPI_VTBA_JID0_DID0)
         != SpiInstance->SpiInitTable.DeviceId0) ||
        (((FlashDescriptor & B_PCH_SPI_VTBA_JID0_DID1) >> N_PCH_SPI_VTBA_JID0_DID1)
         != SpiInstance->SpiInitTable.DeviceId1)) {
      JedecIdRegIndex += SIZE_OF_SPI_VTBA_ENTRY;
    } else {
      MatchedVtbEntryFound = TRUE;
      break;
    }
  }

  if (!MatchedVtbEntryFound) {
    return EFI_NOT_FOUND;
  }

  Status = SpiProtocolExecute (
             This,
             ReadDataCmdOpcodeIndex,
             0,
             TRUE,
             TRUE,
             FALSE,
             (UINTN) (VsccTableBaseAddr + JedecIdRegIndex + R_PCH_SPI_VTBA_VSCC0),
             sizeof (UINT32),
             (UINT8 *) LowerVsccValue,
             EnumSpiRegionDescriptor
             );
  if ((EFI_ERROR (Status)) || (*LowerVsccValue == 0xFFFFFFFF)) {
    return EFI_NOT_FOUND;
  }
  //
  // Extract the upper and lower values from the VSCC entry
  //
  *UpperVsccValue = *LowerVsccValue;
  *UpperVsccValue = (*UpperVsccValue & B_PCH_SPI_VTBA_VSCC0_UCAPS) >> 16;
  *LowerVsccValue = (*LowerVsccValue & B_PCH_SPI_VTBA_VSCC0_LCAPS);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpiDiscoveryParameters (
  IN      EFI_SPI_PROTOCOL      *This,
  IN      UINT8                 SFDPCmdOpcodeIndex,
  IN OUT  UINT32                *UpperVsccValue
  )
/*++

Routine Description:

  Get UVSCC values from the Flash Basics Address with SFDP opode.

Arguments:

  This                      A pointer to "EFI_SPI_PROTOCOL" for issuing commands
  SFDPCmdOpcodeIndex        The index of the SFDP opcode
  UpperVsccValue            Upper VSCC (Vendor Specific Component Capabilities) Value

Returns:

  EFI_SUCCESS               The flash part is capable of supporting the dual output fast read command
  EFI_UNSUPPORTED           The flash part hasn't supported

--*/
{
  EFI_STATUS    Status;
  UINT8         SerialFlashDiscovery[0x100];
  UINT32        ParameterID0Addr;

  if (SFDPCmdOpcodeIndex >= SPI_NUM_OPCODE) {
    return EFI_UNSUPPORTED;
  }
  //
  // Send Serial Flash Discovery Parameters command
  //
  Status = SpiProtocolExecute (
             This,
             SFDPCmdOpcodeIndex,
             0,
             TRUE,
             TRUE,
             FALSE,
             (UINTN) 0,
             sizeof (SerialFlashDiscovery),
             (UINT8 *) SerialFlashDiscovery,
             EnumSpiRegionDescriptor
             );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }
  //
  // The 1st eight bits must be ignored
  // Serial Flash Discoverable Parameters (SFDP) Signature = 50444653h
  // Byte 0 = "S", Byte 1 = "F", Byte 2 = "D", Byte 3 = "P"
  //
  if (*(UINT32 *) &SerialFlashDiscovery[1] != 0x50444653) {
    return EFI_UNSUPPORTED;
  }
  //
  // SFDP opcode at address 05h(SFPD Major Revisions) must = 0001h
  //
  if (SerialFlashDiscovery[0x6] != 0x1) {
    return EFI_UNSUPPORTED;
  }
  //
  // SFDP opcode at address 0Ah(Serial Flash Basic Major Revisions) must = 0001h
  //
  if (SerialFlashDiscovery[0xB] != 0x1) {
    return EFI_UNSUPPORTED;
  }
  //
  // SFDP opcode at address Ch bits 23:00 = Parameter ID 0 table Address
  //
  ParameterID0Addr = (*(UINT32 *) &SerialFlashDiscovery[0xD] & 0x00FFFFFF);
  //
  // Make sure the Parameter ID 0 table is visible
  //
  if ((ParameterID0Addr + 0x10) > sizeof (SerialFlashDiscovery)) {
    return EFI_UNSUPPORTED;
  }
  //
  // SFDP opcode at address Parameter ID 0 table bit 16 will confirm that Single input address Dual
  // Output Fast read is supported.
  //
  if ((*(UINT32 *) &SerialFlashDiscovery[ParameterID0Addr + 1] & BIT16) == 0) {
    return EFI_UNSUPPORTED;
  }
  //
  // Parameter ID 0 table bits 15:00 should be programmed into UVSCC register.
  //
  *UpperVsccValue = *(UINT16 *) &SerialFlashDiscovery[ParameterID0Addr + 1];

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
UnlockFlashComponents (
  IN      EFI_SPI_PROTOCOL      *This,
  IN      UINT8                 UnlockCmdOpcodeIndex
)
/*++

Routine Description:

  Issue unlock command to disable block protection, this only needs to be done once per SPI power on

Arguments:

  This                      A pointer to "EFI_SPI_PROTOCOL" for issuing commands
  UnlockCmdOpcodeIndex      The index of the Unlock command

Returns:

  EFI_SUCCESS               UnLock operation succeed.
  EFI_DEVICE_ERROR          Device error, operation failed.

--*/
{
  EFI_STATUS    Status;
  SPI_INSTANCE  *SpiInstance;
  UINT8         SpiStatus;
  UINT32        Data32;
  UINTN         SpiBase;

  if (UnlockCmdOpcodeIndex >= SPI_NUM_OPCODE) {
    return EFI_UNSUPPORTED;
  }

  SpiInstance = SPI_INSTANCE_FROM_SPIPROTOCOL (This);
  SpiBase     = SpiInstance->SpiBase;

  //
  // Issue unlock command to disable block protection, this only needs to be done once per SPI power on
  //
  SpiStatus = 0;
  //
  // Issue unlock command to the flash component 1 at first
  //
  Status = SpiProtocolExecute (
             This,
             UnlockCmdOpcodeIndex,
             SpiInstance->SpiInitTable.PrefixOpcode[0] == PCH_SPI_COMMAND_WRITE_ENABLE ? 0 : 1,
             TRUE,
             TRUE,
             TRUE,
             (UINTN) 0,
             sizeof (SpiStatus),
             &SpiStatus,
             EnumSpiRegionAll
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Unlock flash component 1 fail!\n"));
    return Status;
  }

  if (PchIsSpiDescriptorMode (SpiBase)) {
    //
    // Select to Flash Map 0 Register to get the number of flash Component
    //
    MmioAndThenOr32 (
      SpiBase + R_PCH_SPI_FDOC,
      (UINT32) (~(B_PCH_SPI_FDOC_FDSS_MASK | B_PCH_SPI_FDOC_FDSI_MASK)),
      (UINT32) (V_PCH_SPI_FDOC_FDSS_FSDM | R_PCH_SPI_FDBAR_FLASH_MAP0)
      );
    //
    // If the number of flash Component is 2, then check the flash component 1 size
    //
    Data32 = MmioRead32 (SpiBase + R_PCH_SPI_FDOD);
    if ((Data32 & B_PCH_SPI_FDBAR_NC) == V_PCH_SPI_FDBAR_NC_2) {
      MmioAndThenOr32 (
        SpiBase + R_PCH_SPI_FDOC,
        (UINT32) (~(B_PCH_SPI_FDOC_FDSS_MASK | B_PCH_SPI_FDOC_FDSI_MASK)),
        (UINT32) (V_PCH_SPI_FDOC_FDSS_COMP | R_PCH_SPI_FCBA_FLCOMP)
        );
      Data32 = MmioRead32 (SpiBase + R_PCH_SPI_FDOD);
      switch (Data32 & B_PCH_SPI_FLCOMP_COMP1_MASK) {
        case V_PCH_SPI_FLCOMP_COMP1_512KB:
          Data32 = 0x80000;
          break;

        case V_PCH_SPI_FLCOMP_COMP1_1MB:
          Data32 = 0x100000;
          break;

        case V_PCH_SPI_FLCOMP_COMP1_2MB:
          Data32 = 0x200000;
          break;

        case V_PCH_SPI_FLCOMP_COMP1_4MB:
          Data32 = 0x400000;
          break;

        case V_PCH_SPI_FLCOMP_COMP1_8MB:
          Data32 = 0x800000;
          break;

        case V_PCH_SPI_FLCOMP_COMP1_16MB:
          Data32 = 0x1000000;
          break;
      }
      //
      // The secondary SPI's address is equal to the first SPI's size, issue unlock to the secondary SPI
      //
      Status = SpiProtocolExecute (
                 This,
                 UnlockCmdOpcodeIndex,
                 SpiInstance->SpiInitTable.PrefixOpcode[0] == PCH_SPI_COMMAND_WRITE_ENABLE ? 0 : 1,
                 TRUE,
                 TRUE,
                 TRUE,
                 (UINTN) Data32,
                 sizeof (SpiStatus),
                 &SpiStatus,
                 EnumSpiRegionAll
                 );
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "Unlock flash component 2 fail!\n"));
        return Status;
      }
    }
  }
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpiProtocolInit (
  IN EFI_SPI_PROTOCOL       *This,
  IN SPI_INIT_TABLE         *InitTable
)
/*++

Routine Description:

  Initialize the host controller to execute SPI command.

Arguments:

  This                    Pointer to the EFI_SPI_PROTOCOL instance.
  InitTable               Initialization data to be programmed into the SPI host controller.

Returns:

  EFI_SUCCESS             Initialization completed.
  EFI_ACCESS_DENIED       The SPI static configuration interface has been locked-down.
  EFI_INVALID_PARAMETER   Bad input parameters.
  EFI_UNSUPPORTED         Can't get Descriptor mode VSCC values
--*/
{
  EFI_STATUS    Status;
  UINT8         Index;
  UINT16        OpcodeType;
  SPI_INSTANCE  *SpiInstance;
  UINT32        LowerVsccValue;
  UINT32        UpperVsccValue;
  BOOLEAN       MultiPartitionIsSupported;
  UINTN         SpiBase;
  UINTN         FlashPartitionBoundaryAddr;
  UINT8         SFDPCmdOpcodeIndex;
  UINT8         UnlockCmdOpcodeIndex;
  UINT8         ReadDataCmdOpcodeIndex;
  UINT8         FlashPartId[3];

  SpiInstance = SPI_INSTANCE_FROM_SPIPROTOCOL (This);
  SpiBase     = SpiInstance->SpiBase;

  if (InitTable != NULL) {
    //
    // Copy table into SPI driver Private data structure
    //
    CopyMem (
      &SpiInstance->SpiInitTable,
      InitTable,
      sizeof (SPI_INIT_TABLE)
      );
  } else {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Check if the SPI interface has been locked-down.
  //
  if ((MmioRead16 (SpiBase + R_PCH_SPI_HSFS) & B_PCH_SPI_HSFS_FLOCKDN) != 0) {
    ASSERT_EFI_ERROR (EFI_ACCESS_DENIED);
    return EFI_ACCESS_DENIED;
  }
  //
  // Clear all the status bits for hardware registers.
  //
  MmioOr16 (
    (UINTN) (SpiBase + R_PCH_SPI_HSFS),
    (UINT16) ((B_PCH_SPI_HSFS_AEL | B_PCH_SPI_HSFS_FCERR | B_PCH_SPI_HSFS_FDONE))
    );
  MmioRead16 (SpiBase + R_PCH_SPI_HSFS);

  //
  // Clear all the status bits for software registers.
  //
  MmioOr8 (
    (UINTN) (SpiBase + R_PCH_SPI_SSFCS),
    (UINT8) ((B_PCH_SPI_SSFCS_FCERR | B_PCH_SPI_SSFCS_CDS))
    );
  MmioRead8 (SpiBase + R_PCH_SPI_SSFCS);

  //
  // Set the Prefix Opcode registers.
  //
  MmioWrite16 (
    SpiBase + R_PCH_SPI_PREOP,
    (SpiInstance->SpiInitTable.PrefixOpcode[1] << 8) | InitTable->PrefixOpcode[0]
    );
  MmioRead16 (SpiBase + R_PCH_SPI_PREOP);

  //
  // Set Opcode Type Configuration registers.
  //
  for (Index = 0, OpcodeType = 0; Index < SPI_NUM_OPCODE; Index++) {
    switch (SpiInstance->SpiInitTable.OpcodeMenu[Index].Type) {
      case EnumSpiOpcodeRead:
        OpcodeType |= (UINT16) (V_PCH_SPI_OPTYPE_RDADDR << (Index * 2));
        break;
      case EnumSpiOpcodeWrite:
        OpcodeType |= (UINT16) (V_PCH_SPI_OPTYPE_WRADDR << (Index * 2));
        break;
      case EnumSpiOpcodeWriteNoAddr:
        OpcodeType |= (UINT16) (V_PCH_SPI_OPTYPE_WRNOADDR << (Index * 2));
        break;
      default:
        OpcodeType |= (UINT16) (V_PCH_SPI_OPTYPE_RDNOADDR << (Index * 2));
        break;
    }
  }
  MmioWrite16 (SpiBase + R_PCH_SPI_OPTYPE, OpcodeType);
  MmioRead16 (SpiBase + R_PCH_SPI_OPTYPE);

  //
  // Setup the Opcode Menu registers.
  //
  ReadDataCmdOpcodeIndex = SPI_NUM_OPCODE;
  SFDPCmdOpcodeIndex = SPI_NUM_OPCODE;
  UnlockCmdOpcodeIndex = SPI_NUM_OPCODE;
  for (Index = 0; Index < SPI_NUM_OPCODE; Index++) {
    MmioWrite8 (
      SpiBase + R_PCH_SPI_OPMENU0 + Index,
      SpiInstance->SpiInitTable.OpcodeMenu[Index].Code
      );
    MmioRead8 (SpiBase + R_PCH_SPI_OPMENU0 + Index);
    if (SpiInstance->SpiInitTable.OpcodeMenu[Index].Operation == EnumSpiOperationJedecId) {
      Status = SpiProtocolExecute (
                 This,
                 Index,
                 0,
                 TRUE,
                 TRUE,
                 FALSE,
                 (UINTN) 0,
                 3,
                 FlashPartId,
                 EnumSpiRegionDescriptor
                 );
      if (EFI_ERROR (Status)) {
        return Status;
      }
      if (FlashPartId[0] != SpiInstance->SpiInitTable.VendorId  ||
          FlashPartId[1] != SpiInstance->SpiInitTable.DeviceId0 ||
          FlashPartId[2] != SpiInstance->SpiInitTable.DeviceId1) {
        return EFI_INVALID_PARAMETER;
      }
    }

    if (SpiInstance->SpiInitTable.OpcodeMenu[Index].Operation == EnumSpiOperationReadData ||
        SpiInstance->SpiInitTable.OpcodeMenu[Index].Operation == EnumSpiOperationFastRead ||
        SpiInstance->SpiInitTable.OpcodeMenu[Index].Operation == EnumSpiOperationDualOutputFastRead) {
      ReadDataCmdOpcodeIndex = Index;
    }

    if (SpiInstance->SpiInitTable.OpcodeMenu[Index].Operation == EnumSpiOperationDiscoveryParameters) {
      SFDPCmdOpcodeIndex = Index;
    }

    if (SpiInstance->SpiInitTable.OpcodeMenu[Index].Operation == EnumSpiOperationWriteStatus) {
      UnlockCmdOpcodeIndex = Index;
    }
  }
  //
  // Initialize the flash partition boundary address and the flag of multi-partition indication.
  //
  FlashPartitionBoundaryAddr = (UINTN) ((MmioRead32 (SpiBase + R_PCH_SPI_FPB) & B_PCH_SPI_FPB_FPBA_MASK) << 12);
  if ((FlashPartitionBoundaryAddr != 0) && (FlashPartitionBoundaryAddr != (B_PCH_SPI_FPB_FPBA_MASK << 12))) {
    MultiPartitionIsSupported = TRUE;
  } else {
    MmioWrite32 (SpiBase + R_PCH_SPI_FPB, 0);
    MultiPartitionIsSupported = FALSE;
  }
  //
  // Setup the SPI flash VSCC registers if it's Descriptor Mode.
  //
  if (PchIsSpiDescriptorMode (SpiBase)) {
    Status = GetDescriptorVsccValues (
               This,
               ReadDataCmdOpcodeIndex,
               &LowerVsccValue,
               &UpperVsccValue
               );
    //
    // No valid VSCC value in descriptor, get it with Discovery Parameters Opcode
    //
    if (EFI_ERROR (Status)) {
      if (!MultiPartitionIsSupported) {
        Status = SpiDiscoveryParameters (This, SFDPCmdOpcodeIndex, &UpperVsccValue);
      } else {
        Status = EFI_UNSUPPORTED;
      }
      if (EFI_ERROR (Status)) {
        //
        // Get UVSCC value with Discovery Parameters Opcode fail,
        // Program the VSCC registers by getting the data from SpiInitTable
        //
        for (Index = 0; Index < SPI_NUM_OPCODE; Index++) {
          //
          // For every Valleyview based platform that supports SeC, only 4 KB erase is supported
          // Get the opcode from SpiInitTable if the operation is 4 KB erase
          //
          if (SpiInstance->SpiInitTable.OpcodeMenu[Index].Operation == EnumSpiOperationErase_4K_Byte) {
            UpperVsccValue = UpperVsccValue | (UINT32) (V_PCH_SPI_LVSCC_BSES_4K);
            UpperVsccValue = UpperVsccValue | (UINT32) (SpiInstance->SpiInitTable.OpcodeMenu[Index].Code << 8);
          } else if (SpiInstance->SpiInitTable.OpcodeMenu[Index].Operation == EnumSpiOperationProgramData_64_Byte) {
            UpperVsccValue = UpperVsccValue | (UINT32) (B_PCH_SPI_LVSCC_WG_64B);
          }
        }
        //
        // Bit WSR and WEWS should NOT be both set to 1, so we check if there is any "Write enable on Write status" prefix opcode
        // from SpiInitTable at first, then check "Write Status Enable" prefix opcode
        //
        if ((SpiInstance->SpiInitTable.PrefixOpcode[0] == PCH_SPI_COMMAND_WRITE_ENABLE) ||
            (SpiInstance->SpiInitTable.PrefixOpcode[1] == PCH_SPI_COMMAND_WRITE_ENABLE)) {
          UpperVsccValue = UpperVsccValue | (UINT32) (B_PCH_SPI_LVSCC_WEWS);
        } else if ((SpiInstance->SpiInitTable.PrefixOpcode[0] == PCH_SPI_COMMAND_WRITE_STATUS_EN) ||
                   (SpiInstance->SpiInitTable.PrefixOpcode[1] == PCH_SPI_COMMAND_WRITE_STATUS_EN)) {
          UpperVsccValue = UpperVsccValue | (UINT32) (B_PCH_SPI_LVSCC_WSR);
        }

        LowerVsccValue = UpperVsccValue;
      }
    }
    //
    // The VCL locks itself when set, it will assert because we have no way to update VSCC value
    //
    if ((MmioRead32 ((UINTN) (SpiBase + R_PCH_SPI_LVSCC)) & B_PCH_SPI_LVSCC_VCL) != 0) {
      ASSERT_EFI_ERROR (EFI_ACCESS_DENIED);
      return EFI_ACCESS_DENIED;
    }

    ASSERT (UpperVsccValue != 0);
    MmioWrite32 ((UINTN) (SpiBase + R_PCH_SPI_UVSCC), UpperVsccValue);
    if (MultiPartitionIsSupported) {
      MmioWrite32 ((UINTN) (SpiBase + R_PCH_SPI_LVSCC), LowerVsccValue);
    }
  }

  Status = UnlockFlashComponents (
             This,
             UnlockCmdOpcodeIndex
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Unlock flash components fail!\n"));
  }

  // PnP settings
  MmioAndThenOr8 (SpiBase + R_PCH_SPI_BCR, (UINT8)(~B_PCH_SPI_BCR_SRC), (UINT8)(V_PCH_SPI_BCR_SRC_PREF_DIS_CACHE_EN));
  MmioAnd16 (SpiBase + R_PCH_SPI_TCGC, (UINT16)(~B_PCH_SPI_TCGC_FCGDIS));

  SpiPhaseInit ();

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpiProtocolLock (
  IN EFI_SPI_PROTOCOL     *This
)
/*++

Routine Description:

  Lock the SPI Static Configuration Interface.
  Once locked, the interface can not be changed and can only be clear by system reset.

Arguments:

  This      Pointer to the EFI_SPI_PROTOCOL instance.

Returns:

  EFI_SUCCESS             Lock operation succeed.
  EFI_DEVICE_ERROR        Device error, operation failed.
  EFI_ACCESS_DENIED       The interface has already been locked.

--*/
{
  SPI_INSTANCE  *SpiInstance;
  UINTN         SpiBase;

  SpiInstance = SPI_INSTANCE_FROM_SPIPROTOCOL (This);
  SpiBase     = SpiInstance->SpiBase;

  //
  // Check if the interface has already been locked-down.
  //
  if ((MmioRead16 (SpiBase + R_PCH_SPI_HSFS) & B_PCH_SPI_HSFS_FLOCKDN) != 0) {
    return EFI_ACCESS_DENIED;
  }
  //
  // Lock-down the configuration interface.
  //
  MmioOr16 ((UINTN) (SpiBase + R_PCH_SPI_HSFS), (UINT16) (B_PCH_SPI_HSFS_FLOCKDN));
  MmioRead16 (SpiBase + R_PCH_SPI_HSFS);

  //
  // Verify if it's really locked.
  //
  if ((MmioRead16 (SpiBase + R_PCH_SPI_HSFS) & B_PCH_SPI_HSFS_FLOCKDN) == 0) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpiProtocolExecute (
  IN     EFI_SPI_PROTOCOL   *This,
  IN     UINT8              OpcodeIndex,
  IN     UINT8              PrefixOpcodeIndex,
  IN     BOOLEAN            DataCycle,
  IN     BOOLEAN            Atomic,
  IN     BOOLEAN            ShiftOut,
  IN     UINTN              Address,
  IN     UINT32             DataByteCount,
  IN OUT UINT8              *Buffer,
  IN     SPI_REGION_TYPE    SpiRegionType
  )
/*++

Routine Description:

  Execute SPI commands from the host controller.
  This function would be called by runtime driver, please do not use any MMIO marco here

Arguments:

  This              Pointer to the EFI_SPI_PROTOCOL instance.
  OpcodeIndex       Index of the command in the OpCode Menu.
  PrefixOpcodeIndex Index of the first command to run when in an atomic cycle sequence.
  DataCycle         TRUE if the SPI cycle contains data
  Atomic            TRUE if the SPI cycle is atomic and interleave cycles are not allowed.
  ShiftOut          If DataByteCount is not zero, TRUE to shift data out and FALSE to shift data in.
  Address           In Descriptor Mode, for Descriptor Region, GbE Region, ME Region and Platform
                    Region, this value specifies the offset from the Region Base; for BIOS Region,
                    this value specifies the offset from the start of the BIOS Image. In Non
                    Descriptor Mode, this value specifies the offset from the start of the BIOS Image.
                    Please note BIOS Image size may be smaller than BIOS Region size (in Descriptor
                    Mode) or the flash size (in Non Descriptor Mode), and in this case, BIOS Image is
                    supposed to be placed at the top end of the BIOS Region (in Descriptor Mode) or
                    the flash (in Non Descriptor Mode)
  DataByteCount     Number of bytes in the data portion of the SPI cycle. This function may break the
                    data transfer into multiple operations. This function ensures each operation does
                    not cross 256 byte flash address boundary.
                    *NOTE: if there is some SPI chip that has a stricter address boundary requirement
                    (e.g., its write page size is < 256 byte), then the caller cannot rely on this
                    function to cut the data transfer at proper address boundaries, and it's the
                    caller's responsibility to pass in a properly cut DataByteCount parameter.
  Buffer            Pointer to caller-allocated buffer containing the data received or sent during the
                    SPI cycle.
  SpiRegionType     SPI Region type. Values EnumSpiRegionBios, EnumSpiRegionGbE, EnumSpiRegionSeC,
                    EnumSpiRegionDescriptor, and EnumSpiRegionPlatformData are only applicable in
                    Descriptor mode. Value EnumSpiRegionAll is applicable to both Descriptor Mode
                    and Non Descriptor Mode, which indicates "SpiRegionOffset" is actually relative
                    to base of the 1st flash device (i.e., it is a Flash Linear Address).

Returns:

  EFI_SUCCESS             Command succeed.
  EFI_INVALID_PARAMETER   The parameters specified are not valid.
  EFI_UNSUPPORTED         Command not supported.
  EFI_DEVICE_ERROR        Device error, command aborts abnormally.

--*/
{
  EFI_STATUS  Status;
  UINT8       BiosCtlSave;
  UINT32      Data32;
  UINT32      AcpiBase;
  UINT32      SmiEnSave;
  SPI_INSTANCE  *SpiInstance;
  UINTN         SpiBase;

  SpiInstance     = SPI_INSTANCE_FROM_SPIPROTOCOL (This);
  SpiBase = SpiInstance->SpiBase;

  BiosCtlSave = 0;
  SmiEnSave   = 0;

  //
  // Check if the parameters are valid.
  //
  if ((OpcodeIndex >= SPI_NUM_OPCODE) || (PrefixOpcodeIndex >= SPI_NUM_PREFIX_OPCODE)) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Make sure it's safe to program the command.
  // Poll both Hardware Sequencing and Software Sequencing Status
  //
  if (!WaitForSpiCycleComplete (This, TRUE, FALSE)) {
    return EFI_DEVICE_ERROR;
  }

  if (!WaitForSpiCycleComplete (This, FALSE, FALSE)) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Acquire access to the SPI interface is not required any more.
  //
  //
  // Disable SMIs to make sure normal mode flash access is not interrupted by an SMI
  // whose SMI handler accesses flash (e.g. for error logging)
  //
  // *** NOTE: if the SMI_LOCK bit is set (i.e., B0:D31:F0:Offset A0h [4]='1'),
  // clearing B_GBL_SMI_EN will not have effect. In this situation, some other
  // synchronization methods must be applied either here or in the consumer of the
  // EFI_SPI_PROTOCOl.Execute(). An example method is disabling the specific SMI sources
  // whose SMI handlers access flash before calling Execute() and re-enabling the SMI
  // sources after the call.
  //
  AcpiBase = ReadPCIRegDWord(R_PCH_LPC_ACPI_BASE, DEFAULT_PCI_BUS_NUMBER_PCH, \
                             ((PCI_DEVICE_NUMBER_PCH_LPC << 3) + PCI_FUNCTION_NUMBER_PCH_LPC)) & 0xFF80;
  SmiEnSave = IoRead32 ((UINTN) (AcpiBase + R_PCH_SMI_EN));
  Data32    = SmiEnSave &~B_PCH_SMI_EN_GBL_SMI;
  IoWrite32 ((UINTN) (AcpiBase + R_PCH_SMI_EN), Data32);

  //
  // Enable flash writing
  //
  MmioOr8 (SpiBase + R_PCH_SPI_BCR,
           (UINT8) (B_PCH_SPI_BCR_BIOSWE)
            );

  //
  // If shifts the data out, disable Prefetching and Caching.
  //
  if (ShiftOut) {
    BiosCtlSave = MmioRead8 (SpiBase + R_PCH_SPI_BCR) & B_PCH_SPI_BCR_SRC;

    MmioAndThenOr8 (SpiBase + R_PCH_SPI_BCR,
      (UINT8) (~B_PCH_SPI_BCR_SRC),
      (UINT8) (V_PCH_SPI_BCR_SRC_PREF_DIS_CACHE_DIS)
      );
  }
  //
  // Sends the command to the SPI interface to execute.
  //
  Status = SendSpiCmd (
             This,
             OpcodeIndex,
             PrefixOpcodeIndex,
             DataCycle,
             Atomic,
             ShiftOut,
             Address,
             DataByteCount,
             Buffer,
             SpiRegionType
             );

  //
  // Disable flash writing
  //
  MmioAnd8 (SpiBase + R_PCH_SPI_BCR,
    (UINT8) (~B_PCH_SPI_BCR_BIOSWE)
    );

  //
  // Restore the settings for SPI Prefetching and Caching.
  //
  if (ShiftOut) {
    MmioAndThenOr8 (SpiBase + R_PCH_SPI_BCR,
      (UINT8) (~B_PCH_SPI_BCR_SRC),
      (UINT8) (BiosCtlSave)
      );
  }
  //
  // Restore SMIs.
  //
  IoWrite32 ((UINTN) (AcpiBase + R_PCH_SMI_EN), SmiEnSave);

  return Status;
}

UINT32
ReadPCIRegDWord (
  IN UINT8  RegNum,
  IN UINT8  BusNum,
  IN UINT8  DevFunc
)
/*++

Routine Description:

  Reads a double word value from the PCI address space

Arguments:

  RegNum   PCI Register number
  BusNum   PCI Bus number
  DevFunc  PCI Device and function number



Returns:

  Double word read

--*/
{
  UINT32  PciAddr;

  PciAddr = ((UINT32) (((UINT16) BusNum) << 8) + DevFunc) << 8;
  PciAddr += (RegNum & 0xfc);
  PciAddr |= 0x80000000;
  IoWrite32 (0xcf8, PciAddr);
  return IoRead32 (0xcfc + (RegNum & 0x3));
}

VOID
SpiOffset2Physical (
  IN      EFI_SPI_PROTOCOL  *This,
  IN      UINTN             SpiRegionOffset,
  IN      SPI_REGION_TYPE   SpiRegionType,
  OUT     UINTN             *HardwareSpiAddress,
  OUT     UINTN             *BaseAddress,
  OUT     UINTN             *LimitAddress
)
/*++

Routine Description:

  Convert SPI offset to Physical address of SPI hardware

Arguments:

  This               Pointer to the EFI_SPI_PROTOCOL instance.
  SpiRegionOffset    In Descriptor Mode, for Descriptor Region, GbE Region, ME Region and Platform
                     Region, this value specifies the offset from the Region Base; for BIOS Region,
                     this value specifies the offset from the start of the BIOS Image. In Non
                     Descriptor Mode, this value specifies the offset from the start of the BIOS Image.
                     Please note BIOS Image size may be smaller than BIOS Region size (in Descriptor
                     Mode) or the flash size (in Non Descriptor Mode), and in this case, BIOS Image is
                     supposed to be placed at the top end of the BIOS Region (in Descriptor Mode) or
                     the flash (in Non Descriptor Mode)
  BaseAddress        Base Address of the region.
  SpiRegionType      SPI Region type. Values EnumSpiRegionBios, EnumSpiRegionGbE, EnumSpiRegionSeC,
                     EnumSpiRegionDescriptor, and EnumSpiRegionPlatformData are only applicable in
                     Descriptor mode. Value EnumSpiRegionAll is applicable to both Descriptor Mode
                     and Non Descriptor Mode, which indicates "SpiRegionOffset" is actually relative
                     to base of the 1st flash device (i.e., it is a Flash Linear Address).
  HardwareSpiAddress Return absolution SPI address (i.e., Flash Linear Address)
  BaseAddress        Return base address of the region type
  LimitAddress       Return limit address of the region type

Returns:

  EFI_SUCCESS             Command succeed.

--*/
{
  SPI_INSTANCE  *SpiInstance;
  UINTN         SpiBase;

  SpiInstance = SPI_INSTANCE_FROM_SPIPROTOCOL (This);
  SpiBase     = SpiInstance->SpiBase;

  if (PchIsSpiDescriptorMode (SpiBase)) {
    switch (SpiRegionType) {

      case EnumSpiRegionBios:
        *LimitAddress = (((MmioRead32 (SpiBase + R_PCH_SPI_FREG1_BIOS)
                           & B_PCH_SPI_FREG1_LIMIT_MASK) >> 16) + 1) << 12;
        //
        // Adjust BaseAddress because of FTOOLC
        // FTOOLC will create a bigger region than BIOS actual size at the end of whole SPI
        // and Base Address reported by PCH will not the same with what BIOS assumes.
        // For example, FTOOLC create a 3MB size of 4MB for BIOS region and the Base Address
        // will be 0x100000, and BIOS will be placed at 0x300000 to 0x3fffff.
        // PCH_SPI_FREG1_BIOS will report BIOS address at 0x100000 and BIOS will fail to read
        // Variable from 0x100000.
        //
        // *BaseAddress = (MmioRead32 (SpiBase + PCH_SPI_FREG1_BIOS)
        //                & B_PCH_SPI_FREG1_BASE_MASK) << 12;
        //
        *BaseAddress = *LimitAddress - SpiInstance->SpiInitTable.BiosSize;
        break;


      case EnumSpiRegionSeC:
        *BaseAddress = (MmioRead32 (SpiBase + R_PCH_SPI_FREG2_SEC) & B_PCH_SPI_FREG2_BASE_MASK) << 12;
        *LimitAddress = (((MmioRead32 (SpiBase + R_PCH_SPI_FREG2_SEC)
                           & B_PCH_SPI_FREG2_LIMIT_MASK) >> 16) + 1) << 12;
        break;

      case EnumSpiRegionDescriptor:
        *BaseAddress = (MmioRead32 (SpiBase + R_PCH_SPI_FREG0_FLASHD) & B_PCH_SPI_FREG0_BASE_MASK) << 12;
        *LimitAddress = (((MmioRead32 (SpiBase + R_PCH_SPI_FREG0_FLASHD)
                           & B_PCH_SPI_FREG0_LIMIT_MASK) >> 16) + 1) << 12;
        break;

      case EnumSpiRegionPlatformData:
        *BaseAddress = (MmioRead32 (SpiBase + R_PCH_SPI_FREG4_PLATFORM_DATA) & B_PCH_SPI_FREG4_BASE_MASK) << 12;
        *LimitAddress = (((MmioRead32 (SpiBase + R_PCH_SPI_FREG4_PLATFORM_DATA)
                           & B_PCH_SPI_FREG4_LIMIT_MASK) >> 16) + 1) << 12;
        break;

      default:
        //
        // EnumSpiRegionAll indicates address is relative to flash device (i.e., address is Flash
        // Linear Address)
        //
        *BaseAddress  = 0;
        *LimitAddress = 0;
        break;
    }

    *HardwareSpiAddress = SpiRegionOffset +*BaseAddress;
  } else {
    if (SpiRegionType == EnumSpiRegionAll) {
      //
      // EnumSpiRegionAll indicates address is relative to flash device (i.e., address is Flash
      // Linear Address)
      //
      *HardwareSpiAddress = SpiRegionOffset;
    } else {
      //
      // Otherwise address is relative to BIOS image
      //
      *HardwareSpiAddress = SpiRegionOffset + SpiInstance->SpiInitTable.BiosStartOffset;
    }
  }
}

EFI_STATUS
SendSpiCmd (
  IN     EFI_SPI_PROTOCOL   *This,
  IN     UINT8              OpcodeIndex,
  IN     UINT8              PrefixOpcodeIndex,
  IN     BOOLEAN            DataCycle,
  IN     BOOLEAN            Atomic,
  IN     BOOLEAN            ShiftOut,
  IN     UINTN              Address,
  IN     UINT32             DataByteCount,
  IN OUT UINT8              *Buffer,
  IN     SPI_REGION_TYPE    SpiRegionType
)
/*++

Routine Description:

  This function sends the programmed SPI command to the slave device.

Arguments:

  OpcodeIndex       Index of the command in the OpCode Menu.
  PrefixOpcodeIndex Index of the first command to run when in an atomic cycle sequence.
  DataCycle         TRUE if the SPI cycle contains data
  Atomic            TRUE if the SPI cycle is atomic and interleave cycles are not allowed.
  ShiftOut          If DataByteCount is not zero, TRUE to shift data out and FALSE to shift data in.
  Address           In Descriptor Mode, for Descriptor Region, GbE Region, ME Region and Platform
                    Region, this value specifies the offset from the Region Base; for BIOS Region,
                    this value specifies the offset from the start of the BIOS Image. In Non
                    Descriptor Mode, this value specifies the offset from the start of the BIOS Image.
                    Please note BIOS Image size may be smaller than BIOS Region size (in Descriptor
                    Mode) or the flash size (in Non Descriptor Mode), and in this case, BIOS Image is
                    supposed to be placed at the top end of the BIOS Region (in Descriptor Mode) or
                    the flash (in Non Descriptor Mode)
  DataByteCount     Number of bytes in the data portion of the SPI cycle. This function may break the
                    data transfer into multiple operations. This function ensures each operation does
                    not cross 256 byte flash address boundary.
                    *NOTE: if there is some SPI chip that has a stricter address boundary requirement
                    (e.g., its write page size is < 256 byte), then the caller cannot rely on this
                    function to cut the data transfer at proper address boundaries, and it's the
                    caller's responsibility to pass in a properly cut DataByteCount parameter.
  Buffer            Data received or sent during the SPI cycle.
  SpiRegionType     SPI Region type. Values EnumSpiRegionBios, EnumSpiRegionGbE, EnumSpiRegionSeC,
                    EnumSpiRegionDescriptor, and EnumSpiRegionPlatformData are only applicable in
                    Descriptor mode. Value EnumSpiRegionAll is applicable to both Descriptor Mode
                    and Non Descriptor Mode, which indicates "SpiRegionOffset" is actually relative
                    to base of the 1st flash device (i.e., it is a Flash Linear Address).

Returns:

  EFI_SUCCESS             SPI command completes successfully.
  EFI_DEVICE_ERROR        Device error, the command aborts abnormally.
  EFI_ACCESS_DENIED       Some unrecognized command encountered in hardware sequencing mode
  EFI_INVALID_PARAMETER   The parameters specified are not valid.

--*/
{
  UINT32        Index;
  SPI_INSTANCE  *SpiInstance;
  UINTN         HardwareSpiAddr;
  UINTN         SpiBiosSize;
  BOOLEAN       UseSoftwareSequence;
  UINTN         BaseAddress;
  UINTN         LimitAddress;
  UINT32        SpiDataCount;
  UINT8         OpCode;
  SPI_OPERATION Operation;
  UINTN         SpiBase;
  UINT32        SpiSoftFreq;
  UINT16        FlashCycle;

  SpiInstance = SPI_INSTANCE_FROM_SPIPROTOCOL (This);
  SpiBase     = SpiInstance->SpiBase;
  SpiBiosSize = SpiInstance->SpiInitTable.BiosSize;
  Operation   = SpiInstance->SpiInitTable.OpcodeMenu[OpcodeIndex].Operation;
  OpCode      = MmioRead8 (SpiBase + R_PCH_SPI_OPMENU0 + OpcodeIndex);

  //
  // Check if the value of opcode register is 0 or the BIOS Size of SpiInitTable is 0
  //
  if (OpCode == 0 || SpiBiosSize == 0) {
    ASSERT (FALSE);
    return EFI_INVALID_PARAMETER;
  }
  //
  // When current code is read id OR current is not descriptor mode, we will use compatible mode
  //
  UseSoftwareSequence = FALSE;
  if ((Operation == EnumSpiOperationJedecId) ||
      (Operation == EnumSpiOperationReadStatus) ||
      (Operation == EnumSpiOperationWriteStatus) ||
      (Operation == EnumSpiOperationWriteDisable) ||
      (Operation == EnumSpiOperationWriteEnable) ||
      (Operation == EnumSpiOperationEnableWriteStatus) ||
      (Operation == EnumSpiOperationOther) ||
      (Operation == EnumSpiOperationDiscoveryParameters) ||
      (!PchIsSpiDescriptorMode (SpiBase))
     ) {
    UseSoftwareSequence = TRUE;
  }

  SpiOffset2Physical (This, Address, SpiRegionType, &HardwareSpiAddr, &BaseAddress, &LimitAddress);
  //
  // Have direct access to BIOS region in Descriptor mode,
  //
  if (SpiInstance->SpiInitTable.OpcodeMenu[OpcodeIndex].Type == EnumSpiOpcodeRead &&
      SpiRegionType == EnumSpiRegionBios) {
    CopyMem (
      Buffer,
      (UINT8 *) ((HardwareSpiAddr - BaseAddress) + (UINT32) (~(SpiBiosSize - 1))),
      DataByteCount
      );
    return EFI_SUCCESS;
  }
  //
  // DEBUG((EFI_D_ERROR, "SPIADDR %x, %x, %x, %x\n", Address, HardwareSpiAddr, BaseAddress,
  // LimitAddress));
  //
  if ((DataCycle == FALSE) && (DataByteCount > 0)) {
    DataByteCount = 0;
  }

  do {
    //
    // Trim at 256 byte boundary per operation,
    // - PCH SPI controller requires trimming at 4KB boundary
    // - Some SPI chips require trimming at 256 byte boundary for write operation
    // - Trimming has limited performance impact as we can read / write atmost 64 byte
    //   per operation
    //
    if (HardwareSpiAddr + DataByteCount > ((HardwareSpiAddr + BIT8) &~(BIT8 - 1))) {
      SpiDataCount = (((UINT32) (HardwareSpiAddr) + BIT8) &~(BIT8 - 1)) - (UINT32) (HardwareSpiAddr);
    } else {
      SpiDataCount = DataByteCount;
    }
    //
    // Calculate the number of bytes to shift in/out during the SPI data cycle.
    // Valid settings for the number of bytes during each data portion of the
    // PCH SPI cycles are: 0, 1, 2, 3, 4, 5, 6, 7, 8, 16, 24, 32, 40, 48, 56, 64
    //
    if ((!PchIsSpiDescriptorMode (SpiBase)) &&
        (OpCode == PCH_SPI_COMMAND_PROGRAM_BYTE || Operation == EnumSpiOperationProgramData_1_Byte)) {
      SpiDataCount = 1;
    } else if (SpiDataCount >= 64) {
      SpiDataCount = 64;
    } else if ((SpiDataCount &~0x07) != 0) {
      SpiDataCount = SpiDataCount &~0x07;
    }
    //
    // If shifts data out, load data into the SPI data buffer.
    //
    if (ShiftOut) {
      for (Index = 0; Index < SpiDataCount; Index++) {
        MmioWrite8 (SpiBase + R_PCH_SPI_FDATA00 + Index, Buffer[Index]);
        MmioRead8 (SpiBase + R_PCH_SPI_FDATA00 + Index);
      }
    }

    MmioWrite32 (
      (SpiBase + R_PCH_SPI_FADDR),
      (UINT32) (HardwareSpiAddr & B_PCH_SPI_FADDR_MASK)
      );
    MmioRead32 (SpiBase + R_PCH_SPI_FADDR);

    //
    // Execute the command on the SPI compatible mode
    //
    if (UseSoftwareSequence) {
      //
      // Software sequencing ...
      //
      //
      // Clear error flags
      //
      MmioWrite16 (SpiBase + R_PCH_SPI_HSFS, (UINT16) B_PCH_SPI_HSFS_AEL);
      MmioWrite8 (SpiBase + R_PCH_SPI_SSFCS, (UINT8) B_PCH_SPI_SSFCS_FCERR);

      /*
            switch (SpiInstance->SpiInitTable.OpcodeMenu[OpcodeIndex].Frequency) {
            case EnumSpiCycle20MHz:
              SpiSoftFreq = V_PCH_SPI_SSFCS_SCF_20MHZ;
              break;

            case EnumSpiCycle33MHz:
              SpiSoftFreq = V_PCH_SPI_SSFCS_SCF_33MHZ;
              break;

            case EnumSpiCycle50MHz:
              SpiSoftFreq = V_PCH_SPI_SSFCS_SCF_50MHZ;
              break;

            default:
              //
              // This is an invalid use of the protocol
              // See definition, but caller must call with valid value
              //
              SpiSoftFreq = 0;
              ASSERT (!EFI_UNSUPPORTED);
              break;
            }
      */

      // PnP settings
      SpiSoftFreq = V_PCH_SPI_SSFCS_SCF_50MHZ;

      ///
      /// PCH Chipset EDS 1.1, Section 22.1.19
      /// SSFC BIT23:19 are reserved, BIOS must set this field to '11111'b
      /// To change the offset to the right DWORD boundary, so use offset 0x90 as the operation address
      ///
      if (DataCycle) {
        MmioWrite32 (
          (SpiBase + R_PCH_SPI_SSFCS),
          ( (UINT32) (BIT31 | BIT30 | BIT29 | BIT28 | BIT27) |
            (UINT32) ((SpiSoftFreq << 24) & B_PCH_SPI_SSFCS_SCF_MASK) |
            (UINT32) (B_PCH_SPI_SSFCS_DC) | (UINT32) (((SpiDataCount - 1) << 16) & B_PCH_SPI_SSFCS_DBC_MASK) |
            (UINT32) ((OpcodeIndex << 12) & B_PCH_SPI_SSFCS_COP) |
            (UINT32) ((PrefixOpcodeIndex << 11) & B_PCH_SPI_SSFCS_SPOP) |
            (UINT32) (Atomic ? B_PCH_SPI_SSFCS_ACS : 0) |
            (UINT32) (B_PCH_SPI_SSFCS_SCGO)));
      } else {
        MmioWrite32 (
          (SpiBase + R_PCH_SPI_SSFCS),
          ( (UINT32) (BIT31 | BIT30 | BIT29 | BIT28 | BIT27) |
            (UINT32) ((SpiSoftFreq << 24) & B_PCH_SPI_SSFCS_SCF_MASK) |
            (UINT32) ((OpcodeIndex << 12) & B_PCH_SPI_SSFCS_COP) |
            (UINT32) ((PrefixOpcodeIndex << 11) & B_PCH_SPI_SSFCS_SPOP) |
            (UINT32) (Atomic ? B_PCH_SPI_SSFCS_ACS : 0) |
            (UINT32) (B_PCH_SPI_SSFCS_SCGO)));
      }
      MmioRead32 (SpiBase + R_PCH_SPI_SSFCS);
    } else {
      //
      // Hardware sequencing ...
      //
      //
      // check for PCH SPI hardware sequencing required commands
      //
      if (Operation == EnumSpiOperationReadData ||
          Operation == EnumSpiOperationFastRead ||
          Operation == EnumSpiOperationDualOutputFastRead) {
        //
        // Read Cycle
        //
        FlashCycle = (UINT16) (V_PCH_SPI_HSFC_FCYCLE_READ << 1);
      } else if (Operation == EnumSpiOperationProgramData_1_Byte ||
                 Operation == EnumSpiOperationProgramData_64_Byte) {
        //
        // Write Cycle
        //
        FlashCycle = (UINT16) (V_PCH_SPI_HSFC_FCYCLE_WRITE << 1);
      } else if (Operation == EnumSpiOperationErase_256_Byte ||
                 Operation == EnumSpiOperationErase_4K_Byte ||
                 Operation == EnumSpiOperationErase_8K_Byte ||
                 Operation == EnumSpiOperationErase_64K_Byte ||
                 Operation == EnumSpiOperationFullChipErase) {
        //
        // Erase Cycle
        //
        FlashCycle = (UINT16) (V_PCH_SPI_HSFC_FCYCLE_ERASE << 1);
      } else {
        //
        // Unrecognized Operation
        //
        ASSERT (FALSE);
        return EFI_ACCESS_DENIED;
      }
      //
      // Clear error flags
      //
      MmioWrite16 (
        SpiBase + R_PCH_SPI_HSFS,
        (UINT16) (B_PCH_SPI_HSFS_AEL | B_PCH_SPI_HSFS_FCERR)
        );
      //
      // Send the command
      //
      MmioWrite16 (
        SpiBase + R_PCH_SPI_HSFC,
        (UINT16) (((SpiDataCount - 1) << 8) & B_PCH_SPI_HSFC_FDBC_MASK) |
        FlashCycle | B_PCH_SPI_HSFC_FCYCLE_FGO
        );
      //
      // Read back for posted write to take effect
      //
      MmioRead16 (SpiBase + R_PCH_SPI_HSFC);
    }
    //
    // end of command execution
    //
    // Wait the SPI cycle to complete.
    //
    if (!WaitForSpiCycleComplete (This, UseSoftwareSequence, TRUE)) {
      return EFI_DEVICE_ERROR;
    }
    //
    // If shifts data in, get data from the SPI data buffer.
    //
    if (!ShiftOut) {
      for (Index = 0; Index < SpiDataCount; Index++) {
        Buffer[Index] = MmioRead8 (SpiBase + R_PCH_SPI_FDATA00 + Index);
      }
    }

    HardwareSpiAddr += SpiDataCount;
    Buffer += SpiDataCount;
    DataByteCount -= SpiDataCount;
  } while (DataByteCount > 0);

  return EFI_SUCCESS;
}

BOOLEAN
WaitForSpiCycleComplete (
  IN     EFI_SPI_PROTOCOL   *This,
  IN     BOOLEAN            UseSoftwareSequence,
  IN     BOOLEAN            ErrorCheck
)
/*++

Routine Description:

  Wait execution cycle to complete on the SPI interface. Check both Hardware
  and Software Sequencing status registers

Arguments:

  This                - The SPI protocol instance
  UseSoftwareSequence - TRUE if this is a Hardware Sequencing operation
  ErrorCheck          - TRUE if the SpiCycle needs to do the error check

Returns:

  TRUE       SPI cycle completed on the interface.
  FALSE      Time out while waiting the SPI cycle to complete.
             It's not safe to program the next command on the SPI interface.

--*/
{
  UINT64        WaitTicks;
  UINT64        WaitCount;
  UINT32        StatusRegAddr;
  UINT32        CycleInProgressBit;
  UINT16        AelBit;
  UINT16        FcErrBit;
  UINT16        FcycleDone;
  UINT16        Data16;
  SPI_INSTANCE  *SpiInstance;
  UINTN         SpiBase;

  SpiInstance = SPI_INSTANCE_FROM_SPIPROTOCOL (This);
  SpiBase     = SpiInstance->SpiBase;

  if (UseSoftwareSequence) {
    //
    // This is Software Sequencing
    //
    StatusRegAddr       = R_PCH_SPI_SSFCS;
    CycleInProgressBit  = B_PCH_SPI_SSFCS_SCIP;
    AelBit              = B_PCH_SPI_SSFCS_AEL;
    FcErrBit            = B_PCH_SPI_SSFCS_FCERR;
    FcycleDone          = B_PCH_SPI_SSFCS_CDS;
  } else {
    //
    // This is Hardware Sequencing
    //
    StatusRegAddr       = R_PCH_SPI_HSFS;
    CycleInProgressBit  = B_PCH_SPI_HSFS_SCIP;
    AelBit              = B_PCH_SPI_HSFS_AEL;
    FcErrBit            = B_PCH_SPI_HSFS_FCERR;
    FcycleDone          = B_PCH_SPI_HSFS_FDONE;
  }
  //
  // Convert the wait period allowed into to tick count
  //
  WaitCount = WAIT_TIME / WAIT_PERIOD;

  //
  // Wait for the SPI cycle to complete.
  //
  for (WaitTicks = 0; WaitTicks < WaitCount; WaitTicks++) {
    Data16 = MmioRead16 (SpiBase + StatusRegAddr);
    if ((Data16 & CycleInProgressBit) == 0) {
      if (UseSoftwareSequence) {
        MmioWrite8 (SpiBase + StatusRegAddr, (UINT8)(AelBit | FcErrBit | FcycleDone));
      } else {
        MmioWrite16 (SpiBase + StatusRegAddr, (AelBit | FcErrBit | FcycleDone));
      }
      if (((Data16 & AelBit) || (Data16 & FcErrBit)) && (ErrorCheck == TRUE)) {
        return FALSE;
      } else {
        return TRUE;
      }
    }

    PchPmTimerStall (WAIT_PERIOD);
  }

  return FALSE;
}
