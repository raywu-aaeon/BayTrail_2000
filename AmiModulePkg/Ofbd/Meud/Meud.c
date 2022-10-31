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
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
// Revision History
// ----------------
// $Log: $
// 
// 
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	Meud.c
//
// Description:
//
//<AMI_FHDR_END>
//**********************************************************************
#include "Efi.h"
#include "Token.h"
#include <AmiLib.h>
#include <AmiDxeLib.h>
#include "Meud.h"
#include "../Ofbd.h"
#include <Protocol\SmiFlash.h>
#include <Flash.h>

EFI_SMI_FLASH_PROTOCOL *mSmiFlash;
UINT32 FlashCapacity;
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	MEUDCallback
//
// Description:	Locate SmiFlash protocol callback
//
// Input:
//      IN EFI_EVENT    Event
//      IN VOID         *Context
//
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
EFI_STATUS
MEUDCallback (
    CONST EFI_GUID  *Protocol,
    VOID            *Interface,
    EFI_HANDLE      Handle
)
#else
VOID
MEUDCallback (
    EFI_EVENT       Event,
    VOID            *Context
)
#endif
{
    EFI_GUID       gEfiSmiFlashProtocolGuid = EFI_SMI_FLASH_GUID;
    EFI_STATUS     Status;

#if PI_SPECIFICATION_VERSION >= 0x1000A
    Status = pSmst->SmmLocateProtocol (&gEfiSmiFlashProtocolGuid, NULL, &mSmiFlash);
    return EFI_SUCCESS;
#else
    Status = pBS->LocateProtocol (&gEfiSmiFlashProtocolGuid, NULL, &mSmiFlash);
    if(EFI_ERROR(Status)) mSmiFlash = NULL:
#endif
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	MEUDInSmm
//
// Description:	OFBD ME Firmware Update InSmm Function
//
// Input:
//      VOID
//
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID MEUDInSmm(VOID)
{
    EFI_STATUS     Status;
    EFI_GUID       gEfiSmiFlashProtocolGuid = EFI_SMI_FLASH_GUID;
    EFI_SMM_BASE2_PROTOCOL *mSmmBase2;
    EFI_SMM_SYSTEM_TABLE2*		  pSmst = NULL;
    EFI_GUID 	gEfiSmmBase2ProtocolGuid     = EFI_SMM_BASE2_PROTOCOL_GUID;

    FlashCapacity = GetFlashCapacity();

    Status = pBS->LocateProtocol (&gEfiSmmBase2ProtocolGuid, NULL, &mSmmBase2);
    TRACE((-1,"The Status of locate protocol(gEfiSmmBase2ProtocolGuid) is %r\n",Status));
    mSmmBase2->GetSmstLocation (mSmmBase2, &pSmst);

#if PI_SPECIFICATION_VERSION >= 0x1000A
    Status = pSmst->SmmLocateProtocol (&gEfiSmiFlashProtocolGuid, NULL, &mSmiFlash);
#else
    Status = pBS->LocateProtocol (&gEfiSmiFlashProtocolGuid, NULL, &mSmiFlash);
#endif

    if (EFI_ERROR(Status)){

#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
        VOID    *ProtocolNotifyRegistration;
        Status = pSmst->SmmRegisterProtocolNotify (
                    &gEfiSmiFlashProtocolGuid,
                    MEUDCallback,
                    &ProtocolNotifyRegistration
                    );
#else
        EFI_EVENT   SmiFlashCallbackEvt;
                VOID        *MEUDReg;
                RegisterProtocolCallback(
                     &gEfiSmiFlashProtocolGuid, MEUDCallback,
                     NULL,&SmiFlashCallbackEvt, &MEUDReg
                );
#endif
    }
    CSP_MEUDInSmm();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	MEUDEntry
//
// Description:	OFBD ME Firmware Update Entry point
//
// Input:
//      IN VOID             *Buffer
//      IN OUT UINT8        *pOFBDDataHandled
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID MEUDEntry (
    IN VOID             *Buffer,
    IN OUT UINT8        *pOFBDDataHandled )
{

    OFBD_HDR *pOFBDHdr;
    OFBD_EXT_HDR *pOFBDExtHdr;
//-    VOID *pOFBDTblEnd;
    OFBD_TC_55_MEUD_STRUCT *MEUDStructPtr;
    EFI_STATUS     Status = EFI_SUCCESS;
    UINT32         Buffer1, Buffer2;
    OFBD_TC_55_ME_INFO_STRUCT  *MEInfoStructPtr;
    OFBD_TC_55_ME_PROCESS_STRUCT  *MEProcessStructPtr;

#if (OFBD_VERSION >= 0x0210)
    static UINT8   MacAddr[8];
    UINT32         Length, Offset;
    BOOLEAN        FlashStatus = TRUE;
    UINT8          *Address;
    UINT32         Size;
    UINT8*         Data;
#endif

    pOFBDHdr = (OFBD_HDR *)Buffer;
    pOFBDExtHdr = (OFBD_EXT_HDR *)((UINT8 *)Buffer + \
                  (pOFBDHdr->OFBD_HDR_SIZE));
    MEUDStructPtr = (OFBD_TC_55_MEUD_STRUCT *)((UINT8 *)pOFBDExtHdr + \
                    sizeof(OFBD_EXT_HDR));
    MEInfoStructPtr = (OFBD_TC_55_ME_INFO_STRUCT*)MEUDStructPtr;
    MEProcessStructPtr = (OFBD_TC_55_ME_PROCESS_STRUCT*)MEUDStructPtr;
//-    pOFBDTblEnd = (VOID *)((UINT8 *)Buffer + (pOFBDHdr->OFBD_Size));
    
    if (*pOFBDDataHandled == 0)
    {
        if (pOFBDHdr->OFBD_FS & OFBD_FS_MEUD)
        {
            //Check Type Code ID
            if (pOFBDExtHdr->TypeCodeID == OFBD_EXT_TC_MEUD)
            {
                switch (MEUDStructPtr->bSubFunction)
                {
                case 0 :

                    Status = CSP_ReportMEInfo(0 , \
                            &(MEUDStructPtr->dMERuntimeBase), \
                            &(MEUDStructPtr->dMERuntimeLength));

                    MEUDStructPtr->dMEBiosRegionBase = \
                                                 FlashCapacity - FLASH_SIZE;
                    MEUDStructPtr->dMEBiosRegionLength = (UINT32)FLASH_SIZE;

                    if (FlashCapacity == 0)
                    {
                        // Fail , Return
                        *pOFBDDataHandled = 0xFE;
                        return;
                    }
                    *pOFBDDataHandled = 0xFF;
                    MEUDStructPtr->bReturnStatus =  OFBD_TC_MEUD_OK;
                    return;
                case 3 :
                    Status = CSP_ReportMEInfo(3, \
                            &(MEUDStructPtr->dMERuntimeBase), \
                            &(MEUDStructPtr->dMERuntimeLength));

                    MEUDStructPtr->dMEBiosRegionBase = \
                                                 FlashCapacity - FLASH_SIZE;
                    MEUDStructPtr->dMEBiosRegionLength = (UINT32)FLASH_SIZE;

                    // If FlashCapacity == 0, Not support ME Update
                    if (FlashCapacity == 0)
                    {
                        // Fail , Return
                        *pOFBDDataHandled = 0xFE;
                        return;
                    }

                    *pOFBDDataHandled = 0xFF;
                    MEUDStructPtr->bReturnStatus =  OFBD_TC_MEUD_OK;
                    return;
                case 1 :
                    // Send this again to check is this suppoet.
                    Status = CSP_ReportMEInfo(0, \
                            &Buffer1, \
                            &Buffer2);
                    if (!EFI_ERROR(Status))
                        Status = HMRFPO_ENABLE_MSG();

                    if (EFI_ERROR(Status))
                    {
                        // Fail , Return
                        *pOFBDDataHandled = 0xFE;
                        return;
                    }
                case 4 :
                    // Send this again to check is this suppoet.
                    Status = CSP_ReportMEInfo(3, \
                            &Buffer1, \
                            &Buffer2);
                    if (EFI_ERROR(Status))
                    {
                        // Fail , Return
                        *pOFBDDataHandled = 0xFE;
                        return;
                    }
                    MEUDStructPtr->bReturnStatus =  OFBD_TC_MEUD_OK;
                    *pOFBDDataHandled = 0xFF;
                    mSmiFlash->FlashCapacity = FlashCapacity;
#if (OFBD_VERSION >= 0x0210)
                    // Get GBE Region for Restore MAC address
                    Status = GetRegionOffset(3, &Offset, &Length);

                    // Before Erase, Get mac address
                    if(Length)
                    {
                        Address = (UINT8*)FLASH_BASE_ADDRESS(Offset);
                        FlashRead(Address, MacAddr, 6);
                    }
#endif
                    return;
                case 2 :
                    Status = HMRFPO_LOCK_MSG();
                    if (EFI_ERROR(Status))
                    {
                        // Fail , Return
                        *pOFBDDataHandled = 0xFE;
                        return;
                    }
                case 5 :
                    mSmiFlash->FlashCapacity = FLASH_SIZE;
                    MEUDStructPtr->bReturnStatus =  OFBD_TC_MEUD_OK;
                    *pOFBDDataHandled = 0xFF;
                    return;

                default :
                    *pOFBDDataHandled = 0xFE;
                    break;
// =============== OFBD Version 2.1 and AFU version 2.37 or newer ===============
#if (OFBD_VERSION >= 0x0210)
                // earse
                case 6 :

                    Size = MEUDStructPtr->ddBlockSize;
                    Status = CSP_ReportMEInfo(3, \
                                &Buffer1,
                                &Buffer2);

                    // Return if Update Block address is over our expect
                    if(MEUDStructPtr->ddBlockAddr > Buffer2)
                    {
                         *pOFBDDataHandled = 0xFF;
                         MEUDStructPtr->bReturnStatus =  OFBD_TC_MEUD_OK;

                         return;
                    }

                    Address = (UINT8*)FLASH_BASE_ADDRESS(MEUDStructPtr->ddBlockAddr);
                    // Skip the address not reported by CSP_ReportMEInf
                    if(Buffer1 > MEUDStructPtr->ddBlockAddr)
                        (UINTN)Address = FLASH_BASE_ADDRESS(Buffer1);

                    FlashDeviceWriteEnable();

                    for (; FlashStatus && Size > 0; Address += FlashBlockSize, Size -= FlashBlockSize)
                    {
                         FlashBlockWriteEnable(Address);
                         FlashStatus = FlashEraseBlock(Address);
                         FlashBlockWriteDisable(Address);
                     }
                     FlashDeviceWriteDisable();
                     if(FlashStatus)
                     {
                         *pOFBDDataHandled = 0xFF;
                         MEUDStructPtr->bReturnStatus =  OFBD_TC_MEUD_OK;
                     }
                    break;
                // program
                case 7 :
                    Size = MEUDStructPtr->ddBlockSize;
                    Data = (UINT8*)pOFBDHdr;
                    Status = CSP_ReportMEInfo(3, \
                                &Buffer1,
                                &Buffer2);

                    Status = GetRegionOffset(3, &Offset, &Length);

                    if((MEUDStructPtr->ddBlockAddr > Buffer2) ||
                       (Buffer1 > MEUDStructPtr->ddBlockAddr))
                    {
                        *pOFBDDataHandled = 0xFF;
                        MEUDStructPtr->bReturnStatus =  OFBD_TC_MEUD_OK;
                        return;
                    }

                    (UINTN)Data += MEUDStructPtr->ddFlashBufOffset;
                    (UINTN)Address = FLASH_BASE_ADDRESS(MEUDStructPtr->ddBlockAddr);
                    // Restore MAC Address
                    if((MEUDStructPtr->ddBlockAddr == Offset) && (Length != 0))
                        MemCpy(Data,MacAddr,6);

                    FlashDeviceWriteEnable();

                    FlashBlockWriteEnable(Address);
                    FlashStatus = FlashProgram(Address, Data, Size);
                    FlashBlockWriteDisable(Address);

                    FlashDeviceWriteDisable();
                    if(FlashStatus)
                    {
                        *pOFBDDataHandled = 0xFF;
                        MEUDStructPtr->bReturnStatus =  OFBD_TC_MEUD_OK;
                    }
                    break;
                       // Read
                       case 8 :
                           (UINTN)Address = FLASH_BASE_ADDRESS(MEUDStructPtr->ddBlockAddr);
                           Data = (UINT8*)pOFBDHdr;
                           (UINTN)Data += MEUDStructPtr->ddFlashBufOffset;

                           FlashRead(Address, Data, MEUDStructPtr->ddBlockSize);
                           *pOFBDDataHandled = 0xFF;
                           MEUDStructPtr->bReturnStatus =  OFBD_TC_MEUD_OK;
                           break;

                       case 9 :
                           // Get Info
                           {
                               UINT16    TotalBlocks = 0;

                               // Get FD
                               Status = GetRegionOffset(0 ,
                                   &MEInfoStructPtr->BlockInfo[TotalBlocks].StartAddress,
                                   &MEInfoStructPtr->BlockInfo[TotalBlocks].BlockSize);

                               // Fill FD Info
                               if(!EFI_ERROR(Status))
                               {
                                   MemCpy(MEInfoStructPtr->BlockInfo[TotalBlocks].Command,"FDR",4);
                                   MemCpy(MEInfoStructPtr->BlockInfo[TotalBlocks].Description,
                                          "Flash Flash-Descriptor Region",64);
                                   MEInfoStructPtr->BlockInfo[TotalBlocks].Type = FDT_BLK;
                                   // Status = 1 means Protect
                                   MEInfoStructPtr->BlockInfo[TotalBlocks].Status = 0;
                                   TotalBlocks += 1;
                               }
                               // Get GBE
                               Status = GetRegionOffset(3 ,
                                   &MEInfoStructPtr->BlockInfo[TotalBlocks].StartAddress,
                                   &MEInfoStructPtr->BlockInfo[TotalBlocks].BlockSize);

                               // Fill GBE Info
                               if(!EFI_ERROR(Status))
                               {
                                   MemCpy(MEInfoStructPtr->BlockInfo[TotalBlocks].Command,"GBER",4);
                                   MemCpy(MEInfoStructPtr->BlockInfo[TotalBlocks].Description,
                                          "Flash GBE Region",64);
                                   MEInfoStructPtr->BlockInfo[TotalBlocks].Type = GBE_BLK;
                                   // Status = 1 means Protect
                                   MEInfoStructPtr->BlockInfo[TotalBlocks].Status = 0;
                                   TotalBlocks += 1;
                               }

                               // Get PDR
                               Status = GetRegionOffset(4 ,
                                   &MEInfoStructPtr->BlockInfo[TotalBlocks].StartAddress,
                                   &MEInfoStructPtr->BlockInfo[TotalBlocks].BlockSize);

                               // Fill PDR Info
                               if(!EFI_ERROR(Status))
                               {
                                   MemCpy(MEInfoStructPtr->BlockInfo[TotalBlocks].Command,"PDR",4);
                                   MemCpy(MEInfoStructPtr->BlockInfo[TotalBlocks].Description,
                                          "Flash PDR Region",64);
                                   MEInfoStructPtr->BlockInfo[TotalBlocks].Type = PDR_BLK;
                                   // Status = 1 means Protect
                                   MEInfoStructPtr->BlockInfo[TotalBlocks].Status = 0;
                                   TotalBlocks += 1;
                               }

                               // Get ME
                               Status = GetRegionOffset(2 ,
                                   &MEInfoStructPtr->BlockInfo[TotalBlocks].StartAddress,
                                   &MEInfoStructPtr->BlockInfo[TotalBlocks].BlockSize);

                               // Fill ME Info
                               if(!EFI_ERROR(Status))
                               {
                                   MemCpy(MEInfoStructPtr->BlockInfo[TotalBlocks].Command,"MER",4);
                                   MemCpy(MEInfoStructPtr->BlockInfo[TotalBlocks].Description,
                                          "Flash Entire ME Region",64);
                                   MEInfoStructPtr->BlockInfo[TotalBlocks].Type = ME_BLK;
                                   // Status = 1 means Protect
                                   MEInfoStructPtr->BlockInfo[TotalBlocks].Status = 0;
                                   TotalBlocks += 1;
                               }

                               //
                               // Fill SPS Partition Info
                               //
#if defined(OPR1_LENGTH)
                               // OPR1
                               MEInfoStructPtr->BlockInfo[TotalBlocks].StartAddress = OPR1_START;
                               MEInfoStructPtr->BlockInfo[TotalBlocks].BlockSize = OPR1_LENGTH;
                                   // OPR2
                               if(OPR2_LENGTH != 0)
                                   MEInfoStructPtr->BlockInfo[TotalBlocks].BlockSize += OPR2_LENGTH;

                               MemCpy(MEInfoStructPtr->BlockInfo[TotalBlocks].Command,"OPR",4);
                               MemCpy(MEInfoStructPtr->BlockInfo[TotalBlocks].Description,
                                      "Flash Operation Region of SPS",64);

                               MEInfoStructPtr->BlockInfo[TotalBlocks].Type = ME_OPR_BLK;
                               // Status = 1 means Protect
                               MEInfoStructPtr->BlockInfo[TotalBlocks].Status = 0;
                               TotalBlocks += 1;
#endif
                               MEInfoStructPtr->TotalBlocks = TotalBlocks;
                               *pOFBDDataHandled = 0xFF;
                               MEUDStructPtr->bReturnStatus =  OFBD_TC_MEUD_OK;

                           }
                           break;
                       case 10 :
                           // ME Process Handle
                           // In CSP_MEUD.c
                           MEProcessHandler(&MEProcessStructPtr);
                           *pOFBDDataHandled = 0xFF;
                           MEUDStructPtr->bReturnStatus =  OFBD_TC_MEUD_OK;
                           break;
#endif //#if (OFBD_VERSION >= 0x0210)
                }// End of Switch
            }

        }

    }
    return;
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
