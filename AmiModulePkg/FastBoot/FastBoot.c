//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
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
// $Header: /Alaska/SOURCE/Modules/PTT/FastBoot.c 31    6/13/12 8:49a Bibbyyeh $
//
// $Revision: 31 $
//
// $Date: 6/13/12 8:49a $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  FastBoot.c
//
//  Description:
//  Implementation of fast boot functionality
//
//<AMI_FHDR_END>
//*************************************************************************

// ===========================================================================
// Includes
// ===========================================================================
#include <Token.h>
#include <AmiDxeLib.h>
#include <Setup.h>
#include <Protocol/BlockIo.h>
#include <Protocol/LoadedImage.h>

#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciIo.h>
#include <Protocol/ConsoleControl.h>
#include <Pci.h>
#include <Protocol/AMIPostMgr.h>
#include <Protocol/FastBootProtocol.h>
#include <Protocol/AmiUsbController.h>	//(EIP85135+)
#include <FastBootLinks.h>
#include <Protocol/UsbPolicy.h>
#include <FastBoot.h>
#if AMIUSB_SUPPORT
#include "USB\rt\usbdef.h"
#endif

#if CSM_SUPPORT
#include <Protocol/LegacyBios.h>
#endif

// ===========================================================================
// Define
// ===========================================================================
                                        //(EIP71257+)>
#if !defined(AMITSE_SUPPORT) || (!AMITSE_SUPPORT)
    #ifndef TSE_MAJOR
        #define TSE_MAJOR 0
    #endif
    #ifndef TSE_MINOR
        #define TSE_MINOR 0
    #endif     
#endif
                                        //<(EIP71257+)
#define AMI_MEDIA_DEVICE_PATH_GUID \
    { 0x5023b95c, 0xdb26, 0x429b, 0xa6, 0x48, 0xbd, 0x47, 0x66, 0x4c, 0x80, 0x12 }

#define BOOT_IA32  44     //size in bytes of string L"EFI\\BOOT\\BOOTIA32.EFI"
#define BOOT_X64   42     //size in bytes of string L"EFI\\BOOT\\BOOTx64.EFI"
#define	BOOT_FLOW_CONDITION_FAST_BOOT 7
#define BOOT_FLOW_CONDITION_NORMAL  0

#define EFI_SHELL_PROTOCOL_GUID \
  {0x47C7B223, 0xC42A, 0x11D2, 0x8E, 0x57, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}

#if defined(TSE_CAPITAL_BOOT_OPTION) && (TSE_CAPITAL_BOOT_OPTION)
#define gBootName L"Boot%04X"
#else
#define gBootName L"Boot%04x"
#endif

// ===========================================================================
// External Variable Declaration
// ===========================================================================
extern EFI_GUID gEfiGlobalVariableGuid;
extern EFI_GUID AmiPostMgrProtocolGuid;
extern EFI_GUID ConInStartedProtocolGuid;
extern EFI_GUID ConOutStartedProtocolGuid;
extern EFI_GUID BdsAllDriversConnectedProtocolGuid;
extern EFI_GUID gEfiPciIoProtocolGuid;
extern EFI_GUID gEfiUsbPolicyProtocolGuid;
extern EFI_HANDLE ThisImageHandle;

// ===========================================================================
// External Function Declaration
// ===========================================================================
VOID ReportConnectConOutProgressCode();
VOID ReportConnectConInProgressCode();
VOID InstallConOutStartedProtocol();
VOID InstallConInStartedProtocol();
VOID ConnectVgaConOut();
VOID ConnectPs2ConIn();
VOID ConnectUsbConIn();
VOID ConnectConInVariable();
VOID InstallConsoleStartedProtocol(
    CHAR16* ConDevVar, EFI_GUID* ProtocolGuid
);
VOID ReadyToBoot(UINT16 OptionNumber);
VOID ConnectDevicePath(IN EFI_DEVICE_PATH_PROTOCOL *pPath);
VOID InstallFwLoadFile(VOID);
EFI_DEVICE_PATH_PROTOCOL* DiscoverPartition(
    IN EFI_DEVICE_PATH_PROTOCOL *DevicePath
);

#ifdef EFI_DXE_PERFORMANCE
VOID SavePerformanceData(IN EFI_EVENT Event, IN VOID *Context);
#endif

extern VOID RecoverTheMemoryAbove4Gb();
// ===========================================================================
// Golbal Variable Declaration
// ===========================================================================
struct {
    EFI_DEVICE_PATH_PROTOCOL Header;
    CHAR16 FileName[22];
} FilePathNode = {
    {
        MEDIA_DEVICE_PATH,
        MEDIA_FILEPATH_DP,
#ifdef EFIx64
        BOOT_X64 + 4
#else
        BOOT_IA32 + 4
#endif
    },
#ifdef EFIx64
    { L"EFI\\BOOT\\BOOTx64.EFI"}
#else
    { L"EFI\\BOOT\\BOOTIA32.EFI" }
#endif
};

struct {
    EFI_DEVICE_PATH_PROTOCOL Header;
    CHAR16 FileName[20];
} TestFilePathNode = {
    {
        MEDIA_DEVICE_PATH,
        MEDIA_FILEPATH_DP,
#ifdef EFIx64
        38 + 4
#else
        40 + 4
#endif
    },
#ifdef EFIx64
    { L"EFI\\DP\\BOOTx64.EFI"}
#else
    { L"EFI\\DP\\BOOTIA32.EFI" }
#endif
};

static EFI_GUID AmiMediaDevicePathGuid = AMI_MEDIA_DEVICE_PATH_GUID;
static EFI_GUID FastBootVariableGuid = FAST_BOOT_VARIABLE_GUID;
static EFI_GUID guidBootFlow = BOOT_FLOW_VARIABLE_GUID;
FAST_BOOT_TSE_PROTOCOL *gFastBootTseProtocol=NULL;  //(EIP63924+)
EFI_HANDLE      EFIBootImageHanlde = NULL;
EFI_HANDLE      LegacyBootDeviceHandle = NULL;
EFI_HANDLE      *RootHandles;
UINTN           NumberOfHandles;
SETUP_DATA      FbSetupData;
static BOOLEAN  Runtime = FALSE;
FAST_BOOT_POLICY    *gFastBootPolicy;
BOOLEAN FBUsbSkipTableIsSet = FALSE;
UINT8   *BackupSkipTable = NULL;
EFI_USB_POLICY_PROTOCOL *gUsbPolicyProtocol = NULL;
BOOLEAN BackupUsbMassDriverSupport;

static AMI_FAST_BOOT_PROTOCOL FastBootProtocol = { 
    NULL, 
    FbConnectInputDevices,
    IsRuntime
};

// ===========================================================================
// Function Definitions
// ===========================================================================
BOOLEAN CapsulePresent(VOID);
EFI_STATUS ConnectFastEfiBootDevice();
EFI_STATUS ConnectFastLegacyBootDevice();

//============================================================================
// Elinks and Hooks
//============================================================================
typedef BOOLEAN (IsFastBootElink)(
    IN SETUP_DATA *Setupdata
);
extern IsFastBootElink IS_FAST_BOOT_LIST EndOfIsFastBootList;
IsFastBootElink* IsFastBootList[] = {IS_FAST_BOOT_LIST NULL};
										//(EIP62683+)>
typedef VOID (AfterAllDrirverConnectElink)();
extern AfterAllDrirverConnectElink AFTER_ALL_DRIVER_CONNECT_HOOK EndOfList;
AfterAllDrirverConnectElink* AfterAllDriverConnectList[] = {AFTER_ALL_DRIVER_CONNECT_HOOK NULL};
										//<(EIP62683+)
										//(EIP63924+)>
typedef BOOLEAN (FastBootCheckModeChangeElink)();
extern FastBootCheckModeChangeElink FAST_BOOT_CHECK_MODE_CHANGE_HOOK EndOfFastBootModeChangeList;
FastBootCheckModeChangeElink* FastBootModeChange[] = {FAST_BOOT_CHECK_MODE_CHANGE_HOOK NULL};
										//<(EIP63924+)
										//<(EIP62845+)
typedef VOID (BeforeConnectFastBootDeviceElink)();	//(EIP85135+)
extern BeforeConnectFastBootDeviceElink BEFORE_CONNECT_FAST_BOOT_DEVICE_HOOK EndOfBeforeConnectFastBootDeviceElink;
BeforeConnectFastBootDeviceElink* BeforeConnectFastBootDeviceHook[] = {BEFORE_CONNECT_FAST_BOOT_DEVICE_HOOK NULL};
										//<(EIP62845+)
typedef VOID (ReturnNormalModeElink)();
extern ReturnNormalModeElink RETURN_NORMAL_MODE_HOOK EndOfReturnNormalModeElink;
ReturnNormalModeElink* ReturnNormalModeHook[] = {RETURN_NORMAL_MODE_HOOK NULL};

extern IS_VALID_FASTBOOT_BOOT_OPTION_FUNC_PTR  IS_VALID_FASTBOOT_BOOT_OPTION_FUNC;
IS_VALID_FASTBOOT_BOOT_OPTION_FUNC_PTR *IsValidFBBootOptionPtr = IS_VALID_FASTBOOT_BOOT_OPTION_FUNC;

//============================================================================
// Procedures
//============================================================================

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FbAddDevicePath
//
// Description: 
//  Makes a new device path with pDp1 as its first instance and pDp2 as its 
//  second instance, then returns the result in a newly allocated buffer.
//
// Input:       
//  IN EFI_DEVICE_PATH_PROTOCOL *pDp1 - the first instance
//  IN EFI_DEVICE_PATH_PROTOCOL *pDp2 - the second instance
//
// Output:     
//  EFI_DEVICE_PATH_PROTOCOL, the new devcie path that includes pDp1 and pDP2.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_DEVICE_PATH_PROTOCOL* 
FbAddDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL *pDp1, 
  IN EFI_DEVICE_PATH_PROTOCOL *pDp2 )
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

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FbGetPciHandlesByClass
//
// Description: 
//  Get PCI IO handle list that Calss code is match with input parameter 
//  Class and SubClass.
//
// Input:       
//  IN  UINT8 Class - Pci class code 
//  IN  UINT8 SubClass - Pci sub class code
//  OUT UINTN *NumberOfHandles - The number of handles that match the 
//                               indicated class code.
//  OUT EFI_HANDLE **HandleBuffer - The handle buffer of PCI IO handle.
//
// Output:      
//  EFI_STATUS 
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS 
FbGetPciHandlesByClass (
  IN  UINT8 Class, 
  IN  UINT8 SubClass, 
  OUT UINTN *NumberOfHandles, 
  OUT EFI_HANDLE **HandleBuffer )
{
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
		Status=pBS->HandleProtocol(Handle[i],&gEfiPciIoProtocolGuid,&PciIo);
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

#if CSM_SUPPORT
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FbInstallOnBoardVgaOpRom
//
// Description: Check and install pci oprom for VGA device.
//
// Input:       
//  IN UINTN HdlNum - Number of Pci Io handle
//  IN EFI_HANDLE *pHandle - Pci Io handle list
//  IN EFI_LEGACY_BIOS_PROTOCOL *LegacyBios - EFI_LEGACY_BIOS_PROTOCOL pointer
//  IN BOOLEAN OnBoard - OnBoard (TRUE) or OffBoard (FALSE)
//
// Output:      
//  BOOLEAN TRUE - Legacy Oprom is installed successfully
//          FALSE -  Oprom is installed failure.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN 
FbInstallOnBoardVgaOpRom (
  IN UINTN HdlNum,
  IN EFI_HANDLE *pHandle,
  IN EFI_LEGACY_BIOS_PROTOCOL *LegacyBios, 
  IN BOOLEAN OnBoard )
{
    UINTN   i;
    EFI_STATUS  Status;
    UINT64 PciAttributes;    
    UINTN   Flags;
    UINT64  Capabilities;	
    
	for(i=0; i<HdlNum; i++)	{
    	EFI_PCI_IO_PROTOCOL *PciIo;
    	EFI_DEVICE_PATH_PROTOCOL *Dp;
    	UINT8 PciClass;
    	Status=pBS->HandleProtocol(pHandle[i],&gEfiPciIoProtocolGuid,&PciIo);
    	if (EFI_ERROR(Status)) continue;
        
		Status=PciIo->Pci.Read(PciIo, EfiPciIoWidthUint8, 0xB, 1, &PciClass);
		if (EFI_ERROR(Status)) continue;        
		if (PciClass!=PCI_CL_DISPLAY) continue;
        
		Status=pBS->HandleProtocol(pHandle[i],&gEfiDevicePathProtocolGuid,&Dp); 
		if (EFI_ERROR(Status)) continue;
        
        Status = PciIo->Attributes(
            PciIo, EfiPciIoAttributeOperationGet, 0, &PciAttributes
        );

        if ( !EFI_ERROR(Status) && (PciAttributes & EFI_PCI_IO_ATTRIBUTE_EMBEDDED_DEVICE) && OnBoard)        
            continue;

										//(EIP75718)>
        Status = PciIo->Attributes (PciIo,
                                    EfiPciIoAttributeOperationSupported, 0,
                                    &Capabilities);     // Get device capabilities
        if (EFI_ERROR(Status)) continue;

        Status = PciIo->Attributes (
                        PciIo,
                        EfiPciIoAttributeOperationEnable,
                        //Capabilities & EFI_PCI_DEVICE_ENABLE,
                        Capabilities & (EFI_PCI_DEVICE_ENABLE | EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY | EFI_PCI_IO_ATTRIBUTE_VGA_IO),
                        NULL);              // Enable device
        if (EFI_ERROR(Status)) goto done; 
              

        Status = LegacyBios->CheckPciRom ( LegacyBios,
                                           pHandle[i],
                                           NULL,
                                           NULL,
                                           &Flags);
        if (EFI_ERROR(Status) || (Flags != 2)) goto done;
               
        Status = LegacyBios->InstallPciRom (
                              LegacyBios,
                              pHandle[i],
                              NULL,
                              &Flags,
                              NULL,
                              NULL,
                              NULL,
                              NULL
                              );

done:        
        if (!EFI_ERROR(Status)) return TRUE;
        else {
            
            if (PciIo != NULL) {     
            //
            // Turn off the PCI device and disable forwarding of VGA cycles to this device
            //
            PciIo->Attributes (
                    PciIo,
                    EfiPciIoAttributeOperationDisable,
                    Capabilities & EFI_PCI_DEVICE_ENABLE | EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY | EFI_PCI_IO_ATTRIBUTE_VGA_IO,
                    NULL);
            }
        }
										//<(EIP75718)
    }
    return FALSE;
}
#endif

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FbInstallVgaOpRom
//
// Description: 
//  Install and exectued VGA legacy Oprom. If Legacy Vga OpRom is not 
//  installed successfully, then try to connect EFI VGA driver.
//
// Input:   NONE       
//
// Output:  NONE    
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
FbInstallVgaOpRom()
{
#if CSM_SUPPORT
	EFI_STATUS Status;
	EFI_HANDLE *Handle;
	UINTN Number;
    BOOLEAN VgaInstalled = FALSE;
    EFI_LEGACY_BIOS_PROTOCOL *LegacyBios;

    //Locate LegacyBios Protocol
    Status = pBS->LocateProtocol(&gEfiLegacyBiosProtocolGuid, NULL, &LegacyBios);    
    if (EFI_ERROR(Status)) return ;

	//Get a list of all PCI devices
	Status = pBS->LocateHandleBuffer(
        ByProtocol,&gEfiPciIoProtocolGuid, NULL, &Number, &Handle
    );
	if (EFI_ERROR(Status)) return;

    VgaInstalled = FbInstallOnBoardVgaOpRom(Number,Handle,LegacyBios,FALSE);
    if (VgaInstalled == FALSE)    
        VgaInstalled = FbInstallOnBoardVgaOpRom(Number,Handle,LegacyBios,TRUE);        

	pBS->FreePool(Handle);

    //If Legacy Vga OpRom is not installed successfully, then try to connect EFI VGA driver
    if (VgaInstalled == FALSE)    
         ConnectVgaConOut();    
#endif    
}
										//(EIP85135+)>

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsRootBridgeHandle
//
// Description: 
//  Check whether this handle is bridge handle or not.
//
// Input:       
//  IN EFI_HANDLE Handle
//
// Output:      
//  BOOLEAN TRUE - the input handle is pci root bridge handle
//          FALSE - the input handle is not pci root bridge handle 
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN 
IsRootBridgeHandle (
  IN EFI_HANDLE Handle )
{
    EFI_STATUS  Status;
    EFI_DEVICE_PATH_PROTOCOL    *DevicePath;
    EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *PciRootBridgeIo;
    EFI_GUID PciRootBridgeIoProtocolGuid = EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_GUID;

    Status = pBS->HandleProtocol (
                    Handle,
                    &gEfiDevicePathProtocolGuid,
                    &DevicePath
                    );
    if(EFI_ERROR(Status)) return FALSE;

    Status = pBS->HandleProtocol (
                    Handle,
                    &PciRootBridgeIoProtocolGuid,
                    &PciRootBridgeIo
                    );
    if(EFI_ERROR(Status)) return FALSE;

    return TRUE;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ConnectEveryPciHandles
//
// Description: 
//  This function connects all PCI handles excpet PCI devices in 
//  FAST_BOOT_PCI_SKIP_LIST.
//
// Input:   NONE       
//
// Output:  NONE      
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
ConnectEveryPciHandles()
{
    EFI_STATUS  Status;
	UINTN Number;
	EFI_HANDLE *Handle;    
    UINTN i,j;  
    SKIP_PCI_LIST *SkipPciList = gFastBootPolicy->SkipPciList;

	Status = pBS->LocateHandleBuffer(
        ByProtocol,&gEfiPciIoProtocolGuid, NULL, &Number, &Handle
    );
    if (EFI_ERROR(Status)) return;

 	for(i=0; i<Number; i++)
	{
		EFI_PCI_IO_PROTOCOL *PciIo;
		UINT8 PciClass[4];
		Status=pBS->HandleProtocol(Handle[i],&gEfiPciIoProtocolGuid,&PciIo);
		if (EFI_ERROR(Status)) continue;
		Status=PciIo->Pci.Read(PciIo, EfiPciIoWidthUint32, PCI_REV_ID_OFFSET, 1, &PciClass);

        //check skip table
        for (j=0;j<(gFastBootPolicy->SkipPciListSize/(sizeof(SKIP_PCI_LIST)));j++) {

            if (SkipPciList[j].SubClass == 0xFF && PciClass[3] == SkipPciList[j].Class) 
                break;
                
            if (PciClass[3] == SkipPciList[j].Class && PciClass[2] == SkipPciList[j].SubClass)
                break;
        }

        if (j<(gFastBootPolicy->SkipPciListSize/(sizeof(SKIP_PCI_LIST))))
            continue;
                
        //check fastboot policy
        if (gFastBootPolicy->UsbSupport == 0 && PciClass[3] == PCI_CL_SER_BUS && PciClass[2] == PCI_CL_SER_BUS_SCL_USB)
            continue;    

        if(PciClass[3] == PCI_CL_NETWORK && gFastBootPolicy->NetWorkStackSupport == 0) {
            pBS->ConnectController(Handle[i],NULL,NULL,FALSE);
            continue;
        }
        
        //all pass, then we connect this controller
        pBS->ConnectController(Handle[i],NULL,NULL,TRUE);        
            	
	}

    pBS->FreePool(Handle);      
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   FbConnectEverything
//
//  Description:
//   This function connects handles in the system.
//
//  Input:
// 	None
//
//  Output:
//  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
FbConnectEverything()
{
    UINTN i;
       
    for(i=0; i<NumberOfHandles; i++) {
        if(IsRootBridgeHandle(RootHandles[i]))
            ConnectEveryPciHandles();
        else     
            pBS->ConnectController(RootHandles[i],NULL,NULL,TRUE);        
    }

    pBS->FreePool(RootHandles);

// Signal it anyway for Consplitter to take care the ConIn/ConOut 
// after everything is connected
    InstallConsoleStartedProtocol(NULL, &ConOutStartedProtocolGuid);
    InstallConsoleStartedProtocol(NULL, &ConInStartedProtocolGuid);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   FbConnectEverything
//
//  Description:
//   This procedure is executed before elink BDS_CONTROL_FLOW for getting all 
//   system handles.
//
//  Input:
// 	None
//
//  Output:
//  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
BeforeBDSFlow()
{
	pBS->LocateHandleBuffer(AllHandles, NULL, NULL, &NumberOfHandles, &RootHandles);
}
										//<(EIP85135+)

										//(EIP85135+)>

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   SetUsbSkipTable
//
//  Description:
//   Skip certain usb port in fastboot path by setting skip table in usb 
//   protocol. 
//
//  Input:
// 	None
//
//  Output:
//  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>  
#if AMIUSB_SUPPORT
VOID  
SetUsbSkipTable()
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_GUID gEfiUsbProtocolGuid = EFI_USB_PROTOCOL_GUID;    
    EFI_USB_PROTOCOL *UsbProtocol = NULL;    
    USB_GLOBAL_DATA *UsbData = NULL;

    Status = pBS->LocateProtocol( &gEfiUsbProtocolGuid, \
                                      NULL, \
                                      &UsbProtocol );
    if (EFI_ERROR(Status)) return;

//backup skip table pointer
    UsbData = (USB_GLOBAL_DATA*)UsbProtocol->USBDataPtr;

    BackupSkipTable = UsbData->gUsbSkipListTable;

//Set new skip table    
    UsbProtocol->UsbCopySkipTable( \
            (USB_SKIP_LIST*)gFastBootPolicy->UsbSkipTable, \
                     (UINT8)gFastBootPolicy->UsbSkipTableSize);  

}   
#endif
										//<(EIP85135+)
										//(EIP63924)>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FastConnectConsoles
//
// Description: 
//  Connect console in and console out device in fast boot path.
//
// Input: NONE       
//
// Output: NONE      
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
FastConnectConsoles()
{    
    TRACE((-1,"FB:Connect Console...\n"));

    // Connect Console Out
    ReportConnectConOutProgressCode();              

    if (gFastBootPolicy->VGASupport || gFastBootPolicy->UEfiBoot == TRUE){  
        TRACE((-1,"FB:Connect Vga\n"));    
        ConnectVgaConOut();          
    } else {
        TRACE((-1,"FB:Install Vga OpRom Only\n"));            
        FbInstallVgaOpRom();        
    }
        
    InstallConOutStartedProtocol();               

    // Connect Console In
    ReportConnectConInProgressCode();
    
	//EIP154575(+) >>
    if (gFastBootPolicy->Ps2Support){
        TRACE((-1,"FB:Connect Ps2\n"));                        
        ConnectPs2ConIn();
    }    
	//EIP154575(+) <<

#if AMIUSB_SUPPORT            
										//(EIP85135)>
    if (gFastBootPolicy->UsbSupport >= 1){
                       
        if (gFastBootPolicy->UsbSupport == 2){  

            //Disable Usb storage driver support
            pBS->LocateProtocol(&gEfiUsbPolicyProtocolGuid, NULL, &gUsbPolicyProtocol);
            if (gUsbPolicyProtocol != NULL) {
                BackupUsbMassDriverSupport = gUsbPolicyProtocol->UsbDevPlcy->UsbMassDriverSupport;
                gUsbPolicyProtocol->UsbDevPlcy->UsbMassDriverSupport = FALSE;
            }
            
            TRACE((-1,"FB:Set Usb Skip Table\n"));           
            FBUsbSkipTableIsSet = TRUE;
            SetUsbSkipTable();
        }
        TRACE((-1,"FB:Connect Usb\n")); 
        ConnectUsbConIn();
    } 
#else 
    ConnectUsbConIn(); // If AMIUSB_SUPPORT disable, always connect USB device
#endif  
										//<(EIP85135)
	//EIP154575(-) >>
    //if (gFastBootPolicy->Ps2Support){
    //    TRACE((-1,"FB:Connect Ps2\n"));                        
    //    ConnectPs2ConIn();
    //}    
	//EIP154575(-) <<
    InstallConInStartedProtocol();
    
    TRACE((-1,"FB:Connect Console...End\n"));
}
										//<(EIP63924)

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FastBoot
//
// Description: 
//  FastBoot entry point
//
// Input:       None	
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
FastBoot(VOID)
{
										//(EIP63924+)>
    EFI_STATUS Status;
    EFI_GUID    FastBootTseGuid = FAST_BOOT_TSE_PROTOCOL_GUID;    
    
    Status = pBS->LocateProtocol(&FastBootTseGuid, NULL, &gFastBootTseProtocol);
    if (EFI_ERROR(Status)) return;
										//<(EIP63924+)
    gFastBootPolicy->InFastBootPath = TRUE;										
    FastBootWorker();

//if we're here - fast boot failed
    ReturnToNormalBoot();
    gFastBootPolicy->InFastBootPath = FALSE;										
    TRACE((-1,"FB: FastBoot Failure, return to BDS\n"));
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   RemoveFilePathNode
//
// Description: 
//  Remove file path node from Device path Dp.
//
// Input:       
//  IN EFI_DEVICE_PATH_PROTOCOL *Dp
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID RemoveFilePathNode (
  IN EFI_DEVICE_PATH_PROTOCOL *Dp )
{
    EFI_DEVICE_PATH_PROTOCOL *FileDpNode = NULL;
    UINTN   FileDpNodeSize;
    EFI_DEVICE_PATH_PROTOCOL *TempDp = Dp;    
    UINTN DpSize = DPLength(Dp);
    UINTN RemainingDpLength=0;
    UINT8 ReaminingDpBuffer[100];
    
    for( ; !(isEndNode(TempDp)); TempDp = NEXT_NODE(TempDp)) {
        if(TempDp->Type == MEDIA_DEVICE_PATH && 
            TempDp->SubType == MEDIA_FILEPATH_DP) {
            FileDpNode = TempDp;
            break;
        }
    }

    if (FileDpNode == NULL) return;
    
    RemainingDpLength = DPLength(FileDpNode);
    FileDpNodeSize = NODE_LENGTH(FileDpNode);

    pBS->SetMem(ReaminingDpBuffer,100,0);

    pBS->CopyMem(ReaminingDpBuffer,FileDpNode,RemainingDpLength);

    pBS->SetMem(FileDpNode,RemainingDpLength,0);

    pBS->CopyMem(FileDpNode,
        (VOID*)((UINTN)ReaminingDpBuffer+FileDpNodeSize),
        RemainingDpLength - FileDpNodeSize);
}

#ifdef EFI_DXE_PERFORMANCE
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UpdatePolicyForTestMode
//
// Description: 
//  Update FastBootPolicy setting for test mode. The file path of BootX64 is 
//  changed from EFI\\BOOT\\BOOTx64.EFI to EFI\\DP\\BOOTx64.EFI
//
// Input:       None	
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
UpdatePolicyForTestMode()
{
    BOOLEAN UsbMassStorage = FALSE;
    BOOLEAN HddFilePath = FALSE;
    EFI_DEVICE_PATH_PROTOCOL *Dp = gFastBootPolicy->FastBootOption;
    EFI_DEVICE_PATH_PROTOCOL *NewDp=NULL;
    EFI_DEVICE_PATH_PROTOCOL *TempDp=NULL;    
    UINTN   NewDpSize;
	UINT32  LoadOptionsSize = 0;
	VOID    *LoadOptions;
    EFI_STATUS  Status;    

    if(gFastBootPolicy->TestMode == FALSE || gFastBootPolicy->UEfiBoot == FALSE) return;

    if(!IsSupportedDevice(gFastBootPolicy->FastBootOption, &UsbMassStorage,&HddFilePath))
        return;

    LoadOptions = (UINT8*)Dp + DPLength(Dp);
    LoadOptionsSize = *(UINT32*)LoadOptions;   

	NewDpSize = DPLength(Dp)+ LoadOptionsSize+ sizeof(UINT32)+ \
        NODE_LENGTH(&TestFilePathNode.Header);
   
    Status = pBS->AllocatePool(EfiBootServicesData,NewDpSize,&NewDp);
    if(EFI_ERROR(Status)) return;
   
    pBS->SetMem(NewDp,NewDpSize,0);
    pBS->CopyMem(NewDp,Dp,DPLength(Dp));

    //Replace file path from "efi\boot\" to "efi\dp\"
    RemoveFilePathNode(NewDp);      
    TempDp = DPAddNode(NewDp, &TestFilePathNode.Header);

    pBS->CopyMem(NewDp,TempDp,DPLength(TempDp));

    //Copy optional data
    pBS->CopyMem( (VOID*)((UINTN)NewDp+DPLength(NewDp)),
                   LoadOptions, 
                   LoadOptionsSize+sizeof(UINT32));


    gFastBootPolicy->FastBootOption = NewDp;
    pBS->FreePool(TempDp);

    gFastBootPolicy->CheckBootOptionNumber = FALSE;

}
#endif

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ShowFastBootPolicy
//
// Description: 
//  Output the debug message to show current fast boot plicy setting.
//
// Input:       None	
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
ShowFastBootPolicy()
{
    EFI_DEVICE_PATH_PROTOCOL *Dp = gFastBootPolicy->FastBootOption;    
    UINTN   FastBootOptionSize=0;
	VOID    *LoadOptions;
	UINT32  LoadOptionsSize = 0;
    UINT8   *ptr8=NULL;  
    SKIP_PCI_LIST *PciSkipList=NULL;
    UINTN   i;
    USB_SKIP_LIST  *usbskip = NULL; //(EIP96276.7)+

    TRACE((-1,"FB: ==================================================\n"));
    TRACE((-1,"FB: FastBootPolicy->FastBootEnable = %x\n",gFastBootPolicy->FastBootEnable));
    TRACE((-1,"FB: FastBootPolicy->TestMode = %x\n",gFastBootPolicy->TestMode));

    TRACE((-1,"FB: FastBootPolicy->UEfiBoot = %x\n",gFastBootPolicy->UEfiBoot));
    TRACE((-1,"FB: FastBootPolicy->BootOptionNumber = %x\n",gFastBootPolicy->BootOptionNumber));
    TRACE((-1,"FB: FastBootPolicy->DevStrCheckSum = %x\n",gFastBootPolicy->DevStrCheckSum));

    FastBootOptionSize = DPLength(Dp);
    if (gFastBootPolicy->UEfiBoot) {
        LoadOptions = (UINT8*)Dp + DPLength(Dp);
        LoadOptionsSize = *(UINT32*)LoadOptions;  
        FastBootOptionSize += LoadOptionsSize+sizeof(UINT32);
    }
    
    for (i=0,ptr8=(UINT8*)Dp;i<FastBootOptionSize;i++)
        TRACE((-1,"FB: FastBootPolicy->FastBootOption[%d] = %x\n",i,ptr8[i]));        

    TRACE((-1,"FB: FastBootPolicy->LastBootFailure = %x\n",gFastBootPolicy->LastBootFailure));
    TRACE((-1,"FB: FastBootPolicy->LastBootVarPresence = %x\n",gFastBootPolicy->LastBootVarPresence));
    TRACE((-1,"FB: FastBootPolicy->BootCount = %x\n",gFastBootPolicy->BootCount));


    TRACE((-1,"FB: FastBootPolicy->CheckBootOptionNumber = %x\n",gFastBootPolicy->CheckBootOptionNumber));
    TRACE((-1,"FB: FastBootPolicy->CheckDevStrCheckSum = %x\n",gFastBootPolicy->CheckDevStrCheckSum));
    TRACE((-1,"FB: FastBootPolicy->VGASupport = %x\n",gFastBootPolicy->VGASupport));
    TRACE((-1,"FB: FastBootPolicy->UsbSupport = %x\n",gFastBootPolicy->UsbSupport));

                                        //(EIP96276.7)>
    usbskip = gFastBootPolicy->UsbSkipTable;
    for (i = 0; i < gFastBootPolicy->UsbSkipTableSize; i++)
    {
        TRACE((-1,"FB: FastBootPolicy->UsbSkipTable[%d] = {%02X,%02X,%04X,%02X,%08X,%02X}\n",i,usbskip->bSkipType,usbskip->bFlag,usbskip->wBDF,usbskip->bRootPort,usbskip->dRoutePath,usbskip->bBaseClass));
        usbskip++;
    }
                                        //<(EIP96276.7)

    TRACE((-1,"FB: FastBootPolicy->Ps2Support = %x\n",gFastBootPolicy->Ps2Support));
    TRACE((-1,"FB: FastBootPolicy->NetWorkStackSupport = %x\n",gFastBootPolicy->NetWorkStackSupport));

    for (i=0,PciSkipList = gFastBootPolicy->SkipPciList;i<gFastBootPolicy->SkipPciListSize/sizeof(SKIP_PCI_LIST);i++)
        TRACE((-1,"FB: FastBootPolicy->SkipPciList[%d] = Class:%x,SubClass:%x\n",i,PciSkipList[i].Class,PciSkipList[i].SubClass));

    TRACE((-1,"FB: FastBootPolicy->CheckPassword = %x\n",gFastBootPolicy->CheckPassword));

    TRACE((-1,"FB: FastBootPolicy->SkipTSEHandshake = %x\n",gFastBootPolicy->SkipTSEHandshake));

    TRACE((-1,"FB: ==================================================\n"));

}
                                        //(EIP96276.4)>>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsValidFastBootOption
//
// Description: 
//  This function check the boot option and parse the device path to knwo this
//  boot option is belong to LegacyType, UsbDevice, HardDrive, CDROM, NetWork,
//  or WindowsBootManager.
//
// Input:      
// IN UINTN BootOrderIndex - Index number of BootOrder
// IN EFI_LOAD_OPTION *BootOption - The boot option of 
//                                  BootOrder[BootOrderIndex]
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN 
IsValidFastBootOption(
  IN UINTN BootOrderIndex, 
  IN EFI_LOAD_OPTION *BootOption )
{    
    EFI_DEVICE_PATH_PROTOCOL *Dp;
    CHAR16 *Description;
    CHAR16 *gWindowsBootManagerStr = L"Windows Boot Manager";    
    BOOLEAN LegacyType = FALSE;
    BOOLEAN UsbDevice = FALSE;
    BOOLEAN HardDrive = FALSE;
    BOOLEAN CDROM = FALSE;
    BOOLEAN NetWork = FALSE;
    BOOLEAN WindowsBootManager = FALSE;
    
//check attribute

    if (!(BootOption->Attributes & LOAD_OPTION_ACTIVE)) return FALSE;

//check windows boot manager
	
    Description = (CHAR16 *)(BootOption + 1);
    if(!MemCmp(gWindowsBootManagerStr,Description,Strlen((char*)gWindowsBootManagerStr)))
        WindowsBootManager = TRUE;
    
//check device type
	
    Dp = (EFI_DEVICE_PATH_PROTOCOL *)((UINT8*)Description + (Wcslen(Description) + 1) * sizeof(CHAR16));
    
    for( ; !(isEndNode(Dp)); Dp = NEXT_NODE(Dp)) {
        if(Dp->Type == BBS_DEVICE_PATH) LegacyType = TRUE; 
	
        if(Dp->Type == MESSAGING_DEVICE_PATH && Dp->SubType == MSG_USB_DP)
            UsbDevice = TRUE;
	
        if(Dp->Type == MESSAGING_DEVICE_PATH && Dp->SubType == MSG_MAC_ADDR_DP)
            NetWork = TRUE;        
	
        if(Dp->Type == MEDIA_DEVICE_PATH && Dp->SubType == MEDIA_HARDDRIVE_DP) 
            HardDrive = TRUE;
        
        if(Dp->Type == MEDIA_DEVICE_PATH && Dp->SubType == MEDIA_CDROM_DP) 
            CDROM = TRUE;
    }
	
    TRACE((-1,"FB: BootOrder Index %x\n",BootOrderIndex));
    TRACE((-1,"FB: Windows Boot Manager: %x\n",WindowsBootManager));
    TRACE((-1,"FB: Legacy %x,Usb %x, HardDrive %x, CDROM %x, NetWork %x\n", \
        LegacyType,UsbDevice,HardDrive,CDROM,NetWork));    
    
// there are some samples for selecting a boot option as fast boot device.
/*
//Case 1. Fast boot with windows boot manager and it's the highest priority in boot order.
    if(BootOrderIndex == 0 && WindowsBootManager == 1) return TRUE;

//Case 2. don't support Legacy device
    if(LegacyType == TRUE) return FALSE;

//Case3. don't support usb type
    if(UsbDevice == TRUE) return FALSE;
*/

    return TRUE;   
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsOneOfDP
//
// Description: 
//  Determines whether a device path, pAll, contains the device path, pOne.  
//
// Input:       
//  IN EFI_DEVICE_PATH_PROTOCOL *pAll - The device path to be scanned.
//  IN EFI_DEVICE_PATH_PROTOCOL *pOne - The device path to locate within pAll.
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN 
IsOneOfDP(
  IN EFI_DEVICE_PATH_PROTOCOL *pAll, 
  IN EFI_DEVICE_PATH_PROTOCOL *pOne )
{
    EFI_DEVICE_PATH_PROTOCOL *pPath;
    
    pPath = pOne;

    do{
        if (isEndNode(pAll)) return FALSE;        
        
        if (DPLength(pAll) < DPLength(pOne)) return FALSE;
    
        if(!(pAll->Type == pPath->Type && pAll->SubType == pPath->SubType))
        {
            pAll = NEXT_NODE(pAll);
            continue;
        }

        if (!MemCmp(pAll,pAll,DPLength(pOne)))
            return TRUE;
        else
            pAll = NEXT_NODE(pAll);
        
    }while(TRUE);
    
}
                                        //<<(EIP96276.4)
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FastBootWorker
//
// Description: FastBoot main function
//
// Input:       None	
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
FastBootWorker(VOID)
{
    EFI_EVENT  FastBootReadyToBootEvent;
    CHAR16     BootOptionName[9];
	EFI_LOAD_OPTION *BootOption = NULL;    
    UINTN      Size;
    EFI_STATUS Status;
    UINT16     *BootOrder = NULL;
    UINT32     BootFlow;
    UINT8       i;                      //(EIP62683+)
    BOOLEAN     VaildBootOption = FALSE;
    UINTN       BootOptionSize; 		//(EIP96276.4+)
    TRACE((-1, "FB: FastBoot started\n"));
	Size = sizeof(BootFlow);
	Status = pRS->GetVariable(L"BootFlow", &guidBootFlow, NULL, &Size, (VOID *)&BootFlow);
	if(!EFI_ERROR(Status) && (BootFlow != BOOT_FLOW_CONDITION_NORMAL)) {
		// Some other driver wants different bootflow - abandon fast boot.
		return;
	}

#if CAPSULE_SUPPORT
    if(CapsulePresent())
        return; 
#endif

    if(OemConfigurationChanged())
        return;   

    Status = CreateReadyToBootEvent(TPL_CALLBACK,
                                    FastBootReadyToBootNotify,
                                    NULL,
                                    &FastBootReadyToBootEvent);
	if (EFI_ERROR(Status)) return;

    if (!gFastBootPolicy->LastBootVarPresence) return;
        
#ifdef EFI_DXE_PERFORMANCE
        UpdatePolicyForTestMode();
#endif

    ShowFastBootPolicy();        
        
    if (gFastBootPolicy->CheckBootOptionNumber) {
    
        Size = 0;
        Status = GetEfiVariable(L"BootOrder", &gEfiGlobalVariableGuid, NULL, &Size, &BootOrder);

        for (i=0;i<Size/sizeof(UINT16);i++,BootOption = NULL) {	//(EIP96276.4)

            Swprintf(BootOptionName, gBootName, BootOrder[i]);
            Status = GetEfiVariable(BootOptionName, &gEfiGlobalVariableGuid, NULL, &BootOptionSize, &BootOption);//(EIP96276.4)
            if(EFI_ERROR(Status))
                return;         
            
            VaildBootOption = IsValidFBBootOptionPtr(i,BootOption);
            if (VaildBootOption) break;

            pBS->FreePool(BootOption);

            if (VaildBootOption == FALSE && i == (Size/sizeof(UINT16) -1 )) {
                TRACE((-1, "FB: There is no valid boot option\n"));
                pBS->FreePool(BootOrder);
                return;

            }
        }
        
        TRACE((-1, "FB: LastBootOption=%x, BootOrder[%d]=%x\n",gFastBootPolicy->BootOptionNumber,i,BootOrder[i]));
        
        if(gFastBootPolicy->BootOptionNumber != BootOrder[i]) 
        {
        //
        // BootOption Number check fail, then check device path.
        // If device path is also wrong, abort the fast boot.
        //                
            EFI_DEVICE_PATH_PROTOCOL *Dp;
            CHAR16                   *Description;   
            Description = (CHAR16 *)(BootOption + 1);                
            Dp = (EFI_DEVICE_PATH_PROTOCOL *)((UINT8*)Description + (Wcslen(Description) + 1) * sizeof(CHAR16));

            if(!IsOneOfDP(gFastBootPolicy->FastBootOption,Dp)) {
                pBS->FreePool(BootOrder);
                pBS->FreePool(BootOption);
                return;
            }
            
            TRACE((-1,"FB: Update new boot option number\n")); 
            gFastBootPolicy->BootOptionNumber = BootOrder[i];               
        }

        pBS->FreePool(BootOrder);
        pBS->FreePool(BootOption);
    }
   
    FastConnectConsoles();
										//(EIP62683+)>
    // before all driver connect elink 
    for (i=0;BeforeConnectFastBootDeviceHook[i]; i++)
        BeforeConnectFastBootDeviceHook[i](); //(EIP85135)

										//<(EIP62683+)

    if(gFastBootPolicy->UEfiBoot == TRUE)
        Status = ConnectFastEfiBootDevice();
    else
        Status = ConnectFastLegacyBootDevice();

    if(EFI_ERROR(Status)) return;

										//(EIP62683+)>
    // after all driver connect elink 
    for (i=0;AfterAllDriverConnectList[i]; i++)
        AfterAllDriverConnectList[i]();
										//<(EIP62683+)
    RecoverTheMemoryAbove4Gb();

    if((gFastBootPolicy->SkipTSEHandshake == 0) && (TSE_MAJOR >= 0x02) && (TSE_MINOR >= 0x10)) {
        Status = TseHandShake();
        if(Status == EFI_ABORTED)   //fast boot failed, proceed to full boot
            return;
    }

                                        //(EIP63924+)>
//Stop CheckForKey callback timer in TSE
        gFastBootTseProtocol->FastBootStopCheckForKeyTimer();
                                        //<(EIP63924+)
//if we are here, somehow we failed launch fast boot through TSE - do it on our own
    
    if(gFastBootPolicy->UEfiBoot == TRUE)
        FastEfiBoot();
    else
        FastLegacyBoot();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsBootToShell
//
// Description: 
//  Function to determine if the boot path is to UEFI Shell
//
// Input:
//  IN EFI_DEVICE_PATH_PROTOCOL *Dp - pointer to device path of bootable device
//
// Output:      
//  BOOLEAN TRUE - if it's UEFI Shell boot path, FALSE otherwise
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN 
IsBootToShell (
  IN EFI_DEVICE_PATH_PROTOCOL *Dp )
{
    if(Dp->Type == MEDIA_DEVICE_PATH &&
       Dp->SubType == MEDIA_VENDOR_DP &&
       !guidcmp(&(((VENDOR_DEVICE_PATH *)Dp)->Guid), &AmiMediaDevicePathGuid))
       return TRUE;
    
    return FALSE;
}
                                        //(EIP63924+)>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CheckPostManagerKey
//
// Description: 
//  Check TSE Post key is pressed or not.
//
// Input:		None
//
// Output:      TRUE - Post key is pressed
//              FALSE - Post key is not pressed
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN 
CheckPostManagerKey()
{
    EFI_STATUS CheckForKeyStatus;
    BOOLEAN EnterSetup;
    UINT32  BootFlow;

    CheckForKeyStatus = gFastBootTseProtocol->FastBootCheckForKey(&EnterSetup,&BootFlow);
    if(CheckForKeyStatus == EFI_SUCCESS) return TRUE;
    return FALSE;  
}
                                        //<(EIP63924+)

                                        //(EIP62845+)>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ConnectAllSataDevices
//
// Description: 
//  Connect All Stat Devcies
//
// Input:		NONE
//
// Output:      NONE 
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
ConnectAllSataDevices()			//(EIP85135)
{
	EFI_STATUS Status;
	EFI_HANDLE *Handle;
	UINTN Number,i;
    if (gFastBootPolicy->ConnectAllSata == FALSE) return;    //(EIP96276.3)+
    
	//Connect all SATA IDE Controllers
	Status = FbGetPciHandlesByClass(
        PCI_CL_MASS_STOR, PCI_CL_MASS_STOR_SCL_IDE, &Number, &Handle
    );

    if(!EFI_ERROR(Status)){
        for(i=0; i<Number; i++){
            pBS->ConnectController(Handle[i],NULL,NULL,TRUE);
        }        
        pBS->FreePool(Handle);
    }
    
	//Connect all SATA AHCI Controllers
	Status = FbGetPciHandlesByClass(
        PCI_CL_MASS_STOR, 0x06, &Number, &Handle
    );

    if(!EFI_ERROR(Status)){
        for(i=0; i<Number; i++){
            pBS->ConnectController(Handle[i],NULL,NULL,TRUE);
        }        
        pBS->FreePool(Handle);
    }

										//(EIP95568+)>
	//Connect all SATA RAID Controllers
	Status = FbGetPciHandlesByClass(
        PCI_CL_MASS_STOR, PCI_CL_MASS_STOR_SCL_RAID, &Number, &Handle
    );

    if(!EFI_ERROR(Status)){
        for(i=0; i<Number; i++){
            pBS->ConnectController(Handle[i],NULL,NULL,TRUE);
        }        
        pBS->FreePool(Handle);
    }
										//<(EIP95568+)
    return;								//(EIP85135)

}
                                        //<(EIP62845+)
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FastBootReadyToBootNotify
//
// Description: 
//  FastBoot ReadyToBoot callback
//
// Input:		
//  IN EFI_EVENT Event - Callback event
//  IN VOID *Context - pointer to calling context
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
FastBootReadyToBootNotify(
	IN EFI_EVENT Event, 
    IN VOID *Context
)
{
    EFI_STATUS               Status;
    UINTN                    i,Size;
    FAST_BOOT                Var;
    UINT16                   *BootOrder = NULL;
    CHAR16                   BootOptionName[9];
	EFI_LOAD_OPTION          *BootOption = NULL;
    UINTN                    BootOptionSize;	//(EIP96276.4+)
    CHAR16                   *Description;
    EFI_DEVICE_PATH_PROTOCOL *Dp;
    UINT32                   BbsPopup;
    BOOLEAN     VaildBootOption = FALSE;

//check if we are on bbs-popup boot path
    Size = sizeof(BbsPopup);
    Status = pRS->GetVariable(L"BbsPopupCalled", &FastBootVariableGuid, NULL, &Size, (VOID *)&BbsPopup);
    if(!EFI_ERROR(Status) && (BbsPopup == 0x55aa55aa))  {   //we are on bbs-popup boot path
        ResetFastBootVariable();    //clear variable if it was created on previous boots
        return;
    }

    Size = sizeof(Var);
    Status = pRS->GetVariable(L"LastBoot", &FastBootVariableGuid, NULL, &Size, (VOID *)&Var);
    if(EFI_ERROR(Status)) {       
        //variable not found - create a new one
        
        //get first actived boot option
        Status = GetEfiVariable(L"BootOrder", &gEfiGlobalVariableGuid, NULL, &Size, &BootOrder);
        if(EFI_ERROR(Status)) return;        

        for (i=0;i<Size/sizeof(UINT16);i++,BootOption=NULL) {	//(EIP96276.4)

            Swprintf(BootOptionName, gBootName, BootOrder[i]);
           
            Status = GetEfiVariable(BootOptionName, &gEfiGlobalVariableGuid, NULL, &BootOptionSize, &BootOption);	//(EIP96276.4)
            if(EFI_ERROR(Status)) return;


            VaildBootOption = IsValidFBBootOptionPtr(i,BootOption);
            if (VaildBootOption) break;
            
            pBS->FreePool(BootOption);
            
            if (VaildBootOption == FALSE && i == (Size/sizeof(UINT16) -1 )) {
                TRACE((-1, "FB: There is no valid boot option\n"));
                pBS->FreePool(BootOrder);
                return;
            }
        }

        Var.BootOptionNumber = BootOrder[i];
        Var.BootCount = 0;

        //retrieve device path
        Description = (CHAR16 *)(BootOption + 1);
	    Dp = (EFI_DEVICE_PATH_PROTOCOL *)((UINT8*)Description + (Wcslen(Description) + 1) * sizeof(CHAR16));

        if(Dp->Type == BBS_DEVICE_PATH) {
            Var.BootType =  FAST_BOOT_TYPE_LEGACY;
            Status = CreateLegacyFastBootOption(&Var);
            TRACE((-1,"FB: Create FastBoot Legacy Boot Option %r\n",Status));                                
        } else {
            Var.BootType = FAST_BOOT_TYPE_EFI;  
            Status = CreateEfiFastBootOption(BootOption, BootOptionSize, BootOrder[i]);	//(EIP96276.4)
            TRACE((-1,"FB: Create FastBoot EFI Boot Option %r\n",Status));                        
        }

        pBS->FreePool(BootOrder);
        pBS->FreePool(BootOption);

        if(EFI_ERROR(Status))
            return;
    } else {
        Var.BootCount++;
        Var.BootOptionNumber = gFastBootPolicy->BootOptionNumber;
    }
   
    TRACE((-1,"FB: Create LastBoot Variable\n"));  
    pRS->SetVariable(L"LastBoot",
                     &FastBootVariableGuid,
                     EFI_VARIABLE_NON_VOLATILE | 
                     EFI_VARIABLE_BOOTSERVICE_ACCESS | 
                     EFI_VARIABLE_RUNTIME_ACCESS,
                     sizeof(Var),
                     (VOID *)&Var);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ConnectFastLegacyBoot
//
// Description: 
//  Connect Legacy FastBoot Device
//
// Input:       None
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS 
ConnectFastLegacyBootDevice()
{
#if CSM_SUPPORT 
    EFI_STATUS Status;
    UINTN Size = 0;
    UINT8 *BootOption = NULL;
    EFI_DEVICE_PATH_PROTOCOL *Dp;
    BOOLEAN UsbMassStorage = FALSE;
    BOOLEAN HddFilePath = FALSE;
    EFI_BLOCK_IO_PROTOCOL *BlockIo;
    EFI_LEGACY_BIOS_PROTOCOL *LegacyBios;
    UINT16 HddCount;
    UINT16 BbsCount;
    HDD_INFO *HddInfo;
    BBS_TABLE *BbsTable;
    EFI_HANDLE  Handle;
    UINT8 i;  							//(EIP63924+)
    TRACE((-1, "FB: Connect Fast LegacyBoot Device\n"));     
     
    Dp = gFastBootPolicy->FastBootOption;
    if(!IsSupportedDevice(Dp, &UsbMassStorage, &HddFilePath))         //CD-ROM legacy boot not supported
        return EFI_NOT_FOUND;

    if ( UsbMassStorage && gFastBootPolicy->UsbSupport == 0)
        return EFI_NOT_FOUND;  //If skip usb enable, don't boot into usb mass storage     
        
    TRACE((-1, "FB: Dp connecting\n"));
    ConnectDevicePath(Dp);
    Status = pBS->LocateDevicePath(&gEfiBlockIoProtocolGuid, &Dp, &Handle);
    if(EFI_ERROR(Status))
        return Status;
    TRACE((-1, "FB: Dp connected Handle %x\n",Handle));
        
    LegacyBootDeviceHandle = Handle;
        
    Status = pBS->HandleProtocol(Handle, &gEfiBlockIoProtocolGuid, &BlockIo);
    if(EFI_ERROR(Status))
        return Status;
    TRACE((-1, "FB: BlockIo found\n"));

    Status = pBS->LocateProtocol(&gEfiLegacyBiosProtocolGuid, NULL, &LegacyBios);
    if (EFI_ERROR(Status)) 
        return Status;
    TRACE((-1, "FB: Legacybios discovered\n"));

										//(EIP63924)>
    // check FastBootModeChange
    for (i=0;FastBootModeChange[i]; i++) 
        if(FastBootModeChange[i]())return EFI_NOT_FOUND;
										//<(EIP63924)
        
    Status = LegacyBios->GetBbsInfo(LegacyBios, &HddCount, &HddInfo, &BbsCount, &BbsTable);  //install int 13
    if (EFI_ERROR(Status)) return EFI_NOT_FOUND;

#endif 
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FastLegacyBoot
//
// Description: 
//  FastBoot with legacy device
//
// Input:       None
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS 
FastLegacyBoot(VOID)
{
#if CSM_SUPPORT 
    UINTN Size = 0;
    EFI_STATUS Status;
    UINT8 *BootOption = NULL;
    UINT8 Index;	
    EFI_LEGACY_BIOS_PROTOCOL *LegacyBios;
    UINT16 HddCount;
    UINT16 BbsCount;
    HDD_INFO *HddInfo;
    BBS_TABLE *BbsTable;
    EFI_EVENT  FastBootLegacyBootEvent;
    UINT8   *pDeviceString;
    UINT8   i;
    UINT16   StringCheckSum;

    TRACE((-1, "FB: Fast legacy boot started\n"));

										//(EIP87390+)>
	pBS->RaiseTPL( TPL_HIGH_LEVEL );	// guarantees that RestoreTPL won't ASSERT
	pBS->RestoreTPL( TPL_APPLICATION );
										//<(EIP87390+)
	Status = pBS->LocateProtocol(&gEfiLegacyBiosProtocolGuid, NULL, &LegacyBios);
	if (EFI_ERROR(Status)) 
        return Status;
    TRACE((-1, "FB: Legacybios discovered\n"));
        
    LegacyBios->GetBbsInfo(LegacyBios, &HddCount, &HddInfo, &BbsCount, &BbsTable);  //install int 13

										//(EIP87390+)>
    //Set USB device BootPriority as BBS_DO_NOT_BOOT_FROM, Install USB INT13 service for TCG Bitlocker Driver
    for (Index=0;Index<BbsCount;Index++){       

                                        //(EIP103422+)>
        // if BBS table priority have been arranged already 
        // increase one priority to all prioritized BBS table entry
        if (BbsTable[Index].BootPriority < BBS_DO_NOT_BOOT_FROM) { 
            BbsTable[Index].BootPriority = BbsTable[Index].BootPriority + 1;
            continue;
        }
                                        //<(EIP103422+)         

        // If BBS table priority have not been arranged yet, set device 
        // priority as BBS_DO_NOT_BOOT_FROM.
        // It means don't boot from this devcie but BIOS still need to install
        // INT13 service for it.

        if (BbsTable[Index].BootPriority != BBS_IGNORE_ENTRY)
            BbsTable[Index].BootPriority = BBS_DO_NOT_BOOT_FROM;                    
            
    }
										//<(EIP87390+)

    for (Index=0;Index<BbsCount;Index++){
    
        if (BbsTable[Index].IBV1 == (UINT32)LegacyBootDeviceHandle) break;
    
        if (Index == (BbsCount-1)) return Status; //can't find fast device in bbstable 
    }
    
    if(gFastBootPolicy->CheckDevStrCheckSum) {
        
        pDeviceString = (UINT8*)((UINTN)((UINTN)BbsTable[Index].DescStringSegment<<4) + BbsTable[Index].DescStringOffset);
        StringCheckSum =0;    
        for (i=0;i<50;i++) {        
            if (pDeviceString[i] == 0) break;
            StringCheckSum+=pDeviceString[i];
        }        
         
        if (StringCheckSum != gFastBootPolicy->DevStrCheckSum) return EFI_DEVICE_ERROR;
  
    }

    BbsTable[Index].BootPriority = 0;

    Status = CreateLegacyBootEvent(TPL_CALLBACK,
                                    FastBootClearBootFailFlag,
                                    NULL,
                                    &FastBootLegacyBootEvent);
	if (EFI_ERROR(Status)) 
        return Status; 

    TRACE((-1,"FB: LEGACY BOOT Singal Ready to Boot\n"));

    ReadyToBoot(0xffff);

                                        //(EIP68329)>
    if((SKIP_TSE_HANDSHAKE == 0) && (TSE_MAJOR >= 0x02) && (TSE_MINOR >= 0x10)) {            
    } else {
        gFastBootTseProtocol->FastBootPasswordFreezeDevices();
    }
                                        //<(EIP68329)

//Set Runtime as TRUE before pass control to OS.
    Runtime = TRUE;

    LegacyBios->LegacyBoot(LegacyBios, 0, 0, 0);
    
    return Status;
#else
    return EFI_SUCCESS;
#endif    
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ConnectFastEfiBootDevice
//
// Description: 
//  Connect UEFI FastBoot Device
//
// Input:       None
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS 
ConnectFastEfiBootDevice()
{
    EFI_STATUS  Status;
    EFI_DEVICE_PATH_PROTOCOL *Dp = gFastBootPolicy->FastBootOption;
    UINT8 *BootOption = NULL;
    UINTN Size = 0;
    EFI_HANDLE  Handle;    
    UINT8   i;                          //(EIP63924+)
    TRACE((-1, "FB: Connect Fast EFIBoot Device\n"));
    
    if(IsBootToShell(Dp))
        InstallFwLoadFile();
    else {
        ConnectDevicePath(Dp);
        Status = pBS->LocateDevicePath(&gEfiBlockIoProtocolGuid, &Dp, &Handle);
        if(EFI_ERROR(Status)) return Status;
        TRACE((-1, "FB: Dp connected Handle %x\n",Handle));

        Status = CheckLoader(gFastBootPolicy->FastBootOption);
        if(EFI_ERROR(Status)) {
            TRACE((-1, "FB: Check loader %r\n",Status));            
            return Status;
        }

    }
   										//(EIP63924)>
    for (i=0;FastBootModeChange[i]; i++) 
        if(FastBootModeChange[i]())return EFI_NOT_FOUND;
										//<(EIP63924)

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	_ShellClearScreen
//
// Description:	
//  Clears the screen for shell boot
//
// Input:		
//  IN EFI_EVENT Event - Callback event
//  IN VOID *Context - pointer to calling context
//
// Output:		NONE
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID _ShellClearScreen (EFI_EVENT Event, VOID *Context)
{
    pST->ConOut->ClearScreen (pST->ConOut);
	pBS->CloseEvent (Event);
	pST->ConOut->EnableCursor (pST->ConOut, TRUE);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	_RegisterShellGuid
//
// Description:	
//  Registers the shell guid
//
// Input:		NONE
//
// Output:		NONE
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
_RegisterShellGuid (VOID)
{
	EFI_STATUS 	Status = EFI_UNSUPPORTED;
	VOID 		*Registration = NULL;
	EFI_GUID 	EfiShellInterfaceGuid = EFI_SHELL_PROTOCOL_GUID;
    EFI_EVENT   ShellLaunchEvent;
	Status = pBS->CreateEvent (
				EFI_EVENT_NOTIFY_SIGNAL, 
				TPL_CALLBACK,
				_ShellClearScreen,
				NULL,
				&ShellLaunchEvent);
	if (!EFI_ERROR (Status))
	{
		Status = pBS->RegisterProtocolNotify(
				&EfiShellInterfaceGuid,
				ShellLaunchEvent,
				&Registration
				);
	}
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FastEfiBoot
//
// Description: 
//  UEFI FastBoot path
//
// Input:       None
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS 
FastEfiBoot(VOID) 
{
    EFI_STATUS Status;
    UINT8 *BootOption = NULL;
    UINTN Size = 0;
    UINT16 OptionNumber = 0xFFFF;
    EFI_DEVICE_PATH_PROTOCOL *Dp = NULL;
    UINTN DpSize;
	UINT32 LoadOptionsSize;
	VOID *LoadOptions;
	EFI_LOADED_IMAGE_PROTOCOL *Image;

    TRACE((-1, "FB: Fast EFI boot started\n"));

    DpSize = DPLength(gFastBootPolicy->FastBootOption);
    LoadOptionsSize = *(UINT32 *)((UINT8 *)gFastBootPolicy->FastBootOption + DpSize);
    LoadOptions = (VOID *)((UINT8 *)gFastBootPolicy->FastBootOption + DpSize + sizeof(UINT32));                         

	// this *MUST* be run a EFI_TPL_APPLICATION
	pBS->RaiseTPL( TPL_HIGH_LEVEL );	// guarantees that RestoreTPL won't ASSERT (EIP87390)+
	pBS->RestoreTPL( TPL_APPLICATION );

    TRACE((-1,"FB: EFI BOOT Signal Ready to Boot\n"));

//
//  ReadyToBoot have signaled already in TSE path.
//
    if (gFastBootPolicy->SkipTSEHandshake)
        ReadyToBoot(gFastBootPolicy->BootOptionNumber);

    
    Status = pBS->LoadImage(TRUE, ThisImageHandle, gFastBootPolicy->FastBootOption, NULL, 0, &EFIBootImageHanlde);

    TRACE((-1,"FB: load image %r\n",Status));
    
    if (EFI_ERROR(Status)) return Status;        
    if(LoadOptionsSize != 0) {
        Status = pBS->HandleProtocol(EFIBootImageHanlde, &gEfiLoadedImageProtocolGuid, &Image);
        if (EFI_ERROR(Status)) 
            return Status;

        Image->LoadOptionsSize = LoadOptionsSize;   
        Image->LoadOptions = LoadOptions;
    }    
                                        //(EIP68329)>
    if((gFastBootPolicy->SkipTSEHandshake == 0) && (TSE_MAJOR >= 0x02) && (TSE_MINOR >= 0x10)) {            
    } else {
        gFastBootTseProtocol->FastBootPasswordFreezeDevices();
    }
                                        //<(EIP68329)
#ifdef EFI_DXE_PERFORMANCE
    SavePerformanceData(NULL, NULL);
#endif

    FastBootClearBootFailFlag(NULL, NULL);

                                        //(EIP60794+)>
    if (pST->ConOut != NULL) {
        pST->ConOut->EnableCursor(pST->ConOut,FALSE);
    }

    _RegisterShellGuid();    
                                        //<(EIP60794+)
                                        
    TRACE((-1,"FB: EFI BOOT start image\n"));        
//enabld usb mass driver, so that OS have chance to connect usb mass storage.
    if (gUsbPolicyProtocol != NULL) 
        gUsbPolicyProtocol->UsbDevPlcy->UsbMassDriverSupport = TRUE;
//Set Runtime as TRUE before pass control to OS.
    Runtime = TRUE;
    PERF_END (NULL, "BDS", NULL, 0);	// (EIP130784+)
    IoWrite8(0x80,0xFB);    //output 80port to show system boot with fast boot path.
    Status = pBS->StartImage(EFIBootImageHanlde, NULL, NULL);
     
    return Status;     
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CreateLegacyFastBootOption
//
// Description: 
//  This function creates Legacy fast boot option and save it to NVRAM
//
// Input:      
//  IN FAST_BOOT *Var - The Variable contains the fast boot information 
//                      for next boot.
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS 
CreateLegacyFastBootOption ( 
  IN FAST_BOOT *Var )
{
#if CSM_SUPPORT
    EFI_STATUS Status;
    EFI_LEGACY_BIOS_PROTOCOL *LegacyBios;
    UINT16 HddCount;
    UINT16 BbsCount;
    HDD_INFO *HddInfo;
    BBS_TABLE *BbsTable;
    EFI_HANDLE Handle;
    EFI_DEVICE_PATH_PROTOCOL *Dp;
    UINT8 BootOption[200];
    UINTN DpSize;
    UINT8 i;
    UINT8 Index = 0;                    //(EIP73019+)
    UINT16 Priority = 0xfffc;
    UINT8   *pDeviceString;
    UINT16   StringCheckSum=0;

    Status = pBS->LocateProtocol(&gEfiLegacyBiosProtocolGuid, NULL, &LegacyBios);
    if(EFI_ERROR(Status))
        return Status;

	LegacyBios->GetBbsInfo(LegacyBios, &HddCount, &HddInfo, &BbsCount, &BbsTable);

//find record with highest priority
    for(i = 0; i < BbsCount; i++) {
        if(BbsTable[i].BootPriority < Priority) {
            Priority = BbsTable[i].BootPriority;
            Index = i;
        }
    }

    if(BbsTable[Index].DeviceType != BBS_HARDDISK)
        return EFI_UNSUPPORTED;

    Handle = (EFI_HANDLE) *(UINTN*)(&(BbsTable[Index].IBV1));
    Status = pBS->HandleProtocol(Handle, &gEfiDevicePathProtocolGuid, &Dp);
    if(EFI_ERROR(Status))
        return Status;

    //save device string check sum    
    pDeviceString = (UINT8*)((UINTN)((UINTN)BbsTable[Index].DescStringSegment<<4) + BbsTable[Index].DescStringOffset);
    
    for (i=0;i<50;i++) {        
        if (pDeviceString[i] == 0) break;
        StringCheckSum+=pDeviceString[i];
    }

    Var->DevStrCheckSum = StringCheckSum;

    DpSize = DPLength(Dp);
    MemCpy(BootOption, Dp, DpSize);

    Status = pRS->SetVariable(L"FastBootOption",
                              &FastBootVariableGuid,
                              EFI_VARIABLE_NON_VOLATILE | 
                              EFI_VARIABLE_BOOTSERVICE_ACCESS | 
                              EFI_VARIABLE_RUNTIME_ACCESS,
                              DpSize,
                              BootOption);
    
    return Status;
#else
    return EFI_SUCCESS;    
#endif 
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsSupportedDevice
//
// Description: This function detects if FastBoot is supported for given device
//
// Input:
//  IN EFI_DEVICE_PATH_PROTOCOL *Dp - pointer to device path of bootable device
//  OUT BOOLEAN *UsbMassStorage - TRUE if device is USB device, FALSE otherwise
//
// Output:      TRUE if FastBoot is supported for given device, FALSE otherwise
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN 
IsSupportedDevice(
    IN EFI_DEVICE_PATH_PROTOCOL *Dp,
    OUT BOOLEAN *UsbMassStorage,
    OUT BOOLEAN *HddFilePath )
{
    BOOLEAN USBDev = FALSE;

    for( ; !(isEndNode(Dp)); Dp = NEXT_NODE(Dp)) {
        if(Dp->Type == BBS_DEVICE_PATH)
            return FALSE;

        if(Dp->Type == MESSAGING_DEVICE_PATH) {
            if(Dp->SubType == MSG_USB_DP) 
                USBDev = TRUE;
    
            if(Dp->SubType == MSG_MAC_ADDR_DP)
                return FALSE;        

            continue;
        }

        if(Dp->Type == MEDIA_DEVICE_PATH) {
            
            if(Dp->SubType == MEDIA_HARDDRIVE_DP) {

                if(USBDev == TRUE) 
                    *UsbMassStorage = TRUE;
                
                Dp = NEXT_NODE(Dp);
                //check whether there is a file path behind hard drive path.
                if(Dp->SubType == MEDIA_FILEPATH_DP)
                    *HddFilePath = TRUE;  

                return TRUE;
            }

            if(Dp->SubType == MEDIA_CDROM_DP)
                return FALSE;              
        }
    }
    
    return TRUE;    
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CheckLoader
//
// Description: This function checks if OS loader present on given device
//
// Input:
//  IN EFI_DEVICE_PATH_PROTOCOL *Dp - pointer to device path of bootable device
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS 
CheckLoader(
  IN EFI_DEVICE_PATH_PROTOCOL *Dp )
{
    EFI_STATUS Status;
    EFI_HANDLE Handle;

    Status = pBS->LoadImage(FALSE, ThisImageHandle, Dp, NULL, 0, &Handle);
	if (EFI_ERROR(Status)) 
        return Status;

    Status = pBS->UnloadImage(Handle);
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CreateEfiFastBootOption
//
// Description: This function creates UEFI fast boot option and save it to NVRAM
//
// Input:
//  IN EFI_LOAD_OPTION *BootOption - pointer to regular boot option to boot to
//  IN UINTN OptionSize - size of option additional parameters
//  IN UINT16 OptionNumber - number of boot option in BootOrder variable
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS 
CreateEfiFastBootOption(
  IN EFI_LOAD_OPTION *BootOption,
  IN UINTN           OptionSize,
  IN UINT16          OptionNumber
)
{
    EFI_STATUS Status;
    UINTN Size;
    UINTN DpSize;
    EFI_DEVICE_PATH_PROTOCOL *Dp;
    EFI_DEVICE_PATH_PROTOCOL *PartitionDp;
    CHAR16 *Description;
	UINT32 LoadOptionsSize;
	VOID *LoadOptions;
    UINT8 *FastBootOption;
    VOID *SavePtr;
    BOOLEAN UpdatedDp = FALSE;
    BOOLEAN UsbMassStorage = FALSE;
    BOOLEAN HddFilePath = FALSE;
//retrieve device path
    Description = (CHAR16 *)(BootOption + 1);
	Dp = (EFI_DEVICE_PATH_PROTOCOL *)((UINT8*)Description + (Wcslen(Description) + 1) * sizeof(CHAR16));
    
    if(!IsSupportedDevice(Dp, &UsbMassStorage,&HddFilePath))
        return EFI_UNSUPPORTED;

    //only hard drive or USB can come here

    LoadOptions = (UINT8*)Dp + BootOption->FilePathListLength;
    LoadOptionsSize = (UINT32)((UINT8*)BootOption + OptionSize - (UINT8 *)LoadOptions);

    if(Dp->Type == MEDIA_DEVICE_PATH && Dp->SubType == MEDIA_HARDDRIVE_DP) {    //Windowns boot manager?
        PartitionDp = DiscoverPartition(Dp);
        if(PartitionDp == NULL)
            return EFI_UNSUPPORTED;
        Dp = DPAdd(PartitionDp, NEXT_NODE(Dp));
        UpdatedDp = TRUE;
    } else if (Dp->Type == MEDIA_DEVICE_PATH && Dp->SubType == MEDIA_VENDOR_DP) { // bulit in shell?

    } else if (HddFilePath == FALSE) { //add file path for SATA Hdd or USB Hdd without file path
        Dp = DPAddNode(Dp, &FilePathNode.Header);
        Status = CheckLoader(Dp);
        if(EFI_ERROR(Status)) {
            pBS->FreePool(Dp);
            return EFI_UNSUPPORTED;
        }
        UpdatedDp = TRUE;

    } 

    DpSize = DPLength(Dp);

//prepare data
    Size = DpSize + sizeof(UINT32) + LoadOptionsSize;
    Status = pBS->AllocatePool(EfiBootServicesData, Size, &FastBootOption);
    SavePtr = FastBootOption;
    MemCpy(FastBootOption, Dp, DpSize);
    FastBootOption += DpSize;
    *(UINT32 *)FastBootOption = LoadOptionsSize;
    FastBootOption += sizeof(UINT32);
    MemCpy(FastBootOption, LoadOptions, LoadOptionsSize);

    Status = pRS->SetVariable(L"FastBootOption", 
                              &FastBootVariableGuid,
                              EFI_VARIABLE_NON_VOLATILE | 
                              EFI_VARIABLE_BOOTSERVICE_ACCESS | 
                              EFI_VARIABLE_RUNTIME_ACCESS,
                              Size,
                              SavePtr);

    pBS->FreePool(SavePtr);
    if(UpdatedDp)
        pBS->FreePool(Dp);
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ReturnToNormalBoot
//
// Description: 
//  This function execute e-links "ReturnNormalMode".
//
// Input:   NONE
//
// Output:  NONE
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
ReturnToNormalBoot(VOID)
{
    UINT8   i;
    for (i=0;ReturnNormalModeHook[i]; i++)
        ReturnNormalModeHook[i]();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ResetFastBootVariable
//
// Description: 
//  This function resets FastBoot variable if FastBoot path failed.
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
ResetFastBootVariable(VOID)
{
    UINTN Size = sizeof(FAST_BOOT);
    FAST_BOOT Var;

    Runtime = FALSE;

//clear LastBootFailed variable if present
    FastBootClearBootFailFlag(NULL, NULL);

    if (!gFastBootPolicy->LastBootVarPresence) return;

    pRS->SetVariable(L"LastBoot", 
                     &FastBootVariableGuid,
                     EFI_VARIABLE_NON_VOLATILE,
                     0,
                     &Var);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DisconnectUsbController
//
// Description: This function resets skip table for Usb driver and disconnect 
//				all usb controllers
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
DisconnectUsbController(VOID)
{
    EFI_STATUS    Status = EFI_SUCCESS;
    EFI_HANDLE *Handle;    
    UINTN Number,i;    
#if AMIUSB_SUPPORT
    EFI_GUID gEfiUsbProtocolGuid = EFI_USB_PROTOCOL_GUID;
    EFI_USB_PROTOCOL *UsbProtocol = NULL;
    USB_GLOBAL_DATA   *UsbData = NULL;
#endif

#if AMIUSB_SUPPORT
    if (gFastBootPolicy->UsbSupport != 2) return;
    
    //Restore Usb storage driver support
    if (gUsbPolicyProtocol != NULL) 
        gUsbPolicyProtocol->UsbDevPlcy->UsbMassDriverSupport = BackupUsbMassDriverSupport;
    
    if (FBUsbSkipTableIsSet == FALSE) return;
          
    Status = pBS->LocateProtocol( &gEfiUsbProtocolGuid, \
                                      NULL, \
                                      &UsbProtocol );
    if (EFI_ERROR(Status)) return;

    //restore backup skip table pointer
    UsbData = (USB_GLOBAL_DATA*)UsbProtocol->USBDataPtr;    
    UsbData->gUsbSkipListTable = BackupSkipTable;
#endif

    //Get a list of all USB Controllers
    Status = FbGetPciHandlesByClass(
        PCI_CL_SER_BUS, PCI_CL_SER_BUS_SCL_USB, &Number, &Handle
    );
    if (EFI_ERROR(Status)) return;
    
    for(i=0; i<Number; i++)
    {
        pBS->DisconnectController(Handle[i],NULL,NULL);
    }
    pBS->FreePool(Handle);     
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   OemConfigurationChanged
//
// Description: This function checks if configuration was changed since last boot
//
// Input:       None
//
// Output:      TRUE if configuration was changed, FALSE otherwise
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN 
OemConfigurationChanged(VOID)
{
    EFI_STATUS Status;
    UINT32 BootFlow;
    UINTN Size = sizeof(UINT32);

    Status = pRS->GetVariable(L"BootFlow", &guidBootFlow, NULL, &Size, &BootFlow);

    return (!EFI_ERROR(Status) && BootFlow == BOOT_FLOW_CONDITION_FIRST_BOOT) ? TRUE : FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FastBootClearBootFailFlag
//
// Description: FastBoot clear boot fail flag callback
//
// Input:		
//  IN EFI_EVENT Event - Callback event
//  IN VOID *Context - pointer to calling context
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
FastBootClearBootFailFlag (
  IN EFI_EVENT Event, 
  IN VOID *Context )
{
    EFI_STATUS Status;
    UINT32 LastBootFailed;
    UINTN Size = sizeof(UINT32);

    Status = pRS->GetVariable(L"LastBootFailed", &FastBootVariableGuid, NULL, &Size, &LastBootFailed);
    if(!EFI_ERROR(Status)) {
        Status = pRS->SetVariable(L"LastBootFailed", 
                                  &FastBootVariableGuid, 
                                  EFI_VARIABLE_NON_VOLATILE,
                                  0,
                                  &LastBootFailed);
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsFastBoot
//
// Description: This function FastBoot is enabled or disabled by checking 
//              elink "IsFastBootList"
//
// Input:       None
//
// Output:      TRUE if fast boot is enabled, FALSE otherwise
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN 
IsFastBoot(VOID)
{
    static EFI_GUID SetupVariableGuid = SETUP_GUID;    
    EFI_STATUS  Status;
	UINTN         Size;
    BOOLEAN Result = TRUE;
    UINTN   i;
    EFI_GUID FastBootPolicyGuid = FAST_BOOT_POLICY_PROTOCOL_GUID;

    Status = pBS->LocateProtocol(&FastBootPolicyGuid,NULL,&gFastBootPolicy);
    if (EFI_ERROR(Status) || gFastBootPolicy->FastBootEnable == FALSE)
        return FALSE;

    Size = sizeof(FbSetupData);
    Status = pRS->GetVariable(L"Setup", &SetupVariableGuid, NULL, &Size, &FbSetupData);
    if (EFI_ERROR(Status)) return FALSE;


    // check IsFastBootList
    for (i=0;IsFastBootList[i] && Result; i++)
        Result = IsFastBootList[i](&FbSetupData);


    if (!Result)
        ReturnToNormalBoot();    //return to normal boot

    return Result;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsFastBootDeafult
//
// Description: This function returns the status of Fast boot setup option
//
// Input:       
//  IN  SETUP_DATA  *FbSetupData - ptr of SetupData
//
// Output:      TRUE if fast boot is enabled, FALSE otherwise
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN 
IsFastBootDefault (
  IN SETUP_DATA *FbSetupData )
{
    static EFI_GUID SetupVariableGuid = SETUP_GUID;

    UINT16        BootNext;
	UINTN         Size;
    EFI_STATUS    Status;
    EFI_BOOT_MODE BootMode;
    BOOLEAN       FastBoot;

    if (!gFastBootPolicy->FastBootEnable) return FALSE;

	BootMode = GetBootMode();
    
    if (BootMode == BOOT_WITH_MINIMAL_CONFIGURATION) 
    return TRUE;        

    if (!gFastBootPolicy->FirstFastBootInS4 && BootMode == BOOT_ON_S4_RESUME) {
        if (gFastBootPolicy->LastBootVarPresence)
            return (gFastBootPolicy->BootCount > 0) ? TRUE : FALSE;        
    }

//Check for "BootNext" variable
    Size = sizeof(BootNext);
    Status = pRS->GetVariable(L"BootNext", &gEfiGlobalVariableGuid, NULL, &Size, &BootNext);
    if(!EFI_ERROR(Status)) {
        FastBoot = FALSE;
    } else {
        FastBoot = gFastBootPolicy->FastBootEnable;
    }

	return FastBoot;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ChangeSetupBootFlow
//
// Description: Changea Setup Boot Flow
//
// Input:       UINT32 BootFlow       
//
// Output:      EFI_STATUS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS 
ChangeSetupBootFlow (
  IN UINT32 BootFlow )
{
    EFI_STATUS  Status;
    EFI_GUID guidBootFlow = BOOT_FLOW_VARIABLE_GUID;    

    Status = pRS->SetVariable(L"BootFlow", 
                               &guidBootFlow, 
                               EFI_VARIABLE_BOOTSERVICE_ACCESS,
                               sizeof(BootFlow),
                               &BootFlow);
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsAmtBoot
//
// Description: Do NOT perform FastBoot if AMT boot is request.
//
// Input:       
//  IN  SETUP_DATA  *FbSetupData - ptr of SetupData
//
// Output:      TRUE if fast boot is enabled, FALSE otherwise
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
#if iME_SUPPORT
#include "ReferenceCode\ME\Protocol\AlertStandardFormat\AlertStandardFormat.h"
#include "Board\EM\MeWrapper\AmtWrapper\Protocol\AmtWrapper\AmtWrapper.h"
BOOLEAN IsAmtBoot(IN SETUP_DATA *FbSetupData)
{
    EFI_GUID gEfiAmtWrapperProtocolGuidTse = EFI_AMT_WRAPPER_PROTOCOL_GUID;
    EFI_GUID gEfiAlertStandardFormatProtocolGuid = EFI_ALERT_STANDARD_FORMAT_PROTOCOL_GUID;
    AMT_WRAPPER_PROTOCOL *pAmtWrapper = NULL;
    EFI_STATUS            Status;
    EFI_ALERT_STANDARD_FORMAT_PROTOCOL  *AsfCheck;
    EFI_ASF_BOOT_OPTIONS  *mInternalAsfBootOptions;

    if (pAmtWrapper == NULL) {
        Status = pBS->LocateProtocol(&gEfiAmtWrapperProtocolGuidTse, NULL, &pAmtWrapper);
    }

    //case IDER
    if (pAmtWrapper != NULL) {
        if (pAmtWrapper->ActiveManagementEnableIdeR()||pAmtWrapper->ActiveManagementEnableSol()){
            ChangeSetupBootFlow(BOOT_FLOW_CONDITION_NORMAL);
            return FALSE;   //Is AMT boot, return FALSE for fast boot disabled.
        }
    }

    //case ASF
    //Get the ASF options
    //if set then we have to do and Asfboot
    Status = pBS->LocateProtocol (
                    &gEfiAlertStandardFormatProtocolGuid,
                    NULL,
                    &AsfCheck
                    );
	
    if (EFI_ERROR (Status)) {
        //Is not AMT boot, return TRUE for fast boot enabled.
        return TRUE;
    }

    Status = AsfCheck->GetBootOptions (AsfCheck, &mInternalAsfBootOptions);
	  
    if (mInternalAsfBootOptions->SubCommand != ASF_BOOT_OPTIONS_PRESENT)
        return TRUE;   //Is not AMT boot, return TRUE for fast boot enabled.
    else {
        ChangeSetupBootFlow(BOOT_FLOW_CONDITION_NORMAL);
        return FALSE;    //Is AMT boot, return FALSE for fast boot disabled.
    }

}
#else // for order ME version

#define AMI_AMT_BOOT_OPTION_GUID \
{0x9ba25957, 0x21bf, 0x41d0, 0x81, 0xe7, 0xe7, 0xb6, 0xd8, 0x88, 0x2a, 0x49}

BOOLEAN IsAmtBoot(IN SETUP_DATA *FbSetupData)
{
    EFI_GUID gAmtBootOptionGuid = AMI_AMT_BOOT_OPTION_GUID;
    EFI_STATUS Status;
    UINT16  AmtBootOption;
    UINTN   VariableSize = 0;

    // Get Device Type Variable of AMT Boot Option.
    VariableSize = sizeof(UINT16);

    Status = pRS->GetVariable ( L"AmtBootOption",
                                  &gAmtBootOptionGuid,
                                  NULL,
                                  &VariableSize,
                                  &AmtBootOption  );

    if (EFI_ERROR(Status))  
//Is not AMT boot, return TRUE for fastboot enabled.
	    return TRUE;
    else {
//Is AMT boot, return FALSE for fastboot disabled.
        ChangeSetupBootFlow(BOOT_FLOW_CONDITION_NORMAL);
        return FALSE;
    }   
}								
#endif	

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   TseHandShake
//
// Description: This function executes FastBoot path via AMI TSE boot manager
//
// Input:       FAST_BOOT LastBoot - last successful fast boot information
//
// Output:      EFI_ERROR - if fast boot via AMI TSE cannot be executed
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS 
TseHandShake()
{
    EFI_STATUS Status;
	AMI_POST_MANAGER_PROTOCOL *AmiPostMgr;
    EFI_HANDLE Handle = NULL;

    Status = pBS->LocateProtocol(&AmiPostMgrProtocolGuid, NULL, &AmiPostMgr);
    if(EFI_ERROR(Status)) return Status;

	if( gFastBootPolicy->UEfiBoot == TRUE)
        FastBootProtocol.Launch =  FastEfiBoot;
    else
		FastBootProtocol.Launch =  FastLegacyBoot;

	Status = pBS->InstallMultipleProtocolInterfaces(
		                        &Handle,
		                        &gAmiFastBootProtocolGuid,
                                &FastBootProtocol,
		                        NULL);
    if(EFI_ERROR(Status)) return Status;

    Status = ChangeSetupBootFlow(BOOT_FLOW_CONDITION_FAST_BOOT);
    if(EFI_ERROR(Status)) return Status;
    
    TRACE((-1,"FB: TseHandShake\n"));
        
	Status = AmiPostMgr->Handshake();

//if we're here fast boot failed, change Boot flow back to BOOT_FLOW_CONDITION_NORMAL

    ChangeSetupBootFlow(BOOT_FLOW_CONDITION_NORMAL);

    return EFI_ABORTED;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsRuntime
//
// Description: return the status system in runtime or not.
//
// Input:       NONE
//
// Output:      BOOLEAN - TRUE if system is in runtime, FALSE otherwise
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN 
IsRuntime(VOID)
{
    return Runtime;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FbConnectInputDevices
//
// Description: Connect console in device by ConIn variable.
//
// Input:      NONE 
//
// Output:     NONE 
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
FbConnectInputDevices(VOID)
{
    ConnectConInVariable();
}

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
