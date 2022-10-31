#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
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
# $Header:  6      $
#
# $Revision:  $
#
# $Date:  $
#**********************************************************************
#<AMI_FHDR_START>
#
# Name: ReportFV.mak
#
# Description:  
#
#<AMI_FHDR_END>
#**********************************************************************
Prepare : $(BUILD_DIR)/ReportFVeLinks.h

$(BUILD_DIR)/ReportFVeLinks.h : $(BUILD_DIR)/Token.h AmiModulePkg/ReportFV/ReportFV.mak
	$(ECHO) \
"#define ProcessFvBeforePublishing $(ProcessFvBeforePublishing)\
$(EOL)#define ProcessDxeFvInDxeIpl $(ProcessDxeFvInDxeIpl)\
$(EOL)#define ProcessNestedFvBeforePublishing $(ProcessNestedFvBeforePublishing)\
$(EOL)"\
>$(BUILD_DIR)/ReportFVeLinks.h

 
#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
#**                                                                  **
#**                       All Rights Reserved.                       **
#**                                                                  **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
#**                                                                  **
#**                       Phone: (770)-246-8600                      **
#**                                                                  **
#**********************************************************************
#**********************************************************************