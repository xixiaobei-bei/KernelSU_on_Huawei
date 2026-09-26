/*
 * mt5735_tx.c
 *
 * mt5735 tx driver
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

#define HWLOG_TAG wireless_mt5735_tx
HWLOG_REGIST();

static bool g_mt5735_tx_open_flag;

static const char * const g_mt5735_tx_irq_name[] = {
	/* [n]: n means bit in registers */
	[0]  = "tx_rst",
	[1]  = "tx_otp",
	[8]  = "tx_ss_pkt_rcvd",
	[9]  = "tx_id_pkt_rcvd",
	[10] = "tx_cfg_pkt_rcvd",
	[11] = "tx_pwr_trans",
	[12] = "tx_rmv_pwr",
	[13] = "tx_disable",
	[14] = "tx_enable",
	[15] = "tx_pp_pkt_rcvd",
	[16] = "tx_dping_rcvd",
	[17] = "tx_ept_pkt_rcvd",
	[18] = "tx_start_ping",
	[19] = "tx_ovp",
	[20] = "tx_ocp",
	[23] = "tx_fod_det",
	[24] = "tx_cep_timeout",
	[25] = "tx_rpp_timeout",
	[26] = "tx_irq_fsk_ack",
};
static const char * const g_mt5735_tx_ept_name[] = {
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

static int mt5735_sw2tx(void)
{
	int ret;
	int i;
	u16 mode = 0;
	int cnt = MT5735_SW2TX_RETRY_TIME / MT5735_SW2TX_SLEEP_TIME;

	for (i = 0; i < cnt; i++) {
		if (!g_mt5735_tx_open_flag) {
			hwlog_err("sw2tx: tx_open_flag false\n");
			return -WLC_ERR_STOP_CHRG;
		}
		msleep(MT5735_SW2TX_SLEEP_TIME);
		ret = mt5735_get_mode(&mode);
		if (ret) {
			hwlog_err("sw2tx: get mode failed\n");
			continue;
		}
		if (mode == MT5735_OP_MODE_TX) {
			hwlog_info("sw2tx: succ, cnt=%d\n", i);
			msleep(MT5735_SW2TX_SLEEP_TIME);
			return 0;
		}
		ret = mt5735_write_dword_mask(MT5735_TX_CMD_ADDR,
			MT5735_TX_CMD_EN_TX, MT5735_TX_CMD_EN_TX_SHIFT,
			MT5735_TX_CMD_VAL);
		if (ret)
			hwlog_err("sw2tx: write cmd(sw2tx) failed\n");
	}
	hwlog_err("sw2tx: failed, cnt=%d\n", i);
	return -WLC_ERR_I2C_WR;
}

static bool mt5735_tx_is_tx_mode(void *dev_data)
{
	int ret;
	u32 mode = 0;

	ret = mt5735_read_block(MT5735_OP_MODE_ADDR, (u8 *)&mode,
		MT5735_OP_MODE_LEN);
	if (ret) {
		hwlog_err("is_tx_mode: get op_mode failed\n");
		return false;
	}

	return (mode & MT5735_OP_MODE_TX);
}

static bool mt5735_tx_is_rx_mode(void *dev_data)
{
	int ret;
	u32 mode = 0;

	ret = mt5735_read_block(MT5735_OP_MODE_ADDR, (u8 *)&mode,
		MT5735_OP_MODE_LEN);
	if (ret) {
		hwlog_err("is_rx_mode: get rx mode failed\n");
		return false;
	}

	return (mode & MT5735_OP_MODE_RX);
}

static int mt5735_tx_set_tx_open_flag(bool enable, void *dev_data)
{
	g_mt5735_tx_open_flag = enable;
	return 0;
}

static int mt5735_tx_get_full_bridge_ith(u16 *ith, void *dev_data)
{
	struct mt5735_dev_info *di = dev_data;

	if (!ith)
		return -EINVAL;

	if (!di)
		*ith = MT5735_TX_FULL_BRIDGE_ITH;
	else
		*ith = (u16)di->full_bridge_ith;
	return 0;
}

static int mt5735_tx_set_pt_bridge(u8 type)
{
	int ret;

	ret = mt5735_write_byte_mask(MT5735_TX_CUST_CTRL_ADDR,
		MT5735_TX_PT_BRIDGE_MASK, MT5735_TX_PT_BRIDGE_SHIFT, type);
	if (ret) {
		hwlog_err("set pt bridge: write failed\n");
		return ret;
	}

	return 0;
}

static int mt5735_tx_set_ping_bridge(u8 type)
{
	int ret;

	ret = mt5735_write_byte_mask(MT5735_TX_CUST_CTRL_ADDR,
		MT5735_TX_PING_BRIDGE_MASK, MT5735_TX_PING_BRIDGE_SHIFT, type);
	ret += mt5735_tx_set_pt_bridge(MT5735_TX_PT_BRIDGE_NO_CHANGE);
	if (ret) {
		hwlog_err("set ping bridge: write failed\n");
		return ret;
	}

	return 0;
}

static int mt5735_tx_set_bridge(unsigned int v_ask, unsigned int type, void *dev_data)
{
	int ret;

	switch (type) {
	case WLTX_PING_HALF_BRIDGE:
		return mt5735_tx_set_ping_bridge(MT5735_TX_PING_HALF_BRIDGE);
	case WLTX_PING_FULL_BRIDGE:
		return mt5735_tx_set_ping_bridge(MT5735_TX_PING_FULL_BRIDGE);
	case WLTX_PT_HALF_BRIDGE:
		ret = mt5735_write_byte(MT5735_TX_PT_DC_ADDR, MT5735_TX_HALF_BRIDGE_DC);
		if (ret) {
			hwlog_err("set_pt_half_bridge: write failed\n");
			return -WLC_ERR_I2C_W;
		}
		return mt5735_tx_set_pt_bridge(MT5735_TX_PT_HALF_BRIDGE);
	case WLTX_PT_FULL_BRIDGE:
		ret = mt5735_write_byte(MT5735_TX_PT_DC_ADDR, MT5735_TX_FULL_BRIDGE_DC);
		if (ret) {
			hwlog_err("set_pt_full_bridge: write failed\n");
			return -WLC_ERR_I2C_W;
		}
		return mt5735_tx_set_pt_bridge(MT5735_TX_PT_FULL_BRIDGE);
	default:
		return -EINVAL;
	}
}

static bool mt5735_tx_check_rx_disconnect(void *dev_data)
{
	struct mt5735_dev_info *di = NULL;

	mt5735_get_dev_info(&di);
	if (!di)
		return true;

	if (di->ept_type & MT5735_TX_EPT_SRC_CEP_TIMEOUT) {
		di->ept_type &= ~MT5735_TX_EPT_SRC_CEP_TIMEOUT;
		hwlog_info("[check_rx_disconnect] rx disconnect\n");
		return true;
	}

	return false;
}

static int mt5735_tx_get_ping_interval(u16 *ping_interval, void *dev_data)
{
	int ret;
	u16 data = 0;

	if (!ping_interval) {
		hwlog_err("get_ping_interval: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = mt5735_read_word(MT5735_TX_PING_INTERVAL_ADDR, &data);
	if (ret) {
		hwlog_err("get_ping_interval: read failed\n");
		return ret;
	}
	*ping_interval = data / MT5735_TX_PING_INTERVAL_STEP;

	return 0;
}

static int mt5735_tx_set_ping_interval(u16 ping_interval, void *dev_data)
{
	if ((ping_interval < MT5735_TX_PING_INTERVAL_MIN) ||
		(ping_interval > MT5735_TX_PING_INTERVAL_MAX)) {
		hwlog_err("set_ping_interval: para out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	return mt5735_write_word(MT5735_TX_PING_INTERVAL_ADDR,
		ping_interval * MT5735_TX_PING_INTERVAL_STEP);
}

static int mt5735_tx_get_ping_freq(u16 *ping_freq, void *dev_data)
{
	int ret;
	u16 data = 0;

	if (!ping_freq) {
		hwlog_err("get_ping_frequency: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = mt5735_read_word(MT5735_TX_PING_FREQ_ADDR, &data);
	if (ret) {
		hwlog_err("get_ping_frequency: read failed\n");
		return ret;
	}
	*ping_freq = data / MT5735_TX_PING_STEP;

	return 0;
}

static int mt5735_tx_set_ping_freq(u16 ping_freq, void *dev_data)
{
	if ((ping_freq < MT5735_TX_PING_FREQ_MIN) ||
		(ping_freq > MT5735_TX_PING_FREQ_MAX)) {
		hwlog_err("set_ping_frequency: para out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	return mt5735_write_word(MT5735_TX_PING_FREQ_ADDR,
		ping_freq * MT5735_TX_PING_STEP);
}

static int mt5735_tx_get_min_fop(u16 *fop, void *dev_data)
{
	int ret;
	u16 data = 0;

	if (!fop) {
		hwlog_err("get_min_fop: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = mt5735_read_word(MT5735_TX_MIN_FOP_ADDR, &data);
	if (ret) {
		hwlog_err("get_min_fop: read failed\n");
		return ret;
	}
	*fop = data / MT5735_TX_FOP_STEP;

	return 0;
}

static int mt5735_tx_set_min_fop(u16 fop, void *dev_data)
{
	if ((fop < MT5735_TX_MIN_FOP) || (fop > MT5735_TX_MAX_FOP)) {
		hwlog_err("set_min_fop: para out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	return mt5735_write_word(MT5735_TX_MIN_FOP_ADDR,
		fop * MT5735_TX_FOP_STEP);
}

static int mt5735_tx_get_max_fop(u16 *fop, void *dev_data)
{
	int ret;
	u16 data = 0;

	if (!fop) {
		hwlog_err("get_max_fop: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = mt5735_read_word(MT5735_TX_MAX_FOP_ADDR, &data);
	if (ret) {
		hwlog_err("get_max_fop: read failed\n");
		return ret;
	}
	*fop = (u16)data / MT5735_TX_FOP_STEP;

	return 0;
}

static int mt5735_tx_set_max_fop(u16 fop, void *dev_data)
{
	if ((fop < MT5735_TX_MIN_FOP) || (fop > MT5735_TX_MAX_FOP)) {
		hwlog_err("set_max_fop: para out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	return mt5735_write_word(MT5735_TX_MAX_FOP_ADDR,
		fop * MT5735_TX_FOP_STEP);
}

static int mt5735_tx_get_fop(u16 *fop, void *dev_data)
{
	int ret;
	u16 val = 0;

	if (!fop) {
		hwlog_err("get_fop: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = mt5735_read_word(MT5735_TX_OP_FREQ_ADDR, &val);
	if (ret) {
		hwlog_err("get_fop: failed\n");
		return ret;
	}

	*fop = val / MT5735_TX_OP_FREQ_STEP; /* 1kHz=1000Hz */
	return 0;
}

static int mt5735_tx_set_duty_cycle(u8 min_dc, u8 max_dc)
{
	int ret;

	if (min_dc > max_dc)
		return -EINVAL;

	ret = mt5735_write_byte(MT5735_TX_PT_DC_ADDR, MT5735_TX_HALF_BRIDGE_DC);
	if (ret) {
		hwlog_err("set_duty_cycle: failed\n");
		return ret;
	}

	return 0;
}

static int mt5735_tx_get_temp(s16 *chip_temp, void *dev_data)
{
	return mt5735_read_word(MT5735_TX_CHIP_TEMP_ADDR, chip_temp);
}

static int mt5735_tx_get_cep(s8 *cep, void *dev_data)
{
	return mt5735_read_byte(MT5735_TX_CEP_ADDR, cep);
}

static int mt5735_tx_get_duty(u8 *duty, void *dev_data)
{
	return mt5735_read_byte(MT5735_TX_PWM_DUTY_ADDR, duty);
}

static int mt5735_tx_get_ptx(u32 *ptx, void *dev_data)
{
	return mt5735_read_dword(MT5735_TX_PTX_ADDR, ptx);
}

static int mt5735_tx_get_prx(u32 *prx, void *dev_data)
{
	return mt5735_read_dword(MT5735_TX_PRX_ADDR, prx);
}

static int mt5735_tx_get_ploss(s32 *ploss, void *dev_data)
{
	s16 val = 0;

	if (!ploss || mt5735_read_word(MT5735_TX_PLOSS_ADDR, &val))
		return -EINVAL;

	*ploss = val;
	return 0;
}

static int mt5735_tx_get_ploss_id(u8 *id, void *dev_data)
{
	return mt5735_read_byte(MT5735_TX_FOD_TH_ID_ADDR, id);
}

static int mt5735_tx_get_vrect(u16 *tx_vrect, void *dev_data)
{
	return mt5735_read_word(MT5735_TX_VRECT_ADDR, tx_vrect);
}

static int mt5735_tx_get_fod_status(u32 *fod_status, void *dev_data)
{
	return mt5735_read_dword(MT5735_TX_Q_FOD_ADDR, fod_status);
}

static int mt5735_tx_get_vin(u16 *tx_vin, void *dev_data)
{
	return mt5735_read_word(MT5735_TX_VIN_ADDR, tx_vin);
}

static int mt5735_tx_get_iin(u16 *tx_iin, void *dev_data)
{
	return mt5735_read_word(MT5735_TX_IIN_ADDR, tx_iin);
}

static int mt5735_tx_set_ilimit(u16 tx_ilim, void *dev_data)
{
	return 0;
}

static int mt5735_init_fod_para(struct mt5735_dev_info *di)
{
	int ret;

	ret = mt5735_write_word(MT5735_TX_PLOSS_TH0_ADDR,
		di->tx_fod.ploss_th0);
	ret += mt5735_write_word(MT5735_TX_PLOSS_TH1_ADDR,
		di->tx_fod.ploss_th1);
	ret += mt5735_write_word(MT5735_TX_PLOSS_TH2_ADDR,
		di->tx_fod.ploss_th2);
	ret += mt5735_write_word(MT5735_TX_PLOSS_TH3_ADDR,
		di->tx_fod.ploss_th3);
	ret += mt5735_write_word(MT5735_TX_HP_PLOSS_TH0_ADDR,
		di->tx_fod.hp_ploss_th0);
	ret += mt5735_write_word(MT5735_TX_HP_PLOSS_TH1_ADDR,
		di->tx_fod.hp_ploss_th1);
	ret += mt5735_write_byte(MT5735_TX_PLOSS_CNT_ADDR,
		di->tx_fod.ploss_cnt);
	if (ret) {
		hwlog_err("init_fod_coef: failed\n");
		return ret;
	}

	return 0;
}

static int mt5735_tx_stop_config(void *dev_data)
{
	struct mt5735_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	di->g_val.tx_stop_chrg_flag = true;
	return 0;
}

static int mt5735_tx_activate_chip(void *dev_data)
{
	int ret;

	ret = mt5735_sw2tx();
	if (ret)
		hwlog_err("activate_tx_chip: sw2tx failed\n");

	return 0;
}

static int mt5735_tx_set_irq_en(u32 val)
{
	int ret;

	ret = mt5735_write_block(MT5735_TX_IRQ_EN_ADDR, (u8 *)&val,
		MT5735_TX_IRQ_EN_LEN);
	if (ret) {
		hwlog_err("irq_en: write failed\n");
		return ret;
	}

	return 0;
}

static void mt5735_tx_select_init_para(struct mt5735_dev_info *di,
	unsigned int client)
{
	switch (client) {
	case WLTX_OPEN_BY_CLIENT:
		di->tx_init_para.ping_freq = MT5735_TX_PING_FREQ;
		di->tx_init_para.ping_interval = MT5735_TX_PING_INTERVAL;
		break;
	case WLTX_OPEN_BY_COIL_TEST:
		di->tx_init_para.ping_freq = MT5735_COIL_TEST_PING_FREQ;
		di->tx_init_para.ping_interval = MT5735_COIL_TEST_PING_INTERVAL;
		break;
	default:
		di->tx_init_para.ping_freq = MT5735_TX_PING_FREQ;
		di->tx_init_para.ping_interval = MT5735_TX_PING_INTERVAL;
		break;
	}
}

static int mt5735_tx_set_init_para(struct mt5735_dev_info *di)
{
	int ret;

	ret = mt5735_sw2tx();
	if (ret) {
		hwlog_err("set_init_para: sw2tx failed\n");
		return ret;
	}
	ret = mt5735_write_byte(MT5735_TX_OTP_TH_ADDR, MT5735_TX_OTP_TH);
	ret += mt5735_write_dword_mask(MT5735_TX_CMD_ADDR, MT5735_TX_CMD_OTP,
		MT5735_TX_CMD_OTP_SHIFT, MT5735_TX_CMD_VAL);
	ret += mt5735_write_word(MT5735_TX_OVP_TH_ADDR, MT5735_TX_OVP_TH);
	ret += mt5735_write_dword_mask(MT5735_TX_CMD_ADDR, MT5735_TX_CMD_OVP,
		MT5735_TX_CMD_OVP_SHIFT, MT5735_TX_CMD_VAL);
	ret += mt5735_write_word(MT5735_TX_OCP_TH_ADDR, MT5735_TX_OCP_TH);
	ret += mt5735_write_dword_mask(MT5735_TX_CMD_ADDR, MT5735_TX_CMD_OCP,
		MT5735_TX_CMD_OCP_SHIFT, MT5735_TX_CMD_VAL);
	ret += mt5735_init_fod_para(di);
	ret += mt5735_tx_set_ping_freq(di->tx_init_para.ping_freq, di);
	ret += mt5735_tx_set_min_fop(MT5735_TX_MIN_FOP, di);
	ret += mt5735_tx_set_max_fop(MT5735_TX_MAX_FOP, di);
	ret += mt5735_tx_set_ping_interval(di->tx_init_para.ping_interval, di);
	ret += mt5735_tx_set_duty_cycle(MT5735_TX_MIN_DC, MT5735_TX_MAX_DC);
	ret += mt5735_write_word(MT5735_TX_PT_DELTA_FREQ_ADDR, MT5735_TX_PT_DELTA_FREQ);
	ret += mt5735_tx_set_irq_en(MT5735_TX_IRQ_EN_VAL);
	ret += mt5735_write_dword_mask(MT5735_TX_CMD_ADDR, MT5735_TX_CMD_DIS_Q_CHECK,
		MT5735_TX_CMD_DIS_Q_CHECK_SHIFT, MT5735_TX_CMD_VAL);
	ret += mt5735_write_dword_mask(MT5735_TX_CMD_ADDR, MT5735_TX_CMD_FOD_CTRL,
		MT5735_TX_CMD_FOD_CTRL_SHIFT, MT5735_TX_CMD_VAL);
	if (ret) {
		hwlog_err("set_init_para: write failed\n");
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static int mt5735_tx_chip_init(unsigned int client, void *dev_data)
{
	struct mt5735_dev_info *di = dev_data;

	if (!di)
		return -WLC_ERR_PARA_NULL;

	di->irq_cnt = 0;
	di->g_val.irq_abnormal_flag = false;
	di->g_val.tx_stop_chrg_flag = false;
	mt5735_enable_irq(di);

	mt5735_tx_select_init_para(di, client);
	return mt5735_tx_set_init_para(di);
}

static int mt5735_tx_enable_tx_mode(bool enable, void *dev_data)
{
	int ret;

	if (enable)
		ret = mt5735_write_dword_mask(MT5735_TX_CMD_ADDR,
			MT5735_TX_CMD_START_TX, MT5735_TX_CMD_START_TX_SHIFT,
			MT5735_TX_CMD_VAL);
	else
		ret = mt5735_write_dword_mask(MT5735_TX_CMD_ADDR,
			MT5735_TX_CMD_STOP_TX, MT5735_TX_CMD_STOP_TX_SHIFT,
			MT5735_TX_CMD_VAL);
	if (ret) {
		hwlog_err("%s tx_mode failed\n", enable ? "enable" : "disable");
		return ret;
	}

	return 0;
}

static void mt5735_tx_show_ept_type(u32 ept)
{
	u32 i;

	for (i = 0; i < ARRAY_SIZE(g_mt5735_tx_ept_name); i++) {
		if (ept & BIT(i))
			hwlog_info("[tx_ept] %s\n", g_mt5735_tx_ept_name[i]);
	}
}

static int mt5735_tx_get_ept_type(u32 *ept)
{
	int ret;
	u32 data = 0;

	if (!ept) {
		hwlog_err("get_ept_type: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = mt5735_read_block(MT5735_TX_EPT_SRC_ADDR, (u8 *)&data,
		MT5735_TX_EPT_SRC_LEN);
	if (ret) {
		hwlog_err("get_ept_type: read failed\n");
		return ret;
	}
	hwlog_info("[get_ept_type] type=0x%08x", data);
	mt5735_tx_show_ept_type(data);
	*ept = data;

	ret = mt5735_write_word(MT5735_TX_EPT_SRC_ADDR, 0);
	if (ret) {
		hwlog_err("get_ept_type: clr failed\n");
		return ret;
	}

	return 0;
}

static void mt5735_tx_ept_handler(struct mt5735_dev_info *di)
{
	int ret;
	u32 fod_status = 0;

	ret = mt5735_tx_get_ept_type(&di->ept_type);
	if (ret)
		return;

	switch (di->ept_type) {
	case MT5735_TX_EPT_SRC_RX_EPT:
	case MT5735_TX_EPT_SRC_RX_RST:
	case MT5735_TX_EPT_SRC_CEP_TIMEOUT:
		di->ept_type &= ~MT5735_TX_EPT_SRC_CEP_TIMEOUT;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_CEP_TIMEOUT, NULL);
		break;
	case MT5735_TX_EPT_SRC_Q_FOD:
		di->ept_type &= ~MT5735_TX_EPT_SRC_Q_FOD;
		(void)mt5735_tx_get_fod_status(&fod_status, di);
		hwlog_info("[ept_handler] fod_status=0x%x\n", fod_status);
		/* fall-through */
	case MT5735_TX_EPT_SRC_FOD:
		di->ept_type &= ~MT5735_TX_EPT_SRC_FOD;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_TX_FOD, NULL);
		break;
	default:
		break;
	}
}

static int mt5735_tx_clear_irq(u32 itr)
{
	int ret;

	ret = mt5735_write_block(MT5735_TX_IRQ_CLR_ADDR, (u8 *)&itr,
		MT5735_TX_IRQ_CLR_LEN);
	ret += mt5735_write_dword_mask(MT5735_TX_CMD_ADDR,
		MT5735_TX_CMD_CLEAR_INT, MT5735_TX_CMD_CLEAR_INT_SHIFT,
		MT5735_TX_CMD_VAL);
	if (ret) {
		hwlog_err("clear_irq: write failed\n");
		return ret;
	}

	return 0;
}

static void mt5735_tx_ask_pkt_handler(struct mt5735_dev_info *di)
{
	if (di->irq_val & MT5735_TX_IRQ_SS_PKG_RCVD) {
		di->irq_val &= ~MT5735_TX_IRQ_SS_PKG_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
	}

	if (di->irq_val & MT5735_TX_IRQ_ID_PKT_RCVD) {
		di->irq_val &= ~MT5735_TX_IRQ_ID_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
	}

	if (di->irq_val & MT5735_TX_IRQ_CFG_PKT_RCVD) {
		di->irq_val &= ~MT5735_TX_IRQ_CFG_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_GET_CFG, NULL);
	}

	if (di->irq_val & MT5735_TX_IRQ_PP_PKT_RCVD) {
		di->irq_val &= ~MT5735_TX_IRQ_PP_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_non_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_non_qi_ask_pkt(di);
	}
}

static void mt5735_tx_show_irq(u32 intr)
{
	u32 i;

	for (i = 0; i < ARRAY_SIZE(g_mt5735_tx_irq_name); i++) {
		if (intr & BIT(i))
			hwlog_info("[tx_irq] %s\n", g_mt5735_tx_irq_name[i]);
	}
}

static int mt5735_tx_get_interrupt(u32 *intr)
{
	int ret;

	ret = mt5735_read_block(MT5735_TX_IRQ_ADDR, (u8 *)intr,
		MT5735_TX_IRQ_LEN);
	if (ret)
		return ret;

	hwlog_info("[get_interrupt] irq=0x%08x\n", *intr);
	mt5735_tx_show_irq(*intr);

	return 0;
}

static void mt5735_tx_mode_irq_recheck(struct mt5735_dev_info *di)
{
	int ret;
	u32 irq_val = 0;

	if (gpio_get_value(di->gpio_int))
		return;

	hwlog_info("[tx_mode_irq_recheck] gpio_int low, re-check irq\n");
	ret = mt5735_tx_get_interrupt(&irq_val);
	if (ret)
		return;

	mt5735_tx_clear_irq(MT5735_TX_IRQ_CLR_ALL);
}

void mt5735_tx_mode_irq_handler(struct mt5735_dev_info *di)
{
	int ret;

	if (!di)
		return;

	ret = mt5735_tx_get_interrupt(&di->irq_val);
	if (ret) {
		hwlog_err("irq_handler: get irq failed, clear\n");
		mt5735_tx_clear_irq(MT5735_TX_IRQ_CLR_ALL);
		goto rechk_irq;
	}

	mt5735_tx_clear_irq(di->irq_val);

	mt5735_tx_ask_pkt_handler(di);

	if (di->irq_val & MT5735_TX_IRQ_START_PING) {
		di->irq_val &= ~MT5735_TX_IRQ_START_PING;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_PING_RX, NULL);
	}
	if (di->irq_val & MT5735_TX_IRQ_EPT_PKT_RCVD) {
		di->irq_val &= ~MT5735_TX_IRQ_EPT_PKT_RCVD;
		mt5735_tx_ept_handler(di);
	}
	if (di->irq_val & MT5735_TX_IRQ_DPING_RCVD) {
		di->irq_val &= ~MT5735_TX_IRQ_DPING_RCVD;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_RCV_DPING, NULL);
	}
	if (di->irq_val & MT5735_TX_IRQ_RPP_TIMEOUT) {
		di->irq_val &= ~MT5735_TX_IRQ_RPP_TIMEOUT;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_RP_DM_TIMEOUT, NULL);
	}
	if (di->irq_val & MT5735_TX_IRQ_FOD_DET) {
		di->irq_val &= ~MT5735_TX_IRQ_FOD_DET;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_TX_FOD, NULL);
	}

rechk_irq:
	mt5735_tx_mode_irq_recheck(di);
}

static struct wltx_ic_ops g_mt5735_tx_ic_ops = {
	.get_dev_node           = mt5735_dts_dev_node,
	.fw_update              = mt5735_fw_sram_update,
	.prev_psy               = NULL,
	.chip_init              = mt5735_tx_chip_init,
	.chip_reset             = mt5735_chip_reset,
	.chip_enable            = mt5735_chip_enable,
	.mode_enable            = mt5735_tx_enable_tx_mode,
	.activate_chip          = mt5735_tx_activate_chip,
	.set_open_flag          = mt5735_tx_set_tx_open_flag,
	.set_stop_cfg           = mt5735_tx_stop_config,
	.is_rx_discon           = mt5735_tx_check_rx_disconnect,
	.is_in_tx_mode          = mt5735_tx_is_tx_mode,
	.is_in_rx_mode          = mt5735_tx_is_rx_mode,
	.get_vrect              = mt5735_tx_get_vrect,
	.get_vin                = mt5735_tx_get_vin,
	.get_iin                = mt5735_tx_get_iin,
	.get_temp               = mt5735_tx_get_temp,
	.get_fop                = mt5735_tx_get_fop,
	.get_cep                = mt5735_tx_get_cep,
	.get_duty               = mt5735_tx_get_duty,
	.get_ptx                = mt5735_tx_get_ptx,
	.get_prx                = mt5735_tx_get_prx,
	.get_ploss              = mt5735_tx_get_ploss,
	.get_ploss_id           = mt5735_tx_get_ploss_id,
	.get_ping_freq          = mt5735_tx_get_ping_freq,
	.get_ping_interval      = mt5735_tx_get_ping_interval,
	.get_min_fop            = mt5735_tx_get_min_fop,
	.get_max_fop            = mt5735_tx_get_max_fop,
	.get_full_bridge_ith    = mt5735_tx_get_full_bridge_ith,
	.set_ping_freq          = mt5735_tx_set_ping_freq,
	.set_ping_interval      = mt5735_tx_set_ping_interval,
	.set_min_fop            = mt5735_tx_set_min_fop,
	.set_max_fop            = mt5735_tx_set_max_fop,
	.set_ilim               = mt5735_tx_set_ilimit,
	.set_fod_coef           = NULL,
	.set_rp_dm_to           = NULL,
	.set_bridge             = mt5735_tx_set_bridge,
};

int mt5735_tx_ops_register(struct mt5735_dev_info *di)
{
	if (!di)
		return -ENODEV;

	g_mt5735_tx_ic_ops.dev_data = (void *)di;
	return wltx_ic_ops_register(&g_mt5735_tx_ic_ops, di->ic_type);
}
