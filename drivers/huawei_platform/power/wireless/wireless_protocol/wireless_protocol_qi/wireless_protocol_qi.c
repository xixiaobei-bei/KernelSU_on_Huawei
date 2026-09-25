/*
 * wireless_protocol_qi.c
 *
 * qi protocol driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/delay.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/power_cmdline.h>
#include <huawei_platform/power/wireless_transmitter.h>
#include <huawei_platform/power/wireless/wireless_protocol.h>
#include <huawei_platform/power/wireless/wireless_protocol_qi.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG wireless_protocol_qi
HWLOG_REGIST();

static struct qi_protocol_dev *g_qi_protocol_dev;
static struct qi_protocol_handle g_qi_protocol_handle;

static const struct wireless_protocol_device_data g_qi_protocol_dev_data[] = {
	{ WIRELESS_DEVICE_ID_IDTP9221, "idtp9221" },
	{ WIRELESS_DEVICE_ID_STWLC68, "stwlc68" },
	{ WIRELESS_DEVICE_ID_IDTP9415, "idtp9415" },
	{ WIRELESS_DEVICE_ID_NU1619, "nu1619" },
};

static int qi_protocol_get_device_id(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_qi_protocol_dev_data); i++) {
		if (!strncmp(str, g_qi_protocol_dev_data[i].name,
			strlen(str)))
			return g_qi_protocol_dev_data[i].id;
	}

	return -1;
}

static struct qi_protocol_dev *qi_protocol_get_dev(void)
{
	if (!g_qi_protocol_dev) {
		hwlog_err("g_qi_protocol_dev is null\n");
		return NULL;
	}

	return g_qi_protocol_dev;
}

static struct qi_protocol_ops *qi_protocol_get_ops(void)
{
	if (!g_qi_protocol_dev || !g_qi_protocol_dev->p_ops) {
		hwlog_err("g_qi_protocol_dev or p_ops is null\n");
		return NULL;
	}

	return g_qi_protocol_dev->p_ops;
}

int qi_protocol_ops_register(struct qi_protocol_ops *ops)
{
	int dev_id;

	if (!g_qi_protocol_dev || !ops || !ops->chip_name) {
		hwlog_err("g_qi_protocol_dev or ops or chip_name is null\n");
		return -1;
	}

	dev_id = qi_protocol_get_device_id(ops->chip_name);
	if (dev_id < 0) {
		hwlog_err("%s ops register fail\n", ops->chip_name);
		return -1;
	}

	g_qi_protocol_dev->p_ops = ops;
	g_qi_protocol_dev->dev_id = dev_id;

	hwlog_info("%d:%s ops register ok\n", dev_id, ops->chip_name);
	return 0;
}

struct qi_protocol_handle *qi_protocol_get_handle(void)
{
	return &g_qi_protocol_handle;
}

static int qi_protocol_send_msg(u8 cmd,
	u8 *para, int para_len, u8 *data, int data_len, int retrys)
{
	int i;
	int ret;
	struct qi_protocol_ops *l_ops = NULL;

	l_ops = qi_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->send_msg || !l_ops->receive_msg) {
		hwlog_err("send_msg or receive_msg is null\n");
		return -1;
	}

	for (i = 0; i < retrys; i++) {
		ret = l_ops->send_msg(cmd, para, para_len);
		if (ret) {
			hwlog_err("0x%x msg send fail, retry %d\n", cmd, i);
			continue;
		}

		ret = l_ops->receive_msg(data, data_len);
		if (ret) {
			hwlog_err("0x%x msg receive fail, retry %d\n", cmd, i);
			continue;
		}

		break;
	}

	if (i >= retrys)
		return -1;

	/* protocol define: the first byte of the response must be a command */
	if (data[QI_ACK_CMD_OFFSET] != cmd) {
		hwlog_err("data[%d] 0x%x not equal cmd 0x%x\n",
			QI_ACK_CMD_OFFSET, data[QI_ACK_CMD_OFFSET], cmd);
		return -1;
	}

	return 0;
}

static int qi_protocol_send_msg_ack(u8 cmd, u8 *para, int para_len, int retrys)
{
	int i;
	int ret;
	struct qi_protocol_ops *l_ops = NULL;

	l_ops = qi_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->send_msg_with_ack) {
		hwlog_err("send_msg_with_ack is null\n");
		return -1;
	}

	for (i = 0; i < retrys; i++) {
		ret = l_ops->send_msg_with_ack(cmd, para, para_len);
		if (ret) {
			hwlog_err("0x%x msg_ack send fail, retry %d\n", cmd, i);
			continue;
		}

		break;
	}

	if (i >= retrys)
		return -1;

	return 0;
}

static int qi_protocol_send_fsk_msg(u8 cmd, u8 *para, int para_len, int retrys)
{
	int i;
	int ret;
	struct qi_protocol_ops *l_ops = NULL;

	l_ops = qi_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->send_fsk_msg) {
		hwlog_err("send_fsk_msg is null\n");
		return -1;
	}

	for (i = 0; i < retrys; i++) {
		ret = l_ops->send_fsk_msg(cmd, para, para_len);
		if (ret) {
			hwlog_err("0x%x fsk_msg send fail, retry %d\n", cmd, i);
			continue;
		}

		break;
	}

	if (i >= retrys)
		return -1;

	return 0;
}

static int qi_protocol_send_fsk_ack_msg(void)
{
	return qi_protocol_send_fsk_msg(QI_CMD_ACK, NULL, 0,
		WIRELESS_RETRY_ONE);
}

static int qi_protocol_get_ask_packet(u8 *data, int data_len, int retrys)
{
	int i;
	int ret;
	struct qi_protocol_ops *l_ops = NULL;

	l_ops = qi_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->get_ask_packet) {
		hwlog_err("get_ask_packet is null\n");
		return -1;
	}

	for (i = 0; i < retrys; i++) {
		ret = l_ops->get_ask_packet(data, data_len);
		if (ret) {
			hwlog_err("ask_packet receive fail, retry %d\n", i);
			continue;
		}

		break;
	}

	if (i >= retrys)
		return -1;

	return 0;
}

static int qi_protocol_get_chip_fw_version(u8 *data, int data_len, int retrys)
{
	int i;
	int ret;
	struct qi_protocol_ops *l_ops = NULL;

	l_ops = qi_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->get_chip_fw_version) {
		hwlog_err("get_chip_fw_version is null\n");
		return -1;
	}

	for (i = 0; i < retrys; i++) {
		ret = l_ops->get_chip_fw_version(data, data_len);
		if (ret) {
			hwlog_err("chip_fw_version get fail, retry %d\n", i);
			continue;
		}

		break;
	}

	if (i >= retrys)
		return -1;

	return 0;
}

static int qi_protocol_send_rx_vout(int rx_vout)
{
	int retry = WIRELESS_RETRY_ONE;

	if (qi_protocol_send_msg_ack(QI_CMD_SEND_RX_VOUT,
		(u8 *)&rx_vout, QI_PARA_RX_VOUT_LEN, retry))
		return -1;

	return 0;
}

static int qi_protocol_send_rx_iout(int rx_iout)
{
	int retry = WIRELESS_RETRY_ONE;

	if (qi_protocol_send_msg_ack(QI_CMD_SEND_RX_IOUT,
		(u8 *)&rx_iout, QI_PARA_RX_IOUT_LEN, retry))
		return -1;

	return 0;
}

static int qi_protocol_send_rx_boost_succ(void)
{
	int retry = WIRELESS_RETRY_ONE;

	if (qi_protocol_send_msg_ack(QI_CMD_RX_BOOST_SUCC, NULL, 0, retry))
		return -1;

	return 0;
}

static int qi_protocol_send_rx_ready(void)
{
	int retry = WIRELESS_RETRY_ONE;

	if (qi_protocol_send_msg_ack(QI_CMD_SEND_RX_READY, NULL, 0, retry))
		return -1;

	hwlog_info("send_rx_ready succ\n");
	return 0;
}

static int qi_protocol_send_tx_capability(u8 *cap, int len)
{
	int retry = WIRELESS_RETRY_ONE;

	if (!cap)
		return -1;

	if (len != (QI_PARA_TX_CAP_LEN - 1)) {
		hwlog_err("para error %d!=%d\n", len, QI_PARA_TX_CAP_LEN - 1);
		return -1;
	}

	if (qi_protocol_send_fsk_msg(QI_CMD_GET_TX_CAP, cap, len, retry))
		return -1;

	hwlog_info("send_tx_capability succ\n");
	return 0;
}

static int qi_protocol_send_tx_fw_version(u8 *fw, int len)
{
	int retry = WIRELESS_RETRY_ONE;

	if (!fw)
		return -1;

	if (len != (QI_ACK_TX_FWVER_LEN - 1)) {
		hwlog_err("para error %d!=%d\n", len, QI_ACK_TX_FWVER_LEN - 1);
		return -1;
	}

	if (qi_protocol_send_fsk_msg(QI_CMD_GET_TX_VERSION, fw, len, retry))
		return -1;

	hwlog_info("send_tx_fw_version succ\n");
	return 0;
}

static int qi_protocol_send_tx_id(u8 *id, int len)
{
	int retry = WIRELESS_RETRY_ONE;

	if (!id)
		return -1;

	if (len != QI_PARA_TX_ID_LEN) {
		hwlog_err("para error %d!=%d\n", len, QI_PARA_TX_ID_LEN);
		return -1;
	}

	if (qi_protocol_send_fsk_msg(QI_CMD_GET_TX_ID, id, len, retry))
		return -1;

	hwlog_info("send_tx_id succ\n");
	return 0;
}

static int qi_protocol_send_cert_confirm(bool succ_flag)
{
	int retry = WIRELESS_RETRY_ONE;

	if (succ_flag)
		return qi_protocol_send_msg_ack(QI_CMD_CERT_SUCC,
			NULL, 0, retry);
	else
		return qi_protocol_send_msg_ack(QI_CMD_CERT_FAIL,
			NULL, 0, retry);
}

static int qi_protocol_send_charge_state(u8 state)
{
	int retry = WIRELESS_RETRY_ONE;

	if (qi_protocol_send_msg_ack(QI_CMD_SEND_CHRG_STATE,
		&state, QI_PARA_CHARGE_STATE_LEN, retry))
		return -1;

	return 0;
}

static int qi_protocol_send_fod_status(int status)
{
	int retry = WIRELESS_RETRY_ONE;

	if (qi_protocol_send_msg_ack(QI_CMD_SEND_FOD_STATUS,
		(u8 *)&status, QI_PARA_FOD_STATUS_LEN, retry))
		return -1;

	return 0;
}

static int qi_protocol_send_charge_mode(u8 mode)
{
	int retry = WIRELESS_RETRY_ONE;
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev)
		return -1;

	if (l_dev->dev_id == WIRELESS_DEVICE_ID_IDTP9221) {
		hwlog_info("send_charge_mode: not support\n");
		return 0;
	}

	if (qi_protocol_send_msg_ack(QI_CMD_SEND_CHRG_MODE,
		&mode, QI_PARA_CHARGE_MODE_LEN, retry))
		return -1;

	return 0;
}

static int qi_protocol_set_fan_speed_limit(u8 limit)
{
	int retry = WIRELESS_RETRY_ONE;
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev)
		return -1;

	if (l_dev->dev_id == WIRELESS_DEVICE_ID_IDTP9221) {
		hwlog_info("set_fan_speed_limit: not support\n");
		return 0;
	}

	if (qi_protocol_send_msg_ack(QI_CMD_SET_FAN_SPEED_LIMIT,
		&limit, QI_PARA_FAN_SPEED_LIMIT_LEN, retry))
		return -1;

	return 0;
}

static int qi_protocol_set_rpp_format_post(u8 pmax)
{
	struct qi_protocol_ops *l_ops = NULL;

	l_ops = qi_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->set_rpp_format_post)
		return 0;

	return l_ops->set_rpp_format_post(pmax);
}

static int qi_protocol_set_rpp_format(u8 pmax)
{
	u8 data[QI_PARA_RX_PMAX_LEN] = { 0 };
	int retry = WIRELESS_RETRY_ONE;
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev)
		return -1;

	if (l_dev->dev_id == WIRELESS_DEVICE_ID_IDTP9221) {
		hwlog_info("set_rpp_format: not support\n");
		return 0;
	}

	data[QI_PARA_RX_PMAX_OFFSET] = pmax / QI_PARA_RX_PMAX_UNIT;
	data[QI_PARA_RX_PMAX_OFFSET] <<= QI_PARA_RX_PMAX_SHIFT;

	if (qi_protocol_send_msg_ack(QI_CMD_SET_RX_MAX_POWER,
		data, QI_PARA_RX_PMAX_LEN, retry))
		return -1;

	hwlog_info("send_rpp_format: pmax=%d, format=0x%x\n",
		pmax, data[QI_PARA_RX_PMAX_OFFSET]);

	if (qi_protocol_set_rpp_format_post(pmax))
		return -1;

	return 0;
}

static int qi_protocol_get_ept_type(u16 *type)
{
	u8 data[QI_ACK_EPT_TYPE_LEN] = { 0 };
	int retry = WIRELESS_RETRY_TWO;
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev || !type)
		return -1;

	if (l_dev->dev_id == WIRELESS_DEVICE_ID_IDTP9221) {
		*type = 0;
		hwlog_info("get_ept_type: not support\n");
		return 0;
	}

	if (qi_protocol_send_msg(QI_CMD_GET_EPT_TYPE,
		NULL, 0, data, QI_ACK_EPT_TYPE_LEN, retry))
		return -1;

	*type = (data[QI_ACK_EPT_TYPE_E_OFFSET] << QI_PROTOCOL_BYTE_BITS) |
		data[QI_ACK_EPT_TYPE_S_OFFSET];

	return 0;
}

static int qi_protocol_get_rpp_format(u8 *format)
{
	u8 data[QI_ACK_RPP_FORMAT_LEN] = { 0 };
	int retry = WIRELESS_RETRY_TWO;
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev || !format)
		return -1;
	if (l_dev->dev_id == WIRELESS_DEVICE_ID_IDTP9221) {
		*format = QI_ACK_RPP_FORMAT_8BIT;
		hwlog_info("get_rpp_format: not support\n");
		return 0;
	}

	if (qi_protocol_send_msg(QI_CMD_GET_RPP_FORMAT,
		NULL, 0, data, QI_ACK_RPP_FORMAT_LEN, retry))
		return -1;

	*format = data[QI_ACK_RPP_FORMAT_OFFSET];

	return 0;
}

static int qi_protocol_get_tx_fw_version(char *ver, int size)
{
	int i;
	int retry = WIRELESS_RETRY_TWO;
	u8 tx_fwver[QI_ACK_TX_FWVER_LEN] = { 0 };
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev || !ver)
		return -1;

	memset(l_dev->info.tx_fwver, 0, sizeof(l_dev->info.tx_fwver));
	if (qi_protocol_send_msg(QI_CMD_GET_TX_VERSION,
		NULL, 0, tx_fwver, QI_ACK_TX_FWVER_LEN, retry))
		return -1;

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

static void qi_protocol_get_tx_bd_info(char *info, int size)
{
	int i;
	int str_len;
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev || !info || (size <= sizeof(l_dev->info)))
		return;

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
}

static int qi_protocol_get_tx_id_pre(void)
{
	struct qi_protocol_ops *l_ops = NULL;

	l_ops = qi_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->get_tx_id_pre)
		return 0;

	return l_ops->get_tx_id_pre();
}

static int qi_protocol_get_tx_id(int *id)
{
	u8 para[QI_PARA_TX_ID_LEN] = { QI_HANDSHAKE_ID_HIGH, QI_HANDSHAKE_ID_LOW };
	u8 data[QI_ACK_TX_ID_LEN] = { 0 };
	int retry = WIRELESS_RETRY_TWO;

	if (!id)
		return -1;

	if (qi_protocol_get_tx_id_pre())
		return -1;

	if (qi_protocol_send_msg(QI_CMD_GET_TX_ID,
		para, QI_PARA_TX_ID_LEN, data, QI_ACK_TX_ID_LEN, retry))
		return -1;

	if (power_cmdline_is_factory_mode())
		qi_protocol_send_rx_ready();

	*id = (data[QI_ACK_TX_ID_S_OFFSET] << QI_PROTOCOL_BYTE_BITS) |
		data[QI_ACK_TX_ID_E_OFFSET];

	return 0;
}

static int qi_protocol_get_tx_type(int *type)
{
	u8 tx_fwver[QI_ACK_TX_FWVER_LEN] = { 0 };
	int retry = WIRELESS_RETRY_TWO;
	u16 data;

	if (!type)
		return -1;

	if (qi_protocol_send_msg(QI_CMD_GET_TX_VERSION,
		NULL, 0, tx_fwver, QI_ACK_TX_FWVER_LEN, retry))
		return -1;

	data = (tx_fwver[QI_ACK_TX_FWVER_E] << QI_PROTOCOL_BYTE_BITS) |
		tx_fwver[QI_ACK_TX_FWVER_E - 1];
	switch (data) {
	case QI_TX_TYPE_CP58:
		*type = WIRELESS_TX_TYPE_CP58;
		break;
	case QI_TX_TYPE_CP60:
		*type = WIRELESS_TX_TYPE_CP60;
		break;
	case QI_TX_TYPE_CP61:
		*type = WIRELESS_TX_TYPE_CP61;
		break;
	case QI_TX_TYPE_CP39S:
		*type = WIRELESS_TX_TYPE_CP39S;
		break;
	default:
		*type = WIRELESS_TX_TYPE_UNKNOWN;
		break;
	}

	hwlog_info("get_tx_type: %d\n", *type);
	return 0;
}

static int qi_protocol_get_tx_adapter_type(int *type)
{
	u8 data[QI_ACK_TX_ADP_TYPE_LEN] = { 0 };
	int retry = WIRELESS_RETRY_TWO;

	if (!type)
		return -1;

	if (qi_protocol_send_msg(QI_CMD_GET_TX_ADAPTER_TYPE,
		NULL, 0, data, QI_ACK_TX_ADP_TYPE_LEN, retry))
		return -1;

	*type = data[QI_ACK_TX_ADP_TYPE_OFFSET];

	return 0;
}

static int qi_protocol_get_tx_main_capability(u8 *cap, int len)
{
	int i;
	int retry = WIRELESS_RETRY_TWO;
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev || !cap)
		return -1;

	if (len != QI_PARA_TX_CAP_LEN) {
		hwlog_err("para error %d!=%d\n", len, QI_PARA_TX_CAP_LEN);
		return -1;
	}

	if (qi_protocol_send_msg(QI_CMD_GET_TX_CAP, NULL, 0, cap, len, retry))
		return -1;

	for (i = QI_PARA_TX_MAIN_CAP_S; i <= QI_PARA_TX_MAIN_CAP_E; i++)
		l_dev->info.tx_main_cap[i - QI_PARA_TX_MAIN_CAP_S] = cap[i];

	return 0;
}

static int qi_protocol_get_tx_extra_capability(u8 *cap, int len)
{
	int i;
	int retry = WIRELESS_RETRY_TWO;
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev || !cap)
		return -1;

	if (len != QI_PARA_TX_EXT_CAP_LEN) {
		hwlog_err("para error %d!=%d\n", len, QI_PARA_TX_EXT_CAP_LEN);
		return -1;
	}

	if (qi_protocol_send_msg(QI_CMD_GET_GET_TX_EXT_CAP,
		NULL, 0, cap, len, retry))
		return -1;

	for (i = QI_PARA_TX_EXT_CAP_S; i <= QI_PARA_TX_EXT_CAP_E; i++)
		l_dev->info.tx_ext_cap[i - QI_PARA_TX_EXT_CAP_S] = cap[i];

	return 0;
}

static int qi_protocol_get_tx_capability(struct wireless_protocol_tx_cap *cap)
{
	u8 data1[QI_PARA_TX_CAP_LEN] = { 0 };
	u8 data2[QI_PARA_TX_EXT_CAP_LEN] = { 0 };

	if (!cap)
		return -1;

	if (qi_protocol_get_tx_main_capability(data1, QI_PARA_TX_CAP_LEN))
		return -1;

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

	if ((cap->vout_max > WIRELESS_ADAPTER_9V) && !cap->support_12v) {
		cap->vout_max = WIRELESS_ADAPTER_9V;
		hwlog_info("tx not support 12v, set to %dmv\n", cap->vout_max);
	}
	if (cap->ext_type == QI_PARA_TX_EXT_TYPE_CAR)
		cap->type += QI_PARA_TX_TYPE_CAR_BASE;
	if (cap->ext_type == QI_PARA_TX_EXT_TYPE_PWR_BANK)
		cap->type += QI_PARA_TX_TYPE_PWR_BANK_BASE;

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

	if (qi_protocol_get_tx_extra_capability(data2, QI_PARA_TX_EXT_CAP_LEN))
		return -1;

	cap->support_fan = data2[QI_TX_EXTRA_CAP_ATTR1] &
		QI_PARA_TX_EXT_CAP_SUPPORT_FAN_MASK;
	cap->support_tec = data2[QI_TX_EXTRA_CAP_ATTR1] &
		QI_PARA_TX_EXT_CAP_SUPPORT_TEC_MASK;
	cap->support_fod_status = data2[QI_TX_EXTRA_CAP_ATTR1] &
		QI_PARA_TX_EXT_CAP_SUPPORT_QVAL_MASK;

	hwlog_info("tx_extra_cap: [support] fan=%d tec=%d fod_status=%d\n",
		cap->support_fan, cap->support_tec, cap->support_fod_status);

	return 0;
}

static int qi_protocol_set_encrypt_index(u8 *data, int index)
{
	if (!data)
		return -1;

	data[QI_PARA_KEY_INDEX_OFFSET] = index;

	hwlog_info("set_encrypt_index: %d\n", index);
	return 0;
}

static int qi_protocol_set_random_num(u8 *data, int start, int end)
{
	int i;

	if (!data)
		return -1;

	for (i = start; i <= end; i++)
		get_random_bytes(&data[i], sizeof(u8));

	return 0;
}

static int qi_protocol_send_random_num(u8 *random, int len)
{
	int i;
	int retry = WIRELESS_RETRY_ONE;

	if (!random)
		return -1;

	if (len != QI_PARA_RANDOM_LEN) {
		hwlog_err("para error %d!=%d\n", len, QI_PARA_RANDOM_LEN);
		return -1;
	}

	for (i = 0; i < len / QI_PARA_RANDOM_GROUP_LEN; i++) {
		if (qi_protocol_send_msg_ack(QI_CMD_START_CERT + i,
			random + i * QI_PARA_RANDOM_GROUP_LEN,
			QI_PARA_RANDOM_GROUP_LEN, retry))
			return -1;
	}

	return 0;
}

static int qi_protocol_get_encrypted_value(u8 *hash, int len)
{
	int i;
	int retry = WIRELESS_RETRY_TWO;

	if (!hash)
		return -1;

	if (len != QI_ACK_HASH_LEN) {
		hwlog_err("para error %d!=%d\n", len, QI_ACK_HASH_LEN);
		return -1;
	}

	for (i = 0; i < len / QI_ACK_HASH_GROUP_LEN; i++) {
		/* cmd 0x38 & 0x39 */
		if (qi_protocol_send_msg(QI_CMD_GET_HASH + i, NULL, 0,
			hash + i * QI_ACK_HASH_GROUP_LEN,
			QI_ACK_HASH_GROUP_LEN, retry))
			return -1;
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

static int qi_protocol_auth_encrypt_start(int key,
	u8 *random, int r_size, u8 *hash, int h_size)
{
	u8 data[QI_ACK_HASH_LEN] = { 0 };
	int size;

	if (!random || !hash)
		return -1;

	memset(random, 0, r_size);
	memset(hash, 0, h_size);

	size = QI_PARA_RANDOM_LEN;
	if (r_size != size) {
		hwlog_err("invalid r_size=%d\n", r_size);
		return -1;
	}

	size = QI_ACK_HASH_LEN - QI_ACK_HASH_LEN / QI_ACK_HASH_GROUP_LEN;
	if (h_size != size) {
		hwlog_err("invalid h_size=%d\n", h_size);
		return -1;
	}

	/* first: set key index */
	if (qi_protocol_set_encrypt_index(random, key))
		return -1;

	/* second: host create random num */
	if (qi_protocol_set_random_num(random,
		QI_PARA_RANDOM_S_OFFSET, QI_PARA_RANDOM_E_OFFSET))
		return -1;

	/* third: host set random num to slave */
	if (qi_protocol_send_random_num(random, QI_PARA_RANDOM_LEN))
		return -1;

	/* fouth: host get hash num from slave */
	if (qi_protocol_get_encrypted_value(data, QI_ACK_HASH_LEN))
		return -1;

	/* fifth: copy hash value */
	if (qi_protocol_copy_hash_value(data, QI_ACK_HASH_LEN, hash, h_size))
		return -1;

	return 0;
}

static int qi_protocol_fix_tx_fop(int fop)
{
	int retry = WIRELESS_RETRY_ONE;

	if ((fop < QI_FIXED_FOP_MIN) || (fop > QI_FIXED_FOP_MAX)) {
		hwlog_err("fixed fop %d exceeds range[%d, %d]\n",
			fop, QI_FIXED_FOP_MIN, QI_FIXED_FOP_MAX);
		return -1;
	}

	/* cmd 0x44 */
	if (qi_protocol_send_msg_ack(QI_CMD_FIX_TX_FOP,
		(u8 *)&fop, QI_PARA_TX_FOP_LEN, retry))
		return -1;

	hwlog_info("fix_tx_fop: %d\n", fop);
	return 0;
}

static int qi_protocol_unfix_tx_fop(void)
{
	int retry = WIRELESS_RETRY_ONE;

	/* cmd 0x45 */
	if (qi_protocol_send_msg_ack(QI_CMD_UNFIX_TX_FOP, NULL, 0, retry))
		return -1;

	hwlog_info("unfix_tx_fop success\n");
	return 0;
}

void qi_protocol_reset_dev_info(void)
{
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev)
		return;

	memset(&l_dev->info, 0, sizeof(l_dev->info));
}

static int qi_protocol_acc_set_tx_dev_state(u8 state)
{
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev)
		return -1;

	l_dev->acc_info.dev_state = state;
	hwlog_info("acc_set_tx_dev_state: 0x%x\n", state);
	return 0;
}

static int qi_protocol_acc_set_tx_dev_info_cnt(u8 cnt)
{
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev)
		return -1;

	l_dev->acc_info.dev_info_cnt = cnt;
	hwlog_info("acc_set_tx_dev_info_cnt: 0x%x\n", cnt);
	return 0;
}

static int qi_protocol_acc_get_tx_dev_state(u8 *state)
{
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev || !state)
		return -1;

	*state = l_dev->acc_info.dev_state;
	hwlog_info("acc_get_tx_dev_state: 0x%x\n", *state);
	return 0;
}

static int qi_protocol_acc_get_tx_dev_no(u8 *no)
{
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev || !no)
		return -1;

	*no = l_dev->acc_info.dev_no;
	hwlog_info("acc_get_tx_dev_no: 0x%x\n", *no);
	return 0;
}

static int qi_protocol_acc_get_tx_dev_mac(u8 *mac, int len)
{
	int i;
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev || !mac)
		return -1;

	if (len != QI_ACC_TX_DEV_MAC_LEN) {
		hwlog_err("para error %d!=%d\n", len, QI_ACC_TX_DEV_MAC_LEN);
		return -1;
	}

	for (i = 0; i < len; i++) {
		mac[i] = l_dev->acc_info.dev_mac[i];
		hwlog_info("acc_get_tx_dev_mac: mac[%d]=0x%x\n", i, mac[i]);
	}
	return 0;
}

static int qi_protocol_acc_get_tx_dev_model_id(u8 *id, int len)
{
	int i;
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev || !id)
		return -1;

	if (len != QI_ACC_TX_DEV_MODELID_LEN) {
		hwlog_err("para error %d!=%d\n", len, QI_ACC_TX_DEV_MODELID_LEN);
		return -1;
	}

	for (i = 0; i < len; i++) {
		id[i] = l_dev->acc_info.dev_model_id[i];
		hwlog_info("acc_get_tx_dev_model_id: id[%d]=0x%x\n", i, id[i]);
	}
	return 0;
}

static int qi_protocol_acc_get_tx_dev_submodel_id(u8 *id)
{
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev || !id)
		return -1;

	*id = l_dev->acc_info.dev_submodel_id;
	hwlog_info("acc_get_tx_dev_submodel_id: 0x%x\n", *id);
	return 0;
}

static int qi_protocol_acc_get_tx_dev_version(u8 *ver)
{
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev || !ver)
		return -1;

	*ver = l_dev->acc_info.dev_version;
	hwlog_info("acc_get_tx_dev_version: 0x%x\n", *ver);
	return 0;
}

static int qi_protocol_acc_get_tx_dev_business(u8 *bus)
{
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev || !bus)
		return -1;

	*bus = l_dev->acc_info.dev_business;
	hwlog_info("acc_get_tx_dev_business: 0x%x\n", *bus);
	return 0;
}

static int qi_protocol_acc_get_tx_dev_info_cnt(u8 *cnt)
{
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev || !cnt)
		return -1;

	*cnt = l_dev->acc_info.dev_info_cnt;
	hwlog_info("acc_get_tx_dev_info_cnt: %d\n", *cnt);
	return 0;
}

static struct wireless_protocol_ops wireless_protocol_qi_ops = {
	.type_name = "hw_qi",
	.send_rx_vout = qi_protocol_send_rx_vout,
	.send_rx_iout = qi_protocol_send_rx_iout,
	.send_rx_boost_succ = qi_protocol_send_rx_boost_succ,
	.send_rx_ready = qi_protocol_send_rx_ready,
	.send_tx_capability = qi_protocol_send_tx_capability,
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
	.get_tx_bd_info = qi_protocol_get_tx_bd_info,
	.get_tx_id = qi_protocol_get_tx_id,
	.get_tx_type = qi_protocol_get_tx_type,
	.get_tx_adapter_type = qi_protocol_get_tx_adapter_type,
	.get_tx_capability = qi_protocol_get_tx_capability,
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
};

static void qi_protocol_notify_tx_event(unsigned long e, void *v)
{
	wireless_tx_event_notify(e, v);
}

static void qi_protocol_notify_rx_event(unsigned long e, void *v)
{
	wlc_rx_evt_notify(e, v);
}

/* 0x01 + signal_strength + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x01(u8 *data)
{
	hwlog_info("ask_packet_cmd_0x01: %d\n", data[QI_ASK_PACKET_DAT0]);
	return 0;
}

/* 0x18 + 0x05 + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x05(void)
{
	u8 data[QI_ACK_TX_FWVER_LEN - 1] = { 0 };
	int retry = WIRELESS_RETRY_ONE;

	qi_protocol_get_chip_fw_version(data, QI_ACK_TX_FWVER_LEN - 1, retry);
	qi_protocol_send_tx_fw_version(data, QI_ACK_TX_FWVER_LEN - 1);

	hwlog_info("ask_packet_cmd_0x05\n");
	return 0;
}

/* 0x38 + 0x0a + volt_lbyte + volt_hbyte + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x0a(u8 *data)
{
	int tx_vset;

	tx_vset = (data[QI_ASK_PACKET_DAT2] << QI_PROTOCOL_BYTE_BITS) |
		data[QI_ASK_PACKET_DAT1];
	qi_protocol_notify_tx_event(WLTX_EVT_ASK_SET_VTX, &tx_vset);

	hwlog_info("ask_packet_cmd_0x0a: tx_vset=0x%x\n", tx_vset);
	return 0;
}

/* 0x38 + 0x3b + id_hbyte + id_lbyte + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x3b(u8 *data)
{
	int tx_id;

	tx_id = (data[QI_ASK_PACKET_DAT1] << QI_PROTOCOL_BYTE_BITS) |
		data[QI_ASK_PACKET_DAT2];

	if (tx_id == QI_HANDSHAKE_ID_HW) {
		qi_protocol_send_tx_id(&data[QI_ASK_PACKET_DAT1],
			QI_PARA_TX_ID_LEN);
		hwlog_info("0x8866 handshake succ\n");
		qi_protocol_notify_tx_event(WL_TX_EVENT_HANDSHAKE_SUCC, NULL);
	}

	hwlog_info("ask_packet_cmd_0x3b: tx_id=0x%x\n", tx_id);
	return 0;
}

/* 0x18 + 0x41 + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x41(u8 *data)
{
	qi_protocol_notify_tx_event(WLTX_EVT_GET_TX_CAP, NULL);

	hwlog_info("ask_packet_cmd_0x41\n");
	return 0;
}

/* 0x28 + 0x43 + charger_state + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x43(u8 *data)
{
	int chrg_state;

	chrg_state = data[QI_ASK_PACKET_DAT1];
	qi_protocol_send_fsk_ack_msg();

	if (chrg_state & QI_CHRG_STATE_DONE) {
		hwlog_info("tx received rx charge-done event\n");
		qi_protocol_notify_tx_event(WL_TX_EVENT_CHARGEDONE, NULL);
	}

	hwlog_info("ask_packet_cmd_0x43: charger_state=0x%x\n", chrg_state);
	return 0;
}

/* 0x58 + 0x52 + mac1 + mac2 + mac3 + mac4 + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x52(u8 *data)
{
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev)
		return -1;

	qi_protocol_send_fsk_ack_msg();

	hwlog_info("ask_packet_cmd_0x52: %d\n", l_dev->acc_info.dev_info_cnt);
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
static int qi_protocol_handle_ask_packet_cmd_0x53(u8 *data)
{
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev)
		return -1;

	qi_protocol_send_fsk_ack_msg();

	hwlog_info("ask_packet_cmd_0x53: %d\n", l_dev->acc_info.dev_info_cnt);
	if (l_dev->acc_info.dev_info_cnt < QI_ASK_PACKET_DAT_LEN) {
		hwlog_info("cmd_0x53 cnt not right\n");
		return -1;
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
static int qi_protocol_handle_ask_packet_cmd_0x54(u8 *data)
{
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = qi_protocol_get_dev();
	if (!l_dev)
		return -1;

	qi_protocol_send_fsk_ack_msg();

	hwlog_info("ask_packet_cmd_0x54: %d\n", l_dev->acc_info.dev_info_cnt);
	if (l_dev->acc_info.dev_info_cnt < QI_ASK_PACKET_DAT_LEN * 2) {
		hwlog_info("cmd_0x54 cnt not right\n");
		return -1;
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
	l_dev->acc_info.dev_state = WL_ACC_DEV_STATE_ONLINE;
	qi_protocol_notify_tx_event(WL_TX_EVENT_ACC_DEV_CONNECTD, NULL);

	hwlog_info("get acc dev info succ\n");
	return 0;
}

static int qi_protocol_handle_ask_packet_hdr_0x18(u8 *data)
{
	u8 cmd = data[QI_ASK_PACKET_CMD];

	switch (cmd) {
	case QI_CMD_GET_TX_VERSION:
		return qi_protocol_handle_ask_packet_cmd_0x05();
	case QI_CMD_GET_TX_CAP:
		return qi_protocol_handle_ask_packet_cmd_0x41(data);
	default:
		hwlog_err("invalid hdr=0x18 cmd=0x%x\n", cmd);
		return -1;
	}
}

static int qi_protocol_handle_ask_packet_hdr_0x28(u8 *data)
{
	u8 cmd = data[QI_ASK_PACKET_CMD];

	switch (cmd) {
	case QI_CMD_SEND_CHRG_STATE:
		return qi_protocol_handle_ask_packet_cmd_0x43(data);
	default:
		hwlog_err("invalid hdr=0x28 cmd=0x%x\n", cmd);
		return -1;
	}
}

static int qi_protocol_handle_ask_packet_hdr_0x38(u8 *data)
{
	u8 cmd = data[QI_ASK_PACKET_CMD];

	switch (cmd) {
	case QI_CMD_SET_TX_VIN:
		return qi_protocol_handle_ask_packet_cmd_0x0a(data);
	case QI_CMD_GET_TX_ID:
		return qi_protocol_handle_ask_packet_cmd_0x3b(data);
	default:
		hwlog_err("invalid hdr=0x38 cmd=0x%x\n", cmd);
		return -1;
	}
}

static int qi_protocol_handle_ask_packet_hdr_0x48(u8 *data)
{
	u8 cmd = data[QI_ASK_PACKET_CMD];

	switch (cmd) {
	default:
		hwlog_err("invalid hdr=0x48 cmd=0x%x\n", cmd);
		return -1;
	}
}

static int qi_protocol_handle_ask_packet_hdr_0x58(u8 *data)
{
	u8 cmd = data[QI_ASK_PACKET_CMD];

	switch (cmd) {
	case QI_CMD_SEND_BT_MAC1:
		return qi_protocol_handle_ask_packet_cmd_0x52(data);
	case QI_CMD_SEND_BT_MAC2:
		return qi_protocol_handle_ask_packet_cmd_0x53(data);
	case QI_CMD_SEND_BT_MODEL_ID:
		return qi_protocol_handle_ask_packet_cmd_0x54(data);
	default:
		hwlog_err("invalid hdr=0x58 cmd=0x%x\n", cmd);
		return -1;
	}
}

static int qi_protocol_handle_ask_packet_data(u8 *data)
{
	u8 hdr = data[QI_ASK_PACKET_HDR];

	switch (hdr) {
	case QI_CMD_GET_SIGNAL_STRENGTH:
		return qi_protocol_handle_ask_packet_cmd_0x01(data);
	case QI_ASK_PACKET_HDR_MSG_SIZE_1_BYTE:
		return qi_protocol_handle_ask_packet_hdr_0x18(data);
	case QI_ASK_PACKET_HDR_MSG_SIZE_2_BYTE:
		return qi_protocol_handle_ask_packet_hdr_0x28(data);
	case QI_ASK_PACKET_HDR_MSG_SIZE_3_BYTE:
		return qi_protocol_handle_ask_packet_hdr_0x38(data);
	case QI_ASK_PACKET_HDR_MSG_SIZE_4_BYTE:
		return qi_protocol_handle_ask_packet_hdr_0x48(data);
	case QI_ASK_PACKET_HDR_MSG_SIZE_5_BYTE:
		return qi_protocol_handle_ask_packet_hdr_0x58(data);
	default:
		hwlog_err("invalid hdr=0x%x\n", hdr);
		return -1;
	}
}

/*
 * ask: rx to tx
 * we use ask mode when rx sends a message to tx
 */
static int qi_protocol_handle_ask_packet(void)
{
	u8 data[QI_ASK_PACKET_LEN] = { 0 };
	int retry = WIRELESS_RETRY_ONE;

	if (qi_protocol_get_ask_packet(data, QI_ASK_PACKET_LEN, retry))
		return -1;

	if (qi_protocol_handle_ask_packet_data(data))
		return -1;

	return 0;
}

/*
 * fsk: tx to rx
 * we use fsk mode when tx sends a message to rx
 */
static void qi_protocol_handle_fsk_packet(u8 *pkt, int len)
{
	int plim;

	if (!pkt || (len != QI_PKT_LEN)) {
		hwlog_err("handle_fsk_packet: para err\n");
		return;
	}

	switch (pkt[QI_PKT_CMD]) {
	case QI_CMD_TX_ALARM:
		hwlog_info("[handle_fsk_packet] received cmd: tx alarm\n");
		plim = pkt[QI_PKT_DATA + QI_TX_ALARM_PLIM];
		qi_protocol_notify_rx_event(WLC_EVT_PLIM_TX_ALARM, &plim);
		break;
	case QI_CMD_ACK_BST_ERR:
		hwlog_info("[handle_fsk_packet] received cmd: tx boost err\n");
		qi_protocol_notify_rx_event(WLC_EVT_PLIM_TX_BST_ERR, NULL);
		break;
	default:
		break;
	}
}

static struct qi_protocol_handle g_qi_protocol_handle = {
	.hdl_qi_ask_pkt = qi_protocol_handle_ask_packet,
	.hdl_non_qi_ask_pkt = qi_protocol_handle_ask_packet,
	.hdl_non_qi_fsk_pkt = qi_protocol_handle_fsk_packet,
};

static int __init qi_protocol_init(void)
{
	int ret;
	struct qi_protocol_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_qi_protocol_dev = l_dev;
	l_dev->dev_id = WIRELESS_DEVICE_ID_END;
	l_dev->acc_info.dev_no = ACC_DEV_NO_KB;

	ret = wireless_protocol_ops_register(&wireless_protocol_qi_ops);
	if (ret)
		goto fail_register_ops;

	return 0;

fail_register_ops:
	kfree(l_dev);
	g_qi_protocol_dev = NULL;

	return ret;
}

static void __exit qi_protocol_exit(void)
{
	if (!g_qi_protocol_dev)
		return;

	kfree(g_qi_protocol_dev);
	g_qi_protocol_dev = NULL;
}

subsys_initcall_sync(qi_protocol_init);
module_exit(qi_protocol_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("qi protocol driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
