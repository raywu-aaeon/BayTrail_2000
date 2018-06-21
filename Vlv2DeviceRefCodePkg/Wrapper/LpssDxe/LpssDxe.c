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

  LpssDxe.c

Abstract:

  LPSS Platform Driver


--*/


#include <LpssDxe.h>
#include <token.h> //EIP132398
#include <Protocol/ExitPmAuth.h> //EIP143364
#include "PchAccess.h" //EIP143364

//(CSP20130313E+)>>
LPSS_DEVICE_INFO  mSccDeviceListA0Stepping[] = {

    {
        0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_SCC_SDIO_0, PCI_FUNCTION_NUMBER_PCH_SCC_SDIO,\
        0, 0, 0, GLOBAL_NVS_OFFSET(eMMCAddr),  GLOBAL_NVS_OFFSET(eMMCLen), \
        1, 0, 0, GLOBAL_NVS_OFFSET(eMMC1Addr), GLOBAL_NVS_OFFSET(eMMC1Len), \
        R_PCH_SCC_EP_PCICFGCTR1, (B_PCH_SCC_EP_PCICFGCTR1_ACPI_INT_EN1 | B_PCH_SCC_EP_PCICFGCTR1_PCI_CFG_DIS1)
    }
	//EIP143364 >>
	,{
        0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_SCC_SDIO_2, PCI_FUNCTION_NUMBER_PCH_SCC_SDIO,\
        0, 0, 0, GLOBAL_NVS_OFFSET(SDCardAddr),  GLOBAL_NVS_OFFSET(SDCardLen), \
        1, 0, 0, GLOBAL_NVS_OFFSET(SDCard1Addr), GLOBAL_NVS_OFFSET(SDCard1Len), \
        R_PCH_SCC_EP_PCICFGCTR2, (B_PCH_SCC_EP_PCICFGCTR2_ACPI_INT_EN1 | B_PCH_SCC_EP_PCICFGCTR2_PCI_CFG_DIS1)
    }  
	//EIP143364 <<

};

LPSS_DEVICE_INFO  mSccDeviceListB0Stepping[] = {

    {
        0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_SCC_SDIO_3, PCI_FUNCTION_NUMBER_PCH_SCC_SDIO,\
        0, 0, 0, GLOBAL_NVS_OFFSET(eMMCAddr),  GLOBAL_NVS_OFFSET(eMMCLen), \
        1, 0, 0, GLOBAL_NVS_OFFSET(eMMC1Addr), GLOBAL_NVS_OFFSET(eMMC1Len), \
        R_PCH_SCC_EP_PCICFGCTR4, (B_PCH_SCC_EP_PCICFGCTR1_ACPI_INT_EN1 | B_PCH_SCC_EP_PCICFGCTR1_PCI_CFG_DIS1)
    }
	//EIP143364 >>
	,{
        0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_SCC_SDIO_2, PCI_FUNCTION_NUMBER_PCH_SCC_SDIO,\
        0, 0, 0, GLOBAL_NVS_OFFSET(SDCardAddr),  GLOBAL_NVS_OFFSET(SDCardLen), \
        1, 0, 0, GLOBAL_NVS_OFFSET(SDCard1Addr), GLOBAL_NVS_OFFSET(SDCard1Len), \
        R_PCH_SCC_EP_PCICFGCTR2, (B_PCH_SCC_EP_PCICFGCTR2_ACPI_INT_EN1 | B_PCH_SCC_EP_PCICFGCTR2_PCI_CFG_DIS1)
    }
	//EIP143364 <<

};

//
// Default set to A0 device table
//
LPSS_DEVICE_INFO *mSccDeviceList = mSccDeviceListA0Stepping;

//
// A0 and B0 have the same table size
//
#define SCC_DEVICE_NUMBER  sizeof(mSccDeviceListA0Stepping)/sizeof(LPSS_DEVICE_INFO)

EFI_HANDLE          mImageHandle;
UINT8               EmmcSwSmi = 0; //EIP132398
UINT8               SdCardSwSmi = 0; //EIP143364 
EFI_GLOBAL_NVS_AREA_PROTOCOL  *mGlobalNvsArea; //EIP143364 

extern EFI_GUID gEfiEventExitBootServicesGuid;

extern PCH_STEPPING EFIAPI PchStepping(VOID);

EFI_STATUS
EFIAPI
SccAcpiModeEnable(
    LPSS_DEVICE_INFO   *Device
)
{
    UINT32      Buffer32;
    DEBUG((DEBUG_ERROR, "Start to write configuration \n"));
    DEBUG((DEBUG_ERROR, " Port id: 0x%x\n", PCH_SCC_EP_PORT_ID));
    DEBUG((DEBUG_ERROR, " reg offset: 0x%x\n", Device->AcpiModeRegOffset));
    DEBUG((DEBUG_ERROR, " Data write: 0x%x\n", Device->AcpiModeRegValue));

    SccMsgBus32AndThenOr(
        Device->AcpiModeRegOffset,
        Buffer32,
        0xFFFFFFFF,
        Device->AcpiModeRegValue
    );
    DEBUG((DEBUG_ERROR, "Start to read configuration back\n"));
    SccMsgBusRead32(
        Device->AcpiModeRegOffset,
        Buffer32
    );
    DEBUG((DEBUG_ERROR, " Port id: 0x%x\n", PCH_SCC_EP_PORT_ID));
    DEBUG((DEBUG_ERROR, " reg offset: 0x%x\n", Device->AcpiModeRegOffset));
    DEBUG((DEBUG_ERROR, " Data read out: 0x%x\n", Buffer32));
    return EFI_SUCCESS;
}

VOID
UpdateLpssDeviceBar(
    EFI_PCI_IO_PROTOCOL     *PciIo,
    EFI_PHYSICAL_ADDRESS    BaseAddress,
    UINT8                   BarIndex
)
{
    UINT8                   NewCommand;
    UINT32                  NewBar;
    EFI_STATUS              Status;

    UINT16                   Command;
    Status = PciIo->Pci.Read(PciIo,
                             EfiPciIoWidthUint16,
                             PCI_COMMAND_OFFSET,
                             1,
                             &Command
                            );

    NewCommand = 0;
    Status = PciIo->Pci.Write(PciIo,
                              EfiPciIoWidthUint8,
                              PCI_COMMAND_OFFSET,
                              1,
                              &NewCommand
                             );
    NewBar = (UINT32)BaseAddress;
    Status = PciIo->Pci.Write(PciIo,
                              EfiPciIoWidthUint32,
                              PCI_BASE_ADDRESSREG_OFFSET + BarIndex * sizeof(UINT32),
                              1,
                              &NewBar
                             );

    Status = PciIo->Pci.Write(PciIo,
                              EfiPciIoWidthUint16,
                              PCI_COMMAND_OFFSET,
                              1,
                              &Command
                             );


}


VOID
UpdateLpssSccDevice(
    EFI_PCI_IO_PROTOCOL     *PciIo,
    LPSS_DEVICE_INFO        *DeviceInfo
)
{
    VOID         *Resources;
    EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *ptr;
    EFI_STATUS                        Status;
    UINT8                             BarIndex;
    EFI_PHYSICAL_ADDRESS              BaseAddress;
    for(BarIndex = 0; BarIndex < PCI_MAX_BAR; BarIndex ++) {
        Status = PciIo->GetBarAttributes(//scan pci device's bar
                     PciIo,
                     BarIndex,
                     NULL,
                     &Resources
                 );
        if(EFI_ERROR(Status)) {
            continue;
        }
        ptr = Resources;
        if(ptr->ResType != ACPI_ADDRESS_SPACE_TYPE_MEM) {
            gBS->FreePool(Resources);
            continue;
        }
        DEBUG((DEBUG_ERROR, "Devices  %x %x %x %x %x %lx %lx %lx %lx %lx\n",
               ptr->Desc, ptr->Len, ptr->ResType, ptr->GenFlag, ptr->SpecificFlag,
               ptr->AddrSpaceGranularity, ptr->AddrRangeMin, ptr->AddrRangeMax,
               ptr->AddrTranslationOffset, ptr->AddrLen
              ));
        BaseAddress = 0xFFFFFFFF;
#if 1
        Status = gDS->AllocateMemorySpace(
                     EfiGcdAllocateMaxAddressSearchBottomUp,
                     EfiGcdMemoryTypeMemoryMappedIo,
                     HighBitSet64(ptr->AddrSpaceGranularity),
                     ptr->AddrLen,
                     &BaseAddress,
                     mImageHandle,
                     NULL
                 );
        ASSERT_EFI_ERROR(Status);
#else
        RegBase = MmPciAddress(
                      0,
                      DeviceInfo->BusNum,
                      DeviceInfo->DeviceNum,
                      DeviceInfo->FunctionNum,
                      0
                  );
        BaseAddress = (EFI_PHYSICAL_ADDRESS)(MmioRead32(RegBase + R_PCH_LPSS_SDIO_BAR + 0x04 * BarIndex) & B_PCH_LPSS_SDIO_BAR_BA);
#endif
        DEBUG((DEBUG_ERROR, "New Resource is %x %lx\n", BarIndex, BaseAddress));
        UpdateLpssDeviceBar(PciIo, BaseAddress, BarIndex);
        if(BarIndex == DeviceInfo->ReportBarIndex) {
            DeviceInfo->ReportBar = BaseAddress;
            DeviceInfo->ReportBarLen = ptr->AddrLen;
        }
        if(BarIndex == DeviceInfo->ReportBarIndex1) {
            DeviceInfo->ReportBar1 = BaseAddress;
            DeviceInfo->ReportBarLen1 = ptr->AddrLen;
        }
        gBS->FreePool(Resources);
    }
}

EFI_STATUS
UpdateLpssSccDeviceList()
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
    Status = gBS->LocateHandleBuffer(
                 ByProtocol,
                 &gEfiPciIoProtocolGuid,
                 NULL,
                 &HandleCount,
                 &Handles
             );
    if(EFI_ERROR(Status)) {
        return Status;
    }

    for(Index = 0; Index < HandleCount; Index ++) {

        Status = gBS->HandleProtocol(
                     Handles[Index],
                     &gEfiPciIoProtocolGuid,
                     &PciIo
                 );
        if(EFI_ERROR(Status)) {
            continue;
        }
        Status = PciIo->GetLocation(
                     PciIo,
                     &Segment,
                     &BusNum,
                     &DeviceNum,
                     &FunctionNum
                 );

        for(Index2 = 0; Index2 < SCC_DEVICE_NUMBER; Index2 ++) {
            if(mSccDeviceList[Index2].Segment     == Segment &&
                    mSccDeviceList[Index2].BusNum      == BusNum &&
                    mSccDeviceList[Index2].DeviceNum   == DeviceNum &&
                    mSccDeviceList[Index2].FunctionNum == FunctionNum) {
                DEBUG((EFI_D_ERROR, "Update Device %x %x %x\n", BusNum, DeviceNum, FunctionNum));
                UpdateLpssSccDevice(PciIo, &mSccDeviceList[Index2]);
            }
        }

    }
    gBS->FreePool(Handles);
    return EFI_SUCCESS;
}

EFI_STATUS
GetPCIAddrRange(
    EFI_PHYSICAL_ADDRESS *RangeMin,
    EFI_PHYSICAL_ADDRESS *RangeMax
)
{
    EFI_STATUS                    Status;

    EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *PciResource;
    EFI_HANDLE                                       RootBridgeHandle;
    UINT64                                           Attributes;
    VOID                                            *Configuration;
    EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR               *ptr;
    BOOLEAN                                         Found;
    *RangeMin = (EFI_PHYSICAL_ADDRESS)(-1);
    *RangeMax = 0;
    Status = gBS->LocateProtocol(
                 &gEfiPciHostBridgeResourceAllocationProtocolGuid,
                 NULL,
                 &PciResource
             );
    if(EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, "Can't locate PciResource Protocol\n"));
        return EFI_NOT_FOUND;

    }
    RootBridgeHandle = NULL;
    Found = FALSE;
    while(TRUE) {
        Status = PciResource->GetNextRootBridge(
                     PciResource,
                     &RootBridgeHandle
                 );
        if(EFI_ERROR(Status)) {
            break;
        }
        Status = PciResource->GetAllocAttributes(
                     PciResource,
                     RootBridgeHandle,
                     &Attributes
                 );

        DEBUG((DEBUG_ERROR, "RootBridgeHandle %x %lx\n", RootBridgeHandle, Attributes));
        Status = PciResource->GetProposedResources(
                     PciResource,
                     RootBridgeHandle,
                     &Configuration
                 );
        if(EFI_ERROR(Status)) {
            continue;
        }
        ptr = Configuration;
        while(ptr->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR) {
            DEBUG((DEBUG_ERROR, "  %x %x %x %x %x %lx %lx %lx %lx %lx\n",
                   ptr->Desc, ptr->Len, ptr->ResType, ptr->GenFlag, ptr->SpecificFlag,
                   ptr->AddrSpaceGranularity, ptr->AddrRangeMin, ptr->AddrRangeMax,
                   ptr->AddrTranslationOffset, ptr->AddrLen
                  ));
            if(ptr->ResType == ACPI_ADDRESS_SPACE_TYPE_MEM) {
                if(*RangeMin > ptr->AddrRangeMin)  *RangeMin = ptr->AddrRangeMin;
                if(*RangeMax < (ptr->AddrRangeMin + ptr->AddrLen))  *RangeMax = ptr->AddrRangeMin + ptr->AddrLen;
                Found = TRUE;
            }
            ptr ++;
        }
        gBS->FreePool(Configuration);
    }
    if(!Found) {
        return EFI_NOT_FOUND;
    }
    return EFI_SUCCESS;
}

VOID
UpdateLpssSccDeviceInfo(
  EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsArea, 
  UINT32  						PciMode
                     )
{
  UINTN         Index;

  for(Index = 0; Index < SCC_DEVICE_NUMBER; Index ++) {
    if (mSccDeviceList[Index].ReportBar != 0) {
      *(UINT32*)((CHAR8*)GlobalNvsArea->Area + mSccDeviceList[Index].AddrOffset) = (UINT32)mSccDeviceList[Index].ReportBar;
      *(UINT32*)((CHAR8*)GlobalNvsArea->Area + mSccDeviceList[Index].LenOffset) =  (UINT32)mSccDeviceList[Index].ReportBarLen;
      *(UINT32*)((CHAR8*)GlobalNvsArea->Area + mSccDeviceList[Index].AddrOffset1) = (UINT32)mSccDeviceList[Index].ReportBar1;
      *(UINT32*)((CHAR8*)GlobalNvsArea->Area + mSccDeviceList[Index].LenOffset1) =  (UINT32)mSccDeviceList[Index].ReportBarLen1;
      if (PciMode == 1) {
        DEBUG((DEBUG_ERROR, " Bypass ACPI mode, work in PCI mode\n"));
      } else {
        DEBUG((DEBUG_ERROR, " SccAcpiModeEnable started, work in ACPI mode\n"));
        SccAcpiModeEnable(&mSccDeviceList[Index]);
      }
    }
  }
}

//EIP143364 >>
VOID
OnExitBootServices4LPSS(
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  UpdateLpssSccDeviceList();
  
  //EIP132398 >>
  if (EmmcSwSmi != 0) {
    DEBUG ((EFI_D_ERROR, "Generate eMMC SW SMI = %x\n", EmmcSwSmi));
    IoWrite8 (SW_SMI_IO_ADDRESS, EmmcSwSmi);
  }
  //EIP132398 <<

  if (SdCardSwSmi != 0) {
    DEBUG ((EFI_D_ERROR, "Generate Sd Card SW SMI = %x\n", SdCardSwSmi));
    IoWrite8 (SW_SMI_IO_ADDRESS, SdCardSwSmi);
  }

  // EFI boot stays in ACPI mode
  UpdateLpssSccDeviceInfo(mGlobalNvsArea, 0);

  //
  // Switch GPIO to F0 for SD card 
  //
  if (SdCardSwSmi != 0) {
    MmioAnd32 (IO_BASE_ADDRESS + 0x03A0, 0xFFFFFFFE);
  }
}
//EIP143364 <<

STATIC
VOID
EFIAPI
OnReadyToBoot4LPSS(
    IN EFI_EVENT  Event,
    IN VOID       *Context
)
{
    EFI_PHYSICAL_ADDRESS          RangeMin;
    EFI_PHYSICAL_ADDRESS          RangeMax;
    EFI_STATUS                    Status;
    EFI_EVENT                     EventForExitBootServices; //EIP143364 

    static UINT8 FirstTime=1;
    if(1==FirstTime)
        FirstTime=0;
    else
        return;
//CSP20130723 - Update PciBottom and PciTop Address correctly >>
//    Status = GetPCIAddrRange(&RangeMin, &RangeMax);
//    if(EFI_ERROR(Status)) {
//        return;
//    }
//CSP20130723 - Update PciBottom and PciTop Address correctly <<

    Status = gBS->LocateProtocol(
                 &gEfiGlobalNvsAreaProtocolGuid,
                 NULL,
                 &mGlobalNvsArea //EIP143364 
             );
    if(EFI_ERROR(Status)) {
        return;
    }

//CSP20130723 - Update PciBottom and PciTop Address correctly >>    
    Status = GetPCIAddrRange(&RangeMin, &RangeMax);
    
//EIP143364 >>
    if(EFI_ERROR(Status)) {
        mGlobalNvsArea->Area->PCIBottomAddress = mGlobalNvsArea->Area->BmBound; 	//AMI_OVERRIDE - Update PciBottom and PciTop Address correctly
        mGlobalNvsArea->Area->PCITopAddress = (UINT32)0xDFFFFFFF; //AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base
    } else {
      mGlobalNvsArea->Area->PCIBottomAddress = (UINT32)RangeMin;
      mGlobalNvsArea->Area->PCITopAddress = (UINT32)RangeMax - 1;
    }
//CSP20130723 - Update PciBottom and PciTop Address correctly <<

    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    OnExitBootServices4LPSS,
                    (VOID *) mSccDeviceList,
                    &gEfiEventExitBootServicesGuid,
                    &EventForExitBootServices
                    );

    if (SdCardSwSmi != 0) {
      // Enable SD Card controller Memory Decode
      PchMmPci32Or (0, 0, 
          PCI_DEVICE_NUMBER_PCH_SCC_SDIO_2,
          PCI_FUNCTION_NUMBER_PCH_SCC_SDIO,
          R_PCH_SCC_SDIO_STSCMD,
          0x06
          );
    }
//EIP143364  <<

    return;
}

STATIC
VOID
EFIAPI
OnReadyToBoot4LPSSLegacy(
IN EFI_EVENT  Event,
IN VOID       *Context
  )
{
    UpdateLpssSccDeviceList();
    
    //EIP132398 >>
    if (EmmcSwSmi != 0) {
      DEBUG ((EFI_D_ERROR, "Generate eMMC SW SMI = %x\n", EmmcSwSmi));
      IoWrite8 (SW_SMI_IO_ADDRESS, EmmcSwSmi);
    }
    //EIP132398 <<

    // Legacy boot stays in ACPI mode
    UpdateLpssSccDeviceInfo(mGlobalNvsArea, 0); 
  
    return;
}

EFI_STATUS
EFIAPI
LpssDxeEntryPoint(
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
)
/*++

Routine Description:

  Entry point for Acpi platform driver.

Arguments:

  ImageHandle  -  A handle for the image that is initializing this driver.
  SystemTable  -  A pointer to the EFI system table.

Returns:

  EFI_SUCCESS           -  Driver initialized successfully.
  EFI_LOAD_ERROR        -  Failed to Initialize or has been loaded.
  EFI_OUT_OF_RESOURCES  -  Could not allocate needed resources.

--*/
{
  EFI_STATUS                    Status;
  EFI_EVENT                     Event;
  DXE_PCH_PLATFORM_POLICY_PROTOCOL *DxePlatformPchPolicy;
  VOID                              *RegistrationExitPmAuth; //EIP143364 
//  UINT8                            Index;

  mImageHandle = ImageHandle;
  mSccDeviceList = mSccDeviceListA0Stepping;

  Status  = gBS->LocateProtocol (&gDxePchPlatformPolicyProtocolGuid, NULL, &DxePlatformPchPolicy); 
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Locate the gDxePchPlatformPolicyProtocolGuid Failed\n"));
  } else {
    if (DxePlatformPchPolicy->SccConfig->eMMCEnabled) {
      DEBUG ((EFI_D_ERROR, "A0/A1 SCC eMMC 4.41 table\n"));
      mSccDeviceList = mSccDeviceListA0Stepping;
	  //EIP132398 >>
      if (DxePlatformPchPolicy->LpssConfig->LpssPciModeEnabled == 0){
        EmmcSwSmi = EMMC_441_SW_SMI;
      }
	  //EIP132398 <<
    } else if (DxePlatformPchPolicy->SccConfig->eMMC45Enabled) {
      DEBUG ((EFI_D_ERROR, "B0 SCC eMMC 4.5 table\n"));
      mSccDeviceList = mSccDeviceListB0Stepping;
	  //EIP132398 >>
      if (DxePlatformPchPolicy->LpssConfig->LpssPciModeEnabled == 0){
        EmmcSwSmi = EMMC_45_SW_SMI;
      }
	  //EIP132398 <<
    } else {
      DEBUG ((EFI_D_ERROR, "eMMC device is not enabled!!!\n"));
    }
	//EIP132398 >>
    if (EmmcSwSmi != 0) {
      S3BootScriptSaveIoWrite(
          EfiBootScriptWidthUint8,
          (UINTN) R_PCH_APM_CNT,
          1,
          &EmmcSwSmi
          );
    }
	//EIP132398 <<

	//EIP143364 >>
    if (DxePlatformPchPolicy->SccConfig->SdcardEnabled) {
      if (DxePlatformPchPolicy->LpssConfig->LpssPciModeEnabled == 0){
        DEBUG ((EFI_D_ERROR, "SD Card device is enabled!!!\n"));
        SdCardSwSmi = SD_CARD_SW_SMI;
        S3BootScriptSaveIoWrite(
            EfiBootScriptWidthUint8,
            (UINTN) R_PCH_APM_CNT,
            1,
            &SdCardSwSmi
            );
      }
    }
	//EIP143364 <<
  }

#if 0  
  switch (PchStepping()) {
  case PchA0:
  case PchA1:
    DEBUG ((EFI_D_ERROR, "A0/A1 SCC eMMC 4.41 table\n"));
    mSccDeviceList = mSccDeviceListA0Stepping;
    break;
    
  case PchB0:
     DEBUG ((EFI_D_ERROR, "B0 SCC eMMC 4.5 table\n"));
     mSccDeviceList = mSccDeviceListB0Stepping;
     break;

  default:
     DEBUG ((EFI_D_ERROR, "Unknown Steppting, using A0 eMMC 4.41 table\n"));
     mSccDeviceList = mSccDeviceListA0Stepping;
     break;
  }
#endif 

//EIP137990 >>
  //
  // LpssPciModeEnabled = 1;
  // enable device to run in pci mode,
  // Don't need create event for translate device into acpi mode
  //
  // LpssPciModeEnabled = 0; 
  // enable device to run in acpi mode,
  // Need to create event for translate device into acpi mode
  //
  if ((DxePlatformPchPolicy->LpssConfig->LpssPciModeEnabled == 0) &&
      ((DxePlatformPchPolicy->SccConfig->eMMCEnabled !=0) || 
       (DxePlatformPchPolicy->SccConfig->eMMC45Enabled !=0) || 
       (DxePlatformPchPolicy->SccConfig->SdcardEnabled != 0))) { //EIP143364 
    // S4682406 - Also register OnReadyToBoot4LPSS callback for legacy boot 
    Status = EfiCreateEventLegacyBootEx (
               TPL_CALLBACK,
               OnReadyToBoot4LPSSLegacy,
               NULL,
               &Event
               );
    ASSERT_EFI_ERROR (Status);
  
  //EIP143364 >>
    Status = gBS->CreateEvent ( 
                     EVT_NOTIFY_SIGNAL,
                     TPL_CALLBACK,
                     OnReadyToBoot4LPSS,
                     NULL,
                     &Event
                     );
   
    Status = gBS->RegisterProtocolNotify (
                    &gExitPmAuthProtocolGuid,
                    Event,
                    &RegistrationExitPmAuth
                    );
    //EIP143364 <<
	
    //Status = gBS->CreateEventEx (
    //                EVT_NOTIFY_SIGNAL,
    //                TPL_NOTIFY,
    //                OnReadyToBoot4LPSS,
    //                NULL,
    //                &gEfiEventExitBootServicesGuid,
    //                &Event
    //                ); 
  }
//EIP137990 <<
  return EFI_SUCCESS;
}
