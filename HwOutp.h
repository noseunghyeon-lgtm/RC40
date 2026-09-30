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

#ifndef _HWOUTP_H_
#define _HWOUTP_H_

#ifdef _HWOUTP_C_
#define HWOUTP_SCOPE_D
#else
#define HWOUTP_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Output)                        *
 * @suprtd-ecutype(RC5-6/40, RC18-12/40, RC27-18/40)     *
 * *******************************************************/

/* include files ***********************************************************************************
 * List of include files needed in this module.                                                    *
 * DO NOT INCLUDE HEADER FILES WITH VARIABLE DECLARATIONS HERE!                                    *
 * ONLY INCLUDES OF TYPEDEFS OR DEFINES ARE ALLOWED!                                               */

/* Bas includes */
#include "HwOutpCmn.h"
#include "HwOutpPropPwr.h"
#include "HwOutpPropSig.h"
#include "HwOutpDigSig.h"
#include "HwOutpDigPwr.h"
#include "HwOutpAbsltU.h"
#include "HwOutpRelU.h"
#include "HwOutpSfty.h"

/* defines *****************************************************************************************
 * List of defines needed in this header file and defines to be made available to other modules.  */
/* Functions that convert a pin number to the corresponding array index                           */
//PRQA S:"A function could probably be used instead of this function like macro" 3453 6 # Justification: Performance
#define HwOutp_getPinIdxPropPwr_DU16(pin) (uint16)((pin) - OUT_PO_START_SIGNALS    )
#define HwOutp_getPinIdxPropSig_DU16(pin) (uint16)((pin) - OUT_PO_FSC_NUM_SIGNALS  )
#define HwOutp_getPinIdxDigPwr_DU16(pin)  (uint16)((pin) - OUT_DO_START_SIGNALS    )
#define HwOutp_getPinIdxDigSig_DU16(pin)  (uint16)((pin) - OUT_DO_FSC_NUM_SIGNALS  )
#define HwOutp_getPinIdxRelU_DU16(pin)    (uint16)((pin) - OUT_AOV_START_SIGNALS   )
#define HwOutp_getPinIdxAbsltU_DU16(pin)  (uint16)((pin) - OUT_AOV_PVG_NUM_SIGNALS )

#define HwOutp_ErrBitMaskInvldPinTyp_DU8  ((uint8) 0x04u)  // error symptom for invalid pin/channel type used in pin/channel list

/* typedefs/structures *****************************************************************************
 * Typedefs/structures to be made available to other modules.                                      */

typedef enum HwOutp_OutpTyp_t
{
  HwOutp_OutpTypAbsltU_E,  // analog absolute voltage pin
  HwOutp_OutpTypDigPwr_E,  // digital power pin
  HwOutp_OutpTypDigSig_E,  // digital signal pin
  HwOutp_OutpTypPropPwr_E, // proportional power pin
  HwOutp_OutpTypPropSig_E, // proportional signal pin
  HwOutp_OutpTypRelU_E,    // analog relative voltage pin
  HwOutp_OutpTypSftyCh_E   // safout channel
} HwOutp_OutpTyp_te;

typedef struct HwOutp_PinChListEntry_t
{
  HwOutp_OutpTyp_te stTyp_e;      // output pin/channel type
  uint16            idxPinCh_u16; // index of pin/channel, used as identifier
} HwOutp_PinChListEntry_ts;

typedef enum HwOutp_Fct_t
{
  HwOutp_FctOff_E,    // Output and error detection off
  HwOutp_FctOn_E,     // Output and error detection on
} HwOutp_Fct_te;

typedef struct HwOutp_prm_t
{
  HwOutp_Fct_te stFct_e; // Function mode selector
} HwOutp_prm_ts;

typedef struct HwOutp_prv_t
{
  HwOutp_Fct_te stFctIni_e; /* Remember state on initialization */
  uint16 nrCallCfg_u16;     /* Number of time configuration function was called */
  uint16 nrCallSet_u16;     /* Number of time set function was called */
  HwOutpSfty_ts SftyCh_as[SAFOUT_CHNL_NUM_DU16]; /* Safout structures */
} HwOutp_prv_ts;

typedef struct HwOutp_outp_t
{
  uint8 stErrDetn_u8;
} HwOutp_outp_ts;

typedef struct HwOutp_t
{
  HwOutpPropPwr_ts    PropPwr_as[HwOutpCmn_nrPropPwrPins_DU16]; // Proportional power outputs
  HwOutpPropSig_ts    PropSig_as[HwOutpCmn_nrPropSigPins_DU16]; // Proportional signal outputs
  HwOutpDigPwr_ts     DigPwr_as[HwOutpCmn_nrDigPwrPins_DU16];   // Digital power outputs
  HwOutpDigSig_ts     DigSig_as[HwOutpCmn_nrDigSigPins_DU16];   // Digital signal outputs
  HwOutpRelU_ts       RelU_as[HwOutpCmn_nrRelUPins_DU16];       // Outputs relative to supply voltage
  HwOutpAbsltU_ts     AbsltU_as[HwOutpCmn_nrAbsltUPins_DU16];   // Analog voltage outputs
  HwOutp_prv_ts       prv_s;
  HwOutp_prm_ts       prm_s;
  HwOutp_outp_ts      outp_s;
} HwOutp_ts;

/* public data *************************************************************************************
* Declarations of variables to be made available to other modules. Use SCOPE_D in front of each    *
* line.                                                                                           */
HWOUTP_SCOPE_D HwOutp_ts HwOutp_s; /* Instantiation of top-level data structure */

/* public functions ********************************************************************************
* Declarations of functions to be made available to other modules. Use SCOPE_D in front of each    *
* line.                                                                                           */
HWOUTP_SCOPE_D void HwOutp_ini(void); // Initialization function for the outputs
HWOUTP_SCOPE_D void HwOutp(const HwOutp_PinChListEntry_ts *PinChList_cas, const uint16 nrPinChListLen_cu16); // Cyclic function for setting output values and for error detection

/* ********************************************************************************************** */

#endif /* #ifndef _HWOUTP_H_ */

/* EOF ########################################################################################## */
