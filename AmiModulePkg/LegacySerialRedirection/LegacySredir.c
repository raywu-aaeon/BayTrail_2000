//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

/** @file LegacySredir.c
    Serial Redirection Compatibility Support Module entry point 
    and interface functions
*/

//
// Disabling the warnings.
//
#pragma warning (disable : 4100 )

#pragma warning (disable : 4306 ) 

//---------------------------------------------------------------------------

#include    "Token.h"
#include    "Protocol/LegacySredir.h"
#include    "Protocol/LegacyBios.h"
#include    "Protocol/LegacyBiosExt.h"
#include    "AmiDxeLib.h"

//---------------------------------------------------------------------------

#ifndef     SERIAL_WRITE_ERROR_CHECK
#define     SERIAL_WRITE_ERROR_CHECK 1
#ifndef     MAXIMUM_SERIAL_WRITE_ERROR_COUNT
#define     MAXIMUM_SERIAL_WRITE_ERROR_COUNT 10
#endif
#endif

#ifndef     MAX_FAILURES_ALLOWED
#define     MAX_FAILURES_ALLOWED 5
#endif


#define     CONVENTIONAL_MEMORY_TOP     0xA0000
#define     SREDIR_BIN_SIZE             0x3C00

//
//Parameter passed to sredir.bin
//
#define     DISPLAY_SCREEN              0x01
#define     INT10_WORKAROUND            0x02
#define     MMIO_DEVICE                 0x04
#define     SERIAL_READWRITE_CALLBACK   0x08
#define     CTRLI_MAPPING               0x10

#define     MMIO_BYTE_WIDTH             0x00
#define     MMIO_WORD_WIDTH             0x20
#define     MMIO_DWORD_WIDTH            0x40
#define     SERIAL_ERROR_CHECK          0x80
#define     ESC_SEQUENCES               0x0100
#define     CTRLH_MAPPING               0x0200

UINT32      Int10hAddress;
UINT32      Int0chAddress;
UINT32      Int14hAddress;
VOID        *SreDirImageStart = 0;
UINTN       SreDirImageSize = 0;

AMI_LEGACYSREDIR_TABLE      *mLegacySreDirHeader;
EFI_EVENT                   SreDir;
BOOLEAN                     RedirectionStatus=FALSE;
UINTN                       gSreDirImageStart=0;
EFI_LEGACY_BIOS_PROTOCOL    *pLegacy=NULL;
BOOLEAN                     IsCopied = FALSE;
BOOLEAN                     IsDataCopied = FALSE;
VOID                        *ProcessOpRomRegistration = NULL;
AMI_LEGACY_SREDIR_PROTOCOL  *LegacySredir=NULL;
extern BOOLEAN              IsPciDevice;
BOOLEAN                     LegacyOSBooting=FALSE;

//
// The GetSetupValuesForLegacySredir procedure to get the setup values
//
extern
EFI_STATUS GetSetupValuesForLegacySredir (
    OUT AMI_COM_PARAMETERS  *AmiComParameters
);

EFI_STATUS 
InitilizeNonCommonSerialRegsiters (
    IN  AMI_COM_PARAMETERS  *AmiComParameters
);


EFI_STATUS
EnableLegacySredir (
    IN  AMI_LEGACY_SREDIR_PROTOCOL  *This
);

EFI_STATUS
DisableLegacySredir (
    IN  AMI_LEGACY_SREDIR_PROTOCOL  *This
);

EFI_STATUS
LegacySreDirInSmmFunction (
    IN  EFI_HANDLE          ImageHandle,
    IN  EFI_SYSTEM_TABLE    *SystemTable
 );

EFI_STATUS
FindLegacySredirRom (
    UINT16  ModuleId,
    UINT16  VendorId,
    UINT16  DeviceId,
    VOID    **ImageStart,
    UINT32  *ImageSize
);

EFI_STATUS
DataCopiedToCsm16Function (
    UINTN   StructStartAddress,
    AMI_COM_PARAMETERS  *AmiComParameters
);

//
// AMI_LEGACY_SREDIR_PROTOCOL
//
AMI_LEGACY_SREDIR_PROTOCOL gLegacySredir = {
    EnableLegacySredir,
    DisableLegacySredir
};
#define DLAB_BIT            1   
#define LSR_OFFSET          0x05
#define LCR_OFFSET          0x03
#define DIV_MSB             0x01
#define DIV_LSB             0

//
// ACPI PM timer signature,for 'TM' the ASCII code is 0x544D
// the MSB bit of the signature denotes 24bit/32bit timer
//
#if ACPI_SUPPORT
#if FACP_FLAG_TMR_VAL_EXT
#define TIMER_SIGNATURE_32BIT 0xD44D0000
#else
#define TIMER_SIGNATURE_24BIT 0x544D0000
#endif
#endif

/**
    This function copies the segment and offset address of sredir.bin

    @param  StructStartAddress
    @param  AmiComParameters

    @retval EFI_SUCCESS
*/

EFI_STATUS
DataCopiedToCsm16Function (
    IN  UINTN                   StructStartAddress,
    IN  AMI_COM_PARAMETERS      *AmiComParameters
)
{
    EFI_STATUS                      Status;
    UINTN                           ImageSize = 0x10;
    UINT32                          LockUnlockAddr, LockUnlockSize; 
    UINT16                          *Addr;
    UINT32                          Addr16;
    EFI_LEGACY_BIOS_EXT_PROTOCOL    *BiosExtensions = NULL;
    UINT8                           *RpSBCSig = "$SBC";
    UINT8                           *RpSBFSig = "$SBF";

    //
    // Check Data already copied into CSM16 call back function
    //
    if(IsDataCopied) {
        return EFI_SUCCESS;
    }

    //
    //Data copied into CSM16 call back function
    //
    IsDataCopied=TRUE;

    Status = pBS->LocateProtocol(
                    &gEfiLegacyBiosExtProtocolGuid,\
                    NULL,\
                    &BiosExtensions);

    if (EFI_ERROR(Status)) 
        return Status;

    Status = BiosExtensions->Get16BitFuncAddress(
                                CSM16_OEM_BEFORE_CALL_BOOT_VECTOR, \
                                &Addr16);

    if (EFI_ERROR(Status)) 
        return Status;

    Status = BiosExtensions->UnlockShadow(
                                (UINT8*)Addr16, \
                                ImageSize, \
                                &LockUnlockAddr, \
                                &LockUnlockSize);

    ASSERT_EFI_ERROR(Status);

    while(Addr16 < 0xf0000)
    {
        if (!MemCmp((UINT8*)Addr16, RpSBCSig, 4))
        {
            Addr16 = Addr16+4;// Skip 2 bytes of jmp short (see SerialBootCall.ASM)
            Addr = (UINT16*) Addr16;
            *Addr =((StructStartAddress>>4) & 0xF000);
            Addr++;
            *Addr =(UINT16)StructStartAddress;
            Addr++;
            *Addr = (UINT16)(((UINT8 *)&(AmiComParameters->Flag)) - ((UINT8*)(AmiComParameters)));
            break;
        }
        Addr16++;
    }

    Status = BiosExtensions->Get16BitFuncAddress(
                                CSM16_OEM_ON_BOOT_FAIL, \
                                &Addr16);

    if (EFI_ERROR(Status)) 
        return Status;
    
    while(Addr16 < 0xf0000)
    {
        if (!MemCmp((UINT8*)Addr16, RpSBFSig, 4))
        {
            Addr16 = Addr16+4;// Skip 2 bytes of jmp short (see SredirBootFail.ASM)
            Addr = (UINT16*) Addr16;
            *Addr =((StructStartAddress>>4) & 0xF000);
            Addr++;
            *Addr =(UINT16)StructStartAddress;
            Addr++;
            *Addr = (UINT16)(((UINT8 *)&(AmiComParameters->Flag)) - ((UINT8*)(AmiComParameters))) ;
            break;
        }
        Addr16++;
    }

    BiosExtensions->LockShadow(LockUnlockAddr, LockUnlockSize);

    return EFI_SUCCESS;
}

/**
    This function copies the Sredir.bin into Base Memory

    @param This Indicates the EFI_LEGACY_SREDIR_PROTOCOL instance.

    @retval EFI_SUCCESS
*/

EFI_STATUS
CopySredirBinIntoBaseMemory (
    IN AMI_LEGACY_SREDIR_PROTOCOL   *This
)
{
    UINT16  EbdaSeg = (UINT16)(*(UINT16*)0x40E);
    UINT8   *EbdaAddress = (UINT8*)((UINTN)EbdaSeg<<4);
    UINT32  EbdaSize = (*EbdaAddress)<<10;
    UINT32  NewEbdaAdd;
    UINT32  NewEbdaSeg;

    //
    // Check Sredir.bin already copied into Memory
    //
    if(IsCopied) {
        return EFI_SUCCESS;
    }

    //
    //Sredir.bin copied into Base Memory
    //
    IsCopied=TRUE;

    //
    // Sredir.bin is placed in below A0000. It will not be reported as EBDA memory. 
    // So Sredir place will not be changed when there is allocateEbda.
    //
    if(EbdaSeg== 0) { 
        return EFI_NOT_FOUND;
    }

    //
    //New EBDA Address will be EbdaAddress - Sredir.bin
    //
    NewEbdaAdd = (EbdaSeg<<4)-SREDIR_BIN_SIZE;        //To allocate memory for sredir.bin
    NewEbdaSeg = NewEbdaAdd>>4;

    //
    //We are taking memory for the Sredir.bin from the base memory. So adjust the Base Memory
    //
    *(UINT16*)0x413 = ((*(UINT16*)0x413) - (SREDIR_BIN_SIZE >> 10));

    //
    //Update the New EBDA address
    //
    *(UINT16*)0x40e = (UINT16)NewEbdaSeg;   

    //
    //Move the OLD EBDA data to new EBDA area. We have not changed the EBDA Size here. This makes
    //Sredir.bin area will not be moved any where.it means Sredir.bin is not placed under EBDA area.
    //
    pBS->CopyMem((VOID*)NewEbdaAdd,(VOID*)(EbdaSeg<<4),EbdaSize);
    
    //
    // Copy the Sredir.bin Base Memory.
    //
    pBS->CopyMem((VOID*)(NewEbdaAdd+EbdaSize), SreDirImageStart, SreDirImageSize);
    gSreDirImageStart = (UINTN)(NewEbdaAdd+EbdaSize);  
   
    
    return EFI_SUCCESS;

}

/**
    This Function is used to Initialize the Baud Rate of the COM port 

    @param  AmiComParameters

    @retval EFI_STATUS
*/

EFI_STATUS
InitilizeBuadRate (
    OUT AMI_COM_PARAMETERS  *AmiComParameters
)
{

    UINT8   Data8;
    UINT16  Divisor;
    UINTN   Remainder;
    UINTN   UartInputClock;

    if( IsPciDevice ) {
#ifdef PCI_UART_INPUT_CLOCK
        UartInputClock = PCI_UART_INPUT_CLOCK;
#else
    //
    // Set the default value((24000000/13)MHz input clock)
    // if the PCI_UART_INPUT_CLOCK SDL token is not present.
    //
        UartInputClock=1843200;
#endif
    } else {
#ifdef UART_INPUT_CLOCK
        UartInputClock=UART_INPUT_CLOCK;
#else
    //
    // Set the default value((24000000/13)MHz input clock)
    // if the UART_INPUT_CLOCK SDL token is not present.
    //
        UartInputClock=1843200;
#endif
    }

    //
    // Compute the baud rate divisor.
    //
    Divisor = (UINT32) Div64 (UartInputClock,
                                ((UINT32)AmiComParameters->Baudrate * 16), 
                                &Remainder);
    if ( Remainder ) {
        Divisor += 1;
    }
    
    if ((Divisor == 0) || (Divisor & 0xffff0000)) {
        return EFI_INVALID_PARAMETER;
    }

    //
    // Check for the MMIO device. If it's MMIO device do MMIO access to 
    // Read and Write to the COM port Registers. Otherwise use IO access
    // to Read and Write to COM port Registers.
    //    
    if(AmiComParameters->MMIOBaseAddress != 0) {
        //
        // Program Serial port. 
        // Set Line Control Register (LCR)
        //
        Data8 = DLAB_BIT << 7;

#if COM_MMIO_WIDTH == 4
        *(UINT32*)(AmiComParameters->MMIOBaseAddress+(LCR_OFFSET*COM_MMIO_WIDTH))=(UINT32)Data8;
#else
    #if COM_MMIO_WIDTH == 2
            *(UINT16*)(AmiComParameters->MMIOBaseAddress+(LCR_OFFSET*COM_MMIO_WIDTH))=(UINT16)Data8;
    #else
            *(UINT8*)(AmiComParameters->MMIOBaseAddress+(LCR_OFFSET*COM_MMIO_WIDTH))=(UINT8)Data8;
    #endif
#endif  
    
        //
        //Program the Baud Rate
        //

#if COM_MMIO_WIDTH == 4
        *(UINT32*)(AmiComParameters->MMIOBaseAddress+(DIV_LSB*COM_MMIO_WIDTH))=(UINT32)Divisor & 0xFF;
        *(UINT32*)(AmiComParameters->MMIOBaseAddress+(DIV_MSB*COM_MMIO_WIDTH))=(UINT32)Divisor >> 8;
#else
    #if COM_MMIO_WIDTH == 2
            *(UINT16*)(AmiComParameters->MMIOBaseAddress+(DIV_LSB*COM_MMIO_WIDTH))=(UINT16)Divisor & 0xFF;
            *(UINT16*)(AmiComParameters->MMIOBaseAddress+(DIV_MSB*COM_MMIO_WIDTH))=(UINT16)Divisor >> 8;

    #else
            *(UINT8*)(AmiComParameters->MMIOBaseAddress+(DIV_LSB*COM_MMIO_WIDTH))=(UINT8)Divisor & 0xFF;
            *(UINT8*)(AmiComParameters->MMIOBaseAddress+(DIV_MSB*COM_MMIO_WIDTH))=(UINT8)(Divisor >> 8);
    #endif
#endif  

        //
        // Clear DLAB bit in LCR
        //
        Data8 &= ~((UINT8)DLAB_BIT << 7);

#if COM_MMIO_WIDTH == 4
        *(UINT32*)(AmiComParameters->MMIOBaseAddress+(LCR_OFFSET*COM_MMIO_WIDTH))=(UINT32)Data8;
#else
    #if COM_MMIO_WIDTH == 2
            *(UINT16*)(AmiComParameters->MMIOBaseAddress+(LCR_OFFSET*COM_MMIO_WIDTH))=(UINT16)Data8;
    #else
            *(UINT8*)(AmiComParameters->MMIOBaseAddress+(LCR_OFFSET*COM_MMIO_WIDTH))=(UINT8)Data8;
    #endif
#endif  

    } else {
        //
        // Program Serial port. 
        // Set Line Control Register (LCR)
        //
        Data8 = DLAB_BIT << 7;
        IoWrite8(AmiComParameters->BaseAddress + LCR_OFFSET, Data8 );
    
        //
        //Program the Baud Rate
        //
        IoWrite8(AmiComParameters->BaseAddress + DIV_LSB, Divisor & 0xFF);
        IoWrite8(AmiComParameters->BaseAddress + DIV_MSB, Divisor >> 8);
    
        //
        // Clear DLAB bit in LCR
        //
        Data8 &= ~((UINT8)DLAB_BIT << 7);
        IoWrite8(AmiComParameters->BaseAddress + LCR_OFFSET, Data8 );
    }

    return EFI_SUCCESS;
}

/**
    This function is used for enabling the legacy Serial Redirection
    This function contains calls for Initializing Serial ports and 
    hooking the interrupts which are required for purpose of Redirection

    @param  This Indicates the AMI_LEGACY_SREDIR_PROTOCOL instance.

    @retval EFI_SUCCESS
*/

EFI_STATUS
EnableLegacySredir (
    IN  AMI_LEGACY_SREDIR_PROTOCOL  *This
)
{
    EFI_STATUS              Status = EFI_SUCCESS;
    EFI_IA32_REGISTER_SET   RegSet;
    AMI_COM_PARAMETERS      AmiComParameters;
    UINT32                  SdlParameters=0;
    AMI_COM_PARAMETERS      *SredirSetupComParameters;
    UINTN                   StructStartAddress = 0;
    
    if(RedirectionStatus) {
        return EFI_NOT_FOUND;
    }
    
    if(pLegacy==NULL) {
        Status = pBS->LocateProtocol(&gEfiLegacyBiosProtocolGuid, NULL, &pLegacy);
        if (EFI_ERROR(Status)) {
            return Status;
        }
    }

    //
    //Getting Setup values from terminal module
    //
    Status = GetSetupValuesForLegacySredir(&AmiComParameters);

    if(EFI_ERROR(Status)){
        return Status;
    }
    //
    //Copy the Sredir.bin into Base Memory
    //
    Status=CopySredirBinIntoBaseMemory(This);

    if(EFI_ERROR(Status)){
        return Status;
    }

    //
    //Serial Redirection starting Address
    //
    mLegacySreDirHeader = (AMI_LEGACYSREDIR_TABLE*)gSreDirImageStart;

    if (mLegacySreDirHeader == NULL) return EFI_NOT_FOUND;

#if DISPLAY_WHOLE_SCREEN
    SdlParameters |= DISPLAY_SCREEN;
#endif
#if TRAP_INT10_WORKAROUND
    SdlParameters |= INT10_WORKAROUND;
#endif
#if SERIAL_READ_WRITE_CALLBACK
    SdlParameters |= SERIAL_READWRITE_CALLBACK;
#endif
#if CTRLI_KEY_MAPPING
    SdlParameters |= CTRLI_MAPPING;
#endif
#if CTRLH_KEY_MAPPING
    SdlParameters |= CTRLH_MAPPING;
#endif
#if SERIAL_WRITE_ERROR_CHECK
    SdlParameters |= SERIAL_ERROR_CHECK;
#endif
#if OEM_ESC_SEQUENCES
    SdlParameters |= ESC_SEQUENCES;
#endif
#if COM_MMIO_WIDTH == 1
    SdlParameters |= MMIO_BYTE_WIDTH;
#endif
#if COM_MMIO_WIDTH == 2
    SdlParameters |= MMIO_WORD_WIDTH;
#endif
#if COM_MMIO_WIDTH == 4
    SdlParameters |= MMIO_DWORD_WIDTH;
#endif


    Status=InitilizeBuadRate(&AmiComParameters);

    if(EFI_ERROR(Status)){
        return Status;
    }

    //
    //Initialize the Non Standard Serial Port Registers.
    //
    Status = InitilizeNonCommonSerialRegsiters(&AmiComParameters);

    if(EFI_ERROR(Status)){
        return Status;
    }

    if(AmiComParameters.MMIOBaseAddress != 0) {
        SdlParameters |= MMIO_DEVICE;
        AmiComParameters.BaseAddress = 0;
        AmiComParameters.SwSMIValue = LEGACY_SREDIR_SWSMI;
#if defined(SW_SMI_IO_ADDRESS)
        AmiComParameters.SwSMIPort = SW_SMI_IO_ADDRESS;
#endif
    } 
    
    AmiComParameters.SdlParameters = SdlParameters;

    AmiComParameters.SredirBinSize=SREDIR_BIN_SIZE;
    AmiComParameters.RefreshScreenKey = REFRESH_SCREEN_KEY;
    AmiComParameters.UartPollingRedirection = UART_POLLING_REDIRECTION;
    
    // InstallLegacyOSthroughRemote is valid only when booting to Legacy OS
    // Not valid for Option rom Redirection.
    if(!LegacyOSBooting) {
        AmiComParameters.InstallLegacyOSthroughRemote = 0;
    }
    
    //
    // Transfer the Setup Values and COM Parameters from EFI to Legacy.
    //

    SredirSetupComParameters = (AMI_COM_PARAMETERS  *)(gSreDirImageStart + (mLegacySreDirHeader->SreDirEfiToLegacyOffset));

    pBS->CopyMem((VOID*)SredirSetupComParameters, (VOID*)&AmiComParameters, sizeof(AMI_COM_PARAMETERS));

    StructStartAddress = (UINTN )SredirSetupComParameters;

    //
    // It is commented as the csm16 had some problem. we have commented out csm16 call back also in sdl file.
    // Will be done in future.
    //
    if(LegacyOSBooting) {
        Status=DataCopiedToCsm16Function(StructStartAddress, &AmiComParameters);

        if(EFI_ERROR(Status)){
            return Status;
        }
    }
    pBS->SetMem(&RegSet, sizeof (EFI_IA32_REGISTER_SET), 0);
    RegSet.X.AX = LEGACY_SreDirInitializeSerialPort; 

    RegSet.X.BX = MAXIMUM_SERIAL_WRITE_ERROR_COUNT;
    RegSet.X.CX = MAX_FAILURES_ALLOWED;
#if ACPI_SUPPORT
    #if FACP_FLAG_TMR_VAL_EXT
        RegSet.E.EDX = TIMER_SIGNATURE_32BIT;
    #else
        RegSet.E.EDX = TIMER_SIGNATURE_24BIT;
    #endif
#if ACPI_TIMER_IN_LEGACY_SUPPORT
     RegSet.X.DX = PM_TMR_BLK_ADDRESS;
#else
    RegSet.X.DX = 0;
#endif
#endif
    
    Status = pLegacy->FarCall86(pLegacy,
             (UINT16)(gSreDirImageStart>>4), 
             mLegacySreDirHeader->SreDirOffset, 
             &RegSet, 
             NULL, 
             0);
    if(EFI_ERROR(Status)){
        return Status;
    }
      
    pBS->SetMem(&RegSet, sizeof (EFI_IA32_REGISTER_SET), 0);
    RegSet.X.AX = LEGACY_SerialRedirection;  


    Status = pLegacy->FarCall86(pLegacy, 
             (UINT16)(gSreDirImageStart>>4), 
             mLegacySreDirHeader->SreDirOffset, 
             &RegSet, 
             NULL, 
             0);

/*
    //
    // Intel wants to have interface to get the Original Int10, Int0c and Int14
    // Address. By default this code is disabled.
    //
    pBS->SetMem(&RegSet, sizeof (EFI_IA32_REGISTER_SET), 0);

    RegSet.X.AX = LEGACY_GetInterruptAddress;   //returns Int10,Int0c,Int14 original
                                                //vector address in EAX,EBX and ECX

    Status = pLegacy->FarCall86(pLegacy, 
             (UINT16)(gSreDirImageStart>>4), 
             mLegacySreDirHeader->SreDirOffset, 
             &RegSet, 
             NULL, 
             0);
         
    Int10hAddress = RegSet.E.EAX;
    Int0chAddress = RegSet.E.EBX;
    Int14hAddress = RegSet.E.ECX;
*/
    RedirectionStatus=TRUE;

    return Status;
}


/**
    This function is used to disable the Legacy Serial redirection
    This function contains calls for functions which are used to 
    release the interrupts which are used for Serial Redirection

    @param  This Indicates the AMI_LEGACY_SREDIR_PROTOCOL instance.

    @retval EFI_SUCCESS

*/

EFI_STATUS
DisableLegacySredir (
    IN  AMI_LEGACY_SREDIR_PROTOCOL   *This
)
{
    EFI_STATUS                  Status = EFI_SUCCESS;
    EFI_IA32_REGISTER_SET       RegSet;

    if(!RedirectionStatus) {
        return EFI_NOT_FOUND;
    }

    if(pLegacy==NULL) {
        Status = pBS->LocateProtocol(&gEfiLegacyBiosProtocolGuid, NULL, &pLegacy);
        if (EFI_ERROR(Status)) { 
            return Status;
        }
    }
 
    //
    //Serial Redirection starting Address
    //
    mLegacySreDirHeader = (AMI_LEGACYSREDIR_TABLE*)gSreDirImageStart;

    if (mLegacySreDirHeader == NULL) {
        return EFI_NOT_FOUND;
    }
 
    pBS->SetMem(&RegSet, sizeof (EFI_IA32_REGISTER_SET), 0);
    RegSet.X.AX = LEGACY_ReleaseSerialRedirection;  

    Status = pLegacy->FarCall86(pLegacy, 
             (UINT16)(gSreDirImageStart>>4), 
             mLegacySreDirHeader->SreDirOffset, 
             &RegSet, 
             NULL, 
             0);

    RedirectionStatus=FALSE;

    return Status;
}


/**
    This function is used for Legacy Serial Redirection

    @param EFI_Hanlde - Image Handle
    @param EFI_System_Table - Pointer to System Table

    @retval EFI_SUCCESS or EFI_NOT_FOUND

*/
EFI_STATUS
Legacy_OS_SerialRedirection (
    IN  EFI_EVENT   Event,
    IN  VOID        *Context
)   
{

    EFI_STATUS  Status;

    if(!LegacySredir) {
        Status=pBS->LocateProtocol(&gEfiLegacySredirProtocolGuid, NULL, &LegacySredir);
        if(EFI_ERROR(Status)) {
            return Status;
        }
    }

    LegacyOSBooting=TRUE;
    Status = LegacySredir->EnableLegacySredir(LegacySredir);

    return Status;

}

#if CLEAR_LEGACYSREDIR_KB_BUFFER_AT_READYTOBOOT
/**
    Function to clear the Keyboard character buffer
    (in the 16-bit Serial Redirection module)

    @param Event 
    @param Context 

    @retval VOID

*/
VOID
ClearKbCharBuffer (
    IN  EFI_EVENT   Event,
    IN  VOID        *Context
)
{
    EFI_IA32_REGISTER_SET   RegSet;

    //
    // Serial Redirection starting Address
    //
    mLegacySreDirHeader = (AMI_LEGACYSREDIR_TABLE*)gSreDirImageStart;

    if (mLegacySreDirHeader != NULL) {
        pBS->SetMem(&RegSet, sizeof (EFI_IA32_REGISTER_SET), 0);
        RegSet.X.AX = LEGACY_ClearKbCharBuffer;  

         pLegacy->FarCall86(pLegacy, 
                         mLegacySreDirHeader->SreDirSegment, 
                         mLegacySreDirHeader->SreDirOffset, 
                         &RegSet, 
                         NULL, 
                         0);
    }

    pBS->CloseEvent(Event);
}
#endif

/**
    This call back will be called before and after installing legacy
    OpROM. Before installing will enable Legacy redirection and
    after installing Legacy Redirection will be disabled.

    @param Event Call back event
    @param Context pointer to calling context

    @retval VOID

*/
VOID
ProcessOpRomCallback (
    IN  EFI_EVENT   Event,
    IN  VOID        *Context
)
{
    EFI_STATUS                  Status;
    EFI_HANDLE                  Handle;
    UINTN                       Size = sizeof(EFI_HANDLE);
    CSM_PLATFORM_POLICY_DATA    *OpRomStartEndProtocol;

    //
    // Locate gOpromStartEndProtocolGuid. If interface is NULL, it means OpRom has been
    // already Launched and exited else Option Rom is about to Launch.
    //
    Status = pBS->LocateHandle(ByRegisterNotify, NULL, ProcessOpRomRegistration, &Size, &Handle);
    if (EFI_ERROR(Status)) {
        return;
    }

    Status = pBS->HandleProtocol(Handle, &gOpromStartEndProtocolGuid, &OpRomStartEndProtocol);
    if (EFI_ERROR(Status)) {
        return;
    }
    
    if((OpRomStartEndProtocol != NULL) && (OpRomStartEndProtocol->ExecuteThisRom == FALSE))
        return;
    
    if(!LegacySredir) {
        Status=pBS->LocateProtocol(&gEfiLegacySredirProtocolGuid, NULL, &LegacySredir);
        if(EFI_ERROR(Status)) {
            return;
        }
    }

    if(OpRomStartEndProtocol != NULL) {
        //
        // Option Rom about to launch, Enable Legacy Redirection.
        //
        LegacySredir->EnableLegacySredir(LegacySredir);
    } else {
        //
        // Option Rom has been launched and exited, Disable Legacy Redirection.
        //
        LegacySredir->DisableLegacySredir(LegacySredir);
    }

    return;
}

/**
    Legacy Serial Redirection driver entry point

    @param Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT
        EFI System Table - Pointer to System Table

    @retval EFI_STATUS OR EFI_NOT_FOUND

*/

EFI_STATUS
LegacySredirEntryPoint (
    IN  EFI_HANDLE          ImageHandle,
    IN  EFI_SYSTEM_TABLE    *SystemTable
)
{

    EFI_STATUS                      Status;
    EFI_HANDLE                      NewHandle;
    EFI_LEGACY_BIOS_EXT_PROTOCOL    *BiosExtensions = NULL;
    #if CLEAR_LEGACYSREDIR_KB_BUFFER_AT_READYTOBOOT
        EFI_EVENT                    ReadyToBootEvent;
    #endif
    EFI_EVENT                        Event;

    InitAmiLib(ImageHandle, SystemTable);

    //
    // Get the AHCI INT13 runtime image
    //
    Status = pBS->LocateProtocol(
               &gEfiLegacyBiosExtProtocolGuid, NULL, &BiosExtensions);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    Status = BiosExtensions->GetEmbeddedRom(
                SREDIR_MODULEID, SREDIR_VENDORID, SREDIR_DEVICEID, &SreDirImageStart, &SreDirImageSize);
    if (EFI_ERROR(Status)) {
         return Status;
    }

    NewHandle = NULL;
    Status = pBS->InstallProtocolInterface (
                      &NewHandle,
                      &gEfiLegacySredirProtocolGuid,
                      EFI_NATIVE_INTERFACE,
                      &gLegacySredir
                      );
    ASSERT_EFI_ERROR(Status);
    Status = RegisterProtocolCallback(&gOpromStartEndProtocolGuid,
                     ProcessOpRomCallback,
                     NULL,
                     &Event,
                     &ProcessOpRomRegistration);
    ASSERT_EFI_ERROR(Status);
    #if CLEAR_LEGACYSREDIR_KB_BUFFER_AT_READYTOBOOT
        //
        // Register the ReadyToBoot event function to clear the Keyboard
        // character buffer (in the 16-bit Serial Redirection module)
        //
        CreateReadyToBootEvent(TPL_CALLBACK,
                               ClearKbCharBuffer,
                               NULL,
                               &ReadyToBootEvent
                               );
    #endif
    
    Status = CreateLegacyBootEvent(
                  TPL_CALLBACK,
                  Legacy_OS_SerialRedirection,
                  NULL,
                  &SreDir
        );
    
    ASSERT_EFI_ERROR(Status);
    return Status;


}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
