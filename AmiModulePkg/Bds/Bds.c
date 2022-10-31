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
// $Header: /Alaska/SOURCE/Core/EDK/DxeMain/BDS.c 118   12/04/12 5:19p Felixp $
//
// $Revision: 118 $
//
// $Date: 12/04/12 5:19p $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	BDS.c
//
// Description:	Implementation of the BDS architectural protocol
//
//<AMI_FHDR_END>
//**********************************************************************
#include "BootOptions.h"
#include <Protocol/Bds.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciIo.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/ConsoleControl.h>
#include <Protocol/AMIPostMgr.h>
#include <Protocol/LoadFile.h>
#include <Dxe.h>
#include <Hob.h>
#include <Guid/MemoryTypeInformation.h>
#include <Library/PcdLib.h>
#include <Library\TrEEPhysicalPresenceLib.h>

/**************************** TYPES ***********************************/
#define EFI_DXE_PERFORMANCE
/***************** FUNCTION DECLARATIONS *****************************/
//this funciton is created from InitList.c template file during build process
VOID InitParts2(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);

//Local functions
VOID BdsEntry (IN EFI_BDS_ARCH_PROTOCOL *This);

EFI_STATUS FwLoadFile (
	IN EFI_LOAD_FILE_PROTOCOL *This, IN EFI_DEVICE_PATH_PROTOCOL *FilePath,
	IN BOOLEAN BootPolicy, IN OUT UINTN *BufferSize,
	IN VOID *Buffer OPTIONAL
);

/**************************** CONSTANTS *******************************/
#define BDS_ALL_DRIVERS_CONNECTED_PROTOCOL_GUID \
    {0xdbc9fd21, 0xfad8, 0x45b0, 0x9e, 0x78, 0x27, 0x15, 0x88, 0x67, 0xcc, 0x93}
// {3AA83745-9454-4f7a-A7C0-90DBD02FAB8E}
#define BDS_CONNECT_DRIVERS_PROTOCOL_GUID \
    { 0x3aa83745, 0x9454, 0x4f7a, { 0xa7, 0xc0, 0x90, 0xdb, 0xd0, 0x2f, 0xab, 0x8e } }
// {8DB699CC-BC81-41e2-AAC6-D81D5300D759}
#define PARTITION_VARIABLE_GUID\
    { 0x8db699cc, 0xbc81, 0x41e2, { 0xaa, 0xc6, 0xd8, 0x1d, 0x53, 0x0, 0xd7, 0x59 } }
// {5023B95C-DB26-429b-A648-BD47664C8012}
#define AMI_MEDIA_DEVICE_PATH_GUID \
    { 0x5023b95c, 0xdb26, 0x429b, { 0xa6, 0x48, 0xbd, 0x47, 0x66, 0x4c, 0x80, 0x12 } }

#define BDS_DISPATCHER_PROTOCOL_GUID \
    {0xcfc5b882, 0xebde, 0x4782, { 0xb1, 0x82, 0x2f, 0xec, 0x7e, 0x3f, 0x3e, 0x90 }}


extern EFI_GUID gEfiBdsArchProtocolGuid; // = EFI_BDS_ARCH_PROTOCOL_GUID;
extern EFI_GUID gEfiDevicePathProtocolGuid ;//= EFI_DEVICE_PATH_PROTOCOL_GUID;
extern EFI_GUID gEfiFirmwareVolume2ProtocolGuid; // = EFI_FIRMWARE_VOLUME_PROTOCOL_GUID;
extern EFI_GUID gEfiLoadedImageProtocolGuid; //=EFI_LOADED_IMAGE_PROTOCOL_GUID;
extern EFI_GUID gEfiPciIoProtocolGuid; // = EFI_PCI_IO_PROTOCOL_GUID;
extern EFI_GUID gEfiBlockIoProtocolGuid;
extern EFI_GUID gEfiLoadFileProtocolGuid;
extern EFI_GUID gEfiSimpleFileSystemProtocolGuid;
//defined in BdsBoard.c
extern EFI_GUID SetupEnterProtocolGuid;
extern EFI_GUID SecondBootOptionProtocolGuid;
extern EFI_GUID BeforeBootProtocolGuid;
extern EFI_GUID BeforeLegacyBootProtocolGuid;
extern EFI_GUID *DefaultAppFfsGuidPtr;

extern BDS_CONTROL_FLOW_FUNCTION *BdsControlFlowFunctions[];
extern CHAR8 *BdsControlFlowFunctionNames[];

EFI_GUID EfiVariableGuid = EFI_GLOBAL_VARIABLE;
EFI_GUID AmiPostMgrProtocolGuid=AMI_POST_MANAGER_PROTOCOL_GUID;
EFI_GUID ConInStartedProtocolGuid = CONSOLE_IN_DEVICES_STARTED_PROTOCOL_GUID;
EFI_GUID ConOutStartedProtocolGuid = CONSOLE_OUT_DEVICES_STARTED_PROTOCOL_GUID;
EFI_GUID BdsAllDriversConnectedProtocolGuid = BDS_ALL_DRIVERS_CONNECTED_PROTOCOL_GUID;
EFI_GUID BdsConnectDriversProtocolGuid = BDS_CONNECT_DRIVERS_PROTOCOL_GUID;
EFI_GUID PartitionVariableGuid = PARTITION_VARIABLE_GUID;

EFI_GUID BdsDispatcherProtocolGuid = BDS_DISPATCHER_PROTOCOL_GUID;


EFI_BDS_ARCH_PROTOCOL BDS = {&BdsEntry};

/**************************** VARIABLES *******************************/
//externals defined in BdsBoard.c
extern UINT16 DefaultTimeout;
extern BOOLEAN QuietBoot;
extern STRING_REF StrToken[];
extern CONST CHAR16 *FirmwareVendorString;
extern CONST UINT32 FirmwareRevision;

// Global Variables
EFI_LOAD_FILE_PROTOCOL FwLoadFileInterface  = {FwLoadFile};

struct {
	VENDOR_DEVICE_PATH Media;
	EFI_DEVICE_PATH_PROTOCOL End;
} FwLoadFileDp = {
	{
        {
            MEDIA_DEVICE_PATH, MEDIA_VENDOR_DP,
            sizeof(VENDOR_DEVICE_PATH)
        },
        AMI_MEDIA_DEVICE_PATH_GUID
    },
	{
        END_DEVICE_PATH, END_ENTIRE_SUBTYPE,
        sizeof(EFI_DEVICE_PATH_PROTOCOL)
    }
};

EFI_HANDLE LpcHandle = NULL;
EFI_DEVICE_PATH_PROTOCOL *LpcDevicePath = NULL;

VOID ConnectDevicePath(IN EFI_DEVICE_PATH_PROTOCOL *pPath)
{
    EFI_HANDLE Handle;
    EFI_STATUS Status;

    if (pPath == NULL) return;
	while (TRUE)
	{
		EFI_DEVICE_PATH_PROTOCOL *pLastPath=NULL, *pFirstNode = pPath;
		if (isEndNode(pPath))
		{
			if (pPath->SubType == END_ENTIRE_SUBTYPE) break;
			pPath++;
			continue;
		}
		while(TRUE){
            EFI_DEVICE_PATH_PROTOCOL *Dp;
            UINT8 SubType;

	        pPath = pFirstNode;

            //LocateDevicePath can not work with multi-instance device paths.
            //Prepare single instance device path and call LocateDevicePath
            Dp = DPGetEndNode(pPath);
	        SubType = Dp->SubType;
	        Dp->SubType=END_ENTIRE_SUBTYPE;
            Status = pBS->LocateDevicePath(&gEfiDevicePathProtocolGuid, &pPath, &Handle);
            Dp->SubType=SubType;
			if (EFI_ERROR(Status)) break;

			if (isEndNode(pPath))
			{
 				//Last time let's do it recursively	
				pBS->ConnectController(Handle,NULL,NULL,TRUE);	
				break;
			}
			if (pPath==pLastPath) break;
			pLastPath = pPath;
			if (EFI_ERROR(pBS->ConnectController(Handle,NULL,pPath,FALSE))) break;
		}
		while (!isEndNode(pPath))
			pPath = NEXT_NODE(pPath);
	}
}

EFI_DEVICE_PATH_PROTOCOL* AddDevicePath(EFI_DEVICE_PATH_PROTOCOL *pDp1, EFI_DEVICE_PATH_PROTOCOL *pDp2)
{
	if (!pDp2) return pDp1;
	if (!pDp1)
	{
		return DPCopy(pDp2);
	}
	else
	{
		pDp2 = DPAddInstance(pDp1,pDp2);
		pBS->FreePool(pDp1);
		return pDp2;
	}
} 

EFI_STATUS GetPciHandlesByClass(
    UINT8 Class, UINT8 SubClass, UINTN *NumberOfHandles, EFI_HANDLE **HandleBuffer
){
	EFI_STATUS Status;
	EFI_HANDLE *Handle;
	UINTN Number,i;

    if (!NumberOfHandles || !HandleBuffer) return EFI_INVALID_PARAMETER;
	//Get a list of all PCI devices
	Status = pBS->LocateHandleBuffer(
        ByProtocol,&gEfiPciIoProtocolGuid, NULL, &Number, &Handle
    );
	if (EFI_ERROR(Status)) return Status;
    *NumberOfHandles = 0;
	for(i=0; i<Number; i++)
	{
		EFI_PCI_IO_PROTOCOL *PciIo;
		UINT8 PciClass[4];
		Status=pBS->HandleProtocol(Handle[i],&gEfiPciIoProtocolGuid,(VOID**)&PciIo);
		if (EFI_ERROR(Status)) continue;
		Status=PciIo->Pci.Read(PciIo, EfiPciIoWidthUint32, PCI_REV_ID_OFFSET, 1, &PciClass);
		if( PciClass[3]==Class && PciClass[2]==SubClass)
            Handle[(*NumberOfHandles)++] = Handle[i];
	}
	if (*NumberOfHandles == 0){
        pBS->FreePool(Handle);
        return EFI_NOT_FOUND;
    }
    *HandleBuffer = Handle;
    return EFI_SUCCESS;
}

EFI_STATUS InitSystemVariable(
	IN CHAR16 *NameStr, IN UINTN DataSize, IN VOID *Data
)
{
	UINTN Size = 0;
	EFI_STATUS Status;
	Status = pRS->GetVariable(NameStr, &EfiVariableGuid, NULL, &Size, NULL);
	if (Status==EFI_NOT_FOUND)
	{
		return pRS->SetVariable(
			NameStr, &EfiVariableGuid,
			EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
			DataSize, Data);
	}
	return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
BdsLibConnectAllEfi (
  VOID
  )
{
  EFI_STATUS  Status;
  UINTN       HandleCount;
  EFI_HANDLE  *HandleBuffer;
  UINTN       Index;

  Status = pBS->LocateHandleBuffer (
                  AllHandles,
                  NULL,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = pBS->ConnectController (HandleBuffer[Index], NULL, NULL, TRUE);
  }

  if (HandleBuffer != NULL) {
    pBS->FreePool (HandleBuffer);
  }

  return EFI_SUCCESS;
}

static EFI_HANDLE *RootHandles;
static UINTN NumberOfHandles;
VOID ConnectEverything()
{
#ifdef CSM_SUPPORT
    EFI_LEGACY_BIOS_PROTOCOL *LegacyBios;
#endif
	UINTN i;
	EFI_STATUS Status;
	
	for(i=0; i<NumberOfHandles; i++) pBS->ConnectController(RootHandles[i],NULL,NULL,TRUE);

    BdsLibConnectAllEfi();

/////////////////////////////////////////
//TODO: Ugly patch for the Floppy Controller. Find the better place for it!
// It is necessary to Disconnect Floppy Device Handle when Floppy Drive is not connected.
// This is necessary to disable Floppy Device in the Super I/O
// and eliminate BBS Floppy boot option.
// It was previously done (In Core 4.0) by the Floppy Controller driver.
// However, EDK DXE Core (DxeMain) crashes during the 
// DisconnectController operation performed from within the Start function.
// Because of that, the Floppy Controller driver code is commented out
// and this patch is created.
// If you remove this code, you should also remove 
// LpcHandle & LpcDevicePath global variables.
// They are only used to implement this patch.
	if (LpcDevicePath)
	{
		EFI_HANDLE Handle;
		ACPI_HID_DEVICE_PATH FloppyCtrl = {
			{ACPI_DEVICE_PATH,ACPI_DP,sizeof(ACPI_HID_DEVICE_PATH)},
			EISA_PNP_ID(0x0604),0
		};
		UINT32 AltPnpId[] = {EISA_PNP_ID(0x0604), EISA_PNP_ID(0x700), 0};
		EFI_DEVICE_PATH_PROTOCOL *ChildDp, *TmpDp;
		for(i=0; AltPnpId[i]!=0; i++){
			FloppyCtrl.HID = AltPnpId[i];
			ChildDp=TmpDp = DPAddNode(LpcDevicePath, &FloppyCtrl.Header);
			Status=pBS->LocateDevicePath(&gEfiDevicePathProtocolGuid, &TmpDp, &Handle);
			if (!EFI_ERROR(Status) && isEndNode(TmpDp))
			{
				VOID* pDummy;
				Status = pBS->HandleProtocol(Handle,&gEfiBlockIoProtocolGuid,&pDummy);
				if (EFI_ERROR(Status)) pBS->DisconnectController(LpcHandle,NULL,Handle);
				pBS->FreePool(ChildDp);
				break;
			}
			pBS->FreePool(ChildDp);
		}
	}
#ifdef CSM_SUPPORT
    Status = pBS->LocateProtocol(&gEfiLegacyBiosProtocolGuid, NULL, &LegacyBios);
    if(!EFI_ERROR(Status)){
		LegacyBios->ShadowAllLegacyOproms(LegacyBios);
    }
#endif
}
VOID DisconnectEverything()
{
	UINTN i;
	for(i=0; i<NumberOfHandles; i++) pBS->DisconnectController(RootHandles[i],NULL,NULL);
}

VOID ReadyToBoot(UINT16 OptionNumber)
{
	//signal EFI_EVENT_SIGNAL_READY_TO_BOOT
	EFI_EVENT ReadToBootEvent;
	EFI_STATUS Status;
    if (OptionNumber!= (UINT16)-1)
    	pRS->SetVariable(
    		L"BootCurrent", &EfiVariableGuid,
    		EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
    		sizeof(OptionNumber), &OptionNumber
    	);
	Status = CreateReadyToBootEvent(
		TPL_CALLBACK, NULL, NULL, &ReadToBootEvent
	);
	if (!EFI_ERROR(Status)) {
		pBS->SignalEvent(ReadToBootEvent);
	   	pBS->CloseEvent(ReadToBootEvent);
	}
}

#if CSM_SUPPORT
EFI_STATUS BootLegacy(EFI_DEVICE_PATH_PROTOCOL *Dp, UINT16 Number)
{
	UINTN i, Old=-1, New, Priority=-1;
	BBS_BBS_DEVICE_PATH *BbsEntry = (BBS_BBS_DEVICE_PATH*)Dp;
    EFI_STATUS Status;
    EFI_LEGACY_BIOS_PROTOCOL *LegacyBios;
    UINT16 HddCount;
    UINT16 BbsCount;
    HDD_INFO *HddInfo;
    BBS_TABLE *BbsTable;

	// Legacy Boot
    Status = pBS->LocateProtocol(&gEfiLegacyBiosProtocolGuid, NULL, &LegacyBios);
    if(EFI_ERROR(Status)) return Status;
	LegacyBios->GetBbsInfo(LegacyBios, &HddCount, &HddInfo, &BbsCount, &BbsTable);
	if (!BbsCount) return EFI_NOT_FOUND;	
	for(i=0; i<BbsCount; i++)
	{
		if ( //  !BbsTable[i].StatusFlags.Enabled
			/*||*/ BbsTable[i].BootPriority==BBS_IGNORE_ENTRY
			|| BbsTable[i].BootPriority==BBS_DO_NOT_BOOT_FROM
		) continue;
		if (!BbsTable[i].BootPriority && Old==-1) Old=i;
		if (	BbsTable[i].DeviceType==BbsEntry->DeviceType
			&&	BbsTable[i].BootPriority < Priority
		)
		{
			Priority = BbsTable[i].BootPriority;
			New = i;
		}
	}
	if (Old!=-1) BbsTable[Old].BootPriority=BbsTable[New].BootPriority;
	BbsTable[New].BootPriority=0;
	ReadyToBoot(Number);
	PERF_END (NULL, "BDS", NULL, 0);
	return LegacyBios->LegacyBoot(LegacyBios,(BBS_BBS_DEVICE_PATH*)Dp,0,0);
}
#endif

EFI_STATUS BootEfi(EFI_DEVICE_PATH_PROTOCOL *Dp, UINT16 Number, VOID *pOptions, UINT32 Size)
{
	EFI_STATUS Status;
	EFI_HANDLE Handle;
	EFI_LOADED_IMAGE_PROTOCOL *Image;
	Status=pBS->LoadImage(TRUE, TheImageHandle, Dp, NULL, 0, &Handle);
	//If LoadImage has failed, try the default OS loader path.
	if (EFI_ERROR(Status)){
        UINT8 FileNodeBuffer[sizeof(EFI_DEVICE_PATH_PROTOCOL)+sizeof(EFI_REMOVABLE_MEDIA_FILE_NAME)];
        EFI_DEVICE_PATH_PROTOCOL *FileNode = (EFI_DEVICE_PATH_PROTOCOL*)FileNodeBuffer;
        FileNode->Type=MEDIA_DEVICE_PATH;
        FileNode->SubType=MEDIA_FILEPATH_DP;
        FileNode->Length[0]=(UINT8)(sizeof(EFI_DEVICE_PATH_PROTOCOL)+sizeof(EFI_REMOVABLE_MEDIA_FILE_NAME));
        FileNode->Length[1]=0;
        Wcscpy((CHAR16*)(FileNode+1),EFI_REMOVABLE_MEDIA_FILE_NAME);
        Dp=DPAddNode(Dp,FileNode);
        Status=pBS->LoadImage(TRUE, TheImageHandle, Dp, NULL, 0, &Handle);
        pBS->FreePool(Dp);
        if (EFI_ERROR(Status)) return Status;
    }
	Status=pBS->HandleProtocol(Handle,&gEfiLoadedImageProtocolGuid,(VOID**)&Image);
	if (!EFI_ERROR(Status) && Size)
	{
		Image->LoadOptionsSize = Size;	
		Image->LoadOptions = pOptions;
	}
	ReadyToBoot(Number);
	PERF_END (NULL, "BDS", NULL, 0);
	return pBS->StartImage(Handle, NULL, NULL);
}

EFI_STATUS Boot(EFI_LOAD_OPTION *BootOption, UINT16 Number, UINTN Size)
{
	EFI_DEVICE_PATH_PROTOCOL *Dp;
	Dp = (EFI_DEVICE_PATH_PROTOCOL*)
			(	//skip the header
				(UINT8*)(BootOption+1) 
				//skip the string
				+(Wcslen((CHAR16*)(BootOption+1))+1)*sizeof(CHAR16)
			);
	if (Dp->Type!=BBS_DEVICE_PATH)
	{
		UINT8 *pOptions = (UINT8*)Dp+BootOption->FilePathListLength;		
		return BootEfi(Dp,Number,pOptions, (UINT32)((UINT8*)BootOption+Size-pOptions));
	}
#if CSM_SUPPORT
	else
	{
		return BootLegacy(Dp,Number);
	}
#endif
	return EFI_UNSUPPORTED;
}

VOID RunDrivers(){
	EFI_LOAD_OPTION *DriverOption = NULL; //buffer for DriverXXX variables
	UINT16 *DriverOrder = NULL; //old(saved) Driver Order
	UINTN DriverOrderSize = 0; //size of DriverOrder Variable
	EFI_STATUS Status;
	UINTN Size,i;
    BOOLEAN ReconnectAll = FALSE;

	//================== Init Driver Order buffers ========================//
	Status = GetEfiVariable(L"DriverOrder", &EfiVariableGuid, NULL, &DriverOrderSize, (VOID**)&DriverOrder);
    if (EFI_ERROR(Status)) return;
	//===================================================================//
	// Start drivers refered to by DriverXXXX                            //
	//===================================================================//
	for(i=0; i<DriverOrderSize/sizeof(UINT16); i++){	
		CHAR16 DriverStr[9];
		EFI_DEVICE_PATH_PROTOCOL *DevicePath;
	    EFI_HANDLE Handle;
	    EFI_LOADED_IMAGE_PROTOCOL *Image;
        UINT32 SizeOfOptions;
        UINT8 *Options;

		// Get Driver Option
		Swprintf(DriverStr,L"Driver%04X",DriverOrder[i]);
		Status=GetEfiVariable(DriverStr, &EfiVariableGuid, NULL, &Size, (VOID**)&DriverOption);
        if (   EFI_ERROR(Status) 
            || (DriverOption->Attributes & LOAD_OPTION_ACTIVE)==0
        ) continue; 

		DevicePath = (EFI_DEVICE_PATH_PROTOCOL*)
				(	//skip the header
					(UINT8*)(DriverOption+1) 
					//skip the string
					+(Wcslen((CHAR16*)(DriverOption+1))+1)*sizeof(CHAR16)
		);
	    Status=pBS->LoadImage(
            TRUE, TheImageHandle, DevicePath, NULL, 0, &Handle
        );
	    if (EFI_ERROR(Status)) continue;
	    Status=pBS->HandleProtocol(
            Handle,&gEfiLoadedImageProtocolGuid,(VOID**)&Image
        );
		Options = (UINT8*)DevicePath+DriverOption->FilePathListLength;
		SizeOfOptions=(UINT32)((UINT8*)DriverOption+Size-Options);
	    if (!EFI_ERROR(Status)&& SizeOfOptions!=0){
		    Image->LoadOptionsSize = SizeOfOptions;	
		    Image->LoadOptions = Options;
	    }
	    Status=pBS->StartImage(Handle, NULL, NULL);
        if (   !EFI_ERROR(Status) 
            && (DriverOption->Attributes & LOAD_OPTION_FORCE_RECONNECT)!=0
        ) ReconnectAll=TRUE;
	}
	pBS->FreePool(DriverOption);
    pBS->FreePool(DriverOrder);
    if (ReconnectAll){
        DisconnectEverything();
        ConnectEverything();
    }
}

EFI_DEVICE_PATH_PROTOCOL* DiscoverPartition(
    IN EFI_DEVICE_PATH_PROTOCOL *DevicePath
){
    UINTN Count,i;
    EFI_HANDLE *Handle;
    EFI_STATUS Status;
    EFI_DEVICE_PATH_PROTOCOL *PartDevicePath=NULL;	
    HARDDRIVE_DEVICE_PATH* BootParitionDevicePath  = (HARDDRIVE_DEVICE_PATH*)DevicePath;
    //get list of available Block I/O devices    
    Status=pBS->LocateHandleBuffer(ByProtocol,&gEfiBlockIoProtocolGuid,NULL,&Count,&Handle);    
    if (EFI_ERROR(Status)) return NULL;

    for(i=0;i<Count;i++){
        EFI_BLOCK_IO_PROTOCOL *BlockIo;
        EFI_DEVICE_PATH_PROTOCOL *PartitionDevicePath;
        HARDDRIVE_DEVICE_PATH* PartitionNode;
        Status = pBS->HandleProtocol(Handle[i],&gEfiBlockIoProtocolGuid,(VOID**)&BlockIo);
        if (EFI_ERROR(Status)) continue;
        // if this is not partition, continue
        if (!BlockIo->Media->LogicalPartition) continue;
        Status = pBS->HandleProtocol(Handle[i],&gEfiDevicePathProtocolGuid,(VOID**)&PartitionDevicePath);
        if (EFI_ERROR(Status)) continue;
        // Get last node of the device path. It should be partition node
        PartitionNode = (HARDDRIVE_DEVICE_PATH*)DPGetLastNode(PartitionDevicePath);
        //Check if our partition matches Boot partition
        if (   PartitionNode->Header.Type!=MEDIA_DEVICE_PATH 
            || PartitionNode->Header.SubType!=MEDIA_HARDDRIVE_DP
        ) continue;
        if (   PartitionNode->PartitionNumber==BootParitionDevicePath->PartitionNumber 
            && PartitionNode->SignatureType==BootParitionDevicePath->SignatureType 
            && !MemCmp(PartitionNode->Signature,BootParitionDevicePath->Signature,16) 
        ){
            //Match found
			PartDevicePath = PartitionDevicePath;
            break;
        }
    }
    pBS->FreePool(Handle);
    return PartDevicePath;
}

EFI_STATUS FwLoadFile (
	IN EFI_LOAD_FILE_PROTOCOL *This, IN EFI_DEVICE_PATH_PROTOCOL *FilePath,
	IN BOOLEAN BootPolicy, IN OUT UINTN *BufferSize,
	IN VOID *Buffer OPTIONAL
){
    EFI_GUID *FileGuid;
	UINTN FvCount,i;
	EFI_HANDLE *FvHandle;
    EFI_STATUS Status;

    if (!BufferSize || *BufferSize && !Buffer)
        return EFI_INVALID_PARAMETER;
    if (!FilePath || isEndNode(FilePath)){
        if (BootPolicy){
            if (DefaultAppFfsGuidPtr==NULL) return EFI_UNSUPPORTED;
            else FileGuid = DefaultAppFfsGuidPtr;
        }else{
            return EFI_INVALID_PARAMETER;
        }
    }else{
        if (   FilePath->Type!=MEDIA_DEVICE_PATH 
            || FilePath->SubType!=MEDIA_FV_FILEPATH_DP
        ) return EFI_INVALID_PARAMETER;
        FileGuid = &((MEDIA_FW_VOL_FILEPATH_DEVICE_PATH*)FilePath)->FvFileName;
    }
    //If Buffer is NULL, ReadSection will allocate the memory.
    //That's not what we need.
    //Initialize Buffer with some value.
    //We don't care what value is that because *BufferSize is 0 anyway,
    //so nothing will be copied into the buffer.
    //We know that *BufferSize is always 0 for NULL buffer because we checked that
    //at the beginning of the routine.
    if (!Buffer) Buffer = (VOID*)1;
	Status=pBS->LocateHandleBuffer(ByProtocol, &gEfiFirmwareVolume2ProtocolGuid, NULL, &FvCount, &FvHandle);
	if (EFI_ERROR(Status)) return Status;
	Status=EFI_NOT_FOUND;
	for(i=0; i<FvCount; i++)
	{
	    EFI_FIRMWARE_VOLUME_PROTOCOL *Fv;
	    UINT32 AuthStatus;
        Status = pBS->HandleProtocol(FvHandle[i], &gEfiFirmwareVolume2ProtocolGuid, (VOID**)&Fv);
		if (EFI_ERROR(Status)) continue;
	    Status = Fv->ReadSection(
            Fv, FileGuid, EFI_SECTION_PE32, 
			0, &Buffer, BufferSize, &AuthStatus
		);
        if (!EFI_ERROR(Status)){
            if (Status==EFI_WARN_BUFFER_TOO_SMALL) Status=EFI_BUFFER_TOO_SMALL;
            break;
        }
	}
	pBS->FreePool(FvHandle);
	return Status;
}

VOID InstallFwLoadFile(){
    EFI_HANDLE Handle=NULL;
    pBS->InstallMultipleProtocolInterfaces(
        &Handle, 
        &gEfiLoadFileProtocolGuid, &FwLoadFileInterface,
        &gEfiDevicePathProtocolGuid, &FwLoadFileDp,
        NULL
    );
}

//----------------------------------------------------------------------------
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	SignalProtocolEvent
//
// Description:	Internal function that installs/uninstall protocol
//				with a specified GUID and NULL interface.
//              Such protocols can be used as event signaling mechanism.
//
// Input:		ProtocolGuid Pointer to the protocol GUID
//
// Output:		None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SignalProtocolEvent(IN EFI_GUID *ProtocolGuid){
    EFI_HANDLE  Handle = NULL;
    pBS->InstallProtocolInterface (
        &Handle, ProtocolGuid, EFI_NATIVE_INTERFACE,NULL
    );
    pBS->UninstallProtocolInterface (
        Handle, ProtocolGuid, NULL
    );
}

#ifdef EFI_DXE_PERFORMANCE
VOID SavePerformanceData(IN EFI_EVENT Event, IN VOID *Context){
    PERF_END (NULL, "BDS", NULL, 0);
    WriteBootToOsPerformanceData();
}
#endif

VOID SaveFpdtDataOnLegacyBoot(IN EFI_EVENT Event, IN VOID *Context){
    AmiFillFpdt (FillOsLoaderStartImageStart); // Fill OsLoaderStartImageStart field in FPDT
}

static VOID DummyEndOfDxeEventCallback(IN EFI_EVENT Event, IN VOID *Context){}

VOID SignalEndOfDxeEvent(VOID){
    EFI_EVENT Event;
    EFI_STATUS Status;

    Status = pBS->CreateEventEx(
		EVT_NOTIFY_SIGNAL, TPL_CALLBACK, DummyEndOfDxeEventCallback,
        NULL, &gEfiEndOfDxeEventGroupGuid, &Event
	);
    if(EFI_ERROR(Status)) return;
    pBS->SignalEvent(Event);
    pBS->CloseEvent(Event);
}

VOID SignalConnectDriversEvent(){
    PROGRESS_CODE(DXE_BDS_CONNECT_DRIVERS);
    SignalProtocolEvent(&BdsConnectDriversProtocolGuid);
	SignalEndOfDxeEvent();
}

VOID ConnectRootBridgeHandles(){
	EFI_HANDLE *Handle;
	UINTN NumberOfHandles;
    EFI_STATUS Status;
	UINTN i;

	//Enumerate PCI Bus and Create handles for all PCI devices
	Status = pBS->LocateHandleBuffer(
        ByProtocol,&gEfiPciRootBridgeIoProtocolGuid, NULL, 
        &NumberOfHandles, &Handle
    );
	if (EFI_ERROR(Status)) return;
	for(i=0; i<NumberOfHandles; i++) 
        pBS->ConnectController(Handle[i],NULL,NULL,FALSE);
	pBS->FreePool(Handle);	
}

VOID ReportConnectConOutProgressCode(){
    PROGRESS_CODE(DXE_CON_OUT_CONNECT);
}

VOID ConnectVgaConOut(){
	EFI_STATUS Status;
	EFI_HANDLE *Handle;
	UINTN Number,i;
	EFI_DEVICE_PATH_PROTOCOL *OnBoard=NULL, *OffBoard=NULL;
    UINT64 PciAttributes;

	//Get a list of all PCI devices
	Status = pBS->LocateHandleBuffer(
        ByProtocol,&gEfiPciIoProtocolGuid, NULL, &Number, &Handle
    );
	if (EFI_ERROR(Status)) return;
	for(i=0; i<Number; i++)
	{
		EFI_PCI_IO_PROTOCOL *PciIo;
		EFI_DEVICE_PATH_PROTOCOL *Dp;
		UINT8 PciClass;
		Status=pBS->HandleProtocol(Handle[i],&gEfiPciIoProtocolGuid,(VOID**)&PciIo);
		if (EFI_ERROR(Status)) continue;
		Status=PciIo->Pci.Read(PciIo, EfiPciIoWidthUint8, 0xB, 1, &PciClass);
		if (EFI_ERROR(Status)) continue;
		if (PciClass!=PCI_CL_DISPLAY) continue;
		Status=pBS->HandleProtocol(Handle[i],&gEfiDevicePathProtocolGuid,(VOID**)&Dp); 
		if (EFI_ERROR(Status)) continue;
		//We found Display adapter
		// Check if this is on-board device 
        //(EFI_PCI_IO_ATTRIBUTE_EMBEDDED_DEVICE is set).
        Status = PciIo->Attributes(
            PciIo, EfiPciIoAttributeOperationGet, 0, &PciAttributes
        );
        if (   !EFI_ERROR(Status) 
            && (PciAttributes & EFI_PCI_IO_ATTRIBUTE_EMBEDDED_DEVICE)
        )  OnBoard = AddDevicePath(OnBoard,Dp);
		else OffBoard = AddDevicePath(OffBoard,Dp);
	}
	pBS->FreePool(Handle);
    //Offboard has a higher priority
	OffBoard = AddDevicePath(OffBoard,OnBoard);
	if (OffBoard)
	{
		ConnectDevicePath(OffBoard);
		pBS->FreePool(OffBoard);
	}
}

VOID ConnecConsoleVariable(CHAR16* ConVar){
    EFI_DEVICE_PATH_PROTOCOL *ConPath=NULL;
    UINTN Size = 0;

    if (EFI_ERROR(
        GetEfiVariable(ConVar, &EfiVariableGuid, NULL, &Size, (VOID**)&ConPath)
    )) return;
	if (EFI_ERROR(IsValidDevicePath(ConPath))){
		TRACE((TRACE_DXE_CORE, 
			"ERROR: Console variable %s contains invalid device path.\n", 
			ConVar
		));
		return;
	}
    //Connect all active console devices
    ConnectDevicePath(ConPath);
    pBS->FreePool(ConPath);
}

VOID InstallConsoleStartedProtocol(
    CHAR16* ConDevVar, EFI_GUID* ProtocolGuid
){
    UINTN Size = 0;
    VOID *Interface;

    //Signal to Console Splitter that all console devices have been started
    //if at least one console device exists (ConDev variable exists)
    if (ConDevVar!=NULL && pRS->GetVariable(
            ConDevVar, &EfiVariableGuid, NULL, &Size, NULL
        ) == EFI_NOT_FOUND
    ) return;
    //if the protocol is already installed, return
    if (!EFI_ERROR(pBS->LocateProtocol(ProtocolGuid, NULL, &Interface)))
        return;
    pBS->InstallProtocolInterface(
        &TheImageHandle, ProtocolGuid, EFI_NATIVE_INTERFACE, NULL
    );
}

VOID ConnectConOutVariable(){
    ConnecConsoleVariable(L"ConOut");
}

VOID InstallConOutStartedProtocol(){
    InstallConsoleStartedProtocol(L"ConOutDev", &ConOutStartedProtocolGuid);
}

VOID ReportConnectConInProgressCode(){
    PROGRESS_CODE(DXE_CON_IN_CONNECT);
}

VOID ConnectPs2ConIn(){
	EFI_STATUS Status;
	EFI_HANDLE *Handle;
	UINTN Number,i;

	//Get a list of all PCI to ISA Bridges
	Status = GetPciHandlesByClass(
        PCI_CL_BRIDGE, PCI_CL_BRIDGE_SCL_ISA, &Number, &Handle
    );
	if (EFI_ERROR(Status)) return;
	for(i=0; i<Number; i++)
	{
		EFI_DEVICE_PATH_PROTOCOL *Dp, *ChildDp;
		ACPI_HID_DEVICE_PATH Ps2Kbd = {
			{ACPI_DEVICE_PATH,ACPI_DP,sizeof(ACPI_HID_DEVICE_PATH)},
			EISA_PNP_ID(0x303),0
		};
		ACPI_HID_DEVICE_PATH Ps2Mouse = {
			{ACPI_DEVICE_PATH,ACPI_DP,sizeof(ACPI_HID_DEVICE_PATH)},
			EISA_PNP_ID(0xF03),0
		};
		Status=pBS->HandleProtocol(Handle[i],&gEfiDevicePathProtocolGuid,(VOID**)&Dp);
		if (EFI_ERROR(Status)) continue;
		pBS->ConnectController(Handle[i],NULL,NULL,FALSE);
		ChildDp=DPAddNode(Dp, &Ps2Kbd.Header);
		ConnectDevicePath(ChildDp);
		pBS->FreePool(ChildDp);
		ChildDp=DPAddNode(Dp, &Ps2Mouse.Header);
		ConnectDevicePath(ChildDp);
		pBS->FreePool(ChildDp);
		LpcHandle = Handle[i];
		LpcDevicePath = Dp;
	}
	pBS->FreePool(Handle);

}

VOID ConnectUsbConIn(){
	EFI_STATUS Status;
	EFI_HANDLE *Handle;
	UINTN Number,i;
	//Get a list of all USB Controllers
	Status = GetPciHandlesByClass(
        PCI_CL_SER_BUS, PCI_CL_SER_BUS_SCL_USB, &Number, &Handle
    );
	if (EFI_ERROR(Status)) return;
	for(i=0; i<Number; i++)
	{
        pBS->ConnectController(Handle[i],NULL,NULL,TRUE);
	}
	pBS->FreePool(Handle);
}

VOID ConnectConInVariable(){
    ConnecConsoleVariable(L"ConIn");
}

VOID InstallConInStartedProtocol(){
    InstallConsoleStartedProtocol(L"ConInDev", &ConInStartedProtocolGuid);
}

VOID ConInAvailabilityBeep(){
    LibReportStatusCode(EFI_PROGRESS_CODE, AMI_STATUS_CODE_BEEP_CLASS|1, 0, NULL, NULL);
}

VOID InitConVars()
{
	UINTN i;
	UINTN Size = 0;
    EFI_STATUS Status;
	EFI_DEVICE_PATH_PROTOCOL *ConPath=NULL;

    static CHAR16* ConVar[] = {L"ConOut", L"ConIn"};
    static CHAR16* ConDev[] = {L"ConOutDev", L"ConInDev"};

    // Install Console Stared Protocols
    // ConSplitter will process notification by populating 
    // corresponding fields of the system table.
    // At this point the protocol need to be installed 
    // even if no actual console devices are available
    // to prevent problems on headless systems 
    // caused by NULL console pointers in the system table.
    // The functions will not install the protocol if it has already been installed
    InstallConsoleStartedProtocol(NULL, &ConOutStartedProtocolGuid);
    InstallConsoleStartedProtocol(NULL, &ConInStartedProtocolGuid);

	//Create non-existent ConVar variables for ConIn and ConOut
	//ErrOut will be treated differently 
	for( i=0; i<2; i++){
		if (EFI_ERROR(
				GetEfiVariable(ConDev[i], &EfiVariableGuid, NULL, &Size, (VOID**)&ConPath)
			)
		) continue;
        //Set ConVar[i] equal to the ConDev[i]
		pRS->SetVariable(
			ConVar[i], &EfiVariableGuid,
			EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS, 
			Size, ConPath
		);
	}
	//Let's take care about ErrOut
    Status = GetEfiVariable(
        L"ErrOutDev", &EfiVariableGuid, NULL, &Size, (VOID**)&ConPath
    );
	if ( Status == EFI_NOT_FOUND ){
        Status = GetEfiVariable(
            L"ConOutDev", &EfiVariableGuid, NULL, &Size, (VOID**)&ConPath
        );
	    if (!EFI_ERROR(Status))
           	//Set ConErrDev equal to the ConOutDev
			pRS->SetVariable(
				L"ErrOutDev", &EfiVariableGuid,
				EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS, 
				Size, ConPath
			);
    }
	if (!EFI_ERROR(Status)){
        //Set ErrOut
		pRS->SetVariable(
		    L"ErrOut", &EfiVariableGuid,
            EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS, 
            Size, ConPath
        );
    }
	if (ConPath) pBS->FreePool(ConPath);
}

#define IsRuntimeMemoryType(MemoryType) \
  ((MemoryType) == EfiACPIReclaimMemory   || \
   (MemoryType) == EfiACPIMemoryNVS       || \
   (MemoryType) == EfiRuntimeServicesCode || \
   (MemoryType) == EfiRuntimeServicesData || \
   (MemoryType) == EfiReservedMemoryType )

VOID SaveMemoryTypeInformation (
    IN EFI_EVENT Event, IN VOID *Context
){
#ifdef EFI_DEBUG
    CONST CHAR8* EfiMemTypeStr[] = {
    	"Reserved   ",
    	"LoaderCode ",
    	"LoaderData ",
    	"BSCode     ",
    	"BSData     ",
    	"RSCode     ",
    	"RSData     ",
    	"Free       ",
    	"Unusable   ",
    	"ACPIReclaim",
    	"ACPINVS    ",
    	"MMIO       ",
    	"MMIOIOPort ",
    	"PalCode    "
    };
    struct{
        UINT32 Previous,Current,Next;
    } MemoryInfoHistory[EfiMaxMemoryType];
#endif
    static EFI_GUID HobListGuid = HOB_LIST_GUID;
    EFI_STATUS Status;
    EFI_HOB_GUID_TYPE *MemoryInformationHob;
    EFI_MEMORY_TYPE_INFORMATION *MemoryTypeInformation = NULL;
    UINTN MemoryTypeInformationSize = 0;
    BOOLEAN IsFirstBoot = FALSE;
    EFI_MEMORY_TYPE_INFORMATION *CurrentMemoryTypeInformation;
    UINTN i, j;
    BOOLEAN MemoryTypeInformationModified;
    BOOLEAN RtMemoryQuotasIncreased = FALSE;
    UINT32 Current, Next;

    static BOOLEAN MemoryTypeInformationIsSaved = FALSE;

    // Make sure the processing is performed only once.
    // (we are registering callback on multiple events in RegisterMemoryTypeInformationUpdateCallback)
    if (MemoryTypeInformationIsSaved){
        pBS->CloseEvent(Event);
        return;
    }

    // Get the Memory Type Information settings from Hob if they exist,
    // PEI is responsible for getting them from variable and building a Hob to save them.
    MemoryInformationHob = GetEfiConfigurationTable(pST, &HobListGuid);
    if (MemoryInformationHob == NULL) return;
    if (EFI_ERROR(
        FindNextHobByGuid(&gEfiMemoryTypeInformationGuid, (VOID**)&MemoryInformationHob)
    )) return;

	Status = pRS->GetVariable(
        L"PreviousMemoryTypeInformation", &gEfiMemoryTypeInformationGuid, NULL, 
        &MemoryTypeInformationSize, NULL
    );
	IsFirstBoot = Status==EFI_NOT_FOUND;

    MemoryTypeInformation = (EFI_MEMORY_TYPE_INFORMATION*)(MemoryInformationHob+1);
    MemoryTypeInformationSize =   MemoryInformationHob->Header.HobLength 
                                - sizeof (EFI_HOB_GUID_TYPE);
    // Save memory information for the current boot.
    // It will be used if next boot is S4 resume.
    Status = pRS->SetVariable (
        L"PreviousMemoryTypeInformation", &gEfiMemoryTypeInformationGuid,
        EFI_VARIABLE_NON_VOLATILE  | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        MemoryTypeInformationSize, MemoryTypeInformation
    );
    ASSERT_EFI_ERROR(Status);

    // Retrieve the current memory usage statistics.  If they are not found, then
    // no adjustments can be made to the Memory Type Information variable.
    CurrentMemoryTypeInformation = GetEfiConfigurationTable(
        pST, &gEfiMemoryTypeInformationGuid
    );
    if (CurrentMemoryTypeInformation == NULL) return;
    MemoryTypeInformationModified = FALSE;
    TRACE((TRACE_DXE_CORE, "BDS.%s(%X)\n", 
        "SaveMemoryTypeInformation", SaveMemoryTypeInformation
    ));
    // Adjust the Memory Type Information for the next boot
    for (i = 0; MemoryTypeInformation[i].Type != EfiMaxMemoryType; i++) {
        Current = 0;
        for (j = 0; CurrentMemoryTypeInformation[j].Type != EfiMaxMemoryType; j++) {
            if (MemoryTypeInformation[i].Type == CurrentMemoryTypeInformation[j].Type) {
                Current = CurrentMemoryTypeInformation[j].NumberOfPages;
                break;
            }
        }

        // Set next value to 125% of the current
        Next = Current + (Current >> 2);
#ifdef EFI_DEBUG
        MemoryInfoHistory[i].Previous = MemoryTypeInformation[i].NumberOfPages;
        MemoryInfoHistory[i].Current = Current;
        MemoryInfoHistory[i].Next = (Next > MemoryTypeInformation[i].NumberOfPages) ? Next : MemoryTypeInformation[i].NumberOfPages;
#endif
        // We are never decreasing the memory type usage values.
        // It would've been more fair to check for inequality (!=) here to 
        // keep memory type information consistent with the actual memory usage.
        // We are not doing it to workaround UEFI Windows 7 and Windows 8 bug. 
        // Windows loader can't properly handle (it crashes)
        // memory map changes that happen after OS load has been launched
        // and before the ExitBootServices call. 
        // It's very difficult to predict how much memory will be used during 
        // the execution of the Windows loader because in certain cases Windows loader 
        // is pretty active. For example, it sometimes calls 
        // ConnectController for all the devices.
        // By never decreasing the memory type usage values, we are avoiding the problem 
        // by always assuming the worst case scenario (the heaviest memory usage).
        // The drawback is, we are stealing more memory than is actually used from the user.
        if (Next > MemoryTypeInformation[i].NumberOfPages){
            if ( IsRuntimeMemoryType(MemoryTypeInformation[i].Type) )
                RtMemoryQuotasIncreased = TRUE;
            MemoryTypeInformation[i].NumberOfPages = Next;
            MemoryTypeInformationModified = TRUE;
        }
    }

    // If any changes were made to the Memory Type Information settings,
    // set the new variable value;
    if ( MemoryTypeInformationModified ){
        TRACE((TRACE_DXE_CORE, "   Memory    Previous  Current    Next   \n"));
        TRACE((TRACE_DXE_CORE, "    Type      Pages     Pages     Pages  \n"));
        TRACE((TRACE_DXE_CORE, "===========  ========  ========  ========\n"));
        for (i = 0; MemoryTypeInformation[i].Type != EfiMaxMemoryType; i++) {
            TRACE((
                TRACE_DXE_CORE, "%s %8X  %8X  %8X\n", 
                EfiMemTypeStr[MemoryTypeInformation[i].Type], 
                MemoryInfoHistory[i].Previous,
                MemoryInfoHistory[i].Current,
                MemoryInfoHistory[i].Next
            ));
        }
        Status = pRS->SetVariable(
            EFI_MEMORY_TYPE_INFORMATION_VARIABLE_NAME, &gEfiMemoryTypeInformationGuid,
            EFI_VARIABLE_NON_VOLATILE  | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
            MemoryTypeInformationSize, MemoryTypeInformation
        );
        ASSERT_EFI_ERROR(Status);
        if (IsFirstBoot && RtMemoryQuotasIncreased){
            TRACE(( 
                TRACE_DXE_CORE, 
                "Default RT memory quotas have been increased. Resetting the system...\n"
            ));
#if NV_SIMULATION != 1
            pRS->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
#endif
        }
    }
    pBS->CloseEvent(Event);
    MemoryTypeInformationIsSaved = TRUE;
}

VOID RegisterMemoryTypeInformationUpdateCallback(){
	EFI_EVENT Event;
    VOID *Registration;
    
    EFI_BOOT_MODE BootMode = GetBootMode();
    if (   BootMode == BOOT_ON_S4_RESUME 
        || BootMode == BOOT_ON_FLASH_UPDATE
        || BootMode == BOOT_IN_RECOVERY_MODE
    ) return;
    
    // We really want to get control.
    // That's why we are registering callbacks for multiple boot events hoping that
    // at least one of them will be triggered.
    // If multiple events are signaled, only the first one is handled
    RegisterProtocolCallback(
        &BeforeBootProtocolGuid,
        SaveMemoryTypeInformation,
        NULL, &Event, &Registration
    );
    RegisterProtocolCallback(
        &BeforeLegacyBootProtocolGuid,
        SaveMemoryTypeInformation,
        NULL, &Event, &Registration
    );
    CreateLegacyBootEvent(TPL_CALLBACK, &SaveMemoryTypeInformation, NULL, &Event);
	pBS->CreateEvent(
        EVT_SIGNAL_EXIT_BOOT_SERVICES,TPL_CALLBACK, 
        &SaveMemoryTypeInformation, NULL, &Event
    );
}

VOID CallTheDispatcher(){
	DXE_SERVICES *pDxe;
    
    if (EFI_ERROR(LibGetDxeSvcTbl(&pDxe)))
        return;
	if (pDxe) pDxe->Dispatch();
	
	TrEEPhysicalPresenceLibProcessRequest(NULL);	

	SignalProtocolEvent(&BdsDispatcherProtocolGuid);
}

VOID SignalAllDriversConnectedEvent(){
    SignalProtocolEvent(&BdsAllDriversConnectedProtocolGuid);
}

VOID HandoffToTse(){
    AMI_POST_MANAGER_PROTOCOL *AmiPostMgr=NULL;

    if (!EFI_ERROR(pBS->LocateProtocol(
            &AmiPostMgrProtocolGuid, NULL, &AmiPostMgr
    ))) AmiPostMgr->Handshake();

}

VOID BdsEntry (IN EFI_BDS_ARCH_PROTOCOL *This)
{
	
	UINTN i;

	PERF_END (NULL, "DXE", NULL, 0);
	PERF_START (NULL, "BDS", NULL, 0);

	//InitParts2(TheImageHandle, pST);
    //Lang & LangCodes are initialized by Setup driver
	InitSystemVariable(L"Timeout",	sizeof(UINT16), &DefaultTimeout);

{
    EFI_EVENT Event;
    VOID      *Registration;
#ifdef EFI_DXE_PERFORMANCE
    RegisterProtocolCallback(
        &BeforeBootProtocolGuid,
        SavePerformanceData,
        NULL, &Event, &Registration
    );
    RegisterProtocolCallback(
        &BeforeLegacyBootProtocolGuid,
        SavePerformanceData,
        NULL, &Event, &Registration
    );

#endif
    RegisterProtocolCallback(
        &BeforeLegacyBootProtocolGuid,
        SaveFpdtDataOnLegacyBoot,
        NULL, &Event, &Registration
    );

}
	pBS->LocateHandleBuffer(AllHandles, NULL, NULL, &NumberOfHandles, &RootHandles);

	for(i=0; BdsControlFlowFunctions[i]!=NULL; i++){
        TRACE((TRACE_DXE_CORE, "BDS.%s(%X)\n", 
            BdsControlFlowFunctionNames[i], BdsControlFlowFunctions[i]
        ));
        BdsControlFlowFunctions[i]();
    }
	PERF_END (NULL, "BDS", NULL, 0);
}

VOID SetSystemTableFirmwareInfo(){
    EFI_STATUS Status;
    UINTN Size = (Wcslen((CHAR16*)FirmwareVendorString)+1)*sizeof(CHAR16);

    // =========== Initialize System Table Fields
    Status = pBS->AllocatePool(EfiRuntimeServicesData, Size, (VOID**)&pST->FirmwareVendor);
    if (!EFI_ERROR(Status))
        Wcscpy(pST->FirmwareVendor, (CHAR16*)FirmwareVendorString);
	pST->FirmwareRevision = FirmwareRevision;
}

// Deprecated Variable.
// Use TheImageHandle instead.
EFI_HANDLE ThisImageHandle = NULL;

EFI_STATUS EFIAPI BdsInit (IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_HANDLE Handle = NULL;
	InitAmiLib(ImageHandle, SystemTable);
    ThisImageHandle = ImageHandle;
    SetSystemTableFirmwareInfo();
	return pBS->InstallProtocolInterface(&Handle, &gEfiBdsArchProtocolGuid, EFI_NATIVE_INTERFACE, &BDS);
}



//////////////////////////////// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
EFI_STATUS ReadBootOptionAndBoot()
{
	UINTN OptionSize, BootOrderSize,i;
	UINT16 *BootOrder=NULL;
	EFI_LOAD_OPTION *Option=NULL;
	CHAR16 BootVarName[15]; //Boot0000

	EFI_STATUS Status = GetEfiVariable(
		L"BootOrder", &EfiVariableGuid, NULL, &BootOrderSize, (VOID**)&BootOrder
	);

	if (EFI_ERROR(Status)) return Status;
    for(i=0; i< BootOrderSize/sizeof(UINT16); i++){
	    Swprintf(BootVarName,L"Boot%04x",BootOrder[i]);
    	Status = GetEfiVariable(
    		BootVarName, &EfiVariableGuid, NULL, &OptionSize, (VOID**)&Option
    	);
        if (EFI_ERROR(Status)) continue;
        Boot(Option,BootOrder[i],OptionSize);
    }
	pBS->FreePool(BootOrder);
	pBS->FreePool(Option);
	return Status;
}

#define EFI_MEMORY_PRESENT      0x0100000000000000ULL
#define EFI_MEMORY_INITIALIZED  0x0200000000000000ULL
#define EFI_MEMORY_TESTED       0x0400000000000000ULL

VOID RecoverTheMemoryAbove4Gb(){
    DXE_SERVICES *gDS;
    UINTN NumberOfDescriptors;
    EFI_GCD_MEMORY_SPACE_DESCRIPTOR *MemorySpaceMap;
    UINTN Index;
    EFI_STATUS Status;
    if (EFI_ERROR(LibGetDxeSvcTbl(&gDS))) return;
    Status = gDS->GetMemorySpaceMap (&NumberOfDescriptors, &MemorySpaceMap);
    if (EFI_ERROR(Status)) return;

    for (Index = 0; Index < NumberOfDescriptors; Index++) {
        if (MemorySpaceMap[Index].GcdMemoryType != EfiGcdMemoryTypeReserved) continue;
        if (    (MemorySpaceMap[Index].Capabilities & (EFI_MEMORY_PRESENT | EFI_MEMORY_INITIALIZED | EFI_MEMORY_TESTED))
             != (EFI_MEMORY_PRESENT | EFI_MEMORY_INITIALIZED)
        ) continue;

        // For those reserved memory that have not been tested, simply promote to system memory.
        gDS->RemoveMemorySpace (
            MemorySpaceMap[Index].BaseAddress, MemorySpaceMap[Index].Length
        );
        gDS->AddMemorySpace (
            EfiGcdMemoryTypeSystemMemory,
            MemorySpaceMap[Index].BaseAddress,MemorySpaceMap[Index].Length,
              MemorySpaceMap[Index].Capabilities 
            & ~(EFI_MEMORY_PRESENT | EFI_MEMORY_INITIALIZED | EFI_MEMORY_TESTED | EFI_MEMORY_RUNTIME)
        );

    }
    pBS->FreePool (MemorySpaceMap);
}

//////////////////////////////// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
