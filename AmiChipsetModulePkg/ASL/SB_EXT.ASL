//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
// $Header: /Alaska/BIN/Chipset/Intel/SouthBridge/CougarPoint_A/Chipset/ASL/SBExt.asl 3     3/07/11 4:55a Yurenlai $
//
// $Revision: 3 $
//
// $Date: 3/07/11 4:55a $
//*************************************************************************
Scope(\){

//;<AMI_PHDR_START>
//;------------------------------------------------------------------------
//;
//; Procedure:    UXDV
//;
//; Description:  Convert UART port address to the decoded value in LPC bridge
//;
//; Input:        Arg0 = Port to Route/Release 
//;
//; Output:       UART Port Decoded Value in the LPC bridge
//;
//;------------------------------------------------------------------------
//;<AMI_PHDR_END>

    Method(UXDV, 1, Serialized) {
    // Porting If needed.
    }


//;<AMI_PHDR_START>
//;------------------------------------------------------------------------
//;
//; Procedure:    RRIO
//;
//; Description:  Route/Release I/O resources from/to EIO/LPC Bus 
//;
//; Input:        Arg0 = Device Category
//;               Arg1 = 0/1 Disable/Enable resource decoding
//;               Arg2 = Port to Route/Release
//;               Arg3 = Port SIZE to Route
//;
//; Output:       Nothing
//;
//;------------------------------------------------------------------------
//;<AMI_PHDR_END>

    Method(RRIO, 4, Serialized) {
    // Porting If needed.
    }


//;<AMI_PHDR_START>
//;------------------------------------------------------------------------
//;
//; Procedure:    rDMA
//;
//; Description:  Route/Release DMA channel from/to being ISA/PCI mode
//;
//; Input:        Arg0 = Device Category
//;               Arg1 = 0/1 Disable/Enable resource decoding
//;               Arg2 = DMA channel to Route/Release
//;
//; Output:       Nothing
//;
//;------------------------------------------------------------------------
//;<AMI_PHDR_END>

    Method(RDMA, 3) {
    // Porting If needed.
    }
} //end Scope(\)

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
