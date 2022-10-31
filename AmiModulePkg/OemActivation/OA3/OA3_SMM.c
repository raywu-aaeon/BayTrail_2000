//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2012, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**             5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093          **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************

//*****************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//*****************************************************************************
//<AMI_FHDR_START>
//
// Name: OA3_SMM.c
//
// Description: This part of code allows to update ACPI in the OS Runtime through 
//              the SW SMI for OEM Activation without rebooting the system.
//
//<AMI_FHDR_END>
//*****************************************************************************


#include <AmiDxeLib.h>
#include <AmiSmm.h>
#include "OA3.h"
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/SmmControl2.h>
#include <Protocol/SmmCpu.h>
#include <AmiHobs.h>


//---------------------------------------------------------------------------

static EFI_PHYSICAL_ADDRESS gTsegAddress_ = 0;
static UINT32 gTsegSize_ = 0;

//---------------------------------------------------------------------------


//<AMI_PHDR_START>
//--------------------------------------------------------------------------
//
// Procedure:   GetTsegVar
// 
// Description: This function stores TSEG address and size. 
//
// Input:   VOID
//
// Output:  VOID
//
//--------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID GetTsegVar(VOID)
{
    EFI_STATUS Status;
    EFI_GUID CpuInfoHobGuid = AMI_CPUINFO_HOB_GUID;
    EFI_GUID HobListGuid = HOB_LIST_GUID;
    CPUINFO_HOB *CpuInfoHob = (CPUINFO_HOB*)GetEfiConfigurationTable(pST,&HobListGuid);
    
    if (CpuInfoHob == NULL) 
        return;

    Status = FindNextHobByGuid(&CpuInfoHobGuid,&CpuInfoHob); 
    if (EFI_ERROR(Status))
        return;

    gTsegAddress_ = CpuInfoHob->TsegAddress;
    gTsegSize_ = CpuInfoHob->TsegSize;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: CheckAddressRange
//
// Description: Check address range to avoid TSEG area.
//
// Input: 
//  Address - starting address
//  Range   - length of the area
//
// Output: 
//  EFI_SUCCESS             - Access granted
//  EFI_SECURITY_VIOLATION  - Access denied!
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS CheckAddressRange( IN UINT8 *Address, IN UINTN Range )
{
    // if TSEG variables are not initialized then do not validate the access.
    if ( gTsegAddress_ == 0 || gTsegSize_ == 0 )
        return EFI_SUCCESS;

    if ( ((EFI_PHYSICAL_ADDRESS)Address >= gTsegAddress_) && 
         ((EFI_PHYSICAL_ADDRESS)Address <= (gTsegAddress_ + gTsegSize_)) )
        return EFI_SECURITY_VIOLATION;

    if ( (((EFI_PHYSICAL_ADDRESS)Address + Range) >= gTsegAddress_) && 
         (((EFI_PHYSICAL_ADDRESS)Address + Range) <= (gTsegAddress_ + gTsegSize_)) )
        return EFI_SECURITY_VIOLATION;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name: ChsumTbl
//
// Description: This function calculates a checksum of table starting at 
//  TblStart of length BytesCount and returns the checksum value.
//
// Input:   
//  *TblStart   - Starting address of the memory area to checksum.
//  BytesCount  - Length in bytes of the memory area to checksum.
//
// Output:
//  Checksum value starting from TblStart and ending at TblStart + BytesCount.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 ChsumTbl( IN UINT8* TblStart, IN UINT32 BytesCount )
{
    UINT8  Result = *TblStart;
    UINTN  i;
    
    for ( i = 1; i < BytesCount; i++ )  
        Result += TblStart[i];
    return 0 - Result;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: Oa3UpdateAcpiTable
//
// Description: This function will update ACPI table for OA3.
//
// Input: ProductKey - pointer to the Product Key
//
// Output: EFI_STATUS
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS Oa3UpdateAcpiTable( IN UINT8 *ProductKey )
{
    EFI_STATUS Status;
    EFI_OA3_MSDM_STRUCTURE MsdmVariable = {0};
    EFI_GUID AmiGlobalVariableGuid = AMI_GLOBAL_VARIABLE_GUID;
    UINTN  Size = sizeof(EFI_OA3_MSDM_STRUCTURE);
    EFI_ACPI_MSDM_TABLE *MsdmTable;
    ACPI_HDR *AcpiHdr;
    UINT8 *Ptr;
    EFI_PHYSICAL_ADDRESS *AcpiPtr;

    if (ProductKey == NULL)
        return EFI_INVALID_PARAMETER;

    Status = CheckAddressRange(ProductKey, sizeof(MsdmTable->Data));
    if (EFI_ERROR(Status))
        return Status;
    
    Status = pRS->GetVariable(
        EFI_OA3_MSDM_VARIABLE,
        &AmiGlobalVariableGuid,
        NULL,
        &Size,
        &MsdmVariable
    );
    TRACE((TRACE_ALWAYS,"Oa3UpdateAcpiTable: XsdtAddr=%X, MsdmAddr=%X\n",MsdmVariable.XsdtAddress,MsdmVariable.MsdmAddress));
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status))
        return Status;

    if ( MsdmVariable.XsdtAddress == 0 || MsdmVariable.MsdmAddress == 0 )
        return EFI_INVALID_PARAMETER;

    // Validate MSDM table signature 
    MsdmTable = (EFI_ACPI_MSDM_TABLE*)(UINTN)MsdmVariable.MsdmAddress;
    if (MsdmTable->Header.Signature != MSDM_SIG)
        return EFI_INVALID_PARAMETER;
    
    // Validate XSDT table signature 
    AcpiHdr = (ACPI_HDR*)(UINTN)MsdmVariable.XsdtAddress;
    if (AcpiHdr->Signature != XSDT_SIG)
        return EFI_INVALID_PARAMETER;

    // Copy a new Product Key to MSDM Data array
    MemCpy( 
        (UINT8*)&MsdmTable->Version,
#if OEM_ACTIVATION_TABLE_LOCATION == 0
        ProductKey,
#else
        (UINT8*)OEM_ACTIVATION_TABLE_ADDRESS,   // Copy Key from the NCB
#endif
        sizeof(EFI_ACPI_MSDM_TABLE) - sizeof(ACPI_HDR) );

    // Updates MSDM table checksum
    MsdmTable->Header.Checksum = 0;
    MsdmTable->Header.Checksum = ChsumTbl((UINT8*)MsdmTable, sizeof(EFI_ACPI_MSDM_TABLE));

    // Check if pointer to MSDM ACPI table has been set already
    Ptr = (UINT8*)AcpiHdr + AcpiHdr->Length - sizeof(EFI_PHYSICAL_ADDRESS);
    AcpiPtr = (EFI_PHYSICAL_ADDRESS*)Ptr;
    if ( *AcpiPtr != (EFI_PHYSICAL_ADDRESS)MsdmTable ) {

        // Validate ACPI header's length. Check a range to avoid SMM
        Ptr = (UINT8*)AcpiHdr + AcpiHdr->Length;
        Status = CheckAddressRange(Ptr, sizeof(EFI_PHYSICAL_ADDRESS));
        if (EFI_ERROR(Status))
            return Status;
        
        // Check destination if it's cleared by zero 
        AcpiPtr = (EFI_PHYSICAL_ADDRESS*)Ptr;
        if (*AcpiPtr != 0)
            return EFI_SECURITY_VIOLATION;
        
        // Add address of MSDM table as the last pointer in XSDT
        *AcpiPtr = (EFI_PHYSICAL_ADDRESS)MsdmTable;

        // Updates Length filed of the XSDT table (increments by 8)
        AcpiHdr->Length += sizeof(EFI_PHYSICAL_ADDRESS);

        // Updates XSDT checksum
        AcpiHdr->Checksum = 0; 
        AcpiHdr->Checksum = ChsumTbl((UINT8*)AcpiHdr, AcpiHdr->Length);
    }

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: SwSmiOa3Function
//
// Description:
// Calling the SMI Interface
// The caller will write AL (the value 0xee) to the SMI Command Port as 
// defined in the ACPI FADT.
// The SMI handler will update the callers' buffer(s) and return.
//
// Input: 
//  DispatchHandle
//  DispatchContext
//
// Output: VOID
//
// Notes:
// The function will clear the carry bit if it is successful (CF = 0). 
// If the function is unsuccessful, it will set the carry bit and set the 
// error code in the AH register as indicated by the error table below.
// The function returns the following data in the provided parameter block. 
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SwSmiOa3Function( 
    IN EFI_HANDLE DispatchHandle,
    IN CONST VOID  *Context OPTIONAL,
    IN OUT   VOID  *CommBuffer OPTIONAL,
    IN OUT   UINTN *CommBufferSize OPTIONAL )
{
    EFI_STATUS Status;
    EFI_SMM_SW_CONTEXT *SmmSwContext = ((EFI_SMM_SW_CONTEXT*)CommBuffer); 
    EFI_GUID EfiSmmCpuProtocolGuid = EFI_SMM_CPU_PROTOCOL_GUID;
    EFI_SMM_CPU_PROTOCOL *SmmCpuProtocol = NULL;
    EFI_PHYSICAL_ADDRESS Register = 0;

    Status = pSmst->SmmLocateProtocol(
        &EfiSmmCpuProtocolGuid,
        NULL, 
        &SmmCpuProtocol 
    );
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status))
        return Status;

    // Read the RBX register
    Status = SmmCpuProtocol->ReadSaveState( 
        SmmCpuProtocol,
        sizeof(UINT32), 
        EFI_SMM_SAVE_STATE_REGISTER_RBX,
        SmmSwContext->SwSmiCpuIndex,
        &Register
    );
    if (!EFI_ERROR(Status))
        Oa3UpdateAcpiTable((UINT8*)Register); // Update the ACPI table

    return Status;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: InSmmFunction
//
// Description: Registration of the SMI function
//
// Input: 
//  ImageHandle - Image handle
//  SystemTable - Pointer to the system table
//
// Output: EFI_STATUS
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS InSmmFunction(
    IN EFI_HANDLE ImageHandle, 
    IN EFI_SYSTEM_TABLE *SystemTable )
{
    EFI_STATUS Status;
    EFI_HANDLE Handle = 0;
    EFI_GUID EfiSmmSwDispatchProtocolGuid = EFI_SMM_SW_DISPATCH2_PROTOCOL_GUID;
    EFI_SMM_SW_DISPATCH2_PROTOCOL *SwDispatch = NULL;
    EFI_SMM_SW_REGISTER_CONTEXT   SwContext;

    Status = pSmst->SmmLocateProtocol(
        &EfiSmmSwDispatchProtocolGuid, 
        NULL, 
        &SwDispatch 
    );
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status))
        return Status;

    SwContext.SwSmiInputValue = SW_SMI_OA3_FUNCTION_NUMBER;
    Status = SwDispatch->Register( 
        SwDispatch, 
        SwSmiOa3Function,
        &SwContext, 
        &Handle 
    );
    ASSERT_EFI_ERROR(Status);
    return Status;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   OA3_SMM_EntryPoint
//
// Description: This function is the entry point of the module.
//
// Input:       
//  ImageHandle  - Image handle
//  *SystemTable - Pointer to the system table
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI OA3_SMM_EntryPoint(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable )
{
    EFI_STATUS Status;
    EFI_OA3_MSDM_STRUCTURE MsdmVariable;
    EFI_GUID AmiGlobalVariableGuid = AMI_GLOBAL_VARIABLE_GUID;
    UINTN  Size = sizeof(EFI_OA3_MSDM_STRUCTURE);
    
    InitAmiLib(ImageHandle, SystemTable);

    // Get TSEG address and TSEG size
    GetTsegVar();

    // Check if OA3 MSDM variable does exist
    Status = pRS->GetVariable(
        EFI_OA3_MSDM_VARIABLE,
        &AmiGlobalVariableGuid,
        NULL,
        &Size,
        &MsdmVariable
    );
    if (!EFI_ERROR(Status)) {

        // then install the SMI handler
        Status = InitSmmHandler( 
            ImageHandle, 
            SystemTable, 
            InSmmFunction, 
            NULL 
        );
        TRACE((TRACE_ALWAYS,"OEM Activation: InitSmmHandler Status=%r\n",Status));
        ASSERT_EFI_ERROR(Status);
    }

    return Status;
}


//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2012, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**             5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093          **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
