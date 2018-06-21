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
// $Header: /Alaska/SOURCE/Modules/TcgNext/Common/TcgSmm/TCGSmm.c 1     10/08/13 12:05p Fredericko $
//
// $Revision: 1 $
//
// $Date: 10/08/13 12:05p $
//*************************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/Modules/TcgNext/Common/TcgSmm/TCGSmm.c $
// 
// 1     10/08/13 12:05p Fredericko
// Initial Check-In for Tpm-Next module
// 
// 2     8/30/13 11:02p Fredericko
// 
// 1     7/10/13 5:57p Fredericko
// [TAG]  		EIP120969
// [Category]  	New Feature
// [Description]  	TCG (TPM20)
// 
// 17    7/31/12 6:27p Fredericko
// [TAG]  		EIP94589
// [Category]  	Improvement
// [Description]  	Use better variable names in TcgSmm.c
// [Files]  		TcgSmm.c
// 
// 16    5/20/12 2:12p Fredericko
// 
// 15    5/09/12 5:31p Fredericko
// Change Port address to support 16bit port addresses. Some Chipsets
// require this.
// 
// 14    3/19/12 6:37p Fredericko
// [TAG]  		EIP82866
// [Description]  	AMIUEFI: Implement the NoPPIClear flag and provide
// operations to set/clear the value or a BIOS config option -  Windows
// Partner Bug Management Bug #679996
// [Files]  		AmiTcgNvFlagSample.c, AmiTcgNvFlagSample.sdl,
// AmiTcgPlatformDxe.c
// 
// 13    12/12/11 1:08p Fredericko
// [TAG]  		EIP59683
// [Category]  	Improvement
// [Description]  	Allow selection between writing to SMI port as a word
// or as a Byte.
// Some platforms might require word writes to the SMI Status port.
// [Files]  		Tcg.cif, Tcg.sdl, Tcg_ppi1_2_Ex.asl, TcgSmm.mak, TcgSmm.c
// 
// 12    12/07/11 4:27p Fredericko
// 
// 11    12/07/11 4:26p Fredericko
// [TAG]  		EIP59683
// [Category]  	Improvement
// [Description]  	Allow selection between writing to SMI port as a word
// or as a Byte.
// Some platforms might require word writes to the SMI Status port.
// [Files]  		Tcg.cif, Tcg.sdl, Tcg_ppi1_2_Ex.asl, TcgSmm.mak, TcgSmm.c
// 
// 10    8/10/11 4:30p Fredericko
// [TAG]  		EIPEIP66468 
// [Category]  	Spec Update
// [Severity]  	Minor
// [Description]  	1. Added some more boundary checking for unsupported
// functions and for handling of Ppi 0
// [Files]  		1. TcgSmm.c
// 
// 9     8/09/11 6:29p Fredericko
// [TAG]  		EIP66468
// [Category]  	Spec Update
// [Severity]  	Minor
// [Description]  	1. Changes for Tcg Ppi 1.2 support. 
// [Files]  		1 TcgSmm.h
// 2.TcgSmm.c
// 3.Tcg_ppi1_2.asl
// 4. AmiTcgNvflagsSample.c
// 5. AmiTcgPlatformPeiLib.c
// 6. AmiTcgPlatformDxe.sdl
// 7. AmiTcgPlatformDxe.c
// 
// 8     7/25/11 3:20a Fredericko
// [TAG]  		EIP65177
// [Category]  	Spec Update
// [Severity]  	Minor
// [Description]  	TCG Ppi Sec ver 1.2 update
// 
// 7     2/16/11 10:37a Fredericko
//  [TAG]    	 EIP54014
//  [Category] BUG FIX
//  [Severity]   HIGH
//  [Symptom] TPM initialize failed using win7 tool tpm.msc after drive
// bitlocker test.
//  [RootCause]  wrong size used for getvariable
//  [Solution]	 Use correct size
//  [Files]	TCGSmm.c
// 
// 6     8/23/10 4:21p Fredericko
// Code Clean up. Removed port 80 checkpoint writes from code.
// 
// 5     8/09/10 2:34p Fredericko
// Added NVRAM writes functions for TCG PPI support. Moved from TcgBoard
// component
// 
// 4     8/04/10 5:07p Fredericko
// Changed AMI interface to use only one SMI value instead of 3
// 
// 3     5/20/10 8:54a Fredericko
// Included File Header
// Included File Revision History 
// Code Beautification
// EIP 37653
//
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
#include <Setup.h>
//#include <Guid\MemoryOverwriteControl.h>
#include <Library\IoLib.h>
#include <Protocol\SmmVariable.h>
#include<Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/Debuglib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <AmiDxeLib.h>


#pragma optimize("",off)

#if WORD_ACCESS_SMI_PORT == 0x01
void DisablePlatformSMI();
#endif


static EFI_GUID AmiNvramControlProtocolGuid = { 0xf7ca7568, 0x5a09, 0x4d2c, { 0x8a, 0x9b, 0x75, 0x84, 0x68, 0x59, 0x2a, 0xe2 } };
typedef EFI_STATUS (*SHOW_BOOT_TIME_VARIABLES)(BOOLEAN Show);

typedef struct{
    SHOW_BOOT_TIME_VARIABLES ShowBootTimeVariables;
} AMI_NVRAM_CONTROL_PROTOCOL;

AMI_NVRAM_CONTROL_PROTOCOL *NvramControl = NULL;

EFI_RUNTIME_SERVICES         *ptrRuntimeServices = NULL;


VOID NVOSWrite_PPI_request (t );

VOID NVOSWrite_MOR_request ();

VOID NVOSRead_PPI_request ( );

EFI_SMM_BASE2_PROTOCOL				*pSmmBase2;

EFI_STATUS TcgGetNextGuidHob(
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


VOID* TcgGetSmstConfigurationTablePi(IN EFI_GUID *TableGuid){
    EFI_CONFIGURATION_TABLE *Table;
    UINTN i;

    ASSERT(gSmst!=NULL); //pSmstFramework must be initialized with the call to InitSmiHandler
    if (gSmst==NULL) return NULL;
    Table = gSmst->SmmConfigurationTable;
    i = gSmst->NumberOfTableEntries;

    for (;i;--i,++Table)
    {
        if (CompareMem(&Table->VendorGuid,TableGuid, sizeof(EFI_GUID))==0) 
            return Table->VendorTable;
    }
    return NULL;
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

    if (NvramControl == NULL)
        NvramControl = TcgGetSmstConfigurationTablePi(&AmiNvramControlProtocolGuid);
            
    if (NvramControl) NvramControl->ShowBootTimeVariables(TRUE);
    Status = ptrRuntimeServices->GetVariable( L"AMITCGPPIVAR", \
                                   &SmmtcgefiOsVariableGuid, \
                                   NULL, \
                                   &Size, \
                                   &Temp );
    
    if (NvramControl) NvramControl->ShowBootTimeVariables(FALSE);

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
    UINT32          attrib =     EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS;

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
        
        if (NvramControl == NULL)
            NvramControl = TcgGetSmstConfigurationTablePi(&AmiNvramControlProtocolGuid);
        
        if (NvramControl) NvramControl->ShowBootTimeVariables(TRUE);

        Status = ptrRuntimeServices->SetVariable ( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               attrib, \
                               Size, \
                               &Temp );
        
        if(Status == EFI_INVALID_PARAMETER)
        {
             Status = ptrRuntimeServices->SetVariable(L"AMITCGPPIVAR", \
                             &SmmtcgefiOsVariableGuid, \
                             0, \
                             0, \
                             NULL); 

             if(EFI_ERROR(Status)){

                if (NvramControl) NvramControl->ShowBootTimeVariables(FALSE);
			 	return;
			 
			 }         

             Status = ptrRuntimeServices->SetVariable( L"AMITCGPPIVAR", \
                             &SmmtcgefiOsVariableGuid, \
                             attrib, \
                             Size, \
                            &Temp );  
        }


        if (NvramControl) NvramControl->ShowBootTimeVariables(FALSE);

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
    SETUP_DATA                  SetupDataBuffer;
    UINTN                       SetupVariableSize = sizeof(SETUP_DATA);
    UINT32                      SetupVariableAttributes=0;
    EFI_GUID                    gSetupGuid = SETUP_GUID;
    UINT32                      attrib =     EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS;
    
    Read_value = ReadSmiPort( TCGSMIDATAPORT );
    
    if (NvramControl == NULL)
             NvramControl = TcgGetSmstConfigurationTablePi(&AmiNvramControlProtocolGuid);
         
    if (NvramControl) NvramControl->ShowBootTimeVariables(TRUE);
    
    Status = ptrRuntimeServices->GetVariable( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               NULL, \
                               &BiosSize, \
                               &Temp );

    //reset ppi transaction flag
    Temp.Flag  = 0;

    Status = ptrRuntimeServices->SetVariable ( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               EFI_VARIABLE_NON_VOLATILE   \
                               | EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                               BiosSize, \
                               &Temp );

    if(Status == EFI_INVALID_PARAMETER)
    {
         Status = ptrRuntimeServices->SetVariable(L"AMITCGPPIVAR", \
                        &SmmtcgefiOsVariableGuid, \
                         0, \
                         0, \
                         NULL); 

         if(EFI_ERROR(Status)){

         	if (NvramControl) NvramControl->ShowBootTimeVariables(FALSE);
		 		return;
			 
		 }        

         Status = ptrRuntimeServices->SetVariable( L"AMITCGPPIVAR", \
                             &SmmtcgefiOsVariableGuid, \
                             attrib, \
                             BiosSize, \
                             &Temp );
    }



    Status = ptrRuntimeServices->GetVariable( L"TPMPERBIOSFLAGS", \
                              &SmmFlagsStatusguid, \
                              NULL, \
                              &Size, \
                              &TpmNvFlags );

    if (NvramControl) NvramControl->ShowBootTimeVariables(FALSE);
    
    Status = ptrRuntimeServices->GetVariable (
                            L"Setup",
                            &gSetupGuid,
                            &SetupVariableAttributes,
                            &SetupVariableSize,
                            &SetupDataBuffer);

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
                if(SetupDataBuffer.Tpm20Device == 1){
                    WritebyteSmiPort( TCGSMIDATAPORT, 0x4 );
                }else{                            
                    WritebyteSmiPort( TCGSMIDATAPORT, 0x3 );
                }
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
    UINT32          attrib =     EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS;
    AMI_PPI_NV_VAR  Temp;

    Read_value = ReadSmiPort( TCGSMIDATAPORT );

    Status = ptrRuntimeServices->SetVariable ( UefiMor, \
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
    
    if (NvramControl) NvramControl->ShowBootTimeVariables(TRUE);

    Status = ptrRuntimeServices->GetVariable( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               NULL, \
                               &PpiVarSize, \
                               &Temp );
    if (NvramControl) NvramControl->ShowBootTimeVariables(FALSE);
    
    if(Status){
        WritebyteSmiPort( TCGSMIDATAPORT, 0xFF );
        return;
    }

    Temp.Flag  = 0;

    if (NvramControl) NvramControl->ShowBootTimeVariables(TRUE);
    
    Status = ptrRuntimeServices->SetVariable( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               attrib, \
                               PpiVarSize, \
                               &Temp );   

    if(Status == EFI_INVALID_PARAMETER)
    {
           Status = ptrRuntimeServices->SetVariable(L"AMITCGPPIVAR", \
                         &SmmtcgefiOsVariableGuid, \
                         0, \
                         0, \
                         NULL); 

           if(EFI_ERROR(Status)){

         		if (NvramControl) NvramControl->ShowBootTimeVariables(FALSE);
		 		return;
			 
		   }     

           Status = ptrRuntimeServices->SetVariable( L"AMITCGPPIVAR", \
                             &SmmtcgefiOsVariableGuid, \
                             attrib, \
                             PpiVarSize, \
                              &Temp);  
    }

    if (NvramControl) NvramControl->ShowBootTimeVariables(FALSE);

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
    UINT32 attrib =     EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS;

    if (NvramControl == NULL)
        NvramControl = TcgGetSmstConfigurationTablePi(&AmiNvramControlProtocolGuid);
          
    if (NvramControl) NvramControl->ShowBootTimeVariables(TRUE);

    Status = ptrRuntimeServices->GetVariable( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               NULL, \
                               &Size, \
                               &Temp );
    if (NvramControl) NvramControl->ShowBootTimeVariables(FALSE);

    if(Status){
        return Status;
    }
    
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
    
    if (NvramControl) NvramControl->ShowBootTimeVariables(TRUE);
    
    Status = ptrRuntimeServices->SetVariable( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                                   attrib, \
                                   Size, \
                                   &Temp );   

    if(Status == EFI_INVALID_PARAMETER)
    {
       Status = ptrRuntimeServices->SetVariable(L"AMITCGPPIVAR", \
                        &SmmtcgefiOsVariableGuid, \
                        0, \
                        0, \
                        NULL); 

       if(EFI_ERROR(Status)){

         	if (NvramControl) NvramControl->ShowBootTimeVariables(FALSE);
		 		return Status;
			 
		 }       

       Status = ptrRuntimeServices->SetVariable( L"AMITCGPPIVAR", \
                            &SmmtcgefiOsVariableGuid, \
                            attrib, \
                            Size, \
                            &Temp);  
    }


    if (NvramControl) NvramControl->ShowBootTimeVariables(FALSE);
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
    
    if (NvramControl == NULL)
	    NvramControl = TcgGetSmstConfigurationTablePi(&AmiNvramControlProtocolGuid);
    
    if (NvramControl) NvramControl->ShowBootTimeVariables(TRUE);

    Status = ptrRuntimeServices->GetVariable( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               NULL, \
                               &Size, \
                               &Temp );
    
    if (NvramControl) NvramControl->ShowBootTimeVariables(FALSE);

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
EFI_STATUS TCGSmmInit(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable )
{
    EFI_SMM_SW_DISPATCH2_PROTOCOL *pSwDispatch2;
    EFI_SMM_SW_REGISTER_CONTEXT  SwContext;
    EFI_HANDLE                   Handle;
    EFI_HANDLE                   DummyHandle = NULL;
    EFI_STATUS                   Status;
    EFI_GUID                     SmmRtServTableGuid  = EFI_SMM_RUNTIME_SERVICES_TABLE_GUID; 
   
    Status =  gSmst->SmmLocateProtocol(&gEfiSmmSwDispatch2ProtocolGuid, NULL, &pSwDispatch2);
    if (EFI_ERROR(Status)) return Status;
    
    ptrRuntimeServices = (EFI_RUNTIME_SERVICES *)TcgGetSmstConfigurationTablePi(&SmmRtServTableGuid);
    if(ptrRuntimeServices == NULL) return EFI_NOT_FOUND;

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

#pragma optimize("",on)

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
