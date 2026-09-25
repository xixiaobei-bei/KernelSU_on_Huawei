/*
 * batt_aut_util_v1.c
 *
 * functions for battery symmetric encryption authentication checker
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include "batt_aut_checker.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG batt_aut_util_v1
HWLOG_REGIST();

static int legal_sn_check_0(struct batt_chk_data *drv_data)
{
	drv_data->result.sn_status = SN_PASS;
	return 0;
}

static void data_to_dev_info(struct batt_chk_data *data,
	enum batt_info_subcmd cmd, struct nl_dev_info *info)
{
	info->id_in_grp = data->id_in_grp;
	info->chks_in_grp = data->chks_in_grp;
	info->id_of_grp = data->id_of_grp;
	info->ic_type = data->ic_type;
	info->subcmd = cmd;
}

void bsc_key_func(struct work_struct *work)
{
	struct batt_chk_data *drv_data = container_of(work,
		struct batt_chk_data, check_key_work);
	struct batt_ct_wrapper_ops *bco = &drv_data->bco;
	struct batt_res res;
	struct nl_dev_info info;

	hwlog_info("checker no:%d group:%d prepare to certification\n",
		drv_data->id_in_grp, drv_data->id_of_grp);
	if (bco->prepare(drv_data, RES_CT, &res)) {
		hwlog_err("checker no:%d group:%d prepear for key failed\n",
			drv_data->id_in_grp, drv_data->id_of_grp);
		drv_data->result.key_status = KEY_FAIL_IC;
		return;
	}

	if (res.len > 0) {
		data_to_dev_info(drv_data, CT_PREPARE, &info);
		if (send_batt_info_mesg(&info, (void *)res.data, res.len)) {
			hwlog_err("send attributes failed in %s\n", __func__);
			drv_data->result.key_status = KEY_FAIL_TIMEOUT;
			return;
		}
		wait_for_completion(&drv_data->key_prepare_ready);
	}

	hwlog_info("checker no:%d group:%d certification start\n",
		drv_data->id_in_grp, drv_data->id_of_grp);
	if (bco->certification(drv_data, &drv_data->key_res,
		&drv_data->result.key_status)) {
		hwlog_err("checker no:%d group:%d certification failed\n",
			drv_data->id_in_grp, drv_data->id_of_grp);
		drv_data->result.key_status = KEY_FAIL_IC;
		return;
	}
}

void bsc_sn_func(struct work_struct *work)
{
	struct batt_chk_data *drv_data = container_of(work,
		struct batt_chk_data, check_sn_work);
	struct batt_res res;
	struct batt_ct_wrapper_ops *bco = &drv_data->bco;
	struct nl_dev_info info;
	int sn_fail = 0;

sn_func_entry:
	if (bco->prepare(drv_data, RES_SN, &res)) {
		hwlog_err("checker no:%d group:%d key prepear failed in %s\n",
			drv_data->id_in_grp, drv_data->id_of_grp, __func__);
		drv_data->result.sn_status = SN_FAIL_IC;
		return;
	}

	if (res.len > 0) {
		data_to_dev_info(drv_data, SN_PREPARE, &info);
		if (send_batt_info_mesg(&info, (void *)res.data, res.len)) {
			hwlog_err("checker no:%d group:%d send attr fail %s\n",
				drv_data->id_in_grp,
				drv_data->id_of_grp,
				__func__);
			drv_data->result.sn_status = SN_FAIL_TIMEOUT;
			return;
		}
		wait_for_completion(&drv_data->sn_prepare_ready);
	}

	if (bco->get_batt_sn(drv_data, &drv_data->sn_res, &drv_data->sn,
		&drv_data->sn_len)) {
		hwlog_err("battery no:%d group:%d get batt sn failed in %s\n",
			drv_data->id_in_grp, drv_data->id_of_grp, __func__);
		sn_fail++;
		hwlog_err("get batt sn fail %d@%d\n", sn_fail,
			BATT_SN_READ_RETRY);
		if (sn_fail >= BATT_SN_READ_RETRY) {
			drv_data->result.sn_status = SN_FAIL_IC;
			return;
		}
		goto sn_func_entry;
	}

	if (legal_sn_check_0(drv_data)) {
		hwlog_err("battery no:%d group:%d sn_checker failed in %s\n",
			drv_data->id_in_grp, drv_data->id_of_grp, __func__);
		return;
	}
}

static int bsc_charge_cycles_cb(struct notifier_block *nb,
	unsigned long action, void *data)
{
	int charge_cycles;
	int ret;
	struct batt_chk_data *drv_data = container_of(nb,
		struct batt_chk_data, batt_cycles_listener);
	enum batt_match_type temp = BATTERY_UNREMATCHABLE;

	if (action == HISI_EEPROM_CYC) {
		if (!data) {
			hwlog_err("null data point found in %s\n", __func__);
			return NOTIFY_BAD;
		}

		/* coul-data:charger_cycles is 100*real charge cycles */
		charge_cycles = (*(unsigned int *)data) / 100;
		hwlog_info("%s recv charge cycles:%u\n", __func__,
			*(unsigned int *)data);
		if ((drv_data->batt_rematch_current == BATTERY_REMATCHABLE) &&
			(drv_data->batt_rematch_onboot ==
			BATTERY_REMATCHABLE) &&
			(drv_data->free_cycles <= charge_cycles)) {
			ret = drv_data->bco.set_batt_safe_info(drv_data,
				BATT_MATCH_ABILITY, &temp);
			if (ret)
				hwlog_err("set battery old failed in %s\n",
					__func__);
			else
				drv_data->batt_rematch_current =
					BATTERY_UNREMATCHABLE;
		}
		return NOTIFY_OK;
	}

	return NOTIFY_DONE;
}

int bsc_register_cycles_listener(struct batt_chk_data *drv_data)
{
	int ret;

	ret = drv_data->bco.get_batt_safe_info(drv_data, BATT_MATCH_ABILITY,
		&drv_data->batt_rematch_onboot);
	if (ret) {
		hwlog_err("get battery matchable status failed\n");
		return BATTERY_DRIVER_FAIL;
	}

	/* if on factory mode, battery always rematchable */
#ifdef BATTBD_FORCE_MATCH
	drv_data->batt_rematch_onboot = BATTERY_REMATCHABLE;
#endif /* BATTBD_FORCE_MATCH */
	drv_data->batt_rematch_current = drv_data->batt_rematch_onboot;
	if (drv_data->batt_rematch_onboot == BATTERY_REMATCHABLE) {
		/* register notifier list for update battery to unrematchable */
		drv_data->batt_cycles_listener.notifier_call =
			bsc_charge_cycles_cb;
		if (hisi_coul_register_blocking_notifier(
			&drv_data->batt_cycles_listener)) {
			hwlog_err("regist to coul failed in %s\n", __func__);
			return BATTERY_DRIVER_FAIL;
		}
	}

	return BATTERY_DRIVER_SUCCESS;
}
