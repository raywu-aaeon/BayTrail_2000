//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1987-2013, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//**********************************************************************//
// $Header: 
//
// $Revision: 
//
// $Date: 
//**********************************************************************//
// Revision History
// ----------------
// $Log: $
// 
//**********************************************************************

//<AMI_FHDR_START>
//---------------------------------------------------------------------------
// Name:		CpuS3Pei.h
//
// Description:	
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>


#ifndef  __CPU_S3_PEI_H__   //To Avoid this header get compiled twice
#define  __CPU_S3_PEI_H__

#include <Guid/AcpiS3Context.h>
#include <Guid/BootScriptExecutorVariable.h>

#include <Ppi/BootScriptExecuter.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Ppi/S3Resume2.h>
#include <Ppi/SmmAccess.h>
#include <Ppi/PostBootScriptTable.h>

#include <Library/DebugLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/IoLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/LocalApicLib.h>
#include <Library/HobLib.h>
#include <Library/LockBoxLib.h>

#include <IndustryStandard/Acpi.h>

#endif

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1987-2013, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
