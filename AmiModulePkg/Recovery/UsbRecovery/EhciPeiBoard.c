//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
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
//
// $Header: $
//
// $Revision: $
//
// $Date: $
//
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        EhciPeiBoard.c
//
// Description: This file contains routines that are board specific for
//              initializing the EHCI controller in the USB Recovery module.
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>


#include <Pei.h>
#include <AmiPeiLib.h>
#include <Token.h>
#include <PeiGetUCtrl.h>
#include <OhciPPI.h>
#include <Ppi/Stall.h>
#include "EhciPei.h"
#if (PEI_EHCI_SUPPORT == 1)

//----------------------------------------------------------------------------
// Porting Steps:
//
// 1)  define/undefine the appropriate southbridge type
// 2)  define registers and their set/clear bits for each register to be
//     modified in each controller's PCI configuration space
// 3)  do not do any generic initialization - programming of BAR and command
//     register is done generically
//
//----------------------------------------------------------------------------



//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure: EhciPeiBoardInit
//
// Description:
//      This is a porting hook for board-specific EHCI controller
//      initialization
//
// Input:
//      IN EFI_PEI_SERVICES **PeiServices -
//      IN EFI_PEI_PCI_CFG_PPI *Pci -
//      IN EFI_PEI_STALL_PPI *StallPpi -
//
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EhciPeiBoardInit (
    IN EFI_PEI_SERVICES    **PeiServices,
    IN EFI_PEI_PCI_CFG2_PPI *Pci,
    IN EFI_PEI_STALL_PPI   *StallPpi )
{
    EFI_STATUS Status = EFI_SUCCESS;

    return Status;

}

#endif
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
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
