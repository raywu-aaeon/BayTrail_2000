//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

/** @file
  Definition of the AmiCriticalSectionLib library class.
*/

#ifndef __AMI_CRITICAL_SECTION_LIB__H__
#define __AMI_CRITICAL_SECTION_LIB__H__
#ifdef __cplusplus
extern "C" {
#endif

typedef VOID* CRITICAL_SECTION;

EFI_STATUS CreateCriticalSection(OUT CRITICAL_SECTION *);
EFI_STATUS BeginCriticalSection(IN CRITICAL_SECTION);
EFI_STATUS EndCriticalSection(IN CRITICAL_SECTION);
EFI_STATUS DestroyCriticalSection(IN CRITICAL_SECTION);

/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif
#endif
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
