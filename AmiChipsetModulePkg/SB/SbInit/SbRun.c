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
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        SbRun.c
//
// Description: This file contains code for SouthBridge runtime
//              protocol
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

// Module specific Includes

#include "Efi.h"
#include "token.h"
#include <AmiDxeLib.h>
#include <Library/SbCspLib.h>
#include <Library/AmiChipsetRuntimeServiceLib.h>
#include <Sb.h>
#include <Library/PcdLib.h>

// Produced Protocols
#include <Protocol/Metronome.h>
#include <Protocol/RealTimeClock.h>

//----------------------------------------------------------------------------
//          Variable Declaration
//----------------------------------------------------------------------------

// Function Prototypes
EFI_STATUS
WaitForTick(
    IN  EFI_METRONOME_ARCH_PROTOCOL     *This,
    IN  UINT32                          TickNumber
);

// Architectural Protocol Definitions
EFI_METRONOME_ARCH_PROTOCOL mMetronomeProtocol = {
    WaitForTick,
    1
};

// Function Definitions

//----------------------------------------------------------------------------
//   USUALLY NO PORTING REQUIRED FOR THE FOLLOWING ROUTINES
//----------------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   WaitForTick
//
// Description: This function calculates the time needed to delay and then
//              calls a library function to delay that amount of time
//
// Input:       IN EFI_METRONOME_ARCH_PROTOCOL *This - Pointer to the instance of
//                                                     the Metronome Arch Protocol
//              IN UINT32 TickNumber - Number of ticks needed based off of tick period
//                                     defined in Protocol Definiton
//
// Output:      Return Status based on errors that occurred while waiting for
//              time to expire.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
WaitForTick(
    IN  EFI_METRONOME_ARCH_PROTOCOL     *This,
    IN  UINT32                          TickNumber
)
{
    EFI_STATUS          Status;
    UINT32              TotalTime;

    // Manipulate TickNumber into a valid value for the library function call
    // the Current Resolution is 10us.
    // The Library uses Microseconds to count delayed time.
    TotalTime = (TickNumber * This->TickPeriod) / 10;

    // Make Library Function call here
    Status = CountTime(TotalTime, PcdGet16(PcdAcpiIoPortBaseAddress));

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SbRuntimeServiceInit
//
// Description: Submit runtime services both SMM and runtime.
//
// Input:       IN EFI_HANDLE ImageHandle - Image handle
//              IN EFI_SYSTEM_TABLE *SystemTable - Pointer to the system table
//
// Output:      Return Status based on errors that occurred while waiting for
//              time to expire.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SbRuntimeServiceInit(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    pRS->ResetSystem    = AmiChipsetResetSystem;
    pRS->GetTime        = AmiChipsetGetTime;
    pRS->SetTime        = AmiChipsetSetTime;
    pRS->GetWakeupTime  = AmiChipsetGetWakeupTime;
    pRS->SetWakeupTime  = AmiChipsetSetWakeupTime;

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SbRuntimeInitEntryPoint
//
// Description: This function is the entry point for this DXE. This function
//              installs the runtime services related to SB
//
// Input:       IN EFI_HANDLE ImageHandle - Image handle
//              IN EFI_SYSTEM_TABLE *SystemTable - Pointer to the system table
//
// Output:      Return Status based on errors that occurred while waiting for
//              time to expire.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SbRuntimeInitEntryPoint(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS              Status;
    EFI_HANDLE              Handle = NULL;

    InitAmiRuntimeLib(ImageHandle, SystemTable, NULL, NULL);

    PROGRESS_CODE(DXE_SBRUN_INIT);
    //
    // Install runtime services
    //
    Status = SbRuntimeServiceInit(ImageHandle, SystemTable);
    ASSERT_EFI_ERROR(Status);

    Status = pBS->InstallProtocolInterface(
                 &ImageHandle, \
                 &gEfiMetronomeArchProtocolGuid, \
                 EFI_NATIVE_INTERFACE, \
                 &mMetronomeProtocol
             );
    ASSERT_EFI_ERROR(Status);
    //
    // This protocol is to notify core that the Runtime Table has been
    // updated, so it can update the runtime table CRC.
    //
    Handle = NULL;
    return pBS->InstallMultipleProtocolInterfaces(
               &Handle, \
               &gEfiRealTimeClockArchProtocolGuid, \
               NULL, \
               NULL
           );
}

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
