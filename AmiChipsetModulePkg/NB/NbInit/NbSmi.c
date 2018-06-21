//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
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
//
// Name:        NbSmi.c
//
// Description: This file contains code for all North Bridge SMI events
//
//<AMI_FHDR_END>
//*************************************************************************

//----------------------------------------------------------------------------
// Include(s)
//----------------------------------------------------------------------------

#include <Token.h>
#include <AmiDxeLib.h>
#include <AmiCspLib.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/SmmSxDispatch2.h>
//EIP150350 >>
#if CSM_SUPPORT
#include <Library/NbPolicy.h>
#include <Protocol/SmmCpu.h>
#endif 
//EIP150350 <<

//----------------------------------------------------------------------------
// Constant, Macro and Type Definition(s)
//----------------------------------------------------------------------------
// Constant Definition(s)

#define AMI_SMM_SW_DISPATCH_PROTOCOL EFI_SMM_SW_DISPATCH2_PROTOCOL
#define AMI_SMM_SW_DISPATCH_CONTEXT  EFI_SMM_SW_REGISTER_CONTEXT
#define AMI_SMM_SX_DISPATCH_PROTOCOL EFI_SMM_SX_DISPATCH2_PROTOCOL
#define AMI_SMM_SX_DISPATCH_CONTEXT  EFI_SMM_SX_REGISTER_CONTEXT
#define SMM_CHILD_DISPATCH_SUCCESS   EFI_SUCCESS

// Macro Definition(s)

// Type Definition(s)

// Function Prototype(s)

//----------------------------------------------------------------------------
// Variable and External Declaration(s)
//----------------------------------------------------------------------------
// Variable Declaration(s)
//EIP150350 >>
#if CSM_SUPPORT
NB_SETUP_DATA           *gNbSetupData = NULL;
EFI_SMM_CPU_PROTOCOL    *gSmmCpu = NULL;
#endif
//EIP150350 <<

// GUID Definition(s)

// Protocol Definition(s)

// External Declaration(s)

// Function Definition(s)

//----------------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   GetNbSmiContext
//
// Description: This is a template NB SMI GetContext for Porting.
//
// Input:       None
//
// Output:      BOOLEAN
//
// Notes:       Here is the control flow of this function:
//              1. Check if NB Smi source.
//              2. If yes, return TRUE.
//              3. If not, return FALSE.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN GetNbSmiContext(VOID)
{
    return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NbSmiHandler
//
// Description: This is a template NB SMI Handler for Porting.
//
// Input:       None
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID NbSmiHandler(VOID)
{

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NbSwSmiHandler
//
// Description: This is a template NB software SMI Handler for Porting.
//
// Input:       PI 0.91, 1.0
//                  DispatchHandle   - SMI dispatcher handle
//                  *DispatchContext - Pointer to the dispatch context
//              PI 1.1, 1.2
//                  DispatchHandle  - SMI dispatcher handle
//                  *DispatchContext- Points to an optional S/W SMI context
//                  CommBuffer      - Points to the optional communication
//                                    buffer
//                  CommBufferSize  - Points to the size of the optional
//                                    communication buffer
//
// Output:      EFI_STATUS if the new SMM PI is applied.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS NbSwSmiHandler(
    IN EFI_HANDLE       DispatchHandle,
    IN CONST VOID       *DispatchContext OPTIONAL,
    IN OUT VOID         *CommBuffer OPTIONAL,
    IN OUT UINTN        *CommBufferSize OPTIONAL)
{
//EIP150350 >>
#if CSM_SUPPORT
    EFI_STATUS  Status = EFI_UNSUPPORTED;    
    UINTN       Cpu = (UINTN)-1;
    UINT16      RegBx;    

    if (CommBuffer != NULL && CommBufferSize != NULL) {
      Cpu = ((EFI_SMM_SW_CONTEXT*)CommBuffer)->SwSmiCpuIndex;
    }
    
    if(Cpu == (UINTN)-1) return Status;
    
    Status = gSmmCpu->ReadSaveState (gSmmCpu,
                                     2,
                                     EFI_SMM_SAVE_STATE_REGISTER_RBX,
                                     Cpu,
                                     &RegBx);
    if(EFI_ERROR(Status)) return Status;

    switch (RegBx & 0xff) {
        case IGFX_LCD_PANEL_TYPE:  // 0x80
             RegBx = (UINT16)gNbSetupData->LcdPanelType;
             break;
        case IGFX_LCD_PANEL_SCALING: // 0x81
             RegBx = (UINT16)gNbSetupData->LcdPanelScaling;
             break;
        case IGFX_BOOT_TYPE: // 0x82
             RegBx = (UINT16)gNbSetupData->IgdBootType;
             break;
        case IGFX_BACKLIGHT_TYPE: // 0x83
             RegBx = (UINT16)gNbSetupData->IgdLcdBlc;
             break;
//        case IGFX_LFP_PANEL_COLOR_DEPTH_TYPE: // 0x84
//             RegBx = (UINT16)gNbSetupData->LfpColorDepth;
             break;
        case IGFX_EDP_ACTIVE_LFP_CONFIG_TYPE: // 0x85
             RegBx = (UINT16)gNbSetupData->ActiveLFP;
             break;
        case IGFX_PRIMARY_DISPLAY_TYPE: // 0x86
             RegBx = (UINT16)gNbSetupData->PrimaryDisplay;
             break;
        case IGFX_DISPLAY_PIPE_B_TYPE: // 0x87
             RegBx = (UINT16)gNbSetupData->DisplayPipeB;
             break;
        case IGFX_SDVO_PANEL_TYPE: // 0x88
             RegBx = (UINT16)gNbSetupData->SdvoPanelType;
             break;
        default:
             RegBx = 0;
             break;
    } // switch
    
    Status = gSmmCpu->WriteSaveState(gSmmCpu, 
                                     2,
                                     EFI_SMM_SAVE_STATE_REGISTER_RBX, 
                                     Cpu, 
                                     &RegBx);    
    return Status;   
#else
//EIP150350 <<
//    WRITE_IO8(0x80, NB_SWSMI);
	return SMM_CHILD_DISPATCH_SUCCESS;
#endif //EIP150350
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NbSxSmiHandler
//
// Description: This is a template NB Sx SMI Handler for Porting.
//
// Input:       PI 0.91, 1.0
//                  DispatchHandle   - SMI dispatcher handle
//                  *DispatchContext - Pointer to the dispatch context
//              PI 1.1, 1.2
//                  DispatchHandle  - SMI dispatcher handle
//                  *DispatchContext- Points to an optional Sx SMI context
//                  CommBuffer      - Points to the optional communication
//                                    buffer
//                  CommBufferSize  - Points to the size of the optional
//                                    communication buffer
//
// Output:      EFI_STATUS if the new SMM PI is applied.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS NbSxSmiHandler(
    IN EFI_HANDLE       DispatchHandle,
    IN CONST VOID       *DispatchContext OPTIONAL,
    IN OUT VOID         *CommBuffer OPTIONAL,
    IN OUT UINTN        *CommBufferSize OPTIONAL)
{
    /*
        // SMBAVUMA Workaround
        WRITE_IO8(0x3c4, 0x01);
        SET_IO8(0x3c5, 0x20);
    */
    return SMM_CHILD_DISPATCH_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NbChildDispatcher
//
// Description: North Bridge SMM Child Dispatcher Handler.
//
// Input:       PI 0.91, 1.0
//                  SmmImageHandle       - SMI Image Hander
//                  *CommunicationBuffer - Pointer to optional communication
//                                         buffer
//                  *SourceSize          - Pointer to size of communication
//                                         buffer
//              PI 1.1, 1.2
//                  DispatchHandle  - SMI dispatcher handle
//                  *DispatchContext- Pointer to the dispatched context
//                  CommBuffer      - Pointer to a collection of data in
//                                    memory that will be conveyed from a
//                                    non-SMM environment into an SMM
//                                    environment
//                  CommBufferSize  - Pointer to the size of the CommBuffer
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:   EfiSmmSwDispatch EfiSmmSxDispatch
//
// Notes:       Here is the control flow of this function:
//              1. Read SMI source status registers.
//              2. If source, call handler.
//              3. Repeat #2 for all sources registered.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS NbChildDispatcher(
    IN EFI_HANDLE       DispatchHandle,
    IN CONST VOID       *DispatchContext OPTIONAL,
    IN OUT VOID         *CommBuffer OPTIONAL,
    IN OUT UINTN        *CommBufferSize OPTIONAL)
{
    if(GetNbSmiContext()) NbSmiHandler();

    return EFI_HANDLER_SUCCESS;
}

//----------------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   InSmmFunction
//
// Description: Installs North Bridge SMM Child Dispatcher Handler.
//
// Input:       ImageHandle  - Image handle
//              *SystemTable - Pointer to the system table
//
// Output:      EFI_STATUS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS InSmmFunction(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable)
{
    EFI_STATUS                   Status;
    AMI_SMM_SW_DISPATCH_PROTOCOL *pSwDispatch;
    AMI_SMM_SX_DISPATCH_PROTOCOL *pSxDispatch;
    AMI_SMM_SW_DISPATCH_CONTEXT  SwContext = {NB_SWSMI};
    AMI_SMM_SX_DISPATCH_CONTEXT  SxContext = {SxS3, SxEntry};
    EFI_HANDLE                   Handle;
    EFI_HANDLE                   RootHandle;
//EIP150350 >>
#if CSM_SUPPORT	
    UINTN                        VariableSize = sizeof(NB_SETUP_DATA);
#endif
//EIP150350 <<
    Status = InitAmiSmmLib(ImageHandle, SystemTable);
    if(EFI_ERROR(Status)) return Status;

//EIP150350 >>
#if CSM_SUPPORT
    Status = pSmst->SmmAllocatePool(EfiRuntimeServicesData, VariableSize, &gNbSetupData);
    if (!EFI_ERROR (Status)){
        GetNbSetupData(pRS, gNbSetupData, FALSE); 
    } else {
        gNbSetupData->LcdPanelType = 0;
        gNbSetupData->LcdPanelScaling = 0;
        gNbSetupData->IgdBootType = 0;
        gNbSetupData->IgdLcdBlc = 0;        
        //gNbSetupData->LfpColorDepth = 0;
        gNbSetupData->ActiveLFP = 1;
        gNbSetupData->PrimaryDisplay = 3;        
        gNbSetupData->DisplayPipeB = 0;        
        gNbSetupData->SdvoPanelType = 0;
    }
    
    Status = pSmst->SmmLocateProtocol(&gEfiSmmCpuProtocolGuid, NULL, &gSmmCpu);
    if (EFI_ERROR(Status)) return Status;
#endif
//EIP150350 <<
    
    Status  = pSmst->SmmLocateProtocol(&gEfiSmmSwDispatch2ProtocolGuid, \
                                       NULL, \
                                       &pSwDispatch);
    if(!EFI_ERROR(Status)) {
        if (MmioRead32(IGD_REG(0)) != 0xFFFFFFFF) { //EIP150350
        	Status  = pSwDispatch->Register(pSwDispatch, \
                                        	NbSwSmiHandler, \
                                        	&SwContext, \
                                        	&Handle);
        } //EIP150350
    }

    Status  = pSmst->SmmLocateProtocol(&gEfiSmmSxDispatch2ProtocolGuid, \
                                       NULL, \
                                       &pSxDispatch);
    if(!EFI_ERROR(Status)) {
        Status  = pSxDispatch->Register(pSxDispatch, \
                                        NbSxSmiHandler, \
                                        &SxContext, \
                                        &Handle);
    }

    Status  = pSmst->SmiHandlerRegister(NbChildDispatcher, \
                                        NULL, \
                                        &RootHandle);

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   InitializeNbSmm
//
// Description: Installs North Bridge SMM Child Dispatcher Handler.
//
// Input:
//  EFI_HANDLE ImageHandle - Image handle
//  EFI_SYSTEM_TABLE *SystemTable - Pointer to the system table
//
// Output:      EFI_STATUS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS InitializeNbSmm(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable)
{
    InitAmiLib(ImageHandle, SystemTable);
    return InitSmmHandler(ImageHandle, SystemTable, InSmmFunction, NULL);
}

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
