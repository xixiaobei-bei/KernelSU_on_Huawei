/*
 * wireless_tx_acc.h
 *
 * wireless accessory reverse charging  driver
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

#ifndef _WIRELESS_TX_ACC_H_
#define _WIRELESS_TX_ACC_H_

#define WLTX_ACC_DEV_MAC_LEN            6
#define WLTX_ACC_DEV_MODELID_LEN        3
#define WLTX_ACC_DEV_SUBMODELID_LEN     1
#define WLTX_ACC_DEV_VERSION_LEN        1
#define WLTX_ACC_DEV_BUSINESS_LEN       1
#define WLTX_ACC_DEV_INFO_CNT           12

enum wireless_tx_acc_info_index {
	WLTX_ACC_INFO_BEGIN = 0,
	WLTX_ACC_INFO_NO = WLTX_ACC_INFO_BEGIN,
	WLTX_ACC_INFO_STATE,
	WLTX_ACC_INFO_MAC,
	WLTX_ACC_INFO_MODEL_ID,
	WLTX_ACC_INFO_SUBMODEL_ID,
	WLTX_ACC_INFO_VERSION,
	WLTX_ACC_INFO_BUSINESS,
	WLTX_ACC_INFO_END,
};

enum wltx_acc_dev_state {
	WLTX_ACC_DEV_STATE_BEGIN = 0,
	WLTX_ACC_DEV_STATE_OFFLINE = WLTX_ACC_DEV_STATE_BEGIN,
	WLTX_ACC_DEV_STATE_ONLINE,
	WLTX_ACC_DEV_STATE_PING_SUCC,
	WLTX_ACC_DEV_STATE_END,
};

#ifdef CONFIG_WIRELESS_CHARGER
void wltx_acc_set_dev_state(u8 dev_state);
int wltx_acc_get_dev_info_and_notify(void);
#else
static inline void wltx_acc_set_dev_state(u8 dev_state)
{
}

static inline int wltx_acc_get_dev_info_and_notify(void)
{
	return 0;
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_TX_ACC_H_ */
