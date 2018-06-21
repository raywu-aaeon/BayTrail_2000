//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**     5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093            **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//*************************************************************************
// $Header:  $
//
// $Revision:  $
//
// $Date: $
//*************************************************************************
// Revision History
// ----------------
// $Log:  $
// 
//*************************************************************************
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  
//
// Description: 
//  
//
//<AMI_FHDR_END>
//*************************************************************************
#include <Efi.h>
#include <AmiTcg\TcgCommon.h>
#include <AmiTcg\Sha.h>
#include <AmiTcg\TcgMisc.h>
#include <token.h>
#include <AmiTcg\TpmLib.h>
#include <AmiTcg\TcgPc.h>
#include <AmiTcg\Tpm20Pei.h>
#include "PPI\TcgService.h"
#include "PPI\TpmDevice.h"
#include "PPI\CpuIo.h"
#include "PPI\LoadFile.h"
#include <FFS.h>
#include "PPI\TcgPlatformSetupPeiPolicy.h"
#include <Library\DebugLib.h>
#include <Library/BaseMemoryLib.h>


UINT8 GetCurrentInterfaceType();
BOOLEAN IsPTP();
BOOLEAN CrbSupported();
BOOLEAN FIFOSupported();

#pragma pack(push,1)

typedef struct {
  TPM2_COMMAND_HEADER       Header;
  TPM_CAP                   Capability;
  UINT32                    Property;
  UINT32                    PropertyCount;
} TPM2_GET_CAPABILITY_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER      Header;
  TPMI_YES_NO               MoreData;
  TPMS_CAPABILITY_DATA      CapabilityData;
} TPM2_GET_CAPABILITY_RESPONSE;

EFI_GUID gTrEEPeiGuid = AMI_PEI_TREE_SERVICE_PPI_GUID;
EFI_GUID gEfiCrbPeiAmiTcgLogHobGuid        = EFI_TCG_TREE_LOG_HOB_GUID;

#define     PEI_MAX_LOG_AREA_SIZE (400) // 1KB

#pragma pack(pop,1)

TCG_LOG_HOB                 *TcgLog = NULL;
static EFI_PHYSICAL_ADDRESS TreeEventLogLocation;
static EFI_PHYSICAL_ADDRESS LastEntry = 0;
static EFI_PHYSICAL_ADDRESS LastEventPtr = 0;
static BOOLEAN  IEventLogTruncated = FALSE;

EFI_STATUS
EFIAPI TrEETisPeiInit()
{
    return TisRequestLocality(
               (TPM_1_2_REGISTERS_PTR)( UINTN ) PORT_TPM_IOMEMBASE);
}

EFI_STATUS
EFIAPI TrEETisPeiClose()
{
    return TisReleaseLocality(
               (TPM_1_2_REGISTERS_PTR)( UINTN ) PORT_TPM_IOMEMBASE);
}



EFI_STATUS
EFIAPI
Tpm2GetCapability (
  IN EFI_PEI_SERVICES               **PeiServices,
  IN      TPM_CAP                   Capability,
  IN      UINT32                    Property,
  IN      UINT32                    PropertyCount,
  OUT     TPMI_YES_NO               *MoreData,
  OUT     TPMS_CAPABILITY_DATA      *CapabilityData
  )
{
  EFI_STATUS                        Status;
  TPM2_GET_CAPABILITY_COMMAND       SendBuffer;
  TPM2_GET_CAPABILITY_RESPONSE      RecvBuffer;
  UINT32                            SendBufferSize;
  UINT32                            RecvBufferSize;
  EFI_TREE_PPI                      *pTreePpi  =  NULL;
  EFI_GUID               gEfiTrEEPpiGuid =  AMI_PEI_TREE_SERVICE_PPI_GUID;

  Status = (*PeiServices)->LocatePpi(PeiServices,
                  &gEfiTrEEPpiGuid,
                  0,
                  NULL,
                  &pTreePpi);

  if(EFI_ERROR(Status))return Status;

  //
  // Construct command
  //
  SendBuffer.Header.tag = SwapBytes16(TPM_ST_NO_SESSIONS);
  SendBuffer.Header.commandCode = SwapBytes32(TPM_CC_GetCapability);

  SendBuffer.Capability = SwapBytes32 (Capability);
  SendBuffer.Property = SwapBytes32 (Property);
  SendBuffer.PropertyCount = SwapBytes32 (PropertyCount);
 
  SendBufferSize = (UINT32) sizeof (SendBuffer);
  SendBuffer.Header.paramSize = SwapBytes32 (SendBufferSize);
    
  //
  // send Tpm command
  //
  RecvBufferSize = sizeof (RecvBuffer);
  Status = pTreePpi->SubmitCommand (pTreePpi, SendBufferSize, (UINT8 *)&SendBuffer, RecvBufferSize, (UINT8 *)&RecvBuffer );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (RecvBufferSize <= sizeof (TPM2_RESPONSE_HEADER) + sizeof (UINT8)) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Return the response
  //
  *MoreData = RecvBuffer.MoreData;
  //
  // Does not unpack all possiable property here, the caller should unpack it and note the byte order.
  //
  CopyMem (CapabilityData, &RecvBuffer.CapabilityData, RecvBufferSize - sizeof (TPM2_RESPONSE_HEADER) - sizeof (UINT8));
  
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
Tpm2GetCapabilityManufactureID (
  IN      EFI_PEI_SERVICES        **PeiServices,
  OUT     UINT32                  *ManufactureId
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  EFI_STATUS              Status; 

  Status = Tpm2GetCapability ( PeiServices,
             TPM_CAP_TPM_PROPERTIES, 
             TPM_PT_MANUFACTURER, 
             1, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *ManufactureId = SwapBytes32 (TpmCap.data.tpmProperties.tpmProperty->value);

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
Tpm2GetCapabilityMaxCommandResponseSize (
  IN  EFI_PEI_SERVICES        **PeiServices,     
  OUT UINT32                    *MaxCommandSize,
  OUT UINT32                    *MaxResponseSize
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  EFI_STATUS              Status;

  Status = Tpm2GetCapability (PeiServices,
             TPM_CAP_TPM_PROPERTIES, 
             TPM_PT_MAX_COMMAND_SIZE, 
             1, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *MaxCommandSize = SwapBytes32 (TpmCap.data.tpmProperties.tpmProperty->value);

  Status = Tpm2GetCapability (PeiServices,
             TPM_CAP_TPM_PROPERTIES, 
             TPM_PT_MAX_RESPONSE_SIZE, 
             1, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *MaxResponseSize = SwapBytes32(TpmCap.data.tpmProperties.tpmProperty->value);
  return EFI_SUCCESS; 
}


EFI_STATUS
EFIAPI
Tpm2GetCapabilitySupportedAlg (
  IN  EFI_PEI_SERVICES        **PeiServices,
  OUT TPML_ALG_PROPERTY      *AlgList
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  UINTN                   Index;
  EFI_STATUS              Status;
 
  Status = Tpm2GetCapability (PeiServices,
             TPM_CAP_ALGS, 
             1, 
             1, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  CopyMem (AlgList, &TpmCap.data.algorithms, sizeof (TPML_ALG_PROPERTY));

  AlgList->count = SwapBytes32 (AlgList->count);
  for (Index = 0; Index < AlgList->count; Index++) {
    AlgList->algProperties[Index].alg = SwapBytes16 (AlgList->algProperties[Index].alg);
    *(UINT32 *)&AlgList->algProperties[Index].algProperties = SwapBytes32 (*(UINT32 *)&AlgList->algProperties[Index].algProperties);
  }
  return EFI_SUCCESS;
}



EFI_STATUS
TreeGetCapability (
  IN EFI_PEI_SERVICES        **PeiServices,        
  IN EFI_TREE_PPI                *This,
  IN OUT TREE_BOOT_SERVICE_CAPABILITY *ProtocolCapability
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;
  UINT8       HashAlg = 0;   //TODO::FIX
  static UINT32 ManufactureID = 0xFFFFFFFF;
  static UINT32 MaxResponseSize = 0xFFFFFFFF;
  static UINT32 MaxCommandSize  = 0xFFFFFFFF;
 
  if((ProtocolCapability == NULL) || (This == NULL)){
    Status = EFI_INVALID_PARAMETER;
  }
  else {

    if(ManufactureID == 0xFFFFFFFF && MaxResponseSize == 0xFFFFFFFF && MaxCommandSize ==0xFFFFFFFF)
    {
        Status = Tpm2GetCapabilityManufactureID (PeiServices, &ManufactureID);
        if(EFI_ERROR(Status)){
            return Status;
        }

        Status = Tpm2GetCapabilityMaxCommandResponseSize (PeiServices, &MaxCommandSize, &MaxResponseSize);
        if(EFI_ERROR(Status)){
            return Status;
        }

    }

    if(ProtocolCapability->Size < (sizeof(UINT8) + sizeof(TREE_VERSION) + sizeof(TREE_VERSION))){
      Status = EFI_BUFFER_TOO_SMALL;
        }
    else {
      ProtocolCapability->StructureVersion.Major = 1;
      ProtocolCapability->StructureVersion.Minor = 0;
      ProtocolCapability->ProtocolVersion.Major  = 1;
      ProtocolCapability->ProtocolVersion.Minor  = 0;

      if (ProtocolCapability->Size < sizeof(TREE_BOOT_SERVICE_CAPABILITY)){
        ProtocolCapability->Size = sizeof(TREE_BOOT_SERVICE_CAPABILITY);
        Status = EFI_BUFFER_TOO_SMALL;
      }
      else {

        if( HashAlg == 0){
            ProtocolCapability->HashAlgorithmBitmap = TREE_BOOT_HASH_ALG_SHA1;
        }else if(HashAlg == 1){
            ProtocolCapability->HashAlgorithmBitmap = TREE_BOOT_HASH_ALG_SHA256;
        }

        ProtocolCapability->SupportedEventLogs  = TREE_EVENT_LOG_FORMAT_TCG_1_2;
        ProtocolCapability->TrEEPresentFlag     = TRUE;
        ProtocolCapability->MaxCommandSize      = MaxCommandSize;   
        ProtocolCapability->MaxResponseSize     = MaxResponseSize;   
        ProtocolCapability->ManufacturerID      = ManufactureID;
      }
    }
  }

  return Status;
}




EFI_STATUS
EFIAPI TrEECRBPeiTransmit(
    IN  EFI_TREE_PPI             *This,
    IN  UINT32                  InputParameterBlockSize,
    IN  UINT8                   *InputParameterBlock,
    IN  UINT32                  OutputParameterBlockSize,
    IN  UINT8                   *OutputParameterBlock )
{
    EFI_STATUS            Status     = EFI_SUCCESS;
    UINT32                ReturnSize = 0;
    UINT32                Size = 0;

    if (This == NULL || InputParameterBlock == NULL || OutputParameterBlock == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    Size = OutputParameterBlockSize;
    Status = CrbSubmitCmd(InputParameterBlock,
                       InputParameterBlockSize,
                       OutputParameterBlock,
                       &Size);

    return Status;
}



EFI_STATUS
EFIAPI TrEETisPeiTransmit(
    IN  EFI_TREE_PPI             *This,
    IN  UINT32                  InputParameterBlockSize,
    IN  UINT8                   *InputParameterBlock,
    IN  UINT32                  OutputParameterBlockSize,
    IN  UINT8                   *OutputParameterBlock )
{
    EFI_STATUS             Status     = EFI_SUCCESS;
    TPM_1_2_REGISTERS_PTR  TpmReg;
    EFI_PHYSICAL_ADDRESS   TPM_Base = (EFI_PHYSICAL_ADDRESS)PORT_TPM_IOMEMBASE;
    UINT32                 ReturnSize = 0;
    UINT32                 Size = 0;
    TPM_TRANSMIT_BUFFER      InBuffer[1], OutBuffer[1];

    if (This == NULL || InputParameterBlock == NULL || OutputParameterBlock == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    
    InBuffer[0].Buffer  = InputParameterBlock;
    InBuffer[0].Size    = InputParameterBlockSize;
    OutBuffer[0].Buffer = OutputParameterBlock;
    OutBuffer[0].Size   = OutputParameterBlockSize;
    
    TpmReg  = (TPM_1_2_REGISTERS_PTR)( UINTN ) TPM_Base;
    
    TrEETisPeiInit();
    
    Status =  TpmLibPassThrough(
               TpmReg,
               sizeof (InBuffer) / sizeof (*InBuffer),
               InBuffer,
               sizeof (OutBuffer) / sizeof (*OutBuffer),
               OutBuffer
               );
    
    TrEETisPeiClose();
    return Status;
}




EFI_STATUS SelectAndLockInterface(CONST EFI_PEI_SERVICES  **PeiServices, UINT8 Interface)
{
    volatile UINT8 *IntefSel = (volatile UINT8 *)(PORT_TPM_IOMEMBASE + 0x32);

    DEBUG((-1, "GetCurrentInterfaceType results = %x \n", GetCurrentInterfaceType())); 
    DEBUG((-1, "Input InterfaceType = %x \n", Interface)); 
    
    if(GetCurrentInterfaceType()!=Interface)
    {
        if(Interface == 1){
            //CRB interface, check if supported
            if(CrbSupported() == 0x02){
                *IntefSel |= 0x02;  //set CRB
                (*(PeiServices))->ResetSystem(PeiServices);
                return EFI_SUCCESS;
            }
        }
        if(Interface == 0){
            //CRB interface, check if supported
            if(FIFOSupported()){
                *IntefSel &= 0xFC;  //set CRB
                (*(PeiServices))->ResetSystem(PeiServices);
                return EFI_SUCCESS;
            }
        }
    }
    
    DEBUG((-1, "IntefSel = %x \n", *IntefSel)); 
    if((*IntefSel & 0x08)==0){
        *IntefSel |= 0x08;  //lock CRB
    }
    DEBUG((-1, "IntefSel = %x \n", *IntefSel)); 
    return EFI_SUCCESS;
}




EFI_STATUS
TrEEPeiCrbExtend(
    IN VOID           *CallbackContext,
    IN TPM_PCRINDEX   PcrIndex,
    IN TCG_DIGEST     *Digest,
    OUT TCG_DIGEST    *NewPCRValue )
{
    TPM2_PCRExtend_cmd_t  Cmd;
    TPM2_PCRExtend_res_t  Res;
    TPM2_PCRExtend_res_t  Tmpres;
    UINT32                CmdSize;
    UINT8                 *Buffer;
    UINT8                 *AuthSizeOffset;
    UINT8                 *ResultBuf = NULL;
    UINT32                ResultBufSize = 0;
    UINT32                DigestSize=0;
    EFI_STATUS            Status;

    Cmd.Tag          = (TPMI_ST_COMMAND_TAG)TPM_H2NS(TPM_ST_SESSIONS);
    Cmd.CommandSize  = TPM_H2NL(sizeof(Cmd));
    Cmd.CommandCode  = TPM_H2NL(TPM_CC_PCR_Extend);

    Buffer = (UINT8 *)&Cmd.inputParameters;
    *(UINT32 *)Buffer = TPM_H2NL(PcrIndex);
    Buffer += sizeof(UINT32);

    AuthSizeOffset = Buffer;
    *(UINT32 *)Buffer = 0;
    Buffer += sizeof(UINT32);

    //  pcr authHandle
    *(UINT32 *)Buffer = TPM_H2NL(TPM_RS_PW);
    Buffer += sizeof(UINT32);

    // nonce = nullNonce
    *(UINT16 *)Buffer = 0;
    Buffer += sizeof(UINT16);

    // sessionAttributes = 0
    *(UINT8 *)Buffer = 0;
    Buffer += sizeof(UINT8);

    // auth = nullAuth
    *(UINT16 *)Buffer = 0;
    Buffer += sizeof(UINT16);

    // authorizationSize
    *(UINT32 *)AuthSizeOffset = TPM_H2NL((UINT32)(Buffer - AuthSizeOffset - sizeof(UINT32)));

    //Digest count
    *(UINT32 *)Buffer = TPM_H2NL(1);
    Buffer += sizeof(UINT32);


   *(UINT16 *)Buffer = TPM_H2NS(TPM2_ALG_SHA1);
    Buffer += sizeof(UINT16);
    DigestSize = 0x14;
    CopyMem( Buffer, &Digest->digest, DigestSize);

    Buffer += DigestSize;

    CmdSize = (UINT32)(Buffer - (UINT8 *)&Cmd);
    Cmd.CommandSize = TPM_H2NL(CmdSize);

    ResultBuf     = (UINT8 *) &Tmpres;
    ResultBufSize = sizeof(Res);
    
    Status = CrbSubmitCmd((UINT8 *)&Cmd, CmdSize, ResultBuf, &ResultBufSize);

    return Status;
}



EFI_STATUS
PeiTrEEExtend(
    IN  EFI_TREE_PPI        *TrEEPpi,
    IN  TPM_PCRINDEX        PcrIndex,
    IN  TPM2_DIGEST_VALUES  *Digest,
    OUT TPM2_DIGEST_VALUES  *NewPCRValue,
    IN  UINTN               HashAlgo)
{
    TPM2_PCRExtend_cmd_t  Cmd;
    TPM2_PCRExtend_res_t  Res;
    TPM2_PCRExtend_res_t  Tmpres;
    UINT32                CmdSize;
    UINT8                 *Buffer;
    UINT8                 *AuthSizeOffset;
    UINT8                 *ResultBuf = NULL;
    UINT32                ResultBufSize = 0;
    UINT32                DigestSize=0;
    EFI_STATUS            Status;
    UINT8                 *CrbSupportOfst;
    UINTN                 i=0;

    Cmd.Tag          = (TPMI_ST_COMMAND_TAG)TPM_H2NS(TPM_ST_SESSIONS);
    Cmd.CommandSize  = TPM_H2NL(sizeof(Cmd));
    Cmd.CommandCode  = TPM_H2NL(TPM_CC_PCR_Extend);

    Buffer = (UINT8 *)&Cmd.inputParameters;
    *(UINT32 *)Buffer = TPM_H2NL(PcrIndex);
    Buffer += sizeof(UINT32);

    AuthSizeOffset = Buffer;
    *(UINT32 *)Buffer = 0;
    Buffer += sizeof(UINT32);

    //  pcr authHandle
    *(UINT32 *)Buffer = TPM_H2NL(TPM_RS_PW);
    Buffer += sizeof(UINT32);

    // nonce = nullNonce
    *(UINT16 *)Buffer = 0;
    Buffer += sizeof(UINT16);

    // sessionAttributes = 0
    *(UINT8 *)Buffer = 0;
    Buffer += sizeof(UINT8);

    // auth = nullAuth
    *(UINT16 *)Buffer = 0;
    Buffer += sizeof(UINT16);

    // authorizationSize
    *(UINT32 *)AuthSizeOffset = TPM_H2NL((UINT32)(Buffer - AuthSizeOffset - sizeof(UINT32)));

    //Digest count
    *(UINT32 *)Buffer = TPM_H2NL(1);
    Buffer += sizeof(UINT32);


	// Get the digest size based on Hash Alg
    if(HashAlgo==TREE_BOOT_HASH_ALG_SHA1){
        //Hash alg
	    *(UINT16 *)Buffer = TPM_H2NS(TPM2_ALG_SHA1);
        Buffer += sizeof(UINT16);
        DigestSize = SHA1_DIGEST_SIZE;
        CopyMem(Buffer, &Digest->digests.sha1, DigestSize);
    }
    
    if(HashAlgo==TREE_BOOT_HASH_ALG_SHA256){
	    //Hash alg
	    *(UINT16 *)Buffer = TPM_H2NS(TPM2_ALG_SHA256);
        Buffer += sizeof(UINT16);
        DigestSize = SHA256_DIGEST_SIZE;
        CopyMem(Buffer, &Digest->digests.sha256, DigestSize);
    }

  
    Buffer += DigestSize;

    CmdSize = (UINT32)(Buffer - (UINT8 *)&Cmd);
    Cmd.CommandSize = TPM_H2NL(CmdSize);

    ResultBuf     = (UINT8 *) &Tmpres;
    CrbSupportOfst = (UINT8 *)&Tmpres;;
    ResultBufSize = sizeof(Res);
    
    Status = TrEEPpi->SubmitCommand(TrEEPpi, CmdSize, (UINT8 *)&Cmd, ResultBufSize, ResultBuf);

    for (i=0; i<0x80; i++)
    {
      if(i%16 == 0 && i!=0){
          DEBUG((-1, "\n")); 
      }
      DEBUG((-1, " %02x", *CrbSupportOfst)); 
      CrbSupportOfst+=1;
    }
    
    return Status;
}




EFI_STATUS
TrEEPeiHashLogExtentEventInternal(
  IN CONST EFI_PEI_SERVICES       **PeiServices,
  IN  EFI_TREE_PPI    *TrEEPpi,
  IN  UINT8                    *DataToHash,
  IN  UINT64                   Flags,
  IN  UINTN                    DataSize,
  IN  OUT  TCG_PCR_EVENT2_HDR  *NewEventHdr,
  IN  UINT8                    *NewEventData
)
{
  EFI_STATUS                Status;
  TPM2_DIGEST_VALUES        Out_Digest;
  UINT32                    TempSize=0;
  UINTN                     RequiredSpace=0;
  TCG_PLATFORM_SETUP_INTERFACE *TcgPeiPolicy;
  EFI_GUID                      gTcgPeiPolicyGuid =\
                                   TCG_PLATFORM_SETUP_PEI_POLICY_GUID;
  TCG_CONFIGURATION       ConfigFlags;
  
  DEBUG(( -1," TpmHashLogExtendEvent Entry \n"));
#if defined LOG_EV_EFI_ACTION && LOG_EV_EFI_ACTION == 0 
  if(NewEventHdr->EventType == EV_EFI_ACTION)
    {
      return EFI_SUCCESS;
  }
#endif
   Status = (*PeiServices)->LocatePpi(PeiServices,
                  &gTcgPeiPolicyGuid,
                  0, NULL,
                  &TcgPeiPolicy);

   if(EFI_ERROR(Status) || TcgPeiPolicy == NULL )return Status;
      
   Status = TcgPeiPolicy->getTcgPeiPolicy((EFI_PEI_SERVICES **)PeiServices, &ConfigFlags);
      
   if(TrEEPpi == NULL)return EFI_INVALID_PARAMETER;
   
   if(ConfigFlags.HashPolicy == TREE_BOOT_HASH_ALG_SHA1)
   {
      RequiredSpace = sizeof(TCG_PCR_EVENT2_HDR) - sizeof(NewEventHdr->Digests)\
                        + sizeof(NewEventHdr->Digests.digests.sha1) +
                        NewEventHdr->EventSize;
      Status = SHA1HashAll( NULL, DataToHash, DataSize, (UINT8 *)&NewEventHdr->Digests.digests.sha1);        
     if(EFI_ERROR(Status))return Status;
   }
   
   if(ConfigFlags.HashPolicy == TREE_BOOT_HASH_ALG_SHA256)  //for Sha256
   {
       RequiredSpace = sizeof(TCG_PCR_EVENT2_HDR) - sizeof(NewEventHdr->Digests)\
                              + sizeof(NewEventHdr->Digests.digests.sha256) +
                              NewEventHdr->EventSize;

       Status = SHA2HashAll( NULL, DataToHash, DataSize, (UINT8 *)&NewEventHdr->Digests.digests.sha256);        
       if(EFI_ERROR(Status))return Status;
   }

   if((RequiredSpace + LastEventPtr) > \
                (TreeEventLogLocation  + PEI_MAX_LOG_AREA_SIZE)){
       IEventLogTruncated = TRUE;
   }

   Status = PeiTrEEExtend(TrEEPpi,
                     NewEventHdr->PCRIndex,
                     &NewEventHdr->Digests,
                     &Out_Digest,
                     ConfigFlags.HashPolicy);

   DEBUG(( -1," PeiTrEEExtend Status = %r \n", Status));
   if(EFI_ERROR(Status))return Status;
     if(IEventLogTruncated)return EFI_SUCCESS;;

   DEBUG(( -1," LastEntry = %x \n", LastEntry));
     if(LastEntry == 0) return EFI_ABORTED;
    
   LastEventPtr = LastEntry;

   CopyMem((VOID*)(UINTN)LastEntry,
                                    NewEventHdr,
                                    sizeof(TCG_PCR_EVENT2_HDR) -sizeof(NewEventHdr->Digests) - sizeof(UINT32));   

    TempSize =  sizeof(TCG_PCR_EVENT2_HDR) -sizeof(NewEventHdr->Digests) - sizeof(UINT32);
    
    if(ConfigFlags.HashPolicy == TREE_BOOT_HASH_ALG_SHA1)
    {
            CopyMem((VOID*)(UINTN)(LastEntry + TempSize),
                            (UINT8 *)&NewEventHdr->Digests.digests.sha1,
                            sizeof(NewEventHdr->Digests.digests.sha1));
            
            TempSize+=sizeof(NewEventHdr->Digests.digests.sha1);
    }
    
    if(ConfigFlags.HashPolicy == TREE_BOOT_HASH_ALG_SHA256)
    {
           CopyMem((VOID*)(UINTN)(LastEntry + TempSize),
                                    (UINT8 *)&NewEventHdr->Digests.digests.sha256,
                                    sizeof(NewEventHdr->Digests.digests.sha256));
            
           TempSize+=sizeof(NewEventHdr->Digests.digests.sha256);
    }
    
    CopyMem((VOID*)(UINTN)(LastEntry + TempSize) ,
                    (UINT8 *)&NewEventHdr->EventSize,
                    sizeof(UINT32));
  
    TempSize+=sizeof(UINT32);

    CopyMem((VOID*)(UINTN)(LastEntry + TempSize) ,
                           NewEventData,
                           NewEventHdr->EventSize);
  
    
    LastEntry = LastEventPtr + ((EFI_PHYSICAL_ADDRESS)(UINTN)(NewEventHdr->EventSize \
                              + TempSize));

    TcgLog->TableSize += RequiredSpace;   
    return EFI_SUCCESS;   
}





EFI_STATUS
EFIAPI Tpm20GetEventLog(
    IN EFI_PEI_SERVICES **PeiServices,
    OUT TCG_LOG_HOB     **EventLog )
{
    EFI_STATUS Status;
    VOID       *HobStart;

    Status = (*PeiServices)->GetHobList( PeiServices, &HobStart );

    if ( EFI_ERROR( Status ))
    {
        return Status;
    }

    return TcgGetNextGuidHob( &HobStart, &gEfiCrbPeiAmiTcgLogHobGuid, EventLog, NULL );
}



EFI_STATUS
TrEEHashLogExtentEvent (
  IN CONST EFI_PEI_SERVICES       **PeiServices,
  IN  EFI_TREE_PPI          *This,
  IN  UINT64                Flags,
  IN  EFI_PHYSICAL_ADDRESS  DataToHash,
  IN  UINT64                DataToHashLen,
  IN  TrEE_EVENT            *TreeEvent
  )
{
  EFI_STATUS            Status     = EFI_SUCCESS;
  TCG_PCR_EVENT2_HDR    TcgEvent;
  
  if((This == NULL ) || (DataToHash == 0) || (TreeEvent == NULL)) {
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }
  else if(TreeEvent->Size < (TreeEvent->Header.HeaderSize + sizeof(UINT32))\
                                 || (TreeEvent->Header.PCRIndex > 23)){
   Status = EFI_INVALID_PARAMETER;
   goto Exit;
  }
  
  TcgEvent.PCRIndex= TreeEvent->Header.PCRIndex;
  TcgEvent.EventType = TreeEvent->Header.EventType;
  TcgEvent.EventSize = TreeEvent->Size - sizeof(TrEE_EVENT_HEADER) \
                        -sizeof(UINT32);
  
  Status = TrEEPeiHashLogExtentEventInternal(PeiServices,
                       This,
                      (UINT8 *)(UINTN)DataToHash,
                      Flags,
                      (UINTN)  DataToHashLen,
                      &TcgEvent,
                      (UINT8 *)&TreeEvent->Event
                      );
Exit:
    return Status;
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   InternalPeiBuildHobGuid
//
// Description: Internal abstracted function to create a Hob
//
// Input:       IN  EFI_PEI_SERVICES  **PeiServices,
//              IN  EFI_GUID          *Guid,
//              IN  UINTN             DataLength,
//              OUT VOID              **Hob
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS InternalPeiBuildHobGuid(
    IN EFI_PEI_SERVICES **PeiServices,
    IN EFI_GUID         *Guid,
    IN UINTN            DataLength,
    OUT VOID            **Hob )
{
    EFI_STATUS Status;

    Status = (*PeiServices)->CreateHob(
        PeiServices,
        EFI_HOB_TYPE_GUID_EXTENSION,
        (UINT16) ( sizeof (EFI_HOB_GUID_TYPE) + DataLength ),
        Hob
        );

    if ( EFI_ERROR( Status ))
    {
        return Status;
    }
    
    DEBUG((-1, "Hob created \n")); 
    ((EFI_HOB_GUID_TYPE*)(*Hob))->Name = *Guid;

    return EFI_SUCCESS;
}



EFI_STATUS
TreeGetEventLog (
  IN  EFI_TREE_PPI           *This,
  IN  TREE_EVENTLOGTYPE      EventLogFormat,
  OUT EFI_PHYSICAL_ADDRESS  *EventLogLocation,
  OUT EFI_PHYSICAL_ADDRESS  *EventLogLastEntry,
  OUT BOOLEAN               *EventLogTruncated
    )
{
    EFI_STATUS Status = EFI_SUCCESS;

    if(EventLogFormat != TREE_EVENT_LOG_FORMAT_TCG_1_2)
    {
        Status = EFI_INVALID_PARAMETER;
    }

    *EventLogLocation  = TreeEventLogLocation;
    *EventLogLastEntry = LastEventPtr;
    *EventLogTruncated = IEventLogTruncated;  

    return Status;
}



static EFI_TREE_PPI   CrbmTpmPrivate = {
     TreeGetCapability,
     TreeGetEventLog,
     TrEEHashLogExtentEvent,
     TrEECRBPeiTransmit
};

static EFI_PEI_PPI_DESCRIPTOR mCrbPpiList[] = {
     EFI_PEI_PPI_DESCRIPTOR_PPI
     | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
     &gTrEEPeiGuid,
     &CrbmTpmPrivate
};


static EFI_TREE_PPI   TisTpmPrivate = {
     TreeGetCapability,
     TreeGetEventLog,
     TrEEHashLogExtentEvent,
     TrEETisPeiTransmit
};

static EFI_PEI_PPI_DESCRIPTOR mTisPpiList[] = {
    EFI_PEI_PPI_DESCRIPTOR_PPI
    | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gTrEEPeiGuid,
    &TisTpmPrivate
};



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TrEEPeiEntry
//
// Description: 
//
//
// Input:       
//
// Output:     
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
EFIAPI TrEEPeiEntry(
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN CONST EFI_PEI_SERVICES    **PeiServices)
{
    EFI_STATUS              Status;
    TCG_CONFIGURATION       ConfigFlags;
    BOOLEAN                 InterfaceSupported = FALSE;
    BOOLEAN                 ErrorCrbInterface=FALSE;
    EFI_PHYSICAL_ADDRESS    TPM_Base = (EFI_PHYSICAL_ADDRESS)PORT_TPM_IOMEMBASE;
    TCG_PLATFORM_SETUP_INTERFACE *TcgPeiPolicy;
    EFI_GUID                        gTcgPeiPolicyGuid =\
                                        TCG_PLATFORM_SETUP_PEI_POLICY_GUID;
    EFI_HOB_GUID_TYPE       *Hob;
    EFI_GUID gTpm20Supporthobguid = TPM20_HOB_GUID;
       
    Status = (*PeiServices)->LocatePpi(
                PeiServices,
                &gTcgPeiPolicyGuid,
                0, NULL,
                &TcgPeiPolicy);

    if(EFI_ERROR(Status) || TcgPeiPolicy == NULL )return Status;
    
    Status = TcgPeiPolicy->getTcgPeiPolicy((EFI_PEI_SERVICES **)PeiServices, &ConfigFlags);
    
    if(EFI_ERROR(Status))return Status;
    
    if(ConfigFlags.DeviceType == 0) return EFI_SUCCESS;
    
    if(ConfigFlags.TpmSupport == 0) return EFI_SUCCESS;
       
    if(ConfigFlags.InterfaceSel == 0){
        //check if CRB interface is supported
        if(CrbSupported()!=0){
            InterfaceSupported = TRUE;
        }else{
            ConfigFlags.InterfaceSel = 1;
        }
    }
    
    if(ConfigFlags.InterfaceSel == 1){
        //check if TIS interface
        Status = IsTpmPresent((TPM_1_2_REGISTERS_PTR)( UINTN ) TPM_Base );
        if(!EFI_ERROR(Status)){
            InterfaceSupported = TRUE;
        }
    }
    
    if(InterfaceSupported){
        //Install TrEE for the Interface Type (TIS or CRB)
        if(ConfigFlags.InterfaceSel == 0)
        {
            if(CrbSupported()==0x02){
                //dTPM 2.0 CRB interface
                if(IsPTP())
                {
                    DEBUG((-1,  "Calling SelectAndLockInterface\n")); 
                    Status = SelectAndLockInterface( PeiServices, (~ConfigFlags.InterfaceSel & 01));
                    if(EFI_ERROR(Status)) ErrorCrbInterface = TRUE;  
                }
                
                //install TrEEPei
                if(ErrorCrbInterface != TRUE){
                    Status = (*PeiServices)->InstallPpi( PeiServices, mCrbPpiList );   
                }
                
            }else if(CrbSupported()==0x01){
                Status = (*PeiServices)->InstallPpi( PeiServices, mCrbPpiList );   
            }
            
            if(ErrorCrbInterface == TRUE)ConfigFlags.InterfaceSel = 1;
        }
        
        if(ConfigFlags.InterfaceSel == 1){
            
            //dTPM 2.0 TIS interface
            Status = IsTpmPresent((TPM_1_2_REGISTERS_PTR)(UINTN )TPM_Base );
            if(!EFI_ERROR(Status)){
                if(IsPTP())
                {
                    DEBUG((-1,  "Calling SelectAndLockInterface\n")); 
                    Status = SelectAndLockInterface( PeiServices, (~ConfigFlags.InterfaceSel & 01));
                    if(!EFI_ERROR(Status)){
                        Status = (*PeiServices)->InstallPpi( PeiServices, mTisPpiList );
                    }
                    else{
                        ErrorCrbInterface = TRUE;
                    }
                }else{
                    Status = (*PeiServices)->InstallPpi( PeiServices, mTisPpiList );
                }
            }
        }
        
        if(!ErrorCrbInterface && !EFI_ERROR(Status)){
            
            Status = InternalPeiBuildHobGuid((EFI_PEI_SERVICES **)PeiServices, &gEfiCrbPeiAmiTcgLogHobGuid,
                   (sizeof (*TcgLog) + 0x200),  & Hob);

            DEBUG((-1, "CrbBuild Hob Status = %r \n", Status)); 

            TcgLog = (TCG_LOG_HOB*)(Hob + 1);
            (*PeiServices)->SetMem( TcgLog, sizeof (*TcgLog), 0 );
            TcgLog->TableMaxSize = PEI_MAX_LOG_AREA_SIZE;
            TcgLog->TableSize = 0;
            
            TreeEventLogLocation = (EFI_PHYSICAL_ADDRESS) (UINTN)TcgLog;
            LastEventPtr = TreeEventLogLocation + sizeof(TCG_LOG_HOB);
            LastEntry = LastEventPtr;
        }
    }
        
    return Status;
}
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**     5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093            **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
