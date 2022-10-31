//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/TCGSmm/TCGSmm.c 15    5/09/12 5:31p Fredericko $
//
// $Revision: 15 $
//
// $Date: 5/09/12 5:31p $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  TCGSmm.c
//
// Description:
// Function definition file for TCGSMM subcomponent
//
//<AMI_FHDR_END>
//*************************************************************************

#include "TCGSmm.h"

#if WORD_ACCESS_SMI_PORT == 0x01
void DisablePlatformSMI();
#endif

VOID NVOSWrite_PPI_request (t );

VOID NVOSWrite_MOR_request ();

VOID NVOSRead_PPI_request ( );

EFI_SMM_BASE2_PROTOCOL				*pSmmBase2;

EFI_STATUS GetNextGuidHob(
    IN OUT VOID          **HobStart,
    IN EFI_GUID          * Guid,
    OUT VOID             **Buffer,
    OUT UINTN*BufferSize OPTIONAL )
{
    return EFI_SUCCESS;
}


UINT8 ReadSmiPort(UINT16 Port)
{
   #if WORD_ACCESS_SMI_PORT == 0x00
        return (IoRead8(Port ));
   #else
        if(Port == (TCGSMIDATAPORT)){
           Port = TCGSMIPORT;   
           return ((UINT8)(((IoRead16( Port ) & 0xFF00))>> 8));
        }
        else{
            return ((UINT8)(IoRead16( Port ) & 0x00FF));
        }
   #endif
}


VOID WritebyteSmiPort(UINT16 Port, UINT8 data)
{
   UINT16 SmiPortVal = 0;

   #if WORD_ACCESS_SMI_PORT == 0x00
        IoWrite8(Port, data );    
   #else     
        SmiPortVal = ReadSmiPort (TCGSMIPORT);
        if(Port == TCGSMIDATAPORT)
        {
            SmiPortVal |= (data << 8);
        }else{
            return; //don't change current SMI value
        }
        Port = TCGSMIPORT;
        DisablePlatformSMI();  //put this function under board so that it is added to the CSP lib
        IoWrite16(Port, SmiPortVal );
        EnablePlatformSMI();
   #endif
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:  NVOSread_PPI_request
//
// Description: Returns TCG PPI variable values to the Operating system
//
//
// Input:       IN    EFI_HANDLE        DispatchHandle,
//              IN    EFI_SMM_SW_DISPATCH_CONTEXT    *DispatchContext
//
// Output:      VOID
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
VOID NVOSRead_PPI_request()
{
    UINTN          Size = sizeof(AMI_PPI_NV_VAR);
    AMI_PPI_NV_VAR Temp;
    UINT8          Read_value = 0;
    EFI_STATUS     Status;

    Read_value = ReadSmiPort( TCGSMIDATAPORT );

    Status = pRS->GetVariable( L"AMITCGPPIVAR", \
                                   &SmmtcgefiOsVariableGuid, \
                                   NULL, \
                                   &Size, \
                                   &Temp );

    if(Status){
        WritebyteSmiPort( TCGSMIDATAPORT, 0xFF );
        return;
    }
    
    switch (Read_value & TYPE_MASK ){
       case RQSTVAR: 
            WritebyteSmiPort( TCGSMIDATAPORT, Temp.RQST );
            break;
       case RCNTVAR:
            WritebyteSmiPort( TCGSMIDATAPORT, Temp.RCNT ); 
            break;
       case ERRORVAR:
             WritebyteSmiPort( TCGSMIDATAPORT, Temp.ERROR );
       case ERRORVAR2:
             WritebyteSmiPort( TCGSMIDATAPORT, Temp.AmiMisc );
            break;
       default:
            WritebyteSmiPort( TCGSMIDATAPORT, 0xFF );
            break;
    }
}

//****************************************************************************************
//<AMI_PHDR_START>
//
// Procedure:  NVWrite_PPI_request
//
// Description: Writes TCG PPI variable values to NVRAM on SMI request the Operating system
//
//
// Input:       IN    EFI_HANDLE        DispatchHandle,
//              IN    EFI_SMM_SW_DISPATCH_CONTEXT    *DispatchContext
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
VOID NVOSWrite_PPI_request( )
{
    UINTN          Size = sizeof(AMI_PPI_NV_VAR);
    AMI_PPI_NV_VAR Temp;
    EFI_STATUS     Status;
    UINT8          Read_value = 0;

    Read_value = ReadSmiPort( TCGSMIDATAPORT );

    if( Read_value == TCPA_PPIOP_UNOWNEDFIELDUPGRADE
            || Read_value == TCPA_PPIOP_SETOPAUTH 
            || Read_value == TCPA_PPIOP_SETNOPPIMAINTENANCE_FALSE
            || Read_value == TCPA_PPIOP_SETNOPPIMAINTENANCE_TRUE  
            || Read_value > TCPA_PPIOP_ENABLE_ACTV_CLEAR_ENABLE_ACTV)
    {
      WritebyteSmiPort( TCGSMIDATAPORT, 0xF1 );
      return;
    }

    if(Read_value >= 0 && Read_value < 23)
    {
        Temp.RQST  = Read_value;
        Temp.RCNT  = Read_value;
        Temp.ERROR = 0;
        Temp.Flag  = 0;
        Temp.AmiMisc = 0;

        Status = pRS->SetVariable( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               EFI_VARIABLE_NON_VOLATILE   \
                               | EFI_VARIABLE_RUNTIME_ACCESS   \
                               | EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                               Size, \
                               &Temp );

        if(Status){
            WritebyteSmiPort( TCGSMIDATAPORT, 0xFF );
            return;
        }
    }else{
        WritebyteSmiPort( TCGSMIDATAPORT, 0xF1 );
        return;
    }
}



#if defined TCGPPISPEC_1_2_SUPPORT && TCGPPISPEC_1_2_SUPPORT == 1
//****************************************************************************************
//<AMI_PHDR_START>
//
// Procedure: Read_User_Confirmation_Status
//
// Description: Reads the user confirmation satus for PPI requests
//
//
// Input:       IN    EFI_HANDLE        DispatchHandle,
//              IN    EFI_SMM_SW_DISPATCH_CONTEXT    *DispatchContext
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
VOID Read_User_Confirmation_Status( )
{
    UINTN                       Size = sizeof(PERSISTENT_BIOS_TPM_FLAGS);
    UINTN                       BiosSize = sizeof(AMI_PPI_NV_VAR);
    AMI_PPI_NV_VAR              Temp;
    PERSISTENT_BIOS_TPM_FLAGS   TpmNvFlags;
    UINT8                       Read_value = 0;
    EFI_STATUS                  Status;
    
    Read_value = ReadSmiPort( TCGSMIDATAPORT );

    Status = pRS->GetVariable( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               NULL, \
                               &BiosSize, \
                               &Temp );

    //reset ppi transaction flag
    Temp.Flag  = 0;

    Status = pRS->SetVariable( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               EFI_VARIABLE_NON_VOLATILE   \
                               | EFI_VARIABLE_RUNTIME_ACCESS   \
                               | EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                               BiosSize, \
                               &Temp );


    Status = pRS->GetVariable( L"TPMPERBIOSFLAGS", \
                              &SmmFlagsStatusguid, \
                              NULL, \
                              &Size, \
                              &TpmNvFlags );

    if(Read_value >= 0  && Read_value < 23)
    {
        if(Read_value == 0 )
        {
           WritebyteSmiPort( TCGSMIDATAPORT, 0x4 );
           return; 
        }

        if( Read_value == TCPA_PPIOP_UNOWNEDFIELDUPGRADE
            || Read_value == TCPA_PPIOP_SETOPAUTH 
            || Read_value == TCPA_PPIOP_SETNOPPIMAINTENANCE_FALSE
            || Read_value == TCPA_PPIOP_SETNOPPIMAINTENANCE_TRUE  
            || Read_value > TCPA_PPIOP_ENABLE_ACTV_CLEAR_ENABLE_ACTV)
        {
            WritebyteSmiPort( TCGSMIDATAPORT, 0x0 );
            return;
        }else if(Read_value == TCPA_PPIOP_CLEAR || Read_value == TCPA_PPIOP_ENABLE_ACTV_CLEAR )
        {
            if(TpmNvFlags.NoPpiClear  == TRUE){
                WritebyteSmiPort( TCGSMIDATAPORT, 0x4 );
            }else{
                WritebyteSmiPort( TCGSMIDATAPORT, 0x3 );
            }
            return;
        }else if(Read_value == TCPA_PPIOP_CLEAR_ENACT || Read_value == TCPA_PPIOP_ENABLE_ACTV_CLEAR_ENABLE_ACTV)
        {
            if(TpmNvFlags.NoPpiClear  == TRUE  && TpmNvFlags.NoPpiProvision == TRUE ){
                WritebyteSmiPort( TCGSMIDATAPORT, 0x4 );
            }else{
                WritebyteSmiPort( TCGSMIDATAPORT, 0x3 );
            }
            return;
        }else if(Read_value == TCPA_PPIOP_SETNOPPIPROVISION_FALSE || Read_value == TCPA_PPIOP_SETNOPPIPROVISION_TRUE)
        {
            if(Read_value == TCPA_PPIOP_SETNOPPIPROVISION_TRUE ){
                WritebyteSmiPort( TCGSMIDATAPORT, 0x3 );
            }else{
                WritebyteSmiPort( TCGSMIDATAPORT, 0x4 );
            }
            return;
        }else if(Read_value == TCPA_PPIOP_SETNOPPICLEAR_FALSE || Read_value == TCPA_PPIOP_SETNOPPICLEAR_TRUE)
        {
            if(Read_value == TCPA_PPIOP_SETNOPPICLEAR_TRUE ){
                WritebyteSmiPort( TCGSMIDATAPORT, 0x3 );
            }else{
                WritebyteSmiPort( TCGSMIDATAPORT, 0x4 );
            }
            return;
        }
        else if(TpmNvFlags.NoPpiProvision == TRUE)
        {
                WritebyteSmiPort( TCGSMIDATAPORT, 0x4 );
        }else
        {
                WritebyteSmiPort( TCGSMIDATAPORT, 0x3 );
        } 
    }else{
                WritebyteSmiPort( TCGSMIDATAPORT, 0x0 );
    } 
}
#endif



//****************************************************************************************
//<AMI_PHDR_START>
//
// Procedure: NVOSWrite_MOR_request
//
// Description: Writes TCG PPI MOR variable to NVRAM on SMI request the Operating system
//
//
// Input:       IN    EFI_HANDLE        DispatchHandle,
//              IN    EFI_SMM_SW_DISPATCH_CONTEXT    *DispatchContext
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
VOID NVOSWrite_MOR_request( )
{
    UINT8           mor  = 0;
    UINTN           MorSize = sizeof(mor);
    EFI_STATUS      Status;
    CHAR16          UefiMor[]   = L"MemoryOverwriteRequestControl";
    EFI_GUID        MorUefiGuid = MEMORY_ONLY_RESET_CONTROL_GUID;
    UINT8           Read_value  = 0;
    UINTN           PpiVarSize = sizeof(AMI_PPI_NV_VAR);
    AMI_PPI_NV_VAR  Temp;

    Read_value = ReadSmiPort( TCGSMIDATAPORT );

    Status = pRS->SetVariable( UefiMor, \
                               &MorUefiGuid, \
                               EFI_VARIABLE_NON_VOLATILE   \
                               | EFI_VARIABLE_RUNTIME_ACCESS   \
                               | EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                               MorSize, \
                               &Read_value );

    if(Status){
        WritebyteSmiPort( TCGSMIDATAPORT, 0xFF );
        return;
    }

    Status = pRS->GetVariable( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               NULL, \
                               &PpiVarSize, \
                               &Temp );

    if(Status){
        WritebyteSmiPort( TCGSMIDATAPORT, 0xFF );
        return;
    }

    Temp.Flag  = 0;

    Status = pRS->SetVariable( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               EFI_VARIABLE_NON_VOLATILE   \
                               | EFI_VARIABLE_RUNTIME_ACCESS   \
                               | EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                               PpiVarSize, \
                               &Temp );   

    if(Status){
        WritebyteSmiPort( TCGSMIDATAPORT, 0xFF );
        return;
    }
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TcgCommonSetFlag
//
// Description: Common function to set flag for PPI write transactions
//
//
// Input:       UINT8 Data 
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS  TcgCommonSetFlag(
    UINT8   Data 
)
{
    EFI_STATUS          Status;
    UINTN               Size = sizeof(AMI_PPI_NV_VAR);
    AMI_PPI_NV_VAR      Temp;

    Status = pRS->GetVariable( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               NULL, \
                               &Size, \
                               &Temp );

    if(Status)return Status;
    
    if((Data & TYPE_MASK) == WRITERQST){

         Temp.Flag = WRITEENDRQSTFLAG;

    }
    else if((Data & TYPE_MASK) == WRITEMOR){
     
         Temp.Flag = WRITEENDMORFLAG;
    }
#if defined TCGPPISPEC_1_2_SUPPORT && TCGPPISPEC_1_2_SUPPORT == 1
    else if((Data & TYPE_MASK) == CONFIRMATION){
     
         Temp.Flag = READENDCONFLAG;
    }
#endif
    
    Status = pRS->SetVariable( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               EFI_VARIABLE_NON_VOLATILE   \
                               | EFI_VARIABLE_RUNTIME_ACCESS   \
                               | EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                               Size, \
                               &Temp );   

    return Status;
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TcgSmiCommonHandler
//
// Description: Common function to handle TCG SMI's
//
//
// Input:       IN    EFI_HANDLE        DispatchHandle,
//              IN    EFI_SMM_SW_DISPATCH_CONTEXT    *DispatchContext
//
// Output:      VOID
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS TcgSmiCommonHandler(
	IN EFI_HANDLE  DispatchHandle,
	IN CONST VOID  *Context         OPTIONAL,
	IN OUT VOID    *CommBuffer      OPTIONAL,
	IN OUT UINTN   *CommBufferSize  OPTIONAL)
{
    EFI_STATUS          Status;
    UINTN               Size = sizeof(AMI_PPI_NV_VAR);
    AMI_PPI_NV_VAR      Temp;
    UINT8               Data;


    Data = ReadSmiPort( TCGSMIDATAPORT );

    Status = pRS->GetVariable( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               NULL, \
                               &Size, \
                               &Temp );

    if(Status){
        WritebyteSmiPort( TCGSMIDATAPORT, 0xFF );
        return Status;
    }

#if defined TCGPPISPEC_1_2_SUPPORT && TCGPPISPEC_1_2_SUPPORT == 1    
    if(Temp.Flag == READENDCONFLAG){
       Read_User_Confirmation_Status ( );
       return Status;
    }
#endif

    if(Temp.Flag == WRITEENDRQSTFLAG){

         NVOSWrite_PPI_request();
         return Status;

    }else if(Temp.Flag == WRITEENDMORFLAG){
     
        NVOSWrite_MOR_request(  );
        return Status;
    }
    
    switch(Data & TRANSACTION_MASK)
    {
        case READTRANSACTION:
                NVOSRead_PPI_request();
                break;
        case WRITETRANSACTION:
                Status = TcgCommonSetFlag(Data);
                if(Status){
                    WritebyteSmiPort( TCGSMIDATAPORT, 0xFF );
                 }
                 break;
#if defined TCGPPISPEC_1_2_SUPPORT && TCGPPISPEC_1_2_SUPPORT == 1
        case GETCONFTRANSACTION:
                Status = TcgCommonSetFlag(Data);
                if(Status){
                    WritebyteSmiPort( TCGSMIDATAPORT, 0x00 );
                 }
                break;
#endif
        default:
                break;
    }

    return Status;
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   HelpRegisterPPISMI
//
// Description: Common function to handle TCG SMI's
//
//
// Input:       IN    EFI_HANDLE ImageHandle,
//              IN    EFI_SYSTEM_TABLE *SystemTable
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS InSmmFunction(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable )
{
    EFI_SMM_SW_DISPATCH2_PROTOCOL *pSwDispatch2;
    EFI_SMM_SW_REGISTER_CONTEXT  SwContext;
    EFI_HANDLE                   Handle;
    EFI_HANDLE                   DummyHandle = NULL;
    EFI_STATUS                   Status;


    InitAmiSmmLib(ImageHandle, SystemTable);

    Status =  pSmstPi->SmmLocateProtocol(&gEfiSmmSwDispatch2ProtocolGuid, NULL, &pSwDispatch2);
    if (EFI_ERROR(Status)) return Status;

    SwContext.SwSmiInputValue = PPI_OFFSET;
    Status                    = pSwDispatch2->Register( pSwDispatch2,
                                                       TcgSmiCommonHandler,
                                                       &SwContext,
                                                       &Handle );
    
    ASSERT_EFI_ERROR( Status );

    if ( EFI_ERROR( Status )){
            return EFI_SUCCESS;
    }

    return Status;
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:    TCGSmmInit
//
// Description: Entry point for subcomponent
//
// Input:       IN    EFI_HANDLE ImageHandle,
//              IN    EFI_SYSTEM_TABLE *SystemTable
//
// Output: EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS TCGSmmInit(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable )
{
	EFI_STATUS Status;
    InitAmiLib( ImageHandle, SystemTable );

       Status = pBS->LocateProtocol(&gEfiSmmBase2ProtocolGuid, NULL, &pSmmBase2);
       if (EFI_ERROR(Status)) return Status;

    return InitSmmHandler( ImageHandle, SystemTable, InSmmFunction, NULL );
}

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
