/*
 * bq25882_charger.c
 *
 * bq25882 driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/hisi/hisi_adc.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <huawei_platform/power/huawei_charger.h>
#ifdef CONFIG_BCI_BATTERY
#include <linux/power/hisi/bci_battery.h>
#endif
#include <huawei_platform/power/battery_voltage.h>
#include <huawei_platform/power/series_batt_charger.h>
#include "bq25882_charger.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG bq25882_charger
HWLOG_REGIST();

#define ADC_CONTINUOUS 0x2
#define ADC_ONESHOT 0x3

static struct bq25882_device_info *g_bq25882_dev;

static int iin_set = IINLIM_MIN;
static bool g_shutdown_flag;

#define BUF_LEN                      26
#define CONV_RETRY                   8
#define GET_ADC_TIME                 5000
#define TIMEOUT_SEC                  100

static int bq25882_write_byte(u8 reg, u8 value)
{
	struct bq25882_device_info *di = g_bq25882_dev;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_write_byte(di->client, reg, value);
}

static int bq25882_read_byte(u8 reg, u8 *value)
{
	struct bq25882_device_info *di = g_bq25882_dev;

	if (!di) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	return power_i2c_u8_read_byte(di->client, reg, value);
}

static int bq25882_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = bq25882_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return bq25882_write_byte(reg, val);
}

static int bq25882_5v_chip_init(struct bq25882_device_info *di)
{
	int ret = 0;

	/* Fast Charge Current Limit 1000mA */
	ret |= bq25882_write_byte(BQ25882_REG_CCL, 0x54);

	/* iprechg = 150ma,iterm current = 150ma */
	ret |= bq25882_write_byte(BQ25882_REG_PTCL, 0x22);

	/*
	 * watchdog timer setting = 80s, fast charge timer setting = 12h,
	 * EN_TERM 0, EN_TIMER 1
	 */
	ret |= bq25882_write_byte(BQ25882_REG_CC1, 0x2d);

	/*
	 * Battery Precharge to Fast Charge Threshold 6,
	 * Battery Recharge 100mV
	 */
	ret |= bq25882_write_byte(BQ25882_REG_CC2, 0x7c);

	/* WD_RST 1, SYS_MIN 7.0V */
	ret |= bq25882_write_byte(BQ25882_REG_CC3, 0x4a);

	/* JEITA Low Temperature Current Setting 20% of ICHG */
	ret |= bq25882_write_byte(BQ25882_REG_CC4, 0x7d);

	/* buck mode current limit = 2A, OTG_VLIM 5.1V */
	ret |= bq25882_write_byte(BQ25882_REG_OC, 0xf6);

	/* enable charging */
	gpio_set_value(di->gpio_cd, 0);

	return ret;
}

static int bq25882_chip_init(struct chip_init_crit *init_crit)
{
	int ret = -1;
	struct bq25882_device_info *di = g_bq25882_dev;

	if (!di || !init_crit) {
		hwlog_err("di or init_crit is null\n");
		return -ENOMEM;
	}

	switch (init_crit->vbus) {
	case ADAPTER_5V:
		ret = bq25882_5v_chip_init(di);
		break;
	default:
		hwlog_err("invaid init_crit vbus mode\n");
		break;
	}

	return ret;
}

static int bq25882_device_check(void)
{
	int ret;
	u8 reg = 0xff;

	ret = bq25882_read_byte(BQ25882_REG_PI, &reg);
	if (ret)
		return CHARGE_IC_BAD;

	if ((reg & BQ25882_REG_PI_PN_MASK) == BQ25882 &&
		(reg & BQ25882_REG_PI_DEV_REV_MASK) == CHIP_REVISION) {
		hwlog_info("bq25882 is good\n");
		return CHARGE_IC_GOOD;
	}

	hwlog_err("bq25882 is bad\n");
	return CHARGE_IC_BAD;
}

static int bq25882_set_covn_start(int enable)
{
	return 0;
}

static int bq25882_set_input_current(int value)
{
	int limit_current;
	u8 iin_limit;

	limit_current = value;

	if (limit_current < IINLIM_MIN)
		limit_current = IINLIM_MIN;
	else if (limit_current > IINLIM_MAX)
		limit_current = IINLIM_MAX;

	iin_limit = (limit_current - IINLIM_OFFSET) / IINLIM_STEP;
	iin_set = iin_limit * IINLIM_STEP + IINLIM_OFFSET;

	hwlog_info("set_input_current [%x]=0x%x\n", BQ25882_REG_ICL, iin_limit);

	return bq25882_write_mask(BQ25882_REG_ICL,
		BQ25882_REG_ICL_IINDPM_MASK,
		BQ25882_REG_ICL_IINDPM_SHIFT,
		iin_limit);
}

static int bq25882_get_input_current_set(void)
{
	return iin_set;
}

static int bq25882_set_charge_current(int value)
{
	int currentma;
	u8 ichg;

	currentma = value;

	if (currentma < ICHG_MIN)
		currentma = ICHG_MIN;
	else if (currentma > ICHG_MAX)
		currentma = ICHG_MAX;

	ichg = currentma / ICHG_STEP;

	hwlog_info("set_charge_current [%x]=0x%x\n", BQ25882_REG_CCL, ichg);

	return bq25882_write_mask(BQ25882_REG_CCL,
		BQ25882_REG_CCL_ICHG_MASK,
		BQ25882_REG_CCL_ICHG_SHIFT,
		ichg);
}

static int bq25882_set_terminal_voltage(int value)
{
	int voltagemv;
	u8 voreg;

	voltagemv = value;

	if (voltagemv < VCHARGE_MIN)
		voltagemv = VCHARGE_MIN;
	else if (voltagemv > VCHARGE_MAX)
		voltagemv = VCHARGE_MAX;

	voreg = (voltagemv - VCHARGE_MIN) / VCHARGE_STEP;

	hwlog_info("set_terminal_voltage [%x]=0x%x\n", BQ25882_REG_BVRL, voreg);

	return bq25882_write_byte(BQ25882_REG_BVRL, voreg);
}

static int bq25882_set_dpm_voltage(int value)
{
	int vindpm_voltage;
	u8 vindpm;

	vindpm_voltage = value;

	if (vindpm_voltage > VINDPM_MAX)
		vindpm_voltage = VINDPM_MAX;
	else if (vindpm_voltage < VINDPM_MIN)
		vindpm_voltage = VINDPM_MIN;

	vindpm = (vindpm_voltage - VINDPM_OFFSET) / VINDPM_STEP;

	hwlog_info("set_dpm_voltage [%x]=0x%x\n", BQ25882_REG_IVL, vindpm);

	return bq25882_write_mask(BQ25882_REG_IVL,
		BQ25882_REG_IVL_VINDPM_MASK,
		BQ25882_REG_IVL_VINDPM_SHIFT,
		vindpm);
}

static int bq25882_set_terminal_current(int value)
{
	int term_currentma;
	u8 iterm_reg;

	term_currentma = value;

	if (term_currentma < ITERM_MIN)
		term_currentma = ITERM_MIN;
	else if (term_currentma > ITERM_MAX)
		term_currentma = ITERM_MAX;

	iterm_reg = (term_currentma - ITERM_OFFSET) / ITERM_STEP;

	hwlog_info("set_terminal_current [%x]=0x%x\n",
		BQ25882_REG_PTCL, iterm_reg);

	return bq25882_write_mask(BQ25882_REG_PTCL,
		BQ25882_REG_PTCL_ITERM_MASK,
		BQ25882_REG_PTCL_ITERM_SHIFT,
		iterm_reg);
}

static int bq25882_set_charge_enable(int enable)
{
	struct bq25882_device_info *di = g_bq25882_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	gpio_set_value(di->gpio_cd, !enable);

	return bq25882_write_mask(BQ25882_REG_CC2,
		BQ25882_REG_CC2_EN_CHG_MASK,
		BQ25882_REG_CC2_EN_CHG_SHIFT,
		enable);
}

static int bq25882_set_otg_enable(int enable)
{
	struct bq25882_device_info *di = g_bq25882_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (g_shutdown_flag)
		return -1;

	gpio_set_value(di->gpio_cd, !enable);
	mutex_lock(&di->adc_lock);
	if (enable) {
		di->adc_mode = ADC_CONTINUOUS;
		if (bq25882_write_mask(BQ25882_REG_AC,
			BQ25882_REG_AC_ADC_CTRL_MASK,
			BQ25882_REG_AC_ADC_CTRL_SHIFT, ADC_CONTINUOUS)) {
			di->adc_mode = ADC_ONESHOT;
			hwlog_err("otg cfg adc continue err\n");
		}
	} else {
		di->adc_mode = ADC_ONESHOT;
		if (bq25882_write_mask(BQ25882_REG_AC,
			BQ25882_REG_AC_ADC_CTRL_MASK,
			BQ25882_REG_AC_ADC_CTRL_SHIFT, ADC_ONESHOT))
			hwlog_err("otg cfg adc one shot err\n");
	}
	mutex_unlock(&di->adc_lock);
	/* notice:
	 * why enable irq when entry to OTG mode only?
	 * because we care VBUS overloaded OCP or OVP's interrupt in buck mode
	 */
	if ((!di->irq_active) && (enable)) {
		di->irq_active = 1; /* active */
		enable_irq(di->irq_int);
	} else if ((di->irq_active) && (!enable)) {
		di->irq_active = 0; /* inactive */
		disable_irq(di->irq_int);
	}

	return bq25882_write_mask(BQ25882_REG_CC2,
		BQ25882_REG_CC2_EN_OTG_MASK,
		BQ25882_REG_CC2_EN_OTG_SHIFT,
		enable);
}

static int bq25882_set_term_enable(int enable)
{
	return bq25882_write_mask(BQ25882_REG_CC1,
		BQ25882_REG_CC1_EN_TERM_MASK,
		BQ25882_REG_CC1_EN_TERM_SHIFT,
		enable);
}

static int bq25882_get_charge_state(unsigned int *state)
{
	u8 reg = 0;
	int ret;

	if (!state)
		return -1;

	ret = bq25882_read_byte(BQ25882_REG_CS2, &reg);

	hwlog_info("get_charge_state [%x]=0x%x\n", BQ25882_REG_CS2, reg);

	if (!(reg & BQ25882_NOT_PG_STAT))
		*state |= CHAGRE_STATE_NOT_PG;

	ret |= bq25882_read_byte(BQ25882_REG_CS1, &reg);

	hwlog_info("get_charge_state [%x]=0x%x\n", BQ25882_REG_CS1, reg);

	if ((reg & BQ25882_REG_CS1_CHRG_STAT_MASK) ==
		BQ25882_CHGR_STAT_CHARGE_DONE)
		*state |= CHAGRE_STATE_CHRG_DONE;

	if ((reg & BQ25882_REG_CS1_CHRG_STAT_MASK) ==
		BQ25882_CHGR_STAT_CV_MODE)
		*state |= CHAGRE_STATE_CV_MODE;

	if (reg & BQ25882_WATCHDOG_FAULT)
		*state |= CHAGRE_STATE_WDT_FAULT;

	ret |= bq25882_read_byte(BQ25882_REG_NS, &reg);

	hwlog_info("get_charge_state [%x]=0x%x\n", BQ25882_REG_NS, reg);

	if (reg & BQ25882_REG_NS_TS_STAT_MASK)
		*state |= CHAGRE_STATE_NTC_FAULT;

	ret |= bq25882_read_byte(BQ25882_REG_FS, &reg);

	hwlog_info("get_charge_state [%x]=0x%x\n", BQ25882_REG_FS, reg);

	if (reg & BQ25882_REG_FS_VBUS_OVP_STAT_MASK)
		*state |= CHAGRE_STATE_VBUS_OVP;

	if (reg & BQ25882_REG_FS_BATOVP_STAT_MASK)
		*state |= CHAGRE_STATE_BATT_OVP;

	return ret;
}

static int bq25882_reset_watchdog_timer(void)
{
	return bq25882_write_mask(BQ25882_REG_CC3,
		BQ25882_REG_CC3_WD_RST_MASK,
		BQ25882_REG_CC3_WD_RST_SHIFT,
		TRUE);
}

static int bq25882_get_adc_ibus(void)
{
	s16 reg_adc;
	u8 reg_adc1 = 0;
	u8 reg_adc0 = 0;
	int ret;
	int ibus_ma;

	ret = bq25882_read_byte(BQ25882_REG_IBUS_ADC1, &reg_adc1);
	ret += bq25882_read_byte(BQ25882_REG_IBUS_ADC0, &reg_adc0);
	if (ret)
		return -1;

	reg_adc = (reg_adc1 << 8) | reg_adc0;
	ibus_ma = (int)reg_adc * BQ25882_REG_IBUS_ADC_STEP_MA;
	hwlog_info("get_adc_ibus ibus is %dmA\n", ibus_ma);
	return ibus_ma;
}

static int bq25882_get_adc_vbus(void)
{
	s16 reg_adc;
	u8 reg_adc1 = 0;
	u8 reg_adc0 = 0;
	int vbus;
	int ret;

	ret = bq25882_read_byte(BQ25882_REG_VBUS_ADC1, &reg_adc1);
	ret += bq25882_read_byte(BQ25882_REG_VBUS_ADC0, &reg_adc0);
	if (ret)
		return -1;

	reg_adc = (reg_adc1 << 8) | reg_adc0;
	vbus = (int)reg_adc * BQ25882_REG_VBUS_ADC_STEP_MV;
	hwlog_info("get_adc_vbus vbus is %dmV\n", vbus);
	return vbus;
}

static int bq25882_get_adc_vbat(void)
{
	s16 reg_adc;
	u8 reg_adc1 = 0;
	u8 reg_adc0 = 0;
	int ret;
	int vbat_mv;

	ret = bq25882_read_byte(BQ25882_REG_VBAT_ADC1, &reg_adc1);
	ret += bq25882_read_byte(BQ25882_REG_VBAT_ADC0, &reg_adc0);
	if (ret)
		return -1;

	reg_adc = (reg_adc1 << 8) | reg_adc0;
	vbat_mv = (int)reg_adc * BQ25882_REG_VBAT_ADC_STEP_MV;
	hwlog_info("get_adc_vbat vbat is %dmV\n", vbat_mv);
	return vbat_mv;
}

static int bq25882_get_ibus_ma(void)
{
	if (!g_bq25882_dev) {
		hwlog_err("g_bq25882_dev is null\n");
		return -1;
	}

	return g_bq25882_dev->adc_value.ibus;
}

static int bq25882_get_vbus_mv(unsigned int *vbus_mv)
{
	if (!g_bq25882_dev || !vbus_mv) {
		hwlog_err("g_bq25882_dev or vbus_mv is null\n");
		return -1;
	}

	*vbus_mv = g_bq25882_dev->adc_value.vbus;
	return 0;
}

static int bq25882_get_vbat_mv(void)
{
	if (!g_bq25882_dev) {
		hwlog_err("g_bq25882_dev is null\n");
		return -1;
	}

	return g_bq25882_dev->adc_value.vbat;
}

static int bq25882_check_input_dpm_state(void)
{
	u8 reg = 0;
	int ret;

	ret = bq25882_read_byte(BQ25882_REG_CS1, &reg);
	if (ret < 0)
		return FALSE;

	hwlog_info("check_input_dpm_state [%x]=0x%x\n", BQ25882_REG_CS1, reg);

	if ((reg & BQ25882_REG_CS1_IINDPM_STAT_MASK) ||
		(reg & BQ25882_REG_CS1_VINDPM_STAT_MASK))
		return TRUE;

	return FALSE;
}

static int bq25882_set_otg_current(int value)
{
	int otg_currentma;
	u8 iotg_reg;

	otg_currentma = value;

	if (otg_currentma < OTG_ILIM_MIN)
		otg_currentma = OTG_ILIM_MIN;
	else if (otg_currentma > OTG_ILIM_MAX)
		otg_currentma = OTG_ILIM_MAX;

	iotg_reg = (otg_currentma - OTG_ILIM_OFFSET) / OTG_ILIM_STEP;

	hwlog_info("set_otg_current [%x]=0x%x\n",
		BQ25882_REG_OC, iotg_reg);

	return bq25882_write_mask(BQ25882_REG_OC,
		BQ25882_REG_OC_OTG_ILIM_MASK,
		BQ25882_REG_OC_OTG_ILIM_SHIFT,
		iotg_reg);
}

static int bq25882_dump_register(char *reg_value, int size, void *dev_data)
{
	u8 reg[BQ25882_REG_NUM] = {0};
	char buff[BUF_LEN] = {0};
	int i;
	int ret;

	if (!reg_value)
		return 0;

	memset(reg_value, 0, size);

	for (i = 0; i < BQ25882_REG_NUM; i++) {
		ret = bq25882_read_byte(i, &reg[i]);
		if (ret)
			hwlog_err("dump_register read fail\n");

		snprintf(buff, BUF_LEN, "0x%-7.2x   ", reg[i]);
		strncat(reg_value, buff, strlen(buff));
	}

	return 0;
}

static int bq25882_get_register_head(char *reg_head, int size, void *dev_data)
{
	char buff[BUF_LEN] = {0};
	int i;

	if (!reg_head)
		return 0;

	memset(reg_head, 0, size);

	for (i = 0; i < BQ25882_REG_NUM; i++) {
		snprintf(buff, BUF_LEN, "Reg[0x%-2.2x]   ", i);
		strncat(reg_head, buff, strlen(buff));
	}

	return 0;
}

static int bq25882_set_batfet_disable(int disable)
{
	return 0;
}

static int bq25882_set_watchdog_timer(int value)
{
	u8 val;
	u8 dog_time = value;

	if (dog_time >= WATCHDOG_TIMER_160_S)
		val = BQ25882_REG_CC1_WATCHDOG_160;
	else if (dog_time >= WATCHDOG_TIMER_80_S)
		val = BQ25882_REG_CC1_WATCHDOG_80;
	else if (dog_time >= WATCHDOG_TIMER_40_S)
		val = BQ25882_REG_CC1_WATCHDOG_40;
	else
		val = BQ25882_REG_CC1_WATCHDOG_DIS;

	hwlog_info("set_watchdog_timer [%x]=0x%x\n", BQ25882_REG_CC1, val);

	return bq25882_write_mask(BQ25882_REG_CC1,
		BQ25882_REG_CC1_WATCHDOG_MASK,
		BQ25882_REG_CC1_WATCHDOG_SHIFT,
		val);
}

static int bq25882_set_charger_hiz(int enable)
{
	int ret;

	if (enable > 0)
		ret = bq25882_write_mask(BQ25882_REG_CCL,
			BQ25882_REG_CCL_EN_HIZ_MASK,
			BQ25882_REG_CCL_EN_HIZ_SHIFT,
			TRUE);
	else
		ret = bq25882_write_mask(BQ25882_REG_CCL,
			BQ25882_REG_CCL_EN_HIZ_MASK,
			BQ25882_REG_CCL_EN_HIZ_SHIFT,
			FALSE);

	return ret;
}

static int bq25882_force_set_term_enable(int enable)
{
	return 0;
}

static int bq25882_get_terminal_voltage(void)
{
	u8 vterm_reg = 0;
	int vterm;
	int ret;

	ret = bq25882_read_byte(BQ25882_REG_BVRL, &vterm_reg);
	if (ret)
		return -1;

	vterm = (int)vterm_reg * VCHARGE_STEP + VCHARGE_MIN;
	return vterm;
}

static void bq25882_get_adc_value(struct bq25882_device_info *di)
{
	int ret = 0;
	u8 reg = 0;
	int i;
	int mode;

	mutex_lock(&di->adc_lock);
	/* adc en, one shot */
	if (di->adc_mode == ADC_ONESHOT)
		ret = bq25882_write_mask(BQ25882_REG_AC,
			BQ25882_REG_AC_ADC_CTRL_MASK,
			BQ25882_REG_AC_ADC_CTRL_SHIFT, 0x3);
	mode = di->adc_mode;
	mutex_unlock(&di->adc_lock);

	if (ret) {
		hwlog_err("otg cfg adc one shot err\n");
		return;
	}

	if (mode == ADC_CONTINUOUS) {
		hwlog_info("contiue mode is set\n");
		di->adc_value.vbus = bq25882_get_adc_vbus();
		di->adc_value.ibus = bq25882_get_adc_ibus();
		di->adc_value.vbat = bq25882_get_adc_vbat();
		return;
	}

	/* The conversion result is ready after tCONV, max (3*8)ms */
	for (i = 0; i < CONV_RETRY; i++) {
		ret = bq25882_read_byte(BQ25882_REG_CS1, &reg);
		if (ret)
			return;

		/* if ADC Conversion finished, ADC_DONE_STAT bit will be 1 */
		if (reg & BQ25882_REG_CS1_ADC_DONE_STAT_MASK)
			break;

		usleep_range(3000, 3100); /* sleep 3ms */
	}

	hwlog_info("one-shot covn start is set\n");
	di->adc_value.vbus = bq25882_get_adc_vbus();
	di->adc_value.ibus = bq25882_get_adc_ibus();
	di->adc_value.vbat = bq25882_get_adc_vbat();
}

static void get_adc_work(struct work_struct *work)
{
	struct bq25882_device_info *di = NULL;

	di = container_of(work, struct bq25882_device_info, adc_work.work);
	bq25882_get_adc_value(di);
	schedule_delayed_work(&di->adc_work, msecs_to_jiffies(GET_ADC_TIME));
}

static struct charge_device_ops bq25882_ops = {
	.chip_init = bq25882_chip_init,
	.dev_check = bq25882_device_check,
	.set_input_current = bq25882_set_input_current,
	.set_charge_current = bq25882_set_charge_current,
	.set_terminal_voltage = bq25882_set_terminal_voltage,
	.set_dpm_voltage = bq25882_set_dpm_voltage,
	.set_terminal_current = bq25882_set_terminal_current,
	.set_charge_enable = bq25882_set_charge_enable,
	.set_otg_enable = bq25882_set_otg_enable,
	.set_term_enable = bq25882_set_term_enable,
	.get_charge_state = bq25882_get_charge_state,
	.reset_watchdog_timer = bq25882_reset_watchdog_timer,
	.set_watchdog_timer = bq25882_set_watchdog_timer,
	.set_batfet_disable = bq25882_set_batfet_disable,
	.get_ibus = bq25882_get_ibus_ma,
	.get_vbus = bq25882_get_vbus_mv,
	.set_covn_start = bq25882_set_covn_start,
	.check_input_dpm_state = bq25882_check_input_dpm_state,
	.set_otg_current = bq25882_set_otg_current,
	.set_force_term_enable = bq25882_force_set_term_enable,
	.set_charger_hiz = bq25882_set_charger_hiz,
	.get_iin_set = bq25882_get_input_current_set,
	.get_terminal_voltage = bq25882_get_terminal_voltage,
	.soft_vbatt_ovp_protect = NULL,
	.rboost_buck_limit = NULL,
	.get_charge_current = NULL,
	.turn_on_ico = NULL,
};

static struct power_log_ops bq25882_log_ops = {
	.dev_name = "bq25882",
	.dump_log_head = bq25882_get_register_head,
	.dump_log_content = bq25882_dump_register,
};

static struct charger_otg_device_ops bq25882_otg_ops = {
	.dev_name = "bq25882",
	.otg_set_charger_enable = bq25882_set_charge_enable,
	.otg_set_enable = bq25882_set_otg_enable,
	.otg_set_current = bq25882_set_otg_current,
	.otg_set_watchdog_timer = bq25882_set_watchdog_timer,
	.otg_reset_watchdog_timer = bq25882_reset_watchdog_timer,
};

static struct hw_batt_vol_ops bq25882_batt_vol_ops = {
	.get_batt_vol = bq25882_get_vbat_mv,
};

static void bq25882_irq_work(struct work_struct *work)
{
	struct bq25882_device_info *di = NULL;
	u8 reg = 0;
	u8 reg1 = 0;
	int ret;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct bq25882_device_info, irq_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	msleep(100); /* sleep 100ms */

	ret = bq25882_read_byte(BQ25882_REG_CS1, &reg1);
	ret |= bq25882_read_byte(BQ25882_REG_FF, &reg);
	if (ret)
		hwlog_err("irq_work read fail\n");

	hwlog_info("reg[%x]:0x%x, reg[%x]:0x%0x\n",
		BQ25882_REG_CS1, reg1, BQ25882_REG_FF, reg);

	if (reg & BQ25882_REG_FF_OTG_FLAG_MASK) {
		hwlog_info("boost ocp happened\n");
		power_event_anc_notify(POWER_ANT_CHARGE_FAULT,
			POWER_NE_CHG_FAULT_BOOST_OCP, NULL);
	}

	if (di->irq_active == 0) {
		di->irq_active = 1;
		enable_irq(di->irq_int);
	}
}

static irqreturn_t bq25882_interrupt(int irq, void *_di)
{
	struct bq25882_device_info *di = _di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	hwlog_info("bq25882 int happened\n");

	if (di->irq_active == 1) {
		di->irq_active = 0;
		disable_irq_nosync(di->irq_int);
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("the irq is not enable, do nothing\n");
	}

	return IRQ_HANDLED;
}

static int bq25882_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct bq25882_device_info *di = NULL;
	struct device_node *np = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_bq25882_dev = di;

	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	INIT_WORK(&di->irq_work, bq25882_irq_work);
	INIT_DELAYED_WORK(&di->adc_work, get_adc_work);
	wakeup_source_init(&di->wlock, "bq25882_wakelock");

	/* 0x04: disable TS ADC, check whether bq25882 exists */
	ret = bq25882_write_byte(BQ25882_REG_AFD, 0x04);
	if (ret) {
		hwlog_err("ts adc disable fail\n");
		goto bq25882_fail_0;
	}

	/* set gpio to control CD pin to disable/enable bq25882 IC */
	ret = power_gpio_config_output(np,
		"gpio_cd", "charger_cd", &di->gpio_cd, 0);
	if (ret)
		goto bq25882_fail_0;

	ret = power_gpio_config_interrupt(np,
		"gpio_int", "charger_int", &di->gpio_int, &di->irq_int);
	if (ret)
		goto bq25882_fail_1;

	ret = request_irq(di->irq_int, bq25882_interrupt,
		IRQF_TRIGGER_FALLING, "charger_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		goto bq25882_fail_2;
	}

	disable_irq(di->irq_int);
	di->irq_active = 0;

	/* one shot 12bit adc */
	ret = bq25882_write_byte(BQ25882_REG_AC, 0x70);
	if (ret)
		hwlog_err("ts adc disable fail\n");

	di->adc_mode = ADC_ONESHOT;
	mutex_init(&di->adc_lock);

	ret = hw_battery_voltage_ops_register(&bq25882_batt_vol_ops,
		"bq25882");
	if (ret)
		hwlog_err("bq25882 hw_battery_voltage ops register failed\n");

	ret = series_batt_ops_register(&bq25882_ops);
	if (ret) {
		hwlog_err("bq25882 series_batt ops register fail\n");
		goto bq25882_fail_3;
	}

	ret = charger_otg_ops_register(&bq25882_otg_ops);
	if (ret) {
		hwlog_err("bq25882 charger_otg ops register fail\n");
		goto bq25882_fail_3;
	}

	bq25882_get_adc_value(di);
	schedule_delayed_work(&di->adc_work, msecs_to_jiffies(GET_ADC_TIME));

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = 0;
		power_dev_info->ver_id = 0;
	}

	bq25882_log_ops.dev_data = (void *)di;
	power_log_ops_register(&bq25882_log_ops);
	return 0;

bq25882_fail_3:
	free_irq(di->irq_int, di);
bq25882_fail_2:
	gpio_free(di->gpio_int);
bq25882_fail_1:
	gpio_free(di->gpio_cd);
bq25882_fail_0:
	wakeup_source_trash(&di->wlock);
	kfree(di);
	g_bq25882_dev = NULL;

	return ret;
}

static int bq25882_remove(struct i2c_client *client)
{
	struct bq25882_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	wakeup_source_trash(&di->wlock);
	gpio_set_value(di->gpio_cd, 1);

	if (di->gpio_cd)
		gpio_free(di->gpio_cd);

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	kfree(di);
	g_bq25882_dev = NULL;

	return 0;
}

static void bq25882_shutdown(struct i2c_client *client)
{
	g_shutdown_flag = true;
	/* 0x30: default mode */
	(void)bq25882_write_byte(BQ25882_REG_AC, 0x30);
}

#ifdef CONFIG_PM
static int bq25882_resume(struct device *dev)
{
	struct bq25882_device_info *di = g_bq25882_dev;

	hwlog_info("resume\n");
	__pm_wakeup_event(&di->wlock, msecs_to_jiffies(TIMEOUT_SEC));
	schedule_delayed_work(&di->adc_work, msecs_to_jiffies(0));
	return 0;
}

static int bq25882_suspend(struct device *dev)
{
	hwlog_info("suspend\n");
	return 0;
}

static const struct dev_pm_ops bq25882_pm_ops = {
	.suspend   = bq25882_suspend,
	.resume    = bq25882_resume,
};

#define BQ25882_PM_OPS (&bq25882_pm_ops)
#else
#define BQ25882_PM_OPS (NULL)
#endif /* CONFIG_PM */

MODULE_DEVICE_TABLE(i2c, bq25882);
static const struct of_device_id bq25882_of_match[] = {
	{
		.compatible = "huawei,bq25882_charger",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id bq25882_i2c_id[] = {
	{ "bq25882_charger", 0 }, {}
};

static struct i2c_driver bq25882_driver = {
	.probe = bq25882_probe,
	.remove = bq25882_remove,
	.shutdown = bq25882_shutdown,
	.id_table = bq25882_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "bq25882_charger",
		.of_match_table = of_match_ptr(bq25882_of_match),
		.pm = BQ25882_PM_OPS,
	},
};

static int __init bq25882_init(void)
{
	return i2c_add_driver(&bq25882_driver);
}

static void __exit bq25882_exit(void)
{
	i2c_del_driver(&bq25882_driver);
}

module_init(bq25882_init);
module_exit(bq25882_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("bq25882 charger module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
