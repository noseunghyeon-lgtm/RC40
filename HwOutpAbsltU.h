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

#ifndef _HWOUTPABSLTU_H_
#define _HWOUTPABSLTU_H_

#ifdef _HWOUTPABSLTU_C_
#define HWOUTPABSLTU_SCOPE_D
#else
#define HWOUTPABSLTU_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Output Absolute Voltage)       *
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
#define HwOutpAbsltU_ErrBitMaskRngChkPrm_DU16    0x0001u // Parameter range check error
#define HwOutpAbsltU_ErrBitMaskRngChkInpSig_DU16 0x0002u // Input signal range check error
#define HwOutpAbsltU_ErrBitMaskScb_DU16          0x0004u // DFC_rbr_out[K|A][XX]SCB - Short circuit to battery (SCB) at connector pin K[XX] or A[XX]
#define HwOutpAbsltU_ErrBitMaskScg_DU16          0x0008u // DFC_rbr_out[K|A][XX]SCG - Short circuit to ground (SCG) at connector pin K[XX] or A[XX]
#define HwOutpAbsltU_ErrBitMaskOutOfRng_DU16     0x0010u // DFC_rbr_out[K|A][XX]OR - Setpoint deviation (OR) at connector pin K[XX] or A[XX] - Only warning. Output is not locked!
#define HwOutpAbsltU_ErrBitMaskLockd_DU16        0x0100u // Output pin locked by error (see HwOutpCmn_ErrBitCmbnLockd_D)
#define HwOutpAbsltU_ErrBitMaskBswCallErr_DU16   0x4000u // BSW function call failed.

/* typedefs/structures *****************************************************************************
* Typedefs/structures to be made available to other modules.                                      */
typedef enum HwOutpAbsltU_ErrReactn_t
{
  HwOutpAbsltU_ErrReactnNorm_E = 0,        // No Reaction
  HwOutpAbsltU_ErrReactnPwrOff_E           // Set output value to zero
} HwOutpAbsltU_ErrReactn_te;

typedef enum HwOutpAbsltU_Fct_t
{
  HwOutpAbsltU_FctOff_E = 0,               // Output and error detection off
  HwOutpAbsltU_FctOn_E                     // Function on
} HwOutpAbsltU_Fct_te;

typedef struct HwOutpAbsltU_inp_t
{
  HwOutpAbsltU_ErrReactn_te stErrReactn_e; // Error reaction
  uint16 uAbslt_mV_u16;                    // Absolute output voltage setpoint; @physrange[0mV..10000mV]
} HwOutpAbsltU_inp_ts;

typedef struct HwOutpAbsltU_prm_t
{
  HwOutpAbsltU_Fct_te stFct_e;             // Channel type unused or used in application SW
  bds_out_aov_signal_te idPin_e;           // Pin identifier; @mrxml-ignore(rationale: assigned in HwOutp)
  uint16 uDe_mV_u16;                       // Maximum voltage deviation for output; @physrange[0mV..500mV]
} HwOutpAbsltU_prm_ts;

typedef struct HwOutpAbsltU_prv_t
{
  HwOutpAbsltU_Fct_te stFctIni_e;          // Store stFct during initialization
  bds_out_aov_ts stApi_s;                  // Channel information by API
} HwOutpAbsltU_prv_ts;

typedef struct HwOutpAbsltU_outp_t
{
  uint16 uMeasd_mV_u16;                   // Measured voltage at output
  uint16 stErrDetn_u16;                   // Error detection; @errdetnref(HwOutpAbsltU_ErrBitMask)
} HwOutpAbsltU_outp_ts;

typedef struct HwOutpAbsltU_t
{
  HwOutpAbsltU_inp_ts inp_s;
  HwOutpAbsltU_prm_ts prm_s;
  HwOutpAbsltU_prv_ts prv_s;
  HwOutpAbsltU_outp_ts outp_s;
} HwOutpAbsltU_ts;

/* public data *************************************************************************************
* Declarations of variables to be made available to other modules. Use SCOPE_D in front of each    *
* line.                                                                                           */
/* public functions ********************************************************************************
* Declarations of functions to be made available to other modules. Use SCOPE_D in front of each    *
* line.                                                                                           */
HWOUTPABSLTU_SCOPE_D void HwOutpAbsltU_ini (HwOutpAbsltU_ts *absltU_ps);  // Initialization of the output pin
HWOUTPABSLTU_SCOPE_D void HwOutpAbsltU(HwOutpAbsltU_ts *absltU_ps);  // Setting output pin to the desired value and reading the status of the pin and providing an error status information in the output structure.

/* ********************************************************************************************** */
#endif /* #ifndef _HWOUTPABSLTU_H_ */
/* EOF ########################################################################################## */
