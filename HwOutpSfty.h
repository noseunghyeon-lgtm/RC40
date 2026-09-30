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

#ifndef _HWOUTPSFTY_H_
#define _HWOUTPSFTY_H_

#ifdef _HWOUTPSFTY_C_
#define HWOUTPSFTY_SCOPE_D
#else
#define HWOUTPSFTY_SCOPE_D extern
#endif

/* function component metadata ***************************
 * @fc-longname (Hardware Output Safety)                 *
 * @suprtd-ecutype(RC5-6/40, RC18-12/40, RC27-18/40)     *
 * *******************************************************/

/* include files ***********************************************************************************
 * List of include files needed in this module.                                                    *
 * DO NOT INCLUDE HEADER FILES WITH VARIABLE DECLARATIONS HERE!                                    *
 * ONLY INCLUDES OF TYPEDEFS OR DEFINES ARE ALLOWED!                                               */

/* Bsw includes */
#include "bds_safout.h"

/* Bas includes */
#include "HwOutpCmn.h"
#include "HwOutpPropPwr.h"
#include "HwOutpDigPwr.h"

/* defines *****************************************************************************************
 * List of defines needed in this header file and defines to be made available to other modules.   */
#define HwOutpSfty_nrPinsMax_DU8      (uint8)5u  // Maximum number of pins per safout channel
#define HwOutpSfty_nrNormPinsMax_DU8  (uint8)4u  // Maximum number of normal pins (non common) per safout channel

/* typedefs/structures *****************************************************************************
 * Typedefs/structures to be made available to other modules.                                      */
typedef enum HwOutpSfty_PinTyp_t
{
  HwOutpSfty_PinTypNotSet_E,        // No pin type set
  HwOutpSfty_PinTypProp_E,          // Proportional Output
  HwOutpSfty_PinTypDig_E            // Digital Output
} HwOutpSfty_PinTyp_te;

typedef enum HwOutpSfty_Err_t
{
  HwOutpSfty_ErrNone_E,             // No error during configuration
  HwOutpSfty_ErrCfgBswCall_E,       // bds_safout_cfg() returned error
  HwOutpSfty_ErrNrCmnPins_E,        // The number of common pins assigned to the safout channel is not equal to one
  HwOutpSfty_ErrWrgPinSide_E,       // At least one signal pin is configured on the wrong side (HS vs. LS)
  HwOutpSfty_ErrNrPins_E,           // The total number of pins assigned to the safout channel is wrong
  HwOutpSfty_ErrModNotUni_E,        // Not all pins in the safout group have the same safout mode
  HwOutpSfty_ErrNoCmnPinFnd_E,      // No common pin was found
  HwOutpSfty_ErrWrgTyp_E,           // One of the pins in the safout group has incorrect function type
  HwOutpSfty_ErrWrgTypCmn_E,        // The common pin in the safout group has incorrect function type
  HwOutpSfty_ErrSet_E,              // The base software returned an error when setting an output to the requested state/setpoint
  HwOutpSfty_ErrTwoSp_E,            // Two digital outputs active simultaneously in one of the two switchable modes (PHDLS or PLDHS).
  HwOutpSfty_ErrUnkwnSftyChnMod_E   // There is a unknown safout channel mode configured
} HwOutpSfty_Err_te;

typedef struct HwOutpSfty_PinInfo_t
{
  HwOutpSfty_PinTyp_te stPinTyp_e;  // Specifies the pin type
  uint8 idxAry_u8;                  // Array index for accessing PropPwr_as and DigPwr_as arrays defined in HwOutp
  sint32 nrPin_s32;                 // represents the pin number
} HwOutpSfty_PinInfo_ts;

typedef struct HwOutpSfty_PinSrch_t
{
  uint8 nrPinsFnd_u8;
  uint8 aryIdxPin_au8[HwOutpSfty_nrPinsMax_DU8];
} HwOutpSfty_PinSrch_ts;

typedef struct HwOutpSfty_PinFnd_t
{
  uint8 nrCmnPinsFnd_u8;
  uint8 aryIdxCmnPin_u8;
  uint8 nrNormPinsFnd_u8;
  uint8 aryIdxNormPin_au8[HwOutpSfty_nrNormPinsMax_DU8];
} HwOutpSfty_PinFnd_ts;

typedef struct HwOutpSfty_prv_t
{
  // Helpers to debug the pin search
  HwOutpSfty_PinSrch_ts PropPinSrch_s;  // Search result for all associated proportional power outputs
  HwOutpSfty_PinSrch_ts DigPinSrch_s;   // Search result for all associated digital power outputs
  HwOutpSfty_PinFnd_ts  PropPinFnd_s;   // Found proportional power outputs (common pin and normal pins)
  HwOutpSfty_PinFnd_ts  DigPinFnd_s;    // Found digital power outputs (common pin and normal pins)

  // Safout pin info
  HwOutpSfty_PinInfo_ts PinInfoCmn_s;   // Contains details about the common pins
  HwOutpSfty_PinInfo_ts PinInfoNorm_as[HwOutpSfty_nrNormPinsMax_DU8]; // Contains details about the normal pins
  uint8 nrNormPins_u8;                  // Number of non-common pins

  // Safout channel Info
  HwOutpCmn_SftyMod_te stFctIni_e;      // The safety mode of the safety channel
  uint16 ChName_u16;                    // The calculated channel name
  uint16 nrCh_u16;                      // The specified channel number

  // API structure and error helpers
  bds_safout_ts stApi_s;                // Status details of the satefy channel from the API
  bool flgLockdOutpErrSet_l;            // Flag active is stDFC_u16 is active, used for performance reasons
  bool flgLockdIDevErrSet_l;            // Flag active is stDFC_iDev_u16 is active, used for performance reasons

  // Initialization status and configuration error reason
  bool flgIniSuc_l;                     // Indicates if the initialization process was successful.
  HwOutpSfty_Err_te stCfgErr_e;         // Provides details on configuration errors

} HwOutpSfty_prv_ts;

typedef struct HwOutpSfty_t
{
  HwOutpSfty_prv_ts prv_s;
} HwOutpSfty_ts;

/* public data *************************************************************************************
 * Declarations of variables to be made available to other modules. Use SCOPE_D in front of each   *
 * line.                                                                                           */

/* public functions ********************************************************************************
 * Declarations of functions to be made available to other modules. Use SCOPE_D in front of each   *
 * line.                                                                                           */
HWOUTPSFTY_SCOPE_D void HwOutpSfty_ini(uint8 ChIdx_u8, HwOutpSfty_ts *SftyCh_ps, HwOutpPropPwr_ts PropPwr_as[], HwOutpDigPwr_ts DigPwr_as[]); // Initialization function for safety outputs.
HWOUTPSFTY_SCOPE_D void HwOutpSfty(HwOutpSfty_ts *SftyCh_ps, HwOutpPropPwr_ts PropPwr_as[], HwOutpDigPwr_ts DigPwr_as[]); // Cyclic function for setting safety outputs.
HWOUTPSFTY_SCOPE_D void HwOutpSfty_procSftyPins(HwOutpSfty_ts *SftyCh_ps, HwOutpPropPwr_ts PropPwr_as[], HwOutpDigPwr_ts DigPwr_as[]); // Cyclic function for processing all the pins of the channel.
HWOUTPSFTY_SCOPE_D void HwOutpSfty_setAllPinsToZr(HwOutpSfty_ts *SftyCh_ps, HwOutpPropPwr_ts PropPwr_as[], HwOutpDigPwr_ts DigPwr_as[]); // Setting of all safout power pins to 0. Needed for the After Run mode.

/* *********************************************************************************************** */

#endif /* #ifndef _HWOUTPSFTY_H_ */

/* EOF ########################################################################################### */
