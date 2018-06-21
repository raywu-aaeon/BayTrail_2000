External(ASL_SG_ULT_RP_NUM.LNKD)
External(ASL_DGPUPCH_SCOPE.TGPC, MethodObj)
External(ASL_DGPUPCH_SCOPE.NGC6, MethodObj)

#define JT_REVISION_ID        0x00000103               // Revision number
#define JT_FUNC_SUPPORT       0x00000000               // Function is supported?
#define JT_FUNC_CAPS          0x00000001               // Capabilities
#define JT_FUNC_POWERCONTROL  0x00000003               // dGPU Power Control
#define JT_FUNC_PLATPOLICY    0x00000004               // Platform Policy
#define JT_FUNC_DISPLAYSTATUS 0x00000005               // Query the Display Hot-Key
#define JT_FUNC_MDTK          0x00000006               // Display Hot-Key Toggle List


Scope (ASL_DGPUPCH_SCOPE)
{
  Name(TGPC, Buffer(0x04)
  {
           0x00
  }
  )
  Method(GC6I, 0, Serialized)
  {
    Store("<<< GC6I >>>", Debug)
    // 1. Disable PCIE_Link
    Store(One, ASL_SG_ULT_RP_NUM.LNKD)
			
  }
  Method(GC6O, 0, Serialized)
  {
    Store("<<< GC6O >>>", Debug)
    // 1. Sensing GC6_FB_EN = 1
    While(LNotEqual(ASL_DGPUPCH_SCOPE.SGPI(ASL_GPIO_GC6_FB_EN), Zero))
    {
       Sleep(One)
    }
    // 2. Enable PCIE_Link
    Store(Zero, ASL_SG_ULT_RP_NUM.LNKD)
    // 3. Asserts GPU_EVENT#
//    ASL_DGPUPCH_SCOPE.SGPO(ASL_GPU_EVENT, One)
    // 4. Sensing GC6_FB_EN = 0
    While(LNotEqual(ASL_DGPUPCH_SCOPE.SGPI(ASL_GPIO_GC6_FB_EN), One))
    {
        Sleep(One)
    }
    // 5. De-asserts GPU_EVENT#
//    ASL_DGPUPCH_SCOPE.SGPO(ASL_GPU_EVENT, Zero)
   }

   Method(GETS, 0, Serialized)
   {
          If(LEqual(ASL_DGPUPCH_SCOPE.SGPI(ASL_GPIO_GC6_FB_EN), One))
          {
                Store("<<< GETS() return 0x1 >>>", Debug)
                Return(One)
          }Else
          {
                Store("<<< GETS() return 0x3 >>>", Debug)
                Return(0x03)
          }
    }


//<AMI_PHDR_START>
//------------------------------------------------------------------------
//
// Procedure:    NGC6
//
// Description:  Called from _DSM -Device Specific Method for dGPU device.
//               Implement Ventura specific callback functions
//
// Input:
// Arg0:   UUID      Unique function identifier. 
//               Ventura DSM_GUID CBECA351-067B4924-9CBDB46B00B86F34
// Arg1:   Integer   Revision Level
// Arg2:   Integer   Function Index (0 = Return Supported Functions)
// Arg3:   Package   Parameters
//
// Output:
//  Sub-function 0 and unsupported function calls always returns a buffer. 
//  Other subfunctions may return a buffer or a package as defined in the function.
//  When a single DWord is returned the following values have special meaning,
//  controlled by reserved Bit31 as follows:
//      MXM_ERROR_SUCCESS 0x00000000 Success
//      MXM_ERROR_UNSPECIFIED 0x80000001 Generic unspecified error code
//      MXM_ERROR_UNSUPPORTED 0x80000002 FunctionCode or SubFunctionCode not
//          supported by this system
//-------------------------------------------------------------------------
//<AMI_PHDR_END>
    Method (NGC6, 4, NotSerialized)
    {

        Store("------- GC6 DSM --------", Debug)
        // Only Interface Revision 0x0100 is supported
        If (LLess(Arg1, 0x100))
        {
                Return(0x80000001)
        }

        // (Arg2) Sub-Function
        Switch (ToInteger(Arg2))
        {
            //
            // Function 0:  
            //
            case (JT_FUNC_SUPPORT)
            {
				Return(Buffer(0x04)
				{
					0x1B, 0x00, 0x00, 0x00
				})
            }
            //
            // Function 1:  
            //
            case (JT_FUNC_CAPS)
            {

                Name(JTB1, Buffer(0x4)
                {
    				0x00
                })
				CreateField(JTB1,Zero,One,JTEN)
				CreateField(JTB1,One,0x02,SREN)
				CreateField(JTB1,0x03,0x03,PLPR)
				CreateField(JTB1,0x06,0x02,FBPR)
				CreateField(JTB1,0x08,0x02,GUPR)
				CreateField(JTB1,0x0A,One,GC6R)
				CreateField(JTB1,0x0B,One,PTRH)
                                CreateField(JTB1,0x0F,0x02,GC6M)
				CreateField(JTB1,0x14,0x0C,JTRV)
				Store(One, JTEN)     // JT enable
				Store(One, GC6R)     // GC6 integrated ROM
				Store(One, PTRH)     // No SMI Handler
                                Store(One, GC6M)
				Store(One, SREN)     // Disable NVSR
				Store(JT_REVISION_ID, JTRV)  // JT rev

                Return(JTB1)
            }
            //
            // Function 2:  
            //
	        case(0x00000002)
    		{
                Store("GPS fun 19", Debug)
                return(arg3)
    		}
            //
            // Function 3:  
            //
	        case(0x00000003)
    		{
				CreateField(Arg3,Zero,0x03,GUPC)
				CreateField(Arg3,0x04,One,PLPC)
				Name(JTB3, Buffer(0x04)
				{
					0x00
				})
				CreateField(JTB3,Zero,0x03,GUPS)
				CreateField(JTB3,0x03,One,GPGS)	    // dGPU Power status 
				CreateField(JTB3,0x07,One,PLST)
				If(LEqual(ToInteger(GUPC), One))           // EGNS 
				{
                                        Store(Arg3,TGPC)         // Store GC6 control input for GC6I GC6O
					GC6I()
					Store(One, PLST)
				}
				Else
				{
					If(LEqual(ToInteger(GUPC), 0x02))  // EGIS 
					{
                                                Store(Arg3,TGPC)         // Store GC6 control input for GC6I GC6O
						GC6I()
						If(LEqual(ToInteger(PLPC), Zero))
						{
							Store(Zero, PLST)
						}
					}
					Else
					{
						If(LEqual(ToInteger(GUPC), 0x03)) // XGXS 
						{
							Store(Arg3,TGPC)         // Store GC6 control input for GC6I GC6O
							GC6O()
							If(LNotEqual(ToInteger(PLPC), Zero))
							{
								Store(Zero, PLST)
							}
						}						
						Else
						{
							If(LEqual(ToInteger(GUPC), 0x04)) // XGIS
							{
								Store(Arg3,TGPC)         // Store GC6 control input for GC6I GC6O
								GC6O()
								If(LNotEqual(ToInteger(PLPC), Zero))
								{
									Store(Zero, PLST)
								}
							}
							Else
							{
								If(LEqual(ToInteger(GUPC), Zero)) 
								{
									Store(GETS(), GUPS)
									If(LEqual(ToInteger(GUPS), 0x01)) 
									{
                                  					   Store(One, GPGS)       // dGPU power status is Power OK
									}
									Else
									{
									    Store(Zero, GPGS)       // dGPU power status is Power off
									} 
								  }
								  Else 
								  {
								      If(LEqual(ToInteger(GUPC), 0x6))  
								      { 
								   
								  } 
								}
							}
						}
					}
				}
				Return(JTB3)
    		}
            //
            // Function 4:  
            //
	        case(JT_FUNC_PLATPOLICY)
    		{
                Return(0x80000002)
    		}

        } // end of switch

        Return(0x80000002)
    } // end NGC6


} // end DGPUPCH_SCOPE scope
