//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/AmiTcgOemlib/AmiTcgOemlib.c 4     11/14/11 1:25p Fredericko $
//
// $Revision: 4 $
//
// $Date: 11/14/11 1:25p $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:  AmiTcgOemlib.c
//
// Description: Contains Oem functions used for TPM
//      
//
//<AMI_FHDR_END>
//**********************************************************************
#include "AmiTcgOemlib.h"
#include <AmiPeiLib.h>
#include <token.h>



BOOLEAN iTPMVerifyMeStatus( )
{
    //***********PORTING REQUIRD BEGIN********************//
    return TRUE;
    //***********PORTING REQUIRD END**********************//
}





BOOLEAN OemTPM_EnterConfig( )
{
    //***********PORTING REQUIRD BEGIN********************//
    //sample Io config mode is 0x55
    IoWrite8( TPM_IOBASE2, 0x55 );
    return TRUE;
    //***********PORTING REQUIRD END**********************//
}






BOOLEAN OemTPM_ExitConfig( )
{
    //***********PORTING REQUIRD BEGIN********************//
    //sample Io config mode is 0x55
    IoWrite8( TPM_IOBASE2, 0xAA );
    return TRUE;
    //***********PORTING REQUIRD END**********************//
}






UINT8 Read_Tpm_Chip(
   IN  UINT8 Val )
{
    //***********PORTING REQUIRD BEGIN********************//
    OemTPM_EnterConfig( );
    IoWrite8( TPM_IOBASE2, Val );
    Val = IoRead8( TPM_IOBASE2 + 1 );
    return Val;
    //***********PORTING REQUIRD END**********************//
}




EFI_STATUS Configure_Tpm_Chip( )
{
    //Preparation: Decode range in Generic decode should be
    //programmed before this function is called (Example done for Nuvoton WPCT200 TPM
    //***********PORTING REQUIRD BEGIN********************//
    UINT8 Byte = (UINT8)( TPM_IOBASE >> 8 );

         IoWrite8( TPM_IOBASE2,       0x07 );
         IoWrite8( (TPM_IOBASE2 + 1), 0x1A );
         IoWrite8( TPM_IOBASE2,       0x60 );
         IoWrite8( TPM_IOBASE2 + 1,   Byte );
     
         Byte = (UINT8)( TPM_IOBASE & 0xFF );

         IoWrite8( TPM_IOBASE2,       0x61 );
         IoWrite8( TPM_IOBASE2 + 1,   Byte );
         IoWrite8( TPM_IOBASE2,       0x30 );
         IoWrite8( TPM_IOBASE2 + 1,    0x1 );

    if ( IoRead8( TPM_IOBASE ) == 0xFF )
    {
        return EFI_DEVICE_ERROR;
    }

    if ( IoRead8( TPM_IOBASE2 ) == 0xFF )
    {
        return EFI_DEVICE_ERROR;
    }
    else {
        return EFI_SUCCESS;
    }
/*
    Example for Infineon TPM 1.2 chip 

//1. Enable the 4E/4f registers for a microcontroller in the LPC registers. (Bit 13 of D31, Func 0 reg 83) in ICH 8
//2. Enter the percieved decode range into one of the LPC's Generic Decode registers. (4701).
//3. Enter config mode of TPM by writing 55 to 4e. (4f will become zero if succesful)
//4. Write 30 to 4e port.
//5. Write 26 to 4e and verify that 4f returns with value 4e.
//6. Entering Base to TPM. (write 60h to port 4e)
//7. write 47 to port 4f
//8. write 61 to port 4e
//9. write 00 to port 4f
//10. write 70 to port 4e
//11. write 00 to port 4f
//12. write 71 to port 4e
//13. write 02, port 4f
//14. write 30,port 4e
//15. write 01, port 4f
//16. close config mode by writing aa to port 4e
//17. Release MMIO by writing 20 to FED4000h
    
    IoWrite8( TPM_IOBASE2,       0x55 );      //enter config mode
    if(IoRead8( TPM_IOBASE2_DATA ) != 0x00) return EFI_DEVICE_ERROR; //verify 4f is decoded
    IoWrite8( TPM_IOBASE2,       0x30 );      //Write 30 to 4e port
    IoWrite8( TPM_IOBASE2,       0x26 );      //26 to 4e 
    if(IoRead8( TPM_IOBASE2_DATA ) != 0x4E) return EFI_DEVICE_ERROR;  //4f returns with value 4e
    IoWrite8( TPM_IOBASE2,       0x60 );      //Entering Base to TPM. (write 60h to port 4e)
    IoWrite8( TPM_IOBASE2_DATA,       0x47 ); //write 47 to port 4f
    IoWrite8( TPM_IOBASE2,       0x61 );      //write 61 to port 4e
    IoWrite8( TPM_IOBASE2_DATA,       0x00 ); //write 00 to port 4f
    IoWrite8( TPM_IOBASE2,       0x70 );      //write 70 to port 4e
    IoWrite8( TPM_IOBASE2_DATA,       0x00 ); //write 00 to port 4f
    IoWrite8( TPM_IOBASE2,       0x71 );      //write 71 to port 4e
    IoWrite8( TPM_IOBASE2_DATA,       0x02 ); //write 02, port 4f
    IoWrite8( TPM_IOBASE2,       0x30 );      //write 30,port 4e
    IoWrite8( TPM_IOBASE2_DATA,       0x01 ); //write 01, port 4f
    IoWrite8( TPM_IOBASE2,       0xaa );      //close config mode by writing aa to port 4e
    *(UINT8 *)(UINTN)(0xFED40000) = 0x20;     //Release MMIO by writing 20 to FED4000h
    
    return EFI_SUCCESS;

*/
    //***********PORTING REQUIRD END**********************//
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**     5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093            **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
