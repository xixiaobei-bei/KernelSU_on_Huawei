/*
 * batt_aut_util_v2.c
 *
 * functions for battery ssymmetric encryption authentication checker
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

#include "batt_aut_checker.h"
#include "batt_uuid_binder.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG batt_aut_util_v2
HWLOG_REGIST();

static void data_to_dev_info(struct batt_chk_data *data,
	enum batt_info_subcmd cmd, struct nl_dev_info *info)
{
	info->id_in_grp = data->id_in_grp;
	info->chks_in_grp = data->chks_in_grp;
	info->id_of_grp = data->id_of_grp;
	info->ic_type = data->ic_type;
	info->subcmd = cmd;
}

static int send_batt_res_mesg(struct batt_chk_data *drv_data,
	unsigned char subcmd, void *data, unsigned int len)
{
	struct nl_dev_info info;

	data_to_dev_info(drv_data, subcmd, &info);
	if (send_batt_info_mesg(&info, data, len)) {
		hwlog_err("checker no:%c group:%c send %c attr fail\n",
			drv_data->id_in_grp, drv_data->id_of_grp, subcmd);
		return -1;
	}
	return 0;
}

static int prepare_key_res(struct batt_chk_data *drv_data)
{
	struct power_genl_attr res;
	struct batt_ct_wrapper_ops *bco = &drv_data->bco;

	if (!bco->prepare)
		return -1;

	if (bco->prepare(drv_data, RES_CT, &res)) {
		hwlog_err("checker no:%d group:%d prepare for key failed\n",
			drv_data->id_in_grp,
			drv_data->id_of_grp);
		return -1;
	}

	if (send_batt_res_mesg(drv_data, CT_PREPARE, (void *)res.data, res.len))
		return -1;

	wait_for_completion(&drv_data->key_prepare_ready);

	if (!drv_data->key_res.data ||
		(drv_data->key_res.len == 0)) {
		hwlog_err("battery authentication resource error %u\n",
			drv_data->key_res.len);
		return -1;
	}

	return 0;
}

static int check_ecc_key(struct batt_chk_data *drv_data)
{
	struct power_genl_attr res;
	struct batt_ct_wrapper_ops *bco = &drv_data->bco;

	hwlog_info("checker no:%c group:%c certification start\n",
		drv_data->id_in_grp, drv_data->id_of_grp);
	if (bco->prepare(drv_data, RES_CT, &res)) {
		hwlog_err("checker no:%c group:%c prepear for key failed\n",
			drv_data->id_in_grp, drv_data->id_of_grp);
		return -1;
	}

	if (bco->certification(drv_data, &drv_data->key_res,
		&drv_data->result.key_status)) {
		hwlog_err("checker no:%c group:%c certification failed\n",
			drv_data->id_in_grp, drv_data->id_of_grp);
		return -1;
	}

	return 0;
}

static int check_act_key(struct batt_chk_data *drv_data)
{
	struct power_genl_attr ecc_res;
	struct power_genl_attr act_res;
	struct batt_act_request request;
	struct batt_ct_wrapper_ops *bco = &drv_data->bco;

	/* prepare ecc signature */
	if (bco->prepare(drv_data, RES_CT, &ecc_res) || !ecc_res.data ||
		(ecc_res.len == 0) || (ecc_res.len > BATT_MAX_SIGN_LEN)) {
		hwlog_err("checker no:%c group:%c prepear for ecc sign %u failed\n",
			drv_data->id_in_grp, drv_data->id_of_grp, ecc_res.len);
		return -1;
	}

	/* prepare act signature */
	if (bco->prepare(drv_data, RES_ACT, &act_res) || !act_res.data ||
		(act_res.len == 0) || (act_res.len > BATT_MAX_SIGN_LEN) ||
		(act_res.data[0] > BATT_MAX_SIGN_LEN)) {
		hwlog_err("checker no:%c group:%c prepear for act sign %u failed\n",
			drv_data->id_in_grp, drv_data->id_of_grp, act_res.len);
		return -1;
	}

	memset(&request, 0, sizeof(request));
	memcpy(request.act_key_id, BATT_ACT_SIGN_ID, strlen(BATT_ACT_SIGN_ID));
	memcpy(request.retrofit_key_id, BATT_RETROFIT_SIGN_ID,
		strlen(BATT_RETROFIT_SIGN_ID));
	memcpy(request.ecc_signature, ecc_res.data, ecc_res.len);
	request.ecc_sign_len = ecc_res.len;
	memcpy(request.act_signature, act_res.data + 1, act_res.data[0]);
	request.act_sign_len = act_res.data[0];

	if (send_batt_res_mesg(drv_data, ACT_SIGN, (void *)&request,
		sizeof(request)))
		return -1;

	wait_for_completion(&drv_data->act_sign_complete);
	return 0;
}

#ifdef BATTBD_FORCE_MATCH
static bool check_cert_ready(struct batt_chk_data *drv_data)
{
	enum batt_cert_state state;

	if (drv_data->bco.get_batt_safe_info(drv_data, BATT_CERT_READY,
		&state))
		return true;

	if (state == CERT_UNREADY)
		return false;
	return true;
}
#endif

void byc_key_func(struct work_struct *work)
{
	struct batt_chk_data *drv_data = container_of(work,
		struct batt_chk_data, check_key_work);

	hwlog_info("checker no:%c group:%c prepare to check\n",
		drv_data->id_in_grp, drv_data->id_of_grp);
#ifdef BATTBD_FORCE_MATCH
	if (!check_cert_ready(drv_data)) {
		drv_data->result.key_status = KEY_PASS;
		return;
	}
#endif
	if (prepare_key_res(drv_data)) {
		drv_data->result.key_status = KEY_FAIL_TIMEOUT;
		return;
	}

	if (check_ecc_key(drv_data)) {
		drv_data->result.key_status = KEY_FAIL_IC;
		return;
	}

	if (check_act_key(drv_data) ||
		(drv_data->act_state == SIGN_FAIL)) {
		hwlog_err("checker no:%c group:%c battery act failed %d\n",
			drv_data->id_in_grp, drv_data->id_of_grp,
			drv_data->act_state);
		drv_data->result.key_status = KEY_FAIL_ACT;
		return;
	}
	drv_data->result.key_status = KEY_PASS;
}

void byc_act_sign_func(struct work_struct *work)
{
	struct batt_chk_data *drv_data = container_of(work,
		struct batt_chk_data, act_signature_work);

	hwlog_info("checker no:%c group:%c prepare to act\n",
		drv_data->id_in_grp, drv_data->id_of_grp);

	drv_data->act_state = SIGN_FAIL;
	if (check_act_key(drv_data) ||
		(drv_data->act_state == SIGN_FAIL))
		hwlog_err("checker no:%c group:%c battery act failed %d\n",
			drv_data->id_in_grp, drv_data->id_of_grp,
			drv_data->act_state);
}

static int check_batt_sn(struct batt_chk_data *drv_data)
{
	struct power_genl_attr res;
	struct batt_ct_wrapper_ops *bco = &drv_data->bco;
	int sn_fail = 0;

sn_func_entry:
	if (bco->prepare(drv_data, RES_SN, &res)) {
		hwlog_err("checker no:%c group:%c key prepear failed in %s\n",
			drv_data->id_in_grp, drv_data->id_of_grp, __func__);
		drv_data->result.sn_status = SN_FAIL_IC;
		return -1;
	}

	if (res.len > 0) {
		if (send_batt_res_mesg(drv_data, SN_PREPARE,
			(void *)res.data, res.len)) {
			drv_data->result.sn_status = SN_FAIL_TIMEOUT;
			return -1;
		}
		wait_for_completion(&drv_data->sn_prepare_ready);
	}

	if (bco->get_batt_sn(drv_data, &drv_data->sn_res, &drv_data->sn,
		&drv_data->sn_len)) {
		hwlog_err("battery no:%c group:%c get batt sn failed %d in %s\n",
			drv_data->id_in_grp, drv_data->id_of_grp, sn_fail,
			__func__);

		sn_fail++;
		if (sn_fail >= BATT_SN_READ_RETRY) {
			drv_data->result.sn_status = SN_FAIL_IC;
			return -1;
		}
		goto sn_func_entry;
	}

	return 0;
}

static void check_uuid_bind(struct batt_chk_data *drv_data)
{
	const unsigned char *uuid = NULL;
	unsigned int len;
	struct batt_ct_wrapper_ops *bco = &drv_data->bco;

	if (!bco->get_ic_uuid) {
		hwlog_err("battery no:%c group:%c get_ic_uuid is null in %s\n",
			drv_data->id_in_grp, drv_data->id_of_grp,
			__func__);
		drv_data->result.sn_status = SN_FAIL_IC;
		return;
	}

	if (bco->get_ic_uuid(drv_data, &uuid, &len)) {
		hwlog_err("battery no:%c group:%c get uuid failed in %s\n",
			drv_data->id_in_grp, drv_data->id_of_grp,
			__func__);
		drv_data->result.sn_status = SN_FAIL_IC;
		return;
	}

	batt_bind_check(drv_data, uuid, len);

	hwlog_info("battery no:%c group:%c uuid check status %d\n",
		drv_data->id_in_grp, drv_data->id_of_grp,
		drv_data->result.sn_status);
}

void byc_sn_func(struct work_struct *work)
{
	struct batt_chk_data *drv_data = container_of(work,
		struct batt_chk_data, check_sn_work);

#ifdef BATTBD_FORCE_MATCH
	if (!check_cert_ready(drv_data)) {
		drv_data->result.sn_status = SN_PASS;
		return;
	}
#endif

	if (check_batt_sn(drv_data))
		return;

	check_uuid_bind(drv_data);
}

static void byc_rematchable_update(struct batt_chk_data *drv_data)
{
	int real_ic_cycles = drv_data->ic_cycles / BATT_IC_CYCLES_RATIO;

	if ((drv_data->batt_rematch_current != BATTERY_REMATCHABLE) ||
		(real_ic_cycles <= drv_data->free_cycles))
		return;

	drv_data->batt_rematch_current = BATTERY_UNREMATCHABLE;
	hwlog_info("set battery unrematchable, ic cycles:%d:%u\n",
		drv_data->ic_cycles, drv_data->free_cycles);
}

static void byc_batt_source_update(struct batt_chk_data *drv_data,
	int real_cycles)
{
	enum batt_source temp = BATTERY_ORIGINAL;

	if ((drv_data->batt_src_current != BATTERY_SPARE_PART) ||
		(real_cycles <= drv_data->spare_cycles))
		return;

	if (drv_data->bco.set_batt_safe_info(drv_data,
		BATT_SPARE_PART, &temp)) {
		hwlog_err("clear battery spare sign failed in %s\n", __func__);
	} else {
		drv_data->batt_src_current = BATTERY_ORIGINAL;
		hwlog_info("clear battery spare sign, ic cycles:%d:%u\n",
			drv_data->ic_cycles, drv_data->spare_cycles);
	}
}

static void byc_ic_cycles_update(struct batt_chk_data *drv_data,
	int current_cycles)
{
	int coul_add_cycles = current_cycles - drv_data->coul_cycles;
	int ic_add_cycles = coul_add_cycles / BATT_RELATIVE_CYCLES_RATIO;

	if (ic_add_cycles == 0)
		return;

	if (drv_data->bco.set_batt_safe_info(drv_data,
		BATT_CHARGE_CYCLES, &ic_add_cycles)) {
		hwlog_err("increase ic charge cycles failed in %s\n", __func__);
		return;
	}
	drv_data->coul_cycles += ic_add_cycles * BATT_RELATIVE_CYCLES_RATIO;

	if (drv_data->bco.get_batt_safe_info(drv_data, BATT_CHARGE_CYCLES,
		&drv_data->ic_cycles)) {
		hwlog_err("get battery charge cycles from ic failed in %s\n",
			__func__);
		return;
	}

	hwlog_info("after refresh charge cycles coul:%d, ic:%d\n",
		drv_data->coul_cycles, drv_data->ic_cycles);
	byc_rematchable_update(drv_data);
	byc_batt_source_update(drv_data,
		drv_data->ic_cycles / BATT_IC_CYCLES_RATIO);
}

static int byc_charge_cycles_cb(struct notifier_block *nb,
	unsigned long action, void *data)
{
	int current_coul_cycles;
	struct batt_chk_data *drv_data = container_of(nb,
		struct batt_chk_data, batt_cycles_listener);

	if (action != BATT_EEPROM_CYC)
		return NOTIFY_DONE;
	if (!data)
		return NOTIFY_BAD;

	memcpy(&current_coul_cycles, data, sizeof(current_coul_cycles));
	hwlog_info("recv charge cycles:%d, coul:%d, ic:%d\n",
		current_coul_cycles, drv_data->coul_cycles,
		drv_data->ic_cycles);
	if (drv_data->ic_cycles == BATT_INVALID_CYCLES) {
		byc_batt_source_update(drv_data,
			current_coul_cycles / BATT_IC_CYCLES_RATIO);
		return NOTIFY_OK;
	}
	if ((drv_data->coul_cycles == BATT_INVALID_CYCLES) ||
		(drv_data->coul_cycles >= current_coul_cycles)) {
		drv_data->coul_cycles = current_coul_cycles;
		return NOTIFY_OK;
	}

	byc_ic_cycles_update(drv_data, current_coul_cycles);
	return NOTIFY_OK;
}

int byc_register_cycles_listener(struct batt_chk_data *drv_data)
{
#ifndef BATTBD_FORCE_MATCH
	int ret;

	if (drv_data->bco.get_batt_safe_info(drv_data, BATT_SPARE_PART,
		&drv_data->batt_src_onboot)) {
		hwlog_err("get battery cell type from ic failed\n");
		goto ic_driver_fail;
	}
	drv_data->batt_src_current = drv_data->batt_src_onboot;
	drv_data->coul_cycles = BATT_INVALID_CYCLES;

	ret = drv_data->bco.get_batt_safe_info(drv_data, BATT_CHARGE_CYCLES,
		&drv_data->ic_cycles);
	if (ret) {
		hwlog_err("get battery charge cycles from ic failed\n");
		goto ic_driver_fail;
	}

	drv_data->batt_rematch_onboot = BATTERY_REMATCHABLE;
	if (drv_data->ic_cycles > drv_data->free_cycles)
		drv_data->batt_rematch_onboot = BATTERY_UNREMATCHABLE;
	drv_data->batt_rematch_current = drv_data->batt_rematch_onboot;

	/* register notifier list for update battery to unrematchable */
	drv_data->batt_cycles_listener.notifier_call = byc_charge_cycles_cb;
	if (coul_register_blocking_notifier(
		&drv_data->batt_cycles_listener)) {
		hwlog_err("regist to coul failed in %s\n", __func__);
		return BATTERY_DRIVER_FAIL;
	}

	return BATTERY_DRIVER_SUCCESS;
ic_driver_fail:
#endif /* BATTBD_FORCE_MATCH */
	/* if on factory mode, battery always rematchable */
	drv_data->batt_src_onboot = BATTERY_UNKNOWN;
	drv_data->batt_src_current = BATTERY_UNKNOWN;
	drv_data->batt_rematch_onboot = BATTERY_REMATCHABLE;
	drv_data->batt_rematch_current = BATTERY_REMATCHABLE;
	return BATTERY_DRIVER_SUCCESS;
}
