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
#ifndef _HWINP_H_
#define _HWINP_H_

#ifdef _HWINP_C_
#define HWINP_SCOPE_D
#else
#define HWINP_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Input)                         *
 * @suprtd-ecutype(RC5-6/40, RC18-12/40, RC27-18/40)     *
 * *******************************************************/

/* include files ***********************************************************************************
 * List of include files needed in this module.                                                    *
 * DO NOT INCLUDE HEADER FILES WITH VARIABLE DECLARATIONS HERE!                                    *
 * ONLY INCLUDES OF TYPEDEFS OR DEFINES ARE ALLOWED!                                               */

/* Bsw includes */
#include "bds_in.h"
/* Bas includes */
#include "HwInpAnU.h"
#include "HwInpDig.h"
#include "HwInpAnI.h"
#include "HwInpFrqShnt.h"
#include "HwInpFrqStd.h"
#include "HwInpFrqPha.h"
#include "HwInpR.h"
#include "HwInpSent.h"

/* defines *****************************************************************************************
 * List of defines needed in this header file and defines to be made available to other modules.   */

// Macros to get the array index for a pin (e.g. HwInp_s.AnU_as[HwInp_GetPinIdxAnU_DU16(K59)])
//PRQA S:"A function could probably be used instead of this function like macro" 3453 8 # Justification: Performance
#define HwInp_getPinIdxDig_DU16(pin)     (uint16)((pin) - IN_DI_START_SIGNALS        ) // function like macro returning the array index of a digital input pin
#define HwInp_getPinIdxAnU_DU16(pin)     (uint16)((pin) - IN_AIV_START_SIGNALS       ) // function like macro returning the array index of a analog voltage input pin
#define HwInp_getPinIdxAnI_DU16(pin)     (uint16)((pin) - IN_FI_START_SIGNALS        ) // function like macro returning the array index of a analog current input pin
#define HwInp_getPinIdxFrqShnt_DU16(pin) (uint16)((pin) - BDS_IN_FI_AIC_NUM_SIGNALS_D) // function like macro returning the array index of a DSM capable frequency input pin
#define HwInp_getPinIdxFrqStd_DU16(pin)  (uint16)((pin) - IN_FI_DSM_NUM_SIGNALS      ) // function like macro returning the array index of a standard frequency input pin
#define HwInp_getPinIdxR_DU16(pin)       (uint16)((pin) - IN_RI_START_SIGNALS        ) // function like macro returning the array index of a resistance input pin
#define HwInp_getPinIdxSent_DU16(pin)    (uint16)((pin) - IN_SENT_START_SIGNALS      ) // function like macro returning the array index of a SENT input pin
#define HwInp_getChIdxFrqPha_DU16(ch)    (uint16)((ch)  - IN_PHASE_START_SIGNALS     ) // function like macro returning the array index of a frequency + phase input channel

// Macro to calculate number of each pin type configured in eC/FSP
#define HwInp_nrInpDigPins_DU16  (uint16)(IN_DI_NUM_SIGNALS           - IN_DI_START_SIGNALS        ) // number of available digital input pins
#define HwInp_nrInpAnUPins_DU16  (uint16)(IN_AIV_NUM_SIGNALS          - IN_AIV_START_SIGNALS       ) // number of available analog voltage input pins
#define HwInp_nrTotFrqPins_DU16  (uint16)(IN_FI_NUM_SIGNALS           - IN_FI_START_SIGNALS        ) // number of all available frequency input pins
#define HwInp_nrInpAnIPins_DU16  (uint16)(BDS_IN_FI_AIC_NUM_SIGNALS_D - IN_FI_START_SIGNALS        ) // number of available analog current input pins
#define HwInp_nrShntFrqPins_DU16 (uint16)(IN_FI_DSM_NUM_SIGNALS       - BDS_IN_FI_AIC_NUM_SIGNALS_D) // number of available DSM capable frequency input pins
#define HwInp_nrStdFrqPins_DU16  (uint16)(IN_FI_NUM_SIGNALS           - IN_FI_DSM_NUM_SIGNALS      ) // number of available standard frequency input pins
#define HwInp_nrInpRPins_DU16    (uint16)(IN_RI_NUM_SIGNALS           - IN_RI_START_SIGNALS        ) // number of available resistance input pins
#define HwInp_nrSentPins_DU16    (uint16)(IN_SENT_NUM_SIGNALS         - IN_SENT_START_SIGNALS      ) // number of available SENT input pins
#define HwInp_nrFrqPhaCh_DU16    (uint16)(IN_PHASE_NUM_SIGNALS        - IN_PHASE_START_SIGNALS     ) // number of available frequency + phase input channels

#define HwInp_ErrBitMaskInvldPinTyp_DU8     (uint8)0x01u  // error symptom for invalid pin type used in pin list

/* typedefs/structures *****************************************************************************
 * Typedefs/structures to be made available to other modules.                                      */

typedef enum HwInp_InpTyp_t
{
  HwInp_InpTypDig_E,      // digital input pin
  HwInp_InpTypAnU_E,      // analog voltage pin
  HwInp_InpTypAnI_E,      // analog current pin (analog pin with current measurement shunt)
  HwInp_InpTypR_E,        // resistance input pin
  HwInp_InpTypFrqShnt_E,  // DSM capable frequency or analog current pin (frequency pin with optional current measurement shunt)
  HwInp_InpTypFrqStd_E,   // standard frequencies input (frequency pin without current measurement shunt)
  HwInp_InpTypFrqPha_E,   // frequency + phase input channel
  HwInp_InpTypSent_E,     // SENT pin

} HwInp_InpTyp_te;


typedef struct HwInp_PinListEntry_t
{
    HwInp_InpTyp_te stTyp_e;  // input pin type
    uint16 idxPin_u16;        // index of pin, used as identifier of the pin

} HwInp_PinListEntry_ts;

typedef struct HwInp_prv_t
{
  uint16 nrCallIni_u16; // Number of calls of the function HwInp_ini
  uint16 nrCallMai_u16; // Number of calls of the function HwInp

} HwInp_prv_ts;

typedef struct HwInp_outp_t
{
  uint8 stErrDetn_u8;

} HwInp_outp_ts;

/* ********************************************************************************************** */
typedef struct HwInp_t
{
  HwInpDig_ts     Dig_as[HwInp_nrInpDigPins_DU16];      // Array of digital input pins
  HwInpAnU_ts     AnU_as[HwInp_nrInpAnUPins_DU16];      // Array of analog voltage pins
  HwInpAnI_ts     AnI_as[HwInp_nrInpAnIPins_DU16];      // Array of analog current pins (analog pins with current measurement shunt)
  HwInpFrqStd_ts  FrqStd_as[HwInp_nrStdFrqPins_DU16];   // Array of DSM capable frequency or analog current pins (frequency pins with optional current measurement shunt)
  HwInpFrqShnt_ts FrqShnt_as[HwInp_nrShntFrqPins_DU16]; // Array of standard frequencies inputs (frequency pins without current measurement shunt)
  HwInpFrqPha_ts  FrqPha_as[HwInp_nrFrqPhaCh_DU16];     // Array of frequency + phase input channels
  HwInpR_ts       R_as[HwInp_nrInpRPins_DU16];          // Array of resistance input pins
  HwInpSent_ts    Sent_as[HwInp_nrSentPins_DU16];       // Array of SENT pins
  HwInp_prv_ts    prv_s;
  HwInp_outp_ts   outp_s;
} HwInp_ts;

/* public data *************************************************************************************
 * Declarations of variables to be made available to other modules.                                *
 * Use HWINP_SCOPE_D in front of each line.                                                        */
HWINP_SCOPE_D HwInp_ts HwInp_s; /* Instantiation of top-level data structure */

/* public functions ********************************************************************************
 * Declarations of functions to be made available to other modules.                                *
 * Use HWINP_SCOPE_D in front of each line.                                                        */
HWINP_SCOPE_D void HwInp_ini(void); /* Initialization function for the inputs */
HWINP_SCOPE_D void HwInp(const HwInp_PinListEntry_ts* const PinList_as, const uint16 nrPinListLen_u16); /* Cyclic function for retrieving values from the inputs and for error detection */

/* *********************************************************************************************** */
#endif /* #ifndef _HWINP_H_ */
/* EOF ########################################################################################## */
