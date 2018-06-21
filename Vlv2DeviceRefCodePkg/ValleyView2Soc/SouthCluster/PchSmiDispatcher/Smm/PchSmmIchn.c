/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

  @file
  PchSmmIchn.c

  @brief
  File to contain all the hardware specific stuff for the Smm Ichn dispatch protocol.

**/
#include "PchSmmHelpers.h"
#include "PlatformBaseAddresses.h"

PCH_SMM_SOURCE_DESC ICHN_SOURCE_DESCS[NUM_ICHN_TYPES] = {
  ///
  /// IchnMch
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnPme
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnRtcAlarm
  ///
  {
    PCH_SMM_SCI_EN_DEPENDENT,
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_PCH_ACPI_PM1_EN
        },
        S_PCH_ACPI_PM1_EN,
        N_PCH_ACPI_PM1_EN_RTC
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_PCH_ACPI_PM1_STS
        },
        S_PCH_ACPI_PM1_STS,
        N_PCH_ACPI_PM1_STS_RTC
      }
    }
  },
  ///
  /// IchnRingIndicate
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnAc97Wake
  /// VLV removed IchnAc97Wake,
  /// we just fill in invalid initializer and not use it.
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnSerialIrq
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_PCH_SMI_STS
        },
        S_PCH_SMI_STS,
        N_PCH_SMI_STS_ILB
      }
    }
  },
  ///
  /// IchnY2KRollover
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnTcoTimeout
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_PCH_SMI_EN
        },
        S_PCH_SMI_EN,
        N_PCH_SMI_EN_TCO
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_PCH_SMI_STS
        },
        S_PCH_SMI_STS,
        N_PCH_SMI_STS_TCO
      }
    }
  },
  ///
  /// IchnOsTco
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnNmi
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_PCH_SMI_EN
        },
        S_PCH_SMI_EN,
        N_PCH_SMI_EN_TCO
      },
      {
        {
          MEMORY_MAPPED_IO_ADDRESS_TYPE,
          (ILB_BASE_ADDRESS + R_PCH_ILB_GNMI)
        },
        S_PCH_ILB_GNMI,
        N_PCH_ILB_GNMI_NMI2SMIEN
      }
    },
    {
      {
        {
          MEMORY_MAPPED_IO_ADDRESS_TYPE,
          (ILB_BASE_ADDRESS + R_PCH_ILB_GNMI)
        },
        S_PCH_ILB_GNMI,
        N_PCH_ILB_GNMI_NMI2SMIST
      }
    }
  },
  ///
  /// IchnIntruderDetect
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnBiosWp
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_PCH_SMI_EN
        },
        S_PCH_SMI_EN,
        N_PCH_SMI_EN_TCO
      },
      {
        {
          MEMORY_MAPPED_IO_ADDRESS_TYPE,
          (SPI_BASE_ADDRESS + R_PCH_SPI_SCS)
        },
        S_PCH_SPI_SCS,
        N_PCH_SPI_SCS_SMIWPEN
      }
    },
    {
      {
        {
          MEMORY_MAPPED_IO_ADDRESS_TYPE,
          (SPI_BASE_ADDRESS + R_PCH_SPI_SCS)
        },
        S_PCH_SPI_SCS,
        N_PCH_SPI_SCS_SMIWPST
      }
    }
  },
  ///
  /// IchnMcSmi
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnPmeB0
  ///
  {
    PCH_SMM_SCI_EN_DEPENDENT,
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_PCH_ACPI_GPE0a_EN
        },
        S_PCH_ACPI_GPE0a_EN,
        N_PCH_ACPI_GPE0a_EN_PME_B0
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_PCH_ACPI_GPE0a_STS
        },
        S_PCH_ACPI_GPE0a_STS,
        N_PCH_ACPI_GPE0a_STS_PME_B0
      }
    }
  },
  ///
  /// IchnThrmSts
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnSmBus
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_PCH_SMI_STS
        },
        S_PCH_SMI_STS,
        N_PCH_SMI_STS_SMBUS
      }
    }
  },
  ///
  /// IchnIntelUsb2
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_PCH_SMI_EN
        },
        S_PCH_SMI_EN,
        N_PCH_SMI_EN_INTEL_USB2
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_PCH_SMI_STS
        },
        S_PCH_SMI_STS,
        N_PCH_SMI_STS_INTEL_USB2
      }
    }
  },
  ///
  /// IchnMonSmi7
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnMonSmi6
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnMonSmi5
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnMonSmi4
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap13
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap12, KBC_ACT_STS
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap11
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap10
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap9, PIRQDH_ACT_STS
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap8, PIRQCG_ACT_STS
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap7, PIRQBF_ACT_STS
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap6, PIRQAE_ACT_STS
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap5
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap3
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap2
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap1
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap0, IDE_ACT_STS
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// PCH I/O Trap register 3 monitor,
  /// The "PCH_RCRB_BASE_NEED_FIX" should be fixed since the RCRB base should get from the RCBA register filled by platform module.
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// PCH I/O Trap register 2 monitor
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// PCH I/O Trap register 1 monitor
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// PCH I/O Trap register 0 monitor
  ///
  NULL_SOURCE_DESC_INITIALIZER,
};

PCH_SMM_SOURCE_DESC ICHN_EX_SOURCE_DESCS[IchnExTypeMAX - IchnExPciExpress] = {
  ///
  /// IchnExPciExpress
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnExMonitor
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnExSpi
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnExQRT
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnExGpioUnlock
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnExTmrOverflow
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_PCH_ACPI_PM1_EN
        },
        S_PCH_ACPI_PM1_EN,
        N_PCH_ACPI_PM1_EN_TMROF
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_PCH_ACPI_PM1_STS
        },
        S_PCH_ACPI_PM1_STS,
        N_PCH_ACPI_PM1_STS_TMROF
      }
    }
  },

  ///
  /// IchnExPcie0Hotplug
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
            (DEFAULT_PCI_BUS_NUMBER_PCH << 24) |
            (PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS << 16) |
            (PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_1 << 8) |
            R_PCH_PCIE_MPC
          )
        },
        S_PCH_PCIE_MPC,
        N_PCH_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
            (DEFAULT_PCI_BUS_NUMBER_PCH << 24) |
            (PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS << 16) |
            (PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_1 << 8) |
            R_PCH_PCIE_SMSCS
          )
        },
        S_PCH_PCIE_SMSCS,
        N_PCH_PCIE_SMSCS_HPPDM
      }
    }
  },
  ///
  /// IchnExPcie1Hotplug
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
            (DEFAULT_PCI_BUS_NUMBER_PCH << 24) |
            (PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS << 16) |
            (PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_2 << 8) |
            R_PCH_PCIE_MPC
          )
        },
        S_PCH_PCIE_MPC,
        N_PCH_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
            (DEFAULT_PCI_BUS_NUMBER_PCH << 24) |
            (PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS << 16) |
            (PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_2 << 8) |
            R_PCH_PCIE_SMSCS
          )
        },
        S_PCH_PCIE_SMSCS,
        N_PCH_PCIE_SMSCS_HPPDM
      }
    }
  },
  ///
  /// IchnExPcie2Hotplug
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
            (DEFAULT_PCI_BUS_NUMBER_PCH << 24) |
            (PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS << 16) |
            (PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_3 << 8) |
            R_PCH_PCIE_MPC
          )
        },
        S_PCH_PCIE_MPC,
        N_PCH_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
            (DEFAULT_PCI_BUS_NUMBER_PCH << 24) |
            (PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS << 16) |
            (PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_3 << 8) |
            R_PCH_PCIE_SMSCS
          )
        },
        S_PCH_PCIE_SMSCS,
        N_PCH_PCIE_SMSCS_HPPDM
      }
    }
  },
  ///
  /// IchnExPcie3Hotplug
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
            (DEFAULT_PCI_BUS_NUMBER_PCH << 24) |
            (PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS << 16) |
            (PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_4 << 8) |
            R_PCH_PCIE_MPC
          )
        },
        S_PCH_PCIE_MPC,
        N_PCH_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
            (DEFAULT_PCI_BUS_NUMBER_PCH << 24) |
            (PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS << 16) |
            (PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_4 << 8) |
            R_PCH_PCIE_SMSCS
          )
        },
        S_PCH_PCIE_SMSCS,
        N_PCH_PCIE_SMSCS_HPPDM
      }
    }
  },
  ///
  /// IchnExPcie0LinkActive
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
            (DEFAULT_PCI_BUS_NUMBER_PCH << 24) |
            (PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS << 16) |
            (PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_1 << 8) |
            R_PCH_PCIE_MPC
          )
        },
        S_PCH_PCIE_MPC,
        N_PCH_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
            (DEFAULT_PCI_BUS_NUMBER_PCH << 24) |
            (PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS << 16) |
            (PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_1 << 8) |
            R_PCH_PCIE_SMSCS
          )
        },
        S_PCH_PCIE_SMSCS,
        N_PCH_PCIE_SMSCS_HPLAS
      }
    }
  },
  ///
  /// IchnExPcie1LinkActive
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
            (DEFAULT_PCI_BUS_NUMBER_PCH << 24) |
            (PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS << 16) |
            (PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_2 << 8) |
            R_PCH_PCIE_MPC
          )
        },
        S_PCH_PCIE_MPC,
        N_PCH_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
            (DEFAULT_PCI_BUS_NUMBER_PCH << 24) |
            (PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS << 16) |
            (PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_2 << 8) |
            R_PCH_PCIE_SMSCS
          )
        },
        S_PCH_PCIE_SMSCS,
        N_PCH_PCIE_SMSCS_HPLAS
      }
    }
  },
  ///
  /// IchnExPcie2LinkActive
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
            (DEFAULT_PCI_BUS_NUMBER_PCH << 24) |
            (PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS << 16) |
            (PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_3 << 8) |
            R_PCH_PCIE_MPC
          )
        },
        S_PCH_PCIE_MPC,
        N_PCH_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
            (DEFAULT_PCI_BUS_NUMBER_PCH << 24) |
            (PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS << 16) |
            (PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_3 << 8) |
            R_PCH_PCIE_SMSCS
          )
        },
        S_PCH_PCIE_SMSCS,
        N_PCH_PCIE_SMSCS_HPLAS
      }
    }
  },
  ///
  /// IchnExPcie3LinkActive
  ///
  {
    PCH_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
            (DEFAULT_PCI_BUS_NUMBER_PCH << 24) |
            (PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS << 16) |
            (PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_4 << 8) |
            R_PCH_PCIE_MPC
          )
        },
        S_PCH_PCIE_MPC,
        N_PCH_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
            (DEFAULT_PCI_BUS_NUMBER_PCH << 24) |
            (PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS << 16) |
            (PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_4 << 8) |
            R_PCH_PCIE_SMSCS
          )
        },
        S_PCH_PCIE_SMSCS,
        N_PCH_PCIE_SMSCS_HPLAS
      }
    }
  },
};

///
/// TCO_STS bit that needs to be cleared
///
PCH_SMM_SOURCE_DESC TCO_STS = {
  PCH_SMM_NO_FLAGS,
  {
    NULL_BIT_DESC_INITIALIZER,
    NULL_BIT_DESC_INITIALIZER
  },
  {
    {
      {
        ACPI_ADDR_TYPE,
        R_PCH_SMI_STS
      },
      S_PCH_SMI_STS,
      N_PCH_SMI_STS_TCO
    }
  }
};

VOID
PchSmmIchnClearSource (
  PCH_SMM_SOURCE_DESC   *SrcDesc
  )
/**

  @brief
  Clear the SMI status bit after the SMI handling is done

  @param[in] SrcDesc              Pointer to the PCH SMI source description table

  @retval None

**/
{
  PchSmmClearSource (SrcDesc);
  ///
  /// Any TCO-based status bits require special handling.
  /// SMI_STS.TCO_STS must be cleared in addition to the status bit in the TCO registers
  ///
  PchSmmClearSource (&TCO_STS);
}
