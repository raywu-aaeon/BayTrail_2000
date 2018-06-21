//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
//
//*************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  <F81866PEI.C>
//
// Description: Porting for PEI phase.Just for necessary devices porting.
//
//
//<AMI_FHDR_END>
//*************************************************************************
//-------------------------------------------------------------------------
// Include Files
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// Constants, Macros and Type Definitions
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
//Variable, Prototype, and External Declarations
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// Include Files
//-------------------------------------------------------------------------
#include <Efi.h>
#include <Pei.h>
#include <Token.h>
#include <AmiPeiLib.h>
#include <Protocol\PciIo.h>
#include <Protocol\AmiSio.h>
#include <Library\AmiSioPeiLib.h> //CSP20131230
#include <Library\PciCf8Lib.h>

//-------------------------------------------------------------------------
//Variable, Prototype, and External Declarations
//-------------------------------------------------------------------------
#define ICH_LPC_IO_DECODE_OFFSET   	0x80
#define ICH_LPC_IO_ENABLE_OFFSET   	0x82
#define ICH_LPC_REG_GEN1_DEC_OFFSET 0x84
#define SIO_SB_BUS_NUMBER    	   	0x00
#define SIO_SB_DEV_NUMBER   	   	0x1F
#define SIO_SB_FUNC_NUMBER  	   	0x00


//<AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure:   CSP_SetLpcGenericDecoding
//
// Description:
//  This function set LPC Bridge Generic Decoding
//
// Input:
//  *LpcPciIo - Pointer to LPC PCI IO Protocol
//  Base      - I/O base address
//  Length    - I/O Length
//  Enabled   - Enable/Disable the generic decode range register
//
// Output:
//  EFI_SUCCESS - Set successfully.
//  EFI_UNSUPPORTED - This function is not implemented or the
//                    Length more than the maximum supported
//                    size of generic range decoding.
//  EFI_INVALID_PARAMETER - the Input parameter is invalid.
//  EFI_OUT_OF_RESOURCES - There is not available Generic
//                         Decoding Register.
//  EFI_NOT_FOUND - the generic decode range will be disabled
//                  is not found.
//
// Notes:
//
//-------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS AmiSioLibSetLpcGenericDecoding (
    IN VOID				       *LpcPciIo,
    IN UINT16                   Base,
    IN UINT16                   Length,
    IN BOOLEAN                  Enable )
{
    // Porting Required
    UINT32                  IoGenDecode32,NumOfBdf;
    UINT16                  IoGenDecIndex;
    UINT16                  Buffer16;
    UINT8                   Bsf8 = 0;
    UINT8                   Bsr8 = 0;

    if (Length > 0x100) return EFI_UNSUPPORTED;

    if (Length == 0) return EFI_INVALID_PARAMETER;

    if (Length < 4) Length = 4;

    //NumOfBdf = (UINT32) (SIO_SB_BUS_NUMBER << 16)|(SIO_SB_DEV_NUMBER << 11) |(SIO_SB_FUNC_NUMBER << 8);
    NumOfBdf = (UINT32)(PCI_CF8_LIB_ADDRESS(SIO_SB_BUS_NUMBER,SIO_SB_DEV_NUMBER, SIO_SB_FUNC_NUMBER, 0));

    // Read I/O Generic Decodes Register.
    for (IoGenDecIndex = 0; IoGenDecIndex < 4; IoGenDecIndex++) {
        //IoGenDecode32 = ReadPciReg32(NumOfBdf,ICH_LPC_REG_GEN1_DEC_OFFSET + IoGenDecIndex * 4);
    	IoGenDecode32=PciCf8Read32(NumOfBdf+(ICH_LPC_REG_GEN1_DEC_OFFSET + IoGenDecIndex * 4));

        if (Enable) {
            if ((IoGenDecode32 & 1) == 0) break;
            else if ((IoGenDecode32 & 0xfffc) == Base) break;
        } else {
            if (((IoGenDecode32 & 0xfffc) == Base) && (IoGenDecode32 & 1)) {
                IoGenDecode32 = 0; // Disable & clear the base/mask fields
                break;
            }
        }
    }

    if (Enable) {
        if (IoGenDecIndex == 4) return EFI_OUT_OF_RESOURCES;

        Buffer16 = Length;
        while ((Buffer16 % 2) == 0) {
            Buffer16 /= 2;
            Bsf8++;
        }

        while (Length) {
            Length >>= 1;
            Bsr8++;
        }

        if (Bsf8 == (Bsr8 - 1)) Bsr8--;

        Length = (1 << Bsr8) - 1 ;

        Base &= (~Length);

        IoGenDecode32 = Base | (UINT32)((Length >> 2) << 18) | 1;

    } else {
        if (IoGenDecIndex == 4) return EFI_NOT_FOUND;
    }

    //WritePciReg32(NumOfBdf,ICH_LPC_REG_GEN1_DEC_OFFSET + IoGenDecIndex * 4, IoGenDecode32);
    PciCf8Write32(NumOfBdf+(ICH_LPC_REG_GEN1_DEC_OFFSET + IoGenDecIndex * 4),IoGenDecode32);

    // Porting End

    return EFI_SUCCESS;

}


//<AMI_PHDR_START>
//-------------------------------------------------------------------------
// Procedure:  CSP_SetLpcDeviceDecoding
//
// Description:
//  This function goes through the elinked list of identify functions
//  giving control when the token "IODECODETYPE == 1".
//
// Input:
//  Base    - I/O base address
//            Base=0 means disable the decode of the device
//  DevUid  - The device Unique ID
//            If type is 0xFF, DevUid contain the IO length
//  Type    - Device type
//            If type is 0xFF, DevUid contain the IO length
//
// Output:
//  EFI_SUCCESS - Set successfully.
//  EFI_INVALID_PARAMETER - the Input parameter is invalid.
//
// Notes:
//  Chipset porting should provide the Io Ranage decode function.
//  If chipset porting provide this function, set IODECODETYPE = 0.
//  If chipset porting doesn't provide this function, you can eLink your
//  function to IoRangeDecodeList or replace CSP_SetLpcDeviceDecoding elink
//
//-------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS AmiSioLibSetLpcDeviceDecoding(
    IN    VOID           *LpcPciIo,
    IN    UINT16         Base,
    IN    UINT8          DevUid,
    IN    SIO_DEV_TYPE   Type)
{

    return EFI_SUCCESS;
}

//CSP20131230 >>
// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure: ProgramRtRegisterTable
//
// Description:
//  This function will program the runtime register.
//
// Input:
//  IN  UINT16  Base - Runtime register IO base
//  IN  SIO_DEVICE_INIT_DATA  *Table - initial table
//  IN  UINT8   Count - table data count
// Output:
//  NONE
//
//-------------------------------------------------------------------------
// <AMI_PHDR_END>
void ProgramRtRegisterTable(
    IN  UINT16  Base,
    IN  SIO_DEVICE_INIT_DATA  *Table,
    IN  UINT8   Count
)
{
    UINT8   i;
    UINT8   Value8;

    for(i=0;i<Count;i++) {
        Value8 = (IoRead8(Base+Table[i].Reg16) & Table[i].AndData8)| Table[i].OrData8;
        IoWrite8(Base+Table[i].Reg16, Value8 );
    }
}
//CSP20131230 <<

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

