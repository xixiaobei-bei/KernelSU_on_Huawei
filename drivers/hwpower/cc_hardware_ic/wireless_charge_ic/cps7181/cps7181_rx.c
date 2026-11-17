/*
 * cps7181_rx.c
 *
 * cps7181 rx driver
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

#define HWLOG_TAG wireless_cps7181_rx
HWLOG_REGIST();

static const char * const g_cps7181_rx_irq_name[] = {
	/* [n]: n means bit in registers */
	[0]  = "rx_power_on",
	[1]  = "rx_ldo_off",
	[2]  = "rx_ldo_on",
	[3]  = "rx_ready",
	[4]  = "rx_fsk_ack",
	[5]  = "rx_fsk_timeout",
	[6]  = "rx_fsk_pkt",
	[7]  = "rx_vrect_ovp",
	[8]  = "rx_otp",
	[9]  = "rx_ocp",
	[10] = "rx_hocp",
	[11] = "rx_scp",
	[12] = "rx_mldo_ovp",
};

static int cps7181_rx_get_temp(int *temp, void *dev_data)
{
	s16 l_temp = 0;

	if (!temp || cps7181_read_word(dev_data, CPS7181_RX_CHIP_TEMP_ADDR, (u16 *)&l_temp))
		return -EINVAL;

	*temp = l_temp;
	return 0;
}

static int cps7181_rx_get_fop(int *fop, void *dev_data)
{
	return cps7181_read_word(dev_data, CPS7181_RX_OP_FREQ_ADDR, (u16 *)fop);
}

static int cps7181_rx_get_cep(int *cep, void *dev_data)
{
	s8 l_cep = 0;

	if (!cep || cps7181_read_byte(dev_data, CPS7181_RX_CE_VAL_ADDR, (u8 *)&l_cep))
		return -EINVAL;

	*cep = l_cep;
	return 0;
}

static int cps7181_rx_get_vrect(int *vrect, void *dev_data)
{
	return cps7181_read_word(dev_data, CPS7181_RX_VRECT_ADDR, (u16 *)vrect);
}

static int cps7181_rx_get_vout(int *vout, void *dev_data)
{
	return cps7181_read_word(dev_data, CPS7181_RX_VOUT_ADDR, (u16 *)vout);
}

static int cps7181_rx_get_iout(int *iout, void *dev_data)
{
	return cps7181_read_word(dev_data, CPS7181_RX_IOUT_ADDR, (u16 *)iout);
}

static int cps7181_rx_get_rx_vout_reg(int *vreg, void *dev_data)
{
	return cps7181_read_word(dev_data, CPS7181_RX_VOUT_SET_ADDR, (u16 *)vreg);
}

static int cps7181_rx_get_vfc_reg(int *vfc_reg, void *dev_data)
{
	return cps7181_read_word(dev_data, CPS7181_RX_FC_VOLT_ADDR, (u16 *)vfc_reg);
}

static void cps7181_rx_show_irq(u16 intr)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_cps7181_rx_irq_name); i++) {
		if (intr & BIT(i))
			hwlog_info("[rx_irq] %s\n", g_cps7181_rx_irq_name[i]);
	}
}

static int cps7181_rx_get_interrupt(struct cps7181_dev_info *di, u16 *intr)
{
	int ret;

	ret = cps7181_read_word(di, CPS7181_RX_IRQ_ADDR, intr);
	if (ret)
		return ret;

	hwlog_info("[get_interrupt] irq=0x%04x\n", *intr);
	cps7181_rx_show_irq(*intr);

	return 0;
}

static int cps7181_rx_clear_irq(struct cps7181_dev_info *di, u16 intr)
{
	return cps7181_write_word(di, CPS7181_RX_IRQ_CLR_ADDR, intr);
}

static void cps7181_sleep_enable(bool enable, void *dev_data)
{
	int gpio_val;
	struct cps7181_dev_info *di = dev_data;

	if (!di || di->g_val.irq_abnormal_flag)
		return;

	if (di->ic_type == WLTRX_IC_AUX)
		return;

	gpio_set_value(di->gpio_sleep_en,
		enable ? WLTRX_IC_SLEEP_ENABLE : WLTRX_IC_SLEEP_DISABLE);
	gpio_val = gpio_get_value(di->gpio_sleep_en);
	hwlog_info("[sleep_enable] gpio %s now\n", gpio_val ? "high" : "low");
}

static bool cps7181_is_sleep_enable(void *dev_data)
{
	int gpio_val;
	struct cps7181_dev_info *di = dev_data;

	if (!di)
		return false;

	gpio_val = gpio_get_value(di->gpio_sleep_en);
	return gpio_val == WLTRX_IC_SLEEP_ENABLE;
}

static void cps7181_rx_chip_reset(void *dev_data)
{
	int ret;
	struct cps7181_dev_info *di = dev_data;

	ret = cps7181_fw_sram_i2c_init(di, CPS7181_BYTE_INC);
	if (ret) {
		hwlog_err("chip_reset: i2c init failed\n");
		return;
	}
	ret = cps7181_write_byte_mask(di, CPS7181_RX_CMD_ADDR,
		CPS7181_RX_CMD_SYS_RST, CPS7181_RX_CMD_SYS_RST_SHIFT,
		CPS7181_RX_CMD_VAL);
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

static void cps7181_rx_ext_pwr_prev_ctrl(bool flag, void *dev_data)
{
}

static void cps7181_ask_mode_cfg(struct cps7181_dev_info *di, u16 val, int polarity)
{
	int ret;
	u16 func_val = 0;

	ret = cps7181_write_word_mask(di, CPS7181_RX_FUNC_EN_ADDR,
		CPS7181_RX_CMALL_EN_MASK, CPS7181_RX_CMALL_EN_SHIFT, val);
	ret += cps7181_write_word_mask(di, CPS7181_RX_FUNC_EN_ADDR,
		CPS7181_RX_CM_POLARITY_MASK, CPS7181_RX_CM_POLARITY_SHIFT, polarity);
	ret += cps7181_read_word(di, CPS7181_RX_FUNC_EN_ADDR, &func_val);
	if (ret) {
		hwlog_err("ask_mode_cfg: write fail\n");
		return;
	}

	hwlog_info("[FUNC_EN] func_val=0x%x\n", func_val);
}

static void cps7181_set_mode_cfg(struct cps7181_dev_info *di, int vset)
{
	if (vset <= RX_HIGH_VOUT)
		cps7181_ask_mode_cfg(di, CPS7181_RX_CMALL_EN_VAL,
			CPS7181_RX_CM_POSITIVE);
	else if (!power_cmdline_is_factory_mode())
		cps7181_ask_mode_cfg(di, CPS7181_RX_CMAB_EN_VAL,
			CPS7181_RX_CM_NEGTIVE);
	else
		cps7181_ask_mode_cfg(di, CPS7181_RX_CMALL_EN_VAL,
			CPS7181_RX_CM_POSITIVE);
}

static int cps7181_rx_set_rx_vout(int vol, void *dev_data)
{
	int ret;

	if ((vol < CPS7181_RX_VOUT_MIN) || (vol > CPS7181_RX_VOUT_MAX)) {
		hwlog_err("set_rx_vout: out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	ret = cps7181_write_word(dev_data, CPS7181_RX_VOUT_SET_ADDR, (u16)vol);
	if (ret) {
		hwlog_err("set_rx_vout: failed\n");
		return ret;
	}

	return 0;
}

static bool cps7181_rx_is_cp_open(struct cps7181_dev_info *di)
{
	int rx_ratio;
	int rx_vset = 0;
	int rx_vout = 0;
	int cp_vout;

	if (!charge_pump_is_cp_open(CP_TYPE_MAIN))
		return false;

	rx_ratio = charge_pump_get_cp_ratio(CP_TYPE_MAIN);
	(void)cps7181_rx_get_rx_vout_reg(&rx_vset, di);
	(void)cps7181_rx_get_vout(&rx_vout, di);
	cp_vout = charge_pump_get_cp_vout(CP_TYPE_MAIN);
	cp_vout = (cp_vout > 0) ? cp_vout : wldc_get_ls_vbus();

	hwlog_info("[is_cp_open] [rx] ratio:%d vset:%d vout:%d [cp] vout:%d\n",
		rx_ratio, rx_vset, rx_vout, cp_vout);
	if ((cp_vout * rx_ratio) < (rx_vout - CPS7181_RX_FC_VOUT_ERR_LTH))
		return false;
	if ((cp_vout * rx_ratio) > (rx_vout + CPS7181_RX_FC_VOUT_ERR_UTH))
		return false;

	return true;
}

static int cps7181_rx_check_cp_mode(struct cps7181_dev_info *di)
{
	int i;
	int cnt;
	int ret;

	ret = charge_pump_set_cp_mode(CP_TYPE_MAIN);
	if (ret) {
		hwlog_err("check_cp_mode: set cp_mode failed\n");
		return ret;
	}
	cnt = CPS7181_RX_BPCP_TIMEOUT / CPS7181_RX_BPCP_SLEEP_TIME;
	for (i = 0; i < cnt; i++) {
		(void)power_msleep(CPS7181_RX_BPCP_SLEEP_TIME, 0, NULL);
		if (cps7181_rx_is_cp_open(di)) {
			hwlog_info("[check_cp_mode] set cp_mode succ\n");
			return 0;
		}
		if (di->g_val.rx_stop_chrg_flag)
			return -WLC_ERR_STOP_CHRG;
	}

	return -WLC_ERR_MISMATCH;
}

static int cps7181_rx_send_fc_cmd(struct cps7181_dev_info *di, int vset)
{
	int ret;

	ret = cps7181_write_word(di, CPS7181_RX_FC_VOLT_ADDR, (u16)vset);
	if (ret) {
		hwlog_err("send_fc_cmd: set fc_reg failed\n");
		return ret;
	}
	ret = cps7181_write_byte_mask(di, CPS7181_RX_CMD_ADDR,
		CPS7181_RX_CMD_SEND_FC, CPS7181_RX_CMD_SEND_FC_SHIFT,
		CPS7181_RX_CMD_VAL);
	if (ret) {
		hwlog_err("send_fc_cmd: send fc_cmd failed\n");
		return ret;
	}
	ret = cps7181_rx_set_rx_vout(vset, di);
	if (ret) {
		hwlog_err("send_fc_cmd: set rx_vout failed\n");
		return ret;
	}

	return 0;
}

static bool cps7181_rx_is_fc_succ(struct cps7181_dev_info *di, int vset)
{
	int i;
	int cnt;
	int vout = 0;

	cnt = CPS7181_RX_FC_VOUT_TIMEOUT / CPS7181_RX_FC_VOUT_SLEEP_TIME;
	for (i = 0; i < cnt; i++) {
		if (di->g_val.rx_stop_chrg_flag &&
			(vset > CPS7181_RX_FC_VOUT_DEFAULT))
			return false;
		(void)power_msleep(CPS7181_RX_FC_VOUT_SLEEP_TIME, 0, NULL);
		(void)cps7181_rx_get_vout(&vout, di);
		if ((vout >= (vset - CPS7181_RX_FC_VOUT_ERR_LTH)) &&
			(vout <= (vset + CPS7181_RX_FC_VOUT_ERR_UTH))) {
			hwlog_info("[is_fc_succ] succ, cost_time: %dms\n",
				(i + 1) * CPS7181_RX_FC_VOUT_SLEEP_TIME);
			return true;
		}
	}

	return false;
}

static int cps7181_rx_set_vfc(int vfc, void *dev_data)
{
	int ret;
	int i;
	struct cps7181_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("set_vfc: di null\n");
		return -WLC_ERR_PARA_NULL;
	}

	if (vfc >= RX_HIGH_VOUT2) {
		ret = cps7181_rx_check_cp_mode(di);
		if (ret)
			return ret;
	}

	cps7181_set_mode_cfg(di, vfc);

	for (i = 0; i < CPS7181_RX_FC_VOUT_RETRY_CNT; i++) {
		if (di->g_val.rx_stop_chrg_flag &&
			(vfc > CPS7181_RX_FC_VOUT_DEFAULT))
			return -WLC_ERR_STOP_CHRG;
		ret = cps7181_rx_send_fc_cmd(di, vfc);
		if (ret) {
			hwlog_err("set_vfc: send fc_cmd failed\n");
			continue;
		}
		hwlog_info("[set_vfc] send fc_cmd, cnt: %d\n", i);
		if (cps7181_rx_is_fc_succ(di, vfc)) {
			if (vfc < RX_HIGH_VOUT2)
				(void)charge_pump_set_bp_mode(CP_TYPE_MAIN);
			cps7181_set_mode_cfg(di, vfc);
			hwlog_info("[set_vfc] succ\n");
			return 0;
		}
	}

	return -WLC_ERR_MISMATCH;
}

static void cps7181_rx_send_ept(int ept_type, void *dev_data)
{
	int ret;
	struct cps7181_dev_info *di = dev_data;

	switch (ept_type) {
	case WIRELESS_EPT_ERR_VRECT:
	case WIRELESS_EPT_ERR_VOUT:
		break;
	default:
		return;
	}
	ret = cps7181_write_byte(di, CPS7181_RX_EPT_MSG_ADDR, (u8)ept_type);
	ret += cps7181_write_byte_mask(di, CPS7181_RX_CMD_ADDR,
		CPS7181_RX_CMD_SEND_EPT, CPS7181_RX_CMD_SEND_EPT_SHIFT,
		CPS7181_RX_CMD_VAL);
	if (ret)
		hwlog_err("send_ept: failed, ept=0x%x\n", ept_type);
}

static bool cps7181_rx_is_tx_exist(void *dev_data)
{
	int ret;
	u8 mode = 0;
	struct cps7181_dev_info *di = dev_data;

	ret = cps7181_get_mode(di, &mode);
	if (ret) {
		hwlog_err("check_tx_exist: get rx mode failed\n");
		return false;
	}

	return mode == CPS7181_OP_MODE_RX;
}

static int cps7181_rx_kick_watchdog(void *dev_data)
{
	return cps7181_write_word(dev_data, CPS7181_RX_WDT_TIMEOUT_ADDR,
		CPS7181_RX_WDT_TIMEOUT);
}

static int cps7181_rx_get_fod(char *fod_str, int len, void *dev_data)
{
	int i;
	int ret;
	char tmp[CPS7181_RX_FOD_TMP_STR_LEN] = { 0 };
	u8 fod_arr[CPS7181_RX_FOD_LEN] = { 0 };
	struct cps7181_dev_info *di = dev_data;

	if (!di || !fod_str || (len < WLRX_IC_FOD_COEF_LEN))
		return -EINVAL;

	ret = cps7181_read_block(di, CPS7181_RX_FOD_ADDR,
		fod_arr, CPS7181_RX_FOD_LEN);
	if (ret) {
		hwlog_err("rx_get_fod read fod failed\n");
		return -EIO;
	}

	for (i = 0; i < CPS7181_RX_FOD_LEN; i++) {
		snprintf(tmp, CPS7181_RX_FOD_TMP_STR_LEN,
			"%x ", fod_arr[i]);
		strncat(fod_str, tmp, strlen(tmp));
	}

	return strlen(fod_str);
}

static int cps7181_rx_set_fod(const char *fod_str, void *dev_data)
{
	int ret;
	char *cur = (char *)fod_str;
	char *token = NULL;
	int i;
	u8 val = 0;
	const char *sep = " ,";
	u8 fod_arr[CPS7181_RX_FOD_LEN] = { 0 };
	struct cps7181_dev_info *di = dev_data;

	if (!di || !fod_str) {
		hwlog_err("set_fod: di or input fod_str null\n");
		return -WLC_ERR_PARA_NULL;
	}

	for (i = 0; i < CPS7181_RX_FOD_LEN; i++) {
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

	return cps7181_write_block(di, CPS7181_RX_FOD_ADDR,
		fod_arr, CPS7181_RX_FOD_LEN);
}

static int cps7181_rx_init_fod_coef(struct cps7181_dev_info *di)
{
	int vfc_reg = 0;
	u8 *rx_fod = NULL;

	(void)cps7181_rx_get_vfc_reg(&vfc_reg, di);
	hwlog_info("[init_fod_coef] vfc_reg: %dmV\n", vfc_reg);

	if (vfc_reg < 9000) /* (0, 9)V, set 5v fod */
		rx_fod = di->rx_fod_5v;
	else if (vfc_reg < 12000) /* [9, 12)V, set 9V fod */
		rx_fod = di->rx_fod_9v;
	else if (vfc_reg < 15000) /* [12, 15)V, set 12V fod */
		rx_fod = di->rx_fod_12v;
	else if (vfc_reg < 18000) /* [15, 18)V, set 15V fod */
		rx_fod = di->rx_fod_15v;
	else
		return -WLC_ERR_MISMATCH;

	return cps7181_write_block(di, CPS7181_RX_FOD_ADDR,
		rx_fod, CPS7181_RX_FOD_LEN);
}

static int cps7181_rx_chip_init(unsigned int init_type, unsigned int tx_type, void *dev_data)
{
	int ret = 0;
	struct cps7181_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	switch (init_type) {
	case WIRELESS_CHIP_INIT:
		hwlog_info("[chip_init] default chip init\n");
		di->g_val.rx_stop_chrg_flag = false;
		ret += cps7181_write_byte_mask(di, CPS7181_RX_FUNC_EN_ADDR,
			CPS7181_RX_HV_WDT_EN_MASK, CPS7181_RX_HV_WDT_EN_SHIFT,
			CPS7181_RX_FUNC_EN);
		ret += cps7181_write_word(di, CPS7181_RX_WDT_TIMEOUT_ADDR,
			CPS7181_RX_WDT_TIMEOUT);
		/* fall through */
	case ADAPTER_5V * WL_MVOLT_PER_VOLT:
		hwlog_info("[chip_init] 5v chip init\n");
		ret += cps7181_write_block(di, CPS7181_RX_LDO_CFG_ADDR,
			di->rx_ldo_cfg_5v, CPS7181_RX_LDO_CFG_LEN);
		ret += cps7181_rx_init_fod_coef(di);
		cps7181_set_mode_cfg(di, init_type);
		break;
	case ADAPTER_9V * WL_MVOLT_PER_VOLT:
		hwlog_info("[chip_init] 9v chip init\n");
		ret += cps7181_write_block(di, CPS7181_RX_LDO_CFG_ADDR,
			di->rx_ldo_cfg_9v, CPS7181_RX_LDO_CFG_LEN);
		ret += cps7181_rx_init_fod_coef(di);
		break;
	case ADAPTER_12V * WL_MVOLT_PER_VOLT:
		hwlog_info("[chip_init] 12v chip init\n");
		ret += cps7181_write_block(di, CPS7181_RX_LDO_CFG_ADDR,
			di->rx_ldo_cfg_12v, CPS7181_RX_LDO_CFG_LEN);
		ret += cps7181_rx_init_fod_coef(di);
		break;
	case WILREESS_SC_CHIP_INIT:
		hwlog_info("[chip_init] sc chip init\n");
		ret += cps7181_write_block(di, CPS7181_RX_LDO_CFG_ADDR,
			di->rx_ldo_cfg_sc, CPS7181_RX_LDO_CFG_LEN);
		ret += cps7181_rx_init_fod_coef(di);
		break;
	default:
		hwlog_err("chip_init: input para invalid\n");
		break;
	}

	return ret;
}

static void cps7181_rx_stop_charging(void *dev_data)
{
	struct cps7181_dev_info *di = dev_data;

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
		wlps_control(di->ic_type, WLPS_RX_SW, true);
	} else {
		di->irq_cnt = 0;
		di->g_val.irq_abnormal_flag = false;
		cps7181_enable_irq(di);
		hwlog_info("[stop_charging] wired channel on, enable irq\n");
	}
}

static int cps7181_rx_data_rcvd_handler(struct cps7181_dev_info *di)
{
	int ret;
	int i;
	u8 cmd;
	u8 buff[QI_PKT_LEN] = { 0 };

	ret = cps7181_read_block(di, CPS7181_RCVD_MSG_HEADER_ADDR,
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
		di->irq_val &= ~CPS7181_RX_IRQ_FSK_PKT;
		if (di->g_val.qi_hdl &&
			di->g_val.qi_hdl->hdl_non_qi_fsk_pkt)
			di->g_val.qi_hdl->hdl_non_qi_fsk_pkt(buff, QI_PKT_LEN, di);
		break;
	default:
		break;
	}
	return 0;
}

void cps7181_rx_abnormal_irq_handler(struct cps7181_dev_info *di)
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
	wlps_control(di->ic_type, WLPS_RX_SW, true);
	cps7181_disable_irq_nosync(di);
	if (di->ic_type == WLTRX_IC_MAIN)
		gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
	hwlog_err("handle_abnormal_irq: more than %d irq in %ds, disable irq\n",
		WIRELESS_INT_CNT_TH, WIRELESS_INT_TIMEOUT_TH / WL_MSEC_PER_SEC);
}

static void cps7181_rx_ready_handler(struct cps7181_dev_info *di)
{
	if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON) {
		hwlog_err("rx_ready_handler: wired channel on, ignore\n");
		return;
	}

	hwlog_info("[rx_ready_handler] rx ready, goto wireless charging\n");
	di->g_val.rx_stop_chrg_flag = false;
	di->irq_cnt = 0;
	wired_chsw_set_wired_channel(WIRED_CHANNEL_ALL, WIRED_CHANNEL_CUTOFF);
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, true);
	(void)power_msleep(CHANNEL_SW_TIME, 0, NULL);
	if (di->ic_type == WLTRX_IC_MAIN)
		gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
	power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_READY, NULL);
}

static void cps7181_rx_power_on_handler(struct cps7181_dev_info *di)
{
	u8 rx_ss = 0; /* ss: signal strength */
	int pwr_flag = RX_PWR_ON_NOT_GOOD;

	if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON) {
		hwlog_err("rx_power_on_handler: wired channel on, ignore\n");
		return;
	}

	cps7181_rx_abnormal_irq_handler(di);
	cps7181_read_byte(di, CPS7181_RX_SS_ADDR, &rx_ss);
	hwlog_info("[rx_power_on_handler] ss=%u\n", rx_ss);
	if ((rx_ss > di->rx_ss_good_lth) && (rx_ss <= CPS7181_RX_SS_MAX))
		pwr_flag = RX_PWR_ON_GOOD;
	power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_PWR_ON, &pwr_flag);
}

static void cps7181_rx_mode_irq_recheck(struct cps7181_dev_info *di)
{
	int ret;
	u16 irq_val = 0;

	if (gpio_get_value(di->gpio_int))
		return;

	hwlog_info("[rx_mode_irq_recheck] gpio_int low, re-check irq\n");
	ret = cps7181_rx_get_interrupt(di, &irq_val);
	if (ret)
		return;

	if (irq_val & CPS7181_RX_IRQ_READY)
		cps7181_rx_ready_handler(di);

	cps7181_rx_clear_irq(di, CPS7181_RX_IRQ_CLR_ALL);
}

static void cps7181_rx_fault_irq_handler(struct cps7181_dev_info *di)
{
	if (di->irq_val & CPS7181_RX_IRQ_OCP) {
		di->irq_val &= ~CPS7181_RX_IRQ_OCP;
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_OCP, NULL);
	}

	if (di->irq_val & CPS7181_RX_IRQ_VRECT_OVP) {
		di->irq_val &= ~CPS7181_RX_IRQ_VRECT_OVP;
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_OVP, NULL);
	}

	if (di->irq_val & CPS7181_RX_IRQ_MLDO_OVP) {
		di->irq_val &= ~CPS7181_RX_IRQ_MLDO_OVP;
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_OVP, NULL);
	}

	if (di->irq_val & CPS7181_RX_IRQ_OTP) {
		di->irq_val &= ~CPS7181_RX_IRQ_OTP;
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_OTP, NULL);
	}
}

void cps7181_rx_mode_irq_handler(struct cps7181_dev_info *di)
{
	int ret;

	if (!di)
		return;

	ret = cps7181_rx_get_interrupt(di, &di->irq_val);
	if (ret) {
		hwlog_err("irq_handler: read irq failed, clear\n");
		cps7181_rx_clear_irq(di, CPS7181_RX_IRQ_CLR_ALL);
		cps7181_rx_abnormal_irq_handler(di);
		goto rechk_irq;
	}

	cps7181_rx_clear_irq(di, di->irq_val);

	if (di->irq_val & CPS7181_RX_IRQ_POWER_ON) {
		di->irq_val &= ~CPS7181_RX_IRQ_POWER_ON;
		cps7181_rx_power_on_handler(di);
	}
	if (di->irq_val & CPS7181_RX_IRQ_READY) {
		di->irq_val &= ~CPS7181_RX_IRQ_READY;
		cps7181_rx_ready_handler(di);
	}
	if (di->irq_val & CPS7181_RX_IRQ_FSK_PKT)
		cps7181_rx_data_rcvd_handler(di);

	cps7181_rx_fault_irq_handler(di);

rechk_irq:
	cps7181_rx_mode_irq_recheck(di);
}

static void cps7181_rx_pmic_vbus_handler(bool vbus_state, void *dev_data)
{
	int ret;
	u16 irq_val = 0;
	struct cps7181_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("pmic_vbus_handler: di null\n");
		return;
	}

	if (!vbus_state || !di->g_val.irq_abnormal_flag)
		return;

	if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON)
		return;

	if (!cps7181_rx_is_tx_exist(di))
		return;

	ret = cps7181_rx_get_interrupt(di, &irq_val);
	if (ret) {
		hwlog_err("pmic_vbus_handler: read irq failed, clear\n");
		return;
	}
	hwlog_info("[pmic_vbus_handler] irq_val=0x%x\n", irq_val);
	if (irq_val & CPS7181_RX_IRQ_READY) {
		cps7181_rx_clear_irq(di, CPS7181_RX_IRQ_CLR_ALL);
		cps7181_rx_ready_handler(di);
		di->irq_cnt = 0;
		di->g_val.irq_abnormal_flag = false;
		cps7181_enable_irq(di);
	}
}

void cps7181_rx_probe_check_tx_exist(struct cps7181_dev_info *di)
{
	if (!di)
		return;

	if (cps7181_rx_is_tx_exist(di)) {
		cps7181_rx_clear_irq(di, CPS7181_RX_IRQ_CLR_ALL);
		hwlog_info("[rx_probe_check_tx_exist] rx exsit\n");
		cps7181_rx_ready_handler(di);
	} else {
		cps7181_sleep_enable(true, di);
	}
}

void cps7181_rx_shutdown_handler(struct cps7181_dev_info *di)
{
	if (!di)
		return;

	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
	(void)power_msleep(DT_MSLEEP_50MS, 0, NULL);
	(void)cps7181_rx_set_vfc(ADAPTER_5V * WL_MVOLT_PER_VOLT, di);
	(void)cps7181_rx_set_rx_vout(ADAPTER_5V * WL_MVOLT_PER_VOLT, di);
	(void)cps7181_chip_enable(false, di);
	(void)power_msleep(CPS7181_SHUTDOWN_SLEEP_TIME, 0, NULL);
	(void)cps7181_chip_enable(true, di);
}

static struct wlrx_ic_ops g_cps7181_rx_ic_ops = {
	.get_dev_node           = cps7181_dts_dev_node,
	.fw_update              = cps7181_fw_sram_update,
	.chip_init              = cps7181_rx_chip_init,
	.chip_reset             = cps7181_rx_chip_reset,
	.chip_enable            = cps7181_chip_enable,
	.is_chip_enable         = cps7181_is_chip_enable,
	.sleep_enable           = cps7181_sleep_enable,
	.is_sleep_enable        = cps7181_is_sleep_enable,
	.ext_pwr_prev_ctrl      = cps7181_rx_ext_pwr_prev_ctrl,
	.get_chip_info          = cps7181_get_chip_info_str,
	.get_vrect              = cps7181_rx_get_vrect,
	.get_vout               = cps7181_rx_get_vout,
	.get_iout               = cps7181_rx_get_iout,
	.get_vout_reg           = cps7181_rx_get_rx_vout_reg,
	.get_vfc_reg            = cps7181_rx_get_vfc_reg,
	.set_vfc                = cps7181_rx_set_vfc,
	.set_vout               = cps7181_rx_set_rx_vout,
	.get_fop                = cps7181_rx_get_fop,
	.get_cep                = cps7181_rx_get_cep,
	.get_temp               = cps7181_rx_get_temp,
	.get_fod_coef           = cps7181_rx_get_fod,
	.set_fod_coef           = cps7181_rx_set_fod,
	.is_tx_exist            = cps7181_rx_is_tx_exist,
	.kick_watchdog          = cps7181_rx_kick_watchdog,
	.send_ept               = cps7181_rx_send_ept,
	.stop_charging          = cps7181_rx_stop_charging,
	.pmic_vbus_handler      = cps7181_rx_pmic_vbus_handler,
	.need_chk_pu_shell      = NULL,
	.set_pu_shell_flag      = NULL,
};

int cps7181_rx_ops_register(struct wltrx_ic_ops *ops, struct cps7181_dev_info *di)
{
	if (!ops || !di)
		return -ENODEV;

	ops->rx_ops = kzalloc(sizeof(*(ops->rx_ops)), GFP_KERNEL);
	if (!ops->rx_ops)
		return -ENODEV;

	memcpy(ops->rx_ops, &g_cps7181_rx_ic_ops, sizeof(g_cps7181_rx_ic_ops));
	ops->rx_ops->dev_data = (void *)di;
	return wlrx_ic_ops_register(ops->rx_ops, di->ic_type);
}
