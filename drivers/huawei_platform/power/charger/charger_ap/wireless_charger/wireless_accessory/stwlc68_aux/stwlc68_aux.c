/*
 * stwlc68_aux.c
 *
 * stwlc68_aux driver
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

#include <linux/bitops.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/wireless_charge/wireless_firmware.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <huawei_platform/power/wireless/wireless_transmitter_aux.h>
#include <stwlc68_aux.h>
#include <stwlc68_aux_fw_otp.h>
#include <stwlc68_aux_fw_sram.h>

#define HWLOG_TAG wireless_stwlc68_aux
HWLOG_REGIST();

static struct stwlc68_aux_dev_info *g_stwlc68_aux_di;
static struct wakeup_source g_stwlc68_aux_wakelock;
static bool g_otp_skip_soak_recovery_flag;
static int g_ram_rom_status = STWLC68_AUX_RAM_ROM_STATUS_UNKNOWN;
static struct wltx_acc_dev *g_wltx_acc;
/* for sram scan */
static int g_sram_bad_addr = STWLC68_AUX_RAM_MAX_SIZE;
static bool g_sram_chk_complete;

static u8 stwlc68_aux_get_fsk_header(int data_len)
{
	struct stwlc68_aux_dev_info *di = g_stwlc68_aux_di;

	if (!di || !di->g_val.qi_hdl || !di->g_val.qi_hdl->get_fsk_hdr) {
		hwlog_err("get_fsk_header: para null\n");
		return 0;
	}

	return di->g_val.qi_hdl->get_fsk_hdr(data_len);
}

static bool stwlc68_aux_is_i2c_addr_valid(u16 addr)
{
	if (!g_sram_chk_complete) {
		if ((addr < STWLC68_AUX_RAM_FW_START_ADDR +
			STWLC68_AUX_RAM_MAX_SIZE) &&
			(addr >= STWLC68_AUX_RAM_FW_START_ADDR))
			return true;
		return false;
	}

	return true;
}

static int stwlc68_aux_i2c_read(struct i2c_client *client, u8 *cmd,
	int cmd_len, u8 *buf, int buf_len)
{
	int i;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!power_i2c_read_block(client, cmd, cmd_len, buf, buf_len))
			return 0;
		(void)power_usleep(DT_USLEEP_10MS);
	}

	return -WLC_ERR_I2C_R;
}

static int stwlc68_aux_i2c_write(struct i2c_client *client,
	u8 *buf, int buf_len)
{
	int i;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!power_i2c_write_block(client, buf, buf_len))
			return 0;
		(void)power_usleep(DT_USLEEP_10MS);
	}

	return -WLC_ERR_I2C_W;
}

static int stwlc68_aux_read_block(struct stwlc68_aux_dev_info *di,
	u16 reg, u8 *data, u8 len)
{
	u8 cmd[STWLC68_AUX_ADDR_LEN];

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	if (!stwlc68_aux_is_i2c_addr_valid(reg)) {
		hwlog_err("%s: stwlc68_aux_is_i2c_addr_valid\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;

	return stwlc68_aux_i2c_read(di->client, cmd,
		STWLC68_AUX_ADDR_LEN, data, len);
}

static int stwlc68_aux_write_block(struct stwlc68_aux_dev_info *di,
	u16 reg, u8 *data, u8 data_len)
{
	u8 cmd[STWLC68_AUX_ADDR_LEN + data_len];

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	if (!stwlc68_aux_is_i2c_addr_valid(reg))
		return -WLC_ERR_I2C_W;

	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;
	memcpy(&cmd[STWLC68_AUX_ADDR_LEN], data, data_len);

	return stwlc68_aux_i2c_write(di->client, cmd,
		STWLC68_AUX_ADDR_LEN + data_len);
}

static int stwlc68_aux_4addr_read_block(u32 addr, u8 *data, u8 len)
{
	u8 cmd[STWLC68_AUX_4ADDR_F_LEN];
	struct stwlc68_aux_dev_info *di = g_stwlc68_aux_di;

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	 /* bit[0]: flag 0xFA, bit[1:4]: addr */
	cmd[0] = STWLC68_AUX_4ADDR_FLAG;
	cmd[1] = (u8)((addr >> 24) & BYTE_MASK);
	cmd[2] = (u8)((addr >> 16) & BYTE_MASK);
	cmd[3] = (u8)((addr >> 8) & BYTE_MASK);
	cmd[4] = (u8)((addr >> 0) & BYTE_MASK);

	return stwlc68_aux_i2c_read(di->client,
		cmd, STWLC68_AUX_4ADDR_F_LEN, data, len);
}

static int stwlc68_aux_4addr_write_block(u32 addr, u8 *data, u8 data_len)
{
	u8 cmd[STWLC68_AUX_4ADDR_F_LEN + data_len];
	struct stwlc68_aux_dev_info *di = g_stwlc68_aux_di;

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	 /* bit[0]: flag 0xFA, bit[1:4]: addr */
	cmd[0] = STWLC68_AUX_4ADDR_FLAG;
	cmd[1] = (u8)((addr >> 24) & BYTE_MASK);
	cmd[2] = (u8)((addr >> 16) & BYTE_MASK);
	cmd[3] = (u8)((addr >> 8) & BYTE_MASK);
	cmd[4] = (u8)((addr >> 0) & BYTE_MASK);
	memcpy(&cmd[STWLC68_AUX_4ADDR_F_LEN], data, data_len);

	return stwlc68_aux_i2c_write(di->client,
		cmd, STWLC68_AUX_4ADDR_F_LEN + data_len);
}

static int stwlc68_aux_read_byte(u16 reg, u8 *data)
{
	struct stwlc68_aux_dev_info *di = g_stwlc68_aux_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	return stwlc68_aux_read_block(di, reg, data, BYTE_LEN);
}

static int stwlc68_aux_read_word(u16 reg, u16 *data)
{
	int ret;
	u8 buff[WORD_LEN] = { 0 };
	struct stwlc68_aux_dev_info *di = g_stwlc68_aux_di;

	if (!di || !data) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_aux_read_block(di, reg, buff, WORD_LEN);
	if (ret)
		return -WLC_ERR_I2C_R;

	*data = buff[0] | (buff[1] << BITS_PER_BYTE);
	return 0;
}

static int stwlc68_aux_write_byte(u16 reg, u8 data)
{
	struct stwlc68_aux_dev_info *di = g_stwlc68_aux_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	return stwlc68_aux_write_block(di, reg, &data, BYTE_LEN);
}

static int stwlc68_aux_write_word(u16 reg, u16 data)
{
	u8 buff[WORD_LEN];
	struct stwlc68_aux_dev_info *di = g_stwlc68_aux_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	buff[0] = data & BYTE_MASK;
	buff[1] = data >> BITS_PER_BYTE;

	return stwlc68_aux_write_block(di, reg, buff, WORD_LEN);
}

static int stwlc68_aux_write_word_mask(u16 reg, u16 mask, u16 shift, u16 data)
{
	int ret;
	u16 val = 0;

	ret = stwlc68_aux_read_word(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return stwlc68_aux_write_word(reg, val);
}

static void stwlc68_aux_wake_lock(void)
{
	if (!g_stwlc68_aux_wakelock.active) {
		__pm_stay_awake(&g_stwlc68_aux_wakelock);
		hwlog_info("%s\n", __func__);
	}
}

static void stwlc68_aux_wake_unlock(void)
{
	if (g_stwlc68_aux_wakelock.active) {
		__pm_relax(&g_stwlc68_aux_wakelock);
		hwlog_info("%s\n", __func__);
	}
}

static void stwlc68_aux_enable_irq(struct stwlc68_aux_dev_info *di)
{
	hwlog_info("[%s] ++\n", __func__);
	mutex_lock(&di->mutex_irq);
	if (!di->irq_active) {
		enable_irq(di->irq_int);
		di->irq_active = 1;
	}
	mutex_unlock(&di->mutex_irq);
	hwlog_info("[%s] --\n", __func__);
}

static void stwlc68_aux_disable_irq_nosync(struct stwlc68_aux_dev_info *di)
{
	hwlog_info("[%s] ++\n", __func__);
	mutex_lock(&di->mutex_irq);
	if (di->irq_active) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = 0;
	}
	mutex_unlock(&di->mutex_irq);
	hwlog_info("[%s] --\n", __func__);
}

static void stwlc68_aux_chip_enable(int enable)
{
	int gpio_val;
	struct stwlc68_aux_dev_info *di = g_stwlc68_aux_di;

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

static int stwlc68_aux_chip_reset(void)
{
	int ret;
	u8 data = STWLC68_AUX_CHIP_RESET;

	ret = stwlc68_aux_4addr_write_block(STWLC68_AUX_CHIP_RESET_ADDR, &data,
		STWLC68_AUX_FW_ADDR_LEN);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	hwlog_info("[%s] succ\n", __func__);
	return 0;
}

static int stwlc68_aux_get_chip_id(u16 *chip_id)
{
	return stwlc68_aux_read_word(STWLC68_AUX_CHIP_ID_ADDR, chip_id);
}

static int stwlc68_aux_get_cfg_id(u16 *cfg_id)
{
	return stwlc68_aux_read_word(STWLC68_AUX_CFG_ID_ADDR, cfg_id);
}

static int stwlc68_aux_get_patch_id(u16 *patch_id)
{
	return stwlc68_aux_read_word(STWLC68_AUX_PATCH_ID_ADDR, patch_id);
}

static int stwlc68_aux_get_cut_id(u8 *cut_id)
{
	return stwlc68_aux_4addr_read_block(STWLC68_AUX_CUT_ID_ADDR,
		cut_id, STWLC68_AUX_FW_ADDR_LEN);
}

static int stwlc68_aux_get_clean_addr_pointer(u32 *clean_addr_pointer)
{
	u8 cut_id = 0;

	if (!clean_addr_pointer)
		return -WLC_ERR_PARA_NULL;

	if (stwlc68_aux_get_cut_id(&cut_id)) {
		hwlog_err("%s: get cut_id fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	if (cut_id == STWLC68_AUX_CUT_ID_V10)
		*clean_addr_pointer = STWLC68_AUX_CLEAN_ADDR_REV0;
	else if (cut_id >= STWLC68_AUX_CUT_ID_V11)
		*clean_addr_pointer = STWLC68_AUX_CLEAN_ADDR_REV1;
	else
		return -WLC_ERR_MISMATCH;

	return 0;
}

static void stwlc68_aux_get_clean_addr(u32 *clean_addr)
{
	int ret;
	u32 clean_addr_pointer = 0;
	u8 rd_buff[STWLC68_AUX_4ADDR_LEN] = { 0 };

	ret = stwlc68_aux_get_clean_addr_pointer(&clean_addr_pointer);
	if (ret) {
		hwlog_err("%s: get clean_addr_addr fail\n", __func__);
		return;
	}

	ret = stwlc68_aux_4addr_read_block(clean_addr_pointer,
		rd_buff, STWLC68_AUX_4ADDR_LEN);
	if (ret)
		return;

	/* clean_addr obtained from rd_buff[0:3] in little endian */
	*clean_addr = (u32)((rd_buff[0] << 0) | (rd_buff[1] << 8) |
		(rd_buff[2] << 16) | (rd_buff[3] << 24));
	hwlog_info("[%s] clean addr: 0x%08x\n", __func__, *clean_addr);
}

static int stwlc68_aux_update_clean_addr(u32 clean_addr)
{
	int ret;

	ret = stwlc68_aux_write_word(STWLC68_AUX_FW_CLEAN_ADDR_ADDR,
		(u16)(clean_addr & WORD_MASK));
	if (ret)
		return ret;

	return stwlc68_aux_write_byte(STWLC68_AUX_OTP_WR_CMD_ADDR,
		STWLC68_AUX_CLEAN_ADDR_UPDATE_VAL);
}

static int stwlc68_aux_get_chip_info(struct stwlc68_aux_chip_info *info)
{
	int ret;
	u8 chip_info[STWLC68_AUX_CHIP_INFO_LEN] = { 0 };
	struct stwlc68_aux_dev_info *di = g_stwlc68_aux_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_aux_read_block(di, STWLC68_AUX_CHIP_INFO_ADDR,
		chip_info, STWLC68_AUX_CHIP_INFO_LEN);
	if (ret)
		return ret;

	/*
	 * addr[0:1]:     chip unique id;
	 * addr[2:2]:     chip revision number;
	 * addr[3:3]:     customer id;
	 * addr[4:5]:     sram id;
	 * addr[6:7]:     svn revision number;
	 * addr[8:9]:     configuration id;
	 * addr[10:11]:   pe id;
	 * addr[12:13]:   patch id;
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

	return stwlc68_aux_get_cut_id(&info->cut_id);
}

static int stwlc68_aux_get_chip_info_str(char *info_str, int len)
{
	int ret;
	struct stwlc68_aux_chip_info chip_info;

	if (!info_str || (len != STWLC68_AUX_CHIP_INFO_STR_LEN))
		return -WLC_ERR_PARA_WRONG;

	ret = stwlc68_aux_get_chip_info(&chip_info);
	if (ret)
		return ret;

	memset(info_str, 0, len);
	snprintf(info_str, len,
		"chip_id:%u cfg_id:0x%x patch_id:0x%x cut_id:%u sram_id:0x%x",
		chip_info.chip_id, chip_info.cfg_id, chip_info.patch_id,
		chip_info.cut_id, chip_info.sram_id);
	hwlog_info("[aux_chip_info] %s\n", info_str);

	return 0;
}

static int stwlc68_aux_get_mode(u8 *mode)
{
	return stwlc68_aux_read_byte(STWLC68_AUX_OP_MODE_ADDR, mode);
}

static int stwlc68_aux_disable_iload(void)
{
	u8 wr_buff = STWLC68_AUX_ILOAD_DISABLE_VALUE;

	return stwlc68_aux_4addr_write_block(STWLC68_AUX_ILOAD_DRIVE_ADDR,
		&wr_buff, STWLC68_AUX_ILOAD_DATA_LEN);
}

static int stwlc68_aux_get_iload(void)
{
	int ret;
	u8 iload_value = 0;

	ret = stwlc68_aux_4addr_read_block(STWLC68_AUX_ILOAD_DRIVE_ADDR,
		&iload_value, STWLC68_AUX_ILOAD_DATA_LEN);
	if (ret)
		return ret;
	hwlog_info("%s: iload_value=0x%x\n", __func__, iload_value);

	return 0;
}

static int stwlc68_aux_check_dc_power(void)
{
	int ret;
	u8 op_mode = 0;

	ret = stwlc68_aux_read_byte(STWLC68_AUX_OP_MODE_ADDR, &op_mode);
	if (ret)
		return -WLC_ERR_I2C_R;

	hwlog_info("%s: Op mode %02x\n", __func__, op_mode);
	if (op_mode != STWLC68_AUX_FW_OP_MODE_SA)
		return -WLC_ERR_MISMATCH;

	return 0;
}

static void stwlc68_aux_reset_system(void)
{
	int ret;
	u8 wr_buff = STWLC68_AUX_RST_SYS | STWLC68_AUX_RST_M0;

	ret = stwlc68_aux_4addr_write_block(STWLC68_AUX_RST_ADDR,
		&wr_buff, STWLC68_AUX_FW_ADDR_LEN);
	if (ret)
		hwlog_info("[%s] ignore i2c error\n", __func__);
}

static void stwlc68_aux_soft_reset_system(void)
{
	int ret;
	u8 wr_buff = STWLC68_AUX_RST_SYS;

	ret = stwlc68_aux_4addr_write_block(STWLC68_AUX_RST_ADDR,
		&wr_buff, STWLC68_AUX_FW_ADDR_LEN);
	if (ret)
		hwlog_info("[%s] ignore i2c error\n", __func__);
}

static int stwlc68_aux_disable_fw_i2c(void)
{
	u8 wr_buff = STWLC68_AUX_FW_I2C_DISABLE;

	return stwlc68_aux_4addr_write_block(STWLC68_AUX_FW_I2C_ADDR,
		&wr_buff, STWLC68_AUX_FW_ADDR_LEN);
}

static int stwlc68_aux_hold_m0(void)
{
	u8 wr_buff = STWLC68_AUX_RST_M0;

	return stwlc68_aux_4addr_write_block(STWLC68_AUX_RST_ADDR,
		&wr_buff, STWLC68_AUX_FW_ADDR_LEN);
}

static int stwlc68_aux_osc_trim(void)
{
	u8 wr_buff = STWLC68_AUX_OSC_TRIM_VAL;

	return stwlc68_aux_4addr_write_block(STWLC68_AUX_SYSTEM_OSC_ADDR,
		&wr_buff, STWLC68_AUX_FW_ADDR_LEN);
}

static int stwlc68_aux_clk_div(void)
{
	u8 wr_buff = STWLC68_AUX_CLK_DIV_VAL;

	return stwlc68_aux_4addr_write_block(STWLC68_AUX_SYSTEM_CLK_DIV_ADDR,
		&wr_buff, STWLC68_AUX_FW_ADDR_LEN);
}

static int stwlc68_aux_reset_otp_block(void)
{
	u8 rd_buff = 0;
	u8 wr_buff;
	u8 reset_reg;
	int ret;

	ret = stwlc68_aux_4addr_read_block(STWLC68_AUX_RST_ADDR,
		&rd_buff, STWLC68_AUX_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_R;

	reset_reg = rd_buff;
	/* 4 bit */
	reset_reg = reset_reg | (1 << 4);
	wr_buff = reset_reg;

	ret = stwlc68_aux_4addr_write_block(STWLC68_AUX_RST_ADDR,
		&wr_buff, STWLC68_AUX_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_W;

	/* 4 bit */
	reset_reg = reset_reg & ~(1 << 4);
	wr_buff = reset_reg;

	return stwlc68_aux_4addr_write_block(STWLC68_AUX_RST_ADDR,
		&wr_buff, STWLC68_AUX_FW_ADDR_LEN);
}

static int stwlc68_aux_enable_otp(void)
{
	u8 wr_buff = STWLC68_AUX_OTP_ENABLE;

	return stwlc68_aux_4addr_write_block(STWLC68_AUX_OTP_ENABLE_ADDR,
		&wr_buff, STWLC68_AUX_FW_ADDR_LEN);
}

static int stwlc68_aux_unlock_otp(void)
{
	u8 wr_buff = STWLC68_AUX_OTP_UNLOCK_CODE;

	return stwlc68_aux_4addr_write_block(STWLC68_AUX_OTP_PROGRAM_ADDR,
		&wr_buff, STWLC68_AUX_FW_ADDR_LEN);
}

static bool stwlc68_aux_is_otp_err(void)
{
	int ret;
	u8 err_status = 0;

	ret = stwlc68_aux_4addr_read_block(STWLC68_AUX_OTP_ERR_STATUS_ADDR,
		&err_status, STWLC68_AUX_4ADDR_LEN);
	if (ret)
		return true;

	if (err_status) {
		hwlog_err("%s: status=0x%02x\n", __func__, err_status);
		if (g_otp_skip_soak_recovery_flag &&
			(err_status == 0x08)) /* 0x08:user compare error */
			return false;
		return true;
	}

	return false;
}

static bool stwlc68_aux_is_otp_corrupt(void)
{
	int ret;
	u8 err_status = 0;

	ret = stwlc68_aux_4addr_read_block(STWLC68_AUX_OTP_ERR_STATUS_ADDR,
		&err_status, STWLC68_AUX_4ADDR_LEN);
	if (ret)
		return true;

	if (err_status & STWLC68_AUX_OTP_ERR_CORRUPT) {
		hwlog_err("%s: status=0x%02x\n", __func__, err_status);
		return true;
	}

	return false;
}

static int stwlc68_aux_config_otp_registers(void)
{
	int ret;
	u32 addr;
	u8 write_buff;

	addr = STWLC68_AUX_PGM_IPS_MRR_HI_ADDR;
	write_buff = STWLC68_AUX_PGM_IPS_MRR_HI_VAL;
	ret = stwlc68_aux_4addr_write_block(addr, &write_buff,
		STWLC68_AUX_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_W;

	addr = STWLC68_AUX_RD_VERIFY1_MREF_HI_ADDR;
	write_buff = STWLC68_AUX_RD_VERIFY1_MREF_VAL;
	ret = stwlc68_aux_4addr_write_block(addr,
		&write_buff, STWLC68_AUX_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_W;

	addr = STWLC68_AUX_RD_VERIFY_OTP_MREF_ADDR;
	write_buff = STWLC68_AUX_RD_VERIFY2_OTP_HI_VAL;
	ret = stwlc68_aux_4addr_write_block(addr,
		&write_buff, STWLC68_AUX_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_W;

	addr = STWLC68_AUX_OTP_FREQ_CTRL_ADDR;
	write_buff = STWLC68_AUX_OTP_FREQ_CTRL_VAL;
	ret = stwlc68_aux_4addr_write_block(addr, &write_buff,
		STWLC68_AUX_FW_ADDR_LEN);
	if (ret)
		return WLC_ERR_I2C_W;

	return 0;
}

static int stwlc68_aux_recover_otp_data(u32 current_clean_addr,
	const u8 *otp_data, int otp_size, u8 *dirty_otp_id, int dirty_otp_size)
{
	int id = 0;
	int ret;
	int remaining = otp_size;
	int size_to_wr;
	u32 wr_already = 0;
	u32 addr = current_clean_addr;
	u8 buff[STWLC68_AUX_OTP_RECOVER_WR_SIZE] = { 0 };

	while ((remaining > 0) && (id < dirty_otp_size)) {
		size_to_wr = remaining > STWLC68_AUX_OTP_RECOVER_WR_SIZE ?
			STWLC68_AUX_OTP_RECOVER_WR_SIZE : remaining;
		if (dirty_otp_id[id]) {
			memcpy(buff, otp_data + wr_already, size_to_wr);
			ret = stwlc68_aux_4addr_write_block(addr, buff,
				size_to_wr);
			if (ret) {
				hwlog_err("%s: fail, addr=0x%08x\n",
					__func__, addr);
				return ret;
			}
			(void)power_usleep(DT_USLEEP_10MS);
			if (!stwlc68_aux_is_otp_err())
				dirty_otp_id[id] = 0;
		}
		id++;
		addr += size_to_wr;
		wr_already += size_to_wr;
		remaining -= size_to_wr;
	}

	for (id = 0; id < otp_size / STWLC68_AUX_OTP_RECOVER_WR_SIZE; id++) {
		if ((id >= dirty_otp_size) || dirty_otp_id[id])
			return -WLC_ERR_CHECK_FAIL;
	}

	return 0;
}

static int stwlc68_aux_show_ram_fw_version(void)
{
	int ret;
	u16 fw_ver = 0;

	ret = stwlc68_aux_read_word(STWLC68_AUX_RAM_FW_VER_ADDR, &fw_ver);
	if (ret)
		return ret;

	hwlog_info("[%s] 0x%04x\n", __func__, fw_ver);
	return 0;
}

static int stwlc68_aux_set_boot_mode(u8 boot_mode)
{
	int ret;

	ret = stwlc68_aux_4addr_write_block(STWLC68_AUX_BOOT_SET_ADDR,
		&boot_mode, STWLC68_AUX_FW_ADDR_LEN);
	if (ret)
		return ret;

	stwlc68_aux_reset_system();

	return 0;
}

static int stwlc68_aux_check_fw_wr(u32 wr_already,
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
			if (bad_addr < STWLC68_AUX_RAM_FW_CODE_OFFSET)
				continue;
			if (g_sram_bad_addr >= bad_addr)
				g_sram_bad_addr = bad_addr;
			return -WLC_ERR_CHECK_FAIL;
		}
	}

	return 0;
}

static int stwlc68_aux_write_fw_data(u32 current_clean_addr,
	const u8 *otp_data, int otp_size)
{
	int ret;
	int remaining = otp_size;
	int size_to_wr;
	u32 wr_already = 0;
	u32 addr = current_clean_addr;
	u8 wr_buff[STWLC68_AUX_OTP_PROGRAM_WR_SIZE] = { 0 };
	u8 rd_buff[STWLC68_AUX_OTP_PROGRAM_WR_SIZE] = { 0 };

	while (remaining > 0) {
		size_to_wr = remaining > STWLC68_AUX_OTP_PROGRAM_WR_SIZE ?
			STWLC68_AUX_OTP_PROGRAM_WR_SIZE : remaining;
		memcpy(wr_buff, otp_data + wr_already, size_to_wr);
		ret = stwlc68_aux_4addr_write_block(addr, wr_buff, size_to_wr);
		if (ret) {
			hwlog_err("%s: wr fail, addr=0x%08x\n", __func__, addr);
			return ret;
		}
		(void)power_usleep(DT_USLEEP_1MS);
		ret = stwlc68_aux_4addr_read_block(addr, rd_buff, size_to_wr);
		if (ret) {
			hwlog_err("%s: rd fail, addr=0x%08x\n", __func__, addr);
			return ret;
		}
		ret = stwlc68_aux_check_fw_wr(wr_already, wr_buff,
			rd_buff, size_to_wr);
		if (ret)
			break;
		addr += size_to_wr;
		wr_already += size_to_wr;
		remaining -= size_to_wr;
	}

	return 0;
}

static int stwlc68_aux_load_ram_fw(const u8 *ram, int len)
{
	int ret;

	ret = stwlc68_aux_disable_fw_i2c();
	if (ret) {
		hwlog_err("%s: disable fw_i2c fail\n", __func__);
		return ret;
	}
	ret = stwlc68_aux_hold_m0();
	if (ret) {
		hwlog_err("%s: hold M0 fail\n", __func__);
		return ret;
	}

	ret = stwlc68_aux_write_fw_data(STWLC68_AUX_RAM_FW_START_ADDR,
		ram, len);
	if (ret) {
		hwlog_err("%s: write fw data fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_aux_write_otp_len(u16 len)
{
	hwlog_info("[%s] len = %u bytes\n", __func__, len);

	return stwlc68_aux_write_word(STWLC68_AUX_OTP_WRITE_LENGTH_ADDR, len);
}

static int stwlc68_aux_exe_wr_otp_cmd(void)
{
	return stwlc68_aux_write_byte(STWLC68_AUX_OTP_WR_CMD_ADDR,
		STWLC68_AUX_OTP_ENABLE);
}

static bool stwlc68_aux_is_otp_successfully_written(void)
{
	int i;
	int ret;
	struct stwlc68_aux_dev_info *di = g_stwlc68_aux_di;

	if (!di)
		return false;

	/* timeout: 500ms * 5 = 2.5s */
	for (i = 0; i < 5; i++) {
		(void)power_msleep(DT_MSLEEP_500MS, 0, NULL);
		if (!gpio_get_value(di->gpio_int)) {
			ret = stwlc68_aux_write_byte(
				STWLC68_AUX_GPIO_STATUS_ADDR,
				STWLC68_AUX_GPIO_RESET_VAL);
			if (ret) {
				hwlog_err("%s: reset gpio fail\n", __func__);
				return false;
			}
			hwlog_info("[%s] succ, cnt=%d\n", __func__, i);
			return true;
		}
	}

	return false;
}

static int stwlc68_aux_write_otp_data(const u8 *otp_data, int otp_size)
{
	int ret;
	int remaining = otp_size;
	int size_to_wr;
	u32 wr_already = 0;
	u8 *wr_buff = NULL;
	struct stwlc68_aux_dev_info *di = g_stwlc68_aux_di;

	if (!di)
		return -WLC_ERR_PARA_NULL;

	wr_buff = kzalloc(sizeof(u8) * STWLC68_AUX_OTP_WR_BLOCK_SIZE,
		GFP_KERNEL);
	if (!wr_buff)
		return -WLC_ERR_NO_SPACE;

	while (remaining > 0) {
		hwlog_info("[%s] gpio_int val = %d\n", __func__,
			gpio_get_value(di->gpio_int));
		size_to_wr = remaining > STWLC68_AUX_OTP_WR_BLOCK_SIZE ?
			STWLC68_AUX_OTP_WR_BLOCK_SIZE : remaining;
		ret = stwlc68_aux_write_otp_len(size_to_wr);
		if (ret) {
			hwlog_err("%s: wr otp len fail\n", __func__);
			goto write_fail;
		}
		memcpy(wr_buff, otp_data + wr_already, size_to_wr);
		ret = stwlc68_aux_write_fw_data(STWLC68_AUX_OTP_FW_START_ADDR,
			wr_buff, size_to_wr);
		if (ret) {
			hwlog_err("%s: wr otp data fail\n", __func__);
			goto write_fail;
		}
		ret = stwlc68_aux_exe_wr_otp_cmd();
		if (ret) {
			hwlog_err("%s: wr otp data fail\n", __func__);
			goto write_fail;
		}
		wr_already += size_to_wr;
		remaining -= size_to_wr;
		if (!stwlc68_aux_is_otp_successfully_written())
			goto write_fail;
	}

	kfree(wr_buff);
	return 0;

write_fail:
	kfree(wr_buff);
	return -WLC_ERR_I2C_WR;
}

static int stwlc68_aux_check_system_err(void)
{
	int ret;
	u8 err = 0;

	ret = stwlc68_aux_read_byte(STWLC68_AUX_SYSTEM_ERR_ADDR, &err);
	if (ret)
		return ret;

	if (err) {
		hwlog_err("%s: err=0x%x\n", __func__, err);
		return -WLC_ERR_CHECK_FAIL;
	}

	return 0;
}

static int stwlc68_aux_fw_ram_check(void)
{
	int ret;
	u8 *ram = NULL;
	int size = sizeof(u8) * STWLC68_AUX_RAM_MAX_SIZE;

	ram = kzalloc(size, GFP_KERNEL);
	if (!ram)
		return -WLC_ERR_NO_SPACE;

	stwlc68_aux_reset_system();
	(void)power_msleep(STWLC68_AUX_OTP_OPS_DELAY, 0, NULL);

	memset(ram, STWLC68_AUX_RAM_CHECK_VAL1, size);
	ret = stwlc68_aux_load_ram_fw(ram, size);
	if (ret) {
		kfree(ram);
		return ret;
	}
	memset(ram, STWLC68_AUX_RAM_CHECK_VAL2, size);
	ret = stwlc68_aux_load_ram_fw(ram, size);
	if (ret) {
		kfree(ram);
		return ret;
	}

	kfree(ram);
	return 0;
}

static bool stwlc68_aux_is_rom_ok(void)
{
	int i;
	int ret;
	u8 rd_buff = 0;

	ret = stwlc68_aux_write_byte(STWLC68_AUX_OTP_WR_CMD_ADDR,
		STWLC68_AUX_ROM_CHECK_VAL);
	if (ret)
		return ret;

	/* timeout: 25ms * 40 = 1s */
	for (i = 0; i < 40; i++) {
		ret = stwlc68_aux_4addr_read_block(STWLC68_AUX_ROM_CHECK_ADDR,
			&rd_buff, STWLC68_AUX_FW_ADDR_LEN);
		if (ret)
			return false;
		if (!rd_buff)
			goto check_sys_err;
		(void)power_msleep(DT_MSLEEP_25MS, 0, NULL);
	}

	return false;

check_sys_err:
	ret = stwlc68_aux_read_byte(STWLC68_AUX_SYSTEM_ERR_ADDR, &rd_buff);
	if (ret)
		return false;
	if (!rd_buff)
		return true;
	return false;
}

static int stwlc68_aux_fw_rom_check(void)
{
	int ret;

	stwlc68_aux_reset_system();
	(void)power_msleep(STWLC68_AUX_OTP_OPS_DELAY, 0, NULL);

	ret = stwlc68_aux_load_ram_fw(g_stwlc68_aux_ram_data,
		ARRAY_SIZE(g_stwlc68_aux_ram_data));
	if (ret) {
		hwlog_err("%s: load sram fail\n", __func__);
		return ret;
	}

	ret = stwlc68_aux_set_boot_mode(STWLC68_AUX_BOOT_FROM_RAM);
	if (ret) {
		hwlog_err("%s: boot from sram fail\n", __func__);
		return ret;
	}

	stwlc68_aux_reset_system();
	(void)power_msleep(STWLC68_AUX_OTP_OPS_DELAY, 0, NULL);

	ret = stwlc68_aux_show_ram_fw_version();
	if (ret) {
		hwlog_err("%s: show sram version fail\n", __func__);
		goto check_fail;
	}

	if (!stwlc68_aux_is_rom_ok()) {
		hwlog_err("%s: rom is bad\n", __func__);
		goto check_fail;
	}

	(void)stwlc68_aux_set_boot_mode(STWLC68_AUX_BOOT_FROM_ROM);
	return 0;

check_fail:
	(void)stwlc68_aux_set_boot_mode(STWLC68_AUX_BOOT_FROM_ROM);
	return ret;
}

static int stwlc68_aux_system_check(void)
{
	int ret;

	if (g_ram_rom_status == STWLC68_AUX_RAM_ROM_STATUS_OK)
		return 0;

	ret = stwlc68_aux_fw_ram_check();
	if (ret) {
		hwlog_err("%s: ram check fail\n", __func__);
		g_ram_rom_status = STWLC68_AUX_RAM_ROM_STATUS_BAD;
		return ret;
	}

	ret = stwlc68_aux_fw_rom_check();
	if (ret) {
		hwlog_err("%s: rom check fail\n", __func__);
		g_ram_rom_status = STWLC68_AUX_RAM_ROM_STATUS_BAD;
		return ret;
	}

	g_ram_rom_status = STWLC68_AUX_RAM_ROM_STATUS_OK;
	return 0;
}

static int stwlc68_aux_program_otp_pre_check(void)
{
	int ret;

	ret = stwlc68_aux_check_dc_power();
	if (ret) {
		hwlog_err("%s: OTP must be programmed in DC power\n", __func__);
		return ret;
	}

	ret = stwlc68_aux_system_check();
	if (ret) {
		hwlog_err("%s: system check fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_aux_recover_otp_pre_check(void)
{
	int ret;

	ret = stwlc68_aux_check_dc_power();
	if (ret) {
		hwlog_err("%s: OTP must be programmed in DC power\n", __func__);
		return ret;
	}

	ret = stwlc68_aux_system_check();
	if (ret) {
		hwlog_err("%s: system check fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_aux_program_otp_choose_otp(int *otp_id)
{
	int i;
	int ret;
	u8 cut_id = 0;
	int otp_num;

	ret = stwlc68_aux_get_cut_id(&cut_id);
	if (ret) {
		hwlog_err("%s: get cut_id fail\n", __func__);
		return ret;
	}
	hwlog_info("[%s] cut_id = 0x%x\n", __func__, cut_id);
	/* determine what has to be programmed depending on version ids */
	otp_num = ARRAY_SIZE(st_otp_aux_info);
	for (i = 0; i < otp_num; i++) {
		if ((cut_id >= st_otp_aux_info[i].cut_id_from) &&
			(cut_id <= st_otp_aux_info[i].cut_id_to)) {
			*otp_id = i;
			return 0;
		}
	}

	hwlog_err("%s: cut_id mismatch\n", __func__);
	return -WLC_ERR_MISMATCH;
}

static int stwlc68_aux_program_otp_calc_otp_addr(int otp_id,
	const u8 **data_to_program, u32 *data_to_program_size)
{
	int ret;
	u16 cfg_id = 0;
	u16 patch_id = 0;
	int cfg_id_mismatch = 0;
	int patch_id_mismatch = 0;

	ret = stwlc68_aux_get_cfg_id(&cfg_id);
	if (ret) {
		hwlog_err("%s: get cfg_id fail\n", __func__);
		return ret;
	}
	if (cfg_id != st_otp_aux_info[otp_id].cfg_id) {
		hwlog_err("[%s] cfg_id mismatch, running|latest:0x%x|0x%x\n",
			__func__, cfg_id, st_otp_aux_info[otp_id].cfg_id);
		cfg_id_mismatch = 1;
	}
	ret = stwlc68_aux_get_patch_id(&patch_id);
	if (ret) {
		hwlog_err("%s: get patch_id fail\n", __func__);
		return ret;
	}
	if (patch_id != st_otp_aux_info[otp_id].patch_id) {
		hwlog_err("[%s] patch_id mismatch, running|latest:0x%x|0x%x\n",
			__func__, patch_id, st_otp_aux_info[otp_id].patch_id);
		patch_id_mismatch = 1;
	}
	if (cfg_id_mismatch && patch_id_mismatch) {
		*data_to_program_size = st_otp_aux_info[otp_id].cfg_size +
			st_otp_aux_info[otp_id].patch_size +
			STWLC68_AUX_OTP_DUMMY_LEN;
		*data_to_program = st_otp_aux_info[otp_id].otp_arr;
	} else if (cfg_id_mismatch && !patch_id_mismatch) {
		*data_to_program_size = st_otp_aux_info[otp_id].cfg_size;
		*data_to_program = st_otp_aux_info[otp_id].otp_arr;
	} else if (!cfg_id_mismatch && patch_id_mismatch) {
		*data_to_program_size = st_otp_aux_info[otp_id].patch_size;
		*data_to_program = st_otp_aux_info[otp_id].otp_arr +
			st_otp_aux_info[otp_id].cfg_size;
	} else {
		hwlog_info("[%s] cfg && patch are latest\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	return 0;
}

static int stwlc68_aux_program_otp_check_free_addr(
	u32 current_clean_addr, u32 data_to_program_size)
{
	hwlog_info("[%s] opt_max_size: 0x%04X, otp_size: 0x%08X\n",
		__func__, STWLC68_AUX_OTP_MAX_SIZE, data_to_program_size);
	if ((STWLC68_AUX_OTP_MAX_SIZE - (current_clean_addr & WORD_MASK)) <
		data_to_program_size) {
		hwlog_err("%s: not enough space available\n", __func__);
		return -WLC_ERR_NO_SPACE;
	}

	return 0;
}

static int stwlc68_aux_skip_soak_recover_otp(u32 err_addr,
	const u8 *otp_data, int otp_size, u8 *dirty_otp_id, int dirty_otp_size)
{
	int ret;
	u32 addr;
	u8 write_buff;

	addr = STWLC68_AUX_OTP_SKIP_SOAK_ADDR;
	write_buff = STWLC68_AUX_OTP_SKIP_SOAK_VAL;
	ret = stwlc68_aux_4addr_write_block(addr, &write_buff,
		STWLC68_AUX_FW_ADDR_LEN);
	if (ret)
		return ret;

	return stwlc68_aux_recover_otp_data(err_addr, otp_data, otp_size,
		dirty_otp_id, dirty_otp_size);
}

static int stwlc68_aux_recover_otp_prepare(void)
{
	int ret;

	stwlc68_aux_reset_system();
	(void)power_msleep(STWLC68_AUX_OTP_OPS_DELAY, 0, NULL);
	ret = stwlc68_aux_disable_fw_i2c();
	if (ret) {
		hwlog_err("%s: disable fw_i2c fail\n", __func__);
		return ret;
	}
	ret = stwlc68_aux_hold_m0();
	if (ret) {
		hwlog_err("%s: hold M0 fail\n", __func__);
		return ret;
	}
	(void)power_msleep(STWLC68_AUX_OTP_OPS_DELAY, 0, NULL);
	ret = stwlc68_aux_osc_trim();
	if (ret) {
		hwlog_err("%s: hold M0 fail\n", __func__);
		return ret;
	}
	ret = stwlc68_aux_clk_div();
	if (ret) {
		hwlog_err("%s: hold M0 fail\n", __func__);
		return ret;
	}
	ret = stwlc68_aux_reset_otp_block();
	if (ret) {
		hwlog_err("%s: OTP block reset fail\n", __func__);
		return ret;
	}
	ret = stwlc68_aux_enable_otp();
	if (ret) {
		hwlog_err("%s: OTP enable fail\n", __func__);
		return ret;
	}
	ret = stwlc68_aux_unlock_otp();
	if (ret) {
		hwlog_err("%s: unlock OTP fail\n", __func__);
		return ret;
	}
	ret = stwlc68_aux_config_otp_registers();
	if (ret) {
		hwlog_err("%s: config otp registers fail\n", __func__);
		return ret;
	}
	ret = stwlc68_aux_disable_iload();
	if (ret) {
		hwlog_err("%s: disable otp iload fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_aux_program_otp_post_check(int otp_id)
{
	int ret;
	u16 cfg_id = 0;
	u16 patch_id = 0;

	ret = stwlc68_aux_get_cfg_id(&cfg_id);
	if (ret) {
		hwlog_err("%s: get cfg_id fail\n", __func__);
		return ret;
	}
	if (cfg_id != st_otp_aux_info[otp_id].cfg_id)
		hwlog_err("%s: cfg_id(0x%x) mismatch after flashing\n",
			__func__, cfg_id);

	ret = stwlc68_aux_get_patch_id(&patch_id);
	if (ret) {
		hwlog_err("%s: get patch_id fail\n", __func__);
		return ret;
	}
	if (patch_id != st_otp_aux_info[otp_id].patch_id)
		hwlog_err("%s: patch_id(0x%x) mismatch after flashing\n",
			__func__, patch_id);

	return 0;
}

static int stwlc68_aux_recover_otp_post_check(int otp_id)
{
	int ret;

	ret = stwlc68_aux_program_otp_post_check(otp_id);
	if (ret)
		return ret;

	if (stwlc68_aux_is_otp_corrupt())
		return -WLC_ERR_CHECK_FAIL;

	return 0;
}

static void stwlc68_aux_ps_control(unsigned int ic_type, int scene, bool flag)
{
	static int ref_cnt;

	hwlog_info("[ps_control] cnt=%d, flag=%d\n", ref_cnt, flag);
	if (flag)
		++ref_cnt;
	else if (--ref_cnt > 0)
		return;

	wlps_control(ic_type, scene, flag);
}

static int stwlc68_aux_recover_otp(struct stwlc68_aux_dev_info *di)
{
	int ret;
	int otp_id = 0;
	u32 data_to_program_size;
	const u8 *data_to_program = NULL;
	u8 *dirty_otp_id = NULL;
	int dirty_otp_size = STWLC68_AUX_OTP_MAX_SIZE / STWLC68_AUX_OTP_RECOVER_WR_SIZE;

	stwlc68_aux_disable_irq_nosync(di);
	stwlc68_aux_ps_control(di->ic_type, WLPS_PROC_OTP_PWR, true);
	(void)power_msleep(STWLC68_AUX_OTP_OPS_DELAY, 0, NULL);

	ret = stwlc68_aux_recover_otp_pre_check();
	if (ret)
		goto exit;
	ret = stwlc68_aux_program_otp_choose_otp(&otp_id);
	if (ret)
		goto exit;

	hwlog_info("[%s] otp_index = %d\n", __func__, otp_id);
	data_to_program = st_otp_aux_info[otp_id].otp_arr;
	data_to_program_size = st_otp_aux_info[otp_id].cfg_size +
		st_otp_aux_info[otp_id].patch_size + STWLC68_AUX_OTP_DUMMY_LEN;
	ret = stwlc68_aux_recover_otp_prepare();
	if (ret)
		goto exit;
	g_otp_skip_soak_recovery_flag = false;
	dirty_otp_id = kzalloc(sizeof(u8) * dirty_otp_size, GFP_KERNEL);
	if (!dirty_otp_id)
		goto exit;
	memset(dirty_otp_id, 1, dirty_otp_size);
	ret = stwlc68_aux_recover_otp_data(STWLC68_AUX_OTP_CLEAN_ADDR,
		data_to_program, data_to_program_size, dirty_otp_id, dirty_otp_size);

	hwlog_info("[%s] result = %d\n", __func__, ret);
	if (ret) {
		g_otp_skip_soak_recovery_flag = true;
		ret = stwlc68_aux_recover_otp_prepare();
		if (ret)
			goto exit;
		ret = stwlc68_aux_skip_soak_recover_otp(
			STWLC68_AUX_OTP_CLEAN_ADDR,
			data_to_program, data_to_program_size, dirty_otp_id, dirty_otp_size);
		if (ret)
			goto exit;
	}
	ret = stwlc68_aux_get_iload();
	if (ret)
		goto exit;

	stwlc68_aux_soft_reset_system();

	(void)power_msleep(STWLC68_AUX_SHUTDOWN_SLEEP_TIME, 0, NULL);
	ret = stwlc68_aux_recover_otp_post_check(otp_id);
	if (ret)
		goto exit;
	stwlc68_aux_reset_system();
	(void)power_msleep(STWLC68_AUX_OTP_OPS_DELAY, 0, NULL);
	stwlc68_aux_ps_control(di->ic_type, WLPS_PROC_OTP_PWR, false);
	stwlc68_aux_enable_irq(di);
	kfree(dirty_otp_id);
	hwlog_info("[%s] succ\n", __func__);
	return 0;

exit:
	stwlc68_aux_reset_system();
	(void)power_msleep(STWLC68_AUX_OTP_OPS_DELAY, 0, NULL);
	stwlc68_aux_ps_control(di->ic_type, WLPS_PROC_OTP_PWR, false);
	stwlc68_aux_enable_irq(di);
	kfree(dirty_otp_id);
	hwlog_info("[%s] fail\n", __func__);
	return ret;
}

static int stwlc68_aux_program_otp(struct stwlc68_aux_dev_info *di)
{
	int ret;
	int otp_id = 0;
	u32 current_clean_addr = 0;
	u32 data_to_program_size = 0;
	const u8 *data_to_program = NULL;

	stwlc68_aux_disable_irq_nosync(di);
	stwlc68_aux_ps_control(di->ic_type, WLPS_PROC_OTP_PWR, true);
	(void)power_msleep(STWLC68_AUX_OTP_OPS_DELAY, 0, NULL);

	ret = stwlc68_aux_program_otp_pre_check();
	if (ret)
		goto exit;
	stwlc68_aux_reset_system();
	(void)power_msleep(STWLC68_AUX_OTP_OPS_DELAY, 0, NULL);
	stwlc68_aux_get_clean_addr(&current_clean_addr);
	ret = stwlc68_aux_program_otp_choose_otp(&otp_id);
	if (ret)
		goto exit;
	hwlog_info("[%s] otp_index = %d\n", __func__, otp_id);
	ret = stwlc68_aux_program_otp_calc_otp_addr(otp_id,
		&data_to_program, &data_to_program_size);
	if (ret)
		goto exit;
	ret = stwlc68_aux_load_ram_fw(g_stwlc68_aux_ram_data,
		ARRAY_SIZE(g_stwlc68_aux_ram_data));
	if (ret)
		goto set_mode_exit;
	ret = stwlc68_aux_set_boot_mode(STWLC68_AUX_BOOT_FROM_RAM);
	if (ret)
		goto set_mode_exit;
	(void)power_msleep(STWLC68_AUX_OTP_OPS_DELAY, 0, NULL);
	ret = stwlc68_aux_show_ram_fw_version();
	if (ret)
		goto set_mode_exit;
	ret = stwlc68_aux_program_otp_check_free_addr(
		STWLC68_AUX_OTP_CLEAN_ADDR,
		data_to_program_size);
	if (ret)
		goto set_mode_exit;
	ret = stwlc68_aux_update_clean_addr(STWLC68_AUX_OTP_CLEAN_ADDR);
	if (ret)
		goto set_mode_exit;
	ret = stwlc68_aux_disable_iload();
	if (ret)
		goto set_mode_exit;

	ret = stwlc68_aux_write_otp_data(data_to_program, data_to_program_size);
	if (ret)
		goto set_mode_exit;
	ret = stwlc68_aux_get_iload();
	if (ret)
		goto set_mode_exit;
	ret = stwlc68_aux_check_system_err();
	if (ret)
		goto set_mode_exit;
	ret = stwlc68_aux_set_boot_mode(STWLC68_AUX_BOOT_FROM_ROM);
	if (ret)
		goto exit;
	stwlc68_aux_reset_system();
	(void)power_msleep(STWLC68_AUX_OTP_OPS_DELAY, 0, NULL);

	ret = stwlc68_aux_program_otp_post_check(otp_id);
	if (ret)
		goto exit;
	stwlc68_aux_reset_system();
	(void)power_msleep(STWLC68_AUX_OTP_OPS_DELAY, 0, NULL);

	stwlc68_aux_ps_control(di->ic_type, WLPS_PROC_OTP_PWR, false);
	stwlc68_aux_enable_irq(di);
	hwlog_info("[%s] succ\n", __func__);
	return 0;

set_mode_exit:
	(void)stwlc68_aux_set_boot_mode(STWLC68_AUX_BOOT_FROM_ROM);
exit:
	stwlc68_aux_reset_system();
	(void)power_msleep(STWLC68_AUX_OTP_OPS_DELAY, 0, NULL);
	stwlc68_aux_ps_control(di->ic_type, WLPS_PROC_OTP_PWR, false);
	stwlc68_aux_enable_irq(di);
	hwlog_info("[%s] fail\n", __func__);
	return ret;
}

static int stwlc68_aux_rx_program_otp(unsigned int proc_type, void *dev_data)
{
	struct stwlc68_aux_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	switch (proc_type) {
	case WLC_PROGRAM_OTP:
		return stwlc68_aux_program_otp(di);
	case WLC_RECOVER_OTP:
		return stwlc68_aux_recover_otp(di);
	default:
		break;
	}

	return -EINVAL;
}

static int stwlc68_aux_check_otp(void *dev_data)
{
	int ret;
	int i;
	int otp_num;
	u8 cut_id = 0;
	u16 cfg_id = 0;
	u16 patch_id = 0;
	struct stwlc68_aux_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	stwlc68_aux_ps_control(di->ic_type, WLPS_PROC_OTP_PWR, true);
	stwlc68_aux_chip_enable(RX_EN_ENABLE); /* enable RX for i2c WR */
	(void)power_msleep(STWLC68_AUX_OTP_OPS_DELAY, 0, NULL);
	hwlog_info("[%s] ++\n", __func__);
	ret = stwlc68_aux_system_check();
	if (ret) {
		hwlog_err("%s: system check fail\n", __func__);
		goto exit;
	}
	stwlc68_aux_reset_system();
	(void)power_msleep(STWLC68_AUX_OTP_OPS_DELAY, 0, NULL);
	ret = stwlc68_aux_get_cut_id(&cut_id);
	if (ret) {
		hwlog_err("%s: get cut_id fail\n", __func__);
		goto exit;
	}
	hwlog_info("[%s] cut_id = 0x%x\n", __func__, cut_id);
	otp_num = ARRAY_SIZE(st_otp_aux_info);
	for (i = 0; i < otp_num; i++) {
		if ((cut_id >= st_otp_aux_info[i].cut_id_from) &&
			(cut_id <= st_otp_aux_info[i].cut_id_to))
			break;
	}
	if (i == otp_num) {
		hwlog_err("%s: cut_id mismatch\n", __func__);
		ret = WLC_ERR_MISMATCH;
		goto exit;
	}
	ret = stwlc68_aux_get_patch_id(&patch_id);
	if (ret) {
		hwlog_err("%s: get patch_id fail\n", __func__);
		goto exit;
	}
	ret = stwlc68_aux_get_cfg_id(&cfg_id);
	if (ret) {
		hwlog_err("%s: get cfg_id fail\n", __func__);
		goto exit;
	}

	if ((patch_id != st_otp_aux_info[i].patch_id) ||
		(cfg_id != st_otp_aux_info[i].cfg_id))
		ret = WLC_ERR_MISMATCH;
exit:
	stwlc68_aux_ps_control(di->ic_type, WLPS_PROC_OTP_PWR, false);
	hwlog_info("[%s] --\n", __func__);
	return ret;
}

static int stwlc68_aux_get_otp_status(unsigned int *status, void *dev_data)
{
	int ret;
	struct stwlc68_aux_dev_info *di = dev_data;

	if (!di || !status)
		return -EINVAL;

	ret = stwlc68_aux_check_otp(di);
	if (!ret)
		*status = WIRELESS_FW_PROGRAMED;
	else
		*status = WIRELESS_FW_ERR_PROGRAMED;

	return 0;
}

static bool stwlc68_aux_is_updated_sram_match(u16 sram_id)
{
	int ret;
	struct stwlc68_aux_chip_info chip_info;

	ret = stwlc68_aux_get_chip_info(&chip_info);
	if (ret) {
		hwlog_err("%s: get chip info fail\n", __func__);
		return false;
	}

	hwlog_info("[%s] sram_id = 0x%x\n", __func__, chip_info.sram_id);
	return (sram_id == chip_info.sram_id);
}

static int stwlc68_aux_write_sram_exe_cmd(void)
{
	return stwlc68_aux_write_byte(STWLC68_AUX_EXE_SRAM_ADDR,
		STWLC68_AUX_EXE_SRAM_CMD);
}

static int stwlc68_aux_write_sram_start_addr(u32 start_addr)
{
	struct stwlc68_aux_dev_info *di = g_stwlc68_aux_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	return stwlc68_aux_write_block(di, STWLC68_AUX_SRAM_START_ADDR_ADDR,
		(u8 *)&start_addr, STWLC68_AUX_SRAM_START_ADDR_LEN);
}

static int stwlc68_aux_write_data_to_sram(u32 start_addr,
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
		ret = stwlc68_aux_4addr_write_block(address, buff,
			size_to_write);
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

static int stwlc68_aux_program_sramupdate(const struct st_sram_info *sram_info)
{
	int ret;
	u32 start_addr;

	/* start_addr obtained from sram_data[4:7] in little endian */
	start_addr = (u32)((sram_info->sram_data[4] << 0) |
		(sram_info->sram_data[5] << 8) |
		(sram_info->sram_data[6] << 16) |
		(sram_info->sram_data[7] << 24));

	ret = stwlc68_aux_write_data_to_sram(start_addr,
		sram_info->sram_data + ST_RAMPATCH_HEADER_SIZE,
		sram_info->sram_size - ST_RAMPATCH_HEADER_SIZE);
	if (ret)
		return ret;

	ret = stwlc68_aux_write_sram_start_addr(start_addr + 1);
	if (ret)
		return ret;

	ret = stwlc68_aux_write_sram_exe_cmd();
	if (ret)
		return ret;

	mdelay(5); /* delay 5ms for exe cmd */
	if (stwlc68_aux_is_updated_sram_match(sram_info->sram_id))
		return 0;

	mdelay(5); /* delay 5ms for exe cmd */
	if (!stwlc68_aux_is_updated_sram_match(sram_info->sram_id))
		return -WLC_ERR_MISMATCH;

	return 0;
}

static int stwlc68_aux_find_sram_id(
	enum wireless_mode sram_mode, unsigned int *sram_id)
{
	int i;
	int ret;
	unsigned int fw_sram_num;
	struct stwlc68_aux_chip_info chip_info;

	ret = stwlc68_aux_get_chip_info(&chip_info);
	if (ret) {
		hwlog_err("%s: get chip info fail\n", __func__);
		return ret;
	}

	fw_sram_num = ARRAY_SIZE(stwlc68_aux_sram);
	for (i = 0; i < fw_sram_num; i++) {
		if (sram_mode != stwlc68_aux_sram[i].fw_sram_mode)
			continue;
		if ((chip_info.cut_id < stwlc68_aux_sram[i].cut_id_from) ||
			(chip_info.cut_id > stwlc68_aux_sram[i].cut_id_to))
			continue;
		if ((chip_info.cfg_id < stwlc68_aux_sram[i].cfg_id_from) ||
			(chip_info.cfg_id > stwlc68_aux_sram[i].cfg_id_to))
			continue;
		if ((chip_info.patch_id < stwlc68_aux_sram[i].patch_id_from) ||
			(chip_info.patch_id > stwlc68_aux_sram[i].patch_id_to))
			continue;
		if (!power_cmdline_is_factory_mode() &&
			((g_sram_bad_addr <
			stwlc68_aux_sram[i].bad_addr_from) ||
			(g_sram_bad_addr > stwlc68_aux_sram[i].bad_addr_to)))
			continue;
		if ((stwlc68_aux_sram[i].sram_size <=
			ST_RAMPATCH_HEADER_SIZE) ||
			(stwlc68_aux_sram[i].sram_size > ST_RAMPATCH_MAX_SIZE))
			continue;

		hwlog_info("[%s] bad_addr=0x%x sram_id=%d\n",
			__func__, g_sram_bad_addr, i);
		*sram_id = i;
		return 0;
	}

	return -WLC_ERR_MISMATCH;
}

static int stwlc68_aux_sw2tx(void)
{
	int ret;
	int i;
	u8 mode = 0;
	int cnt = STWLC68_AUX_SW2TX_RETRY_TIME /
		STWLC68_AUX_SW2TX_RETRY_SLEEP_TIME;

	for (i = 0; i < cnt; i++) {
		(void)power_msleep(STWLC68_AUX_SW2TX_RETRY_SLEEP_TIME, 0, NULL);
		ret = stwlc68_aux_get_mode(&mode);
		if (ret) {
			hwlog_err("%s: get mode fail\n", __func__);
			continue;
		}

		ret = stwlc68_aux_write_word_mask(STWLC68_AUX_CMD_ADDR,
			STWLC68_AUX_CMD_SW2TX, STWLC68_AUX_CMD_SW2TX_SHIFT,
			STWLC68_AUX_CMD_VAL);
		if (ret) {
			hwlog_err("%s: write cmd(sw2tx) fail\n", __func__);
			continue;
		}
		if (mode == STWLC68_AUX_TX_MODE) {
			hwlog_info("%s: succ, cnt = %d\n", __func__, i);
			return 0;
		}
	}
	hwlog_err("%s: fail, cnt = %d\n", __func__, i);
	return -WLC_ERR_I2C_WR;
}

static int stwlc68_aux_pre_sramupdate(enum wireless_mode sram_mode)
{
	int ret;

	if (sram_mode == WIRELESS_TX_MODE) {
		ret = stwlc68_aux_sw2tx();
		if (ret)
			return ret;

		(void)power_msleep(DT_MSLEEP_50MS, 0, NULL);
	}

	return 0;
}

static int stwlc68_aux_check_fwupdate(enum wireless_mode sram_mode)
{
	int ret;
	unsigned int fw_sram_id = 0;
	unsigned int fw_sram_num;
	u8 chip_info[STWLC68_AUX_CHIP_INFO_STR_LEN] = { 0 };

	ret = stwlc68_aux_get_chip_info_str(chip_info,
		STWLC68_AUX_CHIP_INFO_STR_LEN);
	if (ret)
		hwlog_err("get chip_info fail\n");

	ret = stwlc68_aux_find_sram_id(sram_mode, &fw_sram_id);
	if (ret) {
		hwlog_err("%s: sram no need update\n", __func__);
		return ret;
	}

	fw_sram_num = ARRAY_SIZE(stwlc68_aux_sram);
	if (fw_sram_id >= fw_sram_num) {
		hwlog_err("%s: fw_sram_id = %u err\n", __func__, fw_sram_id);
		return -WLC_ERR_PARA_WRONG;
	}

	if (stwlc68_aux_is_updated_sram_match(
		stwlc68_aux_sram[fw_sram_id].sram_id))
		return 0;

	ret = stwlc68_aux_pre_sramupdate(sram_mode);
	if (ret) {
		hwlog_err("%s: pre sramupdate fail\n", __func__);
		return ret;
	}

	ret = stwlc68_aux_program_sramupdate(&stwlc68_aux_sram[fw_sram_id]);
	if (ret) {
		hwlog_err("%s: sram update fail\n", __func__);
		return ret;
	}

	ret = stwlc68_aux_get_chip_info_str(chip_info,
		STWLC68_AUX_CHIP_INFO_STR_LEN);
	if (ret)
		hwlog_err("get chip_info fail\n");

	hwlog_info("[%s] sram update succ\n", __func__);

	return 0;
}

static int stwlc68_aux_get_chip_fw_version(u8 *data, int len)
{
	struct stwlc68_aux_chip_info chip_info;

	if (stwlc68_aux_get_chip_info(&chip_info)) {
		hwlog_err("%s: get chip info fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	/* fw version length must be 4 */
	if (len != 4)
		return -WLC_ERR_CHECK_FAIL;

	/* byte[0:1]=patch_id, byte[2:3]=sram_id */
	data[0] = (u8)((chip_info.patch_id >> 0) & BYTE_MASK);
	data[1] = (u8)((chip_info.patch_id >> BITS_PER_BYTE) & BYTE_MASK);
	data[2] = (u8)((chip_info.sram_id >> 0) & BYTE_MASK);
	data[3] = (u8)((chip_info.sram_id >> BITS_PER_BYTE) & BYTE_MASK);
	return 0;
}

static int stwlc68_aux_get_ask_pkt(u8 *pkt_data, int pkt_data_len)
{
	int ret;
	int i;
	char buff[STWLC68_AUX_RCVD_PKT_BUFF_LEN] = { 0 };
	char pkt_str[STWLC68_AUX_RCVD_PKT_STR_LEN] = { 0 };
	struct stwlc68_aux_dev_info *di = g_stwlc68_aux_di;

	if (!di || !pkt_data || (pkt_data_len <= 0) ||
		(pkt_data_len > STWLC68_AUX_RCVD_MSG_PKT_LEN)) {
		hwlog_err("%s: para err\n", __func__);
		return -EINVAL;
	}
	ret = stwlc68_aux_read_block(di, STWLC68_AUX_RCVD_MSG_HEADER_ADDR,
		pkt_data, pkt_data_len);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}
	for (i = 0; i < pkt_data_len; i++) {
		snprintf(buff, STWLC68_AUX_RCVD_PKT_BUFF_LEN, "0x%02x ",
			pkt_data[i]);
		strncat(pkt_str, buff, strlen(buff));
	}
	hwlog_info("[%s] RX back packet: %s\n", __func__, pkt_str);
	return 0;
}

static int stwlc68_aux_send_fsk_msg(u8 cmd, u8 *data, int data_len)
{
	int ret;
	u8 header;
	u8 write_data[STWLC68_AUX_SEND_MSG_DATA_LEN] = { 0 };
	struct stwlc68_aux_dev_info *di = g_stwlc68_aux_di;

	if (!di) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	if ((data_len > STWLC68_AUX_SEND_MSG_DATA_LEN) || (data_len < 0)) {
		hwlog_err("%s: data len out of range\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	if (cmd == STWLC68_AUX_CMD_ACK)
		header = STWLC68_AUX_CMD_ACK_HEAD;
	else
		header = stwlc68_aux_get_fsk_header(data_len + 1);

	ret = stwlc68_aux_write_byte(STWLC68_AUX_SEND_MSG_HEADER_ADDR, header);
	if (ret) {
		hwlog_err("%s: write header fail\n", __func__);
		return ret;
	}
	ret = stwlc68_aux_write_byte(STWLC68_AUX_SEND_MSG_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("%s: write cmd fail\n", __func__);
		return ret;
	}

	if (data && data_len > 0) {
		memcpy(write_data, data, data_len);
		ret = stwlc68_aux_write_block(di,
			STWLC68_AUX_SEND_MSG_DATA_ADDR,
			write_data, data_len);
		if (ret) {
			hwlog_err("%s: write fsk reg fail\n", __func__);
			return ret;
		}
	}
	ret = stwlc68_aux_write_word_mask(STWLC68_AUX_CMD_ADDR,
		STWLC68_AUX_CMD_SEND_MSG, STWLC68_AUX_CMD_SEND_MSG_SHIFT,
		STWLC68_AUX_CMD_VAL);
	if (ret) {
		hwlog_err("%s: send fsk fail\n", __func__);
		return ret;
	}

	hwlog_info("[%s] success\n", __func__);
	return 0;
}

static int qi_protocol_send_fsk_msg(u8 cmd, u8 *para, int para_len, int retrys)
{
	int i;
	int ret;

	for (i = 0; i < retrys; i++) {
		ret = stwlc68_aux_send_fsk_msg(cmd, para, para_len);
		if (ret) {
			hwlog_err("0x%x fsk_msg send fail, retry %d\n", cmd, i);
			continue;
		}

		break;
	}

	if (i >= retrys)
		return -WLC_ERR_CHECK_FAIL;

	return 0;
}

static int qi_protocol_send_fsk_ack_msg(void)
{
	return qi_protocol_send_fsk_msg(QI_CMD_ACK, NULL, 0,
		WIRELESS_RETRY_ONE);
}

static int qi_protocol_get_ask_packet(u8 *data, int data_len, int retrys)
{
	int i;
	int ret;

	for (i = 0; i < retrys; i++) {
		ret = stwlc68_aux_get_ask_pkt(data, data_len);
		if (ret) {
			hwlog_err("ask_packet receive fail, retry %d\n", i);
			continue;
		}

		break;
	}

	if (i >= retrys)
		return -WLC_ERR_CHECK_FAIL;

	return 0;
}

static int qi_protocol_get_chip_fw_version(u8 *data, int data_len, int retrys)
{
	int i;
	int ret;

	for (i = 0; i < retrys; i++) {
		ret = stwlc68_aux_get_chip_fw_version(data, data_len);
		if (ret) {
			hwlog_err("chip_fw_version get fail, retry %d\n", i);
			continue;
		}

		break;
	}

	if (i >= retrys)
		return -WLC_ERR_CHECK_FAIL;

	return 0;
}

static int qi_protocol_send_tx_fw_version(u8 *fw, int len)
{
	int retry = WIRELESS_RETRY_ONE;

	if (!fw)
		return -EINVAL;

	if (len != (QI_ACK_TX_FWVER_LEN - 1)) {
		hwlog_err("para error %d!=%d\n", len, QI_ACK_TX_FWVER_LEN - 1);
		return -EINVAL;
	}

	if (qi_protocol_send_fsk_msg(QI_CMD_GET_TX_VERSION, fw, len, retry))
		return -WLC_ERR_I2C_W;

	hwlog_info("send_tx_fw_version succ\n");
	return 0;
}

static int qi_protocol_send_tx_id(u8 *id, int len)
{
	int retry = WIRELESS_RETRY_ONE;

	if (!id)
		return -EINVAL;

	if (len != QI_PARA_TX_ID_LEN) {
		hwlog_err("para error %d!=%d\n", len, QI_PARA_TX_ID_LEN);
		return -EINVAL;
	}

	if (qi_protocol_send_fsk_msg(QI_CMD_GET_TX_ID, id, len, retry))
		return -WLC_ERR_I2C_W;

	hwlog_info("send_tx_id succ\n");
	return 0;
}

/* 0x01 + signal_strength + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x01(u8 *data)
{
	hwlog_info("ask_packet_cmd_0x01: %u\n", data[QI_ASK_PACKET_DAT0]);
	return 0;
}

/* 0x18 + 0x05 + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x05(void)
{
	u8 data[QI_ACK_TX_FWVER_LEN - 1] = { 0 };
	int retry = WIRELESS_RETRY_ONE;

	qi_protocol_get_chip_fw_version(data, QI_ACK_TX_FWVER_LEN - 1, retry);
	qi_protocol_send_tx_fw_version(data, QI_ACK_TX_FWVER_LEN - 1);

	hwlog_info("ask_packet_cmd_0x05\n");
	return 0;
}

/* 0x38 + 0x0a + volt_lbyte + volt_hbyte + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x0a(u8 *data)
{
	int tx_vset;

	tx_vset = (data[QI_ASK_PACKET_DAT2] << QI_PROTOCOL_BYTE_BITS) |
		data[QI_ASK_PACKET_DAT1];
	return 0;
}

/* 0x38 + 0x3b + id_hbyte + id_lbyte + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x3b(u8 *data)
{
	int tx_id;

	tx_id = (data[QI_ASK_PACKET_DAT1] << QI_PROTOCOL_BYTE_BITS) |
		data[QI_ASK_PACKET_DAT2];

	if (tx_id == QI_HANDSHAKE_ID_HW) {
		qi_protocol_send_tx_id(&data[QI_ASK_PACKET_DAT1],
			QI_PARA_TX_ID_LEN);
		hwlog_info("0x8866 handshake succ\n");
		power_event_bnc_notify(POWER_BNT_WLTX_AUX,
			POWER_NE_WLTX_HANDSHAKE_SUCC, NULL);
	}

	hwlog_info("ask_packet_cmd_0x3b: tx_id=0x%x\n", tx_id);
	return 0;
}

/* 0x28 + 0x43 + charger_state + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x43(u8 *data)
{
	int chrg_state;

	chrg_state = data[QI_ASK_PACKET_DAT1];
	qi_protocol_send_fsk_ack_msg();

	if (chrg_state & QI_CHRG_STATE_DONE) {
		hwlog_info("tx received rx charge-done event\n");
		power_event_bnc_notify(POWER_BNT_WLTX_AUX,
			POWER_NE_WLTX_CHARGEDONE, NULL);
	}

	hwlog_info("ask_packet_cmd_0x43: charger_state=0x%x\n", chrg_state);
	return 0;
}

/* 0x58 + 0x52 + mac1 + mac2 + mac3 + mac4 + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x52(u8 *data)
{
	qi_protocol_send_fsk_ack_msg();

	hwlog_info("ask_packet_cmd_0x52: %u\n", g_wltx_acc->dev_info_cnt);
	if (g_wltx_acc->dev_info_cnt != 0)
		g_wltx_acc->dev_info_cnt = 0;

	g_wltx_acc->dev_mac[QI_ACC_OFFSET0] = data[QI_ASK_PACKET_DAT1];
	g_wltx_acc->dev_mac[QI_ACC_OFFSET1] = data[QI_ASK_PACKET_DAT2];
	g_wltx_acc->dev_mac[QI_ACC_OFFSET2] = data[QI_ASK_PACKET_DAT3];
	g_wltx_acc->dev_mac[QI_ACC_OFFSET3] = data[QI_ASK_PACKET_DAT4];
	g_wltx_acc->dev_info_cnt += QI_ASK_PACKET_DAT_LEN;

	return 0;
}

/* 0x58 + 0x53 + mac5 + mac6 + version + business + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x53(u8 *data)
{
	qi_protocol_send_fsk_ack_msg();

	hwlog_info("ask_packet_cmd_0x53: %u\n", g_wltx_acc->dev_info_cnt);
	if (g_wltx_acc->dev_info_cnt < QI_ASK_PACKET_DAT_LEN) {
		hwlog_info("cmd_0x53 cnt not right\n");
		return -WLC_ERR_CHECK_FAIL;
	}

	/*
	 * solve rx not receive ack from tx, and sustain send ask packet,
	 * but tx data count check fail, reset info count
	 */
	g_wltx_acc->dev_info_cnt = QI_ASK_PACKET_DAT_LEN;
	g_wltx_acc->dev_mac[QI_ACC_OFFSET4] = data[QI_ASK_PACKET_DAT1];
	g_wltx_acc->dev_mac[QI_ACC_OFFSET5] = data[QI_ASK_PACKET_DAT2];
	g_wltx_acc->dev_version = data[QI_ASK_PACKET_DAT3];
	g_wltx_acc->dev_business = data[QI_ASK_PACKET_DAT4];
	g_wltx_acc->dev_info_cnt += QI_ASK_PACKET_DAT_LEN;

	return 0;
}

/* 0x58 + 0x54 + model1 + model2 + model3 + submodel + checksum */
static int qi_protocol_handle_ask_packet_cmd_0x54(u8 *data)
{
	qi_protocol_send_fsk_ack_msg();

	hwlog_info("ask_packet_cmd_0x54: %u\n", g_wltx_acc->dev_info_cnt);
	/* dev info len 8 cnt */
	if (g_wltx_acc->dev_info_cnt < QI_ASK_PACKET_DAT_LEN * 2) {
		hwlog_info("cmd_0x54 cnt not right\n");
		return -WLC_ERR_CHECK_FAIL;
	}

	/*
	 * solve rx not receive ack from tx, and sustain send ask packet,
	 * but tx data count check fail, reset info count
	 */
	g_wltx_acc->dev_info_cnt = QI_ASK_PACKET_DAT_LEN * 2;
	g_wltx_acc->dev_model_id[QI_ACC_OFFSET0] = data[QI_ASK_PACKET_DAT1];
	g_wltx_acc->dev_model_id[QI_ACC_OFFSET1] = data[QI_ASK_PACKET_DAT2];
	g_wltx_acc->dev_model_id[QI_ACC_OFFSET2] = data[QI_ASK_PACKET_DAT3];
	g_wltx_acc->dev_submodel_id = data[QI_ASK_PACKET_DAT4];
	g_wltx_acc->dev_info_cnt += QI_ASK_PACKET_DAT_LEN;
	g_wltx_acc->dev_state = WL_ACC_DEV_STATE_ONLINE;
	power_event_bnc_notify(POWER_BNT_WLTX_AUX,
		POWER_NE_WLTX_ACC_DEV_CONNECTED, NULL);

	hwlog_info("get acc dev info succ\n");
	return 0;
}

static int qi_protocol_handle_ask_packet_hdr_0x18(u8 *data)
{
	u8 cmd = data[QI_ASK_PACKET_CMD];

	switch (cmd) {
	case QI_CMD_GET_TX_VERSION:
		return qi_protocol_handle_ask_packet_cmd_0x05();
	default:
		hwlog_err("invalid hdr=0x18 cmd=0x%x\n", cmd);
		return -EINVAL;
	}
}

static int qi_protocol_handle_ask_packet_hdr_0x28(u8 *data)
{
	u8 cmd = data[QI_ASK_PACKET_CMD];

	switch (cmd) {
	case QI_CMD_SEND_CHRG_STATE:
		return qi_protocol_handle_ask_packet_cmd_0x43(data);
	default:
		hwlog_err("invalid hdr=0x28 cmd=0x%x\n", cmd);
		return -EINVAL;
	}
}

static int qi_protocol_handle_ask_packet_hdr_0x38(u8 *data)
{
	u8 cmd = data[QI_ASK_PACKET_CMD];

	switch (cmd) {
	case QI_CMD_SET_TX_VIN:
		return qi_protocol_handle_ask_packet_cmd_0x0a(data);
	case QI_CMD_GET_TX_ID:
		return qi_protocol_handle_ask_packet_cmd_0x3b(data);
	default:
		hwlog_err("invalid hdr=0x38 cmd=0x%x\n", cmd);
		return -EINVAL;
	}
}

static int qi_protocol_handle_ask_packet_hdr_0x48(u8 *data)
{
	u8 cmd = data[QI_ASK_PACKET_CMD];

	switch (cmd) {
	default:
		hwlog_err("invalid hdr=0x48 cmd=0x%x\n", cmd);
		return -EINVAL;
	}
}

static int qi_protocol_handle_ask_packet_hdr_0x58(u8 *data)
{
	u8 cmd = data[QI_ASK_PACKET_CMD];

	switch (cmd) {
	case QI_CMD_SEND_BT_MAC1:
		return qi_protocol_handle_ask_packet_cmd_0x52(data);
	case QI_CMD_SEND_BT_MAC2:
		return qi_protocol_handle_ask_packet_cmd_0x53(data);
	case QI_CMD_SEND_BT_MODEL_ID:
		return qi_protocol_handle_ask_packet_cmd_0x54(data);
	default:
		hwlog_err("invalid hdr=0x58 cmd=0x%x\n", cmd);
		return -EINVAL;
	}
}

static int qi_protocol_handle_ask_packet_data(u8 *data, u8 len)
{
	u8 hdr = data[QI_ASK_PACKET_HDR];

	if (len > QI_ASK_PACKET_LEN)
		return -EINVAL;

	switch (hdr) {
	case QI_CMD_GET_SIGNAL_STRENGTH:
		return qi_protocol_handle_ask_packet_cmd_0x01(data);
	case QI_ASK_PACKET_HDR_MSG_SIZE_1_BYTE:
		return qi_protocol_handle_ask_packet_hdr_0x18(data);
	case QI_ASK_PACKET_HDR_MSG_SIZE_2_BYTE:
		return qi_protocol_handle_ask_packet_hdr_0x28(data);
	case QI_ASK_PACKET_HDR_MSG_SIZE_3_BYTE:
		return qi_protocol_handle_ask_packet_hdr_0x38(data);
	case QI_ASK_PACKET_HDR_MSG_SIZE_4_BYTE:
		return qi_protocol_handle_ask_packet_hdr_0x48(data);
	case QI_ASK_PACKET_HDR_MSG_SIZE_5_BYTE:
		return qi_protocol_handle_ask_packet_hdr_0x58(data);
	default:
		hwlog_err("invalid hdr=0x%x\n", hdr);
		return -EINVAL;
	}
}

/*
 * ask: rx to tx
 * we use ask mode when rx sends a message to tx
 */
static int qi_protocol_handle_ask_packet(void)
{
	u8 data[QI_ASK_PACKET_LEN] = { 0 };
	int retry = WIRELESS_RETRY_ONE;

	if (qi_protocol_get_ask_packet(data, QI_ASK_PACKET_LEN, retry))
		return -WLC_ERR_I2C_W;

	if (qi_protocol_handle_ask_packet_data(data, QI_ASK_PACKET_LEN))
		return -WLC_ERR_I2C_W;

	return 0;
}

static bool stwlc68_aux_in_tx_mode(void)
{
	int ret;
	u8 mode = 0;

	ret = stwlc68_aux_read_byte(STWLC68_AUX_OP_MODE_ADDR, &mode);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return false;
	}

	return mode == STWLC68_AUX_TX_MODE;
}

static bool stwlc68_aux_check_rx_disconnect(void)
{
	struct stwlc68_aux_dev_info *di = g_stwlc68_aux_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return true;
	}

	if (di->ept_type & STWLC68_AUX_TX_EPT_SRC_CEP_TIMEOUT) {
		di->ept_type &= ~STWLC68_AUX_TX_EPT_SRC_CEP_TIMEOUT;
		hwlog_info("[%s] RX disconnect\n", __func__);
		return true;
	}

	return false;
}

static int stwlc68_aux_get_tx_ping_interval(u16 *ping_interval)
{
	return stwlc68_aux_read_word(STWLC68_AUX_TX_PING_INTERVAL,
		ping_interval);
}

static int stwlc68_aux_set_tx_ping_interval(u16 ping_interval)
{
	int ret;

	ret = stwlc68_aux_write_word(STWLC68_AUX_TX_PING_INTERVAL,
		ping_interval);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_aux_get_tx_ping_frequency(u16 *ping_freq)
{
	int ret;
	u8 data = 0;

	if (!ping_freq) {
		hwlog_err("%s: ping_freq null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_aux_read_byte(STWLC68_AUX_TX_PING_FREQ_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return ret;
	}
	*ping_freq = (u16)data;

	return 0;
}

static int stwlc68_aux_set_tx_ping_frequency(u16 ping_freq)
{
	int ret;

	if ((ping_freq < STWLC68_AUX_TX_PING_FREQ_MIN) ||
		(ping_freq > STWLC68_AUX_TX_PING_FREQ_MAX)) {
		hwlog_err("%s: ping frequency is out of range\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	ret = stwlc68_aux_write_byte(STWLC68_AUX_TX_PING_FREQ_ADDR,
		(u8)ping_freq);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return ret;
	}

	return ret;
}

static int stwlc68_aux_get_tx_min_fop(u16 *fop)
{
	int ret;
	u8 data = 0;

	if (!fop) {
		hwlog_err("%s: fop null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_aux_read_byte(STWCL68_TX_MIN_FOP_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return ret;
	}
	*fop = (u16)data;

	return 0;
}

static int stwlc68_aux_set_tx_min_fop(u16 fop)
{
	int ret;

	if ((fop < STWLC68_AUX_TX_MIN_FOP_VAL) ||
		(fop > STWLC68_AUX_TX_MAX_FOP_VAL))
		return -WLC_ERR_PARA_WRONG;

	ret = stwlc68_aux_write_byte(STWCL68_TX_MIN_FOP_ADDR, (u8)fop);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_aux_get_tx_max_fop(u16 *fop)
{
	int ret;
	u8 data = 0;

	if (!fop) {
		hwlog_err("%s: fop null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_aux_read_byte(STWLC68_AUX_TX_MAX_FOP_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return ret;
	}
	*fop = (u16)data;

	return 0;
}

static int stwlc68_aux_set_tx_max_fop(u16 fop)
{
	int ret;

	if ((fop < STWLC68_AUX_TX_MIN_FOP_VAL) ||
		(fop > STWLC68_AUX_TX_MAX_FOP_VAL))
		return -WLC_ERR_PARA_WRONG;

	ret = stwlc68_aux_write_byte(STWLC68_AUX_TX_MAX_FOP_ADDR, (u8)fop);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_aux_get_tx_fop(u16 *fop)
{
	return stwlc68_aux_read_word(STWLC68_AUX_TX_OP_FREQ_ADDR, fop);
}

static int stwlc68_aux_get_tx_temp(u8 *chip_temp)
{
	return stwlc68_aux_read_byte(STWLC68_AUX_CHIP_TEMP_ADDR, chip_temp);
}

static int stwlc68_aux_get_tx_vin(u16 *tx_vin)
{
	return stwlc68_aux_read_word(STWLC68_AUX_TX_VIN_ADDR, tx_vin);
}

static int stwlc68_aux_get_tx_vrect(u16 *tx_vrect)
{
	return stwlc68_aux_read_word(STWLC68_AUX_TX_VRECT_ADDR, tx_vrect);
}

static int stwlc68_aux_get_tx_iin(u16 *tx_iin)
{
	return stwlc68_aux_read_word(STWLC68_AUX_TX_IIN_ADDR, tx_iin);
}

static int stwlc68_aux_tx_stop_config(void)
{
	return 0;
}

static int stwlc68_aux_tx_chip_init(void)
{
	int ret;
	struct stwlc68_aux_dev_info *di = g_stwlc68_aux_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	/*
	 * when stwlc68_aux_handle_abnormal_irq disable irq, tx will irq-mask
	 * so, here enable irq
	 */
	stwlc68_aux_enable_irq(di);

	ret = stwlc68_aux_write_byte(STWLC68_AUX_TX_OTP_ADDR,
		STWLC68_AUX_TX_OTP_THRES);
	ret += stwlc68_aux_write_byte(STWLC68_AUX_TX_OCP_ADDR,
		STWLC68_AUX_TX_OCP_VAL / STWLC68_AUX_TX_OCP_STEP);
	ret += stwlc68_aux_write_byte(STWLC68_AUX_TX_OVP_ADDR,
		STWLC68_AUX_TX_OVP_VAL / STWLC68_AUX_TX_OVP_STEP);
	ret += stwlc68_aux_write_word_mask(STWLC68_AUX_CMD_ADDR,
		STWLC68_AUX_CMD_TX_FOD_EN, STWLC68_AUX_CMD_TX_FOD_EN_SHIFT,
		STWLC68_AUX_CMD_VAL);
	ret += stwlc68_aux_set_tx_ping_frequency(STWLC68_AUX_TX_PING_FREQ_INIT);
	ret += stwlc68_aux_set_tx_min_fop(STWLC68_AUX_TX_MIN_FOP_VAL);
	ret += stwlc68_aux_set_tx_max_fop(STWLC68_AUX_TX_MAX_FOP_VAL);
	ret += stwlc68_aux_write_byte(STWLC68_AUX_TX_PLOSS_TH_ADDR, STWLC68_AUX_TX_PLOSS_TH_VAL);
	ret += stwlc68_aux_write_byte(STWLC68_AUX_TX_PLOSS_CNT_ADDR, STWLC68_AUX_TX_PLOSS_CNT_VAL);
	ret += stwlc68_aux_set_tx_ping_interval(di->tx_ping_interval);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static int stwlc68_aux_enable_tx_mode(bool enable)
{
	int ret;

	if (enable)
		ret = stwlc68_aux_write_word_mask(STWLC68_AUX_CMD_ADDR,
			STWLC68_AUX_CMD_TX_EN, STWLC68_AUX_CMD_TX_EN_SHIFT,
			STWLC68_AUX_CMD_VAL);
	else
		ret = stwlc68_aux_write_word_mask(STWLC68_AUX_CMD_ADDR,
			STWLC68_AUX_CMD_TX_DIS, STWLC68_AUX_CMD_TX_DIS_SHIFT,
			STWLC68_AUX_CMD_VAL);

	if (ret) {
		hwlog_err("%s: %s tx mode fail\n", __func__,
			enable ? "enable" : "disable");
		return ret;
	}

	return 0;
}

static int stwlc68_aux_get_tx_ept_type(u16 *ept_type)
{
	int ret;
	u16 data = 0;

	if (!ept_type) {
		hwlog_err("%s: ept_type null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_aux_read_word(STWLC68_AUX_TX_EPT_REASON_RCVD_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return ret;
	}
	*ept_type = data;
	hwlog_info("[%s] EPT type = 0x%04x\n", __func__, *ept_type);
	ret = stwlc68_aux_write_word(STWLC68_AUX_TX_EPT_REASON_RCVD_ADDR, 0);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return ret;
	}

	return 0;
}

static void stwlc68_aux_handle_tx_ept(struct stwlc68_aux_dev_info *di)
{
	int ret;

	ret = stwlc68_aux_get_tx_ept_type(&di->ept_type);
	if (ret) {
		hwlog_err("%s: get tx ept type fail\n", __func__);
		return;
	}
	switch (di->ept_type) {
	case STWLC68_AUX_TX_EPT_SRC_SS:
		di->ept_type &= ~STWLC68_AUX_TX_EPT_SRC_SS;
		hwlog_info("[%s] SS timeout\n", __func__);
		break;
	case STWLC68_AUX_TX_EPT_SRC_ID:
		di->ept_type &= ~STWLC68_AUX_TX_EPT_SRC_ID;
		hwlog_info("[%s] ID packet error\n", __func__);
		break;
	case STWLC68_AUX_TX_EPT_SRC_XID:
		di->ept_type &= ~STWLC68_AUX_TX_EPT_SRC_XID;
		hwlog_info("[%s] XID packet error\n", __func__);
		break;
	case STWLC68_AUX_TX_EPT_SRC_CFG_CNT_ERR:
		di->ept_type &= ~STWLC68_AUX_TX_EPT_SRC_CFG_CNT_ERR;
		hwlog_info("[%s] wrong CFG optional pkt number\n", __func__);
		break;
	case STWLC68_AUX_TX_EPT_SRC_CFG_ERR:
		di->ept_type &= ~STWLC68_AUX_TX_EPT_SRC_CFG_ERR;
		hwlog_info("[%s] CFG pkt error\n", __func__);
		break;
	case STWLC68_AUX_TX_EPT_SRC_RPP_TIMEOUT:
		di->ept_type &= ~STWLC68_AUX_TX_EPT_SRC_RPP_TIMEOUT;
		hwlog_info("[%s] RPP pkt timeout\n", __func__);
		break;
	case STWLC68_AUX_TX_EPT_SRC_CEP_TIMEOUT:
		di->ept_type &= ~STWLC68_AUX_TX_EPT_SRC_CEP_TIMEOUT;
		hwlog_info("[%s] RX disconnect\n", __func__);
		power_event_bnc_notify(POWER_BNT_WLTX_AUX,
			POWER_NE_WLTX_CEP_TIMEOUT, NULL);
		break;
	case STWLC68_AUX_TX_EPT_SRC_FOD:
		di->ept_type &= ~STWLC68_AUX_TX_EPT_SRC_FOD;
		hwlog_info("[%s] fod happened\n", __func__);
		power_event_bnc_notify(POWER_BNT_WLTX_AUX,
			POWER_NE_WLTX_TX_FOD, NULL);
		break;
	default:
		break;
	}
}

static int stwlc68_aux_clear_tx_interrupt(u16 itr)
{
	int ret;

	ret = stwlc68_aux_write_word(STWLC68_AUX_TX_INTR_CLR_ADDR, itr);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static void stwlc68_aux_tx_mode_irq_handler(struct stwlc68_aux_dev_info *di)
{
	int ret;
	int i;
	u16 irq_value = 0;

	ret = stwlc68_aux_read_word(STWLC68_AUX_TX_INTR_LATCH_ADDR,
		&di->irq_val);
	if (ret) {
		hwlog_err("%s: read interrupt fail, clear\n", __func__);
		stwlc68_aux_clear_tx_interrupt(STWLC68_AUX_ALL_INTR_CLR);
		goto clr_irq_again;
	}

	hwlog_info("[%s] interrupt 0x%04x\n", __func__, di->irq_val);
	if (di->irq_val == STWLC68_AUX_ABNORMAL_INTR) {
		hwlog_err("%s: abnormal interrupt\n", __func__);
		stwlc68_aux_clear_tx_interrupt(STWLC68_AUX_ALL_INTR_CLR);
		goto clr_irq_again;
	}

	stwlc68_aux_clear_tx_interrupt(di->irq_val);

	if (di->irq_val & STWLC68_AUX_TX_START_PING_LATCH) {
		hwlog_info("%s: tx PING interrupt\n", __func__);
		di->irq_val &= ~STWLC68_AUX_TX_START_PING_LATCH;
		power_event_bnc_notify(POWER_BNT_WLTX_AUX,
			POWER_NE_WLTX_PING_RX, NULL);
	}

	if (di->irq_val & STWLC68_AUX_TX_SS_PKG_RCVD_LATCH) {
		hwlog_info("%s: Signal Strength packet interrupt\n", __func__);
		di->irq_val &= ~STWLC68_AUX_TX_SS_PKG_RCVD_LATCH;
		qi_protocol_handle_ask_packet();
	}

	if (di->irq_val & STWLC68_AUX_TX_ID_PCKET_RCVD_LATCH) {
		hwlog_info("%s: ID packet interrupt\n", __func__);
		di->irq_val &= ~STWLC68_AUX_TX_ID_PCKET_RCVD_LATCH;
		qi_protocol_handle_ask_packet();
	}

	if (di->irq_val & STWLC68_AUX_TX_CFG_PKT_RCVD_LATCH) {
		hwlog_info("%s: Config packet interrupt\n", __func__);
		di->irq_val &= ~STWLC68_AUX_TX_CFG_PKT_RCVD_LATCH;
		qi_protocol_handle_ask_packet();
		power_event_bnc_notify(POWER_BNT_WLTX_AUX,
			POWER_NE_WLTX_GET_CFG, NULL);
	}
	if (di->irq_val & STWLC68_AUX_TX_EPT_PKT_RCVD_LATCH) {
		di->irq_val &= ~STWLC68_AUX_TX_EPT_PKT_RCVD_LATCH;
		stwlc68_aux_handle_tx_ept(di);
	}

	if (di->irq_val & STWLC68_AUX_TX_PP_PKT_RCVD_LATCH) {
		hwlog_info("[%s] TX rcv personal property ASK pkt\n", __func__);
		di->irq_val &= ~STWLC68_AUX_TX_PP_PKT_RCVD_LATCH;
		qi_protocol_handle_ask_packet();
	}

	if (di->irq_val & STWLC68_AUX_TX_OVTP_INTR_LATCH) {
		di->irq_val &= ~STWLC68_AUX_TX_OVTP_INTR_LATCH;
		hwlog_info("[%s] TX_OVTP_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_AUX_TX_OCP_INTR_LATCH) {
		di->irq_val &= ~STWLC68_AUX_TX_OCP_INTR_LATCH;
		hwlog_info("[%s] TX_OCP_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_AUX_TX_OVP_INTR_LATCH) {
		di->irq_val &= ~STWLC68_AUX_TX_OVP_INTR_LATCH;
		hwlog_info("[%s] TX_OVP_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_AUX_TX_SYS_ERR_INTR_LATCH) {
		di->irq_val &= ~STWLC68_AUX_TX_SYS_ERR_INTR_LATCH;
		hwlog_info("[%s] TX_SYS_ERR_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_AUX_TX_EXT_MON_INTR_LATCH) {
		di->irq_val &= ~STWLC68_AUX_TX_EXT_MON_INTR_LATCH;
		hwlog_info("rcv dping in tx mode\n");
		power_event_bnc_notify(POWER_BNT_WLTX_AUX,
			POWER_NE_WLTX_RCV_DPING, NULL);
	}

	if (di->irq_val & STWLC68_AUX_TX_SEND_PKT_SUCC_LATCH) {
		di->irq_val &= ~STWLC68_AUX_TX_SEND_PKT_SUCC_LATCH;
		hwlog_info("[%s] TX_SEND_PKT_SUCC_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_AUX_TX_SEND_PKT_TIMEOUT_LATCH) {
		di->irq_val &= ~STWLC68_AUX_TX_SEND_PKT_TIMEOUT_LATCH;
		hwlog_info("[%s] TX_SEND_PKT_TIMEOUT_LATCH\n", __func__);
	}
	if (di->irq_val & STWLC68_AUX_TX_RP_DM_TIMEOUT_LATCH) {
		di->irq_val &= ~STWLC68_AUX_TX_RP_DM_TIMEOUT_LATCH;
		hwlog_info("[%s] TX_RP_DM_TIMEOUT_LATCH\n", __func__);
		power_event_bnc_notify(POWER_BNT_WLTX_AUX,
			POWER_NE_WLTX_RP_DM_TIMEOUT, NULL);
	}

clr_irq_again:
	for (i = 0; (!gpio_get_value(di->gpio_int)) && (i < STWLC68_AUX_TX_INTR_CLR_CNT); i++) {
		stwlc68_aux_read_word(STWLC68_AUX_TX_INTR_LATCH_ADDR,
			&irq_value);
		hwlog_info("[tx_irq_handler] gpio_int low, clear irq cnt=%d irq=0x%x\n",
			i, irq_value);
		stwlc68_aux_clear_tx_interrupt(STWLC68_AUX_ALL_INTR_CLR);
		power_usleep(DT_USLEEP_1MS); /* delay for gpio int pull up */
	}
}

static void stwlc68_aux_irq_work(struct work_struct *work)
{
	int ret;
	int gpio_val;
	u8 mode = 0;
	struct stwlc68_aux_dev_info *di = container_of(work,
		struct stwlc68_aux_dev_info, irq_work);

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		goto exit;
	}

	gpio_val = gpio_get_value(di->gpio_en);
	if (gpio_val != di->gpio_en_valid_val) {
		hwlog_err("[%s] gpio is %s now\n",
		__func__, gpio_val ? "high" : "low");
		goto exit;
	}
	gpio_val = gpio_get_value(di->gpio_int);
	if (gpio_val != STWLC68_AUX_IRQ_INT_VALID_VAL) {
		hwlog_err("[%s] gpio_int is %s now\n",
			__func__, gpio_val ? "high" : "low");
		goto exit;
	}
	/* get System Operating Mode */
	ret = stwlc68_aux_get_mode(&mode);
	if (!ret)
		hwlog_info("[%s] mode = 0x%x\n", __func__, mode);

	/* handler interrupt */
	if ((mode == STWLC68_AUX_TX_MODE) ||
		(mode == STWLC68_AUX_STANDALONE_MODE))
		stwlc68_aux_tx_mode_irq_handler(di);

exit:
	stwlc68_aux_enable_irq(di);
	stwlc68_aux_wake_unlock();
}

static irqreturn_t stwlc68_aux_interrupt(int irq, void *_di)
{
	struct stwlc68_aux_dev_info *di = _di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return IRQ_HANDLED;
	}

	hwlog_info("[%s] ++\n", __func__);
	stwlc68_aux_wake_lock();
	if (di->irq_active) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = 0;
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("irq is not enable,do nothing\n");
		stwlc68_aux_wake_unlock();
	}
	hwlog_info("[%s] --\n", __func__);

	return IRQ_HANDLED;
}

static void stwlc68_aux_sram_scan_work(struct work_struct *work)
{
	struct stwlc68_aux_dev_info *di = container_of(work,
		struct stwlc68_aux_dev_info, irq_work);

	if (power_cmdline_is_factory_mode()) {
		g_sram_chk_complete = true;
		return;
	}

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	hwlog_info("[%s] sram scan begin\n", __func__);
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, true);
	(void)power_usleep(DT_USLEEP_10MS);
	stwlc68_aux_fw_ram_check();
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
	hwlog_info("[%s] sram scan end\n", __func__);

	g_sram_chk_complete = true;
}

static int stwlc68_aux_dev_check(void)
{
	int ret;
	u16 chip_id = 0;
	struct stwlc68_aux_dev_info *di = g_stwlc68_aux_di;

	if (!di)
		return -EINVAL;

	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, true);
	(void)power_usleep(DT_USLEEP_10MS);
	g_sram_chk_complete = true;
	ret = stwlc68_aux_get_chip_id(&chip_id);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
		g_sram_chk_complete = false;
		return ret;
	}
	g_sram_chk_complete = false;
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);

	hwlog_info("[%s] chip_id = %u\n", __func__, chip_id);

	if (chip_id != STWLC68_AUX_CHIP_ID)
		hwlog_err("%s: wlc_chip not match\n", __func__);

	return 0;
}

static int stwlc68_aux_parse_dts(struct device_node *np,
	struct stwlc68_aux_dev_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_en_valid_val", (u32 *)&di->gpio_en_valid_val, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ping_interval", (u32 *)&di->tx_ping_interval,
		STWLC68_AUX_TX_PING_INTERVAL_INIT);

	return 0;
}

static int stwlc68_aux_gpio_init(struct stwlc68_aux_dev_info *di,
	struct device_node *np)
{
	int ret;

	if (!di || !np)
		return -EINVAL;

	/* gpio_en */
	if (power_gpio_config_output(np,
		"gpio_en", "stwlc68_aux_en",
		&di->gpio_en, di->gpio_en_valid_val)) {
		ret = -EINVAL;
		goto gpio_en_fail;
	}

	return 0;

gpio_en_fail:
	gpio_free(di->gpio_en);
	return ret;
}

static int stwlc68_aux_irq_init(struct stwlc68_aux_dev_info *di,
	struct device_node *np)
{
	int ret;

	if (!di || !np)
		return -EINVAL;

	if (power_gpio_config_interrupt(np,
		"gpio_int", "stwlc68_aux_int", &di->gpio_int, &di->irq_int)) {
		ret = -EINVAL;
		goto irq_init_fail_0;
	}

	ret = request_irq(di->irq_int, stwlc68_aux_interrupt,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "stwlc68_aux_irq", di);
	if (ret) {
		hwlog_err("could not request stwlc68_aux_irq\n");
		di->irq_int = -EINVAL;
		goto irq_init_fail_1;
	}
	enable_irq_wake(di->irq_int);
	di->irq_active = 1;
	INIT_WORK(&di->irq_work, stwlc68_aux_irq_work);

	return 0;

irq_init_fail_1:
	gpio_free(di->gpio_int);
irq_init_fail_0:
	return ret;
}

/* aux acc */
static int stwlc68_aux_get_tx_acc_dev_no(u8 *dev_no)
{
	if (!dev_no || !g_wltx_acc) {
		hwlog_err("%s para is null\n", __func__);
		return -EINVAL;
	}

	*dev_no = g_wltx_acc->dev_no;
	hwlog_info("%s, dev_no = %u\n", __func__, *dev_no);
	return 0;
}

static int stwlc68_aux_get_tx_acc_dev_state(u8 *dev_state)
{
	if (!dev_state || !g_wltx_acc) {
		hwlog_err("%s para is null\n", __func__);
		return -EINVAL;
	}

	*dev_state = g_wltx_acc->dev_state;
	hwlog_info("%s, dev_state = %u\n", __func__, *dev_state);
	return 0;
}

static int stwlc68_aux_get_tx_acc_dev_mac(u8 *dev_mac)
{
	int i;

	if (!dev_mac || !g_wltx_acc) {
		hwlog_err("%s para is null\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < WL_TX_ACC_DEV_MAC_LEN; i++) {
		dev_mac[i] = g_wltx_acc->dev_mac[i];
		hwlog_info("dev_mac[%d] = 0x%02x\n", i, dev_mac[i]);
	}
	return 0;
}

static int stwlc68_aux_get_tx_acc_dev_model_id(u8 *dev_model_id)
{
	int i;

	if (!dev_model_id || !g_wltx_acc) {
		hwlog_err("%s para is null\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < WL_TX_ACC_DEV_MODELID_LEN; i++) {
		dev_model_id[i] = g_wltx_acc->dev_model_id[i];
		hwlog_info("dev_model_id[%d] = 0x%02x\n", i, dev_model_id[i]);
	}
	return 0;
}

static int stwlc68_aux_get_tx_acc_dev_submodel_id(u8 *dev_submodel_id)
{
	if (!dev_submodel_id || !g_wltx_acc) {
		hwlog_err("%s para is null\n", __func__);
		return -EINVAL;
	}

	*dev_submodel_id = g_wltx_acc->dev_submodel_id;
	hwlog_info("%s, dev_submodel_id = 0x%02x\n",
		__func__, *dev_submodel_id);
	return 0;
}

static int stwlc68_aux_get_tx_acc_dev_version(u8 *dev_version)
{
	if (!dev_version || !g_wltx_acc) {
		hwlog_err("%s para is null\n", __func__);
		return -EINVAL;
	}

	*dev_version = g_wltx_acc->dev_version;
	hwlog_info("%s, dev_version = 0x%02x\n", __func__, *dev_version);
	return 0;
}

static int stwlc68_aux_get_tx_acc_dev_business(u8 *dev_business)
{
	if (!dev_business || !g_wltx_acc) {
		hwlog_err("%s para is null\n", __func__);
		return -EINVAL;
	}

	*dev_business = g_wltx_acc->dev_business;
	hwlog_info("%s, dev_business = 0x%02x\n", __func__, *dev_business);
	return 0;
}

static int stwlc68_aux_set_tx_acc_dev_state(u8 dev_state)
{
	if (!g_wltx_acc)
		return -EINVAL;

	g_wltx_acc->dev_state = dev_state;
	hwlog_info("%s, dev_state = %u\n", __func__, dev_state);
	return 0;
}

static int stwlc68_aux_get_tx_acc_dev_info_cnt(u8 *dev_info_cnt)
{
	if (!dev_info_cnt || !g_wltx_acc) {
		hwlog_err("%s para is null\n", __func__);
		return -EINVAL;
	}

	*dev_info_cnt = g_wltx_acc->dev_info_cnt;
	hwlog_info("%s, dev_info_cnt = %u\n", __func__, *dev_info_cnt);
	return 0;
}

static int stwlc68_aux_set_tx_acc_dev_info_cnt(u8 dev_info_cnt)
{
	if (!g_wltx_acc)
		return -EINVAL;

	g_wltx_acc->dev_info_cnt = dev_info_cnt;
	hwlog_info("set dev_info_cnt = %u\n", dev_info_cnt);
	return 0;
}

static int stwlc68_aux_enable_tx_ping(void)
{
	return 0;
}

static int stwlc68_aux_tx_adjust_vin(u16 iin_avg)
{
	return 0;
}

static struct wltx_dev_ops stwlc68_aux_tx_ops = {
	.rx_enable                  = stwlc68_aux_chip_enable,
	.chip_reset                 = stwlc68_aux_chip_reset,
	.enable_tx_mode             = stwlc68_aux_enable_tx_mode,
	.enable_tx_ping             = stwlc68_aux_enable_tx_ping,
	.tx_chip_init               = stwlc68_aux_tx_chip_init,
	.tx_stop_config             = stwlc68_aux_tx_stop_config,
	.check_fwupdate             = stwlc68_aux_check_fwupdate,
	.get_tx_iin                 = stwlc68_aux_get_tx_iin,
	.get_tx_vrect               = stwlc68_aux_get_tx_vrect,
	.get_tx_vin                 = stwlc68_aux_get_tx_vin,
	.get_chip_temp              = stwlc68_aux_get_tx_temp,
	.get_tx_fop                 = stwlc68_aux_get_tx_fop,
	.set_tx_max_fop             = stwlc68_aux_set_tx_max_fop,
	.get_tx_max_fop             = stwlc68_aux_get_tx_max_fop,
	.set_tx_min_fop             = stwlc68_aux_set_tx_min_fop,
	.get_tx_min_fop             = stwlc68_aux_get_tx_min_fop,
	.set_tx_ping_frequency      = stwlc68_aux_set_tx_ping_frequency,
	.get_tx_ping_frequency      = stwlc68_aux_get_tx_ping_frequency,
	.set_tx_ping_interval       = stwlc68_aux_set_tx_ping_interval,
	.get_tx_ping_interval       = stwlc68_aux_get_tx_ping_interval,
	.check_rx_disconnect        = stwlc68_aux_check_rx_disconnect,
	.in_tx_mode                 = stwlc68_aux_in_tx_mode,
	.set_tx_open_flag           = NULL,
	.get_tx_acc_dev_no          = stwlc68_aux_get_tx_acc_dev_no,
	.get_tx_acc_dev_state       = stwlc68_aux_get_tx_acc_dev_state,
	.get_tx_acc_dev_mac         = stwlc68_aux_get_tx_acc_dev_mac,
	.get_tx_acc_dev_model_id    = stwlc68_aux_get_tx_acc_dev_model_id,
	.get_tx_acc_dev_submodel_id = stwlc68_aux_get_tx_acc_dev_submodel_id,
	.get_tx_acc_dev_version     = stwlc68_aux_get_tx_acc_dev_version,
	.get_tx_acc_dev_business    = stwlc68_aux_get_tx_acc_dev_business,
	.set_tx_acc_dev_state       = stwlc68_aux_set_tx_acc_dev_state,
	.get_tx_acc_dev_info_cnt    = stwlc68_aux_get_tx_acc_dev_info_cnt,
	.set_tx_acc_dev_info_cnt    = stwlc68_aux_set_tx_acc_dev_info_cnt,
	.tx_adjust_vin              =  stwlc68_aux_tx_adjust_vin,
};

static struct wireless_fw_ops g_stwlc68_aux_fw_ops = {
	.ic_name                   = "stwlc68_aux",
	.program_fw                = stwlc68_aux_rx_program_otp,
	.get_fw_status             = stwlc68_aux_get_otp_status,
	.check_fw                  = stwlc68_aux_check_otp,
};

static void stwlc68_aux_shutdown(struct i2c_client *client)
{
}

static int stwlc68_aux_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret = -EINVAL;
	u16 chip_id = 0;
	struct stwlc68_aux_dev_info *di = NULL;
	struct device_node *np = NULL;
	struct power_devices_info_data *power_dev_info = NULL;
	struct wltx_acc_dev *tx_acc_dev_di = NULL;

	if (!client || !client->dev.of_node)
		return -ENODEV;

	tx_acc_dev_di = kzalloc(sizeof(*tx_acc_dev_di), GFP_KERNEL);
	if (!tx_acc_dev_di)
		return -ENOMEM;

	g_wltx_acc = tx_acc_dev_di;
	g_wltx_acc->dev_no = ACC_DEV_NO_PEN;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		goto tx_acc_dev_fail;

	g_stwlc68_aux_di = di;
	di->dev = &client->dev;
	np = client->dev.of_node;
	di->client = client;
	di->ic_type = WLTRX_IC_AUX;
	i2c_set_clientdata(client, di);

	ret = stwlc68_aux_dev_check();
	if (ret)
		goto dev_ck_fail;

	ret = stwlc68_aux_parse_dts(np, di);
	if (ret)
		goto parse_dts_fail;

	ret = stwlc68_aux_gpio_init(di, np);
	if (ret)
		goto gpio_init_fail;

	ret = stwlc68_aux_irq_init(di, np);
	if (ret)
		goto irq_init_fail;

	wakeup_source_init(&g_stwlc68_aux_wakelock, "stwlc68_aux_wakelock");
	mutex_init(&di->mutex_irq);

	ret = wltx_aux_ops_register(&stwlc68_aux_tx_ops);
	if (ret) {
		hwlog_err("probe: register tx_ops fail\n");
		goto ops_regist_fail;
	}
	g_stwlc68_aux_fw_ops.dev_data = (void *)di;
	ret = wireless_fw_ops_register(&g_stwlc68_aux_fw_ops);
	if (ret) {
		hwlog_err("probe: register fw_ops fail\n");
		goto ops_regist_fail;
	}

	stwlc68_aux_get_chip_id(&chip_id);
	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = chip_id;
		power_dev_info->ver_id = 0;
	}
	if (!power_cmdline_is_powerdown_charging_mode()) {
		INIT_DELAYED_WORK(&di->sram_scan_work,
		stwlc68_aux_sram_scan_work);
		schedule_delayed_work(&di->sram_scan_work,
			msecs_to_jiffies(5000)); /* delay for wireless probe */
	} else {
		g_sram_chk_complete = true;
	}
	di->g_val.qi_hdl = qi_protocol_get_handle();

	hwlog_info("wireless_stwlc68_aux probe ok\n");
	return 0;

ops_regist_fail:
	gpio_free(di->gpio_int);
	free_irq(di->irq_int, di);
	wakeup_source_trash(&g_stwlc68_aux_wakelock);
	mutex_destroy(&di->mutex_irq);
irq_init_fail:
	gpio_free(di->gpio_en);
gpio_init_fail:
parse_dts_fail:
dev_ck_fail:
	devm_kfree(&client->dev, di);
tx_acc_dev_fail:
	kfree(tx_acc_dev_di);
	di = NULL;
	g_stwlc68_aux_di = NULL;
	np = NULL;
	return ret;
}

MODULE_DEVICE_TABLE(i2c, wireless_stwlc68_aux);
static const struct of_device_id stwlc68_aux_of_match[] = {
	{
		.compatible = "huawei, wireless_stwlc68_pencil",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id stwlc68_aux_i2c_id[] = {
	{ "wireless_stwlc68_aux", 0 }, {}
};

static struct i2c_driver stwlc68_aux_driver = {
	.probe = stwlc68_aux_probe,
	.shutdown = stwlc68_aux_shutdown,
	.id_table = stwlc68_aux_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "wireless_stwlc68_aux",
		.of_match_table = of_match_ptr(stwlc68_aux_of_match),
	},
};

static int __init stwlc68_aux_init(void)
{
	return i2c_add_driver(&stwlc68_aux_driver);
}

static void __exit stwlc68_aux_exit(void)
{
	i2c_del_driver(&stwlc68_aux_driver);
}

device_initcall(stwlc68_aux_init);
module_exit(stwlc68_aux_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("stwlc68_aux module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
