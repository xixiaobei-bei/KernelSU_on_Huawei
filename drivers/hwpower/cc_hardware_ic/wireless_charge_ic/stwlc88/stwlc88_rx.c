/*
 * stwlc88_rx.c
 *
 * stwlc88 rx driver
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

#define HWLOG_TAG wireless_stwlc88_rx
HWLOG_REGIST();

static const char * const g_stwlc88_rx_irq_name[] = {
	/* [n]: n means bit in registers */
	[0]  = "rx_otp",
	[1]  = "rx_ocp",
	[2]  = "rx_ovp",
	[3]  = "rx_sys_err",
	[5]  = "rx_data_rcvd",
	[6]  = "rx_output_on",
	[7]  = "rx_output_off",
	[8]  = "rx_send_pkt_succ",
	[9]  = "rx_send_pkt_timeout",
	[10] = "rx_power_on",
	[11] = "rx_ready",
	[16] = "rx_dts_send_succ",
	[17] = "rx_dts_send_end_timeout",
	[18] = "rx_dts_send_end_reset",
	[20] = "rx_dts_rcvd_succ",
	[21] = "rx_dts_rcvd_end_timeout",
	[22] = "rx_dts_rcvd_end_reset",
};

static int stwlc88_rx_get_temp(int *temp, void *dev_data)
{
	s16 l_temp = 0;

	if (!temp || stwlc88_read_word(ST88_RX_CHIP_TEMP_ADDR, (u16 *)&l_temp))
		return -EINVAL;

	*temp = (l_temp / 10); /* chip_temp in 0.1degC */
	return 0;
}

static int stwlc88_rx_get_fop(int *fop, void *dev_data)
{
	return stwlc88_read_word(ST88_RX_OP_FREQ_ADDR, (u16 *)fop);
}

static int stwlc88_rx_get_cep(int *cep, void *dev_data)
{
	s8 l_cep = 0;

	if (!cep || stwlc88_read_byte(ST88_RX_CE_VAL_ADDR, (u8 *)&l_cep))
		return -EINVAL;

	*cep = l_cep;
	return 0;
}

static int stwlc88_rx_get_vrect(int *vrect, void *dev_data)
{
	return stwlc88_read_word(ST88_RX_VRECT_ADDR, (u16 *)vrect);
}

static int stwlc88_rx_get_vout(int *vout, void *dev_data)
{
	return stwlc88_read_word(ST88_RX_VOUT_ADDR, (u16 *)vout);
}

static int stwlc88_rx_get_iout(int *iout, void *dev_data)
{
	return stwlc88_read_word(ST88_RX_IOUT_ADDR, (u16 *)iout);
}

static int stwlc88_rx_get_rx_vout_reg(int *vreg, void *dev_data)
{
	int ret;

	ret = stwlc88_read_word(ST88_RX_VOUT_SET_ADDR, (u16 *)vreg);
	if (ret) {
		hwlog_err("get_rx_vout_reg: failed\n");
		return ret;
	}
	*vreg *= ST88_RX_VOUT_SET_STEP;
	return 0;
}

static int stwlc88_rx_get_vfc_reg(int *vfc_reg, void *dev_data)
{
	return stwlc88_read_word(ST88_RX_FC_VOLT_ADDR, (u16 *)vfc_reg);
}

static void stwlc88_rx_show_irq(u32 intr)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_stwlc88_rx_irq_name); i++) {
		if (intr & BIT(i))
			hwlog_info("[rx_irq] %s\n", g_stwlc88_rx_irq_name[i]);
	}
}

static int stwlc88_rx_get_interrupt(u32 *intr)
{
	int ret;

	ret = stwlc88_read_block(ST88_RX_IRQ_ADDR, (u8 *)intr, ST88_RX_IRQ_LEN);
	if (ret)
		return ret;

	hwlog_info("[get_interrupt] irq=0x%08x\n", *intr);
	stwlc88_rx_show_irq(*intr);

	return 0;
}

static int stwlc88_rx_clear_irq(u32 intr)
{
	int ret;

	ret = stwlc88_write_block(ST88_RX_IRQ_CLR_ADDR, (u8 *)&intr,
		ST88_RX_IRQ_CLR_LEN);
	if (ret) {
		hwlog_err("clear_irq: failed\n");
		return ret;
	}

	return 0;
}

static void stwlc88_sleep_enable(bool enable, void *dev_data)
{
	int gpio_val;
	struct stwlc88_dev_info *di = dev_data;

	if (!di || di->g_val.irq_abnormal_flag)
		return;

	gpio_set_value(di->gpio_sleep_en,
		enable ? WLTRX_IC_SLEEP_ENABLE : WLTRX_IC_SLEEP_DISABLE);
	gpio_val = gpio_get_value(di->gpio_sleep_en);
	hwlog_info("[sleep_enable] gpio %s now\n", gpio_val ? "high" : "low");
}

static bool stwlc88_is_sleep_enable(void *dev_data)
{
	int gpio_val;
	struct stwlc88_dev_info *di = dev_data;

	if (!di)
		return false;

	gpio_val = gpio_get_value(di->gpio_sleep_en);
	return gpio_val == WLTRX_IC_SLEEP_ENABLE;
}

static void stwlc88_rx_ext_pwr_prev_ctrl(bool flag, void *dev_data)
{
	int ret;
	u8 wr_buff;

	if (flag)
		wr_buff = ST88_RX_LDO5V_EN;
	else
		wr_buff = ST88_RX_LDO5V_DIS;

	hwlog_info("[ext_pwr_prev_ctrl] ldo_5v %s\n", flag ? "on" : "off");
	ret = stwlc88_hw_write_block(ST88_RX_LDO5V_EN_ADDR,
		&wr_buff, BYTE_LEN);
	if (ret)
		hwlog_err("ext_pwr_prev_ctrl: write reg failed\n");
}

static int stwlc88_rx_set_rx_vout(int vol, void *dev_data)
{
	int ret;

	if ((vol < ST88_RX_VOUT_MIN) || (vol > ST88_RX_VOUT_MAX)) {
		hwlog_err("set_rx_vout: out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	vol = vol / ST88_RX_VOUT_SET_STEP;
	ret = stwlc88_write_word(ST88_RX_VOUT_SET_ADDR, (u16)vol);
	if (ret) {
		hwlog_err("set_rx_vout: failed\n");
		return ret;
	}

	return 0;
}

static bool stwlc88_rx_is_cp_open(struct stwlc88_dev_info *di)
{
	int rx_ratio;
	int rx_vset = 0;
	int rx_vout = 0;
	int cp_vout;

	if (!charge_pump_is_cp_open(CP_TYPE_MAIN)) {
		hwlog_err("rx_is_cp_open: failed\n");
		return false;
	}

	rx_ratio = charge_pump_get_cp_ratio(CP_TYPE_MAIN);
	(void)stwlc88_rx_get_rx_vout_reg(&rx_vset, di);
	(void)stwlc88_rx_get_vout(&rx_vout, di);
	cp_vout = charge_pump_get_cp_vout(CP_TYPE_MAIN);
	cp_vout = (cp_vout > 0) ? cp_vout : wldc_get_ls_vbus();

	hwlog_info("[is_cp_open] [rx] ratio:%d vset:%d vout:%d [cp] vout:%d\n",
		rx_ratio, rx_vset, rx_vout, cp_vout);
	if ((cp_vout * rx_ratio) < (rx_vout - ST88_RX_FC_VOUT_ERR_LTH))
		return false;
	if ((cp_vout * rx_ratio) > (rx_vout + ST88_RX_FC_VOUT_ERR_UTH))
		return false;

	return true;
}

static int stwlc88_rx_check_cp_mode(struct stwlc88_dev_info *di)
{
	int i;
	int cnt;
	int ret;

	ret = charge_pump_set_cp_mode(CP_TYPE_MAIN);
	if (ret) {
		hwlog_err("check_cp_mode: set cp_mode failed\n");
		return ret;
	}
	cnt = ST88_RX_BPCP_TIMEOUT / ST88_RX_BPCP_SLEEP_TIME;
	for (i = 0; i < cnt; i++) {
		msleep(ST88_RX_BPCP_SLEEP_TIME);
		if (stwlc88_rx_is_cp_open(di)) {
			hwlog_info("[check_cp_mode] set cp_mode succ\n");
			return 0;
		}
		if (di->g_val.rx_stop_chrg_flag) {
			hwlog_err("check_cp_mode: wlc already stopped\n");
			return -WLC_ERR_STOP_CHRG;
		}
	}

	hwlog_err("check_cp_mode: set cp_mode timeout\n");
	return -WLC_ERR_MISMATCH;
}

static int stwlc88_rx_send_fc_cmd(int vset)
{
	int ret;

	ret = stwlc88_write_word(ST88_RX_FC_VOLT_ADDR, (u16)vset);
	if (ret) {
		hwlog_err("send_fc_cmd: set fc_reg failed\n");
		return ret;
	}
	ret = stwlc88_write_word_mask(ST88_RX_CMD_ADDR, ST88_RX_CMD_SEND_FC,
		ST88_RX_CMD_SEND_FC_SHIFT, ST88_RX_CMD_VAL);
	if (ret) {
		hwlog_err("send_fc_cmd: send fc_cmd failed\n");
		return ret;
	}

	return 0;
}

static bool stwlc88_rx_is_fc_succ(struct stwlc88_dev_info *di, int vset)
{
	int i;
	int cnt;
	int vout = 0;

	cnt = ST88_RX_FC_VOUT_TIMEOUT / ST88_RX_FC_VOUT_SLEEP_TIME;
	for (i = 0; i < cnt; i++) {
		if (di->g_val.rx_stop_chrg_flag &&
			(vset > ST88_RX_FC_VOUT_DEFAULT))
			return false;
		msleep(ST88_RX_FC_VOUT_SLEEP_TIME);
		(void)stwlc88_rx_get_vout(&vout, di);
		if ((vout >= vset - ST88_RX_FC_VOUT_ERR_LTH) &&
			(vout <= vset + ST88_RX_FC_VOUT_ERR_UTH)) {
			hwlog_info("[is_fc_succ] succ, cost_time: %dms\n",
				(i + 1) * ST88_RX_FC_VOUT_SLEEP_TIME);
			(void)stwlc88_rx_set_rx_vout(vset, di);
			return true;
		}
	}

	return false;
}

static void stwlc88_ask_mode_cfg(u8 mode_cfg)
{
	int ret;
	u8 val = 0;

	ret = stwlc88_write_byte(ST88_RX_ASK_CFG_ADDR, mode_cfg);
	if (ret)
		hwlog_err("ask_mode_cfg: write fail\n");

	ret = stwlc88_read_byte(ST88_RX_ASK_CFG_ADDR, &val);
	if (ret) {
		hwlog_err("ask_mode_cfg: read fail\n");
		return;
	}

	hwlog_info("[ask_mode_cfg] val=0x%x\n", val);
}

static void stwlc88_set_mode_cfg(struct stwlc88_dev_info *di, int vset)
{
	if (vset <= RX_HIGH_VOUT)
		stwlc88_ask_mode_cfg(di->mod_cfg.low_volt);
	else
		stwlc88_ask_mode_cfg(di->mod_cfg.high_volt);
}

static int stwlc88_rx_set_vfc(int vfc, void *dev_data)
{
	int ret;
	int i;
	struct stwlc88_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("set_vfc: di null\n");
		return -WLC_ERR_PARA_NULL;
	}

	if (vfc >= RX_HIGH_VOUT2) {
		ret = stwlc88_rx_check_cp_mode(di);
		if (ret)
			return ret;
		msleep(100); /* wait for vout stability */
	}

	for (i = 0; i < ST88_RX_FC_VOUT_RETRY_CNT; i++) {
		if (di->g_val.rx_stop_chrg_flag &&
			(vfc > ST88_RX_FC_VOUT_DEFAULT))
			return -WLC_ERR_STOP_CHRG;
		ret = stwlc88_rx_send_fc_cmd(vfc);
		if (ret) {
			hwlog_err("set_vfc: send fc_cmd failed\n");
			continue;
		}
		hwlog_info("[set_vfc] send fc_cmd, cnt: %d\n", i);
		if (stwlc88_rx_is_fc_succ(di, vfc)) {
			if (vfc < RX_HIGH_VOUT2)
				(void)charge_pump_set_bp_mode(CP_TYPE_MAIN);
			stwlc88_set_mode_cfg(di, vfc);
			hwlog_info("[set_vfc] succ\n");
			return 0;
		}
	}

	return -WLC_ERR_MISMATCH;
}

static void stwlc88_rx_send_ept(int ept_type, void *dev_data)
{
	int ret;

	switch (ept_type) {
	case WIRELESS_EPT_ERR_VRECT:
	case WIRELESS_EPT_ERR_VOUT:
		break;
	default:
		return;
	}
	ret = stwlc88_write_byte(ST88_RX_EPT_MSG_ADDR, ept_type);
	ret += stwlc88_write_word_mask(ST88_RX_CMD_ADDR, ST88_RX_CMD_SEND_EPT,
		ST88_RX_CMD_SEND_EPT_SHIFT, ST88_RX_CMD_VAL);
	if (ret)
		hwlog_err("send_ept: failed, ept=0x%x\n", ept_type);
}

bool stwlc88_rx_is_tx_exist(void *dev_data)
{
	int ret;
	u8 mode = 0;

	ret = stwlc88_get_mode(&mode);
	if (ret) {
		hwlog_err("is_tx_exist: get rx mode failed\n");
		return false;
	}
	if (mode == ST88_OP_MODE_RX)
		return true;

	hwlog_info("[is_tx_exist] mode = %d\n", mode);
	return false;
}

static int stwlc88_rx_kick_watchdog(void *dev_data)
{
	int ret;

	ret = stwlc88_write_byte(ST88_RX_WDT_FEED_ADDR, 0);
	if (ret)
		return ret;

	return 0;
}

static int stwlc88_rx_get_fod(char *fod_str, int len, void *dev_data)
{
	int i;
	int ret;
	char tmp[ST88_RX_FOD_TMP_STR_LEN] = { 0 };
	u8 fod_arr[ST88_RX_FOD_LEN] = { 0 };

	if (!fod_str || (len < WLRX_IC_FOD_COEF_LEN))
		return -EINVAL;

	ret = stwlc88_read_block(ST88_RX_FOD_ADDR, fod_arr, ST88_RX_FOD_LEN);
	if (ret) {
		hwlog_err("get_fod: read fod failed\n");
		return -EIO;
	}

	for (i = 0; i < ST88_RX_FOD_LEN; i++) {
		snprintf(tmp, ST88_RX_FOD_TMP_STR_LEN, "%x ", fod_arr[i]);
		strncat(fod_str, tmp, strlen(tmp));
	}

	return strlen(fod_str);
}

static int stwlc88_rx_set_fod(const char *fod_str, void *dev_data)
{
	int ret;
	char *cur = (char *)fod_str;
	char *token = NULL;
	int i;
	u8 val = 0;
	const char *sep = " ,";
	u8 fod_arr[ST88_RX_FOD_LEN] = { 0 };

	if (!fod_str) {
		hwlog_err("set_fod: input fod_str err\n");
		return -WLC_ERR_PARA_NULL;
	}

	for (i = 0; i < ST88_RX_FOD_LEN; i++) {
		token = strsep(&cur, sep);
		if (!token) {
			hwlog_err("set_fod: input fod_str number err\n");
			return -WLC_ERR_PARA_WRONG;
		}
		ret = kstrtou8(token, POWER_BASE_DEC, &val);
		if (ret) {
			hwlog_err("set_fod: input fod_str type err\n");
			return -WLC_ERR_PARA_WRONG;
		}
		fod_arr[i] = val;
		hwlog_info("[set_fod] fod[%d]=0x%x\n", i, fod_arr[i]);
	}

	return stwlc88_write_block(ST88_RX_FOD_ADDR, fod_arr, ST88_RX_FOD_LEN);
}

static int stwlc88_rx_init_fod_coef(struct stwlc88_dev_info *di, unsigned int tx_type)
{
	int vfc;
	int vfc_reg = 0;
	u8 *rx_fod = NULL;

	(void)stwlc88_rx_get_vfc_reg(&vfc_reg, di);
	hwlog_info("[init_fod_coef] vfc_reg: %dmV\n", vfc_reg);

	if (vfc_reg < 9000) /* (0, 9)V, set 5v fod */
		vfc = 5000;
	else if (vfc_reg < 12000) /* [9, 12)V, set 9V fod */
		vfc = 9000;
	else if (vfc_reg < 15000) /* [12, 15)V, set 12V fod */
		vfc = 12000;
	else if (vfc_reg < 18000) /* [15, 18)V, set 15V fod */
		vfc = 15000;
	else
		return -EINVAL;

	rx_fod = wlrx_get_fod_ploss_th(di->ic_type, vfc, tx_type, ST88_RX_FOD_LEN);
	if (!rx_fod)
		return -EINVAL;

	return stwlc88_write_block(ST88_RX_FOD_ADDR, rx_fod, ST88_RX_FOD_LEN);
}

static int stwlc88_rx_chip_init(unsigned int init_type, unsigned int tx_type, void *dev_data)
{
	int ret = 0;
	struct stwlc88_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	switch (init_type) {
	case WIRELESS_CHIP_INIT:
		hwlog_info("[chip_init] default chip init\n");
		di->g_val.rx_stop_chrg_flag = false;
		ret += stwlc88_write_byte(ST88_RX_FC_VRECT_DIFF_ADDR,
			ST88_RX_FC_VRECT_DIFF / ST88_RX_FC_VRECT_DIFF_STEP);
		ret += stwlc88_write_word(ST88_RX_WDT_TIMEOUT_ADDR,
			ST88_RX_WDT_TIMEOUT);
		ret += stwlc88_write_word(ST88_RX_FC_VOLT_ADDR,
			ST88_RX_FC_VOUT_DEFAULT);
		ret += stwlc88_rx_clear_irq(ST88_RX_IRQ_CLR_ALL);
		/* fall through */
	case ADAPTER_5V * WL_MVOLT_PER_VOLT:
		hwlog_info("[chip_init] 5v chip init\n");
		ret += stwlc88_write_block(ST88_RX_LDO_CFG_ADDR,
			di->rx_ldo_cfg_5v, ST88_RX_LDO_CFG_LEN);
		ret += stwlc88_rx_init_fod_coef(di, tx_type);
		stwlc88_set_mode_cfg(di, init_type);
		break;
	case ADAPTER_9V * WL_MVOLT_PER_VOLT:
		hwlog_info("[chip_init] 9v chip init\n");
		ret += stwlc88_write_block(ST88_RX_LDO_CFG_ADDR,
			di->rx_ldo_cfg_9v, ST88_RX_LDO_CFG_LEN);
		ret += stwlc88_rx_init_fod_coef(di, tx_type);
		break;
	case ADAPTER_12V * WL_MVOLT_PER_VOLT:
		hwlog_info("[chip_init] 12v chip init\n");
		ret += stwlc88_write_block(ST88_RX_LDO_CFG_ADDR,
			di->rx_ldo_cfg_12v, ST88_RX_LDO_CFG_LEN);
		ret += stwlc88_rx_init_fod_coef(di, tx_type);
		break;
	case WILREESS_SC_CHIP_INIT:
		hwlog_info("[chip_init] sc chip init\n");
		ret += stwlc88_write_block(ST88_RX_LDO_CFG_ADDR,
			di->rx_ldo_cfg_sc, ST88_RX_LDO_CFG_LEN);
		ret += stwlc88_rx_init_fod_coef(di, tx_type);
		break;
	default:
		hwlog_info("chip_init: input para invalid\n");
		break;
	}

	return ret;
}

static void stwlc88_rx_stop_charging(void *dev_data)
{
	struct stwlc88_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("stop_charging: para null\n");
		return;
	}

	di->g_val.rx_stop_chrg_flag = true;

	if (!di->g_val.irq_abnormal_flag)
		return;

	if (wlrx_get_wired_channel_state() != WIRED_CHANNEL_ON) {
		hwlog_info("[stop_charging] irq_abnormal, keep rx_sw on\n");
		di->g_val.irq_abnormal_flag = true;
		charge_pump_chip_enable(CP_TYPE_MAIN, true);
		wlps_control(di->ic_type, WLPS_RX_SW, true);
	} else {
		di->irq_cnt = 0;
		di->g_val.irq_abnormal_flag = false;
		stwlc88_enable_irq(di);
		hwlog_info("[stop_charging] wired channel on, enable irq\n");
	}
}

static int stwlc88_rx_data_rcvd_handler(struct stwlc88_dev_info *di)
{
	int ret;
	int i;
	u8 cmd;
	u8 buff[QI_PKT_LEN] = { 0 };

	ret = stwlc88_read_block(ST88_RCVD_MSG_HEADER_ADDR,
		buff, QI_PKT_LEN);
	if (ret) {
		hwlog_err("data_received_handler: read received data failed\n");
		return ret;
	}

	cmd = buff[QI_PKT_CMD];
	hwlog_info("[data_received_handler] cmd: 0x%x\n", cmd);
	for (i = QI_PKT_DATA; i < QI_PKT_LEN; i++)
		hwlog_info("[data_received_handler] data: 0x%x\n", buff[i]);

	switch (cmd) {
	case QI_CMD_TX_ALARM:
	case QI_CMD_ACK_BST_ERR:
		di->irq_val &= ~ST88_RX_IRQ_DATA_RCVD;
		if (di->g_val.qi_hdl &&
			di->g_val.qi_hdl->hdl_non_qi_fsk_pkt)
			di->g_val.qi_hdl->hdl_non_qi_fsk_pkt(buff, QI_PKT_LEN, di);
		break;
	default:
		break;
	}
	return 0;
}

void stwlc88_rx_abnormal_irq_handler(struct stwlc88_dev_info *di)
{
	static struct timespec64 ts64_timeout;
	struct timespec64 ts64_interval;
	struct timespec64 ts64_now;

	ts64_now = current_kernel_time64();
	ts64_interval.tv_sec = 0;
	ts64_interval.tv_nsec = WIRELESS_INT_TIMEOUT_TH * NSEC_PER_MSEC;

	if (!di)
		return;

	hwlog_info("[handle_abnormal_irq] irq_cnt = %d\n", ++di->irq_cnt);
	/* power on irq occurs first time, so start monitor now */
	if (di->irq_cnt == 1) {
		ts64_timeout = timespec64_add_safe(ts64_now, ts64_interval);
		if (ts64_timeout.tv_sec == TIME_T_MAX) {
			di->irq_cnt = 0;
			hwlog_err("handle_abnormal_irq: time overflow\n");
			return;
		}
	}

	if (timespec64_compare(&ts64_now, &ts64_timeout) < 0)
		return;

	if (di->irq_cnt < WIRELESS_INT_CNT_TH) {
		di->irq_cnt = 0;
		return;
	}

	di->g_val.irq_abnormal_flag = true;
	charge_pump_chip_enable(CP_TYPE_MAIN, true);
	wlps_control(di->ic_type, WLPS_RX_SW, true);
	stwlc88_disable_irq_nosync(di);
	gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
	hwlog_err("handle_abnormal_irq: more than %d irq in %ds, disable irq\n",
		WIRELESS_INT_CNT_TH, WIRELESS_INT_TIMEOUT_TH / WL_MSEC_PER_SEC);
}

static void stwlc88_rx_ready_handler(struct stwlc88_dev_info *di)
{
	if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON) {
		hwlog_err("rx_ready_handler: wired channel on, ignore\n");
		return;
	}

	hwlog_info("[rx_ready_handler] rx ready, goto wireless charging\n");
	di->g_val.rx_stop_chrg_flag = false;
	di->irq_cnt = 0;
	wired_chsw_set_wired_channel(WIRED_CHANNEL_ALL, WIRED_CHANNEL_CUTOFF);
	charge_pump_chip_enable(CP_TYPE_MAIN, true);
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, true);
	msleep(CHANNEL_SW_TIME);
	gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
	power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_READY, NULL);
}

static void stwlc88_rx_power_on_handler(struct stwlc88_dev_info *di)
{
	u8 rx_ss = 0; /* ss: signal strength */
	int pwr_flag = RX_PWR_ON_NOT_GOOD;

	if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON) {
		hwlog_err("rx_power_on_handler: wired channel on, ignore\n");
		return;
	}

	stwlc88_rx_abnormal_irq_handler(di);
	stwlc88_read_byte(ST88_RX_SS_ADDR, &rx_ss);
	hwlog_info("[rx_power_on_handler] ss=%u\n", rx_ss);
	if ((rx_ss > di->rx_ss_good_lth) && (rx_ss <= ST88_RX_SS_MAX))
		pwr_flag = RX_PWR_ON_GOOD;
	power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_PWR_ON, &pwr_flag);
}

static void stwlc88_rx_mode_irq_recheck(struct stwlc88_dev_info *di)
{
	int ret;
	u32 irq_val = 0;

	if (gpio_get_value(di->gpio_int))
		return;

	hwlog_info("[rx_mode_irq_recheck] gpio_int low, re-check irq\n");
	ret = stwlc88_rx_get_interrupt(&irq_val);
	if (ret)
		return;

	if (irq_val & ST88_RX_IRQ_READY)
		stwlc88_rx_ready_handler(di);

	stwlc88_rx_clear_irq(ST88_RX_IRQ_CLR_ALL);
}

static void stwlc88_rx_fault_irq_handler(struct stwlc88_dev_info *di)
{
	if (di->irq_val & ST88_RX_IRQ_OCP) {
		di->irq_val &= ~ST88_RX_IRQ_OCP;
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_OCP, NULL);
	}

	if (di->irq_val & ST88_RX_IRQ_OVP) {
		di->irq_val &= ~ST88_RX_IRQ_OVP;
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_OVP, NULL);
	}

	if (di->irq_val & ST88_RX_IRQ_OTP) {
		di->irq_val &= ~ST88_RX_IRQ_OTP;
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_OTP, NULL);
	}
}

void stwlc88_rx_mode_irq_handler(struct stwlc88_dev_info *di)
{
	int ret;

	if (!di)
		return;

	ret = stwlc88_rx_get_interrupt(&di->irq_val);
	if (ret) {
		hwlog_err("irq_handler: read irq failed, clear\n");
		stwlc88_rx_clear_irq(ST88_RX_IRQ_CLR_ALL);
		stwlc88_rx_abnormal_irq_handler(di);
		goto rechk_irq;
	}

	stwlc88_rx_clear_irq(di->irq_val);

	if (di->irq_val & ST88_RX_IRQ_POWER_ON) {
		di->irq_val &= ~ST88_RX_IRQ_POWER_ON;
		stwlc88_rx_power_on_handler(di);
	}
	if (di->irq_val & ST88_RX_IRQ_READY) {
		di->irq_val &= ~ST88_RX_IRQ_READY;
		stwlc88_rx_ready_handler(di);
	}
	if (di->irq_val & ST88_RX_IRQ_DATA_RCVD)
		stwlc88_rx_data_rcvd_handler(di);

	stwlc88_rx_fault_irq_handler(di);

rechk_irq:
	stwlc88_rx_mode_irq_recheck(di);
}

static void stwlc88_rx_pmic_vbus_handler(bool vbus_state, void *dev_data)
{
	int ret;
	u32 irq_val = 0;
	struct stwlc88_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("pmic_vbus_handler: di null\n");
		return;
	}

	if (!vbus_state || !di->g_val.irq_abnormal_flag)
		return;

	if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON)
		return;

	if (!stwlc88_rx_is_tx_exist(dev_data))
		return;

	ret = stwlc88_rx_get_interrupt(&irq_val);
	if (ret) {
		hwlog_err("pmic_vbus_handler: read irq failed, clear\n");
		return;
	}
	hwlog_info("[pmic_vbus_handler] irq_val=0x%x\n", irq_val);
	if (irq_val & ST88_RX_IRQ_READY) {
		stwlc88_rx_clear_irq(ST88_RX_IRQ_CLR_ALL);
		stwlc88_rx_ready_handler(di);
		di->irq_cnt = 0;
		di->g_val.irq_abnormal_flag = false;
		stwlc88_enable_irq(di);
	}
}

void stwlc88_rx_probe_check_tx_exist(struct stwlc88_dev_info *di)
{
	if (!di)
		return;

	if (stwlc88_rx_is_tx_exist(di)) {
		stwlc88_rx_clear_irq(ST88_RX_IRQ_CLR_ALL);
		hwlog_info("[rx_probe_check_tx_exist] rx exsit\n");
		stwlc88_rx_ready_handler(di);
	} else {
		stwlc88_sleep_enable(true, di);
	}
}

void stwlc88_rx_shutdown_handler(struct stwlc88_dev_info *di)
{
	if (!di)
		return;

	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
	msleep(50); /* dalay 50ms for power off */
	(void)stwlc88_rx_set_vfc(ADAPTER_5V * WL_MVOLT_PER_VOLT, di);
	(void)stwlc88_rx_set_rx_vout(ADAPTER_5V * WL_MVOLT_PER_VOLT, di);
	stwlc88_chip_enable(false, di);
	msleep(ST88_SHUTDOWN_SLEEP_TIME);
	stwlc88_chip_enable(true, di);
}

static struct wlrx_ic_ops g_stwlc88_rx_ic_ops = {
	.get_dev_node           = stwlc88_dts_dev_node,
	.fw_update              = stwlc88_fw_sram_update,
	.chip_init              = stwlc88_rx_chip_init,
	.chip_reset             = stwlc88_chip_reset,
	.chip_enable            = stwlc88_chip_enable,
	.is_chip_enable         = stwlc88_is_chip_enable,
	.sleep_enable           = stwlc88_sleep_enable,
	.is_sleep_enable        = stwlc88_is_sleep_enable,
	.ext_pwr_prev_ctrl      = stwlc88_rx_ext_pwr_prev_ctrl,
	.get_chip_info          = stwlc88_get_chip_info_str,
	.get_vrect              = stwlc88_rx_get_vrect,
	.get_vout               = stwlc88_rx_get_vout,
	.get_iout               = stwlc88_rx_get_iout,
	.get_vout_reg           = stwlc88_rx_get_rx_vout_reg,
	.get_vfc_reg            = stwlc88_rx_get_vfc_reg,
	.set_vfc                = stwlc88_rx_set_vfc,
	.set_vout               = stwlc88_rx_set_rx_vout,
	.get_fop                = stwlc88_rx_get_fop,
	.get_cep                = stwlc88_rx_get_cep,
	.get_temp               = stwlc88_rx_get_temp,
	.get_fod_coef           = stwlc88_rx_get_fod,
	.set_fod_coef           = stwlc88_rx_set_fod,
	.is_tx_exist            = stwlc88_rx_is_tx_exist,
	.kick_watchdog          = stwlc88_rx_kick_watchdog,
	.send_ept               = stwlc88_rx_send_ept,
	.stop_charging          = stwlc88_rx_stop_charging,
	.pmic_vbus_handler      = stwlc88_rx_pmic_vbus_handler,
	.need_chk_pu_shell      = NULL,
	.set_pu_shell_flag      = NULL,
};

int stwlc88_rx_ops_register(struct stwlc88_dev_info *di)
{
	if (!di)
		return -ENODEV;

	g_stwlc88_rx_ic_ops.dev_data = (void *)di;

	return wlrx_ic_ops_register(&g_stwlc88_rx_ic_ops, di->ic_type);
}
