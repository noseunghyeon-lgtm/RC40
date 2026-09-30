/*<BRHead>
 ***************************************************************************************************
 *
 * Copyright © 2020 - 2023 Bosch Rexroth AG
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

#ifndef _HWOUTPDIGPWR_
#define _HWOUTPDIGPWR_

#ifdef _HwOutpDigPwr_C_
#define HWOUTPDIGPWR_SCOPE_D
#else
#define HWOUTPDIGPWR_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Output Digital Power)          *
 * @suprtd-ecutype(RC5-6/40, RC18-12/40, RC27-18/40)     *
 * *******************************************************/

/* include files ***********************************************************************************
 * List of include files needed in this module.                                                    *
 * DO NOT INCLUDE HEADER FILES WITH VARIABLE DECLARATIONS HERE!                                    *
 * ONLY INCLUDES OF TYPEDEFS OR DEFINES ARE ALLOWED!                                               */
/* API include files ***************************************************************************** */
#include "HwOutpCmn.h"

/* defines *****************************************************************************************
 * List of defines needed in this header file and defines to be made available to other modules.   */
// Error detection symptoms
#define HwOutpDigPwr_ErrBitMaskRngChkPrm_DU16         0x0001u // Parameter range check error
#define HwOutpDigPwr_ErrBitMaskScb_DU16               0x0004u // DFC_rbr_out[K|A][XX]SCB - Short circuit to battery (SCB) at connector pin K[XX] or A[XX]
#define HwOutpDigPwr_ErrBitMaskScg_DU16               0x0008u // DFC_rbr_out[K|A][XX]SCG - Short circuit to ground (SCG) at connector pin K[XX] or A[XX]
#define HwOutpDigPwr_ErrBitMaskOl_DU16                0x0020u // DFC_rbr_out[K|A][XX]OL - Open load (OL) at connector pin K[XX] or A[XX]
#define HwOutpDigPwr_ErrBitMaskOc_DU16                0x0040u // DFC_rbr_out[K|A][XX]OC - Over current (OC) at connector pin K[XX] or A[XX]
#define HwOutpDigPwr_ErrBitMaskOt_DU16                0x0080u // DFC_rbr_out[K|A][XX]OT - Over temperature (OT) at connector pin K[XX] or A[XX]
#define HwOutpDigPwr_ErrBitMaskLockd_DU16             0x0100u // output pin locked by error (see HwOutpCmn_ErrBitCmbnLockd_D)
#define HwOutpDigPwr_ErrBitMaskPwrSplyFaild_DU16      0x0200u // Supply mode configuration failed or not supported
#define HwOutpDigPwr_ErrBitMaskSftyLockdOutpErr_DU16  0x0400u // DFC_rbr_safoutChnl[nrSftyCh_u8] - Safety channel [nrSftyCh_u8] locked by output error
#define HwOutpDigPwr_ErrBitMaskSftyLockdIDev_DU16     0x0800u // DFC_rbr_safoutChnl[nrSftyCh_u8]Dev - Safety channel [nrSftyCh_u8] locked by current deviation high/low side
#define HwOutpDigPwr_ErrBitMaskSftySetErr_DU16        0x1000u // Safety channel [nrSftyCh_u8] call to set output failed
#define HwOutpDigPwr_ErrBitMaskBthSwilPinsActv_DU16   0x2000u // Both normal switchable pins are active.
#define HwOutpDigPwr_ErrBitMaskBswCallErr_DU16        0x4000u // BSW function call failed.

/* typedefs/structures *****************************************************************************
 * Typedefs/structures to be made available to other modules.                                      */
typedef enum HwOutpDigPwr_ErrReactn_t
{
  HwOutpDigPwr_ErrReactnNorm_E,            // Normal operation
  HwOutpDigPwr_ErrReactnPwrOff_E           // Set output setpoint to zero
} HwOutpDigPwr_ErrReactn_te;

typedef enum HwOutpDigPwr_Fct_t
{
  HwOutpDigPwr_FctOff_E,                   // Output and error detection off
  HwOutpDigPwr_FctDigCtrl_E,               // Digitally controlled output
  HwOutpDigPwr_FctPwrSply_E,               // Power supply for low side output (only RC27-18/40 and RC18-12/40)
} HwOutpDigPwr_Fct_te;

typedef struct HwOutpDigPwr_inp_t
{
  HwOutpDigPwr_ErrReactn_te stErrReactn_e; // Error reaction state
  bool flgSp_l;                            // Flag setpoint output
} HwOutpDigPwr_inp_ts;

typedef struct HwOutpDigPwr_prm_t
{
  HwOutpDigPwr_Fct_te stFct_e;             // Selected function type
  bds_out_do_signal_te idPin_e;            // Pin identifier; @mrxml-ignore(rationale: assigned in HwOutp)
  bool diTestPls_l;                        // If the test pulses should be disabled.
  HwOutpCmn_SftyMod_te stFctSfty_e;        // Set safety channel mode, default HwOutpCmn_SftyModOff_E
  uint8 nrSftyCh_u8;                       // Set desired safety channel number
} HwOutpDigPwr_prm_ts;

typedef struct HwOutpDigPwr_prv_t
{
  HwOutpDigPwr_Fct_te stFctIni_e;          // Initial configured control type. Used for cyclic update of application SW
  bds_out_do_ts stApi_s;                   // Channel information by API
} HwOutpDigPwr_prv_ts;

typedef struct HwOutpDigPwr_outp_t
{
  bool flgMeasd_l;                         // Measured state
  uint16 iMeasd_mA_u16;                    // Measured current
  uint16 stErrDetn_u16;                    // State error detection; @errdetnref(HwOutpDigPwr_ErrBitMask)
} HwOutpDigPwr_outp_ts;

typedef struct HwOutpDigPwr_t
{
  HwOutpDigPwr_inp_ts  inp_s;
  HwOutpDigPwr_prm_ts  prm_s;
  HwOutpDigPwr_prv_ts  prv_s;
  HwOutpDigPwr_outp_ts outp_s;
} HwOutpDigPwr_ts;

/* public data *************************************************************************************
 * Declarations of variables to be made available to other modules. Use SCOPE_D in front of each   *
 * line.                                                                                           */

/* public functions ********************************************************************************
 * Declarations of functions to be made available to other modules. Use SCOPE_D in front of each   *
 * line.                                                                                           */
HWOUTPDIGPWR_SCOPE_D void HwOutpDigPwr_ini(HwOutpDigPwr_ts *digPwr_ps); /* Initialization of the output pin. */
HWOUTPDIGPWR_SCOPE_D void HwOutpDigPwr(HwOutpDigPwr_ts *digPwr_ps);     // Setting output pin to the desired value and reading the status of the pin and providing an error status information in the output structure.

/* *********************************************************************************************** */

#endif /* #ifndef _HWOUTPDIGPWR_ */

/* EOF ########################################################################################### */
