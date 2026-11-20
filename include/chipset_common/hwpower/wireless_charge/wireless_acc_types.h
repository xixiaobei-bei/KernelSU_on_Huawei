/*
 * wireless_acc_types.h
 *
 * accessory(tx,cable,adapter etc.) types for wireless charging
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

#ifndef _WIRELESS_ACC_TYPES_H_
#define _WIRELESS_ACC_TYPES_H_

#define TX_TYPE_FAC_BASE              0x20 /* tx for factory */
#define TX_TYPE_FAC_MAX               0x3F
#define TX_TYPE_CAR_BASE              0x40 /* tx for car */
#define TX_TYPE_CAR_MAX               0x5F
#define TX_TYPE_PWR_BANK_BASE         0x60 /* tx for power bank */
#define TX_TYPE_PWR_BANK_MAX          0x7F

enum tx_adap_type {
	TX_TYPE_UNKOWN   = 0x00,
	TX_TYPE_SDP      = 0x01,
	TX_TYPE_CDP      = 0x02,
	TX_TYPE_NON_STD  = 0x03,
	TX_TYPE_DCP      = 0x04,
	TX_TYPE_FCP      = 0x05,
	TX_TYPE_SCP      = 0x06,
	TX_TYPE_PD       = 0x07,
	TX_TYPE_QC       = 0x08,
	TX_TYPE_OTG_A    = 0x09, /* rvs charging powered by battery */
	TX_TYPE_OTG_B    = 0x0A, /* rvs charging powered by adaptor */
	TX_TYPE_ERR      = 0xff,
};

#endif /* _WIRELESS_ACC_TYPES_H_ */
