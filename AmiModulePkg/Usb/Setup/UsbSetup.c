//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2008, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************

//****************************************************************************
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/Setup/usbsetup.c 17    7/15/11 6:32a Ryanchou $
//
// $Revision: 17 $
//
// $Date: 7/15/11 6:32a $
//
//****************************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
//  Name:           UsbSetup.c
//
//  Description:    USB driver setup related functions implementation.
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <AmiLib.h>
#include <AmiDxeLib.h>
#include <Setup.h>
#include <Protocol/AmiUsbController.h>
#include <Protocol/UsbPolicy.h>

#define MAX_DEVS_LINE_LENGTH 80
#define MAX_DEVICE_NUMBER_LENGTH 10
#define MAX_DEVICE_AMOUNT 127

static EFI_GUID gEfiSetupGuid = SETUP_GUID;

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetConnectedDevices
//
// Description: This function retrieves the information about connected
//              USB devices.
//
// Output:      returns TRUE if device connection status has changed since this
//              function is called last time; otherwise FALSE.
// Notes:       When FALSE is returned, none of the output parameters are valid.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetConnectedDevices(CHAR16* sName, EFI_USB_PROTOCOL *pUsb)
{
    CHAR16 	StrMassStorage[] = L"Drive";
    CHAR16 	StrKeyboard[] = L"Keyboard";
    CHAR16 	StrMouse[] = L"Mouse";
    CHAR16 	StrPoint[] = L"Point"; 		//(EIP38434+)
    CHAR16 	StrMice[] = L"Mice";
    CHAR16 	StrHub[] = L"Hub";
	CHAR16 	StrCcid[] = L"SmartCard Reader";
    CHAR16 	Name[MAX_DEVS_LINE_LENGTH];
    CHAR16 	*StrPtr = Name;
    CHAR16 	NumberToString [MAX_DEVICE_NUMBER_LENGTH];
    UINTN 	NumSize;
    CHAR16 	Comma[] = L", ";
    CHAR16 	Space[] = L" ";
    CHAR16 	LeadingSpace[] = L"      None";
	UINT8	MassStorageNumber;
	UINT8 	KeyboardNumber; 
	UINT8	MouseNumber;
	UINT8	PointNumber; 
	UINT8	HubNumber; 
	UINT8	CcidNumber;	//(EIP38434)
    CONNECTED_USB_DEVICES_NUM   Devs;
    BOOLEAN	Is1stItem = TRUE;

    pUsb->UsbReportDevices(&Devs);
    MassStorageNumber 	= Devs.NumUsbMass;
    KeyboardNumber		= Devs.NumUsbKbds;
    MouseNumber			= Devs.NumUsbMice;
    PointNumber			= Devs.NumUsbPoint;			//(EIP38434+)
    HubNumber			= Devs.NumUsbHubs;
	CcidNumber 			= Devs.NumUsbCCIDs;

    // Form the string
    pBS->SetMem (StrPtr, MAX_DEVS_LINE_LENGTH * sizeof(CHAR16), 0);
    pBS->CopyMem (StrPtr, LeadingSpace, 10 * sizeof(CHAR16)); StrPtr += 6; 
										// leave string pointer at "None"

    // Drive/Drives
    if ((MassStorageNumber) && (MassStorageNumber < MAX_DEVICE_AMOUNT)) 
	{
        ItowEx (MassStorageNumber, NumberToString, 10, FALSE);
        NumSize = Wcslen (NumberToString);

        pBS->CopyMem (StrPtr, NumberToString, (NumSize * sizeof (CHAR16)));
        StrPtr += NumSize;
        // move pointer 1 more space
        pBS->CopyMem (StrPtr, Space, 2); StrPtr += 1;

        pBS->CopyMem (StrPtr, StrMassStorage, 10); StrPtr += 5;		// L"Drive"
        if (MassStorageNumber > 1) {
          *StrPtr++ = L's';                      // L"Drives"
        }
        Is1stItem = FALSE;
    }

    // Keyboard/Keyboards
    if ((KeyboardNumber) && (KeyboardNumber < MAX_DEVICE_AMOUNT)) 
	{
        if (!Is1stItem) {
            pBS->CopyMem (StrPtr, Comma, 4); StrPtr += 2;    // L" ,"
        }
        ItowEx (KeyboardNumber, NumberToString, 10, FALSE);
        NumSize = Wcslen (NumberToString);

        // move pointer 1 more space then string length
        pBS->CopyMem (StrPtr, NumberToString, (NumSize * sizeof (CHAR16)));
        StrPtr += NumSize;
        // move pointer 1 more space
        pBS->CopyMem (StrPtr, Space, 2); StrPtr += 1;

        pBS->CopyMem (StrPtr, StrKeyboard, 16); StrPtr += 8;	// L"Keyboard"
        if (KeyboardNumber > 1) {
          *StrPtr++ = L's';                      // L"Keyboards"
        }
        Is1stItem = FALSE;
    }

    // Mouse/Mice
    if ((MouseNumber) && (MouseNumber < MAX_DEVICE_AMOUNT)) 
	{
        if (!Is1stItem) {
            pBS->CopyMem (StrPtr, Comma, 4); StrPtr += 2;    // L" ,"
        }
        ItowEx (MouseNumber, NumberToString, 10, FALSE);
        NumSize = Wcslen (NumberToString);

        // move pointer 1 more space then string length
        pBS->CopyMem (StrPtr, NumberToString, (NumSize * sizeof (CHAR16)));
        StrPtr += NumSize;
        // move pointer 1 more space
        pBS->CopyMem (StrPtr, Space, 2); StrPtr += 1;

        if (MouseNumber == 1) {
            pBS->CopyMem (StrPtr, StrMouse, 10); StrPtr += 5;    // L"Mouse"
        } else {
            pBS->CopyMem (StrPtr, StrMice, 8); StrPtr += 4;     // L"Mice"
        }
        Is1stItem = FALSE;
    }
										//(EIP38434+)>
    // Point/Points
    if ((PointNumber) && (PointNumber < MAX_DEVICE_AMOUNT)) 
	{
        if (!Is1stItem) {
            pBS->CopyMem (StrPtr, Comma, 4); StrPtr += 2;    // L" ,"
        }
        ItowEx (PointNumber, NumberToString, 10, FALSE);
        NumSize = Wcslen (NumberToString);

        // move pointer 1 more space then string length
        pBS->CopyMem (StrPtr, NumberToString, (NumSize * sizeof (CHAR16)));
        StrPtr += NumSize;
        // move pointer 1 more space
        pBS->CopyMem (StrPtr, Space, 2); StrPtr += 1;

        pBS->CopyMem (StrPtr,StrPoint, 10); StrPtr += 5;        // L"Point"
        if (PointNumber > 1) {
          *StrPtr++ = L's';                      	// L"Points"
        }
        Is1stItem = FALSE;
    } 
										//<(EIP38434+)
    // Hub/Hubs
    if ((HubNumber) && (HubNumber < MAX_DEVICE_AMOUNT)) 
	{
        if (!Is1stItem) {
            pBS->CopyMem (StrPtr, Comma, 4); StrPtr += 2;    // L" ,"
        }
        ItowEx (HubNumber, NumberToString, 10, FALSE);
        NumSize = Wcslen (NumberToString);

        // move pointer 1 more space then string length
        pBS->CopyMem (StrPtr, NumberToString, (NumSize * sizeof (CHAR16)));
        StrPtr += NumSize;
        // move pointer 1 more space
        pBS->CopyMem (StrPtr, Space, 2); StrPtr += 1;
        pBS->CopyMem (StrPtr, StrHub, 6); StrPtr += 3;	// L"Hub"
        if (HubNumber > 1) {
            *StrPtr++ = L's';                        // L"Hubs"
        }
        Is1stItem = FALSE;
    }
	// Ccid/Ccids
	if (CcidNumber) 
	{
		if (!Is1stItem) {
			pBS->CopyMem (StrPtr, Comma, 4); StrPtr += 2;	// L" ,"
		}
        ItowEx (CcidNumber, NumberToString, 10, FALSE);
        NumSize = Wcslen (NumberToString);

        // move pointer 1 more space then string length
        pBS->CopyMem (StrPtr, NumberToString, (NumSize * sizeof (CHAR16)));
        StrPtr += NumSize;

        // move pointer 1 more space
        pBS->CopyMem (StrPtr, Space, 2); StrPtr += 1;	//	L" "

		pBS->CopyMem (StrPtr, StrCcid, 32); StrPtr += 16; 	// L"SmartCard Reader"
		if (CcidNumber > 1) {
			*StrPtr++ = L's';						    // L'SmartCard Readers'
		}
		Is1stItem = FALSE;
	}

    pBS->CopyMem (sName, Name, MAX_DEVS_LINE_LENGTH * sizeof(CHAR16));

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetMassDeviceName
//
// Description: This function retrieves the USB mass storage device ASCII name.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
GetMassDeviceName(
    UINT8 *devName,
    UINT8 devAddr,
    EFI_USB_PROTOCOL *pUsb
)
{
    CHAR8   data[48];
    UINT8   nextDev;

    // Get the name using USBMassAPIGetDeviceInformation
    nextDev = pUsb->UsbGetNextMassDeviceName(data, sizeof(data), devAddr);
    if (nextDev != 0xFF) {
        Sprintf((char*)devName, "%a", data);
    }
    return nextDev;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   InitUSBStrings
//
// Description: This function is eLink'ed with the chain executed right before
//              the Setup.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID InitUSBStrings(EFI_HII_HANDLE HiiHandle, UINT16 Class)
{
    CHAR16  s16[MAX_DEVS_LINE_LENGTH];
    CHAR8   s8[MAX_DEVS_LINE_LENGTH];
    UINT8   NextDev;
    UINT16  sMassDev[16] = {
        STRING_TOKEN(STR_USB_MASS_DEVICE1),
        STRING_TOKEN(STR_USB_MASS_DEVICE2),
        STRING_TOKEN(STR_USB_MASS_DEVICE3),
        STRING_TOKEN(STR_USB_MASS_DEVICE4),
        STRING_TOKEN(STR_USB_MASS_DEVICE5),
        STRING_TOKEN(STR_USB_MASS_DEVICE6),
        STRING_TOKEN(STR_USB_MASS_DEVICE7),
        STRING_TOKEN(STR_USB_MASS_DEVICE8),
        STRING_TOKEN(STR_USB_MASS_DEVICE9),
        STRING_TOKEN(STR_USB_MASS_DEVICE10),
        STRING_TOKEN(STR_USB_MASS_DEVICE11),
        STRING_TOKEN(STR_USB_MASS_DEVICE12),
        STRING_TOKEN(STR_USB_MASS_DEVICE13),
        STRING_TOKEN(STR_USB_MASS_DEVICE14),
        STRING_TOKEN(STR_USB_MASS_DEVICE15),
        STRING_TOKEN(STR_USB_MASS_DEVICE16)

    };
    UINT8               i;
    UINTN               VariableSize;
    USB_MASS_DEV_NUM    SetupData;
    EFI_STATUS          Status;
    EFI_USB_PROTOCOL    *pUsb;
    EFI_GUID            guid = EFI_USB_PROTOCOL_GUID;


    if (Class!=ADVANCED_FORM_SET_CLASS) return;
                                        //(EIP102493+)>
    InitString(HiiHandle, STRING_TOKEN(STR_USB_MODULE_VER), L"%d.%02d.%02d", 
       USB_DRIVER_MAJOR_VER, USB_DRIVER_MINOR_VER, USB_DRIVER_BUILD_VER);
                                        //<(EIP102493+)

// TODO:: remove this later when INTERACTIVE options are properly implemented for
// both USB and BOOT page to display THE SAME boot devices.
    VariableSize = sizeof(SetupData);
    Status = pRS->GetVariable( L"UsbMassDevNum",
        &gEfiSetupGuid,
        NULL,
        &VariableSize,
        &SetupData );

    if(EFI_ERROR(Status)) {
        return;
    }

    SetupData.IsInteractionAllowed = 0;

    Status = pRS->SetVariable( L"UsbMassDevNum",
        &gEfiSetupGuid,
        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        VariableSize,
        &SetupData );

    Status = pBS->LocateProtocol(&guid, NULL, &pUsb);
//####  ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status)) return;

    // Number of connected devices
    GetConnectedDevices(s16, pUsb);
    InitString(HiiHandle, STRING_TOKEN(STR_USB_DEVICES_ENABLED_LIST), L"%s", s16);

    // Mass storage device names
    for (i = 0, NextDev = 0; i < 16; i++) {
        NextDev = GetMassDeviceName(s8, NextDev, pUsb);
        if (NextDev == 0xFF) return;    // No more devices
        InitString(HiiHandle, sMassDev[i], L"%S", s8);
        if (NextDev & 0x80) return;     // Last device
    }
    //ASSERT_EFI_ERROR(EFI_DEVICE_ERROR);   // Wrong place to be, no more than 8 devices to display

    //example: InitString(HiiHandle, STRING_TOKEN(STR_USB_MODULE_VERSION_VALUE), L"%d", 25);
}

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2008, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
