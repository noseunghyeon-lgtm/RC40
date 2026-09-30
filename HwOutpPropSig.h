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
 </BRHead> */

#ifndef _HWOUTPPROPSIG_H_
#define _HWOUTPPROPSIG_H_

#ifdef _HWOUTPPROPSIG_C_
#define HWOUTPPROPSIG_SCOPE_D
#else
#define HWOUTPPROPSIG_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Output Proportional Signal)    *
 * @suprtd-ecutype(RC5-6/40, RC18-12/40, RC27-18/40)     *
 * *******************************************************/

/* include files ***********************************************************************************
* List of include files needed in this module.                                                     *
* DO NOT INCLUDE HEADER FILES WITH VARIABLE DECLARATIONS HERE!                                     *
* ONLY INCLUDES OF TYPEDEFS OR DEFINES ARE ALLOWED!                                               */
/* API include files **************************************************************************** */
#include "HwOutpCmn.h"

/* defines *****************************************************************************************/
#define HwOutpPropSig_ErrBitMaskRngChkPrm_DU16         0x0001u // Parameter range check error
#define HwOutpPropSig_ErrBitMaskRngChkInpSig_DU16      0x0002u // Input signal range check error
#define HwOutpPropSig_ErrBitMaskScb_DU16               0x0004u // DFC_rbr_out[K|A][XX]SCB - Short circuit to battery (SCB) at connector pin K[XX] or A[XX]
#define HwOutpPropSig_ErrBitMaskScg_DU16               0x0008u // DFC_rbr_out[K|A][XX]SCG - Short circuit to ground (SCG) at connector pin K[XX] or A[XX]
#define HwOutpPropSig_ErrBitMaskOl_DU16                0x0020u // DFC_rbr_out[K|A][XX]OL - Open load (OL) at connector pin K[XX] or A[XX]
#define HwOutpPropSig_ErrBitMaskOt_DU16                0x0080u // DFC_rbr_out[K|A][XX]OT - Over temperature (OT) at connector pin K[XX] or A[XX]
#define HwOutpPropSig_ErrBitMaskLockd_DU16             0x0100u // Output pin locked by error (see HwOutpCmn_ErrBitCmbnLockd_D)
#define HwOutpPropSig_ErrBitMaskBswCallErr_DU16        0x4000u // BSW function call failed.

/* List of defines needed in this header file and defines to be made available to other modules.   */
/* typedefs/structures *****************************************************************************
 * Typedefs/structures to be made available to other modules.                                      */

// Error reaction mode
typedef enum HwOutpPropSig_ErrReactn_t
{
  HwOutpPropSig_ErrReactnNorm_E,  // Normal operation
  HwOutpPropSig_ErrReactnPwrOff_E // Set output setpoint to zero
} HwOutpPropSig_ErrReactn_te;

// Control type of output pin
typedef enum HwOutpPropSig_Fct_t
{
  HwOutpPropSig_FctOff_E,     // Output and error detection off
  HwOutpPropSig_FctPwm_E,     // Duty-cycle controlled proportional out
  HwOutpPropSig_FctDigCtrl_E  // Digitally controlled proportional out
} HwOutpPropSig_Fct_te;
/* ********************************************************************************************** */
// Input for proportional signal outputs (200 mA current max)
typedef struct HwOutpPropSig_inp_t
{
  HwOutpPropSig_ErrReactn_te stErrReactn_e; // Error reaction
  uint16 dutyCycSp_perml_u16;               // Setpoint dutycycle; @physrange[0perml..1000perml]
  bool flgSp_l;                             // Flag setpoint output
} HwOutpPropSig_inp_ts;

// Parameter for proportional signal outputs (200 mA current max)
typedef struct HwOutpPropSig_prm_t
{
  HwOutpPropSig_Fct_te stFct_e;   // Channel type unused or used in application SW
  bds_out_po_signal_te idPin_e;   // Pin identifier; @mrxml-ignore(rationale: assigned in HwOutp)
  uint32 tiPerd_us_u32;           // PWM period of output; @physrange[300us..31000us]
} HwOutpPropSig_prm_ts;

// Private for proportional signal outputs
typedef struct HwOutpPropSig_prv_t
{
  HwOutpPropSig_Fct_te stFctIni_e;  // Initial configured control type. Used for cyclic update of application SW
  bds_out_po_ts stApi_s;            // Channel information by API
} HwOutpPropSig_prv_ts;

// Output for proportional signal outputs (200 mA max current)
typedef struct HwOutpPropSig_outp_t
{
  uint16 dutyCycMeasd_perml_u16;   // Measured dutycycle; @physrange[0perml..1000perml]
  uint16 stErrDetn_u16;            // Error detection state for proportional signal output; @errdetnref(HwOutpPropSig_ErrBitMask)
} HwOutpPropSig_outp_ts;

typedef struct HwOutpPropSig_t
{
  HwOutpPropSig_inp_ts inp_s;
  HwOutpPropSig_prm_ts prm_s;
  HwOutpPropSig_prv_ts prv_s;
  HwOutpPropSig_outp_ts outp_s;
} HwOutpPropSig_ts;

/* public functions ********************************************************************************
 * Declarations of functions to be made available to other modules.                                *
 * Use HWOUTPPROPSIG_SCOPE_D in front of each line.               */
HWOUTPPROPSIG_SCOPE_D void HwOutpPropSig_ini(HwOutpPropSig_ts *propSig_ps); // Initialization of the output pin
HWOUTPPROPSIG_SCOPE_D void HwOutpPropSig(HwOutpPropSig_ts *propSig_ps);     // Setting output pin to the desired value and reading the status of the pin and providing an error status information in the output structure.

/* *********************************************************************************************** */
#endif /* #ifndef _HWOUTPPROPSIG_H_ */
/* EOF ########################################################################################## */
