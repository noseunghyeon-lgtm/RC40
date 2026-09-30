/*<BRHead>
 ***************************************************************************************************
 *
 * Copyright © 2019 - 2023 Bosch Rexroth AG
 *
 ***************************************************************************************************
 *
 *    __   __   ___  ___           __   __        __   __  ___
 *   /_ / /  / /__  /    /__/     /_ / /_   |_ / /_ / /  /  /  /__/
 *  /__/ /__/ __ / /__  /  /     /  | /__  /  | /  | /__/  /  /  /
 *
 *
 ***************************************************************************************************
 </BRHead>*/
#ifndef _HWINPSENT_H_
#define _HWINPSENT_H_

#ifdef _HWINPSENT_C_
#define HWINPSENT_SCOPE_D
#else
#define HWINPSENT_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Input Sent)                    *
 * @suprtd-ecutype(RC5-6/40, RC18-12/40, RC27-18/40)     *
 * *******************************************************/

/* include files ***********************************************************************************
 * List of include files needed in this module.                                                    *
 * DO NOT INCLUDE HEADER FILES WITH VARIABLE DECLARATIONS HERE!                                    *
 * ONLY INCLUDES OF TYPEDEFS OR DEFINES ARE ALLOWED!                                               */

/* Bsw includes */
#include "bds_in.h"
#include "string.h"

/* defines *****************************************************************************************
 * List of defines needed in this header file and defines to be made available to other modules.   */
#define HwInpSent_SizeAry_DU8 32u

/* ############################################################################################### */
/* Error Symptom Bit Masks */
/* ############################################################################################### */
#define HwInpSent_ErrBitMaskScg_DU8         0x01u // Short circuit to ground
#define HwInpSent_ErrBitMaskScbOl_DU8       0x02u // Short circuit to battery or open-load
#define HwInpSent_ErrBitMaskComErr_DU8      0x04u // Communication error (any errors indicated by BSW matching the pattern SENT_ERROR_*, except for short circuits and open-load).
#define HwInpSent_ErrBitMaskBswCallErr_DU8  0x08u // The BSW returned an error (pin number invalid or SENT channel inactive). This error indicates a BSW configuration issue.
#define HwInpSent_ErrBitMaskBufOvf_DU8      0x10u // Fast data buffer overflow
#define HwInpSent_ErrBitMaskBufEmpty_DU8    0x20u // Fast data buffer empty
#define HwInpSent_ErrBitMaskBufCopyErr_DU8  0x40u // Error while copying the fast data buffer


/* typedefs/structures *****************************************************************************
 * Typedefs/structures to be made available to other modules.                                      */
typedef struct HwInpSent_prm_t
{
  bds_in_sent_signal_te idPin_e; // Pin Identifier; @rawrange[0x800..0x8FF]; @mrxml-ignore(rationale: assigned in HwInp)
} HwInpSent_prm_ts;

/* *********************************************************************************************** */
typedef struct HwInpSent_prv_t
{
  uint32 stCh_u32;
  uint8 retValGetSt_u8;
  uint8 retValGetSd_u8;
  uint8 retValregisterFDFifo_u8;
  uint8 retValregisterCopyFDFifo_u8;
  uint8 retValcopyFDFifo_u8;
  bds_in_sent_slowData_ts  SerlMsg_s;
  bds_in_sent_decodedFD_ts buf_as[HwInpSent_SizeAry_DU8];
  bds_in_sent_decodedFD_ts bufCopy_as[HwInpSent_SizeAry_DU8];
  bds_in_sent_copyFDBuf_ts infoBuf_s;
} HwInpSent_prv_ts;

/* *********************************************************************************************** */
typedef struct HwInpSent_outp_t
{
  uint16  dataFastCh1_au16[HwInpSent_SizeAry_DU8]; /* Data of fast channel 1 */
  uint16  dataFastCh2_au16[HwInpSent_SizeAry_DU8]; /* Data of fast channel 2 */
  bool    flgStBit0_al[HwInpSent_SizeAry_DU8];     /* Bit 0 of the status and communication nibble */
  bool    flgStBit1_al[HwInpSent_SizeAry_DU8];     /* Bit 1 of the status and communication nibble */
  uint8   sizeSample_u8;                           /* Sample size of fast channel data and status bits; @rawrange[0..32] */
  bool    flgNewSerlMsgRxd_l;                      /* Flag indicating that a new serial message has been received (slow channel) */
  uint8   idSerlMsg_u8;                            /* Message ID of the serial message (slow channel) */
  uint16  dataSerlMsg_u16;                         /* Data of the serial message (slow channel) */
  uint8   stErrDetn_u8;                            /* Status of the error detection; @errdetnref(HwInpSent_ErrBitMask) */
} HwInpSent_outp_ts;

/* *********************************************************************************************** */
typedef struct HwInpSent_t
{
  HwInpSent_prm_ts  prm_s;
  HwInpSent_prv_ts  prv_s;
  HwInpSent_outp_ts outp_s;
} HwInpSent_ts;

/* public functions ********************************************************************************
 * Declarations of functions to be made available to other modules.                                *
 * Use HWINPSENT_SCOPE_D in front of each line.                                                    */
HWINPSENT_SCOPE_D void HwInpSent_ini(HwInpSent_ts *Sent_ps); // Initialize SENT input
HWINPSENT_SCOPE_D void HwInpSent(HwInpSent_ts *Sent_ps);     // Evaluate SENT pin status and read data

/* *********************************************************************************************** */
#endif /* #ifndef _HWINPSENT_H_ */
/* EOF ########################################################################################### */
