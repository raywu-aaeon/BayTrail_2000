#include "Uefi.h"
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>

extern EFI_GUID gEfiSetupVariableGuid;

extern
EFI_STATUS
HeciHmrfpoEnable (
  IN  UINT64        Nonce,
  OUT UINT8         *Result
  );

extern
EFI_STATUS
HeciSendCbmResetRequest (
  IN  UINT8         ResetOrigin,
  IN  UINT8         ResetType
  );