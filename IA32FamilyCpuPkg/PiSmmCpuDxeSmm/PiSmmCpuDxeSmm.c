/*++
  This file contains an 'Intel Peripheral Driver' and is        
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/
/** @file
  Agent Module to load other modules to deploy SMM Entry Vector for X86 CPU.

  Copyright (c) 2009 - 2012, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/

#include "PiSmmCpuDxeSmm.h"

//
// SMM CPU Private Data structure that contains SMM Configuration Protocol 
// along its supporting fields.
//
SMM_CPU_PRIVATE_DATA  mSmmCpuPrivateData = {
  SMM_CPU_PRIVATE_DATA_SIGNATURE,               // Signature
  NULL,                                         // SmmCpuHandle
  0,                                            // IEDBase
  0,                                            // SmrrBase
  0,                                            // SmrrSize
  { 0 },                                        // ApicIds array
  { 0 },                                        // SmBases array
  { 0 },                                        // CpuSaveStateSize array
  { NULL },                                     // CpuSaveState array
  { 0 },                                        // TstateMsr array
  { 0 },                                        // SmmReservedSmramRegion
  {
    SmmStartupThisAp,                           // SmmCoreEntryContext.SmmStartupThisAp
    0,                                          // SmmCoreEntryContext.CurrentlyExecutingCpu
    0,                                          // SmmCoreEntryContext.NumberOfCpus
    mSmmCpuPrivateData.CpuSaveStateSize,        // SmmCoreEntryContext.CpuSaveStateSize
    mSmmCpuPrivateData.CpuSaveState             // SmmCoreEntryContext.CpuSaveStateSize
  },
  NULL,                                         // SmmCoreEntry
  {
    mSmmCpuPrivateData.SmmReservedSmramRegion,  // SmmConfiguration.SmramReservedRegions
    RegisterSmmEntry                            // SmmConfiguration.RegisterSmmEntry
  },
};

//
// Global pointer used to access mSmmCpuPrivateData from outside and inside SMM
//
SMM_CPU_PRIVATE_DATA  *gSmmCpuPrivate = &mSmmCpuPrivateData;

//
// SMM Reloc variables
//
volatile BOOLEAN             mRebased[FixedPcdGet32 (PcdCpuMaxLogicalProcessorNumber)];
volatile BOOLEAN             mIsBsp;

///
/// Handle for the SMM CPU Protocol
///
EFI_HANDLE  mSmmCpuHandle = NULL;

///
/// SMM CPU Protocol instance
///
EFI_SMM_CPU_PROTOCOL  mSmmCpu  = {
  SmmReadSaveState,
  SmmWriteSaveState
};

///
/// SMM CPU Save State Protocol instance
///
EFI_SMM_CPU_SAVE_STATE_PROTOCOL  mSmmCpuSaveState = {
  (EFI_SMM_CPU_STATE  **)mSmmCpuPrivateData.CpuSaveState
};

//
// SMM stack information
//
UINTN mSmmStackArrayBase;
UINTN mSmmStackArrayEnd;
UINTN mSmmStackSize;

//
// Pointer to structure used during S3 Resume
//
SMM_S3_RESUME_STATE *mSmmS3ResumeState = NULL;

///
/// Macro used to simplfy the lookup table entries of type CPU_SMM_SAVE_STATE_LOOKUP_ENTRY
///
#define SMM_CPU_OFFSET(Field) OFFSET_OF (SOCKET_LGA_775_SMM_CPU_STATE, Field)

///
/// Macro used to simplfy the lookup table entries of type CPU_SMM_SAVE_STATE_REGISTER_RANGE
///
#define SMM_REGISTER_RANGE(Start, End) { Start, End, End - Start + 1 }

///
/// Table used by GetRegisterIndex() to convert an EFI_SMM_SAVE_STATE_REGISTER 
/// value to an index into a table of type CPU_SMM_SAVE_STATE_LOOKUP_ENTRY
///
CONST CPU_SMM_SAVE_STATE_REGISTER_RANGE mSmmCpuRegisterRanges[] = {
  SMM_REGISTER_RANGE (EFI_SMM_SAVE_STATE_REGISTER_GDTBASE, EFI_SMM_SAVE_STATE_REGISTER_LDTINFO),
  SMM_REGISTER_RANGE (EFI_SMM_SAVE_STATE_REGISTER_ES,      EFI_SMM_SAVE_STATE_REGISTER_RIP),
  SMM_REGISTER_RANGE (EFI_SMM_SAVE_STATE_REGISTER_RFLAGS,  EFI_SMM_SAVE_STATE_REGISTER_CR4),
  { (EFI_SMM_SAVE_STATE_REGISTER)0, (EFI_SMM_SAVE_STATE_REGISTER)0, 0 }
};

///
/// Lookup table used to retrieve the widths and offsets associated with each 
/// supported EFI_SMM_SAVE_STATE_REGISTER value 
///
CONST CPU_SMM_SAVE_STATE_LOOKUP_ENTRY mSmmCpuWidthOffset[] = {
  {0, 0, 0, 0, 0},                                                                                                     //  Reserved

  {0, 8, 0                            , SMM_CPU_OFFSET (x64.GdtBaseLoDword) , SMM_CPU_OFFSET (x64.GdtBaseHiDword) },  //  EFI_SMM_SAVE_STATE_REGISTER_GDTBASE  = 4
  {0, 8, 0                            , SMM_CPU_OFFSET (x64.IdtBaseLoDword) , SMM_CPU_OFFSET (x64.IdtBaseHiDword) },  //  EFI_SMM_SAVE_STATE_REGISTER_IDTBASE  = 5
  {0, 8, 0                            , SMM_CPU_OFFSET (x64.LdtBaseLoDword) , SMM_CPU_OFFSET (x64.LdtBaseHiDword) },  //  EFI_SMM_SAVE_STATE_REGISTER_LDTBASE  = 6
  {0, 4, 0                            , SMM_CPU_OFFSET (x64.GdtLimit)       , 0                                   },  //  EFI_SMM_SAVE_STATE_REGISTER_GDTLIMIT = 7
  {0, 4, 0                            , SMM_CPU_OFFSET (x64.IdtLimit)       , 0                                   },  //  EFI_SMM_SAVE_STATE_REGISTER_IDTLIMIT = 8
  {0, 4, 0                            , SMM_CPU_OFFSET (x64.LdtLimit)       , 0                                   },  //  EFI_SMM_SAVE_STATE_REGISTER_LDTLIMIT = 9
  {0, 4, 0                            , SMM_CPU_OFFSET (x64.LdtInfo)        , 0                                   },  //  EFI_SMM_SAVE_STATE_REGISTER_LDTINFO  = 10

  {4, 4, SMM_CPU_OFFSET (x86._ES)     , SMM_CPU_OFFSET (x64._ES)     , 0                                },  //  EFI_SMM_SAVE_STATE_REGISTER_ES       = 20
  {4, 4, SMM_CPU_OFFSET (x86._CS)     , SMM_CPU_OFFSET (x64._CS)     , 0                                },  //  EFI_SMM_SAVE_STATE_REGISTER_CS       = 21
  {4, 4, SMM_CPU_OFFSET (x86._SS)     , SMM_CPU_OFFSET (x64._SS)     , 0                                },  //  EFI_SMM_SAVE_STATE_REGISTER_SS       = 22
  {4, 4, SMM_CPU_OFFSET (x86._DS)     , SMM_CPU_OFFSET (x64._DS)     , 0                                },  //  EFI_SMM_SAVE_STATE_REGISTER_DS       = 23
  {4, 4, SMM_CPU_OFFSET (x86._FS)     , SMM_CPU_OFFSET (x64._FS)     , 0                                },  //  EFI_SMM_SAVE_STATE_REGISTER_FS       = 24
  {4, 4, SMM_CPU_OFFSET (x86._GS)     , SMM_CPU_OFFSET (x64._GS)     , 0                                },  //  EFI_SMM_SAVE_STATE_REGISTER_GS       = 25
  {0, 4, 0                            , SMM_CPU_OFFSET (x64._LDTR)   , 0                                },  //  EFI_SMM_SAVE_STATE_REGISTER_LDTR_SEL = 26
  {4, 4, SMM_CPU_OFFSET (x86._TR)     , SMM_CPU_OFFSET (x64._TR)     , 0                                },  //  EFI_SMM_SAVE_STATE_REGISTER_TR_SEL   = 27
  {4, 8, SMM_CPU_OFFSET (x86._DR7)    , SMM_CPU_OFFSET (x64._DR7)    , SMM_CPU_OFFSET (x64._DR7)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_DR7      = 28
  {4, 8, SMM_CPU_OFFSET (x86._DR6)    , SMM_CPU_OFFSET (x64._DR6)    , SMM_CPU_OFFSET (x64._DR6)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_DR6      = 29
  {0, 8, 0                            , SMM_CPU_OFFSET (x64._R8)     , SMM_CPU_OFFSET (x64._R8)     + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_R8       = 30
  {0, 8, 0                            , SMM_CPU_OFFSET (x64._R9)     , SMM_CPU_OFFSET (x64._R9)     + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_R9       = 31
  {0, 8, 0                            , SMM_CPU_OFFSET (x64._R10)    , SMM_CPU_OFFSET (x64._R10)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_R10      = 32
  {0, 8, 0                            , SMM_CPU_OFFSET (x64._R11)    , SMM_CPU_OFFSET (x64._R11)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_R11      = 33
  {0, 8, 0                            , SMM_CPU_OFFSET (x64._R12)    , SMM_CPU_OFFSET (x64._R12)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_R12      = 34
  {0, 8, 0                            , SMM_CPU_OFFSET (x64._R13)    , SMM_CPU_OFFSET (x64._R13)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_R13      = 35
  {0, 8, 0                            , SMM_CPU_OFFSET (x64._R14)    , SMM_CPU_OFFSET (x64._R14)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_R14      = 36
  {0, 8, 0                            , SMM_CPU_OFFSET (x64._R15)    , SMM_CPU_OFFSET (x64._R15)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_R15      = 37  
  {4, 8, SMM_CPU_OFFSET (x86._EAX)    , SMM_CPU_OFFSET (x64._RAX)    , SMM_CPU_OFFSET (x64._RAX)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_RAX      = 38
  {4, 8, SMM_CPU_OFFSET (x86._EBX)    , SMM_CPU_OFFSET (x64._RBX)    , SMM_CPU_OFFSET (x64._RBX)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_RBX      = 39
  {4, 8, SMM_CPU_OFFSET (x86._ECX)    , SMM_CPU_OFFSET (x64._RCX)    , SMM_CPU_OFFSET (x64._RCX)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_RCX      = 40
  {4, 8, SMM_CPU_OFFSET (x86._EDX)    , SMM_CPU_OFFSET (x64._RDX)    , SMM_CPU_OFFSET (x64._RDX)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_RDX      = 41
  {4, 8, SMM_CPU_OFFSET (x86._ESP)    , SMM_CPU_OFFSET (x64._RSP)    , SMM_CPU_OFFSET (x64._RSP)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_RSP      = 42
  {4, 8, SMM_CPU_OFFSET (x86._EBP)    , SMM_CPU_OFFSET (x64._RBP)    , SMM_CPU_OFFSET (x64._RBP)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_RBP      = 43
  {4, 8, SMM_CPU_OFFSET (x86._ESI)    , SMM_CPU_OFFSET (x64._RSI)    , SMM_CPU_OFFSET (x64._RSI)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_RSI      = 44
  {4, 8, SMM_CPU_OFFSET (x86._EDI)    , SMM_CPU_OFFSET (x64._RDI)    , SMM_CPU_OFFSET (x64._RDI)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_RDI      = 45
  {4, 8, SMM_CPU_OFFSET (x86._EIP)    , SMM_CPU_OFFSET (x64._RIP)    , SMM_CPU_OFFSET (x64._RIP)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_RIP      = 46

  {4, 8, SMM_CPU_OFFSET (x86._EFLAGS) , SMM_CPU_OFFSET (x64._RFLAGS) , SMM_CPU_OFFSET (x64._RFLAGS) + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_RFLAGS   = 51
  {4, 8, SMM_CPU_OFFSET (x86._CR0)    , SMM_CPU_OFFSET (x64._CR0)    , SMM_CPU_OFFSET (x64._CR0)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_CR0      = 52 
  {4, 8, SMM_CPU_OFFSET (x86._CR3)    , SMM_CPU_OFFSET (x64._CR3)    , SMM_CPU_OFFSET (x64._CR3)    + 4 },  //  EFI_SMM_SAVE_STATE_REGISTER_CR3      = 53
  {0, 0, 0, 0, 0},                                                                                          //  EFI_SMM_SAVE_STATE_REGISTER_CR4      = 54
};

///
/// Lookup table for the IOMisc width information
///
CONST CPU_SMM_SAVE_STATE_IO_WIDTH mSmmCpuIoWidth[] = {
  { 0, EFI_SMM_SAVE_STATE_IO_WIDTH_UINT8  },  // Undefined           = 0
  { 1, EFI_SMM_SAVE_STATE_IO_WIDTH_UINT8  },  // SMM_IO_LENGTH_BYTE  = 1
  { 2, EFI_SMM_SAVE_STATE_IO_WIDTH_UINT16 },  // SMM_IO_LENGTH_WORD  = 2  
  { 0, EFI_SMM_SAVE_STATE_IO_WIDTH_UINT8  },  // Undefined           = 3 
  { 4, EFI_SMM_SAVE_STATE_IO_WIDTH_UINT32 },  // SMM_IO_LENGTH_DWORD = 4 
  { 0, EFI_SMM_SAVE_STATE_IO_WIDTH_UINT8  },  // Undefined           = 5 
  { 0, EFI_SMM_SAVE_STATE_IO_WIDTH_UINT8  },  // Undefined           = 6 
  { 0, EFI_SMM_SAVE_STATE_IO_WIDTH_UINT8  }   // Undefined           = 7 
};

///
/// Lookup table for the IOMisc type information
///
CONST CPU_SMM_SAVE_STATE_IO_TYPE mSmmCpuIoType[] = {
  { EFI_SMM_SAVE_STATE_IO_TYPE_OUTPUT,     SMM_CPU_OFFSET (x86._EAX),      SMM_CPU_OFFSET (x64._RAX)      }, // SMM_IO_TYPE_OUT_DX        = 0        
  { EFI_SMM_SAVE_STATE_IO_TYPE_INPUT,      SMM_CPU_OFFSET (x86._EAX),      SMM_CPU_OFFSET (x64._RAX)      }, // SMM_IO_TYPE_IN_DX         = 1         
  { EFI_SMM_SAVE_STATE_IO_TYPE_STRING,     SMM_CPU_OFFSET (x86.IOMemAddr), SMM_CPU_OFFSET (x64.IOMemAddr) }, // SMM_IO_TYPE_OUTS          = 2          
  { EFI_SMM_SAVE_STATE_IO_TYPE_STRING,     SMM_CPU_OFFSET (x86.IOMemAddr), SMM_CPU_OFFSET (x64.IOMemAddr) }, // SMM_IO_TYPE_INS           = 3           
  { (EFI_SMM_SAVE_STATE_IO_TYPE)0, 0, 0},                                                                    // Undefined                 = 4
  { (EFI_SMM_SAVE_STATE_IO_TYPE)0, 0, 0},                                                                    // Undefined                 = 5
  { EFI_SMM_SAVE_STATE_IO_TYPE_REP_PREFIX, SMM_CPU_OFFSET (x86.IOMemAddr), SMM_CPU_OFFSET (x64.IOMemAddr) }, // SMM_IO_TYPE_REP_OUTS      = 6      
  { EFI_SMM_SAVE_STATE_IO_TYPE_REP_PREFIX, SMM_CPU_OFFSET (x86.IOMemAddr), SMM_CPU_OFFSET (x64.IOMemAddr) }, // SMM_IO_TYPE_REP_INS       = 7       
  { EFI_SMM_SAVE_STATE_IO_TYPE_OUTPUT,     SMM_CPU_OFFSET (x86._EAX),      SMM_CPU_OFFSET (x64._RAX)      }, // SMM_IO_TYPE_OUT_IMMEDIATE = 8 
  { EFI_SMM_SAVE_STATE_IO_TYPE_INPUT,      SMM_CPU_OFFSET (x86._EAX),      SMM_CPU_OFFSET (x64._RAX)      }, // SMM_IO_TYPE_OUT_IMMEDIATE = 9 
  { (EFI_SMM_SAVE_STATE_IO_TYPE)0, 0, 0},                                                                    // Undefined                 = 10
  { (EFI_SMM_SAVE_STATE_IO_TYPE)0, 0, 0},                                                                    // Undefined                 = 11
  { (EFI_SMM_SAVE_STATE_IO_TYPE)0, 0, 0},                                                                    // Undefined                 = 12
  { (EFI_SMM_SAVE_STATE_IO_TYPE)0, 0, 0},                                                                    // Undefined                 = 13
  { (EFI_SMM_SAVE_STATE_IO_TYPE)0, 0, 0},                                                                    // Undefined                 = 14
  { (EFI_SMM_SAVE_STATE_IO_TYPE)0, 0, 0}                                                                     // Undefined                 = 15
};

/**
  Read information from the CPU save state.

  @param  Register  Specifies the CPU register to read form the save state.

  @retval 0   Register is not valid
  @retval >0  Index into mSmmCpuWidthOffset[] associated with Register

**/
UINTN
GetRegisterIndex (
  IN EFI_SMM_SAVE_STATE_REGISTER  Register
)
{
  UINTN  Index;
  UINTN  Offset;
  
  for (Index = 0, Offset = 1; mSmmCpuRegisterRanges[Index].Length != 0; Index++) {
    if (Register >= mSmmCpuRegisterRanges[Index].Start && Register <= mSmmCpuRegisterRanges[Index].End) {
      return Register - mSmmCpuRegisterRanges[Index].Start + Offset;
    }
    Offset += mSmmCpuRegisterRanges[Index].Length; 
  }
  return 0;
}

/**
  Read information from the CPU save state.

  @param  This      EFI_SMM_CPU_PROTOCOL instance
  @param  Widthe    The number of bytes to read from the CPU save state.
  @param  Register  Specifies the CPU register to read form the save state.
  @param  CpuIndex  Specifies the zero-based index of the CPU save state
  @param  Buffer    Upon return, this holds the CPU register value read from the save state.

  @retval EFI_SUCCESS   The register was read from Save State
  @retval EFI_NOT_FOUND The register is not defined for the Save State of Processor  
  @retval EFI_INVALID_PARAMTER   This or Buffer is NULL.

**/
EFI_STATUS
EFIAPI
SmmReadSaveState (
  IN CONST EFI_SMM_CPU_PROTOCOL         *This,
  IN UINTN                              Width,
  IN EFI_SMM_SAVE_STATE_REGISTER        Register,
  IN UINTN                              CpuIndex,
  OUT VOID                              *Buffer
  )
{
  SOCKET_LGA_775_SMM_CPU_STATE         *StateCopy;
  UINT32                               SmmRevId;
  SOCKET_LGA_775_SMM_CPU_STATE_IOMISC  IoMisc;
  EFI_SMM_SAVE_STATE_IO_INFO           *IoInfo;
  UINTN                                RegisterIndex;

  //
  // Retrieve pointer to the specified CPU's SMM Save State buffer
  //
  if ((CpuIndex >= gSmst->NumberOfCpus) || (Buffer == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  StateCopy = gSmst->CpuSaveState[CpuIndex];

  SmmRevId = StateCopy->x86.SMMRevId;

  //
  // Check for special EFI_SMM_SAVE_STATE_REGISTER_LMA
  //
  if (Register == EFI_SMM_SAVE_STATE_REGISTER_LMA) {
    //
    // Only byte access is supported for this register
    //
    if (Width != 1) {
      return EFI_INVALID_PARAMETER;          
    }

    if ((SmmRevId < SOCKET_LGA_775_SMM_MIN_REV_ID_x64)) {
      *(UINT8 *)Buffer = EFI_SMM_SAVE_STATE_REGISTER_LMA_32BIT;
    } else {
      *(UINT8 *)Buffer = EFI_SMM_SAVE_STATE_REGISTER_LMA_64BIT;
    }
    
    return EFI_SUCCESS;
  }

  //
  // Check for special EFI_SMM_SAVE_STATE_REGISTER_IO
  //
  if (Register == EFI_SMM_SAVE_STATE_REGISTER_IO) {
    //
    // See if the CPU supports the IOMisc register in the save state
    //
    if (SmmRevId < SOCKET_LGA_775_SMM_MIN_REV_ID_IOMISC) {
      return EFI_NOT_FOUND;          
    }
    
    //
    // Get the IOMisc register value
    //
    if (SmmRevId < SOCKET_LGA_775_SMM_MIN_REV_ID_x64) {
      IoMisc.Uint32 = StateCopy->x86.IOMisc;
    } else {
      IoMisc.Uint32 = StateCopy->x64.IOMisc;
    }

    //
    // Check for the SMI_FLAG in IOMisc
    //
    if (IoMisc.Bits.SmiFlag == 0) {
      return EFI_NOT_FOUND;          
    }
    
    //
    // Compute index for the I/O Length and I/O Type lookup tables
    //
    if (mSmmCpuIoWidth[IoMisc.Bits.Length].Width == 0 || mSmmCpuIoType[IoMisc.Bits.Type].IoType == 0) {
      return EFI_NOT_FOUND;          
    }
    
    //
    // Zero the IoInfo structure that will be returned in Buffer
    //
    IoInfo = (EFI_SMM_SAVE_STATE_IO_INFO *)Buffer;
    ZeroMem (IoInfo, sizeof (EFI_SMM_SAVE_STATE_IO_INFO));
    
    //
    // Use lookup tables to help fill in all the fields of the IoInfo structure  
    //
    IoInfo->IoPort  = (UINT16)IoMisc.Bits.Port;
    IoInfo->IoWidth = mSmmCpuIoWidth[IoMisc.Bits.Length].IoWidth;
    IoInfo->IoType  = mSmmCpuIoType[IoMisc.Bits.Type].IoType;
    if (SmmRevId < SOCKET_LGA_775_SMM_MIN_REV_ID_x64) {
      CopyMem(&IoInfo->IoData, (UINT8*)StateCopy + mSmmCpuIoType[IoMisc.Bits.Type].IoDataOffset32, mSmmCpuIoWidth[IoMisc.Bits.Length].Width);
    } else {
      CopyMem(&IoInfo->IoData, (UINT8*)StateCopy + mSmmCpuIoType[IoMisc.Bits.Type].IoDataOffset64, mSmmCpuIoWidth[IoMisc.Bits.Length].Width);
    }
    
    return EFI_SUCCESS;
  }

  //
  // Convert Register to a register lookup table index
  //
  RegisterIndex = GetRegisterIndex (Register);
  if (RegisterIndex == 0) {
    return EFI_NOT_FOUND;
  }

  if (SmmRevId < SOCKET_LGA_775_SMM_MIN_REV_ID_x64) {
    //
    // If 32-bit mode width is zero, then the specified register can not be accessed
    //
    if (mSmmCpuWidthOffset[RegisterIndex].Width32 == 0) {
      return EFI_NOT_FOUND;
    }
    
    //
    // If Width is bigger than the 32-bit mode width, then the specified register can not be accessed
    //
    if (Width > mSmmCpuWidthOffset[RegisterIndex].Width32) {
      return EFI_INVALID_PARAMETER;          
    }
    
    //
    // Write return buffer
    //
    CopyMem(Buffer, (UINT8 *)StateCopy + mSmmCpuWidthOffset[RegisterIndex].Offset32, Width);
  } else {
    //
    // If 64-bit mode width is zero, then the specified register can not be accessed
    //
    if (mSmmCpuWidthOffset[RegisterIndex].Width64 == 0) {
      return EFI_NOT_FOUND;
    }
    
    //
    // If Width is bigger than the 64-bit mode width, then the specified register can not be accessed
    //
    if (Width > mSmmCpuWidthOffset[RegisterIndex].Width64) {
      return EFI_INVALID_PARAMETER;          
    }
    
    //
    // Write lower 32-bits of return buffer
    //
    CopyMem(Buffer, (UINT8 *)StateCopy + mSmmCpuWidthOffset[RegisterIndex].Offset64Lo, MIN (4, Width));
    if (Width >= 4) {
      //
      // Write upper 32-bits of return buffer
      //
      CopyMem((UINT8 *)Buffer + 4, (UINT8 *)StateCopy + mSmmCpuWidthOffset[RegisterIndex].Offset64Hi, Width - 4);
    }
  }
  return EFI_SUCCESS;
}

/**
  Write data to the CPU save state.

  @param  This      EFI_SMM_CPU_PROTOCOL instance
  @param  Widthe    The number of bytes to read from the CPU save state.
  @param  Register  Specifies the CPU register to write to the save state.
  @param  CpuIndex  Specifies the zero-based index of the CPU save state
  @param  Buffer    Upon entry, this holds the new CPU register value.

  @retval EFI_SUCCESS   The register was written from Save State
  @retval EFI_NOT_FOUND The register is not defined for the Save State of Processor  
  @retval EFI_INVALID_PARAMTER   ProcessorIndex or Width is not correct

**/
EFI_STATUS
EFIAPI
SmmWriteSaveState (
  IN CONST EFI_SMM_CPU_PROTOCOL         *This,
  IN UINTN                              Width, 
  IN EFI_SMM_SAVE_STATE_REGISTER        Register,
  IN UINTN                              CpuIndex,
  IN CONST VOID                         *Buffer
  )
{
  SOCKET_LGA_775_SMM_CPU_STATE  *StateCopy;
  UINTN                         RegisterIndex;

  //
  // Retrieve pointer to the specified CPU's SMM Save State buffer
  //
  if ((CpuIndex >= gSmst->NumberOfCpus) || (Buffer == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  StateCopy = gSmst->CpuSaveState[CpuIndex];

  //
  // Writes to EFI_SMM_SAVE_STATE_REGISTER_LMA are ignored
  //
  if (Register == EFI_SMM_SAVE_STATE_REGISTER_LMA) {
    return EFI_SUCCESS;
  }

  //
  // Writes to EFI_SMM_SAVE_STATE_REGISTER_IO are not supported
  //
  if (Register == EFI_SMM_SAVE_STATE_REGISTER_IO) {
    return EFI_NOT_FOUND;          
  }

  //
  // Convert Register to a register lookup table index
  //
  RegisterIndex = GetRegisterIndex (Register);
  if (RegisterIndex == 0) {
    //
    // If Register is not valie, then return EFI_NOT_FOUND
    //
    return EFI_NOT_FOUND;
  }

  //
  // Check CPU mode
  //
  if (StateCopy->x86.SMMRevId < SOCKET_LGA_775_SMM_MIN_REV_ID_x64) {
    //
    // If 32-bit mode width is zero, then the specified register can not be accessed
    //
    if (mSmmCpuWidthOffset[RegisterIndex].Width32 == 0) {
      return EFI_NOT_FOUND;
    }
    
    //
    // If Width is bigger than the 32-bit mode width, then the specified register can not be accessed
    //
    if (Width > mSmmCpuWidthOffset[RegisterIndex].Width32) {
      return EFI_INVALID_PARAMETER;
    }
    //
    // Write SMM State register
    //
    CopyMem((UINT8 *)StateCopy + mSmmCpuWidthOffset[RegisterIndex].Offset32, Buffer, Width);
  } else {
    //
    // If 64-bit mode width is zero, then the specified register can not be accessed
    //
    if (mSmmCpuWidthOffset[RegisterIndex].Width64 == 0) {
      return EFI_NOT_FOUND;
    }
    
    //
    // If Width is bigger than the 64-bit mode width, then the specified register can not be accessed
    //
    if (Width > mSmmCpuWidthOffset[RegisterIndex].Width64) {
      return EFI_INVALID_PARAMETER;          
    }
    
    //
    // Write lower 32-bits of SMM State register
    //
    CopyMem((UINT8 *)StateCopy + mSmmCpuWidthOffset[RegisterIndex].Offset64Lo, Buffer, MIN (4, Width));
    if (Width >= 4) {
      //
      // Write upper 32-bits of SMM State register
      //
      CopyMem((UINT8 *)StateCopy + mSmmCpuWidthOffset[RegisterIndex].Offset64Hi, (UINT8 *)Buffer + 4, Width - 4);
    }
  }
  return EFI_SUCCESS;
}

/**
  C function for SMI handler. To change all processor's SMMBase Register.

**/
VOID
EFIAPI
SmmInitHandler (
  VOID
  )
{
  UINT32                            ApicId;
  UINTN                             Index;
  SOCKET_LGA_775_SMM_CPU_STATE      *CpuState;

  ApicId = GetApicId ();

  for (Index = 0; Index < gSmmCpuPrivate->SmmCoreEntryContext.NumberOfCpus; Index++) {
    if (ApicId == gSmmCpuPrivate->ApicIds[Index]) {
      CpuState = (SOCKET_LGA_775_SMM_CPU_STATE *)(UINTN)(SMM_DEFAULT_SMBASE + SMM_CPU_STATE_OFFSET);
      CpuState->x86.SMBASE = (UINT32)gSmmCpuPrivate->SmBases[Index];

      SmmInitiFeatures (gSmmCpuPrivate->SmrrBase, gSmmCpuPrivate->SmrrSize, gSmmCpuPrivate->IEDBase, mIsBsp);

      if (!mIsBsp) {
        SemaphoreHook ((BOOLEAN *) &mRebased[Index], CpuState);
      } else {
          //
          //   BSP rebase is already done above.
          //   Initialize private data during S3 resume
          //
          InitializeUcData ();
      }
      return;
    }
  }
  ASSERT (FALSE);
}

/**
  Relocate SmmBases for each processor.

  Execute on first boot and all S3 resumes

**/
VOID
EFIAPI
SmmRelocateBases (
  VOID
  )
{
  UINT8                             BakBuf[BACK_BUF_SIZE];
  SOCKET_LGA_775_SMM_CPU_STATE      BakBuf2;
  SOCKET_LGA_775_SMM_CPU_STATE      *CpuStatePtr;
  UINT8                             *U8Ptr;
  UINT32                            ApicId;
  UINTN                             Index;

  //
  // Make sure the reserved size is large enough for procedure SmmInitTemplate.
  //
  ASSERT (sizeof (BakBuf) >= gcSmmInitSize);

  //
  // Patch ASM code template with current CR0, CR3, and CR4 values
  //
  gSmmCr0 = (UINT32)AsmReadCr0 ();
  gSmmCr3 = (UINT32)AsmReadCr3 ();
  gSmmCr4 = (UINT32)AsmReadCr4 ();

  U8Ptr = (UINT8*)(UINTN)(SMM_DEFAULT_SMBASE + SMM_HANDLER_OFFSET);
  CpuStatePtr = (SOCKET_LGA_775_SMM_CPU_STATE *)(UINTN)(SMM_DEFAULT_SMBASE + SMM_CPU_STATE_OFFSET);

  //
  // Backup original contents @ 0x38000
  //
  CopyMem (BakBuf, U8Ptr, sizeof (BakBuf));
  CopyMem (&BakBuf2, CpuStatePtr, sizeof (BakBuf2));

  //
  // Load image for relocation
  //
  CopyMem (U8Ptr, gcSmmInitTemplate, gcSmmInitSize);

  //
  // Retrieve the local APIC ID of current processor
  //
  ApicId = GetApicId ();

  //
  // Relocate SM bases for all APs
  // This is APs' 1st SMI - rebase will be done here, and APs' default SMI handler will be overriden by gcSmmInitTemplate
  //
  mIsBsp = FALSE;
  for (Index = 0; Index < gSmmCpuPrivate->SmmCoreEntryContext.NumberOfCpus; Index++) {
    if (ApicId != gSmmCpuPrivate->ApicIds[Index]) {
      SendSmiIpi (gSmmCpuPrivate->ApicIds[Index]);

      //
      // Wait for this AP to finish its 1st SMI
      //
      while (!mRebased[Index]);
      mRebased[Index] = FALSE;
    }
  }

  //
  // Relocate BSP's SM base
  //
  mIsBsp = TRUE;
  SendSmiIpi (ApicId);

  //
  // Restore contents @ 0x38000
  //
  CopyMem (CpuStatePtr, &BakBuf2, sizeof (BakBuf2));
  CopyMem (U8Ptr, BakBuf, sizeof (BakBuf));
}

/**
  Perform SMM initialization for all processors in the S3 boot path.

  For a native platform, MP initialization in the S3 boot path is also performed in this function.
**/
VOID
SmmRestoreCpu (
  VOID
  )
{
  SMM_S3_RESUME_STATE  *SmmS3ResumeState;

  DEBUG ((EFI_D_ERROR, "SmmRestoreCpu()\n"));

  //
  // Do below CPU things for native platform only
  //
  if (!FeaturePcdGet(PcdFrameworkCompatibilitySupport)) {
    //
    // First time microcode load and restore MTRRs
    //
    EarlyInitializeCpu ();
  }

  //
  // Restore SMBASE for BSP and all APs
  //
  SmmRelocateBases ();

  //
  // Do below CPU things for native platform only
  //
  if (!FeaturePcdGet(PcdFrameworkCompatibilitySupport)) {
    //
    // Restore MSRs for BSP and all APs
    //
    InitializeCpu ();
  }

  //
  // See if there is enough context to resume PEI Phase
  //
  if (mSmmS3ResumeState == NULL) {
    DEBUG ((EFI_D_ERROR, "No context to return to PEI Phase\n"));
    CpuDeadLoop ();
  }

  SmmS3ResumeState = mSmmS3ResumeState;

  DEBUG (( EFI_D_ERROR, "SMM S3 Return CS                = %x\n", SmmS3ResumeState->ReturnCs));
  DEBUG (( EFI_D_ERROR, "SMM S3 Return Entry Point       = %x\n", SmmS3ResumeState->ReturnEntryPoint));
  DEBUG (( EFI_D_ERROR, "SMM S3 Return Context1          = %x\n", SmmS3ResumeState->ReturnContext1));
  DEBUG (( EFI_D_ERROR, "SMM S3 Return Context2          = %x\n", SmmS3ResumeState->ReturnContext2));
  DEBUG (( EFI_D_ERROR, "SMM S3 Return Stack Pointer     = %x\n", SmmS3ResumeState->ReturnStackPointer));

  //
  // If SMM is in 32-bit mode, then use SwitchStack() to resume PEI Phase
  //
  ASSERT (SmmS3ResumeState != NULL);
  if (SmmS3ResumeState->Signature == SMM_S3_RESUME_SMM_32) {
    DEBUG ((EFI_D_ERROR, "Call SwitchStack() to return to S3 Resume in PEI Phase\n"));

    SwitchStack (
      (SWITCH_STACK_ENTRY_POINT)(UINTN)SmmS3ResumeState->ReturnEntryPoint,
      (VOID *)(UINTN)SmmS3ResumeState->ReturnContext1,
      (VOID *)(UINTN)SmmS3ResumeState->ReturnContext2, 
      (VOID *)(UINTN)SmmS3ResumeState->ReturnStackPointer
      );
  } 

  //
  // If SMM is in 64-bit mode, then use AsmDisablePaging64() to resume PEI Phase
  //
  if (SmmS3ResumeState->Signature == SMM_S3_RESUME_SMM_64) {
    DEBUG ((EFI_D_ERROR, "Call AsmDisablePaging64() to return to S3 Resume in PEI Phase\n"));

    AsmDisablePaging64 (
      SmmS3ResumeState->ReturnCs,
      (UINT32)SmmS3ResumeState->ReturnEntryPoint,
      (UINT32)SmmS3ResumeState->ReturnContext1,
      (UINT32)SmmS3ResumeState->ReturnContext2,
      (UINT32)SmmS3ResumeState->ReturnStackPointer
      );
  }

  //
  // Can not resume PEI Phase
  //
  DEBUG ((EFI_D_ERROR, "No context to return to PEI Phase\n"));
  CpuDeadLoop ();
}

/**
  Smm Ready To Lock event notification handler.

  The CPU S3 data is copied to SMRAM for security.
  
  @param[in] Protocol   Points to the protocol's unique identifier.
  @param[in] Interface  Points to the interface instance.
  @param[in] Handle     The handle on which the interface was installed.

  @retval EFI_SUCCESS   Notification handler runs successfully.
 **/
EFI_STATUS
EFIAPI
SmmReadyToLockEventNotify (
  IN CONST EFI_GUID  *Protocol,
  IN VOID            *Interface,
  IN EFI_HANDLE      Handle
  )
{
  UINTN                      Index;
  CPU_REGISTER_TABLE         *SourceRegisterTableList;
  CPU_REGISTER_TABLE         *DestinationRegisterTableList;
  ACPI_CPU_DATA              *AcpiCpuData;
  IA32_DESCRIPTOR            *Gdtr;
  IA32_DESCRIPTOR            *Idtr;

  //
  // Get CPU S3 data address
  //
  AcpiCpuData = (ACPI_CPU_DATA *)(UINTN)PcdGet64 (PcdCpuS3DataAddress);

  //*** AMI PORTING BEGIN ***//
  //Bug fix: handle the case when PcdCpuS3DataAddress has not been set.
  //AMI code does not set PcdCpuS3DataAddress.
  //In Intel's code PcdCpuS3DataAddress is set in: 
  //package: IA32FamilyCpuPkg module: CpuMpDxe  file: ProcessorConfig.c.
  if (AcpiCpuData == NULL) return EFI_UNSUPPORTED;
  //*** AMI PORTING END *****//

  //
  // Copy CPU S3 data into SMRAM for use on CPU S3 Resume.
  //
  CopyMem (&mAcpiCpuData, AcpiCpuData, sizeof (mAcpiCpuData));

  mAcpiCpuData.MtrrTable = (EFI_PHYSICAL_ADDRESS)(UINTN)AllocatePool (sizeof (MTRR_SETTINGS));
  ASSERT (mAcpiCpuData.MtrrTable != 0);

  CopyMem ((VOID *)(UINTN)mAcpiCpuData.MtrrTable, (VOID *)(UINTN)AcpiCpuData->MtrrTable, sizeof (MTRR_SETTINGS));

  mAcpiCpuData.GdtrProfile = (EFI_PHYSICAL_ADDRESS)(UINTN)AllocatePool (sizeof (IA32_DESCRIPTOR));
  ASSERT (mAcpiCpuData.GdtrProfile != 0);

  CopyMem ((VOID *)(UINTN)mAcpiCpuData.GdtrProfile, (VOID *)(UINTN)AcpiCpuData->GdtrProfile, sizeof (IA32_DESCRIPTOR));

  mAcpiCpuData.IdtrProfile = (EFI_PHYSICAL_ADDRESS)(UINTN)AllocatePool (sizeof (IA32_DESCRIPTOR));
  ASSERT (mAcpiCpuData.IdtrProfile != 0);

  CopyMem ((VOID *)(UINTN)mAcpiCpuData.IdtrProfile, (VOID *)(UINTN)AcpiCpuData->IdtrProfile, sizeof (IA32_DESCRIPTOR));

  mAcpiCpuData.RegisterTable = (EFI_PHYSICAL_ADDRESS)(UINTN)AllocatePool (mAcpiCpuData.NumberOfCpus * sizeof (CPU_REGISTER_TABLE));
  ASSERT (mAcpiCpuData.RegisterTable != 0);

  CopyMem ((CPU_REGISTER_TABLE *)(UINTN)mAcpiCpuData.RegisterTable, (CPU_REGISTER_TABLE *)(UINTN)AcpiCpuData->RegisterTable, mAcpiCpuData.NumberOfCpus * sizeof (CPU_REGISTER_TABLE));

  SourceRegisterTableList      = (CPU_REGISTER_TABLE *)(UINTN)AcpiCpuData->RegisterTable;
  DestinationRegisterTableList = (CPU_REGISTER_TABLE *)(UINTN)mAcpiCpuData.RegisterTable;
  for (Index = 0; Index < mAcpiCpuData.NumberOfCpus; Index++) {
    DestinationRegisterTableList[Index].RegisterTableEntry = AllocatePool (DestinationRegisterTableList[Index].AllocatedSize);
    ASSERT (DestinationRegisterTableList[Index].RegisterTableEntry != NULL);

    CopyMem (DestinationRegisterTableList[Index].RegisterTableEntry, SourceRegisterTableList[Index].RegisterTableEntry, DestinationRegisterTableList[Index].AllocatedSize);
  }

  //
  // Copy AP's GDT, IDT and Machine Check handler into SMRAM. 
  //
  Gdtr = (IA32_DESCRIPTOR *)(UINTN)mAcpiCpuData.GdtrProfile;
  Idtr = (IA32_DESCRIPTOR *)(UINTN)mAcpiCpuData.IdtrProfile;

  mGdtForAp = AllocatePool ((Gdtr->Limit + 1) + (Idtr->Limit + 1) +  mAcpiCpuData.ApMachineCheckHandlerSize);
  ASSERT (mGdtForAp != NULL);
  mIdtForAp = (VOID *) ((UINTN)mGdtForAp + (Gdtr->Limit + 1));
  mMachineCheckHandlerForAp = (VOID *) ((UINTN)mIdtForAp + (Idtr->Limit + 1));

  CopyMem (mGdtForAp, (VOID *)Gdtr->Base, Gdtr->Limit + 1);
  CopyMem (mIdtForAp, (VOID *)Idtr->Base, Idtr->Limit + 1);
  CopyMem (mMachineCheckHandlerForAp, (VOID *)(UINTN)mAcpiCpuData.ApMachineCheckHandlerBase, mAcpiCpuData.ApMachineCheckHandlerSize);

  return EFI_SUCCESS;
}

/**
  The module Entry Point of the CPU SMM driver.

  @param  ImageHandle    The firmware allocated handle for the EFI image.
  @param  SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS    The entry point is executed successfully.
  @retval Other          Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
PiCpuSmmEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                 Status;
  EFI_MP_SERVICES_PROTOCOL   *MpServices;
  UINTN                      NumberOfEnabledProcessors;
  UINTN                      Index;
  EFI_PROCESSOR_INFORMATION  ProcessorInformation;
  VOID                       *Buffer;
  UINTN                      TileSize;
  VOID                       *GuidHob;
  EFI_SMRAM_DESCRIPTOR       *SmramDescriptor;
  SMM_S3_RESUME_STATE        *SmmS3ResumeState;
  UINT8                      *Stacks;
  VOID                       *Registration;

  //
  // Initialize Debug Agent to support source level debug in SMM code
  //
  InitializeDebugAgent (DEBUG_AGENT_INIT_SMM, NULL, NULL);

  //
  // Fix segment address of the long-mode-switch jmp
  //
  if (sizeof (UINTN) == sizeof (UINT64)) {
    gSmmJmpAddr.Segment = 8;
  }

  //
  // Find out SMRR Base and Size and IED Base
  //
  FindSmramInfo (&gSmmCpuPrivate->SmrrBase, &gSmmCpuPrivate->SmrrSize, &gSmmCpuPrivate->IEDBase);

  //
  // Get MP Services Protocol
  //
  Status = SystemTable->BootServices->LocateProtocol (&gEfiMpServiceProtocolGuid, NULL, (VOID **)&MpServices);
  ASSERT_EFI_ERROR (Status);

  //
  // Use MP Services Protocol to retrieve the number of processors and number of enabled processors
  //
  Status = MpServices->GetNumberOfProcessors (MpServices, &gSmmCpuPrivate->SmmCoreEntryContext.NumberOfCpus, &NumberOfEnabledProcessors);
  ASSERT_EFI_ERROR (Status);

  //
  // The CPU save state and code for the SMI entry point are tiled within an SMRAM 
  // allocated buffer.  The miniumu size of this buffer for a uniprocessor system 
  // is 32 KB, because the entry point is SMBASE + 32KB, and CPU save state area 
  // just below SMBASE + 64KB.  If more than one CPU is present in the platform, 
  // then the SMI entry point and the CPU save state areas can be tiles to minimize 
  // the total amount SMRAM required for all the CPUs.  The tile size can be computed 
  // by adding the   // CPU save state size, any extra CPU specific context, and 
  // the size of code that must be placed at the SMI entry point to transfer 
  // control to a C function in the native SMM execution mode.  This size is 
  // rounded up to the nearest power of 2 to give the tile size for a each CPU.
  // The total amount of memory required is the maximum number of CPUs that 
  // platform supports times the tile size.  The picture below shows the tiling, 
  // where m is the number of tiles that fit in 32KB.
  //
  //  +-----------------------------+  <-- 2^n offset from Base of allocated buffer
  //  |   CPU m+1 Save State        |
  //  +-----------------------------+
  //  |   CPU m+1 Extra Data        |
  //  +-----------------------------+  
  //  |   Padding                   |
  //  +-----------------------------+   
  //  |   CPU 2m  SMI Entry         |
  //  +#############################+  <-- Base of allocated buffer + 64 KB 
  //  |   CPU m-1 Save State        |
  //  +-----------------------------+
  //  |   CPU m-1 Extra Data        |
  //  +-----------------------------+  
  //  |   Padding                   |
  //  +-----------------------------+   
  //  |   CPU 2m-1 SMI Entry        |
  //  +=============================+  <-- 2^n offset from Base of allocated buffer
  //  |   . . . . . . . . . . . .   |
  //  +=============================+  <-- 2^n offset from Base of allocated buffer
  //  |   CPU 2 Save State          |
  //  +-----------------------------+
  //  |   CPU 2 Extra Data          |
  //  +-----------------------------+  
  //  |   Padding                   |
  //  +-----------------------------+   
  //  |   CPU m+1 SMI Entry         |
  //  +=============================+  <-- Base of allocated buffer + 32 KB
  //  |   CPU 1 Save State          |
  //  +-----------------------------+
  //  |   CPU 1 Extra Data          |
  //  +-----------------------------+  
  //  |   Padding                   |
  //  +-----------------------------+   
  //  |   CPU m SMI Entry           |
  //  +#############################+  <-- Base of allocated buffer + 32 KB == CPU 0 SMBASE + 64 KB
  //  |   CPU 0 Save State          |
  //  +-----------------------------+
  //  |   CPU 0 Extra Data          |
  //  +-----------------------------+  
  //  |   Padding                   |
  //  +-----------------------------+   
  //  |   CPU m-1 SMI Entry         |
  //  +=============================+  <-- 2^n offset from Base of allocated buffer
  //  |   . . . . . . . . . . . .   |
  //  +=============================+  <-- 2^n offset from Base of allocated buffer
  //  |   Padding                   |
  //  +-----------------------------+   
  //  |   CPU 1 SMI Entry           |
  //  +=============================+  <-- 2^n offset from Base of allocated buffer
  //  |   Padding                   |
  //  +-----------------------------+   
  //  |   CPU 0 SMI Entry           |
  //  +#############################+  <-- Base of allocated buffer == CPU 0 SMBASE + 32 KB
  //

  //
  // Compute tile size of buffer required to hold CPU save state, any extra CPU 
  // specific context, and the SMI entry point.  This size of rounded up to 
  // nearest power of 2.
  //
  TileSize = 2 * GetPowerOfTwo32 (sizeof (SOCKET_LGA_775_SMM_CPU_STATE) + sizeof (PROCESSOR_SMM_DESCRIPTOR) + gcSmiHandlerSize - 1);

  //
  // Allocate buffer for all of the tiles. 
  //
  Buffer = AllocatePages (EFI_SIZE_TO_PAGES (SIZE_32KB + TileSize * (gSmmCpuPrivate->SmmCoreEntryContext.NumberOfCpus - 1)));
  ASSERT (Buffer != NULL);

  //
  // Retrieve APIC ID of each enabled processor from the MP Services protocol.
  // Also compute the SMBASE address, CPU Save State address, and CPU Save state 
  // size for each CPU in the platform
  //
  for (Index = 0; Index < gSmmCpuPrivate->SmmCoreEntryContext.NumberOfCpus; Index++) {
    Status = MpServices->GetProcessorInfo (MpServices, Index, &ProcessorInformation);
    ASSERT_EFI_ERROR (Status);
    gSmmCpuPrivate->ApicIds[Index]          = (UINT32)ProcessorInformation.ProcessorId;
    gSmmCpuPrivate->SmBases[Index]          = (UINTN)Buffer + Index * TileSize - SIZE_32KB;
    gSmmCpuPrivate->CpuSaveStateSize[Index] = sizeof(SOCKET_LGA_775_SMM_CPU_STATE);
    gSmmCpuPrivate->CpuSaveState[Index]     = (VOID *)(gSmmCpuPrivate->SmBases[Index] + SIZE_64KB - gSmmCpuPrivate->CpuSaveStateSize[Index]);

    DEBUG ((EFI_D_ERROR, "CPU[%03x]  APIC ID=%04x  SMBASE=%08x  SaveState=%08x  Size=%08x\n",
      Index, 
      gSmmCpuPrivate->ApicIds[Index], 
      gSmmCpuPrivate->SmBases[Index], 
      gSmmCpuPrivate->CpuSaveState[Index], 
      gSmmCpuPrivate->CpuSaveStateSize[Index]
      ));
  }

  //
  // Allocate SMI stacks for all processors.
  //
  if (FeaturePcdGet (PcdCpuSmmStackGuard)) {
    //
    // 2 more pages is allocated for each processor.
    // one is guard page and the other is known good stack.
    //
    // +-------------------------------------------+-----+-------------------------------------------+
    // | Known Good Stack | Guard Page | SMM Stack | ... | Known Good Stack | Guard Page | SMM Stack |
    // +-------------------------------------------+-----+-------------------------------------------+
    // |                                           |     |                                           | 
    // |<-------------- Processor 0 -------------->|     |<-------------- Processor n -------------->|
    //
    mSmmStackSize = EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES (PcdGet32 (PcdCpuSmmStackSize)) + 2);
    Stacks = (UINT8 *) AllocatePages (gSmmCpuPrivate->SmmCoreEntryContext.NumberOfCpus * (EFI_SIZE_TO_PAGES (PcdGet32 (PcdCpuSmmStackSize)) + 2));
    ASSERT (Stacks != NULL);
    mSmmStackArrayBase = (UINTN)Stacks;
    mSmmStackArrayEnd = mSmmStackArrayBase + gSmmCpuPrivate->SmmCoreEntryContext.NumberOfCpus * mSmmStackSize - 1;
  } else {
    mSmmStackSize = PcdGet32 (PcdCpuSmmStackSize);
    Stacks = (UINT8 *) AllocatePages (EFI_SIZE_TO_PAGES (gSmmCpuPrivate->SmmCoreEntryContext.NumberOfCpus * mSmmStackSize));
    ASSERT (Stacks != NULL);
  }

  //
  // Set SMI stack for SMM base relocation
  //
  gSmmInitStack = (UINTN) (Stacks + mSmmStackSize - sizeof (UINTN));

  //
  // Relocate SMM Base addresses to the ones allocated from SMRAM
  // BUGBUG: Work on later
  //
  SmmRelocateBases ();

  //
  // SMM Init
  // BUGBUG: Work on later
  //
  InitializeSmmTimer ();

  //
  // Initialize IDT
  // BUGBUG: Work on later
  //
  InitializeIDT ();

  //
  // Initialize MP globals
  //
  InitializeMpServiceData (ImageHandle, Stacks, mSmmStackSize);

  //
  // Fill in SMM Reserved Regions
  //
  gSmmCpuPrivate->SmmReservedSmramRegion[0].SmramReservedStart = 0;
  gSmmCpuPrivate->SmmReservedSmramRegion[0].SmramReservedSize  = 0;

  //
  // Install the SMM Configuration Protocol onto a new handle on the handle database.
  // The entire SMM Configuration Protocol is allocated from SMRAM, so only a pointer
  // to an SMRAM address will be present in the handle database 
  //
  Status = SystemTable->BootServices->InstallMultipleProtocolInterfaces (
                                        &gSmmCpuPrivate->SmmCpuHandle,
                                        &gEfiSmmConfigurationProtocolGuid, &gSmmCpuPrivate->SmmConfiguration,
                                        NULL
                                        );
  ASSERT_EFI_ERROR (Status);

  //
  // Install the SMM CPU Protocol into SMM protocol database
  //
  Status = gSmst->SmmInstallProtocolInterface (
                    &mSmmCpuHandle,
                    &gEfiSmmCpuProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &mSmmCpu
                    );
  ASSERT_EFI_ERROR (Status);

  if (FeaturePcdGet (PcdFrameworkCompatibilitySupport)) {
    //
    // Install Framework SMM Save State Protocol into UEFI protocol database for backward compatibility
    //
    Status = SystemTable->BootServices->InstallMultipleProtocolInterfaces (
                                          &gSmmCpuPrivate->SmmCpuHandle,
                                          &gEfiSmmCpuSaveStateProtocolGuid,
                                          &mSmmCpuSaveState,
                                          NULL
                                          );
    ASSERT_EFI_ERROR (Status);
    //
    // The SmmStartupThisAp service in Framework SMST should always be non-null.
    // Update SmmStartupThisAp pointer in PI SMST here so that PI/Framework SMM thunk
    // can have it ready when constructing Framework SMST.
    //
    gSmst->SmmStartupThisAp = SmmStartupThisAp;
  } else {
    //
    // For a native platform, register SMM Ready To Lock Protocol notification to
    // copy the CPU S3 data into SMRAM for security.
    //
    Status = gSmst->SmmRegisterProtocolNotify (
                      &gEfiSmmReadyToLockProtocolGuid,
                      SmmReadyToLockEventNotify,
                      &Registration
                      );
    ASSERT_EFI_ERROR (Status);
  }

  GuidHob = GetFirstGuidHob (&gEfiAcpiVariableGuid);
  if (GuidHob != NULL) {
    SmramDescriptor = (EFI_SMRAM_DESCRIPTOR *) GET_GUID_HOB_DATA (GuidHob);
    
    DEBUG ((EFI_D_ERROR, "SMM S3 SMRAM Structure = %x\n", SmramDescriptor));
    DEBUG ((EFI_D_ERROR, "SMM S3 Structure = %x\n", SmramDescriptor->CpuStart));
    
    SmmS3ResumeState = (SMM_S3_RESUME_STATE *)(UINTN)SmramDescriptor->CpuStart;
    ZeroMem (SmmS3ResumeState, sizeof (SMM_S3_RESUME_STATE));

    mSmmS3ResumeState = SmmS3ResumeState;
    SmmS3ResumeState->Smst = (EFI_PHYSICAL_ADDRESS)(UINTN)gSmst;
    
    SmmS3ResumeState->SmmS3ResumeEntryPoint = (EFI_PHYSICAL_ADDRESS)(UINTN)SmmRestoreCpu;
    
    SmmS3ResumeState->SmmS3StackSize = SIZE_32KB;
    SmmS3ResumeState->SmmS3StackBase = (EFI_PHYSICAL_ADDRESS)(UINTN)AllocatePages (EFI_SIZE_TO_PAGES ((UINTN)SmmS3ResumeState->SmmS3StackSize));
    if (SmmS3ResumeState->SmmS3StackBase == 0) {
      SmmS3ResumeState->SmmS3StackSize = 0;
    }
    
    SmmS3ResumeState->SmmS3Cr0 = gSmmCr0;
    SmmS3ResumeState->SmmS3Cr3 = gSmiCr3;
    SmmS3ResumeState->SmmS3Cr4 = gSmmCr4;
        
    if (sizeof (UINTN) == sizeof (UINT64)) {
      SmmS3ResumeState->Signature = SMM_S3_RESUME_SMM_64;
    }
    if (sizeof (UINTN) == sizeof (UINT32)) {
      SmmS3ResumeState->Signature = SMM_S3_RESUME_SMM_32;
    }
  }
  if (FeaturePcdGet (PcdCpuSmmProfileEnable)) {
    InitSmmProfile ();
  }
  DEBUG ((EFI_D_ERROR, "SMM CPU Module exit from SMRAM with EFI_SUCCESS\n"));

  return EFI_SUCCESS;
}

/**

  Find out SMRAM information including SMRR base, SMRR size and IEDBase.

  @param          SmrrBase          SMRR base
  @param          SmrrSize          SMRR size
  @param          IedBase           IED Base
  
**/
VOID
FindSmramInfo (
  OUT UINT32   *SmrrBase,
  OUT UINT32   *SmrrSize,
  OUT UINT32   *IedBase
  )
{
  EFI_STATUS                        Status;
  UINTN                             Size;
  EFI_SMM_ACCESS2_PROTOCOL          *SmmAccess;
  EFI_SMRAM_DESCRIPTOR              *CurrentSmramRange;
  EFI_SMRAM_DESCRIPTOR              *SmramRanges;
  UINTN                             SmramRangeCount;
  UINTN                             Index;
  UINT64                            MaxSize;
  BOOLEAN                           Found;

  //
  // Get SMM Access Protocol
  //
  Status = gBS->LocateProtocol (&gEfiSmmAccess2ProtocolGuid, NULL, (VOID **)&SmmAccess);
  ASSERT_EFI_ERROR (Status);

  //
  // Get SMRAM information
  //
  Size = 0;
  Status = SmmAccess->GetCapabilities (SmmAccess, &Size, NULL);
  ASSERT (Status == EFI_BUFFER_TOO_SMALL);

  SmramRanges = (EFI_SMRAM_DESCRIPTOR *)AllocatePool (Size);
  ASSERT (SmramRanges != NULL);

  Status = SmmAccess->GetCapabilities (SmmAccess, &Size, SmramRanges);
  ASSERT_EFI_ERROR (Status);

  SmramRangeCount = Size / sizeof (EFI_SMRAM_DESCRIPTOR);

  //
  // Find the largest SMRAM range between 1MB and 4GB that is at least 256K - 4K in size
  //
  CurrentSmramRange = NULL;
  for (Index = 0, MaxSize = SIZE_256KB - EFI_PAGE_SIZE; Index < SmramRangeCount; Index++) {
    //
    // Skip any SMRAM region that is already allocated, needs testing, or needs ECC initialization
    //
    if ((SmramRanges[Index].RegionState & (EFI_ALLOCATED | EFI_NEEDS_TESTING | EFI_NEEDS_ECC_INITIALIZATION)) != 0) {
      continue;
    }

    if (SmramRanges[Index].CpuStart >= BASE_1MB) {
      if ((SmramRanges[Index].CpuStart + SmramRanges[Index].PhysicalSize) <= BASE_4GB) {
        if (SmramRanges[Index].PhysicalSize >= MaxSize) {
          MaxSize = SmramRanges[Index].PhysicalSize;
          CurrentSmramRange = &SmramRanges[Index];
        }
      }
    }
  }

  ASSERT (CurrentSmramRange != NULL);

  *SmrrBase = (UINT32)CurrentSmramRange->CpuStart;
  *SmrrSize = (UINT32)CurrentSmramRange->PhysicalSize;

  do {
    Found = FALSE;
    for (Index = 0; Index < SmramRangeCount; Index++) {
      if (SmramRanges[Index].CpuStart < *SmrrBase && *SmrrBase == (SmramRanges[Index].CpuStart + SmramRanges[Index].PhysicalSize)) {
        *SmrrBase = (UINT32)SmramRanges[Index].CpuStart;
        *SmrrSize = (UINT32)(*SmrrSize + SmramRanges[Index].PhysicalSize);
        Found = TRUE;
      } else if ((*SmrrBase + *SmrrSize) == SmramRanges[Index].CpuStart && SmramRanges[Index].PhysicalSize > 0) {
        *SmrrSize = (UINT32)(*SmrrSize + SmramRanges[Index].PhysicalSize);
        Found = TRUE;
      }
    }
  } while (Found);

  //
  // if IED is enabled, IEDRAM won't be reported within TSEG.
  // IED is located at the top of TSEG.
  //
  if (PcdGetBool (PcdCpuIEDEnabled)) {
    *IedBase = *SmrrBase + *SmrrSize;
    DEBUG ((EFI_D_INFO, "IEDRAM Base: 0x%x\n", *IedBase));

    *SmrrSize += PcdGet32 (PcdCpuIEDRamSize);
  } else {
    *IedBase = (UINT32) -1;
  }

  DEBUG ((EFI_D_INFO, "SMRR Base: 0x%x, SMRR Size: 0x%x\n", *SmrrBase, *SmrrSize));
}

