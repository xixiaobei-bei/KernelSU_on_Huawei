/*
 * cps4057_tx.c
 *
 * cps4057 tx driver
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

#include "cps4057.h"

#define HWLOG_TAG wireless_cps4057_tx
HWLOG_REGIST();

static bool g_cps4057_tx_open_flag;

static const char * const g_cps4057_tx_irq_name[] = {
	/* [n]: n means bit in registers */
	[0] = "tx_start_ping",
	[1] = "tx_ss_pkt_rcvd",
	[2] = "tx_id_pkt_rcvd",
	[3] = "tx_cfg_pkt_rcvd",
	[4] = "tx_ask_pkt_rcvd",
	[5] = "tx_ept",
	[6] = "tx_rpp_timeout",
	[7] = "tx_cep_timeout",
	[8] = "tx_ac_detect",
	[9] = "tx_init",
	[11] = "rpp_type_err",
	[12] = "ask_ack",
};

static const char * const g_cps4057_tx_ept_name[] = {
	/* [n]: n means bit in registers */
	[0]  = "tx_ept_src_wrong_pkt",
	[1]  = "tx_ept_src_ac_detect",
	[2]  = "tx_ept_src_ss",
	[3]  = "tx_ept_src_rx_ept",
	[4]  = "tx_ept_src_cep_timeout",
	[6]  = "tx_ept_src_ocp",
	[7]  = "tx_ept_src_ovp",
	[8]  = "tx_ept_src_uvp",
	[9]  = "tx_ept_src_fod",
	[10] = "tx_ept_src_otp",
	[11] = "tx_ept_src_ping_ocp",
	[12] = "tx_ept_src_q_fod"
};

static bool cps4057_tx_is_in_tx_mode(void *dev_data)
{
	int ret;
	u8 mode = 0;

	ret = cps4057_read_byte(CPS4057_OP_MODE_ADDR, &mode);
	if (ret) {
		hwlog_err("is_tx_mode: get op_mode failed\n");
		return false;
	}

	return mode == CPS4057_OP_MODE_TX;
}

static bool cps4057_tx_is_in_rx_mode(void *dev_data)
{
	int ret;
	u8 mode = 0;

	ret = cps4057_read_byte(CPS4057_OP_MODE_ADDR, &mode);
	if (ret) {
		hwlog_err("is_rx_mode: get op_mode failed\n");
		return false;
	}

	return mode == CPS4057_OP_MODE_RX;
}

static int cps4057_tx_set_tx_open_flag(bool enable, void *dev_data)
{
	g_cps4057_tx_open_flag = enable;
	return 0;
}

static void cps4057_tx_chip_reset(void *dev_data)
{
	int ret;

	ret = cps4057_write_byte_mask(CPS4057_TX_CMD_ADDR,
		CPS4057_TX_CMD_SYS_RST, CPS4057_TX_CMD_SYS_RST_SHIFT,
		CPS4057_TX_CMD_VAL);
	if (ret) {
		hwlog_err("chip_reset: set cmd failed\n");
		return;
	}

	hwlog_info("[chip_reset] succ\n");
}

static int cps4057_tx_get_full_bridge_ith(u16 *ith, void *dev_data)
{
	struct cps4057_dev_info *di = dev_data;

	if (!ith)
		return -EINVAL;

	if (!di)
		*ith = CPS4057_FULL_BRIDGE_ITH;
	else
		*ith = (u16)di->full_bridge_ith;
	return 0;
}

static int cps4057_tx_set_pt_full_bridge(int v_ask, bool flag)
{
	int ret;
	u16 wr;

	if (flag)
		wr = CPS4057_TX_FUNC_EN;
	else
		wr = CPS4057_TX_FUNC_DIS;

	/* increase pt_freq and decrease duty to avoid of heavy load, in case of weak power source, such as otg src */
	if (v_ask < WIRELESS_ADAPTER_9V) {
		ret = cps4057_write_byte(CPS4057_TX_H2F_INC_FREQ_ADDR, CPS4057_TX_H2F_INC_FREQ_VAL);
		ret += cps4057_write_byte(CPS4057_TX_H2F_DEC_DUTY_ADDR, CPS4057_TX_H2F_DEC_DUTY_VAL);
		if (ret) {
			hwlog_err("set_pt_bridge: write manual_bridge_fc failed\n");
			return ret;
		}
	}
	ret = cps4057_write_word_mask(CPS4057_TX_FUNC_EN_ADDR,
		CPS4057_FULL_BRIDGE_EN_MASK, CPS4057_FULL_BRIDGE_EN_SHIFT, wr);
	if (ret) {
		hwlog_err("set_pt_bridge: write failed\n");
		return ret;
	}

	return 0;
}

static int cps4057_tx_set_bridge(unsigned int v_ask, unsigned int type, void *dev_data)
{
	if ((type == WLTX_PING_HALF_BRIDGE) || (type == WLTX_PING_FULL_BRIDGE))
		return 0;
	if (type == WLTX_PT_HALF_BRIDGE)
		return cps4057_tx_set_pt_full_bridge(v_ask, false);
	else if (type == WLTX_PT_FULL_BRIDGE)
		return cps4057_tx_set_pt_full_bridge(v_ask, true);

	return -EINVAL;
}

static bool cps4057_tx_check_rx_disconnect(void *dev_data)
{
	struct cps4057_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("check_rx_disconnect: di null\n");
		return true;
	}

	if (di->ept_type & CPS4057_TX_EPT_SRC_CEP_TIMEOUT) {
		di->ept_type &= ~CPS4057_TX_EPT_SRC_CEP_TIMEOUT;
		hwlog_info("[check_rx_disconnect] rx disconnect\n");
		return true;
	}

	return false;
}

static int cps4057_tx_get_ping_interval(u16 *ping_interval, void *dev_data)
{
	return cps4057_read_word(CPS4057_TX_PING_INTERVAL_ADDR, ping_interval);
}

static int cps4057_tx_set_ping_interval(u16 ping_interval, void *dev_data)
{
	if ((ping_interval < CPS4057_TX_PING_INTERVAL_MIN) ||
		(ping_interval > CPS4057_TX_PING_INTERVAL_MAX)) {
		hwlog_err("set_ping_interval: para out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	return cps4057_write_word(CPS4057_TX_PING_INTERVAL_ADDR, ping_interval);
}

static int cps4057_tx_get_ping_freq(u16 *ping_freq, void *dev_data)
{
	return cps4057_read_byte(CPS4057_TX_PING_FREQ_ADDR, (u8 *)ping_freq);
}

static int cps4057_tx_set_ping_freq(u16 ping_freq, void *dev_data)
{
	if ((ping_freq < CPS4057_TX_PING_FREQ_MIN) ||
		(ping_freq > CPS4057_TX_PING_FREQ_MAX)) {
		hwlog_err("set_ping_frequency: para out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	return cps4057_write_byte(CPS4057_TX_PING_FREQ_ADDR, (u8)ping_freq);
}

static int cps4057_tx_get_min_fop(u16 *fop, void *dev_data)
{
	return cps4057_read_byte(CPS4057_TX_MIN_FOP_ADDR, (u8 *)fop);
}

static int cps4057_tx_set_min_fop(u16 fop, void *dev_data)
{
	if ((fop < CPS4057_TX_MIN_FOP) || (fop > CPS4057_TX_MAX_FOP)) {
		hwlog_err("set_min_fop: para out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	return cps4057_write_byte(CPS4057_TX_MIN_FOP_ADDR, (u8)fop);
}

static int cps4057_tx_get_max_fop(u16 *fop, void *dev_data)
{
	return cps4057_read_byte(CPS4057_TX_MAX_FOP_ADDR, (u8 *)fop);
}

static int cps4057_tx_set_max_fop(u16 fop, void *dev_data)
{
	if ((fop < CPS4057_TX_MIN_FOP) || (fop > CPS4057_TX_MAX_FOP)) {
		hwlog_err("set_max_fop: para out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	return cps4057_write_byte(CPS4057_TX_MAX_FOP_ADDR, (u8)fop);
}

static int cps4057_tx_get_fop(u16 *fop, void *dev_data)
{
	return cps4057_read_word(CPS4057_TX_OP_FREQ_ADDR, fop);
}

static int cps4057_tx_get_cep(s8 *cep, void *dev_data)
{
	return cps4057_read_byte(CPS4057_TX_CEP_ADDR, cep);
}

static int cps4057_tx_get_duty(u8 *duty, void *dev_data)
{
	int ret;
	u16 pwm_duty = 0;

	if (!duty) {
		hwlog_err("get_duty: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = cps4057_read_word(CPS4057_TX_PWM_DUTY_ADDR, &pwm_duty);
	if (ret)
		return ret;

	*duty = (u8)(pwm_duty / CPS4057_TX_PWM_DUTY_UNIT);
	return 0;
}

static int cps4057_tx_get_ptx(u32 *ptx, void *dev_data)
{
	return cps4057_read_word(CPS4057_TX_PTX_ADDR, (u16 *)ptx);
}

static int cps4057_tx_get_prx(u32 *prx, void *dev_data)
{
	return cps4057_read_word(CPS4057_TX_PRX_ADDR, (u16 *)prx);
}

static int cps4057_tx_get_ploss(s32 *ploss, void *dev_data)
{
	s16 val = 0;

	if (cps4057_read_word(CPS4057_TX_PLOSS_ADDR, &val))
		return -EIO;

	*ploss = val;
	return 0;
}

static int cps4057_tx_get_ploss_id(u8 *id, void *dev_data)
{
	return cps4057_read_byte(CPS4057_TX_FOD_TH_ID_ADDR, id);
}

static int cps4057_tx_get_temp(s16 *chip_temp, void *dev_data)
{
	return cps4057_read_word(CPS4057_TX_CHIP_TEMP_ADDR, chip_temp);
}

static int cps4057_tx_get_vin(u16 *tx_vin, void *dev_data)
{
	return cps4057_read_word(CPS4057_TX_VIN_ADDR, tx_vin);
}

static int cps4057_tx_get_vrect(u16 *tx_vrect, void *dev_data)
{
	return cps4057_read_word(CPS4057_TX_VRECT_ADDR, tx_vrect);
}

static int cps4057_tx_get_iin(u16 *tx_iin, void *dev_data)
{
	return cps4057_read_word(CPS4057_TX_IIN_ADDR, tx_iin);
}

static int cps4057_tx_set_ilimit(u16 tx_ilim, void *dev_data)
{
	int ret;

	if ((tx_ilim < CPS4057_TX_ILIM_MIN) ||
		(tx_ilim > CPS4057_TX_ILIM_MAX)) {
		hwlog_err("set_ilimit: out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	ret = cps4057_write_word(CPS4057_TX_ILIM_ADDR, tx_ilim);
	if (ret) {
		hwlog_err("set_ilimit: failed\n");
		return ret;
	}

	return 0;
}

static int cps4057_tx_init_fod_coef(struct cps4057_dev_info *di)
{
	int ret;

	ret = cps4057_write_word(CPS4057_TX_PLOSS_TH0_ADDR,
		di->tx_fod.ploss_th0);
	ret += cps4057_write_word(CPS4057_TX_PLOSS_TH1_ADDR,
		di->tx_fod.ploss_th1);
	ret += cps4057_write_word(CPS4057_TX_PLOSS_TH2_ADDR,
		di->tx_fod.ploss_th2);
	ret += cps4057_write_word(CPS4057_TX_PLOSS_TH3_ADDR,
		di->tx_fod.ploss_th3);
	ret = cps4057_write_word(CPS4057_TX_HP_PLOSS_TH0_ADDR,
		di->tx_fod.hp_ploss_th0);
	ret = cps4057_write_word(CPS4057_TX_HP_PLOSS_TH1_ADDR,
		di->tx_fod.hp_ploss_th1);
	ret = cps4057_write_word(CPS4057_TX_HP_PLOSS_TH2_ADDR,
		di->tx_fod.hp_ploss_th2);
	ret = cps4057_write_word(CPS4057_TX_HP_FOD_CUR_TH0_ADDR,
		di->tx_fod.hp_cur_th0);
	ret = cps4057_write_word(CPS4057_TX_HP_FOD_CUR_TH1_ADDR,
		di->tx_fod.hp_cur_th1);
	ret += cps4057_write_byte(CPS4057_TX_PLOSS_CNT_ADDR,
		di->tx_fod.ploss_cnt);
	if (ret) {
		hwlog_err("init_fod_coef: failed\n");
		return ret;
	}

	return 0;
}

static int cps4057_tx_stop_config(void *dev_data)
{
	struct cps4057_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	di->g_val.tx_stop_chrg_flag = true;
	return 0;
}

static int cps4057_sw2tx(void)
{
	int ret;
	int i;
	u8 mode = 0;
	int cnt = CPS4057_SW2TX_RETRY_TIME / CPS4057_SW2TX_SLEEP_TIME;

	for (i = 0; i < cnt; i++) {
		msleep(CPS4057_SW2TX_SLEEP_TIME);
		ret = cps4057_get_mode(&mode);
		if (ret) {
			hwlog_err("sw2tx: get mode failed\n");
			continue;
		}
		if (mode == CPS4057_OP_MODE_TX) {
			hwlog_info("sw2tx: succ, cnt=%d\n", i);
			msleep(CPS4057_SW2TX_SLEEP_TIME);
			return 0;
		}
		ret = cps4057_write_byte_mask(CPS4057_TX_CMD_ADDR,
			CPS4057_TX_CMD_EN_TX, CPS4057_TX_CMD_EN_TX_SHIFT,
			CPS4057_TX_CMD_VAL);
		if (ret)
			hwlog_err("sw2tx: write cmd(sw2tx) failed\n");
	}
	hwlog_err("sw2tx: failed, cnt=%d\n", i);
	return -WLC_ERR_I2C_WR;
}

static void cps4057_tx_select_init_para(struct cps4057_dev_info *di,
	unsigned int client)
{
	switch (client) {
	case WLTX_OPEN_BY_CLIENT:
		di->tx_init_para.ping_freq = CPS4057_TX_PING_FREQ;
		di->tx_init_para.ping_interval = CPS4057_TX_PING_INTERVAL;
		break;
	case WLTX_OPEN_BY_COIL_TEST:
		di->tx_init_para.ping_freq = CPS4057_COIL_TEST_PING_FREQ;
		di->tx_init_para.ping_interval = CPS4057_COIL_TEST_PING_INTERVAL;
		break;
	default:
		di->tx_init_para.ping_freq = CPS4057_TX_PING_FREQ;
		di->tx_init_para.ping_interval = CPS4057_TX_PING_INTERVAL;
		break;
	}
}

static int cps4057_tx_set_init_para(struct cps4057_dev_info *di)
{
	int ret;

	ret = cps4057_sw2tx();
	if (ret) {
		hwlog_err("set_init_para: sw2tx failed\n");
		return ret;
	}

	ret = cps4057_write_word(CPS4057_TX_OCP_TH_ADDR, CPS4057_TX_OCP_TH);
	ret += cps4057_write_word(CPS4057_TX_OVP_TH_ADDR, CPS4057_TX_OVP_TH);
	ret += cps4057_write_word(CPS4057_TX_IRQ_EN_ADDR, CPS4057_TX_IRQ_VAL);
	ret += cps4057_write_word(CPS4057_TX_PING_OCP_ADDR, CPS4057_TX_PING_OCP_TH);
	ret += cps4057_tx_init_fod_coef(di);
	ret += cps4057_tx_set_ping_freq(di->tx_init_para.ping_freq, di);
	ret += cps4057_tx_set_min_fop(CPS4057_TX_MIN_FOP, di);
	ret += cps4057_tx_set_max_fop(CPS4057_TX_MAX_FOP, di);
	ret += cps4057_tx_set_ping_interval(di->tx_init_para.ping_interval, di);
	ret += cps4057_write_byte_mask(CPS4057_TX_FUNC_EN_ADDR,
		CPS4057_TX_Q_EN_MASK, CPS4057_TX_Q_EN_SHIFT, di->tx_fod.q_en);
	ret += cps4057_write_word(CPS4057_TX_Q_ONLY_COIL_ADDR, di->tx_fod.q_coil_th);
	ret += cps4057_write_word(CPS4057_TX_Q_TH_ADDR, di->tx_fod.q_th);
	if (ret) {
		hwlog_err("set_init_para: write failed\n");
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static int cps4057_tx_chip_init(unsigned int client, void *dev_data)
{
	struct cps4057_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	di->irq_cnt = 0;
	di->g_val.irq_abnormal_flag = false;
	di->g_val.tx_stop_chrg_flag = false;
	cps4057_enable_irq(di);

	cps4057_tx_select_init_para(di, client);
	return cps4057_tx_set_init_para(di);
}

static int cps4057_tx_enable_tx_mode(bool enable, void *dev_data)
{
	int ret;

	if (enable)
		ret = cps4057_write_byte_mask(CPS4057_TX_FUNC_EN_ADDR,
			CPS4057_TX_FUNC_EN_TX_MASK, CPS4057_TX_FUNC_EN_SHIFT,
			CPS4057_TX_FUNC_EN);
	else
		ret = cps4057_write_byte_mask(CPS4057_TX_FUNC_EN_ADDR,
			CPS4057_TX_FUNC_EN_TX_MASK, CPS4057_TX_FUNC_EN_SHIFT,
			CPS4057_TX_FUNC_DIS);

	if (ret) {
		hwlog_err("%s tx_mode failed\n", enable ? "enable" : "disable");
		return ret;
	}

	return 0;
}

static void cps4057_tx_show_ept_type(u16 ept)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_cps4057_tx_ept_name); i++) {
		if (ept & BIT(i))
			hwlog_info("[tx_ept] %s\n", g_cps4057_tx_ept_name[i]);
	}
}

static int cps4057_tx_get_ept_type(u16 *ept)
{
	int ret;
	u16 ept_value = 0;

	if (!ept) {
		hwlog_err("get_ept_type: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = cps4057_read_word(CPS4057_TX_EPT_SRC_ADDR, &ept_value);
	if (ret) {
		hwlog_err("get_ept_type: read failed\n");
		return ret;
	}
	*ept = ept_value;
	hwlog_info("[get_ept_type] type=0x%02x", *ept);
	cps4057_tx_show_ept_type(*ept);

	return 0;
}

static int cps4057_tx_clear_ept_src(void)
{
	int ret;

	ret = cps4057_write_word(CPS4057_TX_EPT_SRC_ADDR, CPS4057_TX_EPT_SRC_CLEAR);
	if (ret) {
		hwlog_err("clear_ept_src: failed\n");
		return ret;
	}

	hwlog_info("[clear_ept_src] success\n");
	return 0;
}

static void cps4057_tx_ept_handler(struct cps4057_dev_info *di)
{
	int ret;
	u16 q_detect = 0;

	ret = cps4057_tx_get_ept_type(&di->ept_type);
	ret += cps4057_tx_clear_ept_src();
	if (ret)
		return;

	switch (di->ept_type) {
	case CPS4057_TX_EPT_SRC_RX_EPT:
	case CPS4057_TX_EPT_SRC_SSP:
	case CPS4057_TX_EPT_SRC_CEP_TIMEOUT:
	case CPS4057_TX_EPT_SRC_OCP:
		di->ept_type &= ~CPS4057_TX_EPT_SRC_CEP_TIMEOUT;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_CEP_TIMEOUT, NULL);
		break;
	case CPS4057_TX_EPT_SRC_Q_FOD:
		cps4057_read_word(CPS4057_TX_Q_VAL_ADDR, &q_detect);
		hwlog_err("tx_ept_handler: detected q_val=%u\n", q_detect);
		/* fall-through */
	case CPS4057_TX_EPT_SRC_FOD:
	case CPS4057_TX_EPT_SRC_POCP:
		di->ept_type &= ~CPS4057_TX_EPT_SRC_FOD;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_TX_FOD, NULL);
		break;
	default:
		break;
	}
}

static int cps4057_tx_clear_irq(u16 itr)
{
	int ret;

	ret = cps4057_write_word(CPS4057_TX_IRQ_CLR_ADDR, itr);
	if (ret) {
		hwlog_err("clear_irq: write failed\n");
		return ret;
	}

	return 0;
}

static void cps4057_tx_ask_pkt_handler(struct cps4057_dev_info *di)
{
	if (di->irq_val & CPS4057_TX_IRQ_SS_PKG_RCVD) {
		di->irq_val &= ~CPS4057_TX_IRQ_SS_PKG_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
	}

	if (di->irq_val & CPS4057_TX_IRQ_ID_PKT_RCVD) {
		di->irq_val &= ~CPS4057_TX_IRQ_ID_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
	}

	if (di->irq_val & CPS4057_TX_IRQ_CFG_PKT_RCVD) {
		di->irq_val &= ~CPS4057_TX_IRQ_CFG_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_GET_CFG, NULL);
	}

	if (di->irq_val & CPS4057_TX_IRQ_ASK_PKT_RCVD) {
		di->irq_val &= ~CPS4057_TX_IRQ_ASK_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_non_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_non_qi_ask_pkt(di);
	}
}

static void cps4057_tx_show_irq(u32 intr)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_cps4057_tx_irq_name); i++) {
		if (intr & BIT(i))
			hwlog_info("[tx_irq] %s\n", g_cps4057_tx_irq_name[i]);
	}
}

static int cps4057_tx_get_interrupt(u16 *intr)
{
	int ret;

	ret = cps4057_read_word(CPS4057_TX_IRQ_ADDR, intr);
	if (ret)
		return ret;

	hwlog_info("[get_interrupt] irq=0x%04x\n", *intr);
	cps4057_tx_show_irq(*intr);

	return 0;
}

static void cps4057_tx_mode_irq_recheck(struct cps4057_dev_info *di)
{
	int ret;
	u16 irq_val = 0;

	if (gpio_get_value(di->gpio_int))
		return;

	hwlog_info("[tx_mode_irq_recheck] gpio_int low, re-check irq\n");
	ret = cps4057_tx_get_interrupt(&irq_val);
	if (ret)
		return;

	cps4057_tx_clear_irq(CPS4057_TX_IRQ_CLR_ALL);
}

void cps4057_tx_mode_irq_handler(struct cps4057_dev_info *di)
{
	int ret;

	if (!di)
		return;

	ret = cps4057_tx_get_interrupt(&di->irq_val);
	if (ret) {
		hwlog_err("irq_handler: get irq failed, clear\n");
		cps4057_tx_clear_irq(CPS4057_TX_IRQ_CLR_ALL);
		goto rechk_irq;
	}

	cps4057_tx_clear_irq(di->irq_val);

	cps4057_tx_ask_pkt_handler(di);

	if (di->irq_val & CPS4057_TX_IRQ_START_PING) {
		di->irq_val &= ~CPS4057_TX_IRQ_START_PING;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_PING_RX, NULL);
	}
	if (di->irq_val & CPS4057_TX_IRQ_EPT_PKT_RCVD) {
		di->irq_val &= ~CPS4057_TX_IRQ_EPT_PKT_RCVD;
		cps4057_tx_ept_handler(di);
	}
	if (di->irq_val & CPS4057_TX_IRQ_AC_DET) {
		di->irq_val &= ~CPS4057_TX_IRQ_AC_DET;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_RCV_DPING, NULL);
	}
	if (di->irq_val & CPS4057_TX_IRQ_RPP_TIMEOUT) {
		di->irq_val &= ~CPS4057_TX_IRQ_RPP_TIMEOUT;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_RP_DM_TIMEOUT, NULL);
	}

rechk_irq:
	cps4057_tx_mode_irq_recheck(di);
}

static struct wltx_ic_ops g_cps4057_tx_ic_ops = {
	.get_dev_node           = cps4057_dts_dev_node,
	.fw_update              = cps4057_fw_sram_update,
	.prev_psy               = NULL,
	.chip_init              = cps4057_tx_chip_init,
	.chip_reset             = cps4057_tx_chip_reset,
	.chip_enable            = cps4057_chip_enable,
	.mode_enable            = cps4057_tx_enable_tx_mode,
	.activate_chip          = NULL,
	.set_open_flag          = cps4057_tx_set_tx_open_flag,
	.set_stop_cfg           = cps4057_tx_stop_config,
	.is_rx_discon           = cps4057_tx_check_rx_disconnect,
	.is_in_tx_mode          = cps4057_tx_is_in_tx_mode,
	.is_in_rx_mode          = cps4057_tx_is_in_rx_mode,
	.get_vrect              = cps4057_tx_get_vrect,
	.get_vin                = cps4057_tx_get_vin,
	.get_iin                = cps4057_tx_get_iin,
	.get_temp               = cps4057_tx_get_temp,
	.get_fop                = cps4057_tx_get_fop,
	.get_cep                = cps4057_tx_get_cep,
	.get_duty               = cps4057_tx_get_duty,
	.get_ptx                = cps4057_tx_get_ptx,
	.get_prx                = cps4057_tx_get_prx,
	.get_ploss              = cps4057_tx_get_ploss,
	.get_ploss_id           = cps4057_tx_get_ploss_id,
	.get_ping_freq          = cps4057_tx_get_ping_freq,
	.get_ping_interval      = cps4057_tx_get_ping_interval,
	.get_min_fop            = cps4057_tx_get_min_fop,
	.get_max_fop            = cps4057_tx_get_max_fop,
	.get_full_bridge_ith    = cps4057_tx_get_full_bridge_ith,
	.set_ping_freq          = cps4057_tx_set_ping_freq,
	.set_ping_interval      = cps4057_tx_set_ping_interval,
	.set_min_fop            = cps4057_tx_set_min_fop,
	.set_max_fop            = cps4057_tx_set_max_fop,
	.set_ilim               = cps4057_tx_set_ilimit,
	.set_fod_coef           = NULL,
	.set_rp_dm_to           = NULL,
	.set_bridge             = cps4057_tx_set_bridge,
};

int cps4057_tx_ops_register(struct cps4057_dev_info *di)
{
	if (!di)
		return -ENODEV;

	g_cps4057_tx_ic_ops.dev_data = (void *)di;
	return wltx_ic_ops_register(&g_cps4057_tx_ic_ops, di->ic_type);
}
