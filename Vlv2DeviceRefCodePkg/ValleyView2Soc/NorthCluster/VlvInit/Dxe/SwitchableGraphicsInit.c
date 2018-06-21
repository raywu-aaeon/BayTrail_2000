/** @file
  SwitchableGraphics /HybridGraphics Dxe driver.
  This DXE driver loads SwitchableGraphics acpi tables, for the platform.

@copyright
  Copyright (c) 2010 - 2013 Intel Corporation. All rights reserved.
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is licensed for 
  Intel CPUs and chipsets under the terms of your license agreement 
  with Intel or your vendor. This file may be modified by the user, 
  subject to additional terms of the license agreement.

**/

#include "SwitchableGraphicsInit.h"
#ifdef SG_SUPPORT


extern DXE_VLV_PLATFORM_POLICY_PROTOCOL  *DxePlatformSaPolicy;
extern EFI_GUID gSgAcpiTableGuid;    // AMI_OVERRIDE

EFI_PLATFORM_INFO_HOB       *PlatformInfo;
EFI_PEI_HOB_POINTERS        Hob;

VOID                  *VbiosAddress = NULL;
BOOLEAN               DgpuOpRomCopied = FALSE;
UINT32                VbiosSize;

UINT8                 EndpointBus;
UINT8                 GpioSupport;

UINT8                 RootPortDev;
UINT8                 RootPortFun;

/**
  Initialize the SwitchableGraphics support (DXE).

  @param[in] ImageHandle         - Handle for the image of this driver
  @param[in] DxePlatformSaPolicy - SA DxePlatformPolicy protocol

  @retval EFI_SUCCESS         - SwitchableGraphics initialization complete
  @retval EFI_OUT_OF_RESOURCES - Unable to allocated memory
  @retval EFI_NOT_FOUND        - SA DataHob not found
  @retval EFI_DEVICE_ERROR     - Error Accessing SG GPIO
**/
EFI_STATUS
SwitchableGraphicsInit (
  IN EFI_HANDLE                      ImageHandle,
  IN DXE_VLV_PLATFORM_POLICY_PROTOCOL *DxePlatformSaPolicy
  )
{
  EFI_STATUS  Status;
  VOID        *Registration;

  // For Hybrid Graphics support, In Bayley Bay board dGPU sits on Bus0:Device28:RootPort Function0
  RootPortDev = PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS;
  RootPortFun = 0;

  Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (Hob.Raw != NULL);
  PlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);

  GpioSupport = PlatformInfo->SgInfo.SgGpioSupport;

  //
  // Update GlobalNvs data for runtime usage
  //
  Status = UpdateGlobalNvsData (DxePlatformSaPolicy);
  if (EFI_ERROR (Status)) {
    return Status;
  }

//************************************************************************
//
// Load Intel SG SSDT tables
//
//************************************************************************
  Status = LoadAcpiTables ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Check to see if Switchable Graphics Mode is enabled
  //
  if (PlatformInfo->SgInfo.SgMode == SgModeMuxless) {
  //
  // Create ReadyToBoot callback for SG
  //
	EfiCreateProtocolNotifyEvent (
		&gExitPmAuthProtocolGuid,
		TPL_CALLBACK,
		SgExitPmAuthCallback,
		NULL,
		&Registration
	);
  }

  return Status;
}

/**
  Initialize the runtime SwitchableGraphics support data for ACPI tables in GlobalNvs.

  @param[in] SaDataHob->SgInfo   - Pointer to Hob for SG system details.
  @param[in] DxePlatformSaPolicy - Pointer to the loaded image protocol for this driver.

  @retval EFI_SUCCESS - The data updated successfully.
**/
EFI_STATUS
UpdateGlobalNvsData (
  IN DXE_VLV_PLATFORM_POLICY_PROTOCOL *DxePlatformSaPolicy
  )
{
  EFI_GLOBAL_NVS_AREA_PROTOCOL          *GlobalNvsArea;
  UINT8                                  CapOffset;
  UINT16                                 ExtendedCapOffset;
  EFI_STATUS                             Status;
  UINT32                                 Data32;
  UINT8                   RpFunction;
  UINTN                   RpBase;
  UINTN                   EpBase;
  UINT8             	  Data;
  UINT8                   PcieCmd;
  //
  //  Locate Global NVS Area Protocol.
  //
  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  &GlobalNvsArea
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  // AMI_OVERRIDE >>
  // Due to Windows 7 didn't support SG, so if OS selection is for Windows 7, return unsupport.
  if (GlobalNvsArea->Area->OsSelection == OS_WINDOWS7) {  // OsSelect == Windows 7  //CSP20140122 Change token name 
    return EFI_UNSUPPORTED;
  }
  // AMI_OVERRIDE <<
  
  //
  // SG Mode for ASL usage
  //
  GlobalNvsArea->Area->SgMode               |= PlatformInfo->SgInfo.SgMode;
  GlobalNvsArea->Area->IoBaseAddress        = IO_BASE_ADDRESS;				// 0xFED0C000;
  GlobalNvsArea->Area->SgGPIOSupport		= PlatformInfo->SgInfo.SgGpioSupport;

  DEBUG ((EFI_D_INFO, "SG:: Switchable Graphics Mode : 0x%x\n", PlatformInfo->SgInfo.SgMode));

  if (PlatformInfo->SgInfo.SgMode == SgModeMuxless) {

    if (PlatformInfo->SgInfo.SgGpioSupport) {
      ///
      /// GPIO Assignment for ASL usage
      ///
      GlobalNvsArea->Area->SgDgpuPwrOK      = PlatformInfo->SgInfo.SgDgpuPwrOK;
      GlobalNvsArea->Area->SgDgpuHoldRst    = PlatformInfo->SgInfo.SgDgpuHoldRst;
      GlobalNvsArea->Area->SgDgpuPwrEnable  = PlatformInfo->SgInfo.SgDgpuPwrEnable;
      GlobalNvsArea->Area->SgDgpuPrsnt      = PlatformInfo->SgInfo.SgDgpuPrsnt;

      DEBUG ((EFI_D_INFO, "SG:: dGPU_PWROK GPIO   GPIO assigned = %d\n", PlatformInfo->SgInfo.SgDgpuPwrOK & 0x7f));
      DEBUG ((EFI_D_INFO, "SG:: dGPU_HOLD_RST#    GPIO assigned = %d\n", PlatformInfo->SgInfo.SgDgpuHoldRst & 0x7f));
      DEBUG ((EFI_D_INFO, "SG:: dGPU_PWR_EN#      GPIO assigned = %d\n", PlatformInfo->SgInfo.SgDgpuPwrEnable & 0x7f));
      DEBUG ((EFI_D_INFO, "SG:: dGPU_PRSNT#       GPIO assigned = %d\n", PlatformInfo->SgInfo.SgDgpuPrsnt & 0x7f));
    }

    DEBUG ((EFI_D_INFO, "SG:: VBIOS Configurations:\n"));
    DEBUG ((EFI_D_INFO, "SG:: Load VBIOS  (0=No Vbios;1=Load VBIOS)     =%d\n", DxePlatformSaPolicy->VbiosConfig.LoadVbios));
    DEBUG ((EFI_D_INFO, "SG:: Execute VBIOS (0=Do not execute;1=Execute Vbios)  =%d\n", DxePlatformSaPolicy->VbiosConfig.ExecuteVbios));
    DEBUG ((EFI_D_INFO, "SG:: VBIOS Source  (0=PCIE Card;1=FW Volume)         =%d\n", DxePlatformSaPolicy->VbiosConfig.VbiosSource));

    //
    // Save bus numbers on the PCH bridge.
    //
    Data32 = MmPci32 (0, 0, RootPortDev, RootPortFun, PCI_PBUS);
    Data32 &= 0x00FFFF00;

  //
  // Scan PCH PCI-EX slots (Root Port) : Device 28 Function 0~3
  //
  for (RpFunction = 0; RpFunction < PCH_PCIE_MAX_ROOT_PORTS; RpFunction ++) {
    RpBase = MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS, RpFunction, 0);
    DEBUG ((EFI_D_INFO, "SwitchableGfx: PCI-EX Root Port: 0x%x ...\n", RpFunction));

    if ((MmioRead32 (RpBase + R_PCH_PCIE_SLCTL_SLSTS) & B_PCH_PCIE_SLCTL_SLSTS_PDS) != 0 && MmioRead16 (RpBase + R_PCH_PCIE_ID) != 0xFFFF) {
		DEBUG ((EFI_D_INFO, "SwitchableGfx: RP-Base: 0x%x ...\n", RpBase));
		DEBUG ((EFI_D_INFO, "SwitchableGfx: VendorId: 0x%x ...\n", MmioRead16 (RpBase + R_PCH_PCIE_ID) ));
	  //
      // Set PCH PortBus = 1 to Read Endpoint.
      //
      MmioAndThenOr32(RpBase + R_PCH_PCIE_BNUM_SLT, 0xFF0000FF, 0x00010100);

    //
    // A config write is required in order for the device to re-capture the Bus number,
    // according to PCI Express Base Specification, 2.2.6.2
    // Write to a read-only register VendorID to not cause any side effects.
    //
	MmPci16 (0, 1, 0, 0, PCI_VID) = 0;    //MmPci16(Segment, Bus, Dev, Fn, Reg)

    EpBase = MmPciAddress (0, 1, 0, 0, 0);
    DEBUG ((EFI_D_INFO, "SwitchableGfx: EP-Base: 0x%x ...\n", EpBase));

  Data = MmPci8( 0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS, PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_1, R_PCH_PCIE_CMD_PSTS );

  PcieCmd = Data;
  Data |= (B_PCH_PCIE_CMD_PSTS_BME | B_PCH_PCIE_CMD_PSTS_MSE | B_PCH_PCIE_CMD_PSTS_IOSE);
  MmPci8( 0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS, PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_1, R_PCH_PCIE_CMD_PSTS ) = Data;

	  GlobalNvsArea->Area->RootPortBaseAddress = (UINT32)RpBase;
	  GlobalNvsArea->Area->EndPointBaseAddress = (UINT32)EpBase;

    EndpointBus = MmPci8 (0, 0, RootPortDev, RootPortFun, PCI_SBUS);

    if (EndpointBus != 0xFF) {
      GlobalNvsArea->Area->CapStrPresence = 0;

      CapOffset = (UINT8) PcieFindCapId (EndpointBus, 0, 0, PEG_CAP_ID);
      GlobalNvsArea->Area->EndpointPcieCapOffset = CapOffset;
      DEBUG ((EFI_D_INFO, "SG:: Endpoint PCI Express Capability Offset : 0x%x\n", GlobalNvsArea->Area->EndpointPcieCapOffset));

      ExtendedCapOffset = (UINT16) PcieFindExtendedCapId (EndpointBus, 0, 0, PEG_CAP_VER);
      if (ExtendedCapOffset != 0) {
        GlobalNvsArea->Area->CapStrPresence |= BIT0;
        GlobalNvsArea->Area->EndpointVcCapOffset = ExtendedCapOffset;
        DEBUG ((EFI_D_INFO, "SG:: Endpoint Virtual Channel Capability Offset : 0x%x\n", GlobalNvsArea->Area->EndpointVcCapOffset));
      }
   }

    //
    // Restore bus numbers on the PCH bridge.
    //
//    MmPci32AndThenOr (0, 0, RootPortDev, RootPortFun, PCI_PBUS, 0xFF0000FF, Data32);
    //
    // Restore bus numbers on the PCH bridge. (Bus 0)
    //
        MmioAnd32(RpBase + R_PCH_PCIE_BNUM_SLT, 0xFF0000FF); 
		break;
		}  //if

	}  //for

  } else {
    DEBUG ((EFI_D_ERROR, "SG:: Switchable Graphics Mode disabled!!!\n"));
    Status = EFI_LOAD_ERROR;
  }

  return Status;
}

/**
  Load and execute the dGPU GOP.

  @param[in] VbiosConfig - Pointer to VbiosConfig policy for Load/Execute and VBIOS Source.
      LoadVbios    - 0 = Do Not Load   ; 1 = Load VBIOS
      ExecuteVbios - 0 = Do Not Execute; 1 = Execute VBIOS
      VbiosSource  - 0 = PCIE Device   ; 1 = FirmwareVolume => TBD

  @retval EFI_SUCCESS     - Load and execute successful.
  @exception EFI_UNSUPPORTED - Secondary VBIOS not loaded.
**/
EFI_STATUS
LoadAndExecuteDgpuVbios (
  IN SG_VBIOS_CONFIGURATION    VbiosConfig
  )
{
  EFI_HANDLE                *HandleBuffer;
  UINTN                     HandleCount;
  UINTN                     Index;
  VBIOS_PCIR_STRUCTURE      *PcirBlockPtr;
  EFI_STATUS                Status;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  VBIOS_OPTION_ROM_HEADER   *VBiosRomImage;

  HandleBuffer = NULL;
  DgpuOpRomCopied = FALSE;

  DEBUG ((EFI_D_INFO, "SG:: LoadAndExecuteDgpuVbios\n"));

  //
  // Endpoint Device Bus#
  //
  EndpointBus = MmPci8 (0, 0, RootPortDev, RootPortFun, PCI_SBUS);

  //
  //  Endpoint Device Not found
  //
  if (EndpointBus == 0xFF) {
    DEBUG ((EFI_D_ERROR, "SG:: 0x00/0x%x/0x%x Rootport's Endpoint Device Not found\n", RootPortDev, RootPortFun));
    return EFI_UNSUPPORTED;
  }

  //
  // Check Policy setting for loading OPROM Image.
  // Note: These settings are specified in Platform specific package
  //       i.e., Load OPROM (GOP/VBIOS) from dGPU card and updated its location for ACPI methods. (Don't execute)
  //
  
  if (1)  {		//(VbiosConfig.LoadVbios != 0) {	// By default loading OPROM BIOS into memory and updating to ACPI

    DEBUG ((EFI_D_INFO, "SG:: Start to load dGPU OPROM if available\n"));

    //
    // Set as if an umcompressed video BIOS image was not obtainable.
    //
    VBiosRomImage = NULL;

    //
    // Get all PCI IO protocols
    //
    Status = gBS->LocateHandleBuffer (
                    ByProtocol,
                    &gEfiPciIoProtocolGuid,
                    NULL,
                    &HandleCount,
                    &HandleBuffer
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Find the video BIOS by checking each PCI IO handle for DGPU video BIOS OPROM.
    //
    for (Index = 0; Index < HandleCount; Index++) {
      Status = gBS->HandleProtocol (
                      HandleBuffer[Index],
                      &gEfiPciIoProtocolGuid,
                      (VOID **) &PciIo
                      );
      if (EFI_ERROR (Status) || (PciIo->RomImage == NULL)) {
        //
        // If this PCI device doesn't have a ROM image, skip to the next device.
        //
        continue;
      }

      VBiosRomImage = PciIo->RomImage;

      //
      // Get pointer to PCIR structure
      //
      PcirBlockPtr = (VBIOS_PCIR_STRUCTURE *) ((UINTN) VBiosRomImage + VBiosRomImage->PcirOffset);

      //
      // Check if we have video BIOS OPROM for DGPU.
      //
      if ((VBiosRomImage->Signature == OPTION_ROM_SIGNATURE) &&
		  (MmPci16 (0, EndpointBus, 0, 0, PCI_VID) == PcirBlockPtr->VendorId) && 
          (PcirBlockPtr->ClassCode[2] == 0x03)
          ) {

        DEBUG ((EFI_D_INFO, "SG:: Loading dGPU OPROM...\n"));

        //
        // Allocate space for copying Oprom
        //
        VbiosSize = (PcirBlockPtr->ImageLength) * 512;
        Status    = (gBS->AllocatePool) (EfiBootServicesData, VbiosSize, &VbiosAddress);
        if (EFI_ERROR (Status)) {
          break;
        }
	  
        //
        // Copy Oprom to allocated space  for the following scenario:
        // # Load vbios and Execute vbios policy setting in which dGPU execution is not called
        // # Load vbios but don't Execute vbios policy setting
        //
        if ((VbiosAddress!=NULL) && (!DgpuOpRomCopied)) {
          DEBUG ((EFI_D_INFO, "Copy Oprom to allocated space: Load policy satisfied\n"));
          (gBS->CopyMem) (VbiosAddress, PciIo->RomImage, VbiosSize);
          DgpuOpRomCopied = TRUE;
        }

        break;
      }
    }

  }
 
  //
  // Location to load Card specific ACPI tables and Install OpRegion (NVIDIA / AMD)
  //

  Status = LoadTpvAcpiTables ();

  if (VbiosAddress!=NULL) {
    (gBS->FreePool) (VbiosAddress);
  }

  if (HandleBuffer!=NULL) {
    (gBS->FreePool) (HandleBuffer);
  }

  return EFI_SUCCESS;
}


/**
  Load Intel SG SSDT Tables

  @param[in] None

  @retval EFI_SUCCESS - SG SSDT Table load successful.
**/
EFI_STATUS
LoadAcpiTables (
  VOID
  )
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    *HandleBuffer;
  BOOLEAN                       LoadTable;
  UINTN                         NumberOfHandles;
  UINTN                         Index;
  INTN                          Instance;
  UINTN                         Size;
  UINT32                        FvStatus;
  UINTN                         TableHandle;
  EFI_FV_FILETYPE               FileType;
  EFI_FV_FILE_ATTRIBUTES        Attributes;
  EFI_FIRMWARE_VOLUME_PROTOCOL  *FwVol;
  EFI_ACPI_TABLE_PROTOCOL       *AcpiTable;
  EFI_ACPI_TABLE_VERSION        Version;
  EFI_ACPI_DESCRIPTION_HEADER   *TableHeader;
  EFI_ACPI_COMMON_HEADER        *Table;

  FwVol         = NULL;
  Table         = NULL;


  DEBUG ((EFI_D_INFO, "SG:: Loading ACPI Tables...\n"));

  ///
  /// Locate FV protocol.
  ///
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareVolumeProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "SG:: Number of handles and Handlebuffer found...\n"));
  ///
  /// Look for FV with ACPI storage file
  ///
  for (Index = 0; Index < NumberOfHandles; Index++) {
    ///
    /// Get the protocol on this handle
    /// This should not fail because of LocateHandleBuffer
    ///
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiFirmwareVolumeProtocolGuid,
                    (VOID **) &FwVol
                    );
    ASSERT_EFI_ERROR (Status);
    if (FwVol == NULL) {
      return EFI_NOT_FOUND;
    }
    ///
    /// See if it has the ACPI storage file
    ///
	DEBUG ((EFI_D_INFO, "SG:: FwVol found...\n"));
    Size      = 0;
    FvStatus  = 0;
    Status = FwVol->ReadFile (
                      FwVol,
                      //&gEfiAcpiTableStorageGuid,    // AMI_OVERRIDE
                      &gSgAcpiTableGuid,              // AMI_OVERRIDE
                      NULL,
                      &Size,
                      &FileType,
                      &Attributes,
                      &FvStatus
                      );

    ///
    /// If we found it, then we are done
    ///
    if (!EFI_ERROR (Status)) {
      break;
    }
  }
  ///
  /// Our exit status is determined by the success of the previous operations
  /// If the protocol was found, Instance already points to it.
  ///
  ///
  /// Free any allocated buffers
  ///
  (gBS->FreePool) (HandleBuffer);

  ///
  /// Sanity check that we found our data file
  ///
  ASSERT (FwVol);

  ///
  /// By default, a table belongs in all ACPI table versions published.
  ///
  Version = EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0 | EFI_ACPI_TABLE_VERSION_3_0;

  ///
  /// Locate ACPI tables
  ///
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID **) &AcpiTable);

  ///
  /// Read tables from the storage file.
  ///
  if (FwVol == NULL) {
    ASSERT_EFI_ERROR (EFI_NOT_FOUND);
    return EFI_NOT_FOUND;
  }
  Instance = 0;

  while (Status == EFI_SUCCESS) {
    ///
    /// Read the ACPI tables
    ///
    Status = FwVol->ReadSection (
                      FwVol,
                      //&gEfiAcpiTableStorageGuid,    // AMI_OVERRIDE
                      &gSgAcpiTableGuid,              // AMI_OVERRIDE
                      EFI_SECTION_RAW,
                      Instance,
                      (VOID **) &Table,
                      &Size,
                      &FvStatus
                      );
    if (!EFI_ERROR (Status)) {
      ///
      /// check and load SwitchableGraphics SSDT table
      ///
      LoadTable   = FALSE;
      TableHeader = (EFI_ACPI_DESCRIPTION_HEADER *) Table;
	  if (((EFI_ACPI_DESCRIPTION_HEADER *) TableHeader)->OemTableId == EFI_SIGNATURE_64 (
          'S',
          'g',
          'P',
          'c',
          'h',
          0,
          0,
          0
          )
          ) {
        ///
        /// This is SG SSDT [dGPU is present on PCH RootPort]
        ///
        DEBUG ((EFI_D_INFO, "SG:: ---- SG SSDT ----\n"));
        DEBUG ((EFI_D_INFO, "SG:: Found out SSDT:SgPch [SgSsdtPch.asl]. dGPU is present on PCH RootPort.\n"));
        LoadTable = TRUE;
      }

      ///
      /// Add the table
      ///
      if (LoadTable) {
        TableHandle = 0;
        Status = AcpiTable->InstallAcpiTable (
                                  AcpiTable,
                                  TableHeader,
                                  TableHeader->Length,
                                  &TableHandle
                                  );
		DEBUG ((EFI_D_INFO, "SG:: ACPI Table installed...\n"));
      }
      ///
      /// Increment the instance
      ///
      Instance++;
      Table = NULL;
    }
  }

  return EFI_SUCCESS;
}


/**
  Load Third part graphics vendor support SSDT Tables

  @param[in] None

  @retval EFI_SUCCESS     - SSDT Table load successful.
  @exception EFI_UNSUPPORTED - Supported SSDT not found.
**/
EFI_STATUS
LoadTpvAcpiTables (
  VOID
  )
{
//#ifdef SG_SUPPORT
  EFI_STATUS                    Status;
  EFI_HANDLE                    *HandleBuffer;
  EFI_GUID                      AcpiTableGuid;
  BOOLEAN                       LoadTable;
  INTN                          Instance;
  UINTN                         NumberOfHandles;
  UINTN                         Index;
  UINTN                         Size;
  UINTN                         TableHandle;
  UINT16                        Data16;
  UINT32                        Data32;
  UINT32                        FvStatus;
  EFI_FIRMWARE_VOLUME_PROTOCOL  *FwVol;
  EFI_FV_FILETYPE               FileType;
  EFI_FV_FILE_ATTRIBUTES        Attributes;
  EFI_ACPI_DESCRIPTION_HEADER   *TableHeader;
  EFI_ACPI_TABLE_VERSION        Version;
  EFI_ACPI_TABLE_PROTOCOL       *AcpiTable;
  EFI_ACPI_COMMON_HEADER        *Table;

  FwVol = NULL;
  Table = NULL;

  DEBUG ((EFI_D_INFO, "SG:: Load 'Third Party graphics Vendor' support SSDT Tables [LoadTpvAcpiTables] ...\n"));

	if ((MmPci16 (0, EndpointBus, 0, 0, PCI_VID) == NVIDIA_VID)) {
    //
    // If PCH-dGPU is NVIDIA and supports HG set AcpiTableGuid to Nvidia's
    //
      DEBUG ((EFI_D_INFO, "SG:: Found a NVIDIA PCIE graphics card\n"));
      AcpiTableGuid = gNvidiaAcpiTablePchGuid;
	} else if ((MmPci16 (0, EndpointBus, 0, 0, PCI_VID) == AMD_VID)) {
    //
    // If PCH-dGPU is AMD and supports HG set AcpiTableGuid to Amd's
    //
      DEBUG ((EFI_D_INFO, "SG:: Found a AMD PCIE graphics card\n"));
      AcpiTableGuid = gAmdAcpiTablePchGuid;
    } else {
    //
    // either means the Device ID is not on the list of devices we know to ensure
    // no ACPI info gets leakout we return from this function
    //
      DEBUG ((EFI_D_INFO, "SG:: Found a PCIE graphics card [not NVIDIA/not AMD]\n"));
//#endif
      return EFI_UNSUPPORTED;
//#ifdef SG_SUPPORT
    }
  //
  // Locate FV protocol.
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareVolumeProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  ASSERT_EFI_ERROR (Status);

  ///
  /// Look for FV with ACPI storage file
  ///
  for (Index = 0; Index < NumberOfHandles; Index++) {
    ///
    /// Get the protocol on this handle
    /// This should not fail because of LocateHandleBuffer
    ///
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiFirmwareVolumeProtocolGuid,
                    &FwVol
                    );
    ASSERT_EFI_ERROR (Status);
    if (FwVol == NULL) {
       return EFI_NOT_FOUND;
    }

    ///
    /// See if it has the ACPI storage file
    ///
    Size      = 0;
    FvStatus  = 0;
    Status = FwVol->ReadFile (
                      FwVol,
                      &AcpiTableGuid,
                      NULL,
                      &Size,
                      &FileType,
                      &Attributes,
                      &FvStatus
                      );

    ///
    /// If we found it, then we are done
    ///
    if (!EFI_ERROR (Status)) {
      break;
    }
  }
  ///
  /// Our exit status is determined by the success of the previous operations
  /// If the protocol was found, Instance already points to it.
  ///
  ///
  /// Free any allocated buffers
  ///
  (gBS->FreePool) (HandleBuffer);

  ///
  /// Sanity check that we found our data file
  ///
  if (FwVol == NULL) {
      ASSERT_EFI_ERROR (EFI_NOT_FOUND);
      return EFI_NOT_FOUND;
  }

  ///
  /// By default, a table belongs in all ACPI table versions published.
  ///
  Version = EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0 | EFI_ACPI_TABLE_VERSION_3_0;

  ///
  /// Locate ACPI tables
  ///
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, &AcpiTable);

  ///
  /// Read tables from the storage file.
  ///
  Instance = 0;

  while (Status == EFI_SUCCESS) {
    ///
    /// Read the ACPI tables
    ///
    Status = FwVol->ReadSection (
                      FwVol,
                      &AcpiTableGuid,
                      EFI_SECTION_RAW,
                      Instance,
                      &Table,
                      &Size,
                      &FvStatus
                      );
    if (!EFI_ERROR (Status)) {
      ///
      /// check for SwitchableGraphics tables and decide which SSDT should be loaded
      ///
      LoadTable   = FALSE;
      TableHeader = (EFI_ACPI_DESCRIPTION_HEADER *) Table;

      switch (((EFI_ACPI_DESCRIPTION_HEADER *) TableHeader)->OemTableId) {

      case EFI_SIGNATURE_64 ('N', 'v', 'd', 'P', 'c', 'h', 0, 0):
        ///
        /// This is Nvidia SSDT
        ///
        DEBUG ((EFI_D_INFO, "SG:: ---- Nvidia SSDT ----\n"));

        if ((((EFI_ACPI_DESCRIPTION_HEADER *) TableHeader)->OemTableId) == EFI_SIGNATURE_64 ('N', 'v', 'd', 'P', 'c', 'h', 0, 0)) {
          DEBUG ((EFI_D_INFO, "SG:: Found out SSDT:NvdPch [NvsgPch.asl]. dGPU is present on PCH RootPort.\n"));
        }

        LoadTable = TRUE;
        Status    = InstallNvidiaOpRegion ();
        if (EFI_ERROR (Status)) {
          return Status;
        }
        break;

      case EFI_SIGNATURE_64 ('A', 'm', 'd', 'P', 'c', 'h', 0, 0):
        ///
        /// This is Amd SSDT
        ///
        DEBUG ((EFI_D_INFO, "SG:: ---- AMD SSDT ----\n"));

        if ((((EFI_ACPI_DESCRIPTION_HEADER *) TableHeader)->OemTableId) == EFI_SIGNATURE_64 ('A', 'm', 'd', 'P', 'c', 'h', 0, 0)) {
          DEBUG ((EFI_D_INFO, "SG:: Found out SSDT:AmdPch [AmdpxPch.asl]. dGPU is present on PCH RootPort.\n"));
        }

        LoadTable = TRUE;
        Status    = InstallAmdOpRegion ();
        if (EFI_ERROR (Status)) {
          return Status;
        }
        ///
        /// Store the Root port Bus assignment for S3 resume path
        ///
		Data32 = MmPci32 (0, 0, RootPortDev, RootPortFun, PCI_PBUS);
    	S3BootScriptSaveMemWrite (
      		S3BootScriptWidthUint32,
      		(UINTN) (MmPciAddress (0x0,
                  0,
                  RootPortDev,
                  RootPortFun,
                  PCI_PBUS)),
      			  1,
      			  &Data32
      		);

        Data16 = MmPci16 (0, 0, RootPortDev, RootPortFun, PCI_BAR3);
        S3BootScriptSaveMemWrite (
        	S3BootScriptWidthUint16,
        	(UINTN) (MmPciAddress (0x0,
                  0,
                  RootPortDev,
                  RootPortFun,
                  PCI_BAR3)),
      			1,
      			&Data16
      			);

        ///
        /// Set a unique SSID on the AMD MXM
        ///
        Data16  = McD2PciCfg16 (PCI_SVID);
		MmPci16 (0, EndpointBus, 0, 0, AMD_SVID_OFFSET) = Data16;
    		S3BootScriptSaveMemWrite (
      		S3BootScriptWidthUint16,
          (UINTN) (MmPciAddress (0x0,
                  EndpointBus,
                  0,
                  0,
                  AMD_SVID_OFFSET)),
      			1,
      			&Data16
     			);
        DEBUG ((EFI_D_INFO, "SG:: AMD MXM SVID [Subsystem Vendor ID]: 0x%x\n", Data16));

        Data16  = McD2PciCfg16 (PCI_SID);
		MmPci16 (0, EndpointBus, 0, 0, AMD_SDID_OFFSET) = Data16;
 	   	S3BootScriptSaveMemWrite (
      		S3BootScriptWidthUint16,
          	(UINTN) (MmPciAddress (0x0,
                  EndpointBus,
                  0,
                  0,
                  AMD_SDID_OFFSET)),
      			1,
      			&Data16
      			);
        DEBUG ((EFI_D_INFO, "SG:: AMD MXM SDID [Subsystem ID]: 0x%x\n", Data16));

        break;

      default:
        break;
      }
	  //
      // Add the table
      //
      if (LoadTable) {
        TableHandle = 0;
        Status = AcpiTable->InstallAcpiTable (
                                  AcpiTable,
                                  TableHeader,
                                  TableHeader->Length,
                                  &TableHandle
                                  );
		DEBUG ((EFI_D_INFO, "SG:: Installed ACPI Table\n"));
      }
      ///
      /// Increment the instance
      ///
      Instance++;
      Table = NULL;
    }
  }

  return EFI_SUCCESS;
//#endif
}

/**
  Nvidia Graphics OpRegion installation function.

  @param[in] None

  @retval EFI_SUCCESS     The driver installed without error.
  @retval EFI_ABORTED     The driver encountered an error and could not complete
                  installation of the ACPI tables.
**/
EFI_STATUS
InstallNvidiaOpRegion (
  VOID
  )
{
  EFI_STATUS                             Status;
  UINTN                                  Size;
  EFI_GLOBAL_NVS_AREA_PROTOCOL          *GlobalNvsArea;
  NVIDIA_OPREGION                        NvidiaOpRegion;

  ///
  ///  Locate Global NVS Protocol.
  ///
  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  &GlobalNvsArea
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ///
  /// Allocate an ACPI NVS memory buffer as the Nvidia NVIG OpRegion, zero initialize
  /// the entire 1K, and set the Nvidia NVIG OpRegion pointer in the Global NVS
  /// area structure.
  ///
  Size    = sizeof (NVIG_OPREGION);
  Status  = (gBS->AllocatePool) (EfiACPIMemoryNVS, Size, &NvidiaOpRegion.NvIgOpRegion);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  (gBS->SetMem) (NvidiaOpRegion.NvIgOpRegion, Size, 0);

  ///
  /// Set up DeviceID values for _DOD.
  /// Note that Display Subtype bits[15-12] and Port index bits[7:4] are set as per NV Switchable 3.0 spec.
  /// Not used by Intel driver.
  ///
  /// Display Type - CRT
  ///
  GlobalNvsArea->Area->DeviceId1 = 0x80010100;

//  if (GlobalNvsArea->Area->ActiveLFP == 3) {
    ///
    /// If Active LFP = EDP_A
    ///
    /// Display type - LFP Device Sub Type - eDP
    ///
    GlobalNvsArea->Area->DeviceId2 = 0x8001A420;
//  } else {
    ///
    /// Display Type - LFP Device Sub Type - LVDS
    ///
//    GlobalNvsArea->Area->DeviceId2 = 0x80010410;
//  }
  ///
  /// Display type - EFP  Device Sub type - DisplayPort 1.1
  ///
  GlobalNvsArea->Area->DeviceId3 = 0x80016330;

  ///
  /// Display type - EFP  Device Sub type - HDMI 1.2 or 1.3a
  ///
  GlobalNvsArea->Area->DeviceId4 = 0x80017331;

  ///
  /// Display type - EFP  Device Sub type - HDMI 1.2 or 1.3a
  ///
  GlobalNvsArea->Area->DeviceId5 = 0x80017342;

  ///
  /// Display type - EFP  Device Sub type - DisplayPort 1.1
  ///
  GlobalNvsArea->Area->DeviceId6 = 0x80016353;

  ///
  /// Display type - EFP  Device Sub type - HDMI 1.2 or 1.3a
  ///
  GlobalNvsArea->Area->DeviceId7 = 0x80017354;

  ///
  /// DeviceId8 is not being used on Calpella SG
  ///
  GlobalNvsArea->Area->DeviceId8 = 0x0;

  ///
  /// NDID
  ///
  GlobalNvsArea->Area->NumberOfValidDeviceId = 0x7;

  ///
  /// NVIG
  ///
  GlobalNvsArea->Area->NvIgOpRegionAddress = (UINT32) (UINTN) (NvidiaOpRegion.NvIgOpRegion);

  ///
  /// NVIG Header
  ///
  (gBS->CopyMem) (NvidiaOpRegion.NvIgOpRegion->NISG, NVIG_HEADER_SIGNATURE, sizeof (NVIG_HEADER_SIGNATURE));
  NvidiaOpRegion.NvIgOpRegion->NISZ = NVIG_OPREGION_SIZE;
  NvidiaOpRegion.NvIgOpRegion->NIVR = NVIG_OPREGION_VER;

  ///
  /// Panel Scaling Preference
  ///
  NvidiaOpRegion.NvIgOpRegion->GPSP = GlobalNvsArea->Area->IgdPanelScaling;

  ///
  /// Allocate an ACPI NVS memory buffer as the Nvidia NVHM OpRegion, zero initialize
  /// the entire 62K, and set the Nvidia NVHM OpRegion pointer in the Global NVS
  /// area structure.
  ///
  Size    = sizeof (NVHM_OPREGION);
  Status  = (gBS->AllocatePool) (EfiACPIMemoryNVS, Size, &NvidiaOpRegion.NvHmOpRegion);
  if (EFI_ERROR (Status)) {
    (gBS->FreePool) (NvidiaOpRegion.NvIgOpRegion);
    return Status;
  }

  (gBS->SetMem) (NvidiaOpRegion.NvHmOpRegion, Size, 0);

  ///
  /// NVHM
  ///
  GlobalNvsArea->Area->NvHmOpRegionAddress = (UINT32) (UINTN) (NvidiaOpRegion.NvHmOpRegion);

  ///
  /// NVHM Header Signature, Size, Version
  ///
  (gBS->CopyMem) (NvidiaOpRegion.NvHmOpRegion->NVSG, NVHM_HEADER_SIGNATURE, sizeof (NVHM_HEADER_SIGNATURE));
  NvidiaOpRegion.NvHmOpRegion->NVSZ = NVHM_OPREGION_SIZE;
  NvidiaOpRegion.NvHmOpRegion->NVVR = NVHM_OPREGION_VER;

  ///
  /// NVHM opregion address
  ///
  NvidiaOpRegion.NvHmOpRegion->NVHO = (UINT32) (UINTN) (NvidiaOpRegion.NvHmOpRegion);

  ///
  /// Copy Oprom to allocated space in NV Opregion
  ///
  NvidiaOpRegion.NvHmOpRegion->RVBS = VbiosSize;
  if (VbiosAddress != NULL) {
	(gBS->CopyMem) ((VOID *) (UINTN) NvidiaOpRegion.NvHmOpRegion->RBUF, VbiosAddress, NvidiaOpRegion.NvHmOpRegion->RVBS);
  }
  return Status;
}

/**
  AMD graphics OpRegion installation function.

  @param[in] None

  @retval EFI_SUCCESS     The driver installed without error.
  @retval EFI_ABORTED     The driver encountered an error and could not complete
                  installation of the ACPI tables.
**/
EFI_STATUS
InstallAmdOpRegion (
  VOID
  )
{
  EFI_STATUS                             Status;
  UINTN                                  Size;
  EFI_GLOBAL_NVS_AREA_PROTOCOL          *GlobalNvsArea;
  AMD_OPREGION                           AmdOpRegion;

  ///
  ///  Locate Global NVS Protocol.
  ///
  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  &GlobalNvsArea
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ///
  /// Allocate an ACPI NVS memory buffer as the Amd APXM OpRegion, zero initialize
  /// the entire 1K, and set the Amd APXM OpRegion pointer in the Global NVS
  /// area structure.
  ///
  Size    = sizeof (APXM_OPREGION);
  Status  = (gBS->AllocatePool) (EfiACPIMemoryNVS, Size, &AmdOpRegion.ApXmOpRegion);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  (gBS->SetMem) (AmdOpRegion.ApXmOpRegion, Size, 0);

  ///
  /// APXM address
  ///
  GlobalNvsArea->Area->ApXmOpRegionAddress  = (UINT32) (UINTN) (AmdOpRegion.ApXmOpRegion);
  AmdOpRegion.ApXmOpRegion->APXA            = (UINT32) (UINTN) (AmdOpRegion.ApXmOpRegion);

  ///
  /// Set up DIDx values for _DOD
  ///
  /// Device ID - CRT on IGPU
  ///
  GlobalNvsArea->Area->DeviceId1 = 0x80010100;

  ///
  /// Device ID - LFP (LVDS or eDP)
  ///
  GlobalNvsArea->Area->DeviceId2 = 0x80010400;

  ///
  /// Display type - EFP  Device Sub type - DisplayPort 1.1
  ///
  GlobalNvsArea->Area->DeviceId3 = 0x80010300;

  ///
  /// Display type - EFP  Device Sub type - HDMI 1.2 or 1.3a
  ///
  GlobalNvsArea->Area->DeviceId4 = 0x80010301;

  ///
  /// Display type - EFP  Device Sub type - HDMI 1.2 or 1.3a
  ///
  GlobalNvsArea->Area->DeviceId5 = 0x80010302;

  ///
  /// Display type - EFP  Device Sub type - DisplayPort 1.1
  ///
  GlobalNvsArea->Area->DeviceId6 = 0x80010303;

  ///
  /// Display type - EFP  Device Sub type - HDMI 1.2 or 1.3a
  ///
  GlobalNvsArea->Area->DeviceId7 = 0x80010304;

  ///
  /// DeviceId8 is not being used on Calpella SG
  ///
  GlobalNvsArea->Area->DeviceId8 = 0x0;

  ///
  /// NDID
  ///
  GlobalNvsArea->Area->NumberOfValidDeviceId = 0x7;

  ///
  /// APXM Header
  ///
  (gBS->CopyMem) (AmdOpRegion.ApXmOpRegion->APSG, APXM_HEADER_SIGNATURE, sizeof (APXM_HEADER_SIGNATURE));
  AmdOpRegion.ApXmOpRegion->APSZ  = APXM_OPREGION_SIZE;
  AmdOpRegion.ApXmOpRegion->APVR  = APXM_OPREGION_VER;

  ///
  /// Total number of toggle list entries
  ///
  AmdOpRegion.ApXmOpRegion->NTLE = 15;

  ///
  /// The display combinations in the list...
  ///
  AmdOpRegion.ApXmOpRegion->TLEX[0] = 0x0002; ///< CRT
  AmdOpRegion.ApXmOpRegion->TLEX[1] = 0x0001; ///< LFP
  AmdOpRegion.ApXmOpRegion->TLEX[2] = 0x0008; ///< DP_B
  AmdOpRegion.ApXmOpRegion->TLEX[3] = 0x0080; ///< HDMI_B
  AmdOpRegion.ApXmOpRegion->TLEX[4] = 0x0200; ///< HDMI_C
  AmdOpRegion.ApXmOpRegion->TLEX[5] = 0x0400; ///< DP_D
  AmdOpRegion.ApXmOpRegion->TLEX[6] = 0x0800; ///< HDMI_D
  AmdOpRegion.ApXmOpRegion->TLEX[7] = 0x0003; ///< LFP+CRT
  AmdOpRegion.ApXmOpRegion->TLEX[8] = 0x0009; ///< LFP+DP_B
  AmdOpRegion.ApXmOpRegion->TLEX[9] = 0x0081; ///< LFP+HDMI_B
  AmdOpRegion.ApXmOpRegion->TLEX[10] = 0x0201; ///< LFP+HDMI_C
  AmdOpRegion.ApXmOpRegion->TLEX[11] = 0x0401; ///< LFP+DP_D
  AmdOpRegion.ApXmOpRegion->TLEX[12] = 0x0801; ///< LFP+HDMI_D
  AmdOpRegion.ApXmOpRegion->TLEX[13] = 0x0; ///< Dummy 1
  AmdOpRegion.ApXmOpRegion->TLEX[14] = 0x0; ///< Dummy 2

  ///
  /// Panel Scaling Preference
  ///
  AmdOpRegion.ApXmOpRegion->EXPM = GlobalNvsArea->Area->IgdPanelScaling;

  ///
  /// Copy Oprom to allocated space in ATI Opregion
  ///
  AmdOpRegion.ApXmOpRegion->RVBS = VbiosSize;
  if (VbiosAddress != NULL) {
  (gBS->CopyMem) ((VOID *) (UINTN) AmdOpRegion.ApXmOpRegion->RBUF, VbiosAddress, AmdOpRegion.ApXmOpRegion->RVBS);
  }
  return Status;
}


VOID
EFIAPI
SgExitPmAuthCallback (
  IN EFI_EVENT Event,
  IN VOID      *Context
  )
{
  EFI_STATUS  Status;
  VOID        *ProtocolPointer;

  ///
  /// Check if this is first time called by EfiCreateProtocolNotifyEvent() or not,
  /// if it is, we will skip it until real event is triggered
  ///
  Status = gBS->LocateProtocol (&gExitPmAuthProtocolGuid, NULL, &ProtocolPointer);
  if (EFI_SUCCESS != Status) {
     return;
  }

  gBS->CloseEvent (Event);

  DEBUG ((EFI_D_INFO, "SG:: ExitPmAuth Callback\n"));
  ///
  /// Load and Execute dGPU VBIOS
  ///
  Status = LoadAndExecuteDgpuVbios (DxePlatformSaPolicy->VbiosConfig);
}

#endif //SG_SUPPORT
