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
// $Header: /Alaska/SOURCE/Modules/TCG/AmiTcgPlatform/AmiTcgPlatformDxe/AmiTcgPlatformDxeLib.c 5     9/27/11 10:33p Fredericko $
//
// $Revision: 5 $
//
// $Date: 9/27/11 10:33p $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:	AmiTcgPlatformDxeLib.c
//
// Description:	Function file for AmiTcgPlatformDxe library
//
//<AMI_FHDR_END>
//*************************************************************************
#include <AmiTcg\AmiTcgPlatformDxeLib.h>


EFI_GUID AmitcgefiOsVariableGuid       = AMI_TCG_EFI_OS_VARIABLE_GUID;

EFI_GUID  gSetupGuid = SETUP_GUID;
EFI_GUID  gEfiAmiboardTcgWakeEventDataHobGuid = EFI_TCG_WAKE_EVENT_DATA_HOB_GUID;
EFI_GUID  gEfiAmiboardHobListGuid = TCG_EFI_HOB_LIST_GUID;


//EFI_SMM_SYSTEM_TABLE             *mSmst;
//EFI_SMM_BASE_PROTOCOL            *pSmmBase;

EFI_GUID                         gDsdtGuid = DSDT_GUID;

EFI_STATUS getSetupData (
    SETUP_DATA** ppsd,
    UINT32    * pattr,
    UINTN     * psz );

//**********************************************************************
//                  TCG_Helper functions
//**********************************************************************
#pragma pack(1)
typedef struct _TCG_DXE_PRIVATE_DATA
{
    EFI_TCG_PROTOCOL        TcgServiceProtocol;
    EFI_TPM_DEVICE_PROTOCOL *TpmDevice;
} TCG_DXE_PRIVATE_DATA;
#pragma pack()

#define _CR( Record, TYPE,\
       Field )((TYPE*) ((CHAR8*) (Record) - (CHAR8*) &(((TYPE*) 0)->Field)))

#define TCG_DXE_PRIVATE_DATA_FROM_THIS( This )  \
    _CR( This, TCG_DXE_PRIVATE_DATA, TcgServiceProtocol )


EFI_STATUS
__stdcall TcgCommonPassThrough(
    IN VOID                    *Context,
    IN UINT32                  NoInputBuffers,
    IN TPM_TRANSMIT_BUFFER     *InputBuffers,
    IN UINT32                  NoOutputBuffers,
    IN OUT TPM_TRANSMIT_BUFFER *OutputBuffers )
{
    TCG_DXE_PRIVATE_DATA *Private;

    Private = TCG_DXE_PRIVATE_DATA_FROM_THIS( Context );
    return Private->TpmDevice->Transmit(
               Private->TpmDevice,
               NoInputBuffers,
               InputBuffers,
               NoOutputBuffers,
               OutputBuffers
               );
}





//*******************************************************************************
//<AMI_PHDR_START>
//
// Procedure:   GetTcgWakeEventType
//
// Description: Reads and Reports the source of the wake-up event.
//
// Input:       IN OUT UINT8   *pWake   - output parameter returns the indication of the
//                                        type of the wakup source:
//                              one of the following:
//                              SMBIOS_WAKEUP_TYPE_OTHERS
//                              SMBIOS_WAKEUP_TYPE_UNKNOWN
//                              SMBIOS_WAKEUP_TYPE_APM_TIMER
//                              SMBIOS_WAKEUP_TYPE_MODEM_RING
//                              SMBIOS_WAKEUP_TYPE_LAN_REMOTE
//                              SMBIOS_WAKEUP_TYPE_POWER_SWITCH
//                              SMBIOS_WAKEUP_TYPE_PCI_PME
//                              SMBIOS_WAKEUP_TYPE_AC_POWER_RESTORED
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//******************************************************************************
EFI_STATUS GetTcgWakeEventType(
    IN OUT UINT8 *pWake  )
{
    EFI_BOOT_MODE           *BootMode = NULL;
    UINTN                   NoTableEntries;
    EFI_CONFIGURATION_TABLE *ConfigTable;
    VOID                    *HobStart;

    *pWake = SMBIOS_WAKEUP_TYPE_UNKNOWN;

    NoTableEntries = pST->NumberOfTableEntries,
    ConfigTable    = pST->ConfigurationTable;

    while ( NoTableEntries > 0 )
    {
        NoTableEntries--;

        if ( !MemCmp(
                 &ConfigTable[NoTableEntries].VendorGuid,
                 &gEfiAmiboardHobListGuid, sizeof(EFI_GUID)
                 ))
        {
            HobStart = ConfigTable[NoTableEntries].VendorTable;

            if ( !EFI_ERROR(
            		TcgGetNextGuidHob( &HobStart, //EIP146351_146352
                                     &gEfiAmiboardTcgWakeEventDataHobGuid,
                                     &BootMode, NULL )
                     ))
            {
                break;
            }
        }
    }

    if ( BootMode != NULL )
    {
        if ( *BootMode == BOOT_ON_S4_RESUME || *BootMode == BOOT_ON_S5_RESUME  
              || *BootMode == BOOT_WITH_FULL_CONFIGURATION )
        {
            *pWake = (UINT8)SMBIOS_WAKEUP_TYPE_POWER_SWITCH;
        }
    }
    return EFI_SUCCESS;
}





//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:  read_PPI_request
//
// Description: Reads and returns TCG PPI requests Value
//
//
// Input:
//
// Output:      UINT8
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
UINT8 ReadPpiRequest( )
{
    UINTN          Size = sizeof(AMI_PPI_NV_VAR);
    AMI_PPI_NV_VAR Temp;
    EFI_STATUS     Status;

    Status = pRS->GetVariable( L"AMITCGPPIVAR", \
                               &AmitcgefiOsVariableGuid, \
                               NULL, \
                               &Size, \
                               &Temp );

    if(Status == EFI_NOT_FOUND)
    {
        Temp.RQST    = 0;
        Temp.RCNT    = 0;
        Temp.ERROR   = 0;
        Temp.Flag    = 0;
        Temp.AmiMisc = 0;

         Status = pRS->SetVariable( L"AMITCGPPIVAR", \
                             &AmitcgefiOsVariableGuid, \
                             EFI_VARIABLE_NON_VOLATILE   \
                               | EFI_VARIABLE_RUNTIME_ACCESS   \
                               | EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                               Size, \
                               &Temp );

    }

    return Temp.RQST;
}




//****************************************************************************************
//<AMI_PHDR_START>
//
// Procedure: Update_PpiVar
//
// Description: Updates TCG PPI variable in NVRAM
//
//
// Input:       IN  UINT8 value
//
// Output:      VOID
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//****************************************************************************************
void Update_PpiVar(
    IN UINT8 value )
{
    UINTN          Size = sizeof(AMI_PPI_NV_VAR);
    AMI_PPI_NV_VAR Temp;
    EFI_STATUS     Status;

    //now set variable to data
    Status = pRS->GetVariable( L"AMITCGPPIVAR", \
                               &AmitcgefiOsVariableGuid, \
                               NULL, \
                               &Size, \
                               &Temp );

    if ( EFI_ERROR( Status ))
    {
        TRACE((TRACE_ALWAYS, "Error Setting Return value\n"));
        return;
    }

    //get current value and set new value
    Temp.RQST = value;

    Status = pRS->SetVariable( L"AMITCGPPIVAR", \
                               &AmitcgefiOsVariableGuid, \
                               EFI_VARIABLE_NON_VOLATILE   \
                               | EFI_VARIABLE_RUNTIME_ACCESS   \
                               | EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                               Size, \
                               &Temp );
}

//****************************************************************************************
//<AMI_PHDR_START>
//
// Procedure: write_PPI_result
//
// Description: Updates TCG PPI variable in NVRAM
//
//
// Input:       IN  UINT8 last_op,
//              IN  UINT16 status
//
// Output:      VOID
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//****************************************************************************************
void WritePpiResult(
    IN UINT8  last_op,
    IN UINT16 status )
{
    UINTN          Size = sizeof(AMI_PPI_NV_VAR);
    AMI_PPI_NV_VAR Temp;
    EFI_STATUS     Status;
    UINT8          Manip = 0;

    Status = pRS->GetVariable( L"AMITCGPPIVAR", \
                               &AmitcgefiOsVariableGuid, \
                               NULL, \
                               &Size, \
                               &Temp );

    //now set variable to data
    Temp.RQST  = Manip;
    Manip      = (UINT8)( status & 0xFFFF );
    Temp.ERROR = Manip;

    if ( EFI_ERROR( Status ))
    {
        TRACE((TRACE_ALWAYS, "Error Setting Return value\n"));
        return;
    }

    Status = pRS->SetVariable( L"AMITCGPPIVAR", \
                               &AmitcgefiOsVariableGuid, \
                               EFI_VARIABLE_NON_VOLATILE   \
                               | EFI_VARIABLE_RUNTIME_ACCESS   \
                               | EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                               Size, \
                               &Temp );
}



//****************************************************************************************
//<AMI_PHDR_START>
//
// Procedure: getSetupData
//
// Description: Retrieved SETUP_DATA structure from NVRAM
//
//
// Input:       IN  OUT   SETUP_DATA** ppsd,
//              IN  UINT32* pattr,
//              IN  UINTN* psz
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//****************************************************************************************
EFI_STATUS getSetupData(
    IN OUT SETUP_DATA** ppsd,
    IN UINT32        * pattr,
    IN UINTN         * psz )
{
    EFI_STATUS Status;
    UINTN      sz = 0;

    *ppsd  = NULL;
    Status = pRS->GetVariable( L"Setup", &gSetupGuid, pattr, &sz, *ppsd );

    if ( !EFI_ERROR( Status ))
    {
        return Status;
    }

    if ( Status == EFI_BUFFER_TOO_SMALL )
    {
        Status = pBS->AllocatePool( EfiBootServicesData, sz, ppsd );

        if ( !(*ppsd))
        {
            return EFI_OUT_OF_RESOURCES;
        }
        Status = pRS->GetVariable( L"Setup", &gSetupGuid, pattr, &sz, *ppsd );
    }

    if ( psz != NULL )
    {
        *psz = sz;
    }
    return Status;
}
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
