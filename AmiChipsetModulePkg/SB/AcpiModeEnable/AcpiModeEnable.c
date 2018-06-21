//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.            **
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
// $Header: $
//
// $Revision: $
//
// $Date: $
//*************************************************************************

//<AMI_FHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AcpiModeEnable.c
//
// Description: Provide functions to enable and disable ACPI mode
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <Token.h>
#include <AmiDxeLib.h>
#include <AmiCspLib.h>

#include <Protocol/SmmBase2.h>
#include <Protocol/SmmSwDispatch2.h>

#include <Protocol/DevicePath.h>
#include "AcpiModeEnable.h"
#include <Library/ElinkLib.h>
#include <PchAccess.h>

#define AMI_SMM_SW_DISPATCH_PROTOCOL EFI_SMM_SW_DISPATCH2_PROTOCOL
#define AMI_SMM_SW_DISPATCH_CONTEXT  EFI_SMM_SW_REGISTER_CONTEXT
#define SMM_CHILD_DISPATCH_SUCCESS   EFI_SUCCESS

//---------------------------------------------------------------------------
// Variable and External Declaration(s)
//---------------------------------------------------------------------------
// Variable Declaration(s)

ACPI_DISPATCH_LINK  *gAcpiEnDispatchHead = 0, *gAcpiEnDispatchTail = 0;
ACPI_DISPATCH_LINK  *gAcpiDisDispatchHead = 0, *gAcpiDisDispatchTail = 0;

UINT16 wPM1_SaveState;
UINT32 dGPE_SaveState;

typedef VOID (ACPI_MODE_CALLBACK)(
    IN EFI_HANDLE                   DispatchHandle,
    IN AMI_SMM_SW_DISPATCH_CONTEXT  *DispatchContext
);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   AddLink
//
// Description: Create and add link to specified list.
//
// Input:  Size -
//              Head -
//              Tail -
//
// Output:     VOID Pointer
//
// Modified:
//
// Referrals:   SmmAllocatePool
//
// Notes:       Here is the control flow of this function:
//              1. Allocate Link in Smm Pool.
//              2. Add Link to end.
//              3. Return Link address.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID * AddLink(
    IN UINT32       Size,
    IN VOID         **Head,
    IN VOID         **Tail)
{
    VOID *Link;

    if(pSmst->SmmAllocatePool(EfiRuntimeServicesData, Size, &Link) != EFI_SUCCESS) return 0;

    ((GENERIC_LINK*)Link)->Link = 0;
    if(!*Head) {
        *Head = *Tail = Link;
    } else {
        ((GENERIC_LINK*)*Tail)->Link = Link;
        *Tail = Link;
    }

    return Link;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   RemoveLink
//
// Description: Remove link from specified list.
//
// Input:  Handle - EFI Handle
//              Head -
//              Tail -
//
// Output:     BOOLEAN
//                  TRUE if link was removed. FALSE if link not in the list.
//
// Modified:
//
// Referrals:   SmmFreePool
//
// Notes:       Here is the control flow of this function:
//              1. Search link list for Link.
//              2. Remove link from list.
//              3. Free link.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN RemoveLink(
    IN EFI_HANDLE   Handle,
    IN VOID         **Head,
    IN VOID         **Tail)
{
    GENERIC_LINK *PrevLink, *Link;

    PrevLink = *Head;

    // Is link first. Link address is the same as the Handle.
    if(((GENERIC_LINK*)*Head) == Handle) {
        if(PrevLink == *Tail) *Tail = 0;  // If Tail = Head, then 0.
        *Head = PrevLink->Link;
        pSmst->SmmFreePool(PrevLink);
        return TRUE;
    }

    // Find Link.
    for(Link = PrevLink->Link; Link; PrevLink = Link, Link = Link->Link) {
        if(Link == Handle) {    // Link address is the same as the Handle.
            if(Link == *Tail) *Tail = PrevLink;
            PrevLink->Link = Link->Link;
            pSmst->SmmFreePool(Link);
            return TRUE;
        }
    }

    return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EfiAcpiEnRegister
//
// Description: Register a Link on ACPI enable SMI.
//
// Input:  This -
//              Function -
//              Context -
//
//
// Output:     Handle -
//              EFI_STATUS
//
// Modified:    gAcpiEnDispatchHead, gAcpiEnDispatchTail
//
// Referrals:   AddLink
//
// Notes:       Here is the control flow of this function:
//              1. Verify if Context if valid. If invalid,
//                 return EFI_INVALID_PARAMETER.
//              2. Allocate structure and add to link list.
//              3. Fill link.
//              4. Enable Smi Source.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EfiAcpiEnRegister(
    IN EFI_ACPI_DISPATCH_PROTOCOL   *This,
    IN EFI_ACPI_DISPATCH            Function,
    OUT EFI_HANDLE                  *Handle)
{
    ACPI_DISPATCH_LINK *NewLink;

    NewLink = AddLink(sizeof(ACPI_DISPATCH_LINK), \
                      &gAcpiEnDispatchHead, \
                      &gAcpiEnDispatchTail);
    if(!NewLink) return EFI_OUT_OF_RESOURCES;

    NewLink->Function   = Function;
    *Handle = NewLink;

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EfiAcpiEnUnregister
//
// Description: Unregister a Link on ACPI enable SMI.
//
// Input:  This -
//              Handle -
//
// Output:     EFI_STATUS
//
// Modified:    gAcpiEnDispatchHead, gAcpiEnDispatchTail
//
// Referrals:   RemoveLink
//
// Notes:       Here is the control flow of this function:
//              1. Remove link. If no link, return EFI_INVALID_PARAMETER.
//              2. Disable SMI Source if no other handlers using source.
//              3. Return EFI_SUCCESS.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EfiAcpiEnUnregister(
    IN EFI_ACPI_DISPATCH_PROTOCOL   *This,
    IN EFI_HANDLE                   Handle)
{
    if(!RemoveLink(Handle, &gAcpiEnDispatchHead, &gAcpiEnDispatchTail))
        return EFI_INVALID_PARAMETER;
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EfiAcpiDisRegister
//
// Description: Register a Link on ACPI disable SMI.
//
// Input:  This -
//              Function -
//              *Context -
//
//
// Output:     Handle - EFI Handle
//              EFI_STATUS
//
// Modified:    gAcpiDisDispatchHead, gAcpiDisDispatchTail
//
// Referrals:   AddLink
//
// Notes:       Here is the control flow of this function:
//              1. Verify if Context if valid. If invalid,
//                 return EFI_INVALID_PARAMETER.
//              2. Allocate structure and add to link list.
//              3. Fill link.
//              4. Enable Smi Source.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EfiAcpiDisRegister(
    IN EFI_ACPI_DISPATCH_PROTOCOL   *This,
    IN EFI_ACPI_DISPATCH            Function,
    OUT EFI_HANDLE                  *Handle)
{
    ACPI_DISPATCH_LINK *NewLink;

    NewLink = AddLink(sizeof(ACPI_DISPATCH_LINK), \
                      &gAcpiDisDispatchHead, \
                      &gAcpiDisDispatchTail);
    if(!NewLink) return EFI_OUT_OF_RESOURCES;

    NewLink->Function   = Function;
    *Handle = NewLink;

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EfiAcpiDisUnregister
//
// Description: Unregister a Link on ACPI Disable SMI.
//
// Input:  This -
//              Handle - EFI Handle
//
// Output:     EFI_STATUS
//
// Modified:    gAcpiDisDispatchHead, gAcpiDisDispatchTail
//
// Referrals:   RemoveLink
//
// Notes:       Here is the control flow of this function:
//              1. Remove link. If no link, return EFI_INVALID_PARAMETER.
//              2. Disable SMI Source if no other handlers using source.
//              3. Return EFI_SUCCESS.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EfiAcpiDisUnregister(
    IN EFI_ACPI_DISPATCH_PROTOCOL   *This,
    IN EFI_HANDLE                   Handle)
{
    if(!RemoveLink(Handle, &gAcpiDisDispatchHead, &gAcpiDisDispatchTail))
        return EFI_INVALID_PARAMETER;
    return EFI_SUCCESS;
}

EFI_ACPI_DISPATCH_PROTOCOL gEfiAcpiEnDispatchProtocol = \
{EfiAcpiEnRegister, EfiAcpiEnUnregister};

EFI_ACPI_DISPATCH_PROTOCOL gEfiAcpiDisDispatchProtocol = \
{EfiAcpiDisRegister, EfiAcpiDisUnregister};


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NbOemSetupCallbacks
//
// Description: This function calls registered callbacks for OEM/custom setup.
//
// Input:       *Services    - Pointer to PeiServices or RuntimeServices
//                             structure
//              *NbSetupData - Pointer to custom setup data to return
//              *SetupData   - Pointer to system setup data.
//              Pei          - Pei flag. If TRUE we are in PEI phase
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID EnableAcpiModeCallbacks(
    IN EFI_HANDLE                   DispatchHandle,
    IN AMI_SMM_SW_DISPATCH_CONTEXT  *DispatchContext)
{
    UINT32                  ElinkPtr;
    AMI_HOOK_LINK           *AmiHookLink;
    UINT32                  Index;
    ACPI_MODE_CALLBACK      *Elink;

    ElinkPtr = ElinkGet(PcdToken(PcdEnableAcpiModeElink));

    if(ElinkPtr == 0) {
        return;
    }

    AmiHookLink = (AMI_HOOK_LINK *) ElinkPtr;

    for(Index = 0; Index < ELINK_ARRAY_NUM; Index++) {
        if(AmiHookLink->ElinkArray[Index] == NULL) {
            break;
        }
        Elink = AmiHookLink->ElinkArray[Index];
        Elink(DispatchHandle, DispatchContext);
    }
}

//EIP128360 >>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SbGpioSciInit
//
// Description: Programming the corresponding GPIO pin to generate SCI#.
//
// Input:       None
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SbGpioSciInit (VOID)
{
  UINT32 GpioRoute;
  UINT8  GpioIndex;

  GpioRoute = MmioRead32(PMC_BASE_ADDRESS + R_PCH_PMC_GPI_ROUT);
  for (GpioIndex = 0; GpioIndex < 16; GpioIndex++) {
    if (GPIO_SCI_BITMAP & (UINT16)(1 << GpioIndex)) {
      GpioRoute &= ~(3 << (GpioIndex * 2));
      GpioRoute |= (2 << (GpioIndex * 2));
    }
  }
  MmioWrite32(PMC_BASE_ADDRESS + R_PCH_PMC_GPI_ROUT , GpioRoute); 
}
//EIP128360 <<
  
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EnableAcpiMode
//
// Description: This function enable ACPI mode by clearing all SMI and
//              enabling SCI generation
//              This routine is also called on a S3 resume for special ACPI
//              programming.
//              Status should not be cleared on S3 resume. Enables are
//              already taken care of.
//
// Input:       PI 1.1, 1.2
//                  DispatchHandle  - SMI dispatcher handle
//                  *DispatchContext- Points to an optional S/W SMI context
//                  CommBuffer      - Points to the optional communication
//                                    buffer
//                  CommBufferSize  - Points to the size of the optional
//                                    communication buffer
//
// Output:      EFI_STATUS if the new SMM PI is applied.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EnableAcpiMode(
    IN EFI_HANDLE       DispatchHandle,
    IN CONST VOID       *DispatchContext OPTIONAL,
    IN OUT VOID         *CommBuffer OPTIONAL,
    IN OUT UINTN        *CommBufferSize OPTIONAL)
{
    ACPI_DISPATCH_LINK      *Link;
//    UINT8                   OutputValue;
//    UINT32                  SmiEn;
    UINT16                  Pm1Cnt;
    UINT16    wordValue;
    UINT32    dwordValue;
    
    TRACE((TRACE_ALWAYS, "[[ EnableAcpiMode() Start...... ]]\n"));
    
    //  Check if WAK bit is set, if yes skip clearing status
    wordValue = IoRead16(PM_BASE_ADDRESS + R_PCH_ACPI_PM1_STS); // 0x00
    if (wordValue & B_PCH_ACPI_PM1_STS_WAK) {
    
    	  //  NAPA-CHANGES Disable and Clear GPE0 Sources
    	IoWrite32 (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_EN, 0);    //GPE0_EN 0x28
    	IoWrite32 (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_STS, 0xFFFFFFFF);  //GPE0_STS
    	
    }
    else{
    	
        //Disable SMI Sources
        dwordValue = IoRead32(PM_BASE_ADDRESS + R_PCH_SMI_EN);    //SMI_EN (SMI Control and Enable register.)
    	
        dwordValue &= ~(B_PCH_SMI_EN_SWSMI_TMR | B_PCH_SMI_EN_LEGACY_USB2);  // Clear SLP_SMI_EN and SWSMI_TMR bit. //EIP128356 //EIP173310
        IoWrite32 (PM_BASE_ADDRESS + R_PCH_SMI_EN, dwordValue);
        
        //Disable and Clear PM1 Sources except power buttonsymptom
        wPM1_SaveState = IoRead16(PM_BASE_ADDRESS + R_PCH_ACPI_PM1_EN);  //PM1_EN
        IoWrite16 (PM_BASE_ADDRESS + R_PCH_ACPI_PM1_EN, (UINT16)B_PCH_ACPI_PM1_STS_PWRBTN);    //PM1_EN Bit 8: PWRBTN_EN
        IoWrite16 (PM_BASE_ADDRESS + R_PCH_ACPI_PM1_STS, 0xffff);    //PM1_STS 0x00
        
        //
        // Disable and Clear GPE0 Sources
        //
        IoWrite32 (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_EN, 0); //GPE0_EN 0x28
        IoWrite32 (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_STS, 0xFFFFFFFF);  //GPE0_STS

        //
        // Guarantee day-of-month alarm is invalid (ACPI 1.0 section 4.7.2.4)
        //
        /*
        OutputValue = 0x0D;
        IoWrite8(0x74, OutputValue);
        OutputValue = 0x0;
        OutputValue = IoRead8(0x75);
        */
        IoWrite8(CMOS_ADDR_PORT, 0xd | 0x80);                //RTC_REGD
        IoWrite8(CMOS_DATA_PORT, 0);
    }

    //
    // Enable SCI
    //
    Pm1Cnt = IoRead16(PM_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT);
    Pm1Cnt |= B_PCH_ACPI_PM1_CNT_SCI_EN;
    IoWrite16(PM_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT, Pm1Cnt);

    SbGpioSciInit(); //EIP128360 

    for (Link = gAcpiEnDispatchHead; Link; Link = Link->Link)
    {
        Link->Function(Link);
    }

    EnableAcpiModeCallbacks (DispatchHandle, DispatchContext);

    IoWrite8(0x80, SW_SMI_ACPI_ENABLE);
    TRACE((TRACE_ALWAYS, "[[ EnableAcpiMode() End...... ]]\n"));
    return SMM_CHILD_DISPATCH_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NbOemSetupCallbacks
//
// Description: This function calls registered callbacks for OEM/custom setup.
//
// Input:       *Services    - Pointer to PeiServices or RuntimeServices
//                             structure
//              *NbSetupData - Pointer to custom setup data to return
//              *SetupData   - Pointer to system setup data.
//              Pei          - Pei flag. If TRUE we are in PEI phase
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID DisableAcpiModeCallbacks(
    IN EFI_HANDLE                   DispatchHandle,
    IN AMI_SMM_SW_DISPATCH_CONTEXT  *DispatchContext)
{
    UINT32                  ElinkPtr;
    AMI_HOOK_LINK           *AmiHookLink;
    UINT32                  Index;
    ACPI_MODE_CALLBACK      *Elink;

    ElinkPtr = ElinkGet(PcdToken(PcdDisableAcpiModeElink));

    if(ElinkPtr == 0) {
        return;
    }

    AmiHookLink = (AMI_HOOK_LINK *) ElinkPtr;

    for(Index = 0; Index < ELINK_ARRAY_NUM; Index++) {
        if(AmiHookLink->ElinkArray[Index] == NULL) {
            break;
        }
        Elink = AmiHookLink->ElinkArray[Index];
        Elink(DispatchHandle, DispatchContext);
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   DisableAcpiMode
//
// Description: This function disables ACPI mode by enabling SMI generation
//
// Input:       PI 1.1, 1.2
//                  DispatchHandle  - SMI dispatcher handle
//                  *DispatchContext- Points to an optional S/W SMI context
//                  CommBuffer      - Points to the optional communication
//                                    buffer
//                  CommBufferSize  - Points to the size of the optional
//                                    communication buffer
//
// Output:      EFI_STATUS if the new SMM PI is applied.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS DisableAcpiMode(
    IN EFI_HANDLE       DispatchHandle,
    IN CONST VOID       *DispatchContext OPTIONAL,
    IN OUT VOID         *CommBuffer OPTIONAL,
    IN OUT UINTN        *CommBufferSize OPTIONAL)
{
    ACPI_DISPATCH_LINK      *Link;
    /** PORTING REQUIRED.  Disable all SCI sources and enable SMI generation for non-ACPI Mode
        UINT8 byteValue;

        //Clear PM Sources and set Enables
        IoWrite16(PM_BASE_ADDRESS + AMD_8111_PM_1STS, 0xffff);                  //PM1_STS
        IoWrite16(PM_BASE_ADDRESS + AMD_8111_PM_1NABL, wPM1_SaveState);         //PM1_EN

        //Clear GPE0 Sources and set Enables
        IoWrite16(PM_BASE_ADDRESS + AMD_8111_PM_GP0_STS, 0xffff);               //GPE0_STS
        IoWrite16(PM_BASE_ADDRESS + AMD_8111_PM_GP0_NABL, wGPE_SaveState);      //GPE0_EN

        //Disable SCI
        byteValue = IoRead8(PM_BASE_ADDRESS + AMD_8111_PM_1CTL);                //PM1_CNT
        byteValue &= 0xfe;                                                      //SCI_EN = 0
        IoWrite8(PM_BASE_ADDRESS + AMD_8111_PM_1CTL, byteValue);
     **/

    for(Link = gAcpiDisDispatchHead; Link; Link = Link->Link) {
        Link->Function(Link);
    }

    DisableAcpiModeCallbacks(DispatchHandle, DispatchContext);

    IoWrite8(0x80, SW_SMI_ACPI_DISABLE);

    return SMM_CHILD_DISPATCH_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   AcpiModeEnableInSmmFunction
//
// Description: This function is part of the ACPI mode enable/disable
//              driver and invoked during SMM initialization.  As the name
//              suggests this function is called from SMM
//
// Input:       IN  EFI_HANDLE ImageHandle - Handle for this FFS image
//              IN  EFI_SYSTEM_TABLE *SystemTable - Pointer to the system table
//
// Output:      EFI_STATUS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS AcpiModeEnableInSmmFunction(
    IN  EFI_HANDLE          ImageHandle,
    IN  EFI_SYSTEM_TABLE    *SystemTable
)
{
    EFI_STATUS                   Status;
    EFI_HANDLE                   Handle = NULL;
    AMI_SMM_SW_DISPATCH_PROTOCOL *SwDispatch = NULL;
    EFI_HANDLE                   DummyHandle = NULL;
    AMI_SMM_SW_DISPATCH_CONTEXT  AcpiEnableContext = {SW_SMI_ACPI_ENABLE};
    AMI_SMM_SW_DISPATCH_CONTEXT  AcpiDisableContext = {SW_SMI_ACPI_DISABLE};

    Status = InitAmiSmmLib(ImageHandle, SystemTable);
    if(EFI_ERROR(Status)) return Status;

    Status = pSmst->SmmLocateProtocol(&gEfiSmmSwDispatch2ProtocolGuid , \
                                      NULL, \
                                      &SwDispatch);
    if(EFI_ERROR(Status)) return Status;

    Status = SwDispatch->Register(SwDispatch, \
                                  EnableAcpiMode, \
                                  &AcpiEnableContext, \
                                  &Handle);
    if(EFI_ERROR(Status)) return Status;

    Status = pSmst->SmmInstallProtocolInterface(
                 &DummyHandle,
                 &gEfiAcpiEnDispatchProtocolGuid,
                 EFI_NATIVE_INTERFACE,
                 &gEfiAcpiEnDispatchProtocol
             );
    if(EFI_ERROR(Status)) return Status;

    Status = SwDispatch->Register(SwDispatch, \
                                  DisableAcpiMode, \
                                  &AcpiDisableContext,\
                                  &Handle);
    if(EFI_ERROR(Status)) return Status;

    Status = pSmst->SmmInstallProtocolInterface(
                 &DummyHandle,
                 &gEfiAcpiDisDispatchProtocolGuid,
                 EFI_NATIVE_INTERFACE,
                 &gEfiAcpiDisDispatchProtocol
             );

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   AcpiModeEnableInit
//
// Description: This function is the entry point for the ACPI mode enable/disable
//              driver.  This function is called twice: first time by the
//              DXE dispatcher and the next time when it is loaded into the
//              SMRAM
//
// Input:       IN  EFI_HANDLE ImageHandle - Handle for this FFS image
//              IN  EFI_SYSTEM_TABLE *SystemTable - Pointer to the system table
//
// Output:      EFI_STATUS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
AcpiModeEnableInit(
    IN  EFI_HANDLE              ImageHandle,
    IN  EFI_SYSTEM_TABLE        *SystemTable
)
{
    InitAmiLib(ImageHandle,SystemTable);

    return InitSmmHandler(ImageHandle,
                          SystemTable,
                          AcpiModeEnableInSmmFunction,
                          NULL);
}

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
