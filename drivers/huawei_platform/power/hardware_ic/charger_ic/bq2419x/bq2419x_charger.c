/*
 * bq2419x_charger.c
 *
 * bq2419x driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/pm_wakeup.h>
#include <linux/usb/otg.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/power_supply.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/raid/pq.h>

#include <linux/hisi/usb/hisi_usb.h>
#include <linux/hisi/hisi_adc.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <huawei_platform/power/huawei_charger.h>
#ifdef CONFIG_HISI_BCI_BATTERY
#include <linux/power/hisi/hisi_bci_battery.h>
#endif
#include "bq2419x_charger.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG bq2419x_charger
HWLOG_REGIST();

struct bq2419x_device_info *g_bq2419x_dev;
/* configured in dts based on the real value of the iin limit resistance */
static unsigned int rilim = BQ2419X_RILIM_220_OHM;
/* configured in dts based on the real adc channel number */
static unsigned int adc_channel_iin = BQ2419X_ADC_CHANNEL_IIN_10;

#define MSG_LEN                      2
#define BUF_LEN                      26

static int params_to_reg(const int tbl[], int tbl_size, int val)
{
	int i;

	for (i = 1; i < tbl_size; i++) {
		if (val < tbl[i])
			return (i - 1);
	}

	return (tbl_size - 1);
}

static int bq2419x_write_block(struct bq2419x_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[1];
	int ret;

	if (!di || !di->client || !value) {
		hwlog_err("di or value is null\n");
		return -EIO;
	}

	*value = reg;

	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = value;
	msg[0].len = num_bytes + 1;

	ret = i2c_transfer(di->client->adapter, msg, 1);

	/* i2c_transfer returns number of messages transferred */
	if (ret != 1) {
		hwlog_err("write_block failed[%x]\n", reg);
		return -EIO;
	}

	return 0;
}

static int bq2419x_read_block(struct bq2419x_device_info *di,
	u8 *value, u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[MSG_LEN];
	u8 buf;
	int ret;

	if (!di || !di->client || !value) {
		hwlog_err("di or value is null\n");
		return -EIO;
	}

	buf = reg;

	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = &buf;
	msg[0].len = 1;

	msg[1].addr = di->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = value;
	msg[1].len = num_bytes;

	ret = i2c_transfer(di->client->adapter, msg, MSG_LEN);

	/* i2c_transfer returns number of messages transferred */
	if (ret != MSG_LEN) {
		hwlog_err("read_block failed[%x]\n", reg);
		return -EIO;
	}

	return 0;
}

static int bq2419x_write_byte(u8 reg, u8 value)
{
	struct bq2419x_device_info *di = g_bq2419x_dev;
	/* 2 bytes offset 1 contains the data offset 0 is used by i2c_write */
	u8 temp_buffer[MSG_LEN] = {0};

	/* offset 1 contains the data */
	temp_buffer[1] = value;
	return bq2419x_write_block(di, temp_buffer, reg, 1);
}

static int bq2419x_read_byte(u8 reg, u8 *value)
{
	struct bq2419x_device_info *di = g_bq2419x_dev;

	return bq2419x_read_block(di, value, reg, 1);
}

static int bq2419x_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = bq2419x_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return bq2419x_write_byte(reg, val);
}

static int bq2419x_device_check(void)
{
	u8 reg = 0;
	int ret;

	ret = bq2419x_read_byte(BQ2419X_REG_VPRS, &reg);
	if (ret)
		return CHARGE_IC_BAD;

	hwlog_info("device_check [%x]=0x%x\n", BQ2419X_REG_VPRS, reg);

	if ((reg & BQ2419X_REG_VPRS_DEV_REG_MASK) &&
		(reg & BQ2419X_REG_VPRS_PN_MASK)) {
		hwlog_info("bq2419x is good\n");
		return CHARGE_IC_GOOD;
	}

	hwlog_err("bq2419x is bad\n");
	return CHARGE_IC_BAD;
}

static int bq2419x_5v_chip_init(struct bq2419x_device_info *di)
{
	int ret;

	/* boost mode current limit = 500mA */
	ret = bq2419x_write_byte(BQ2419X_REG_POC, 0x1a);

	/* I2C watchdog timer setting = 80s */
	/* fast charge timer setting = 12h */
	ret |= bq2419x_write_byte(BQ2419X_REG_CTTC, 0x2d);

	/* iprechg = 256ma,iterm current = 128ma */
	ret |= bq2419x_write_byte(BQ2419X_REG_PCTCC, 0x10);

	/* IR compensation voatge clamp = 48mV */
	/* IR compensation resistor setting = 40mohm */
	ret |= bq2419x_write_byte(BQ2419X_REG_ICTRC, 0x8f);

	/* enable charging */
	gpio_set_value(di->gpio_cd, 0);

	return ret;
}

static int bq2419x_chip_init(struct chip_init_crit *init_crit)
{
	int ret = -1;
	struct bq2419x_device_info *di = g_bq2419x_dev;

	if (!di || !init_crit) {
		hwlog_err("di or init_crit is null\n");
		return -ENOMEM;
	}

	switch (init_crit->vbus) {
	case ADAPTER_5V:
		ret = bq2419x_5v_chip_init(di);
		break;
	default:
		hwlog_err("invaid init_crit vbus mode\n");
		break;
	}

	return ret;
}

static int bq2419x_set_input_current(int value)
{
	int val;
	int array_size = ARRAY_SIZE(bq2419x_iin_values);

	if (value > bq2419x_iin_values[array_size - 1])
		return bq2419x_iin_values[array_size - 1];
	else if (value < bq2419x_iin_values[0])
		return bq2419x_iin_values[0];

	val = params_to_reg(bq2419x_iin_values, array_size, value);

	hwlog_info("set_input_current [%x]=0x%x\n", BQ2419X_REG_ISC, val);

	return bq2419x_write_mask(BQ2419X_REG_ISC,
		BQ2419X_REG_ISC_IINLIM_MASK,
		BQ2419X_REG_ISC_IINLIM_SHIFT,
		val);
}

static int bq2419x_set_charge_current(int value)
{
	int ret;
	int val;
	int force_20pct_en = 0;
	int array_size = ARRAY_SIZE(bq2419x_ichg_values);

	if (value > bq2419x_ichg_values[array_size - 1])
		return bq2419x_ichg_values[array_size - 1];

	/*
	 * 1. If currentmA is below ICHG_512,
	 * we can set the ICHG to 5*currentmA and set the FORCE_20PCT in REG02
	 * to make the true current 20% of the ICHG.
	 * 2. To slove the OCP BUG of BQ2419X,
	 * we need set the ICHG (lower than 1024mA) to
	 * 5*currentmA and set the FORCE_20PCT in REG02.
	 */
	if (value < 1024) {
		value *= 5;
		force_20pct_en = 1;
	}

	/*
	 * why don't return bq2419x_ichg_values[0] when
	 * value < bq2419x_ichg_values[0]? because the charge current
	 * have 20pct setting mode, if someone want to setting 100mA,
	 * we should set 100*5 by 20pct mode, instead of return 512mA.
	 */
	ret = bq2419x_write_mask(BQ2419X_REG_CCC,
		BQ2419X_REG_CCC_FORCE_20PCT_MASK,
		BQ2419X_REG_CCC_FORCE_20PCT_SHIFT,
		force_20pct_en);
	if (ret < 0)
		return ret;

	val = params_to_reg(bq2419x_ichg_values, array_size, value);

	hwlog_info("set_charge_current [%x]=0x%x\n", BQ2419X_REG_CCC, val);

	return bq2419x_write_mask(BQ2419X_REG_CCC,
		BQ2419X_REG_CCC_ICHG_MASK,
		BQ2419X_REG_CCC_ICHG_SHIFT,
		val);
}

static int bq2419x_set_terminal_voltage(int value)
{
	int val;
	int array_size = ARRAY_SIZE(bq2419x_vreg_values);

	if (value > bq2419x_vreg_values[array_size - 1])
		return bq2419x_vreg_values[array_size - 1];
	else if (value < bq2419x_vreg_values[0])
		return bq2419x_vreg_values[0];

	val = params_to_reg(bq2419x_vreg_values, array_size, value);

	hwlog_info("set_terminal_voltage [%x]=0x%x\n", BQ2419X_REG_CVC, val);

	return bq2419x_write_mask(BQ2419X_REG_CVC,
		BQ2419X_REG_CVC_VREG_MASK,
		BQ2419X_REG_CVC_VREG_SHIFT,
		val);
}

static int bq2419x_set_dpm_voltage(int value)
{
	int val;
	int array_size = ARRAY_SIZE(bq2419x_vindpm_values);

	if (value > bq2419x_vindpm_values[array_size - 1])
		return bq2419x_vindpm_values[array_size - 1];
	else if (value < bq2419x_vindpm_values[0])
		return bq2419x_vindpm_values[0];

	val = params_to_reg(bq2419x_vindpm_values, array_size, value);

	hwlog_info("set_dpm_voltage [%x]=0x%x\n", BQ2419X_REG_ISC, val);

	return bq2419x_write_mask(BQ2419X_REG_ISC,
		BQ2419X_REG_ISC_VINDPM_MASK,
		BQ2419X_REG_ISC_VINDPM_SHIFT,
		val);
}

static int bq2419x_set_terminal_current(int value)
{
	int val;
	int array_size = ARRAY_SIZE(bq2419x_iterm_values);

	if (value > bq2419x_iterm_values[array_size - 1])
		return bq2419x_iterm_values[array_size - 1];
	else if (value < bq2419x_iterm_values[0])
		return bq2419x_iterm_values[0];

	val = params_to_reg(bq2419x_iterm_values, array_size, value);

	hwlog_info("set_terminal_current [%x]=0x%x\n", BQ2419X_REG_PCTCC, val);

	return bq2419x_write_mask(BQ2419X_REG_PCTCC,
		BQ2419X_REG_PCTCC_ITERM_MASK,
		BQ2419X_REG_PCTCC_ITERM_SHIFT,
		val);
}

static int bq2419x_set_charge_enable(int enable)
{
	struct bq2419x_device_info *di = g_bq2419x_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	gpio_set_value(di->gpio_cd, !enable);

	return bq2419x_write_mask(BQ2419X_REG_POC,
		BQ2419X_REG_POC_CHG_CONFIG_MASK,
		BQ2419X_REG_POC_CHG_CONFIG_SHIFT,
		enable);
}

static int bq2419x_set_otg_enable(int enable)
{
	u8 val;
	struct bq2419x_device_info *di = g_bq2419x_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	gpio_set_value(di->gpio_cd, !enable);

	if (enable)
		val = 1 << 1; /* 1 left shift bit1 */
	else
		val = 0 << 1; /* 0 left shift bit1 */

	/* notice:
	 * why enable irq when entry to OTG mode only?
	 * because we care VBUS overloaded OCP or OVP's interrupt in boost mode
	 */
	if ((!di->irq_active) && (enable)) {
		di->irq_active = 1; /* active */
		enable_irq(di->irq_int);
	} else if ((di->irq_active) && (!enable)) {
		di->irq_active = 0; /* inactive */
		disable_irq(di->irq_int);
	}

	return bq2419x_write_mask(BQ2419X_REG_POC,
		BQ2419X_REG_POC_CHG_CONFIG_MASK,
		BQ2419X_REG_POC_CHG_CONFIG_SHIFT,
		val);
}

static int bq2419x_set_term_enable(int enable)
{
	return bq2419x_write_mask(BQ2419X_REG_CTTC,
		BQ2419X_REG_CTTC_EN_TERM_MASK,
		BQ2419X_REG_CTTC_EN_TERM_SHIFT,
		enable);
}

static int bq2419x_get_charge_state(unsigned int *state)
{
	u8 reg = 0;
	int ret;

	if (!state)
		return -1;

	ret = bq2419x_read_byte(BQ2419X_REG_SS, &reg);

	hwlog_info("get_charge_state [%x]=0x%x\n", BQ2419X_REG_SS, reg);

	if ((reg & BQ2419X_REG_SS_PG) == BQ2419X_REG_SS_NOTPG)
		*state |= CHAGRE_STATE_NOT_PG;

	if ((reg & BQ2419X_REG_SS_DPM) == BQ2419X_REG_SS_DPM)
		*state |= CHAGRE_STATE_INPUT_DPM;

	if ((reg & BQ2419X_REG_SS_CHRGDONE) == BQ2419X_REG_SS_CHRGDONE)
		*state |= CHAGRE_STATE_CHRG_DONE;

	ret |= bq2419x_read_byte(BQ2419X_REG_F, &reg);
	ret |= bq2419x_read_byte(BQ2419X_REG_F, &reg);

	hwlog_info("get_charge_state [%x]=0x%x\n", BQ2419X_REG_F, reg);

	if ((reg & BQ2419X_REG_F_WDT_TIMEOUT) == BQ2419X_REG_F_WDT_TIMEOUT)
		*state |= CHAGRE_STATE_WDT_FAULT;

	if ((reg & BQ2419X_REG_F_VBUS_OVP) == BQ2419X_REG_F_VBUS_OVP)
		*state |= CHAGRE_STATE_VBUS_OVP;

	if ((reg & BQ2419X_REG_F_BATT_OVP) == BQ2419X_REG_F_BATT_OVP)
		*state |= CHAGRE_STATE_BATT_OVP;

	return ret;
}

static int bq2419x_reset_watchdog_timer(void)
{
	return bq2419x_write_mask(BQ2419X_REG_POC,
		BQ2419X_REG_POC_WDT_RESET_MASK,
		BQ2419X_REG_POC_WDT_RESET_SHIFT,
		0x01);
}

static int bq2419x_get_vilim_sample(void)
{
	int i;
	int retry_times = 3; /* retry 3 times */
	int v_sample;

	for (i = 0; i < retry_times; ++i) {
		v_sample = hisi_adc_get_value(adc_channel_iin);
		if (v_sample < 0)
			hwlog_err("adc read channel 10 fail\n");
		else
			break;
	}

	hwlog_info("get vilim_sample=%d\n", v_sample);
	return v_sample;
}

static int bq2419x_get_ilim(void)
{
	int i;
	int cnt = 0;
	int v_temp;
	int delay_times = 100; /* 100ms */
	int sample_num = 5; /* use 5 samples to get an average value */
	int sum = 0;
	int kilim = 485; /* based bq2419x spec */

	for (i = 0; i < sample_num; ++i) {
		v_temp = bq2419x_get_vilim_sample();
		if (v_temp >= 0) {
			sum += v_temp;
			++cnt;
		} else {
			hwlog_err("get_ilim get v_temp fail\n");
		}

		msleep(delay_times);
	}

	if (cnt > 0)
		return (sum * kilim) / (rilim * cnt);

	hwlog_err("use 0 as default vilim\n");
	return 0;
}

static int bq2419x_check_charger_plugged(void)
{
	u8 reg = 0;
	int ret;

	ret = bq2419x_read_byte(BQ2419X_REG_SS, &reg);
	if (ret < 0)
		return FALSE;

	hwlog_info("check_charger_plugged [%x]=0x%x\n", BQ2419X_REG_SS, reg);

	if ((reg & BQ2419X_REG_SS_VBUS_STAT_MASK) ==
		BQ2419X_REG_SS_VBUS_PLUGGED)
		return TRUE;

	return FALSE;
}

static int bq2419x_check_input_dpm_state(void)
{
	u8 reg = 0;
	int ret;

	ret = bq2419x_read_byte(BQ2419X_REG_SS, &reg);
	if (ret < 0)
		return FALSE;

	hwlog_info("check_input_dpm_state [%x]=0x%x\n", BQ2419X_REG_SS, reg);

	if (reg & BQ2419X_REG_SS_DPM_STAT_MASK)
		return TRUE;

	return FALSE;
}

static int bq2419x_dump_register(char *reg_value, int size)
{
	u8 reg[BQ2419X_REG_NUM] = {0};
	char buff[BUF_LEN] = {0};
	int i;
	int ret;

	if (!reg_value)
		return 0;

	memset(reg_value, 0, size);
	snprintf(buff, BUF_LEN, "%-8.2d", bq2419x_get_ilim());
	strncat(reg_value, buff, strlen(buff));

	for (i = 0; i < BQ2419X_REG_NUM; i++) {
		ret = bq2419x_read_byte(i, &reg[i]);
		if (ret)
			hwlog_err("dump_register read fail\n");

		snprintf(buff, BUF_LEN, "0x%-8.2x", reg[i]);
		strncat(reg_value, buff, strlen(buff));
	}

	return 0;
}

static int bq2419x_get_register_head(char *reg_head, int size)
{
	char buff[BUF_LEN] = {0};
	int i;

	if (!reg_head)
		return 0;

	memset(reg_head, 0, size);
	snprintf(buff, BUF_LEN, "Ibus    ");
	strncat(reg_head, buff, strlen(buff));

	for (i = 0; i < BQ2419X_REG_NUM; i++) {
		snprintf(buff, BUF_LEN, "Reg[%d]    ", i);
		strncat(reg_head, buff, strlen(buff));
	}

	return 0;
}

static int bq2419x_set_batfet_disable(int disable)
{
	return bq2419x_write_mask(BQ2419X_REG_MOC,
		BQ2419X_REG_MOC_BATFET_DISABLE_MASK,
		BQ2419X_REG_MOC_BATFET_DISABLE_SHIFT,
		disable);
}

static int bq2419x_set_watchdog_timer(int value)
{
	u8 val;
	int array_size = ARRAY_SIZE(bq2419x_watchdog_values);

	if (value > bq2419x_watchdog_values[array_size - 1])
		return bq2419x_watchdog_values[array_size - 1];
	else if (value < bq2419x_watchdog_values[0])
		return bq2419x_watchdog_values[0];

	val = params_to_reg(bq2419x_watchdog_values, array_size, value);

	hwlog_info("set_watchdog_timer [%x]=0x%x\n", BQ2419X_REG_CTTC, val);

	return bq2419x_write_mask(BQ2419X_REG_CTTC,
		BQ2419X_REG_CTTC_WATCHDOG_MASK,
		BQ2419X_REG_CTTC_WATCHDOG_SHIFT,
		val);
}

static int bq2419x_set_charger_hiz(int enable)
{
	int ret;

	if (enable > 0)
		ret = bq2419x_write_mask(BQ2419X_REG_ISC,
			BQ2419X_REG_ISC_EN_HIZ_MASK,
			BQ2419X_REG_ISC_EN_HIZ_SHIFT,
			TRUE);
	else
		ret = bq2419x_write_mask(BQ2419X_REG_ISC,
			BQ2419X_REG_ISC_EN_HIZ_MASK,
			BQ2419X_REG_ISC_EN_HIZ_SHIFT,
			FALSE);

	return ret;
}

static struct charge_device_ops bq2419x_ops = {
	.chip_init = bq2419x_chip_init,
	.dev_check = bq2419x_device_check,
	.set_input_current = bq2419x_set_input_current,
	.set_charge_current = bq2419x_set_charge_current,
	.set_terminal_voltage = bq2419x_set_terminal_voltage,
	.set_dpm_voltage = bq2419x_set_dpm_voltage,
	.set_terminal_current = bq2419x_set_terminal_current,
	.set_charge_enable = bq2419x_set_charge_enable,
	.set_otg_enable = bq2419x_set_otg_enable,
	.set_term_enable = bq2419x_set_term_enable,
	.get_charge_state = bq2419x_get_charge_state,
	.reset_watchdog_timer = bq2419x_reset_watchdog_timer,
	.dump_register = bq2419x_dump_register,
	.get_register_head = bq2419x_get_register_head,
	.set_watchdog_timer = bq2419x_set_watchdog_timer,
	.set_batfet_disable = bq2419x_set_batfet_disable,
	.get_ibus = bq2419x_get_ilim,
	.check_charger_plugged = bq2419x_check_charger_plugged,
	.check_input_dpm_state = bq2419x_check_input_dpm_state,
	.set_charger_hiz = bq2419x_set_charger_hiz,
	.get_charge_current = NULL,
};

static struct charger_otg_device_ops bq2419x_otg_ops = {
	.chip_name = "bq2419x",
	.otg_set_charger_enable = bq2419x_set_charge_enable,
	.otg_set_enable = bq2419x_set_otg_enable,
	.otg_set_current = NULL,
	.otg_set_watchdog_timer = bq2419x_set_watchdog_timer,
	.otg_reset_watchdog_timer = bq2419x_reset_watchdog_timer,
};

static void bq2419x_irq_work(struct work_struct *work)
{
	struct bq2419x_device_info *di = NULL;
	u8 reg = 0;
	int ret;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct bq2419x_device_info, irq_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	msleep(100); /* sleep 100ms */

	ret = bq2419x_read_byte(BQ2419X_REG_F, &reg);
	ret |= bq2419x_read_byte(BQ2419X_REG_F, &reg);
	if (ret)
		hwlog_err("irq_work read fail\n");

	hwlog_err("boost_ovp_reg [%x]=0x%x\n", BQ2419X_REG_F, reg);

	if (reg & BQ2419X_REG_F_BOOST_OCP) {
		hwlog_info("CHARGE_FAULT_BOOST_OCP happened\n");

		atomic_notifier_call_chain(&fault_notifier_list,
			CHARGE_FAULT_BOOST_OCP, NULL);
	}

	if (di->irq_active == 0) {
		di->irq_active = 1;
		enable_irq(di->irq_int);
	}
}

static irqreturn_t bq2419x_interrupt(int irq, void *_di)
{
	struct bq2419x_device_info *di = _di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	hwlog_info("bq2419x int happened\n");

	if (di->irq_active == 1) {
		di->irq_active = 0;
		disable_irq_nosync(di->irq_int);
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("the irq is not enable, do nothing\n");
	}

	return IRQ_HANDLED;
}

static int bq2419x_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct bq2419x_device_info *di = NULL;
	struct device_node *np = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	hwlog_info("probe begin\n");

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_bq2419x_dev = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	INIT_WORK(&di->irq_work, bq2419x_irq_work);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rilim", &rilim, BQ2419X_RILIM_220_OHM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adc_channel_iin", &adc_channel_iin,
		BQ2419X_ADC_CHANNEL_IIN_10);

	/* set gpio to control CD pin to disable/enable bq2419x IC */
	ret = power_gpio_config_output(power_gpio_tag(HWLOG_TAG), np,
		"gpio_cd", "charger_cd", &di->gpio_cd, 0);
	if (ret)
		goto bq2419x_fail_0;

	ret = power_gpio_config_interrupt(power_gpio_tag(HWLOG_TAG), np,
		"gpio_int", "charger_int", &di->gpio_int, &di->irq_int);
	if (ret)
		goto bq2419x_fail_1;

	ret = request_irq(di->irq_int, bq2419x_interrupt, IRQF_TRIGGER_FALLING,
		"charger_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		goto bq2419x_fail_2;
	}

	disable_irq(di->irq_int);
	di->irq_active = 0;

	ret = charge_ops_register(&bq2419x_ops);
	if (ret) {
		hwlog_err("bq2419x charge ops register fail\n");
		goto bq2419x_fail_3;
	}

	ret = charger_otg_ops_register(&bq2419x_otg_ops);
	if (ret) {
		hwlog_err("bq2419x charger_otg ops register fail\n");
		goto bq2419x_fail_3;
	}

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = 0;
		power_dev_info->ver_id = 0;
	}

	hwlog_info("probe end\n");
	return 0;

bq2419x_fail_3:
	free_irq(di->irq_int, di);
bq2419x_fail_2:
	gpio_free(di->gpio_int);
bq2419x_fail_1:
	gpio_free(di->gpio_cd);
bq2419x_fail_0:
	kfree(di);
	g_bq2419x_dev = NULL;

	return ret;
}

static int bq2419x_remove(struct i2c_client *client)
{
	struct bq2419x_device_info *di = i2c_get_clientdata(client);

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	gpio_set_value(di->gpio_cd, 1);

	if (di->gpio_cd)
		gpio_free(di->gpio_cd);

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	kfree(di);
	g_bq2419x_dev = NULL;

	hwlog_info("remove end\n");
	return 0;
}

MODULE_DEVICE_TABLE(i2c, bq24192);
static const struct of_device_id bq2419x_of_match[] = {
	{
		.compatible = "huawei,bq2419x_charger",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id bq2419x_i2c_id[] = {
	{ "bq2419x_charger", 0 }, {}
};

static struct i2c_driver bq2419x_driver = {
	.probe = bq2419x_probe,
	.remove = bq2419x_remove,
	.id_table = bq2419x_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "bq2419x_charger",
		.of_match_table = of_match_ptr(bq2419x_of_match),
	},
};

static int __init bq2419x_init(void)
{
	return i2c_add_driver(&bq2419x_driver);
}

static void __exit bq2419x_exit(void)
{
	i2c_del_driver(&bq2419x_driver);
}

module_init(bq2419x_init);
module_exit(bq2419x_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("bq2419x charger module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
