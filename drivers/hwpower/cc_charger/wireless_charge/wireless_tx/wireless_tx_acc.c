/*
 * wireless_tx_acc.c
 *
 * wireless accessory reverse charging driver
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

#include <linux/module.h>
#include <linux/slab.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/wireless_charge/wireless_accessory.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_acc.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>
#include <chipset_common/hwpower/protocol/wireless_protocol.h>
#include <chipset_common/hwpower/protocol/wireless_protocol_qi.h>

#define HWLOG_TAG wireless_tx_acc
HWLOG_REGIST();

static struct qi_protocol_acc_device_info g_wltx_acc_dev_info;

/* each dev should done */
static struct wireless_acc_key_info g_wltx_acc_info_tab[WLTX_ACC_INFO_END] = {
	{ "DEVICENO", "0" },
	{ "DEVICESTATE", "UNKNOWN" },
	{ "DEVICEMAC", "FF:FF:FF:FF:FF:FF" },
	{ "DEVICEMODELID", "000000" },
	{ "DEVICESUBMODELID", "00" },
	{ "DEVICEVERSION", "00" },
	{ "DEVICEBUSINESS", "00" },
};

void wltx_acc_set_dev_state(u8 dev_state)
{
	wireless_acc_set_tx_dev_state(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI, dev_state);
}

static int wltx_acc_get_dev_info_cnt(struct qi_protocol_acc_device_info *acc_dev_info)
{
	return wireless_acc_get_tx_dev_info_cnt(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI,
		&acc_dev_info->dev_info_cnt);
}

static int wltx_acc_reset_dev_info_cnt(u8 dev_info_cnt)
{
	return wireless_acc_set_tx_dev_info_cnt(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI,
		dev_info_cnt);
}

static int wltx_acc_get_dev_state(struct qi_protocol_acc_device_info *acc_dev_info)
{
	return wireless_acc_get_tx_dev_state(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI,
		&acc_dev_info->dev_state);
}

static int wltx_acc_get_dev_no(struct qi_protocol_acc_device_info *acc_dev_info)
{
	return wireless_acc_get_tx_dev_no(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI,
		&acc_dev_info->dev_no);
}

static int wltx_acc_get_dev_mac(struct qi_protocol_acc_device_info *acc_dev_info)
{
	return wireless_acc_get_tx_dev_mac(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI,
		acc_dev_info->dev_mac, QI_ACC_TX_DEV_MAC_LEN);
}

static int wltx_acc_get_dev_model_id(struct qi_protocol_acc_device_info *acc_dev_info)
{
	return wireless_acc_get_tx_dev_model_id(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI,
		acc_dev_info->dev_model_id, QI_ACC_TX_DEV_MODELID_LEN);
}

static int wltx_acc_get_dev_submodel_id(struct qi_protocol_acc_device_info *acc_dev_info)
{
	return wireless_acc_get_tx_dev_submodel_id(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI,
		&acc_dev_info->dev_submodel_id);
}

static int wltx_acc_get_dev_version(struct qi_protocol_acc_device_info *acc_dev_info)
{
	return wireless_acc_get_tx_dev_version(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI,
		&acc_dev_info->dev_version);
}

static int wltx_acc_get_dev_business(struct qi_protocol_acc_device_info *acc_dev_info)
{
	return wireless_acc_get_tx_dev_business(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI,
		&acc_dev_info->dev_business);
}

static int wltx_acc_get_info(struct qi_protocol_acc_device_info *acc_dev_info)
{
	int ret;

	ret = wltx_acc_get_dev_info_cnt(acc_dev_info);
	ret += wltx_acc_get_dev_state(acc_dev_info);
	ret += wltx_acc_get_dev_no(acc_dev_info);
	ret += wltx_acc_get_dev_mac(acc_dev_info);
	ret += wltx_acc_get_dev_model_id(acc_dev_info);
	ret += wltx_acc_get_dev_submodel_id(acc_dev_info);
	ret += wltx_acc_get_dev_version(acc_dev_info);
	ret += wltx_acc_get_dev_business(acc_dev_info);

	return ret;
}

static void wltx_acc_notify_android_uevent(struct qi_protocol_acc_device_info *acc_dev_info)
{
	if (!acc_dev_info)
		return;

	if ((acc_dev_info->dev_no < ACC_DEV_NO_BEGIN) ||
		(acc_dev_info->dev_no >= ACC_DEV_NO_END))
		return;

	switch (acc_dev_info->dev_state) {
	case ACC_DEVICE_ONLINE:
		snprintf(g_wltx_acc_info_tab[WLTX_ACC_INFO_STATE].value,
			ACC_VALUE_MAX_LEN, "%s", ACC_CONNECTED_STR);
		break;
	case ACC_DEVICE_OFFLINE:
		snprintf(g_wltx_acc_info_tab[WLTX_ACC_INFO_STATE].value,
			ACC_VALUE_MAX_LEN, "%s", ACC_DISCONNECTED_STR);
		break;
	default:
		snprintf(g_wltx_acc_info_tab[WLTX_ACC_INFO_STATE].value,
			ACC_VALUE_MAX_LEN, "%s", ACC_UNKNOWN_STR);
		break;
	}

	snprintf(g_wltx_acc_info_tab[WLTX_ACC_INFO_NO].value, ACC_VALUE_MAX_LEN, "%u", acc_dev_info->dev_no);
	/* dev_mac[0 1 2 3 4 5] is BT MAC ADDR */
	snprintf(g_wltx_acc_info_tab[WLTX_ACC_INFO_MAC].value, ACC_VALUE_MAX_LEN,
		"%02x:%02x:%02x:%02x:%02x:%02x", acc_dev_info->dev_mac[0],
		acc_dev_info->dev_mac[1], acc_dev_info->dev_mac[2], acc_dev_info->dev_mac[3],
		acc_dev_info->dev_mac[4], acc_dev_info->dev_mac[5]);
	/* dev_model_id[0 1 2] is BT model id */
	snprintf(g_wltx_acc_info_tab[WLTX_ACC_INFO_MODEL_ID].value,
		ACC_VALUE_MAX_LEN, "%02x%02x%02x", acc_dev_info->dev_model_id[0],
		acc_dev_info->dev_model_id[1], acc_dev_info->dev_model_id[2]);
	snprintf(g_wltx_acc_info_tab[WLTX_ACC_INFO_SUBMODEL_ID].value,
		ACC_VALUE_MAX_LEN, "%02x", acc_dev_info->dev_submodel_id);
	snprintf(g_wltx_acc_info_tab[WLTX_ACC_INFO_VERSION].value,
		ACC_VALUE_MAX_LEN, "%02x", acc_dev_info->dev_version);
	snprintf(g_wltx_acc_info_tab[WLTX_ACC_INFO_BUSINESS].value,
		ACC_VALUE_MAX_LEN, "%02x", acc_dev_info->dev_business);

	wireless_acc_report_uevent(g_wltx_acc_info_tab,
		WLTX_ACC_INFO_END, acc_dev_info->dev_no);
	hwlog_info("notify_android_uevent succ\n");
}

int wltx_acc_get_dev_info_and_notify(void)
{
	int ret;

	ret = wltx_acc_get_info(&g_wltx_acc_dev_info);
	if (ret) {
		hwlog_err("get_acc_info fail, no notify\n");
		return -EINVAL;
	}
	wltx_acc_notify_android_uevent(&g_wltx_acc_dev_info);
	wltx_acc_reset_dev_info_cnt(0);

	return 0;
}
