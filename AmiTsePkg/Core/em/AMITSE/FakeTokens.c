//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
//**                                                             **//
//**        (C)Copyright 1985-2011, American Megatrends, Inc.    **//
//**                                                             **//
//**                     All Rights Reserved.                    **//
//**                                                             **//
//**   5555 Oakbrook Pkwy, Building 200,Norcross, Georgia 30093  **//
//**                                                             **//
//**                     Phone (770)-246-8600                    **//
//**                                                             **//
//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
// $Archive: /Alaska/BIN/Modules/AMITSE2_0/AMITSE/FakeTokens.c $
//
// $Author: Rajashakerg $
//
// $Revision: 18 $
//
// $Date: 9/17/12 5:59a $
//
//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
//**********************************************************************
/** @file FakeTokens.c
    This file does not add any code. It has, all the string tokens that
    are used by Minisetup, to fake the usage to strgather.exe. Add to
    this list, the new string tokens that are defined in
    AmiTSEStrstr.uni and to be used by Minisetup

**/
//**********************************************************************


//Following code is to force strgatherer to include these strings
#if 0
STRING_TOKEN(STR_MAIN_TITLE)
STRING_TOKEN(STR_MAIN_COPYRIGHT)
STRING_TOKEN(STR_CTRL_CHKBOX_ENABLE)
STRING_TOKEN(STR_CTRL_CHKBOX_DISABLE)
STRING_TOKEN(STR_HELP_TITLE)

STRING_TOKEN(STR_FDD)
STRING_TOKEN(STR_HDD)
STRING_TOKEN(STR_CDROM)
STRING_TOKEN(STR_NETWORK)
STRING_TOKEN(STR_BEV)
STRING_TOKEN(STR_PRI_MAS)
STRING_TOKEN(STR_PRI_SLA)
STRING_TOKEN(STR_SEC_MAS)
STRING_TOKEN(STR_SEC_SLA)
STRING_TOKEN(STR_SATA_X)
STRING_TOKEN(STR_NO_BOOT_OPTIONS)
STRING_TOKEN(STR_PASSWORD_PROMPT)
STRING_TOKEN(STR_ERROR_PSWD)
STRING_TOKEN(STR_DRV_HLTH_REBOOT_POST)
STRING_TOKEN(STR_EVAL_MSG)
STRING_TOKEN(STR_ACK_BBS_POPUP)
STRING_TOKEN(STR_DEL_ENTER_SETUP)
STRING_TOKEN(STR_ACK_ENTER_SETUP)
STRING_TOKEN(STR_GENERAL_HELP)
STRING_TOKEN(STR_GENERAL_HELP_MSG)
STRING_TOKEN(STR_BOOT_MANAGER)
STRING_TOKEN(STR_BOOT_MANAGER_HELP)
STRING_TOKEN(STR_LANGUAGE)
STRING_TOKEN(STR_LANGUAGE_HELP)
STRING_TOKEN(STR_LOAD_PREVIOUS)
STRING_TOKEN(STR_LOAD_PREVIOUS_MSG)
STRING_TOKEN(STR_LOAD_FAILSAFE)
STRING_TOKEN(STR_LOAD_FAILSAFE_HELP)
STRING_TOKEN(STR_LOAD_FAILSAFE_MSG)
STRING_TOKEN(STR_LOAD_OPTIMAL)
STRING_TOKEN(STR_LOAD_OPTIMAL_HELP)
STRING_TOKEN(STR_LOAD_OPTIMAL_MSG)
STRING_TOKEN(STR_SAVE_EXIT)
STRING_TOKEN(STR_SAVE_EXIT_HELP)
STRING_TOKEN(STR_SAVE_EXIT_MSG)
STRING_TOKEN(STR_SAVE_RESET)
STRING_TOKEN(STR_SAVE_RESET_HELP)
STRING_TOKEN(STR_SAVE_RESET_MSG)
STRING_TOKEN(STR_SAVE_VALUES)
STRING_TOKEN(STR_SAVE_VALUES_MSG)
STRING_TOKEN(STR_EXIT)
STRING_TOKEN(STR_EXIT_MSG)
STRING_TOKEN(STR_RESET)
STRING_TOKEN(STR_RESET_MSG)
STRING_TOKEN(STR_SAVE_USER_DEFAULTS)
STRING_TOKEN(STR_LOAD_USER_DEFAULTS)
STRING_TOKEN(STR_LOAD_USER_MSG)
STRING_TOKEN(STR_SUBMENU_OPTION)
STRING_TOKEN(STR_CTRL_OK)
STRING_TOKEN(STR_CTRL_CANCEL)
STRING_TOKEN(STR_CTRL_YES)
STRING_TOKEN(STR_CTRL_NO)
STRING_TOKEN(STR_DAY_SUNDAY)
STRING_TOKEN(STR_DAY_MONDAY)
STRING_TOKEN(STR_DAY_TUESDAY)
STRING_TOKEN(STR_DAY_WEDNESDAY)
STRING_TOKEN(STR_DAY_THURSDAY)
STRING_TOKEN(STR_DAY_FRIDAY)
STRING_TOKEN(STR_DAY_SATURDAY)
STRING_TOKEN(STR_OLD_PSWD)
STRING_TOKEN(STR_NEW_PSWD)
STRING_TOKEN(STR_CONFIRM_NEW_PSWD)
STRING_TOKEN(STR_PSWD_CLR)
STRING_TOKEN(STR_ERROR)
STRING_TOKEN(STR_WARNING)
STRING_TOKEN(STR_WARNING_NOT_FOUND)
STRING_TOKEN(STR_ERROR_INPUT)
STRING_TOKEN(STR_EMPTY_STRING)
STRING_TOKEN(STR_INCONSISTENT_MSG_TITLE)
STRING_TOKEN(STR_NOSUBMITIF_MSG_TITLE)
STRING_TOKEN(STR_BBS_POPUP_TITLE_STRING)
STRING_TOKEN(STR_BBS_POPUP_HELP1_STRING)
STRING_TOKEN(STR_BBS_POPUP_HELP2_STRING)
STRING_TOKEN(STR_BBS_POPUP_HELP3_STRING)
STRING_TOKEN(STR_BBS_POPUP_HELP_STRING)
STRING_TOKEN(STR_POPUPMENU_ENTER_SETUP)
STRING_TOKEN(STR_USB)
STRING_TOKEN(STR_VENDOR)
STRING_TOKEN(STR_FILEPATH)
STRING_TOKEN(STR_PROTOCOL)
STRING_TOKEN(STR_FV_FILEPATH)
STRING_TOKEN(STR_USB_DP)
STRING_TOKEN(STR_SCSI_DP)
STRING_TOKEN(STR_FIBRE_CHANNEL_DP)
STRING_TOKEN(STR_1394_DP)
STRING_TOKEN(STR_I20_DP)
STRING_TOKEN(STR_INFINIBAND_DP)
STRING_TOKEN(STR_VENDOR_DP)
STRING_TOKEN(STR_MAC_DP)
STRING_TOKEN(STR_NETWORK_IPV4_DP)
STRING_TOKEN(STR_NETWORK_IPV6_DP)
STRING_TOKEN(STR_UART_DP)
STRING_TOKEN(STR_USB_CLASS_DP)
STRING_TOKEN(STR_USB_CLASS_PHY_DP)
STRING_TOKEN(STR_USB_CLASS_MASS_DP)
STRING_TOKEN(STR_USB_LOGICAL_DP)
STRING_TOKEN(STR_USB_SATA_DP)
STRING_TOKEN(STR_USB_ISCSI_DP)
STRING_TOKEN(STR_HARD_DISK_ORDER)
STRING_TOKEN(STR_CDROM_ORDER)
STRING_TOKEN(STR_FLOPPY_DISK_ORDER)
STRING_TOKEN(STR_NETWORK_ORDER)
STRING_TOKEN(STR_BEV_ORDER)
STRING_TOKEN(STR_USB_ORDER)
STRING_TOKEN(STR_PCMCIA_ORDER)
STRING_TOKEN(STR_UNKNOWN_ORDER)
STRING_TOKEN(STR_ADD_BOOT_OPTION_RESERVED)
STRING_TOKEN(STR_ADD_BOOT_OPTION_EMPTY)
STRING_TOKEN(STR_ADD_DRIVER_OPTION_EMPTY)
STRING_TOKEN(STR_DEL_BOOT_OPTION_RESERVED)
STRING_TOKEN(STR_MSGBOX_PROGRESS_TITLE)
STRING_TOKEN(STR_MSGBOX_PROGRESS_TEXT)
STRING_TOKEN(STR_FILE_SYSTEM)
STRING_TOKEN(STR_FILE_PATH)
STRING_TOKEN(STR_DRIVER_PATH)
STRING_TOKEN(STR_DRV_HLTH_TITLE)
STRING_TOKEN(STR_DRV_HLTH_RECON)
STRING_TOKEN(STR_DRV_HLTH_REBOOT)
STRING_TOKEN(STR_SECBOOT_VIOLATION)
STRING_TOKEN(STR_SECBOOT_INV_SIGN)
STRING_TOKEN(STR_DBG_PRINT_CHECKED)
STRING_TOKEN(STR_DBG_PRINT_HIIPARSING)
STRING_TOKEN(STR_DBG_PRINT_HIIFUNCTION)
STRING_TOKEN(STR_DBG_PRINT_HIICALLBACK) 
STRING_TOKEN(STR_DBG_PRINT_HIINOTIFICATION)
STRING_TOKEN(STR_DBG_PRINT_IFRFORM)
STRING_TOKEN(STR_DBG_PRINT_VARIABLE_CACHE)
STRING_TOKEN(STR_OVERRIDE_BOOTNEXT_OPTION_1)						
STRING_TOKEN(STR_OVERRIDE_BOOTNEXT_OPTION_2)						
STRING_TOKEN(STR_OVERRIDE_BOOTNEXT_OPTION_3)
STRING_TOKEN(STR_OVERRIDE_OPTION_MENU_TITLE)
STRING_TOKEN(STR_OVERRIDE_OPTION_MENU_LEGEND)
STRING_TOKEN(STR_OLD_PSWD_LABEL)
STRING_TOKEN(STR_NEW_PSWD_LABEL)
STRING_TOKEN(STR_CONFIRM_NEW_PSWD_LABEL)
STRING_TOKEN(STR_PSWD_CLR_LABEL)
STRING_TOKEN(STR_FILE_SYSTEM_TO_SAVE_IMG)
STRING_TOKEN(STR_TSE_CACHE_CHANGE_WARNING)
STRING_TOKEN(STR_CANT_CHANGE_PASSWORD)
STRING_TOKEN(STR_PARSING_ERROR)
STRING_TOKEN(STR_NON_DEVICE_GROUP)
STRING_TOKEN(STR_DISK_DEVICE_GROUP)
STRING_TOKEN(STR_VIDEO_DEVICE_GROUP)
STRING_TOKEN(STR_NETWORK_DEVICE_GROUP)
STRING_TOKEN(STR_INPUT_DEVICE_GROUP)
STRING_TOKEN(STR_ONBOARD_DEVICE_GROUP)
STRING_TOKEN(STR_OTHER_DEVICE_GROUP)
STRING_TOKEN(STR_DEVICE_GROUP_HELP)
#endif


//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
//**                                                             **//
//**        (C)Copyright 1985-2013, American Megatrends, Inc.    **//
//**                                                             **//
//**                     All Rights Reserved.                    **//
//**                                                             **//
//**           5555 Oakbrook Pkwy, Norcross, Georgia 30093       **//
//**                                                             **//
//**                     Phone (770)-246-8600                    **//
//**                                                             **//
//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
