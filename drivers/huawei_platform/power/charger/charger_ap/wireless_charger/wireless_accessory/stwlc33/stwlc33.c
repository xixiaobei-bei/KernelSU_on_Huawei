/*
 * stwlc33.c
 *
 * stwlc33 chip driver
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
#include <huawei_platform/power/huawei_charger.h>
#ifdef CONFIG_BCI_BATTERY
#include <linux/power/hisi/bci_battery.h>
#endif
#include <chipset_common/hwpower/wireless_charge/wireless_acc_types.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <huawei_platform/power/wireless/wireless_charger.h>
#include <huawei_platform/power/wireless/wireless_transmitter_aux.h>
#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <stwlc33.h>
#include <stwlc33_fw_sram.h>
#ifdef WIRELESS_CHARGER_FACTORY_VERSION
#include <stwlc33_fw_nvm.h>
#endif

#define HWLOG_TAG wireless_stwlc33
HWLOG_REGIST();

static struct stwlc33_device_info *g_stwlc33_di;
static struct wakeup_source g_stwlc33_wakelock;
static int g_support_idt_wlc;
static int g_support_st_wlc;
static int g_i2c_fail_cnt;
static u16 stwlc33_iout_adc_val;
static int g_irq_abnormal_flag;
static const u8 g_stwlc33_send_msg_len[STWLC33_RX_TO_TX_PACKET_LEN] = {
	0, 0x18, 0x28, 0x38, 0x48, 0x58
};
static const u8 g_stwlc33_send_fsk_msg_len[STWLC33_RX_TO_TX_PACKET_LEN] = {
	0, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f
};
#ifdef WIRELESS_CHARGER_FACTORY_VERSION
static const u8 *g_latest_otp_version;
static u8 g_st_nvm_bufs[STWLC33_NVM_SEC_VAL_SIZE];
#endif /* WIRELESS_CHARGER_FACTORY_VERSION */

static struct wltx_acc_dev *g_wltx_acc;

static int stwlc33_read_block(struct stwlc33_device_info *di,
	u16 reg, u8 *data, u8 len)
{
	struct i2c_msg msg[2];
	int ret;
	int i;
	u8 reg_be[STWLC33_ADDR_LEN];

	reg_be[0] = reg >> BITS_PER_BYTE;
	reg_be[1] = reg & BYTE_MASK;

	if (!di->client->adapter)
		return -ENODEV;

	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = reg_be;
	msg[0].len = STWLC33_ADDR_LEN;

	msg[1].addr = di->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = data;
	msg[1].len = len;

	if ((di->i2c_trans_fail_limit > 0) &&
		(g_i2c_fail_cnt >= di->i2c_trans_fail_limit) &&
		(reg != STWLC33_SYS_MODE_ADDR) &&
		(reg != STWLC33_RX_INT_STATUS_ADDR) &&
		(reg != STWLC33_TX_VIN_ADDR)) {
		hwlog_err("%s: i2c err cnt = %d\n",
			__func__, g_i2c_fail_cnt);
		return -1;
	}
	for (i = 0; i < I2C_RETRY_CNT; i++) {
		/* msg len = 2 */
		ret = i2c_transfer(di->client->adapter, msg, 2);
		if (ret >= 0)
			break;
		++g_i2c_fail_cnt;
		usleep_range(9500, 10500); /* 10ms */
	}

	if (ret < 0) {
		hwlog_err("%s:stwlc33 read block fail, start_reg = 0x%x\n",
			__func__, reg);
		return -1;
	}
	g_i2c_fail_cnt = 0;

	return 0;
}

static int stwlc33_write_block(struct stwlc33_device_info *di,
	u16 reg, u8 *buff, u8 len)
{
	struct i2c_msg msg;
	int ret;
	int i;

	if (!di->client->adapter)
		return -ENODEV;

	buff[0] = reg >> BITS_PER_BYTE;
	buff[1] = reg & BYTE_MASK;

	msg.buf = buff;
	msg.addr = di->client->addr;
	msg.flags = 0;
	msg.len = len + STWLC33_ADDR_LEN;

	if ((di->i2c_trans_fail_limit > 0) &&
		(g_i2c_fail_cnt >= di->i2c_trans_fail_limit) &&
		(reg != STWLC33_RX_INT_STATUS_ADDR)) {
		hwlog_err("%s: i2c err cnt = %d\n",
			__func__, g_i2c_fail_cnt);
		return -1;
	}
	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = i2c_transfer(di->client->adapter, &msg, 1);
		if (ret >= 0)
			break;
		++g_i2c_fail_cnt;
		usleep_range(9500, 10500); /* 10ms */
	}

	if (ret != 1) {
		hwlog_err("%s:stwlc33 write block fail, start_reg = 0x%x\n",
			__func__, reg);
		return -1;
	}
	g_i2c_fail_cnt = 0;

	return 0;
}

static int stwlc33_read_byte(u16 reg, u8 *data)
{
	struct stwlc33_device_info *di = g_stwlc33_di;

	if (!di) {
		hwlog_err("%s:id is null\n", __func__);
		return -1;
	}

	return stwlc33_read_block(di, reg, data, BYTE_LEN);
}

static int stwlc33_read_word(u16 reg, u16 *data)
{
	struct stwlc33_device_info *di = g_stwlc33_di;
	u8 buff[WORD_LEN] = { 0 };
	int ret;

	if (!di) {
		hwlog_err("%s:id is null\n", __func__);
		return -1;
	}
	ret = stwlc33_read_block(di, reg, buff, WORD_LEN);
	if (ret)
		return -1;

	*data = (buff[0] | (buff[1] << BITS_PER_BYTE));
	return 0;
}

static int stwlc33_write_byte(u16 reg, u8 data)
{
	struct stwlc33_device_info *di = g_stwlc33_di;
	u8 buff[STWLC33_ADDR_LEN + BYTE_LEN] = { 0 };

	if (!di) {
		hwlog_err("%s:id is null\n", __func__);
		return -1;
	}

	buff[STWLC33_ADDR_LEN] = data;
	return stwlc33_write_block(di, reg, buff, BYTE_LEN);
}

static int stwlc33_write_word(u16 reg, u16 data)
{
	struct stwlc33_device_info *di = g_stwlc33_di;
	u8 buff[STWLC33_ADDR_LEN + WORD_LEN] = { 0 };

	if (!di) {
		hwlog_err("%s:id is null\n", __func__);
		return -1;
	}

	buff[STWLC33_ADDR_LEN] = data & BYTE_MASK;
	buff[STWLC33_ADDR_LEN + 1] = data >> BITS_PER_BYTE;

	return stwlc33_write_block(di, reg, buff, 2); /* msg len = 2 */
}

static int stwlc33_write_word_mask(u16 reg, u16 mask, u16 shift, u16 data)
{
	int ret;
	u16 val = 0;

	ret = stwlc33_read_word(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	ret = stwlc33_write_word(reg, val);

	return ret;
}

static void stwlc33_wake_lock(void)
{
	if (!g_stwlc33_wakelock.active) {
		__pm_stay_awake(&g_stwlc33_wakelock);
		hwlog_info("%s:wake lock\n", __func__);
	}
}

static void stwlc33_wake_unlock(void)
{
	if (g_stwlc33_wakelock.active) {
		__pm_relax(&g_stwlc33_wakelock);
		hwlog_info("%s:wake unlock\n", __func__);
	}
}

static void stwlc33_enable_irq(struct stwlc33_device_info *di)
{
	mutex_lock(&di->mutex_irq);
	if (di->irq_active == 0) {
		enable_irq(di->irq_int);
		di->irq_active = 1;
	}
	mutex_unlock(&di->mutex_irq);
}

static void stwlc33_chip_enable(int enable)
{
	struct stwlc33_device_info *di = g_stwlc33_di;

	if (di) {
		if (enable == RX_EN_ENABLE)
			gpio_set_value(di->gpio_en, di->gpio_en_valid_val);
		else
			gpio_set_value(di->gpio_en, !di->gpio_en_valid_val);

		hwlog_info("gpio value is %d now\n",
			gpio_get_value(di->gpio_en));
	}
}

static int stwlc33_chip_reset(void)
{
	int ret;

	ret = stwlc33_write_byte(STWLC33_CHIP_RST_ADDR,
		STWLC33_CHIP_RST_VAL);
	if (ret) {
		hwlog_err("%s fail\n", __func__);
		return ret;
	}
	return 0;
}

#ifdef WIRELESS_CHARGER_FACTORY_VERSION
static char *stwlc33_nvm_read(int sec_no)
{
	int i;
	int ret;
	u16 cmd_status = 0;
	u8 nvm_sec_val[STWLC33_NVM_REG_SIZE] = { 0 };
	static char nvm_buf[STWLC33_NVM_VALUE_SIZE];
	char temp[STWLC33_NVM_SEC_VAL_SIZE] = { 0 };
	u8 sec_val[STWLC33_SEC_NO_SIZE] = { 0 };

	if (!g_stwlc33_di) {
		hwlog_err("%s: g_stwlc33_di is null\n", __func__);
		return "error";
	}
	if ((sec_no < 0) || (sec_no > STWLC33_NVM_SEC_NO_MAX)) {
		hwlog_err("%s: nvm sec no is invalid\n", __func__);
		return "error";
	}
	memset(nvm_buf, 0, STWLC33_NVM_VALUE_SIZE);
	/* Step 1 : check for zero, to confirm pre cmds completed */
	ret = stwlc33_read_word(STWLC33_CMD_STATUS_ADDR, &cmd_status);
	if (ret) {
		hwlog_err("%s: get cmd status failed\n", __func__);
		return "error";
	}
	if (cmd_status) {
		hwlog_err("%s: pre cmds not completed [0x%x]\n",
			__func__, cmd_status);
		return "error";
	}
	/* Step 2 : Update I2C sector no */
	sec_val[0] = sec_no;
	memcpy(nvm_sec_val + STWLC33_ADDR_LEN, sec_val,
		STWLC33_SEC_NO_SIZE);
	ret = stwlc33_write_block(g_stwlc33_di, STWLC33_OFFSET_REG_ADDR,
		nvm_sec_val, STWLC33_SEC_NO_SIZE);
	if (ret) {
		hwlog_err("%s: write sector no fail\n", __func__);
		return "error";
	}
	/* Step 3 : Update I2C sector value length */
	ret = stwlc33_write_byte(STWLC33_SRAM_SIZE_ADDR,
		STWLC33_NVM_SEC_VAL_SIZE - 1);
	if (ret) {
		hwlog_err("%s: write sector length fail\n", __func__);
		return "error";
	}
	/* Step 4 : Write I2C nvm read cmd val */
	ret = stwlc33_write_byte(STWLC33_CMD_STATUS_ADDR, STWLC33_NVM_RD_CMD);
	if (ret) {
		hwlog_err("%s: write nvm read cmd fail\n", __func__);
		return "error";
	}
	/* Step 5 : check for zero, to confirm pre cmds completed */
	ret = stwlc33_read_word(STWLC33_CMD_STATUS_ADDR, &cmd_status);
	if (ret) {
		hwlog_err("%s: get cmd status failed\n", __func__);
		return "error";
	}
	if (cmd_status) {
		hwlog_err("%s: pre cmds not completed [0x%x]\n",
			__func__, cmd_status);
		return "error";
	}
	/* Step 6 : Read back corresponding NVM sector content */
	ret = stwlc33_read_block(g_stwlc33_di, STWLC33_NVM_RD_ADDR,
		g_st_nvm_bufs, STWLC33_NVM_SEC_VAL_SIZE);
	if (ret) {
		hwlog_err("%s: read NVM data failed\n", __func__);
		return "error";
	}
	for (i = 0; i < STWLC33_NVM_SEC_VAL_SIZE; i++) {
		snprintf(temp, STWLC33_NVM_SEC_VAL_SIZE, "%x ",
			g_st_nvm_bufs[i]);
		strncat(nvm_buf, temp, strlen(temp));
	}

	return nvm_buf;
}

static void stwlc33_disable_irq_nosync(struct stwlc33_device_info *di)
{
	mutex_lock(&di->mutex_irq);
	if (di->irq_active == 1) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = 0;
	}
	mutex_unlock(&di->mutex_irq);
}

static int stwlc33_check_nvm(void)
{
	int i;
	int j;
	int nvm_update_flag = STWLC33_NVM_PROGRAMED;
	u8 *nvm_fw_data = NULL;

	for (i = 0; i < ARRAY_SIZE(g_st_fw_nvm_data); i++) {
		if ((g_st_fw_nvm_data[i].sec_size !=
			STWLC33_NVM_SEC_VAL_SIZE) ||
			(g_st_fw_nvm_data[i].sec_no < 0) ||
			(g_st_fw_nvm_data[i].sec_no > STWLC33_NVM_SEC_NO_MAX)) {
			hwlog_err("%s: nvm sec no is invalid\n", __func__);
			return -EINVAL;
		}
		nvm_fw_data = stwlc33_nvm_read(g_st_fw_nvm_data[i].sec_no);
		if (strcmp(nvm_fw_data, "error") == 0) {
			hwlog_err("%s:read nvm data:%d fail\n",
				__func__, g_st_fw_nvm_data[i].sec_no);
			return STWLC33_NVM_ERR_PROGRAMED;
		}
		for (j = 0; j < g_st_fw_nvm_data[i].sec_size; j++) {
			if (g_st_nvm_bufs[j] !=
					g_st_fw_nvm_data[i].sec_data[j]) {
				g_st_fw_nvm_data[i].same_flag = 0;
				nvm_update_flag = STWLC33_NVM_NON_PROGRAMED;
				hwlog_info("%s: sec_no:%d not same\n",
					__func__, g_st_fw_nvm_data[i].sec_no);
				break;
			}
		}
		if (j == g_st_fw_nvm_data[i].sec_size)
			g_st_fw_nvm_data[i].same_flag = 1;
	}

	return nvm_update_flag;
}

static int stwlc33_nvm_write(void)
{
	int i;
	int ret;
	u16 cmd_status;
	u8 nvm_sec_val[STWLC33_NVM_REG_SIZE] = { 0 };
	u8 bufs[STWLC33_NVM_SEC_VAL_SIZE + STWLC33_ADDR_LEN] = { 0 };
	u8 sec_val[STWLC33_SEC_NO_SIZE] = { 0 };
	struct stwlc33_device_info *di = g_stwlc33_di;

	if (!di) {
		hwlog_err("%s: di is null\n", __func__);
		return -EINVAL;
	}
	for (i = 0; i < ARRAY_SIZE(g_st_fw_nvm_data); i++) {
		if (g_st_fw_nvm_data[i].same_flag)
			continue;
		/* Step 1 : check for zero, to confirm pre cmds completed */
		ret = stwlc33_read_word(STWLC33_CMD_STATUS_ADDR, &cmd_status);
		if (ret) {
			hwlog_err("%s: get cmd status failed\n", __func__);
			return -EINVAL;
		}
		if (cmd_status) {
			hwlog_err("%s: pre cmds not completed [0x%x]\n",
				__func__, cmd_status);
			return -EINVAL;
		}
		/* Step 2 : Write new NVM sector content  */
		memcpy(bufs + STWLC33_ADDR_LEN, g_st_fw_nvm_data[i].sec_data,
			g_st_fw_nvm_data[i].sec_size);
		ret = stwlc33_write_block(di, STWLC33_NVM_WR_ADDR, bufs,
			g_st_fw_nvm_data[i].sec_size);
		if (ret) {
			hwlog_err("%s: write nvm failed\n", __func__);
			return -EINVAL;
		}
		/* Step 3 : Update sector number */
		sec_val[0] = g_st_fw_nvm_data[i].sec_no;
		memcpy(nvm_sec_val + STWLC33_ADDR_LEN, sec_val,
			STWLC33_SEC_NO_SIZE);
		ret = stwlc33_write_block(di, STWLC33_OFFSET_REG_ADDR,
			nvm_sec_val, STWLC33_SEC_NO_SIZE);
		if (ret) {
			hwlog_err("%s: write sector no fail\n", __func__);
			return -EINVAL;
		}
		/* Step 4 : Update nvm byte length */
		ret = stwlc33_write_byte(STWLC33_SRAM_SIZE_ADDR,
			g_st_fw_nvm_data[i].sec_size - 1);
		if (ret) {
			hwlog_err("%s: write sector length fail\n", __func__);
			return -EINVAL;
		}
		/* Step 5 : Write nvm_wr command */
		ret = stwlc33_write_byte(STWLC33_CMD_STATUS_ADDR,
			STWLC33_NVM_WR_CMD);
		if (ret) {
			hwlog_err("%s: write nvm write cmd fail\n", __func__);
			return -EINVAL;
		}
		msleep(STWLC33_NVM_WR_TIME);
		/* Step 6 : check for zero, to confirm pre cmds completed */
		ret = stwlc33_read_word(STWLC33_CMD_STATUS_ADDR, &cmd_status);
		if (ret) {
			hwlog_err("%s: get cmd status failed\n", __func__);
			return -EINVAL;
		}
		if (cmd_status) {
			hwlog_err("%s: pre cmds not completed [0x%x]\n",
				__func__, cmd_status);
			return -EINVAL;
		}
	}
	hwlog_info("%s: success\n", __func__);
	return 0;
}

static int stwlc33_program_nvm(void)
{
	int ret;
	struct stwlc33_device_info *di = g_stwlc33_di;

	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return -EINVAL;
	}
	ret = stwlc33_check_nvm();
	if (ret < 0) {
		hwlog_err("%s: read nvm fail\n", __func__);
		return -EINVAL;
	}
	if (ret == STWLC33_NVM_PROGRAMED) {
		hwlog_info("%s: NVM is already programed\n", __func__);
		return 0;
	}

	stwlc33_disable_irq_nosync(di);
	ret = stwlc33_nvm_write();
	if (ret) {
		hwlog_err("%s: write nvm fail\n", __func__);
		stwlc33_enable_irq(di);
		return -EINVAL;
	}
	stwlc33_enable_irq(di);
	hwlog_info("%s: program NVM succ\n", __func__);
	return 0;
}
#endif /* WIRELESS_CHARGER_FACTORY_VERSION */

static int stwlc33_get_mode(u8 *mode)
{
	int ret;

	ret = stwlc33_read_byte(STWLC33_SYS_MODE_ADDR, mode);
	if (ret) {
		hwlog_err("%s:read failed\n", __func__);
		return -1;
	}
	return 0;
}

static int stwlc33_check_cmd_status(void)
{
	int ret;
	u16 cmd_status = 0;

	ret = stwlc33_read_word(STWLC33_CMD_STATUS_ADDR, &cmd_status);
	if (ret) {
		hwlog_err("%s: get cmd status fail\n", __func__);
		return -EINVAL;
	}
	if (cmd_status) {
		hwlog_err("%s: pre cmd not completed [0x%x]\n",
			__func__, cmd_status);
		return -EINVAL;
	}
	return 0;
}

static int stwlc33_write_ram_data(const struct fw_update_info *fw_sram_info)
{
	int i;
	int ret;
	u16 offset;
	u8 write_size;
	/* write datas must offset 2 bytes */
	u8 bufs[STWLC33_PAGE_SIZE + STWLC33_ADDR_LEN] = { 0 };
	u8 offset_reg_value[STWLC33_OFFSET_REG_SIZE] = { 0 };
	u8 st_fw_start_addr[] = { 0x00, 0x00, 0x00, 0x00 };

	if (!g_stwlc33_di || !fw_sram_info || !fw_sram_info->fw_sram) {
		hwlog_err("%s: para is null\n", __func__);
		return -ENODEV;
	}

	/* write FW into SRAM */
	i = fw_sram_info->fw_sram_size;
	offset = 0;
	while (i > 0) {
		if (i >= STWLC33_PAGE_SIZE)
			write_size = STWLC33_PAGE_SIZE;
		else
			write_size = i;
		/* Step 1 : confirm previous commands completed */
		if (stwlc33_check_cmd_status()) {
			hwlog_err("%s: check cmd status fail\n", __func__);
			return -EINVAL;
		}
		/* Step 2 : Load data */
		memcpy(bufs + STWLC33_ADDR_LEN, fw_sram_info->fw_sram + offset,
			write_size);
		ret = stwlc33_write_block(g_stwlc33_di,
			fw_sram_info->fw_sram_update_addr, bufs, write_size);
		if (ret) {
			hwlog_err("%s: write SRAM fm failed\n", __func__);
			return -EINVAL;
		}
		/* Step 3 : Update I2C reg mem_access_offset */
		st_fw_start_addr[0] = offset & BYTE_MASK;
		st_fw_start_addr[1] = offset >> BITS_PER_BYTE;
		memcpy(offset_reg_value + STWLC33_ADDR_LEN, st_fw_start_addr,
			STWLC33_OFFSET_VALUE_SIZE);
		ret = stwlc33_write_block(g_stwlc33_di, STWLC33_OFFSET_REG_ADDR,
			offset_reg_value, STWLC33_OFFSET_VALUE_SIZE);
		/* Step 4 : Update number of bytes */
		ret |= stwlc33_write_byte(STWLC33_SRAM_SIZE_ADDR,
			write_size - 1);
		if (ret) {
			hwlog_err("%s: Update write size failed\n", __func__);
			return -EINVAL;
		}
		/* Step 5 : Issue patch_wr command */
		ret = stwlc33_write_byte(STWLC33_ACT_CMD_ADDR,
			STWLC33_WRITE_CMD_VALUE);
		if (ret) {
			hwlog_err("%s: Issue patch_wr cmd failed\n", __func__);
			return -EINVAL;
		}
		offset += write_size;
		i -= write_size;
	}

	hwlog_info("%s: write FW into SRAM succ\n", __func__);
	return 0;
}

static int stwlc33_check_fw_sram_data(const unsigned char *src,
	const unsigned char *dest, u16 len)
{
	u16 i;

	if (!src || !dest) {
		hwlog_err("%s: input vaild\n", __func__);
		return -EINVAL;
	}
	for (i = 0; i < len; i++) {
		if (src[i] != dest[i]) {
			hwlog_err("%s: read 0x%x != write 0x%x\n",
				__func__, src[i], dest[i]);
			return -EINVAL;
		}
	}
	return 0;
}

static int stwlc33_check_ram_data(const struct fw_update_info *fw_sram_info)
{
	int i;
	int ret;
	u16 offset;
	u8 write_size;
	/* write datas must offset 2 bytes */
	u8 bufs[STWLC33_PAGE_SIZE + STWLC33_ADDR_LEN] = { 0 };
	u8 offset_reg_value[STWLC33_OFFSET_REG_SIZE] = { 0 };
	u8 st_fw_start_addr[] = { 0x00, 0x00, 0x00, 0x00 };

	if (!g_stwlc33_di || !fw_sram_info || !fw_sram_info->fw_sram) {
		hwlog_err("%s: para is null\n", __func__);
		return -EINVAL;
	}

	/* read ram patch and check */
	i = fw_sram_info->fw_sram_size;
	offset = 0;
	while (i > 0) {
		if (i >= STWLC33_PAGE_SIZE)
			write_size = STWLC33_PAGE_SIZE;
		else
			write_size = i;
		/* Step 1 : confirm previous commands completed */
		if (stwlc33_check_cmd_status()) {
			hwlog_err("%s: check cmd status fail\n", __func__);
			return -EINVAL;
		}
		/* Step 2 : Update I2C reg mem_access_offset */
		st_fw_start_addr[0] = offset & BYTE_MASK;
		st_fw_start_addr[1] = offset >> BITS_PER_BYTE;
		memcpy(offset_reg_value + STWLC33_ADDR_LEN, st_fw_start_addr,
			STWLC33_OFFSET_VALUE_SIZE);
		ret = stwlc33_write_block(g_stwlc33_di, STWLC33_OFFSET_REG_ADDR,
			offset_reg_value, STWLC33_OFFSET_VALUE_SIZE);
		/* Step 3 : Update number of bytes */
		ret |= stwlc33_write_byte(STWLC33_SRAM_SIZE_ADDR,
			write_size - 1);
		if (ret) {
			hwlog_err("%s: Update read size failed\n", __func__);
			return -EINVAL;
		}
		/* Step 4 : Issue patch_rd command */
		ret = stwlc33_write_byte(STWLC33_ACT_CMD_ADDR,
			STWLC33_READ_CMD_VALUE);
		if (ret) {
			hwlog_err("%s: Issue patch_rd cmd fail\n", __func__);
			return -EINVAL;
		}
		/* Step 5 : confirm previous commands completed */
		if (stwlc33_check_cmd_status()) {
			hwlog_err("%s: check cmd status fail\n", __func__);
			return -EINVAL;
		}
		/* Step 6 : Read data from buffer and check */
		ret = stwlc33_read_block(g_stwlc33_di,
			fw_sram_info->fw_sram_update_addr, bufs, write_size);
		if (ret) {
			hwlog_err("%s: read SRAM failed\n", __func__);
			return -EINVAL;
		}
		ret = stwlc33_check_fw_sram_data(bufs,
			fw_sram_info->fw_sram + offset, write_size);
		if (ret) {
			hwlog_err("%s: check_fw_sram_data failed\n", __func__);
			return -EINVAL;
		}
		i -= write_size;
		offset += write_size;
	}

	hwlog_info("%s: read and check ram patch succ\n", __func__);
	return 0;
}

static int stwlc33_exec_ram_data(void)
{
	int ret;
	u16 cmd_status = 0;
	u8 offset_reg_val[STWLC33_OFFSET_REG_SIZE] = { 0 };
	unsigned char magic_number[] = { 0x01, 0x10, 0x00, 0x20 };
	struct stwlc33_device_info *di = g_stwlc33_di;

	if (!di) {
		hwlog_err("%s: para is null\n", __func__);
		return -EINVAL;
	}
	/* Step 1 : check for zero, to confirm previous commands completed */
	ret = stwlc33_read_word(STWLC33_CMD_STATUS_ADDR, &cmd_status);
	if (ret) {
		hwlog_err("%s: get cmd status failed\n", __func__);
		return -EINVAL;
	}
	if (cmd_status) {
		hwlog_err("%s: pre cmds not completed [0x%x]\n",
			__func__, cmd_status);
		return -EINVAL;
	}
	/* Step 2 : Write 4-byte magic number */
	memcpy(offset_reg_val + STWLC33_ADDR_LEN, magic_number,
		STWLC33_OFFSET_VALUE_SIZE);
	ret = stwlc33_write_block(di, STWLC33_OFFSET_REG_ADDR,
		offset_reg_val, STWLC33_OFFSET_VALUE_SIZE);
	if (ret) {
		hwlog_err("%s: Write magic number failed\n", __func__);
		return -EINVAL;
	}
	/* Step 3 : Issue patch_exec command */
	ret |= stwlc33_write_byte(STWLC33_ACT_CMD_ADDR,
		STWLC33_EXEC_CMD_VALUE);
	if (ret) {
		hwlog_err("%s: Issue patch_exec cmd failed\n", __func__);
		return -EINVAL;
	}
	/* wait for swtiching to SRAM running */
	msleep(STWLC33_SRAM_EXEC_TIME);
	/* Step 4 : check for zero, to confirm patch execution */
	ret = stwlc33_read_word(STWLC33_CMD_STATUS_ADDR, &cmd_status);
	if (ret) {
		hwlog_err("%s: get cmd_status failed\n", __func__);
		return -EINVAL;
	}
	if (cmd_status) {
		hwlog_err("%s: pre cmds not completed [0x%x]\n",
			__func__, cmd_status);
		return -EINVAL;
	}

	hwlog_info("%s: exec ram patch succ\n", __func__);
	return 0;
}

static int stwlc33_program_sramupdate(
	const struct fw_update_info *fw_sram_info)
{
	int ret;
	u8 iout_offset;
	u16 ram_version;

	/* Step -1 write FW patch into SRAM */
	ret = stwlc33_write_ram_data(fw_sram_info);
	if (ret) {
		hwlog_err("%s: write ram data fail\n", __func__);
		return -EINVAL;
	}
	/* Step -2 read ram patch and check */
	ret = stwlc33_check_ram_data(fw_sram_info);
	if (ret) {
		hwlog_err("%s: check ram data fail\n", __func__);
		return -EINVAL;
	}
	/* Step -3 : To execute ram patch */
	ret = stwlc33_exec_ram_data();
	if (ret) {
		hwlog_err("%s: exec ram data fail\n", __func__);
		return -EINVAL;
	}
	if (fw_sram_info->fw_sram_mode == WIRELESS_TX_MODE) {
		usleep_range(9500, 10500); /* 10ms */
		stwlc33_write_byte(STWLC33_TX_ENABLE_ADDR,
			STWLC33_TX_ENABLE_VAL);
		ret = stwlc33_exec_ram_data();
		if (ret) {
			hwlog_err("%s: exec ram data fail\n", __func__);
			return -EINVAL;
		}
	}
	if ((fw_sram_info->fw_sram_mode == WIRELESS_RX_MODE) &&
		(stwlc33_iout_adc_val > STWLC33_IOUT_ADC_VAL_LTH) &&
		(stwlc33_iout_adc_val < STWLC33_IOUT_ADC_VAL_HTH)) {
		if (stwlc33_iout_adc_val >= STWLC33_IOUT_ADC_CRITICAL_VAL)
			iout_offset = stwlc33_iout_adc_val -
				STWLC33_IOUT_ADC_CRITICAL_VAL;
		else
			iout_offset = ~(STWLC33_IOUT_ADC_CRITICAL_VAL -
				stwlc33_iout_adc_val) + 1;
		hwlog_info("%s: iout_offset = 0x%x\n", __func__, iout_offset);
		ret = stwlc33_write_byte(STWLC33_IOUT_CALI_VAL_ADDR,
			iout_offset);

		/* clear iout calibration register */
		ret += stwlc33_write_byte(STWLC33_IOUT_CALI_ADDR, 0x00);
		if (ret)
			hwlog_err("%s: write reg failed\n", __func__);
	}
	ret = stwlc33_read_word(STWLC33_RAM_VER_ADDR, &ram_version);
	if (ret) {
		hwlog_err("%s: read ram version fail\n", __func__);
		return -EINVAL;
	}
	hwlog_info("%s: success ram_version:0x%x\n", __func__, ram_version);

	return 0;
}

static u8 *stwlc33_get_otp_fw_version(void)
{
	int i;
	int ret;
	static u8 fw_version[STWLC33_RX_OTP_FW_VERSION_STRING_LEN];
	u8 buff[STWLC33_RX_TMP_BUFF_LEN] = { 0 };
	u8 data[STWLC33_RX_OTP_FW_VERSION_LEN] = { 0 };
	struct stwlc33_device_info *di = g_stwlc33_di;

	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return "error";
	}
	memset(fw_version, 0, STWLC33_RX_OTP_FW_VERSION_STRING_LEN);
	ret = stwlc33_read_block(di, STWLC33_RX_OTP_FW_VERSION_ADDR,
		data, STWLC33_RX_OTP_FW_VERSION_LEN);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return "error";
	}
	strncat(fw_version, STWLC33_OTP_FW_HEAD, strlen(STWLC33_OTP_FW_HEAD));
	for (i = STWLC33_RX_OTP_FW_VERSION_LEN - 1; i >= 0; i--) {
		snprintf(buff, STWLC33_RX_TMP_BUFF_LEN, " 0x%02x", data[i]);
		strncat(fw_version, buff, strlen(buff));
	}

	hwlog_info("%s: otp version:%s\n", __func__, fw_version);
	return fw_version;
}

static int stwlc33_check_fwupdate(enum wireless_mode fw_sram_mode)
{
	int ret;
	int i;
	u8 *otp_fw_version = NULL;
	unsigned int fw_update_size;

#ifdef WIRELESS_CHARGER_FACTORY_VERSION
	ret = stwlc33_program_nvm();
	if (ret) {
		hwlog_err("%s: stwlc33_program_nvm fail\n", __func__);
		return -1;
	}
#endif /* WIRELESS_CHARGER_FACTORY_VERSION */
	fw_update_size = ARRAY_SIZE(stwlc33_sram);
	otp_fw_version = stwlc33_get_otp_fw_version();
	if (strcmp(otp_fw_version, "error") == 0) {
		hwlog_err("%s:get firmware version fail\n", __func__);
		return -1;
	}

	for (i = 0; i < fw_update_size; i++) {
		if ((fw_sram_mode == stwlc33_sram[i].fw_sram_mode) &&
			(strcmp(otp_fw_version,
				stwlc33_sram[i].name_fw_update_from) >= 0) &&
			(strcmp(otp_fw_version,
				stwlc33_sram[i].name_fw_update_to) <= 0)) {
			hwlog_info("SRAM update start, otp_fw_version = %s\n",
				otp_fw_version);
			ret = stwlc33_program_sramupdate(&stwlc33_sram[i]);
			if (ret) {
				hwlog_err("%s: SRAM update fail\n", __func__);
				return -1;
			}
			otp_fw_version = stwlc33_get_otp_fw_version();
			hwlog_info("SRAM update succ! otp_fw_version = %s\n",
				otp_fw_version);
			return 0;
		}
	}
	hwlog_info("SRAM no need update, otp_fw_version = %s\n",
		otp_fw_version);
	return -1;
}

static int stwlc33_tx_stop_config(void)
{
	return 0;
}

static int stwlc33_enable_tx_mode(bool enable)
{
	int ret;

	if (enable)
		ret = stwlc33_write_word_mask(STWLC33_CMD3_ADDR,
			STWLC33_CMD3_TX_EN_MASK,
			STWLC33_CMD3_TX_EN_SHIFT, 1);
	else
		ret = stwlc33_write_word_mask(STWLC33_CMD3_ADDR,
			STWLC33_CMD3_TX_DIS_MASK,
			STWLC33_CMD3_TX_DIS_SHIFT, 1);

	if (ret) {
		hwlog_err("%s tx mode fail\n", enable ? "enable" : "disable");
		return -1;
	}
	return 0;
}

static int stwlc33_enable_tx_ping(void)
{
	int ret;

	ret = stwlc33_write_word_mask(STWLC33_CHIP_RST_ADDR,
		STWLC33_TX_PING_EN_MASK, STWLC33_TX_PING_EN_SHIFT, 1);
	if (ret) {
		hwlog_err("%s: enable tx ping fail\n", __func__);
		return -1;
	}
	return 0;
}

static int stwlc33_get_tx_iin(u16 *tx_iin)
{
	int ret;

	if (!tx_iin) {
		hwlog_err("%s:tx_iin is null\n", __func__);
		return -1;
	}

	ret = stwlc33_read_word(STWLC33_TX_IIN_ADDR, tx_iin);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	return 0;
}

static int stwlc33_get_tx_vrect(u16 *tx_vrect)
{
	int ret;

	if (!tx_vrect) {
		hwlog_err("%s:tx_vrect is null\n", __func__);
		return -1;
	}

	ret = stwlc33_read_word(STWLC33_TX_VRECT_ADDR,
		tx_vrect);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	return 0;
}

static int stwlc33_get_tx_vin(u16 *tx_vin)
{
	int ret;

	if (!tx_vin) {
		hwlog_err("%s:tx_vin is null\n", __func__);
		return -1;
	}

	ret = stwlc33_read_word(STWLC33_TX_VIN_ADDR, tx_vin);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}

	if (*tx_vin == 0)
		*tx_vin = TX_DEFAULT_VOUT;

	return 0;
}

static int stwlc33_get_tx_fop(u16 *fop)
{
	int ret;
	u16 data = 0;

	if (!fop) {
		hwlog_err("%s:fop is null\n", __func__);
		return -1;
	}

	ret = stwlc33_read_word(STWLC33_TX_FOP_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	if (data)
		*fop = 60000 / data; /* Ping freq(khz) = 60000/PingPeriod */
	return 0;
}

static int stwlc33_set_tx_max_fop(u16 fop)
{
	int ret;

	if (fop == 0)
		return -1;

	ret = stwlc33_write_word(STWLC33_TX_MAX_FOP_ADDR,
		60000 / fop); /* Ping freq(khz) = 60000/PingPeriod */
	if (ret) {
		hwlog_err("%s: write failed\n", __func__);
		return -1;
	}
	return 0;
}

static int stwlc33_get_tx_max_fop(u16 *fop)
{
	int ret;
	u16 data = 0;

	if (!fop) {
		hwlog_err("%s:fop is null\n", __func__);
		return -1;
	}

	ret = stwlc33_read_word(STWLC33_TX_MAX_FOP_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	if (data)
		*fop = 60000 / data; /* Ping freq(khz) = 60000/PingPeriod */

	return 0;
}

static int stwlc33_set_tx_min_fop(u16 fop)
{
	int ret;

	if (fop == 0)
		return -1;

	/* Ping freq(khz) = 60000/PingPeriod */
	ret = stwlc33_write_word(STWLC33_TX_MIN_FOP_ADDR,
		60000 / fop);
	if (ret) {
		hwlog_err("%s: write failed\n", __func__);
		return -1;
	}
	return 0;
}

static int stwlc33_get_tx_min_fop(u16 *fop)
{
	int ret;
	u16 data = 0;

	if (!fop) {
		hwlog_err("%s:fop is null\n", __func__);
		return -1;
	}

	ret = stwlc33_read_word(STWLC33_TX_MIN_FOP_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	if (data)
		*fop = 60000 / data; /* Ping freq(khz) = 60000/PingPeriod */

	return 0;
}

static int stwlc33_set_tx_ping_frequency(u16 ping_freq)
{
	int ret;
	struct stwlc33_device_info *di = g_stwlc33_di;

	if (!di) {
		hwlog_err("set_tx_ping_frequency: di null\n");
		return -1;
	}

	if ((ping_freq < STWLC33_TX_PING_FREQUENCY_MIN) ||
		(ping_freq > STWLC33_TX_PING_FREQUENCY_MAX)) {
		hwlog_err("%s: ping frequency is out of range\n", __func__);
		return -1;
	}
	/* Ping freq(khz) = 60000/PingPeriod */
	ret = stwlc33_write_word(STWLC33_TX_PING_FREQUENCY_ADDR,
		60000 / ping_freq);
	if (ret) {
		hwlog_err("%s: write failed\n", __func__);
		return -1;
	}
	ret = stwlc33_write_word(STWLC33_TX_PING_FREQUENCY_ADDR_DYM,
		di->ping_freq_init_dym);
	if (ret) {
		hwlog_err("%s: write dynamic freq failed\n", __func__);
		return -1;
	}
	return ret;
}

static int stwlc33_get_tx_ping_frequency(u16 *ping_freq)
{
	int ret;
	u16 data = 0;

	if (!ping_freq) {
		hwlog_err("%s:ping_freq is null\n", __func__);
		return -1;
	}

	ret = stwlc33_read_word(STWLC33_TX_PING_FREQUENCY_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	if (data) /* Ping freq(khz) = 60000/PingPeriod */
		*ping_freq = 60000 / data;

	return 0;
}

static int stwlc33_set_tx_ping_interval(u16 ping_interval)
{
	int ret;

	if ((ping_interval < STWLC33_TX_PING_INTERVAL_MIN) ||
		(ping_interval > STWLC33_TX_PING_INTERVAL_MAX)) {
		hwlog_err("%s: ping interval is out of range\n", __func__);
		return -1;
	}
	ret = stwlc33_write_byte(STWLC33_TX_PING_INTERVAL_ADDR,
		ping_interval / STWLC33_TX_PING_INTERVAL_STEP);
	if (ret) {
		hwlog_err("%s: write failed\n", __func__);
		return -1;
	}
	return 0;
}

static int stwlc33_get_tx_ping_interval(u16 *ping_interval)
{
	int ret;
	u8 data;

	if (!ping_interval) {
		hwlog_err("%s:ping_interval is null\n", __func__);
		return -1;
	}

	ret = stwlc33_read_byte(STWLC33_TX_PING_INTERVAL_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	*ping_interval = data * STWLC33_TX_PING_INTERVAL_STEP;
	return 0;
}

static int stwlc33_get_chip_temp(u8 *chip_temp)
{
	int ret;

	if (!chip_temp) {
		hwlog_err("%s:chip_temp is null\n", __func__);
		return -1;
	}

	ret = stwlc33_read_byte(STWLC33_CHIP_TEMP_ADDR, chip_temp);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	return 0;
}

static int stwlc33_tx_chip_init(void)
{
	int ret;
	struct stwlc33_device_info *di = g_stwlc33_di;

	if (!di) {
		hwlog_err("%s: di is null\n", __func__);
		return -1;
	}

	di->irq_cnt = 0;
	g_irq_abnormal_flag = false;
	stwlc33_enable_irq(di);
	ret = stwlc33_write_word(STWLC33_TX_OCP_ADDR, STWLC33_TX_OCP_VAL);
	ret |= stwlc33_write_word(STWLC33_TX_OVP_ADDR, STWLC33_TX_OVP_VAL);
	ret |= stwlc33_write_word_mask(STWLC33_CMD3_ADDR,
		STWLC33_CMD3_TX_FOD_EN_MASK,
		STWLC33_CMD3_TX_FOD_EN_SHIFT, 1);
	ret |= stwlc33_write_word(STWLC33_TX_FOD_THD0_ADDR, di->tx_fod_th_5v);
	ret |= stwlc33_set_tx_ping_frequency(STWLC33_TX_PING_FREQUENCY_INIT);
	ret |= stwlc33_set_tx_ping_interval(STWLC33_TX_PING_INTERVAL_INIT);
	ret |= stwlc33_write_byte(STWLC33_TX_FOD_PL_ADDR, STWLC33_TX_FOD_PL_VAL);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return -1;
	}

	if (di->support_tx_adjust_vin) {
		ret = stwlc33_write_byte(STWLC33_TX_MAX_FREQ_ADDR, STWLC33_TX_MAX_FREQ);
		ret += stwlc33_write_byte(STWLC33_TX_MIN_FREQ_ADDR, STWLC33_TX_MIN_FREQ);
		/* default 5.8v */
		ret += stwlc33_write_byte(STWLC33_TX_GPIO_B2_ADDR, STWLC33_TX_GPIO_PU);
	} else {
		ret = stwlc33_write_byte(STWLC33_TX_MAX_FREQ_ADDR, STWLC33_TX_MAX_FREQ_DEFAULT);
		ret += stwlc33_write_byte(STWLC33_TX_MIN_FREQ_ADDR, STWLC33_TX_MIN_FREQ_DEFAULT);
	}
	if (ret) {
		hwlog_err("%s: freq write fail\n", __func__);
		return -1;
	}
	return 0;
}

static int stwlc33_send_fsk_msg(u8 cmd, u8 *data, int data_len)
{
	int ret;
	u8 msg_byte_num;
	/* transfer into I2C client,must offset 2 for i2c address */
	u8 write_data[STWLC33_TX_TO_RX_DATA_LEN + STWLC33_ADDR_LEN] = { 0 };
	struct stwlc33_device_info *di = g_stwlc33_di;

	if (!di) {
		hwlog_err("%s: para is null\n", __func__);
		return -1;
	}

	if ((data_len > STWLC33_TX_TO_RX_DATA_LEN) || (data_len < 0)) {
		hwlog_err("%s: send data out of range\n", __func__);
		return -1;
	}

	if (cmd == STWLC33_CMD_ACK)
		msg_byte_num = STWLC33_CMD_ACK_HEAD;
	else
		msg_byte_num = g_stwlc33_send_fsk_msg_len[data_len + 1];

	ret = stwlc33_write_byte(STWLC33_TX_TO_RX_HEADER_ADDR, msg_byte_num);
	if (ret) {
		hwlog_err("%s: write header failed\n", __func__);
		return -1;
	}
	ret = stwlc33_write_byte(STWLC33_TX_TO_RX_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("%s: write cmd failed\n", __func__);
		return -1;
	}

	if (data && (data_len > 0)) {
		memcpy(write_data + STWLC33_ADDR_LEN, data, data_len);
		ret = stwlc33_write_block(di, STWLC33_TX_TO_RX_DATA_ADDR,
			write_data, data_len);
		if (ret) {
			hwlog_err("write data into fsk register failed\n");
			return -1;
		}
	}
	ret = stwlc33_write_word_mask(STWLC33_CMD3_ADDR,
		STWLC33_CMD3_TX_SEND_FSK_MASK,
		STWLC33_CMD3_TX_SEND_FSK_SHIFT, 1);
	if (ret) {
		hwlog_err("%s: send fsk failed\n", __func__);
		return -1;
	}

	hwlog_info("%s: success\n", __func__);
	return 0;
}

static int stwlc33_send_fsk_msg_tx_id(void)
{
	/* bit[0:1]: tx id parameters, not include command */
	u8 id_para[STWLC33_TX_ID_LEN - 1] = { 0x88, 0x66 };
	int ret;

	ret = stwlc33_send_fsk_msg(STWLC33_CMD_GET_TX_ID,
		id_para, STWLC33_TX_ID_LEN - 1);
	if (ret) {
		hwlog_err("%s: send fsk message tx_id failed\n", __func__);
		return -1;
	}
	return 0;
}

static int stwlc33_send_fsk_msg_tx_cap(void)
{
	int ret;
	u8 tx_cap_data[TX_CAP_TOTAL] = { 0 };

	if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_OFF)
		tx_cap_data[TX_CAP_TYPE] = TX_TYPE_OTG_A;
	else
		tx_cap_data[TX_CAP_TYPE] = TX_TYPE_OTG_B;

	tx_cap_data[TX_CAP_VOUT_MAX] = STWLC33_TX_ADAPTER_OTG_MAX_VOL;
	tx_cap_data[TX_CAP_IOUT_MAX] = STWLC33_TX_ADAPTER_OTG_MAX_CUR;
	tx_cap_data[TX_CAP_ATTR] = 0;
	ret = stwlc33_send_fsk_msg(STWLC33_CMD_GET_TX_CAP,
		&tx_cap_data[TX_CAP_TYPE],
		TX_CAP_TOTAL - TX_CAP_TYPE);
	if (ret) {
		hwlog_err("send fsk message tx capacity failed\n");
		return -1;
	}
	return 0;
}

static int stwlc33_send_fsk_ack_msg(void)
{
	int ret;

	ret = stwlc33_send_fsk_msg(STWLC33_CMD_ACK, NULL, 0);
	if (ret) {
		hwlog_err("%s: send tx ack to rx failed\n", __func__);
		return -1;
	}
	return 0;
}

static bool stwlc33_check_rx_disconnect(void)
{
	struct stwlc33_device_info *di = g_stwlc33_di;

	if (di->ept_type & STWLC33_TX_EPT_CEP_TIMEOUT) {
		di->ept_type &= ~STWLC33_TX_EPT_CEP_TIMEOUT;
		hwlog_info("%s: RX disconnect\n", __func__);
		return true;
	}
	return false;
}

static bool stwlc33_in_tx_mode(void)
{
	u8 mode = 0;
	int ret;

	ret = stwlc33_get_mode(&mode);
	if (ret) {
		hwlog_err("%s: get mode failed\n", __func__);
		return false;
	}
	if (mode & STWLC33_TX_WPCMODE)
		return true;

	return false;
}

static int stwlc33_set_tx_acc_dev_state(u8 dev_state)
{
	g_wltx_acc->dev_state = dev_state;
	hwlog_info("%s, dev_state = %d\n", __func__, dev_state);
	return 0;
}

static int stwlc33_set_tx_acc_dev_info_cnt(u8 dev_info_cnt)
{
	g_wltx_acc->dev_info_cnt = dev_info_cnt;
	hwlog_info("set dev_info_cnt = %d\n", dev_info_cnt);
	return 0;
}

static int stwlc33_get_tx_acc_dev_state(u8 *dev_state)
{
	if (!dev_state) {
		hwlog_err("%s dev_state is null\n", __func__);
		return -1;
	}

	*dev_state = g_wltx_acc->dev_state;
	hwlog_info("%s, dev_state = %d\n", __func__, *dev_state);
	return 0;
}

static void stwlc33_set_tx_open_flag(bool enable)
{
	if (enable)
		g_i2c_fail_cnt = 0;
}

static int stwlc33_get_tx_acc_dev_no(u8 *dev_no)
{
	if (!dev_no) {
		hwlog_err("%s dev_no is null\n", __func__);
		return -1;
	}

	*dev_no = g_wltx_acc->dev_no;
	hwlog_info("%s, dev_no = %d\n", __func__, *dev_no);
	return 0;
}

static int stwlc33_get_tx_acc_dev_mac(u8 *dev_mac)
{
	int i;

	if (!dev_mac) {
		hwlog_err("%s dev_mac is null\n", __func__);
		return -1;
	}

	for (i = 0; i < WL_TX_ACC_DEV_MAC_LEN; i++) {
		dev_mac[i] = g_wltx_acc->dev_mac[i];
		hwlog_info("dev_mac[%d] = 0x%02x\n", i, dev_mac[i]);
	}
	return 0;
}

static int stwlc33_get_tx_acc_dev_model_id(u8 *dev_model_id)
{
	int i;

	if (!dev_model_id) {
		hwlog_err("%s dev_model_id is null\n", __func__);
		return -1;
	}

	for (i = 0; i < WL_TX_ACC_DEV_MODELID_LEN; i++) {
		dev_model_id[i] = g_wltx_acc->dev_model_id[i];
		hwlog_info("dev_model_id[%d] = 0x%02x\n", i, dev_model_id[i]);
	}
	return 0;
}

static int stwlc33_get_tx_acc_dev_submodel_id(u8 *dev_submodel_id)
{
	if (!dev_submodel_id) {
		hwlog_err("%s dev_submodel_id is null\n", __func__);
		return -1;
	}

	*dev_submodel_id = g_wltx_acc->dev_submodel_id;
	hwlog_info("%s, dev_submodel_id = 0x%02x\n",
		__func__, *dev_submodel_id);
	return 0;
}

static int stwlc33_get_tx_acc_dev_version(u8 *dev_version)
{
	if (!dev_version) {
		hwlog_err("%s dev_version is null\n", __func__);
		return -1;
	}

	*dev_version = g_wltx_acc->dev_version;
	hwlog_info("%s, dev_version = 0x%02x\n", __func__, *dev_version);
	return 0;
}

static int stwlc33_get_tx_acc_dev_business(u8 *dev_business)
{
	if (!dev_business) {
		hwlog_err("%s dev_business is null\n", __func__);
		return -1;
	}

	*dev_business = g_wltx_acc->dev_business;
	hwlog_info("%s, dev_business = 0x%02x\n", __func__, *dev_business);
	return 0;
}

static int stwlc33_get_tx_acc_dev_info_cnt(u8 *dev_info_cnt)
{
	if (!dev_info_cnt) {
		hwlog_err("%s dev_info_cnt is null\n", __func__);
		return -1;
	}

	*dev_info_cnt = g_wltx_acc->dev_info_cnt;
	hwlog_info("%s, dev_info_cnt = %d\n", __func__, *dev_info_cnt);
	return 0;
}

static int stwlc33_get_tx_ept_type(u16 *ept_type)
{
	int ret;
	u16 data = 0;

	if (!ept_type) {
		hwlog_err("%s ept_type is null\n", __func__);
		return -1;
	}

	ret = stwlc33_read_word(STWLC33_TX_EPT_TYPE_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return -1;
	}
	*ept_type = data;
	hwlog_info("%s: EPT type = 0x%04x", __func__, *ept_type);
	ret = stwlc33_write_word(STWLC33_TX_EPT_TYPE_ADDR, 0);
	if (ret) {
		hwlog_err("%s: write failed\n", __func__);
		return -1;
	}
	return 0;
}

static void stwlc33_get_ask_packet(u8 *packet_data,
	unsigned int packet_data_len)
{
	int ret;
	int i;
	char buff[STWLC33_RX_TO_TX_PACKET_BUFF_LEN] = { 0 };
	char packet_str[STWLC33_RX_TO_TX_PACKET_STR_LEN] = { 0 };
	struct stwlc33_device_info *di = g_stwlc33_di;

	if (!di || !packet_data || (packet_data_len <
		STWLC33_RX_TO_TX_PACKET_LEN)) {
		hwlog_err("%s: NULL pointer", __func__);
		return;
	}
	ret = stwlc33_read_block(di, STWLC33_RX_TO_TX_HEADER_ADDR,
		packet_data, STWLC33_RX_TO_TX_PACKET_LEN);
	if (ret) {
		hwlog_err("%s: read failed\n", __func__);
		return;
	}
	for (i = 0; i < STWLC33_RX_TO_TX_PACKET_LEN; i++) {
		snprintf(buff, STWLC33_RX_TO_TX_PACKET_BUFF_LEN,
			"0x%02x ", packet_data[i]);
		strncat(packet_str, buff, strlen(buff));
	}
	hwlog_info("%s: RX back packet: %s\n", __func__, packet_str);
}

static void stwlc33_handle_qi_ask_packet(struct stwlc33_device_info *di)
{
	u8 packet_data[STWLC33_RX_TO_TX_PACKET_LEN] = { 0 };

	/* handle wpc standard ask packet byte]0]: cmd; byte[1 2 3 4 5]: data */
	stwlc33_get_ask_packet(packet_data, STWLC33_RX_TO_TX_PACKET_LEN);
	if (packet_data[0] == STWLC33_QI_SIGNAL_STRENGTH)
		hwlog_info("%s:signal strength = %d\n", __func__,
			packet_data[1]);
}

static bool stwlc33_check_ask_header(u8 head)
{
	int i;

	for (i = 1; i < STWLC33_RX_TO_TX_PACKET_LEN; i++) {
		if (head == g_stwlc33_send_msg_len[i])
			return true;
	}
	return false;
}

static int stwlc33_tx_adjust_vin(u16 iin_avg)
{
	int ret = 0;
	struct stwlc33_device_info *di = g_stwlc33_di;

	if (!di) {
		hwlog_err("%s: di is null\n", __func__);
		return -EINVAL;
	}
	if (!di->support_tx_adjust_vin)
		return 0;

	if (iin_avg >= STWLC33_TX_MAX_IIN_AVG) {
		/* STWLC33_TX_GPIO_OPEN: 5.5v */
		ret = stwlc33_write_byte(STWLC33_TX_GPIO_B2_ADDR, STWLC33_TX_GPIO_OPEN);
		hwlog_info("%s: 0x0087 haven been set to open, 5.5v flm iin_avg:%d\n", __func__, iin_avg);
	} else if (iin_avg <= STWLC33_TX_MIN_IIN_AVG) {
		/* STWLC33_TX_GPIO_PD: 4.8v */
		ret = stwlc33_write_byte(STWLC33_TX_GPIO_B2_ADDR, STWLC33_TX_GPIO_PD);
		hwlog_info("%s: 0x0087 haven been set to pd, 4.8v flm iin_avg:%d\n", __func__, iin_avg);
	}
	return ret;
}

static void stwlc33_handle_ask_packet(struct stwlc33_device_info *di)
{
	int i;
	u16 tx_id;
	u8 chrg_stage;
	u8 packet_data[STWLC33_RX_TO_TX_PACKET_LEN] = { 0 };

	/* byte]0]: header;  byte[1]: cmd;  byte[2 3 4 5]: data */
	stwlc33_get_ask_packet(packet_data, STWLC33_RX_TO_TX_PACKET_LEN);
	if (!stwlc33_check_ask_header(packet_data[0])) {
		hwlog_err("%s:head 0x%x  not correct\n", __func__,
			packet_data[0]);
		return;
	}
	switch (packet_data[1]) {
	case STWLC33_CMD_GET_TX_ID:
		tx_id = (packet_data[2] << BITS_PER_BYTE) | packet_data[3];
		if (tx_id == TX_ID_HW) {
			stwlc33_send_fsk_msg_tx_id();
			hwlog_info("%s:0x8866 handshake succ\n", __func__);
			power_event_bnc_notify(POWER_BNT_WLTX_AUX,
				POWER_NE_WLTX_HANDSHAKE_SUCC, NULL);
		}
		break;
	case STWLC33_CMD_GET_TX_CAP:
		stwlc33_send_fsk_msg_tx_cap();
		break;
	case STWLC33_CMD_SEND_CHRG_STATE:
		chrg_stage = packet_data[2];
		hwlog_info("%s: charge state 0x%x\n", __func__, chrg_stage);
		stwlc33_send_fsk_ack_msg();
		if (chrg_stage & WIRELESS_STATE_CHRG_DONE) {
			hwlog_info("%s:receive RX charge-done\n", __func__);
			power_event_bnc_notify(POWER_BNT_WLTX_AUX,
				POWER_NE_WLTX_CHARGEDONE, NULL);
		}
		break;
	/* STWLC33_CMD_RX_TO_TX_BT_MAC contain first 4Byte BT MAC */
	case STWLC33_CMD_RX_TO_TX_BT_MAC:
		hwlog_info("%s: g_wltx_acc->dev_info_cnt = %d\n",
			__func__, g_wltx_acc->dev_info_cnt);
		if (g_wltx_acc->dev_info_cnt != 0)
			g_wltx_acc->dev_info_cnt = 0;
		/* first 4byte BT mac address data */
		for (i = 0; i < STWLC33_RX_TO_TX_DATA_LEN; i++) {
			/* data begin packet_data[2] */
			g_wltx_acc->dev_mac[
				g_wltx_acc->dev_info_cnt] =
				packet_data[2 + i];
			hwlog_info("%s:dev_mac[%d] = 0x%02x\n", __func__,
				g_wltx_acc->dev_info_cnt,
				g_wltx_acc->dev_mac[
					g_wltx_acc->dev_info_cnt]);
			g_wltx_acc->dev_info_cnt++;
		}
		stwlc33_send_fsk_ack_msg();
		break;
	/*
	 * STWLC33_CMD_RX_TO_TX_BT_MAC2
	 * contain last 2Byte BT MAC+dev_version+dev_business
	 */
	case STWLC33_CMD_RX_TO_TX_BT_MAC2:
		hwlog_info("dev_info_cnt = %d\n",
			g_wltx_acc->dev_info_cnt);
		if (g_wltx_acc->dev_info_cnt !=
				STWLC33_RX_TO_TX_DATA_LEN) {
			hwlog_info("%s:mac2 cmd and cnt not right\n",
				__func__);
			break;
		}
		/* last 2 Byte BT mac address data */
		for (i = 0; i < (WL_TX_ACC_DEV_MAC_LEN -
			STWLC33_RX_TO_TX_DATA_LEN); i++) {
			/* data begin packet_data[2] */
			g_wltx_acc->dev_mac[
				g_wltx_acc->dev_info_cnt] =
				packet_data[2 + i];
			hwlog_info("dev_mac2[%d] = 0x%02x\n",
				g_wltx_acc->dev_info_cnt,
				g_wltx_acc->dev_mac[
					g_wltx_acc->dev_info_cnt]);
			g_wltx_acc->dev_info_cnt++;
		}
		/* packet_data[4] is dev version info */
		g_wltx_acc->dev_version = packet_data[4];
		hwlog_info("dev_version = 0x%02x\n",
			g_wltx_acc->dev_version);
		g_wltx_acc->dev_info_cnt++;
		/* packet_data[5] is dev business info */
		g_wltx_acc->dev_business = packet_data[5];
		hwlog_info("dev_business = 0x%02x\n",
			g_wltx_acc->dev_business);
		g_wltx_acc->dev_info_cnt++;

		stwlc33_send_fsk_ack_msg();
		break;
	/*
	 * STWLC33_CMD_RX_TO_TX_BT_MODEL_ID
	 * contain dev_model_id + dev_submodel_id
	 */
	case STWLC33_CMD_RX_TO_TX_BT_MODEL_ID:
		hwlog_info("%s:dev_info_cnt = %d\n", __func__,
			g_wltx_acc->dev_info_cnt);
		if (g_wltx_acc->dev_info_cnt !=
			(WL_TX_ACC_DEV_MAC_LEN +
			WL_TX_ACC_DEV_VERSION_LEN +
			WL_TX_ACC_DEV_BUSINESS_LEN)) {
			hwlog_err("%s:modeid cmd and cnt not right\n",
				__func__);
			break;
		}
		/* packet_data[2 3 4] is dev model id */
		for (i = 0; i < WL_TX_ACC_DEV_MODELID_LEN; i++) {
			g_wltx_acc->dev_model_id[i] = packet_data[2 + i];
			hwlog_info("%s: dev_model_id[%d] = 0x%02x\n",
				__func__, i,
				g_wltx_acc->dev_model_id[i]);
			g_wltx_acc->dev_info_cnt++;
		}
		/* packet_data[5] is dev submodel id */
		g_wltx_acc->dev_submodel_id = packet_data[5];
		g_wltx_acc->dev_info_cnt++;
		hwlog_info("%s:dev_info_cnt = %d\n", __func__,
			g_wltx_acc->dev_info_cnt);

		if (g_wltx_acc->dev_info_cnt == (
			WL_TX_ACC_DEV_MAC_LEN +
			WL_TX_ACC_DEV_VERSION_LEN +
			WL_TX_ACC_DEV_BUSINESS_LEN +
			WL_TX_ACC_DEV_MODELID_LEN +
			WL_TX_ACC_DEV_SUBMODELID_LEN)) {
			hwlog_info("%s:get acc dev info succ\n",
				__func__);
			g_wltx_acc->dev_state = WL_ACC_DEV_STATE_ONLINE;
			power_event_bnc_notify(POWER_BNT_WLTX_AUX,
				POWER_NE_WLTX_ACC_DEV_CONNECTED, NULL);
		}
		stwlc33_send_fsk_ack_msg();
		break;
	default:
		/* do nothing */
		break;
	}
}

static void stwlc33_handle_tx_ept(struct stwlc33_device_info *di)
{
	int ret;

	ret = stwlc33_get_tx_ept_type(&di->ept_type);
	if (ret) {
		hwlog_err("%s: get tx ept type failed\n", __func__);
		return;
	}
	switch (di->ept_type) {
	case STWLC33_TX_EPT_CMD:
		di->ept_type &= ~STWLC33_TX_EPT_CMD;
		hwlog_info("%s: ept command\n", __func__);
		break;
	case STWLC33_TX_EPT_CEP_TIMEOUT:
		di->ept_type &= ~STWLC33_TX_EPT_CEP_TIMEOUT;
		hwlog_info("%s: RX disconnect\n", __func__);
		power_event_bnc_notify(POWER_BNT_WLTX_AUX,
			POWER_NE_WLTX_CEP_TIMEOUT, NULL);
		break;
	case STWLC33_TX_EPT_FOD:
		di->ept_type &= ~STWLC33_TX_EPT_FOD;
		hwlog_info("%s: fod happened\n", __func__);
		power_event_bnc_notify(POWER_BNT_WLTX_AUX,
			POWER_NE_WLTX_TX_FOD, NULL);
		break;
	case STWLC33_TX_EPT_OVP:
		di->ept_type &= ~STWLC33_TX_EPT_OVP;
		hwlog_info("%s: ovp happened\n", __func__);
		power_event_bnc_notify(POWER_BNT_WLTX_AUX,
			POWER_NE_WLTX_OVP, NULL);
		break;
	case STWLC33_TX_EPT_OCP:
		di->ept_type &= ~STWLC33_TX_EPT_OCP;
		hwlog_info("%s: ocp happened\n", __func__);
		power_event_bnc_notify(POWER_BNT_WLTX_AUX,
			POWER_NE_WLTX_OCP, NULL);
		break;
	case STWLC33_TX_EPT_TIMEOUT:
		di->ept_type &= ~STWLC33_TX_EPT_TIMEOUT;
		hwlog_info("%s: chip reset happened\n", __func__);
		break;
	default:
		/* do nothing */
		break;
	}
}

static int stwlc33_clear_tx_interrupt(u16 itr)
{
	int ret;

	ret = stwlc33_write_word(STWLC33_RX_INT_CLEAR_ADDR, itr);
	if (ret) {
		hwlog_err("%s: write failed\n", __func__);
		return -1;
	}

	ret = stwlc33_write_word_mask(STWLC33_CMD3_ADDR,
		STWLC33_CMD3_TX_CLRINT_MASK,
		STWLC33_CMD3_TX_CLRINT_SHIFT, 1);
	if (ret) {
		hwlog_err("%s: write failed\n", __func__);
		return -1;
	}
	return 0;
}

static void stwlc33_tx_mode_irq_handler(struct stwlc33_device_info *di)
{
	int ret;
	u16 irq_value = 0;

	ret = stwlc33_read_word(STWLC33_RX_INT_STATUS_ADDR, &di->irq_val);
	if (ret) {
		hwlog_err("%s:read int status fail,clear\n", __func__);
		stwlc33_clear_tx_interrupt(WORD_MASK);
		goto func_end;
	}

	hwlog_info("%s: interrupt 0x%04x\n", __func__, di->irq_val);
	stwlc33_clear_tx_interrupt(di->irq_val);

	/* receice message from RX, please handler it! */
	if (di->irq_val & STWLC33_TX_STATUS_START_DPING) {
		hwlog_info("%s: tx PING interrupt\n", __func__);
		di->irq_val &= ~STWLC33_TX_STATUS_START_DPING;
		power_event_bnc_notify(POWER_BNT_WLTX_AUX,
			POWER_NE_WLTX_PING_RX, NULL);
	}

	if (di->irq_val & STWLC33_TX_STATUS_GET_SS) {
		hwlog_info("%s: Signal Strength packet interrupt\n", __func__);
		di->irq_val &= ~STWLC33_TX_STATUS_GET_SS;
		stwlc33_handle_qi_ask_packet(di);
	}

	if (di->irq_val & STWLC33_TX_STATUS_GET_ID) {
		hwlog_info("%s: ID packet interrupt\n", __func__);
		di->irq_val &= ~STWLC33_TX_STATUS_GET_ID;
		stwlc33_handle_qi_ask_packet(di);
	}

	if (di->irq_val & STWLC33_TX_STATUS_GET_CFG) {
		hwlog_info("%s: Config packet interrupt\n", __func__);
		di->irq_val &= ~STWLC33_TX_STATUS_GET_CFG;
		stwlc33_handle_qi_ask_packet(di);
		power_event_bnc_notify(POWER_BNT_WLTX_AUX,
			POWER_NE_WLTX_GET_CFG, NULL);
	}
	if (di->irq_val & STWLC33_TX_STATUS_EPT_TYPE) {
		di->irq_val &= ~STWLC33_TX_STATUS_EPT_TYPE;
		stwlc33_handle_tx_ept(di);
	}

	if (di->irq_val & STWLC33_TX_STATUS_GET_PPP) {
		hwlog_info("%s:receive personal ASK packet\n", __func__);
		di->irq_val &= ~STWLC33_TX_STATUS_GET_PPP;
		stwlc33_handle_ask_packet(di);
	}

func_end:
	/* clear interrupt again */
	if (!gpio_get_value(di->gpio_int)) {
		stwlc33_read_word(STWLC33_RX_INT_STATUS_ADDR, &irq_value);
		hwlog_info("%s:gpio_int low,clear interrupt,irq_value=0x%x\n",
			__func__, irq_value);
		stwlc33_clear_tx_interrupt(WORD_MASK);
	}
}

static void stwlc33_irq_work(struct work_struct *work)
{
	u8 mode = 0;
	struct stwlc33_device_info *di =
		container_of(work, struct stwlc33_device_info, irq_work);

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		stwlc33_wake_unlock();
		return;
	}

	/* get System Operating Mode */
	stwlc33_get_mode(&mode);

	/* handler interrupt */
	if ((mode & STWLC33_TX_WPCMODE) || (mode & STWLC33_BACKPOWERED))
		stwlc33_tx_mode_irq_handler(di);

	if (!g_irq_abnormal_flag)
		stwlc33_enable_irq(di);
	stwlc33_wake_unlock();
}

static irqreturn_t stwlc33_interrupt(int irq, void *_di)
{
	struct stwlc33_device_info *di = _di;

	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return IRQ_HANDLED;
	}

	stwlc33_wake_lock();
	hwlog_info("%s ++\n", __func__);
	if (di->irq_active == 1) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = 0;
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("irq is not enable,do nothing\n");
		stwlc33_wake_unlock();
	}
	hwlog_info("%s --\n", __func__);

	return IRQ_HANDLED;
}

static int stwlc33_parse_fod(struct device_node *np,
	struct stwlc33_device_info *di)
{
	if (!np || !di) {
		hwlog_err("%s: np or di null\n", __func__);
		return -EINVAL;
	}
	if (!g_support_st_wlc)
		return 0;
	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"st_rx_fod_5v", di->st_rx_fod_5v, STWLC33_RX_FOD_COEF_LEN))
		return -EINVAL;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"st_rx_fod_9v", di->st_rx_fod_9v, STWLC33_RX_FOD_COEF_LEN))
		return -EINVAL;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"st_rx_fod_12v", di->st_rx_fod_12v, STWLC33_RX_FOD_COEF_LEN))
		return -EINVAL;

	return 0;
}

static int stwlc33_parse_dts(struct device_node *np,
	struct stwlc33_device_info *di)
{
	int ret;

	if (!np || !di) {
		hwlog_err("%s: np or di null\n", __func__);
		return -EINVAL;
	}

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_ss_good_lth", &di->rx_ss_good_lth, STWLC33_RX_SS_MAX);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_en_valid_val", &di->gpio_en_valid_val, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_fod_th_5v", &di->tx_fod_th_5v, STWLC33_TX_FOD_THD0_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sc_rx_vrmax_gap", &di->sc_rx_vrmax_gap,
		300); /* volt=gap*21/4095=1.53v */

#ifdef WIRELESS_CHARGER_FACTORY_VERSION
	if (power_dts_read_string(power_dts_tag(HWLOG_TAG), np,
		"latest_otp_version", (const char **)&g_latest_otp_version))
		return -EINVAL;
#endif /* WIRELESS_CHARGER_FACTORY_VERSION */

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_idt_wlc", &g_support_idt_wlc, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_st_wlc", &g_support_st_wlc, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"i2c_trans_fail_limit", &di->i2c_trans_fail_limit, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_tx_adjust_vin", &di->support_tx_adjust_vin, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ping_freq_init_dym", (u32 *)&di->ping_freq_init_dym,
			STWLC33_TX_PING_FREQUENCY_INIT_DYM);

	ret = stwlc33_parse_fod(np, di);
	if (ret) {
		hwlog_err("%s: get st fod para fail\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int stwlc33_gpio_init(struct stwlc33_device_info *di,
	struct device_node *np)
{
	if (power_gpio_config_output(np,
		"gpio_en", "stwlc33_en",
		&di->gpio_en, di->gpio_en_valid_val))
		return -EINVAL;

	return 0;
}

static int stwlc33_irq_init(struct stwlc33_device_info *di,
	struct device_node *np)
{
	int ret;

	if (power_gpio_config_interrupt(np,
		"gpio_int", "stwlc33_int", &di->gpio_int, &di->irq_int)) {
		ret = -EINVAL;
		goto irq_init_fail_0;
	}

	ret = request_irq(di->irq_int, stwlc33_interrupt,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND,
		"stwlc33_irq", di);
	if (ret) {
		hwlog_err("could not request stwlc33_irq\n");
		di->irq_int = -1;
		goto irq_init_fail_1;
	}
	enable_irq_wake(di->irq_int);
	di->irq_active = 1;
	INIT_WORK(&di->irq_work, stwlc33_irq_work);

	return 0;

irq_init_fail_1:
	gpio_free(di->gpio_int);
irq_init_fail_0:
	return ret;
}

static struct wltx_dev_ops stwlc33_tx_ops = {
	.rx_enable                  = stwlc33_chip_enable,
	.chip_reset                 = stwlc33_chip_reset,
	.enable_tx_mode             = stwlc33_enable_tx_mode,
	.enable_tx_ping             = stwlc33_enable_tx_ping,
	.tx_chip_init               = stwlc33_tx_chip_init,
	.tx_stop_config             = stwlc33_tx_stop_config,
	.check_fwupdate             = stwlc33_check_fwupdate,
	.get_tx_iin                 = stwlc33_get_tx_iin,
	.get_tx_vrect               = stwlc33_get_tx_vrect,
	.get_tx_vin                 = stwlc33_get_tx_vin,
	.get_chip_temp              = stwlc33_get_chip_temp,
	.get_tx_fop                 = stwlc33_get_tx_fop,
	.tx_adjust_vin              = stwlc33_tx_adjust_vin,
	.set_tx_max_fop             = stwlc33_set_tx_max_fop,
	.get_tx_max_fop             = stwlc33_get_tx_max_fop,
	.set_tx_min_fop             = stwlc33_set_tx_min_fop,
	.get_tx_min_fop             = stwlc33_get_tx_min_fop,
	.set_tx_ping_frequency      = stwlc33_set_tx_ping_frequency,
	.get_tx_ping_frequency      = stwlc33_get_tx_ping_frequency,
	.set_tx_ping_interval       = stwlc33_set_tx_ping_interval,
	.get_tx_ping_interval       = stwlc33_get_tx_ping_interval,
	.check_rx_disconnect        = stwlc33_check_rx_disconnect,
	.in_tx_mode                 = stwlc33_in_tx_mode,
	.set_tx_open_flag           = stwlc33_set_tx_open_flag,
	.get_tx_acc_dev_no          = stwlc33_get_tx_acc_dev_no,
	.get_tx_acc_dev_state       = stwlc33_get_tx_acc_dev_state,
	.get_tx_acc_dev_mac         = stwlc33_get_tx_acc_dev_mac,
	.get_tx_acc_dev_model_id    = stwlc33_get_tx_acc_dev_model_id,
	.get_tx_acc_dev_submodel_id = stwlc33_get_tx_acc_dev_submodel_id,
	.get_tx_acc_dev_version     = stwlc33_get_tx_acc_dev_version,
	.get_tx_acc_dev_business    = stwlc33_get_tx_acc_dev_business,
	.set_tx_acc_dev_state       = stwlc33_set_tx_acc_dev_state,
	.get_tx_acc_dev_info_cnt    = stwlc33_get_tx_acc_dev_info_cnt,
	.set_tx_acc_dev_info_cnt    = stwlc33_set_tx_acc_dev_info_cnt,
};

static void stwlc33_shutdown(struct i2c_client *client)
{
	int ret;

	hwlog_info("%s ++\n", __func__);
	if (stwlc33_in_tx_mode()) {
		ret = stwlc33_enable_tx_mode(false);
		if (ret < 0)
			hwlog_err("%s:disable tx mode failed\n", __func__);
	}
	hwlog_info("%s --\n", __func__);
}

static int stwlc33_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct stwlc33_device_info *di = NULL;
	struct device_node *np = NULL;
	struct wltx_dev_ops *tx_ops = NULL;
	struct wltx_acc_dev *tx_acc_dev_di = NULL;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;
	g_stwlc33_di = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);
	ret = stwlc33_parse_dts(np, di);
	if (ret)
		goto stwlc33_fail_0;

	ret = stwlc33_gpio_init(di, np);
	if (ret)
		goto stwlc33_fail_0;
	ret = stwlc33_irq_init(di, np);
	if (ret)
		goto stwlc33_fail_1;

	tx_acc_dev_di = kzalloc(sizeof(*tx_acc_dev_di), GFP_KERNEL);
	if (!tx_acc_dev_di)
		goto stwlc33_fail_2;
	g_wltx_acc = tx_acc_dev_di;
	g_wltx_acc->dev_no = ACC_DEV_NO_PEN;

	wakeup_source_init(&g_stwlc33_wakelock, "stwlc33_wakelock");
	mutex_init(&di->mutex_irq);
	tx_ops = &stwlc33_tx_ops;
	ret = wltx_aux_ops_register(tx_ops);
	if (ret) {
		hwlog_err("%s:register aux tx ops failed\n", __func__);
		goto stwlc33_fail_3;
	}
	hwlog_info("%s:wireless_stwlc33 probe ok\n", __func__);
	return 0;

stwlc33_fail_3:
	wakeup_source_trash(&g_stwlc33_wakelock);
	kfree(tx_acc_dev_di);
stwlc33_fail_2:
	free_irq(di->irq_int, di);
stwlc33_fail_1:
	gpio_free(di->gpio_en);
stwlc33_fail_0:
	devm_kfree(&client->dev, di);
	di = NULL;
	np = NULL;
	return ret;
}

MODULE_DEVICE_TABLE(i2c, stwlc33);
static const struct of_device_id stwlc33_of_match[] = {
	{
		.compatible = "huawei, stwlc33",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id stwlc33_i2c_id[] = {
	{ "stwlc33", 0 },
	{}
};

static struct i2c_driver stwlc33_driver = {
	.probe = stwlc33_probe,
	.shutdown = stwlc33_shutdown,
	.id_table = stwlc33_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "stwlc33",
		.of_match_table = of_match_ptr(stwlc33_of_match),
	},
};

static int __init stwlc33_init(void)
{
	int ret;

	ret = i2c_add_driver(&stwlc33_driver);
	if (ret)
		hwlog_err("%s: i2c_add_driver error\n", __func__);

	return ret;
}

static void __exit stwlc33_exit(void)
{
	i2c_del_driver(&stwlc33_driver);
}

device_initcall(stwlc33_init);
module_exit(stwlc33_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("wpc receiver module driver");
MODULE_AUTHOR("HUAWEI Inc");
