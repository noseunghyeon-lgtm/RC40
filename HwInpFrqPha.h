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
#ifndef _HWINPFRQPHA_H_
#define _HWINPFRQPHA_H_

#ifdef _HWINPFRQPHA_C_
#define HWINPFRQPHA_SCOPE_D
#else
#define HWINPFRQPHA_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Input Frequency Phase)         *
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
#define HwInpFrqPha_ErrBitMaskFctCallErr_DU8      0x01u   /* Function call was erroneous */
#define HwInpFrqPha_ErrBitMaskSig1PerdAbvMax_DU8  0x02u   /* Measured period exceeds maximum period length (as configured in easyConfig) for primary signal. */
#define HwInpFrqPha_ErrBitMaskSig2PerdAbvMax_DU8  0x04u   /* Measured period exceeds maximum period length (as configured in easyConfig) for secondary signal. */

/* typedefs/structures *****************************************************************************
 * Typedefs/structures to be made available to other modules.                                      */
typedef enum HwInpFrqPha_stDir_t
{
  HwInpFrqPha_stStandStill_E = 0, // Standstill
  HwInpFrqPha_stDirLe_E = 1,      // Direction left (CCW)
  HwInpFrqPha_stDirRi_E = 2,      // Direction right (CW)
} HwInpFrqPha_stDir_te;

/* *********************************************************************************************** */
typedef struct HwInpFrqPha_prm_t
{
  bds_in_phase_signal_te idCh_e;  // Channel Identifier; @rawrange[0x900..0x9FF]; @mrxml-ignore(rationale: assigned in HwInp)
} HwInpFrqPha_prm_ts;

/* *********************************************************************************************** */
typedef struct HwInpFrqPha_prv_t
{
  bds_in_stPhase_ts stApi_s;      // API structure for whole data of phase channel measurement
} HwInpFrqPha_prv_ts;

/* *********************************************************************************************** */
typedef struct HwInpFrqPha_outp_t
{
  uint32 frq_p1Hz_u32;            // Frequency input signal 1
  uint32 frq2_p1Hz_u32;           // Frequency input signal 2
  HwInpFrqPha_stDir_te stDir_e;   // Rotation direction of 2-frequency sensor
  uint16 cntrPerd_u16;            // Frequency input period counter (signal 1)
  uint8  stErrDetn_u8;            // State error detection, @errdetnref(HwInpFrqPha_ErrBitMask)
} HwInpFrqPha_outp_ts;

/* *********************************************************************************************** */
typedef struct HwInpFrqPha_t
{
  HwInpFrqPha_prm_ts  prm_s;
  HwInpFrqPha_prv_ts  prv_s;
  HwInpFrqPha_outp_ts outp_s;
} HwInpFrqPha_ts;

/* public data *************************************************************************************
 * Declarations of variables to be made available to other modules.                                */


/* public functions ********************************************************************************
 * Declarations of functions to be made available to other modules.                                *
 * Use HWINPFRQPHA_SCOPE_D in front of each line.                                                  */
HWINPFRQPHA_SCOPE_D void HwInpFrqPha_ini(HwInpFrqPha_ts *frqPha_ps); // Initialize values of pin structure
HWINPFRQPHA_SCOPE_D void HwInpFrqPha(HwInpFrqPha_ts *frqPha_ps);     // Get values from phase channel specified with Ch_e, interpret data and copy results to structure frqPha_ps
/* *********************************************************************************************** */
#endif /* #ifndef _HWINPFRQPHA_H_ */
/* EOF ########################################################################################### */
