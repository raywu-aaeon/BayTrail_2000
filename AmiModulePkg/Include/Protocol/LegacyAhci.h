//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/AHCI/INT13/protocol/LegacyAhci.h 5     2/10/11 10:50a Rameshr $
//
// $Revision: 5 $
//
// $Date: 2/10/11 10:50a $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:    LegacyAhci.h
//
// Description: AHCI INT13 initialization protocol
//
//<AMI_FHDR_END>
//**********************************************************************
#ifndef __LEGACY_AHCI_PROTOCOL_H__
#define __LEGACY_AHCI_PROTOCOL_H__
#ifdef __cplusplus
extern "C" {
#endif

//#define EFI_AHCI_INT13_INIT_PROTOCOL_GUID \
//  { 0x67820532, 0x7613, 0x4dd3, 0x9e, 0xd7, 0x3d, 0x9b, 0xe3, 0xa7, 0xda, 0x63 }

// the following ifndef is used when this .h file is included from EDK module...
#ifndef GUID_VARIABLE_DECLARATION
#define GUID_VARIABLE_DECLARATION(Variable, Guid) extern EFI_GUID Variable
#endif

//GUID_VARIABLE_DECLARATION(gAint13ProtocolGuid,EFI_AHCI_INT13_INIT_PROTOCOL_GUID);
extern EFI_GUID gAint13ProtocolGuid;

#ifndef GUID_VARIABLE_DEFINITION
#pragma pack (1)

typedef struct _EFI_AHCI_INT13_INIT_PROTOCOL EFI_AHCI_INT13_INIT_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_INIT_AHCI_INT13_SUPPORT) ();

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        EFI_AHCI_INT13_INIT_PROTOCOL
//
// Description: AHCI Int13 initialization protocol definition
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct _EFI_AHCI_INT13_INIT_PROTOCOL {
  EFI_INIT_AHCI_INT13_SUPPORT InitAhciInt13Support;
} EFI_AHCI_INT13_INIT_PROTOCOL;

#pragma pack ()

/****** DO NOT WRITE BELOW THIS LINE *******/
#endif // #ifndef GUID_VARIABLE_DEFINITION
#ifdef __cplusplus
}
#endif
#endif

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
