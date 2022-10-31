//**********************************************************************//
//**********************************************************************//
//**                                                                  **//
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **//
//**                                                                  **//
//**                       All Rights Reserved.                       **//
//**                                                                  **//
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **//
//**                                                                  **//
//**                       Phone: (770)-246-8600                      **//
//**                                                                  **//
//**********************************************************************//
//**********************************************************************//

//****************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//
//****************************************************************************
// Revision History
// ----------------
// $Log: $
// 
// 
//****************************************************************************
#pragma warning ( disable : 4201 )
#define PORT80( value )  \
_asm {                   \
  _asm mov dx, 0x80      \
  _asm mov al, value     \
  _asm out dx, al        \
}
#define PORT80INC  \
_asm {                   \
  _asm in al, 0x80      \
  _asm inc al     \
  _asm out 0x80, al        \
}

#include <Token.h>
#include <AmiDxeLib.h>
#include <Protocol/SmmBase.h>
#include <Protocol/SmmGpiDispatch2.h>
#include <Acpi11.h>
#include <EC.H>
#include <Protocol/GlobalNvsArea.h>
#include <Protocol/EcAccess.h>
#include <Protocol/AcpiSupport.h>
#include <AcpiRes.h>
#include <PchAccess.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/AcpiModeEnable.h>

EFI_GUID gEfiEcAccessProtocolGuid = EC_PROTOCOL_GUID;
EFI_GUID gAcpiSupportGuid = EFI_ACPI_SUPPORT_GUID;
EFI_GUID gAcpiEnDispatchProtocolGuid  = EFI_ACPI_EN_DISPATCH_PROTOCOL_GUID;
EFI_GUID gAcpiDisDispatchProtocolGuid = EFI_ACPI_DIS_DISPATCH_PROTOCOL_GUID;
extern EFI_GUID                 gAmiGlobalVariableGuid;

//
// Global variables
//
static EFI_EC_ACCESS_PROTOCOL     mEcAccess;
EFI_SMM_GPI_DISPATCH2_PROTOCOL    *mSmmGpiDispatch;
EFI_ACPI_SUPPORT_PROTOCOL	        *gEfiAcpiSupport = 0;
UINT8                             *IGDFPtr;
VOID                              *mAcpiReg;
EFI_GLOBAL_NVS_AREA               *mGlobalNvsAreaPtr;
BOOLEAN                           ASLFlagsFound;
EFI_EVENT                         mAcpiEvent;

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	ECGetMotherBoardID
//
// Description:
//
// Input:       UINT8		*FabID
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ECGetMotherBoardID(
    UINT8 *FabID )
{
    EFI_STATUS Status;

    EcWriteCmd( SMC_FAB_ID );
    Status = EcReadData( FabID );
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	LoadPopupToMemory
//
// Description: Loads Popup Binary to memory
//
// Input:       None
//
// Output:      Updates global variables
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID LoadPopupToMemory( )
{
    // Porting if needed.
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	*GetDSDTTable
//
// Description:
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID* GetDSDTTable( )
{
    INTN                   Index;
    PACPI_HDR              Table;
    EFI_ACPI_TABLE_VERSION Version;
    UINTN                  Handle;
    EFI_STATUS             Status;

    for ( Index = 0;; ++Index )
    {
        Status = gEfiAcpiSupport->GetAcpiTable(
            gEfiAcpiSupport,
            Index,
            &Table,
            &Version,
            &Handle
            );

        if ( EFI_ERROR( Status )) {
            return 0;
        }

        if (((PACPI_HDR)Table)->Signature == FACP_SIG ) {
            return (VOID*)(UINTN)((PFACP32)Table )->DSDT;
        }
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	LocateFlagsInAsl
//
// Description: Locates flags in Asl
//
// Input:       None
//
// Output:      Updates global variable
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS LocateFlagsInAsl( )
{
    EFI_STATUS   Status;
    PACPI_HDR    DsdtTable;
    UINT32       Length;
    UINT8        *ptr;
    UINT8        *pPRScope;
    UINT32       PRScopeLength;

    ASL_OBJ_INFO ObjInfo;

    Status = pBS->LocateProtocol( &gAcpiSupportGuid, NULL, &gEfiAcpiSupport );

    if ( EFI_ERROR( Status )) {
        TRACE(((UINTN)TRACE_ALWAYS, "ACPI Support Protocol not found.\n"));
        return Status;
    }

    DsdtTable = GetDSDTTable( );

    if ( !DsdtTable ) {
        TRACE(((UINTN)TRACE_ALWAYS, "DSDT Table not found.\n"));
        return EFI_NOT_FOUND;
    }

    Length = DsdtTable->Length - sizeof(ACPI_HDR);
    ptr    = (UINT8*)DsdtTable + sizeof(ACPI_HDR);

    // Locate _SB scope
    Status = GetAslObj( ptr, Length, "_SB", otScope, &ObjInfo );

    if ( EFI_ERROR( Status )) {
        TRACE(((UINTN)TRACE_ALWAYS, "Scope _SB not found.\n"));
        return Status;
    }
    pPRScope      = ObjInfo.DataStart;
    PRScopeLength = (UINT32)ObjInfo.Length;

    // Locate IGD flag
    Status = GetAslObj( pPRScope, Length, "IGDF", otName, &ObjInfo );

    if ( EFI_ERROR( Status )) {
        TRACE(((UINTN)TRACE_ALWAYS, "Name IGDF not found.\n"));
        return Status;
    }
    IGDFPtr = (UINT8*)ObjInfo.DataStart + 1;    //Skip byte prefix

    ASLFlagsFound = TRUE;
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	EcSMIHandler
//
// Description: Invoked on reads from SW SMI port with value SW_SMI_USB. This
//              function dispatches the USB Request Packets (URP) to the
//              appropriate functions.
//
// Input:       mSmst->CpuSaveState->ESI - Pointer to the URP (USB Request
//                                                          Packet structure)
//              DispatchHandle  - EFI Handle
//              DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT
//
// Output:      bRetValue       Zero on successfull completion
//              Non-zero on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS EcSMIHandler (
	IN  EFI_HANDLE	DispatchHandle,
	IN CONST VOID  *DispatchContext  OPTIONAL,
    IN OUT VOID    *CommBuffer      OPTIONAL,
    IN OUT UINTN   *CommBufferSize  OPTIONAL
	)
{
  EFI_STATUS  Status; 
	UINT8 		  Query;
  UINT16      Data16;
  // Make sure that SMI was caused by GPI07
	Data16 = IoRead16(PM_BASE_ADDRESS + R_PCH_ALT_GP_SMI_STS);  //EIP140507
	if (Data16 & BIT07) {
		do {
			Query = 0;
			Status = EcQueryCommand (&Query);
	    switch(Query){
	    case 0x33:
	      TRACE((-1,"Lid\n"));
	      break;
	    case 0x34:
        TRACE((-1,"Virtual battery\n"));
        break;
	    default:
        break;
	    }
	    
			if (Query == 0) break;
			IoWrite8(0x80, Query);
		}while (1);

	ECDisableSMINotify();
	IoWrite16((PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_STS), Data16 & BIT23);		// Clear Status //EIP140507
	ECEnableSMINotify();
  }
	
	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	ECEnableACPIMode
//
// Description: Switch EC to ACPI Mode.
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ECEnableACPIMode(
    )
{
		UINT32 	GpiRoute;
		UINT16	Data16;

//	Route GPIO_SUS 7 & GPIO_CORE 7 to SCI, and GPIO_SUS 0 to NOP
		GpiRoute = MmioRead32 (PMC_BASE_ADDRESS + R_PCH_PMC_GPI_ROUT);
		GpiRoute = GpiRoute & 0xfffc3fff;
		GpiRoute = GpiRoute | 0x00028000;
		MmioWrite32((PMC_BASE_ADDRESS + R_PCH_PMC_GPI_ROUT), GpiRoute);	

		// Disable GPIO_SUS 0 to cause SMI in ALT_GPI_SMI_EN
		Data16 = IoRead16(PM_BASE_ADDRESS + R_PCH_ALT_GP_SMI_EN); //EIP140507

		IoWrite16((PM_BASE_ADDRESS + R_PCH_ALT_GP_SMI_EN), (Data16 & 0xFFFE));   //EIP140507

//    ECDisableSMINotify();                 // 0x05
    EcWriteCmd( SMC_SMI_DISABLE );        // 0xBC
    EcWriteCmd( SMC_ENABLE_ACPI_MODE );   // 0xAA

    if ( mGlobalNvsAreaPtr->AlsEnable ) {
        EcWriteCmd( SMC_ALS_ENABLE );     // 0x0E
        EcWriteData( 0x01 ); // enable ALS
    }
    else {
        EcWriteCmd( SMC_ALS_ENABLE );
        EcWriteData( 0x00 ); // disable ALS
    }
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	ECDisableACPIMode
//
// Description: Switch EC to Non- ACPI Mode.
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ECDisableACPIMode(
    )
{
    EcWriteCmd( SMC_DISABLE_ACPI_MODE );
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	ECDisableSMINotify
//
// Description:
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ECDisableSMINotify(
    )
{
    EcWriteCmd( SMC_DISABLE_SMI_NOTIFY );
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	ECEnableSMINotify
//
// Description:
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ECEnableSMINotify(
    )
{
    EcWriteCmd( SMC_ENABLE_SMI_NOTIFY );
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	ECShutDownSystem
//
// Description:
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ECShutDownSystem(
    )
{
    EFI_STATUS Status;

    Status = EcWriteCmd( SMC_SYSTEM_POWER_OFF );
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	ECGetECVersion
//
// Description: Switch EC to ACPI Mode.
//
// Input:       UINT8		*Revision
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ECGetECVersion(
    UINT8 *Revision )
{
    EFI_STATUS Status;

    EcWriteCmd( SMC_READ_REVISION );
    Status = EcReadData( Revision );
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	ECEnableLan
//
// Description:
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ECEnableLan(
    )
{
    EFI_STATUS Status;

    Status = EcWriteCmd( SMC_LAN_ON );
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	ECDisableLan
//
// Description:
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ECDisableLan(
    )
{
    EFI_STATUS Status;

    Status = EcWriteCmd( SMC_LAN_OFF );
    return Status;
}

//----------------------------------------------------------------------------
// Procedure:	ECDeepSxConfig
//
// Description: 
//
// Input:       None
//              
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
ECDeepSxConfig(
UINT8     EcData
)
{
	EFI_STATUS	Status;
	Status = EcWriteCmd (SMC_DEEPSX_CMD);
	Status = EcWriteData (EcData);
	return Status;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	EcTurboCtrlMode
//
// Description: 
//
// Input:       None
//              
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
EcTurboCtrlMode(
UINT8    Enable,
UINT8    ACBrickCap,
UINT8    ECPollPeriod,
UINT8    ECGuardBandValue,
UINT8    ECAlgorithmSel,
UINT8    ECHybridPowerBoost,
UINT8    ECHybridCurrentLow,
UINT8    ECHybridCurrentHigh
)
{
	EFI_STATUS	Status;
    if (Enable) {
	Status = EcWriteCmd (SMC_TURBOCTRL_TESTMODE_ENABLE);
	Status = EcWriteData (ACBrickCap);
	Status = EcWriteData (ECPollPeriod);
	Status = EcWriteData (ECGuardBandValue);
	Status = EcWriteData (ECAlgorithmSel);
    Status = EcWriteData (ECHybridPowerBoost);
    Status = EcWriteData (ECHybridCurrentLow);
    Status = EcWriteData (ECHybridCurrentHigh);
    } else {
	Status = EcWriteCmd (SMC_TURBOCTRL_TESTMODE_DISABLE);
    }

	return Status;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ECAcpiEnableLink
//
// Description: This routine will be called when ACPI enabled.
//
// Parameters:  DispatchHandle - SMM Dispatch Handle
//
// Returns:     None
//
// Modified:
//
// Referrals:
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ECAcpiEnableLink (
    IN EFI_HANDLE   DispatchHandle )
{
    mEcAccess.SMINotifyDisable();
    mEcAccess.AcpiEnable();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ECAcpiDisableLink
//
// Description: This routine will be called when ACPI disabled.
//
// Parameters:  DispatchHandle - SMM Dispatch Handle
//
// Returns:     None
//
// Modified:
//
// Referrals:
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ECAcpiDisableLink (
    IN EFI_HANDLE   DispatchHandle )
{
    mEcAccess.AcpiDisable();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: CreateECAcpiEnDisLink
//
// Description: 
//
// Input:       None
//              
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID CreateECAcpiEnDisLink (
    IN EFI_EVENT        Event,
    IN VOID             *Context )
{
    EFI_STATUS                  Status;
    EFI_HANDLE                  Handle;
    EFI_ACPI_DISPATCH_PROTOCOL  *mAcpiEnDispatch;
    EFI_ACPI_DISPATCH_PROTOCOL  *mAcpiDisDispatch;

    Status = gSmst->SmmLocateProtocol( &gAcpiEnDispatchProtocolGuid, \
                                  NULL, \
                                  &mAcpiEnDispatch );
    if(EFI_ERROR(Status))return;

    Status = mAcpiEnDispatch->Register( mAcpiEnDispatch, \
                                        ECAcpiEnableLink, \
                                        &Handle );

    Status = gSmst->SmmLocateProtocol( &gAcpiDisDispatchProtocolGuid, \
                                  NULL, \
                                  &mAcpiDisDispatch );
    if(EFI_ERROR(Status))return;

    Status = mAcpiDisDispatch->Register( mAcpiDisDispatch, \
                                         ECAcpiDisableLink, \
                                         &Handle );
    //Kill the Event
    pBS->CloseEvent(Event);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	InitializeEcSmm
//
// Description: 
//
// Input:
//	IN EFI_HANDLE           ImageHandle,
//	IN EFI_SYSTEM_TABLE     *SystemTable
//
// Output:                      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS InitializeEcSmm(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable )
{
    EFI_STATUS 						Status;
    EFI_HANDLE						EcHandle;
    EFI_SMM_GPI_REGISTER_CONTEXT	EcContext;
    UINT32                          GpiRoute;
    UINT8                           ECRev1 = 0;
    UINT8                           ECRev2 = 0;
    EFI_GLOBAL_NVS_AREA_PROTOCOL   *GlobalNvsAreaProtocol;
    
    ECRev1        = 0;
    ECRev2        = 0;
    
    InitAmiLib(ImageHandle, SystemTable);
  
    Status = gSmst->SmmLocateProtocol( &gEfiSmmGpiDispatch2ProtocolGuid, NULL, &mSmmGpiDispatch );
  
    if ( EFI_ERROR( Status )) {
        return Status;
    }
  
    //
    // Register SMI handler for GPI1
    //
  
    EcContext.GpiNum =  BIT07; // for Intel GPI pin number //bit location 1 -> BIT 1
  
    Status = mSmmGpiDispatch->Register( mSmmGpiDispatch,
                                        EcSMIHandler,
                                        &EcContext,
                                        &EcHandle );

    mEcAccess.Handle              = NULL;
    mEcAccess.QuerryCmd           = EcQueryCommand;
    mEcAccess.WriteCmd            = EcWriteCmd;
    mEcAccess.WriteData           = EcWriteData;
    mEcAccess.ReadData            = EcReadData;
    mEcAccess.ReadMem             = EcReadMem;
    mEcAccess.WriteMem            = EcWriteMem;
    mEcAccess.AcpiEnable          = ECEnableACPIMode;
    mEcAccess.AcpiDisable         = ECDisableACPIMode;
    mEcAccess.SMINotifyEnable     = ECEnableSMINotify;
    mEcAccess.SMINotifyDisable    = ECDisableSMINotify;
    mEcAccess.ShutDownSystem      = ECShutDownSystem;
    mEcAccess.GetMotherBoardID    = ECGetMotherBoardID;
    mEcAccess.GetECVersion        = ECGetECVersion;
    mEcAccess.EnableLan           = ECEnableLan;
    mEcAccess.DisableLan          = ECDisableLan;
    mEcAccess.DeepSxConfig        = ECDeepSxConfig;
    mEcAccess.TurboCtrlMode       = EcTurboCtrlMode;
  
    Status = pBS->InstallMultipleProtocolInterfaces(
        &mEcAccess.Handle,
        &gEfiEcAccessProtocolGuid,
        &mEcAccess,
        NULL
        );
  
    if ( EFI_ERROR( Status )) {
        TRACE(((UINTN)-1, "InstallMultipleProtocolInterfaces returned %r\n", Status));
        return EFI_UNSUPPORTED;
    }
  
    if ( EFI_ERROR( Status )) {
        TRACE(((UINTN)-1, "Couldn't register the EC SMI handler.  Status: %r\n", Status));
        return Status;
    }
    //
    // Program GPI_ROUT according to board schematics.
    // 0:1F:0:0x44 --> PMC_BASE_ADDRESS, GPIO_ROUT --> (PMC_BASE_ADDRESS + 0x58)
    // GPIO_SUS0 --> Routed to generate SMI#
    // GPIO_SUS7 --> Routed to generate SMI#
    // GPIOC_7 --> Routed to generate Runtime SCI

    GpiRoute = MmioRead32 (PMC_BASE_ADDRESS + R_PCH_PMC_GPI_ROUT);

    GpiRoute = GpiRoute & 0xfffc3ffc;
    GpiRoute = GpiRoute | 0x00024001;
  	MmioWrite32((PMC_BASE_ADDRESS + R_PCH_PMC_GPI_ROUT), GpiRoute);	

    //
    // Enable GPIO_SUS 0 to cause SMI in ALT_GPI_SMI_EN
    //
  	//IoWrite8((PM_BASE_ADDRESS + R_PCH_ALT_GP_SMI_EN), 0x01);  //EIP140507
    //
    // Enable GPIO_SUS 7 & GPIO_CORE 7  in GPE0a_EN to cause SCI
    //
  	IoWrite32((PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_EN), (BIT23 | BIT24) );  //EIP140507

    EcWriteCmd( SMC_DISABLE_ACPI_MODE );
    EcWriteCmd( SMC_ENABLE_POWER_SWITCH );
    EcWriteCmd( SMC_ENABLE_SMI_NOTIFY );
    EcWriteCmd( SMC_SMI_ENABLE );
    
    //
    // Locate our shared data area
    //
    Status = pBS->LocateProtocol( &gEfiGlobalNvsAreaProtocolGuid, NULL, &GlobalNvsAreaProtocol );
    ASSERT_EFI_ERROR( Status );
    mGlobalNvsAreaPtr = GlobalNvsAreaProtocol->Area;
    
    //
    // SoftSDV v0.64 can not simulate this command so cause the endless while-loop.
    //
    EcWriteCmd( SMC_READ_REVISION );
  
    do {
        Status = EcReadData( &ECRev1 );
    } while ( ECRev1 == 0 );
    Status = EcReadData( &ECRev2 );
    pRS->SetVariable(
        L"ECRev1",
        &gAmiGlobalVariableGuid,
            EFI_VARIABLE_BOOTSERVICE_ACCESS
                + EFI_VARIABLE_RUNTIME_ACCESS,
            sizeof (UINT8),
            &ECRev1
        );
    pRS->SetVariable(
        L"ECRev2",
        &gAmiGlobalVariableGuid,
        EFI_VARIABLE_BOOTSERVICE_ACCESS
        + EFI_VARIABLE_RUNTIME_ACCESS,
        sizeof (UINT8),
        &ECRev2
        );
    
    Status=RegisterProtocolCallback( &gAcpiEnDispatchProtocolGuid, \
                                     CreateECAcpiEnDisLink, \
                                     NULL, \
                                     &mAcpiEvent, \
                                     &mAcpiReg );
    ASSERT_EFI_ERROR(Status);

    //If this protocol has been installed we can use it rigth on the way
    CreateECAcpiEnDisLink(mAcpiEvent, NULL);

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	EcQueryCommand
//
// Description: Read the EC Query Value
//
// Input:       UINT8   *pQdata
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EcQueryCommand(
    UINT8 *pQdata )
{
    EFI_STATUS Status;

    Status = EcWriteCmd( SMC_QUERY_SMI );
    Status = EcReadData( pQdata );
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	EcIbFree
//
// Description: Wait till EC I/P buffer is free.
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS EcIbFree( )
{
    UINT8 Status;

    do {
      gSmst->SmmIo.Io.Read( &gSmst->SmmIo,
                              SMM_IO_UINT8,
                              EcCommandPort,
                              1,
                              &Status );
    } while ( Status & 2 );
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	EcObFull
//
// Description: Wait till EC O/P buffer is full
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS EcObFull( )
{
    UINT8 Status;

    do {
      gSmst->SmmIo.Io.Read( &gSmst->SmmIo, SMM_IO_UINT8, EcCommandPort, 1, &Status );
    } while ( !(Status & 1));

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	EcWriteCmd
//
// Description: Send EC command
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS EcWriteCmd(
    UINT8 cmd )
{
    EcIbFree();
    gSmst->SmmIo.Io.Write( &gSmst->SmmIo,
                           SMM_IO_UINT8,
                           EcCommandPort,
                           1,
                           &cmd );
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	EcWriteData
//
// Description: Write Data from EC data port
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS EcWriteData(
    UINT8 data )
{
    EcIbFree( );
    gSmst->SmmIo.Io.Write( &gSmst->SmmIo,
                           SMM_IO_UINT8,
                           EcDataPort,
                           1,
                           &data );
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	EcReadDatas
//
// Description: Read Data from EC data Port.
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS EcReadData(
    UINT8 *pData )
{
    if ( EFI_ERROR( EcObFull( ))) {
        return EFI_DEVICE_ERROR;
    }
    gSmst->SmmIo.Io.Read( &gSmst->SmmIo, SMM_IO_UINT8, EcDataPort, 1, pData );
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	EcReadMem
//
// Description: Read Data from EC Memory from location pointed by Index.
//
// Input:       UINT8	Index,
//				UINT8	*Data
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS EcReadMem(
    UINT8 Index,
    UINT8 *Data )
{
    UINT8 cmd = SMC_READ_EC;

    EcWriteCmd( cmd );
    EcWriteData( Index );
    EcReadData( Data );
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	EcWriteMem
//
// Description: Write Data to EC memory at location pointed by Index.
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS EcWriteMem(
    UINT8 Index,
    UINT8 Data )
{
    UINT8 cmd = SMC_WRITE_EC;

    EcWriteCmd( cmd );
    EcWriteData( Index );
    EcWriteData( Data );
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	GetDevicePathSize
//
// Description:
//
// Input:       EFI_DEVICE_PATH_PROTOCOL *Path
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static UINTN GetDevicePathSize(
    EFI_DEVICE_PATH_PROTOCOL *Path )
{
    EFI_DEVICE_PATH_PROTOCOL *ptr = Path;

    //Find last node of device path.
    while ( !isEndNode( ptr ))
    {
        ptr = NEXT_NODE( ptr );
    }

    //above loop doesn't count last node.
    return (UINTN)ptr - (UINTN)Path + sizeof(EFI_DEVICE_PATH_PROTOCOL);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	AppendDevicePath
//
// Description:
//
// Input:       EFI_DEVICE_PATH_PROTOCOL *Path1,
//				EFI_DEVICE_PATH_PROTOCOL *Path2
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static EFI_DEVICE_PATH_PROTOCOL* AppendDevicePath(
    EFI_DEVICE_PATH_PROTOCOL *Path1,
    EFI_DEVICE_PATH_PROTOCOL *Path2 )
{
    EFI_DEVICE_PATH_PROTOCOL *NewPath;
    UINTN                    PathSize1, PathSize2;

    if ( !Path1 && !Path2 ) {
        return NULL;  //No paths
    }
    PathSize1 = GetDevicePathSize( Path1 );
    PathSize2 = GetDevicePathSize( Path2 );

    //Only one copy of the End Device Path.
    if ( PathSize1 && PathSize2 ) {
        PathSize1 -= sizeof (EFI_DEVICE_PATH_PROTOCOL);
    }

    pBS->AllocatePool( EfiBootServicesData, PathSize1 + PathSize2, (void**)&NewPath );

    pBS->CopyMem( NewPath,                     Path1, PathSize1 );
    pBS->CopyMem( (UINT8*)NewPath + PathSize1, Path2, PathSize2 );

    return NewPath;
}

//**********************************************************************//
//**********************************************************************//
//**                                                                  **//
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **//
//**                                                                  **//
//**                       All Rights Reserved.                       **//
//**                                                                  **//
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **//
//**                                                                  **//
//**                       Phone: (770)-246-8600                      **//
//**                                                                  **//
//**********************************************************************//
//**********************************************************************//
