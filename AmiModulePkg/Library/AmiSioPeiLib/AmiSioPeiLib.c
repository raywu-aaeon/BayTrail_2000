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
#include <Library\AmiSioPeiLib.h>
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
    UINT32                  NumOfBdf;
//	    UINT16                  ComRange[] = { 0x3f8, 0x2f8, 0x220, 0x228,\
//	                                           0x238, 0x2e8, 0x338, 0x3e8, 0};
    UINT16                  ComRange[] = { 0x3f8, 0x2f8, 0x220, 0x228,\
                                           0x238, 0x2e8, 0x338, 0x3e8, 0x2f0, 0x2e0, 0};
    UINT16                  LptRange[] = { 0x378, 0x278, 0x3bc, 0};
    UINT16                  FpcRange[] = { 0x3f0, 0x370, 0};
    UINT16                  IoRangeMask16 = 0xffff;
    UINT16                  IoRangeSet16 = 0;
    UINT16                  IoEnMask16 = 0xffff;
    UINT16                  IoEnSet16 = 0;
    UINT8                   i;

    switch (Type) {
        // FDC Address Range
        case (dsFDC) :
            if (Base == 0) IoEnMask16 &= ~BIT03;
            else {
                for (i = 0; (FpcRange[i] != 0) && (FpcRange[i] != Base); i++);
                if (FpcRange[i]) {
                    IoEnSet16 |= BIT03;
                    IoRangeMask16 &= ~BIT12;
                    IoRangeSet16 |= (i << 12);
                }
                else return EFI_UNSUPPORTED;
            }
            break;

        // LPT Address Range
        case (dsLPT) :
            if (Base == 0) IoEnMask16 &= ~BIT02;
            else {
                for (i = 0; (LptRange[i] != 0) && (LptRange[i] != Base); i++);
                if (LptRange[i]) {
                    IoEnSet16 |= BIT02;
                    IoRangeMask16 &= ~(BIT09 | BIT08);
                    IoRangeSet16 |= (i << 8);
                } else return EFI_UNSUPPORTED;
            }
            break;

        // ComA Address Range
        case (dsCIR) :
        case (dsUART) :
            if (Base == 0) {//Dxe Phase to Close PEI phase opened decode
                if (DevUid==2) IoEnMask16 &= ~BIT01;//close COMB decode
                else if (DevUid==1) IoEnMask16 &= ~BIT00; //close COMA decode
            } else {
                for (i = 0; (ComRange[i] != 0) && (ComRange[i] != Base); i++);
                if (ComRange[i]) {
                    if (DevUid == 1) {  //COMA decode
                        IoEnSet16 |= BIT00;
                        IoRangeMask16 &= ~(BIT02 | BIT01 | BIT00);
                        IoRangeSet16 |= i;
                    }else if (DevUid == 2) {  //COMB decode
                        IoEnSet16 |= BIT01;
                        IoRangeMask16 &= ~(BIT06 | BIT05 | BIT04);
                        IoRangeSet16 |= (i << 4);
                    };
                } else return EFI_UNSUPPORTED;
            }
            break;

        // KBC Address Enable
        case (dsPS2K) :
        case (dsPS2CK) :
            if (Base == 0) IoEnMask16 &= ~BIT10;
            else IoEnSet16 |= BIT10;
            break;

        case (dsPS2M) :
        case (dsPS2CM) :
            break;
        // Game Port Address Enable
        case (dsGAME) :
            if (Base == 0) IoEnMask16 &= ~(BIT09 | BIT08);
            else {
                if (Base == 0x200) {
                    IoEnSet16 |= BIT08;
                } else {
                    if (Base == 0x208) IoEnSet16 |= BIT09;
                    else return EFI_UNSUPPORTED;
                }
            }
            break;

        // LPC CFG Address Enable
        case (0xff) :
            if (Base == 0x2e) IoEnSet16 |= BIT12;
            else {
                if (Base == 0x4e) IoEnSet16 |= BIT13;
                else {
                    //!!!Attention!!!If type is 0xFF, DevUid contain the IO length
                    if (Base) {
                        AmiSioLibSetLpcGenericDecoding( LpcPciIo, \
                                                      Base , \
                                                     DevUid, \
                                                       TRUE );
                        return EFI_SUCCESS;
                    } else return EFI_UNSUPPORTED;
                }
            }
            break;

        default :
            return EFI_UNSUPPORTED;
    }

    NumOfBdf = (UINT32)(PCI_CF8_LIB_ADDRESS(SIO_SB_BUS_NUMBER,SIO_SB_DEV_NUMBER, SIO_SB_FUNC_NUMBER, 0));
    //NumOfBdf=(UINT32)(SIO_SB_BUS_NUMBER << 16) | (SIO_SB_DEV_NUMBER << 11) | (SIO_SB_FUNC_NUMBER << 8);


    //  RWPciReg16(NumOfBdf,ICH_LPC_IO_DECODE_OFFSET, IoRangeSet16, ~IoRangeMask16);	//0X82
    PciCf8AndThenOr16((UINTN)NumOfBdf+ICH_LPC_IO_DECODE_OFFSET,IoRangeMask16, IoRangeSet16);

    //RWPciReg16(NumOfBdf,ICH_LPC_IO_ENABLE_OFFSET, IoEnSet16, ~IoEnMask16);		//0X84
    PciCf8AndThenOr16(NumOfBdf+ICH_LPC_IO_ENABLE_OFFSET,IoEnMask16,IoEnSet16);
    // Porting End

    return EFI_SUCCESS;
}

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

