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

//*************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name: SbCspDxeLib.c
//
// Description: This file contains code for Sorth Bridge platform
//              initialization in the Library stage
//
//<AMI_FHDR_END>
//*************************************************************************

//-------------------------------------------------------------------------
// Include(s)
//-------------------------------------------------------------------------

//EIP134904 >>
#ifndef SBCSP_PEIM
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Guid/EventGroup.h>
#include <Protocol\ExitPmAuth.h>
#include <Protocol\DxeSmmReadyToLock.h> //EIP130725
#endif
#include <Token.h>
#include <PchAccess.h>
#include <Library/DebugLib.h>

//EIP134904 <<

//---------------------------------------------------------------------------
// Variable and External Declaration(s)
//---------------------------------------------------------------------------
// Variable Declaration(s)
volatile UINT8  *mSpiBase = (UINT8*)(SPI_BASE_ADDRESS);
volatile UINT8  *mIlbBase = (UINT8*)(ILB_BASE_ADDRESS);


#ifdef SBCSP_RUNTIME
EFI_EVENT mSbCspDxeLibVirtualAddressChangeEvent;
#endif

#ifndef SBCSP_PEIM //EIP134904
//<AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure:   SignalExitPmAuthProtocolEvent
//
// Description: The elink will signal gExitPmAuthProtocolGuid Event.
//
// Parameters:  VOID
//
//
// Returns:     None
//
//-------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID SignalExitPmAuthProtocolEvent (
  VOID
)
{
	EFI_HANDLE  Handle = NULL;

	//
	// Signaling gExitPmAuthProtocolGuid Event
	//
	gBS->InstallProtocolInterface (
	      &Handle,
	      &gExitPmAuthProtocolGuid,
	      EFI_NATIVE_INTERFACE,
	      NULL
	      );
//EIP130725 >>
/*
	gBS->UninstallProtocolInterface (
			Handle,
			&gExitPmAuthProtocolGuid,
			NULL
        	);
*/	
	//
	// Signaling gEfiDxeSmmReadyToLockProtocolGuid Event
	//
	Handle = NULL;
	gBS->InstallProtocolInterface (
	      &Handle,
	      &gEfiDxeSmmReadyToLockProtocolGuid,
	      EFI_NATIVE_INTERFACE,
	      NULL
	      );
//EIP130725 <<
}
#endif //EIP134904

//Generic Flash part porting hooks

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ChipsetFlashDeviceWriteEnable
//
// Description: This function is invoked to do any chipset specific operations
//              that are necessary when enabling the Flash Part for writing
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ChipsetFlashDeviceWriteEnable(
    VOID
)
{
    UINT8       Data8;

    Data8 = *(volatile UINT8*)( mSpiBase + R_PCH_SPI_BCR );
    Data8 &= (UINT8) ~B_PCH_SPI_BCR_SMM_BWP;
    Data8 |= (UINT8) B_PCH_SPI_BCR_BIOSWE;
    *(volatile UINT8*)( mSpiBase + R_PCH_SPI_BCR ) = Data8;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ChipsetFlashDeviceWriteDisable
//
// Description: This function is invoked to do any chipset specific operations
//              that are necessary when disabling the Flash Part for writing
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ChipsetFlashDeviceWriteDisable(
    VOID
)
{
    UINT8       Data8;

    Data8 = *(volatile UINT8*)( mSpiBase + R_PCH_SPI_BCR );
    Data8 &= (UINT8) ~B_PCH_SPI_BCR_BIOSWE;
    *(volatile UINT8*)( mSpiBase + R_PCH_SPI_BCR ) = Data8;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SbLibEnableAltAccessMode
//
// Description: Enable/Disable Alternate access mode. When Alternate access
//              mode is enabled, port 70h can be readable.
//
// Input:       Control     Control Alternate access mode
//                          TRUE: Enable Alternate access mode
//                          FALSE: Disable Alternate access mode
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID SbLibEnableAltAccessMode (
    IN BOOLEAN        Control
    )
{
    UINT32            Data32;

    Data32 = *(volatile UINT32*)( mIlbBase + R_PCH_ILB_MC );

    if (Control == TRUE) {
      //
      // Enable Alternate access mode, let port 70h can be readable.
      //
      Data32 |= (UINT32) (B_PCH_ILB_MC_AME);
    } else {
      //
      // Disable Alternate access mode.
      //
      Data32 &= (UINT32) ~(B_PCH_ILB_MC_AME);
    }

    *(volatile UINT32*)( mIlbBase + R_PCH_ILB_MC ) = Data32;

    ///
    /// Reads back for posted write to take effect
    ///
    Data32 = *(volatile UINT32*)( mIlbBase + R_PCH_ILB_MC );
}

#ifdef SBCSP_RUNTIME
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SpiDeviceVirtualFixup
//
// Description: This function will be invoked by the core to convert
//              runtime pointers to virtual address
//
// Input:       *pRS    Pointer to runtime services
//
// Output:      None
//
// Return:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
SbSpiDeviceVirtualFixup (
    IN EFI_EVENT                Event,
    IN VOID                     *Context
)
{
    EfiConvertPointer (0, (VOID **) &mSpiBase);
    EfiConvertPointer (0, (VOID **) &mIlbBase);
    return;
}

EFI_STATUS
EFIAPI
SbCspDxeLibConstructor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS  Status;

  ASSERT (gBS != NULL);
  //
  // Register notify function for EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  SbSpiDeviceVirtualFixup,
                  NULL,
                  &gEfiEventVirtualAddressChangeGuid,
                  &mSbCspDxeLibVirtualAddressChangeEvent
                  );

  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SbCspDxeLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  ASSERT (gBS != NULL);
  Status = gBS->CloseEvent (mSbCspDxeLibVirtualAddressChangeEvent);

  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
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
