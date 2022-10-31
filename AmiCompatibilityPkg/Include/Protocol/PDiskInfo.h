//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/BIN/Core/Modules/IdeBus/PDiskInfo.h 4     7/05/11 2:54a Anandakrishnanl $
//
// $Revision: 4 $
//
// $Date: 7/05/11 2:54a $
//**********************************************************************

#ifndef __PDISK_INFO_H__
#define __PDISK_INFO_H__

#include <Protocol/DiskInfo.h>

// NOTE: definitions below are commented as they are available in MdePkg/Include/Protocol/DiskInfo.h
//#define EFI_DISK_INFO_PROTOCOL_GUID \
//  { 0xd432a67f, 0x14dc, 0x484b, 0xb3, 0xbb, 0x3f, 0x2, 0x91, 0x84, 0x93, 0x27 }

extern EFI_GUID gDiskInfoProtocolGuid;

/*
typedef	struct _EFI_DISK_INFO_PROTOCOL EFI_DISK_INFO_PROTOCOL;

typedef
EFI_STATUS
(*EFI_DISK_INFO_INQUIRY) (
	IN EFI_DISK_INFO_PROTOCOL			*This,
	IN OUT VOID							*InquiryData,
	IN OUT UINT32						*InquiryDataSize
);

typedef
EFI_STATUS
(*EFI_DISK_INFO_IDENTIFY) (
	IN EFI_DISK_INFO_PROTOCOL			*This,
	IN OUT VOID							*IdentifyData,
	IN OUT UINT32						*IdentifyDataSize
);

typedef
EFI_STATUS
(*EFI_DISK_INFO_SENSE_DATA) (
	IN EFI_DISK_INFO_PROTOCOL			*This,
	IN OUT VOID							*SenseData,
	IN OUT UINT32						*SenseDataSize,
	OUT UINT8							*SenseDataNumber
);

typedef
EFI_STATUS
(*EFI_DISK_INFO_WHICH_IDE) (
	IN EFI_DISK_INFO_PROTOCOL			*This,
	OUT UINT32							*IdeChannel,
	OUT UINT32							*IdeDevice
);

#define EFI_DISK_INFO_IDE_INTERFACE_GUID \
  { 0x5e948fe3, 0x26d3, 0x42b5, 0xaf, 0x17, 0x61, 0x2, 0x87, 0x18, 0x8d, 0xec }

#define EFI_DISK_INFO_AHCI_INTERFACE_GUID \
{   0x9e498932, 0x4abc, 0x45af, 0xa3, 0x4d, 0x2, 0x47, 0x78, 0x7b, 0xe7, 0xc6 }

typedef struct _EFI_DISK_INFO_PROTOCOL {
  EFI_GUID                    Interface;
  EFI_DISK_INFO_INQUIRY       Inquiry;
  EFI_DISK_INFO_IDENTIFY      Identify;
  EFI_DISK_INFO_SENSE_DATA    SenseData;
  EFI_DISK_INFO_WHICH_IDE     WhichIde;
} EFI_DISK_INFO_PROTOCOL;
*/

#endif

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************
