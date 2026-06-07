/*
 * stwlc68.c
 *
 * stwlc68 driver
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
#include <huawei_platform/power/wlc_qi.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/wireless_power_supply.h>
#include <huawei_platform/power/wireless_charger.h>
#include <huawei_platform/power/wireless_direct_charger.h>
#include <huawei_platform/power/wireless_transmitter.h>
#include <huawei_platform/power/wired_channel_switch.h>
#include <stwlc68.h>
#include <stwlc68_fw_sram.h>
#include <stwlc68_fw_otp.h>

#define HWLOG_TAG wireless_stwlc68
HWLOG_REGIST();

static struct wireless_cp_ops *g_wlcp_ops;
static struct stwlc68_dev_info *g_stwlc68_di;
static struct wakeup_source g_stwlc68_wakelock;
static int stop_charging_flag;
static int irq_abnormal_flag;
static bool g_tx_open_flag;
/* for sram scan */
static int g_sram_bad_addr = STWLC68_RAM_MAX_SIZE;
static bool g_sram_chk_complete;

static const u8 stwlc68_rx2tx_header[STWLC68_SEND_MSG_PKT_LEN] = {
	0, 0x18, 0x28, 0x38, 0x48, 0x58
};
static const u8 stwlc68_tx2rx_header[STWLC68_RCVD_MSG_PKT_LEN] = {
	0, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f
};

#ifndef WIRELESS_CHARGER_FACTORY_VERSION
static bool stwlc68_is_pwr_good(void)
{
	int gpio_val;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di)
		return false;
	if (di->gpio_pwr_good <= 0)
		return true;

	gpio_val = gpio_get_value(di->gpio_pwr_good);

	return gpio_val == STWLC68_GPIO_PWR_GOOD_VAL;
}
#else
static bool stwlc68_is_pwr_good(void)
{
	return true;
}
#endif /* WIRELESS_CHARGER_FACTORY_VERSION */

static bool stwlc68_is_i2c_addr_valid(u16 addr)
{
	if (!g_sram_chk_complete) {
		if ((addr < STWLC68_RAM_FW_START_ADDR + STWLC68_RAM_MAX_SIZE) &&
			(addr >= STWLC68_RAM_FW_START_ADDR))
			return true;

		return false;
	}

	return true;
}

static int stwlc68_i2c_read(struct i2c_client *client,
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
		if (!stwlc68_is_pwr_good())
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

static int stwlc68_i2c_write(struct i2c_client *client,
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
		if (!stwlc68_is_pwr_good())
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

static int stwlc68_read_block(struct stwlc68_dev_info *di,
				u16 reg, u8 *data, u8 len)
{
	u8 cmd[STWLC68_ADDR_LEN];

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	if (!stwlc68_is_i2c_addr_valid(reg))
		return -WLC_ERR_I2C_R;

	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;

	return stwlc68_i2c_read(di->client, cmd, STWLC68_ADDR_LEN, data, len);
}

static int stwlc68_write_block(struct stwlc68_dev_info *di,
				u16 reg, u8 *data, u8 data_len)
{
	u8 cmd[STWLC68_ADDR_LEN + data_len];

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	if (!stwlc68_is_i2c_addr_valid(reg))
		return -WLC_ERR_I2C_W;

	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;
	memcpy(&cmd[STWLC68_ADDR_LEN], data, data_len);

	return stwlc68_i2c_write(di->client, cmd, STWLC68_ADDR_LEN + data_len);
}

static int stwlc68_4addr_read_block(u32 addr, u8 *data, u8 len)
{
	u8 cmd[STWLC68_4ADDR_F_LEN];
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	 /* bit[0]: flag 0xFA; bit[1:4]: addr */
	cmd[0] = STWLC68_4ADDR_FLAG;
	cmd[1] = (u8)((addr >> 24) & BYTE_MASK);
	cmd[2] = (u8)((addr >> 16) & BYTE_MASK);
	cmd[3] = (u8)((addr >> 8) & BYTE_MASK);
	cmd[4] = (u8)((addr >> 0) & BYTE_MASK);

	return stwlc68_i2c_read(di->client,
		cmd, STWLC68_4ADDR_F_LEN, data, len);
}

static int stwlc68_4addr_write_block(u32 addr, u8 *data, u8 data_len)
{
	u8 cmd[STWLC68_4ADDR_F_LEN + data_len];
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	 /* bit[0]: flag 0xFA; bit[1:4]: addr */
	cmd[0] = STWLC68_4ADDR_FLAG;
	cmd[1] = (u8)((addr >> 24) & BYTE_MASK);
	cmd[2] = (u8)((addr >> 16) & BYTE_MASK);
	cmd[3] = (u8)((addr >> 8) & BYTE_MASK);
	cmd[4] = (u8)((addr >> 0) & BYTE_MASK);
	memcpy(&cmd[STWLC68_4ADDR_F_LEN], data, data_len);

	return stwlc68_i2c_write(di->client,
		cmd, STWLC68_4ADDR_F_LEN + data_len);
}

static int stwlc68_read_byte(u16 reg, u8 *data)
{
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	return stwlc68_read_block(di, reg, data, BYTE_LEN);
}

static int stwlc68_read_word(u16 reg, u16 *data)
{
	int ret;
	u8 buff[WORD_LEN] = { 0 };
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_read_block(di, reg, buff, WORD_LEN);
	if (ret)
		return -WLC_ERR_I2C_R;

	*data = buff[0] | buff[1] << BITS_PER_BYTE;
	return 0;
}

static int stwlc68_write_byte(u16 reg, u8 data)
{
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	return stwlc68_write_block(di, reg, &data, BYTE_LEN);
}

static int stwlc68_write_word(u16 reg, u16 data)
{
	u8 buff[WORD_LEN];
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	buff[0] = data & BYTE_MASK;
	buff[1] = data >> BITS_PER_BYTE;

	return stwlc68_write_block(di, reg, buff, WORD_LEN);
}

static int stwlc68_write_word_mask(u16 reg, u16 mask, u16 shift, u16 data)
{
	int ret;
	u16 val = 0;

	ret = stwlc68_read_word(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return stwlc68_write_word(reg, val);
}

int stwlc68_cp_ops_register(struct wireless_cp_ops *ops)
{
	int ret = 0;

	if (ops && !g_wlcp_ops) {
		g_wlcp_ops = ops;
	} else {
		hwlog_err("%s: charge-pump ops register fail\n", __func__);
		ret = -EPERM;
	}

	return ret;
}

static int stwlc68_clear_interrupt(u16 intr)
{
	int ret;

	ret = stwlc68_write_word(STWLC68_INTR_CLR_ADDR, intr);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static int stwlc68_cp_chip_init(void)
{
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || !di->cp_ops || !di->cp_ops->chip_init)
		return -WLC_ERR_PARA_NULL;

	return di->cp_ops->chip_init();
}

static int stwlc68_set_bp_mode(void)
{
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || !di->cp_ops || !di->cp_ops->set_bp_mode)
		return -WLC_ERR_PARA_NULL;

	return di->cp_ops->set_bp_mode();
}

static int stwlc68_set_cp_mode(void)
{
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || !di->cp_ops || !di->cp_ops->set_cp_mode)
		return -WLC_ERR_PARA_NULL;

	return di->cp_ops->set_cp_mode();
}

static int stwlc68_get_cp_vout(void)
{
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || !di->cp_ops || !di->cp_ops->get_cp_vout)
		return wldc_get_ls_vbus();

	return di->cp_ops->get_cp_vout();
}

static int stwlc68_get_cp_ratio(void)
{
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || !di->cp_ops || !di->cp_ops->get_cp_ratio)
		return WLC_CP_RATIO;

	return di->cp_ops->get_cp_ratio();
}

static bool stwlc68_is_cp_open(void)
{
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || !di->cp_ops || !di->cp_ops->is_cp_open)
		return false;

	return di->cp_ops->is_cp_open();
}

static bool stwlc68_is_bp_open(void)
{
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || !di->cp_ops || !di->cp_ops->is_bp_open)
		return true;

	return di->cp_ops->is_bp_open();
}

static void stwlc68_wake_lock(void)
{
	if (!g_stwlc68_wakelock.active) {
		__pm_stay_awake(&g_stwlc68_wakelock);
		hwlog_info("%s\n", __func__);
	}
}

static void stwlc68_wake_unlock(void)
{
	if (g_stwlc68_wakelock.active) {
		__pm_relax(&g_stwlc68_wakelock);
		hwlog_info("%s\n", __func__);
	}
}

static void stwlc68_enable_irq(struct stwlc68_dev_info *di)
{
	mutex_lock(&di->mutex_irq);
	if (!di->irq_active) {
		hwlog_info("[%s] ++\n", __func__);
		enable_irq(di->irq_int);
		di->irq_active = 1;
	}
	hwlog_info("[%s] --\n", __func__);
	mutex_unlock(&di->mutex_irq);
}

static void stwlc68_disable_irq_nosync(struct stwlc68_dev_info *di)
{
	mutex_lock(&di->mutex_irq);
	if (di->irq_active) {
		hwlog_info("[%s] ++\n", __func__);
		disable_irq_nosync(di->irq_int);
		di->irq_active = 0;
	}
	hwlog_info("[%s] --\n", __func__);
	mutex_unlock(&di->mutex_irq);
}

static void stwlc68_chip_enable(int enable)
{
	int gpio_val;
	struct stwlc68_dev_info *di = g_stwlc68_di;

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

static void stwlc68_sleep_enable(int enable)
{
	int gpio_val;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || irq_abnormal_flag)
		return;

	gpio_set_value(di->gpio_sleep_en, enable);
	gpio_val = gpio_get_value(di->gpio_sleep_en);
	hwlog_info("[%s] gpio is %s now\n",
		__func__, gpio_val ? "high" : "low");
}

static int stwlc68_chip_reset(void)
{
	int ret;
	u8 data = STWLC68_CHIP_RESET;

	ret = stwlc68_4addr_write_block(STWLC68_CHIP_RESET_ADDR, &data,
		STWLC68_FW_ADDR_LEN);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	hwlog_info("[%s] succ\n", __func__);
	return 0;
}

static int stwlc68_ext_pwr_ctrl_init(int flag)
{
	u8 wr_buff;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}
	if (flag == WLPS_CTRL_ON)
		wr_buff = STWLC68_LDO5V_EN;
	else
		wr_buff = STWLC68_LDO5V_DIS;

	hwlog_info("[%s] ldo_5v %s\n",
		__func__, (flag == WLPS_CTRL_ON) ? "on" : "off");
	return stwlc68_4addr_write_block(STWLC68_LDO5V_EN_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
}

static int stwlc68_get_chip_id(u16 *chip_id)
{
	return stwlc68_read_word(STWLC68_CHIP_ID_ADDR, chip_id);
}

static int stwlc68_get_cfg_id(u16 *cfg_id)
{
	return stwlc68_read_word(STWLC68_CFG_ID_ADDR, cfg_id);
}

static int stwlc68_get_patch_id(u16 *patch_id)
{
	return stwlc68_read_word(STWLC68_PATCH_ID_ADDR, patch_id);
}

static int stwlc68_get_cut_id(u8 *cut_id)
{
	return stwlc68_4addr_read_block(STWLC68_CUT_ID_ADDR,
		cut_id, STWLC68_FW_ADDR_LEN);
}

static int stwlc68_get_clean_addr_pointer(u32 *clean_addr_pointer)
{
	u8 cut_id = 0;

	if (!clean_addr_pointer)
		return -WLC_ERR_PARA_NULL;

	if (stwlc68_get_cut_id(&cut_id)) {
		hwlog_err("%s: get cut_id fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	if (cut_id == STWLC68_CUT_ID_V10)
		*clean_addr_pointer = STWLC68_CLEAN_ADDR_REV0;
	else if (cut_id >= STWLC68_CUT_ID_V11)
		*clean_addr_pointer = STWLC68_CLEAN_ADDR_REV1;
	else
		return -WLC_ERR_MISMATCH;

	return 0;
}

static int stwlc68_get_clean_addr(u32 *clean_addr)
{
	int ret;
	u32 clean_addr_pointer = 0;
	u8 rd_buff[STWLC68_4ADDR_LEN] = { 0 };

	ret = stwlc68_get_clean_addr_pointer(&clean_addr_pointer);
	if (ret) {
		hwlog_err("%s: get clean_addr_addr fail\n", __func__);
		return ret;
	}

	ret = stwlc68_4addr_read_block(clean_addr_pointer,
		rd_buff, STWLC68_4ADDR_LEN);
	if (ret)
		return ret;

	*clean_addr = (u32)((rd_buff[0] << 0) | (rd_buff[1] << 8) |
		(rd_buff[2] << 16) | (rd_buff[3] << 24));
	hwlog_info("[%s] clean addr: 0x%08x\n", __func__, *clean_addr);

	return 0;
}

static int stwlc68_get_chip_info(struct stwlc68_chip_info *info)
{
	int ret;
	u8 chip_info[STWLC68_CHIP_INFO_LEN] = { 0 };
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_read_block(di, STWLC68_CHIP_INFO_ADDR,
		chip_info, STWLC68_CHIP_INFO_LEN);
	if (ret)
		return ret;

	/*
	 * addr[0:1]:     chip unique id;
	 * addr[2:2]:     chip revision number;
	 * addr[3:3]:     customer id;
	 * addr[4:5]:     sram id;
	 * addr[6:7]:     svn revision number;
	 * addr[8:9]:     configuration id;
	 * addr[10:11]:  pe id;
	 * addr[12:13]:  patch id;
	 * 1byte = 8bit
	 */
	info->chip_id = (u16)(chip_info[0] | (chip_info[1] << 8));
	info->chip_rev = chip_info[2];
	info->cust_id = chip_info[3];
	info->sram_id = (u16)(chip_info[4] | (chip_info[5] << 8));
	info->svn_rev = (u16)(chip_info[6] | (chip_info[7] << 8));
	info->cfg_id = (u16)(chip_info[8] | (chip_info[9] << 8));
	info->pe_id = (u16)(chip_info[10] | (chip_info[11] << 8));
	info->patch_id = (u16)(chip_info[12] | (chip_info[13] << 8));

	ret = stwlc68_get_cut_id(&info->cut_id);
	if (ret)
		return ret;

	return 0;
}

static u8 *stwlc68_get_chip_info_str(void)
{
	int ret;
	struct stwlc68_chip_info chip_info;
	static u8 chip_info_str[STWLC68_CHIP_INFO_STR_LEN] = { 0 };

	ret = stwlc68_get_chip_info(&chip_info);
	if (ret)
		return "error read";

	memset(chip_info_str, 0, STWLC68_CHIP_INFO_STR_LEN);

	snprintf(chip_info_str, STWLC68_CHIP_INFO_STR_LEN,
		"chip_id:%d cfg_id:0x%x patch_id:0x%x cut_id:%d sram_id:0x%x",
		chip_info.chip_id, chip_info.cfg_id, chip_info.patch_id,
		chip_info.cut_id, chip_info.sram_id);

	return chip_info_str;
}

static u8 *stwlc68_get_die_id(void)
{
	int i;
	int ret;
	u8 tmp[STWLC68_DIE_ID_TMP_STR_LEN] = { 0 };
	u8 die_id_arr[STWLC68_DIE_ID_LEN] = { 0 };
	static u8 die_id_str[STWLC68_DIE_ID_STR_LEN] = { 0 };
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di)
		return "error";

	if (STWLC68_DIE_ID_STR_LEN <=
		STWLC68_DIE_ID_TMP_STR_LEN * STWLC68_DIE_ID_LEN)
		return "space error";

	if (strlen(die_id_str))
		return die_id_str;

	ret = stwlc68_read_block(di, STWLC68_DIE_ID_ADDR,
		die_id_arr, STWLC68_DIE_ID_LEN);
	if (ret)
		return "read error";

	memset(die_id_str, 0, STWLC68_DIE_ID_STR_LEN);
	for (i = 0; i < STWLC68_DIE_ID_LEN; i++) {
		snprintf(tmp, STWLC68_DIE_ID_TMP_STR_LEN, "%x", die_id_arr[i]);
		strncat(die_id_str, tmp, strlen(tmp));
	}

	return die_id_str;
}

static int stwlc68_get_mode(u8 *mode)
{
	int ret;

	if (!mode)
		return WLC_ERR_PARA_NULL;

	ret = stwlc68_read_byte(STWLC68_OP_MODE_ADDR, mode);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	return 0;
}

static int stwlc68_get_rx_temp(void)
{
	int ret;
	u16 temp = 0;

	ret = stwlc68_read_word(STWLC68_CHIP_TEMP_ADDR, &temp);
	if (ret)
		return -WLC_ERR_I2C_R;

	return (int)temp;
}

static int stwlc68_get_rx_fop(void)
{
	int ret;
	u16 fop = 0;

	ret = stwlc68_read_word(STWLC68_OP_FREQ_ADDR, &fop);
	if (ret)
		return -WLC_ERR_I2C_R;

	return (int)fop;
}

static int stwlc68_get_rx_vrect(void)
{
	int ret;
	u16 vrect = 0;

	ret = stwlc68_read_word(STWLC68_VRECT_ADDR, &vrect);
	if (ret)
		return -WLC_ERR_I2C_R;

	return (int)vrect;
}

static int stwlc68_get_rx_vout(void)
{
	int ret;
	u16 vout = 0;

	ret = stwlc68_read_word(STWLC68_VOUT_ADDR, &vout);
	if (ret)
		return -WLC_ERR_I2C_R;

	return (int)vout;
}

static int stwlc68_get_rx_iout(void)
{
	int ret;
	u16 iout = 0;

	ret = stwlc68_read_word(STWLC68_IOUT_ADDR, &iout);
	if (ret)
		return -WLC_ERR_I2C_R;

	return (int)iout;
}

static int stwlc68_get_rx_vout_reg(void)
{
	int ret;
	u16 vreg = 0;

	ret = stwlc68_read_word(STWLC68_RX_VOUT_SET_ADDR, &vreg);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	return (int)(vreg * STWLC68_RX_VOUT_SET_STEP);
}

static int stwlc68_get_tx_vout_reg(void)
{
	int ret;
	u16 tx_vreg;

	ret = stwlc68_read_word(STWLC68_FC_VOLT_ADDR, &tx_vreg);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	return (int)tx_vreg;
}

static int stwlc68_max_iout_by_temp(struct stwlc68_dev_info *di)
{
	int i;
	int rx_temp;
	static int last_i;

	rx_temp = stwlc68_get_rx_temp();
	for (i = 0; i < STWLC68_TEMP_PARA_LEVEL; i++) {
		if ((rx_temp > di->temp_para[i].temp_lth) &&
			(rx_temp <= di->temp_para[i].temp_hth)) {
			if (di->fist_monitor_flag || (last_i < i) ||
				(di->temp_para[i].temp_hth - rx_temp >
				di->temp_para[i].temp_back))
				break;
		}
	}

	if ((i >= STWLC68_TEMP_PARA_LEVEL) ||
		(di->temp_para[i].iout_max <= 0))
		return STWLC68_DFT_IOUT_MAX;

	last_i = i;

	return di->temp_para[i].iout_max;
}

static int stwlc68_max_iout_by_time(struct stwlc68_dev_info *di)
{
	int i;
	int vset;
	static unsigned int start_time;
	unsigned int delta_time;

	if ((di->fist_monitor_flag) || (start_time < STWLC68_START_TIME_TH))
		start_time = current_kernel_time().tv_sec;

	vset = stwlc68_get_tx_vout_reg();
	if (vset < RX_HIGH_VOUT2)
		return STWLC68_DFT_IOUT_MAX;

	delta_time = current_kernel_time().tv_sec - start_time;
	for (i = 0; i < STWLC68_TIME_PARA_LEVEL; i++) {
		if (delta_time >= di->time_para[i].time_th)
			break;
	}

	if ((i >= STWLC68_TIME_PARA_LEVEL) ||
		(di->time_para[i].iout_max <= 0))
		return STWLC68_DFT_IOUT_MAX;

	return di->time_para[i].iout_max;
}

static void stwlc68_monitor_rx_max_iout(struct stwlc68_dev_info *di)
{
	int iout_max = STWLC68_IOUT_MAX;
	int iout_max_by_temp;
	int iout_max_by_time;

	iout_max_by_temp = stwlc68_max_iout_by_temp(di);
	if (iout_max > iout_max_by_temp)
		iout_max = iout_max_by_temp;

	iout_max_by_time = stwlc68_max_iout_by_time(di);
	if (iout_max > iout_max_by_time)
		iout_max = iout_max_by_time;

	di->max_iout = iout_max;
}

static int stwlc68_get_rx_max_iout(void)
{
	int ret;
	u8 cut_id = 0;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || (di->max_iout <= 0))
		return STWLC68_DFT_IOUT_MAX;

	/* cut_id < 4, iout_max = 1300mA */
	ret = stwlc68_get_cut_id(&cut_id);
	if (ret || cut_id < 4)
		return STWLC68_DFT_IOUT_MAX;

	return di->max_iout;
}

static int stwlc68_set_rx_vout(int vol)
{
	int ret;

	if (vol < STWLC68_RX_VOUT_MIN || vol > STWLC68_RX_VOUT_MAX) {
		hwlog_err("%s: out of range\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	vol = vol / STWLC68_RX_VOUT_SET_STEP;
	ret = stwlc68_write_word(STWLC68_RX_VOUT_SET_ADDR, (u16)vol);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	return 0;
}

static bool stwlc68_is_cp_really_open(void)
{
	int rx_ratio;
	int rx_vset;
	int rx_vout;
	int cp_vout;

	if (!stwlc68_is_cp_open())
		return false;

	rx_ratio = stwlc68_get_cp_ratio();
	rx_vset = stwlc68_get_rx_vout_reg();
	rx_vout =  stwlc68_get_rx_vout();
	cp_vout = stwlc68_get_cp_vout();

	hwlog_info("[%s] rx_ratio:%d rx_vset:%d rx_vout:%d cp_vout:%d\n",
		__func__, rx_ratio, rx_vset, rx_vout, cp_vout);
	if ((cp_vout * rx_ratio) < (rx_vout - STWLC68_FC_VOUT_ERR_LTH))
		return false;
	if ((cp_vout * rx_ratio) > (rx_vout + STWLC68_FC_VOUT_ERR_UTH))
		return false;

	return true;
}

static int stwlc68_check_cp_mode(void)
{
	int i;
	int cnt;
	int ret;

	ret = stwlc68_set_cp_mode();
	if (ret) {
		hwlog_err("%s: set cp mode fail\n", __func__);
		return ret;
	}
	cnt = STWLC68_BPCP_TIMEOUT / STWLC68_BPCP_SLEEP_TIME;
	for (i = 0; i < cnt; i++) {
		msleep(STWLC68_BPCP_SLEEP_TIME);
		if (stwlc68_is_cp_really_open()) {
			hwlog_info("[%s] set cp mode succ\n", __func__);
			break;
		}
		if (stop_charging_flag)
			return -WLC_ERR_STOP_CHRG;
	}
	if (i == cnt) {
		hwlog_err("%s: set cp mode fail\n", __func__);
		return -WLC_ERR_MISMATCH;
	}

	return 0;
}

static int stwlc68_send_fc_cmd(int vset)
{
	int ret;

	ret = stwlc68_write_word(STWLC68_FC_VOLT_ADDR, (u16)vset);
	if (ret) {
		hwlog_err("%s: set fc reg fail\n", __func__);
		return ret;
	}
	ret = stwlc68_write_word_mask(STWLC68_CMD_ADDR,
		STWLC68_CMD_SEND_FC, STWLC68_CMD_SEND_FC_SHIFT,
		STWLC68_CMD_VAL);
	if (ret) {
		hwlog_err("%s: send fc cmd fail\n", __func__);
		return ret;
	}
	ret = stwlc68_set_rx_vout(vset);
	if (ret) {
		hwlog_err("%s: set rx vout fail\n", __func__);
		return ret;
	}

	return 0;
}

static bool stwlc68_is_fc_succ(int vset)
{
	int i;
	int cnt;
	int vout;

	cnt = STWLC68_FC_VOUT_TIMEOUT / STWLC68_FC_VOUT_SLEEP_TIME;
	for (i = 0; i < cnt; i++) {
		if (stop_charging_flag && (vset > STWLC68_FC_VOUT_DEFAULT))
			return false;
		msleep(STWLC68_FC_VOUT_SLEEP_TIME);
		vout = stwlc68_get_rx_vout();
		if ((vout >= vset - STWLC68_FC_VOUT_ERR_LTH) &&
			(vout <= vset + STWLC68_FC_VOUT_ERR_UTH)) {
			hwlog_info("[%s] succ, cost_time: %dms\n", __func__,
				(i + 1) * STWLC68_FC_VOUT_SLEEP_TIME);
			return true;
		}
	}

	return false;
}

static void stwlc68_ask_mode_cfg(u8 mode_cfg)
{
	int ret;

	ret = stwlc68_write_byte(STWLC68_ASK_CFG_ADDR, mode_cfg);
	if (ret)
		hwlog_err("%s: fail\n", __func__);
}

static int stwlc68_set_tx_vout(int vset)
{
	int ret;
	int i;

	if (vset >= RX_HIGH_VOUT2) {
		ret = stwlc68_check_cp_mode();
		if (ret)
			return ret;
	}
	if (vset <= RX_HIGH_VOUT)
		stwlc68_ask_mode_cfg(STWLC68_BOTH_CAP_POSITIVE);

	for (i = 0; i < STWLC68_FC_VOUT_RETRY_CNT; i++) {
		if (stop_charging_flag && (vset > STWLC68_FC_VOUT_DEFAULT))
			return -WLC_ERR_STOP_CHRG;
		ret = stwlc68_send_fc_cmd(vset);
		if (ret) {
			hwlog_err("%s: send fc_cmd fail\n", __func__);
			continue;
		}
		hwlog_info("[%s] send fc cmd, cnt: %d\n", __func__, i);
		if (stwlc68_is_fc_succ(vset)) {
			if (vset < RX_HIGH_VOUT2)
				(void)stwlc68_set_bp_mode();
			if (vset > RX_HIGH_VOUT)
				stwlc68_ask_mode_cfg(STWLC68_MOD_C_NEGATIVE);
			hwlog_info("[%s] succ\n", __func__);
			return 0;
		}
	}

	return -WLC_ERR_MISMATCH;
}


static int stwlc68_send_msg(u8 cmd, u8 *data, int data_len)
{
	int ret;
	u8 header;
	u8 write_data[STWLC68_SEND_MSG_DATA_LEN] = { 0 };
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_I2C_WR;
	}

	if ((data_len > STWLC68_SEND_MSG_DATA_LEN) || (data_len < 0)) {
		hwlog_err("%s: send data number out of range\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	di->irq_val &= ~STWLC68_RX_STATUS_TX2RX_ACK;
	/* msg_len=cmd_len+data_len  cmd_len=1 */
	header = stwlc68_rx2tx_header[data_len + 1];

	ret = stwlc68_write_byte(STWLC68_SEND_MSG_HEADER_ADDR, header);
	if (ret) {
		hwlog_err("%s: write header fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}
	ret = stwlc68_write_byte(STWLC68_SEND_MSG_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("%s: write cmd fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	if (data && (data_len > 0)) {
		memcpy(write_data, data, data_len);
		ret = stwlc68_write_block(di,
			STWLC68_SEND_MSG_DATA_ADDR, write_data, data_len);
		if (ret) {
			hwlog_err("%s: write RX2TX-reg fail\n", __func__);
			return -WLC_ERR_I2C_W;
		}
	}

	ret = stwlc68_write_word_mask(STWLC68_CMD_ADDR,
		STWLC68_CMD_SEND_MSG_WAIT_RPLY,
		STWLC68_CMD_SEND_MSG_WAIT_RPLY_SHIFT, STWLC68_CMD_VAL);
	if (ret) {
		hwlog_err("%s: send RX msg to TX fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	hwlog_info("%s: send msg(cmd:0x%x) success\n", __func__, cmd);
	return 0;
}

static int stwlc68_send_msg_ack(u8 cmd, u8 *data, int data_len)
{
	int ret;
	int count = 0;
	int ack_cnt;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	do {
		stwlc68_send_msg(cmd, data, data_len);
		for (ack_cnt = 0;
			ack_cnt < STWLC68_WAIT_FOR_ACK_RETRY_CNT; ack_cnt++) {
			msleep(STWLC68_WAIT_FOR_ACK_SLEEP_TIME);
			if (STWLC68_RX_STATUS_TX2RX_ACK & di->irq_val) {
				di->irq_val &= ~STWLC68_RX_STATUS_TX2RX_ACK;
				hwlog_info("[%s] succ, retry times = %d\n",
					   __func__, count);
				return 0;
			}
			if (stop_charging_flag)
				return -WLC_ERR_STOP_CHRG;
		}
		count++;
		hwlog_info("[%s] retry\n", __func__);
	} while (count < STWLC68_SNED_MSG_RETRY_CNT);

	if (count < STWLC68_SNED_MSG_RETRY_CNT) {
		hwlog_info("[%s] succ\n", __func__);
		return 0;
	}

	ret = stwlc68_read_byte(STWLC68_RCVD_MSG_CMD_ADDR, &cmd);
	if (ret) {
		hwlog_err("%s: get rcv cmd data fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}
	if (cmd != STWLC68_CMD_ACK) {
		hwlog_err("[%s] fail, ack = 0x%x, retry times = %d\n",
			__func__, cmd, count);
		return -WLC_ERR_ACK_TIMEOUT;
	}

	return 0;
}

static int stwlc68_receive_msg(u8 *data, int data_len)
{
	int ret;
	int count = 0;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	do {
		if (di->irq_val & STWLC68_RX_RCVD_MSG_INTR_LATCH) {
			di->irq_val &= ~STWLC68_RX_RCVD_MSG_INTR_LATCH;
			goto func_end;
		}
		if (stop_charging_flag)
			return -WLC_ERR_STOP_CHRG;
		msleep(STWLC68_RCV_MSG_SLEEP_TIME);
		count++;
	} while (count < STWLC68_RCV_MSG_SLEEP_CNT);

func_end:
	ret = stwlc68_read_block(di, STWLC68_RCVD_MSG_CMD_ADDR, data, data_len);
	if (ret) {
		hwlog_err("%s:get tx to rx data fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}
	if (!data[0]) { /* data[0]: cmd */
		hwlog_err("%s: no msg received from tx\n", __func__);
		return -WLC_ERR_ACK_TIMEOUT;
	}
	hwlog_info("[%s] get tx2rx data(cmd:0x%x) succ\n", __func__, data[0]);
	return 0;
}

static int stwlc68_send_ept(enum wireless_etp_type ept_type)
{
	int ret;
	u8 rx_ept_type;

	switch (ept_type) {
	case WIRELESS_EPT_ERR_VRECT:
		rx_ept_type = STWLC68_EPT_ERR_VRECT;
		break;
	case WIRELESS_EPT_ERR_VOUT:
		rx_ept_type = STWLC68_EPT_ERR_VOUT;
		break;
	default:
		return -WLC_ERR_PARA_WRONG;
	}
	ret = stwlc68_write_byte(STWLC68_EPT_MSG_ADDR, rx_ept_type);
	ret += stwlc68_write_word_mask(STWLC68_CMD_ADDR, STWLC68_CMD_SEND_EPT,
		STWLC68_CMD_SEND_EPT_SHIFT, STWLC68_CMD_VAL);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_get_tx_para(u8 cmd, u8 *receive_data,
				int receive_len)
{
	int ret;
	int count = 0;

	do {
		ret = stwlc68_send_msg(cmd, NULL, 0);
		ret |= stwlc68_receive_msg(receive_data, receive_len);
		if (!ret) {
			hwlog_info("[%s] get tx para, succ\n", __func__);
			return 0;
		}
		count++;
		hwlog_info("%s: get tx para fail, try next time\n", __func__);
	} while (count < STWLC68_GET_TX_PARA_RETRY_CNT);

	hwlog_info("%s: get tx para, fail\n", __func__);
	return -WLC_ERR_ACK_TIMEOUT;
}

static int stwlc68_check_dc_power(void)
{
	int ret;
	u8 op_mode = 0;

	ret = stwlc68_read_byte(STWLC68_OP_MODE_ADDR, &op_mode);
	if (ret)
		return -WLC_ERR_I2C_R;

	hwlog_info("%s: Op mode %02X\n", __func__, op_mode);
	if (op_mode != STWLC68_FW_OP_MODE_SA)
		return -WLC_ERR_MISMATCH;

	return 0;
}

static int stwlc68_reset_system(void)
{
	u8 wr_buff = STWLC68_RST_SYS | STWLC68_RST_M0;

	return stwlc68_4addr_write_block(STWLC68_RST_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
}

static int stwlc68_disable_fw_i2c(void)
{
	u8 wr_buff = STWLC68_FW_I2C_DISABLE;

	return stwlc68_4addr_write_block(STWLC68_FW_I2C_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
}

static int stwlc68_hold_m0(void)
{
	u8 wr_buff = STWLC68_RST_M0;

	return stwlc68_4addr_write_block(STWLC68_RST_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
}

static int stwlc68_reset_otp_block(void)
{
	u8 rd_buff = 0;
	u8 wr_buff;
	u8 reset_reg;
	int ret;

	ret = stwlc68_4addr_read_block(STWLC68_RST_ADDR,
		&rd_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_R;

	reset_reg = rd_buff;
	reset_reg = reset_reg | (1 << 4);
	wr_buff = reset_reg;

	ret = stwlc68_4addr_write_block(STWLC68_RST_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_W;

	reset_reg = reset_reg & ~(1 << 4);
	wr_buff = reset_reg;

	return stwlc68_4addr_write_block(STWLC68_RST_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
}

static int stwlc68_enable_otp(void)
{
	u8 wr_buff = STWLC68_OTP_ENABLE;

	return stwlc68_4addr_write_block(STWLC68_OTP_ENABLE_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
}

static int stwlc68_unlock_otp(void)
{
	u8 wr_buff = STWLC68_OTP_UNLOCK_CODE;

	return stwlc68_4addr_write_block(STWLC68_OTP_PROGRAM_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
}

static int stwlc68_lock_otp(void)
{
	u8 wr_buff = STWLC68_OTP_LOCK_CODE;

	return stwlc68_4addr_write_block(STWLC68_OTP_PROGRAM_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
}

static int stwlc68_config_otp_registers(void)
{
	int ret;
	u32 addr;
	u8 write_buff;

	addr = STWLC68_RD_ROM_OTP_PULSE_ADDR;
	write_buff = STWLC68_RD_ROM_OTP_PULSE_VAL;
	ret = stwlc68_4addr_write_block(addr, &write_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_W;

	addr = STWLC68_CLEAN_OTP_PULSE_ADDR;
	write_buff = STWLC68_CLEAN_OTP_PULSE_VAL;
	ret = stwlc68_4addr_write_block(addr, &write_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_W;

	addr = STWLC68_RD_VERIFY1_OTP_ADDR;
	write_buff = STWLC68_RD_VERIFY1_OTP_VAL;
	ret = stwlc68_4addr_write_block(addr, &write_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_W;

	addr = STWLC68_RD_VERIFY2_OTP_ADDR;
	write_buff = STWLC68_RD_VERIFY2_OTP_VAL;
	ret = stwlc68_4addr_write_block(addr, &write_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_W;

	addr = STWLC68_RD_USRCMP_OTP_ADDR;
	write_buff = STWLC68_RD_USRCMP_OTP_VAL;
	ret = stwlc68_4addr_write_block(addr, &write_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_W;

	addr = STWLC68_RD_MISSION_OTP_ADDR;
	write_buff = STWLC68_RD_MISSION_OTP_VAL;
	ret = stwlc68_4addr_write_block(addr, &write_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_W;

	addr = STWLC68_RD_PGM_LO_OTP_ADDR;
	write_buff = STWLC68_RD_PGM_LO_OTP_VAL;
	ret = stwlc68_4addr_write_block(addr, &write_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_W;

	addr = STWLC68_RD_PGM_HI_OTP_ADDR;
	write_buff = STWLC68_RD_PGM_HI_OTP_VAL;
	ret = stwlc68_4addr_write_block(addr, &write_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_W;

	addr = STWLC68_RD_SOAK_OTP_LO_ADDR;
	write_buff = STWLC68_RD_SOAK_OTP_LO_VAL;
	ret = stwlc68_4addr_write_block(addr, &write_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_W;

	addr = STWLC68_RD_SOAK_OTP_HI_ADDR;
	write_buff = STWLC68_RD_SOAK_OTP_HI_VAL;
	ret = stwlc68_4addr_write_block(addr, &write_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_W;

	addr = STWLC68_OTP_FREQ_CTRL_ADDR;
	write_buff = STWLC68_OTP_FREQ_CTRL_VAL;
	ret = stwlc68_4addr_write_block(addr, &write_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_W;

	return 0;
}

static int stwlc68_check_fw_wr(u32 wr_already,
	u8 *wr_buff, u8 *rd_buff, int size_to_wr)
{
	int i;
	int ret;
	u32 bad_addr;

	ret = memcmp(wr_buff, rd_buff, size_to_wr);
	if (!ret)
		return 0;

	for (i = 0; i < size_to_wr; i++) {
		if (wr_buff[i] != rd_buff[i]) {
			bad_addr = wr_already + i;
			hwlog_err("%s: wr&&rd data mismatch, bad_addr=0x%x\n",
				__func__, bad_addr);
			if (bad_addr < STWLC68_RAM_FW_CODE_OFFSET)
				continue;
			if (g_sram_bad_addr >= bad_addr)
				g_sram_bad_addr = bad_addr;
			return -WLC_ERR_CHECK_FAIL;
		}
	}

	return 0;
}

static int stwlc68_write_fw_data(u32 current_clean_addr,
	const u8 *otp_data, int otp_size)
{
	int ret;
	int remaining = otp_size;
	int size_to_wr;
	u32 wr_already = 0;
	u32 addr = current_clean_addr;
	u8 wr_buff[STWLC68_OTP_PROGRAM_WR_SIZE] = { 0 };
	u8 rd_buff[STWLC68_OTP_PROGRAM_WR_SIZE] = { 0 };

	while (remaining > 0) {
		size_to_wr = remaining > STWLC68_OTP_PROGRAM_WR_SIZE ?
			STWLC68_OTP_PROGRAM_WR_SIZE : remaining;
		memcpy(wr_buff, otp_data + wr_already, size_to_wr);
		ret = stwlc68_4addr_write_block(addr, wr_buff, size_to_wr);
		if (ret) {
			hwlog_err("%s: wr fail, addr=0x%08x\n", __func__, addr);
			return ret;
		}
		usleep_range(500, 9500); /* 1ms */
		ret = stwlc68_4addr_read_block(addr, rd_buff, size_to_wr);
		if (ret) {
			hwlog_err("%s: rd fail, addr=0x%08x\n", __func__, addr);
			return ret;
		}
		ret = stwlc68_check_fw_wr(wr_already, wr_buff, rd_buff, size_to_wr);
		if (ret)
			break;
		addr += size_to_wr;
		wr_already += size_to_wr;
		remaining -= size_to_wr;
	}

	return 0;
}

static int stwlc68_load_ram_fw(const u8 *ram, int len)
{
	int ret;

	ret = stwlc68_disable_fw_i2c();
	if (ret) {
		hwlog_err("%s: disable fw_i2c fail\n", __func__);
		return ret;
	}
	ret = stwlc68_hold_m0();
	if (ret) {
		hwlog_err("%s: hold M0 fail\n", __func__);
		return ret;
	}

	ret = stwlc68_write_fw_data(STWLC68_RAM_FW_START_ADDR, ram, len);
	if (ret) {
		hwlog_err("%s: write fw data fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_write_otp_data(u32 current_clean_addr,
	const u8 *otp_data, int otp_size)
{
	int ret;
	int remaining = otp_size;
	int size_to_write;
	u32 written_already = 0;
	int address = current_clean_addr;
	u8 buff[STWLC68_OTP_I2C_CHUNK_SIZE] = { 0 };

	if (stwlc68_unlock_otp()) {
		hwlog_err("%s: unlock OTP fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	while (remaining > 0) {
		size_to_write = remaining > STWLC68_OTP_I2C_CHUNK_SIZE ?
			STWLC68_OTP_I2C_CHUNK_SIZE : remaining;
		memcpy(buff, otp_data + written_already, size_to_write);
		ret = stwlc68_4addr_write_block(address, buff, size_to_write);
		if (ret) {
			hwlog_err("%s: fail, addr=0x%8x\n", __func__, address);
			if (stwlc68_lock_otp())
				hwlog_err("%s: lock OTP fail\n", __func__);
			return ret;
		}
		address += size_to_write;
		written_already += size_to_write;
		remaining -= size_to_write;
		usleep_range(9500, 10500); /* 10ms */
	}

	ret = stwlc68_lock_otp();
	if (ret) {
		hwlog_err("%s: lock OTP fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_fw_ram_check(void)
{
	int ret;
	u8 *ram = NULL;
	int size = sizeof(u8) * STWLC68_RAM_MAX_SIZE;

	ram = kzalloc(size, GFP_KERNEL);
	if (!ram)
		return -WLC_ERR_NO_SPACE;

	(void)stwlc68_reset_system();
	msleep(STWLC68_OTP_OPS_DELAY);

	memset(ram, STWLC68_RAM_CHECK_VAL1, size);
	ret = stwlc68_load_ram_fw(ram, size);
	if (ret) {
		kfree(ram);
		return ret;
	}
	memset(ram, STWLC68_RAM_CHECK_VAL2, size);
	ret = stwlc68_load_ram_fw(ram, size);
	if (ret) {
		kfree(ram);
		return ret;
	}

	kfree(ram);
	return 0;
}

static int stwlc68_program_otp_pre_check(void)
{
	int ret;

	ret = stwlc68_check_dc_power();
	if (ret) {
		hwlog_err("%s: OTP must be programmed in DC power", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_program_otp_choose_otp(int *otp_id)
{
	int i;
	int ret;
	u8 cut_id = 0;
	int otp_num;

	ret = stwlc68_get_cut_id(&cut_id);
	if (ret) {
		hwlog_err("%s: get cut_id fail\n", __func__);
		return ret;
	}
	hwlog_info("[%s] cut_id = 0x%x\n", __func__, cut_id);
	/* determine what has to be programmed depending on version ids */
	otp_num = ARRAY_SIZE(st_otp_info);
	for (i = 0; i < otp_num; i++) {
		if ((cut_id >= st_otp_info[i].cut_id_from) &&
			(cut_id <= st_otp_info[i].cut_id_to)) {
			*otp_id = i;
			return 0;
		}
	}

	hwlog_err("%s: cut_id mismatch\n", __func__);
	return -WLC_ERR_MISMATCH;
}

static int stwlc68_program_otp_calc_otp_addr(int otp_id,
	const u8 **data_to_program, u32 *data_to_program_size)
{
	int ret;
	u16 cfg_id = 0;
	u16 patch_id = 0;
	int cfg_id_mismatch = 0;
	int patch_id_mismatch = 0;

	ret = stwlc68_get_cfg_id(&cfg_id);
	if (ret) {
		hwlog_err("%s: get cfg_id fail\n", __func__);
		return ret;
	}
	if (cfg_id != st_otp_info[otp_id].cfg_id) {
		hwlog_err("[%s] cfg_id mismatch, running|latest:0x%x|0x%x\n",
			__func__, cfg_id, st_otp_info[otp_id].cfg_id);
		cfg_id_mismatch = 1;
	}
	ret = stwlc68_get_patch_id(&patch_id);
	if (ret) {
		hwlog_err("%s: get patch_id fail\n", __func__);
		return ret;
	}
	if (patch_id != st_otp_info[otp_id].patch_id) {
		hwlog_err("[%s] patch_id mismatch, running|latest:0x%x|0x%x\n",
			__func__, patch_id, st_otp_info[otp_id].patch_id);
		patch_id_mismatch = 1;
	}
	if (cfg_id_mismatch && patch_id_mismatch) {
		*data_to_program_size = st_otp_info[otp_id].cfg_size +
			st_otp_info[otp_id].patch_size;
		*data_to_program = st_otp_info[otp_id].otp_arr;
	} else if (cfg_id_mismatch && !patch_id_mismatch) {
		*data_to_program_size = st_otp_info[otp_id].cfg_size;
		*data_to_program = st_otp_info[otp_id].otp_arr;
	} else if (!cfg_id_mismatch && patch_id_mismatch) {
		*data_to_program_size = st_otp_info[otp_id].patch_size;
		*data_to_program = st_otp_info[otp_id].otp_arr +
			st_otp_info[otp_id].cfg_size;
	} else {
		hwlog_info("[%s] cfg && patch are latest\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	return 0;
}

static int stwlc68_program_otp_check_clean_addr(
	u32 *current_clean_addr, u32 data_to_program_size)
{
	int ret;

	ret = stwlc68_get_clean_addr(current_clean_addr);
	if (ret) {
		hwlog_err("%s: get current_clean_addr fail\n", __func__);
		return ret;
	}
	hwlog_info("[%s] opt_max_size: 0x%04X, opt_size: 0x%04X\n",
		__func__, STWLC68_OTP_MAX_SIZE, data_to_program_size);
	if ((STWLC68_OTP_MAX_SIZE - (*current_clean_addr & WORD_MASK)) <
		data_to_program_size) {
		hwlog_err("%s: not enough space available\n", __func__);
		return -WLC_ERR_NO_SPACE;
	}

	return 0;
}

static int stwlc68_program_otp_prepare(void)
{
	int ret;

	ret = stwlc68_disable_fw_i2c();
	if (ret) {
		hwlog_err("%s: disable fw_i2c fail\n", __func__);
		return ret;
	}
	ret = stwlc68_hold_m0();
	if (ret) {
		hwlog_err("%s: hold M0 fail\n", __func__);
		return ret;
	}
	msleep(STWLC68_OTP_OPS_DELAY);
	ret = stwlc68_reset_otp_block();
	if (ret) {
		hwlog_err("%s: OTP block reset fail\n", __func__);
		return ret;
	}
	ret = stwlc68_enable_otp();
	if (ret) {
		hwlog_err("%s: enable OTP fail\n", __func__);
		return ret;
	}
	ret = stwlc68_unlock_otp();
	if (ret) {
		hwlog_err("%s: unlock OTP fail\n", __func__);
		return ret;
	}
	ret = stwlc68_config_otp_registers();
	if (ret) {
		hwlog_err("%s: config otp registers fail\n", __func__);
		if (stwlc68_lock_otp())
			hwlog_err("%s: lock otp fail\n", __func__);
		return ret;
	}
	ret = stwlc68_lock_otp();
	if (ret) {
		hwlog_err("%s: lock OTP fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_program_otp_post_check(int otp_id,
	u32 pre_clean_addr, u32 data_to_program_size)
{
	int ret;
	u16 cfg_id = 0;
	u16 patch_id = 0;
	u32 clean_addr = 0;

	ret = stwlc68_get_clean_addr(&clean_addr);
	if (ret) {
		hwlog_err("%s: get clean_addr fail\n", __func__);
		return ret;
	}
	if (clean_addr != (pre_clean_addr + data_to_program_size)) {
		hwlog_err("%s: clean addr mismatch after flashing\n", __func__);
		return -WLC_ERR_MISMATCH;
	}

	msleep(STWLC68_OTP_OPS_DELAY);

	ret = stwlc68_get_cfg_id(&cfg_id);
	if (ret) {
		hwlog_err("%s: get cfg_id fail\n", __func__);
		return ret;
	}
	if (cfg_id != st_otp_info[otp_id].cfg_id)
		hwlog_err("%s: cfg_id(0x%x) mismatch after flashing\n",
			__func__, cfg_id);

	ret = stwlc68_get_patch_id(&patch_id);
	if (ret) {
		hwlog_err("%s: get patch_id fail\n", __func__);
		return ret;
	}
	if (patch_id != st_otp_info[otp_id].patch_id)
		hwlog_err("%s: patch_id(0x%x) mismatch after flashing\n",
			__func__, patch_id);

	ret = stwlc68_lock_otp();
	if (ret)
		hwlog_err("%s: lock otp fail\n", __func__);

	return 0;
}

static int stwlc68_program_otp(void)
{
	int ret;
	int otp_id = 0;
	u32 current_clean_addr = 0;
	u32 data_to_program_size = 0;
	const u8 *data_to_program = NULL;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}
	stwlc68_disable_irq_nosync(di);
	wlps_control(WLPS_PROC_OTP_PWR, WLPS_CTRL_ON);
	msleep(STWLC68_OTP_OPS_DELAY); /* delay for power on */

	ret = stwlc68_program_otp_pre_check();
	if (ret)
		goto exit;
	ret = stwlc68_reset_system();
	if (ret)
		hwlog_err("%s: reset system fail, ignore ACK\n", __func__);
	msleep(STWLC68_OTP_OPS_DELAY); /* delay for system reset */
	ret = stwlc68_program_otp_choose_otp(&otp_id);
	if (ret)
		goto exit;
	hwlog_info("[%s] otp_index = %d\n", __func__, otp_id);
	ret = stwlc68_program_otp_calc_otp_addr(otp_id,
		&data_to_program, &data_to_program_size);
	if (ret)
		goto exit;
	ret = stwlc68_program_otp_check_clean_addr(&current_clean_addr,
		data_to_program_size);
	if (ret)
		goto exit;
	ret = stwlc68_program_otp_prepare();
	if (ret)
		goto exit;
	ret = stwlc68_write_otp_data(current_clean_addr,
		data_to_program, data_to_program_size);
	if (ret)
		goto exit;
	ret = stwlc68_reset_system();
	if (ret)
		hwlog_err("%s: reset system fail, ignore ACK\n", __func__);
	msleep(STWLC68_OTP_OPS_DELAY); /* delay for system reset */
	ret = stwlc68_program_otp_post_check(otp_id, current_clean_addr,
		data_to_program_size);
	if (ret)
		goto exit;

	wlps_control(WLPS_PROC_OTP_PWR, WLPS_CTRL_OFF);
	stwlc68_enable_irq(di);
	hwlog_info("[%s] succ\n", __func__);
	return 0;

exit:
	wlps_control(WLPS_PROC_OTP_PWR, WLPS_CTRL_OFF);
	stwlc68_enable_irq(di);
	hwlog_info("[%s] fail\n", __func__);
	return ret;

}

static int stwlc68_check_otp(void)
{
	int ret;
	int i;
	int otp_num;
	u8 cut_id = 0;
	u16 cfg_id = 0;
	u16 patch_id = 0;

	wlps_control(WLPS_PROC_OTP_PWR, WLPS_CTRL_ON);
	stwlc68_chip_enable(RX_EN_ENABLE); /* enable RX for i2c WR */
	msleep(STWLC68_OTP_OPS_DELAY); /* delay for power on */
	ret = stwlc68_get_cut_id(&cut_id);
	if (ret) {
		hwlog_err("%s: get cut_id fail\n", __func__);
		goto exit;
	}
	hwlog_info("[%s] cut_id = 0x%x\n", __func__, cut_id);
	otp_num = ARRAY_SIZE(st_otp_info);
	for (i = 0; i < otp_num; i++) {
		if ((cut_id >= st_otp_info[i].cut_id_from) &&
			(cut_id <= st_otp_info[i].cut_id_to))
			break;
	}
	if (i == otp_num) {
		hwlog_err("%s: cut_id mismatch\n", __func__);
		ret = WLC_ERR_MISMATCH;
		goto exit;
	}
	ret = stwlc68_get_patch_id(&patch_id);
	if (ret) {
		hwlog_err("%s: get patch_id fail\n", __func__);
		goto exit;
	}
	ret = stwlc68_get_cfg_id(&cfg_id);
	if (ret) {
		hwlog_err("%s: get cfg_id fail\n", __func__);
		goto exit;
	}

	if ((patch_id != st_otp_info[i].patch_id) ||
		(cfg_id != st_otp_info[i].cfg_id))
		ret = WLC_ERR_MISMATCH;
exit:
	wlps_control(WLPS_PROC_OTP_PWR, WLPS_CTRL_OFF);
	return ret;
}

static int stwlc68_check_is_otp_exist(void)
{
	if (!stwlc68_check_otp())
		return WLC_OTP_PROGRAMED;

	return WLC_OTP_NON_PROGRAMED;
}

static int stwlc68_sw2tx(void)
{
	int ret;
	int i;
	u8 mode = 0;
	int cnt = STWLC68_SW2TX_RETRY_TIME / STWLC68_SW2TX_RETRY_SLEEP_TIME;

	for (i = 0; i < cnt; i++) {
		if (!g_tx_open_flag) {
			hwlog_err("%s: tx_open_flag false\n", __func__);
			return -WLC_ERR_STOP_CHRG;
		}
		msleep(STWLC68_SW2TX_RETRY_SLEEP_TIME);
		ret = stwlc68_get_mode(&mode);
		if (ret) {
			hwlog_err("%s: get mode fail\n", __func__);
			continue;
		}

		ret = stwlc68_write_word_mask(STWLC68_CMD_ADDR,
			STWLC68_CMD_SW2TX, STWLC68_CMD_SW2TX_SHIFT,
			STWLC68_CMD_VAL);
		if (ret) {
			hwlog_err("%s: write cmd(sw2tx) fail\n", __func__);
			continue;
		}
		if (mode == STWLC68_TX_MODE) {
			hwlog_info("%s: succ, cnt = %d\n", __func__, i);
			return 0;
		}
	}
	hwlog_err("%s: fail, cnt = %d\n", __func__, i);
	return -WLC_ERR_I2C_WR;
}

static bool stwlc68_is_updated_sram_match(u16 sram_id)
{
	int ret;
	struct stwlc68_chip_info chip_info;

	ret = stwlc68_get_chip_info(&chip_info);
	if (ret) {
		hwlog_err("%s: get chip info fail\n", __func__);
		return false;
	}

	hwlog_info("[%s] sram_id = 0x%x\n", __func__, chip_info.sram_id);
	return (sram_id == chip_info.sram_id);
}

static int stwlc68_write_sram_exe_cmd(void)
{
	return stwlc68_write_byte(STWLC68_EXE_SRAM_ADDR, STWLC68_EXE_SRAM_CMD);
}

static int stwlc68_write_sram_start_addr(u32 start_addr)
{
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	return stwlc68_write_block(di, STWLC68_SRAM_START_ADDR_ADDR,
		(u8 *)&start_addr, STWLC68_SRAM_START_ADDR_LEN);
}

static int stwlc68_write_data_to_sram(u32 start_addr,
	const u8 *sram_data, const int sram_size)
{
	int ret;
	int remaining = sram_size;
	int size_to_write;
	u32 written_already = 0;
	int address = start_addr;
	u8 buff[ST_RAMPATCH_PAGE_SIZE] = { 0 };

	while (remaining > 0) {
		size_to_write = remaining > ST_RAMPATCH_PAGE_SIZE ?
			ST_RAMPATCH_PAGE_SIZE : remaining;
		memcpy(buff, sram_data + written_already, size_to_write);
		ret = stwlc68_4addr_write_block(address, buff, size_to_write);
		if (ret) {
			hwlog_err("%s: fail, addr=0x%8x\n", __func__, address);
			return ret;
		}
		address += size_to_write;
		written_already += size_to_write;
		remaining -= size_to_write;
	}

	return 0;
}

static int stwlc68_program_sramupdate(const struct st_sram_info *sram_info)
{
	int ret;
	u32 start_addr;

	/* start_addr obtained from sram_data[4:7] in little endian */
	start_addr = (u32)((sram_info->sram_data[4] << 0) |
		(sram_info->sram_data[5] << 8) |
		(sram_info->sram_data[6] << 16) |
		(sram_info->sram_data[7] << 24));

	ret = stwlc68_write_data_to_sram(start_addr,
		sram_info->sram_data + ST_RAMPATCH_HEADER_SIZE,
		sram_info->sram_size - ST_RAMPATCH_HEADER_SIZE);
	if (ret)
		return ret;

	ret = stwlc68_write_sram_start_addr(start_addr + 1);
	if (ret)
		return ret;

	ret = stwlc68_write_sram_exe_cmd();
	if (ret)
		return ret;

	mdelay(5); /* delay 5ms for exe cmd */
	if (stwlc68_is_updated_sram_match(sram_info->sram_id))
		return 0;

	mdelay(5); /* delay 5ms for exe cmd */
	if (!stwlc68_is_updated_sram_match(sram_info->sram_id))
		return -WLC_ERR_MISMATCH;

	return 0;
}

static int stwlc68_find_sram_id(
	enum wireless_mode sram_mode, unsigned int *sram_id)
{
	int i;
	int ret;
	unsigned int fw_sram_num;
	struct stwlc68_chip_info chip_info;

	ret = stwlc68_get_chip_info(&chip_info);
	if (ret) {
		hwlog_err("%s: get chip info fail\n", __func__);
		return ret;
	}

	fw_sram_num = ARRAY_SIZE(stwlc68_sram);
	for (i = 0; i < fw_sram_num; i++) {
		if (sram_mode != stwlc68_sram[i].fw_sram_mode)
			continue;
		if ((chip_info.cut_id < stwlc68_sram[i].cut_id_from) ||
			(chip_info.cut_id > stwlc68_sram[i].cut_id_to))
			continue;
		if ((chip_info.cfg_id < stwlc68_sram[i].cfg_id_from) ||
			(chip_info.cfg_id > stwlc68_sram[i].cfg_id_to))
			continue;
		if ((chip_info.patch_id < stwlc68_sram[i].patch_id_from) ||
			(chip_info.patch_id > stwlc68_sram[i].patch_id_to))
			continue;
#ifndef WIRELESS_CHARGER_FACTORY_VERSION
		if ((g_sram_bad_addr < stwlc68_sram[i].bad_addr_from) ||
			(g_sram_bad_addr > stwlc68_sram[i].bad_addr_to))
			continue;
#endif
		if ((stwlc68_sram[i].sram_size <= ST_RAMPATCH_HEADER_SIZE) ||
			(stwlc68_sram[i].sram_size > ST_RAMPATCH_MAX_SIZE))
			continue;

		hwlog_info("[%s] bad_addr=0x%x sram_id=%d\n",
			__func__, g_sram_bad_addr, i);
		*sram_id = i;
		return 0;
	}

	return -WLC_ERR_MISMATCH;
}

static int stwlc68_pre_sramupdate(enum wireless_mode sram_mode)
{
	int ret;

	if (sram_mode == WIRELESS_TX_MODE) {
		ret = stwlc68_sw2tx();
		if (ret)
			return ret;

		msleep(50); /* delay 50ms for switching to tx mode */
	}

	return 0;
}

static int stwlc68_check_fwupdate(enum wireless_mode sram_mode)
{
	int ret;
	unsigned int sram_id = 0;
	unsigned int fw_sram_num;

	hwlog_info("[%s] chip_info: %s\n",
		__func__, stwlc68_get_chip_info_str());

	ret = stwlc68_pre_sramupdate(sram_mode);
	if (ret) {
		hwlog_err("%s: pre sramupdate failed\n", __func__);
		return ret;
	}

	ret = stwlc68_find_sram_id(sram_mode, &sram_id);
	if (ret) {
		hwlog_err("%s: sram no need update\n", __func__);
		return ret;
	}

	fw_sram_num = ARRAY_SIZE(stwlc68_sram);
	if (sram_id >= fw_sram_num) {
		hwlog_err("%s: sram_id=%d err\n", __func__, sram_id);
		return -WLC_ERR_PARA_WRONG;
	}

	if (stwlc68_is_updated_sram_match(stwlc68_sram[sram_id].sram_id))
		return 0;

	ret = stwlc68_program_sramupdate(&stwlc68_sram[sram_id]);
	if (ret) {
		hwlog_err("%s: sram update fail\n", __func__);
		return ret;
	}

	hwlog_info("[%s] sram update succ, chip_info: %s\n",
		__func__, stwlc68_get_chip_info_str());
	return 0;
}

static int stwlc68_get_rpp_format(u8 *rpp_format)
{
	int ret;
	int count = 0;
	u8 rcv_data[WLC_RPP_FORMAT_LEN] = { 0 };

	do {
		ret = stwlc68_send_msg(WLC_CMD_GET_RPP_FORMAT, NULL, 0);
		ret += stwlc68_receive_msg(rcv_data, WLC_RPP_FORMAT_LEN);
		if (!ret) {
			hwlog_info("[%s] succ\n", __func__);
			break;
		}
		count++;
		hwlog_info("%s: failed, try next time\n", __func__);
	} while (count < STWLC68_GET_TX_PARA_RETRY_CNT);

	/* rcv_data[0]:cmd rcv_data[1]:data */
	if (rcv_data[0] != WLC_CMD_GET_RPP_FORMAT) {
		hwlog_err("%s: cmd 0x%x err\n", __func__, rcv_data[0]);
		return -WLC_ERR_MISMATCH;
	}
	*rpp_format = rcv_data[1];

	return 0;
}

static int stwlc68_set_rpp_format(int rpp_val)
{
	int ret;
	u8 temp;

	if ((rpp_val <= 0) || ((rpp_val / WLC_TX_RPP_VAL_UNIT) > BYTE_MASK)) {
		hwlog_err("%s: input rpp val invalid\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	temp = ((u8)rpp_val) / WLC_TX_RPP_VAL_UNIT; /* 5W per bit, max 75W */
	temp <<= 4; /* high 4 bits means Pmax */

	/* switch tx 24bit rpp, pmax 20W */
	ret = stwlc68_send_msg_ack(WLC_CMD_SET_RPP_FORMAT,
		&temp, WLC_RPP_FORMAT_LEN - 1);
	if (ret) {
		hwlog_err("%s: set tx failed\n", __func__);
		return -WLC_ERR_I2C_WR;
	}

	/* switch rx 24bit rpp, pmax 20W */
	ret = stwlc68_write_byte(STWLC68_RX_RPP_SET_ADDR,
		STWLC68_RX_20W_RPP_VAL);
	if (ret) {
		hwlog_err("%s: set rx failed\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_limit_fan_speed(u8 limit_val)
{
	int ret;

	ret = stwlc68_send_msg_ack(WLC_CMD_LIMIT_FAN_SPEED, &limit_val,
		WLC_FAN_SPEED_LEN);
	if (ret) {
		hwlog_err("%s: fail, limit_val = 0x%x\n", __func__, limit_val);
		return ret;
	}

	hwlog_info("[%s] succ, limit_val = 0x%x\n", __func__, limit_val);
	return 0;
}

static int stwlc68_get_ept_type(u16 *ept_type)
{
	int ret;
	int count = 0;
	u8 rcv_data[WLC_EPT_TYPE_LEN] = { 0 };

	if (!ept_type) {
		hwlog_err("%s: input para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	do {
		ret = stwlc68_send_msg(WLC_CMD_GET_EPT_TYPE, NULL, 0);
		ret += stwlc68_receive_msg(rcv_data, WLC_EPT_TYPE_LEN);
		if (!ret) {
			hwlog_info("[%s] succ\n", __func__);
			break;
		}
		count++;
		hwlog_info("%s: failed, try next time\n", __func__);
	} while (count < STWLC68_GET_TX_PARA_RETRY_CNT);

	/* rcv_data[0]:cmd, rcv_data[1] && rcv_data[2]:data */
	if (rcv_data[0] != WLC_CMD_GET_EPT_TYPE) {
		hwlog_err("%s: cmd 0x%x err\n", __func__, rcv_data[0]);
		return -WLC_ERR_MISMATCH;
	}
	*ept_type = (rcv_data[2] << BITS_PER_BYTE) | rcv_data[1];

	return 0;
}

static bool stwlc68_get_ext_5v_fod_cfg(void)
{
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di)
		return false;

	return di->ext_5v_fod_cfg;
}

static int stwlc68_check_dev_back_color(char *color)
{
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || !color) {
		hwlog_err("%s: di null\n", __func__);
		return -1;
	}
	if (strlen(color) <= 0) {
		hwlog_err("%s: color invalid\n", __func__);
		return -1;
	}

	if (!strncmp(color, "puorange", strlen("puorange")) ||
		!strncmp(color, "pugreen", strlen("pugreen")))
		di->pu_shell_flag = true;
	else
		di->pu_shell_flag = false;

	hwlog_info("[%s] pu_shell_flag = %d\n", __func__, di->pu_shell_flag);
	return 0;
}

static bool stwlc68_check_tx_exist(void)
{
	int ret;
	u8 mode = 0;

	ret = stwlc68_get_mode(&mode);
	if (ret) {
		hwlog_err("%s: get rx mode fail\n", __func__);
		return false;
	}
	if (mode == STWLC68_RX_MODE)
		return true;

	return false;
}

static int stwlc68_send_charge_state(u8 chrg_state)
{
	int ret;

	ret = stwlc68_send_msg_ack(WLC_CMD_SEND_CHRG_STATE, &chrg_state,
		WLC_CHRG_STATE_LEN);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	hwlog_info("[%s] succ, charge_state = 0x%x\n", __func__, chrg_state);
	return 0;
}

static int stwlc68_send_charge_mode(u8 chrg_mode)
{
	int ret;

	ret = stwlc68_send_msg_ack(WLC_CMD_CHRG_MODE,
		&chrg_mode, WLC_CHRG_MODE_LEN);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	hwlog_info("[%s] succ, charge_mode = 0x%x\n", __func__, chrg_mode);
	return 0;
}

static int stwlc68_send_fod_status(u16 fod_status)
{
	int ret;

	ret = stwlc68_send_msg_ack(STWLC68_CMD_SEND_FOD_STATUS,
		(u8 *)&fod_status, STWLC68_FOD_STATUS_LEN);
	if (ret) {
		hwlog_err("%s: failed\n", __func__);
		return -WLC_ERR_I2C_WR;
	}
	hwlog_info("[%s] fod_status = 0x%x\n", __func__, fod_status);
	return 0;
}

static int stwlc68_kick_watchdog(void)
{
	int ret;

	ret = stwlc68_write_byte(STWLC68_WDT_FEED_ADDR, 0);
	if (ret)
		return -WLC_ERR_I2C_W;

	return 0;
}

static int stwlc68_fix_tx_fop(int fop)
{
	if (fop < STWLC68_FIXED_FOP_MIN || fop > STWLC68_FIXED_FOP_MAX) {
		hwlog_err("%s fixed fop %d exceeds range[%d, %d]\n",
			  __func__, fop, STWLC68_FIXED_FOP_MIN,
			  STWLC68_FIXED_FOP_MAX);
		return -WLC_ERR_I2C_R;
	}

	return stwlc68_send_msg_ack(STWLC68_CMD_FIX_TX_FOP,
				     (u8 *)(&fop), STWLC68_TX_FOP_LEN);
}

static int stwlc68_unfix_tx_fop(void)
{
	return stwlc68_send_msg_ack(STWLC68_CMD_UNFIX_TX_FOP, NULL, 0);
}

static char *stwlc68_get_rx_fod(void)
{
	int i;
	int ret;
	char tmp[STWLC68_RX_FOD_TMP_STR_LEN] = { 0 };
	u8 fod_arr[STWLC68_RX_FOD_LEN] = { 0 };
	static char fod_str[STWLC68_RX_FOD_STR_LEN] = { 0 };
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return "error";
	}

	if (STWLC68_RX_FOD_STR_LEN <=
		STWLC68_RX_FOD_TMP_STR_LEN * STWLC68_RX_FOD_LEN)
		return "space error";

	memset(fod_str, 0, STWLC68_RX_FOD_STR_LEN);
	ret = stwlc68_read_block(di, STWLC68_RX_FOD_ADDR,
		fod_arr, STWLC68_RX_FOD_LEN);
	if (ret) {
		hwlog_err("%s: read fod[%d] fail\n", __func__, i);
		return "read error";
	}

	for (i = 0; i < STWLC68_RX_FOD_LEN; i++) {
		snprintf(tmp, STWLC68_RX_FOD_TMP_STR_LEN, "%x ", fod_arr[i]);
		strncat(fod_str, tmp, strlen(tmp));
	}

	return fod_str;
}

static int stwlc68_set_rx_fod(char *fod_str)
{
	int ret;
	char *cur = fod_str;
	char *token = NULL;
	int i;
	u8 val = 0;
	const char *sep = " ,";
	u8 fod_arr[STWLC68_RX_FOD_LEN] = { 0 };
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}
	if (!fod_str) {
		hwlog_err("%s: input fod_str null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	for (i = 0; i < STWLC68_RX_FOD_LEN; i++) {
		token = strsep(&cur, sep);
		if (!token) {
			hwlog_err("%s: input fod_str number err\n", __func__);
			return -WLC_ERR_PARA_WRONG;
		}
		ret = kstrtou8(token, WLC_DECIMAL, &val);
		if (ret) {
			hwlog_err("%s: input fod_str type err\n", __func__);
			return -WLC_ERR_PARA_WRONG;
		}
		fod_arr[i] = val;
		hwlog_info("[%s] fod[%d] = 0x%x\n", __func__, i, fod_arr[i]);
	}

	return stwlc68_write_block(di, STWLC68_RX_FOD_ADDR,
		fod_arr, STWLC68_RX_FOD_LEN);
}

static int stwlc68_init_rx_fod_coef(struct stwlc68_dev_info *di)
{
	int tx_vset;
	int ret = -WLC_ERR_I2C_W;

	/* (0, 9)V, set 5v fod; [9, 12)V, set 9V fod; [12, 18)V, set 12V fod */
	tx_vset = stwlc68_get_tx_vout_reg();
	hwlog_info("[%s] tx_vout_reg: %dmV\n", __func__, tx_vset);

	if (tx_vset < 9000) {
		if (di->pu_shell_flag && di->ext_5v_fod_cfg)
			ret = stwlc68_write_block(di, STWLC68_RX_FOD_ADDR,
				di->ext_rx_fod_5v, STWLC68_RX_FOD_LEN);
		else
			ret = stwlc68_write_block(di, STWLC68_RX_FOD_ADDR,
				di->rx_fod_5v, STWLC68_RX_FOD_LEN);
	}
	else if (tx_vset < 12000)
		ret = stwlc68_write_block(di, STWLC68_RX_FOD_ADDR,
			di->rx_fod_9v, STWLC68_RX_FOD_LEN);
	else if (tx_vset < 18000)
		ret = stwlc68_write_block(di, STWLC68_RX_FOD_ADDR,
			di->rx_fod_15v, STWLC68_RX_FOD_LEN);
	if (ret)
		hwlog_err("%s: fail\n", __func__);

	return ret;
}

static int stwlc68_chip_init(int init_type)
{
	int ret = 0;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	switch (init_type) {
	case WIRELESS_CHIP_INIT:
		hwlog_info("[%s] DEFAULT_CHIP_INIT\n", __func__);
		ret += stwlc68_write_byte(STWLC68_FC_VRECT_DELTA_ADDR,
			STWLC68_FC_VRECT_DELTA / STWLC68_FC_VRECT_DELTA_STEP);
		/* full through */
	case ADAPTER_5V * WL_MVOLT_PER_VOLT:
		hwlog_info("[%s] 5V_CHIP_INIT\n", __func__);
		ret += stwlc68_write_block(di, STWLC68_LDO_CFG_ADDR,
			di->rx_ldo_cfg_5v, STWLC68_LDO_CFG_LEN);
		ret += stwlc68_init_rx_fod_coef(di);
		break;
	case ADAPTER_9V * WL_MVOLT_PER_VOLT:
		hwlog_info("[%s] 9V_CHIP_INIT\n", __func__);
		ret += stwlc68_write_block(di, STWLC68_LDO_CFG_ADDR,
			di->rx_ldo_cfg_9v, STWLC68_LDO_CFG_LEN);
		ret += stwlc68_init_rx_fod_coef(di);
		break;
	case WILREESS_SC_CHIP_INIT:
		hwlog_info("[%s] SC_CHIP_INIT\n", __func__);
		ret += stwlc68_write_block(di, STWLC68_LDO_CFG_ADDR,
			di->rx_ldo_cfg_sc, STWLC68_LDO_CFG_LEN);
		ret += stwlc68_init_rx_fod_coef(di);
		break;
	default:
		hwlog_info("%s: input para is invalid\n", __func__);
		break;
	}

	return ret;
}

static int stwlc68_stop_charging(void)
{
	int wired_channel_state;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	stop_charging_flag = 1;

	if (irq_abnormal_flag) {
		wired_channel_state =
			wireless_charge_get_wired_channel_state();
		if (wired_channel_state != WIRED_CHANNEL_ON) {
			hwlog_info("[%s] irq_abnormal,\t"
				"keep wireless switch on\n", __func__);
			irq_abnormal_flag = true;
			wlps_control(WLPS_RX_SW, WLPS_CTRL_ON);
		} else {
			di->irq_cnt = 0;
			irq_abnormal_flag = false;
			stwlc68_enable_irq(di);
			hwlog_info("[%s] wired channel on, enable irq_int\n",
				__func__);
		}
	}

	return 0;
}

static enum tx_adaptor_type stwlc68_get_tx_type(void)
{
	int ret;
	u8 adapter[STWLC68_TX_ADAPTER_TYPE_LEN] = { 0 };

	ret = stwlc68_get_tx_para(STWLC68_CMD_GET_TX_ADAPTER_TYPE,
				   adapter, STWLC68_TX_ADAPTER_TYPE_LEN);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return WIRELESS_TYPE_ERR;
	}
	if (adapter[0] != STWLC68_CMD_GET_TX_ADAPTER_TYPE) { /* bit[1]: cmd */
		hwlog_err("%s: cmd 0x%x err\n", __func__, adapter[0]);
		return WIRELESS_TYPE_ERR;
	}

	return (enum tx_adaptor_type)adapter[1]; /* bit[1]: type */
}

static void stwlc68_get_tx_main_capability(struct tx_capability *tx_cap)
{
	int ret;
	u8 ext_type;
	u8 tx_cap_data[TX_CAP_TOTAL] = { 0 };

	ret = stwlc68_get_tx_para(STWLC68_CMD_GET_TX_CAP,
				   tx_cap_data, TX_CAP_TOTAL);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		tx_cap->type = WIRELESS_TYPE_ERR;
		return;
	}
	if (tx_cap_data[0] != STWLC68_CMD_GET_TX_CAP) {
		hwlog_err("%s: cmd 0x%x err\n", __func__, tx_cap_data[0]);
		tx_cap->type = WIRELESS_TYPE_ERR;
		return;
	}
	ext_type = tx_cap_data[TX_CAP_ATTR] & STWLC68_TX_CAP_EXT_TYPE_MASK;
	tx_cap->type = tx_cap_data[TX_CAP_TYPE];
	tx_cap->vout_max =
	    tx_cap_data[TX_CAP_VOUT_MAX] * STWLC68_TX_CAP_VOUT_STEP;
	tx_cap->iout_max =
	    tx_cap_data[TX_CAP_IOUT_MAX] * STWLC68_TX_CAP_IOUT_STEP;
	tx_cap->can_boost =
	    tx_cap_data[TX_CAP_ATTR] & STWLC68_TX_CAP_CAN_BOOST_MASK;
	tx_cap->cable_ok =
	    tx_cap_data[TX_CAP_ATTR] & STWLC68_TX_CAP_CABLE_OK_MASK;
	tx_cap->no_need_cert =
	    tx_cap_data[TX_CAP_ATTR] & STWLC68_TX_CAP_NO_NEED_CERT_MASK;
	tx_cap->support_scp =
	    tx_cap_data[TX_CAP_ATTR] & STWLC68_TX_CAP_SUPPORT_SCP_MASK;
	tx_cap->support_12v =
	    tx_cap_data[TX_CAP_ATTR] & STWLC68_TX_CAP_SUPPORT_12V_MASK;
	tx_cap->support_extra_cap =
	    tx_cap_data[TX_CAP_ATTR] & STWLC68_TX_CAP_SUPPORT_EXTRA_BIT_MASK;
	if (tx_cap->vout_max > ADAPTER_9V * WL_MVOLT_PER_VOLT &&
	    !tx_cap->support_12v) {
		tx_cap->vout_max = ADAPTER_9V * WL_MVOLT_PER_VOLT;
		hwlog_info("[%s] tx not support 12V, vout_max set to %dmV\n",
		     __func__, tx_cap->vout_max);
	}
	if (ext_type == WLC_TX_EXT_TYPE_CAR)
		tx_cap->type += WLC_CAR_TX_TYPE_BASE;
	hwlog_info("[%s] type = 0x%x, vout_max = %d, iout_max = %d,\t"
		"can_boost = %d, cable_ok = %d, no_need_cert = %d,\t"
		"support_scp = %d, support_12V = %d, support_extra_cap = %d\n",
		__func__, tx_cap->type, tx_cap->vout_max, tx_cap->iout_max,
		tx_cap->can_boost, tx_cap->cable_ok, tx_cap->no_need_cert,
		tx_cap->support_scp, tx_cap->support_12v,
		tx_cap->support_extra_cap);
}

static void stwlc68_get_tx_extra_capability(
		struct tx_capability *tx_cap)
{
	int ret;
	u8 tx_extra_cap_data[TX_EXTRA_CAP_TOTAL];

	if (!tx_cap->support_extra_cap) {
		hwlog_info("[%s] tx not support extra capability\n", __func__);
		return;
	}
	ret = stwlc68_get_tx_para(STWLC68_CMD_GET_TX_EXTRA_CAP,
				   tx_extra_cap_data, TX_EXTRA_CAP_TOTAL);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return;
	}
	if (tx_extra_cap_data[0] != STWLC68_CMD_GET_TX_EXTRA_CAP) {
		hwlog_err("%s: cmd 0x%x err\n", __func__,
			  tx_extra_cap_data[0]);
		return;
	}
	tx_cap->support_fan =
	    tx_extra_cap_data[TX_EXTRA_CAP_ATTR1] &
	    STWLC68_TX_CAP_SUPPORT_FAN_MASK;
	tx_cap->support_tec =
	    tx_extra_cap_data[TX_EXTRA_CAP_ATTR1] &
	    STWLC68_TX_CAP_SUPPORT_TEC_MASK;
	tx_cap->support_fod_status =
	    tx_extra_cap_data[TX_EXTRA_CAP_ATTR1] &
	    STWLC68_TX_CAP_SUPPORT_QVAL_MASK;
	hwlog_info("[%s] support: fan = %d, tec = %d, fod_status = %d\n",
		__func__, tx_cap->support_fan, tx_cap->support_tec,
		tx_cap->support_fod_status);
}

static struct tx_capability *stwlc68_get_tx_capability(void)
{
	static struct tx_capability tx_cap;

	memset(&tx_cap, 0, sizeof(tx_cap));
	stwlc68_get_tx_main_capability(&tx_cap);
	stwlc68_get_tx_extra_capability(&tx_cap);
	return &tx_cap;
}

static u8 *stwlc68_get_tx_fw_version(void)
{
	int ret;
	int i;
	static u8 tx_fw_version[STWLC68_TX_FW_VERSION_STRING_LEN] = { 0 };
	/* bit[0]:command,  bit[1:2:3:4]: tx fw version */
	u8 data[STWLC68_TX_FW_VERSION_LEN + 1] = { 0 };
	u8 buff[STWLC68_RX_TMP_BUFF_LEN] = { 0 };

	memset(tx_fw_version, 0, STWLC68_TX_FW_VERSION_STRING_LEN);
	ret = stwlc68_get_tx_para(STWLC68_CMD_GET_TX_VERSION,
				   data, STWLC68_TX_FW_VERSION_LEN + 1);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return "error";
	}
	if (data[0] != STWLC68_CMD_GET_TX_VERSION) {
		hwlog_err("%s: cmd 0x%x err\n", __func__, data[0]);
		return "error";
	}

	for (i = STWLC68_TX_FW_VERSION_LEN; i >= 1; i--) {
		if (i != 1) /*data[1:4] - fw_version*/
			snprintf(buff, STWLC68_RX_TMP_BUFF_LEN, "0x%02x ",
				 data[i]);
		else
			snprintf(buff, STWLC68_RX_TMP_BUFF_LEN, "0x%02x",
				 data[i]);
		strncat(tx_fw_version, buff, strlen(buff));
	}
	return tx_fw_version;
}

#ifdef WIRELESS_CHARGER_FACTORY_VERSION
static int stwlc68_ldo_ready(void)
{
	int ret;

	ret = stwlc68_send_msg_ack(STWLC68_CMD_SEND_READY, NULL, 0);
	if (ret) {
		hwlog_info("%s:send ldo ready fail\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	hwlog_info("[%s]send ldo ready success\n", __func__);

	return ret;
}
#endif /* WIRELESS_CHARGER_FACTORY_VERSION */

static int stwlc68_get_tx_id(void)
{
	int ret;
	/* bit[0:1]: tx id parameters, not include command */
	u8 id_para[STWLC68_TX_ID_LEN - 1] = { 0x88, 0x66 }; /* stwlc68 id */
	/* bit[0]:command,  bit[1:2]: tx id parameters */
	u8 id_received[STWLC68_TX_ID_LEN] = { 0 };
	int tx_id;
	int count = 0;

	if (!power_cmdline_is_factory_mode()) {
		ret = stwlc68_check_fwupdate(WIRELESS_RX_MODE);
		if (!ret)
			(void)stwlc68_chip_init(WIRELESS_CHIP_INIT);
	}

	do {
		ret = stwlc68_send_msg(STWLC68_CMD_GET_TX_ID, id_para,
					STWLC68_TX_ID_LEN - 1);
		ret += stwlc68_receive_msg(id_received, STWLC68_TX_ID_LEN);
		if (!ret) {
			hwlog_info("[%s] get tx id, succ\n", __func__);
			break;
		}
		count++;
		hwlog_info("%s: get tx id, fail, try next time\n", __func__);
	} while (count < STWLC68_GET_TX_PARA_RETRY_CNT);

	if (id_received[0] != STWLC68_CMD_GET_TX_ID) {
		hwlog_err("%s:cmd 0x%x err\n", __func__, id_received[0]);
		return -WLC_ERR_MISMATCH;
	}

	tx_id = (id_received[1] << BITS_PER_BYTE) | id_received[2];

#ifdef WIRELESS_CHARGER_FACTORY_VERSION
	stwlc68_ldo_ready(); /*send ldo ready message */
#endif
	return tx_id;
}

static int stwlc68_send_msg_rx_vout(int rx_vout)
{
	int ret;

	ret = stwlc68_send_msg_ack(STWLC68_CMD_START_SAMPLE,
			(u8 *)(&rx_vout), STWLC68_VOUT_LEN);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_WR;
	}
	hwlog_info("[%s] rx_vout = %d\n", __func__, rx_vout);
	return ret;
}

static int stwlc68_send_msg_rx_iout(int rx_iout)
{
	int ret;

	ret = stwlc68_send_msg_ack(STWLC68_CMD_STOP_SAMPLE,
			(u8 *)(&rx_iout), STWLC68_IOUT_LEN);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_WR;
	}
	hwlog_info("[%s] rx_iout = %d\n", __func__, rx_iout);
	return ret;
}

static int stwlc68_send_msg_certification_confirm(bool succ_flag)
{
	if (succ_flag)
		return stwlc68_send_msg_ack(STWLC68_CMD_CERT_SUCC, NULL, 0);

	return stwlc68_send_msg_ack(STWLC68_CMD_CERT_FAIL, NULL, 0);
}

static int stwlc68_send_msg_rx_boost_succ(void)
{
	int ret;

	ret = stwlc68_send_msg_ack(STWLC68_CMD_RX_BOOST_SUCC, NULL, 0);
	if (ret) {
		hwlog_err("%s: send rx boost succ to TX fail\n", __func__);
		return -WLC_ERR_I2C_WR;
	}
	return ret;
}

static int stwlc68_get_tx_certification(
		u8 *random, unsigned int random_len,
		u8 *cipherkey, unsigned int key_len)
{
	int ret;
	int i;
	u8 data[STWLC68_RCVD_MSG_DATA_LEN + 1] = { 0 };

	if (!random || (random_len < WIRELESS_RANDOM_LEN) ||
		!cipherkey || (key_len < WIRELESS_TX_KEY_LEN)) {
		hwlog_err("%s: invalid para\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	/*send message  to tx: random number */
	for (i = 0; i < WIRELESS_RANDOM_LEN / STWLC68_SEND_MSG_DATA_LEN; i++) {
		ret = stwlc68_send_msg_ack(STWLC68_CMD_START_CERT + i,
			random + i * STWLC68_SEND_MSG_DATA_LEN,
			STWLC68_SEND_MSG_DATA_LEN);
		if (ret) {
			hwlog_err("%s: send msg to TX fail\n", __func__);
			return -WLC_ERR_I2C_WR;
		}
		hwlog_info("[%s] send rx %dth random data succ\n",
			__func__, i + 1);
	}

	/*get cipherkey from TX */
	for (i = 0; i < WIRELESS_TX_KEY_LEN / STWLC68_RCVD_MSG_DATA_LEN; i++) {
		ret = stwlc68_get_tx_para(STWLC68_CMD_GET_HASH + i,
			data, STWLC68_RCVD_MSG_DATA_LEN + 1);
		if (ret) {
			hwlog_err("%s: get tx cipherkey fail\n", __func__);
			return -WLC_ERR_I2C_WR;
		}
		/* byte[0]:cmd byte[1:4]:data */
		if (STWLC68_CMD_GET_HASH + i == data[0]) {
			memcpy(cipherkey + i * STWLC68_RCVD_MSG_DATA_LEN,
				data + 1, STWLC68_RCVD_MSG_DATA_LEN);
			hwlog_info("[%s] get tx %dth cipherkey succ\n",
				__func__, i + 1);
		}
	}
	return 0;
}

static int stwlc68_data_received_handler(struct stwlc68_dev_info *di)
{
	int ret;
	int i;
	u8 cmd;
	int plim;
	u8 buff[STWLC68_RCVD_MSG_PKT_LEN] = { 0 };

	/* byte[0]:header byte[1]:cmd byte[2:5]:data */
	ret = stwlc68_read_block(di, STWLC68_RCVD_MSG_HEADER_ADDR,
		buff, STWLC68_RCVD_MSG_PKT_LEN);
	if (ret) {
		hwlog_err("%s: read data received from TX fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	cmd = buff[1];
	hwlog_info("[%s] data received from TX, cmd: 0x%x\n", __func__, cmd);
	for (i = 2; i < STWLC68_RCVD_MSG_PKT_LEN; i++)
		hwlog_info("[%s] data:0x%x\n", __func__, buff[i]);

	switch (cmd) {
	case STWLC68_CMD_START_SAMPLE:
		di->irq_val &= ~STWLC68_RX_RCVD_MSG_INTR_LATCH;
		hwlog_info("[%s] received cmd: start sample\n", __func__);
		blocking_notifier_call_chain(&rx_event_nh,
			WIRELESS_CHARGE_START_SAMPLE, NULL);
		break;
	case STWLC68_CMD_STOP_SAMPLE:
		di->irq_val &= ~STWLC68_RX_RCVD_MSG_INTR_LATCH;
		hwlog_info("[%s] received cmd: stop sample\n", __func__);
		blocking_notifier_call_chain(&rx_event_nh,
			WIRELESS_CHARGE_STOP_SAMPLE, NULL);
		break;
	case WLC_CMD_TX_ALARM:
		di->irq_val &= ~STWLC68_RX_RCVD_MSG_INTR_LATCH;
		hwlog_info("[%s] received cmd: tx alarm\n", __func__);
		plim = buff[3]; /* byte[3]: plimit */
		blocking_notifier_call_chain(&rx_event_nh,
			WLC_RX_PWR_LIM_TX_ALARM, &plim);
		break;
	case WLC_CMD_TX_BOOST_ERR:
		di->irq_val &= ~STWLC68_RX_RCVD_MSG_INTR_LATCH;
		hwlog_info("[%s] received cmd: tx boost err\n", __func__);
		blocking_notifier_call_chain(&rx_event_nh,
			WLC_RX_PWR_LIM_TX_BST_ERR, NULL);
		break;
	default:
		break;
	}
	return 0;
}

static void stwlc68_rx_ready_handler(struct stwlc68_dev_info *di)
{
	int wired_ch_state;

	wired_ch_state = wireless_charge_get_wired_channel_state();
	if (wired_ch_state == WIRED_CHANNEL_ON) {
		hwlog_err("%s: wired channel on, ignore\n", __func__);
		return;
	}

	hwlog_info("%s rx ready, goto wireless charging\n", __func__);
	stop_charging_flag = 0;
	di->irq_cnt = 0;
	wired_chsw_set_wired_channel(WIRED_CHANNEL_CUTOFF);
	wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_ON);
	msleep(CHANNEL_SW_TIME);
	gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
	wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_OFF);
	blocking_notifier_call_chain(&rx_event_nh,
		WIRELESS_CHARGE_RX_READY, NULL);
	di->fist_monitor_flag = true;
	mod_delayed_work(system_wq, &di->monitor_work, msecs_to_jiffies(0));
}

static void stwlc68_handle_abnormal_irq(struct stwlc68_dev_info *di)
{
	static struct timespec64 ts64_timeout;
	struct timespec64 ts64_interval;
	struct timespec64 ts64_now;

	ts64_now = current_kernel_time64();
	ts64_interval.tv_sec = 0;
	ts64_interval.tv_nsec = WIRELESS_INT_TIMEOUT_TH * NSEC_PER_MSEC;

	hwlog_info("[%s] irq_cnt = %d\n", __func__, ++di->irq_cnt);
	/*power on interrupt happen first time, so start monitor it! */
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
			irq_abnormal_flag = true;
			wlps_control(WLPS_RX_SW, WLPS_CTRL_ON);
			stwlc68_disable_irq_nosync(di);
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

static void stwlc68_rx_power_on_handler(struct stwlc68_dev_info *di)
{
	u8 rx_ss = 0; /* ss: Signal Strength */
	bool power_on_good_flag = false;
	int wired_ch_state;

	wired_ch_state = wireless_charge_get_wired_channel_state();
	if (wired_ch_state == WIRED_CHANNEL_ON) {
		hwlog_err("%s: wired channel on, ignore\n", __func__);
		return;
	}

	stwlc68_handle_abnormal_irq(di);
	stwlc68_read_byte(STWLC68_RX_SS_ADDR, &rx_ss);
	hwlog_info("[%s] rx power on, Signal_Strength = %u\n", __func__, rx_ss);
	if ((rx_ss > di->rx_ss_good_lth) && (rx_ss <= STWLC68_RX_SS_MAX))
		power_on_good_flag = true;
	blocking_notifier_call_chain(&rx_event_nh,
		WIRELESS_CHARGE_RX_POWER_ON, &power_on_good_flag);
}

static void stwlc68_rx_mode_irq_recheck(
	struct stwlc68_dev_info *di, u16 irq_val)
{
	if (irq_val & STWLC68_RX_STATUS_READY)
		stwlc68_rx_ready_handler(di);
}

static void stwlc68_rx_mode_irq_handler(struct stwlc68_dev_info *di)
{
	int ret;
	u16 irq_value = 0;

	ret = stwlc68_read_word(STWLC68_INTR_LATCH_ADDR, &di->irq_val);
	if (ret) {
		hwlog_err("%s: read interrupt fail, clear\n", __func__);
		stwlc68_clear_interrupt(STWLC68_ALL_INTR_CLR);
		stwlc68_handle_abnormal_irq(di);
		goto clr_irq_again;
	}

	hwlog_info("%s interrupt 0x%04x\n", __func__, di->irq_val);
	stwlc68_clear_interrupt(di->irq_val);

	if (di->irq_val & STWLC68_RX_STATUS_POWER_ON) {
		di->irq_val &= ~STWLC68_RX_STATUS_POWER_ON;
		stwlc68_rx_power_on_handler(di);
	}
	if (di->irq_val & STWLC68_RX_STATUS_READY) {
		di->irq_val &= ~STWLC68_RX_STATUS_READY;
		stwlc68_rx_ready_handler(di);
	}
	if (di->irq_val & STWLC68_OCP_INTR_LATCH) {
		di->irq_val &= ~STWLC68_OCP_INTR_LATCH;
		blocking_notifier_call_chain(&rx_event_nh,
			WIRELESS_CHARGE_RX_OCP, NULL);
	}
	if (di->irq_val & STWLC68_OVP_INTR_LATCH) {
		di->irq_val &= ~STWLC68_OVP_INTR_LATCH;
		blocking_notifier_call_chain(&rx_event_nh,
			WIRELESS_CHARGE_RX_OVP, NULL);
	}
	if (di->irq_val & STWLC68_OVTP_INTR_LATCH) {
		di->irq_val &= ~STWLC68_OVTP_INTR_LATCH;
		blocking_notifier_call_chain(&rx_event_nh,
			WIRELESS_CHARGE_RX_OTP, NULL);
	}

	/* receice data from TX, please handler the interrupt */
	if (di->irq_val & STWLC68_RX_RCVD_MSG_INTR_LATCH)
		stwlc68_data_received_handler(di);

	if (di->irq_val & STWLC68_SYS_ERR_INTR_LATCH) {
		di->irq_val &= ~STWLC68_SYS_ERR_INTR_LATCH;
		hwlog_info("[%s] SYS_ERR_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_OUTPUT_ON_INTR_LATCH) {
		di->irq_val &= ~STWLC68_OUTPUT_ON_INTR_LATCH;
		hwlog_info("[%s] OUTPUT_ON_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_OUTPUT_OFF_INTR_LATCH) {
		di->irq_val &= ~STWLC68_OUTPUT_OFF_INTR_LATCH;
		hwlog_info("[%s] OUTPUT_OFF_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_SEND_PKT_SUCC_INTR_LATCH) {
		di->irq_val &= ~STWLC68_SEND_PKT_SUCC_INTR_LATCH;
		hwlog_info("[%s] SEND_PKT_SUCC_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_SEND_PKT_TIMEOUT_INTR_LATCH) {
		di->irq_val &= ~STWLC68_SEND_PKT_TIMEOUT_INTR_LATCH;
		hwlog_info("[%s] SEND_PKT_TIMEOUT_INTR_LATCH\n", __func__);
	}

clr_irq_again:
	/* clear interrupt again */
	if (!gpio_get_value(di->gpio_int)) {
		stwlc68_read_word(STWLC68_INTR_LATCH_ADDR, &irq_value);
		stwlc68_rx_mode_irq_recheck(di, irq_value);
		hwlog_info("[%s] gpio_int low, clear irq again! irq = 0x%x\n",
			   __func__, irq_value);
		stwlc68_clear_interrupt(STWLC68_ALL_INTR_CLR);
	}
}

static bool stwlc68_in_tx_mode(void)
{
	int ret;
	u8 mode = 0;

	ret = stwlc68_read_byte(STWLC68_OP_MODE_ADDR, &mode);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return false;
	}

	if (mode == STWLC68_TX_MODE)
		return true;
	else
		return false;
}

static void stwlc68_set_tx_open_flag(bool enable)
{
	g_tx_open_flag = enable;
}

static int stwlc68_tx_mode_vset(int tx_vset)
{
	if (tx_vset == STWLC68_PS_TX_VOLT_5V5)
		return stwlc68_write_byte(STWLC68_PS_TX_GPIO_ADDR,
			STWLC68_PS_TX_GPIO_PU);
	else if (tx_vset == STWLC68_PS_TX_VOLT_6V8)
		return stwlc68_write_byte(STWLC68_PS_TX_GPIO_ADDR,
			STWLC68_PS_TX_GPIO_OPEN);
	else if (tx_vset == STWLC68_PS_TX_VOLT_10V)
		return stwlc68_write_byte(STWLC68_PS_TX_GPIO_ADDR,
			STWLC68_PS_TX_GPIO_PD);

	hwlog_err("%s: para err\n", __func__);
	return -WLC_ERR_PARA_WRONG;
}

static bool stwlc68_check_rx_disconnect(void)
{
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return true;
	}

	if (di->ept_type & STWLC68_TX_EPT_SRC_CEP_TIMEOUT) {
		di->ept_type &= ~STWLC68_TX_EPT_SRC_CEP_TIMEOUT;
		hwlog_info("[%s] RX disconnect\n", __func__);
		return true;
	}

	return false;
}

static int stwlc68_get_tx_ping_interval(u16 *ping_interval)
{
	int ret;
	u8 data = 0;

	if (!ping_interval) {
		hwlog_err("%s: ping_interval null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_read_byte(STWLC68_TX_PING_INTERVAL, &data);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return ret;
	}
	*ping_interval = data * STWLC68_TX_PING_INTERVAL_STEP;

	return 0;
}

static int stwlc68_set_tx_ping_interval(u16 ping_interval)
{
	int ret;
	u16 reg_val;

	if ((ping_interval < STWLC68_TX_PING_INTERVAL_MIN) ||
		(ping_interval > STWLC68_TX_PING_INTERVAL_MAX)) {
		hwlog_err("%s: ping interval is out of range\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	reg_val = ping_interval / STWLC68_TX_PING_INTERVAL_STEP;
	ret = stwlc68_write_byte(STWLC68_TX_PING_INTERVAL, (u8)reg_val);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_get_tx_ping_frequency(u16 *ping_freq)
{
	int ret;
	u8 data = 0;

	if (!ping_freq) {
		hwlog_err("%s: ping_freq null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_read_byte(STWLC68_TX_PING_FREQ_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return ret;
	}
	*ping_freq = (u16)data;

	return 0;
}

static int stwlc68_set_tx_ping_frequency(u16 ping_freq)
{
	int ret;

	if ((ping_freq < STWLC68_TX_PING_FREQ_MIN) ||
		(ping_freq > STWLC68_TX_PING_FREQ_MAX)) {
		hwlog_err("%s: ping frequency is out of range\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	ret = stwlc68_write_byte(STWLC68_TX_PING_FREQ_ADDR, (u8)ping_freq);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return ret;
	}

	return ret;
}

static int stwlc68_get_tx_min_fop(u16 *fop)
{
	int ret;
	u8 data = 0;

	if (!fop) {
		hwlog_err("%s: fop null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_read_byte(STWCL68_TX_MIN_FOP_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return ret;
	}
	*fop = (u16)data;

	return 0;
}

static int stwlc68_set_tx_min_fop(u16 fop)
{
	int ret;

	if ((fop < STWLC68_TX_MIN_FOP_VAL) || (fop > STWLC68_TX_MAX_FOP_VAL))
		return -WLC_ERR_PARA_WRONG;

	ret = stwlc68_write_byte(STWCL68_TX_MIN_FOP_ADDR, (u8)fop);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_get_tx_max_fop(u16 *fop)
{
	int ret;
	u8 data = 0;

	if (!fop) {
		hwlog_err("%s: fop null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_read_byte(STWLC68_TX_MAX_FOP_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return ret;
	}
	*fop = (u16)data;

	return 0;
}

static int stwlc68_set_tx_max_fop(u16 fop)
{
	int ret;

	if ((fop < STWLC68_TX_MIN_FOP_VAL) || (fop > STWLC68_TX_MAX_FOP_VAL))
		return -WLC_ERR_PARA_WRONG;

	ret = stwlc68_write_byte(STWLC68_TX_MAX_FOP_ADDR, (u8)fop);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_get_tx_fop(u16 *fop)
{
	int ret;

	if (!fop) {
		hwlog_err("%s: fop null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_read_word(STWLC68_TX_OP_FREQ_ADDR, fop);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	return 0;
}


static int stwlc68_get_tx_temp(u8 *chip_temp)
{
	int ret;

	if (!chip_temp) {
		hwlog_err("%s: chip_temp null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_read_byte(STWLC68_CHIP_TEMP_ADDR, chip_temp);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_get_tx_vin(u16 *tx_vin)
{
	int ret;

	if (!tx_vin) {
		hwlog_err("%s: tx_vin null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_read_word(STWLC68_TX_VIN_ADDR, tx_vin);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_get_tx_vrect(u16 *tx_vrect)
{
	int ret;

	if (!tx_vrect) {
		hwlog_err("%s: tx_vrect null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_read_word(STWLC68_TX_VRECT_ADDR, tx_vrect);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_get_tx_iin(u16 *tx_iin)
{
	int ret;

	if (!tx_iin) {
		hwlog_err("%s: tx_iin null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_read_word(STWLC68_TX_IIN_ADDR, tx_iin);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_set_tx_ilimit(int tx_ilim)
{
	int ret;

	if ((tx_ilim < STWLC68_TX_ILIMIT_MIN) ||
		(tx_ilim > STWLC68_TX_ILIMIT_MAX))
		return -WLC_ERR_PARA_WRONG;

	ret = stwlc68_write_byte(STWLC68_TX_ILIMIT,
		(u8)(tx_ilim / STWLC68_TX_ILIMIT_STEP));
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_set_tx_fod_coef(u32 pl_th, u8 pl_cnt)
{
	int ret;

	pl_th /= STWLC68_TX_PLOSS_TH_UNIT;
	/* tx ploss threshold 0:disabled */
	ret = stwlc68_write_byte(STWLC68_TX_PLOSS_TH_ADDR, (u8)pl_th);
	/* tx ploss fod debounce count 0:no debounce */
	ret += stwlc68_write_byte(STWLC68_TX_PLOSS_CNT_ADDR, pl_cnt);

	return ret;
}

static void stwlc68_set_rp_dm_timeout_val(u8 val)
{
	if (stwlc68_write_byte(STWLC68_TX_RP_TIMEOUT_ADDR, val))
		hwlog_err("%s fail\n", __func__);
}

static int stwlc68_tx_stop_config(void)
{
	return 0;
}

static int stwlc68_tx_chip_init(void)
{
	int ret;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	di->irq_cnt = 0;
	irq_abnormal_flag = false;
	/*
	 * when stwlc68_handle_abnormal_irq disable irq, tx will irq-mask
	 * so, here enable irq
	 */
	stwlc68_enable_irq(di);

	ret = stwlc68_write_byte(STWLC68_TX_OTP_ADDR, STWLC68_TX_OTP_THRES);
	ret += stwlc68_write_byte(STWLC68_TX_OCP_ADDR,
		STWLC68_TX_OCP_VAL / STWLC68_TX_OCP_STEP);
	ret += stwlc68_write_byte(STWLC68_TX_OVP_ADDR,
		STWLC68_TX_OVP_VAL / STWLC68_TX_OVP_STEP);
	ret += stwlc68_write_word_mask(STWLC68_CMD_ADDR, STWLC68_CMD_TX_FOD_EN,
		STWLC68_CMD_TX_FOD_EN_SHIFT, STWLC68_CMD_VAL);
	ret += stwlc68_set_tx_ping_frequency(STWLC68_TX_PING_FREQ_INIT);
	ret += stwlc68_set_tx_min_fop(STWLC68_TX_MIN_FOP_VAL);
	ret += stwlc68_set_tx_max_fop(STWLC68_TX_MAX_FOP_VAL);
	ret += stwlc68_set_tx_ping_interval(STWLC68_TX_PING_INTERVAL_INIT);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static int stwlc68_enable_tx_mode(bool enable)
{
	int ret;

	if (enable)
		ret = stwlc68_write_word_mask(STWLC68_CMD_ADDR,
			STWLC68_CMD_TX_EN, STWLC68_CMD_TX_EN_SHIFT,
			STWLC68_CMD_VAL);
	else
		ret = stwlc68_write_word_mask(STWLC68_CMD_ADDR,
			STWLC68_CMD_TX_DIS, STWLC68_CMD_TX_DIS_SHIFT,
			STWLC68_CMD_VAL);

	if (ret) {
		hwlog_err("%s: %s tx mode fail\n", __func__,
			  enable ? "enable" : "disable");
		return ret;
	}

	return 0;
}

static int stwlc68_send_fsk_msg(u8 cmd, u8 *data, int data_len)
{
	int ret;
	u8 header;
	u8 write_data[STWLC68_SEND_MSG_DATA_LEN] = { 0 };
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	if ((data_len > STWLC68_SEND_MSG_DATA_LEN) || (data_len < 0)) {
		hwlog_err("%s: data len out of range\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	if (cmd == STWLC68_CMD_ACK)
		header = STWLC68_CMD_ACK_HEAD;
	else
		header = stwlc68_tx2rx_header[data_len + 1];

	ret = stwlc68_write_byte(STWLC68_SEND_MSG_HEADER_ADDR, header);
	if (ret) {
		hwlog_err("%s: write header fail\n", __func__);
		return ret;
	}
	ret = stwlc68_write_byte(STWLC68_SEND_MSG_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("%s: write cmd fail\n", __func__);
		return ret;
	}

	if (data && data_len > 0) {
		memcpy(write_data, data, data_len);
		ret = stwlc68_write_block(di, STWLC68_SEND_MSG_DATA_ADDR,
			write_data, data_len);
		if (ret) {
			hwlog_err("%s: write fsk reg fail\n", __func__);
			return ret;
		}
	}
	ret = stwlc68_write_word_mask(STWLC68_CMD_ADDR, STWLC68_CMD_SEND_MSG,
		STWLC68_CMD_SEND_MSG_SHIFT, STWLC68_CMD_VAL);
	if (ret) {
		hwlog_err("%s: send fsk fail\n", __func__);
		return ret;
	}

	hwlog_info("[%s] success\n", __func__);
	return 0;
}

static void stwlc68_send_fsk_ack_msg(void)
{
	int ret = stwlc68_send_fsk_msg(STWLC68_CMD_ACK, NULL, 0);

	if (ret)
		hwlog_err("%s: send tx ack to rx fail\n", __func__);
}

static void stwlc68_send_fsk_msg_tx_cap(u8 *cap, int len)
{
	int ret;

	if (!cap || (len != WLC_TX_CAP_TOTAL)) {
		hwlog_err("%s: para err\n", __func__);
		return;
	}

	ret = stwlc68_send_fsk_msg(WLC_CMD_GET_TX_CAP, cap, len);
	if (ret)
		hwlog_err("%s: send fsk msg tx capacity fail\n", __func__);
}

static void stwlc68_send_fsk_msg_tx_id(void)
{
	/* bit[0:1]: tx id parameters, not include command */
	u8 id_para[STWLC68_TX_ID_LEN - 1] = { 0x88, 0x66 };
	int ret;

	ret = stwlc68_send_fsk_msg(STWLC68_CMD_GET_TX_ID, id_para,
		STWLC68_TX_ID_LEN - 1);
	if (ret)
		hwlog_err("%s: send fsk msg tx_id fail\n", __func__);
}

static void stwlc68_send_fsk_msg_tx_fw(void)
{
	int ret;
	struct stwlc68_chip_info chip_info;
	/* len=4, byte[0:1] patch_id, byte[2:3] sram_id */
	u8 fw_ver[WLC_TX_FW_LEN] = { 0 };

	ret = stwlc68_get_chip_info(&chip_info);
	if (ret) {
		hwlog_err("%s: get chip info fail\n", __func__);
		return;
	}

	fw_ver[0] = (u8)((chip_info.patch_id >> 0) & BYTE_MASK);
	fw_ver[1] = (u8)((chip_info.patch_id >> 8) & BYTE_MASK);
	fw_ver[2] = (u8)((chip_info.sram_id >> 0) & BYTE_MASK);
	fw_ver[3] = (u8)((chip_info.sram_id >> 8) & BYTE_MASK);

	ret = stwlc68_send_fsk_msg(WLC_CMD_GET_TX_FW, fw_ver, WLC_TX_FW_LEN);
	if (ret)
		hwlog_err("%s: fail\n", __func__);
}

static int stwlc68_check_ask_header(u8 head)
{
	int i;

	for (i = 1; i < STWLC68_SEND_MSG_PKT_LEN; i++) {
		if (head == stwlc68_rx2tx_header[i])
			return true;
	}

	return false;
}

static int stwlc68_get_tx_ept_type(u16 *ept_type)
{
	int ret;
	u16 data = 0;

	if (!ept_type) {
		hwlog_err("%s: ept_type null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_read_word(STWLC68_TX_EPT_REASON_RCVD_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return ret;
	}
	*ept_type = data;
	hwlog_info("[%s] EPT type = 0x%04x", __func__, *ept_type);
	ret = stwlc68_write_word(STWLC68_TX_EPT_REASON_RCVD_ADDR, 0);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return ret;
	}

	return 0;
}

static void stwlc68_get_ask_pkt(u8 *pkt_data, unsigned int pkt_data_len)
{
	int ret;
	int i;
	char buff[STWLC68_RCVD_PKT_BUFF_LEN] = { 0 };
	char pkt_str[STWLC68_RCVD_PKT_STR_LEN] = { 0 };
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || !pkt_data || (pkt_data_len < STWLC68_RCVD_MSG_PKT_LEN)) {
		hwlog_err("%s: NULL pointer", __func__);
		return;
	}
	ret = stwlc68_read_block(di, STWLC68_RCVD_MSG_HEADER_ADDR, pkt_data,
		STWLC68_RCVD_MSG_PKT_LEN);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return;
	}
	for (i = 0; i < STWLC68_RCVD_MSG_PKT_LEN; i++) {
		snprintf(buff, STWLC68_RCVD_PKT_BUFF_LEN, "0x%02x ",
			pkt_data[i]);
		strncat(pkt_str, buff, strlen(buff));
	}
	hwlog_info("[%s] RX back packet: %s\n", __func__, pkt_str);
}

static void stwlc68_handle_ask_pkt(struct stwlc68_dev_info *di)
{
	u16 tx_id;
	u16 tx_vset;
	u8 chrg_stage;
	/* byte[0]:header; byte[1]:cmd; byte[2:5]:data */
	u8 pkt_data[STWLC68_RCVD_MSG_PKT_LEN] = { 0 };

	stwlc68_get_ask_pkt(pkt_data, STWLC68_RCVD_MSG_PKT_LEN);
	if (!stwlc68_check_ask_header(pkt_data[0])) {
		hwlog_err("%s: head(0x%x) not correct\n", __func__,
			pkt_data[0]);
		return;
	}
	switch (pkt_data[1]) {
	case STWLC68_CMD_GET_TX_ID:
		tx_id = (pkt_data[2] << BITS_PER_BYTE) | pkt_data[3];
		if (tx_id == TX_ID_HW) {
			stwlc68_send_fsk_msg_tx_id();
			hwlog_info("[%s] 0x8866 handshake succ\n", __func__);
			blocking_notifier_call_chain(&tx_event_nh,
				WL_TX_EVENT_HANDSHAKE_SUCC, NULL);
		}
		break;
	case WLC_CMD_GET_TX_CAP:
		blocking_notifier_call_chain(&tx_event_nh,
			WLTX_EVT_GET_TX_CAP, NULL);
		break;
	case WLC_CMD_SEND_CHRG_STATE:
		chrg_stage = pkt_data[2];
		hwlog_info("[%s] charge state 0x%x\n", __func__, chrg_stage);
		stwlc68_send_fsk_ack_msg(); /* tx ack to rx */
		if (chrg_stage & WIRELESS_STATE_CHRG_DONE) {
			hwlog_info("[%s] RX charge done\n", __func__);
			blocking_notifier_call_chain(&tx_event_nh,
				WL_TX_EVENT_CHARGEDONE, NULL);
		}
		break;
	case WLC_CMD_SET_TX_VOUT:
		/* pkt: 0x38 + 0x0a + (L_V) +(H_V) + Checksum */
		tx_vset = pkt_data[2] | (pkt_data[3] << BITS_PER_BYTE);
		hwlog_info("%s: tx_vset=%d\n", __func__, tx_vset);
		blocking_notifier_call_chain(&tx_event_nh,
			WLTX_EVT_TX_VSET, &tx_vset);
		break;
	case WLC_CMD_GET_TX_FW:
		stwlc68_send_fsk_msg_tx_fw();
		break;
	default:
		break;
	}
}

static void stwlc68_handle_tx_ept(struct stwlc68_dev_info *di)
{
	int ret;

	ret = stwlc68_get_tx_ept_type(&di->ept_type);
	if (ret) {
		hwlog_err("%s: get tx ept type fail\n", __func__);
		return;
	}
	switch (di->ept_type) {
	case STWLC68_TX_EPT_SRC_SS:
		di->ept_type &= ~STWLC68_TX_EPT_SRC_SS;
		hwlog_info("[%s] SS timeout\n", __func__);
		break;
	case STWLC68_TX_EPT_SRC_ID:
		di->ept_type &= ~STWLC68_TX_EPT_SRC_ID;
		hwlog_info("[%s] ID packet error\n", __func__);
		break;
	case STWLC68_TX_EPT_SRC_XID:
		di->ept_type &= ~STWLC68_TX_EPT_SRC_XID;
		hwlog_info("[%s] XID packet error\n", __func__);
		break;
	case STWLC68_TX_EPT_SRC_CFG_CNT_ERR:
		di->ept_type &= ~STWLC68_TX_EPT_SRC_CFG_CNT_ERR;
		hwlog_info("[%s] wrong CFG optional pkt number\n", __func__);
		break;
	case STWLC68_TX_EPT_SRC_CFG_ERR:
		di->ept_type &= ~STWLC68_TX_EPT_SRC_CFG_ERR;
		hwlog_info("[%s] CFG pkt error\n", __func__);
		break;
	case STWLC68_TX_EPT_SRC_RPP_TIMEOUT:
		di->ept_type &= ~STWLC68_TX_EPT_SRC_RPP_TIMEOUT;
		hwlog_info("[%s] RPP pkt timeout\n", __func__);
		break;
	case STWLC68_TX_EPT_SRC_CEP_TIMEOUT:
		di->ept_type &= ~STWLC68_TX_EPT_SRC_CEP_TIMEOUT;
		hwlog_info("[%s] RX disconnect\n", __func__);
		blocking_notifier_call_chain(&tx_event_nh,
			WL_TX_EVENT_CEP_TIMEOUT, NULL);
		break;
	case STWLC68_TX_EPT_SRC_FOD:
		di->ept_type &= ~STWLC68_TX_EPT_SRC_FOD;
		hwlog_info("[%s] fod happened\n", __func__);
		blocking_notifier_call_chain(&tx_event_nh,
			WLTX_EVT_TX_FOD, NULL);
		break;
	default:
		break;
	}
}

static void stwlc68_handle_qi_ask_pkt(struct stwlc68_dev_info *di)
{
	/* pkt: byte[0]:header; byte[2:5]:data */
	u8 pkt_data[STWLC68_RCVD_MSG_PKT_LEN] = { 0 };

	stwlc68_get_ask_pkt(pkt_data, STWLC68_RCVD_MSG_PKT_LEN);
	if (pkt_data[0] == WLC_QI_HEADER_SS)
		hwlog_info("[%s] signal strength=%d\n", __func__, pkt_data[1]);
}

static int stwlc68_clear_tx_interrupt(u16 itr)
{
	int ret;

	ret = stwlc68_write_word(STWLC68_TX_INTR_CLR_ADDR, itr);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static void stwlc68_tx_mode_irq_handler(struct stwlc68_dev_info *di)
{
	int ret;
	u16 irq_value;

	ret = stwlc68_read_word(STWLC68_TX_INTR_LATCH_ADDR, &di->irq_val);
	if (ret) {
		hwlog_err("%s: read interrupt fail, clear\n", __func__);
		stwlc68_clear_tx_interrupt(STWLC68_ALL_INTR_CLR);
		goto clr_irq_again;
	}

	hwlog_info("%s interrupt 0x%04x\n", __func__, di->irq_val);
	stwlc68_clear_tx_interrupt(di->irq_val);

	if (di->irq_val & STWLC68_TX_START_PING_LATCH) {
		hwlog_info("%s: tx PING interrupt\n", __func__);
		di->irq_val &= ~STWLC68_TX_START_PING_LATCH;
		blocking_notifier_call_chain(&tx_event_nh,
			WL_TX_EVENT_PING_RX, NULL);
	}

	if (di->irq_val & STWLC68_TX_SS_PKG_RCVD_LATCH) {
		hwlog_info("%s: Signal Strength packet interrupt\n", __func__);
		di->irq_val &= ~STWLC68_TX_SS_PKG_RCVD_LATCH;
		stwlc68_handle_qi_ask_pkt(di);
	}

	if (di->irq_val & STWLC68_TX_ID_PCKET_RCVD_LATCH) {
		hwlog_info("%s: ID packet interrupt\n", __func__);
		di->irq_val &= ~STWLC68_TX_ID_PCKET_RCVD_LATCH;
		stwlc68_handle_qi_ask_pkt(di);
	}

	if (di->irq_val & STWLC68_TX_CFG_PKT_RCVD_LATCH) {
		hwlog_info("%s: Config packet interrupt\n", __func__);
		di->irq_val &= ~STWLC68_TX_CFG_PKT_RCVD_LATCH;
		stwlc68_handle_qi_ask_pkt(di);
		blocking_notifier_call_chain(&tx_event_nh,
			WL_TX_EVENT_GET_CFG, NULL);
	}
	if (di->irq_val & STWLC68_TX_EPT_PKT_RCVD_LATCH) {
		di->irq_val &= ~STWLC68_TX_EPT_PKT_RCVD_LATCH;
		stwlc68_handle_tx_ept(di);
	}

	if (di->irq_val & STWLC68_TX_PP_PKT_RCVD_LATCH) {
		hwlog_info("[%s] TX rcv personal property ASK pkt\n", __func__);
		di->irq_val &= ~STWLC68_TX_PP_PKT_RCVD_LATCH;
		stwlc68_handle_ask_pkt(di);
	}

	if (di->irq_val & STWLC68_TX_OVTP_INTR_LATCH) {
		di->irq_val &= ~STWLC68_TX_OVTP_INTR_LATCH;
		hwlog_info("[%s] TX_OVTP_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_TX_OCP_INTR_LATCH) {
		di->irq_val &= ~STWLC68_TX_OCP_INTR_LATCH;
		hwlog_info("[%s] TX_OCP_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_TX_OVP_INTR_LATCH) {
		di->irq_val &= ~STWLC68_TX_OVP_INTR_LATCH;
		hwlog_info("[%s] TX_OVP_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_TX_SYS_ERR_INTR_LATCH) {
		di->irq_val &= ~STWLC68_TX_SYS_ERR_INTR_LATCH;
		hwlog_info("[%s] TX_SYS_ERR_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_TX_EXT_MON_INTR_LATCH) {
		di->irq_val &= ~STWLC68_TX_EXT_MON_INTR_LATCH;
		hwlog_info("[%s] TX_EXT_MON_INTR_LATCH\n", __func__);
		blocking_notifier_call_chain(&tx_event_nh,
			WLTX_EVT_EXT_MON_INTR, NULL);
	}

	if (di->irq_val & STWLC68_TX_SEND_PKT_SUCC_LATCH) {
		di->irq_val &= ~STWLC68_TX_SEND_PKT_SUCC_LATCH;
		hwlog_info("[%s] TX_SEND_PKT_SUCC_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_TX_SEND_PKT_TIMEOUT_LATCH) {
		di->irq_val &= ~STWLC68_TX_SEND_PKT_TIMEOUT_LATCH;
		hwlog_info("[%s] TX_SEND_PKT_TIMEOUT_LATCH\n", __func__);
	}
	if (di->irq_val & STWLC68_TX_RP_DM_TIMEOUT_LATCH) {
		di->irq_val &= ~STWLC68_TX_RP_DM_TIMEOUT_LATCH;
		hwlog_info("[%s] TX_RP_DM_TIMEOUT_LATCH\n", __func__);
		blocking_notifier_call_chain(&tx_event_nh,
			WLTX_EVT_RP_DM_TIMEOUT, NULL);
	}

clr_irq_again:
	if (!gpio_get_value(di->gpio_int)) {
		stwlc68_read_word(STWLC68_INTR_LATCH_ADDR, &irq_value);
		hwlog_info("[%s] gpio_int low, clear irq(0x%x) again\n",
			   __func__, irq_value);
		stwlc68_clear_tx_interrupt(STWLC68_ALL_INTR_CLR);
	}
}


static void stwlc68_irq_work(struct work_struct *work)
{
	int ret;
	u8 mode = 0;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		stwlc68_wake_unlock();
		return;
	}
	/* get System Operating Mode */
	ret = stwlc68_get_mode(&mode);
	if (!ret)
		hwlog_info("[%s] mode = 0x%x\n", __func__, mode);
	else
		stwlc68_handle_abnormal_irq(di);

	/* handler interrupt */
	if ((mode == STWLC68_TX_MODE) || (mode == STWLC68_STANDALONE_MODE))
		stwlc68_tx_mode_irq_handler(di);
	else if (mode == STWLC68_RX_MODE)
		stwlc68_rx_mode_irq_handler(di);

	if (!irq_abnormal_flag)
		stwlc68_enable_irq(di);

	stwlc68_wake_unlock();
}

static irqreturn_t stwlc68_interrupt(int irq, void *_di)
{
	struct stwlc68_dev_info *di = _di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return IRQ_HANDLED;
	}

	stwlc68_wake_lock();
	hwlog_info("[%s] ++\n", __func__);
	if (di->irq_active) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = 0;
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("irq is not enable,do nothing\n");
		stwlc68_wake_unlock();
	}
	hwlog_info("[%s] --\n", __func__);

	return IRQ_HANDLED;
}

static void stwlc68_monitor_work(struct work_struct *work)
{
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	stwlc68_monitor_rx_max_iout(di);

	di->fist_monitor_flag = false;
	if (!stop_charging_flag)
		schedule_delayed_work(&di->monitor_work,
			msecs_to_jiffies(100)); /* monitor interval: 100ms */
}

#ifndef WIRELESS_CHARGER_FACTORY_VERSION
static void stwlc68_sram_scan_work(struct work_struct *work)
{
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	hwlog_info("[%s] sram scan begin\n", __func__);
	wlps_control(WLPS_PROBE_PWR, WLPS_CTRL_ON);
	usleep_range(9500, 10500); /* wait 10ms for power supply */
	stwlc68_fw_ram_check();
	wlps_control(WLPS_PROBE_PWR, WLPS_CTRL_OFF);
	hwlog_info("[%s] sram scan end\n", __func__);

	g_sram_chk_complete = true;
}
#else
static void stwlc68_sram_scan_work(struct work_struct *work)
{
	g_sram_chk_complete = true;
}
#endif /* WIRELESS_CHARGER_FACTORY_VERSION */

static void stwlc68_pmic_vbus_handler(bool vbus_state)
{
	int ret;
	int wired_ch_state;
	u16 irq_val = 0;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	wired_ch_state = wireless_charge_get_wired_channel_state();
	if (vbus_state && irq_abnormal_flag &&
		wired_ch_state != WIRED_CHANNEL_ON &&
		stwlc68_check_tx_exist()) {
		ret = stwlc68_read_word(STWLC68_INTR_LATCH_ADDR, &irq_val);
		if (ret) {
			hwlog_err("%s: read interrupt fail, clear\n", __func__);
			return;
		}
		hwlog_info("[%s] irq_val = 0x%x\n", __func__, irq_val);
		if (irq_val & STWLC68_RX_STATUS_READY) {
			stwlc68_clear_interrupt(WORD_MASK);
			stwlc68_rx_ready_handler(di);
			di->irq_cnt = 0;
			irq_abnormal_flag = false;
			stwlc68_enable_irq(di);
		}
	}
}

static int stwlc68_parse_fod(struct device_node *np,
	struct stwlc68_dev_info *di)
{
	int i;
	int ret;
	u32 tmp[STWLC68_RX_FOD_LEN] = { 0 };

	ret = of_property_read_u32_array(np, "rx_fod_5v",
		tmp, STWLC68_RX_FOD_LEN);
	if (ret) {
		hwlog_err("%s: rx_fod_5v null\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < STWLC68_RX_FOD_LEN; i++) {
		di->rx_fod_5v[i] = (u8)tmp[i];
		hwlog_info("[%s] rx_fod_5v[%d] = 0x%02x\n",
			__func__, i, di->rx_fod_5v[i]);
	}

	ret = of_property_read_u32_array(np, "ext_rx_fod_5v",
		tmp, STWLC68_RX_FOD_LEN);
	if (!ret) {
		for (i = 0; i < STWLC68_RX_FOD_LEN; i++) {
			di->ext_rx_fod_5v[i] = (u8)tmp[i];
			hwlog_info("[%s] ext_rx_fod_5v[%d] = 0x%02x\n",
				__func__, i, di->ext_rx_fod_5v[i]);
		}
		di->ext_5v_fod_cfg = true;
	}
	hwlog_info("[%s] ext_5v_fod_cfg = %d\n", __func__, di->ext_5v_fod_cfg);

	ret = of_property_read_u32_array(np, "rx_fod_9v",
		tmp, STWLC68_RX_FOD_LEN);
	if (ret) {
		hwlog_err("%s: rx_fod_9v null\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < STWLC68_RX_FOD_LEN; i++) {
		di->rx_fod_9v[i] = (u8)tmp[i];
		hwlog_info("[%s] rx_fod_9v[%d] = 0x%02x\n",
			__func__, i, di->rx_fod_9v[i]);
	}

	ret = of_property_read_u32_array(np, "rx_fod_15v",
		tmp, STWLC68_RX_FOD_LEN);
	if (ret) {
		hwlog_err("%s: rx_fod_15v null\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < STWLC68_RX_FOD_LEN; i++) {
		di->rx_fod_15v[i] = (u8)tmp[i];
		hwlog_info("[%s] rx_fod_15v[%d] = 0x%02x\n",
			__func__, i, di->rx_fod_15v[i]);
	}

	return 0;
}

static int stwlc68_parse_ldo_cfg(struct device_node *np,
	struct stwlc68_dev_info *di)
{
	int i;
	int ret;
	u32 tmp[STWLC68_LDO_CFG_LEN] = { 0 };

	ret = of_property_read_u32_array(np, "rx_ldo_cfg_5v",
		tmp, STWLC68_LDO_CFG_LEN);
	if (ret) {
		hwlog_err("%s: rx_ldo_cfg_5v err\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < STWLC68_LDO_CFG_LEN; i++) {
		di->rx_ldo_cfg_5v[i] = (u8)tmp[i];
		hwlog_info("[%s] rx_ldo_cfg_5v[%d] = 0x%x\n",
			__func__, i, di->rx_ldo_cfg_5v[i]);
	}

	ret = of_property_read_u32_array(np, "rx_ldo_cfg_9v",
		tmp, STWLC68_LDO_CFG_LEN);
	if (ret) {
		hwlog_err("%s: rx_ldo_cfg_9v err\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < STWLC68_LDO_CFG_LEN; i++) {
		di->rx_ldo_cfg_9v[i] = (u8)tmp[i];
		hwlog_info("[%s] rx_ldo_cfg_9v[%d] = 0x%x\n",
			__func__, i, di->rx_ldo_cfg_9v[i]);
	}

	ret = of_property_read_u32_array(np, "rx_ldo_cfg_sc",
		tmp, STWLC68_LDO_CFG_LEN);
	if (ret) {
		hwlog_err("%s: rx_ldo_cfg_sc err\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < STWLC68_LDO_CFG_LEN; i++) {
		di->rx_ldo_cfg_sc[i] = (u8)tmp[i];
		hwlog_info("[%s] rx_ldo_cfg_sc[%d] = 0x%x\n",
			__func__, i, di->rx_ldo_cfg_sc[i]);
	}

	return 0;
}

static void stwlc68_parse_rx_temp_para(struct device_node *np,
	struct stwlc68_dev_info *di)
{
	int i;
	int ret;
	int array_len;
	u32 tmp_para[STWLC68_TEMP_PARA_LEVEL * ST68_TEMP_INFO_MAX] = { 0 };

	array_len = of_property_count_u32_elems(np, "temp_para");
	if ((array_len <= 0) || (array_len % ST68_TEMP_INFO_MAX) ||
		(array_len > STWLC68_TEMP_PARA_LEVEL * ST68_TEMP_INFO_MAX)) {
		hwlog_err("%s: temp_para invalid\n", __func__);
		return;
	}

	ret = of_property_read_u32_array(np, "temp_para",
		tmp_para, array_len);
	if (ret) {
		hwlog_err("%s: get temp_para fail\n", __func__);
		return;
	}

	for (i = 0; i < array_len / ST68_TEMP_INFO_MAX; i++) {
		di->temp_para[i].temp_lth = tmp_para[ST68_TEMP_INFO_MAX * i +
			ST68_TEMP_INFO_TEMP_LTH];
		di->temp_para[i].temp_hth = tmp_para[ST68_TEMP_INFO_MAX * i +
			ST68_TEMP_INFO_TEMP_HTH];
		di->temp_para[i].iout_max = tmp_para[ST68_TEMP_INFO_MAX * i +
			ST68_TEMP_INFO_IOUT_MAX];
		di->temp_para[i].temp_back = tmp_para[ST68_TEMP_INFO_MAX * i +
			ST68_TEMP_INFO_TEMP_BACK];

		hwlog_info("[%s][%d] t_lth:%d t_hth:%d iout:%dmA tmp_back:%d\n",
			__func__, i, di->temp_para[i].temp_lth,
			di->temp_para[i].temp_hth, di->temp_para[i].iout_max,
			di->temp_para[i].temp_back);
	}
}

static void stwlc68_parse_rx_time_para(struct device_node *np,
	struct stwlc68_dev_info *di)
{
	int i;
	int ret;
	int array_len;
	u32 tmp_para[STWLC68_TIME_PARA_LEVEL * ST68_TIME_INFO_MAX] = { 0 };

	array_len = of_property_count_u32_elems(np, "time_para");
	if ((array_len <= 0) || (array_len % ST68_TIME_INFO_MAX) ||
		(array_len > STWLC68_TIME_PARA_LEVEL * ST68_TIME_INFO_MAX)) {
		hwlog_err("%s: time_para invalid\n", __func__);
		return;
	}

	ret = of_property_read_u32_array(np, "time_para",
		tmp_para, array_len);
	if (ret) {
		hwlog_err("%s: get time_para fail\n", __func__);
		return;
	}

	for (i = 0; i < array_len / ST68_TIME_INFO_MAX; i++) {
		di->time_para[i].time_th = tmp_para[ST68_TIME_INFO_MAX * i +
			ST68_TIME_INFO_TIME_TH];
		di->time_para[i].iout_max = tmp_para[ST68_TIME_INFO_MAX * i +
			ST68_TIME_INFO_IOUT_MAX];

		hwlog_info("[%s][%d] time_lth:%ds iout:%dmA\n",
			__func__, i, di->time_para[i].time_th,
			di->time_para[i].iout_max);
	}
}

static int stwlc68_parse_dts(struct device_node *np,
	struct stwlc68_dev_info *di)
{
	int ret;

	ret = of_property_read_u32(np, "rx_ss_good_lth",
			(u32 *)&di->rx_ss_good_lth);
	if (ret)
		di->rx_ss_good_lth = STWLC68_RX_SS_MAX;
	hwlog_info("[%s] rx_ss_good_lth = %d\n",
		__func__, di->rx_ss_good_lth);

	ret = of_property_read_u32(np, "gpio_en_valid_val",
			(u32 *)&di->gpio_en_valid_val);
	if (ret)
		di->gpio_en_valid_val = 0;
	hwlog_info("[%s] gpio_en_valid_val = %d\n",
		__func__, di->gpio_en_valid_val);

	ret = stwlc68_parse_fod(np, di);
	if (ret) {
		hwlog_err("%s: parse fod para fail\n", __func__);
		return ret;
	}

	ret = stwlc68_parse_ldo_cfg(np, di);
	if (ret) {
		hwlog_err("%s: parse ldo cfg fail\n", __func__);
		return ret;
	}

	stwlc68_parse_rx_temp_para(np, di);
	stwlc68_parse_rx_time_para(np, di);

	return 0;
}

static int stwlc68_gpio_init(struct stwlc68_dev_info *di,
			      struct device_node *np)
{
	int ret;

	/* gpio_en */
	di->gpio_en = of_get_named_gpio(np, "gpio_en", 0);
	hwlog_info("[%s] gpio_en = %d\n", __func__, di->gpio_en);
	if (!gpio_is_valid(di->gpio_en)) {
		hwlog_err("%s: gpio_en not valid\n", __func__);
		ret = -EINVAL;
		goto gpio_en_fail;
	}
	ret = gpio_request(di->gpio_en, "stwlc68_en");
	if (ret) {
		hwlog_err("%s: could not request stwlc68_en\n", __func__);
		goto gpio_en_fail;
	}
	ret = gpio_direction_output(di->gpio_en, di->gpio_en_valid_val);
	if (ret)
		hwlog_err("%s: gpio_dir_output fail\n", __func__);

	/* gpio_sleep_en */
	di->gpio_sleep_en = of_get_named_gpio(np, "gpio_sleep_en", 0);
	if (!gpio_is_valid(di->gpio_sleep_en)) {
		hwlog_err("%s: gpio_sleep_en not valid\n", __func__);
		ret = -EINVAL;
		goto gpio_sleep_en_fail;
	}
	hwlog_info("[%s] gpio_sleep_en = %d\n", __func__, di->gpio_sleep_en);
	ret = gpio_request(di->gpio_sleep_en, "stwlc68_sleep_en");
	if (ret) {
		hwlog_err("%s: could not request stwlc68_sleep_en\n", __func__);
		goto gpio_sleep_en_fail;
	}
	ret = gpio_direction_output(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
	if (ret)
		hwlog_err("%s: gpio_dir_output fail\n", __func__);

	/* gpio_pwr_good */
	di->gpio_pwr_good = of_get_named_gpio(np, "gpio_pwr_good", 0);
	if (di->gpio_pwr_good <= 0)
		return 0;
	if (!gpio_is_valid(di->gpio_pwr_good)) {
		hwlog_err("%s: gpio_pwr_good not valid\n", __func__);
		ret = -EINVAL;
		goto gpio_pwr_good_fail;
	}
	hwlog_info("[%s] gpio_pwr_good = %d\n", __func__, di->gpio_pwr_good);
	ret = gpio_request(di->gpio_pwr_good, "stwlc68_pwr_good");
	if (ret) {
		hwlog_err("%s: could not request stwlc68_pwr_good\n", __func__);
		goto gpio_pwr_good_fail;
	}
	ret = gpio_direction_input(di->gpio_pwr_good);
	if (ret) {
		hwlog_err("%s: set gpio_pwr_good input fail\n", __func__);
		goto pwr_good_dir_fail;
	}

	return 0;

pwr_good_dir_fail:
	gpio_free(di->gpio_pwr_good);
gpio_pwr_good_fail:
	gpio_free(di->gpio_sleep_en);
gpio_sleep_en_fail:
	gpio_free(di->gpio_en);
gpio_en_fail:
	return ret;
}

static int stwlc68_irq_init(struct stwlc68_dev_info *di,
			     struct device_node *np)
{
	int ret;

	di->gpio_int = of_get_named_gpio(np, "gpio_int", 0);
	hwlog_info("[%s] gpio_int = %d\n", __func__, di->gpio_int);
	if (!gpio_is_valid(di->gpio_int)) {
		hwlog_err("%s: gpio_int is not valid\n", __func__);
		ret = -EINVAL;
		goto irq_init_fail_0;
	}
	ret = gpio_request(di->gpio_int, "stwlc68_int");
	if (ret) {
		hwlog_err("%s: could not request stwlc68_int\n", __func__);
		goto irq_init_fail_0;
	}
	gpio_direction_input(di->gpio_int);
	di->irq_int = gpio_to_irq(di->gpio_int);
	if (di->irq_int < 0) {
		hwlog_err("%s: could not map gpio_int to irq\n", __func__);
		ret = -EINVAL;
		goto irq_init_fail_1;
	}
	ret = request_irq(di->irq_int, stwlc68_interrupt,
			  IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND,
			  "stwlc68_irq", di);
	if (ret) {
		hwlog_err("could not request stwlc68_irq\n");
		di->irq_int = -EINVAL;
		goto irq_init_fail_1;
	}
	enable_irq_wake(di->irq_int);
	di->irq_active = 1;
	INIT_WORK(&di->irq_work, stwlc68_irq_work);

	return 0;

irq_init_fail_1:
	gpio_free(di->gpio_int);
irq_init_fail_0:
	return ret;
}

static struct wireless_charge_device_ops stwlc68_ops = {
	.chip_init              = stwlc68_chip_init,
	.chip_reset             = stwlc68_chip_reset,
	.ext_pwr_ctrl_init      = stwlc68_ext_pwr_ctrl_init,
	.get_die_id             = stwlc68_get_die_id,
	.get_tx_id              = stwlc68_get_tx_id,
	.get_rx_max_iout        = stwlc68_get_rx_max_iout,
	.get_rx_vrect           = stwlc68_get_rx_vrect,
	.get_rx_vout            = stwlc68_get_rx_vout,
	.get_rx_iout            = stwlc68_get_rx_iout,
	.get_rx_vout_reg        = stwlc68_get_rx_vout_reg,
	.get_tx_vout_reg        = stwlc68_get_tx_vout_reg,
	.set_tx_vout            = stwlc68_set_tx_vout,
	.set_rx_vout            = stwlc68_set_rx_vout,
	.get_rx_fop             = stwlc68_get_rx_fop,
	.get_rx_temp            = stwlc68_get_rx_temp,
	.get_tx_adaptor_type    = stwlc68_get_tx_type,
	.get_chip_info          = stwlc68_get_chip_info_str,
	.get_rx_chip_id         = stwlc68_get_chip_id,
	.get_rx_fod_coef        = stwlc68_get_rx_fod,
	.set_rx_fod_coef        = stwlc68_set_rx_fod,
	.fix_tx_fop             = stwlc68_fix_tx_fop,
	.unfix_tx_fop           = stwlc68_unfix_tx_fop,
	.rx_enable              = stwlc68_chip_enable,
	.rx_sleep_enable        = stwlc68_sleep_enable,
	.check_tx_exist         = stwlc68_check_tx_exist,
	.send_chrg_state        = stwlc68_send_charge_state,
	.send_chrg_mode         = stwlc68_send_charge_mode,
	.send_fod_status        = stwlc68_send_fod_status,
	.kick_watchdog          = stwlc68_kick_watchdog,
	.check_fwupdate         = stwlc68_check_fwupdate,
	.get_tx_capability      = stwlc68_get_tx_capability,
	.get_tx_fw_version      = stwlc68_get_tx_fw_version,
	.send_ept               = stwlc68_send_ept,
	.stop_charging          = stwlc68_stop_charging,
	.get_tx_cert            = stwlc68_get_tx_certification,
	.send_msg_rx_vout       = stwlc68_send_msg_rx_vout,
	.send_msg_rx_iout       = stwlc68_send_msg_rx_iout,
	.send_msg_cert_confirm  = stwlc68_send_msg_certification_confirm,
	.send_msg_rx_boost_succ = stwlc68_send_msg_rx_boost_succ,
	.pmic_vbus_handler      = stwlc68_pmic_vbus_handler,
	.rx_program_otp         = stwlc68_program_otp,
	.rx_check_otp           = stwlc68_check_otp,
	.check_is_otp_exist     = stwlc68_check_is_otp_exist,
	.cp_chip_init           = stwlc68_cp_chip_init,
	.set_bp_mode            = stwlc68_set_bp_mode,
	.set_cp_mode            = stwlc68_set_cp_mode,
	.is_bp_open             = stwlc68_is_bp_open,
	.is_cp_open             = stwlc68_is_cp_really_open,
	.get_cp_ratio           = stwlc68_get_cp_ratio,
	.get_rpp_format         = stwlc68_get_rpp_format,
	.set_rpp_format         = stwlc68_set_rpp_format,
	.limit_fan_speed        = stwlc68_limit_fan_speed,
	.get_ept_type           = stwlc68_get_ept_type,
	.get_ext_5v_fod_cfg     = stwlc68_get_ext_5v_fod_cfg,
	.check_dev_back_color   = stwlc68_check_dev_back_color,
};

static struct wireless_tx_device_ops stwlc68_tx_ops = {
	.rx_enable              = stwlc68_chip_enable,
	.rx_sleep_enable        = stwlc68_sleep_enable,
	.chip_reset             = stwlc68_chip_reset,
	.enable_tx_mode         = stwlc68_enable_tx_mode,
	.tx_chip_init           = stwlc68_tx_chip_init,
	.tx_stop_config         = stwlc68_tx_stop_config,
	.check_fwupdate         = stwlc68_check_fwupdate,
	.kick_watchdog          = stwlc68_kick_watchdog,
	.get_tx_iin             = stwlc68_get_tx_iin,
	.get_tx_vrect           = stwlc68_get_tx_vrect,
	.get_tx_vin             = stwlc68_get_tx_vin,
	.get_chip_temp          = stwlc68_get_tx_temp,
	.get_tx_fop             = stwlc68_get_tx_fop,
	.set_tx_max_fop         = stwlc68_set_tx_max_fop,
	.get_tx_max_fop         = stwlc68_get_tx_max_fop,
	.set_tx_min_fop         = stwlc68_set_tx_min_fop,
	.get_tx_min_fop         = stwlc68_get_tx_min_fop,
	.set_tx_ping_frequency  = stwlc68_set_tx_ping_frequency,
	.get_tx_ping_frequency  = stwlc68_get_tx_ping_frequency,
	.set_tx_ping_interval   = stwlc68_set_tx_ping_interval,
	.get_tx_ping_interval   = stwlc68_get_tx_ping_interval,
	.check_rx_disconnect    = stwlc68_check_rx_disconnect,
	.in_tx_mode             = stwlc68_in_tx_mode,
	.set_tx_open_flag       = stwlc68_set_tx_open_flag,
	.send_tx_cap            = stwlc68_send_fsk_msg_tx_cap,
	.set_tx_ilimit          = stwlc68_set_tx_ilimit,
	.set_tx_fod_coef        = stwlc68_set_tx_fod_coef,
	.set_rp_dm_timeout_val  = stwlc68_set_rp_dm_timeout_val,
};

static struct wlps_tx_ops swtlc68_ps_tx_ops = {
	.tx_vset                = stwlc68_tx_mode_vset,
};

static int wlps_tx_ops_init(struct stwlc68_dev_info *di)
{
	int ret;
	u32 tx_ps_ctrl_src = 0;

	ret = power_dts_read_u32_compatible("huawei,wireless_ps",
		"tx_ps_ctrl_src", &tx_ps_ctrl_src, 0);
	if (!ret && (tx_ps_ctrl_src == WLPS_TX_SRC_TX_CHIP))
		return wlps_tx_ops_register(&swtlc68_ps_tx_ops);

	return -WLC_ERR_MISMATCH;
}

static void stwlc68_shutdown(struct i2c_client *client)
{
	int wired_channel_state;

	hwlog_info("[%s] ++\n", __func__);
	wired_channel_state = wireless_charge_get_wireless_channel_state();
	if (wired_channel_state == WIRELESS_CHANNEL_ON) {
		wlps_control(WLPS_RX_EXT_PWR, WLPS_CTRL_OFF);
		msleep(50); /* dalay 50ms for power off */
		stwlc68_set_tx_vout(ADAPTER_5V * WL_MVOLT_PER_VOLT);
		stwlc68_set_rx_vout(ADAPTER_5V * WL_MVOLT_PER_VOLT);
		stwlc68_chip_enable(RX_EN_DISABLE);
		msleep(STWLC68_SHUTDOWN_SLEEP_TIME);
		stwlc68_chip_enable(RX_EN_ENABLE);
	}
	hwlog_info("[%s] --\n", __func__);
}

static int stwlc68_probe(struct i2c_client *client,
			  const struct i2c_device_id *id)
{
	int ret;
	u16 chip_id = 0;
	struct stwlc68_dev_info *di = NULL;
	struct device_node *np = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!client || !client->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_stwlc68_di = di;
	di->dev = &client->dev;
	np = client->dev.of_node;
	di->client = client;
	di->cp_ops = g_wlcp_ops;
	i2c_set_clientdata(client, di);

	ret = stwlc68_parse_dts(np, di);
	if (ret)
		goto parse_dts_fail;

	ret = stwlc68_gpio_init(di, np);
	if (ret)
		goto gpio_init_fail;
	ret = stwlc68_irq_init(di, np);
	if (ret)
		goto irq_init_fail;

	wakeup_source_init(&g_stwlc68_wakelock, "stwlc68_wakelock");
	mutex_init(&di->mutex_irq);
	INIT_DELAYED_WORK(&di->monitor_work, stwlc68_monitor_work);

	ret = wireless_charge_ops_register(&stwlc68_ops);
	if (ret) {
		hwlog_err("%s: register ops fail\n", __func__);
		goto ops_regist_fail;
	}

	if (stwlc68_check_tx_exist()) {
		stwlc68_clear_interrupt(STWLC68_ALL_INTR_CLR);
		hwlog_info("[%s] rx exsit, exe rx_ready_handler\n", __func__);
		stwlc68_rx_ready_handler(di);
	} else {
		gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_ENABLE);
	}

	ret = wireless_tx_ops_register(&stwlc68_tx_ops);
	if (ret) {
		hwlog_err("%s: register wireless tx ops fail\n", __func__);
		goto ops_regist_fail;
	}

	wlps_tx_ops_init(di);

	stwlc68_get_chip_id(&chip_id);
	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = chip_id;
		power_dev_info->ver_id = 0;
	}
	if (!power_cmdline_is_powerdown_charging_mode()) {
		INIT_DELAYED_WORK(&di->sram_scan_work, stwlc68_sram_scan_work);
		schedule_delayed_work(&di->sram_scan_work,
			msecs_to_jiffies(1500)); /* delay for wireless probe */
	} else {
		g_sram_chk_complete = true;
	}

	hwlog_info("wireless_stwlc68 probe ok\n");
	return 0;

ops_regist_fail:
	gpio_free(di->gpio_int);
	free_irq(di->irq_int, di);
irq_init_fail:
	gpio_free(di->gpio_en);
	gpio_free(di->gpio_sleep_en);
	if (di->gpio_pwr_good > 0)
		gpio_free(di->gpio_pwr_good);
gpio_init_fail:
parse_dts_fail:
	di->cp_ops = NULL;
	devm_kfree(&client->dev, di);
	di = NULL;
	g_stwlc68_di = NULL;
	g_wlcp_ops = NULL;
	np = NULL;
	return ret;
}

MODULE_DEVICE_TABLE(i2c, wireless_stwlc68);
static const struct of_device_id stwlc68_of_match[] = {
	{
		.compatible = "huawei, wireless_stwlc68",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id stwlc68_i2c_id[] = {
	{ "wireless_stwlc68", 0 }, {}
};

static struct i2c_driver stwlc68_driver = {
	.probe = stwlc68_probe,
	.shutdown = stwlc68_shutdown,
	.id_table = stwlc68_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "wireless_stwlc68",
		.of_match_table = of_match_ptr(stwlc68_of_match),
	},
};

static int __init stwlc68_init(void)
{
	return i2c_add_driver(&stwlc68_driver);
}

static void __exit stwlc68_exit(void)
{
	i2c_del_driver(&stwlc68_driver);
}

device_initcall(stwlc68_init);
module_exit(stwlc68_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("stwlc68 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
