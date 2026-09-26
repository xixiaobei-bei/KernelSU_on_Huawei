/*
 * mt5735_qi.c
 *
 * mt5735 qi_protocol driver; ask: rx->tx; fsk: tx->rx
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

#include "mt5735.h"

#define HWLOG_TAG wireless_mt5735_qi
HWLOG_REGIST();

static u8 mt5735_get_ask_header(int data_len)
{
	struct mt5735_dev_info *di = NULL;

	mt5735_get_dev_info(&di);
	if (!di || !di->g_val.qi_hdl || !di->g_val.qi_hdl->get_ask_hdr) {
		hwlog_err("get_ask_header: para null\n");
		return 0;
	}

	return di->g_val.qi_hdl->get_ask_hdr(data_len);
}

static u8 mt5735_get_fsk_header(int data_len)
{
	struct mt5735_dev_info *di = NULL;

	mt5735_get_dev_info(&di);
	if (!di || !di->g_val.qi_hdl || !di->g_val.qi_hdl->get_fsk_hdr) {
		hwlog_err("get_fsk_header: para null\n");
		return 0;
	}

	return di->g_val.qi_hdl->get_fsk_hdr(data_len);
}

static int mt5735_qi_send_ask_msg(u8 cmd, u8 *data, int data_len, void *dev_data)
{
	int ret;
	u8 header;
	u8 write_data[MT5735_SEND_MSG_DATA_LEN] = { 0 };
	struct mt5735_dev_info *di = NULL;

	mt5735_get_dev_info(&di);
	if (!di) {
		hwlog_err("send_ask_msg: para null\n");
		return -WLC_ERR_I2C_WR;
	}

	if ((data_len > MT5735_SEND_MSG_DATA_LEN) || (data_len < 0)) {
		hwlog_err("send_ask_msg: data number out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	di->irq_val &= ~MT5735_RX_IRQ_DATA_RCVD;
	/* msg_len=cmd_len+data_len  cmd_len=1 */
	header = mt5735_get_ask_header(data_len + 1);
	if (header <= 0) {
		hwlog_err("send_ask_msg: header wrong\n");
		return -WLC_ERR_PARA_WRONG;
	}
	ret = mt5735_write_byte(MT5735_SEND_MSG_HEADER_ADDR, header);
	if (ret) {
		hwlog_err("send_ask_msg: write header failed\n");
		return -WLC_ERR_I2C_W;
	}
	ret = mt5735_write_byte(MT5735_SEND_MSG_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("send_ask_msg: write cmd failed\n");
		return -WLC_ERR_I2C_W;
	}

	if (data && (data_len > 0)) {
		memcpy(write_data, data, data_len);
		ret = mt5735_write_block(MT5735_SEND_MSG_DATA_ADDR,
			write_data, data_len);
		if (ret) {
			hwlog_err("send_ask_msg: write rx2tx-reg failed\n");
			return -WLC_ERR_I2C_W;
		}
	}

	ret = mt5735_write_dword_mask(MT5735_RX_CMD_ADDR,
		MT5735_RX_CMD_SEND_MSG_RPLY, MT5735_RX_CMD_SEND_MSG_RPLY_SHIFT,
		MT5735_RX_CMD_VAL);
	if (ret) {
		hwlog_err("send_ask_msg: send rx msg to tx failed\n");
		return -WLC_ERR_I2C_W;
	}

	hwlog_info("send_ask_msg: succ, cmd=0x%x\n", cmd);
	return 0;
}

static int mt5735_qi_send_ask_msg_ack(u8 cmd, u8 *data, int data_len, void *dev_data)
{
	int ret;
	int i, j;
	struct mt5735_dev_info *di = NULL;

	mt5735_get_dev_info(&di);
	if (!di) {
		hwlog_err("send_ask_msg_ack: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	for (i = 0; i < MT5735_SNED_MSG_RETRY_CNT; i++) {
		ret = mt5735_qi_send_ask_msg(cmd, data, data_len, di);
		if (ret)
			continue;
		for (j = 0; j < MT5735_WAIT_FOR_ACK_RETRY_CNT; j++) {
			msleep(MT5735_WAIT_FOR_ACK_SLEEP_TIME);
			if (di->irq_val & MT5735_RX_IRQ_DATA_RCVD) {
				di->irq_val &= ~MT5735_RX_IRQ_DATA_RCVD;
				hwlog_info("[send_ask_msg_ack] succ\n");
				return 0;
			}
			if (di->g_val.rx_stop_chrg_flag)
				return -WLC_ERR_STOP_CHRG;
		}
		hwlog_info("[send_ask_msg_ack] retry, cnt=%d\n", i);
	}

	ret = mt5735_read_byte(MT5735_RCVD_MSG_CMD_ADDR, &cmd);
	if (ret) {
		hwlog_err("send_ask_msg_ack: get rcv cmd data failed\n");
		return -WLC_ERR_I2C_R;
	}
	if ((cmd != QI_CMD_ACK) && (cmd != QI_CMD_NACK)) {
		hwlog_err("[send_ask_msg_ack] failed, ack=0x%x, cnt=%d\n",
			cmd, i);
		return -WLC_ERR_ACK_TIMEOUT;
	}

	return 0;
}

static int mt5735_qi_receive_fsk_msg(u8 *data, int data_len, void *dev_data)
{
	int ret;
	int cnt = 0;
	struct mt5735_dev_info *di = NULL;

	mt5735_get_dev_info(&di);
	if (!di || !data) {
		hwlog_err("receive_msg: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	do {
		if (di->irq_val & MT5735_RX_IRQ_DATA_RCVD) {
			di->irq_val &= ~MT5735_RX_IRQ_DATA_RCVD;
			goto func_end;
		}
		if (di->g_val.rx_stop_chrg_flag)
			return -WLC_ERR_STOP_CHRG;
		msleep(MT5735_RCV_MSG_SLEEP_TIME);
		cnt++;
	} while (cnt < MT5735_RCV_MSG_SLEEP_CNT);

func_end:
	ret = mt5735_read_block(MT5735_RCVD_MSG_CMD_ADDR, data, data_len);
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

static int mt5735_qi_send_fsk_msg(u8 cmd, u8 *data, int data_len,
	void *dev_data)
{
	int ret;
	u8 header;
	u8 write_data[MT5735_SEND_MSG_DATA_LEN] = { 0 };

	if ((data_len > MT5735_SEND_MSG_DATA_LEN) || (data_len < 0)) {
		hwlog_err("send_fsk_msg: data number out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	if (cmd == QI_CMD_ACK)
		header = QI_CMD_ACK_HEAD;
	else
		header = mt5735_get_fsk_header(data_len + 1);
	if (header <= 0) {
		hwlog_err("send_fsk_msg: header wrong\n");
		return -WLC_ERR_PARA_WRONG;
	}
	ret = mt5735_write_byte(MT5735_TX_SEND_MSG_HEADER_ADDR, header);
	if (ret) {
		hwlog_err("send_fsk_msg: write header failed\n");
		return ret;
	}
	ret = mt5735_write_byte(MT5735_TX_SEND_MSG_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("send_fsk_msg: write cmd failed\n");
		return ret;
	}

	if (data && (data_len > 0)) {
		memcpy(write_data, data, data_len);
		ret = mt5735_write_block(MT5735_TX_SEND_MSG_DATA_ADDR,
			write_data, data_len);
		if (ret) {
			hwlog_err("send_fsk_msg: write fsk reg failed\n");
			return ret;
		}
	}

	ret = mt5735_write_dword_mask(MT5735_TX_CMD_ADDR, MT5735_TX_CMD_SEND_MSG,
		MT5735_TX_CMD_SEND_MSG_SHIFT, MT5735_TX_CMD_VAL);
	if (ret) {
		hwlog_err("send_fsk_msg: send fsk failed\n");
		return ret;
	}

	hwlog_info("[send_fsk_msg] succ\n");
	return 0;
}

static int mt5735_qi_send_fsk_with_ack(u8 cmd, u8 *data, int data_len,
	void *dev_data)
{
	int i;
	int ret;
	struct mt5735_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("send_fsk_with_ack: para null\n");
		return -ENODEV;
	}

	di->irq_val &= ~MT5735_TX_IRQ_FSK_ACK;
	ret = mt5735_qi_send_fsk_msg(cmd, data, data_len, dev_data);
	if (ret)
		return ret;

	for (i = 0; i < MT5735_WAIT_FOR_ACK_RETRY_CNT; i++) {
		msleep(MT5735_WAIT_FOR_ACK_SLEEP_TIME);
		if (di->irq_val & MT5735_TX_IRQ_FSK_ACK) {
			di->irq_val &= ~MT5735_TX_IRQ_FSK_ACK;
			hwlog_info("[send_fsk_with_ack] succ\n");
			return 0;
		}
		if (di->g_val.tx_stop_chrg_flag)
			return -EPERM;
	}

	hwlog_err("send_fsk_with_ack: failed\n");
	return -EIO;
}

static int mt5735_qi_receive_ask_pkt(u8 *pkt_data, int pkt_data_len,
	void *dev_data)
{
	int ret;
	int i;
	char buff[MT5735_RCVD_PKT_BUFF_LEN] = { 0 };
	char pkt_str[MT5735_RCVD_PKT_STR_LEN] = { 0 };

	if (!pkt_data || (pkt_data_len <= 0) ||
		(pkt_data_len > MT5735_RCVD_MSG_PKT_LEN)) {
		hwlog_err("get_ask_pkt: para err\n");
		return -1;
	}
	ret = mt5735_read_block(MT5735_TX_RCVD_MSG_HEADER_ADDR,
		pkt_data, pkt_data_len);
	if (ret) {
		hwlog_err("get_ask_pkt: read failed\n");
		return -1;
	}
	for (i = 0; i < pkt_data_len; i++) {
		snprintf(buff, MT5735_RCVD_PKT_BUFF_LEN, "0x%02x ", pkt_data[i]);
		strncat(pkt_str, buff, strlen(buff));
	}
	hwlog_info("[get_ask_pkt] %s\n", pkt_str);
	return 0;
}

static int mt5735_qi_set_rx_rpp_format(u8 pmax)
{
	int ret;

	ret = mt5735_write_byte(MT5735_RX_RP_PMAX_ADDR,
		pmax * MT5735_RX_RP_VAL_UNIT);
	ret += mt5735_write_dword_mask(MT5735_RX_CMD_ADDR,
		MT5735_RX_CMD_SET_24BIT_RPP, MT5735_RX_CMD_SET_24BIT_RPP_SHIFT,
		MT5735_RX_CMD_VAL);
	if (ret)
		hwlog_err("set_rx_rpp_format: failed\n");

	return ret;
}

static int mt5735_qi_set_tx_rpp_format(u8 pmax)
{
	int ret;

	ret = mt5735_write_byte(MT5735_TX_RP_PMAX_ADDR,
		pmax * MT5735_TX_RP_VAL_UNIT);
	ret += mt5735_write_dword_mask(MT5735_TX_CMD_ADDR,
		MT5735_TX_CMD_SET_24BIT_RPP, MT5735_TX_CMD_SET_24BIT_RPP_SHIFT,
		MT5735_TX_CMD_VAL);
	if (ret)
		hwlog_err("set_tx_rpp_format: failed\n");

	return ret;
}

static int mt5735_qi_set_rpp_format(u8 pmax, int mode, void *dev_data)
{
	if (mode == WIRELESS_RX)
		return mt5735_qi_set_rx_rpp_format(pmax);

	return mt5735_qi_set_tx_rpp_format(pmax);
}

static struct qi_protocol_ops g_mt5735_qi_ops = {
	.chip_name              = "mt5735",
	.send_msg               = mt5735_qi_send_ask_msg,
	.send_msg_with_ack      = mt5735_qi_send_ask_msg_ack,
	.receive_msg            = mt5735_qi_receive_fsk_msg,
	.send_fsk_msg           = mt5735_qi_send_fsk_msg,
	.auto_send_fsk_with_ack = mt5735_qi_send_fsk_with_ack,
	.get_ask_packet         = mt5735_qi_receive_ask_pkt,
	.get_chip_fw_version    = mt5735_get_chip_fw_version,
	.get_tx_id_pre          = NULL,
	.set_rpp_format_post    = mt5735_qi_set_rpp_format,
};

int mt5735_qi_ops_register(struct mt5735_dev_info *di)
{
	if (!di)
		return -ENODEV;

	g_mt5735_qi_ops.dev_data = (void *)di;
	return qi_protocol_ops_register(&g_mt5735_qi_ops, di->ic_type);
}
