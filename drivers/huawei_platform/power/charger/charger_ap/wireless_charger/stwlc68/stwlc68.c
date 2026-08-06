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
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/wireless_power_supply.h>
#include <huawei_platform/power/wireless_charger.h>
#include <huawei_platform/power/wireless_transmitter.h>
#include <huawei_platform/power/wired_channel_switch.h>
#include <stwlc68.h>
#include <stwlc68_fw_sram.h>
#include <stwlc68_fw_otp.h>

#define HWLOG_TAG wireless_stwlc68
HWLOG_REGIST();

static struct stwlc68_dev_info *g_stwlc68_di;
static struct wake_lock g_stwlc68_wakelock;
static int stop_charging_flag;
static int irq_abnormal_flag;
static const u8 stwlc68_send_msg_len[STWLC68_RX2TX_PKT_LEN] = {
	0, 0x18, 0x28, 0x38, 0x48, 0x58
};

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

static int stwlc68_sc_write_byte(u8 reg, u8 data)
{
	int ret;

	ret = stwlc68_write_word(STWLC68_MI2C_REG_ADDR, reg);
	ret |= stwlc68_write_byte(STWLC68_MI2C_SLAVE_ADDR,
			STWLC68_MI2C_SLAVE_ADDR_VAL);
	ret |= stwlc68_write_byte(STWLC68_MI2C_REG_LEN, STWLC68_MI2C_REG_1BYTE);
	ret |= stwlc68_write_byte(STWLC68_MI2C_DATA_LEN, BYTE_LEN);
	ret |= stwlc68_write_byte(STWLC68_MI2C_DATA, data);
	if (ret || stop_charging_flag)
		return -WLC_ERR_STOP_CHRG;
	msleep(STWLC68_MI2C_WRD_DELAY_TIME);
	ret |= stwlc68_write_word(STWLC68_MI2C_CMD_ADDR, STWLC68_MI2C_CMD_WR);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}
	if (stop_charging_flag)
		return -WLC_ERR_STOP_CHRG;

	msleep(STWLC68_MI2C_WRD_DELAY_TIME);

	hwlog_info("[%s] reg[0x%x] write 0x%x success\n", __func__, reg, data);
	return 0;
}

static int stwlc68_clear_interrupt(u16 intr)
{
	int ret;

	ret = stwlc68_write_word(STWLC68_INTR_CLR_ADDR, intr);
	if (ret) {
		hwlog_err("%s: write clr_intr failed\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static int stwlc68_cp_open(void)
{
	int ret;

	hwlog_info("[%s] ++\n", __func__);
	/* frq cfg */
	ret = stwlc68_write_word(STWLC68_MI2C_FRQ_ADDR, STWLC68_MI2C_FRQ_VAL);
	/* i2c init */
	ret |= stwlc68_sc_write_byte(CP_I2C_INIT_ADDR, CP_I2C_INIT_CLR);
	ret |= stwlc68_sc_write_byte(CP_I2C_INIT_ADDR, CP_I2C_INIT_RST);
	/* reg init */
	ret |= stwlc68_sc_write_byte(CP_PMID_VLIMIT_ADDR, CP_PMID_VLIMIT_VAL);
	ret |= stwlc68_sc_write_byte(CP_ILIMIT_ADDR, CP_ILIMIT_VAL);
	ret |= stwlc68_sc_write_byte(CP_FRQ_ADDR, CP_FRQ_VAL);
	ret |= stwlc68_sc_write_byte(CP_PMID_OV_ADDR, CP_PMID_OV_VAL);
	msleep(100); /* delay 100ms to clr irq */
	ret |= stwlc68_clear_interrupt(STWLC68_ALL_INTR_CLR);
	hwlog_info("[%s] --\n", __func__);
	return ret;
}

static void stwlc68_wake_lock(void)
{
	if (!wake_lock_active(&g_stwlc68_wakelock)) {
		wake_lock(&g_stwlc68_wakelock);
		hwlog_info("%s\n", __func__);
	}
}

static void stwlc68_wake_unlock(void)
{
	if (wake_lock_active(&g_stwlc68_wakelock)) {
		wake_unlock(&g_stwlc68_wakelock);
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
	return 0;
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
		hwlog_err("%s: get cut_id failed\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	if (cut_id == STWLC68_CUT_ID_V10)
		*clean_addr_pointer = STWLC68_CLEAN_ADDR_REV0;
	else if (cut_id == STWLC68_CUT_ID_V11)
		*clean_addr_pointer = STWLC68_CLEAN_ADDR_REV1;
	else
		return -WLC_ERR_MISMATCH;

	return 0;
}

static int stwlc68_get_clean_addr(u32 clean_addr_addr, u32 *clean_addr)
{
	int ret;
	u8 rd_buff[STWLC68_4ADDR_LEN] = { 0 };

	ret = stwlc68_4addr_read_block(clean_addr_addr,
		rd_buff, STWLC68_4ADDR_LEN);
	if (ret)
		return -WLC_ERR_I2C_R;

	*clean_addr = (u32)((rd_buff[0] << 0) | (rd_buff[1] << 8) |
		(rd_buff[2] << 16) | (rd_buff[3] << 24));
	hwlog_info("[%s] clean addr: 0x%08x\n", __func__, *clean_addr);

	return 0;
}

static u8 *stwlc68_get_chip_info(void)
{
	int ret;
	struct stwlc68_chip_info info;
	u8 read_buff[STWLC68_CHIP_INFO_LEN] = { 0 };
	static u8 chip_info_str[STWLC68_CHIP_INFO_STR_LEN] = { 0 };
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return "error";
	}

	ret = stwlc68_read_block(di, STWLC68_CHIP_INFO_ADDR,
		read_buff, STWLC68_CHIP_INFO_LEN);
	if (ret)
		return "error read";
	/*
	 * addr[0:1]:     chip unique id;
	 * addr[2:2]:     chip revision number;
	 * addr[3:3]:     customer id;
	 * addr[4:5]:     firmware id;
	 * addr[6:7]:     svn revision number;
	 * addr[8:9]:     configuration id;
	 * addr[10:11]:  Pe ID;
	 * addr[12:13]:  patch_id;
	 */
	info.chip_id = (u16)(read_buff[0] | (read_buff[1] << BITS_PER_BYTE));
	info.chip_rev = read_buff[2];
	info.cust_id = read_buff[3];
	info.fw_id = (u16)(read_buff[4] | (read_buff[5] << BITS_PER_BYTE));
	info.svn_rev = (u16)(read_buff[6] | (read_buff[7] << BITS_PER_BYTE));
	info.cfg_id = (u16)(read_buff[8] | (read_buff[9] << BITS_PER_BYTE));
	info.pe_id = (u16)(read_buff[10] | (read_buff[11] << BITS_PER_BYTE));
	info.patch_id = (u16)(read_buff[12] | (read_buff[13] << BITS_PER_BYTE));

	memset(chip_info_str, 0, STWLC68_CHIP_INFO_STR_LEN);

	ret = stwlc68_get_cut_id(&info.cut_id);
	if (ret)
		snprintf(chip_info_str, STWLC68_CHIP_INFO_STR_LEN,
			"chip_id:%d cfg_id:0x%x patch_id:0x%x",
			info.chip_id, info.cfg_id, info.patch_id);

	snprintf(chip_info_str, STWLC68_CHIP_INFO_STR_LEN,
		"chip_id:%d cfg_id:0x%x patch_id:0x%x cut_id:%d",
		info.chip_id, info.cfg_id, info.patch_id, info.cut_id);

	return chip_info_str;
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

static int stwlc68_get_rx_fop(void)
{
	int ret;
	u16 fop = 0;

	ret = stwlc68_read_word(STWLC68_OP_FREQ_ADDR, &fop);
	if (ret) {
		hwlog_err("%s: failed\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	return (int)fop;
}

static int stwlc68_get_rx_vrect(void)
{
	int ret;
	u16 vrect = 0;

	ret = stwlc68_read_word(STWLC68_VRECT_ADDR, &vrect);
	if (ret) {
		hwlog_err("%s: failed\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	return (int)vrect;
}

static int stwlc68_get_rx_vout(void)
{
	int ret;
	u16 vout = 0;

	ret = stwlc68_read_word(STWLC68_VOUT_ADDR, &vout);
	if (ret) {
		hwlog_err("%s: failed\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	return (int)vout;
}

static int stwlc68_get_rx_iout(void)
{
	int ret;
	u16 iout = 0;

	ret = stwlc68_read_word(STWLC68_IOUT_ADDR, &iout);
	if (ret) {
		hwlog_err("%s: failed\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	return (int)iout;
}

static int stwlc68_get_rx_vout_reg(void)
{
	int ret;
	u16 vreg = 0;

	ret = stwlc68_read_word(STWLC68_RX_VOUT_SET_ADDR, &vreg);
	if (ret) {
		hwlog_err("get stwlc68 rx vout reg failed\n");
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
		hwlog_err("get stwlc68 rx iout	value failed\n");
		return -WLC_ERR_I2C_R;
	}

	return (int)tx_vreg;
}

static int stwlc68_set_rx_vout(int vol)
{
	int ret;

	if (vol < STWLC68_RX_VOUT_MIN || vol > STWLC68_RX_VOUT_MAX) {
		hwlog_err("%s: out of range\n", __func__);
		return -1;
	}

	vol = vol / STWLC68_RX_VOUT_SET_STEP;
	ret = stwlc68_write_word(STWLC68_RX_VOUT_SET_ADDR, (u16)vol);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	return 0;
}

static int stwlc68_set_tx_vout(int vset)
{
	int ret;
	int i;
	int vout;
	int cnt = STWLC68_FC_VOUT_TIMEOUT / STWLC68_FC_VOUT_SLEEP_TIME;

	ret = stwlc68_write_word(STWLC68_FC_VOLT_ADDR, vset);
	if (ret) {
		hwlog_err("%s: set fc reg fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}
	ret = stwlc68_write_word(STWLC68_CMD_ADDR, STWLC68_CMD_SEND_FC);
	if (ret) {
		hwlog_err("%s: send fc cmd fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}
	for (i = 0; i < cnt; i++) {
		msleep(STWLC68_FC_VOUT_SLEEP_TIME);
		vout = stwlc68_get_rx_vout();
		if (vout < 0) {
			hwlog_err("%s: get rx_vout fail\n", __func__);
			return -WLC_ERR_I2C_R;
		}
		if (vout >= vset - STWLC68_FC_VOUT_ERR_LTH &&
		    vout <= vset + STWLC68_FC_VOUT_ERR_UTH)
			return 0;

		if (stop_charging_flag) {
			hwlog_err("%s: already stop charging\n", __func__);
			return -WLC_ERR_STOP_CHRG;
		}
	}

	return -WLC_ERR_I2C_WR;
}

static int stwlc68_send_msg(u8 cmd, u8 *data, int data_len)
{
	int ret;
	u8 msg_len;
	u8 write_data[STWLC68_RX2TX_DATA_LEN] = { 0 };
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_I2C_WR;
	}

	if (data_len > STWLC68_RX2TX_DATA_LEN || data_len < 0) {
		hwlog_err("%s: send data number out of range\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	di->irq_val &= ~STWLC68_RX_STATUS_TX2RX_ACK;
	/* msg_len=cmd_len+data_len  cmd_len=1 */
	msg_len = stwlc68_send_msg_len[data_len + 1];

	ret = stwlc68_write_byte(STWLC68_RX2TX_HEADER_ADDR, msg_len);
	if (ret) {
		hwlog_err("%s: write header failed\n", __func__);
		return -WLC_ERR_I2C_W;
	}
	ret = stwlc68_write_byte(STWLC68_RX2TX_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("%s: write cmd failed\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	if (data && data_len > 0) {
		memcpy(write_data, data, data_len);
		ret = stwlc68_write_block(di, STWLC68_RX2TX_DATA_ADDR,
					   write_data, data_len);
		if (ret) {
			hwlog_err("%s: write RX2TX-reg failed\n", __func__);
			return -WLC_ERR_I2C_W;
		}
	}

	ret = stwlc68_write_byte(STWLC68_CMD_ADDR,
			STWLC68_CMD_SEND_MSG_WAIT_RPLY);
	if (ret) {
		hwlog_err("%s: send RX msg to TX failed\n", __func__);
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

	ret = stwlc68_read_byte(STWLC68_RCVD_CMD_ADDR, &cmd);
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
	u8 buff[STWLC68_RCVD_MSG_DATA_LEN] = { 0 };
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_write_block(di, STWLC68_RCVD_HEADER_ADDR, buff,
			STWLC68_RCVD_MSG_DATA_LEN);
	if (ret)
		hwlog_err("%s: clr data buffer fail\n", __func__);
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
	ret = stwlc68_read_block(di, STWLC68_RCVD_CMD_ADDR, data, data_len);
	if (ret) {
		hwlog_err("%s:get tx to rx data failed\n", __func__);
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
		hwlog_err("%s: unlock OTP failed\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	while (remaining > 0) {
		size_to_write = remaining > STWLC68_OTP_I2C_CHUNK_SIZE ?
			STWLC68_OTP_I2C_CHUNK_SIZE : remaining;
		memcpy(buff, otp_data + written_already, size_to_write);
		ret = stwlc68_4addr_write_block(address, buff, size_to_write);
		if (ret) {
			hwlog_err("%s: addr = 0x%8x\n", __func__, address);
			return -WLC_ERR_I2C_W;
		}
		address += size_to_write;
		written_already += size_to_write;
		remaining -= size_to_write;
		usleep_range(9500, 10500); /* 10ms */
	}

	if (stwlc68_lock_otp()) {
		hwlog_err("%s: lock OTP failed\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static int stwlc68_program_otp(void)
{
	int ret;
	u8 cut_id;
	u16 cfg_id;
	u16 patch_id;
	u32 clean_addr_pointer;
	u32 current_clean_addr;
	u32 updated_clean_addr;
	int config_id_mismatch = 0;
	int patch_id_mismatch = 0;
	u32 data_to_program_size;
	const u8 *data_to_program = NULL;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}
	stwlc68_disable_irq_nosync(di);
	wlps_control(WLPS_PROC_OTP_PWR, WLPS_CTRL_ON);
	msleep(STWLC68_OTP_OPS_DELAY); /* delay for power on */

	ret = stwlc68_check_dc_power();
	if (ret) {
		hwlog_err("%s: OTP must be programmed in DC power", __func__);
		goto exit;
	}
	if (stwlc68_reset_system())
		hwlog_err("%s: reset system failed, ignore ACK\n", __func__);

	msleep(STWLC68_OTP_OPS_DELAY); /* delay for system reset */
	ret = stwlc68_get_cut_id(&cut_id);
	if (ret) {
		hwlog_err("%s: get cut_id failed\n", __func__);
		goto exit;
	}
	hwlog_info("[%s] cut_id id = 0x%x\n", __func__, cut_id);
	/* determine what has to be programmed depending on version ids */
	if (cut_id != st_otp_info[0].target_cut_id) {
		hwlog_err("[%s] cut_id mismatch, running|header:0x%x|0x%x\n",
			__func__, cut_id, st_otp_info[0].target_cut_id);
		ret = WLC_ERR_MISMATCH;
		goto exit;
	}
	ret = stwlc68_get_cfg_id(&cfg_id);
	if (ret) {
		hwlog_err("%s: get cfg_id failed\n", __func__);
		goto exit;
	}
	if (cfg_id != st_otp_info[0].cfg_id) {
		hwlog_err("[%s] cfg_id mismatch, running|header:0x%x|0x%x\n",
			__func__, cfg_id, st_otp_info[0].cfg_id);
		config_id_mismatch = 1;
	}
	ret = stwlc68_get_patch_id(&patch_id);
	if (ret) {
		hwlog_err("%s: get patch_id failed\n", __func__);
		return -WLC_ERR_I2C_R;
	}
	if (patch_id != st_otp_info[0].patch_id) {
		hwlog_err("[%s] patch_id mismatch, running|header:0x%x|0x%x\n",
			__func__, patch_id, st_otp_info[0].patch_id);
		patch_id_mismatch = 1;
	}
	if (!config_id_mismatch && !patch_id_mismatch) {
		hwlog_info("[%s] cfg&&patch are latest\n", __func__);
		goto exit;
	} else if (config_id_mismatch && patch_id_mismatch) {
		data_to_program_size = st_otp_info[0].cfg_size +
					st_otp_info[0].patch_size;
		data_to_program = stwlc68_otp_data;
	} else if (config_id_mismatch && !patch_id_mismatch) {
		data_to_program_size = st_otp_info[0].cfg_size;
		data_to_program = stwlc68_otp_data;
	} else {
		data_to_program_size = st_otp_info[0].patch_size;
		data_to_program = stwlc68_otp_data + st_otp_info[0].cfg_size;
	}
	/* set clean_address pointer depending on hw cut Id */
	ret = stwlc68_get_clean_addr_pointer(&clean_addr_pointer);
	if (ret) {
		hwlog_err("%s: get clean_addr_addr failed\n", __func__);
		goto exit;
	}
	ret = stwlc68_get_clean_addr(clean_addr_pointer, &current_clean_addr);
	if (ret) {
		hwlog_err("%s: get current_clean_addr failed\n", __func__);
		goto exit;
	}
	hwlog_info("[%s] opt_max_size: 0x%04X, opt_size: 0x%04X\n",
		__func__, STWLC68_OTP_MAX_SIZE, data_to_program_size);
	if ((STWLC68_OTP_MAX_SIZE - (current_clean_addr & WORD_MASK)) <
		data_to_program_size) {
		hwlog_err("%s: not enough space available\n", __func__);
		ret = WLC_ERR_NO_SPACE;
		goto exit;
	}
	ret = stwlc68_disable_fw_i2c();
	if (ret) {
		hwlog_err("%s: disable fw_i2c failed\n", __func__);
		goto exit;
	}
	ret = stwlc68_hold_m0();
	if (ret) {
		hwlog_err("%s: hold M0 failed\n", __func__);
		goto exit;
	}
	msleep(STWLC68_OTP_OPS_DELAY);
	ret = stwlc68_reset_otp_block();
	if (ret) {
		hwlog_err("%s: OTP block reset failed\n", __func__);
		goto exit;
	}
	ret = stwlc68_enable_otp();
	if (ret) {
		hwlog_err("%s: enable OTP failed\n", __func__);
		goto exit;
	}
	ret = stwlc68_unlock_otp();
	if (ret) {
		hwlog_err("%s: unlock OTP failed\n", __func__);
		goto exit;
	}
	ret = stwlc68_config_otp_registers();
	if (ret) {
		hwlog_err("%s: config otp registers failed\n", __func__);
		ret = stwlc68_lock_otp();
		if (ret)
			hwlog_err("%s: lock otp fail\n", __func__);
		goto exit;
	}
	ret = stwlc68_lock_otp();
	if (ret) {
		hwlog_err("%s: lock OTP failed\n", __func__);
		goto exit;
	}
	ret = stwlc68_write_otp_data(current_clean_addr,
		data_to_program, data_to_program_size);
	if (ret) {
		hwlog_err("%s: write OTP data to memory failed\n", __func__);
		ret = stwlc68_lock_otp();
		if (ret)
			hwlog_err("%s: lock otp fail\n", __func__);
		goto exit;
	}
	ret = stwlc68_reset_system();
	if (ret)
		hwlog_err("%s: reset system failed, ignore ACK\n", __func__);
	msleep(STWLC68_OTP_OPS_DELAY); /* delay for system reset */
	ret = stwlc68_get_clean_addr(clean_addr_pointer, &updated_clean_addr);
	if (ret) {
		hwlog_err("%s: get clean_addr__ failed\n", __func__);
		goto exit;
	}
	if (updated_clean_addr !=
		(current_clean_addr + data_to_program_size)) {
		hwlog_err("%s: clean addr mismatch after flashing\n", __func__);
		ret = WLC_ERR_MISMATCH;
		goto exit;
	}
	msleep(STWLC68_OTP_OPS_DELAY);
	ret = stwlc68_get_cfg_id(&cfg_id);
	if (ret) {
		hwlog_err("%s: get cfg_id failed\n", __func__);
		goto exit;
	}
	if (config_id_mismatch && (cfg_id != st_otp_info[0].cfg_id))
		hwlog_err("[WLC] Config Id mismatch after OTP programming\n");
	ret = stwlc68_get_patch_id(&patch_id);
	if (ret) {
		hwlog_err("%s: get patch_id failed\n", __func__);
		goto exit;
	}
	if (patch_id_mismatch && (patch_id != st_otp_info[0].patch_id))
		hwlog_err("[WLC] Patch Id mismatch after OTP programming\n");

	ret = stwlc68_lock_otp();
	if (ret)
		hwlog_err("%s: lock otp fail\n", __func__);

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
	u16 cfg_id = 0;
	u16 patch_id = 0;

	wlps_control(WLPS_PROC_OTP_PWR, WLPS_CTRL_ON);
	stwlc68_chip_enable(RX_EN_ENABLE); /* enable RX for i2c WR */
	msleep(STWLC68_OTP_OPS_DELAY); /* delay for power on */
	ret = stwlc68_get_patch_id(&patch_id);
	if (ret) {
		hwlog_err("%s: get patch_id failed\n", __func__);
		goto exit;
	}
	ret = stwlc68_get_cfg_id(&cfg_id);
	if (ret) {
		hwlog_err("%s: get cfg_id failed\n", __func__);
		goto exit;
	}
	if ((patch_id != st_otp_info[0].patch_id) ||
		(cfg_id != st_otp_info[0].cfg_id))
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

static bool stwlc68_check_tx_exist(void)
{
	int ret;
	u8 mode = 0;

	ret = stwlc68_get_mode(&mode);
	if (ret) {
		hwlog_err("%s: get rx mode failed\n", __func__);
		return false;
	}
	if (mode == STWLC68_RX_MODE)
		return true;

	return false;
}

static int stwlc68_send_charge_state(u8 chrg_state)
{
	return 0;
}

static int stwlc68_send_rx_qval(u8 qval)
{
	return 0;
}

static int stwlc68_kick_watchdog(void)
{
	int ret;

	ret = stwlc68_write_byte(STWLC68_WDT_FEED_ADDR, 0);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static char *stwlc68_get_rx_fod_coef(void)
{
	return "error";
}

static int stwlc68_set_rx_fod_coef(char *fod_coef)
{
	return 0;
}

static int stwlc68_fix_tx_fop(int fop)
{
	if (fop < STWLC68_FIXED_FOP_MIN || fop > STWLC68_FIXED_FOP_MAX) {
		hwlog_err("%s fixed fop(%d) exceeds range[%d, %d]\n",
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

static int stwlc68_chip_init(int tx_vset)
{
	int ret = 0;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	switch (tx_vset) {
	case WIRELESS_CHIP_INIT:
		stwlc68_write_word(STWLC68_MI2C_CMD_ADDR,
			STWLC68_MI2C_CMD_HIGH);
		hwlog_info("[%s] DEFAULT_CHIP_INIT\n", __func__);
		break;
	case ADAPTER_5V * WL_MVOLT_PER_VOLT:
		hwlog_info("[%s] 5V_CHIP_INIT\n", __func__);
		break;
	case ADAPTER_9V * WL_MVOLT_PER_VOLT:
		hwlog_info("[%s] 9V_CHIP_INIT\n", __func__);
		break;
	case WILREESS_SC_CHIP_INIT:
		hwlog_info("[%s] SC_CHIP_INIT\n", __func__);
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
	u8 adapter[STWLC68_TX_ADAPTER_TYPE_LEN];

	ret = stwlc68_get_tx_para(STWLC68_CMD_GET_TX_ADAPTER_TYPE,
				   adapter, STWLC68_TX_ADAPTER_TYPE_LEN);
	if (ret) {
		hwlog_err("%s: get tx adapter failed\n", __func__);
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
	u8 tx_cap_data[TX_CAP_TOTAL] = { 0 };

	ret = stwlc68_get_tx_para(STWLC68_CMD_GET_TX_CAP,
				   tx_cap_data, TX_CAP_TOTAL);
	if (ret) {
		hwlog_err("%s: get tx capability failed\n", __func__);
		tx_cap->type = WIRELESS_TYPE_ERR;
		return;
	}
	if (tx_cap_data[0] != STWLC68_CMD_GET_TX_CAP) {
		hwlog_err("%s: cmd 0x%x err\n", __func__, tx_cap_data[0]);
		tx_cap->type = WIRELESS_TYPE_ERR;
		return;
	}
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
	hwlog_info("[%s] type = %d, vout_max = %d, iout_max = %d,\t"
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
		hwlog_err("%s: get tx extra capability failed\n", __func__);
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
	tx_cap->support_qval =
	    tx_extra_cap_data[TX_EXTRA_CAP_ATTR1] &
	    STWLC68_TX_CAP_SUPPORT_QVAL_MASK;
	hwlog_info("[%s] support: fan = %d, tec = %d, qval = %d\n",
		__func__, tx_cap->support_fan, tx_cap->support_tec,
		tx_cap->support_qval);
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
		hwlog_err("%s: get tx info failed\n", __func__);
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

static u8 *stwlc68_get_otp_fw_version(void)
{
	return "stwlc68";
}

static int stwlc68_check_fwupdate(enum wireless_mode fw_sram_mode)
{
	return 0;
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

	do {
		ret = stwlc68_send_msg(STWLC68_CMD_GET_TX_ID, id_para,
					STWLC68_TX_ID_LEN - 1);
		ret |= stwlc68_receive_msg(id_received, STWLC68_TX_ID_LEN);
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

static int stwlc68_send_msg_certification_confirm(bool result)
{
	int ret;

	if (!result)
		ret = stwlc68_send_msg_ack(STWLC68_CMD_CERT_SUCC, NULL, 0);
	else
		ret = stwlc68_send_msg_ack(STWLC68_CMD_CERT_FAIL, NULL, 0);

	if (ret) {
		hwlog_err("%s:send CERT result to TX failed\n", __func__);
		return -WLC_ERR_I2C_WR;
	}
	hwlog_info("[%s] cert_result = %d\n", __func__, result);

	return ret;
}

static int stwlc68_send_msg_rx_boost_succ(void)
{
	int ret;

	ret = stwlc68_send_msg_ack(STWLC68_CMD_RX_BOOST_SUCC, NULL, 0);
	if (ret) {
		hwlog_err("%s:send rx boost succ to TX failed\n", __func__);
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
	u8 data[STWLC68_RX2TX_DATA_LEN + 1] = { 0 };

	if (!random || random_len < WIRELESS_RANDOM_LEN ||
		!cipherkey || key_len < WIRELESS_TX_KEY_LEN) {
		hwlog_err("%s: NULL pointer\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	/*send message  to tx: random number */
	for (i = 0; i < WIRELESS_RANDOM_LEN / STWLC68_RX2TX_DATA_LEN; i++) {
		ret = stwlc68_send_msg_ack(STWLC68_CMD_START_CERT + i,
				random + i * STWLC68_RX2TX_DATA_LEN,
				STWLC68_RX2TX_DATA_LEN);
		if (ret) {
			hwlog_err("%s: send msg to TX fail, after retry\n",
				  __func__);
			return -WLC_ERR_I2C_WR;
		}
		hwlog_info("[%s] send rx %dth random data succ\n",
			__func__, i + 1);
	}

	/*get cipherkey from TX */
	for (i = 0; i < WIRELESS_TX_KEY_LEN / STWLC68_RX2TX_DATA_LEN; i++) {
		ret = stwlc68_get_tx_para(STWLC68_CMD_GET_HASH + i, data,
					   STWLC68_RX2TX_DATA_LEN + 1);
		if (ret) {
			hwlog_err("%s: get tx cipherkey, failed\n", __func__);
			return -WLC_ERR_I2C_WR;
		}
		if (STWLC68_CMD_GET_HASH + i == data[0]) {
			memcpy(cipherkey + i * STWLC68_RX2TX_DATA_LEN,
			       data + 1, STWLC68_RX2TX_DATA_LEN);
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
	u8 buff[STWLC68_RCVD_MSG_DATA_LEN] = { 0 };
	u8 command;
	u8 data_received[STWLC68_RCVD_DATA_LEN] = { 0 };

	/*
	 * TX send message to RX on it's own initiative
	 * to get something it needs. handler the interruption
	 */
	ret = stwlc68_read_block(di, STWLC68_RCVD_CMD_ADDR,
				  &(buff[0]), STWLC68_RCVD_MSG_DATA_LEN);
	if (ret) {
		hwlog_err("%s:read data received from TX failed\n", __func__);
		return -WLC_ERR_I2C_R;
	}
	command = buff[0];
	hwlog_info("[%s] data received from TX, command:0x%x\n",
		   __func__, command);
	for (i = 0; i < STWLC68_RCVD_DATA_LEN; i++) {
		data_received[i] = buff[i + 1];
		hwlog_info("%s:data received from TX, data:0x%x\n",
			   __func__, data_received[i]);
	}

	/* analysis data received */
	switch (command) {
	case STWLC68_CMD_START_SAMPLE:
		di->irq_val &= ~STWLC68_RX_RCVD_MSG_INTR_LATCH;
		hwlog_info("[%s] receive command: start sample\n", __func__);
		blocking_notifier_call_chain(&rx_event_nh,
			WIRELESS_CHARGE_START_SAMPLE, NULL);
		break;
	case STWLC68_CMD_STOP_SAMPLE:
		di->irq_val &= ~STWLC68_RX_RCVD_MSG_INTR_LATCH;
		hwlog_info("[%s] receive command: stop sample\n", __func__);
		blocking_notifier_call_chain(&rx_event_nh,
			WIRELESS_CHARGE_STOP_SAMPLE, NULL);
		break;
	default:
		hwlog_info("[%s] data received from TX, not handler it\n",
			   __func__);
		break;
	}
	return 0;
}

static void stwlc68_rx_ready_handler(struct stwlc68_dev_info *di)
{
	int wired_ch_state;

	wired_ch_state = wireless_charge_get_wired_channel_state();
	if (wired_ch_state != WIRED_CHANNEL_ON) {
		hwlog_info("%s rx ready, goto wireless charging\n", __func__);
		stop_charging_flag = 0;
		di->irq_cnt = 0;
		wired_chsw_set_wired_channel(WIRED_CHANNEL_CUTOFF);
		msleep(CHANNEL_SW_TIME);
		gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
		di->cp_state = STWLC68_CP_OPEN;
		schedule_work(&di->cp_work);
		blocking_notifier_call_chain(&rx_event_nh,
			WIRELESS_CHARGE_RX_READY, NULL);
	}
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

	stwlc68_handle_abnormal_irq(di);
	stwlc68_read_byte(STWLC68_RX_SS_ADDR, &rx_ss);
	if (rx_ss > di->rx_ss_good_lth && rx_ss != STWLC68_RX_SS_MAX)
		power_on_good_flag = true;
	hwlog_info("[%s] Signal_Strength = %u\n", __func__, rx_ss);
	blocking_notifier_call_chain(&rx_event_nh,
		WIRELESS_CHARGE_RX_POWER_ON, &power_on_good_flag);
}

static void stwlc68_tx_mode_irq_handler(struct stwlc68_dev_info *di)
{
	hwlog_info("%s\n", __func__);
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

	/* receice data from TX, please handler the interrupt! */
	if (di->irq_val & STWLC68_RX_RCVD_MSG_INTR_LATCH)
		stwlc68_data_received_handler(di);

clr_irq_again:
	/* clear interrupt again */
	if (!gpio_get_value(di->gpio_int)) {
		stwlc68_read_word(STWLC68_INTR_LATCH_ADDR, &irq_value);
		hwlog_info("[%s] gpio_int low, clear irq again! irq = 0x%x\n",
			   __func__, irq_value);
		stwlc68_clear_interrupt(STWLC68_ALL_INTR_CLR);
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

	/* handler interrupt */
	if ((mode == STWLC68_TX_MODE) || (mode == STWLC68_STANDALONE_MODE))
		stwlc68_tx_mode_irq_handler(di);
	else
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

static void stwlc68_cp_work(struct work_struct *work)
{
	int ret;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	switch (di->cp_state) {
	case STWLC68_CP_OPEN:
		ret = stwlc68_cp_open();
		if (ret)
			hwlog_err("%s: cp_open fail\n", __func__);
		break;
	default:
		hwlog_err("%s: err cp_state(0x%x)\n", __func__, di->cp_state);
		break;
	}
}

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

	return 0;
}

static int stwlc68_gpio_init(struct stwlc68_dev_info *di,
			      struct device_node *np)
{
	int ret;

	di->gpio_en = of_get_named_gpio(np, "gpio_en", 0);
	hwlog_info("[%s] gpio_en = %d\n", __func__, di->gpio_en);
	if (!gpio_is_valid(di->gpio_en)) {
		hwlog_err("%s: gpio_en is not valid\n", __func__);
		ret = -EINVAL;
		goto gpio_init_fail_0;
	}
	ret = gpio_request(di->gpio_en, "stwlc68_en");
	if (ret) {
		hwlog_err("%s: could not request stwlc68_en\n", __func__);
		goto gpio_init_fail_0;
	}
	ret = gpio_direction_output(di->gpio_en, di->gpio_en_valid_val);
	if (ret)
		hwlog_err("%s: gpio_dir_output fail\n", __func__);

	di->gpio_sleep_en = of_get_named_gpio(np, "gpio_sleep_en", 0);
	if (!gpio_is_valid(di->gpio_sleep_en)) {
		hwlog_err("%s: gpio_sleep_en is not valid\n", __func__);
		ret = -EINVAL;
		goto gpio_init_fail_1;
	}
	hwlog_info("[%s] gpio_sleep_en = %d\n", __func__, di->gpio_sleep_en);
	ret = gpio_request(di->gpio_sleep_en, "stwlc68_sleep_en");
	if (ret) {
		hwlog_err("%s: could not request stwlc68_sleep_en\n", __func__);
		goto gpio_init_fail_1;
	}
	ret = gpio_direction_output(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
	if (ret)
		hwlog_err("%s: gpio_dir_output fail\n", __func__);

	return 0;

gpio_init_fail_1:
	gpio_free(di->gpio_en);
gpio_init_fail_0:
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
	.get_tx_id              = stwlc68_get_tx_id,
	.get_rx_vrect           = stwlc68_get_rx_vrect,
	.get_rx_vout            = stwlc68_get_rx_vout,
	.get_rx_iout            = stwlc68_get_rx_iout,
	.get_rx_vout_reg        = stwlc68_get_rx_vout_reg,
	.get_tx_vout_reg        = stwlc68_get_tx_vout_reg,
	.set_tx_vout            = stwlc68_set_tx_vout,
	.set_rx_vout            = stwlc68_set_rx_vout,
	.get_rx_fop             = stwlc68_get_rx_fop,
	.get_tx_adaptor_type    = stwlc68_get_tx_type,
	.get_chip_info          = stwlc68_get_chip_info,
	.get_rx_chip_id         = stwlc68_get_chip_id,
	.get_rx_fw_version      = stwlc68_get_otp_fw_version,
	.get_rx_fod_coef        = stwlc68_get_rx_fod_coef,
	.set_rx_fod_coef        = stwlc68_set_rx_fod_coef,
	.fix_tx_fop             = stwlc68_fix_tx_fop,
	.unfix_tx_fop           = stwlc68_unfix_tx_fop,
	.rx_enable              = stwlc68_chip_enable,
	.rx_sleep_enable        = stwlc68_sleep_enable,
	.check_tx_exist         = stwlc68_check_tx_exist,
	.send_chrg_state        = stwlc68_send_charge_state,
	.send_rx_qval           = stwlc68_send_rx_qval,
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
};

static void stwlc68_shutdown(struct i2c_client *client)
{
	int wired_channel_state;

	hwlog_info("[%s] ++\n", __func__);
	wired_channel_state = wireless_charge_get_wireless_channel_state();
	if (wired_channel_state == WIRELESS_CHANNEL_ON) {
		stwlc68_set_tx_vout(ADAPTER_5V * WL_MVOLT_PER_VOLT);
		stwlc68_set_rx_vout(ADAPTER_5V * WL_MVOLT_PER_VOLT);
		msleep(STWLC68_SHUTDOWN_SLEEP_TIME);
	}
	hwlog_info("[%s] --\n", __func__);
}

static int stwlc68_probe(struct i2c_client *client,
			  const struct i2c_device_id *id)
{
	int ret;
	struct stwlc68_dev_info *di = NULL;
	struct device_node *np = NULL;

	if (!client || !client->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_stwlc68_di = di;
	di->dev = &client->dev;
	np = client->dev.of_node;
	di->client = client;
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

	wake_lock_init(&g_stwlc68_wakelock, WAKE_LOCK_SUSPEND,
		       "stwlc68_wakelock");
	mutex_init(&di->mutex_irq);
	ret = wireless_charge_ops_register(&stwlc68_ops);
	if (ret) {
		hwlog_err("%s: register ops failed\n", __func__);
		goto ops_regist_fail;
	}
	INIT_WORK(&di->cp_work, stwlc68_cp_work);

	if (stwlc68_check_tx_exist()) {
		stwlc68_clear_interrupt(STWLC68_ALL_INTR_CLR);
		hwlog_info("[%s] rx exsit, exe rx_ready_handler\n", __func__);
		stwlc68_rx_ready_handler(di);
	} else {
		gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_ENABLE);
	}

	hwlog_info("wireless_stwlc68 probe ok\n");
	return 0;

ops_regist_fail:
	gpio_free(di->gpio_int);
	free_irq(di->irq_int, di);
irq_init_fail:
	gpio_free(di->gpio_en);
	gpio_free(di->gpio_sleep_en);
gpio_init_fail:
parse_dts_fail:
	devm_kfree(&client->dev, di);
	di = NULL;
	g_stwlc68_di = NULL;
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

fs_initcall_sync(stwlc68_init);
module_exit(stwlc68_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("stwlc68 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
