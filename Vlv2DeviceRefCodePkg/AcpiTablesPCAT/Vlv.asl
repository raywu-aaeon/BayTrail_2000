/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  VLV.ASL

Abstract:

  Valleyview PCI configuration space definition.

--*/
#ifndef AMI_ACPI_SUPPORT //AMI_OVERRIDE
Scope (\_SB.PCI0)
{

  Device(GFX0)   // Mobile I.G.D
  {
    Name(_ADR, 0x00020000)
#endif //AMI_OVERRIDE
#ifdef WIN8_SUPPORT 
    Method(GDEP, 0)
    {
      If(LEqual(OSYS,2013))
      {
        Name(_DEP, Package(0x1)
        {
          PEPD
        })
      }
    }
#endif
    include("INTELGFX.ASL")
    include("INTELISPDev2.ASL")
#ifndef AMI_ACPI_SUPPORT //AMI_OVERRIDE
  } // end "Mobile I.G.D"
}//end scope
#endif //AMI_OVERRIDE
