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
// $Header: /Alaska/SOURCE/Modules/TCG/TcgDxeplatform/TcgDxeplatform.c 5     1/20/12 9:14p Fredericko $
//
// $Revision: 5 $
//
// $Date: 1/20/12 9:14p $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:	TcgDxeplatform.c
//
// Description:	Function file for TcgDxeplatform
//
//<AMI_FHDR_END>
//*************************************************************************
#include<EFI.h>
#include <AmiTcg\AmiTcgPlatformDxe.h>


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TcgDxeplatformEntry
//
// Description: 
//
// Input:       IN EFI_HANDLE       ImageHandle,
//              IN EFI_SYSTEM_TABLE *SystemTable
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
EFI_STATUS
EFIAPI TcgDxeplatformEntry(
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable ){

    AMI_TCG_PLATFORM_PROTOCOL  *AmiTcgPlatformProtocol = NULL;
    EFI_STATUS                 Status;
    BOOLEAN                    *ResetAllTcgVar = NULL;
    EFI_GUID                   legTcgGuid = AMI_TCG_RESETVAR_HOB_GUID;
    void                       ** DummyPtr;

    InitAmiLib( ImageHandle, SystemTable );

    DummyPtr       = &ResetAllTcgVar;
    ResetAllTcgVar = (UINT8*)LocateATcgHob(
        pST->NumberOfTableEntries,
        pST->ConfigurationTable,
        &legTcgGuid );

    Status = pBS->LocateProtocol( &gAmiTcgPlatformProtocolguid, NULL, 
                                            &AmiTcgPlatformProtocol);

    DummyPtr = &ResetAllTcgVar;

    if ( *DummyPtr != NULL )
    {
        //if ResetAllTcgVar, call setAllTcgVariable to zero
        if ( *ResetAllTcgVar == TRUE )
        {
            AmiTcgPlatformProtocol->ResetOSTcgVar();
        }
    }
  
    if(EFI_ERROR(Status)){
        return EFI_SUCCESS;
    }   


    Status = AmiTcgPlatformProtocol->ProcessTcgPpiRequest();
    if(EFI_ERROR(Status)){
      TRACE((TRACE_ALWAYS, "\n Possible ERROR Processing Ppi Request from O.S.\n"));
    }

    Status = AmiTcgPlatformProtocol->ProcessTcgSetup();
    if(EFI_ERROR(Status)){
      TRACE((TRACE_ALWAYS, "\n Possible ERROR Processing Tcg Setup\n"));
    }

#if (defined(MeasureCPUMicrocodeToken) && (MeasureCPUMicrocodeToken == 1))
    Status = AmiTcgPlatformProtocol->MeasureCpuMicroCode();
    if(EFI_ERROR(Status)){
        TRACE((TRACE_ALWAYS, "\n Possible ERROR Measuring CPU Microde\n"));
    }
#endif

    Status = AmiTcgPlatformProtocol->MeasurePCIOproms();
    if(EFI_ERROR(Status)){
        TRACE((TRACE_ALWAYS, "\n Possible ERROR Measuring PCI Option Roms\n"));
    }
    
    Status = AmiTcgPlatformProtocol->SetTcgReadyToBoot();
    if(EFI_ERROR(Status)){
      TRACE((TRACE_ALWAYS, "\n Possible ERROR process Tcg Ready to boot Callback\n"));
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
//**********************************************************************
