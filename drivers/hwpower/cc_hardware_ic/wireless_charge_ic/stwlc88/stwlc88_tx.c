/*
 * stwlc88_tx.c
 *
 * stwlc88 tx driver
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

#include "stwlc88.h"

#define HWLOG_TAG wireless_stwlc88_tx
HWLOG_REGIST();

static bool g_st88_tx_open_flag;

static const char * const g_stwlc88_tx_irq_name[] = {
	/* [n]: n means bit in registers */
	[0]  = "tx_otp",
	[1]  = "tx_ocp",
	[2]  = "tx_ovp",
	[3]  = "tx_sys_err",
	[4]  = "tx_rpp_rcvd",
	[5]  = "tx_cep_rcvd",
	[6]  = "tx_send_pkt_succ",
	[7]  = "tx_dping_rcvd",
	[8]  = "tx_cep_timeout",
	[9]  = "tx_rpp_timeout",
	[10] = "tx_ept",
	[11] = "tx_start_ping",
	[12] = "tx_ss_pkt_rcvd",
	[13] = "tx_id_pkt_rcvd",
	[14] = "tx_cfg_pkt_rcvd",
	[15] = "tx_pp_pkt_rcvd",
	[19] = "tx_fod_det",
	[24] = "bridge_md",
	[25] = "tx_ack_rcvd"
};

static const char * const g_stwlc88_tx_ept_name[] = {
	/* [n]: n means bit in registers */
	[0]  = "tx_ept_src_ovp",
	[1]  = "tx_ept_src_ocp",
	[2]  = "tx_ept_src_otp",
	[3]  = "tx_ept_src_fod",
	[4]  = "tx_ept_src_cmd",
	[5]  = "tx_ept_src_rx",
	[6]  = "tx_ept_src_cep_timeout",
	[7]  = "tx_ept_src_rpp_timeout",
	[8]  = "tx_ept_src_rx_rst",
	[9]  = "tx_ept_src_sys_err",
	[10] = "tx_ept_src_ss_timeout",
	[11] = "tx_ept_src_ss",
	[12] = "tx_ept_src_id",
	[13] = "tx_ept_src_cfg",
	[14] = "tx_ept_src_cfg_cnt",
	[15] = "tx_ept_src_pch",
	[16] = "tx_ept_src_xid",
	[17] = "tx_ept_src_nego",
	[18] = "tx_ept_src_nego_timeout",
};

static int stwlc88_sw2tx(void)
{
	int ret;
	int i;
	u8 mode = 0;
	int cnt = ST88_SW2TX_RETRY_TIME / ST88_SW2TX_SLEEP_TIME;

	for (i = 0; i < cnt; i++) {
		if (!g_st88_tx_open_flag) {
			hwlog_err("sw2tx: tx_open_flag false\n");
			return -WLC_ERR_STOP_CHRG;
		}
		msleep(ST88_SW2TX_SLEEP_TIME);
		ret = stwlc88_get_mode(&mode);
		if (ret) {
			hwlog_err("sw2tx: get mode failed\n");
			continue;
		}
		if (mode == ST88_OP_MODE_TX) {
			hwlog_info("sw2tx: succ, cnt=%d\n", i);
			msleep(ST88_SW2TX_SLEEP_TIME);
			return 0;
		}
		ret = stwlc88_write_word_mask(ST88_SYS_CMD_ADDR,
			ST88_SYS_CMD_SW2TX, ST88_SYS_CMD_SW2TX_SHIFT,
			ST88_SYS_CMD_VAL);
		if (ret)
			hwlog_err("sw2tx: write cmd(sw2tx) failed\n");
	}
	hwlog_err("sw2tx: failed, cnt=%d\n", i);
	return -WLC_ERR_I2C_WR;
}

static bool stwlc88_tx_is_tx_mode(void *dev_data)
{
	int ret;
	u8 mode = 0;

	ret = stwlc88_read_byte(ST88_OP_MODE_ADDR, &mode);
	if (ret) {
		hwlog_err("is_tx_mode: get op_mode failed\n");
		return false;
	}

	if (mode == ST88_OP_MODE_TX)
		return true;
	else
		return false;
}

static bool stwlc88_tx_is_rx_mode(void *dev_data)
{
	int ret;
	u8 mode = 0;

	ret = stwlc88_read_byte(ST88_OP_MODE_ADDR, &mode);
	if (ret) {
		hwlog_err("is_rx_mode: get rx mode failed\n");
		return false;
	}

	return mode == ST88_OP_MODE_RX;
}

static int stwlc88_tx_set_tx_open_flag(bool enable, void *dev_data)
{
	g_st88_tx_open_flag = enable;
	return 0;
}

static int stwlc88_tx_set_vset(int tx_vset, void *dev_data)
{
	int ret;
	u8 gpio_val;

	if (tx_vset == ST88_TX_PS_VOLT_5V5)
		gpio_val = ST88_TX_PS_GPIO_PU;
	else if (tx_vset == ST88_TX_PS_VOLT_6V8)
		gpio_val = ST88_TX_PS_GPIO_OPEN;
	else if (tx_vset == ST88_TX_PS_VOLT_10V)
		gpio_val = ST88_TX_PS_GPIO_PD;
	else
		return -WLC_ERR_PARA_WRONG;

	ret = stwlc88_write_byte_mask(ST88_TX_CUST_CTRL_ADDR,
		ST88_TX_PS_GPIO_MASK, ST88_TX_PS_GPIO_SHIFT, gpio_val);
	if (ret) {
		hwlog_err("tx_mode_vset: write failed\n");
		return ret;
	}

	return 0;
}

static int stwlc88_tx_set_pt_bridge(int v_ask, u8 type)
{
	int ret;

	ret = stwlc88_write_byte_mask(ST88_TX_CUST_CTRL_ADDR,
		ST88_TX_PT_BRIDGE_MASK, ST88_TX_PT_BRIDGE_SHIFT, type);
	if (ret) {
		hwlog_err("set_pt_bridge: write pt_brige failed\n");
		return ret;
	}

	if (v_ask < WIRELESS_ADAPTER_9V)
		return 0;

	ret = stwlc88_write_byte_mask(ST88_TX_CUST_CTRL_ADDR,
		ST88_TX_MANUAL_BRIDGE_FC_MASK, ST88_TX_MANUAL_BRIDGE_FC_SHIFT,
		ST88_TX_MANUAL_BRIDGE_FC_EN);
	if (ret) {
		hwlog_err("set_pt_bridge: write manual_bridge_fc failed\n");
		return ret;
	}

	return 0;
}

static int stwlc88_tx_set_ping_bridge(int v_ask, u8 type)
{
	int ret;

	ret = stwlc88_write_byte_mask(ST88_TX_CUST_CTRL_ADDR,
		ST88_TX_PING_BRIDGE_MASK, ST88_TX_PING_BRIDGE_SHIFT, type);
	ret += stwlc88_tx_set_pt_bridge(v_ask, ST88_TX_PT_BRIDGE_NO_CHANGE);
	if (ret) {
		hwlog_err("set_ping_bridge: write failed\n");
		return ret;
	}

	return 0;
}

static int stwlc88_tx_set_bridge(unsigned int v_ask, unsigned int type, void *dev_data)
{
	switch (type) {
	case WLTX_PING_HALF_BRIDGE:
		return stwlc88_tx_set_ping_bridge(v_ask, ST88_TX_PING_HALF_BRIDGE);
	case WLTX_PING_FULL_BRIDGE:
		return stwlc88_tx_set_ping_bridge(v_ask, ST88_TX_PING_FULL_BRIDGE);
	case WLTX_PT_HALF_BRIDGE:
		return stwlc88_tx_set_pt_bridge(v_ask, ST88_TX_PT_HALF_BRIDGE);
	case WLTX_PT_FULL_BRIDGE:
		return stwlc88_tx_set_pt_bridge(v_ask, ST88_TX_PT_FULL_BRIDGE);
	default:
		return -EINVAL;
	}
}

static bool stwlc88_tx_check_rx_disconnect(void *dev_data)
{
	struct stwlc88_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("check_rx_disconnect: di null\n");
		return true;
	}

	if (di->ept_type & ST88_TX_EPT_SRC_CEP_TIMEOUT) {
		di->ept_type &= ~ST88_TX_EPT_SRC_CEP_TIMEOUT;
		hwlog_info("[check_rx_disconnect] rx disconnect\n");
		return true;
	}

	return false;
}

static int stwlc88_tx_get_ping_interval(u16 *ping_interval, void *dev_data)
{
	int ret;
	u8 data = 0;

	if (!ping_interval) {
		hwlog_err("get_ping_interval: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc88_read_byte(ST88_TX_PING_INTERVAL_ADDR, &data);
	if (ret) {
		hwlog_err("get_ping_interval: read failed\n");
		return ret;
	}
	*ping_interval = data * ST88_TX_PING_INTERVAL_STEP;

	return 0;
}

static int stwlc88_tx_set_ping_interval(u16 ping_interval, void *dev_data)
{
	if ((ping_interval < ST88_TX_PING_INTERVAL_MIN) ||
		(ping_interval > ST88_TX_PING_INTERVAL_MAX)) {
		hwlog_err("set_ping_interval: para out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	return stwlc88_write_byte(ST88_TX_PING_INTERVAL_ADDR,
		(u8)(ping_interval / ST88_TX_PING_INTERVAL_STEP));
}

static int stwlc88_tx_get_ping_freq(u16 *ping_freq, void *dev_data)
{
	return stwlc88_read_byte(ST88_TX_PING_FREQ_ADDR, (u8 *)ping_freq);
}

static int stwlc88_tx_set_ping_freq(u16 ping_freq, void *dev_data)
{
	if ((ping_freq < ST88_TX_PING_FREQ_MIN) ||
		(ping_freq > ST88_TX_PING_FREQ_MAX)) {
		hwlog_err("set_ping_frequency: para out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	return stwlc88_write_byte(ST88_TX_PING_FREQ_ADDR, (u8)ping_freq);
}

static int stwlc88_tx_get_min_fop(u16 *fop, void *dev_data)
{
	return stwlc88_read_byte(ST88_TX_MIN_FOP_ADDR, (u8 *)fop);
}

static int stwlc88_tx_set_min_fop(u16 fop, void *dev_data)
{
	if ((fop < ST88_TX_MIN_FOP) || (fop > ST88_TX_MAX_FOP)) {
		hwlog_err("set_min_fop: para out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	return stwlc88_write_byte(ST88_TX_MIN_FOP_ADDR, (u8)fop);
}

static int stwlc88_tx_get_max_fop(u16 *fop, void *dev_data)
{
	return stwlc88_read_byte(ST88_TX_MAX_FOP_ADDR, (u8 *)fop);
}

static int stwlc88_tx_set_max_fop(u16 fop, void *dev_data)
{
	return stwlc88_write_byte(ST88_TX_MAX_FOP_ADDR, (u8)fop);
}

static int stwlc88_tx_get_fop(u16 *fop, void *dev_data)
{
	int ret;
	u16 val = 0;

	if (!fop) {
		hwlog_err("get_fop: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc88_read_word(ST88_TX_OP_FREQ_ADDR, &val);
	if (ret) {
		hwlog_err("get_fop: failed\n");
		return ret;
	}

	*fop = val * ST88_TX_OP_FREQ_STEP / 1000; /* 1kHz=1000Hz */
	return 0;
}

static int stwlc88_tx_set_duty_cycle(u8 min_dc, u8 max_dc)
{
	int ret;

	if (min_dc > max_dc)
		return -EINVAL;

	ret = stwlc88_write_byte(ST88_TX_MIN_DC_ADDR, min_dc);
	ret += stwlc88_write_byte(ST88_TX_MAX_DC_ADDR, max_dc);
	ret += stwlc88_write_byte(ST88_TX_PING_DC_ADDR, ST88_TX_PING_DC_VAL);
	if (ret) {
		hwlog_err("set_duty_cycle: failed\n");
		return ret;
	}

	return 0;
}

static int stwlc88_tx_get_duty(u8 *duty, void *dev_data)
{
	return stwlc88_read_byte(ST88_TX_DUTY_ADDR, duty);
}

static int stwlc88_tx_get_cep(s8 *cep, void *dev_data)
{
	return stwlc88_read_byte(ST88_TX_CEP_ADDR, cep);
}

static int stwlc88_tx_get_ptx(u32 *ptx, void *dev_data)
{
	return stwlc88_read_word(ST88_TX_TFRD_PWR_ADDR, (u16 *)ptx);
}

static int stwlc88_tx_get_prx(u32 *prx, void *dev_data)
{
	return stwlc88_read_word(ST88_TX_RCVD_PWR_ADDR, (u16 *)prx);
}

static int stwlc88_tx_get_ploss(s32 *ploss, void *dev_data)
{
	int ret;
	u32 ptx = 0;
	u32 prx = 0;

	if (!ploss)
		return -EINVAL;

	ret = stwlc88_tx_get_ptx(&ptx, dev_data);
	ret += stwlc88_tx_get_prx(&prx, dev_data);
	if (ret)
		return -EIO;

	*ploss = ptx - prx;
	return 0;
}

static int stwlc88_tx_get_ploss_id(u8 *id, void *dev_data)
{
	return stwlc88_read_byte(ST88_TX_FOD_TH_ID_ADDR, id);
}

static int stwlc88_tx_get_temp(s16 *chip_temp, void *dev_data)
{
	int ret;
	s16 temp = 0;

	if (!chip_temp) {
		hwlog_err("get_temp: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc88_read_word(ST88_TX_CHIP_TEMP_ADDR, &temp);
	if (ret) {
		hwlog_err("get_temp: read failed\n");
		return ret;
	}

	*chip_temp = (temp / 10); /* chip_temp in 0.1degC */

	return 0;
}

static int stwlc88_tx_get_vin(u16 *tx_vin, void *dev_data)
{
	return stwlc88_read_word(ST88_TX_VIN_ADDR, tx_vin);
}

static int stwlc88_tx_get_vrect(u16 *tx_vrect, void *dev_data)
{
	return stwlc88_read_word(ST88_TX_VRECT_ADDR, tx_vrect);
}

static int stwlc88_tx_get_iin(u16 *tx_iin, void *dev_data)
{
	return stwlc88_read_word(ST88_TX_IIN_ADDR, tx_iin);
}

static int stwlc88_tx_set_ilimit(u16 tx_ilim, void *dev_data)
{
	int ret;

	if ((tx_ilim < ST88_TX_ILIM_MIN) || (tx_ilim > ST88_TX_ILIM_MAX)) {
		hwlog_err("set_ilimit: out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	ret = stwlc88_write_byte(ST88_TX_ILIM_ADDR,
		(u8)(tx_ilim / ST88_TX_ILIM_STEP));
	if (ret) {
		hwlog_err("set_ilimit: failed\n");
		return ret;
	}

	return 0;
}

static int stwlc88_init_fod_para(struct stwlc88_dev_info *di)
{
	int ret;

	ret = stwlc88_write_block(ST88_TX_FOD_ADDR, di->tx_fod_para,
		ST88_TX_FOD_LEN);
	ret += stwlc88_write_byte(ST88_TX_PLOSS_CNT_ADDR, di->tx_fod_cnt);
	if (ret) {
		hwlog_err("init_fod_para: write fod_para failed\n");
		return ret;
	}

	return 0;
}

static int stwlc88_tx_stop_config(void *dev_data)
{
	struct stwlc88_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	di->g_val.tx_stop_chrg_flag = true;
	return 0;
}

static int stwlc88_tx_activate_chip(void *dev_data)
{
	int ret;

	ret = stwlc88_sw2tx();
	if (ret)
		hwlog_err("activate_tx_chip: sw2tx failed\n");

	return 0;
}

static int stwlc88_tx_set_irq_en(u32 val)
{
	int ret;

	ret = stwlc88_write_block(ST88_TX_IRQ_EN_ADDR, (u8 *)&val,
		ST88_TX_IRQ_EN_LEN);
	if (ret) {
		hwlog_err("irq_en: write failed\n");
		return ret;
	}

	return 0;
}

static void stwlc88_tx_select_init_para(struct stwlc88_dev_info *di,
	unsigned int client)
{
	switch (client) {
	case WLTX_OPEN_BY_CLIENT:
		di->tx_init_para.ping_freq = ST88_TX_PING_FREQ;
		di->tx_init_para.ping_interval = ST88_TX_PING_INTERVAL;
		break;
	case WLTX_OPEN_BY_COIL_TEST:
		di->tx_init_para.ping_freq = ST88_COIL_TEST_PING_FREQ;
		di->tx_init_para.ping_interval = ST88_COIL_TEST_PING_INTERVAL;
		break;
	default:
		di->tx_init_para.ping_freq = ST88_TX_PING_FREQ;
		di->tx_init_para.ping_interval = ST88_TX_PING_INTERVAL;
		break;
	}
}

static int stwlc88_tx_set_init_para(struct stwlc88_dev_info *di)
{
	int ret;

	ret = stwlc88_sw2tx();
	if (ret) {
		hwlog_err("set_init_para: sw2tx failed\n");
		return ret;
	}

	ret = stwlc88_write_byte(ST88_TX_OTP_TH_ADDR, ST88_TX_OTP_TH);
	ret += stwlc88_write_byte(ST88_TX_OCP_TH_ADDR,
		ST88_TX_OCP_TH / ST88_TX_OCP_TH_STEP);
	ret += stwlc88_write_byte(ST88_TX_OVP_TH_ADDR,
		ST88_TX_OVP_TH / ST88_TX_OVP_TH_STEP);
	ret += stwlc88_write_byte(ST88_TX_CEP_MAX_CNT_ADDR,
		ST88_TX_CEP_MAX_CNT_VAL);
	ret += stwlc88_init_fod_para(di);
	ret += stwlc88_tx_set_ping_freq(di->tx_init_para.ping_freq, di);
	ret += stwlc88_tx_set_min_fop(ST88_TX_MIN_FOP, di);
	ret += stwlc88_tx_set_max_fop(ST88_TX_MAX_FOP, di);
	ret += stwlc88_tx_set_ping_interval(di->tx_init_para.ping_interval, di);
	ret += stwlc88_tx_set_duty_cycle(ST88_TX_MIN_DC, ST88_TX_MAX_DC);
	ret += stwlc88_tx_set_irq_en(ST88_TX_IRQ_EN_VAL);
	if (ret) {
		hwlog_err("set_init_para: write failed\n");
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static int stwlc88_tx_chip_init(unsigned int client, void *dev_data)
{
	struct stwlc88_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	di->irq_cnt = 0;
	di->g_val.irq_abnormal_flag = false;
	di->g_val.tx_stop_chrg_flag = false;
	stwlc88_enable_irq(di);

	stwlc88_tx_select_init_para(di, client);
	return stwlc88_tx_set_init_para(di);
}

static int stwlc88_tx_enable_tx_mode(bool enable, void  *dev_data)
{
	int ret;

	if (enable)
		ret = stwlc88_write_word_mask(ST88_TX_CMD_ADDR,
			ST88_TX_CMD_EN_TX, ST88_TX_CMD_EN_TX_SHIFT,
			ST88_TX_CMD_VAL);
	else
		ret = stwlc88_write_word_mask(ST88_TX_CMD_ADDR,
			ST88_TX_CMD_DIS_TX, ST88_TX_CMD_DIS_TX_SHIFT,
			ST88_TX_CMD_VAL);

	if (ret) {
		hwlog_err("%s tx_mode failed\n", enable ? "enable" : "disable");
		return ret;
	}

	return 0;
}

static void stwlc88_tx_show_ept_type(u32 ept)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_stwlc88_tx_ept_name); i++) {
		if (ept & BIT(i))
			hwlog_info("[tx_ept] %s\n", g_stwlc88_tx_ept_name[i]);
	}
}

static int stwlc88_tx_get_ept_type(u32 *ept)
{
	int ret;
	u32 data = 0;

	if (!ept) {
		hwlog_err("get_ept_type: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc88_read_block(ST88_TX_EPT_SRC_ADDR, (u8 *)&data,
		ST88_TX_EPT_SRC_LEN);
	if (ret) {
		hwlog_err("get_ept_type: read failed\n");
		return ret;
	}
	hwlog_info("[get_ept_type] type=0x%08x", data);
	stwlc88_tx_show_ept_type(data);
	*ept = data;

	ret = stwlc88_write_word(ST88_TX_EPT_SRC_ADDR, 0);
	if (ret) {
		hwlog_err("get_ept_type: clr failed\n");
		return ret;
	}

	return 0;
}

static void stwlc88_tx_ept_handler(struct stwlc88_dev_info *di)
{
	int ret;

	ret = stwlc88_tx_get_ept_type(&di->ept_type);
	if (ret)
		return;

	switch (di->ept_type) {
	case ST88_TX_EPT_SRC_RX:
	case ST88_TX_EPT_SRC_RX_RST:
	case ST88_TX_EPT_SRC_CEP_TIMEOUT:
	case ST88_TX_EPT_SRC_OCP:
		di->ept_type &= ~ST88_TX_EPT_SRC_CEP_TIMEOUT;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_CEP_TIMEOUT, NULL);
		break;
	case ST88_TX_EPT_SRC_FOD:
		di->ept_type &= ~ST88_TX_EPT_SRC_FOD;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_TX_FOD, NULL);
		break;
	default:
		break;
	}
}

static int stwlc88_tx_clear_irq(u32 itr)
{
	int ret;

	ret = stwlc88_write_block(ST88_TX_IRQ_CLR_ADDR, (u8 *)&itr,
		ST88_TX_IRQ_CLR_LEN);
	if (ret) {
		hwlog_err("clear_irq: write failed\n");
		return ret;
	}

	return 0;
}

static void stwlc88_tx_ask_pkt_handler(struct stwlc88_dev_info *di)
{
	if (di->irq_val & ST88_TX_IRQ_SS_PKG_RCVD) {
		di->irq_val &= ~ST88_TX_IRQ_SS_PKG_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
	}

	if (di->irq_val & ST88_TX_IRQ_ID_PKT_RCVD) {
		di->irq_val &= ~ST88_TX_IRQ_ID_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
	}

	if (di->irq_val & ST88_TX_IRQ_CFG_PKT_RCVD) {
		di->irq_val &= ~ST88_TX_IRQ_CFG_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_GET_CFG, NULL);
	}

	if (di->irq_val & ST88_TX_IRQ_PP_PKT_RCVD) {
		di->irq_val &= ~ST88_TX_IRQ_PP_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_non_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_non_qi_ask_pkt(di);
	}
}

static void stwlc88_tx_show_irq(u32 intr)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_stwlc88_tx_irq_name); i++) {
		if (intr & BIT(i))
			hwlog_info("[tx_irq] %s\n", g_stwlc88_tx_irq_name[i]);
	}
}

static int stwlc88_tx_get_interrupt(u32 *intr)
{
	int ret;

	ret = stwlc88_read_block(ST88_TX_IRQ_ADDR, (u8 *)intr, ST88_TX_IRQ_LEN);
	if (ret)
		return ret;

	hwlog_info("[get_interrupt] irq=0x%08x\n", *intr);
	stwlc88_tx_show_irq(*intr);

	return 0;
}

static void stwlc88_tx_mode_irq_recheck(struct stwlc88_dev_info *di)
{
	int ret;
	u32 irq_val = 0;

	if (gpio_get_value(di->gpio_int))
		return;

	hwlog_info("[tx_mode_irq_recheck] gpio_int low, re-check irq\n");
	ret = stwlc88_tx_get_interrupt(&irq_val);
	if (ret)
		return;

	stwlc88_tx_clear_irq(ST88_TX_IRQ_CLR_ALL);
}

void stwlc88_tx_mode_irq_handler(struct stwlc88_dev_info *di)
{
	int ret;

	if (!di)
		return;

	ret = stwlc88_tx_get_interrupt(&di->irq_val);
	if (ret) {
		hwlog_err("irq_handler: get irq failed, clear\n");
		stwlc88_tx_clear_irq(ST88_TX_IRQ_CLR_ALL);
		goto rechk_irq;
	}

	stwlc88_tx_clear_irq(di->irq_val);

	stwlc88_tx_ask_pkt_handler(di);

	if (di->irq_val & ST88_TX_IRQ_START_PING) {
		di->irq_val &= ~ST88_TX_IRQ_START_PING;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_PING_RX, NULL);
	}
	if (di->irq_val & ST88_TX_IRQ_EPT_PKT_RCVD) {
		di->irq_val &= ~ST88_TX_IRQ_EPT_PKT_RCVD;
		stwlc88_tx_ept_handler(di);
	}
	if (di->irq_val & ST88_TX_IRQ_DPING_RCVD) {
		di->irq_val &= ~ST88_TX_IRQ_DPING_RCVD;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_RCV_DPING, NULL);
	}
	if (di->irq_val & ST88_TX_IRQ_RPP_TIMEOUT) {
		di->irq_val &= ~ST88_TX_IRQ_RPP_TIMEOUT;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_RP_DM_TIMEOUT, NULL);
	}
	if (di->irq_val & ST88_TX_IRQ_FOD_DET) {
		di->irq_val &= ~ST88_TX_IRQ_FOD_DET;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_TX_FOD, NULL);
	}

rechk_irq:
	stwlc88_tx_mode_irq_recheck(di);
}

static struct wltx_ic_ops g_stwlc88_tx_ic_ops = {
	.get_dev_node           = stwlc88_dts_dev_node,
	.fw_update              = stwlc88_fw_sram_update,
	.prev_psy               = NULL,
	.chip_init              = stwlc88_tx_chip_init,
	.chip_reset             = stwlc88_chip_reset,
	.chip_enable            = stwlc88_chip_enable,
	.mode_enable            = stwlc88_tx_enable_tx_mode,
	.activate_chip          = stwlc88_tx_activate_chip,
	.set_open_flag          = stwlc88_tx_set_tx_open_flag,
	.set_stop_cfg           = stwlc88_tx_stop_config,
	.is_rx_discon           = stwlc88_tx_check_rx_disconnect,
	.is_in_tx_mode          = stwlc88_tx_is_tx_mode,
	.is_in_rx_mode          = stwlc88_tx_is_rx_mode,
	.get_vrect              = stwlc88_tx_get_vrect,
	.get_vin                = stwlc88_tx_get_vin,
	.get_iin                = stwlc88_tx_get_iin,
	.get_temp               = stwlc88_tx_get_temp,
	.get_fop                = stwlc88_tx_get_fop,
	.get_cep                = stwlc88_tx_get_cep,
	.get_duty               = stwlc88_tx_get_duty,
	.get_ptx                = stwlc88_tx_get_ptx,
	.get_prx                = stwlc88_tx_get_prx,
	.get_ploss              = stwlc88_tx_get_ploss,
	.get_ploss_id           = stwlc88_tx_get_ploss_id,
	.get_ping_freq          = stwlc88_tx_get_ping_freq,
	.get_ping_interval      = stwlc88_tx_get_ping_interval,
	.get_min_fop            = stwlc88_tx_get_min_fop,
	.get_max_fop            = stwlc88_tx_get_max_fop,
	.get_full_bridge_ith    = NULL,
	.set_ping_freq          = stwlc88_tx_set_ping_freq,
	.set_ping_interval      = stwlc88_tx_set_ping_interval,
	.set_min_fop            = stwlc88_tx_set_min_fop,
	.set_max_fop            = stwlc88_tx_set_max_fop,
	.set_ilim               = stwlc88_tx_set_ilimit,
	.set_vset               = stwlc88_tx_set_vset,
	.set_fod_coef           = NULL,
	.set_rp_dm_to           = NULL,
	.set_bridge             = stwlc88_tx_set_bridge,
};

int stwlc88_tx_ops_register(struct stwlc88_dev_info *di)
{
	if (!di)
		return -ENODEV;

	g_stwlc88_tx_ic_ops.dev_data = (void *)di;
	return wltx_ic_ops_register(&g_stwlc88_tx_ic_ops, di->ic_type);
}
