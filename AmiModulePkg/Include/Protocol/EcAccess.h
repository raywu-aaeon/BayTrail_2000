//**********************************************************************//
//**********************************************************************//
//**                                                                  **//
//**        (C)Copyright 1985-2003, American Megatrends, Inc.         **//
//**                                                                  **//
//**                       All Rights Reserved.                       **//
//**                                                                  **//
//**             6145-F Northbelt Pkwy, Norcross, GA 30071            **//
//**                                                                  **//
//**                       Phone: (770)-246-8600                      **//
//**                                                                  **//
//**********************************************************************//
//**********************************************************************//
//****************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//
//****************************************************************************
// Revision History
// ----------------
// $Log: $
// 
// 
//****************************************************************************

#ifndef __EFI_EC_ACCESS_H___
#define __EFI_EC_ACCESS_H___

#include <efi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EC_PROTOCOL_GUID \
	{0x70EEECBE, 0x727A, 0x4244, 0x90, 0x4C, 0xDB, 0x6B, 0xF0, 0x05, 0x53, 0x92}

//typedef struct gEfiEcAccessProtocolGuid _gEfiEcAccessProtocolGuid;
//
// EC Access specification constant and types
//


//
// EC Access specification Data Structures
//


typedef
EFI_STATUS
(EFIAPI *EC_QUERY_COMMAND) (
	UINT8 	*Qdata
);

typedef
EFI_STATUS
(EFIAPI *EC_WRITE_CMD) (
	UINT8    cmd
);

typedef
EFI_STATUS
(EFIAPI *EC_WRITE_DATA) (
	UINT8    data
);

typedef
EFI_STATUS
(EFIAPI *EC_READ_DATA) (
	UINT8 	*data
);

typedef
EFI_STATUS
(EFIAPI *EC_READ_MEM) (
	UINT8	Index,
	UINT8	*Data
);

typedef
EFI_STATUS
(EFIAPI *EC_WRITE_MEM) (
	UINT8	Index,
	UINT8	Data
);

typedef
EFI_STATUS
(EFIAPI *EC_ACPI_ENABLE) (
);


typedef
EFI_STATUS
(EFIAPI *EC_ACPI_DISABLE) (
);

typedef
EFI_STATUS
(EFIAPI *EC_SMI_NOTIFY_ENABLE) (
);

typedef
EFI_STATUS
(EFIAPI *EC_SMI_NOTIFY_DISABLE) (
);


typedef
EFI_STATUS	(EFIAPI *EC_SHUTDOWN_SYSTEM)(
);   			

typedef
EFI_STATUS  (EFIAPI *EC_GET_MOTHER_BOARD_ID)(
UINT8		*FabID
);

typedef
EFI_STATUS  (EFIAPI *EC_GET_EC_VERSION)(
UINT8		*Revision
);

typedef
EFI_STATUS	(EFIAPI *EC_ENABLE_LAN)(
);

typedef
EFI_STATUS  (EFIAPI *EC_DISABLE_LAN)(
);

typedef
EFI_STATUS  (EFIAPI *EC_DEEPSX_CONFIG)(
UINT8    ECData
);

typedef
EFI_STATUS  (EFIAPI *EC_TURBOCTRL_TESTMODE)(
UINT8    Enable,
UINT8    ACBrickCap,
UINT8    ECPollPeriod,
UINT8    ECGuardBandValue,
UINT8    ECAlgorithmSel,
UINT8    ECHybridPowerBoost,
UINT8    ECHybridCurrentLow,
UINT8    EDHybridCurrentHigh
);

typedef struct _EFI_EC_ACCESS_PROTOCOL {
  EFI_HANDLE            		Handle;
  EC_QUERY_COMMAND      		QuerryCmd;
  EC_WRITE_CMD          		WriteCmd;
  EC_WRITE_DATA         		WriteData;
  EC_READ_DATA					    ReadData;
  EC_READ_MEM					      ReadMem;
  EC_WRITE_MEM					    WriteMem;
  EC_ACPI_ENABLE				    AcpiEnable; 
  EC_ACPI_DISABLE				    AcpiDisable;
  EC_SMI_NOTIFY_ENABLE			SMINotifyEnable;
  EC_SMI_NOTIFY_DISABLE 		SMINotifyDisable;
  EC_SHUTDOWN_SYSTEM   			ShutDownSystem;
  EC_GET_MOTHER_BOARD_ID		GetMotherBoardID;
  EC_GET_EC_VERSION				  GetECVersion;
  EC_ENABLE_LAN					    EnableLan;
  EC_DISABLE_LAN				    DisableLan;
  EC_DEEPSX_CONFIG				  DeepSxConfig; 
  EC_TURBOCTRL_TESTMODE			TurboCtrlMode; 
} EFI_EC_ACCESS_PROTOCOL;


/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif
#endif





