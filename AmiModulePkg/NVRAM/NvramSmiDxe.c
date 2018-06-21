//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**             5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093          **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************

//****************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//****************************************************************************
//<AMI_FHDR_START>
//---------------------------------------------------------------------------
//
// Name: SmiVariable.c
//
// Description: Interface to a subset of EFI Framework protocols using 
// legacy interfaces that will allow external software to access EFI 
// protocols in a legacy environment.
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>


#include <AmiDxeLib.h>
#include <AmiCspLib.h>
#include <Protocol/SmmBase.h>
#include "NvramSmi.h"
#include <Protocol/SmmCommunication.h>

EFI_RUNTIME_SERVICES *SmmpRs = NULL;
EFI_GUID SmmRtServTableGuid = EFI_SMM_RUNTIME_SERVICES_TABLE_GUID; // in dec ->SmmRsTableGuid
BOOLEAN ExitBS = FALSE;

EFI_SMM_COMMUNICATION_PROTOCOL *SmmCommProtocol = NULL;

typedef struct
{
    BOOLEAN Busy;
    UINT8   IntState[2];
} NVRAM_SMI_CRITICAL_SECTION;

//----------------------------------------------------------------------------
// Local Variables

EFI_GUID    gNvramSmiGuid = NVRAM_SMI_GUID;

VOID        *gNvramBuffer = NULL;

NVRAM_SMI_CRITICAL_SECTION NvramSmiCs = {FALSE, {0, 0}};


EFI_SMM_COMMUNICATE_HEADER CommHeader = {NVRAM_SMI_GUID, 1 , {0}};
VOID *CommHeaderPtr;
//EIP151328 >>
#include "Nvram.h"
EFI_SET_VARIABLE gSavedSetVariable;
//EIP151328 >><<

//----------------------------------------------------------------------------
// Function Definitions

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   LocateCommunicateProt
//
//  Description: This function will be called to locate Smm Communication Protocol
//
//  Input:  IN EFI_EVENT Event - signalled event
//          IN VOID *Context - calling context
//
//  Output: EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS LocateCommunicateProt (
            IN EFI_EVENT    Event,
            IN VOID         *Context
)
{
 //   TRACE((-1, "LocateCommunicateProt.\n"));
    if (SmmCommProtocol != NULL)
        return EFI_SUCCESS;
//    TRACE((-1, "LocateCommunicateProt1.\n"));
    return (pBS->LocateProtocol ( &gEfiSmmCommunicationProtocolGuid,
                                   NULL,
                                   &SmmCommProtocol ));

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   BeginCriticalSection
//
// Description: This function calls when critical section begins. It disables interupts,
//              and Smi and fills CRITICAL_SECTION structure fields
//
// Input:       CRITICAL_SECTION *Cs - pointer to CRITICAL_SECTION structure
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
NvSmiBeginCriticalSection (
    NVRAM_SMI_CRITICAL_SECTION      *Cs
)
{
    if (Cs->Busy) return EFI_ACCESS_DENIED;
    Cs->IntState[0] = IoRead8(0x21);
    Cs->IntState[1] = IoRead8(0xa1);
    IoWrite8 (0x21, 0xff);
    IoWrite8 (0xa1, 0xff);
    Cs->Busy = TRUE;
    return EFI_SUCCESS;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   EndCriticalSection
//
// Description: This function calls when critical section ends. It enable interupts,
//              and Smi and fills CRITICAL_SECTION structure fields
//
// Input:       CRITICAL_SECTION *Cs - pointer to CRITICAL_SECTION structure
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
NvSmiEndCriticalSection (
    NVRAM_SMI_CRITICAL_SECTION      *Cs
)
{
    Cs->Busy = FALSE;
    IoWrite8 (0x21, Cs->IntState[0]);
    IoWrite8 (0xa1, Cs->IntState[1]);
    return EFI_SUCCESS;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   EndCriticalSection
//
// Description: TThis function initiates communication between VAR services outside
//      of SMM and SMI handlers inside of SMM. With the help of Smm Communication
//      Protocol, if it is present, or direct I/O write, if not.
//
// Input:       None
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID InitCommunicate ()
{

    UINTN CommSize = sizeof(EFI_SMM_COMMUNICATE_HEADER);

   if (SmmCommProtocol){
//	   TRACE((-1,"Using Communicate\n"));
       SmmCommProtocol->Communicate (SmmCommProtocol, CommHeaderPtr, &CommSize);
   }

    return;
}

VOID GetSmmRsPtr()
{
    if (SmmpRs == NULL)
        SmmpRs = GetSmstConfigurationTable(&SmmRtServTableGuid);
    return;
}

#if NVRAM_SMI_FULL_PROTECTION == 1
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetVariableToSmi
//
// Description: This function searches for Var with specific GUID and Name
//
// Input:       IN CHAR16 *VariableName - pointer to Var Name in Unicode
//              IN EFI_GUID *VendorGuid - pointer to Var GUID
//              OPTIONAL OUT UINT32* Attributes - Pointer to memory where Attributes will be returned
//              IN OUT UINTN *DataSize - size of Var - if smaller than actual EFI_BUFFER_TOO_SMALL
//              will be returned and DataSize will be set to actual size needed
//              OUT VOID *Data - Pointer to memory where Var will be returned
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetVariableToSmi (
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes OPTIONAL,
    IN OUT UINTN *DataSize, OUT VOID *Data
)
{
    EFI_STATUS          Status;
    UINTN               Length;
    SMI_VARIABLE        *Buffer;
    BOOLEAN				InSmm = FALSE;
    //TRACE((-1,"GetVariableToSmi. Name %S\n", VariableName));
    if (!VariableName || !VendorGuid || !DataSize || !Data && *DataSize)
        return EFI_INVALID_PARAMETER;
    Length = (Wcslen(VariableName) + 1)*sizeof(CHAR16);
    if ((NVRAM_SIZE - sizeof(SMI_VARIABLE)) <=  Length)
    	return EFI_OUT_OF_RESOURCES;

    Status = EFI_NO_RESPONSE;
    if (gNvramBuffer != NULL) {
        if (EFI_ERROR(NvSmiBeginCriticalSection(&NvramSmiCs)))
            return EFI_ACCESS_DENIED;
        Buffer = (SMI_VARIABLE*)gNvramBuffer;
        
        Buffer->Subfunction = SMI_GET_VARIABLE;
        Buffer->Signature = NVAR_SIGNATURE;
        if (Attributes) Buffer->VarAttrib = *Attributes;
        Buffer->VarSize = *DataSize;
        Buffer->Status = Status;
        Buffer->VarGuid = *VendorGuid;
        MemCpy((UINT8*)&Buffer->VarData, (UINT8*)VariableName, Length);
        if (!ExitBS)
            pSmmBase->InSmm(pSmmBase, &InSmm);
        if (!InSmm)
            InitCommunicate ();
        else
        {
	        Status = EFI_SECURITY_VIOLATION;
            if (pRS->GetVariable == GetVariableToSmi)
            {
		        if (pSmst==NULL)
		        {
                    pSmmBase->GetSmstLocation(pSmmBase,&pSmst);
   		        }
                GetSmmRsPtr();
                if (SmmpRs != NULL)
		        {
                Status = SmmpRs->GetVariable(VariableName, VendorGuid,
                                                Attributes, DataSize, Data);
   		        }
            }
            NvSmiEndCriticalSection (&NvramSmiCs);
	        return Status;
        }
        Status = Buffer->Status;
        if (Status != EFI_NO_RESPONSE) {

            if (Status == EFI_BUFFER_TOO_SMALL) *DataSize = Buffer->VarSize;
            if (!EFI_ERROR(Status)) {

                if (Attributes) *Attributes = Buffer->VarAttrib;
                *DataSize = Buffer->VarSize;
                MemCpy((UINT8*)Data, &Buffer->VarData, Buffer->VarSize);
            }
        }

        NvSmiEndCriticalSection (&NvramSmiCs);
    }
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetNextVariableNameToSmi
//
// Description: This function searches for next Var after Var with specific name and GUID and returns it's Name
//              in SMM synchronizing Varstors before and after operation.
//
// Input:       IN OUT UINTN *VariableNameSize - size of Varname - if smaller than actual EFI_BUFFER_TOO_SMALL
//              will be returned and DataSize will be set to actual size needed
//              IN OUT CHAR16 *VariableName - pointer where Var Name in Unicode will be stored
//              IN OUT EFI_GUID *VendorGuid - pointer to menory where Var GUID is stored
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GetNextVariableNameToSmi (
    IN OUT UINTN *VariableNameSize,
    IN OUT CHAR16 *VariableName, IN OUT EFI_GUID *VendorGuid
)
{
    EFI_STATUS          Status;
    UINTN               Length;
    SMI_VARIABLE        *Buffer;
    BOOLEAN				InSmm = FALSE;
    //TRACE((-1,"GetNextVariableNameToSmi. Name %S\n", VariableName));
    if ( !VariableNameSize || !VariableName || !VendorGuid)
        return EFI_INVALID_PARAMETER;
    Status = EFI_NO_RESPONSE;
    if (gNvramBuffer != NULL) {
        if (EFI_ERROR(NvSmiBeginCriticalSection(&NvramSmiCs)))
            return EFI_ACCESS_DENIED;

        Length = (Wcslen(VariableName) + 1)*sizeof(CHAR16);
        Buffer = (SMI_VARIABLE*)gNvramBuffer;
        Buffer->Subfunction = SMI_GET_NEXT_VAR_NAME;
        Buffer->Signature = NVAR_SIGNATURE;
        Buffer->VarSize = *VariableNameSize;
        Buffer->Status = Status;
        Buffer->VarGuid = *VendorGuid;

        MemCpy((UINT8*)&Buffer->VarData, (UINT8*)VariableName, Length);
        if (!ExitBS)
            pSmmBase->InSmm(pSmmBase, &InSmm);
        if (!InSmm)
            InitCommunicate ();
        else
        {
	        Status = EFI_SECURITY_VIOLATION;
            if (pRS->GetNextVariableName == GetNextVariableNameToSmi)
            {
		        if (pSmst==NULL)
		        {
                    pSmmBase->GetSmstLocation(pSmmBase,&pSmst);
   		        }
                GetSmmRsPtr();
                if (SmmpRs != NULL)
		        {
                Status = SmmpRs->GetNextVariableName(VariableNameSize, VariableName,VendorGuid);
   		        }
            }
            NvSmiEndCriticalSection (&NvramSmiCs);
	        return Status;
        }

        Status = Buffer->Status;
        if (Status != EFI_NO_RESPONSE) {

            *VariableNameSize = Buffer->VarSize;
            if (!EFI_ERROR(Status)) {

                MemCpy((UINT8*)VariableName, &Buffer->VarData, Buffer->VarSize);
                MemCpy((UINT8*)VendorGuid, (UINT8*)&Buffer->VarGuid, sizeof(EFI_GUID));
            }
        }

        NvSmiEndCriticalSection (&NvramSmiCs);
    }

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   QueryVariableInfoToSmi
//
// Description: This function returns parameters of VarStore with passed attributes
//
// Input:       IN UINT32 Attributes - Atributes to search for
//              OUT UINT64 *MaximumVariableStorageSize - Maximum Variable Storage Size
//              OUT UINT64 *RemainingVariableStorageSize - Remaining Variable Storage Size
//              OUT UINT64 *MaximumVariableSize - Maximum Variable Size
//
// Output:      EFI_STATUS based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS QueryVariableInfoToSmi (
    IN UINT32 Attributes,
    OUT UINT64 *MaximumVariableStorageSize,
    OUT UINT64 *RemainingVariableStorageSize,
    OUT UINT64 *MaximumVariableSize
)
{

    EFI_STATUS          Status;
    SMI_VARIABLE        *Buffer;
    BOOLEAN				InSmm = FALSE;
    if (!Attributes || !MaximumVariableStorageSize || !RemainingVariableStorageSize || !MaximumVariableSize)
        return EFI_INVALID_PARAMETER;
    //TRACE((-1, "QueryVariableInfoToSmi.\n"));
    Status = EFI_NO_RESPONSE;
    if (gNvramBuffer != NULL) {
        if (EFI_ERROR(NvSmiBeginCriticalSection(&NvramSmiCs)))
            return EFI_ACCESS_DENIED;
        Buffer = (SMI_VARIABLE*)gNvramBuffer;

        Buffer->Subfunction = SMI_QUERY_VAR_INFO;
        Buffer->Signature = NVAR_SIGNATURE;
        Buffer->VarAttrib = Attributes;
        if (!ExitBS)
            pSmmBase->InSmm(pSmmBase, &InSmm);
        if (!InSmm)
            InitCommunicate ();
        else
        {
	        Status = EFI_SECURITY_VIOLATION;
            if (pRS->QueryVariableInfo == QueryVariableInfoToSmi)
            {
		        if (pSmst==NULL)
		        {
                    pSmmBase->GetSmstLocation(pSmmBase,&pSmst);
   		        }
                GetSmmRsPtr();
                if (SmmpRs != NULL)
		        {
                Status = SmmpRs->QueryVariableInfo(Attributes, MaximumVariableStorageSize,
                                                RemainingVariableStorageSize, MaximumVariableSize);
   		        }
            }
            NvSmiEndCriticalSection (&NvramSmiCs);
	        return Status;
        }

        Status = Buffer->Status;
        if (!EFI_ERROR(Status)) {
            *MaximumVariableStorageSize = Buffer->MaxVarStorageSize;
            *RemainingVariableStorageSize = Buffer->RemVarStorageSize;
            *MaximumVariableSize = Buffer->MaxVarSize;
        }

        NvSmiEndCriticalSection(&NvramSmiCs);
    }

    return Status;
}

#endif //#if NVRAM_SMI_FULL_PROTECTION == 1
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetVariableToSmi
//
// Description: This function sets Var with specific GUID, Name and attributes
//              beginning and ending critical section.
//
// Input:       IN CHAR16 *VariableName - pointer to Var Name in Unicode
//              IN EFI_GUID *VendorGuid - pointer to Var GUID
//              IN UINT32 Attributes - Attributes of the Var
//              IN UINTN DataSize - size of Var
//              IN VOID *Data - Pointer to memory where Var data is stored
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SetVariableToSmi (
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    IN UINT32 Attributes, IN UINTN DataSize, IN VOID *Data
)
{
    EFI_STATUS          Status;
    UINTN               Length;
    SMI_VARIABLE        *Buffer;
    BOOLEAN				InSmm = FALSE;
    //TRACE((-1,"SetVariableToSmi. Name %S\n", VariableName));
    if (!VariableName || VariableName[0]==0 || !VendorGuid || (DataSize && !Data))
        return EFI_INVALID_PARAMETER;
    Length = (Wcslen(VariableName) + 1)*sizeof(CHAR16);
    if (((UINTN)(~0) - DataSize) < Length)
      return EFI_OUT_OF_RESOURCES; // Prevent whole variable size overflow (if DataSize + Length > UINTN)

    if ((DataSize + Length) > (NVRAM_SIZE - sizeof(SMI_VARIABLE) - 1)) 
    	return EFI_OUT_OF_RESOURCES;
//EIP151328 >>
    if (!(Attributes & EFI_VARIABLE_NON_VOLATILE) && (DataSize != 0)) {
        Status = gSavedSetVariable (VariableName, \
                                VendorGuid, Attributes, DataSize, Data);
//-        SMDbgPrint("[SmiSetVariable-RT] Name : %S, Attr %x :: %r ]\n",
//-                        VariableName, Attributes, Status);
//EIP190823
        if (!EFI_ERROR(Status))   return Status;                
//EIP190823
    }
//EIP151328 <<

    Status = EFI_NO_RESPONSE;
    if (gNvramBuffer != NULL) {

        if (EFI_ERROR(NvSmiBeginCriticalSection(&NvramSmiCs)))
            return EFI_ACCESS_DENIED;

        Buffer = (SMI_VARIABLE*)gNvramBuffer;
       
        Buffer->Subfunction = SMI_SET_VARIABLE;
        Buffer->Signature = NVAR_SIGNATURE;
        Buffer->VarAttrib = Attributes;
        Buffer->VarSize = DataSize;
        Buffer->Status = Status;
        Buffer->VarGuid = *VendorGuid;
        // VariableName address must be allined on 16 bit boundary
        if (((UINTN)&Buffer->VarData + DataSize) & 1)
        	MemCpy(((UINT8*)&Buffer->VarData + DataSize + 1), (UINT8*)VariableName, Length);
        else 
        	MemCpy(((UINT8*)&Buffer->VarData + DataSize), (UINT8*)VariableName, Length);
        MemCpy(&Buffer->VarData, (UINT8*)Data, DataSize);
        if (!ExitBS)
            pSmmBase->InSmm(pSmmBase, &InSmm);
        if (!InSmm)
            InitCommunicate ();
        else
        {
	        Status = EFI_SECURITY_VIOLATION;
            if (pRS->SetVariable == SetVariableToSmi)
            {
		        if (pSmst==NULL)
		        {
                    pSmmBase->GetSmstLocation(pSmmBase,&pSmst);
   		        }
                GetSmmRsPtr();
                if (SmmpRs != NULL)
		        {
                Status = SmmpRs->SetVariable(VariableName, VendorGuid,
                                                Attributes, DataSize, Data);
   		        }
            }
            NvSmiEndCriticalSection (&NvramSmiCs);
	        return Status;
        }

        Status = Buffer->Status;

        NvSmiEndCriticalSection(&NvramSmiCs);
        //TRACE((-1, "[ SmiSetVariable - %S %r , Data=%x]\n",VariableName, Status, Data));
    }

    return Status;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   NvramSmiReadyToBoot
//
//  Description: This function will be called when Ready To Boot will signaled
//  will update data to work in RunTime.
//
//  Input:  IN EFI_EVENT Event - signalled event
//          IN VOID *Context - calling context
//
//  Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID NvramSmiReadyToBoot (
    IN EFI_EVENT    Event,
    IN VOID         *Context
)
{
    UINT32 CRC32=0;
#if NVRAM_SMI_FULL_PROTECTION == 1

    typedef struct
    {
        BOOLEAN StopRtDataAccess;
        //TODO: there are more field inthe mailbox.
        // we assume that StopRtDataAccess will always be the first field.
    } NVRAM_MAILBOX;

    #define NVRAM_MAILBOX_ADDRESS_VARIABLE_GUID \
        {0x54913a6d, 0xf4ee, 0x4cdb, 0x84, 0x75, 0x74, 0x6, 0x2b, 0xfc, 0xec, 0xf5}

    static EFI_GUID NvRamMailboxVariableGuid = NVRAM_MAILBOX_ADDRESS_VARIABLE_GUID;
    NVRAM_MAILBOX *NvRamMailbox;
    EFI_STATUS  Status;
    UINTN       Size=sizeof(NvRamMailbox);

    Status = pRS->GetVariable(
        L"NvRamMailbox",&NvRamMailboxVariableGuid,
        NULL, &Size, &NvRamMailbox
    );
    if (EFI_ERROR(Status)) return;

    NvRamMailbox->StopRtDataAccess = TRUE;
    //Make a dummy SMM GetVariable call to process StopRtDataAccess
    GetVariableToSmi(
        L"NvRamMailbox",&NvRamMailboxVariableGuid,
        NULL, &Size, &NvRamMailbox
    );

#endif //#if NVRAM_SMI_FULL_PROTECTION == 1

    //Init CommHeaderPtr----

    if (gNvramBuffer == NULL) return;

    CommHeaderPtr = &CommHeader;

    TRACE((-1, "[ NvramSmi-ReadyToLock ]\n"));
//EIP151328 >>
    gSavedSetVariable = pRS->SetVariable;
//EIP151328 <<
    pRS->SetVariable = SetVariableToSmi;
#if NVRAM_SMI_FULL_PROTECTION == 1
    pRS->GetVariable = GetVariableToSmi;
    pRS->GetNextVariableName = GetNextVariableNameToSmi;
    pRS->QueryVariableInfo = QueryVariableInfoToSmi;
#endif //#if NVRAM_SMI_FULL_PROTECTION == 1
    pRS->Hdr.CRC32 = 0;
    pBS->CalculateCrc32(pRS, sizeof(*pRS), &CRC32);
    pRS->Hdr.CRC32 = CRC32;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   NvramSmiVirtualFixup
//
//  Description: Called on Virtual address change event and converts
//               SmmCommProtocol pointer.
//
//  Input:  IN EFI_EVENT Event - signalled event
//          IN VOID *Context - calling context
//
//  Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID NvramSmiVirtualFixup (
    IN EFI_EVENT    Event,
    IN VOID         *Context
)
{
    TRACE((-1, "NvramSmi Go Virtual\n"));
    pRS->ConvertPointer (0, &gNvramBuffer);
//EIP151328 >>
    pRS->ConvertPointer (0, (VOID**)&gSavedSetVariable);
//EIP151328 <<
    if (SmmCommProtocol != NULL)
        pRS->ConvertPointer (0, (VOID**)&SmmCommProtocol);

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   NvramSmiExitBs
//
//  Description: Called on Exit BS and updates ExitBS variable
//
//
//  Input:  IN EFI_EVENT Event - signalled event
//          IN VOID *Context - calling context
//
//  Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID NvramSmiExitBs (
    IN EFI_EVENT    Event,
    IN VOID         *Context
)
{
    ExitBS = TRUE;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NvramSmiDxeNotInSmm
//
// Description: This is the "NotInSmmFunction" parameter for the
//              InitSmmHandler function in the entry point. It allocates
//              memory for parameters buffer, saves its location, etc.
//
// Input:       EFI_HANDLE          - ImageHandle
//              EFI_SYSTEM_TABLE*   - SystemTable
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS 
EFIAPI
NvramSmiDxeEntry(
    IN EFI_HANDLE          ImageHandle,
    IN EFI_SYSTEM_TABLE    *SystemTable
)
{
    EFI_STATUS  Status;
    EFI_EVENT   EvtReadyToBoot, EvtExitBS;

    EFI_EVENT   EvtComm;

    VOID        *RegReadyToBoot =  NULL, *RegComm =  NULL;
    static EFI_GUID    StartNvramSmiServicesGuid = NVRAM_START_SMI_SERVICES_GUID;
    static EFI_GUID    gAmiSmiVariableDxeDriverStartedGuid = { 0xc35f9520, 0x5791, 0x4667, { 0xad, 0xe4, 0x1c, 0xfd, 0xa8, 0x37, 0x72, 0x2d } };
    EFI_HANDLE  DummyHandle = NULL;
    Status = InitAmiSmmLib (ImageHandle, SystemTable);
    InitAmiRuntimeLib(
                       ImageHandle, SystemTable, NvramSmiExitBs, NvramSmiVirtualFixup
                       );
    TRACE((-1,"SmiNVRAM NotInSmmFunction.\n"));
    Status = pBS->AllocatePool ( EfiRuntimeServicesData,
                                 NVRAM_SIZE,
                                 &gNvramBuffer );
    if (EFI_ERROR(Status)) return Status;

    Status = pRS->SetVariable ( L"NvramSmiBuffer",
                                &gNvramSmiGuid,
                                EFI_VARIABLE_BOOTSERVICE_ACCESS,
                                sizeof(gNvramBuffer),
                                &gNvramBuffer  );
    if (EFI_ERROR(Status)) return Status;

    CreateLegacyBootEvent(TPL_CALLBACK, &NvramSmiExitBs, NULL, &EvtExitBS);
    Status = RegisterProtocolCallback ( &StartNvramSmiServicesGuid,
                                        NvramSmiReadyToBoot,
                                        NULL,
                                        &EvtReadyToBoot,
                                        &RegReadyToBoot );
    ASSERT_EFI_ERROR (Status);

    Status = LocateCommunicateProt (NULL, NULL);

    if (EFI_ERROR(Status)){

        Status = RegisterProtocolCallback ( &gEfiSmmCommunicationProtocolGuid,
                                            LocateCommunicateProt,
                                            NULL,
                                            &EvtComm,
                                            &RegComm );
    }

    //---Installing a Dummy protocol, on which Smm part is relying to make sure
    //---InSmm function will start after NotInSmm
    pBS->InstallProtocolInterface (
        &DummyHandle, &gAmiSmiVariableDxeDriverStartedGuid, EFI_NATIVE_INTERFACE, NULL
    );
    return Status;
}

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**             5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093          **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
