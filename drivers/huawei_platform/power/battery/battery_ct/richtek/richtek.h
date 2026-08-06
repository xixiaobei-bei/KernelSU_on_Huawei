/*
 * richtek.h
 *
 * Authentication driver for richtek
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#ifndef _RICHTEK_H_
#define _RICHTEK_H_

#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include "../batt_aut_checker.h"
#include "richtek_comm.h"

#define RT_FAIL    (-1)
#define RT_SUCCESS 0
#define RT_CURRENT_TASK 0

#define RT_SN_CHAR_PRINT_SIZE 8
#define RT_SN_HEX_PRINT_SIZE 8
#define RT_SN_MIN_SIZE (RT_SN_CHAR_PRINT_SIZE + RT_SN_HEX_PRINT_SIZE)
#define RT_HEX_PER_CHAR 2
#define RT_BYTE_BITS 8
#define RT_AUTH_RETRY 8
#define RT_MAX_MAC_SIZE 64
#define RT_WORK_CHALLENGE_SIZE 32
#define RT_WORK_SECRET_SIZE 32
#define RT_TEST_BUF_LEN 256

#define RT_REAL_CHAR_NUM 7
#define RT_YEAR_MONTH_NUM 3
#define RT_SERIAL_CODE_NUM 4
#define RT_DEVICE_ID_LEN 4
#define RT_BATT_TYPE_BIT1 5
#define RT_BATT_TYPE_BIT2 4

#define is_odd(a) ((a) & 0x1)

struct rt_battery_constraint {
	unsigned char *id_mask;
	unsigned char *id_example;
	unsigned char *id_chk;
};

struct rt_drv_data {
	unsigned char *device_id;
	unsigned char *rom_id;
	unsigned char *sn;
	unsigned char *batt_type;
	unsigned char *status;
	unsigned char *mac_datum;
	unsigned int device_id_size;
	unsigned int rom_id_size;
	unsigned int sn_size;
	unsigned int batt_type_size;
	unsigned int status_size;
	unsigned int mac_datum_size;
	unsigned int work_secret_size;
	unsigned int work_challenge_size;
	unsigned int root_challenge_size;
	unsigned int mac_size;
	unsigned int phy_id;
	struct batt_ct_ops_list ct_node;
	struct rt_ic_des ic_des;
	struct rt_battery_constraint batt_cons;
	struct wakeup_source write_lock;
	struct mutex batt_safe_info_lock;
};

#endif /* _RICHTEK_H_ */
