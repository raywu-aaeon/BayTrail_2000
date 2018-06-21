//#**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//***********************************************************************

//**********************************************************************
// $Header:
//
// $Revision:
//
// $Date:
//**********************************************************************
// Revision History
// ----------------
//
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:  EfiOsBootOptionNames.c
//
// Description:	Implements hook to be called at ProcessEnterSetup.
//  Looks for "Windows Boot Manager" boot options and adds information
//  about which drive the associated Windows installation is on.
//
//<AMI_FHDR_END>
//**********************************************************************

//=======================================================================
// Module specific Includes
#include <AmiDxeLib.h>
#include <Protocol\BlockIo.h>
#include <Protocol\DevicePath.h>
#include <Protocol\SimpleFileSystem.h>
#include <Protocol\PDiskinfo.h>
#include "boot.h" // Part of TSE Binary

// External variables (part of AMITSEBin)
extern UINTN gBootOptionCount;
extern BOOT_DATA* gBootData;

// External functions

// This function locates the handle with BlockIo installed that provides block
// access to the partition containing the Windows boot loader.  The load option
// (BootXXXX variable) created by Windows starts with a HD node instead of a
// complete device path.  The partition signature is used to find the complete
// device path.
EFI_DEVICE_PATH_PROTOCOL* _DiscoverPartition(
    EFI_DEVICE_PATH_PROTOCOL *DevicePath
);

// Constant definitions
#define NEW_STRING_BUFFER_LENGTH 100
#define CONTROLLER_NAME_BUFFER_LENGTH 100

BOOLEAN BBSValidDevicePath( EFI_DEVICE_PATH_PROTOCOL *DevicePath );

//*************************************************************************
//<AMI_PHDR_START>
//
// Name: GetPhysicalBlockIoHandle
//
// Description:
//  Takes a handle with BlockIo providing partition access and returns
//  the handle with the BlockIo providing raw access to the drive that
//  contains the partition.
//
// Input:
//  IN EFI_HANDLE BlockIoHandle - Image handle with BlockIo partition access.
//
// Output:
//  None.
//
// Returns:
//  EFI_HANDLE - Handle with BlockIo that provides raw access to drive
//  containing partition.
//
// Modified:
//  None.
//
// Referrals:
//  HandleProtocol()
//  DPCut()
//  LocateDevicePath()
//  FreePool()
//
// Notes:
//  None.
//
//<AMI_PHDR_END>
//*************************************************************************
EFI_HANDLE GetPhysicalBlockIoHandle (
    IN EFI_HANDLE BlockIoHandle
)
{
    EFI_BLOCK_IO_PROTOCOL *BlkIo;
    EFI_DEVICE_PATH_PROTOCOL *DevicePath, *Dp;
    EFI_STATUS Status;
    EFI_HANDLE Handle = BlockIoHandle;

    Status=pBS->HandleProtocol(
               Handle, &gEfiBlockIoProtocolGuid, &BlkIo
           );
    if (EFI_ERROR(Status)) return Handle;
    if (!BlkIo->Media->LogicalPartition) return Handle;
    Status=pBS->HandleProtocol(
               Handle, &gEfiDevicePathProtocolGuid, &DevicePath
           );
    if (EFI_ERROR(Status)) return Handle;
    Dp=DevicePath;
    while (BlkIo->Media->LogicalPartition) {
        EFI_DEVICE_PATH_PROTOCOL *PrevDp=Dp;
        //We need to cut Devicepath node to get Phisycal Partition
        Dp=DPCut(PrevDp);
        if (PrevDp != DevicePath) pBS->FreePool(PrevDp);
        if (Dp == NULL) break;
        PrevDp=Dp;
        Status=pBS->LocateDevicePath(
                   &gEfiBlockIoProtocolGuid,&PrevDp,&Handle
               );
        if (EFI_ERROR(Status)) break;
        Status=pBS->HandleProtocol(
                   Handle, &gEfiBlockIoProtocolGuid, &BlkIo
               );
        if (EFI_ERROR(Status)) break;
    }
    if (Dp != NULL && Dp != DevicePath) pBS->FreePool(Dp);
    //if physical Block I/O handle is not found, return original handle
    return (BlkIo->Media->LogicalPartition) ? BlockIoHandle : Handle;
}

//*************************************************************************
//<AMI_PHDR_START>
//
// Name: RemoveTrailingSpaces
//
// Description:
//  Removes trailing spaces from *Name.
//
// Input:
//  IN CHAR16 *Name - String from which trailing spaces should be removed.
//  IN UINTN NumberOfCharacters - Length of string at *Name.
//
// Output:
//  None.
//
// Returns:
//  UINTN - Length of string after spaces have been removed.
//
// Modified:
//  String pointed to by *Name.
//
// Referrals:
//  None.
//
// Notes:
//  None.
//
//<AMI_PHDR_END>
//*************************************************************************
UINTN RemoveTrailingSpaces (
    IN CHAR16 *Name,
    IN UINTN NumberOfCharacters
)
{
    //remove trailing spaces
    while (NumberOfCharacters>0 && Name[NumberOfCharacters-1] == L' ')
        NumberOfCharacters--;
    Name[NumberOfCharacters]=0;
    return NumberOfCharacters;
}

//*************************************************************************
//<AMI_PHDR_START>
//
// Name: ConstructBootOptionNameByHandle
//
// Description:
//  Determines controller name for parent of Handle.
//
// Input:
//  IN EFI_HANDLE Handle - Handle with partition BlockIo.
//  IN CHAR16 *Name - Caller allocated string to store name in.
//  IN UINTN NameSize - Size of string buffer.
//
// Output:
//  None.
//
// Returns:
//  UINTN - Length of controller name string.
//
// Modified:
//  String pointed to by *Name.
//
// Referrals:
//  GetPhysicalBlockIoHandle()
//  Swprintf_s()
//  RemoveTrailingSpaces()
//
// Notes:
//  None.
//
//<AMI_PHDR_END>
//*************************************************************************
UINTN ConstructBootOptionNameByHandle (
    IN EFI_HANDLE Handle,
    IN CHAR16 *Name,
    IN UINTN NameSize
)
{
    CHAR16 *ControllerName;
    UINTN  NumberOfCharacters;

    if (Handle == NULL) return 0;

    //Name from Controller Handle
    Handle = GetPhysicalBlockIoHandle(Handle);
    if (!GetControllerName(Handle, &ControllerName)) return 0;
    NumberOfCharacters = Swprintf_s(Name, NameSize, L"%s", ControllerName);
    return RemoveTrailingSpaces(Name, NumberOfCharacters);
}

//TODO:: OEM FUNCTION, PLEASE reference SbSetup.c and modify this function to get currect port number	>>>>
UINT16 gSATA[3][2] = {
    { 0, 1 },
    { 2, 3 },
    { 4, 5 }
};

//*************************************************************************
//<AMI_PHDR_START>
//
// Name: GetHDDPort
//
// Description:
//  Use handle and EFI_DISK_INFO_PROTOCOL to find sata hard disk port.
//
// Input:
//  IN EFI_HANDLE
//
// Output:
//  None.
//
// Returns:
//  UINT16 - Sata port number.
//
// Modified:
//  None.
//
// Referrals:
//  None.
//
// Notes:
//  None.
//
//<AMI_PHDR_END>
//*************************************************************************
UINT16 GetHDDPort( IN EFI_HANDLE Handle )
{
    EFI_STATUS Status;
    EFI_GUID gEfiDiskInfoProtocolGuid = EFI_DISK_INFO_PROTOCOL_GUID;
    EFI_DISK_INFO_PROTOCOL *DiskInfo;
    EFI_DEVICE_PATH_PROTOCOL *DevicePath;
    UINT32 IdeChannel;
    UINT32 IdeDevice;

    Status = pBS->HandleProtocol (
                 Handle,
                 &gEfiDevicePathProtocolGuid,
                 (VOID *) &DevicePath );

    if ( !EFI_ERROR( Status ))
    {
        EFI_DEVICE_PATH_PROTOCOL *DevicePathNode;
        EFI_DEVICE_PATH_PROTOCOL *MessagingDevicePath;
        PCI_DEVICE_PATH *PciDevicePath;

        DevicePathNode = DevicePath;
        while (!isEndNode (DevicePathNode))
        {
            if ((DevicePathNode->Type == HARDWARE_DEVICE_PATH)
                    && (DevicePathNode->SubType == HW_PCI_DP))
                PciDevicePath = (PCI_DEVICE_PATH *) DevicePathNode;
            else if (DevicePathNode->Type == MESSAGING_DEVICE_PATH)
                MessagingDevicePath = DevicePathNode;

            DevicePathNode = NEXT_NODE (DevicePathNode);
        }

        Status = pBS->HandleProtocol ( Handle, &gEfiDiskInfoProtocolGuid, &DiskInfo );
        if ( !EFI_ERROR(Status) )
        {
            Status = DiskInfo->WhichIde ( DiskInfo, &IdeChannel, &IdeDevice );
            if ( !EFI_ERROR(Status) )
            {
                if ( MessagingDevicePath->SubType == MSG_ATAPI_DP ) //IDE mode?
                {
                    if (PciDevicePath->Function == 5)
                        return gSATA[IdeDevice+2][IdeChannel];
                    else
                        return gSATA[IdeDevice][IdeChannel];
                }
                else
                    return IdeChannel;	//AHCI Port Number
            }
        }
    }
    return 0xff;
}
//TODO END :: <<<<<<


//*************************************************************************
//<AMI_PHDR_START>
//
// Name: CreateNameWithUefiOS
//
// Description:
//  Examines boot options and adds drive information string to any
//  Windows Boot Manager entries.  If SATA controller is in AHCI mode,
//  also adds port number.  This function is called at the
//  ProcessEnterSetup hook provided by TSE.
//
// Input:
//  IN EFI_DEVICE_PATH_PROTOCOL *DevicePath.
//  IN CHAR16 *BootOptionName.
// Output:
//  None.
//
// Returns:
//  EFI_STATUS.
//
// Modified:
//  String used by TSE when displaying  Boot Manager boot options.
//
// Referrals:
//
// Notes:
//  None.
//
//<AMI_PHDR_END>
//*************************************************************************
CHAR16* CreateNameWithUefiOS( IN EFI_DEVICE_PATH_PROTOCOL *DevicePath, IN CHAR16 *BootOptionName )
{
    EFI_STATUS Status;
    CHAR16* NewString = NULL;
    UINTN NewStringLength = 0;
    BOOLEAN SupportsSimpleFileSystem = FALSE;
    EFI_DEVICE_PATH_PROTOCOL* FullDevicePath = NULL;


    FullDevicePath = _DiscoverPartition(DevicePath);

    // Make sure that handle associated with full
    // device path supports Simple File System.
    if (FullDevicePath != NULL)
    {
        EFI_DEVICE_PATH_PROTOCOL* pTempDevicePath = FullDevicePath;
        EFI_HANDLE TempHandle = NULL;

        Status = pBS->LocateDevicePath (
                     &gEfiSimpleFileSystemProtocolGuid,
                     &pTempDevicePath,
                     &TempHandle );

        if (EFI_ERROR(Status))
        {
            TRACE((-1, "Does not support Simple File System.\n"));
            SupportsSimpleFileSystem = FALSE;
        }
        else
            SupportsSimpleFileSystem = TRUE;

        if ( SupportsSimpleFileSystem )
        {
            EFI_HANDLE BlkIoPartitionHandle = NULL;
            EFI_HANDLE ControllerHandle = NULL;
            CHAR16 ControllerNameBuffer[CONTROLLER_NAME_BUFFER_LENGTH];
            UINTN ControllerNameLength = 0;
            UINT16 PortNumber = 0;

            // Locate handle with above device path..
            Status = pBS->LocateDevicePath (
                         &gEfiBlockIoProtocolGuid,
                         &FullDevicePath,
                         &BlkIoPartitionHandle );

            if (EFI_ERROR(Status))
            {
                TRACE((-1, "LocateDevicePath: %r\n", Status));
                return NULL;
            }

            ControllerHandle = GetPhysicalBlockIoHandle(BlkIoPartitionHandle);
            if (EFI_ERROR(Status))
            {
                TRACE((-1, "GetPhysicalBlockIoHandle: %r\n", Status));
                return NULL;
            }

            ControllerNameLength = ConstructBootOptionNameByHandle (
                                       ControllerHandle,
                                       ControllerNameBuffer,
                                       CONTROLLER_NAME_BUFFER_LENGTH );

            // Allocate memory for new string and copy updated string to it.
            NewStringLength = Wcslen(ControllerNameBuffer) + Wcslen(BootOptionName) + 1;

            Status = pBS->AllocatePool (
                         EfiBootServicesData,
                         NewStringLength * sizeof(CHAR16),
                         &NewString );

            if (EFI_ERROR(Status))
            {
                TRACE((-1, "AllocatePool: %r\n", Status));
                return NULL;
            }

            // Append OS name and update bootdata pointer
            PortNumber = GetHDDPort( ControllerHandle );
            if (PortNumber != 0xff)
            {
                Swprintf_s ( NewString,
                             NEW_STRING_BUFFER_LENGTH,
                             L"%s (P%d: %s)",
                             BootOptionName,
                             PortNumber,
                             ControllerNameBuffer );
            }
            else
            {
                if ( ControllerNameLength == 0)
                {
                    Swprintf_s ( NewString,
                                 NEW_STRING_BUFFER_LENGTH,
                                 L"%s",
                                 BootOptionName );
                }
                else
                {
                    Swprintf_s ( NewString,
                                 NEW_STRING_BUFFER_LENGTH,
                                 L"%s (%s)",
                                 BootOptionName,
                                 ControllerNameBuffer );
                }
            }

            return NewString;
        }
        // Could not find drive with matching partition.
        else
        {
            NewStringLength = Wcslen(L" (Drive not present)") + Wcslen(BootOptionName) + 1;

            Status = pBS->AllocatePool ( EfiBootServicesData,
                                         NewStringLength * sizeof(CHAR16),
                                         &NewString );

            if (EFI_ERROR(Status))
            {
                TRACE((-1, "AllocatePool: %r\n", Status));
                return NULL;
            }

            Swprintf_s ( NewString,
                         NewStringLength,
                         L"%s (Drive not present)",
                         BootOptionName );

            return NewString;
        }
    }// if (FullDevicePath != NULL)
    return NULL;
}

//*************************************************************************
//<AMI_PHDR_START>
//
// Name: ChangeUefiBootNames
//
// Description:
//  Examines boot options and adds drive information string to any
//  Windows Boot Manager entries.  If SATA controller is in AHCI mode,
//  also adds port number.  This function is called at the
//  ProcessEnterSetup hook provided by TSE.
//
// Input:
//  None.
//
// Output:
//  None.
//
// Returns:
//  None.
//
// Modified:
//  String used by TSE when displaying Windows Boot Manager boot options.
//
// Referrals:
//  _DiscoverPartition()
//  GetSataPortNumber()
//  LocateDevicePath()
//  GetPhysicalBlockIoHandle()
//  ConstructBootOptionNameByHandle()
//  Swprintf_s()
//
// Notes:
//  None.
//
//<AMI_PHDR_END>
//*************************************************************************
VOID ChangeUefiBootNames(
    VOID
)
{
    UINTN i = 0;

    // Loop through all present boot options
    for ( i = 0; i < gBootOptionCount; i++ )
    {
        BOOT_DATA *bootData = NULL;

        bootData = &(gBootData[i]);

        if ( bootData->DevicePath->Type == MEDIA_DEVICE_PATH
                && bootData->DevicePath->SubType == MEDIA_HARDDRIVE_DP )
        {
            CHAR16 *NewBootOptionName;
            NewBootOptionName = CreateNameWithUefiOS( bootData->DevicePath, bootData->Name);
            if ( NewBootOptionName )
                bootData->Name = NewBootOptionName;
        }
    }//for( i = 0; i < gBootOptionCount; i++ )
    return;
}

//***********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//***********************************************************************
