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
// Revision History
// ----------------
// $Log: $
// 
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:        CrbSmi.c
//
// Description: This file contains code for all CRB SMI events
//
//<AMI_FHDR_END>
//*************************************************************************

//---------------------------------------------------------------------------
// Include(s)
//---------------------------------------------------------------------------

#include <PiSmm.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/DebugLib.h>

#include <token.h>
#include <AmiDxeLib.h> // Optional. should use Mde Library instead.
#include <AmiCspLib.h> // Optional. should use Mde Library instead.

// Produced Protocols

// Consumed Protocols
#include <Protocol/SmmBase2.h>
#include <Protocol/S3SmmSaveState.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/SmmSxDispatch2.h>
#include <Protocol/SmmPeriodicTimerDispatch2.h>
#include <Protocol/SmmUsbDispatch2.h>
#include <Protocol/SmmGpiDispatch2.h>
#include <Protocol/SmmStandbyButtonDispatch2.h>
#include <Protocol/SmmPowerButtonDispatch2.h>
#include <Protocol/SmmIoTrapDispatch2.h>

//---------------------------------------------------------------------------
// Constant, Macro and Type Definition(s)
//---------------------------------------------------------------------------
// Constant Definition(s)

// Macro Definition(s)

#define CRB_GPI_SMI_TEST            1 // switch to turn on GPI smi test.
#if CRB_GPI_SMI_TEST
	//
	// from PI1.2 VOLUME 4:
	// 	6.6 SMM General Purpose Input (GPI) Dispatch Protocol
	//  we should use bits mask as GpiNum.
	//
	// this macro is for compatible some Aptio-4 component to use index base.
	//
    #if GPI_DISPATCH_BY_BITMAP == 1
        #define MACRO_CONVER_TO_GPI(x)     BIT##x
    #else
        #define MACRO_CONVER_TO_GPI(x)     x
    #endif
#else
    #define MACRO_CONVER_TO_GPI(x)     x
#endif

// Type Definition(s)

// Function Prototype(s)

//---------------------------------------------------------------------------
// Variable and External Declaration(s)
//---------------------------------------------------------------------------
// Variable Declaration(s)


// GUID Definition(s)

// Protocol Definition(s)

// External Declaration(s)

// Function Definition(s)

//---------------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   GetCrbSmiContext
//
// Description: This is a template CRB SMI GetContext for Porting.
//
// Input:       None
//
// Output:      None
//
// Notes:       Here is the control flow of this function:
//              1. Check if CRB Smi source.
//              2. If yes, return TRUE.
//              3. If not, return FALSE.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN
GetCrbSmiContext (
  VOID
  )
{
    // Porting if needed
    return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CrbSmiHandler
//
// Description: This is a template CRB SMI Handler for Porting.
//
// Input:       None
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
CrbSmiHandler (
  VOID
  )
{
    // Porting if needed
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CrbSwSmiHandler
//
// Description: This is a template CRB software SMI Handler for Porting.
//
// Input:       DispatchHandle  - EFI Handle
//              DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH2_CONTEXT
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
CrbSwSmiHandler (
  IN EFI_HANDLE	DispatchHandle,
  IN CONST VOID	*Context OPTIONAL,
  IN OUT VOID  	*CommBuffer OPTIONAL,
  IN OUT UINTN 	*CommBufferSize OPTIONAL
  )
{
	EFI_STATUS						Status = EFI_SUCCESS;
	EFI_SMM_SW_REGISTER_CONTEXT  	*DispatchContext = (EFI_SMM_SW_REGISTER_CONTEXT*)Context;

	// Porting if needed

    if (DispatchContext->SwSmiInputValue == CRB_SWSMI) {

    }

    {
		//
		// following items is Aptio-4 compatible.
		// We should use Mde library instead.
		//   only one reason to use this library is for get EFI_RUNTIME_SERVICES which located in SMM.
		//
    	EFI_TIME                    Time;
    	TRACE((-1,"<< CrbSwSmiHandler 001 %x >>\n",pRS->GetTime));
    	pRS->GetTime(&Time, NULL);
    	TRACE((-1,"<< CrbSwSmiHandler Year(%x) Month(%x) Day(%x) Hour(%x) >>\n"
    		,Time.Year
			,Time.Month
			,Time.Day
			,Time.Hour
    		));
    	pRS->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    }

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CrbSxSmiHandler
//
// Description: This is a template CRB Sx SMI Handler for Porting.
//
// Input:       DispatchHandle  - EFI Handle
//              DispatchContext - Pointer to the EFI_SMM_SX_DISPATCH_CONTEXT
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
CrbSxSmiHandler (
  IN EFI_HANDLE	DispatchHandle,
  IN CONST VOID	*Context OPTIONAL,
  IN OUT VOID  	*CommBuffer OPTIONAL,
  IN OUT UINTN 	*CommBufferSize OPTIONAL
  )
{
	EFI_STATUS						Status = EFI_SUCCESS;
	EFI_SMM_SX_REGISTER_CONTEXT  	*DispatchContext = (EFI_SMM_SX_REGISTER_CONTEXT*)Context;

    // Porting if needed

	return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CrbGpiSmiHandler
//
// Description: This is a template CRB Gpi SMI Handler for Porting.
//
// Input:       DispatchHandle  - EFI Handle
//              DispatchContext - Pointer to the EFI_SMM_GPI_DISPATCH2_CONTEXT
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
CrbGpiSmiHandler (
  IN EFI_HANDLE	DispatchHandle,
  IN CONST VOID	*Context OPTIONAL,
  IN OUT VOID  	*CommBuffer OPTIONAL,
  IN OUT UINTN 	*CommBufferSize OPTIONAL
  )
{
	EFI_STATUS						Status = EFI_SUCCESS;
	EFI_SMM_GPI_REGISTER_CONTEXT  	*DispatchContext = (EFI_SMM_GPI_REGISTER_CONTEXT*)Context;

    // Porting if needed

    // For GPI 5 for PCIE express card
    if (DispatchContext->GpiNum == MACRO_CONVER_TO_GPI(5)) {

    }

    if (DispatchContext->GpiNum == MACRO_CONVER_TO_GPI(23)) {

    }

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CrbChildDispatcher
//
// Description: This is an entry for CRB SMM Child Dispatcher Handler.
//
// Input:       SmmImageHandle       - EFI Handler
//              *CommunicationBuffer - OPTIONAL
//              *SourceSize          - OPTIONAL
//
// Output:      EFI_STATUS
//                  EFI_HANDLER_SUCCESS
//
// Referrals:   GetCrbSmiContext, CrbSmiHandler
//
// Notes:       Here is the control flow of this function:
//              1. Read SMI source status registers.
//              2. If source, call handler.
//              3. Repeat #2 for all sources registered.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
CrbChildDispatcher (
  IN EFI_HANDLE	DispatchHandle,
  IN CONST VOID	*Context OPTIONAL,
  IN OUT VOID 	*CommBuffer OPTIONAL,
  IN OUT UINTN 	*CommBufferSize OPTIONAL
  )
{
	EFI_STATUS						Status = EFI_SUCCESS;

    if (GetCrbSmiContext()) CrbSmiHandler();

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CrbDummyFunction
//
// Description: This function is Aptio-4 compatible. not required no more.
// 					We should use Mde library instead.
//
// Input:       ImageHandle - Image handle
//              *SystemTable - Pointer to the system table
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
EFIAPI
CrbDummyFunction (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable )
{
	EFI_STATUS           	Status = EFI_SUCCESS;
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CrbSmiInit
//
// Description: Installs CRB SMM Child Dispatcher Handler.
//
// Input:       ImageHandle - Image handle
//              *SystemTable - Pointer to the system table
//
// Output:      EFI_STATUS
//                  EFI_NOT_FOUND - The SMM Base protocol is not found.
//                  EFI_SUCCESS   - Installs CRB SMM Child Dispatcher Handler
//                                  successfully.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
EFIAPI
CrbSmiInit (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
    EFI_STATUS                		Status;
	EFI_SMM_SW_DISPATCH2_PROTOCOL  	*SwDispatch = NULL;
	EFI_SMM_SX_DISPATCH2_PROTOCOL  	*SxDispatch = NULL;
	EFI_SMM_GPI_DISPATCH2_PROTOCOL 	*GpiDispatch = NULL;
    EFI_SMM_SW_REGISTER_CONTEXT     SwContext = {CRB_SWSMI};
    EFI_SMM_SX_REGISTER_CONTEXT     SxContext = {SxS3, SxEntry};
    EFI_SMM_GPI_REGISTER_CONTEXT    GpiContext = {MACRO_CONVER_TO_GPI(5)}; // _L05 default for PCIExpress card
    EFI_HANDLE                      Handle = NULL;

    Status  = gSmst->SmmLocateProtocol (&gEfiSmmSwDispatch2ProtocolGuid, NULL, &SwDispatch);
    ASSERT_EFI_ERROR(Status);
    if (!EFI_ERROR(Status)) {
        Status  = SwDispatch->Register( SwDispatch, \
                                         CrbSwSmiHandler, \
                                         &SwContext, \
                                         &Handle );
        ASSERT_EFI_ERROR(Status);
    }

    Status  = gSmst->SmmLocateProtocol (&gEfiSmmSxDispatch2ProtocolGuid, NULL, &SxDispatch);
    ASSERT_EFI_ERROR(Status);
    if (!EFI_ERROR(Status)) {
        Status  = SxDispatch->Register( SxDispatch, \
                                         CrbSxSmiHandler, \
                                         &SxContext, \
                                         &Handle );
    }

#if CRB_GPI_SMI_TEST
    Status  = gSmst->SmmLocateProtocol (&gEfiSmmGpiDispatch2ProtocolGuid, NULL, &GpiDispatch);
    ASSERT_EFI_ERROR(Status);
    if (!EFI_ERROR(Status)) {
        Status  = GpiDispatch->Register( GpiDispatch, \
                                         CrbGpiSmiHandler, \
                                         &GpiContext, \
                                         &Handle );
        ASSERT_EFI_ERROR(Status);

        GpiContext.GpiNum = MACRO_CONVER_TO_GPI(23);
        Status  = GpiDispatch->Register( GpiDispatch, \
                                         CrbGpiSmiHandler, \
                                         &GpiContext, \
                                         &Handle );
        ASSERT_EFI_ERROR(Status);
    }
#endif

    Handle = NULL;
    //Register call backs
    Status = gSmst->SmiHandlerRegister(CrbChildDispatcher, NULL, &Handle);


    //
    // locate S3 Save protocol.
    //
    //### 5.004 not support yet ###Status = gSmst->SmmLocateProtocol (&gEfiS3SmmSaveStateProtocolGuid, \
    //### 5.004 not support yet ###                              NULL, \
    //### 5.004 not support yet ###                              &gBootScriptSave );
    //### 5.004 not support yet ###ASSERT_EFI_ERROR(Status);



    //
    // following items is Aptio-4 compatible.
    // We should use Mde library instead.
    //   only one reason to use this library is for get EFI_RUNTIME_SERVICES which located in SMM.
    //
    InitAmiLib(ImageHandle, SystemTable);
    Status = InitSmmHandler(ImageHandle, SystemTable, CrbDummyFunction, NULL);

    return Status;
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
