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
#ifndef _HWINPR_H_
#define _HWINPR_H_

#ifdef _HWINPR_C_
#define HWINPR_SCOPE_D
#else
#define HWINPR_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Input Resistance)              *
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
#define  HwInpR_ErrBitMaskFctCallErr_DU8  0x10u /* Function call was erroneous*/


/* typedefs/structures *****************************************************************************
 * Typedefs/structures to be made available to other modules.                                      */
typedef struct HwInpR_prm_t
{
  bds_in_ri_signal_te idPin_e; // Pin Identifier; @rawrange[0x600..0x6FF]; @mrxml-ignore(rationale: assigned in HwInp)
} HwInpR_prm_ts;

/* *********************************************************************************************** */
typedef struct HwInpR_prv_t
{
  bds_in_ri_ts  stApi_s;       // API structure for analog resistance pin
} HwInpR_prv_ts;

/* *********************************************************************************************** */
typedef struct HwInpR_outp_t
{
  uint32 r_Ohm_u32;    // Resistance input values; @physrange[0Ohm..700000Ohm]
  uint8  stErrDetn_u8; // State error detection; @errdetnref(HwInpR_ErrBitMask)
} HwInpR_outp_ts;

/* *********************************************************************************************** */
typedef struct HwInpR_t
{
  HwInpR_prm_ts  prm_s;
  HwInpR_prv_ts  prv_s;
  HwInpR_outp_ts outp_s;
} HwInpR_ts;

/* public functions ********************************************************************************
 * Declarations of functions to be made available to other modules.                                *
 * Use HWINPR_SCOPE_D in front of each line.                                                       */
HWINPR_SCOPE_D void HwInpR_ini(HwInpR_ts *r_ps); // Initialize pin structure
HWINPR_SCOPE_D void HwInpR(HwInpR_ts *r_ps);     // Get values from resistance pin PinId_e and copy result to structure r_ps
/* *********************************************************************************************** */
#endif /* #ifndef _HWINPR_H_ */
/* EOF ########################################################################################### */
