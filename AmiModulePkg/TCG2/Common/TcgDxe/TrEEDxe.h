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
#ifndef TrEE_DXE_H
#define TrEE_DXE_H

#include <Uefi.h>
#include <Token.h>
#include <AmiTcg\Tpm20.h>
#include <AmiTcg\TrEEProtocol.h>

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

#pragma pack(pop)


typedef struct _AMI_INTERNAL_HLXE_PROTOCOL  AMI_INTERNAL_HLXE_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI * INTERNAL_HASH_LOG_EXTEND_EVENT) (
  IN  EFI_TREE_PROTOCOL        *This,
  IN  UINT8                    *DataToHash,
  IN  UINT64                   Flags,
  IN  UINTN                    DataSize,
  IN  OUT  TCG_PCR_EVENT2_HDR  *NewEventHdr,
  IN  UINT8                    *NewEventData
);

struct _AMI_INTERNAL_HLXE_PROTOCOL {
    INTERNAL_HASH_LOG_EXTEND_EVENT  AmiHashLogExtend2;
};

#endif

