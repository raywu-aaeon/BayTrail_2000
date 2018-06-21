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
//*************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//*************************************************************************
// Revision History
// ----------------
// $Log: $
// 
//
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	CspMeud.c
//
// Description:
//
//<AMI_FHDR_END>
//**********************************************************************
#include "Efi.h"
#include "Token.h"
#include <AmiDxeLib.h>
#include "CspMeud.h"
#include "Ofbd\Meud\Meud.h"
#include <Protocol/SmmSwDispatch.h>
#if PI_SPECIFICATION_VERSION >= 0x1000A
#include <Protocol/SmmCpu.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmSwDispatch2.h>
#else
#include <Protocol/SmmBase.h>
#include <Protocol/SmmSwDispatch.h>
#endif
#include <Protocol/SmmSwDispatch.h>
#include <Flash.h>
#include <Protocol/Heci.h>

UINT32	Factory_Base;
UINT32	Factory_Limit;
UINT64  Nonce = 0;
UINT8   MacAddr[6];
OFBD_TC_55_ME_PROCESS_STRUCT  *StructPtr;
EFI_PHYSICAL_ADDRESS   Phy_Address;
BOOLEAN IsSecoverMode = FALSE;

FLASH_REGIONS_DESCRIPTOR
FlashRegionsDescriptor[] =
{
    { FDT_BLK, "DSC" },
    { BIOS_BLK, "BIOS" },
    { GBE_BLK, "GBE" },
    { PDR_BLK, "PDR" },
    { REG8_BLK, "EC" },
    { ME_BLK, "ME" },
    { MAX_BLK, "" },
};

// Define in OFBD module.
extern EFI_SMM_BASE2_PROTOCOL          *gSmmBase2;
extern EFI_SMM_CPU_PROTOCOL            *gSmmCpu;
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   GetHFS
//
// Description: Get Host Firmware Status pass to MEUD
//
// Input:   NONE
//
// Output:  Host Firmware Status
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32 GetHFS(VOID)
{
    return 0;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   HMRFPO_ENABLE_MSG
//
// Description: Send Enable HECI message to ME FW.
//
// Input:   NONE
//
// Output:  EFI_STATUS
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS HMRFPO_ENABLE_MSG(VOID)
{

    return EFI_UNSUPPORTED;

}
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   HMRFPO_LOCK_MSG
//
// Description: Send LOCK HECI message and lock ME.
//
// Input:   NONE
//
// Output:  NONE
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS HMRFPO_LOCK_MSG(VOID)
{
    return EFI_UNSUPPORTED;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   GetFlashCapacity
//
// Description: Send a HECI message to lock ME.
//
// Input:   NONE
//
// Output:  FlashDensity - Real Flash Size
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32 GetFlashCapacity(VOID)
{
	    volatile UINT32	*FDOC;
	    volatile UINT32	*FDOD;
	    UINT32	FlashDescriptorSig = 0x0FF0A55A;
	    static UINT32     FlashDensity = 0;
	    UINT16	Components;
	    UINT8	i,j;

	    if(FlashDensity)
		    return FlashDensity;
	    FDOC = (UINT32*)(SPI_BASE + 0xB0);
	    FDOD = (UINT32*)(SPI_BASE + 0xB4);
	    *FDOC = 0;

	    if (*FDOD != FlashDescriptorSig)
	        return 0;

	    *FDOC = 0x04;
	    Components = (*FDOD >> 8) & 0x03;

	    *FDOC = 0x1000;
	    j = *FDOD;


	    for (i=0; i<(Components + 1); i++)
	    {
	        switch (j & 0x07)
	        {
	        case 0:
	            FlashDensity += 0x80000;
	            break;
	        case 1:
	            FlashDensity += 0x100000;
	            break;
	        case 2:
	            FlashDensity += 0x200000;
	            break;
	        case 3:
	            FlashDensity += 0x400000;
	            break;
	        case 4:
	            FlashDensity += 0x800000;
	            break;
	        case 5:
	            FlashDensity += 0x1000000;
	            break;
	        default:
	            break;
	        }
	        j = j >> 4;
	    }
	    return	FlashDensity;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   CSP_ReportMEInfo
//
// Description: Report ME Base address and Length to AFU
//
// Input:   BASE_Address - address of ME region to be updated
//          Length - Length of ME region to be updated
//          Func_Num - 0 for Command /MEUF , 3 for Command /ME
//
// Output:  NONE
//
// Returns: NONE
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CSP_ReportMEInfo
(
    IN UINT8 Func_Num,
    IN OUT UINT32* BASE_Address,
    IN OUT UINT32* Length
)
{
	    switch (Func_Num)
	    {
	    case 0:

	        *BASE_Address = 0;
	        *Length = GetFlashCapacity() - FLASH_SIZE;
	        return EFI_UNSUPPORTED;

	        break;
	    case 3:
	        // Flash the whole SPI but BIOS region
	        *BASE_Address = 0;
	        *Length = GetFlashCapacity() - FLASH_SIZE;
	        if(IsSecoverMode)
                    return EFI_SUCCESS;
	        else
                    return EFI_UNSUPPORTED;
	        break;
	    default:
	        return EFI_UNSUPPORTED;
	        break;
	    }
	    return EFI_SUCCESS;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   MEUDSMIHandler
//
// Description: Send Enable and Global reset MSG to ME FW.
//
// Input:   NONE
//
// Output:  NONE
//
// Returns: NONE
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
#if PI_SPECIFICATION_VERSION >= 0x1000A
EFI_STATUS 
MEUDSMIHandler (
        IN  EFI_HANDLE                  DispatchHandle,
		IN CONST VOID                   *Context OPTIONAL,
		IN OUT VOID                     *CommBuffer OPTIONAL,
		IN OUT UINTN                    *CommBufferSize OPTIONAL
)
#else
VOID MEUDSMIHandler (
    IN  EFI_HANDLE                  DispatchHandle,
    IN  EFI_SMM_SW_DISPATCH_CONTEXT *DispatchContext
)
#endif
{
    UINT8    Result;


    HeciHmrfpoEnable (Nonce, &Result);
    HeciSendCbmResetRequest (0x02, 0x01);
#if PI_SPECIFICATION_VERSION >= 0x1000A
    return EFI_SUCCESS;
#endif
}
VOID HeciProtocolCallBack(
    EFI_EVENT   Event,
    VOID        *Context
)
{
    EFI_GUID    gEfiHeciProtocolGuid = HECI_PROTOCOL_GUID;
    EFI_HECI_PROTOCOL *gHeci = NULL;
    EFI_STATUS  Status;
    UINT32      HeciMode = 0;
    
    Status = pBS->LocateProtocol(
                 &gEfiHeciProtocolGuid, NULL, &gHeci);
    if(EFI_ERROR(Status))
	    return;
    Status = gHeci->GetSeCMode(&HeciMode);
    if(EFI_ERROR(Status))
	    return;
    
    if(HeciMode == SEC_MODE_SECOVER)
	    IsSecoverMode = TRUE;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   CSP_MEUDInSmm
//
// Description: Get Host Firmware Status.
//              If needed, Send LOCK if needed in SMM.
//
// Input:   NONE
//
// Output:  NONE
//
// Returns: NONE
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID CSP_MEUDInSmm(VOID)
{  
    EFI_STATUS                      Status;
    EFI_GUID    gEfiHeciProtocolGuid = HECI_PROTOCOL_GUID;
    EFI_HANDLE  Handle;
    VOID        *gHeciNotifyRegistration;
    EFI_EVENT   gHeciEvent;
#if PI_SPECIFICATION_VERSION >= 0x1000A
    EFI_SMM_SW_DISPATCH2_PROTOCOL    *pSwDispatch = NULL;
    EFI_SMM_SW_REGISTER_CONTEXT      SwContext = {Disable_ME_SW_SMI};
#else
    EFI_SMM_SW_DISPATCH_PROTOCOL    *pSwDispatch;
    EFI_SMM_SW_DISPATCH_CONTEXT     SwContext = {Disable_ME_SW_SMI};
#endif

#if PI_SPECIFICATION_VERSION >= 0x1000A
    
    Status = pSmmBase->GetSmstLocation (gSmmBase2, &pSmst);
    if (EFI_ERROR(Status)) return;


    Status = pSmst->SmmLocateProtocol( \
                        &gEfiSmmSwDispatch2ProtocolGuid, NULL, &pSwDispatch);
    if (EFI_ERROR(Status)) return;


    Status = pSmst->SmmLocateProtocol(&gEfiSmmCpuProtocolGuid, NULL, &gSmmCpu);
    if (EFI_ERROR(Status)) return;

#else
    VERIFY_EFI_ERROR(pBS->LocateProtocol(
                          &gEfiSmmSwDispatchProtocolGuid, NULL, &pSwDispatch));
#endif

    Status = pSwDispatch->Register(pSwDispatch, MEUDSMIHandler, &SwContext, \
                                                                     &Handle);

    Status = pBS->LocateProtocol(
                    &gEfiHeciProtocolGuid,
                    NULL,
                    &gHeciNotifyRegistration );
    if( !EFI_ERROR(Status) )
    {
        HeciProtocolCallBack( NULL, NULL );
    }
    else
    {
        Status = RegisterProtocolCallback(
                    &gEfiHeciProtocolGuid,
                    HeciProtocolCallBack,
                    NULL,   
                    &gHeciEvent,
                    &gHeciNotifyRegistration );
    }
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	GetRegionOffset
//
// Description:	Get GBE Region Offet of whole FlashPart
//
// Input:
//      VOID
// Output:
//      UINT32  The offset of GBE Region
//
// Returns:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
GetRegionOffset(
    UINT8    Region,
    UINT32*  Offset,
    UINT32*  Length
)
{
    volatile UINT32	*FDOC;
    volatile UINT32	*FDOD;
    UINT32	FlashDescriptorSig = 0x0FF0A55A;
    UINT32	Buffer32, RegionStart, RegionEnd;
    UINT8   EdsRegion;

    FDOC = (UINT32*)(SPI_BASE + 0xB0);
    FDOD = (UINT32*)(SPI_BASE + 0xB4);

    *FDOC = 0;

    if( *FDOD != FlashDescriptorSig )
        return EFI_UNSUPPORTED;

    switch( Region )
    {
        //Mapping old definition to region number
        case PDR_BLK:
            EdsRegion = 4;
            break;

        case GBE_BLK:
            EdsRegion = 3;
            break;

        case ME_BLK:
            EdsRegion = 2;
            break;

        case ME_OPR_BLK:
            EdsRegion = 0xFF;
            break;

        case BIOS_BLK:
            EdsRegion = 1;
            break;

        case DER_BLK:
            EdsRegion = 5;
            break;
            
        case BIOS_2ND_BLK:
            EdsRegion = 6;
            break;

        default:
            EdsRegion = Region;
            break;
    }

    if( EdsRegion != 0xFF )
        *FDOC = (0x2000 + (EdsRegion * 0x04));
    else
        return EFI_UNSUPPORTED;

    Buffer32 = *FDOD;

    //If the region is unsued
    if( Buffer32 == 0x00007FFF )
        return EFI_UNSUPPORTED;

    RegionEnd = Buffer32 >> 16;
    RegionStart = Buffer32 & 0xFFFF;

    *Offset = RegionStart << 12;
    *Length = (RegionEnd - RegionStart + 1) << 12;
    if((Region == 0) && (RegionEnd == 0))
    {
        *Length = 0x1000;
        return EFI_SUCCESS;
    }
    if(RegionEnd == 0)
    {
        *Length = 0;
        return EFI_NOT_FOUND;
    }

    return EFI_SUCCESS;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	MEProcessHandleResult
//
// Description:	Handle ME Process
//
// Input:
//      UpdateResult
//      Message
// Output:
//      VOID
//
// Returns:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
MEProcessHandleResult(
    IN UINT16   Result,
    IN CHAR8*   Message
)
{
    StructPtr->UpdateResult = Result;
    MemCpy((UINT8*)(StructPtr->ddMessageBuffer), 
                    Message, Strlen(Message));

    *(CHAR8*)(StructPtr->ddMessageBuffer + Strlen(Message)) = 0;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	HandleBuffer
//
// Description:	Init the Length and Offset need to be updated
//              If needed, send ENABLE MESSAGE
//
// Input:
//      UpdateResult
//      Message
// Output:
//      VOID
//
// Returns:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
HandleBuffer(
    IN OUT UINT32*   ProgramOffset,
    IN OUT UINT32*   ProgramLength,
    IN OUT UINT8*    Step,
    IN     BOOLEAN   InSmm
)
{
    EFI_STATUS    Status;
    UINT32        Offset, Length;
    UINT32        HFS = GetHFS();

    switch( StructPtr->bBlockType )
    {
        case ME_BLK:
            Status = GetRegionOffset( StructPtr->bBlockType, &Offset, &Length );
            if( EFI_ERROR(Status) ) return Status;
            if( (HFS & BIT05) || (HFS & BIT10) )
                *Step = 2;
            else
                *Step = 1;
        break;

        case GBE_BLK:
            Status = GetRegionOffset( StructPtr->bBlockType, &Offset, &Length );
            if(Status == EFI_NOT_FOUND)
                return EFI_UNSUPPORTED;
            // Store Mac address
            if(Length)
            {
                UINT8   *Address = (UINT8*)FLASH_BASE_ADDRESS(Offset);
                FlashRead(Address, MacAddr, 6);
            }
            *Step = 0;
        break;

        default:
            Status = GetRegionOffset( StructPtr->bBlockType, &Offset, &Length );
            if( EFI_ERROR(Status) )
                return EFI_UNSUPPORTED;
            *Step = 0;
        break;
    }

    *ProgramOffset = Offset;
    *ProgramLength = Length;

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	UpdateRegions
//
// Description:	UpdateRegions
//
// Input:
//      Buffer
//
// Output:
//      VOID
//
// Returns:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
UpdateRegions(
    IN UINT8*    Buffer,
    IN BOOLEAN   InSmm
)
{
    static UINT32          Offset, Length;
    UINT8*                 Address;
    EFI_STATUS             Status;
    BOOLEAN                FlashStatus = TRUE, NeedToVerify = FALSE;
    static UINT8           Step = 0;
    static BOOLEAN         NewRegion;
    UINTN                  Counter = 0;
    static UINT8*          ProgramBuffer;

    // Prepare Offset and Length to be updated
    // If BIT02 , update buffer
    if((StructPtr->bHandleRequest & BIT02))
    {
        Status = HandleBuffer(&Offset, &Length, &Step, InSmm);
        if(EFI_ERROR(Status))
        {
            MEProcessHandleResult(BIT03, 
                      "UN SUPPORT TYPE");
            return Status;
        }
        // Frist In
        NewRegion = TRUE;
        ProgramBuffer = (UINT8*)(Phy_Address + Offset);
    }

    // Set MAC address to buffer
    if(((StructPtr->bBlockType) == GBE_BLK) && NewRegion)
        MemCpy((Buffer + Offset),MacAddr,6);

    if(NewRegion)
    {
        NewRegion = FALSE;
    }

    Address = (UINT8*)FLASH_BASE_ADDRESS(Offset);

    FlashBlockWriteEnable(Address);
    FlashStatus = FlashEraseBlock(Address);

    if(FlashStatus)
    {

        FlashStatus = FlashProgram(Address, ProgramBuffer, FLASH_BLOCK_SIZE);
        if(FlashStatus)
            Status = EFI_SUCCESS;
        else
            Status = EFI_DEVICE_ERROR;
    }else
        Status = EFI_DEVICE_ERROR;
    FlashBlockWriteDisable(Address);
    ProgramBuffer = ProgramBuffer + FLASH_BLOCK_SIZE;
    Length -= FLASH_BLOCK_SIZE;
    Offset += FLASH_BLOCK_SIZE;

    // End of Region Update
    if(Length == 0)
    {
        NewRegion = TRUE;
    }
    // TODO :
    // OEM can output message here in every block updated.
    // Remember to Set BIT02
    else
    {
        MEProcessHandleResult((BIT01), 
                         " ");
        return EFI_SUCCESS;
    }
    // Show Strings
    if(!EFI_ERROR(Status))
    {
        UINT8   ResultString[25];
        UINT8   Index;

        for( Index = 0; Index < MAX_BLK; Index++ )
        {
            if( FlashRegionsDescriptor[Index].FlashRegion == MAX_BLK )
                break;

            if( FlashRegionsDescriptor[Index].FlashRegion == StructPtr->bBlockType )
            {
                Sprintf( ResultString, "Update success for %s", FlashRegionsDescriptor[Index].Command );
                    break;
            }
        }

        if( (Index == MAX_BLK) || (FlashRegionsDescriptor[Index].FlashRegion == MAX_BLK) )
            Sprintf( ResultString, "Update success" );

        MEProcessHandleResult( (BIT03 | BIT02), ResultString );
    }
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	MEProcessHandler
//
// Description:	Handle ME Process
//
// Input:
//      VOID
// Output:
//      OFBD_TC_55_ME_PROCESS_STRUCT
//
// Returns:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
MEProcessHandler
(
    IN OUT OFBD_TC_55_ME_PROCESS_STRUCT  **MEProcessStructPtr
)
{
    static UINT32                 SizeCopied;
    static EFI_PHYSICAL_ADDRESS   SMM_Address;
    static BOOLEAN                UseSmmMem = FALSE;
    static UINTN                  NumberOfPages;
    EFI_STATUS                    Status;
    
    StructPtr = *MEProcessStructPtr;
    switch(StructPtr->bHandleRequest)
    {
        // Allocate Buffer
        case 1:
            NumberOfPages = StructPtr->TotalBlocks;
            Status = pSmst->SmmAllocatePages(AllocateAnyPages, 
                     EfiRuntimeServicesData, NumberOfPages, &SMM_Address);
            if(!EFI_ERROR(Status))
            {
                UseSmmMem = TRUE;
                Phy_Address = SMM_Address;
            }
            // No memory allocated
            if(!Phy_Address)
               MEProcessHandleResult((BIT00 | BIT02), 
                          "Error : No Memory Allocated!!");

            SizeCopied = 0;

        break;

        // Recieve Data from AFU
        case 2:
            MemCpy((UINT8*)(Phy_Address + SizeCopied),
                    (UINT8*)StructPtr->ddMeDataBuffer,StructPtr->ddMeDataSize);

            SizeCopied += StructPtr->ddMeDataSize;

        break;

        // Update
        case 4:
                UpdateRegions((UINT8*)Phy_Address, TRUE);
        break;

        // Continue....
        case 8:
                UpdateRegions((UINT8*)Phy_Address, TRUE);
        break;

        // Free Buffer
        case 0x10:
                if(UseSmmMem)
                    pSmst->SmmFreePages(Phy_Address, NumberOfPages);
        break;
    }
}

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
