//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:  		ManageShadowRam.c
//
// Description:	Installing two protocol function for other module used.
//
//<AMI_FHDR_END>
//**********************************************************************

#include <AmiDxeLib.h>
#include <Protocol/ConsoleControl.h>
#include <token.h>
#include <AmiCspLib.h>
#include <Protocol/ManageShadowProtocol.h>
#include "ShadowRamProtocol.h"
#include <RsdpPlusElink.h>	//Define in RsdpPlus.mak file.

#define BDS_ALL_DRIVERS_CONNECTED_PROTOCOL_GUID \
    {0xdbc9fd21, 0xfad8, 0x45b0, 0x9e, 0x78, 0x27, 0x15, 0x88, 0x67, 0xcc, 0x93}

//EFI_GUID gConOutStartedGuid = CONSOLE_OUT_DEVICES_STARTED_PROTOCOL_GUID;
EFI_GUID gAllDriverConnectGuid = BDS_ALL_DRIVERS_CONNECTED_PROTOCOL_GUID;	
EFI_GUID gShdowRamProtocolGuid = SHADOW_RAM_PROTOCOL_GUID;
EFI_GUID ManageShdowRamProtocolGuid = MANAGE_SHADOW_RAM_PROTOCOL_GUID;

VOID UpdateShadowBeforEfiBoot(VOID);
VOID EraseShadowAfterEfiBoot(VOID);
EFI_STATUS HeapToE000(IN UINT8 *pData, UINT32 Align, IN UINTN Length, IN OUT DATA_BUFF_STRUC	*pData2 OPTIONAL);
EFI_STATUS HeapToF000(IN UINT8 *pData, UINT32 Align, IN UINTN Length, IN OUT DATA_BUFF_STRUC	*pData2 OPTIONAL);

EFI_HANDLE	gShadowRameHandle = NULL;
EFI_HANDLE	gManageShadowRamHandle = NULL;
UINT8 	*gE000HeapPtr = NULL;
UINT8 	*gF000HeapPtr = NULL;
UINT8	*gESegStore = NULL;
UINT8	*gFSegStore = NULL;
UINT8	EsegUserCount = 0, FsegUserCount = 0;
UINT32	gE000BuffLength = 0;
UINT32	gF000BuffLength = 0;
UINT32	gBufferSize = 0x10000;


SHADOW_RAM_PROTOCOL gShadowRamProtocol = 
{
	UpdateShadowBeforEfiBoot,
	EraseShadowAfterEfiBoot
};

MANAGE_SHADOW_RAM_PROTOCOL gManageShadowRamProtocol =
{
	HeapToE000,
	HeapToF000
};

//************** Update Shadow Ram Hook support ****************************
extern UPDATE_E000_SHDOW_RAM_HOOK UPDATE_E000_SHADOW_RAM_HOOK_LIST EndOfUpdateE000ShadowRamHookList;
UPDATE_E000_SHDOW_RAM_HOOK* UpdateE000ShdowRamHookList[] = {UPDATE_E000_SHADOW_RAM_HOOK_LIST NULL};

extern UPDATE_F000_SHDOW_RAM_HOOK UPDATE_F000_SHADOW_RAM_HOOK_LIST EndOfUpdateF000ShadowRamHookList;
UPDATE_F000_SHDOW_RAM_HOOK* UpdateF000ShdowRamHookList[] = {UPDATE_F000_SHADOW_RAM_HOOK_LIST NULL};

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   OemUpdateE000ShdowRamHook
//
// Description: 
//              
//
// Input:
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID OemUpdateE000ShdowRamHook(
    IN UINT32*	pShadowRam,
	IN UINTN	UsageLength
)
{
    UINTN i;
    
    for (i = 0; UpdateE000ShdowRamHookList[i] != NULL; i++) 
        UpdateE000ShdowRamHookList[i](pShadowRam,UsageLength);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   OemUpdateF000ShdowRamHook
//
// Description: 
//              
//
// Input:	IN UINT32*	pShadowRam
//			IN UINTN	UsageLength
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID OemUpdateF000ShdowRamHook(
    IN UINT32*	pShadowRam,
	IN UINTN	UsageLength
)
{
    UINTN i;
    
    for (i = 0; UpdateF000ShdowRamHookList[i] != NULL; i++) 
        UpdateF000ShdowRamHookList[i](pShadowRam,UsageLength);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: 	UpdateShadowBeforEfiBoot
//
// Description:	This is "BeforeEfiBootLaunchHook" elink function.
//				It will store original data of Shadow ram and then copy
//				shadow buff's data to shadow ram. 
//				
// Input:	VOID
//
// Output:	VOID
//      
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID UpdateShadowBeforEfiBoot(VOID)
{
	EFI_STATUS	Status;
	UINT32 E000Offset = 0xE0000;
	UINT32 F000Offset = 0xF0000;

	TRACE((-1,"Entry Update Shadow Ram!!!\n"));	

	//unlock shadow ram
	//OemRuntimeShadowRamWrite(TRUE);
	 NbRuntimeShadowRamWrite(TRUE);

	TRACE((-1,"RsdpPlus (UpdateShadowBeforEfiBoot): gE000BuffLength [0x%x] \n",gE000BuffLength));

	if(gE000BuffLength != 0){
		Status = pBS->AllocatePool(
					EfiBootServicesData,
					gE000BuffLength,
					&gESegStore
					);
		ASSERT_EFI_ERROR(Status);
		if(EFI_ERROR(Status))
			return;
		
		pBS->CopyMem(gESegStore,(UINT32*)E000Offset,(UINTN)gE000BuffLength);
		MemSet((VOID*)E000Offset,gE000BuffLength, 0);
		pBS->CopyMem((UINT32*)E000Offset,gE000HeapPtr,(UINTN)gE000BuffLength);
		TRACE((-1,"E000 Info : Data length %d bytes, There are %d data in Shadow Ram!!!\n",gE000BuffLength, EsegUserCount));
		OemUpdateE000ShdowRamHook((UINT32*)E000Offset, (UINTN)gE000BuffLength);
	}

	TRACE((-1,"RsdpPlus (UpdateShadowBeforEfiBoot): gF000BuffLength [0x%x] \n",gF000BuffLength));
	if(gF000BuffLength != 0){
		Status = pBS->AllocatePool(
					EfiBootServicesData,
					gF000BuffLength,
					&gFSegStore
					);
		ASSERT_EFI_ERROR(Status);
		if(EFI_ERROR(Status))
			return;
		
		pBS->CopyMem(gFSegStore,(UINT32*)F000Offset,(UINTN)gF000BuffLength);
		MemSet((VOID*)F000Offset,gF000BuffLength, 0);
		pBS->CopyMem((UINT32*)F000Offset,gF000HeapPtr,(UINTN)gF000BuffLength);
		TRACE((-1,"F000 Info : Data length %d bytes, There are %d data in Shadow Ram!!!\n",gF000BuffLength, FsegUserCount));
		OemUpdateF000ShdowRamHook((UINT32*)F000Offset, (UINTN)gF000BuffLength);
	}

	//Lock shadow ram
	//OemRuntimeShadowRamWrite(FALSE);
	 NbRuntimeShadowRamWrite(FALSE);

	return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: 	EraseShadowAfterEfiBoot
//
// Description: This is "AfterEfiBootLaunchHook" elink function.
//				It will restore original data to Shadow ram. 
//				
// Input:	VOID
//
// Output:	VOID
//      
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID EraseShadowAfterEfiBoot(VOID)
{
	UINT32 E000Offset = 0xE0000;
	UINT32 F000Offset = 0xF0000;
	BOOLEAN ExitToSetup = TRUE;	
	EFI_STATUS 	Status;

	if(gE000BuffLength == 0 && gF000BuffLength == 0) return;
	
	TRACE((-1,"Store Shadow Ram to default!!!\n"));	
	//OemRuntimeShadowRamWrite(TRUE);
	NbRuntimeShadowRamWrite(TRUE);

 	if(gE000BuffLength != 0){
		pBS->CopyMem((UINT32*)E000Offset,gESegStore,gE000BuffLength);
		pBS->FreePool(gESegStore);
	}

	if(gF000BuffLength != 0){
		pBS->CopyMem((UINT32*)F000Offset,gFSegStore,gF000BuffLength);
		Status = pBS->FreePool(gFSegStore);
	}
	
	//OemRuntimeShadowRamWrite(FALSE);
	NbRuntimeShadowRamWrite(FALSE);

	Status = pRS->SetVariable(
					L"Exitflag",
					&ManageShdowRamProtocolGuid,
					EFI_VARIABLE_BOOTSERVICE_ACCESS \
					|EFI_VARIABLE_RUNTIME_ACCESS,
					sizeof(ExitToSetup),
					&ExitToSetup);
	ASSERT_EFI_ERROR (Status);
	//TRACE((-1,"Rsdp Dbg : Set ExitFlg variable %r \n",Status));

	return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: 	HeapToE000
//
// Description: This is protocol function.
//				According to input data and copy those datas to buffer.
//
// Input:	UINT8 	*pData
//			UINT32	Align
//			UINTN 	Length
//			DATA_BUFF_STRUC *pData2 OPTIONAL
//
// Output:	EFI_STATUS
//      
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS HeapToE000(
	IN UINT8 *pData,
	IN UINT32 Align,
	IN UINTN Length,
	IN OUT DATA_BUFF_STRUC *pData2 OPTIONAL)
{

	UINT8	*DataPtr = NULL;
	UINT8	*EndOfDataPtr = NULL;

	if(pData == NULL || Length == 0)
		return EFI_INVALID_PARAMETER;

	if(((UINTN)gE000BuffLength + Length) > gBufferSize)
		return EFI_BUFFER_TOO_SMALL;

	if(Align != 0){
		DataPtr = (UINT8*)(( (UINT32)(gE000HeapPtr + gE000BuffLength)& ~(Align - 1)) + Align);
	}else{
		DataPtr = gE000HeapPtr + gE000BuffLength;
	}

	if(pData2 != NULL){
		pData2->BuffAddress = (UINTN)gE000HeapPtr;
		//pData2->UsedLength = gE000BuffLength;
		pData2->UsedLength = (UINTN)(DataPtr - gE000HeapPtr);

	}

	EndOfDataPtr = (UINT8*)(DataPtr + Length);
	TRACE((-1,"RsdpPlus : Align [0x%x],DataPtr [0x%lx],EndOfDataPtr [0x%lx] \n",Align,DataPtr,EndOfDataPtr));
	
	if(EndOfDataPtr > (gE000HeapPtr + gBufferSize))
		return EFI_BUFFER_TOO_SMALL;

	pBS->CopyMem(DataPtr, pData, Length);

	//gE000BuffLength = gE000BuffLength + (UINTN)(EndOfDataPtr - DataPtr);
	gE000BuffLength = (UINT32)(EndOfDataPtr - gE000HeapPtr);
	TRACE((-1,"RsdpPlus (HeapToE000): gE000BuffLength [0x%x] \n",gE000BuffLength));

	EsegUserCount++;

	return EFI_SUCCESS;		
}
		
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: 	HeapToF000
//
// Description: This is protocol function.
//				According to input data and copy those datas to buffer.
//				
// Input:	UINT8 *pData
//			UINT32 Align
//			UINTN Length
//			DATA_BUFF_STRUC *pData2 OPTIONAL
//
// Output:	EFI_STATUS
//      
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS HeapToF000(
	IN UINT8 *pData,
	IN UINT32 Align,
	IN UINTN Length,
	IN OUT DATA_BUFF_STRUC *pData2 OPTIONAL)
{
	UINT8	*DataPtr = NULL;
	UINT8	*EndOfDataPtr = NULL;


	if(pData == NULL || Length == 0)
		return EFI_INVALID_PARAMETER;

	if(((UINTN)gF000BuffLength + Length) > gBufferSize)
		return EFI_BUFFER_TOO_SMALL;

	if(Align != 0){
		DataPtr = (UINT8*)(( (UINT32)(gF000HeapPtr + gF000BuffLength)& ~(Align - 1)) + Align);
	}else{
		DataPtr = gF000HeapPtr + gF000BuffLength;
	}

	if(pData2 != NULL){
		pData2->BuffAddress = (UINTN)gF000HeapPtr;
		//pData2->UsedLength = gF000BuffLength;
		pData2->UsedLength = (UINTN)(DataPtr - gF000HeapPtr);
	}

	EndOfDataPtr = (UINT8*)(DataPtr + Length);
	TRACE((-1,"RsdpPlus : Align [0x%x],DataPtr [0x%lx],EndOfDataPtr [0x%lx] \n",Align,DataPtr,EndOfDataPtr));
	
	if(EndOfDataPtr > (gF000HeapPtr + gBufferSize))
		return EFI_BUFFER_TOO_SMALL;

	pBS->CopyMem(DataPtr, pData, Length);

	//gF000BuffLength = gF000BuffLength + (UINTN)(EndOfDataPtr - DataPtr);
	gF000BuffLength = (UINT32)(EndOfDataPtr - gF000HeapPtr);
	TRACE((-1,"RsdpPlus (HeapToF000): gF000BuffLength [0x%x] \n",gF000BuffLength));

	FsegUserCount++;

	return EFI_SUCCESS;		
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: 	ShadowRamCallBack
//
// Description: Install Shadow Ram Protocol.
//				
// Input:	EFI_EVENT	Event
//			VOID 		*Context
//
// Output:	EFI_STATUS	Status
//      
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ShadowRamCallBack(IN EFI_EVENT Event,IN VOID *Context)
{
	EFI_STATUS	Status;
	
	Status = pBS->InstallProtocolInterface(
								&gShadowRameHandle,
								&gShdowRamProtocolGuid,
								EFI_NATIVE_INTERFACE,
								&gShadowRamProtocol
								);
	if(EFI_ERROR(Status)) return Status;
	pBS->CloseEvent(Event);
	
	return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: 	ManageShadowRamEntryPoint
//
// Description: Entry point for RsdpPlus initialization.
//				Register a ConOutStarted protocol callback function.
//				And allocate two buff for Manage Shadow Ram protocol used.				 
//				Install Manage Shadow Ram protocol.
//
// Input:	EFI_HANDLE			ImageHandle
//			EFI_SYSTEM_TABLE 	*SystemTable
//
// Output:	EFI_STATUS	Status
//      
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ManageShadowRamEntryPoint(
	IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
)
{
	EFI_EVENT	Event;
	EFI_STATUS	Status;
	VOID		*Registration;
	static	EFI_PHYSICAL_ADDRESS	E000PagePtr;
	static 	EFI_PHYSICAL_ADDRESS	F000PagePtr;

	InitAmiLib(ImageHandle,SystemTable);

   	Status = RegisterProtocolCallback(
				&gAllDriverConnectGuid,
				ShadowRamCallBack,
				NULL,   // Context
				&Event,
				&Registration
				);
	if(EFI_ERROR(Status)) return Status;

	Status = pBS->AllocatePages(
                 AllocateAnyPages, 
                 EfiBootServicesData, 
                 16,
                 &E000PagePtr);
	ASSERT_EFI_ERROR(Status);
	gE000HeapPtr = (UINT8*)E000PagePtr;
	TRACE((-1,"RsdpPlus : gE000HeapPtr [0x%lx] \n",gE000HeapPtr));

	Status = pBS->AllocatePages(
				AllocateAnyPages, 
                EfiBootServicesData, 
                16,
                &F000PagePtr);
	ASSERT_EFI_ERROR(Status);
	gF000HeapPtr = (UINT8*)F000PagePtr;
	TRACE((-1,"RsdpPlus : gF000HeapPtr [0x%lx] \n",gF000HeapPtr));

	Status = pBS->InstallProtocolInterface(
					&gManageShadowRamHandle,
					&ManageShdowRamProtocolGuid,
					EFI_NATIVE_INTERFACE,
					&gManageShadowRamProtocol
					);
	return Status;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
