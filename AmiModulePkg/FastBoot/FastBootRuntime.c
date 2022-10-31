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
// $Header: /Alaska/SOURCE/Modules/PTT/FastBootRuntime.c 3     6/01/12 6:55a Bibbyyeh $
//
// $Revision: 3 $
//
// $Date: 6/01/12 6:55a $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  FastBootRuntime.c
//
//  Description:
//  Implementation of callback when variable services is enabled
//
//<AMI_FHDR_END>
//*************************************************************************
//============================================================================
// Includes
//============================================================================

#include <AmiDxeLib.h>
#include <Setup.h>
#include <Protocol/Variable.h>
#include "FastBoot.h"
#include <Protocol/FastBootProtocol.h>
#include <Protocol/AmiUsbController.h>
#include <token.h>
#include <AMIVfr.h>
//============================================================================
// Define
//============================================================================

//============================================================================
// External Golbal Variable Declaration
//============================================================================

//============================================================================
// External Function Definitions
//============================================================================

//============================================================================
// Golbal Variable Declaration
//============================================================================
static EFI_GUID EfiVariableGuid = EFI_GLOBAL_VARIABLE;
static EFI_GUID FastBootVariableGuid = FAST_BOOT_VARIABLE_GUID;
static EFI_GUID FastBootPolicyGuid = FAST_BOOT_POLICY_PROTOCOL_GUID;
FAST_BOOT_POLICY    gFastBootPolicy;
USB_SKIP_LIST DefaultSkipTable[] = USB_SKIP_TABLE;
SKIP_PCI_LIST DeafultSkipPciList[]=FAST_BOOT_PCI_SKIP_LIST;

//============================================================================
// Function Definitions
//============================================================================
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetDefaultFastBootPolicy
//
// Description: Fill default Fast Boot Plicy 
//
// Input:		
//  IN SETUP_DATA *SetupData - pointer to SetupData
//  IN FAST_BOOT *FbVariable - pointer to FastBoot variable
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SetDefaultFastBootPolicy(SETUP_DATA *SetupData,FAST_BOOT *FbVariable)
{
    EFI_STATUS  Status;
    UINT8 *BootOption = NULL;
    UINTN Size = 0;
    AMITSESETUP AmiTseData;    
    EFI_GUID AmiTseSetupGuid = AMITSESETUP_GUID;
    UINTN    VariableSize = 0;  

    gFastBootPolicy.TestMode = SetupData->FbTestMode;   //(EIP96276.5)    

    //Informatoin for pervious boot
    gFastBootPolicy.UEfiBoot = FbVariable->BootType;
    gFastBootPolicy.BootOptionNumber = FbVariable->BootOptionNumber;
    gFastBootPolicy.CheckBootOptionNumber = TRUE;
    gFastBootPolicy.DevStrCheckSum = FbVariable->DevStrCheckSum;    
    gFastBootPolicy.CheckDevStrCheckSum = TRUE;
    gFastBootPolicy.BootCount = FbVariable->BootCount;

    Status = GetEfiVariable(L"FastBootOption", &FastBootVariableGuid, NULL, &Size, &BootOption);         
    gFastBootPolicy.FastBootOption = (EFI_DEVICE_PATH_PROTOCOL*)BootOption;
   
    //Config Behavior in fastboot path
    gFastBootPolicy.VGASupport = SetupData->FbVga;
    
    gFastBootPolicy.UsbSupport = SetupData->FbUsb;    
#if (((USB_DRIVER_MAJOR_VER*100 ) + (USB_DRIVER_MINOR_VER*10) + (USB_DRIVER_BUILD_VER)) >= 920)    
    gFastBootPolicy.UsbSkipTable = DefaultSkipTable;    
    gFastBootPolicy.UsbSkipTableSize = sizeof(DefaultSkipTable)/sizeof(USB_SKIP_LIST);
#endif

    gFastBootPolicy.Ps2Support = SetupData->FbPs2;
    gFastBootPolicy.NetWorkStackSupport = SetupData->FbNetWrokStack;

    gFastBootPolicy.SkipPciList = DeafultSkipPciList;
    gFastBootPolicy.SkipPciListSize = sizeof(DeafultSkipPciList);
    gFastBootPolicy.SkipTSEHandshake = SKIP_TSE_HANDSHAKE;
    gFastBootPolicy.FirstFastBootInS4 = ALLOW_FIRST_FASTBOOT_IN_S4;
    gFastBootPolicy.ConnectAllSata = CONNECT_ALL_SATA_DEVICE_IN_FASTBOOT;   //(EIP96276.3)+
    //check password
    VariableSize = sizeof(AMITSESETUP);
    Status = pRS->GetVariable ( L"AMITSESetup", \
    	                        &AmiTseSetupGuid, \
    	                        NULL, \
    	                        &VariableSize, \
    	                        &AmiTseData );    

    if (!EFI_ERROR(Status)) {   
        if (AmiTseData.UserPassword[0] != 0) {
            TRACE((-1,"FB: User PW is set\n"));
            //user password is set
            gFastBootPolicy.CheckPassword = TRUE;
        }
        
        if (AmiTseData.AdminPassword[0] != 0) {
            TRACE((-1,"FB: Admin PW is set\n"));                        
            //Admin password is set, don't do anything now.
        }        	
    }

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FastBootGetVarCallback
//
// Description: FastBoot runtime callback
//
// Input:		
//  IN EFI_EVENT Event - Callback event
//  IN VOID *Context - pointer to calling context
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID FastBootGetVarCallback(
    IN EFI_EVENT Event, 
    IN VOID *Context
)
{
    EFI_STATUS Status;
    UINT32 Variable = 0;
    FAST_BOOT FbVariable;
    UINTN Size = sizeof(SETUP_DATA);
    static EFI_GUID SetupVariableGuid = SETUP_GUID;
    SETUP_DATA SetupData;
    EFI_HANDLE  Handle = NULL;

    static UINT32 BootFlow = BOOT_FLOW_CONDITION_FIRST_BOOT;
    static EFI_GUID guidBootFlow = BOOT_FLOW_VARIABLE_GUID;

    //Initial fast boot policy 
    
    pBS->SetMem(&gFastBootPolicy,sizeof(FAST_BOOT_POLICY),0);
    
    Status=pBS->InstallProtocolInterface(
                                &Handle, 
                                &FastBootPolicyGuid, 
                                EFI_NATIVE_INTERFACE,
                                &gFastBootPolicy
                                );   
    ASSERT(!EFI_ERROR(Status));

	Status = pRS->GetVariable(L"Setup", &SetupVariableGuid, NULL, &Size, &SetupData);
	if (EFI_ERROR(Status) || SetupData.FastBoot == 0) 
        gFastBootPolicy.FastBootEnable = FALSE;
    else if (SetupData.FastBoot == 1)
        gFastBootPolicy.FastBootEnable = TRUE;

    Size = sizeof(FbVariable);
    Status = pRS->GetVariable(L"LastBoot", &FastBootVariableGuid, NULL, &Size, (VOID *)&FbVariable);
    if(EFI_ERROR(Status))
        gFastBootPolicy.LastBootVarPresence = FALSE;
    else 
        gFastBootPolicy.LastBootVarPresence = TRUE;
    
    SetDefaultFastBootPolicy(&SetupData,&FbVariable);

    if (gFastBootPolicy.FastBootEnable == FALSE ||gFastBootPolicy.LastBootVarPresence == FALSE)
        return;    


    Size = sizeof(UINT32);
    Status = pRS->GetVariable(L"LastBootFailed", &FastBootVariableGuid, NULL, &Size, &Variable);
    if(EFI_ERROR(Status)) {
        Variable = 0x55aa55aa;
        Status = pRS->SetVariable(L"LastBootFailed", 
                                  &FastBootVariableGuid, 
                                  EFI_VARIABLE_NON_VOLATILE |
                                  EFI_VARIABLE_BOOTSERVICE_ACCESS |
                                  EFI_VARIABLE_RUNTIME_ACCESS,
                                  Size,
                                  &Variable);
    } else {
#if LAST_BOOT_FAIL_MECHANISM    
        //reset LastBootFailed variable
        Status = pRS->SetVariable(L"LastBootFailed", 
                                  &FastBootVariableGuid, 
                                  EFI_VARIABLE_NON_VOLATILE,
                                  0,
                                  &Variable);
        //force setup
        Status = pRS->SetVariable(L"BootFlow", 
                                  &guidBootFlow, 
                                  EFI_VARIABLE_BOOTSERVICE_ACCESS,
                                  sizeof(BootFlow),
                                  &BootFlow);

        gFastBootPolicy.LastBootFailure = TRUE;
#else
        //check the fail count, if reach max count then perform a full boot
        if (Variable == 0x55aa55aa) 
            Variable = 0x01;
        else
            Variable++;

        if (Variable == MAX_LAST_BOOT_FAIL_COUNT) {

            pRS->SetVariable(L"LastBoot", 
                             &FastBootVariableGuid,
                             EFI_VARIABLE_NON_VOLATILE,
                             0,
                             &FbVariable);

            gFastBootPolicy.LastBootVarPresence = FALSE;

            Variable = 0x55aa55aa;
        }

        Status = pRS->SetVariable(L"LastBootFailed", 
                                  &FastBootVariableGuid, 
                                  EFI_VARIABLE_NON_VOLATILE |
                                  EFI_VARIABLE_BOOTSERVICE_ACCESS |
                                  EFI_VARIABLE_RUNTIME_ACCESS,
                                  Size,
                                  &Variable);
#endif
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FastBootEntry
//
// Description: FastBoot runtime callback entry point
//
// Input:		
//  IN EFI_HANDLE ImageHandle - Image handle
//  IN EFI_SYSTEM_TABLE *SystemTable - pointer to system table
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS 
FastBootEntry(
  IN EFI_HANDLE ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_EVENT Event;
    VOID *Registration;
    static EFI_GUID VariableArchProtocolGuid = EFI_VARIABLE_ARCH_PROTOCOL_GUID;
    VOID    *Protocol;
    EFI_STATUS  Status;
    InitAmiLib(ImageHandle,SystemTable);

    Status = pBS->LocateProtocol(&VariableArchProtocolGuid,NULL,&Protocol);
    
    if (EFI_ERROR(Status)) {              
        RegisterProtocolCallback(&VariableArchProtocolGuid, FastBootGetVarCallback, NULL, &Event, &Registration);        
    } else {
        FastBootGetVarCallback(NULL,NULL);
    }   
    return EFI_SUCCESS;   
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
