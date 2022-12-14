/*++

Copyright (c)  1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PlatformDxe.h

Abstract:

  Header file for Platform Initialization Driver.



++*/

#ifndef _PLATFORM_DRIVER_H
#define _PLATFORM_DRIVER_H

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/PcdLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/HobLib.h>
//#include <Library/EfiRegTableLib.h>

#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/IsaAcpi.h>
#include <Framework/FrameworkInternalFormRepresentation.h>
#include <Protocol/FrameworkHii.h>
#include <Protocol/FrameworkFormCallback.h>
#include <Protocol/CpuIo.h>
#include <Protocol/BootScriptSave.h>
#include <Framework/BootScript.h>
#include <Guid/BoardFeatures.h>
#include <Guid/DataHubRecords.h>
#include <Protocol/DataHub.h>
#include <Protocol/PciIo.h>
#include <IndustryStandard/Pci22.h>
#include <Setup.h>
#include <Guid/PlatformInfo.h>
#include "Configuration.h"
#include "PchAccess.h"
#include "VlvAccess.h"
#include "CMOSMap.h"
#include "PlatformBaseAddresses.h"
#include "SetupMode.h"
#include "PlatformBootMode.h"
#include <Library/SbPolicy.h>
#include <Library/NbPolicy.h>

#define PCAT_RTC_ADDRESS_REGISTER   0x74
#define PCAT_RTC_DATA_REGISTER      0x75

#define RTC_ADDRESS_SECOND_ALARM    0x01
#define RTC_ADDRESS_MINUTE_ALARM    0x03
#define RTC_ADDRESS_HOUR_ALARM      0x05

#define RTC_ADDRESS_REGISTER_A      0x0A
#define RTC_ADDRESS_REGISTER_B      0x0B
#define RTC_ADDRESS_REGISTER_C      0x0C
#define RTC_ADDRESS_REGISTER_D      0x0D

#define B_RTC_ALARM_INT_ENABLE      0x20
#define B_RTC_ALARM_INT_STATUS      0x20

#define B_RTC_DATE_ALARM_MASK       0x3F

// Default CPU Alternate Duty Cycle (255=100%, 0=0%)
#define DEF_CPU_ALT_DUTY_CYCLE 0xFF

#define MAX_ONBOARD_SATA_DEVICE 2

#define DXE_DEVICE_ENABLED  1
#define DXE_DEVICE_DISABLED 0

#define AZALIA_MAX_LOOP_TIME  0x10000

//
// Platform driver GUID
//
#define EFI_PLATFORM_DRIVER_GUID \
  { 0x056E7324, 0xA718, 0x465b, 0x9A, 0x84, 0x22, 0x8F, 0x06, 0x64, 0x2B, 0x4F }

//#define PASSWORD_MAX_SIZE               20
#define PLATFORM_NORMAL_MODE          0x01
#define PLATFORM_SAFE_MODE            0x02
#define PLATFORM_RECOVERY_MODE        0x04
#define PLATFORM_MANUFACTURING_MODE   0x08
#define PLATFORM_BACK_TO_BIOS_MODE    0x10

#define EFI_OEM_SPECIFIC      0x8000
#define EFI_CU_PLATFORM_DXE_INIT                     (EFI_OEM_SPECIFIC | 0x00000011)
#define EFI_CU_PLATFORM_DXE_STEP1                    (EFI_OEM_SPECIFIC | 0x00000012)
#define EFI_CU_PLATFORM_DXE_STEP2                    (EFI_OEM_SPECIFIC | 0x00000013)
#define EFI_CU_PLATFORM_DXE_STEP3                    (EFI_OEM_SPECIFIC | 0x00000014)
#define EFI_CU_PLATFORM_DXE_STEP4                    (EFI_OEM_SPECIFIC | 0x00000015)
#define EFI_CU_PLATFORM_DXE_INIT_DONE                (EFI_OEM_SPECIFIC | 0x00000016)


#define EFI_SECTION_STRING                  0x1C
#define EFI_FORWARD_DECLARATION(x) typedef struct _##x x
#define PREFIX_BLANK                        0x04

#pragma pack(1)

typedef UINT64 EFI_BOARD_FEATURES;
//BUGBUG: should remove these EDK hii definition once Hii transtion is done
typedef UINT16  STRING_REF;
typedef UINT16  EFI_FORM_LABEL;

typedef enum {
    EfiUserPassword,
    EfiAdminPassword
} EFI_PASSWORD_TYPE;

typedef struct {
    CHAR16            TempPassword[PASSWORD_MAX_SIZE];
    CHAR16            EncodedPassword[PASSWORD_MAX_SIZE];
    VOID              *PasswordLocation;
    EFI_PASSWORD_TYPE PasswordType;
} EFI_PASSWORD_DATA;

typedef struct {
    CHAR8 AaNumber[7];
    UINT8 BoardId;
    EFI_BOARD_FEATURES Features;
    UINT16 SubsystemDeviceId;
    UINT16 AudioSubsystemDeviceId;
    UINT64 AcpiOemTableId;
} BOARD_ID_DECODE;

typedef
EFI_STATUS
(EFIAPI *EFI_FORM_ROUTINE)(
    SETUP_DATA *SetupBuffer
);

typedef struct {
    UINT16 DeviceNumber;
    UINT16 FunctionNumber;
} PCI_DEVICE_FUNC_INFO;

typedef struct {
    CHAR16 PortNumber[4];
    STRING_REF SataDeviceInfoStringId;
} SATA_DEVICE_STRING_INFO;

typedef struct {
    UINT16  Signature;
    UINT8   Size;
    UINT32  EntryPoint;
    UINT8   Reserve[17];
    UINT16  PciDataOff;
    UINT16  ExpansionOff;
} PNP_OPTION_ROM_HEADER;

typedef struct {
    UINT32  Signature;
    UINT8   Revision;
    UINT8   Length;
    UINT16  NextHeader;
    UINT8   Reserve;
    UINT8   CheckSum;
    UINT32  DeviceId;
    UINT16  ManufactureStrOff;
    UINT16  ProductStrOff;
} PNP_EXPANSION_HEADER;

typedef struct {
    BOOLEAN                         Enable;
    UINT8                           VerbTableNum;
    UINT16                          CodecSSID;
    EFI_PHYSICAL_ADDRESS            HDABar;
    EFI_PHYSICAL_ADDRESS            UpperHDABar;
    UINT8                           SDIPresent;
    BOOLEAN                         Pme;
    BOOLEAN                         LegacyFrontPanelAudio;
    BOOLEAN                         HighDefinitionFrontPanelAudio;
} EFI_AZALIA_S3;

//following structs are from R8. Remove them once R8->R9 transition is done

typedef struct {
    CHAR16      *OptionString;  // Passed in string to generate a token for in a truly dynamic form creation
    STRING_REF  StringToken;    // This is used when creating a single op-code without generating a StringToken (have one already)
    UINT16      Value;
    UINT8       Flags;
    UINT16      Key;
} IFR_OPTION;

typedef struct {
    UINT32  RegEax;
    UINT32  RegEbx;
    UINT32  RegEcx;
    UINT32  RegEdx;
} EFI_CPUID_REGISTER;

typedef enum {
    EnumCpuUarchUnknown = 0,
    EnumNehalemUarch,
} EFI_CPU_UARCH;

typedef enum {
    EnumCpuPlatformUnknown = 0,
    EnumDesktop,
    EnumMobile,
    EnumServer,
    EnumNetTop
} EFI_CPU_PLATFORM;

typedef enum {
    EnumCpuTypeUnknown = 0,
    EnumAtom,
    EnumNehalemEx,
    EnumBloomfield,
    EnumGainestown,
    EnumHavendale,
    EnumLynnfield,
    EnumAuburndale,
    EnumClarksfield,
    EnumPineview,
    EnumCedarview,
    EnumClarkdale // Havendale 32nm
} EFI_CPU_TYPE;

typedef enum {
    EnumCpuFamilyUnknown = 0,
    EnumFamilyField,
    EnumFamilyDale
} EFI_CPU_FAMILY;

#pragma pack()

//
// Prototypes
//
EFI_STATUS
EfiMain(
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
);

EFI_STATUS
ProcessEventLog(
);

EFI_STATUS
FindDataRecords(
);

EFI_STATUS
ProcessPasswords(
);

VOID
MemorySetup(
);


UINTN
EfiValueToString(
    IN  OUT CHAR16  *Buffer,
    IN  INT64       Value,
    IN  UINTN       Flags,
    IN  UINTN       Width
);

VOID
ReadyToBootFunction(
    EFI_EVENT  Event,
    VOID       *Context
);

VOID
InstallHiiDataAndGetSettings(
    IN EFI_HII_STRING_PACK            *StringPack,
    ... // 0 or more of => IN EFI_HII_IFR_PACK *IfrPack,
    // Terminate list with NULL
);

EFI_STATUS
ReadOrInitSetupVariable(
    IN UINTN         RequiredVariableSize,
    IN UINTN         RequiredPasswordSize,
    IN VOID          *DefaultData,
    IN VOID          *MfgDefaultData,
    OUT VOID         *SetupVariableData,
    OUT VOID         *SystemPassword
);

VOID
EfiLogicalOrMem(
    IN VOID   *Destination,
    IN VOID   *Source,
    IN UINTN  Length
);

EFI_STATUS
GetStringFromToken(
    IN      EFI_GUID                  *ProducerGuid,
    IN      STRING_REF                Token,
    OUT     CHAR16                    **String
);

UINT32
ConvertBase2ToRaw(
    IN  EFI_EXP_BASE2_DATA             *Data);

UINT32
ConvertBase10ToRaw(
    IN  EFI_EXP_BASE10_DATA             *Data);

CHAR16 *
GetStringById(
    IN  STRING_REF   Id,
    EFI_HII_HANDLE   StringPackHandle
);

VOID
EFIAPI
SetupDataFilter(
    IN EFI_EVENT    Event,
    IN VOID*        Context
);

VOID
EFIAPI
IdeDataFilter(
    IN EFI_EVENT    Event,
    IN VOID*        Context
);

VOID
EFIAPI
UpdateAhciRaidDiskInfo(
    IN EFI_EVENT    Event,
    IN VOID*        Context
);

VOID
EFIAPI
EventLogFilter(
    IN EFI_EVENT    Event,
    IN VOID*        Context
);

VOID
SwapEntries(
    IN  CHAR8 *Data
);

VOID
AsciiToUnicode(
    IN    CHAR8     *AsciiString,
    IN    CHAR16    *UnicodeString
);

UINT16
ConfigModeStateGet();

VOID
SetSkus();

VOID
CPUSetupItems();

EFI_STATUS
SecurityDriverCallback(
    IN EFI_FORM_CALLBACK_PROTOCOL       *This,
    IN UINT16                           KeyValue,
    IN EFI_IFR_DATA_ARRAY               *Data,
    OUT EFI_HII_CALLBACK_PACKET         **Packet
);

VOID
SetPasswordState(
);

VOID
EncodePassword(
    IN  CHAR16                      *Password
);

VOID
EFIAPI
PciBusEvent(
    IN EFI_EVENT    Event,
    IN VOID*        Context
);
#if 0
EFI_STATUS
PciBusDriverHook(
);
#endif
VOID
AsfInitialize(
);

VOID
InitializeAsf(
);

UINT8
ReadCmosBank1Byte(
    IN  EFI_CPU_IO_PROTOCOL             *CpuIo,
    IN  UINT8                           Index
);

VOID
WriteCmosBank1Byte(
    IN  EFI_CPU_IO_PROTOCOL             *CpuIo,
    IN  UINT8                           Index,
    IN  UINT8                           Data
);

EFI_STATUS
InstallBootCallbackRoutine(
);

EFI_STATUS
InstallConfigurationCallbackRoutine(
);

EFI_STATUS
InstallPerformanceCallbackRoutine(
);

EFI_STATUS
InstallSecurityCallbackRoutine(
);

EFI_STATUS
InstallMainCallbackRoutine(
);

EFI_STATUS
MemoryConfigurationUpdate(
    UINT16                *Key,
    EFI_FORM_LABEL        *Label,
    UINT16                *OpcodeCount,
    UINT8                 **OpcodeData,
    EFI_FORM_ROUTINE      *Routine
);

EFI_STATUS
MemoryConfigurationCallbackRoutine(
    SETUP_DATA  *SetupBuffer
);

EFI_STATUS
MemoryConfigurationCalculateSpeed(
    SETUP_DATA  *SetupBuffer
);

VOID
UpdateMemoryString(
    IN  STRING_REF                  TokenToUpdate,
    IN  CHAR16                      *NewString
);

VOID
InitFeaturePolicy(
    IN EFI_PLATFORM_INFO_HOB      *PlatformInfo
);

VOID
InitializeSetupVarHide(
);

VOID
PreparePCIePCISlotInformation(
    VOID
);


EFI_STATUS
BootConfigurationUpdate(
    IN  OUT SETUP_DATA  *SystemConfiguration
);

EFI_STATUS
InitializeBootConfiguration(
    VOID
);

UINT16
GetStringSize(
    IN      CHAR16 *ThisString
);

UINT16
GetDriveCount(
    IN      STRING_REF *BootMap
);

CHAR16 *
GetBootString(
    IN      STRING_REF    Id,
    OUT UINTN        *Length
);

EFI_STATUS
BootCfgCreateTwoOptionOneOf(
    IN      UINT16                QuestionId,
    IN      EFI_FORM_LABEL        Label,
    IN      STRING_REF            OptionPrompt,
    IN      STRING_REF            OptionHelp,
    IN      STRING_REF            OptionOneString,
    IN      STRING_REF            OptionTwoString,
    IN      UINT8                 OptionOneFlags,
    IN      UINT8                 OptionTwoFlags,
    IN      UINT16                KeyValueOne,
    IN      UINT16                KeyValueTwo
);

EFI_STATUS
ReplaceOpcodeWithText(
    IN      STRING_REF            OptionPrompt,
    IN      STRING_REF            OptionHelp,
    IN      STRING_REF            OptionOneString,
    IN      EFI_FORM_LABEL        Label
);

EFI_STATUS
CreateDriveBootOrderOpcode(
    IN      VOID                 *Data,
    IN      STRING_REF           *BootMap,
    IN      EFI_FORM_LABEL        Label,
    IN      UINT16                QuestionId,
    IN      STRING_REF            OptionOneString,
    IN      STRING_REF            OptionTwoString
);

VOID
SetHyperBootCfgFlags(
    IN  OUT SETUP_DATA *SystemConfiguration
);

VOID
GetHyperBootCfgFlags(
    IN  OUT SETUP_DATA *SystemConfiguration
);

VOID
PrepareBootCfgForHyperBoot(
    IN  OUT SETUP_DATA *SystemConfiguration
);

BOOLEAN
BootCfgChanged(
    IN      SETUP_DATA *SystemConfiguration
);

EFI_STATUS
InsertOpcodeAtIndex(
    IN      SETUP_DATA *SystemConfiguration,
    IN  OUT IFR_OPTION           *OptionList,
    IN      IFR_OPTION            IfrOption,
    IN      UINT16                OptionCount
);

VOID
ConfigureBootOrderStrings(
    IN      SETUP_DATA *SystemConfiguration
);

VOID
InitializeAllBootStrings(
    VOID
);

VOID
SaveUsbCfgSettings(
    IN  OUT SETUP_DATA *SystemConfiguration
);

VOID
RestoreUsbCfgSettings(
    IN  OUT SETUP_DATA *SystemConfiguration
);

EFI_STATUS
UpdateBootDevicePriority(
    IN  OUT SETUP_DATA *SystemConfiguration
);

EFI_STATUS
DisableHyperBoot(
    IN  OUT SETUP_DATA *SystemConfiguration
);

BOOLEAN
CheckForUserPassword(
    VOID
);

EFI_STATUS
EFIAPI
HyperBootPasswordCallback(
    IN  OUT VOID*  Data
);

EFI_STATUS
EFIAPI
HyperBootF9Callback(
    IN VOID*  Data
);

EFI_STATUS
InstallHiiEvents(
    VOID
);

EFI_STATUS
PciBusDriverHook();

// Global externs
//
extern UINT8 MaintenanceBin[];
extern UINT8 MainBin[];
extern UINT8 ConfigurationBin[];
extern UINT8 MemoryConfigurationBin[];
extern UINT8 PerformanceBin[];
extern UINT8 SecurityBin[];
extern UINT8 BootBin[];
extern UINT8 PowerBin[];
extern UINT8 SystemSetupBin[];

extern VOID                 *mDxePlatformStringPack;
extern EFI_HII_PROTOCOL     *mHii;
extern SETUP_DATA mSystemConfiguration;
extern FRAMEWORK_EFI_HII_HANDLE       mMaintenanceHiiHandle;
extern FRAMEWORK_EFI_HII_HANDLE       mMainHiiHandle;
extern FRAMEWORK_EFI_HII_HANDLE       mConfigurationHiiHandle;
extern FRAMEWORK_EFI_HII_HANDLE       mPerformanceHiiHandle;
extern FRAMEWORK_EFI_HII_HANDLE       mPowerHiiHandle;
extern FRAMEWORK_EFI_HII_HANDLE       mBootHiiHandle;
extern FRAMEWORK_EFI_HII_HANDLE       mSecurityHiiHandle;

//extern SYSTEM_PASSWORDS     mSystemPassword;
//extern EFI_PASSWORD_DATA    mAdminPassword;
//extern EFI_PASSWORD_DATA    mUserPassword;

extern EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *mPciRootBridgeIo;

//extern EFI_REG_TABLE mSubsystemIdRegs[];
extern UINT32 mSubsystemVidDid;
extern UINT32 mSubsystemAudioVidDid;

extern UINT8 mBoardIdIndex;
extern BOOLEAN mFoundAANum;
extern EFI_BOARD_FEATURES mBoardFeatures;
extern UINT16 mSubsystemDeviceId;
extern UINT16 mSubsystemAudioDeviceId;
extern BOARD_ID_DECODE mBoardIdDecodeTable[];
extern UINTN mBoardIdDecodeTableSize;

extern UINT8 mSmbusRsvdAddresses[];
extern UINT8 mNumberSmbusAddress;
extern BOOLEAN mMfgMode;
extern UINT32 mPlatformBootMode;
extern CHAR8 BoardAaNumber[];

extern  EFI_GUID gEfiGlobalNvsAreaProtocolGuid;
#endif
