//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
/** @file
  @brief Source code for the AmiBufferValidationLib library class.

  Buffer Validation Function source code.
**/
#include <Protocol/SmmAccess2.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>

/// Internal list of SMRAM regions
EFI_SMRAM_DESCRIPTOR *SmmAmiBufferValidationLibSmramRanges = NULL;

/// Number of SMRAM regions in the internal list
UINTN SmmAmiBufferValidationLibNumberOfSmramRange = 0;

/**
    Validates memory buffer.
    
    Function verifies the buffer to make sure its address range is legal for a memory buffer. A legal memory 
    buffer is one that lies entirely outside of SMRAM.  SMI handlers that receive buffer address and/or size 
    from outside of SMM at runtime must validate the buffer using this function prior to using it or passing 
    to other SMM interfaces.

    @param  Buffer Buffer address 
    @param  BufferSize Size of the Buffer
    
    @retval  EFI_SUCCESS - The buffer address range is valid and can be safely used.
    @retval  EFI_ACCESS_DENIED - The buffer can't be used because its address range overlaps with a protected area such as SMRAM.
    @retval  EFI_INVALID_PARAMETER - The buffer can't be used because its address range is invalid.
    @retval  EFI_NOT_FOUND - The buffer can't be used because its validity cannot be verified. Normally due to the SMRAM ranges were not available.
**/
EFI_STATUS AmiValidateMemoryBuffer(VOID* Buffer, UINTN BufferSize){
	UINTN  i;
	UINTN BufferAddress;

	if (SmmAmiBufferValidationLibNumberOfSmramRange==0) return EFI_NOT_FOUND;

	BufferAddress = (UINTN)Buffer;
// [ EIP288953, EIP306158+> ]
//	if (BufferAddress + BufferSize < BufferAddress) return EFI_INVALID_PARAMETER; // overflow
	if (BufferAddress - 1 + BufferSize < BufferAddress) return EFI_INVALID_PARAMETER; // overflow
// [ EIP288953, EIP306158+< ]
	for (i = 0; i < SmmAmiBufferValidationLibNumberOfSmramRange; i ++) {
		if (    BufferAddress >= SmmAmiBufferValidationLibSmramRanges[i].CpuStart
    	     && BufferAddress < SmmAmiBufferValidationLibSmramRanges[i].CpuStart + SmmAmiBufferValidationLibSmramRanges[i].PhysicalSize
    	) return EFI_ACCESS_DENIED; // Buffer starts in SMRAM
        if (    BufferAddress < SmmAmiBufferValidationLibSmramRanges[i].CpuStart
    	     && BufferAddress+BufferSize > SmmAmiBufferValidationLibSmramRanges[i].CpuStart
        ) return EFI_ACCESS_DENIED; // Buffer overlaps with SMRAM
	}
	
	return EFI_SUCCESS;
}

/**
    Validates MMIO buffer.
    
    Function verifies the buffer to make sure its address range is legal for a MMIO buffer.  A legal MMIO buffer is one that lies 
    entirely outside of SMRAM.  SMI handlers that receive a buffer address and/or size from outside of SMM at runtime must validate 
    the buffer using this function prior to using the MMIO Buffer or passing to other SMM interfaces.

    @param  Buffer Buffer address 
    @param  BufferSize Size of the Buffer
    
    @retval  EFI_SUCCESS - The buffer address range is valid and can be safely used.
    @retval  EFI_ACCESS_DENIED - The buffer can't be used because its address range overlaps with a protected area such as SMRAM.
    @retval  EFI_INVALID_PARAMETER - The buffer can't be used because its address range is invalid.
    @retval  EFI_NOT_FOUND - The buffer can't be used because its validity cannot be verified. Normally due to the SMRAM ranges were not available.
**/
EFI_STATUS AmiValidateMmioBuffer(VOID* Buffer, UINTN BufferSize){
	return AmiValidateMemoryBuffer(Buffer,BufferSize);
}

/**
    Validates SMRAM buffer.
    
    Function verifies the buffer to make sure it wholly resides in the SMRAM.
    
    @param  Buffer Buffer address 
    @param  BufferSize Size of the Buffer
    
    @retval  EFI_SUCCESS - The buffer resides in the SMRAM and can be safely used.
    @retval  EFI_ACCESS_DENIED - The buffer can't be used because at least one byte of the buffer is outside of SMRAM.
    @retval  EFI_INVALID_PARAMETER - The buffer can't be used because its address range is invalid.
    @retval  EFI_NOT_FOUND - The buffer can't be used because its validity cannot be verified. Normally due to the SMRAM ranges were not available.
**/
EFI_STATUS AmiValidateSmramBuffer(VOID* Buffer, UINTN BufferSize){
	UINTN  i;
	UINTN BufferAddress;

	if (SmmAmiBufferValidationLibNumberOfSmramRange==0) return EFI_NOT_FOUND;

	BufferAddress = (UINTN)Buffer;
//  if (BufferAddress + BufferSize < BufferAddress) return EFI_INVALID_PARAMETER; // overflow
  if (BufferAddress - 1 + BufferSize < BufferAddress) return EFI_INVALID_PARAMETER; // overflow
	for (i = 0; i < SmmAmiBufferValidationLibNumberOfSmramRange; i ++) {
		if (    BufferAddress >= SmmAmiBufferValidationLibSmramRanges[i].CpuStart
    	     && BufferAddress+BufferSize <= SmmAmiBufferValidationLibSmramRanges[i].CpuStart + SmmAmiBufferValidationLibSmramRanges[i].PhysicalSize
    	) return EFI_SUCCESS; // Entire Buffer is in SMRAM
	}
	
	return EFI_ACCESS_DENIED;
}

/**
    Library constructor for the AmiBufferValidationLib
    
    Performs the necessary initialization so that the buffer validation functions will operate 
    correctly when they are called.

    @param  ImageHandle The handle of this image
    @param  SystemTable Pointer to the EFI_SYSTEM_TABLE
    
    @retval  EFI_NOT_FOUND The Smm Access protocol could not be found
    @retval  EFI_OUT_OF_RESOURCES An allocation failed because it could not find any memory resources 
    @retval  EFI_INVALID_PARAMETER An invalid parameter was passed to one of the functions
    @retval  EFI_SUCCESS The necessary functions were initialized
**/
EFI_STATUS EFIAPI SmmAmiBufferValidationLibConstructor(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable){
    EFI_STATUS Status;
    EFI_SMM_ACCESS2_PROTOCOL *SmmAccess;
    UINTN Size;
    
    // Get SMRAM information
    Status = gBS->LocateProtocol (&gEfiSmmAccess2ProtocolGuid, NULL, (VOID **)&SmmAccess);
    if (EFI_ERROR(Status)) return Status;
    
    Size = 0;
    Status = SmmAccess->GetCapabilities (SmmAccess, &Size, NULL);
    if (Status != EFI_BUFFER_TOO_SMALL) return Status;
    Status = gSmst->SmmAllocatePool (EfiRuntimeServicesData, Size, (VOID **)&SmmAmiBufferValidationLibSmramRanges);
    if (EFI_ERROR(Status)){
    	SmmAmiBufferValidationLibSmramRanges = NULL;
    	return Status;
    }
    Status = SmmAccess->GetCapabilities (SmmAccess, &Size, SmmAmiBufferValidationLibSmramRanges);
    if (EFI_ERROR(Status)){
    	gSmst->SmmFreePool (SmmAmiBufferValidationLibSmramRanges);
    	SmmAmiBufferValidationLibSmramRanges = NULL;
    	return Status;
    }
    SmmAmiBufferValidationLibNumberOfSmramRange = Size / sizeof (EFI_SMRAM_DESCRIPTOR);

    return EFI_SUCCESS;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
