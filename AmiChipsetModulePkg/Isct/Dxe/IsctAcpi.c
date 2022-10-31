//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
// Revision History
// ----------------
// $Log: $
//
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:  IsctAcpi.c
//
// Description:	This Dxe driver will initialize ISCT Persistent Data Variable and also verify ISCT Store valid or not
//
//<AMI_FHDR_END>
//**********************************************************************

#include "IsctAcpi.h"
ISCT_NVS_AREA_PROTOCOL mIsctNvsAreaProtocol;
EFI_PLATFORM_INFO_HOB         *mPlatformInfo;

EFI_STATUS
IsctDxeUpdateSetupVariableToACPIGNVS (
    IN OUT SETUP_DATA                        *gSetupData
)
/*++

Routine Description:

  Update ISCT SetupVariable to ACPI GNVS

Arguments:

Returns:

  EFI_SUCCESS    Isct ACPI GNVS are updated successfully
  EFI_NOT_FOUND  Isct ACPI GNVS not found

--*/
{
    EFI_STATUS                    Status;
//  EFI_GUID                      gEfiGlobalNvsAreaProtocolGuid = EFI_GLOBAL_NVS_AREA_PROTOCOL_GUID;
    EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsAreaProtocol;
    EFI_GLOBAL_NVS_AREA           *mGlobalNvsAreaPtr;

    Status = gBS->LocateProtocol( &gEfiGlobalNvsAreaProtocolGuid, NULL, &GlobalNvsAreaProtocol );
    if ( EFI_ERROR(Status) ) {
        return Status;
    }
    mGlobalNvsAreaPtr = GlobalNvsAreaProtocol->Area;

    //
    // Intel Smart Connect Technology 4.0 Spec (Document Number: 507302)
    //
    // Table 4-2. IAOE Control Method GABS
    // Bit0    Intel Smart Connect Technology Configured: 0 = Disabled, 1 = Enabled
    // Bit1    Intel Smart Connect Technology Notification Control: 0 = Unsupported, 1 = Supported
    // Bit2    Intel Smart Connect Technology WLAN Power Control:0 = Unsupported, 1 = Supported
    // Bit3    Intel Smart Connect Technology WWAN Power Control: 0 = Unsupported, 1 = Supported
    // Bit4    Must be set to 1
    // Bit5    Sleep duration value format: 0 = Actual time, 1 = duration in seconds (see SASD for actual format)
    // Bit6    RF Kill Support (Radio On/Off): 0 = Soft Switch, 1 = Physical Switch
    // Bit7    Reserved (must set to 0)
    //
    // ISCT configuration
    //
    mGlobalNvsAreaPtr->IsctCfg = 0;
    if (gSetupData->IsctConfiguration) {
        mGlobalNvsAreaPtr->IsctCfg                       = mGlobalNvsAreaPtr->IsctCfg | BIT0;

        if (gSetupData->ISCTNOTIFICATION) {
            mGlobalNvsAreaPtr->IsctCfg                     = mGlobalNvsAreaPtr->IsctCfg | BIT1;
        }
        if (gSetupData->ISCTWLAN) {
            mGlobalNvsAreaPtr->IsctCfg                     = mGlobalNvsAreaPtr->IsctCfg | BIT2;
        }
        if (gSetupData->ISCTWWAN) {
            mGlobalNvsAreaPtr->IsctCfg                     = mGlobalNvsAreaPtr->IsctCfg | BIT3;
        }
        
        mGlobalNvsAreaPtr->IsctCfg                       = mGlobalNvsAreaPtr->IsctCfg | BIT4;
        
        if (gSetupData->ISCTSleepFormat) {
            mGlobalNvsAreaPtr->IsctCfg                       = mGlobalNvsAreaPtr->IsctCfg | BIT5;  // Duration in seconds
        }

        if (gSetupData->ISCTRFKillSwitch) {
            mGlobalNvsAreaPtr->IsctCfg                     = mGlobalNvsAreaPtr->IsctCfg | BIT6;
        }
    }

    return Status;
}

STATIC
EFI_STATUS
InitializeIsctAcpiTables (
    VOID
)
/*++

Routine Description:

  Initialize ISCT ACPI tables

Arguments:

Returns:

  EFI_SUCCESS    Isct ACPI tables are initialized successfully
  EFI_NOT_FOUND  Isct ACPI tables not found

--*/
{
    EFI_STATUS                    Status;
    EFI_HANDLE                    *HandleBuffer;
    UINTN                         NumberOfHandles;
    EFI_FV_FILETYPE               FileType;
    UINT32                        FvStatus;
    EFI_FV_FILE_ATTRIBUTES        Attributes;
    UINTN                         Size;
    UINTN                         Index;
    EFI_FIRMWARE_VOLUME_PROTOCOL  *FwVol;
    INTN                          Instance;
    EFI_ACPI_COMMON_HEADER        *CurrentTable;
    UINT8                         *CurrPtr;
    UINT8                         *EndPtr;
    UINT32                        *Signature;
    EFI_ACPI_DESCRIPTION_HEADER   *IsctAcpiTable;
    BOOLEAN                       LoadTable;
    UINTN                         TableHandle;
    EFI_ACPI_TABLE_VERSION        Version;
    EFI_ACPI_TABLE_PROTOCOL       *AcpiTable;

    FwVol         = NULL;
    IsctAcpiTable = NULL;

    Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, &AcpiTable);
    if ( EFI_ERROR(Status) ) {
        DEBUG((EFI_D_INFO, "ISCT :gBS->LocateProtocol -> AcpiTable Status = %r\n", Status));
        return Status;
    }

    //
    // Locate protocol.
    // There is little chance we can't find an FV protocol
    //
    Status = gBS->LocateHandleBuffer (
                 ByProtocol,
                 &gEfiFirmwareVolumeProtocolGuid,
                 NULL,
                 &NumberOfHandles,
                 &HandleBuffer
             );
    if ( EFI_ERROR(Status) ) {
        DEBUG((EFI_D_INFO, "ISCT :gBS->LocateHandleBuffer Status = %r\n", Status));
        return Status;
    }

    //
    // Looking for FV with ACPI storage file
    //
    for (Index = 0; Index < NumberOfHandles; Index++) {
        //
        // Get the protocol on this handle
        // This should not fail because of LocateHandleBuffer
        //
        Status = gBS->HandleProtocol (
                     HandleBuffer[Index],
                     &gEfiFirmwareVolumeProtocolGuid,
                     &FwVol
                 );
        if ( EFI_ERROR(Status) ) {
            DEBUG((EFI_D_INFO, "ISCT :gBS->HandleProtocol Status = %r\n", Status));
            return Status;
        }

        //
        // See if it has the ACPI storage file
        //
        Size      = 0;
        FvStatus  = 0;
        Status = FwVol->ReadFile (
                     FwVol,
                     &gIsctAcpiTableStorageGuid,
                     NULL,
                     &Size,
                     &FileType,
                     &Attributes,
                     &FvStatus
                 );
        DEBUG((EFI_D_INFO, "ISCT :FwVol->ReadFile Status = %r\n", Status));
        //
        // If we found it, then we are done
        //
        if (Status == EFI_SUCCESS) {
            break;
        }
    }
    //
    // Free any allocated buffers
    //
    FreePool (HandleBuffer);

    //
    // Sanity check that we found our data file
    //
    if (FwVol == NULL) {
        return EFI_NOT_FOUND;
    }
    //
    // By default, a table belongs in all ACPI table versions published.
    //
    Version = EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0 | EFI_ACPI_TABLE_VERSION_3_0;

    //
    // Our exit status is determined by the success of the previous operations
    // If the protocol was found, Instance already points to it.
    // Read tables from the storage file.
    //
    Instance      = 0;
    CurrentTable  = NULL;
    while (Status == EFI_SUCCESS) {
        Status = FwVol->ReadSection (
                     FwVol,
                     &gIsctAcpiTableStorageGuid,
                     EFI_SECTION_RAW,
                     Instance,
                     &CurrentTable,
                     &Size,
                     &FvStatus
                 );
        DEBUG((EFI_D_INFO, "ISCT :FwVol->ReadSection Status = %r\n", Status));

        if (!EFI_ERROR (Status)) {
            LoadTable = FALSE;
            //
            // Check the table ID to modify the table
            //
            if (((EFI_ACPI_DESCRIPTION_HEADER *) CurrentTable)->OemTableId == EFI_SIGNATURE_64 ('I', 's', 'c', 't', 'T', 'a', 'b', 'l')) {
                IsctAcpiTable = (EFI_ACPI_DESCRIPTION_HEADER *) CurrentTable;
                DEBUG((EFI_D_ERROR, "ISCT :Find out IsctTabl\n"));
                //
                // Locate the SSDT package
                //
                CurrPtr = (UINT8 *) IsctAcpiTable;
                EndPtr  = CurrPtr + IsctAcpiTable->Length;

                for (; CurrPtr <= EndPtr; CurrPtr++) {
                    Signature = (UINT32 *) (CurrPtr + 3);
                    if (*Signature == EFI_SIGNATURE_32 ('I', 'S', 'C', 'T')) {
                        LoadTable = TRUE;
                        if((*(UINT32 *) (CurrPtr + 3 + sizeof (*Signature) + 2) == 0xFFFF0008)) {
                            //
                            // ISCT NVS Area address
                            //
                            *(UINT32 *) (CurrPtr + 3 + sizeof (*Signature) + 2) = (UINT32) (UINTN) mIsctNvsAreaProtocol.Area;
                            DEBUG((EFI_D_INFO, "ISCT :Modify OpRegion Address to %x\n", (*(UINT32 *) (CurrPtr + 3 + sizeof (*Signature) + 2))));
                        }

                        if((*(UINT16 *) (CurrPtr + 3 + sizeof (*Signature) + 2 + sizeof (UINT32) + 1) == 0xAA58)) {
                            //
                            // ISCT NVS Area size
                            //
                            *(UINT16 *) (CurrPtr + 3 + sizeof (*Signature) + 2 + sizeof (UINT32) + 1) = sizeof (ISCT_NVS_AREA);
                            DEBUG((EFI_D_INFO, "ISCT :Modify OpRegion Size to %x\n", *(UINT16 *) (CurrPtr + 3 + sizeof (*Signature) + 2 + sizeof (UINT32) + 1)));
                        }

                        ///
                        /// Add the table
                        ///
                        if (LoadTable) {
                            TableHandle = 0;
                            Status = AcpiTable->InstallAcpiTable (
                                         AcpiTable,
                                         CurrentTable,
                                         CurrentTable->Length,
                                         &TableHandle
                                     );
                            if ( EFI_ERROR(Status) ) {
                                return Status;
                            }
                        }
                        return EFI_SUCCESS;
                    }
                }
            }
            //
            // Increment the instance
            //
            Instance++;
            CurrentTable = NULL;
        }
    }

    return Status;
}

VOID
IsctOnReadyToBoot (
    IN EFI_EVENT  Event,
    IN VOID       *Context
)
/*++

Routine Description:

  Install Isct ACPI tables only when Isct is enabled

Arguments:

  Event    - The event that triggered this notification function
  Context  - Pointer to the notification functions context

Returns:

  None

--*/
{
    EFI_STATUS  Status;
//  IGD_OPREGION_PROTOCOL         *IgdOpRegionProtocol;

    DEBUG ((EFI_D_INFO, "IsctOnReadyToBoot()\n"));

    Status = InitializeIsctAcpiTables ();
    if ( EFI_ERROR(Status) ) {
        DEBUG((EFI_D_INFO, "Initializes ISCT SSDT tables Status = %r\n", Status));
        return;
    }

    gBS->CloseEvent (Event);

    //
    // Notify the Graphics Driver that Isct is enabled
    //
    /*
      Status = gBS->LocateProtocol (
                    &gIgdOpRegionProtocolGuid,
                    NULL,
                    &IgdOpRegionProtocol
                    );
      if (Status == EFI_SUCCESS) {
        IgdOpRegionProtocol->OpRegion->Header.PCON |= 0x60;
        DEBUG((EFI_D_INFO, "IsctOnReadyToBoot() PCON = 0x%x\n", IgdOpRegionProtocol->OpRegion->Header.PCON));
      } else {
        DEBUG ((EFI_D_ERROR, "IsctOnReadyToBoot() Unable to locate IgdOpRegionProtocol"));
      }
    */
}

EFI_STATUS
IsctDxeEntryPoint (
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
/*++

Routine Description:

  ISCT DXE driver entry point function

Arguments:

  ImageHandle - Image handle for this driver image
  SystemTable - Pointer to the EFI System Table

Returns:

  EFI_OUT_OF_RESOURCES - no enough memory resource when installing reference code information protocol
  EFI_SUCCESS          - function completed successfully

--*/
{
    EFI_STATUS                    Status;
    EFI_EVENT                     Event;
    ISCT_NVS_AREA                 *IsctNvs;
    ISCT_PERSISTENT_DATA          *mIsctData;
    UINT8                         IsctEnabled;

    SETUP_DATA                    SetupData;
    UINTN                         VarSize;
    EFI_PEI_HOB_POINTERS          Hob;

    DEBUG ((EFI_D_INFO, "IsctDxe: Entry Point...\n"));

    VarSize = sizeof (SETUP_DATA);
    Status = gRT->GetVariable (
                 L"Setup",
                 &gSetupGuid,
                 NULL,
                 &VarSize,
                 &SetupData
             );
    if ( EFI_ERROR(Status) ) {
        DEBUG ((EFI_D_INFO, "ISCT: Get Variable Status = %r\n", Status));
        return Status;
    }

    IsctEnabled = SetupData.IsctConfiguration;

    if(IsctEnabled == 0) {
        DEBUG ((EFI_D_INFO, "ISCT is Disabled \n"));
        return EFI_SUCCESS;
    }

    Hob.Raw = GetFirstGuidHob(&gEfiPlatformInfoGuid);
    ASSERT(Hob.Raw != NULL);
    mPlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);
    
    //
    // Allocate pools for ISCT Global NVS area
    //
    Status = (gBS->AllocatePool) (EfiReservedMemoryType, sizeof (ISCT_NVS_AREA), &mIsctNvsAreaProtocol.Area);
    if ( EFI_ERROR (Status) ) {
        DEBUG ((EFI_D_ERROR, "Error to allocate pool for ISCT_NVS_AREA"));
        ASSERT_EFI_ERROR (Status);
        return Status;
    }
    ZeroMem ((VOID *) mIsctNvsAreaProtocol.Area, sizeof (ISCT_NVS_AREA));

    Status = (gBS->AllocatePool) (EfiReservedMemoryType, sizeof (ISCT_PERSISTENT_DATA), &mIsctNvsAreaProtocol.IsctData);
    if ( EFI_ERROR (Status) ) {
        DEBUG ((EFI_D_ERROR, "Error to allocate pool for ISCT_PERSISTENT_DATA"));
        ASSERT_EFI_ERROR (Status);
        return Status;
    }
    ZeroMem ((VOID *) mIsctNvsAreaProtocol.IsctData, sizeof (ISCT_PERSISTENT_DATA));

    IsctNvs = mIsctNvsAreaProtocol.Area;

    IsctNvs->IsctNvsPtr = (UINT32) (UINTN) IsctNvs;

#if IsctSmm_SUPPORT
    IsctNvs->IsctRTCTimerSupport = SetupData.IsctRTCTimerSupport; //get timer choice from setup options
    if(mPlatformInfo->PlatformFlavor == FlavorDesktop) {
      IsctNvs->IsctRTCTimerSupport = 1; //use RTC timer for Desktop
    }
#endif

    IsctNvs->IsctEnabled = SetupData.IsctConfiguration;
    
    //
    // Assign IsctData pointer to GlobalNvsArea
    //
    mIsctData = mIsctNvsAreaProtocol.IsctData;
    mIsctData->IsctNvsPtr = (UINT32) (UINTN) IsctNvs;

    //
    // Install ISCT Global NVS protocol
    //

    Status = gBS->InstallMultipleProtocolInterfaces (
                 &ImageHandle,
                 &gIsctNvsAreaProtocolGuid,
                 &mIsctNvsAreaProtocol,
                 NULL
             );
    DEBUG((EFI_D_INFO, "Install IsctNvsAreaProtocolGuid = %r\n", Status));

    if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "Error to install ISCT_NVS_AREA_PROTOCOL"));
        ASSERT_EFI_ERROR (Status);
        return Status;
    }

    //
    // Save ISCT Data to Variable
    //
    Status = gRT->SetVariable (
                 ISCT_PERSISTENT_DATA_NAME,
                 &gIsctPersistentDataGuid,
                 EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS, //EIP168675
                 sizeof (ISCT_PERSISTENT_DATA),
                 mIsctData
             );
    if (EFI_ERROR (Status)) {
        DEBUG((EFI_D_INFO, "ISCT DXE: Save ISCT Data to Variable Status = %r\n", Status));
        return Status;
    }

    //
    // Update SetupVariable to ACPI GNVS
    //
    Status = IsctDxeUpdateSetupVariableToACPIGNVS (&SetupData);
    if ( EFI_ERROR(Status) ) {
        return Status;
    }

    //
    // Register ready to boot event for ISCT
    //

    Status = EfiCreateEventReadyToBootEx (
                 EFI_TPL_NOTIFY,
                 IsctOnReadyToBoot,
                 NULL,
                 &Event
             );
    DEBUG((EFI_D_INFO, "Create ReadyToBoot event for ISCT Status = %r\n", Status));
    if (EFI_ERROR (Status)) {
        ASSERT_EFI_ERROR (Status);
        return Status;
    }


    DEBUG ((EFI_D_INFO, "(IsctDxe) entry End...\n"));

    return EFI_SUCCESS;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
