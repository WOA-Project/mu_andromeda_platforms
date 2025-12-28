/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#ifndef _XBL_HLOS_HOB_H_
#define _XBL_HLOS_HOB_H_

#pragma pack(1)
#define MCFG_MODE 0
#define CUST_MODE 1
#define MAX_VERSION_LEN 16

// Note: Need to take care alignment of struct member
typedef struct XBL_HLOS_HOB
{
    UINT8 BoardID;                          // (00)
    UINT8 BatteryPresent;                   // (01) Indicates battery presence: 0 - battery absent, 1 - battery present
    UINT8 HwInitRetries;                    // (02) Indicates retries attempted to initialize descrete hardware circuit
    UINT8 IsCustomerMode;                   // (03) Indicates whether the device is in Manufacturing Mode: 0 - in manuf mode, 1 - in Customer mode
    UINT8 IsActMode;                        // (04) Indicates whether device has act mode enabled. 0 - disabled 1 - enabled
    UINT8 PmicResetReason;                  // (05) PmicResetReason: 9 - battery driver triggered
    CHAR8 TouchFWVersion[MAX_VERSION_LEN];  // (06) Current Touch Firmware version number
    UINT16 OCPErrorLocation;                // (07) Identify which power rail has the OCP Error
                                            //      Bit(s)     Meaning
                                            //      15         More than one OCP error occurred
                                            //      14-12      PMIC
                                            //      11-7       SMPS
                                            //      6-2        LDO
                                            //      1-0        BOB
} *PXBL_HLOS_HOB;

#pragma pack()

#endif /* _XBL_HLOS_HOB_H_ */