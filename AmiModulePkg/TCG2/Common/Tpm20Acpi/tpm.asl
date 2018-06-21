//tpm.asl
External(CRBI)
    Device(\_SB.TPM)
	{
        Method(_HID, 0){		//PnP Device ID
		    If(TCMF)		
			    {
			    	Return(EISAID("ZIT0101"))
				}	
		    Else
			    {
				  If(LEqual(TTDP, 0)){
                    Return(EISAID("PNP0C31"))
                  }else{
                     Return("MSFT0101")
                  }
				}
	    }


	    Method(_STR,0)
        {
           If(LEqual(TTDP, 0)){
                Return (Unicode ("TPM 1.2 Device"))
           }else {
                Return (Unicode ("TPM 2.0 Device"))
           }
        }
        
	    Name(_UID,0x01)
	    
      Name(CRST,ResourceTemplate()
	  {
            Memory32Fixed (ReadOnly, 0x00000000, 0x1000,PCRB)
            Memory32Fixed (ReadOnly, 0xFED70000, 0x1000,PCRC)
	  })
	  
	  Name(CRSD,ResourceTemplate()
      {
            Memory32Fixed (ReadOnly, 0xFED40000, 0x1000, PCRE)
      })

      Name(CRSI,ResourceTemplate()
	  {
            Memory32Fixed (ReadOnly, 0xFED40000, 0x1000,PCRS)            
	  })
      //
      // Return the resource consumed by TPM device
      //
      Method(_CRS,0,Serialized)
	  {
         If(LEqual(AMDT, 1))
         {
             CreateDWordField(CRST, ^PCRB._BAS, MTFB) // Min
             CreateDWordField(CRST, ^PCRB._LEN, LTFB) // Length
                   
             Store(TPMB, MTFB)
             Store(0x1000, LTFB)

             CreateDWordField(CRST, ^PCRC._BAS, MTFC) // Min
             CreateDWordField(CRST, ^PCRC._LEN, LTFC) // Length
               
             Store(TPMC, MTFC)
             Store(0x1000, LTFC)
            
             Return (CRST)
         }Else{
            If(LEqual(DTPT,1))
            {
               CreateDWordField(CRSD, ^PCRE._BAS, MTFE) // Min
               CreateDWordField(CRSD, ^PCRE._LEN, LTFE) // Length
               
               Store(0x0FED40000, MTFE)
               Store(0x00000880, LTFE)
                              
               Return(CRSD)
            }            
            ElseIf(LEqual(TTPF, 1))
            {
               CreateDWordField(CRSI, ^PCRS._BAS, MTFD) // Min
               CreateDWordField(CRSI, ^PCRS._LEN, LTFD) // Length
               
               Store(0x0FED40000, MTFD)
               Store(0x00005000, LTFD)
               Return (CRSI)
            }ElseIf(LEqual(TTPF, 0))
            {
               CreateDWordField(CRSI, ^PCRS._BAS, MTFF) // Min
               Store(TPMM, MTFF)
               Return (CRSI)
            }
         }
	  }

	  OperationRegion(TMMB, SystemMemory, 0x0FED40000, 0x5000)
	  Field(TMMB, ByteAcc, Lock, Preserve)
	  {
	    Offset(0x04),
	    LCST, 32,          // LOC_STATE
	    Offset(0x40),
        CREQ, 32,          // CTRL_REQ
	    Offset(0x44), 
	    CSTS, 32,          // CTRL STS
	    Offset(0x4C),
	    SCMD, 32,          // CTRL STRT
	  }

      OperationRegion (CRBD, SystemMemory, TPMM, 0x48)
      Field (CRBD, AnyAcc, NoLock, Preserve)
      {
        Offset(0x04),
        HERR, 32,
        Offset (0x40),
        HCMD, 32,
        Offset(0x44), 
        HSTS, 32,
      }

//AMI_OVERRIDE_START : Support FTPM  //EIP226550 >>
      OperationRegion (TICR, SystemMemory, 0xE00D0000, 0x100)
      Field (TICR, AnyAcc, NoLock, Preserve)
      { 
        Offset(0x8C),
        STAT, 32,
      }
//AMI_OVERRIDE_END //EIP226550 <<

      Method(_STA, 0){
	   	If(LEqual(TTDP, 0)){
            If(TPMF){
                 Return(0x0F)  // TPM Support 
             }
             Return(0x00)  // No TPM Support
         }ElseIF(LEqual(TTDP, 1)){
            If(TPMF){
             Return(0x0F)  // TPM Support 
            }
            return (0x00)
         }  
	   }
   
       Method (STRT, 3, Serialized, 0, IntObj, {UnknownObj, UnknownObj, UnknownObj}) // IntObj, IntObj, PkgObj
       {
          OperationRegion (TPMR, SystemMemory, FTPM, 0x1000)
          Field (TPMR, AnyAcc, NoLock, Preserve)
          {
            Offset(0x04),
            FERR, 32,
            Offset(0x0c),
            BEGN, 32,
          }
          
          Name (TIMR, 0)  
		  
          //
          // Switch by function index
          //
          Switch (ToInteger (Arg1))
          {
              Case (0)
              {
                //
                // Standard query, supports function 1-1
                //
                Return (Buffer () {0x03})
              }
              Case (1)
              {   
//AMI_OVERRIDE_START : Support FTPM  EIP226550 >>
#if 0
                Store(0, TIMR)
                
                If(LEqual(AMDT, 1))
                {
                  While(LAND(LEqual(BEGN, One), LLESS(TIMR, 0x200)))
       		      {
        			 If(LEqual(BEGN, One))
    				 {
    				   Sleep(0x1)
    				   Increment(TIMR)
    		         }
    			  }
    			  Return(Zero)
                }Else{
                  If(LEqual(Or(And(HSTS,0x00000002),And(HSTS,0x00000001)),0x00000003))
                  {
                    //
                    // Trigger the FTPM_CMD interrupt
                    //
                    Store (0x00000001, HCMD)
                  }              
                  Else
                  {
                    //Set Error Bit
                    Store(0x00000001,FERR)
                    //Clear Start Bit
                    Store(0x00000000,BEGN)
                  }

                  Return (0)     
              }
#endif

                Store(0x00000001,STAT)
                Return (0)
//AMI_OVERRIDE_END EIP226550 <<
           }    
        }
	  
	    Return (1)    
	  }

      Method (CRYF, 3, Serialized, 0, {BuffObj, PkgObj}, {UnknownObj, UnknownObj, UnknownObj}) // IntObj, IntObj, PkgObj
      {
        //
        // Switch by function index
        //
        Switch (ToInteger(Arg1))
        {
          Case (0)
          {
            //
            // Standard query
            //
            Return (Buffer () {0x03})
          }
          Case (1)
          {
            //
            // Return failure if no TPM present
            //
            Name(TPMV, Package () {0x01, Package () {0x1, 0x20}})
            if (LEqual (_STA (), 0x00))
            {
              Return (Package () {0x00})
            }
            Return (TPMV)
          }
        }
        Return (Buffer () {0})
      }

  }