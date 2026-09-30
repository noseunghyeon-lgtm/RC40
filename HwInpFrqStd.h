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
#ifndef _HWINPFRQSTD_H_
#define _HWINPFRQSTD_H_

#ifdef _HWINPFRQSTD_C_
#define HWINPFRQSTD_SCOPE_D
#else
#define HWINPFRQSTD_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Input Frequency Standard)      *
 * @suprtd-ecutype(RC5-6/40, RC18-12/40, RC27-18/40)     *
 * *******************************************************/

/* include files ***********************************************************************************
 * List of include files needed in this module.                                                    *
 * DO NOT INCLUDE HEADER FILES WITH VARIABLE DECLARATIONS HERE!                                    *
 * ONLY INCLUDES OF TYPEDEFS OR DEFINES ARE ALLOWED!                                               */

/* Bsw includes */
#include "bds_in.h"

/* defines *****************************************************************************************
 * List of defines needed in this header file and defines to be made available to other modules.   */

/* ############################################################################################### */
/* Error Symptom Bit Masks */
/* ############################################################################################### */
#define HwInpFrqStd_ErrBitMaskFctCallErr_DU8    0x01u // Function call was erroneous
#define HwInpFrqStd_ErrBitMaskRngChkPrm_DU8     0x02u // Parameter range check error
#define HwInpFrqStd_ErrBitMaskPerdAbvMax_DU8    0x04u // Measured period exceeds maximum period length (as configured in easyConfig).
#define HwInpFrqStd_ErrBitMaskDutyCycAbvMax_DU8 0x08u // Current duty cycle returned from API is greater than 1000perml, output is limited to 1000perml


/* typedefs/structures *****************************************************************************
 * Typedefs/structures to be made available to other modules.                                      */
typedef struct HwInpFrqStd_prm_t
{
  bds_in_fi_signal_te idPin_e; // Pin Identifier; @rawrange[0x700..0x7FF]; @mrxml-ignore(rationale: assigned in HwInp)
  bool flgAvrgFrq_l;           // Averaging frequency with bds_in_fi_buffered() API if available
} HwInpFrqStd_prm_ts;

/* *********************************************************************************************** */
typedef struct HwInpFrqStd_prv_t
{
  bds_in_fi_ts  stApi_s;       // API structure for frequency pin
  uint32 tiPerdMemd_us_u32;    // Memorized period from a previous cycle
  bool   flgPerdMemAvl_l;      // Flag indicating whether the period memory is available
  uint32 frqOrig_p1Hz_u32;     // frequency input signal from bds_in_fi_getStatus API (original/reference for debugging)
  uint32 cntrPerdPrev_u32;     // Previous period count value from last call
  uint8  nrUsedPerd_u8;        // Number of period samples used for averaging

} HwInpFrqStd_prv_ts;

/* *********************************************************************************************** */
typedef struct HwInpFrqStd_outp_t
{
  uint32 frq_p1Hz_u32;         // Frequency input value; @physrange[0Hz..20000Hz]
  uint32 cntrPerd_u32;         // Frequency input period counter value; @rawrange[0..16777215]
  uint8  stErrDetn_u8;         // State error detection; @errdetnref(HwInpFrqStd_ErrBitMask)
  uint32 tiPls_us_u32;         // Last high time duration in µs; @physrange[0us..1000000us]
  uint32 tiPerd_us_u32;        // Period duration in µs; @physrange[0us..1000000us]
  uint16 ratDutyCyc_perml_u16; // Current duty cycle; @physrange[0perml..1000perml]
} HwInpFrqStd_outp_ts;

/* *********************************************************************************************** */
typedef struct HwInpFrqStd_t
{
  HwInpFrqStd_prm_ts  prm_s;
  HwInpFrqStd_prv_ts  prv_s;
  HwInpFrqStd_outp_ts outp_s;
} HwInpFrqStd_ts;

/* public functions ********************************************************************************
 * Declarations of functions to be made available to other modules.                                *
 * Use HWINPFRQSTD_SCOPE_D in front of each line.                                                  */
HWINPFRQSTD_SCOPE_D void HwInpFrqStd_ini(HwInpFrqStd_ts *frqStd_ps); // Initialize pin structure
HWINPFRQSTD_SCOPE_D void HwInpFrqStd(HwInpFrqStd_ts *frqStd_ps);     // Get values from standard frequency pin PinId_e_e and copy results to structure frqStd_ps

/* *********************************************************************************************** */
#endif /* #ifndef _HWINPFRQSTD_H_ */
/* EOF ########################################################################################### */
