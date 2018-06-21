/** @file
  Clovertrail I2C test application

  This application tests "UEFI Entropy-Gathering Protocol"

Copyright (c) 2012, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/

#include "PssKey.h"
#include "Key.h"
#include <Setup.h>     // AMI_OVERRIDE - EIP140009 Support Pss


PSS_DRIVER_CONTEXT         *DriverContext;



EFI_STATUS
EFIAPI
PssDeviceInit()
{
  EFI_STATUS                Status;
  EFI_I2C_BUS_PROTOCOL      *I2cBusProtocol = NULL;
  SETUP_DATA                mSystemConfiguration;     // AMI_OVERRIDE - EIP140009 Support Pss
  UINTN                     VarSize = 0;

  EFI_HANDLE                *HandleArray = NULL;
  UINTN                     HandleArrayCount = 0;
  UINTN                     Index = 0;
  CHAR8                     AcpiID[I2C_ACPI_ID_LEN + 1];

  DriverContext = AllocatePool(sizeof(PSS_DRIVER_CONTEXT));
  if(DriverContext == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    return Status;
  }

  VarSize = sizeof(SETUP_DATA);     // AMI_OVERRIDE - EIP140009 Support Pss

  Status = gRT->GetVariable(L"Setup",
                            &gEfiNormalSetupGuid,
                            NULL,
                            &VarSize,
                            &mSystemConfiguration);

  if(mSystemConfiguration.PssEnabled == 0) {
    DriverContext->PssEnabled = FALSE;
  } else {
    DriverContext->PssEnabled = TRUE;
  }

  DriverContext->I2cBusProtocol = NULL;
  DriverContext->PssVerified = FALSE;

  //
  // Compose the device path to be check by DevicePath lib.
  //
  AsciiStrCpy(AcpiID, DID_ACPI_ID_PREFIX);
  AcpiID[4] = '0'+ PSS_I2C_CONTROLLER_ID;
  AsciiStrCpy(AcpiID+5, DID_ACPI_ID_SUFFIX);
  AsciiStrCpy(AcpiID+11, DID_ACPI_ID_SUFFIX_400K);

  Status = gBS->LocateHandleBuffer(
                  ByProtocol,
                  &gEfiI2cBusProtocolGuid,
                  NULL,
                  &HandleArrayCount,
                  &HandleArray
                  );
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to locate I2C bus protocol.\n"));
    return Status;
  }

  for ( Index = 0; HandleArrayCount > Index; Index ++ ) {
    //
    // Determine if the device is available
    //
    if ( NULL != DlAcpiFindDeviceWithMatchingCid ( HandleArray [ Index ],
         0,
         (CONST CHAR8 *)AcpiID
         )) {
      //
      // The device was found
      //
      Status = gBS->OpenProtocol (
                      HandleArray [Index],
                      &gEfiI2cBusProtocolGuid,
                      (VOID **)&I2cBusProtocol,
                      NULL,
                      NULL,
                      EFI_OPEN_PROTOCOL_GET_PROTOCOL
                      );
      if(!EFI_ERROR(Status)) {
        break;
      }
    }
  }

  //
  // Done with the handle array
  //
  gBS->FreePool (HandleArray);

  if (I2cBusProtocol == NULL) {
    DEBUG((EFI_D_ERROR, "Failed to locate i2c device.\n"));
    return Status;
  }

  DriverContext->I2cBusProtocol = I2cBusProtocol;
  DriverContext->PssVerified = FALSE;

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
ReadPssData (
  UINT8     *Buffer,
  UINT32    Address,
  UINT32    Size
  )
{
  EFI_STATUS                Status;
  EFI_I2C_REQUEST_PACKET    Request;
  UINT8                     WriteBuffer[1];  //Buffer to send cmd to PSS.

  if (DriverContext == NULL || (DriverContext->I2cBusProtocol==NULL)) {
    return EFI_NOT_READY;
  }

  if(Size == 0) {
    return EFI_SUCCESS;
  }

  //
  // Prepare the request parameter
  //
  WriteBuffer[0] = (UINT8)Address;

  Request.ReadBytes = Size;
  Request.ReadBuffer = Buffer;
  Request.WriteBytes = 1;
  Request.WriteBuffer = &WriteBuffer[0];
  Request.Timeout = I2C_TIMEOUT_DEFAULT;


  Status = DriverContext->I2cBusProtocol->StartRequest (
                                            DriverContext->I2cBusProtocol,
                                            (PSS_I2C_SLAVE_ADDR + Address/0x100) | 0x400,
                                            NULL,
                                            &Request,
                                            NULL
                                            );

  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to Read Data PSS.\n"));
    return EFI_ABORTED;
  }

  return EFI_SUCCESS;
}

UINT16
Monzax_Detect_Chip (
  VOID
  )
{
  EFI_STATUS    Status;
  UINT8         PssData[4];

  //
  // Read the chip's Class ID from the TID bank, it should be 0xE2 (Gen2)
  // And the TID Module should be 0x140
  // Check whether the PSS IC is Monza X-2K
  //
  Status = ReadPssData(&PssData[0], BASE_ADDRESS_CLASSID_2K, 4);
  if (!(EFI_ERROR(Status)) &&
      (PssData[0] == 0xE2) &&
      ((PssData[2] & 0x0F) == 0x01) &&
      (PssData[3] == 0x40)
     ) {
    //
    // Monza X-2K
    //
    return MonzaX_2K_Dura;
  }

  //
  // Read the chip's Class ID from the TID bank, it should be 0xE2 (Gen2)
  // And the TID Module should be 0x150
  // Check whether the PSS IC is Monza X-8K
  //
  Status = ReadPssData(&PssData[0], BASE_ADDRESS_CLASSID_8K, 4);
  if (!(EFI_ERROR(Status)) &&
      (PssData[0] == 0xE2) &&
      ((PssData[2] & 0x0F) == 0x01) &&
      (PssData[3] == 0x50)
     ) {
    //
    // Monza X-8K
    //
    return MonzaX_8K_Dura;
  }

  return 0;
}

EFI_STATUS
EFIAPI
GetTID(
  UINT8                 *Buffer
  )
{
  EFI_STATUS                Status;
  EFI_I2C_REQUEST_PACKET    Request;
  UINT8                     WriteBuffer[1];  //Buffer to send cmd to PSS.
  UINT8                     ReadBuffer[24];  //24 byte read for 2K; 12 byte for 8K part.
  UINT32                     BaseAddress;


  if ((DriverContext == NULL) || (DriverContext->I2cBusProtocol==NULL) || (Buffer == NULL)) {
    return EFI_NOT_READY;
  }

  if(DriverContext->Is2KPart) {

    //
    // Prepare the request parameter
    //
    BaseAddress = BASE_ADDRESS_TID_2K;
    WriteBuffer[0] = (UINT8)BaseAddress;

    Request.ReadBytes = 24;
    Request.ReadBuffer = &ReadBuffer[0];
    Request.WriteBytes = 1;
    Request.WriteBuffer = &WriteBuffer[0];
    Request.Timeout = I2C_TIMEOUT_DEFAULT;
  } else {
    BaseAddress = BASE_ADDRESS_TID_8K;
    WriteBuffer[0] = (UINT8)BaseAddress;

    Request.ReadBytes = 12;
    Request.ReadBuffer = &ReadBuffer[0];
    Request.WriteBytes = 1;
    Request.WriteBuffer = &WriteBuffer[0];
    Request.Timeout = I2C_TIMEOUT_DEFAULT;


  }



  Status = DriverContext->I2cBusProtocol->StartRequest (
                                            DriverContext->I2cBusProtocol,
                                            (PSS_I2C_SLAVE_ADDR + BaseAddress/0x100) | 0x400,
                                            NULL,
                                            &Request,
                                            NULL
                                            );

  if (EFI_ERROR(Status)) {
    Print(L"Failed to Read TID Bank.\n");
    return EFI_ABORTED;

  }


  if(DriverContext->Is2KPart) {
    CopyMem(Buffer+4, &ReadBuffer[22], 2);
    CopyMem(Buffer, &ReadBuffer[0],4);

  } else {
    CopyMem(Buffer, &ReadBuffer[6], 6);
  }



  return EFI_SUCCESS;

}



EFI_STATUS
EFIAPI
WritePssData (
  UINT8     *Buffer,
  UINT32    Address,
  UINTN    Size
  )
{
  EFI_STATUS                Status;
  EFI_I2C_REQUEST_PACKET    Request;
  UINTN                     Index = 0;


  UINT8                     WriteBuffer[5];  //PSS device allows only most DWORD I2C write.


  if (DriverContext == NULL || (DriverContext->I2cBusProtocol==NULL)) {
    return EFI_NOT_READY;
  }

  if(Size != 128) {
    return EFI_INVALID_PARAMETER;
  }

  Print(L"\r\nStoring encrypted unlocker key into PSS.\r\n");

  for(Index=0; Index<=128;) {
    //
    // Prepare the request parameter
    //
    gBS->Stall(400*1000);
    WriteBuffer[0] = (UINT8)Address;
    CopyMem(&WriteBuffer[1], &Buffer[Index],4);

    Request.ReadBytes = 0;
    Request.ReadBuffer = 0;
    Request.WriteBytes = 5;
    Request.WriteBuffer = &WriteBuffer[0];
    Request.Timeout = I2C_TIMEOUT_DEFAULT;

    //Send request to FG for remaining capacity.
    Status = DriverContext->I2cBusProtocol->StartRequest (
                                              DriverContext->I2cBusProtocol,
                                              (PSS_I2C_SLAVE_ADDR + Address/0x100) | 0x400,
                                              NULL,
                                              &Request,
                                              NULL
                                              );

    if (EFI_ERROR(Status)) {
      Print(L"\r\nFailed to write encrypted TID to PSS.\r\n");
      return EFI_INVALID_PARAMETER;
    }

    Print(L"*");

    Index += 4;
    Address += 4;

  }

  return EFI_SUCCESS;
}

/**
  UEFI application entry point which has an interface similar to a
  standard C main function.

  The ShellCEntryLib library instance wrappers the actual UEFI application
  entry point and calls this ShellAppMain function.

  @param  ImageHandle  The image handle of the UEFI Application.
  @param  SystemTable  A pointer to the EFI System Table.

  @retval  0               The application exited normally.
  @retval  Other           An error occurred.

**/

INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  EFI_STATUS        Status;
  VOID              *Rsa;
  UINT8             PssTID[6]= {0,0,0,0,0,0};
  UINTN             SigSize = 0;
  UINT8             *TidEncrypted;
  UINT32            Index = 0;
  UINT16            PssModule = 0;
  UINT8             PssKeyOffset = 0;
  UINT8             HashValue[SHA256_DIGEST_SIZE];
  UINTN             HashSize;
  UINTN             CtxSize;
  VOID              *Sha256Ctx;
  BOOLEAN           Verified = FALSE;
  BOOLEAN           Set = FALSE;

  UINT8             UnlockPublicKey[PUBLIC_KEY_SETUP_SIZE]= {
    0xBB, 0xF8, 0x2F, 0x09, 0x06, 0x82, 0xCE, 0x9C, 0x23, 0x38, 0xAC, 0x2B, 0x9D, 0xA8, 0x71, 0xF7,
    0x36, 0x8D, 0x07, 0xEE, 0xD4, 0x10, 0x43, 0xA4, 0x40, 0xD6, 0xB6, 0xF0, 0x74, 0x54, 0xF5, 0x1F,
    0xB8, 0xDF, 0xBA, 0xAF, 0x03, 0x5C, 0x02, 0xAB, 0x61, 0xEA, 0x48, 0xCE, 0xEB, 0x6F, 0xCD, 0x48,
    0x76, 0xED, 0x52, 0x0D, 0x60, 0xE1, 0xEC, 0x46, 0x19, 0x71, 0x9D, 0x8A, 0x5B, 0x8B, 0x80, 0x7F,
    0xAF, 0xB8, 0xE0, 0xA3, 0xDF, 0xC7, 0x37, 0x72, 0x3E, 0xE6, 0xB4, 0xB7, 0xD9, 0x3A, 0x25, 0x84,
    0xEE, 0x6A, 0x64, 0x9D, 0x06, 0x09, 0x53, 0x74, 0x88, 0x34, 0xB2, 0x45, 0x45, 0x98, 0x39, 0x4E,
    0xE0, 0xAA, 0xB1, 0x2D, 0x7B, 0x61, 0xA5, 0x1F, 0x52, 0x7A, 0x9A, 0x41, 0xF6, 0xC1, 0x68, 0x7F,
    0xE2, 0x53, 0x72, 0x98, 0xCA, 0x2A, 0x8F, 0x59, 0x46, 0xF8, 0xE5, 0xFD, 0x09, 0x1D, 0xBD, 0xCB
  };

  UINT8 RsaE[] = { 0x11 };

  UINT8 RsaD[] = {
    0xA5, 0xDA, 0xFC, 0x53, 0x41, 0xFA, 0xF2, 0x89, 0xC4, 0xB9, 0x88, 0xDB, 0x30, 0xC1, 0xCD, 0xF8,
    0x3F, 0x31, 0x25, 0x1E, 0x06, 0x68, 0xB4, 0x27, 0x84, 0x81, 0x38, 0x01, 0x57, 0x96, 0x41, 0xB2,
    0x94, 0x10, 0xB3, 0xC7, 0x99, 0x8D, 0x6B, 0xC4, 0x65, 0x74, 0x5E, 0x5C, 0x39, 0x26, 0x69, 0xD6,
    0x87, 0x0D, 0xA2, 0xC0, 0x82, 0xA9, 0x39, 0xE3, 0x7F, 0xDC, 0xB8, 0x2E, 0xC9, 0x3E, 0xDA, 0xC9,
    0x7F, 0xF3, 0xAD, 0x59, 0x50, 0xAC, 0xCF, 0xBC, 0x11, 0x1C, 0x76, 0xF1, 0xA9, 0x52, 0x94, 0x44,
    0xE5, 0x6A, 0xAF, 0x68, 0xC5, 0x6C, 0x09, 0x2C, 0xD3, 0x8D, 0xC3, 0xBE, 0xF5, 0xD2, 0x0A, 0x93,
    0x99, 0x26, 0xED, 0x4F, 0x74, 0xA1, 0x3E, 0xDD, 0xFB, 0xE1, 0xA1, 0xCE, 0xCC, 0x48, 0x94, 0xAF,
    0x94, 0x28, 0xC2, 0xB7, 0xB8, 0x88, 0x3F, 0xE4, 0x46, 0x3A, 0x4B, 0xC8, 0x5B, 0x1C, 0xB3, 0xC1
  };

  RandomSeed(NULL, 0);

  Status = gRT->SetVariable(
                  PUBLIC_KEY_SETUP_NAME,
                  &gEfiNormalSetupGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  PUBLIC_KEY_SETUP_SIZE,
                  &UnlockPublicKey[0]
                  );
  if (EFI_ERROR(Status)) {
    Print(L"Fail to set the Unlock Public Key.\n");
    Print(L"Please try again\n");
  } else {
    Print(L"Successfully set Unlock Public Key.\n");
  }

  Print(L"Initializing Pss Device.\r\n");
  Status = PssDeviceInit();
  if(EFI_ERROR(Status)) {
    Print(L"Failed to initialize PSS device.\r\n");
    return Status;
  }

  Print(L"Pss Device Initailized.\r\n");
  //
  // Detect PSS Chip
  //
  PssModule = Monzax_Detect_Chip();
  if (PssModule == MonzaX_2K_Dura) {
    Print(L"PSS is a MonzaX_2K_Dura\n");
    DriverContext->Is2KPart = TRUE;
    PssKeyOffset = BASE_ADDRESS_USER_2K;
  } else if (PssModule == MonzaX_8K_Dura) {
    Print(L"PSS is a MonzaX_8K_Dura\n");
    DriverContext->Is2KPart = FALSE;
    PssKeyOffset = BASE_ADDRESS_USER_8K;
  } else {
    //
    // The PSS doesn't exist
    //
    return EFI_UNSUPPORTED;
  }

  //
  //Get TID
  //
  ZeroMem(PssTID, sizeof(PssTID));

  Status = GetTID(&PssTID[0]);
  if(EFI_ERROR(Status)) {
    Print(L"Failed to get TID.\n");
    return Status;
  } else {
    Print(L"TID is:0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x.\r\n",
          PssTID[0], PssTID[1], PssTID[2], PssTID[3], PssTID[4], PssTID[5]);
  }

  Print(L"Generating hash of TID for RSA-PKCS1_V1.5.\r\n");

  HashSize = SHA256_DIGEST_SIZE;
  ZeroMem (HashValue, HashSize);
  CtxSize = Sha256GetContextSize ();
  Sha256Ctx = AllocatePool (CtxSize);

  Status  = Sha256Init (Sha256Ctx);
  if (!Status) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  }

  Status  = Sha256Update (Sha256Ctx, PssTID, sizeof(PssTID));
  if (!Status) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  }

  Status  = Sha256Final (Sha256Ctx, HashValue);
  if (!Status) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  }

  FreePool (Sha256Ctx);

  //
  //Encrypt TID
  //
  Rsa = RsaNew();
  if(Rsa == NULL) {
    Print(L"Failed to generate Rsa context.\r\n");
  }

  Set = RsaSetKey(Rsa, RsaKeyN, UnlockPublicKey, sizeof(UnlockPublicKey));
  if(Set == FALSE) {
    Print(L"Failed to set N.\r\n");
    return EFI_ABORTED;
  }

  Set = RsaSetKey(Rsa, RsaKeyE, RsaE, sizeof(RsaE));
  if(Set == FALSE) {
    Print(L"Failed to set E.\r\n");
    return EFI_ABORTED;
  }

  Set = RsaSetKey(Rsa, RsaKeyD, RsaD, sizeof(RsaD));
  if(Set == FALSE) {
    Print(L"Failed to set D.\r\n");
    return EFI_ABORTED;
  }

  Status = RsaPkcs1Sign(Rsa, HashValue, HashSize, NULL, &SigSize);

  Print(L"Signature size is:%d.\r\n", SigSize);

  TidEncrypted = AllocatePool(SigSize);
  if (TidEncrypted == NULL) {
  	return EFI_OUT_OF_RESOURCES;
  }
  Status = RsaPkcs1Sign(Rsa, HashValue, HashSize, TidEncrypted, &SigSize);

  if(EFI_ERROR(Status)) {
    Print(L"Failed to sign it.\r\n");
    return Status;
  }

  //
  //Write this to PSS
  //
  for(Index = 0; Index < 128; Index++) {
    Print(L"0x%02x ", TidEncrypted[Index]);
  }

  WritePssData(TidEncrypted, PssKeyOffset, SigSize);

  //
  //Decrypt it to check the signing result
  //
  Verified = RsaPkcs1Verify(Rsa, HashValue, HashSize, TidEncrypted, SigSize);

  if(!Verified) {
    Print(L"Verification failure.\r\n");
  } else {
    Print(L"\r\nVerified.\r\n");
  }
  RsaFree(Rsa);

  FreePool(TidEncrypted);

  Print(L"Done\r\n");

  if(DriverContext != NULL) {
    FreePool(DriverContext);
  }

  return EFI_SUCCESS;
}
