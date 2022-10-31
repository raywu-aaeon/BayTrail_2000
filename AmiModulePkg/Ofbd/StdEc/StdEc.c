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
// $Header: /AptioV/Source/Modules/Ofbd/StdEc/StdEc.c $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	StdEc.c
//
// Description: 
// This function is used to support flashing AMI EC firmware.
//
//<AMI_FHDR_END>
//**********************************************************************
#include <Efi.h>
#include <Token.h>
#include <AmiLib.h>
#include <AmiDxeLib.h>
#include "StdEc.h"
#include "../Ofbd.h"

UINT8	PMBase30 = 0 ;		//	(for Intel ICHx)
//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:	ObFullRead
//
// Description:	Waits for Output Buffer Full and then reads the data port
//
// Input:		None
//
// Output:		KBC Data port data
//
//<AMI_PHDR_END>
//**********************************************************************                

UINT8 ObFullRead()
{
	for (;;) {
		if (IoRead8(EC_KBC_CMD_PORT) & EC_KBC_OBF) {
			return IoRead8(EC_KBC_DATA_PORT);
		}
	}
}

void IbFree()
{
	for (;;) {
		if (!(IoRead8(EC_KBC_CMD_PORT) & EC_KBC_IBF)) {
			break;
		}
	}
}
//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:	WriteKeyboardCommand
//
// Description:	Writes command to KBC.
//
// Input:		Command to send to KBC
//
// Output:		None
//
//<AMI_PHDR_END>
//**********************************************************************                

void WriteKeyboardCommand(UINT8 bCmd)
{
	IbFree();
	IoWrite8(EC_KBC_CMD_PORT, bCmd);
	IbFree();
}
//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:	WriteKeyboardData
//
// Description:	Writes data to KBC.
//
// Input:		Data to send to KBC
//
// Output:		None
//
//<AMI_PHDR_END>
//**********************************************************************                
void WriteKeyboardData(UINT8 bData)
{
	IbFree();
	IoWrite8(EC_KBC_DATA_PORT, bData);
	IbFree();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	OFBDECCommand
//
// Description:	EC command
//
// Input:
//      IN UINT8            bCmd
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
OFBDECCommand(IN UINT8 bCmd)
{
    EFI_STATUS Status = EFI_SUCCESS;

    WriteKeyboardCommand(bCmd);
    if(ObFullRead() != EC_NO_ERROR_FM_ACK)
        Status = RETURN_ERR;

    return(Status);
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	OFBDECCheckSum
//
// Description:	computes EC data checksum
//
// Input:
//      IN UINT8 *BuffPtr
//      IN UINT32 Size
// Output:
//      UINT32
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32
OFBDECCheckSum(IN UINT8 *BuffPtr, IN UINT32 Size)
{
    UINT32 CheckSum = 0;
    UINT32 i = 0;

    for (i = 0; i < Size / sizeof (UINT32); i++)
        CheckSum += *(UINT32*)(BuffPtr + i * sizeof (UINT32));

    return (~CheckSum);
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	USBKeyboardMouseDisable
//
// Description:	USB Keyboard Mouse Disable
//
// Input:
//      IN OFBD_TC_54_STD_EC_STRUCT *ECStructPtr
//      OUT UINT8 *ReturnErr
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
USBKeyboardMouseDisable(
    IN OFBD_TC_54_STD_EC_STRUCT *ECStructPtr,
    OUT UINT8   *ReturnErr)
{
    EFI_STATUS Status = EFI_SUCCESS;

    if(ECStructPtr->dbFlaSts & OFBD_TC_STD_EC_START)
    {
        *ReturnErr = 0;
        // Disable Legacy USB SMI	
        //for example (for Intel ICHx)
	    //PMBase30 = IoRead8 (PM_BASE_ADDRESS + 0x30) ;
	    //IoWrite8 ( (PM_BASE_ADDRESS + 0x30), (PMBase30 & ~8));
    }

    return(Status);
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	USBKeyboardMouseEnable
//
// Description:	USB Keyboard Mouse Enable
//
// Input:
//      IN OFBD_TC_54_STD_EC_STRUCT *ECStructPtr,
//      IN UINT8 *ReturnErr)
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
USBKeyboardMouseEnable(
    IN OFBD_TC_54_STD_EC_STRUCT *ECStructPtr,
    IN UINT8 *ReturnErr)
{
    EFI_STATUS Status = EFI_SUCCESS;

    if((ECStructPtr->dbFlaSts & OFBD_TC_STD_EC_END) | (*ReturnErr != 0))
    {
        // Restore Legacy USB SMI	
        //for example (for Intel ICHx)
	    //IoWrite8 ( (PM_BASE_ADDRESS + 0x30), PMBase30 ) ;
    }

    return(Status);
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	OFBDStdECStart
//
// Description:	for EC firmware entry flash mode and start transmit data
//
// Input:
//      IN OUT OFBD_TC_54_STD_EC_STRUCT *ECStructPtr
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
OFBDStdECStart(
    IN OUT OFBD_TC_54_STD_EC_STRUCT *ECStructPtr)
{
    EFI_STATUS Status = EFI_SUCCESS;
    UINT16 ChipID = 0;
    VOID *pOFBDTblEnd = (VOID *)((UINT8 *)ECStructPtr + sizeof(OFBD_TC_54_STD_EC_STRUCT) + sizeof(OFBD_END));

    WriteKeyboardCommand(EC_CMD_DISABLE_KBD); //0xAD

    //Identify Chip ID
    WriteKeyboardCommand(EC_CMD_GET_CHIPID);    //0xA3
    ChipID = ObFullRead() << 8;                 //Chip ID HiByte
    ChipID |= ObFullRead();                     //save Chip ID LowByte
    if(*(UINT16 *)pOFBDTblEnd != ChipID)        
    {
        ECStructPtr->dwRetSts |= OFBD_RS_EC_CHIPID; 
        WriteKeyboardCommand(EC_CMD_ENABLE_KBD);   //0xAE
        return(RETURN_ERR);
    }

    WriteKeyboardCommand(EC_CMD_ENABLE_KBD);   //0xAE

    //Entry Flash Mode			0xDF
    if(OFBDECCommand(EC_CMD_ENTRY_FLASHMODE) != EFI_SUCCESS)
    {
        ECStructPtr->dwRetSts |= OFBD_RS_EC_ENTRY;    
        return(RETURN_ERR);
    }

    //Erase Flash Memory			0x01
    if(OFBDECCommand(EC_CMD_ERASE_MEMORY) != EFI_SUCCESS)
    {
        ECStructPtr->dwRetSts |= OFBD_RS_EC_ERASE;      
        WriteKeyboardCommand(EC_CMD_EXIT_FLASHMODE);    //0xFE  
        return(RETURN_ERR);
    }

    //Initial Programming Mode			0x02
    if(OFBDECCommand(EC_CMD_INITIAL_PM) != EFI_SUCCESS)
    {
        ECStructPtr->dwRetSts |= OFBD_RS_EC_INIT;       
        WriteKeyboardCommand(EC_CMD_EXIT_FLASHMODE);    //0xFE  
        return(RETURN_ERR);
    }
       
    ECStructPtr->dwRetSts |= OFBD_RS_EC_OK;              

    return(Status);
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	OFBDStdECFlash
//
// Description:	for EC firmware to transmit data
//
// Input:
//      IN OFBD_TC_54_STD_EC_STRUCT *ECStructPtr
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
OFBDStdECFlash(IN OFBD_TC_54_STD_EC_STRUCT *ECStructPtr)
{
    EFI_STATUS Status = EFI_SUCCESS;
    UINT32 CheckSum = 0;
    UINT8 ByteCheckSum = 0;
    VOID *pOFBDTblEnd = (VOID *)((UINT8 *)ECStructPtr + sizeof(OFBD_TC_54_STD_EC_STRUCT) + sizeof(OFBD_END));
    UINT8 *DataPtr = (UINT8 *)(*(UINT64 *)((UINT8 *)pOFBDTblEnd - sizeof(OFBD_END)));
    UINT32 i, j;

    //calc Check Sum
    CheckSum = OFBDECCheckSum(DataPtr, ECStructPtr->ddSize);
    if(ECStructPtr->ddChkSum != CheckSum)
    {
        ECStructPtr->dwRetSts |= OFBD_RS_EC_CHKSUM;
        return(RETURN_ERR);
    }

    for(i=0; i < ECStructPtr->ddSize; i += 0x80)
    {
        //Start Flash Data Transmit
        if(OFBDECCommand(EC_CMD_START_TRANS) != EFI_SUCCESS)
        {
             ECStructPtr->dwRetSts |= OFBD_RS_EC_START;       
             WriteKeyboardCommand(EC_CMD_EXIT_FLASHMODE);    //0xFE  
             return(RETURN_ERR);
        }
        ByteCheckSum = 0;
        for(j = 0; j < 0x80; j++)
        {
             WriteKeyboardData(*(DataPtr + i + j));
             ByteCheckSum += *(DataPtr + i + j);   
        }
        if(ObFullRead() == ByteCheckSum)
        {
             //Write Flash Data	        0x04
             if(OFBDECCommand(EC_CMD_WRITE_DATA) != EFI_SUCCESS)
             {
                 ECStructPtr->dwRetSts |= OFBD_RS_EC_WRITE;       
                 WriteKeyboardCommand(EC_CMD_EXIT_FLASHMODE);    //0xFE  
                 return(RETURN_ERR);
             }
       }
       else
       {
             i -= 0x80;
       }
    }
    ECStructPtr->dwRetSts |= OFBD_RS_EC_OK;             

    return(Status);
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	OFBDStdECEnd
//
// Description:	for EC firmware leave flash mode and finished transmit data
//
// Input:
//      IN OUT OFBD_TC_54_STD_EC_STRUCT *ECStructPtr
//      IN OUT OFBD_HDR *pOFBDHdr
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
OFBDStdECEnd(
    IN OUT OFBD_TC_54_STD_EC_STRUCT *ECStructPtr, 
    IN OUT OFBD_HDR *pOFBDHdr)
{
    EFI_STATUS Status = EFI_SUCCESS;

    //Exit Programming Mode				0x05
    if(OFBDECCommand(EC_CMD_EXIT_PM) != EFI_SUCCESS)
    {
        ECStructPtr->dwRetSts |= OFBD_RS_EC_EXIT;      
        WriteKeyboardCommand(EC_CMD_EXIT_FLASHMODE);    //0xFE  
        return(RETURN_ERR);
    }

    if(pOFBDHdr->OFBD_OS & OFBD_OS_DOS)         
        WriteKeyboardCommand(EC_CMD_RESTART);    //0xFF
    else
        WriteKeyboardCommand(EC_CMD_EXIT_FLASHMODE);    //0xFE
        
    ECStructPtr->dwRetSts |= OFBD_RS_EC_OK;              

    return(Status);
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	OFBDStdECFlashHandle
//
// Description:	OFBD Standard EC Flash Handle
//
// Input:
//      IN OUT OFBD_TC_54_STD_EC_STRUCT *ECStructPtr
//      IN OUT OFBD_HDR *pOFBDHdr
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
OFBDStdECFlashHandle(
    IN OUT OFBD_TC_54_STD_EC_STRUCT *ECStructPtr, 
    IN OUT OFBD_HDR *pOFBDHdr)
{
    EFI_STATUS Status = EFI_SUCCESS;
    UINT8   ReturnErr = 0;

    USBKeyboardMouseDisable(ECStructPtr, &ReturnErr); 

    if(ECStructPtr->dbFlaSts & OFBD_TC_STD_EC_START) 
    {   
        //Entry flash mode
        ReturnErr = (UINT8)OFBDStdECStart(ECStructPtr);
    }
    else if(ECStructPtr->dbFlaSts & OFBD_TC_STD_EC_FLASH)                       
    {
        //Transmit data
        ReturnErr = (UINT8)OFBDStdECFlash(ECStructPtr);
    }
    else if(ECStructPtr->dbFlaSts & OFBD_TC_STD_EC_END)         
    {
        //Leave flash mode
        ReturnErr = (UINT8)OFBDStdECEnd(ECStructPtr, pOFBDHdr);
    }
    else
    {
        Status = RETURN_ERR;
    }
    
    USBKeyboardMouseEnable(ECStructPtr, &ReturnErr);

    return(Status);
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	OFBDEntryPoint
//
// Description:	OFBD Standard EC Flash Entry Point
//
// Input:
//      IN VOID             *Buffer
//      IN OUT UINT8        *pOFBDDataHandled
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID StdECEntry (
    IN VOID             *Buffer,
    IN OUT UINT8        *pOFBDDataHandled )
{
    OFBD_HDR *pOFBDHdr;
    OFBD_EXT_HDR *pOFBDExtHdr; 
    VOID *pOFBDTblEnd;
    OFBD_TC_54_STD_EC_STRUCT *ECStructPtr;    

    if(*pOFBDDataHandled == 0)
    {
        pOFBDHdr = (OFBD_HDR *)Buffer;
        pOFBDExtHdr = (OFBD_EXT_HDR *)((UINT8 *)Buffer + (pOFBDHdr->OFBD_HDR_SIZE));
        ECStructPtr = (OFBD_TC_54_STD_EC_STRUCT *)((UINT8 *)pOFBDExtHdr + sizeof(OFBD_EXT_HDR)); 
        pOFBDTblEnd = (VOID *)((UINT8 *)Buffer + (pOFBDHdr->OFBD_Size));    

        //TRACE((-1,"pOFBDHdr address is:%x ------\n",pOFBDHdr));
        //TRACE((-1,"pOFBDTblEnd address is:%x ------\n",*(UINT64 *)((UINT8 *)pOFBDTblEnd -sizeof(OFBD_END))));
        
        if(pOFBDHdr->OFBD_FS & OFBD_FS_EC)
        {   
            //Check Type Code ID
            if(pOFBDExtHdr->TypeCodeID == OFBD_EXT_TC_EC)
            {  
                //IoWrite32(0x300, *(UINT32 *)((UINT8 *)pOFBDTblEnd -4)); //debug

                if(OFBDStdECFlashHandle(ECStructPtr, pOFBDHdr) == EFI_SUCCESS)
                {
                    //Standard EC Flash Handled
                    *pOFBDDataHandled = 0xFF;      
                    return;
                }
                else
                {
                    //Not Standard EC Flash Data
                    *pOFBDDataHandled = 0;          
                    return;
                }
            } 
            else
            {
                 //Error occured
                *pOFBDDataHandled = 0xFE;          
                return;
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
