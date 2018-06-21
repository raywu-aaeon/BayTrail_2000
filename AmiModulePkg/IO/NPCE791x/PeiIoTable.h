//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
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
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
// Revision History
// ----------------
// $Log: $
//
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:  <This File's Name>
//
// Description:
//
//<AMI_FHDR_END>
//**********************************************************************
#if (NPCE791x_SUPPORT)
#ifdef SIO_PEI_DECODE_TABLE
    // -----------------------------
    //|  BaseAdd | UID  | Type |
    // -----------------------------
    {NPCE791x_CONFIG_INDEX, 2, 0xFF},

	#if (defined(Recovery_SUPPORT) && (SERIAL_RECOVERY_SUPPORT))
	#if (NPCE791x_SERIAL_PORT1_PRESENT)
		{NPCE791x_SERIAL_PORT1_BASE_ADDRESS, 0x01, dsUART}, // COMA decode
	#endif
	#endif

	#if defined(Recovery_SUPPORT) && (Recovery_SUPPORT)
	#if (NPCE791x_KEYBOARD_PRESENT)
		{0x60, 0, dsPS2K},                                  // KBC decode
	#endif
	#endif

	#if defined(NPCE791x_TOTAL_BASE_ADDRESS) && (NPCE791x_TOTAL_BASE_ADDRESS != 0)
		{NPCE791x_TOTAL_BASE_ADDRESS, NPCE791x_TOTAL_LENGTH, 0xFF}, // open a IODecode section for GPIO+PME+...
	#endif

#endif

#ifdef SIO_PEI_INIT_TABLE
    // -----------------------------
    //|  Addr | DataMask  | DataValue |
    // -----------------------------

    //---------------------------------------------------------------------
    // Enter Configuration Mode.
    //---------------------------------------------------------------------

	//--------------------------------------------------------------------------
	// Before init all logical devices, program Global register if needed.
	//--------------------------------------------------------------------------
    {NPCE791x_CONFIG_INDEX, 0x00, 0x21},  //SuperI/O Configuration 1 Register (SIOCF1) Default: 11h
    {NPCE791x_CONFIG_DATA,  0xFE, 0x01},  //BIT0: GLOBEN (Global Device Enable)
    {NPCE791x_CONFIG_INDEX, 0x00, 0x25},  //SuperI/O Configuration 5 Register (SIOCF5) Default: 00h
    {NPCE791x_CONFIG_DATA,  0xEF, 0x00},  //BIT4: SMI2IRQ_EN (SMI to IRQ2 Enable)
    {NPCE791x_CONFIG_INDEX, 0x00, 0x26},  //SuperI/O Configuration 6 Register (SIOCF6) Default: 00h
    {NPCE791x_CONFIG_DATA,  0xF7, 0x00},  //BIT3: CIRPDIS (Serial Port 1 with CIR Port Disable)
    {NPCE791x_CONFIG_INDEX, 0x00, 0x28},  //SuperI/O General-Purpose Scratch Register (SIOGPS) Default: 00h
    {NPCE791x_CONFIG_DATA,  0x00, 0x00},  //BIT[7:0]: SGP_SCR (SuperI/O General-Purpose Scratch)
    {NPCE791x_CONFIG_INDEX, 0x00, 0x29},  //SuperI/O Configuration 9 Register (SIOCF9) Default: 000X0100
    {NPCE791x_CONFIG_DATA,  0x7B, 0x04},  //BIT7: CCR_LOCK (Clock Control Register LOCK), BIT2: CKVALID (Clock Enable)
    {NPCE791x_CONFIG_INDEX, 0x00, 0x2D},  //SuperI/O Configuration D Register (SIOCFD) Default: 00h
    {NPCE791x_CONFIG_DATA,  0x7C, 0x00},  //BIT7: JENK_HSL (JTAG over KBSOUT, Select by Host), BIT1; PWSO (Power Supply Off), BIT0: PWBM(Power Button Mode)

	#if (defined(Recovery_SUPPORT) && (SERIAL_RECOVERY_SUPPORT))
	#if (NPCE791x_SERIAL_PORT1_PRESENT)
	// Select device
	{NPCE791x_CONFIG_INDEX, 0x00, NPCE791x_LDN_SEL_REGISTER},
	{NPCE791x_CONFIG_DATA,  0x00, NPCE791x_LDN_UART1},
	// Program Base Addr
	{NPCE791x_CONFIG_INDEX, 0x00, NPCE791x_BASE1_LO_REGISTER},
	{NPCE791x_CONFIG_DATA,  0x00, (UINT8)(NPCE791x_SERIAL_PORT1_BASE_ADDRESS & 0xFF)},
	{NPCE791x_CONFIG_INDEX, 0x00, NPCE791x_BASE1_HI_REGISTER},
	{NPCE791x_CONFIG_DATA,  0x00, (UINT8)(NPCE791x_SERIAL_PORT1_BASE_ADDRESS >> 8)},
	// Activate Device
	{NPCE791x_CONFIG_INDEX, 0x00, NPCE791x_ACTIVATE_REGISTER},
	{NPCE791x_CONFIG_DATA,  0x00, NPCE791x_ACTIVATE_VALUE},
	#endif
	#endif
    //---------------------------------------------------------------------
    // Program and initialize some logical device if needed (Disable).
    //---------------------------------------------------------------------

    //---------------------------------------------------------------------
    // Initialize the Serial Port for debug useage. Default is COMA
    //---------------------------------------------------------------------

    //---------------------------------------------------------------------
    // Initialize the KeyBoard and floppy controller for Recovery
    //---------------------------------------------------------------------
    #if defined(Recovery_SUPPORT) && (Recovery_SUPPORT)
    #if (NPCE791x_KEYBOARD_PRESENT)
    // Seclect device KEYBOARD
    {NPCE791x_CONFIG_INDEX, 0x00, NPCE791x_LDN_SEL_REGISTER},
    {NPCE791x_CONFIG_DATA,  0x00, NPCE791x_LDN_PS2K},
    // Program Base Addr
    {NPCE791x_CONFIG_INDEX, 0x00, NPCE791x_BASE1_HI_REGISTER},
    {NPCE791x_CONFIG_DATA,  0x00, 0x00},
    {NPCE791x_CONFIG_INDEX, 0x00, NPCE791x_BASE1_LO_REGISTER},
    {NPCE791x_CONFIG_DATA,  0x00, 0x60},
    {NPCE791x_CONFIG_INDEX, 0x00, NPCE791x_BASE2_HI_REGISTER},
    {NPCE791x_CONFIG_DATA,  0x00, 0x00},
    {NPCE791x_CONFIG_INDEX, 0x00, NPCE791x_BASE2_LO_REGISTER},
    {NPCE791x_CONFIG_DATA,  0x00, 0x64},
    // Program Interrupt
    {NPCE791x_CONFIG_INDEX, 0x00, NPCE791x_IRQ1_REGISTER},
    {NPCE791x_CONFIG_DATA,  0x00, 0x01},
    // Activate Device
    {NPCE791x_CONFIG_INDEX, 0x00, NPCE791x_ACTIVATE_REGISTER},
    {NPCE791x_CONFIG_DATA,  0x00, NPCE791x_ACTIVATE_VALUE},
    #endif //NPCE791x_KEYBOARD_PRESENT
    #endif //#if defined(Recovery_SUPPORT) && (Recovery_SUPPORT == 1)

    //---------------------------------------------------------------------
    // After init all logical devices, program Global register if needed.
    //---------------------------------------------------------------------

    //---------------------------------------------------------------------
    // After init all logical devices,  Exit Configuration Mode.
    //---------------------------------------------------------------------
#endif

#ifdef COM_PORT_DBG_DECODE
    // -----------------------------
    //|  BaseAdd | UID  | Type |
    // -----------------------------
    //Below decode is for DEBUG Mode
    {NPCE791x_CONFIG_INDEX, 2, 0xFF},
    //Below decode is for DEBUG Mode
    #ifdef EFI_DEBUG
    #if (NPCE791x_SERIAL_PORT1_PRESENT)
    {NPCE791x_SERIAL_PORT1_BASE_ADDRESS, 0x01, dsUART}, // COMA decode
    #endif
    #endif
#endif

#ifdef COM_PORT_DBG_INIT
    // -----------------------------
    //|  Addr | DataMask  | DataValue |
    // -----------------------------
    //---------------------------------------------------------------------
    // Enter Configuration Mode.
    //---------------------------------------------------------------------

	#ifdef EFI_DEBUG
	#if (NPCE791x_SERIAL_PORT1_PRESENT)
	// Select device
	{NPCE791x_CONFIG_INDEX, 0x00, NPCE791x_LDN_SEL_REGISTER},
	{NPCE791x_CONFIG_DATA,  0x00, NPCE791x_LDN_UART1},
	// Program Base Addr
	{NPCE791x_CONFIG_INDEX, 0x00, NPCE791x_BASE1_LO_REGISTER},
	{NPCE791x_CONFIG_DATA,  0x00, (UINT8)(NPCE791x_SERIAL_PORT1_BASE_ADDRESS & 0xFF)},
	{NPCE791x_CONFIG_INDEX, 0x00, NPCE791x_BASE1_HI_REGISTER},
	{NPCE791x_CONFIG_DATA,  0x00, (UINT8)(NPCE791x_SERIAL_PORT1_BASE_ADDRESS >> 8)},
	// Activate Device
	{NPCE791x_CONFIG_INDEX, 0x00, NPCE791x_ACTIVATE_REGISTER},
	{NPCE791x_CONFIG_DATA,  0x00, NPCE791x_ACTIVATE_VALUE},
	#endif // NPCE791x_SERIAL_PORT1_PRESENT
	#endif // #ifdef EFI_DEBUG
    //---------------------------------------------------------------------
    // After init all logical devices,  Exit Configuration Mode.
    //---------------------------------------------------------------------
#endif
#endif //NPCE791x_SUPPORT

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************



