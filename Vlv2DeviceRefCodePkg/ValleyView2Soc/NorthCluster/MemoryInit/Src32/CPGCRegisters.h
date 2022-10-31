#ifndef _CPGC_H_
#define _CPGC_H_

#define CPGC_CMD_CAP_REG 0x10

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Command Capability Register
 */
typedef union {
  struct {
    /* mclk_2_dclk_cap - Bits[1:0], RO, default = 0b 
       The ratio of the MCLK to the DCLK is encoded as follows:
     */
    UINT32 mclk_2_dclk_cap : 2;

    /* rsvd_4 - Bits[3:2], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_4 : 2;

    /* norm_mode_cap - Bits[4:4], RO, default = 1b 
       Set if functional command/address pattern generation mode (also known as normal 
       mode) is supported. 
     */
    UINT32 norm_mode_cap : 1;

    /* dir_mode_cap - Bits[5:5], RO, default = 1b 
       Set if direct data mode for compliance master, compliance slave, and loopback, 
       data functionality is supported. 
     */
    UINT32 dir_mode_cap : 1;

    /* mrs_mode_cap - Bits[6:6], RO, default = 0b 
       Set if MRS mode functionality is supported.
     */
    UINT32 mrs_mode_cap : 1;

    /* rsvd_3 - Bits[7:7], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_3 : 1;

    /* fix_addr_width_cap - Bits[12:8], RO, default = 1Fh 
       This field indicates the width of the fixed address generation segment (and 
       hence the channel address width). 
     */
    UINT32 fix_addr_width_cap : 5;

    /* rsvd_2 - Bits[15:13], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_2 : 3;

    /* ref_blk_cap - Bits[16:16], RO, default = 1b 
       Set if DRAM refresh blocking is supported.
     */
    UINT32 ref_blk_cap : 1;

    /* rsvd_1 - Bits[17:17], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_1 : 1;

    /* mult_eng_syn_cap - Bits[18:18], RO, default = 1b 
       This bit indicates if phase synchronization for a multi-channel/multi-engine 
       implementation is supported through the START_DELAY, GLB_START_ON_START, and 
       GBL_STOP_ON_STOP capabilities. 
     */
    UINT32 mult_eng_syn_cap : 1;

    /* rsvd_0 - Bits[31:19], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_0 : 13;

  } Bits;
  UINT32 Data;
} CPGC_CMD_CAP_STRUCT;
#endif /* ASM_INC */

#define CPGC_CMD_CTL_REG 0x11

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Command Control Register
 */
typedef union {
  struct {
    /* start_delay - Bits[7:0], RW, default = 0h 
       Number of clock cycles (in CPGC-S clock domain) the start of the test is delayed 
       by after START_TEST has been asserted by the user.  This is usually used to 
       synchronize multiple SoC CPGC engines on multiple channels according to a 
       defined phase relationship.  Note: this field is only available if multiple 
       channels/engines are supported for the current implementation, otherwise it is 
       reserved. 
     */
    UINT32 start_delay : 8;

    /* start_test - Bits[8:8], RW1S, default = 0h 
       Used to initiate a transition to active mode on this engine/channel (note that 
       INIT_MODE has to be programmed first).  This bit will always return '0' if read 
       by software.  Note that this will also cause a test start for all engines with 
       GLB_START_ON_START set. 
     */
    UINT32 start_test : 1;

    /* stop_test - Bits[9:9], RW1S, default = 0h 
       Forces an exit from the tests running on this engine/channel.  This bit will 
       always return '0' if read by software.  Note that this will also cause a test 
       stop for all engines with GLB_STOP_ON_STOP set. 
     */
    UINT32 stop_test : 1;

    /* glb_start_on_start - Bits[10:10], RW, default = 0h 
       Setting this bit will bind this channel engine to all other channel engines with 
       this bit set. A test start for any channel with GLB_START_ON_START set will 
       cause the same action to occur on all engines with GLB_START_ON_STAR set.  This 
       feature is usually used when synchronization between multiple engines/channels 
       necessitates a global control of all supported engines.  Note: this field is 
       only available if multiple channels/engines are supported for the current 
       implementation, otherwise it is reserved. 
     */
    UINT32 glb_start_on_start : 1;

    /* glb_stop_on_stop - Bits[11:11], RW, default = 0h 
       Setting this bit will bind this channel engine to all other channel engines with 
       this bit set.  A test stop (both forced and due to a stop condition) for any 
       channel engine with GLB_STOP_ON_STOP set will cause the same action to occur on 
       all engines with GLB_STOP_ON_STOP set.  This feature is usually used when 
       synchronization between multiple engines/channels necessitates a global control 
       of all supported engines. Note: this field is only available if multiple 
       channels/engines are supported for the current implementation, otherwise it is 
       reserved. 
     */
    UINT32 glb_stop_on_stop : 1;

    /* init_mode - Bits[13:12], RW, default = 0h 
       00 - IDLE MODE
     */
    UINT32 init_mode : 2;

    /* rsvd_8 - Bits[15:14], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_8 : 2;

    /* exp_loop_cnt - Bits[19:16], RW, default = 6h 
       Sets the number of the command sequence loops to issue to 2^(EXP_LOOP_CNT- 1) 
       before a test exits normally (if no other stop condition has been detected).  A 
       command sequence loop is defined as the completion of all subsequences between 
       SEQ_STRT_PTR to SEQ_END_PTR.  A value of 0 will force an infinite loop mode.  In 
       this case, only some stop condition can end the test or by the user directly 
       asserting the STOP_TEST bit. 
     */
    UINT32 exp_loop_cnt : 4;

    /* stop_on_wrap - Bits[20:20], RW, default = 0h 
       If any wrap trigger occurs and this bit is set, then a stop condition will be 
       generated immediately forcing the test to stop. 
     */
    UINT32 stop_on_wrap : 1;

    /* rsvd_7 - Bits[23:21], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_7 : 3;

    /* seq_strt_ptr - Bits[25:24], RW, default = 0h 
       Pointer to first subsequence in a command sequence loop.  A full command 
       sequence loop will start with the sequence pointed to by SEQ_STRT_PTR and end at 
       the sequence pointed to by SEQ_END_PTR before wrapping back to the SEQ_STRT_PTR. 
     */
    UINT32 seq_strt_ptr : 2;

    /* rsvd_6 - Bits[27:26], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_6 : 2;

    /* seq_end_ptr - Bits[29:28], RW, default = 0h 
       Pointer to last subsequence in a command sequence loop.  A full command sequence 
       loop will start with the subsequence pointed to by SEQ_STRT_PTR and end at the 
       subsequence pointed to by SEQ_END_PTR before wrapping back to the SEQ_STRT_PTR. 
     */
    UINT32 seq_end_ptr : 2;

    /* rsvd_5 - Bits[31:30], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_5 : 2;

  } Bits;
  UINT32 Data;
} CPGC_CMD_CTL_STRUCT;
#endif /* ASM_INC */

#define CPGC_CMD_TESTSTAT_REG 0x12

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Command Sequence Test Status Register
 */
typedef union {
  struct {
    /* loop_cnt - Bits[15:0], RO, default = 0h 
       Current number of command sequence loops that have been executed .
     */
    UINT32 loop_cnt : 16;

    /* rsvd_10 - Bits[23:16], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_10 : 8;

    /* subseq_ptr - Bits[25:24], RO, default = 0h 
       Indicates the current subsequence being executed within the command sequence.
     */
    UINT32 subseq_ptr : 2;

    /* rsvd_9 - Bits[29:26], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_9 : 4;

    /* test_busy - Bits[30:30], RO, default = 0h 
       This bit will be set when once a test has started.  Bit is cleared on a reset or 
       once test is done (or has been forced to exit due a stop condition). 
     */
    UINT32 test_busy : 1;

    /* test_done - Bits[31:31], RO, default = 0h 
       This bit will be set when the test is complete (or has been forced to exit due 
       to a stop condition). But is cleared on a reset or when user starts another 
       test. 
     */
    UINT32 test_done : 1;

  } Bits;
  UINT32 Data;
} CPGC_CMD_TESTSTAT_STRUCT;
#endif /* ASM_INC */

#define CPGC_CMD_BURSTCNTR_REG 0x13

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Command Subsequence Burst Counter Register
 */
typedef union {
  struct {
    /* burst_cnt - Bits[31:0], RO, default = 0h 
       Number of bursts (CAS commands) that have been executed completely for the 
       current subsequence pointed to by SUBSEQ_PTR. 
     */
    UINT32 burst_cnt : 32;

  } Bits;
  UINT32 Data;
} CPGC_CMD_BURSTCNTR_STRUCT;
#endif /* ASM_INC */

#define CPGC_CMD_SEQ0CTL_REG 0x20

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Subsequence 0 Control Register
 */
typedef union {
  struct {
    /* num_bursts - Bits[3:0], RW-D, default = 1h 
       Number of bursts (CAS commands) composing the subsequence is equal to 
       2^(NUM_BURSTS - 1).  A value of 0 stands for infinite number of bursts and 
       enables the Infinite Burst Addressing mode. 
     */
    UINT32 num_bursts : 4;

    /* comp_on_wrap - Bits[4:4], RW-D, default = 0h 
       If the subsequence is running in the infinite burst mode, setting this bit will 
       cause a transition from this subsequence to the next subsequence (after the wait 
       period) once the channel address hits the wrap address defined by WRAP_ADDR - 1. 
       This bit is not used for finite burst addressing mode. 
     */
    UINT32 comp_on_wrap : 1;

    /* subseq_wait_ref_blk - Bits[5:5], RW-D, default = 0h 
       If set, refresh commands will be blocked for the duration of the wait period 
       specified by SUBSEQ_WAIT.  Otherwise refreshes will be enabled for that period 
       of time. 
     */
    UINT32 subseq_wait_ref_blk : 1;

    /* rsvd_14 - Bits[7:6], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_14 : 2;

    /* subseq_wait - Bits[15:8], RW-D, default = 0h 
       Number of clock cycles (in CPGC-S clock domain) between finishing a subsequence 
       and the beginning of the next subsequence is defined by SUBSEQ_WAIT.  The wait 
       period is considered independent from the subsequence itself and hence refresh 
       commands can be Enabled/Disabled for the wait period independent of the 
       preceding subsequence. 
     */
    UINT32 subseq_wait : 8;

    /* req_data_size - Bits[16:16], RW-D, default = 0h 
       Size of requests CPGC is issuing, 64B and 32B transactions and is encoded as 
       follows: 
     */
    UINT32 req_data_size : 1;

    /* rsvd_13 - Bits[19:17], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_13 : 3;

    /* subseq_type - Bits[23:20], RV, default = 0h 
       This field defines the type of commands this sequence will issue:
     */
    UINT32 subseq_type : 4;

    /* reset_addr - Bits[24:24], RW-D, default = 0h 
       Setting this bit will force the subsequence to reload its initial address from 
       each time before it starts.  This bit is not applicable to Infinite Burst 
       Addressing mode. 
     */
    UINT32 reset_addr : 1;

    /* rsvd_12 - Bits[31:25], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_12 : 7;

  } Bits;
  UINT32 Data;
} CPGC_CMD_SEQ0CTL_STRUCT;
#endif /* ASM_INC */

#define CPGC_CMD_SEQ0VAR_REG 0x21

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Subsequence 0 Variable Address Register or Start Address
 */
typedef union {
  struct {
    /* start_addr - Bits[30:0], RW-D, default = 0h 
       If the subsequence is running in the Infinite Burst Addressing mode, this field 
       defines the starting address for the subsequence.  This address is used along 
       with the WRAP_ADDR to control the test execution.  Note:  The width of this 
       field is dependent on the channel address width and maybe different for 
       different implementations. 
     */
    UINT32 start_addr : 31;

    /* rsvd_15 - Bits[31:31], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_15 : 1;

  } Bits;
  struct {
    /* Variable Segment Scrambling (VAR_SCRAMBLE) Bits[0:0], RW-D, default = 0h
       0 – Linear Variable Address Segment
       1 – Scrambled Variable Address Segment
    */
    UINT32 var_scramble : 1;
    /* Variable Segment (VAR_SEGMENT) Bits[15:1], RW-D, default = 0h
       This field is used to program the VAR address segment for the
       command sequence
    */
    UINT32 var_segment : 15;
    /* Variable Segment Width (VAR_WIDTH) Bits[19:16], RW-D, default = 0h
       The Width of the VAR address segment for the sequence will be set
       to the following number of bits:
       0000 – 5-bits
       0001 – 6-bits
       0010 – 7-bits
       0011 – 8-bits
       0100 – 9-bits
       0101 – 10-bits
       0110 – 11-bits
       0111 – 12-bits
       1000 – 13-bits
       1001 – 14-bits
       1010 – 15-bits
       All other values are reserved.
    */
    UINT32 var_width : 4;
    /* Variable Segment Low Shift (VAR_LO_SHIFT) Bits[24:20], RW-D, default = 0h
       The lower 3-bits of the VAR address segment will be shifted to the
       left by VAR_LO_SHIFT bits
    */
    UINT32 var_lo_shift : 5;
    /*
    Variable Segment High Shift (VAR_HI_SHIFT) Bits[29:25], RW-D, default = 0h
    The upper (VAR_WIDTH – 3)-bits of the VAR address segment will be     
    shifted to the left by VAR_HI_SHIFT bits 
    */
    UINT32 var_hi_shift : 5;
    /* rsvd - Bits[31:30], RV, default = 2h 
       Reserved
     */
    UINT32 rsvd : 2;

  } Bits2;
  UINT32 Data;
} CPGC_CMD_SEQ0VAR_STRUCT;
#endif /* ASM_INC */

#define CPGC_CMD_SEQ0FIX_REG 0x22

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Subsequence 0 Fixed Address Register or Wrap Address Register
 */
typedef union {
  struct {
    /* wrap_addr - Bits[30:0], RW-D, default = 0h 
       If the subsequence is running in the Infinite Burst Addressing mode, WRAP_ADDR - 
       1 defines the top value for the channel address.  Once hit, a wrap trigger is 
       issued and used with COMP_ON_WRAP and/or STOP_ON_WRAP to control the test 
       execution.  Note:  The width of this field is dependent on the channel address 
       width and maybe different for different implementations. 
     */
    UINT32 wrap_addr : 31;

    /* rsvd_16 - Bits[31:31], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_16 : 1;

  } Bits;
  struct {
    /* Fixed Address Segment (FIX_SEGMENT) - Bits[30:0], RW-D, default = 0h 
       This field is used to program the FIX address segment for this
       subsequence. Note: The width of this field is dependent on the
       channel address width and maybe different for different
       implementations.
     */
    UINT32 fix_segment : 31;

    /* rsvd_16 - Bits[31:31], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_16 : 1;

  } Bits2;
  UINT32 Data;
} CPGC_CMD_SEQ0FIX_STRUCT;
#endif /* ASM_INC */

#define CPGC_CMD_SEQ1CTL_REG 0x23

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Subsequence 1 Control Register
 */
typedef union {
  struct {
    /* num_bursts - Bits[3:0], RW-D, default = 1h 
       Number of bursts (CAS commands) composing the subsequence is equal to 
       2^(NUM_BURSTS - 1).  A value of 0 stands for infinite number of bursts and 
       enables the Infinite Burst Addressing mode. 
     */
    UINT32 num_bursts : 4;

    /* comp_on_wrap - Bits[4:4], RW-D, default = 0h 
       If the subsequence is running in the infinite burst mode, setting this bit will 
       cause a transition from this subsequence to the next subsequence (after the wait 
       period) once the channel address hits the wrap address defined by WRAP_ADDR - 1. 
       This bit is not used for finite burst addressing mode. 
     */
    UINT32 comp_on_wrap : 1;

    /* subseq_wait_ref_blk - Bits[5:5], RW-D, default = 0h 
       If set, refresh commands will be blocked for the duration of the wait period 
       specified by SUBSEQ_WAIT.  Otherwise refreshes will be enabled for that period 
       of time. 
     */
    UINT32 subseq_wait_ref_blk : 1;

    /* rsvd_19 - Bits[7:6], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_19 : 2;

    /* subseq_wait - Bits[15:8], RW-D, default = 0h 
       Number of clock cycles (in CPGC-S clock domain) between finishing a subsequence 
       and the beginning of the next subsequence is defined by SUBSEQ_WAIT.  The wait 
       period is considered independent from the subsequence itself and hence refresh 
       commands can be Enabled/Disabled for the wait period independent of the 
       preceding subsequence. 
     */
    UINT32 subseq_wait : 8;

    /* req_data_size - Bits[16:16], RW-D, default = 0h 
       Size of requests CPGC is issuing, 64B and 32B transactions and is encoded as 
       follows: 
     */
    UINT32 req_data_size : 1;

    /* rsvd_18 - Bits[19:17], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_18 : 3;

    /* subseq_type - Bits[23:20], RV, default = 0h 
       This field defines the type of commands this sequence will issue:
     */
    UINT32 subseq_type : 4;

    /* reset_addr - Bits[24:24], RW-D, default = 0h 
       Setting this bit will force the subsequence to reload its initial address from 
       each time before it starts.  This bit is not applicable to Infinite Burst 
       Addressing mode. 
     */
    UINT32 reset_addr : 1;

    /* rsvd_17 - Bits[31:25], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_17 : 7;

  } Bits;
  UINT32 Data;
} CPGC_CMD_SEQ1CTL_STRUCT;
#endif /* ASM_INC */

#define CPGC_CMD_SEQ1VAR_REG 0x24

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Subsequence 1 Variable Address Register or Start Address
 */
typedef union {
  struct {
    /* start_addr - Bits[30:0], RW-D, default = 0h 
       If the subsequence is running in the Infinite Burst Addressing mode, this field 
       defines the starting address for the subsequence.  This address is used along 
       with the WRAP_ADDR to control the test execution.  Note:  The width of this 
       field is dependent on the channel address width and maybe different for 
       different implementations. 
     */
    UINT32 start_addr : 31;

    /* rsvd_20 - Bits[31:31], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_20 : 1;

  } Bits;
  struct {
    /* Variable Segment Scrambling (VAR_SCRAMBLE) Bits[0:0], RW-D, default = 0h
       0 – Linear Variable Address Segment
       1 – Scrambled Variable Address Segment
    */
    UINT32 var_scramble : 1;
    /* Variable Segment (VAR_SEGMENT) Bits[15:1], RW-D, default = 0h
       This field is used to program the VAR address segment for the
       command sequence
    */
    UINT32 var_segment : 15;
    /* Variable Segment Width (VAR_WIDTH) Bits[19:16], RW-D, default = 0h
       The Width of the VAR address segment for the sequence will be set
       to the following number of bits:
       0000 – 5-bits
       0001 – 6-bits
       0010 – 7-bits
       0011 – 8-bits
       0100 – 9-bits
       0101 – 10-bits
       0110 – 11-bits
       0111 – 12-bits
       1000 – 13-bits
       1001 – 14-bits
       1010 – 15-bits
       All other values are reserved.
    */
    UINT32 var_width : 4;
    /* Variable Segment Low Shift (VAR_LO_SHIFT) Bits[24:20], RW-D, default = 0h
       The lower 3-bits of the VAR address segment will be shifted to the
       left by VAR_LO_SHIFT bits
    */
    UINT32 var_lo_shift : 5;
    /*
    Variable Segment High Shift (VAR_HI_SHIFT) Bits[29:25], RW-D, default = 0h
    The upper (VAR_WIDTH – 3)-bits of the VAR address segment will be     
    shifted to the left by VAR_HI_SHIFT bits 
    */
    UINT32 var_hi_shift : 5;
    /* rsvd - Bits[31:30], RV, default = 2h 
       Reserved
     */
    UINT32 rsvd : 2;

  } Bits2;
  UINT32 Data;
} CPGC_CMD_SEQ1VAR_STRUCT;
#endif /* ASM_INC */

#define CPGC_CMD_SEQ1FIX_REG 0x25

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Subsequence 1 Fixed Address Register or Wrap Address Register
 */
typedef union {
  struct {
    /* wrap_addr - Bits[30:0], RW-D, default = 0h 
       If the subsequence is running in the Infinite Burst Addressing mode, WRAP_ADDR - 
       1 defines the top value for the channel address.  Once hit, a wrap trigger is 
       issued and used with COMP_ON_WRAP and/or STOP_ON_WRAP to control the test 
       execution.  Note:  The width of this field is dependent on the channel address 
       width and maybe different for different implementations. 
     */
    UINT32 wrap_addr : 31;

    /* rsvd_21 - Bits[31:31], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_21 : 1;

  } Bits;
  struct {
    /* Fixed Address Segment (FIX_SEGMENT) - Bits[30:0], RW-D, default = 0h 
       This field is used to program the FIX address segment for this
       subsequence. Note: The width of this field is dependent on the
       channel address width and maybe different for different
       implementations.
     */
    UINT32 fix_segment : 31;

    /* rsvd_16 - Bits[31:31], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_16 : 1;

  } Bits2;
  UINT32 Data;
} CPGC_CMD_SEQ1FIX_STRUCT;
#endif /* ASM_INC */

#define CPGC_CMD_SEQ2CTL_REG 0x26

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Subsequence 2 Control Register
 */
typedef union {
  struct {
    /* num_bursts - Bits[3:0], RW-D, default = 1h 
       Number of bursts (CAS commands) composing the subsequence is equal to 
       2^(NUM_BURSTS - 1).  A value of 0 stands for infinite number of bursts and 
       enables the Infinite Burst Addressing mode. 
     */
    UINT32 num_bursts : 4;

    /* comp_on_wrap - Bits[4:4], RW-D, default = 0h 
       If the subsequence is running in the infinite burst mode, setting this bit will 
       cause a transition from this subsequence to the next subsequence (after the wait 
       period) once the channel address hits the wrap address defined by WRAP_ADDR - 1. 
       This bit is not used for finite burst addressing mode. 
     */
    UINT32 comp_on_wrap : 1;

    /* subseq_wait_ref_blk - Bits[5:5], RW-D, default = 0h 
       If set, refresh commands will be blocked for the duration of the wait period 
       specified by SUBSEQ_WAIT.  Otherwise refreshes will be enabled for that period 
       of time. 
     */
    UINT32 subseq_wait_ref_blk : 1;

    /* rsvd_24 - Bits[7:6], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_24 : 2;

    /* subseq_wait - Bits[15:8], RW-D, default = 0h 
       Number of clock cycles (in CPGC-S clock domain) between finishing a subsequence 
       and the beginning of the next subsequence is defined by SUBSEQ_WAIT.  The wait 
       period is considered independent from the subsequence itself and hence refresh 
       commands can be Enabled/Disabled for the wait period independent of the 
       preceding subsequence. 
     */
    UINT32 subseq_wait : 8;

    /* req_data_size - Bits[16:16], RW-D, default = 0h 
       Size of requests CPGC is issuing, 64B and 32B transactions and is encoded as 
       follows: 
     */
    UINT32 req_data_size : 1;

    /* rsvd_23 - Bits[19:17], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_23 : 3;

    /* subseq_type - Bits[23:20], RV, default = 0h 
       This field defines the type of commands this sequence will issue:
     */
    UINT32 subseq_type : 4;

    /* reset_addr - Bits[24:24], RW-D, default = 0h 
       Setting this bit will force the subsequence to reload its initial address from 
       each time before it starts.  This bit is not applicable to Infinite Burst 
       Addressing mode. 
     */
    UINT32 reset_addr : 1;

    /* rsvd_22 - Bits[31:25], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_22 : 7;

  } Bits;
  UINT32 Data;
} CPGC_CMD_SEQ2CTL_STRUCT;
#endif /* ASM_INC */

#define CPGC_CMD_SEQ2VAR_REG 0x27

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Subsequence 2 Variable Address Register or Start Address
 */
typedef union {
  struct {
    /* start_addr - Bits[30:0], RW-D, default = 0h 
       If the subsequence is running in the Infinite Burst Addressing mode, this field 
       defines the starting address for the subsequence.  This address is used along 
       with the WRAP_ADDR to control the test execution.  Note:  The width of this 
       field is dependent on the channel address width and maybe different for 
       different implementations. 
     */
    UINT32 start_addr : 31;

    /* rsvd_25 - Bits[31:31], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_25 : 1;

  } Bits;
  struct {
    /* Variable Segment Scrambling (VAR_SCRAMBLE) Bits[0:0], RW-D, default = 0h
       0 – Linear Variable Address Segment
       1 – Scrambled Variable Address Segment
    */
    UINT32 var_scramble : 1;
    /* Variable Segment (VAR_SEGMENT) Bits[15:1], RW-D, default = 0h
       This field is used to program the VAR address segment for the
       command sequence
    */
    UINT32 var_segment : 15;
    /* Variable Segment Width (VAR_WIDTH) Bits[19:16], RW-D, default = 0h
       The Width of the VAR address segment for the sequence will be set
       to the following number of bits:
       0000 – 5-bits
       0001 – 6-bits
       0010 – 7-bits
       0011 – 8-bits
       0100 – 9-bits
       0101 – 10-bits
       0110 – 11-bits
       0111 – 12-bits
       1000 – 13-bits
       1001 – 14-bits
       1010 – 15-bits
       All other values are reserved.
    */
    UINT32 var_width : 4;
    /* Variable Segment Low Shift (VAR_LO_SHIFT) Bits[24:20], RW-D, default = 0h
       The lower 3-bits of the VAR address segment will be shifted to the
       left by VAR_LO_SHIFT bits
    */
    UINT32 var_lo_shift : 5;
    /*
    Variable Segment High Shift (VAR_HI_SHIFT) Bits[29:25], RW-D, default = 0h
    The upper (VAR_WIDTH – 3)-bits of the VAR address segment will be     
    shifted to the left by VAR_HI_SHIFT bits 
    */
    UINT32 var_hi_shift : 5;
    /* rsvd - Bits[31:30], RV, default = 2h 
       Reserved
     */
    UINT32 rsvd : 2;

  } Bits2;
  UINT32 Data;
} CPGC_CMD_SEQ2VAR_STRUCT;
#endif /* ASM_INC */

#define CPGC_CMD_SEQ2FIX_REG 0x28

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Subsequence 2 Fixed Address Register or Wrap Address Register
 */
typedef union {
  struct {
    /* wrap_addr - Bits[30:0], RW-D, default = 0h 
       If the subsequence is running in the Infinite Burst Addressing mode, WRAP_ADDR - 
       1 defines the top value for the channel address.  Once hit, a wrap trigger is 
       issued and used with COMP_ON_WRAP and/or STOP_ON_WRAP to control the test 
       execution.  Note:  The width of this field is dependent on the channel address 
       width and maybe different for different implementations. 
     */
    UINT32 wrap_addr : 31;

    /* rsvd_26 - Bits[31:31], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_26 : 1;

  } Bits;
  struct {
    /* Fixed Address Segment (FIX_SEGMENT) - Bits[30:0], RW-D, default = 0h 
       This field is used to program the FIX address segment for this
       subsequence. Note: The width of this field is dependent on the
       channel address width and maybe different for different
       implementations.
     */
    UINT32 fix_segment : 31;

    /* rsvd_16 - Bits[31:31], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_16 : 1;

  } Bits2;
  UINT32 Data;
} CPGC_CMD_SEQ2FIX_STRUCT;
#endif /* ASM_INC */

#define CPGC_CMD_SEQ3CTL_REG 0x29

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Subsequence 3 Control Register
 */
typedef union {
  struct {
    /* num_bursts - Bits[3:0], RW-D, default = 1h 
       Number of bursts (CAS commands) composing the subsequence is equal to 
       2^(NUM_BURSTS - 1).  A value of 0 stands for infinite number of bursts and 
       enables the Infinite Burst Addressing mode. 
     */
    UINT32 num_bursts : 4;

    /* comp_on_wrap - Bits[4:4], RW-D, default = 0h 
       If the subsequence is running in the infinite burst mode, setting this bit will 
       cause a transition from this subsequence to the next subsequence (after the wait 
       period) once the channel address hits the wrap address defined by WRAP_ADDR - 1. 
       This bit is not used for finite burst addressing mode. 
     */
    UINT32 comp_on_wrap : 1;

    /* subseq_wait_ref_blk - Bits[5:5], RW-D, default = 0h 
       If set, refresh commands will be blocked for the duration of the wait period 
       specified by SUBSEQ_WAIT.  Otherwise refreshes will be enabled for that period 
       of time. 
     */
    UINT32 subseq_wait_ref_blk : 1;

    /* rsvd_29 - Bits[7:6], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_29 : 2;

    /* subseq_wait - Bits[15:8], RW-D, default = 0h 
       Number of clock cycles (in CPGC-S clock domain) between finishing a subsequence 
       and the beginning of the next subsequence is defined by SUBSEQ_WAIT.  The wait 
       period is considered independent from the subsequence itself and hence refresh 
       commands can be Enabled/Disabled for the wait period independent of the 
       preceding subsequence. 
     */
    UINT32 subseq_wait : 8;

    /* req_data_size - Bits[16:16], RW-D, default = 0h 
       Size of requests CPGC is issuing, 64B and 32B transactions and is encoded as 
       follows: 
     */
    UINT32 req_data_size : 1;

    /* rsvd_28 - Bits[19:17], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_28 : 3;

    /* subseq_type - Bits[23:20], RV, default = 0h 
       This field defines the type of commands this sequence will issue:
     */
    UINT32 subseq_type : 4;

    /* reset_addr - Bits[24:24], RW-D, default = 0h 
       Setting this bit will force the subsequence to reload its initial address from 
       each time before it starts.  This bit is not applicable to Infinite Burst 
       Addressing mode. 
     */
    UINT32 reset_addr : 1;

    /* rsvd_27 - Bits[31:25], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_27 : 7;

  } Bits;
  UINT32 Data;
} CPGC_CMD_SEQ3CTL_STRUCT;
#endif /* ASM_INC */

#define CPGC_CMD_SEQ3VAR_REG 0x2A

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Subsequence 3 Variable Address Register or Start Address
 */
typedef union {
  struct {
    /* start_addr - Bits[30:0], RW-D, default = 0h 
       If the subsequence is running in the Infinite Burst Addressing mode, this field 
       defines the starting address for the subsequence.  This address is used along 
       with the WRAP_ADDR to control the test execution.  Note:  The width of this 
       field is dependent on the channel address width and maybe different for 
       different implementations. 
     */
    UINT32 start_addr : 31;

    /* rsvd_30 - Bits[31:31], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_30 : 1;

  } Bits;
  struct {
    /* Variable Segment Scrambling (VAR_SCRAMBLE) Bits[0:0], RW-D, default = 0h
       0 – Linear Variable Address Segment
       1 – Scrambled Variable Address Segment
    */
    UINT32 var_scramble : 1;
    /* Variable Segment (VAR_SEGMENT) Bits[15:1], RW-D, default = 0h
       This field is used to program the VAR address segment for the
       command sequence
    */
    UINT32 var_segment : 15;
    /* Variable Segment Width (VAR_WIDTH) Bits[19:16], RW-D, default = 0h
       The Width of the VAR address segment for the sequence will be set
       to the following number of bits:
       0000 – 5-bits
       0001 – 6-bits
       0010 – 7-bits
       0011 – 8-bits
       0100 – 9-bits
       0101 – 10-bits
       0110 – 11-bits
       0111 – 12-bits
       1000 – 13-bits
       1001 – 14-bits
       1010 – 15-bits
       All other values are reserved.
    */
    UINT32 var_width : 4;
    /* Variable Segment Low Shift (VAR_LO_SHIFT) Bits[24:20], RW-D, default = 0h
       The lower 3-bits of the VAR address segment will be shifted to the
       left by VAR_LO_SHIFT bits
    */
    UINT32 var_lo_shift : 5;
    /*
    Variable Segment High Shift (VAR_HI_SHIFT) Bits[29:25], RW-D, default = 0h
    The upper (VAR_WIDTH – 3)-bits of the VAR address segment will be     
    shifted to the left by VAR_HI_SHIFT bits 
    */
    UINT32 var_hi_shift : 5;
    /* rsvd - Bits[31:30], RV, default = 2h 
       Reserved
     */
    UINT32 rsvd : 2;

  } Bits2;
  UINT32 Data;
} CPGC_CMD_SEQ3VAR_STRUCT;
#endif /* ASM_INC */

#define CPGC_CMD_SEQ3FIX_REG 0x2B

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Subsequence 3 Fixed Address Register or Wrap Address Register
 */
typedef union {
  struct {
    /* wrap_addr - Bits[30:0], RW-D, default = 0h 
       If the subsequence is running in the Infinite Burst Addressing mode, WRAP_ADDR - 
       1 defines the top value for the channel address.  Once hit, a wrap trigger is 
       issued and used with COMP_ON_WRAP and/or STOP_ON_WRAP to control the test 
       execution.  Note:  The width of this field is dependent on the channel address 
       width and maybe different for different implementations. 
     */
    UINT32 wrap_addr : 31;

    /* rsvd_31 - Bits[31:31], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_31 : 1;

  } Bits;
  struct {
    /* Fixed Address Segment (FIX_SEGMENT) - Bits[30:0], RW-D, default = 0h 
       This field is used to program the FIX address segment for this
       subsequence. Note: The width of this field is dependent on the
       channel address width and maybe different for different
       implementations.
     */
    UINT32 fix_segment : 31;

    /* rsvd_16 - Bits[31:31], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_16 : 1;

  } Bits2;
  UINT32 Data;
} CPGC_CMD_SEQ3FIX_STRUCT;
#endif /* ASM_INC */

#define CPGC_CAPAT_CAP_REG 0x40

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Cmd/Address Pattern Capability Register
 */
typedef union {
  struct {
    /* num_uniseq_cap - Bits[1:0], RO, default = 1b 
       The number of unified sequencers supported for command/address pattern 
       generation is encoded as follows: 
     */
    UINT32 num_uniseq_cap : 2;

    /* rsvd_33 - Bits[2:2], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_33 : 1;

    /* uniseq_width_cap - Bits[3:3], RO, default = 0b 
       The Unified Sequencer Width supported for command/address pattern generation is 
       encoded as follows: 
     */
    UINT32 uniseq_width_cap : 1;

    /* rsvd_32 - Bits[31:4], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_32 : 28;

  } Bits;
  UINT32 Data;
} CPGC_CAPAT_CAP_STRUCT;
#endif /* ASM_INC */

#define CPGC_CAPAT_CTL_REG 0x41

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Cmd/Address Pattern Control Register
 */
typedef union {
  struct {
    /* uniseq0_mode - Bits[1:0], RW-D, default = 1h 
       Defines the operational mode for unified sequence 0 as follows:
     */
    UINT32 uniseq0_mode : 2;

    /* uniseq1_mode - Bits[3:2], RW-D, default = 1h 
       Defines the operational mode for unified sequence 1 as follows:
     */
    UINT32 uniseq1_mode : 2;

    /* rsvd_35 - Bits[7:4], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_35 : 4;

    /* capat_mode - Bits[9:8], RW-D, default = 0h 
       The operation mode of the command/address pattern generator is encoded as 
       follows: 
     */
    UINT32 capat_mode : 2;

    /* rsvd_34 - Bits[31:10], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_34 : 22;

  } Bits;
  UINT32 Data;
} CPGC_CAPAT_CTL_STRUCT;
#endif /* ASM_INC */

#define CPGC_CAPAT_UNISEQ0_REG 0x42

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Cmd/Address Pattern Unified Sequencer 0 Register (Shared, depends on C-Uniseq0 
 * Mode) 
 */
typedef union {
  struct {
    /* pat_buf - Bits[15:0], RW-D, default = AA55h 
       The initial content of the rotating pattern buffer for the unified sequencer.
     */
    UINT32 pat_buf : 16;

    /* rsvd_36 - Bits[31:16], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_36 : 16;

  } Bits;
  UINT32 Data;
} CPGC_CAPAT_UNISEQ0_STRUCT;
#endif /* ASM_INC */

#define CPGC_CAPAT_UNISEQ1_REG 0x43

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Cmd/Address Pattern Unified Sequencer 1 Register (Shared, depends on C-Uniseq1 
 * Mode) 
 */
typedef union {
  struct {
    /* pat_buf - Bits[15:0], RW-D, default = AA55h 
       The initial content of the rotating pattern buffer for the unified sequencer.
     */
    UINT32 pat_buf : 16;

    /* rsvd_37 - Bits[31:16], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_37 : 16;

  } Bits;
  UINT32 Data;
} CPGC_CAPAT_UNISEQ1_STRUCT;
#endif /* ASM_INC */

#define CPGC_CAPAT_BUFA0_REG 0x44

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Cmd/Address Pattern Unified Buffer A0 Register
 */
typedef union {
  struct {
    /* row_col_addr - Bits[15:0], RW-D, default = 0h 
       The DRAM row/column address.
     */
    UINT32 row_col_addr : 16;

    /* rsvd_40 - Bits[19:16], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_40 : 4;

    /* bank_addr - Bits[22:20], RW-D, default = 0h 
       The DRAM bank address.
     */
    UINT32 bank_addr : 3;

    /* rsvd_39 - Bits[23:23], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_39 : 1;

    /* cmd - Bits[26:24], RW-D, default = 7h 
       The DRAM command signals, [RAS#,CAS#,WE#].  All three signals are active low and 
       hence default to '1's on reset. 
     */
    UINT32 cmd : 3;

    /* rsvd_38 - Bits[31:27], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_38 : 5;

  } Bits;
  UINT32 Data;
} CPGC_CAPAT_BUFA0_STRUCT;
#endif /* ASM_INC */

#define CPGC_CAPAT_BUFB0_REG 0x45

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Cmd/Address Pattern Unified Buffer B0 Register
 */
typedef union {
  struct {
    /* rsvd_44 - Bits[7:0], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_44 : 8;

    /* odt - Bits[11:8], RW-D, default = 0h 
       The DRAM On-Die Termination signals.
     */
    UINT32 odt : 4;

    /* rsvd_43 - Bits[15:12], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_43 : 4;

    /* cke - Bits[19:16], RW-D, default = Fh 
       The DRAM Clock Enable signals.
     */
    UINT32 cke : 4;

    /* rsvd_42 - Bits[23:20], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_42 : 4;

    /* cs - Bits[27:24], RW-D, default = 0h 
       The DRAM Chip Select Signals.
     */
    UINT32 cs : 4;

    /* rsvd_41 - Bits[31:28], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_41 : 4;

  } Bits;
  UINT32 Data;
} CPGC_CAPAT_BUFB0_STRUCT;
#endif /* ASM_INC */

#define CPGC_CAPAT_BUFA1_REG 0x46

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Cmd/Address Pattern Unified Buffer A1 Register
 */
typedef union {
  struct {
    /* row_col_addr - Bits[15:0], RW-D, default = 0h 
       The DRAM row/column address.
     */
    UINT32 row_col_addr : 16;

    /* rsvd_47 - Bits[19:16], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_47 : 4;

    /* bank_addr - Bits[22:20], RW-D, default = 0h 
       The DRAM bank address.
     */
    UINT32 bank_addr : 3;

    /* rsvd_46 - Bits[23:23], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_46 : 1;

    /* cmd - Bits[26:24], RW-D, default = 7h 
       The DRAM command signals, [RAS#,CAS#,WE#].  All three signals are active low and 
       hence default to '1's on reset. 
     */
    UINT32 cmd : 3;

    /* rsvd_45 - Bits[31:27], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_45 : 5;

  } Bits;
  UINT32 Data;
} CPGC_CAPAT_BUFA1_STRUCT;
#endif /* ASM_INC */

#define CPGC_CAPAT_BUFB1_REG 0x47

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Cmd/Address Pattern Unified Buffer B1 Register
 */
typedef union {
  struct {
    /* rsvd_51 - Bits[7:0], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_51 : 8;

    /* odt - Bits[11:8], RW-D, default = 0h 
       The DRAM On-Die Termination signals.
     */
    UINT32 odt : 4;

    /* rsvd_50 - Bits[15:12], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_50 : 4;

    /* cke - Bits[19:16], RW-D, default = Fh 
       The DRAM Clock Enable signals.
     */
    UINT32 cke : 4;

    /* rsvd_49 - Bits[23:20], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_49 : 4;

    /* cs - Bits[27:24], RW-D, default = 0h 
       The DRAM Chip Select Signals.
     */
    UINT32 cs : 4;

    /* rsvd_48 - Bits[31:28], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_48 : 4;

  } Bits;
  UINT32 Data;
} CPGC_CAPAT_BUFB1_STRUCT;
#endif /* ASM_INC */

#define CPGC_CAPAT_BUFA2_REG 0x48

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Cmd/Address Pattern Unified Buffer A2 Register
 */
typedef union {
  struct {
    /* row_col_addr - Bits[15:0], RW-D, default = 0h 
       The DRAM row/column address.
     */
    UINT32 row_col_addr : 16;

    /* rsvd_54 - Bits[19:16], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_54 : 4;

    /* bank_addr - Bits[22:20], RW-D, default = 0h 
       The DRAM bank address.
     */
    UINT32 bank_addr : 3;

    /* rsvd_53 - Bits[23:23], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_53 : 1;

    /* cmd - Bits[26:24], RW-D, default = 7h 
       The DRAM command signals, [RAS#,CAS#,WE#].  All three signals are active low and 
       hence default to '1's on reset. 
     */
    UINT32 cmd : 3;

    /* rsvd_52 - Bits[31:27], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_52 : 5;

  } Bits;
  UINT32 Data;
} CPGC_CAPAT_BUFA2_STRUCT;
#endif /* ASM_INC */

#define CPGC_CAPAT_BUFB2_REG 0x49

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Cmd/Address Pattern Unified Buffer B2 Register
 */
typedef union {
  struct {
    /* rsvd_58 - Bits[7:0], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_58 : 8;

    /* odt - Bits[11:8], RW-D, default = 0h 
       The DRAM On-Die Termination signals.
     */
    UINT32 odt : 4;

    /* rsvd_57 - Bits[15:12], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_57 : 4;

    /* cke - Bits[19:16], RW-D, default = Fh 
       The DRAM Clock Enable signals.
     */
    UINT32 cke : 4;

    /* rsvd_56 - Bits[23:20], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_56 : 4;

    /* cs - Bits[27:24], RW-D, default = 0h 
       The DRAM Chip Select Signals.
     */
    UINT32 cs : 4;

    /* rsvd_55 - Bits[31:28], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_55 : 4;

  } Bits;
  UINT32 Data;
} CPGC_CAPAT_BUFB2_STRUCT;
#endif /* ASM_INC */

#define CPGC_CAPAT_BUFA3_REG 0x4A

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Cmd/Address Pattern Unified Buffer A3 Register
 */
typedef union {
  struct {
    /* row_col_addr - Bits[15:0], RW-D, default = 0h 
       The DRAM row/column address.
     */
    UINT32 row_col_addr : 16;

    /* rsvd_61 - Bits[19:16], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_61 : 4;

    /* bank_addr - Bits[22:20], RW-D, default = 0h 
       The DRAM bank address.
     */
    UINT32 bank_addr : 3;

    /* rsvd_60 - Bits[23:23], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_60 : 1;

    /* cmd - Bits[26:24], RW-D, default = 7h 
       The DRAM command signals, [RAS#,CAS#,WE#].  All three signals are active low and 
       hence default to '1's on reset. 
     */
    UINT32 cmd : 3;

    /* rsvd_59 - Bits[31:27], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_59 : 5;

  } Bits;
  UINT32 Data;
} CPGC_CAPAT_BUFA3_STRUCT;
#endif /* ASM_INC */

#define CPGC_CAPAT_BUFB3_REG 0x4B

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Cmd/Address Pattern Unified Buffer B3 Register
 */
typedef union {
  struct {
    /* rsvd_65 - Bits[7:0], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_65 : 8;

    /* odt - Bits[11:8], RW-D, default = 0h 
       The DRAM On-Die Termination signals.
     */
    UINT32 odt : 4;

    /* rsvd_64 - Bits[15:12], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_64 : 4;

    /* cke - Bits[19:16], RW-D, default = Fh 
       The DRAM Clock Enable signals.
     */
    UINT32 cke : 4;

    /* rsvd_63 - Bits[23:20], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_63 : 4;

    /* cs - Bits[27:24], RW-D, default = 0h 
       The DRAM Chip Select Signals.
     */
    UINT32 cs : 4;

    /* rsvd_62 - Bits[31:28], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_62 : 4;

  } Bits;
  UINT32 Data;
} CPGC_CAPAT_BUFB3_STRUCT;
#endif /* ASM_INC */

#define CPGC_CAPAT_UNISEQ0STAT_REG 0x4C

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Cmd/Address Pattern Unified Sequence 0 Status Register
 */
typedef union {
  struct {
    /* uniseq_stat - Bits[15:0], RO-D, default = 0h 
       Current Unified Sequencer Status (UNISEQ_STAT):The current contents of the 
       command/address pattern unified sequencer buffer.  In case a stop condition 
       occurs, this register will contain the last value that was used to generate the 
       command/address pattern.  Bit 0 represents the value used for the current chunk. 
     */
    UINT32 uniseq_stat : 16;

    /* rsvd_66 - Bits[31:16], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_66 : 16;

  } Bits;
  UINT32 Data;
} CPGC_CAPAT_UNISEQ0STAT_STRUCT;
#endif /* ASM_INC */

#define CPGC_CAPAT_UNISEQ1STAT_REG 0x4D

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Cmd/Address Pattern Unified Sequence 1 Status Register
 */
typedef union {
  struct {
    /* uniseq_stat - Bits[15:0], RO-D, default = 0h 
       Current Unified Sequencer Status (UNISEQ_STAT):The current contents of the 
       command/address pattern unified sequencer buffer.  In case a stop condition 
       occurs, this register will contain the last value that was used to generate the 
       command/address pattern.  Bit 0 represents the value used for the current chunk. 
     */
    UINT32 uniseq_stat : 16;

    /* rsvd_67 - Bits[31:16], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_67 : 16;

  } Bits;
  UINT32 Data;
} CPGC_CAPAT_UNISEQ1STAT_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_CAP_REG 0x60

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern Capability Register
 */
typedef union {
  struct {
    /* data_bus_width_cap - Bits[1:0], RO, default = 11b 
       The number of data bus lanes (excluding any ECC lanes if supported) this 
       implementation supports is encoded as follows: 
     */
    UINT32 data_bus_width_cap : 2;

    /* ecc_width_cap - Bits[3:2], RO, default = 11b 
       The number of ECC lanes this implementation supports is encoded as follows:
     */
    UINT32 ecc_width_cap : 2;

    /* rsvd_72 - Bits[7:4], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_72 : 4;

    /* data_pat_width_cap - Bits[9:8], RO, default = 1b 
       The width of the pattern that can be generated by this implementation before any 
       replication is encoded in this field. 
     */
    UINT32 data_pat_width_cap : 2;

    /* rsvd_71 - Bits[11:10], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_71 : 2;

    /* data_pat_depth_cap - Bits[14:12], RO, default = 1b 
       The depth of the pattern that can be generated by this implementation is encoded 
       in this field. 
     */
    UINT32 data_pat_depth_cap : 3;

    /* rsvd_70 - Bits[15:15], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_70 : 1;

    /* mem_array_cap - Bits[16:16], RO, default = 0b 
       This bit will be set if the data pattern buffers are implemented using a memory 
       array (i.e. register file).  Otherwise extended buffers are used in a CBD 
       implementation. 
     */
    UINT32 mem_array_cap : 1;

    /* rsvd_69 - Bits[19:17], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_69 : 3;

    /* data_idc_mask_width_cap - Bits[21:20], RO, default = 11b 
       The width of the Data Inversion/DC mask supported by this implementation is 
       encoded as follows: 
     */
    UINT32 data_idc_mask_width_cap : 2;

    /* ecc_idc_mask_width_cap - Bits[23:22], RO, default = 10b 
       The width of the ECC Inversion/DC mask (if supported by this implementation) is 
       encoded as follows: 
     */
    UINT32 ecc_idc_mask_width_cap : 2;

    /* dram_data_mask_cap - Bits[24:24], RO, default = 1b 
       This bit is set if implementation supports DRAM Data Mask programming for data 
       lanes. 
     */
    UINT32 dram_data_mask_cap : 1;

    /* dram_ecc_mask_cap - Bits[25:25], RO, default = 1b 
       This bit is set if implementation supports DRAM Data Mask programming for ECC 
       lanes. 
     */
    UINT32 dram_ecc_mask_cap : 1;

    /* rsvd_68 - Bits[31:26], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_68 : 6;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_CAP_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_BUFCTL_REG 0x61

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern Buffer Control Register
 */
typedef union {
  struct {
    /* buf_ptr_inc_rate - Bits[2:0], RW-D, default = 0h 
       Each buffer entry will be used 2^BUF_PTR_INC_RATE time before incrementing the 
       buffer pointer by 1 if BUF_PTR_INC_EN is set. 
     */
    UINT32 buf_ptr_inc_rate : 3;

    /* buf_ptr_inc_en - Bits[3:3], RW-D, default = 0h 
       If set, the buffer pointer will be incremented at a rate given by 
       BUF_PTR_INC_RATE, otherwise only the buffer entry pointed to BUF_STRT_PTR will 
       be used for the entire test. 
     */
    UINT32 buf_ptr_inc_en : 1;

    /* rsvd_76 - Bits[7:4], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_76 : 4;

    /* buf_strt_ptr - Bits[11:8], RW-D, default = 0h 
       Pointer to first buffer entry.  Also used as the only entry if BUF_INC_EN is not 
       enabled. 
     */
    UINT32 buf_strt_ptr : 4;

    /* rsvd_75 - Bits[15:12], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_75 : 4;

    /* buf_end_ptr - Bits[19:16], RW-D, default = 0h 
       Pointer to last data pattern buffer entry before wrapping back to BUF_STRT_PTR 
       if incrementing is enabled through BUF_INC_EN. 
     */
    UINT32 buf_end_ptr : 4;

    /* rsvd_74 - Bits[22:20], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_74 : 3;

    /* subentry_wr_ptr - Bits[25:23], RW, default = 0h 
       Indicates which 32-bit/64-bit line within the data entry selected by 
       ENTRY_WR_PTR is written to when the user writes CPGC_DPAT_BA_LO and 
       CPGC_DPAT_BA_HI. 
     */
    UINT32 subentry_wr_ptr : 3;

    /* entry_wr_ptr - Bits[29:26], RW, default = 0h 
       Indicates which entry within the data pattern buffer will be written to when the 
       user writes CPGC_DPAT_BA_LO and CPGC_DPAT_BA_HI. 
     */
    UINT32 entry_wr_ptr : 4;

    /* rsvd_73 - Bits[31:30], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_73 : 2;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_BUFCTL_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_MUXCTL_REG 0x62

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern Mux Control Register
 */
typedef union {
  struct {
    /* uniseq0_mode - Bits[1:0], RW-D, default = 1h 
       Defines the operational mode for unified sequencer 0 as follows:
     */
    UINT32 uniseq0_mode : 2;

    /* uniseq1_mode - Bits[3:2], RW-D, default = 1h 
       Defines the operational mode for unified sequencre 1 as follows:
     */
    UINT32 uniseq1_mode : 2;

    /* uniseq2_mode - Bits[5:4], RW-D, default = 1h 
       Defines the operational mode for unified sequence 2 as follows:
     */
    UINT32 uniseq2_mode : 2;

    /* rsvd_78 - Bits[22:6], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_78 : 17;

    /* ecc_disable - Bits[23:23], RW-D, default = 0h 
       Setting this bit will internally disable all related ECC features in CPGC-S 
       having the effect of generating all 0 in the ECC CPGC-S interface outputs, and 
       of binding to 0 all the ECC interface CPGC-S inputs. ECC masks and error 
       comparisons will be disabled as well. Note: If ECC is not supported at compile 
       time, this bit will be reserved. 
     */
    UINT32 ecc_disable : 1;

    /* uniseq_reload_rate - Bits[26:24], RW-D, default = 0h 
       Defines the number of bursts (CAS commands) that will be issued before reloading 
       the unified sequencer with there starting values as 2^UNISEQ_RELOAD_RATE bursts 
       (CAS commands) 
     */
    UINT32 uniseq_reload_rate : 3;

    /* rsvd_77 - Bits[30:27], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_77 : 4;

    /* uniseq_reload_en - Bits[31:31], RW-D, default = 0h 
       If set, all data pattern unified sequencers will be reloaded with their initial 
       values at a rate defined by the UNISEQ_RELOAD_RATE 
     */
    UINT32 uniseq_reload_en : 1;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_MUXCTL_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_UNISEQ0_REG 0x63

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern Unified Sequencer 0 Register (Shared, depends on D-Uniseq0 Mode)
 */
typedef union {
  struct {
    /* pat_buf - Bits[31:0], RW-D, default = AA55AA55h 
       The initial content of the rotating pattern buffer for the unified sequencer.
     */
    UINT32 pat_buf : 32;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_UNISEQ0_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_UNISEQ1_REG 0x64

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern Unified Sequencer 1 Register (Shared, depends on D-Uniseq1 Mode)
 */
typedef union {
  struct {
    /* pat_buf - Bits[31:0], RW-D, default = AA55AA55h 
       The initial content of the rotating pattern buffer for the unified sequencer.
     */
    UINT32 pat_buf : 32;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_UNISEQ1_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_UNISEQ2_REG 0x65

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern Unified Sequencer 2 Register (Shared, depends on D-Uniseq2 Mode)
 */
typedef union {
  struct {
    /* pat_buf - Bits[31:0], RW-D, default = AA55AA55h 
       The initial content of the rotating pattern buffer for the unified sequencer.
     */
    UINT32 pat_buf : 32;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_UNISEQ2_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_INVDCCTL_REG 0x66

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern Inversion/DC Control Register
 */
typedef union {
  struct {
    /* ecc_inv_dc_mask - Bits[7:0], RW, default = 0h 
       A value of '1' for any of the bits means the corresponding ECC lane(s) will be 
       inverted or a DC value driven on it.  Note that this field is used to load bits 
       [71:64] of the continuous shift register composed of this field along with 
       DATA_INV_DC_MASK_HI and DATA_INV_DC_MASK_LO.   Note: this field is only 
       available if ECC is supported for the current implementation, otherwise it is 
       reserved. 
     */
    UINT32 ecc_inv_dc_mask : 8;

    /* rsvd_80 - Bits[22:8], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_80 : 15;

    /* ecc_indep_sr_mask - Bits[23:23], RW, default = 0h 
       This bit will split the continuous 72 bits shift register ( 64 data + 8 ECC ), 
       into two shift registers: One 8 bit shift register for ECCs and One 64 bit shift 
       register for Data. Note: this field is only available if ECC is supported for 
       the current implementation, otherwise it is reserved. 
     */
    UINT32 ecc_indep_sr_mask : 1;

    /* mask_rotate_rate - Bits[26:24], RW, default = 0h 
       If inversion mask rotation is enabled through INV_ROTATE_EN, the mask will 
       rotate to the left every time 2^INV_ROTATE_RATE bursts (CAS commands) have been 
       have been issued. 
     */
    UINT32 mask_rotate_rate : 3;

    /* rsvd_79 - Bits[28:27], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_79 : 2;

    /* dc_pol - Bits[29:29], RW, default = 0h 
       Selects the polarity of the signal to be driven through the inversion/DC mask if 
       DC_OR_INV is set to 1: 
     */
    UINT32 dc_pol : 1;

    /* dc_or_inv - Bits[30:30], RW, default = 0h 
       Selects between using the inversion/DC mask for inversion or DC as follows:
     */
    UINT32 dc_or_inv : 1;

    /* mask_rotate_en - Bits[31:31], RW, default = 0h 
       If set, the inversion/DC mask will rotate to the left at a rate defined by the 
       MASK_ROTATE_RATE field. 
     */
    UINT32 mask_rotate_en : 1;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_INVDCCTL_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_INVDC_MASK_LO_REG 0x67

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern Inversion/DC Low Mask
 */
typedef union {
  struct {
    /* data_inv_dc_mask_lo - Bits[31:0], RW-D, default = 0h 
       A value of '1' for any of the bits means the corresponding data lane(s) will be 
       inverted or a DC value driven on it. 
     */
    UINT32 data_inv_dc_mask_lo : 32;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_INVDC_MASK_LO_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_INVDC_MASK_HI_REG 0x68

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern Inversion/DC High Mask
 */
typedef union {
  struct {
    /* data_inv_dc_mask_hi - Bits[31:0], RW-D, default = 0h 
       A value of '1' for any of the bits means the corresponding data lane(s) will be 
       inverted or a DC value driven on it. 
     */
    UINT32 data_inv_dc_mask_hi : 32;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_INVDC_MASK_HI_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_DRAMDM_REG 0x69

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern DRAM Data Mask Register
 */
typedef union {
  struct {
    /* data_mask - Bits[31:0], RW-D, default = FFFFFFFFh 
       A one-hot field with each bit corresponding to a data byte sent during a burst 
       such that: 
     */
    UINT32 data_mask : 32;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_DRAMDM_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_XDRAMDM_REG 0x6A

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Data Pattern DRAM Data Mask Register
 */
typedef union {
  struct {
    /* ecc_mask - Bits[3:0], RW-D, default = Fh 
       A one-hot field with each bit corresponding to an ECC byte sent during a burst 
       such that: 
     */
    UINT32 ecc_mask : 4;

    /* rsvd_81 - Bits[31:4], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_81 : 28;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_XDRAMDM_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_BA_LO_REG 0x6B

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern Low Buffer Access Register
 */
typedef union {
  struct {
    /* data_line_lo - Bits[31:0], RW, default = 0h 
       Provides a write window into bits [31:0] of the data pattern memory array 
       indexed through the ENTRY_WR_PTR and SUBENTRY_WR_PTR. 
     */
    UINT32 data_line_lo : 32;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_BA_LO_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_BA_HI_REG 0x6C

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern High Buffer Access Register
 */
typedef union {
  struct {
    /* data_line_hi - Bits[31:0], RW, default = 0h 
       Provides a write window into bits [63:32] of the data pattern memory array 
       indexed through the ENTRY_WR_PTR and SUBENTRY_WR_PTR 
     */
    UINT32 data_line_hi : 32;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_BA_HI_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_BUFSTAT_REG 0x70

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern Buffer Status Register
 */
typedef union {
  struct {
    /* rd_buf_ptr - Bits[3:0], RO-D, default = 0h 
       Pointer to current entry in the data pattern buffers used for generating the RD 
       data. 
     */
    UINT32 rd_buf_ptr : 4;

    /* wr_buf_ptr - Bits[7:4], RO-D, default = 0h 
       Pointer to current entry in the data pattern buffers used for generating the WR 
       data. 
     */
    UINT32 wr_buf_ptr : 4;

    /* rsvd_82 - Bits[31:8], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_82 : 24;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_BUFSTAT_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_UNISEQ0WRSTAT_REG 0x71

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern Write Unified Sequencer 0 Status Register
 */
typedef union {
  struct {
    /* uniseq_wrstat - Bits[31:0], RO-D, default = 0h 
       The current contents of the data pattern unified write sequencer buffer.  Bit 0 
       represents the value used for the current chunk. 
     */
    UINT32 uniseq_wrstat : 32;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_UNISEQ0WRSTAT_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_UNISEQ1WRSTAT_REG 0x72

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern Write Unified Sequencer 1 Status Register
 */
typedef union {
  struct {
    /* uniseq_wrstat - Bits[31:0], RO-D, default = 0h 
       The current contents of the data pattern unified write sequencer buffer.  Bit 0 
       represents the value used for the current chunk. 
     */
    UINT32 uniseq_wrstat : 32;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_UNISEQ1WRSTAT_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_UNISEQ2WRSTAT_REG 0x73

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern Write Unified Sequencer 2 Status Register
 */
typedef union {
  struct {
    /* uniseq_wrstat - Bits[31:0], RO-D, default = 0h 
       The current contents of the data pattern unified write sequencer buffer.  Bit 0 
       represents the value used for the current chunk. 
     */
    UINT32 uniseq_wrstat : 32;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_UNISEQ2WRSTAT_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_UNISEQ0RDSTAT_REG 0x74

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern Read Unified Sequencer 0 Status Register
 */
typedef union {
  struct {
    /* uniseq_rdstat - Bits[31:0], RO-D, default = 0h 
       The current contents of the data pattern unified read sequencer buffer.  Bit 0 
       represents the value used for the current chunk. 
     */
    UINT32 uniseq_rdstat : 32;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_UNISEQ0RDSTAT_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_UNISEQ1RDSTAT_REG 0x75

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern Read Unified Sequencer 1 Status Register
 */
typedef union {
  struct {
    /* uniseq_rdstat - Bits[31:0], RO-D, default = 0h 
       The current contents of the data pattern unified read sequencer buffer.  Bit 0 
       represents the value used for the current chunk. 
     */
    UINT32 uniseq_rdstat : 32;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_UNISEQ1RDSTAT_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_UNISEQ2RDSTAT_REG 0x76

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Data Pattern Read Unified Sequencer 2 Status Register
 */
typedef union {
  struct {
    /* uniseq_rdstat - Bits[31:0], RO-D, default = 0h 
       The current contents of the data pattern unified read sequencer buffer.  Bit 0 
       represents the value used for the current chunk. 
     */
    UINT32 uniseq_rdstat : 32;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_UNISEQ2RDSTAT_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_EXTBUF0_REG 0x80

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Pattern Buffer 0
 */
typedef union {
  struct {
    /* data_line0 - Bits[7:0], RW-D, default = AAh 
       Contents of Pattern Buffer 0 Data Line 0
     */
    UINT32 data_line0 : 8;

    /* data_line1 - Bits[15:8], RW-D, default = AAh 
       Contents of Pattern Buffer 0 Data Line 1
     */
    UINT32 data_line1 : 8;

    /* data_line2 - Bits[23:16], RW-D, default = AAh 
       Contents of Pattern Buffer 0 Data Line 2
     */
    UINT32 data_line2 : 8;

    /* data_line3 - Bits[31:24], RW-D, default = AAh 
       Contents of Pattern Buffer 0 Data Line 3
     */
    UINT32 data_line3 : 8;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_EXTBUF0_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_EXTBUF1_REG 0x81

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Pattern Buffer 1
 */
typedef union {
  struct {
    /* data_line0 - Bits[7:0], RW-D, default = AAh 
       Contents of Pattern Buffer 1 Data Line 0
     */
    UINT32 data_line0 : 8;

    /* data_line1 - Bits[15:8], RW-D, default = AAh 
       Contents of Pattern Buffer 1 Data Line 1
     */
    UINT32 data_line1 : 8;

    /* data_line2 - Bits[23:16], RW-D, default = AAh 
       Contents of Pattern Buffer 1 Data Line 2
     */
    UINT32 data_line2 : 8;

    /* data_line3 - Bits[31:24], RW-D, default = AAh 
       Contents of Pattern Buffer 1 Data Line 3
     */
    UINT32 data_line3 : 8;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_EXTBUF1_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_EXTBUF2_REG 0x82

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Pattern Buffer 2
 */
typedef union {
  struct {
    /* data_line0 - Bits[7:0], RW-D, default = AAh 
       Contents of Pattern Buffer 2 Data Line 0
     */
    UINT32 data_line0 : 8;

    /* data_line1 - Bits[15:8], RW-D, default = AAh 
       Contents of Pattern Buffer 2 Data Line 1
     */
    UINT32 data_line1 : 8;

    /* data_line2 - Bits[23:16], RW-D, default = AAh 
       Contents of Pattern Buffer 2 Data Line 2
     */
    UINT32 data_line2 : 8;

    /* data_line3 - Bits[31:24], RW-D, default = AAh 
       Contents of Pattern Buffer 2 Data Line 3
     */
    UINT32 data_line3 : 8;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_EXTBUF2_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_EXTBUF3_REG 0x83

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Pattern Buffer 3
 */
typedef union {
  struct {
    /* data_line0 - Bits[7:0], RW-D, default = AAh 
       Contents of Pattern Buffer 3 Data Line 0
     */
    UINT32 data_line0 : 8;

    /* data_line1 - Bits[15:8], RW-D, default = AAh 
       Contents of Pattern Buffer 3 Data Line 1
     */
    UINT32 data_line1 : 8;

    /* data_line2 - Bits[23:16], RW-D, default = AAh 
       Contents of Pattern Buffer 3 Data Line 2
     */
    UINT32 data_line2 : 8;

    /* data_line3 - Bits[31:24], RW-D, default = AAh 
       Contents of Pattern Buffer 3 Data Line 3
     */
    UINT32 data_line3 : 8;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_EXTBUF3_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_EXTBUF4_REG 0x84

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Pattern Buffer 4
 */
typedef union {
  struct {
    /* data_line0 - Bits[7:0], RW-D, default = AAh 
       Contents of Pattern Buffer 4 Data Line 0
     */
    UINT32 data_line0 : 8;

    /* data_line1 - Bits[15:8], RW-D, default = AAh 
       Contents of Pattern Buffer 4 Data Line 1
     */
    UINT32 data_line1 : 8;

    /* data_line2 - Bits[23:16], RW-D, default = AAh 
       Contents of Pattern Buffer 4 Data Line 2
     */
    UINT32 data_line2 : 8;

    /* data_line3 - Bits[31:24], RW-D, default = AAh 
       Contents of Pattern Buffer 4 Data Line 3
     */
    UINT32 data_line3 : 8;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_EXTBUF4_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_EXTBUF5_REG 0x85

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Pattern Buffer 5
 */
typedef union {
  struct {
    /* data_line0 - Bits[7:0], RW-D, default = AAh 
       Contents of Pattern Buffer 5 Data Line 0
     */
    UINT32 data_line0 : 8;

    /* data_line1 - Bits[15:8], RW-D, default = AAh 
       Contents of Pattern Buffer 5 Data Line 1
     */
    UINT32 data_line1 : 8;

    /* data_line2 - Bits[23:16], RW-D, default = AAh 
       Contents of Pattern Buffer 5 Data Line 2
     */
    UINT32 data_line2 : 8;

    /* data_line3 - Bits[31:24], RW-D, default = AAh 
       Contents of Pattern Buffer 5 Data Line 3
     */
    UINT32 data_line3 : 8;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_EXTBUF5_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_EXTBUF6_REG 0x86

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Pattern Buffer 6
 */
typedef union {
  struct {
    /* data_line0 - Bits[7:0], RW-D, default = AAh 
       Contents of Pattern Buffer 6 Data Line 0
     */
    UINT32 data_line0 : 8;

    /* data_line1 - Bits[15:8], RW-D, default = AAh 
       Contents of Pattern Buffer 6 Data Line 1
     */
    UINT32 data_line1 : 8;

    /* data_line2 - Bits[23:16], RW-D, default = AAh 
       Contents of Pattern Buffer 6 Data Line 2
     */
    UINT32 data_line2 : 8;

    /* data_line3 - Bits[31:24], RW-D, default = AAh 
       Contents of Pattern Buffer 6 Data Line 3
     */
    UINT32 data_line3 : 8;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_EXTBUF6_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_EXTBUF7_REG 0x87

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Pattern Buffer 7
 */
typedef union {
  struct {
    /* data_line0 - Bits[7:0], RW-D, default = AAh 
       Contents of Pattern Buffer 7 Data Line 0
     */
    UINT32 data_line0 : 8;

    /* data_line1 - Bits[15:8], RW-D, default = AAh 
       Contents of Pattern Buffer 7 Data Line 1
     */
    UINT32 data_line1 : 8;

    /* data_line2 - Bits[23:16], RW-D, default = AAh 
       Contents of Pattern Buffer 7 Data Line 2
     */
    UINT32 data_line2 : 8;

    /* data_line3 - Bits[31:24], RW-D, default = AAh 
       Contents of Pattern Buffer 7 Data Line 3
     */
    UINT32 data_line3 : 8;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_EXTBUF7_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_EXTBUF8_REG 0x88

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Pattern Buffer 8
 */
typedef union {
  struct {
    /* data_line0 - Bits[7:0], RW-D, default = AAh 
       Contents of Pattern Buffer 8 Data Line 0
     */
    UINT32 data_line0 : 8;

    /* data_line1 - Bits[15:8], RW-D, default = AAh 
       Contents of Pattern Buffer 8 Data Line 1
     */
    UINT32 data_line1 : 8;

    /* data_line2 - Bits[23:16], RW-D, default = AAh 
       Contents of Pattern Buffer 8 Data Line 2
     */
    UINT32 data_line2 : 8;

    /* data_line3 - Bits[31:24], RW-D, default = AAh 
       Contents of Pattern Buffer 8 Data Line 3
     */
    UINT32 data_line3 : 8;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_EXTBUF8_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_EXTBUF9_REG 0x89

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Pattern Buffer 9
 */
typedef union {
  struct {
    /* data_line0 - Bits[7:0], RW-D, default = AAh 
       Contents of Pattern Buffer 9 Data Line 0
     */
    UINT32 data_line0 : 8;

    /* data_line1 - Bits[15:8], RW-D, default = AAh 
       Contents of Pattern Buffer 9 Data Line 1
     */
    UINT32 data_line1 : 8;

    /* data_line2 - Bits[23:16], RW-D, default = AAh 
       Contents of Pattern Buffer 9 Data Line 2
     */
    UINT32 data_line2 : 8;

    /* data_line3 - Bits[31:24], RW-D, default = AAh 
       Contents of Pattern Buffer 9 Data Line 3
     */
    UINT32 data_line3 : 8;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_EXTBUF9_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_EXTBUF10_REG 0x8A

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Pattern Buffer 10
 */
typedef union {
  struct {
    /* data_line0 - Bits[7:0], RW-D, default = AAh 
       Contents of Pattern Buffer 10 Data Line 0
     */
    UINT32 data_line0 : 8;

    /* data_line1 - Bits[15:8], RW-D, default = AAh 
       Contents of Pattern Buffer 10 Data Line 1
     */
    UINT32 data_line1 : 8;

    /* data_line2 - Bits[23:16], RW-D, default = AAh 
       Contents of Pattern Buffer 10 Data Line 2
     */
    UINT32 data_line2 : 8;

    /* data_line3 - Bits[31:24], RW-D, default = AAh 
       Contents of Pattern Buffer 10 Data Line 3
     */
    UINT32 data_line3 : 8;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_EXTBUF10_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_EXTBUF11_REG 0x8B

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Pattern Buffer 11
 */
typedef union {
  struct {
    /* data_line0 - Bits[7:0], RW-D, default = AAh 
       Contents of Pattern Buffer 11 Data Line 0
     */
    UINT32 data_line0 : 8;

    /* data_line1 - Bits[15:8], RW-D, default = AAh 
       Contents of Pattern Buffer 11 Data Line 1
     */
    UINT32 data_line1 : 8;

    /* data_line2 - Bits[23:16], RW-D, default = AAh 
       Contents of Pattern Buffer 11 Data Line 2
     */
    UINT32 data_line2 : 8;

    /* data_line3 - Bits[31:24], RW-D, default = AAh 
       Contents of Pattern Buffer 11 Data Line 3
     */
    UINT32 data_line3 : 8;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_EXTBUF11_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_EXTBUF12_REG 0x8C

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Pattern Buffer 12
 */
typedef union {
  struct {
    /* data_line0 - Bits[7:0], RW-D, default = AAh 
       Contents of Pattern Buffer 12 Data Line 0
     */
    UINT32 data_line0 : 8;

    /* data_line1 - Bits[15:8], RW-D, default = AAh 
       Contents of Pattern Buffer 12 Data Line 1
     */
    UINT32 data_line1 : 8;

    /* data_line2 - Bits[23:16], RW-D, default = AAh 
       Contents of Pattern Buffer 12 Data Line 2
     */
    UINT32 data_line2 : 8;

    /* data_line3 - Bits[31:24], RW-D, default = AAh 
       Contents of Pattern Buffer 12 Data Line 3
     */
    UINT32 data_line3 : 8;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_EXTBUF12_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_EXTBUF13_REG 0x8D

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Pattern Buffer 13
 */
typedef union {
  struct {
    /* data_line0 - Bits[7:0], RW-D, default = AAh 
       Contents of Pattern Buffer 13 Data Line 0
     */
    UINT32 data_line0 : 8;

    /* data_line1 - Bits[15:8], RW-D, default = AAh 
       Contents of Pattern Buffer 13 Data Line 1
     */
    UINT32 data_line1 : 8;

    /* data_line2 - Bits[23:16], RW-D, default = AAh 
       Contents of Pattern Buffer 13 Data Line 2
     */
    UINT32 data_line2 : 8;

    /* data_line3 - Bits[31:24], RW-D, default = AAh 
       Contents of Pattern Buffer 13 Data Line 3
     */
    UINT32 data_line3 : 8;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_EXTBUF13_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_EXTBUF14_REG 0x8E

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Pattern Buffer 14
 */
typedef union {
  struct {
    /* data_line0 - Bits[7:0], RW-D, default = AAh 
       Contents of Pattern Buffer 14 Data Line 0
     */
    UINT32 data_line0 : 8;

    /* data_line1 - Bits[15:8], RW-D, default = AAh 
       Contents of Pattern Buffer 14 Data Line 1
     */
    UINT32 data_line1 : 8;

    /* data_line2 - Bits[23:16], RW-D, default = AAh 
       Contents of Pattern Buffer 14 Data Line 2
     */
    UINT32 data_line2 : 8;

    /* data_line3 - Bits[31:24], RW-D, default = AAh 
       Contents of Pattern Buffer 14 Data Line 3
     */
    UINT32 data_line3 : 8;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_EXTBUF14_STRUCT;
#endif /* ASM_INC */

#define CPGC_DPAT_EXTBUF15_REG 0x8F

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Pattern Buffer 15
 */
typedef union {
  struct {
    /* data_line0 - Bits[7:0], RW-D, default = AAh 
       Contents of Pattern Buffer 15 Data Line 0
     */
    UINT32 data_line0 : 8;

    /* data_line1 - Bits[15:8], RW-D, default = AAh 
       Contents of Pattern Buffer 15 Data Line 1
     */
    UINT32 data_line1 : 8;

    /* data_line2 - Bits[23:16], RW-D, default = AAh 
       Contents of Pattern Buffer 15 Data Line 2
     */
    UINT32 data_line2 : 8;

    /* data_line3 - Bits[31:24], RW-D, default = AAh 
       Contents of Pattern Buffer 15 Data Line 3
     */
    UINT32 data_line3 : 8;

  } Bits;
  UINT32 Data;
} CPGC_DPAT_EXTBUF15_STRUCT;
#endif /* ASM_INC */

#define CPGC_ERR_CAP_REG 0xA0

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Error Checking Capability Register
 */
typedef union {
  struct {
    /* data_err_mask_width_cap - Bits[1:0], RO, default = 11b 
       The width of the data lane error mask supported by this implementation is 
       encoded as follows: 
     */
    UINT32 data_err_mask_width_cap : 2;

    /* ecc_err_mask_width_cap - Bits[3:2], RO, default = 10b 
       The width of the ECC lane error mask supported by this implementation is encoded 
       as follows: 
     */
    UINT32 ecc_err_mask_width_cap : 2;

    /* data_err_stat_cap - Bits[4:4], RO, default = 1b 
       Number of byte groups composing the error status for the data lanes (excluding 
       ECC byte group if supported) is encoded as follows: 
     */
    UINT32 data_err_stat_cap : 1;

    /* ecc_err_stat_cap - Bits[5:5], RO, default = 1b 
       This bit is set if ECC error status logging is supported.
     */
    UINT32 ecc_err_stat_cap : 1;

    /* rsvd_86 - Bits[7:6], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_86 : 2;

    /* chunk_err_stat_cap - Bits[8:8], RO, default = 1b 
       This bit is set if error chunk logging capability is supported.
     */
    UINT32 chunk_err_stat_cap : 1;

    /* cmd_err_capt_cap - Bits[9:9], RO, default = 1b 
       This bit is set if command error capture is supported.
     */
    UINT32 cmd_err_capt_cap : 1;

    /* rsvd_85 - Bits[15:10], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_85 : 6;

    /* err_inj_cap - Bits[16:16], RO, default = 0b 
       If this is set then error injection through disabling RD data Inversion/DC mask 
       rotation is supported. 
     */
    UINT32 err_inj_cap : 1;

    /* rsvd_84 - Bits[31:17], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_84 : 15;

  } Bits;
  UINT32 Data;
} CPGC_ERR_CAP_STRUCT;
#endif /* ASM_INC */

#define CPGC_ERR_CTL_REG 0xA1

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Error Checking Control Register
 */
typedef union {
  struct {
    /* stop_on_n - Bits[5:0], RW-D, default = 0h 
       Reserved
     */
    UINT32 stop_on_n : 6;

    /* rsvd_89 - Bits[7:6], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_89 : 2;

    /* clr_all_err - Bits[8:8], RW1S, default = 0h  */
    UINT32 clr_all_err : 1;

    /* inj_err - Bits[9:9], RW, default = 0h  */
    UINT32 inj_err : 1;

    /* rsvd_88 - Bits[11:10], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_88 : 2;

    /* stop_on_error - Bits[13:12], RW-D, default = 0h 
       Defines test stop conditions based on error checking as follows:
     */
    UINT32 stop_on_error : 2;

    /* rsvd_87 - Bits[15:14], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_87 : 2;

    /* chunk_errchk_en - Bits[23:16], RW-D, default = FFh 
       Defines which chunk within the burst of data (i.e. bit within a burst of 8-bits 
       for each lane) to check for errors. 
     */
    UINT32 chunk_errchk_en : 8;

    /* burst_errchk_en - Bits[31:24], RW-D, default = FFh 
       Defines a periodic burst (CAS command) mask that repeats every 8 CAS commands as 
       follows: 
     */
    UINT32 burst_errchk_en : 8;

  } Bits;
  UINT32 Data;
} CPGC_ERR_CTL_STRUCT;
#endif /* ASM_INC */

#define CPGC_ERR_LANE_MASK_LO_REG 0xA2

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Error Lane Mask for Byte Groups 0-3 Register
 */
typedef union {
  struct {
    /* data_errchk_en_lo - Bits[31:0], RW-D, default = FFFFFFFFh 
       A one-hot mask used to enable error checking on data lanes [31:0].  Only lanes 
       selected through this mask will be checked for errors. 
     */
    UINT32 data_errchk_en_lo : 32;

  } Bits;
  UINT32 Data;
} CPGC_ERR_LANE_MASK_LO_STRUCT;
#endif /* ASM_INC */

#define CPGC_ERR_LANE_MASK_HI_REG 0xA3

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Error Lane Mask for Byte Groups 4-7 Register
 */
typedef union {
  struct {
    /* data_errchk_en_hi - Bits[31:0], RW-D, default = FFFFFFFFh 
       A one-hot mask used to enable error checking on data lanes [63:32].  Only lanes 
       selected through this mask will be checked for errors. 
     */
    UINT32 data_errchk_en_hi : 32;

  } Bits;
  UINT32 Data;
} CPGC_ERR_LANE_MASK_HI_STRUCT;
#endif /* ASM_INC */

#define CPGC_ERR_LANE_XMASK_REG 0xA4

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Error Lane Mask
 */
typedef union {
  struct {
    /* ecc_errchk_en - Bits[7:0], RW-D, default = FFh 
       Used as an error check enable mask for ECC lanes.  Only the lanes selected 
       through the mask will be checked for errors. 
     */
    UINT32 ecc_errchk_en : 8;

    /* rsvd_90 - Bits[31:8], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_90 : 24;

  } Bits;
  UINT32 Data;
} CPGC_ERR_LANE_XMASK_STRUCT;
#endif /* ASM_INC */

#define CPGC_ERR_STAT_LO_REG 0xA5

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Error Status for Byte Groups 0-3 Register
 */
typedef union {
  struct {
    /* lane_err_stat03 - Bits[31:0], RW1C-D, default = 0h 
       Indicates if a mismatch was detected between the WR and the RD data on one of 
       the lanes belonging to byte groups 0 - 3. 
     */
    UINT32 lane_err_stat03 : 32;

  } Bits;
  UINT32 Data;
} CPGC_ERR_STAT_LO_STRUCT;
#endif /* ASM_INC */

#define CPGC_ERR_STAT_HI_REG 0xA6

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Error Status for Byte Groups 4-7 Register
 */
typedef union {
  struct {
    /* lane_err_stat47 - Bits[31:0], RW1C-D, default = 0h 
       Indicates if a mismatch was detected between the WR and the RD data on one of 
       the lanes belonging to byte groups 0 - 3. 
     */
    UINT32 lane_err_stat47 : 32;

  } Bits;
  UINT32 Data;
} CPGC_ERR_STAT_HI_STRUCT;
#endif /* ASM_INC */

#define CPGC_ERR_XSTAT_REG 0xA7

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Extended Error Status Register
 */
typedef union {
  struct {
    /* ecc_lane_err_stat - Bits[7:0], RW1C-D, default = 0h 
       Indicates if a mismatch was detected between the WR and the RD data on one of 
       the lanes belonging to the ECC byte group.  The error status information is 
       encoded as follows: 
     */
    UINT32 ecc_lane_err_stat : 8;

    /* bytegrp_err_stat - Bits[15:8], RW1C-D, default = 0h 
       One-hot field with each bit corresponding to a specific byte group.  Bit 0 
       corresponds to byte group 0, bit 1 corresponds to byte group 1, 
     */
    UINT32 bytegrp_err_stat : 8;

    /* eccgrp_err_stat - Bits[16:16], RW1C-D, default = 0h 
       Set if the ECC byte group has at least one lane that accumulated at least one 
       error.  Note: this field is only available if ECC is supported for the current 
       implementation, 
     */
    UINT32 eccgrp_err_stat : 1;

    /* rsvd_91 - Bits[23:17], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_91 : 7;

    /* chunk_err_stat - Bits[31:24], RO-D, default = 0h 
       A one-hot field where each bit corresponds to a specific chunk (i.e. bit within 
       a burst of 8-bits). 
     */
    UINT32 chunk_err_stat : 8;

  } Bits;
  UINT32 Data;
} CPGC_ERR_XSTAT_STRUCT;
#endif /* ASM_INC */

#define CPGC_ERR_CNTRCTL_REG 0xA8

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Error Counter Control Register
 */
typedef union {
  struct {
    /* target_lane - Bits[3:0], RW-D, default = Fh 
       Selects the lane that the (specific) error counter will be bound to in the byte 
       group selected by TARGET_BYTELANE. 
     */
    UINT32 target_lane : 4;

    /* target_bytegrp - Bits[7:4], RW-D, default = Fh 
       Selects the byte group that the error counter will be bound to.  Legal values 
       are 0 through 7 (0 through 8 if ECC is supported). 
     */
    UINT32 target_bytegrp : 4;

    /* rsvd_92 - Bits[31:8], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_92 : 24;

  } Bits;
  UINT32 Data;
} CPGC_ERR_CNTRCTL_STRUCT;
#endif /* ASM_INC */

#define CPGC_ERR_CNTR_REG 0xA9

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Error Counter Register
 */
typedef union {
  struct {
    /* err_cntr - Bits[22:0], RW1C-D, default = 0h 
       Errors accumulated for the error type selected through TARGET_BYTEGRP and 
       TARGET_LANE. 
     */
    UINT32 err_cntr : 23;

    /* err_ovrflow - Bits[23:23], RW1C-D, default = 0h 
       Indicates that ERR_CNTR has wrapped around at least once.
     */
    UINT32 err_ovrflow : 1;

    /* rsvd_93 - Bits[31:24], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_93 : 8;

  } Bits;
  UINT32 Data;
} CPGC_ERR_CNTR_STRUCT;
#endif /* ASM_INC */

#define CPGC_ERR_ERRLOOP_REG 0xAA

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Error Loop Status Register
 */
typedef union {
  struct {
    /* err_loop - Bits[15:0], RO-D, default = 0h 
       The last command sequence executing before test exit.  When stopping on an 
       error, ERR_LOOP will be the last command sequence loop that 
     */
    UINT32 err_loop : 16;

    /* rsvd_95 - Bits[23:16], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_95 : 8;

    /* err_seq_ptr - Bits[25:24], RO-D, default = 0h 
       Pointer to last executing subsequence before test exit.  When stopping on an 
       error, ERR_SEQ_PTR will point to the subsequence that 
     */
    UINT32 err_seq_ptr : 2;

    /* rsvd_94 - Bits[31:26], RV, default = 0h 
       Reserved
     */
    UINT32 rsvd_94 : 6;

  } Bits;
  UINT32 Data;
} CPGC_ERR_ERRLOOP_STRUCT;
#endif /* ASM_INC */

#define CPGC_ERR_ERRBURST_REG 0xAB

#ifndef ASM_INC
/* Struct format extracted from XML file ..\..\..\data\xml\ConfigDB-05152012_03_30\cpgc.xml.
 * Error Burst Status Register
 */
typedef union {
  struct {
    /* err_burst - Bits[31:0], RO-D, default = 0h 
       The last burst issued before test exit.  When stopping on an error, ERR_BURST 
       will be the last burst issued that experienced an error. 
     */
    UINT32 err_burst : 32;

  } Bits;
  UINT32 Data;
} CPGC_ERR_ERRBURST_STRUCT;
#endif /* ASM_INC */

#endif /* _CPGC_H_ */

#ifndef _MCHREGS_H_
#define _MCHREGS_H_

#define MC_DCO_OFFSET                   0xF

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 DRPLOCK            :1;             /**<DRP lock bit */
        UINT32 reserved1          :7;
        UINT32 REUTLOCK           :1;             /**<REUT lock bit*/
        UINT32 reserved2          :19;
        UINT32 PMICTL             :1;             /**< PRI Control Select: */
        UINT32 PMIDIS             :1;
        UINT32 DIOIC              :1;             /** < DDRIO initialization is complete */
        UINT32 IC                 :1;             /**< D-unit Initialization Complete */
    } field;
} RegDCO;                                           /**< DRAM Controller Operation Register*/
#pragma pack()

#endif /* _MCHREGS_H_ */
