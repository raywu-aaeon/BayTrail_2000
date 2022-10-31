//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************
#ifndef PEI_BUILD

#include <AmiLib.h>

#include <Token.h>
#include <AmiDxeLib.h>
#include <Protocol/SmmBase.h> // used for SMM Malloc

#include <CryptLib.h>

//
// Global variables
//
extern EFI_BOOT_SERVICES    *pBS;
extern EFI_RUNTIME_SERVICES *pRS;


// Crypto Memory Manager heap address
/*volatile*/ UINT8 *gDstAddress = NULL;
UINTN  gHeapSize   = CR_DXE_MAX_HEAP_SIZE;

BOOLEAN   IsVirtualAddress = FALSE;

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   RuntimeCryptLibAddressChangeEvent
//
// Description:  Notification function of EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE.
//               This is a notification function registered on EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE
//               event. It converts a pointer to a new virtual address.
//
// Input:   Event      The event whose notification function is being invoked.
//          Context    The pointer to the notification function's context.
//
// Output: 
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
EFIAPI
RuntimeCryptLibAddressChangeEvent (
  IN  EFI_EVENT        Event,
  IN  VOID             *Context
  )
{
  IsVirtualAddress = TRUE;

  //  
  // Stop Crypto debug traces after switch to Virtual mode
  //
  wpa_set_trace_level(0); 
  //
  // Converts a pointer for runtime memory management to a new virtual address.
  //
TRACE((-1,"Crypto Lib Callback.\nHeap addr before Virtual MemFixup = %x, ", gDstAddress));
  pRS->ConvertPointer(0, (VOID**)&gDstAddress);
TRACE((-1,"After = %x\n", gDstAddress));

  InitCRmm((VOID*)gDstAddress, gHeapSize);

  return;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   RuntimeCryptLibConstructorInSmm
//
// Description:  Init Crypto lib internal state
//
// Input:   
//
// Output: 
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS RuntimeCryptLibConstructorInSmm(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS  Status;
    EFI_TIME  EfiTime;

    Status = pSmst->SmmAllocatePool(EfiRuntimeServicesData, gHeapSize, (void**)&gDstAddress);
    ASSERT_EFI_ERROR(Status);
    if(EFI_ERROR(Status)) 
        return Status;

    InitCRmm( (void*)gDstAddress, gHeapSize);

    Status = pST->RuntimeServices->GetTime(&EfiTime, NULL);
    if(!EFI_ERROR(Status))
        set_crypt_efitime(&EfiTime);
TRACE((-1,"Crypto Lib SMM Init %r\nCR Heap Addr = %x\n", Status, gDstAddress));

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   RuntimeCryptLibConstructor
//
// Description: This function is called from outside of SMM during SMM registration.
//
// Input:
//  IN EFI_HANDLE       ImageHandle
//  IN EFI_SYSTEM_TABLE *SystemTable
//
// Output: EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS RuntimeCryptLibConstructor(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS  Status;
    EFI_TIME  EfiTime;
/*    BOOLEAN    InSmm= FALSE;
    ////////////////////////////////////////////////////////////////////////////////////
    // Init Crypto Memory Manager
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Check to see if we are already in SMM
    //
    pSmmBase->InSmm (pSmmBase, &InSmm);
    //
    //
    //
    if (InSmm) {
        // Init Crypto mem manager to use heap inside the SMM
        Status = pSmst->SmmAllocatePool(EfiRuntimeServicesData, gHeapSize, (void**)&gDstAddress);
        ASSERT_EFI_ERROR(Status);
    EFI_DEADLOOP()
        InitCRmm( (void*)gDstAddress, gHeapSize);

        Status = pST->RuntimeServices->GetTime(&EfiTime, NULL);

        TRACE((-1,"Crypto Lib SMM Init\nCR Heap Addr = %x\n", gDstAddress));
    }
    else {
*/
// not in SMM
///////////////////////////////////////////////////////////////////////////////
//
// Init Aux Memory Manager
//
///////////////////////////////////////////////////////////////////////////////
  //
  // Pre-allocates runtime space for possible cryptographic operations
  //
        if(gDstAddress == NULL) {
            Status = pBS->AllocatePool(EfiRuntimeServicesData, gHeapSize, (void**)&gDstAddress);
            ASSERT_EFI_ERROR (Status);
            if(EFI_ERROR(Status)) return Status;
        }


        InitCRmm((void*)gDstAddress, gHeapSize);

        // !!!Bug in SBrun.c GetTime with Virtual Addressing
        Status = pRS->GetTime(&EfiTime, NULL);
    if(EFI_ERROR(Status)) 
        return Status;
    set_crypt_efitime(&EfiTime);

TRACE((-1,"Crypto Lib Init %r\nCR Heap Addr = %x\n", Status, gDstAddress));
        //
        // Create virtual address change event
        //
        InitAmiRuntimeLib(
            ImageHandle, SystemTable, NULL, RuntimeCryptLibAddressChangeEvent
        );
//    }

    return Status;
}
#endif //#ifndef PEI_BUILD
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
