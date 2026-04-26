/*
 * cps4029_qi.c
 *
 * cps4029 qi_protocol driver; ask: rx->tx; fsk: tx->rx
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include "cps4029.h"

#define HWLOG_TAG wireless_cps4029_qi
HWLOG_REGIST();

static u8 cps4029_get_ask_header(struct cps4029_dev_info *di, int data_len)
{
	if (!di || !di->g_val.qi_hdl || !di->g_val.qi_hdl->get_ask_hdr) {
		hwlog_err("get_ask_header: para null\n");
		return 0;
	}

	return di->g_val.qi_hdl->get_ask_hdr(data_len);
}

static u8 cps4029_get_fsk_header(struct cps4029_dev_info *di, int data_len)
{
	if (!di || !di->g_val.qi_hdl || !di->g_val.qi_hdl->get_fsk_hdr) {
		hwlog_err("get_fsk_header: para null\n");
		return 0;
	}

	return di->g_val.qi_hdl->get_fsk_hdr(data_len);
}

static int cps4029_qi_send_ask_msg(u8 cmd, u8 *data, int data_len,
	void *dev_data)
{
	int ret;
	u8 header;
	u8 write_data[CPS4029_SEND_MSG_DATA_LEN] = { 0 };
	struct cps4029_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("send_ask_msg: para null\n");
		return -ENODEV;
	}

	if ((data_len > CPS4029_SEND_MSG_DATA_LEN) || (data_len < 0)) {
		hwlog_err("send_ask_msg: data number out of range\n");
		return -EINVAL;
	}

	di->irq_val &= ~CPS4029_RX_IRQ_FSK_PKT;
	di->irq_val &= ~CPS4029_RX_IRQ_FSK_ACK;
	/* msg_len=cmd_len+data_len cmd_len=1 */
	header = cps4029_get_ask_header(di, data_len + 1);
	if (header <= 0) {
		hwlog_err("send_ask_msg: header wrong\n");
		return -EINVAL;
	}
	ret = cps4029_write_byte(di, CPS4029_SEND_MSG_HEADER_ADDR, header);
	if (ret) {
		hwlog_err("send_ask_msg: write header failed\n");
		return ret;
	}
	ret = cps4029_write_byte(di, CPS4029_SEND_MSG_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("send_ask_msg: write cmd failed\n");
		return ret;
	}

	if (data && (data_len > 0)) {
		memcpy(write_data, data, data_len);
		ret = cps4029_write_block(di, CPS4029_SEND_MSG_DATA_ADDR,
			write_data, data_len);
		if (ret) {
			hwlog_err("send_ask_msg: write rx2tx-reg failed\n");
			return ret;
		}
	}

	ret = cps4029_write_byte_mask(di, CPS4029_RX_CMD_ADDR,
		CPS4029_RX_CMD_SEND_MSG_RPLY,
		CPS4029_RX_CMD_SEND_MSG_RPLY_SHIFT, CPS4029_RX_CMD_VAL);
	if (ret) {
		hwlog_err("send_ask_msg: send rx msg to tx failed\n");
		return ret;
	}

	hwlog_info("[send_ask_msg] succ, cmd=0x%x\n", cmd);
	return 0;
}

static int cps4029_qi_send_ask_msg_ack(u8 cmd, u8 *data, int data_len,
	void *dev_data)
{
	int ret;
	int i, j;
	struct cps4029_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("send_ask_msg_ack: para null\n");
		return -ENODEV;
	}

	for (i = 0; i < CPS4029_SEND_MSG_RETRY_CNT; i++) {
		ret = cps4029_qi_send_ask_msg(cmd, data, data_len, di);
		if (ret)
			continue;
		for (j = 0; j < CPS4029_WAIT_FOR_ACK_RETRY_CNT; j++) {
			(void)power_msleep(CPS4029_WAIT_FOR_ACK_SLEEP_TIME, 0, NULL);
			if (di->irq_val & CPS4029_RX_IRQ_FSK_ACK) {
				di->irq_val &= ~CPS4029_RX_IRQ_FSK_ACK;
				hwlog_info("[send_ask_msg_ack] succ\n");
				return 0;
			}
		}
		hwlog_info("[send_ask_msg_ack] retry, cnt=%d\n", i);
	}

	ret = cps4029_read_byte(di, CPS4029_RCVD_MSG_CMD_ADDR, &cmd);
	if (ret) {
		hwlog_err("send_ask_msg_ack: get rcv cmd data failed\n");
		return ret;
	}
	if ((cmd != QI_CMD_ACK) && (cmd != QI_CMD_NACK)) {
		hwlog_err("send_ask_msg_ack: failed, ack=0x%x, cnt=%d\n",
			cmd, i);
		return -EINVAL;
	}

	return 0;
}

static int cps4029_qi_receive_fsk_msg(u8 *data, int data_len, void *dev_data)
{
	int ret;
	int cnt = 0;
	struct cps4029_dev_info *di = dev_data;

	if (!di || !data) {
		hwlog_err("receive_msg: para null\n");
		return -EINVAL;
	}

	do {
		if (di->irq_val & CPS4029_RX_IRQ_FSK_PKT) {
			di->irq_val &= ~CPS4029_RX_IRQ_FSK_PKT;
			goto func_end;
		}
		(void)power_msleep(CPS4029_RCV_MSG_SLEEP_TIME, 0, NULL);
		cnt++;
	} while (cnt < CPS4029_RCV_MSG_SLEEP_CNT);

func_end:
	ret = cps4029_read_block(di, CPS4029_RCVD_MSG_CMD_ADDR, data, data_len);
	if (ret) {
		hwlog_err("receive_msg: get tx2rx data failed\n");
		return ret;
	}
	if (!data[0]) { /* data[0]: cmd */
		hwlog_err("receive_msg: no msg received from tx\n");
		return -EIO;
	}

	hwlog_info("[receive_msg] get tx2rx data(cmd:0x%x) succ\n", data[0]);
	return 0;
}

static int cps4029_qi_send_fsk_msg(u8 cmd, u8 *data, int data_len, void *dev_data)
{
	int ret;
	u8 header;
	u8 write_data[CPS4029_SEND_MSG_DATA_LEN] = { 0 };
	struct cps4029_dev_info *di = dev_data;

	if ((data_len > CPS4029_SEND_MSG_DATA_LEN) || (data_len < 0)) {
		hwlog_err("send_fsk_msg: data number out of range\n");
		return -EINVAL;
	}

	if (cmd == QI_CMD_ACK)
		header = QI_CMD_ACK_HEAD;
	else
		header = cps4029_get_fsk_header(di, data_len + 1);
	if (header <= 0) {
		hwlog_err("send_fsk_msg: header wrong\n");
		return -EINVAL;
	}
	ret = cps4029_write_byte(di, CPS4029_SEND_MSG_HEADER_ADDR, header);
	if (ret) {
		hwlog_err("send_fsk_msg: write header failed\n");
		return ret;
	}
	ret = cps4029_write_byte(di, CPS4029_SEND_MSG_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("send_fsk_msg: write cmd failed\n");
		return ret;
	}

	if (data && (data_len > 0)) {
		memcpy(write_data, data, data_len);
		ret = cps4029_write_block(di, CPS4029_SEND_MSG_DATA_ADDR,
			write_data, data_len);
		if (ret) {
			hwlog_err("send_fsk_msg: write fsk reg failed\n");
			return ret;
		}
	}
	ret = cps4029_write_byte_mask(di, CPS4029_TX_CMD_ADDR,
		CPS4029_TX_CMD_SEND_MSG, CPS4029_TX_CMD_SEND_MSG_SHIFT,
		CPS4029_TX_CMD_VAL);
	if (ret) {
		hwlog_err("send_fsk_msg: send fsk failed\n");
		return ret;
	}

	hwlog_info("[send_fsk_msg] succ\n");
	return 0;
}

static int cps4029_qi_send_fsk_with_ack(u8 cmd, u8 *data, int data_len,
	void *dev_data)
{
	int i;
	int ret;
	struct cps4029_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("send_fsk_with_ack: para null\n");
		return -ENODEV;
	}

	di->irq_val &= ~CPS4029_TX_IRQ_FSK_ACK;
	ret = cps4029_qi_send_fsk_msg(cmd, data, data_len, di);
	if (ret)
		return ret;

	for (i = 0; i < CPS4029_WAIT_FOR_ACK_RETRY_CNT; i++) {
		(void)power_msleep(CPS4029_WAIT_FOR_ACK_SLEEP_TIME, 0, NULL);
		if (di->irq_val & CPS4029_TX_IRQ_FSK_ACK) {
			di->irq_val &= ~CPS4029_TX_IRQ_FSK_ACK;
			hwlog_info("[send_fsk_with_ack] succ\n");
			return 0;
		}
		if (di->g_val.tx_stop_chrg_flag)
			return -EINVAL;
	}

	hwlog_err("send_fsk_with_ack: failed\n");
	return -EINVAL;
}

static int cps4029_qi_receive_ask_pkt(u8 *pkt_data, int pkt_data_len, void *dev_data)
{
	int ret;
	int i;
	char buff[CPS4029_RCVD_PKT_BUFF_LEN] = { 0 };
	char pkt_str[CPS4029_RCVD_PKT_STR_LEN] = { 0 };
	struct cps4029_dev_info *di = dev_data;

	if (!pkt_data || (pkt_data_len <= 0) ||
		(pkt_data_len > CPS4029_RCVD_MSG_PKT_LEN)) {
		hwlog_err("get_ask_pkt: para err\n");
		return -EINVAL;
	}
	ret = cps4029_read_block(di, CPS4029_RCVD_MSG_HEADER_ADDR,
		pkt_data, pkt_data_len);
	if (ret) {
		hwlog_err("get_ask_pkt: read failed\n");
		return ret;
	}
	for (i = 0; i < pkt_data_len; i++) {
		snprintf(buff, CPS4029_RCVD_PKT_BUFF_LEN,
			"0x%02x ", pkt_data[i]);
		strncat(pkt_str, buff, strlen(buff));
	}

	hwlog_info("[get_ask_pkt] RX back packet: %s\n", pkt_str);
	return 0;
}

static int cps4029_qi_set_rpp_format(u8 pmax, int mode, void *dev_data)
{
	return 0;
}

int cps4029_qi_get_fw_version(u8 *data, int len, void *dev_data)
{
	struct cps4029_chip_info chip_info;
	struct cps4029_dev_info *di = dev_data;

	/* fw version length must be 4 */
	if (!data || (len != 4)) {
		hwlog_err("get_fw_version: para err");
		return -EINVAL;
	}

	if (cps4029_get_chip_info(di, &chip_info)) {
		hwlog_err("get_fw_version: get chip_info failed\n");
		return -EINVAL;
	}

	/* byte[0:1]=chip_id, byte[2:3]=mtp_ver */
	data[0] = (u8)((chip_info.chip_id >> 0) & BYTE_MASK);
	data[1] = (u8)((chip_info.chip_id >> BITS_PER_BYTE) & BYTE_MASK);
	data[2] = (u8)((chip_info.mtp_ver >> 0) & BYTE_MASK);
	data[3] = (u8)((chip_info.mtp_ver >> BITS_PER_BYTE) & BYTE_MASK);

	return 0;
}

static struct qi_protocol_ops g_cps4029_qi_ops = {
	.chip_name              = "cps4029",
	.send_msg               = cps4029_qi_send_ask_msg,
	.send_msg_with_ack      = cps4029_qi_send_ask_msg_ack,
	.receive_msg            = cps4029_qi_receive_fsk_msg,
	.send_fsk_msg           = cps4029_qi_send_fsk_msg,
	.auto_send_fsk_with_ack = cps4029_qi_send_fsk_with_ack,
	.get_ask_packet         = cps4029_qi_receive_ask_pkt,
	.get_chip_fw_version    = cps4029_qi_get_fw_version,
	.get_tx_id_pre          = NULL,
	.set_rpp_format_post    = cps4029_qi_set_rpp_format,
};

int cps4029_qi_ops_register(struct wltrx_ic_ops *ops, struct cps4029_dev_info *di)
{
	if (!ops || !di)
		return -ENODEV;

	ops->qi_ops = kzalloc(sizeof(*(ops->qi_ops)), GFP_KERNEL);
	if (!ops->qi_ops)
		return -ENOMEM;

	memcpy(ops->qi_ops, &g_cps4029_qi_ops, sizeof(g_cps4029_qi_ops));
	ops->qi_ops->dev_data = (void *)di;

	return qi_protocol_ops_register(ops->qi_ops, di->ic_type);
}
