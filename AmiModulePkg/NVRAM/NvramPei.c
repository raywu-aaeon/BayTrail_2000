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
// $Header: /Alaska/SOURCE/Core/Modules/NVRAM/NVRAMPEI.c 18    1/04/13 10:35a Felixp $
//
// $Revision: 18 $
//
// $Date: 1/04/13 10:35a $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	NVRAMPEI.h
//
// Description:	
//
//<AMI_FHDR_END>
//**********************************************************************
#include <Ppi/ReadOnlyVariable.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <AmiPeiLib.h>
#include <Token.h>
#include "NVRAM.h"

//Defined in CSPLib(OEMPort.c)
BOOLEAN IsNvramDataCompatible(
    IN EFI_PEI_SERVICES **PeiServices,
    IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI *ReadVariablePpi
);

//Defined in Tokens.c
extern UINTN NvramAddress;
extern UINTN NvramBackupAddress;
extern const UINTN NvramSize;
extern const BOOLEAN NvSimulation;

typedef struct{
    EFI_PEI_READ_ONLY_VARIABLE2_PPI Ppi;
    UINT32 InfoCount;
    UINT32 LastInfoIndex;
    NVRAM_STORE_INFO NvramInfo[3];
    NVRAM_STORE_INFO *MainInfo;
} VARIABLE_PPI;

//--- GetVariable and GetNextVarName Hooks ------
//============================================================================
// Type definitions
typedef EFI_STATUS (PEI_HOOK_GET_VARIABLE)(
    IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI *This,
    IN CONST CHAR16 *VariableName, IN CONST EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes, IN OUT UINTN *DataSize, OUT VOID *Data
);

typedef EFI_STATUS (PEI_HOOK_GET_NEXT_VARIABLE_NAME)(
    IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI *This,
    IN OUT UINTN *VariableNameSize,
    IN OUT CHAR16 *VariableName, IN OUT EFI_GUID *VendorGuid
);

//======================================================================
// Global extern variables

extern PEI_HOOK_GET_VARIABLE PEI_GET_VAR_LIST EndOfGetVariableHook;
extern PEI_HOOK_GET_NEXT_VARIABLE_NAME PEI_GET_NEXT_VAR_NAME_LIST EndOfGetNextVarNameHook;

PEI_HOOK_GET_VARIABLE* PeiGetVariableHookList[]=
    {PEI_GET_VAR_LIST NULL};
PEI_HOOK_GET_NEXT_VARIABLE_NAME* PeiGetNextVarNameHookList[]=
    {PEI_GET_NEXT_VAR_NAME_LIST NULL};

EFI_STATUS PeiGetVariableHook(
    IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI *This,
    IN CONST CHAR16 *VariableName, IN CONST EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes, IN OUT UINTN *DataSize, OUT VOID *Data
){
    UINTN i;
    EFI_STATUS Result = EFI_UNSUPPORTED;
    for(i=0; PeiGetVariableHookList[i] && (Result == EFI_UNSUPPORTED); i++) 
        Result = PeiGetVariableHookList[i](This,VariableName, VendorGuid,Attributes, DataSize, Data);
    return Result;
}

EFI_STATUS PeiGetNextVarNameHook(
    IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI *This,
    IN OUT UINTN *VariableNameSize,
    IN OUT CHAR16 *VariableName, IN OUT EFI_GUID *VendorGuid
){
    UINTN i;
    EFI_STATUS Result = EFI_UNSUPPORTED;
    for(i=0; PeiGetNextVarNameHookList[i] && (Result == EFI_UNSUPPORTED); i++) 
        Result = PeiGetNextVarNameHookList [i](This,VariableNameSize,VariableName, VendorGuid);
    return Result;
}
//---GetVariable and GetNextVarName Hooks END------

//NVRAM mode detection hooks
typedef BOOLEAN (NVRAM_MODE_DETECTION_FUNCTION)(
    IN EFI_PEI_SERVICES **PeiServices,
    IN CONST VOID *ReadVariablePpi
);

extern NVRAM_MODE_DETECTION_FUNCTION IS_MFG_MODE_LIST EndOfNvramModeFuncList;
extern NVRAM_MODE_DETECTION_FUNCTION IS_RESET_CONFIG_MODE_LIST EndOfNvramModeFuncList;
extern NVRAM_MODE_DETECTION_FUNCTION IS_DEFAULT_CONFIG_MODE_LIST EndOfNvramModeFuncList;

NVRAM_MODE_DETECTION_FUNCTION* IsMfgModeList[] = {IS_MFG_MODE_LIST NULL};
NVRAM_MODE_DETECTION_FUNCTION* IsResetConfigModeList[] = {IS_RESET_CONFIG_MODE_LIST NULL};
NVRAM_MODE_DETECTION_FUNCTION* IsDefaultConfigModeList[] = {IS_DEFAULT_CONFIG_MODE_LIST NULL};

//<AMI_PHDR_START>
//**********************************************************************
//
// Procedure: CallTwoParamHooks
//
// Description: 
//  This function calls a list of function defined at build time and 
//  return whether the functions completed successfully or not
//
// Input:
//  IN EFI_PEI_SERVICES **PeiServices - pointer to the PeiServices Table
//  IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI *ReadVariablePpi - pointer to 
//      the Read Only Variable 2 PPI
//
// Output:
//  BOOLEAN 
//  - - TRUE - all functions returned correctly
//  - - FALSE - one of the functions failed during execution
//
// Notes:
//  Similar to CallOneParamHooks except that these function also require
//  a pointer to the Read Only Variable 2 PPI
//          
//**********************************************************************
//<AMI_PHDR_END>
BOOLEAN CallNvramModeDetectionHooks(
    IN NVRAM_MODE_DETECTION_FUNCTION* FunctionList[],
    IN EFI_PEI_SERVICES **PeiServices, 
    IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI *ReadVariablePpi
){
    UINTN i;
    BOOLEAN Result = FALSE;
    for(i=0; FunctionList[i] && !Result; i++) 
        Result = FunctionList[i](PeiServices,ReadVariablePpi);
    return Result;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: IsMfgMode
//
// Description: 
//  This function determines if the system in in manufacturing mode.
//  This function is called by NVRAM code. 
//  If system is in manufacturing mode, manufacturing values for NVRAM variables 
//  are used.
//
// Input:   
//  **PeiServices - pointer to a pointer to the PEI Services Table. 
//  *ReadVariablePpi - pointer to EFI_PEI_READ_ONLY_VARIABLE2_PPI PPI. The pointer 
//                     can be used to read and enumerate existing NVRAM variables.
//
// Output:    
//  TRUE - Manufacturing mode is detected
//  FALSE - Manufacturing mode is not detected
//
// Notes: 
//  This routine is called very early, prior to SBPEI and NBPEI
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsMfgMode(
    IN EFI_PEI_SERVICES **PeiServices, 
    IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI *ReadVariablePpi
){
#if MANUFACTURING_MODE_SUPPORT
    //You can use IsMfgMode eLink to install customer handlers
    //or just add the code here
    return CallNvramModeDetectionHooks(IsMfgModeList, PeiServices, ReadVariablePpi);
#else
    //Do not change this.
    //This is needed to be able to disable manufacturing mode support using SDL token.
    return FALSE;
#endif
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: IsResetConfigMode
//
// Description: 
//  This function determines if the system configuration needs to be reset.
//  This function is called by NVRAM code. 
//
// Input:   
//  **PeiServices - pointer to a pointer to the PEI Services Table. 
//  *ReadVariablePpi - pointer to EFI_PEI_READ_ONLY_VARIABLE2_PPI PPI. The 
//                     pointer can be used to read and enumerate existing NVRAM 
//                     variables.
//
// Output:    
//  TRUE - Reset Configuration condition is detected
//  FALSE - Reset Configuration condition is not detected
//
// Notes: This routine is called very early, prior to SBPEI and NBPEI
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsResetConfigMode(
    IN EFI_PEI_SERVICES **PeiServices, 
    IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI *ReadVariablePpi
){
    //You can use IsResetConfigMode eLink to install customer handlers
    //or just add the code here
    return CallNvramModeDetectionHooks(
        IsResetConfigModeList, PeiServices, ReadVariablePpi
    );
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: IsDefaultConfigMode
//
// Description: 
//  This function determines if the system should boot with the default configuration.
//  This function is called by NVRAM code. 
//  If boot with default configuration is detected, default values for NVRAM 
//  variables are used.
//
// Input:   
//  **PeiServices - pointer to a pointer to the PEI Services Table. 
//  *ReadVariablePpi - pointer to EFI_PEI_READ_ONLY_VARIABLE2_PPI PPI. The pointer 
//                     can be used to read and enumerate existing NVRAM variables.
//
// Output:    
//  TRUE - Boot with default configuration is detected
//  FALSE - Boot with default configuration is not detected
//
// Notes: 
//  This routine is callsed very early, prior to SBPEI and NBPEI
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsDefaultConfigMode(
    IN EFI_PEI_SERVICES **PeiServices, 
    IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI *ReadVariablePpi
){
    //You can use IsDefaultConfigMode eLink to install customer handlers
    //or just add the code here
    return CallNvramModeDetectionHooks(
        IsDefaultConfigModeList, PeiServices, ReadVariablePpi
    );
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   VarPpiGetVariable
//
// Description: This function searches for a Variable with specific GUID and Name
//
// Input:       IN EFI_PEI_SERVICES **PeiServices - Double pointer to Pei Services instance
//              IN CHAR16 *VariableName - pointer to Var Name in Unicode
//              IN EFI_GUID *VendorGuid - pointer to Var GUID
//              OPTIONAL OUT UINT32* Attributes - Pointer to memory where Attributes will be returned 
//              IN OUT UINTN *DataSize - size of Var - if smaller than actual EFI_BUFFER_TOO_SMALL 
//              will be returned and DataSize will be set to actual size needed
//              OUT VOID *Data - Pointer to memory where Var will be returned
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS VarPpiGetVariable(
	IN EFI_PEI_SERVICES **PeiServices,
	IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
	OUT UINT32 *Attributes OPTIONAL,
	IN OUT UINTN *DataSize, OUT VOID *Data
)
{
	return PeiGetVariable(
        PeiServices, VariableName, VendorGuid, Attributes, DataSize, Data
    );
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   VarPpiGetNextVariableName
//
// Description: This function searches for Var folowed by the Var with specific name (optional) 
//              and GUID and returns it's Name. 
//
// Input:       IN EFI_PEI_SERVICES **PeiServices - Double pointer to Pei Services instance
//              IN OUT UINTN *VariableNameSize - size of Varname - if smaller than actual EFI_BUFFER_TOO_SMALL 
//              will be returned and DataSize will be set to actual size needed
//              IN OUT CHAR16 *VariableName - pointer where Var Name in Unicode will be stored
//              IN OUT EFI_GUID *VendorGuid - pointer to menory where Var GUID is stored
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS VarPpiGetNextVariableName(
	IN EFI_PEI_SERVICES **PeiServices,
	IN OUT UINTN *VariableNameSize,
	IN OUT CHAR16 *VariableName, IN OUT EFI_GUID *VendorGuid
)
{
	return PeiGetNextVariableName(
        PeiServices, VariableNameSize, VariableName, VendorGuid
    );
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   VarPpi2GetVariable
//
// Description: This function searches for Var with specific GUID
//
// Input:       IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI *This - pointer to FI_PEI_READ_ONLY_VARIABLE2_PPI
//              IN CHAR16 *VariableName - pointer to Var Name in Unicode
//              IN EFI_GUID *VendorGuid - pointer to Var GUID
//              OPTIONAL OUT UINT32* Attributes - Pointer to memory where Attributes will be returned 
//              IN OUT UINTN *DataSize - size of Var - if smaller than actual EFI_BUFFER_TOO_SMALL 
//              will be returned and DataSize will be set to actual size needed
//              OUT VOID *Data - Pointer to memory where Var will be returned
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS VarPpi2GetVariable(
    IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI *This,
    IN CONST CHAR16 *VariableName, IN CONST EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes, IN OUT UINTN *DataSize, OUT VOID *Data
)
{
	EFI_STATUS Status;
    VARIABLE_PPI *VarPpi = (VARIABLE_PPI*)This;
    Status = PeiGetVariableHook (
                This, VariableName, VendorGuid, Attributes, DataSize, Data
             );
    if (Status != EFI_UNSUPPORTED) return Status;
	Status = NvGetVariable2(
				(CHAR16*)VariableName, (EFI_GUID*)VendorGuid, Attributes,
				DataSize, Data, VarPpi->InfoCount, VarPpi->NvramInfo
			 );
	return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NvGetNextVariableName2
//
// Description: This function searches for Var with specific GUID and Enty number 
//              not betwin LastInfoIndex - InfoCount parameter and returns it's Name. 
//
// Input:       IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI *This - pointer to FI_PEI_READ_ONLY_VARIABLE2_PPI
//              IN OUT UINTN *VariableNameSize - size of Varname - if smaller than actual EFI_BUFFER_TOO_SMALL 
//              will be returned and DataSize will be set to actual size needed
//              IN OUT CHAR16 *VariableName - pointer where Var Name in Unicode will be stored
//              IN OUT EFI_GUID *VendorGuid - pointer to menory where Var GUID will be stored
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS VarPpi2GetNextVariableName(
    IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI *This,
    IN OUT UINTN *VariableNameSize,
    IN OUT CHAR16 *VariableName, IN OUT EFI_GUID *VendorGuid
)
{
	EFI_STATUS Status;
    VARIABLE_PPI *VarPpi = (VARIABLE_PPI*)This;
    Status = PeiGetNextVarNameHook (
                This, VariableNameSize, VariableName, VendorGuid
             );
    if (Status != EFI_UNSUPPORTED) return Status;
    Status = NvGetNextVariableName2(
                 VariableNameSize, VariableName, VendorGuid,
                 VarPpi->InfoCount, VarPpi->NvramInfo,
                 &(VarPpi->LastInfoIndex),FALSE
		   );
	return Status;
}

// PPI interface definition
EFI_PEI_READ_ONLY_VARIABLE_PPI VariablePpi = {VarPpiGetVariable, VarPpiGetNextVariableName};
EFI_PEI_READ_ONLY_VARIABLE2_PPI Variable2Ppi = {VarPpi2GetVariable, VarPpi2GetNextVariableName};

// PPI to be installed
EFI_PEI_PPI_DESCRIPTOR VariablePpiListTemplate[] =
{ 
    {
        EFI_PEI_PPI_DESCRIPTOR_PPI,
	    &gEfiPeiReadOnlyVariable2PpiGuid, &Variable2Ppi
    },
    {
        EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
	    &gEfiPeiReadOnlyVariablePpiGuid, &VariablePpi
    }
};

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PeiInitNV
//
// Description: This function inits NVRAM
//
// Input:       IN EFI_FFS_FILE_HEADER *FfsHeader - pointer to FfsHeader
//              IN EFI_PEI_SERVICES **PeiServices - double pointer to the Pei Sevices structure
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
EFIAPI
PeiInitNVHelper (
    IN       EFI_PEI_FILE_HANDLE  FileHandle,
    IN CONST EFI_PEI_SERVICES     **PeiServices
)
{
    EFI_STATUS Status;
    EFI_PEI_PPI_DESCRIPTOR *VariablePpiList;
    VARIABLE_PPI *VarPpi;
    UINT32 NvramMode=0;
    BOOLEAN ResetConfigMode;
    NVRAM_STORE_INFO MainNvram;
    VARIABLE_PPI TmpVarPpi;
    VOID *BackupNvramAddress = (VOID*)NvramBackupAddress;
    BOOLEAN NvramIsCorrupted = FALSE;
#if !NV_SIMULATION_NO_FLASH_READS
    NVRAM_HOB *pHob;
    BOOLEAN BackupStoreValid;
#endif

    Status = (*PeiServices)->AllocatePool(PeiServices, sizeof(VariablePpiListTemplate)+sizeof(VARIABLE_PPI), &VariablePpiList);
    if (EFI_ERROR(Status)) return Status;
    VariablePpiList[0]=VariablePpiListTemplate[0];
    VariablePpiList[1]=VariablePpiListTemplate[1];
    VarPpi = (VARIABLE_PPI*)(VariablePpiList + 2);
    VariablePpiList[0].Ppi = VarPpi;
    VarPpi->Ppi = Variable2Ppi;
    VarPpi->InfoCount = 0;
    VarPpi->LastInfoIndex = 0;
    MainNvram.NvramAddress = (UINT8*)NvramAddress;
    MainNvram.NvramSize = NvramSize;
#if NV_SIMULATION_NO_FLASH_READS    
    NvramIsCorrupted = TRUE;
#else
    if (!IsMainNvramStoreValid(&MainNvram, BackupNvramAddress,&BackupStoreValid)){
        if (BackupStoreValid){
            VOID *Tmp = BackupNvramAddress;
            BackupNvramAddress = MainNvram.NvramAddress;
            MainNvram.NvramAddress = Tmp;
        }else{
            NvramIsCorrupted = TRUE;
        }
    }
    NvInitInfoBuffer(
        &MainNvram,
        NvramHeaderLength,
        NVRAM_STORE_FLAG_NON_VALATILE
    );
    //Check if FV signature is valid
    NvramIsCorrupted =   NvramIsCorrupted 
                      || ((EFI_FIRMWARE_VOLUME_HEADER*)MainNvram.NvramAddress)->Signature!=FV_SIGNATURE;
#endif
    if (NvramIsCorrupted){
        VarPpi->MainInfo = NULL;
        NvramMode=NVRAM_MODE_RESET_CONFIGURATION;
#if NV_SIMULATION_NO_FLASH_READS
        PEI_TRACE((-1, PeiServices, "PEI: NVRAM PEIM is working in sumulation mode.\n"));
#else
        PEI_TRACE((-1, PeiServices, "PEI: NVRAM header corruption is detected\n"));
#endif        
    }else{
        TmpVarPpi = *VarPpi;
        TmpVarPpi.NvramInfo[0]=MainNvram;
        TmpVarPpi.MainInfo = &TmpVarPpi.NvramInfo[0];
        TmpVarPpi.InfoCount=1;
    
        if (   IsMfgMode(PeiServices,&TmpVarPpi.Ppi)
            && NvGetDefaultsInfo(
                   MfgDefaults,&MainNvram,&VarPpi->NvramInfo[VarPpi->InfoCount]
               ) != NULL
        ){
            VarPpi->InfoCount++;
            NvramMode|=NVRAM_MODE_MANUFACTORING;
        }
        ResetConfigMode = IsResetConfigMode(PeiServices,&TmpVarPpi.Ppi);
        if (!ResetConfigMode && !IsDefaultConfigMode(PeiServices,&TmpVarPpi.Ppi)){
            VarPpi->NvramInfo[VarPpi->InfoCount]=MainNvram;
            VarPpi->MainInfo = &VarPpi->NvramInfo[VarPpi->InfoCount];
            VarPpi->InfoCount++;
            if (NvGetDefaultsInfo(
                    StdDefaults,&MainNvram,&VarPpi->NvramInfo[VarPpi->InfoCount]
                ) != NULL
            ){
                VarPpi->InfoCount++;
            }
        }else{
            if (NvGetDefaultsInfo(
                    StdDefaults,&MainNvram,&VarPpi->NvramInfo[VarPpi->InfoCount]
                ) != NULL
            ){
                VarPpi->InfoCount++;
            }
            if (ResetConfigMode){
                VarPpi->MainInfo = NULL;
                NvramMode|=NVRAM_MODE_RESET_CONFIGURATION;
            }else{
                VarPpi->NvramInfo[VarPpi->InfoCount]=MainNvram;
                VarPpi->MainInfo = &VarPpi->NvramInfo[VarPpi->InfoCount];
                VarPpi->InfoCount++;
                NvramMode|=NVRAM_MODE_DEFAULT_CONFIGURATION;
            }
        }
        if (!IsNvramDataCompatible(PeiServices, &VarPpi->Ppi)){
            PEI_TRACE((-1, PeiServices, "PEI: Incompactible NVRAM detected\n"));
            VarPpi->MainInfo = NULL;
            VarPpi->InfoCount=0;
        }
    }
#if !NV_SIMULATION_NO_FLASH_READS
    Status=(*PeiServices)->CreateHob(PeiServices, EFI_HOB_TYPE_GUID_EXTENSION, sizeof(NVRAM_HOB),&pHob);
    ASSERT_PEI_ERROR(PeiServices,Status)
	if (!EFI_ERROR(Status)){
	    pHob->Header.Name=AmiNvramHobGuid;
	    pHob->NvramAddress=(EFI_PHYSICAL_ADDRESS)(UINTN)MainNvram.NvramAddress;
	    pHob->BackupAddress = (EFI_PHYSICAL_ADDRESS)(UINTN)BackupNvramAddress;
	    pHob->NvramSize = MainNvram.NvramSize;
	    if (NvSimulation) NvramMode|=NVRAM_MODE_SIMULATION;
	    pHob->NvramMode = NvramMode;
	    pHob->HeaderLength = NvramHeaderLength;
	}
#endif
	return (*PeiServices)->InstallPpi(PeiServices,VariablePpiList);
}

EFI_STATUS
EFIAPI
PeiInitNV (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
    return PeiInitNVHelper(FileHandle, PeiServices);
}
   
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
