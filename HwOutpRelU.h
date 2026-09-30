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

#ifndef _HWOUTPRELU_H_
#define _HWOUTPRELU_H_

#ifdef _HWOUTPU_C_
#define HWOUTPRELU_SCOPE_D
#else
#define HWOUTPRELU_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Output Relative Voltage)       *
 * @suprtd-ecutype(RC5-6/40, RC18-12/40, RC27-18/40)     *
 * *******************************************************/

/* include files ***********************************************************************************
* List of include files needed in this module.                                                     *
* DO NOT INCLUDE HEADER FILES WITH VARIABLE DECLARATIONS HERE!                                     *
* ONLY INCLUDES OF TYPEDEFS OR DEFINES ARE ALLOWED!                                               */
/* API include files **************************************************************************** */
#include "HwOutpCmn.h"

/* defines *****************************************************************************************
* List of defines needed in this header file and defines to be made available to other modules.   */
// Error detection symptoms
#define HwOutpRelU_ErrBitMaskRngChkPrm_DU16    0x0001u // Parameter range check error
#define HwOutpRelU_ErrBitMaskRngChkInpSig_DU16 0x0002u // Input signal range check error
#define HwOutpRelU_ErrBitMaskScb_DU16          0x0004u // DFC_rbr_out[K|A][XX]SCB - Short circuit to battery (SCB) at connector pin K[XX] or A[XX]
#define HwOutpRelU_ErrBitMaskScg_DU16          0x0008u // DFC_rbr_out[K|A][XX]SCG - Short circuit to ground (SCG) at connector pin K[XX] or A[XX]
#define HwOutpRelU_ErrBitMaskOutOfRng_DU16     0x0010u // DFC_rbr_out[K|A][XX]OR - Setpoint deviation (OR) at connector pin K[XX] or A[XX] - Only warning. Output is not locked!
#define HwOutpRelU_ErrBitMaskLockd_DU16        0x0100u // Output pin locked by error (see HwOutpCmn_ErrBitCmbnLockd_D)
#define HwOutpRelU_ErrBitMaskBswCallErr_DU16   0x4000u // BSW function call failed.

/* typedefs/structures *****************************************************************************
* Typedefs/structures to be made available to other modules.                                      */
typedef enum HwOutpRelU_ErrReactn_t
{
  HwOutpRelU_ErrReactnNorm_E = 0,         // No Reaction
  HwOutpRelU_ErrReactnPwrOff_E            // Set output value to zero
} HwOutpRelU_ErrReactn_te;

typedef enum HwOutpRelU_Fct_t
{
  HwOutpRelU_FctOff_E = 0,                // Output and error detection off
  HwOutpRelU_FctOn_E                      // Function on
} HwOutpRelU_Fct_te;

typedef struct HwOutpRelU_inp_t
{
  HwOutpRelU_ErrReactn_te stErrReactn_e;  // Error reaction
  uint16 uRel_perml_u16;                  // Voltage relative to supply battery; @physrange[0perml..750perml]
} HwOutpRelU_inp_ts;

typedef struct HwOutpRelU_prm_t
{
  HwOutpRelU_Fct_te stFct_e;              // Channel type unused or used in application SW
  bds_out_aov_signal_te idPin_e;          // Pin identifier; @mrxml-ignore(rationale: assigned in HwOutp)
  uint16 uDe_mV_u16;                      // Maximum voltage deviation for output; @physrange[0mV..500mV]
} HwOutpRelU_prm_ts;

typedef struct HwOutpRelU_prv_t
{
  HwOutpRelU_Fct_te stFctIni_e;           // Store stFct during initialization
  bds_out_aov_ts stApi_s;                 // Channel information by API
} HwOutpRelU_prv_ts;

typedef struct HwOutpRelU_outp_t
{
  uint16 uMeasd_mV_u16;                   // Measured voltage at output
  uint16 stErrDetn_u16;                   // Error detection; @errdetnref(HwOutpRelU_ErrBitMask)
} HwOutpRelU_outp_ts;

typedef struct HwOutpRelU_t
{
  HwOutpRelU_inp_ts inp_s;
  HwOutpRelU_prm_ts prm_s;
  HwOutpRelU_prv_ts prv_s;
  HwOutpRelU_outp_ts outp_s;
} HwOutpRelU_ts;

/* public data *************************************************************************************
* Declarations of variables to be made available to other modules. Use SCOPE_D in front of each    *
* line.                                                                                           */
/* public functions ********************************************************************************
* Declarations of functions to be made available to other modules. Use SCOPE_D in front of each    *
* line.                                                                                           */
HWOUTPRELU_SCOPE_D void HwOutpRelU_ini (HwOutpRelU_ts *relU_ps);  // Initialization of the output pin
HWOUTPRELU_SCOPE_D void HwOutpRelU(HwOutpRelU_ts *relU_ps);  // Setting output pin to the desired value and reading the status of the pin and providing an error status information in the output structure.

/* ********************************************************************************************** */

#endif /* #ifndef _HWOUTPRELU_H_ */

/* EOF ########################################################################################## */
