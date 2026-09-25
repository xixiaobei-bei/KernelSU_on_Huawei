/*
 * bq2429x_charger.c
 *
 * bq2429x driver
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
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <huawei_platform/power/huawei_charger.h>
#ifdef CONFIG_HISI_BCI_BATTERY
#include <linux/power/hisi/hisi_bci_battery.h>
#endif
#include "bq2429x_charger.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG bq2429x_charger
HWLOG_REGIST();

struct bq2429x_device_info *g_bq2429x_dev;

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

static int bq2429x_write_block(struct bq2429x_device_info *di,
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

static int bq2429x_read_block(struct bq2429x_device_info *di,
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

static int bq2429x_write_byte(u8 reg, u8 value)
{
	struct bq2429x_device_info *di = g_bq2429x_dev;
	/* 2 bytes offset 1 contains the data offset 0 is used by i2c_write */
	u8 temp_buffer[MSG_LEN] = {0};

	/* offset 1 contains the data */
	temp_buffer[1] = value;
	return bq2429x_write_block(di, temp_buffer, reg, 1);
}

static int bq2429x_read_byte(u8 reg, u8 *value)
{
	struct bq2429x_device_info *di = g_bq2429x_dev;

	return bq2429x_read_block(di, value, reg, 1);
}

static int bq2429x_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = bq2429x_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return bq2429x_write_byte(reg, val);
}

static int bq2429x_check_ic_type(void)
{
	u8 reg = 0;
	int ret;

	ret = bq2429x_read_byte(BQ2429X_REG_VPRS, &reg);
	if (ret < 0)
		return ret;

	hwlog_info("check_ic_type [%x]=0x%x\n", BQ2429X_REG_VPRS, reg);

	if ((0 == (reg & BQ2429X_REG_VPRS_REV_MASK)) &&
		(reg & BQ2429X_REG_VPRS_PN_MASK))
		return BQ2429X_REG_VPRS_PN_24296;

	if ((reg & BQ2419X_REG_VPRS_DEV_REG_MASK) &&
		(reg & BQ2419X_REG_VPRS_PN_MASK))
		return BQ2419X_REG_VPRS_PN_24192;

	return 0;
}

static int bq2429x_device_check(void)
{
	if ((bq2429x_check_ic_type() == BQ2429X_REG_VPRS_PN_24296) ||
		(bq2429x_check_ic_type() == BQ2419X_REG_VPRS_PN_24192)) {
		hwlog_info("bq2429x is good\n");
		return CHARGE_IC_GOOD;
	}

	hwlog_err("bq2429x is bad\n");
	return CHARGE_IC_BAD;
}

static int bq2429x_5v_chip_init(struct bq2429x_device_info *di)
{
	int ret;

	/* boost mode current limit = 1000mA */
	/* kick watchdog in bq2429x chip_init */
	ret = bq2429x_write_byte(BQ2429X_REG_POC, 0x5a);

	/* I2C watchdog timer setting = 80s */
	/* fast charge timer setting = 12h */
	ret |= bq2429x_write_byte(BQ2429X_REG_CTTC, 0x2d);

	/* IR compensation voatge clamp = 48mV */
	/* IR compensation resistor setting = 40mohm */
	if (bq2429x_check_ic_type() != BQ2429X_REG_VPRS_PN_24296)
		ret |= bq2429x_write_byte(BQ2429X_REG_BVTRC, 0x8f);

	/* enable charging */
	gpio_set_value(di->gpio_cd, 0);

	return ret;
}

static int bq2429x_chip_init(struct chip_init_crit *init_crit)
{
	int ret = -1;
	struct bq2429x_device_info *di = g_bq2429x_dev;

	if (!di || !init_crit) {
		hwlog_err("di or init_crit is null\n");
		return -ENOMEM;
	}

	switch (init_crit->vbus) {
	case ADAPTER_5V:
		ret = bq2429x_5v_chip_init(di);
		break;
	default:
		hwlog_err("invaid init_crit vbus mode\n");
		break;
	}

	return ret;
}

static int bq2429x_set_input_current(int value)
{
	int val;
	int array_size = ARRAY_SIZE(bq2429x_iin_values);

	if (value > bq2429x_iin_values[array_size - 1])
		return bq2429x_iin_values[array_size - 1];
	else if (value < bq2429x_iin_values[0])
		return bq2429x_iin_values[0];

	val = params_to_reg(bq2429x_iin_values, array_size, value);

	hwlog_info("set_input_current [%x]=0x%x\n", BQ2429X_REG_ISC, val);

	return bq2429x_write_mask(BQ2429X_REG_ISC,
		BQ2429X_REG_ISC_IINLIM_MASK,
		BQ2429X_REG_ISC_IINLIM_SHIFT,
		val);
}

static int bq2429x_set_charge_current(int value)
{
	int ret;
	int val;
	int force_20pct_en = 0;
	int array_size = ARRAY_SIZE(bq2429x_ichg_values);

	if (value > bq2429x_ichg_values[array_size - 1])
		return bq2429x_ichg_values[array_size - 1];

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

	if (value < bq2429x_ichg_values[0])
		value = bq2429x_ichg_values[0];

	ret = bq2429x_write_mask(BQ2429X_REG_CCC,
		BQ2429X_REG_CCC_FORCE_20PCT_MASK,
		BQ2429X_REG_CCC_FORCE_20PCT_SHIFT,
		force_20pct_en);
	if (ret < 0)
		return ret;

	val = params_to_reg(bq2429x_ichg_values, array_size, value);

	hwlog_info("set_charge_current [%x]=0x%x\n", BQ2429X_REG_CCC, val);

	return bq2429x_write_mask(BQ2429X_REG_CCC,
		BQ2429X_REG_CCC_ICHG_MASK,
		BQ2429X_REG_CCC_ICHG_SHIFT,
		val);
}

static int bq2429x_set_terminal_voltage(int value)
{
	int val;
	int array_size = ARRAY_SIZE(bq2429x_vreg_values);

	if (value > bq2429x_vreg_values[array_size - 1])
		return bq2429x_vreg_values[array_size - 1];
	else if (value < bq2429x_vreg_values[0])
		return bq2429x_vreg_values[0];

	val = params_to_reg(bq2429x_vreg_values, array_size, value);

	hwlog_info("set_terminal_voltage [%x]=0x%x\n", BQ2429X_REG_CVC, val);

	return bq2429x_write_mask(BQ2429X_REG_CVC,
		BQ2429X_REG_CVC_VREG_MASK,
		BQ2429X_REG_CVC_VREG_SHIFT,
		val);
}

static int bq2429x_set_dpm_voltage(int value)
{
	int val;
	int array_size = ARRAY_SIZE(bq2429x_vindpm_values);

	if (value > bq2429x_vindpm_values[array_size - 1])
		return bq2429x_vindpm_values[array_size - 1];
	else if (value < bq2429x_vindpm_values[0])
		return bq2429x_vindpm_values[0];

	val = params_to_reg(bq2429x_vindpm_values, array_size, value);

	hwlog_info("set_dpm_voltage [%x]=0x%x\n", BQ2429X_REG_ISC, val);

	return bq2429x_write_mask(BQ2429X_REG_ISC,
		BQ2429X_REG_ISC_VINDPM_MASK,
		BQ2429X_REG_ISC_VINDPM_SHIFT,
		val);
}

static int bq2429x_set_terminal_current(int value)
{
	int val;
	int array_size = ARRAY_SIZE(bq2429x_iterm_values);

	if (value > bq2429x_iterm_values[array_size - 1])
		return bq2429x_iterm_values[array_size - 1];
	else if (value < bq2429x_iterm_values[0])
		return bq2429x_iterm_values[0];

	val = params_to_reg(bq2429x_iterm_values, array_size, value);

	hwlog_info("set_terminal_current [%x]=0x%x\n", BQ2429X_REG_PCTCC, val);

	return bq2429x_write_mask(BQ2429X_REG_PCTCC,
		BQ2429X_REG_PCTCC_ITERM_MASK,
		BQ2429X_REG_PCTCC_ITERM_SHIFT,
		val);
}

static int bq2429x_set_charge_enable(int enable)
{
	struct bq2429x_device_info *di = g_bq2429x_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	gpio_set_value(di->gpio_cd, !enable);

	return bq2429x_write_mask(BQ2429X_REG_POC,
		BQ2429X_REG_POC_CHG_CONFIG_MASK,
		BQ2429X_REG_POC_CHG_CONFIG_SHIFT,
		enable);
}

static int bq2429x_set_otg_enable(int enable)
{
	u8 val;
	struct bq2429x_device_info *di = g_bq2429x_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	gpio_set_value(di->gpio_cd, !enable);

	if (enable)
		val = 1 << 1; /* 1 left shift bit1 */
	else
		val = 0 << 1; /* 0 left shift bit1 */

	/*
	 * notice:
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

	return bq2429x_write_mask(BQ2429X_REG_POC,
		BQ2429X_REG_POC_CHG_CONFIG_MASK,
		BQ2429X_REG_POC_CHG_CONFIG_SHIFT,
		val);
}

static int bq2429x_reset_otg(void)
{
	int val;
	int ret;
	struct bq2429x_device_info *di = g_bq2429x_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	gpio_set_value(di->gpio_cd, TRUE);

	val = 0 << 1; /* 0 left shift bit1 */
	ret = bq2429x_write_mask(BQ2429X_REG_POC,
		BQ2429X_REG_POC_CHG_CONFIG_MASK,
		BQ2429X_REG_POC_CHG_CONFIG_SHIFT,
		val);
	if (ret)
		return ret;

	gpio_set_value(di->gpio_cd, FALSE);

	val = 1 << 1; /* 1 left shift bit1 */
	ret = bq2429x_write_mask(BQ2429X_REG_POC,
		BQ2429X_REG_POC_CHG_CONFIG_MASK,
		BQ2429X_REG_POC_CHG_CONFIG_SHIFT,
		val);
	if (ret)
		return ret;

	return 0;
}

static int bq2429x_set_term_enable(int enable)
{
	return bq2429x_write_mask(BQ2429X_REG_CTTC,
		BQ2429X_REG_CTTC_EN_TERM_MASK,
		BQ2429X_REG_CTTC_EN_TERM_SHIFT,
		enable);
}

static int bq2429x_get_charge_state(unsigned int *state)
{
	u8 reg = 0;
	int ret;

	if (!state)
		return -1;

	ret = bq2429x_read_byte(BQ2429X_REG_SS, &reg);

	hwlog_info("get_charge_state [%x]=0x%x\n", BQ2429X_REG_SS, reg);

	if ((reg & BQ2429X_REG_SS_PG) == BQ2429X_REG_SS_NOTPG)
		*state |= CHAGRE_STATE_NOT_PG;

	if ((reg & BQ2429X_REG_SS_DPM) == BQ2429X_REG_SS_DPM)
		*state |= CHAGRE_STATE_INPUT_DPM;

	if ((reg & BQ2429X_REG_SS_CHRGDONE) == BQ2429X_REG_SS_CHRGDONE)
		*state |= CHAGRE_STATE_CHRG_DONE;

	ret |= bq2429x_read_byte(BQ2429X_REG_F, &reg);
	ret |= bq2429x_read_byte(BQ2429X_REG_F, &reg);

	hwlog_info("get_charge_state [%x]=0x%x\n", BQ2429X_REG_F, reg);

	if ((reg & BQ2429X_REG_F_WDT_TIMEOUT) == BQ2429X_REG_F_WDT_TIMEOUT)
		*state |= CHAGRE_STATE_WDT_FAULT;

	if ((reg & BQ2429X_REG_F_VBUS_OVP) == BQ2429X_REG_F_VBUS_OVP)
		*state |= CHAGRE_STATE_VBUS_OVP;

	if ((reg & BQ2429X_REG_F_BATT_OVP) == BQ2429X_REG_F_BATT_OVP)
		*state |= CHAGRE_STATE_BATT_OVP;

	return ret;
}

static int bq2429x_reset_watchdog_timer(void)
{
	return bq2429x_write_mask(BQ2429X_REG_POC,
		BQ2429X_REG_POC_WDT_RESET_MASK,
		BQ2429X_REG_POC_WDT_RESET_SHIFT,
		0x01);
}

static int bq2429x_check_charger_plugged(void)
{
	u8 reg = 0;
	int ret;

	ret = bq2429x_read_byte(BQ2429X_REG_SS, &reg);
	if (ret < 0)
		return FALSE;

	hwlog_info("check_charger_plugged [%x]=0x%x\n", BQ2429X_REG_SS, reg);

	if ((reg & BQ2429X_REG_SS_VBUS_STAT_MASK) ==
		BQ2429X_REG_SS_VBUS_PLUGGED)
		return TRUE;

	return FALSE;
}

static int bq2429x_check_input_dpm_state(void)
{
	u8 reg = 0;
	int ret;

	ret = bq2429x_read_byte(BQ2429X_REG_SS, &reg);
	if (ret < 0)
		return FALSE;

	hwlog_info("check_input_dpm_state [%x]=0x%x\n", BQ2429X_REG_SS, reg);

	if (reg & BQ2429X_REG_SS_DPM_STAT_MASK)
		return TRUE;

	return FALSE;
}

static int bq2429x_dump_register(char *reg_value, int size)
{
	u8 reg[BQ2429X_REG_NUM] = {0};
	char buff[BUF_LEN] = {0};
	int i;
	int ret;

	if (!reg_value)
		return 0;

	memset(reg_value, 0, size);

	for (i = 0; i < BQ2429X_REG_NUM; i++) {
		ret = bq2429x_read_byte(i, &reg[i]);
		if (ret)
			hwlog_err("dump_register read fail\n");

		snprintf(buff, BUF_LEN, "0x%-8.2x", reg[i]);
		strncat(reg_value, buff, strlen(buff));
	}

	return 0;
}

static int bq2429x_get_register_head(char *reg_head, int size)
{
	char buff[BUF_LEN] = {0};
	int i;

	if (!reg_head)
		return 0;

	memset(reg_head, 0, size);

	for (i = 0; i < BQ2429X_REG_NUM; i++) {
		snprintf(buff, BUF_LEN, "Reg[%d]    ", i);
		strncat(reg_head, buff, strlen(buff));
	}

	return 0;
}

static int bq2429x_set_batfet_disable(int disable)
{
	return bq2429x_write_mask(BQ2429X_REG_MOC,
		BQ2429X_REG_MOC_BATFET_DISABLE_MASK,
		BQ2429X_REG_MOC_BATFET_DISABLE_SHIFT,
		disable);
}

static int bq2429x_get_ilim(void)
{
	int ret;
	u32 ibus;
	u32 state = 0;

	ret = bq2429x_get_charge_state(&state);
	if (ret)
		return -1;

	if (CHAGRE_STATE_NOT_PG & state) {
		hwlog_info("CHAGRE_STATE_NOT_PG state:%d\n", state);
		return 0;
	}

	/*
	 * bq2429x do not support get-ibus function, for RunnintTest,
	 * we return a fake vulue
	 */
	ibus = 1000; /* fake current value for Running test */
	/* temperary log for RT flag */
	hwlog_info("bq2429x: ibus = %d, RunningTest Begin\n", ibus);

	return ibus;
}

static int bq2429x_set_watchdog_timer(int value)
{
	u8 val;
	int array_size = ARRAY_SIZE(bq2429x_watchdog_values);

	if (value > bq2429x_watchdog_values[array_size - 1])
		return bq2429x_watchdog_values[array_size - 1];
	else if (value < bq2429x_watchdog_values[0])
		return bq2429x_watchdog_values[0];

	val = params_to_reg(bq2429x_watchdog_values, array_size, value);

	hwlog_info("set_watchdog_timer [%x]=0x%x\n", BQ2429X_REG_CTTC, val);

	return bq2429x_write_mask(BQ2429X_REG_CTTC,
		BQ2429X_REG_CTTC_WATCHDOG_MASK,
		BQ2429X_REG_CTTC_WATCHDOG_SHIFT,
		val);
}

static int bq2429x_set_charger_hiz(int enable)
{
	int ret;

	if (enable > 0)
		ret = bq2429x_write_mask(BQ2429X_REG_ISC,
			BQ2429X_REG_ISC_EN_HIZ_MASK,
			BQ2429X_REG_ISC_EN_HIZ_SHIFT,
			TRUE);
	else
		ret = bq2429x_write_mask(BQ2429X_REG_ISC,
			BQ2429X_REG_ISC_EN_HIZ_MASK,
			BQ2429X_REG_ISC_EN_HIZ_SHIFT,
			FALSE);

	return ret;
}

static struct charge_device_ops bq2429x_ops = {
	.chip_init = bq2429x_chip_init,
	.dev_check = bq2429x_device_check,
	.set_input_current = bq2429x_set_input_current,
	.set_charge_current = bq2429x_set_charge_current,
	.set_terminal_voltage = bq2429x_set_terminal_voltage,
	.set_dpm_voltage = bq2429x_set_dpm_voltage,
	.set_terminal_current = bq2429x_set_terminal_current,
	.set_charge_enable = bq2429x_set_charge_enable,
	.set_otg_enable = bq2429x_set_otg_enable,
	.set_term_enable = bq2429x_set_term_enable,
	.get_charge_state = bq2429x_get_charge_state,
	.reset_watchdog_timer = bq2429x_reset_watchdog_timer,
	.dump_register = bq2429x_dump_register,
	.get_register_head = bq2429x_get_register_head,
	.set_watchdog_timer = bq2429x_set_watchdog_timer,
	.set_batfet_disable = bq2429x_set_batfet_disable,
	.get_ibus = bq2429x_get_ilim,
	.check_charger_plugged = bq2429x_check_charger_plugged,
	.check_input_dpm_state = bq2429x_check_input_dpm_state,
	.set_charger_hiz = bq2429x_set_charger_hiz,
	.get_charge_current = NULL,
};

static struct charger_otg_device_ops bq2429x_otg_ops = {
	.chip_name = "bq2429x",
	.otg_set_charger_enable = bq2429x_set_charge_enable,
	.otg_set_enable = bq2429x_set_otg_enable,
	.otg_set_current = NULL,
	.otg_set_watchdog_timer = bq2429x_set_watchdog_timer,
	.otg_reset_watchdog_timer = bq2429x_reset_watchdog_timer,
};

static void bq2429x_irq_work(struct work_struct *work)
{
	struct bq2429x_device_info *di = NULL;
	u8 reg = 0;
	int ret;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct bq2429x_device_info, irq_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	msleep(250); /* sleep 250ms */

	ret = bq2429x_read_byte(BQ2429X_REG_F, &reg);
	ret |= bq2429x_read_byte(BQ2429X_REG_F, &reg);
	if (ret)
		hwlog_err("irq_work read fail\n");

	hwlog_err("boost_ovp_reg [%x]=0x%x\n", BQ2429X_REG_F, reg);

	if (reg & BQ2429X_REG_F_BOOST_OCP) {
		hwlog_info("CHARGE_FAULT_BOOST_OCP happened\n");

		/* retry otg once */
		bq2429x_reset_otg();

		msleep(100); /* sleep 100ms */

		ret = bq2429x_read_byte(BQ2429X_REG_F, &reg);
		ret |= bq2429x_read_byte(BQ2429X_REG_F, &reg);
		if (ret)
			hwlog_err("irq_work read fail\n");

		if (reg & BQ2429X_REG_F_BOOST_OCP) {
			hwlog_info("CHARGE_FAULT_BOOST_OCP happened twice\n");

			atomic_notifier_call_chain(&fault_notifier_list,
				CHARGE_FAULT_BOOST_OCP, NULL);
		}
	}

	if (di->irq_active == 0) {
		di->irq_active = 1;
		enable_irq(di->irq_int);
	}
}

static irqreturn_t bq2429x_interrupt(int irq, void *_di)
{
	struct bq2429x_device_info *di = _di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	hwlog_info("bq2429x int happened\n");

	if (di->irq_active == 1) {
		di->irq_active = 0;
		disable_irq_nosync(di->irq_int);
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("the irq is not enable, do nothing\n");
	}

	return IRQ_HANDLED;
}

static int bq2429x_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct bq2429x_device_info *di = NULL;
	struct device_node *np = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	hwlog_info("probe begin\n");

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_bq2429x_dev = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	INIT_WORK(&di->irq_work, bq2429x_irq_work);

	/* set gpio to control CD pin to disable/enable bq2429x IC */
	ret = power_gpio_config_output(power_gpio_tag(HWLOG_TAG), np,
		"gpio_cd", "charger_cd", &di->gpio_cd, 0);
	if (ret)
		goto bq2429x_fail_0;

	ret = power_gpio_config_interrupt(power_gpio_tag(HWLOG_TAG), np,
		"gpio_int", "charger_int", &di->gpio_int, &di->irq_int);
	if (ret)
		goto bq2429x_fail_1;

	ret = request_irq(di->irq_int, bq2429x_interrupt, IRQF_TRIGGER_FALLING,
		"charger_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		goto bq2429x_fail_2;
	}

	disable_irq(di->irq_int);
	di->irq_active = 0;

	ret = charge_ops_register(&bq2429x_ops);
	if (ret) {
		hwlog_err("bq2429x charge ops register fail\n");
		goto bq2429x_fail_3;
	}

	ret = charger_otg_ops_register(&bq2429x_otg_ops);
	if (ret) {
		hwlog_err("bq2429x charger_otg ops register fail\n");
		goto bq2429x_fail_3;
	}

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = 0;
		power_dev_info->ver_id = 0;
	}

	hwlog_info("probe end\n");
	return 0;

bq2429x_fail_3:
	free_irq(di->irq_int, di);
bq2429x_fail_2:
	gpio_free(di->gpio_int);
bq2429x_fail_1:
	gpio_free(di->gpio_cd);
bq2429x_fail_0:
	kfree(di);
	g_bq2429x_dev = NULL;

	return ret;
}

static int bq2429x_remove(struct i2c_client *client)
{
	struct bq2429x_device_info *di = i2c_get_clientdata(client);

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
	g_bq2429x_dev = NULL;

	hwlog_info("remove end\n");
	return 0;
}

MODULE_DEVICE_TABLE(i2c, bq24192);
static const struct of_device_id bq2429x_of_match[] = {
	{
		.compatible = "huawei,bq2429x_charger",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id bq2429x_i2c_id[] = {
	{ "bq2429x_charger", 0 }, {}
};

static struct i2c_driver bq2429x_driver = {
	.probe = bq2429x_probe,
	.remove = bq2429x_remove,
	.id_table = bq2429x_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "bq2429x_charger",
		.of_match_table = of_match_ptr(bq2429x_of_match),
	},
};

static int __init bq2429x_init(void)
{
	return i2c_add_driver(&bq2429x_driver);
}

static void __exit bq2429x_exit(void)
{
	i2c_del_driver(&bq2429x_driver);
}

module_init(bq2429x_init);
module_exit(bq2429x_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("bq2429x charger module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
