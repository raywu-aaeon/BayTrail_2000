#ifndef TPM_2_0_PEI_H
#define TPM_2_0_PEI_H

#include <AmiTcg\Tpm20.h>
#include <PiPei.h>

#define AMI_PEI_TREE_SERVICE_PPI_GUID \
{0x86f5680a, 0x155c, 0x4bc8, 0xac, 0x77, 0x57, 0x38, 0x48, 0xe2,\
 0xad, 0x3d}

#pragma pack(push, 1)

typedef struct {
    BYTE  sha1[SHA1_DIGEST_SIZE];  
    BYTE  sha256[SHA256_DIGEST_SIZE];  
} TPM2_HALG;

typedef struct {
    UINT32     count;
    TPM2_HALG   digests;
} TPM2_DIGEST_VALUES;

typedef struct{
   TCG_PCRINDEX         PCRIndex;
   TCG_EVENTTYPE        EventType;
   TPM2_DIGEST_VALUES   Digests;
   UINT32               EventSize; // UINT32 aligned
} TCG_PCR_EVENT2_HDR; 

typedef struct{
   TCG_PCR_EVENT2_HDR   hdr;
   UINT8                Event[1];
} TCG_PCR_EVENT2; 


typedef struct _TPM2_Startup{
    TPMI_ST_COMMAND_TAG tag;
    UINT32              CommandSize;
    TPM_CC              CommandCode;
    TPM_SU              StartupType;
}TPM2_Startup_Cmd;

typedef struct _TPM2_SelfTest{
    TPMI_ST_COMMAND_TAG tag;
    UINT32              CommandSize;
    TPM_CC              CommandCode;
    TPMI_YES_NO         SelfTestType;
}TPM2_SelfTest;

typedef struct _TPM2_Startup_Resp{
    TPM_ST              tag;
    UINT32              ResponsSize;
    TPM_RC              ResponseCode;
}TPM2_Common_RespHdr;

#pragma pack(pop)


typedef struct _EFI_TREE_PPI EFI_TREE_PPI;

EFI_STATUS Tpm20PeiSendStartup(IN CONST EFI_PEI_SERVICES **PeiServices,
                               EFI_TREE_PPI *TrEEPeiPpi,
                               IN EFI_BOOT_MODE BootMode);//EIP226550

typedef EFI_STATUS (TPM20_MEASURE_CRTM_VERSION_PEI_FUNC_PTR)(
    IN CONST EFI_PEI_SERVICES **PeiServices, 
    IN EFI_TREE_PPI *TrEEppi
);

extern TPM20_MEASURE_CRTM_VERSION_PEI_FUNC_PTR        *Tpm20MeasureCRTMVersionFuncPtr;

typedef struct _TREE_VERSION {
    UINT8 Major;
    UINT8 Minor;
} TREE_VERSION;

typedef UINT32 TREE_EVENT_LOG_BITMAP;
typedef UINT32 TREE_EVENT_LOG_FORMAT;

#define TREE_EVENT_LOG_FORMAT_TCG_1_2 0x00000001

#define TREE_STRUCTURE_VERSION_MAJOR  (1)
#define TREE_STRUCTURE_VERSION_MINOR  (0)

#define TREE_PROTOCOL_VERSION_MAJOR (1)
#define TREE_PROTOCOL_VERSION_MINOR (0)

typedef struct _TREE_BOOT_SERVICE_CAPABILITY {
    UINT8                   Size;
    TREE_VERSION            StructureVersion;
    TREE_VERSION            ProtocolVersion;
    UINT32                  HashAlgorithmBitmap;
    TREE_EVENT_LOG_BITMAP   SupportedEventLogs;
    BOOLEAN                 TrEEPresentFlag;
    UINT16                  MaxCommandSize;
    UINT16                  MaxResponseSize;
    UINT32                  ManufacturerID;
} TREE_BOOT_SERVICE_CAPABILITY;

#define TREE_BOOT_HASH_ALG_SHA1   0x00000001
#define TREE_BOOT_HASH_ALG_SHA256 0x00000002
#define TREE_BOOT_HASH_ALG_SHA384 0x00000004
#define TREE_BOOT_HASH_ALG_SHA512 0x00000008


typedef UINT32 TrEE_PCRINDEX;
typedef UINT32 TrEE_EVENTTYPE;
typedef UINT32 TREE_EVENTLOGTYPE;

#pragma pack(push, 1)

typedef struct _TrEE_EVENT_HEADER {
    UINT32          HeaderSize;
    UINT16          HeaderVersion;
    TrEE_PCRINDEX   PCRIndex;
    TrEE_EVENTTYPE  EventType;
} TrEE_EVENT_HEADER;

typedef struct _TrEE_EVENT {
    UINT32            Size;
    TrEE_EVENT_HEADER Header;
    UINT8             Event[1];
} TrEE_EVENT;

#pragma pack(pop)

#define SIZE_OF_TrEE_EVENT OFFSET_OF (TrEE_EVENT, Event)

typedef
EFI_STATUS
(EFIAPI *EFI_TREE_GET_CAPABILITY) (
    IN EFI_PEI_SERVICES        **PeiServices,  
    IN EFI_TREE_PPI *This,
    IN OUT TREE_BOOT_SERVICE_CAPABILITY *ProtocolCapability
);

typedef
EFI_STATUS
(EFIAPI *EFI_TREE_GET_EVENT_LOG) (
    IN EFI_TREE_PPI *This,
    IN TREE_EVENTLOGTYPE EventLogFormat,
    OUT EFI_PHYSICAL_ADDRESS *EventLogLocation,
    OUT EFI_PHYSICAL_ADDRESS *EventLogLastEntry,
    OUT BOOLEAN *EventLogTruncated
);

typedef
EFI_STATUS
(EFIAPI * EFI_TREE_HASH_LOG_EXTEND_EVENT) (
    IN CONST EFI_PEI_SERVICES **PeiServices,
    IN EFI_TREE_PPI *This,
    IN UINT64 Flags,
    IN EFI_PHYSICAL_ADDRESS DataToHash,
    IN UINT64 DataToHashLen,
    IN TrEE_EVENT *Event
);

typedef
EFI_STATUS
(EFIAPI *EFI_TREE_SUBMIT_COMMAND) (
    IN EFI_TREE_PPI *This,
    IN UINT32 InputParameterBlockSize,
    IN UINT8 *InputParameterBlock,
    IN UINT32 OutputParameterBlockSize,
    IN UINT8 *OutputParameterBlock
);

struct _EFI_TREE_PPI {
    EFI_TREE_GET_CAPABILITY         GetCapability;
    EFI_TREE_GET_EVENT_LOG          GetEventLog;
    EFI_TREE_HASH_LOG_EXTEND_EVENT  HashLogExtendEvent;
    EFI_TREE_SUBMIT_COMMAND         SubmitCommand;
};
#endif
