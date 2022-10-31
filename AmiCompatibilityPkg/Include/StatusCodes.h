//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2005, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             6145-F Northbelt Pkwy, Norcross, GA 30071            **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/BIN/Core/Include/StatusCodes.h 11    5/13/11 6:26p Oleksiyy $
//
// $Revision: 11 $
//
// $Date: 5/13/11 6:26p $
//*****************************************************************************
//<AMI_FHDR_START>
//
// Name:	  StatusCodes.h
//
// Description:  Status Code Definitions, according to Intel Platform
//		Innovation Framework for EFI Status Codes Specification Revision 0.90
//
// 		The code where logically separated into file is parts for a sesnible
//		grouping of the values
//
//  Part:    Contents:
//    1           General Status Code Definitions --> moved to EFI.h
//    2           Class definitions
//    3           Computing Unit Subclasses, Progress and Error Codes
//    4           Peripheral Subclasses, Progress and Error Codes.
//    5           IO Bus Subclasses, Progress and Error Codes.
//    6           Software Subclasses, Progress and Error Codes.
//    7           Debug Codes
//
//<AMI_FHDR_END>
//*****************************************************************************
#ifndef _EFI_STATUS_CODE_H_
#define _EFI_STATUS_CODE_H_
#include <Efi.h>
#if PI_SPECIFICATION_VERSION>=0x0001000A
#include <UefiHii.h>
#include <DataHubSubClass.h>
#endif

// Include from MdePkg
#include <Guid/StatusCodeDataTypeId.h>
#include <Pi/PiStatusCode.h>
// IntelFrameworkModulePkg
#include <Guid/DataHubStatusCodeRecord.h>
// IntelFrameworkPkg
#include <Framework/StatusCode.h>

// Guid/StatusCodeDataTypeId.h
/*
#define EFI_STATUS_CODE_DATA_TYPE_STRING_GUID \
	{0x92D11080,0x496F,0x4D95,0xBE,0x7E,0x03,0x74,0x88,0x38,0x2B,0x0A }

#define EFI_STATUS_CODE_SPECIFIC_DATA_GUID \
	{0x335984bd,0xe805,0x409a,0xb8,0xf8,0xd2,0x7e,0xce,0x5f,0xf7,0xa6}
*/

///////////////////////////////////////////////////////////////////////////////
// Section 1
///////////////////////////////////////////////////////////////////////////////
// Moved to EFI.h 
// See EFI.h for General status code definitions

// Guid/DataHubStatusCodeRecord.h
/*
// Data Hub Status Code class record definition
typedef struct {
  EFI_STATUS_CODE_TYPE  CodeType;
  EFI_STATUS_CODE_VALUE Value;
  UINT32                Instance;
  EFI_GUID              CallerId;
  EFI_STATUS_CODE_DATA  Data;
} DATA_HUB_STATUS_CODE_DATA_RECORD;
*/

///////////////////////////////////////////////////////////////////////////////
// Section 2
///////////////////////////////////////////////////////////////////////////////

// Pi/PiStatusCode.h
/*
// Class definitions
//	Values of 4-127 are reserved for future use by this specification.
//	Values of 127-255 are reserved for OEM use.
#define EFI_COMPUTING_UNIT                      0x00000000
#define EFI_PERIPHERAL                          0x01000000
#define EFI_IO_BUS                              0x02000000
#define EFI_SOFTWARE                            0x03000000

// General partitioning scheme for Progress and Error Codes are
// 0x0000-0x0FFF  - Shared by all sub-classes in a given class
// 0x1000-0x7FFF  - Subclass Specific
// 0x8000-0xFFFF  - OEM specific
#define EFI_SUBCLASS_SPECIFIC                   0x1000
#define EFI_OEM_SPECIFIC                        0x8000



///////////////////////////////////////////////////////////////////////////////
// Section 3
///////////////////////////////////////////////////////////////////////////////

// Computing Unit Subclass definitions.
//	Values of 8-127 are reserved for future use by this specification.
//	Values of 128-255 are reserved for OEM use.
#define EFI_COMPUTING_UNIT_UNSPECIFIED        (EFI_COMPUTING_UNIT | 0x00000000)
#define EFI_COMPUTING_UNIT_HOST_PROCESSOR     (EFI_COMPUTING_UNIT | 0x00010000)
#define EFI_COMPUTING_UNIT_FIRMWARE_PROCESSOR (EFI_COMPUTING_UNIT | 0x00020000)
#define EFI_COMPUTING_UNIT_IO_PROCESSOR       (EFI_COMPUTING_UNIT | 0x00030000)
#define EFI_COMPUTING_UNIT_CACHE              (EFI_COMPUTING_UNIT | 0x00040000)
#define EFI_COMPUTING_UNIT_MEMORY             (EFI_COMPUTING_UNIT | 0x00050000)
#define EFI_COMPUTING_UNIT_CHIPSET            (EFI_COMPUTING_UNIT | 0x00060000)

// Computing Unit Class Progress Code definitions. These are shared by
//	all subclasses.
#define EFI_CU_PC_INIT_BEGIN                    0x00000000
#define EFI_CU_PC_INIT_END                      0x00000001

// Computing Unit Unspecified Subclass Progress Code definitions.

// Computing Unit Host Processor Subclass Progress Code definitions.
#define EFI_CU_HP_PC_POWER_ON_INIT         (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_CU_HP_PC_CACHE_INIT            (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_CU_HP_PC_RAM_INIT              (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#define EFI_CU_HP_PC_MEMORY_CONTROLLER_INIT \
										   (EFI_SUBCLASS_SPECIFIC | 0x00000003)
#define EFI_CU_HP_PC_IO_INIT               (EFI_SUBCLASS_SPECIFIC | 0x00000004)
#define EFI_CU_HP_PC_BSP_SELECT            (EFI_SUBCLASS_SPECIFIC | 0x00000005)
#define EFI_CU_HP_PC_BSP_RESELECT          (EFI_SUBCLASS_SPECIFIC | 0x00000006)
#define EFI_CU_HP_PC_AP_INIT               (EFI_SUBCLASS_SPECIFIC | 0x00000007)
#define EFI_CU_HP_PC_SMM_INIT              (EFI_SUBCLASS_SPECIFIC | 0x00000008)

// Computing Unit Firmware Processor Subclass Progress Code definitions.

// Computing Unit IO Processor Subclass Progress Code definitions.

// Computing Unit Cache Subclass Progress Code definitions.
#define EFI_CU_CACHE_PC_PRESENCE_DETECT    (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_CU_CACHE_PC_CONFIGURATION      (EFI_SUBCLASS_SPECIFIC | 0x00000001)

// Computing Unit Memory Subclass Progress Code definitions.
#define EFI_CU_MEMORY_PC_SPD_READ          (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_CU_MEMORY_PC_PRESENCE_DETECT   (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_CU_MEMORY_PC_TIMING            (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#define EFI_CU_MEMORY_PC_CONFIGURING       (EFI_SUBCLASS_SPECIFIC | 0x00000003)
#define EFI_CU_MEMORY_PC_OPTIMIZING        (EFI_SUBCLASS_SPECIFIC | 0x00000004)
#define EFI_CU_MEMORY_PC_INIT              (EFI_SUBCLASS_SPECIFIC | 0x00000005)
#define EFI_CU_MEMORY_PC_TEST              (EFI_SUBCLASS_SPECIFIC | 0x00000006)

// Computing Unit Chipset Subclass Progress Code definitions.

// Computing Unit Class Error Code definitions. These are shared by
//	all subclasses.
#define EFI_CU_EC_NON_SPECIFIC                  0x00000000
#define EFI_CU_EC_DISABLED                      0x00000001
#define EFI_CU_EC_NOT_SUPPORTED                 0x00000002
#define EFI_CU_EC_NOT_DETECTED                  0x00000003
#define EFI_CU_EC_NOT_CONFIGURED                0x00000004

// Computing Unit Unspecified Subclass Error Code definitions.

// Computing Unit Host Processor Subclass Error Code definitions.
#define EFI_CU_HP_EC_INVALID_TYPE          (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_CU_HP_EC_INVALID_SPEED         (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_CU_HP_EC_MISMATCH              (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#define EFI_CU_HP_EC_TIMER_EXPIRED         (EFI_SUBCLASS_SPECIFIC | 0x00000003)
#define EFI_CU_HP_EC_SELF_TEST             (EFI_SUBCLASS_SPECIFIC | 0x00000004)
#define EFI_CU_HP_EC_INTERNAL              (EFI_SUBCLASS_SPECIFIC | 0x00000005)
#define EFI_CU_HP_EC_THERMAL               (EFI_SUBCLASS_SPECIFIC | 0x00000006)
#define EFI_CU_HP_EC_LOW_VOLTAGE           (EFI_SUBCLASS_SPECIFIC | 0x00000007)
#define EFI_CU_HP_EC_HIGH_VOLTAGE          (EFI_SUBCLASS_SPECIFIC | 0x00000008)
#define EFI_CU_HP_EC_CACHE                 (EFI_SUBCLASS_SPECIFIC | 0x00000009)
#define EFI_CU_HP_EC_MICROCODE_UPDATE      (EFI_SUBCLASS_SPECIFIC | 0x0000000A)
#define EFI_CU_HP_EC_CORRECTABLE           (EFI_SUBCLASS_SPECIFIC | 0x0000000B)
#define EFI_CU_HP_EC_UNCORRECTABLE         (EFI_SUBCLASS_SPECIFIC | 0x0000000C)
#define EFI_CU_HP_EC_NO_MICROCODE_UPDATE  (EFI_SUBCLASS_SPECIFIC | 0x0000000D)

// Computing Unit Firmware Processor Subclass Error Code definitions.
#define EFI_CU_FP_EC_HARD_FAIL             (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_CU_FP_EC_SOFT_FAIL             (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_CU_FP_EC_COMM_ERROR            (EFI_SUBCLASS_SPECIFIC | 0x00000002)

// Computing Unit IO Processor Subclass Error Code definitions.

// Computing Unit Cache Subclass Error Code definitions.
#define EFI_CU_CACHE_EC_INVALID_TYPE       (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_CU_CACHE_EC_INVALID_SPEED      (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_CU_CACHE_EC_INVALID_SIZE       (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#define EFI_CU_CACHE_EC_MISMATCH           (EFI_SUBCLASS_SPECIFIC | 0x00000003)

// Computing Unit Memory Subclass Error Code definitions.
#define EFI_CU_MEMORY_EC_INVALID_TYPE      (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_CU_MEMORY_EC_INVALID_SPEED     (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_CU_MEMORY_EC_CORRECTABLE       (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#define EFI_CU_MEMORY_EC_UNCORRECTABLE     (EFI_SUBCLASS_SPECIFIC | 0x00000003)
#define EFI_CU_MEMORY_EC_SPD_FAIL          (EFI_SUBCLASS_SPECIFIC | 0x00000004)
#define EFI_CU_MEMORY_EC_INVALID_SIZE      (EFI_SUBCLASS_SPECIFIC | 0x00000005)
#define EFI_CU_MEMORY_EC_MISMATCH          (EFI_SUBCLASS_SPECIFIC | 0x00000006)
#define EFI_CU_MEMORY_EC_S3_RESUME_FAIL    (EFI_SUBCLASS_SPECIFIC | 0x00000007)
#define EFI_CU_MEMORY_EC_UPDATE_FAIL       (EFI_SUBCLASS_SPECIFIC | 0x00000008)
#define EFI_CU_MEMORY_EC_NONE_DETECTED     (EFI_SUBCLASS_SPECIFIC | 0x00000009)
#define EFI_CU_MEMORY_EC_NONE_USEFUL       (EFI_SUBCLASS_SPECIFIC | 0x0000000A)


#if PI_SPECIFICATION_VERSION>=0x0001000A
//
// Computing Unit Chipset Subclass Progress Code definitions.
//
#define EFI_CHIPSET_PC_PEI_CAR_SB_INIT      (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_CHIPSET_PC_PEI_CAR_NB_INIT      (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_CHIPSET_PC_PEI_MEM_SB_INIT      (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#define EFI_CHIPSET_PC_PEI_MEM_NB_INIT      (EFI_SUBCLASS_SPECIFIC | 0x00000003)
#define EFI_CHIPSET_PC_DXE_HB_INIT          (EFI_SUBCLASS_SPECIFIC | 0x00000004)
#define EFI_CHIPSET_PC_DXE_NB_INIT          (EFI_SUBCLASS_SPECIFIC | 0x00000005)
#define EFI_CHIPSET_PC_DXE_NB_SMM_INIT      (EFI_SUBCLASS_SPECIFIC | 0x00000006)
#define EFI_CHIPSET_PC_DXE_SB_RT_INIT       (EFI_SUBCLASS_SPECIFIC | 0x00000007)
#define EFI_CHIPSET_PC_DXE_SB_INIT          (EFI_SUBCLASS_SPECIFIC | 0x00000008)
#define EFI_CHIPSET_PC_DXE_SB_SMM_INIT      (EFI_SUBCLASS_SPECIFIC | 0x00000009)
#define EFI_CHIPSET_PC_DXE_SB_DEVICES_INIT  (EFI_SUBCLASS_SPECIFIC | 0x0000000A)

//
// Computing Unit Chipset Subclass Error Code definitions.
//
#define EFI_CHIPSET_EC_BAD_BATTERY          (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_CHIPSET_EC_DXE_NB_ERROR         (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_CHIPSET_EC_DXE_SB_ERROR         (EFI_SUBCLASS_SPECIFIC | 0x00000002)

#endif

///////////////////////////////////////////////////////////////////////////////
// Section 4
///////////////////////////////////////////////////////////////////////////////

// Peripheral Subclass definitions.
// Values of 12-127 are reserved for future use by this specification.
// Values of 128-255 are reserved for OEM use.
#define EFI_PERIPHERAL_UNSPECIFIED              (EFI_PERIPHERAL | 0x00000000)
#define EFI_PERIPHERAL_KEYBOARD                 (EFI_PERIPHERAL | 0x00010000)
#define EFI_PERIPHERAL_MOUSE                    (EFI_PERIPHERAL | 0x00020000)
#define EFI_PERIPHERAL_LOCAL_CONSOLE            (EFI_PERIPHERAL | 0x00030000)
#define EFI_PERIPHERAL_REMOTE_CONSOLE           (EFI_PERIPHERAL | 0x00040000)
#define EFI_PERIPHERAL_SERIAL_PORT              (EFI_PERIPHERAL | 0x00050000)
#define EFI_PERIPHERAL_PARALLEL_PORT            (EFI_PERIPHERAL | 0x00060000)
#define EFI_PERIPHERAL_FIXED_MEDIA              (EFI_PERIPHERAL | 0x00070000)
#define EFI_PERIPHERAL_REMOVABLE_MEDIA          (EFI_PERIPHERAL | 0x00080000)
#define EFI_PERIPHERAL_AUDIO_INPUT              (EFI_PERIPHERAL | 0x00090000)
#define EFI_PERIPHERAL_AUDIO_OUTPUT             (EFI_PERIPHERAL | 0x000A0000)
#define EFI_PERIPHERAL_LCD_DEVICE               (EFI_PERIPHERAL | 0x000B0000)
#define EFI_PERIPHERAL_NETWORK                  (EFI_PERIPHERAL | 0x000C0000)

// Peripheral Class Progress Code definitions. These are shared by
//	all subclasses.
#define EFI_P_PC_INIT                           0x00000000
#define EFI_P_PC_RESET                          0x00000001
#define EFI_P_PC_DISABLE                        0x00000002
#define EFI_P_PC_PRESENCE_DETECT                0x00000003
#define EFI_P_PC_ENABLE                         0x00000004
#define EFI_P_PC_RECONFIG                       0x00000005
#define EFI_P_PC_DETECTED                       0x00000006

// Peripheral Class Unspecified Subclass Progress Code definitions.

// Peripheral Class Keyboard Subclass Progress Code definitions.
#define EFI_P_KEYBOARD_PC_CLEAR_BUFFER     (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_P_KEYBOARD_PC_SELF_TEST        (EFI_SUBCLASS_SPECIFIC | 0x00000001)

// Peripheral Class Mouse Subclass Progress Code definitions.
#define EFI_P_MOUSE_PC_SELF_TEST           (EFI_SUBCLASS_SPECIFIC | 0x00000000)

// Peripheral Class Local Console Subclass Progress Code definitions.

// Peripheral Class Remote Console Subclass Progress Code definitions.

// Peripheral Class Serial Port Subclass Progress Code definitions.
#define EFI_P_SERIAL_PORT_PC_CLEAR_BUFFER  (EFI_SUBCLASS_SPECIFIC | 0x00000000)

// Peripheral Class Parallel Port Subclass Progress Code definitions.

// Peripheral Class Fixed Media Subclass Progress Code definitions.

// Peripheral Class Removable Media Subclass Progress Code definitions.

// Peripheral Class Audio Input Subclass Progress Code definitions.

// Peripheral Class Audio Output Subclass Progress Code definitions.


// Peripheral Class LCD Device Subclass Progress Code definitions.

// Peripheral Class Network Subclass Progress Code definitions.

// Peripheral Class Error Code definitions.
// These are shared by all subclasses.
#define EFI_P_EC_NON_SPECIFIC                   0x00000000
#define EFI_P_EC_DISABLED                       0x00000001
#define EFI_P_EC_NOT_SUPPORTED                  0x00000002
#define EFI_P_EC_NOT_DETECTED                   0x00000003
#define EFI_P_EC_NOT_CONFIGURED                 0x00000004
#define EFI_P_EC_INTERFACE_ERROR                0x00000005
#define EFI_P_EC_CONTROLLER_ERROR               0x00000006
#define EFI_P_EC_INPUT_ERROR                    0x00000007
#define EFI_P_EC_OUTPUT_ERROR                   0x00000008
#define EFI_P_EC_RESOURCE_CONFLICT              0x00000009

// Peripheral Class Unspecified Subclass Error Code definitions.

// Peripheral Class Keyboard Subclass Error Code definitions.
#define EFI_P_KEYBOARD_EC_LOCKED           (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_P_KEYBOARD_EC_STUCK_KEY        (EFI_SUBCLASS_SPECIFIC | 0x00000001)

// Peripheral Class Mouse Subclass Error Code definitions.
#define EFI_P_MOUSE_EC_LOCKED              (EFI_SUBCLASS_SPECIFIC | 0x00000000)

// Peripheral Class Local Console Subclass Error Code definitions.

// Peripheral Class Remote Console Subclass Error Code definitions.

// Peripheral Class Serial Port Subclass Error Code definitions.

// Peripheral Class Parallel Port Subclass Error Code definitions.

// Peripheral Class Fixed Media Subclass Error Code definitions.

// Peripheral Class Removable Media Subclass Error Code definitions.

// Peripheral Class Audio Input Subclass Error Code definitions.

// Peripheral Class Audio Output Subclass Error Code definitions.

// Peripheral Class LCD Device Subclass Error Code definitions.

// Peripheral Class Network Subclass Error Code definitions.

///////////////////////////////////////////////////////////////////////////////
// Section 5
///////////////////////////////////////////////////////////////////////////////

// IO Bus Subclass definitions.
// Values of 14-127 are reserved for future use by this specification.
// Values of 128-255 are reserved for OEM use.
#define EFI_IO_BUS_UNSPECIFIED                  (EFI_IO_BUS | 0x00000000)
#define EFI_IO_BUS_PCI                          (EFI_IO_BUS | 0x00010000)
#define EFI_IO_BUS_USB                          (EFI_IO_BUS | 0x00020000)
#define EFI_IO_BUS_IBA                          (EFI_IO_BUS | 0x00030000)
#define EFI_IO_BUS_AGP                          (EFI_IO_BUS | 0x00040000)
#define EFI_IO_BUS_PC_CARD                      (EFI_IO_BUS | 0x00050000)
#define EFI_IO_BUS_LPC                          (EFI_IO_BUS | 0x00060000)
#define EFI_IO_BUS_SCSI                         (EFI_IO_BUS | 0x00070000)
#define EFI_IO_BUS_ATA_ATAPI                    (EFI_IO_BUS | 0x00080000)
#define EFI_IO_BUS_FC                           (EFI_IO_BUS | 0x00090000)
#define EFI_IO_BUS_IP_NETWORK                   (EFI_IO_BUS | 0x000A0000)
#define EFI_IO_BUS_SMBUS                        (EFI_IO_BUS | 0x000B0000)
#define EFI_IO_BUS_I2C                          (EFI_IO_BUS | 0x000C0000)

// IO Bus Class Progress Code definitions.
// These are shared by all subclasses.
#define EFI_IOB_PC_INIT                         0x00000000
#define EFI_IOB_PC_RESET                        0x00000001
#define EFI_IOB_PC_DISABLE                      0x00000002
#define EFI_IOB_PC_DETECT                       0x00000003
#define EFI_IOB_PC_ENABLE                       0x00000004
#define EFI_IOB_PC_RECONFIG                     0x00000005
#define EFI_IOB_PC_HOTPLUG                      0x00000006

// IO Bus Class Unspecified Subclass Progress Code definitions.
*/

// IO Bus Class PCI Subclass Progress Code definitions.
#define EFI_IOB_PCI_PC_BUS_ENUM            (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_IOB_PCI_PC_RES_ALLOC           (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_IOB_PCI_PC_HPC_INIT            (EFI_SUBCLASS_SPECIFIC | 0x00000002)

// IO Bus Class USB Subclass Progress Code definitions.

// IO Bus Class IBA Subclass Progress Code definitions.

// IO Bus Class AGP Subclass Progress Code definitions.

// IO Bus Class PC Card Subclass Progress Code definitions.

// IO Bus Class LPC Subclass Progress Code definitions.

// IO Bus Class SCSI Subclass Progress Code definitions.

// IO Bus Class ATA/ATAPI Subclass Progress Code definitions.

/*
// IntelFrameworkPkg: Framework/StatusCode.h
#define EFI_IOB_ATA_BUS_SMART_ENABLE          (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_IOB_ATA_BUS_SMART_DISABLE         (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_IOB_ATA_BUS_SMART_OVERTHRESHOLD   (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#define EFI_IOB_ATA_BUS_SMART_UNDERTHRESHOLD  (EFI_SUBCLASS_SPECIFIC | 0x00000003)
*/

// IO Bus Class FC Subclass Progress Code definitions.

// IO Bus Class IP Network Subclass Progress Code definitions.

// IO Bus Class SMBUS Subclass Progress Code definitions.

// IO Bus Class I2C Subclass Progress Code definitions.

// Pi/PiStatusCode.h
/*
// IO Bus Class Error Code definitions.
// These are shared by all subclasses.
#define EFI_IOB_EC_NON_SPECIFIC                 0x00000000
#define EFI_IOB_EC_DISABLED                     0x00000001
#define EFI_IOB_EC_NOT_SUPPORTED                0x00000002
#define EFI_IOB_EC_NOT_DETECTED                 0x00000003
#define EFI_IOB_EC_NOT_CONFIGURED               0x00000004
#define EFI_IOB_EC_INTERFACE_ERROR              0x00000005
#define EFI_IOB_EC_CONTROLLER_ERROR             0x00000006
#define EFI_IOB_EC_READ_ERROR                   0x00000007
#define EFI_IOB_EC_WRITE_ERROR                  0x00000008
#define EFI_IOB_EC_RESOURCE_CONFLICT            0x00000009

// IO Bus Class Unspecified Subclass Error Code definitions.

// IO Bus Class PCI Subclass Error Code definitions.
#define EFI_IOB_PCI_EC_PERR                (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_IOB_PCI_EC_SERR                (EFI_SUBCLASS_SPECIFIC | 0x00000001)

// IO Bus Class USB Subclass Error Code definitions.

// IO Bus Class IBA Subclass Error Code definitions.

// IO Bus Class AGP Subclass Error Code definitions.

// IO Bus Class PC Card Subclass Error Code definitions.

// IO Bus Class LPC Subclass Error Code definitions.

// IO Bus Class SCSI Subclass Error Code definitions.
*/

// IO Bus Class ATA/ATAPI Subclass Error Code definitions.
/*
// IntelFrameworkPkg: Framework/StatusCode.h
#define EFI_IOB_ATA_BUS_SMART_NOTSUPPORTED  (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_IOB_ATA_BUS_SMART_DISABLED      (EFI_SUBCLASS_SPECIFIC | 0x00000001)
*/
// IO Bus Class FC Subclass Error Code definitions.

// IO Bus Class IP Network Subclass Error Code definitions.

// IO Bus Class SMBUS Subclass Error Code definitions.

// IO Bus Class I2C Subclass Error Code definitions.

///////////////////////////////////////////////////////////////////////////////
// Section 6
///////////////////////////////////////////////////////////////////////////////

// Pi/PiStatusCode.h
/*
// Software Subclass definitions.
// Values of 14-127 are reserved for future use by this specification.
// Values of 128-255 are reserved for OEM use.
#define EFI_SOFTWARE_UNSPECIFIED                (EFI_SOFTWARE | 0x00000000)
#define EFI_SOFTWARE_SEC                        (EFI_SOFTWARE | 0x00010000)
#define EFI_SOFTWARE_PEI_CORE                   (EFI_SOFTWARE | 0x00020000)
#define EFI_SOFTWARE_PEI_MODULE                 (EFI_SOFTWARE | 0x00030000)
#define EFI_SOFTWARE_DXE_CORE                   (EFI_SOFTWARE | 0x00040000)
#define EFI_SOFTWARE_DXE_BS_DRIVER              (EFI_SOFTWARE | 0x00050000)
#define EFI_SOFTWARE_DXE_RT_DRIVER              (EFI_SOFTWARE | 0x00060000)
#define EFI_SOFTWARE_SMM_DRIVER                 (EFI_SOFTWARE | 0x00070000)
#define EFI_SOFTWARE_EFI_APPLICATION            (EFI_SOFTWARE | 0x00080000)
#define EFI_SOFTWARE_EFI_OS_LOADER              (EFI_SOFTWARE | 0x00090000)
#define EFI_SOFTWARE_RT                         (EFI_SOFTWARE | 0x000A0000)
#define EFI_SOFTWARE_AL                         (EFI_SOFTWARE | 0x000B0000)
#define EFI_SOFTWARE_EBC_EXCEPTION              (EFI_SOFTWARE | 0x000C0000)
#define EFI_SOFTWARE_IA32_EXCEPTION             (EFI_SOFTWARE | 0x000D0000)
#define EFI_SOFTWARE_IPF_EXCEPTION              (EFI_SOFTWARE | 0x000E0000)
#define EFI_SOFTWARE_PEI_SERVICE                (EFI_SOFTWARE | 0x000F0000)
#define EFI_SOFTWARE_EFI_BOOT_SERVICE           (EFI_SOFTWARE | 0x00100000)
#define EFI_SOFTWARE_EFI_RUNTIME_SERVICE        (EFI_SOFTWARE | 0x00110000)
#define EFI_SOFTWARE_EFI_DXE_SERVICE            (EFI_SOFTWARE | 0x00120000)

#if PI_SPECIFICATION_VERSION>=0x0001000A

#define EFI_SOFTWARE_X64_EXCEPTION              (EFI_SOFTWARE | 0x00130000)
#define EFI_SOFTWARE_ARM_EXCEPTION              (EFI_SOFTWARE | 0x00140000)

#endif

// Software Class Progress Code definitions. These are shared by all subclasses
#define EFI_SW_PC_INIT                          0x00000000
#define EFI_SW_PC_LOAD                          0x00000001
#define EFI_SW_PC_INIT_BEGIN                    0x00000002
#define EFI_SW_PC_INIT_END                      0x00000003
#define EFI_SW_PC_AUTHENTICATE_BEGIN            0x00000004
#define EFI_SW_PC_AUTHENTICATE_END              0x00000005
#define EFI_SW_PC_INPUT_WAIT                    0x00000006
#define EFI_SW_PC_USER_SETUP                    0x00000007


// Software Class Unspecified Subclass Progress Code definitions.

// Software Class SEC Subclass Progress Code definitions.
#define EFI_SW_SEC_PC_ENTRY_POINT          (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_SW_SEC_PC_HANDOFF_TO_NEXT      (EFI_SUBCLASS_SPECIFIC | 0x00000001)

// Software Class PEI Core Subclass Progress Code definitions.
#define EFI_SW_PEI_CORE_PC_ENTRY_POINT     (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_SW_PEI_CORE_PC_HANDOFF_TO_NEXT (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_SW_PEI_CORE_PC_RETURN_TO_LAST  (EFI_SUBCLASS_SPECIFIC | 0x00000002)
*/
/*
// IntelFrameworkPkg: Framework/StatusCode.h
// Software Class PEI Module Subclass Progress Code definitions.
#define EFI_SW_PEIM_PC_RECOVERY_BEGIN      (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_SW_PEIM_PC_CAPSULE_LOAD        (EFI_SUBCLASS_SPECIFIC | 0x00000001)
//Status Code Specification 0.92 defines both CAPSULE_LOAD and CAPSULE_START as 1, which is a bug
#define EFI_SW_PEIM_PC_CAPSULE_START       (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#define EFI_SW_PEIM_PC_RECOVERY_USER       (EFI_SUBCLASS_SPECIFIC | 0x00000003)
#define EFI_SW_PEIM_PC_RECOVERY_AUTO       (EFI_SUBCLASS_SPECIFIC | 0x00000004)
*/
#if PI_SPECIFICATION_VERSION>=0x0001000A
//
// Software Class PEI Module Subclass Progress Code definitions.
//
// Pi/PiStatusCode.h
/*
#define EFI_SW_PEI_PC_RECOVERY_BEGIN        EFI_SW_PEIM_PC_RECOVERY_BEGIN
#define EFI_SW_PEI_PC_CAPSULE_LOAD          EFI_SW_PEIM_PC_CAPSULE_LOAD
#define EFI_SW_PEI_PC_CAPSULE_START         EFI_SW_PEIM_PC_CAPSULE_START
#define EFI_SW_PEI_PC_RECOVERY_USER         EFI_SW_PEIM_PC_RECOVERY_USER
#define EFI_SW_PEI_PC_RECOVERY_AUTO         EFI_SW_PEIM_PC_RECOVERY_AUTO
#define EFI_SW_PEI_PC_S3_BOOT_SCRIPT        (EFI_SUBCLASS_SPECIFIC | 0x00000005)
#define EFI_SW_PEI_PC_OS_WAKE               (EFI_SUBCLASS_SPECIFIC | 0x00000006)
*/
#endif

// Pi/PiStatusCode.h
/*
// Software Class DXE Core Subclass Progress Code definitions.
#define EFI_SW_DXE_CORE_PC_ENTRY_POINT     (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_SW_DXE_CORE_PC_HANDOFF_TO_NEXT (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_SW_DXE_CORE_PC_RETURN_TO_LAST  (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#define EFI_SW_DXE_CORE_PC_START_DRIVER    (EFI_SUBCLASS_SPECIFIC | 0x00000003)

// Software Class DXE BS Driver Subclass Progress Code definitions.
#define EFI_SW_DXE_BS_PC_LEGACY_OPROM_INIT            (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_SW_DXE_BS_PC_READY_TO_BOOT_EVENT          (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_SW_DXE_BS_PC_LEGACY_BOOT_EVENT            (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#define EFI_SW_DXE_BS_PC_EXIT_BOOT_SERVICES_EVENT     (EFI_SUBCLASS_SPECIFIC | 0x00000003)
#define EFI_SW_DXE_BS_PC_VIRTUAL_ADDRESS_CHANGE_EVENT (EFI_SUBCLASS_SPECIFIC | 0x00000004)
#define EFI_SW_DXE_BS_PC_BEGIN_CONNECTING_DRIVERS     (EFI_SUBCLASS_SPECIFIC | 0x00000005)
#define EFI_SW_DXE_BS_PC_VERIFYING_PASSWORD           (EFI_SUBCLASS_SPECIFIC | 0x00000006)

// Software Class DXE RT Driver Subclass Progress Code definitions.
#define EFI_SW_DXE_RT_PC_S0 (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_SW_DXE_RT_PC_S1 (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_SW_DXE_RT_PC_S2 (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#define EFI_SW_DXE_RT_PC_S3 (EFI_SUBCLASS_SPECIFIC | 0x00000003)
#define EFI_SW_DXE_RT_PC_S4 (EFI_SUBCLASS_SPECIFIC | 0x00000004)
#define EFI_SW_DXE_RT_PC_S5 (EFI_SUBCLASS_SPECIFIC | 0x00000005)

// Software Class SMM Driver Subclass Progress Code definitions.

// Software Class EFI Application Subclass Progress Code definitions.

// Software Class EFI OS Loader Subclass Progress Code definitions.

// Software Class EFI RT Subclass Progress Code definitions.
#define EFI_SW_RT_PC_ENTRY_POINT           (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_SW_RT_PC_HANDOFF_TO_NEXT       (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_SW_RT_PC_RETURN_TO_LAST        (EFI_SUBCLASS_SPECIFIC | 0x00000002)
*/

/*
// IntelFrameworkPkg: Framework/StatusCode.h
// Software Class EFI AL Subclass Progress Code definitions.
#define EFI_SW_AL_PC_ENTRY_POINT           (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_SW_AL_PC_RETURN_TO_LAST        (EFI_SUBCLASS_SPECIFIC | 0x00000001)
*/

// Software Class EBC Exception Subclass Progress Code definitions.

// Software Class IA32 Exception Subclass Progress Code definitions.

// Software Class IPF Exception Subclass Progress Code definitions.

// Pi/PiStatusCode.h
/*
// Software Class PEI Services Subclass Progress Code definitions.
#define EFI_SW_PS_PC_INSTALL_PPI           (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_SW_PS_PC_REINSTALL_PPI         (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_SW_PS_PC_LOCATE_PPI            (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#define EFI_SW_PS_PC_NOTIFY_PPI            (EFI_SUBCLASS_SPECIFIC | 0x00000003)
#define EFI_SW_PS_PC_GET_BOOT_MODE         (EFI_SUBCLASS_SPECIFIC | 0x00000004)
#define EFI_SW_PS_PC_SET_BOOT_MODE         (EFI_SUBCLASS_SPECIFIC | 0x00000005)
#define EFI_SW_PS_PC_GET_HOB_LIST          (EFI_SUBCLASS_SPECIFIC | 0x00000006)
#define EFI_SW_PS_PC_CREATE_HOB            (EFI_SUBCLASS_SPECIFIC | 0x00000007)
#define EFI_SW_PS_PC_FFS_FIND_NEXT_VOLUME  (EFI_SUBCLASS_SPECIFIC | 0x00000008)
#define EFI_SW_PS_PC_FFS_FIND_NEXT_FILE    (EFI_SUBCLASS_SPECIFIC | 0x00000009)
#define EFI_SW_PS_PC_FFS_FIND_SECTION_DATA (EFI_SUBCLASS_SPECIFIC | 0x0000000A)
#define EFI_SW_PS_PC_INSTALL_PEI_MEMORY    (EFI_SUBCLASS_SPECIFIC | 0x0000000B)
#define EFI_SW_PS_PC_ALLOCATE_PAGES        (EFI_SUBCLASS_SPECIFIC | 0x0000000C)
#define EFI_SW_PS_PC_ALLOCATE_POOL         (EFI_SUBCLASS_SPECIFIC | 0x0000000D)
#define EFI_SW_PS_PC_COPY_MEM              (EFI_SUBCLASS_SPECIFIC | 0x0000000E)
#define EFI_SW_PS_PC_SET_MEM               (EFI_SUBCLASS_SPECIFIC | 0x0000000F)

#if PI_SPECIFICATION_VERSION>=0x0001000A
#define EFI_SW_PS_PC_RESET_SYSTEM               (EFI_SUBCLASS_SPECIFIC | 0x00000010)
#define EFI_SW_PS_PC_FFS_FIND_FILE_BY_NAME      (EFI_SUBCLASS_SPECIFIC | 0x00000013)
#define EFI_SW_PS_PC_FFS_GET_FILE_INFO          (EFI_SUBCLASS_SPECIFIC | 0x00000014)
#define EFI_SW_PS_PC_FFS_GET_VOLUME_INFO        (EFI_SUBCLASS_SPECIFIC | 0x00000015)
#define EFI_SW_PS_PC_FFS_REGISTER_FOR_SHADOW    (EFI_SUBCLASS_SPECIFIC | 0x00000016)
#endif
// Software Class EFI Boot Services Subclass Progress Code definitions.
#define EFI_SW_BS_PC_RAISE_TPL             (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_SW_BS_PC_RESTORE_TPL           (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_SW_BS_PC_ALLOCATE_PAGES        (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#define EFI_SW_BS_PC_FREE_PAGES            (EFI_SUBCLASS_SPECIFIC | 0x00000003)
#define EFI_SW_BS_PC_GET_MEMORY_MAP        (EFI_SUBCLASS_SPECIFIC | 0x00000004)
#define EFI_SW_BS_PC_ALLOCATE_POOL         (EFI_SUBCLASS_SPECIFIC | 0x00000005)
#define EFI_SW_BS_PC_FREE_POOL             (EFI_SUBCLASS_SPECIFIC | 0x00000006)
#define EFI_SW_BS_PC_CREATE_EVENT          (EFI_SUBCLASS_SPECIFIC | 0x00000007)
#define EFI_SW_BS_PC_SET_TIMER             (EFI_SUBCLASS_SPECIFIC | 0x00000008)
#define EFI_SW_BS_PC_WAIT_FOR_EVENT        (EFI_SUBCLASS_SPECIFIC | 0x00000009)
#define EFI_SW_BS_PC_SIGNAL_EVENT          (EFI_SUBCLASS_SPECIFIC | 0x0000000A)
#define EFI_SW_BS_PC_CLOSE_EVENT           (EFI_SUBCLASS_SPECIFIC | 0x0000000B)
#define EFI_SW_BS_PC_CHECK_EVENT           (EFI_SUBCLASS_SPECIFIC | 0x0000000C)
#define EFI_SW_BS_PC_INSTALL_PROTOCOL_INTERFACE \
										   (EFI_SUBCLASS_SPECIFIC | 0x0000000D)
#define EFI_SW_BS_PC_REINSTALL_PROTOCOL_INTERFACE \
										   (EFI_SUBCLASS_SPECIFIC | 0x0000000E)
#define EFI_SW_BS_PC_UNINSTALL_PROTOCOL_INTERFACE \
										   (EFI_SUBCLASS_SPECIFIC | 0x0000000F)
#define EFI_SW_BS_PC_HANDLE_PROTOCOL       (EFI_SUBCLASS_SPECIFIC | 0x00000010)
#define EFI_SW_BS_PC_PC_HANDLE_PROTOCOL    (EFI_SUBCLASS_SPECIFIC | 0x00000011)
#define EFI_SW_BS_PC_REGISTER_PROTOCOL_NOTIFY \
										   (EFI_SUBCLASS_SPECIFIC | 0x00000012)
#define EFI_SW_BS_PC_LOCATE_HANDLE         (EFI_SUBCLASS_SPECIFIC | 0x00000013)
#define EFI_SW_BS_PC_INSTALL_CONFIGURATION_TABLE \
										   (EFI_SUBCLASS_SPECIFIC | 0x00000014)
#define EFI_SW_BS_PC_LOAD_IMAGE            (EFI_SUBCLASS_SPECIFIC | 0x00000015)
#define EFI_SW_BS_PC_START_IMAGE           (EFI_SUBCLASS_SPECIFIC | 0x00000016)
#define EFI_SW_BS_PC_EXIT                  (EFI_SUBCLASS_SPECIFIC | 0x00000017)
#define EFI_SW_BS_PC_UNLOAD_IMAGE          (EFI_SUBCLASS_SPECIFIC | 0x00000018)
#define EFI_SW_BS_PC_EXIT_BOOT_SERVICES    (EFI_SUBCLASS_SPECIFIC | 0x00000019)
#define EFI_SW_BS_PC_GET_NEXT_MONOTONIC_COUNT   \
										   (EFI_SUBCLASS_SPECIFIC | 0x0000001A)
#define EFI_SW_BS_PC_STALL                 (EFI_SUBCLASS_SPECIFIC | 0x0000001B)
#define EFI_SW_BS_PC_SET_WATCHDOG_TIMER    (EFI_SUBCLASS_SPECIFIC | 0x0000001C)
#define EFI_SW_BS_PC_CONNECT_CONTROLLER    (EFI_SUBCLASS_SPECIFIC | 0x0000001D)
#define EFI_SW_BS_PC_DISCONNECT_CONTROLLER (EFI_SUBCLASS_SPECIFIC | 0x0000001E)
#define EFI_SW_BS_PC_OPEN_PROTOCOL         (EFI_SUBCLASS_SPECIFIC | 0x0000001F)
#define EFI_SW_BS_PC_CLOSE_PROTOCOL        (EFI_SUBCLASS_SPECIFIC | 0x00000020)
#define EFI_SW_BS_PC_OPEN_PROTOCOL_INFORMATION  \
										   (EFI_SUBCLASS_SPECIFIC | 0x00000021)
#define EFI_SW_BS_PC_PROTOCOLS_PER_HANDLE  (EFI_SUBCLASS_SPECIFIC | 0x00000022)
#define EFI_SW_BS_PC_LOCATE_HANDLE_BUFFER  (EFI_SUBCLASS_SPECIFIC | 0x00000023)
#define EFI_SW_BS_PC_LOCATE_PROTOCOL       (EFI_SUBCLASS_SPECIFIC | 0x00000024)
#define EFI_SW_BS_PC_INSTALL_MULTIPLE_INTERFACES   (EFI_SUBCLASS_SPECIFIC | 0x00000025)
#define EFI_SW_BS_PC_UNINSTALL_MULTIPLE_INTERFACES (EFI_SUBCLASS_SPECIFIC | 0x00000026)
#define EFI_SW_BS_PC_CALCULATE_CRC_32      (EFI_SUBCLASS_SPECIFIC | 0x00000027)
#define EFI_SW_BS_PC_COPY_MEM              (EFI_SUBCLASS_SPECIFIC | 0x00000028)
#define EFI_SW_BS_PC_SET_MEM               (EFI_SUBCLASS_SPECIFIC | 0x00000029)
#if PI_SPECIFICATION_VERSION>=0x0001000A
#define EFI_SW_BS_PC_CREATE_EVENT_EX       (EFI_SUBCLASS_SPECIFIC | 0x0000002A)
#endif
//
// Software Class EFI Runtime Services Subclass Progress Code definitions.
//
#define EFI_SW_RS_PC_GET_TIME              (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_SW_RS_PC_SET_TIME              (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_SW_RS_PC_GET_WAKEUP_TIME       (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#define EFI_SW_RS_PC_SET_WAKEUP_TIME       (EFI_SUBCLASS_SPECIFIC | 0x00000003)
#define EFI_SW_RS_PC_SET_VIRTUAL_ADDRESS_MAP \
									       (EFI_SUBCLASS_SPECIFIC | 0x00000004)
#define EFI_SW_RS_PC_CONVERT_POINTER       (EFI_SUBCLASS_SPECIFIC | 0x00000005)
#define EFI_SW_RS_PC_GET_VARIABLE          (EFI_SUBCLASS_SPECIFIC | 0x00000006)
#define EFI_SW_RS_PC_GET_NEXT_VARIABLE_NAME \
									       (EFI_SUBCLASS_SPECIFIC | 0x00000007)
#define EFI_SW_RS_PC_SET_VARIABLE          (EFI_SUBCLASS_SPECIFIC | 0x00000008)
#define EFI_SW_RS_PC_GET_NEXT_HIGH_MONOTONIC_COUNT \
										   (EFI_SUBCLASS_SPECIFIC | 0x00000009)
#define EFI_SW_RS_PC_RESET_SYSTEM          (EFI_SUBCLASS_SPECIFIC | 0x0000000A)
#if PI_SPECIFICATION_VERSION>=0x0001000A
#define EFI_SW_RS_PC_UPDATE_CAPSULE         (EFI_SUBCLASS_SPECIFIC | 0x0000000B)
#define EFI_SW_RS_PC_QUERY_CAPSULE_CAPABILITIES \
                                            (EFI_SUBCLASS_SPECIFIC | 0x0000000C)
#define EFI_SW_RS_PC_QUERY_VARIABLE_INFO    (EFI_SUBCLASS_SPECIFIC | 0x0000000D)
#endif
// Software Class EFI DXE Services Subclass Progress Code definitions
#define EFI_SW_DS_PC_ADD_MEMORY_SPACE      (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_SW_DS_PC_ALLOCATE_MEMORY_SPACE (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_SW_DS_PC_FREE_MEMORY_SPACE     (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#define EFI_SW_DS_PC_REMOVE_MEMORY_SPACE   (EFI_SUBCLASS_SPECIFIC | 0x00000003)
#define EFI_SW_DS_PC_GET_MEMORY_SPACE_DESCRIPTOR  \
										   (EFI_SUBCLASS_SPECIFIC | 0x00000004)
#define EFI_SW_DS_PC_SET_MEMORY_SPACE_ATTRIBUTES \
										   (EFI_SUBCLASS_SPECIFIC | 0x00000005)
#define EFI_SW_DS_PC_GET_MEMORY_SPACE_MAP  (EFI_SUBCLASS_SPECIFIC | 0x00000006)
#define EFI_SW_DS_PC_ADD_IO_SPACE          (EFI_SUBCLASS_SPECIFIC | 0x00000007)
#define EFI_SW_DS_PC_ALLOCATE_IO_SPACE     (EFI_SUBCLASS_SPECIFIC | 0x00000008)
#define EFI_SW_DS_PC_FREE_IO_SPACE         (EFI_SUBCLASS_SPECIFIC | 0x00000009)
#define EFI_SW_DS_PC_REMOVE_IO_SPACE       (EFI_SUBCLASS_SPECIFIC | 0x0000000A)
#define EFI_SW_DS_PC_GET_IO_SPACE_DESCRIPTOR \
									       (EFI_SUBCLASS_SPECIFIC | 0x0000000B)
#define EFI_SW_DS_PC_GET_IO_SPACE_MAP      (EFI_SUBCLASS_SPECIFIC | 0x0000000C)
#define EFI_SW_DS_PC_DISPATCH              (EFI_SUBCLASS_SPECIFIC | 0x0000000D)
#define EFI_SW_DS_PC_SCHEDULE              (EFI_SUBCLASS_SPECIFIC | 0x0000000E)
#define EFI_SW_DS_PC_TRUST                 (EFI_SUBCLASS_SPECIFIC | 0x0000000F)
#define EFI_SW_DS_PC_PROCESS_FIRMWARE_VOLUME  \
									       (EFI_SUBCLASS_SPECIFIC | 0x00000010)

// Software Class Error Code definitions.
// These are shared by all subclasses.
#define EFI_SW_EC_NON_SPECIFIC                  0x00000000
#define EFI_SW_EC_LOAD_ERROR                    0x00000001
#define EFI_SW_EC_INVALID_PARAMETER             0x00000002
#define EFI_SW_EC_UNSUPPORTED                   0x00000003
#define EFI_SW_EC_INVALID_BUFFER                0x00000004
#define EFI_SW_EC_OUT_OF_RESOURCES              0x00000005
#define EFI_SW_EC_ABORTED                       0x00000006
#define EFI_SW_EC_ILLEGAL_SOFTWARE_STATE        0x00000007
#define EFI_SW_EC_ILLEGAL_HARDWARE_STATE        0x00000008
#define EFI_SW_EC_START_ERROR                   0x00000009
#define EFI_SW_EC_BAD_DATE_TIME                 0x0000000A
#define EFI_SW_EC_CFG_INVALID                   0x0000000B
#define EFI_SW_EC_CFG_CLR_REQUEST               0x0000000C
#define EFI_SW_EC_CFG_DEFAULT                   0x0000000D
#define EFI_SW_EC_PWD_INVALID                   0x0000000E
#define EFI_SW_EC_PWD_CLR_REQUEST               0x0000000F
#define EFI_SW_EC_PWD_CLEARED                   0x00000010
#define EFI_SW_EC_EVENT_LOG_FULL                0x00000011

// Software Class Unspecified Subclass Error Code definitions.

// Software Class SEC Subclass Error Code definitions.

// Software Class PEI Core Subclass Error Code definitions.
#define EFI_SW_PEI_CORE_EC_DXE_CORRUPT              (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#if PI_SPECIFICATION_VERSION>=0x0001000A
#define EFI_SW_PEI_CORE_EC_DXEIPL_NOT_FOUND         (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_SW_PEI_CORE_EC_MEMORY_NOT_INSTALLED     (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#endif
*/

// Software Class PEI Module Subclass Error Code definitions.
#define EFI_SW_PEIM_EC_NO_RECOVERY_CAPSULE (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_SW_PEIM_EC_INVALID_CAPSULE_DESCRIPTOR   \
										   (EFI_SUBCLASS_SPECIFIC | 0x00000001)

// Pi/PiStatusCode.h
/*
#if PI_SPECIFICATION_VERSION>=0x0001000A
#define EFI_SW_PEI_EC_S3_RESUME_PPI_NOT_FOUND   (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#define EFI_SW_PEI_EC_S3_BOOT_SCRIPT_ERROR      (EFI_SUBCLASS_SPECIFIC | 0x00000003)
#define EFI_SW_PEI_EC_S3_OS_WAKE_ERROR          (EFI_SUBCLASS_SPECIFIC | 0x00000004)
#define EFI_SW_PEI_EC_S3_RESUME_FAILED          (EFI_SUBCLASS_SPECIFIC | 0x00000005)
#define EFI_SW_PEI_EC_RECOVERY_PPI_NOT_FOUND    (EFI_SUBCLASS_SPECIFIC | 0x00000006)
#define EFI_SW_PEI_EC_RECOVERY_FAILED           (EFI_SUBCLASS_SPECIFIC | 0x00000007)
#endif
// Software Class DXE Core Subclass Error Code definitions.

// Software Class DXE Boot Service Driver Subclass Error Code definitions.
#define EFI_SW_DXE_BS_EC_LEGACY_OPROM_NO_SPACE  \
										    (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#if PI_SPECIFICATION_VERSION>=0x0001000A
#define EFI_SW_DXE_BS_EC_INVALID_PASSWORD           (EFI_SUBCLASS_SPECIFIC | 0x00000001)
#define EFI_SW_DXE_BS_EC_BOOT_OPTION_LOAD_ERROR     (EFI_SUBCLASS_SPECIFIC | 0x00000002)
#define EFI_SW_DXE_BS_EC_BOOT_OPTION_FAILED         (EFI_SUBCLASS_SPECIFIC | 0x00000003)
#define EFI_SW_DXE_BS_EC_INVALID_IDE_PASSWORD       (EFI_SUBCLASS_SPECIFIC | 0x00000004)
#endif
// Software Class DXE Runtime Service Driver Subclass Error Code definitions.
//
#if PI_SPECIFICATION_VERSION>=0x0001000A
//
// Software Class DXE Foundation Subclass Error Code
// definitions.
//
#define EFI_SW_DXE_CORE_EC_NO_ARCH      (EFI_SUBCLASS_SPECIFIC | 0x00000000)

#endif
// Software Class SMM Driver Subclass Error Code definitions.

// Software Class EFI Application Subclass Error Code definitions.

// Software Class EFI OS Loader Subclass Error Code definitions.

// Software Class EFI RT Subclass Error Code definitions.

// Software Class EFI AL Subclass Error Code definitions.

// Software Class EBC Exception Subclass Error Code definitions.
// These exceptions are derived from the debug protocol definitions in the EFI
// specification.
#define EFI_SW_EC_EBC_UNDEFINED                 0x00000000
#define EFI_SW_EC_EBC_DIVIDE_ERROR              EXCEPT_EBC_DIVIDE_ERROR
#define EFI_SW_EC_EBC_DEBUG                     EXCEPT_EBC_DEBUG
#define EFI_SW_EC_EBC_BREAKPOINT                EXCEPT_EBC_BREAKPOINT
#define EFI_SW_EC_EBC_OVERFLOW                  EXCEPT_EBC_OVERFLOW
#define EFI_SW_EC_EBC_INVALID_OPCODE            EXCEPT_EBC_INVALID_OPCODE
#define EFI_SW_EC_EBC_STACK_FAULT               EXCEPT_EBC_STACK_FAULT
#define EFI_SW_EC_EBC_ALIGNMENT_CHECK           EXCEPT_EBC_ALIGNMENT_CHECK
#define EFI_SW_EC_EBC_INSTRUCTION_ENCODING      EXCEPT_EBC_INSTRUCTION_ENCODING
#define EFI_SW_EC_EBC_BAD_BREAK                 EXCEPT_EBC_BAD_BREAK
#define EFI_SW_EC_EBC_STEP                      EXCEPT_EBC_STEP

// Software Class IA32 Exception Subclass Error Code definitions.
//	These exceptions are derived from the debug protocol definitions in the EFI
//	specification.
#define EFI_SW_EC_IA32_DIVIDE_ERROR             EXCEPT_IA32_DIVIDE_ERROR
#define EFI_SW_EC_IA32_DEBUG                    EXCEPT_IA32_DEBUG
#define EFI_SW_EC_IA32_NMI                      EXCEPT_IA32_NMI
#define EFI_SW_EC_IA32_BREAKPOINT               EXCEPT_IA32_BREAKPOINT
#define EFI_SW_EC_IA32_OVERFLOW                 EXCEPT_IA32_OVERFLOW
#define EFI_SW_EC_IA32_BOUND                    EXCEPT_IA32_BOUND
#define EFI_SW_EC_IA32_INVALID_OPCODE           EXCEPT_IA32_INVALID_OPCODE
#define EFI_SW_EC_IA32_DOUBLE_FAULT             EXCEPT_IA32_DOUBLE_FAULT
#define EFI_SW_EC_IA32_INVALID_TSS              EXCEPT_IA32_INVALID_TSS
#define EFI_SW_EC_IA32_SEG_NOT_PRESENT          EXCEPT_IA32_SEG_NOT_PRESENT
#define EFI_SW_EC_IA32_STACK_FAULT              EXCEPT_IA32_STACK_FAULT
#define EFI_SW_EC_IA32_GP_FAULT                 EXCEPT_IA32_GP_FAULT
#define EFI_SW_EC_IA32_PAGE_FAULT               EXCEPT_IA32_PAGE_FAULT
#define EFI_SW_EC_IA32_FP_ERROR                 EXCEPT_IA32_FP_ERROR
#define EFI_SW_EC_IA32_ALIGNMENT_CHECK          EXCEPT_IA32_ALIGNMENT_CHECK
#define EFI_SW_EC_IA32_MACHINE_CHECK            EXCEPT_IA32_MACHINE_CHECK
#define EFI_SW_EC_IA32_SIMD                     EXCEPT_IA32_SIMD

// Software Class IPF Exception Subclass Error Code definitions. These
//	exceptions are derived from the debug protocol definitions in the EFI
// 	specification.
#define EFI_SW_EC_IPF_ALT_DTLB                  EXCEPT_IPF_ALT_DTLB
#define EFI_SW_EC_IPF_DNESTED_TLB               EXCEPT_IPF_DNESTED_TLB
#define EFI_SW_EC_IPF_BREAKPOINT                EXCEPT_IPF_BREAKPOINT
#define EFI_SW_EC_IPF_EXTERNAL_INTERRUPT        EXCEPT_IPF_EXTERNAL_INTERRUPT
#define EFI_SW_EC_IPF_GEN_EXCEPT                EXCEPT_IPF_GEN_EXCEPT
#define EFI_SW_EC_IPF_NAT_CONSUMPTION           EXCEPT_IPF_NAT_CONSUMPTION
#define EFI_SW_EC_IPF_DEBUG_EXCEPT              EXCEPT_IPF_DEBUG_EXCEPT
#define EFI_SW_EC_IPF_UNALIGNED_ACCESS          EXCEPT_IPF_UNALIGNED_ACCESS
#define EFI_SW_EC_IPF_FP_FAULT                  EXCEPT_IPF_FP_FAULT
#define EFI_SW_EC_IPF_FP_TRAP                   EXCEPT_IPF_FP_TRAP
#define EFI_SW_EC_IPF_TAKEN_BRANCH              EXCEPT_IPF_TAKEN_BRANCH
#define EFI_SW_EC_IPF_SINGLE_STEP               EXCEPT_IPF_SINGLE_STEP

// Software Class PEI Service Subclass Error Code definitions.
#if PI_SPECIFICATION_VERSION>=0x0001000A
#define EFI_SW_PS_EC_RESET_NOT_AVAILABLE        (EFI_SUBCLASS_SPECIFIC | 0x00000000)
#define EFI_SW_PS_EC_MEMORY_INSTALLED_TWICE     (EFI_SUBCLASS_SPECIFIC | 0x00000001)

//
// Software Class X64 Exception Subclass Error Code definitions.
// These exceptions are derived from the debug protocol
// definitions in the EFI
// specification.
//
#define EFI_SW_EC_X64_DIVIDE_ERROR          EXCEPT_X64_DIVIDE_ERROR
#define EFI_SW_EC_X64_DEBUG                 EXCEPT_X64_DEBUG
#define EFI_SW_EC_X64_NMI                   EXCEPT_X64_NMI
#define EFI_SW_EC_X64_BREAKPOINT            EXCEPT_X64_BREAKPOINT
#define EFI_SW_EC_X64_OVERFLOW              EXCEPT_X64_OVERFLOW
#define EFI_SW_EC_X64_BOUND                 EXCEPT_X64_BOUND
#define EFI_SW_EC_X64_INVALID_OPCODE        EXCEPT_X64_INVALID_OPCODE
#define EFI_SW_EC_X64_DOUBLE_FAULT          EXCEPT_X64_DOUBLE_FAULT
#define EFI_SW_EC_X64_INVALID_TSS           EXCEPT_X64_INVALID_TSS
#define EFI_SW_EC_X64_SEG_NOT_PRESENT       EXCEPT_X64_SEG_NOT_PRESENT
#define EFI_SW_EC_X64_STACK_FAULT           EXCEPT_X64_STACK_FAULT
#define EFI_SW_EC_X64_GP_FAULT              EXCEPT_X64_GP_FAULT
#define EFI_SW_EC_X64_PAGE_FAULT            EXCEPT_X64_PAGE_FAULT
#define EFI_SW_EC_X64_FP_ERROR              EXCEPT_X64_FP_ERROR
#define EFI_SW_EC_X64_ALIGNMENT_CHECK       EXCEPT_X64_ALIGNMENT_CHECK
#define EFI_SW_EC_X64_MACHINE_CHECK         EXCEPT_X64_MACHINE_CHECK
#define EFI_SW_EC_X64_SIMD                  EXCEPT_X64_SIMD
//
// Software Class ARM Exception Subclass Error Code definitions.
// These exceptions are derived from the debug protocol
// definitions in the EFI
// specification.
//
#define EFI_SW_EC_ARM_RESET                 EXCEPT_ARM_RESET
#define EFI_SW_EC_ARM_UNDEFINED_INSTRUCTION EXCEPT_ARM_UNDEFINED_INSTRUCTION
#define EFI_SW_EC_ARM_SOFTWARE_INTERRUPT    EXCEPT_ARM_SOFTWARE_INTERRUPT
#define EFI_SW_EC_ARM_PREFETCH_ABORT        EXCEPT_ARM_PREFETCH_ABORT
#define EFI_SW_EC_ARM_DATA_ABORT            EXCEPT_ARM_DATA_ABORT
#define EFI_SW_EC_ARM_RESERVED              EXCEPT_ARM_RESERVED
#define EFI_SW_EC_ARM_IRQ                   EXCEPT_ARM_IRQ
#define EFI_SW_EC_ARM_FIQ                   EXCEPT_ARM_FIQ

#endif
// Software Class EFI Boot Service Subclass Error Code definitions.

// Software Class EFI Runtime Service Subclass Error Code definitions.

// Software Class EFI DXE Service Subclass Error Code definitions.

///////////////////////////////////////////////////////////////////////////////
// Section 7
///////////////////////////////////////////////////////////////////////////////

// Debug Code definitions for all classes and subclass
// Only one debug code is defined at this point and should
// be used for anything that gets sent to debug stream.
#define EFI_DC_UNSPECIFIED                      0x0
*/

///////////////////////////////////////////////////////////////////////////////
// Section 7
///////////////////////////////////////////////////////////////////////////////
//Status Code Data Structures
#pragma pack(push,1)

// Guid/StatusCodeDataTypeId.h
/*
typedef enum {
	EfiStringAscii, 
	EfiStringUnicode, 
	EfiStringToken
} EFI_STRING_TYPE;
#if PI_SPECIFICATION_VERSION>=0x0001000A
//
// HII string token
//
typedef struct {
    EFI_HII_HANDLE  Handle;
    EFI_STRING_ID   Token;
} EFI_STATUS_CODE_STRING_TOKEN;

#endif

typedef union {
	CHAR8							*Ascii;
	CHAR16							*Unicode;
#if PI_SPECIFICATION_VERSION>=0x0001000A
	EFI_STATUS_CODE_STRING_TOKEN	Hii;
#endif
} EFI_STATUS_CODE_STRING;

typedef struct {
	EFI_STATUS_CODE_DATA	DataHeader;
	EFI_STRING_TYPE			StringType;
	EFI_STATUS_CODE_STRING	String;
} EFI_STATUS_CODE_STRING_DATA;

typedef struct {
	EFI_STATUS_CODE_DATA		DataHeader;
	UINT32						LineNumber;
	UINT32						FileNameSize;
	EFI_STATUS_CODE_STRING_DATA	*FileName;
} EFI_DEBUG_ASSERT_DATA;
*/

///////////////////////////////////////////////////////////////////////////////
// AMI-specific operations
///////////////////////////////////////////////////////////////////////////////
//AMI Status Code definitions
#define AMI_STATUS_CODE_CLASS EFI_OEM_SPECIFIC //0x8000
#define AMI_STATUS_CODE_BEEP_CLASS 0x8100

// Host Processor AMI-specific codes
#define AMI_CU_HP_PC_DXE_CPU_INIT (AMI_STATUS_CODE_CLASS | 0x00000000)
#define AMI_CU_HP_EC_DXE_CPU_ERROR (AMI_STATUS_CODE_CLASS | 0x00000001)

//Chipset AMI-specific codes
#define AMI_CHIPSET_PC_PEI_CAR_SB_INIT (AMI_STATUS_CODE_CLASS | 0x00000000)
#define AMI_CHIPSET_PC_PEI_CAR_NB_INIT (AMI_STATUS_CODE_CLASS | 0x00000001)
#define AMI_CHIPSET_PC_PEI_MEM_SB_INIT (AMI_STATUS_CODE_CLASS | 0x00000002)
#define AMI_CHIPSET_PC_PEI_MEM_NB_INIT (AMI_STATUS_CODE_CLASS | 0x00000003)
#define AMI_CHIPSET_PC_DXE_HB_INIT (AMI_STATUS_CODE_CLASS | 0x00000004)
#define AMI_CHIPSET_PC_DXE_NB_INIT (AMI_STATUS_CODE_CLASS | 0x00000005)
#define AMI_CHIPSET_PC_DXE_NB_SMM_INIT (AMI_STATUS_CODE_CLASS | 0x00000006)
#define AMI_CHIPSET_PC_DXE_SB_RT_INIT (AMI_STATUS_CODE_CLASS | 0x00000007)
#define AMI_CHIPSET_PC_DXE_SB_INIT (AMI_STATUS_CODE_CLASS | 0x00000008)
#define AMI_CHIPSET_PC_DXE_SB_SMM_INIT (AMI_STATUS_CODE_CLASS | 0x00000009)
#define AMI_CHIPSET_PC_DXE_SB_DEVICES_INIT (AMI_STATUS_CODE_CLASS | 0x0000000A)
#define AMI_CHIPSET_EC_BAD_BATTERY     (AMI_STATUS_CODE_CLASS | 0x0000000B)
#define AMI_CHIPSET_EC_INTRUDER_DETECT (AMI_STATUS_CODE_CLASS | 0x0000000C)
#define AMI_CHIPSET_EC_DXE_NB_ERROR (AMI_STATUS_CODE_CLASS | 0x0000000D)
#define AMI_CHIPSET_EC_DXE_SB_ERROR (AMI_STATUS_CODE_CLASS | 0x0000000E)
#define AMI_CHIPSET_PC_PEI_CAR_NB2_INIT (AMI_STATUS_CODE_CLASS | 0x0000000F)
#define AMI_CHIPSET_PC_PEI_MEM_NB2_INIT (AMI_STATUS_CODE_CLASS | 0x00000010)
#define AMI_CHIPSET_PC_DXE_NB2_INIT (AMI_STATUS_CODE_CLASS | 0x00000011)

// Keyboard AMI-specific codes
#define AMI_P_KEYBOARD_EC_BUFFER_FULL (AMI_STATUS_CODE_CLASS | 0x00000000)

//PEI Core AMI-specific codes
#define AMI_PEI_CORE_EC_DXEIPL_NOT_FOUND (AMI_STATUS_CODE_CLASS | 0x00000000)

//PEI Services AMI-specific codes
#define AMI_PS_PC_RESET_SYSTEM (AMI_STATUS_CODE_CLASS | 0x00000000)
#define AMI_PS_EC_RESET_NOT_AVAILABLE (AMI_STATUS_CODE_CLASS | 0x00000001)
#define AMI_PS_EC_MEMORY_NOT_INSTALLED (AMI_STATUS_CODE_CLASS | 0x00000002)
#define AMI_PS_EC_MEMORY_INSTALLED_TWICE (AMI_STATUS_CODE_CLASS | 0x00000003)

//PEIM AMI-specific codes
#define AMI_PEIM_PC_S3_STARTED (AMI_STATUS_CODE_CLASS | 0x00000000)
#define AMI_PEIM_PC_S3_BOOT_SCRIPT (AMI_STATUS_CODE_CLASS | 0x00000001)
#define AMI_PEIM_PC_VIDEO_REPOST (AMI_STATUS_CODE_CLASS | 0x00000003)
#define AMI_PEIM_PC_OS_WAKE (AMI_STATUS_CODE_CLASS | 0x00000004)
#define AMI_PEIM_EC_S3_RESUME_PPI_NOT_FOUND (AMI_STATUS_CODE_CLASS | 0x00000005)
#define AMI_PEIM_EC_S3_BOOT_SCRIPT_ERROR (AMI_STATUS_CODE_CLASS | 0x00000006)
#define AMI_PEIM_EC_S3_OS_WAKE_ERROR (AMI_STATUS_CODE_CLASS | 0x00000007)
#define AMI_PEIM_EC_S3_RESUME_ERROR (AMI_STATUS_CODE_CLASS | 0x00000008)
#define AMI_PEIM_EC_RECOVERY_PPI_NOT_FOUND (AMI_STATUS_CODE_CLASS | 0x00000009)
#define AMI_PEIM_EC_RECOVERY_FAILED (AMI_STATUS_CODE_CLASS | 0x0000000A)
#define AMI_PEIM_EC_S3_RESUME_FAILED (AMI_STATUS_CODE_CLASS | 0x0000000B)

//DXE Core AMI-specific codes
#define AMI_DXE_CORE_PC_ARCH_READY (AMI_STATUS_CODE_CLASS | 0x00000000)
#define AMI_DXE_CORE_EC_NO_ARCH (AMI_STATUS_CODE_CLASS | 0x00000001)

//Boot Services AMI-specific codes
#define AMI_BS_PC_NVRAM_INIT (AMI_STATUS_CODE_CLASS | 0x00000000)
#define AMI_BS_PC_NVRAM_GC (AMI_STATUS_CODE_CLASS | 0x00000001)
#define AMI_BS_PC_CONFIG_RESET (AMI_STATUS_CODE_CLASS | 0x00000002)

//DXE Driver AMI-specific codes
#define AMI_DXE_BS_PC_ACPI_INIT (AMI_STATUS_CODE_CLASS | 0x00000000)
#define AMI_DXE_BS_PC_CSM_INIT (AMI_STATUS_CODE_CLASS | 0x00000001)
#define AMI_DXE_BS_EC_INVALID_PASSWORD (AMI_STATUS_CODE_CLASS | 0x00000002)
#define AMI_DXE_BS_EC_BOOT_OPTION_LOAD_ERROR (AMI_STATUS_CODE_CLASS | 0x00000003)
#define AMI_DXE_BS_EC_BOOT_OPTION_FAILED (AMI_STATUS_CODE_CLASS | 0x00000004)
#define AMI_DXE_BS_EC_INVALID_IDE_PASSWORD (AMI_STATUS_CODE_CLASS | 0x00000005)

#pragma pack(pop)
#endif
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2005, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             6145-F Northbelt Pkwy, Norcross, GA 30071            **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
