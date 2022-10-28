
//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
//
// $Header:  $
//
// $Revision:  $
//
// $Date:  $
//
//*****************************************************************************


//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        SbDxe.c
//
// Description: This file contains code for Template Southbridge initialization
//              in the DXE stage
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>


// Module specific Includes
#include <Efi.h>
#include <token.h>
#include <AmiLib.h>
#include <AmiDxeLib.h>
#include <AmiCspLib.h>
#include <PCI.h>
#include <PchAccess.h>
#include <Protocol/TpmMp.h>
#include <CMOSMap.h>
// Consumed Protocols
#include <Protocol/PciIo.h>
#include <Protocol/Cpu.h>
#include <Protocol/PciRootBridgeIo.h>
//#include <Library/UefiLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/SbPolicy.h>

#include <Protocol/PciIo.h>
#include <Protocol/BlockIo.h>
#include <Protocol/PDiskInfo.h>
#include <Protocol/PIDEController.h>
//EIPEIP180881 >>
#include <IndustryStandard/AmiAtaAtapi.h>
#include <Protocol/AmiIdeBus.h>

#include <Protocol/AmiHddSecurity.h>
//EIPEIP180881 <<
// Produced Protocols
#include <Protocol/RealTimeClock.h>
#include <Protocol/WatchdogTimer.h>
#include <Protocol/Legacy8259.h>
#include <Protocol/Timer.h>
#include <Protocol/SbSataProtocol.h>

#include <Protocol/AmiSmbios.h> //EIP150551

#include <Sb.h>
#if defined AMIUSB_SUPPORT && AMIUSB_SUPPORT == 1
#include <Protocol/AmiUsbController.h>
#else
#define AMIUSB_SUPPORT 0
#endif

#if CMOS_MANAGER_SUPPORT == 1
#include <CmosAccess.h>
#include <SspTokens.h>
#endif //#if CMOS_MANAGER_SUPPORT == 1
#include <Protocol\PchExtendedReset.h>    // Xhci workaround for disabling/enabling USB ports. (EIP135854+)

// Build flag adjustments
#ifndef     SMM_SUPPORT
#define     SMM_SUPPORT         0
#endif

//EIP150551 >>
#pragma pack (1)
//TYPE136
typedef struct {
  UINT8     Type;
  UINT8     Length;
  UINT16    Handle;
  UINT16    OemInfo;
  UINT16    Zero;   //terminator
} EFI_MISC_OEM_TYPE_136;
#pragma pack ()
//EIP150551 <<

// Constant definitions
//----------------------------------------------------------------------------
//          Timer Constants
//----------------------------------------------------------------------------
#define SYSTEM_TIMER_IRQ 0

// Cpu I/O space defines
#define TIMER_CTRL              0x43
#define TIMER_0_COUNT           0x40
#define TIMER_1_COUNT           0x41

// Timer Period
#define TIMER_TICK            838 //ns

// default duration is 0xffff ticks
#define DEFAULT_TICK_DURATION ((65535*838+50)/100)
#define MAX_TICK_DURATION DEFAULT_TICK_DURATION

//8259 PIC defines
#define ICW1            0x11    // Slave exists and ICW4 required.
#define ICW3_M          1 << 2  // IRQ 2 connects to slave
#define ICW3_S          2       // IRQ 2 connects to master
#define ICW4            1       // Bit 4 Normal Nested Mode
// Bit 3 Non-buffered Mode
// Bit 2 Unused with non-buffered mode
// Bit 1 Set manual EOI instead of automatic
// Bit 0 8086/8088 mode

#define OCW1_M          0xff    // Master Mask
#define OCW1_S          0xff    // Slave Mask

#define EOI_COMMAND     0x20    // EOI Command

#define INTERRUPTS_TRIGGER_REG    0x4d0   //Trigger for Interrupts (Edge or Level).
#define INTERRUPTS_EDGE_TRIGGER   0       //Set all interrupts at edge level.

//----------------------------------------------------------------------------

//CSP20140123 >>
typedef struct
{
    UINTN                   Address;
    EFI_BOOT_SCRIPT_WIDTH   Width;
    UINT32                  Value;
} BOOT_SCRIPT_PCI_REGISTER_SAVE;

BOOT_SCRIPT_PCI_REGISTER_SAVE gSbPciRegistersSave[] =
{
    0x00, EfiBootScriptWidthUint32, 0,
};

BOOT_SCRIPT_PCI_REGISTER_SAVE gSbMmioRegistersSave[] =
{
    AZALIA_REG(R_PCH_HDA_HDBARL), EfiBootScriptWidthUint32, 0,
    AZALIA_REG(R_PCH_HDA_HDBARU), EfiBootScriptWidthUint32, 0,
//    SPI_BASE_ADDRESS + R_PCH_SPI_SSFCS, EfiBootScriptWidthUint32, 0,
//    SPI_BASE_ADDRESS + R_PCH_SPI_PREOP, EfiBootScriptWidthUint32, 0,
//    SPI_BASE_ADDRESS + R_PCH_SPI_OPMENU0, EfiBootScriptWidthUint32, 0,
//    SPI_BASE_ADDRESS + R_PCH_SPI_OPMENU1, EfiBootScriptWidthUint32, 0,
//    SPI_BASE_ADDRESS + R_PCH_SPI_LVSCC, EfiBootScriptWidthUint32, 0,
//    SPI_BASE_ADDRESS + R_PCH_SPI_UVSCC, EfiBootScriptWidthUint32, 0
};
//CSP20140123 <<

BOOT_SCRIPT_PCI_REGISTER_SAVE gSbMmioRegistersSaveEarly[] =
{
    SPI_BASE_ADDRESS + R_PCH_SPI_SSFCS, EfiBootScriptWidthUint32, 0,
    SPI_BASE_ADDRESS + R_PCH_SPI_PREOP, EfiBootScriptWidthUint32, 0,
    SPI_BASE_ADDRESS + R_PCH_SPI_OPMENU0, EfiBootScriptWidthUint32, 0,
    SPI_BASE_ADDRESS + R_PCH_SPI_OPMENU1, EfiBootScriptWidthUint32, 0,
    SPI_BASE_ADDRESS + R_PCH_SPI_LVSCC, EfiBootScriptWidthUint32, 0,
    SPI_BASE_ADDRESS + R_PCH_SPI_UVSCC, EfiBootScriptWidthUint32, 0
};

//----------------------------------------------------------------------------
//          Variable Declaration
//----------------------------------------------------------------------------
EFI_HANDLE  mTimerProtocolHandle        = NULL;
EFI_HANDLE  mWatchdogHandle             = NULL;
EFI_EVENT   mWatchdogEvent;
EFI_EVENT   gEvtBiosSecurity = NULL;
EFI_EVENT   gEvtUsbProtocol = NULL;

// GUID Definitions


//This the number of days in a month - 1. (0 Based)
UINT8       gDaylight                   = 0;                    //Save daylight when set.
UINT8       gMasterBase,
            gSlaveBase;
UINT8       gMode                       = 1;                    //Initially in protected mode. (0 = Real, 1 = 32 bit protected)
UINT16      gIrqMask[2]                 = {0xffff, 0xffff};     //Initially all Real IRQs masked, protected masked
UINT16      gIrqTrigger[2]              = {0, 0};               //Initially all Real IRQs Edge, protected Edge.
UINT64      mWatchdogPeriod             = 0;
UINT64      mProgrammedTimerValue;


EFI_RUNTIME_SERVICES        *gRT;
EFI_BOOT_SERVICES           *gBS;
EFI_TIMER_NOTIFY            mNotifyFunction;
EFI_LEGACY_8259_PROTOCOL    *mLegacy8259;
EFI_WATCHDOG_TIMER_NOTIFY   mWatchdogNotifyFunction = NULL;
AMI_S3_SAVE_PROTOCOL        *gBootScript = NULL;
VOID                        *gRegBiosSecurity = NULL;
VOID                        *gRegUsbProtocol = NULL;
AMI_S3_SAVE_PROTOCOL        *gBootScriptSave;

#if AMIUSB_SUPPORT
EFI_USB_PROTOCOL            *gUsbProtocol = NULL;
#endif

#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0)
extern UINT8 gBspLocalApicID;
#endif

extern EFI_GUID gAmiTseEventBeforeBootGuid; //EIP127537 
extern EFI_GUID gAmiLegacyBootProtocolGuid; //EIP138173 
extern EFI_GUID gAmiExtPciBusProtocolGuid;  //EIP142372
extern EFI_GUID gAmiSmbiosProtocolGuid; //EIP150551
//----------------------------------------------------------------------------

// Function Prototypes
//(CSP20130606B+)(EIP125722)>>
VOID SbSpiProgramVscc (
    VOID
);
//(CSP20130606B+)(EIP125722)<<

EFI_STATUS SbDxeBoardInit(
    IN EFI_HANDLE                 ImageHandle,
    IN EFI_SYSTEM_TABLE           *SystemTable
);

EFI_STATUS CountTime(
    IN  UINTN   DelayTime,
    IN  UINT16  BaseAddr
);

EFI_STATUS WatchdogInit(
    IN EFI_HANDLE                 ImageHandle,
    IN EFI_SYSTEM_TABLE           *SystemTable
);

EFI_STATUS Initialize8259(
    IN EFI_HANDLE                 ImageHandle,
    IN EFI_SYSTEM_TABLE           *SystemTable
);

VOID EFIAPI WatchdogHandler(
    IN EFI_EVENT                  Event,
    IN VOID                       *Context
);

EFI_STATUS RegisterHandler(
    IN EFI_WATCHDOG_TIMER_ARCH_PROTOCOL  *This,
    IN EFI_WATCHDOG_TIMER_NOTIFY         NotifyFunction
);

EFI_STATUS WatchdogSetTimerPeriod(
    IN EFI_WATCHDOG_TIMER_ARCH_PROTOCOL  *This,
    IN UINT64                            TimerPeriod
);

EFI_STATUS WatchdogGetTimerPeriod(
    IN EFI_WATCHDOG_TIMER_ARCH_PROTOCOL  *This,
    IN UINT64                            *TimerPeriod
);

VOID
EFIAPI
TimerInit(
    IN EFI_EVENT          Event,
    IN VOID               *Context
);

EFI_STATUS TimerRegisterHandler(
    EFI_TIMER_ARCH_PROTOCOL     *This,
    EFI_TIMER_NOTIFY            NotifyFunction
);

EFI_STATUS SetTimerPeriod(
    EFI_TIMER_ARCH_PROTOCOL     *This,
    UINT64                      TimerPeriod
);

EFI_STATUS GetTimerPeriod(
    EFI_TIMER_ARCH_PROTOCOL     *This,
    UINT64                      *TimerPeriod
);

EFI_STATUS GenerateSoftIntr(
    EFI_TIMER_ARCH_PROTOCOL     *This
);

EFI_STATUS InstallSbPolicyProtocol(VOID);

VOID InitSbRegsBeforeBoot(
    IN EFI_EVENT        Event,
    IN VOID             *Context
);

VOID
ReadyToBootFunction(
    EFI_EVENT  Event,
    VOID       *Context
);

#if BIOS_LOCK_ENABLE
VOID SbCallbackBiosSecurity(
    IN EFI_EVENT        Event,
    IN VOID             *Context
);
#endif

#if AMIUSB_SUPPORT
VOID SbUsbProtocolCallback(
    IN EFI_EVENT        Event,
    IN VOID             *Context
);
#endif

//EIP127537 >> 
#if (EOP_USB_PER_PORT_CTRL == 2)
VOID UsbPerPortDisableCallback (
    IN EFI_EVENT      Event,
    IN VOID           *Context
);

EFI_STATUS SbUsbPortsControlHook (
//EIP160754 >>
    IN  UINT32        UsbPortsCtrlValue,
    IN  UINT8         UsbXhciMode
//EIP160754 <<	
);
#endif
//EIP127537 << 

//EIP142372 >>
VOID SmbusReadyCallback (
    IN EFI_EVENT      Event,
    IN VOID           *Context
);

VOID SmbusBootScriptSave();
//EIP142372 <<

//CSP20140123 >>
//EIP142393 >>
VOID SbPchCallbackReadyToBoot (
	    IN EFI_EVENT      Event,
	    IN VOID           *Context
);
//EIP142393 <<
//CSP20140123 <<

//EIP158981 >>
#if CRID_SUPPORT
//EIP150551 >>
VOID UpdateSmbios136Table(
    IN EFI_EVENT    Event,
    IN VOID         *Context
);
//EIP150551 <<
#endif
//EIP158981 <<

//EIP138173 >>
#if DISABLE_USB_CONTROLLER_WHEN_DISBALE_ALL_PORT
VOID DisableUsbController (
	SB_SETUP_DATA  *PchPolicyData
);
#endif
//EIP138173 <<

// Architectural Protocol Definitions
EFI_WATCHDOG_TIMER_ARCH_PROTOCOL mWatchdog = {
    RegisterHandler,
    WatchdogSetTimerPeriod,
    WatchdogGetTimerPeriod
};

EFI_TIMER_ARCH_PROTOCOL mTimerProtocol = {
    TimerRegisterHandler,
    SetTimerPeriod,
    GetTimerPeriod,
    GenerateSoftIntr
};


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   SbInitBootScriptSave
//
// Description: Callback on AMI CPU INFO protocol. Then call to init strings.
//
// Input:
//  IN EFI_EVENT Event - Not used
//  IN VOID *Context - Not Used
//
// Output:  VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID SbInitBootScriptSave (
    IN EFI_EVENT    Event,
    IN VOID         *Context
    )
{
    EFI_STATUS                      Status;
    SB_SETUP_DATA                   PchPolicyData;
    UINT32                          Buffer32;
    UINT32                          Index;


    Status = pBS->LocateProtocol (
                    &gEfiS3SaveStateProtocolGuid,
                    NULL,
                    &gBootScriptSave
                    );
    ASSERT_EFI_ERROR (Status);

    // Get the value of the SB Setup data.
    GetSbSetupData ((VOID*)gRT, &PchPolicyData, FALSE);

    // Program Last State
    Buffer32 = READ_MEM32 (PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1) & ~BIT00;    // PMC_BASE_ADDRESS offset 0x20
    if (PchPolicyData.LastState == 0) Buffer32 |= 1;
    WRITE_MEM32_S3(gBootScriptSave, PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1, Buffer32); 
    
    //(CSP20130606B+)(EIP125722)>>
    #if ENABLE_OVERRIDE_VSCC_SUPPORT
        Buffer32 = READ_MEM32 (SPI_BASE_ADDRESS + LOWER_VSCC_REG);
        BOOT_SCRIPT_S3_MEM_WRITE_MACRO (
            gBootScriptSave, \
            EfiBootScriptWidthUint32, \
            SPI_BASE_ADDRESS + LOWER_VSCC_REG, \
            1, \
            &Buffer32 \
            );

        Buffer32 = READ_MEM32 (SPI_BASE_ADDRESS + UPPER_VSCC_REG);
        BOOT_SCRIPT_S3_MEM_WRITE_MACRO (
            gBootScriptSave, \
            EfiBootScriptWidthUint32, \
            SPI_BASE_ADDRESS + UPPER_VSCC_REG, \
            1, \
            &Buffer32 \
            );
    #endif
    //(CSP20130606B+)(EIP125722)<<
    
    for (Index=0; Index < sizeof(gSbMmioRegistersSaveEarly) / sizeof(BOOT_SCRIPT_PCI_REGISTER_SAVE); Index++)
    {
            switch (gSbMmioRegistersSaveEarly[Index].Width)
            {
                case (EfiBootScriptWidthUint32):
                  gSbMmioRegistersSaveEarly[Index].Value = MmioRead32(gSbMmioRegistersSaveEarly[Index].Address);
                  break;
                case (EfiBootScriptWidthUint16):
                  gSbMmioRegistersSaveEarly[Index].Value = MmioRead16(gSbMmioRegistersSaveEarly[Index].Address);
                  break;
                case (EfiBootScriptWidthUint8):
                  gSbMmioRegistersSaveEarly[Index].Value = MmioRead8(gSbMmioRegistersSaveEarly[Index].Address);
                  break;
                default:
                  gSbMmioRegistersSaveEarly[Index].Value = MmioRead32(gSbMmioRegistersSaveEarly[Index].Address);
                  break;
            }
            BOOT_SCRIPT_S3_MEM_WRITE_MACRO(
                gBootScriptSave,
                gSbMmioRegistersSaveEarly[Index].Width,
                gSbMmioRegistersSaveEarly[Index].Address,
                1,
                &gSbMmioRegistersSaveEarly[Index].Value
            );
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: SbDxeInit
//
// Description: This function is the entry point for this DXE. This function
//              initializes the chipset SB
//
// Input: ImageHandle Image handle
//        SystemTable Pointer to the system table
//
// Output: Return Status based on errors that occurred while waiting for
//         time to expire.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SbDxeInit(
    IN EFI_HANDLE                 ImageHandle,
    IN EFI_SYSTEM_TABLE           *SystemTable
)
{
    EFI_STATUS                      Status;
    UINT64                          BaseAddress;
    UINT64                          Length;
    EFI_GCD_MEMORY_SPACE_DESCRIPTOR LpcMemorySpaceDescriptor;
    UINT64                          Attributes;
    EFI_EVENT                       SbInitBootScriptSaveEvt;
    VOID                            *SbInitBootScriptSaveNotifyReg;
//EIP127537 >> 
#if (EOP_USB_PER_PORT_CTRL == 2)
    EFI_EVENT                       Event;
    VOID                            *Registration = NULL;
#endif
//EIP127537 << 
    EFI_EVENT                       SmbusEvent;  //EIP142372
    VOID                            *SmbusRegistration = NULL; //EIP142372
    EFI_EVENT                     ReadyToBootEvent; //(EIP131491+)

//EIP158981 >>
#if CRID_SUPPORT
//EIP150551 >>    
    EFI_EVENT						UpdateSmbios136TableEvent;
//EIP150551 <<
#endif
//EIP158981 <<
    
    InitAmiLib(ImageHandle, SystemTable);

    // ReserveSbResources
    BaseAddress = (EFI_PHYSICAL_ADDRESS) FLASH_BASE_ADDRESS;
    Length  = FLASH_SIZE;

    Status  = gDS->GetMemorySpaceDescriptor (BaseAddress, &LpcMemorySpaceDescriptor);
    ASSERT_EFI_ERROR (Status);

    Attributes = LpcMemorySpaceDescriptor.Attributes | EFI_MEMORY_RUNTIME;

    Status = gDS->SetMemorySpaceAttributes (
                    BaseAddress,
                    Length,
                    Attributes
                    );
    ASSERT_EFI_ERROR (Status);

	//EIP20140403_22 >>
    BaseAddress = (EFI_PHYSICAL_ADDRESS)(ILB_BASE_ADDRESS);

    Status  = gDS->GetMemorySpaceDescriptor(BaseAddress, &LpcMemorySpaceDescriptor);
    ASSERT_EFI_ERROR(Status);
    
    Length  = LpcMemorySpaceDescriptor.Length;

    Attributes = LpcMemorySpaceDescriptor.Attributes | EFI_MEMORY_RUNTIME;

    Status = gDS->SetMemorySpaceAttributes(
                 BaseAddress,
                 Length,
                 Attributes
             );
    ASSERT_EFI_ERROR(Status);
	//EIP20140403_22 <<

    // Report Progress code
    PROGRESS_CODE (DXE_SB_INIT);

    // Install 8259 services
    Initialize8259(ImageHandle, SystemTable);

    // Install watchdog timer services
    WatchdogInit(ImageHandle, SystemTable);

    // Create a ReadyToBoot Event
	//(EIP131491+)
    Status = CreateReadyToBootEvent (
                                      TPL_NOTIFY,
                                      InitSbRegsBeforeBoot,
                                      NULL,
                                      &ReadyToBootEvent
                                      );
	//(EIP131491+)


    Status = RegisterProtocolCallback (
              &gEfiS3SaveStateProtocolGuid,
              SbInitBootScriptSave,
              NULL,
              &SbInitBootScriptSaveEvt,
              &SbInitBootScriptSaveNotifyReg
              );

	//CSP20140123 >>
    Status = CreateReadyToBootEvent (
                                      TPL_NOTIFY,
                                      SbPchCallbackReadyToBoot,
                                      NULL,
                                      &ReadyToBootEvent
                                      );
	//CSP20140123 <<
    
    //(CSP20130606B+)(EIP125722)>>
    #if ENABLE_OVERRIDE_VSCC_SUPPORT    
        //
        // Program SPI VSCC
        //
        SbSpiProgramVscc ();
    #endif //ENABLE_OVERRIDE_VSCC_SUPPORT
    //(CSP20130606B+)(EIP125722)<<

//EIP127537 >> 
    #if (EOP_USB_PER_PORT_CTRL == 2)
    Status = RegisterProtocolCallback(
              &gAmiTseEventBeforeBootGuid,
              UsbPerPortDisableCallback,
              NULL,
              &Event,
              &Registration
              );
	#if (CSM_SUPPORT == 1)
    Status = RegisterProtocolCallback(
              &gAmiLegacyBootProtocolGuid,
              UsbPerPortDisableCallback,
              NULL,
              &Event,
              &Registration
              );    
	#endif //CSM_SUPPORT
	#endif //EOP_USB_PER_PORT_CTRL
    
//EIP127537 << 
    
//EIP158981 >>	
#if CRID_SUPPORT    
    //EIP150551 >>
    Status = CreateReadyToBootEvent (
              TPL_NOTIFY,
              UpdateSmbios136Table,
              NULL,
              &UpdateSmbios136TableEvent
              );
    //EIP150551 <<
#endif
//EIP158981 <<
    
	//EIP142372 >>
    Status = RegisterProtocolCallback(
              &gEfiS3SaveStateProtocolGuid,
              SmbusReadyCallback,
              NULL,
              &SmbusEvent,
              &SmbusRegistration
              );    

    Status = RegisterProtocolCallback(
              &gAmiExtPciBusProtocolGuid,
              SmbusReadyCallback,
              NULL,
              &SmbusEvent,
              &SmbusRegistration
              );  
	//EIP142372 <<  

    return Status;
}
//(EIP131491+)>>
typedef struct {
    UINT64                      Address;
    EFI_BOOT_SCRIPT_WIDTH       Width;
} BOOT_SCRIPT_SB_PCI_REG_SAVE;

BOOT_SCRIPT_SB_PCI_REG_SAVE gSata1RegistersSave[] = {
  SATA_REG(SATA_REG_MAP)       , EfiBootScriptWidthUint8, // 0x90
  SATA_REG(SATA_REG_PCIPI)     , EfiBootScriptWidthUint8, // 0x09
  SATA_REG(SATA_REG_INTR_LN)   , EfiBootScriptWidthUint8, // 0x3c
  SATA_REG(SATA_REG_IDETIM)    , EfiBootScriptWidthUint32, // 0x40
  SATA_REG(SATA_REG_SIDETIM)   , EfiBootScriptWidthUint8, // 0x44
  SATA_REG(SATA_REG_SDMACTL)   , EfiBootScriptWidthUint8, // 0x48
  SATA_REG(SATA_REG_SDMATIM)   , EfiBootScriptWidthUint16, // 0x4a
  SATA_REG(SATA_REG_IDE_CONFIG), EfiBootScriptWidthUint32, // 0x54
  SATA_REG(SATA_REG_PMCS)      , EfiBootScriptWidthUint16, // 0x74
  SATA_REG(SATA_REG_PCS)       , EfiBootScriptWidthUint16, // 0x92
  SATA_REG(SATA_REG_PCMD_BAR)  , EfiBootScriptWidthUint32, // 0x10
  SATA_REG(SATA_REG_PCNL_BAR)  , EfiBootScriptWidthUint32, // 0x14
  SATA_REG(SATA_REG_SCMD_BAR)  , EfiBootScriptWidthUint32, // 0x18
  SATA_REG(SATA_REG_SCNL_BAR)  , EfiBootScriptWidthUint32, // 0x1c
  SATA_REG(SATA_REG_BM_BASE)   , EfiBootScriptWidthUint32, // 0x20
  SATA_REG(SATA_REG_ABAR)      , EfiBootScriptWidthUint32, // 0x24
  SATA_REG(SATA_REG_PCICMD)    , EfiBootScriptWidthUint8, // 0x04
};
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   InitSbRegsBeforeBoot
//
// Description: This function can initialize any SB registers before DXE
//              stage exiting.
//
// Input:       Event   - Event of callback
//              Context - Context of callback.
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID InitSbRegsBeforeBoot(
    IN EFI_EVENT                  Event,
    IN VOID                       *Context
)
{
    UINT32                            Data32;
    UINT32                            PortImplemented = 0x0f;
    UINT32                            GlobalPchControl;
    UINTN                             AHCIBar = 0;
    UINT16                            Offset;
    UINT32                            i;
    UINT8			                  SataMode;
    UINT16			                  PortControlStatus;  //EIP152970 
// Xhci workaround for disabling/enabling USB ports. (EIP135854+)>>
#if (EOP_USB_PER_PORT_CTRL == 0)
    UINT32                            Index;
    UINT32                            XhciUsb2Pdo;
    UINTN                             XhciPciMmBase;
    EFI_PCH_EXTENDED_RESET_PROTOCOL   *gExtendedReset;
    SB_SETUP_DATA                     PchPolicyData;
    PCH_EXTENDED_RESET_TYPES          ResetType;
#endif
// Xhci workaround for disabling/enabling USB ports. (EIP135854+)<<
	EFI_STATUS              		  Status = EFI_SUCCESS;
	UINT16							  PasswordSecurity = 0;
	UINT16					  		  SecurityStatus = 0;
    EFI_HANDLE                        *HandleBuffer = NULL;
    AMI_HDD_SECURITY_PROTOCOL         *IDEPasswordSecurity = NULL;    //EIPEIP180881

    UINTN 							  Count = 0;
    //EIP156971 (-)>>
    //EIP146629 >> 
//#if CF9_REG_LOCK_ENABLE
//    UINT32                            PmcBase;
//    UINT32                            Cf9Lock;
//#endif
	//EIP146629 <<   
    //EIP156971 (-)<<
    // [ EIP355841 ]+>>    
    UINT8                             PciCmdValue;
    UINT8                             PciBarIndex;
    BOOLEAN                           PciBarXinit;
    // [ EIP355841 ]+<<


    TRACE((TRACE_ALWAYS, "[[ InitSbRegsBeforeBoot() Start. ]]\n"));
    
// Xhci workaround for disabling/enabling USB ports. (EIP135854+)>>
#if (EOP_USB_PER_PORT_CTRL == 0)
    // Get the value of the SB Setup data.
    GetSbSetupData ((VOID*)gRT, &PchPolicyData, FALSE);
    
    // Checking Xhci Ports
    if(PchPolicyData.PchUsb30Mode == 1) {
      XhciPciMmBase   = CSP_PCIE_CFG_ADDRESS (  DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_XHCI, PCI_FUNCTION_NUMBER_PCH_XHCI, 0);
      XhciUsb2Pdo = MmioRead32 (XhciPciMmBase + R_PCH_XHCI_USB2PDO) & B_PCH_XHCI_USB2PDO_MASK;
      for (Index = 0; Index < PCH_USB_MAX_PHYSICAL_PORTS; Index++) {
        if ((PchPolicyData.PchUsbPort[Index] == 0 && !(XhciUsb2Pdo & (BIT0 << Index))) || 
            (PchPolicyData.PchUsbPort[Index] == 1 && (XhciUsb2Pdo & (BIT0 << Index)))) {
          ResetType.GlobalReset = 1;
          gBS->LocateProtocol(&gEfiPchExtendedResetProtocolGuid, NULL, &gExtendedReset);
          gExtendedReset->Reset(gExtendedReset, ResetType);
        }
      }
    }
//EIP138173 >>
#if DISABLE_USB_CONTROLLER_WHEN_DISBALE_ALL_PORT 
    DisableUsbController(&PchPolicyData);
#endif
//EIP138173 <<
    
#endif
// Xhci workaround for disabling/enabling USB ports. (EIP135854+)<<
    
      //EIP156971 (-)>>
      //EIP148236 >>
	  //EIP146629 >> 	
//  #if CF9_REG_LOCK_ENABLE
//      PmcBase   = MmioRead32(SB_REG(R_PCH_LPC_PMC_BASE)) & B_PCH_LPC_PMC_BASE_BAR;
//      MmioOr32 ((UINTN) (PmcBase + R_PCH_PMC_PMIR), B_PCH_PMC_PMIR_CF9LOCK);
//      Cf9Lock = READ_MEM32 (PmcBase + R_PCH_PMC_PMIR);
//      BOOT_SCRIPT_S3_MEM_WRITE_MACRO (
//            gBootScriptSave, \
//            EfiBootScriptWidthUint32, \
//            PmcBase + R_PCH_PMC_PMIR, \
//            1, \
//            &Cf9Lock \
//            );
//  #endif
    //EIP156971 (-)<<    
	  //EIP146629 <<
      //EIP148236 <<
    
      //Save Sata and ABAR S3 reg.
      Data32 = READ_MEM32 (SATA_REG(PCI_VID));
// [ EIP355841 ]+>>
      if ((Data32 & 0x0000FFFF) == 0x8086)          
      {
// [ EIP355841 ]+<<<      
          SataMode = READ_MEM8(SATA_REG(PCI_SCC));
//(EIP152970+)>>
          PortControlStatus = READ_MEM16 (SATA_REG(SATA_REG_PCS)); //Get Port Control Status. Offset 0x92.   
      
          if ((PortControlStatus & 0x0300) != 0) { //Check Sata Port1 & Port0 Present Status.
//      if (Data32 != 0xffffffff) {
//(EIP152970+)<<

// [ EIP355841 ]-
//        WRITE_MEM8 (SATA_REG(SATA_REG_PCICMD),0x07); //Force open the MEM & I/O decode for S3 Boot Script to save.
// [ EIP355841 ]+>>
              PciCmdValue = READ_MEM8 (SATA_REG(SATA_REG_PCICMD));
              for (PciBarIndex=PCI_BAR0, PciBarXinit=FALSE; PciBarIndex <= PCI_BAR5; PciBarIndex+=4)
              {
                  // BIT[0] 0 indicate memory space.
                  // BIT[2:1] 10 indicate 64 bit base address
                  if ((READ_MEM32(SATA_REG(PciBarIndex)) & (BIT2+BIT1+BIT0)) == BIT2)
                  {
                      TRACE((TRACE_ALWAYS, "PciBar(64bit)"));
                      if ((MmioRead64(SATA_REG(PciBarIndex)) & 0xFFFFFFFFFFFFFFF0) != 0)
                      {
                          PciBarXinit = TRUE;
                          break;
                      }
                      else
                          PciBarIndex+=4;
                    
                  }
                  else if ((READ_MEM32(SATA_REG(PciBarIndex)) & 0xFFFFFFF0) != 0)
                  {
                      PciBarXinit = TRUE;
                      break;
                  }
              }
        

              if ((SataMode == V_PCH_SATA_CC_SCC_AHCI) && PciBarXinit)
              {
                  WRITE_MEM8 (SATA_REG(SATA_REG_PCICMD), B_PCH_SATA_COMMAND_BME+B_PCH_SATA_COMMAND_MSE+B_PCH_SATA_COMMAND_IOSE);  //Force open the MEM & I/O decode for S3 Boot Script to save.
              }
              else if ((SataMode == V_PCH_SATA_CC_SCC_IDE) && PciBarXinit)
              {
                  WRITE_MEM8 (SATA_REG(SATA_REG_PCICMD), B_PCH_SATA_COMMAND_BME+B_PCH_SATA_COMMAND_IOSE);                         //Force open the MEM & I/O decode for S3 Boot Script to save.
              }    
// [ EIP355841 ]+<<

              for (i = 0; i < sizeof(gSata1RegistersSave)/ sizeof(BOOT_SCRIPT_SB_PCI_REG_SAVE); ++i) {
        		BOOT_SCRIPT_S3_MEM_WRITE_MACRO (
        				gBootScriptSave, \
        				gSata1RegistersSave[i].Width, \
        				gSata1RegistersSave[i].Address, \
        				1, \
        				(VOID *) (gSata1RegistersSave[i].Address) \
        				);
              }

              if (SataMode == V_PCH_SATA_CC_SCC_AHCI) {                       // [ EIP347466 ]+
            
                  AHCIBar = READ_MEM32 (SATA_REG(SATA_REG_ABAR));
                  AHCIBar &= 0xFFFFFFF0;
        
                  GlobalPchControl = READ_MEM32 (AHCIBar + 0x04);
                      BOOT_SCRIPT_S3_MEM_WRITE_MACRO (
                              gBootScriptSave, \
                              EfiBootScriptWidthUint32, \
                              AHCIBar + 0x04, \
                              1, \
                              &GlobalPchControl \
                  );
        
                  PortImplemented = READ_MEM32 (AHCIBar + 0x0c);
                  BOOT_SCRIPT_S3_MEM_WRITE_MACRO (
                    gBootScriptSave, \
                    EfiBootScriptWidthUint32, \
                    AHCIBar + 0x0c, \
                    1, \
                    &PortImplemented \
                  );
	
                  for (i = 0, Offset = 0x100; i < 2 ; i++, Offset += 0x80) {
                      if ( PortImplemented & (BIT00 << i) ) {
                          Data32 = READ_MEM32 (AHCIBar + Offset);
                          BOOT_SCRIPT_S3_MEM_WRITE_MACRO (
                            gBootScriptSave, \
                            EfiBootScriptWidthUint32, \
                            AHCIBar + Offset, \
                            1, \
                            &Data32 \
                          );
       	    
                          Data32 = READ_MEM32 (AHCIBar + Offset + 0x04);
                          BOOT_SCRIPT_S3_MEM_WRITE_MACRO (
                            gBootScriptSave, \
                            EfiBootScriptWidthUint32, \
                            AHCIBar + Offset + 0x04, \
                            1, \
                            &Data32 \
                          );
		
                          Data32 = READ_MEM32 (AHCIBar + Offset + 0x08);
                          BOOT_SCRIPT_S3_MEM_WRITE_MACRO (
                            gBootScriptSave, \
                            EfiBootScriptWidthUint32, \
                            AHCIBar + Offset + 0x08, \
                            1, \
                            &Data32 \
                          );
	    
                          Data32 = READ_MEM32 (AHCIBar + Offset + 0x0c);
                          BOOT_SCRIPT_S3_MEM_WRITE_MACRO (
                            gBootScriptSave, \
                            EfiBootScriptWidthUint32, \
                            AHCIBar + Offset + 0x0c, \
                            1, \
                            &Data32 \
                          );

                          Data32 = READ_MEM32 (AHCIBar + Offset + 0x18);
                          Data32 &= 0xFFFFFFEE; //Make sure Clear the Start and FIS Receive Enable bit
                          BOOT_SCRIPT_S3_MEM_WRITE_MACRO (
                            gBootScriptSave, \
                            EfiBootScriptWidthUint32, \
                            AHCIBar + Offset + 0x18, \
                            1, \
                            &Data32 \
                          );
		
                          Data32 = READ_MEM32 (AHCIBar + Offset + 0x2c);
                          BOOT_SCRIPT_S3_MEM_WRITE_MACRO (
                            gBootScriptSave, \
                            EfiBootScriptWidthUint32, \
                            AHCIBar + Offset + 0x2c, \
                            1, \
                            &Data32 \
                          );
                      }
                  }

                  Data32 = READ_MEM32 (AHCIBar);
                  BOOT_SCRIPT_S3_MEM_WRITE_MACRO (
                          gBootScriptSave, \
                          EfiBootScriptWidthUint32, \
                          AHCIBar, \
                          1, \
                          &Data32 \
                  );
              }                                                               // [ EIP347466 ]+
              else if(SataMode == V_PCH_SATA_CC_SCC_IDE)                      //SSD IDE mode need delay time when S3 resume to unlock HDD password. temporary solution.
              {
//EIPEIP180881 >>
                  Status = pBS->LocateHandleBuffer(ByProtocol,
                                             &gAmiHddSecurityProtocolGuid,
                                             NULL,
                                             &Count,
                                             &HandleBuffer);
                  if(!EFI_ERROR(Status))
                  {
                      //
                      // Get the PasswordSecurity Protocol
                      //
                      for ( i = 0; i < Count; i++ ) 
                      {
                          Status = pBS->OpenProtocol(HandleBuffer[i],
                                               &gAmiHddSecurityProtocolGuid,
                                               (VOID**) &IDEPasswordSecurity,
                                               NULL,
                                               HandleBuffer[i],
                                               EFI_OPEN_PROTOCOL_GET_PROTOCOL);
//EIPEIP180881 <<
                          if(!EFI_ERROR(Status))
                          {
                              IDEPasswordSecurity->ReturnSecurityStatus(IDEPasswordSecurity, &SecurityStatus);
                              PasswordSecurity |= SecurityStatus;
                          }
                      }
            	    
                      if(PasswordSecurity & BIT1) // BIT1 : Security Enabled
                          BOOT_SCRIPT_S3_STALL_MACRO(gBootScriptSave, 1000*1000); 
                  }
              }
          } //if ((PortControlStatus & 0x0300) != 0)
      
//[ EIP355841 ]+>>>
          WRITE_MEM8 (SATA_REG(SATA_REG_PCICMD), PciCmdValue);
      } //if ((Data32 & 0x0000FFFF) == 0x8086)
//[ EIP355841 ]+<<<
   
      TRACE((TRACE_ALWAYS, "[[ InitSbRegsBeforeBoot() End. ]]\n"));
    pBS->CloseEvent(Event);
}
//(EIP131491+)<<
#if BIOS_LOCK_ENABLE
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: SbCallbackBiosSecurity
//
// Description: SB call back function before legacy boot.
//
// Input:  Event   - Event of callback
//         Context - Context of callback
//
// Output: None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SbCallbackBiosSecurity(
    IN EFI_EVENT            Event,
    IN VOID                 *Context
)
{
    /**** PORTING REQUIRED ****
        UINT8   Buffer8 = 0;
        UINT32  Buffer32 = 0;                   // (P050509E)
        UINT32  D31F0Addr = (UINT32)(UINTN)SB_PCIE_CFG_ADDRESS(LPC_BUS, LPC_DEVICE, LPC_FUNC, ICH_REG_LPC_VID); // (P122208D)

        TRACE((TRACE_ALWAYS, "[[ SbPchCallbackBiosSecurity() Start. ]]\n"));
        // Enable BIOS Lock & SMM_BWP
        Buffer8 = MmPci8(LPC_BUS, LPC_DEVICE, LPC_FUNC, ICH_REG_LPC_BIOS_CNTL) |= (BIT01+BIT05);
        BOOT_SCRIPT_S3_MEM_WRITE_MACRO(gBootScriptSave,
                                       EfiBootScriptWidthUint8,
                                       (D31F0Addr + ICH_REG_LPC_BIOS_CNTL),
                                       1,
                                       &Buffer8);

        // Enable BIOS Interface Lock-Down
        Buffer32 = Mmio32(SB_RCBA, ICH_RCRB_GCS);  // SB_RCBA + 0x3410
        Buffer32 |= (BIT00);
        Mmio32(SB_RCBA, ICH_RCRB_GCS) = Buffer32;
        BOOT_SCRIPT_S3_MEM_WRITE_MACRO(gBootScriptSave,
                                       EfiBootScriptWidthUint32,
                                       (SB_RCBA + ICH_RCRB_GCS),
                                       1,
                                       &Buffer32);
                                            // <(P111009A)
        TRACE((TRACE_ALWAYS, "[[ SbPchCallbackBiosSecurity() Done. ]]\n"));
    ****/
    pBS->CloseEvent(Event);
}
#endif

#if AMIUSB_SUPPORT
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SbUsbProtocolCallback
//
// Description: This callback function is called after USB Protocol is
//              installed.
//
// Input:       Event   - Event of callback
//              Context - Context of callback.
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID SbUsbProtocolCallback(
    IN EFI_EVENT                  Event,
    IN VOID                       *Context
)
{
    EFI_STATUS              Status = EFI_SUCCESS;
    /**** PORTING REQUIRED ****
        // This example is skipped all USB mass storage initialization
        // If all USB controllers are disabled
        USB_SKIP_LIST SkipMassTable[] = { {1, 0, 0xff, 0, 0, 0x8},
                                          {0, 0, 0, 0, 0, 0}};

        if (gSetupData->Usb11 == 0) {   // If all USB controllers are disabled
            Status = pBS->LocateProtocol( &gEfiUsbProtocolGuid,
                                          NULL,
                                          &gUsbProtocol );

            gUsbProtocol->UsbCopySkipTable(SkipMassTable, sizeof(SkipMassTable)/sizeof (USB_SKIP_LIST));
        }
    ****/

    pBS->CloseEvent(Event);
}
#endif

//----------------------------------------------------------------------------
//   USUALLY NO PORTING REQUIRED FOR THE FOLLOWING ROUTINES
//----------------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   WatchdogHandler
//
// Description: This function is called when the watchdog timer event is
//              signaled.  It calls the registered handler and then
//              resets the system
//
// Input:       Event   - Watchdog event
//              Context - Context pointer
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID EFIAPI WatchdogHandler(
    IN EFI_EVENT                  Event,
    IN VOID                       *Context
)
{
    //
    // Call the registered handler if there is one
    //
    if(mWatchdogNotifyFunction != NULL) {
        mWatchdogNotifyFunction(mWatchdogPeriod);
    }
    //
    // Reset the system
    //
    pRS->ResetSystem(EfiResetCold, EFI_TIMEOUT, 0, NULL);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: RegisterHandler
//
// Description: This function registers a handler that is called when the Timer
//              event has been signalled
//
// Input:  *This   Pointer to the instance of the Architectural Protocol
//         NotifyFunction The function to call when the interrupt fires
//
// Output:  EFI_STATUS
//          EFI_SUCCESS   When new handle is registered
//          EFI_ALREADY_STARTEd If notify function is already defined
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI RegisterHandler(
    IN EFI_WATCHDOG_TIMER_ARCH_PROTOCOL *This,
    IN EFI_WATCHDOG_TIMER_NOTIFY        NotifyFunction
)
{
    //
    // only register the handler if it is still NULL
    //
    if(NotifyFunction && mWatchdogNotifyFunction) return EFI_ALREADY_STARTED;
    if(!NotifyFunction && !mWatchdogNotifyFunction) return EFI_INVALID_PARAMETER;

    mWatchdogNotifyFunction = NotifyFunction;

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: WatchdogSetTimerPeriod
//
// Description: This function sets the timer period before the watchdog goes off
//              every TimerPeriod number of 100ns intervals, if the period is set to 0 the
//              timer event is cancelled
//
// Input:  *This  Pointer to the instance of the Architectural Protocol
//         TimerPeriod The number of 100ns intervals to which the watchdog
//         will be programmed.
//
// Output: EFI_SUCCESS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI WatchdogSetTimerPeriod(
    IN EFI_WATCHDOG_TIMER_ARCH_PROTOCOL *This,
    IN UINT64                           TimerPeriod
)
{
    EFI_TIMER_DELAY  TimerDelayType;

    //
    // store new timer length
    //
    mWatchdogPeriod = TimerPeriod;
    //
    // cancel timer event if Timer Period is 0
    //
    TimerDelayType = (TimerPeriod) ? TimerRelative : TimerCancel;
    //
    // set the timer for the event
    //
    return pBS->SetTimer(mWatchdogEvent, TimerDelayType, mWatchdogPeriod);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: WatchdogGetTimerPeriod
//
// Description: This function returns the current watchdog timer period
//
// Input:  *This   Pointer to the instance of the Architectural Protocol
//         *TimerPeriod Pointer to a memory location to load the current Timer
//         period into
//
// Output:  *TimerPeriod Current Timer Period if function returns EFI_SUCCESS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI WatchdogGetTimerPeriod(
    IN EFI_WATCHDOG_TIMER_ARCH_PROTOCOL *This,
    IN UINT64                           *TimerPeriod
)
{
    //
    // return the current Watchdog period
    //
    *TimerPeriod = mWatchdogPeriod;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   WatchdogInit
//
// Description: This function installs the the Watchdog Timer protocol on its
//              handle, and initializes the Watchdog timer.
//
// Input:       ImageHandle - ImageHandle of the loaded driver
//              SystemTable - Pointer to the System Table
//
// Output:      EFI_STATUS
//              EFI_SUCCESS           - The Watchdog Timer protocol was
//                                      installed.
//              EFI_OUT_OF_RESOURCES  - Space for a new handle could not
//                                      be allocated.
//              EFI_INVALID_PARAMETER - One of the parameters has an
//                                      invalid value.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS WatchdogInit(
    IN EFI_HANDLE                 ImageHandle,
    IN EFI_SYSTEM_TABLE           *SystemTable
)
{
    EFI_STATUS  Status;
    EFI_HANDLE  WatchdogHandle = NULL;

    //
    // Use the Timer event to trigger the Watchdog.  No specific hardware
    // exists for this
    //
    pBS->CreateEvent(
                 EVT_TIMER | EVT_NOTIFY_SIGNAL,
                 TPL_NOTIFY,
                 WatchdogHandler,
                 NULL,
                 &mWatchdogEvent
             );
    //
    // Create a handle for the ArchProtocol and install Watchdog Arch
    // Protocol on the handle
    //
    Status = pBS->InstallProtocolInterface(
                 &WatchdogHandle,
                 &gEfiWatchdogTimerArchProtocolGuid,
                 EFI_NATIVE_INTERFACE,
                 &mWatchdog
             );

    return Status;
}

#if 0//defined(HPET_PROTOCOL_SUPPORT) && (HPET_PROTOCOL_SUPPORT != 0) //EIP144604
// Mask used for counter and comparator calculations to adjust for a 32-bit or 64-bit counter.
UINT64  gCounterMask;
// Cached state of the HPET General Capabilities register managed by this driver.
// Caching the state reduces the number of times the configuration register is read.
volatile HPET_GENERAL_CAPABILITIES_ID_REGISTER   gHpetGeneralCapabilities;
// Cached state of the HPET General Configuration register managed by this driver.
// Caching the state reduces the number of times the configuration register is read.
volatile HPET_GENERAL_CONFIGURATION_REGISTER     gHpetGeneralConfiguration;
// Cached state of the Configuration register for the HPET Timer managed by
// this driver.  Caching the state reduces the number of times the configuration
// register is read.
volatile HPET_TIMER_CONFIGURATION_REGISTER       gTimerConfiguration;

EFI_EVENT                               gHpetLegacyBootEvent;

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EnableHpetInChipset
//
// Description: This function enables HPET register decode.
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID EnableHpetInChipset(VOID)
{
    // Porting required.
//    WRITE_MEM32_RCRB_S3(gBootScript, RCRB_MMIO_HPTC, ((HPET_BASE_ADDRESS >> 12) & 3) | 0x80);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   HpetRead
//
// Description: This function reads a 64-bit register from the HPET register.
//
// Input:       Offset - Specifies the offset of the HPET register to read.
//
// Output:      The 64-bit value read from the HPET register specified by
//              Offset.
//
// Notes:       No porting required.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT64 HpetRead(
    IN UINTN        Offset)
{
    return MMIO_READ64(HPET_BASE_ADDRESS + Offset);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   HpetWrite
//
// Description: This function writes a 64-bit HPET register.
//
// Input:       Offset - Specifies the ofsfert of the HPET register to write.
//              Value  - Specifies the value to write to the HPET register
//                       specified by Offset.
//
// Output:      The 64-bit value written to HPET register specified by Offset.
//
// Notes:       No porting required.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT64 HpetWrite(
    IN UINTN        Offset,
    IN UINT64       Value)
{
    MMIO_WRITE64(HPET_BASE_ADDRESS + Offset, Value);
    return HpetRead(Offset);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   HpetEnable
//
// Description: This function enables or disables the main counter in the
//              HPET Timer.
//
// Input:       Enable  TRUE  - Enable the main counter in the HPET Timer.
//                      FALSE - Disable the main counter in the HPET Timer.
// Output:      None
//
// Notes:       No porting required.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID HpetEnable(
    IN BOOLEAN      Enable)
{
    gHpetGeneralConfiguration.Bits.MainCounterEnable = Enable ? 1 : 0;
    HpetWrite(HPET_GENERAL_CONFIGURATION_OFFSET,
              gHpetGeneralConfiguration.Uint64);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   StopHpetBeforeLagecyBoot
//
// Description: This function stops HPET counter & interrupt.
//
// Input:       Event   - Event of callback
//              Context - Context of callback.
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID StopHpetBeforeLagecyBoot(
    IN EFI_EVENT        Event,
    IN VOID             *Context)
{
    // Disable HPET and Legacy Replacement Support.
    HpetEnable(FALSE);
    CountTime((HPET_DEFAULT_TICK_DURATION / 10) * 2, PM_BASE_ADDRESS);
    HpetWrite(HPET_TIMER_CONFIGURATION_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE, 0);

#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0)
    IoApicDisableIrq(HPET_APIC_INTERRUPT_PIN);
#else
    gHpetGeneralConfiguration.Bits.LegacyRouteEnable = 0;

    HpetEnable(FALSE);
#endif

    pBS->CloseEvent(Event);
}
#endif

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   TimerRegisterHandler
//
// Description: This function registers a handler that is called every time the
//              timer interrupt fires
//
// Input:       IN  EFI_TIMER_ARCH_PROTOCOL *This - Pointer to the instance of the
//                                                  Architectural Protocol
//              IN  EFI_TIMER_NOTIFY NotifyFunction - The function to call when the interrupt fires
//
// Output:      EFI_SUCCESS - new handle registered
//              EFI_ALREADY_STARTED - if Notify function is already defined
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS TimerRegisterHandler(
    IN  EFI_TIMER_ARCH_PROTOCOL *This,
    IN  EFI_TIMER_NOTIFY        NotifyFunction
)
{
    // check to see if the handler has already been installed
    if(NotifyFunction != NULL && mNotifyFunction != NULL) {
        return EFI_ALREADY_STARTED;
    }

    // if not install it
    mNotifyFunction = NotifyFunction;
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetTimerPeriod
//
// Description: This function sets the timer to create an Intr on IRQ0
//              every TimerPeriod number of 100ns intervals
//
// Input:       IN EFI_TIMER_ARCH_PROTOCOL *This - Pointer to the instance of the Architectural Protocol
//              IN UINT64 TimerPeriod - The number of 100ns intervals to which the timer
//                                      will be programmed. This value will be rounded up to
//                                      the nearest timer interval
//
// Output:      EFI_SUCCESS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SetTimerPeriod(
    IN EFI_TIMER_ARCH_PROTOCOL      *This,
    IN UINT64                       TimerPeriod
)
{

#if 0//defined(HPET_PROTOCOL_SUPPORT) && (HPET_PROTOCOL_SUPPORT != 0) //EIP144604
    UINTN                   Remainder;
    UINT64                  TimerCount;

    // Disable HPET timer when adjusting the timer period
    HpetEnable(FALSE);
#else
    UINT32                  NumberOfTicks;
    UINT8                   Value8;
#endif

    // If timer period is 0 then disable the Timer interrupt
    if(TimerPeriod == 0) {
#if 0//defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0) //EIP144604
        IoApicDisableIrq(HPET_APIC_INTERRUPT_PIN);
#endif
        mLegacy8259->DisableIrq(mLegacy8259, SYSTEM_TIMER_IRQ);
        //DisableIrq(SYSTEM_TIMER_IRQ);

    } else {
#if 0//defined(HPET_PROTOCOL_SUPPORT) && (HPET_PROTOCOL_SUPPORT != 0) //EIP144604
        // Convert TimerPeriod to femtoseconds and divide by the number if
        // femtoseconds per tick of the HPET counter to determine the number
        // of HPET counter ticks in TimerPeriod 100 ns units.
        TimerCount = Div64(Mul64(TimerPeriod, 100000000),
                           gHpetGeneralCapabilities.Bits.CounterClockPeriod,
                           &Remainder);

        // Reset Main Counter
        HpetWrite(HPET_MAIN_COUNTER_OFFSET, 0);
        // ValueSetEnable must be set if the timer is set to periodic mode.
        gTimerConfiguration.Bits.ValueSetEnable = 1;
        HpetWrite(HPET_TIMER_CONFIGURATION_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE, gTimerConfiguration.Uint64);
        // Clear ValueSetEnable bit.
        gTimerConfiguration.Bits.ValueSetEnable = 0;
        HpetWrite(HPET_TIMER_COMPARATOR_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE, TimerCount);
#else
        if(TimerPeriod > MAX_TICK_DURATION) TimerPeriod = MAX_TICK_DURATION;

        // since TimerPeriod in 100ns units and TIMER_TICK in ns
        // we have to multiple TimerPeriod by 100
        // To round up result:

        NumberOfTicks = ((UINT32)TimerPeriod * 100 + TIMER_TICK / 2)
                        / TIMER_TICK;
        //write to port 0x43 to setup the timer
        IoWrite8(TIMER_CTRL, 0x36);
        // write to port 0x40 to set the time
        IoWrite8(TIMER_0_COUNT, (UINT8)NumberOfTicks);
        IoWrite8(TIMER_0_COUNT, *(((UINT8*)&NumberOfTicks) + 1));
        Value8 = 0x36;
        BOOT_SCRIPT_S3_IO_WRITE_MACRO(gBootScript, EfiBootScriptWidthUint8, TIMER_CTRL, 1, &Value8);
        Value8 = (UINT8)NumberOfTicks;
        BOOT_SCRIPT_S3_IO_WRITE_MACRO(gBootScript, EfiBootScriptWidthUint8, TIMER_0_COUNT, 1, &Value8);
        Value8 = *(((UINT8*) & NumberOfTicks) + 1);
        BOOT_SCRIPT_S3_IO_WRITE_MACRO(gBootScript, EfiBootScriptWidthUint8, TIMER_0_COUNT, 1, &Value8);
#endif
        // Now enable the interrupt
#if 0//defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0) //EIP144604
        IoApicEnableIrq(HPET_APIC_INTERRUPT_PIN, FALSE, (HPET_INTERRUPT_POLARITY == 0) ? TRUE : FALSE);
#endif
        //EnableIrq(SYSTEM_TIMER_IRQ);
        mLegacy8259->EnableIrq(mLegacy8259, SYSTEM_TIMER_IRQ, FALSE);

#if 0//defined(HPET_PROTOCOL_SUPPORT) && (HPET_PROTOCOL_SUPPORT != 0) //EIP144604
        // Enable HPET Interrupt Generation
        gTimerConfiguration.Bits.InterruptEnable = 1;
        HpetWrite(HPET_TIMER_CONFIGURATION_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE, gTimerConfiguration.Uint64);

        // Enable the HPET counter once new timer period has been established
        // The HPET counter should run even if the HPET Timer interrupts are
        // disabled.  This is used to account for time passed while the interrupt
        // is disabled.
        HpetEnable(TRUE);
#endif
    }

    mProgrammedTimerValue = TimerPeriod;

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetTimerPeriod
//
// Description: This function returns the current timer period
//
// Input:       IN EFI_TIMER_ARCH_PROTOCOL *This - Pointer to the instance of the Architectural Protocol
//              IN OUT  UINT64 *TimerPeriod - pointer to a memory location to load the current
//                                            Timer period into
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GetTimerPeriod(
    IN      EFI_TIMER_ARCH_PROTOCOL *This,
    IN OUT  UINT64                  *TimerPeriod
)
{
    *TimerPeriod = mProgrammedTimerValue;

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GenerateSoftIntr
//
// Description: This function generates a soft timer interrupt
//
// Input:       IN EFI_TIMER_ARCH_PROTOCOL *This - Pointer to the instance of the Architectural Protocol
//
// Output:      EFI_UNSUPPORTED
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GenerateSoftIntr(
    IN EFI_TIMER_ARCH_PROTOCOL      *This
)
{
    return EFI_UNSUPPORTED;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   TimerInterruptHandler
//
// Description: This function is called when the Timer reaches 0.  It raises
//              the TPL and then calls the registered notify function
//
// Input:       IN EFI_EXCEPTION_TYPE InterruptType - Interrupt type
//              IN EFI_SYSTEM_CONTEXT SystemContext - System context
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID TimerInterruptHandler(
    IN EFI_EXCEPTION_TYPE   InterruptType,
    IN EFI_SYSTEM_CONTEXT   SystemContext
)
{
    EFI_TPL                 OldTpl;
    static volatile UINT32  StoreCF8;

    SaveRestoreRegisters(TRUE);

    StoreCF8 = IoRead32(0xcf8);    // Store CF8 (PCI index)

    OldTpl = pBS->RaiseTPL(TPL_HIGH_LEVEL);

#if 0//defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0) //EIP144604
    IoApicEoi(GetHpetApicPin());
#if defined(HPET_INTERRUPT_TRIGGER) && (HPET_INTERRUPT_TRIGGER != 0)
    HpetWrite(HPET_GENERAL_INTERRUPT_STATUS_OFFSET, (BIT0 << HPET_OFFSET));
#endif
#else
    // clear the interrupt flag
    //SendEoi(SYSTEM_TIMER_IRQ);
    mLegacy8259->EndOfInterrupt(mLegacy8259, SYSTEM_TIMER_IRQ);
#endif

    // This checks for the existance of a registered notify function and if it exists
    //  it calls the function with the current programmed Timer Period
    if(mNotifyFunction) {
        mNotifyFunction(mProgrammedTimerValue);
    }

    pBS->RestoreTPL(OldTpl);

    IoWrite32(0xcf8, StoreCF8);    // Restore 0xCF8 (PCI index)

    SaveRestoreRegisters(FALSE);
}

#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0)
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   Irq0InterruptHandler
//
// Description: This function is called when the 8254 Timer 0 reaches 0.
//              It raises the TPL and then calls the registered notify
//              function.
//
// Input:       InterruptType - Interrupt type
//              SystemContext - System context
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID Irq0InterruptHandler(
    IN EFI_EXCEPTION_TYPE   InterruptType,
    IN EFI_SYSTEM_CONTEXT   SystemContext)
{
    EFI_TPL                 OldTpl;

    OldTpl = pBS->RaiseTPL(TPL_HIGH_LEVEL);

    // Clear the interrupt flag
    mLegacy8259->EndOfInterrupt(mLegacy8259, SYSTEM_TIMER_IRQ);

    pBS->RestoreTPL(OldTpl);
}
#endif

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   TimerInit
//
// Description: This function installs the the timer protocol on its handle,
//              and initializes the timer.
//
// Input:       IN EFI_HANDLE ImageHandle - ImageHandle of the loaded driver
//              IN EFI_SYSTEM_TABLE *SystemTable - Pointer to the System Table
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
EFIAPI
TimerInit(
    IN EFI_EVENT          Event,
    IN VOID               *Context
)
{
    EFI_STATUS                      Status;
    EFI_CPU_ARCH_PROTOCOL           *CpuArch;
    UINT8 Value;
    //TimerVector must be initialized to 0, since GetVector only modifies the lowest byte,
    //and RegisterInterruptHandler requires TimerVector to be 4 bytes.
    UINT32                          TimerVector = 0;
#if 0//defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0) //EIP144604
    UINT32                          Irq0TimerVector = 0;
#endif
    // Find the CPU Arch Protocol
    Status = pBS->LocateProtocol(&gEfiCpuArchProtocolGuid, NULL, &CpuArch);
    if(EFI_ERROR(Status)) {
        return ;
    }
    Status = pBS->LocateProtocol(&gEfiLegacy8259ProtocolGuid, NULL, &mLegacy8259);
    if(EFI_ERROR(Status)) {
        TRACE((-1, "Locate gEfiLegacy8259ProtocolGuid error.\n"));
    }
#if 0//defined(HPET_PROTOCOL_SUPPORT) && (HPET_PROTOCOL_SUPPORT != 0) //EIP144604
    // Enable HPET (0x3404)
    EnableHpetInChipset();

    // Retrieve HPET Capabilities and Configuration Information
    gHpetGeneralCapabilities.Uint64  = HpetRead(HPET_GENERAL_CAPABILITIES_ID_OFFSET);
    gHpetGeneralConfiguration.Uint64 = HpetRead(HPET_GENERAL_CONFIGURATION_OFFSET);

    // If Revision is not valid, then ASSERT() and unload the driver because the HPET
    // device is not present.
    if(gHpetGeneralCapabilities.Uint64 == 0 || gHpetGeneralCapabilities.Uint64 == 0xFFFFFFFFFFFFFFFF) {
        TRACE((-1, "HPET device is not present.  Unload HPET driver.\n"));
        return EFI_DEVICE_ERROR;
    }

    HpetEnable(FALSE);
#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE == 0)
    // Enable Legacy Interrupt
    gHpetGeneralConfiguration.Bits.LegacyRouteEnable = 1;
#endif
#endif

    // Disable timer, make sure no interrupt will be created
    Status = SetTimerPeriod(&mTimerProtocol, 0);
    ASSERT_EFI_ERROR(Status);

#if 0//defined(HPET_PROTOCOL_SUPPORT) && (HPET_PROTOCOL_SUPPORT != 0) //EIP144604
    // Configure the selected HPET Timer (Timer#0), clear InterruptEnable to keep
    // interrupts disabled until full init is complete
    // Enable PeriodicInterruptEnable to use perioidic mode
    // Configure as a 32-bit counter
    gTimerConfiguration.Uint64 = HpetRead(HPET_TIMER_CONFIGURATION_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE);
    gTimerConfiguration.Bits.InterruptEnable         = 0;
    gTimerConfiguration.Bits.PeriodicInterruptEnable = 1;
    gTimerConfiguration.Bits.CounterSizeEnable       = 1;
    gTimerConfiguration.Bits.LevelTriggeredInterrupt = 0;
#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0)
    gTimerConfiguration.Bits.InterruptRoute = HPET_APIC_INTERRUPT_PIN;
    gTimerConfiguration.Bits.LevelTriggeredInterrupt = (HPET_INTERRUPT_TRIGGER != 0) ? 1 : 0;
#endif
    HpetWrite(HPET_TIMER_CONFIGURATION_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE, gTimerConfiguration.Uint64);

    // Read the HPET Timer Capabilities and Configuration register back again.
    // CounterSizeEnable will be read back as a 0 if it is a 32-bit only timer
    gTimerConfiguration.Uint64 = HpetRead(HPET_TIMER_CONFIGURATION_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE);
#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0)
    // If the interrupt pin isn't supported by the particular timer, then the value read back won't match that is written.
    if(gTimerConfiguration.Bits.InterruptRoute != HPET_APIC_INTERRUPT_PIN) {
        ASSERT_EFI_ERROR(EFI_UNSUPPORTED);
        return EFI_UNSUPPORTED;
    }
#endif
    if((gTimerConfiguration.Bits.CounterSizeEnable == 1) && (sizeof(UINTN) == sizeof(UINT64))) {
        // 64-bit BIOS can use 64-bit HPET timer
        gCounterMask = 0xffffffffffffffff;
        // Set timer back to 64-bit
        gTimerConfiguration.Bits.CounterSizeEnable = 0;
        HpetWrite(HPET_TIMER_CONFIGURATION_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE, gTimerConfiguration.Uint64);
    } else {
        gCounterMask = 0x00000000ffffffff;
    }
#endif

#if 0//defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0) //EIP144604
    TimerVector = MASTER_INTERRUPT_BASE + HPET_APIC_INTERRUPT_PIN;

    Status = mLegacy8259->GetVector(mLegacy8259,
                                    Efi8259Irq0,
                                    (UINT8 *) & Irq0TimerVector);
    ASSERT_EFI_ERROR(Status);

    Status = CpuArch->RegisterInterruptHandler(CpuArch,
             Irq0TimerVector,
             Irq0InterruptHandler);
    ASSERT_EFI_ERROR(Status);
#else
    Status = mLegacy8259->GetVector(mLegacy8259,
                                    Efi8259Irq0,
                                    (UINT8 *) & TimerVector);
    ASSERT_EFI_ERROR(Status);
#endif

    Status = CpuArch->RegisterInterruptHandler(CpuArch,
             TimerVector,
             TimerInterruptHandler);
    ASSERT_EFI_ERROR(Status);
    // Initialize the handle pointer
    mNotifyFunction = NULL;

#if 0//defined(HPET_PROTOCOL_SUPPORT) && (HPET_PROTOCOL_SUPPORT != 0) //EIP144604
    // Init default for Timer 1
    IoWrite8(TIMER_CTRL, 0x36);
    IoWrite8(TIMER_0_COUNT, 0);
    IoWrite8(TIMER_0_COUNT, 0);
    // Add boot script programming
    Value = 0x36;
    BOOT_SCRIPT_S3_IO_WRITE_MACRO(gBootScript, EfiBootScriptWidthUint8, TIMER_CTRL, 1, &Value);
    Value = 0x0;
    BOOT_SCRIPT_S3_IO_WRITE_MACRO(gBootScript, EfiBootScriptWidthUint8, TIMER_0_COUNT, 1, &Value);
    BOOT_SCRIPT_S3_IO_WRITE_MACRO(gBootScript, EfiBootScriptWidthUint8, TIMER_0_COUNT, 1, &Value);
    // The default value of 10000 100 ns units is the same as 1 ms.
    Status = SetTimerPeriod(&mTimerProtocol, HPET_DEFAULT_TICK_DURATION);
    Status = CreateLegacyBootEvent(TPL_CALLBACK,
                                   StopHpetBeforeLagecyBoot,
                                   NULL,
                                   &gHpetLegacyBootEvent);
#else
    // Force the timer to be enabled at its default period
    Status = SetTimerPeriod(&mTimerProtocol, DEFAULT_TICK_DURATION);
#endif
    ASSERT_EFI_ERROR(Status);

    //Program Timer1 to pass certain customer's test
    IoWrite8(TIMER_CTRL, 0x54);
    IoWrite8(TIMER_1_COUNT, 0x12);
    //add boot script programming
    Value = 0x54;
    BOOT_SCRIPT_S3_IO_WRITE_MACRO(gBootScript, EfiBootScriptWidthUint8, TIMER_CTRL, 1, &Value);
    Value = 0x12;
    BOOT_SCRIPT_S3_IO_WRITE_MACRO(gBootScript, EfiBootScriptWidthUint8, TIMER_1_COUNT, 1, &Value);


    // Install the Timer Architectural Protocol onto a new handle
    Status = pBS->InstallProtocolInterface(
                 &mTimerProtocolHandle,
                 &gEfiTimerArchProtocolGuid,
                 EFI_NATIVE_INTERFACE,
                 &mTimerProtocol
             );
    ASSERT_EFI_ERROR(Status);

    //
    // Close event, so it will not be invoked again.
    //
    pBS->CloseEvent(Event);

    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: ProgramIrqMaskTrigger
//
// Description: Program the Irq Mask and Trigger.
//
// Input: None
//
// Output: None
//
// Notes:  Here is the control flow of this function:
//     1. Program Master Irq Mask.
//     2. Program Slave Irq Mask.
//     3. Program Trigger Level.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ProgramIrqMaskTrigger(
    VOID
)
{
    IoWrite8(0x21, (UINT8)gIrqMask[gMode]);
    IoWrite8(0xa1, (UINT8)(gIrqMask[gMode] >> 8));
    //
    // 4d0 can not be accessed as by IoWrite16, we have to split
    //
    IoWrite8(0x4d0, (UINT8)gIrqTrigger[gMode]);
    IoWrite8(0x4d1, (UINT8)(gIrqTrigger[gMode] >> 8));
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: SetVectorBase
//
// Description: Initializes the interrupt controller and program the Irq
//              Master and Slave Vector Base.
//
// Input:  *This  Pointer to this object
//         MasterBase IRQ base for the master 8259 controller
//         SlaveBase IRQ base for the slave 8259 controller
//
// Output: EFI_SUCCESS - Interrupt on the interrupt controllers was enabled.
//
// Notes:  Here is the control flow of this function:
//    1. If Master base is changed, initialize master 8259 setting
//       the interrupt offset.
//    2. If Slave base is changed, initialize slave 8259 setting
//       the interrupt offset.
//    3. Return EFI_SUCCESS.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SetVectorBase(
    IN EFI_LEGACY_8259_PROTOCOL   *This,
    IN UINT8                      MasterBase,
    IN UINT8                      SlaveBase
)
{
    //
    // 8259 Master
    //
    if(MasterBase != gMasterBase) {
        //
        // Start 8259 Master Initialization.
        //
        IoWrite8(LEGACY_8259_CONTROL_REGISTER_MASTER, ICW1);
        //
        // Set Interrupt Offset
        //
        IoWrite8(LEGACY_8259_MASK_REGISTER_MASTER, MasterBase);
        //
        // Set Slave IRQ.
        //
        IoWrite8(LEGACY_8259_MASK_REGISTER_MASTER, ICW3_M);
        //
        // Set 8259 mode. See ICW4 comments with #define.
        //
        IoWrite8(LEGACY_8259_MASK_REGISTER_MASTER, ICW4);
        gMasterBase = MasterBase;
    }
    //
    // 8259 Slave
    //
    if(SlaveBase != gSlaveBase) {
        //
        // Start 8259 Slave  Initialization.
        //
        IoWrite8(LEGACY_8259_CONTROL_REGISTER_SLAVE, ICW1);
        //
        // Set Interrupt Offset
        //
        IoWrite8(LEGACY_8259_MASK_REGISTER_SLAVE, SlaveBase);
        //
        // Set Slave IRQ.
        //
        IoWrite8(LEGACY_8259_MASK_REGISTER_SLAVE, ICW3_S);
        //
        // Set 8259 mode. See ICW4 comments with #define.
        //
        IoWrite8(LEGACY_8259_MASK_REGISTER_SLAVE, ICW4);
        gSlaveBase = SlaveBase;
    }

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: GetMask
//
// Description: Get the Master/Slave Irq Mask, Irq Level for Legacy real
//              mode and protected mode.
//
// Input:  *This    Pointer to this object
//         *LegacyMask   Legacy mode interrupt mask
//         *LegacyEdgeLevel Legacy mode edge/level trigger value
//         *ProtectedMask  Protected mode interrupt mask
//         *ProtectedEdgeLevel Protected mode edge/level trigger value
//
// Output:  EFI_SUCCESS - Returned irq mask/level.
//
// Notes:  Here is the control flow of this function:
//    1. If *LegacyMask not NULL, get legacy Mask.
//    2. If *LegacyEdgeLevel not NULL, get legacy Trigger Level.
//    3. If *ProtectedMask not NULL, get protected Mask.
//    4. If *ProtectedEdgeLevel not NULL, get protected trigger level.
//    5. Return EFI_SUCCESS.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetMask(
    IN EFI_LEGACY_8259_PROTOCOL   *This,
    OUT UINT16                    *LegacyMask   OPTIONAL,
    OUT UINT16                    *LegacyEdgeLevel OPTIONAL,
    OUT UINT16                    *ProtectedMask  OPTIONAL,
    OUT UINT16                    *ProtectedEdgeLevel OPTIONAL
)
{
    if(LegacyMask)   *LegacyMask   = gIrqMask[0];
    if(LegacyEdgeLevel) *LegacyEdgeLevel = gIrqTrigger[0];
    if(ProtectedMask)  *ProtectedMask  = gIrqMask[1];
    if(ProtectedEdgeLevel) *ProtectedEdgeLevel = gIrqTrigger[1];

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: SetMask
//
// Description: Set the Master/Slave Irq Mask, Irq Level for Legacy real mode
//              and protected mode.
//
// Input:  *This    Pointer to this object
//         *LegacyMask   Legacy mode interrupt mask
//         *LegacyEdgeLevel Legacy mode edge/level trigger value
//         *ProtectedMask  Protected mode interrupt mask
//         *ProtectedEdgeLevel Protected mode edge/level trigger value
//
// Output:  EFI_SUCCESS - Set irq mask/level.
//
// Notes:  Here is the control flow of this function:
//    1. If *LegacyMask not NULL, set legacy mask variable.
//    2. If *LegacyEdgeLevel not NULL, set legacy Trigger Level variable.
//    3. If *ProtectedMask not NULL, set protected mask variable.
//    4. If *ProtectedEdgeLevel not NULL, set protected trigger level variable.
//    5. Call function to program 8259 with mask/trigger of current mode.
//    6. Return EFI_SUCCESS.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SetMask(
    IN EFI_LEGACY_8259_PROTOCOL   *This,
    IN UINT16                     *LegacyMask   OPTIONAL,
    IN UINT16                     *LegacyEdgeLevel OPTIONAL,
    IN UINT16                     *ProtectedMask  OPTIONAL,
    IN UINT16                     *ProtectedEdgeLevel OPTIONAL
)
{
    if(LegacyMask)   gIrqMask[0]  = *LegacyMask;
    if(LegacyEdgeLevel) gIrqTrigger[0] = *LegacyEdgeLevel;
    if(ProtectedMask)  gIrqMask[1]  = *ProtectedMask;
    if(ProtectedEdgeLevel) gIrqTrigger[1] = *ProtectedEdgeLevel;

    ProgramIrqMaskTrigger();

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: SetMode
//
// Description: Sets the interrupt mode operation to legacy or protected.
//              New mask and edge/level status can be provided as input
//
// Input:  *This  Pointer to this object
//         Mode  Interrupt mode setting
//         *Mask  New interrupt mask for this mode
//         *EdgeLevel New edge/level trigger value for this mode
//
// Output: EFI_SUCCESS - Set mode was successful
//
// Notes:  Here is the control flow of this function:
//    1. If invalid mode, return EFI_INVALID_PARAMETER.
//    2. If *Mask not NULL, set mode mask variable.
//    3. If *EdgeLevel not NULL, set mode trigger level variable.
//    4. Call function to program 8259 with mask/trigger of current mode.
//    5. Return EFI_SUCCESS.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SetMode(
    IN EFI_LEGACY_8259_PROTOCOL   *This,
    IN EFI_8259_MODE              Mode,
    IN UINT16                     *Mask   OPTIONAL,
    IN UINT16                     *EdgeLevel OPTIONAL
)
{
    if(Mode >= Efi8259MaxMode) return EFI_INVALID_PARAMETER;

#if defined (HPET_INTERRUPT_TRIGGER) && (HPET_INTERRUPT_TRIGGER != 0)
    if(Mode == Efi8259LegacyMode)
        gTimerConfiguration.Bits.InterruptEnable = 0;
    else    // Efi8259ProtectedMode
        gTimerConfiguration.Bits.InterruptEnable = 1;
    HpetWrite(
        HPET_TIMER_CONFIGURATION_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE,
        gTimerConfiguration.Uint64
    );
#endif

    gMode = Mode;
    if(Mask) gIrqMask[Mode] = *Mask;
    if(EdgeLevel) gIrqTrigger[Mode] = *EdgeLevel;

    ProgramIrqMaskTrigger();

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: GetVector
//
// Description: Returns the vector number for the requested IRQ
//
// Input:  This Legacy8259 protocol object
//         Irq  IRQ number for which vector is needed
//         Vector Vector value is returned in this pointer
//
// Output: EFI_STATUS
//         EFI_SUCCESS - Get Irq Vector for Irq.
//
// Notes:  Here is the control flow of this function:
//    1. If invalid irq, return EFI_INVALID_PARAMETER.
//    2. If Set *Vector to Irq base + interrupt offset.
//    3. Return EFI_SUCCESS.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetVector(
    IN EFI_LEGACY_8259_PROTOCOL   *This,
    IN EFI_8259_IRQ               Irq,
    OUT UINT8                     *Vector
)
{
    if((UINT8)Irq >= (UINT8)Efi8259IrqMax) return EFI_INVALID_PARAMETER;
    *Vector = (Irq <= Efi8259Irq7) ? gMasterBase + Irq : gSlaveBase + Irq - 8;

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: EnableIrq
//
// Description: Enable the Interrupt controllers to respond in a specific IRQ.
//
// Input:  This   Legacy8259 protocol object
//         Irq    IRQ that has to be enabled
//         LevelTriggered Trigger mechanism (level or edge) for this IRQ
//
// Output:  EFI_STATUS
//          EFI_SUCCESS - Interrupt on the interrupt controllers was enabled.
//          EFI_INVALID_PARAMETER - Interrupt not existent.
//          The 8259 master/slave supports IRQ 0-15.
//
// Notes:  Here is the control flow of this function:
//     1. Check if Irq is valid. If not, return EFI_INVALID_PARAMETER.
//     2. Clear interrupt mask bit in variable of current mode.
//     3. Set/Clear bit of trigger level variable of current mode.
//     4. Program mask/trigger.
//     5. Return EFI_SUCCESS.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EnableIrq(
    IN EFI_LEGACY_8259_PROTOCOL   *This,
    IN EFI_8259_IRQ               Irq,
    IN BOOLEAN                    LevelTriggered
)
{
    if((UINT8)Irq > (UINT8)15) return EFI_INVALID_PARAMETER;
    //
    // Clear mask for the Irq.
    //
    gIrqMask[gMode] &= ~(1 << Irq);
    //
    // Mask Irq to change.
    //
    gIrqTrigger[gMode] &= ~(1 << Irq);
    //
    // Change irq bit, 0 = edge, 1 = level.
    //
    if(LevelTriggered) gIrqTrigger[gMode] |= (1 << Irq);

    ProgramIrqMaskTrigger();
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: DisableIrq
//
// Description: Disable the Interrupt controllers to stop responding to
//              a specific IRQ.
//
// Input: This   Legacy8259 protocol object
//        Irq    IRQ that has to be disabled
//
// Output: EFI_STATUS
//         EFI_SUCCESS - Interrupt on the interrupt controllers was enabled.
//         EFI_INVALID_PARAMETER - Interrupt not existent.
//         The 8259 master/slave supports IRQ 0-15.
//
// Notes:  Here is the control flow of this function:
//     1. Check if Irq is valid. If not, return EFI_INVALID_PARAMETER.
//     2. Set interrupt mask bit in variable of current mode.
//     3. Program mask/trigger.
//     4. Return EFI_SUCCESS.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS DisableIrq(
    IN EFI_LEGACY_8259_PROTOCOL   *This,
    IN EFI_8259_IRQ               Irq
)
{
    if((UINT8)Irq > (UINT8)15) return EFI_INVALID_PARAMETER;
    //
    // Set mask for the Irq.
    //
    gIrqMask[gMode] |= 1 << Irq;

    ProgramIrqMaskTrigger();
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: GetInterruptLine
//
// Description: Get IRQ vector asigned to Pci card.
//
// Input: This  Pointer to this object
//        PciHandle PCI handle for this device
//        Vector  Interrupt vector this device
//
// Output: EFI_STATUS
//         EFI_SUCCESS - Vector returned.
//         EFI_INVALID_PARAMETER - Invalid PciHandle.
//
// Notes:  Here is the control flow of this function:
//     1. Get Handle of PciIo installed on PciHandle.
//     2. If PciIo not installed, return EFI_INVALID_DEVICE.
//     3. Set *vector to Irq vector programmed into card.
//     4. Return EFI_SUCCESS.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetInterruptLine(
    IN EFI_LEGACY_8259_PROTOCOL   *This,
    IN EFI_HANDLE                 PciHandle,
    OUT UINT8                     *Vector
)
{
    EFI_STATUS    Status;
    EFI_PCI_IO_PROTOCOL  *PciIo;
    UINT8     InterruptLine;

    Status = pBS->HandleProtocol(
                 PciHandle,
                 &gEfiPciIoProtocolGuid,
                 &PciIo
             );
    if(EFI_ERROR(Status)) return EFI_INVALID_PARAMETER;
    //
    // Read vector from card.
    //
    PciIo->Pci.Read(
        PciIo,
        EfiPciIoWidthUint8,
        PCI_INTLINE,
        1,
        &InterruptLine
    );

    *Vector = InterruptLine;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: EndOfInterrupt
//
// Description: Send end of interrupt command to interrupt controller(s).
//
// Input: This  Pointer to this object
//        Irq   IRQ number for this EOI has to be sent
//
// Output: EFI_STATUS
//         EFI_SUCCESS - EOI command sent to controller(s)
//         EFI_INVALID_PARAMETER - Interrupt not existent. The 8259 master/slave supports IRQ 0-15.
//
// Notes:  Here is the control flow of this function:
//     1. Check if Irq is valid. If not, return EFI_INVALID_PARAMETER.
//     2. If Irq >= 8, then Send EOI to slave controller.
//     3. Send EOI to master controller. (This is for both master/slave Irqs)
//     4. Return EFI_SUCCESS.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EndOfInterrupt(
    IN EFI_LEGACY_8259_PROTOCOL   *This,
    IN EFI_8259_IRQ               Irq
)
{
    if(Irq > 15) return EFI_INVALID_PARAMETER;
    //
    // If Slave, send EOI to slave first.
    //
    if(Irq >= 8) {
        //
        // Send Slave EOI
        //
        IoWrite8(LEGACY_8259_CONTROL_REGISTER_SLAVE, EOI_COMMAND);
    }
    //
    // Send Master EOI
    //
    IoWrite8(LEGACY_8259_CONTROL_REGISTER_MASTER, EOI_COMMAND);

    return EFI_SUCCESS;
}

EFI_LEGACY_8259_PROTOCOL gLegacy8259Protocol = {
    SetVectorBase,
    GetMask,
    SetMask,
    SetMode,
    GetVector,
    EnableIrq,
    DisableIrq,
    GetInterruptLine,
    EndOfInterrupt
};


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: Initialize8259
//
// Description: Program the Irq Mask and Trigger.
//
// Input: ImageHandle  Image handle for this driver
//        SystemTable  Pointer to the sytem table
//
// Output: EFI_STATUS
//
// Notes:  Here is the control flow of this function:
//     1. Initialize Ami Lib.
//     2. Install Legacy 8259 Interface. If error, return error.
//     3. Initialize the Cpu setting vector bases.
//     4. Set Cpu Mode, mask, and trigger level.
//     5. Return EFI_SUCCESS.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS Initialize8259(
    IN EFI_HANDLE                 ImageHandle,
    IN EFI_SYSTEM_TABLE           *SystemTable
)
{
    EFI_STATUS    Status;
    UINT16        Mask  = 0xffff;     //Mask all interrupts.
    UINT16        EdgeLevel = 0x00;   //Set all edge.

    SetVectorBase(0, MASTER_INTERRUPT_BASE, SLAVE_INTERRUPT_BASE);
    SetMode(0, Efi8259ProtectedMode, &Mask, &EdgeLevel);

    Status = pBS->InstallMultipleProtocolInterfaces(
                 &ImageHandle,
                 &gEfiLegacy8259ProtocolGuid,
                 &gLegacy8259Protocol,
                 NULL
             );

    return Status;
}

UINT8
ReadCmosBank1Byte(
    IN UINT8                  Index
)
{
    IoWrite8(R_PCH_RTC_EXT_INDEX, Index);
    return IoRead8(R_PCH_RTC_EXT_TARGET);
}

VOID
WriteCmosBank1Byte(
    IN UINT8                  Index,
    IN UINT8                  Data
)
{
    IoWrite8(R_PCH_RTC_EXT_INDEX, Index);
    IoWrite8(R_PCH_RTC_EXT_TARGET, Data);
}

VOID
InitTpm(VOID)
{
    EFI_STATUS                      Status;
    EFI_TPM_MP_DRIVER_PROTOCOL      *TpmMpDriver;
    UINT8                           Data;
    UINT8                           ReceiveBuffer [64];
    UINT32                          ReceiveBufferSize;
    UINT8 TpmForceClearCommand [] =              {0x00, 0xC1,
            0x00, 0x00, 0x00, 0x0A,
            0x00, 0x00, 0x00, 0x5D
                                                 };
    UINT8 TpmPhysicalPresenceCommand [] =        {0x00, 0xC1,
            0x00, 0x00, 0x00, 0x0C,
            0x40, 0x00, 0x00, 0x0A,
            0x00, 0x00
                                                 };
    UINT8 TpmPhysicalDisableCommand [] =         {0x00, 0xC1,
            0x00, 0x00, 0x00, 0x0A,
            0x00, 0x00, 0x00, 0x70
                                                 };
    UINT8 TpmPhysicalEnableCommand [] =          {0x00, 0xC1,
            0x00, 0x00, 0x00, 0x0A,
            0x00, 0x00, 0x00, 0x6F
                                                 };
    UINT8 TpmPhysicalSetDeactivatedCommand [] =  {0x00, 0xC1,
            0x00, 0x00, 0x00, 0x0B,
            0x00, 0x00, 0x00, 0x72,
            0x00
                                                 };
    UINT8 TpmSetOwnerInstallCommand [] =         {0x00, 0xC1,
            0x00, 0x00, 0x00, 0x0B,
            0x00, 0x00, 0x00, 0x71,
            0x00
                                                 };

    // Handle ACPI OS TPM requests here
    Status = gBS->LocateProtocol(&gEfiTpmMpDriverProtocolGuid, NULL, &TpmMpDriver);
    if(!EFI_ERROR(Status)) {
        Data = CmosReadByte(SB_EFI_ACPI_TPM_REQUEST);
        CmosWriteByte(SB_EFI_ACPI_TPM_REQUEST, 0x00);
        if(Data != 0) {
            CmosWriteByte(SB_EFI_ACPI_TPM_LAST_REQUEST, Data);
            // Assert Physical Presence for these commands
            TpmPhysicalPresenceCommand [11] = 0x20;
            ReceiveBufferSize = sizeof(ReceiveBuffer);
            TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalPresenceCommand,
                                           sizeof(TpmPhysicalPresenceCommand),
                                           ReceiveBuffer, &ReceiveBufferSize);
            // PF PhysicalPresence = TRUE
            TpmPhysicalPresenceCommand [11] = 0x08;
            ReceiveBufferSize = sizeof(ReceiveBuffer);
            TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalPresenceCommand,
                                           sizeof(TpmPhysicalPresenceCommand),
                                           ReceiveBuffer, &ReceiveBufferSize);
            if(Data == 0x01) {
                // TPM_PhysicalEnable
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalEnableCommand,
                                               sizeof(TpmPhysicalEnableCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
            }
            if(Data == 0x02) {
                // TPM_PhysicalDisable
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalDisableCommand,
                                               sizeof(TpmPhysicalDisableCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
            }
            if(Data == 0x03) {
                // TPM_PhysicalSetDeactivated=FALSE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmPhysicalSetDeactivatedCommand [10] = 0x00;
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalSetDeactivatedCommand,
                                               sizeof(TpmPhysicalSetDeactivatedCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
                gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
            }
            if(Data == 0x04) {
                // TPM_PhysicalSetDeactivated=TRUE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmPhysicalSetDeactivatedCommand [10] = 0x01;
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalSetDeactivatedCommand,
                                               sizeof(TpmPhysicalSetDeactivatedCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
                gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
            }
            if(Data == 0x05) {
                // TPM_ForceClear
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmForceClearCommand,
                                               sizeof(TpmForceClearCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
                gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
            }
            if(Data == 0x06) {
                // TPM_PhysicalEnable
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalEnableCommand,
                                               sizeof(TpmPhysicalEnableCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
                // TPM_PhysicalSetDeactivated=FALSE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmPhysicalSetDeactivatedCommand [10] = 0x00;
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalSetDeactivatedCommand,
                                               sizeof(TpmPhysicalSetDeactivatedCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
                gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
            }
            if(Data == 0x07) {
                // TPM_PhysicalSetDeactivated=TRUE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmPhysicalSetDeactivatedCommand [10] = 0x01;
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalSetDeactivatedCommand,
                                               sizeof(TpmPhysicalSetDeactivatedCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
                // TPM_PhysicalDisable
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalDisableCommand,
                                               sizeof(TpmPhysicalDisableCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
                gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
            }
            if(Data == 0x08) {
                // TPM_SetOwnerInstall=TRUE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmSetOwnerInstallCommand [10] = 0x01;
                TpmMpDriver->Transmit(TpmMpDriver, TpmSetOwnerInstallCommand,
                                               sizeof(TpmSetOwnerInstallCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
            }
            if(Data == 0x09) {
                // TPM_SetOwnerInstall=FALSE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmSetOwnerInstallCommand [10] = 0x00;
                TpmMpDriver->Transmit(TpmMpDriver, TpmSetOwnerInstallCommand,
                                               sizeof(TpmSetOwnerInstallCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
            }
            if(Data == 0x0A) {
                // TPM_PhysicalEnable
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalEnableCommand,
                                               sizeof(TpmPhysicalEnableCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
                // TPM_PhysicalSetDeactivated=FALSE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmPhysicalSetDeactivatedCommand [10] = 0x00;
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalSetDeactivatedCommand,
                                               sizeof(TpmPhysicalSetDeactivatedCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
                // Do TPM_SetOwnerInstall=TRUE on next reboot
                CmosWriteByte(SB_EFI_ACPI_TPM_REQUEST, 0xF0);
                gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
            }
            if(Data == 0x0B) {
                // TPM_SetOwnerInstall=FALSE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmSetOwnerInstallCommand [10] = 0x00;
                TpmMpDriver->Transmit(TpmMpDriver, TpmSetOwnerInstallCommand,
                                               sizeof(TpmSetOwnerInstallCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
                // TPM_PhysicalSetDeactivated=TRUE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmPhysicalSetDeactivatedCommand [10] = 0x01;
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalSetDeactivatedCommand,
                                               sizeof(TpmPhysicalSetDeactivatedCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
                // TPM_PhysicalDisable
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalDisableCommand,
                                               sizeof(TpmPhysicalDisableCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
                gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
            }
            if(Data == 0x0E) {
                // TPM_ForceClear
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmForceClearCommand,
                                               sizeof(TpmForceClearCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
                // TPM_PhysicalEnable
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalEnableCommand,
                                               sizeof(TpmPhysicalEnableCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
                // TPM_PhysicalSetDeactivated=FALSE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmPhysicalSetDeactivatedCommand [10] = 0x00;
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalSetDeactivatedCommand,
                                               sizeof(TpmPhysicalSetDeactivatedCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
                gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
            }
            if(Data == 0xF0) {
                // Second part of ACPI TPM request 0x0A: OEM custom TPM_SetOwnerInstall=TRUE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmSetOwnerInstallCommand [10] = 0x01;
                TpmMpDriver->Transmit(TpmMpDriver, TpmSetOwnerInstallCommand,
                                               sizeof(TpmSetOwnerInstallCommand),
                                               ReceiveBuffer, &ReceiveBufferSize);
                CmosWriteByte(SB_EFI_ACPI_TPM_LAST_REQUEST, 0xA0);
            }
            // Deassert Physical Presence
            TpmPhysicalPresenceCommand [11] = 0x10;
            ReceiveBufferSize = sizeof(ReceiveBuffer);
            TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalPresenceCommand,
                                           sizeof(TpmPhysicalPresenceCommand),
                                           ReceiveBuffer, &ReceiveBufferSize);
        }
    }
}

VOID
InitPciDevPme(VOID)
{
	if(0){
	///Program EHCI PME_EN
	  PchMmPci32Or (0, 
	                0, 
	                PCI_DEVICE_NUMBER_PCH_USB, 
	                PCI_FUNCTION_NUMBER_PCH_EHCI, 
	                R_PCH_EHCI_PWR_CNTL_STS, 
	                B_PCH_EHCI_PWR_CNTL_STS_PME_EN
	                );
	  }
	   {
	     UINTN                 EhciPciMmBase;
	     UINT32                Buffer32 = 0;

	    EhciPciMmBase = MmPciAddress (0,
	                      0,
	                      PCI_DEVICE_NUMBER_PCH_USB,
	                      PCI_FUNCTION_NUMBER_PCH_EHCI,
	                      0
	                    );
	    TRACE((TRACE_ALWAYS, "ConfigureAdditionalPm() EhciPciMmBase = 0x%x \n",EhciPciMmBase)); 
	    Buffer32 = MmioRead32(EhciPciMmBase + R_PCH_EHCI_PWR_CNTL_STS);
	    TRACE((TRACE_ALWAYS, "ConfigureAdditionalPm() R_PCH_EHCI_PWR_CNTL_STS = 0x%x \n",Buffer32));
	  }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: ReadyToBootFunction
//
// Description: The function called for Ready To Boot.
//
// Input:       Event   - Watchdog event
//              Context - Context pointer
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
ReadyToBootFunction(
    EFI_EVENT                 Event,
    VOID                      *Context
)
{
    /*
      if (mMfgMode) {
        //
        // If in manufacturing mode....
        // Set the Normal Setup to its defaults, clear memory configuration
        //

        Status = gRT->SetVariable (
                  gEfiNormalSetupName,
                  &gEfiNormalSetupGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  sizeof(SETUP_DATA),
                  &mSystemDefaults
                  );
        //
        // Clear memory configuration moved to SaveMemoryConfig driver.
        //

        //
        // Get the Event Log Protocol.
        //
        Status = gBS->LocateProtocol (&gEfiEventLogProtocolGuid, NULL, &EventLog);
        if (!EFI_ERROR(Status)) {
          //
          // If in manufacturing mode....
          //
          Status = EventLog->ClearEvents(EventLog);
        }
      }*/

    // Init TPM
//  InitTpm ();

    // Init Pci Device PME bit
    InitPciDevPme();

    return;
}

//(CSP20130508B+)>>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: SbSpiProgramVscc
//
// Description: Program SPI VSCC.
//
// Input: None
//
// Output: None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID SbSpiProgramVscc (
    VOID
)
{
    volatile UINT8  *SPIBASE = (UINT8*)(UINTN)SPI_BASE;
    UINT32          dVSCC = 0;
    TRACE((TRACE_ALWAYS, "[[ SbSpiProgramVscc() Start. ]]\n"));
    
    dVSCC = VSCCSETTING;
    
#if LOWER_VSCC_REG != 0
        *(volatile UINT32*)(SPIBASE + LOWER_VSCC_REG) = dVSCC;
#endif 
#if UPPER_VSCC_REG != 0
        *(volatile UINT32*)(SPIBASE + UPPER_VSCC_REG) = dVSCC;
#endif 
    TRACE((TRACE_ALWAYS, "[[ SbSpiProgramVscc() Done. ]]\n"));
}
//(CSP20130508B+)<<

//EIP127537 >> 
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   UsbPerPortDisableCallback
//
// Description: This function can disable USB port before OS booting.             
//
// Input:       Event   - Event of callback
//              Context - Context of callback.
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

#if (EOP_USB_PER_PORT_CTRL == 2)
VOID UsbPerPortDisableCallback (
    IN EFI_EVENT      Event,
    IN VOID           *Context
)
{
    SB_SETUP_DATA     PchPolicyData;
    UINT32            UsbPortsCtrlValue;
    UINT32            Index;

    // Get the value of the SB Setup data.
    GetSbSetupData ((VOID*)gRT, &PchPolicyData, FALSE);
    
    UsbPortsCtrlValue = 0x1F;

//EIP160754 >>
    for (Index = 0; Index < PCH_USB_MAX_PHYSICAL_PORTS; Index++) 
    {
      if (PchPolicyData.PchUsbPort[Index] == 0) 
      {
        UsbPortsCtrlValue &= (UINT32) ~(1 << Index);
      }        
    }

    for (Index = 0; Index < PCH_XHCI_MAX_USB3_PORTS; Index++) 
    {
      if (PchPolicyData.PchUsbPort[Index] == 0) 
      {
        UsbPortsCtrlValue &= (UINT32) ~(1 << (PCH_USB_MAX_PHYSICAL_PORTS + Index));
      }        
    }

    SbUsbPortsControlHook (UsbPortsCtrlValue, PchPolicyData.PchUsb30Mode);
//EIP160754 <<

//EIP138173 >>
#if DISABLE_USB_CONTROLLER_WHEN_DISBALE_ALL_PORT
    DisableUsbController(&PchPolicyData);
#endif
//EIP138173 <<
    
    pBS->CloseEvent(Event);
}
#endif
//EIP127537 << 

//EIP138173 >>
#if DISABLE_USB_CONTROLLER_WHEN_DISBALE_ALL_PORT
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   DisableUsbController
//
// Description: This function can disable USB controller when all USB port are disabled.             
//
// Input:       UsbPortsCtrlValue - Enable Ports //(EOP_USB_PER_PORT_CTRL == 2)
//				PchPolicyData - SB Setup Data
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID DisableUsbController (
    IN 	SB_SETUP_DATA  *PchPolicyData
)
{
    EFI_STATUS         Status;
    UINT32             Ehci1PciMmBase; 
    UINT32             XhciPciMmBase;
    UINT32             PmcBase; 
    UINT32             RootComplexBar;
    UINT32             Buffer32;
    UINT8 			   Index;
    BOOLEAN			   AnyPortEnable = FALSE;
    
    for(Index=0; Index<PCH_USB_MAX_PHYSICAL_PORTS; Index++)
    	AnyPortEnable |= PchPolicyData->PchUsbPort[Index];

    if(AnyPortEnable == 0)
	{   
        Status = pBS->LocateProtocol (
                        &gEfiS3SaveStateProtocolGuid,
                        NULL,
                        &gBootScriptSave
                        );
        ASSERT_EFI_ERROR (Status);        

//EIP138569 >> 
#if defined AMIUSB_SUPPORT && AMIUSB_SUPPORT == 1        
        Status = pBS->LocateProtocol (
                        &gEfiUsbProtocolGuid,
                        NULL,
                        &gUsbProtocol
                        );
        ASSERT_EFI_ERROR (Status);        
#endif
//EIP138569 <<
        
		    PmcBase = MmioRead32(SB_REG(R_PCH_LPC_PMC_BASE)) & B_PCH_LPC_PMC_BASE_BAR;

//EIP160754 >>		
		    if (PchPolicyData->PchUsb30Mode == 2 || PchPolicyData->PchUsb30Mode == 3) //AUTO or SMART AUTO
		    {
		      //EHCI controller
		      Ehci1PciMmBase  = MmioRead32(USB_EHCI_BUS_DEV_FUN);
		      RootComplexBar  = MmioRead32(SB_REG(R_PCH_LPC_RCBA)) & B_PCH_LPC_RCBA_BAR;

//EIP138569 >>
#if defined AMIUSB_SUPPORT && AMIUSB_SUPPORT == 1   
		      gUsbProtocol->UsbRtStopController(EHCI_BUS<<8 | EHCI_DEV<<3 | EHCI_FUN);
#endif    	
//EIP138569	<<
            
		      ///
    		  /// Put device in D3hot state.
		      ///   	
		      Buffer32 = MmioRead32(Ehci1PciMmBase + R_PCH_EHCI_PWR_CNTL_STS) | V_PCH_EHCI_PWR_CNTL_STS_PWR_STS_D3;			
		      MmioWrite32(Ehci1PciMmBase + R_PCH_EHCI_PWR_CNTL_STS, Buffer32);
		      BOOT_SCRIPT_S3_MEM_WRITE_MACRO( gBootScriptSave, \
    	                                    EfiBootScriptWidthUint32, \
    	                                    Ehci1PciMmBase + R_PCH_EHCI_PWR_CNTL_STS, \
    	                                    1, \
    	                                    &Buffer32 );
    		
		      ///
		      /// Program function disable bit in IOSF2SPXB bridge.
		      ///  		
		      Buffer32 = MmioRead32(RootComplexBar + 0x220) | 0x01;		
		      MmioWrite32(RootComplexBar + 0x220, Buffer32);
		      BOOT_SCRIPT_S3_MEM_WRITE_MACRO( gBootScriptSave, \
    	                                    EfiBootScriptWidthUint32, \
    	                                    RootComplexBar + 0x220, \
    	                                    1, \
    	                                    &Buffer32 );
		      Buffer32 = MmioRead32(PmcBase + R_PCH_PMC_FUNC_DIS) | B_PCH_PMC_FUNC_DIS_USB;		
		      MmioWrite32(PmcBase + R_PCH_PMC_FUNC_DIS, Buffer32);
		      BOOT_SCRIPT_S3_MEM_WRITE_MACRO( gBootScriptSave, \
    	                                    EfiBootScriptWidthUint32, \
    	                                    PmcBase + R_PCH_PMC_FUNC_DIS, \
    	                                    1, \
    	                                    &Buffer32 );

		      //XHCI controller
		      XhciPciMmBase  = MmioRead32(USB_XHCI_BUS_DEV_FUN);

//EIP138569 >>
#if defined AMIUSB_SUPPORT && AMIUSB_SUPPORT == 1   
		      gUsbProtocol->UsbRtStopController(XHCI_BUS<<8 | XHCI_DEV<<3 | XHCI_FUN);
#endif  
//EIP138569 <<
			
		      ///
		      /// Put device in D3hot state.
		      ///   	
		      Buffer32 = MmioRead32(XhciPciMmBase + R_PCH_XHCI_PWR_CNTL_STS) | V_PCH_XHCI_PWR_CNTL_STS_PWR_STS_D3;		
		      MmioWrite32(XhciPciMmBase + R_PCH_XHCI_PWR_CNTL_STS, Buffer32);
		      BOOT_SCRIPT_S3_MEM_WRITE_MACRO( gBootScriptSave, \
    	                                    EfiBootScriptWidthUint32, \
    	                                    XhciPciMmBase + R_PCH_XHCI_PWR_CNTL_STS, \
    	                                    1, \
    	                                    &Buffer32 );
    		
		      ///
		      /// Program function disable bit.
		      ///    
		      Buffer32 = MmioRead32(PmcBase + R_PCH_PMC_FUNC_DIS) | B_PCH_PMC_FUNC_DIS_USH;		
		      MmioWrite32(PmcBase + R_PCH_PMC_FUNC_DIS, Buffer32);
		      BOOT_SCRIPT_S3_MEM_WRITE_MACRO( gBootScriptSave, \
    	                                    EfiBootScriptWidthUint32, \
    	                                    PmcBase + R_PCH_PMC_FUNC_DIS, \
    	                                    1, \
    	                                    &Buffer32 );
		      Buffer32 = MmioRead32(PmcBase + R_PCH_PMC_FUNC_DIS2) | B_PCH_PMC_FUNC_DIS2_USH_SS_PHY;	
		      MmioWrite32(PmcBase + R_PCH_PMC_FUNC_DIS2, Buffer32);
		      BOOT_SCRIPT_S3_MEM_WRITE_MACRO( gBootScriptSave, \
    	                                    EfiBootScriptWidthUint32, \
    	                                    PmcBase + R_PCH_PMC_FUNC_DIS2, \
    	                                    1, \
    	                                    &Buffer32 );
		    }
		    else
		    {
		      if(PchPolicyData->PchUsb20 == 1)
		      { 
		        Ehci1PciMmBase  = MmioRead32(USB_EHCI_BUS_DEV_FUN);
		        RootComplexBar  = MmioRead32(SB_REG(R_PCH_LPC_RCBA)) & B_PCH_LPC_RCBA_BAR;

//EIP138569 >>
#if defined AMIUSB_SUPPORT && AMIUSB_SUPPORT == 1   
		        gUsbProtocol->UsbRtStopController(EHCI_BUS<<8 | EHCI_DEV<<3 | EHCI_FUN);
#endif      
//EIP138569 <<
            
		        ///
		        /// Put device in D3hot state.
		        ///     
		        Buffer32 = MmioRead32(Ehci1PciMmBase + R_PCH_EHCI_PWR_CNTL_STS) | V_PCH_EHCI_PWR_CNTL_STS_PWR_STS_D3;     
		        MmioWrite32(Ehci1PciMmBase + R_PCH_EHCI_PWR_CNTL_STS, Buffer32);
		        BOOT_SCRIPT_S3_MEM_WRITE_MACRO( gBootScriptSave, \
                                          EfiBootScriptWidthUint32, \
                                          Ehci1PciMmBase + R_PCH_EHCI_PWR_CNTL_STS, \
                                          1, \
                                          &Buffer32 );
        
		        ///
		        /// Program function disable bit in IOSF2SPXB bridge.
		        ///     
		        Buffer32 = MmioRead32(RootComplexBar + 0x220) | 0x01;   
		        MmioWrite32(RootComplexBar + 0x220, Buffer32);
		        BOOT_SCRIPT_S3_MEM_WRITE_MACRO( gBootScriptSave, \
                                          EfiBootScriptWidthUint32, \
                                          RootComplexBar + 0x220, \
                                          1, \
                                          &Buffer32 );
		        Buffer32 = MmioRead32(PmcBase + R_PCH_PMC_FUNC_DIS) | B_PCH_PMC_FUNC_DIS_USB;   
		        MmioWrite32(PmcBase + R_PCH_PMC_FUNC_DIS, Buffer32);
		        BOOT_SCRIPT_S3_MEM_WRITE_MACRO( gBootScriptSave, \
                                          EfiBootScriptWidthUint32, \
                                          PmcBase + R_PCH_PMC_FUNC_DIS, \
                                          1, \
                                          &Buffer32 );
		      }
      
		      if(PchPolicyData->PchUsb30Mode == 1)
		      {   
		        XhciPciMmBase  = MmioRead32(USB_XHCI_BUS_DEV_FUN);

//EIP138569 >>
#if defined AMIUSB_SUPPORT && AMIUSB_SUPPORT == 1   
		        gUsbProtocol->UsbRtStopController(XHCI_BUS<<8 | XHCI_DEV<<3 | XHCI_FUN);
#endif  
//EIP138569 <<
      
		        ///
		        /// Put device in D3hot state.
		        ///     
		        Buffer32 = MmioRead32(XhciPciMmBase + R_PCH_XHCI_PWR_CNTL_STS) | V_PCH_XHCI_PWR_CNTL_STS_PWR_STS_D3;    
		        MmioWrite32(XhciPciMmBase + R_PCH_XHCI_PWR_CNTL_STS, Buffer32);
		        BOOT_SCRIPT_S3_MEM_WRITE_MACRO( gBootScriptSave, \
                                          EfiBootScriptWidthUint32, \
                                          XhciPciMmBase + R_PCH_XHCI_PWR_CNTL_STS, \
                                          1, \
                                          &Buffer32 );
        
		        ///
		        /// Program function disable bit.
		        ///    
		        Buffer32 = MmioRead32(PmcBase + R_PCH_PMC_FUNC_DIS) | B_PCH_PMC_FUNC_DIS_USH;   
		        MmioWrite32(PmcBase + R_PCH_PMC_FUNC_DIS, Buffer32);
		        BOOT_SCRIPT_S3_MEM_WRITE_MACRO( gBootScriptSave, \
                                          EfiBootScriptWidthUint32, \
                                          PmcBase + R_PCH_PMC_FUNC_DIS, \
                                          1, \
                                          &Buffer32 );
		        Buffer32 = MmioRead32(PmcBase + R_PCH_PMC_FUNC_DIS2) | B_PCH_PMC_FUNC_DIS2_USH_SS_PHY;  
		        MmioWrite32(PmcBase + R_PCH_PMC_FUNC_DIS2, Buffer32);
		        BOOT_SCRIPT_S3_MEM_WRITE_MACRO( gBootScriptSave, \
                                          EfiBootScriptWidthUint32, \
                                          PmcBase + R_PCH_PMC_FUNC_DIS2, \
                                          1, \
                                          &Buffer32 );
		      } 
//EIP160754 <<		  
		    }
	  }
}
#endif //DISABLE_USB_CONTROLLER_WHEN_DISBALE_ALL_PORT
//EIP138173 <<

//EIP142372 >>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SmbusReadyCallback
//
// Description: This function can set SMbus register after PciBus driver.             
//
// Input:       Event   - Event of callback
//              Context - Context of callback.
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SmbusReadyCallback (
    IN EFI_EVENT      Event,
    IN VOID           *Context
)
{
    SmbusBootScriptSave();
    pBS->CloseEvent(Event);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SmbusBootScriptSave
//
// Description: This function can set SMbus register.             
//
// Input:       None
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SmbusBootScriptSave()
{
    EFI_STATUS                      Status;
    UINT8                           Buffer8;
    UINT16                          Buffer16;
    UINT32                          Buffer32;
	     
    Status = pBS->LocateProtocol (
                    &gEfiS3SaveStateProtocolGuid,
                    NULL,
                    &gBootScriptSave
                    );
    ASSERT_EFI_ERROR (Status);

    MmioWrite8(SMBUS_REG(R_COMMAND), 0x03);
    Buffer8 = MmioRead8(SMBUS_REG(R_COMMAND));
    BOOT_SCRIPT_S3_MEM_WRITE_MACRO (
            gBootScriptSave, \
            EfiBootScriptWidthUint8, \
            SMBUS_BUS_DEV_FUN + R_COMMAND, \
            1, \
            &Buffer8 \
            );	    

    Buffer32 = MmioRead32(SMBUS_REG(R_PCH_SMBUS_BAR0));	    
    if(Buffer32 != 0xFFFF && Buffer32 != 0x0000)
    {
	    BOOT_SCRIPT_S3_MEM_WRITE_MACRO (
			    gBootScriptSave, \
			    EfiBootScriptWidthUint32, \
			    SMBUS_BUS_DEV_FUN + R_PCH_SMBUS_BAR0, \
			    1, \
			    &Buffer32 \
            	    );
    }

    Buffer32 = MmioRead32(SMBUS_REG(R_PCH_SMBUS_BAR1));	    
    if(Buffer32 != 0xFFFF && Buffer32 != 0x0000)
    {
	    BOOT_SCRIPT_S3_MEM_WRITE_MACRO (
			    gBootScriptSave, \
			    EfiBootScriptWidthUint32, \
			    SMBUS_BUS_DEV_FUN + R_PCH_SMBUS_BAR1, \
			    1, \
			    &Buffer32 \
            	    );
    }

	    
    Buffer16 = MmioRead16(SMBUS_REG(R_BASE_ADDRESS));	    
    if(Buffer16 != 0xFFFF && Buffer16 != 0x0000)
    {
	    BOOT_SCRIPT_S3_MEM_WRITE_MACRO (
			    gBootScriptSave, \
			    EfiBootScriptWidthUint16, \
			    SMBUS_BUS_DEV_FUN + R_BASE_ADDRESS, \
			    1, \
			    &Buffer16 \
            	    );
    }
}
//EIP142372 <<

//CSP20140123 >>
//EIP142393 >>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SpiPhaseInit
//
// Description: This function is a a hook for Spi Smm phase specific initialization.             
//
// Input:       Event   - Event of callback
//              Context - Context of callback.
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
SbPchCallbackReadyToBoot (
	    IN EFI_EVENT      Event,
	    IN VOID           *Context
)
{
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo;
  EFI_STATUS Status;
  UINT8   i;
  UINT8   Buffer8 = 0;
  UINT16  Buffer16 = 0;
  UINT32  Buffer32 = 0;

  static BOOLEAN BootScriptSaved = FALSE;
  if (BootScriptSaved) return;

  TRACE((TRACE_ALWAYS, "[[ SbPchCallbackReadyToBoot() Start. ]]\n"));

  Status = pBS->LocateProtocol (
                  &gEfiS3SaveStateProtocolGuid,
                  NULL,
                  &gBootScriptSave
                  );
  ASSERT_EFI_ERROR (Status); 
  
  Status = pBS->LocateProtocol(
               &gEfiPciRootBridgeIoProtocolGuid,
               NULL,
               &PciRootBridgeIo
           );
  if (EFI_ERROR(Status)) return;

  for (i = 0; i < sizeof(gSbPciRegistersSave) / sizeof(BOOT_SCRIPT_PCI_REGISTER_SAVE); ++i)
  {
	  if(gSbPciRegistersSave[i].Address == 0x00) break;

	  PciRootBridgeIo->Pci.Read(
          PciRootBridgeIo,
          gSbPciRegistersSave[i].Width,
          gSbPciRegistersSave[i].Address,
          1,
          &gSbPciRegistersSave[i].Value
      );
      BOOT_SCRIPT_S3_PCI_CONFIG_WRITE_MACRO(
          gBootScript,
          gSbPciRegistersSave[i].Width,
          gSbPciRegistersSave[i].Address,
          1,
          &gSbPciRegistersSave[i].Value
      );
  }

  for (i = 0; i < sizeof(gSbMmioRegistersSave) / sizeof(BOOT_SCRIPT_PCI_REGISTER_SAVE); ++i)
  {
      switch (gSbMmioRegistersSave[i].Width)
      {
      	  case (EfiBootScriptWidthUint32):
      		gSbMmioRegistersSave[i].Value = MmioRead32(gSbMmioRegistersSave[i].Address);
      	    break;
      	  case (EfiBootScriptWidthUint16):
			gSbMmioRegistersSave[i].Value = MmioRead16(gSbMmioRegistersSave[i].Address);
    	    break;
      	  case (EfiBootScriptWidthUint8):
			gSbMmioRegistersSave[i].Value = MmioRead8(gSbMmioRegistersSave[i].Address); 
    	    break;
      	  default:
      		gSbMmioRegistersSave[i].Value = MmioRead32(gSbMmioRegistersSave[i].Address);  
      	    break;
      }
      BOOT_SCRIPT_S3_MEM_WRITE_MACRO(
    	  gBootScriptSave,
          gSbMmioRegistersSave[i].Width,
          gSbMmioRegistersSave[i].Address,
          1,
          &gSbMmioRegistersSave[i].Value
      );
  }

  BootScriptSaved = TRUE;

  TRACE((TRACE_ALWAYS, "[[ SbPchCallbackReadyToBoot() Done. ]]\n"));

  pBS->CloseEvent(Event);
}
//EIP142393 <<
//CSP20140123 <<

#if CRID_SUPPORT
//EIP150551 >>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: UpdateSmbios136Table
//
// Description: Build and fill SmBios type 136 for CRID.
//
// Input:       EFI_EVENT   - Event,
//              VOID        - *Context
//
// Output:      EFI_STATUS  - EFI_SUCCESS.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
UpdateSmbios136Table(
    IN EFI_EVENT    Event,
    IN VOID         *Context
)
{
    EFI_STATUS                        Status;
    EFI_MISC_OEM_TYPE_136             Smbios136;
    AMI_SMBIOS_PROTOCOL               *AmiSmbiosProtocol = NULL;
    SB_SETUP_DATA                     PchPolicyData;

    // Get the value of the SB Setup data.
    GetSbSetupData ((VOID*)gRT, &PchPolicyData, FALSE);
    
    Status = pBS->LocateProtocol(&gAmiSmbiosProtocolGuid, NULL, (VOID **)&AmiSmbiosProtocol);
    if (EFI_ERROR(Status)) return;
    
    //Clear all data
    pBS->SetMem (&Smbios136, sizeof(EFI_MISC_OEM_TYPE_136), 0);

    Smbios136.Type = 136; //0x88
    Smbios136.Length = 6;
    Smbios136.Handle = 0xFFFF;

    //
    //OEM can change the value.
    //
    if (PchPolicyData.PchCrid != 0)
    	Smbios136.OemInfo = 0x5a5a; 
 
    Status = AmiSmbiosProtocol->SmbiosAddStructure((UINT8 *)&Smbios136, sizeof(EFI_MISC_OEM_TYPE_136));
    pBS->CloseEvent(Event);
}
//EIP150551 <<
#endif

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
