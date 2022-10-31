//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
// $Header: /Alaska/SOURCE/Modules/HddSecurity/HddPassword/HddPassword.c 31    6/07/12 12:34a Jittenkumarp $
//
// $Revision: 31 $
//
// $Date: 6/07/12 12:34a $
//
//*****************************************************************************
//*****************************************************************************//

//<AMI_FHDR_START>
//---------------------------------------------------------------------------
//
// Name: Hddpassword.c
//
// Description: Provides the Hddpassword Screen support in the setup.
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

#include "AmiDxeLib.h"
#include "token.h"
#include "Include\UefiHii.h"
#include "Protocol\HiiDatabase.h"
#include "Protocol\HiiString.h"
#if defined(SECURITY_SETUP_ON_SAME_PAGE) && SECURITY_SETUP_ON_SAME_PAGE
#include "minisetup.h"
#endif
#include "Protocol\PciIo.h"
#include "Protocol\BlockIo.h"
#include "Protocol\PDiskInfo.h"
#include "Protocol\PIDEController.h"
#include "Protocol\PIDEBus.h"
#include "Protocol\PAhciBus.h"
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/AMIPostMgr.h>
#include "AmiVfr.h"
#include "AmiTseHddSecurity.h"
#include "Protocol\DevicePath.h"
#if SETUP_SAME_SYS_HDD_PW
#include "Core\EM\AMITSE\Inc\PwdLib.h"
#include "Core\EM\AMITSE\Inc\Variable.h"
#endif

#if defined(AmiChipsetPkg_SUPPORT) && AmiChipsetPkg_SUPPORT
#include "SbSataProtocol.h"
SB_HDD_POLICY_PROTOCOL      *gSbHddPolicyPtr=NULL;
EFI_GUID   gSbHddPolicyProtocolGuid    = SB_HDD_POLICY_PROTOCOL_GUID;
#endif
extern EFI_GUID   gHddSecurityEndProtocolGuid ;
extern EFI_GUID   gIdeSecurityInterfaceGuid ;
EFI_GUID   gHddUnlockedGuid             = HDD_UNLOCKED_GUID;
EFI_GUID   gIDESecGuid                 = IDE_SECURITY_CONFIG_GUID;

static EFI_HII_STRING_PROTOCOL *HiiString = NULL;
static CHAR8          *SupportedLanguages=NULL;


typedef struct
{
    IDE_SECURITY_PROTOCOL *IDEPasswordSecurity;
    UINT16                NameToken;
    UINT16                PromptToken;
    BOOLEAN               Locked;
    BOOLEAN               LoggedInAsMaster;
    BOOLEAN               Validated;
    UINT8                 PWD[IDE_PASSWORD_LENGTH + 1];
} IDE_SECURITY_DATA;

#if TSE_BUILD > 0x1206
BOOLEAN IsPasswordSupportNonCaseSensitive();
VOID UpdatePasswordToNonCaseSensitive(CHAR16 *Password, UINTN PwdLength);
#endif

VOID HddNotificationFunction(EFI_EVENT Event, VOID *HddRegContext);
VOID IDEPasswordCheck(VOID);
UINT16 IDEPasswordGetName(UINT16 Index);
UINT16 IDESecurityProtocolInit();
BOOLEAN HddPasswordGetDeviceName(EFI_HANDLE Controller,CHAR16 **wsName);
BOOLEAN CheckSecurityStatus (
    IDE_SECURITY_PROTOCOL *IDEPasswordSecurity,
    BOOLEAN               *Locked,
    UINT16                Mask );
EFI_STATUS IDEPasswordAuthenticateHdd(
    CHAR16  *Password,
    VOID    * Ptr,
    BOOLEAN bCheckUser );

#if defined(SECURITY_SETUP_ON_SAME_PAGE) && SECURITY_SETUP_ON_SAME_PAGE
VOID IDEUpdateConfig(
    VOID  *TempideSecConfig,
    UINTN value );

VOID SearchTseHardDiskField ( 
	BOOLEAN *pbCheckUser, BOOLEAN *pEnabledBit, 
	UINT8 *pHardDiskNumber, VOID *data ); 	

EFI_STRING_ID  ConfigPromptToken = 0;
extern UINTN gInvalidPasswordFailMsgBox;
#else
UINTN               gCurrIDESecPage;
#endif

IDE_SECURITY_DATA   *IDEPasswordSecurityData = NULL;
EFI_HANDLE          gHddSecEndHandle = NULL;
EFI_HANDLE          HddNotifyHandle;
static  EFI_HANDLE  *gHandleBuffer = NULL;
EFI_EVENT           HddNotifyEvent;
VOID                *HddNotifyRegistration;
BOOLEAN             HddFreeze      = FALSE;
UINT16              gIDESecurityCount = 0;
BOOLEAN             gFlag = FALSE;


VOID EfiStrCpy (IN CHAR16 *Destination,IN CHAR16 *Source);
UINTN   EfiStrLen (IN CHAR16 *String);
extern  VOID TSEIDEPasswordCheck();

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   RegisterHddNotification
//
// Description: Register the Protocol call back event
//
//
// Input:       VOID
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN RegisterHddNotification( VOID )
{
    EFI_STATUS Status ;

    Status = gBS->CreateEvent(  EFI_EVENT_NOTIFY_SIGNAL,
                                TPL_CALLBACK,
                                HddNotificationFunction,
                                &HddNotifyRegistration,
                                &HddNotifyEvent);

	ASSERT_EFI_ERROR(Status);									
    Status = gBS->RegisterProtocolNotify(   &gHddSecurityEndProtocolGuid,
                                            HddNotifyEvent,
                                            &HddNotifyRegistration);
	ASSERT_EFI_ERROR(Status);										

    //
    // get any of these events that have occured in the past
    //
    gBS->SignalEvent( HddNotifyEvent );

    return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   HddNotificationFunction
//
// Description: Hdd notification function gets called when HddSecurityEnd Protocol get installed.
//
// Input:
//                  EFI_EVENT Event - Event to signal
//                  void HddRegContext - Event specific context (pointer to NotifyRegisteration
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID HddNotificationFunction( EFI_EVENT Event, VOID *HddRegContext )
{
    EFI_STATUS          Status;
    EFI_HANDLE          *HandleBuffer = NULL;
    IDE_SECURITY_CONFIG     *IdeSecConfig = NULL;
    IDE_SECURITY_CONFIG     ideSecConfig;
    IDE_SECURITY_DATA       *DataPtr=NULL;
    UINTN                   NumHandles;
    UINTN                   IdeSecConfigSize = 0;
    UINTN                   Index=0;

    if ( HddRegContext == NULL )
        return;
    //
    // Initialise IdeSecConfig information if this variable is not set already.
    // 
    IdeSecConfig = VarGetNvramName( L"IDESecDev", &gIDESecGuid, NULL, &IdeSecConfigSize );

    if ( !IdeSecConfig ) {

        IdeSecConfig = EfiLibAllocateZeroPool( sizeof(IDE_SECURITY_CONFIG));

        if ( IdeSecConfig == NULL ) {
            return;
        }
        MemSet( IdeSecConfig, sizeof(IDE_SECURITY_CONFIG), 0);
        VarSetNvramName( L"IDESecDev",
                         &gIDESecGuid,
                         EFI_VARIABLE_BOOTSERVICE_ACCESS,
                         IdeSecConfig,
                         sizeof(IDE_SECURITY_CONFIG));
    } else {
        MemFreePointer( (VOID **)&IdeSecConfig );
    }

    //
    //Locate the handle
    //
    Status = gBS->LocateHandleBuffer(   ByRegisterNotify,
                                        NULL,
                                        *(VOID**)HddRegContext,
                                        &NumHandles,
                                        &HandleBuffer);

    //
    // If protocol not installed return
    //
    if ( EFI_ERROR( Status ))
        return;

    gHddSecEndHandle = HandleBuffer[0];

    //
    //Locate the Security Protocols
    //
    gIDESecurityCount = IDESecurityProtocolInit();
    
    for(Index=0; Index<gIDESecurityCount; Index++){
        //
        //Initialize the DataPtr
        //
        DataPtr = (IDE_SECURITY_DATA *) IDEPasswordGetDataPtr(Index);

        //
        // Search for locked Hard disc and not password verification done 
        //
        if(DataPtr->Locked && !DataPtr->Validated){
            break;            
        }
    }

    //
    // Validate the password only if HDD is locked
    //
    if( (gIDESecurityCount != 0 ) && (NULL != DataPtr ) && 
            (DataPtr->Locked ) && (!DataPtr->Validated ) ){
        TSEIDEPasswordCheck();
    } else {
          //
          // Update the IdeSecConfig information .
          // 
          MemSet( &ideSecConfig, sizeof(ideSecConfig), 0 );
          ideSecConfig.Count = gIDESecurityCount;
          VarSetNvramName( L"IDESecDev",
                     &gIDESecGuid,
                     EFI_VARIABLE_BOOTSERVICE_ACCESS,
                     &ideSecConfig,
                     sizeof(ideSecConfig));
    }

    //
    // Install the Unlocked Protocol to nitify HDD has been unlocked
    //
    if ( gHddSecEndHandle != NULL ) {
        Status = gBS->InstallProtocolInterface( &gHddSecEndHandle,
                                                &gHddUnlockedGuid,
                                                EFI_NATIVE_INTERFACE,
                                                NULL);
		ASSERT_EFI_ERROR (Status);
    }

    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PrivateHiiAddString
//
// Description: Add the String to Hii Database using HiiString Protocol
//
// Input:       
//              IN EFI_HII_HANDLE HiiHandle,
//              IN CHAR16 *     String
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STRING_ID PrivateHiiAddString(
    IN EFI_HII_HANDLE HiiHandle,
    IN CHAR16 *     String)
{
    EFI_STATUS Status;
    CHAR8* Languages = NULL;
    UINTN LangSize = 0;
    CHAR8* CurrentLanguage;
    BOOLEAN LastLanguage = FALSE;
    EFI_STRING_ID  StringId = 0;

    if(HiiString == NULL) {
        Status = pBS->LocateProtocol(&gEfiHiiStringProtocolGuid, NULL, (VOID **) &HiiString);
        if(EFI_ERROR(Status)) {
            return 0;
        }
    }

    if(SupportedLanguages == NULL) {
        Status = HiiString->GetLanguages(HiiString, HiiHandle, Languages, &LangSize);
        if(Status == EFI_BUFFER_TOO_SMALL) {
            Status = pBS->AllocatePool(EfiBootServicesData, LangSize, &Languages);
            if(EFI_ERROR(Status)) {
                //
                //not enough resources to allocate string
                //
                return 0;
            }
            Status = HiiString->GetLanguages(HiiString, HiiHandle, Languages, &LangSize);
        }
        SupportedLanguages=Languages;
    } else {
        Languages=SupportedLanguages;
    }

    while(!LastLanguage) {
        //
        //point CurrentLanguage to start of new language
        //
        CurrentLanguage = Languages;        
        while(*Languages != ';' && *Languages != 0)
            Languages++;

        //
        //Last language in language list
        //
        if(*Languages == 0) {       
            LastLanguage = TRUE;
            if(StringId == 0) {
                Status = HiiString->NewString(HiiString, HiiHandle, &StringId, CurrentLanguage, NULL, String, NULL);
            } else {
                Status = HiiString->SetString(HiiString, HiiHandle, StringId, CurrentLanguage, String, NULL);
            }
            if(EFI_ERROR(Status)) {
                return 0;
            }
        } else {
            //
            //put null-terminator
            //
            *Languages = 0;         
            if(StringId == 0) {
                Status = HiiString->NewString(HiiString, HiiHandle, &StringId, CurrentLanguage, NULL, String, NULL);
            } else {
                Status = HiiString->SetString(HiiString, HiiHandle, StringId, CurrentLanguage, String, NULL);
            }
            *Languages = ';';       //restore original character
            Languages++;
            if(EFI_ERROR(Status)) {
                return 0;
            }
        }
    }
    return StringId;        
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PasswordHiiAddString
//
// Description: Add the String to HiiDatabase 
//
// Input:       
//              IN EFI_HII_HANDLE HiiHandle,
//              IN CHAR16 *     String
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STRING_ID PasswordHiiAddString(
    IN EFI_HII_HANDLE HiiHandle,
    IN CHAR16 * String )
{
    EFI_STRING_ID  StringId = 0;

    StringId=PrivateHiiAddString( HiiHandle, String );

    //
    // Error in Adding String. Try with Default function that AMITSE has.
    //
    if(StringId == 0) {
        StringId= HiiAddString( HiiHandle, String );
    }

    return StringId;
}    

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IDESecurityProtocolInit
//
// Description: Locate the Security Protocols and return the information
//
// Input:       none
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16 IDESecurityProtocolInit( )
{
    EFI_STATUS                      Status;
    EFI_HANDLE                      *HandleBuffer = NULL;
    UINT16                          i, j, HDDCount = 0;
    UINTN                           Count;
    CHAR16                          * Name, *Temp1;
    CHAR16                          Temp[60];
    IDE_SECURITY_PROTOCOL           *IDEPasswordSecurity = NULL;
    IDE_SECURITY_DATA               *DataPtr             = NULL;
    EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
    EFI_DEVICE_PATH_PROTOCOL        *DevicePathNode;
    PCI_DEVICE_PATH                 *PciDevicePath;
    SECURITY_PROTOCOL               *Security = NULL;
    UINT32                          HddPortNumber;
    CHAR16                          *Buff=L"P";
    EFI_DISK_INFO_PROTOCOL          *DiskInfoPtr=NULL;
    UINT32                          PortNumber;
    UINT32                          PMPortNumber;
    IDE_SECURITY_DATA               *TempIDEPasswordSecurityData = NULL;
#if defined(AmiChipsetPkg_SUPPORT) && AmiChipsetPkg_SUPPORT
    UINT32                          IdeChannel;
    UINT32                          IdeDevice;
#endif
			
#if defined(SECURITY_SETUP_ON_SAME_PAGE) && SECURITY_SETUP_ON_SAME_PAGE
    if(ConfigPromptToken == 0)
    ConfigPromptToken = PasswordHiiAddString( gHiiHandle, L"HDD Security Configuration" );
#endif
    Status = gBS->LocateHandleBuffer(   ByProtocol,
                                        &gIdeSecurityInterfaceGuid,
                                        NULL,
                                        &Count,
                                        &HandleBuffer);

    //
    // If already data has been found return with that information
    //
    if ( gIDESecurityCount != 0 && IDEPasswordSecurityData != NULL ) {
        if ( gIDESecurityCount == Count ) {
            return gIDESecurityCount;     //the IDE struct is valid
        }

        //
        // New HDD device found. Need to validate the password for the new HDD 
        // and skip the HDD that has been already validated.
        //
        TempIDEPasswordSecurityData = IDEPasswordSecurityData;
        IDEPasswordSecurityData = EfiLibAllocateZeroPool( Count * sizeof(IDE_SECURITY_DATA));
        //
        // Copy the Existing HDD data
        //
        MemCopy( IDEPasswordSecurityData, TempIDEPasswordSecurityData, sizeof(IDE_SECURITY_DATA) * gIDESecurityCount );
        MemFreePointer((VOID**)&TempIDEPasswordSecurityData );
        DataPtr = IDEPasswordSecurityData;
        //
        // DataPtr moved to free Entry
        //       
        DataPtr+=gIDESecurityCount;
        HDDCount=gIDESecurityCount;  
  
    } else {
        //
        // Allocate the buffer for DataPtr
        //
        IDEPasswordSecurityData = EfiLibAllocateZeroPool( Count * sizeof(IDE_SECURITY_DATA));
        DataPtr = IDEPasswordSecurityData;
    }

    if ( EFI_ERROR( Status )) {
        return 0;
    }


    if(DataPtr == NULL) {
        return 0;
    }

    for ( i = 0; i < Count; i++ ) {
        //
        // Check if already Validate or not. If already validate don't verify the password again.
        //
        if ( gHandleBuffer != NULL && gIDESecurityCount != 0 ) {
            j = gIDESecurityCount;

            do {
                if ( HandleBuffer[i] == gHandleBuffer[j - 1] ) {
                    break;
                }
                j--;
            } while ( j != 0 );

            if ( j != 0 )
                continue;
        }

        //
        // Get the PasswordSecurity Protocol
        //
        Status = gBS->OpenProtocol( HandleBuffer[i],
                                    &gIdeSecurityInterfaceGuid,
                                    (VOID**) &IDEPasswordSecurity,
                                    NULL,
                                    HandleBuffer[i],
                                    EFI_OPEN_PROTOCOL_GET_PROTOCOL);

        if ( EFI_ERROR( Status ))
            continue;
        //
        // Handle the DiskInfo Protocol
        //   
        Status = gBS->OpenProtocol( HandleBuffer[i],
                                    &gEfiDiskInfoProtocolGuid,
                                    (VOID**) &DiskInfoPtr,
                                    NULL,
                                    HandleBuffer[i],
                                    EFI_OPEN_PROTOCOL_GET_PROTOCOL);

        if ( EFI_ERROR( Status )){
            continue;
        }
        //
        // Locate the device path Protocol
        //
        Status = gBS->OpenProtocol( HandleBuffer[i],
                                    &gEfiDevicePathProtocolGuid,
                                    (VOID**)&DevicePath,
                                    NULL,
                                    HandleBuffer[i],
                                    EFI_OPEN_PROTOCOL_GET_PROTOCOL);

        if ( EFI_ERROR( Status )){
            continue;
        }

        DevicePathNode = DevicePath;

        //
        // Traverse the Device Path structure till we reach HARDWARE_DEVICE_PATH
        //
        while (!isEndNode (DevicePathNode)) {

            if ((DevicePathNode->Type == HARDWARE_DEVICE_PATH) &&
                (DevicePathNode->SubType == HW_PCI_DP)){

                PciDevicePath = (PCI_DEVICE_PATH *) DevicePathNode;
                break;
            }

            DevicePathNode = NEXT_NODE (DevicePathNode);
        }

        if (PciDevicePath == NULL) continue;

        Security=(SECURITY_PROTOCOL *)IDEPasswordSecurity;

        if(Security->ModeFlag){
            //
            //  Controller is in Ahci Mode, Call WhichIde function to find out Port Number
            //
            DiskInfoPtr->WhichIde(DiskInfoPtr,&PortNumber,&PMPortNumber);
            //
            //  Assign the PortNumber to HddPortNumber.This Port Number is displayed in Setup.
            //  
            HddPortNumber=PortNumber;

            gFlag=TRUE;

        }else{


#if defined(AmiChipsetPkg_SUPPORT) && AmiChipsetPkg_SUPPORT

            if(gSbHddPolicyPtr==NULL){
                Status=gBS->LocateProtocol(&gSbHddPolicyProtocolGuid,
                                            NULL, \
                                            &gSbHddPolicyPtr);
            }

            if(gSbHddPolicyPtr!=NULL){

                //
                //  Find out the Primary/Secondary,Master/Slave Info from WhichIde function  
                //
                DiskInfoPtr->WhichIde(DiskInfoPtr,&IdeChannel,&IdeDevice);
                //
                //  Get the Port Number to which the HDD is connected
                //
                gSbHddPolicyPtr->GeneratePortNumber(PciDevicePath->Device,PciDevicePath->Function,
                                                    IdeChannel,IdeDevice,&PortNumber);

                HddPortNumber=PortNumber;
            
                gFlag=TRUE;

            }else{

                //
                // SB HDD Policy Protocol is not Present. 
                //
                gFlag=FALSE;
            }
#endif
        }
        if ( CheckSecurityStatus( IDEPasswordSecurity, &(DataPtr->Locked), SecurityLockedMask )) {
            DataPtr->IDEPasswordSecurity = IDEPasswordSecurity;

            if ( HddPasswordGetDeviceName( HandleBuffer[i], &Name )) {
                DataPtr->NameToken = PasswordHiiAddString( gHiiHandle, Name );
                Name[12] = 0;
                if(gFlag){
                    //
                    //  Display the the Port Number in Setup
                    //             
                    SPrint( Temp, 60, L"%s%d:%s", Buff, HddPortNumber, Name );
                    DataPtr->PromptToken = PasswordHiiAddString( gHiiHandle, Temp );
                }else{
                    //
                    //  If SB HDD Policy Protocol is not Installed Use STR_IDE_SECURITY_PROMPT
                    //  token to display the String Information.
                    //
                    Temp1 = HiiGetString( gHiiHandle, STRING_TOKEN( STR_IDE_SECURITY_PROMPT ));
                    SPrint( Temp, 60, L"%s%d:%s", Temp1,HDDCount,Name);
                    DataPtr->PromptToken = PasswordHiiAddString( gHiiHandle, Temp );
                }
            } else {
                if(gFlag){
                    //
                    //  Display the the Port Number in Setup
                    //      
                    SPrint( Temp, 60, L"%s%d", Buff, HddPortNumber );
                    DataPtr->NameToken   = PasswordHiiAddString( gHiiHandle, Temp );
                    DataPtr->PromptToken = PasswordHiiAddString( gHiiHandle, Temp );
                }else{
                    //
                    //  If SB HDD Policy Protocol is not Installed Use STR_IDE_SECURITY_PROMPT
                    //  token to display the String Information.
                    //
                    Temp1 = HiiGetString( gHiiHandle, STRING_TOKEN( STR_IDE_SECURITY_PROMPT ));
                    SPrint( Temp, 60, L"%s%d", Temp1, HDDCount);
                    DataPtr->NameToken   = PasswordHiiAddString( gHiiHandle, Temp );
                    DataPtr->PromptToken = PasswordHiiAddString( gHiiHandle, Temp );
                }
            }
            DataPtr->Validated = FALSE;
            DataPtr++;
            HDDCount++;
         }// end if

    }// end of for

    if ( gHandleBuffer != NULL ) {
        MemFreePointer( (VOID **)&gHandleBuffer );
    }

    gHandleBuffer = EfiLibAllocateZeroPool( sizeof(EFI_HANDLE) * Count );
    MemCopy( gHandleBuffer, HandleBuffer, sizeof(EFI_HANDLE) * Count );

    MemFreePointer((VOID**)&HandleBuffer );

    //
    //if no hd is supported
    //
    if ( HDDCount == 0 ) {
        MemFreePointer((VOID**)&IDEPasswordSecurityData );
        IDEPasswordSecurityData=NULL;
    }

    return HDDCount;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IDEPasswordGetLocked
//
// Description: Return Hdd Locked Information
//
// Input:       UINTN       Index
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IDEPasswordGetLocked(
    UINTN Index )
{
    IDE_SECURITY_DATA *DataPtr = (IDE_SECURITY_DATA*)IDEPasswordGetDataPtr( Index );

	if(DataPtr == NULL) {
		return 0;	
    }
    return DataPtr->Locked;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CheckSecurityStatus
//
// Description: return the Security Status Information
//
// Input:       none
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN CheckSecurityStatus(
    IDE_SECURITY_PROTOCOL *IDEPasswordSecurity,
    BOOLEAN               *ReqStatus,
    UINT16                Mask )
{
    UINT16     SecurityStatus = 0;
    EFI_STATUS Status;

    //
    //get the security status of the device
    //
    Status = IDEPasswordSecurity->ReturnSecurityStatus( IDEPasswordSecurity, &SecurityStatus );

    if ( EFI_ERROR( Status ))
        return FALSE;

    *ReqStatus = (BOOLEAN)((SecurityStatus & Mask) ? TRUE : FALSE );
    return TRUE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   HddPasswordDPLength
//
// Description: return the Device path Length
//
// Input:       none
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINTN HddPasswordDPLength( EFI_DEVICE_PATH_PROTOCOL *pDp )
{
    UINTN Size = 0;

    for (; !(IsDevicePathEnd( pDp )); pDp = NextDevicePathNode( pDp ))
        Size += DevicePathNodeLength( pDp );

    //
    // add size of END_DEVICE_PATH node
    //
    return Size + END_DEVICE_PATH_LENGTH;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   HddPasswordDPGetLastNode
//
// Description: Returns pointer on very last DP node before END_OF_DEVICE_PATH node
//
// Input:       none
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID* HddPasswordDPGetLastNode( EFI_DEVICE_PATH_PROTOCOL *pDp )
{
    EFI_DEVICE_PATH_PROTOCOL *dp = NULL;

    for (; !IsDevicePathEnd( pDp ); pDp = NextDevicePathNode( pDp ))
        dp = pDp;

    return dp;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   HddPasswordDPCopy
//
// Description: Copy the Device path to another Memory buffer
//
// Input:
//              EFI_DEVICE_PATH_PROTOCOL *pDp
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID* HddPasswordDPCopy( EFI_DEVICE_PATH_PROTOCOL *pDp )
{
    UINTN l  = HddPasswordDPLength( pDp );
    UINT8 *p = EfiLibAllocateZeroPool( l );

    MemCopy( p, pDp, l );
    return p;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   HddPasswordGetDeviceName
//
// Description: Return the Drive String Name
//
// Input:   EFI_HANDLE Controller - the handle of the drive
//          CHAR16 **wsName - returned pointer to the drive string
//
// Output:  BOOLEAN - TRUE - drive string has been found and is in wsName
//                  - FALSE - drive string has not been found
//
//  Notes: it is the caller's responsibility to deallocate the space used for
//      wsName
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN HddPasswordGetDeviceName(EFI_HANDLE Controller, CHAR16 **wsName)
{
    EFI_STATUS Status;
    SECURITY_PROTOCOL *Security = NULL;

    CHAR16 *DeviceName;
    BOOLEAN ReturnStatus = FALSE;

    // Get the SECURITY_PROTOCOL (actually getting the IDE_SECURITY_PROTOCOL, but
    //  the SECURITY_PROTOCOL is an extended version with more information)
    Status = gBS->HandleProtocol(Controller, &gIdeSecurityInterfaceGuid, &Security);
    if ( !EFI_ERROR(Status) ) {
        // Check the SATA controller operating mode, and based on the mode, get the UnicodeString
        //  name of the device
        if ( Security->ModeFlag ) {
            DeviceName = ((SATA_DEVICE_INTERFACE*)Security->BusInterface)->UDeviceName->UnicodeString;
        } else {
            DeviceName = ((IDE_BUS_PROTOCOL*)Security->BusInterface)->IdeDevice.UDeviceName->UnicodeString;
        }

        // Allocate space to copy the unicode device name string
        *wsName = EfiLibAllocateZeroPool( sizeof(CHAR16)*(EfiStrLen(DeviceName)+1));

        if ( *wsName!=NULL ) {
            EfiStrCpy( *wsName, DeviceName);
            ReturnStatus = TRUE;
        }
    }

    // Return true to signify that a device name was discovered
    return ReturnStatus;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   AMI_CheckIDEPassword
//
// Description: Get the password and Validate the HDD password
//
// Input:
//          UINT16 PromptToken,
//          VOID *DataPtr
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID AMI_CheckIDEPassword(UINT16 PromptToken, VOID *DataPtr)
{
    UINTN      CurrXPos, CurrYPos, i;
    CHAR16     *PasswordEntered;
    EFI_STATUS Status     = EFI_ACCESS_DENIED;
	
	#if SETUP_ASK_MASTER_PASSWORD
    UINT32     IdePasswordFlags = 0;
	#endif
	
    UINTN      BoxLength  = IDE_PASSWORD_LENGTH;
    CHAR16     *DescToken = NULL;

    UINT16     SecurityStatus=0;
    IDE_SECURITY_PROTOCOL *IDEPasswordSecurity = NULL;  
    CHAR16     *UnlckHddCBToken = NULL;    
    UINTN      CB_BoxLength=0;     

    CheckForKeyHook((EFI_EVENT)NULL, NULL );
    gST->ConIn->Reset( gST->ConIn, FALSE );

    DescToken = HiiGetString( gHiiHandle, STRING_TOKEN( STR_IDE_ENTER_USER ));

    if ( DescToken ) {
        if ( (TestPrintLength( DescToken ) / NG_SIZE) > BoxLength ) {
            BoxLength = TestPrintLength( DescToken ) / NG_SIZE;
        }
    }
    MemFreePointer((VOID**) &DescToken );

    UnlckHddCBToken = HiiGetString( gHiiHandle, STRING_TOKEN(STR_IDE_UNLCK_COLD));
    if ( UnlckHddCBToken ) {
        if ( (TestPrintLength( UnlckHddCBToken ) / NG_SIZE) > CB_BoxLength ) {
            CB_BoxLength = TestPrintLength( UnlckHddCBToken ) / NG_SIZE;
        }
    }
    MemFreePointer((VOID**) &UnlckHddCBToken );          

    //
    //Draw password window
    //
#if ALL_HDD_SAME_PW
    PromptToken = STRING_TOKEN( STR_IDE_SECURITY_PROMPT );
#endif
    _DrawPasswordWindow( PromptToken, BoxLength, &CurrXPos, &CurrYPos );

    PasswordEntered = EfiLibAllocateZeroPool((IDE_PASSWORD_LENGTH + 1) * sizeof(CHAR16));
    IDEPasswordSecurity=((IDE_SECURITY_DATA* )DataPtr)->IDEPasswordSecurity;    
    //
    //Give four chances to enter user password
    //
    for ( i = 0; i < USER_PASSWORD_RETRY_ATTEMPTS; i++ ) {
        Status = IDEPasswordSecurity->ReturnSecurityStatus(IDEPasswordSecurity, &SecurityStatus );
        if(Status == EFI_SUCCESS && (SecurityStatus>>4)& 0x1){
            _DrawPasswordWindow( PromptToken, CB_BoxLength, &CurrXPos, &CurrYPos );
            _ReportInBox(CB_BoxLength,STRING_TOKEN(STR_IDE_UNLCK_COLD),CurrXPos,CurrYPos,TRUE);
            return ;
        }        
        _ReportInBox( BoxLength, STRING_TOKEN(STR_IDE_ENTER_USER), CurrXPos, CurrYPos - 1, FALSE );

        if ( EFI_SUCCESS !=_GetPassword(
                                PasswordEntered,
                                IDE_PASSWORD_LENGTH,
                                CurrXPos,
                                CurrYPos,
                                NULL )) {
            break;
        } // end if

        //
        // Validate the Password
        //
        Status = IDEPasswordAuthenticate( PasswordEntered, DataPtr, TRUE );

        if ( EFI_SUCCESS == Status ) {
            break;
        } else if ((i + 1) != USER_PASSWORD_RETRY_ATTEMPTS ) {
            _ReportInBox( IDE_PASSWORD_LENGTH, STRING_TOKEN(STR_ERROR_PSWD), CurrXPos, CurrYPos, TRUE );
        }
    }// end of for

    #if SETUP_ASK_MASTER_PASSWORD

    if ( EFI_SUCCESS != Status ) {       
         _ReportInBox( IDE_PASSWORD_LENGTH, STRING_TOKEN(STR_ERROR_PSWD), CurrXPos, CurrYPos, TRUE );
        //
        // Checking if the master password is installed
        //
        Status=((IDE_SECURITY_DATA*)DataPtr)->IDEPasswordSecurity->ReturnIdePasswordFlags(
            ((IDE_SECURITY_DATA*)DataPtr)->IDEPasswordSecurity,
            &IdePasswordFlags );

        if((Status == EFI_SUCCESS)&&((IdePasswordFlags>>16)&1)) {
            if ( i < MAXIMUM_HDD_UNLOCK_ATTEMPTS ) {           
                BoxLength = IDE_PASSWORD_LENGTH;
                DescToken = HiiGetString( gHiiHandle, STRING_TOKEN( STR_IDE_ENTER_MASTER ));

                if ( DescToken ) {
                    if (( TestPrintLength( DescToken ) / NG_SIZE) > BoxLength ) {
                        BoxLength = TestPrintLength( DescToken ) / NG_SIZE;
                    }
                }
                MemFreePointer((VOID**) &DescToken );

                ClearScreen( EFI_BACKGROUND_BLACK | EFI_LIGHTGRAY );

                //
                //Draw password window
                //
                _DrawPasswordWindow( PromptToken, BoxLength, &CurrXPos, &CurrYPos );
            }

            //
            //Give remaining chances to enter Master password
            //
            for (; i < MAXIMUM_HDD_UNLOCK_ATTEMPTS; i++ ) {
                Status = IDEPasswordSecurity->ReturnSecurityStatus(IDEPasswordSecurity, &SecurityStatus );
                if(Status == EFI_SUCCESS && (SecurityStatus>>4)& 0x1){
                    _DrawPasswordWindow( PromptToken, CB_BoxLength, &CurrXPos, &CurrYPos );
                    _ReportInBox(CB_BoxLength,STRING_TOKEN(STR_IDE_UNLCK_COLD),CurrXPos,CurrYPos,TRUE);
                    return ;
                }                
                _ReportInBox( BoxLength, STRING_TOKEN(
                              STR_IDE_ENTER_MASTER ), CurrXPos, CurrYPos - 1,
                          FALSE );

                if ( EFI_SUCCESS !=
                     _GetPassword( PasswordEntered, IDE_PASSWORD_LENGTH, CurrXPos,
                                   CurrYPos, NULL )) {
                    break;
                }

                //
                // Vaidate the Master password
                //
                Status = IDEPasswordAuthenticate( PasswordEntered, DataPtr, FALSE );

                if ( EFI_SUCCESS == Status ) {
                    break;
                } else {
                    if ( (i + 1) != MAXIMUM_HDD_UNLOCK_ATTEMPTS ) {
                        _ReportInBox( IDE_PASSWORD_LENGTH,
                                  STRING_TOKEN(
                                          STR_ERROR_PSWD ), CurrXPos, CurrYPos,
                                      TRUE );
                    }
                }
            }// end of for
        }// end if
    }// end if
    #endif

    MemSet( PasswordEntered, (IDE_PASSWORD_LENGTH + 1) * sizeof(CHAR16), 0);

    MemFreePointer((VOID**)&PasswordEntered );

    if ( EFI_SUCCESS != Status ) {
        //Report Invalid password
        _ReportInBox( IDE_PASSWORD_LENGTH, STRING_TOKEN(
                          STR_IDE_ERROR_PSWD ), CurrXPos, CurrYPos, TRUE );
        // Unlock failed.
        EfiLibReportStatusCode( EFI_ERROR_CODE | EFI_ERROR_MAJOR,
                                DXE_INVALID_IDE_PASSWORD,
                                0,
                                NULL,
                                NULL );
    }

    ClearScreen( EFI_BACKGROUND_BLACK | EFI_LIGHTGRAY );

    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IDEPasswordFreezeDevices
//
// Description: Send Frezze command all the HDD
//
// Input:       none
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID IDEPasswordFreezeDevices( )
{
    EFI_STATUS            Status;
    EFI_HANDLE            *HandleBuffer;
    UINT16                i;
    UINTN                 Count;
    IDE_SECURITY_PROTOCOL *IDEPasswordSecurity = NULL;

    HddFreeze = TRUE;

    if(IDEPasswordSecurityData != NULL) {
        //
        // Clear the Password
        //
        MemSet( IDEPasswordSecurityData, sizeof(IDE_SECURITY_DATA) * gIDESecurityCount, 0);
        //
        // Free up the Memory
        //
        MemFreePointer((VOID**)&IDEPasswordSecurityData );
        IDEPasswordSecurityData=NULL;
    }

    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gIdeSecurityInterfaceGuid,
        NULL,
        &Count,
        &HandleBuffer
        );

    if ( EFI_ERROR( Status )) {
        return;
    }

    for ( i = 0; i < Count; i++ ) {
        //
        // get password security protocol
        //
        Status = gBS->OpenProtocol(
            HandleBuffer[i],
            &gIdeSecurityInterfaceGuid,
            (VOID**) &IDEPasswordSecurity,
            NULL,
            HandleBuffer[i],
            EFI_OPEN_PROTOCOL_GET_PROTOCOL
            );

        if ( EFI_ERROR( Status )) {
            continue;
        }

        //
        //Send Freeze lock command
        //
        IDEPasswordSecurity->SecurityFreezeLock( IDEPasswordSecurity );

        Status = gBS->CloseProtocol(
            HandleBuffer[i],
            &gIdeSecurityInterfaceGuid,
            NULL,
            HandleBuffer[i]
            );
    }// end of for
    MemFreePointer((VOID**)&HandleBuffer );

    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IDEPasswordUpdateHdd
//
// Description: Updates the HDD password for the current HDD alone.
//
// Input:
//              UINT32 Index,
//              CHAR16 *Password,
//              BOOLEAN bCheckUser
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IDEPasswordUpdateHdd(UINT32 Index,CHAR16 *Password,BOOLEAN bCheckUser)
{
    IDE_SECURITY_PROTOCOL *IDEPasswordSecurity = NULL;
    UINT16                Control              = bCheckUser ? 0 : 1;
    UINT8                 RevisionCode         = 0;
    BOOLEAN               Locked, Enabled;
    EFI_STATUS            Status = EFI_UNSUPPORTED;
    UINT8                 Buffer[IDE_PASSWORD_LENGTH + 1];
    UINT8 Selection;
    #if !SETUP_SAME_SYS_HDD_PW
    UINTN                 ii;
    #endif
    IDE_SECURITY_DATA     *DataPtr;

//
// While password is set via hook in tse to perfom some OEM feature
// and SETUP_PASSWORD_NON_CASE_SENSITIVE is set, even then the
// password will be updated as if it is case sensitive but Authenticates 
// as non case sensitive so in order to avoid such situation making
// IDEPasswordUpdateHdd() symmetric with IDEPasswordAuthenticateHdd()
// to update case sensivity {EIP99649}
//     
#if TSE_BUILD > 0x1206
{        
	UINTN             NewPwLen = 0;
    if( IsPasswordSupportNonCaseSensitive() ) {
        NewPwLen = EfiStrLen(Password);
        UpdatePasswordToNonCaseSensitive(Password, NewPwLen);
    }
}
#endif

    DataPtr = (IDE_SECURITY_DATA*)IDEPasswordGetDataPtr( Index );

    if ( DataPtr == NULL ) {
        ShowPostMsgBox(
                NULL,
                HiiGetString(
                            gHiiHandle,
                            STRING_TOKEN(STR_IDE_SECURITY_UNSUPPORTED)
                            ),
                MSGBOX_TYPE_OK,
                &Selection
         ); // ShowPostMsgBox

        return FALSE;
    }
    IDEPasswordSecurity = DataPtr->IDEPasswordSecurity;

    //
    //get the status of the device
    //
    if ( !(
                CheckSecurityStatus(
                                     IDEPasswordSecurity, &Locked,
                                     SecurityLockedMask )
             && CheckSecurityStatus( IDEPasswordSecurity, &Enabled,
                                     SecurityEnabledMask ))) {
        return FALSE;
    }

    if ( !Locked ) {
        if ( Password[0] == 0 ) {
            //
            //empty string is entered -> disable password
            //
            Status = IDEPasswordSecurity->SecurityDisablePassword(
                IDEPasswordSecurity,
                Control,
                IDEPasswordSecurityData[Index].PWD );
        } else {
            //
            //set new password
            //
            MemSet( &Buffer, IDE_PASSWORD_LENGTH + 1, 0 );
            #if !SETUP_SAME_SYS_HDD_PW

            for ( ii = 0; ii < IDE_PASSWORD_LENGTH + 1; ii++ ) {
                Buffer[ii] = (UINT8)Password[ii];

                if ( Password[ii] == L'\0' ) {
                    break;
                }
            }// end of for
            #else
            MemCopy( Buffer, Password, IDE_PASSWORD_LENGTH + 1 );
            #endif

            Status = IDEPasswordSecurity->SecuritySetPassword(
                IDEPasswordSecurity,
                Control,
                Buffer,
                RevisionCode );
        }
    }// end if(!Locked)

    if ( EFI_ERROR( Status )) {
        return FALSE;
    }

    return TRUE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IDEPasswordUpdateAllHdd
//
// Description: Updates the HDD password for all the HDDs present.
//
// Input:
//              UINT32 Index,
//              CHAR16 *Password,
//              BOOLEAN bCheckUser
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IDEPasswordUpdateAllHdd(
    UINT32  Index,
    CHAR16  *Password,
    BOOLEAN bCheckUser )
{
    UINTN i;
    BOOLEAN Status = FALSE;
        for ( i = 0; i < gIDESecurityCount; i++ ) {
            Status = IDEPasswordUpdateHdd( (UINT32)i, Password, bCheckUser);
        }
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IDEPasswordUpdate
//
// Description: Hook function to update the password for the HDDs based 
//              on the token ALL_HDD_SAME_PW.
// Input:
//              UINT32 Index,
//              CHAR16 *Password,
//              BOOLEAN bCheckUser
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IDEPasswordUpdate(
    UINT32  Index,
    CHAR16  *Password,
    BOOLEAN bCheckUser )
{
    #if ALL_HDD_SAME_PW
     return IDEPasswordUpdateAllHdd( Index, Password, bCheckUser);
    #else
     return IDEPasswordUpdateHdd( Index, Password, bCheckUser);
    #endif

}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UnlockHDD
//
// Description: Unlock the HDD
//
// Input:       none
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID UnlockHDD(
    VOID )
{
    #if SETUP_SAME_SYS_HDD_PW
    UINTN             Size=0, i;
    AMITSESETUP       *TSESetup;
    IDE_SECURITY_DATA *DataPtr;
    EFI_STATUS        Status;
    EFI_GUID AmiTseSetupGuid = AMITSESETUP_GUID;

    //
    //Do not proceed if admin pssword is not set
    //
    if ( !(AMI_PASSWORD_USER & PasswordCheckInstalled()) ) {
        return;
    }

    //
    //Get password from NVRAM
    //
    Size = 0;
    TSESetup = VarGetNvramName (L"AMITSESetup", &AmiTseSetupGuid, NULL, &Size);

    if ( Size ) {
        //
        //For all drives
        //
        DataPtr = IDEPasswordSecurityData;

        for ( i = 0; i < gIDESecurityCount; i++ ) {
            if ( DataPtr->Locked ) {
                //
                //ask fot the password if locked
                //
                Status = IDEPasswordAuthenticateHdd( TSESetup->UserPassword,
                                                  DataPtr,
                                                  TRUE );

                if ( EFI_SUCCESS != Status ) {
                    //
                    // Unlock failed.
                    //
                    EfiLibReportStatusCode( EFI_ERROR_CODE | EFI_ERROR_MAJOR,
                                            DXE_INVALID_IDE_PASSWORD,
                                            0,
                                            NULL,
                                            NULL );
                }
            }
            DataPtr++;
        } // end of for
    } // end if

    MemFreePointer((VOID**) &TSESetup );
    return;
    #endif //#if SETUP_SAME_SYS_HDD_PW
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetHDDPassword
//
// Description: Set the HDD password
//
// Input:       none
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SetHDDPassword(VOID)
{
    #if SETUP_SAME_SYS_HDD_PW
    UINTN       Size=0, i;
    AMITSESETUP *TSESetup;
    BOOLEAN     Status;
    EFI_GUID AmiTseSetupGuid = AMITSESETUP_GUID;

    //
    //Get password from NVRAM
    //
    Size = 0;
    TSESetup = VarGetNvramName (L"AMITSESetup", &AmiTseSetupGuid, NULL, &Size);

    if ( Size ) {
        //
        //For all drives
        //
        for ( i = 0; i < gIDESecurityCount; i++ ) {
            Status = IDEPasswordUpdateHdd( (UINT32)i, TSESetup->UserPassword, TRUE);
			ASSERT_EFI_ERROR (Status);
        }
    }

    MemFreePointer((VOID**) &TSESetup );
    #endif //#if SETUP_SAME_SYS_HDD_PW
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IDEPasswordCheck
//
// Description: Validate the HDD password
//
// Input:       none
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID IDEPasswordCheck()
{
    #if !SETUP_SAME_SYS_HDD_PW
    IDE_SECURITY_DATA   *DataPtr;
#if !ALL_HDD_SAME_PW
    UINT16              i;
#endif
#if defined(SECURITY_SETUP_ON_SAME_PAGE) && SECURITY_SETUP_ON_SAME_PAGE
    UINTN				IDE_idex = 0;
#endif
    BOOLEAN             ScreenCorrupted = FALSE;
    #endif
    IDE_SECURITY_CONFIG ideSecConfig;

    //
    // build IDESecurity data
    //

    gIDESecurityCount = IDESecurityProtocolInit( );

    if ( IDEPasswordSecurityData == NULL || gIDESecurityCount == 0
         || HddFreeze ) {
        return;
    }

    #if SETUP_SAME_SYS_HDD_PW
        UnlockHDD();
    #else
    DataPtr   = IDEPasswordSecurityData;
#if !ALL_HDD_SAME_PW
    for ( i = 0; i < gIDESecurityCount; i++ ) {
#endif
        if ( DataPtr->Locked && (!DataPtr->Validated)) {
            //
            //ask fot the password if locked
            //
            DataPtr->Validated = TRUE;
            AMI_CheckIDEPassword( DataPtr->PromptToken, (VOID*)DataPtr );
            ScreenCorrupted = TRUE;
        }
#if !ALL_HDD_SAME_PW
        DataPtr++;
    }// end of for
#endif
    //
    // If the Screen Corrupted , Redraw the Screen
    //
    //    if(ScreenCorrupted) {
    //        DrawScreenAgain(OldScreen);
    //    }
    #endif

    MemSet( &ideSecConfig, sizeof(ideSecConfig), 0 );
    ideSecConfig.Count = gIDESecurityCount;
#if defined(SECURITY_SETUP_ON_SAME_PAGE) && SECURITY_SETUP_ON_SAME_PAGE
    for( IDE_idex = 0 ; IDE_idex < gIDESecurityCount ; IDE_idex++ )
    {
        IDEUpdateConfig( &ideSecConfig, IDE_idex );
    }
#endif
    VarSetNvramName( L"IDESecDev",
                     &gIDESecGuid,
                     EFI_VARIABLE_BOOTSERVICE_ACCESS,
                     &ideSecConfig,
                     sizeof(ideSecConfig));
    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IDEUpdateConfigHdd
//
// Description: Initializes the structure IDE_SECURITY_CONFIG for the current
//              HDD if the data pointer to the structure IDE_SECURITY_DATA is
//              initialized already.
//
// Input:
//              IDE_SECURITY_CONFIG *ideSecConfig
//              UINTN value
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID IDEUpdateConfigHdd(
    VOID  *TempideSecConfig,
    UINTN value )
{
    IDE_SECURITY_DATA     *DataPtr             = NULL;
    IDE_SECURITY_PROTOCOL *IDEPasswordSecurity = NULL;
    BOOLEAN               Status;
    UINT32                IdePasswordFlags = 0;
    EFI_STATUS            ReturnStatus;
    IDE_SECURITY_CONFIG   *ideSecConfig
        = (IDE_SECURITY_CONFIG*)TempideSecConfig;


    //
    //Set current IDE security page
    //
#if defined(SECURITY_SETUP_ON_SAME_PAGE) && SECURITY_SETUP_ON_SAME_PAGE

    DataPtr = (IDE_SECURITY_DATA*)IDEPasswordGetDataPtr( value );

    if ( DataPtr ) {
        IDEPasswordSecurity = DataPtr->IDEPasswordSecurity;

        CheckSecurityStatus(
            IDEPasswordSecurity,
            &Status,
            SecuritySupportedMask );
        ideSecConfig->Supported[value] = Status ? 1 : 0;
        CheckSecurityStatus(
            IDEPasswordSecurity,
            &Status,
            SecurityEnabledMask );
        ideSecConfig->Enabled[value] = Status ? 1 : 0;
        CheckSecurityStatus(
            IDEPasswordSecurity,
            &Status,
            SecurityLockedMask );
        ideSecConfig->Locked[value] = Status ? 1 : 0;
        CheckSecurityStatus(
            IDEPasswordSecurity,
            &Status,
            SecurityFrozenMask );
        ideSecConfig->Frozen[value] = Status ? 1 : 0;
        ReturnStatus         = IDEPasswordSecurity->ReturnIdePasswordFlags(
            IDEPasswordSecurity,
            &IdePasswordFlags );

        if ( EFI_ERROR( ReturnStatus )) {
            return;
        }

        ideSecConfig->UserPasswordStatus[value]
            = (IdePasswordFlags & 0x00020000) ? 1 : 0;
        ideSecConfig->MasterPasswordStatus[value]
            = (IdePasswordFlags & 0x00010000) ? 1 : 0;

        ideSecConfig->ShowMaster[value] = 0x0000;

        if ( ideSecConfig->Locked[value] ) {
            ideSecConfig->ShowMaster[value] = 0x0001;
        } else if ( (DataPtr->LoggedInAsMaster)) {
            ideSecConfig->ShowMaster[value] = 0x0001;
        } else if ( !(ideSecConfig->UserPasswordStatus[value])) {
            ideSecConfig->ShowMaster[value] = 0x0001;
        }
    }// end if
#else
    gCurrIDESecPage = value;

    DataPtr = (IDE_SECURITY_DATA*)IDEPasswordGetDataPtr( value );

    if ( DataPtr ) {
        IDEPasswordSecurity = DataPtr->IDEPasswordSecurity;

        CheckSecurityStatus(
            IDEPasswordSecurity,
            &Status,
            SecuritySupportedMask );
        ideSecConfig->Supported = Status ? 1 : 0;
        CheckSecurityStatus(
            IDEPasswordSecurity,
            &Status,
            SecurityEnabledMask );
        ideSecConfig->Enabled = Status ? 1 : 0;
        CheckSecurityStatus(
            IDEPasswordSecurity,
            &Status,
            SecurityLockedMask );
        ideSecConfig->Locked = Status ? 1 : 0;
        CheckSecurityStatus(
            IDEPasswordSecurity,
            &Status,
            SecurityFrozenMask );
        ideSecConfig->Frozen = Status ? 1 : 0;
        ReturnStatus         = IDEPasswordSecurity->ReturnIdePasswordFlags(
            IDEPasswordSecurity,
            &IdePasswordFlags );

        if ( EFI_ERROR( ReturnStatus )) {
            return;
        }

        ideSecConfig->UserPasswordStatus
            = (IdePasswordFlags & 0x00020000) ? 1 : 0;
        ideSecConfig->MasterPasswordStatus
            = (IdePasswordFlags & 0x00010000) ? 1 : 0;

        ideSecConfig->ShowMaster = 0x0000;

        if ( ideSecConfig->Locked ) {
            ideSecConfig->ShowMaster = 0x0001;
        } else if ( (DataPtr->LoggedInAsMaster)) {
            ideSecConfig->ShowMaster = 0x0001;
        } else if ( !(ideSecConfig->UserPasswordStatus)) {
            ideSecConfig->ShowMaster = 0x0001;
        }
    }// end if
#endif
    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IDEUpdateConfigAllHdd
//
// Description: Initializes the structure IDE_SECURITY_CONFIG for all the
//              HDDs present if the data pointer to the structure
//              IDE_SECURITY_DATA is initialized already.
//
// Input:
//              IDE_SECURITY_CONFIG *ideSecConfig
//              UINTN value
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID IDEUpdateConfigAllHdd(
    VOID  *TempideSecConfig,
    UINTN value )
{
    UINTN i;

    for ( i = 0; i < gIDESecurityCount; i++ ) {
        IDEUpdateConfigHdd( TempideSecConfig, i);
    }
    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IDEUpdateConfig
//
// Description: Hook function to Initialize the structure IDE_SECURITY_CONFIG 
//              for the HDDs based on the token ALL_HDD_SAME_PW.   
//
// Input:
//              IDE_SECURITY_CONFIG *ideSecConfig
//              UINTN value
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID IDEUpdateConfig(
    VOID  *TempideSecConfig,
    UINTN value )
{
    #if ALL_HDD_SAME_PW
     IDEUpdateConfigAllHdd( TempideSecConfig, value);
    #else
     IDEUpdateConfigHdd( TempideSecConfig, value);
    #endif

}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IDEPasswordGetName
//
// Description: Get the Hdd name
//
// Input:       none
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16 IDEPasswordGetName(
    UINT16 Index )
{
#if defined(SECURITY_SETUP_ON_SAME_PAGE) && SECURITY_SETUP_ON_SAME_PAGE
    // workaround for code in special.c which fills "goto string" token with
    // hdd name string token. In our case we dont need that.
    return ConfigPromptToken;
#else
    UINTN size=0;
    IDE_SECURITY_CONFIG *ideSecConfig;
    IDE_SECURITY_DATA *DataPtr;

	ideSecConfig = VarGetNvramName (L"IDESecDev", &gIDESecGuid, NULL, &size);
    IDEUpdateConfigHdd (ideSecConfig, Index);
    VarSetNvramName (L"IDESecDev",
                     &gIDESecGuid,
                     EFI_VARIABLE_BOOTSERVICE_ACCESS,
                     ideSecConfig,
                     size);

    MemFreePointer((VOID **)&ideSecConfig);

    DataPtr = (IDE_SECURITY_DATA*)IDEPasswordGetDataPtr( Index );

    if(DataPtr == NULL) {
        return 0;
    }    

    return DataPtr->PromptToken;
#endif
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IDEPasswordAuthenticateHdd
//
// Description: Validates the Ide password for the current HDD alone.
//
// Input:
//          CHAR16 *Password,
//          VOID* Ptr,
//          BOOLEAN bCheckUser
//
// Output:  none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS IDEPasswordAuthenticateHdd(
    CHAR16  *Password,
    VOID    * Ptr,
    BOOLEAN bCheckUser )
{
    UINT16            Control = 0;
    EFI_STATUS        Status;
    UINT8             Buffer[IDE_PASSWORD_LENGTH + 1];

    #if !SETUP_SAME_SYS_HDD_PW
    UINTN             i;
    #endif

    IDE_SECURITY_DATA * DataPtr = (IDE_SECURITY_DATA*)Ptr;

    MemSet( &Buffer, IDE_PASSWORD_LENGTH + 1, 0 );

#if TSE_BUILD > 0x1206
{        
	UINTN             NewPwLen = 0;
    if( IsPasswordSupportNonCaseSensitive() ) {
        NewPwLen = EfiStrLen(Password);
        UpdatePasswordToNonCaseSensitive(Password, NewPwLen);
    }
}
#endif

    #if !SETUP_SAME_SYS_HDD_PW

    for ( i = 0; i < IDE_PASSWORD_LENGTH + 1; i++ ) {
        Buffer[i] = (UINT8)Password[i];

        if ( Password[i] == L'\0' ) {
            break;
        }
    }
    #else
    MemCopy( Buffer, Password, IDE_PASSWORD_LENGTH + 1 );
    #endif

    Control = bCheckUser ? 0 : 1;

    Status = (DataPtr->IDEPasswordSecurity)->SecurityUnlockPassword(
        DataPtr->IDEPasswordSecurity,
        Control,
        Buffer );

    if ( EFI_ERROR( Status )) {
        return EFI_ACCESS_DENIED;
    }

    //
    //save password in case we need to disable it during the setup
    //
    MemCopy( &(DataPtr->PWD), &Buffer, IDE_PASSWORD_LENGTH + 1 );
//    DataPtr->Locked = FALSE;

    if ( !bCheckUser ) {
        DataPtr->LoggedInAsMaster = TRUE;
    }

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IDEPasswordAuthenticateAllHdd
//
// Description: Validates the Ide password for all the HDDs Present.
//
// Input:
//          CHAR16 *Password,
//          VOID* Ptr,
//          BOOLEAN bCheckUser
//
// Output:  none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS IDEPasswordAuthenticateAllHdd(
    CHAR16  *Password,
    VOID    * Ptr,
    BOOLEAN bCheckUser )
{
    IDE_SECURITY_DATA *DataPtr;
    UINTN i;
    EFI_STATUS        Status=EFI_NOT_FOUND;
    
        //
        //For all drives
        //
        DataPtr = IDEPasswordSecurityData;

        if(DataPtr == NULL) {
            return EFI_NOT_FOUND;
        }

        for ( i = 0; i < gIDESecurityCount; i++ ) {

            Status = IDEPasswordAuthenticateHdd( Password,
                                              DataPtr,
                                              bCheckUser );
            if ( EFI_SUCCESS != Status ) {
                //
                // Unlock failed.
                //
                EfiLibReportStatusCode( EFI_ERROR_CODE | EFI_ERROR_MAJOR,
                                        DXE_INVALID_IDE_PASSWORD,
                                        0,
                                        NULL,
                                        NULL );
            }
            DataPtr++;
        }
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IDEPasswordAuthenticate
//
// Description: Hook function to validate IDE Password for the HDDs based on
//              the token ALL_HDD_SAME_PW
// Input:
//          CHAR16 *Password,
//          VOID* Ptr,
//          BOOLEAN bCheckUser
//
// Output:  none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS IDEPasswordAuthenticate(
    CHAR16  *Password,
    VOID    * Ptr,
    BOOLEAN bCheckUser )
{
    #if ALL_HDD_SAME_PW
     return IDEPasswordAuthenticateAllHdd( Password, Ptr, bCheckUser);
    #else
     return IDEPasswordAuthenticateHdd( Password, Ptr, bCheckUser);
    #endif

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IDEPasswordGetDataPtr
//
// Description: Get the Ide password Data pointer
//
// Input:       none
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID* IDEPasswordGetDataPtr( UINTN Index )
{
    IDE_SECURITY_DATA *DataPtr;

    if ( gIDESecurityCount == 0 || IDEPasswordSecurityData == NULL ) {
        //
        //try to initialize, if not initialized
        //
        gIDESecurityCount = IDESecurityProtocolInit( );
    }

    if ( gIDESecurityCount == 0 || IDEPasswordSecurityData == NULL || Index >=
         gIDESecurityCount ) {
        return NULL;
    }

    DataPtr = (IDE_SECURITY_DATA*)IDEPasswordSecurityData;
    
    if(DataPtr == NULL) {
        return 0;
    }

    return (VOID*)&DataPtr[Index];
}

#if defined(SECURITY_SETUP_ON_SAME_PAGE) && SECURITY_SETUP_ON_SAME_PAGE
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	FramePwdCallbackIdePasswordUpdate
//
// Description:	function to update the ide password
//
// Input:		CONTROL_DATA *control : Selected password control data,
//				VOID *saveData : New password
//
// Output:		EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS FramePwdCallbackIdePasswordUpdate ( CONTROL_DATA *control, CHAR16 *saveData)
{
    BOOLEAN bCheckUser = FALSE;
	VOID * data =control->ControlData.ControlPtr;
	UINT8 HardDiskNumber = 0xFF;

    // Check whether selected password control is a HDD Password control
	if (control->ControlData.ControlVariable == VARIABLE_ID_IDE_SECURITY ) 
	{
        // find index of currently selected HDD and type of password(user/master) to update
		SearchTseHardDiskField( &bCheckUser, NULL, &HardDiskNumber, data );
		
		if( HardDiskNumber != 0xFF ) // If HDD index is valid
		{
			IDEPasswordUpdate( (UINT32)HardDiskNumber, (CHAR16*) saveData, bCheckUser ); //update it
		}
		return EFI_SUCCESS;
	}
	else
    	return EFI_UNSUPPORTED;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	PopupPwdAuthenticateIDEPwd
//
// Description:	Function to authenticate the IDE password
//
// Input:		POPUP_PASSWORD_DATA *popuppassword, 
//				BOOLEAN *AbortUpdate,
//				VOID *data
//
// Output:		EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS PopupPwdAuthenticateIDEPwd(POPUP_PASSWORD_DATA *popuppassword, BOOLEAN *AbortUpdate,VOID *data)
{
   	EFI_STATUS Status = EFI_UNSUPPORTED;
    CHAR16 *Text=NULL;
	UINT8 HardDiskNumber = 0xFF;

    // Check whether selected password control is a HDD Password control
	if(popuppassword->ControlData.ControlVariable == VARIABLE_ID_IDE_SECURITY ) 
	{
        BOOLEAN bCheckUser = FALSE;
        BOOLEAN EnabledBit = FALSE;
        UINTN size = 0;
        IDE_SECURITY_CONFIG *ideSecConfig;

        ideSecConfig = VarGetVariable( VARIABLE_ID_IDE_SECURITY, &size ); // Get the data from setup page
        if (NULL == ideSecConfig) {
            return EFI_NOT_FOUND;
        }
		// find index of currently selected HDD and type of password(user/master) to authenticate
        SearchTseHardDiskField( &bCheckUser, &EnabledBit, &HardDiskNumber, data );
        // Check if password has been set for selected HDD
		if(  ( HardDiskNumber != 0xFF )  && ideSecConfig->Enabled[HardDiskNumber] )
		{
			EnabledBit = TRUE;
		}

        // If password has been set then proceed
        if(EnabledBit)
        {
			if( bCheckUser || ideSecConfig->MasterPasswordStatus[HardDiskNumber] )
            {
                // Ask for the password
    			Status = _DoPopupEdit( popuppassword, STRING_TOKEN(STR_OLD_PSWD), &Text);
                if(EFI_SUCCESS != Status )
                {
                    *AbortUpdate = TRUE; // Status: Password not updated
                }
    			else
    			{
                    // Get IDE_SECURITY_PROTOCOL instance for current HDD
					void* DataPtr = TSEIDEPasswordGetDataPtr( HardDiskNumber );
                    Status = TSEIDEPasswordAuthenticate( Text, DataPtr, bCheckUser ); //Authenticate it
                    if(EFI_ERROR( Status ))
        			{
                        // Show error message if password is wrong
        				CallbackShowMessageBox( (UINTN)gInvalidPasswordFailMsgBox, MSGBOX_TYPE_OK );
        				*AbortUpdate = TRUE; // Status: Password not updated
        			}
					StringZeroFreeMemory ((VOID **)&Text); // Erase string and free allocated memory
    			}
            }
		}

        MemFreePointer((VOID **) &ideSecConfig); // Free setup data memory
		return EFI_SUCCESS;
	}
	return EFI_UNSUPPORTED;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	PopupPwdUpdateIDEPwd
//
// Description:	function to update the setup config page after IDE password update
//
// Input:		None
//
// Output:		None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID PopupPwdUpdateIDEPwd (VOID)
{
    UINTN size = 0;
    IDE_SECURITY_CONFIG *ideSecConfig;
	UINT8 HardDiskNumber = 0xFF;

    // Get the old setup config data
    ideSecConfig = VarGetVariable( VARIABLE_ID_IDE_SECURITY, &size );
    if (NULL == ideSecConfig) {
        return;
    }
    // Update setup data for all HDDs
    for( HardDiskNumber = 0 ; HardDiskNumber < ideSecConfig->Count ; HardDiskNumber++ )
	{
		IDEUpdateConfig( (VOID*)ideSecConfig, HardDiskNumber );
	}
    // Set the new setup config data
    VarSetValue (VARIABLE_ID_IDE_SECURITY, 0, size, ideSecConfig);
	if (gApp != NULL)
		gApp->CompleteRedraw = TRUE; // redraw setup config page to reflect updated configuration
    MemFreePointer((VOID **)&ideSecConfig);		
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	SearchTseHardDiskField
//
// Description:	function to search TSE hard disk field.
//
// Input:      IN OUT BOOLEAN *pbCheckUser : Password type - User/Master, 
//             IN OUT BOOLEAN *pEnabledBit : Password is set / not, 
//             IN OUT UINT8 *pHardDiskNumber : HDD index, 
//             IN VOID *data 
//
// Output:     None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SearchTseHardDiskField ( 
	IN OUT BOOLEAN *pbCheckUser, IN OUT BOOLEAN *pEnabledBit, 
	IN OUT UINT8 *pHardDiskNumber, IN VOID *data 
  )
{
	UINTN	size = 0;
	IDE_SECURITY_CONFIG *ideSecConfig;

	ideSecConfig = VarGetVariable( VARIABLE_ID_IDE_SECURITY, &size );

	//
	// Check if User password field is selected, if yes then set HDD index
	//
	if( UefiGetQuestionOffset(data) /*data->QuestionId*/
		== STRUCT_OFFSET(IDE_SECURITY_CONFIG,IDEUserPassword) )
	{
		*pHardDiskNumber = 0;
		*pbCheckUser = TRUE;
	}
	else if( UefiGetQuestionOffset(data) /*data->QuestionId*/
		== STRUCT_OFFSET(IDE_SECURITY_CONFIG,IDEUserPassword_HDD2) )
	{
		*pHardDiskNumber = 1;
		*pbCheckUser = TRUE;
	}
	else if( UefiGetQuestionOffset(data) /*data->QuestionId*/
		== STRUCT_OFFSET(IDE_SECURITY_CONFIG,IDEUserPassword_HDD3) )
	{
		*pHardDiskNumber = 2;
		*pbCheckUser = TRUE;
	}
	else if( UefiGetQuestionOffset(data) /*data->QuestionId*/
		== STRUCT_OFFSET(IDE_SECURITY_CONFIG,IDEUserPassword_HDD4) )
	{
		*pHardDiskNumber = 3;
		*pbCheckUser = TRUE;
	}
	else if( UefiGetQuestionOffset(data) /*data->QuestionId*/
		== STRUCT_OFFSET(IDE_SECURITY_CONFIG,IDEUserPassword_HDD5) )
	{
		*pHardDiskNumber = 4;
		*pbCheckUser = TRUE;
	}
	else if( UefiGetQuestionOffset(data) /*data->QuestionId*/
		== STRUCT_OFFSET(IDE_SECURITY_CONFIG,IDEUserPassword_HDD6) )
	{
		*pHardDiskNumber = 5;
		*pbCheckUser = TRUE;
	}
	//
	// Check if Master password field is selected, if yes then set HDD index
	//
	else if( UefiGetQuestionOffset(data) /*data->QuestionId*/
		== STRUCT_OFFSET(IDE_SECURITY_CONFIG,IDEMasterPassword) )
	{
		*pHardDiskNumber = 0;
		*pbCheckUser = FALSE;
	}
	else if( UefiGetQuestionOffset(data) /*data->QuestionId*/
		== STRUCT_OFFSET(IDE_SECURITY_CONFIG,IDEMasterPassword_HDD2) )
	{
		*pHardDiskNumber = 1;
		*pbCheckUser = FALSE;
	}
	else if( UefiGetQuestionOffset(data) /*data->QuestionId*/
		== STRUCT_OFFSET(IDE_SECURITY_CONFIG,IDEMasterPassword_HDD3) )
	{
		*pHardDiskNumber = 2;
		*pbCheckUser = FALSE;
	}
	else if( UefiGetQuestionOffset(data) /*data->QuestionId*/
		== STRUCT_OFFSET(IDE_SECURITY_CONFIG,IDEMasterPassword_HDD4) )
	{
		*pHardDiskNumber = 3;
		*pbCheckUser = FALSE;
	}
	else if( UefiGetQuestionOffset(data) /*data->QuestionId*/
		== STRUCT_OFFSET(IDE_SECURITY_CONFIG,IDEMasterPassword_HDD5) )
	{
		*pHardDiskNumber = 4;
		*pbCheckUser = FALSE;
	}
	else if( UefiGetQuestionOffset(data) /*data->QuestionId*/
		== STRUCT_OFFSET(IDE_SECURITY_CONFIG,IDEMasterPassword_HDD6) )
	{
		*pHardDiskNumber = 5;
		*pbCheckUser = FALSE;
	}
	else // Question offset is not from any of the password fields
	{
		*pHardDiskNumber = 0xFF; // No HDD selected
		if( pEnabledBit != NULL )
		{
			*pEnabledBit = FALSE; // No HDD ie no password is set
		}
	}
    // if HDD index is invalid, set it to 0xFF
	if( *pHardDiskNumber >= ideSecConfig->Count )
	{
		*pHardDiskNumber = 0xFF;
	}

	MemFreePointer( (VOID **) &ideSecConfig );
	return;
}

#endif

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
