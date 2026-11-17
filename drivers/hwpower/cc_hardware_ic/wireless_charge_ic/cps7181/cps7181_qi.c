/*
 * cps7181_qi.c
 *
 * cps7181 qi_protocol driver; ask: rx->tx; fsk: tx->rx
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

#include "cps7181.h"

#define HWLOG_TAG wireless_cps7181_qi
HWLOG_REGIST();

static u8 cps7181_get_ask_header(struct cps7181_dev_info *di, int data_len)
{
	if (!di || !di->g_val.qi_hdl || !di->g_val.qi_hdl->get_ask_hdr) {
		hwlog_err("get_ask_header: para null\n");
		return 0;
	}

	return di->g_val.qi_hdl->get_ask_hdr(data_len);
}

static u8 cps7181_get_fsk_header(struct cps7181_dev_info *di, int data_len)
{
	if (!di || !di->g_val.qi_hdl || !di->g_val.qi_hdl->get_fsk_hdr) {
		hwlog_err("get_fsk_header: para null\n");
		return 0;
	}

	return di->g_val.qi_hdl->get_fsk_hdr(data_len);
}

static int cps7181_qi_send_ask_msg(u8 cmd, u8 *data, int data_len,
	void *dev_data)
{
	int ret;
	u8 header;
	u8 write_data[CPS7181_SEND_MSG_DATA_LEN] = { 0 };
	struct cps7181_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("send_ask_msg: para null\n");
		return -WLC_ERR_I2C_WR;
	}

	if ((data_len > CPS7181_SEND_MSG_DATA_LEN) || (data_len < 0)) {
		hwlog_err("send_ask_msg: data number out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	di->irq_val &= ~CPS7181_RX_IRQ_FSK_PKT;
	di->irq_val &= ~CPS7181_RX_IRQ_FSK_ACK;
	/* msg_len=cmd_len+data_len cmd_len=1 */
	header = cps7181_get_ask_header(di, data_len + 1);
	if (header <= 0) {
		hwlog_err("send_ask_msg: header wrong\n");
		return -WLC_ERR_PARA_WRONG;
	}
	ret = cps7181_write_byte(di, CPS7181_SEND_MSG_HEADER_ADDR, header);
	if (ret) {
		hwlog_err("send_ask_msg: write header failed\n");
		return -WLC_ERR_I2C_W;
	}
	ret = cps7181_write_byte(di, CPS7181_SEND_MSG_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("send_ask_msg: write cmd failed\n");
		return -WLC_ERR_I2C_W;
	}

	if (data && (data_len > 0)) {
		memcpy(write_data, data, data_len);
		ret = cps7181_write_block(di, CPS7181_SEND_MSG_DATA_ADDR,
			write_data, data_len);
		if (ret) {
			hwlog_err("send_ask_msg: write rx2tx-reg failed\n");
			return -WLC_ERR_I2C_W;
		}
	}

	ret = cps7181_write_byte_mask(di, CPS7181_RX_CMD_ADDR,
		CPS7181_RX_CMD_SEND_MSG_RPLY,
		CPS7181_RX_CMD_SEND_MSG_RPLY_SHIFT, CPS7181_RX_CMD_VAL);
	if (ret) {
		hwlog_err("send_ask_msg: send rx msg to tx failed\n");
		return -WLC_ERR_I2C_W;
	}

	hwlog_info("[send_ask_msg] succ, cmd=0x%x\n", cmd);
	return 0;
}

static int cps7181_qi_send_ask_msg_ack(u8 cmd, u8 *data, int data_len,
	void *dev_data)
{
	int ret;
	int i, j;
	struct cps7181_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("send_ask_msg_ack: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	for (i = 0; i < CPS7181_SEND_MSG_RETRY_CNT; i++) {
		ret = cps7181_qi_send_ask_msg(cmd, data, data_len, di);
		if (ret)
			continue;
		for (j = 0; j < CPS7181_WAIT_FOR_ACK_RETRY_CNT; j++) {
			(void)power_msleep(CPS7181_WAIT_FOR_ACK_SLEEP_TIME, 0, NULL);
			if (di->irq_val & CPS7181_RX_IRQ_FSK_ACK) {
				di->irq_val &= ~CPS7181_RX_IRQ_FSK_ACK;
				hwlog_info("[send_ask_msg_ack] succ\n");
				return 0;
			}
			if (di->g_val.rx_stop_chrg_flag)
				return -WLC_ERR_STOP_CHRG;
		}
		hwlog_info("[send_ask_msg_ack] retry, cnt=%d\n", i);
	}

	ret = cps7181_read_byte(di, CPS7181_RCVD_MSG_CMD_ADDR, &cmd);
	if (ret) {
		hwlog_err("send_ask_msg_ack: get rcv cmd data failed\n");
		return -WLC_ERR_I2C_R;
	}
	if (cmd != QI_CMD_ACK) {
		hwlog_err("send_ask_msg_ack: failed, ack=0x%x, cnt=%d\n",
			cmd, i);
		return -WLC_ERR_ACK_TIMEOUT;
	}

	return 0;
}

static int cps7181_qi_receive_fsk_msg(u8 *data, int data_len, void *dev_data)
{
	int ret;
	int cnt = 0;
	struct cps7181_dev_info *di = dev_data;

	if (!di || !data) {
		hwlog_err("receive_msg: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	do {
		if (di->irq_val & CPS7181_RX_IRQ_FSK_PKT) {
			di->irq_val &= ~CPS7181_RX_IRQ_FSK_PKT;
			goto func_end;
		}
		if (di->g_val.rx_stop_chrg_flag)
			return -WLC_ERR_STOP_CHRG;
		(void)power_msleep(CPS7181_RCV_MSG_SLEEP_TIME, 0, NULL);
		cnt++;
	} while (cnt < CPS7181_RCV_MSG_SLEEP_CNT);

func_end:
	ret = cps7181_read_block(di, CPS7181_RCVD_MSG_CMD_ADDR, data, data_len);
	if (ret) {
		hwlog_err("receive_msg: get tx2rx data failed\n");
		return -WLC_ERR_I2C_R;
	}
	if (!data[0]) { /* data[0]: cmd */
		hwlog_err("receive_msg: no msg received from tx\n");
		return -WLC_ERR_ACK_TIMEOUT;
	}
	hwlog_info("[receive_msg] get tx2rx data(cmd:0x%x) succ\n", data[0]);
	return 0;
}

static int cps7181_qi_send_fsk_msg(u8 cmd, u8 *data, int data_len,
	void *dev_data)
{
	int ret;
	u8 header;
	u8 write_data[CPS7181_SEND_MSG_DATA_LEN] = { 0 };
	struct cps7181_dev_info *di = dev_data;

	if (!data && (cmd != QI_CMD_ACK)) {
		hwlog_err("send_fsk_msg: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	if ((data_len > CPS7181_SEND_MSG_DATA_LEN) || (data_len < 0)) {
		hwlog_err("send_fsk_msg: data number out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	if (cmd == QI_CMD_ACK)
		header = QI_CMD_ACK_HEAD;
	else
		header = cps7181_get_fsk_header(di, data_len + 1);
	if (header <= 0) {
		hwlog_err("send_fsk_msg: header wrong\n");
		return -WLC_ERR_PARA_WRONG;
	}
	ret = cps7181_write_byte(di, CPS7181_SEND_MSG_HEADER_ADDR, header);
	if (ret) {
		hwlog_err("send_fsk_msg: write header failed\n");
		return ret;
	}
	ret = cps7181_write_byte(di, CPS7181_SEND_MSG_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("send_fsk_msg: write cmd failed\n");
		return ret;
	}

	if (data && (data_len > 0)) {
		memcpy(write_data, data, data_len);
		ret = cps7181_write_block(di, CPS7181_SEND_MSG_DATA_ADDR,
			write_data, data_len);
		if (ret) {
			hwlog_err("send_fsk_msg: write fsk reg failed\n");
			return ret;
		}
	}
	ret = cps7181_write_byte_mask(di, CPS7181_TX_CMD_ADDR,
		CPS7181_TX_CMD_SEND_MSG, CPS7181_TX_CMD_SEND_MSG_SHIFT,
		CPS7181_TX_CMD_VAL);
	if (ret) {
		hwlog_err("send_fsk_msg: send fsk failed\n");
		return ret;
	}

	hwlog_info("[send_fsk_msg] succ\n");
	return 0;
}

static int cps7181_qi_receive_ask_pkt(u8 *pkt_data, int pkt_data_len,
	void *dev_data)
{
	int ret;
	int i;
	char buff[CPS7181_RCVD_PKT_BUFF_LEN] = { 0 };
	char pkt_str[CPS7181_RCVD_PKT_STR_LEN] = { 0 };
	struct cps7181_dev_info *di = dev_data;

	if (!pkt_data || (pkt_data_len <= 0) ||
		(pkt_data_len > CPS7181_RCVD_MSG_PKT_LEN)) {
		hwlog_err("get_ask_pkt: para err\n");
		return -1;
	}
	ret = cps7181_read_block(di, CPS7181_RCVD_MSG_HEADER_ADDR,
		pkt_data, pkt_data_len);
	if (ret) {
		hwlog_err("get_ask_pkt: read failed\n");
		return -1;
	}
	for (i = 0; i < pkt_data_len; i++) {
		snprintf(buff, CPS7181_RCVD_PKT_BUFF_LEN,
			"0x%02x ", pkt_data[i]);
		strncat(pkt_str, buff, strlen(buff));
	}

	hwlog_info("[get_ask_pkt] RX back packet: %s\n", pkt_str);
	return 0;
}

static int cps7181_qi_set_rpp_format(u8 pmax, int mode, void *dev_data)
{
	int ret;
	struct cps7181_dev_info *di = dev_data;

	ret = cps7181_write_byte(di, CPS7181_RX_RP_PMAX_ADDR,
		pmax * CPS7181_RX_RP_VAL_UNIT);
	ret += cps7181_write_byte(di, CPS7181_RX_RP_TYPE_ADDR,
		CPS7181_RX_RP_NO_REPLY);
	ret += cps7181_write_byte_mask(di, CPS7181_RX_FUNC_EN_ADDR,
		CPS7181_RX_RP24BIT_EN_MASK, CPS7181_RX_RP24BIT_EN_SHIFT,
		CPS7181_RX_FUNC_EN);
	if (ret) {
		hwlog_err("set_rpp_format: failed\n");
		return ret;
	}

	return 0;
}

int cps7181_qi_get_fw_version(u8 *data, int len, void *dev_data)
{
	struct cps7181_chip_info chip_info;
	struct cps7181_dev_info *di = dev_data;

	/* fw version length must be 4 */
	if (!data || (len != 4)) {
		hwlog_err("get_fw_version: para err");
		return -WLC_ERR_PARA_WRONG;
	}

	if (cps7181_get_chip_info(di, &chip_info)) {
		hwlog_err("get_fw_version: get chip_info failed\n");
		return -WLC_ERR_I2C_R;
	}

	/* byte[0:1]=chip_id, byte[2:3]=mtp_ver */
	data[0] = (u8)((chip_info.chip_id >> 0) & BYTE_MASK);
	data[1] = (u8)((chip_info.chip_id >> BITS_PER_BYTE) & BYTE_MASK);
	data[2] = (u8)((chip_info.mtp_ver >> 0) & BYTE_MASK);
	data[3] = (u8)((chip_info.mtp_ver >> BITS_PER_BYTE) & BYTE_MASK);

	return 0;
}

static struct qi_protocol_ops g_cps7181_qi_ops = {
	.chip_name           = "cps7181",
	.send_msg            = cps7181_qi_send_ask_msg,
	.send_msg_with_ack   = cps7181_qi_send_ask_msg_ack,
	.receive_msg         = cps7181_qi_receive_fsk_msg,
	.send_fsk_msg        = cps7181_qi_send_fsk_msg,
	.get_ask_packet      = cps7181_qi_receive_ask_pkt,
	.get_chip_fw_version = cps7181_qi_get_fw_version,
	.get_tx_id_pre       = NULL,
	.set_rpp_format_post = cps7181_qi_set_rpp_format,
};

int cps7181_qi_ops_register(struct wltrx_ic_ops *ops, struct cps7181_dev_info *di)
{
	if (!ops || !di)
		return -ENODEV;

	ops->qi_ops = kzalloc(sizeof(*(ops->qi_ops)), GFP_KERNEL);
	if (!ops->qi_ops)
		return -ENODEV;

	memcpy(ops->qi_ops, &g_cps7181_qi_ops, sizeof(g_cps7181_qi_ops));
	ops->qi_ops->dev_data = (void *)di;
	return qi_protocol_ops_register(ops->qi_ops, di->ic_type);
}
