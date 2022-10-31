//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2012, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Parkway, Norcross, GA 30093              **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************

//****************************************************************************
// $Header: /Alaska/SOURCE/Modules/SdioDriver/SdioSetup.c 7     3/07/12 4:25a Rajeshms $
//
// $Revision: 7 $
//
// $Date: 3/07/12 4:25a $
//
//****************************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
//  Name:           SdioSetup.c
//
//  Description:    Sdio driver setup related functions implementation.
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <AmiLib.h>
#include <AmiDxeLib.h>
#if EFI_SPECIFICATION_VERSION>0x20000
#include <Protocol/HiiString.h>
#include <Protocol/HiiDatabase.h>
#else
#include <Protocol/Hii.h>
#endif

#include <Setup.h>
#include <SdioElink.h>
#include <Protocol\PciIo.h>
#include <protocol\BlockIo.h>
#include <Protocol\PDiskInfo.h>
#include <Protocol\SdioBus.h>

typedef struct{
    UINT16  ManufactureId;
    UINT16  ManufactureCode;
    CHAR8   VendorString[30];
}SDIO_MANUFACTURE_DETAILS;


typedef struct {
    UINT8   SdioMode;
    UINT8   SdioEmu1;
    UINT8   SdioEmu2;
    UINT8   SdioEmu3;
    UINT8   SdioEmu4;
    UINT8   SdioEmu5;
    UINT8   SdioEmu6;
    UINT8   SdioEmu7;
    UINT8   SdioEmu8;
	UINT8	SdioMassDevNum;
} SDIO_DEV_CONFIGURATION;

//
// SD I/O device Vendor List.
//
SDIO_MANUFACTURE_DETAILS ManuFactureDetails[] = {
    SDIO_MANUFACTURE_DEVICE_LIST
    {0xFFFF, 0xFFFF, "Unknown Vendor"}
};

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   InitSdioStrings
//
// Description: This function is eLink'ed with the chain executed right before
//              the Setup.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID InitSdioStrings(EFI_HII_HANDLE HiiHandle, UINT16 Class)
{
    UINT16  SdioDev[8] = {
        STRING_TOKEN(STR_SDIO_DEVICE1),
        STRING_TOKEN(STR_SDIO_DEVICE2),
        STRING_TOKEN(STR_SDIO_DEVICE3),
        STRING_TOKEN(STR_SDIO_DEVICE4),
        STRING_TOKEN(STR_SDIO_DEVICE5),
        STRING_TOKEN(STR_SDIO_DEVICE6),
        STRING_TOKEN(STR_SDIO_DEVICE7),
        STRING_TOKEN(STR_SDIO_DEVICE8)
    };

    UINT16  SdioDevDetails[8] = {
        STRING_TOKEN(STR_SDIO_DEVICE1_DETAILS),
        STRING_TOKEN(STR_SDIO_DEVICE2_DETAILS),
        STRING_TOKEN(STR_SDIO_DEVICE3_DETAILS),
        STRING_TOKEN(STR_SDIO_DEVICE4_DETAILS),
        STRING_TOKEN(STR_SDIO_DEVICE5_DETAILS),
        STRING_TOKEN(STR_SDIO_DEVICE6_DETAILS),
        STRING_TOKEN(STR_SDIO_DEVICE7_DETAILS),
        STRING_TOKEN(STR_SDIO_DEVICE8_DETAILS)
    };

    UINT16  SdioDevLocation[8] = {
        STRING_TOKEN(STR_SDIO_DEVICE1_LOCATION),
        STRING_TOKEN(STR_SDIO_DEVICE2_LOCATION),
        STRING_TOKEN(STR_SDIO_DEVICE3_LOCATION),
        STRING_TOKEN(STR_SDIO_DEVICE4_LOCATION),
        STRING_TOKEN(STR_SDIO_DEVICE5_LOCATION),
        STRING_TOKEN(STR_SDIO_DEVICE6_LOCATION),
        STRING_TOKEN(STR_SDIO_DEVICE7_LOCATION),
        STRING_TOKEN(STR_SDIO_DEVICE8_LOCATION)
    };

    EFI_STATUS          Status;
    UINTN               HandleCount;
    EFI_HANDLE          *HandleBuffer;
    SDIO_BUS_PROTOCOL   *SdioBusInterface;
    DLINK                   *dlink = NULL;
    SDIO_DEVICE_INTERFACE   *SdioDevInterface = NULL;
    UINT8               SdioDevIndex=0;
    CHAR16              sName[55];
    UINT8               Index;
    UINT8               i,j;
    CHAR8               *NewString;
    CHAR16              *NewStringTemp;
    UINTN               seg;
    UINTN               bus;
    UINTN               dev;
    UINTN               func;
    UINT16              ManufactureId=0;
    EFI_GUID            gSetupGuid = SETUP_GUID;
    UINTN               VariableSize;
    SDIO_DEV_CONFIGURATION  SdioConfiguration;

    if (Class!=ADVANCED_FORM_SET_CLASS) return;

    //
    // Assume no line strings is longer than 256 bytes.
    //
    Status = pBS->AllocatePool(EfiBootServicesData, 0x100, &NewString);
    ASSERT_EFI_ERROR(Status);

    //
    // Assume no line strings is longer than 256 bytes.
    //
    Status = pBS->AllocatePool(EfiBootServicesData, 0x100, &NewStringTemp);
    ASSERT_EFI_ERROR(Status);

    Status = pBS->LocateHandleBuffer (
                                      ByProtocol,
                                      &gSdioBusInitProtocolGuid,
                                      NULL,
                                      &HandleCount,
                                      &HandleBuffer
                                      );
    if (EFI_ERROR(Status)) HandleCount = 0;

    for (Index = 0; Index < HandleCount; Index++) {
        Status = pBS->HandleProtocol (
                                    HandleBuffer[Index],
                                    &gSdioBusInitProtocolGuid,
                                    &SdioBusInterface
                                    );

        ASSERT_EFI_ERROR(Status);

        dlink = SdioBusInterface->SdioDeviceList.pHead;
        if (!dlink) {
            continue;
        }
        do {
            SdioDevInterface = OUTTER(dlink, SdioDeviceLink, SDIO_DEVICE_INTERFACE);
            dlink = dlink-> pNext;

            if(SdioDevInterface->MassStorageDevice == TRUE) {
                for(i=0;i<27;i++) {
                    sName[i]=(CHAR16)SdioDevInterface->PNM[i];
                }
            } else {

                //
                // SD slot has an IO device. Map the Manufacture ID into the table
                // and get the Vendor name and display it in Setup
                //

                ManufactureId=(UINT16)((UINT16)(SdioDevInterface->SdIOManufactureId[1] << 8)
                                        + ((UINT16)SdioDevInterface->SdIOManufactureId[0]));

                for(j=0;ManuFactureDetails[j].ManufactureId != 0xFFFF;j++) {
                    if(ManuFactureDetails[j].ManufactureId == ManufactureId) {
                        for(i=0;i<30;i++) {
                            sName[i]=(CHAR16)ManuFactureDetails[j].VendorString[i];
                        }
                        break;
                    }
                }
                //
                // Manufacture Id not Found. Initilize to Unknown Vendor.
                //
                if(ManuFactureDetails[j].ManufactureId == 0xFFFF) {
                    for(i=0;i<30;i++) {
                        sName[i]=(CHAR16)ManuFactureDetails[j].VendorString[i];
                    }
                }
            }

            InitString(HiiHandle, SdioDev[SdioDevIndex], L"%s", &sName[0]);


            //
            // Get the Device Pci Location
            //
            Status = SdioBusInterface->PciIO->GetLocation(SdioBusInterface->PciIO,&seg,&bus,&dev,&func);
            pBS->SetMem(NewString,0x100,0);
            Sprintf(NewString, "Bus %x Dev %x Func %x", bus,dev,func);
            for(i=0;i<50;i++) {
                NewStringTemp[i]=(CHAR16)NewString[i];
            }
            InitString(HiiHandle, SdioDevLocation[SdioDevIndex], L"%s", NewStringTemp);

            pBS->SetMem(NewString,0x100,0);
            Sprintf(NewString, "Sdio Device %d Details:", SdioDevIndex+1);
            for(i=0;i<50;i++) {
                NewStringTemp[i]=(CHAR16)NewString[i];
            }
            InitString(HiiHandle, SdioDevDetails[SdioDevIndex], L"%s", NewStringTemp);

            SdioDevIndex++;

        }while (dlink);

    }
    VariableSize = sizeof(SDIO_DEV_CONFIGURATION);
    Status = pRS->GetVariable( \
                    L"SdioDevConfiguration", \
                    &gSetupGuid, \
                    NULL, \
                    &VariableSize, \
                    &SdioConfiguration );

    if ( EFI_ERROR(Status) ) {
        //
        // If Error, Set default values and save "SdioConfiguration" variable.
        //
        pBS->SetMem(&SdioConfiguration, sizeof(SDIO_DEV_CONFIGURATION), 0);
    }
     //
     // Update "SdioMassDevNum" setup variable according to the number
     // of installed mass storage devices 
     //
    SdioConfiguration.SdioMassDevNum = SdioDevIndex;
    Status = pRS->SetVariable( \
                 L"SdioDevConfiguration", &gSetupGuid, \
                 EFI_VARIABLE_NON_VOLATILE | \
                 EFI_VARIABLE_BOOTSERVICE_ACCESS | \
                 EFI_VARIABLE_RUNTIME_ACCESS, \
                 VariableSize, &SdioConfiguration );
	ASSERT_EFI_ERROR(Status);

    if (HandleBuffer) {
        pBS->FreePool (HandleBuffer);
    }

    pBS->FreePool (NewString);

    return;
}

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2012, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Parkway, Norcross, GA 30093              **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
