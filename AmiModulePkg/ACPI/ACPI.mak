#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
#**                                                                  **
#**                       All Rights Reserved.                       **
#**                                                                  **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
#**                                                                  **
#**                       Phone: (770)-246-8600                      **
#**                                                                  **
#**********************************************************************
#**********************************************************************
#**********************************************************************
# $Header: /Alaska/Tools/template.mak 6     1/13/10 2:13p Felixp $
#
# $Revision: 6 $
#
# $Date: 1/13/10 2:13p $
#**********************************************************************
#<AMI_FHDR_START>
#
# Name:	ACPI.mak
#
# Description:	
#
#<AMI_FHDR_END>
#**********************************************************************
Prepare : $(BUILD_DIR)/AcpiOemElinks.h

ifeq (\"$(T_ACPI_OEM_ID:|=a)\", \"$(T_ACPI_OEM_ID)\")
TMP_OEM_ID := \"$(T_ACPI_OEM_ID)\"
else
TMP_OEM_ID := $(T_ACPI_OEM_ID:|=\")
endif
ifeq (\"$(T_ACPI_OEM_TBL_ID:|=a)\", \"$(T_ACPI_OEM_TBL_ID)\")
TMP_OEM_TBL_ID := \"$(T_ACPI_OEM_TBL_ID)\"
else
TMP_OEM_TBL_ID := $(T_ACPI_OEM_TBL_ID:|=\")
endif
$(BUILD_DIR)/AcpiOemElinks.h :  $(BUILD_DIR)/token.mak
	$(ECHO) \
" #define ACPI_OEM_ID_MAK $(TMP_OEM_ID)$(EOL)\
#define ACPI_OEM_TBL_ID_MAK $(TMP_OEM_TBL_ID)$(EOL)\
#define OEM_LIST $(OemUpdateHeader)$(EOL)" \
> $(BUILD_DIR)/AcpiOemElinks.h

#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
#**                                                                  **
#**                       All Rights Reserved.                       **
#**                                                                  **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
#**                                                                  **
#**                       Phone: (770)-246-8600                      **
#**                                                                  **
#**********************************************************************
#**********************************************************************