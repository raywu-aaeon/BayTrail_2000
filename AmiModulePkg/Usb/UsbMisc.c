//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2009, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************

//****************************************************************************
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/usbmisc.c 22    8/29/12 8:37a Ryanchou $
//
// $Revision: 22 $
//
// $Date: 8/29/12 8:37a $
//****************************************************************************


//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
//  Name:           UsbMisc.c
//
//  Description:    AMI USB driver miscellaneous routines
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <Efi.h>

#include "AmiDef.h"
#include "UsbDef.h"
#include "Uhcd.h"

#if USB_RUNTIME_DRIVER_IN_SMM
#include <Protocol/SmmControl2.h>
EFI_SMM_CONTROL2_PROTOCOL *gSmmCtl = NULL;
#endif

BOOLEAN gFirstCall = TRUE;
VOID *gGlobalPointer;
VOID *gStartPointer;
VOID *gEndPointer;

extern USB_GLOBAL_DATA             *gUsbData;
extern EFI_USB_PROTOCOL            *gAmiUsbController;

#if USB_RUNTIME_DRIVER_IN_SMM
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        USBGenerateSWSMI
//
// Description: Generates SW SMI using global SmmCtl pointer.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBGenerateSWSMI (
    UINT8   Data
)
{
					                            //(EIP57354)>
	EFI_STATUS Status;
	UINT8 SwSmiValue = Data;
	UINT8 DataSize = 1;

	if (gSmmCtl == NULL) {
	    Status = gBS->LocateProtocol(&gEfiSmmControl2ProtocolGuid, NULL, &gSmmCtl);
		if (EFI_ERROR(Status)){
			return;
		}
	}
					                            //<(EIP57354)
    gSmmCtl->Trigger(gSmmCtl, &SwSmiValue, &DataSize, 0, 0);
}
#endif

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        InvokeUsbApi
//
// Description: 
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
InvokeUsbApi(
	URP_STRUC *Urp
)
{
#if	USB_RUNTIME_DRIVER_IN_SMM
	UINTN		Temp;

	Temp = (UINTN)gUsbData->fpURP;
	gUsbData->fpURP = Urp;

	USBGenerateSWSMI (USB_SWSMI);

	gUsbData->fpURP = (URP_STRUC*)Temp;
#else
    EFI_TPL               OldTpl;
    OldTpl = gBS->RaiseTPL(TPL_NOTIFY);

	if (gAmiUsbController->UsbInvokeApi) {
		gAmiUsbController->UsbInvokeApi(Urp);
	}
	gBS->RestoreTPL(OldTpl);
#endif
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AlignPhysicalAddress
//
// Description: Returns the aligned address.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINTN
AlignPhysicalAddress (
    UINTN   PhyAddress,
    UINT16  AlignSize,
    UINT32  TotalSize)
{
    UINTN   AlignAddr;

    USB_DEBUG(DEBUG_LEVEL_7, "Un-aligned address : %lX\n", PhyAddress);
    if ((PhyAddress % AlignSize) != 0) {
        AlignAddr = PhyAddress - (PhyAddress % (UINT32)AlignSize) + AlignSize;
    }
    else {
        AlignAddr = PhyAddress;
    }
    USB_DEBUG(DEBUG_LEVEL_7, "Aligned address : %lX\n", AlignAddr);

    return AlignAddr;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AllocAlignedMemory
//
// Description: Allocates memory with the given alignment.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID*
AllocAlignedMemory (
    UINT32 AllocSize,
    UINT16 Align
)
{
    UINTN   Ptr;
    UINT32  Size;
    EFI_STATUS  Status;
    EFI_PHYSICAL_ADDRESS    MemAddress;

    if (AllocSize == 0) return NULL;
    //
    // If this is the first time the function is called,
    // allocate the USB memory and make the size 4K aligned (VTD).
    //
    if(gFirstCall) {
        gFirstCall = FALSE;                 // Make sure to only allocate once.
        Size = CalculateMemorySize();       // Determine total required size.
        Size = (Size + 0x1000) >> 12;       // Express Size in pages.
        //
        // Allocate and zero memory in pages.
        //
        MemAddress = 0xFFFFFFFF;
        Status = gBS->AllocatePages(AllocateMaxAddress, EfiRuntimeServicesData,
                Size, &MemAddress);
        ASSERT_EFI_ERROR(Status);

        gGlobalPointer = (VOID*)(UINTN)MemAddress;
        gBS->SetMem (gGlobalPointer, (Size << 12), 0);
        //
        // Save pointers to beginning and end of region.
        //
        gStartPointer = gGlobalPointer;
        gEndPointer = (VOID *)((UINTN)gGlobalPointer + (Size << 12) - 1);
    }

    //USB_DEBUG(DEBUG_LEVEL_6, "Unaligned : %Fp, %X, %X\n", gGlobalPointer, AllocSize, Align);
    Ptr  = AlignPhysicalAddress( (UINTN)gGlobalPointer, Align, AllocSize);
    //USB_DEBUG(DEBUG_LEVEL_6, "Aligned : %Fp, %X, %X\n", Ptr, AllocSize, Align);

    gGlobalPointer = (VOID*)(Ptr + AllocSize);

    if (gGlobalPointer < gEndPointer)
    {
        return (VOID*)Ptr;
    }
    return NULL;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AllocateHcMemory
//
// Description: Allocates a number of pages with the given alignment.
//
// Note:        The minimum alignment passed to this function is CPU page, 4K
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID *
AllocateHcMemory (
	IN EFI_PCI_IO_PROTOCOL	*PciIo,
    IN UINTN            	Pages,
    IN UINTN            	Alignment
)
{
    EFI_STATUS            Status;
    VOID                  *Memory;
    UINTN                 AlignedMemory;
    UINTN                 AlignmentMask;
    UINTN                 UnalignedPages;
    UINTN                 RealPages;

    //
    // Alignment must be a power of two or zero.
    //
    ASSERT ((Alignment & (Alignment - 1)) == 0);
 
    if (Pages == 0) {
        return NULL;
    }
    if (Alignment > EFI_PAGE_SIZE) {
        //
        // Caculate the total number of pages since alignment is larger than page size.
        //
        AlignmentMask  = Alignment - 1;
        RealPages      = Pages + EFI_SIZE_TO_PAGES (Alignment);
        //
        // Make sure that Pages plus EFI_SIZE_TO_PAGES (Alignment) does not overflow.
        //
        ASSERT (RealPages > Pages);

 		Memory = (VOID*)0xFFFFFFFF;
        Status = PciIo->AllocateBuffer (PciIo, AllocateMaxAddress, EfiRuntimeServicesData, RealPages,
        			&Memory, 0);
        if (EFI_ERROR (Status)) {
            return NULL;
        }
        AlignedMemory  = ((UINTN) Memory + AlignmentMask) & ~AlignmentMask;
        UnalignedPages = EFI_SIZE_TO_PAGES (AlignedMemory - (UINTN) Memory);
        if (UnalignedPages > 0) {
            //
            // Free first unaligned page(s).
            //
            Status = PciIo->FreeBuffer(PciIo, UnalignedPages, Memory);
            ASSERT_EFI_ERROR (Status);
        }
        Memory         = (VOID*)(AlignedMemory + EFI_PAGES_TO_SIZE (Pages));
        UnalignedPages = RealPages - Pages - UnalignedPages;
        if (UnalignedPages > 0) {
            //
            // Free last unaligned page(s).
            //
			Status = PciIo->FreeBuffer(PciIo, UnalignedPages, Memory);
            ASSERT_EFI_ERROR (Status);
        }
    } else {
        //
        // Do not over-allocate pages in this case.
        //
        Memory = (VOID*)0xFFFFFFFF;
		Status = PciIo->AllocateBuffer (PciIo, AllocateMaxAddress, EfiRuntimeServicesData, Pages,
					 &Memory, 0);
        if (EFI_ERROR (Status)) {
            return NULL;
        }
        AlignedMemory  = (UINTN) Memory;
    }
    return (VOID*) AlignedMemory;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        FreeHcMemory
//
// Description: Free the memory allocated by AllocateHcMemory().
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
FreeHcMemory(
	IN EFI_PCI_IO_PROTOCOL	*PciIo,
	IN UINTN				Pages,
	IN VOID*				Memory
)
{
	EFI_STATUS            Status;

	Status = PciIo->FreeBuffer(PciIo, Pages, Memory);
	ASSERT_EFI_ERROR(Status);
	return;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        ReallocateMemory
//
// Description: 
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID *
ReallocateMemory (
  IN UINTN  OldSize,
  IN UINTN  NewSize,
  IN VOID   *OldBuffer  OPTIONAL
)
{
	EFI_STATUS  Status;
    VOID        *NewBuffer = NULL;

	Status = gBS->AllocatePool (EfiRuntimeServicesData, NewSize, &NewBuffer);
    if (EFI_ERROR(Status)) {
        return NULL;
    }

    gBS->SetMem(NewBuffer, NewSize, 0);

    if (OldSize > 0 && OldBuffer != NULL && NewBuffer != NULL) {
        gBS->CopyMem(NewBuffer, OldBuffer, (OldSize < NewSize) ? OldSize : NewSize);
        gBS->FreePool(OldBuffer);
    }
    return NewBuffer;
}

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2009, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
