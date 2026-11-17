/*
 * batt_uuid_binder.h
 *
 * Battery and board bind with uuid.
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

#ifndef _BATT_UUID_BINDER_H_
#define _BATT_UUID_BINDER_H_

#include <linux/mutex.h>
#include <linux/completion.h>
#include "batt_aut_checker.h"

#define MAX_BATT_UUID_LEN 13
#define MAX_BATT_BIND_NUM 10
#define POWER_NORMAL_CBMESG "OK"
#define POWER_ABR_CBMESG "FAIL"
#define POWER_MESG_TIME_OUT (HZ * 1.1)
#define POWER_MESG_RETRY_TIMES 10

enum {
	INVALID_UUID_BIND_VER = 0,
	RAW_UUID_BIND_VER,
};

enum {
	OLD_BOARD = 0,
	NEW_BOARD,
};

struct batt_uuid_record {
	unsigned int version;
	char uuid[MAX_BATT_BIND_NUM][MAX_BATT_UUID_LEN];
};

struct batt_uuid_bind {
	int new_board;
	struct batt_uuid_record record;
};

struct batt_bind_data {
	int init_sign;
	int update_sign;
	int read_wait_sign;
	struct mutex lock;
	struct completion uuid_record_ready;
	struct completion write_bind_end;
	struct batt_uuid_bind bind_info;
};

int batt_uuid_read_cb(unsigned char version, void *data, int len);
int batt_uuid_write_cb(unsigned char version, void *data, int len);
int batt_read_bind_mesg(void);
void batt_write_bind_mesg(void);
int batt_get_bind_uuid(struct batt_uuid_bind *info);
int batt_clear_bind_uuid(void);
void batt_bind_check(struct batt_chk_data *drv_data,
	const char *uuid, int len);

#endif /* _BATT_UUID_BINDER_H_ */
