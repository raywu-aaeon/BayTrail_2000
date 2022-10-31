#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2008, American Megatrends, Inc.         **
#**                                                                  **
#**                       All Rights Reserved.                       **
#**                                                                  **
#**             5555 Oakbrook Pkwy, Norcross, GA 30093               **
#**                                                                  **
#**                       Phone: (770)-246-8600                      **
#**                                                                  **
#**********************************************************************
#**********************************************************************

#**********************************************************************
# $Header: /Alaska/SOURCE/Modules/USB/ALASKA/rt/usbrt.mak 23    8/29/12 8:41a Ryanchou $
#
# $Revision: 23 $
#
# $Date: 8/29/12 8:41a $
#
#****************************************************************************

#**********************************************************************
#<AMI_FHDR_START>
#
# Name:		UsbRt.mak
#
# Description:	Make file for the UsbRt component
#
#<AMI_FHDR_END>
#**********************************************************************

USB_RT_SOURCES := \
  ../$(USBRT_DIR)/AmiUsb.c$(EOL)\
  ../$(USBRT_DIR)/UsbKbd.c$(EOL)\
  ../$(USBRT_DIR)/Debug.c$(EOL)\
  ../$(USBRT_DIR)/Elib.c$(EOL)\
  ../$(USBRT_DIR)/Uhci.c$(EOL)\
  ../$(USBRT_DIR)/Usb.c$(EOL)\
  ../$(USBRT_DIR)/UsbHub.c$(EOL)\
  ../$(USBRT_DIR)/UsbMass.c$(EOL)\
  ../$(USBRT_DIR)/UsbMass.h$(EOL)\
  ../$(USBRT_DIR)/UsbCcid.c$(EOL)\
  ../$(USBRT_DIR)/UsbMs.c$(EOL)\
  ../$(USBRT_DIR)/UsbHid.c$(EOL)\
  ../$(USBRT_DIR)/UsbPoint.c$(EOL)\
  ../$(USBRT_DIR)/Uhci.h$(EOL)\
  ../$(USBRT_DIR)/UsbKbd.h$(EOL)\
  ../$(USBRT_DIR)/Ehci.c$(EOL)\
  ../$(USBRT_DIR)/Ehci.h$(EOL)\
  ../$(USBRT_DIR)/Ohci.c$(EOL)\
  ../$(USBRT_DIR)/Ohci.h$(EOL)\
  ../$(USBRT_DIR)/SysKbc.c$(EOL)\
  ../$(USBRT_DIR)/SysNoKbc.c$(EOL)\
  ../$(USBRT_DIR)/Xhci.h$(EOL)\
  ../$(USBRT_DIR)/Xhci.c$(EOL)\
  ../$(UHCD_DIR)/UsbPort.c$(EOL)
  
ifneq ($(USB_RT_SOURCES_LIST),"")
USB_RT_SOURCES += $(patsubst %,../%,$(subst $(SPACE),$(EOL)$(SPACE),$(USB_RT_SOURCES_LIST)))$(EOL)
endif
  
USB_RT_PACKAGES := \
  MdePkg/MdePkg.dec$(EOL)\
  IntelFrameworkPkg/IntelFrameworkPkg.dec$(EOL)\
  AmiCompatibilityPkg/AmiCompatibilityPkg.dec$(EOL)\
  AmiModulePkg/AmiModulePkg.dec$(EOL)\
  AmiChipsetPkg/AmiChipsetPkg.dec$(EOL) #EIP148801
  
USB_RT_PROTOCOLS := \
  gEfiUsbProtocolGuid$(EOL)

ifeq ($(USB_RUNTIME_DRIVER_IN_SMM),1)
USB_RT_MODULE_TYPE = DXE_SMM_DRIVER
USB_RT_SOURCES += ../$(UHCD_DIR)/UsbSb.c$(EOL)
#EIP148801 >>
USB_RT_PROTOCOLS += \
  gAmiUsbSmmProtocolGuid$(EOL)\
  gEfiSmmUsbDispatch2ProtocolGuid$(EOL)\
  gEfiSmmSwDispatch2ProtocolGuid$(EOL)\
  gEfiSmmPeriodicTimerDispatch2ProtocolGuid$(EOL)\
  gEfiSmmGpiDispatch2ProtocolGuid$(EOL)\
  gEfiSmmPowerButtonDispatchProtocolGuid$(EOL)\
  gEfiSmmSxDispatchProtocolGuid$(EOL)\
  gEmul6064MsInputProtocolGuid$(EOL)\
  gEmul6064TrapProtocolGuid$(EOL)\
  gEmul6064KbdInputProtocolGuid$(EOL)
#EIP148801 <<
USB_RT_DEPEX := \
  gEfiUsbProtocolGuid AND$(EOL)\
  gEfiSmmBase2ProtocolGuid AND$(EOL)\
  gEfiSmmSwDispatch2ProtocolGuid$(EOL)
else
USB_RT_MODULE_TYPE = DXE_DRIVER
USB_RT_DEPEX := \
	gEfiUsbProtocolGuid $(EOL)
endif

ifeq ($(USB_ACPI_ENABLE_WORKAROUND),1)
USB_RT_PROTOCOLS += \
  gEfiAcpiEnDispatchProtocolGuid$(EOL)
USB_RT_PACKAGES += \
  AmiChipsetPkg/AmiChipsetPkg.dec$(EOL)
endif

Prepare : $(BUILD_DIR)/UsbDevDriverElinks.h $(BUILD_DIR)/UsbRt.inf

$(BUILD_DIR)/UsbDevDriverElinks.h :
	$(ECHO) \
"// Don't delete this line$(EOL)\
#define USB_DEV_EFI_DRIVER $(USB_DEV_EFI_DRIVER_LIST)$(EOL)\
#define USB_DEV_DELAYED_DRIVER $(USB_DEV_DELAYED_DRIVER_LIST)$(EOL)\
#define USB_DEV_DRIVER $(USB_DEV_DRIVER_LIST)$(EOL)\
#define KBD_BUFFER_CHECK_ELINK_LIST $(CheckKeyBoardBufferForSpecialChars)$(EOL)\
// Don't delete this line$(EOL)"\
> $(BUILD_DIR)/UsbDevDriverElinks.h

$(BUILD_DIR)/UsbRt.inf : $(BUILD_DIR)/Token.h $(USBRT_DIR)/UsbRt.mak
	$(ECHO) \
"[Defines]$(EOL)\
  INF_VERSION                    = 0x00010005$(EOL)\
  BASE_NAME                      = UsbRt$(EOL)\
  FILE_GUID                      = 04EAAAA1-29A1-11d7-8838-00500473D4EB$(EOL)\
  MODULE_TYPE                    = $(USB_RT_MODULE_TYPE)$(EOL)\
  VERSION_STRING                 = 1.0$(EOL)\
  PI_SPECIFICATION_VERSION       = 0x00010014$(EOL)\
  ENTRY_POINT                    = USBDriverEntryPoint$(EOL)\
$(EOL)\
[Sources]$(EOL)\
	$(USB_RT_SOURCES)\
$(EOL)\
[Packages]$(EOL)\
	$(USB_RT_PACKAGES)\
$(EOL)\
[LibraryClasses]$(EOL)\
  AmiDxeLib$(EOL)\
  UefiDriverEntryPoint$(EOL)\
$(EOL)\
[Protocols]$(EOL)\
  $(USB_RT_PROTOCOLS)\
$(EOL)\
[Depex]$(EOL)\
  $(USB_RT_DEPEX)\
$(EOL)\
[BuildOptions]$(EOL)\
  MSFT:*_*_*_CC_FLAGS = /D USB_RT_DRIVER$(EOL)\
  *_ARMGCC_ARM_CC_FLAGS= -DUSB_RT_DRIVER$(EOL)\
  *_ARMLINUXGCC_ARM_CC_FLAGS= -DUSB_RT_DRIVER$(EOL)"\
> $(BUILD_DIR)/UsbRt.inf

#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2008, American Megatrends, Inc.         **
#**                                                                  **
#**                       All Rights Reserved.                       **
#**                                                                  **
#**             5555 Oakbrook Pkwy, Norcross, GA 30093               **
#**                                                                  **
#**                       Phone: (770)-246-8600                      **
#**                                                                  **
#**********************************************************************
#**********************************************************************
