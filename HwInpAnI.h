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
#ifndef _HWINPANI_H_
#define _HWINPANI_H_

#ifdef _HWINPANI_C_
#define HWINPANI_SCOPE_D
#else
#define HWINPANI_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Input Analog Current)          *
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
#define HwInpAnI_ErrBitMaskFctCallErr_DU8    0x01u /* Function call was erroneous*/
#define HwInpAnI_ErrBitMaskShnt_DU8          0x02u /* shunt error (disabled) */

/* typedefs/structures *****************************************************************************
 * Typedefs/structures to be made available to other modules.                                      */
typedef struct HwInpAnI_prv_t
{
  bool flgIsShntActv_l;         // AIC shunt is active on the pin
  bds_in_aic_ts stApi_s;        // API structure for analog current pin
} HwInpAnI_prv_ts;

/* *********************************************************************************************** */
typedef struct HwInpAnI_prm_t
{
  bds_in_fi_signal_te idPin_e;  // Pin Identifier; @rawrange[0x700..0x7FF]; @mrxml-ignore(rationale: assigned in HwInp)
} HwInpAnI_prm_ts;

/* *********************************************************************************************** */
typedef struct HwInpAnI_outp_t
{
  uint16 i_uA_u16;              // Analog current value; @physrange[0uA..25000uA]
  uint8  stErrDetn_u8;          // State error detection; @errdetnref(HwInpAnI_ErrBitMask)
} HwInpAnI_outp_ts;

/* *********************************************************************************************** */
typedef struct HwInpAnI_t
{
  HwInpAnI_prm_ts  prm_s;
  HwInpAnI_prv_ts  prv_s;
  HwInpAnI_outp_ts outp_s;
} HwInpAnI_ts;

/* public data *************************************************************************************
 * Declarations of variables to be made available to other modules.                                */


/* public functions ********************************************************************************
 * Declarations of functions to be made available to other modules.                                *
 * Use HWINPANI_SCOPE_D in front of each line.                                                     */
HWINPANI_SCOPE_D void HwInpAnI_ini(HwInpAnI_ts *anI_ps); // Intialize pin structure
HWINPANI_SCOPE_D void HwInpAnI(HwInpAnI_ts *anI_ps);     // Get values from analog current pin and copy result to structure anI_ps

/* *********************************************************************************************** */
#endif /* #ifndef _HWINPANI_H_ */
/* EOF ########################################################################################### */
