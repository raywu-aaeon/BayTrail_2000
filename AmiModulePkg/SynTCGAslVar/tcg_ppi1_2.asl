//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:    _DSM PPI Method for TPM device 
//
// Description:  Implement Phisical Presence Interface
//
// Input:        \_SB.PCI0.LPCB.TP
//
// Output:      PPI result
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************

DefinitionBlock (
	"tcg_ppi1_2.aml",
	"SSDT",
	1,
	"AMITCG",
	"_SynTCG_",
    1)
{

//External(\_SB, DeviceObj)

//#include<TPM_Device.asl>

//#include "..\..\Build\MyToken.asl"

    

Scope(\_SB)
{    

    Device(TPM)
    {
        Name(TMF1, 0x0) // TPM ASL update Variable
        Name(TMF2, 0x0) // TPM ASL update Variable
//        Name(TRST, 0x2)
#include "MyToken.asl"

        Method(_HID, 0){        //PnP Device ID
            Return(EISAID("PNP0C31"))
        }


        Name(_STR, Unicode ("TPM 1.2 Device"))
        Name(_UID,0x01)
        Name(_CRS,ResourceTemplate()
        {
        DWORDMEMORY(            // descriptor for TPM Memory Decode behind SystemBoard
            ResourceProducer,   // bit 0 of general flags is 0
            PosDecode,
            MinFixed,       // Range is fixed
            MaxFixed,       // Range is Fixed
            NonCacheable,
            ReadWrite,
            0x00000000,     // Granularity
            0x0FED40000,        // Min
            0x0FED44fff,        // Max
            0x00000000,     // Translation
            0x00005000,     // Range Length
            ,,
            IFXR
        )
        })
 
       Method(_STA, 0){
                    Return(0x0F)  // TPM Support 
       }
       
    OperationRegion (TSMI, SystemIO, SYNI , 0x2) 
    Field (TSMI, ByteAcc, NoLock, Preserve) 
    { 
        INQ,8,
        DAT,8,
    } 
    
    Method(SVAL, 1) {
        // For debug using, or the SMI method to communicate with BIOS.
        // Currently, using the EFI Runtime method.
        if( SYNI )
        {
            Store(ToInteger(Arg0), DAT)
            Store(SYNN, INQ)
        }
    }
    
    OperationRegion(PVAR,SystemMemory,0xFFFF0000,0xAA55)
    Field(PVAR,AnyAcc,NoLock,Preserve)
    {
        Offset(0x0),
    	P_WR,	8,	//   (00) Write PPI sumit request.
    	P_PD,	8,	//   (01) Retrive the Pending Operation
    	P_NR,	8,	//   (02) Retrive the most recent request
    	PMOR,	8,	//   (03) Write MOR sumit request
        P_RC,   32, //   (04~07) Corresponding TPM return code
        Offset(0x10),
        RP00,   8,
        RP01,   8,
        RP02,   8,
        RP03,   8,
        RP04,   8,
        RP05,   8,
        RP06,   8,
        RP07,   8,
        RP08,   8,
        RP09,   8,
        RP10,   8,
        RP11,   8,
        RP12,   8,
        RP13,   8,
        RP14,   8,
        RP15,   8,
        RP16,   8,
        RP17,   8,
        RP18,   8,
        RP19,   8,
        RP20,   8,
        RP21,   8,
        RP22,   8,
    }

    Method( _DSM , 4)
    {
        if( LEqual(Arg0,ToUUID("3DDDFAA6-361B-4EB4-A424-8D10089D1653")))
        {
                switch(ToInteger(Arg2))            
                {
                    //
                    // Function 0: Return supported funcitons
                    //
                    case(0)
                    {
                        return (Buffer() {0xff,0x01}) //support functions 1-8                       
                    }

                    //
                    // Function 1: Get PPI Version
                    //
                    case(1)
                    {
                        return ("1.2")                        
                    }

                    //
                    // Function 2: Submit TPM Operation request
                    // Arg3[0]: Integer - Operation Value
                    case(2)
                    {
						ToInteger(DeRefOf(Index(Arg3,0)), TMF2)  //save request in temp flag
                        // For PPI 1.2 OpNum sould less than 22
                        if(LGreater(TMF2, 0x16)){
                            return(0x1)
                        }
                        Store(TMF2,P_PD)
                        Store(TMF2,P_WR)
                        // SMI Emul Dbg +>
                        SVAL(0xFF)
                        // <+ SMI Emul Dbg
                        return (Zero) 		 //Success                       
                    }

                    //
                    // Function 3: Get pending TPM operation
                    case(3)
                    {
 	                	Name(PPI1, Package(){0,0})
                        if(LGreater(P_PD, 0x16)){
                            Store(0x01, Index(PPI1,0))
                            return(PPI1)
                        }

						Store(P_PD, Index(PPI1,1))
						return(PPI1)
                    }

                    //
                    // Function 4: Get platform-specific action to transition 
                    // ot Pre-OS
                    // Returns: 
                    // 0: None
                    // 1: Shutdown
                    // 2: Reboot
                    // 3: OS Vendor Specific
                    case(4)
                    {         
                        return (TRST) //Shutdown                      
                    }

                    //
                    // Function 5: Return TPM responce
                    //    
                    case(5)
                    {
	                	Name(PPI2, Package(){0,0,0})
                        if(Lequal(P_NR,0xFF)){
                            Store(0x01, Index(PPI2,0))
                            return(PPI2)
                        }

                        Store(P_NR, Index(PPI2,1)) 
                        Store(P_RC, Index(PPI2,2)) 
                        return(PPI2)
                    }


                    //
                    // Function 6: Submit preferred user language
                    // Ppi Spec 1.2 section 2.1.6      
                    //  Arg3[0]: String  - preferred language code
                    case(6)
                    {
                        return ( 0x03 ) //Success                             
                    }
                    
                    
                    //
                    // Function 7: Submit TPM Operation Request to Pre-OS Environment 2
                    // Ppi Spec 1.2 section 2.1.7   
                    //  Arg3[0]: String  - preferred language code
                    case(7)
                    {
                        ToInteger(DeRefOf(Index(Arg3,0)), TMF2)  //save request in temp flag
                        if(LGreater(TMF2, 0x16)){
                            return(0x1)
                        }
                        Store(TMF2,P_PD)
                        Store(TMF2,P_WR)
                        // SMI Emul Dbg +>
                        SVAL(0xFF)
                        // <+ SMI Emul Dbg
                        return (Zero) 		 //Success                       
                    }

                    case(8)
                    {
                        ToInteger(DeRefOf(Index(Arg3,0)), TMF2)  //save request in temp flag
                        switch(TMF2)
                        {
                            case(0)
                            { return (RP00) }
                            case(1)
                            { return (RP01) }
                            case(2)
                            { return (RP02) }
                            case(3)
                            { return (RP03) }
                            case(4)
                            { return (RP04) }
                            case(5)
                            { return (RP05) }
                            case(6)
                            { return (RP06) }
                            case(7)
                            { return (RP07) }
                            case(8)
                            { return (RP08) }
                            case(9)
                            { return (RP09) }
                            case(10)
                            { return (RP10) }
                            case(11)
                            { return (RP11) }
                            case(12)
                            { return (RP12) }
                            case(13)
                            { return (RP13) }
                            case(14)
                            { return (RP14) }
                            case(15)
                            { return (RP15) }
                            case(16)
                            { return (RP16) }
                            case(17)
                            { return (RP17) }
                            case(18)
                            { return (RP18) }
                            case(19)
                            { return (RP19) }
                            case(20)
                            { return (RP20) }
                            case(21)
                            { return (RP21) }
                            case(22)
                            { return (RP22) }
                            
                            default { return (Zero) }

                        }
                        return (Zero)                
                    }

                    default { }                    
                }
        } else {if(LEqual(Arg0,
        ToUUID("376054ED-CC13-4675-901C-4756D7F2D45D"))){
            //
            // Reset Atack Mitigation
            //
             switch(ToInteger(Arg2))            
                {
                    //
                    // Function 0: Return supported funcitons
                    //
                    case(0)
                    {
                        return (Buffer() {0x3}) //support functions 0 and 1
                    }

                    //
                    // Function 1: Set MOR Bit State
                    //
                    case(1)
                    {
                        ToInteger(DeRefOf(Index(Arg3,0)), TMF1)  //save request in temp flag
                        Store(TMF1, PMOR)

  	                    // SMI Emul Dbg +>
                        SVAL(0xFF)
                        // <+ SMI Emul Dbg
                        return (Zero) 		 
                    }
                    default { }                    
                }
            
        }}           
        return (Buffer() {0})
    }
    }
}

}
