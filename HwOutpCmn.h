/*<BRHead>
 ***************************************************************************************************
 *
 * Copyright © 2019 - 2022 Bosch Rexroth AG
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

/* QA-C suppression block **************************************************************************/
/* disable QAC rule 3469 in entire file because all used function-like macros are accepted by performance reasons. */
/* PRQA S:"All toplevel uses of this function-like macro look like they could be replaced by equivalent function calls." 3472 EOF # Justification: Performance */
/* *************************************************************************************************/

#ifndef _HWOUTPCMN_H_
#define _HWOUTPCMN_H_

#ifdef _HWOUTPCMN_C
#define HWOUTPCMN_SCOPE_D
#else
#define HWOUTPCMN_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Output Common)                 *
 * @suprtd-ecutype(RC5-6/40, RC18-12/40, RC27-18/40)     *
 * *******************************************************/

/* include files ***********************************************************************************
* List of include files needed in this module.                                                     *
* DO NOT INCLUDE HEADER FILES WITH VARIABLE DECLARATIONS HERE!                                     *
* ONLY INCLUDES OF TYPEDEFS OR DEFINES ARE ALLOWED!                                               */
/* API include files **************************************************************************** */
#include "bds_out.h"

/* defines *****************************************************************************************
* List of defines needed in this header file and defines to be made available to other modules.   */
// DFC status evaluation
// Check if Test Status (TS) and Last Debounced Fault (LDF) bits in the DFC status word are set.
// TS is 1 and LDF is 1 -> fault is active
// TS is 0 and LDF is 1 -> fault is previously active
// TS is 1 and LDF is 0 -> fault is tested inactive
// TS is 0 and LDF is 0 -> fault event never occurred or cyclic test is not complete
#define HwOutpCmn_DiagLstDebFltMask_DU16   0x2010u

// Combined error statе from status_b16 (...getStatus()).
#define HwOutpCmn_ErrBitCmbnLockd_DU16  (lockedSCB_DB16 | lockedSCG_DB16 | lockedOC_DB16 | locked_DB16 | lockedMO_DB16 | lockCouTiReact_DB16)

// Macro to calculate number of each pin type
#define HwOutpCmn_nrPropPwrPins_DU16 (uint16)(OUT_PO_FSC_NUM_SIGNALS  - OUT_PO_START_SIGNALS   ) // Number of proportional power outputs
#define HwOutpCmn_nrPropSigPins_DU16 (uint16)(OUT_PO_NUM_SIGNALS      - OUT_PO_FSC_NUM_SIGNALS ) // Number of proportional signal outputs
#define HwOutpCmn_nrDigPwrPins_DU16  (uint16)(OUT_DO_FSC_NUM_SIGNALS  - OUT_DO_START_SIGNALS   ) // Number of digital power outputs
#define HwOutpCmn_nrDigSigPins_DU16  (uint16)(OUT_DO_NUM_SIGNALS      - OUT_DO_FSC_NUM_SIGNALS ) // Number of digital signal outputs
#define HwOutpCmn_nrRelUPins_DU16    (uint16)(OUT_AOV_PVG_NUM_SIGNALS - OUT_AOV_START_SIGNALS  ) // Number of proportional voltage outputs (PVG)
#define HwOutpCmn_nrAbsltUPins_DU16  (uint16)(OUT_AOV_NUM_SIGNALS     - OUT_AOV_PVG_NUM_SIGNALS) // Number of analog voltage outputs (AOV)

// check if BSW function call returned Invalid Parameter or Signal
#define HwOutpCmn_setFlgOnInvldPrmOrSig(flg_pl, fctRes_cu8) \
{ \
  /* invalid parameter BSW API return value is everywhere and always uint8 and 1 */ \
  if (1u == (fctRes_cu8)) \
  { \
    *(flg_pl) = TRUE; \
  } \
  /* otherwise nothing, keep previous state of flag */ \
}

/* typedefs/structures *****************************************************************************
* Typedefs/structures to be made available to other modules.                                      */
// structure used by PropPwr, DigPwr and Sfty instances
typedef enum {
  HwOutpCmn_SftyModOff_E,               // Safout mode off
  HwOutpCmn_SftyModPropHiDigLoSwil_E,   // Safout mode Proportional high - digital low switchable (PHDLS)
  HwOutpCmn_SftyModDigHiDigLoMpl_E,     // Digital high - digital low multiple (DHDLM)
  HwOutpCmn_SftyModPropHiMplDigLo_E,    // Proportional high multiple - digital low (PHMDL)
  HwOutpCmn_SftyModPropLoDigHiSwil_E,   // Proportional Low - Digital High Switchable (PLDHS)
  HwOutpCmn_SftyModDigLoDigHiMpl_E,     // Digital Low - Digital High Multiple (DLDHM)
  HwOutpCmn_SftyModPropLoMplDigHi_E     // Proportional Low Multiple - Digital High (PLMDH)
} HwOutpCmn_SftyMod_te;

/* public functions ********************************************************************************
* Declarations of functions to be made available to other modules. Use SCOPE_D in front of each    *
* line.                                                                                           */
HWOUTPCMN_SCOPE_D bool HwOutpCmn_u16RngChkLim(uint16 *tmp_u16, uint16 min_u16, uint16 max_u16); // range check for u16
HWOUTPCMN_SCOPE_D bool HwOutpCmn_u32RngChkLim(uint32 *tmp_u32, uint32 min_u32, uint32 max_u32); // range check for u32
HWOUTPCMN_SCOPE_D bool HwOutpCmn_chkFltActv(const uint16 stDiagFltChk_cu16);                    // check if fault is active
HWOUTPCMN_SCOPE_D uint8 HwOutpCmn_flg2DigSt(const bool flg_cl);                                 // flag to digital state
HWOUTPCMN_SCOPE_D uint8 HwOutpCmn_flg2DigStOnOff(const bool flg_cl);                            // flag to On/Off state
HWOUTPCMN_SCOPE_D bool HwOutpCmn_digSt2Flg(uint8 stDig_u8);                                     // digital state to flag

#endif /* #ifndef _HWOUTPCMN_H_ */
/* EOF ########################################################################################## */
