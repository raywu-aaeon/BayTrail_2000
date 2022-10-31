//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
// $Header: /Alaska/SOURCE/Modules/HddSecurity/HddPassword/HddPassword.h 8     6/28/11 6:24a Anandv $
//
// $Revision: 8 $
//
// $Date: 6/28/11 6:24a $
//
//*****************************************************************************
//*****************************************************************************//

//<AMI_FHDR_START>
//---------------------------------------------------------------------------
//
// Name: HddPassword.h
//
// Description:	Header file for the HddPassword
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef _IDEPASSWORD_H_
#define _IDEPASSWORD_H_

#include "AmiStatusCodes.h"

#include "AMITSEStrTokens.h"

#define HDD_UNLOCKED_GUID \
    { 0x1fd29be6, 0x70d0, 0x42a4, 0xa6, 0xe7, 0xe5, 0xd1, 0xe, 0x6a, 0xc3, 0x76};


#define     SECURITY_SET_PASSWORD           0xF1
#define     SECURITY_UNLOCK                 0xF2
#define     SECURITY_ERASE_PREPARE          0xF3
#define     SECURITY_ERASE_UNIT             0xF4
#define     SECURITY_FREEZE_LOCK            0xF5
#define     SECURITY_DISABLE_PASSWORD       0xF6
#define     SECURITY_BUFFER_LENGTH          512     // Bytes

#define     SecuritySupportedMask       0x0001
#define     SecurityEnabledMask         0x0002
#define     SecurityLockedMask          0x0004
#define     SecurityFrozenMask          0x0008


#ifndef AMI_DXE_BS_EC_INVALID_IDE_PASSWORD
  #define AMI_DXE_BS_EC_INVALID_IDE_PASSWORD (AMI_STATUS_CODE_CLASS\
                                              | 0x00000005)
#endif

#ifndef DXE_INVALID_IDE_PASSWORD
  #define DXE_INVALID_IDE_PASSWORD (EFI_SOFTWARE_DXE_BS_DRIVER\
                                    | AMI_DXE_BS_EC_INVALID_IDE_PASSWORD)
#endif

#define NG_SIZE                     19
#define VARIABLE_ID_AMITSESETUP     5
#if !defined(SECURITY_SETUP_ON_SAME_PAGE) || SECURITY_SETUP_ON_SAME_PAGE == 0
#define INVALID_HANDLE  ((VOID*)-1)
#endif
extern VOID * gHiiHandle;
extern EFI_BOOT_SERVICES *gBS;
extern EFI_SYSTEM_TABLE *gST;

extern VOID *VarGetNvramName( CHAR16 *name, EFI_GUID *guid, UINT32 *attributes, UINTN *size );
extern EFI_STATUS VarSetNvramName( CHAR16 *name, EFI_GUID *guid, UINT32 attributes, VOID *buffer, UINTN size );
extern VOID MemFreePointer (VOID **ptr);
extern VOID MemCopy( VOID *dest, VOID *src, UINTN size );
extern UINT16 HiiAddString( /*EFI_HII_HANDLE*/VOID* handle, CHAR16 *string );
extern CHAR16 *HiiGetString( VOID* handle, UINT16 token );
VOID* IDEPasswordGetDataPtr( UINTN Index);
extern VOID    CheckForKeyHook( EFI_EVENT Event, VOID *Context );
extern UINTN TestPrintLength ( IN CHAR16   *String );
extern VOID _DrawPasswordWindow(UINT16 PromptToken, UINTN PasswordLength, UINTN *CurrXPos, UINTN *CurrYPos);
extern VOID _ReportInBox(UINTN PasswordLength, UINT16 BoxToken, UINTN CurrXPos, UINTN CurrYPos, BOOLEAN bWaitForReturn);
extern EFI_STATUS _GetPassword(CHAR16 *PasswordEntered, UINTN PasswordLength, UINTN CurrXPos, UINTN CurrYPos, UINTN *TimeOut);
extern VOID    ClearScreen( UINT8 Attrib );
extern EFI_STATUS ShowPostMsgBox(IN CHAR16  *MsgBoxTitle,IN CHAR16  *Message,IN UINT8  MsgBoxType, UINT8 *pSelection);
extern VOID    *SaveScreen( VOID );

EFI_STATUS IDEPasswordAuthenticate(
    CHAR16 *Password,
    VOID* Ptr,
    BOOLEAN bCheckUser
);

EFI_STATUS
EfiLibReportStatusCode (
  IN EFI_STATUS_CODE_TYPE     Type,
  IN EFI_STATUS_CODE_VALUE    Value,
  IN UINT32                   Instance,
  IN EFI_GUID                 *CallerId OPTIONAL,
  IN EFI_STATUS_CODE_DATA     *Data     OPTIONAL
  );

VOID *
EfiLibAllocateZeroPool (
  IN  UINTN   AllocationSize
  );

UINTN
SPrint (
  OUT CHAR16        *Buffer,
  IN  UINTN         BufferSize,
  IN  CONST CHAR16  *Format,
  ...
  );
//#include "tsecommon.h"

//#define EFI_DP_TYPE_MASK                    0x7F
//#define EFI_DP_TYPE_UNPACKED                0x80
//#if !defined(SECURITY_SETUP_ON_SAME_PAGE) || SECURITY_SETUP_ON_SAME_PAGE == 0
//#define END_DEVICE_PATH_TYPE                0x7f
//#define END_ENTIRE_DEVICE_PATH_SUBTYPE      0xff
//
//#define DevicePathType( a )           (((a)->Type) & EFI_DP_TYPE_MASK)
//#define DevicePathSubType( a )        ((a)->SubType)
//#define DevicePathNodeLength( a )     (((a)->Length[0]) | ((a)->Length[1] << 8))
//#define NextDevicePathNode( a )       ((EFI_DEVICE_PATH_PROTOCOL*) (((UINT8*) (\
//                                                                         a))\
//                                                                    +\
//                                                                   DevicePathNodeLength( a )))
//#define IsDevicePathEndType( a )      (\
//                                                                    DevicePathType( \
//            a ) == END_DEVICE_PATH_TYPE)
//#define IsDevicePathEndSubType( a )   ((a)->SubType ==\
//                                       END_ENTIRE_DEVICE_PATH_SUBTYPE)
//#define IsDevicePathEnd( a )          (IsDevicePathEndType( a )\
//                                       && IsDevicePathEndSubType( a ))
//#endif

#endif /* _PASSWORD_H_ */

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
