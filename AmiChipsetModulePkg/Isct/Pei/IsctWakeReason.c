//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
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
// Name:  IsctWakeReason.c
//
// Description:	Provide Wake Reason for ISCT.
//
//<AMI_FHDR_END>
//**********************************************************************

#include <Library/BaseLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/HobLib.h>

#include "PchAccess.h"
#include "Pci.h"

#include <IsctPersistentData.h>
#include <Ppi/ReadOnlyVariable2.h>

#include <token.h>
#include <Setup.h>
#include <Pei.h>

#define KSC_D_PORT      0x62
#define KSC_C_PORT      0x66
#define KSC_S_IBF       0x02  /// Input buffer is full/empty
#define KSC_S_OBF       0x01  /// Output buffer is full/empty
#define KSC_TIME_OUT    0x20000

#define ISCT_WAKE_REASON_UNKNOWN               0x00
#define ISCT_WAKE_REASON_USER_EVENT            0x01 
#define ISCT_WAKE_REASON_PERIODIC_WAKE         0x02 
#define ISCT_WAKE_REASON_RTC_TIMER             0x04
#define ISCT_WAKE_REASON_PME_WAKE              0x08
#define ISCT_WAKE_REASON_DISPLAY_OFF           0x10

#if !defined(EDK_RELEASE_VERSION) || (EDK_RELEASE_VERSION < 0x00020000)
//
// GUID to AMI_ISCT Module
//
#define AMI_ISCT_HOB_GUID \
  { \
    0x1af7b744, 0xcdfc, 0x4825, 0xa1, 0x77, 0xca, 0x48, 0xd2, 0xdf, 0xe2, 0xc6 \
  }
#else
#define AMI_ISCT_HOB_GUID \
  { \
    0x1af7b744, 0xcdfc, 0x4825, \
    { \
      0xa1, 0x77, 0xca, 0x48, 0xd2, 0xdf, 0xe2, 0xc6 \
    } \
  }
#endif

#pragma pack (1)

typedef struct {
    EFI_HOB_GUID_TYPE EfiHobGuidType;
    UINT16            ISCT_PM1_STS;
} AMI_ISCT_HOB;

#pragma pack ()

EFI_GUID  gEfiPeiEndOfPeiPhasePpiGuid = EFI_PEI_END_OF_PEI_PHASE_PPI_GUID;

#define KSC_GETWAKE_STATUS    0x35
#define KSC_CLEARWAKE_STATUS  0x36

EFI_STATUS
IsctGetWakeReason (
    IN EFI_PEI_SERVICES           **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDesc,
    IN VOID                       *Ppi
);

STATIC EFI_PEI_NOTIFY_DESCRIPTOR mIsctGetWakeReasonNotifyDesc = {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiPeiEndOfPeiPhasePpiGuid,
    IsctGetWakeReason
};

EFI_STATUS
IsctGetWakeReason (
    IN EFI_PEI_SERVICES           **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDesc,
    IN VOID                       *Ppi
)
/*++

Routine Description:

  Get system Wake Reason and save into CMOS 72/73 for ACPI ASL to use.

Arguments:

  PeiServices       General purpose services available to every PEIM.

Returns:

--*/
{
    EFI_STATUS                  Status;
    UINT32                      PmBase;
    UINT16                      PM1STS;
    UINT16                      USB29VID;
    UINT16                      USB29STS;
    UINT16                      xHCIVID;
    UINT16                      xHCISTS;
    UINT8                       WakeReason;
    UINT8                       KscStatus;
    UINT8                       WakeStatus;
    UINTN                       Count;
    UINTN                       Size;
    UINT8                       PcieWake;
    ISCT_PERSISTENT_DATA        IsctData;
    EFI_PEI_READ_ONLY_VARIABLE2_PPI  *ReadOnlyVariable;
    UINT8                       *IsctNvsPtr;
    AMI_ISCT_HOB                *AmiIsctHobPtr;
    EFI_GUID                    AmiIsctHobGuid = AMI_ISCT_HOB_GUID;
    //
    // Locate PEI Read Only Variable PPI.
    //
    Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &ReadOnlyVariable);
    if ( EFI_ERROR(Status) ) {
        return Status;
    }

    Size = sizeof (ISCT_PERSISTENT_DATA);
    Status = ReadOnlyVariable->GetVariable (
                 ReadOnlyVariable,
                 ISCT_PERSISTENT_DATA_NAME,
                 &gIsctPersistentDataGuid,
                 NULL,
                 &Size,
                 &IsctData
             );

    if ( EFI_ERROR(Status) ) {
        DEBUG ((EFI_D_INFO, "IsctPei: GetVariable for IsctData Status = %r \n", Status));
        return Status;
    }

    DEBUG ((EFI_D_INFO, "IsctPei: IsctNvsPtr = %x \n", IsctData.IsctNvsPtr));

    //
    // Clear Isct Wake Reason
    //
    DEBUG ((EFI_D_INFO, "IsctPei: Previous Isct Wake Reason = %x \n", *(UINT8 *) IsctData.IsctNvsPtr));
    IsctNvsPtr  = (UINT8 *) IsctData.IsctNvsPtr;
    *IsctNvsPtr = 0;
    WakeReason = 0;
    PcieWake = 0;

    DEBUG ((EFI_D_INFO, "IsctPei: Address for Isct Nvs Region = %x \n", IsctNvsPtr));
    DEBUG ((EFI_D_INFO, "IsctPei: Timer Value saved for RTC timer = %x \n", *(UINT32 *)(IsctData.IsctNvsPtr + 0x03)));

    //
    // Initialize base address for Power Management
    //
    PmBase = PchLpcPciCfg16 (R_PCH_LPC_ACPI_BASE)  & B_PCH_LPC_ACPI_BASE_BAR;

    AmiIsctHobPtr = GetFirstGuidHob (&AmiIsctHobGuid);
    if (AmiIsctHobPtr == NULL) {
        DEBUG ((EFI_D_ERROR, "IsctPei: AmiIsctHobPtr not available\n"));
        return  EFI_NOT_FOUND;
    }

//  PM1STS  = IoRead16(PmBase + R_PCH_ACPI_PM1_STS);
    PM1STS = AmiIsctHobPtr->ISCT_PM1_STS;
    PM1STS &= (B_PCH_ACPI_PM1_STS_PWRBTN | B_PCH_ACPI_PM1_STS_RTC | BIT14);

    //
    // Check PM1_STS
    //
    DEBUG ((EFI_D_INFO, "IsctPei: PM1_STS Value= %x \n", PM1STS));
    DEBUG ((EFI_D_INFO, "  Bit set in PM1_STS: \n"));
    switch (PM1STS) {
    case B_PCH_ACPI_PM1_STS_PWRBTN:
        WakeReason |= ISCT_WAKE_REASON_USER_EVENT; //User event
        DEBUG ((EFI_D_INFO, "    PowerButton\n"));
        break;
    case B_PCH_ACPI_PM1_STS_RTC:
        WakeReason |= ISCT_WAKE_REASON_RTC_TIMER; //RTC Timer
        DEBUG ((EFI_D_INFO, "    RTC Timer\n"));
        break;
    case BIT14: //PCIe Wake
        PcieWake = 1;
        WakeReason |= ISCT_WAKE_REASON_PME_WAKE; //PME
        DEBUG ((EFI_D_INFO, "    PCIe PME\n"));
        break;
    default:
        WakeReason = ISCT_WAKE_REASON_UNKNOWN;
        DEBUG ((EFI_D_INFO, "    Unknown\n"));
        break;
    }
    DEBUG ((EFI_D_INFO, "IsctPei: PCIe Wake: %x\n", PcieWake));

    //
    // EHCI PME : Offset 0x54(15)
    //
    USB29VID = MmioRead16 (
                   MmPciAddress (
                       0,
                       DEFAULT_PCI_BUS_NUMBER_PCH,
                       PCI_DEVICE_NUMBER_PCH_USB,
                       PCI_FUNCTION_NUMBER_PCH_EHCI,
                       R_PCH_USB_VENDOR_ID
                   ));

    USB29STS = MmioRead16 (
                   MmPciAddress (
                       0,
                       DEFAULT_PCI_BUS_NUMBER_PCH,
                       PCI_DEVICE_NUMBER_PCH_USB,
                       PCI_FUNCTION_NUMBER_PCH_EHCI,
                       R_PCH_EHCI_PWR_CNTL_STS
                   )) & (B_PCH_EHCI_PWR_CNTL_STS_PME_STS | B_PCH_EHCI_PWR_CNTL_STS_PME_EN);

    if (USB29VID != 0xFFFF && USB29VID != 0) {
        if (USB29STS == 0x8100) {
            DEBUG ((EFI_D_INFO, "IsctPei: EHCI Wake\n"));
            WakeReason |= ISCT_WAKE_REASON_USER_EVENT; //User event
        }
    }

    //
    // xHCI PME : Offset 0x74(15)
    //
    xHCIVID = MmioRead16 (
                  MmPciAddress (
                      0,
                      DEFAULT_PCI_BUS_NUMBER_PCH,
                      PCI_DEVICE_NUMBER_PCH_XHCI,
                      PCI_FUNCTION_NUMBER_PCH_XHCI,
                      R_PCH_USB_VENDOR_ID
                  ));

    xHCISTS = MmioRead16 (
                  MmPciAddress (
                      0,
                      DEFAULT_PCI_BUS_NUMBER_PCH,
                      PCI_DEVICE_NUMBER_PCH_XHCI,
                      PCI_FUNCTION_NUMBER_PCH_XHCI,
                      R_PCH_XHCI_PWR_CNTL_STS
                  )) & (B_PCH_XHCI_PWR_CNTL_STS_PME_STS | B_PCH_XHCI_PWR_CNTL_STS_PME_EN);


    if (xHCIVID != 0xFFFF && xHCIVID != 0) {
        if (xHCISTS == 0x8100) {
            DEBUG ((EFI_D_INFO, "IsctPei: xHCI Wake\n"));
            WakeReason |= ISCT_WAKE_REASON_PME_WAKE; //PME
        }
    }

    //
    // Check if IsctRTCTimerSupport is disabled.
    //
    if (*(UINT8 *)((UINTN)(IsctData.IsctNvsPtr + 0x02)) == 0) {
        DEBUG ((EFI_D_INFO, "IsctPei: EC timer is being used\n"));
        //
        // Check KSC Input Buffer
        //
        Count     = 0;
        KscStatus = IoRead8 (KSC_C_PORT);

        while (((KscStatus & KSC_S_IBF) != 0) && (Count < KSC_TIME_OUT)) {
            KscStatus = IoRead8 (KSC_C_PORT);
            Count++;
        }

        //
        // Send EC GetWakeStatus command
        //
        IoWrite8(KSC_C_PORT, KSC_GETWAKE_STATUS);

        //
        // Check KSC Output Buffer
        //
        Count     = 0;
        KscStatus = IoRead8 (KSC_C_PORT);

        while (((KscStatus & KSC_S_OBF) == 0) && (Count < KSC_TIME_OUT)) {
            KscStatus = IoRead8 (KSC_C_PORT);
            Count++;
        }

        //
        // Receive wake reason
        //
        WakeStatus = IoRead8 (KSC_D_PORT);

        //
        // Check KSC Input Buffer
        //
        Count     = 0;
        KscStatus = IoRead8 (KSC_C_PORT);

        while (((KscStatus & KSC_S_IBF) != 0) && (Count < KSC_TIME_OUT)) {
            KscStatus = IoRead8 (KSC_C_PORT);
            Count++;
        }

        //
        // Send EC ClearWakeStatus command
        //
        IoWrite8(KSC_C_PORT, KSC_CLEARWAKE_STATUS);
        DEBUG ((EFI_D_INFO, "IsctPei: EC Wake Status: %r\n", WakeStatus));
        switch (WakeStatus) {
        case BIT1:  // Lid Wake
            WakeReason |= ISCT_WAKE_REASON_USER_EVENT; //Bit0 is user event wake
            break;
        case BIT2:  // Keyboard/Mouse Wake
            WakeReason |= ISCT_WAKE_REASON_USER_EVENT; //Bit0 is user event wake
            break;
        case BIT3: // EC Timer Wake
            WakeReason |= ISCT_WAKE_REASON_PERIODIC_WAKE; //Bit1 is EC timer wake
            break;
        case BIT4: // PCIe Wake
            WakeReason |= ISCT_WAKE_REASON_PME_WAKE; //Wake due to PME
            break;
        default: // Unknown
            WakeReason |= 0x00;
            break;
        }
        //
        // Override because of EC timer wake from FFS_S3 or S4 (Need EC support it!)
        //
        if ((PM1STS == B_PCH_ACPI_PM1_STS_PWRBTN) && (WakeStatus == BIT3)) {
            WakeReason |= ISCT_WAKE_REASON_PERIODIC_WAKE;
        }
    }
    //
    //If RTC wake, check if IsctOverWrite is set to OS
    //
    if ((WakeReason & ISCT_WAKE_REASON_RTC_TIMER) == ISCT_WAKE_REASON_RTC_TIMER) {
        if (*(UINT8 *)((UINTN)(IsctData.IsctNvsPtr + 0x0B)) == 1) {
            WakeReason |= ISCT_WAKE_REASON_PERIODIC_WAKE;
            WakeReason &= ~ISCT_WAKE_REASON_RTC_TIMER;
        }
    }

    //
    // Check for Network Device PME from PCIe if PME wake reason
    //
    /*
      if(PcieWake == 1) //PME
      {
        DEBUG ((EFI_D_INFO, "IsctPei: PME wake reason- check if from network device\n"));
        if(IsNetworkDevicePME())
        {
          WakeReason |= BIT4;
          DEBUG ((EFI_D_INFO, "IsctPei: IsNetworkDevicePME() returned Yes\n"));
        }
      }
    */

    //
    // Set Isct Wake Reason
    //
    DEBUG ((EFI_D_INFO, "IsctPei: Wake Reason reported to Agent= %x \n", WakeReason));
    *(UINT8 *)IsctData.IsctNvsPtr = WakeReason;

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
IsctPeiEntryPoint (
    IN       EFI_PEI_FILE_HANDLE   FileHandle,
    IN CONST EFI_PEI_SERVICES     **PeiServices
)
/*++

Routine Description:

  Set up

Arguments:

  PeiServices       General purpose services available to every PEIM.

Returns:

--*/
{
    EFI_STATUS                      Status;
    UINT8                           IsctEnabled;
    EFI_GUID SetupGuid = SETUP_GUID;
    EFI_PEI_READ_ONLY_VARIABLE2_PPI      *ReadOnlyVariable;
    SETUP_DATA                      SetupData;
    UINTN                           VariableSize;
    EFI_BOOT_MODE                   BootMode;
    AMI_ISCT_HOB                    *AmiIsctHobPtr;
    EFI_GUID                        AmiIsctHobGuid = AMI_ISCT_HOB_GUID;
    UINT32                          PmBase;

    DEBUG ((EFI_D_INFO, "IsctPei Entry Point\n"));
    IsctEnabled = 0;

    //
    // Locate PEI Read Only Variable PPI.
    //
    Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &ReadOnlyVariable);
    if ( EFI_ERROR(Status) ) {
        return Status;
    }

    //
    // Get Setup Variable
    //
    VariableSize = sizeof (SETUP_DATA);
    Status = ReadOnlyVariable->GetVariable (
                 ReadOnlyVariable,
                 L"Setup",
                 &SetupGuid,
                 NULL,
                 &VariableSize,
                 &SetupData
             );
    if ( EFI_ERROR(Status) ) {
        return Status;
    }

    IsctEnabled = SetupData.IsctConfiguration;

    if (IsctEnabled == 0) {
        DEBUG ((EFI_D_INFO, "Isct Disabled\n"));
        return EFI_SUCCESS;
    }
    else {
        Status = PeiServicesGetBootMode (&BootMode);
        if ( EFI_ERROR( Status ) ) {
            DEBUG ((EFI_D_ERROR, "IsctPei: Get Boot Mode is fail\n"));
            return   Status;
        }

        if ( BootMode == BOOT_ON_S3_RESUME ) {
            DEBUG ((EFI_D_ERROR, "IsctPei: In the BOOT_ON_S3_RESUME\n"));

            Status = (*PeiServices)->CreateHob (PeiServices, EFI_HOB_TYPE_GUID_EXTENSION, sizeof (AMI_ISCT_HOB), (VOID **) &AmiIsctHobPtr);
            if ( EFI_ERROR( Status ) ) {
                DEBUG ((EFI_D_ERROR, "IsctPei: CreateHob is fail for AmiIsctHobPtr\n"));
                return   Status;
            }

            //
            // Initialize base address for Power Management
            //
            PmBase = PchLpcPciCfg16 (R_PCH_LPC_ACPI_BASE)  & B_PCH_LPC_ACPI_BASE_BAR;

            AmiIsctHobPtr->EfiHobGuidType.Name = AmiIsctHobGuid;
            AmiIsctHobPtr->ISCT_PM1_STS = IoRead16(PmBase + R_PCH_ACPI_PM1_STS);
            DEBUG ((EFI_D_ERROR, "IsctPei: AmiIsctHobPtr->ISCT_PM1_STS = %x \n", AmiIsctHobPtr->ISCT_PM1_STS));

            Status = PeiServicesNotifyPpi (&mIsctGetWakeReasonNotifyDesc);
            if ( EFI_ERROR(Status) ) {
                DEBUG ((EFI_D_INFO, "IsctPei: Notify  EFI_PEI_END_OF_PEI_PHASE_PPI_GUID Status = %r \n", Status));
                return Status;
            }

            return   Status;
        }
    }

    return EFI_SUCCESS;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
