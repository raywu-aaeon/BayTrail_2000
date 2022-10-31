#include <PchAccess.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library\AmiChipsetIoLib.h>
#include <Setup.h>
#include <Token.h>
#include <Protocol\PchExtendedReset.h>
#include <PchRegs\PchRegsUsb.h>
#include <Library/SbPolicy.h>
#include <Guid\MemoryTypeInformation.h> //EIP139595

static EFI_GUID gEfiMemoryTypeInformationGuid = EFI_MEMORY_TYPE_INFORMATION_GUID; //EIP139595

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: ChangeUsbPortWorkaround
//
// Description: This function is a hook called after some control 
//              modified in the setup utility by user. This
//              function is available as ELINK. 
//              Xhci workaround for disabling/enabling USB ports. (EIP135854+)
//
// Input:   VOID
//
// Output:    VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ChangeUsbPortWorkaround(VOID)
{
  SB_SETUP_DATA                   PchPolicyData;
  EFI_PCH_EXTENDED_RESET_PROTOCOL *gExtendedReset;
  PCH_EXTENDED_RESET_TYPES        ResetType;
  UINT32                          Index;
  UINT32                          XhciUsb2Pdo;
  UINTN                           XhciPciMmBase;
  UINTN                           VariableSize; //EIP139595
  EFI_STATUS                      Status; //EIP139595

  // Get the value of the SB Setup data.
  GetSbSetupData ((VOID*)gRT, &PchPolicyData, FALSE);

  //EIP139595 >>
  Status = gRT->GetVariable( L"MemoryTypeInformation", &gEfiMemoryTypeInformationGuid, NULL, &VariableSize, NULL); 
  if(!EFI_ERROR(Status))
  {
  //EIP139595 <<
	if(PchPolicyData.PchUsb30Mode == 1) {
    // XHCI PDO for HS
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
  } //EIP139595
}
