//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
// Revision History
// ----------------
// $Log:  $
//
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        SbSetup.c
//
// Description: South Bridge Setup related routines
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <AmiLib.h>
#include <AmiDxeLib.h>
#include <Sb.h>
#include <Setup.h>
#include <SetupStrDefs.h>
#include <Protocol/PciIo.h>
#include <Protocol/DevicePath.h>
#include <protocol/BlockIo.h>
#include <Protocol/PDiskInfo.h>
#include <Protocol/PIDEController.h>
#include <IndustryStandard/AmiAtaAtapi.h> //EIP180881
#include <Protocol/AmiIdeBus.h>           //EIP180881
#include <Protocol/PciRootBridgeIo.h>
// EIP 177820 (show eMMC storage infomation)   >>
#include <IndustryStandard/Mmc.h>
#include <IndustryStandard/CeAta.h>
#include <IndustryStandard/SdCard.h>
#include <Protocol/SdHostIo.h>
#include <SbEmmcMid.h>

typedef struct _CARD_DATA CARD_DATA;
typedef enum {
  UnknownCard = 0,
  MMCCard,                // MMC card
  CEATACard,              // CE-ATA device
  SDMemoryCard,           // SD 1.1 card
  SDMemoryCard2,          // SD 2.0 or above standard card
  SDMemoryCard2High       // SD 2.0 or above high capacity card
} CARD_TYPE;

typedef struct {
  //
  //BlockIO
  //
  UINT32                    Signature;

  EFI_HANDLE                Handle;

  BOOLEAN                   Present;

  EFI_DEVICE_PATH_PROTOCOL  *DevPath;

  EFI_BLOCK_IO_PROTOCOL     BlockIo;

  EFI_BLOCK_IO_MEDIA        BlockIoMedia;

  CARD_DATA                 *CardData;

} MMC_PARTITION_DATA;

typedef struct {
  UINT8      ManufacturerId;
  CHAR16     *ManufacturerName;
} AMI_EMMC_MID_AND_THE_NAME;

struct _CARD_DATA {
  //
  //BlockIO
  //
  UINT32                    Signature;

  EFI_HANDLE                Handle;

  MMC_PARTITION_DATA        Partitions[8];

  EFI_SD_HOST_IO_PROTOCOL   *SdHostIo;
  EFI_UNICODE_STRING_TABLE  *ControllerNameTable;
  CARD_TYPE                 CardType;

  UINT8                     CurrentBusWidth;
  BOOLEAN                   DualVoltage;
  BOOLEAN                   NeedFlush;
  UINT8                     Reserved[3];

  UINT16                    Address;
  UINT32                    BlockLen;
  UINT32                    MaxFrequency;
  UINT64                    BlockNumber;
  //
  //Common used
  //
  CARD_STATUS               CardStatus;
  OCR                       OCRRegister;
  CID                       CIDRegister;
  CSD                       CSDRegister;
  EXT_CSD                   ExtCSDRegister;
  UINT8                     *RawBufferPointer;
  UINT8                     *AlignedBuffer;
  //
  //CE-ATA specific
  //
  TASK_FILE                 TaskFile;
  IDENTIFY_DEVICE_DATA      IndentifyDeviceData;
  //
  //SD specific
  //
  SCR                       SCRRegister;
  SD_STATUS_REG             SDSattus;
  SWITCH_STATUS             SwitchStatus;
#if defined (BYTI_PF_ENABLE) && (BYTI_PF_ENABLE == 1)
  EFI_PCI_IO_PROTOCOL       *PciIo;
#endif
};
#include <Protocol/EmmcCardInfoProtocol.h>
// EIP 177820 (show eMMC storage infomation)   <<

//
// Print primitives
//
#define LEFT_JUSTIFY  0x01
#define PREFIX_SIGN   0x02
#define PREFIX_BLANK  0x04
#define COMMA_TYPE    0x08
#define LONG_TYPE     0x10
#define PREFIX_ZERO   0x20

//
// Length of temp string buffer to store value string.
//
#define CHARACTER_NUMBER_FOR_VALUE              30
#define _48_BIT_ADDRESS_FEATURE_SET_SUPPORTED   0x0400
#define ATAPI_DEVICE                            0x8000
typedef enum {
    EfiNonCombined,
    EfiCombinedPrimary,
    EfiCombinedSecondary
} EFI_IDE_MODE;


STRING_REF gPATA[2] = {
    STRING_TOKEN(STR_PATA_MASTER_NAME), STRING_TOKEN(STR_PATA_SLAVE_NAME)
};

STRING_REF gSATANonComb[2][2] = {
    {STRING_TOKEN(STR_SATA_0_NAME), STRING_TOKEN(STR_SATA_1_NAME)},
    {STRING_TOKEN(STR_SATA_2_NAME), STRING_TOKEN(STR_SATA_3_NAME)}
};

/*
STRING_REF gSATACombPrimary[2][2] = {
  {STRING_TOKEN(STRING_TOKEN(STR_PATA_MASTER_NAME)), STRING_TOKEN(STR_SATA_1_NAME)},
  {STRING_TOKEN(STR_PATA_SLAVE_NAME), STRING_TOKEN(STR_SATA_3_NAME)}
};

STRING_REF gSATADefault[2][2] = {
  {STRING_TOKEN(STRING_TOKEN(STR_SATA_0_NAME)), STRING_TOKEN(STR_PATA_MASTER_NAME)},
  {STRING_TOKEN(STR_SATA_2_NAME), STRING_TOKEN(STR_PATA_SLAVE_NAME)}
};
*/

// Function Definitions

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   AsciiToUnicode
//
// Description: This function converts the provided ASCII string into
//              Unicode string
//
// Input:       IN CHAR8 *AsciiString - ASCII String to be converted
//              OUT CHAR16 *UnicodeString - Converted unicode String
//
// Output:      UnicodeString variable updated
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID AsciiToUnicode(
    IN  CHAR8     *AsciiString,
    OUT CHAR16    *UnicodeString
)
{
    UINT8 Index = 0;

    while(AsciiString[Index] != 0) {
        UnicodeString[Index] = (CHAR16) AsciiString[Index];
        Index++;
    }
}

// EIP 177820 (show eMMC storage infomation)   >>
EFI_STATUS GetEmmcInstance (EFI_EMMC_CARD_INFO_PROTOCOL **eMMCCardInfoProtocol)
{
  EFI_STATUS                     Status;
  UINTN                          NumberOfHandles;
  EFI_HANDLE                     *HandleBuffer = NULL;
  UINTN                          HandleIndex;
  EFI_EMMC_CARD_INFO_PROTOCOL    *eMMCCardInfo;
  EFI_GUID                       gEfiEmmcCardInfoProtocolGuid = EFI_EMMC_CARD_INFO_PROTOCOL_GUID;

  Status = pBS->LocateHandleBuffer (
                 ByProtocol,
                 &gEfiEmmcCardInfoProtocolGuid,
                 NULL,
                 &NumberOfHandles,
                 &HandleBuffer
                 );
  if (EFI_ERROR(Status)) {
    return EFI_NOT_FOUND;
  }

  for (HandleIndex = 0; HandleIndex < NumberOfHandles; HandleIndex++) {
    Status = pBS->HandleProtocol(
                   HandleBuffer[HandleIndex],
                   &gEfiEmmcCardInfoProtocolGuid,
                   eMMCCardInfoProtocol
                   );
    eMMCCardInfo = *eMMCCardInfoProtocol;
    if (eMMCCardInfo->CardData->CardType == MMCCard) {
      ///
      /// once eMMC found, system skips other instances
      ///
      return EFI_SUCCESS;
    } else {
      ///
      /// other type, it may be a SD card
      ///
      continue;
    }
  }

  return EFI_NOT_FOUND;
}


CHAR16* GeteMMCName()
{
    EFI_STATUS    Status;
    static CHAR16 StrBuf[100];
    EFI_EMMC_CARD_INFO_PROTOCOL *eMMCCardInfoProtocol = NULL;
    UINTN WordAmount;
    AMI_EMMC_MID_AND_THE_NAME EmmcBrand[] = {GlobalEmmcMid};
    UINTN                     TableIndex;

    Status = GetEmmcInstance (&eMMCCardInfoProtocol);
    if (EFI_ERROR(Status)) {
      return NULL;
    }

    TRACE((-1,"\n[eMMC] CIDRegister.MID %x\n",eMMCCardInfoProtocol->CardData->CIDRegister.MID));
    TRACE((-1,"[eMMC] CIDRegister.MDT %x\n",eMMCCardInfoProtocol->CardData->CIDRegister.MDT));
    TRACE((-1,"[eMMC] CIDRegister.PSN %x\n",eMMCCardInfoProtocol->CardData->CIDRegister.PSN));
    TRACE((-1,"[eMMC] CIDRegister.PRV %x\n",eMMCCardInfoProtocol->CardData->CIDRegister.PRV));
    TRACE((-1,"[eMMC] CIDRegister.OID %x\n",eMMCCardInfoProtocol->CardData->CIDRegister.OID));
    TRACE((-1,"[eMMC] CIDRegister.PNM %02x%02x%02x%02x%02x%02x\n",
                                            eMMCCardInfoProtocol->CardData->CIDRegister.PNM[5],
                                            eMMCCardInfoProtocol->CardData->CIDRegister.PNM[4],
                                            eMMCCardInfoProtocol->CardData->CIDRegister.PNM[3],
                                            eMMCCardInfoProtocol->CardData->CIDRegister.PNM[2],
                                            eMMCCardInfoProtocol->CardData->CIDRegister.PNM[1],
                                            eMMCCardInfoProtocol->CardData->CIDRegister.PNM[0]));

    ///
    /// Search the matching ManufacturerName from "AMI_EMMC_MID_AND_THE_NAME" table
    ///
    for (TableIndex = 0; TableIndex < sizeof (EmmcBrand) / sizeof (AMI_EMMC_MID_AND_THE_NAME); TableIndex++) {
      if (eMMCCardInfoProtocol->CardData->CIDRegister.MID == EmmcBrand[TableIndex].ManufacturerId) {
        StrCpy (StrBuf, EmmcBrand[TableIndex].ManufacturerName);
        WordAmount = StrLen (EmmcBrand[TableIndex].ManufacturerName);
        break;
      }

      ///
      /// No matching record
      ///
      if (TableIndex == (sizeof (EmmcBrand) / sizeof (AMI_EMMC_MID_AND_THE_NAME) - 1)) {
        StrCpy (StrBuf, L"Unknown");
        WordAmount = 7;
      }
    }

    StrBuf[WordAmount]     = ' ';
    StrBuf[WordAmount + 1] = eMMCCardInfoProtocol->CardData->CIDRegister.PNM[5];
    StrBuf[WordAmount + 2] = eMMCCardInfoProtocol->CardData->CIDRegister.PNM[4];
    StrBuf[WordAmount + 3] = eMMCCardInfoProtocol->CardData->CIDRegister.PNM[3];
    StrBuf[WordAmount + 4] = eMMCCardInfoProtocol->CardData->CIDRegister.PNM[2];
    StrBuf[WordAmount + 5] = eMMCCardInfoProtocol->CardData->CIDRegister.PNM[1];
    StrBuf[WordAmount + 6] = eMMCCardInfoProtocol->CardData->CIDRegister.PNM[0];

    Swprintf(&(StrBuf[WordAmount + 7]), L" %04X", eMMCCardInfoProtocol->CardData->CIDRegister.PSN);

    TRACE((-1,"[eMMC] StrBuf  (%S)\n",StrBuf));
    TRACE((-1,"[eMMC] StrBuf  (%s)\n",StrBuf));
    
    return StrBuf;
}
// EIP 177820 (show eMMC storage infomation)   <<


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SwapEntries
//
// Description: This function is swaps the (2) bytes in the array of words.
//              Basically changes the endian format.
//
// Input:       IN OUT CHAR8 *Data - Buffer containing the word data
//              IN UINT16 Size - Size of the buffer
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SwapEntries(
    IN OUT  CHAR8    *Data,
    IN      UINT16   Size
)
{
    UINT16  Index;
    CHAR8   Temp8;

    for(Index = 0; (Index+1) < Size; Index+=2) {
        Temp8           = Data[Index];
        Data[Index]     = Data[Index + 1];
        Data[Index + 1] = Temp8;
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetNumTenthsOfGB
//
// Description: This function is returns the tenths of the value provided.
//              This is used to display the driver size in GB particularly
//              in converting fractions into appropriate (or reasonable)
//              GB
//
// Input:       UINT32 RemainderBytesOfGB      Data to be converted
//
//
// Output:      The return value follows the following table:
//                  "x" is the input (RemainderBytesOfGB)
//                  "num tenths" is the output
//                  x   x/16    num tenths
//                  -   ----    ----------
//                  0   0           0
//                  1   .0625       1
//                  2   .125        1
//                  3   .1875       2
//                  4   .25         2
//                  5   .3125       3
//                  6   .375        4
//                  7   .4375       4
//                  8   .5          5
//                  9   .5625       6
//                  10  .625        6
//                  11  .6875       7
//                  12  .75         7
//                  13  .8125       8
//                  14  .875        9
//                  15  .9375       9
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32 GetNumTenthsOfGB(
    UINT32 RemainderBytesOfGB
)
{
    UINT32  Tenths = 0;                   //num tenths of of a GB
    UINT32  Sixteenths = RemainderBytesOfGB / (1 << 26); //num sixteenths of a GB

    switch(Sixteenths) {
    case 0:
        Tenths = 0;
        break;
    case 1:
    case 2:
        Tenths = 1;
        break;
    case 3:
    case 4:
        Tenths = 2;
        break;
    case 5:
        Tenths = 3;
        break;
    case 6:
    case 7:
        Tenths = 4;
        break;
    case 8:
        Tenths = 5;
        break;
    case 9:
    case 10:
        Tenths = 6;
        break;
    case 11:
    case 12:
        Tenths = 7;
        break;
    case 13:
        Tenths = 8;
        break;
    case 14:
    case 15:
        Tenths = 9;
        break;
    default:
        Tenths = 0;
        break;
    }

    return Tenths;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   InitSBStrings
//
// Description: This function initializes the SB related setup option values
//
// Input:       IN EFI_HII_HANDLE HiiHandle - Handle to HII database
//              IN UINT16 Class - Indicates the setup class
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID InitSbStrings(
    IN EFI_HII_HANDLE   HiiHandle,
    IN UINT16           Class
)
{
    EFI_STATUS                      Status;
    PCI_DEVICE_PATH                 *PciDevicePath;
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo;
    CHAR8                           *NewString;
    UINT8                           Index;
//  UINT8                           PciBuffer;
//  EFI_IDE_MODE                    mIdeMode = EfiNonCombined;
    UINTN                           HandleCount;
    EFI_HANDLE                      *HandleBuffer;
    EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
    EFI_DEVICE_PATH_PROTOCOL        *DevicePathNode;
    EFI_DISK_INFO_PROTOCOL          *DiskInfo;
    UINT32                          IdeChannel;
    UINT32                          IdeDevice;
    IDENTIFY_DATA                   *IdentifyDriveInfo = NULL;
    UINT32                          BufferSize = 0;
    STRING_REF                      Token;
    CHAR8                           ModelNumber[42];
    UINT64                          NumSectors = 0;
    UINT64                          DriveSizeInBytes = 0;
    UINTN                           RemainderInBytes = 0;
    UINT32                          DriveSizeInGB = 0;
    UINTN                           NumTenthsOfGB = 0;
//  UINT8                           *SbRcba = (UINT8*)SB_RCBA;
    UINT32                          SectorSize = 512; // Default Sector Size

    if(Class==ADVANCED_FORM_SET_CLASS) {
        // EIP 177820 (show eMMC storage infomation)   >>
        CHAR16 *EmmcInfo = NULL;        
        EmmcInfo = GeteMMCName();
        
        if(EmmcInfo != NULL){
            InitString(
                HiiHandle,
                STRING_TOKEN(STR_SCC_CONFIG_TITLE),
                L"eMMC Infomation                      %s",
                EmmcInfo);
        }
        // EIP 177820 (show eMMC storage infomation)   <<

        //
        // Assume no line strings is longer than 256 bytes.
        //
        Status = pBS->AllocatePool(EfiBootServicesData, 0x100, &NewString);
        ASSERT_EFI_ERROR(Status);

        PciDevicePath = NULL;

        //
        // Initialize IDE Combined mode
        //
        Status = pBS->LocateProtocol(&gEfiPciRootBridgeIoProtocolGuid, NULL, &PciRootBridgeIo);
        ASSERT_EFI_ERROR(Status);

        //
        // Fill IDE Infomation
        //
        Status = pBS->LocateHandleBuffer(
                     ByProtocol,
                     &gEfiDiskInfoProtocolGuid,
                     NULL,
                     &HandleCount,
                     &HandleBuffer);
        if(EFI_ERROR(Status)) HandleCount = 0;

        for(Index = 0; Index < HandleCount; Index++) {
            Status = pBS->HandleProtocol(
                         HandleBuffer[Index],
                         &gEfiDevicePathProtocolGuid,
                         (VOID *) &DevicePath);
            ASSERT_EFI_ERROR(Status);

            DevicePathNode = DevicePath;
            while(!isEndNode(DevicePathNode)) {
                if((DevicePathNode->Type == HARDWARE_DEVICE_PATH) &&
                        (DevicePathNode->SubType == HW_PCI_DP)) {
                    PciDevicePath = (PCI_DEVICE_PATH *) DevicePathNode;
                    break;
                }

                DevicePathNode = NEXT_NODE(DevicePathNode);
            }

            if(PciDevicePath == NULL) continue;

            //
            // Check for onboard IDE
            //
            if((PciDevicePath->Function == SATA_FUNC) &&  //EIP129785 
            	    (PciDevicePath->Device == SATA_DEV)) { //EIP203109
                Status = pBS->HandleProtocol(
                             HandleBuffer[Index],
                             &gEfiDiskInfoProtocolGuid,
                             &DiskInfo);
                ASSERT_EFI_ERROR(Status);

                Status = DiskInfo->WhichIde(
                             DiskInfo,
                             &IdeChannel,
                             &IdeDevice);
                ASSERT_EFI_ERROR(Status);

                Status = pBS->AllocatePool(EfiBootServicesData, sizeof(IDENTIFY_DATA), &IdentifyDriveInfo);
                ASSERT_EFI_ERROR(Status);
                pBS->SetMem(IdentifyDriveInfo, sizeof(IDENTIFY_DATA), 0);

                BufferSize = sizeof(IDENTIFY_DATA);
                Status = DiskInfo->Identify(
                             DiskInfo,
                             IdentifyDriveInfo,
                             &BufferSize);
                ASSERT_EFI_ERROR(Status);

                switch(PciDevicePath->Function) {
                //EIP129785 >>
                case SATA_FUNC: // SATA Device
               	//EIP129785 <<
                    if(IdeChannel > 2 || IdeDevice > 1)
                        continue;
                    Token = gSATANonComb[IdeDevice][IdeChannel];
                }

                pBS->SetMem(ModelNumber, 42, 0);
                pBS->CopyMem(ModelNumber, IdentifyDriveInfo->Model_Number_27, 40);
                SwapEntries(ModelNumber, 40);
                ModelNumber[DEVICE_NAME_LENGTH] = '\0';           // Truncate it at 14 characters //EIP149397

                //
                // For HardDisk append the size. Otherwise display atapi
                //
                if(!(IdentifyDriveInfo->General_Config_0 & ATAPI_DEVICE)) {
                    if(IdentifyDriveInfo->Command_Set_Supported_83 & _48_BIT_ADDRESS_FEATURE_SET_SUPPORTED) {
                        NumSectors = IdentifyDriveInfo->LBA_48;
                        if((IdentifyDriveInfo->Reserved_104_126[2] & 0x4000) && // WORD 106 valid? - BIT 14 - 1
                                (!(IdentifyDriveInfo->Reserved_104_126[2] & 0x8000)) && // WORD 106 valid? - BIT 15 - 0
                                (IdentifyDriveInfo->Reserved_104_126[2] & 0x1000)) { // WORD 106 bit 12 - Sectorsize > 256 words
                            // The sector size is in words 117-118.
                            SectorSize = (UINT32)(IdentifyDriveInfo->Reserved_104_126[13] + \
                                                  (IdentifyDriveInfo->Reserved_104_126[14] << 16)) * 2;
                        }
                    } else {
                        NumSectors = IdentifyDriveInfo->Addressable_Sector_60;
                    }

                    DriveSizeInBytes = Mul64(NumSectors, SectorSize);

                    //DriveSizeInGB is DriveSizeInBytes / 1 GB (1 Binary GB = 2^30 bytes)
//        DriveSizeInGB = (UINT32) Div64(DriveSizeInBytes, (1 << 30), &RemainderInBytes);
                    //Convert the Remainder, which is in bytes, to number of tenths of a Binary GB.
//        NumTenthsOfGB = GetNumTenthsOfGB(RemainderInBytes);

                    //DriveSizeInGB is DriveSizeInBytes / 1 GB (1 Decimal GB = 10^9 bytes)
                    DriveSizeInGB = (UINT32) Div64(DriveSizeInBytes, 1000000000, &RemainderInBytes);
                    //Convert the Remainder, which is in bytes, to number of tenths of a Decimal GB.
                    NumTenthsOfGB = RemainderInBytes / 100000000;

                    Sprintf(NewString, "%s (%d.%dGB)", ModelNumber, DriveSizeInGB, NumTenthsOfGB);
                } else {
                    Sprintf(NewString, "%s ATAPI", ModelNumber);
                }

                InitString(
                    HiiHandle,
                    Token,
                    L"%S",
                    NewString);
            }
            if(IdentifyDriveInfo) {
                pBS->FreePool(IdentifyDriveInfo);
                IdentifyDriveInfo = NULL;
            }
        }

        if(HandleBuffer) {
            pBS->FreePool(HandleBuffer);
            pBS->FreePool(NewString);
        }

    }
}

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
