/*
 * idtp9415.c
 *
 * idtp9415 driver
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
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>
#include <linux/jiffies.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/boost_5v.h>
#include <huawei_platform/power/charge_pump.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/wireless_power_supply.h>
#include <huawei_platform/power/wireless_charger.h>
#include <huawei_platform/power/wireless_direct_charger.h>
#include <huawei_platform/power/wireless_transmitter.h>
#include <huawei_platform/power/wired_channel_switch.h>
#include "idtp9415.h"
#include "idtp9415_mtp.h"

#define HWLOG_TAG wireless_idtp9415
HWLOG_REGIST();

static struct qi_protocol_handle *g_idtp9415_handle;
static struct idtp9415_dev_info *g_idtp9415_di;
static struct wakeup_source g_idtp9415_wakelock;
static bool g_stop_charging_flag;
static bool g_irq_abnormal_flag;
static bool g_tx_open_flag;
static bool g_mtp_chk_complete;

static const u8 g_idtp9415_rx2tx_header[IDTP9415_RX2TX_PKT_LEN] = {
	0, 0x18, 0x28, 0x38, 0x48, 0x58
};
static const u8 g_idtp9415_tx2rx_header[IDTP9415_TX2RX_PKT_LEN] = {
	0, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f
};

static bool idtp9415_is_pwr_good(void)
{
	int gpio_val;
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di)
		return false;
	if (di->gpio_pwr_good <= 0)
		return true;
	if (!g_mtp_chk_complete)
		return true;

	gpio_val = gpio_get_value(di->gpio_pwr_good);

	return gpio_val == IDTP9415_GPIO_PWR_GOOD_VAL;
}

static int idtp9415_i2c_read(struct i2c_client *client,
	u8 *cmd, int cmd_length, u8 *read_data, int read_cnt)
{
	int i;
	int ret;
	struct i2c_msg msg[I2C_RD_MSG_LEN];

	if (!client || !cmd || !read_data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	msg[0].addr = client->addr;
	msg[0].buf = cmd;
	msg[0].len = cmd_length;
	msg[0].flags = 0;

	msg[1].addr = client->addr;
	msg[1].buf = read_data;
	msg[1].len = read_cnt;
	msg[1].flags = I2C_M_RD;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!idtp9415_is_pwr_good())
			return -WLC_ERR_I2C_R;
		ret = i2c_transfer(client->adapter, msg, I2C_RD_MSG_LEN);
		if (ret == I2C_RD_MSG_LEN)
			break;
		usleep_range(9500, 10500); /* 10ms */
	}

	if (ret != I2C_RD_MSG_LEN) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	return 0;
}

static int idtp9415_i2c_write(struct i2c_client *client,
	u8 *cmd, int cmd_length)
{
	int i;
	int ret;
	struct i2c_msg msg[I2C_WR_MSG_LEN];

	if (!client || !cmd) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	msg[0].addr = client->addr;
	msg[0].buf = cmd;
	msg[0].len = cmd_length;
	msg[0].flags = 0;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!idtp9415_is_pwr_good())
			return -WLC_ERR_I2C_W;
		ret = i2c_transfer(client->adapter, msg, I2C_WR_MSG_LEN);
		if (ret == I2C_WR_MSG_LEN)
			break;
		usleep_range(9500, 10500); /* 10ms */
	}

	if (ret != I2C_WR_MSG_LEN) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static int idtp9415_read_block(struct idtp9415_dev_info *di,
	u16 reg, u8 *data, int len)
{
	u8 cmd[IDTP9415_ADDR_LEN];

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	/* double bytes addr */
	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;

	return idtp9415_i2c_read(di->client, cmd,
		IDTP9415_ADDR_LEN, data, len);
}

static int idtp9415_write_block(struct idtp9415_dev_info *di,
	u16 reg, u8 *data, int data_len)
{
	u8 cmd[IDTP9415_ADDR_LEN + data_len];

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	/* double bytes addr */
	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;
	memcpy(&cmd[IDTP9415_ADDR_LEN], data, data_len);

	return idtp9415_i2c_write(di->client, cmd,
		IDTP9415_ADDR_LEN + data_len);
}

static int idtp9415_read_byte(u16 reg, u8 *data)
{
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	return idtp9415_read_block(di, reg, data, BYTE_LEN);
}

static int idtp9415_read_word(u16 reg, u16 *data)
{
	int ret;
	u8 buff[WORD_LEN] = { 0 };
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = idtp9415_read_block(di, reg, buff, WORD_LEN);
	if (ret)
		return -WLC_ERR_I2C_R;

	/* B0: low byte; B1: high byte */
	*data = buff[0] | (buff[1] << BITS_PER_BYTE);
	return 0;
}

static int idtp9415_write_byte(u16 reg, u8 data)
{
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	return idtp9415_write_block(di, reg, &data, BYTE_LEN);
}

static int idtp9415_write_word(u16 reg, u16 data)
{
	u8 buff[WORD_LEN];
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	/* B0: low byte; B1: high byte */
	buff[0] = data & BYTE_MASK;
	buff[1] = data >> BITS_PER_BYTE;

	return idtp9415_write_block(di, reg, buff, WORD_LEN);
}

static int idtp9415_write_word_mask(u16 reg, u16 mask, u16 shift, u16 data)
{
	int ret;
	u16 val = 0;

	ret = idtp9415_read_word(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return idtp9415_write_word(reg, val);
}

/*
 * idtp9415 chip_info
 */

static int idtp9415_get_chip_id(u16 *chip_id)
{
	int ret;
	u8 chip_id_h = 0; /* high byte, shoul be 0x94 */
	u8 chip_id_l = 0; /* low byte, shoul be 0x15 */

	ret = idtp9415_read_byte(IDTP9415_CHIP_ID_H_ADDR, &chip_id_h);
	ret += idtp9415_read_byte(IDTP9415_CHIP_ID_L_ADDR, &chip_id_l);
	if (ret)
		return ret;

	*chip_id = chip_id_l | (chip_id_h << BITS_PER_BYTE);

	return 0;
}

static int idtp9415_get_major_fw_version(u16 *fw)
{
	return idtp9415_read_word(IDTP9415_MAJOR_FW_ADDR, fw);
}

static int idtp9415_get_minor_fw_version(u16 *fw)
{
	return idtp9415_read_word(IDTP9415_MINOR_FW_ADDR, fw);
}

static int idtp9415_get_chip_info(struct idtp9415_chip_info *info)
{
	int ret;

	ret = idtp9415_get_chip_id(&info->chip_id);
	if (ret)
		return ret;
	ret = idtp9415_get_major_fw_version(&info->major_fw_ver);
	if (ret)
		return ret;
	ret = idtp9415_get_minor_fw_version(&info->minor_fw_ver);
	if (ret)
		return ret;

	return 0;
}

static u8 *idtp9415_get_chip_info_str(void)
{
	int ret;
	struct idtp9415_chip_info chip_info;
	static u8 chip_info_str[IDTP9415_CHIP_INFO_STR_LEN] = { 0 };

	ret = idtp9415_get_chip_info(&chip_info);
	if (ret)
		return "error read";

	memset(chip_info_str, 0, IDTP9415_CHIP_INFO_STR_LEN);

	snprintf(chip_info_str, IDTP9415_CHIP_INFO_STR_LEN,
		"chip_id:idtp%x fw:0x%04x\n",
		chip_info.chip_id, chip_info.minor_fw_ver);

	return chip_info_str;
}

static int idtp9415_get_fw_version(u8 *data, int len)
{
	struct idtp9415_chip_info chip_info;

	if (idtp9415_get_chip_info(&chip_info)) {
		hwlog_err("%s: get chip info fail\n", __func__);
		return -1;
	}

	/* fw version length must be 4 */
	if (len != 4)
		return -1;

	/* byte[0:1]=minor_fw_ver, byte[2:3]=minor_fw_ver */
	data[0] = (u8)((chip_info.major_fw_ver >> 0) & BYTE_MASK);
	data[1] = (u8)((chip_info.major_fw_ver >> BITS_PER_BYTE) & BYTE_MASK);
	data[2] = (u8)((chip_info.minor_fw_ver >> 0) & BYTE_MASK);
	data[3] = (u8)((chip_info.minor_fw_ver >> BITS_PER_BYTE) & BYTE_MASK);

	return 0;
}

static u8 *idtp9415_get_die_id(void)
{
	return "idtp9415";
}

static int idtp9415_get_mode(u8 *mode)
{
	int ret;

	if (!mode)
		return WLC_ERR_PARA_NULL;

	ret = idtp9415_read_byte(IDTP9415_OP_MODE_ADDR, mode);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	return 0;
}

/*
 * idtp9415 rx mode
 */

static void idtp9415_wake_lock(void)
{
	if (g_idtp9415_wakelock.active) {
		hwlog_info("[%s] already locked\n", __func__);
		return;
	}

	__pm_stay_awake(&g_idtp9415_wakelock);
	hwlog_info("%s\n", __func__);
}

static void idtp9415_wake_unlock(void)
{
	if (!g_idtp9415_wakelock.active) {
		hwlog_info("[%s] already unlocked\n", __func__);
		return;
	}

	__pm_relax(&g_idtp9415_wakelock);
	hwlog_info("%s\n", __func__);
}

static void idtp9415_enable_irq(struct idtp9415_dev_info *di)
{
	mutex_lock(&di->mutex_irq);
	if (!di->irq_active) {
		hwlog_info("[%s] ++\n", __func__);
		enable_irq(di->irq_int);
		di->irq_active = true;
	}
	hwlog_info("[%s] --\n", __func__);
	mutex_unlock(&di->mutex_irq);
}

static void idtp9415_disable_irq_nosync(struct idtp9415_dev_info *di)
{
	mutex_lock(&di->mutex_irq);
	if (di->irq_active) {
		hwlog_info("[%s] ++\n", __func__);
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
	}
	hwlog_info("[%s] --\n", __func__);
	mutex_unlock(&di->mutex_irq);
}

/* chip_en: fw software control according to gpio state */
static void idtp9415_chip_enable(int enable)
{
	int gpio_val;
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di)
		return;

	if (enable == RX_EN_ENABLE)
		gpio_set_value(di->gpio_en, di->gpio_en_valid_val);
	else
		gpio_set_value(di->gpio_en, !di->gpio_en_valid_val);

	gpio_val = gpio_get_value(di->gpio_en);
	hwlog_info("[%s] gpio is %s now\n",
		__func__, gpio_val ? "high" : "low");
}

static void idtp9415_sleep_enable(int enable)
{
	int gpio_val;
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di || g_irq_abnormal_flag)
		return;

	gpio_set_value(di->gpio_sleep_en, enable);
	gpio_val = gpio_get_value(di->gpio_sleep_en);
	hwlog_info("[%s] gpio is %s now\n",
		__func__, gpio_val ? "high" : "low");
}

static int idtp9415_chip_reset(void)
{
	int ret;

	ret = idtp9415_write_byte(IDTP9415_M0_ADDR, IDTP9415_M0_RESET);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	return 0;
}

static int idtp9415_get_rx_temp(void)
{
	int ret;
	u16 temp = 0;

	ret = idtp9415_read_word(IDTP9415_RX_TEMP_ADDR, &temp);
	if (ret)
		return -WLC_ERR_I2C_R;

	/* refer to datasheet, [degC]= ADC * 75/1000 - 177 */
	temp = temp * 75 / 1000 - 177;

	return (int)temp;
}

static int idtp9415_get_rx_fop(void)
{
	int ret;
	u16 fop = 0;

	ret = idtp9415_read_word(IDTP9415_RX_FOP_ADDR, &fop);
	if (ret || (fop <= 0))
		return -WLC_ERR_I2C_R;

	/* refer to datasheet, Fop(KHz) = 64 * 6000 / value */
	fop = 64 * 6000 / fop;

	return (int)fop;
}

static int idtp9415_get_rx_cep(void)
{
	int ret;
	s16 cep = 0;

	ret = idtp9415_read_word(IDTP9415_RX_CEP_VAL_ADDR, (u16 *)&cep);
	if (ret)
		return -WLC_ERR_I2C_R;

	return (int)cep;
}

static int idtp9415_get_rx_vrect(void)
{
	int ret;
	u16 vrect = 0;

	ret = idtp9415_read_word(IDTP9415_RX_VRECT_ADDR, &vrect);
	if (ret)
		return -WLC_ERR_I2C_R;

	return (int)vrect;
}

static int idtp9415_get_rx_vout(void)
{
	int ret;
	u16 vout = 0;

	ret = idtp9415_read_word(IDTP9415_RX_VOUT_ADDR, &vout);
	if (ret)
		return -WLC_ERR_I2C_R;

	return (int)vout;
}

static int idtp9415_get_rx_iout(void)
{
	int ret;
	u16 iout = 0;

	ret = idtp9415_read_word(IDTP9415_RX_IOUT_ADDR, &iout);
	if (ret)
		return -WLC_ERR_I2C_R;

	return (int)iout;
}

static int idtp9415_get_rx_vout_reg(void)
{
	int ret;
	u16 vreg = 0;

	ret = idtp9415_read_word(IDTP9415_RX_VOUT_SET_ADDR, &vreg);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	/* refer to datasheet, vout = value * 84/10 (mV) + 2800 (mV) */
	return (int)(vreg * 84 / 10 + 2800);
}

static int idtp9415_get_tx_vout_reg(void)
{
	int ret;
	u16 tx_vreg = 0;

	ret = idtp9415_read_word(IDTP9415_RX_FC_ADDR, &tx_vreg);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	return (int)tx_vreg;
}

static int idtp9415_rx_def_max_iout(void)
{
	return IDTP9415_DFT_IOUT_MAX;
}

static int idtp9415_get_rx_interrupt(struct idtp9415_dev_info *di, u32 *intr)
{
	return idtp9415_read_block(di, IDTP9415_RX_INT_ADDR,
		(u8 *)intr, IDTP9415_RX_INT_LEN);
}

static void idtp9415_rx_clear_interrupt(struct idtp9415_dev_info *di, u32 intr)
{
	int ret;

	ret = idtp9415_write_block(di, IDTP9415_RX_INT_CLR_ADDR,
		(u8 *)&intr, IDTP9415_RX_INT_CLR_LEN);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return;
	}
	ret = idtp9415_write_word_mask(IDTP9415_RX_CMD_ADDR,
		IDTP9415_RX_CMD_CLR_INT_MASK, IDTP9415_RX_CMD_CLR_INT_SHIFT,
		IDTP9415_RX_CMD_VAL);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return;
	}
}

static int idtp9415_set_rx_vout(int vol)
{
	int ret;

	if ((vol < IDTP9415_RX_VOUT_MIN) || (vol > IDTP9415_RX_VOUT_MAX)) {
		hwlog_err("%s: out of range\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	/* refer to datasheet, vout = value * 84/10 (mV) + 2800 (mV) */
	vol = (vol - 2800) * 10 / 84;
	ret = idtp9415_write_word(IDTP9415_RX_VOUT_SET_ADDR, (u16)vol);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	return 0;
}

static bool idtp9415_is_cp_really_open(void)
{
	int rx_ratio;
	int rx_vset;
	int rx_vout;
	int cp_vout;

	if (!charge_pump_is_cp_open())
		return false;

	rx_ratio = charge_pump_get_cp_ratio();
	rx_vset = idtp9415_get_rx_vout_reg();
	rx_vout = idtp9415_get_rx_vout();
	cp_vout = charge_pump_get_cp_vout();
	cp_vout = (cp_vout > 0) ? cp_vout : wldc_get_ls_vbus();

	hwlog_info("[%s] rx_ratio:%d rx_vset:%d rx_vout:%d cp_vout:%d\n",
		__func__, rx_ratio, rx_vset, rx_vout, cp_vout);
	if ((cp_vout * rx_ratio) < (rx_vout - IDTP9415_RX_FC_ERR_LTH))
		return false;
	if ((cp_vout * rx_ratio) > (rx_vout + IDTP9415_RX_FC_ERR_UTH))
		return false;

	return true;
}

static int idtp9415_check_cp_mode(void)
{
	int i;
	int cnt;
	int ret;

	ret = charge_pump_set_cp_mode();
	if (ret) {
		hwlog_err("%s: set cp mode fail\n", __func__);
		return ret;
	}
	cnt = IDTP9415_BPCP_TIMEOUT / IDTP9415_BPCP_SLEEP_TIME;
	for (i = 0; i < cnt; i++) {
		msleep(IDTP9415_BPCP_SLEEP_TIME);
		if (idtp9415_is_cp_really_open()) {
			hwlog_info("[%s] set cp mode succ\n", __func__);
			break;
		}
		if (g_stop_charging_flag)
			return -WLC_ERR_STOP_CHRG;
	}

	if (i == cnt) {
		hwlog_err("%s: set cp mode fail\n", __func__);
		return -WLC_ERR_MISMATCH;
	}

	return 0;
}

static int idtp9415_send_fc_cmd(int vset)
{
	int ret;

	ret = idtp9415_write_word(IDTP9415_RX_FC_ADDR, (u16)vset);
	if (ret) {
		hwlog_err("%s: set fc reg fail\n", __func__);
		return ret;
	}
	ret = idtp9415_write_word_mask(IDTP9415_RX_CMD_ADDR,
		IDTP9415_RX_CMD_SEND_FC, IDTP9415_RX_CMD_SEND_FC_SHIFT,
		IDTP9415_RX_CMD_VAL);
	if (ret) {
		hwlog_err("%s: send fc cmd fail\n", __func__);
		return ret;
	}

	return 0;
}

static bool idtp9415_is_fc_succ(int vset)
{
	int i;
	int cnt;
	int vout;

	cnt = IDTP9415_RX_FC_TIMEOUT / IDTP9415_RX_FC_SLEEP_TIME;
	for (i = 0; i < cnt; i++) {
		if (g_stop_charging_flag && (vset > IDTP9415_RX_FC_DEF_VOLT))
			return false;
		msleep(IDTP9415_RX_FC_SLEEP_TIME);
		vout = idtp9415_get_rx_vout();
		if ((vout >= vset - IDTP9415_RX_FC_ERR_LTH) &&
			(vout <= vset + IDTP9415_RX_FC_ERR_UTH)) {
			hwlog_info("[%s] succ, cost_time: %dms\n", __func__,
				(i + 1) * IDTP9415_RX_FC_SLEEP_TIME);
			return true;
		}
	}

	return false;
}

static void idtp9415_ask_mode_cfg(u8 mode_cfg)
{
	int ret;
	u8 val = 0;

	ret = idtp9415_write_byte(IDTP9415_RX_COM_CFG_ADDR, mode_cfg);
	if (ret)
		hwlog_err("ask_mode_cfg: write fail\n");

	ret = idtp9415_read_byte(IDTP9415_RX_COM_CFG_ADDR, &val);
	if (ret) {
		hwlog_err("ask_mode_cfg: read fail\n");
		return;
	}

	hwlog_info("[ask_mode_cfg] val=0x%x\n", val);
}

static void idtp9415_set_mode_cfg(int vset)
{
	if (vset <= RX_HIGH_VOUT) {
		idtp9415_ask_mode_cfg(IDTP9415_COM_EN | IDTP9415_CLAMP_EN);
		return;
	}

	if (!power_cmdline_is_factory_mode())
		idtp9415_ask_mode_cfg(IDTP9415_COM_EN);
	else
		idtp9415_ask_mode_cfg(IDTP9415_COM_EN | IDTP9415_CLAMP_EN);
}

static int idtp9415_set_tx_vout(int vset)
{
	int ret;
	int i;

	if (vset >= RX_HIGH_VOUT2) {
		ret = idtp9415_check_cp_mode();
		if (ret && (ret != -ENOTSUPP))
			return ret;
	}

	idtp9415_set_mode_cfg(vset);

	for (i = 0; i < IDTP9415_RX_FC_RETRY_CNT; i++) {
		if (g_stop_charging_flag && (vset > IDTP9415_RX_FC_DEF_VOLT))
			return -WLC_ERR_STOP_CHRG;
		ret = idtp9415_send_fc_cmd(vset);
		if (ret) {
			hwlog_err("%s: send fc_cmd fail\n", __func__);
			continue;
		}
		hwlog_info("[%s] send fc cmd, cnt: %d\n", __func__, i);
		if (idtp9415_is_fc_succ(vset)) {
			if (vset < RX_HIGH_VOUT2)
				(void)charge_pump_set_bp_mode();
			idtp9415_set_mode_cfg(vset);
			hwlog_info("[%s] succ\n", __func__);
			return 0;
		}
	}

	return -WLC_ERR_MISMATCH;
}

static int idtp9415_send_msg(u8 cmd, u8 *data, int data_len)
{
	int ret;
	u8 header;
	u8 write_data[IDTP9415_RX2TX_DATA_LEN] = { 0 };
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_I2C_WR;
	}

	if ((data_len > IDTP9415_RX2TX_DATA_LEN) || (data_len < 0)) {
		hwlog_err("%s: send data number out of range\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	di->irq_val &= ~IDTP9415_RX_INT_SEND_PKT_SUCC;
	/* msg_len=cmd_len+data_len cmd_len=1 */
	header = g_idtp9415_rx2tx_header[data_len + 1];

	ret = idtp9415_write_byte(IDTP9415_RX2TX_HEADER_ADDR, header);
	if (ret) {
		hwlog_err("%s: write header fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}
	ret = idtp9415_write_byte(IDTP9415_RX2TX_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("%s: write cmd fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	if (data && (data_len > 0)) {
		memcpy(write_data, data, data_len);
		ret = idtp9415_write_block(di,
			IDTP9415_RX2TX_DATA_ADDR, write_data, data_len);
		if (ret) {
			hwlog_err("%s: write RX2TX-reg fail\n", __func__);
			return -WLC_ERR_I2C_W;
		}
	}

	ret = idtp9415_write_word_mask(IDTP9415_RX_CMD_ADDR,
		IDTP9415_RX_CMD_SEND_MSG, IDTP9415_RX_CMD_SEND_MSG_SHIFT,
		IDTP9415_RX_CMD_VAL);
	if (ret) {
		hwlog_err("%s: send RX msg to TX fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	hwlog_info("[%s] send msg[cmd:0x%x] succ\n", __func__, cmd);
	return 0;
}

static int idtp9415_send_msg_ack(u8 cmd, u8 *data, int data_len)
{
	int ret;
	int count = 0;
	int ack_cnt;
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	do {
		idtp9415_send_msg(cmd, data, data_len);
		for (ack_cnt = 0;
			ack_cnt < IDTP9415_WAIT_FOR_ACK_RETRY_CNT; ack_cnt++) {
			msleep(IDTP9415_WAIT_FOR_ACK_SLEEP_TIME);
			if (di->irq_val & IDTP9415_RX_INT_SEND_PKT_SUCC) {
				di->irq_val &= ~IDTP9415_RX_INT_SEND_PKT_SUCC;
				hwlog_info("[%s] succ, retry times = %d\n",
					__func__, count);
				return 0;
			}
			if (g_stop_charging_flag)
				return -WLC_ERR_STOP_CHRG;
		}
		count++;
		hwlog_info("[%s] retry\n", __func__);
	} while (count < IDTP9415_SNED_MSG_RETRY_CNT);

	if (count < IDTP9415_SNED_MSG_RETRY_CNT) {
		hwlog_info("[%s] succ\n", __func__);
		return 0;
	}

	ret = idtp9415_read_byte(IDTP9415_TX2RX_CMD_ADDR, &cmd);
	if (ret) {
		hwlog_err("%s: get rcv cmd data fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}
	if (cmd != QI_CMD_ACK) {
		hwlog_err("[%s] fail, ack = 0x%x, retry times = %d\n",
			__func__, cmd, count);
		return -WLC_ERR_ACK_TIMEOUT;
	}

	return 0;
}

static int idtp9415_receive_msg(u8 *data, int data_len)
{
	int ret;
	int count = 0;
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	do {
		if (di->irq_val & IDTP9415_RX_INT_TXDATA_RCVD) {
			di->irq_val &= ~IDTP9415_RX_INT_TXDATA_RCVD;
			goto func_end;
		}
		if (g_stop_charging_flag)
			return -WLC_ERR_STOP_CHRG;
		msleep(IDTP9415_RCV_MSG_SLEEP_TIME);
		count++;
	} while (count < IDTP9415_RCV_MSG_SLEEP_CNT);

func_end:
	ret = idtp9415_read_block(di, IDTP9415_TX2RX_CMD_ADDR, data, data_len);
	if (ret) {
		hwlog_err("%s: get tx to rx data fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}
	if (!data[0]) { /* data[0]: cmd */
		hwlog_err("%s: no msg received from tx\n", __func__);
		return -WLC_ERR_ACK_TIMEOUT;
	}
	hwlog_info("[%s] get tx2rx data[cmd:0x%x] succ\n", __func__, data[0]);
	return 0;
}

static int idtp9415_send_ept(enum wireless_etp_type ept_type)
{
	int ret;

	switch (ept_type) {
	case WIRELESS_EPT_ERR_VRECT:
	case WIRELESS_EPT_ERR_VOUT:
		break;
	default:
		return -WLC_ERR_PARA_WRONG;
	}
	ret = idtp9415_write_byte(IDTP9415_RX_EPT_ADDR, (u8)ept_type);
	ret += idtp9415_write_word_mask(IDTP9415_RX_CMD_ADDR,
		IDTP9415_RX_CMD_SEND_EPT, IDTP9415_RX_CMD_SEND_EPT_SHIFT,
		IDTP9415_RX_CMD_VAL);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	return 0;
}

static int idtp9415_set_rpp_format(u8 pmax)
{
	int ret;
	u8 format = 0x80; /* bit[7:6]=10: Qi RP24 mode No reply */

	format |= ((pmax * IDTP9415_RX_RPP_VAL_UNIT) &
		IDTP9415_RX_RPP_VAL_MASK);
	hwlog_info("[%s] format=0x%x\n", __func__, format);
	ret = idtp9415_write_byte(IDTP9415_RX_RPP_CTR_ADDR, format);
	ret += idtp9415_write_word_mask(IDTP9415_RX_CMD_ADDR,
		IDTP9415_RX_CMD_CHANGE_RPP_CTR,
		IDTP9415_RX_CMD_CHANGE_RPP_CTR_SHIFT, IDTP9415_RX_CMD_VAL);

	return ret;
}

static bool idtp9415_check_tx_exist(void)
{
	int ret;
	u8 mode = 0;

	ret = idtp9415_get_mode(&mode);
	if (ret) {
		hwlog_err("%s: get rx mode fail\n", __func__);
		return false;
	}
	if (mode == IDTP9415_RX_MODE)
		return true;

	return false;
}

static int idtp9415_kick_watchdog(void)
{
	return idtp9415_write_word_mask(IDTP9415_RX_CMD_ADDR,
		IDTP9415_RX_CMD_FEED_FC_WD, IDTP9415_RX_CMD_FEED_FC_WD_SHIFT,
		IDTP9415_RX_CMD_VAL);
}

static bool idtp9415_need_check_pu_shell(void)
{
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di)
		return false;

	return di->need_chk_pu_shell;
}

static void idtp9415_set_pu_shell_flag(bool flag)
{
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	di->pu_shell_flag = flag;
	hwlog_info("[%s] %s succ\n", __func__, flag ? "true" : "false");
}

static char *idtp9415_get_rx_fod(void)
{
	int i;
	int ret;
	static char fod_str[IDTP9415_RX_FOD_STR_LEN] = { 0 };
	u16 fod_coef[IDTP9415_RX_FOD_LEN] = { 0 };
	char tmp[IDTP9415_RX_TMP_BUFF_LEN] = { 0 };

	memset(fod_str, 0, IDTP9415_RX_FOD_STR_LEN);
	for (i = 0; i < IDTP9415_RX_FOD_LEN; i++) {
		ret = idtp9415_read_word(IDTP9415_RX_FOD_ADDR + i * WORD_LEN,
			&fod_coef[i]);
		if (ret) {
			hwlog_err("%s: read fod_coed[%d] fail\n", __func__, i);
			return "error";
		}
	}

	for (i = 0; i < IDTP9415_RX_FOD_LEN; i++) {
		snprintf(tmp, IDTP9415_RX_TMP_BUFF_LEN, "%d ", fod_coef[i]);
		strncat(fod_str, tmp, strlen(tmp));
	}

	return fod_str;
}

static int idtp9415_set_rx_fod(char *fod_str)
{
	int i;
	int ret;
	u16 val = 0;
	char *cur = fod_str;
	char *token = NULL;
	const char *sep = " ,";

	if (!fod_str) {
		hwlog_err("%s: input fod_str null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	for (i = 0; i < IDTP9415_RX_FOD_LEN; i++) {
		token = strsep(&cur, sep);
		if (!token) {
			hwlog_err("%s: input fod_str number err\n", __func__);
			return -WLC_ERR_PARA_WRONG;
		}
		ret = kstrtou16(token, WLC_DECIMAL, &val);
		if (ret) {
			hwlog_err("%s: input fod_str type err\n", __func__);
			return -WLC_ERR_PARA_WRONG;
		}
		hwlog_info("[%s] fod[%d] = 0x%x\n", __func__, i, val);
		ret = idtp9415_write_word(IDTP9415_RX_FOD_ADDR + i * WORD_LEN,
			(u16)val);
		if (ret) {
			hwlog_err("%s: set fod_reg[%d] fail\n", __func__, i);
			return ret;
		}
	}

	return 0;
}

static int idtp9415_init_rx_fod_coef(struct idtp9415_dev_info *di)
{
	int i;
	int ret;
	int tx_vset;
	u16 addr;
	u16 *fod = NULL;

	tx_vset = idtp9415_get_tx_vout_reg();
	hwlog_info("[%s] tx_vout_reg: %dmV\n", __func__, tx_vset);

	if (tx_vset < 9000) { /* (0, 9)V, set 5v fod */
		if (di->pu_shell_flag && di->need_chk_pu_shell)
			fod = di->pu_rx_fod_5v;
		else
			fod = di->rx_fod_5v;
	} else if (tx_vset < 12000) { /* [9, 12)V, set 9V fod */
		fod = di->rx_fod_9v;
	} else if (tx_vset < 15000) { /* [12, 15)V, set 12V fod */
		fod = di->rx_fod_12v;
	} else if (tx_vset < 18000) { /* [15, 18)V, set 15V fod */
		fod = di->rx_fod_15v;
	} else {
		return -WLC_ERR_MISMATCH;
	}

	for (i = 0; i < IDTP9415_RX_FOD_LEN; i++) {
		addr = IDTP9415_RX_FOD_ADDR + i * WORD_LEN;
		ret = idtp9415_write_word(addr, fod[i]);
		if (ret) {
			hwlog_err("%s: fail, addr=0x%x\n", __func__, addr);
			return ret;
		}
	}

	return 0;
}

static int idtp9415_chip_init(int init_type, int tx_type)
{
	int ret = 0;
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di)
		return -WLC_ERR_PARA_NULL;

	hwlog_info("[chip_init] tx_type=%d\n", tx_type);
	switch (init_type) {
	case WIRELESS_CHIP_INIT:
		hwlog_info("[%s] DEFAULT_CHIP_INIT\n", __func__);
		g_stop_charging_flag = false;
		ret += idtp9415_write_word_mask(IDTP9415_RX_CMD_ADDR,
			IDTP9415_RX_CMD_EN_FC_WD,
			IDTP9415_RX_CMD_EN_FC_WD_SHIFT,
			IDTP9415_RX_CMD_VAL);
		ret = idtp9415_write_word(IDTP9415_RX_FC_WD_TIMEOUT_ADDR,
			IDTP9415_RX_FC_WD_TIMEOUT);
		/* fall through */
	case ADAPTER_5V * WL_MVOLT_PER_VOLT:
		hwlog_info("[%s] 5V_CHIP_INIT\n", __func__);
		ret += idtp9415_write_block(di, IDTP9415_RX_VRECT_COEF_ADDR,
			di->rx_vrect_coef_5v, IDTP9415_RX_VRECT_COEF_LEN);
		ret += idtp9415_init_rx_fod_coef(di);
		idtp9415_set_mode_cfg(init_type);
		break;
	case ADAPTER_9V * WL_MVOLT_PER_VOLT:
		hwlog_info("[%s] 9V_CHIP_INIT\n", __func__);
		ret += idtp9415_write_block(di, IDTP9415_RX_VRECT_COEF_ADDR,
			di->rx_vrect_coef_9v, IDTP9415_RX_VRECT_COEF_LEN);
		ret += idtp9415_init_rx_fod_coef(di);
		break;
	case ADAPTER_12V * WL_MVOLT_PER_VOLT:
		hwlog_info("[%s] 12V_CHIP_INIT\n", __func__);
		ret += idtp9415_write_block(di, IDTP9415_RX_VRECT_COEF_ADDR,
			di->rx_vrect_coef_12v, IDTP9415_RX_VRECT_COEF_LEN);
		ret += idtp9415_init_rx_fod_coef(di);
		break;
	case WILREESS_SC_CHIP_INIT:
		hwlog_info("[%s] SC_CHIP_INIT\n", __func__);
		ret += idtp9415_write_block(di, IDTP9415_RX_VRECT_COEF_ADDR,
			di->rx_vrect_coef_sc, IDTP9415_RX_VRECT_COEF_LEN);
		ret += idtp9415_init_rx_fod_coef(di);
		break;
	default:
		hwlog_info("%s: input para is invalid\n", __func__);
		break;
	}

	return ret;
}

static int idtp9415_stop_charging(void)
{
	int wired_channel_state;
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	g_stop_charging_flag = true;

	if (g_irq_abnormal_flag) {
		wired_channel_state =
			wireless_charge_get_wired_channel_state();
		if (wired_channel_state != WIRED_CHANNEL_ON) {
			hwlog_info("[%s] irq_abnormal,\t"
				"keep wireless switch on\n", __func__);
			g_irq_abnormal_flag = true;
			wlps_control(WLPS_RX_SW, WLPS_CTRL_ON);
		} else {
			di->irq_cnt = 0;
			g_irq_abnormal_flag = false;
			idtp9415_enable_irq(di);
			hwlog_info("[%s] wired channel on, enable irq_int\n",
				__func__);
		}
	}

	return 0;
}

static int idtp9415_data_received_handler(struct idtp9415_dev_info *di)
{
	int ret;
	int i;
	u8 cmd;
	u8 buff[QI_PKT_LEN] = { 0 };

	ret = idtp9415_read_block(di, IDTP9415_TX2RX_HEADER_ADDR,
		buff, QI_PKT_LEN);
	if (ret) {
		hwlog_err("data_received_handler: read received data failed\n");
		return -WLC_ERR_I2C_R;
	}

	cmd = buff[QI_PKT_CMD];
	hwlog_info("[data_received_handler] cmd: 0x%x\n", cmd);
	for (i = QI_PKT_DATA; i < QI_PKT_LEN; i++)
		hwlog_info("[data_received_handler] data: 0x%x\n", buff[i]);

	switch (cmd) {
	case QI_CMD_TX_ALARM:
	case QI_CMD_ACK_BST_ERR:
		di->irq_val &= ~IDTP9415_RX_INT_TXDATA_RCVD;
		if (g_idtp9415_handle &&
			g_idtp9415_handle->hdl_non_qi_fsk_pkt)
			g_idtp9415_handle->hdl_non_qi_fsk_pkt(buff, QI_PKT_LEN);
		break;
	default:
		break;
	}
	return 0;
}

static void idtp9415_rx_ready_handler(struct idtp9415_dev_info *di)
{
	int wired_ch_state;

	wired_ch_state = wireless_charge_get_wired_channel_state();
	if (wired_ch_state == WIRED_CHANNEL_ON) {
		hwlog_err("%s: wired channel on, ignore\n", __func__);
		return;
	}

	hwlog_info("[%s] rx ready, goto wireless charging\n", __func__);
	g_stop_charging_flag = false;
	di->irq_cnt = 0;
	wired_chsw_set_wired_channel(WIRED_CHANNEL_CUTOFF);
	wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_ON);
	msleep(CHANNEL_SW_TIME);
	gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
	wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_OFF);
	wlc_rx_evt_notify(WLC_EVT_RX_READY, NULL);
}

static void idtp9415_handle_abnormal_irq(struct idtp9415_dev_info *di)
{
	static struct timespec64 ts64_timeout;
	struct timespec64 ts64_interval;
	struct timespec64 ts64_now;

	ts64_now = current_kernel_time64();
	ts64_interval.tv_sec = 0;
	ts64_interval.tv_nsec = WIRELESS_INT_TIMEOUT_TH * NSEC_PER_MSEC;

	hwlog_info("[%s] irq_cnt = %d\n", __func__, ++di->irq_cnt);
	/* power on interrupt happen first time, so start monitor it */
	if (di->irq_cnt == 1) {
		ts64_timeout = timespec64_add_safe(ts64_now, ts64_interval);
		if (ts64_timeout.tv_sec == TIME_T_MAX) {
			di->irq_cnt = 0;
			hwlog_err("%s: time overflow happened\n", __func__);
			return;
		}
	}

	if (timespec64_compare(&ts64_now, &ts64_timeout) >= 0) {
		if (di->irq_cnt >= WIRELESS_INT_CNT_TH) {
			g_irq_abnormal_flag = true;
			wlps_control(WLPS_RX_SW, WLPS_CTRL_ON);
			idtp9415_disable_irq_nosync(di);
			gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
			hwlog_err("%s: more than %d irq in %ds, disable irq\n",
				__func__, WIRELESS_INT_CNT_TH,
				WIRELESS_INT_TIMEOUT_TH / WL_MSEC_PER_SEC);
		} else {
			di->irq_cnt = 0;
			hwlog_info("%s: less than %d irq in %ds, clr irq cnt\n",
				__func__, WIRELESS_INT_CNT_TH,
				WIRELESS_INT_TIMEOUT_TH / WL_MSEC_PER_SEC);
		}
	}
}

static void idtp9415_rx_power_on_handler(struct idtp9415_dev_info *di)
{
	u8 rx_ss = 0; /* ss: Signal Strength */
	int pwr_flag = RX_PWR_ON_NOT_GOOD;
	int wired_ch_state;

	wired_ch_state = wireless_charge_get_wired_channel_state();
	if (wired_ch_state == WIRED_CHANNEL_ON) {
		hwlog_err("rx_power_on_handler: wired channel on, ignore\n");
		return;
	}

	idtp9415_handle_abnormal_irq(di);
	idtp9415_read_byte(IDTP9415_RX_SS_ADDR, &rx_ss);
	hwlog_info("[rx_power_on_handler] SS = %u\n", rx_ss);
	if ((rx_ss > di->rx_ss_good_lth) && (rx_ss <= IDTP9415_RX_SS_MAX))
		pwr_flag = RX_PWR_ON_GOOD;
	wlc_rx_evt_notify(WLC_EVT_RX_PWR_ON, &pwr_flag);
}

static void idtp9415_rx_mode_irq_recheck(
	struct idtp9415_dev_info *di, u32 irq_val)
{
	if (irq_val & IDTP9415_RX_INT_READY)
		idtp9415_rx_ready_handler(di);
}

static void idtp9415_clear_rx_irq_again(struct idtp9415_dev_info *di)
{
	int ret;
	u32 irq_value = 0;

	hwlog_err("%s: gpio_int low, recheck irq\n", __func__);
	ret = idtp9415_get_rx_interrupt(di, &irq_value);
	if (!ret) {
		hwlog_info("[%s] gpio_int low, irq=0x%x, recheck irq\n",
			__func__, irq_value);
		idtp9415_rx_mode_irq_recheck(di, irq_value);
	}
	idtp9415_rx_clear_interrupt(di, IDTP9415_ALL_INTR_CLR);
}

static void idtp9415_rx_mode_irq_handler(struct idtp9415_dev_info *di)
{
	int ret;

	ret = idtp9415_get_rx_interrupt(di, &di->irq_val);
	if (ret) {
		hwlog_err("%s: read interrupt fail, clear\n", __func__);
		idtp9415_rx_clear_interrupt(di, IDTP9415_ALL_INTR_CLR);
		idtp9415_handle_abnormal_irq(di);
		goto clr_irq_again;
	}

	hwlog_info("[%s] interrupt 0x%08x\n", __func__, di->irq_val);
	idtp9415_rx_clear_interrupt(di, di->irq_val);

	if (di->irq_val & IDTP9415_RX_INT_POWER_ON) {
		di->irq_val &= ~IDTP9415_RX_INT_POWER_ON;
		idtp9415_rx_power_on_handler(di);
	}
	if (di->irq_val & IDTP9415_RX_INT_READY) {
		di->irq_val &= ~IDTP9415_RX_INT_READY;
		idtp9415_rx_ready_handler(di);
	}
	if (di->irq_val & IDTP9415_RX_INT_OCP) {
		di->irq_val &= ~IDTP9415_RX_INT_OCP;
		wlc_rx_evt_notify(WLC_EVT_RX_OCP, NULL);
	}
	if (di->irq_val & IDTP9415_RX_INT_OVP) {
		di->irq_val &= ~IDTP9415_RX_INT_OVP;
		wlc_rx_evt_notify(WLC_EVT_RX_OVP, NULL);
	}
	if (di->irq_val & IDTP9415_RX_INT_OTP) {
		di->irq_val &= ~IDTP9415_RX_INT_OTP;
		wlc_rx_evt_notify(WLC_EVT_RX_OTP, NULL);
	}
	if (di->irq_val & IDTP9415_RX_INT_SEND_PKT_SUCC)
		hwlog_info("[%s] send pkt succ\n", __func__);

	if (di->irq_val & IDTP9415_RX_INT_TXDATA_RCVD)
		idtp9415_data_received_handler(di);

clr_irq_again:
	if (!gpio_get_value(di->gpio_int))
		idtp9415_clear_rx_irq_again(di);
}

static void idtp9415_pmic_vbus_handler(bool vbus_state)
{
	int ret;
	int wired_ch_state;
	u32 irq_val = 0;
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	wired_ch_state = wireless_charge_get_wired_channel_state();
	if (vbus_state && g_irq_abnormal_flag &&
		wired_ch_state != WIRED_CHANNEL_ON &&
		idtp9415_check_tx_exist()) {
		ret = idtp9415_get_rx_interrupt(di, &irq_val);
		if (ret) {
			hwlog_err("%s: read interrupt fail, clear\n", __func__);
			return;
		}
		hwlog_info("[%s] irq_val = 0x%x\n", __func__, irq_val);
		if (irq_val & IDTP9415_RX_INT_READY) {
			idtp9415_rx_clear_interrupt(di, IDTP9415_ALL_INTR_CLR);
			idtp9415_rx_ready_handler(di);
			di->irq_cnt = 0;
			g_irq_abnormal_flag = false;
			idtp9415_enable_irq(di);
		}
	}
}

/*
 * idtp9415 tx mode
 */
static int idtp9415_get_tx_vrect(u16 *tx_vrect)
{
	int ret;

	if (!tx_vrect) {
		hwlog_err("%s: input para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = idtp9415_read_word(IDTP9415_TX_VRECT_ADDR, tx_vrect);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	return 0;
}

static int idtp9415_get_tx_vin(u16 *tx_vin)
{
	int ret;

	if (!tx_vin) {
		hwlog_err("%s: input para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = idtp9415_read_word(IDTP9415_TX_VIN_ADDR, tx_vin);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	return 0;
}

static int idtp9415_get_tx_iin(u16 *tx_iin)
{
	int ret;

	if (!tx_iin) {
		hwlog_err("%s: input para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = idtp9415_read_word(IDTP9415_TX_IIN_ADDR, tx_iin);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	return 0;
}

static int idtp9415_get_tx_temp(u8 *chip_temp)
{
	int ret;

	if (!chip_temp) {
		hwlog_err("%s: input para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = idtp9415_read_byte(IDTP9415_CHIP_TEMP_ADDR, chip_temp);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -1;
	}

	return 0;
}

static int idtp9415_get_tx_fop(u16 *fop)
{
	int ret;
	u16 data = 0;

	if (!fop) {
		hwlog_err("%s: input para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = idtp9415_read_word(IDTP9415_TX_FOP_ADDR, &data);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	if (data > 0)
		*fop = IDTP9415_TX_PING_HCLK / data;

	return 0;
}

static int idtp9415_enable_tx_mode(bool enable)
{
	int ret;

	if (enable)
		ret = idtp9415_write_word_mask(IDTP9415_TX_CMD_ADDR,
			IDTP9415_TX_CMD_EN_MASK, IDTP9415_TX_CMD_EN_SHIFT,
			IDTP9415_TX_CMD_VAL);
	else
		ret = idtp9415_write_word_mask(IDTP9415_TX_CMD_ADDR,
			IDTP9415_TX_CMD_DIS_MASK, IDTP9415_TX_CMD_DIS_SHIFT,
			IDTP9415_TX_CMD_VAL);

	if (ret) {
		hwlog_err("%s: %s fail\n", __func__,
			enable ? "enable" : "disable");
		return ret;
	}

	return 0;
}

static int idtp9415_get_tx_min_fop(u16 *fop)
{
	int ret;
	u16 data = 0;

	if (!fop) {
		hwlog_err("%s: input para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = idtp9415_read_word(IDTP9415_TX_MIN_FOP_ADDR, &data);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	if (data > 0)
		*fop = IDTP9415_TX_PING_HCLK / data;

	return 0;
}

static int idtp9415_set_tx_min_fop(u16 fop)
{
	int ret;

	if ((fop < IDTP9415_TX_MIN_FOP_VAL) || (fop > IDTP9415_TX_MAX_FOP_VAL))
		return -WLC_ERR_PARA_WRONG;

	ret = idtp9415_write_word(IDTP9415_TX_MIN_FOP_ADDR,
		IDTP9415_TX_PING_HCLK / fop);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	return 0;
}

static int idtp9415_get_tx_max_fop(u16 *fop)
{
	int ret;
	u16 data = 0;

	if (!fop) {
		hwlog_err("%s: input para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = idtp9415_read_word(IDTP9415_TX_MAX_FOP_ADDR, &data);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	if (data > 0)
		*fop = IDTP9415_TX_PING_HCLK / data;

	return 0;
}

static int idtp9415_set_tx_max_fop(u16 fop)
{
	int ret;

	if ((fop < IDTP9415_TX_MIN_FOP_VAL) || (fop > IDTP9415_TX_MAX_FOP_VAL))
		return -WLC_ERR_PARA_WRONG;

	ret = idtp9415_write_word(IDTP9415_TX_MAX_FOP_ADDR,
		IDTP9415_TX_PING_HCLK / fop);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	return 0;
}

static int idtp9415_get_tx_ping_frequency(u16 *ping_freq)
{
	int ret;
	u16 data = 0;

	if (!ping_freq) {
		hwlog_err("%s: input para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = idtp9415_read_word(IDTP9415_TX_PING_FREQUENCY_ADDR, &data);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	if (data > 0)
		*ping_freq = IDTP9415_TX_PING_HCLK / data;

	return 0;
}

static int idtp9415_set_tx_ping_frequency(u16 ping_freq)
{
	int ret;

	if ((ping_freq < IDTP9415_TX_PING_FREQUENCY_MIN) ||
		(ping_freq > IDTP9415_TX_PING_FREQUENCY_MAX)) {
		hwlog_err("%s: ping frequency out of range\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	ret = idtp9415_write_word(IDTP9415_TX_PING_FREQUENCY_ADDR,
		IDTP9415_TX_PING_HCLK / ping_freq);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	return 0;
}

static int idtp9415_get_tx_ping_interval(u16 *ping_interval)
{
	int ret;
	u8 data = 0;

	if (!ping_interval) {
		hwlog_err("%s: input para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = idtp9415_read_byte(IDTP9415_TX_PING_INTERVAL_ADDR, &data);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}
	*ping_interval = data * IDTP9415_TX_PING_INTERVAL_STEP;

	return 0;
}

static int idtp9415_set_tx_ping_interval(u16 ping_interval)
{
	int ret;

	if ((ping_interval < IDTP9415_TX_PING_INTERVAL_MIN) ||
		(ping_interval > IDTP9415_TX_PING_INTERVAL_MAX)) {
		hwlog_err("%s: ping interval out of range\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}
	ret = idtp9415_write_byte(IDTP9415_TX_PING_INTERVAL_ADDR,
		ping_interval / IDTP9415_TX_PING_INTERVAL_STEP);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	return 0;
}

static bool idtp9415_check_rx_disconnect(void)
{
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return true;
	}

	if (di->ept_type & IDTP9415_TX_EPT_CEP_TIMEOUT) {
		di->ept_type &= ~IDTP9415_TX_EPT_CEP_TIMEOUT;
		hwlog_info("[%s] rx disconnect\n", __func__);
		return true;
	}

	return false;
}

static bool idtp9415_in_tx_mode(void)
{
	int ret;
	u8 mode = 0;

	ret = idtp9415_read_byte(IDTP9415_OP_MODE_ADDR, &mode);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return false;
	}

	if (mode == IDTP9415_TX_MODE)
		return true;
	else
		return false;
}

static void idtp9415_set_tx_open_flag(bool enable)
{
	struct idtp9415_dev_info *di = g_idtp9415_di;

	g_tx_open_flag = enable;
	g_irq_abnormal_flag = false;

	if (!di)
		return;
	/*
	 * when idtp9415_handle_abnormal_irq disable irq, tx will irq-mask
	 * so, here enable irq
	 */
	di->irq_cnt = 0;
	idtp9415_enable_irq(di);
}

void idtp9415_pre_pwr_supply(bool flag)
{
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di)
		return;

	if (flag) {
		/* avoid trx_en and sleep_en all high cause i2c block */
		gpio_set_value(di->gpio_en, di->gpio_en_valid_val);
		usleep_range(5000, 5100); /* 5ms */
		wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_ON);
	} else {
		wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_OFF);
	}
}

static int idtp9415_set_tx_fod(struct idtp9415_dev_info *di)
{
	int ret;

	ret = idtp9415_write_block(di, IDTP9415_TX_FOD_ADDR, di->tx_fod_coef,
		IDTP9415_TX_FOD_LEN);
	ret += idtp9415_write_word_mask(IDTP9415_TX_CMD_ADDR,
		IDTP9415_TX_CMD_FOD_EN_MASK, IDTP9415_TX_CMD_FOD_EN_SHIFT,
		IDTP9415_TX_CMD_VAL);

	return ret;
}

static int idtp9415_tx_mode_vset(int tx_vset)
{
	int ret;

	msleep(20); /* delay for M0 init */
	if (tx_vset == IDTP9415_PS_TX_VOLT_5V5)
		ret = idtp9415_write_byte(IDTP9415_PS_TX_GPIO_ADDR,
			IDTP9415_PS_TX_GPIO_PU);
	else if (tx_vset == IDTP9415_PS_TX_VOLT_6V8)
		ret = idtp9415_write_byte(IDTP9415_PS_TX_GPIO_ADDR,
			IDTP9415_PS_TX_GPIO_OPEN);
	else if (tx_vset == IDTP9415_PS_TX_VOLT_10V)
		ret = idtp9415_write_byte(IDTP9415_PS_TX_GPIO_ADDR,
			IDTP9415_PS_TX_GPIO_PD);
	else
		return -WLC_ERR_PARA_WRONG;

	ret += idtp9415_write_word_mask(IDTP9415_TX_CMD_ADDR,
		IDTP9415_TX_CMD_BOOST_CFG_MASK,
		IDTP9415_TX_CMD_BOOST_CFG_SHIFT, IDTP9415_TX_CMD_VAL);
	if (ret)
		return -WLC_ERR_I2C_WR;

	return 0;
}

static int idtp9415_tx_chip_init(void)
{
	int ret;
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	di->irq_cnt = 0;
	g_irq_abnormal_flag = false;
	/*
	 * when idtp9415_handle_abnormal_irq disable irq, tx will irq-mask
	 * so, here enable irq
	 */
	idtp9415_enable_irq(di);

	ret = idtp9415_write_word(IDTP9415_TX_OCP_ADDR, IDTP9415_TX_OCP_VAL);
	ret += idtp9415_write_word(IDTP9415_TX_OVP_ADDR, IDTP9415_TX_OVP_VAL);
	ret += idtp9415_set_tx_min_fop(IDTP9415_TX_MIN_FOP_VAL);
	ret += idtp9415_set_tx_max_fop(IDTP9415_TX_MAX_FOP_VAL);
	ret += idtp9415_set_tx_ping_frequency(IDTP9415_TX_PING_FREQUENCY_INIT);
	ret += idtp9415_set_tx_ping_interval(IDTP9415_TX_PING_INTERVAL_INIT);
	ret += idtp9415_set_tx_fod(di);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static int idtp9415_get_tx_interrupt(struct idtp9415_dev_info *di, u32 *intr)
{
	return idtp9415_read_block(di, IDTP9415_TX_INT_ADDR,
		(u8 *)intr, IDTP9415_TX_INT_LEN);
}

static void idtp9415_clear_tx_interrupt(struct idtp9415_dev_info *di, u32 intr)
{
	int ret;

	ret = idtp9415_write_block(di, IDTP9415_TX_INT_CLR_ADDR,
		(u8 *)&intr, IDTP9415_TX_INT_CLR_LEN);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return;
	}

	ret = idtp9415_write_word_mask(IDTP9415_TX_CMD_ADDR,
		IDTP9415_TX_CMD_CLRINT_MASK, IDTP9415_TX_CMD_CLRINT_SHIFT,
		IDTP9415_TX_CMD_VAL);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return;
	}
}

static int idtp9415_get_ask_pkt(u8 *pkt_data, int pkt_data_len)
{
	int ret;
	int i;
	char buff[IDTP9415_RX2TX_BUFF_LEN] = { 0 };
	char pkt_str[IDTP9415_RX2TX_STR_LEN] = { 0 };
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di || !pkt_data || (pkt_data_len <= 0) ||
		(pkt_data_len > IDTP9415_RX2TX_PKT_LEN)) {
		hwlog_err("%s: para err\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	ret = idtp9415_read_block(di, IDTP9415_RX2TX_HEADER_ADDR,
		pkt_data, pkt_data_len);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return ret;
	}
	for (i = 0; i < pkt_data_len; i++) {
		snprintf(buff, IDTP9415_RX2TX_BUFF_LEN, "0x%02x ", pkt_data[i]);
		strncat(pkt_str, buff, strlen(buff));
	}
	hwlog_info("[%s] RX back packet: %s\n", __func__, pkt_str);

	return 0;
}

static int idtp9415_tx_get_ept_type(u16 *ept_type)
{
	int ret;
	u16 data = 0;

	ret = idtp9415_read_word(IDTP9415_TX_EPT_TYPE_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return ret;
	}

	*ept_type = data;
	hwlog_info("[%s] EPT type = 0x%04x", __func__, *ept_type);
	ret = idtp9415_write_word(IDTP9415_TX_EPT_TYPE_ADDR, 0);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	return 0;
}

static void idtp9415_handle_tx_ept(struct idtp9415_dev_info *di)
{
	int ret;

	di->irq_val &= ~IDTP9415_TX_INT_EPT_TYPE;
	ret = idtp9415_tx_get_ept_type(&di->ept_type);
	if (ret)
		return;

	switch (di->ept_type) {
	case IDTP9415_TX_EPT_CMD:
		di->ept_type &= ~IDTP9415_TX_EPT_CMD;
		hwlog_info("[%s] ept command\n", __func__);
		break;
	case IDTP9415_TX_EPT_CEP_TIMEOUT:
		di->ept_type &= ~IDTP9415_TX_EPT_CEP_TIMEOUT;
		hwlog_info("[%s] rx disconnect\n", __func__);
		blocking_notifier_call_chain(&tx_event_nh,
			WL_TX_EVENT_CEP_TIMEOUT, NULL);
		break;
	case IDTP9415_TX_EPT_FOD:
		di->ept_type &= ~IDTP9415_TX_EPT_FOD;
		hwlog_info("[%s] fod happened\n", __func__);
		blocking_notifier_call_chain(&tx_event_nh,
			WLTX_EVT_TX_FOD, NULL);
		break;
	case IDTP9415_TX_EPT_OVP:
		di->ept_type &= ~IDTP9415_TX_EPT_OVP;
		hwlog_info("[%s] ovp happened\n", __func__);
		blocking_notifier_call_chain(&tx_event_nh,
			WL_TX_EVENT_OVP, NULL);
		break;
	case IDTP9415_TX_EPT_OCP:
		di->ept_type &= ~IDTP9415_TX_EPT_OCP;
		hwlog_info("[%s] ocp happened\n", __func__);
		blocking_notifier_call_chain(&tx_event_nh,
			WL_TX_EVENT_OCP, NULL);
		break;
	case IDTP9415_TX_EPT_TIMEOUT:
		di->ept_type &= ~IDTP9415_TX_EPT_TIMEOUT;
		hwlog_info("[%s] chip reset happened\n", __func__);
		break;
	case IDTP9415_TX_EPT_POCP:
		di->ept_type &= ~IDTP9415_TX_EPT_POCP;
		hwlog_info("[%s] ping ocp happened\n", __func__);
		break;
	default:
		hwlog_err("%s: unrecognized type\n", __func__);
		break;
	}
}

static int idtp9415_send_fsk_msg(u8 cmd, u8 *data, int data_len)
{
	int ret;
	u8 header;
	u8 write_data[IDTP9415_TX2RX_DATA_LEN] = { 0 };
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	if ((data_len > IDTP9415_TX2RX_DATA_LEN) || (data_len < 0)) {
		hwlog_err("%s: data len out of range\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	if (cmd == QI_CMD_ACK)
		header = QI_CMD_ACK_HEAD;
	else
		header = g_idtp9415_tx2rx_header[data_len + 1];

	ret = idtp9415_write_byte(IDTP9415_TX2RX_HEADER_ADDR, header);
	if (ret) {
		hwlog_err("%s: write header fail\n", __func__);
		return ret;
	}
	ret = idtp9415_write_byte(IDTP9415_TX2RX_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("%s: write cmd fail\n", __func__);
		return ret;
	}

	if (data && data_len > 0) {
		memcpy(write_data, data, data_len);
		ret = idtp9415_write_block(di, IDTP9415_TX2RX_DATA_ADDR,
			write_data, data_len);
		if (ret) {
			hwlog_err("%s: write fsk reg fail\n", __func__);
			return ret;
		}
	}
	ret = idtp9415_write_word_mask(IDTP9415_TX_CMD_ADDR,
		IDTP9415_TX_CMD_SEND_FSK_MASK, IDTP9415_TX_CMD_SEND_FSK_SHIFT,
		IDTP9415_TX_CMD_VAL);
	if (ret) {
		hwlog_err("%s: send fsk fail\n", __func__);
		return ret;
	}

	hwlog_info("[%s] succ\n", __func__);
	return 0;
}

static int idtp9415_tx_stop_config(void)
{
	return 0;
}

static void idtp9415_tx_handle_ping_evt(struct idtp9415_dev_info *di)
{
	hwlog_info("tx ping interrupt\n");
	di->irq_val &= ~IDTP9415_TX_INT_START_DPING;
	blocking_notifier_call_chain(&tx_event_nh, WL_TX_EVENT_PING_RX, NULL);
}

static void idtp9415_tx_handle_ss_evt(struct idtp9415_dev_info *di)
{
	hwlog_info("signal strength packet interrupt\n");
	di->irq_val &= ~IDTP9415_TX_INT_GET_SS;
	if (g_idtp9415_handle && g_idtp9415_handle->hdl_qi_ask_pkt)
		g_idtp9415_handle->hdl_qi_ask_pkt();
}

static void idtp9415_tx_handle_id_evt(struct idtp9415_dev_info *di)
{
	hwlog_info("ID packet interrupt\n");
	di->irq_val &= ~IDTP9415_TX_INT_GET_ID;
	if (g_idtp9415_handle && g_idtp9415_handle->hdl_qi_ask_pkt)
		g_idtp9415_handle->hdl_qi_ask_pkt();
}

static void idtp9415_tx_handle_cfg_evt(struct idtp9415_dev_info *di)
{
	hwlog_info("Config packet interrupt\n");
	di->irq_val &= ~IDTP9415_TX_INT_GET_CFG;
	if (g_idtp9415_handle && g_idtp9415_handle->hdl_qi_ask_pkt)
		g_idtp9415_handle->hdl_qi_ask_pkt();
	blocking_notifier_call_chain(&tx_event_nh, WL_TX_EVENT_GET_CFG, NULL);
}

static void idtp9415_tx_handle_init_evt(struct idtp9415_dev_info *di)
{
	hwlog_info("tx init interrupt\n");
	di->irq_val &= ~IDTP9415_TX_INT_TX_INIT;
	blocking_notifier_call_chain(&tx_event_nh, WLTX_EVT_TX_INIT, NULL);
}

static void idtp9415_tx_handle_tx_vset_evt(struct idtp9415_dev_info *di)
{
	int vset = IDTP9415_PS_TX_VOLT_6V8;

	hwlog_info("tx need increase vset to %dmV\n", vset);
	di->irq_val &= ~IDTP9415_TX_INT_SET_VTX;
	blocking_notifier_call_chain(&tx_event_nh, WLTX_EVT_IRQ_SET_VTX, &vset);
}

static void idtp9415_tx_handle_dping_evt(struct idtp9415_dev_info *di)
{
	di->irq_val &= ~IDTP9415_TX_INT_GET_DPING;
	hwlog_info("rcv dping in tx mode\n");
	blocking_notifier_call_chain(&tx_event_nh, WLTX_EVT_RCV_DPING, NULL);
}

static void idtp9415_tx_handle_ppp_evt(struct idtp9415_dev_info *di)
{
	hwlog_info("tx rcv personal property ASK pkt\n");
	di->irq_val &= ~IDTP9415_TX_INT_GET_PPP;
	if (g_idtp9415_handle && g_idtp9415_handle->hdl_non_qi_ask_pkt)
		g_idtp9415_handle->hdl_non_qi_ask_pkt();
}

static void idtp9415_tx_mode_irq_handler(struct idtp9415_dev_info *di)
{
	int ret;
	u32 irq_value = 0;

	ret = idtp9415_get_tx_interrupt(di, &di->irq_val);
	if (ret) {
		hwlog_err("%s: read interrupt fail, clear\n", __func__);
		idtp9415_clear_tx_interrupt(di, IDTP9415_ALL_INTR_CLR);
		goto clr_irq_again;
	}

	hwlog_info("[%s] interrupt 0x%08x\n", __func__, di->irq_val);
	idtp9415_clear_tx_interrupt(di, di->irq_val);

	if (di->irq_val & IDTP9415_TX_INT_START_DPING)
		idtp9415_tx_handle_ping_evt(di);
	if (di->irq_val & IDTP9415_TX_INT_GET_SS)
		idtp9415_tx_handle_ss_evt(di);
	if (di->irq_val & IDTP9415_TX_INT_GET_ID)
		idtp9415_tx_handle_id_evt(di);
	if (di->irq_val & IDTP9415_TX_INT_GET_CFG)
		idtp9415_tx_handle_cfg_evt(di);
	if (di->irq_val & IDTP9415_TX_INT_TX_INIT)
		idtp9415_tx_handle_init_evt(di);
	if (di->irq_val & IDTP9415_TX_INT_EPT_TYPE)
		idtp9415_handle_tx_ept(di);
	if (di->irq_val & IDTP9415_TX_INT_GET_PPP)
		idtp9415_tx_handle_ppp_evt(di);
	if (di->irq_val & IDTP9415_TX_INT_SET_VTX)
		idtp9415_tx_handle_tx_vset_evt(di);
	if (di->irq_val & IDTP9415_TX_INT_GET_DPING)
		idtp9415_tx_handle_dping_evt(di);

clr_irq_again:
	if (!gpio_get_value(di->gpio_int)) {
		ret = idtp9415_get_tx_interrupt(di, &irq_value);
		hwlog_info("[%s] gpio_int low, clear irq(0x%x) again\n",
			__func__, irq_value);
		idtp9415_clear_tx_interrupt(di, IDTP9415_ALL_INTR_CLR);
	}
}

static void idtp9415_mode_change_handler(void)
{
	int ret;

	if (!g_tx_open_flag)
		return;

	ret = idtp9415_write_byte(IDTP9415_TX_AP_ON_ADDR, IDTP9415_TX_AP_ON);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return;
	}
	blocking_notifier_call_chain(&tx_event_nh, WLTX_EVT_TX_AP_ON, NULL);
}

static void idtp9415_stand_by_mode_irq_handler(struct idtp9415_dev_info *di)
{
	int ret;

	ret = idtp9415_get_rx_interrupt(di, &di->irq_val);
	if (ret) {
		hwlog_err("%s: read interrupt fail, clear\n", __func__);
		return;
	}

	hwlog_info("[%s] interrupt 0x%08x\n", __func__, di->irq_val);

	if (di->irq_val & IDTP9415_RX_INT_MODE_CHANGE) {
		hwlog_info("[%s] mode change interrupt\n", __func__);
		di->irq_val &= ~IDTP9415_RX_INT_MODE_CHANGE;
		idtp9415_mode_change_handler();
	}

	idtp9415_clear_tx_interrupt(di, IDTP9415_ALL_INTR_CLR);
}

static void idtp9415_irq_work(struct work_struct *work)
{
	int ret;
	u8 mode = 0;
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		idtp9415_wake_unlock();
		return;
	}

	/* get System Operating Mode */
	ret = idtp9415_get_mode(&mode);
	if (!ret)
		hwlog_info("[%s] mode = 0x%x\n", __func__, mode);
	else
		idtp9415_handle_abnormal_irq(di);

	/* handler interrupt */
	if (mode == IDTP9415_TX_MODE)
		idtp9415_tx_mode_irq_handler(di);
	else if (mode == IDTP9415_RX_MODE)
		idtp9415_rx_mode_irq_handler(di);
	else
		idtp9415_stand_by_mode_irq_handler(di);

	if (!g_irq_abnormal_flag)
		idtp9415_enable_irq(di);

	idtp9415_wake_unlock();
}

static irqreturn_t idtp9415_interrupt(int irq, void *_di)
{
	struct idtp9415_dev_info *di = _di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return IRQ_HANDLED;
	}

	idtp9415_wake_lock();
	hwlog_info("[%s] ++\n", __func__);
	if (di->irq_active) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("irq is not enable,do nothing\n");
		idtp9415_wake_unlock();
	}
	hwlog_info("[%s] --\n", __func__);

	return IRQ_HANDLED;
}

static int idtp9415_mtp_load_bootloader(struct idtp9415_dev_info *di)
{
	int ret;
	int remaining = ARRAY_SIZE(g_idtp9415_bootloader);
	int size_to_wr;
	int wr_already = 0;
	u16 addr = IDTP9415_BTLOADR_DATA_ADDR;
	u8 wr_buff[IDTP9415_PAGE_SIZE] = { 0 };

	hwlog_info("%s: bootloader_size=%d\n", __func__, remaining);
	while (remaining > 0) {
		size_to_wr = remaining > IDTP9415_PAGE_SIZE ?
			IDTP9415_PAGE_SIZE : remaining;
		memcpy(wr_buff, g_idtp9415_bootloader + wr_already, size_to_wr);
		ret = idtp9415_write_block(di, addr, wr_buff, size_to_wr);
		if (ret) {
			hwlog_err("%s: wr fail, addr=0x%08x\n", __func__, addr);
			return ret;
		}
		addr += size_to_wr;
		wr_already += size_to_wr;
		remaining -= size_to_wr;
	}

	ret = idtp9415_write_byte(0x0400, 0x00); /* initialize buffer */
	ret += idtp9415_write_byte(0x3048, 0xD0); /* map ram addr to MTP */
	if (ret) {
		hwlog_err("%s: remap to RAM fail\n", __func__);
		return ret;
	}
	ret = idtp9415_write_byte(0x3040, 0x80); /* run M0 */
	if (ret)
		hwlog_err("%s: reset M0, NAK, ignore\n", __func__);
	msleep(100); /* delay to reset the M0 and run the MTP downloader */

	hwlog_info("[%s] succ\n", __func__);
	return 0;
}

static int idtp9415_mtp_status_check(void)
{
	int i;
	int ret;
	u8 status = 0;

	/* wait for 1000ms for mtp/crc check */
	for (i = 0; i < 50; i++) {
		msleep(20);
		ret = idtp9415_read_byte(0x0401, &status); /* get mtp status */
		if (ret) {
			hwlog_err("%s: read 0x0401 fail\n", __func__);
			return ret;
		}
		if (!status)
			continue;
		if (!(status & IDTP9415_MTP_BUSY))
			break;
	}

	if (status != IDTP9415_MTP_WR_OK) {
		hwlog_info("%s: fail, status = 0x%x\n", __func__, status);
		return -WLC_ERR_CHECK_FAIL;
	}

	return 0;
}

static int idtp9415_mtp_crc_check(struct idtp9415_dev_info *di)
{
	int ret;

	ret = idtp9415_mtp_load_bootloader(di);
	if (ret) {
		hwlog_err("%s: load bootloader fail\n", __func__);
		return ret;
	}

	/* write mtp start address */
	ret = idtp9415_write_word(0x0402, IDTP9415_MTP_STRT_ADDR);
	/* write 16-bit MTP data size */
	ret += idtp9415_write_word(0x0404, (u16)ARRAY_SIZE(g_idtp9415_fw));
	/* write the 16-bit CRC */
	ret += idtp9415_write_word(0x0406, (u16)IDTP9415_MTP_CRC);
	/* start MTP data CRC-16 check */
	ret += idtp9415_write_byte(0x0400, 0x11);
	ret = idtp9415_mtp_status_check();
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	return 0;
}

static int idt9415_mtp_program_check(void)
{
	int ret;

	ret = idtp9415_write_byte(0x0400, 0x01); /* start programming */
	if (ret) {
		hwlog_err("%s: start programming fail\n", __func__);
		return ret;
	}
	ret = idtp9415_mtp_status_check();
	if (ret) {
		hwlog_err("%s: check mtp status fail\n", __func__);
		return ret;
	}

	return 0;
}

static int idtp9415_mtp_load_fw(struct idtp9415_dev_info *di)
{
	int i;
	int ret;
	u16 addr = IDTP9415_MTP_STRT_ADDR;
	int offset = 0;
	int remaining = ARRAY_SIZE(g_idtp9415_fw);
	int wr_size;
	struct idtp9415_pgm_str pgm_str;

	hwlog_info("%s: mtp_size=%d\n", __func__, remaining);
	while (remaining > 0) {
		memset(&pgm_str, 0, sizeof(struct idtp9415_pgm_str));
		wr_size = remaining > IDTP9415_PAGE_SIZE ?
			IDTP9415_PAGE_SIZE : remaining;
		pgm_str.addr = addr;
		/* 16 bytes aligned */
		pgm_str.code_len = (wr_size + 15) / 16 * 16;
		memcpy(pgm_str.fw, g_idtp9415_fw + offset, wr_size);
		pgm_str.chk_sum = addr + pgm_str.code_len;
		for (i = 0; i < wr_size; i++)
			pgm_str.chk_sum += pgm_str.fw[i];
		 /* write 8-bytes header + fw, 16 bytes aligned */
		ret = idtp9415_write_block(di, IDTP9415_MTP_DATA_ADDR,
			(u8 *)&pgm_str, (pgm_str.code_len + 8 + 15) / 16 * 16);
		if (ret) {
			hwlog_err("%s: wr fail\n", __func__);
			return ret;
		}
		ret = idt9415_mtp_program_check();
		if (ret) {
			hwlog_err("%s: check fail\n", __func__);
			return ret;
		}
		addr += wr_size;
		offset += wr_size;
		remaining -= wr_size;
	}

	return 0;
}

static int idtp9415_mtp_pwr_cycle_chip(struct idtp9415_dev_info *di)
{
	int ret;

	ret = idtp9415_write_byte(0x3000, 0x5A); /* unlock system registers */
	ret += idtp9415_write_byte(0x3004, 0x00); /* set HS clock */
	ret += idtp9415_write_byte(0x3008, 0x09); /* set AHB clock */
	ret += idtp9415_write_byte(0x300C, 0x05); /* configure 1us pulse */
	ret += idtp9415_write_byte(0x300D, 0x1D); /* configure 500ns pulse */
	ret += idtp9415_write_byte(0x304C, 0x00); /* remove MTP protection */
	ret += idtp9415_write_byte(0x304D, 0x00); /* reset M0 */
	ret += idtp9415_write_byte(0x3040, 0x11); /* Enable MTP access */
	usleep_range(9500, 10500); /* delay for mtp enable */
	ret += idtp9415_write_byte(0x3040, 0x10); /* halt M0 */
	usleep_range(9500, 10500); /* delay for mtp enable */
	if (ret) {
		hwlog_err("%s: enable mtp fail\n", __func__);
		return ret;
	}

	return 0;
}

static int idtp9415_mtp_cfg_system(struct idtp9415_dev_info *di)
{
	int ret;

	ret = idtp9415_write_byte(0x3000, 0x5A); /* write key */
	ret += idtp9415_write_byte(0x3004, 0x00); /* set HS clock */
	ret += idtp9415_write_byte(0x3008, 0x09); /* set AHB clock */
	ret += idtp9415_write_byte(0x300C, 0x05); /* configure 1us pulse */
	ret += idtp9415_write_byte(0x300D, 0x1D); /* configure 500ns pulse */
	ret += idtp9415_write_byte(0x3040, 0x11); /* Enable MTP access */
	usleep_range(9500, 10500); /* delay for mtp enable */
	ret += idtp9415_write_byte(0x3040, 0x10); /* halt M0 */
	usleep_range(9500, 10500); /* delay for halt M0 */

	return 0;
}

static int idtp9415_program_mtp(struct idtp9415_dev_info *di)
{
	int ret;

	idtp9415_disable_irq_nosync(di);
	wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_ON);
	wlps_control(WLPS_RX_SW, WLPS_CTRL_OFF); /* prevent reverse current */
	usleep_range(4500, 5500); /* delay 5ms for power on */

	ret = idtp9415_mtp_cfg_system(di);
	if (ret) {
		hwlog_err("%s: cfg system fail\n", __func__);
		goto exit;
	}
	ret = idtp9415_mtp_load_bootloader(di);
	if (ret) {
		hwlog_err("%s: load bootloader fail\n", __func__);
		goto exit;
	}
	ret = idtp9415_mtp_load_fw(di);
	if (ret) {
		hwlog_err("%s: load fw fail\n", __func__);
		goto exit;
	}
	wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_OFF);
	usleep_range(9500, 10500); /* sleep 10ms */
	wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_ON);
	usleep_range(9500, 10500); /* sleep 10ms */
	ret = idtp9415_mtp_pwr_cycle_chip(di);
	if (ret) {
		hwlog_err("%s: power cycle chip fail\n", __func__);
		goto exit;
	}
	ret = idtp9415_mtp_crc_check(di);
	if (ret) {
		hwlog_err("%s: crc check fail\n", __func__);
		goto exit;
	}

	di->mtp_status = IDTP9415_MTP_STATUS_GOOD;
	wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_OFF);
	idtp9415_enable_irq(di);
	hwlog_info("[%s] succ\n", __func__);
	return 0;

exit:
	di->mtp_status = IDTP9415_MTP_STATUS_BAD;
	wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_OFF);
	idtp9415_enable_irq(di);
	hwlog_info("%s: fail\n", __func__);
	return -WLC_ERR_CHECK_FAIL;
}

static int idtp9415_recover_mtp(struct idtp9415_dev_info *di)
{
	return idtp9415_program_mtp(di);
}

static int idtp9415_rx_program_mtp(int proc_type)
{
	int ret;
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	if (!g_mtp_chk_complete)
		return -WLC_ERR_I2C_WR;
	switch (proc_type) {
	case WLC_PROGRAM_OTP:
		g_mtp_chk_complete = false;
		ret = idtp9415_program_mtp(di);
		g_mtp_chk_complete = true;
		return ret;
	case WLC_RECOVER_OTP:
		g_mtp_chk_complete = false;
		ret = idtp9415_recover_mtp(di);
		g_mtp_chk_complete = true;
		return ret;
	default:
		break;
	}

	return -WLC_ERR_MISMATCH;
}

static int idtp9415_mtp_version_check(void)
{
	int ret;
	u16 major_fw_ver = 0;
	u16 minor_fw_ver = 0;

	ret = idtp9415_get_major_fw_version(&major_fw_ver);
	if (ret) {
		hwlog_err("%s: get major fw_ver fail\n", __func__);
		return ret;
	}
	hwlog_err("[%s] major_fw=0x%04x\n", __func__, major_fw_ver);

	ret = idtp9415_get_minor_fw_version(&minor_fw_ver);
	if (ret) {
		hwlog_err("%s: get minor fw_ver fail\n", __func__);
		return ret;
	}
	hwlog_err("[%s] minor_fw=0x%04x\n", __func__, minor_fw_ver);

	if ((major_fw_ver != IDTP9415_MTP_MAJOR_VER) ||
		(minor_fw_ver != IDTP9415_MTP_MINOR_VER))
		return -WLC_ERR_MISMATCH;

	return 0;
}

static int idtp9415_check_mtp(void)
{
	int ret;
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	if (di->mtp_status == IDTP9415_MTP_STATUS_GOOD)
		return 0;

	wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_ON);
	wlps_control(WLPS_RX_SW, WLPS_CTRL_OFF);
	msleep(100); /* delay for power on */

	ret = idtp9415_mtp_version_check();
	if (ret) {
		hwlog_err("%s: version check fail\n", __func__);
		goto check_fail;
	}

	ret = idtp9415_mtp_pwr_cycle_chip(di);
	if (ret) {
		hwlog_err("%s: power cycle chip fail\n", __func__);
		goto check_fail;
	}

	ret = idtp9415_mtp_crc_check(di);
	if (ret) {
		hwlog_err("%s: crc check fail\n", __func__);
		goto check_fail;
	}

	di->mtp_status = IDTP9415_MTP_STATUS_GOOD;
	wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_OFF);
	return 0;

check_fail:
	di->mtp_status = IDTP9415_MTP_STATUS_BAD;
	wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_OFF);
	return -WLC_ERR_CHECK_FAIL;
}

static int idtp9415_check_is_mtp_exist(void)
{
	int ret;

	g_mtp_chk_complete = false;
	ret = idtp9415_check_mtp();
	if (!ret) {
		g_mtp_chk_complete = true;
		return WLC_OTP_PROGRAMED;
	}
	g_mtp_chk_complete = true;

	return WLC_OTP_NON_PROGRAMED;
}

static void idtp9415_mtp_check_work(struct work_struct *work)
{
	int i;
	int ret;
	int rx_en_val;
	int sleep_en_val;
	struct idtp9415_dev_info *di = g_idtp9415_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		goto exit;
	}

	g_mtp_chk_complete = false;
	ret = idtp9415_check_mtp();
	if (!ret) {
		hwlog_info("[%s] succ\n", __func__);
		goto exit;
	}

	/* program for 3 times until it's ok */
	for (i = 1; i <= 3; i++) {
		ret = idtp9415_program_mtp(di);
		if (!ret) {
			hwlog_info("[%s] update mtp succ, cnt=%d\n",
				__func__, i);
			break;
		}
		/* reset chip i2c after 2 times fail */
		if (i == 2) {
			hwlog_err("%s: update mtp fail\n", __func__);
			rx_en_val = gpio_get_value(di->gpio_en);
			sleep_en_val = gpio_get_value(di->gpio_sleep_en);
			gpio_set_value(di->gpio_en, RX_EN_ENABLE);
			gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_ENABLE);
			usleep_range(9500, 10500); /* 10ms */
		}
	}
	/* restore rx_en and sleep_en status when program over 2 times */
	if (i > 2) {
		gpio_set_value(di->gpio_en, rx_en_val);
		gpio_set_value(di->gpio_sleep_en, sleep_en_val);
	}
exit:
	g_mtp_chk_complete = true;
}

static int idtp9415_check_fwupdate(enum wireless_mode sram_mode)
{
	return 0;
}

static int idtp9415_dev_check(void)
{
	int ret;
	u16 chip_id = 0;

	wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_ON);
	usleep_range(9500, 10500); /* 10ms */
	ret = idtp9415_get_chip_id(&chip_id);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_OFF);
		return ret;
	}
	wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_OFF);

	hwlog_info("[%s] chip_id = 0x%x\n", __func__, chip_id);

	if (chip_id != IDTP9415_CHIP_ID) {
		hwlog_err("%s: wlc_chip not match\n", __func__);
		return -WLC_ERR_MISMATCH;
	}

	return 0;
}

static int idtp9415_parse_vrect_coef(struct device_node *np,
	struct idtp9415_dev_info *di)
{
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_vrect_coef_5v", di->rx_vrect_coef_5v,
		IDTP9415_RX_VRECT_COEF_LEN))
		return -EINVAL;
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_vrect_coef_9v", di->rx_vrect_coef_9v,
		IDTP9415_RX_VRECT_COEF_LEN))
		return -EINVAL;
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_vrect_coef_12v", di->rx_vrect_coef_12v,
		IDTP9415_RX_VRECT_COEF_LEN))
		return -EINVAL;
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_vrect_coef_sc", di->rx_vrect_coef_sc,
		IDTP9415_RX_VRECT_COEF_LEN))
		return -EINVAL;

	return 0;
}

static int idtp9415_parse_fod(struct device_node *np,
	struct idtp9415_dev_info *di)
{
	if (power_dts_read_u16_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_5v", di->rx_fod_5v, IDTP9415_RX_FOD_LEN))
		return -EINVAL;
	if (power_dts_read_u16_array(power_dts_tag(HWLOG_TAG), np,
		"pu_rx_fod_5v", di->pu_rx_fod_5v, IDTP9415_RX_FOD_LEN))
		di->need_chk_pu_shell = false;
	else
		di->need_chk_pu_shell = true;
	hwlog_info("[%s] need_chk_pu_shell=%d\n",
		__func__, di->need_chk_pu_shell);

	if (power_dts_read_u16_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_9v", di->rx_fod_9v, IDTP9415_RX_FOD_LEN))
		return -EINVAL;

	if (power_dts_read_u16_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_12v", di->rx_fod_12v, IDTP9415_RX_FOD_LEN))
		return -EINVAL;

	if (power_dts_read_u16_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_15v", di->rx_fod_15v, IDTP9415_RX_FOD_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"tx_fod_coef", di->tx_fod_coef, IDTP9415_TX_FOD_LEN))
		return -EINVAL;

	return 0;
}

static struct device_node *idtp9415_dts_dev_node(void)
{
	if (!g_idtp9415_di || !g_idtp9415_di->dev)
		return NULL;

	return g_idtp9415_di->dev->of_node;
}

static int idtp9415_parse_dts(struct device_node *np,
	struct idtp9415_dev_info *di)
{
	int ret;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_ss_good_lth", (u32 *)&di->rx_ss_good_lth,
		IDTP9415_RX_SS_MAX);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_en_valid_val", (u32 *)&di->gpio_en_valid_val, 0);

	ret = idtp9415_parse_fod(np, di);
	if (ret) {
		hwlog_err("%s: parse fod para fail\n", __func__);
		return ret;
	}
	ret = idtp9415_parse_vrect_coef(np, di);
	if (ret) {
		hwlog_err("%s: parse vrect coef para fail\n", __func__);
		return ret;
	}

	return 0;
}

static int idtp9415_gpio_init(struct idtp9415_dev_info *di,
	struct device_node *np)
{
	/* gpio_en */
	if (power_gpio_config_output(power_gpio_tag(HWLOG_TAG), np,
		"gpio_en", "idtp9415_en",
		&di->gpio_en, di->gpio_en_valid_val))
		return -EINVAL;

	/* gpio_sleep_en */
	if (power_gpio_config_output(power_gpio_tag(HWLOG_TAG), np,
		"gpio_sleep_en", "idtp9415_sleep_en",
		&di->gpio_sleep_en, RX_SLEEP_EN_DISABLE)) {
		gpio_free(di->gpio_en);
		return -EINVAL;
	}

	/* gpio_pwr_good */
	if (power_gpio_config_input(power_gpio_tag(HWLOG_TAG), np,
		"gpio_pwr_good", "idtp9415_pwr_good", &di->gpio_pwr_good)) {
		gpio_free(di->gpio_sleep_en);
		gpio_free(di->gpio_en);
		return -EINVAL;
	}

	return 0;
}

static int idtp9415_irq_init(struct idtp9415_dev_info *di,
	struct device_node *np)
{
	int ret;

	ret = power_gpio_config_interrupt(power_gpio_tag(HWLOG_TAG), np,
		"gpio_int", "idtp9415_int", &di->gpio_int, &di->irq_int);
	if (ret)
		goto irq_init_fail_0;

	ret = request_irq(di->irq_int, idtp9415_interrupt,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "idtp9415_irq", di);
	if (ret) {
		hwlog_err("%s: could not request idtp9415_irq\n", __func__);
		di->irq_int = -EINVAL;
		goto irq_init_fail_1;
	}
	enable_irq_wake(di->irq_int);
	di->irq_active = true;
	INIT_WORK(&di->irq_work, idtp9415_irq_work);

	return 0;

irq_init_fail_1:
	gpio_free(di->gpio_int);
irq_init_fail_0:
	return ret;
}

static struct wireless_charge_device_ops idtp9415_rx_ops = {
	.dev_node               = idtp9415_dts_dev_node,
	.chip_init              = idtp9415_chip_init,
	.chip_reset             = idtp9415_chip_reset,
	.get_die_id             = idtp9415_get_die_id,
	.get_rx_def_imax        = idtp9415_rx_def_max_iout,
	.get_rx_vrect           = idtp9415_get_rx_vrect,
	.get_rx_vout            = idtp9415_get_rx_vout,
	.get_rx_iout            = idtp9415_get_rx_iout,
	.get_rx_vout_reg        = idtp9415_get_rx_vout_reg,
	.get_tx_vout_reg        = idtp9415_get_tx_vout_reg,
	.set_tx_vout            = idtp9415_set_tx_vout,
	.set_rx_vout            = idtp9415_set_rx_vout,
	.get_rx_fop             = idtp9415_get_rx_fop,
	.get_rx_cep             = idtp9415_get_rx_cep,
	.get_rx_temp            = idtp9415_get_rx_temp,
	.get_chip_info          = idtp9415_get_chip_info_str,
	.get_rx_chip_id         = idtp9415_get_chip_id,
	.get_rx_fod_coef        = idtp9415_get_rx_fod,
	.set_rx_fod_coef        = idtp9415_set_rx_fod,
	.rx_enable              = idtp9415_chip_enable,
	.rx_sleep_enable        = idtp9415_sleep_enable,
	.check_tx_exist         = idtp9415_check_tx_exist,
	.kick_watchdog          = idtp9415_kick_watchdog,
	.check_fwupdate         = idtp9415_check_fwupdate,
	.send_ept               = idtp9415_send_ept,
	.stop_charging          = idtp9415_stop_charging,
	.pmic_vbus_handler      = idtp9415_pmic_vbus_handler,
	.rx_program_otp         = idtp9415_rx_program_mtp,
	.rx_check_otp           = idtp9415_check_mtp,
	.check_is_otp_exist     = idtp9415_check_is_mtp_exist,
	.need_chk_pu_shell      = idtp9415_need_check_pu_shell,
	.set_pu_shell_flag      = idtp9415_set_pu_shell_flag,
};

static struct wireless_tx_device_ops idtp9415_tx_ops = {
	.pre_ps                 = idtp9415_pre_pwr_supply,
	.rx_enable              = idtp9415_chip_enable,
	.rx_sleep_enable        = idtp9415_sleep_enable,
	.chip_reset             = idtp9415_chip_reset,
	.enable_tx_mode         = idtp9415_enable_tx_mode,
	.tx_chip_init           = idtp9415_tx_chip_init,
	.tx_stop_config         = idtp9415_tx_stop_config,
	.check_fwupdate         = idtp9415_check_fwupdate,
	.kick_watchdog          = idtp9415_kick_watchdog,
	.get_tx_iin             = idtp9415_get_tx_iin,
	.get_tx_vrect           = idtp9415_get_tx_vrect,
	.get_tx_vin             = idtp9415_get_tx_vin,
	.get_chip_temp          = idtp9415_get_tx_temp,
	.get_tx_fop             = idtp9415_get_tx_fop,
	.set_tx_max_fop         = idtp9415_set_tx_max_fop,
	.get_tx_max_fop         = idtp9415_get_tx_max_fop,
	.set_tx_min_fop         = idtp9415_set_tx_min_fop,
	.get_tx_min_fop         = idtp9415_get_tx_min_fop,
	.set_tx_ping_frequency  = idtp9415_set_tx_ping_frequency,
	.get_tx_ping_frequency  = idtp9415_get_tx_ping_frequency,
	.set_tx_ping_interval   = idtp9415_set_tx_ping_interval,
	.get_tx_ping_interval   = idtp9415_get_tx_ping_interval,
	.check_rx_disconnect    = idtp9415_check_rx_disconnect,
	.in_tx_mode             = idtp9415_in_tx_mode,
	.set_tx_open_flag       = idtp9415_set_tx_open_flag,
};

static struct wlps_tx_ops idtp9415_ps_tx_ops = {
	.tx_vset                = idtp9415_tx_mode_vset,
};

static struct qi_protocol_ops idtp9415_qi_protocol_ops = {
	.chip_name              = "idtp9415",
	.send_msg               = idtp9415_send_msg,
	.send_msg_with_ack      = idtp9415_send_msg_ack,
	.receive_msg            = idtp9415_receive_msg,
	.send_fsk_msg           = idtp9415_send_fsk_msg,
	.get_ask_packet         = idtp9415_get_ask_pkt,
	.get_chip_fw_version    = idtp9415_get_fw_version,
	.get_tx_id_pre          = NULL,
	.set_rpp_format_post    = idtp9415_set_rpp_format,
};

static int idtp9415_tx_ps_ops_register(struct idtp9415_dev_info *di)
{
	int ret;
	u32 tx_ps_ctrl_src = 0;

	ret = power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,wireless_ps", "tx_ps_ctrl_src", &tx_ps_ctrl_src, 0);
	if (!ret && (tx_ps_ctrl_src == WLPS_TX_SRC_TX_CHIP))
		return wlps_tx_ops_register(&idtp9415_ps_tx_ops);

	return -WLC_ERR_MISMATCH;

}

static void idtp9415_shutdown(struct i2c_client *client)
{
	int wired_channel_state;

	hwlog_info("[%s] ++\n", __func__);
	wired_channel_state = wireless_charge_get_wireless_channel_state();
	if (wired_channel_state == WIRELESS_CHANNEL_ON) {
		wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_OFF);
		msleep(50); /* dalay 50ms for power off */
		idtp9415_set_tx_vout(ADAPTER_5V * WL_MVOLT_PER_VOLT);
		idtp9415_set_rx_vout(ADAPTER_5V * WL_MVOLT_PER_VOLT);
		idtp9415_chip_enable(RX_EN_DISABLE);
		msleep(IDTP9415_SHUTDOWN_SLEEP_TIME);
		idtp9415_chip_enable(RX_EN_ENABLE);
	}
	hwlog_info("[%s] --\n", __func__);
}

static int idtp9415_check_dev_fail_handler(struct idtp9415_dev_info *di)
{
	int ret;
	int rx_en_val;
	int sleep_en_val;

	rx_en_val = gpio_get_value(di->gpio_en);
	sleep_en_val = gpio_get_value(di->gpio_sleep_en);
	gpio_set_value(di->gpio_en, RX_EN_ENABLE);
	gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_ENABLE);
	usleep_range(9500, 10500); /* 10ms */
	ret = idtp9415_dev_check();
	gpio_set_value(di->gpio_en, rx_en_val);
	gpio_set_value(di->gpio_sleep_en, sleep_en_val);

	return ret;
}

static int idtp9415_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	u16 chip_id = 0;
	bool dev_check_flag = true;
	struct idtp9415_dev_info *di = NULL;
	struct device_node *np = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!client || !client->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_idtp9415_di = di;
	di->dev = &client->dev;
	np = client->dev.of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	ret = idtp9415_dev_check();
	if (ret)
		dev_check_flag = false;

	ret = idtp9415_parse_dts(np, di);
	if (ret)
		goto parse_dts_fail;

	ret = idtp9415_gpio_init(di, np);
	if (ret)
		goto gpio_init_fail;

	if (!dev_check_flag) {
		ret = idtp9415_check_dev_fail_handler(di);
		if (ret)
			goto dev_ck_fail;
	}

	ret = idtp9415_irq_init(di, np);
	if (ret)
		goto irq_init_fail;

	wakeup_source_init(&g_idtp9415_wakelock, "idtp9415_wakelock");
	mutex_init(&di->mutex_irq);

	if (!power_cmdline_is_powerdown_charging_mode()) {
		INIT_DELAYED_WORK(&di->mtp_check_work, idtp9415_mtp_check_work);
		schedule_delayed_work(&di->mtp_check_work,
			msecs_to_jiffies(500)); /* dalay 500ms for power on */
	} else {
		g_mtp_chk_complete = true;
	}
	ret = wireless_charge_ops_register(&idtp9415_rx_ops);
	if (ret) {
		hwlog_err("%s: register ops fail\n", __func__);
		goto ops_regist_fail;
	}

	if (idtp9415_check_tx_exist()) {
		idtp9415_rx_clear_interrupt(di, IDTP9415_ALL_INTR_CLR);
		hwlog_info("[%s] rx exsit, exe rx_ready_handler\n", __func__);
		idtp9415_rx_ready_handler(di);
	} else {
		gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_ENABLE);
	}

	ret = wireless_tx_ops_register(&idtp9415_tx_ops);
	if (ret) {
		hwlog_err("%s: register wireless tx ops fail\n", __func__);
		goto ops_regist_fail;
	}

	qi_protocol_ops_register(&idtp9415_qi_protocol_ops);
	g_idtp9415_handle = qi_protocol_get_handle();
	ret = idtp9415_tx_ps_ops_register(di);
	if (ret) {
		hwlog_err("%s: register wireless tx_ps ops fail\n", __func__);
		goto ops_regist_fail;
	}

	idtp9415_get_chip_id(&chip_id);
	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = chip_id;
		power_dev_info->ver_id = 0;
	}

	hwlog_info("wireless_idtp9415 probe ok\n");
	return 0;

ops_regist_fail:
	gpio_free(di->gpio_int);
	free_irq(di->irq_int, di);
dev_ck_fail:
irq_init_fail:
	gpio_free(di->gpio_en);
	gpio_free(di->gpio_sleep_en);
	if (di->gpio_pwr_good > 0)
		gpio_free(di->gpio_pwr_good);
gpio_init_fail:
parse_dts_fail:
	devm_kfree(&client->dev, di);
	di = NULL;
	g_idtp9415_di = NULL;
	np = NULL;
	return ret;
}

MODULE_DEVICE_TABLE(i2c, wireless_idtp9415);
static const struct of_device_id idtp9415_of_match[] = {
	{
		.compatible = "huawei, wireless_idtp9415",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id idtp9415_i2c_id[] = {
	{ "wireless_idtp9415", 0 }, {}
};

static struct i2c_driver idtp9415_driver = {
	.probe = idtp9415_probe,
	.shutdown = idtp9415_shutdown,
	.id_table = idtp9415_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "wireless_idtp9415",
		.of_match_table = of_match_ptr(idtp9415_of_match),
	},
};

static int __init idtp9415_init(void)
{
	return i2c_add_driver(&idtp9415_driver);
}

static void __exit idtp9415_exit(void)
{
	i2c_del_driver(&idtp9415_driver);
}

device_initcall(idtp9415_init);
module_exit(idtp9415_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("idtp9415 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
