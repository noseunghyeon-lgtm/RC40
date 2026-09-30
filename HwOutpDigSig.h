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

#ifndef _HWOUTPDIGSIG_H_
#define _HWOUTPDIGSIG_H_

#ifdef _HWOUTPDIG_C_
#define HWOUTPDIGSIG_SCOPE_D
#else
#define HWOUTPDIGSIG_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Output Digital Signal)         *
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
#define HwOutpDigSig_ErrBitMaskScb_DU16               0x0004u // DFC_rbr_out[K|A][XX]SCB - Short circuit to battery (SCB) at connector pin K[XX] or A[XX]
#define HwOutpDigSig_ErrBitMaskScg_DU16               0x0008u // DFC_rbr_out[K|A][XX]SCG - Short circuit to ground (SCG) at connector pin K[XX] or A[XX]
#define HwOutpDigSig_ErrBitMaskOl_DU16                0x0020u // DFC_rbr_out[K|A][XX]OL - Open load (OL) at connector pin K[XX] or A[XX]
#define HwOutpDigSig_ErrBitMaskOt_DU16                0x0080u // DFC_rbr_out[K|A][XX]OT - Over temperature (OT) at connector pin K[XX] or A[XX]
#define HwOutpDigSig_ErrBitMaskLockd_DU16             0x0100u // Output pin locked by error (see HwOutpCmn_ErrBitCmbnLockd_D)
#define HwOutpDigSig_ErrBitMaskBswCallErr_DU16        0x4000u // BSW function call failed.

/* typedefs/structures *****************************************************************************
* Typedefs/structures to be made available to other modules.                                      */
typedef enum HwOutpDigSig_ErrReactn_t
{
  HwOutpDigSig_ErrReactnNorm_E,            // Normal operation
  HwOutpDigSig_ErrReactnPwrOff_E           // Set output setpoint to zero
} HwOutpDigSig_ErrReactn_te;

typedef enum HwOutpDigSig_Fct_t
{
  HwOutpDigSig_FctOff_E,                   // Output and error detection off
  HwOutpDigSig_FctOn_E,                    // Output and error detection on
} HwOutpDigSig_Fct_te;

typedef struct HwOutpDigSig_inp_t
{
  HwOutpDigSig_ErrReactn_te stErrReactn_e; // Error reaction state
  bool flgSp_l;                            // Flag setpoint output
} HwOutpDigSig_inp_ts;

typedef struct HwOutpDigSig_prm_t
{
  HwOutpDigSig_Fct_te stFct_e;             // Selected function type
  bds_out_do_signal_te idPin_e;            // Pin identifier; @mrxml-ignore(rationale: assigned in HwOutp)
} HwOutpDigSig_prm_ts;

typedef struct HwOutpDigSig_prv_t
{
  HwOutpDigSig_Fct_te stFctIni_e;          // Initial configured control type. Used for cyclic update of application SW
  bds_out_do_ts stApi_s;                   // Channel information by API
} HwOutpDigSig_prv_ts;

typedef struct HwOutpDigSig_outp_t
{
  bool flgMeasd_l;                         // Measured state
  uint16 stErrDetn_u16;                    // State error detection; @errdetnref(HwOutpDigSig_ErrBitMask)
} HwOutpDigSig_outp_ts;

typedef struct HwOutpDigSig_t
{
  HwOutpDigSig_inp_ts  inp_s;
  HwOutpDigSig_prm_ts  prm_s;
  HwOutpDigSig_prv_ts  prv_s;
  HwOutpDigSig_outp_ts outp_s;
} HwOutpDigSig_ts;

/* public data *************************************************************************************
* Declarations of variables to be made available to other modules. Use SCOPE_D in front of each    *
* line.                                                                                */

/* public functions ********************************************************************************
* Declarations of functions to be made available to other modules. Use SCOPE_D in front of each    *
* line.                                                                                           */
HWOUTPDIGSIG_SCOPE_D void HwOutpDigSig_ini(HwOutpDigSig_ts *digSig_ps); // Initialization of the output pin.
HWOUTPDIGSIG_SCOPE_D void HwOutpDigSig(HwOutpDigSig_ts *digSig_ps);     // Setting output pin to the desired value and reading the status of the pin and providing an error status information in the output structure.

/* ********************************************************************************************** */

#endif /* #ifndef _HWOUTPDIGSIG_H_ */

/* EOF ########################################################################################## */
