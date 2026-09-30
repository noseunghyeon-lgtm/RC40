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
#ifndef _HWINPANU_H_
#define _HWINPANU_H_

#ifdef _HWINPANU_C_
#define HWINPANU_SCOPE_D
#else
#define HWINPANU_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Input Analog Voltage)          *
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
#define  HwInpAnU_ErrBitMaskFctCallErr_DU8  0x01u /* Function call was erroneous*/


/* typedefs/structures *****************************************************************************
 * Typedefs/structures to be made available to other modules.                                      */
typedef struct HwInpAnU_prv_t
{
  bds_in_aiv_ts stApi_s;  		// API structure for analog voltage pin
} HwInpAnU_prv_ts;

/* *********************************************************************************************** */
typedef struct HwInpAnU_prm_t
{
  bds_in_aiv_signal_te idPin_e; // Pin Identifier; @rawrange[0x500..0x5FF]; @mrxml-ignore(rationale: assigned in HwInp)
} HwInpAnU_prm_ts;

/* *********************************************************************************************** */
typedef struct HwInpAnU_outp_t
{
  uint16 u_mV_u16;              // Analog voltage value; @physrange[0mV..40000mV]
  uint8  stErrDetn_u8;          // State error detection; @errdetnref(HwInpAnU_ErrBitMask)
} HwInpAnU_outp_ts;

/* *********************************************************************************************** */
typedef struct HwInpAnU_t
{
  HwInpAnU_prm_ts  prm_s;
  HwInpAnU_prv_ts  prv_s;
  HwInpAnU_outp_ts outp_s;
} HwInpAnU_ts;

/* public data *************************************************************************************
 * Declarations of variables to be made available to other modules.                                */


/* public functions ********************************************************************************
 * Declarations of functions to be made available to other modules.                                *
 * Use HWINPANU_SCOPE_D in front of each line.                                                     */
HWINPANU_SCOPE_D void HwInpAnU_ini(HwInpAnU_ts *anU_ps); // Intialize pin structure
HWINPANU_SCOPE_D void HwInpAnU(HwInpAnU_ts *anU_ps);     // Get values from calibrated analog voltage pin and copy result to structure anU_ps

/* *********************************************************************************************** */
#endif /* #ifndef _HWINPANU_H_ */
/* EOF ########################################################################################### */
