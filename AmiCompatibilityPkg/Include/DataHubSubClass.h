//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************


//**********************************************************************
// $Header: /Alaska/BIN/Core/Include/DataHubSubClass.h 1     10/10/05 4:26p Markw $
//
// $Revision: 1 $
//
// $Date: 10/10/05 4:26p $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name: DataHubSubClass.h
//
// Description:	DataHubSubClass definitions.
//
//<AMI_FHDR_END>
//**********************************************************************

#ifndef __DATAHUB_SUBCLASS_H__
#define __DATAHUB_SUBCLASS_H__

#include <Efi.h>

// Include files from EDKII
// IntelFrameworkPkg:
#include <Framework/FrameworkInternalFormRepresentation.h>
#include <Guid/DataHubRecords.h>
// MdePkg:
#include <Guid/StatusCodeDataTypeId.h>

#ifdef __cplusplus
extern "C" {
#endif

// Guid/DataHubRecords.h
/*
typedef struct {
	UINT32 Version;
	UINT32 HeaderSize;
	UINT16 Instance;
	UINT16 SubInstance;
	UINT32 RecordType;
} EFI_SUBCLASS_TYPE1_HEADER;
*/

// Guid/StatusCodeDataTypeId.h
/*
typedef struct {
	INT16 Value;
	INT16 Exponent;
} EFI_EXP_BASE10_DATA;
*/

// Guid/DataHubRecords.h
/*
typedef struct {
	UINT16 Value;
	UINT16 Exponent;
} EFI_EXP_BASE2_DATA;

typedef struct {
	EFI_GUID	ProducerName;
	UINT16		Instance;
	UINT16		SubInstance;
} EFI_INTER_LINK_DATA;
*/

// Framework/FrameworkInternalFormRepresentation.h
//typedef UINT16 STRING_REF;




/****** DO NOT WRITE BELOW THIS LINE *******/
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
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

