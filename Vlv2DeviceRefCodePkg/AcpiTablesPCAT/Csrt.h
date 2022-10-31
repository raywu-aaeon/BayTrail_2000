//#define EFI_ACPI_5_0_CORE_SYSTEM_RESOURCE_TABLE_SIGNATURE  SIGNATURE_32('C', 'S', 'R', 'T')
//
// CSRT Definitions
//
//#define EFI_ACPI_CSRT_SIGNATURE SIGNATURE_32 ('C', 'S', 'R', 'T') // "CSRT"

#define EFI_ACPI_CSRT_TABLE_REVISION 0x00000000

#define NUMBER_RESOURCE_GROUP_INFO 1 //2

#define MAX_NO_CHANNEL1_SUPPORTED 7
#define MAX_NO_CHANNEL2_SUPPORTED 9

#define NAMESPACE_STRING_MAX_LENGTH 16

//
// Ensure proper structure formats
//
#pragma pack (1)


typedef struct _SHARED_INFO_SECTION {
  UINT16 MajVersion;
  UINT16 MinVersion;
  UINT32 MMIOLowPart;
  UINT32 MMIOHighPart;
  UINT32 IntGSI;
  UINT8 IntPol;
  UINT8 IntMode;
  UINT8 NoOfCh;
  UINT8 DMAAddressWidth;
  UINT16 BaseReqLine;
  UINT16 NoOfHandSig;
  UINT32 MaxBlockTransferSize;
} SHARED_INFO_SECTION;

typedef struct _RESOURCE_GROUP_HEADER {
  UINT32 Length;
  UINT32 VendorId;
  UINT32 SubVendorId;
  UINT16 DeviceId;
  UINT16 SubDeviceId;
  UINT16 Revision;
  UINT16 Reserved;
  UINT32 SharedInfoLength;
  SHARED_INFO_SECTION SharedInfoSection;
} RESOURCE_GROUP_HEADER;

typedef struct _RESOURCE_DESCRIPTOR {
  UINT32 Length;
  UINT16 ResourceType;
  UINT16 ResourceSubType;
  UINT32 UUID;
} RESOURCE_DESCRIPTOR;

typedef struct {
  RESOURCE_GROUP_HEADER          ResourceGroupHeaderInfo;
  RESOURCE_DESCRIPTOR            ResourceDescriptorInfo[MAX_NO_CHANNEL1_SUPPORTED];
} RESOURCE_GROUP_INFO1;

typedef struct {
  RESOURCE_GROUP_HEADER          ResourceGroupHeaderInfo;
  RESOURCE_DESCRIPTOR            ResourceDescriptorInfo[MAX_NO_CHANNEL2_SUPPORTED];
} RESOURCE_GROUP_INFO2;

//
// DBGP structure
//
typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER            Header;
  RESOURCE_GROUP_INFO1              ResourceGroupsInfo1;
  RESOURCE_GROUP_INFO2              ResourceGroupsInfo2;
} EFI_ACPI_CSRT_TABLE;

#pragma pack ()