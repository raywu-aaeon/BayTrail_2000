/*++

Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PlatformPmic.c

Abstract:

  Platform PMIC Driver.


--*/

#include "PlatformPmic.h"
#include <Library/TimerLib.h>
#include <Library/SbPolicy.h>
#include <token.h>
#include <Guid/PlatformInfo.h>
#include <Library/PchPlatformLib.h>


EFI_EVENT                             mFSAEvent = NULL;
BOOLEAN                               FSAUSB=FALSE;

EFI_STATUS
EFIAPI
InitializeOtgVbus(void)
{
    SB_SETUP_DATA                   PchPolicyData;
    /*
      UINTN                         VarSize;
      EFI_STATUS                    Status;

      VarSize = sizeof(SYSTEM_CONFIGURATION);
      ASSERT (gRT);
      Status = gRT->GetVariable(
                                L"Setup",
                                &mSystemConfigurationGuid,
                                NULL,
                                &VarSize,
                                &mSystemConfiguration
                                );
      ASSERT_EFI_ERROR (Status);
      DEBUG ((DEBUG_ERROR, "InitializeOtgVbus(return 0x%x):%d\n", Status, mSystemConfiguration.PchUsbVbusOn));

      if(1 == mSystemConfiguration.PchUsbVbusOn) {
        PmicVbusControl(TRUE);
      }else{
        PmicVbusControl(FALSE);
      }
    */

    // Get the value of the SB Setup data.
    GetSbSetupData((VOID*)gRT, &PchPolicyData, FALSE);
    DEBUG((DEBUG_ERROR, "InitializeOtgVbus :%d\n", PchPolicyData.PchUsbVbusOn));

    if(PchPolicyData.PchUsbVbusOn)
        PmicVbusControl(TRUE);
    else
        PmicVbusControl(FALSE);

    return EFI_SUCCESS;
}


/**
  Timer handler for FSA to enable USB/UART.

  @param  Event                    Indicates the event that invoke this function.
  @param  Context                  Indicates the calling context.
**/
VOID
EFIAPI
FSATimerHandler (
  IN  EFI_EVENT                 Event,
  IN  VOID                      *Context
  )
{
  UINT8	Offset=0x2;
  UINT8	Value=0xD0;

  Offset=0x07;
  ByteReadI2C(2, 0x25, Offset, 1, &Value);
  DEBUG ((DEBUG_INFO, "==ByteReadI2C 0ffset 0x%x  value 0x%x\n", Offset, Value));   
  
  if (Value==0x16){//USB slave
    if (!FSAUSB){
      PmicVbusControl(TRUE);
      FSAUSB = TRUE;
    }
  }else if (Value==0x0){
    if (FSAUSB){//usb slave unplug    
      PmicVbusControl(FALSE);
      FSAUSB = FALSE;
     }else{//uart/audio/aca unplug
      //restore Manual SW
      Offset=0x13;
      Value=0x27;
      ByteWriteI2C(2, 0x25, Offset, 1, &Value); 
    }
  }else if (Value==0xc){//UART
    Offset=0x13;
    Value=0x6f;
    ByteWriteI2C(2, 0x25, Offset, 1, &Value); 
  }
  //de-assert interrupt
  // Offset=0x03;
  //ByteReadI2C(2, 0x25, Offset, 1, &Value);
  //DEBUG ((DEBUG_INFO, "ByteReadI2C 0ffset 0x%x  value 0x%x\n", Offset, Value));   

  return;
}


EFI_STATUS
EFIAPI
FSAInit (
VOID  )
{
  //FSA init on PR1
  EFI_STATUS                         Status;  
  UINT8	Offset=0x2;
  UINT8	Value=0xD0;

 
  Offset=0x2;
  Value=0xD0;
  ByteWriteI2C(2, 0x25, Offset, 1, &Value);
  DEBUG ((DEBUG_INFO, "ByteWriteI2C 0ffset 0x%x	value 0x%x\n", Offset, Value));   
  
  
  Value = 0xff;
  ByteReadI2C(2, 0x25, Offset, 1, &Value);
  DEBUG ((DEBUG_INFO, "ByteReadI2C 0ffset 0x%x  value 0x%x\n", Offset, Value));   
  MicroSecondDelay ( 1000 );
  Value = 0xff;
  Offset = 0x3;
  ByteReadI2C(2, 0x25, Offset, 1, &Value);
  DEBUG ((DEBUG_INFO, "ByteReadI2C 0ffset 0x%x  value 0x%x\n", Offset, Value));   
  
  Value = 0xff;
  Offset=0x07;
  ByteReadI2C(2, 0x25, Offset, 1, &Value);  
  DEBUG ((DEBUG_INFO, "==ByteReadI2C 0ffset 0x%x value 0x%x\n", Offset, Value)); 
  if (Value==0x16){//usb slave
    PmicVbusControl(TRUE);
    FSAUSB = TRUE;
  } 


  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  FSATimerHandler,
                  NULL,
                  &mFSAEvent
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = gBS->SetTimer (
                  mFSAEvent, 
                  TimerPeriodic, 
                  5000000 // 0.5s //200000=0.02s
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

EFI_STATUS
EFIAPI
InitializeVhost(void)
{
    /*
    VOID                                        *HobList;
    EFI_PLATFORM_INFO_HOB          *PlatformInfoHobPtr=NULL;

    //
    // Get Platform Info HOB
    //
    HobList = GetHobList ();
    ASSERT_EFI_ERROR (HobList == NULL);
    if ((HobList = GetNextGuidHob (&gEfiPlatformInfoGuid, HobList)) != NULL) {
    PlatformInfoHobPtr = GET_GUID_HOB_DATA (HobList);
    }
    ASSERT_EFI_ERROR ((PlatformInfoHobPtr == NULL));

    if (PlatformInfoHobPtr != NULL) {
    if (PlatformInfoHobPtr->BoardId==BOARD_ID_BL_FFRD){
      PmicVhostControl(TRUE);
    }
    }
    */
    if(TURN_ON_V5P0S) {
        DEBUG((DEBUG_ERROR, "PMICVHostControl\n"));
        PmicVhostControl(TRUE);
    }

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
InitializePmic(
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
)
/*++

  Routine Description:

  This is the standard EFI driver point for the Driver. This
    driver is responsible for setting up any platform specific policy or
    initialization information.

  Arguments:

  ImageHandle  -  Handle for the image of this driver.
  SystemTable  -  Pointer to the EFI System Table.

  Returns:

  EFI_SUCCESS  -  Policy decisions set.

--*/
{
    //UINT8 DevId, RevId;
    EFI_PLATFORM_INFO_HOB       *PlatformInfo;
    UINT32                  				Page0_Profile=0;
    PCH_STEPPING             			stepping;
    EFI_PEI_HOB_POINTERS        	Hob;
    //
    // Set the some PCI and chipset range as UC
    // And align to 1M at leaset
    //
    Hob.Raw = GetFirstGuidHob(&gEfiPlatformInfoGuid);
    ASSERT(Hob.Raw != NULL);
    PlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);

    //PmicGetDevID(&DevId, &RevId);
#if 0
    if(0x00 == DevId)   {
        DEBUG((DEBUG_ERROR, "MountVille PMIC (FPGA emul) Dev:0x%x Rev:0x%x init\n", DevId, RevId));
        //FPGA does not support Thermal/Battery charging/BCU
        //it supports limited GPIO
        PmicGpioInit();
        PmicIntrInit();
        PmicMiscInit();
    } else
#endif
    {
//    DEBUG ((DEBUG_ERROR, "Sunset Highway PMIC Dev:0x%x Rev:0x%x  init\n", DevId, RevId));
//    PmicThermInit ();
//    PmicGpioInit ();
//    PmicIntrInit ();
//    PmicBcuInit ();
//    DEBUG ((DEBUG_ERROR, "Sunset:-----------------------\n"));
        PmicMiscInit();

        if(PMIC_THERM_INIT)
            PmicThermInit();

        if(PMIC_GPIO_INIT)
            PmicGpioInit(PlatformInfo);

        PmicPage0Init(&Page0_Profile);
        InitializeOtgVbus();

//(EIP115594)>>
//    if (mPlatformInfo != NULL) {
//      if (mPlatformInfo->BoardId==BOARD_ID_BL_FFRD){
        if(PMIC_VHOST_CONTROL) {
            DEBUG((DEBUG_ERROR, "Program Vhost on FFRD\n"));
            PmicVhostControl(TRUE);
        }
//      }
//    }

//    InitializeVhost ();
//(EIP115594)<<
    }
  
  stepping =  PchStepping();
  if (stepping >= PchB0) {
    //for SoC B0 + Rohm A2 / B0
    PmicSetVIDDecayWA();
  }

  //FSA init on PR1
//  if(PlatformInfo->BoardId == BOARD_ID_BL_FFRD && PlatformInfo->BoardRev == PR1)
  if(FSA_SUPPORT == 1)
  {	
    FSAInit();
  }
    DEBUG((DEBUG_ERROR, "InitializePmic Done\n"));
    //PmicDebugRegDump();  //dump all PMIC registers
    return EFI_SUCCESS;
}

