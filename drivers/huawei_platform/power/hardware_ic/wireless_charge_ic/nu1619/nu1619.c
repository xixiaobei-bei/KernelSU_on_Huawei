/*
 * nu1619.c
 *
 * nu1619 driver
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/hardware_ic/charge_pump.h>
#include <huawei_platform/power/huawei_charger.h>
#include <chipset_common/hwpower/wireless_charge/wireless_firmware.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <huawei_platform/power/wireless/wireless_charger.h>
#include <huawei_platform/power/wireless/wireless_direct_charger.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include "nu1619.h"
#include "nu1619_mtp.h"

#define HWLOG_TAG wireless_nu1619
HWLOG_REGIST();

static struct qi_protocol_handle *g_nu1619_handle;
static struct nu1619_dev_info *g_nu1619_di;
static struct wakeup_source g_nu1619_wakelock;
static bool g_stop_charging_flag;
static bool g_irq_abnormal_flag;
static bool g_tx_open_flag;
static bool g_mtp_chk_complete;
static bool g_activate_tx_chip_flag;
static bool g_wr_reg_busy_flag;

static const u8 g_nu1619_rx2tx_header[NU1619_RX2TX_PKT_LEN] = {
	0, 0x18, 0x28, 0x38, 0x48, 0x58
};

static const u8 g_nu1619_tx2rx_header[NU1619_TX2RX_PKT_LEN] = {
	0, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f
};

static struct nu1619_read_reg g_rd_regs[] = {
	{ 0x000c, 0x0008, 0 }, { 0x000e, 0x0020, 0 }
};

static bool nu1619_is_pwr_good(void)
{
	int gpio_val;
	struct nu1619_dev_info *di = g_nu1619_di;

	if (!di)
		return false;
	if (di->gpio_pwr_good <= 0)
		return true;
	if (!g_mtp_chk_complete)
		return true;
	if (g_activate_tx_chip_flag)
		return true;

	gpio_val = gpio_get_value(di->gpio_pwr_good);

	return gpio_val == NU1619_GPIO_PWR_GOOD_VAL;
}

static int nu1619_i2c_read(struct i2c_client *client, u8 *cmd, int cmd_len,
	u8 *buf, int buf_len)
{
	int i;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!nu1619_is_pwr_good())
			return -WLC_ERR_I2C_R;
		if (!power_i2c_read_block(client, cmd, cmd_len, buf, buf_len))
			return 0;
		usleep_range(9500, 10500); /* 10ms */
	}

	return -WLC_ERR_I2C_R;
}

static int nu1619_i2c_write(struct i2c_client *client, u8 *buf, int buf_len)
{
	int i;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!nu1619_is_pwr_good())
			return -WLC_ERR_I2C_W;
		if (!power_i2c_write_block(client, buf, buf_len))
			return 0;
		usleep_range(9500, 10500); /* 10ms */
	}

	return -WLC_ERR_I2C_W;
}

static bool nu1619_is_mtp_reg_invalid(u16 reg)
{
	if (g_mtp_chk_complete)
		return true;

	if ((reg == NU1619_MTP_WRITE_HEAD_ADDR) ||
		(reg == NU1619_MTP_DOWNLOAD_ADDR) ||
		(reg == NU1619_DTM_REG0_ADDR) ||
		(reg == NU1619_DTM_REG1_ADDR) ||
		(reg == NU1619_DTM_REG2_ADDR) ||
		(reg == NU1619_DTM_REG3_ADDR) ||
		(reg == NU1619_DTM_REG4_ADDR) ||
		(reg == NU1619_MTP_WM_REG0) ||
		(reg == NU1619_MTP_WM_REG1) ||
		(reg == NU1619_MTP_WM_REG2) ||
		(reg == NU1619_MTP_WM_REG3) ||
		(reg == NU1619_MTP_WM_REG4) ||
		(reg == NU1619_MTP_CHIP_ID_ADDR) ||
		(reg == NU1619_MTP_FW_VER_ADDR) ||
		(reg == NU1619_MTP_FW_CRC_ADDR) ||
		(reg == NU1619_CMD_WRITE_REG))
		return true;

	return false;
}

static int nu1619_read_block(struct nu1619_dev_info *di,
	u16 reg, u8 *data, u8 len)
{
	u8 cmd[NU1619_ADDR_LEN];

	if (!di || !data) {
		hwlog_err("read_block: para null\n");
		return -WLC_ERR_PARA_NULL;
	}
	if (!nu1619_is_mtp_reg_invalid(reg))
		return -WLC_ERR_PARA_WRONG;

	/* double bytes addr */
	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;

	return nu1619_i2c_read(di->client, cmd, NU1619_ADDR_LEN, data, len);
}

static int nu1619_write_block(struct nu1619_dev_info *di,
	u16 reg, u8 *data, int data_len)
{
	u8 cmd[NU1619_ADDR_LEN + data_len];

	if (!di || !data) {
		hwlog_err("write_block: para null\n");
		return -WLC_ERR_PARA_NULL;
	}
	if (!nu1619_is_mtp_reg_invalid(reg))
		return -WLC_ERR_PARA_WRONG;

	/* double bytes addr */
	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;
	memcpy(&cmd[NU1619_ADDR_LEN], data, data_len);

	return nu1619_i2c_write(di->client, cmd,
		NU1619_ADDR_LEN + data_len);
}

static int nu1619_read_byte(u16 reg, u8 *data)
{
	struct nu1619_dev_info *di = g_nu1619_di;

	if (!di) {
		hwlog_err("read_byte: di null\n");
		return -WLC_ERR_PARA_NULL;
	}

	return nu1619_read_block(di, reg, data, BYTE_LEN);
}

static int nu1619_read_word(u16 reg, u16 *data)
{
	int ret;
	u8 buff[WORD_LEN] = { 0 };
	struct nu1619_dev_info *di = g_nu1619_di;

	if (!di || !data) {
		hwlog_err("read_word: di null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = nu1619_read_block(di, reg, buff, WORD_LEN);
	if (ret)
		return -WLC_ERR_I2C_R;

	/* B0: low byte; B1: high byte */
	*data = buff[0] | (buff[1] << BITS_PER_BYTE);
	return 0;
}

static int nu1619_write_byte(u16 reg, u8 data)
{
	struct nu1619_dev_info *di = g_nu1619_di;

	if (!di) {
		hwlog_err("write_byte: di null\n");
		return -WLC_ERR_PARA_NULL;
	}

	return nu1619_write_block(di, reg, &data, BYTE_LEN);
}

static int nu1619_write_word(u16 reg, u16 data)
{
	u8 buff[WORD_LEN];
	struct nu1619_dev_info *di = g_nu1619_di;

	if (!di) {
		hwlog_err("write_word: di null\n");
		return -WLC_ERR_PARA_NULL;
	}

	/* B0: low byte; B1: high byte */
	buff[0] = data & BYTE_MASK;
	buff[1] = data >> BITS_PER_BYTE;

	return nu1619_write_block(di, reg, buff, WORD_LEN);
}

static bool nu1619_is_read_regs_busy(int *id)
{
	int i;
	int j;

	for (i = 0; i < NU1619_CMD_READ_RETRY_CNT; i++) {
		for (j = 0; j < NU1619_CMD_READ_REGS_SIZE; j++) {
			if (g_rd_regs[j].busy_flag)
				continue;
			g_rd_regs[j].busy_flag = true;
			*id = j;
			return false;
		}
		usleep_range(950, 1050); /* 1ms */
	}

	return true;
}

static bool nu1619_is_write_regs_busy(void)
{
	int i;

	for (i = 0; i < NU1619_CMD_WRITE_RETRY_CNT; i++) {
		usleep_range(950, 1050); /* 1ms */
		if (g_wr_reg_busy_flag)
			continue;

		g_wr_reg_busy_flag = true;
		return false;
	}
	return true;
}

static int nu1619_read_cmd_data(u8 cmd, u8 *data, u8 len)
{
	int i;
	int ret;
	int cnt;
	int id = 0;
	u8 v_cmd = cmd;
	u8 temp[NU1619_CMD_DATA_TOTAL] = { 0 };
	struct nu1619_dev_info *di = g_nu1619_di;

	if (!di || !data || (len <= 0))
		return -WLC_ERR_PARA_WRONG;

	if (nu1619_is_read_regs_busy(&id)) {
		hwlog_err("read cmd regs busy, cmd:0x%x\n", cmd);
		return -WLC_ERR_NO_SPACE;
	}
	/* index starting from 0 */
	if ((id < 0) || (id > NU1619_CMD_READ_REGS_SIZE - 1))
		return -WLC_ERR_PARA_WRONG;

	/* up to two bytes each time, read multiple times */
	cnt = len % NU1619_CMD_REG_DATA_LEN  + len / NU1619_CMD_REG_DATA_LEN;
	for (i = 0; i < cnt; i++) {
		ret = nu1619_write_byte(g_rd_regs[id].cmd_addr, v_cmd);
		if (ret)
			goto read_err;
		usleep_range(1000, 1050); /* 1ms */
		ret = nu1619_read_block(di, g_rd_regs[id].data_addr,
			temp, NU1619_CMD_DATA_TOTAL);
		if (ret)
			goto read_err;
		if (temp[NU1619_CMD_HEAD] != (v_cmd ^ NU1619_CMD_READ_MASK))
			goto read_err;
		if (len-- > 0)
			*data++ = temp[NU1619_CMD_DATA0];
		if (len-- > 0)
			*data++ = temp[NU1619_CMD_DATA1];
		v_cmd += NU1619_CMD_REG_WORD_LEN;
	}
	g_rd_regs[id].busy_flag = false;
	return 0;

read_err:
	g_rd_regs[id].busy_flag = false;
	return -WLC_ERR_I2C_WR;
}

static int nu1619_write_cmd_data(u8 cmd, u8 *data, u8 len)
{
	int ret = 0;
	static bool cmd_flag; /* cmd bit5 need inverse everytime */
	struct nu1619_dev_info *di = g_nu1619_di;

	if (!di) {
		hwlog_err("write_cmd_data: di null\n");
		return -WLC_ERR_PARA_NULL;
	}
	if (nu1619_is_write_regs_busy()) {
		hwlog_err("write cmd regs busy, cmd:0x%x\n", cmd);
		return -WLC_ERR_NO_SPACE;
	}
	if (cmd_flag) {
		cmd_flag = false;
		cmd |= NU1619_CMD_WRITE_MASK;
	} else {
		cmd_flag = true;
		cmd &= ~NU1619_CMD_WRITE_MASK;
	}
	if (data && len > 0)
		ret += nu1619_write_block(di, NU1619_CMD_WRITE_DATA1_REG,
			data, len);
	ret += nu1619_write_byte(NU1619_CMD_WRITE_REG, cmd);

	usleep_range(4500, 5500); /* delay 5ms for trx chip handle data */
	g_wr_reg_busy_flag = false;
	return ret;
}

/* nu1619 chip_info */

static int nu1619_get_chip_id(u16 *chip_id)
{
	return nu1619_read_cmd_data(NU1619_REG_CMD_CHIP_ID, (u8 *)chip_id,
		NU1619_CMD_REG_WORD_LEN);
}

static int nu1619_get_mtp_fw_version(u32 *mtp_fw_version)
{
	/* fw_version: boot,rx,tx,fw version,chip_code:Differentiate product */
	return nu1619_read_cmd_data(NU1619_REG_CMD_MTP_FW_VER,
		(u8 *)mtp_fw_version,
		NU1619_MTP_FW_VER_LEN + NU1619_MTP_FW_CHIP_CODE_LEN);
}

static int nu1619_get_chip_info(struct nu1619_chip_info *info)
{
	int ret;

	ret = nu1619_get_chip_id(&info->chip_id);
	if (ret)
		return ret;

	ret = nu1619_get_mtp_fw_version(&info->mtp_fw_ver);
	if (ret)
		return ret;

	return 0;
}

static int nu1619_get_chip_info_str(char *info_str, int len, void *dev_data)
{
	int ret;
	struct nu1619_chip_info chip_info;

	if (!info_str || (len < WLTRX_IC_CHIP_INFO_LEN))
		return -EINVAL;

	ret = nu1619_get_chip_info(&chip_info);
	if (ret)
		return -EIO;

	return snprintf(info_str, WLTRX_IC_CHIP_INFO_LEN, "chip_id:nu%x fw:%x",
		chip_info.chip_id, chip_info.mtp_fw_ver);
}

static int nu1619_get_fw_version(u8 *data, int len, void *dev_data)
{
	struct nu1619_chip_info chip_info;

	if (!data)
		return -EINVAL;

	if (nu1619_get_chip_info(&chip_info)) {
		hwlog_err("get_fw_version: get chip info fail\n");
		return -EIO;
	}

	/* fw version length must be 4 */
	if (len != 4)
		return -EINVAL;

	/* data[0:2]=boot,rx,tx version, data[3]=chip code */
	data[0] = (u8)((chip_info.mtp_fw_ver >> 0) & BYTE_MASK);
	data[1] = (u8)((chip_info.mtp_fw_ver >> 8) & BYTE_MASK);
	data[2] = (u8)((chip_info.mtp_fw_ver >> 16) & BYTE_MASK);
	data[3] = (u8)((chip_info.mtp_fw_ver >> 24) & BYTE_MASK);

	return 0;
}

static int nu1619_get_mode(u8 *mode)
{
	int ret;

	if (!mode)
		return WLC_ERR_PARA_NULL;

	ret = nu1619_read_byte(NU1619_OP_MODE_ADDR, mode);
	if (ret) {
		hwlog_err("get_mode: fail\n");
		return -WLC_ERR_I2C_R;
	}

	return 0;
}

/* nu1619 rx mode */

static void nu1619_wake_lock(void)
{
	if (g_nu1619_wakelock.active) {
		hwlog_info("wake_lock: already locked\n");
		return;
	}
	__pm_stay_awake(&g_nu1619_wakelock);
	hwlog_info("wake_lock\n");
}

static void nu1619_wake_unlock(void)
{
	if (!g_nu1619_wakelock.active) {
		hwlog_info("wake_unlock: already unlocked\n");
		return;
	}
	__pm_relax(&g_nu1619_wakelock);
	hwlog_info("wake_unlock\n");
}

static void nu1619_enable_irq(struct nu1619_dev_info *di)
{
	mutex_lock(&di->mutex_irq);
	if (!di->irq_active) {
		hwlog_info("[enable_irq] ++\n");
		enable_irq(di->irq_int);
		di->irq_active = true;
	}
	hwlog_info("[enable_irq] --\n");
	mutex_unlock(&di->mutex_irq);
}

static void nu1619_disable_irq_nosync(struct nu1619_dev_info *di)
{
	mutex_lock(&di->mutex_irq);
	if (di->irq_active) {
		hwlog_info("[disable_irq_nosync] ++\n");
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
	}
	hwlog_info("[disable_irq_nosync] --\n");
	mutex_unlock(&di->mutex_irq);
}

static void nu1619_chip_enable(bool enable, void *dev_data)
{
	int gpio_val;
	struct nu1619_dev_info *di = dev_data;

	if (!di)
		return;

	gpio_set_value(di->gpio_en,
		enable ? di->gpio_en_valid_val : !di->gpio_en_valid_val);
	gpio_val = gpio_get_value(di->gpio_en);
	hwlog_info("[%s] gpio is %s now\n",
		__func__, gpio_val ? "high" : "low");
}

static bool nu1619_is_chip_enable(void *dev_data)
{
	int gpio_val;
	struct nu1619_dev_info *di = dev_data;

	if (!di)
		return false;

	gpio_val = gpio_get_value(di->gpio_en);
	return gpio_val == di->gpio_en_valid_val;
}

static void nu1619_sleep_enable(bool enable, void *dev_data)
{
	int gpio_val;
	struct nu1619_dev_info *di = dev_data;

	if (!di || g_irq_abnormal_flag)
		return;

	gpio_set_value(di->gpio_sleep_en,
		enable ? WLTRX_IC_SLEEP_ENABLE : WLTRX_IC_SLEEP_DISABLE);
	gpio_val = gpio_get_value(di->gpio_sleep_en);
	hwlog_info("[%s] gpio is %s now\n",
		__func__, gpio_val ? "high" : "low");
}

static bool nu1619_is_sleep_enable(void *dev_data)
{
	int gpio_val;
	struct nu1619_dev_info *di = dev_data;

	if (!di)
		return false;

	gpio_val = gpio_get_value(di->gpio_sleep_en);
	return gpio_val == WLTRX_IC_SLEEP_ENABLE;
}

static void nu1619_chip_reset(void *dev_data)
{
	(void)nu1619_write_cmd_data(NU1619_REG_CMD_CHIP_RESET, NULL, 0);
}

static int nu1619_get_rx_temp(int *temp, void *dev_data)
{
	s8 l_temp = 0;

	if (!temp || nu1619_read_cmd_data(NU1619_REG_CMD_RX_TEMP,
		(u8 *)&l_temp, NU1619_CMD_REG_BYTE_LEN))
		return -EINVAL;

	*temp = l_temp;
	return 0;
}

static int nu1619_get_rx_fop(int *fop, void *dev_data)
{
	int ret;

	ret = nu1619_read_cmd_data(NU1619_REG_CMD_FREQ, (u8 *)fop,
		NU1619_CMD_REG_WORD_LEN);
	if (ret)
		return -WLC_ERR_I2C_R;

	if (*fop)
		*fop = NU1619_RX_FREQ_COUNT_VAL / *fop;
	return 0;
}

static int nu1619_get_rx_cep(int *cep, void *dev_data)
{
	s8 l_cep = 0;

	if (!cep || nu1619_read_cmd_data(NU1619_REG_CMD_RX_CEP_VAL,
		(u8 *)&l_cep, NU1619_CMD_REG_BYTE_LEN))
		return -EINVAL;

	*cep = l_cep;
	return 0;
}

static int nu1619_get_rx_vrect(int *vrect, void *dev_data)
{
	return nu1619_read_cmd_data(NU1619_REG_CMD_RX_VRECT, (u8 *)vrect,
		NU1619_CMD_REG_WORD_LEN);
}

static int nu1619_get_rx_vout(int *vout, void *dev_data)
{
	return nu1619_read_cmd_data(NU1619_REG_CMD_RX_VOUT, (u8 *)vout,
		NU1619_CMD_REG_WORD_LEN);
}

static int nu1619_get_rx_iout(int *iout, void *dev_data)
{
	return nu1619_read_cmd_data(NU1619_REG_CMD_RX_IOUT, (u8 *)iout,
		NU1619_CMD_REG_WORD_LEN);
}

static int nu1619_get_rx_vout_reg(int *vreg, void *dev_data)
{
	return nu1619_read_cmd_data(NU1619_REG_CMD_RX_VOUT_REG, (u8 *)vreg,
		NU1619_CMD_REG_WORD_LEN);
}

static int nu1619_get_vfc_reg(int *vfc_reg, void *dev_data)
{
	return nu1619_read_cmd_data(NU1619_REG_CMD_TX_VOUT_REG, (u8 *)vfc_reg,
		NU1619_CMD_REG_WORD_LEN);
}

static void nu1619_ext_pwr_post_ctrl(bool flag, void *dev_data)
{
	int val;
	struct nu1619_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("ext_pwr_post_ctrl: di null\n");
		return;
	}
	if (flag)
		val = NU1619_PWR_CTRL_EXTRA;
	else
		val = NU1619_PWR_CTRL_AC;

	usleep_range(9500, 10500); /* delay 10ms for power stability */
	gpio_set_value(di->gpio_pwr_ctrl, val);
	hwlog_info("[ext_pwr_post_ctrl] gpio_pwr_ctrl %s\n", flag ? "on" : "off");
}

static void nu1619_clear_rx_interrupt(u16 intr)
{
	if (nu1619_write_cmd_data(NU1619_REG_CMD_RX_INT_CLR, (u8 *)&intr,
		NU1619_CMD_REG_WORD_LEN))
		hwlog_err("clear_rx_interrupt: fail\n");
}

static int nu1619_set_rx_vout(int vol, void *dev_data)
{
	int ret;

	if ((vol < NU1619_RX_VOUT_MIN) || (vol > NU1619_RX_VOUT_MAX)) {
		hwlog_err("set_rx_vout: out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	ret = nu1619_write_cmd_data(NU1619_REG_CMD_SET_RX_VOUT, (u8 *)&vol,
		NU1619_CMD_REG_WORD_LEN);
	if (ret) {
		hwlog_err("set_rx_vout: fail\n");
		return -WLC_ERR_I2C_R;
	}

	return 0;
}

static bool nu1619_is_cp_really_open(void *dev_data)
{
	int rx_ratio;
	int rx_vset = 0;
	int rx_vout = 0;
	int cp_vout;

	if (!charge_pump_is_cp_open(CP_TYPE_MAIN))
		return false;

	rx_ratio = charge_pump_get_cp_ratio(CP_TYPE_MAIN);
	(void)nu1619_get_rx_vout_reg(&rx_vset, dev_data);
	(void)nu1619_get_rx_vout(&rx_vout, dev_data);
	cp_vout = charge_pump_get_cp_vout(CP_TYPE_MAIN);
	cp_vout = (cp_vout > 0) ? cp_vout : wldc_get_ls_vbus();

	hwlog_info("[is_cp_really_open]\t"
		"rx_ratio:%d rx_vset:%d rx_vout:%d cp_vout:%d\n",
		rx_ratio, rx_vset, rx_vout, cp_vout);
	if ((cp_vout * rx_ratio) < (rx_vout - NU1619_RX_FC_ERR_LTH))
		return false;
	if ((cp_vout * rx_ratio) > (rx_vout + NU1619_RX_FC_ERR_UTH))
		return false;

	return true;
}

static int nu1619_check_cp_mode(void *dev_data)
{
	int i;
	int cnt;
	int ret;

	ret = charge_pump_set_cp_mode(CP_TYPE_MAIN);
	if (ret) {
		hwlog_err("check_cp_mode: set cp mode fail\n");
		return ret;
	}
	cnt = NU1619_BPCP_TIMEOUT / NU1619_BPCP_SLEEP_TIME;
	for (i = 0; i < cnt; i++) {
		msleep(NU1619_BPCP_SLEEP_TIME);
		if (nu1619_is_cp_really_open(dev_data)) {
			hwlog_info("[check_cp_mode] set cp mode succ\n");
			return 0;
		}
		if (g_stop_charging_flag)
			return -WLC_ERR_STOP_CHRG;
	}

	return -WLC_ERR_MISMATCH;
}

static int nu1619_send_fc_cmd(int vset)
{
	int ret;

	ret = nu1619_write_cmd_data(NU1619_REG_CMD_SEND_FC, (u8 *)&vset,
		NU1619_CMD_REG_WORD_LEN);
	if (ret) {
		hwlog_err("send_fc_cmd: send fc cmd fail\n");
		return ret;
	}

	return 0;
}

static bool nu1619_is_fc_succ(int vset, void *dev_data)
{
	int i;
	int cnt;
	int vout = 0;

	cnt = NU1619_RX_FC_TIMEOUT / NU1619_RX_FC_SLEEP_TIME;
	for (i = 0; i < cnt; i++) {
		if (g_stop_charging_flag && (vset > NU1619_RX_FC_DEF_VOLT))
			return false;
		msleep(NU1619_RX_FC_SLEEP_TIME);
		(void)nu1619_get_rx_vout(&vout, dev_data);
		if ((vout >= vset - NU1619_RX_FC_ERR_LTH) &&
			(vout <= vset + NU1619_RX_FC_ERR_UTH)) {
			hwlog_info("[is_fc_succ] succ, cost_time: %dms\n",
				(i + 1) * NU1619_RX_FC_SLEEP_TIME);
			return true;
		}
	}

	return false;
}

static int nu1619_set_vfc(int vfc, void *dev_data)
{
	int ret;
	int i;

	if (vfc >= RX_HIGH_VOUT2) {
		ret = nu1619_check_cp_mode(dev_data);
		if (ret)
			return ret;
	}

	for (i = 0; i < NU1619_RX_FC_RETRY_CNT; i++) {
		if (g_stop_charging_flag && (vfc > NU1619_RX_FC_DEF_VOLT))
			return -WLC_ERR_STOP_CHRG;
		ret = nu1619_send_fc_cmd(vfc);
		if (ret) {
			hwlog_err("set_vfc: send fc_cmd fail\n");
			continue;
		}
		hwlog_info("[set_vfc] send fc cmd, cnt: %d\n", i);
		if (nu1619_is_fc_succ(vfc, dev_data)) {
			if (vfc < RX_HIGH_VOUT2)
				(void)charge_pump_set_bp_mode(CP_TYPE_MAIN);
			hwlog_info("[set_vfc] succ\n");
			return 0;
		}
	}

	return -WLC_ERR_MISMATCH;
}

static int nu1619_send_msg(u8 cmd, u8 *data, int data_len, void *dev_data)
{
	int ret;
	u8 header;
	u8 write_data[NU1619_RX2TX_DATA_LEN] = { 0 };
	struct nu1619_dev_info *di = g_nu1619_di;

	if (!di) {
		hwlog_err("send_msg: di null\n");
		return -WLC_ERR_PARA_NULL;
	}
	if ((data_len > NU1619_RX2TX_DATA_LEN) || (data_len < 0)) {
		hwlog_err("send_msg: send data number out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	di->irq_val &= ~NU1619_RX_INT_SEND_PKT_SUCC;
	/* msg_len=cmd_len+data_len  cmd_len=1 */
	header = g_nu1619_rx2tx_header[data_len + 1];
	ret = nu1619_write_byte(NU1619_CMD_WRITE_DATA1_REG, header);
	if (ret) {
		hwlog_err("send_msg: write header fail\n");
		return -WLC_ERR_I2C_W;
	}
	ret = nu1619_write_byte(NU1619_CMD_WRITE_DATA2_REG, cmd);
	if (ret) {
		hwlog_err("send_msg: write cmd fail\n");
		return -WLC_ERR_I2C_W;
	}
	if (data && (data_len > 0)) {
		memcpy(write_data, data, data_len);
		ret = nu1619_write_block(di,
			NU1619_CMD_WRITE_DATA3_REG, write_data, data_len);
		if (ret) {
			hwlog_err("send_msg: write data fail\n");
			return -WLC_ERR_I2C_W;
		}
	}

	ret = nu1619_write_cmd_data(NU1619_REG_CMD_SEND_MSG, NULL, 0);
	if (ret) {
		hwlog_err("send_msg: send RX msg to TX fail\n");
		return -WLC_ERR_I2C_W;
	}
	hwlog_info("[send_msg] send msg cmd:0x%x success\n", cmd);
	return 0;
}

static int nu1619_send_msg_ack(u8 cmd, u8 *data, int data_len, void *dev_data)
{
	int ret;
	int count = 0;
	int ack_cnt;
	struct nu1619_dev_info *di = g_nu1619_di;

	if (!di) {
		hwlog_err("send_msg_ack: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	do {
		(void)nu1619_send_msg(cmd, data, data_len, di);
		for (ack_cnt = 0;
			ack_cnt < NU1619_WAIT_FOR_ACK_RETRY_CNT; ack_cnt++) {
			msleep(NU1619_WAIT_FOR_ACK_SLEEP_TIME);
			if (NU1619_RX_INT_SEND_PKT_SUCC & di->irq_val) {
				di->irq_val &= ~NU1619_RX_INT_SEND_PKT_SUCC;
				hwlog_info("[send_msg_ack] succ, cnt = %d\n",
					count);
				return 0;
			}
			if (g_stop_charging_flag)
				return -WLC_ERR_STOP_CHRG;
		}
		count++;
		hwlog_info("[send_msg_ack] retry\n");
	} while (count < NU1619_SNED_MSG_RETRY_CNT);

	if (count < NU1619_SNED_MSG_RETRY_CNT) {
		hwlog_info("[send_msg_ack] succ\n");
		return 0;
	}

	ret = nu1619_read_cmd_data(NU1619_REG_CMD_TX2RX_CMD, &cmd,
		NU1619_CMD_REG_BYTE_LEN);
	if (ret) {
		hwlog_err("send_msg_ack: get rcv cmd data fail\n");
		return -WLC_ERR_I2C_R;
	}
	if (cmd != NU1619_CMD_ACK) {
		hwlog_err("[send_msg_ack] fail, ack = 0x%x, cnt = %d\n",
			cmd, count);
		return -WLC_ERR_ACK_TIMEOUT;
	}

	return 0;
}

static int nu1619_receive_msg(u8 *data, int data_len, void *dev_data)
{
	int ret;
	int count = 0;
	struct nu1619_dev_info *di = g_nu1619_di;

	if (!di || !data) {
		hwlog_err("receive_msg: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	do {
		if (di->irq_val & NU1619_RX_INT_TXDATA_RCVD) {
			di->irq_val &= ~NU1619_RX_INT_TXDATA_RCVD;
			goto func_end;
		}
		if (g_stop_charging_flag)
			return -WLC_ERR_STOP_CHRG;
		msleep(NU1619_RCV_MSG_SLEEP_TIME);
		count++;
	} while (count < NU1619_RCV_MSG_SLEEP_CNT);

func_end:
	ret = nu1619_read_cmd_data(NU1619_REG_CMD_TX2RX_CMD, data, data_len);
	if (ret) {
		hwlog_err("receive_msg: fail\n");
		return -WLC_ERR_I2C_R;
	}
	if (!data[0]) { /* data[0]: cmd */
		hwlog_err("receive_msg: no msg received from tx\n");
		return -WLC_ERR_ACK_TIMEOUT;
	}
	hwlog_info("[receive_msg] get tx2rx data[cmd:0x%x] succ\n", data[0]);
	return 0;
}

static void nu1619_send_ept(int ept_type, void *dev_data)
{
	int ret;
	u8 ept = ept_type;

	switch (ept_type) {
	case WIRELESS_EPT_ERR_VRECT:
	case WIRELESS_EPT_ERR_VOUT:
		break;
	default:
		return;
	}
	ret = nu1619_write_cmd_data(NU1619_REG_CMD_SEND_EPT, &ept,
		NU1619_CMD_REG_BYTE_LEN);
	if (ret)
		hwlog_err("send_ept: fail\n");
}

static int nu1619_set_rpp_format(u8 pmax, int mode, void *dev_data)
{
	u8 format = 0x80; /* bit[7:6]=10: Qi RP24 mode No reply */

	format |= (pmax * NU1619_RX_RPP_VAL_UNIT) & NU1619_RX_RPP_VAL_MASK;
	hwlog_info("[set_rpp_format] format=0x%x\n", format);

	return nu1619_write_cmd_data(NU1619_REG_CMD_RPP, &format,
		NU1619_CMD_REG_BYTE_LEN);
}

static bool nu1619_is_tx_exist(void *dev_data)
{
	int ret;
	u8 mode = 0;

	ret = nu1619_get_mode(&mode);
	if (ret) {
		hwlog_err("is_tx_exist: get rx mode fail\n");
		return false;
	}

	return mode == NU1619_RX_MODE;
}

static int nu1619_kick_watchdog(void *dev_data)
{
	u16 kick_val = NU1619_WD_KICK_VAL;

	return nu1619_write_cmd_data(NU1619_REG_CMD_WD, (u8 *)&kick_val,
		NU1619_CMD_REG_WORD_LEN);
}

static int nu1619_check_fwupdate(void *dev_data)
{
	return 0;
}

static int nu1619_dev_check(struct nu1619_dev_info *di, struct device_node *np)
{
	int ret;
	u16 chip_id = 0;

	/* gpio_pwr_ctrl */
	if (power_gpio_config_output(np,
		"gpio_pwr_ctrl", "nu1619_pwr_ctrl",
		&di->gpio_pwr_ctrl, NU1619_PWR_CTRL_AC))
		return -EINVAL;

	g_mtp_chk_complete = true;
	if (nu1619_is_tx_exist(di)) {
		ret = nu1619_get_chip_id(&chip_id);
		if (ret) {
			hwlog_err("[dev_check] tx exist, get chip id failed\n");
			return ret;
		}
		if (chip_id != NU1619_CHIP_ID)
			ret = -EINVAL;
		goto check_exit;
	}
	g_mtp_chk_complete = false;
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, true);
	usleep_range(4500, 5500); /* delay 5ms for power stability */
	gpio_set_value(di->gpio_pwr_ctrl, NU1619_PWR_CTRL_EXTRA);
	usleep_range(9500, 10500); /* delay for power on */

	ret = nu1619_write_byte(NU1619_CHIP_CTRL_ADDR,
		NU1619_ENTER_FW_CHECK_MODE_VAL);
	ret += nu1619_read_word(NU1619_MTP_CHIP_ID_ADDR, &chip_id);
	if (ret) {
		gpio_free(di->gpio_pwr_ctrl);
		wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
		return ret;
	}
	gpio_set_value(di->gpio_pwr_ctrl, NU1619_PWR_CTRL_AC);
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);

check_exit:
	hwlog_info("[dev_check] chip_id = 0x%x\n", chip_id);
	return ret;
}

static int nu1619_get_rx_fod(char *fod_str, int len, void *dev_data)
{
	int i;
	int ret;
	char tmp[NU1619_RX_FOD_TMP_STR_LEN] = { 0 };
	u8 fod_arr[NU1619_RX_FOD_LEN] = { 0 };

	if (!fod_str || (len < WLRX_IC_FOD_COEF_LEN))
		return -EINVAL;

	ret = nu1619_read_cmd_data(NU1619_REG_CMD_RX_FOD_RD, fod_arr,
		NU1619_RX_FOD_LEN);
	if (ret) {
		hwlog_err("get_rx_fod: read fod fail\n");
		return -EIO;
	}

	for (i = 0; i < NU1619_RX_FOD_LEN; i++) {
		snprintf(tmp, NU1619_RX_FOD_TMP_STR_LEN, "%x ", fod_arr[i]);
		strncat(fod_str, tmp, strlen(tmp));
	}

	return strlen(fod_str);
}

static int nu1619_set_rx_fod(const char *fod_str, void *dev_data)
{
	int i;
	int ret;
	u8 val = 0;
	char *cur = (char *)fod_str;
	char *token = NULL;
	const char *sep = " ,";
	u8 fod_arr[NU1619_RX_FOD_LEN] = { 0 };

	if (!fod_str) {
		hwlog_err("set_rx_fod: input fod_str err\n");
		return -WLC_ERR_PARA_NULL;
	}

	for (i = 0; i < NU1619_RX_FOD_LEN; i++) {
		token = strsep(&cur, sep);
		if (!token) {
			hwlog_err("set_rx_fod: input fod_str number err\n");
			return -WLC_ERR_PARA_WRONG;
		}
		ret = kstrtou8(token, POWER_BASE_DEC, &val);
		if (ret) {
			hwlog_err("set_rx_fod: input fod_str type err\n");
			return -WLC_ERR_PARA_WRONG;
		}
		fod_arr[i] = val;
		hwlog_info("[set_rx_fod] fod[%d] = 0x%x\n", i, fod_arr[i]);
	}

	ret = nu1619_write_cmd_data(NU1619_REG_CMD_RX_FOD_WR1, fod_arr,
		NU1619_RX_FOD_SET_LEN);
	ret += nu1619_write_cmd_data(NU1619_REG_CMD_RX_FOD_WR2,
		&fod_arr[NU1619_RX_FOD_SET_LEN], NU1619_RX_FOD_SET_LEN);

	return ret;
}

static int nu1619_init_rx_fod_coef(struct nu1619_dev_info *di, int tx_type)
{
	int ret;
	int vfc_reg = 0;
	u8 *rx_fod = NULL;

	(void)nu1619_get_vfc_reg(&vfc_reg, di);
	hwlog_info("[init_rx_fod_coef] vfc_reg: %dmV\n", vfc_reg);

	if (vfc_reg < 9000) { /* (0, 9)V, set 5v fod */
		rx_fod = di->rx_fod_5v;
	} else if (vfc_reg < 12000) { /* [9, 12)V, set 9V fod */
		if (tx_type == WIRELESS_TX_TYPE_CP39S_HK)
			rx_fod = di->rx_fod_9v_cp39s_hk;
		else
			rx_fod = di->rx_fod_9v;
	} else if (vfc_reg < 15000) { /* [12, 15)V, set 12V fod */
		rx_fod = di->rx_fod_12v;
	} else if (vfc_reg < 18000) { /* [15, 18)V, set 15V fod */
		if (tx_type == WIRELESS_TX_TYPE_CP39S_HK)
			rx_fod = di->rx_fod_15v_cp39s_hk;
		else
			rx_fod = di->rx_fod_15v;
	} else {
		return -WLC_ERR_MISMATCH;
	}

	ret = nu1619_write_cmd_data(NU1619_REG_CMD_RX_FOD_WR1, rx_fod,
		NU1619_RX_FOD_SET_LEN);
	ret += nu1619_write_cmd_data(NU1619_REG_CMD_RX_FOD_WR2,
		&rx_fod[NU1619_RX_FOD_SET_LEN], NU1619_RX_FOD_SET_LEN);

	return ret;
}

static int nu1619_chip_init(unsigned int init_type, unsigned int tx_type, void *dev_data)
{
	int ret = 0;
	u16 wd_set_val = NU1619_WD_SET_VAL;
	struct nu1619_dev_info *di = dev_data;

	if (!di)
		return -WLC_ERR_PARA_NULL;

	hwlog_info("[chip_init] tx_type=%d\n", tx_type);
	switch (init_type) {
	case WIRELESS_CHIP_INIT:
		hwlog_info("[chip_init] DEFAULT_CHIP_INIT\n");
		g_stop_charging_flag = false;
		ret += nu1619_write_cmd_data(NU1619_REG_CMD_WD,
			(u8 *)&wd_set_val, NU1619_CMD_REG_WORD_LEN);
		/* fall through */
	case ADAPTER_5V * WL_MVOLT_PER_VOLT:
		hwlog_info("[chip_init] 5V_CHIP_INIT\n");
		ret += nu1619_write_cmd_data(NU1619_REG_CMD_RX_LDO_CFG,
			di->rx_ldo_cfg_5v, NU1619_LDO_CFG_LEN);
		ret += nu1619_init_rx_fod_coef(di, tx_type);
		break;
	case ADAPTER_9V * WL_MVOLT_PER_VOLT:
		hwlog_info("[chip_init] 9V_CHIP_INIT\n");
		ret += nu1619_write_cmd_data(NU1619_REG_CMD_RX_LDO_CFG,
			di->rx_ldo_cfg_9v, NU1619_LDO_CFG_LEN);
		ret += nu1619_init_rx_fod_coef(di, tx_type);
		break;
	case ADAPTER_12V * WL_MVOLT_PER_VOLT:
		hwlog_info("[chip_init] 12V_CHIP_INIT\n");
		ret += nu1619_write_cmd_data(NU1619_REG_CMD_RX_LDO_CFG,
			di->rx_ldo_cfg_12v, NU1619_LDO_CFG_LEN);
		ret += nu1619_init_rx_fod_coef(di, tx_type);
		break;
	case WILREESS_SC_CHIP_INIT:
		hwlog_info("[chip_init] SC_CHIP_INIT\n");
		ret += nu1619_write_cmd_data(NU1619_REG_CMD_RX_LDO_CFG,
			di->rx_ldo_cfg_sc, NU1619_LDO_CFG_LEN);
		ret += nu1619_init_rx_fod_coef(di, tx_type);
		break;
	default:
		hwlog_info("chip_init: input para is invalid\n");
		break;
	}

	return ret;
}

static void nu1619_stop_charging(void *dev_data)
{
	struct nu1619_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("stop_charging: para null\n");
		return;
	}

	g_stop_charging_flag = true;

	if (!g_irq_abnormal_flag)
		return;

	if (wlrx_get_wired_channel_state() != WIRED_CHANNEL_ON) {
		hwlog_info("[stop_charging] irq_abnormal,keep rx_sw on\n");
		g_irq_abnormal_flag = true;
		wlps_control(di->ic_type, WLPS_RX_SW, true);
	} else {
		di->irq_cnt = 0;
		g_irq_abnormal_flag = false;
		nu1619_enable_irq(di);
		hwlog_info("[stop_charging] w_channel on, en irq\n");
	}
}

static int nu1619_data_received_handler(struct nu1619_dev_info *di)
{
	int ret;
	int i;
	u8 cmd;
	u8 buff[QI_PKT_LEN] = { 0 };

	ret = nu1619_read_cmd_data(NU1619_REG_CMD_TX2RX_HEADER,
		buff, QI_PKT_LEN);
	if (ret) {
		hwlog_err("data_received_handler: read data fail\n");
		return -WLC_ERR_I2C_R;
	}

	cmd = buff[QI_PKT_CMD];
	hwlog_info("[data_received_handler] cmd: 0x%x\n", cmd);
	for (i = QI_PKT_DATA; i < QI_PKT_LEN; i++)
		hwlog_info("[data_received_handler] data: 0x%x\n", buff[i]);

	switch (cmd) {
	case QI_CMD_TX_ALARM:
	case QI_CMD_ACK_BST_ERR:
		di->irq_val &= ~NU1619_RX_INT_TXDATA_RCVD;
		if (g_nu1619_handle &&
			g_nu1619_handle->hdl_non_qi_fsk_pkt)
			g_nu1619_handle->hdl_non_qi_fsk_pkt(buff, QI_PKT_LEN, di);
		break;
	default:
		break;
	}

	return 0;
}

static void nu1619_rx_ready_handler(struct nu1619_dev_info *di)
{
	if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON) {
		hwlog_err("rx_ready_handler: wired channel on, ignore\n");
		return;
	}

	hwlog_info("[rx_ready_handler] rx ready, goto wireless charging\n");
	g_stop_charging_flag = false;
	di->irq_cnt = 0;
	wired_chsw_set_wired_channel(WIRED_CHANNEL_ALL, WIRED_CHANNEL_CUTOFF);
	msleep(CHANNEL_SW_TIME);
	gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
	power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_READY, NULL);
}

static void nu1619_handle_abnormal_irq(struct nu1619_dev_info *di)
{
	static struct timespec64 ts64_timeout;
	struct timespec64 ts64_interval;
	struct timespec64 ts64_now;

	ts64_now = current_kernel_time64();
	ts64_interval.tv_sec = 0;
	ts64_interval.tv_nsec = WIRELESS_INT_TIMEOUT_TH * NSEC_PER_MSEC;

	hwlog_info("[handle_abnormal_irq] irq_cnt = %d\n", ++di->irq_cnt);
	/* power on interrupt happen first time, so start monitor it */
	if (di->irq_cnt == 1) {
		ts64_timeout = timespec64_add_safe(ts64_now, ts64_interval);
		if (ts64_timeout.tv_sec == TIME_T_MAX) {
			di->irq_cnt = 0;
			hwlog_err("handle_abnormal_irq: time ov_flow happen\n");
			return;
		}
	}

	if (timespec64_compare(&ts64_now, &ts64_timeout) >= 0) {
		if (di->irq_cnt >= WIRELESS_INT_CNT_TH) {
			g_irq_abnormal_flag = true;
			wlps_control(di->ic_type, WLPS_RX_SW, true);
			nu1619_disable_irq_nosync(di);
			gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
			hwlog_err("handle_abnormal_irq:\t"
				"more than %d irq in %ds, disable irq\n",
				WIRELESS_INT_CNT_TH,
				WIRELESS_INT_TIMEOUT_TH / WL_MSEC_PER_SEC);
		} else {
			di->irq_cnt = 0;
			hwlog_info("handle_abnormal_irq:\t"
				"less than %d irq in %ds, clr irq cnt\n",
				WIRELESS_INT_CNT_TH,
				WIRELESS_INT_TIMEOUT_TH / WL_MSEC_PER_SEC);
		}
	}
}

static void nu1619_rx_power_on_handler(struct nu1619_dev_info *di)
{
	u8 rx_ss = 0; /* ss: Signal Strength */
	int pwr_flag = RX_PWR_ON_NOT_GOOD;

	if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON) {
		hwlog_err("rx_power_on_handler: wired channel on, ignore\n");
		return;
	}

	nu1619_handle_abnormal_irq(di);
	(void)nu1619_read_cmd_data(NU1619_REG_CMD_RX_SS, &rx_ss,
		NU1619_CMD_REG_BYTE_LEN);
	hwlog_info("[rx_power_on_handler] SS = %u\n", rx_ss);
	if ((rx_ss > di->rx_ss_good_lth) && (rx_ss <= NU1619_RX_SS_MAX))
		pwr_flag = RX_PWR_ON_GOOD;
	power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_PWR_ON, &pwr_flag);
}

static void nu1619_rx_mode_irq_recheck(
	struct nu1619_dev_info *di, u16 irq_val)
{
	if (irq_val & NU1619_RX_INT_READY)
		nu1619_rx_ready_handler(di);
}

static void nu1619_clear_rx_irq_again(struct nu1619_dev_info *di)
{
	int ret;
	u16 irq_value = 0;

	hwlog_err("clear_rx_irq_again: gpio_int low, recheck irq\n");
	ret = nu1619_read_word(NU1619_RX_INT_ADDR, &irq_value);
	if (!ret) {
		hwlog_info("[clear_rx_irq_again] gpio low, recheck irq=0x%x\n",
			irq_value);
		nu1619_rx_mode_irq_recheck(di, irq_value);
	}
	nu1619_clear_rx_interrupt(NU1619_ALL_INTR_CLR);
}

static void nu1619_rx_mode_irq_handler(struct nu1619_dev_info *di)
{
	int ret;

	ret = nu1619_read_word(NU1619_RX_INT_ADDR, &di->irq_val);
	if (ret) {
		hwlog_err("rx_mode_irq_handler: read interrupt fail, clear\n");
		nu1619_clear_rx_interrupt(NU1619_ALL_INTR_CLR);
		nu1619_handle_abnormal_irq(di);
		goto clr_irq_again;
	}

	hwlog_info("[rx_mode_irq_handler] interrupt 0x%04x\n", di->irq_val);
	nu1619_clear_rx_interrupt(di->irq_val);
	if (di->irq_val & NU1619_RX_INT_POWER_ON) {
		di->irq_val &= ~NU1619_RX_INT_POWER_ON;
		nu1619_rx_power_on_handler(di);
	}
	if (di->irq_val & NU1619_RX_INT_READY) {
		di->irq_val &= ~NU1619_RX_INT_READY;
		nu1619_rx_ready_handler(di);
	}
	if (di->irq_val & NU1619_RX_INT_OCP) {
		di->irq_val &= ~NU1619_RX_INT_OCP;
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_OCP, NULL);
	}
	if (di->irq_val & NU1619_RX_INT_OVP) {
		di->irq_val &= ~NU1619_RX_INT_OVP;
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_OVP, NULL);
	}
	if (di->irq_val & NU1619_RX_INT_OTP) {
		di->irq_val &= ~NU1619_RX_INT_OTP;
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_OTP, NULL);
	}
	if (di->irq_val & NU1619_RX_INT_SEND_PKT_SUCC)
		hwlog_info("[rx_mode_irq_handler] send pkt succ\n");
	if (di->irq_val & NU1619_RX_INT_SEND_PKT_TIMEOUT) {
		di->irq_val &= ~NU1619_RX_INT_SEND_PKT_TIMEOUT;
		hwlog_info("[rx_mode_irq_handler] send pkt timeout\n");
	}
	if (di->irq_val & NU1619_RX_INT_TXDATA_RCVD)
		nu1619_data_received_handler(di);

clr_irq_again:
	if (!gpio_get_value(di->gpio_int))
		nu1619_clear_rx_irq_again(di);
}

static void nu1619_pmic_vbus_handler(bool vbus_state, void *dev_data)
{
	int ret;
	int wired_ch_state;
	u16 irq_val = 0;
	struct nu1619_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("pmic_vbus_handler: di null\n");
		return;
	}
	wired_ch_state = wlrx_get_wired_channel_state();
	if (vbus_state && g_irq_abnormal_flag &&
		(wired_ch_state != WIRED_CHANNEL_ON) &&
		nu1619_is_tx_exist(dev_data)) {
		ret = nu1619_read_word(NU1619_RX_INT_ADDR, &irq_val);
		if (ret) {
			hwlog_err("pmic_vbus_handler: read int fail, clear\n");
			return;
		}
		hwlog_info("[pmic_vbus_handler] irq_val = 0x%x\n", irq_val);
		if (irq_val & NU1619_RX_INT_READY) {
			nu1619_clear_rx_interrupt(NU1619_ALL_INTR_CLR);
			nu1619_rx_ready_handler(di);
			di->irq_cnt = 0;
			g_irq_abnormal_flag = false;
			nu1619_enable_irq(di);
		}
	}
}

/* nu1619 tx mode */

static int nu1619_get_tx_vrect(u16 *tx_vrect, void *dev_data)
{
	int ret;

	if (!tx_vrect) {
		hwlog_err("get_tx_vrect: input para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = nu1619_read_cmd_data(NU1619_REG_CMD_TX_VRECT, (u8 *)tx_vrect,
		NU1619_CMD_REG_WORD_LEN);
	if (ret) {
		hwlog_err("get_tx_vrect: fail\n");
		return ret;
	}

	return 0;
}

static int nu1619_get_tx_vin(u16 *tx_vin, void *dev_data)
{
	int ret;

	if (!tx_vin) {
		hwlog_err("get_tx_vin: input para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = nu1619_read_cmd_data(NU1619_REG_CMD_TX_VIN, (u8 *)tx_vin,
		NU1619_CMD_REG_WORD_LEN);
	if (ret) {
		hwlog_err("get_tx_vin: fail\n");
		return ret;
	}

	return 0;
}

static int nu1619_get_tx_iin(u16 *tx_iin, void *dev_data)
{
	int ret;

	if (!tx_iin) {
		hwlog_err("get_tx_iin: input para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = nu1619_read_cmd_data(NU1619_REG_CMD_TX_IIN, (u8 *)tx_iin,
		NU1619_CMD_REG_WORD_LEN);
	if (ret) {
		hwlog_err("get_tx_iin: fail\n");
		return ret;
	}

	return 0;
}

static int nu1619_get_tx_temp(s16 *chip_temp, void *dev_data)
{
	int ret;

	if (!chip_temp) {
		hwlog_err("%s: input para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = nu1619_read_cmd_data(NU1619_REG_CMD_TX_TEMP, (u8 *)chip_temp,
		NU1619_CMD_REG_BYTE_LEN);
	if (ret)
		return -WLC_ERR_I2C_R;

	return 0;
}

static int nu1619_get_tx_fop(u16 *fop, void *dev_data)
{
	int ret;
	u16 data = 0;

	if (!fop) {
		hwlog_err("get_tx_fop: input para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = nu1619_read_cmd_data(NU1619_REG_CMD_TX_FOP, (u8 *)&data,
		NU1619_CMD_REG_WORD_LEN);
	if (ret) {
		hwlog_err("get_tx_fop: fail\n");
		return ret;
	}

	if (data > 0)
		*fop = NU1619_TX_PING_HCLK / data;

	return 0;
}

static int nu1619_enable_tx_mode(bool enable, void *dev_data)
{
	return nu1619_write_cmd_data(NU1619_REG_CMD_EN_TX_MODE, NULL, 0);
}

static int nu1619_get_tx_min_fop(u16 *fop, void *dev_data)
{
	int ret;
	u16 data = 0;

	if (!fop) {
		hwlog_err("get_tx_min_fop: input para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = nu1619_read_cmd_data(NU1619_REG_CMD_TX_MIN_FOP, (u8 *)&data,
		NU1619_CMD_REG_WORD_LEN);
	if (ret) {
		hwlog_err("get_tx_min_fop: fail\n");
		return ret;
	}
	if (data > 0)
		*fop = NU1619_TX_PING_HCLK / data;

	return 0;
}

static int nu1619_set_tx_min_fop(u16 fop, void *dev_data)
{
	int ret;

	if ((fop < NU1619_TX_MIN_FOP_VAL) || (fop > NU1619_TX_MAX_FOP_VAL))
		return -WLC_ERR_PARA_WRONG;

	fop = NU1619_TX_PING_HCLK / fop;
	ret = nu1619_write_cmd_data(NU1619_REG_CMD_SET_TX_MIN_FOP, (u8 *)&fop,
		NU1619_CMD_REG_WORD_LEN);
	if (ret) {
		hwlog_err("set_tx_min_fop: fail\n");
		return ret;
	}

	return 0;
}

static int nu1619_get_tx_max_fop(u16 *fop, void *dev_data)
{
	int ret;
	u16 data = 0;

	if (!fop) {
		hwlog_err("get_tx_max_fop: input para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = nu1619_read_cmd_data(NU1619_REG_CMD_TX_MAX_FOP, (u8 *)&data,
		NU1619_CMD_REG_WORD_LEN);
	if (ret) {
		hwlog_err("get_tx_max_fop: fail\n");
		return ret;
	}
	if (data > 0)
		*fop = NU1619_TX_PING_HCLK / data;

	return 0;
}

static int nu1619_set_tx_max_fop(u16 fop, void *dev_data)
{
	int ret;

	if ((fop < NU1619_TX_MIN_FOP_VAL) || (fop > NU1619_TX_MAX_FOP_VAL))
		return -WLC_ERR_PARA_WRONG;

	fop = NU1619_TX_PING_HCLK / fop;
	ret = nu1619_write_cmd_data(NU1619_REG_CMD_SET_TX_MAX_FOP, (u8 *)&fop,
		NU1619_CMD_REG_WORD_LEN);
	if (ret) {
		hwlog_err("set_tx_max_fop: fail\n");
		return ret;
	}

	return 0;
}

static int nu1619_get_tx_ping_freq(u16 *ping_freq, void *dev_data)
{
	int ret;
	u16 data = 0;

	if (!ping_freq) {
		hwlog_err("get_tx_ping_frequency: input para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = nu1619_read_cmd_data(NU1619_REG_CMD_TX_PINGFRE, (u8 *)&data,
		NU1619_CMD_REG_WORD_LEN);
	if (ret) {
		hwlog_err("get_tx_ping_frequency: fail\n");
		return ret;
	}
	if (data > 0)
		*ping_freq = NU1619_TX_PING_HCLK / data;

	return 0;
}

static int nu1619_set_tx_ping_freq(u16 ping_freq, void *dev_data)
{
	int ret;

	if ((ping_freq < NU1619_TX_PING_FREQUENCY_MIN) ||
		(ping_freq > NU1619_TX_PING_FREQUENCY_MAX)) {
		hwlog_err("set_tx_ping_frequency: value out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	ping_freq = NU1619_TX_PING_HCLK / ping_freq;
	ret = nu1619_write_cmd_data(NU1619_REG_CMD_SET_TX_PING_FREQ,
		(u8 *)&ping_freq, NU1619_CMD_REG_WORD_LEN);
	if (ret) {
		hwlog_err("set_tx_ping_frequency: fail\n");
		return ret;
	}

	return 0;
}

static int nu1619_get_tx_ping_interval(u16 *ping_interval, void *dev_data)
{
	int ret;
	u8 data = 0;

	if (!ping_interval) {
		hwlog_err("get_tx_ping_interval: input para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	ret = nu1619_read_cmd_data(NU1619_REG_CMD_TX_PING_INTERVAL,
		&data, NU1619_CMD_REG_BYTE_LEN);
	if (ret) {
		hwlog_err("get_tx_ping_interval: fail\n");
		return ret;
	}
	*ping_interval = data * NU1619_TX_PING_INTERVAL_STEP;

	return 0;
}

static int nu1619_set_tx_ping_interval(u16 ping_interval, void *dev_data)
{
	int ret;

	if ((ping_interval < NU1619_TX_PING_INTERVAL_MIN) ||
		(ping_interval > NU1619_TX_PING_INTERVAL_MAX)) {
		hwlog_err("set_tx_ping_interval: ping interval out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	ping_interval = ping_interval / NU1619_TX_PING_INTERVAL_STEP;
	ret = nu1619_write_cmd_data(NU1619_REG_CMD_SET_TX_PING_INTERVAL,
		(u8 *)&ping_interval, NU1619_CMD_REG_BYTE_LEN);
	if (ret) {
		hwlog_err("set_tx_ping_interval: fail\n");
		return ret;
	}

	return 0;
}

static bool nu1619_check_rx_disconnect(void *dev_data)
{
	struct nu1619_dev_info *di = dev_data;

	if (!di)
		return true;

	if (di->ept_type & NU1619_TX_EPT_CEP_TIMEOUT) {
		di->ept_type &= ~NU1619_TX_EPT_CEP_TIMEOUT;
		hwlog_info("[check_rx_disconnect] rx disconnect\n");
		return true;
	}

	return false;
}

static bool nu1619_in_tx_mode(void *dev_data)
{
	int ret;
	u8 mode = 0;

	ret = nu1619_read_byte(NU1619_OP_MODE_ADDR, &mode);
	if (ret) {
		hwlog_err("in_tx_mode: read reg fail\n");
		return false;
	}

	return mode == NU1619_TX_MODE;
}

static bool nu1619_in_rx_mode(void *dev_data)
{
	return nu1619_is_tx_exist(dev_data);
}

static int nu1619_set_tx_open_flag(bool enable, void *dev_data)
{
	struct nu1619_dev_info *di = dev_data;

	g_tx_open_flag = enable;
	/*
	 * when nu1619_handle_abnormal_irq disable irq, tx will irq-mask
	 * so, here enable irq
	 */
	if (di)
		nu1619_enable_irq(di);

	return 0;
}

static int nu1619_activate_tx_chip(void *dev_data)
{
	int ret;

	g_activate_tx_chip_flag = true;
	ret = nu1619_write_byte(NU1619_CHIP_CTRL_ADDR,
		NU1619_ENTER_TX_MODE_VAL);
	if (ret)
		hwlog_err("activate_tx_chip: fail\n");

	g_activate_tx_chip_flag = false;
	return 0;
}

static int nu1619_set_tx_fod(struct nu1619_dev_info *di)
{
	int ret;
	u8 tx_fod_en_val = NU1619_TX_FOD_EN_VAL;

	/* fod content: value_compensation_gain, 5v_ploss, 10v_ploss, cnt */
	ret = nu1619_write_cmd_data(NU1619_REG_CMD_SET_TX_FOD, di->tx_fod_coef,
		NU1619_TX_FOD_LEN);
	ret += nu1619_write_cmd_data(NU1619_REG_CMD_TX_FOD_EN, &tx_fod_en_val,
		NU1619_CMD_REG_BYTE_LEN);

	return ret;
}

static int nu1619_set_tx_vset(int tx_vset, void *dev_data)
{
	u8 vset;

	msleep(20); /* 20ms: delay for trx_chip init */
	if (tx_vset == NU1619_PS_TX_VOLT_5V5)
		vset = NU1619_PS_TX_GPIO_PU;
	else if (tx_vset == NU1619_PS_TX_VOLT_6V8)
		vset = NU1619_PS_TX_GPIO_OPEN;
	else if (tx_vset == NU1619_PS_TX_VOLT_10V)
		vset = NU1619_PS_TX_GPIO_PD;
	else
		return -WLC_ERR_PARA_WRONG;

	return nu1619_write_cmd_data(NU1619_REG_CMD_BST_CFG,
		&vset, NU1619_CMD_REG_BYTE_LEN);
}

static int nu1619_tx_chip_init(unsigned int client, void *dev_data)
{
	int ret;
	u16 tx_ocp_val = NU1619_TX_OCP_VAL;
	u16 tx_ovp_val = NU1619_TX_OVP_VAL;
	struct nu1619_dev_info *di = dev_data;

	if (!di)
		return -WLC_ERR_PARA_NULL;

	di->irq_cnt = 0;
	g_irq_abnormal_flag = false;
	/*
	 * when nu1619_handle_abnormal_irq disable irq, tx will irq-mask
	 * so, here enable irq
	 */
	nu1619_enable_irq(di);

	ret = nu1619_write_cmd_data(NU1619_REG_CMD_TX_OCP, (u8 *)&tx_ocp_val,
		NU1619_CMD_REG_WORD_LEN);
	ret += nu1619_write_cmd_data(NU1619_REG_CMD_TX_OVP, (u8 *)&tx_ovp_val,
		NU1619_CMD_REG_WORD_LEN);
	ret += nu1619_set_tx_min_fop(NU1619_TX_PING_FREQUENCY_MIN, di);
	ret += nu1619_set_tx_max_fop(NU1619_TX_PING_FREQUENCY_MAX, di);
	ret += nu1619_set_tx_ping_freq(NU1619_TX_PING_FREQUENCY_INIT, di);
	ret += nu1619_set_tx_ping_interval(NU1619_TX_PING_INTERVAL_INIT, di);
	ret += nu1619_set_tx_fod(di);
	if (ret) {
		hwlog_err("tx_chip_init: write fail\n");
		return -WLC_ERR_I2C_W;
	}
	return 0;
}

static void nu1619_clear_tx_interrupt(u16 intr)
{
	if (nu1619_write_cmd_data(NU1619_REG_CMD_TX_INT_CLR, (u8 *)&intr,
		NU1619_CMD_REG_WORD_LEN))
		hwlog_err("clear_tx_interrupt: fail\n");
}

static int nu1619_get_ask_pkt(u8 *pkt_data, int pkt_data_len, void *dev_data)
{
	int ret;
	int i;
	char buff[NU1619_RX2TX_BUFF_LEN] = { 0 };
	char pkt_str[NU1619_RX2TX_STR_LEN] = { 0 };
	struct nu1619_dev_info *di = g_nu1619_di;

	if (!di || !pkt_data || (pkt_data_len <= 0) ||
		(pkt_data_len > NU1619_RX2TX_PKT_LEN)) {
		hwlog_err("get_ask_pkt: para err\n");
		return -WLC_ERR_PARA_WRONG;
	}

	ret = nu1619_read_cmd_data(NU1619_REG_CMD_RX2TX_HEADER, pkt_data,
		pkt_data_len);
	if (ret) {
		hwlog_err("get_ask_pkt: fail\n");
		return -WLC_ERR_I2C_R;
	}

	for (i = 0; i < pkt_data_len; i++) {
		snprintf(buff, NU1619_RX2TX_BUFF_LEN, "0x%02x ", pkt_data[i]);
		strncat(pkt_str, buff, strlen(buff));
	}
	hwlog_info("[get_ask_pkt] RX back packet: %s\n", pkt_str);

	return 0;
}

static int nu1619_tx_get_ept_type(u16 *ept_type)
{
	int ret;
	u16 data = 0;

	ret = nu1619_read_cmd_data(NU1619_REG_CMD_TX_EPT, (u8 *)&data,
		NU1619_CMD_REG_WORD_LEN);
	if (ret) {
		hwlog_err("tx_get_ept_type: read fail\n");
		return ret;
	}

	*ept_type = data;
	hwlog_info("[tx_get_ept_type] EPT type = 0x%04x", *ept_type);
	ret = nu1619_write_cmd_data(NU1619_REG_CMD_TX_EPT_CLR, NULL, 0);
	if (ret) {
		hwlog_err("tx_get_ept_type: write fail\n");
		return ret;
	}

	return 0;
}

static void nu1619_handle_tx_ept(struct nu1619_dev_info *di)
{
	int ret;

	di->irq_val &= ~NU1619_TX_INT_EPT_TYPE;
	ret = nu1619_tx_get_ept_type(&di->ept_type);
	if (ret)
		return;

	switch (di->ept_type) {
	case NU1619_TX_EPT_CMD:
	case NU1619_TX_EPT_FIRSTCEP:
	case NU1619_TX_EPT_CEP_TIMEOUT:
		di->ept_type &= ~NU1619_TX_EPT_CEP_TIMEOUT;
		hwlog_info("[handle_tx_ept] rx disconnect\n");
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_CEP_TIMEOUT, NULL);
		break;
	case NU1619_TX_EPT_FOD:
		di->ept_type &= ~NU1619_TX_EPT_FOD;
		hwlog_info("[handle_tx_ept] fod happened\n");
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_TX_FOD, NULL);
		break;
	case NU1619_TX_EPT_OVP:
		di->ept_type &= ~NU1619_TX_EPT_OVP;
		hwlog_info("[handle_tx_ept] ovp happened\n");
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_OVP, NULL);
		break;
	case NU1619_TX_EPT_OCP:
		di->ept_type &= ~NU1619_TX_EPT_OCP;
		hwlog_info("[handle_tx_ept] ocp happened\n");
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_OCP, NULL);
		break;
	case NU1619_TX_EPT_TIMEOUT:
		di->ept_type &= ~NU1619_TX_EPT_TIMEOUT;
		hwlog_info("[handle_tx_ept] chip reset happened\n");
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_CEP_TIMEOUT, NULL);
		break;
	case NU1619_TX_EPT_POCP:
		di->ept_type &= ~NU1619_TX_EPT_POCP;
		hwlog_info("[handle_tx_ept] ping ocp happened\n");
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_CEP_TIMEOUT, NULL);
		break;
	default:
		hwlog_err("handle_tx_ept: unrecognized type\n");
		break;
	}
}

static int nu1619_send_fsk_msg(u8 cmd, u8 *data, int data_len, void *dev_data)
{
	int ret;
	u8 header;
	u8 write_data[NU1619_TX2RX_DATA_LEN] = { 0 };
	struct nu1619_dev_info *di = g_nu1619_di;

	if (!di) {
		hwlog_err("send_fsk_msg: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	if ((data_len > NU1619_TX2RX_DATA_LEN) || (data_len < 0)) {
		hwlog_err("send_fsk_msg: data len out of range\n");
		return -WLC_ERR_PARA_WRONG;
	}

	if (cmd == QI_CMD_ACK)
		header = QI_CMD_ACK_HEAD;
	else
		header = g_nu1619_tx2rx_header[data_len + 1]; /* 1: offset */

	ret = nu1619_write_byte(NU1619_CMD_WRITE_DATA1_REG, header);
	if (ret) {
		hwlog_err("send_fsk_msg: write header fail\n");
		return -WLC_ERR_I2C_W;
	}
	ret = nu1619_write_byte(NU1619_CMD_WRITE_DATA2_REG, cmd);
	if (ret) {
		hwlog_err("send_fsk_msg: write cmd fail\n");
		return -WLC_ERR_I2C_W;
	}

	if (data && (data_len > 0)) {
		memcpy(write_data, data, data_len);
		ret = nu1619_write_block(di,
			NU1619_CMD_WRITE_DATA3_REG, write_data, data_len);
		if (ret) {
			hwlog_err("send_fsk_msg: write data fail\n");
			return -WLC_ERR_I2C_W;
		}
	}

	ret = nu1619_write_cmd_data(NU1619_REG_CMD_SEND_MSG, NULL, 0);
	if (ret) {
		hwlog_err("send_fsk_msg: send RX msg to TX fail\n");
		return -WLC_ERR_I2C_W;
	}

	hwlog_info("[send_fsk_msg] succ\n");
	return 0;
}

static int nu1619_tx_stop_config(void *dev_data)
{
	return 0;
}

static void nu1619_tx_handle_ping_evt(struct nu1619_dev_info *di)
{
	hwlog_info("tx ping interrupt\n");
	di->irq_val &= ~NU1619_TX_INT_START_DPING;
	power_event_bnc_notify(POWER_BNT_WLTX, POWER_NE_WLTX_PING_RX, NULL);
}

static void nu1619_tx_handle_ss_evt(struct nu1619_dev_info *di)
{
	hwlog_info("signal strength packet interrupt\n");
	di->irq_val &= ~NU1619_TX_INT_GET_SS;
	if (g_nu1619_handle && g_nu1619_handle->hdl_qi_ask_pkt)
		g_nu1619_handle->hdl_qi_ask_pkt(di);
}

static void nu1619_tx_handle_id_evt(struct nu1619_dev_info *di)
{
	hwlog_info("ID packet interrupt\n");
	di->irq_val &= ~NU1619_TX_INT_GET_ID;
	if (g_nu1619_handle && g_nu1619_handle->hdl_qi_ask_pkt)
		g_nu1619_handle->hdl_qi_ask_pkt(di);
}

static void nu1619_tx_handle_cfg_evt(struct nu1619_dev_info *di)
{
	hwlog_info("Config packet interrupt\n");
	di->irq_val &= ~NU1619_TX_INT_GET_CFG;
	if (g_nu1619_handle && g_nu1619_handle->hdl_qi_ask_pkt)
		g_nu1619_handle->hdl_qi_ask_pkt(di);
	power_event_bnc_notify(POWER_BNT_WLTX, POWER_NE_WLTX_GET_CFG, NULL);
}

static void nu1619_tx_handle_init_evt(struct nu1619_dev_info *di)
{
	hwlog_info("tx init interrupt\n");
	di->irq_val &= ~NU1619_TX_INT_TX_INIT;
	power_event_bnc_notify(POWER_BNT_WLTX, POWER_NE_WLTX_TX_INIT, NULL);
}

static void nu1619_tx_handle_ppp_evt(struct nu1619_dev_info *di)
{
	hwlog_info("tx rcv personal property ASK pkt\n");
	di->irq_val &= ~NU1619_TX_INT_GET_PPP;
	if (g_nu1619_handle && g_nu1619_handle->hdl_non_qi_ask_pkt)
		g_nu1619_handle->hdl_non_qi_ask_pkt(di);
}

static void nu1619_tx_handle_tx_vset_evt(struct nu1619_dev_info *di)
{
	int vset = NU1619_PS_TX_VOLT_6V8;

	hwlog_info("tx need increase vset to %dmV\n", vset);
	di->irq_val &= ~NU1619_TX_INT_SET_VTX;
	power_event_bnc_notify(POWER_BNT_WLTX, POWER_NE_WLTX_IRQ_SET_VTX, &vset);
}

static void nu1619_tx_handle_dping_evt(struct nu1619_dev_info *di)
{
	hwlog_info("rcv dping in tx mode\n");
	di->irq_val &= ~NU1619_TX_INT_GET_DPING;
	power_event_bnc_notify(POWER_BNT_WLTX, POWER_NE_WLTX_RCV_DPING, NULL);
}

static void nu1619_tx_mode_irq_handler(struct nu1619_dev_info *di)
{
	int ret;
	u16 irq_value = 0;

	ret = nu1619_read_word(NU1619_TX_INT_ADDR, &di->irq_val);
	if (ret) {
		hwlog_err("tx_mode_irq_handler: read interrupt fail, clear\n");
		nu1619_clear_tx_interrupt(NU1619_ALL_INTR_CLR);
		goto clr_irq_again;
	}

	hwlog_info("[tx_mode_irq_handler] interrupt: 0x%04x\n", di->irq_val);
	nu1619_clear_tx_interrupt(di->irq_val);

	if (di->irq_val & NU1619_TX_INT_START_DPING)
		nu1619_tx_handle_ping_evt(di);
	if (di->irq_val & NU1619_TX_INT_GET_SS)
		nu1619_tx_handle_ss_evt(di);
	if (di->irq_val & NU1619_TX_INT_GET_ID)
		nu1619_tx_handle_id_evt(di);
	if (di->irq_val & NU1619_TX_INT_GET_CFG)
		nu1619_tx_handle_cfg_evt(di);
	if (di->irq_val & NU1619_TX_INT_TX_INIT)
		nu1619_tx_handle_init_evt(di);
	if (di->irq_val & NU1619_TX_INT_EPT_TYPE)
		nu1619_handle_tx_ept(di);
	if (di->irq_val & NU1619_TX_INT_GET_PPP)
		nu1619_tx_handle_ppp_evt(di);
	if (di->irq_val & NU1619_TX_INT_SET_VTX)
		nu1619_tx_handle_tx_vset_evt(di);
	if (di->irq_val & NU1619_TX_INT_GET_DPING)
		nu1619_tx_handle_dping_evt(di);

clr_irq_again:
	if (!gpio_get_value(di->gpio_int)) {
		ret = nu1619_read_word(NU1619_TX_INT_ADDR, &irq_value);
		hwlog_info("[tx_mode_irq_handler] clear irq:0x%04x again\n",
			irq_value);
		nu1619_clear_tx_interrupt(NU1619_ALL_INTR_CLR);
	}
}

static void nu1619_irq_work(struct work_struct *work)
{
	int ret;
	u8 mode = 0;
	struct nu1619_dev_info *di = g_nu1619_di;

	if (!di) {
		hwlog_err("irq_work: di null\n");
		nu1619_wake_unlock();
		return;
	}

	/* get System Operating Mode */
	ret = nu1619_get_mode(&mode);
	if (!ret)
		hwlog_info("[irq_work] mode = 0x%x\n", mode);
	else
		nu1619_handle_abnormal_irq(di);

	/* handler interrupt */
	if (mode == NU1619_TX_MODE)
		nu1619_tx_mode_irq_handler(di);
	else if (mode == NU1619_RX_MODE)
		nu1619_rx_mode_irq_handler(di);

	if (!g_irq_abnormal_flag)
		nu1619_enable_irq(di);

	nu1619_wake_unlock();
}

static irqreturn_t nu1619_interrupt(int irq, void *_di)
{
	struct nu1619_dev_info *di = _di;

	if (!di) {
		hwlog_err("interrupt: di null\n");
		return IRQ_HANDLED;
	}

	nu1619_wake_lock();
	hwlog_info("[interrupt] ++\n");
	if (di->irq_active) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("interrupt: irq is not enable,do nothing\n");
		nu1619_wake_unlock();
	}
	hwlog_info("[interrupt] --\n");

	return IRQ_HANDLED;
}

static int nu1619_mtp_enter_dtm_mode(struct nu1619_dev_info *di)
{
	int ret;

	/* dtm mode: design test mode */
	ret = nu1619_write_byte(NU1619_DTM_REG0_ADDR, NU1619_DTM_REG0_VAL0);
	ret += nu1619_write_byte(NU1619_DTM_REG0_ADDR, NU1619_DTM_REG0_VAL1);
	ret += nu1619_write_byte(NU1619_DTM_REG0_ADDR, NU1619_DTM_REG0_VAL2);
	ret += nu1619_write_byte(NU1619_DTM_REG0_ADDR, NU1619_DTM_REG0_VAL3);
	ret += nu1619_write_byte(NU1619_DTM_REG1_ADDR, NU1619_DTM_REG1_VAL);
	ret += nu1619_write_byte(NU1619_DTM_REG2_ADDR, NU1619_DTM_REG2_VAL);
	ret += nu1619_write_byte(NU1619_DTM_REG3_ADDR, NU1619_DTM_REG3_VAL);
	ret += nu1619_write_byte(NU1619_DTM_REG4_ADDR, NU1619_DTM_REG4_VAL);
	/* use nu1619 default i2c addr */
	di->client->addr = NU1619_MTP_I2C_ADDR;
	/* config gpio 1.8v */
	ret += nu1619_write_byte(NU1619_MTP_WM_REG0, NU1619_MTP_WM_REG0_VAL);
	ret += nu1619_write_byte(NU1619_MTP_WM_REG1, NU1619_MTP_WM_REG1_VAL);
	usleep_range(9500, 10500); /* delay 10ms for config power good gpio */

	return ret;
}

static int nu1619_mtp_exit_dtm_mode(void)
{
	int ret;
	struct nu1619_dev_info *di = g_nu1619_di;

	if (!di)
		return -WLC_ERR_PARA_NULL;

	ret = nu1619_write_byte(NU1619_DTM_REG1_ADDR, NU1619_MTP_EXIT_VAL);
	di->client->addr = NU1619_NORMAL_I2C_ADDR;
	ret = nu1619_write_byte(NU1619_DTM_REG2_ADDR, NU1619_MTP_EXIT_VAL);
	ret = nu1619_write_byte(NU1619_DTM_REG3_ADDR, NU1619_MTP_EXIT_VAL);
	ret = nu1619_write_byte(NU1619_DTM_REG4_ADDR, NU1619_MTP_EXIT_VAL);
	ret = nu1619_write_byte(NU1619_DTM_REG0_ADDR, NU1619_DTM_REG0_VAL4);

	return ret;
}

static bool nu1619_mtp_is_load_byte_err(void)
{
	int i;
	struct nu1619_dev_info *di = g_nu1619_di;

	if (!di)
		return true;

	for (i = 0; i < NU1619_MTP_LOAD_BYTE_RETRY_CNT; i++) {
		if (!gpio_get_value(di->gpio_pwr_good))
			return false;
		usleep_range(15, 25); /* 20us */
	}

	return true;
}

static int nu1619_mtp_version_check(struct nu1619_dev_info *di)
{
	int ret;

	ret = nu1619_write_byte(NU1619_CHIP_CTRL_ADDR,
		NU1619_ENTER_FW_CHECK_MODE_VAL);
	if (ret) {
		hwlog_err("mtp_version_check: write mcu run reg fail\n");
		return ret;
	}
	usleep_range(19500, 20500); /* delay 20ms for crc check */
	ret = nu1619_read_block(di, NU1619_MTP_FW_VER_ADDR,
		(u8 *)&di->mtp_fw_ver, NU1619_MTP_FW_VER_LEN);
	if (ret) {
		hwlog_err("mtp_version_check: read fw_version fail\n");
		return ret;
	}
	hwlog_info("[mtp_version_check] fw_version = 0x%x\n", di->mtp_fw_ver);
	if (di->mtp_fw_ver != NU1619_MTP_FW_VERSION)
		return -WLC_ERR_MISMATCH;

	return 0;
}

static int nu1619_mtp_crc_check(struct nu1619_dev_info *di)
{
	int ret;

	ret = nu1619_read_block(di, NU1619_MTP_FW_CRC_ADDR,
		(u8 *)&di->mtp_check_sum, NU1619_MTP_FW_CRC_LEN);
	if (ret) {
		hwlog_err("mtp_crc_check: read crc fail\n");
		return ret;
	}
	hwlog_info("[mtp_crc_check] crc_val = 0x%x\n", di->mtp_check_sum);
	if (di->mtp_check_sum != NU1619_MTP_FW_CRC_VAL)
		return -WLC_ERR_MISMATCH;

	return 0;
}

static int nu1619_mtp_load_fw_data(enum nu1619_mtp_fw_type type)
{
	int ret;
	int remaining;
	const u8 *data_array = NULL;
	int size_to_wr;
	int wr_already = 0;

	if (type == NU1619_MTP_BOOT) {
		remaining = ARRAY_SIZE(g_nu1619_boot_fw);
		data_array = g_nu1619_boot_fw;
	} else if (type == NU1619_MTP_RX) {
		remaining = ARRAY_SIZE(g_nu1619_rx_fw);
		data_array = g_nu1619_rx_fw;
	} else if (type == NU1619_MTP_TX) {
		remaining = ARRAY_SIZE(g_nu1619_tx_fw);
		data_array = g_nu1619_tx_fw;
	} else {
		return -WLC_ERR_MISMATCH;
	}

	hwlog_info("[mtp_load_fw_data] remaining = %d\n", remaining);
	while (remaining > 0) {
		size_to_wr = remaining > NU1619_MTP_DOWNLOAD_LEN ?
			NU1619_MTP_DOWNLOAD_LEN : remaining;
		/* set of four bytes, big endian */
		ret = nu1619_write_byte(NU1619_MTP_DOWNLOAD_ADDR,
			data_array[wr_already + NU1619_MTP_DATA_INDEX_3]);
		if (ret || nu1619_mtp_is_load_byte_err())
			return -WLC_ERR_I2C_W;
		ret = nu1619_write_byte(NU1619_MTP_DOWNLOAD_ADDR,
			data_array[wr_already + NU1619_MTP_DATA_INDEX_2]);
		if (ret || nu1619_mtp_is_load_byte_err())
			return -WLC_ERR_I2C_W;
		ret = nu1619_write_byte(NU1619_MTP_DOWNLOAD_ADDR,
			data_array[wr_already + NU1619_MTP_DATA_INDEX_1]);
		if (ret || nu1619_mtp_is_load_byte_err())
			return -WLC_ERR_I2C_W;
		ret = nu1619_write_byte(NU1619_MTP_DOWNLOAD_ADDR,
			data_array[wr_already]);
		if (ret || nu1619_mtp_is_load_byte_err())
			return -WLC_ERR_I2C_W;

		wr_already += size_to_wr;
		remaining -= size_to_wr;
	}

	return 0;
}

static int nu1619_mtp_load_fw(enum nu1619_mtp_fw_type type)
{
	int ret;
	u16 head_addr;

	if (type == NU1619_MTP_BOOT)
		head_addr = NU1619_MTP_BOOT_HEAD_ADDR;
	else if (type == NU1619_MTP_RX)
		head_addr = NU1619_MTP_RX_HEAD_ADDR;
	else if (type == NU1619_MTP_TX)
		head_addr = NU1619_MTP_TX_HEAD_ADDR;
	else
		return -WLC_ERR_MISMATCH;
	/* write head addr */
	ret = nu1619_write_word(NU1619_MTP_WRITE_HEAD_ADDR, head_addr);
	/* enter write mode */
	ret += nu1619_write_byte(NU1619_MTP_WM_REG2, NU1619_MTP_WM_REG2_VAL);
	ret += nu1619_write_byte(NU1619_MTP_WM_REG3, NU1619_MTP_WM_REG3_VAL);
	ret += nu1619_write_byte(NU1619_MTP_WM_REG4, NU1619_MTP_WM_REG4_VAL);
	if (ret) {
		hwlog_err("mtp_load_fw: write head or w_mode reg fail\n");
		return -WLC_ERR_I2C_W;
	}
	ret = nu1619_mtp_load_fw_data(type);
	if (ret) {
		hwlog_err("mtp_load_fw: load fw data fail\n");
		return -WLC_ERR_I2C_W;
	}
	/* exit write mode */
	ret = nu1619_write_byte(NU1619_MTP_WM_REG4, NU1619_MTP_EXIT_VAL);
	ret += nu1619_write_byte(NU1619_MTP_WM_REG2, NU1619_MTP_EXIT_VAL);
	if (ret) {
		hwlog_err("mtp_load_fw: exit write mode fail\n");
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static int nu1619_program_mtp_step(void)
{
	int ret;

	hwlog_info("[program_mtp_step] load boot mtp fw\n");
	ret = nu1619_mtp_load_fw(NU1619_MTP_BOOT);
	if (ret) {
		hwlog_err("program_mtp_step: load boot fw fail\n");
		return ret;
	}
	hwlog_info("[program_mtp_step] load rx mtp fw\n");
	ret = nu1619_mtp_load_fw(NU1619_MTP_RX);
	if (ret) {
		hwlog_err("program_mtp_step: load rx fw fail\n");
		return ret;
	}
	hwlog_info("[program_mtp_step] load tx mtp fw\n");
	ret = nu1619_mtp_load_fw(NU1619_MTP_TX);
	if (ret) {
		hwlog_err("program_mtp_step: load tx fw fail\n");
		return ret;
	}

	return 0;
}

static int nu1619_program_mtp(struct nu1619_dev_info *di)
{
	int ret;

	nu1619_disable_irq_nosync(di);
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, true);
	wlps_control(di->ic_type, WLPS_RX_SW, false); /* prevent reverse current */
	usleep_range(9500, 10500); /* delay 10ms for power on */
	gpio_set_value(di->gpio_pwr_ctrl, NU1619_PWR_CTRL_EXTRA);
	usleep_range(9500, 10500); /* delay 10ms for power stability */
	di->mtp_status = NU1619_MTP_STATUS_BAD;

	ret = nu1619_mtp_enter_dtm_mode(di);
	if (ret) {
		hwlog_err("program_mtp: enter dtm mode fail\n");
		goto exit;
	}
	ret = nu1619_program_mtp_step();
	if (ret)
		goto exit;

	ret = nu1619_mtp_exit_dtm_mode();
	if (ret) {
		hwlog_err("program_mtp: exit dtm mode fail\n");
		goto exit;
	}
	ret = nu1619_mtp_version_check(di);
	if (ret) {
		hwlog_err("program_mtp: mtp version err\n");
		goto exit;
	}
	ret = nu1619_mtp_crc_check(di);
	if (ret) {
		hwlog_err("program_mtp: crc check fail\n");
		goto exit;
	}
	di->mtp_status = NU1619_MTP_STATUS_GOOD;
	hwlog_info("[program_mtp] succ\n");

exit:
	di->client->addr = NU1619_NORMAL_I2C_ADDR;
	gpio_set_value(di->gpio_pwr_ctrl, NU1619_PWR_CTRL_AC);
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
	nu1619_enable_irq(di);
	return ret;
}

static int nu1619_recover_mtp(struct nu1619_dev_info *di)
{
	return nu1619_program_mtp(di);
}

static int nu1619_rx_program_mtp(unsigned int proc_type, void *dev_data)
{
	int ret;
	struct nu1619_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (!g_mtp_chk_complete)
		return -EBUSY;

	switch (proc_type) {
	case WIRELESS_PROGRAM_FW:
		g_mtp_chk_complete = false;
		ret = nu1619_program_mtp(di);
		g_mtp_chk_complete = true;
		return ret;
	case WIRELESS_RECOVER_FW:
		g_mtp_chk_complete = false;
		ret =  nu1619_recover_mtp(di);
		g_mtp_chk_complete = true;
		return ret;
	default:
		break;
	}

	return -EINVAL;
}

static int nu1619_check_mtp(void *dev_data)
{
	int ret;
	struct nu1619_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (di->mtp_status == NU1619_MTP_STATUS_GOOD) {
		hwlog_info("[check_mtp] mtp fw status good\n");
		return 0;
	}

	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, true);
	wlps_control(di->ic_type, WLPS_RX_SW, false);
	usleep_range(9500, 10500); /* delay for power stability */
	gpio_set_value(di->gpio_pwr_ctrl, NU1619_PWR_CTRL_EXTRA);
	usleep_range(9500, 10500); /* delay for power on */

	ret = nu1619_mtp_version_check(di);
	ret += nu1619_mtp_crc_check(di);
	if (ret) {
		hwlog_err("check_mtp: fail\n");
		goto check_fail;
	}
	di->mtp_status = NU1619_MTP_STATUS_GOOD;

check_fail:
	gpio_set_value(di->gpio_pwr_ctrl, NU1619_PWR_CTRL_AC);
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
	return ret;
}

static int nu1619_get_mtp_status(unsigned int *status, void *dev_data)
{
	int ret;
	struct nu1619_dev_info *di = dev_data;

	if (!di || !status)
		return -EINVAL;

	g_mtp_chk_complete = false;
	ret = nu1619_check_mtp(di);
	if (!ret)
		*status = WIRELESS_FW_PROGRAMED;
	else
		*status = WIRELESS_FW_ERR_PROGRAMED;
	g_mtp_chk_complete = true;

	return 0;
}

static void nu1619_mtp_check_work(struct work_struct *work)
{
	int i;
	int ret;
	struct nu1619_dev_info *di = container_of(work,
		struct nu1619_dev_info, mtp_check_work);

	if (!di)
		goto exit;

	if (g_mtp_chk_complete)
		return;

	g_mtp_chk_complete = false;
	ret = nu1619_check_mtp(di);
	if (!ret) {
		hwlog_info("[mtp_check_work] succ\n");
		goto exit;
	}

	/* program for 3 times until it's ok */
	for (i = 0; i < 3; i++) {
		ret = nu1619_program_mtp(di);
		if (!ret) {
			hwlog_info("mtp_check_work: update mtp succ, cnt=%d\n",
				i + 1); /* index start from zero */
			goto exit;
		}
	}
	hwlog_err("mtp_check_work: update mtp fail\n");

exit:
	g_mtp_chk_complete = true;
}

static int nu1619_parse_ldo_cfg(struct device_node *np,
	struct nu1619_dev_info *di)
{
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_5v", di->rx_ldo_cfg_5v, NU1619_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_9v", di->rx_ldo_cfg_9v, NU1619_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_12v", di->rx_ldo_cfg_12v, NU1619_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_sc", di->rx_ldo_cfg_sc, NU1619_LDO_CFG_LEN))
		return -EINVAL;

	return 0;
}

static int nu1619_parse_fod(struct device_node *np,
	struct nu1619_dev_info *di)
{
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_5v", di->rx_fod_5v, NU1619_RX_FOD_LEN))
		return -EINVAL;
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_9v", di->rx_fod_9v, NU1619_RX_FOD_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_9v_cp39s_hk", di->rx_fod_9v_cp39s_hk,
		NU1619_RX_FOD_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_12v", di->rx_fod_12v, NU1619_RX_FOD_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_15v", di->rx_fod_15v, NU1619_RX_FOD_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_15v_cp39s_hk", di->rx_fod_15v_cp39s_hk,
		NU1619_RX_FOD_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"tx_fod_coef", di->tx_fod_coef, NU1619_TX_FOD_LEN))
		return -EINVAL;

	return 0;
}

static struct device_node *nu1619_dts_dev_node(void *dev_data)
{
	struct nu1619_dev_info *di = dev_data;

	if (!di || !di->dev)
		return NULL;

	return di->dev->of_node;
}

static int nu1619_parse_dts(struct nu1619_dev_info *di,
	struct device_node *np)
{
	int ret;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_ss_good_lth", (u32 *)&di->rx_ss_good_lth,
		NU1619_RX_SS_MAX);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_en_valid_val", (u32 *)&di->gpio_en_valid_val, 0);

	ret = nu1619_parse_fod(np, di);
	if (ret) {
		hwlog_err("parse_dts: parse fod para fail\n");
		return ret;
	}
	ret = nu1619_parse_ldo_cfg(np, di);
	if (ret) {
		hwlog_err("parse_dts: parse ldo cfg fail\n");
		return ret;
	}

	return 0;
}

static int nu1619_gpio_init(struct nu1619_dev_info *di,
	struct device_node *np)
{
	/* gpio_en */
	if (power_gpio_config_output(np,
		"gpio_en", "nu1619_en",
		&di->gpio_en, di->gpio_en_valid_val))
		return -EINVAL;

	/* gpio_sleep_en */
	if (power_gpio_config_output(np,
		"gpio_sleep_en", "nu1619_sleep_en",
		&di->gpio_sleep_en, RX_SLEEP_EN_DISABLE)) {
		gpio_free(di->gpio_en);
		return -EINVAL;
	}

	/* gpio_pwr_good */
	if (power_gpio_config_input(np,
		"gpio_pwr_good", "nu1619_pwr_good", &di->gpio_pwr_good)) {
		gpio_free(di->gpio_sleep_en);
		gpio_free(di->gpio_en);
		return -EINVAL;
	}

	return 0;
}

static int nu1619_irq_init(struct nu1619_dev_info *di,
	struct device_node *np)
{
	int ret;

	if (power_gpio_config_interrupt(np,
		"gpio_int", "nu1619_int", &di->gpio_int, &di->irq_int))
		return -EINVAL;

	ret = request_irq(di->irq_int, nu1619_interrupt,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "nu1619_irq", di);
	if (ret) {
		hwlog_err("could not request nu1619_irq\n");
		di->irq_int = -EINVAL;
		gpio_free(di->gpio_int);
		return ret;
	}
	enable_irq_wake(di->irq_int);
	di->irq_active = true;
	INIT_WORK(&di->irq_work, nu1619_irq_work);

	return 0;
}

static struct wlrx_ic_ops g_nu1619_rx_ic_ops = {
	.get_dev_node           = nu1619_dts_dev_node,
	.fw_update              = nu1619_check_fwupdate,
	.chip_init              = nu1619_chip_init,
	.chip_reset             = nu1619_chip_reset,
	.chip_enable            = nu1619_chip_enable,
	.is_chip_enable         = nu1619_is_chip_enable,
	.sleep_enable           = nu1619_sleep_enable,
	.is_sleep_enable        = nu1619_is_sleep_enable,
	.ext_pwr_post_ctrl      = nu1619_ext_pwr_post_ctrl,
	.get_vrect              = nu1619_get_rx_vrect,
	.get_vout               = nu1619_get_rx_vout,
	.get_iout               = nu1619_get_rx_iout,
	.get_vout_reg           = nu1619_get_rx_vout_reg,
	.get_vfc_reg            = nu1619_get_vfc_reg,
	.set_vfc                = nu1619_set_vfc,
	.set_vout               = nu1619_set_rx_vout,
	.get_fop                = nu1619_get_rx_fop,
	.get_cep                = nu1619_get_rx_cep,
	.get_temp               = nu1619_get_rx_temp,
	.get_chip_info          = nu1619_get_chip_info_str,
	.get_fod_coef           = nu1619_get_rx_fod,
	.set_fod_coef           = nu1619_set_rx_fod,
	.is_tx_exist            = nu1619_is_tx_exist,
	.kick_watchdog          = nu1619_kick_watchdog,
	.send_ept               = nu1619_send_ept,
	.stop_charging          = nu1619_stop_charging,
	.pmic_vbus_handler      = nu1619_pmic_vbus_handler,
};

static struct wltx_ic_ops g_nu1619_tx_ic_ops = {
	.get_dev_node           = nu1619_dts_dev_node,
	.fw_update              = nu1619_check_fwupdate,
	.activate_chip          = nu1619_activate_tx_chip,
	.chip_init              = nu1619_tx_chip_init,
	.chip_reset             = nu1619_chip_reset,
	.chip_enable            = nu1619_chip_enable,
	.mode_enable            = nu1619_enable_tx_mode,
	.set_open_flag          = nu1619_set_tx_open_flag,
	.set_stop_cfg           = nu1619_tx_stop_config,
	.is_rx_discon           = nu1619_check_rx_disconnect,
	.is_in_tx_mode          = nu1619_in_tx_mode,
	.is_in_rx_mode          = nu1619_in_rx_mode,
	.get_vrect              = nu1619_get_tx_vrect,
	.get_vin                = nu1619_get_tx_vin,
	.get_iin                = nu1619_get_tx_iin,
	.get_temp               = nu1619_get_tx_temp,
	.get_fop                = nu1619_get_tx_fop,
	.get_cep                = NULL,
	.get_duty               = NULL,
	.get_ptx                = NULL,
	.get_prx                = NULL,
	.get_ploss              = NULL,
	.get_ploss_id           = NULL,
	.get_ping_freq          = nu1619_get_tx_ping_freq,
	.get_ping_interval      = nu1619_get_tx_ping_interval,
	.get_min_fop            = nu1619_get_tx_min_fop,
	.get_max_fop            = nu1619_get_tx_max_fop,
	.set_ping_freq          = nu1619_set_tx_ping_freq,
	.set_ping_interval      = nu1619_set_tx_ping_interval,
	.set_min_fop            = nu1619_set_tx_min_fop,
	.set_max_fop            = nu1619_set_tx_max_fop,
	.set_vset               = nu1619_set_tx_vset,
	.set_ilim               = NULL,
	.set_fod_coef           = NULL,
	.set_rp_dm_to           = NULL,
};

static struct wireless_fw_ops g_nu1619_fw_ops = {
	.ic_name                = "nu1619",
	.program_fw             = nu1619_rx_program_mtp,
	.get_fw_status          = nu1619_get_mtp_status,
	.check_fw               = nu1619_check_mtp,
};

static struct qi_protocol_ops nu1619_qi_protocol_ops = {
	.chip_name = "nu1619",
	.send_msg = nu1619_send_msg,
	.send_msg_with_ack = nu1619_send_msg_ack,
	.receive_msg = nu1619_receive_msg,
	.send_fsk_msg = nu1619_send_fsk_msg,
	.get_ask_packet = nu1619_get_ask_pkt,
	.get_chip_fw_version = nu1619_get_fw_version,
	.get_tx_id_pre = NULL,
	.set_rpp_format_post = nu1619_set_rpp_format,
};

static void nu1619_shutdown(struct i2c_client *client)
{
	struct nu1619_dev_info *di = i2c_get_clientdata(client);

	if (!di)
		return;

	hwlog_info("[shutdown] ++\n");
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_ON) {
		wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
		msleep(50); /* dalay 50ms for power off */
		(void)nu1619_set_vfc(ADAPTER_5V * WL_MVOLT_PER_VOLT, di);
		(void)nu1619_set_rx_vout(ADAPTER_5V * WL_MVOLT_PER_VOLT, di);
		nu1619_chip_enable(false, di);
		msleep(NU1619_SHUTDOWN_SLEEP_TIME);
		nu1619_chip_enable(true, di);
	}
	hwlog_info("[shutdown] --\n");
}

static void nu1619_power_dev_info_register(struct nu1619_dev_info *di)
{
	struct power_devices_info_data *power_dev_info = NULL;

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = di->chip_id;
		power_dev_info->ver_id = 0;
	}
}

static int nu1619_ops_register(struct nu1619_dev_info *di)
{
	int ret;

	g_nu1619_fw_ops.dev_data = (void *)di;
	ret = wireless_fw_ops_register(&g_nu1619_fw_ops);
	if (ret) {
		hwlog_err("ops_register: register fw_ops failed\n");
		return ret;
	}

	g_nu1619_rx_ic_ops.dev_data = (void *)di;
	ret = wlrx_ic_ops_register(&g_nu1619_rx_ic_ops, di->ic_type);
	if (ret)
		return ret;
	g_nu1619_tx_ic_ops.dev_data = (void *)di;
	ret = wltx_ic_ops_register(&g_nu1619_tx_ic_ops, di->ic_type);
	if (ret)
		return ret;

	nu1619_qi_protocol_ops.dev_data = (void *)di;
	ret = qi_protocol_ops_register(&nu1619_qi_protocol_ops, di->ic_type);
	if (ret) {
		hwlog_err("ops_register: register qi_ops failed\n");
		return ret;
	}
	g_nu1619_handle = qi_protocol_get_handle();

	return 0;
}

static int nu1619_dts_para_init(struct nu1619_dev_info *di,
	struct device_node *np)
{
	int ret;

	ret = nu1619_parse_dts(di, np);
	if (ret)
		return ret;

	ret = nu1619_gpio_init(di, np);
	if (ret)
		return ret;

	ret = nu1619_irq_init(di, np);
	if (ret)
		goto irq_init_fail;

	return 0;

irq_init_fail:
	gpio_free(di->gpio_en);
	gpio_free(di->gpio_sleep_en);
	if (di->gpio_pwr_good > 0)
		gpio_free(di->gpio_pwr_good);
	return ret;
}

static void nu1619_ops_register_fail_handler(struct nu1619_dev_info *di)
{
	gpio_free(di->gpio_int);
	free_irq(di->irq_int, di);
	gpio_free(di->gpio_en);
	gpio_free(di->gpio_sleep_en);
	gpio_free(di->gpio_pwr_ctrl);
	if (di->gpio_pwr_good > 0)
		gpio_free(di->gpio_pwr_good);
}

static int nu1619_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct nu1619_dev_info *di = NULL;
	struct device_node *np = NULL;

	if (!client || !id || !client->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_nu1619_di = di;
	di->dev = &client->dev;
	np = client->dev.of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	ret = nu1619_dev_check(di, np);
	if (ret)
		goto dev_ck_fail;

	wakeup_source_init(&g_nu1619_wakelock, "nu1619_wakelock");
	mutex_init(&di->mutex_irq);

	ret = nu1619_dts_para_init(di, np);
	if (ret)
		goto dts_para_init_fail;

	if (!power_cmdline_is_powerdown_charging_mode()) {
		INIT_WORK(&di->mtp_check_work, nu1619_mtp_check_work);
		schedule_work(&di->mtp_check_work);
	} else {
		g_mtp_chk_complete = true;
	}

	ret = nu1619_ops_register(di);
	if (ret)
		goto ops_register_fail;

	if (nu1619_is_tx_exist(di)) {
		nu1619_clear_rx_interrupt(NU1619_ALL_INTR_CLR);
		hwlog_info("[probe] tx exsit, exe rx_ready_handler\n");
		nu1619_rx_ready_handler(di);
	} else {
		gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_ENABLE);
	}

	nu1619_power_dev_info_register(di);

	hwlog_info("probe ok\n");
	return 0;

ops_register_fail:
	nu1619_ops_register_fail_handler(di);
dts_para_init_fail:
	wakeup_source_trash(&g_nu1619_wakelock);
dev_ck_fail:
	devm_kfree(&client->dev, di);
	g_nu1619_di = NULL;
	return ret;
}

MODULE_DEVICE_TABLE(i2c, wireless_nu1619);
static const struct of_device_id nu1619_of_match[] = {
	{
		.compatible = "nu,nu1619",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id nu1619_i2c_id[] = {
	{ "nu1619", WLTRX_IC_MAIN }, {}
};

static struct i2c_driver nu1619_driver = {
	.probe = nu1619_probe,
	.shutdown = nu1619_shutdown,
	.id_table = nu1619_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "wireless_nu1619",
		.of_match_table = of_match_ptr(nu1619_of_match),
	},
};

static int __init nu1619_init(void)
{
	return i2c_add_driver(&nu1619_driver);
}

static void __exit nu1619_exit(void)
{
	i2c_del_driver(&nu1619_driver);
}

device_initcall(nu1619_init);
module_exit(nu1619_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("nu1619 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
