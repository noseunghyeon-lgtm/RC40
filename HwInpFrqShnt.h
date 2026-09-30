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
#ifndef _HWINPFRQSHNT_H_
#define _HWINPFRQSHNT_H_

#ifdef _HWINPFRQSHNT_C_
#define HWINPFRQSHNT_SCOPE_D
#else
#define HWINPFRQSHNT_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Input Frequency Shunt)         *
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
#define HwInpFrqShnt_ErrBitMaskShnt_DU16           0x0001u /* DFC_rbr_in[K|A][XX]ShuntSCB - Shunt error (disabled) at connector pin K[XX] or A[XX] */
#define HwInpFrqShnt_ErrBitMaskDsmAirGap_DU16      0x0004u /* Air gap too big (1x LR pulse width == 45us) - air gap error (pulse is lower than 64us). This error is only evaluated if the measured frequency is above 5 Hz (only detected when DSM sensor is in use). */
#define HwInpFrqShnt_ErrBitMaskDsmMount_DU16       0x0008u /* Mounting error (EL == 8x or 16x LR pulse width) - mounting error (pulse is greater than 254us). This error is only evaluated if the measured frequency is between 5 Hz and 115 Hz (only detected when DSM sensor is in use). */
#define HwInpFrqShnt_ErrBitMaskSnsrSigLen_DU16     0x0010u /* Unexpected pulse length */
#define HwInpFrqShnt_ErrBitMaskDsmPerdAbvMax_DU16  0x0020u /* Period too long (700 ms is max for standstill) - period too long or measured period exceeds maximum period (only detected when DSM sensor is in use). */
#define HwInpFrqShnt_ErrBitMaskSnsrSpdAbvMax_DU16  0x0040u /* Speed too high (no direction or diagnose information) */
#define HwInpFrqShnt_ErrBitMaskFctCallErr_DU16     0x0080u /* Function call was erroneous */
#define HwInpFrqShnt_ErrBitMaskSnsrScgOl_DU16      0x0100u /* Short circuit to ground or open-load */
#define HwInpFrqShnt_ErrBitMaskPerdAbvMax_DU16     0x0200u /* Measured period exceeds maximum period length (as configured in easyConfig). */
#define HwInpFrqShnt_ErrBitMaskDutyCycAbvMax_DU16  0x0400u // Current duty cycle returned from API is greater than 1000perml, output is limited to 1000perml

/* typedefs/structures *****************************************************************************
 * Typedefs/structures to be made available to other modules.                                      */
typedef enum HwInpFrqShnt_stDir_t
{
  HwInpFrqShnt_stStandStill_E = 0, // Standstill
  HwInpFrqShnt_stDirLe_E = 1,      // Direction left
  HwInpFrqShnt_stDirRi_E = 2       // Direction right
} HwInpFrqShnt_stDir_te;

/* *********************************************************************************************** */
typedef struct HwInpFrqShnt_prv_t
{
  bool flgActvSnsr_l;              // Flag indicate the indexed pin is configured for DSM/DST sensor
  bds_in_fi_ts  stApi_s;           // API structure for frequency pins

  uint32 tiPerdMemd_us_u32;        // Memorized period from a previous cycle
  bool   flgPerdMemAvl_l;          // Flag indicating whether the period memory is available
  uint32 frqOrig_p1Hz_u32;         // frequency input signal from bds_in_fi_getStatus API (original/reference for debugging)
  uint32 cntrPerdPrev_u32;         // Previous period count value from last call
  uint8  nrUsedPerd_u8;            // Number of period samples used for averaging

} HwInpFrqShnt_prv_ts;

/* *********************************************************************************************** */
typedef struct HwInpFrqShnt_prm_t
{
  bds_in_fi_signal_te idPin_e;     // Pin Identifier; @rawrange[0x700..0x7FF]; @mrxml-ignore(rationale: assigned in HwInp)
  bool flgDstSnsr_l;               // DST Sensor in use
  bool flgAvrgFrq_l;               // Averaging frequency with bds_in_fi_buffered() API if available

} HwInpFrqShnt_prm_ts;

/* *********************************************************************************************** */
typedef struct HwInpFrqShnt_outp_t
{
  uint32 frq_p1Hz_u32;             // Frequency input value; @physrange[0Hz..20000Hz]
  uint32 cntrPerd_u32;             // Frequency input period counter value
  HwInpFrqShnt_stDir_te stDir_e;   // Rotation direction of DSM/DST sensor
  uint16 i_uA_u16;                 // Analog current input value; @physrange[0uA..25000uA]
  uint16 stErrDetn_u16;            // Error detection state of DSM and DST capable input, @errdetnref(HwInpFrqShnt_ErrBitMask)
  uint32 tiPls_us_u32;             // Last high time duration in µs
  uint32 tiPerd_us_u32;            // Period duration in µs
  uint16 ratDutyCyc_perml_u16;     // Current duty cycle; @physrange[0perml..1000perml]

} HwInpFrqShnt_outp_ts;

/* *********************************************************************************************** */
typedef struct HwInpFrqShnt_t
{
  HwInpFrqShnt_prm_ts  prm_s;
  HwInpFrqShnt_prv_ts  prv_s;
  HwInpFrqShnt_outp_ts outp_s;
} HwInpFrqShnt_ts;

/* public data *************************************************************************************
 * Declarations of variables to be made available to other modules.                                */

/* public functions ********************************************************************************
 * Declarations of functions to be made available to other modules.                                *
 * Use HWINPFRQSHNT_SCOPE_D in front of each line.                                                 */
HWINPFRQSHNT_SCOPE_D void HwInpFrqShnt_ini(HwInpFrqShnt_ts *frqShnt_ps); // Initialize frequency input pin with shunt resistor PinId_e and save configuration in a structure frqShnt_ps
HWINPFRQSHNT_SCOPE_D void HwInpFrqShnt(HwInpFrqShnt_ts *frqShnt_ps);     // Get values from frequency input pin with shunt resistor PinId_e and copy results to structure frqShnt_ps

/* *********************************************************************************************** */
#endif /* #ifndef _HWINPFRQSHNT_H_ */
/* EOF ########################################################################################### */
