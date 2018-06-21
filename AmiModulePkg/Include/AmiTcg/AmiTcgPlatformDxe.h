//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/AmiTcgPlatform/AmiTcgPlatformDxe/AmiTcgPlatformDxe.h 11    1/20/12 9:20p Fredericko $
//
// $Revision: 11 $
//
// $Date: 1/20/12 9:20p $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:	AmiTcgPlatformDxe.h
//
// Description:	Header file for AmiTcgPlatformDxe
//
//<AMI_FHDR_END>
//*************************************************************************
#include <AmiTcg\TcgPc.h>
//#include <AmiDxeLib.h>
#include <AmiTcg\TcgCommon.h>
#include <token.h>
#include <AmiTcg\tcg.h>
#if SMBIOS_SUPPORT == 1
//  #include <Protocol\SmBios.h>
//  #include <Protocol\AmiSmBios.h>
#endif
#include <AmiTcg\TcgMisc.h>
#include <Protocol\TcgTcmService.h>
#include <Protocol\TcgService.h>
#include <Protocol\TpmDevice.h>
#include <Include/Setup.h>
#include "Protocol/CpuIo.h"
#include "Protocol/FirmwareVolume.h"
#include "Protocol/DevicePath.h"
#include <Library\IoLib.h>
#include<Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include<Protocol/AmiTpmSupportTypeProtocol.h>
#if ( defined(CSM_SUPPORT) && (CSM_SUPPORT != 0) )
#include "Protocol/LegacyBios.h"
#endif

#include "Protocol/PciIo.h"
#include "Protocol/Variable.h"
#include "Protocol/Reset.h"
#include "Protocol/SimpleTextOut.h"
#include "Protocol/SimpleTextIn.h"
//#include "Protocol/UgaDraw.h"
//#include "Protocol/AMIPostMgr.h"
/*
#if EFI_SPECIFICATION_VERSION>0x20000 
    #include "Include\UefiHii.h"
    #include "Protocol/HiiDatabase.h"
    #include "Protocol/HiiString.h"
#else
  #include "Protocol/HII.h"
#endif
*/
#include <Protocol\SmmBase.h>
#include <Protocol\SmmSwDispatch.h>
#include <Protocol\Runtime.h>
//#include <Include\Acpi.h>
//#include <Include\Acpi11.h>
//#include "AmiTcgPlatformDxeLib.h"
//#include <AcpiRes.h>




//--------------------------------------------------------------------------
//GUID Definitions
//--------------------------------------------------------------------------
#define EFI_SMBIOS_TABLE_GUID \
    {0xeb9d2d31, 0x2d88, 0x11d3, 0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d}

#define AMI_TCG_PLATFORM_PROTOCOL_GUID\
    {0x8c939604, 0x700, 0x4415, 0x9d, 0x62, 0x11, 0x61, 0xdb, 0x81, 0x64, 0xa6}

#define  AMI_PROCESS_TCG_PPI_REQUEST_GUID\
    { 0x30ad2b83, 0xadd0, 0x414b, 0xb1, 0x1c, 0xf9, 0x3c, 0xc1, 0xd0, 0xb7,\
      0x9b}

#define  AMI_PROCESS_TCG_SETUP_GUID\
    { 0xc77dd102, 0x1db4, 0x4997, 0xae, 0x37, 0x4e, 0x8c, 0x52, 0x1e, 0xf5,\
      0x67}

#define  AMI_MEASURE_CPU_MICROCODE_GUID\
    { 0x5cf308b5, 0xfa23, 0x4100, 0x8a, 0x76, 0xf3, 0x26, 0xc2, 0x81, 0x48,\
      0x80}

#define  AMI_MEASURE_PCIOPROM_GUID\
    { 0x76f3992d, 0x529e, 0x4efe, 0x8b, 0xbe, 0x8e, 0x1e, 0xd4, 0x32, 0xc2,\
      0x23}

#define  AMI_SET_TCG_READYTOBOOT_GUID\
    { 0xa4524a9c, 0xb5e, 0x492d, 0xae, 0xc9, 0x30, 0x86, 0x31, 0xb1, 0x89,\
      0xb4}

#define TCG_SMBIOS_EFI_TABLE_GROUP \
    { 0xb3dae700, 0x2a77, 0x4ea4, 0xaf, 0x79, 0x32, 0x97, 0xb4, 0x84, 0xbe, 0x61 } 


EFI_GUID gEfiSmbiosTableGuid = EFI_SMBIOS_TABLE_GUID;

#define    MINI_SETUP_GUID { 0xB1DA0ADF, 0x4F77, 0x4070,\
      { 0xA8, 0x8E, 0xBF, 0xFE, 0x1C, 0x60, 0x52, 0x9A } }

#define   GUID_VARIABLE_DECLARATION( Variable, Guid ) extern EFI_GUID Variable

EFI_GUID  gMicrocodeGuid = {
    0x17088572, 0x377F, 0x44ef, 0x8F, 0x4E, 0xB0, 0x9F, 0xFF, 0x46, 0xA0, 0x70
};

#if TPM_PASSWORD_AUTHENTICATION

EFI_GUID  gAmitseAdminPasswordValidGuid = AMITSE_ADMIN_PASSWORD_VALID_GUID;


#define AMITSE_PASSWORD_PROMPT_EXIT_GUID { 0xb9b038b0, 0xe2b6, 0x4aab, \
    0x94, 0x35, 0x41, 0x65, 0xec, 0xfe, 0xd0, 0x32 }


EFI_GUID  gAmitsePasswordPromptExitGuid = AMITSE_PASSWORD_PROMPT_EXIT_GUID;

#define AMITSE_PASSWORD_PROMPT_ENTER_GUID \
{ 0x73e7e01, 0x2611, 0x4e85, 0xb8, 0x96, 0xa3, 0xb6, 0x76, 0x7c, 0xba, 0x0 }
  
EFI_GUID  gAmitsePasswordPromptEnterGuid = AMITSE_PASSWORD_PROMPT_ENTER_GUID;
#endif

EFI_GUID  gDsdtGuidDxe        = DSDT_GUID;
EFI_GUID  TcgEfiGlobalVariableGuid = TCG_EFI_GLOBAL_VARIABLE_GUID;

#define TCG_DEACTIVED_ERROR  0x06

//---------------------------------------------------------------------------
//      Structure Definitions
//---------------------------------------------------------------------------
#if defined CORE_REVISION && CORE_REVISION < 5
typedef struct _EFI_LIST_ENTRY
{
    struct _EFI_LIST_ENTRY *ForwardLink;
    struct _EFI_LIST_ENTRY *BackLink;
} EFI_LIST_ENTRY;
#endif

typedef struct
{
    UINTN          Signature;
    EFI_LIST_ENTRY AllEntries;            // All entries
    EFI_GUID       ProtocolID;            // ID of the protocol
    EFI_LIST_ENTRY Protocols;             // All protocol interfaces
    EFI_LIST_ENTRY Notify;                // Registerd notification handlers
} TCG_PROTOCOL_ENTRY;

typedef struct
{
    UINTN              Signature;
    TCG_PROTOCOL_ENTRY *Protocol;
    EFI_LIST_ENTRY     Link;                  // All notifications for this protocol
    EFI_EVENT          Event;                 // Event to notify
    EFI_LIST_ENTRY     *Position;              // Last position notified
} TCG_PROTOCOL_NOTIFY;

typedef struct
{
    UINT8 MajorVersion;
    UINT8 MinorVersion;
    UINT8 Reserve;
    UINT8 Flag;
} AMI_TCG_PROTOCOL_VERSION;




//**********************************************************************
//              Function Declarations
//**********************************************************************
EFI_STATUS 
OnAcpiInstalled(IN EFI_EVENT ev,
  IN VOID  *ctx );



EFI_STATUS measureCpuMicroCode();


EFI_FORWARD_DECLARATION( AMI_TCG_PLATFORM_PROTOCOL );


typedef
EFI_STATUS
(EFIAPI * MEASURE_CPU_MICROCODE)(

);


typedef
EFI_STATUS
(EFIAPI * MEASURE_HANDOFF_TABLES)(

);


typedef
EFI_STATUS
(EFIAPI * MEASURE_PCI_OPROMS)(

);


typedef
EFI_STATUS
(EFIAPI * PROCESS_TCG_SETUP)(

);



typedef
EFI_STATUS
(EFIAPI * PROCESS_TCG_PPI_REQUEST)(

);



typedef
EFI_STATUS
(EFIAPI * MEASURE_VARIABLES)(

);


typedef
EFI_STATUS
(EFIAPI * TCG_READY_TO_BOOT)(

);

typedef
EFI_STATUS
(EFIAPI * GET_PROTOCOL_VERSION)(
    AMI_TCG_PROTOCOL_VERSION *
);

typedef
EFI_STATUS
(EFIAPI * GET_)(
    AMI_TCG_PROTOCOL_VERSION *
);


typedef
VOID
(EFIAPI * RESETOSTCGVAR)(
); 


typedef
EFI_STATUS
(EFIAPI * AMI_TCG_DXE_FUNCTION_OVERRIDE)(
);



typedef struct _AMI_TCG_DXE_FUNCTION_OVERRIDE_PROTOCOL
{
    AMI_TCG_DXE_FUNCTION_OVERRIDE   Function;
} AMI_TCG_DXE_FUNCTION_OVERRIDE_PROTOCOL;



typedef EFI_STATUS (MEASURE_PCI_OPTION_ROM_FUNC_PTR)(

);

typedef EFI_STATUS (MEASURE_HANDOFF_TABLES_FUNC_PTR)(

);


typedef EFI_STATUS (MEASURE_CPU_MICROCODE_FUNC_PTR)(

);


typedef EFI_STATUS (MEASURE_BOOT_VAR_FUNC_PTR)(

);

#if (defined(TCGMeasureSecureBootVariables) && (TCGMeasureSecureBootVariables!= 0))
typedef EFI_STATUS (MEASURE_SECURE_BOOT_FUNC_PTR)(

);
#endif


typedef EFI_STATUS (MEASURES_TCG_BOOT_SEPARATORS_DXE_FUNC_PTR)(
    
);


typedef EFI_STATUS (MEASURE_WAKE_EVENT_DXE_FUNC_PTR)(

);


extern MEASURE_PCI_OPTION_ROM_FUNC_PTR  *MeasurePCIOpromsFuncPtr;
extern MEASURE_HANDOFF_TABLES_FUNC_PTR  *MeasureHandoffTablesFuncPtr;
extern MEASURE_CPU_MICROCODE_FUNC_PTR   *MeasureCpuMicroCodeFuncPtr;
extern MEASURE_BOOT_VAR_FUNC_PTR        *MeasureAllBootVariablesFuncPtr;
#if (defined(TCGMeasureSecureBootVariables) && (TCGMeasureSecureBootVariables != 0))
extern MEASURE_SECURE_BOOT_FUNC_PTR     *MeasureSecurebootVariablesFuncPtr;
#endif
extern MEASURES_TCG_BOOT_SEPARATORS_DXE_FUNC_PTR      *MeasureSeparatorsFuncPtr;
extern MEASURE_WAKE_EVENT_DXE_FUNC_PTR                *MeasureWakeEventFuncPtr;

typedef struct _AMI_TCG_PLATFORM_PROTOCOL
{
    MEASURE_CPU_MICROCODE       MeasureCpuMicroCode;
    MEASURE_PCI_OPROMS          MeasurePCIOproms;
    PROCESS_TCG_SETUP           ProcessTcgSetup;
    PROCESS_TCG_PPI_REQUEST     ProcessTcgPpiRequest;
    TCG_READY_TO_BOOT           SetTcgReadyToBoot;
    GET_PROTOCOL_VERSION        GetProtocolVersion;
    RESETOSTCGVAR               ResetOSTcgVar;
} AMI_TCG_PLATFORM_PROTOCOL;


extern EFI_GUID gAmiTcgPlatformProtocolguid;
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**     5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093            **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
