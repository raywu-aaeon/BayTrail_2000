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
//*************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  SbPciDxeLib.C
//
// Description: Library Class for Pci Dxe Init Driver.
//
//
//<AMI_FHDR_END>
//*************************************************************************
//-------------------------------------------------------------------------
// Include Files
//-------------------------------------------------------------------------
#include <Token.h>
#include <PciBus.h>
#include <PciHostBridge.h>
#include <Setup.h>
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/AmiBoardInfo2.h>
#include <Protocol/AmiBoardInitPolicy.h>
#include <Library/SbPolicy.h>
//-------------------------------------------------------------------------
// Constants, Macros and Type Definitions
//-------------------------------------------------------------------------
EFI_RUNTIME_SERVICES  *mRT = NULL;
SB_SETUP_DATA         mPchPolicyData;

// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure: PcieRootPortInit
//
// Description:
//  This function provide the initial routine for Pcie Root Ports
//
// Input:
//	AMI_BOARD_INIT_PROTOCOL		*This - AMI BOARD INIT PROTOCOL
//	IN UINTN					*Function,
//	IN OUT VOID					*ParameterBlock
//
// Output:
//  EFI_SUCCESS - Initial step sucessfully
//  EFI_INVALID_PARAMETER - not find the initial step
//
// Modified:    Nothing
//
// Referrals:   None
//
// Notes:
//-------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS
PcieRootPortInit (
  IN      AMI_BOARD_INIT_PROTOCOL         *This,
  IN      UINTN                           *Function,
  IN OUT  AMI_BOARD_INIT_PARAMETER_BLOCK  *ParameterBlock
)
{
  AMI_BOARD_INIT_PARAMETER_BLOCK  *Args=(AMI_BOARD_INIT_PARAMETER_BLOCK*)ParameterBlock;
  PCI_INIT_STEP                   InitStep=(PCI_INIT_STEP)Args->InitStep;
  EFI_STATUS                      Status=EFI_UNSUPPORTED;

  if(Args->Signature != AMI_PCI_PARAM_SIG) {
    PCI_TRACE((TRACE_PCI,"Error: Args->Signature != AMI_PCI_PARAM_SIG \n" ));
    return EFI_INVALID_PARAMETER;
  }

  if(InitStep>=isPciMaxStep) {
    PCI_TRACE((TRACE_PCI,"Error: InitStep>=isPciMaxStep \n" ));
    return EFI_INVALID_PARAMETER;
  }

  switch (InitStep)
  {
    //-------------------------------------------------------------------------
    case isPciNone:
      PCI_TRACE((TRACE_PCI," (isPciNone); " ));
      break;
    //-------------------------------------------------------------------------
    case isPciGetSetupConfig:
      PCI_TRACE((TRACE_PCI," (isPciGetSetupConfig); " ));
      break;
    //-------------------------------------------------------------------------
    case isPciSkipDevice:
      PCI_TRACE((TRACE_PCI," (isPciSkipDevice); " ));
      break;
    //-------------------------------------------------------------------------
    case isPciSetAttributes:
      PCI_TRACE((TRACE_PCI," (isPciSetAttributes); " ));
      break;
    //-------------------------------------------------------------------------
    case isPciProgramDevice:
      PCI_TRACE((TRACE_PCI," (isPciProgramDevice); " ));
      break;
    //-------------------------------------------------------------------------
    case isPcieInitLink:
      PCI_TRACE((TRACE_PCI," (isPcieInitLink); " ));
      break;
    //-------------------------------------------------------------------------
    case isPcieSetAspm:
      PCI_TRACE((TRACE_PCI," (isPcieSetAspm); " ));
      break;
    //-------------------------------------------------------------------------
    case isPcieSetLinkSpeed:
      PCI_TRACE((TRACE_PCI," (isPcieSetLinkSpeed); " ));
      {
        PCI_DEV_INFO  *dn=(PCI_DEV_INFO*)Args->Param1;
        PCI_DEV_INFO  *up=(PCI_DEV_INFO*)Args->Param2;
        UINT16        *ls=(UINT16*)Args->Param3;
        UINT8         LinkSpeedSetup;

        //---------------------------------------
        // Override Link Speed Settings for the link.....
        //
        if ((dn->Address.Addr.Bus == 0) && (dn->Address.Addr.Device == 0x1C)) {
          LinkSpeedSetup = mPchPolicyData.PcieRootPortSpeed[dn->Address.Addr.Function];
          if (LinkSpeedSetup == 1) {
            *ls = 1;
          }
          if (LinkSpeedSetup == 2) {
            *ls = 2;
          }
        }
        Status=EFI_SUCCESS;
      }
      break;
    //-------------------------------------------------------------------------
    case isPciGetOptionRom:
      PCI_TRACE((TRACE_PCI," (isPciGetOptionRom); " ));
      break;
    //-------------------------------------------------------------------------
    case isPciOutOfResourcesCheck:
      PCI_TRACE((TRACE_PCI," (isPciOutOfResourcesCheck); " ));
      break;
    //-------------------------------------------------------------------------
    case isPciReadyToBoot:
      PCI_TRACE((TRACE_PCI," (isPciReadyToBoot); " ));
      break;
    //-------------------------------------------------------------------------
    case isPciQueryDevice:
      PCI_TRACE((TRACE_PCI," (isPciQueryDevice); " ));
      break;
    //-------------------------------------------------------------------------
    case isHbBasicInit:
      PCI_TRACE((TRACE_PCI," (isHbBasicInit); " ));
      break;
    //-------------------------------------------------------------------------
    case isRbCheckPresence:
      PCI_TRACE((TRACE_PCI," (isRbCheckPresence); " ));
      break;
    //-------------------------------------------------------------------------
    case isRbBusUpdate:
      PCI_TRACE((TRACE_PCI," (isRbBusUpdate); " ));
      break;
    //-------------------------------------------------------------------------
    default:
      Status=EFI_INVALID_PARAMETER;
      PCI_TRACE((TRACE_PCI," (!!!isPciMaxStep!!!); " ));
  }//switch

  return Status;
}

EFI_STATUS EFIAPI
SbPciDxeLibConstructor (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  mRT = SystemTable->RuntimeServices;
  GetSbSetupData ((VOID*)mRT, &mPchPolicyData, FALSE);
  return EFI_SUCCESS;
}

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

