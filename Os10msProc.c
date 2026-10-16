/*<BRHead>
 ***************************************************************************************************
 *
 * Copyright © 2021 - 2024 Bosch Rexroth AG
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

#ifndef _OS10MSPROC_C_
#define _OS10MSPROC_C_

#define Os10msProc_Use_D // xxx multitasking - Disable: comment this line to mute Os10msProc

/* include files ***********************************************************************************
 * List of include files needed in the scope of this c-file.                                       */

/* Bsw includes */
#include "bds_bsw.h"
#include "bds_uds.h"

/* Bas includes */
#include "OsCmn.h"
#include "Bas.h"
#include "CanRx.h"
#include "CanTx.h"
#include "DbgMgr.h"
#include "DbgMgrIntegn.h"
#include "EcuHlth.h"
#include "EcuMainRels.h"
#include "ErrMemMgr.h"
#include "ErrMgr.h"
#include "ErrReactn.h"
#include "ErrReactnTwoStep.h"
#include "HwInp.h"
#include "HwOutp.h"
#include "J1939Rx.h"
#include "J1939Tx.h"
#include "PrmCfg.h"
#include "PrmUpd.h"
#include "PrmMgr.h"
#include "UdsSafeSt.h"

/* other includes */
#include "Cust.h"
#include "DevInp.h"
#include "DevOutp.h"
#include "Veh.h"

/* own include */
#include "Os10msProc.h"

/* defines *****************************************************************************************
 * List of defines needed in the scope of this c-file, but not needed by other components.         */

/* typedefs/structures *****************************************************************************
 * Typedefs/structures needed in the scope of this c-file, but not needed by other components.     */

/* static data *************************************************************************************
 * Instantiation of data structures needed in the scope of this c-file.                            */

/* HwInp pin configuration: add your list of pins to be processed here.                       *
 * This list and all other HwInp pin lists in all other OS processes need to be                    *
 * reviewed and checked for:                                                                       *
 * - every pin is only contained once in all lists                                                 *
 * - every pin configured in easyConfig must be contained in one list                              *
 *                                                                                                 *
 * If you need to process all configured pins in only 1 OS process, use cyclic function call       *
 * HwInp(NULL, 0u); in the corresponding OS process.                                               *
 * this ensures all configured pins are processed and you don't need to specify such list of pins  *
 * explicitly.                                                                                     *
 * Ensure that you are not calling HwInp(NULL, 0u) from one OS process and HwInp with a dedicated  *
 * pin list from another OS process because this will result in all pins in the dedicated list     *
 * are processed twice (in both OS processes).                                                     *
 *                                                                                                 *
 * To create initial list of all pins (where you could derive your specific pin list from),        *
 * you could use script CreatePinList.bat from Bas/Hw/HwInp/_tools.                                *
 *                                                                                                 */

#ifdef Os10msProc_Use_D

// ASWT default: assigned all pins, except for the ones in 1ms
// xxx Input Pins 0.1: configure input pins in easyConfig and re-build Bsw
// xxx Input Pins 0.2: define cycle time for each input pin in _conf / Bsw_Pins / HwInp_proc_mapping.txt
// xxx Input Pins 1.1: merge pin configuration to HwInp Pin List (run script in MASAR-studio / Documentations / Actions / easyConfig / get Hw pins from Bsw)
static const HwInp_PinListEntry_ts Os10msProc_HwInpPinList_as[] = {
      { HwInp_InpTypDig_E,     HwInp_getPinIdxDig_DU16(K22) },
      { HwInp_InpTypDig_E,     HwInp_getPinIdxDig_DU16(K21) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K62) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K42) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K64) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K60) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K58) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K78) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K76) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K74) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K77) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K10) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K75) },
//    { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K53) },  // used in 1ms
//    { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K36) },  // used in 1ms
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K39) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K45) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K38) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K44) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K40) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K65) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K41) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K63) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K43) },
      { HwInp_InpTypR_E,       HwInp_getPinIdxR_DU16(K52) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K57) },
//    { HwInp_InpTypSent_E,    HwInp_getPinIdxSent_DU16(K59) },  // used in 1ms
      { HwInp_InpTypSent_E,    HwInp_getPinIdxSent_DU16(K56) },
#if defined(HWTGT_RC271840) || defined(HWTGT_RC181240)
      { HwInp_InpTypAnI_E,     HwInp_getPinIdxAnI_DU16(K42_VI) },
      { HwInp_InpTypAnI_E,     HwInp_getPinIdxAnI_DU16(K64_VI) },
      { HwInp_InpTypAnI_E,     HwInp_getPinIdxAnI_DU16(K39_VI) },
      { HwInp_InpTypAnI_E,     HwInp_getPinIdxAnI_DU16(K45_VI) },
      { HwInp_InpTypAnI_E,     HwInp_getPinIdxAnI_DU16(K38_VI) },
      { HwInp_InpTypAnI_E,     HwInp_getPinIdxAnI_DU16(K44_VI) },
      { HwInp_InpTypAnI_E,     HwInp_getPinIdxAnI_DU16(K40_VI) },
      { HwInp_InpTypAnI_E,     HwInp_getPinIdxAnI_DU16(K65_VI) },
      { HwInp_InpTypAnI_E,     HwInp_getPinIdxAnI_DU16(K41_VI) },
      { HwInp_InpTypAnI_E,     HwInp_getPinIdxAnI_DU16(K63_VI) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K55) },
      { HwInp_InpTypFrqShnt_E, HwInp_getPinIdxFrqShnt_DU16(K55_VI) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K61) },
      { HwInp_InpTypFrqShnt_E, HwInp_getPinIdxFrqShnt_DU16(K61_VI) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K54) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K37) },
      { HwInp_InpTypFrqPha_E,  HwInp_getChIdxFrqPha_DU16(K54_VI_K37_VI) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K31) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K34) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(A13) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(A14) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(A57) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(A28) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(A29) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K11) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K35) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(A40) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(A41) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(A42) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(A25) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(A26) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(A27) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(A36) },
      { HwInp_InpTypAnI_E,     HwInp_getPinIdxAnI_DU16(A36_VI) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(A37) },
      { HwInp_InpTypAnI_E,     HwInp_getPinIdxAnI_DU16(A37_VI) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(A38) },
      { HwInp_InpTypAnI_E,     HwInp_getPinIdxAnI_DU16(A38_VI) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(A39) },
      { HwInp_InpTypAnI_E,     HwInp_getPinIdxAnI_DU16(A39_VI) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K32) },
      { HwInp_InpTypR_E,       HwInp_getPinIdxR_DU16(K33) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(A21) },
      { HwInp_InpTypFrqStd_E,  HwInp_getPinIdxFrqStd_DU16(A22) },
      { HwInp_InpTypFrqStd_E,  HwInp_getPinIdxFrqStd_DU16(A23) },
      { HwInp_InpTypFrqStd_E,  HwInp_getPinIdxFrqStd_DU16(A24) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K30) },
      { HwInp_InpTypFrqShnt_E, HwInp_getPinIdxFrqShnt_DU16(K30_VI) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(A43) },
      { HwInp_InpTypFrqShnt_E, HwInp_getPinIdxFrqShnt_DU16(A43_VI) },
#endif
#if defined(HWTGT_RC5640)
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K55) },
      { HwInp_InpTypFrqShnt_E, HwInp_getPinIdxFrqShnt_DU16(K55_VI) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K61) },
      { HwInp_InpTypFrqShnt_E, HwInp_getPinIdxFrqShnt_DU16(K61_VI) },
      { HwInp_InpTypFrqPha_E,  HwInp_getChIdxFrqPha_DU16(K55_VI_K61_VI) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K54) },
      { HwInp_InpTypFrqShnt_E, HwInp_getPinIdxFrqShnt_DU16(K54_VI) },
      { HwInp_InpTypAnU_E,     HwInp_getPinIdxAnU_DU16(K37) },
      { HwInp_InpTypFrqShnt_E, HwInp_getPinIdxFrqShnt_DU16(K37_VI) },
      { HwInp_InpTypFrqPha_E,  HwInp_getChIdxFrqPha_DU16(K54_VI_K37_VI) },
#endif
};

// define length of HwInp pin list automatically derived from the pin list above.
static const uint16 Os10msProc_HwInpPinListLen_u16 = (uint16)(sizeof(Os10msProc_HwInpPinList_as)/sizeof(HwInp_PinListEntry_ts));

// abort compilation if pin list is empty. this would lead to a memory leak issue during elf2a2l conversion.
_Static_assert(sizeof(Os10msProc_HwInpPinList_as) >= (sizeof(char[1])),
               "The HwInp Pin List must not be empty. Either use at least one pin or mute Os10msProc by commenting #define Os10msProc_Use_D");


/* HwOutp pin configuration: add your list of pins/channels to be processed here.             *
 * This list and all other HwOutp pin/channel lists in all other OS processes need to be           *
 * reviewed and checked for:                                                                       *
 * - every pin/channel is only contained once in all lists                                         *
 *                                                                                                 *
 * If you need to process all configured pins in only 1 OS process, use cyclic function call       *
 * HwOutp(NULL, 0u); in the corresponding OS process.                                              *
 * this ensures all configured pins are processed and you don't need to specify such list of pins  *
 * explicitly.                                                                                     *
 * Ensure that you are not calling HwOutp(NULL, 0u) from one OS process and HwOutp with a dedicated*
 * pin list from another OS process because this will result in all pins in the dedicated list     *
 * are processed twice (in both OS processes).                                                     *
 *                                                                                                 *
 * To process a safety channel via pin/channel list, you need to add the channel (identified by    *
 * its number) to the list but you must not add the corresponding pins to the list. It is          *
 * sufficient to add the channel number!                                                           *
 *                                                                                                 *
 * Be aware of the difference between a pin contained in the pin/channel list and its stFct_e      *
 * parameter setting. If the pin is contained in the pin/channel list, the cyclic function of the  *
 * pin is executed in this task. This has no influence on the configuration of the pin with its    *
 * stFct_e parameter. If this parameter is FctOff_E and the pin is in the list, the cyclic         *
 * function is called but the output is not set.                                                   *
 * If the stFct_e parameter is not FctOff_E but the pin is not in any of the pin/channel lists the *
 * cyclic function is never called and the output is not set.                                      *
 *                                                                                                 *
 * To create initial list of all pins (where you could derive your specific pin list from),        *
 * you could use script CreatePinList.bat from Bas/Hw/HwOutp/_tools.                               *
 *                                                                                                 */

// ASWT default: assigned all pins, except for the ones in 1ms
// xxx Output Pins 0.1: configure output pins in easyConfig and re-build Bsw
// xxx Output Pins 0.2: define cycle time for each output pin in _conf / Bsw_Pins / HwOutp_proc_mapping.txt
// xxx Output Pins 1.1: merge pin configuration to HwOutp Pin List (run script in MASAR-studio / Documentations / Actions / easyConfig / get Hw pins from Bsw)
static const HwOutp_PinChListEntry_ts Os10msProc_HwOutpPinChList_as[] = {
//    { HwOutp_OutpTypSftyCh_E,  CHNL1 }, // uncomment manually, if used
//    { HwOutp_OutpTypSftyCh_E,  CHNL2 }, // uncomment manually, if used
//    { HwOutp_OutpTypSftyCh_E,  CHNL3 }, // uncomment manually, if used
//    { HwOutp_OutpTypSftyCh_E,  CHNL4 }, // uncomment manually, if used
//    { HwOutp_OutpTypSftyCh_E,  CHNL5 }, // uncomment manually, if used
#if defined(HWTGT_RC271840) || defined(HWTGT_RC181240)
//    { HwOutp_OutpTypSftyCh_E,  CHNL6 }, // uncomment manually, if used
//    { HwOutp_OutpTypSftyCh_E,  CHNL7 }, // uncomment manually, if used
//    { HwOutp_OutpTypSftyCh_E,  CHNL8 }, // uncomment manually, if used
//    { HwOutp_OutpTypSftyCh_E,  CHNL9 }, // uncomment manually, if used
//    { HwOutp_OutpTypSftyCh_E,  CHNL10 }, // uncomment manually, if used
//    { HwOutp_OutpTypSftyCh_E,  CHNL11 }, // uncomment manually, if used
//    { HwOutp_OutpTypSftyCh_E,  CHNL12 }, // uncomment manually, if used
#endif
#if defined(HWTGT_RC271840)
//    { HwOutp_OutpTypSftyCh_E,  CHNL13 }, // uncomment manually, if used
//    { HwOutp_OutpTypSftyCh_E,  CHNL14 }, // uncomment manually, if used
//    { HwOutp_OutpTypSftyCh_E,  CHNL15 }, // uncomment manually, if used
//    { HwOutp_OutpTypSftyCh_E,  CHNL16 }, // uncomment manually, if used
//    { HwOutp_OutpTypSftyCh_E,  CHNL17 }, // uncomment manually, if used
//    { HwOutp_OutpTypSftyCh_E,  CHNL18 }, // uncomment manually, if used
#endif
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(K07) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(K08) },
//    { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(K09) },  // used in 1ms
      { HwOutp_OutpTypRelU_E,    HwOutp_getPinIdxRelU_DU16(K25)    },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(K29) },
//    { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(K51) },  // used in 1ms
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(K73) },
      { HwOutp_OutpTypPropSig_E, HwOutp_getPinIdxPropSig_DU16(K80) },
      { HwOutp_OutpTypPropSig_E, HwOutp_getPinIdxPropSig_DU16(K81) },
      { HwOutp_OutpTypPropSig_E, HwOutp_getPinIdxPropSig_DU16(K82) },
      { HwOutp_OutpTypPropSig_E, HwOutp_getPinIdxPropSig_DU16(K83) },
      { HwOutp_OutpTypDigSig_E,  HwOutp_getPinIdxDigSig_DU16(K84)  },
      { HwOutp_OutpTypDigSig_E,  HwOutp_getPinIdxDigSig_DU16(K85)  },
      { HwOutp_OutpTypDigSig_E,  HwOutp_getPinIdxDigSig_DU16(K86)  },
      { HwOutp_OutpTypDigSig_E,  HwOutp_getPinIdxDigSig_DU16(K87)  },
#if defined(HWTGT_RC271840) || defined(HWTGT_RC181240)
      { HwOutp_OutpTypDigPwr_E,  HwOutp_getPinIdxDigPwr_DU16(A01)  },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A03) },
      { HwOutp_OutpTypDigPwr_E,  HwOutp_getPinIdxDigPwr_DU16(A04)  },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A05) },
      { HwOutp_OutpTypDigPwr_E,  HwOutp_getPinIdxDigPwr_DU16(A11)  },
      { HwOutp_OutpTypRelU_E,    HwOutp_getPinIdxRelU_DU16(A12)    },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A18) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A19) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A20) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A31) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A32) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A33) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A46) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A47) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A48) },
      { HwOutp_OutpTypDigPwr_E,  HwOutp_getPinIdxDigPwr_DU16(A49)  },
      { HwOutp_OutpTypDigPwr_E,  HwOutp_getPinIdxDigPwr_DU16(A55)  },
      { HwOutp_OutpTypDigPwr_E,  HwOutp_getPinIdxDigPwr_DU16(A56)  },
      { HwOutp_OutpTypDigPwr_E,  HwOutp_getPinIdxDigPwr_DU16(A58)  },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(K12) },
      { HwOutp_OutpTypDigPwr_E,  HwOutp_getPinIdxDigPwr_DU16(K13)  },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(K14) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(K15) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(K16) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(K17) },
#endif
#if defined(HWTGT_RC271840)
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A02) },
      { HwOutp_OutpTypDigPwr_E,  HwOutp_getPinIdxDigPwr_DU16(A06)  },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A07) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A08) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A09) },
      { HwOutp_OutpTypDigPwr_E,  HwOutp_getPinIdxDigPwr_DU16(A10)  },
      { HwOutp_OutpTypDigPwr_E,  HwOutp_getPinIdxDigPwr_DU16(A16)  },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A17) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A34) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A35) },
      { HwOutp_OutpTypDigPwr_E,  HwOutp_getPinIdxDigPwr_DU16(A50)  },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A51) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A52) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A53) },
      { HwOutp_OutpTypPropPwr_E, HwOutp_getPinIdxPropPwr_DU16(A54) },
#endif
#if defined(HWTGT_RC5640)
      { HwOutp_OutpTypDigPwr_E,  HwOutp_getPinIdxDigPwr_DU16(K12)  },
      { HwOutp_OutpTypDigPwr_E,  HwOutp_getPinIdxDigPwr_DU16(K13)  },
      { HwOutp_OutpTypDigPwr_E,  HwOutp_getPinIdxDigPwr_DU16(K14)  },
      { HwOutp_OutpTypDigPwr_E,  HwOutp_getPinIdxDigPwr_DU16(K15)  },
      { HwOutp_OutpTypDigPwr_E,  HwOutp_getPinIdxDigPwr_DU16(K16)  },
#endif
};

// define length of HwOutp pin list automatically derived from the pin list above.
static const uint16 Os10msProc_HwOutpPinChListLen_u16 = (uint16)(sizeof(Os10msProc_HwOutpPinChList_as)/sizeof(HwOutp_PinChListEntry_ts));

// abort compilation if pin list is empty. this would lead to a memory leak issue during elf2a2l conversion.
_Static_assert(sizeof(Os10msProc_HwOutpPinChList_as) >= (sizeof(char[1])),
               "The HwOutp Pin/Channel List must not be empty. Either use at least one pin/channel or mute Os10msProc by commenting #define Os10msProc_Use_D");

#endif

/* static function prototypes **********************************************************************
 * Declarations of functions needed in the scope of this c-file.                                   *
 * Memory class "static" has to be used for each function to assure it only exist in this scope.   */

/* functions ***************************************************************************************
 * public or static functions implemented in this component.                                       */

//**************************************************************************************************
//  Function:       Os10msProc
//
//! \brief          Application task for 10ms calculation cycle
//!                 Execution of signal flow and basic MASAR EcoSystem features
//!
//! \param[in]      -
//! \return         -
//!
/* this function is called by BSW scheduler.                                                       *
 * - needs to be implemented if process is mapped in easyConfig.                                   *
 * - calling this component only affects run-time in nano-seconds, if left empty                   *
 *                                                                                                 */
//**************************************************************************************************
void Os10msProc(void)
{
  const uint16 tiCyc_ms_u16 = 10u; // cycle time of this process; @rawinit(10), @rawrange[0..65535]
  PARAM_UNUSED(tiCyc_ms_u16);  // avoid compiler warning...  can be removed if used
  Os10msProc_s.prv_s.nrCall_u16++;
  #ifdef Os10msProc_Use_D

  OsCmn_tic(&Os10msProc_s.prv_s.tiTskRtStrt_us_u64); // Start runtime measurement (total)
  OsCmn_tic(&Os10msProc_s.prv_s.tiTskRtHelp_us_u64); // Start runtime measurement (individual)

  //  bds_can_sendData(BDS_CAN_1_D, 0x20000, BDS_CAN_EXD_DU8, 2, (uint8*)&Os10msProc_s.prv_s.nrCall_u16); // xxx Debug multitasking (10ms): uncomment to generate CAN Tx Msg

  /* --------------------------------------------------------- */
  /* Parameter Management */
  /* --------------------------------------------------------- */

  /* Parameter tree to component parameters */
  PrmMgr( DevInp_s.ModReq_s.outp_s.stModReq_u8 );

  OsCmn_toc(&Os10msProc_s.prv_s.tiTskRtHelp_us_u64, &Os10msProc_s.prv_s.tiTskRtPrmMgr_us_u32); // Run time measurement

  /* --------------------------------------------------------- */
  /* Debug Management */
  /* --------------------------------------------------------- */

  DbgMgr_s.prm_s.tiCyc_ms_u16 = tiCyc_ms_u16;
  DbgMgr();
  DbgMgrIntegn();

  OsCmn_toc(&Os10msProc_s.prv_s.tiTskRtHelp_us_u64, &Os10msProc_s.prv_s.tiTskRtDbg_us_u32); // Run time measurement

  /* --------------------------------------------------------- */
  /* UDS Safe State */
  /* --------------------------------------------------------- */
  UdsSafeSt_s.inp_s.flgMacSafeSt_l = TRUE; // todo in Application Project: UDS permission: assign condition for safe UDS permission
  //UdsSafeSt_s.inp_s.flgMacSafeSt_l = Veh_s.MtnSt_s.outp_s.flgStop_u8; // example for drive applications
  UdsSafeSt();

  // Allow/Prevent UDS requests to start CB or reset ECU
  if( UdsSafeSt_s.outp_s.flgUdsSafeSt_l )
  {
    bds_uds_permitAccess( UDS_PERMIT_ALL_DU8 );
  }
  else
  {
    bds_uds_permitAccess( UDS_PREVENT_ALL_DU8 );
  }

  OsCmn_toc(&Os10msProc_s.prv_s.tiTskRtHelp_us_u64, &Os10msProc_s.prv_s.tiTskRtUds_us_u32); // Run time measurement

  // "Wait for XCP ready" feature
  if( 0u == (DbgMgr_s.outp_s.stErrDetn_u8 & DbgMgr_ErrBitMaskWaitForXcp_DU8) )
  {
    /* Delay clearing of flgIni for that process */
    bool flgIni_l = OsCmn_isStrtDlyActv( &Os10msProc_s.prv_s.nrStrtDlyActvCall_u8 );

    /* ############################################################################################################################# */
    /* Api -> Bas */
    /* ############################################################################################################################# */

    /* --------------------------------------------------------- */
    /* ECU Monitoring */
    /* --------------------------------------------------------- */

    /* ECU application monitor and API hardware monitor */
    EcuHlth_s.prm_s.tiCyc_ms_u16 = tiCyc_ms_u16;
    EcuHlth();

    OsCmn_toc(&Os10msProc_s.prv_s.tiTskRtHelp_us_u64, &Os10msProc_s.prv_s.tiTskRtEcu_us_u32); // Run time measurement

    /* --------------------------------------------------------- */
    /* Electrical Inputs */
    /* --------------------------------------------------------- */

    #ifdef DEVLFCT_ON
      // in case of DEVLFCT_ON, execution of HwInp can be skipped for e.g testing purposes
      if( !DbgMgr_s.prm_s.Xcp_s.flgSkipHwInp_l )
    #endif
    {

      // HwInp backward compatibility: use HwInp(NULL, 0u);
      HwInp(Os10msProc_HwInpPinList_as, Os10msProc_HwInpPinListLen_u16);

    }
	
    OsCmn_toc(&Os10msProc_s.prv_s.tiTskRtHelp_us_u64, &Os10msProc_s.prv_s.tiTskRtHwInp_us_u32); // Run time measurement

    /* --------------------------------------------------------- */
    /* Communication Inputs */
    /* --------------------------------------------------------- */

    /* CAN legacy inputs */
    CanRx();

    /* J1939 inputs */
    // INFO: dynamic parameter value J1939Rx_s.prm_s.tiCyc_ms_u16 is assigned in OsIniProc
    J1939Rx();

    OsCmn_toc(&Os10msProc_s.prv_s.tiTskRtHelp_us_u64, &Os10msProc_s.prv_s.tiTskRtComRx_us_u32); // Run time measurement

    /* --------------------------------------------------------- */
    /* Diagnosis Management */
    /* --------------------------------------------------------- */

    /* Error detection and report */
    ErrMgr_s.prm_s.tiCyc_ms_u16 = tiCyc_ms_u16;
    ErrMgr();

    OsCmn_toc(&Os10msProc_s.prv_s.tiTskRtHelp_us_u64, &Os10msProc_s.prv_s.tiTskRtErr_us_u32); // Run time measurement

    #ifdef DEVLFCT_ON
      // in case of DEVLFCT_ON, execution of ErrReactn can be skipped for e.g testing purposes
      if( !DbgMgr_s.prm_s.Xcp_s.flgSkipErrReactn_l )
    #endif
    {
      /* Error reactions */
      ErrReactn();

      /* Two-step error reactions */
      ErrReactnTwoStep();
    }

    // handling of error memory, limit maximum DFES entries in order to keep space for new active errors
    ErrMemMgr();

    /* Release Condition definitions for Error Configuration */
    EcuMainRels();

    OsCmn_toc(&Os10msProc_s.prv_s.tiTskRtHelp_us_u64, &Os10msProc_s.prv_s.tiTskRtErrReactn_us_u32); // Run time measurement

    /* ############################################################################################################################# */
    /* Bas -> Dev */
    /* ############################################################################################################################# */

    /* --------------------------------------------------------- */
    /* Device Input */
    /* --------------------------------------------------------- */

    #ifdef DEVLFCT_ON
      // in case of DEVLFCT_ON, execution of complete DevInp can be skipped for e.g testing purposes
      if( !DbgMgr_s.prm_s.Xcp_s.flgSkipDevInpLyr_l )
    #endif
    {
      /* Call your DevInp integration component here.                                                    *
       *                                                                                                 *
       * IMPORTANT: If you want to share data/signals between this process and another one,              *
       * you need to take care of synchronization of data written here and there.                        *
       * To do this:                                                                                     *
       * - share only data that can be accessed atomically (e.g. single signals <= 32 bit)               *
       *   see IntegrationGuide / Training / multitasking / atomic data                                  *
       * - use Bas/Os/OsIpc that provides mechanisms to share data consistently.                         */

      DevInp_s.inp_s.flgInit10ms_l    = flgIni_l;
      DevInp_s.inp_s.flgModTran_l = PrmMgr_s.outp_s.flgModTran_l;
      DevInp_10ms();

    }

    OsCmn_toc(&Os10msProc_s.prv_s.tiTskRtHelp_us_u64, &Os10msProc_s.prv_s.tiTskRtDevInp_us_u32); // Run time measurement


    /* ############################################################################################################################# */
    /* Dev -> Veh */
    /* ############################################################################################################################# */

    /* --------------------------------------------------------- */
    /* Vehicle */
    /* --------------------------------------------------------- */

    /* Vehicle layer */
    #ifdef DEVLFCT_ON
      // in case of DEVLFCT_ON, execution of complete Veh can be skipped for e.g testing purposes
      if( !DbgMgr_s.prm_s.Xcp_s.flgSkipVehLyr_l )
    #endif
    {
      /* Call your Veh integration component here.                                                       *
       *                                                                                                 *
       * IMPORTANT: If you want to share data/signals between this process and another one,              *
       * you need to take care of synchronization of data written here and there.                        *
       * To do this:                                                                                     *
       * - share only data that can be accessed atomically (e.g. single signals <= 32 bit)               *
       *   see IntegrationGuide / Training / multitasking / atomic data                                  *
       * - use Bas/Os/OsIpc that provides mechanisms to share data consistently.                         */

      Veh_s.inp_s.flgInit10ms_l = flgIni_l;
      Veh_s.inp_s.flgModTran_l = PrmMgr_s.outp_s.flgModTran_l;
      Veh_10ms();

    }

    OsCmn_toc(&Os10msProc_s.prv_s.tiTskRtHelp_us_u64, &Os10msProc_s.prv_s.tiTskRtVeh_us_u32); // Run time measurement

    /* Customer layer */

    /* Call your Cust integration component here.                                                      *
     *                                                                                                 *
     * IMPORTANT: If you want to share data/signals between this process and another one,              *
     * you need to take care of synchronization of data written here and there.                        *
     * To do this:                                                                                     *
     * - share only data that can be accessed atomically (e.g. single signals <= 32 bit)               *
     *   see IntegrationGuide / Training / multitasking / atomic data                                  *
     * - use Bas/Os/OsIpc that provides mechanisms to share data consistently.                         */

    Cust_s.inp_s.flgInit10ms_l = flgIni_l;
    Cust_s.inp_s.flgModTran_l = PrmMgr_s.outp_s.flgModTran_l;
    Cust_10ms();

    OsCmn_toc(&Os10msProc_s.prv_s.tiTskRtHelp_us_u64, &Os10msProc_s.prv_s.tiTskRtCust_us_u32); // Run time measurement

    /* ############################################################################################################################# */
    /* Veh -> Dev */
    /* ############################################################################################################################# */

    /* --------------------------------------------------------- */
    /* Device Output */
    /* --------------------------------------------------------- */

    #ifdef DEVLFCT_ON
      // in case of DEVLFCT_ON, execution of complete DevOutp can be skipped for e.g testing purposes
      if( !DbgMgr_s.prm_s.Xcp_s.flgSkipDevOutpLyr_l )
    #endif
    {
      /* Call your DevOutp integration component here.                                                   *
       *                                                                                                 *
       * IMPORTANT: If you want to share data/signals between this process and another one,              *
       * you need to take care of synchronization of data written here and there.                        *
       * To do this:                                                                                     *
       * - share only data that can be accessed atomically (e.g. single signals <= 32 bit)               *
       *   see IntegrationGuide / Training / multitasking / atomic data                                  *
       * - use Bas/Os/OsIpc that provides mechanisms to share data consistently.                         */

      DevOutp_s.inp_s.flgInit10ms_l    = flgIni_l;
      DevOutp_s.inp_s.flgModTran_l = PrmMgr_s.outp_s.flgModTran_l;
      DevOutp_10ms();

    }

    OsCmn_toc(&Os10msProc_s.prv_s.tiTskRtHelp_us_u64, &Os10msProc_s.prv_s.tiTskRtDevOutp_us_u32); // Run time measurement


    /* ############################################################################################################################# */
    /* Dev -> Bas */
    /* ############################################################################################################################# */

    /* --------------------------------------------------------- */
    /* Communication Outputs */
    /* --------------------------------------------------------- */

    /* J1939 output */
    // INFO: dynamic parameter value J1939Tx_s.prm_s.tiCyc_ms_u16 is assigned in OsIniProc
    J1939Tx();

    /* CAN legacy output */
    CanTx();

    OsCmn_toc(&Os10msProc_s.prv_s.tiTskRtHelp_us_u64, &Os10msProc_s.prv_s.tiTskRtComTx_us_u32); // Run time measurement

    /* --------------------------------------------------------- */
    /* Electrical Outputs */
    /* --------------------------------------------------------- */

    #ifdef DEVLFCT_ON
      // in case of DEVLFCT_ON, execution of complete HwOutp including HwOutpIntegn can be skipped for e.g testing purposes
    if( !DbgMgr_s.prm_s.Xcp_s.flgSkipHwOutp_l )
    #endif
    {

      // xxx Output Pins 2.2: assign setpoint values to pins in HwOutp pin arrays
      // example for High-side and Low-Side wiring of a current regulated proportional valve:
      // HwOutp_s.PropPwr_as[DevOutp_MotHS_D].inp_s.iSp_mA_u16         = (uint16)(DbgCrntOvrd_as[DevOutp_MotHS_D].outp_s.i_mA_s16);
      // HwOutp_s.PropPwr_as[DevOutp_MotHS_D].inp_s.stErrReactn_e      = ErrReactn_s.outp_s.ErrReactn_s.PropPwr_as_DevOutp_MotHS_D_stErrReactn_e;
      // HwOutp_s.DigPwr_as[DevOutp_MotLS_D].inp_s.stErrReactn_e       = ErrReactn_s.outp_s.ErrReactn_s.DigPwr_as_DevOutp_MotLS_D_stErrReactn_e;
      // example for a digital voltage output
      // HwOutp_s.DigSig_as[DevOutp_FltLampLed_D].inp_s.flgSp_l        = (bool)(DevOutp_s.FltLampActr_s.outp_s.flgEnerOutp_u8);
      // HwOutp_s.DigSig_as[DevOutp_FltLampLed_D].inp_s.stErrReactn_e  = ErrReactn_s.outp_s.ErrReactn_s.DigSig_as_DevOutp_FltLampLed_D_stErrReactn_e;

     // LLC output
      HwOutp_s.PropPwr_as[DevOutp_Pmp1HS_D].inp_s.stErrReactn_e = ErrReactn_s.outp_s.ErrReactn_s.PropPwr_as_DevOutp_Pmp1HS_D_stErrReactn_e;
      HwOutp_s.PropPwr_as[DevOutp_Pmp1HS_D].inp_s.iSp_mA_u16 = DevOutp_s.PumpA_s.outp_s.iPmp_mA_u16;
      HwOutp_s.DigPwr_as[DevOutp_Pmp1LS_D].inp_s.stErrReactn_e = ErrReactn_s.outp_s.ErrReactn_s.DigPwr_as_DevOutp_Pmp1LS_D_stErrReactn_e;
      HwOutp_s.DigPwr_as[DevOutp_Pmp1LS_D].inp_s.flgSp_l = TRUE;

      HwOutp_s.PropPwr_as[DevOutp_Pmp2HS_D].inp_s.stErrReactn_e = ErrReactn_s.outp_s.ErrReactn_s.PropPwr_as_DevOutp_Pmp2HS_D_stErrReactn_e;
      HwOutp_s.PropPwr_as[DevOutp_Pmp2HS_D].inp_s.iSp_mA_u16 = DevOutp_s.PumpB_s.outp_s.iPmp_mA_u16;
      HwOutp_s.DigPwr_as[DevOutp_Pmp2LS_D].inp_s.stErrReactn_e = ErrReactn_s.outp_s.ErrReactn_s.DigPwr_as_DevOutp_Pmp2LS_D_stErrReactn_e;
      HwOutp_s.DigPwr_as[DevOutp_Pmp2LS_D].inp_s.flgSp_l = TRUE;



      // HwOutp backward compatibility: use HwOutp(NULL, 0u);
      HwOutp(Os10msProc_HwOutpPinChList_as, Os10msProc_HwOutpPinChListLen_u16);

    }
    
    OsCmn_toc(&Os10msProc_s.prv_s.tiTskRtHelp_us_u64, &Os10msProc_s.prv_s.tiTskRtHwOutp_us_u32); // Run time measurement

    /* --------------------------------------------------------- */
    /* Parameter Update */
    /* --------------------------------------------------------- */

    /* Feedback calibration parameters and runtime data to parameter tree */
    PrmUpd();

    OsCmn_toc(&Os10msProc_s.prv_s.tiTskRtHelp_us_u64, &Os10msProc_s.prv_s.tiTskRtPrmUpd_us_u32); // Run time measurement

    // Total task runtime measurement
    OsCmn_toc(&Os10msProc_s.prv_s.tiTskRtStrt_us_u64, &Os10msProc_s.prv_s.tiTskRt_us_u32 );
  }

  #endif // Os10msProc_Use_D

} // Os10msProc


/* *********************************************************************************************** */
#endif /* #ifndef _OS10MSPROC_C_ */
/* EOF ########################################################################################### */
