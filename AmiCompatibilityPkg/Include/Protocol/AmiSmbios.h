//**********************************************************************//
//**********************************************************************//
//**                                                                  **//
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **//
//**                                                                  **//
//**                       All Rights Reserved.                       **//
//**                                                                  **//
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **//
//**                                                                  **//
//**                       Phone: (770)-246-8600                      **//
//**                                                                  **//
//**********************************************************************//
//**********************************************************************//

//**********************************************************************//
// $Header: /Alaska/BIN/Modules/SMBIOS/SmbiosProtocol/SMBios.h 25    6/11/12 11:24a Davidd $
//
// $Revision: 25 $
//
// $Date: 6/11/12 11:24a $
//**********************************************************************//
//**********************************************************************//

#ifndef _Smbios_DRIVER_H
#define _Smbios_DRIVER_H

#include <token.h>
#include <Protocol/Smbios.h>

#define AMI_SMBIOS_PROTOCOL_GUID    {0x5e90a50d, 0x6955, 0x4a49, { 0x90, 0x32, 0xda, 0x38, 0x12, 0xf8, 0xe8, 0xe5 }}

extern EFI_GUID gAmiSmbiosProtocolGuid;
extern EFI_GUID gAmiSmbiosBoardProtocolGuid;
extern EFI_GUID gAmiSmbiosStaticDataGuid;
extern EFI_GUID gAmiSmbiosNvramGuid;
extern EFI_GUID gAmiSmbrunGuid;

//**********************************************************************//
//  DMI ERROR Codes
//**********************************************************************//

#define DMI_SUCCESS                 0x00
#define DMI_UNKNOWN_FUNCTION        0x81
#define DMI_FUNCTION_NOT_SUPPORTED  0x82
#define DMI_INVALID_HANDLE          0x83
#define DMI_BAD_PARAMETER           0x84
#define DMI_INVALID_SUBFUNCTION     0x85
#define DMI_NO_CHANGE               0x86
#define DMI_ADD_STRUCTURE_FAILED    0x87
#define DMI_READ_ONLY               0x8D
#define DMI_LOCK_NOT_SUPPORTED      0x90
#define DMI_CURRENTLY_LOCKED        0x91
#define DMI_INVALID_LOCK            0x92

//**********************************************************************//

#pragma pack(1)

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_TABLE_ENTRY_POINT
//
// Description: SMBIOS Structure Table Entry Point
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT8   AnchorString[4];
    UINT8   EntryPointStructureChecksum;
    UINT8   EntryPointLength;
    UINT8   MajorVersion;
    UINT8   MinorVersion;
    UINT16  MaxStructureSize;
    UINT8   EntryPointRevision;
    UINT8   FormattedArea[5];
    UINT8   IntermediateAnchorString[5];
    UINT8   IntermediateChecksum;
    UINT16  TableLength;
    UINT32  TableAddress;
    UINT16  NumberOfSmbiosStructures;
    UINT8   SmbiosBCDRevision;
} SMBIOS_TABLE_ENTRY_POINT;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_STRUCTURE_HEADER
//
// Description: SMBIOS Structure Header - Common for all structures
//              Describing the type, size of the fixed area, and handle
//              of the structure.
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT8   Type;
    UINT8   Length;
    UINT16  Handle;
} SMBIOS_STRUCTURE_HEADER;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_BIOS_INFO
//
// Description: BIOS Information Structure (Type 0)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     BiosVendor;                     // String number
    UINT8                     BiosVersion;                    // String number
    UINT16                    BiosStartingAddrSegment;
    UINT8                     BiosReleaseDate;                // String number
    UINT8                     BiosRomSize;
    UINT32                    BiosChar_1;
    UINT32                    BiosChar_2;
    UINT8                     BiosCharExtByte1;
    UINT8                     BiosCharExtByte2;
    UINT8                     SystemBiosMajorRelease;
    UINT8                     SystemBiosMinorRelease;
    UINT8                     ECFirmwareMajorRelease;
    UINT8                     ECFirmwareMinorRelease;
} SMBIOS_BIOS_INFO;                                           // TYPE 0

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_SYSTEM_INFO
//
// Description: System Information Structure (Type 1)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     Manufacturer;                   // String number
    UINT8                     ProductName;                    // String number
    UINT8                     Version;                        // String number
    UINT8                     SerialNumber;                   // String number
    EFI_GUID                  Uuid;
    UINT8                     WakeupType;
    UINT8                     SkuNumber;
    UINT8                     Family;
} SMBIOS_SYSTEM_INFO;                                         // TYPE 1

#if BASE_BOARD_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_BASE_BOARD_INFO
//
// Description: Base Board (or Module) Information Structure (Type 2)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     Manufacturer;                   // String number
    UINT8                     ProductName;                    // String number
    UINT8                     Version;                        // String number
    UINT8                     SerialNumber;                   // String number
    UINT8                     AssetTag;                       // String number
    UINT8                     FeatureTag;
    UINT8                     Location;                       // String number
    UINT16                    ChassisHandle;
    UINT8                     BoardType;
    UINT8                     NumberOfObjectHandles;
#if NUMBER_OF_OBJECT_HANDLES != 0
    UINT8                     ObjectHandles[NUMBER_OF_OBJECT_HANDLES];
#endif
} SMBIOS_BASE_BOARD_INFO;                                     // TYPE 2
#endif

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_SYSTEM_ENCLOSURE_INFO
//
// Description: System Enclosure or Chassis Information Structure (Type 3)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     Manufacturer;                   // String number
    UINT8                     Type;
    UINT8                     Version;                        // String number
    UINT8                     SerialNumber;                   // String number
    UINT8                     AssetTag;                       // String number
    UINT8                     BootupState;
    UINT8                     PowerSupplyState;
    UINT8                     ThermalState;
    UINT8                     SecurityStatus;
    UINT32                    OemDefined;
    UINT8                     Height;
    UINT8                     NumberOfPowerCord;
    UINT8                     ElementCount;
    UINT8                     ElementRecordLength;
#if ((ELEMENT_COUNT != 0) && (ELEMENT_LEN != 0))
    UINT8                     Elements[ELEMENT_COUNT * ELEMENT_LEN];
#endif
    UINT8                     SkuNumber;
} SMBIOS_SYSTEM_ENCLOSURE_INFO;                               // TYPE 3

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_PROCESSOR_INFO
//
// Description: Processor Information Structure (Type 4)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     SocketDesignation;              // String number
    UINT8                     ProcessotType;
    UINT8                     Family;
    UINT8                     ProcessorManufacturer;          // String number
    UINT32                    ProcessorID_1;
    UINT32                    ProcessorID_2;
    UINT8                     ProcessorVersion;               // String number
    UINT8                     Voltage;
    UINT16                    ExtClockFreq;
    UINT16                    MaxSpeed;
    UINT16                    CurrentSpeed;
    UINT8                     Status;
    UINT8                     Upgrade;
    UINT16                    L1CacheHandle;
    UINT16                    L2CacheHandle;
    UINT16                    L3CacheHandle;
    UINT8                     SerialNumber;                   // String number
    UINT8                     AssetTag;                       // String number
    UINT8                     PartNumber;                     // String number
    UINT8                     CoreCount;                      // Number of cores per processor socket
    UINT8                     CoreEnabled;                    // Number of enabled cores per processor socket
    UINT8                     ThreadCount;                    // Number of threads per processor socket
    UINT16                    ProcessorChar;                  // Defines which functions the processor supports
    UINT16                    Family2;
} SMBIOS_PROCESSOR_INFO;                                      // TYPE 4

#if MEM_CTRL_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_MEM_CONTROLLER_INFO
//
// Description: Memory Controller Information Structure (Type 5)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     ErrDetMethod;
    UINT8                     ECC;
    UINT8                     SupportInterleave;
    UINT8                     CurrentInterleave;
    UINT8                     MaxMemModuleSize;
    UINT16                    SupportSpeed;
    UINT16                    SupportMemTypes;
    UINT8                     MemModuleVoltage;
    UINT8                     NumberMemSlots;
    UINT16                    MemModuleConfigHandle[NUMBER_OF_MEM_MODULE];
    UINT8                     EnabledECC;
} SMBIOS_MEM_CONTROLLER_INFO;                                 // TYPE 5
#endif

#if MEM_MODULE_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_MEM_MODULE_INFO
//
// Description: Memory Module Information Structure (Type 6)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     SocketDesignation;
    UINT8                     BankConnections;
    UINT8                     CurrentSpeed;
    UINT16                    CurrentMemType;
    UINT8                     InstalledSize;
    UINT8                     EnabledSize;
    UINT8                     ErrorStatus;
} SMBIOS_MEM_MODULE_INFO;                                     // TYPE 6
#endif

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_CACHE_INFO
//
// Description: Cache Information Structure (Type 7)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     SocketDesignation;
    UINT16                    CacheConfig;
    UINT16                    MaxCacheSize;
    UINT16                    InstalledSize;
    UINT16                    SupportSRAM;
    UINT16                    CurrentSRAM;
    UINT8                     CacheSpeed;
    UINT8                     ErrorCorrectionType;
    UINT8                     SystemCacheType;
    UINT8                     Associativity;
} SMBIOS_CACHE_INFO;                                          // TYPE 7

#if PORT_CONNECTOR_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_PORT_CONN_INFO
//
// Description: Port Connector Information Structure (Type 8)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     IntRefDesig;
    UINT8                     IntConnType;
    UINT8                     ExtRefDesig;
    UINT8                     ExtConnType;
    UINT8                     PortType;
} SMBIOS_PORT_CONN_INFO;                                      // TYPE 8
#endif

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_SYSTEM_SLOTS_INFO
//
// Description: System Slot Information Structure (Type 9)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     SlotDesig;
    UINT8                     SlotType;
    UINT8                     SlotDataBusWidth;
    UINT8                     CurrentUsage;
    UINT8                     SlotLength;
    UINT16                    SlotID;
    UINT8                     SlotChar_1;
    UINT8                     SlotChar_2;
    UINT16                    SegGroupNumber;
    UINT8                     BusNumber;
    UINT8                     DevFuncNumber;
} SMBIOS_SYSTEM_SLOTS_INFO;                                   // TYPE 9

#if ONBOARD_DEVICE_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SINGLE_DEV_INFO
//
// Description: Single Device Information
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT8                     DeviceType;
    UINT8                     DescStringNum;
} SINGLE_DEV_INFO;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_ONBOARD_DEV_INFO
//
// Description: On Board Devices Information Structure (Type 10)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    SINGLE_DEV_INFO    OnBoardDev;
} SMBIOS_ONBOARD_DEV_INFO;                                    // TYPE 10
#endif

#if OEM_STRING_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_OEM_STRINGS_INFO
//
// Description: OEM Strings Structure (Type 11)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     Count;
} SMBIOS_OEM_STRINGS_INFO;                                    // TYPE 11
#endif

#if SYSTEM_CONFIG_OPTION_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_SYSTEM_CONFIG_INFO
//
// Description: System Configuration Options (Type 12)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     Count;
} SMBIOS_SYSTEM_CONFIG_INFO;                                  // TYPE 12
#endif

#if BIOS_LANGUAGE_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_BIOS_LANG_INFO
//
// Description: BIOS Language Information (Type 13)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     InstallableLang;
    UINT8                     Flags;
    UINT8                     Reserved[15];
    UINT8                     CurrentLang;
} SMBIOS_BIOS_LANG_INFO;                                      // TYPE 13
#endif

#if EVENT_LOG_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  EVENT_LOG_DESCRIPTOR
//
// Description: Event Log Descriptor - Part of the System Event Log (Type 15)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT8                     LogType;
    UINT8                     VariableDataFormatType;
} EVENT_LOG_DESCRIPTOR;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_EVENT_LOG_INFO
//
// Description: System Event Log (Type 15)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT16                    LogAreaLength;
    UINT16                    LogHeaderStartOffset;
    UINT16                    LogDataStartOffset;
    UINT8                     AccessMethod;
    UINT8                     LogStatus;
    UINT32                    LogChangeToken;
    UINT32                    AccesMethodAddr;
    UINT8                     LogHeaderFormat;
    UINT8                     NumbetOfLogTypeDesc;
    UINT8                     LengthOfLogTypeDesc;
    EVENT_LOG_DESCRIPTOR      LogDescriptors[NO_OF_SUPPORTED_EVENTS];
} SMBIOS_EVENT_LOG_INFO;                                      // TYPE 15
#endif

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_PHYSICAL_MEM_ARRAY_INFO
//
// Description: Physical Memory Array (Type 16)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     Location;
    UINT8                     Use;
    UINT8                     MemErrorCorrection;
    UINT32                    MaxCapacity;
    UINT16                    MemErrInfoHandle;
    UINT16                    NumberOfMemDev;
    UINT64                    ExtMaxCapacity;
} SMBIOS_PHYSICAL_MEM_ARRAY_INFO;                             // TYPE 16

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_MEMORY_DEVICE_INFO
//
// Description: Memory Device (Type 17)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT16                    PhysicalMemArrayHandle;
    UINT16                    MemErrorInfoHandle;
    UINT16                    TotalWidth;
    UINT16                    DataWidth;
    UINT16                    Size;
    UINT8                     FormFactor;
    UINT8                     DeviceSet;
    UINT8                     DeviceLocator;
    UINT8                     BankLocator;
    UINT8                     MemoryType;
    UINT16                    TypeDetail;
    UINT16                    Speed;
    UINT8                     Manufacturer;
    UINT8                     SerialNumber;
    UINT8                     AssetTag;
    UINT8                     PartNumber;
    UINT8                     Attributes;
    UINT32                    ExtendedSize;
    UINT16                    ConfMemClkSpeed;
    UINT16                    MinimumVoltage;
    UINT16                    MaximumVoltage;
    UINT16                    ConfiguredVoltage;
} SMBIOS_MEMORY_DEVICE_INFO;                                  // TYPE 17

#if MEMORY_ERROR_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_MEMORY_ERROR_INFO
//
// Description: 32-bit Memory Error Information (Type 18)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     ErrorType;
    UINT8                     ErrorGranularity;
    UINT8                     ErrorOperation;
    UINT32                    VendorSyndrome;
    UINT32                    MemArrayErrorAddress;
    UINT32                    DeviceErrorAddress;
    UINT32                    ErrorResolution;
} SMBIOS_MEMORY_ERROR_INFO;                                   // TYPE 18
#endif

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_MEM_ARRAY_MAP_ADDR_INFO
//
// Description: Memory Array Mapped Address (Type 19)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT32                    StartingAddress;
    UINT32                    EndingAddress;
    UINT16                    MemoryArrayHandle;
    UINT8                     PartitionWidth;
    UINT64                    ExtendedStartAddr;
    UINT64                    ExtendedEndAddr;
} SMBIOS_MEM_ARRAY_MAP_ADDR_INFO;                             // TYPE 19

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_MEM_DEV_MAP_ADDR_INFO
//
// Description: Memory Device Mapped Address (Type 20)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT32                    StartingAddress;
    UINT32                    EndingAddress;
    UINT16                    MemoryDeviceHandle;
    UINT16                    MemoryArrayMapAddrHandle;
    UINT8                     PartitionRowPosition;
    UINT8                     InterleavePosition;
    UINT8                     InterleaveDataDepth;
    UINT64                    ExtendedStartAddr;
    UINT64                    ExtendedEndAddr;
} SMBIOS_MEM_DEV_MAP_ADDR_INFO;                               // TYPE 20

#if BUILTIN_POINTING_DEVICE_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_POINTING_DEV_INFO
//
// Description: Built-in Pointing Device (Type 21)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     Type;
    UINT8                     Interface;
    UINT8                     NumberOfButtons;
} SMBIOS_POINTING_DEV_INFO;                                   // TYPE 21
#endif

#if PORTABLE_BATTERY_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_PORTABLE_BATTERY_INFO
//
// Description: Portable Battery (Type 22)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     Location;
    UINT8                     Manufacturer;
    UINT8                     ManufacturerData;
    UINT8                     SerialNumber;
    UINT8                     DeviceName;
    UINT8                     DeviceChemistry;
    UINT16                    DesignCapacity;
    UINT16                    DesignVoltage;
    UINT8                     SBDSVersion;
    UINT8                     MaxErrorInBatteryData;
    UINT16                    SBDSSerialNumber;
    UINT16                    SBDSManufacturerDate;
    UINT8                     SBDSDeviceChecmistry;
    UINT8                     DesignCapabilityMult;
    UINT32                    OEMSpecific;
} SMBIOS_PORTABLE_BATTERY_INFO;                               // TYPE 22
#endif

#if SYSTEM_RESET_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_SYSTEM_RESET_INFO
//
// Description: System Reset (Type 23)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     Capabilities;
    UINT16                    ResetCount;
    UINT16                    ResetLimit;
    UINT16                    TimerInterval;
    UINT16                    TimeOut;
} SMBIOS_SYSTEM_RESET_INFO;                                   // TYPE 23
#endif

#if HARDWARE_SECURITY_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_HARDWARE_SECURITY_INFO
//
// Description: Hardware Security (Type 24)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     HardwareSecSettings;
} SMBIOS_HARDWARE_SECURITY_INFO;                              // TYPE 24
#endif

#if SYSTEM_POWER_CONTROLS_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_SYSTEM_PWR_CONTROL_INFO
//
// Description: System Power Control (Type 25)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     NextSchedulePwrOnMonth;
    UINT8                     NextSchedulePwrOnDayOfMonth;
    UINT8                     NextSchedulePwrOnHour;
    UINT8                     NextSchedulePwrOnMinute;
    UINT8                     NextSchedulePwrOnSecond;
} SMBIOS_SYSTEM_PWR_CONTROL_INFO;                             // TYPE 25
#endif

#if VOLTAGE_PROBE_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_VOLTAGE_PROBE_INFO
//
// Description: Voltage Probe (Type 26)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     Description;
    UINT8                     LocationAndStatus;
    UINT16                    MaxValue;
    UINT16                    MinValue;
    UINT16                    Resolution;
    UINT16                    Tolerance;
    UINT16                    Accuracy;
    UINT32                    OEMDefine;
    UINT16                    NorminalValue;
} SMBIOS_VOLTAGE_PROBE_INFO;                                  // TYPE 26
#endif

#if COOLING_DEVICE_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_COOLING_DEV_INFO
//
// Description: Cooling Device (Type 27)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT16                    TempProbeHandle;
    UINT8                     DevTypeAndStatus;
    UINT8                     CoolingUnitGroup;
    UINT32                    OEMDefine;
    UINT16                    NorminalSpeed;
    UINT8                     Description;
} SMBIOS_COOLING_DEV_INFO;                                    // TYPE 27
#endif

#if TEMPERATURE_PROBE_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_TEMPERATURE_PROBE_INFO
//
// Description: Temperature Probe (Type 28)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     Description;
    UINT8                     LocationAndStatus;
    UINT16                    MaxValue;
    UINT16                    MinValue;
    UINT16                    Resolution;
    UINT16                    Tolerance;
    UINT16                    Accuracy;
    UINT32                    OEMDefine;
    UINT16                    NorminalValue;
} SMBIOS_TEMPERATURE_PROBE_INFO;                              // TYPE 28
#endif

#if ELECTRICAL_PROBE_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_ELECT_CURRENT_PROBE_INFO
//
// Description: Electrical Current Probe (Type 29)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     Description;
    UINT8                     LocationAndStatus;
    UINT16                    MaxValue;
    UINT16                    MinValue;
    UINT16                    Resolution;
    UINT16                    Tolerance;
    UINT16                    Accuracy;
    UINT32                    OEMDefine;
    UINT16                    NorminalValue;
} SMBIOS_ELECT_CURRENT_PROBE_INFO;                            // TYPE 29
#endif

#if OUT_OF_BAND_REMOTE_ACCESS_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_OUT_OF_BAND_REMOTE_ACCESS_INFO
//
// Description: Out-of-Band Remote Access (Type 30)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     ManufacturerName;
    UINT8                     Connections;
} SMBIOS_OUT_OF_BAND_REMOTE_ACCESS_INFO;                      // TYPE 30
#endif

#if MKF_BIS_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_BIS_ENTRYPOINT_INFO
//
// Description: Boot Integrity Services (BIS) Entry Point (Type 31)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     Checksum;
    UINT8                     Reserved_1;
    UINT16                    Reserved_2;
    UINT32                    BISEntrySt;
    UINT32                    BISEntryTt;
    UINT64                    Reserved_3;
    UINT32                    Reserved_4;
} SMBIOS_BIS_ENTRYPOINT_INFO;                                 // TYPE 31
#endif

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_SYSTEM_BOOT_INFO
//
// Description: System Boot Information (Type 32)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     Reserved[6];
    UINT8                     BootStatus[10];
} SMBIOS_SYSTEM_BOOT_INFO;                                    // TYPE 32

#if SIXTY_FOURBIT_MEMORY_ERROR_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_SIXTYFOUR_MEM_ERROR_INFO
//
// Description: 64-bit Memory Error Information (Type 33)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     ErrorType;
    UINT8                     ErrorGranularity;
    UINT8                     ErrorOperation;
    UINT32                    VendorSyndrome;
    UINT64                    MemoryArrayErrAddr;
    UINT64                    DeviceErrAddr;
    UINT32                    ErrorResolution;
} SMBIOS_SIXTYFOUR_MEM_ERROR_INFO;                            // TYPE 33
#endif

#if MANAGEMENT_DEVICE_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_MANAGEMENT_DEV_INFO
//
// Description: Management Device (Type 34)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     Description;
    UINT8                     Type;
    UINT32                    Address;
    UINT8                     AddressType;
} SMBIOS_MANAGEMENT_DEV_INFO;                                 // TYPE 34
#endif

#if MEMORY_CHANNEL_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  TYPE37_MEMORY_DEVICE
//
// Description: Type 37 Memory Device
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT8                     MemDevLoad;
    UINT16                    MemDevHandle;
} TYPE37_MEMORY_DEVICE;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_MEMORY_CHANNEL_INFO
//
// Description: Memory Channel (Type 37)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     ChannelType;
    UINT8                     MaxChannelLoad;
    UINT8                     MemDeviceCount;
    TYPE37_MEMORY_DEVICE      MemoryDevice[NUMBER_OF_MEMORY_CHANNELS];
} SMBIOS_MEMORY_CHANNEL_INFO;                                 // TYPE 37
#endif

#if IPMI_DEVICE_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_IPMI_DEV_INFO
//
// Description: IPMI Device Information (Type 38)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     InterfaceType;
    UINT8                     IPMISpecRevision;
    UINT8                     I2CSlaveAddr;
    UINT8                     NVStorageDevAddr;
    UINT64                    BaseAddress;
} SMBIOS_IPMI_DEV_INFO;                                       // TYPE 38
#endif

#if SYSTEM_POWER_SUPPLY_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_SYSTEM_PWR_SUPPY_INFO
//
// Description: System Power Supply (Type 39)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     PwrUnitGroup;
    UINT8                     Location;
    UINT8                     DeviceName;
    UINT8                     Manufacturer;
    UINT8                     SerialNumber;
    UINT8                     AssetTagNumber;
    UINT8                     ModelPartNumber;
    UINT8                     RevisionLevel;
    UINT16                    MaxPwrCapacity;
    UINT16                    PwrSupplyChar;
    UINT16                    InputVoltProbeHandle;
    UINT16                    CoolingDevHandle;
    UINT16                    InputCurrentProbeHandle;
} SMBIOS_SYSTEM_PWR_SUPPY_INFO;                               // TYPE 39
#endif

#if ADDITIONAL_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_ADDITIONAL_INFO
//
// Description: System Power Supply (Type 40)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT8                     EntryLength;
    UINT16                    RefHandle;
    UINT8                     RefOffset;
    UINT8                     StringNum;
    union {
        UINT8                 Value8;
        UINT16                Value16;
        UINT32                Value32;
    };
} ADDITIONAL_INFO_ENTRY;

typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     NumAdditionalInfoEntries;
    ADDITIONAL_INFO_ENTRY     AdditionalEntries[ADDITIONAL_INFO_COUNT];
} SMBIOS_ADDITIONAL_INFO;                                     // TYPE 40
#endif

#if ONBOARD_DEVICE_EXTENDED_INFO
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_ONBOARD_DEVICE_EXTENDED_INFO
//
// Description: Onboard Devices Extended Information (Type 41)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    SMBIOS_STRUCTURE_HEADER   StructureType;
    UINT8                     RefDesignation;
    UINT8                     DeviceType;
    UINT8                     DeviceTypeInstance;
    UINT16                    SegGroupNumber;
    UINT8                     BusNumber;
    UINT8                     DevFuncNumber;
} SMBIOS_ONBOARD_DEV_EXT_INFO;                                // TYPE 41
#endif

//**********************************************************************//
// SMBIOS NVRAM DATA
//**********************************************************************//
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_NVRAM_TYPE
//
// Description: DMI Type X data
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT32                    Flag;
    CHAR8                     *StringSet[32];
} SMBIOS_NVRAM_TYPE;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_NVRAM_TYPE0
//
// Description: DMI Type 0 data
// UINT32   Flag;		// Bit0 = Vendor string changed
//						// Bit1 = BIOS version string changed
//						// Bit2 = BIOS Release Date string changed
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT32                    Flag;
    CHAR8                     *StringSet[3];
} SMBIOS_NVRAM_TYPE0;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_NVRAM_TYPE1
//
// Description: DMI Type 1 data
// UINT32   Flag;		// Bit0 = Manufacturer string changed
//						// Bit1 = Product Name string changed
//						// Bit2 = Version string changed
//						// Bit3 = Serial Number string changed
//						// Bit4 = SKU string changed
//						// Bit5 = Family string changed
//						// Bit16 = UUID changed
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT32                    Flag;
    CHAR8                     *StringSet[6];
    EFI_GUID                  Uuid;
} SMBIOS_NVRAM_TYPE1;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_NVRAM_TYPE2
//
// Description: DMI Type 2 data
// UINT32   Flag;		// Bit0 = Manufacturer string changed
//						// Bit1 = Product Name string changed
//						// Bit2 = Version string changed
//						// Bit3 = Serial Number string changed
//						// Bit4 = Asset Tag string changed
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT32                    Flag;
    CHAR8                     *StringSet[6];
    UINT16                    Handle;
} SMBIOS_NVRAM_TYPE2;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_NVRAM_TYPE3
//
// Description: DMI Type 3 data
// UINT32   Flag;		// Bit0 = Manufacturer string changed
//						// Bit1 = Version string changed
//						// Bit2 = Serial Number string changed
//						// Bit3 = Asset Tag string changed
//                      // Bit4 = SKU Number string changed
//						// Bit16 = Chassis Type changed
//						// Bit17 = OEM-defined changed
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT32                    Flag;
    CHAR8                     *StringSet[5];
    UINT8                     Type;
    UINT32                    OemDefined;
    UINT16                    Handle;
} SMBIOS_NVRAM_TYPE3;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_NVRAM_TYPE4
//
// Description: DMI Type 4 data
// UINT32   Flag;		// Bit0 = Serial Number string changed
//						// Bit1 = Asset Tag string changed
//						// Bit2 = Part Number string changed
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT32                    Flag;
    CHAR8                     *StringSet[6];
} SMBIOS_NVRAM_TYPE4;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_NVRAM_TYPE11
//
// Description: DMI Type 11 data
// UINT32   Flag;		// Bit(x) = String #(x) changed
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT32                    Flag;
    CHAR8                     *StringSet[32];
} SMBIOS_NVRAM_TYPE11;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_NVRAM_TYPE12
//
// Description: DMI Type 12 data
// UINT32   Flag;		// Bit(x) = String #(x) changed
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT32                    Flag;
    CHAR8                     *StringSet[32];
} SMBIOS_NVRAM_TYPE12;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_NVRAM_TYPE22
//
// Description: DMI Type 22 data
// UINT8   Flag;		// Bit0 = Location string changed
//						// Bit1 = Manufacturer string changed
//						// Bit2 = Manufacturer Date changed
//						// Bit3 = Serial Number string changed
//						// Bit4 = Device Name string changed
//						// Bit5 = SBDS Version Number string changed
//						// Bit6 = SBDS Device Chemistry string changed
//						// Bit16 = Device Chemistry changed
//						// Bit17 = Design Capacity changed
//						// Bit18 = Design Voltage changed
//						// Bit19 = Maximum Error in Battery Data changed
//						// Bit20 = SBDS Serial Number changed
//						// Bit21 = SBDS Manufacturer Date changed
//						// Bit22 = Design Capacity Multiplier changed
//						// Bit23 = OEM-specific changed
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT32                    Flag;
    CHAR8                     *StringSet[7];
    UINT8                     DeviceChemistry;
    UINT16                    DesignCapacity;
    UINT16                    DesignVoltage;
    UINT8                     MaxErrorInBatteryData;
    UINT16                    SbdsSerialNumber;
    UINT16                    SbdsManufacturerDate;
    UINT8                     DesignCapacityMultiplier;
    UINT32                    OemSpecific;
    UINT16                    Handle;
} SMBIOS_NVRAM_TYPE22;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_NVRAM_TYPE39
//
// Description: DMI Type 39 data
// UINT8   Flag;		// Bit0 = Location string changed
//						// Bit1 = Device Name string changed
//						// Bit2 = Manufacturer string changed
//						// Bit3 = Serial Number string changed
//						// Bit4 = Asset Tag string changed
//						// Bit5 = Model Part Number string changed
//						// Bit6 = Revision Level string changed
//						// Bit16 = Power Unit Group changed
//						// Bit17 = Max Power Capacity changed
//						// Bit18 = Power Supply Characteristics changed
//						// Bit19 = Input Voltage Probe Handle changed
//						// Bit20 = Cooling Device Handle changed
//						// Bit21 = Input Current Probe Handle changed
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT32                    Flag;
    CHAR8                     *StringSet[7];
    UINT8                     PwrUnitGroup;
    UINT16                    MaxPwrCapacity;
    UINT16                    PwrSupplyChar;
    UINT16                    InputVoltageProbeHandle;
    UINT16                    CoolingDevHandle;
    UINT16                    InputCurrentProbeHandle;
    UINT16                    Handle;
} SMBIOS_NVRAM_TYPE39;

typedef struct {
    UINT8                   Type;
	UINT16					Handle;
    UINT8                   Offset;
    UINT8   				Flags;      // Bit0 = Write Once
                        				// Bit1 = Delete Structure
                        				// Bit2 = Add structure
} DMI_VAR;

typedef struct {
    UINT8                   Offset;
    UINT8                   SpecStrNum;
    UINT8                   CurStrValue;
} STRING_TABLE;

typedef enum {
    ADD_STRUCTURE,
    DELETE_STRUCTURE,
} SMBIOS_OPERATION;

#pragma pack()

//**********************************************************************//
// AMI PI Smbios Protocol
//**********************************************************************//

#define SMBIOS_MAJOR_VERSION    02
#define SMBIOS_MINOR_VERSION    07

#pragma pack(1)

typedef struct {
    EFI_SMBIOS_HANDLE       SmbiosHandle;
    EFI_HANDLE              ProducerHandle;
} EFI_PRODUCER_HANDLE;

#pragma pack()

//**********************************************************************//
// AMI PI SMBIOS Function Declarations
//**********************************************************************//

EFI_STATUS
SmbiosPiAddStructure (
    IN CONST EFI_SMBIOS_PROTOCOL    *This,
    IN EFI_HANDLE                   ProducerHandle, OPTIONAL
    IN OUT EFI_SMBIOS_HANDLE        *SmbiosHandle,
    IN EFI_SMBIOS_TABLE_HEADER      *Record
);

EFI_STATUS
SmbiosPiUpdateString (
    IN CONST EFI_SMBIOS_PROTOCOL    *This,
    IN EFI_SMBIOS_HANDLE            *SmbiosHandle,
    IN UINTN                        *StringNumber,
    IN CHAR8                        *String
);

EFI_STATUS
SmbiosPiRemoveStructure (
    IN CONST EFI_SMBIOS_PROTOCOL    *This,
    IN UINTN                        SmbiosHandle
);

EFI_STATUS
SmbiosPiGetNextStructure (
    IN CONST EFI_SMBIOS_PROTOCOL    *This,
    IN OUT EFI_SMBIOS_HANDLE        *SmbiosHandle,
    IN EFI_SMBIOS_TYPE              *Type, OPTIONAL
    OUT EFI_SMBIOS_TABLE_HEADER     **Record,
    OUT EFI_HANDLE                  *ProducerHandle OPTIONAL
);

//**********************************************************************//
// AMI Smbios Protocol
//**********************************************************************//

typedef struct _AMI_SMBIOS_PROTOCOL AMI_SMBIOS_PROTOCOL;

typedef VOID* (EFIAPI *EFI_SMBIOS_GET_TABLE_ENTRY) (
);

typedef VOID* (EFIAPI *EFI_SMBIOS_GET_SCRATCH_BUFFER) (
);

typedef UINT16 (EFIAPI *EFI_SMBIOS_GET_BUFFER_MAX_SIZE) (
);

typedef UINT16 (EFIAPI *EFI_SMBIOS_GET_FREE_HANDLE) (
);

typedef EFI_STATUS (EFIAPI *EFI_SMBIOS_ADD_STRUCTURE) (
    IN UINT8                  *Buffer,
    IN UINT16                 Size
);

typedef EFI_STATUS (EFIAPI *EFI_SMBIOS_ADD_STRUC_HANDLE) (
    IN UINT16                 Handle,
    IN UINT8                  *Buffer,
    IN UINT16                 Size
);

typedef EFI_STATUS (EFIAPI *EFI_SMBIOS_DELETE_STRUCTURE) (
    IN UINT16                 Handle
);

typedef EFI_STATUS (EFIAPI *EFI_SMBIOS_READ_STRUCTURE) (
    IN      UINT16            Handle,
    IN OUT  UINT8             **BufferPtr,
    IN OUT  UINT16            *BufferSize
);

typedef EFI_STATUS (EFIAPI *EFI_SMBIOS_READ_STRUC_TYPE) (
    IN UINT8                  Type,
    IN UINT8                  Instance,
    IN UINT8                  **BufferPtr,
    IN UINT16                 *BufferSize
);

typedef EFI_STATUS (EFIAPI *EFI_SMBIOS_WRITE_STRUCTURE) (
    IN UINT16                 Handle,
    IN UINT8                  *BufferPtr,
    IN UINT16                 BufferSize
);

typedef EFI_STATUS (EFIAPI *EFI_SMBIOS_UPDATE_HEADER) (
);

typedef struct _AMI_SMBIOS_PROTOCOL {
    EFI_SMBIOS_GET_TABLE_ENTRY      SmbiosGetTableEntryPoint;   // SMBIOS Table Entry Point
    EFI_SMBIOS_GET_SCRATCH_BUFFER   SmbiosGetScratchBufferPtr;  // Scratch Buffer of maximum table size
    EFI_SMBIOS_GET_BUFFER_MAX_SIZE  SmbiosGetBufferMaxSize;     // Maximum SMBIOS Table Size
    EFI_SMBIOS_GET_FREE_HANDLE      SmbiosGetFreeHandle;        // Get available free handle
    EFI_SMBIOS_ADD_STRUCTURE        SmbiosAddStructure;         // Add structure
    EFI_SMBIOS_ADD_STRUC_HANDLE     SmbiosAddStrucByHandle;     // Add structure
    EFI_SMBIOS_DELETE_STRUCTURE     SmbiosDeleteStructure;      // Delete structure (by handle)
    EFI_SMBIOS_READ_STRUCTURE       SmbiosReadStructure;        // Read structure. Caller is responsible
                                                                // for deallocating the memory
    EFI_SMBIOS_READ_STRUC_TYPE      SmbiosReadStrucByType;      // Read structure by type. Caller is
                                                                // responsible for deallocating the memory
    EFI_SMBIOS_WRITE_STRUCTURE      SmbiosWriteStructure;       // Write structure
    EFI_SMBIOS_UPDATE_HEADER        SmbiosUpdateHeader;         // Update SMBIOS Table Header
};

//**********************************************************************//
// AMI SMBIOS Functions
//**********************************************************************//

VOID*
GetSmbiosTableEntryPoint(
);

VOID*
GetScratchBufferPtr(
);

UINT16
GetBufferMaxSize(
);

UINT16
GetFreeHandle(
);

EFI_STATUS
AddStructure(
    IN UINT8                  *Buffer,
    IN UINT16                 Size
);

EFI_STATUS
AddStructureByHandle(
    IN UINT16                 Handle,
    IN UINT8                  *Buffer,
    IN UINT16                 Size
);

EFI_STATUS
DeleteStructureByHandle(
    IN UINT16                 Handle
);

EFI_STATUS
ReadStructureByHandle(
    IN      UINT16            Handle,
    IN OUT  UINT8             **BufferPtr,
    IN OUT  UINT16            *BufferSize
);

EFI_STATUS
ReadStructureByType(
    IN      UINT8             Type,
    IN      UINT8             Instance,
    IN OUT  UINT8             **BufferPtr,
    IN OUT  UINT16            *BufferSize
);

EFI_STATUS
WriteStructureByHandle(
    IN UINT16                 Handle,
    IN UINT8                  *BufferPtr,
    IN UINT16                 BufferSize
);

VOID
ClearStringNumber (
    IN STRING_TABLE     *StringTablePtr,
    IN UINT8            Index
);

EFI_STATUS
ReplaceString(
    IN UINT8    *DestStructPtr,
    IN UINT8    StringNum,
    IN UINT8    *StringData
);

EFI_STATUS
UpdateSmbiosTableHeader(
);

UINT16
GetStructureLength(
    IN UINT8    *BufferStart
);

BOOLEAN
FindStructureHandle(
    IN OUT UINT8    **Buffer,
    IN     UINT16   Handle
);

#endif                              // _Smbios_DRIVER_H

//**********************************************************************//
//**********************************************************************//
//**                                                                  **//
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **//
//**                                                                  **//
//**                       All Rights Reserved.                       **//
//**                                                                  **//
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **//
//**                                                                  **//
//**                       Phone: (770)-246-8600                      **//
//**                                                                  **//
//**********************************************************************//
//**********************************************************************//
