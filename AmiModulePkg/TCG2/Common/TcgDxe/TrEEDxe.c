/*++
   Module Name:

   TcgDxe.c

   Abstract:

   DXE Driver that provides TCG services

   --*/
//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/TcgNext/Common/TcgDxe/TcgDxe20.c 2     10/09/13 6:30p Fredericko $
//
// $Revision: 2 $
//
// $Date: 10/09/13 6:30p $
//*************************************************************************
// Revision History
// ----------------
// $Log:
#include <Uefi.h>
#include "AmiTcg\TcgCommon.h"
#include <AmiTcg\TcgMisc.h>
#include <Token.h>
#include <AmiTcg\Tpm20.h>
#include <AmiTcg\TrEEProtocol.h>
#include "protocol\TpmDevice.h"
#include <Protocol\ComponentName.h>
#include <Protocol\ComponentName2.h>
#include <Protocol\DriverBinding.h>
#include <Protocol\AcpiSupport.h>
#include "Protocol\TcgPlatformSetupPolicy.h"
#include <industrystandard\Acpi30.h>
#include <Acpi.h>
#include "../../CRB_Lib/Tpm20CRBLib.h"
#include<Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include<Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/AcpiSystemDescriptionTable.h>
#include <Protocol/AcpiSupport.h>
#include <TrEEDxe.h>
#include <Library/IoLib.h>


/*
  locates the TPM20 hob from Pei. If found we are processing TPM 20 devic
  need to install the TreeProtocol and do TPM20 binding measurements
*/
#define EFI_ACPI_TABLE_VERSION_X        (EFI_ACPI_TABLE_VERSION_2_0 | EFI_ACPI_TABLE_VERSION_3_0 | EFI_ACPI_TABLE_VERSION_4_0)
#define EFI_ACPI_TABLE_VERSION_ALL      (EFI_ACPI_TABLE_VERSION_1_0B|EFI_ACPI_TABLE_VERSION_X)

extern EFI_GUID gInternalAcpiSupportGuid;

EFI_EVENT                   Event;
static VOID                 *reg;

#define     MAX_LOG_AREA_SIZE (32 * 1024) // 32KB

static EFI_PHYSICAL_ADDRESS TreeEventLogLocation;
static EFI_PHYSICAL_ADDRESS LastEntry = 0;
static EFI_PHYSICAL_ADDRESS LastEventPtr = 0;
static EFI_TPM_DEVICE_PROTOCOL *TpmDevice=NULL;
Tpm20DeviceHob  *TpmSupport = NULL;
static BOOLEAN  IEventLogTruncated = FALSE;

BOOLEAN IsTpm20Device();
EFI_STATUS TcgLibGetDsdt(EFI_PHYSICAL_ADDRESS *DsdtAddr, EFI_ACPI_TABLE_VERSION Version);
EFI_STATUS TcgUpdateAslNameObject(ACPI_HDR *PDsdt, UINT8 *ObjName, UINT64 Value);
BOOLEAN StrcmpNoCase(CHAR8 *Str1, CHAR8 *Str2);
BOOLEAN IsPTP();

EFI_STATUS
__stdcall
TpmLibPassThrough (
  IN      TPM_1_2_REGISTERS_PTR     TpmReg,
  IN      UINTN                     NoInputBuffers,
  IN      TPM_TRANSMIT_BUFFER       *InputBuffers,
  IN      UINTN                     NoOutputBuffers,
  IN OUT  TPM_TRANSMIT_BUFFER       *OutputBuffers
  );

EFI_STATUS
__stdcall
TisRequestLocality (
  IN      TPM_1_2_REGISTERS_PTR     TpmReg
  );

EFI_STATUS
__stdcall
TisReleaseLocality (
  IN      TPM_1_2_REGISTERS_PTR     TpmReg
  );


EFI_STATUS
TreeSubmitCommand (
IN  EFI_TREE_PROTOCOL   *This,
IN  UINT32              InputParameterBlockSize,
IN  UINT8               *InputParameterBlock,
IN  UINT32              OutputParameterBlockSize,
IN  UINT8               *OutputParameterBlock
  );

static UINT16   *gDriverName=L"TPM Driver";
static BOOLEAN   Tpm20FwDevice = FALSE;


UINT8  GetHashPolicy()
{
    TCG_PLATFORM_SETUP_PROTOCOL     *ProtocolInstance;
    EFI_STATUS                      Status;
    EFI_GUID                        Policyguid = TCG_PLATFORM_SETUP_POLICY_GUID;

   Status = gBS->LocateProtocol (&Policyguid,  NULL, &ProtocolInstance);
   if (EFI_ERROR (Status)) {
      return 0;
   }
   
   return(ProtocolInstance->ConfigFlags.HashPolicy);
}

BOOLEAN IsTpm20Device()
{
   EFI_GUID gTpm20Supporthobguid = TPM20_HOB_GUID;
   
   TpmSupport = LocateATcgHob( gST->NumberOfTableEntries,
                  gST->ConfigurationTable,
                  &gTpm20Supporthobguid);

   if(TpmSupport != NULL){
      if(TpmSupport->Tpm20DeviceState == 1){
           return TRUE;
      }
   }  
      
   return FALSE;
}


EFI_STATUS
__stdcall TcgCommonPassThrough(
    IN VOID                    *Context,
    IN UINT32                  NoInputBuffers,
    IN TPM_TRANSMIT_BUFFER     *InputBuffers,
    IN UINT32                  NoOutputBuffers,
    IN OUT TPM_TRANSMIT_BUFFER *OutputBuffers )
{
    return EFI_UNSUPPORTED;

}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TcmCommonPassThrough
//
// Description: Helper function for TCM transmit command
//
// Input:       VOID *Context
//              UINT32 NoInputBuffers
//              TPM_TRANSMIT_BUFFER InputBuffers
//              UINT32 NoOutputBuffers
//              TPM_TRANSMIT_BUFFER OutputBuffers
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
EFI_STATUS
__stdcall TcmCommonPassThrough(
    IN VOID                    *Context,
    IN UINT32                  NoInputBuffers,
    IN TPM_TRANSMIT_BUFFER     *InputBuffers,
    IN UINT32                  NoOutputBuffers,
    IN OUT TPM_TRANSMIT_BUFFER *OutputBuffers )
{
    return EFI_UNSUPPORTED;
}



UINT8  GetInterfacePolicy()
{
    TCG_PLATFORM_SETUP_PROTOCOL     *ProtocolInstance;
    EFI_STATUS                      Status;
    EFI_GUID                        Policyguid = TCG_PLATFORM_SETUP_POLICY_GUID;

   Status = gBS->LocateProtocol (&Policyguid,  NULL, &ProtocolInstance);
   if (EFI_ERROR (Status)) {
      return 0;
   }

   return(ProtocolInstance->ConfigFlags.InterfaceSel);
}

EFI_STATUS
EFIAPI
Tpm2GetCapability (
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
  EFI_TREE_PROTOCOL                 *pTreeProtocol  =  NULL;
  EFI_GUID  gEfiTrEEProtocolGuid =  EFI_TREE_PROTOCOL_GUID;

  Status = gBS->LocateProtocol(
                  &gEfiTrEEProtocolGuid,
                  NULL,
                  &pTreeProtocol);

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
  Status = TreeSubmitCommand (pTreeProtocol, SendBufferSize, (UINT8 *)&SendBuffer, RecvBufferSize, (UINT8 *)&RecvBuffer );
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
  gBS->CopyMem (CapabilityData, &RecvBuffer.CapabilityData, RecvBufferSize - sizeof (TPM2_RESPONSE_HEADER) - sizeof (UINT8));
  
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
Tpm2GetCapabilityManufactureID (
  OUT     UINT32                    *ManufactureId
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  EFI_STATUS              Status; 

  Status = Tpm2GetCapability (
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
  OUT UINT32                    *MaxCommandSize,
  OUT UINT32                    *MaxResponseSize
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  EFI_STATUS              Status;

  Status = Tpm2GetCapability (
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

  Status = Tpm2GetCapability (
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
  OUT TPML_ALG_PROPERTY      *AlgList
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  UINTN                   Index;
  EFI_STATUS              Status;
 
  Status = Tpm2GetCapability (
             TPM_CAP_ALGS, 
             1, 
             1, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  gBS->CopyMem (AlgList, &TpmCap.data.algorithms, sizeof (TPML_ALG_PROPERTY));

  AlgList->count = SwapBytes32 (AlgList->count);
  for (Index = 0; Index < AlgList->count; Index++) {
    AlgList->algProperties[Index].alg = SwapBytes16 (AlgList->algProperties[Index].alg);
    *(UINT32 *)&AlgList->algProperties[Index].algProperties = SwapBytes32 (*(UINT32 *)&AlgList->algProperties[Index].algProperties);
  }
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
Tpm20ComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  )
{
    //Supports only English
	if(!Language || !DriverName) return EFI_INVALID_PARAMETER;

	if (!StrcmpNoCase( Language, "en-US") &&
        !StrcmpNoCase( Language, "eng"))
        return EFI_UNSUPPORTED;
	else 
        *DriverName=gDriverName;
	
	return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
Tpm20ComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL                     *This,
  IN  EFI_HANDLE                                      ControllerHandle,
  IN  EFI_HANDLE                                      ChildHandle        OPTIONAL,
  IN  CHAR8                                           *Language,
  OUT CHAR16                                          **ControllerName
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
Tpm20ComponentNameGetDriverName2 (
  IN  EFI_COMPONENT_NAME2_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  )
{
    //Supports only English
	if(!Language || !DriverName) return EFI_INVALID_PARAMETER;
//LANGUAGE_CODE_ENGLISH
	if (!StrcmpNoCase( Language, "en-US") &&
        !StrcmpNoCase( Language, "eng"))
        return EFI_UNSUPPORTED;
	else 
        *DriverName=gDriverName;
	
	return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
Tpm20ComponentNameGetControllerName2 (
  IN  EFI_COMPONENT_NAME2_PROTOCOL                     *This,
  IN  EFI_HANDLE                                      ControllerHandle,
  IN  EFI_HANDLE                                      ChildHandle        OPTIONAL,
  IN  CHAR8                                           *Language,
  OUT CHAR16                                          **ControllerName
  )
{
  return EFI_UNSUPPORTED;
}


static EFI_COMPONENT_NAME_PROTOCOL gComponentName = {
  Tpm20ComponentNameGetDriverName,
  Tpm20ComponentNameGetControllerName,
  "en-US"
};


static EFI_COMPONENT_NAME2_PROTOCOL gComponentName2 = {
  Tpm20ComponentNameGetDriverName2,
  Tpm20ComponentNameGetControllerName2,
  "en-US"
};


EFI_STATUS
EFIAPI
Tpm20DriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath   OPTIONAL
  )
{
  EFI_STATUS          Status      =  EFI_SUCCESS;
  static BOOLEAN      Initialized = FALSE;

  if(Initialized == FALSE){
        Initialized = TRUE;
    if(IsTpm20Device()){
        return EFI_SUCCESS;
    }else if(Tpm20FwDevice)return EFI_SUCCESS;
  }
 
  return EFI_UNSUPPORTED;
}


EFI_STATUS
EFIAPI
Tpm20DriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath   OPTIONAL
  )
{
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
Tpm20DriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer   OPTIONAL
  )
{
	return EFI_UNSUPPORTED;
}

EFI_DRIVER_BINDING_PROTOCOL Tpm20DriverBinding = {
  Tpm20DriverBindingSupported,
  Tpm20DriverBindingStart,
  Tpm20DriverBindingStop,
  0xa,
  NULL,
  NULL
};




EFI_STATUS
TreeGetCapability (
  IN EFI_TREE_PROTOCOL                *This,
  IN OUT TREE_BOOT_SERVICE_CAPABILITY *ProtocolCapability
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;
  UINT8       HashAlg = GetHashPolicy();
  static UINT32 ManufactureID = 0xFFFFFFFF;
  static UINT32 MaxResponseSize = 0xFFFFFFFF;
  static UINT32 MaxCommandSize  = 0xFFFFFFFF;
  TPM_CRB_ACCESS_REG_PTR dCrbAccessRegPtr = (TPM_CRB_ACCESS_REG_PTR)(( UINTN ) (PORT_TPM_IOMEMBASE));
 
  if((ProtocolCapability == NULL) || (This == NULL)){
    Status = EFI_INVALID_PARAMETER;
  }
  else {

    if(ManufactureID == 0xFFFFFFFF && MaxResponseSize == 0xFFFFFFFF && MaxCommandSize ==0xFFFFFFFF)
    {
        Status = Tpm2GetCapabilityManufactureID (&ManufactureID);
        if(EFI_ERROR(Status)){
            ManufactureID=0x0;
            Status = EFI_SUCCESS;
        }

        Status = Tpm2GetCapabilityMaxCommandResponseSize (&MaxCommandSize, &MaxResponseSize);
        if(EFI_ERROR(Status) && !IsPTP()){
            MaxCommandSize = 0x800;
            MaxResponseSize = 0x800;
            Status = EFI_SUCCESS;
        }
        
        if(MaxCommandSize == 0 || MaxResponseSize == 0){
            if(IsPTP())
            {
                MaxCommandSize = dCrbAccessRegPtr->TpmCrbCtrlCmdSize;
                MaxResponseSize = dCrbAccessRegPtr->TpmCrbCtrlRespSize;
                ManufactureID = dCrbAccessRegPtr->TpmCrbIntfId[1];
            }
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

        if( HashAlg == 1){
            ProtocolCapability->HashAlgorithmBitmap = TREE_BOOT_HASH_ALG_SHA1;
        }else if(HashAlg == 2){
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
TreeGetEventLog (
  IN  EFI_TREE_PROTOCOL     *This,
  IN  TREE_EVENTLOGTYPE     EventLogFormat,
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

EFI_STATUS
InternalTcg20CommonExtend(
    IN  EFI_TREE_PROTOCOL  *TrEEProtocol,
    IN  TPM_PCRINDEX PcrIndex,
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
        TcgCommonCopyMem(NULL, Buffer, &Digest->digests.sha1, DigestSize);
    }
    
    if(HashAlgo==TREE_BOOT_HASH_ALG_SHA256){
	    //Hash alg
	    *(UINT16 *)Buffer = TPM_H2NS(TPM2_ALG_SHA256);
        Buffer += sizeof(UINT16);
        DigestSize = SHA256_DIGEST_SIZE;
        TcgCommonCopyMem(NULL, Buffer, &Digest->digests.sha256, DigestSize);
    }

    Buffer += DigestSize;

    CmdSize = (UINT32)(Buffer - (UINT8 *)&Cmd);
    Cmd.CommandSize = TPM_H2NL(CmdSize);

    ResultBuf     = (UINT8 *) &Tmpres;
    CrbSupportOfst = (UINT8 *)&Tmpres;;
    ResultBufSize = sizeof(Res);
    
    Status  = TrEEProtocol->SubmitCommand(TrEEProtocol,CmdSize, (UINT8 *)&Cmd, ResultBufSize, ResultBuf);

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
TpmHashLogExtendEventI(
  IN  EFI_TREE_PROTOCOL         *This,
  IN  UINT8                     *DataToHash,
  IN  UINT64                    Flags,
  IN  UINTN                     DataSize,
  IN  OUT  TCG_PCR_EVENT2_HDR   *NewEventHdr,
  IN      UINT8                 *NewEventData
)
{
  EFI_STATUS                Status;
  TPM2_DIGEST_VALUES        Out_Digest;
  static UINT32             HashAlgo = 0xFFFFFFFF;
  UINT32                    TempSize=0;
  UINTN                     RequiredSpace=0;
  TCG_PLATFORM_SETUP_PROTOCOL     *PolicyInstance;
  EFI_GUID                        gPolicyguid = TCG_PLATFORM_SETUP_POLICY_GUID;

  DEBUG(( -1," TpmHashLogExtendEvent Entry \n"));
#if defined LOG_EV_EFI_ACTION && LOG_EV_EFI_ACTION == 0 
  if(NewEventHdr->EventType == EV_EFI_ACTION)
    {
      return EFI_SUCCESS;
  }
#endif
  
   if(HashAlgo = 0xFFFFFFFF){       
      Status = gBS->LocateProtocol (&gPolicyguid,  NULL, &PolicyInstance);
      if (EFI_ERROR (Status)) {
          HashAlgo = TREE_BOOT_HASH_ALG_SHA1;
      }else{
          HashAlgo = PolicyInstance->ConfigFlags.HashPolicy;
      }
    }

    if(HashAlgo == TREE_BOOT_HASH_ALG_SHA1){
          RequiredSpace = sizeof(TCG_PCR_EVENT2_HDR) - sizeof(NewEventHdr->Digests)\
                          + sizeof(NewEventHdr->Digests.digests.sha1) +
                          NewEventHdr->EventSize;
 
          if(DataToHash!=NULL)
          {
              Status = SHA1HashAll( NULL, DataToHash, DataSize, (UINT8 *)&NewEventHdr->Digests.digests.sha1 );        
              DEBUG(( -1," SHA1HashAll Status = %r \n", Status));
              if(EFI_ERROR(Status))return Status;        
          }
      }
     
     if(HashAlgo == TREE_BOOT_HASH_ALG_SHA256){
         
         RequiredSpace = sizeof(TCG_PCR_EVENT2_HDR) - sizeof(NewEventHdr->Digests)\
                                   + sizeof(NewEventHdr->Digests.digests.sha256) +
                                   NewEventHdr->EventSize;
         
         if(DataToHash!=NULL)
         {
             Status = SHA2HashAll( NULL, DataToHash, DataSize, (UINT8 *)&NewEventHdr->Digests.digests.sha256);        
               if(EFI_ERROR(Status))return Status;
         }
     }



     if((RequiredSpace + LastEventPtr) > \
                    (TreeEventLogLocation  + MAX_LOG_AREA_SIZE)){
                     IEventLogTruncated = TRUE;
     }

     Status = InternalTcg20CommonExtend(This,
                     NewEventHdr->PCRIndex,
                     &NewEventHdr->Digests,
                     &Out_Digest,
                     HashAlgo);

     DEBUG(( -1," InternalTcg20CommonExtend Status = %r \n", Status));
     if(EFI_ERROR(Status))return Status;
   
     if(IEventLogTruncated)return EFI_SUCCESS;;

     DEBUG(( -1," LastEntry = %x \n", LastEntry));
     if(LastEntry == 0) return EFI_ABORTED;
    
     LastEventPtr = LastEntry;
   
     CopyMem((VOID*)(UINTN)LastEntry,NewEventHdr,sizeof(TCG_PCR_EVENT2_HDR) 
                                           -sizeof(NewEventHdr->Digests) - sizeof(UINT32));   

     TempSize =  sizeof(TCG_PCR_EVENT2_HDR) -sizeof(NewEventHdr->Digests) - sizeof(UINT32);
       

     if(HashAlgo == TREE_BOOT_HASH_ALG_SHA1){ //log event record
         gBS->CopyMem(
                 (VOID*)(UINTN)(LastEntry + TempSize) ,
                 (UINT8 *)&NewEventHdr->Digests.digests.sha1,
                 sizeof(NewEventHdr->Digests.digests.sha1));

         TempSize+=sizeof(NewEventHdr->Digests.digests.sha1);
       
     }
   
     if(HashAlgo == TREE_BOOT_HASH_ALG_SHA256){
         gBS->CopyMem(
            (VOID*)(UINTN)(LastEntry + TempSize) ,
            (UINT8 *)&NewEventHdr->Digests.digests.sha256,
            sizeof(NewEventHdr->Digests.digests.sha256));

         TempSize+=sizeof(NewEventHdr->Digests.digests.sha256);
     }

     gBS->CopyMem(
             (VOID*)(UINTN)(LastEntry + TempSize),
             (UINT8 *)&NewEventHdr->EventSize,
             sizeof(UINT32));
    
     TempSize+=sizeof(UINT32);

     gBS->CopyMem(
             (VOID*)(UINTN)(LastEntry + TempSize),
             NewEventData,
             NewEventHdr->EventSize);
    
             LastEntry = LastEventPtr + ((EFI_PHYSICAL_ADDRESS)(UINTN)(NewEventHdr->EventSize \
                              + TempSize));
    
     return EFI_SUCCESS;   
}

EFI_STATUS
TreeHashLogExtentEvent (
  IN  EFI_TREE_PROTOCOL     *This,
  IN  UINT64                Flags,
  IN  EFI_PHYSICAL_ADDRESS  DataToHash,
  IN  UINT64                DataToHashLen,
  IN  TrEE_EVENT            *TreeEvent
  )
{
  EFI_STATUS            Status     = EFI_SUCCESS;
  TCG_PCR_EVENT2_HDR     TcgEvent;
  
  if((This == NULL ) || (DataToHash == 0) || (TreeEvent == NULL)) {
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }
  else if(TreeEvent->Size < (TreeEvent->Header.HeaderSize + sizeof(UINT32))\
                                 || (TreeEvent->Header.PCRIndex > 23)){
   Status = EFI_INVALID_PARAMETER;
   goto Exit;
  }

  TcgEvent.PCRIndex  = TreeEvent->Header.PCRIndex;
  TcgEvent.EventType = TreeEvent->Header.EventType;
  TcgEvent.EventSize = TreeEvent->Size - sizeof(TrEE_EVENT_HEADER) \
                        -sizeof(UINT32);
  
  Status = TpmHashLogExtendEventI(This,
             (UINT8 *)(UINTN)DataToHash,
                      Flags,
             (UINTN)  DataToHashLen,
                      &TcgEvent,
             (UINT8 *)&TreeEvent->Event
                      );

Exit:

	return Status;
}


EFI_STATUS
TreeSubmitCommand (
IN  EFI_TREE_PROTOCOL   *This,
IN  UINT32              InputParameterBlockSize,
IN  UINT8               *InputParameterBlock,
IN  UINT32              OutputParameterBlockSize,
IN  UINT8               *OutputParameterBlock
  )
{
    EFI_STATUS            Status     = EFI_SUCCESS;
    UINT32                ReturnSize = 0;
    UINT32                Size = 0;
    TPM_TRANSMIT_BUFFER   InBuffer[1], OutBuffer[1];
    TPM_1_2_REGISTERS_PTR     TpmReg = (TPM_1_2_REGISTERS_PTR)(UINTN)PORT_TPM_IOMEMBASE;

    if (This == NULL || InputParameterBlock == NULL || OutputParameterBlock == NULL) {
        return EFI_INVALID_PARAMETER;
    }
        
    if(TpmSupport->InterfaceType == 1){
        
         ReturnSize = OutputParameterBlockSize;

         TisRequestLocality ( TpmReg );

         InBuffer[0].Buffer  = InputParameterBlock;
         InBuffer[0].Size    = InputParameterBlockSize;
         OutBuffer[0].Buffer = OutputParameterBlock;
         OutBuffer[0].Size   = OutputParameterBlockSize;

         TpmLibPassThrough(TpmReg,sizeof (InBuffer) / sizeof (*InBuffer),
                         InBuffer,sizeof (OutBuffer) / sizeof (*OutBuffer),
                         OutBuffer);

         TisReleaseLocality ( TpmReg );
         
    }else{

        Size = OutputParameterBlockSize;
        Status = CrbSubmitCmd(InputParameterBlock,
                       InputParameterBlockSize,
                       OutputParameterBlock,
                       &Size);
    }

    return Status;
}


static EFI_STATUS CopyTcgLog(
    void )
{
    TCG_LOG_HOB     *TcgLog = NULL;
    void**          DummyPtr;
    EFI_GUID        gEfiPeiLogHobGuid = EFI_TCG_TREE_LOG_HOB_GUID;

    TcgLog = (TCG_LOG_HOB*)                   LocateATcgHob(
        gST->NumberOfTableEntries,
        gST->ConfigurationTable,
        &gEfiPeiLogHobGuid );

    DummyPtr = &TcgLog;

    if ( *DummyPtr == NULL )
    {
        return EFI_NOT_FOUND;
    }
    
    TcgLog->TableMaxSize = MAX_LOG_AREA_SIZE;
    
    gBS->CopyMem(
            (UINT8 *)(UINTN)TreeEventLogLocation,
            ((UINT8 *)TcgLog),
            sizeof(TCG_LOG_HOB)
            );

    gBS->CopyMem(
        (UINT8 *)(UINTN)TreeEventLogLocation,
        (((UINT8 *)TcgLog) + sizeof(TCG_LOG_HOB)),
        TcgLog->TableSize
        );

    LastEntry = TreeEventLogLocation  +  TcgLog->TableSize;

    return EFI_SUCCESS;
}


EFI_TREE_PROTOCOL mTreeProtocol = {
  TreeGetCapability,
  TreeGetEventLog,
  TreeHashLogExtentEvent,
  TreeSubmitCommand
};

AMI_INTERNAL_HLXE_PROTOCOL  InternalLogProtocol = {
  TpmHashLogExtendEventI
};



EFI_STATUS
InstallTrEEProtocol(
  IN EFI_HANDLE Handle
  )
{    
  EFI_STATUS Status;
  EFI_GUID  gEfiTrEEProtocolGuid =  EFI_TREE_PROTOCOL_GUID;
  EFI_GUID  gEfiAmiHLXEGuid =  AMI_PROTOCOL_INTERNAL_HLXE_GUID;

  DEBUG(( -1," InstallTrEEProtocol \n"));
  
  //interface installation is 
  Status =  gBS->InstallProtocolInterface (
                      &Handle,
                      &gEfiTrEEProtocolGuid,
                      EFI_NATIVE_INTERFACE,
                      &mTreeProtocol);

  if(EFI_ERROR(Status))return Status;

  Status =  gBS->InstallProtocolInterface (
                &Handle,
                &gEfiAmiHLXEGuid,
                EFI_NATIVE_INTERFACE,
                &InternalLogProtocol
                );
  
  if(EFI_ERROR(Status))return Status;

  Status = gBS->AllocatePages(AllocateAnyPages,
                      EfiACPIMemoryNVS,
                      EFI_SIZE_TO_PAGES(MAX_LOG_AREA_SIZE),
                      (UINT64*)(UINTN)&TreeEventLogLocation);

  if(EFI_ERROR(Status))return Status;
   
      gBS->SetMem(
            (VOID*)((UINTN)TreeEventLogLocation),
            (UINTN)MAX_LOG_AREA_SIZE,
            0x00);  

  //locate PEI hob and copy to the TreeLogArea
  Status = CopyTcgLog();
    
  DEBUG(( -1," InstallTrEEProtocol Exit Status = %r \n", Status));
  return Status;
}


VOID TrEEUpdateTpmDeviceASL(    
    IN EFI_EVENT ev,
    IN VOID      *ctx)
{   
   ACPI_HDR                    *dsdt;
   EFI_PHYSICAL_ADDRESS        dsdtAddress=0;
   EFI_STATUS                  Status;
   UINT64                      Value;

    //locate AcpiProtocol
    Status = TcgLibGetDsdt(&dsdtAddress, EFI_ACPI_TABLE_VERSION_ALL);
    if (EFI_ERROR(Status)){
        DEBUG((-1, "TrEEUpdateTpmDeviceASL::DSDT not found\n"));
        if(Status == EFI_NOT_AVAILABLE_YET){
            //set callback
            Status = gBS->CreateEvent( EFI_EVENT_NOTIFY_SIGNAL,
                                   EFI_TPL_CALLBACK, TrEEUpdateTpmDeviceASL, &reg, &Event );

            if(EFI_ERROR(Status)){
                DEBUG((-1, "Unable to create Event..Exit(1)\n"));
                return;
            }
            Status = gBS->RegisterProtocolNotify( &gInternalAcpiSupportGuid, Event, &reg );   
        }
        return;
    }

    DEBUG((-1, "TrEEUpdateTpmDeviceASL::dsdtAddress %x \n", dsdtAddress));
    dsdt = (ACPI_HDR*)dsdtAddress;

    DEBUG((-1, "dsdt->Signature =  %x \n", dsdt->Signature));
    
    Value = 1;
    DEBUG((-1, "TrEEUpdateTpmDeviceASL::Setting  TTDP to %x \n", Value));
    Status=TcgUpdateAslNameObject(dsdt, "TTDP", (UINT64)Value);
    if (EFI_ERROR(Status)){
        DEBUG((-1, "TrEEUpdateTpmDeviceASL::Failure setting ASL TTDP %r \n", Status));
        return;
    }    

    if(isTpm20CrbPresent()){
        Value = 0;
    }else{
        Value = 1;
    }

    DEBUG((-1, "TrEEUpdateTpmDeviceASL::Setting  TTPF to %x \n", Value));
    
    Status=TcgUpdateAslNameObject(dsdt, "TTPF", (UINT64)Value);
    if (EFI_ERROR(Status)){
        DEBUG((-1, "TrEEUpdateTpmDeviceASL::Failure setting ASL value %r \n", Status));
    }
    gBS->CloseEvent(ev);
    return;
}

VOID ReadMORValue( );


EFI_STATUS
EFIAPI TreeDxeEntry(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable )
{
    EFI_STATUS          Status;
    TCG_PLATFORM_SETUP_PROTOCOL     *PolicyInstance;
    EFI_GUID            gPolicyguid = TCG_PLATFORM_SETUP_POLICY_GUID;
    TCG_CONFIGURATION   Config;
    
    DEBUG(( -1," TreeDxeEntry \n"));
    Status = gBS->LocateProtocol (&gPolicyguid,  NULL, &PolicyInstance);
    if (EFI_ERROR (Status)) {
      return Status;
    }
            
    gBS-> CopyMem(&Config, &PolicyInstance->ConfigFlags, sizeof(TCG_CONFIGURATION));
    
    if(Config.DeviceType == 0){
        Config.Tpm20Device = 0;   
        PolicyInstance->UpdateStatusFlags(&Config, TRUE); 
        return EFI_SUCCESS;
    }
    
    if(!PolicyInstance->ConfigFlags.TpmSupport){
        return EFI_SUCCESS;
    }
          
    if(!IsTpm20Device()){
        Config.Tpm20Device = 0;  
        if(Config.DeviceType == 1){
            Config.TcgSupportEnabled = 0;
            Config.TpmHardware = 1;
        }
        PolicyInstance->UpdateStatusFlags(&Config, TRUE); 
        DEBUG(( -1," isTpm20CrbPresent returned false \n"));
        return EFI_UNSUPPORTED;
    }

    Config.Tpm20Device = 1;
    Config.InterfaceSel = TpmSupport->InterfaceType;

    Status = InstallTrEEProtocol(ImageHandle);
    if(EFI_ERROR(Status))return EFI_ABORTED;

     //install binding protocol TrEE binding protocol
    Tpm20DriverBinding.DriverBindingHandle = ImageHandle;
    Tpm20DriverBinding.ImageHandle = ImageHandle;

    Status = gBS->InstallMultipleProtocolInterfaces (
                               &Tpm20DriverBinding.DriverBindingHandle,
                               &gEfiDriverBindingProtocolGuid, &Tpm20DriverBinding,
                               &gEfiComponentNameProtocolGuid, &gComponentName,
                               &gEfiComponentName2ProtocolGuid, &gComponentName2,
                                NULL);

    TrEEUpdateTpmDeviceASL(Event, reg);
    Config.TcgSupportEnabled = 1;
    
    Status = PolicyInstance->UpdateStatusFlags(&Config, TRUE); 
    return Status;
}
