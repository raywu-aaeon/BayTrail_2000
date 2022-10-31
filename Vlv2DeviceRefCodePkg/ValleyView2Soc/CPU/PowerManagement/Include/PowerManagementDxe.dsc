#/*++
#  This file contains an 'Intel Peripheral Driver' and uniquely
#  identified as "Intel Reference Module" and is
#  licensed for Intel CPUs and chipsets under the terms of your
#  license agreement with Intel or your vendor.  This file may
#  be modified by the user, subject to additional terms of the
#  license agreement
#--*/
#
#/*++
#
#Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved
#This software and associated documentation (if any) is furnished
#under a license and may only be used or copied in accordance
#with the terms of the license. Except as permitted by such
#license, no part of this software or documentation may be
#reproduced, stored in a retrieval system, or transmitted in any
#form or by any means without the express written consent of
#Intel Corporation.
#
#Module Name:
#
#  PowerManagementDxe.dsc
#
#Abstract:
#
#  This is the build description file containing the PowerManagement library modules.
#
#  This should be included in a [Components] section of the DSC files for a platform build.
#  Most likely in the main DXE Firmware Volume
#
#--*/


$(PROJECT_PPM_ROOT)\Smm\PowerManagement.inf SOURCE_OVERRIDE_PATH = $(EDK_SOURCE)\Foundation\Library\EdkIIGlueLib\EntryPoints
$(PROJECT_PPM_ROOT)\AcpiTables\PowerManagementAcpiTables.inf
#$(PROJECT_PPM_ROOT)\Samplecode\PpmPolicyInit\Dxe\PpmPolicyInitDxe.inf SOURCE_OVERRIDE_PATH = $(EDK_SOURCE)\Foundation\Library\EdkIIGlueLib\EntryPoints
