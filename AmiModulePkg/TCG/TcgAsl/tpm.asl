//tpm.asl
    Device(\_SB.TPM)
	{
        Method(_HID, 0){		//PnP Device ID
		    If(TCMF)		
			    {
			    	Return(EISAID("ZIT0101"))
				}	
		    Else
			    {
				  Return(EISAID("PNP0C31"))
				}
	    }


	    Name(_STR, Unicode ("TPM 1.2 Device"))
	    Name(_UID,0x01)
	    Name(_CRS,ResourceTemplate()
	    {
		DWORDMEMORY(			// descriptor for video RAM behind ISA bus
	        ResourceConsumer,	// bit 0 of general flags is 0
            PosDecode,
	        MinFixed,		// Range is fixed
	        MaxFixed,		// Range is Fixed
	        NonCacheable,
	        ReadWrite,
	        0x00000000,		// Granularity
	        0x0FED40000,		// Min
	        0x0FED44fff,		// Max
	        0x00000000,		// Translation
	        0x00005000,		// Range Length
			,,
			IFXR
	    )
	    })

	  OperationRegion(TMMB, SystemMemory, 0x0FED40000, 0x5000)
	  Field(TMMB, ByteAcc, Lock, Preserve)
	  {
	    Offset(0x0000),
	    ACCS, 8,          // Access
	    Offset(0x0018), 
	    TSTA, 8,          // Status 
	    TBCA, 8,          // Burst Count
	    Offset(0x0F00),
	    TVID, 16,         // TPM Chip VID
	    TDID, 16          // TPM Chip DID
	  }

	   Method(_STA, 0){	   		
			if(TPMF) {
       		  		Return(0x0F)  // TPM Support 
   	 		 }
   			Return(0x00)  // No TPM Support
	   }
	 
	}
