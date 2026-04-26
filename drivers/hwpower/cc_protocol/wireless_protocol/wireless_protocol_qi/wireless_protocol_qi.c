/*
 * wireless_protocol_qi.c
 *
 * qi protocol driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <chipset_common/hwpower/protocol/wireless_protocol.h>
#include <chipset_common/hwpower/protocol/wireless_protocol_qi.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/wireless_charge/wireless_acc_types.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_acc.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_auth.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_acc.h>

#define HWLOG_TAG wireless_protocol_qi
HWLOG_REGIST();

static struct qi_protocol_dev *g_qi_protocol_dev[WLTRX_IC_MAX];
static struct qi_protocol_handle g_qi_protocol_handle;

static const u8 g_qi_protocol_ask_hdr[] = {
	0, 0x18, 0x28, 0x38, 0x48, 0x58
};
static const u8 g_qi_protocol_fsk_hdr[] = {
	0, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f
};

static const struct wireless_protocol_device_data g_qi_protocol_dev_data[] = {
	{ WIRELESS_DEVICE_ID_IDTP9221, "idtp9221" },
	{ WIRELESS_DEVICE_ID_STWLC68, "stwlc68" },
	{ WIRELESS_DEVICE_ID_IDTP9415, "idtp9415" },
	{ WIRELESS_DEVICE_ID_NU1619, "nu1619" },
	{ WIRELESS_DEVICE_ID_STWLC88, "stwlc88" },
	{ WIRELESS_DEVICE_ID_CPS7181, "cps7181" },
	{ WIRELESS_DEVICE_ID_CPS4057, "cps4057" },
	{ WIRELESS_DEVICE_ID_MT5735, "mt5735" },
	{ WIRELESS_DEVICE_ID_CPS4029, "cps4029" },
};

static int qi_protocol_get_device_id(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_qi_protocol_dev_data); i++) {
		if (!strncmp(str, g_qi_protocol_dev_data[i].name,
			strlen(str)))
			return g_qi_protocol_dev_data[i].id;
	}

	return -EINVAL;
}

static bool qi_protocol_check_ic_type(unsigned int ic_type)
{
	if (wltrx_ic_is_type_valid(ic_type))
		return true;

	hwlog_err("invalid ic_type=%u\n", ic_type);
	return false;
}

static int qi_protocol_get_ic_type(void *dev_data)
{
	int i;

	for (i = 0; i < WLTRX_IC_MAX; i++) {
		if (!g_qi_protocol_dev[i])
			continue;

		if (!g_qi_protocol_dev[i]->p_ops)
			continue;

		if (g_qi_protocol_dev[i]->p_ops->dev_data != dev_data)
			continue;

		return i;
	}

	hwlog_err("ic_type not find\n");
	return WLTRX_IC_MIN;
}

static int qi_protocol_get_bnt_wltx_type(unsigned int ic_type)
{
	return ic_type == WLTRX_IC_AUX ? POWER_BNT_WLTX_AUX : POWER_BNT_WLTX;
}

static struct qi_protocol_dev *qi_protocol_get_dev(unsigned int ic_type)
{
	if (!qi_protocol_check_ic_type(ic_type))
		return NULL;

	if (!g_qi_protocol_dev[ic_type]) {
		hwlog_err("g_qi_protocol_dev is null\n");
		return NULL;
	}

	return g_qi_protocol_dev[ic_type];
}

static struct qi_protocol_ops *qi_protocol_get_ops(unsigned int ic_type)
{
	if (!qi_protocol_check_ic_type(ic_type))
		return NULL;

	if (!g_qi_protocol_dev[ic_type] || !g_qi_protocol_dev[ic_type]->p_ops) {
		hwlog_err("g_qi_protocol_dev or p_ops is null\n");
		return NULL;
	}

	return g_qi_protocol_dev[ic_type]->p_ops;
}

int qi_protocol_ops_register(struct qi_protocol_ops *ops, unsigned int ic_type)
{
	int dev_id;

	if (!qi_protocol_check_ic_type(ic_type))
		return -EINVAL;

	if (!g_qi_protocol_dev[ic_type] || !ops || !ops->chip_name) {
		hwlog_err("g_qi_protocol_dev or ops or chip_name is null\n");
		return -EINVAL;
	}

	dev_id = qi_protocol_get_device_id(ops->chip_name);
	if (dev_id < 0) {
		hwlog_err("%s ops register fail\n", ops->chip_name);
		return -EINVAL;
	}

	g_qi_protocol_dev[ic_type]->p_ops = ops;
	g_qi_protocol_dev[ic_type]->dev_id = dev_id;

	hwlog_info("%d:%s,%u, ops register ok\n", dev_id, ops->chip_name, ic_type);
	return 0;
}

static int qi_protocol_get_acc_dev_no(unsigned int ic_type)
{
	switch (ic_type) {
	case WLTRX_IC_MAIN:
		return ACC_DEV_NO_KB;
	case WLTRX_IC_AUX:
		return ACC_DEV_NO_PEN;
	default:
		return -EINVAL;
	}
}

struct qi_protocol_handle *qi_protocol_get_handle(void)
{
	return &g_qi_protocol_handle;
}

static int qi_protocol_send_msg(unsigned int ic_type, u8 cmd, u8 *para, int para_len, u8 *data,
	int data_len, int retrys)
{
	int i;
	int ret;
	struct qi_protocol_ops *l_ops = qi_protocol_get_ops(ic_type);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->send_msg || !l_ops->receive_msg) {
		hwlog_err("send_msg or receive_msg is null\n");
		return -EINVAL;
	}

	for (i = 0; i < retrys; i++) {
		ret = l_ops->send_msg(cmd, para, para_len, l_ops->dev_data);
		if (ret) {
			hwlog_err("0x%x msg send fail, retry %d\n", cmd, i);
			continue;
		}

		ret = l_ops->receive_msg(data, data_len, l_ops->dev_data);
		if (ret) {
			hwlog_err("0x%x msg receive fail, retry %d\n", cmd, i);
			continue;
		}

		break;
	}

	if (i >= retrys)
		return -EINVAL;

	/* protocol define: the first byte of the response must be a command */
	if (data[QI_ACK_CMD_OFFSET] != cmd) {
		hwlog_err("data[%d] 0x%x not equal cmd 0x%x\n",
			QI_ACK_CMD_OFFSET, data[QI_ACK_CMD_OFFSET], cmd);
		return -EINVAL;
	}

	return 0;
}

static int qi_protocol_send_msg_ack(unsigned int ic_type, u8 cmd, u8 *para, int para_len,
	int retrys)
{
	int i;
	int ret;
	struct qi_protocol_ops *l_ops = qi_protocol_get_ops(ic_type);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->send_msg_with_ack) {
		hwlog_err("send_msg_with_ack is null\n");
		return -EINVAL;
	}

	for (i = 0; i < retrys; i++) {
		ret = l_ops->send_msg_with_ack(cmd, para, para_len, l_ops->dev_data);
		if (ret) {
			hwlog_err("0x%x msg_ack send fail, retry %d\n", cmd, i);
			continue;
		}

		break;
	}

	if (i >= retrys)
		return -EINVAL;

	return 0;
}

static int qi_protocol_send_msg_only(unsigned int ic_type, u8 cmd, u8 *para, int para_len,
	int retrys)
{
	int i;
	int ret;
	struct qi_protocol_ops *l_ops = qi_protocol_get_ops(ic_type);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->send_msg) {
		hwlog_err("send_msg is null\n");
		return -EINVAL;
	}

	for (i = 0; i < retrys; i++) {
		ret = l_ops->send_msg(cmd, para, para_len, l_ops->dev_data);
		if (ret) {
			hwlog_err("0x%x msg send fail, retry %d\n", cmd, i);
			continue;
		}

		break;
	}

	if (i >= retrys)
		return -EINVAL;

	return 0;
}

static int qi_protocol_send_fsk_msg(unsigned int ic_type, u8 cmd, u8 *para, int para_len,
	int retrys)
{
	int i;
	int ret;
	struct qi_protocol_ops *l_ops = qi_protocol_get_ops(ic_type);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->send_fsk_msg) {
		hwlog_err("send_fsk_msg is null\n");
		return -EINVAL;
	}

	for (i = 0; i < retrys; i++) {
		ret = l_ops->send_fsk_msg(cmd, para, para_len, l_ops->dev_data);
		if (ret) {
			hwlog_err("0x%x fsk_msg send fail, retry %d\n", cmd, i);
			continue;
		}

		break;
	}

	if (i >= retrys)
		return -EINVAL;

	return 0;
}

static int qi_protocol_auto_send_fsk_msg(unsigned int ic_type, u8 cmd, u8 *para, int para_len,
	int retrys)
{
	int i;
	int ret;
	struct qi_protocol_ops *l_ops = qi_protocol_get_ops(ic_type);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->auto_send_fsk_with_ack) {
		hwlog_err("auto_send_fsk_msg is null\n");
		return -EINVAL;
	}

	for (i = 0; i < retrys; i++) {
		ret = l_ops->auto_send_fsk_with_ack(cmd, para, para_len,
			l_ops->dev_data);
		if (ret) {
			hwlog_err("0x%x fsk_msg auto send fail, retry %d\n", cmd, i);
			continue;
		}

		break;
	}

	if (i >= retrys)
		return -EINVAL;

	return 0;
}

static int qi_protocol_send_fsk_ack_msg(unsigned int ic_type)
{
	return qi_protocol_send_fsk_msg(ic_type, QI_CMD_ACK, NULL, 0,
		WIRELESS_RETRY_ONE);
}

static int qi_protocol_get_ask_packet(unsigned int ic_type, u8 *data, int data_len, int retrys)
{
	int i;
	int ret;
	struct qi_protocol_ops *l_ops = qi_protocol_get_ops(ic_type);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->get_ask_packet) {
		hwlog_err("get_ask_packet is null\n");
		return -EINVAL;
	}

	for (i = 0; i < retrys; i++) {
		ret = l_ops->get_ask_packet(data, data_len, l_ops->dev_data);
		if (ret) {
			hwlog_err("ask_packet receive fail, retry %d\n", i);
			continue;
		}

		break;
	}

	if (i >= retrys)
		return -EINVAL;

	return 0;
}

static int qi_protocol_get_chip_fw_version(unsigned int ic_type, u8 *data, int data_len,
	int retrys)
{
	int i;
	int ret;
	struct qi_protocol_ops *l_ops = qi_protocol_get_ops(ic_type);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->get_chip_fw_version) {
		hwlog_err("get_chip_fw_version is null\n");
		return -EINVAL;
	}

	for (i = 0; i < retrys; i++) {
		ret = l_ops->get_chip_fw_version(data, data_len, l_ops->dev_data);
		if (ret) {
			hwlog_err("chip_fw_version get fail, retry %d\n", i);
			continue;
		}

		break;
	}

	if (i >= retrys)
		return -EINVAL;

	return 0;
}

static int qi_protocol_send_rx_event(unsigned int ic_type, u8 rx_evt)
{
	int retry = WIRELESS_RETRY_ONE;

	if (qi_protocol_send_msg_ack(ic_type, QI_CMD_SEND_RX_EVT,
		&rx_evt, QI_PARA_RX_EVT_LEN, retry))
		return -EINVAL;

	hwlog_info("send_rx_event succ\n");
	return 0;
}

static int qi_protocol_send_rx_vout(unsigned int ic_type, int rx_vout)
{
	int retry = WIRELESS_RETRY_ONE;

	if (qi_protocol_send_msg_ack(ic_type, QI_CMD_SEND_RX_VOUT,
		(u8 *)&rx_vout, QI_PARA_RX_VOUT_LEN, retry))
		return -EINVAL;

	hwlog_info("send_rx_vout succ\n");
	return 0;
}

static int qi_protocol_send_rx_iout(unsigned int ic_type, int rx_iout)
{
	int retry = WIRELESS_RETRY_ONE;

	if (qi_protocol_send_msg_ack(ic_type, QI_CMD_SEND_RX_IOUT,
		(u8 *)&rx_iout, QI_PARA_RX_IOUT_LEN, retry))
		return -EINVAL;

	hwlog_info("send_rx_iout succ\n");
	return 0;
}

static int qi_protocol_send_rx_boost_succ(unsigned int ic_type)
{
	int retry = WIRELESS_RETRY_ONE;

	if (qi_protocol_send_msg_ack(ic_type, QI_CMD_RX_BOOST_SUCC, NULL, 0, retry))
		return -EINVAL;

	hwlog_info("send_rx_boost_succ succ\n");
	return 0;
}

static int qi_protocol_send_rx_ready(unsigned int ic_type)
{
	int retry = WIRELESS_RETRY_ONE;

	if (qi_protocol_send_msg_ack(ic_type, QI_CMD_SEND_RX_READY, NULL, 0, retry))
		return -EINVAL;

	hwlog_info("send_rx_ready succ\n");
	return 0;
}

static int qi_protocol_send_tx_capability(unsigned int ic_type, u8 *cap, int len)
{
	int retry = WIRELESS_RETRY_ONE;

	if (!cap)
		return -EINVAL;

	if (len != (QI_PARA_TX_CAP_LEN - 1)) {
		hwlog_err("para error %d!=%d\n", len, QI_PARA_TX_CAP_LEN - 1);
		return -EINVAL;
	}

	if (qi_protocol_send_fsk_msg(ic_type, QI_CMD_GET_TX_CAP, cap, len, retry))
		return -EINVAL;

	hwlog_info("send_tx_capability succ\n");
	return 0;
}

static int qi_protocol_send_tx_alarm(unsigned int ic_type, u8 *alarm, int len)
{
	int retry = WIRELESS_RETRY_ONE;

	if (!alarm)
		return -EINVAL;

	if (len != QI_TX_ALARM_LEN) {
		hwlog_err("para error %d!=%d\n", len, QI_TX_ALARM_LEN);
		return -EINVAL;
	}

	if (qi_protocol_auto_send_fsk_msg(ic_type, QI_CMD_TX_ALARM, alarm, len, retry))
		return -EINVAL;

	hwlog_info("send_tx_alarm succ\n");
	return 0;
}

static int qi_protocol_send_tx_fw_version(unsigned int ic_type, u8 *fw, int len)
{
	int retry = WIRELESS_RETRY_ONE;

	if (!fw)
		return -EINVAL;

	if (len != (QI_ACK_TX_FWVER_LEN - 1)) {
		hwlog_err("para error %d!=%d\n", len, QI_ACK_TX_FWVER_LEN - 1);
		return -EINVAL;
	}

	if (qi_protocol_send_fsk_msg(ic_type, QI_CMD_GET_TX_VERSION, fw, len, retry))
		return -EINVAL;

	hwlog_info("send_tx_fw_version succ\n");
	return 0;
}

static int qi_protocol_send_tx_id(unsigned int ic_type, u8 *id, int len)
{
	int retry = WIRELESS_RETRY_ONE;

	if (!id)
		return -EINVAL;

	if (len != QI_PARA_TX_ID_LEN) {
		hwlog_err("para error %d!=%d\n", len, QI_PARA_TX_ID_LEN);
		return -EINVAL;
	}

	if (qi_protocol_send_fsk_msg(ic_type, QI_CMD_GET_TX_ID, id, len, retry))
		return -EINVAL;

	hwlog_info("send_tx_id succ\n");
	return 0;
}

static int qi_protocol_send_cert_confirm(unsigned int ic_type, bool succ_flag)
{
	int retry = WIRELESS_RETRY_ONE;

	if (succ_flag)
		return qi_protocol_send_msg_ack(ic_type, QI_CMD_CERT_SUCC,
			NULL, 0, retry);
	else
		return qi_protocol_send_msg_ack(ic_type, QI_CMD_CERT_FAIL,
			NULL, 0, retry);
}

static int qi_protocol_send_charge_state(unsigned int ic_type, u8 state)
{
	int retry = WIRELESS_RETRY_ONE;

	if (qi_protocol_send_msg_ack(ic_type, QI_CMD_SEND_CHRG_STATE,
		&state, QI_PARA_CHARGE_STATE_LEN, retry))
		return -EINVAL;

	hwlog_info("send_charge_state succ\n");
	return 0;
}

static int qi_protocol_send_fod_status(unsigned int ic_type, int status)
{
	int retry = WIRELESS_RETRY_ONE;

	if (qi_protocol_send_msg_ack(ic_type, QI_CMD_SEND_FOD_STATUS,
		(u8 *)&status, QI_PARA_FOD_STATUS_LEN, retry))
		return -EINVAL;

	hwlog_info("send_fod_status succ\n");
	return 0;
}

static int qi_protocol_send_charge_mode(unsigned int ic_type, u8 mode)
{
	int retry = WIRELESS_RETRY_ONE;
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev)
		return -EINVAL;

	if (l_dev->dev_id == WIRELESS_DEVICE_ID_IDTP9221) {
		hwlog_info("send_charge_mode: not support\n");
		return 0;
	}

	if (qi_protocol_send_msg_ack(ic_type, QI_CMD_SEND_CHRG_MODE,
		&mode, QI_PARA_CHARGE_MODE_LEN, retry))
		return -EINVAL;

	hwlog_info("send_charge_mode succ\n");
	return 0;
}

static int qi_protocol_set_fan_speed_limit(unsigned int ic_type, u8 limit)
{
	int retry = WIRELESS_RETRY_ONE;
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev)
		return -EINVAL;

	if (l_dev->dev_id == WIRELESS_DEVICE_ID_IDTP9221) {
		hwlog_info("set_fan_speed_limit: not support\n");
		return 0;
	}

	if (qi_protocol_send_msg_ack(ic_type, QI_CMD_SET_FAN_SPEED_LIMIT,
		&limit, QI_PARA_FAN_SPEED_LIMIT_LEN, retry))
		return -EINVAL;

	hwlog_info("set_fan_speed_limit succ\n");
	return 0;
}

static int qi_protocol_set_rpp_format_post(unsigned int ic_type, u8 pmax, int mode)
{
	struct qi_protocol_ops *l_ops = qi_protocol_get_ops(ic_type);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->set_rpp_format_post)
		return 0;

	return l_ops->set_rpp_format_post(pmax, mode, l_ops->dev_data);
}

static int qi_protocol_set_rpp_format(unsigned int ic_type, u8 pmax)
{
	u8 data[QI_PARA_RX_PMAX_LEN] = { 0 };
	int retry = WIRELESS_RETRY_ONE;
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev)
		return -EINVAL;

	if (l_dev->dev_id == WIRELESS_DEVICE_ID_IDTP9221) {
		hwlog_info("set_rpp_format: not support\n");
		return 0;
	}

	data[QI_PARA_RX_PMAX_OFFSET] = pmax / QI_PARA_RX_PMAX_UNIT;
	data[QI_PARA_RX_PMAX_OFFSET] <<= QI_PARA_RX_PMAX_SHIFT;

	if (qi_protocol_send_msg_ack(ic_type, QI_CMD_SET_RX_MAX_POWER,
		data, QI_PARA_RX_PMAX_LEN, retry))
		return -EINVAL;

	hwlog_info("send_rpp_format: pmax=%d, format=0x%x\n",
		pmax, data[QI_PARA_RX_PMAX_OFFSET]);

	if (qi_protocol_set_rpp_format_post(ic_type, pmax, WIRELESS_RX))
		return -EINVAL;

	return 0;
}

static int qi_protocol_get_ept_type(unsigned int ic_type, u16 *type)
{
	u8 data[QI_ACK_EPT_TYPE_LEN] = { 0 };
	int retry = WIRELESS_RETRY_TWO;
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev || !type)
		return -EINVAL;

	if (l_dev->dev_id == WIRELESS_DEVICE_ID_IDTP9221) {
		*type = 0;
		hwlog_info("get_ept_type: not support\n");
		return 0;
	}

	if (qi_protocol_send_msg(ic_type, QI_CMD_GET_EPT_TYPE, NULL, 0, data,
		QI_ACK_EPT_TYPE_LEN, retry))
		return -EINVAL;

	*type = (data[QI_ACK_EPT_TYPE_E_OFFSET] << QI_PROTOCOL_BYTE_BITS) |
		data[QI_ACK_EPT_TYPE_S_OFFSET];

	return 0;
}

static int qi_protocol_get_rpp_format(unsigned int ic_type, u8 *format)
{
	u8 data[QI_ACK_RPP_FORMAT_LEN] = { 0 };
	int retry = WIRELESS_RETRY_TWO;
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev || !format)
		return -EINVAL;

	if (l_dev->dev_id == WIRELESS_DEVICE_ID_IDTP9221) {
		*format = QI_ACK_RPP_FORMAT_8BIT;
		hwlog_info("get_rpp_format: not support\n");
		return 0;
	}

	if (qi_protocol_send_msg(ic_type, QI_CMD_GET_RPP_FORMAT, NULL, 0, data,
		QI_ACK_RPP_FORMAT_LEN, retry))
		return -EINVAL;

	*format = data[QI_ACK_RPP_FORMAT_OFFSET];

	return 0;
}

static int qi_protocol_get_tx_fw_version(unsigned int ic_type, char *ver, int size)
{
	int i;
	int retry = WIRELESS_RETRY_TWO;
	u8 tx_fwver[QI_ACK_TX_FWVER_LEN] = { 0 };
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev || !ver)
		return -EINVAL;

	memset(l_dev->info.tx_fwver, 0, sizeof(l_dev->info.tx_fwver));
	if (qi_protocol_send_msg(ic_type, QI_CMD_GET_TX_VERSION, NULL, 0, tx_fwver,
		QI_ACK_TX_FWVER_LEN, retry))
		return -EINVAL;

	for (i = QI_ACK_TX_FWVER_E; i >= QI_ACK_TX_FWVER_S; i--) {
		l_dev->info.tx_fwver[QI_ACK_TX_FWVER_E - i] = tx_fwver[i];
		if (i != QI_ACK_TX_FWVER_S)
			snprintf(ver + strlen(ver), size,
				"0x%02x ", tx_fwver[i]);
		else
			snprintf(ver + strlen(ver), size,
				"0x%02x", tx_fwver[i]);
	}

	return 0;
}

static int qi_protocol_get_tx_bigdata_info(unsigned int ic_type, char *info, int size)
{
	unsigned int i;
	int str_len;
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev || !info || (size <= (int)sizeof(l_dev->info)))
		return -EINVAL;

	memset(info, 0, size);
	str_len = 0;
	for (i = 0; i < sizeof(l_dev->info.tx_fwver); i++) {
		snprintf(info + str_len, size - str_len,
			"%02x", l_dev->info.tx_fwver[i]);
		str_len += 2; /* 2: hex -> 2 bytes str */
	}
	str_len = strlen(info);
	for (i = 0; i < sizeof(l_dev->info.tx_main_cap); i++) {
		snprintf(info + str_len, size - str_len,
			"%02x", l_dev->info.tx_main_cap[i]);
		str_len += 2; /* 2: hex -> 2 bytes str */
	}
	str_len = strlen(info);
	for (i = 0; i < sizeof(l_dev->info.tx_ext_cap); i++) {
		snprintf(info + str_len, size - str_len,
			"%02x", l_dev->info.tx_ext_cap[i]);
		str_len += 2; /* 2: hex -> 2 bytes str */
	}

	return 0;
}

static int qi_protocol_get_tx_id_pre(unsigned int ic_type)
{
	struct qi_protocol_ops *l_ops = qi_protocol_get_ops(ic_type);

	if (!l_ops)
		return -EINVAL;

	if (!l_ops->get_tx_id_pre)
		return 0;

	return l_ops->get_tx_id_pre(l_ops->dev_data);
}

static int qi_protocol_get_tx_id(unsigned int ic_type, int *id)
{
	u8 para[QI_PARA_TX_ID_LEN] = { QI_HANDSHAKE_ID_HIGH, QI_HANDSHAKE_ID_LOW };
	u8 data[QI_ACK_TX_ID_LEN] = { 0 };
	int retry = WIRELESS_RETRY_TWO;

	if (!id)
		return -EINVAL;

	if (qi_protocol_get_tx_id_pre(ic_type))
		return -EINVAL;

	if (qi_protocol_send_msg(ic_type, QI_CMD_GET_TX_ID, para, QI_PARA_TX_ID_LEN, data,
		QI_ACK_TX_ID_LEN, retry))
		return -EINVAL;

	if (power_cmdline_is_factory_mode())
		qi_protocol_send_rx_ready(ic_type);

	*id = (data[QI_ACK_TX_ID_S_OFFSET] << QI_PROTOCOL_BYTE_BITS) |
		data[QI_ACK_TX_ID_E_OFFSET];

	return 0;
}

static int qi_protocol_get_tx_type(unsigned int ic_type, int *type)
{
	u8 tx_fwver[QI_ACK_TX_FWVER_LEN] = { 0 };
	int retry = WIRELESS_RETRY_TWO;
	u16 data;
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev || !type)
		return -EINVAL;

	if (l_dev->info.tx_type != WIRELESS_TX_TYPE_UNKNOWN) {
		*type = l_dev->info.tx_type;
		return 0;
	}

	if (qi_protocol_send_msg(ic_type, QI_CMD_GET_TX_VERSION,
		NULL, 0, tx_fwver, QI_ACK_TX_FWVER_LEN, retry))
		return -ENOTSUPP;

	data = (tx_fwver[QI_ACK_TX_FWVER_E] << QI_PROTOCOL_BYTE_BITS) |
		tx_fwver[QI_ACK_TX_FWVER_E - 1];
	switch (data) {
	case QI_TX_TYPE_CP85:
		*type = WIRELESS_TX_TYPE_CP85;
		break;
	case QI_TX_TYPE_CP60:
		*type = WIRELESS_TX_TYPE_CP60;
		break;
	case QI_TX_TYPE_CP61:
		*type = WIRELESS_TX_TYPE_CP61;
		break;
	case QI_TX_TYPE_CP62_LX:
		*type = WIRELESS_TX_TYPE_CP62_LX;
		break;
	case QI_TX_TYPE_CP62_XW:
		*type = WIRELESS_TX_TYPE_CP62_XW;
		break;
	case QI_TX_TYPE_CP62R:
		*type = WIRELESS_TX_TYPE_CP62R;
		break;
	case QI_TX_TYPE_CP39S:
		*type = WIRELESS_TX_TYPE_CP39S;
		break;
	case QI_TX_TYPE_CP39S_HK:
		*type = WIRELESS_TX_TYPE_CP39S_HK;
		break;
	case QI_TX_TYPE_CK30_LX:
		*type = WIRELESS_TX_TYPE_CK30_LX;
		break;
	case QI_TX_TYPE_CK30_LP:
		*type = WIRELESS_TX_TYPE_CK30_LP;
		break;
	default:
		*type = WIRELESS_TX_TYPE_ERROR;
		break;
	}

	if (*(u32 *)&tx_fwver[QI_ACK_TX_FWVER_S] == QI_TX_FW_CP39S_HK)
		*type = WIRELESS_TX_TYPE_CP39S_HK;
	l_dev->info.tx_type = *type;
	hwlog_info("get_tx_type: %d\n", *type);
	return 0;
}

static int qi_protocol_get_tx_adapter_type(unsigned int ic_type, int *type)
{
	u8 data[QI_ACK_TX_ADP_TYPE_LEN] = { 0 };
	int retry = WIRELESS_RETRY_TWO;

	if (!type)
		return -EINVAL;

	if (qi_protocol_send_msg(ic_type, QI_CMD_GET_TX_ADAPTER_TYPE,
		NULL, 0, data, QI_ACK_TX_ADP_TYPE_LEN, retry))
		return -EINVAL;

	*type = data[QI_ACK_TX_ADP_TYPE_OFFSET];

	return 0;
}

static int qi_protocol_get_tx_main_capability(unsigned int ic_type, u8 *cap, int len)
{
	int i;
	int retry = WIRELESS_RETRY_TWO;
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev || !cap)
		return -EINVAL;

	if (len != QI_PARA_TX_CAP_LEN) {
		hwlog_err("para error %d!=%d\n", len, QI_PARA_TX_CAP_LEN);
		return -EINVAL;
	}

	if (qi_protocol_send_msg(ic_type, QI_CMD_GET_TX_CAP, NULL, 0, cap, len, retry))
		return -EINVAL;

	for (i = QI_PARA_TX_MAIN_CAP_S; i <= QI_PARA_TX_MAIN_CAP_E; i++)
		l_dev->info.tx_main_cap[i - QI_PARA_TX_MAIN_CAP_S] = cap[i];

	return 0;
}

static int qi_protocol_get_tx_extra_capability(unsigned int ic_type, u8 *cap, int len)
{
	int i;
	int retry = WIRELESS_RETRY_TWO;
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev || !cap)
		return -EINVAL;

	if (len != QI_PARA_TX_EXT_CAP_LEN) {
		hwlog_err("para error %d!=%d\n", len, QI_PARA_TX_EXT_CAP_LEN);
		return -EINVAL;
	}

	if (qi_protocol_send_msg(ic_type, QI_CMD_GET_GET_TX_EXT_CAP, NULL, 0,
		cap, len, retry))
		return -EINVAL;

	for (i = QI_PARA_TX_EXT_CAP_S; i <= QI_PARA_TX_EXT_CAP_E; i++)
		l_dev->info.tx_ext_cap[i - QI_PARA_TX_EXT_CAP_S] = cap[i];

	return 0;
}

static int qi_protocol_get_tx_capability(unsigned int ic_type,
	struct wireless_protocol_tx_cap *cap)
{
	u8 data1[QI_PARA_TX_CAP_LEN] = { 0 };
	u8 data2[QI_PARA_TX_EXT_CAP_LEN] = { 0 };

	if (!cap)
		return -EINVAL;

	if (qi_protocol_get_tx_main_capability(ic_type, data1, QI_PARA_TX_CAP_LEN))
		return -EINVAL;

	cap->type = data1[QI_TX_CAP_TYPE];
	cap->vout_max = data1[QI_TX_CAP_VOUT_MAX] * QI_PARA_TX_CAP_VOUT_STEP;
	cap->iout_max = data1[QI_TX_CAP_IOUT_MAX] * QI_PARA_TX_CAP_IOUT_STEP;
	cap->cable_ok = data1[QI_TX_CAP_ATTR] & QI_PARA_TX_CAP_CABLE_OK_MASK;
	cap->can_boost = data1[QI_TX_CAP_ATTR] & QI_PARA_TX_CAP_CAN_BOOST_MASK;
	cap->ext_type = data1[QI_TX_CAP_ATTR] & QI_PARA_TX_CAP_EXT_TYPE_MASK;
	cap->no_need_cert = data1[QI_TX_CAP_ATTR] & QI_PARA_TX_CAP_CERT_MASK;
	cap->support_scp = data1[QI_TX_CAP_ATTR] & QI_PARA_TX_CAP_SUPPORT_SCP_MASK;
	cap->support_12v = data1[QI_TX_CAP_ATTR] & QI_PARA_TX_CAP_SUPPORT_12V_MASK;
	cap->support_extra_cap = data1[QI_TX_CAP_ATTR] & QI_PARA_TX_CAP_SUPPORT_EXTRA_MASK;

	if ((cap->vout_max > WIRELESS_ADAPTER_9V) &&
		(cap->vout_max < WIRELESS_ADAPTER_12V) && !cap->support_12v) {
		cap->vout_max = WIRELESS_ADAPTER_9V;
		hwlog_info("tx not support 12v, set to %dmv\n", cap->vout_max);
	}
	if (cap->ext_type == QI_PARA_TX_EXT_TYPE_CAR)
		cap->type += TX_TYPE_CAR_BASE;
	if (cap->ext_type == QI_PARA_TX_EXT_TYPE_PWR_BANK)
		cap->type += TX_TYPE_PWR_BANK_BASE;

	hwlog_info("tx_main_cap: type=0x%x vmax=%d imax=%d\t"
		"can_boost=%d cable_ok=%d no_need_cert=%d\t"
		"[support] scp=%d 12V=%d extra_cap=%d\n",
		cap->type, cap->vout_max, cap->iout_max, cap->can_boost,
		cap->cable_ok, cap->no_need_cert, cap->support_scp,
		cap->support_12v, cap->support_extra_cap);

	if (!cap->support_extra_cap) {
		hwlog_info("tx not support extra capability\n");
		return 0;
	}

	if (qi_protocol_get_tx_extra_capability(ic_type, data2, QI_PARA_TX_EXT_CAP_LEN))
		return -EINVAL;

	cap->support_fan = data2[QI_TX_EXTRA_CAP_ATTR1] &
		QI_PARA_TX_EXT_CAP_SUPPORT_FAN_MASK;
	cap->support_tec = data2[QI_TX_EXTRA_CAP_ATTR1] &
		QI_PARA_TX_EXT_CAP_SUPPORT_TEC_MASK;
	cap->support_fod_status = data2[QI_TX_EXTRA_CAP_ATTR1] &
		QI_PARA_TX_EXT_CAP_SUPPORT_QVAL_MASK;
	cap->support_get_ept = data2[QI_TX_EXTRA_CAP_ATTR1] &
		QI_PARA_TX_EXT_CAP_SUPPORT_GET_EPT_MASK;
	cap->support_fop_range = data2[QI_TX_EXTRA_CAP_ATTR1] &
		QI_PARA_TX_EXT_CAP_SUPPORT_FOP_RANGE_MASK;

	hwlog_info("tx_extra_cap: [support] fan=%d tec=%d fod_status=%d fop_range=%d type=%u\n",
		cap->support_fan, cap->support_tec, cap->support_fod_status,
		cap->support_fop_range, ic_type);

	return 0;
}

static int qi_protocol_get_tx_max_power_preprocess(int tx_type)
{
	switch (tx_type) {
	case WIRELESS_TX_TYPE_CP85:
		return QI_TX_MAX_POWER_CP85;
	case WIRELESS_TX_TYPE_CP60:
		return QI_TX_MAX_POWER_CP60;
	case WIRELESS_TX_TYPE_CP61:
	case WIRELESS_TX_TYPE_CP39S:
	case WIRELESS_TX_TYPE_CP39S_HK:
		return QI_TX_MAX_POWER_CP61;
	case WIRELESS_TX_TYPE_CP62_LX:
	case WIRELESS_TX_TYPE_CP62_XW:
		return QI_TX_MAX_POWER_CP62;
	default:
		return 0;
	}
}

static bool qi_protocol_adapter_cable_cmd_support(int tx_type)
{
	switch (tx_type) {
	case WIRELESS_TX_TYPE_CP85:
	case WIRELESS_TX_TYPE_CP60:
	case WIRELESS_TX_TYPE_CP61:
	case WIRELESS_TX_TYPE_CP39S:
	case WIRELESS_TX_TYPE_CP39S_HK:
	case WIRELESS_TX_TYPE_CP62_LX:
	case WIRELESS_TX_TYPE_CP62_XW:
		return false;
	default:
		return true;
	}
}

static int qi_protocol_get_tx_max_power(unsigned int ic_type, int *power)
{
	u8 data[QI_PARA_TX_MAX_PWR_LEN] = { 0 };
	int tx_type = WIRELESS_TX_TYPE_UNKNOWN;

	if (!power)
		return -EINVAL;

	if (!qi_protocol_get_tx_type(ic_type, &tx_type) &&
		!qi_protocol_adapter_cable_cmd_support(tx_type)) {
		*power = qi_protocol_get_tx_max_power_preprocess(tx_type);
		return 0;
	}

	if (qi_protocol_send_msg(ic_type, QI_CMD_GET_TX_MAX_PWR, NULL, 0, data,
		QI_PARA_TX_MAX_PWR_LEN, WIRELESS_RETRY_TWO))
		return -ENOTSUPP;
	*power = ((data[QI_PARA_TX_MAX_PWR_E] << QI_PROTOCOL_BYTE_BITS) |
		data[QI_PARA_TX_MAX_PWR_S]) * QI_PARA_TX_MAX_PWR_BASE;
	*power = *power * WLRX_ACC_TX_PWR_RATIO / POWER_PERCENT;

	hwlog_info("get_tx_max_power: %d\n", *power);

	return 0;
}

static int qi_protocol_get_tx_adapter_capability(unsigned int ic_type, int *vout, int *iout)
{
	u8 data[QI_PARA_TX_MAX_ADAPTER_LEN] = { 0 };
	int tx_type = WIRELESS_TX_TYPE_UNKNOWN;

	if (!vout || !iout)
		return -EINVAL;

	if (!qi_protocol_get_tx_type(ic_type, &tx_type) &&
		!qi_protocol_adapter_cable_cmd_support(tx_type))
		return -ENOTSUPP;

	if (qi_protocol_send_msg(ic_type, QI_CMD_GET_TX_MAX_VIN_IIN, NULL, 0, data,
		QI_PARA_TX_MAX_ADAPTER_LEN, WIRELESS_RETRY_TWO))
		return -ENOTSUPP;
	*vout = (data[QI_PARA_TX_MAX_ADAPTER_VOUT_E] << QI_PROTOCOL_BYTE_BITS) |
		data[QI_PARA_TX_MAX_ADAPTER_VOUT_S];
	*iout = (data[QI_PARA_TX_MAX_ADAPTER_IOUT_E] << QI_PROTOCOL_BYTE_BITS) |
		data[QI_PARA_TX_MAX_ADAPTER_IOUT_S];

	hwlog_info("get_tx_adapter_capability: vout=%d, iout=%d\n", *vout, *iout);

	return 0;
}

static int qi_protocol_get_tx_cable_type(unsigned int ic_type, int *type, int *iout)
{
	u8 data[QI_PARA_TX_CABLE_TYPE_LEN] = { 0 };
	int tx_type = WIRELESS_TX_TYPE_UNKNOWN;

	if (!type || !iout)
		return -EINVAL;

	if (!qi_protocol_get_tx_type(ic_type, &tx_type) &&
		!qi_protocol_adapter_cable_cmd_support(tx_type))
		return -ENOTSUPP;

	if (qi_protocol_send_msg(ic_type, QI_CMD_GET_CABLE_TYPE, NULL, 0, data,
		QI_PARA_TX_CABLE_TYPE_LEN, WIRELESS_RETRY_TWO))
		return -ENOTSUPP;
	*type = data[QI_PARA_TX_CABLE_TYPE_OFFSET];
	*iout = data[QI_PARA_TX_CABLE_IOUT_OFFSET] * QI_PARA_TX_CABLE_TYPE_BASE;

	hwlog_info("get_tx_cable_type: type=%d, iout=%d\n", *type, *iout);

	return 0;
}

static int qi_protocol_get_tx_fop_range(unsigned int ic_type,
	struct wireless_protocol_tx_fop_range *fop)
{
	u8 data[QI_PARA_TX_FOP_RANGE_LEN] = { 0 };

	if (!fop)
		return -EINVAL;

	if (qi_protocol_send_msg(ic_type, QI_CMD_GET_TX_FOP_RANGE, NULL, 0, data,
		QI_PARA_TX_FOP_RANGE_LEN, WIRELESS_RETRY_TWO))
		return -EINVAL;

	fop->base_min = data[QI_TX_FOP_RANGE_BASE_MIN] + QI_PARA_TX_FOP_RANGE_BASE;
	fop->base_max = data[QI_TX_FOP_RANGE_BASE_MAX] + QI_PARA_TX_FOP_RANGE_BASE;
	if (data[QI_TX_FOP_RANGE_EXT1] != 0) {
		fop->ext1_min = data[QI_TX_FOP_RANGE_EXT1] +
			QI_PARA_TX_FOP_RANGE_BASE - QI_PARA_TX_FOP_RANGE_EXT_LIMIT;
		fop->ext1_max = data[QI_TX_FOP_RANGE_EXT1] +
			QI_PARA_TX_FOP_RANGE_BASE + QI_PARA_TX_FOP_RANGE_EXT_LIMIT;
	}
	if (data[QI_TX_FOP_RANGE_EXT2] != 0) {
		fop->ext2_min = data[QI_TX_FOP_RANGE_EXT2] +
			QI_PARA_TX_FOP_RANGE_BASE - QI_PARA_TX_FOP_RANGE_EXT_LIMIT;
		fop->ext2_max = data[QI_TX_FOP_RANGE_EXT2] +
			QI_PARA_TX_FOP_RANGE_BASE + QI_PARA_TX_FOP_RANGE_EXT_LIMIT;
	}

	hwlog_info("tx_fop_range: base_min=%d base_max=%d ext1_min=%d ext1_max=%d ext2_min=%d ext2_max=%d\n",
		fop->base_min, fop->base_max, fop->ext1_min, fop->ext1_max,
		fop->ext2_min, fop->ext2_max);

	return 0;
}

static int qi_protocol_set_encrypt_index(u8 *data, int index)
{
	if (!data)
		return -EINVAL;

	data[QI_PARA_KEY_INDEX_OFFSET] = index;

	hwlog_info("set_encrypt_index: %d\n", index);
	return 0;
}

static int qi_protocol_set_random_num(u8 *data, int start, int end)
{
	int i;

	if (!data)
		return -EINVAL;

	for (i = start; i <= end; i++)
		get_random_bytes(&data[i], sizeof(u8));

	return 0;
}

static int qi_protocol_send_random_num(unsigned int ic_type, u8 *random, int len)
{
	int i;
	int retry = WIRELESS_RETRY_ONE;

	if (!random)
		return -EINVAL;

	if (len != QI_PARA_RANDOM_LEN) {
		hwlog_err("para error %d!=%d\n", len, QI_PARA_RANDOM_LEN);
		return -EINVAL;
	}

	for (i = 0; i < len / QI_PARA_RANDOM_GROUP_LEN; i++) {
		if (qi_protocol_send_msg_ack(ic_type, QI_CMD_START_CERT + i,
			random + i * QI_PARA_RANDOM_GROUP_LEN,
			QI_PARA_RANDOM_GROUP_LEN, retry))
			return -EINVAL;
	}

	return 0;
}

static int qi_protocol_get_encrypted_value(unsigned int ic_type, u8 *hash, int len)
{
	int i;
	int retry = WIRELESS_RETRY_TWO;

	if (!hash)
		return -EINVAL;

	if (len != QI_ACK_HASH_LEN) {
		hwlog_err("para error %d!=%d\n", len, QI_ACK_HASH_LEN);
		return -EINVAL;
	}

	for (i = 0; i < len / QI_ACK_HASH_GROUP_LEN; i++) {
		/* cmd 0x38 & 0x39 */
		if (qi_protocol_send_msg(ic_type, QI_CMD_GET_HASH + i, NULL, 0,
			hash + i * QI_ACK_HASH_GROUP_LEN, QI_ACK_HASH_GROUP_LEN, retry))
			return -EINVAL;
	}

	return 0;
}

static int qi_protocol_copy_hash_value(u8 *data, int len, u8 *hash, int size)
{
	int i, j;

	for (i = 0; i < size; i++) {
		j = i + QI_ACK_HASH_S_OFFSET * (i / QI_ACK_HASH_E_OFFSET + 1);
		hash[i] = data[j];
	}

	return 0;
}

static int qi_protocol_auth_encrypt_start(unsigned int ic_type, int key, u8 *random, int r_size,
	u8 *hash, int h_size)
{
	u8 data[QI_ACK_HASH_LEN] = { 0 };
	int size;

	if (!random || !hash)
		return -EINVAL;

	memset(random, 0, r_size);
	memset(hash, 0, h_size);

	size = QI_PARA_RANDOM_LEN;
	if (r_size != size) {
		hwlog_err("invalid r_size=%d\n", r_size);
		return -EINVAL;
	}

	size = QI_ACK_HASH_LEN - QI_ACK_HASH_LEN / QI_ACK_HASH_GROUP_LEN;
	if (h_size != size) {
		hwlog_err("invalid h_size=%d\n", h_size);
		return -EINVAL;
	}

	/* first: set key index */
	if (qi_protocol_set_encrypt_index(random, key))
		return -EINVAL;

	/* second: host create random num */
	if (qi_protocol_set_random_num(random,
		QI_PARA_RANDOM_S_OFFSET, QI_PARA_RANDOM_E_OFFSET))
		return -EINVAL;

	/* third: host set random num to slave */
	if (qi_protocol_send_random_num(ic_type, random, QI_PARA_RANDOM_LEN))
		return -EINVAL;

	/* fouth: host get hash num from slave */
	if (qi_protocol_get_encrypted_value(ic_type, data, QI_ACK_HASH_LEN))
		return -EINVAL;

	/* fifth: copy hash value */
	if (qi_protocol_copy_hash_value(data, QI_ACK_HASH_LEN, hash, h_size))
		return -EINVAL;

	return 0;
}

static int qi_protocol_fix_tx_fop(unsigned int ic_type, int fop)
{
	int retry = WIRELESS_RETRY_ONE;

	if (!(((fop >= QI_FIXED_FOP_MIN) && (fop <= QI_FIXED_FOP_MAX)) ||
		((fop >= QI_FIXED_HIGH_FOP_MIN) && (fop <= QI_FIXED_HIGH_FOP_MAX)))) {
		hwlog_err("fixed fop %d exceeds range[%d, %d] or [%d, %d]\n",
			fop, QI_FIXED_FOP_MIN, QI_FIXED_FOP_MAX,
			QI_FIXED_HIGH_FOP_MIN, QI_FIXED_HIGH_FOP_MAX);
		return -EINVAL;
	}

	/* cmd 0x44 */
	if (qi_protocol_send_msg_ack(ic_type, QI_CMD_FIX_TX_FOP,
		(u8 *)&fop, QI_PARA_TX_FOP_LEN, retry))
		return -EINVAL;

	hwlog_info("fix_tx_fop: %d\n", fop);
	return 0;
}

static int qi_protocol_unfix_tx_fop(unsigned int ic_type)
{
	int retry = WIRELESS_RETRY_ONE;

	/* cmd 0x45 */
	if (qi_protocol_send_msg_ack(ic_type, QI_CMD_UNFIX_TX_FOP, NULL, 0, retry))
		return -EINVAL;

	hwlog_info("unfix_tx_fop success\n");
	return 0;
}

static int qi_protocol_reset_dev_info(unsigned int ic_type)
{
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev)
		return -EINVAL;

	memset(&l_dev->info, 0, sizeof(l_dev->info));
	return 0;
}

static int qi_protocol_acc_set_tx_dev_state(unsigned int ic_type, u8 state)
{
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev)
		return -EINVAL;

	l_dev->acc_info.dev_state = state;
	hwlog_info("acc_set_tx_dev_state: 0x%x\n", state);
	return 0;
}

static int qi_protocol_acc_set_tx_dev_info_cnt(unsigned int ic_type, u8 cnt)
{
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev)
		return -EINVAL;

	l_dev->acc_info.dev_info_cnt = cnt;
	hwlog_info("acc_set_tx_dev_info_cnt: 0x%x\n", cnt);
	return 0;
}

static int qi_protocol_acc_get_tx_dev_state(unsigned int ic_type, u8 *state)
{
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev || !state)
		return -EINVAL;

	*state = l_dev->acc_info.dev_state;
	hwlog_info("acc_get_tx_dev_state: 0x%x\n", *state);
	return 0;
}

static int qi_protocol_acc_get_tx_dev_no(unsigned int ic_type, u8 *no)
{
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev || !no)
		return -EINVAL;

	*no = l_dev->acc_info.dev_no;
	hwlog_info("acc_get_tx_dev_no: 0x%x\n", *no);
	return 0;
}

static int qi_protocol_acc_get_tx_dev_mac(unsigned int ic_type, u8 *mac, int len)
{
	int i;
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev || !mac)
		return -EINVAL;

	if (len != QI_ACC_TX_DEV_MAC_LEN) {
		hwlog_err("para error %d!=%d\n", len, QI_ACC_TX_DEV_MAC_LEN);
		return -EINVAL;
	}

	for (i = 0; i < len; i++) {
		mac[i] = l_dev->acc_info.dev_mac[i];
		hwlog_info("acc_get_tx_dev_mac: mac[%d]=0x%x\n", i, mac[i]);
	}
	return 0;
}

static int qi_protocol_acc_get_tx_dev_model_id(unsigned int ic_type, u8 *id, int len)
{
	int i;
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev || !id)
		return -EINVAL;

	if (len != QI_ACC_TX_DEV_MODELID_LEN) {
		hwlog_err("para error %d!=%d\n", len, QI_ACC_TX_DEV_MODELID_LEN);
		return -EINVAL;
	}

	for (i = 0; i < len; i++) {
		id[i] = l_dev->acc_info.dev_model_id[i];
		hwlog_info("acc_get_tx_dev_model_id: id[%d]=0x%x\n", i, id[i]);
	}
	return 0;
}

static int qi_protocol_acc_get_tx_dev_submodel_id(unsigned int ic_type, u8 *id)
{
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev || !id)
		return -EINVAL;

	*id = l_dev->acc_info.dev_submodel_id;
	hwlog_info("acc_get_tx_dev_submodel_id: 0x%x\n", *id);
	return 0;
}

static int qi_protocol_acc_get_tx_dev_version(unsigned int ic_type, u8 *ver)
{
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev || !ver)
		return -EINVAL;

	*ver = l_dev->acc_info.dev_version;
	hwlog_info("acc_get_tx_dev_version: 0x%x\n", *ver);
	return 0;
}

static int qi_protocol_acc_get_tx_dev_business(unsigned int ic_type, u8 *bus)
{
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev || !bus)
		return -EINVAL;

	*bus = l_dev->acc_info.dev_business;
	hwlog_info("acc_get_tx_dev_business: 0x%x\n", *bus);
	return 0;
}

static int qi_protocol_acc_get_tx_dev_info_cnt(unsigned int ic_type, u8 *cnt)
{
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev || !cnt)
		return -EINVAL;

	*cnt = l_dev->acc_info.dev_info_cnt;
	hwlog_info("acc_get_tx_dev_info_cnt: %d\n", *cnt);
	return 0;
}

static int qi_protocol_set_tx_cipherkey(unsigned int ic_type, u8 *cipherkey, int len)
{
	int i;
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev)
		return -EINVAL;

	if (!cipherkey || (len != QI_TX_CIPHERKEY_LEN)) {
		hwlog_err("para error\n");
		return -EINVAL;
	}

	memset(l_dev->cipherkey_info.tx_cipherkey, 0,
		sizeof(l_dev->cipherkey_info.tx_cipherkey));
	for (i = 0; i < QI_TX_CIPHERKEY_LEN; i++)
		l_dev->cipherkey_info.tx_cipherkey[i] = cipherkey[i];

	hwlog_info("set_tx_cipherkey succ\n");
	return 0;
}

static int qi_protocol_get_rx_random(unsigned int ic_type, u8 *random, int len)
{
	int i;
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev)
		return -EINVAL;

	if (!random || (len != QI_RX_RANDOM_LEN)) {
		hwlog_err("para error\n");
		return -EINVAL;
	}

	for (i = 0; i < len; i++) {
		random[i] = l_dev->cipherkey_info.rx_random[i];
		l_dev->cipherkey_info.rx_random[i] = 0;
	}

	return 0;
}

static struct wireless_protocol_ops wireless_protocol_qi_ops = {
	.type_name = "hw_qi",
	.send_rx_evt = qi_protocol_send_rx_event,
	.send_rx_vout = qi_protocol_send_rx_vout,
	.send_rx_iout = qi_protocol_send_rx_iout,
	.send_rx_boost_succ = qi_protocol_send_rx_boost_succ,
	.send_rx_ready = qi_protocol_send_rx_ready,
	.send_tx_capability = qi_protocol_send_tx_capability,
	.send_tx_alarm = qi_protocol_send_tx_alarm,
	.send_tx_fw_version = qi_protocol_send_tx_fw_version,
	.send_cert_confirm = qi_protocol_send_cert_confirm,
	.send_charge_state = qi_protocol_send_charge_state,
	.send_fod_status = qi_protocol_send_fod_status,
	.send_charge_mode = qi_protocol_send_charge_mode,
	.set_fan_speed_limit = qi_protocol_set_fan_speed_limit,
	.set_rpp_format = qi_protocol_set_rpp_format,
	.get_ept_type = qi_protocol_get_ept_type,
	.get_rpp_format = qi_protocol_get_rpp_format,
	.get_tx_fw_version = qi_protocol_get_tx_fw_version,
	.get_tx_bigdata_info = qi_protocol_get_tx_bigdata_info,
	.get_tx_id = qi_protocol_get_tx_id,
	.get_tx_type = qi_protocol_get_tx_type,
	.get_tx_adapter_type = qi_protocol_get_tx_adapter_type,
	.get_tx_capability = qi_protocol_get_tx_capability,
	.get_tx_max_power = qi_protocol_get_tx_max_power,
	.get_tx_adapter_capability = qi_protocol_get_tx_adapter_capability,
	.get_tx_cable_type = qi_protocol_get_tx_cable_type,
	.get_tx_fop_range = qi_protocol_get_tx_fop_range,
	.auth_encrypt_start = qi_protocol_auth_encrypt_start,
	.fix_tx_fop = qi_protocol_fix_tx_fop,
	.unfix_tx_fop = qi_protocol_unfix_tx_fop,
	.reset_dev_info = qi_protocol_reset_dev_info,
	.acc_set_tx_dev_state = qi_protocol_acc_set_tx_dev_state,
	.acc_set_tx_dev_info_cnt = qi_protocol_acc_set_tx_dev_info_cnt,
	.acc_get_tx_dev_state = qi_protocol_acc_get_tx_dev_state,
	.acc_get_tx_dev_no = qi_protocol_acc_get_tx_dev_no,
	.acc_get_tx_dev_mac = qi_protocol_acc_get_tx_dev_mac,
	.acc_get_tx_dev_model_id = qi_protocol_acc_get_tx_dev_model_id,
	.acc_get_tx_dev_submodel_id = qi_protocol_acc_get_tx_dev_submodel_id,
	.acc_get_tx_dev_version = qi_protocol_acc_get_tx_dev_version,
	.acc_get_tx_dev_business = qi_protocol_acc_get_tx_dev_business,
	.acc_get_tx_dev_info_cnt = qi_protocol_acc_get_tx_dev_info_cnt,
	.get_rx_random = qi_protocol_get_rx_random,
	.set_tx_cipherkey = qi_protocol_set_tx_cipherkey,
};

/* 0x01 + signal_strength + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x01(unsigned int ic_type, u8 *data)
{
	hwlog_info("ask_packet_cmd_0x01: %d\n", data[QI_ASK_PACKET_DAT0]);
	return 0;
}

/* 0x02 + 0xaa + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x02(unsigned int ic_type, u8 *data)
{
	u8 cmd = data[QI_ASK_PACKET_CMD];

	switch (cmd) {
	case QI_CMD_GET_RX_EPT:
		power_event_bnc_notify(POWER_BNT_LIGHTSTRAP, POWER_NE_LIGHTSTRAP_EPT, NULL);
		break;
	default:
		break;
	}

	return 0;
}

/* 0x18 + 0x05 + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x05(unsigned int ic_type)
{
	u8 data[QI_ACK_TX_FWVER_LEN - 1] = { 0 };
	int retry = WIRELESS_RETRY_ONE;

	qi_protocol_get_chip_fw_version(ic_type, data, QI_ACK_TX_FWVER_LEN - 1, retry);
	qi_protocol_send_tx_fw_version(ic_type, data, QI_ACK_TX_FWVER_LEN - 1);

	hwlog_info("ask_packet_cmd_0x05\n");
	return 0;
}

/* 0x38 + 0x0a + volt_lbyte + volt_hbyte + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x0a(unsigned int ic_type, u8 *data)
{
	int tx_vset;

	tx_vset = (data[QI_ASK_PACKET_DAT2] << QI_PROTOCOL_BYTE_BITS) |
		data[QI_ASK_PACKET_DAT1];
	power_event_bnc_notify(qi_protocol_get_bnt_wltx_type(ic_type),
		POWER_NE_WLTX_ASK_SET_VTX, &tx_vset);
	if (qi_protocol_send_fsk_ack_msg(ic_type))
		hwlog_err("ask_packet_cmd_0x0a: send ack fail\n");

	hwlog_info("ask_packet_cmd_0x0a: tx_vset=0x%x\n", tx_vset);
	return 0;
}

/* 0x28 + 0x1a + rx_evt + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x1a(unsigned int ic_type, u8 *data)
{
	power_event_bnc_notify(qi_protocol_get_bnt_wltx_type(ic_type),
		POWER_NE_WLTX_ASK_RX_EVT, &data[QI_ASK_PACKET_DAT1]);
	if (qi_protocol_send_fsk_ack_msg(ic_type))
		hwlog_err("ask_packet_cmd_0x1a: send ack fail\n");

	return 0;
}

/* 0x38 + 0x3b + id_hbyte + id_lbyte + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x3b(unsigned int ic_type, u8 *data)
{
	int tx_id;

	tx_id = (data[QI_ASK_PACKET_DAT1] << QI_PROTOCOL_BYTE_BITS) |
		data[QI_ASK_PACKET_DAT2];

	if (tx_id == QI_HANDSHAKE_ID_HW) {
		qi_protocol_send_tx_id(ic_type, &data[QI_ASK_PACKET_DAT1],
			QI_PARA_TX_ID_LEN);
		hwlog_info("0x8866 handshake succ\n");
		power_event_bnc_notify(qi_protocol_get_bnt_wltx_type(ic_type),
			POWER_NE_WLTX_HANDSHAKE_SUCC, NULL);
	}

	hwlog_info("ask_packet_cmd_0x3b: tx_id=0x%x\n", tx_id);
	return 0;
}

/* 0x18 + 0x41 + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x41(unsigned int ic_type, u8 *data)
{
	power_event_bnc_notify(qi_protocol_get_bnt_wltx_type(ic_type),
		POWER_NE_WLTX_GET_TX_CAP, NULL);

	hwlog_info("ask_packet_cmd_0x41\n");
	return 0;
}

/* 0x18 + 0x38 + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x38(unsigned int ic_type)
{
	int i;
	int retry = WIRELESS_RETRY_ONE;
	u8 cipherkey[QI_TX_CIPHERKEY_FSK_LEN] = { 0 };
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev)
		return -EINVAL;

	for (i = 0; i < QI_TX_CIPHERKEY_FSK_LEN; i++)
		cipherkey[i] = l_dev->cipherkey_info.tx_cipherkey[i];

	if (qi_protocol_send_fsk_msg(ic_type, QI_CMD_GET_HASH, cipherkey,
		QI_TX_CIPHERKEY_FSK_LEN, retry)) {
		hwlog_err("send fsk 0x38 error\n");
		return -EINVAL;
	}

	hwlog_info("ask_packet_cmd_0x38\n");
	return 0;
}

/* 0x18 + 0x39 + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x39(unsigned int ic_type)
{
	int i;
	int retry = WIRELESS_RETRY_ONE;
	u8 cipherkey[QI_TX_CIPHERKEY_FSK_LEN] = { 0 };
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev)
		return -EINVAL;

	for (i = 0; i < QI_TX_CIPHERKEY_FSK_LEN; i++)
		cipherkey[i] = l_dev->cipherkey_info.tx_cipherkey[QI_TX_CIPHERKEY_FSK_LEN + i];

	if (qi_protocol_send_fsk_msg(ic_type, QI_CMD_GET_HASH7_4, cipherkey,
		QI_TX_CIPHERKEY_FSK_LEN, retry)) {
		hwlog_err("send fsk 0x39 error\n");
		return -EINVAL;
	}

	hwlog_info("ask_packet_cmd_0x39\n");
	return 0;
}

/* 0x28 + 0x43 + charger_state + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x43(unsigned int ic_type, u8 *data)
{
	int chrg_state;

	chrg_state = data[QI_ASK_PACKET_DAT1];
	qi_protocol_send_fsk_ack_msg(ic_type);

	if (chrg_state & QI_CHRG_STATE_DONE) {
		hwlog_info("tx received rx charge-done event\n");
		power_event_bnc_notify(qi_protocol_get_bnt_wltx_type(ic_type),
			POWER_NE_WLTX_CHARGEDONE, NULL);
	}

	hwlog_info("ask_packet_cmd_0x43: charger_state=0x%x\n", chrg_state);
	return 0;
}

/* 0x28 + 0x55 + rx_pmax + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x55(unsigned int ic_type, u8 *data)
{
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);
	u16 rx_pmax = data[QI_ASK_PACKET_DAT1] * QI_SEND_RX_PMAX_UNIT;

	if (!l_dev)
		return -EINVAL;

	qi_protocol_send_fsk_ack_msg(ic_type);
	power_event_bnc_notify(qi_protocol_get_bnt_wltx_type(ic_type),
		POWER_NE_WLTX_GET_RX_MAX_POWER, &rx_pmax);

	hwlog_info("ask_packet_cmd_0x55: rx_pmax=%u\n", rx_pmax);
	return 0;
}

/* 0x58 + 0x0e + mfr_id + product_id + product_type + rx_ver + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x0e(unsigned int ic_type, u8 *data)
{
	u8 product_info[QI_PKT_DATA];

	product_info[QI_ACC_OFFSET0] = data[QI_ASK_PACKET_DAT3];
	product_info[QI_ACC_OFFSET1] = data[QI_ASK_PACKET_DAT2];
	qi_protocol_send_fsk_msg(ic_type, QI_CMD_ACK, product_info, 0, WIRELESS_RETRY_THREE);
	power_event_bnc_notify(POWER_BNT_LIGHTSTRAP,
		POWER_NE_LIGHTSTRAP_GET_PRODUCT_INFO, product_info);
	power_event_bnc_notify(qi_protocol_get_bnt_wltx_type(ic_type),
		POWER_NE_WLTX_GET_RX_PRODUCT_TYPE, &product_info[QI_ACC_OFFSET0]);

	hwlog_info("ask_packet_cmd_0x0e: get rx product_info\n");
	return 0;
}

/* 0x58 + 0x52 + mac1 + mac2 + mac3 + mac4 + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x52(unsigned int ic_type, u8 *data)
{
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev)
		return -EINVAL;

	qi_protocol_send_fsk_ack_msg(ic_type);

	hwlog_info("ask_packet_cmd_0x52: %u\n", l_dev->acc_info.dev_info_cnt);
	if (l_dev->acc_info.dev_info_cnt != 0)
		l_dev->acc_info.dev_info_cnt = 0;

	l_dev->acc_info.dev_mac[QI_ACC_OFFSET0] = data[QI_ASK_PACKET_DAT1];
	l_dev->acc_info.dev_mac[QI_ACC_OFFSET1] = data[QI_ASK_PACKET_DAT2];
	l_dev->acc_info.dev_mac[QI_ACC_OFFSET2] = data[QI_ASK_PACKET_DAT3];
	l_dev->acc_info.dev_mac[QI_ACC_OFFSET3] = data[QI_ASK_PACKET_DAT4];
	l_dev->acc_info.dev_info_cnt += QI_ASK_PACKET_DAT_LEN;

	return 0;
}

/* 0x58 + 0x53 + mac5 + mac6 + version + business + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x53(unsigned int ic_type, u8 *data)
{
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev)
		return -EINVAL;

	qi_protocol_send_fsk_ack_msg(ic_type);

	hwlog_info("ask_packet_cmd_0x53: %u\n", l_dev->acc_info.dev_info_cnt);
	if (l_dev->acc_info.dev_info_cnt < QI_ASK_PACKET_DAT_LEN) {
		hwlog_info("cmd_0x53 cnt not right\n");
		return -EINVAL;
	}

	/*
	 * solve rx not receive ack from tx, and sustain send ask packet,
	 * but tx data count check fail, reset info count
	 */
	l_dev->acc_info.dev_info_cnt = QI_ASK_PACKET_DAT_LEN;
	l_dev->acc_info.dev_mac[QI_ACC_OFFSET4] = data[QI_ASK_PACKET_DAT1];
	l_dev->acc_info.dev_mac[QI_ACC_OFFSET5] = data[QI_ASK_PACKET_DAT2];
	l_dev->acc_info.dev_version = data[QI_ASK_PACKET_DAT3];
	l_dev->acc_info.dev_business = data[QI_ASK_PACKET_DAT4];
	l_dev->acc_info.dev_info_cnt += QI_ASK_PACKET_DAT_LEN;

	return 0;
}

/* 0x58 + 0x54 + model1 + model2 + model3 + submodel + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x54(unsigned int ic_type, u8 *data)
{
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev)
		return -EINVAL;

	qi_protocol_send_fsk_ack_msg(ic_type);

	hwlog_info("ask_packet_cmd_0x54: %u\n", l_dev->acc_info.dev_info_cnt);
	if (l_dev->acc_info.dev_info_cnt < QI_ASK_PACKET_DAT_LEN * 2) {
		hwlog_info("cmd_0x54 cnt not right\n");
		return -EINVAL;
	}

	/*
	 * solve rx not receive ack from tx, and sustain send ask packet,
	 * but tx data count check fail, reset info count
	 */
	l_dev->acc_info.dev_info_cnt = QI_ASK_PACKET_DAT_LEN * 2;
	l_dev->acc_info.dev_model_id[QI_ACC_OFFSET0] = data[QI_ASK_PACKET_DAT1];
	l_dev->acc_info.dev_model_id[QI_ACC_OFFSET1] = data[QI_ASK_PACKET_DAT2];
	l_dev->acc_info.dev_model_id[QI_ACC_OFFSET2] = data[QI_ASK_PACKET_DAT3];
	l_dev->acc_info.dev_submodel_id = data[QI_ASK_PACKET_DAT4];
	l_dev->acc_info.dev_info_cnt += QI_ASK_PACKET_DAT_LEN;
	l_dev->acc_info.dev_state = WLTX_ACC_DEV_STATE_ONLINE;
	power_event_bnc_notify(qi_protocol_get_bnt_wltx_type(ic_type),
		POWER_NE_WLTX_ACC_DEV_CONNECTED, NULL);

	hwlog_info("get acc dev info succ\n");
	return 0;
}

/* 0x18 + 0x6b + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x6b(unsigned int ic_type)
{
	u8 data = QI_ACK_RPP_FORMAT_24BIT;

	return qi_protocol_send_fsk_msg(ic_type, QI_CMD_GET_RPP_FORMAT,
		&data, QI_FSK_RPP_FORMAT_LEN, WIRELESS_RETRY_ONE);
}

/* 0x28 + 0x6c + pmax + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x6c(unsigned int ic_type, u8 *data)
{
	u8 pmax;

	(void)qi_protocol_send_fsk_ack_msg(ic_type);
	pmax = (data[QI_ASK_PACKET_DAT1] >> QI_PARA_RX_PMAX_SHIFT) *
		QI_PARA_RX_PMAX_UNIT;

	hwlog_info("received pmax=%dw\n", pmax);
	return qi_protocol_set_rpp_format_post(ic_type, pmax, WIRELESS_TX);
}

/* 0x58 + 0x36 + random0 + random1 + random2 + random3 + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x36(unsigned int ic_type, u8 *data)
{
	int i;
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev)
		return -EINVAL;

	qi_protocol_send_fsk_ack_msg(ic_type);

	memset(l_dev->cipherkey_info.rx_random, 0,
		sizeof(l_dev->cipherkey_info.rx_random));
	l_dev->cipherkey_info.rx_random_len = 0;
	for (i = 0; i < QI_ASK_PACKET_DAT_LEN; i++) {
		l_dev->cipherkey_info.rx_random[i] = data[QI_PKT_DATA + i];
		l_dev->cipherkey_info.rx_random_len++;
	}

	return 0;
}

/* 0x58 + 0x37 + random4 + random5 + random6 + random7 + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x37(unsigned int ic_type, u8 *data)
{
	u8 *rx_random = NULL;
	u8 rx_random_len;
	int i;
	struct qi_protocol_dev *l_dev = qi_protocol_get_dev(ic_type);

	if (!l_dev)
		return -EINVAL;

	qi_protocol_send_fsk_ack_msg(ic_type);

	for (i = 0; i < QI_ASK_PACKET_DAT_LEN; i++) {
		l_dev->cipherkey_info.rx_random[QI_ASK_PACKET_DAT_LEN + i] =
			data[QI_PKT_DATA + i];
		l_dev->cipherkey_info.rx_random_len++;
	}

	rx_random = wltx_auth_get_hash_data_header();
	if (!rx_random) {
		hwlog_err("get hash data header fail\n");
		return -EINVAL;
	}

	rx_random_len = wltx_auth_get_hash_data_size();
	if (l_dev->cipherkey_info.rx_random_len != rx_random_len) {
		hwlog_err("rx random len error\n");
		return -EINVAL;
	}

	wltx_auth_clean_hash_data();
	for (i = 0; i < l_dev->cipherkey_info.rx_random_len; i++)
		rx_random[i] = l_dev->cipherkey_info.rx_random[i];

	wltx_auth_wait_completion();

	wltx_auth_clean_hash_data();
	for (i = 0; i < l_dev->cipherkey_info.rx_random_len; i++)
		l_dev->cipherkey_info.rx_random[i] = 0;

	return 0;
}

static int qi_protocol_handle_ask_packet_hdr_0x18(unsigned int ic_type, u8 *data)
{
	u8 cmd = data[QI_ASK_PACKET_CMD];

	switch (cmd) {
	case QI_CMD_GET_TX_VERSION:
		return qi_protocol_handle_ask_packet_cmd_0x05(ic_type);
	case QI_CMD_GET_TX_CAP:
		return qi_protocol_handle_ask_packet_cmd_0x41(ic_type, data);
	case QI_CMD_GET_HASH:
		return qi_protocol_handle_ask_packet_cmd_0x38(ic_type);
	case QI_CMD_GET_HASH7_4:
		return qi_protocol_handle_ask_packet_cmd_0x39(ic_type);
	case QI_CMD_GET_RPP_FORMAT:
		return qi_protocol_handle_ask_packet_cmd_0x6b(ic_type);
	default:
		hwlog_err("invalid hdr=0x18 cmd=0x%x\n", cmd);
		return -EINVAL;
	}
}

static int qi_protocol_handle_ask_packet_hdr_0x28(unsigned int ic_type, u8 *data)
{
	u8 cmd = data[QI_ASK_PACKET_CMD];

	switch (cmd) {
	case QI_CMD_SEND_RX_EVT:
		return qi_protocol_handle_ask_packet_cmd_0x1a(ic_type, data);
	case QI_CMD_SEND_CHRG_STATE:
		return qi_protocol_handle_ask_packet_cmd_0x43(ic_type, data);
	case QI_CMD_GET_RX_PMAX:
		return qi_protocol_handle_ask_packet_cmd_0x55(ic_type, data);
	case QI_CMD_SET_RX_MAX_POWER:
		return qi_protocol_handle_ask_packet_cmd_0x6c(ic_type, data);
	default:
		hwlog_err("invalid hdr=0x28 cmd=0x%x\n", cmd);
		return -EINVAL;
	}
}

static int qi_protocol_handle_ask_packet_hdr_0x38(unsigned int ic_type, u8 *data)
{
	u8 cmd = data[QI_ASK_PACKET_CMD];

	switch (cmd) {
	case QI_CMD_SET_TX_VIN:
		return qi_protocol_handle_ask_packet_cmd_0x0a(ic_type, data);
	case QI_CMD_GET_TX_ID:
		return qi_protocol_handle_ask_packet_cmd_0x3b(ic_type, data);
	default:
		hwlog_err("invalid hdr=0x38 cmd=0x%x\n", cmd);
		return -EINVAL;
	}
}

static int qi_protocol_handle_ask_packet_hdr_0x48(unsigned int ic_type, u8 *data)
{
	u8 cmd = data[QI_ASK_PACKET_CMD];

	switch (cmd) {
	default:
		hwlog_err("invalid hdr=0x48 cmd=0x%x\n", cmd);
		return -EINVAL;
	}
}

static int qi_protocol_handle_ask_packet_hdr_0x58(unsigned int ic_type, u8 *data)
{
	u8 cmd = data[QI_ASK_PACKET_CMD];

	switch (cmd) {
	case QI_CMD_GET_RX_PRODUCT_INFO:
		return qi_protocol_handle_ask_packet_cmd_0x0e(ic_type, data);
	case QI_CMD_SEND_BT_MAC1:
		return qi_protocol_handle_ask_packet_cmd_0x52(ic_type, data);
	case QI_CMD_SEND_BT_MAC2:
		return qi_protocol_handle_ask_packet_cmd_0x53(ic_type, data);
	case QI_CMD_SEND_BT_MODEL_ID:
		return qi_protocol_handle_ask_packet_cmd_0x54(ic_type, data);
	case QI_CMD_START_CERT:
		return qi_protocol_handle_ask_packet_cmd_0x36(ic_type, data);
	case QI_CMD_SEND_RAMDOM7_4:
		return qi_protocol_handle_ask_packet_cmd_0x37(ic_type, data);
	default:
		hwlog_err("invalid hdr=0x58 cmd=0x%x\n", cmd);
		return -EINVAL;
	}
}

static int qi_protocol_handle_ask_packet_data(unsigned int ic_type, u8 *data)
{
	u8 hdr = data[QI_ASK_PACKET_HDR];

	switch (hdr) {
	case QI_CMD_GET_SIGNAL_STRENGTH:
		return qi_protocol_handle_ask_packet_cmd_0x01(ic_type, data);
	case QI_CMD_RX_EPT_TYPE:
		return qi_protocol_handle_ask_packet_cmd_0x02(ic_type, data);
	case QI_ASK_PACKET_HDR_MSG_SIZE_1_BYTE:
		return qi_protocol_handle_ask_packet_hdr_0x18(ic_type, data);
	case QI_ASK_PACKET_HDR_MSG_SIZE_2_BYTE:
		return qi_protocol_handle_ask_packet_hdr_0x28(ic_type, data);
	case QI_ASK_PACKET_HDR_MSG_SIZE_3_BYTE:
		return qi_protocol_handle_ask_packet_hdr_0x38(ic_type, data);
	case QI_ASK_PACKET_HDR_MSG_SIZE_4_BYTE:
		return qi_protocol_handle_ask_packet_hdr_0x48(ic_type, data);
	case QI_ASK_PACKET_HDR_MSG_SIZE_5_BYTE:
		return qi_protocol_handle_ask_packet_hdr_0x58(ic_type, data);
	default:
		hwlog_err("invalid hdr=0x%x\n", hdr);
		return -EINVAL;
	}
}

/*
 * ask: rx to tx
 * we use ask mode when rx sends a message to tx
 */
static int qi_protocol_handle_ask_packet(void *dev_data)
{
	u8 data[QI_ASK_PACKET_LEN] = { 0 };
	int retry = WIRELESS_RETRY_ONE;
	unsigned int ic_type = qi_protocol_get_ic_type(dev_data);

	if (qi_protocol_get_ask_packet(ic_type, data, QI_ASK_PACKET_LEN, retry))
		return -EINVAL;

	if (qi_protocol_handle_ask_packet_data(ic_type, data))
		return -EINVAL;

	return 0;
}

static void qi_protocol_handle_fsk_tx_alarm(unsigned int ic_type, u8 *pkt)
{
	u8 header;
	struct wireless_protocol_tx_alarm tx_alarm;

	header = pkt[QI_PKT_HDR];
	tx_alarm.src = pkt[QI_PKT_DATA + QI_TX_ALARM_SRC];
	tx_alarm.plim = pkt[QI_PKT_DATA + QI_TX_ALARM_PLIM] * QI_TX_ALARM_PLIM_STEP;
	if (header == QI_FSK_PKT_CMD_MSG_SIZE_5_BYTE) {
		tx_alarm.vlim = pkt[QI_PKT_DATA + QI_TX_ALARM_VLIM] *
			QI_TX_ALARM_VLIM_STEP;
		tx_alarm.reserved = pkt[QI_PKT_DATA + QI_TX_ALARM_RSVD];
	} else if (tx_alarm.src) {
		tx_alarm.vlim = WIRELESS_ADAPTER_9V;
		tx_alarm.reserved = 0;
	} else {
		tx_alarm.vlim = 0;
		tx_alarm.reserved = 0;
	}
	power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_TX_ALARM, &tx_alarm);
	(void)qi_protocol_send_msg_only(ic_type, QI_CMD_ACK, NULL, 0, WIRELESS_RETRY_THREE);
}

/*
 * fsk: tx to rx
 * we use fsk mode when tx sends a message to rx
 */
static void qi_protocol_handle_fsk_packet(u8 *pkt, int len, void *dev_data)
{
	unsigned int ic_type = qi_protocol_get_ic_type(dev_data);

	if (!pkt || (len != QI_PKT_LEN)) {
		hwlog_err("para error\n");
		return;
	}

	switch (pkt[QI_PKT_CMD]) {
	case QI_CMD_TX_ALARM:
		hwlog_info("received cmd with tx alarm\n");
		qi_protocol_handle_fsk_tx_alarm(ic_type, pkt);
		break;
	case QI_CMD_ACK_BST_ERR:
		hwlog_info("received cmd with tx boost err\n");
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_TX_BST_ERR, NULL);
		break;
	default:
		break;
	}
}

static u8 qi_protocol_get_ask_header(int data_len)
{
	if ((data_len <= 0) ||
		(data_len >= (int)ARRAY_SIZE(g_qi_protocol_ask_hdr))) {
		hwlog_err("para error\n");
		return 0;
	}

	return g_qi_protocol_ask_hdr[data_len];
}

static u8 qi_protocol_get_fsk_header(int data_len)
{
	if ((data_len <= 0) ||
		(data_len >= (int)ARRAY_SIZE(g_qi_protocol_fsk_hdr))) {
		hwlog_err("para error\n");
		return 0;
	}

	return g_qi_protocol_fsk_hdr[data_len];
}

static struct qi_protocol_handle g_qi_protocol_handle = {
	.get_ask_hdr = qi_protocol_get_ask_header,
	.get_fsk_hdr = qi_protocol_get_fsk_header,
	.hdl_qi_ask_pkt = qi_protocol_handle_ask_packet,
	.hdl_non_qi_ask_pkt = qi_protocol_handle_ask_packet,
	.hdl_non_qi_fsk_pkt = qi_protocol_handle_fsk_packet,
};

static int __init qi_protocol_init(void)
{
	int ret;
	int i = 0;
	struct qi_protocol_dev *l_dev = NULL;

	while (i < WLTRX_IC_MAX) {
		l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
		if (!l_dev) {
			ret = -ENOMEM;
			goto fail_malloc_dev;
		}
		g_qi_protocol_dev[i] = l_dev;
		l_dev->dev_id = WIRELESS_DEVICE_ID_END;
		l_dev->acc_info.dev_no = qi_protocol_get_acc_dev_no(i);
		i++;
	}

	ret = wireless_protocol_ops_register(&wireless_protocol_qi_ops);
	if (ret)
		goto fail_register_ops;

	return 0;

fail_malloc_dev:
fail_register_ops:
	while (--i >= 0) {
		kfree(g_qi_protocol_dev[i]);
		g_qi_protocol_dev[i] = NULL;
	}
	return ret;
}

static void __exit qi_protocol_exit(void)
{
	int i = 0;

	while (i < WLTRX_IC_MAX) {
		kfree(g_qi_protocol_dev[i]);
		g_qi_protocol_dev[i] = NULL;
		i++;
	}
}

subsys_initcall_sync(qi_protocol_init);
module_exit(qi_protocol_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("qi protocol driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
