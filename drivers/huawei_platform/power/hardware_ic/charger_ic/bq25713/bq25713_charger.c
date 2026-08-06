/*
 * bq25713_charger.c
 *
 * bq25713 driver
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

#include "bq25713_charger.h"
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <huawei_platform/power/battery_voltage.h>

#define HWLOG_TAG bq25713_charger
HWLOG_REGIST();

static struct bq25713_device_info *g_bq25713_dev;

static struct bq25713_device_info *bq25713_get_di(void)
{
	if (!g_bq25713_dev) {
		hwlog_err("chip not init\n");
		return NULL;
	}

	return g_bq25713_dev;
}

static int bq25713_write_byte(u8 reg, u8 value)
{
	struct bq25713_device_info *di = bq25713_get_di();

	if (!di)
		return -ENODEV;

	return power_i2c_u8_write_byte(di->client, reg, value);
}

static int bq25713_write_word(u8 reg, u16 value)
{
	struct bq25713_device_info *di = bq25713_get_di();

	if (!di)
		return -ENODEV;

	return power_i2c_u8_write_word(di->client, reg, value);
}

static int bq25713_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	struct bq25713_device_info *di = bq25713_get_di();

	if (!di)
		return -ENODEV;

	return power_i2c_u8_write_byte_mask(di->client,
		reg, value, mask, shift);
}

static int bq25713_read_byte(u8 reg, u8 *value)
{
	struct bq25713_device_info *di = bq25713_get_di();

	if (!di)
		return -ENODEV;

	return power_i2c_u8_read_byte(di->client, reg, value);
}

static int bq25713_read_word(u8 reg, u16 *value)
{
	struct bq25713_device_info *di = bq25713_get_di();

	if (!di)
		return -ENODEV;

	return power_i2c_u8_read_word(di->client, reg, value, false);
}

static int bq25713_5v_chip_init(void)
{
	if (bq25713_write_mask(BQ25713_REG_CC0_LOW,
		BQ25713_REG_CC0_EN_CHG_MASK,
		BQ25713_REG_CC0_EN_CHG_SHIFT,
		true))
		return -1;

	/* fast charge current limit 1024mA */
	if (bq25713_write_word(BQ25713_REG_CCL, 0x400))
		return -1;

	/* minimum system voltage 10240mV, step 50mV */
	if (bq25713_write_byte(BQ25713_REG_MIN_VSYS, 0x28))
		return -1;

	/* input dpm voltage 4675mV */
	if (bq25713_write_word(BQ25713_REG_IVL, 0x5C3))
		return -1;

	return 0;
}

static int bq25713_9v_chip_init(void)
{
	if (bq25713_write_mask(BQ25713_REG_CC0_LOW,
		BQ25713_REG_CC0_EN_CHG_MASK,
		BQ25713_REG_CC0_EN_CHG_SHIFT,
		true))
		return -1;

	/* fast charge current limit 1024mA */
	if (bq25713_write_word(BQ25713_REG_CCL, 0x400))
		return -1;

	/* minimum system voltage 10240mV, step 50mV */
	if (bq25713_write_byte(BQ25713_REG_MIN_VSYS, 0x28))
		return -1;

	/* input dpm voltage 7600mV */
	if (bq25713_write_word(BQ25713_REG_IVL, 0x1130))
		return -1;

	return 0;
}

static int bq25713_chip_init(struct chip_init_crit *init_crit)
{
	int ret = -1;

	if (!init_crit) {
		hwlog_err("init_crit is null\n");
		return -ENOMEM;
	}

	switch (init_crit->vbus) {
	case ADAPTER_5V:
		ret = bq25713_5v_chip_init();
		break;
	case ADAPTER_9V:
		ret = bq25713_9v_chip_init();
		break;
	default:
		hwlog_err("invalid init_crit vbus mode\n");
		break;
	}

	/* select iin limit */
	(void)bq25713_write_mask(BQ25713_REG_CC2_LOW,
		BQ25713_REG_CC2_EN_EXTILIM_MASK,
		BQ25713_REG_CC2_EN_EXTILIM_SHIFT,
		false);

	return ret;
}

static int bq25713_device_check(void)
{
	u8 reg = 0xFF;

	bq25713_read_byte(BQ25713_REG_MA_ID, &reg);
	if ((reg == BQ25713_MANUF_ID) || (reg == SC8886_MANUF_ID) || (reg == SY6961_MANUF_ID))
		return CHARGE_IC_GOOD;

	hwlog_info("reg=%x, charger ic is not good\n", reg);
	return CHARGE_IC_BAD;
}

static int bq25713_set_input_current(int current_ma)
{
	struct bq25713_device_info *di = bq25713_get_di();

	if (!di)
		return -ENODEV;

	if (current_ma < BQ25713_IINLIM_MIN)
		current_ma = BQ25713_IINLIM_MIN;
	else if (current_ma > BQ25713_IINLIM_MAX)
		current_ma = BQ25713_IINLIM_MAX;

	current_ma = current_ma / BQ25713_IINLIM_STEP;
	di->iin_set = current_ma * BQ25713_IINLIM_STEP;
	hwlog_info("set input current [%x]=0x%-2x\n",
		BQ25713_REG_IIN_HOST, current_ma);

	return bq25713_write_byte(BQ25713_REG_IIN_HOST, (u8)current_ma);
}

static int bq25713_set_charge_current(int current_ma)
{
	if (current_ma < BQ25713_ICHG_MIN)
		current_ma = BQ25713_ICHG_MIN;
	else if (current_ma > BQ25713_ICHG_MAX)
		current_ma = BQ25713_ICHG_MAX;

	hwlog_info("set charge current [%x]=0x%-4x\n",
		BQ25713_REG_CCL, current_ma);

	return bq25713_write_word(BQ25713_REG_CCL, (u16)current_ma);
}

static int bq25713_set_terminal_voltage(int voltage_mv)
{
	struct bq25713_device_info *di = bq25713_get_di();

	if (!di)
		return -ENODEV;

	if (voltage_mv < BQ25713_VCHARGE_MIN)
		voltage_mv = BQ25713_VCHARGE_MIN;
	else if (voltage_mv > BQ25713_VCHARGE_MAX)
		voltage_mv = BQ25713_VCHARGE_MAX;

	di->vterm = voltage_mv;
	hwlog_info("set terminal voltage [%x]=0x%-4x\n",
		BQ25713_REG_TERM_VOL, voltage_mv);

	return bq25713_write_word(BQ25713_REG_TERM_VOL, (u16)voltage_mv);
}

static int bq25713_set_dpm_voltage(int voltage_mv)
{
	if (voltage_mv > BQ25713_VINDPM_MAX)
		voltage_mv = BQ25713_VINDPM_MAX;
	else if (voltage_mv < BQ25713_VINDPM_MIN)
		voltage_mv = BQ25713_VINDPM_MIN;

	voltage_mv = voltage_mv - BQ25713_VINDPM_OFFSET;
	hwlog_info("set dpm voltage [%x]=0x%-4x\n",
		BQ25713_REG_IVL, voltage_mv);

	return bq25713_write_word(BQ25713_REG_IVL, (u16)voltage_mv);
}

static int bq25713_set_terminal_current(int current_ma)
{
	struct bq25713_device_info *di = bq25713_get_di();

	if (!di)
		return -ENODEV;

	if (current_ma < BQ25713_ITERM_MIN)
		current_ma = BQ25713_ITERM_MIN;
	else if (current_ma > BQ25713_ITERM_MAX)
		current_ma = BQ25713_ITERM_MAX;

	di->term_curr = current_ma;
	hwlog_info("set terminal current=%d\n", current_ma);
	return 0;
}

static void bq25713_charge_term_check(struct bq25713_device_info *di)
{
	int curr, curr_avg, vol_avg;
	int vterm, vterm_real;
	int i;
	int vol_sum = 0;

	if (!di)
		return;

	vterm = di->vterm;
	vterm_real = vterm - BQ25713_VTERM_ACC;
	curr_avg = coul_drv_battery_current_avg();
	curr = -coul_drv_battery_current();

	for (i = 0; i < BQ25713_READ_TIMES; i++)
		vol_sum += hw_battery_voltage(BAT_ID_MAX);

	vol_avg = vol_sum / BQ25713_READ_TIMES;
	hwlog_info("vterm=%d bat_max=%d curr=%d curr_avg=%d iterm=%d\n",
		vterm, vol_avg, curr, curr_avg, di->term_curr);

	/* if terminal currrent changes, continue charging */
	if (di->term_curr != di->iterm_pre) {
		hwlog_info("terminal current change\n");
		di->chg_done = false;
		return;
	}

	if ((vol_avg > vterm_real) &&
		(curr_avg < di->term_curr) &&
		(curr < di->term_curr)) {
		di->chg_done_cnt++;
		hwlog_info("chg_done_cnt=%d\n", di->chg_done_cnt);
	} else {
		di->chg_done_cnt = 0;
	}

	/* re-charge */
	if (di->chg_done) {
		if (vol_avg < (vterm - BQ25713_VRECHG_DEFAULT)) {
			di->chg_done = false;
			hwlog_info("re-charge\n");
		}
	}

	if (di->chg_done_cnt >= BQ25713_CHG_DONE_CNT) {
		di->chg_done_cnt = BQ25713_CHG_DONE_CNT;
		di->chg_done = true;
		hwlog_info("charge done\n");
	}
}

static void bq25713_battery_full_charge_check(struct bq25713_device_info *di)
{
	int curr, curr_avg;

	curr_avg = coul_drv_battery_current_avg();
	curr = -coul_drv_battery_current();
	if ((curr < 0) && (curr_avg < 0)) {
		di->chg_done_cnt = 0;
		di->chg_done = false;
	} else if (coul_drv_is_battery_full_charged()) {
		di->chg_done_cnt++;
		hwlog_info("chg_done_cnt=%d\n", di->chg_done_cnt);
	} else {
		di->chg_done_cnt = 0;
		di->chg_done = false;
	}

	if (di->chg_done_cnt >= BQ25713_CHG_DONE_CNT) {
		di->chg_done_cnt = BQ25713_CHG_DONE_CNT;
		di->chg_done = true;
		hwlog_info("charge done\n");
	}
}

static int bq25713_set_charge_enable(int enable)
{
	u8 en_charge = 0;
	struct bq25713_device_info *di = bq25713_get_di();

	if (!di)
		return -ENODEV;

	if (enable) {
		if (coul_drv_is_smart_battery())
			bq25713_battery_full_charge_check(di);
		else
			bq25713_charge_term_check(di);
	} else {
		di->chg_done_cnt = 0;
		di->chg_done = false;
		en_charge = 1;
	}

	if (di->chg_done)
		en_charge = 1;

	if (!di->en_last && enable) {
		hwlog_info("start charging, reset vterm\n");
		bq25713_set_terminal_voltage(di->vterm);
	}

	di->en_last = enable;
	di->iterm_pre = di->term_curr;

	return bq25713_write_mask(BQ25713_REG_CC0_LOW,
		BQ25713_REG_CC0_EN_CHG_MASK,
		BQ25713_REG_CC0_EN_CHG_SHIFT,
		en_charge);
}

static int bq25713_config_otg_voltage(void)
{
	u8 reg = 0;

	bq25713_read_byte(BQ25713_REG_CC3_LOW, &reg);
	if (reg & BQ25713_REG_CC3_OTG_RANGE_LOW_MASK)
		return bq25713_write_word(BQ25713_REG_OTG_VOL,
			BQ25713_RANGE_LOW_OTG_VOL_LIMIT);

	return bq25713_write_word(BQ25713_REG_OTG_VOL,
		BQ25713_RANGE_HIGH_OTG_VOL_LIMIT);
}

static int bq25713_set_otg_enable(int enable)
{
	u8 en_otg = 0;
	struct bq25713_device_info *di = bq25713_get_di();

	if (!di)
		return -ENODEV;

	if (enable) {
		en_otg = 1;
		/* select otg mode */
		bq25713_write_mask(BQ25713_REG_CC3_LOW,
			BQ25713_REG_CC3_OTG_VAP_MODE_MASK,
			BQ25713_REG_CC3_OTG_VAP_MODE_SHIFT,
			en_otg);
		/* ensure otg voltage limit 5300mv */
		bq25713_config_otg_voltage();
	}
	gpio_set_value(di->gpio_otg, en_otg);

	if (!di->irq_active && enable) {
		di->irq_active = 1;
		enable_irq(di->irq_int);
	} else if (di->irq_active && !enable) {
		di->irq_active = 0;
		disable_irq(di->irq_int);
	}

	return bq25713_write_mask(BQ25713_REG_CC3_HIGH,
		BQ25713_REG_CC3_EN_OTG_MASK,
		BQ25713_REG_CC3_EN_OTG_SHIFT,
		en_otg);
}

static int bq25713_set_term_enable(int enable)
{
	return 0;
}

static bool bq25713_is_batt_ovp(void)
{
	u8 reg = 0;
	int vbat_max;

	vbat_max = hw_battery_voltage(BAT_ID_MAX);
	if (vbat_max > BQ25713_VBAT_OVP_TH)
		return true;

	bq25713_read_byte(BQ25713_REG_PHS, &reg);
	if (reg & BQ25713_REG_PHS_BAT_REMOVE_MASK)
		return true;

	return false;
}

static int bq25713_get_charge_state(unsigned int *state)
{
	u16 reg;
	int ret;
	int vbat_max;
	struct bq25713_device_info *di = bq25713_get_di();

	if (!di || !state)
		return -ENODEV;

	ret = bq25713_read_word(BQ25713_REG_CS0, &reg);
	if (ret)
		return ret;

	hwlog_info("get charge state [%x]=0x%x\n", BQ25713_REG_CS0, reg);
	if (!(reg & BQ25713_REG_CS0_NOT_PG_MASK))
		*state |= CHAGRE_STATE_NOT_PG;

	vbat_max = hw_battery_voltage(BAT_ID_MAX);
	if ((reg & BQ25713_REG_CS0_IN_FCHRG_MASK) && (vbat_max > BQ25713_CC_VOLTAGE))
		*state |= CHAGRE_STATE_CV_MODE;

	if (reg & BQ25713_REG_CS0_ACOV_MASK)
		*state |= CHAGRE_STATE_VBUS_OVP;

	if (bq25713_is_batt_ovp())
		*state |= CHAGRE_STATE_BATT_OVP;

	if (di->chg_done)
		*state |= CHAGRE_STATE_CHRG_DONE;

	return ret;
}

static int bq25713_reset_watchdog_timer(void)
{
	u16 reg_vterm;

	if (bq25713_read_word(BQ25713_REG_TERM_VOL, &reg_vterm))
		return -1;
	if (bq25713_write_word(BQ25713_REG_TERM_VOL, reg_vterm))
		return -1;

	return 0;
}

static int bq25713_set_watchdog_timer(int value)
{
	u8 reg_wdt;

	if (value >= WATCHDOG_TIMER_160_S)
		reg_wdt = BQ25713_WATCHDOG_TIMER_LONG;
	else if (value >= WATCHDOG_TIMER_40_S)
		reg_wdt = BQ25713_WATCHDOG_TIMER_SHORT;
	else
		reg_wdt = BQ25713_WATCHDOG_SHUTDOWN;

	hwlog_info("set watchdog timer [%x]=0x%x\n",
		BQ25713_REG_CC0_HIGH, reg_wdt);

	return bq25713_write_mask(BQ25713_REG_CC0_HIGH,
		BQ25713_REG_CC0_WATCHDOG_MASK,
		BQ25713_REG_CC0_WATCHDOG_SHIFT,
		reg_wdt);
}

static int bq25713_set_batfet_disable(int disable)
{
	return 0;
}

static int bq25713_get_ibus_ma(void)
{
	struct bq25713_device_info *di = bq25713_get_di();

	if (!di)
		return -ENODEV;

	return di->adc_value.ibus;
}

static int bq25713_get_vbus_mv(unsigned int *vbus_mv)
{
	struct bq25713_device_info *di = bq25713_get_di();

	if (!di || !vbus_mv)
		return -ENODEV;

	*vbus_mv = di->adc_value.vbus;
	return 0;
}

static int bq25713_get_vbat_mv(void)
{
	struct bq25713_device_info *di = bq25713_get_di();

	if (!di)
		return -ENODEV;

	return di->adc_value.vbat;
}

static int bq25713_set_covn_start(int enable)
{
	return 0;
}

static int bq25713_check_input_dpm_state(void)
{
	u8 reg;

	if (bq25713_read_byte(BQ25713_REG_CS1, &reg))
		return false;

	hwlog_info("check input dpm state [%x]=0x%x\n",
		BQ25713_REG_CS1, reg);
	if ((reg & BQ25713_REG_CS1_IINDPM_STAT_MASK) ||
		(reg & BQ25713_REG_CS1_VINDPM_STAT_MASK))
		return true;

	return false;
}

static int bq25713_set_otg_current(int current_ma)
{
	if (current_ma < BQ25713_OTG_CUR_MIN)
		current_ma = BQ25713_OTG_CUR_MIN;
	else if (current_ma > BQ25713_OTG_CUR_MAX)
		current_ma = BQ25713_OTG_CUR_MAX;

	current_ma = current_ma / BQ25713_OTG_CUR_STEP;
	hwlog_info("set otg current [%x]=0x%x\n",
		BQ25713_REG_OTG_CUR, current_ma);

	return bq25713_write_byte(BQ25713_REG_OTG_CUR, (u8)current_ma);
}

static int bq25713_force_set_term_enable(int enable)
{
	return 0;
}

static int bq25713_set_charger_hiz(int enable)
{
	u8 en_hiz = 0;

	if (enable)
		en_hiz = 1;

	return bq25713_write_mask(BQ25713_REG_CC3_HIGH,
		BQ25713_REG_CC3_EN_HIZ_MASK,
		BQ25713_REG_CC3_EN_HIZ_SHIFT,
		en_hiz);
}

static int bq25713_get_input_current_set(void)
{
	struct bq25713_device_info *di = bq25713_get_di();

	if (!di)
		return -ENODEV;

	return di->iin_set;
}

static int bq25713_get_terminal_voltage(void)
{
	u16 reg_vterm;

	if (bq25713_read_word(BQ25713_REG_TERM_VOL, &reg_vterm))
		return -1;

	return (int)reg_vterm;
}

static int bq25713_dump_register(char *reg_value, int size, void *dev_data)
{
	u8 reg[BQ25713_REG_NUM] = { 0 };
	char buff[BQ25713_BUF_LEN] = { 0 };
	int i;

	if (!reg_value)
		return 0;

	memset(reg_value, 0, size);

	for (i = 0; i < BQ25713_REG_NUM; i++) {
		if (bq25713_read_byte(i, &reg[i]))
			hwlog_err("dump register read fail\n");

		snprintf(buff, BQ25713_BUF_LEN, "0x%-7.2x   ", reg[i]);
		strncat(reg_value, buff, strlen(buff));
	}

	return 0;
}

static int bq25713_get_register_head(char *reg_head, int size, void *dev_data)
{
	char buff[BQ25713_BUF_LEN] = { 0 };
	int i;

	if (!reg_head)
		return 0;

	memset(reg_head, 0, size);

	for (i = 0; i < BQ25713_REG_NUM; i++) {
		snprintf(buff, BQ25713_BUF_LEN, "reg[0x%-2.2x]   ", i);
		strncat(reg_head, buff, strlen(buff));
	}

	return 0;
}

static int bq25713_dc_init(void *data)
{
	/* only inhibit charge */
	return bq25713_write_mask(BQ25713_REG_CC0_LOW,
		BQ25713_REG_CC0_EN_CHG_MASK,
		BQ25713_REG_CC0_EN_CHG_SHIFT,
		true);
}

static int bq25713_dc_dummy_exit(void *data)
{
	return 0;
}

static int bq25713_dc_enable(int enable, void *data)
{
	return bq25713_set_charge_enable(enable);
}

static int bq25713_dc_dummy_discharge(int val, void *data)
{
	return 0;
}

static int bq25713_dc_is_close(void *data)
{
	u16 reg = 0;

	bq25713_read_word(BQ25713_REG_CC0_LOW, &reg);
	if (reg & BQ25713_REG_CC0_EN_CHG_MASK)
		return false;

	return true;
}

static int bq25713_dc_get_id(void *data)
{
	return DC_DEVICE_ID_END;
}

static int bq25713_dc_set_watchdog_timer_ms(int timer_ms, void *data)
{
	return bq25713_set_watchdog_timer(timer_ms / THOUSAND_UNIT);
}

static int bq25713_dc_reset_watchdog_timer(void *data)
{
	return bq25713_reset_watchdog_timer();
}

static int bq25713_batinfo_dummy_init(void *data)
{
	return 0;
}

static int bq25713_batinfo_dummy_exit(void *data)
{
	return 0;
}

static int bq25713_batinfo_get_bat_vol(void *data)
{
	return hw_battery_voltage(BAT_ID_MAX);
}

static int bq25713_batinfo_get_vbus_mv(int *vbus_mv, void *data)
{
	return bq25713_get_vbus_mv((unsigned int *)vbus_mv);
}

static int bq25713_batinfo_get_ibat(int *ibat_ma, void *data)
{
	if (!ibat_ma)
		return -1;

	*ibat_ma = -coul_drv_battery_current();
	return 0;
}

static int bq25713_batinfo_get_ibus_ma(int *ibus_ma, void *data)
{
	int cur_val;

	if (!ibus_ma)
		return -1;

	cur_val = bq25713_get_ibus_ma();
	if (cur_val < 0)
		return cur_val;

	*ibus_ma = cur_val;
	return 0;
}

static int bq25713_batinfo_get_vout(int *val, void *data)
{
	if (!val)
		return -1;

	*val = hw_battery_voltage(BAT_ID_MAX);
	return 0;
}

static int bq25713_stop_charge_config(void)
{
	(void)bq25713_set_input_current(BQ25713_IINLIM_DEFAULT);

	/* enable ilim hiz pin */
	(void)bq25713_write_mask(BQ25713_REG_CC2_LOW,
		BQ25713_REG_CC2_EN_EXTILIM_MASK,
		BQ25713_REG_CC2_EN_EXTILIM_SHIFT,
		true);

	return 0;
}

static struct charge_device_ops bq25713_ops = {
	.chip_init = bq25713_chip_init,
	.dev_check = bq25713_device_check,
	.set_input_current = bq25713_set_input_current,
	.set_charge_current = bq25713_set_charge_current,
	.set_terminal_voltage = bq25713_set_terminal_voltage,
	.set_dpm_voltage = bq25713_set_dpm_voltage,
	.set_terminal_current = bq25713_set_terminal_current,
	.set_charge_enable = bq25713_set_charge_enable,
	.set_otg_enable = bq25713_set_otg_enable,
	.set_term_enable = bq25713_set_term_enable,
	.get_charge_state = bq25713_get_charge_state,
	.reset_watchdog_timer = bq25713_reset_watchdog_timer,
	.set_watchdog_timer = bq25713_set_watchdog_timer,
	.set_batfet_disable = bq25713_set_batfet_disable,
	.get_ibus = bq25713_get_ibus_ma,
	.get_vbus = bq25713_get_vbus_mv,
	.set_covn_start = bq25713_set_covn_start,
	.check_input_dpm_state = bq25713_check_input_dpm_state,
	.set_otg_current = bq25713_set_otg_current,
	.set_force_term_enable = bq25713_force_set_term_enable,
	.set_charger_hiz = bq25713_set_charger_hiz,
	.get_iin_set = bq25713_get_input_current_set,
	.get_terminal_voltage = bq25713_get_terminal_voltage,
	.stop_charge_config = bq25713_stop_charge_config,
	.soft_vbatt_ovp_protect = NULL,
	.rboost_buck_limit = NULL,
	.get_charge_current = NULL,
	.turn_on_ico = NULL,
};

static struct power_log_ops bq25713_log_ops = {
	.dev_name = "bq25713",
	.dump_log_head = bq25713_get_register_head,
	.dump_log_content = bq25713_dump_register,
};

static struct charger_otg_device_ops bq25713_otg_ops = {
	.dev_name = "bq25713",
	.otg_set_charger_enable = bq25713_set_charge_enable,
	.otg_set_enable = bq25713_set_otg_enable,
	.otg_set_current = bq25713_set_otg_current,
	.otg_set_watchdog_timer = bq25713_set_watchdog_timer,
	.otg_reset_watchdog_timer = bq25713_reset_watchdog_timer,
};

static struct dc_ic_ops bq25713_dc_ops = {
	.dev_name = "bq25713",
	.ic_init = bq25713_dc_init,
	.ic_exit = bq25713_dc_dummy_exit,
	.ic_enable = bq25713_dc_enable,
	.ic_discharge = bq25713_dc_dummy_discharge,
	.is_ic_close = bq25713_dc_is_close,
	.get_ic_id = bq25713_dc_get_id,
	.config_ic_watchdog = bq25713_dc_set_watchdog_timer_ms,
	.kick_ic_watchdog = bq25713_dc_reset_watchdog_timer,
};

static struct dc_batinfo_ops bq25713_batinfo_ops = {
	.init = bq25713_batinfo_dummy_init,
	.exit = bq25713_batinfo_dummy_exit,
	.get_bat_btb_voltage = bq25713_batinfo_get_bat_vol,
	.get_bat_package_voltage = bq25713_batinfo_get_bat_vol,
	.get_vbus_voltage = bq25713_batinfo_get_vbus_mv,
	.get_bat_current = bq25713_batinfo_get_ibat,
	.get_ic_ibus = bq25713_batinfo_get_ibus_ma,
	.get_ic_vout = bq25713_batinfo_get_vout,
};

static void bq25713_irq_work(struct work_struct *work)
{
	struct bq25713_device_info *di = NULL;
	u8 reg = 0;

	if (!work)
		return;

	di = container_of(work, struct bq25713_device_info, irq_work);
	if (!di)
		return;

	msleep(100); /* wait 100ms after irq */

	if (bq25713_read_byte(BQ25713_REG_CS0, &reg)) {
		hwlog_err("irq read reg fail\n");
		goto irq_read_fail;
	}

	hwlog_debug("reg[%x]=0x%x\n", BQ25713_REG_CS0, reg);

	if (reg & BQ25713_REG_CS0_OTG_OC_MASK) {
		hwlog_info("charge fault boost ocp happened\n");
		power_event_anc_notify(POWER_ANT_CHARGE_FAULT,
			POWER_NE_CHG_FAULT_BOOST_OCP, NULL);
	}

irq_read_fail:
	if (!di->irq_active) {
		di->irq_active = 1;
		enable_irq(di->irq_int);
	}
}

static irqreturn_t bq25713_interrupt_handler(int irq, void *_di)
{
	struct bq25713_device_info *di = _di;

	if (!di)
		return IRQ_HANDLED;

	if (di->irq_active) {
		di->irq_active = 0;
		disable_irq_nosync(di->irq_int);
		schedule_work(&di->irq_work);
	}

	return IRQ_HANDLED;
}

static int bq25713_irq_init(struct device_node *np,
	struct bq25713_device_info *di)
{
	int ret;

	ret = power_gpio_config_interrupt(np,
		"gpio_int", "charger_int", &di->gpio_int, &di->irq_int);
	if (ret) {
		hwlog_err("config gpio irq fail\n");
		return ret;
	}

	ret = request_irq(di->irq_int, bq25713_interrupt_handler,
		IRQF_TRIGGER_FALLING, "charger_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		gpio_free(di->gpio_int);
		return ret;
	}

	disable_irq(di->irq_int);
	di->irq_active = 0;
	return ret;
}

static int bq25713_get_adc_ibus(void)
{
	u8 reg_adc;
	u16 reg_sta;
	int ibus;

	if (bq25713_read_byte(BQ25713_REG_IBUS_ADC, &reg_adc))
		return -1;

	ibus = (int)reg_adc * BQ25713_IBUS_ADC_STEP_MA;
	/* adc stop working when adapter unplugged */
	if (bq25713_read_word(BQ25713_REG_CS0, &reg_sta))
		return -1;

	if (!(reg_sta & BQ25713_REG_CS0_NOT_PG_MASK))
		ibus = 0;

	hwlog_info("adc ibus is %dmA\n", ibus);
	return ibus;
}

static int bq25713_get_adc_vbus(void)
{
	u8 reg_adc;
	u16 reg_sta;
	int vbus;

	if (bq25713_read_byte(BQ25713_REG_VBUS_ADC, &reg_adc))
		return -1;

	vbus = (int)reg_adc * BQ25713_VBUS_ADC_STEP_MV +
		BQ25713_VBUS_ADC_OFFSET;
	/* adc stop working when adapter unplugged */
	if (bq25713_read_word(BQ25713_REG_CS0, &reg_sta))
		return -1;

	if (!(reg_sta & BQ25713_REG_CS0_NOT_PG_MASK))
		vbus = 0;

	hwlog_info("adc vbus is %dmV\n", vbus);
	return vbus;
}

static int bq25713_get_adc_vbat(void)
{
	u8 reg_adc;
	int vbat;

	if (bq25713_read_byte(BQ25713_REG_VBAT_ADC, &reg_adc))
		return -1;

	vbat = (int)reg_adc * BQ25713_VBAT_ADC_STEP_MV +
		BQ25713_VBAT_ADC_OFFSET;

	hwlog_info("adc vbat is %dmV\n", vbat);
	return vbat;
}

static void bq25713_get_adc_value(struct bq25713_device_info *di)
{
	int i;
	u8 reg = 0;
	int ret = 0;

	if (!di)
		return;

	mutex_lock(&di->adc_lock);
	/* enable adc one shot */
	if (di->adc_mode == BQ25713_ADC_ONESHOT)
		ret = bq25713_write_mask(BQ25713_REG_ADC_HIGH,
			BQ25713_REG_ADC_OPTION_MASK,
			BQ25713_REG_ADC_OPTION_SHIFT,
			BQ25713_ADC_ONESHOT_EN);
	mutex_unlock(&di->adc_lock);

	if (ret) {
		hwlog_err("cfg adc one shot err\n");
		return;
	}

	if (di->adc_mode == BQ25713_ADC_CONTINUOUS) {
		di->adc_value.vbus = bq25713_get_adc_vbus();
		di->adc_value.ibus = bq25713_get_adc_ibus();
		di->adc_value.vbat = bq25713_get_adc_vbat();
		return;
	}

	/* the conversion time: max (3*8)ms */
	for (i = 0; i < BQ25713_CONV_RETRY_TIMES; i++) {
		if (bq25713_read_byte(BQ25713_REG_ADC_HIGH, &reg))
			return;

		if ((reg & BQ25713_REG_ADC_DONE_STAT_MASK) == 0)
			break;
		usleep_range(3000, 3100); /* wait 3ms for next loop */
	}

	if (reg & BQ25713_REG_ADC_DONE_STAT_MASK)
		hwlog_err("one shot conversion timeout\n");

	di->adc_value.vbat = bq25713_get_adc_vbat();
	di->adc_value.ibus = bq25713_get_adc_ibus();
	di->adc_value.vbus = bq25713_get_adc_vbus();
}

static void bq25713_get_adc_work(struct work_struct *work)
{
	struct bq25713_device_info *di = NULL;

	di = container_of(work, struct bq25713_device_info, adc_work.work);
	if (!di)
		return;

	bq25713_get_adc_value(di);
	schedule_delayed_work(&di->adc_work,
		msecs_to_jiffies(BQ25713_GET_ADC_TIME_MS));
}

static void bq25713_parse_dts(struct device_node *np,
	struct bq25713_device_info *di)
{
	/* check whether to support direct charge */
	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_dc", &di->support_dc, 0);

	power_gpio_config_output(di->dev->of_node,
		"gpio_otg", "charger_otg", &di->gpio_otg, 0);
}

#ifdef CONFIG_DIRECT_CHARGER
static void bq25713_scp_ops_register(void)
{
	int i;
	int ret[BQ25713_RET_SIZE] = { 0 };

	ret[0] = dc_ic_ops_register(LVC_MODE, CHARGE_IC_TYPE_MAIN,
		&bq25713_dc_ops);
	ret[1] = dc_ic_ops_register(SC_MODE, CHARGE_IC_TYPE_MAIN,
		&bq25713_dc_ops);
	ret[2] = dc_batinfo_ops_register(LVC_MODE, CHARGE_IC_TYPE_MAIN,
		&bq25713_batinfo_ops);
	ret[3] = dc_batinfo_ops_register(SC_MODE, CHARGE_IC_TYPE_MAIN,
		&bq25713_batinfo_ops);

	for (i = 0; i < BQ25713_RET_SIZE; i++) {
		if (ret[i])
			hwlog_err("register scp adapter ops failed i=%d\n", i);
	}
}
#else
static void bq25713_scp_ops_register(void)
{
}
#endif /* CONFIG_DIRECT_CHARGER */

static int bq25713_ops_register(void)
{
	struct power_devices_info_data *power_dev_info = NULL;
	struct bq25713_device_info *di = bq25713_get_di();

	if (!di)
		return -EINVAL;

	if (charge_ops_register(&bq25713_ops))
		return -1;

	if (di->gpio_otg) {
		if (charger_otg_ops_register(&bq25713_otg_ops))
			return -1;
	}

	if (di->support_dc)
		bq25713_scp_ops_register();

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = 0;
		power_dev_info->ver_id = 0;
	}
	bq25713_log_ops.dev_data = (void *)di;
	power_log_ops_register(&bq25713_log_ops);
	return 0;
}

static void bq25713_basic_setting_init(struct bq25713_device_info *di)
{
	di->chg_done = false;
	di->iin_set = BQ25713_IINLIM_MIN;
	di->term_curr = BQ25713_ITERM_DEFAULT;
	di->iterm_pre = BQ25713_ITERM_DEFAULT;
	di->chg_done_cnt = 0;
	di->en_last = 1;

	/* enable ilim hiz pin */
	(void)bq25713_write_mask(BQ25713_REG_CC2_LOW,
		BQ25713_REG_CC2_EN_EXTILIM_MASK,
		BQ25713_REG_CC2_EN_EXTILIM_SHIFT,
		true);

	/* enable adc: vbus ibus vbat */
	(void)bq25713_write_byte(BQ25713_REG_ADC_LOW,
		BQ25713_ADC_EN_VAL_DEFAULT);

	/* set adc continuous mode */
	(void)bq25713_write_mask(BQ25713_REG_ADC_HIGH,
		BQ25713_REG_ADC_OPTION_MASK,
		BQ25713_REG_ADC_OPTION_SHIFT,
		BQ25713_ADC_CONTINUOUS);
	di->adc_mode = BQ25713_ADC_CONTINUOUS;

	mutex_init(&di->adc_lock);
}

static int bq25713_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct bq25713_device_info *di = NULL;
	struct device_node *np = NULL;

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_bq25713_dev = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	INIT_WORK(&di->irq_work, bq25713_irq_work);
	INIT_DELAYED_WORK(&di->adc_work, bq25713_get_adc_work);
	wakeup_source_init(&di->wlock, "bq25713_wakelock");

	ret = bq25713_irq_init(np, di);
	if (ret)
		goto irq_init_fail;

	bq25713_parse_dts(np, di);
	ret = bq25713_ops_register();
	if (ret)
		goto ops_register_fail;

	bq25713_basic_setting_init(di);
	bq25713_get_adc_value(di);
	schedule_delayed_work(&di->adc_work,
		msecs_to_jiffies(BQ25713_GET_ADC_TIME_MS));

	return 0;

ops_register_fail:
	free_irq(di->irq_int, di);
	gpio_free(di->gpio_int);
	if (di->gpio_otg)
		gpio_free(di->gpio_otg);
irq_init_fail:
	wakeup_source_trash(&di->wlock);
	kfree(di);
	g_bq25713_dev = NULL;
	return ret;
}

static int bq25713_remove(struct i2c_client *client)
{
	struct bq25713_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	wakeup_source_trash(&di->wlock);

	if (di->irq_int)
		free_irq(di->irq_int, di);
	if (di->gpio_int)
		gpio_free(di->gpio_int);
	if (di->gpio_otg)
		gpio_free(di->gpio_otg);

	mutex_destroy(&di->adc_lock);
	kfree(di);
	g_bq25713_dev = NULL;
	return 0;
}

static void bq25713_shutdown(struct i2c_client *client)
{
	/* disable adc */
	bq25713_write_byte(BQ25713_REG_ADC_HIGH,
		BQ25713_ADC_MODE_DEFAULT);
}

#ifdef CONFIG_PM
static int bq25713_resume(struct device *dev)
{
	struct bq25713_device_info *di = bq25713_get_di();

	if (!di)
		return 0;

	__pm_wakeup_event(&di->wlock, msecs_to_jiffies(BQ25713_TIMEOUT_MS));
	schedule_delayed_work(&di->adc_work, msecs_to_jiffies(0));
	return 0;
}

static int bq25713_suspend(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops bq25713_pm_ops = {
	.suspend = bq25713_suspend,
	.resume = bq25713_resume,
};

#define BQ25713_PM_OPS (&bq25713_pm_ops)
#else
#define BQ25713_PM_OPS (NULL)
#endif /* CONFIG_PM */

MODULE_DEVICE_TABLE(i2c, bq25713);
static const struct of_device_id bq25713_of_match[] = {
	{
		.compatible = "huawei,bq25713_charger",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id bq25713_i2c_id[] = {
	{ "bq25713_charger", 0 },
	{},
};

static struct i2c_driver bq25713_driver = {
	.probe = bq25713_probe,
	.remove = bq25713_remove,
	.shutdown = bq25713_shutdown,
	.id_table = bq25713_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "bq25713_charger",
		.of_match_table = of_match_ptr(bq25713_of_match),
		.pm = BQ25713_PM_OPS,
	},
};

static int __init bq25713_init(void)
{
	return i2c_add_driver(&bq25713_driver);
}

static void __exit bq25713_exit(void)
{
	i2c_del_driver(&bq25713_driver);
}

module_init(bq25713_init);
module_exit(bq25713_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("bq25713 charger module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
