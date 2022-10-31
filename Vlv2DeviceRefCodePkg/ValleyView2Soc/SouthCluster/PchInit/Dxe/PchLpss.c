/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  Pchlpss.c

Abstract:

  Initializes PCH lpss Device

--*/
#include "PchInit.h"
#ifdef ECP_FLAG
#include <Acpi.h>
#define EFI_ACPI_5_0_CORE_SYSTEM_RESOURCE_TABLE_SIGNATURE  SIGNATURE_32('C', 'S', 'R', 'T')
#else
#include <IndustryStandard/Acpi.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/PciHostBridgeResourceAllocation.h>
#endif
#include <Protocol/GlobalNvsArea.h>
#ifdef ECP_FLAG
EFI_GUID gEfiGlobalNvsAreaProtocolGuid = EFI_GLOBAL_NVS_AREA_PROTOCOL_GUID;
#include <Protocol/PciIo/PciIo.h>
#include "Pci22.h"
#else
#include <Protocol/PciIo.h>
#include <IndustryStandard/Pci22.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>
#endif


//
// CSRT Definitions
//
#define EFI_ACPI_CSRT_TABLE_REVISION 0x0
#define MAX_NO_CHANNEL1_SUPPORTED    7
#define MAX_NO_CHANNEL2_SUPPORTED    9
#define EFI_ACPI_OEM_ID           "INTEL "  // OEMID 6 bytes long
#define EFI_ACPI_OEM_TABLE_ID     SIGNATURE_64('E','D','K','2',' ',' ',' ',' ')// OEM table id 8 bytes long
#define EFI_ACPI_OEM_REVISION     0x00000005
#define EFI_ACPI_CREATOR_ID       SIGNATURE_32('I','N','T','L')
#define EFI_ACPI_CREATOR_REVISION 0x20120624
//
// Ensure proper structure formats for CSRT
//
#pragma pack (1)
///
/// Resource  Share Info
///
typedef struct _SHARED_INFO_SECTION {
  UINT16 MajVersion;
  UINT16 MinVersion;
  UINT32 MMIOLowPart;
  UINT32 MMIOHighPart;
  UINT32 IntGSI;
  UINT8 IntPol;
  UINT8 IntMode;
  UINT8 NoOfCh;
  UINT8 DMAAddressWidth;
  UINT16 BaseReqLine;
  UINT16 NoOfHandSig;
  UINT32 MaxBlockTransferSize;
} SHARED_INFO_SECTION;
///
/// Resource Group Header
///
typedef struct _RESOURCE_GROUP_HEADER {
  UINT32 Length;
  UINT32 VendorId;
  UINT32 SubVendorId;
  UINT16 DeviceId;
  UINT16 SubDeviceId;
  UINT16 Revision;
  UINT16 Reserved;
  UINT32 SharedInfoLength;
  SHARED_INFO_SECTION SharedInfoSection;
} RESOURCE_GROUP_HEADER;
///
/// Resource Descriptor Header
///
typedef struct _RESOURCE_DESCRIPTOR {
  UINT32 Length;
  UINT16 ResourceType;
  UINT16 ResourceSubType;
  UINT32 UUID;
} RESOURCE_DESCRIPTOR;
typedef struct {
  RESOURCE_GROUP_HEADER          ResourceGroupHeaderInfo;
  RESOURCE_DESCRIPTOR            ResourceDescriptorInfo[MAX_NO_CHANNEL1_SUPPORTED];
} RESOURCE_GROUP_INFO1;
typedef struct {
  RESOURCE_GROUP_HEADER          ResourceGroupHeaderInfo;
  RESOURCE_DESCRIPTOR            ResourceDescriptorInfo[MAX_NO_CHANNEL2_SUPPORTED];
} RESOURCE_GROUP_INFO2;
///
/// Core System Resources Table Structure (CSRT)
///
typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER            Header;
  RESOURCE_GROUP_INFO1              ResourceGroupsInfo1;
  RESOURCE_GROUP_INFO2              ResourceGroupsInfo2;
} EFI_ACPI_CSRT_TABLE;
#pragma pack ()

//
//Update Lpss/Scc devices
//
#ifndef ECP_FLAG
#include <PiDxe.h>
#include <Protocol/PciHostBridgeResourceAllocation.h>
#include <Protocol/PciEnumerationComplete.h>
#include <Protocol/GlobalNvsArea.h>
#include <PchRegs.h>
#include <PchRegs/PchRegsLpss.h> 
#include <PchRegs/PchRegsScc.h> 
#endif
typedef struct _LPSS_DEVICE_INFO {
  UINTN        Segment;
  UINTN        BusNum;
  UINTN        DeviceNum;
  UINTN        FunctionNum;
  UINTN        ReportBarIndex;
  EFI_PHYSICAL_ADDRESS    ReportBar;
  UINT64                  ReportBarLen;
  UINTN                   AddrOffset;
  UINTN                   LenOffset;
  UINTN        ReportBarIndex1;
  EFI_PHYSICAL_ADDRESS    ReportBar1;
  UINT64                  ReportBarLen1;
  UINTN                   AddrOffset1;
  UINTN                   LenOffset1;
  UINT32                  AcpiModeRegOffset;
  UINT32                  AcpiModeRegValue;
} LPSS_DEVICE_INFO;

#ifdef ECP_FLAG
extern EFI_GUID gLpssDummyProtocolGuid = EFI_LPSS_DUMMY_PROTOCOL_GUID;
#else
extern EFI_GUID gLpssDummyProtocolGuid;
#endif

#define GLOBAL_NVS_OFFSET(Field)    (UINTN)((CHAR8*)&((EFI_GLOBAL_NVS_AREA*)0)->Field - (CHAR8*)0)
LPSS_DEVICE_INFO  mDeviceList[] = {

  //Scc

  {
    0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_SCC_SDIO_1, PCI_FUNCTION_NUMBER_PCH_SCC_SDIO,\
    0, 0, 0, GLOBAL_NVS_OFFSET(SDIOAddr),  GLOBAL_NVS_OFFSET(SDIOLen), \
    1, 0, 0, GLOBAL_NVS_OFFSET(SDIO1Addr), GLOBAL_NVS_OFFSET(SDIO1Len), \
    R_PCH_SCC_EP_PCICFGCTR3, (B_PCH_SCC_EP_PCICFGCTR3_ACPI_INT_EN1 | B_PCH_SCC_EP_PCICFGCTR3_PCI_CFG_DIS1)
  },

  /// Move to LpssDxe for SD device booting function to work


  //Lpss1
  {
    0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_DMAC0, PCI_FUNCTION_NUMBER_PCH_LPSS_DMAC,\
    0, 0, 0, GLOBAL_NVS_OFFSET(LDMA1Addr),    GLOBAL_NVS_OFFSET(LDMA1Len), \
    1, 0, 0, GLOBAL_NVS_OFFSET(LDMA11Addr), GLOBAL_NVS_OFFSET(LDMA11Len), \
    R_PCH_LPSS_FABCTLP0, (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS)
  },

  {
    0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_PWM, PCI_FUNCTION_NUMBER_PCH_LPSS_PWM0,\
    0, 0, 0, GLOBAL_NVS_OFFSET(PWM1Addr),  GLOBAL_NVS_OFFSET(PWM1Len), \
    1, 0, 0, GLOBAL_NVS_OFFSET(PWM11Addr), GLOBAL_NVS_OFFSET(PWM11Len), \
    R_PCH_LPSS_FABCTLP1, (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS)
  },

  {
    0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_PWM, PCI_FUNCTION_NUMBER_PCH_LPSS_PWM1,\
    0, 0, 0, GLOBAL_NVS_OFFSET(PWM2Addr),  GLOBAL_NVS_OFFSET(PWM2Len), \
    1, 0, 0, GLOBAL_NVS_OFFSET(PWM21Addr), GLOBAL_NVS_OFFSET(PWM21Len), \
    R_PCH_LPSS_FABCTLP2, (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS)
  },

  {
    0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_HSUART, PCI_FUNCTION_NUMBER_PCH_LPSS_HSUART0,\
    0, 0, 0, GLOBAL_NVS_OFFSET(UART1Addr),  GLOBAL_NVS_OFFSET(UART1Len), \
    1, 0, 0, GLOBAL_NVS_OFFSET(UART11Addr), GLOBAL_NVS_OFFSET(UART11Len), \
    R_PCH_LPSS_FABCTLP3, (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS)
  },

  {
    0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_HSUART, PCI_FUNCTION_NUMBER_PCH_LPSS_HSUART1,\
    0, 0, 0, GLOBAL_NVS_OFFSET(UART2Addr),  GLOBAL_NVS_OFFSET(UART2Len), \
    1, 0, 0, GLOBAL_NVS_OFFSET(UART21Addr), GLOBAL_NVS_OFFSET(UART21Len), \
    R_PCH_LPSS_FABCTLP4, (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS)
  },

  {
    0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_SPI, PCI_FUNCTION_NUMBER_PCH_LPSS_SPI,\
    0, 0, 0, GLOBAL_NVS_OFFSET(SPIAddr),  GLOBAL_NVS_OFFSET(SPILen), \
    1, 0, 0, GLOBAL_NVS_OFFSET(SPI1Addr), GLOBAL_NVS_OFFSET(SPI1Len), \
    R_PCH_LPSS_FABCTLP5, (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS)
  },

  //LPSS2
  {
    0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_DMAC1, PCI_FUNCTION_NUMBER_PCH_LPSS_DMAC,\
    0, 0, 0, GLOBAL_NVS_OFFSET(LDMA2Addr),  GLOBAL_NVS_OFFSET(LDMA2Len), \
    1, 0, 0, GLOBAL_NVS_OFFSET(LDMA21Addr), GLOBAL_NVS_OFFSET(LDMA21Len), \
    R_PCH_LPSS_FAB2CTLP0, (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS)
  },

  {
    0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C, PCI_FUNCTION_NUMBER_PCH_LPSS_I2C0,\
    0, 0, 0, GLOBAL_NVS_OFFSET(I2C1Addr),  GLOBAL_NVS_OFFSET(I2C1Len), \
    1, 0, 0, GLOBAL_NVS_OFFSET(I2C11Addr), GLOBAL_NVS_OFFSET(I2C11Len), \
    R_PCH_LPSS_FAB2CTLP1, (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS)
  },

  {
    0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C, PCI_FUNCTION_NUMBER_PCH_LPSS_I2C1,\
    0, 0, 0, GLOBAL_NVS_OFFSET(I2C2Addr),  GLOBAL_NVS_OFFSET(I2C2Len), \
    1, 0, 0, GLOBAL_NVS_OFFSET(I2C21Addr), GLOBAL_NVS_OFFSET(I2C21Len), \
    R_PCH_LPSS_FAB2CTLP2, (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS)
  },

  {
    0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C, PCI_FUNCTION_NUMBER_PCH_LPSS_I2C2,\
    0, 0, 0, GLOBAL_NVS_OFFSET(I2C3Addr),  GLOBAL_NVS_OFFSET(I2C3Len), \
    1, 0, 0, GLOBAL_NVS_OFFSET(I2C31Addr), GLOBAL_NVS_OFFSET(I2C31Len), \
    R_PCH_LPSS_FAB2CTLP3, (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS)
  },

  {
    0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C, PCI_FUNCTION_NUMBER_PCH_LPSS_I2C3,\
    0, 0, 0, GLOBAL_NVS_OFFSET(I2C4Addr),  GLOBAL_NVS_OFFSET(I2C4Len), \
    1, 0, 0, GLOBAL_NVS_OFFSET(I2C41Addr), GLOBAL_NVS_OFFSET(I2C41Len), \
    R_PCH_LPSS_FAB2CTLP4, (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS)
  },

  {
    0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C, PCI_FUNCTION_NUMBER_PCH_LPSS_I2C4,\
    0, 0, 0, GLOBAL_NVS_OFFSET(I2C5Addr),  GLOBAL_NVS_OFFSET(I2C5Len), \
    1, 0, 0, GLOBAL_NVS_OFFSET(I2C51Addr), GLOBAL_NVS_OFFSET(I2C51Len), \
    R_PCH_LPSS_FAB2CTLP5, (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS)
  },

  {
    0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C, PCI_FUNCTION_NUMBER_PCH_LPSS_I2C5,\
    0, 0, 0, GLOBAL_NVS_OFFSET(I2C6Addr),  GLOBAL_NVS_OFFSET(I2C6Len), \
    1, 0, 0, GLOBAL_NVS_OFFSET(I2C61Addr), GLOBAL_NVS_OFFSET(I2C61Len), \
    R_PCH_LPSS_FAB2CTLP6, (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS)
  },

  {
    0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C, PCI_FUNCTION_NUMBER_PCH_LPSS_I2C6,\
    0, 0, 0, GLOBAL_NVS_OFFSET(I2C7Addr),  GLOBAL_NVS_OFFSET(I2C7Len), \
    1, 0, 0, GLOBAL_NVS_OFFSET(I2C71Addr), GLOBAL_NVS_OFFSET(I2C71Len), \
    R_PCH_LPSS_FAB2CTLP7, (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS)
  }
};
#define LpssScc_DEVICE_NUMBER  sizeof(mDeviceList)/sizeof(LPSS_DEVICE_INFO)


VOID
SignalAllDriversLpssDone (
  VOID
  )
{
  EFI_HANDLE                 Handle;
  EFI_STATUS                 Status;

  //
  // Inform other code that all drivers have been connected.
  //
  Handle = NULL;
  DEBUG((DEBUG_INFO, "gLpssDummyProtocolGuid\n"));
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gLpssDummyProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

}


VOID
UpdateLpssSccDeviceBar (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN EFI_PHYSICAL_ADDRESS    BaseAddress,
  IN UINT8                   BarIndex
  )
{
  UINT8                   NewCommand;
  UINT32                  NewBar;
  EFI_STATUS              Status;
  UINT16                   Command;
  UINTN                   SccPciMmBase;
  UINTN                   Seg, Bus, Dev, Func;

  Status = PciIo->GetLocation(
                    PciIo,
                    &Seg,
                    &Bus,
                    &Dev,
                    &Func
                    );
  if (EFI_ERROR (Status)) {
    return;
  }

  SccPciMmBase = MmPciAddress (0,
                               Bus,
                               Dev,
                               Func,
                               0
                               );

  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint16,
                        PCI_COMMAND_OFFSET,
                        1,
                        &Command
                        );

  NewCommand = 0;
  Status = PciIo->Pci.Write(
                        PciIo,
                        EfiPciIoWidthUint8,
                        PCI_COMMAND_OFFSET,
                        1,
                        &NewCommand
                        );
  S3BootScriptSaveMemWrite(
    EfiBootScriptWidthUint8,
    (UINTN) (SccPciMmBase + PCI_COMMAND_OFFSET),
    1,
    (VOID *) (UINTN) (SccPciMmBase + PCI_COMMAND_OFFSET)
  );
  NewBar = (UINT32)BaseAddress;
  Status = PciIo->Pci.Write(
                        PciIo,
                        EfiPciIoWidthUint32,
                        PCI_BASE_ADDRESSREG_OFFSET + BarIndex * sizeof(UINT32),
                        1,
                        &NewBar
                        );
  S3BootScriptSaveMemWrite(
    EfiBootScriptWidthUint32,
    (UINTN) (SccPciMmBase + PCI_BASE_ADDRESSREG_OFFSET + BarIndex * sizeof(UINT32)),
    1,
    (VOID *) (UINTN) (SccPciMmBase + PCI_BASE_ADDRESSREG_OFFSET + BarIndex * sizeof(UINT32))
  );
  Status = PciIo->Pci.Write(
                        PciIo,
                        EfiPciIoWidthUint16,
                        PCI_COMMAND_OFFSET,
                        1,
                        &Command
                        );
  S3BootScriptSaveMemWrite(
    EfiBootScriptWidthUint16,
    (UINTN) (SccPciMmBase + PCI_COMMAND_OFFSET),
    1,
    (VOID *) (UINTN) (SccPciMmBase + PCI_COMMAND_OFFSET)
  );
}


VOID
UpdateLpssSccDevice (
  IN EFI_PCI_IO_PROTOCOL     *PciIo,
  IN LPSS_DEVICE_INFO        *DeviceInfo
  )
{
  VOID         *Resources;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *ptr;
  EFI_STATUS                        Status;
  UINT8                             BarIndex;
  EFI_PHYSICAL_ADDRESS              BaseAddress;

  for (BarIndex = 0; BarIndex < PCI_MAX_BAR; BarIndex ++) {
    Status = PciIo->GetBarAttributes(//scan pci device's bar
                      PciIo,
                      BarIndex,
                      NULL,
                      &Resources
                      );
    if (EFI_ERROR(Status)) {
      continue;
    }
    ptr = Resources;
    if (ptr->ResType != ACPI_ADDRESS_SPACE_TYPE_MEM) {
#ifdef ECP_FLAG
      FreePool (Resources);
#else
      gBS->FreePool(Resources);
#endif
      continue;
    }
    DEBUG((DEBUG_ERROR, "Devices  %x %x %x %x %x %lx %lx %lx %lx %lx\n",
           ptr->Desc, ptr->Len, ptr->ResType, ptr->GenFlag, ptr->SpecificFlag,
           ptr->AddrSpaceGranularity, ptr->AddrRangeMin, ptr->AddrRangeMax,
           ptr->AddrTranslationOffset, ptr->AddrLen
          ));
    BaseAddress = 0xFFFFFFFF;
#if 1
    Status = gDS->AllocateMemorySpace (
                    EfiGcdAllocateMaxAddressSearchBottomUp,
                    EfiGcdMemoryTypeMemoryMappedIo,
                    HighBitSet64(ptr->AddrLen),
                    ptr->AddrLen,
                    &BaseAddress,
                    mImageHandle,
                    NULL
                    );
#else
    RegBase = MmPciAddress (
                0,
                DeviceInfo->BusNum,
                DeviceInfo->DeviceNum,
                DeviceInfo->FunctionNum,
                0
                );
    BaseAddress = (EFI_PHYSICAL_ADDRESS)(MmioRead32(RegBase + R_PCH_LPSS_SDIO_BAR + 0x04 * BarIndex) & B_PCH_LPSS_SDIO_BAR_BA);
#endif
    //ASSERT_EFI_ERROR (Status);
    DEBUG((DEBUG_ERROR, "New Resource is %x %lx\n", BarIndex, BaseAddress));
    UpdateLpssSccDeviceBar(PciIo, BaseAddress, BarIndex);
    if (BarIndex == DeviceInfo->ReportBarIndex) {
      DeviceInfo->ReportBar = BaseAddress;
      DeviceInfo->ReportBarLen = ptr->AddrLen;
    }
    if (BarIndex == DeviceInfo->ReportBarIndex1) {
      DeviceInfo->ReportBar1 = BaseAddress;
      DeviceInfo->ReportBarLen1 = ptr->AddrLen;
    }
#ifdef ECP_FLAG
    FreePool (Resources);
#else
    gBS->FreePool(Resources);
#endif
  }
}


EFI_STATUS
UpdateLpssSccDeviceList (
  VOID
  )
{
  EFI_STATUS              Status;
  UINTN                   HandleCount;
  EFI_HANDLE              *Handles;
  EFI_PCI_IO_PROTOCOL     *PciIo;
  UINTN                   Index;
  UINTN                   Index2;
  UINTN                   Segment;
  UINTN                   BusNum;
  UINTN                   DeviceNum;
  UINTN                   FunctionNum;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &Handles
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  for(Index = 0; Index < HandleCount; Index ++) {
    Status = gBS->HandleProtocol (
                    Handles[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &PciIo
                    );
    if (EFI_ERROR(Status)) {
      continue;
    }
    Status = PciIo->GetLocation(
                      PciIo,
                      &Segment,
                      &BusNum,
                      &DeviceNum,
                      &FunctionNum
                      );
    for(Index2 = 0; Index2 < LpssScc_DEVICE_NUMBER; Index2 ++) {
      if (mDeviceList[Index2].Segment     == Segment &&
          mDeviceList[Index2].BusNum      == BusNum &&
          mDeviceList[Index2].DeviceNum   == DeviceNum &&
          mDeviceList[Index2].FunctionNum == FunctionNum) {
        DEBUG((EFI_D_ERROR, "Update Device %x %x %x\n", BusNum, DeviceNum, FunctionNum));
        UpdateLpssSccDevice(PciIo, &mDeviceList[Index2]);
      }
    }
  }
#ifdef ECP_FLAG
  FreePool (Handles);
#else
  gBS->FreePool(Handles);
#endif
  return EFI_SUCCESS;
}


EFI_STATUS
GetPCIAddrRange(
  IN EFI_PHYSICAL_ADDRESS *RangeMin,
  IN EFI_PHYSICAL_ADDRESS *RangeMax
  )
{
  EFI_STATUS                                       Status;
  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *PciResource;
  EFI_HANDLE                                       RootBridgeHandle;
  UINT64                                           Attributes;
  VOID                                             *Configuration;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR                *ptr;
  BOOLEAN                                          Found;
  BOOLEAN                                          NextBridge;

  *RangeMin = (EFI_PHYSICAL_ADDRESS)(-1);
  *RangeMax = 0;
  Status = gBS->LocateProtocol (
                  &gEfiPciHostBridgeResourceAllocationProtocolGuid,
                  NULL,
                  (VOID **) &PciResource
                  );
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Can't locate PciResource Protocol\n"));
    return EFI_NOT_FOUND;

  }
  RootBridgeHandle = NULL;
  Found = FALSE;
  NextBridge = TRUE;
  while(NextBridge) {
    Status = PciResource->GetNextRootBridge(
                            PciResource,
                            &RootBridgeHandle
                            );
    if (EFI_ERROR(Status)) {
      NextBridge = FALSE;
      continue;
    }
    Status = PciResource->GetAllocAttributes (
                            PciResource,
                            RootBridgeHandle,
                            &Attributes
                            );

    DEBUG((DEBUG_ERROR, "RootBridgeHandle %x %lx\n", RootBridgeHandle, Attributes));
    Status = PciResource->GetProposedResources (
                            PciResource,
                            RootBridgeHandle,
                            &Configuration
                            );
    if (EFI_ERROR(Status)) {
      continue;
    }
    ptr = Configuration;
    while(ptr->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR) {
      DEBUG((DEBUG_ERROR, "  %x %x %x %x %x %lx %lx %lx %lx %lx\n",
             ptr->Desc, ptr->Len, ptr->ResType, ptr->GenFlag, ptr->SpecificFlag,
             ptr->AddrSpaceGranularity, ptr->AddrRangeMin, ptr->AddrRangeMax,
             ptr->AddrTranslationOffset, ptr->AddrLen
            ));
      if (ptr->ResType == ACPI_ADDRESS_SPACE_TYPE_MEM) {
        if (*RangeMin > ptr->AddrRangeMin)  *RangeMin = ptr->AddrRangeMin;
        if (*RangeMax < (ptr->AddrRangeMin + ptr->AddrLen))  *RangeMax = ptr->AddrRangeMin + ptr->AddrLen;
        Found = TRUE;
      }
      ptr ++;
    }
#ifdef ECP_FLAG
    FreePool (Configuration);
#else
    gBS->FreePool(Configuration);
#endif
  }
  if (!Found) {
    return EFI_NOT_FOUND;
  }
  return EFI_SUCCESS;
}


VOID
UpdateLpssSccDeviceInfo (
  IN EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsArea
  )
{
  UINTN         Index;

  for(Index = 0; Index < LpssScc_DEVICE_NUMBER; Index ++) {
    if (mDeviceList[Index].ReportBar != 0) {
      *(UINT32*)((CHAR8*)GlobalNvsArea->Area + mDeviceList[Index].AddrOffset)  = (UINT32)mDeviceList[Index].ReportBar;
      *(UINT32*)((CHAR8*)GlobalNvsArea->Area + mDeviceList[Index].LenOffset)   = (UINT32)mDeviceList[Index].ReportBarLen;
      *(UINT32*)((CHAR8*)GlobalNvsArea->Area + mDeviceList[Index].AddrOffset1) = (UINT32)mDeviceList[Index].ReportBar1;
      *(UINT32*)((CHAR8*)GlobalNvsArea->Area + mDeviceList[Index].LenOffset1)  = (UINT32)mDeviceList[Index].ReportBarLen1;
      //LpssAcpiModeEnable(&mDeviceList[Index]);
    }
  }
}


EFI_STATUS
ConfigureLpss (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy,
  IN OUT UINT32                            *FuncDisableReg
  )
/*++

Routine Description:

  Configure lpss devices.

Arguments:

  PchPlatformPolicy       The PCH Platform Policy protocol instance
  FuncDisableReg          Function Disable Register

Returns:

  EFI_SUCCESS             The function completed successfully

--*/
{
  UINTN                              LpssPciMmBase;
  EFI_PHYSICAL_ADDRESS               LpssMmioBase0;
  UINTN                              LpssMmioBase1;
  UINT32                             Buffer32;
  EFI_STATUS                         Status;

  DEBUG ((EFI_D_INFO, "ConfigureLpss() Start\n"));

  LpssPciMmBase = 0;
  LpssMmioBase0 = 0;
  LpssMmioBase1 = 0;
  Buffer32      = 0;

  ///
  /// lpss2 DMA
  ///
  LpssPciMmBase = MmPciAddress (
                    0,
                    PchPlatformPolicy->BusNumber,
                    PCI_DEVICE_NUMBER_PCH_LPSS_DMAC1,
                    PCI_FUNCTION_NUMBER_PCH_LPSS_DMAC,
                    0
                    );
  ///
  /// lpss2 DMA will be disabled under these two conditions:
  /// 1) When the device is instructed to be disabled.
  /// 2) When the rest of the lpss2 devices are disabled.
  ///
  if ((PchPlatformPolicy->LpssConfig->Dma1Enabled == PCH_DEVICE_DISABLE) ||
      ((PchPlatformPolicy->LpssConfig->I2C0Enabled == PCH_DEVICE_DISABLE) &&
       (PchPlatformPolicy->LpssConfig->I2C1Enabled == PCH_DEVICE_DISABLE) &&
       (PchPlatformPolicy->LpssConfig->I2C2Enabled == PCH_DEVICE_DISABLE) &&
       (PchPlatformPolicy->LpssConfig->I2C3Enabled == PCH_DEVICE_DISABLE) &&
       (PchPlatformPolicy->LpssConfig->I2C4Enabled == PCH_DEVICE_DISABLE) &&
       (PchPlatformPolicy->LpssConfig->I2C5Enabled == PCH_DEVICE_DISABLE) &&
       (PchPlatformPolicy->LpssConfig->I2C6Enabled == PCH_DEVICE_DISABLE))) {
    ///
    /// Disable the rest of the lpss2 devices since lpss2 DMA is disabled.
    ///
    PchPlatformPolicy->LpssConfig->I2C0Enabled = PCH_DEVICE_DISABLE;
    PchPlatformPolicy->LpssConfig->I2C1Enabled = PCH_DEVICE_DISABLE;
    PchPlatformPolicy->LpssConfig->I2C2Enabled = PCH_DEVICE_DISABLE;
    PchPlatformPolicy->LpssConfig->I2C3Enabled = PCH_DEVICE_DISABLE;
    PchPlatformPolicy->LpssConfig->I2C4Enabled = PCH_DEVICE_DISABLE;
    PchPlatformPolicy->LpssConfig->I2C5Enabled = PCH_DEVICE_DISABLE;
    PchPlatformPolicy->LpssConfig->I2C6Enabled = PCH_DEVICE_DISABLE;
    ///
    /// Put device in D3Hot state before disabling it.
    ///
    DEBUG ((EFI_D_INFO, "Putting LPSS2 DMA into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_PCS), B_PCH_LPSS_DMAC_PCS_PS);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_PCS),
      1,
      (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_PCS)
      );
    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS2_FUNC0;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (LpssPciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_LPSS_DMAC_BAR_ALIGNMENT,
                      V_PCH_LPSS_DMAC_BAR_SIZE,
                      &LpssMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD), (UINT32) ~(B_PCH_LPSS_DMAC_STSCMD_BME | B_PCH_LPSS_DMAC_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((LpssMmioBase0 & B_PCH_LPSS_DMAC_BAR_BA) == LpssMmioBase0) && (LpssMmioBase0 != 0));
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_BAR), (UINT32) (LpssMmioBase0 & B_PCH_LPSS_DMAC_BAR_BA));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_BAR)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD), (UINT32) (B_PCH_LPSS_DMAC_STSCMD_BME | B_PCH_LPSS_DMAC_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD)
          );
        ///
        /// Enable Power Management Capability
        /// Enable IOSF Snoop
        ///
        PchMsgBusAndThenOr32AddToS3Save (
          PCH_LPSS_EP_PORT_ID,
          R_PCH_LPSS_FAB2CTLP0,
          Buffer32,
          (UINT32) ~(B_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL),
          (V_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL_SNP | B_PCH_LPSS_FABXCTLPX_FAB_PM_CAP_PRSNT),
          PCH_LPSS_EP_PRIVATE_READ_OPCODE,
          PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD), (UINT32) ~(B_PCH_LPSS_DMAC_STSCMD_BME | B_PCH_LPSS_DMAC_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_BAR)
          );
        gDS->FreeMemorySpace (LpssMmioBase0, (UINT64) V_PCH_LPSS_DMAC_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "LPSS2 DMA not present, skipping.\n"));
      ///
      /// Disable the rest of the LPSS2 devices since LPSS2 DMA is not present.
      ///
      PchPlatformPolicy->LpssConfig->Dma1Enabled = PCH_DEVICE_DISABLE;
      PchPlatformPolicy->LpssConfig->I2C0Enabled = PCH_DEVICE_DISABLE;
      PchPlatformPolicy->LpssConfig->I2C1Enabled = PCH_DEVICE_DISABLE;
      PchPlatformPolicy->LpssConfig->I2C2Enabled = PCH_DEVICE_DISABLE;
      PchPlatformPolicy->LpssConfig->I2C3Enabled = PCH_DEVICE_DISABLE;
      PchPlatformPolicy->LpssConfig->I2C4Enabled = PCH_DEVICE_DISABLE;
      PchPlatformPolicy->LpssConfig->I2C5Enabled = PCH_DEVICE_DISABLE;
      PchPlatformPolicy->LpssConfig->I2C6Enabled = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS2_FUNC0;
    }
  }

  ///
  /// LPSS2 I2C 0
  ///
  LpssPciMmBase = MmPciAddress (
                    0,
                    PchPlatformPolicy->BusNumber,
                    PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
                    PCI_FUNCTION_NUMBER_PCH_LPSS_I2C0,
                    0
                    );
  if (PchPlatformPolicy->LpssConfig->I2C0Enabled == PCH_DEVICE_DISABLE) {
    ///
    /// Put device in D3Hot state before disabling it.
    ///
    DEBUG ((EFI_D_INFO, "Putting LPSS2 I2C 0 into D3 Hot State.\n"));
    DEBUG ((EFI_D_INFO, "LpssPciMmBase:------------------%x.\n",LpssPciMmBase));
    MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS), B_PCH_LPSS_I2C_PCS_PS);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS),
      1,
      (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS)
      );

    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS2_FUNC1;

  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (LpssPciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_LPSS_I2C_BAR_ALIGNMENT,
                      V_PCH_LPSS_I2C_BAR_SIZE,
                      &LpssMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) ~(B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((LpssMmioBase0 & B_PCH_LPSS_I2C_BAR_BA) == LpssMmioBase0) && (LpssMmioBase0 != 0));
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (LpssMmioBase0 & B_PCH_LPSS_I2C_BAR_BA));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) (B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ASSERT (MmioRead32 ((UINTN) LpssMmioBase0) != 0xFFFFFFFF);
        ///
        /// Release Resets
        ///
        MmioWrite32 (
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS),
          (UINT32) (B_PCH_LPSS_I2C_MEM_RESETS_FUNC | B_PCH_LPSS_I2C_MEM_RESETS_APB)
          );
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS),
          1,
          (VOID *) (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS)
          );
        ///
        /// Enable Power Management Capability
        /// Enable IOSF Snoop
        ///
        PchMsgBusAndThenOr32AddToS3Save (
          PCH_LPSS_EP_PORT_ID,
          R_PCH_LPSS_FAB2CTLP1,
          Buffer32,
          (UINT32) ~(B_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL),
          (V_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL_SNP | B_PCH_LPSS_FABXCTLPX_FAB_PM_CAP_PRSNT),
          PCH_LPSS_EP_PRIVATE_READ_OPCODE,
          PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) ~(B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR)
          );
        gDS->FreeMemorySpace (LpssMmioBase0, (UINT64) V_PCH_LPSS_I2C_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "LPSS2 I2C 0 not present, skipping.\n"));
      PchPlatformPolicy->LpssConfig->I2C0Enabled = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS2_FUNC1;
    }
  }

  ///
  /// LPSS2 I2C 1
  ///
  LpssPciMmBase = MmPciAddress (
                    0,
                    PchPlatformPolicy->BusNumber,
                    PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
                    PCI_FUNCTION_NUMBER_PCH_LPSS_I2C1,
                    0
                    );

  if (PchPlatformPolicy->LpssConfig->I2C1Enabled == PCH_DEVICE_DISABLE) {
    ///
    /// Put device in D3Hot state before disabling it.
    ///
    DEBUG ((EFI_D_INFO, "Putting LPSS2 I2C 1 into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS), B_PCH_LPSS_I2C_PCS_PS);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS),
      1,
      (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS)
      );

    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS2_FUNC2;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (LpssPciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_LPSS_I2C_BAR_ALIGNMENT,
                      V_PCH_LPSS_I2C_BAR_SIZE,
                      &LpssMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) ~(B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((LpssMmioBase0 & B_PCH_LPSS_I2C_BAR_BA) == LpssMmioBase0) && (LpssMmioBase0 != 0));
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (LpssMmioBase0 & B_PCH_LPSS_I2C_BAR_BA));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) (B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ASSERT (MmioRead32 ((UINTN) LpssMmioBase0) != 0xFFFFFFFF);
        ///
        /// Release Resets
        ///
        MmioWrite32 (
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS),
          (UINT32) (B_PCH_LPSS_I2C_MEM_RESETS_FUNC | B_PCH_LPSS_I2C_MEM_RESETS_APB)
          );
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS),
          1,
          (VOID *) (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS)
          );
        ///
        /// Enable Power Management Capability
        /// Enable IOSF Snoop
        ///
        PchMsgBusAndThenOr32AddToS3Save (
          PCH_LPSS_EP_PORT_ID,
          R_PCH_LPSS_FAB2CTLP2,
          Buffer32,
          (UINT32) ~(B_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL),
          (V_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL_SNP | B_PCH_LPSS_FABXCTLPX_FAB_PM_CAP_PRSNT),
          PCH_LPSS_EP_PRIVATE_READ_OPCODE,
          PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) ~(B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR)
          );
        gDS->FreeMemorySpace (LpssMmioBase0, (UINT64) V_PCH_LPSS_I2C_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "LPSS2 I2C 1 not present, skipping.\n"));
      PchPlatformPolicy->LpssConfig->I2C1Enabled = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS2_FUNC2;
    }
  }

  ///
  /// LPSS2 I2C 2
  ///
  LpssPciMmBase = MmPciAddress (
                    0,
                    PchPlatformPolicy->BusNumber,
                    PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
                    PCI_FUNCTION_NUMBER_PCH_LPSS_I2C2,
                    0
                    );
  if (PchPlatformPolicy->LpssConfig->I2C2Enabled == PCH_DEVICE_DISABLE) {
    ///
    /// Put device in D3Hot state before disabling it.
    ///
    DEBUG ((EFI_D_INFO, "Putting LPSS2 I2C 2 into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS), B_PCH_LPSS_I2C_PCS_PS);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS),
      1,
      (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS)
      );

    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS2_FUNC3;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (LpssPciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_LPSS_I2C_BAR_ALIGNMENT,
                      V_PCH_LPSS_I2C_BAR_SIZE,
                      &LpssMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
    //AMI_OVERRIDE - EIP144300 I2C port 3 IC_COMP_VERSION register not being set >>
	///
	/// Disable Bus Master Enable & Memory Space Enable
	///
	MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) ~(B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
	S3BootScriptSaveMemWrite (
	  EfiBootScriptWidthUint32,
	  (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
	  1,
	  (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
	  );
	///
	/// Program BAR 0
	///
	ASSERT (((LpssMmioBase0 & B_PCH_LPSS_I2C_BAR_BA) == LpssMmioBase0) && (LpssMmioBase0 != 0));
	MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (LpssMmioBase0 & B_PCH_LPSS_I2C_BAR_BA));
	S3BootScriptSaveMemWrite (
	  EfiBootScriptWidthUint32,
	  (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR),
	  1,
	  (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR)
	  );
	///
	/// Bus Master Enable & Memory Space Enable
	///
	MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) (B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
	S3BootScriptSaveMemWrite (
	  EfiBootScriptWidthUint32,
	  (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
	  1,
	  (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
	  );
	ASSERT (MmioRead32 ((UINTN) LpssMmioBase0) != 0xFFFFFFFF);
	//AMI_OVERRIDE - EIP144300 I2C port 3 IC_COMP_VERSION register not being set <<
        ///
        /// Release Resets
        ///
        MmioWrite32 (
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS),
          (UINT32) (B_PCH_LPSS_I2C_MEM_RESETS_FUNC | B_PCH_LPSS_I2C_MEM_RESETS_APB)
          );
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS),
          1,
          (VOID *) (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS)
          );
        ///
        /// Enable Power Management Capability
        /// Enable IOSF Snoop
        ///
        PchMsgBusAndThenOr32AddToS3Save (
          PCH_LPSS_EP_PORT_ID,
          R_PCH_LPSS_FAB2CTLP3,
          Buffer32,
          (UINT32) ~(B_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL),
          (V_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL_SNP | B_PCH_LPSS_FABXCTLPX_FAB_PM_CAP_PRSNT),
          PCH_LPSS_EP_PRIVATE_READ_OPCODE,
          PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
        );
		//AMI_OVERRIDE - EIP144300 I2C port 3 IC_COMP_VERSION register not being set >>
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) ~(B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR)
          );
 	    //AMI_OVERRIDE - EIP144300 I2C port 3 IC_COMP_VERSION register not being set <<
        gDS->FreeMemorySpace (LpssMmioBase0, (UINT64) V_PCH_LPSS_I2C_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "LPSS2 I2C 2 not present, skipping.\n"));
      PchPlatformPolicy->LpssConfig->I2C2Enabled = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS2_FUNC3;
    }
  }

  ///
  /// LPSS2 I2C 3
  ///
  LpssPciMmBase = MmPciAddress (
                    0,
                    PchPlatformPolicy->BusNumber,
                    PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
                    PCI_FUNCTION_NUMBER_PCH_LPSS_I2C3,
                    0
                    );
  if (PchPlatformPolicy->LpssConfig->I2C3Enabled == PCH_DEVICE_DISABLE) {
    ///
    /// Put device in D3Hot state before disabling it.
    ///
    DEBUG ((EFI_D_INFO, "Putting LPSS2 I2C 3 into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS), B_PCH_LPSS_I2C_PCS_PS);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS),
      1,
      (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS)
      );

    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS2_FUNC4;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (LpssPciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_LPSS_I2C_BAR_ALIGNMENT,
                      V_PCH_LPSS_I2C_BAR_SIZE,
                      &LpssMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) ~(B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((LpssMmioBase0 & B_PCH_LPSS_I2C_BAR_BA) == LpssMmioBase0) && (LpssMmioBase0 != 0));
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (LpssMmioBase0 & B_PCH_LPSS_I2C_BAR_BA));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) (B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ASSERT (MmioRead32 ((UINTN) LpssMmioBase0) != 0xFFFFFFFF);
        ///
        /// Release Resets
        ///
        MmioWrite32 (
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS),
          (UINT32) (B_PCH_LPSS_I2C_MEM_RESETS_FUNC | B_PCH_LPSS_I2C_MEM_RESETS_APB)
          );
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS),
          1,
          (VOID *) (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS)
          );
        ///
        /// Enable Power Management Capability
        /// Enable IOSF Snoop
        ///
        PchMsgBusAndThenOr32AddToS3Save (
          PCH_LPSS_EP_PORT_ID,
          R_PCH_LPSS_FAB2CTLP4,
          Buffer32,
          (UINT32) ~(B_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL),
          (V_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL_SNP | B_PCH_LPSS_FABXCTLPX_FAB_PM_CAP_PRSNT),
          PCH_LPSS_EP_PRIVATE_READ_OPCODE,
          PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) ~(B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR)
          );
        gDS->FreeMemorySpace (LpssMmioBase0, (UINT64) V_PCH_LPSS_I2C_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "LPSS2 I2C 3 not present, skipping.\n"));
      PchPlatformPolicy->LpssConfig->I2C3Enabled = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS2_FUNC4;
    }
  }

  ///
  /// LPSS2 I2C 4
  ///
  LpssPciMmBase = MmPciAddress (
                    0,
                    PchPlatformPolicy->BusNumber,
                    PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
                    PCI_FUNCTION_NUMBER_PCH_LPSS_I2C4,
                    0
                    );
  if (PchPlatformPolicy->LpssConfig->I2C4Enabled == PCH_DEVICE_DISABLE) {
    ///
    /// Put device in D3Hot state before disabling it.
    ///
    DEBUG ((EFI_D_INFO, "Putting LPSS2 I2C 4 into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS), B_PCH_LPSS_I2C_PCS_PS);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS),
      1,
      (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS)
      );

    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS2_FUNC5;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (LpssPciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_LPSS_I2C_BAR_ALIGNMENT,
                      V_PCH_LPSS_I2C_BAR_SIZE,
                      &LpssMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) ~(B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((LpssMmioBase0 & B_PCH_LPSS_I2C_BAR_BA) == LpssMmioBase0) && (LpssMmioBase0 != 0));
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (LpssMmioBase0 & B_PCH_LPSS_I2C_BAR_BA));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) (B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ASSERT (MmioRead32 ((UINTN) LpssMmioBase0) != 0xFFFFFFFF);
        ///
        /// Release Resets
        ///
        MmioWrite32 (
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS),
          (UINT32) (B_PCH_LPSS_I2C_MEM_RESETS_FUNC | B_PCH_LPSS_I2C_MEM_RESETS_APB)
          );
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS),
          1,
          (VOID *) (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS)
          );
        ///
        /// Enable Power Management Capability
        /// Enable IOSF Snoop
        ///
        PchMsgBusAndThenOr32AddToS3Save (
          PCH_LPSS_EP_PORT_ID,
          R_PCH_LPSS_FAB2CTLP5,
          Buffer32,
          (UINT32) ~(B_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL),
          (V_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL_SNP | B_PCH_LPSS_FABXCTLPX_FAB_PM_CAP_PRSNT),
          PCH_LPSS_EP_PRIVATE_READ_OPCODE,
          PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) ~(B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR)
          );
        gDS->FreeMemorySpace (LpssMmioBase0, (UINT64) V_PCH_LPSS_I2C_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "LPSS2 I2C 4 not present, skipping.\n"));
      PchPlatformPolicy->LpssConfig->I2C4Enabled = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS2_FUNC5;
    }
  }

  ///
  /// LPSS2 I2C 5
  ///
  LpssPciMmBase = MmPciAddress (
                    0,
                    PchPlatformPolicy->BusNumber,
                    PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
                    PCI_FUNCTION_NUMBER_PCH_LPSS_I2C5,
                    0
                    );
  if (PchPlatformPolicy->LpssConfig->I2C5Enabled == PCH_DEVICE_DISABLE) {
    ///
    /// Put device in D3Hot state before disabling it.
    ///
    DEBUG ((EFI_D_INFO, "Putting LPSS2 I2C 5 into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS), B_PCH_LPSS_I2C_PCS_PS);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS),
      1,
      (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS)
      );

    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS2_FUNC6;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (LpssPciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_LPSS_I2C_BAR_ALIGNMENT,
                      V_PCH_LPSS_I2C_BAR_SIZE,
                      &LpssMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) ~(B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((LpssMmioBase0 & B_PCH_LPSS_I2C_BAR_BA) == LpssMmioBase0) && (LpssMmioBase0 != 0));
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (LpssMmioBase0 & B_PCH_LPSS_I2C_BAR_BA));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) (B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ASSERT (MmioRead32 ((UINTN) LpssMmioBase0) != 0xFFFFFFFF);
        ///
        /// Release Resets
        ///
        MmioWrite32 (
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS),
          (UINT32) (B_PCH_LPSS_I2C_MEM_RESETS_FUNC | B_PCH_LPSS_I2C_MEM_RESETS_APB)
          );
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS),
          1,
          (VOID *) (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS)
          );
        ///
        /// Enable Power Management Capability
        /// Enable IOSF Snoop
        ///
        PchMsgBusAndThenOr32AddToS3Save (
          PCH_LPSS_EP_PORT_ID,
          R_PCH_LPSS_FAB2CTLP6,
          Buffer32,
          (UINT32) ~(B_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL),
          (V_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL_SNP | B_PCH_LPSS_FABXCTLPX_FAB_PM_CAP_PRSNT),
          PCH_LPSS_EP_PRIVATE_READ_OPCODE,
          PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) ~(B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR)
          );
        gDS->FreeMemorySpace (LpssMmioBase0, (UINT64) V_PCH_LPSS_I2C_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "LPSS2 I2C 5 not present, skipping.\n"));
      PchPlatformPolicy->LpssConfig->I2C5Enabled = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS2_FUNC6;
    }
  }

  ///
  /// LPSS2 I2C 6
  ///
  LpssPciMmBase = MmPciAddress (
                    0,
                    PchPlatformPolicy->BusNumber,
                    PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
                    PCI_FUNCTION_NUMBER_PCH_LPSS_I2C6,
                    0
                    );
  if (PchPlatformPolicy->LpssConfig->I2C6Enabled == PCH_DEVICE_DISABLE) {
    ///
    /// Put device in D3Hot state before disabling it.
    ///
    DEBUG ((EFI_D_INFO, "Putting LPSS2 I2C 6 into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS), B_PCH_LPSS_I2C_PCS_PS);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS),
      1,
      (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_PCS)
      );

    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS2_FUNC7;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (LpssPciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_LPSS_I2C_BAR_ALIGNMENT,
                      V_PCH_LPSS_I2C_BAR_SIZE,
                      &LpssMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) ~(B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((LpssMmioBase0 & B_PCH_LPSS_I2C_BAR_BA) == LpssMmioBase0) && (LpssMmioBase0 != 0));
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (LpssMmioBase0 & B_PCH_LPSS_I2C_BAR_BA));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) (B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ASSERT (MmioRead32 ((UINTN) LpssMmioBase0) != 0xFFFFFFFF);
        ///
        /// Release Resets
        ///
        MmioWrite32 (
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS),
          (UINT32) (B_PCH_LPSS_I2C_MEM_RESETS_FUNC | B_PCH_LPSS_I2C_MEM_RESETS_APB)
          );
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS),
          1,
          (VOID *) (UINTN) (LpssMmioBase0 + R_PCH_LPSS_I2C_MEM_RESETS)
          );
        ///
        /// Enable Power Management Capability
        /// Enable IOSF Snoop
        ///
        PchMsgBusAndThenOr32AddToS3Save (
          PCH_LPSS_EP_PORT_ID,
          R_PCH_LPSS_FAB2CTLP7,
          Buffer32,
          (UINT32) ~(B_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL),
          (V_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL_SNP | B_PCH_LPSS_FABXCTLPX_FAB_PM_CAP_PRSNT),
          PCH_LPSS_EP_PRIVATE_READ_OPCODE,
          PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) ~(B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_BAR)
          );
        gDS->FreeMemorySpace (LpssMmioBase0, (UINT64) V_PCH_LPSS_I2C_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "LPSS2 I2C 6 not present, skipping.\n"));
      PchPlatformPolicy->LpssConfig->I2C6Enabled = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS2_FUNC7;
    }
  }

  ///
  /// LPSS1 DMA
  ///
  LpssPciMmBase = MmPciAddress (
                    0,
                    PchPlatformPolicy->BusNumber,
                    PCI_DEVICE_NUMBER_PCH_LPSS_DMAC0,
                    PCI_FUNCTION_NUMBER_PCH_LPSS_DMAC,
                    0
                    );
  ///
  /// LPSS1 DMA will be disabled under these two conditions:
  /// 1) When the device is instructed to be disabled.
  /// 2) When the rest of the LPSS1 devices are disabled.
  ///
  if ((PchPlatformPolicy->LpssConfig->Dma0Enabled == PCH_DEVICE_DISABLE) ||
      ((PchPlatformPolicy->LpssConfig->Pwm0Enabled == PCH_DEVICE_DISABLE) &&
       (PchPlatformPolicy->LpssConfig->Pwm1Enabled == PCH_DEVICE_DISABLE) &&
       (PchPlatformPolicy->LpssConfig->Hsuart0Enabled == PCH_DEVICE_DISABLE) &&
       (PchPlatformPolicy->LpssConfig->Hsuart1Enabled == PCH_DEVICE_DISABLE) &&
       (PchPlatformPolicy->LpssConfig->SpiEnabled == PCH_DEVICE_DISABLE))) {
    ///
    /// Disable the rest of the LPSS1 devices since LPSS1 DMA is disabled.
    ///
    PchPlatformPolicy->LpssConfig->Pwm0Enabled    = PCH_DEVICE_DISABLE;
    PchPlatformPolicy->LpssConfig->Pwm1Enabled    = PCH_DEVICE_DISABLE;
    PchPlatformPolicy->LpssConfig->Hsuart0Enabled = PCH_DEVICE_DISABLE;
    PchPlatformPolicy->LpssConfig->Hsuart1Enabled = PCH_DEVICE_DISABLE;
    PchPlatformPolicy->LpssConfig->SpiEnabled     = PCH_DEVICE_DISABLE;
    ///
    /// Put device in D3Hot state before disabling it.
    ///
    DEBUG ((EFI_D_INFO, "Putting LPSS1 DMA into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_PCS), B_PCH_LPSS_DMAC_PCS_PS);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_PCS),
      1,
      (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_PCS)
      );

    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS1_FUNC0;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (LpssPciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_LPSS_DMAC_BAR_ALIGNMENT,
                      V_PCH_LPSS_DMAC_BAR_SIZE,
                      &LpssMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD), (UINT32) ~(B_PCH_LPSS_DMAC_STSCMD_BME | B_PCH_LPSS_DMAC_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((LpssMmioBase0 & B_PCH_LPSS_DMAC_BAR_BA) == LpssMmioBase0) && (LpssMmioBase0 != 0));
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_BAR), (UINT32) (LpssMmioBase0 & B_PCH_LPSS_DMAC_BAR_BA));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_BAR)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD), (UINT32) (B_PCH_LPSS_DMAC_STSCMD_BME | B_PCH_LPSS_DMAC_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD)
          );
        ///
        /// Enable Power Management Capability
        /// Enable IOSF Snoop
        ///
        PchMsgBusAndThenOr32AddToS3Save (
          PCH_LPSS_EP_PORT_ID,
          R_PCH_LPSS_FABCTLP0,
          Buffer32,
          (UINT32) ~(B_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL),
          (V_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL_SNP | B_PCH_LPSS_FABXCTLPX_FAB_PM_CAP_PRSNT),
          PCH_LPSS_EP_PRIVATE_READ_OPCODE,
          PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD), (UINT32) ~(B_PCH_LPSS_DMAC_STSCMD_BME | B_PCH_LPSS_DMAC_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_BAR)
          );
        gDS->FreeMemorySpace (LpssMmioBase0, (UINT64) V_PCH_LPSS_DMAC_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "LPSS1 DMA not present, skipping.\n"));
      ///
      /// Disable the rest of the LPSS1 devices since LPSS1 DMA is disabled.
      ///
      PchPlatformPolicy->LpssConfig->Dma0Enabled    = PCH_DEVICE_DISABLE;
      PchPlatformPolicy->LpssConfig->Pwm0Enabled    = PCH_DEVICE_DISABLE;
      PchPlatformPolicy->LpssConfig->Pwm1Enabled    = PCH_DEVICE_DISABLE;
      PchPlatformPolicy->LpssConfig->Hsuart0Enabled = PCH_DEVICE_DISABLE;
      PchPlatformPolicy->LpssConfig->Hsuart1Enabled = PCH_DEVICE_DISABLE;
      PchPlatformPolicy->LpssConfig->SpiEnabled     = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS1_FUNC0;
    }
  }

  ///
  /// LPSS1 PWM 0
  ///
  LpssPciMmBase = MmPciAddress (
                    0,
                    PchPlatformPolicy->BusNumber,
                    PCI_DEVICE_NUMBER_PCH_LPSS_PWM,
                    PCI_FUNCTION_NUMBER_PCH_LPSS_PWM0,
                    0
                    );
  if (PchPlatformPolicy->LpssConfig->Pwm0Enabled == PCH_DEVICE_DISABLE) {
    ///
    /// Put device in D3Hot state before disabling it.
    ///
    DEBUG ((EFI_D_INFO, "Putting LPSS1 PWM 0 into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_PCS), B_PCH_LPSS_PWM_PCS_PS);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_PCS),
      1,
      (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_PCS)
      );

    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS1_FUNC1;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (LpssPciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_LPSS_PWM_BAR_ALIGNMENT,
                      V_PCH_LPSS_PWM_BAR_SIZE,
                      &LpssMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD), (UINT32) ~(B_PCH_LPSS_PWM_STSCMD_BME | B_PCH_LPSS_PWM_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((LpssMmioBase0 & B_PCH_LPSS_PWM_BAR_BA) == LpssMmioBase0) && (LpssMmioBase0 != 0));
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_BAR), (UINT32) (LpssMmioBase0 & B_PCH_LPSS_PWM_BAR_BA));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_BAR)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD), (UINT32) (B_PCH_LPSS_PWM_STSCMD_BME | B_PCH_LPSS_PWM_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD)
          );
        ASSERT (MmioRead32 ((UINTN) LpssMmioBase0) != 0xFFFFFFFF);
        ///
        /// Release Resets
        ///
        MmioWrite32 (
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_PWM_MEM_RESETS),
          (UINT32) (B_PCH_LPSS_PWM_MEM_RESETS_FUNC | B_PCH_LPSS_PWM_MEM_RESETS_APB)
          );
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_PWM_MEM_RESETS),
          1,
          (VOID *) (UINTN) (LpssMmioBase0 + R_PCH_LPSS_PWM_MEM_RESETS)
          );
        ///
        /// Enable Power Management Capability
        /// Enable IOSF Snoop
        ///
        PchMsgBusAndThenOr32AddToS3Save (
          PCH_LPSS_EP_PORT_ID,
          R_PCH_LPSS_FABCTLP1,
          Buffer32,
          (UINT32) ~(B_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL),
          (V_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL_SNP | B_PCH_LPSS_FABXCTLPX_FAB_PM_CAP_PRSNT),
          PCH_LPSS_EP_PRIVATE_READ_OPCODE,
          PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD), (UINT32) ~(B_PCH_LPSS_PWM_STSCMD_BME | B_PCH_LPSS_PWM_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_BAR)
          );
        gDS->FreeMemorySpace (LpssMmioBase0, (UINT64) V_PCH_LPSS_PWM_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "LPSS1 PWM 0 not present, skipping.\n"));
      PchPlatformPolicy->LpssConfig->Pwm0Enabled = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS1_FUNC1;
    }
  }

  ///
  /// LPSS1 PWM 1
  ///
  LpssPciMmBase = MmPciAddress (
                    0,
                    PchPlatformPolicy->BusNumber,
                    PCI_DEVICE_NUMBER_PCH_LPSS_PWM,
                    PCI_FUNCTION_NUMBER_PCH_LPSS_PWM1,
                    0
                    );
  if (PchPlatformPolicy->LpssConfig->Pwm1Enabled == PCH_DEVICE_DISABLE) {
    ///
    /// Put device in D3Hot state before disabling it.
    ///
    DEBUG ((EFI_D_INFO, "Putting LPSS1 PWM 1 into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_PCS), B_PCH_LPSS_PWM_PCS_PS);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_PCS),
      1,
      (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_PCS)
      );

    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS1_FUNC2;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (LpssPciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_LPSS_PWM_BAR_ALIGNMENT,
                      V_PCH_LPSS_PWM_BAR_SIZE,
                      &LpssMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD), (UINT32) ~(B_PCH_LPSS_PWM_STSCMD_BME | B_PCH_LPSS_PWM_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((LpssMmioBase0 & B_PCH_LPSS_PWM_BAR_BA) == LpssMmioBase0) && (LpssMmioBase0 != 0));
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_BAR), (UINT32) (LpssMmioBase0 & B_PCH_LPSS_PWM_BAR_BA));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_BAR)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD), (UINT32) (B_PCH_LPSS_PWM_STSCMD_BME | B_PCH_LPSS_PWM_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD)
          );
        ASSERT (MmioRead32 ((UINTN) LpssMmioBase0) != 0xFFFFFFFF);
        ///
        /// Release Resets
        ///
        MmioWrite32 (
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_PWM_MEM_RESETS),
          (UINT32) (B_PCH_LPSS_PWM_MEM_RESETS_FUNC | B_PCH_LPSS_PWM_MEM_RESETS_APB)
          );
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_PWM_MEM_RESETS),
          1,
          (VOID *) (UINTN) (LpssMmioBase0 + R_PCH_LPSS_PWM_MEM_RESETS)
          );
        ///
        /// Enable Power Management Capability
        /// Enable IOSF Snoop
        ///
        PchMsgBusAndThenOr32AddToS3Save (
          PCH_LPSS_EP_PORT_ID,
          R_PCH_LPSS_FABCTLP2,
          Buffer32,
          (UINT32) ~(B_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL),
          (V_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL_SNP | B_PCH_LPSS_FABXCTLPX_FAB_PM_CAP_PRSNT),
          PCH_LPSS_EP_PRIVATE_READ_OPCODE,
          PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD), (UINT32) ~(B_PCH_LPSS_PWM_STSCMD_BME | B_PCH_LPSS_PWM_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_BAR)
          );
        gDS->FreeMemorySpace (LpssMmioBase0, (UINT64) V_PCH_LPSS_PWM_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "LPSS1 PWM 1 not present, skipping.\n"));
      PchPlatformPolicy->LpssConfig->Pwm1Enabled = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS1_FUNC2;
    }
  }

  ///
  /// LPSS1 HS-UART 0
  ///
  LpssPciMmBase = MmPciAddress (
                    0,
                    PchPlatformPolicy->BusNumber,
                    PCI_DEVICE_NUMBER_PCH_LPSS_HSUART,
                    PCI_FUNCTION_NUMBER_PCH_LPSS_HSUART0,
                    0
                    );
  if (PchPlatformPolicy->LpssConfig->Hsuart0Enabled == PCH_DEVICE_DISABLE) {
    ///
    /// Put device in D3Hot state before disabling it.
    ///
    DEBUG ((EFI_D_INFO, "Putting LPSS1 HS-UART 0 into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_PCS), B_PCH_LPSS_HSUART_PCS_PS);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_PCS),
      1,
      (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_PCS)
      );

    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS1_FUNC3;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (LpssPciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_LPSS_HSUART_BAR_ALIGNMENT,
                      V_PCH_LPSS_HSUART_BAR_SIZE,
                      &LpssMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD), (UINT32) ~(B_PCH_LPSS_HSUART_STSCMD_BME | B_PCH_LPSS_HSUART_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((LpssMmioBase0 & B_PCH_LPSS_HSUART_BAR_BA) == LpssMmioBase0) && (LpssMmioBase0 != 0));
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_BAR), (UINT32) (LpssMmioBase0 & B_PCH_LPSS_HSUART_BAR_BA));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_BAR)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD), (UINT32) (B_PCH_LPSS_HSUART_STSCMD_BME | B_PCH_LPSS_HSUART_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD)
          );
        ASSERT (MmioRead32 ((UINTN) LpssMmioBase0) != 0xFFFFFFFF);
        ///
        /// Program Divider & Activate Clock
        /// N = 15625
        /// M = 6912
        /// M/N = 0.442368
        /// Functional Clock = 100 MHz * 0.442368 = 44.2368 MHz
        ///
        MmioWrite32 (
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_HSUART_MEM_PCP),
          (B_PCH_LPSS_HSUART_MEM_PCP_CLKUPDATE |
           (0x3D09 << 16) | /// N value
           (0x1B00 << 1) |  /// M value
           B_PCH_LPSS_HSUART_MEM_PCP_CLKEN)
          );
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_HSUART_MEM_PCP),
          1,
          (VOID *) (UINTN) (LpssMmioBase0 + R_PCH_LPSS_HSUART_MEM_PCP)
          );
        ///
        /// Release Resets
        ///
        MmioWrite32 (
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_HSUART_MEM_RESETS),
          (UINT32) (B_PCH_LPSS_HSUART_MEM_RESETS_FUNC | B_PCH_LPSS_HSUART_MEM_RESETS_APB)
          );
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_HSUART_MEM_RESETS),
          1,
          (VOID *) (UINTN) (LpssMmioBase0 + R_PCH_LPSS_HSUART_MEM_RESETS)
          );
        ///
        /// Enable Power Management Capability
        /// Enable IOSF Snoop
        ///
        PchMsgBusAndThenOr32AddToS3Save (
          PCH_LPSS_EP_PORT_ID,
          R_PCH_LPSS_FABCTLP3,
          Buffer32,
          (UINT32) ~(B_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL),
          (V_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL_SNP | B_PCH_LPSS_FABXCTLPX_FAB_PM_CAP_PRSNT),
          PCH_LPSS_EP_PRIVATE_READ_OPCODE,
          PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD), (UINT32) ~(B_PCH_LPSS_HSUART_STSCMD_BME | B_PCH_LPSS_HSUART_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_BAR)
          );
        gDS->FreeMemorySpace (LpssMmioBase0, (UINT64) V_PCH_LPSS_HSUART_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "LPSS1 HS-UART 0 not present, skipping.\n"));
      PchPlatformPolicy->LpssConfig->Hsuart0Enabled = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS1_FUNC3;
    }
  }

  ///
  /// LPSS1 HS-UART 1
  ///
  LpssPciMmBase = MmPciAddress (
                    0,
                    PchPlatformPolicy->BusNumber,
                    PCI_DEVICE_NUMBER_PCH_LPSS_HSUART,
                    PCI_FUNCTION_NUMBER_PCH_LPSS_HSUART1,
                    0
                    );
  if (PchPlatformPolicy->LpssConfig->Hsuart1Enabled == PCH_DEVICE_DISABLE) {
    ///
    /// Put device in D3Hot state before disabling it.
    ///
    DEBUG ((EFI_D_INFO, "Putting LPSS1 HS-UART 1 into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_PCS), B_PCH_LPSS_HSUART_PCS_PS);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_PCS),
      1,
      (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_PCS)
      );
    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS1_FUNC4;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (LpssPciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_LPSS_HSUART_BAR_ALIGNMENT,
                      V_PCH_LPSS_HSUART_BAR_SIZE,
                      &LpssMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD), (UINT32) ~(B_PCH_LPSS_HSUART_STSCMD_BME | B_PCH_LPSS_HSUART_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((LpssMmioBase0 & B_PCH_LPSS_HSUART_BAR_BA) == LpssMmioBase0) && (LpssMmioBase0 != 0));
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_BAR), (UINT32) (LpssMmioBase0 & B_PCH_LPSS_HSUART_BAR_BA));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_BAR)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD), (UINT32) (B_PCH_LPSS_HSUART_STSCMD_BME | B_PCH_LPSS_HSUART_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD)
          );
        ASSERT (MmioRead32 ((UINTN) LpssMmioBase0) != 0xFFFFFFFF);
        ///
        /// Program Divider & Activate Clock
        /// N = 15625
        /// M = 6912
        /// M/N = 0.442368
        /// Functional Clock = 100 MHz * 0.442368 = 44.2368 MHz
        ///
        MmioWrite32 (
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_HSUART_MEM_PCP),
          (B_PCH_LPSS_HSUART_MEM_PCP_CLKUPDATE |
           (0x3D09 << 16) | /// N value
           (0x1B00 << 1) |  /// M value
           B_PCH_LPSS_HSUART_MEM_PCP_CLKEN)
          );
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_HSUART_MEM_PCP),
          1,
          (VOID *) (UINTN) (LpssMmioBase0 + R_PCH_LPSS_HSUART_MEM_PCP)
          );
        ///
        /// Release Resets
        ///
        MmioWrite32 (
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_HSUART_MEM_RESETS),
          (UINT32) (B_PCH_LPSS_HSUART_MEM_RESETS_FUNC | B_PCH_LPSS_HSUART_MEM_RESETS_APB)
          );
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_HSUART_MEM_RESETS),
          1,
          (VOID *) (UINTN) (LpssMmioBase0 + R_PCH_LPSS_HSUART_MEM_RESETS)
          );
        ///
        /// Enable Power Management Capability
        /// Enable IOSF Snoop
        ///
        PchMsgBusAndThenOr32AddToS3Save (
          PCH_LPSS_EP_PORT_ID,
          R_PCH_LPSS_FABCTLP4,
          Buffer32,
          (UINT32) ~(B_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL),
          (V_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL_SNP | B_PCH_LPSS_FABXCTLPX_FAB_PM_CAP_PRSNT),
          PCH_LPSS_EP_PRIVATE_READ_OPCODE,
          PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD), (UINT32) ~(B_PCH_LPSS_HSUART_STSCMD_BME | B_PCH_LPSS_HSUART_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_BAR)
          );
        gDS->FreeMemorySpace (LpssMmioBase0, (UINT64) V_PCH_LPSS_HSUART_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "LPSS1 HS-UART 1 not present, skipping.\n"));
      PchPlatformPolicy->LpssConfig->Hsuart1Enabled = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS1_FUNC4;
    }
  }

  ///
  /// LPSS1 SPI
  ///
  LpssPciMmBase = MmPciAddress (
                    0,
                    PchPlatformPolicy->BusNumber,
                    PCI_DEVICE_NUMBER_PCH_LPSS_SPI,
                    PCI_FUNCTION_NUMBER_PCH_LPSS_SPI,
                    0
                    );
  if (PchPlatformPolicy->LpssConfig->SpiEnabled == PCH_DEVICE_DISABLE) {
    ///
    /// Put device in D3Hot state before disabling it.
    ///
    DEBUG ((EFI_D_INFO, "Putting LPSS1 SPI into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_PCS), B_PCH_LPSS_SPI_PCS_PS);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_PCS),
      1,
      (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_PCS)
      );
    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS1_FUNC5;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (LpssPciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_LPSS_SPI_BAR_ALIGNMENT,
                      V_PCH_LPSS_SPI_BAR_SIZE,
                      &LpssMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_STSCMD), (UINT32) ~(B_PCH_LPSS_SPI_STSCMD_BME | B_PCH_LPSS_SPI_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((LpssMmioBase0 & B_PCH_LPSS_SPI_BAR_BA) == LpssMmioBase0) && (LpssMmioBase0 != 0));
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_BAR), (UINT32) (LpssMmioBase0 & B_PCH_LPSS_SPI_BAR_BA));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_BAR)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_STSCMD), (UINT32) (B_PCH_LPSS_SPI_STSCMD_BME | B_PCH_LPSS_SPI_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_STSCMD)
          );
        ASSERT (MmioRead32 ((UINTN) LpssMmioBase0) != 0xFFFFFFFF);
        ///
        /// Program Divider & Activate Clock
        /// N = 2
        /// M = 1
        /// M/N = 0.5
        /// Functional Clock = 100 MHz * 0.5 = 50 MHz
        ///
        MmioWrite32 (
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_SPI_MEM_PCP),
          (B_PCH_LPSS_SPI_MEM_PCP_CLKUPDATE |
           (0x0002 << 16) | /// N value
           (0x0001 << 1) |  /// M value
           B_PCH_LPSS_SPI_MEM_PCP_CLKEN)
          );
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_SPI_MEM_PCP),
          1,
          (VOID *) (UINTN) (LpssMmioBase0 + R_PCH_LPSS_SPI_MEM_PCP)
          );
        ///
        /// Release Resets
        ///
        MmioWrite32 (
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_SPI_MEM_RESETS),
          (UINT32) (B_PCH_LPSS_SPI_MEM_RESETS_FUNC | B_PCH_LPSS_SPI_MEM_RESETS_APB)
          );
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssMmioBase0 + R_PCH_LPSS_SPI_MEM_RESETS),
          1,
          (VOID *) (UINTN) (LpssMmioBase0 + R_PCH_LPSS_SPI_MEM_RESETS)
          );
        ///
        /// Enable Power Management Capability
        /// Enable IOSF Snoop
        ///
        PchMsgBusAndThenOr32AddToS3Save (
          PCH_LPSS_EP_PORT_ID,
          R_PCH_LPSS_FABCTLP5,
          Buffer32,
          (UINT32) ~(B_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL),
          (V_PCH_LPSS_FABXCTLPX_FABCTLSNTCSEL_SNP | B_PCH_LPSS_FABXCTLPX_FAB_PM_CAP_PRSNT),
          PCH_LPSS_EP_PRIVATE_READ_OPCODE,
          PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_STSCMD), (UINT32) ~(B_PCH_LPSS_SPI_STSCMD_BME | B_PCH_LPSS_SPI_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_STSCMD),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_BAR),
          1,
          (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_BAR)
          );
        gDS->FreeMemorySpace (LpssMmioBase0, (UINT64) V_PCH_LPSS_SPI_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "LPSS1 SPI not present, skipping.\n"));
      PchPlatformPolicy->LpssConfig->SpiEnabled = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS1_FUNC5;
    }
  }

  ///
  /// LPSS1 Spare 0 & Spare 1
  /// Not used, so let's disable them.
  ///
  *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS1_FUNC6;
  *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPSS1_FUNC7;

  DEBUG ((EFI_D_INFO, "ConfigureLpss() End\n"));

  return EFI_SUCCESS;
}


EFI_STATUS
ConfigureLpssAtBoot (
  IN DXE_PCH_PLATFORM_POLICY_PROTOCOL *PchPlatformPolicy
  )
/**

  @brief
  Hide PCI config space of LPSS devices and do any final initialization.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  UINTN                                     LpssPciMmBase;
  UINT32                                    LpssDMA0Bar0;//for CSRT
  UINT32                                    LpssDMA1Bar0;//for CSRT
  UINT32                                    Buffer32;
  EFI_STATUS                                AcpiTablePresent;
  EFI_STATUS                                Status;
  UINT64                                    Data64;
  UINTN                                     AcpiTableKey;
  UINT8                                     Index;
  EFI_ACPI_TABLE_PROTOCOL                   *AcpiTable;//for CSRT
  EFI_PHYSICAL_ADDRESS                      RangeMin;
  EFI_PHYSICAL_ADDRESS                      RangeMax;
  EFI_GLOBAL_NVS_AREA_PROTOCOL              *GlobalNvsArea;

  DEBUG ((EFI_D_INFO, "ConfigureLpssAtBoot() Start\n"));

  ///
  /// Initialize Variables
  ///
  LpssPciMmBase            = 0;
  LpssDMA0Bar0             = 0;
  LpssDMA1Bar0             = 0;
  Buffer32                 = 0;
  AcpiTablePresent         = EFI_NOT_FOUND;
  AcpiTableKey             = 0;
  Data64                   = 0;
  Index                    = 0;
  Status                   = EFI_SUCCESS;
  AcpiTable                = NULL;

  ///
  /// Locate ACPI table
  ///
  AcpiTablePresent = InitializePchAslUpdateLib ();
  ///
  /// Update LPSS devices ACPI variables
  ///
  if (!EFI_ERROR (AcpiTablePresent)) {
      if (PchPlatformPolicy->LpssConfig->LpssPciModeEnabled == 0) {     // ACPI mode
//AMI_OVERRIDE - Update PciBottom and PciTop Address correctly - CSP20130723 >>	
//    	Status = GetPCIAddrRange(&RangeMin, &RangeMax);
//    	if (EFI_ERROR(Status)) {
//    	  return Status;
//    	}
//AMI_OVERRIDE - Update PciBottom and PciTop Address correctly - CSP20130723<<		
      Status = gBS->LocateProtocol (
                      &gEfiGlobalNvsAreaProtocolGuid,
                      NULL,
                      (VOID **) &GlobalNvsArea
                      );
      if (EFI_ERROR(Status)) {
        return Status;
      }
//AMI_OVERRIDE - Update PciBottom and PciTop Address correctly - CSP20130723>>
    	Status = GetPCIAddrRange(&RangeMin, &RangeMax);
    	if (EFI_ERROR(Status)) {
    	  GlobalNvsArea->Area->PCIBottomAddress = GlobalNvsArea->Area->BmBound;
    	  GlobalNvsArea->Area->PCITopAddress = (UINT32)0xDFFFFFFF; //AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base
    	} else {
    	  GlobalNvsArea->Area->PCIBottomAddress = (UINT32)RangeMin;
    	  GlobalNvsArea->Area->PCITopAddress = (UINT32)RangeMax - 1;	
    	}
//AMI_OVERRIDE - Update PciBottom and PciTop Address correctly - CSP20130723<<
      UpdateLpssSccDeviceList();
      UpdateLpssSccDeviceInfo(GlobalNvsArea);
      }
    ///
    /// LPSS1 DMA
    ///
    LpssPciMmBase = MmPciAddress (
                      0,
                      PchPlatformPolicy->BusNumber,
                      PCI_DEVICE_NUMBER_PCH_LPSS_DMAC0,
                      PCI_FUNCTION_NUMBER_PCH_LPSS_DMAC,
                      0
                      );
    LpssDMA0Bar0 = MmioRead32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_BAR)) & B_PCH_LPSS_DMAC_BAR_BA;
    if ((PchPlatformPolicy->LpssConfig->Dma0Enabled == 1) && (PchPlatformPolicy->LpssConfig->LpssPciModeEnabled == 0)) {
      DEBUG ((EFI_D_INFO, "Switching LPSS1 DMA into ACPI Mode.\n"));
      ///
      /// Disable PCI Interrupt, Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 (
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD),
        (UINT32) (B_PCH_LPSS_DMAC_STSCMD_INTRDIS | B_PCH_LPSS_DMAC_STSCMD_BME | B_PCH_LPSS_DMAC_STSCMD_MSE)
        );
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD),
        1,
        (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD)
        );


      PchMsgBusAndThenOr32AddToS3Save (
        PCH_LPSS_EP_PORT_ID,
        R_PCH_LPSS_FABCTLP0,
        Buffer32,
        0xFFFFFFFF,
        (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS),
        PCH_LPSS_EP_PRIVATE_READ_OPCODE,
        PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
        );
    }

    ///
    /// LPSS2 DMA
    ///
    LpssPciMmBase = MmPciAddress (
                      0,
                      PchPlatformPolicy->BusNumber,
                      PCI_DEVICE_NUMBER_PCH_LPSS_DMAC1,
                      PCI_FUNCTION_NUMBER_PCH_LPSS_DMAC,
                      0
                      );
    LpssDMA1Bar0 = MmioRead32 ((UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_BAR)) & B_PCH_LPSS_DMAC_BAR_BA;
    if ((PchPlatformPolicy->LpssConfig->Dma1Enabled == 1) &&(PchPlatformPolicy->LpssConfig->LpssPciModeEnabled == 0)) {
      DEBUG ((EFI_D_INFO, "Switching LPSS2 DMA into ACPI Mode.\n"));
      ///
      /// Disable PCI Interrupt, Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 (
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD),
        (UINT32) (B_PCH_LPSS_DMAC_STSCMD_INTRDIS | B_PCH_LPSS_DMAC_STSCMD_BME | B_PCH_LPSS_DMAC_STSCMD_MSE)
        );
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD),
        1,
        (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_DMAC_STSCMD)
        );

      PchMsgBusAndThenOr32AddToS3Save (
        PCH_LPSS_EP_PORT_ID,
        R_PCH_LPSS_FAB2CTLP0,
        Buffer32,
        0xFFFFFFFF,
        (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS),
        PCH_LPSS_EP_PRIVATE_READ_OPCODE,
        PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
        );
    }


    //CSRT: setup and assing base address
    if (PchPlatformPolicy->LpssConfig->Dma0Enabled == 1 && PchPlatformPolicy->LpssConfig->Dma1Enabled == 1) {
      EFI_ACPI_CSRT_TABLE              *mCsrt = NULL;

      DEBUG ((EFI_D_INFO, "Initialize CSRT Start\n"));

      Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID **) &AcpiTable);

      if(EFI_ERROR(Status)) {
        return Status;
      }

      mCsrt = AllocateZeroPool(sizeof(EFI_ACPI_CSRT_TABLE));

      if(mCsrt == NULL) {
        return EFI_BUFFER_TOO_SMALL;
      }

      mCsrt->Header.Signature       = EFI_ACPI_5_0_CORE_SYSTEM_RESOURCE_TABLE_SIGNATURE;
      mCsrt->Header.Length          = sizeof(EFI_ACPI_CSRT_TABLE);
      mCsrt->Header.Revision        = EFI_ACPI_CSRT_TABLE_REVISION;
      mCsrt->Header.Checksum        = 0;
      //mCsrt->Header.OemId         = SIGNATURE_64(EFI_ACPI_OEM_ID);
      CopyMem(&mCsrt->Header.OemId, EFI_ACPI_OEM_ID, 6);
      mCsrt->Header.OemTableId      = EFI_ACPI_OEM_TABLE_ID;
      mCsrt->Header.OemRevision     = EFI_ACPI_OEM_REVISION;
      mCsrt->Header.CreatorId       = EFI_ACPI_CREATOR_ID;
      mCsrt->Header.CreatorRevision = EFI_ACPI_CREATOR_REVISION;

      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.Length           = sizeof(RESOURCE_GROUP_INFO1);
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.VendorId         = 0x4C544E49;
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.SubVendorId      = 0x00000000;
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.DeviceId         = 0x9C60;//fix me, LPT-LP 0x9C60;
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.SubDeviceId      = 0x0000;
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.Revision         = 0x0002;
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.Reserved         = 0x0000;
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.SharedInfoLength                           =  sizeof(SHARED_INFO_SECTION);
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.SharedInfoSection.MajVersion               = 0x0001;
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.SharedInfoSection.MinVersion               = 0x0000;

      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.SharedInfoSection.MMIOLowPart      = LpssDMA0Bar0;
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.SharedInfoSection.MMIOHighPart         = 0x00000000;
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.SharedInfoSection.IntGSI                   = 0x0000002A;
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.SharedInfoSection.IntPol                   = 0x02;
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.SharedInfoSection.IntMode                  = 0x00;
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.SharedInfoSection.NoOfCh                   = MAX_NO_CHANNEL1_SUPPORTED - 1;
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.SharedInfoSection.DMAAddressWidth          = 0x20;
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.SharedInfoSection.BaseReqLine              = 0x0000;
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.SharedInfoSection.NoOfHandSig              = 0x0010;
      mCsrt->ResourceGroupsInfo1.ResourceGroupHeaderInfo.SharedInfoSection.MaxBlockTransferSize = 0x0000FFF;

      mCsrt->ResourceGroupsInfo1.ResourceDescriptorInfo[0].Length                 = 0x0000000C;
      mCsrt->ResourceGroupsInfo1.ResourceDescriptorInfo[0].ResourceType           = 0x0003;
      mCsrt->ResourceGroupsInfo1.ResourceDescriptorInfo[0].ResourceSubType        = 0x0001;
      mCsrt->ResourceGroupsInfo1.ResourceDescriptorInfo[0].UUID                   = 0x20495053;     //SPI

      for(Index = 1; Index < MAX_NO_CHANNEL1_SUPPORTED; Index++) {
        mCsrt->ResourceGroupsInfo1.ResourceDescriptorInfo[Index].Length          = 0x0000000C;
        mCsrt->ResourceGroupsInfo1.ResourceDescriptorInfo[Index].ResourceType    = 0x0003;
        mCsrt->ResourceGroupsInfo1.ResourceDescriptorInfo[Index].ResourceSubType = 0x0000;
        mCsrt->ResourceGroupsInfo1.ResourceDescriptorInfo[Index].UUID            = 0x30414843 + ((Index - 1) << 24) ;    //CHAn
      }

      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.Length          = sizeof(RESOURCE_GROUP_INFO2);
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.VendorId        = 0x4C544E49;
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.SubVendorId     = 0x00000000;
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.DeviceId        = 0x9C60;//fix me LPT-LP 0x9C60;
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.SubDeviceId     = 0x0000;
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.Revision        = 0x0003;
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.Reserved        = 0x0000;
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.SharedInfoLength              =  sizeof(SHARED_INFO_SECTION);
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.SharedInfoSection.MajVersion  = 0x0001;
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.SharedInfoSection.MinVersion  = 0x0000;

      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.SharedInfoSection.MMIOLowPart = LpssDMA1Bar0;
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.SharedInfoSection.MMIOHighPart= 0x00000000;
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.SharedInfoSection.IntGSI      = 0x0000002B;
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.SharedInfoSection.IntPol      = 0x02;
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.SharedInfoSection.IntMode     = 0x00;
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.SharedInfoSection.NoOfCh      = MAX_NO_CHANNEL2_SUPPORTED - 1;
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.SharedInfoSection.DMAAddressWidth      = 0x20;
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.SharedInfoSection.BaseReqLine          = 0x0010;
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.SharedInfoSection.NoOfHandSig          = 0x0010;
      mCsrt->ResourceGroupsInfo2.ResourceGroupHeaderInfo.SharedInfoSection.MaxBlockTransferSize = 0x0000FFF;

      mCsrt->ResourceGroupsInfo2.ResourceDescriptorInfo[0].Length            = 0x0000000C;
      mCsrt->ResourceGroupsInfo2.ResourceDescriptorInfo[0].ResourceType      = 0x0003;
      mCsrt->ResourceGroupsInfo2.ResourceDescriptorInfo[0].ResourceSubType   = 0x0001;
      mCsrt->ResourceGroupsInfo2.ResourceDescriptorInfo[0].UUID              = 0x20433249;  //I2C

      for(Index = 1; Index < MAX_NO_CHANNEL2_SUPPORTED; Index++) {
        mCsrt->ResourceGroupsInfo2.ResourceDescriptorInfo[Index].Length            = 0x0000000C;
        mCsrt->ResourceGroupsInfo2.ResourceDescriptorInfo[Index].ResourceType      = 0x0003;
        mCsrt->ResourceGroupsInfo2.ResourceDescriptorInfo[Index].ResourceSubType   = 0x0000;
        mCsrt->ResourceGroupsInfo2.ResourceDescriptorInfo[Index].UUID              = 0x30414843 + ((Index - 1) << 24) ;  //CHAn
      }

      Status = AcpiTable->InstallAcpiTable (AcpiTable, mCsrt, mCsrt->Header.Length, &AcpiTableKey);
      DEBUG ((EFI_D_INFO, "Initialize CSRT End,LpssDMA0Bar0=%x,LpssDMA1Bar0=%x\n",LpssDMA0Bar0,LpssDMA1Bar0));
    }


    ///
    /// LPSS2 I2C 0
    ///
    if ((PchPlatformPolicy->LpssConfig->I2C0Enabled == 1) && (PchPlatformPolicy->LpssConfig->LpssPciModeEnabled == 0)) {
      DEBUG ((EFI_D_INFO, "Switching LPSS2 I2C 0 into ACPI Mode.\n"));
      LpssPciMmBase = MmPciAddress (
                        0,
                        PchPlatformPolicy->BusNumber,
                        PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
                        PCI_FUNCTION_NUMBER_PCH_LPSS_I2C0,
                        0
                        );
      ///
      /// Disable PCI Interrupt, Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 (
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
        (UINT32) (B_PCH_LPSS_I2C_STSCMD_INTRDIS | B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE)
        );
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
        1,
        (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
        );

      PchMsgBusAndThenOr32AddToS3Save (
        PCH_LPSS_EP_PORT_ID,
        R_PCH_LPSS_FAB2CTLP1,
        Buffer32,
        0xFFFFFFFF,
        (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS),
        PCH_LPSS_EP_PRIVATE_READ_OPCODE,
        PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
        );
    }

    ///
    /// LPSS2 I2C 1
    ///
    if ((PchPlatformPolicy->LpssConfig->I2C1Enabled == 1) && (PchPlatformPolicy->LpssConfig->LpssPciModeEnabled == 0)) {
      DEBUG ((EFI_D_INFO, "Switching LPSS2 I2C 1 into ACPI Mode.\n"));
      LpssPciMmBase = MmPciAddress (
                        0,
                        PchPlatformPolicy->BusNumber,
                        PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
                        PCI_FUNCTION_NUMBER_PCH_LPSS_I2C1,
                        0
                        );
      ///
      /// Disable PCI Interrupt, Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 (
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
        (UINT32) (B_PCH_LPSS_I2C_STSCMD_INTRDIS | B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE)
        );
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
        1,
        (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
        );

      PchMsgBusAndThenOr32AddToS3Save (
        PCH_LPSS_EP_PORT_ID,
        R_PCH_LPSS_FAB2CTLP2,
        Buffer32,
        0xFFFFFFFF,
        (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS),
        PCH_LPSS_EP_PRIVATE_READ_OPCODE,
        PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
        );
    }

    ///
    /// LPSS2 I2C 2
    ///
    if ((PchPlatformPolicy->LpssConfig->I2C2Enabled == 1) && (PchPlatformPolicy->LpssConfig->LpssPciModeEnabled == 0)) {
      DEBUG ((EFI_D_INFO, "Switching LPSS2 I2C 2 into ACPI Mode.\n"));
      LpssPciMmBase = MmPciAddress (
                        0,
                        PchPlatformPolicy->BusNumber,
                        PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
                        PCI_FUNCTION_NUMBER_PCH_LPSS_I2C2,
                        0
                        );
      ///
      /// Disable PCI Interrupt, Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 (
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
        (UINT32) (B_PCH_LPSS_I2C_STSCMD_INTRDIS | B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE)
        );
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
        1,
        (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
        );

      PchMsgBusAndThenOr32AddToS3Save (
        PCH_LPSS_EP_PORT_ID,
        R_PCH_LPSS_FAB2CTLP3,
        Buffer32,
        0xFFFFFFFF,
        (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS),
        PCH_LPSS_EP_PRIVATE_READ_OPCODE,
        PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
        );
    }

    ///
    /// LPSS2 I2C 3
    ///
    if ((PchPlatformPolicy->LpssConfig->I2C3Enabled == 1) && (PchPlatformPolicy->LpssConfig->LpssPciModeEnabled == 0)) {
      DEBUG ((EFI_D_INFO, "Switching LPSS2 I2C 3 into ACPI Mode.\n"));
      LpssPciMmBase = MmPciAddress (
                        0,
                        PchPlatformPolicy->BusNumber,
                        PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
                        PCI_FUNCTION_NUMBER_PCH_LPSS_I2C3,
                        0
                        );
      ///
      /// Disable PCI Interrupt, Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 (
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
        (UINT32) (B_PCH_LPSS_I2C_STSCMD_INTRDIS | B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE)
        );
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
        1,
        (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
        );

      PchMsgBusAndThenOr32AddToS3Save (
        PCH_LPSS_EP_PORT_ID,
        R_PCH_LPSS_FAB2CTLP4,
        Buffer32,
        0xFFFFFFFF,
        (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS),
        PCH_LPSS_EP_PRIVATE_READ_OPCODE,
        PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
        );
    }

    ///
    /// LPSS I2C 4
    ///
    if ((PchPlatformPolicy->LpssConfig->I2C4Enabled == 1) && (PchPlatformPolicy->LpssConfig->LpssPciModeEnabled == 0)) {
      DEBUG ((EFI_D_INFO, "Switching LPSS2 I2C 4 into ACPI Mode.\n"));
      LpssPciMmBase = MmPciAddress (
                        0,
                        PchPlatformPolicy->BusNumber,
                        PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
                        PCI_FUNCTION_NUMBER_PCH_LPSS_I2C4,
                        0
                        );
      ///
      /// Disable PCI Interrupt, Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 (
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
        (UINT32) (B_PCH_LPSS_I2C_STSCMD_INTRDIS | B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE)
        );
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
        1,
        (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
        );

      PchMsgBusAndThenOr32AddToS3Save (
        PCH_LPSS_EP_PORT_ID,
        R_PCH_LPSS_FAB2CTLP5,
        Buffer32,
        0xFFFFFFFF,
        (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS),
        PCH_LPSS_EP_PRIVATE_READ_OPCODE,
        PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
        );
    }

    ///
    /// LPSS2 I2C 5
    ///
    if ((PchPlatformPolicy->LpssConfig->I2C5Enabled == 1) && (PchPlatformPolicy->LpssConfig->LpssPciModeEnabled == 0)) {
      DEBUG ((EFI_D_INFO, "Switching LPSS2 I2C 5 into ACPI Mode.\n"));
      LpssPciMmBase = MmPciAddress (
                        0,
                        PchPlatformPolicy->BusNumber,
                        PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
                        PCI_FUNCTION_NUMBER_PCH_LPSS_I2C5,
                        0
                        );
      ///
      /// Disable PCI Interrupt, Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 (
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
        (UINT32) (B_PCH_LPSS_I2C_STSCMD_INTRDIS | B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE)
        );
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
        1,
        (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
        );

      PchMsgBusAndThenOr32AddToS3Save (
        PCH_LPSS_EP_PORT_ID,
        R_PCH_LPSS_FAB2CTLP6,
        Buffer32,
        0xFFFFFFFF,
        (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS),
        PCH_LPSS_EP_PRIVATE_READ_OPCODE,
        PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
        );
    }

    ///
    /// LPSS2 I2C 6
    ///
    if ((PchPlatformPolicy->LpssConfig->I2C6Enabled == 1) && (PchPlatformPolicy->LpssConfig->LpssPciModeEnabled == 0)) {
      DEBUG ((EFI_D_INFO, "Switching LPSS2 I2C 6 into ACPI Mode.\n"));
      LpssPciMmBase = MmPciAddress (
                        0,
                        PchPlatformPolicy->BusNumber,
                        PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
                        PCI_FUNCTION_NUMBER_PCH_LPSS_I2C6,
                        0
                        );
      ///
      /// Disable PCI Interrupt, Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 (
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
        (UINT32) (B_PCH_LPSS_I2C_STSCMD_INTRDIS | B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE)
        );
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD),
        1,
        (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_I2C_STSCMD)
        );

      PchMsgBusAndThenOr32AddToS3Save (
        PCH_LPSS_EP_PORT_ID,
        R_PCH_LPSS_FAB2CTLP7,
        Buffer32,
        0xFFFFFFFF,
        (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS),
        PCH_LPSS_EP_PRIVATE_READ_OPCODE,
        PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
        );
    }



    ///
    /// LPSS1 PWM 0
    ///
    if ((PchPlatformPolicy->LpssConfig->Pwm0Enabled == 1) && (PchPlatformPolicy->LpssConfig->LpssPciModeEnabled == 0)) {
      DEBUG ((EFI_D_INFO, "Switching LPSS1 PWM 0 into ACPI Mode.\n"));
      LpssPciMmBase = MmPciAddress (
                        0,
                        PchPlatformPolicy->BusNumber,
                        PCI_DEVICE_NUMBER_PCH_LPSS_PWM,
                        PCI_FUNCTION_NUMBER_PCH_LPSS_PWM0,
                        0
                        );
      ///
      /// Disable PCI Interrupt, Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 (
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD),
        (UINT32) (B_PCH_LPSS_PWM_STSCMD_INTRDIS | B_PCH_LPSS_PWM_STSCMD_BME | B_PCH_LPSS_PWM_STSCMD_MSE)
        );
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD),
        1,
        (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD)
        );

      PchMsgBusAndThenOr32AddToS3Save (
        PCH_LPSS_EP_PORT_ID,
        R_PCH_LPSS_FABCTLP1,
        Buffer32,
        0xFFFFFFFF,
        (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS),
        PCH_LPSS_EP_PRIVATE_READ_OPCODE,
        PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
        );
    }

    ///
    /// LPSS1 PWM 1
    ///
    if ((PchPlatformPolicy->LpssConfig->Pwm1Enabled == 1) && (PchPlatformPolicy->LpssConfig->LpssPciModeEnabled == 0)) {
      DEBUG ((EFI_D_INFO, "Switching LPSS1 PWM 1 into ACPI Mode.\n"));
      LpssPciMmBase = MmPciAddress (
                        0,
                        PchPlatformPolicy->BusNumber,
                        PCI_DEVICE_NUMBER_PCH_LPSS_PWM,
                        PCI_FUNCTION_NUMBER_PCH_LPSS_PWM1,
                        0
                        );
      ///
      /// Disable PCI Interrupt, Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 (
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD),
        (UINT32) (B_PCH_LPSS_PWM_STSCMD_INTRDIS | B_PCH_LPSS_PWM_STSCMD_BME | B_PCH_LPSS_PWM_STSCMD_MSE)
        );
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD),
        1,
        (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_PWM_STSCMD)
        );

      PchMsgBusAndThenOr32AddToS3Save (
        PCH_LPSS_EP_PORT_ID,
        R_PCH_LPSS_FABCTLP2,
        Buffer32,
        0xFFFFFFFF,
        (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS),
        PCH_LPSS_EP_PRIVATE_READ_OPCODE,
        PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
        );
    }

    ///
    /// LPSS1 HS-UART 0
    ///
    if ((PchPlatformPolicy->LpssConfig->Hsuart0Enabled == 1) && (PchPlatformPolicy->LpssConfig->LpssPciModeEnabled == 0)) {
      DEBUG ((EFI_D_INFO, "Switching LPSS1 HS-UART 0 into ACPI Mode.\n"));
      LpssPciMmBase = MmPciAddress (
                        0,
                        PchPlatformPolicy->BusNumber,
                        PCI_DEVICE_NUMBER_PCH_LPSS_HSUART,
                        PCI_FUNCTION_NUMBER_PCH_LPSS_HSUART0,
                        0
                        );

      ///
      /// Disable PCI Interrupt, Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 (
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD),
        (UINT32) (B_PCH_LPSS_HSUART_STSCMD_INTRDIS | B_PCH_LPSS_HSUART_STSCMD_BME | B_PCH_LPSS_HSUART_STSCMD_MSE)
        );
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD),
        1,
        (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD)
        );

      PchMsgBusAndThenOr32AddToS3Save (
        PCH_LPSS_EP_PORT_ID,
        R_PCH_LPSS_FABCTLP3,
        Buffer32,
        0xFFFFFFFF,
        (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS),
        PCH_LPSS_EP_PRIVATE_READ_OPCODE,
        PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
        );
    }

    ///
    /// LPSS1 HS-UART 1
    ///
    if ((PchPlatformPolicy->LpssConfig->Hsuart1Enabled == 1) && (PchPlatformPolicy->LpssConfig->LpssPciModeEnabled == 0)) {
      DEBUG ((EFI_D_INFO, "Switching LPSS1 HS-UART 1 into ACPI Mode.\n"));
      LpssPciMmBase = MmPciAddress (
                        0,
                        PchPlatformPolicy->BusNumber,
                        PCI_DEVICE_NUMBER_PCH_LPSS_HSUART,
                        PCI_FUNCTION_NUMBER_PCH_LPSS_HSUART1,
                        0
                        );
      ///
      /// Disable PCI Interrupt, Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 (
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD),
        (UINT32) (B_PCH_LPSS_HSUART_STSCMD_INTRDIS | B_PCH_LPSS_HSUART_STSCMD_BME | B_PCH_LPSS_HSUART_STSCMD_MSE)
        );
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD),
        1,
        (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_HSUART_STSCMD)
        );

      PchMsgBusAndThenOr32AddToS3Save (
        PCH_LPSS_EP_PORT_ID,
        R_PCH_LPSS_FABCTLP4,
        Buffer32,
        0xFFFFFFFF,
        (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS),
        PCH_LPSS_EP_PRIVATE_READ_OPCODE,
        PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
        );
    }

    ///
    /// LPSS1 SPI
    ///
    if ((PchPlatformPolicy->LpssConfig->SpiEnabled == 1) && (PchPlatformPolicy->LpssConfig->LpssPciModeEnabled == 0)) {
      DEBUG ((EFI_D_INFO, "Switching LPSS1 SPI into ACPI Mode.\n"));
      LpssPciMmBase = MmPciAddress (
                        0,
                        PchPlatformPolicy->BusNumber,
                        PCI_DEVICE_NUMBER_PCH_LPSS_SPI,
                        PCI_FUNCTION_NUMBER_PCH_LPSS_SPI,
                        0
                        );
      ///
      /// Disable PCI Interrupt, Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 (
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_STSCMD),
        (UINT32) (B_PCH_LPSS_SPI_STSCMD_INTRDIS | B_PCH_LPSS_SPI_STSCMD_BME | B_PCH_LPSS_SPI_STSCMD_MSE)
        );
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_STSCMD),
        1,
        (VOID *) (UINTN) (LpssPciMmBase + R_PCH_LPSS_SPI_STSCMD)
        );

      PchMsgBusAndThenOr32AddToS3Save (
        PCH_LPSS_EP_PORT_ID,
        R_PCH_LPSS_FABCTLP5,
        Buffer32,
        0xFFFFFFFF,
        (B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS),
        PCH_LPSS_EP_PRIVATE_READ_OPCODE,
        PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
        );
    }
  }

  SignalAllDriversLpssDone();
  DEBUG ((EFI_D_INFO, "ConfigureLpssAtBoot() End\n"));

  return EFI_SUCCESS;
}
