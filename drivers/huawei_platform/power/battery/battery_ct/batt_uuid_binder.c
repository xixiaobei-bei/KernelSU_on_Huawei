/*
 * batt_uuid_binder.c
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

#include "batt_uuid_binder.h"
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/power_mesg_srv.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG battery_uuid_binder
HWLOG_REGIST();

static struct batt_bind_data g_bind_info;

static void batt_binder_init(void)
{
	if (g_bind_info.init_sign)
		return;

	memset(&g_bind_info, 0, sizeof(g_bind_info));
	g_bind_info.bind_info.new_board = NEW_BOARD;
	g_bind_info.bind_info.record.version = RAW_UUID_BIND_VER;
	mutex_init(&g_bind_info.lock);
	init_completion(&g_bind_info.uuid_record_ready);
	init_completion(&g_bind_info.write_bind_end);

	/* 1:bind info has been initialize */
	g_bind_info.init_sign = 1;
}

int batt_uuid_read_cb(unsigned char version, void *data, int len)
{
	if (!data || (len != sizeof(struct batt_uuid_bind))) {
		hwlog_err("battery bind data error length is %d\n", len);
		return -1;
	}

	hwlog_info("read battery uuid bind info success\n");
	mutex_lock(&g_bind_info.lock);
	if (!g_bind_info.read_wait_sign) {
		mutex_unlock(&g_bind_info.lock);
		return 0;
	}
	memcpy(&g_bind_info.bind_info, data, len);
	mutex_unlock(&g_bind_info.lock);

	complete(&g_bind_info.uuid_record_ready);
	return 0;
}

int batt_uuid_write_cb(unsigned char version, void *data, int len)
{
	if ((len != strlen(POWER_NORMAL_CBMESG)) ||
		(memcmp(POWER_NORMAL_CBMESG, data, len)))
		return 0;

	hwlog_info("write battery uuid bind info success\n");
	mutex_lock(&g_bind_info.lock);
	/* clear update sign */
	g_bind_info.update_sign = 0;
	mutex_unlock(&g_bind_info.lock);

	complete(&g_bind_info.write_bind_end);
	return 0;
}

int batt_read_bind_mesg(void)
{
	int i;
	int left;
	int bind_ver = RAW_UUID_BIND_VER;

	batt_binder_init();

	hwlog_info("send read battery bind mesg\n");
	for (i = 0; i < POWER_MESG_RETRY_TIMES; i++) {
		if (send_power_genl_mesg(POWER_GENL_CMD_BATT_BIND_RD,
			&bind_ver, sizeof(bind_ver))) {
			hwlog_err("read bind power mesg send failed\n");
			return -1;
		}

		mutex_lock(&g_bind_info.lock);
		g_bind_info.read_wait_sign = 1;
		mutex_unlock(&g_bind_info.lock);
		left = wait_for_completion_timeout(
			&g_bind_info.uuid_record_ready,
			POWER_MESG_TIME_OUT);
		mutex_lock(&g_bind_info.lock);
		g_bind_info.read_wait_sign = 0;
		mutex_unlock(&g_bind_info.lock);
		if (left > 0)
			return 0;
	}

	return -1;
}

void batt_write_bind_mesg(void)
{
	int ret;
	int i;

	if (!g_bind_info.init_sign)
		return;

	for (i = 0; i < POWER_MESG_RETRY_TIMES; i++) {
		hwlog_info("send write battery bind %d mesg\n", i);
		mutex_lock(&g_bind_info.lock);
		if (!g_bind_info.update_sign) {
			mutex_unlock(&g_bind_info.lock);
			return;
		}

		ret = send_power_genl_mesg(POWER_GENL_CMD_BATT_BIND_WR,
			&g_bind_info.bind_info.record,
			sizeof(g_bind_info.bind_info.record));
		mutex_unlock(&g_bind_info.lock);
		if (ret) {
			hwlog_err("write bind power mesg send failed\n");
			return;
		}

		wait_for_completion_timeout(&g_bind_info.write_bind_end,
			POWER_MESG_TIME_OUT);
		if (g_bind_info.update_sign == 0)
			return;
	}
}

int batt_get_bind_uuid(struct batt_uuid_bind *info)
{
	hwlog_info("send read bind mesg in %s\n", __func__);
	batt_binder_init();
	if (batt_read_bind_mesg())
		return -1;

	mutex_lock(&g_bind_info.lock);
	memcpy(info, &g_bind_info.bind_info, sizeof(struct batt_uuid_bind));
	mutex_unlock(&g_bind_info.lock);
	return 0;
}

int batt_clear_bind_uuid(void)
{
	batt_binder_init();

	mutex_lock(&g_bind_info.lock);
	memset(&g_bind_info.bind_info.record, 0, sizeof(struct batt_uuid_record));
	g_bind_info.bind_info.record.version = RAW_UUID_BIND_VER;
	/* 1:enable update sign to refresh tee */
	g_bind_info.update_sign = 1;
	mutex_unlock(&g_bind_info.lock);

	batt_write_bind_mesg();
	return 0;
}

static void batt_move_uuid_to_head(int i)
{
	char temp[MAX_BATT_UUID_LEN] = { 0 };
	struct batt_uuid_record *uuid_record = &g_bind_info.bind_info.record;

	if (i <= 0)
		return;
	memcpy(temp, uuid_record->uuid[i], MAX_BATT_UUID_LEN);

	for (; i > 0; i--)
		memcpy(uuid_record->uuid[i], uuid_record->uuid[i - 1],
			MAX_BATT_UUID_LEN);

	memcpy(uuid_record->uuid[0], temp, MAX_BATT_UUID_LEN);
}

static int batt_check_binded(const char *uuid, int len)
{
	int i;
	struct batt_uuid_record *uuid_record = &g_bind_info.bind_info.record;

	for (i = 0; i < MAX_BATT_BIND_NUM; i++) {
		if (memcmp(uuid_record->uuid[i], uuid, len))
			continue;

		hwlog_info("board and battery well matched\n");
		if ((i > 0) ||
			(uuid_record->version != RAW_UUID_BIND_VER)) {
			batt_move_uuid_to_head(i);
			uuid_record->version = RAW_UUID_BIND_VER;
			/* 1:enable update sign to refresh tee */
			g_bind_info.update_sign = 1;
		}
		return 0;
	}

	return -1;
}

#ifdef BATTBD_FORCE_MATCH
static void batt_force_bind(struct batt_chk_data *drv_data,
	const char *uuid, int len)
{
	int i;
	struct batt_uuid_record *uuid_record = &g_bind_info.bind_info.record;

	for (i = 0; i < MAX_BATT_BIND_NUM; i++)
		memcpy(uuid_record->uuid[i], uuid, len);
	uuid_record->version = RAW_UUID_BIND_VER;
	/* 1:enable update sign to refresh tee */
	g_bind_info.update_sign = 1;

	drv_data->result.sn_status = SN_PASS;
	return 0;
}
#endif

static void batt_try_bind_spare(struct batt_chk_data *drv_data)
{
	bool spare_battery = drv_data->batt_src_onboot != BATTERY_ORIGINAL;
	bool new_board = g_bind_info.bind_info.new_board;

	if (spare_battery && new_board)
		drv_data->result.sn_status = SN_NN_REMATCH;
	else if (spare_battery && !new_board)
		drv_data->result.sn_status = SN_OBD_REMATCH;
	else if (!spare_battery && new_board)
		drv_data->result.sn_status = SN_OBT_REMATCH;
	else
		drv_data->result.sn_status = SN_OO_UNMATCH;
}

static void batt_try_bind_new(struct batt_chk_data *drv_data)
{
	bool new_battery = drv_data->batt_rematch_onboot ==
		BATTERY_REMATCHABLE;
	bool new_board = g_bind_info.bind_info.new_board;

	if (new_battery && new_board)
		drv_data->result.sn_status = SN_NN_REMATCH;
	else if (new_battery && !new_board)
		drv_data->result.sn_status = SN_OBD_REMATCH;
	else if (!new_battery && new_board)
		drv_data->result.sn_status = SN_OBT_REMATCH;
	else
		drv_data->result.sn_status = SN_OO_UNMATCH;
}

static void batt_try_bind(struct batt_chk_data *drv_data,
	const char *uuid, int len)
{
	int i;
	struct batt_uuid_record *uuid_record = &g_bind_info.bind_info.record;

	switch (drv_data->act_state) {
	case SIGN_FAIL:
		return;
	case ACT_PASS:
		batt_try_bind_spare(drv_data);
		break;
	case RETROFIT_PASS:
		batt_try_bind_new(drv_data);
		break;
	default:
		return;
	}

	if ((drv_data->result.sn_status == SN_OBT_REMATCH) ||
	    (drv_data->result.sn_status == SN_OBD_REMATCH) ||
	    (drv_data->result.sn_status == SN_NN_REMATCH)) {
		for (i = MAX_BATT_BIND_NUM - 1; i > 0; i--)
			memcpy(uuid_record->uuid[i], uuid_record->uuid[i - 1],
				MAX_BATT_UUID_LEN);

		memcpy(uuid_record->uuid[0], uuid, len);
		uuid_record->version = RAW_UUID_BIND_VER;
		/* 1:enable update sign to refresh tee */
		g_bind_info.update_sign = 1;
	}
}

void batt_bind_check(struct batt_chk_data *drv_data,
	const char *uuid, int len)
{
	if (!drv_data || !uuid || (len > MAX_BATT_UUID_LEN))
		return;
	if (!g_bind_info.init_sign)
		return;

	mutex_lock(&g_bind_info.lock);
	if (!batt_check_binded(uuid, len)) {
		drv_data->result.sn_status = SN_PASS;
		goto unlock_mutex;
	}

#ifdef BATTBD_FORCE_MATCH
	batt_force_bind(drv_data, uuid, len);
#else
	batt_try_bind(drv_data, uuid, len);
#endif

unlock_mutex:
	mutex_unlock(&g_bind_info.lock);
}

