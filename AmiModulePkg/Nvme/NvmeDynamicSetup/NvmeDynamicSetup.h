//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2015, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

/** @file NvmeDynamicSetup.h
    Has macro definition.
     
**/


#ifndef _NVME_DYNAMIC_SETUP_H
#define _NVME_DYNAMIC_SETUP_H
#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------

#include <Token.h>

//---------------------------------------------------------------------------

#define NVME_FORM_SET_CLASS          0xD1
#define NVME_FORM_SET_GUID\
        { 0x5E39CF2E, 0x6712, 0x45AB, { 0x84, 0xC4, 0x35, 0xD3, 0xC6, 0xA3, 0x68, 0x6D } }

#define NVME_MAIN_FORM_ID            0x1
#define NVME_SUB_FORM_ID             0x2

#define NVME_MAIN_FORM_LABEL_START   0x1000
#define NVME_MAIN_FORM_LABEL_END     0x1001
#define NVME_SUB_FORM_LABEL_START    0x2000
#define NVME_SUB_FORM_LABEL_END      0x2001

#ifdef __cplusplus
}
#endif
#endif

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2015, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
