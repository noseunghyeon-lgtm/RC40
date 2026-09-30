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
#ifndef _HWINPDIG_H_
#define _HWINPDIG_H_

#ifdef _HWINPDIG_C_
#define HWINPDIG_SCOPE_D
#else
#define HWINPDIG_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Input Digital)                 *
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
#define  HwInpDig_ErrBitMaskFctCallErr_DU8  0x01U /* Function call was erroneous*/


/* typedefs/structures *****************************************************************************
 * Typedefs/structures to be made available to other modules.                                      */
typedef struct HwInpDig_prm_t
{
  bds_in_di_signal_te idPin_e;  // Pin Identifier; @rawrange[0x400..0x4FF]; @mrxml-ignore(rationale: assigned in HwInp)
} HwInpDig_prm_ts;

/* *********************************************************************************************** */
typedef struct HwInpDig_outp_t
{
  bool flg_l;         // Digital input flag
  uint8 stErrDetn_u8; // State error detection; @errdetnref(HwInpDig_ErrBitMask)
} HwInpDig_outp_ts;

/* *********************************************************************************************** */
typedef struct HwInpDig_t
{
  HwInpDig_prm_ts  prm_s;
  HwInpDig_outp_ts outp_s;
} HwInpDig_ts;

/* public data *************************************************************************************
 * Declarations of variables to be made available to other modules.                                */


/* public functions ********************************************************************************
 * Declarations of functions to be made available to other modules.                                *
 * Use HWINPDIG_SCOPE_D in front of each line.                                                     */
HWINPDIG_SCOPE_D void HwInpDig_ini(HwInpDig_ts *dig_ps); // Initialize pin structure
HWINPDIG_SCOPE_D void HwInpDig(HwInpDig_ts *dig_ps);     // Get values from digital pin PinId_e and copy result to structure dig_ps

/* *********************************************************************************************** */
#endif /* #ifndef _HWINPDIG_H_ */
/* EOF ########################################################################################### */
