/*
 * charger_type.h
 *
 * charger type definition for charger module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _CHARGER_TYPE_H_
#define _CHARGER_TYPE_H_

enum power_charger_type {
	CHARGER_TYPE_BEGIN = 0,
	CHARGER_TYPE_USB = CHARGER_TYPE_BEGIN, /* 0:SDP */
	CHARGER_TYPE_BC_USB,                   /* 1:CDP */
	CHARGER_TYPE_NON_STANDARD,             /* 2:Unknown */
	CHARGER_TYPE_STANDARD,                 /* 3:DCP */
	CHARGER_TYPE_FCP,                      /* 4:FCP */
	CHARGER_REMOVED,                       /* 5:Not Connected */
	USB_EVENT_OTG_ID,                      /* 6 */
	CHARGER_TYPE_VR,                       /* 7:VR Charger */
	CHARGER_TYPE_TYPEC,                    /* 8:PD Charger */
	CHARGER_TYPE_PD,                       /* 9:PD Charger */
	CHARGER_TYPE_SCP,                      /* 10:SCP Charger */
	CHARGER_TYPE_WIRELESS,                 /* 11:Wireless Charger */
	CHARGER_TYPE_POGOPIN,                  /* 12:POGOPIN Charger */
	CHARGER_TYPE_APPLE_2_1A,               /* 13:2.1A Apple Charger */
	CHARGER_TYPE_APPLE_1_0A,               /* 14:1A Apple Charger */
	CHARGER_TYPE_APPLE_0_5A,               /* 15:0.5A Apple Charger */
	CHARGER_TYPE_BATTERY,                  /* 16 */
	CHARGER_TYPE_UPS,                      /* 17 */
	CHARGER_TYPE_MAINS,                    /* 18 */
	CHARGER_TYPE_ACA,                      /* 19:Accessory Charger Adapters */
	CHARGER_TYPE_PD_DRP,                   /* 20:PD Dual Role Port */
	CHARGER_TYPE_APPLE_BRICK_ID,           /* 21:Apple Charging Method */
	CHARGER_TYPE_HVDCP,                    /* 22:High Voltage DCP */
	CHARGER_TYPE_HVDCP_3,                  /* 23:Efficient High Voltage DCP */
	CHARGER_TYPE_BMS,                      /* 24:Battery Monitor System */
	CHARGER_TYPE_PARALLEL,                 /* 25:Parallel Path */
	CHARGER_TYPE_MAIN,                     /* 26:Main Path */
	CHARGER_TYPE_UFP,                      /* 27:Type-C UFP */
	CHARGER_TYPE_DFP,                      /* 28:Type-C DFP */
	CHARGER_TYPE_CHARGE_PUMP,              /* 29:Charge Pump */
	CHARGER_TYPE_END,
};

#endif /* _CHARGER_TYPE_H_ */
