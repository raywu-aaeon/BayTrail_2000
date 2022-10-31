//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
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
// $Header: /Alaska/SOURCE/Modules/Legacy Serial Redirection/LegacySredir.h 18    2/29/12 11:39p Rameshr $
//
// $Revision: 18 $
//
// $Date: 2/29/12 11:39p $
//**********************************************************************

//<AMI_FHDR_START>
//****************************************************************************
//
// Name:        LegacySreDir.h
//
// Description: Legacy console redirection Protocol header file
//****************************************************************************
//<AMI_FHDR_END>

#ifndef __LEGACY_SREDIR_PROTOCOL_H__
#define __LEGACY_SREDIR_PROTOCOL_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <EFI.h>

#define EFI_LEGACY_SREDIR_PROTOCOL_GUID \
  { 0xA062CF1F, 0x8473, 0x4aa3, 0x87, 0x93, 0x60, 0x0B, 0xC4, 0xFF, 0xA9, 0xA9 }

//#ifndef GUID_VARIABLE_DECLARATION
//#define GUID_VARIABLE_DECLARATION(Variable, Guid) extern EFI_GUID Variable
//#endif

//GUID_VARIABLE_DECLARATION(gEfiLegacySredirProtocolGuid, EFI_LEGACY_SREDIR_PROTOCOL_GUID);

extern EFI_GUID gEfiLegacySredirProtocolGuid;

//#ifndef GUID_VARIABLE_DEFINITION
typedef struct _EFI_LEGACY_SREDIR_PROTOCOL EFI_LEGACY_SREDIR_PROTOCOL;

#define SREDIR_VENDORID 0x7
#define SREDIR_DEVICEID 0x9
#define SREDIR_MODULEID 0x3

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        EFI_LEGACYSREDIR_TABLE
//
// Desription:  There is a table located within the traditional BIOS.It is located on a 16-byte
//              boundary and provides the physical address of the entry point for the Legacy Redirection
//              functions. These functions provide the platform-specific information that is 
//              required by the generic EfiCompatibility code. The functions are invoked via 
//              thunking by using EFI_LEGACY_BIOS_PROTOCOL.FarCall86() with the 32-bit physical 
//              entry point defined below. 
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
#pragma pack(1)
typedef struct _EFI_LEGACYSREDIR_TABLE {
    UINT32      Signature;
    UINT8       Version;
    UINT8       TableChecksum;
    UINT8       TableLength;
    UINT16      SreDirSegment;
    UINT16      SreDirOffset;
    UINT16      SreDirEfiToLegacyOffset;
} EFI_LEGACYSREDIR_TABLE;
#pragma pack()

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:	EFI_LEGACYSREDIR_FUNCTIONS
//
// Description:	This structure consists of functions which are used for Leagcy Redirection
//
// Fields:
//
// LEGACY_SreDirInitializeSerialPort : Initialises the Serial Port
// LEGACY_SerialRedirection          : Starts the Legacy Serial Redirection by hooking the required interrupts
// LEGACY_ReleaseSerialRedirection   : Stops the Legacy Serial Redirection by Releasing the corresponding interrupts
// LEGACY_InvalidFunction_FAR        : Invalid function
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef enum {
    LEGACY_SreDirInitializeSerialPort,  // 0000,
    LEGACY_SerialRedirection,           // 0001,
    LEGACY_ReleaseSerialRedirection,    // 0002,
    LEGACY_GetInterruptAddress,         // 0003,
	LEGACY_ClearKbCharBuffer,			// 0004,
    LEGACY_InvalidFunction_FAR,         // 0005
} EFI_LEGACYSREDIR_FUNCTIONS;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:	EFI_COM_PARAMETERS
//
// Description:	This structure consists of Com parameters and setup values
//              which are used to transfer the data from EFI to Legacy for
//              Legacy Redirection.
//
//
// Notes:       Don't Change this structure,as the same structure is defined in
//              CSM16 Serial Redirection. 
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
#pragma pack(1)
typedef struct _EFI_COM_PARAMETERS {
    UINT32 SdlParameters;
    UINT16 BaseAddress;
    UINT32 MMIOBaseAddress;
    UINT8  SerialIRQ;
    UINT32 Baudrate;
    UINT8  TerminalType;
    UINT8  FlowControl;
    UINT8  DataParityStop;
    UINT8  LegacyOsResolution;
    UINT8  RecorderMode;
    UINT8  VtUtf8;
    UINT8  PuttyKeyPad;
    UINT8  SwSMIValue;
    UINT8  InstallLegacyOSthroughRemote;
    UINT16 SredirBinSize; 

    UINT8  RedirectionAfterBiosPost;
    UINT8  Flag;               //This is not a setup variable rather than used as 
                               // a flag to start or stop Serial Redirection
} EFI_COM_PARAMETERS;
#pragma pack()


//
// The Legacy Console Reirection enable Procedure.
//
typedef EFI_STATUS (EFIAPI *LEGACY_SREDIR_ENABLE) (
    IN EFI_LEGACY_SREDIR_PROTOCOL   *This
);

//
// The Legacy Console Reirection disable Procedure.
//
typedef EFI_STATUS (EFIAPI *LEGACY_SREDIR_DISABLE) (
    IN EFI_LEGACY_SREDIR_PROTOCOL   *This
);

typedef struct _EFI_LEGACY_SREDIR_PROTOCOL {
    LEGACY_SREDIR_ENABLE    EnableLegacySredir;
    LEGACY_SREDIR_DISABLE   DisableLegacySredir;
} EFI_LEGACY_SREDIR_PROTOCOL;


/****** DO NOT WRITE BELOW THIS LINE *******/
//#endif // #ifndef GUID_VARIABLE_DEFINITION
#ifdef __cplusplus
}
#endif
#endif

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
