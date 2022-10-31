//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/TcgPlatformSetupPeiPolicy/TcgPlatformSetupPeiPolicy.h 2     12/18/11 10:25p Fredericko $
//
// $Revision: 2 $
//
// $Date: 12/18/11 10:25p $
//**********************************************************************
//<AMI_FHDR_START>
//---------------------------------------------------------------------------
// Name: TcgPlatformpeipolicy.h
//
// Description:	Header file for TcgPlatformpeipolicy
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef _TCG_PLATFORM_SETUP_PEI_POLICY_H_
#define _TCG_PLATFORM_SETUP_PEI_POLICY_H_

#include <Efi.h>
#include <token.h>
#include <Setup.h>
#include <Ppi\ReadOnlyVariable.h>



#define TCG_PLATFORM_SETUP_PEI_POLICY_GUID \
  { \
    0xa76b4e22, 0xb50a, 0x401d, 0x8b, 0x35, 0x51, 0x24, 0xb0, 0xba, 0x41, 0x4 \
  }

#define TCG_PPI_SYNC_FLAG_GUID \
  {\
    0xf3ed95df, 0x828e, 0x41c7, 0xbc, 0xa0, 0x16, 0xc4, 0x19, 0x65, 0xa6, 0x34 \
  }

#define PEI_TCG_INTERNAL_FLAGS_GUID \
  {\
    0x70fff0ff, 0xa543, 0x45b9, 0x8b, 0xe3, 0x1b, 0xdb, 0x90, 0x41, 0x20, 0x80 \
  }
  
//
// Protocol revision number
// Any backwards compatible changes to this protocol will result in an update in the revision number
// Major changes will require publication of a new protocol
//
#define TCG_PLATFORM_SETUP_PEI_PROTOCOL_REVISION_1 1

#pragma pack(1)
typedef struct {
  //
  // Byte 0, bit definition for functionality enable/disable
  //
  UINT8   TpmSupport;           // 0: Disabled; 1: Enabled
  UINT8   TcmSupport;           // 0: Disabled; 1: Enabled
  UINT8   TpmEnable;            // 0: Disabled; 1: Enabled
  UINT8   TpmAuthenticate;
  UINT8   TpmOperation;           // 0: Disabled; 1: Enabled
  UINT8   DisallowTpm;           // 0: Disabled; 1: Enabled
  UINT8   Reserved1;
  UINT8   Reserved2;

  //
  // Byte 1, bit definition for Status Information 
  //
  UINT8   TpmHardware;     // 0: Disabled; 1: Enabled
  UINT8   TpmEnaDisable;        
  UINT8   TpmActDeact;
  UINT8   TpmOwnedUnowned;      
  UINT8   TcgSupportEnabled;     // 0: Disabled; 1: Enabled
  UINT8   TpmError;   
  UINT8   PpiSetupSyncFlag;   
  UINT8   Reserved3;

  //
  // Byte 2, Reserved bytes
  //
  UINT8   Reserved4;

  //
  // Byte 3, Reserved bytes
  //
  UINT8   Reserved5;

} TCG_CONFIGURATION;

#pragma pack()


typedef
EFI_STATUS
(EFIAPI * GET_TCG_PEI_POLICY)(
    IN EFI_PEI_SERVICES     **PeiServices ,
    IN TCG_CONFIGURATION    *ConfigFlags
);


//
// AMT DXE Platform Policiy ====================================================
//
typedef struct _TCG_PLATFORM_SETUP_INTERFACE {
  UINT8                 Revision;
  GET_TCG_PEI_POLICY    getTcgPeiPolicy;
} TCG_PLATFORM_SETUP_INTERFACE;

extern EFI_GUID gTcgPeiPolicyGuid;
#endif

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
