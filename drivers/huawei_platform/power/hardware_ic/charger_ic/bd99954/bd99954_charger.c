/*
 * bd99954_charger.c
 *
 * bd99954 driver
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
#include <linux/bitops.h>
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
#include <huawei_platform/power/series_batt_charger.h>
#include <huawei_platform/power/battery_voltage.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <huawei_platform/power/battery_balance.h>
#include "bd99954_charger.h"

#define HWLOG_TAG bd99954_charger
HWLOG_REGIST();

#define oper(x) ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))
#define BUF_LEN                26
#define GPIO_LOW_LEVEL         0
#define ITERM_DEFAULT          128
#define VTERM_ACC              36
#define READ_NUM               5
#define CHG_DONE_CNT           2
#define VBAT_DIV_THR           1100
#define VSYSREG_VALUE          0
#define BD99954_THOUSAND       1000
#define VBUS_REVERSE_CHECK     500 /* 500ms */
#define VBUS_REVERSE_THR       1000
#define VSYSREG_DEFAULT        8960
#define CC_VOL                 4200
#define ILIMIT_DEC_STEP        96
#define VOL_20MV               20
#define VOL_10MV               10
#define BD99954_DOUBLE         2
#define WK_TIMEOUT_SEC         60000
#define VTERM_NOT_CHG          7500
#define EN_CHG                 1
#define SUSPEND_DCDC           1
#define ACTIVE_DCDC            0
#define DELAY_TIME             100

struct bd99954_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct delayed_work vbus_reverse_wk;
	struct wakeup_source wakelock;
	struct notifier_block bal_nb;
	enum bal_event_type bal_event;
	int irq_active;
	int irq_int;
	int gpio_int;
	int gpio_otg;
	int term_curr;
	int iterm_pre;
	int vbat_div_ratio;
	int vterm;
	int iindpm;
	int en_last;
	bool dpm_half_occur;
	bool chg_done;
};

static struct bd99954_device_info *g_bd99954_dev;
static int g_chg_done_cnt;

static int bd99954_get_vbat_mv(void);
static int bd99954_get_terminal_voltage(void);
static int bd99954_check_input_dpm_state(void);

static void bd99954_wake_lock(void)
{
	if (!g_bd99954_dev)
		return;

	if (!g_bd99954_dev->wakelock.active) {
		__pm_wakeup_event(&g_bd99954_dev->wakelock,
			msecs_to_jiffies(WK_TIMEOUT_SEC));
		hwlog_info("wake lock\n");
	}
}

static void bd99954_wake_unlock(void)
{
	if (!g_bd99954_dev)
		return;

	if (g_bd99954_dev->wakelock.active) {
		__pm_relax(&g_bd99954_dev->wakelock);
		hwlog_info("wake unlock\n");
	}
}

static int bd99954_read_word(u8 reg, u16 *value)
{
	struct bd99954_device_info *di = g_bd99954_dev;
	int ret;
	u16 reg_value;

	if (!di || !di->client || !value)
		return -EINVAL;

	ret = i2c_smbus_read_word_swapped(di->client, reg);
	if (ret < 0) {
		hwlog_err("read reg:0x%x fail\n", reg);
		return ret;
	}

	reg_value = (u16)ret;
	*value = oper(reg_value);
	return ret;
}

static int bd99954_write_word(u8 reg, u16 value)
{
	struct bd99954_device_info *di = g_bd99954_dev;
	int ret;

	if (!di || !di->client)
		return -EINVAL;

	value = oper(value);
	ret = i2c_smbus_write_word_swapped(di->client, reg, value);
	if (ret < 0)
		hwlog_err("write reg:0x%x fail\n", reg);

	return ret;
}

static int bd99954_write_mask(u8 reg, u16 mask, u16 shift, u16 value)
{
	struct bd99954_device_info *di = g_bd99954_dev;
	int ret;
	u16 reg_value;

	if (!di || !di->client)
		return -EINVAL;

	ret = i2c_smbus_read_word_swapped(di->client, reg);
	if (ret < 0) {
		hwlog_err("read reg:0x%x fail\n", reg);
		return ret;
	}

	reg_value = (u16)ret;
	reg_value = oper(reg_value);
	reg_value = (reg_value & (~mask)) | (value << shift);
	value = oper(reg_value);
	ret = i2c_smbus_write_word_swapped(di->client, reg, value);
	if (ret < 0)
		hwlog_err("write reg:0x%x fail\n", reg);

	return ret;
}

static int init_charge_reg(void)
{
	int ret;

	/* Extended Commands, PROTECT_SET:16'h0000 MAP_SET:16'h0001 */
	ret = bd99954_write_word(BD99954_REG_PROTECT, 0);
	ret |= bd99954_write_word(BD99954_REG_MAP_SET_CHARGE, 0x01);

	ret |= bd99954_write_word(BD99954_REG_SMBREG, BD99954_REG_SMBREG_0H);
	/* enable int: vbus_rbuv_det,vbus_ov_det */
	ret |= bd99954_write_word(BD99954_REG_INT1_SET, 0x8020);
	/* enable int: tmp_out_det,ibat_short_det,vbat_ov_det */
	ret |= bd99954_write_word(BD99954_REG_INT3_SET, 0x0850);
	/* enable int: vsys_ov,vsys_sht_curr,vsys_uv */
	ret |= bd99954_write_word(BD99954_REG_INT4_SET, 0x25);

	ret |= bd99954_write_word(BD99954_REG_0X20, BD99954_REG_VALUE_0X00);
	ret |= bd99954_write_word(BD99954_REG_0X21, BD99954_REG_VALUE_0X00);
	ret |= bd99954_write_word(BD99954_REG_0X22, BD99954_REG_VALUE_0X00);
	ret |= bd99954_write_word(BD99954_REG_0X23, BD99954_REG_VALUE_0X00);
	ret |= bd99954_write_word(BD99954_REG_0X24, BD99954_REG_VALUE_0X00);
	ret |= bd99954_write_word(BD99954_REG_0X25, BD99954_REG_VALUE_0X00);
	ret |= bd99954_write_word(BD99954_REG_0X28, BD99954_REG_VALUE_0X00);
	ret |= bd99954_write_word(BD99954_REG_0X30, BD99954_REG_VALUE_0X00);
	return ret;
}

static bool bd99954_is_vbus_online(void)
{
	u16 reg = 0;
	int ret;

	ret = bd99954_read_word(BD99954_REG_VBUS_VCC_STATUS, &reg);
	if (ret < 0 || !(reg & BD99954_VBUS_ONLINE_MASK))
		return false;

	return true;
}

static bool bd99954_is_vbat_div_open(void)
{
	int vbat0;
	int vbat1;
	int vbat_all;
	int ret;
	int num = 2; /* two times */
	u16 vbat = 0;
	bool div_flag = false;

	if (g_bd99954_dev->vbat_div_ratio == BD99954_THOUSAND ||
		!bd99954_is_vbus_online())
		return false;

	do {
		vbat0 = hw_battery_voltage(BAT_ID_0);
		vbat1 = hw_battery_voltage(BAT_ID_1);
		ret = bd99954_read_word(BD99954_REG_VBAT_AVE_VAL, &vbat);
		if (ret < 0)
			return false;

		vbat_all = (int)vbat * BD99954_REG_VBAT_VAL_STEP;
		hwlog_info("vbat0:%d, vbat1:%d, batt_all:%d\n",
			vbat0, vbat1, vbat_all);
		div_flag = (vbat0 + vbat1) * BD99954_THOUSAND >=
			vbat_all * VBAT_DIV_THR;
		num--;
	} while (!div_flag && num > 0);

	return div_flag;
}

static void bd99954_set_vsysreg(u16 value)
{
	u16 reg = (value / BD99954_VSYSREG_STEP) << BD99954_VSYSREG_SHIFT;

	(void)bd99954_write_word(BD99954_REG_VSYSREG_SET, reg);
}

static void bd99954_set_usb_suspend(u16 mode)
{
	bd99954_write_mask(BD99954_REG_CHGOP_SET2, BD99954_REG_USB_SUS_MASK,
		BD99954_REG_USB_SUS_SHIFT, mode); /* 1:suspend 0:Active */
}

static int bd99954_5v_chip_init(struct bd99954_device_info *di)
{
	int ret = 0;

	/* Fast Charge Current Limit 960mA */
	ret |= bd99954_write_word(BD99954_REG_ICHG_SET,
		0xF << BD99954_ICHG_SHIFT);
	/* term curr 128ma */
	ret |= bd99954_write_word(BD99954_REG_ITERM_SET,
		0x2 << BD99954_ITERM_SHIFT);
	/* fast charge watchdog 17h, pre-charge timer 2h */
	ret |= bd99954_write_word(BD99954_REG_CHGWDT_SET, 0xFF78);
	/* vbat ovp 9488mv */
	ret |= bd99954_write_word(BD99954_REG_VBATOVP_SET, 0x2510);
	/* enabled input dpm */
	ret |= bd99954_write_mask(BD99954_REG_CHGOP_SET1,
		BD99954_DIS_AUTO_LIMIIN_MASK,
		BD99954_DIS_AUTO_LIMIIN_SHIFT,
		BD99954_EN_AUTO_LIMIIN);
	ret |= bd99954_write_word(BD99954_REG_ILIM_DECREASE_SET,
		BD99954_ILIM_DECREASE_VALUE);
	ret |= bd99954_write_mask(BD99954_REG_CHGOP_SET2,
		BD99954_ILIMIT_RESET_STEP_MASK,
		BD99954_ILIMIT_RESET_STEP_SHIFT,
		BD99954_ILIMIT_RESET_STEP_VALUE);
	ret |= bd99954_write_mask(BD99954_REG_VIN_CTRL_SET,
		BD99954_VCC_INPUT_EN_MASK,
		BD99954_VCC_INPUT_EN_SHIFT,
		BD99954_DISABLE_VCC_INPUT);
	/* disable top-off */
	ret |= bd99954_write_mask(BD99954_REG_CHGOP_SET1,
		BD99954_REG_AUTO_TOP_MASK,
		BD99954_REG_AUTO_TOP_SHIFT,
		BD99954_DISABLE_AUTO_TOP);

	bd99954_set_vsysreg(VSYSREG_VALUE *
		BD99954_THOUSAND / (u16)di->vbat_div_ratio);
	g_bd99954_dev->dpm_half_occur = false;
	schedule_delayed_work(&di->vbus_reverse_wk, msecs_to_jiffies(0));
	return ret;
}

static int bd99954_chip_init(struct chip_init_crit *init_crit)
{
	int ret = -1;
	struct bd99954_device_info *di = g_bd99954_dev;

	if (!di || !init_crit) {
		hwlog_err("di or init_crit is null\n");
		return -EINVAL;
	}

	switch (init_crit->vbus) {
	case ADAPTER_5V:
		ret = bd99954_5v_chip_init(di);
		break;
	default:
		hwlog_err("invalid init_crit vbus mode\n");
		break;
	}

	return ret;
}

static int bd99954_set_input_current(int value)
{
	int limit_current;
	u16 iin_limit;
	u16 reg_07 = 0;
	u16 reg_05 = 0;
	struct bd99954_device_info *di = g_bd99954_dev;

	limit_current = value;
	if (limit_current < BD99954_IBUS_LIMIT_MIN)
		limit_current = BD99954_IBUS_LIMIT_MIN;
	else if (limit_current > BD99954_IBUS_LIMIT_MAX)
		limit_current = BD99954_IBUS_LIMIT_MAX;

	/*
	 * if the input current limit -1/2, increase the current by
	 * decreasing the value of the 0x07 register.
	 */
	if (bd99954_check_input_dpm_state() != 0) {
		(void)bd99954_read_word(BD99954_REG_IBUS_LIM_SET, &reg_07);
		(void)bd99954_read_word(BD99954_REG_CUR_IBUS_LIM_SET, &reg_05);
		hwlog_info("reg05:%d, reg07:%d\n", reg_05, reg_07);

		if (di->dpm_half_occur) {
			di->iindpm = reg_07 - ILIMIT_DEC_STEP;
		} else if (reg_07 == reg_05 * BD99954_DOUBLE) {
			di->dpm_half_occur = true;
			di->iindpm = reg_07 - ILIMIT_DEC_STEP;
		}
	}

	if (di->dpm_half_occur) {
		limit_current = (di->iindpm >= BD99954_IBUS_LIMIT_MIN) ?
			di->iindpm : BD99954_IBUS_LIMIT_MIN;
		hwlog_info("limit_current is %d\n", limit_current);
	}

	iin_limit = limit_current / BD99954_IBUS_LIMIT_STEP;
	hwlog_info("set_input_current [%x]=0x%x\n",
		BD99954_REG_IBUS_LIM_SET, iin_limit);

	return bd99954_write_word(BD99954_REG_IBUS_LIM_SET,
		iin_limit << BD99954_IBUS_LIMIT_SHIFT);
}

static int bd99954_set_input_voltage(int value)
{
	int vindpm_voltage;
	u16 vindpm;

	vindpm_voltage = value;
	if (vindpm_voltage > BD9995_VBUS_MAX)
		vindpm_voltage = BD9995_VBUS_MAX;
	else if (vindpm_voltage < BD9995_VBUS_MIN)
		vindpm_voltage = BD9995_VBUS_MIN;

	vindpm = vindpm_voltage / BD9995_VBUS_STEP;
	hwlog_info("set_dpm_voltage [%x]=0x%x\n", BD99954_REG_VBUS_SET, vindpm);
	return bd99954_write_word(BD99954_REG_VBUS_SET,
		vindpm << BD9995_VBUS_SHIFT);
}

static int bd99954_check_input_dpm_state(void)
{
	u16 reg_value = 0;
	int ret;

	ret = bd99954_read_word(BD99954_REG_VBUS_VCC_STATUS, &reg_value);
	if (ret < 0)
		return FALSE;

	hwlog_info("check_input_dpm_state [%x]=0x%x\n",
		BD99954_REG_VBUS_VCC_STATUS, reg_value);

	if (reg_value & BD99954_VBUS_CLPS_MASK)
		return TRUE;

	return FALSE;
}

static int bd99954_set_charge_current(int value)
{
	int currentma;
	u16 ichg;

	currentma = value;
	if (currentma < BD99954_ICHG_MIN)
		currentma = BD99954_ICHG_MIN;
	else if (currentma > BD99954_ICHG_MAX)
		currentma = BD99954_ICHG_MAX;

	ichg = currentma / BD99954_ICHG_STEP;
	hwlog_info("set_charge_current [%x]=0x%x\n",
		BD99954_REG_ICHG_SET, ichg);

	return bd99954_write_word(BD99954_REG_ICHG_SET,
		ichg << BD99954_ICHG_SHIFT);
}

static int bd99954_set_terminal_voltage(int value)
{
	int voltagemv;
	u16 voreg, vrechg;
	int ret;
	struct bd99954_device_info *di = g_bd99954_dev;

	if (di->en_last == 0) {
		hwlog_info("stop charging, stop set_terminal_voltage\n");
		return 0;
	}

	voltagemv = value;
	if (voltagemv < BD99954_VCHARGE_MIN)
		voltagemv = BD99954_VCHARGE_MIN;
	else if (voltagemv > BD99954_VCHARGE_MAX)
		voltagemv = BD99954_VCHARGE_MAX;

	di->vterm = voltagemv;

	if (bd99954_is_vbat_div_open())
		voltagemv = voltagemv * BD99954_THOUSAND /
			di->vbat_div_ratio;

	voreg = voltagemv / BD99954_VCHARGE_STEP;
	hwlog_info("set_terminal_voltage [%x]=0x%x\n",
		BD99954_REG_VFASTCHG_REG_SET1, voreg);

	vrechg = (voltagemv - BD99954_VRECHG_DEFAULT) / BD99954_VRECHG_STEP;
	ret = bd99954_write_word(BD99954_REG_VRECHG_SET,
		vrechg << BD99954_VRECHG_SHITF);
	ret |= bd99954_write_word(BD99954_REG_VFASTCHG_REG_SET1,
		voreg << BD99954_VFASTCHG_SHIFT);
	return ret;
}

static int bd99954_set_terminal_current(int value)
{
	int term_currentma;
	u16 iterm_reg;

	term_currentma = value;
	if (term_currentma < BD99954_ITERM_MIN)
		term_currentma = BD99954_ITERM_MIN;
	else if (term_currentma > BD99954_ITERM_MAX)
		term_currentma = BD99954_ITERM_MAX;

	g_bd99954_dev->term_curr = value;
	iterm_reg = term_currentma / BD99954_ITERM_STEP;
	hwlog_info("set_terminal_current [%x]=0x%x\n",
		BD99954_REG_ITERM_SET, iterm_reg);

	return bd99954_write_word(BD99954_REG_ITERM_SET,
		iterm_reg << BD99954_ITERM_SHIFT);
}

/*
 * because the precision of the charge termination current of bd99954 is low,
 * the coulometer is used to charging termination
 */
static void bd99954_charge_term_check(struct bd99954_device_info *di)
{
	int curr, curr_avg;
	int vterm, vterm_real;
	int vol_sum = 0;
	int vol_avg;
	int i;

	vterm = series_batt_get_vterm_single();
	vterm_real = vterm - VTERM_ACC;
	curr_avg = coul_drv_battery_current_avg();
	curr = -coul_drv_battery_current();

	for (i = 0; i < READ_NUM; i++)
		vol_sum += hw_battery_voltage(BAT_ID_MAX);

	vol_avg = vol_sum / READ_NUM;
	hwlog_info("vterm:%d bat_max:%d curr:%d curr_avg:%d iterm:%d\n",
		vterm, vol_avg, curr, curr_avg, di->term_curr);

	/* if term curr changes, continue charging */
	if (di->term_curr != di->iterm_pre) {
		hwlog_info("term curr change\n");
		di->chg_done = FALSE;
		return;
	}

	if ((vol_avg > vterm_real) &&
		(curr_avg < di->term_curr) &&
		(curr < di->term_curr)) {
		g_chg_done_cnt++;
		hwlog_info("g_chg_done_cnt:%d\n", g_chg_done_cnt);
	} else {
		g_chg_done_cnt = 0;
	}

	/* re-charge */
	if (di->chg_done) {
		if (vol_avg < (vterm - BD99954_VRECHG_DEFAULT)) {
			di->chg_done = FALSE;
			hwlog_info("re-charge\n");
		}
	}

	if (g_chg_done_cnt >= CHG_DONE_CNT) {
		g_chg_done_cnt = CHG_DONE_CNT;
		di->chg_done = TRUE;
		hwlog_info("charge done\n");
	}
}

static int bd99954_set_charge_enable(int enable)
{
	struct bd99954_device_info *di = g_bd99954_dev;
	int ret;

	hwlog_info("charge enable:%d\n", enable);

	/*
	 * problem:the battery is not activated after being protected,
	 * solution:when the OTG or charger is removed, set en to 1.
	 */
	if (charge_get_charger_type() == CHARGER_REMOVED) {
		hwlog_info("charger remove,set en_chg=1\n");
		bd99954_set_vsysreg(VSYSREG_DEFAULT);
		return bd99954_write_mask(BD99954_REG_CHGOP_SET2,
			BD99954_REG_CHG_EN_MASK, BD99954_REG_CHG_EN_SHIFT,
			EN_CHG);
	}

	if (enable) {
		bd99954_charge_term_check(di);
	} else {
		g_chg_done_cnt = 0;
		di->chg_done = FALSE;
	}

	if (di->chg_done)
		enable = 0;

	if (enable == 0 && di->bal_event == BAL_START_PULSE) {
		hwlog_info("BAL_START_PULSE\n");
		enable = 1;
	}

	if (di->en_last == 0 && enable > 0) {
		di->en_last = enable;
		hwlog_info("start charging, reset vterm\n");
		bd99954_set_terminal_voltage(di->vterm);
	}

	if (enable) {
		bd99954_set_vsysreg(VSYSREG_VALUE *
			BD99954_THOUSAND / (u16)di->vbat_div_ratio);
		if (bd99954_is_vbus_online())
			bd99954_wake_lock();
	} else {
		bd99954_wake_unlock();
		bd99954_set_vsysreg(VSYSREG_DEFAULT);
	}

	di->en_last = enable;
	di->iterm_pre = di->term_curr;
	ret = bd99954_write_mask(BD99954_REG_CHGOP_SET2,
		BD99954_REG_CHG_EN_MASK, BD99954_REG_CHG_EN_SHIFT,
		enable);

	if (enable == 0 && bd99954_is_vbat_div_open()) {
		hwlog_info("vbus online and stop charging, set vterm high\n");
		(void)bd99954_write_word(BD99954_REG_VFASTCHG_REG_SET1,
			(VTERM_NOT_CHG / BD99954_VCHARGE_STEP) <<
			BD99954_VFASTCHG_SHIFT);
	}

	return ret;
}

static int bd99954_set_otg_enable(int enable)
{
	if (enable) {
		wired_chsw_set_wired_channel(WIRED_CHANNEL_ALL, WIRED_CHANNEL_CUTOFF);
		boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_BOOST_GPIO_OTG);
		gpio_set_value(g_bd99954_dev->gpio_otg, enable);
	} else {
		gpio_set_value(g_bd99954_dev->gpio_otg, enable);
		boost_5v_enable(BOOST_5V_DISABLE, BOOST_CTRL_BOOST_GPIO_OTG);
		wired_chsw_set_wired_channel(WIRED_CHANNEL_MAIN, WIRED_CHANNEL_RESTORE);
	}

	return 0;
}

static int bd99954_get_charge_state(unsigned int *state)
{
	u16 chg_status = 0;
	u16 vbatt_status = 0;
	u16 vbus_status = 0;
	u16 wdt_status = 0;
	int vbat_max;

	if (!state)
		return -EINVAL;

	(void)bd99954_read_word(BD99954_REG_CHGSTM_STATUS, &chg_status);
	(void)bd99954_read_word(BD99954_REG_VBAT_VSYS_STATUS, &vbatt_status);
	(void)bd99954_read_word(BD99954_REG_VBUS_VCC_STATUS, &vbus_status);
	(void)bd99954_read_word(BD99954_REG_WDT_STATUS, &wdt_status);
	vbat_max = hw_battery_voltage(BAT_ID_MAX);

	hwlog_info("0x0:%0x, 0x1:%0x, 0x2:%0x\n",
		chg_status, vbatt_status, vbus_status);

	if (vbatt_status & BD99954_VBAT_OVP_MASK)
		*state |= CHAGRE_STATE_BATT_OVP;
	if (vbus_status & BD99954_VBUS_OVP_MASK)
		*state |= CHAGRE_STATE_VBUS_OVP;
	if (((chg_status & BD99954_CURR_STATE_MASK) ==
		BD99954_FAST_CHARGE_MODE) && (vbat_max > CC_VOL))
		*state |= CHAGRE_STATE_CV_MODE;
	if (g_bd99954_dev->chg_done)
		*state |= CHAGRE_STATE_CHRG_DONE;

	return 0;
}

static int bd99954_get_ibus_ma(void)
{
	u16 ibus = 0;
	int ret;

	ret = bd99954_read_word(BD99954_REG_IBUS_AVE_VAL, &ibus);
	if (ret < 0)
		return -1;
	return (int)ibus * BD99954_IBUS_VAL_STEP;
}

static int bd99954_get_vbus_mv(unsigned int *vbus_mv)
{
	u16 vbus = 0;
	int ret;

	ret = bd99954_read_word(BD99954_REG_VBUS_AVE_VAL, &vbus);
	if (ret < 0)
		return ret;

	*vbus_mv = (unsigned int)vbus * BD99954_VBUS_VAL_STEP;
	return 0;
}

static int bd99954_get_vbat_mv(void)
{
	u16 vbat = 0;
	int ret;

	ret = bd99954_read_word(BD99954_REG_VBAT_AVE_VAL, &vbat);
	if (ret < 0)
		return -1;

	vbat = (unsigned int)vbat * BD99954_REG_VBAT_VAL_STEP;

	if (bd99954_is_vbat_div_open())
		vbat = vbat * g_bd99954_dev->vbat_div_ratio / BD99954_THOUSAND;

	return vbat;
}

static int bd99954_set_otg_current(int value)
{
	return 0;
}

static int bd99954_set_charger_hiz(int enable)
{
	return bd99954_write_mask(BD99954_REG_CHGOP_SET2,
		BD99954_REG_USB_SUS_MASK, BD99954_REG_USB_SUS_SHIFT,
		enable);
}

static int bd99954_get_terminal_voltage(void)
{
	return g_bd99954_dev->vterm;
}

static int bd99954_dump_register(char *reg_value, int size, void *dev_data)
{
	u16 reg[BD99954_REG_TOTAL_NUM] = { 0 };
	char buff[BUF_LEN] = { 0 };
	int i;
	int ret;

	if (!reg_value) {
		hwlog_err("reg_value is null\n");
		return -EINVAL;
	}

	memset(reg_value, 0, size);

	for (i = 0; i < BD99954_REG_TOTAL_NUM; i++) {
		ret = bd99954_read_word(i, &reg[i]);
		if (ret < 0)
			hwlog_err("dump_register read fail\n");

		snprintf(buff, BUF_LEN, "0x%-7.2x   ", reg[i]);
		strncat(reg_value, buff, strlen(buff));
	}

	return 0;
}

static int bd99954_get_register_head(char *reg_head, int size, void *dev_data)
{
	char buff[BUF_LEN] = { 0 };
	int i;

	if (!reg_head)
		return -EINVAL;

	memset(reg_head, 0, size);

	for (i = 0; i < BD99954_REG_TOTAL_NUM; i++) {
		snprintf(buff, BUF_LEN, "Reg[0x%-2.2x]   ", i);
		strncat(reg_head, buff, strlen(buff));
	}

	return 0;
}

static int bd99954_get_input_current_set(void)
{
	u16 iin_reg = 0;
	int iin_set;
	int ret;

	ret = bd99954_read_word(BD99954_REG_CUR_IBUS_LIM_SET, &iin_reg);
	if (ret < 0)
		return -1;

	iin_set = (int)iin_reg * BD99954_CUR_IBUS_LIM_SET_STEP;
	return iin_set;
}

static int bd99954_device_check(void)
{
	return CHARGE_IC_GOOD;
}

static int bd99954_set_covn_start(int enable)
{
	return 0;
}

static int bd99954_set_batfet_disable(int disable)
{
	return 0;
}

static int bd99954_set_watchdog_timer(int value)
{
	return 0;
}

static int bd99954_set_term_enable(int enable)
{
	return 0;
}

static int bd99954_reset_watchdog_timer(void)
{
	return 0;
}

static struct charge_device_ops bd99954_chg_ops = {
	.chip_init = bd99954_chip_init,
	.dev_check = bd99954_device_check,
	.set_input_current = bd99954_set_input_current,
	.set_dpm_voltage = bd99954_set_input_voltage,
	.set_charge_current = bd99954_set_charge_current,
	.set_terminal_voltage = bd99954_set_terminal_voltage,
	.set_terminal_current = bd99954_set_terminal_current,
	.set_charge_enable = bd99954_set_charge_enable,
	.set_otg_enable = bd99954_set_otg_enable,
	.set_term_enable = bd99954_set_term_enable,
	.get_charge_state = bd99954_get_charge_state,
	.reset_watchdog_timer = bd99954_reset_watchdog_timer,
	.set_watchdog_timer = bd99954_set_watchdog_timer,
	.set_batfet_disable = bd99954_set_batfet_disable,
	.get_ibus = bd99954_get_ibus_ma,
	.get_vbus = bd99954_get_vbus_mv,
	.set_covn_start = bd99954_set_covn_start,
	.set_otg_current = bd99954_set_otg_current,
	.set_charger_hiz = bd99954_set_charger_hiz,
	.get_iin_set = bd99954_get_input_current_set,
	.get_terminal_voltage = bd99954_get_terminal_voltage,
	.check_input_dpm_state = bd99954_check_input_dpm_state,
	.soft_vbatt_ovp_protect = NULL,
	.rboost_buck_limit = NULL,
	.get_charge_current = NULL,
	.turn_on_ico = NULL,
	.set_force_term_enable = NULL,
};

static void bd99954_set_chop_all(u16 mode)
{
	hwlog_info("chop all:%d\n", mode);
	(void)bd99954_write_mask(BD99954_REG_CHGOP_SET2,
		BD99954_REG_CHOP_ALL_MASK, BD99954_REG_CHOP_ALL_SHIFT, mode);
}

static int bd99954_bal_notifier(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct bd99954_device_info *di = container_of(nb,
		struct bd99954_device_info, bal_nb);

	hwlog_info("notify bal_event:%ld\n", event);
	di->bal_event = event;
	return NOTIFY_OK;
}

static void bd99954_vbus_reverse_work(struct work_struct *work)
{
	struct bd99954_device_info *di = NULL;
	int ibus;
	u16 vsys_reg = 0;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work,
		struct bd99954_device_info, vbus_reverse_wk.work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	if (charge_get_charger_type() == CHARGER_REMOVED) {
		hwlog_info("charger removed\n");
		bd99954_set_chop_all(0);
		return;
	}

	if (!bd99954_is_vbus_online()) {
		schedule_delayed_work(&di->vbus_reverse_wk,
			msecs_to_jiffies(VBUS_REVERSE_CHECK));
		return;
	}

	ibus = bd99954_get_ibus_ma();
	if (ibus < VBUS_REVERSE_THR)
		bd99954_set_chop_all(1);
	else
		bd99954_set_chop_all(0);

	(void)bd99954_read_word(BD99954_REG_VBAT_VSYS_STATUS, &vsys_reg);
	if (vsys_reg & BD99954_VSYS_OV_MASK) {
		hwlog_info("vsys ov\n");
		bd99954_set_vsysreg(VSYSREG_DEFAULT);
		bd99954_set_usb_suspend(SUSPEND_DCDC);
		bd99954_set_usb_suspend(ACTIVE_DCDC);
		msleep(DELAY_TIME);
		bd99954_set_vsysreg(VSYSREG_VALUE);
	}

	schedule_delayed_work(&di->vbus_reverse_wk,
		msecs_to_jiffies(VBUS_REVERSE_CHECK));
}

static struct power_log_ops bd99954_log_ops = {
	.dev_name = "bd99954",
	.dump_log_head = bd99954_get_register_head,
	.dump_log_content = bd99954_dump_register,
};

static struct charger_otg_device_ops bd99954_otg_ops = {
	.dev_name = "bd99954",
	.otg_set_charger_enable = bd99954_set_charge_enable,
	.otg_set_enable = bd99954_set_otg_enable,
	.otg_set_current = bd99954_set_otg_current,
	.otg_set_watchdog_timer = bd99954_set_watchdog_timer,
	.otg_reset_watchdog_timer = bd99954_reset_watchdog_timer,
};

struct hw_batt_vol_ops bd99954_vbatt_ops = {
	.get_batt_vol = bd99954_get_vbat_mv,
};

static void bd99954_irq_work(struct work_struct *work)
{
	struct bd99954_device_info *di = NULL;
	u16 chg_status = 0;
	u16 vbatt_status = 0;
	u16 vbus_status = 0;
	u16 chg_op_status = 0;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct bd99954_device_info, irq_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	msleep(100); /* sleep 100ms */

	(void)bd99954_read_word(BD99954_REG_CHGSTM_STATUS, &chg_status);
	(void)bd99954_read_word(BD99954_REG_VBAT_VSYS_STATUS, &vbatt_status);
	(void)bd99954_read_word(BD99954_REG_VBUS_VCC_STATUS, &vbus_status);
	(void)bd99954_read_word(BD99954_REG_CHGOP_STATUS, &chg_op_status);
	hwlog_info("status reg 0x01-0x03:%u,%u,%u,%u\n",
		chg_status, vbatt_status, vbus_status, chg_op_status);

	if (vbus_status & BD99954_VBUS_OVP_MASK) {
		hwlog_info("vbus ovp happened\n");
		power_event_anc_notify(POWER_ANT_CHARGE_FAULT,
			POWER_NE_CHG_FAULT_BOOST_OCP, NULL);
	}

	if (di->irq_active == 0) {
		di->irq_active = 1;
		enable_irq(di->irq_int);
	}
}

static irqreturn_t bd99954_interrupt(int irq, void *_di)
{
	struct bd99954_device_info *di = _di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	hwlog_info("bd99954 int happened %d\n", di->irq_active);

	if (di->irq_active == 1) {
		di->irq_active = 0;
		disable_irq_nosync(di->irq_int);
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("the irq is not enable, do nothing\n");
	}

	return IRQ_HANDLED;
}

static int bd99954_irq_init(struct bd99954_device_info *di,
	struct device_node *np)
{
	int ret;

	ret = power_gpio_config_interrupt(np,
		"gpio_int", "charger_int", &di->gpio_int, &di->irq_int);
	if (ret)
		return -EINVAL;

	ret = request_irq(di->irq_int, bd99954_interrupt,
		IRQF_TRIGGER_FALLING, "charger_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		goto irq_init_err;
	}

	disable_irq(di->irq_int);
	di->irq_active = 0;
	return 0;
irq_init_err:
	gpio_free(di->gpio_int);
	return ret;
}

static void bd99954_init_charge_para(struct bd99954_device_info *di)
{
	u16 reg = 0;
	int ret;

	di->chg_done = FALSE;
	di->term_curr = ITERM_DEFAULT;
	di->iterm_pre = ITERM_DEFAULT;
	di->dpm_half_occur = false;
	di->bal_event = BAL_E_MAX;
	di->en_last = 1;

	ret = bd99954_read_word(BD99954_REG_VFASTCHG_REG_SET1, &reg);
	if (ret < 0)
		hwlog_err("read reg fail\n");
	di->vterm = (int)reg;

	reg = 0;
	bd99954_read_word(BD99954_REG_VSYSREG_SET, &reg);
	hwlog_info("vsys reg 0x%x\n", reg);
}

static int bd99954_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct bd99954_device_info *di = NULL;
	struct device_node *np = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_bd99954_dev = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	wakeup_source_init(&di->wakelock, "bd99954_wakelock");
	INIT_WORK(&di->irq_work, bd99954_irq_work);
	INIT_DELAYED_WORK(&di->vbus_reverse_wk, bd99954_vbus_reverse_work);
	ret = init_charge_reg();
	if (ret < 0) {
		hwlog_info("init_charge_reg fail\n");
		goto destory_wk;
	}

	if (power_gpio_config_output(np,
		"gpio_otg", "gpio_otg", &di->gpio_otg, GPIO_LOW_LEVEL))
		return -EINVAL;

	ret = bd99954_irq_init(di, np);
	if (ret) {
		hwlog_err("bd99954_irq_init failed\n");
		goto gpio_fail;
	}

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "vbat_div_ratio",
		(u32 *)&di->vbat_div_ratio, BD99954_THOUSAND);
	if (di->vbat_div_ratio == 0)
		di->vbat_div_ratio = BD99954_THOUSAND;

	ret = hw_battery_voltage_ops_register(&bd99954_vbatt_ops, "bq25882");
	if (ret)
		hwlog_err("bd99954 hw_battery_voltage ops register failed\n");

	ret = series_batt_ops_register(&bd99954_chg_ops);
	if (ret) {
		hwlog_err("bd99954 series_batt ops register fail\n");
		goto sysfs_fail;
	}

	ret = charger_otg_ops_register(&bd99954_otg_ops);
	if (ret) {
		hwlog_err("bd99954 charger otg ops register fail\n");
		goto sysfs_fail;
	}

	di->bal_nb.notifier_call = bd99954_bal_notifier;
	batt_bal_state_notifier_register(&di->bal_nb);

	bd99954_init_charge_para(di);
	bd99954_log_ops.dev_data = (void *)di;
	power_log_ops_register(&bd99954_log_ops);

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = 0;
		power_dev_info->ver_id = 0;
	}
	return 0;

sysfs_fail:
	free_irq(di->irq_int, di);
	gpio_free(di->gpio_int);
gpio_fail:
	gpio_free(di->gpio_otg);
destory_wk:
	wakeup_source_trash(&di->wakelock);

	return ret;
}

static int bd99954_remove(struct i2c_client *client)
{
	struct bd99954_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	if (di->gpio_otg)
		gpio_free(di->gpio_otg);

	batt_bal_state_notifier_unregister(&di->bal_nb);
	wakeup_source_trash(&di->wakelock);
	g_bd99954_dev = NULL;
	return 0;
}

static void bd99954_shutdown(struct i2c_client *client)
{
	(void)bd99954_write_word(BD99954_REG_SMBREG, BD99954_REG_SMBREG_5H);
	bd99954_set_vsysreg(VSYSREG_DEFAULT);
}

#ifdef CONFIG_PM
static int bd99954_resume(struct device *dev)
{
	struct bd99954_device_info *di = g_bd99954_dev;

	hwlog_info("resume begin\n");

	if (!di)
		return 0;

	schedule_delayed_work(&di->vbus_reverse_wk,
		msecs_to_jiffies(0));

	hwlog_info("resume end\n");
	return 0;
}

static int bd99954_suspend(struct device *dev)
{
	struct bd99954_device_info *di = g_bd99954_dev;

	hwlog_info("suspend begin\n");

	if (!di)
		return 0;

	cancel_delayed_work(&di->vbus_reverse_wk);

	if (charge_get_charger_type() == CHARGER_REMOVED)
		bd99954_set_chop_all(0);
	else
		bd99954_set_chop_all(1);

	hwlog_info("suspend end\n");
	return 0;
}

static const struct dev_pm_ops bd99954_pm_ops = {
	.suspend = bd99954_suspend,
	.resume  = bd99954_resume,
};

#define BD99954_PM_OPS (&bd99954_pm_ops)
#else
#define BD99954_PM_OPS (NULL)
#endif /* CONFIG_PM */

MODULE_DEVICE_TABLE(i2c, bd99954);
static const struct of_device_id bd99954_of_match[] = {
	{
		.compatible = "rohm,bd99954_charger",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id bd99954_i2c_id[] = {
	{ "bd99954_charger", 0 }, {}
};

static struct i2c_driver bd99954_driver = {
	.probe = bd99954_probe,
	.remove = bd99954_remove,
	.shutdown = bd99954_shutdown,
	.id_table = bd99954_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "bd99954_charger",
		.of_match_table = of_match_ptr(bd99954_of_match),
		.pm = BD99954_PM_OPS,
	},
};

static int __init bd99954_init(void)
{
	return i2c_add_driver(&bd99954_driver);
}

static void __exit bd99954_exit(void)
{
	i2c_del_driver(&bd99954_driver);
}

module_init(bd99954_init);
module_exit(bd99954_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("bd99954 charger module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
