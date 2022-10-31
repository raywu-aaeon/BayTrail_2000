#include <AmiTcg\AmiTcgPlatformDxeLib.h>
#include <AmiTcg\TcgMisc.h>
#include <AmiTcg\tcg.h>
#include <Acpi.h>
#include <AcpiRes.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/LoadFile2.h>
#include "SynTCGAslVar.h"
#include <Token.h>

#define SyncDetailDbgMsg    0

#if SyncDetailDbgMsg
#define _TRACE(a)
#else
#define _TRACE TRACE
#endif

EFI_GUID        gTcgAcpiSupportGuid = EFI_ACPI_SUPPORT_GUID;
EFI_GUID        SmmtcgefiOsVariableGuid    = AMI_TCG_EFI_OS_VARIABLE_GUID;
EFI_GUID        SmmFlagsStatusguid         = AMI_TCG_CONFIRMATION_FLAGS_GUID;
static UINT64   PPIVarAddr = 0;

extern  EFI_GUID gEfiGlobalNvsAreaProtocolGuid;
extern EFI_HANDLE _gImageHandle;

EFI_GUID    gAcpi20TableGuid        = ACPI_20_TABLE_GUID;

EFI_STATUS UpdateCSBit()
{
    RSDT_PTR_20 *RSDP = NULL;
    RSDT_20     *RSDT = NULL;
    XSDT_20     *XSDT = NULL;
    FACP_20     *FADT = 0;
    UINT32      i;

    RSDP = GetEfiConfigurationTable(pST,&gAcpi20TableGuid);
    if (!RSDP) {
        TRACE((-1,"TPM12[%d]: Get gAcpi20TableGuid Table Address Failed\n", __LINE__));
        return EFI_NOT_FOUND;   
    }

    RSDT = (RSDT_20*)RSDP->RsdtAddr;    // 32-bit pointer table
    XSDT = (XSDT_20*)RSDP->XsdtAddr;    // 64-bit pointer table.

    // Get XSDT FACS Pointers
    if (XSDT) {
        UINT32 NumPtrs = (XSDT->Header.Length - sizeof(ACPI_HDR)) / 8;
        for(i = 0; i < NumPtrs; ++i) {
            if (((ACPI_HDR*)XSDT->Ptrs[i])->Signature == 'PCAF') {
                FADT = (FACP_20*)XSDT->Ptrs[i];
                _TRACE(( -1, "TPM12[%d]: Disable the Connect Standby bit on FACP(XSDT) Table\n", __LINE__));
                ((EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE *)FADT)->Flags &= 0xFFDFFFFF;
                FADT->Header.Checksum = ChsumTbl((UINT8*)FADT, FADT->Header.Length);
                break;
            }
        }
    }

    // Get RSDT FACS Pointer
    if (RSDT) {
        UINT32 NumPtrs = (RSDT->Header.Length - sizeof(ACPI_HDR)) / 4;
        for(i = 0; i < NumPtrs; ++i) {
            if (((ACPI_HDR*)RSDT->Ptrs[i])->Signature == 'PCAF') {
                FADT = (FACP_20*)RSDT->Ptrs[i];
                _TRACE(( -1, "TPM12[%d]: Disable the Connect Standby bit on FACP(RSDT) Table\n", __LINE__));
                ((EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE *)FADT)->Flags &= 0xFFDFFFFF;
                FADT->Header.Checksum = ChsumTbl((UINT8*)FADT, FADT->Header.Length);
                break;
            }
        }
    } 
    
    return EFI_SUCCESS;
}

EFI_STATUS ConvertGlobalVar()
{
    EFI_STATUS  Status = EFI_SUCCESS;

    Status = pRS->ConvertPointer( 0, (VOID **) &PPIVarAddr);

    return Status;
}

EFI_STATUS GetTCGAslUpdateMem(
        UINT8 **pRetriveAddr
    )
{
    EFI_STATUS  Status;
    UINT64      pPPIVarAddr;
    UINTN       unPPIVarSize = sizeof(pPPIVarAddr);
    CHAR16      TcgPPIVarAddr[] = L"TcgPPIVarAddr";
    EFI_GUID    Guid = EFI_GLOBAL_VARIABLE;

    if( 0 != PPIVarAddr )
    {
        *pRetriveAddr = (UINT8*)(UINTN)PPIVarAddr;
        return EFI_SUCCESS;
    }

    Status = pRS->GetVariable(
                    TcgPPIVarAddr, 
                    &Guid, 
                    NULL, 
                    &unPPIVarSize, 
                    &pPPIVarAddr);
    ASSERT_EFI_ERROR(Status);
    _TRACE((-1, "GetTCGAslUpdateMem Addr[0x%x]\n", (UINTN)pPPIVarAddr ));
    
    PPIVarAddr = pPPIVarAddr;
    *pRetriveAddr = (UINT8*)(UINTN)pPPIVarAddr;

    return Status;
}

EFI_STATUS TcgUpdateAslNameOpReg(
    IN PACPI_HDR PDsdt,
    IN UINT8     *ObjName,
    IN UINT64    Value,
    IN UINT16    Size )
{
    EFI_STATUS   Status;
    ASL_OBJ_INFO obj;
    UINT32       Length;
    UINT8        *ptr;


    Length = PDsdt->Length - sizeof(ACPI_HDR);
    ptr    = (UINT8*)PDsdt + sizeof(ACPI_HDR);

    Status = GetAslObj( ptr, Length, ObjName, otOpReg, &obj );
    _TRACE((-1, "TcgGetAslObj[%r]\n", Status ));
    if( EFI_ERROR(Status) )
        return Status;

    ptr = (UINT8*)obj.DataStart;

    *(UINT32*)(ptr + 2) = (UINT32)Value;      //+1 for RegionSpace, +1 for prefix.
    *(UINT16*)(ptr + 2 + 5) = Size;

//    PDsdt->Checksum = 0;
//	PDsdt->Checksum = ChsumTbl((UINT8*)PDsdt, PDsdt->Length);

    return EFI_SUCCESS;
}

EFI_STATUS UpdateTCGAslVarMem(
    IN EFI_EVENT ev,
    IN VOID      *ctx )
{
    UINT64      pPPIVarAddr = 0;
    UINTN       unPPIVarSize;
    UINTN       unSize;
    CHAR16      TcgPPIVarAddr[] = L"TcgPPIVarAddr";
    EFI_GUID    Guid = EFI_GLOBAL_VARIABLE;
    EFI_STATUS  Status;
    EFI_ACPI_SUPPORT_PROTOCOL  *mTcgAcpiSupport;
    static BOOLEAN flagTcgAslVarMemUpdate = FALSE;

    EFI_GUID FileTcgPpiGuid = {
            0xb3d1dda5,0xd0ef,0x4e18,0x94,0x8c,0x40,0x98,0x27,0xb1,0x03,0x8c};
    ACPI_HDR    *FileTcgPpiAml;
    UINTN       FileTcgPpiAmlLength;
    UINTN       Handle = 0;
    
    if( TRUE == flagTcgAslVarMemUpdate )
        return EFI_SUCCESS;

    _TRACE(( -1, "SynTCGAslVarLib.c[%d]: Enter UpdateTCGAslVarMem(...)\n", __LINE__));

    Status = pBS->LocateProtocol( &gTcgAcpiSupportGuid, NULL, &mTcgAcpiSupport );

    if ( EFI_ERROR( Status ))
    {
        TRACE((TRACE_ALWAYS, "SynTCGAslVraLib.c:Unable to locate AcpiSupport\n"));
        return Status;
    }

    Status = LoadFile(&FileTcgPpiGuid, &FileTcgPpiAml, &FileTcgPpiAmlLength);
    ASSERT_EFI_ERROR (Status);

    ASSERT (((EFI_ACPI_DESCRIPTION_HEADER*)FileTcgPpiAml)->OemTableId == SIGNATURE_64 ('_', 'S', 'y', 'n', 'T', 'C', 'G', '_'));
    if ( EFI_ERROR(Status) )
    {
        TRACE((TRACE_ALWAYS, "SynTCGAslVraLib.c: Load ACPI Table Failed\n"));
        return Status;
    }

    _TRACE((-1, "FileTcgPpiAml->Signature:%x\n",FileTcgPpiAml->Signature));

    unPPIVarSize = 0x30;   
//    pPPIVarAddr = PPIVarAddr;                     
    Status = pBS->AllocatePool(EfiRuntimeServicesData, unPPIVarSize, (VOID**)&pPPIVarAddr);
//    Status = pBS->AllocatePages(AllocateMaxAddress, EfiACPIMemoryNVS, EFI_SIZE_TO_PAGES (unPPIVarSize), &pPPIVarAddr);
    if( EFI_ERROR(Status) )
    {
        TRACE(( -1, "AllocatePages(AllocateMaxAddress...)[%r]\n", Status));
    }
    ASSERT_EFI_ERROR(Status);
    _TRACE((-1, "Locate PPIVarAddr[%x]\n", (UINTN)pPPIVarAddr ));

    pBS->SetMem((VOID*)(UINTN)pPPIVarAddr, unPPIVarSize, PPI_INVAILD_SIG);    

    unSize = sizeof(pPPIVarAddr);
    Status = pRS->SetVariable(
                TcgPPIVarAddr,
                &Guid,
                EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                unSize,
                &pPPIVarAddr);
    ASSERT_EFI_ERROR(Status);
    
    Status = TcgUpdateAslNameOpReg( FileTcgPpiAml, "PVAR", pPPIVarAddr, (UINT16)unPPIVarSize);
    ASSERT_EFI_ERROR(Status);

    Status = mTcgAcpiSupport->SetAcpiTable(
      	mTcgAcpiSupport,
      	FileTcgPpiAml,
      	TRUE,
      	EFI_ACPI_TABLE_VERSION_ALL,
        &Handle);
    _TRACE((-1, "SyncTCGAslVarLib.c: SetAcpiTable[%r]\n", Status ));
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status)) return Status;

    Status = mTcgAcpiSupport->PublishTables (
                            mTcgAcpiSupport,
                            EFI_ACPI_TABLE_VERSION_ALL
                            );
    _TRACE((-1, "SyncTCGAslVarLib.c: PublishTables[%r]\n", Status ));
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR(Status)) return Status;

    flagTcgAslVarMemUpdate = TRUE;

    _TRACE(( -1, "UpdateTCGAslVarMem[Success]\n" ));

    return Status;
}

EFI_STATUS UpdateTcgASLMemAddr(
    VOID )
{

    EFI_EVENT                  ev;
    static VOID                *reg;
    EFI_STATUS                 Status;
    VOID                       *ctx;
    EFI_ACPI_SUPPORT_PROTOCOL  *mTcgAcpiSupport;

    Status = pBS->LocateProtocol( &gTcgAcpiSupportGuid, NULL, &mTcgAcpiSupport );

    if ( EFI_ERROR( Status ))
    {
        Status = pBS->CreateEvent( EFI_EVENT_NOTIFY_SIGNAL,
                                   EFI_TPL_DRIVER, UpdateTCGAslVarMem, &reg, &ev );
        ASSERT( !EFI_ERROR( Status ));
        Status = pBS->RegisterProtocolNotify( &gTcgAcpiSupportGuid, ev, &reg );
        ASSERT( !EFI_ERROR( Status ));
        return Status;
    }
    ev  = NULL;
    ctx = NULL;

    Status = UpdateTCGAslVarMem( ev, ctx );

    return Status;
}

EFI_STATUS UpdateTCGAslVarField(
    IN EFI_EVENT ev,
    IN VOID      *ctx )
{
    EFI_STATUS  Status = EFI_SUCCESS;

    SyncTCGAsl_ExitBIOS();
    
    UpdateCSBit();
    
    pBS->CloseEvent(ev);

    return Status;
}

EFI_STATUS UpdateTcgASLFieldData(
    VOID )
{
    EFI_STATUS  Status = EFI_SUCCESS;
    EFI_EVENT   Event;

    Status = CreateReadyToBootEvent(
                     TPL_CALLBACK,
                     UpdateTCGAslVarField,
                     NULL,
                     &Event);
    ASSERT_EFI_ERROR(Status);

    return Status;
}

EFI_STATUS Update_UserConfirmSts_to_ASL(
    VOID )
{
    UINTN                       Size = sizeof(PERSISTENT_BIOS_TPM_FLAGS);
    UINTN                       BiosSize = sizeof(AMI_PPI_NV_VAR);
    AMI_PPI_NV_VAR              Temp;
    PERSISTENT_BIOS_TPM_FLAGS   TpmNvFlags;
    UINT8                       Read_value = 0;
    EFI_STATUS                  Status;
    UINT8                       *pPPIAslAddr = NULL;
    
    Status = GetTCGAslUpdateMem(&pPPIAslAddr);
    if( EFI_ERROR(Status) )
    {
        return Status;
    }

    // Now pPPIAslAddr is the Start Memory of the PPI Confirm Memroy Offset
    pPPIAslAddr += PPI_Confirm_Ofst;

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
    ASSERT_EFI_ERROR(Status);
    if( EFI_ERROR(Status) )
        return Status;

//    if(Read_value >= 0  && Read_value < 23)
    for( Read_value = 0; Read_value < 23; ++Read_value )
    {
        if(Read_value == 0 )
        {
//           WritebyteSmiPort( TCGSMIDATAPORT, 0x4 );
           pPPIAslAddr[Read_value] = 0x04;
           continue;
//           return; 
        }

        if( Read_value == TCPA_PPIOP_UNOWNEDFIELDUPGRADE
            || Read_value == TCPA_PPIOP_SETOPAUTH || Read_value == TCPA_PPIOP_SETNOPPICLEAR_FALSE
            || Read_value == TCPA_PPIOP_SETNOPPICLEAR_TRUE ||  Read_value == TCPA_PPIOP_SETNOPPIMAINTENANCE_FALSE
            || Read_value == TCPA_PPIOP_SETNOPPIMAINTENANCE_TRUE  || Read_value > TCPA_PPIOP_ENABLE_ACTV_CLEAR_ENABLE_ACTV)
        {
//            WritebyteSmiPort( TCGSMIDATAPORT, 0x0 );
            pPPIAslAddr[Read_value] = 0x0;
//            return;
        }else if(Read_value == TCPA_PPIOP_CLEAR || Read_value == TCPA_PPIOP_ENABLE_ACTV_CLEAR )
        {
            if(TpmNvFlags.NoPpiClear  == TRUE){
//                WritebyteSmiPort( TCGSMIDATAPORT, 0x4 );
                pPPIAslAddr[Read_value] = 0x04;
            }else{
//                WritebyteSmiPort( TCGSMIDATAPORT, 0x3 );
                pPPIAslAddr[Read_value] = 0x03;
            }
//            return;
        }else if(Read_value == TCPA_PPIOP_CLEAR_ENACT || Read_value == TCPA_PPIOP_ENABLE_ACTV_CLEAR_ENABLE_ACTV)
        {
            if(TpmNvFlags.NoPpiClear  == TRUE  && TpmNvFlags.NoPpiProvision == TRUE ){
//                WritebyteSmiPort( TCGSMIDATAPORT, 0x4 );
                pPPIAslAddr[Read_value] = 0x04;
            }else{
//                WritebyteSmiPort( TCGSMIDATAPORT, 0x3 );
                pPPIAslAddr[Read_value] = 0x03;
            }
//            return;
        }else if(Read_value == TCPA_PPIOP_SETNOPPIPROVISION_FALSE || Read_value == TCPA_PPIOP_SETNOPPIPROVISION_TRUE)
        {
            if(Read_value == TCPA_PPIOP_SETNOPPIPROVISION_TRUE ){
//                WritebyteSmiPort( TCGSMIDATAPORT, 0x3 );
                pPPIAslAddr[Read_value] = 0x03;
            }else{
//                WritebyteSmiPort( TCGSMIDATAPORT, 0x4 );
                pPPIAslAddr[Read_value] = 0x04;
            }
//            return;
        }
        else if(TpmNvFlags.NoPpiProvision == TRUE)
        {
//                WritebyteSmiPort( TCGSMIDATAPORT, 0x4 );
                pPPIAslAddr[Read_value] = 0x04;
        }else
        {
//                WritebyteSmiPort( TCGSMIDATAPORT, 0x3 );
                pPPIAslAddr[Read_value] = 0x03;
        } 
    }
//    else{
//                WritebyteSmiPort( TCGSMIDATAPORT, 0x0 );
//    }
    return EFI_SUCCESS; 
}

EFI_STATUS Update_PPIOP_Rst_to_ASL(
    VOID )
{
    UINTN          Size = sizeof(AMI_PPI_NV_VAR);
    AMI_PPI_NV_VAR Temp;
    UINT8          Read_value = 0;
    EFI_STATUS     Status;
    UINT8          *pPPIAslAddr = NULL;
    
    Status = GetTCGAslUpdateMem(&pPPIAslAddr);
    if( EFI_ERROR(Status) )
    {
        return Status;
    }

//    Read_value = ReadSmiPort( TCGSMIDATAPORT );

    Status = pRS->GetVariable( L"AMITCGPPIVAR", \
                                   &SmmtcgefiOsVariableGuid, \
                                   NULL, \
                                   &Size, \
                                   &Temp );

    if(Status){
//        WritebyteSmiPort( TCGSMIDATAPORT, 0xFF );
        return Status;
    }
    
//    switch (Read_value & TYPE_MASK ){
//       case RQSTVAR:    // Get Pending Operation
//            WritebyteSmiPort( TCGSMIDATAPORT, Temp.RQST );
            pPPIAslAddr[PPI_PendOP_Ofst] = Temp.RQST;
//            break;
//       case RCNTVAR:    // Get Most Recent Request
//            WritebyteSmiPort( TCGSMIDATAPORT, Temp.RCNT ); 
            pPPIAslAddr[PPI_CurOP_Ofst] = Temp.RCNT;
//            break;
//       case ERRORVAR:   // Get Most Recent Request Status
//             WritebyteSmiPort( TCGSMIDATAPORT, Temp.ERROR );
            if( 0xFFF0 == Temp.ERROR)
                *(UINT32*)(pPPIAslAddr+PPI_OPRst_Ofst) = 0xFFFFFFF0;
            else if( 0xFFF1 == Temp.ERROR)
                *(UINT32*)(pPPIAslAddr+PPI_OPRst_Ofst) = 0xFFFFFFF1;
            else
                *(UINT32*)(pPPIAslAddr+PPI_OPRst_Ofst) = (UINT32)Temp.ERROR;

//            break;
//       default:
//            WritebyteSmiPort( TCGSMIDATAPORT, 0xFF );
//            break;
//    }

    return EFI_SUCCESS;
}

EFI_STATUS SyncTCGAsl_ExitBIOS(
    VOID )
{
    EFI_STATUS  Status;
    EFI_STATUS  RetStatus = EFI_SUCCESS;
    static BOOLEAN     bFlagSyncPPIRsttoAsl = FALSE;

    _TRACE((-1, "Enter SyncTCGAsl_ExitBIOS[Update the PPI Result to ASL Field]\n" ));

    if( TRUE == bFlagSyncPPIRsttoAsl )
        return EFI_SUCCESS;

    // Update the PPIOP Result to ASL Mem Field.
    Status = Update_PPIOP_Rst_to_ASL();
    ASSERT_EFI_ERROR(Status);
    if( EFI_ERROR(Status) )
    {
        TRACE(( -1, "[Error] Update PPIOP Result to ASL mem failed\n" ));
        RetStatus = Status;
    } 

    // Update the PPIOP User Comfirm Status Result to Mem Field.
    Status = Update_UserConfirmSts_to_ASL();
    ASSERT_EFI_ERROR(Status);
    if( EFI_ERROR(Status) )
    {
        TRACE(( -1, "[Error] Update PPIOP User Comfirm Status Result to ASL Mem Failed\n" ));
        RetStatus = Status;
    } 

    if( !EFI_ERROR(RetStatus) )
        bFlagSyncPPIRsttoAsl = TRUE;

    return RetStatus;
}

EFI_STATUS LoadFile(
	EFI_GUID		*Guid,
	VOID			**Buffer,
	UINTN			*BufferSize
)
{
    EFI_STATUS                      Status;
    UINTN                           NumHandles;
    EFI_HANDLE                      *HandleBuffer;
    EFI_FIRMWARE_VOLUME2_PROTOCOL   *Fv;
    UINT32                          AuthenticationStatus;
    UINTN                           Index;

    *Buffer = 0;
    *BufferSize = 0;

    Status = pBS->LocateHandleBuffer(ByProtocol,&gEfiFirmwareVolume2ProtocolGuid,NULL,&NumHandles,&HandleBuffer);
    if (EFI_ERROR(Status)) {
        TRACE((-1, "Can not locate gEfiFVProtocolGuid[%r]\n", Status));
        return Status;
    }

    for (Index = 0; Index < NumHandles; ++Index) {
        Status = pBS->HandleProtocol(HandleBuffer[Index],&guidFV,&Fv);
        if (EFI_ERROR(Status)) {
            continue;
        }
        Status = Fv->ReadSection(Fv,Guid,EFI_SECTION_RAW,0,Buffer,BufferSize,&AuthenticationStatus);
        if (Status == EFI_SUCCESS) {
            _TRACE(( -1, "Find the SynTCG ASL Table\n"));
            break;
        }
    }

    if( EFI_ERROR(Status) )
    {
        TRACE((-1, "SynTCGAslVar.c[%d]: Did not find the SynTcg ASL Table[%r]\n", __LINE__, Status));
    }
    
    pBS->FreePool(HandleBuffer);
    return Status;
}

//====================================
// Below Code is the Runtime Code ....
//====================================

EFI_STATUS SyncTCGAsl_ResetSystem(
    VOID )
{
    EFI_STATUS  Status;
    EFI_STATUS  RetStatus = EFI_SUCCESS;

    _TRACE((-1, "Enter SyncTCGAsl_ResetSystem(...)\n"));
//#if defined(USING_SMI_DBG) && USING_SMI_DBG == 1
//    if( IoRead8(0xB3) != 0xFF )
//    {
//        _TRACE((-1, "\tASL Dbg Val[%02x]\n", IoRead8(0xB3)));
//        return EFI_SUCCESS;
//    }
//#endif

    // Store if the MOR Request is Set
    Status = Store_MOR_request();
    ASSERT_EFI_ERROR(Status);
    if( EFI_ERROR(Status) )
    {
        RetStatus = Status;
    } 

    // Store if there will have PPIOP Request
    Status = Store_PPI_request();
    ASSERT_EFI_ERROR(Status);
    if( EFI_ERROR(Status) )
    {
        RetStatus = Status;
    } 

    return RetStatus;
}

EFI_STATUS Store_MOR_request(
    VOID )
{
    UINT8           mor  = 0;
    UINTN           Size = sizeof(mor);
    EFI_STATUS      Status;
    CHAR16          UefiMor[]   = L"MemoryOverwriteRequestControl";
    EFI_GUID        MorUefiGuid = MEMORY_ONLY_RESET_CONTROL_GUID;
    UINT8           Read_value  = 0;
    UINTN           size = sizeof(AMI_PPI_NV_VAR);
    AMI_PPI_NV_VAR  Temp;
    UINT8          *pPPIAslAddr = NULL;
    
    Status = GetTCGAslUpdateMem(&pPPIAslAddr);
    if( EFI_ERROR(Status) )
    {
        return Status;
    }

    if( PPI_INVAILD_SIG == pPPIAslAddr[PPI_MOR_Ofst] )
    {
        // Do not need to Set the MOR PPI Request.
        return EFI_SUCCESS;
    }

//    Read_value = ReadSmiPort( TCGSMIDATAPORT );
    Read_value = pPPIAslAddr[PPI_MOR_Ofst];

    Status = pRS->SetVariable( UefiMor, \
                               &MorUefiGuid, \
                               EFI_VARIABLE_NON_VOLATILE   \
                               | EFI_VARIABLE_RUNTIME_ACCESS   \
                               | EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                               Size, \
                               &Read_value );
    ASSERT_EFI_ERROR(Status);
    if(Status){
//        WritebyteSmiPort( TCGSMIDATAPORT, 0xFF );
        return Status;
    }

    Status = pRS->GetVariable( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               NULL, \
                               &size, \
                               &Temp );
    ASSERT_EFI_ERROR(Status);
    if(Status){
//        WritebyteSmiPort( TCGSMIDATAPORT, 0xFF );
        return Status;
    }

    Temp.Flag  = 0;

    Status = pRS->SetVariable( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               EFI_VARIABLE_NON_VOLATILE   \
                               | EFI_VARIABLE_RUNTIME_ACCESS   \
                               | EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                               size, \
                               &Temp );   
    ASSERT_EFI_ERROR(Status);
    if(Status){
//        WritebyteSmiPort( TCGSMIDATAPORT, 0xFF );
        return Status;
    }

    return EFI_SUCCESS;
}

EFI_STATUS Store_PPI_request(
    VOID )
{
    UINTN          Size = sizeof(AMI_PPI_NV_VAR);
    AMI_PPI_NV_VAR Temp;
    EFI_STATUS     Status;
    UINT8          Read_value = 0;
    UINT8          *pPPIAslAddr = NULL;
    
    Status = GetTCGAslUpdateMem(&pPPIAslAddr);
    if( EFI_ERROR(Status) )
    {
        return Status;
    }

//    Read_value = ReadSmiPort( TCGSMIDATAPORT );
    Read_value = pPPIAslAddr[PPI_Submit_Ofst];

    if( Read_value == TCPA_PPIOP_UNOWNEDFIELDUPGRADE
            || Read_value == TCPA_PPIOP_SETOPAUTH || Read_value == TCPA_PPIOP_SETNOPPICLEAR_FALSE
            || Read_value == TCPA_PPIOP_SETNOPPICLEAR_TRUE ||  Read_value == TCPA_PPIOP_SETNOPPIMAINTENANCE_FALSE
            || Read_value == TCPA_PPIOP_SETNOPPIMAINTENANCE_TRUE  || Read_value > TCPA_PPIOP_ENABLE_ACTV_CLEAR_ENABLE_ACTV)
    {
//      WritebyteSmiPort( TCGSMIDATAPORT, 0xF1 );
        _TRACE(( -1, "PPI Request Field not be Identify 1.\n")); 
      return EFI_SUCCESS;
    }

    if(Read_value >= 0 && Read_value < 23)
    {
        Temp.RQST  = Read_value;
        Temp.RCNT  = Read_value;
        Temp.ERROR = 0;
        Temp.Flag  = 0;

        Status = pRS->SetVariable( L"AMITCGPPIVAR", \
                               &SmmtcgefiOsVariableGuid, \
                               EFI_VARIABLE_NON_VOLATILE   \
                               | EFI_VARIABLE_RUNTIME_ACCESS   \
                               | EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                               Size, \
                               &Temp );
        ASSERT_EFI_ERROR(Status);
        if(Status){
//            WritebyteSmiPort( TCGSMIDATAPORT, 0xFF );
            return Status;
        }
    }else{
//        WritebyteSmiPort( TCGSMIDATAPORT, 0xF1 );
        _TRACE(( -1, "PPI Request Field not be Identify 2.\n")); 
        return EFI_UNSUPPORTED;
    }

    return EFI_SUCCESS;
}
