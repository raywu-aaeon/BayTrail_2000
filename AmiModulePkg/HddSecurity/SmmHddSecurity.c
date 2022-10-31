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
// $Header: /Alaska/SOURCE/Modules/HddSecurity/SmmHddSecurity.c 18    8/22/11 2:06a Anandakrishnanl $
//
// $Revision: 18 $
//
// $Date: 8/22/11 2:06a $
//**********************************************************************

//<AMI_FHDR_START>
//---------------------------------------------------------------------------
//
// Name:        SmmHddSecurity.C
//
// Description:	Provide functions to unlock HDD password during S3 resume
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

#include "SmmHddSecurity.h"

#if ( defined(AHCI_SUPPORT) && (AHCI_SUPPORT != 0) )
EFI_GUID              gAhciSmmProtocolGuid      = AHCI_SMM_PROTOCOL_GUID;
UINT8                 *AhciSecurityBuffer = NULL;
AHCI_BUS_SMM_PROTOCOL *mAhciSmm             = NULL;
#endif

EFI_SMM_SW_DISPATCH2_PROTOCOL *SwDispatch;
EFI_SMM_SYSTEM_TABLE2           *pSmst2;


COMMAND_BUFFER        *mCmdBuffer;
UINT8                 *mDataBuffer        = NULL;
UINT8                 *mDataSmmBuffer     = NULL;
UINT8                 *SecurityBuffer     = NULL;
BOOLEAN               AhciInit            = FALSE;

SECURITY_PROTOCOL     *IdeSecurityInterface = NULL;

#define PCI_CFG_ADDR( bus, dev, func, reg ) \
    ((VOID*)(UINTN) (PCIEX_BASE_ADDRESS\
                     + ((bus) << 20) + ((dev) << 15) + ((func) << 12) + reg))

#define AHCI_BAR    0x24
#define PCI_SCC     0x000A        // Sub Class Code Register


#if defined(EFI64) || defined(EFIx64)         // Check Flag to Select the Function for
                                              // the corresponding Execution Platform
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DivU64x32Local
//
// Description: Dividend input is divided by the Divisor input
//              parameter and Stored the Remainder value in Remainder
//              argument and Returns Quotient.This fucntion is selected in
//              64 bit environment architecture. 
//
// Input:
//              IN UINT64           Dividend,
//              IN UINTN            Divisor,   //Can only be 31 bits.
//              OUT UINTN*Remainder OPTIONAL
//
// Output:      static UINT64
//
// Modified:
//      
// Referrals: 
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static UINT64 DivU64x32Local(
    IN UINT64           Dividend,
    IN UINTN            Divisor,
    OUT UINTN*Remainder OPTIONAL )
{
    UINT64 Result = Dividend / Divisor;

    if ( Remainder ) {
        *Remainder = Dividend % Divisor;
    }
    return Result;
}

#else

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DivU64x32Local
//
// Description: Dividend input is divided by the Divisor input
//              parameter and Stored the Remainder value in Remainder
//              argument and Returns Quotient. This fucntion is selected in
//              32 bit environment architecture.
//
// Input:
//              IN UINT64           Dividend,
//              IN UINTN            Divisor,   //Can only be 31 bits.
//              OUT UINTN*Remainder OPTIONAL
//
// Output:      static UINT64
//
// Modified:
//      
// Referrals: 
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static UINT64 DivU64x32Local(
    IN UINT64           Dividend,
    IN UINTN            Divisor,   //Can only be 31 bits.
    OUT UINTN*Remainder OPTIONAL )
{
    UINT64 Result;
    UINT32 Rem;

    _asm
    {
        mov eax, dword ptr Dividend[0]
        mov edx, dword ptr Dividend[4]
        mov esi, Divisor
        xor     edi, edi                    ; Remainder
        mov     ecx, 64                     ; 64 bits
Div64_loop:
        shl     eax, 1                      ;Shift dividend left. This clears bit 0.
        rcl     edx, 1
        rcl     edi, 1                      ;Shift remainder left. Bit 0 = previous dividend bit 63.

        cmp     edi, esi                    ; If Rem >= Divisor, don't adjust
        cmc                                 ; else adjust dividend and subtract divisor.
        sbb     ebx, ebx                    ; if Rem >= Divisor, ebx = 0, else ebx = -1.
        sub     eax, ebx                    ; if adjust, bit 0 of dividend = 1
        and     ebx, esi                    ; if adjust, ebx = Divisor, else ebx = 0. 
        sub     edi, ebx                    ; if adjust, subtract divisor from remainder.
        loop    Div64_loop

        mov     dword ptr Result[0], eax
        mov     dword ptr Result[4], edx
        mov     Rem, edi
    }

        if (Remainder) *Remainder = Rem;

        return Result;
}
#endif


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   Stall
//
// Description: Stalls for the Required Amount of MicroSeconds
//
// Input:
//          UINTN   Usec    // Number of microseconds to delay
//
// Output:      None
//
// Modified:
//      
// Referrals: 
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID 
Stall (
    UINTN           Usec
 )
{
   UINTN   Counter, i;
   UINT32  Data32, PrevData;
  UINTN   Remainder;

  Counter = (UINTN)DivU64x32Local ((Usec * 10), 3, &Remainder);

  if (Remainder != 0) {
    Counter++;
  }

  //
  // Call WaitForTick for Counter + 1 ticks to try to guarantee Counter tick
  // periods, thus attempting to ensure Microseconds of stall time.
  //
  if (Counter != 0) {

    PrevData = IoRead32(PM_BASE_ADDRESS + 8);
    for (i=0; i < Counter; ) {
       Data32 = IoRead32(PM_BASE_ADDRESS + 8);    
        if (Data32 < PrevData) {        // Reset if there is a overlap
            PrevData=Data32;
            continue;
        }
        i += (Data32 - PrevData);        
        PrevData = Data32;
    }
  }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ZeromemorySmm
//
// Description: Clears the buffer
//
// Input:       void    *Buffer,
//              UINTN   Size
//
// Output:      None
//
// Modified:
//      
// Referrals: 
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
void
ZeroMemorySmm (
    void                            *Buffer,
    UINTN                           Size
 )
{
    UINT8   *Ptr;
    Ptr = Buffer;
    while (Size--) {
        *(Ptr++) = 0;
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SMMWaitforBitClear
//
// Description: Waits for the given bit to be clear
//
// Input:   HDD_PASSWORD            *pHddPassword,
//          UINT8                   BitClear,
//          UINT32                  Timeout
//
// Output:  EFI_STATUS
//
// Modified:
//      
// Referrals: 
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SMMWaitforBitClear (
    HDD_PASSWORD                    *pHddPassword,
    UINT8                           BitClear,
    UINT32                          Timeout
 )
{
    UINT8           Delay;
    UINT8           Data8;

    for ( ; Timeout > 0; Timeout--) {
        for ( Delay = 100; Delay > 0; Delay--) {
            Data8 = IoRead8(pHddPassword->DeviceControlReg);
            if (!(Data8 & BitClear)) return EFI_SUCCESS;
            Stall(10);                      // 10 Usec
        }
    }
    return EFI_TIMEOUT;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SMMWaitforBitSet
//
// Description: Checks for a particular Bit to be set for a given amount 
//              of time
//
// Input:   HDD_PASSWORD            *pHddPassword,
//          UINT8                   BitSet,
//          UINT32                  Timeout
//
// Output:  EFI_STATUS
//
// Modified:
//      
// Referrals: 
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
SMMWaitforBitSet (
    HDD_PASSWORD                    *pHddPassword,
    UINT8                           BitSet,
    UINT32                          TimeOut
 )
{
    UINT8           Delay;
    UINT8           Data8;

    for ( ; TimeOut > 0; TimeOut--) {
        for ( Delay = 100; Delay > 0; Delay--) {
            Data8 = IoRead8(pHddPassword->DeviceControlReg);
            if (Data8 & BitSet) return EFI_SUCCESS;
            Stall(10);                      // 10 Usec
        }
    }
    return EFI_TIMEOUT;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SMMWaitForCmdCompletion
//
// Description: Waits for BSY bit to get clear
//
// Input:   HDD_PASSWORD    *pHddPassword 
//
// Output:  EFI_STATUS
//
// Modified:
//      
// Referrals: 
//
// Notes:   Wait for BSY bit to get clear. Check for any errors.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SMMWaitForCmdCompletion (
    HDD_PASSWORD                    *pHddPassword
 )
{
    UINT8           Data8;
    EFI_STATUS      Status;

//  Read ATL_STATUS and ignore the result. Just a delay
    Data8 = IoRead8(pHddPassword->DeviceControlReg);
    
//  Check for BSY bit to be clear
    Status = SMMWaitforBitClear (   pHddPassword, 
                                    BSY, 
                                    DMA_ATAPI_COMMAND_COMPLETE_TIMEOUT);

    if (EFI_ERROR(Status)) return EFI_DEVICE_ERROR;

//  Check for errors. 
    Data8 = IoRead8(pHddPassword->BaseAddress + 7);

    if (Data8 & (ERR | DF)) return EFI_DEVICE_ERROR;  

    return EFI_SUCCESS;
}

#if ( defined(AHCI_SUPPORT) && (AHCI_SUPPORT != 0) )

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SMMAhciSecurityUnlockCommand
//
// Description: This Function unlocks HDD password during S3 resume in 
//              Ahci Mode Using Int 13.
//
// Input:   HDD_PASSWORD    *pHddPassword 
//
// Output:  EFI_STATUS
//
// Modified:
//      
// Referrals: 
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SMMAhciSecurityUnlockCommand (
    HDD_PASSWORD                    *pHddPassword
 )
{
    UINT8           i;
    UINT32          AhciBar = 0;
    UINT8           SccReg = 0 ;
    BOOLEAN         ControllerinAhciMode = FALSE;
    UINT32          GlobalControl;
    COMMAND_STRUCTURE               CommandStructure;

    if(mAhciSmm == NULL) {
        return EFI_NOT_FOUND;
    }

    //    1. Check if Controller is in AHCI Mode.
    //    2. Read ABAR Offset and Get HbaAddress.
    //    3. Disable AHCI_ENABLE. 
    //    4. Issue Controller Reset. 
    //    5. Wait for HBA Reset to Complete.
    //    6. Enable AHCI_ENABLE.

    if(!AhciInit) {
        SccReg = *(UINT8*)PCI_CFG_ADDR(pHddPassword->BusNo,
                                       pHddPassword->DeviceNo,
                                       pHddPassword->FunctionNo,
                                       PCI_SCC);    // Get Scc Register;

        if((SccReg & 0x06) || (SccReg & 0x04)) { 
            ControllerinAhciMode = TRUE;
        }

        if(ControllerinAhciMode) {
            AhciBar = *(UINT32*)PCI_CFG_ADDR(pHddPassword->BusNo,
                                       pHddPassword->DeviceNo,
                                       pHddPassword->FunctionNo,
                                       AHCI_BAR);
            AhciBar &= 0xFFFFFFF0;

            GlobalControl = MMIO_READ32(AhciBar + 0x04);

            GlobalControl &= 0x7FFFFFFF;
            MMIO_WRITE32(AhciBar + 0x04, GlobalControl);

            GlobalControl = 0x01;
            MMIO_WRITE32(AhciBar + 0x04, GlobalControl);

            Stall(5000);   // 5 milli Sec Delay

            GlobalControl = 0x80000000;
            MMIO_WRITE32(AhciBar + 0x04, GlobalControl);

            Stall(1000000);   // 1 Sec Delay
        }
        AhciInit=TRUE;
    }

    //  Clear the Buffer
    ZeroMemorySmm (AhciSecurityBuffer, 512);
    AhciSecurityBuffer[0] = pHddPassword->Control & 1;;
    if(AhciSecurityBuffer[0]) {
    //Copy 32 Bytes of Password
        for (i = 0; i < IDE_PASSWORD_LENGTH; i++) {
            ((UINT8 *)AhciSecurityBuffer)[i + 2] = pHddPassword->MasterPassword[i];
        }
    } else {
    //Copy 32 Bytes of Password
        for (i = 0; i < IDE_PASSWORD_LENGTH; i++) {
                ((UINT8 *)AhciSecurityBuffer)[i + 2] = pHddPassword->UserPassword[i];
        }
    }
    //
    //Resuming from S3. So bring back the AHCI controller to working state
    //
    mAhciSmm->AhciSmmInitPortOnS3Resume(mAhciSmm,pHddPassword->PortNumber);
    
    //
    //Setup the Unlock command 
    //
    ZeroMemorySmm(&CommandStructure, sizeof(COMMAND_STRUCTURE));
    CommandStructure.SectorCount = 1;
    CommandStructure.LBALow = 0;
    CommandStructure.LBAMid = 0;
    CommandStructure.LBAHigh = 0;
    CommandStructure.Device = 0x40;
    CommandStructure.Command = SECURITY_UNLOCK;
    CommandStructure.Buffer = AhciSecurityBuffer;
    CommandStructure.ByteCount = 512;
    //
    //Issue the unlock command
    //
    mAhciSmm->AhciSmmExecutePioDataCommand( mAhciSmm,
                                            &CommandStructure,
                                            pHddPassword->PortNumber,
                                            0xFF,
                                            ATA,
                                            TRUE);
 
#if DISABLE_SOFT_SET_PREV 
    ZeroMemorySmm (&CommandStructure, sizeof(COMMAND_STRUCTURE));
    CommandStructure.Features = DISABLE_SATA2_SOFTPREV;         // Disable Software Preservation
    CommandStructure.SectorCount = 6;
    CommandStructure.Command = SET_FEATURE_COMMAND;

    mAhciSmm->AhciSmmExecuteNonDataCommand( mAhciSmm,
                                            CommandStructure,
                                            pHddPassword->PortNumber,
                                            0xFF,
                                            ATA
                                            );
#endif

    //
    //Issue the Security Freeze lock command
    //
    ZeroMemorySmm(&CommandStructure, sizeof(COMMAND_STRUCTURE)); 
    CommandStructure.Command = SECURITY_FREEZE_LOCK;
    mAhciSmm->AhciSmmExecuteNonDataCommand( mAhciSmm,
                                            CommandStructure,
                                            pHddPassword->PortNumber,
                                            0xFF,
                                            ATA
                                            );

    //
    //Issue the Device config Freeze lock command 
    //
    ZeroMemorySmm (&CommandStructure, sizeof(COMMAND_STRUCTURE));
    CommandStructure.Command = DEV_CONFIG_FREEZE_LOCK;
    CommandStructure.Features = DEV_CONFIG_FREEZE_LOCK_FEATURES;
    mAhciSmm->AhciSmmExecuteNonDataCommand( mAhciSmm,
                                            CommandStructure,
                                            pHddPassword->PortNumber,
                                            0xFF,
                                            ATA
                                            );

    return EFI_SUCCESS;

}
#endif

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SMMSecurityUnlockCommand
//
// Description: This Function unlocks HDD password during S3 resume.
//
// Input:   HDD_PASSWORD    *pHddPassword 
//
// Output:  EFI_STATUS
//
// Modified:
//      
// Referrals: 
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SMMSecurityUnlockCommand (
    HDD_PASSWORD                    *pHddPassword
 )
{
    EFI_STATUS                      Status;
    UINT8                           Data8;
    UINT8                           i;
    UINT16                          Reg;

    TRACE_IDESMM (( -1, 
                    "SMMSecurityUnlockCommand Device : %x, Reg: %x \n", 
                    pHddPassword->Device << 4, 
                    pHddPassword->BaseAddress
                  ));

//  Disable Interrupt
    IoWrite8(pHddPassword->DeviceControlReg, 2);

//  Select the drive
    IoWrite8(pHddPassword->BaseAddress + 6, pHddPassword->Device << 4);

//  Wait for BSY to go low
    Status = SMMWaitforBitClear (pHddPassword, BSY, S3_BUSY_CLEAR_TIMEOUT);
    if (EFI_ERROR(Status)) return EFI_DEVICE_ERROR;

//  Clear the Buffer
    ZeroMemorySmm (SecurityBuffer, 512);

    SecurityBuffer[0] = pHddPassword->Control & 1;

    if(SecurityBuffer[0]) {

//      Copy 32 Bytes of Password
   
        for (i = 0; i < IDE_PASSWORD_LENGTH; i++) {
            ((UINT8 *)SecurityBuffer)[i + 2] = pHddPassword->MasterPassword[i];
        }
    } else {

//      Copy 32 Bytes of Password
        for (i = 0; i < IDE_PASSWORD_LENGTH; i++) {
                ((UINT8 *)SecurityBuffer)[i + 2] = pHddPassword->UserPassword[i];
        }
    }

    Status = SMMIdeNonDataCommand (pHddPassword, 
                                    0,
                                    0,
                                    0,
                                    0,
                                    0, 
                                    SECURITY_UNLOCK); 
    if (EFI_ERROR(Status)) return EFI_DEVICE_ERROR;

//  Wait for Command completion
    Status = SMMWaitForCmdCompletion (pHddPassword);
    if (EFI_ERROR(Status)) return EFI_DEVICE_ERROR;

//  Check for DRQ
    Status = SMMWaitforBitSet(pHddPassword, DRQ, DRQ_TIMEOUT);
    if (EFI_ERROR(Status)) return EFI_DEVICE_ERROR;

    Reg = pHddPassword->BaseAddress;

//  Status = IdeWriteMultipleWord (pHddPassword->BaseAddress, 256, &SecurityBuffer);
    IoWrite(
        CpuIoWidthFifoUint16,
        Reg,
        256,
        SecurityBuffer
    );

//  Check for errors
    Status = SMMWaitForCmdCompletion (pHddPassword);
    if (EFI_ERROR(Status)) return EFI_DEVICE_ERROR;

//  Check for errors. 
    Data8 = IoRead8 ( pHddPassword->BaseAddress + 7);
    if(Data8 & 0x21) {          // ERR OR DF bit set ?
        return EFI_DEVICE_ERROR;  
    }

//IA32_DEBUG
#if DISABLE_SOFT_SET_PREV || FORCE_HDD_PASSWORD_PROMPT
    Status = SMMIdeNonDataCommand (pHddPassword, 
                                    DISABLE_SATA2_SOFTPREV,
                                    6,
                                    0,
                                    0,
                                    0, 
                                    SET_FEATURE_COMMAND);

            //  Check for errors
            Status = SMMWaitForCmdCompletion (pHddPassword);
            if (EFI_ERROR(Status)) return Status;
#endif

     Status = SMMIdeNonDataCommand (pHddPassword, 
                              DEV_CONFIG_FREEZE_LOCK_FEATURES,
                              0,
                              0,
                              0,
                              0, 
                              DEV_CONFIG_FREEZE_LOCK);
	 ASSERT_EFI_ERROR (Status);

//  Check for errors
        Status = SMMWaitForCmdCompletion (pHddPassword);
        if (EFI_ERROR(Status)) return Status;

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SMMIdeNonDataCommand
//
// Description: Issues Set Feature command (Non Data)
//
// Input:       HDD_PASSWORD    *pHddPassword
//              UINT8           Command
//
// Output:      EFI_STATUS
//
// Modified:
//      
// Referrals: 
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS 
SMMIdeNonDataCommand (
    HDD_PASSWORD                        *pHddPassword,
    IN UINT8                            Features,
    IN UINT8                            SectorCount,
    IN UINT8                            LBALow,
    IN UINT8                            LBAMid,
    IN UINT8                            LBAHigh,
    IN UINT8                            Command
) 
{

    EFI_STATUS                      Status;
    UINT8                           Data8;

//  Select the drive
    IoWrite8(pHddPassword->BaseAddress + 6, pHddPassword->Device << 4);

//  Check for Controller presence
    Data8 = IoRead8(pHddPassword->DeviceControlReg);
    if (Data8 == 0xFF) {
        ASSERT(Data8 == 0xFF);
        return EFI_DEVICE_ERROR;
    }

//  Before Writing to Sector Count Reg, BSY and DRQ bit should be zero
    Status = SMMWaitforBitClear(pHddPassword, BSY, S3_BUSY_CLEAR_TIMEOUT);
    if (EFI_ERROR(Status)) return EFI_DEVICE_ERROR;

//  Check for DRDY
    Status = SMMWaitforBitSet(pHddPassword, DRDY, DRDY_TIMEOUT);
    if (EFI_ERROR(Status)) return EFI_TIMEOUT;

//  Issue command

    IoWrite8 ( pHddPassword->BaseAddress + 1, Features);
    IoWrite8 ( pHddPassword->BaseAddress + 2, SectorCount);
    IoWrite8 ( pHddPassword->BaseAddress + 3, LBALow);
    IoWrite8 ( pHddPassword->BaseAddress + 4, LBAMid);
    IoWrite8 ( pHddPassword->BaseAddress + 5, LBAHigh);
    IoWrite8 ( pHddPassword->BaseAddress + 7, Command);

    return EFI_SUCCESS;

}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   LocateSMMServices
//
// Description: This function Locates the AhciSmm protocl from the Smm
//
// Input:       None
//
// Output:      None
//
// Modified:
//      
// Referrals: 
//
// Notes:	
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS LocateSMMServices(
    IN  EFI_GUID    *VariableGuid,
    IN  VOID **VariablePointer
)
{
    UINTN                       Index;


    for (Index = 0; Index < pSmst2->NumberOfTableEntries; ++Index) {
        if (guidcmp(&pSmst2->SmmConfigurationTable[Index].VendorGuid,VariableGuid) == 0) {
            break;
        }
    }

    if (Index != pSmst2->NumberOfTableEntries) {
        *VariablePointer = pSmst2->SmmConfigurationTable[Index].VendorTable;
        return EFI_SUCCESS;
    }

    return EFI_NOT_FOUND;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   LocateIdeSmmProtocols
//
// Description: This function Locates the protocols and saves in global pointe
//
// Input:       None
//
// Output:      None
//
// Modified:
//      
// Referrals: 
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID LocateIdeSmmProtocols()
{
    EFI_STATUS  Status;

    if(IdeSecurityInterface == NULL) {
        Status = pBS->LocateProtocol (&gIdeSecurityInterfaceGuid, NULL, &IdeSecurityInterface);
        if (EFI_ERROR(Status)) {
            TRACE_IDESMM(((UINTN)TRACE_ALWAYS, "Ide Security Interface not located.\n"));
        }
    }

#if ( defined(AHCI_SUPPORT) && (AHCI_SUPPORT != 0) )
    if(mAhciSmm == NULL) {
        Status=LocateSMMServices(&gAhciSmmProtocolGuid,(VOID **)&mAhciSmm);
        if (EFI_ERROR(Status)) {
            TRACE_IDESMM(((UINTN)TRACE_ALWAYS, "Ahci SMM not located.\n"));
        } else {
            TRACE_IDESMM(((UINTN)TRACE_ALWAYS, "SMM Thunk Protocol located.\n"));
            Status = pBS->AllocatePool (EfiReservedMemoryType,
                                        512,
                                        (VOID**)&AhciSecurityBuffer);
        }

    }
#endif
 
    return;  
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SaveHDDPassword
//
// Description: This function saves the HDD password and other information 
//              necessary to unlock HDD password during S3 Resume.
//
// Input:       DispatchHandle      Handle to the Dispatcher
//              DispatchContext     SW SMM dispatcher context
//
// Output:      None
//
// Modified:
//      
// Referrals: 
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS 
SaveHDDPassword(
    IN EFI_HANDLE                   DispatchHandle,
    IN CONST VOID   *DispatchContext OPTIONAL,
    IN OUT VOID    *CommBuffer      OPTIONAL,
    IN OUT UINTN   *CommBufferSize  OPTIONAL
)
{

    HDD_PASSWORD    *pHddPassword, *StoredHDDList;
    DLINK           *dlink;
    UINT8           i;
    BOOLEAN         UserOrMaster = FALSE;
    BOOLEAN         CheckFlag;
    EFI_STATUS      Status=EFI_SUCCESS;

    LocateIdeSmmProtocols();
    
    
    pHddPassword = (HDD_PASSWORD *)CommBuffer;


    if (!pHddPassword) return Status; 

//  Check if signature is present or not
    if ((UINT32) pHddPassword->Signature != '$HDD') {
        return Status;
    }

//  Check whether info about the HDD is already present
    dlink = gPasswordList.pHead;
    for ( ; dlink; dlink = dlink->pNext) {
        StoredHDDList = OUTTER(dlink, LinkList, HDD_PASSWORD);
        CheckFlag = FALSE;
        if(StoredHDDList->ModeFlag) {
            if (StoredHDDList->PortNumber ==    pHddPassword->PortNumber) {
                CheckFlag = TRUE;
            }
        } else {
            if ((StoredHDDList->BaseAddress ==  pHddPassword->BaseAddress) && 
                (StoredHDDList->Device ==   pHddPassword->Device)) {
                CheckFlag = TRUE;
            }
        }

        if (CheckFlag) {
            UserOrMaster = (BOOLEAN)((pHddPassword->Control) & 0x01);

            if(UserOrMaster){
//              Match has been found. Just copy the Password buffer
                for (i = 0; i < sizeof (pHddPassword->MasterPassword); i++) StoredHDDList->MasterPassword[i] = pHddPassword->MasterPassword[i];
         
            } else {
//              Match has been found. Just copy the Password buffer
                for (i = 0; i < sizeof (pHddPassword->UserPassword); i++) StoredHDDList->UserPassword[i] = pHddPassword->UserPassword[i];
            }
            TRACE_IDESMM (( -1, 
                            "Saved HDDPassword Device : %x, Reg: %x \n", 
                            pHddPassword->Device << 4, 
                            pHddPassword->BaseAddress
                          ));
            return Status;
        }
    }

    //  Allocate memory needed while unlocking the Password. Done only once. 
    //  Same buffer will be reused.
    if (SecurityBuffer == NULL) { 
        pSmst2->SmmAllocatePool(EfiRuntimeServicesData, 512, &SecurityBuffer);
    }

    //  Match has not been found. Allocate memory and copy the buffer.
    if (pSmst2->SmmAllocatePool(EfiRuntimeServicesData, sizeof(HDD_PASSWORD), &StoredHDDList) != EFI_SUCCESS){ 
        return Status;
    }
    
    for (i = 0; i < sizeof (HDD_PASSWORD); i++) ((UINT8 *)StoredHDDList)[i] = ((UINT8 *)pHddPassword)[i];
    DListAdd(&gPasswordList, &StoredHDDList->LinkList); 
    TRACE_IDESMM (( -1, 
                    "Saved HDDPassword Device : %x, Reg: %x \n", 
                    pHddPassword->Device << 4, 
                    pHddPassword->BaseAddress
                 ));
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UnlockHDDPassword
//
// Description: This Function unlocks HDD password during S3 resume.
//
// Input:       DispatchHandle      Handle to the Dispatcher
//              DispatchContext     SW SMM dispatcher context
//
// Output:      None
//
// Modified:
//      
// Referrals: 
//
// Notes:   
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS 
UnlockHDDPassword(
	IN EFI_HANDLE       DispatchHandle,
	IN CONST VOID       *DispatchContext OPTIONAL,
	IN OUT VOID         *CommBuffer OPTIONAL,
	IN OUT UINTN        *CommBufferSize OPTIONAL
)
{
    HDD_PASSWORD    *StoredHDDList;
    DLINK           *dlink;
    EFI_STATUS      Status=EFI_SUCCESS;

    dlink = gPasswordList.pHead;
    for ( ; dlink; dlink = dlink->pNext) {
        StoredHDDList = OUTTER(dlink, LinkList, HDD_PASSWORD);
        if (StoredHDDList->Signature == '$HDD') {
            if(!StoredHDDList->ModeFlag) {
                SMMSecurityUnlockCommand (StoredHDDList);
            } else {
#if ( defined(AHCI_SUPPORT) && (AHCI_SUPPORT != 0) )
                SMMAhciSecurityUnlockCommand (StoredHDDList);
#endif
            }
        }
    }
    AhciInit=FALSE;

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   RemoveHDDPassword
//
// Description: This Function removes HDD password from the internal database
//
// Input:       DispatchHandle      Handle to the Dispatcher
//              DispatchContext     SW SMM dispatcher context
//
// Output:      None
//
// Modified:
//      
// Referrals: 
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
RemoveHDDPassword(
    IN EFI_HANDLE                   DispatchHandle,
    IN  CONST VOID                  *DispatchContext OPTIONAL,
    IN OUT VOID    *CommBuffer      OPTIONAL,
    IN OUT UINTN   *CommBufferSize  OPTIONAL
)
{
    HDD_PASSWORD    *pHddPassword, *StoredHDDList;
    DLINK           *dlink;
    EFI_STATUS      Status=EFI_SUCCESS;

    LocateIdeSmmProtocols();
   
    pHddPassword = (HDD_PASSWORD *)CommBuffer;
    
    if (!pHddPassword) return Status; 

//  Check if signature is present or not
    if ((UINT32) pHddPassword->Signature != '$HDD') {
        return Status;
    }

//  Check whether info about the HDD is already present
    dlink = gPasswordList.pHead;
    for ( ; dlink; dlink = dlink->pNext) {
        StoredHDDList = OUTTER(dlink, LinkList, HDD_PASSWORD);
        if(StoredHDDList->ModeFlag) {
            if (StoredHDDList->PortNumber == pHddPassword->PortNumber) {
                DListDelete(&gPasswordList, &StoredHDDList->LinkList);
                TRACE_IDESMM((-1, "Removed HDDPassword\n"));
            }
        } else {
            if ((StoredHDDList->BaseAddress == pHddPassword->BaseAddress) && 
                (StoredHDDList->Device == pHddPassword->Device)) {
                DListDelete(&gPasswordList, &StoredHDDList->LinkList);
                TRACE_IDESMM((-1, "Removed HDDPassword\n"));
            }
        }

    }
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IdeSmmMiscSmiPassword 
//
// Description: This Function Freeze locks HDD, Issues Disable/Enable Software
//              Settings preservation Feature for Security Supported HDDs.
// Input:       DispatchHandle      Handle to the Dispatcher
//              DispatchContext     SW SMM dispatcher context
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS 
IdeSmmMiscSmiPassword(
	IN EFI_HANDLE       DispatchHandle,
	IN CONST VOID       *DispatchContext OPTIONAL,
	IN OUT VOID         *CommBuffer OPTIONAL,
	IN OUT UINTN        *CommBufferSize OPTIONAL
)
{
    EFI_STATUS      Status;
    HDD_PASSWORD    *StoredHDDList;
    DLINK           *dlink;

    
    TRACE_IDESMM((-1, "Entered Ide Smm Misc Smi Handle \n"));

    dlink = gPasswordList.pHead;
    for ( ; dlink; dlink = dlink->pNext) {
        StoredHDDList = OUTTER(dlink, LinkList, HDD_PASSWORD);
        if (StoredHDDList->Signature == '$HDD') {

            Status = SMMIdeNonDataCommand (StoredHDDList, 
                                    0,
                                    0,
                                    0,
                                    0,
                                    0, 
                                    SECURITY_FREEZE_LOCK);
				ASSERT_EFI_ERROR (Status);

            //	Check for errors
            Status = SMMWaitForCmdCompletion (StoredHDDList);
            if (EFI_ERROR(Status)) return Status;

#if DISABLE_SOFT_SET_PREV || FORCE_HDD_PASSWORD_PROMPT

            Status = SMMIdeNonDataCommand (StoredHDDList, 
                                    DISABLE_SATA2_SOFTPREV,
                                    6,
                                    0,
                                    0,
                                    0, 
                                    SET_FEATURE_COMMAND);

            //	Check for errors
            Status = SMMWaitForCmdCompletion (StoredHDDList);
            if (EFI_ERROR(Status)) return Status;
#endif

        }
    }

    return Status;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   LocateAhciSMM
//
// Description: This function Locates the AhciSMM driver and save pointer globally.
//
// Input:       DispatchHandle      Handle to the Dispatcher
//              DispatchContext     SW SMM dispatcher context
//
// Output:      None
//
// Modified:
//      
// Referrals: 
//
// Notes:   
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS 
LocateAhciSMM(
    IN EFI_HANDLE                   DispatchHandle,
    IN  CONST VOID                  *DispatchContext OPTIONAL
 )
{
    EFI_STATUS      Status=EFI_SUCCESS;

    LocateIdeSmmProtocols();
    return Status;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   InSmmFunction
//
// Description: Called from InstallSmiHandler
//
// Input:
//
//
// Output:      None
//
// Modified:
//      
// Referrals: 
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS InSmmFunction(
	    IN EFI_HANDLE       ImageHandle,
	    IN EFI_SYSTEM_TABLE *SystemTable
 )
{
	EFI_SMM_SW_REGISTER_CONTEXT Save_HDD_Password = {SW_SMI_HDD_PASSWORD_SAVE};
	EFI_SMM_SW_REGISTER_CONTEXT Unlock_HDD_Password = {SW_SMI_HDD_UNLOCK_PASSWORD};
	EFI_SMM_SW_REGISTER_CONTEXT Remove_HDD_Password = {SW_SMI_HDD_PASSWORD_REMOVE};
	EFI_SMM_SW_REGISTER_CONTEXT IdeSmm_MiscSmi_Password = {SW_SMI_HDD_MISC_SMM_FEATURES};
	EFI_SMM_SW_REGISTER_CONTEXT Locate_Ahci_SMM = {SW_SMI_AHCI_LOCATE_AHCI_SMM};

    EFI_STATUS  Status;
    EFI_HANDLE  Handle;

    Status = pSmst2->SmiHandlerRegister(
        (VOID *)SaveHDDPassword,
        &gSaveHddPasswordGuid,
        &Handle
        );
	
    if (EFI_ERROR(Status)) {
        ASSERT_EFI_ERROR(Status);
        return Status;	
    }

    Status = pSmst2->SmiHandlerRegister(
        (VOID *)RemoveHDDPassword,
        &gRemoveHddPasswordGuid,
        &Handle
        );

    if (EFI_ERROR(Status)) {
        ASSERT_EFI_ERROR(Status);
        return Status;	
    }

    Status = SwDispatch->Register(
        SwDispatch,
        (VOID *)UnlockHDDPassword,
        &Unlock_HDD_Password,
        &Handle
    );
    if (EFI_ERROR(Status)) {
        ASSERT_EFI_ERROR(Status);
        return Status;	
    }

    Status = SwDispatch->Register(
        SwDispatch,
        (VOID *)IdeSmmMiscSmiPassword ,
        &IdeSmm_MiscSmi_Password ,
        &Handle
    );
    if (EFI_ERROR(Status)) {
        ASSERT_EFI_ERROR(Status);
        return Status;
    }

    Status = SwDispatch->Register(
        SwDispatch,
        (VOID *)LocateAhciSMM ,
        &Locate_Ahci_SMM ,
        &Handle
    );
    if (EFI_ERROR(Status)) {
        ASSERT_EFI_ERROR(Status);
        return Status;
    }

    TRACE_IDESMM((-1, "IDESMM InSmmFunction %r\n", Status));

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   HddHDDPasswordSMMInit
//
// Description: Initializes HDD Password SMM Drivers.
//
// Input:
//
// Output:
//      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//  Here is the control flow of this function:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS HddHDDPasswordSMMInit(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
 )
{
    EFI_STATUS Status;
    EFI_SMM_BASE2_PROTOCOL          *gSmmBase2;

    InitAmiSmmLib(ImageHandle,SystemTable);
    
    Status = pBS->LocateProtocol(&gEfiSmmBase2ProtocolGuid, NULL, &gSmmBase2);
    
    if (EFI_ERROR(Status)) { 
        return Status;
    }

    Status = gSmmBase2->GetSmstLocation( gSmmBase2, &pSmst2);
    
    if (EFI_ERROR(Status)) {  
        return EFI_UNSUPPORTED;
    }

    Status  = pSmst2->SmmLocateProtocol( &gEfiSmmSwDispatch2ProtocolGuid, \
                                          NULL, \
                                          &SwDispatch );
    
    if (EFI_ERROR(Status)) {  
        return EFI_UNSUPPORTED;
    }
    

    DListInit(&gPasswordList);

    return (InitSmmHandler (ImageHandle, SystemTable, InSmmFunction, NULL));
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

