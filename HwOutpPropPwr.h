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

#ifndef _HWOUTPPROPPWR_H_
#define _HWOUTPPROPPWR_H_

#ifdef _HWOUTPPROPPWR_C_
#define HWOUTPPROPPWR_SCOPE_D
#else
#define HWOUTPPROPPWR_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Output Proportional Power)     *
 * @suprtd-ecutype(RC5-6/40, RC18-12/40, RC27-18/40)     *
 * *******************************************************/

/* include files ***********************************************************************************
* List of include files needed in this module.                                                     *
* DO NOT INCLUDE HEADER FILES WITH VARIABLE DECLARATIONS HERE!                                     *
* ONLY INCLUDES OF TYPEDEFS OR DEFINES ARE ALLOWED!                                               */
/* API include files **************************************************************************** */
#include "bds_out.h"
#include "HwOutpCmn.h"

/* defines *****************************************************************************************/
/* List of defines needed in this header file and defines to be made available to other modules.   */
#define HwOutpPropPwr_ISpMin_mA_DU16  0u
#define HwOutpPropPwr_ISpMax_mA_DU16  4000u

// Error detection symptoms
#define HwOutpPropPwr_ErrBitMaskRngChkPrm_DU16         0x0001u // Parameter range check error
#define HwOutpPropPwr_ErrBitMaskRngChkInpSig_DU16      0x0002u // Input signal range check error
#define HwOutpPropPwr_ErrBitMaskScb_DU16               0x0004u // DFC_rbr_out[K|A][XX]SCB - Short circuit to battery (SCB) at connector pin K[XX] or A[XX]
#define HwOutpPropPwr_ErrBitMaskScg_DU16               0x0008u // DFC_rbr_out[K|A][XX]SCG - Short circuit to ground (SCG) at connector pin K[XX] or A[XX]
#define HwOutpPropPwr_ErrBitMaskOutOfRng_DU16          0x0010u // DFC_rbr_out[K|A][XX]OR - Setpoint deviation (OR) at connector pin K[XX] or A[XX] - Only warning. Output is not locked!
#define HwOutpPropPwr_ErrBitMaskOl_DU16                0x0020u // DFC_rbr_out[K|A][XX]OL - Open load (OL) at connector pin K[XX] or A[XX]
#define HwOutpPropPwr_ErrBitMaskOc_DU16                0x0040u // DFC_rbr_out[K|A][XX]OC - Over current (OC) at connector pin K[XX] or A[XX]
#define HwOutpPropPwr_ErrBitMaskOt_DU16                0x0080u // DFC_rbr_out[K|A][XX]OT - Over temperature (OT) at connector pin K[XX] or A[XX]
#define HwOutpPropPwr_ErrBitMaskLockd_DU16             0x0100u // Output pin locked by error (see HwOutpCmn_ErrBitCmbnLockd_D)
#define HwOutpPropPwr_ErrBitMaskPwrSplyFaild_DU16      0x0200u // supply mode configuration failed (reported by BSW call)
#define HwOutpPropPwr_ErrBitMaskSftyLockdOutpErr_DU16  0x0400u // DFC_rbr_safoutChnl[nrSftyCh_u8] - Safety channel [nrSftyCh_u8] locked by output error
#define HwOutpPropPwr_ErrBitMaskSftyLockdIDev_DU16     0x0800u // DFC_rbr_safoutChnl[nrSftyCh_u8]Dev - Safety channel [nrSftyCh_u8] locked by current deviation high/low side
#define HwOutpPropPwr_ErrBitMaskSftySetErr_DU16        0x1000u // Safety channel [nrSftyCh_u8] call to set outp failed
#define HwOutpPropPwr_ErrBitMaskBthSwilPinsActv_DU16   0x2000u // Both normal switchable pins are active.
#define HwOutpPropPwr_ErrBitMaskBswCallErr_DU16        0x4000u // BSW function call failed.

/* typedefs/structures *****************************************************************************
 * Typedefs/structures to be made available to other modules.                                      */
typedef enum HwOutpPropPwr_ErrReactn_t
{
  HwOutpPropPwr_ErrReactnNorm_E,   // Normal operation
  HwOutpPropPwr_ErrReactnPwrOff_E  // Set output setpoint to zero
} HwOutpPropPwr_ErrReactn_te;

// Control type of output pin
typedef enum HwOutpPropPwr_Fct_t
{
  HwOutpPropPwr_FctOff_E,      // Output and error detection off
  HwOutpPropPwr_FctPwm_E,      // Duty-cycle controlled proportional out
  HwOutpPropPwr_FctICtrl_E,    // Current controlled proportional out
  HwOutpPropPwr_FctDigCtrl_E,  // Digitally controlled proportional out
  HwOutpPropPwr_FctPwrSply_E   // Power supply of low side output
} HwOutpPropPwr_Fct_te;

// Configuration of dither frequency
typedef enum HwOutpPropPwr_FrqDthr_t
{
  HwOutpPropPwr_FrqDthr0Hz_E,    // Dither frequency 0Hz
  HwOutpPropPwr_FrqDthr83Hz_E,   // Dither frequency 83Hz
  HwOutpPropPwr_FrqDthr90Hz_E,   // Dither frequency 90Hz
  HwOutpPropPwr_FrqDthr100Hz_E,  // Dither frequency 100Hz
  HwOutpPropPwr_FrqDthr111Hz_E,  // Dither frequency 111Hz
  HwOutpPropPwr_FrqDthr125Hz_E,  // Dither frequency 125Hz
  HwOutpPropPwr_FrqDthr142Hz_E,  // Dither frequency 142Hz
  HwOutpPropPwr_FrqDthr166Hz_E,  // Dither frequency 166Hz
  HwOutpPropPwr_FrqDthr200Hz_E,  // Dither frequency 200Hz
  HwOutpPropPwr_FrqDthr250Hz_E   // Dither frequency 250Hz
} HwOutpPropPwr_FrqDthr_te;

/* ********************************************************************************************** */

// Input for proportional power outputs (4 A current max)
typedef struct HwOutpPropPwr_inp_t
{
  HwOutpPropPwr_ErrReactn_te stErrReactn_e; // Error reaction state
  uint16 iSp_mA_u16;                        // Setpoint current; @physrange[0mA..4000mA]
  uint16 dutyCycSp_perml_u16;               // Setpoint dutycycle; @physrange[0perml..1000perml]
  bool flgSp_l;                             // Flag setpoint output
} HwOutpPropPwr_inp_ts;

// Parameter for proportional power outputs (4 A current max)
typedef struct HwOutpPropPwr_prm_t
{
  HwOutpPropPwr_Fct_te stFct_e;       // Selected function type
  bds_out_po_signal_te idPin_e;       // Pin identifier; @mrxml-ignore(rationale: assigned in HwOutp)
  uint16 facPidKp_u16;                // Proportional factor for output current loop control; @rawrange[0..2000]
  uint16 facPidKi_u16;                // Integral factor for output current loop control; @rawrange[0..500]
  uint16 tiPerd_ms_u16;               // PWM period of output; @physrange[0ms..100ms]
  HwOutpPropPwr_FrqDthr_te frqDthr_e; // Selection of dither frequency
  uint16 iDthrAmp_mA_u16;             // PWM dither amplitude; @physrange[0mA..500mA]
  uint16 iDe_mA_u16;                  // Current deviation; @physrange[0mA..500mA]
  bool diTestPls_l;                   // If the test pulses should be disabled.
  HwOutpCmn_SftyMod_te stFctSfty_e;   // Set safety channel mode, default HwOutpCmn_SftyModOff_E
  uint8 nrSftyCh_u8;                  // Set desired safety channel number
} HwOutpPropPwr_prm_ts;

// Private for proportional power outputs
typedef struct HwOutpPropPwr_prv_t
{
  HwOutpPropPwr_Fct_te stFctIni_e;   // Initial configured control type. Used for cyclic update of application SW
  bds_out_po_ts stApi_s;             // Channel information by API
} HwOutpPropPwr_prv_ts;

// Output for proportional power outputs (4 A max current)
typedef struct HwOutpPropPwr_outp_t
{
  uint16 iMeasd_mA_u16;              // Measured current; @physrange[0mA..4000mA]
  uint16 dutyCycMeasd_perml_u16;     // Measured dutycycle; @physrange[0perml..1000perml]
  uint16 stErrDetn_u16;              // State error detection; @errdetnref(HwOutpPropPwr_ErrBitMask)
} HwOutpPropPwr_outp_ts;

typedef struct HwOutpPropPwr_t
{
  HwOutpPropPwr_inp_ts inp_s;
  HwOutpPropPwr_prm_ts prm_s;
  HwOutpPropPwr_prv_ts prv_s;
  HwOutpPropPwr_outp_ts outp_s;
} HwOutpPropPwr_ts;

/* public functions ********************************************************************************
 * Declarations of functions to be made available to other modules.                                *
 * Use HWOUTPPROPPWR_SCOPE_D in front of each line.               */
HWOUTPPROPPWR_SCOPE_D void HwOutpPropPwr_ini ( HwOutpPropPwr_ts *propPwr_ps); // Initialization of the output pin.
HWOUTPPROPPWR_SCOPE_D void HwOutpPropPwr(HwOutpPropPwr_ts *propPwr_ps);       // Setting output pin to the desired value and reading the status of the pin and providing an error status information in the output structure.

/* *********************************************************************************************** */

#endif /* #ifndef _HWOUTPPROPPWR_H_ */

/* EOF ########################################################################################## */
