//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/HddSecurity/IdeSecurity.c 36    1/17/12 12:50a Jittenkumarp $
//
// $Revision: 36 $
//
// $Date: 1/17/12 12:50a $
//**********************************************************************
//<AMI_FHDR_START>
//---------------------------------------------------------------------------
//
// Name: 		HddSecurity.c 
//
// Description:	HDD Security Support
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

#include "HddSecurity.h"



static EFI_GUID                    gHddSecurityInitProtocolGuid
    = HDD_SECURITY_INIT_PROTOCOL_GUID;
static EFI_GUID                    gEfiBootScriptSaveGuid
    = EFI_BOOT_SCRIPT_SAVE_GUID;
static EFI_GUID                    gIdeSecurityPwNvguid
    = IDE_SECURITY_PWNV_GUID;

extern EFI_DRIVER_BINDING_PROTOCOL gIdeBusDriverBinding;
extern EFI_EVENT                   gIDEBusEvtMiscSmmFeatures;
EFI_EVENT                          BootEvent = NULL;
EFI_EVENT                          gIDEBusEvtBootScript = NULL;

const CHAR16                       *sOemMasterPasswordTag
    = CONVERT_TO_WSTRING( OEM_MASTER_PASSWORD_TAG );
const CHAR16                       *sDefaultMasterPasswordTag
    = CONVERT_TO_WSTRING( DEFAULT_MASTER_PASSWORD_TAG );

#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
EFI_SMM_CONTROL2_PROTOCOL   *mSmmCtl=NULL;
#else
EFI_SMM_CONTROL_PROTOCOL    *mSmmCtl=NULL;
#endif

HDD_PASSWORD                       *HDDPasswordBuffer = NULL;


#ifdef  SW_SMI_IO_ADDRESS
UINT8   SwSmiPort=SW_SMI_IO_ADDRESS;
#else
//
// Default Sw Smi port 
//
UINT8   SwSmiPort=0xB2;
#endif

//
// Global variable
//
EFI_SMM_COMMUNICATION_PROTOCOL  *gSmmCommunication = NULL;

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   InstallSecurityInterface
//
// Description: Checks whether the device supports Security mode or not.
//              If yes, installs SecurityProtocol.
//
// Input:
//      IN VOID             *BusInterface,
//      IN BOOLEAN          ModeFlag
//
// Output:
//  EFI_SUCCESS                      : Security Mode supported.
//  EFI_DEVICE_ERROR/EFI_UNSUPPORTED : Not supported.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS InstallSecurityInterface(
    IN VOID    *BusInterface,
    IN BOOLEAN ModeFlag )
{
    EFI_STATUS            Status;
    SECURITY_PROTOCOL     *IdeSecurityInterface;
    EFI_HANDLE            IdeDeviceHandle;
    IDENTIFY_DATA         IdentifyData;

    #if MASTER_PASSWORD_ENABLE
    UINT32                IdePasswordFlags = 0;
    #endif

    IDE_BUS_PROTOCOL      *IdeBusInterface;
    SATA_DEVICE_INTERFACE *SataDevInterface;
    IdeBusInterface  = (IDE_BUS_PROTOCOL*)BusInterface;
    SataDevInterface = (SATA_DEVICE_INTERFACE*)BusInterface;

    if ( ModeFlag == FALSE )
    {
        IdentifyData    = IdeBusInterface->IdeDevice.IdentifyData;
        IdeDeviceHandle = IdeBusInterface->IdeDeviceHandle;
    }
    else {
        IdentifyData    = SataDevInterface->IdentifyData;
        IdeDeviceHandle = SataDevInterface->IdeDeviceHandle;
    }

    if ((IdentifyData.Command_Set_Supported_82 & 0x2)  == 0 )
    {
        return EFI_UNSUPPORTED;
    }

    Status = pBS->AllocatePool( EfiBootServicesData,
                                sizeof(SECURITY_PROTOCOL),
                                (VOID**)&IdeSecurityInterface );

    if ( EFI_ERROR( Status ))
    {
        return Status;
    }

    //
    // HDDPasswordBuffer is allocated only once. The same buffer will be used for all devices.
    //
    if ( HDDPasswordBuffer == NULL )
    {
        Status = pBS->AllocatePool( EfiBootServicesData,
                                    sizeof(HDD_PASSWORD),
                                    (VOID**)&HDDPasswordBuffer );

        if ( EFI_ERROR( Status ))
        {
            return Status;
        }
    }

    if ( ModeFlag == FALSE )
    {
        IdeBusInterface->IdeSecurityInterface = IdeSecurityInterface;
        IdeSecurityInterface->BusInterface    = IdeBusInterface;
    }
    else {
        SataDevInterface->IdeSecurityInterface = IdeSecurityInterface;
        IdeSecurityInterface->BusInterface     = SataDevInterface;
    }

    IdeSecurityInterface->ReturnSecurityStatus    = ReturnSecurityStatus;
    IdeSecurityInterface->SecuritySetPassword     = SecuritySetPassword;
    IdeSecurityInterface->SecurityUnlockPassword  = SecurityUnlockPassword;
    IdeSecurityInterface->SecurityDisablePassword = SecurityDisablePassword;
    IdeSecurityInterface->SecurityFreezeLock      = SecurityFreezeLock;
    IdeSecurityInterface->SecurityEraseUnit       = SecurityEraseUnit;
    IdeSecurityInterface->ReturnIdePasswordFlags  = ReturnIdePasswordFlags;
    IdeSecurityInterface->pHDDPasswordBuffer      = HDDPasswordBuffer;

    //
    // Set ModeFlag to FALSE if IdeBus else TRUE for Ahci Bus.
    //
    IdeSecurityInterface->ModeFlag = ModeFlag;  // Ide Bus

    //
    //  Install the protocol on the device
    //
    Status = pBS->InstallMultipleProtocolInterfaces(
        &IdeDeviceHandle,
        &gIdeSecurityInterfaceGuid, IdeSecurityInterface,
        NULL );


    if ( gIDEBusEvtBootScript == NULL )
    {
        //
        // Create event for boot script
        //
        Status = CreateReadyToBootEvent(
            TPL_CALLBACK,
            IdeBusCallbackBootScript,
            NULL,
            &gIDEBusEvtBootScript
            );
        ASSERT_EFI_ERROR( Status );
    }

    //
    // Install the Readytoboot event to locate the AhciSMM protocol interface for IdeSmm driver.
    // AhciSmm protocol is valid only on AHCI mode. Other mode Callback is not needed
    //
    if ((BootEvent == NULL) && (ModeFlag))
    {
        //
        // Create event for boot script
        //
        Status = CreateReadyToBootEvent(
            TPL_CALLBACK,
            LocateAhciSmmServiceEvent,
            NULL,
            &BootEvent
            );
        ASSERT_EFI_ERROR( Status );
    }

    #if MASTER_PASSWORD_ENABLE
    //
    // Check if OEM_DEFAULT_MASTER_PASSWORD is enabled and
    // DISPLAY_MASTER_PASSWORD is Disabled
    //
    Status = ReturnIdePasswordFlags(
        (IDE_SECURITY_PROTOCOL*)IdeSecurityInterface,
        &IdePasswordFlags
        );

    ASSERT_EFI_ERROR( Status );

    if ((IdePasswordFlags & 0x0800) && (!(IdePasswordFlags & 0x0008)))
    {
        Status = OEMSetMasterPassword(
            (IDE_SECURITY_PROTOCOL*)IdeSecurityInterface );
        ASSERT_EFI_ERROR( Status );
    }
    #endif

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   StopSecurityModeSupport
//
// Description: Stops the security mode support by uninstalling the
//              security interface.
//
// Input:
//      IN VOID             *BusInterface,
//      IN BOOLEAN          ModeFlag
//
// Output:
//  EFI_SUCCESS         : Security Mode supported
//  EFI_DEVICE_ERROR    : Not supported
//
// Referrals: ReturnSecurityStatus
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS StopSecurityModeSupport(
    IN VOID    *BusInterface,
    IN BOOLEAN ModeFlag )
{
    EFI_STATUS            Status;
    EFI_HANDLE            IdeDeviceHandle;
    SECURITY_PROTOCOL     *IdeSecurityInterface;
    IDE_BUS_PROTOCOL      *IdeBusInterface;
    SATA_DEVICE_INTERFACE *SataDevInterface;

    IdeBusInterface  = (IDE_BUS_PROTOCOL*)BusInterface;
    SataDevInterface = (SATA_DEVICE_INTERFACE*)BusInterface;

    if ( ModeFlag == FALSE )
    {
        IdeSecurityInterface = IdeBusInterface->IdeSecurityInterface;
        IdeDeviceHandle      = IdeBusInterface->IdeDeviceHandle;
    }
    else {
        IdeSecurityInterface = SataDevInterface->IdeSecurityInterface;
        IdeDeviceHandle      = SataDevInterface->IdeDeviceHandle;
    }

    if ( IdeSecurityInterface == NULL )
    {
        return EFI_SUCCESS;
    }

    //
    //  Install Devicepath and BLOCK_IO on a new handle
    //
    Status = pBS->UninstallMultipleProtocolInterfaces(
        IdeDeviceHandle,
        &gIdeSecurityInterfaceGuid,
        IdeSecurityInterface,
        NULL );

    if ( Status == EFI_SUCCESS )
    {
        pBS->FreePool( IdeSecurityInterface );
        IdeSecurityInterface = NULL;
    }

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   IdeBusCallbackBootScript
//
// Description: Saves the IDE registers in a boot script.
//
// Input:
//      IN EFI_EVENT    Event
//      IN VOID         *Context
//
// Output:
//      None
//
// Referrals: SmmHDDPasswordInterface
//
// Notes: Will be called only if HDD loses power in S3 state. A SW SMI will
//        be created during S3 resume, to unlock the HDD and put it in
//        Freeze state.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID IdeBusCallbackBootScript(
    IN EFI_EVENT Event,
    IN VOID      *Context )
{
    EFI_S3_SAVE_STATE_PROTOCOL *BootScriptSave;
    EFI_STATUS                    Status;
    UINT8                         Value = SW_SMI_HDD_UNLOCK_PASSWORD;


    Status = pBS->LocateProtocol(
        &gEfiS3SaveStateProtocolGuid,
        NULL,
        &BootScriptSave
        );

    if ( EFI_ERROR( Status ))
    {
        return;
    }

    BOOT_SCRIPT_S3_IO_WRITE_MACRO(
        BootScriptSave,
        EfiBootScriptWidthUint8,
        SwSmiPort,
        1,
        &Value
        );

    //
    //Kill the Event
    //
    pBS->CloseEvent( Event );
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   IdeBusMiscSmmFeatureCallback
//
// Description: Saves the IDE registers in a boot script.
//
// Input:
//      IN EFI_EVENT    Event
//      IN VOID         *Context
//
// Output:
//      None
//
// Referrals: SmmHDDPasswordInterface
//
// Notes: Will be called only if HDD is in AHCI Mode or SWSP is Disabled
//        A SW SMI will be created during S3 resume, to Freeze the HDD in AHCI
//        or Disable SWSP command through Set feature command is Issued.
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID IdeBusMiscSmmFeatureCallback(
    IN EFI_EVENT Event,
    IN VOID      *Context )
{
    EFI_S3_SAVE_STATE_PROTOCOL *BootScriptSave;
    EFI_STATUS                    Status;
    UINT8                         Value = SW_SMI_HDD_MISC_SMM_FEATURES;

    static BOOLEAN                S3DataSaved = FALSE;

    if ( S3DataSaved )
    {
        return;
    }

    Status = pBS->LocateProtocol(
        &gEfiS3SaveStateProtocolGuid,
        NULL,
        &BootScriptSave
        );

    if ( EFI_ERROR( Status ))
    {
        return;
    }

    BOOT_SCRIPT_S3_IO_WRITE_MACRO(
        BootScriptSave,
        EfiBootScriptWidthUint8,
        SwSmiPort,
        1,
        &Value
        );

    S3DataSaved = TRUE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   HddSecurityEntryPoint
//
// Description: This function is the entry point for IDE SECURITY Driver.
//
// Input:
//  EFI_HANDLE          ImageHandle     Image handle.
//  EFI_SYSTEM_TABLE    *SystemTable    Pointer to the EFI system table.
//
// Output:
//  EFI_SUCCESS         Security interface installed.
//  EFI_ERROR           Security interface not installed or unable to locate
//                      some protocol.
//
// Notes:
//  Entry Points are used to locate or install protocol interfaces and
//  notification events.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS HddSecurityEntryPoint(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable )
{
    EFI_HANDLE Handle = NULL;
    EFI_STATUS Status;

    InitAmiLib( ImageHandle, SystemTable );

    Status = pBS->AllocatePool( EfiBootServicesData,
                                sizeof(HDD_SECURITY_INIT_PROTOCOL),
                                (VOID**)&gHddSecurityInitProtocol );

    if ( EFI_ERROR( Status ))
    {
        return Status;
    }

#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
    Status = pBS->LocateProtocol(&gEfiSmmControl2ProtocolGuid, NULL, &mSmmCtl);
#else
    Status = pBS->LocateProtocol(&gEfiSmmControlProtocolGuid, NULL, &mSmmCtl);
#endif

    if ( EFI_ERROR( Status )) {
        return EFI_UNSUPPORTED;
    }

    gHddSecurityInitProtocol->InstallSecurityInterface
        = InstallSecurityInterface;
    gHddSecurityInitProtocol->StopSecurityModeSupport
        = StopSecurityModeSupport;

    Status = pBS->InstallProtocolInterface(
        &Handle,
        &gHddSecurityInitProtocolGuid,
        EFI_NATIVE_INTERFACE,
        gHddSecurityInitProtocol
        );
    ASSERT_EFI_ERROR( Status );

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   ReturnSecurityStatus
//
// Description: Returns whether the Device supports Security Mode or not.
//              If supported, the current status of the device is returned.
//              Security status information will be retrieved from buffer
//              updated by IDENTIFY DEVICE(Command Code : 0xEC) command.
// Input:
//      IN IDE_SECURITY_PROTOCOL        *This,
//      UINT16                          *SecurityStatus
//
// Output:
//      EFI_SUCCESS                     : Security Mode supported
//      EFI_UNSUPPORTED                 : Not supported
//
//      UINT16                          *SecurityStatus
//                                      Bit 0 : Security Supported
//                                      Bit 1 : Security Enabled
//                                      Bit 2 : Security Locked
//                                      Bit 3 : Security Frozen
//                                      Bit 4 : Security Count Expired
//                                      Bit 5 : Enhanced Security Erase supported
//                                      Bit 6 : Enhanced Security Erase supported
//                                      Bit 8 : Security Level 0:High, 1:Maximum
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS ReturnSecurityStatus(
    IN IDE_SECURITY_PROTOCOL *This,
    UINT16                   *SecurityStatus )
{
    EFI_STATUS            Status = EFI_UNSUPPORTED;
    IDENTIFY_DATA         IdentifyData;
    VOID                  *BusInterface
        = ((SECURITY_PROTOCOL* )This)->BusInterface;
    BOOLEAN               ModeFlag = ((SECURITY_PROTOCOL* )This)->ModeFlag;
    IDE_BUS_PROTOCOL      *IdeBusInterface;
    SATA_DEVICE_INTERFACE *SataDevInterface;

    IdeBusInterface  = (IDE_BUS_PROTOCOL*)BusInterface;
    SataDevInterface = (SATA_DEVICE_INTERFACE*)BusInterface;

    if ( ModeFlag == FALSE )
    {
        IdentifyData = IdeBusInterface->IdeDevice.IdentifyData;
    }
    else {
        IdentifyData = SataDevInterface->IdentifyData;
    }

    //
    //  Check if Word 82 Bit 1 is set
    //
    if ( IdentifyData.Command_Set_Supported_82 & 0x2 )
    {
        Status          = EFI_SUCCESS;
        *SecurityStatus = IdentifyData.Security_Status_128;
    }

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   SecuritySetPassword
//
// Description: Sets User/Master Password using SECURITY_SET_PASSWORD
//              (Commnd Code : 0xF1) security command.
//
// Input:
//      IN IDE_SECURITY_PROTOCOL        *This,
//      UINT16                          Control,
//      UINT8                           *Buffer (32 Bytes)
//      UINT8                           RevisionCode (Valid when Control
//                                       Bit 0 = 1 of Master Password)
//
//  Control : Bit 0 : 0/1 User/Master Password
//            Bit 8 : 0/1 High/Maximum Security level
//
// Output:
//  EFI_SUCCESS     : Password set.
//  EFI_ERROR	    : Error in setting Password.
//
// Notes:
//  1. Buffer should be 32 bytes long (Mandatory). Even if Password is not 32
//     bytes long,pad it with zeros to create a buffer of 32 bytes.
//  2. If a MasterPassword with a valid Revision code(0x0001 through 0xFFFE)
//     already exists, a new MasterPassword with a different revisioncode
//     cannot be installed.MasterPassword needs to be unlocked, disabled and
//     then a new password can be installed.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SecuritySetPassword(
    IN IDE_SECURITY_PROTOCOL *This,
    UINT16                   Control,
    UINT8                    *Buffer,
    UINT16                   RevisionCode )
{
    EFI_STATUS            Status;

    #if (!DISABLE_SOFT_SET_PREV && !FORCE_HDD_PASSWORD_PROMPT ) || HDD_PASSWORD_ON_NVRAM
    UINT32                HddDataId  = 0;
    UINT16                UserMaster = 0;
    #endif
	#if MASTER_PASSWORD_REVISION_SWITCH == 2
    UINT16                Temp;
	IDENTIFY_DATA         IdentifyData;
	#endif
    VOID                  *BusInterface
        = ((SECURITY_PROTOCOL* )This)->BusInterface;
    BOOLEAN               ModeFlag = ((SECURITY_PROTOCOL* )This)->ModeFlag;
    IDE_BUS_PROTOCOL      *IdeBusInterface;
    SATA_DEVICE_INTERFACE *SataDevInterface;

    IdeBusInterface  = (IDE_BUS_PROTOCOL*)BusInterface;
    SataDevInterface = (SATA_DEVICE_INTERFACE*)BusInterface;

    Control &= 0x0101;          // Bit0 and Bit 8 are valid

    //
    //  Update the Identify device buffer
    //
    UpdateIdentifyData( BusInterface, ModeFlag );
	#if MASTER_PASSWORD_REVISION_SWITCH == 2
    if ( ModeFlag == FALSE )
    {
        IdentifyData = IdeBusInterface->IdeDevice.IdentifyData;
    }
    else {
        IdentifyData = SataDevInterface->IdentifyData;
    }
	#endif
    if ((Control & 1) && (RevisionCode == 0))
    {
        RevisionCode = DEFAULT_MASTER_PASSWORD_REVISION;

        #if MASTER_PASSWORD_REVISION_SWITCH == 1
        RevisionCode = CHANGED_MASTER_PASSWORD_REVISION;
        #endif

        #if MASTER_PASSWORD_REVISION_SWITCH == 2
        Temp = (IdentifyData.Master_Password_Rev_92)++;

        if ((Temp == 0) || (Temp == 0xFFFF))
        {
            Temp++;

            if ( Temp == 0xFFFF )
            {
                Temp++;
            }
        }
        RevisionCode = Temp;
        #endif

        if ( RevisionCode == DISABLED_MASTER_PASSWORD_REVISION )
        {
            RevisionCode++;

            if ((RevisionCode == 0) || (RevisionCode == 0xFFFF))
            {
                RevisionCode++;

                if ( RevisionCode == 0xFFFF )
                {
                    RevisionCode++;
                }
            }
        }
    }

    Status = SecurityCommonRoutine(
        BusInterface,
        Control,
        Buffer,
        SECURITY_SET_PASSWORD,
        RevisionCode,
        ModeFlag
        );

    //
    //  Update the Identify device buffer
    //
    UpdateIdentifyData( BusInterface, ModeFlag );

    //
    //  If Password is set successfully, save it.
    //
    if ( !EFI_ERROR( Status ))
    {
		if(!(Control & 1)) {
	        SmmHDDPasswordInterface( This,
                                 	Control,
                                 	Buffer,
                                 	SW_SMI_HDD_PASSWORD_SAVE );
		}

        #if (!DISABLE_SOFT_SET_PREV && !FORCE_HDD_PASSWORD_PROMPT ) || HDD_PASSWORD_ON_NVRAM
        Status = GatherIdeInfo( BusInterface, ModeFlag, &HddDataId );
        ASSERT_EFI_ERROR( Status );

        Status = SetHddSecDataInNvram( BusInterface,
                                       &HddDataId,
                                       Control,
                                       Buffer );
        ASSERT_EFI_ERROR( Status );
        #endif
    }

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   SecurityUnlockPassword
//
// Description: Unlocks the devices for media access using
//              SECURITY_UNLOCK(Command Code : 0xF2) security command.
//
// Input:
//      IN IDE_SECURITY_PROTOCOL        *This,
//      UINT16                          Control,
//      UINT8                           *Buffer (32 Bytes).
//
//      Control : Bit 0 : 0/1 User/Master Password
//      Buffer  : Pointer to password buffer which will be used to unlock the
//                drive.
//
// Output:
//  EFI_SUCCESS         : Hard disk is unlocked.
//  EFI_DEVICE_ERROR    : Failed to unlock the drive.
//
// Notes:
//  1. Buffer should be 32 bytes long (Mandatory). Even if Password is not
//     32 bytes long, pad it with zeros to create a buffer of 32 bytes.
//  2. If the password compare fails then the device returns command aborted
//     to the host and decrements the unlock counter. This counter is initially
//     set to five and is decremented for each password mismatch when SECURITY
//     UNLOCK is issued and the device is locked. When this counter reaches
//     zero then SECURITY UNLOCK and SECURITY ERASE UNIT commands are command
//     aborted until a power-on reset or a hardware reset.SECURITY UNLOCK
//     commands issued when the device is unlocked have no effect on the unlock
//     counter.
//  3. SECURITY_DISABLE_PASSWORD and SECURITY_SET_PASSWORD commands used when
//     security password is set and HDD is in unlocked state. This is done to
//     have consistency of output from this function.
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SecurityUnlockPassword(
    IN IDE_SECURITY_PROTOCOL *This,
    UINT16                   Control,
    UINT8                    *Buffer )
{
    EFI_STATUS            Status;

    #if (!DISABLE_SOFT_SET_PREV && !FORCE_HDD_PASSWORD_PROMPT ) || HDD_PASSWORD_ON_NVRAM
    UINT32                HddDataId  = 0;
    UINT16                UserMaster = 0;
    #endif
    UINT16                RevisionCode = 0;
    EFI_STATUS            Status1;
    VOID                  *BusInterface   \
        = ((SECURITY_PROTOCOL* )This)->BusInterface;
    BOOLEAN               ModeFlag   \
        = ((SECURITY_PROTOCOL* )This)->ModeFlag;
    HDD_PASSWORD          *HDDPwdChkBuffer   \
        = ((SECURITY_PROTOCOL* )This)->pHDDPasswordBuffer;
    UINT16                SecurityStatus = 0;
    #if MASTER_PASSWORD_ENABLE
    UINT32                IdePasswordFlags = 0;
    #endif

    IDE_BUS_PROTOCOL      *IdeBusInterface;
    SATA_DEVICE_INTERFACE *SataDevInterface;
    EFI_BLOCK_IO_PROTOCOL *BlockIo;

    IdeBusInterface  = (IDE_BUS_PROTOCOL*)BusInterface;
    SataDevInterface = (SATA_DEVICE_INTERFACE*)BusInterface;

    //
    // Update the Identify device buffer
    //
    UpdateIdentifyData( BusInterface, ModeFlag );

    Status = ReturnSecurityStatus( This, &SecurityStatus );

    Control &= 0x1;         // Bit0 is valid
    //
    // When in Setup, Current Password is prompted for consistency when Setting a New Password
    // Or Disabling the Existing Password. Follow a Special Path only in this case.
    //*****This Case Applies on only when (1)Password is Set & Security Enabled
    //                                    (2)Unlocked


    if ((SecurityStatus & BIT01) && (!(SecurityStatus & BIT02)))
    {
        if ( !(Control & 1))
        {
            //
            // (a)For Changing or Clearing User Password.
            //  1.Send a disable Password Command from the String Obtained from user.
            //  2.If Success Set the Password Same Password.
            //  3.Suitable if Logged in as Both User or Master
            //
            Status = SecurityCommonRoutine(
                BusInterface,
                Control,
                Buffer,
                SECURITY_DISABLE_PASSWORD,
                0,
                ModeFlag
                );

            if ( !EFI_ERROR( Status ))
            {
                SecurityCommonRoutine(
                    BusInterface,
                    Control,
                    Buffer,
                    SECURITY_SET_PASSWORD,
                    RevisionCode,
                    ModeFlag
                    );
            }
            return Status;
        }
        else {
            //
            // (a)For Changing or Clearing Master Password.
            // 1. Since Set Master Password Question will be Enabled Only if
            // Entered with Valid Master Password at Post Prompt We will have
            // the Password already saved.Use the same to check Now.
            // 2. Suitable only if Logged in as User.
            //
            if ( Strcmp( Buffer, HDDPwdChkBuffer->MasterPassword ) != 0 )
            {
                return EFI_DEVICE_ERROR;
            }
            return EFI_SUCCESS;
        }
    }

    Status = SecurityCommonRoutine(
        BusInterface,
        Control,
        Buffer,
        SECURITY_UNLOCK,
        0,
        ModeFlag
        );

    //
    // Update the Identify device buffer
    //
    UpdateIdentifyData( BusInterface, ModeFlag );

    if ( !EFI_ERROR( Status ))
    {
        //
        // Save the Password if unlocked Successfully.
        //
        SmmHDDPasswordInterface( This,
                                 Control,
                                 Buffer,
                                 SW_SMI_HDD_PASSWORD_SAVE );

        #if (!DISABLE_SOFT_SET_PREV && !FORCE_HDD_PASSWORD_PROMPT ) || HDD_PASSWORD_ON_NVRAM
        Status = GatherIdeInfo( BusInterface, ModeFlag, &HddDataId );
        ASSERT_EFI_ERROR( Status );

        Status = SetHddSecDataInNvram( BusInterface,
                                       &HddDataId,
                                       Control,
                                       Buffer );
        ASSERT_EFI_ERROR( Status );
        #endif

        //
        // After unlock the HDD set the OEM default master password
        // Check if OEM_DEFAULT_MASTER_PASSWORD is enabled and
        // DISPLAY_MASTER_PASSWORD is Disabled.
        //
        #if MASTER_PASSWORD_ENABLE
        Status = ReturnIdePasswordFlags( This, &IdePasswordFlags );
        ASSERT_EFI_ERROR( Status );

        if ((IdePasswordFlags & 0x0800) && (!(IdePasswordFlags & 0x0008)))
        {
            Status = OEMSetMasterPassword( This );
            ASSERT_EFI_ERROR( Status );
        }
        #endif

        if ( ModeFlag == FALSE ) {

            //
            // Check if BlockIO Protocol is already present on this device.
            //
            Status1 = pBS->HandleProtocol(IdeBusInterface->IdeDeviceHandle,
                                          &gEfiBlockIoProtocolGuid,
                                          &BlockIo
                                          );

            //
            // Connect only if BloCkIO protocol is not installed on the device.
            //
            if ( EFI_ERROR( Status1 )) {
                #if EFI_IDE_PROTOCOL || INTEL_IDE_PROTOCOL
                pBS->ConnectController(
                    IdeBusInterface->IdeControllerInterfaceHandle,
                    NULL,
                    NULL,
                    TRUE
                    );
                #else
                pBS->ConnectController(
                    IdeBusInterface->IdeControllerInterface->ControllerHandle,
                    NULL,
                    NULL,
                    TRUE
                    );
                #endif
            }
        }
    }

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   SecurityDisablePassword
//
// Description: Disables Password authentification before media access.
//              SECURITY_DISABLE_PASSWORD(Command Code : 0xF6) security
//              command is used to remove the user password.
//
// Input:
//      IN IDE_SECURITY_PROTOCOL        *This,
//      UINT16                          Control,
//      UINT8                           *Buffer (32 Bytes)
//
//  Control : Bit 0 : 0/1 User/Master Password
//
//  Buffer  : Pointer to password buffer which will be used to when sending
//            SECURITY_DISABLE_PASSWORD command.
//
//
// Output:
//  EFI_SUCCESS      : Security mode feature disabled.
//  EFI_DEVICE_ERROR : Security command failed.
//
// Notes:
//  1. Buffer should be 32 bytes long (Mandatory). Even if Password is not
//     32 bytes long,pad it with zeros to create a buffer of 32 bytes.
//  2. Password should be unlocked before disabling it.
//  3. SECURITY_DISABLE_PASSWORD command does not change the Master password
//     that may be reactivated later by setting a User password.
//  4. Normal erase mode is selected, the SECURITY ERASE UNIT command writes
//     binary zeroes to all user data areas.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SecurityDisablePassword(
    IN IDE_SECURITY_PROTOCOL *This,
    UINT16                   Control,
    UINT8                    *Buffer )
{
    EFI_STATUS Status;
    VOID       *BusInterface   \
        = ((SECURITY_PROTOCOL* )This)->BusInterface;
    BOOLEAN    ModeFlag  \
        = ((SECURITY_PROTOCOL* )This)->ModeFlag;

    #if (!DISABLE_SOFT_SET_PREV && !FORCE_HDD_PASSWORD_PROMPT ) || HDD_PASSWORD_ON_NVRAM
    UINT32     HddDataId = 0;
    UINT8      EmptyBuffer[IDE_PASSWORD_LENGTH];
    #endif

    Control &= 0x1;             // Bit0 is valid

    #if (!DISABLE_SOFT_SET_PREV && !FORCE_HDD_PASSWORD_PROMPT ) || HDD_PASSWORD_ON_NVRAM
    //
    //Update the Nvram buffer with NULL string.
    //
    pBS->SetMem( EmptyBuffer, IDE_PASSWORD_LENGTH, 0 );

    Status = GatherIdeInfo( BusInterface, ModeFlag, &HddDataId );
    ASSERT_EFI_ERROR( Status );

    Status = SetHddSecDataInNvram( BusInterface, &HddDataId, Control, Buffer );
    ASSERT_EFI_ERROR( Status );
    #endif

    Status = SecurityCommonRoutine( BusInterface,
                                    Control,
                                    Buffer,
                                    SECURITY_DISABLE_PASSWORD,
                                    0,
                                    ModeFlag
                                    );

    //
    // Update the Identify device buffer
    //
    UpdateIdentifyData( BusInterface, ModeFlag );

    if ( !EFI_ERROR( Status ))
    {
        SmmHDDPasswordInterface( This,
                                 Control,
                                 Buffer,
                                 SW_SMI_HDD_PASSWORD_REMOVE );
    }

    //
    // If the Empty string entered for the master password, Set the OEM
    // disable password with Disabled revision code.
    //
    if ( Control & 01 )
    {
        EFI_STATUS    DefaultStatus;
        DefaultStatus = SetDefaultMasterPassword( This );
        ASSERT_EFI_ERROR( DefaultStatus );
    }
    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   SetDefaultMasterPassword
//
// Description: Sets the Default Password String provided by OEM as
//              Master Password.
//
// Input:
//              IN IDE_SECURITY_PROTOCOL        *This
//
// Output:
//  EFI_SUCCESS : Default master password is set.
//  EFI_ERROR   : Default master password is not set.
//
// Notes:
//  DEFAULT_MASTER_PASSWORD_TAG token string will be set as master password.
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SetDefaultMasterPassword(
    IN IDE_SECURITY_PROTOCOL *This )
{
    EFI_STATUS Status;
    UINT8      Buffer[IDE_PASSWORD_LENGTH];
    UINT16     Control      = 1;
    UINT16     RevisionCode = DISABLED_MASTER_PASSWORD_REVISION;
    UINTN      i;
    UINT16     SecurityStatus = 0;

    MemSet( &Buffer, IDE_PASSWORD_LENGTH, 0 );

    for ( i = 0; i < IDE_PASSWORD_LENGTH; i++ )
    {
        Buffer[i] = (UINT8)sDefaultMasterPasswordTag[i];

        if ( sDefaultMasterPasswordTag[i] == L'\0' )
        {
            break;
        }
    }
    //
    // Set the OEM master password if the HDD is not in Frozen or locked state.
    //
    Status = ReturnSecurityStatus( This, &SecurityStatus );

    if ( !((SecurityStatus & 0x04) || (SecurityStatus & 0x08)))
    {
        Status = SecuritySetDefaultMasterPassword( This,
                                                   Control,
                                                   Buffer,
                                                   RevisionCode );
    }

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   SecuritySetDefaultMasterPassword
//
// Description: Sets default Master Password with disabled revision code.
//              SECURITY_SET_PASSWORD(Command code : 0xF1) security command
//              used to set default master password.
//
// Input:
//      IN IDE_BUS_PROTOCOL             *This,
//      UINT16                          Control,
//      UINT8                           *Buffer (32 Bytes)
//      UINT8                           RevisionCode (Valid when
//                                      Control Bit 0 = 1 of Master Password)
//
//      Control :       Bit 0 : 0/1 User/Master Password
//                      Bit 8 : 0/1 High/Maximum Security level
//
// Output:
//      EFI_SUCCESS : Password set
//      EFI_ERROR   : Error in setting Password
//
// Notes:
//  1. Buffer should be 32 bytes long (Mandatory). Even if Password is not
//     32 bytes long,pad it with zeros to create a buffer of 32 bytes.
//
//  2. If a MasterPassword with a valid Revision code already exists, a new
//     MasterPassword with a different revisioncode cannot be installed.
//     MasterPassword needs to be unlocked, disabled and then a new password
//     can be installed.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SecuritySetDefaultMasterPassword(
    IN IDE_SECURITY_PROTOCOL *This,
    UINT16                   Control,
    UINT8                    *Buffer,
    UINT16                   RevisionCode )
{
    VOID       *BusInterface = ((SECURITY_PROTOCOL* )This)->BusInterface;
    BOOLEAN    ModeFlag      = ((SECURITY_PROTOCOL* )This)->ModeFlag;
    EFI_STATUS Status;

    #if (!DISABLE_SOFT_SET_PREV && !FORCE_HDD_PASSWORD_PROMPT ) || HDD_PASSWORD_ON_NVRAM
    UINT32     HddDataId  = 0;
    UINT16     UserMaster = 0;
    #endif

    Control &= 0x0101;          // Bit0 and Bit 8 are valid

    //
    // Update the Identify device buffer
    //
    UpdateIdentifyData( BusInterface, ModeFlag );

    Status = SecurityCommonRoutine(
        BusInterface,
        Control,
        Buffer,
        SECURITY_SET_PASSWORD,
        RevisionCode,
        ModeFlag
        );

    //
    // Update the Identify device buffer
    //
    UpdateIdentifyData( BusInterface, ModeFlag );

    //
    // If Password is set successfully, save it.
    //
    if ( !EFI_ERROR( Status ))
    {
        SmmHDDPasswordInterface(
            This,
            Control,
            Buffer,
            SW_SMI_HDD_PASSWORD_SAVE
            );

        #if (!DISABLE_SOFT_SET_PREV && !FORCE_HDD_PASSWORD_PROMPT ) || HDD_PASSWORD_ON_NVRAM
        Status = GatherIdeInfo( BusInterface, ModeFlag, &HddDataId );
        ASSERT_EFI_ERROR( Status );

        Status = SetHddSecDataInNvram( BusInterface,
                                       &HddDataId,
                                       Control,
                                       Buffer );
        ASSERT_EFI_ERROR( Status );
        #endif
    }

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   SecurityFreezeLock
//
// Description: Prevents any further alteration of security status on the
//              device leaving the hard disk in Frozen mode. Frozen mode is
//              disabled by power-off or hardware reset. SECURITY_FREEZE_LOCK
//              (Command Code : 0xF5)security command is used to freeze hard disk.
//
// Input:
//      IN IDE_SECURITY_PROTOCOL        *This,
//
// Output:
//      EFI_SUCCESS : Hard disk is successdully kept in FROZEN state.
//      EFI_ERROR   : Security Freeze command failed.
//
// Notes:
//  Commands disabled by SECURITY FREEZE LOCK are:
//      - SECURITY SET PASSWORD
//      - SECURITY UNLOCK
//      - SECURITY DISABLE PASSWORD
//      - SECURITY ERASE PREPARE
//      - SECURITY ERASE UNIT
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SecurityFreezeLock(
    IN IDE_SECURITY_PROTOCOL *This )
{
    EFI_STATUS            Status;
    VOID                  *BusInterface
        = ((SECURITY_PROTOCOL* )This)->BusInterface;
    BOOLEAN               ModeFlag
        = ((SECURITY_PROTOCOL* )This)->ModeFlag;
    UINT16                SecurityStatus = 0;
	IDENTIFY_DATA         IdentifyData;
    COMMAND_STRUCTURE     CommandStructure;
	IDE_BUS_PROTOCOL      *IdeBusInterface;
    SATA_DEVICE_INTERFACE *SataDevInterface;

    IdeBusInterface  = (IDE_BUS_PROTOCOL*)BusInterface;
    SataDevInterface = (SATA_DEVICE_INTERFACE*)BusInterface;

    Status=ReturnSecurityStatus( This, &SecurityStatus );
    if(EFI_ERROR(Status)) {
        return Status;
    }

    // Frezze Lock command will not work with the HDD is in Locked State.
    // So return with error if the HDD is in Locked State.    
    if ( SecurityStatus & 4 ) {
        return EFI_DEVICE_ERROR;
    }
    
    ZeroMemory( &CommandStructure, sizeof(COMMAND_STRUCTURE));

    if ( ModeFlag == FALSE ) {
        IdentifyData = IdeBusInterface->IdeDevice.IdentifyData;
    } else {
        IdentifyData = SataDevInterface->IdentifyData;
    }
    
    CommandStructure.Command = SECURITY_FREEZE_LOCK;

    #if !DISABLE_SOFT_SET_PREV && !FORCE_HDD_PASSWORD_PROMPT
    if ( SecurityStatus & 2 )
    {
        if ( IdentifyData.Reserved_76_79[3] & 0x0040 )
        {
            Status = TransferPwDataFromNvramToSmram( This );
            ASSERT_EFI_ERROR( Status );
        }
    }
    #endif

    Status =  CommonNonDataHook( BusInterface,
                                    CommandStructure,
                                     ModeFlag );
	ASSERT_EFI_ERROR (Status);
    CommandStructure.Features = DEV_CONFIG_FREEZE_LOCK_FEATURES;
    CommandStructure.Command  = DEV_CONFIG_FREEZE_LOCK;
    Status                    = CommonNonDataHook( BusInterface,
                                                   CommandStructure,
                                                   ModeFlag );

    //
    //	Update the Identify device buffer
    //
    UpdateIdentifyData( BusInterface, ModeFlag );

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   SecurityEraseUnit
//
// Description: Erases the Device content.SECURITY_ERASE_UNIT
//              (Command code : 0xF4) security command is used for device
//              erasing.
//
// Input:
//      IN IDE_SECURITY_PROTOCOL    *This,
//      UINT16                      Control,
//      UINT8                       *Buffer
//
//      Control :       Bit 0 : 0/1 User/Master Password
//						Bit 1 : 0/1 Normal Erase/ Enhanced Erase
//      Buffer  : Pointer to password buffer which will be used to when sending
//                SECURITY_ERASE_UNIT command.
//
// Output:
//  EFI_SUCCESS      : Successfull completion of Erase command.
//  EFI_DEVICE_ERROR : EraseUnit command failed to complete.
//
// Notes:
//  If a device is locked with USER PASSWORD and Maximum security level set
//  then only way to unlock the device with MASTER password is to erase the
//  contents of the device.
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SecurityEraseUnit(
    IN IDE_SECURITY_PROTOCOL *This,
    UINT16                   Control,
    UINT8                    *Buffer )
{
    EFI_STATUS        Status;

    VOID              *BusInterface   \
        = ((SECURITY_PROTOCOL* )This)->BusInterface;
    BOOLEAN           ModeFlag   \
        = ((SECURITY_PROTOCOL* )This)->ModeFlag;
    COMMAND_STRUCTURE CommandStructure;

    ZeroMemory( &CommandStructure, sizeof(COMMAND_STRUCTURE));
    CommandStructure.Command = SECURITY_ERASE_PREPARE;

    Status = CommonNonDataHook( BusInterface, CommandStructure, ModeFlag );
	ASSERT_EFI_ERROR (Status);
    //
    // Issue Security Erase Unit
    //
    Control &= 0x3;             // Bit0 and Bit 1 is valid
    Status   = SecurityCommonRoutine(
        BusInterface,
        Control,
        Buffer,
        SECURITY_ERASE_UNIT,
        0,
        ModeFlag
        );

    //
    // Update the Identify device buffer
    //
    UpdateIdentifyData( BusInterface, ModeFlag );

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   ReturnIdePasswordFlags
//
// Description: Returns Ide Password Variables and Flags.
//
// Input:
//          IN IDE_SECURITY_PROTOCOL        *This,
//          UINT32                          *IdePasswordFlags
//
// Output:
//  EFI_SUCCESS : IDEPassword Status Flag is returned in IdePasswordFlags.
//  EFI_ERROR   : Error while retrieving the SecurityStatus.
//
//
// Referrals:
//
//  IdePasswordFlags:
// **TBD Only Bit0, Bit03, Bit11, Bit16, Bit17 are Supported Currently
//
//         BIT0                             User Password Support;
//         BIT1                             Master Password Support;
//         BIT2                             Display User Password;
//         BIT3                             Display Master Password;
//         BIT4                             Display User Password Status;
//         BIT5                             Display Master Password Status;
//         BIT6                             Freeze Lock Hdds;
//         BIT7                             Single User Password;
//         BIT8                             Single Master Password;
//         BIT9                             System User Password as Hdd User Password;
//         BIT10                            System Admin Password as Hdd Master Password;
//         BIT11                            OEM Default Master Password
//         BIT12                            Prompt for User Password in Post
//         BIT13                            Prompt for Master Password in Post
//         BIT14                            Display Security Status in Setup
//         BIT15                            DUMMY Security Token.
//         BIT16                            MasterPassword Set Status.
//         BIT17                            UserPassword Set Status.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS ReturnIdePasswordFlags(
    IN IDE_SECURITY_PROTOCOL *This,
    UINT32                   *IdePasswordFlags )
{
    EFI_STATUS            Status         = EFI_SUCCESS;
    UINT32                Temp           = 0;
    UINT16                MasterPwdRev   = 0;
    UINT16                SecurityStatus = 0;
    VOID                  *BusInterface
        = ((SECURITY_PROTOCOL* )This)->BusInterface;
    BOOLEAN               ModeFlag
        = ((SECURITY_PROTOCOL* )This)->ModeFlag;
    IDENTIFY_DATA         IdentifyData;
    IDE_BUS_PROTOCOL      *IdeBusInterface;
    SATA_DEVICE_INTERFACE *SataDevInterface;

    IdeBusInterface  = (IDE_BUS_PROTOCOL*)BusInterface;
    SataDevInterface = (SATA_DEVICE_INTERFACE*)BusInterface;

    if ( ModeFlag == FALSE )
    {
        IdentifyData = IdeBusInterface->IdeDevice.IdentifyData;
    }
    else {
        IdentifyData = SataDevInterface->IdentifyData;
    }



    #if MASTER_PASSWORD_ENABLE
    Temp |= BIT01;
    #endif


    #if DISPLAY_MASTER_PASSWORD
    Temp |= BIT03;
    #endif

    #if OEM_DEFAULT_MASTER_PASSWORD
    Temp |= BIT11;
    #endif


    MasterPwdRev = IdentifyData.Master_Password_Rev_92;

    if ((MasterPwdRev != 0x0000) && (MasterPwdRev != 0xFFFF)    \
        && (MasterPwdRev != DISABLED_MASTER_PASSWORD_REVISION))
    {
        Temp |= BIT16;
    }

    Status = ReturnSecurityStatus( This, &SecurityStatus );

    if ( EFI_ERROR( Status ))
    {
        return Status;
    }

    if ( SecurityStatus & BIT01 )
    {
        Temp |= BIT17;
    }

    *IdePasswordFlags = (UINT32)Temp;
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:	SecurityCommonRoutine
//
// Description:	This is common security routine which will be called from
//              security routines. It is from this routine most of the security
//              commands are sent to the drive.
//
// Input:
//      IN VOID                         *BusInterface,
//      UINT16                          Control,
//      UINT8                           *Buffer (32 Bytes),
//      UINT8                           SecurityCommand
//      UINT8                           RevisionCode
//                                      (Valid for Master password)
//
//  Control : Bit 0 : 0/1 User/Master Password
//            Bit 1 : 0/1 Normal/Enhanced Erase
//            Bit 8 : 0/1 High/Maximum Security level
//
//  Buffer  : Pointer to password buffer which will be used to when sending
//            SecurityCommand.
//
//
// Output:
//  EFI_STATUS : EFI_SUCCESS/EFI_DEVICE_ERROR
//
// Notes:
//  1. Buffer should be 32 bytes long (Mandatory). Even if Password is not
//     32 bytes long, pad it with zeros to create a buffer of 32 bytes.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SecurityCommonRoutine(
    IN VOID *BusInterface,
    UINT16  Control,
    UINT8   *Buffer,
    UINT8   SecurityCommand,
    UINT16  RevisionCode,
    BOOLEAN ModeFlag )
{
    EFI_STATUS Status;
    UINT16     *SecurityBuffer;

    //
    // Allocate 512 Bytes
    //
    Status = pBS->AllocatePool( EfiBootServicesData,
                                SECURITY_BUFFER_LENGTH,
                                (VOID**)&SecurityBuffer );

    if ( EFI_ERROR( Status ))
    {
        return Status;
    }

    //
    // Clear the Buffer
    //
    ZeroMemory( SecurityBuffer, SECURITY_BUFFER_LENGTH );

    SecurityBuffer[0] = Control;

    //
    // Copy 32 Bytes
    //
    pBS->CopyMem( &(SecurityBuffer[1]), Buffer, IDE_PASSWORD_LENGTH );

    //
    // If Security Set password command is for Master, then update the
    // revision code.
    //
    if ((SecurityCommand == SECURITY_SET_PASSWORD) & (Control & 1))
    {
        if ((RevisionCode != 0) && (RevisionCode != 0xFFFF))
        {
            SecurityBuffer[17] = RevisionCode;
        }
    }

    //
    // Write 512 Bytes
    //
    Status = CommonReadWritePioHook(
        BusInterface,
        SecurityBuffer,
        SECURITY_BUFFER_LENGTH,
        0,
        SecurityCommand,
        TRUE,
        ModeFlag
        );

    pBS->FreePool( SecurityBuffer );
    return Status;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   SmmHDDPasswordInterface
//
// Description: Manages HDD Password in SMM area.
//
// Input:
//          IN IDE_SECURITY_PROTOCOL    *This,
//          UINT16                      Control,
//          UINT8                       *Buffer
//          UINT8                       Action
//
//
//  Control : Bit 0 : 0/1 User/Master Password
//
//  Buffer  : Pointer to password buffer which will be used to when sending
//            SecurityCommand.
//
// Output:
// EFI_SUCCESS      : Password saved which will be used to unlock the drive
//                    during S3 resume.
// EFI_DEVICE_ERROR : Error in saving Password.
//
// Notes:
//  1. Buffer should be 32 bytes long (Mandatory). Even if Password is not
//     32 bytes long, pad it with zeros to create a buffer of 32 bytes.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SmmHDDPasswordInterface(
    IN IDE_SECURITY_PROTOCOL *This,
    UINT16                   Control,
    UINT8                    *Buffer,
    UINT8                    Action )
{
#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
    UINT8               DataSize = 1;
#else
    UINTN               DataSize = 1;
#endif
    UINT8                 SwSmiValue = Action;
    UINTN                 i;

    EFI_SMM_COMMUNICATE_HEADER *SmmCommunicateHeader;
    
    UINT8                           *CommunicateBuffer = NULL;
    UINTN                            CommunicateBufferSize;
    
    HDD_PASSWORD          *HDDPasswordBuffer   \
        = ((SECURITY_PROTOCOL*)This)->pHDDPasswordBuffer;

    VOID                  *BusInterface   \
        = ((SECURITY_PROTOCOL* )This)->BusInterface;

    BOOLEAN               ModeFlag   \
        = ((SECURITY_PROTOCOL* )This)->ModeFlag;
    EFI_PCI_IO_PROTOCOL *PciIo;
    UINTN   SegNum;
    UINTN   BusNum;
    UINTN   DevNum;
    UINTN   FuncNum;
    EFI_STATUS  Status;

    IDE_BUS_PROTOCOL      *IdeBusInterface;
    SATA_DEVICE_INTERFACE *SataDevInterface;

    IdeBusInterface  = (IDE_BUS_PROTOCOL*)BusInterface;
    SataDevInterface = (SATA_DEVICE_INTERFACE*)BusInterface;

    if ( ModeFlag == FALSE )
    {
        //
        //  If HDD is not going to lose power in S3, no need to unlock password and put it in
        //  Freeze state during S3 resume.
        //
        if ( !(IdeBusInterface->IdeBusInitInterface->Flags   \
               & S3_RESUME_UNLOCK_HDD_PASSWORD))
        {
            return EFI_SUCCESS;
        }

        HDDPasswordBuffer->BaseAddress
            = IdeBusInterface->IdeDevice.Regs.CommandBlock.DataReg;
        HDDPasswordBuffer->Device = IdeBusInterface->IdeDevice.Device;
        HDDPasswordBuffer->DeviceControlReg
            = IdeBusInterface->IdeDevice.Regs.ControlBlock.DeviceControlReg;
        PciIo = IdeBusInterface->PciIO;

    }
    else {

        PciIo=SataDevInterface->AhciBusInterface->PciIO;
        HDDPasswordBuffer->PortNumber   = SataDevInterface->PortNumber;
        HDDPasswordBuffer->PMPortNumber = SataDevInterface->PMPortNumber;
    }

    //
    // Get PCI Bus/Device/Function number
    //
    Status = PciIo->GetLocation (   PciIo,
                                    &SegNum,
                                    &BusNum,
                                    &DevNum,
                                    &FuncNum
                                     );
	ASSERT_EFI_ERROR (Status);
										
    //
    // Save it for IdeSmm to initilize the controller.
    //
    HDDPasswordBuffer->BusNo=(UINT8)BusNum;
    HDDPasswordBuffer->DeviceNo=(UINT8)DevNum;
    HDDPasswordBuffer->FunctionNo=(UINT8)FuncNum;

    HDDPasswordBuffer->Signature = '$HDD'; // 0x44444824; //
    HDDPasswordBuffer->Control   = Control;
    HDDPasswordBuffer->ModeFlag  = ModeFlag;

    //
    //      HDDPasswordBuffer->RevisionCode = RevisionCode;
    //
    if ( Control & 0x01 )
    {
        for ( i = 0; i < sizeof (HDDPasswordBuffer->MasterPassword); i++ )
        {
            HDDPasswordBuffer->MasterPassword[i] = Buffer[i];
        } // end of for
    }
    else {
        for ( i = 0; i < sizeof (HDDPasswordBuffer->UserPassword); i++ )
        {
            HDDPasswordBuffer->UserPassword[i] = Buffer[i];
        } // end of for
    }

    //
    // Calculate Size of Communication buffer
    //
    CommunicateBufferSize  = SMM_COMMUNICATE_HEADER_SIZE + HDD_PASSWORD_SIZE;

    //
    // Allocate Memory for Communication buffer.   
    //
    if ( CommunicateBuffer == NULL ) {
        Status = pBS->AllocatePool( EfiBootServicesData,
                                    CommunicateBufferSize,
                                    (VOID**)&CommunicateBuffer );
        if ( EFI_ERROR( Status )) {    	
            return Status;
        }    
    }

    //
    // Copy Smm Communicate Header Here
    //
    SmmCommunicateHeader = (EFI_SMM_COMMUNICATE_HEADER *)CommunicateBuffer;

    if(SwSmiValue == SW_SMI_HDD_PASSWORD_SAVE) {
        pBS->CopyMem( &SmmCommunicateHeader->HeaderGuid, &gSaveHddPasswordGuid, sizeof( EFI_GUID ) );
    } else if(SwSmiValue == SW_SMI_HDD_PASSWORD_REMOVE) {
        pBS->CopyMem( &SmmCommunicateHeader->HeaderGuid, &gRemoveHddPasswordGuid, sizeof( EFI_GUID ) );
    } else {
        ASSERT(FALSE);
        return EFI_NOT_FOUND;
    }

    SmmCommunicateHeader->MessageLength  = HDD_PASSWORD_SIZE;

    //
    // Copy HDD Password Data Here
    //
    pBS->CopyMem( &SmmCommunicateHeader->Data, HDDPasswordBuffer, sizeof(HDD_PASSWORD) );

    //
    // Send data to SMM.
    //
    Status = pBS->LocateProtocol (&gEfiSmmCommunicationProtocolGuid, NULL, (VOID **) &gSmmCommunication);
    if (EFI_ERROR(Status)) {
        ASSERT_EFI_ERROR(Status);
        return Status;	
    }


    Status = gSmmCommunication->Communicate (gSmmCommunication, CommunicateBuffer, &CommunicateBufferSize);
    if (EFI_ERROR(Status)) {
        ASSERT_EFI_ERROR(Status);
        return Status;	
    }
	
	//
    // Freeing the Memory Allocated for Communication Buffer.
    //
	Status = pBS->FreePool(CommunicateBuffer);
    if (EFI_ERROR(Status)) {
        ASSERT_EFI_ERROR(Status);
        return Status;	
    }
	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   UpdateIdentifyData
//
// Description: Updates Identify command data.IDENTIFY_DATA(Command Code: 0xEC)
//              command used to update the Identifydata buffer.
//
// Input:
//  IN IDE_BUS_PROTOCOL             *IdeBusInterface,
//  IN BOOLEAN                      ModeFlag
//
// Output:
//      EFI_SUCCESS : Identifydata updated successfully.
//      EFI_ERROR   : Failed to update the identifydata.
//
// Notes:   ATA/ATAPI device type should have beeen known already.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS UpdateIdentifyData(
    IN IDE_BUS_PROTOCOL *BusInterface,
    IN BOOLEAN          ModeFlag )
{
    EFI_STATUS        Status;
    COMMAND_STRUCTURE CommandStructure;

    ZeroMemory( &CommandStructure, sizeof(COMMAND_STRUCTURE));

    CommandStructure.ByteCount = sizeof(IDENTIFY_DATA);
    CommandStructure.Command   = IDENTIFY_COMMAND;

    Status = CommonPioDataHook( BusInterface, CommandStructure, ModeFlag );

    return Status;
}

#if MASTER_PASSWORD_ENABLE
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   OEMSetMasterPassword
//
// Description: Sets the Default Password String provided by OEM as
//              Master Password. SECURITY_SET_PASSWORD(Command Code : 0xF1)
//              security command is used to set the master password.
// Input:
//          IN IDE_SECURITY_PROTOCOL        *This,
//
// Output:
//      EFI_SUCCESS : OEM Master password set successfully.
//      EFI_ERROR   : Failed to set OEM Master passord to the hard disk.
//
// Notes:
//  OEM_MASTER_PASSWORD_TAG token string will be set as oem master password.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS OEMSetMasterPassword(
    IN IDE_SECURITY_PROTOCOL *This )
{
    EFI_STATUS Status = 0;
    UINT8      Buffer[IDE_PASSWORD_LENGTH];
    UINT16     Control      = 1;
    UINT16     RevisionCode = 0xFFFE;
    UINTN      i;
    UINT16     SecurityStatus = 0;

    MemSet( &Buffer, IDE_PASSWORD_LENGTH, 0 );

    for ( i = 0; i < IDE_PASSWORD_LENGTH; i++ )
    {
        Buffer[i] = (UINT8)sOemMasterPasswordTag[i];

#if defined SETUP_PASSWORD_NON_CASE_SENSITIVE && SETUP_PASSWORD_NON_CASE_SENSITIVE 
        Buffer[i] = ((Buffer[i]>='a')&&(Buffer[i]<='z'))?(Buffer[i]+'A'-'a'):Buffer[i];
#endif
        if ( sOemMasterPasswordTag[i] == L'\0' )
        {
            break;
        }
    }

    //
    //Set the OEM master password if the HDD is not in Frozen or locked state.
    //
    ReturnSecurityStatus( This, &SecurityStatus );

    if ( !((SecurityStatus & 0x04) || (SecurityStatus & 0x08)))
    {
        Status = SecuritySetPassword( This, Control, Buffer, RevisionCode );
    }

    return Status;
}

#endif

#if (!DISABLE_SOFT_SET_PREV && !FORCE_HDD_PASSWORD_PROMPT ) || HDD_PASSWORD_ON_NVRAM
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   GatherIdeInfo
//
// Description:	Gathers Ide Device's Bus,Func,Device & PM/PS/SM/SS Info.
//
// Input:
//          IN VOID                     *BusInterface,
//          OUT UINT32                  *HddDataId
//
// Output:
//      EFI_SUCCESS : IDE info retrieved successfully.
//      EFI_ERROR   : Failed to get IDE info.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GatherIdeInfo(
    IN VOID    *BusInterface,
    BOOLEAN    ModeFlag,
    OUT UINT32 *HddDataId )
{
    EFI_STATUS            Status;
    UINTN                 SegNum;
    UINTN                 BusNum;
    UINTN                 DevNum;
    UINTN                 FuncNum;
    UINT16                wBDF;
    UINT16                wPortPMModeFlag;
    UINT8                 ChannelorPort;
    UINT8                 DeviceorPMPort;
    EFI_PCI_IO_PROTOCOL   *PciIo;
    IDE_BUS_PROTOCOL      *IdeBusInterface;
    SATA_DEVICE_INTERFACE *SataDevInterface;

    IdeBusInterface  = (IDE_BUS_PROTOCOL*)BusInterface;
    SataDevInterface = (SATA_DEVICE_INTERFACE*)BusInterface;

    if ( ModeFlag == FALSE )
    {
        PciIo          = IdeBusInterface->PciIO;
        ChannelorPort  = IdeBusInterface->IdeDevice.Channel;
        DeviceorPMPort =  IdeBusInterface->IdeDevice.Device;
    }
    else {
        PciIo          = SataDevInterface->AhciBusInterface->PciIO;
        ChannelorPort  = SataDevInterface->PortNumber;
        DeviceorPMPort =  SataDevInterface->PMPortNumber;
    }

    Status = PciIo->GetLocation( PciIo, &SegNum, &BusNum, &DevNum, &FuncNum );

    wBDF            = (UINT16)((BusNum << 8) | (DevNum << 3) | FuncNum );
    wPortPMModeFlag = (UINT16)((ChannelorPort << 12)   \
                               | (DeviceorPMPort << 4)   \
                               | ((UINT8)ModeFlag));

    *HddDataId = (UINT32)(((UINT16)wBDF << 16) + ((UINT16)wPortPMModeFlag));

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   SetHddSecDataInNvram
//
// Description: Sets Hdd Security Data in NVRAM Area.
//
// Input:
//          IN VOID                     *BusInterface,
//          UINT32                      *HddDataId
//          UINT16                      Control,
//          UINT8                       *Buffer
//
//  Control : Bit 0 : 0/1 User/Master Password
//
//  Buffer  : Pointer to password buffer which will be used to when sending
//            SecurityCommand.
// Output:
//  EFI_SUCCESS : HDDPassword saved in NVram which later will be transfered to
//                SMRAM.
//  EFI_ERROR   : Failed to save the HDD password in nvram.
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SetHddSecDataInNvram(
    IN VOID *BusInterface,
    UINT32  *HddDataId,
    UINT16  Control,
    UINT8   *Buffer )
{
    EFI_STATUS Status;
    HDDSECDATA *HddSecData;
    UINTN      i;
    CHAR16     *HddSecutiyData = NULL;
    UINT8      CryptBuffer[IDE_PASSWORD_LENGTH];
    UINTN      HddSecDataSize = sizeof (HDDSECDATA);
    UINT32     ParseIndex;

    pBS->AllocatePool( EfiBootServicesData, sizeof (HDDSECDATA), &HddSecData );
    pBS->SetMem( HddSecData, sizeof(HDDSECDATA), 0 );
    pBS->AllocatePool( EfiBootServicesData,
                       IDE_PASSWORD_LENGTH,
                       &HddSecutiyData );
    pBS->SetMem( HddSecutiyData, IDE_PASSWORD_LENGTH, 0 );
    ConvertHddDataIdToString( *HddDataId, HddSecutiyData );

    //
    // Get the password variable so that other password will be
    // maintained in the nvram variable.
    //
    Status = pRS->GetVariable(
        HddSecutiyData,
        &gIdeSecurityPwNvguid,
        NULL,
        &HddSecDataSize,
        HddSecData
        );

    if ( EFI_ERROR( Status ))
    {
        pBS->SetMem( HddSecData, sizeof(HDDSECDATA), 0 );
    }


    for ( ParseIndex = 0; ParseIndex < IDE_PASSWORD_LENGTH; ParseIndex++ )
    {
        if ( Buffer[ParseIndex] == 0 )
        {
            break;
        }
    }

    HddSecData->UserMaster     = Control;
    HddSecData->PasswordLength = ParseIndex;

    if ( Control & 0x01 )
    {
        EncodeDecodePassword( Buffer, CryptBuffer, ParseIndex );

        for ( i = 0; i < ParseIndex; i++ )
        {
            HddSecData->HddMasterPassword[i] = CryptBuffer[i];
        }

        for ( i = ParseIndex; i < sizeof (HddSecData->HddMasterPassword); i++ )
        {
            HddSecData->HddMasterPassword[i] = 00;
        }
    }
    else {
        EncodeDecodePassword( Buffer, CryptBuffer, ParseIndex );

        for ( i = 0; i < ParseIndex; i++ )
        {
            HddSecData->HddUserPassword[i] = CryptBuffer[i];
        }

        for ( i = ParseIndex; i < sizeof (HddSecData->HddUserPassword); i++ )
        {
            HddSecData->HddUserPassword[i] = 00;
        }
    }

    Status = pRS->SetVariable(
        HddSecutiyData,
        &gIdeSecurityPwNvguid,
        EFI_VARIABLE_NON_VOLATILE  \
        | EFI_VARIABLE_BOOTSERVICE_ACCESS   \
        | EFI_VARIABLE_RUNTIME_ACCESS,
        HddSecDataSize,
        HddSecData
        );

    ASSERT_EFI_ERROR( Status );
    pBS->FreePool( HddSecData );

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   TransferPwDataFromNvramToSmram
//
// Description: Transfers Stored Data from NVRAM to SMRAM.
//
// Input:
//          IN IDE_SECURITY_PROTOCOL    *This,
//
// Output:
//  EFI_SUCCESS      : Password saved in SMRAM.
//  EFI_DEVICE_ERROR : Password not saved in SMRAM.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS TransferPwDataFromNvramToSmram(
    IN IDE_SECURITY_PROTOCOL *This )
{
    UINT32       HddDataId       = 0;
    UINT16       UserMaster      = 0;
    CHAR16       *HddSecutiyData = NULL;
    HDDSECDATA   *HddSecData;
    HDD_PASSWORD *HDDPasswordBuffer   \
        = ((SECURITY_PROTOCOL* )This)->pHDDPasswordBuffer;
    VOID         *BusInterface   \
        = ((SECURITY_PROTOCOL* )This)->BusInterface;
    BOOLEAN      ModeFlag = ((SECURITY_PROTOCOL* )This)->ModeFlag;
    EFI_STATUS   Status;
    UINT16       Control;
    UINTN        i;
    UINTN        HddSecDataSize = sizeof (HDDSECDATA);
    UINT8        CryptBuffer[IDE_PASSWORD_LENGTH];
    UINT8        Buffer[IDE_PASSWORD_LENGTH];

    Status = GatherIdeInfo( BusInterface, ModeFlag, &HddDataId );
    ASSERT_EFI_ERROR( Status );

    pBS->AllocatePool( EfiBootServicesData,
                       IDE_PASSWORD_LENGTH,
                       &HddSecutiyData );
    pBS->SetMem( HddSecutiyData, IDE_PASSWORD_LENGTH, 0 );
    ConvertHddDataIdToString( HddDataId, HddSecutiyData );

    pBS->AllocatePool( EfiBootServicesData, sizeof (HDDSECDATA), &HddSecData );
    pBS->SetMem( HddSecData, sizeof(HDDSECDATA),  0 );

    pBS->SetMem( Buffer,     IDE_PASSWORD_LENGTH, 0 );

    Status = pRS->GetVariable(
        HddSecutiyData,
        &gIdeSecurityPwNvguid,
        NULL,
        &HddSecDataSize,
        HddSecData
        );

    ASSERT_EFI_ERROR( Status );

    Control = HddSecData->UserMaster;

    if ( Control & 0x01 )
    {
        for ( i = 0; i < sizeof (HddSecData->HddMasterPassword); i++ )
        {
            CryptBuffer[i] = HddSecData->HddMasterPassword[i];
        }

        EncodeDecodePassword( CryptBuffer, Buffer, HddSecData->PasswordLength );
    }
    else {
        for ( i = 0; i < sizeof (HddSecData->HddUserPassword); i++ )
        {
            CryptBuffer[i] = HddSecData->HddUserPassword[i];
        }

        EncodeDecodePassword( CryptBuffer, Buffer, HddSecData->PasswordLength );
    }

        SmmHDDPasswordInterface( This,
                             Control,
                             Buffer,
                             SW_SMI_HDD_PASSWORD_SAVE );

    pBS->FreePool( HddSecData );

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   ConvertHddDataIdToString
//
// Description: Converts Integer HDD Data Id to String.
//
// Input:
//           IN  UINT32          DataId,
//           OUT CHAR16          *String
// Output:
//	        None
//
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ConvertHddDataIdToString(
    IN UINT32  DataId,
    OUT CHAR16 *String )
{
    Swprintf( String, L"%*d", 0, DataId );
    return;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:	EncodeDecodePassword
//
// Description:	Encodes/Decodes Password.
//
// Input:
//           IN   UINT8        *InputString,
//           OUT  UINT8        *OutputString,
//           IN   UINT32       StringLen
// Output:
//          None
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID EncodeDecodePassword(
    IN UINT8  *InputString,
    OUT UINT8 *OutputString,
    IN UINT32 StringLength )
{
    UINT8  *Key;
    UINT32 i;
    UINT32 KeyIndex;
    UINT32 KeyLength;

    if ((InputString == NULL) || (StringLength == 0))
    {
        return;
    }

    KeyLength = (UINT32)( Strlen( HDD_PWD_ENCRYPTION_KEY ));
    pBS->AllocatePool( EfiBootServicesData, KeyLength, (VOID**)&Key );
    pBS->SetMem( Key, KeyLength, 0 );
    pBS->CopyMem( Key, HDD_PWD_ENCRYPTION_KEY, KeyLength );
    ASSERT( Key != NULL );

    for ( i = 0, KeyIndex = 0; i < StringLength; i++, KeyIndex++ )
    {
        if ( KeyIndex == (KeyLength - 1))
        {
            KeyIndex = 0;
        }
        OutputString[i] = (UINT8) ( InputString[i] ^ Key[KeyIndex] );
    }

    pBS->FreePool( Key );
    return;
}
  
#endif


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   CommonNonDataHook
//
// Description: Common Hook which Calls Non Data Command.
//
// Input:       IN    VOID*                 BusInterface,
//              IN    COMMAND_STRUCTURE     CommandStructure,
//              IN    BOOLEAN               ModeFlag
//
// Output:
//  EFI_SUCCESS                      : Non Data command completed successfully.
//  EFI_DEVICE_ERROR/EFI_UNSUPPORTED : Not supported.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CommonNonDataHook(
    IN VOID              *BusInterface,
    IN COMMAND_STRUCTURE CommandStructure,
    IN BOOLEAN           ModeFlag )
{
    EFI_STATUS            Status;
    IDE_BUS_PROTOCOL      *IdeBusInterface;
    SATA_DEVICE_INTERFACE *SataDevInterface;

    IdeBusInterface  = (IDE_BUS_PROTOCOL*)BusInterface;
    SataDevInterface = (SATA_DEVICE_INTERFACE*)BusInterface;

    if ( ModeFlag == FALSE )
    {
        CommandStructure.Device = IdeBusInterface->IdeDevice.Device << 4;

        //
        // Idebus API changed from Core 4.6.5.2. Added Core Version check for the OLD 
        // Core support.
        //
#if defined CORE_COMBINED_VERSION && (CORE_COMBINED_VERSION > 0x4028b) 
        Status = IdeBusInterface->IdeNonDataCommand(
            IdeBusInterface,
            CommandStructure.Features,
            (UINT8)CommandStructure.SectorCount,
            (UINT8)(CommandStructure.SectorCount >> 8),
            CommandStructure.LBALow,
            CommandStructure.LBALowExp,
            CommandStructure.LBAMid,
            CommandStructure.LBAMidExp,
            CommandStructure.LBAHigh,
            CommandStructure.LBAHighExp,
            CommandStructure.Device,
            CommandStructure.Command
            );
#else 
        Status = IdeBusInterface->IdeNonDataCommand(
            IdeBusInterface,
            CommandStructure.Features,
            (UINT8)CommandStructure.SectorCount,
            CommandStructure.LBALow,
            CommandStructure.LBAMid,
            CommandStructure.LBAHigh,
            CommandStructure.Device,
            CommandStructure.Command
            );

#endif
    }
    else {
        Status = SataDevInterface->AhciBusInterface->ExecuteNonDataCommand(
            SataDevInterface,
            CommandStructure
            );
    }

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   CommonPioDataHook
//
// Description: Common Hook which Calls Pio Data Command.
//
// Input:       IN    VOID*                 BusInterface,
//              IN    COMMAND_STRUCTURE     CommandStructure,
//              IN    BOOLEAN               ModeFlag
//
// Output:
//  EFI_SUCCESS                      : Pio data command completed.
//  EFI_DEVICE_ERROR/EFI_UNSUPPORTED : Not supported.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CommonPioDataHook(
    IN VOID              *BusInterface,
    IN COMMAND_STRUCTURE CommandStructure,
    IN BOOLEAN           ModeFlag )
{
    EFI_STATUS            Status;
    IDE_BUS_PROTOCOL      *IdeBusInterface;
    SATA_DEVICE_INTERFACE *SataDevInterface;
    VOID                  *IdentifyBuffer = NULL;

    IdeBusInterface  = (IDE_BUS_PROTOCOL*)BusInterface;
    SataDevInterface = (SATA_DEVICE_INTERFACE*)BusInterface;

    Status = pBS->AllocatePool( EfiBootServicesData,
                                sizeof(IDENTIFY_DATA),
                                (VOID**)&IdentifyBuffer
                                );
    //
    //  Read Identifydata
    //
    CommandStructure.Buffer = IdentifyBuffer;

    if ( ModeFlag == FALSE )
    {
        CommandStructure.Device = IdeBusInterface->IdeDevice.Device << 4;
        Status                  = IdeBusInterface->AtaPioDataIn(
            IdeBusInterface,
            CommandStructure
                .Buffer,
            CommandStructure
                .ByteCount,
            CommandStructure
                .Features,
            (UINT8)
            CommandStructure.SectorCount,
            CommandStructure
                .LBALow,
            CommandStructure
                .LBAMid,
            CommandStructure
                .LBAHigh,
            CommandStructure
                .Device,
            CommandStructure
                .Command,
            FALSE
            );
        //
        // Copy Identify Data
        //
        pBS->CopyMem( &(IdeBusInterface->IdeDevice.IdentifyData),
                      IdentifyBuffer,
                      sizeof(IDENTIFY_DATA)
                      );
    }
    else {
        Status = SataDevInterface->AhciBusInterface->ExecutePioDataCommand(
            SataDevInterface,
            &CommandStructure,
            FALSE
            );

        //
        // Copy Identify Data
        //
        pBS->CopyMem( &(SataDevInterface->IdentifyData),
                      IdentifyBuffer,
                      sizeof(IDENTIFY_DATA)
                      );
    }

    pBS->FreePool( IdentifyBuffer );

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
//
// Procedure:   CommonReadWritePioHook
//
// Description: Common Hook which Calls Read Write PIO.
//
// Input:
//  IN VOID                         *BusInterface,
//  VOID                            *Buffer,
//  UINTN                           ByteCount,
//  UINT64                          LBA
//  IN UINT8                        ReadWriteCommand,
//  IN BOOLEAN                      ReadWrite        Read/Write = 0/1
//  IN BOOLEAN                      ModeFlag
//
// Output:
//      *Buffer updated from PIO command.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CommonReadWritePioHook(
    IN VOID     *BusInterface,
    IN OUT VOID *Buffer,
    IN UINTN    ByteCount,
    IN UINT64   LBA,
    IN UINT8    ReadWriteCommand,
    IN BOOLEAN  ReadWrite,
    IN BOOLEAN  ModeFlag )
{
    EFI_STATUS            Status;
    IDE_BUS_PROTOCOL      *IdeBusInterface;
    SATA_DEVICE_INTERFACE *SataDevInterface;

    IdeBusInterface  = (IDE_BUS_PROTOCOL*)BusInterface;
    SataDevInterface = (SATA_DEVICE_INTERFACE*)BusInterface;

    if ( ModeFlag == FALSE )
    {
        Status = IdeBusInterface->AtaReadWritePio(
            IdeBusInterface,
            Buffer,
            ByteCount,
            0,
            ReadWriteCommand,
            TRUE
            );
    }
    else {
        Status = SataDevInterface->AhciBusInterface->SataReadWritePio(
            SataDevInterface,
            Buffer,
            ByteCount,
            0,
            ReadWriteCommand,
            TRUE
            );
    }

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   LocateAhciSmmServiceEvent
//
// Description: Generates the SwSMI to Locate the AhciSMM Protocol
//              that is used by IdeSMM.
//
// Input:
//        EFI_EVENT     Event,
//        VOID          *Context
//
// Output:
//    None
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID LocateAhciSmmServiceEvent(
    EFI_EVENT Event,
    VOID      *Context )
{
#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
    UINT8 DataSize = 1;
#else
    UINTN DataSize = 1;
#endif
    UINT8 SwSmiValue = SW_SMI_AHCI_LOCATE_AHCI_SMM;

    //
    // Now Generate a SW SMI to Locate all the SMM protocols used by IDESMM
    //
    mSmmCtl->Trigger( mSmmCtl, &SwSmiValue, &DataSize, 0, 0 );
    
    return;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
