//**********************************************************************
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
//
//
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:  IsctNvsArea.h
//
// Description:	Data structures defined in this protocol are not naturally aligned.
//
//<AMI_FHDR_END>
//**********************************************************************

#ifndef _ISCT_NVS_AREA_H_
#define _ISCT_NVS_AREA_H_

//
// Includes
//
//
// Forward reference for pure ANSI compatability
//
EFI_FORWARD_DECLARATION (ISCT_NVS_AREA_PROTOCOL);

//
// Isct NVS Area Protocol GUID
//
#define ISCT_NVS_AREA_PROTOCOL_GUID \
  { \
    0x6614a586, 0x788c, 0x47e2, 0x89, 0x2d, 0x72, 0xe2, 0xc, 0x34, 0x48, 0x90 \
  }

//
// Extern the GUID for protocol users.
//
extern EFI_GUID gIsctNvsAreaProtocolGuid;

//
// Isct NVS Area definition
//
#pragma pack(1)
typedef struct {
  UINT8       IsctWakeReason;      //(0): Wake Reason
  UINT8       IsctEnabled;         //(1): 1 - Enabled, 0 - Disabled
  UINT8       IsctRTCTimerSupport; //(2): 1 - RTC timer enable.
  UINT32      RtcDurationTime;     //(3): RTC Duration Time
  UINT32      IsctNvsPtr;          //(7): Ptr of Isct GlobalNvs
  UINT8       IsctOverWrite;       //(11): 1 - Isct , 0 - OS RTC
} ISCT_NVS_AREA;
#pragma pack()

//
// Isct NVS Area Protocol
//
typedef struct _ISCT_NVS_AREA_PROTOCOL {
  ISCT_NVS_AREA     *Area;
  VOID              *IsctData;
} ISCT_NVS_AREA_PROTOCOL;

#endif

//**********************************************************************
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
//**********************************************************************
