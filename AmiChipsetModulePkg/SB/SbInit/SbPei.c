//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.            **
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
// $Header: $
//
// $Revision: $
//
// $Date: $
//
//*****************************************************************************


//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        SbPei.c
//
// Description: This file contains code for Template Southbridge initialization
//              in the PEI stage
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>


// Module specific Includes
#include <Efi.h>
#include <Pei.h>
#include <token.h>
#include <AmiPeiLib.h>
#include <Library/SbCspLib.h>
#include <Hob.h>
#include <Sb.h>
#include <Library/SbPolicy.h>
#include "PchAccess.h"
#include <Library\AmiChipsetIoLib.h>
#include <PchRegs\PchRegsSata.h> //EIP131059
#include <Guid/Rtc.h> //EIP134949 

// Produced/used PPI interfaces
#include <Ppi/PciCfg2.h>
#include <Ppi/SbPpi.h>
#include <Ppi/CpuIo.h>
#include <Ppi/CspLibPpi.h>
#include <Ppi/SmmControl.h>
#include <Ppi/AtaController.h>
#include <Guid/PlatformInfo.h>
// Produced PPIs
#include <Library/HobLib.h>
#include <PlatformGpio.h>
//EIP143188 >>
#if REDUCE_PRE_BB_SIZE_TO_124K == 1
#include <Ppi/Capsule.h> //(EIP143188+)
#endif //#if REDUCE_PRE_BB_SIZE_TO_124K == 1
//EIP143188 <<

#include <Library/PchPlatformLib.h> 

#if CMOS_MANAGER_SUPPORT == 1
#include <CmosAccess.h>
#include <SspTokens.h>
#endif //#if CMOS_MANAGER_SUPPORT == 1
AMI_GPIO_INIT_TABLE_STRUCT DefaultGpioTable[] = {
#include <SbGpio.h>
};
#include <AdjustGpio.h> //CSP20130930

// GUID Definitions

// Portable Constants

// Function Prototypes
EFI_STATUS
UpdateBootMode(
    IN EFI_PEI_SERVICES     **PeiServices,
    IN EFI_PEI_CPU_IO_PPI   *CpuIo,
    IN EFI_PEI_PCI_CFG2_PPI *PciCfg
);

BOOLEAN IsRecovery(
    IN EFI_PEI_SERVICES     **PeiServices,
    IN EFI_PEI_PCI_CFG2_PPI *PciCfg,
    IN EFI_PEI_CPU_IO_PPI   *CpuIo
);

EFI_STATUS EnableAtaChannel( //EIP131059
    IN EFI_PEI_SERVICES         **PeiServices,
    IN PEI_ATA_CONTROLLER_PPI   *This,
    IN UINT8                    ChannelMask
);

EFI_STATUS CountTime(
    IN  UINTN   DelayTime,
    IN  UINT16  BaseAddr
);

VOID InitGpio(
    IN EFI_PEI_SERVICES     **PeiServices,
    IN EFI_PEI_CPU_IO_PPI   *CpuIo
);

EFI_STATUS SbPeiInitAfterMemInstalled(
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
    IN VOID                         *InvokePpi
);

EFI_STATUS ProgramSbRegBeforeEndofPei(
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
    IN VOID                         *InvokePpi
);

EFI_STATUS
MultiPlatformInfoInit(
    IN CONST EFI_PEI_SERVICES          **PeiServices,
    IN OUT EFI_PLATFORM_INFO_HOB *PlatformInfoHob
);

//EIP132001 >>
EFI_STATUS
InitPchUsb(
    IN CONST EFI_PEI_SERVICES       **PeiServices,
    IN SB_SETUP_DATA                *PchPolicyData
);
//EIP132001 <<

#if KBC_SUPPORT && Recovery_SUPPORT
VOID ResetKbc (
    IN EFI_PEI_SERVICES     **PeiServices,
    IN EFI_PEI_CPU_IO_PPI   *CpuIo,
    IN EFI_PEI_PCI_CFG2_PPI  *PciCfg );
#endif

// PPI interface definition
// PPI produced by this PEIM

#if SB_RESET_PPI_SUPPORT
#include <Ppi/Reset.h>

VOID SBLib_ResetSystem(
    IN EFI_RESET_TYPE ResetType
);

EFI_STATUS SbPeiResetSystem(
    IN EFI_PEI_SERVICES **PeiServices
);


static  EFI_PEI_RESET_PPI mResetPpi = { SbPeiResetSystem };
#endif  //SB_RESET_PPI_SUPPORT

#if SB_STALL_PPI_SUPPORT
#include "Ppi/Stall.h"

EFI_STATUS SbPeiStall(
    IN EFI_PEI_SERVICES     **PeiServices,
    IN EFI_PEI_STALL_PPI    *This,
    IN UINTN                MicroSeconds
);


// Constants related to template Stall code
#define TIMER_RESOLUTION  1

static EFI_PEI_STALL_PPI mStallPpi = {
    TIMER_RESOLUTION,
    SbPeiStall
};
#endif  //SB_STALL_PPI_SUPPORT


// PPI Definition
static  AMI_PEI_SBINIT_POLICY_PPI mAmiPeiSbInitPolicyPpi = { TRUE };

static PEI_ATA_CONTROLLER_PPI mAtaControllerPpi = {
    EnableAtaChannel //EIP131059
};

#if SMM_SUPPORT
EFI_STATUS
SbPeiActivateSmi(
    IN EFI_PEI_SERVICES     **PeiServices,
    IN PEI_SMM_CONTROL_PPI  *This,
    IN OUT INT8             *ArgumentBuffer OPTIONAL,
    IN OUT UINTN            *ArgumentBufferSize OPTIONAL,
    IN BOOLEAN              Periodic OPTIONAL,
    IN UINTN                ActivationInterval OPTIONAL
);

EFI_STATUS SbPeiDeactivateSmi(
    IN EFI_PEI_SERVICES     **PeiServices,
    IN PEI_SMM_CONTROL_PPI  *This,
    IN BOOLEAN              Periodic OPTIONAL
);

static PEI_SMM_CONTROL_PPI mSmmControlPpi = {
    SbPeiActivateSmi,
    SbPeiDeactivateSmi
};
#endif  //SMM_SUPPORT


// PPI that are installed
//EIP132198 >>
#if SB_STALL_PPI_SUPPORT
static EFI_PEI_PPI_DESCRIPTOR mBeforeBootModePpiList[] = {
    { EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST, \
      &gEfiPeiStallPpiGuid, &mStallPpi },
};
#endif
//EIP132198 <<

static EFI_PEI_PPI_DESCRIPTOR mBootModePpi[] = {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiPeiMasterBootModePpiGuid, NULL
};

static EFI_PEI_PPI_DESCRIPTOR mRecoveryModePpi[] = {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiPeiBootInRecoveryModePpiGuid, NULL
};

static EFI_PEI_PPI_DESCRIPTOR mPpiList[] = {
    {EFI_PEI_PPI_DESCRIPTOR_PPI, &gPeiAtaControllerPpiGuid, &mAtaControllerPpi},
#if SMM_SUPPORT
    {EFI_PEI_PPI_DESCRIPTOR_PPI, &gPeiSmmControlPpiGuid, &mSmmControlPpi},
#endif
#if SB_RESET_PPI_SUPPORT
    {EFI_PEI_PPI_DESCRIPTOR_PPI, &gEfiPeiResetPpiGuid, &mResetPpi},
#endif
//EIP132198 (-) >>
//#if SB_STALL_PPI_SUPPORT
//    {EFI_PEI_PPI_DESCRIPTOR_PPI, &gEfiPeiStallPpiGuid, &mStallPpi},
//#endif
//EIP132198 (-) <<
    {
        (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
        &gAmiPeiSbInitPolicyGuid, &mAmiPeiSbInitPolicyPpi
    }

};

// PPI that are notified
static EFI_PEI_NOTIFY_DESCRIPTOR mNotifyList[] = {
    {
        EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | \
        EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST, \
        &gEfiEndOfPeiSignalPpiGuid, ProgramSbRegBeforeEndofPei
    },
};

static EFI_PEI_NOTIFY_DESCRIPTOR MemInstalledNotifyList[] = {
    {
        EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | \
        EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST, \
        &gEfiPeiMemoryDiscoveredPpiGuid, SbPeiInitAfterMemInstalled
    },
};

#define SB_DEVICE_NUMBER        0
#define TempBusDevFuncNo        (PEI_PCI_CFG_ADDRESS(0, SB_DEVICE_NUMBER, 0, 0))

extern AMI_PEI_PCI_INIT_TABLE_STRUCT stSBH2P_PCIInitTable [];
extern UINT16 wSBH2P_PCIInitTableSize;

EFI_STATUS PlatformPchInit(
    IN CONST EFI_PEI_SERVICES       **PeiServices,
    IN SB_SETUP_DATA                *PchPolicyData
);

EFI_STATUS CountTime(
    IN  UINTN DelayTime,
    IN  UINT16 BaseAddr
);

//CSP20130712 >>
#if MMC_SUPPORT
EFI_STATUS
ConfigureSoCGpio (
    IN CONST EFI_PEI_SERVICES       **PeiServices,    
    IN SB_SETUP_DATA                *PchPolicyData
  );
#endif
//CSP20130712 <<

// Function Definition
//EIP129481 (-) >>
/*
BOOLEAN
IsRtcUipAlwaysSet(
    IN CONST EFI_PEI_SERVICES       **PeiServices
)
*/
/*++

Routine Description:


Arguments:


Returns:


--*/
/*
{

    EFI_PEI_STALL_PPI *StallPpi;
    UINTN             Count;
#if CMOS_MANAGER_SUPPORT == 1
    UINT8             CmosValue;
#endif

    (**PeiServices).LocatePpi(PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, &StallPpi);

    for(Count = 0; Count < 500; Count++) {  // Maximum waiting approximates to 1.5 seconds (= 3 msec * 500)
        CmosValue = CmosReadByte(SB_PCH_RTC_REGISTERA);
        if((CmosValue & B_PCH_RTC_REGISTERA_UIP) == 0) {
            return FALSE;
        }

        StallPpi->Stall(PeiServices, StallPpi, 3000);
    }

    return TRUE;
}
*/

/*
EFI_STATUS
RtcPowerFailureHandler(
    IN CONST EFI_PEI_SERVICES       **PeiServices
)
/*++

Routine Description:


Arguments:


Returns:


--*/
/*
{

    UINT16          DataUint16;
    UINT8           DataUint8;
    BOOLEAN         RtcUipIsAlwaysSet;
    //
    // VLV BIOS Specification 0.6.2 - Section 18.4.3, "Power Failure Consideration"
    //
    // When the RTC_PWR_STS bit is set, it indicates that the RTCRST# signal went low.
    // Software should clear this bit. Changing the RTC battery sets this bit.
    // System BIOS should reset CMOS to default values if the RTC_PWR_STS bit is set.
    //
    // VLV BIOS Specification Update 0.6.2
    // The System BIOS should execute the sequence below if the RTC_PWR_STS bit is set before memory initialization.
    // This will ensure that the RTC state machine has been initialized.
    //  1.  If the RTC_PWR_STS bit is set which indicates a new coin-cell battery insertion or a battery failure,
    //        steps 2 through 5 should be executed.
    //  2.  Set RTC Register 0x0A[6:4] to '110' or '111'.
    //  3.  Set RTC Register 0x0B[7].
    //  4.  Set RTC Register 0x0A[6:4] to '010'.
    //  5.  Clear RTC Register 0x0B[7].
    //
    DataUint16        = MmioRead16(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1);
    RtcUipIsAlwaysSet = IsRtcUipAlwaysSet(PeiServices);
    if((DataUint16 & B_PCH_PMC_GEN_PMCON_RTC_PWR_STS) || (RtcUipIsAlwaysSet)) {
        //
        // Execute the sequence below. This will ensure that the RTC state machine has been initialized.
        //
        // Step 1.
        // BIOS clears this bit by writing a '0' to it.
        //
        if(DataUint16 & B_PCH_PMC_GEN_PMCON_RTC_PWR_STS) {
            DataUint16 &= ~B_PCH_PMC_GEN_PMCON_RTC_PWR_STS;
            MmioWrite16((PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1), DataUint16);

            //
            // Set to invalid date in order to reset the time to
            // BIOS build time later in the boot (SBRUN.c file).
            //
            CmosWriteByte(SB_PCH_RTC_YEAR, 0xFF);
            CmosWriteByte(SB_PCH_RTC_MONTH, 0xFF);
            CmosWriteByte(SB_PCH_RTC_DAYOFMONTH, 0xFF);
            CmosWriteByte(SB_PCH_RTC_DAYOFWEEK, 0xFF);
        }

        //
        // Step 2.
        // Set RTC Register 0Ah[6:4] to '110' or '111'.
        //
        CmosWriteByte(SB_PCH_RTC_REGISTERA, (V_PCH_RTC_REGISTERA_DV_DIV_RST1 | V_PCH_RTC_REGISTERA_RS_976P5US));

        //
        // Step 3.
        // Set RTC Register 0Bh[7].
        //
        DataUint8 = CmosReadByte(SB_PCH_RTC_REGISTERB);
        DataUint8 |= B_PCH_RTC_REGISTERB_SET;
        CmosWriteByte(SB_PCH_RTC_REGISTERB, DataUint8);

        //
        // Step 4.
        // Set RTC Register 0Ah[6:4] to '010'.
        //
        CmosWriteByte(SB_PCH_RTC_REGISTERA, (V_PCH_RTC_REGISTERA_DV_NORM_OP | V_PCH_RTC_REGISTERA_RS_976P5US));

        //
        // Step 5.
        // Clear RTC Register 0Bh[7].
        //
        DataUint8 = CmosReadByte(SB_PCH_RTC_REGISTERB);
        DataUint8 &= (UINT8)(~B_PCH_RTC_REGISTERB_SET);
        CmosWriteByte(SB_PCH_RTC_REGISTERB, DataUint8);
    }

    return EFI_SUCCESS;
}
*/
//EIP129481 (-) <<

//EIP136936 >>
EFI_STATUS
ConfigureUSBULPI (
  VOID)
{
    //Configure USB ULPI
    //USB_ULPI_0_CLK
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x338, (UINT32)~(0x7), (UINT32) (GPI));
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x330, (UINT32)~(0x187), (UINT32) (0x101));

    //USB_ULPI_0_DATA0
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x388, (UINT32)~(0x7), (UINT32) (GPI));
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x380, (UINT32)~(0x187), (UINT32) (0x101));

    //USB_ULPI_0_DATA1
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x368, (UINT32)~(0x7), (UINT32) (GPI));
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x360, (UINT32)~(0x187), (UINT32) (0x101));

    //USB_ULPI_0_DATA2
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x318, (UINT32)~(0x7), (UINT32) (GPI));
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x310, (UINT32)~(0x187), (UINT32) (0x101));

    //USB_ULPI_0_DATA3
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x378, (UINT32)~(0x7), (UINT32) (GPI));
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x370, (UINT32)~(0x187), (UINT32) (0x101));

    //USB_ULPI_0_DATA4
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x308, (UINT32)~(0x7), (UINT32) (GPI));
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x300, (UINT32)~(0x187), (UINT32) (0x101));

    //USB_ULPI_0_DATA5
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x398, (UINT32)~(0x7), (UINT32) (GPI));
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x390, (UINT32)~(0x187), (UINT32) (0x101));

    //USB_ULPI_0_DATA6
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x328, (UINT32)~(0x7), (UINT32) (GPI));
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x320, (UINT32)~(0x187), (UINT32) (0x101));

    //USB_ULPI_0_DATA7
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x3a8, (UINT32)~(0x7), (UINT32) (GPI));
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x3a0, (UINT32)~(0x187), (UINT32) (0x101));

    //USB_ULPI_0_DIR
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x348, (UINT32)~(0x7), (UINT32) (GPI));
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x340, (UINT32)~(0x187), (UINT32) (0x81));

    //USB_ULPI_0_NXT
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x358, (UINT32)~(0x7), (UINT32) (GPI));
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x350, (UINT32)~(0x187), (UINT32) (0x101));

    //USB_ULPI_0_STP
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x3b8, (UINT32)~(0x7), (UINT32) (GPI));
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x3b0, (UINT32)~(0x187), (UINT32) (0x81));

    //USB_ULPI_0_REFCLK
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x288, (UINT32)~(0x7), (UINT32) (GPI));
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x280, (UINT32)~(0x187), (UINT32) (0x101));

    return EFI_SUCCESS;
}

EFI_STATUS
DisableRTD3 (
  VOID)
{
    //Disable RTD3
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x210, (UINT32)~(0x0f000007), (UINT32) (0x00));
    MmioAndThenOr32 (IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x1e0, (UINT32)~(0x0f000007), (UINT32) (0x00));

    return EFI_SUCCESS;
}
//<< EIP136936

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SbPeiInit
//
// Description: This function is the entry point for this PEI. This function
//              initializes the chipset SB
//
// Input:       IN EFI_FFS_FILE_HEADER *FfsHeader - Pointer to the FFS file header
//              IN EFI_PEI_SERVICES **PeiServices - Pointer to the PEI services table
//
// Output:      Return Status based on errors that occurred while waiting for
//              time to expire.
//
// Notes:       This function should initialize South Bridge for memory detection.
//              Install AMI_PEI_SBINIT_POLICY_PPI to indicate that SB Init PEIM
//              is installed
//              Following things can be done at this point:
//                  - Enabling top of 4GB decode for full flash ROM
//                  - Programming South Bridge ports to enable access to South
//                    Bridge and other I/O bridge access
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SbPeiInit(
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices
)
{
    EFI_STATUS                  Status;
    EFI_PEI_PCI_CFG2_PPI        *PciCfg;
    EFI_PEI_CPU_IO_PPI          *CpuIo;
    SB_SETUP_DATA               PchPolicyData;
    EFI_PLATFORM_INFO_HOB       *PlatformInfo;
    EFI_PEI_HOB_POINTERS        Hob;
    UINT32                      Buffer32;

    //Report Progress code
    PEI_PROGRESS_CODE(PeiServices, PEI_CAR_SB_INIT);

    // Clear SMM BWP bit since it's power-on value is 1.
    // Set SPI Read Configuration to Prefetch Disable, Cache Enable.
    MmioAnd8((UINTN)(SPI_BASE_ADDRESS + R_PCH_SPI_BCR), (UINT8) ~(B_PCH_SPI_BCR_SMM_BWP | B_PCH_SPI_BCR_SRC)); //EIP142393
    
    // Get pointer to the PCI config & CPU I/O PPI
    PciCfg = (*PeiServices)->PciCfg;
    CpuIo = (*PeiServices)->CpuIo;

    // Get the value of the SB Setup data.
    GetSbSetupData((VOID *)PeiServices, &PchPolicyData, TRUE);

    Hob.Raw = GetFirstGuidHob(&gEfiPlatformInfoGuid);
    if(Hob.Raw  != NULL) {
        PlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);
    }

//EIP132198 >>
#if SB_STALL_PPI_SUPPORT
    // Install the SB Stall PPI before update Boot Mode
    Status = (*PeiServices)->InstallPpi( PeiServices, \
                                         &mBeforeBootModePpiList[0] );
    ASSERT_PEI_ERROR( PeiServices, Status );
#endif

    UpdateBootMode(PeiServices, CpuIo, PciCfg);

#if defined(PROGRAM_GPIO_SUPPORT)&&PROGRAM_GPIO_SUPPORT
    // Program GPIO
    InitGpio(PeiServices, CpuIo);
#endif
//EIP132198 <<

//EIP136936 >>
    //
    // Bakersport Board specific
    //
    if (PlatformInfo->BoardId == BOARD_ID_BS_RVP) {
      ConfigureUSBULPI();
      DisableRTD3();
    }
//<< EIP136936

    //
    // RTC power failure handling
    //
    // fRC 0.52 had implement that in function "PchMiscEarlyInit".
//    RtcPowerFailureHandler (PeiServices);

    if(PchPolicyData.LastState != 0) {
      Buffer32 = READ_MEM32 (PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1);    // PMC_BASE_ADDRESS offset 0x20
      WRITE_MEM32(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1, Buffer32 & ~BIT00);
    }
    
    //
    // Setting 8254
    // Program timer 1 as refresh timer
    //
    IoWrite8(0x43, 0x54);
    IoWrite8(0x41, 0x12);

    // Do basic PCH init
    Status = PlatformPchInit(PeiServices, &PchPolicyData);
    ASSERT_PEI_ERROR(PeiServices, Status);

    // Initialize PlatformInfo HOB
    MultiPlatformInfoInit(PeiServices, PlatformInfo);

//EIP132198 >>
    // Install the SB Init Policy PPI
    Status = (*PeiServices)->InstallPpi(PeiServices, &mPpiList[0]);
    ASSERT_PEI_ERROR(PeiServices, Status);
//EIP132198 <<
	
    // Setup a SBPEI entry after PEI permantent memory be installed
    Status = (*PeiServices)->NotifyPpi(PeiServices, MemInstalledNotifyList);
    ASSERT_PEI_ERROR(PeiServices, Status);

    return EFI_SUCCESS;
}


BOOLEAN
GetSleepTypeAfterWakeup(
    IN  CONST EFI_PEI_SERVICES          **PeiServices,
    OUT UINT16                    *SleepType
)
/*++

Routine Description:

  Get sleep type after wakeup

Arguments:

  PeiServices       Pointer to the PEI Service Table.
  SleepType         Sleep type to be returned.

Returns:

  TRUE              A wake event occured without power failure.
  FALSE             Power failure occured or not a wakeup.

--*/
{
    UINT16  Pm1Sts;
    UINT16  Pm1Cnt;
    UINT16  GenPmCon1;
    //
    // VLV BIOS Specification 0.6.2 - Section 18.4, "Power Failure Consideration"
    //
    // When the SUS_PWR_FLR bit is set, it indicates the SUS well power is lost.
    // This bit is in the SUS Well and defaults to 1’b1 based on RSMRST# assertion (not cleared by any type of reset).
    // System BIOS should follow cold boot path if SUS_PWR_FLR (PBASE + 0x20[14]),
    // GEN_RST_STS (PBASE + 0x20[9]) or PWRBTNOR_STS (ABASE + 0x00[11]) is set to 1’b1
    // regardless of the value in the SLP_TYP (ABASE + 0x04[12:10]) field.
    //
    GenPmCon1 = MmioRead16(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1);
    //
    // Read the ACPI registers
    //
    Pm1Sts  = IoRead16(PM_BASE_ADDRESS + R_PCH_ACPI_PM1_STS);
    Pm1Cnt  = IoRead16(PM_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT);

    if((GenPmCon1 & (B_PCH_PMC_GEN_PMCON_SUS_PWR_FLR | B_PCH_PMC_GEN_PMCON_GEN_RST_STS)) ||
            (Pm1Sts & B_PCH_ACPI_PM1_STS_PRBTNOR)) {
        // If power failure indicator, then don't attempt s3 resume.
        // Clear PM1_CNT of S3 and set it to S5 as we just had a power failure, and memory has
        // lost already.  This is to make sure no one will use PM1_CNT to check for S3 after
        // power failure.
        if((Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP) == V_PCH_ACPI_PM1_CNT_S3) {
            Pm1Cnt = ((Pm1Cnt & ~B_PCH_ACPI_PM1_CNT_SLP_TYP) | V_PCH_ACPI_PM1_CNT_S5);
            IoWrite16(PM_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT, Pm1Cnt);
        }
        // Clear Wake Status (WAK_STS)
        //
        IoWrite16((PM_BASE_ADDRESS + R_PCH_ACPI_PM1_STS), B_PCH_ACPI_PM1_STS_WAK);
    }
    //
    // Get sleep type if a wake event occurred and there is no power failure
    //
    if((Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP) == V_PCH_ACPI_PM1_CNT_S3) {
        *SleepType = Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP;
        return TRUE;
    } else if((Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP) == V_PCH_ACPI_PM1_CNT_S4) {
        *SleepType = Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP;
        return TRUE;
    }
    return FALSE;
}

//EIP134949 >>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IsCmosBad
//
// Description: This function determines CMOS data is available.
//
// Input:       PeiServices - Pointer to the Pei Services function and data
//                            structure.
//
// Output:      TRUE - CMOS data is bad
//              FALSE - CMOS DATA is available
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN IsCmosBad (
    VOID
)
{
	return (MmioRead8 (PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1) & B_PCH_PMC_GEN_PMCON_RTC_PWR_STS) ? TRUE : FALSE; //EIP137659
}
//EIP134949 <<

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UpdateBootMode
//
// Description: This function determines the boot mode of the system.  After
//              the correct boot mode has been determined, the PEI Service
//              function SetBootMode is called and then the
//              MasterBootModePpi is installed
//
// Input:       IN EFI_PEI_SERVICES **PeiServices - Pointer to the Pei Services data structure
//              IN EFI_PEI_CPU_IO_PPI *CpuIo - Pointer to the CPU I/O PPI
//              IN EFI_PEI_PCI_CFG2_PPI *PciCfg - Pointer to the PCI Config 2 PPI
//
// Output:      EFI_SUCCESS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UpdateBootMode(
    IN EFI_PEI_SERVICES     **PeiServices,
    IN EFI_PEI_CPU_IO_PPI   *CpuIo,
    IN EFI_PEI_PCI_CFG2_PPI *PciCfg
)
{
    EFI_BOOT_MODE           SbBootMode;
    UINT16                  SleepType;
    //(EIP143188+) >>
    #if REDUCE_PRE_BB_SIZE_TO_124K == 1
    PEI_CAPSULE_PPI     *Capsule;
    EFI_STATUS Status;
    #endif
    //(EIP143188+) <<
    
	//EIP132940 >>
    (*PeiServices)->GetBootMode(PeiServices, &SbBootMode);

    if (SbBootMode  == BOOT_IN_RECOVERY_MODE){
      PEI_TRACE((-1, PeiServices, "Boot mode = BOOT_IN_RECOVERY_MODE"));
    } else {
      // Check for changes in the possible boot modes.  This should be made in
      //  prioritized order.  At the end of this function the boot mode is
      //  determined.  The EFI_BOOT_MODE is defined in the PEI Spec
      SbBootMode = BOOT_WITH_FULL_CONFIGURATION;
      PEI_TRACE((-1, PeiServices, "Boot mode = BOOT_WITH_FULL_CONFIGURATION"));
      // Check for recovery mode
      #if KBC_SUPPORT && Recovery_SUPPORT
        ResetKbc(PeiServices,  CpuIo, PciCfg);
      #endif
//    if(IsRecovery(PeiServices, PciCfg, CpuIo)) {
//        SbBootMode = BOOT_IN_RECOVERY_MODE;
//        PEI_TRACE((-1, PeiServices, "Boot mode = BOOT_IN_RECOVERY_MODE"));
//    } else {
        if(GetSleepTypeAfterWakeup(PeiServices, &SleepType)) {
            switch(SleepType) {
            case V_PCH_ACPI_PM1_CNT_S3:
                SbBootMode = BOOT_ON_S3_RESUME;
                PEI_TRACE((-1, PeiServices, "Boot mode = BOOT_ON_S3_RESUME"));

            //(EIP143188+) >>
            #if REDUCE_PRE_BB_SIZE_TO_124K == 1
            // Because the SbPei is executed after Memory init and IsSecRecoveryPEI module while REDUCE_PRE_BB_SIZE_TO_124K == 1,
            // we need change boot mode here for Capsule Update.
            //
            // Determine if we're in capsule update mode
            //
            Status = (*PeiServices)->LocatePpi (PeiServices,
                                                &gPeiCapsulePpiGuid,
                                                0,
                                                NULL,
                                                (VOID **)&Capsule
                                                );
            if (Status == EFI_SUCCESS) {
              if (Capsule->CheckCapsuleUpdate ((EFI_PEI_SERVICES**)PeiServices) == EFI_SUCCESS) {
                  SbBootMode = BOOT_ON_FLASH_UPDATE;
                PEI_TRACE((-1,PeiServices, "Boot mode = BOOT_ON_FLASH_UPDATE"));
              }
            }
            #endif
            //(EIP143188+) <<
                break;

            case V_PCH_ACPI_PM1_CNT_S4:
                SbBootMode = BOOT_ON_S4_RESUME;
                PEI_TRACE((-1, PeiServices, "Boot mode = BOOT_ON_S4_RESUME"));
                break;

            case V_PCH_ACPI_PM1_CNT_S5:
                SbBootMode = BOOT_ON_S5_RESUME;
                PEI_TRACE((-1, PeiServices, "Boot mode = BOOT_ON_S5_RESUME"));
                break;
            } // switch (SleepType)
        }
        // Check for Safe Mode
//    }
      
	  //EIP134949 >>
      if (IsCmosBad()) {
        SbBootMode = BOOT_WITH_DEFAULT_SETTINGS;
        PEI_TRACE((-1,PeiServices,"Boot mode = BOOT_WITH_DEFAULT_SETTING\n"));
      }
	  //EIP134949 <<

      // Set the system BootMode
      (*PeiServices)->SetBootMode(PeiServices, SbBootMode);
    } // if (SbBootMode  == BOOT_IN_RECOVERY_MODE)
	//EIP132940 <<
   
    // Let everyone know that boot mode has been determined by installing the
    //  MasterBootMode PPI
    (*PeiServices)->InstallPpi(PeiServices, mBootModePpi);

//    if(SbBootMode == BOOT_IN_RECOVERY_MODE)  //Recovery Boot Mode PPI
//        (*PeiServices)->InstallPpi(PeiServices, mRecoveryModePpi);

    return EFI_SUCCESS;
}


#if SB_RESET_PPI_SUPPORT

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SbPeiResetSystem
//
// Description: This function is the reset call interface function published
//              by the reset PPI
//
// Input:       IN EFI_PEI_SERVICES **PeiServices - Pointer to the PEI services table
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SbPeiResetSystem(
    IN EFI_PEI_SERVICES **PeiServices
)
{

    SBLib_ResetSystem(COLD_RESET);

    // We should never get this far
    return EFI_SUCCESS;
}

#endif


#if SB_STALL_PPI_SUPPORT

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SbPeiStall
//
// Description: This function is used to stall the boot process (provides delay)
//              for specified number of microseconds
//
// Input:       IN EFI_PEI_SERVICES **PeiServices - Pointer to the PEI services table
//              IN EFI_PEI_STALL_PPI *This - Pointer to the Stall PPI
//              IN UINTN MicroSeconds - Time to wait for (in Micro seconds)
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SbPeiStall(
    IN EFI_PEI_SERVICES     **PeiServices,
    IN EFI_PEI_STALL_PPI    *This,
    IN UINTN                MicroSeconds
)
{
    EFI_STATUS              Status;

    //
    // Call Library function that is shared with Metronome
    // Architecture Protocol
    //
    Status = CountTime(MicroSeconds, PM_BASE_ADDRESS);

    return Status;
}

#endif

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ProgramLegacyGpio
//
// Description: Program legacy GPIO.
//
// Input:       PeiServices - Pointer to the PEI services table
//              CpuIo       - Pointer to the CPU I/O PPI
//
// Output:      VOID
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ProgramLegacyGpio(
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_CPU_IO_PPI           *CpuIo,
    IN UINT32                       GpioBase,
    IN AMI_GPIO_INIT_TABLE_STRUCT   GpioTable,
    OUT UINT32                      *GpioLvlValue) //EIP140031
{
//(P052813A+)>>
    EFI_STATUS                  Status = EFI_SUCCESS;
    UINT16                      Adjust = 0;
    UINT32                      Bit32Adj = 0;
    UINT32                      HGpioOffset = 0;
    
    if (GpioTable.GpioNo < 32) {
      HGpioOffset = 0;
      Adjust = 0;
    } else if (GpioTable.GpioNo < 64) {
      HGpioOffset = 0x20;
      Adjust = 32;
    } else if (GpioTable.GpioNo < 96) {
      HGpioOffset = 0x40;
      Adjust = 64;
    } else if (GpioTable.GpioNo < 128) {
      HGpioOffset = 0x60;
      Adjust = 96;
    }
    Bit32Adj = 1 << (GpioTable.GpioNo - Adjust);

    if (GpioTable.GpioCfg.Fileds.Gpi || GpioTable.GpioCfg.Fileds.Gpo) {
        IoOr32( GpioBase + GP_IOREG_USE_SEL + HGpioOffset, Bit32Adj );
        
        if (GpioTable.GpioCfg.Fileds.Gpi) {
            IoOr32( GpioBase + GP_IOREG_IO_SEL + HGpioOffset, Bit32Adj );
        } else {
            IoAnd32( GpioBase + GP_IOREG_IO_SEL + HGpioOffset, ~Bit32Adj );
			
            //EIP140031 >>
            //
            // SC/SUS Gpio Level X registers are Write Only, so reads to
            // this register location have no effect.
            //
            if (GpioTable.GpioCfg.Fileds.Gpod4H) {
                *GpioLvlValue |= Bit32Adj;
            } else {
                *GpioLvlValue &= ~Bit32Adj;
            }
            IoWrite32 (GpioBase + GP_IOREG_GP_LVL + HGpioOffset, *GpioLvlValue);
			//EIP140031 <<
        }
        
        if (GpioTable.GpioCfg.Fileds.TPE) {
            IoOr32( GpioBase + GP_IOREG_TPE + HGpioOffset, Bit32Adj);
        } else {
            IoAnd32( GpioBase + GP_IOREG_TPE + HGpioOffset, ~Bit32Adj );
        }
        
        if (GpioTable.GpioCfg.Fileds.TNE) {
            IoOr32( GpioBase + GP_IOREG_TNE + HGpioOffset, Bit32Adj );
        } else {
            IoAnd32( GpioBase + GP_IOREG_TNE + HGpioOffset, ~Bit32Adj );
        }
        
        if (GpioTable.GpioCfg.Fileds.TS) {
            IoOr32( GpioBase + GP_IOREG_TS + HGpioOffset, Bit32Adj );
        } else {
            IoAnd32( GpioBase + GP_IOREG_TS + HGpioOffset, ~Bit32Adj );
        }

        if ((GpioTable.GpioCfg.Dword & LEGACY_GPIO_SUS_OFFSET) == LEGACY_GPIO_SUS_OFFSET) {
            if (GpioTable.GpioCfg.Fileds.WE) {
                IoOr32( GpioBase + GP_IOREG_WE + HGpioOffset, Bit32Adj );
            } else {
                IoAnd32( GpioBase + GP_IOREG_WE + HGpioOffset, ~Bit32Adj );
            }
        }
    } else {
        IoAnd32( GpioBase + GP_IOREG_USE_SEL + HGpioOffset, ~Bit32Adj );
    }
//(P052813A+)<<
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ProgramGPIO
//
// Description: ProgramGPIO
//
// Input:       PeiServices - Pointer to the PEI services table
//              CpuIo       - Pointer to the CPU I/O PPI
//              GpioBase    - GPIO Base Address
//              GpioTable   - Pointer to the GPIO table.
//
// Output:      VOID
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ProgramGpio(
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_CPU_IO_PPI           *CpuIo,
    IN UINT32                       GpioBase,
    IN AMI_GPIO_INIT_TABLE_STRUCT   *GpioTable)
{
    UINT16                          Index = 0;
    UINT32                          OffsetConfig;
    UINT32                          mmio_conf0;
    UINT32                          mmio_padval;
    PAD_CONF0                       conf0_val;
    PAD_VAL                         pad_val;
    UINT32                          ScGpioLvlValue = 0; //EIP140031
    UINT32                          SusGpioLvlValue = 0; //EIP140031

//(P052813A+)>>
    while(GpioTable[Index].GpioNo != 0xffff) {
        switch(GpioTable[Index].GpioCfg.Fileds.Offset) {
        case 1:
            OffsetConfig = GPIO_NCORE_OFFSET + Gpio_Platform_Offset[GpioTable[Index].GpioNo].offset * 16;
            PEI_TRACE((-1, PeiServices, "0x%08x, ", GpioTable[Index].GpioCfg.Dword));
            PEI_TRACE((-1, PeiServices, "Nc GPIO: %03d, ", GpioTable[Index].GpioNo));
            break;
        case 2:
            OffsetConfig = GPIO_SCORE_OFFSET + Gpio_Platform_Offset[GpioTable[Index].GpioNo + ScStartOffset].offset * 16;
            PEI_TRACE((-1, PeiServices, "0x%08x, ", GpioTable[Index].GpioCfg.Dword));
            PEI_TRACE((-1, PeiServices, "Sc GPIO: %03d, ", GpioTable[Index].GpioNo));
            break;
        case 3:
            OffsetConfig = GPIO_SSUS_OFFSET + Gpio_Platform_Offset[GpioTable[Index].GpioNo + SusStartOffset].offset * 16;
            PEI_TRACE((-1, PeiServices, "0x%08x, ", GpioTable[Index].GpioCfg.Dword));
            PEI_TRACE((-1, PeiServices, "Sus GPIO: %03d, ", GpioTable[Index].GpioNo));
            break;
			
		//EIP140031 >>
        case 4:
            //
            // SC Gpio Level X registers are Write Only, so reads to this register location have no effect.
            //
            if ((GpioTable[Index].GpioNo == 0) || (GpioTable[Index].GpioNo == 32) || \
                (GpioTable[Index].GpioNo == 64) || (GpioTable[Index].GpioNo == 96)) {
              ScGpioLvlValue = 0;
            }
            PEI_TRACE((-1, PeiServices, "ScGpioLvlValue[%d] = %x \n", GpioTable[Index].GpioNo, ScGpioLvlValue));

            if(GpioTable[Index].GpioCfg.Dword & ~LEGACY_GPIO_SC_OFFSET) {
              PEI_TRACE((-1, PeiServices, "0x%08x, ", GpioTable[Index].GpioCfg.Dword));
              PEI_TRACE((-1, PeiServices, "Programming Legacy SCore GPIO : %03d\n", GpioTable[Index].GpioNo));
              // Program Legacy SCore GPIO setting.
              ProgramLegacyGpio(PeiServices, CpuIo, GPIO_BASE_ADDRESS, GpioTable[Index], &ScGpioLvlValue);
            }
            Index++;
            continue;
        case 5:
            //
            // SUS Gpio Level X registers are Write Only, so reads to this register location have no effect.
            //
            if ((GpioTable[Index].GpioNo == 0) || (GpioTable[Index].GpioNo == 32)) {
              SusGpioLvlValue = 0;
            }
            PEI_TRACE((-1, PeiServices, "SusGpioLvlValue[%d] = %x \n", GpioTable[Index].GpioNo, SusGpioLvlValue));
            if(GpioTable[Index].GpioCfg.Dword & ~LEGACY_GPIO_SUS_OFFSET) {
              PEI_TRACE((-1, PeiServices, "0x%08x, ", GpioTable[Index].GpioCfg.Dword));
              PEI_TRACE((-1, PeiServices, "Programming Legacy Sus GPIO : %03d\n", GpioTable[Index].GpioNo));
              // Program Legacy SUS GPIO setting.
              ProgramLegacyGpio(PeiServices, CpuIo, GPIO_BASE_ADDRESS + 0x80, GpioTable[Index], &SusGpioLvlValue);
            }
            Index++;
            continue;
		//EIP140031 <<
		
        default:
            PEI_TRACE((-1, PeiServices, "Unknow GPIO Pin\n"));
            Index++;
            continue;
        }
//(P052813A+)<<
        mmio_conf0 = GpioBase + R_PCH_CFIO_PAD_CONF0 + OffsetConfig;
        mmio_padval = GpioBase + R_PCH_CFIO_PAD_VAL + OffsetConfig;

        // Step 1: PadVal Programming
        pad_val.dw = READ_MMIO32(mmio_padval);

        // Config PAD_VAL only for GPIO (Non-Native) Pin
        if(GpioTable[Index].GpioCfg.Fileds.Gpi || GpioTable[Index].GpioCfg.Fileds.Gpo) {
            // Set bits 1:2 according to PadVal
            pad_val.dw &= ~0x6; // Clear bits 1:2
            if(!(GpioTable[Index].GpioCfg.Fileds.Gpo & GpioTable[Index].GpioCfg.Fileds.Gpi)) {
                if(GpioTable[Index].GpioCfg.Fileds.Gpo)
                    pad_val.dw |= GPO;
                else
                    pad_val.dw |= GPI;
            } else
                pad_val.dw |= GPIO;

            // set GPO default value
            if(GpioTable[Index].GpioCfg.Fileds.Gpo) {
                if(GpioTable[Index].GpioCfg.Fileds.Gpod4H)
                    pad_val.r.pad_val = 1;
                else if(GpioTable[Index].GpioCfg.Fileds.Gpod4L)
                    pad_val.r.pad_val = 0;
            }
        }

        WRITE_MMIO32(mmio_padval, pad_val.dw);
        PEI_TRACE((-1, PeiServices, "VAL: %x , value: %x ;", mmio_padval, pad_val.dw));

        // Step 2: CONF0 Programming
        // Read GPIO default CONF0 value, which is assumed to be default value after reset.
        conf0_val.dw = READ_MMIO32(mmio_conf0);

        // Set Function #
        conf0_val.r.Func_Pin_Mux = GpioTable[Index].GpioCfg.Fileds.Func;

        if(GpioTable[Index].GpioCfg.Fileds.Gpo & !GpioTable[Index].GpioCfg.Fileds.Gpi) {
            // If used as GPO, then internal pull need to be disabled
            conf0_val.r.Pull_assign = 0;    // Non-pull
        } else {
		//CSP20130930>>
            // Set PullUp / PullDown
          switch(GpioTable[Index].GpioCfg.Fileds.Pull){
          case P_20K_H:
            conf0_val.r.Pull_assign = 0x1;  // PullUp
            conf0_val.r.Pull_strength = 0x2;// 20K
            break;
          case P_20K_L:
            conf0_val.r.Pull_assign = 0x2;  // PullUp
            conf0_val.r.Pull_strength = 0x2;// 20K
            break;
          case P_10K_H:
            conf0_val.r.Pull_assign = 0x1;  // PullUp
            conf0_val.r.Pull_strength = 0x1;// 20K
            break;
          case P_10K_L:
            conf0_val.r.Pull_assign = 0x2;  // PullUp
            conf0_val.r.Pull_strength = 0x1;// 20K
            break;
          case P_2K_H:
            conf0_val.r.Pull_assign = 0x1;  // PullUp
            conf0_val.r.Pull_strength = 0x0;// 2K
            break;
          case P_2K_L:
            conf0_val.r.Pull_assign = 0x2;  // PullUp
            conf0_val.r.Pull_strength = 0x0;// 2K
            break;
          case P_NONE:
            conf0_val.r.Pull_assign = 0x0;  // PullUp
            break;
          default:
            PEI_TRACE((-1, PeiServices, "\nSet PULL_TYPE error.\n"));
            break;
		//CSP20130930<<
          }
        }

        // Set INT Trigger Type
        conf0_val.dw &= ~0x0f000000;    // Clear bits 27:24

        // Set INT Trigger Type
        if(GpioTable[Index].GpioCfg.Fileds.IntType) {
            conf0_val.dw |= (GpioTable[Index].GpioCfg.Fileds.IntType) << 24;
        } else {
            // Interrupt not capable, clear bits 27:24
        }
		//CSP20130930>>
        if(GpioTable[Index].GpioCfg.Fileds.DirectIrqEn){
          conf0_val.r.DirectIrqEn = (GpioTable[Index].GpioCfg.Fileds.DirectIrqEn);
        }
		//CSP20130930<<
        // Write back the targeted GPIO config value according to platform (board) GPIO setting
        WRITE_MMIO32(mmio_conf0, conf0_val.dw);
        PEI_TRACE((-1, PeiServices, "CONF0: %x , value: %x,\n", mmio_conf0, conf0_val.dw));

        Index++;
    }
}

//CSP20130930>>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   AdjustGpioSetting
//
// Description: This function will update the GPIO setting due to some features are changed.
//
// Input:       PeiServices - Pointer to the PEI services table
//              CpuIo       - Pointer to the CPU I/O PPI
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID AdjustGpioSetting (
    IN EFI_PEI_SERVICES     **PeiServices,
    IN EFI_PEI_CPU_IO_PPI   *CpuIo )
{
    UINT32                      Index=0,TableNum=0;
    UINT32                      Buffer=0; //CSP20140314_21
    SB_SETUP_DATA               PchPolicyData;

    GetSbSetupData((VOID *)PeiServices, &PchPolicyData, TRUE);

    //CSP20140314_21 >>
    // This is for customer programming or other device setting. [Front of the AdjustGpioSetting]
    if((sizeof(AdjustMisc0Table)/sizeof(ADJUST_GPIO_SETTING))-1){
      Index = 0;
      while(AdjustMisc0Table[Index].Offset != -1){
        Buffer = RW_MMIO32(IO_BASE_ADDRESS + AdjustMisc0Table[Index].Offset, AdjustMisc0Table[Index].OrData, AdjustMisc0Table[Index].AndData);
        PEI_TRACE((-1, PeiServices, "Gpio Setting : BASE: 0x%08X , value: 0x%08X\n", IO_BASE_ADDRESS + AdjustMisc0Table[Index].Offset, Buffer));
        Index++;
      }
    }
    
    if((sizeof(AdjusteMMCTable)/sizeof(ADJUST_GPIO_SETTING))-1){
      if(PchPolicyData.eMMCEnabled != 0){
        Index = 0;
        while(AdjusteMMCTable[Index].Offset != -1){
          Buffer = RW_MMIO32(IO_BASE_ADDRESS + AdjusteMMCTable[Index].Offset, AdjusteMMCTable[Index].OrData, AdjusteMMCTable[Index].AndData);
          PEI_TRACE((-1, PeiServices, "eMMC enable : BASE: 0x%08X , value: 0x%08X\n", IO_BASE_ADDRESS + AdjusteMMCTable[Index].Offset, Buffer));
          Index++;
        }
      }
    }

    if((sizeof(AdjustAudioTable)/sizeof(ADJUST_GPIO_SETTING))-1){
      if(PchPolicyData.PchAzalia == 0){
        Index = 0;
        while(AdjustAudioTable[Index].Offset != -1){
          Buffer = RW_MMIO32(IO_BASE_ADDRESS + AdjustAudioTable[Index].Offset, AdjustAudioTable[Index].OrData, AdjustAudioTable[Index].AndData);
          PEI_TRACE((-1, PeiServices, "Audio disable : BASE: 0x%08X , value: 0x%08X\n", IO_BASE_ADDRESS + AdjustAudioTable[Index].Offset, Buffer));
          Index++;
        }
      }
    }

    if((sizeof(AdjustOsSelectTable)/sizeof(ADJUST_GPIO_SETTING))-1){
      if(PchPolicyData.OsSelect == OS_ANDROID){ //EIP149462
        Index = 0;
        while(AdjustAudioTable[Index].Offset != -1){
          Buffer = RW_MMIO32(IO_BASE_ADDRESS + AdjustOsSelectTable[Index].Offset, AdjustOsSelectTable[Index].OrData, AdjustOsSelectTable[Index].AndData);
          PEI_TRACE((-1, PeiServices, "Android OS : BASE: 0x%08X , value: 0x%08X\n", IO_BASE_ADDRESS + AdjustOsSelectTable[Index].Offset, Buffer));
          Index++;
        }
        IoWrite32(GPIO_BASE_ADDRESS,0);
        IoWrite32(GPIO_BASE_ADDRESS + 0x80,0);
      }
    }

    if((sizeof(AdjustI2C0Table)/sizeof(ADJUST_GPIO_SETTING))-1){
      if(PchPolicyData.LpssI2C0Enabled == 1 && PchPolicyData.LpssDma1Enabled == 1){
        Index = 0;
        while(AdjustI2C0Table[Index].Offset != -1){
          Buffer = RW_MMIO32(IO_BASE_ADDRESS + AdjustI2C0Table[Index].Offset, AdjustI2C0Table[Index].OrData, AdjustI2C0Table[Index].AndData);
          PEI_TRACE((-1, PeiServices, "i2C#1 enable : BASE: 0x%08X , value: 0x%08X\n", IO_BASE_ADDRESS + AdjustI2C5Table[Index].Offset, Buffer));
          Index++;
        }
      }
    }

    if((sizeof(AdjustI2C1Table)/sizeof(ADJUST_GPIO_SETTING))-1){
      if(PchPolicyData.LpssI2C1Enabled == 1 && PchPolicyData.LpssDma1Enabled == 1){
        Index = 0;
        while(AdjustI2C1Table[Index].Offset != -1){
          Buffer = RW_MMIO32(IO_BASE_ADDRESS + AdjustI2C1Table[Index].Offset, AdjustI2C1Table[Index].OrData, AdjustI2C1Table[Index].AndData);
          PEI_TRACE((-1, PeiServices, "i2C#2 enable : BASE: 0x%08X , value: 0x%08X\n", IO_BASE_ADDRESS + AdjustI2C5Table[Index].Offset, Buffer));
          Index++;
        }
      }
    }

    if((sizeof(AdjustI2C2Table)/sizeof(ADJUST_GPIO_SETTING))-1){
      if(PchPolicyData.LpssI2C2Enabled == 1 && PchPolicyData.LpssDma1Enabled == 1){
        Index = 0;
        while(AdjustI2C2Table[Index].Offset != -1){
          Buffer = RW_MMIO32(IO_BASE_ADDRESS + AdjustI2C2Table[Index].Offset, AdjustI2C2Table[Index].OrData, AdjustI2C2Table[Index].AndData);
          PEI_TRACE((-1, PeiServices, "i2C#3 enable : BASE: 0x%08X , value: 0x%08X\n", IO_BASE_ADDRESS + AdjustI2C5Table[Index].Offset, Buffer));
          Index++;
        }
      }
    }

    if((sizeof(AdjustI2C3Table)/sizeof(ADJUST_GPIO_SETTING))-1){
      if(PchPolicyData.LpssI2C3Enabled == 1 && PchPolicyData.LpssDma1Enabled == 1){
        Index = 0;
        while(AdjustI2C3Table[Index].Offset != -1){
          Buffer = RW_MMIO32(IO_BASE_ADDRESS + AdjustI2C3Table[Index].Offset, AdjustI2C3Table[Index].OrData, AdjustI2C3Table[Index].AndData);
          PEI_TRACE((-1, PeiServices, "i2C#4 enable : BASE: 0x%08X , value: 0x%08X\n", IO_BASE_ADDRESS + AdjustI2C5Table[Index].Offset, Buffer));
          Index++;
        }
      }
    }

    if((sizeof(AdjustI2C4Table)/sizeof(ADJUST_GPIO_SETTING))-1){
      if(PchPolicyData.LpssI2C4Enabled == 1 && PchPolicyData.LpssDma1Enabled == 1){
        Index = 0;
        while(AdjustI2C4Table[Index].Offset != -1){
          Buffer = RW_MMIO32(IO_BASE_ADDRESS + AdjustI2C4Table[Index].Offset, AdjustI2C4Table[Index].OrData, AdjustI2C4Table[Index].AndData);
          PEI_TRACE((-1, PeiServices, "i2C#5 enable : BASE: 0x%08X , value: 0x%08X\n", IO_BASE_ADDRESS + AdjustI2C5Table[Index].Offset, Buffer));
          Index++;
        }
      }
    }

    if((sizeof(AdjustI2C5Table)/sizeof(ADJUST_GPIO_SETTING))-1){
      if(PchPolicyData.LpssI2C5Enabled == 1 && PchPolicyData.LpssDma1Enabled == 1){
        Index = 0;
        while(AdjustI2C5Table[Index].Offset != -1){
          Buffer = RW_MMIO32(IO_BASE_ADDRESS + AdjustI2C5Table[Index].Offset, AdjustI2C5Table[Index].OrData, AdjustI2C5Table[Index].AndData);
          PEI_TRACE((-1, PeiServices, "i2C#6 enable : BASE: 0x%08X , value: 0x%08X\n", IO_BASE_ADDRESS + AdjustI2C5Table[Index].Offset, Buffer));
          Index++;
        }
      }
    }

    if((sizeof(AdjustI2C6Table)/sizeof(ADJUST_GPIO_SETTING))-1){
      if(PchPolicyData.LpssI2C6Enabled == 1 && PchPolicyData.LpssDma1Enabled == 1){
        Index = 0;
        while(AdjustI2C6Table[Index].Offset != -1){
          Buffer = RW_MMIO32(IO_BASE_ADDRESS + AdjustI2C6Table[Index].Offset, AdjustI2C6Table[Index].OrData, AdjustI2C6Table[Index].AndData);
          PEI_TRACE((-1, PeiServices, "i2C#7 enable : BASE: 0x%08X , value: 0x%08X\n", IO_BASE_ADDRESS + AdjustI2C6Table[Index].Offset, Buffer));
          Index++;
        }
      }
    }

    if((sizeof(AdjustUART0Table)/sizeof(ADJUST_GPIO_SETTING))-1){
      if(PchPolicyData.LpssHsuart0Enabled == 1 && PchPolicyData.LpssDma0Enabled == 1){
        Index = 0;
        while(AdjustUART0Table[Index].Offset != -1){
          Buffer = RW_MMIO32(IO_BASE_ADDRESS + AdjustUART0Table[Index].Offset, AdjustUART0Table[Index].OrData, AdjustUART0Table[Index].AndData);
          PEI_TRACE((-1, PeiServices, "UART0 enable : BASE: 0x%08X , value: 0x%08X\n", IO_BASE_ADDRESS + AdjustUART0Table[Index].Offset, Buffer));
          Index++;
        }
      }
    }

    if((sizeof(AdjustUART1Table)/sizeof(ADJUST_GPIO_SETTING))-1){
      if(PchPolicyData.LpssHsuart1Enabled == 1 && PchPolicyData.LpssDma1Enabled == 1){
        Index = 0;
        while(AdjustUART1Table[Index].Offset != -1){
          Buffer = RW_MMIO32(IO_BASE_ADDRESS + AdjustUART1Table[Index].Offset, AdjustUART1Table[Index].OrData, AdjustUART1Table[Index].AndData);
          PEI_TRACE((-1, PeiServices, "UART1 enable : BASE: 0x%08X , value: 0x%08X\n", IO_BASE_ADDRESS + AdjustUART1Table[Index].Offset, Buffer));
          Index++;
        }
      }
    }
    
    // This is for customer programming or other device setting. [End of the AdjustGpioSetting]
    if((sizeof(AdjustMisc1Table)/sizeof(ADJUST_GPIO_SETTING))-1){
      Index = 0;
      while(AdjustMisc1Table[Index].Offset != -1){
        Buffer = RW_MMIO32(IO_BASE_ADDRESS + AdjustMisc1Table[Index].Offset, AdjustMisc1Table[Index].OrData, AdjustMisc1Table[Index].AndData);
        PEI_TRACE((-1, PeiServices, "Gpio Setting : BASE: 0x%08X , value: 0x%08X\n", IO_BASE_ADDRESS + AdjustMisc1Table[Index].Offset, Buffer));
        Index++;
      }
    }
	//CSP20140314_21 <<
}
//CSP20130930<<

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   InitGpio
//
// Description: This function initializes SB GPIOs
//
// Input:       PeiServices - Pointer to the PEI services table
//              CpuIo       - Pointer to the CPU I/O PPI
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID InitGpio (
    IN EFI_PEI_SERVICES     **PeiServices,
    IN EFI_PEI_CPU_IO_PPI   *CpuIo )
{

    EFI_STATUS                  Status;
    AMI_PEI_SB_CUSTOM_PPI       *SbPeiOemPpi;
    AMI_GPIO_INIT_TABLE_STRUCT  *GpioInitTable = DefaultGpioTable;
    //CSP20140414 (-) PCH_STEPPING                ScStepping;

    PEI_TRACE((-1, PeiServices, "<< InitGpio >>\n"));

    Status = (*PeiServices)->LocatePpi(PeiServices, \
                                       &gAmiPeiSbCustomPpiGuid, \
                                       0, \
                                       NULL, \
                                       &SbPeiOemPpi);

    if(Status == EFI_SUCCESS) {
        if(SbPeiOemPpi->GpioInit != NULL) {
            if(SbPeiOemPpi->GpioInit->GpioBaseAddr == IO_BASE_ADDRESS) {
                if(SbPeiOemPpi->GpioInit->InitDefaultGpioSetting)
                    // Program SB default GPIO setting.
                    ProgramGpio(PeiServices, \
                                CpuIo, \
                                SbPeiOemPpi->GpioInit->GpioBaseAddr, \
                                GpioInitTable);

                GpioInitTable = SbPeiOemPpi->GpioInit->GpioTable;
                ProgramGpio(PeiServices, \
                            CpuIo, \
                            SbPeiOemPpi->GpioInit->GpioBaseAddr, \
                            GpioInitTable);
            }
        } else {
            ProgramGpio(PeiServices, CpuIo, IO_BASE_ADDRESS, GpioInitTable);
        }
    } else {
        ProgramGpio(PeiServices, CpuIo, IO_BASE_ADDRESS, GpioInitTable);
    }
//CSP20130930>>    
#if ADJUST_GPIO_SUPPORT
    AdjustGpioSetting(PeiServices, CpuIo);
#endif
//CSP20130930<<

  //
  //CSP20140414_23 (-)>> 
  //BayTrail DM BWG v1.37 remove chapter27.15 
  //    
    //
    // BayTrail DM BWG v1.35 chapter27.15 Special BIOS requirements for C0 SKU
    //  BIOS should perform the following settings for C0 SKU at PEI stage to 
    //  disable RTD3x function to avoid Sx power state auto-wakeup issue.
    //  Clear [27:24] and [2:0] of GPIO IOBASE MMIO offset 0x2210 and ox21e0.
    //
  //  ScStepping = PchStepping();
  //  if(ScStepping >= PchC0)
  //  {
  //    MmioAnd32(IO_BASE_ADDRESS + 0x2210, (UINT32) ~(0x0F000007));
  //    MmioAnd32(IO_BASE_ADDRESS + 0x21E0, (UINT32) ~(0x0F000007));
  //  }
  //CSP20140414_23 (-)<<
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IsSBDevice
//
// Description: This function detimines SB PCI devices
//
// Input:       UINT64 PciAddress
//              UINT8  *PciSidReg
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS IsSBDevice(
    IN UINT64    PciAddress,
    IN OUT UINT8 *PciSidReg
)
{
    UINT8  i;
    UINT32 TableSize;

    AMI_SB_PCI_DEVICES_TABLE_STRUCT PchDeviceTable[] = {
                                SMBUS_BUS_DEV_FUN,        R_PCH_SMBUS_SVID,
                                SB_BUS_DEV_FUN,           R_PCH_LPC_SS,
                                LPSS_SPI_BUS_DEV_FUN,     R_PCH_LPSS_SPI_SSID,
                                LPSS_HSUART1_BUS_DEV_FUN, R_PCH_LPSS_HSUART_SSID,
                                LPSS_HSUART0_BUS_DEV_FUN, R_PCH_LPSS_HSUART_SSID,
                                LPSS_PWM1_BUS_DEV_FUN,    R_PCH_LPSS_PWM_SSID,
                                LPSS_PWM0_BUS_DEV_FUN,    R_PCH_LPSS_PWM_SSID,
                                LPSS_DMAC1_BUS_DEV_FUN,   R_PCH_LPSS_DMAC_SSID,
                                LPSS_DMAC0_BUS_DEV_FUN,   R_PCH_LPSS_DMAC_SSID,
                                LPSS_I2C6_BUS_DEV_FUN,    R_PCH_LPSS_I2C_SSID,
                                LPSS_I2C5_BUS_DEV_FUN,    R_PCH_LPSS_I2C_SSID,
                                LPSS_I2C4_BUS_DEV_FUN,    R_PCH_LPSS_I2C_SSID,
                                LPSS_I2C3_BUS_DEV_FUN,    R_PCH_LPSS_I2C_SSID,
                                LPSS_I2C2_BUS_DEV_FUN,    R_PCH_LPSS_I2C_SSID,
                                LPSS_I2C1_BUS_DEV_FUN,    R_PCH_LPSS_I2C_SSID,
                                LPSS_I2C0_BUS_DEV_FUN,    R_PCH_LPSS_I2C_SSID,
                                USB_EHCI_BUS_DEV_FUN,     R_PCH_EHCI_SVID,
                                USB_XHCI_BUS_DEV_FUN,     R_PCH_XHCI_SVID,
                                SCC_SDIO3_BUS_DEV_FUN,    R_PCH_SCC_SDIO_SSID,
                                OTG_BUS_DEV_FUN,          R_PCH_OTG_SSID,
                                LPE_BUS_DEV_FUN,          R_PCH_LPE_SSID,
                                SCC_SDIO2_BUS_DEV_FUN,    R_PCH_SCC_SDIO_SSID,
                                SCC_SDIO1_BUS_DEV_FUN,    R_PCH_SCC_SDIO_SSID,
                                SCC_SDIO0_BUS_DEV_FUN,    R_PCH_SCC_SDIO_SSID,
                                PCIE_ROOT_P0_BUS_DEV_FUN, R_PCH_PCIE_SVID,
                                PCIE_ROOT_P1_BUS_DEV_FUN, R_PCH_PCIE_SVID,
                                PCIE_ROOT_P2_BUS_DEV_FUN, R_PCH_PCIE_SVID,
                                PCIE_ROOT_P3_BUS_DEV_FUN, R_PCH_PCIE_SVID,
                                AZALIA_BUS_DEV_FUN,       R_PCH_HDA_SVID,
                                SEC_DEVICE_BUS_DEV_FUN,   0x2c,
//                                LAN_BUS_DEV_FUN,          R_PCH_LAN_SVID,
                                SATA_BUS_DEV_FUN,         R_PCH_SATA_SS
                                };
    TableSize = sizeof(PchDeviceTable) / sizeof(AMI_SB_PCI_DEVICES_TABLE_STRUCT);

    for(i = 0; i < TableSize; i++) {

        if(PciAddress != PchDeviceTable[i].PciAddr) {
            continue;
        } else {
            if(READ_MMIO32((UINTN)PchDeviceTable[i].PciAddr) == 0xffffffff)
                return EFI_UNSUPPORTED;

            *PciSidReg = PchDeviceTable[i].PciSidReg;
            return EFI_SUCCESS;
        }
    }
    return EFI_UNSUPPORTED;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ProgramSbSubId
//
// Description: This function programs SB PCI devices sub-vendor ID and
//              sub-system ID.
//
// Input:       PeiServices - Pointer to the PEI services table
//              PciCfg      - Pointer to the PCI Configuration PPI
//
// Output:      VOID
//
// Notes:       1. This routine only programs the PCI device in SB, hence, we
//                 have to check the bus/device/function numbers whether they
//                 are a SB PCI device or not.
//              2. This routine is invoked by PEI phase.(After PEI permantent
//                 memory be installed)
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ProgramSbSubId(
    IN EFI_PEI_SERVICES         **PeiServices,
    IN EFI_PEI_PCI_CFG2_PPI     *PciCfg)
{
    EFI_STATUS                    Status;
    AMI_PEI_SB_CUSTOM_PPI         *SbPeiOemPpi;
    UINTN                         i = 0;
    UINT32                        PciSid = 0xffffffff;
    UINT8                         PciSidReg = 0xff;
    AMI_SB_PCI_SSID_TABLE_STRUCT  DefaultSidTbl[] = {SB_PCI_DEVICES_SSID_TABLE};
    AMI_SB_PCI_SSID_TABLE_STRUCT  *SsidTblPtr = DefaultSidTbl;

    Status = (*PeiServices)->LocatePpi(PeiServices, \
                                       &gAmiPeiSbCustomPpiGuid, \
                                       0, \
                                       NULL, \
                                       &SbPeiOemPpi);

    if(Status == EFI_SUCCESS) {
        if((SbPeiOemPpi->SsidTable != NULL) && (SbPeiOemPpi->SsidTable[0].PciAddr != 0xffffffffffffffff))
            SsidTblPtr = SbPeiOemPpi->SsidTable;
    }

    while(SsidTblPtr[i].PciAddr != 0xffffffffffffffff) {
        if(IsSBDevice(SsidTblPtr[i].PciAddr, &PciSidReg) == EFI_SUCCESS) {
            if(SsidTblPtr[i].Sid == 0xffffffff) {
                PciSid = READ_MMIO32((UINTN)SsidTblPtr[i].PciAddr);
            } else {
                PciSid = SsidTblPtr[i].Sid;
            }

            if(SsidTblPtr[i].PciAddr == USB_EHCI_BUS_DEV_FUN)
                SET_MMIO8((UINTN)(SsidTblPtr[i].PciAddr + R_PCH_EHCI_ACCESS_CNTL), B_PCH_EHCI_ACCESS_CNTL_WRT_RDONLY);

            WRITE_MMIO32((UINTN)(SsidTblPtr[i].PciAddr + PciSidReg), PciSid);

            if(SsidTblPtr[i].PciAddr == USB_EHCI_BUS_DEV_FUN)
                RESET_MMIO8((UINTN)(SsidTblPtr[i].PciAddr + R_PCH_EHCI_ACCESS_CNTL), B_PCH_EHCI_ACCESS_CNTL_WRT_RDONLY);
        }

        i++;
    }

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SbPeiInitAfterMemInstalled
//
// Description: This function can be used to program any SB regisater after
//              PEI permantent memory be installed.
//
// Input:       PeiServices      - Pointer to the PEI services table
//              NotifyDescriptor - Pointer to the descriptor for the
//                                 notification event.
//              InvokePpi        - Pointer to the PPI that was installed
//
// Output:      EFI_STATUS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SbPeiInitAfterMemInstalled(
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
    IN VOID                         *InvokePpi)
{
    EFI_STATUS                  Status;
    EFI_PEI_CPU_IO_PPI          *CpuIo;
    EFI_PEI_PCI_CFG2_PPI        *PciCfg;
    EFI_BOOT_MODE               BootMode;
    SB_SETUP_DATA               PchPolicyData; //EIP132001
	//EIP131059 >>
    UINTN                       PciD19F0RegBase; 
    UINT32                      Data32And;
    UINT32                      Data32Or;
    //EIP131059 <<

    CpuIo = (*PeiServices)->CpuIo;
    PciCfg = (*PeiServices)->PciCfg;

    PEI_PROGRESS_CODE(PeiServices, PEI_MEM_SB_INIT);

    (*PeiServices)->GetBootMode(PeiServices, &BootMode);
    
    if(IsRecovery(PeiServices, PciCfg, CpuIo)) {
    		BootMode = BOOT_IN_RECOVERY_MODE;
        (*PeiServices)->SetBootMode(PeiServices, BootMode);
        PEI_TRACE((-1, PeiServices, "Boot mode = BOOT_IN_RECOVERY_MODE\n"));
        (*PeiServices)->InstallPpi(PeiServices, mRecoveryModePpi);
    }
    
    //EIP132001 >>
    if(BootMode == BOOT_IN_RECOVERY_MODE)
    {
        // Get the value of the SB Setup data.
        GetSbSetupData((VOID *)PeiServices, &PchPolicyData, TRUE);
        //
        // Initialize USB Controller before entering recovery mode.
        //
        PEI_TRACE((-1, PeiServices, "Recovery Mode!\n"));
        InitPchUsb(PeiServices, &PchPolicyData);
        //(EIP131059+)>>
        PEI_TRACE((-1, PeiServices, "PchSataInitRecovery - Start\n"));
        PciD19F0RegBase = MmPciAddress (0,
                            DEFAULT_PCI_BUS_NUMBER_PCH,
                            PCI_DEVICE_NUMBER_PCH_SATA,
                            PCI_FUNCTION_NUMBER_PCH_SATA,
                            0
                          );
        ///
        /// Enable SATA function and read back to take effect
        ///
        MmioAnd32 (PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS, BIT17);
        MmioRead32 (PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS); // Read back Posted Writes Register
        MmioAnd8 (
            PciD19F0RegBase + R_PCH_SATA_MAP,
            (UINT8)~(B_PCH_SATA_MAP_SMS_MASK | B_PCH_SATA_PORT_TO_CONTROLLER_CFG)
            );
        ///
        /// System BIOS must set D19:F0 + 0x94 [8:0] = 0x183 as part of the chipset initialization prior to
        /// SATA configuration. These bits should be restored while resuming from a S3 sleep state.
        ///
        Data32And = (UINT32)~(BIT8 | BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0);
        Data32Or  = B_PCH_SATA_TM_NQIUFD | B_PCH_SATA_TM_SCTI | B_PCH_SATA_TM_RRSSEL;
        MmioAndThenOr32 (
          (UINTN) (PciD19F0RegBase + R_PCH_SATA_TM),
          Data32And,
          Data32Or
          );
        ///
        /// D19:F0 + 0x92 [15] = 1b
        /// BIOS is requested to set "OOB Retry Mode" (ORM) bit to
        /// to enable the ASR (Asynchronous Signal Recovery) support in SATA HBA.
        /// These bit should be restored while resuming from a S3 sleep state.
        ///
        MmioOr16 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_PCS), (UINT16) (B_PCH_SATA_PCS_OOB_RETRY));
        ///
        /// Program D19:F0 + 0x98 [29]   = 1b to have SATA IOSF Sideband Dynamic Clock Gating Enable
        /// Program D19:F0 + 0x98 [25]   = 1b to have Port Local RxStandby Power Staggering Enable
        /// Program D19:F0 + 0x98 [22]   = 1b to have SRAM Parity Check Disable for non-server platform
        /// Program D19:F0 + 0x98 [20]   = 1b to have Dynamic Squelch Detector during LPM Mechanism Disable
        /// Program D19:F0 + 0x98 [19]   = 1b to have Dynamic Squelch Detector Mechanism Disable
        /// Program D19:F0 + 0x98 [18]   = 1b to have ISM Extended IDLE Entry Delay Enable
        /// Program D19:F0 + 0x98 [12:7] = 04h as the ALIGN Detection Watchdog Timer Count
        /// Program D19:F0 + 0x98 [6:5]  = 1b to use no-periodic-dual-ALIGN as Unsquelch Indicator
        ///
        Data32And = (UINT32) ~(B_PCH_SATA_TM2_ALDWTC | B_PCH_SATA_TM2_UNSQLIND);
        Data32Or  = (UINT32) (B_PCH_SATA_TM2_SIDECLKDCGEN |
                              B_PCH_SATA_TM2_PPST |
                              B_PCH_SATA_TM2_SPLDCGE |
                              B_PCH_SATA_TM2_SRAMPARDIS |
                              B_PCH_SATA_TM2_DSDLPMD |
                              B_PCH_SATA_TM2_DSDD |
                              B_PCH_SATA_TM2_ISMDLYEN |
                              BIT9 |
                              BIT5);
        
        //EIP149024 >>
        //
        //BWG Page 202, Doc# 514148 BYT-M/D BWG Vol2 Rev1p22,
        //Chap 35.1.5 ¡V ¡§SATA Initialization Programming Requirements¡¨
        //
        // Get the value of the SB Setup data.
        GetSbSetupData((VOID *)PeiServices, &PchPolicyData, TRUE);
        if(PchPolicyData.SataOddPort != 2)
        	Data32Or |= B_PCH_SATA_TM2_PLLSHUTDIS;
        //EIP149024 <<
        
        MmioAndThenOr32 (
          (UINTN) (PciD19F0RegBase + R_PCH_SATA_TM2),
          Data32And,
          Data32Or
          );
        ///
        /// Bus Master Enable
        /// Program D19:F0 + 0x04 [2] = 1b
        ///
        MmioOr8 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_COMMAND), (UINT8) (B_PCH_SATA_COMMAND_BME));
        ///
        /// Test Mode Register 3
        /// Program SATA SIR 0x70 = 0x00288301 at 25 MHz free running clock.
        ///
        MmioWrite8  ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRI), (UINT8) 0x70);
        MmioWrite32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRD), (UINT32) 0x00288301);
        ///
        /// Test Mode Register 4
        /// Program SATA SIR 0x54 = 0x00000300.
        ///
        MmioWrite8  ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRI), (UINT8) 0x54);
        //EIP149024 >>
        //
        //BWG Page 202, Doc# 514148 BYT-M/D BWG Vol2 Rev1p22,
        //Chap 35.1.5 ¡V ¡§SATA Initialization Programming Requirements¡¨
        //
        switch(PchPolicyData.SataOddPort)
        {
        	case 0: //Port0
        		MmioWrite32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRD), (UINT32) 0x00000301);
				break;
        	case 1: //Port1
        		MmioWrite32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRD), (UINT32) 0x00000302);
				break;
        	default:
        		MmioWrite32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRD), (UINT32) 0x00000300);
        }
        //EIP149024 <<
        
        ///
        /// Test Mode Register 5
        /// Program SATA SIR 0x58 = 0x50000000.
        ///
        MmioWrite8  ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRI), (UINT8) 0x58);
        MmioWrite32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRD), (UINT32) 0x50000000);
        ///
        /// OOB Detection Margin Register
        /// Program SATA SIR 0x6C = 0x130C0603 at 25 MHz free running clock.
        ///
        MmioWrite8  ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRI), (UINT8) 0x6C);
        MmioWrite32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRD), (UINT32) 0x130C0603);
        ///
        /// Gasket Control Register
        /// Program SATA SIR 0xF4 = 0x00
        ///
        MmioWrite8  ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRI), (UINT8) 0xF4);
        MmioWrite32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRD), (UINT32) 0x00);
        ///
        /// Enable the SATA port0 and port1 only for VLV.
        ///
        MmioOr8 (
          PciD19F0RegBase + R_PCH_SATA_PCS,
          (UINT8) (B_PCH_SATA_PCS_PORT1_EN | B_PCH_SATA_PCS_PORT0_EN)
          );
        PEI_TRACE((-1, PeiServices, "PchSataInitRecovery - End\n"));
        //(EIP131059+)<<
    }
    //EIP132001 <<
    
//    if (BootMode == BOOT_ON_S3_RESUME) {
//
//    } else {
//
//   }

    // Program SB Devices' Subsystem Vendor ID & Subsystem ID
    ProgramSbSubId(PeiServices, PciCfg);

    // Set up necessary PPI notifications after PEI permantent memory
    // be installed
    Status = (*PeiServices)->NotifyPpi(PeiServices, &mNotifyList[0]);
    ASSERT_PEI_ERROR(PeiServices, Status);

    return  EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ProgramSbRegBeforeEndofPei
//
// Description: This function can be used to program any SB regisater before
//              end of PEI phase.
//
// Input:       PeiServices      - Pointer to the PEI services table
//              NotifyDescriptor - Pointer to the descriptor for the
//                                 notification event.
//              InvokePpi        - Pointer to the PPI that was installed
//
// Output:      EFI_STATUS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS ProgramSbRegBeforeEndofPei(
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
    IN VOID                         *InvokePpi)
{
    /*
        EFI_STATUS                  Status = EFI_SUCCESS;
        EFI_PEI_CPU_IO_PPI          *CpuIo;
        EFI_PEI_PCI_CFG2_PPI        *PciCfg;
        EFI_BOOT_MODE               BootMode;

        CpuIo = (*PeiServices)->CpuIo;
        PciCfg = (*PeiServices)->PciCfg;

        Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode);

        if (BootMode == BOOT_ON_S3_RESUME) {
            // Porting if needed.
        } else {
            // Porting if needed.
        }
    */
    return  EFI_SUCCESS;
}
//(EIP131059+)>>
#if ATAPI_RECOVERY_SUPPORT
EFI_GUID gIdeRecoveryNativeModePpiGuid = \
    PEI_IDE_RECOVERY_NATIVE_MODE_PPI_GUID;

PEI_IDE_RECOVERY_NATIVE_MODE_PPI IdeRecoveryNativeModePpi =
{
    SB_TEMP_IO_BASE+0x200,
    SB_TEMP_IO_BASE+0x282,
    SB_TEMP_IO_BASE+0x300,
    SB_TEMP_IO_BASE+0x382
};

EFI_PEI_PPI_DESCRIPTOR IdeRecoveryNativeModePpiDescriptor =
{
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST), \
    &gIdeRecoveryNativeModePpiGuid, &IdeRecoveryNativeModePpi
};

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   EnableAtaChannel
//
// Description: This function is used to initialize the IDE ATA channel
//              for BIOS recovery from ATA devices
//
// Input:       IN EFI_PEI_SERVICES **PeiServices - Pointer to the PEI services table
//              IN PEI_ATA_CONTROLLER_PPI *This - Pointer to the PEI ATA Controller PPI
//              IN UINT8 ChannelMask - Bit flag indicating which channel has to be
//                                     enabled. The following is the bit definition:
//                  Bit0    IDE Primary
//                  Bit1    IDE Secondary
//                  Bit2    No SATA
//                  Bit3    SATA as Primary
//                  Bit4    SATA as Secondary
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS EnableAtaChannel(
    IN EFI_PEI_SERVICES         **PeiServices,
    IN PEI_ATA_CONTROLLER_PPI   *This,
    IN UINT8                    ChannelMask
)
{
    EFI_STATUS          Status;
    UINTN       PciD19F0RegBase;
    UINT32      SataGcReg;
    UINT16      SataPortsEnabled;
    UINT8       Index;
    EFI_PEI_STALL_PPI *StallPpi;

    (**PeiServices).LocatePpi(PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, &StallPpi);
    
    PEI_TRACE((-1, PeiServices, "EnableAtaChannel entry.\n"));
    StallPpi->Stall(PeiServices, StallPpi, 10000);  
    PciD19F0RegBase = MmPciAddress (0,
                        DEFAULT_PCI_BUS_NUMBER_PCH,
                        PCI_DEVICE_NUMBER_PCH_SATA,
                        PCI_FUNCTION_NUMBER_PCH_SATA,
                        0
                      );

    SataGcReg = MmioRead32 (PciD19F0RegBase + R_PCH_SATA_SATAGC);
    SataGcReg |= (UINT32) (B_PCH_SATA_SATAGC_WRRSELMPS);
    /// Set Legacy IDE 
    MmioAnd8 (
      PciD19F0RegBase + R_PCH_SATA_PI_REGISTER,
      (UINT8)~(B_PCH_SATA_PI_REGISTER_PNE | B_PCH_SATA_PI_REGISTER_SNE) // Legacy IDE Mode
      );
    StallPpi->Stall(PeiServices, StallPpi, 10000);  
    MmioWrite8 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_AHCI_CAP_PTR), (UINT8) R_PCH_SATA_PID);
    StallPpi->Stall(PeiServices, StallPpi, 10000);  
    MmioAnd16  ((UINTN) (PciD19F0RegBase + R_PCH_SATA_PID), (UINT16) ~B_PCH_SATA_PID_NEXT);
    StallPpi->Stall(PeiServices, StallPpi, 10000);  
    ///
    /// Set legacy IDE decoding
    /// These bits also effect with AHCI mode when PCH is using legacy mechanisms.
    ///
    MmioOr16 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_PTIM), (UINT16) (B_PCH_SATA_PTIM_DE));
    StallPpi->Stall(PeiServices, StallPpi, 10000);  
    MmioOr16 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_STIM), (UINT16) (B_PCH_SATA_STIM_DE));
    StallPpi->Stall(PeiServices, StallPpi, 10000);  
    ///
    /// Set MAP register
    /// Set D19:F0 MAP[13:8] to 1b if SATA Port 0/1 is disabled
    /// SataPortsEnabled [5:0] = SATA Port 0/1 enable status
    /// MAP.SPD (D19:F0:Reg90h[13:8]) is Write Once
    ///
    SataPortsEnabled = 0x03;
    MmioOr16 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_MAP), ((~SataPortsEnabled << 8) & (UINT16) B_PCH_SATA_MAP_SPD));
    StallPpi->Stall(PeiServices, StallPpi, 10000);  
    ///
    /// D19:F0 PCS[5:0] = Port 0~5 Enabled bit
    ///
    MmioAndThenOr16 (
      (UINTN) (PciD19F0RegBase + R_PCH_SATA_PCS),
      (UINT16)(~( B_PCH_SATA_PCS_PORT5_EN |
              B_PCH_SATA_PCS_PORT4_EN |
              B_PCH_SATA_PCS_PORT3_EN |
              B_PCH_SATA_PCS_PORT2_EN |
              B_PCH_SATA_PCS_PORT1_EN |
                  B_PCH_SATA_PCS_PORT0_EN )),
      (UINT16) (SataPortsEnabled)
      );
    StallPpi->Stall(PeiServices, StallPpi, 10000);  
    MmioOr32 (
      (UINTN) (PciD19F0RegBase + R_PCH_SATA_TM),
      (UINT32) (B_PCH_SATA_TM_PORT5_PCD |
            B_PCH_SATA_TM_PORT4_PCD |
            B_PCH_SATA_TM_PORT3_PCD |
            B_PCH_SATA_TM_PORT2_PCD |
            B_PCH_SATA_TM_PORT1_PCD |
            B_PCH_SATA_TM_PORT0_PCD));
    StallPpi->Stall(PeiServices, StallPpi, 10000);  
    ///
    /// Now only enable clock for those ports that are enabled.
    ///
    for (Index = 0; Index < PCH_AHCI_MAX_PORTS; Index++) {
      if ((SataPortsEnabled & (B_PCH_SATA_PCS_PORT0_EN << Index)) != 0) {
        MmioAnd32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_TM), (UINT32) ~(B_PCH_SATA_TM_PORT0_PCD << Index));
        StallPpi->Stall(PeiServices, StallPpi, 10000);  
      }
    }
    ///
    /// BIOS is requested to program the REGLOCK (D19:F0: 0x9C [31]) RWO bit to '1b'.
    ///
    SataGcReg |= (UINT32) B_PCH_SATA_SATAGC_REGLOCK;
    ///
    /// Unconditional write is necessary to lock the register
    ///
    MmioWrite32 (PciD19F0RegBase + R_PCH_SATA_SATAGC, SataGcReg);
    StallPpi->Stall(PeiServices, StallPpi, 10000);  
    ///
    /// Bus Master,Memory Space,I/O Space Enable
    /// Program D19:F0 + 0x04 = 7b
    ///
    MmioOr8 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_COMMAND), (UINT8) (B_PCH_SATA_COMMAND_BME|B_PCH_SATA_COMMAND_IOSE|B_PCH_SATA_COMMAND_MSE));
    StallPpi->Stall(PeiServices, StallPpi, 10000);  
    Status = (**PeiServices).InstallPpi( PeiServices, \
                                           &IdeRecoveryNativeModePpiDescriptor);
    if (EFI_ERROR (Status)) return Status;
    return EFI_SUCCESS;
}
#endif // ATAPI_RECOVERY_SUPPORT
//(EIP131059+)<<

#if KBC_SUPPORT && Recovery_SUPPORT
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ResetKbc
//
// Description: This function resets Keyboard controller for Ctrl-Home
//              recovery function.     
//
// Input:       PeiServices - Pointer to the Pei Services function and
//                            data structure
//              CpuIo       - Pointer to the CPU I/O PPI
//              PciCfg      - Pointer to the PCI Configuration PPI
//
// Output:      None
//
// Notes:       No porting required.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ResetKbc (
    IN EFI_PEI_SERVICES     **PeiServices,
    IN EFI_PEI_CPU_IO_PPI   *CpuIo,
    IN EFI_PEI_PCI_CFG2_PPI  *PciCfg )
{
    volatile UINT8      KbcSts = 0;
    volatile UINT8      Buffer8;
    UINT32              TimeOut = 0x100;
    EFI_PEI_STALL_PPI *StallPpi;
    
    (**PeiServices).LocatePpi (PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, &StallPpi);

    // Reset KBC
    CpuIo->IoWrite8( PeiServices, CpuIo, KBC_IO_STS, 0xAD ); // 0x64
    if (CpuIo->IoRead8( PeiServices, CpuIo, KBC_IO_STS ) != 0xff) {
        // Clear KBC buffer
        do {
            Buffer8 = CpuIo->IoRead8( PeiServices, CpuIo, KBC_IO_DATA );
            KbcSts = CpuIo->IoRead8( PeiServices, CpuIo, KBC_IO_STS ); // 0x64
            TimeOut--;
        } while ((KbcSts & 3) && (TimeOut != 0));


        // Send BAT command 
        CpuIo->IoWrite8( PeiServices, CpuIo, KBC_IO_STS, 0xaa ); // 0x64

        // IBFree
        for (TimeOut = 0; TimeOut < 0x1000; TimeOut++) {
            CpuIo->IoWrite8( PeiServices, CpuIo, IO_DELAY_PORT, KbcSts );
            KbcSts = CpuIo->IoRead8( PeiServices, CpuIo, KBC_IO_STS ); // 0x64
            if ((KbcSts & 2) == 0) break;
        }

        // OBFree
        for (TimeOut = 0; TimeOut < 0x500; TimeOut++) {
            CpuIo->IoWrite8( PeiServices, CpuIo, IO_DELAY_PORT, KbcSts );
            KbcSts = CpuIo->IoRead8( PeiServices, CpuIo, KBC_IO_STS ); // 0x64
            if (KbcSts & 1) break;
        }

        // Get result if needed
        if (KbcSts & 1)
            Buffer8 = CpuIo->IoRead8( PeiServices, CpuIo, KBC_IO_DATA );
    }

    // Clear KBC status buffer.
    KbcSts = CpuIo->IoRead8 ( PeiServices, CpuIo, KBC_IO_STS ); // 0x64
    
}
#endif

#if     SMM_SUPPORT

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SbPeiActivateSmi
//
// Description: This function is used to generate S/W SMI in the system. This
//              call is mainly used during S3 resume to restore SMMBase
//
// Input:       IN EFI_PEI_SERVICES **PeiServices - Pointer to the PEI services table
//              IN PEI_SMM_CONTROL_PPI *This - Pointer to the SMM Control PPI
//              IN OUT INT8 *ArgumentBuffer - Argument to be used to generate S/W SMI
//              IN OUT UINTN *ArgumentBufferSize - Size of the argument buffer
//              IN BOOLEAN Periodic - Indicates the type of SMI invocation
//              IN UINTN ActivationInterval - If it is periodic invocation, this field
//                                            indicates the period at which the SMI is generated
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
SbPeiActivateSmi(
    IN EFI_PEI_SERVICES     **PeiServices,
    IN PEI_SMM_CONTROL_PPI  *This,
    IN OUT INT8             *ArgumentBuffer OPTIONAL,
    IN OUT UINTN            *ArgumentBufferSize OPTIONAL,
    IN BOOLEAN              Periodic OPTIONAL,
    IN UINTN                ActivationInterval OPTIONAL
)
{
    /** Porting required. Include code to generate S/W SMI
        UINT8   Data;
    //  UINT8   Value;

        if (Periodic) return EFI_INVALID_PARAMETER;

        if (ArgumentBuffer == NULL) {
            Data = 0xFF;                        //If no data given, use 0xFF to trigger SMI.
        } else {
            if (ArgumentBufferSize == NULL || *ArgumentBufferSize != 1)
                return EFI_INVALID_PARAMETER;   // Only able to send 1 byte.
            Data = *ArgumentBuffer;
        }

    /** Porting required. Include code to generate S/W SMI
        // Enable Software SMIs
        Value =  IoRead8(PM_BASE_ADDRESS + ICH_SMI_EN) | (1 << 5) | 1;
        IoWrite8(PM_BASE_ADDRESS + ICH_SMI_EN, Value);

        IoWrite8(ICH_APM_CNT,Data);     //This triggers SMI

     Porting End **/

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SbPeiDeactivateSmi
//
// Description: This function is used to clear the SMI and issue End-of-SMI
//
// Input:       IN EFI_PEI_SERVICES **PeiServices Pointer to the PEI services table
//              IN PEI_SMM_CONTROL_PPI *This        Pointer to the SMM Control PPI
//              IN BOOLEAN Periodic    Indicates the type of SMI invocation to stop
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SbPeiDeactivateSmi(
    IN EFI_PEI_SERVICES     **PeiServices,
    IN PEI_SMM_CONTROL_PPI  *This,
    IN BOOLEAN              Periodic OPTIONAL
)
{
//  UINT8       Value;

    if(Periodic) return EFI_INVALID_PARAMETER;

    /** Porting Required.  Include code to clear all SMI status
    // Clear the Power Button Override Status Bit
    IoWrite16(PM_BASE_ADDRESS, 1 << 8);

     // Clear the APM SMI Status Bit
    IoWrite8(PM_BASE_ADDRESS+ICH_SMI_STS, 1 << 5);

    //Set EOS
    Value = IoRead8(PM_BASE_ADDRESS+ICH_SMI_EN) | 2;
    IoWrite8(PM_BASE_ADDRESS+ICH_SMI_EN,Value);
    **/ // Porting end

    return EFI_SUCCESS;
}
#endif

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************



