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
// $Revision:  $
//
// $Date:  $
//*************************************************************************

//<AMI_FHDR_START>
//-----------------------------------------------------------------------
//
// Name:        NbSetup.c
//
// Description: This C file contains code related to NB setup routines
//
//-----------------------------------------------------------------------
//<AMI_FHDR_END>

#include <Setup.h>
#include <AmiDxeLib.h>
#include <AmiCspLib.h>
#include <NB.h>
#include <Protocol/MemInfo.h>
#include <Protocol/SeCRcInfo.h>
#include <Protocol/Heci.h>
#include <Protocol/ComponentName2.h>
#include <Library/HeciMsgLib.h>
#include <Protocol/SeCOperation.h> //P20130628

EFI_GUID gEfiComponentName2ProtocolGuid = EFI_COMPONENT_NAME2_PROTOCOL_GUID;
EFI_GUID gEfiSeCOperationProtocolGuid = {0x704ebea2, 0x5ee6, 0x4898,{ 0x96, 0x59, 0x1, 0x8b, 0x74, 0xb4, 0x47, 0x89}}; //P20130628

INTN
EfiStrnCmp (
  IN CHAR16   *String,
  IN CHAR16   *String2,
  IN UINTN    Length
  );

CHAR16 *
EFIAPI
EfiStrnCpy (
  OUT     CHAR16                    *Destination,
  IN      CONST CHAR16              *Source,
  IN      UINTN                     Length
  );

UINTN EfiStrLen(CHAR16 *string);
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   InitNbStrings
//
// Description: Initializes North Bridge Setup String
//
// Input:       HiiHandle - Handle to HII database
//              Class - Indicates the setup class
//
// Output:      None
//
// Notes:       PORTING REQUIRED
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
InitNbStrings (
    IN EFI_HII_HANDLE      HiiHandle, 
    IN UINT16              Class
)
{
  EFI_STATUS          			      Status;
  UINT32                          MemorySize;
  MEM_INFO_PROTOCOL       	      *MemInfoProtocol;
  EFI_SEC_RC_INFO_PROTOCOL        *SeCRcInfo;
  EFI_GUID                        gMemInfoProtocolGuid = MEM_INFO_PROTOCOL_GUID;
  EFI_GUID                        gEfiSeCRcInfoProtocolGuid = EFI_SEC_RC_INFO_PROTOCOL_GUID;
  EFI_GUID                        gEfiHeciProtocolGuid =EFI_HECI_PROTOCOL_GUID;
  UINT8                           SecVer1, SecVer2, SecVer3, SecVer4;
  UINT8                           Slot_Couter=0;
  CHAR16                          *DriverName = NULL;
  CHAR16                          *GopStr = L"Intel(R) GOP Driver ";
  CHAR16                          *GopVer = L"[N/A]            ";
  EFI_HANDLE                      *HandleBuffer = NULL;
  UINTN                           HandleCount, i;
  EFI_COMPONENT_NAME2_PROTOCOL    *ComponentName = NULL;
  UINT32                          MaxBufferSize = 0;
    SEC_OPERATION_PROTOCOL    *SeCOp; //P20130628
    SEC_INFOMATION            SeCInfo; //P20130628
  // Get the Memory Info HOB Protocol if it exists.
  Status = pBS->LocateProtocol (&gMemInfoProtocolGuid, NULL, &MemInfoProtocol);

  if ((Class == MAIN_FORM_SET_CLASS) || (Class == CHIPSET_FORM_SET_CLASS)) {
    MemorySize = MemInfoProtocol->MemInfoData.memSize;
    InitString(
              HiiHandle,
              STRING_TOKEN(STR_MEMORY_SIZE_VALUE),
              L"%4d MB (LPDDR3)",
              MemorySize
              );
  }
//P20130624 >>
  if (Class == CHIPSET_FORM_SET_CLASS) {
    MemorySize = MemInfoProtocol->MemInfoData.dimmSize[Slot_Couter];
    Slot_Couter += 1;
    if(MemorySize)
			InitString(
									HiiHandle,
									STRING_TOKEN(STR_MEMORY_SIZE_SLOT0_VALUE), 
									L"%4d MB (LPDDR3)",
									MemorySize
									);
      
#if(DIMM_SLOT_NUM>1)
    MemorySize = MemInfoProtocol->MemInfoData.dimmSize[Slot_Couter];
    Slot_Couter += 1;
    if(MemorySize)
			InitString(
									HiiHandle,
									STRING_TOKEN(STR_MEMORY_SIZE_SLOT1_VALUE),
									L"%4d MB (LPDDR3)",
									MemorySize
									);
#endif  //#if(MEM_RANK_NUM>1)
#if (MEM_CHANNEL_NUM == 2)
    MemorySize = MemInfoProtocol->MemInfoData.dimmSize[Slot_Couter];
    Slot_Couter += 1;
    if(MemorySize)
			InitString(
									HiiHandle,
									STRING_TOKEN(STR_MEMORY_SIZE_SLOT2_VALUE),
									L"%4d MB (LPDDR3)",
									MemorySize
									);

#if(DIMM_SLOT_NUM>1)
    MemorySize = MemInfoProtocol->MemInfoData.dimmSize[Slot_Couter];
    if(MemorySize)
			InitString(
									HiiHandle,
									STRING_TOKEN(STR_MEMORY_SIZE_SLOT3_VALUE),
									L"%4d MB (LPDDR3)",
									MemorySize
									);
//P20130624 << 
#endif  //#if(MEM_RANK_NUM>1)
#endif  //#if (MEM_RANK_NUM == 2)
  }

  // Locate ComponentName2 Protocol handles.
  Status = pBS->LocateHandleBuffer (
                ByProtocol,
                &gEfiComponentName2ProtocolGuid,
                NULL,
                &HandleCount,
                &HandleBuffer
                );
  if (EFI_ERROR(Status)){
      return;
  }
  for (i = 0; i < HandleCount; i++)
  {
      DriverName = NULL;
      Status = pBS->HandleProtocol (
                  HandleBuffer[i],
                  &gEfiComponentName2ProtocolGuid,
                  &ComponentName
                  );
      if (EFI_ERROR (Status))
          continue;
      
      // EFI_COMPONENT_NAME2_PROTOCOL.GetDriverName()
      Status = ComponentName->GetDriverName (
                   ComponentName,
                   ComponentName->SupportedLanguages,
                   &DriverName
                   );
      if (!EFI_ERROR(Status))
      {
          // Compare DriverName with specified string(GopStr)
          if (EfiStrnCmp(GopStr, DriverName, EfiStrLen(GopStr)) == 0)
              EfiStrnCpy(GopVer, DriverName+EfiStrLen(GopStr), EfiStrLen(DriverName)-EfiStrLen(GopStr));
      }
  }
  InitString(
          HiiHandle,
          STRING_TOKEN(STR_GOP_VALUE),
          L"%s", \
          GopVer
          );
  
  // Get the SeC Info Protocol if it exists.
  Status = pBS->LocateProtocol (&gEfiSeCRcInfoProtocolGuid, NULL, &SeCRcInfo);
  if (!EFI_ERROR (Status)) {
    SecVer4 = SeCRcInfo->RCVersion.Fields.RcBuildNo;
    SecVer3 = SeCRcInfo->RCVersion.Fields.RcRevision;
    SecVer2 = SeCRcInfo->RCVersion.Fields.RcMinorVersion;
    SecVer1 = SeCRcInfo->RCVersion.Fields.RcMajorVersion;
  
    InitString(
            HiiHandle,
            STRING_TOKEN(STR_TXE_VALUE),
            L"%02x.%02x.%02x.%02x", \
            SecVer1,SecVer2,SecVer3,SecVer4
            );
  }  

//P20130628 >>
    // Get TXE FW version.
    Status = pBS->LocateProtocol (&gEfiSeCOperationProtocolGuid, NULL, &SeCOp);
    if(!EFI_ERROR(Status))
    {
        Status = SeCOp->GetPlatformSeCInfo(&SeCInfo);
        if(!EFI_ERROR(Status))
        {
            if(SeCInfo.SeCVerValid == TRUE)
            {
                InitString(
                HiiHandle,
                STRING_TOKEN(STR_TXE_FW_VALUE),
                L"%02d.%02d.%02d.%04d", \
                SeCInfo.SeCVer.CodeMajor, SeCInfo.SeCVer.CodeMinor, SeCInfo.SeCVer.CodeHotFix, SeCInfo.SeCVer.CodeBuildNo
                );
            }
        }
    }
//P20130628 <<
}

INTN
EfiStrnCmp (
  IN CHAR16   *String,
  IN CHAR16   *String2,
  IN UINTN    Length
  )
{
  while (*String && Length != 0) {
    if (*String != *String2) {
      break;
    }
    String  += 1;
    String2 += 1;
    Length  -= 1;
  }
  return Length > 0 ? *String - *String2 : 0;
}

static
VOID ZeroMem(
    IN VOID  *Buffer,
    IN UINTN Size )
{
    UINT8 *ptr;

    ptr = Buffer;
    while ( Size-- )
    {
        *(ptr++) = 0;
    }
}

CHAR16 *
EFIAPI
EfiStrnCpy (
  OUT     CHAR16                    *Destination,
  IN      CONST CHAR16              *Source,
  IN      UINTN                     Length
  )
{
  CHAR16                            *ReturnValue;

  if (Length == 0) {
    return Destination;
  }

  //
  // Destination cannot be NULL if Length is not zero
  //
  ASSERT (Destination != NULL);
  ASSERT (((UINTN) Destination & BIT0) == 0);

  ReturnValue = Destination;

  while ((*Source != L'\0') && (Length > 0)) {
    *(Destination++) = *(Source++);
    Length--;
  }

  ZeroMem (Destination, Length * sizeof (*Destination));
  return ReturnValue;
}

UINTN EfiStrLen(CHAR16 *string) {
  UINTN length=0;
  while(*string++) length++;
  return length;
}

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
