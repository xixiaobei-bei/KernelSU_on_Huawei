/*
 * cps7181_tx.c
 *
 * cps7181 tx driver
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

#define HWLOG_TAG wireless_cps7181_tx
HWLOG_REGIST();

static const char * const g_cps7181_tx_irq_name[] = {
	/* [n]: n means bit in registers */
	[0]  = "tx_start_ping",
	[1]  = "tx_ss_pkt_rcvd",
	[2]  = "tx_id_pkt_rcvd",
	[3]  = "tx_cfg_pkt_rcvd",
	[4]  = "tx_pp_pkt_rcvd",
	[5]  = "tx_ept",
	[6]  = "tx_rpp_timeout",
	[7]  = "tx_cep_timeout",
	[8]  = "tx_ac_detect",
	[9]  = "tx_tx_init",
	[11] = "tx_rpp_type_err",
};

static const char * const g_cps7181_tx_ept_name[] = {
	/* [n]: n means bit in registers */
	[0]  = "tx_ept_src_wrong_pkt",
	[1]  = "tx_ept_src_ac_detect",
	[2]  = "tx_ept_src_ss",
	[3]  = "tx_ept_src_rx_ept",
	[4]  = "tx_ept_src_cep_timeout",
	[5]  = "tx_ept_src_rpp_timeout",
	[6]  = "tx_ept_src_ocp",
	[7]  = "tx_ept_src_ovp",
	[8]  = "tx_ept_src_uvp",
	[9]  = "tx_ept_src_fod",
	[10] = "tx_ept_src_otp",
	[11] = "tx_ept_src_ping_ocp",
};

static unsigned int cps7181_tx_get_bnt_wltx_type(int ic_type)
{
	return ic_type == WLTRX_IC_AUX ? POWER_BNT_WLTX_AUX : POWER_BNT_WLTX;
}

static bool cps7181_tx_is_tx_mode(void *dev_data)
{
	int ret;
	u8 mode = 0;
	struct cps7181_dev_info *di = dev_data;

	ret = cps7181_read_byte(di, CPS7181_OP_MODE_ADDR, &mode);
	if (ret) {
		hwlog_err("is_tx_mode: get op_mode failed\n");
		return false;
	}

	return mode == CPS7181_OP_MODE_TX;
}

static bool cps7181_tx_is_rx_mode(void *dev_data)
{
	int ret;
	u8 mode = 0;
	struct cps7181_dev_info *di = dev_data;

	ret = cps7181_read_byte(di, CPS7181_OP_MODE_ADDR, &mode);
	if (ret) {
		hwlog_err("is_rx_mode: get rx mode failed\n");
		return false;
	}

	return mode == CPS7181_OP_MODE_RX;
}

static int cps7181_tx_set_tx_open_flag(bool enable, void *dev_data)
{
	return 0;
}

static void cps7181_tx_chip_reset(void *dev_data)
{
	int ret;
	struct cps7181_dev_info *di = dev_data;

	ret = cps7181_fw_sram_i2c_init(di, CPS7181_BYTE_INC);
	if (ret) {
		hwlog_err("chip_reset: i2c init failed\n");
		return;
	}
	ret = cps7181_write_byte_mask(di, CPS7181_TX_CMD_ADDR,
		CPS7181_TX_CMD_SYS_RST, CPS7181_TX_CMD_SYS_RST_SHIFT,
		CPS7181_TX_CMD_VAL);
	if (ret) {
		hwlog_err("chip_reset: set cmd failed\n");
		return;
	}
	power_usleep(DT_USLEEP_10MS);
	ret = cps7181_fw_sram_i2c_init(di, CPS7181_BYTE_INC);
	if (ret) {
		hwlog_err("chip_reset: i2c init failed\n");
		return;
	}

	hwlog_info("[chip_reset] succ\n");
}

static int cps7181_tx_set_vset(int tx_vset, void *dev_data)
{
	int ret;
	u8 gpio_val;
	struct cps7181_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (tx_vset == CPS7181_TX_PS_VOLT_5V5)
		gpio_val = CPS7181_TX_PS_GPIO_PU;
	else if ((tx_vset == CPS7181_TX_PS_VOLT_6V8) ||
		(tx_vset == CPS7181_TX_PS_VOLT_6V))
		gpio_val = CPS7181_TX_PS_GPIO_OPEN;
	else if ((tx_vset == CPS7181_TX_PS_VOLT_10V) ||
		(tx_vset == CPS7181_TX_PS_VOLT_6V9))
		gpio_val = CPS7181_TX_PS_GPIO_PD;
	else
		return -EINVAL;

	ret = cps7181_write_byte(di, CPS7181_TX_PS_CTRL_ADDR, gpio_val);
	if (ret) {
		hwlog_err("tx_mode_vset: write failed\n");
		return ret;
	}

	return 0;
}

static bool cps7181_tx_check_rx_disconnect(void *dev_data)
{
	struct cps7181_dev_info *di = dev_data;

	if (!di)
		return true;

	if (di->ept_type & CPS7181_TX_EPT_SRC_CEP_TIMEOUT) {
		di->ept_type &= ~CPS7181_TX_EPT_SRC_CEP_TIMEOUT;
		hwlog_info("[check_rx_disconnect] rx disconnect\n");
		return true;
	}

	return false;
}

static int cps7181_tx_get_ping_interval(u16 *ping_interval, void *dev_data)
{
	return cps7181_read_word(dev_data, CPS7181_TX_PING_INTERVAL_ADDR, ping_interval);
}

static int cps7181_tx_set_ping_interval(u16 ping_interval, void *dev_data)
{
	if ((ping_interval < CPS7181_TX_PING_INTERVAL_MIN) ||
		(ping_interval > CPS7181_TX_PING_INTERVAL_MAX)) {
		hwlog_err("set_ping_interval: para out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	return cps7181_write_word(dev_data, CPS7181_TX_PING_INTERVAL_ADDR, ping_interval);
}

static int cps7181_tx_get_ping_freq(u16 *ping_freq, void *dev_data)
{
	return cps7181_read_byte(dev_data, CPS7181_TX_PING_FREQ_ADDR, (u8 *)ping_freq);
}

static int cps7181_tx_set_ping_freq(u16 ping_freq, void *dev_data)
{
	if ((ping_freq < CPS7181_TX_PING_FREQ_MIN) ||
		(ping_freq > CPS7181_TX_PING_FREQ_MAX)) {
		hwlog_err("set_ping_frequency: para out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	return cps7181_write_byte(dev_data, CPS7181_TX_PING_FREQ_ADDR, (u8)ping_freq);
}

static int cps7181_tx_get_min_fop(u16 *fop, void *dev_data)
{
	return cps7181_read_byte(dev_data, CPS7181_TX_MIN_FOP_ADDR, (u8 *)fop);
}

static int cps7181_tx_set_min_fop(u16 fop, void *dev_data)
{
	if ((fop < CPS7181_TX_MIN_FOP) || (fop > CPS7181_TX_MAX_FOP)) {
		hwlog_err("set_min_fop: para out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	return cps7181_write_byte(dev_data, CPS7181_TX_MIN_FOP_ADDR, (u8)fop);
}

static int cps7181_tx_get_max_fop(u16 *fop, void *dev_data)
{
	return cps7181_read_byte(dev_data, CPS7181_TX_MAX_FOP_ADDR, (u8 *)fop);
}

static int cps7181_tx_set_max_fop(u16 fop, void *dev_data)
{
	if ((fop < CPS7181_TX_MIN_FOP) || (fop > CPS7181_TX_MAX_FOP)) {
		hwlog_err("set_max_fop: para out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	return cps7181_write_byte(dev_data, CPS7181_TX_MAX_FOP_ADDR, (u8)fop);
}

static int cps7181_tx_get_fop(u16 *fop, void *dev_data)
{
	return cps7181_read_word(dev_data, CPS7181_TX_OP_FREQ_ADDR, fop);
}

static int cps7181_tx_get_cep(s8 *cep, void *dev_data)
{
	return cps7181_read_byte(dev_data, CPS7181_TX_CEP_ADDR, cep);
}

static int cps7181_tx_get_duty(u8 *duty, void *dev_data)
{
	return cps7181_read_byte(dev_data, CPS7181_TX_PWM_DUTY_ADDR, duty);
}

static int cps7181_tx_get_ptx(u32 *ptx, void *dev_data)
{
	return cps7181_read_word(dev_data, CPS7181_TX_PTX_ADDR, (u16 *)ptx);
}

static int cps7181_tx_get_prx(u32 *prx, void *dev_data)
{
	return cps7181_read_word(dev_data, CPS7181_TX_PRX_ADDR, (u16 *)prx);
}

static int cps7181_tx_get_ploss(s32 *ploss, void *dev_data)
{
	s16 val = 0;

	if (cps7181_read_word(dev_data, CPS7181_TX_PLOSS_ADDR, &val))
		return -EIO;

	*ploss = val;
	return 0;
}

static int cps7181_tx_get_ploss_id(u8 *id, void *dev_data)
{
	return cps7181_read_byte(dev_data, CPS7181_TX_FOD_TH_ID_ADDR, id);
}

static int cps7181_tx_get_temp(s16 *chip_temp, void *dev_data)
{
	return cps7181_read_word(dev_data, CPS7181_TX_CHIP_TEMP_ADDR, chip_temp);
}

static int cps7181_tx_get_vin(u16 *tx_vin, void *dev_data)
{
	return cps7181_read_word(dev_data, CPS7181_TX_VIN_ADDR, tx_vin);
}

static int cps7181_tx_get_vrect(u16 *tx_vrect, void *dev_data)
{
	return cps7181_read_word(dev_data, CPS7181_TX_VRECT_ADDR, tx_vrect);
}

static int cps7181_tx_get_iin(u16 *tx_iin, void *dev_data)
{
	return cps7181_read_word(dev_data, CPS7181_TX_IIN_ADDR, tx_iin);
}

static int cps7181_tx_set_ilimit(u16 tx_ilim, void *dev_data)
{
	int ret;

	if ((tx_ilim < CPS7181_TX_ILIM_MIN) ||
		(tx_ilim > CPS7181_TX_ILIM_MAX)) {
		hwlog_err("set_ilimit: out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	ret = cps7181_write_word(dev_data, CPS7181_TX_ILIM_ADDR, tx_ilim);
	if (ret) {
		hwlog_err("set_ilimit: failed\n");
		return ret;
	}

	return 0;
}

static int cps7181_tx_set_fod_coef(u16 pl_th, u8 pl_cnt, void *dev_data)
{
	int ret;
	struct cps7181_dev_info *di = dev_data;

	/* tx ploss threshold 0:disabled */
	ret = cps7181_write_word(di, CPS7181_TX_PLOSS_V0_TH, pl_th);
	ret += cps7181_write_word(di, CPS7181_TX_PLOSS_V1_TH, pl_th);
	ret += cps7181_write_word(di, CPS7181_TX_PLOSS_V2_TH, pl_th);
	/* tx ploss fod debounce count 0:no debounce */
	ret += cps7181_write_byte(di, CPS7181_TX_PLOSS_CNT_ADDR, pl_cnt);
	if (ret)
		hwlog_err("tx set fod coef failed\n");

	return ret;
}

static int cps7181_tx_set_rp_dm_timeout_val(u8 val, void *dev_data)
{
	return 0;
}

static int cps7181_tx_stop_config(void *dev_data)
{
	return 0;
}

static int cps7181_tx_chip_init(unsigned int client, void *dev_data)
{
	int ret;
	struct cps7181_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	di->irq_cnt = 0;
	di->g_val.irq_abnormal_flag = false;
	cps7181_enable_irq(di);

	ret = cps7181_write_byte_mask(di, CPS7181_TX_IRQ_EN_ADDR,
		CPS7181_TX_IRQ_EN_RPP_TIMEOUT, CPS7181_TX_IRQ_EN_RPP_SHIFT, 0);
	ret += cps7181_write_word(di, CPS7181_TX_OCP_TH_ADDR, CPS7181_TX_OCP_TH);
	ret += cps7181_write_word(di, CPS7181_TX_OVP_TH_ADDR, CPS7181_TX_OVP_TH);
	ret += cps7181_write_word(di, CPS7181_TX_PING_OCP_ADDR, CPS7181_TX_PING_OCP_TH);
	ret += cps7181_tx_set_ping_freq(CPS7181_TX_PING_FREQ, di);
	ret += cps7181_tx_set_min_fop(CPS7181_TX_MIN_FOP, di);
	ret += cps7181_tx_set_max_fop(di->tx_max_fop, di);
	ret += cps7181_tx_set_ping_interval(di->tx_ping_interval, di);
	if (ret) {
		hwlog_err("chip_init: write failed\n");
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static int cps7181_tx_enable_tx_mode(bool enable, void *dev_data)
{
	int ret;
	struct cps7181_dev_info *di = dev_data;

	if (enable)
		ret = cps7181_write_byte_mask(di, CPS7181_TX_CMD_ADDR,
			CPS7181_TX_CMD_EN_TX, CPS7181_TX_CMD_EN_TX_SHIFT,
			CPS7181_TX_CMD_VAL);
	else
		ret = cps7181_write_byte_mask(di, CPS7181_TX_CMD_ADDR,
			CPS7181_TX_CMD_DIS_TX, CPS7181_TX_CMD_DIS_TX_SHIFT,
			CPS7181_TX_CMD_VAL);

	if (ret) {
		hwlog_err("%s tx_mode failed\n", enable ? "enable" : "disable");
		return ret;
	}

	return 0;
}

static void cps7181_tx_show_ept_type(u16 ept)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_cps7181_tx_ept_name); i++) {
		if (ept & BIT(i))
			hwlog_info("[tx_ept] %s\n", g_cps7181_tx_ept_name[i]);
	}
}

static int cps7181_tx_get_ept_type(struct cps7181_dev_info *di, u16 *ept)
{
	int ret;
	u16 ept_value = 0;

	if (!ept) {
		hwlog_err("get_ept_type: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = cps7181_read_word(di, CPS7181_TX_EPT_SRC_ADDR, &ept_value);
	if (ret) {
		hwlog_err("get_ept_type: read failed\n");
		return ret;
	}
	*ept = ept_value;
	hwlog_info("[get_ept_type] type=0x%02x", *ept);
	cps7181_tx_show_ept_type(*ept);

	return 0;
}

static int cps7181_tx_clear_ept_src(struct cps7181_dev_info *di)
{
	int ret;

	ret = cps7181_write_word(di, CPS7181_TX_EPT_SRC_ADDR, CPS7181_TX_EPT_SRC_CLEAR);
	if (ret) {
		hwlog_err("clear_ept_src: failed\n");
		return ret;
	}

	hwlog_info("[clear_ept_src] success\n");
	return 0;
}

static void cps7181_tx_ept_handler(struct cps7181_dev_info *di)
{
	int ret;

	ret = cps7181_tx_get_ept_type(di, &di->ept_type);
	ret += cps7181_tx_clear_ept_src(di);
	if (ret)
		return;

	switch (di->ept_type) {
	case CPS7181_TX_EPT_SRC_CEP_TIMEOUT:
	case CPS7181_TX_EPT_SRC_RX_EPT:
		di->ept_type &= ~(CPS7181_TX_EPT_SRC_CEP_TIMEOUT |
			CPS7181_TX_EPT_SRC_RX_EPT);
		power_event_bnc_notify(cps7181_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_CEP_TIMEOUT, NULL);
		break;
	case CPS7181_TX_EPT_SRC_POCP:
	case CPS7181_TX_EPT_SRC_FOD:
		di->ept_type &= ~(CPS7181_TX_EPT_SRC_POCP | CPS7181_TX_EPT_SRC_FOD);
		power_event_bnc_notify(cps7181_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_TX_FOD, NULL);
		break;
	case CPS7181_TX_EPT_SRC_OCP:
		di->ept_type &= ~CPS7181_TX_EPT_SRC_OCP;
		power_event_bnc_notify(cps7181_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_OCP, NULL);
		break;
	case CPS7181_TX_EPT_SRC_OVP:
		di->ept_type &= ~CPS7181_TX_EPT_SRC_OVP;
		power_event_bnc_notify(cps7181_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_OVP, NULL);
		break;
	default:
		break;
	}
}

static int cps7181_tx_clear_irq(struct cps7181_dev_info *di, u16 itr)
{
	return cps7181_write_word(di, CPS7181_TX_IRQ_CLR_ADDR, itr);
}

static void cps7181_tx_ask_pkt_handler(struct cps7181_dev_info *di)
{
	if (di->irq_val & CPS7181_TX_IRQ_SS_PKG_RCVD) {
		di->irq_val &= ~CPS7181_TX_IRQ_SS_PKG_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
	}

	if (di->irq_val & CPS7181_TX_IRQ_ID_PKT_RCVD) {
		di->irq_val &= ~CPS7181_TX_IRQ_ID_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
	}

	if (di->irq_val & CPS7181_TX_IRQ_CFG_PKT_RCVD) {
		di->irq_val &= ~CPS7181_TX_IRQ_CFG_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_qi_ask_pkt(di);
		power_event_bnc_notify(cps7181_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_GET_CFG, NULL);
	}

	if (di->irq_val & CPS7181_TX_IRQ_PP_PKT_RCVD) {
		di->irq_val &= ~CPS7181_TX_IRQ_PP_PKT_RCVD;
		if (di->g_val.qi_hdl && di->g_val.qi_hdl->hdl_non_qi_ask_pkt)
			di->g_val.qi_hdl->hdl_non_qi_ask_pkt(di);
	}
}

static void cps7181_tx_show_irq(u32 intr)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_cps7181_tx_irq_name); i++) {
		if (intr & BIT(i))
			hwlog_info("[tx_irq] %s\n", g_cps7181_tx_irq_name[i]);
	}
}

static int cps7181_tx_get_interrupt(struct cps7181_dev_info *di, u16 *intr)
{
	int ret;

	ret = cps7181_read_word(di, CPS7181_TX_IRQ_ADDR, intr);
	if (ret)
		return ret;

	hwlog_info("[get_interrupt] irq=0x%04x\n", *intr);
	cps7181_tx_show_irq(*intr);

	return 0;
}

static void cps7181_tx_mode_irq_recheck(struct cps7181_dev_info *di)
{
	int ret;
	u16 irq_val = 0;

	if (gpio_get_value(di->gpio_int))
		return;

	hwlog_info("[tx_mode_irq_recheck] gpio_int low, re-check irq\n");
	ret = cps7181_tx_get_interrupt(di, &irq_val);
	if (ret)
		return;

	cps7181_tx_clear_irq(di, CPS7181_TX_IRQ_CLR_ALL);
}

void cps7181_tx_mode_irq_handler(struct cps7181_dev_info *di)
{
	int ret;

	if (!di)
		return;

	ret = cps7181_tx_get_interrupt(di, &di->irq_val);
	if (ret) {
		hwlog_err("irq_handler: get irq failed, clear\n");
		cps7181_tx_clear_irq(di, CPS7181_TX_IRQ_CLR_ALL);
		goto rechk_irq;
	}

	cps7181_tx_clear_irq(di, di->irq_val);

	cps7181_tx_ask_pkt_handler(di);

	if (di->irq_val & CPS7181_TX_IRQ_START_PING) {
		di->irq_val &= ~CPS7181_TX_IRQ_START_PING;
		power_event_bnc_notify(cps7181_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_PING_RX, NULL);
	}
	if (di->irq_val & CPS7181_TX_IRQ_EPT_PKT_RCVD) {
		di->irq_val &= ~CPS7181_TX_IRQ_EPT_PKT_RCVD;
		cps7181_tx_ept_handler(di);
	}
	if (di->irq_val & CPS7181_TX_IRQ_DPING_RCVD) {
		di->irq_val &= ~CPS7181_TX_IRQ_DPING_RCVD;
		power_event_bnc_notify(cps7181_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_RCV_DPING, NULL);
	}
	if (di->irq_val & CPS7181_TX_IRQ_RPP_TIMEOUT) {
		di->irq_val &= ~CPS7181_TX_IRQ_RPP_TIMEOUT;
		power_event_bnc_notify(cps7181_tx_get_bnt_wltx_type(di->ic_type),
			POWER_NE_WLTX_RP_DM_TIMEOUT, NULL);
	}

rechk_irq:
	cps7181_tx_mode_irq_recheck(di);
}

static int cps7181_tx_activate_chip(void *dev_data)
{
	int ret;
	struct cps7181_dev_info *di = dev_data;

	(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);
	ret = cps7181_fw_sram_i2c_init(di, CPS7181_BYTE_INC);
	if (ret)
		hwlog_err("sram i2c init failed\n");

	return 0;
}

static struct wltx_ic_ops g_cps7181_tx_ic_ops = {
	.get_dev_node           = cps7181_dts_dev_node,
	.fw_update              = cps7181_fw_sram_update,
	.chip_init              = cps7181_tx_chip_init,
	.chip_reset             = cps7181_tx_chip_reset,
	.chip_enable            = cps7181_chip_enable,
	.mode_enable            = cps7181_tx_enable_tx_mode,
	.activate_chip          = cps7181_tx_activate_chip,
	.set_open_flag          = cps7181_tx_set_tx_open_flag,
	.set_stop_cfg           = cps7181_tx_stop_config,
	.is_rx_discon           = cps7181_tx_check_rx_disconnect,
	.is_in_tx_mode          = cps7181_tx_is_tx_mode,
	.is_in_rx_mode          = cps7181_tx_is_rx_mode,
	.get_vrect              = cps7181_tx_get_vrect,
	.get_vin                = cps7181_tx_get_vin,
	.get_iin                = cps7181_tx_get_iin,
	.get_temp               = cps7181_tx_get_temp,
	.get_fop                = cps7181_tx_get_fop,
	.get_cep                = cps7181_tx_get_cep,
	.get_duty               = cps7181_tx_get_duty,
	.get_ptx                = cps7181_tx_get_ptx,
	.get_prx                = cps7181_tx_get_prx,
	.get_ploss              = cps7181_tx_get_ploss,
	.get_ploss_id           = cps7181_tx_get_ploss_id,
	.get_ping_freq          = cps7181_tx_get_ping_freq,
	.get_ping_interval      = cps7181_tx_get_ping_interval,
	.get_min_fop            = cps7181_tx_get_min_fop,
	.get_max_fop            = cps7181_tx_get_max_fop,
	.set_ping_freq          = cps7181_tx_set_ping_freq,
	.set_ping_interval      = cps7181_tx_set_ping_interval,
	.set_min_fop            = cps7181_tx_set_min_fop,
	.set_max_fop            = cps7181_tx_set_max_fop,
	.set_ilim               = cps7181_tx_set_ilimit,
	.set_vset               = cps7181_tx_set_vset,
	.set_fod_coef           = cps7181_tx_set_fod_coef,
	.set_rp_dm_to           = cps7181_tx_set_rp_dm_timeout_val,
};

int cps7181_tx_ops_register(struct wltrx_ic_ops *ops, struct cps7181_dev_info *di)
{
	if (!ops || !di)
		return -ENODEV;

	ops->tx_ops = kzalloc(sizeof(*(ops->tx_ops)), GFP_KERNEL);
	if (!ops->tx_ops)
		return -ENODEV;

	memcpy(ops->tx_ops, &g_cps7181_tx_ic_ops, sizeof(g_cps7181_tx_ic_ops));
	ops->tx_ops->dev_data = (void *)di;
	return wltx_ic_ops_register(ops->tx_ops, di->ic_type);
}
