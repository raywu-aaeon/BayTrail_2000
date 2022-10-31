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

//**********************************************************************
// $Header: /AptioV/Source/Modules/Ofbd/OemSetup/OemSetup.c $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	OemSetup.c
//
// Description: 
// Basically this function is not always necessary for all projects, because there is a more direct method to achieve
// the same target. Two E-Links exposed by SMIFlash module, SMIFlashPreUpdateList and SMIFlashEndUpdateList, are made
// for doing the same thing as this OFBD module.
//
// We prefer you make use of E-Links than this OFBD module. The only advantage of this OFBD module is you don't have
// to link your code with SMIFlash module.If DMI data is stored in the NVRAM, you have two ways to preserve DMI data
// when updating NVRAM.
//
// 1. Controlled by SMBIOS module: SMBIOS module can set SMBIOS_PRESERVE_NVRAM token to preserves DMI data through
// e-Link PreserveDmiEditData and RestoreDmiEditData.
//
// 2. Controlled By Flash utility: AFU will call into OEMSETUP.c only when user issues /r or /sp command. When
// SMBIOS_PRESERVE_NVRAM token is disable, OEM enginner can set OEMSETUP_SUPPORT token to handle the duty of DMI data
// preservation through OEM NVRAM/Setup Variable Preserve module. Thus user can decide to clear DMI data or not.
// In this case, /n parameter will clear NVRAM, and /r parameter is required to preserve DMI data in NVRAM. When /n is
// triggered, without /r all DMI data in NVRAM will be cleared. 
//
//<AMI_FHDR_END>
//**********************************************************************
#include <Efi.h>
#include <Token.h>
#include <AmiLib.h>
#include <AmiDxeLib.h>
#include <Protocol/AmiSmbios.h>
#include "OemSetup.h"
#include "../Ofbd.h"

#if (SMBIOS_DMIEDIT_DATA_LOC == 2) && (SMBIOS_PRESERVE_NVRAM != 1)

#define DefaultVariableSize     100

#pragma pack(1)
typedef struct {
    CHAR16      *BootVarName;
    EFI_GUID    Guid;
    UINT32      Attrib;
    UINTN       Size;
    UINT8       *Data;
    VOID        *NextData;
} SAVED_VAR;
#pragma pack()

VOID PreserveDmiEditVariables (VOID);
VOID RestoreDmiEditVariables (VOID);

SAVED_VAR   *gRestoreVarList = NULL;
#endif

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	OFBDSETUPSupportHandle
//
// Description:	OFBD SETUP Store/Restore Support Handle
//
// Input:
//      IN OUT OFBD_HDR *pOFBDHdr
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
OFBDSETUPSupportHandle( 
    IN OUT OFBD_HDR *pOFBDHdr)
{
    EFI_STATUS Status = EFI_SUCCESS;

#if OEM_SETUP_SAMPLE_TEST
	OFBD_TC_56_SETUP_STRUCT *SETUPStructPtr;     
	SETUPStructPtr = (OFBD_TC_56_SETUP_STRUCT *)((UINT8 *)pOFBDHdr + pOFBDHdr->OFBD_HDR_SIZE + sizeof(OFBD_EXT_HDR)); 
	SETUPStructPtr->dbRetSts =  OFBD_TC_SETUP_OK;
#endif

    return(Status);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	OFBDSETUPStoreHandle
//
// Description:	OFBD SETUP Store Handle
//
// Input:
//      IN OUT OFBD_HDR *pOFBDHdr
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
OFBDSETUPStoreHandle(
    IN OUT OFBD_HDR *pOFBDHdr)
{
    EFI_STATUS Status = EFI_SUCCESS;

#if OEM_SETUP_SAMPLE_TEST || OEM_DMI_SAMPLE_TEST
    OFBD_TC_56_SETUP_STRUCT *SETUPStructPtr;     
	SETUPStructPtr = (OFBD_TC_56_SETUP_STRUCT *)((UINT8 *)pOFBDHdr + pOFBDHdr->OFBD_HDR_SIZE + sizeof(OFBD_EXT_HDR)); 

    //OEM add ...
    
	if (!EFI_ERROR(Status))
		SETUPStructPtr->dbRetSts =  OFBD_TC_SETUP_OK;
	else
    	SETUPStructPtr->dbRetSts =  OFBD_TC_SETUP_STORE_ERROR;
#endif

    return(Status);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	OFBDSETUPRestoreHandle
//
// Description:	OFBD SETUP Restore Handle
//
// Input:
//      IN OUT OFBD_HDR *pOFBDHdr
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
OFBDSETUPRestoreHandle(
    IN OUT OFBD_HDR *pOFBDHdr)
{
    EFI_STATUS Status = EFI_SUCCESS;

#if OEM_SETUP_SAMPLE_TEST
    OFBD_TC_56_SETUP_STRUCT *SETUPStructPtr;     
	SETUPStructPtr = (OFBD_TC_56_SETUP_STRUCT *)((UINT8 *)pOFBDHdr + pOFBDHdr->OFBD_HDR_SIZE + sizeof(OFBD_EXT_HDR)); 

    //OEM add ...

    if (!EFI_ERROR(Status))
		SETUPStructPtr->dbRetSts =  OFBD_TC_SETUP_OK;
	else
    	SETUPStructPtr->dbRetSts =  OFBD_TC_SETUP_RESTORE_ERROR;
#endif

    return(Status);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	OFBDDMIStoreHandle
//
// Description:	OFBD DMI Data Store Handle
//
// Input:
//      IN OUT OFBD_HDR *pOFBDHdr
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
OFBDDMIStoreHandle(
    IN OUT OFBD_HDR *pOFBDHdr)
{
    EFI_STATUS Status = EFI_SUCCESS;

#if OEM_DMI_SAMPLE_TEST
    OFBD_TC_56_SETUP_STRUCT *SETUPStructPtr;     
	SETUPStructPtr = (OFBD_TC_56_SETUP_STRUCT *)((UINT8 *)pOFBDHdr + pOFBDHdr->OFBD_HDR_SIZE + sizeof(OFBD_EXT_HDR)); 

    // OEM add ...
    // Store the DMI data
#if (SMBIOS_DMIEDIT_DATA_LOC == 2) && (SMBIOS_PRESERVE_NVRAM != 1)
    PreserveDmiEditVariables ();
#endif
    
	if (!EFI_ERROR(Status))
		SETUPStructPtr->dbRetSts =  OFBD_TC_SETUP_OK;
	else
    	SETUPStructPtr->dbRetSts =  OFBD_TC_DMI_STORE_ERROR;
#endif

    return(Status);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	OFBDDMIRestoreHandle
//
// Description:	OFBD DMI Data Restore Handle
//
// Input:
//      IN OUT OFBD_HDR *pOFBDHdr
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
OFBDDMIRestoreHandle(
    IN OUT OFBD_HDR *pOFBDHdr)
{
    EFI_STATUS Status = EFI_SUCCESS;

#if OEM_DMI_SAMPLE_TEST
    OFBD_TC_56_SETUP_STRUCT *SETUPStructPtr;     
	SETUPStructPtr = (OFBD_TC_56_SETUP_STRUCT *)((UINT8 *)pOFBDHdr + pOFBDHdr->OFBD_HDR_SIZE + sizeof(OFBD_EXT_HDR)); 

    // OEM add ...
    // Restore the DMI data
#if (SMBIOS_DMIEDIT_DATA_LOC == 2) && (SMBIOS_PRESERVE_NVRAM != 1)
    RestoreDmiEditVariables ();
#endif

    if (!EFI_ERROR(Status))
		SETUPStructPtr->dbRetSts =  OFBD_TC_SETUP_OK;
	else
    	SETUPStructPtr->dbRetSts =  OFBD_TC_DMI_RESTORE_ERROR;
#endif

    return(Status);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	OEMSETUPEntry
//
// Description:	OFBD NVRAM/SETUP Store/Restore Entry point
//
// Input:
//      IN VOID             *Buffer
//      IN OUT UINT8        *pOFBDDataHandled
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID OEMSETUPEntry (
    IN VOID             *Buffer,
    IN OUT UINT8        *pOFBDDataHandled )
{
    OFBD_HDR *pOFBDHdr;
    OFBD_EXT_HDR *pOFBDExtHdr; 
    VOID *pOFBDTblEnd;
    OFBD_TC_56_SETUP_STRUCT *SETUPStructPtr;    

    if(*pOFBDDataHandled == 0)
    {
        pOFBDHdr = (OFBD_HDR *)Buffer;
        pOFBDExtHdr = (OFBD_EXT_HDR *)((UINT8 *)Buffer + (pOFBDHdr->OFBD_HDR_SIZE));
        SETUPStructPtr = (OFBD_TC_56_SETUP_STRUCT *)((UINT8 *)pOFBDExtHdr + sizeof(OFBD_EXT_HDR)); 
        pOFBDTblEnd = (VOID *)((UINT8 *)Buffer + (pOFBDHdr->OFBD_Size));    
        
        if(pOFBDHdr->OFBD_FS & OFBD_FS_SETUP)
        {   
            //Check Type Code ID
            if(pOFBDExtHdr->TypeCodeID == OFBD_EXT_TC_SETUP)
            {  
                //IoWrite32(0x300, *(UINT32 *)((UINT8 *)pOFBDTblEnd -4)); //debug
				if(SETUPStructPtr->dbGetSts == OFBD_TC_SETUP_SUPPORT)
				{
					if(OFBDSETUPSupportHandle(pOFBDHdr) == EFI_SUCCESS)
					{
						//OEM SETUP SUPPORT Handled.
                    	*pOFBDDataHandled = 0xFF;      
                    	return;
                	}
				}
				else if(SETUPStructPtr->dbGetSts == OFBD_TC_SETUP_STORE)
				{
					if(OFBDSETUPStoreHandle(pOFBDHdr) == EFI_SUCCESS)
					{
						//OEM SETUP STORE Handled.
                    	*pOFBDDataHandled = 0xFF;      
                    	return;
                	}
				}
                else if(SETUPStructPtr->dbGetSts == OFBD_TC_SETUP_RESTORE)
				{
					if(OFBDSETUPRestoreHandle(pOFBDHdr) == EFI_SUCCESS)
					{
						//OEM SETUP RESTORE Handled.
                    	*pOFBDDataHandled = 0xFF;      
                    	return;
                	}
				}
				else if(SETUPStructPtr->dbGetSts == OFBD_TC_DMI_STORE)
				{
					if(OFBDDMIStoreHandle(pOFBDHdr) == EFI_SUCCESS)
					{
						//OEM DMI STORE Handled.
                    	*pOFBDDataHandled = 0xFF;      
                    	return;
                	}
				}
                else if(SETUPStructPtr->dbGetSts == OFBD_TC_DMI_RESTORE)
				{
					if(OFBDDMIRestoreHandle(pOFBDHdr) == EFI_SUCCESS)
					{
						//OEM DMI RESTORE Handled.
                    	*pOFBDDataHandled = 0xFF;      
                    	return;
                	}
				}
            } 
            //Error occured
            *pOFBDDataHandled = 0xFE;          
            return;
        }  
    }

    return;
}

//
// Reference from "AmiModulePkg\SmiFlash\SMIFlashLinks.c" file code +>>>
//
// SS: AptioV;$/AptioV/Source/Modules/SmiFlash;
//
// Label 5.001_SmiFlash_05
//   
#if (SMBIOS_DMIEDIT_DATA_LOC == 2) && (SMBIOS_PRESERVE_NVRAM != 1)
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   PreserveDmiEditVariables
//
// Description: Preserve all variables that GUID is gAmiSmbiosNvramGuid.
//
// Input:       NONE
//
// Output:      NONE
//
// Returns:     NONE
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID PreserveDmiEditVariables(VOID)
{
    UINTN               VarNameSize = 2, VariableSize, Result;
    CHAR16              *VarName = NULL, *OldVarName = NULL;
    UINT8               *VariableData, *pData;
    UINT32              VariableAttr;
    EFI_GUID            VarGuid;
    EFI_STATUS          Status;
    SAVED_VAR           *pLastVar = NULL;

    Status = pSmst->SmmAllocatePool (
        EfiRuntimeServicesData,
        VarNameSize, &VarName
    );

    if(EFI_ERROR(Status))
        return;

    // Get frist variable
    VarName[0] = 0;
    do{
        if(VarNameSize != 0)
            Status = pRS->GetNextVariableName (&VarNameSize,
                        VarName, &VarGuid);
        else
            Status = EFI_BUFFER_TOO_SMALL;

        if(Status == EFI_BUFFER_TOO_SMALL)
        {
            // If VarNameSize is 0, VarName is not existed.
            if(VarNameSize != 0)
            {
                // Keep the old variable name for next search
                OldVarName = VarName;
            }

            // Avoid dead loop
            VarNameSize = DefaultVariableSize;

            Status = pSmst->SmmAllocatePool (
                          EfiRuntimeServicesData,
                          VarNameSize, &VarName);

            if(EFI_ERROR(Status))
                return;

            MemCpy(VarName, OldVarName, VarNameSize);

            continue;
        }
        if(!EFI_ERROR(Status))
        {
            // Check gAmiSmbiosNvramGuid.
            Result = guidcmp(&gAmiSmbiosNvramGuid, &VarGuid);

            if (Result != 0)
            {
                // Make sure every time, it gets a buffer too small,
                // don't waste this buffer
                VarNameSize = DefaultVariableSize;
                continue;
            }

            VariableSize = 0;
            Status = pRS->GetVariable(VarName, &VarGuid,
                                      &VariableAttr, &VariableSize, NULL);

            if(Status != EFI_BUFFER_TOO_SMALL)
                continue;

            Status = pSmst->SmmAllocatePool (
                          EfiRuntimeServicesData,
                          VariableSize, &VariableData);

            if(EFI_ERROR(Status))
                continue;

            Status = pRS->GetVariable(VarName, &VarGuid, \
                            &VariableAttr, &VariableSize, \
                            VariableData);

            if(EFI_ERROR(Status))
            {
                pSmst->SmmFreePool(VariableData);
                continue;
            }
            
            // Store variable in Link list
            Status = pSmst->SmmAllocatePool (
                          EfiRuntimeServicesData,
                          sizeof(SAVED_VAR), &pData);

            if(EFI_ERROR(Status))
                continue;

            Status = pSmst->SmmAllocatePool (
                          EfiRuntimeServicesData,
                          VarNameSize, &OldVarName);

            if(EFI_ERROR(Status))
                continue;

            MemCpy(OldVarName, VarName, VarNameSize);
            // Free the bigger Varname
            pSmst->SmmFreePool(VarName);

            MemCpy(&((SAVED_VAR*)pData)->Guid, &VarGuid, sizeof(EFI_GUID));

            ((SAVED_VAR*)pData)->BootVarName = OldVarName;
            ((SAVED_VAR*)pData)->Attrib = VariableAttr;
            ((SAVED_VAR*)pData)->Size = VariableSize;
            ((SAVED_VAR*)pData)->Data = VariableData;
            ((SAVED_VAR*)pData)->NextData = NULL;

            // Init Variable List
            if (gRestoreVarList == NULL) {
                gRestoreVarList = (SAVED_VAR*)pData;
                pLastVar = NULL;
            }    
            if (pLastVar != NULL) pLastVar->NextData = (VOID*)pData;
            pLastVar = (SAVED_VAR*)pData;

            // Make sure every time, it gets a buffer too small,
            // make it allocate new buffer for name
            VarNameSize = 0;
        }

    }while(Status != EFI_NOT_FOUND);
}
//<AMI_PHDR_START>
//---------------------------------------------------------------------
// Procedure:   RestoreDmiEditVariables
//
// Description: Restore all variables that GUID is gAmiSmbiosNvramGuid.
//
// Input:       NONE
//
// Output:      NONE
//
// Returns:     NONE
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID RestoreDmiEditVariables (VOID)
{
    SAVED_VAR           *TempPoint;
    EFI_STATUS          Status;

    while (gRestoreVarList != NULL) {
    
        if (gRestoreVarList != NULL) {
            TempPoint = gRestoreVarList;
            gRestoreVarList = TempPoint->NextData;
        } else return;          // This condition should not happen.

        Status = pRS->SetVariable (  TempPoint->BootVarName,
                                     &TempPoint->Guid,
                                     TempPoint->Attrib,
                                     TempPoint->Size,
                                     TempPoint->Data );
        pSmst->SmmFreePool(TempPoint->BootVarName);
        pSmst->SmmFreePool(TempPoint->Data);
        pSmst->SmmFreePool(TempPoint);
    }    
}
#endif
//
// <<<+
//

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
