/*
 * hisi_scharger_v200.c
 *
 * Description: HI6522 charger driver
 *
 * Copyright (c) 2015-2020 Huawei Technologies Co., Ltd.
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

#include "hisi_scharger_v200.h"
#include "hisi_scharger_v200_common_irq.h"
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/usb/otg.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <huawei_platform/power/huawei_charger.h>
#include <linux/power/hisi/coul/coul_drv.h>
#include <linux/power/hisi/bci_battery.h>
#include <linux/power/hisi_hi6521_charger_power.h>
#include <linux/raid/pq.h>
#ifdef CONFIG_HUAWEI_OCP
#include <huawei_platform/ocp/hw_ocp_ext.h>
#endif
#include <chipset_common/hwpower/hardware_monitor/uscp.h>
#include <linux/version.h>

struct hi6522_device_info *g_hi6522_dev;
struct hi6522_device_info *scharger_di;
static u8 dpm_switch_enable;
static u32 is_board_type; /* 0:sft 1:udp 2:asic */
int hi6522_get_charge_state(unsigned int *state);
/* i_in_lim[I_IN_LIM_GEAR]: the input current limit gear, unit: mA */
int i_in_lim[I_IN_LIM_GEAR] = { 100, 150, 500, 900, 1000, 1200, 1500,
				1600, 1800, 2000, 2200, 2500 };
static unsigned int iin_limit_option[IIN_OPTION_NUM] = {
				500, 1000, 1200, 1500, 1600, 1800, 2000 };
static int hiz_iin_limit_flag = HIZ_IIN_FLAG_FALSE;

struct hi6522_device_info *get_hi6522_dev(void)
{
	return g_hi6522_dev;
}
#define CONFIG_SYSFS_SCHG
#ifdef CONFIG_SYSFS_SCHG

/*
 * There are a numerous options that are configurable on the HI6521
 * that go well beyond what the power_supply properties provide access to.
 * Provide sysfs access to them so they can be examined and possibly modified
 * on the fly.
 */
#define hi6522_sysfs_field(_name, r, f, m, store)                  \
{\
	.attr = __ATTR(_name, m, hi6522_sysfs_show, store),    \
	.reg = CHG_##r##_REG, .mask = CHG_##f##_MSK, .shift = CHG_##f##_SHIFT,\
}

#define hi6522_sysfs_field_rw(_name, r, f)                     \
	hi6522_sysfs_field(_name, r, f, 0640, hi6522_sysfs_store)

static ssize_t hi6522_sysfs_show(struct device *dev,
				 struct device_attribute *attr, char *buf);

static ssize_t hi6522_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

struct hi6522_sysfs_field_info {
	struct device_attribute attr;
	u8 reg;
	u8 mask;
	u8 shift;
};

static struct hi6522_sysfs_field_info hi6522_sysfs_field_tbl[] = {
	hi6522_sysfs_field_rw(dpm_switch_enable, NONE, NONE),
	hi6522_sysfs_field_rw(reg_addr, NONE, NONE),
	hi6522_sysfs_field_rw(reg_value, NONE, NONE),
};

static struct attribute *hi6522_sysfs_attrs[ARRAY_SIZE(hi6522_sysfs_field_tbl) + 1];

static const struct attribute_group hi6522_sysfs_attr_group = {
	.attrs = hi6522_sysfs_attrs,
};

/* initialize hi6522_sysfs_attrs[] for HI6523 attribute */
static void hi6522_sysfs_init_attrs(void)
{
	int i;
	int limit = ARRAY_SIZE(hi6522_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		hi6522_sysfs_attrs[i] = &hi6522_sysfs_field_tbl[i].attr.attr;

	hi6522_sysfs_attrs[limit] = NULL; /* Has additional entry for this */
}

static struct hi6522_sysfs_field_info *hi6522_sysfs_field_lookup(
	const char *name)
{
	int i;
	int limit =  ARRAY_SIZE(hi6522_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		if (!strcmp(name, hi6522_sysfs_field_tbl[i].attr.attr.name))
			break;

	if (i >= limit)
		return NULL;

	return &hi6522_sysfs_field_tbl[i];
}

static ssize_t hi6522_sysfs_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct hi6522_sysfs_field_info *info = NULL;
	struct hi6522_sysfs_field_info *info2 = NULL;
#ifdef CONFIG_HISI_DEBUG_FS
	int ret;
#endif
	u8 v;

	info = hi6522_sysfs_field_lookup(attr->attr.name);
	if (info == NULL)
		return -EINVAL;

	if (!strncmp("dpm_switch_enable", attr->attr.name,
		strlen("dpm_switch_enable")))
		return scnprintf(buf, PAGE_SIZE, "%d\n", dpm_switch_enable);

	if (!strncmp("reg_addr", attr->attr.name, strlen("reg_addr")))
		return scnprintf(buf, PAGE_SIZE, "0x%hhx\n", info->reg);

	if (!strncmp("reg_value", attr->attr.name, strlen("reg_value"))) {
		info2 = hi6522_sysfs_field_lookup("reg_addr");
		if (info2 == NULL)
			return -EINVAL;
		info->reg = info2->reg;
	}
#ifdef CONFIG_HISI_DEBUG_FS
	ret = hi6522_read_mask(info->reg, info->mask, info->shift, &v);
	if (ret)
		return ret;
#endif

	return scnprintf(buf, PAGE_SIZE, "0x%hhx\n", v);
}

/* set the value for all HI6523 device's node */
static ssize_t hi6522_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hi6522_sysfs_field_info *info = NULL;
	struct hi6522_sysfs_field_info *info2 = NULL;
	int ret;
	u8 v;

	info = hi6522_sysfs_field_lookup(attr->attr.name);
	if (info == NULL)
		return -EINVAL;

	ret = kstrtou8(buf, 0, &v);
	if (ret < 0)
		return ret;

	if (!strncmp("dpm_switch_enable", attr->attr.name,
		strlen("dpm_switch_enable"))) {
		if (v == 1 || v == 0) {
			dpm_switch_enable = v;
			return count;
		} else {
			return -EINVAL;
		}
	}

	if (!strncmp("reg_value", attr->attr.name, strlen("reg_value"))) {
		info2 = hi6522_sysfs_field_lookup("reg_addr");
		if (info2 == NULL)
			return -EINVAL;
		info->reg = info2->reg;
	}
	if (!strncmp("reg_addr", attr->attr.name, strlen("reg_addr"))) {
		if (v < (u8)HI6522_REG_TOTAL_NUM) {
			info->reg = v;
			return count;
		} else {
			return -EINVAL;
		}
	}
#ifdef CONFIG_HISI_DEBUG_FS
	ret = hi6522_write_mask(info->reg, info->mask, info->shift, v);
	if (ret)
		return ret;
#endif

	return count;
}

static int hi6522_sysfs_create_group(struct hi6522_device_info *di)
{
	hi6522_sysfs_init_attrs();
	if (!power_sysfs_create_link_group("hw_power", "charger", "HI6522",
					   di->dev, &hi6522_sysfs_attr_group))
		return -1;

	return 0;
}

static void hi6522_sysfs_remove_group(struct hi6522_device_info *di)
{
	(void)power_sysfs_remove_link_group("hw_power", "charger", "HI6522",
					    di->dev, &hi6522_sysfs_attr_group);
}
#else

static int hi6522_sysfs_create_group(struct hi6522_device_info *di)
{
	return 0;
}

static inline void hi6522_sysfs_remove_group(struct hi6522_device_info *di)
{
}
#endif

int hi6522_set_input_current(int cin_limit)
{
	u8 i_in_limit, i;

	if (hiz_iin_limit_flag == HIZ_IIN_FLAG_TRUE) {
		scharger_err("%s hiz_iin_limit_flag,just set 100mA!\n", __func__);
		cin_limit = IINLIM_100;
	}
	/* i_in_limit(0~11): numbers marking different current limit */
	if (cin_limit <= IINLIM_100)
		i_in_limit = 0;
	else if (cin_limit > IINLIM_2500)
		i_in_limit = 4;
	/*
	 * This part is reconstructed from a series of
	 * "else if", such as if cin_limit > IINLIM_2000
	 * and cin_limit <= IINLIM_2200. There are 11 constinuous
	 * gears from IINLIM_100 to IINLIM_2500, the revision
	 * is good for reducing cyclomatic complexity.
	 * 12: the total gear of current limit
	 * cin_limit: current gear value
	 */
	for (i = 1; i < I_IN_LIM_GEAR; i++)
		if (cin_limit > i_in_lim[i - 1] && cin_limit <= i_in_lim[i])
			i_in_limit = i;
	scharger_inf("input current reg is set 0x%x\n", i_in_limit);
	return hi6522_write_mask(CHG_INPUT_SOURCE_REG, CHG_ILIMIT_MSK,
				 CHG_ILIMIT_SHIFT, i_in_limit);
}

static int hi6522_set_dpm_voltage(int vindpm)
{
	u8 vindpm_limit;

	if (vindpm < VINDPM_MIN_3880)
		vindpm = VINDPM_MIN_3880;
	else if (vindpm > VINDPM_MAX_5080)
		vindpm = VINDPM_MAX_5080;
	vindpm_limit =
		(unsigned char)((vindpm - VINDPM_MIN_3880) / VINDPM_STEP_80);

	scharger_inf("dpm will be set to %u mV, expect: vindpm=%d mV\n",
		(VINDPM_MIN_3880 + vindpm_limit * VINDPM_STEP_80), vindpm);
	return hi6522_write_mask(CHG_DPM_SEL_REG, CHG_DPM_SEL_MSK,
				 CHG_DPM_SEL_SHIFT, vindpm_limit);
}

static int hi6522_set_precharge_current(int precharge_current)
{
	u8 prechg_limit;

	if (precharge_current < CHG_PRG_ICHG_MIN)
		precharge_current = CHG_PRG_ICHG_MIN;
	else if (precharge_current > CHG_PRG_ICHG_MAX)
		precharge_current = CHG_PRG_ICHG_MAX;

	prechg_limit =
		(u8)((precharge_current - CHG_PRG_ICHG_MIN) / CHG_PRG_ICHG_STEP);

	return hi6522_write_mask(CHG_PRE_ICHG_REG, CHG_PRE_ICHG_MSK,
				 CHG_PRE_ICHG_SHIFT, prechg_limit);
}

static int hi6522_set_precharge_voltage(u32 pre_vchg)
{
	u8 vprechg;

	/*
	 * 0: preset charge vol <2.8V
	 * 1: preset charge vol 2.8V
	 * 2: preset charge vol 3.0V
	 * 3: preset charge vol 3.1V
	 */
	if (pre_vchg <= CHG_PRG_VCHG_2800)
		vprechg = 0;
	else if (pre_vchg > CHG_PRG_VCHG_2800 && pre_vchg <= CHG_PRG_VCHG_3000)
		vprechg = 1;
	else if (pre_vchg > CHG_PRG_VCHG_3000 && pre_vchg <= CHG_PRG_VCHG_3100)
		vprechg = 2; /* 2: preset charge vol 3.0V */
	else if (pre_vchg > CHG_PRG_VCHG_3100 && pre_vchg <= CHG_PRG_VCHG_3200)
		vprechg = 3; /* 3: preset charge vol 3.1V */
	else
		vprechg = 0; /* default 2.8V */
	return hi6522_write_mask(CHG_PRE_VCHG_REG, CHG_PRE_VCHG_MSK,
				 CHG_PRE_VCHG_SHIFT, vprechg);
}

static int hi6522_set_batfet_ctrl(u32 status)
{
	return hi6522_write_mask(BATFET_CTRL_CFG_REG, BATFET_CTRL_CFG_MSK,
				 BATFET_CTRL_CFG_SHIFT, status);
}

int hi6522_set_batfet_disable(int disable)
{
	struct hi6522_device_info *di = g_hi6522_dev;

	if (di == NULL) {
		scharger_err("%s hi6522_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}

	di->batfet_disable_flag = disable;

	return hi6522_set_batfet_ctrl(!disable);
}

static int hi6522_set_charge_current(int charge_current)
{
	u8 i_chg_limit;

	if (charge_current < CHG_FAST_ICHG_MIN)
		charge_current = CHG_FAST_ICHG_MIN;
	else if (charge_current > CHG_FAST_ICHG_MAX)
		charge_current = CHG_FAST_ICHG_MAX;
	i_chg_limit = (unsigned char)((charge_current - CHG_FAST_ICHG_MIN) /
		       CHG_FAST_ICHG_STEP_100);
	scharger_inf("charge current reg is set %d mA\n", charge_current);
	return hi6522_write_mask(CHG_FAST_CURRENT_REG, CHG_FAST_ICHG_MSK,
				 CHG_FAST_ICHG_SHIFT, i_chg_limit);
}

/* Parameters: value:terminal current value in the unit of mA */
static int hi6522_set_terminal_current(int term_current)
{
	u8 i_term;

	if (term_current < CHG_TERM_ICHG_MIN)
		term_current = CHG_TERM_ICHG_MIN;
	else if (term_current > CHG_TERM_ICHG_MAX)
		term_current = CHG_TERM_ICHG_MAX;
	i_term = (unsigned char)((term_current - CHG_TERM_ICHG_MIN) /
			CHG_TERM_ICHG_STEP_50);

	scharger_inf("term current is set %d mA\n", term_current);
	return hi6522_write_mask(CHG_TERM_ICHG_REG, CHG_TERM_ICHG_MSK,
				 CHG_TERM_ICHG_SHIFT, i_term);
}

/* Parameters: value:terminal voltage value in the unit of mV */
static int hi6522_set_terminal_voltage(int charge_voltage)
{
	u8 data;

	if (charge_voltage < CHG_FAST_VCHG_MIN)
		charge_voltage = CHG_FAST_VCHG_MIN;
	else if (charge_voltage > CHG_FAST_VCHG_MAX)
		charge_voltage = CHG_FAST_VCHG_MAX;

	data = (u8)((charge_voltage - CHG_FAST_VCHG_MIN) /
			CHG_FAST_VCHG_STEP_50);
	return hi6522_write_mask(CHG_FAST_VCHG_REG, CHG_FAST_VCHG_MSK,
				 CHG_FAST_VCHG_SHIFT, data);
}

/* Parameters: value:vclamp in the unit of mV */
static int hi6522_set_vclamp(int value)
{
	u8 reg;

	if (value < CHG_IR_VCLAMP_MIN)
		value = CHG_IR_VCLAMP_MIN;
	else if (value > CHG_IR_VCLAMP_MAX)
		value = CHG_IR_VCLAMP_MAX;
	reg = (value - CHG_IR_VCLAMP_MIN) / CHG_IR_VCLAMP_STEP;
	return hi6522_write_mask(CHG_IR_VCLAMP_REG, CHG_IR_VCLAMP_MSK,
				 CHG_IR_VCLAMP_SHIFT, reg);
}

static int hi6522_set_fast_safe_timer(u32 chg_fastchg_safe_timer)
{
	return hi6522_write_mask(
		CHG_FAST_SAFE_TIMER_TERM_CTRL_REG, CHG_FAST_SAFE_TIMER_MSK,
		CHG_FAST_SAFE_TIMER_SHIFT, (u8)chg_fastchg_safe_timer);
}

static int hi6522_set_term_enable(int enable)
{
	return hi6522_write_mask(CHG_EN_TERM_REG, CHG_EN_TERM_MSK,
				 CHG_EN_TERM_SHIFT, enable);
}

static int hi6522_set_watchdog_timer(int value)
{
	u8 val;
	u8 dog_time = (u8)value;

	if (dog_time >= TIMER_40S)
		val = 3; /* 3: marking [TIMER_40S,+∞) */
	else if (dog_time >= TIMER_20S)
		val = 2; /* 2: marking [TIMER_20S,TIMER_40S) */
	else if (dog_time >= TIMER_10S)
		val = 1; /* 1: marking [TIMER_10S,TIMER_20S) */
	else
		val = 0; /* 0: default state */

	scharger_inf("watch dog timer is %u ,the register value is set %u\n",
		dog_time, val);
	return hi6522_write_mask(CHG_WDT_TIMER_REG, CHG_WDT_TIMER_MSK,
				 CHG_WDT_TIMER_SHIFT, val);
}

static int hi6522_reset_watchdog_timer(void)
{
	u8 dog_status = 0;
	int ret;

	/* scharger v200 need to write wdt timer */
	ret = hi6522_read_byte(CHG_WDT_TIMER_REG, &dog_status);
	if (ret)
		return ret;
	if ((dog_status & CHG_WDT_TIMER_MSK) == 0)
		hi6522_set_watchdog_timer(TIMER_40S);
	return hi6522_write_mask(CHG_WDT_RST_REG, CHG_WDT_RST_MSK,
				 CHG_WDT_RST_SHIFT, CHG_WDT_RST);
}

/* print the register value in charging process */
static int hi6522_dump_register(char *reg_value, int size, void *dev_data)
{
	u8 reg[HI6522_REG_TOTAL_NUM] = {0};
	char buff[DUMP_REG_LEN] = {0};
	int ret;
	int i;
	struct hi6522_device_info *di = g_hi6522_dev;

	if (di == NULL) {
		scharger_err("%s hi6522_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}
	memset(reg_value, 0, size);
	ret = hi6522_read_block(di, &reg[0], 0, HI6522_REG_TOTAL_NUM);
	if (ret != 0)
		scharger_err("%s hi6522_read_block failed!\n", __func__);

	for (i = 0; i < HI6522_REG_TOTAL_NUM; i++) {
		snprintf(buff, DUMP_REG_LEN, "0x%-8x", reg[i]);
		strncat(reg_value, buff, strlen(buff));
	}
	return 0;
}

/* print the register head in charging process */
static int hi6522_get_register_head(char *reg_head, int size, void *dev_data)
{
	char buff[DUMP_REG_LEN] = { 0 };
	int i;

	memset(reg_head, 0, size);
	for (i = 0; i < HI6522_REG_TOTAL_NUM; i++) {
		snprintf(buff, DUMP_REG_LEN, "Reg[0x%2x]", i);
		strncat(reg_head, buff, strlen(buff));
	}
	return 0;
}

/* Description: config onff1 to disable/enable chg/otg/wled/ldo_200mA */
static int hi6522_config_power_on_reg01(struct hi6522_device_info *di)
{
	int ret;

	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s sema fail!\n", __func__);
		return -EINTR;
	}
	di->power_on_config_reg01.reg.chg_en_int = di->chg_en;
	di->power_on_config_reg01.reg.otg_en_int = di->otg_en;
	di->power_on_config_reg01.reg.wled_en_int = di->wled_en;
	di->power_on_config_reg01.reg.ldo1_200ma_en_int = di->ldo1_200ma_en;
	di->power_on_config_reg01.reg.ldo2_200ma_en_int = di->ldo2_200ma_en;

	ret = hi6522_write_byte(POWER_ON_CONFIG_REG01,
		di->power_on_config_reg01.value);
	up(&di->charger_data_busy_lock);
	scharger_inf("%s :0x%x\n", __func__, di->power_on_config_reg01.value);
	return ret;
}

static int hi6522_set_charge_enable(int val)
{
	int ret0, ret1;
	struct hi6522_device_info *di = g_hi6522_dev;

	if (di == NULL) {
		scharger_err("%s hi6522_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}

	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s sema fail!\n", __func__);
		return -EINTR;
	}
	di->chg_en = (unsigned char)val;
	ret0 = hi6522_read_byte(POWER_ON_CONFIG_REG01,
				&di->power_on_config_reg01.value);
	di->power_on_config_reg01.reg.chg_en_int = di->chg_en;
	ret1 = hi6522_write_byte(POWER_ON_CONFIG_REG01,
				 di->power_on_config_reg01.value);
	up(&di->charger_data_busy_lock);
	scharger_inf("[%s] :power_on_config_reg01 is 0x%x\n", __func__,
		     di->power_on_config_reg01.value);
	return (ret0 || ret1);
}

static int hi6522_config_charger_nolock_enable(struct hi6522_device_info *di)
{
	int ret0, ret1;

	ret0 = hi6522_read_byte(POWER_ON_CONFIG_REG01,
				&di->power_on_config_reg01.value);
	di->power_on_config_reg01.reg.chg_en_int = di->chg_en;
	ret1 = hi6522_write_byte(POWER_ON_CONFIG_REG01,
				 di->power_on_config_reg01.value);
	scharger_inf("[%s] :power_on_config_reg01 is 0x%x\n", __func__,
		     di->power_on_config_reg01.value);
	return (ret0 || ret1);
}

int hi6522_set_otg_enable(int val)
{
	int ret0, ret1;
	struct hi6522_device_info *di = g_hi6522_dev;

	if (di == NULL) {
		scharger_err("%s hi6522_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}

	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("hi6522_config_otg_enable sema fail!\n");
		return -EINTR;
	}
	if (val) {
		di->otg_en = CHG_OTG_FORCE_EN;
		di->chg_en = CHG_DISABLE;
	} else {
		di->otg_en = CHG_OTG_FORCE_DIS;
	}
	/* config otg en */
	ret0 = hi6522_read_byte(POWER_ON_CONFIG_REG01,
				&di->power_on_config_reg01.value);
	di->power_on_config_reg01.reg.otg_en_int = di->otg_en;
	di->power_on_config_reg01.reg.chg_en_int = di->chg_en;
	ret1 = hi6522_write_byte(POWER_ON_CONFIG_REG01,
				 di->power_on_config_reg01.value);
	up(&di->charger_data_busy_lock);
	scharger_inf("[%s] :power_on_config_reg01 is 0x%x\n", __func__,
		     di->power_on_config_reg01.value);
	return (ret0 || ret1);
}

int hi6522_set_otg_current(int value)
{
	unsigned int temp_current_ma;
	u8 reg;

	temp_current_ma = (unsigned int)value;
	if (temp_current_ma < BOOST_LIM_MIN || temp_current_ma > BOOST_LIM_MAX)
		scharger_inf("set otg current %dmA is out of range!\n", value);

	if (temp_current_ma < BOOST_LIM_MAX)
		reg = 0;
	else
		reg = 1;

	scharger_inf("otg current reg is set %u mA\n", (reg + 1) * BOOST_LIM_MIN);
	return hi6522_write_mask(CHG_OTG_REG0, CHG_OTG_LIM_MSK,
				 CHG_OTG_LIM_SHIFT, reg);
}

int hi6522_config_wled_enable(struct hi6522_device_info *di)
{
	int ret0, ret1;

	if (di == NULL) {
		scharger_err("%s input hi6522_device_info di is NULL\n", __func__);
		return -ENOMEM;
	}
	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s sema fail!\n", __func__);
		return -EINTR;
	}
	ret0 = hi6522_read_byte(POWER_ON_CONFIG_REG01, &di->power_on_config_reg01.value);
	di->power_on_config_reg01.reg.wled_en_int = di->wled_en;
	ret1 = hi6522_write_byte(POWER_ON_CONFIG_REG01, di->power_on_config_reg01.value);

	up(&di->charger_data_busy_lock);
	scharger_inf("[%s] :power_on_config_reg01 is 0x%x\n", __func__,
		     di->power_on_config_reg01.value);
	return (ret0 || ret1);
}

int hi6522_config_enable_chopper(struct hi6522_device_info *di, int value)
{
	int ret0, ret1;

	unsigned char val = 0;

	if (di == NULL) {
		scharger_err("input ptr di is NULL!\n");
		return -ENOMEM;
	}
	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("hi6522_config_wled_enable sema fail!\n");
		return -EINTR;
	}
	ret0 = hi6522_read_byte(WLED_REG8_CONTROL, (u8 *)&val);
	val &= HI6XXX_WLED_REG8_CHOOPER_MASK;
	if (value == 1)
		val |= HI6XXX_WLED_REG8_CHOOPER_ENABLE;

	ret1 = hi6522_write_byte(WLED_REG8_CONTROL, val);

	up(&di->charger_data_busy_lock);
	scharger_err("[%s] :is %u\n", __func__, val);
	return (ret0 || ret1);
}

int hi6522_config_enable_ifb1_ifb2(struct hi6522_device_info *di, int mode)
{
	int ret0, ret1;
	unsigned char val = 0;

	if (di == NULL) {
		scharger_err("input ptr hi6522_device_info is NULL!\n");
		return -ENOMEM;
	}
	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("hi6522_config_wled_enable sema fail!\n");
		return -EINTR;
	}
	ret0 = hi6522_read_byte(WLED_REG5_CONTROL, (u8 *)&val);
	val &= HI6XXX_IFB1_IFB2_ENABLE_MASK;
	val |= (unsigned char)(mode);

	ret1 = hi6522_write_byte(WLED_REG5_CONTROL, (u8)val);

	up(&di->charger_data_busy_lock);
	scharger_err("[%s] :hi6522_config_enable_ifb1_ifb22 is %u\n",
		     __func__, val);
	return (ret0 || ret1);
}

/* config ldo1_200ma_en enable/disable,for NFC use */
int hi6522_config_ldo1_200ma_enable(struct hi6522_device_info *di)
{
	int ret0, ret1;

	if (di == NULL) {
		scharger_err("input ptr hi6522_device_info is NULL!\n");
		return -ENOMEM;
	}
	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s sema fail!\n", __func__);
		return -EINTR;
	}
	ret0 = hi6522_read_byte(POWER_ON_CONFIG_REG01, &di->power_on_config_reg01.value);
	di->power_on_config_reg01.reg.ldo1_200ma_en_int = di->ldo1_200ma_en;
	ret1 = hi6522_write_byte(POWER_ON_CONFIG_REG01, di->power_on_config_reg01.value);
	up(&di->charger_data_busy_lock);
	scharger_inf("[%s] :power_on_config_reg01 is 0x%x\n", __func__,
		     di->power_on_config_reg01.value);
	return (ret0 || ret1);
}

/* Description: config ldo1_200ma_en enable/disable,NFC use */
int hi6522_config_ldo2_200ma_enable(struct hi6522_device_info *di)
{
	int ret0, ret1;

	if (di == NULL) {
		scharger_err("input ptr hi6522_device_info is NULL!\n");
		return -ENOMEM;
	}
	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s fail!\n", __func__);
		return -EINTR;
	}
	ret0 = hi6522_read_byte(POWER_ON_CONFIG_REG01, &di->power_on_config_reg01.value);
	di->power_on_config_reg01.reg.ldo2_200ma_en_int = di->ldo2_200ma_en;
	ret1 = hi6522_write_byte(POWER_ON_CONFIG_REG01, di->power_on_config_reg01.value);
	up(&di->charger_data_busy_lock);
	scharger_inf("[%s] :power_on_config_reg01 is 0x%x\n", __func__,
		     di->power_on_config_reg01.value);
	return (ret0 || ret1);
}

int hi6522_config_lcd_boost_enable(struct hi6522_device_info *di)
{
	int ret0, ret1;

	if (di == NULL) {
		scharger_err("input ptr hi6522_device_info is NULL!\n");
		return -ENOMEM;
	}
	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s sema fail!\n", __func__);
		return -EINTR;
	}
	ret0 = hi6522_read_byte(POWER_ON_CONFIG_REG00, &di->power_on_config_reg00.value);
	di->power_on_config_reg00.reg.lcd_bst_en_int = di->lcd_bst_en;
	ret1 = hi6522_write_byte(POWER_ON_CONFIG_REG00, di->power_on_config_reg00.value);
	up(&di->charger_data_busy_lock);
	scharger_inf("[%s] :power_on_config_reg00 is 0x%x\n", __func__,
		     di->power_on_config_reg00.value);
	return (ret0 || ret1);
}

/* config lcd_ldo enable/disable,LCD+5v */
int hi6522_config_lcd_ldo_enable(struct hi6522_device_info *di)
{
	int ret0, ret1;

	if (di == NULL) {
		scharger_err("input ptr hi6522_device_info is NULL!\n");
		return -ENOMEM;
	}
	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s sema fail!\n", __func__);
		return -EINTR;
	}
	ret0 = hi6522_read_byte(POWER_ON_CONFIG_REG00,
				&di->power_on_config_reg00.value);
	di->power_on_config_reg00.reg.lcd_ldo_en_int = di->lcd_ldo_en;
	ret1 = hi6522_write_byte(POWER_ON_CONFIG_REG00,
				 di->power_on_config_reg00.value);
	up(&di->charger_data_busy_lock);
	scharger_inf("[%s] :power_on_config_reg00 is 0x%x\n", __func__,
		     di->power_on_config_reg00.value);
	return (ret0 || ret1);
}

/* config lcd_ncp enable/disable,LCD-5V */
int hi6522_config_lcd_ncp_enable(struct hi6522_device_info *di)
{
	int ret0, ret1;

	if (di == NULL) {
		scharger_err("input ptr hi6522_device_info is NULL!\n");
		return -ENOMEM;
	}
	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s sema fail!\n", __func__);
		return -EINTR;
	}
	ret0 = hi6522_read_byte(POWER_ON_CONFIG_REG00,
				&di->power_on_config_reg00.value);
	di->power_on_config_reg00.reg.lcd_ncp_en_int = di->lcd_ncp_en;
	ret1 = hi6522_write_byte(POWER_ON_CONFIG_REG00,
				 di->power_on_config_reg00.value);
	up(&di->charger_data_busy_lock);
	scharger_inf("[%s] :power_on_config_reg00 is 0x%x\n", __func__,
		     di->power_on_config_reg00.value);
	return (ret0 || ret1);
}

/* Description: config lcd_ldo_ncp enable/disable,LCD+5v-5v simultaneously */
static int hi6522_config_lcd_ldo_ncp_enable(struct hi6522_device_info *di)
{
	int ret0, ret1;

	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("hi6522_config_lcd_ldo_enable sema fail!\n");
		return -EINTR;
	}
	ret0 = hi6522_read_byte(POWER_ON_CONFIG_REG00,
				&di->power_on_config_reg00.value);
	di->power_on_config_reg00.reg.lcd_ldo_en_int = di->lcd_ldo_en;
	di->power_on_config_reg00.reg.lcd_ncp_en_int = di->lcd_ncp_en;
	ret1 = hi6522_write_byte(POWER_ON_CONFIG_REG00,
				 di->power_on_config_reg00.value);
	up(&di->charger_data_busy_lock);
	scharger_inf("[%s] :power_on_config_reg00 is 0x%x\n", __func__,
		     di->power_on_config_reg00.value);
	return (ret0 || ret1);
}

int hi6522_config_flash_boost_enable(struct hi6522_device_info *di)
{
	int ret0, ret1;

	if (di == NULL) {
		scharger_err("input ptr hi6522_device_info is NULL!\n");
		return -ENOMEM;
	}
	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s sema fail!\n", __func__);
		return -EINTR;
	}
	ret0 = hi6522_read_byte(POWER_ON_CONFIG_REG00,
				&di->power_on_config_reg00.value);
	di->power_on_config_reg00.reg.flash_bst_en_int = di->flash_bst_en;
	ret1 = hi6522_write_byte(POWER_ON_CONFIG_REG00,
				 di->power_on_config_reg00.value);
	up(&di->charger_data_busy_lock);
	scharger_inf("[%s]: power_on_config_reg00 is 0x%x\n", __func__,
		     di->power_on_config_reg00.value);
	return (ret0 || ret1);
}

int hi6522_config_flash_led_flash_enable(struct hi6522_device_info *di)
{
	int ret0, ret1;

	if (di == NULL) {
		scharger_err("input ptr hi6522_device_info is NULL!\n");
		return -ENOMEM;
	}
	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s sema fail!\n", __func__);
		return -EINTR;
	}
	ret0 = hi6522_read_byte(POWER_ON_CONFIG_REG00,
				&di->power_on_config_reg00.value);
	di->power_on_config_reg00.reg.flash_led_flash_en =
		di->flash_led_flash_en;
	ret1 = hi6522_write_byte(POWER_ON_CONFIG_REG00,
				 di->power_on_config_reg00.value);
	up(&di->charger_data_busy_lock);
	scharger_inf("[%s] :power_on_config_reg00 is 0x%x\n", __func__,
		     di->power_on_config_reg00.value);
	return (ret0 || ret1);
}

int hi6522_config_flash_led_torch_enable(struct hi6522_device_info *di)
{
	int ret0, ret1;

	if (di == NULL) {
		scharger_err("input ptr hi6522_device_info is NULL!\n");
		return -ENOMEM;
	}
	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s sema fail!\n", __func__);
		return -EINTR;
	}
	ret0 = hi6522_read_byte(POWER_ON_CONFIG_REG00,
				&di->power_on_config_reg00.value);
	di->power_on_config_reg00.reg.flash_led_torch_en =
		di->flash_led_torch_en;
	ret1 = hi6522_write_byte(POWER_ON_CONFIG_REG00,
				 di->power_on_config_reg00.value);
	up(&di->charger_data_busy_lock);
	scharger_inf("[%s] :power_on_config_reg00 is 0x%x\n", __func__,
		     di->power_on_config_reg00.value);
	return (ret0 || ret1);
}

static int hi6522_config_flash_led_timeout_enable(struct hi6522_device_info *di)
{
	int ret0, ret1;

	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s sema fail!\n", __func__);
		return -EINTR;
	}
	ret0 = hi6522_read_byte(FLASH_LED_REG7_REG, &di->flash_led_reg7.value);
	di->flash_led_reg7.reg.flash_led_timeout_en =
		di->chg_flash_led_timeout_en;
	ret1 = hi6522_write_byte(FLASH_LED_REG7_REG, di->flash_led_reg7.value);
	up(&di->charger_data_busy_lock);
	return (ret0 || ret1);
}

static int hi6522_config_flash_led_timeout(struct hi6522_device_info *di)
{
	int ret0, ret1;
	unsigned int flash_led_timeout;
	unsigned char tout;

	flash_led_timeout = di->chg_flash_led_timeout;
	if (flash_led_timeout <= FLASH_LED_TIMEOUT_400)
		tout = 0; /* 0: FLASH_LED_TIMEOUT_400 */
	else if ((flash_led_timeout > FLASH_LED_TIMEOUT_400) &&
		(flash_led_timeout <= FLASH_LED_TIMEOUT_600))
		/* 0X1:(FLASH_LED_TIMEOUT_400,FLASH_LED_TIMEOUT_600] */
		tout = 0x1;
	else if ((flash_led_timeout > FLASH_LED_TIMEOUT_600) &&
		(flash_led_timeout <= FLASH_LED_TIMEOUT_800))
		/* 0X2:(FLASH_LED_TIMEOUT_600,FLASH_LED_TIMEOUT_800] */
		tout = 0x2;
	else
		/* 0X3:other conditions */
		tout = 0x3;
	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s sema fail!\n", __func__);
		return -EINTR;
	}
	ret0 = hi6522_read_byte(FLASH_LED_REG6_REG, &di->flash_led_reg6.value);
	di->flash_led_reg6.reg.flash_led_timeout = tout;
	ret1 = hi6522_write_byte(FLASH_LED_REG6_REG, di->flash_led_reg6.value);
	up(&di->charger_data_busy_lock);
	return (ret0 || ret1);
}

static int hi6522_config_flash_torch_timeout(struct hi6522_device_info *di)
{
	int ret0, ret1;
	unsigned int flash_torch_timeout;
	unsigned char tout;

	flash_torch_timeout = di->chg_flash_torch_timeout;
	/* config torch timeout */
	if (flash_torch_timeout > CHG_FLASH_TORCH_TIMEOUT_MAX)
		flash_torch_timeout = CHG_FLASH_TORCH_TIMEOUT_MAX;
	tout = (unsigned char)((flash_torch_timeout - CHG_FLASH_TORCH_TIMEOUT_MIN) /
		CHG_FLASH_TORCH_TIMEOUT_STEP_2);

	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s sema fail!\n", __func__);
		return -EINTR;
	}
	ret0 = hi6522_read_byte(FLASH_LED_REG6_REG, &di->flash_led_reg6.value);
	di->flash_led_reg6.reg.flash_led_watchdog_timer = tout;
	ret1 = hi6522_write_byte(FLASH_LED_REG6_REG, di->flash_led_reg6.value);
	up(&di->charger_data_busy_lock);
	return (ret0 || ret1);
}

/* scharger optimization config */
static void hi6522_config_opt_param(void)
{
	int ret[32] = {0}; /* 32: the number of return values of functions */
	unsigned int i;
	/* the following addresses and values are set according to nManager */
	const int addr[CONFIG_OPT_REG_NUM] = { 0x48, 0x40, 0x42, 0x3B, 0x37,
					       0x45, 0x41, 0x3e, 0x1C, 0x17,
					       0x16, 0x18, 0x2F, 0x31, 0x21,
					       0x25, 0x27, 0x64, 0x69, 0x58,
					       0x59, 0x5B, 0x5D, 0x65, 0x33,
					       0x23 };
	const int val[CONFIG_OPT_REG_NUM] = { 0x14, 0x8A, 0x2A, 0x11, 0xF3,
					      0x82, 0x25, 0x11, 0x7F, 0x78,
					      0xC8, 0x99, 0x18, 0x2C, 0x0A,
					      0x7B, 0x07, 0x26, 0x0F, 0x2F,
					      0x6C, 0xF9, 0x33, 0x12, 0x08,
					      0x85 };

	if (!(strstr(saved_command_line, "androidboot.swtype=factory"))) {
		unsigned char chooper = 0;

		ret[0] = hi6522_read_byte(0x1D, &chooper);
		/* 0x2 and 0x69: nManager chopper bits */
		chooper &= 0x2;
		chooper |= 0x69;
		ret[1] = hi6522_write_byte(0x1D, chooper);
	} else {
		ret[0] = hi6522_write_byte(0x1D, 0x6B);
	}

	/*
	 * this for loop takes place of the original tedious hi6522_write_byte
	 * 2: leap the first two as they are already assigned before
	 */
	for (i = 0; i < CONFIG_OPT_REG_NUM; i++)
		/* 2: leap the first two  */
		ret[i + 2] = hi6522_write_byte(addr[i], val[i]);

	/* 28: Modify flash_led_vbatdroop from 3.5V to 3.3V */
	ret[28] = hi6522_write_byte(0x62, 0x52);

	/* 29: Modify flash_led_vbattorch from 3.5V to 3.2V */
	ret[29] = hi6522_write_byte(0x67, 0x07);

	/* 30: Modify flash_led_isafe from 200mA to 400mA */
	ret[30] = hi6522_write_byte(0x63, 0x18);
	if (g_hi6522_dev == NULL) {
		scharger_err("hi6522_device_info is NULL!\n");
		return;
	}
	if (g_hi6522_dev->wled_ovp_vol != VOL_DEFAULT) {
		scharger_inf("Re-write WLED OVP value to %d!\n",
			     g_hi6522_dev->wled_ovp_vol);
		/* 31: Modify WLED OVP VOLTAGE */
		ret[31] = hi6522_write_mask(WLED_OVP_REG, WLED_OVP_MASK,
			WLED_OVP_SHIFT, (u8)(g_hi6522_dev->wled_ovp_vol));
	}
	for (i = 0; i < ARRAY_SIZE(ret); i++)
		if (ret[i])
			scharger_err("%s:ret[%d] fail!\n", __func__, ret[i]);
}

int scharger_power_status(int id)
{
	int ret;

	if (scharger_di == NULL)
		return -1;
	ret = hi6522_read_byte(POWER_ON_CONFIG_REG00,
			       &scharger_di->power_on_config_reg00.value);
	if (ret) {
		scharger_err("read power_on reg00 fail!\n");
		return ret;
	}
	ret = hi6522_read_byte(POWER_ON_CONFIG_REG01,
			       &scharger_di->power_on_config_reg01.value);
	if (ret) {
		scharger_err("read power_on reg01 fail!\n");
		return ret;
	}
	switch (id) {
	case SCHG_BOOST1_ID:    /* otg */
		ret = scharger_di->power_on_config_reg01.reg.otg_en_int;
		break;
	case SCHG_BOOST2_ID:    /* wled */
		ret = scharger_di->power_on_config_reg01.reg.wled_en_int;
		break;
	case SCHG_BOOST3_ID:    /* flash_bst */
		ret = scharger_di->power_on_config_reg00.reg.flash_bst_en_int;
		break;
	case SCHG_SOURCE1_ID:   /* flash_led_flash */
		ret = scharger_di->power_on_config_reg00.reg.flash_led_flash_en;
		break;
	case SCHG_SOURCE2_ID:   /* flash_led_torch */
		ret = scharger_di->power_on_config_reg00.reg.flash_led_torch_en;
		break;
	case SCHG_BOOST4_ID:    /* lcd_boost */
		ret = scharger_di->power_on_config_reg00.reg.lcd_bst_en_int;
		break;
	case SCHG_LDO3_ID:      /* lcd_ldo */
		ret = scharger_di->power_on_config_reg00.reg.lcd_ldo_en_int;
		break;
	case SCHG_CPN1_ID:      /* lcd_ncp */
		ret = scharger_di->power_on_config_reg00.reg.lcd_ncp_en_int;
		break;
	case SCHG_LDO1_ID:      /* ldo1 */
		ret = scharger_di->power_on_config_reg01.reg.ldo1_200ma_en_int;
		break;
	case SCHG_LDO2_ID:      /* ldo2 */
		ret = scharger_di->power_on_config_reg01.reg.ldo2_200ma_en_int;
		break;
	case SCHG_LDO3_CPN1_ID: /* vsp && vsn */
		ret = (scharger_di->power_on_config_reg00.reg.lcd_ldo_en_int &&
			scharger_di->power_on_config_reg00.reg.lcd_ncp_en_int);
		break;
	default:
		break;
	}
	return ret;
}

int scharger_power_on(int id)
{
	int ret = 0;

	if (scharger_di == NULL)
		return -1;

	switch (id) {
	case SCHG_BOOST1_ID:    /* otg */
		break;
	case SCHG_BOOST2_ID:    /* wled */
		scharger_di->wled_en = CHG_POWER_EN;
		ret = hi6522_config_wled_enable(scharger_di);
		break;
	case SCHG_BOOST3_ID:    /* flash_bst */
		scharger_di->flash_bst_en = CHG_POWER_EN;
		ret = hi6522_config_flash_boost_enable(scharger_di);
		usleep_range(500, 510); /* (500, 510): range of sleep span */
		break;
	case SCHG_SOURCE1_ID:   /* flash_led_flash */
		scharger_di->flash_led_flash_en = CHG_POWER_EN;
		ret = hi6522_config_flash_led_flash_enable(scharger_di);
		break;
	case SCHG_SOURCE2_ID:   /* flash_led_torch */
		scharger_di->flash_led_torch_en = CHG_POWER_EN;
		ret = hi6522_config_flash_led_torch_enable(scharger_di);
		break;
	case SCHG_BOOST4_ID:    /* lcd_boost */
		scharger_di->lcd_bst_en = CHG_POWER_EN;
		ret = hi6522_config_lcd_boost_enable(scharger_di);
		break;
	case SCHG_LDO3_ID:      /* lcd_ldo */
		scharger_di->lcd_ldo_en = CHG_POWER_EN;
		ret = hi6522_config_lcd_ldo_enable(scharger_di);
		break;
	case SCHG_CPN1_ID:      /* lcd_ncp */
		scharger_di->lcd_ncp_en = CHG_POWER_EN;
		ret = hi6522_config_lcd_ncp_enable(scharger_di);
		break;
	case SCHG_LDO1_ID:      /* ldo1 */
		scharger_di->ldo1_200ma_en = CHG_POWER_EN;
		ret = hi6522_config_ldo1_200ma_enable(scharger_di);
		break;
	case SCHG_LDO2_ID:      /* ldo2 */
		scharger_di->ldo2_200ma_en = CHG_POWER_EN;
		ret = hi6522_config_ldo2_200ma_enable(scharger_di);
		break;
	case SCHG_LDO3_CPN1_ID: /* vsp && vsn */
		scharger_di->lcd_ldo_en = CHG_POWER_EN;
		scharger_di->lcd_ncp_en = CHG_POWER_EN;
		ret = hi6522_config_lcd_ldo_ncp_enable(scharger_di);
		break;

	default:
		break;
	}
	return ret;
}

int scharger_power_off(int id)
{
	int ret = 0;

	if (scharger_di == NULL)
		return -1;

	switch (id) {
	case SCHG_BOOST1_ID:    /* otg */
		scharger_di->otg_en = CHG_POWER_DIS;
		break;
	case SCHG_BOOST2_ID:    /* wled */
		scharger_di->wled_en = CHG_POWER_DIS;
		ret = hi6522_config_wled_enable(scharger_di);
		break;
	case SCHG_BOOST3_ID:    /* flash_bst */
		mdelay(3); /* 3: wait for 3mS for register ready */
		scharger_di->flash_bst_en = CHG_POWER_DIS;
		ret = hi6522_config_flash_boost_enable(scharger_di);
		break;
	case SCHG_SOURCE1_ID:   /* flash_led_flash */
		scharger_di->flash_led_flash_en = CHG_POWER_DIS;
		ret = hi6522_config_flash_led_flash_enable(scharger_di);
		break;
	case SCHG_SOURCE2_ID:   /* flash_led_torch */
		scharger_di->flash_led_torch_en = CHG_POWER_DIS;
		ret = hi6522_config_flash_led_torch_enable(scharger_di);
		break;
	case SCHG_BOOST4_ID:    /* lcd_boost */
		scharger_di->lcd_bst_en = CHG_POWER_DIS;
		ret = hi6522_config_lcd_boost_enable(scharger_di);
		break;
	case SCHG_LDO3_ID:      /* lcd_ldo */
		scharger_di->lcd_ldo_en = CHG_POWER_DIS;
		ret = hi6522_config_lcd_ldo_enable(scharger_di);
		break;
	case SCHG_CPN1_ID:      /* lcd_ncp */
		scharger_di->lcd_ncp_en = CHG_POWER_DIS;
		ret = hi6522_config_lcd_ncp_enable(scharger_di);
		break;
	case SCHG_LDO1_ID:      /* ldo1 */
		scharger_di->ldo1_200ma_en = CHG_POWER_DIS;
		ret = hi6522_config_ldo1_200ma_enable(scharger_di);
		break;
	case SCHG_LDO2_ID:      /* ldo2 */
		scharger_di->ldo2_200ma_en = CHG_POWER_DIS;
		ret = hi6522_config_ldo2_200ma_enable(scharger_di);
		break;
	case SCHG_LDO3_CPN1_ID: /* vsp && vsn */
		scharger_di->lcd_ncp_en = CHG_POWER_DIS;
		ret = hi6522_config_lcd_ncp_enable(scharger_di);
		mdelay(5); /* 5: wait for 5mS for register ready */
		scharger_di->lcd_ldo_en = CHG_POWER_DIS;
		ret = hi6522_config_lcd_ldo_enable(scharger_di);
		break;
	default:
		break;
	}
	return ret;
}

int scharger_power_set_voltage(unsigned int vset_regs, unsigned int mask,
			       int shift, int index)
{
	int ret0, ret1;
	struct hi6522_device_info *di = NULL;
	unsigned char reg_value = 0;
	/* return ok when charger probe later than regulator */
	if (scharger_di == NULL)
		return 0;

	di = scharger_di;
	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s get sema fail!\n", __func__);
		return -EINTR;
	}
	/* set voltage */
	ret0 = hi6522_read_byte((u8)vset_regs, &reg_value);
	chg_reg_setbits(reg_value, (u32)shift, mask, (u32)index);
	ret1 = hi6522_write_byte((u8)vset_regs, reg_value);
	up(&di->charger_data_busy_lock);
	return (ret0 || ret1);
}

int scharger_power_get_voltage_index(unsigned int vget_regs, unsigned int mask,
				     int shift)
{
	int ret;
	struct hi6522_device_info *di = NULL;
	unsigned char reg_value = 0;
	int index;
	/* return ok when charger probe later than regulator */
	if (scharger_di == NULL)
		return 0;

	di = scharger_di;
	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("scharger_power_get_current_limit get sema fail!\n");
		return -EINTR;
	}
	/* get voltage */
	ret = hi6522_read_byte((u8)vget_regs, &reg_value);
	index = chg_reg_getbits(reg_value, (u32)shift, mask);
	up(&di->charger_data_busy_lock);
	return index;
}

int scharger_power_set_current_limit(unsigned int vset_regs, unsigned int mask,
				     int shift, int index)
{
	int ret0, ret1;
	struct hi6522_device_info *di = NULL;
	unsigned char reg_value = 0;

	/* return ok when charger probe later than regulator */
	if (scharger_di == NULL)
		return 0;

	di = scharger_di;
	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s get sema fail!\n", __func__);
		return -EINTR;
	}
	/* set current */
	ret0 = hi6522_read_byte((u8)vset_regs, &reg_value);
	chg_reg_setbits(reg_value, (u32)shift, mask, (u32)index);
	ret1 = hi6522_write_byte((u8)vset_regs, reg_value);
	up(&di->charger_data_busy_lock);
	return (ret0 || ret1);
}

int scharger_power_get_current_limit_index(unsigned int vget_regs,
					   unsigned int mask, int shift)
{
	int ret;
	struct hi6522_device_info *di = NULL;
	unsigned char reg_value = 0;
	int index;

	/* return ok when charger probe later than regulator */
	if (scharger_di == NULL)
		return 0;

	di = scharger_di;
	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s get sema fail!\n", __func__);
		return -EINTR;
	}
	/* get current */
	ret = hi6522_read_byte((u8)vget_regs, &reg_value);
	index = chg_reg_getbits(reg_value, shift, mask);
	up(&di->charger_data_busy_lock);
	return index;
}

int scharger_flash_led_timeout_enable(void)
{
	if (scharger_di == NULL)
		return -1;

	scharger_di->chg_flash_led_timeout_en = 1;
	return hi6522_config_flash_led_timeout_enable(scharger_di);
}

int scharger_flash_led_timeout_disable(void)
{
	if (scharger_di == NULL)
		return -1;

	scharger_di->chg_flash_led_timeout_en = 0;
	return hi6522_config_flash_led_timeout_enable(scharger_di);
}

int scharger_flash_led_timeout_config(unsigned int timeout_ms)
{
	if (scharger_di == NULL)
		return -1;

	scharger_di->chg_flash_led_timeout = timeout_ms;

	return hi6522_config_flash_led_timeout(scharger_di);
}

int scharger_flash_torch_timeout_config(unsigned int timeout_sec)
{
	if (scharger_di == NULL)
		return -1;

	scharger_di->chg_flash_torch_timeout = timeout_sec;

	return hi6522_config_flash_torch_timeout(scharger_di);
}

int hi6522_chip_init(struct chip_init_crit *init_crit)
{
	/* 14: totally 14 terms to be stored in the ret array */
	int ret[14] = {0};
	unsigned int i;
	struct hi6522_device_info *di = g_hi6522_dev;

	if (di == NULL || init_crit == NULL) {
		scharger_err("%s hi6522_device_info or init_crit is NULL!\n", __func__);
		return -ENOMEM;
	}
	switch (init_crit->vbus) {
	case ADAPTER_5V:
		ret[0] = hi6522_set_fast_safe_timer(CHG_FAST_SAFE_TIMER_MAX);
		ret[1] = hi6522_set_term_enable(CHG_TERM_DIS);
		ret[2] = hi6522_set_input_current(IINLIM_500); /* 2: input current */
		/* 500: charge current 500mA; 4350: charge voltage 4350mV */
		ret[3] = hi6522_set_charge_current(CHG_FAST_ICHG_VALUE(500));
		ret[4] = hi6522_set_terminal_voltage(CHG_FAST_VCHG_VALUE(4350)); /* 4: vterm */
		ret[5] = hi6522_set_terminal_current(CHG_TERM_ICHG_150MA); /* 5: iterm */
		ret[6] = hi6522_set_watchdog_timer(WATCHDOG_TIMER_40_S); /* 6: whg_t */
		ret[7] = hi6522_set_precharge_current(CHG_PRG_ICHG_200MA); /* 7: ipre */
		ret[8] = hi6522_set_precharge_voltage(CHG_PRG_VCHG_2800); /* 8: vpre */
		ret[9] = hi6522_set_batfet_ctrl(CHG_BATFET_EN); /* 9: bat_ctr */
		ret[10] = hi6522_set_dpm_voltage(VINDPM_4520); /* 10: dpm */
		/* IR compensation voatge clamp, IR compensation resistor setting */
		ret[11] = hi6522_set_vclamp(di->param_dts.vclamp); /* 11: vclamp */
		ret[12] = hi6522_set_otg_current(BOOST_LIM_MAX); /* 12: iotg */
		ret[13] = hi6522_set_otg_enable(CHG_OTG_FORCE_DIS); /* 13: otg_en */
		break;
	default:
		scharger_err("%s: init mode err\n", __func__);
		return -EINVAL;
	}
	for (i = 0; i < ARRAY_SIZE(ret); i++)
		if (ret[i])
			return -1;
	return 0;
}

int hi6522_get_ibus_ma(void)
{
	int ret;
	u32 ibus;
	u32 state = 0;

	ret = hi6522_get_charge_state(&state);
	if (ret) {
		scharger_err("[%s] get_charge_state fail,ret:%d\n",
			     __func__, ret);
		return -1;
	}
	if (CHAGRE_STATE_NOT_PG & state) {
		scharger_inf("[%s] CHAGRE_STATE_NOT_PG ,state:%u\n",
			     __func__, state);
		return 0;
	}
	/*
	 * SchargerV200 do not support get-ibus function,
	 * for RunnintTest, we return a fake vulue
	 */
	ibus = 1000; /* 1000: fake current value for Running test */
	/* Temperary log for RT flag */
	scharger_inf("SchargerV200: ibus=%u, RunningTest Begin!\n", ibus);

	return ibus;
}

static int hi6522_get_vbus_mv(unsigned int *vbus_mv)
{
	int ret;
	u32 state = 0;

	ret = hi6522_get_charge_state(&state);
	if (ret) {
		scharger_err("[%s] get_charge_state fail,ret:%d\n",
			     __func__, ret);
		*vbus_mv = 0; /* fake voltage value for CTS */
		return -1;
	}
	if (CHAGRE_STATE_NOT_PG & state) {
		scharger_inf("[%s] CHAGRE_STATE_NOT_PG ,state:%u\n",
			     __func__, state);
		*vbus_mv = 0; /* fake voltage value for CTS */
		return 0;
	}
	/*
	 * SchargerV200 do not support get-vbus function,
	 * for CTS, we return a fake vulue
	 */
	*vbus_mv = 5000; /* 5000: fake voltage value for CTS */

	return ret;
}

/* set the charger ovlo_47 to close ibus */
int hi6522_set_charger_hiz(int enable)
{
	struct hi6522_device_info *di = g_hi6522_dev;
	static int first_in = 1;

	if (di == NULL) {
		scharger_err("[%s] di is NULL!\n", __func__);
		return 0;
	}

	if (di->ovlo_flag == 1) {
		scharger_err("[%s] HIZ,ovlo is not used\n", __func__);
		return 0;
	}
	if (enable == 1) {
#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
		if (di->hiz_iin_limit_open_flag && uscp_is_in_hiz_mode() &&
			!uscp_is_in_rt_protect_mode()) {
			hiz_iin_limit_flag = HIZ_IIN_FLAG_TRUE;
			if (first_in) {
				scharger_err("[%s] is_uscp_hiz_mode HIZ,enable:%d,set 100mA\n", __func__, enable);
				first_in = 0;
				/* set inputcurrent to 100mA */
				return hi6522_set_input_current(IINLIM_100);
			} else {
				return 0;
			}
		} else {
#endif
			scharger_err("[%s] HIZ,enable:%d\n", __func__, enable);
			return gpio_direction_output(di->gpio_ovlo_en, 1);
#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
		}
#endif
	} else {
		hiz_iin_limit_flag = HIZ_IIN_FLAG_FALSE;
		first_in = 1;
		scharger_err("[%s] HIZ,enable:%d\n", __func__, enable);
		return gpio_direction_output(di->gpio_ovlo_en, 0);
	}
}

/* check chip i2c communication */
static int hi6522_device_check(void)
{
	int ret;
	u8 reg_chip_ver = 0xff;

	ret = hi6522_read_byte(CHG_CHIP_VERSION_REG, &reg_chip_ver);
	if (ret) {
		scharger_err("[%s]:read reg_chip_ver fail\n", __func__);
		return CHARGE_IC_BAD;
	}

	if (reg_chip_ver == CHIP_VER_V2XXX) {
		scharger_inf("hi6522 is good\n");
		return CHARGE_IC_GOOD;
	}
	scharger_err("hi6522 is bad\n");
	return CHARGE_IC_BAD;
}

int hi6522_get_charge_state(unsigned int *state)
{
	u8 reg1 = 0;
	u8 reg2 = 0;
	u8 reg3 = 0;
	int ret0, ret1, ret2;

	if (state == NULL) {
		scharger_err("input ptr state is NULL\n");
		return -ENOMEM;
	}
	ret0 = hi6522_read_byte(CHG_REG6, &reg1);
	ret1 = hi6522_read_byte(CHG_STATUS0, &reg2);
	ret2 = hi6522_read_byte(CHG_STATUS1, &reg3);
	if (ret0 || ret1 || ret2) {
		scharger_err("[%s]read charge status reg fail\n", __func__);
		return -1;
	}

	if ((reg1 & CHG_BUCK_OK) != CHG_BUCK_OK)
		*state |= CHAGRE_STATE_NOT_PG;

	if ((reg2 & CHG_STATUS0_STAT_CHAEGE_DONE) ==
		CHG_STATUS0_STAT_CHAEGE_DONE)
		*state |= CHAGRE_STATE_CHRG_DONE;

	if ((reg2 & CHG_STATUS0_WDT_TIMER_OUT_MASK_BIT) ==
		CHG_STATUS0_WDT_TIMER_OUT)
		*state |= CHAGRE_STATE_WDT_FAULT;

	if ((reg3 & CHG_STATUS1_VBUS_OVP_6P5V) == CHG_STATUS1_VBUS_OVP_6P5V)
		*state |= CHAGRE_STATE_VBUS_OVP;

	scharger_inf("[%s],state:%u, reg1 = 0x%x, reg2 = 0x%x\n", __func__,
		     *state, reg1, reg2);
	return 0;
}

/*
 * check whether VINDPM
 * return value:    TRUE means VINDPM
 *                  FALSE means NoT DPM
 */
static int hi6522_check_input_dpm_state(void)
{
	u8 reg = 0;
	int ret;

	ret = hi6522_read_byte(CHG_STATUS0, &reg);
	if (ret < 0) {
		scharger_err("%s err\n", __func__);
		return ret;
	}

	if ((reg & CHG_STATUS0_BUCK_DPM_STAT) == CHG_STATUS0_BUCK_DPM_STAT)
		return TRUE;
	else
		return FALSE;
}

static int dpm_check_current_threshold_ma = 40;

module_param(dpm_check_current_threshold_ma, int, 0640);

static int dpm_switch_delay_time_ms = 200;
module_param(dpm_switch_delay_time_ms, int, 0640);

static int dpm_check_delay_time_ms = 1000;
module_param(dpm_check_delay_time_ms, int, 0640);

static int dpm_switch_with_charge_stop = 1;
module_param(dpm_switch_with_charge_stop, int, 0640);

static int optimize_delay_ms = 20;
module_param(optimize_delay_ms, int, 0640);
static int ico_hig_thr_count;
static int battery_buffer_first = 1;
static int battery_voltage_buffer[VBATT_AVR_MAX_COUNT] = {0};

static void mt_battery_average_method_init(unsigned int *bufferdata,
					   unsigned int data, int *sum)
{
	int i;

	for (i = 0; i < VBATT_AVR_MAX_COUNT; i++)
		bufferdata[i] = data;

	*sum = data * VBATT_AVR_MAX_COUNT;
}

static int mt_battery_average_method(unsigned int *bufferdata,
				     unsigned int voltage)
{
	int avgdata;
	static int sum, battery_index;

	if (battery_buffer_first == 1) {
		mt_battery_average_method_init(bufferdata, voltage, &sum);
		battery_buffer_first = 0;
	}
	sum -= bufferdata[battery_index];
	sum += voltage;
	bufferdata[battery_index] = voltage;
	avgdata = sum / VBATT_AVR_MAX_COUNT;
	battery_index++;
	if (battery_index >= VBATT_AVR_MAX_COUNT)
		battery_index = 0;
	return avgdata;
}

/* return value: TRUE means VINDPM, FALSE means NoT DPM */
static int hi6522_is_dpm_state_valid(void)
{
	int dpm_valid_check_count = 0;

	do {
		msleep(optimize_delay_ms);
		dpm_valid_check_count++;
		if (dpm_valid_check_count >= DPM_VALID_CHECK_COUNT)
			break;
	} while (!(hi6522_check_input_dpm_state() == TRUE));
	if (dpm_valid_check_count < DPM_VALID_CHECK_COUNT)
		return TRUE;
	else
		return FALSE;
}

/*
 * select iin current by battery current
 * Parameters:      iin_option_num:input current option,
 *                  *di:hi6522_device_info
 */
static void voltage_base_input_current_select(int iin_option_num,
					      struct hi6522_device_info *di)
{
	static int index;
	unsigned int iin_lim;
	unsigned int state = 0;
	int ret = 0;

	ret += hi6522_set_input_current(IIN_LIMIT_500MA);
	/* unlock charge current limit */
	ret += hi6522_set_charge_current(SCHARGER_ICHG_MAX);
	ret += hi6522_set_charge_enable(TRUE);
	msleep(optimize_delay_ms);
	for (index = iin_option_num; index < IIN_OPTION_NUM; index++) {
		iin_lim = iin_limit_option[index];
		ret += hi6522_set_input_current(iin_lim);
		if (hi6522_is_dpm_state_valid() == TRUE)
			break;
	}
	if (iin_option_num == 1) {
		if (di->ico_iin_lim_opt <= index)
			di->ico_iin = IIN_LIMIT_2A;
		else
			di->ico_iin = IIN_LIMIT_1A;
	} else if (iin_option_num == 0) {
		di->ico_iin = ((index == 0) ? iin_limit_option[0] : iin_limit_option[index - 1]);
	}
	ret += hi6522_get_charge_state(&state);
	if (ret < 0) {
		scharger_err("[%s]:get_charge_state fail!!\n", __func__);
		return;
	}
	if (state & CHAGRE_STATE_CHRG_DONE)
		di->ico_iin = IIN_LIMIT_2A;
}

static void standard_chk(const int *avg_vol, struct hi6522_device_info *di)
{
	/*
	 * Which is OK for standard
	 * 1 and 2 are enable states
	 */
	if (*avg_vol < DPM_CHECK_VOLTAGE) {
		if (di->ico_en > 0 && di->ico_en <= 2) {
			voltage_base_input_current_select(1, di);
			if (di->ico_iin == IIN_LIMIT_1A) {
				di->ico_en = di->ico_en + 1;
				scharger_inf("voltage base input current select again\n");
			} else {
				di->ico_en = 0; /* Run once */
			}
		}
	} else {
		ico_hig_thr_count++;
		if (ico_hig_thr_count >= ICO_HIG_THR_COUNT_MAX) {
			ico_hig_thr_count = 0;
			voltage_base_input_current_select(0, di);
			di->ico_en = 1;
		}
	}
}

/* choose the suitable input current for charger */
static int hi6522_input_current_optimize(struct ico_input *input,
					 struct ico_output *output)
{
	struct hi6522_device_info *di = g_hi6522_dev;
	int bat_voltage = coul_drv_battery_voltage();
	int bat_exist = coul_drv_is_battery_exist();
	int bat_temp = coul_drv_battery_temperature();
	int avg_voltage;
	int ret;
	unsigned int state = 0;

	if ((di == NULL) || (input == NULL) || (output == NULL))
		return -1;

	if ((!bat_exist) || (input->charger_type != CHARGER_TYPE_STANDARD))
		return -1;

	ret = hi6522_get_charge_state(&state);
	if (ret < 0) {
		scharger_err("[%s]:get_charge_state fail!!\n", __func__);
		return -1;
	}
	if (bat_temp > BAT_TEMP_50 || bat_temp < BAT_TEMP_0 ||
		(state & CHAGRE_STATE_CHRG_DONE)) {
		scharger_err("[%s]:batt_temp=%d,charge_state=%u\n",
			     __func__, bat_temp, state);
		return -1;
	}
	avg_voltage =
		mt_battery_average_method(&battery_voltage_buffer[0], bat_voltage);
	standard_chk(&avg_voltage, di);
	output->input_current = di->ico_iin;
	return 0;
}

static void hi6522_dpm_check_work(struct work_struct *work)
{
	int current_ma;
	unsigned char chg_en_status;
	int ret[3] = {0}; /* 3: total return values of functions */
	unsigned int i;
	struct hi6522_device_info *di = container_of(
		work, struct hi6522_device_info, hi6522_dpm_check_work.work);

	current_ma = coul_drv_battery_current();
	if ((current_ma > dpm_check_current_threshold_ma) ||
		(dpm_switch_enable)) {
		unsigned char reg;

		scharger_inf("%s cur=%dmA do dpm switch\n", __func__, current_ma);

		if (dpm_switch_with_charge_stop) {
			if (down_interruptible(&di->charger_data_busy_lock)) {
				scharger_err("%s get sema fail!\n", __func__);
				return;
			}
			chg_en_status = di->chg_en;
			di->chg_en = CHG_POWER_DIS;
			(void)hi6522_config_charger_nolock_enable(di);
			ret[0] = hi6522_read_byte(BUCK_REG5_REG, &reg);
			reg = reg | 0x08; /* 1: 0x08: BUCK reg mask */
			ret[1] = hi6522_write_byte(BUCK_REG5_REG, reg);
			msleep(dpm_switch_delay_time_ms);
			reg = reg & (~0x08); /* 2: 0x08: BUCK reg mask */
			ret[2] = hi6522_write_byte(BUCK_REG5_REG, reg);
			di->chg_en = chg_en_status;
			(void)hi6522_config_charger_nolock_enable(di);
			up(&di->charger_data_busy_lock);
		} else {
			ret[0] = hi6522_read_byte(BUCK_REG5_REG, &reg);
			reg = reg | 0x08; /* ret 1: 0x08: BUCK reg mask */
			ret[1] = hi6522_write_byte(BUCK_REG5_REG, reg);
			msleep(dpm_switch_delay_time_ms);
			reg = reg & (~0x08); /* ret 2: 0x08: BUCK reg mask */
			ret[2] = hi6522_write_byte(BUCK_REG5_REG, reg);
		}
	}
	for (i = 0; i < ARRAY_SIZE(ret); i++)
		if (ret[i])
			scharger_err("%s:ret[%d] fail!\n", __func__, ret[i]);

	queue_delayed_work(system_power_efficient_wq,
		&di->hi6522_dpm_check_work,
		msecs_to_jiffies(dpm_check_delay_time_ms));
}

/* Interface for modifing flash boost voltage */
int scharger_flash_bst_vo_config(int config_voltage)
{
	int ret;
	struct hi6522_device_info *di = NULL;
	u8 boost_vol;

	if (scharger_di == NULL) {
		scharger_err("hi6522 scharger_id is null!\n");
		return -1;
	}
	di = scharger_di;
	if (down_interruptible(&di->charger_data_busy_lock)) {
		scharger_err("%s get sema fail!\n", __func__);
		return -EINTR;
	}

	if (config_voltage < FLASH_BST_VOL_MIN)
		config_voltage = FLASH_BST_VOL_MIN;
	else if (config_voltage > FLASH_BST_VOL_MAX)
		config_voltage = FLASH_BST_VOL_MAX;

	boost_vol = (unsigned char)((FLASH_BST_VOL_MAX - config_voltage) /
				    FLASH_BST_VOL_STEP_100);
	ret = hi6522_write_mask(FLASH_BST_REG10_REG, FLASH_BST_VOL_MSK,
				FLASH_BST_VOL_SHIFT, boost_vol);
	up(&di->charger_data_busy_lock);
	scharger_inf("flash boost voltage reg is set 0x%x\n", boost_vol);
	return ret;
}

int hi6522_lcd_ldo_clear_int(void)
{
	u8 reg = 0;
	int ret0, ret1;
	struct hi6522_device_info *di = NULL;

	di = g_hi6522_dev;
	if (di == NULL) {
		scharger_err("%s hi6522_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}

	ret0 = hi6522_read_byte(CHG_IRQ_REG3, &reg);
	reg = reg | HI6522_LCD_LDO_OCP_CLEAR_BIT;
	ret1 = hi6522_write_byte(CHG_IRQ_REG3, reg);
	scharger_inf("clear lcd ldo ocp reg is %u\n", reg);
	return (ret0 || ret1);
}
EXPORT_SYMBOL_GPL(hi6522_lcd_ldo_clear_int);

static int hi6522_force_set_term_enable(int enable)
{
	return 0;
}

static int hi6522_get_charger_state(void)
{
	return 0;
}

static int hi6522_soft_vbatt_ovp_protect(void)
{
	return 0;
}

static int hi6522_rboost_buck_limit(void)
{
	return 0;
}

struct charge_device_ops hi6522_ops = {
	.chip_init = hi6522_chip_init,
	.set_input_current = hi6522_set_input_current,
	.set_charge_current = hi6522_set_charge_current,
	.set_terminal_voltage = hi6522_set_terminal_voltage,
	.set_dpm_voltage = hi6522_set_dpm_voltage,
	.set_terminal_current = hi6522_set_terminal_current,
	.set_charge_enable = hi6522_set_charge_enable,
	.set_otg_enable = hi6522_set_otg_enable,
	.set_term_enable = hi6522_set_term_enable,
	.get_charge_state = hi6522_get_charge_state,
	.reset_watchdog_timer = hi6522_reset_watchdog_timer,
	.set_watchdog_timer = hi6522_set_watchdog_timer,
	.set_batfet_disable = hi6522_set_batfet_disable,
	.set_otg_current = hi6522_set_otg_current,
	.get_ibus = hi6522_get_ibus_ma,
	.get_vbus = hi6522_get_vbus_mv,
	.set_charger_hiz = hi6522_set_charger_hiz,
	.dev_check = hi6522_device_check,
	.check_input_dpm_state = hi6522_check_input_dpm_state,
	.turn_on_ico = hi6522_input_current_optimize,
	.set_force_term_enable = hi6522_force_set_term_enable,
	.get_charger_state = hi6522_get_charger_state,
	.soft_vbatt_ovp_protect = hi6522_soft_vbatt_ovp_protect,
	.rboost_buck_limit = hi6522_rboost_buck_limit,
	.get_charge_current = NULL,
};

static struct power_log_ops hi6522_log_ops = {
	.dev_name = "hi6522",
	.dump_log_head = hi6522_get_register_head,
	.dump_log_content = hi6522_dump_register,
};

struct charger_otg_device_ops hi6522_otg_ops = {
	.dev_name = "hi6522",
	.otg_set_charger_enable = hi6522_set_charge_enable,
	.otg_set_enable = hi6522_set_otg_enable,
	.otg_set_current = hi6522_set_otg_current,
	.otg_set_watchdog_timer = hi6522_set_watchdog_timer,
	.otg_reset_watchdog_timer = hi6522_reset_watchdog_timer,
};

static irqreturn_t hi6522_irq_handle(int irq, void *data)
{
	struct hi6522_device_info *di = data;

	queue_work(system_power_efficient_wq, &di->irq_work);
	disable_irq_nosync(di->irq_int);
	return IRQ_HANDLED;
}

#ifdef CONFIG_HUAWEI_OCP
void scharger_wled_ocp(unsigned char irq_status)
{
	/* handle wled ocp */
	if ((irq_status & CHG_IRQ1_WLED_OVP_38V) ||
		(irq_status & CHG_IRQ1_WLED_OVP_4P5V) ||
		(irq_status & CHG_IRQ1_WLED_UVP) ||
		irq_status & CHG_IRQ1_WLED_SCP))
		hw_ocp_handler("lcd-wled");
}

void scharger_lcd_ocp(unsigned char irq_status)
{
	/* handle vsp/vsn/boost ocp */
	if (irq_status & CHG_IRQ3_LCD_LDO_OCP)
		hw_ocp_handler("lcd-ldo");
	else if (irq_status & CHG_IRQ3_LCD_BST_SCP)
		hw_ocp_handler("lcd-bst");
	else if (irq_status & CHG_IRQ3_LCD_BST_OVP)
		hw_ocp_handler("lcd-bst");
	else if (irq_status & CHG_IRQ3_LCD_BST_UVP)
		hw_ocp_handler("lcd-bst");
	else if (irq_status & CHG_IRQ3_LCD_NCP_SCP)
		hw_ocp_handler("lcd-cpn");
}
#endif

static BLOCKING_NOTIFIER_HEAD(scharger_init_notifier_list);

int scharger_register_notifier(struct notifier_block *nb)
{
	if (nb == NULL) {
		scharger_err("input ptr nb is NULL\n");
		return -ENOMEM;
	}
	return blocking_notifier_chain_register(&scharger_init_notifier_list,
						nb);
}

int scharger_unregister_notifier(struct notifier_block *nb)
{
	if (nb == NULL) {
		scharger_err("input ptr nb is NULL\n");
		return -ENOMEM;
	}
	return blocking_notifier_chain_unregister(&scharger_init_notifier_list,
						  nb);
}

static void scharger_notify_prepare(void)
{
	(void)blocking_notifier_call_chain(&scharger_init_notifier_list,
					   0, NULL);
}

static BLOCKING_NOTIFIER_HEAD(lcd_ldo_ocp_init_notifier_list);

int lcd_ldo_ocp_register_notifier(struct notifier_block *nb)
{
	if (nb == NULL) {
		scharger_err("input ptr nb is NULL\n");
		return -ENOMEM;
	}
	return blocking_notifier_chain_register(&lcd_ldo_ocp_init_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(lcd_ldo_ocp_register_notifier);

int lcd_ldo_ocp_unregister_notifier(struct notifier_block *nb)
{
	if (nb == NULL) {
		scharger_err("input ptr nb is NULL\n");
		return -ENOMEM;
	}
	return blocking_notifier_chain_unregister(
		&lcd_ldo_ocp_init_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(lcd_ldo_ocp_unregister_notifier);

static int hi6522_dpm_switch_notifier_call(struct notifier_block *usb_nb,
					   unsigned long event, void *data)
{
	struct hi6522_device_info *di =
		container_of(usb_nb, struct hi6522_device_info, usb_nb);

	switch (event) {
	case CHARGER_TYPE_SDP:
	case CHARGER_TYPE_DCP:
	case CHARGER_TYPE_CDP:
	case CHARGER_TYPE_UNKNOWN:
		di->ico_en = 1;
		battery_buffer_first = 1;
		queue_delayed_work(system_power_efficient_wq,
			&di->hi6522_dpm_check_work, msecs_to_jiffies(0));
		di->batfet_disable_flag = FALSE;
		break;
	case CHARGER_TYPE_NONE:
		di->ico_en = 0;
		di->ico_iin = IIN_LIMIT_2A;
		ico_hig_thr_count = 0;
		hiz_iin_limit_flag = HIZ_IIN_FLAG_FALSE;
		battery_buffer_first = 1;
		cancel_delayed_work_sync(&di->hi6522_dpm_check_work);
		/*
		 * In LPT, to avoid batfet is auto opened when plugout
		 * usb cable, disable it again
		 */
		if ((strstr(saved_command_line, "androidboot.swtype=factory")) &&
			(di->batfet_disable_flag == TRUE)) {
			msleep(1000); /* 1000: need 1000mS before batfet set */
			(void)hi6522_set_batfet_ctrl(!(di->batfet_disable_flag));
		}
		break;
	case PLEASE_PROVIDE_POWER:
		break;
	default:
		return NOTIFY_OK;
	}

	return NOTIFY_OK;
}

static void parse_dts(struct device_node *np, struct hi6522_device_info *di)
{
	int ret;
	struct device_node *batt_node = NULL;

	di->param_dts.vclamp = 80; /* 80: initial vc lamp value */
	di->ico_iin_lim_opt = IIN_LIMIT_OPTION_DEFAULT;

	ret = of_property_read_u32(np, "hiz_iin_limit_open_flag",
				   &(di->hiz_iin_limit_open_flag));
	if (ret) {
		scharger_err("get hiz_iin_limit_open_flag failed\n");
		di->hiz_iin_limit_open_flag = 0;
	}
	scharger_inf("prase_dts hiz_iin_limit_open_flag=%d\n",
		di->hiz_iin_limit_open_flag);
	ret = of_property_read_u32(np, "vclamp", &(di->param_dts.vclamp));
	if (ret) {
		scharger_err("get vclamp failed\n");
		return;
	}
	scharger_inf("prase_dts vclamp=%d\n", di->param_dts.vclamp);

	ret = of_property_read_u32(np, "fcp_support",
				   &(di->param_dts.fcp_support));
	if (ret) {
		scharger_err("get fcp_support failed\n");
		return;
	}

	ret = of_property_read_u32(np, "input_current_limit_option",
		&(di->ico_iin_lim_opt));
	if (ret) {
		scharger_err("get ico_iin_lim_opt failed\n");
		return;
	}
	scharger_inf("get iin limit option is %d!\n", di->ico_iin_lim_opt);
	ret = of_property_read_u32(np, "wled_ovp", &(di->wled_ovp_vol));
	if (ret) {
		scharger_err("get wled_ovp failed\n");
		di->wled_ovp_vol = VOL_DEFAULT;
	}
	scharger_inf("get wled_ovp is %d!\n", di->wled_ovp_vol);
	batt_node =
		of_find_compatible_node(NULL, NULL, "hisilicon,bci_battery");
	if (batt_node != NULL)
		of_property_read_u32(batt_node, "battery_board_type",
				     &is_board_type);
	else
		scharger_err("get board type fail\n");

	scharger_err("get board type is %u !\n", is_board_type);
}

static void irq_and_work_init(struct hi6522_device_info *di)
{
	di->ico_en = 0;
	/* set irq init */
	di->charge_irq_mask0_reg.value = 0xFF;
	di->charge_irq_mask1_reg.value = 0xFF;
	di->charge_irq_mask2_reg.value = 0xFF;
	di->charge_irq_mask3_reg.value = 0xFF;
	di->charge_irq_mask4_reg.value = 0xFF;
	di->charge_irq_mask0_reg.reg.trickle_chg_fault_rm = CHG_IRQ_EN;
	di->charge_irq_mask0_reg.reg.pre_chg_fault_rm = CHG_IRQ_EN;
	di->charge_irq_mask0_reg.reg.fast_chg_fault_rm = CHG_IRQ_EN;
	di->charge_irq_mask1_reg.reg.wled_scp_rm = CHG_IRQ_EN;
	di->charge_irq_mask1_reg.reg.buck_scp_rm = CHG_IRQ_EN;
	di->charge_irq_mask1_reg.reg.wled_uvp_rm = CHG_IRQ_EN;
	di->charge_irq_mask1_reg.reg.wled_ovp_4p5v_rm = CHG_IRQ_EN;
	di->charge_irq_mask1_reg.reg.wled_ovp_38v_rm = CHG_IRQ_EN;
	di->charge_irq_mask2_reg.reg.flash_bst_uvp_rm = CHG_IRQ_EN;
	di->charge_irq_mask2_reg.reg.flash_bst_ovp_rm = CHG_IRQ_EN;
	di->charge_irq_mask2_reg.reg.flash_bst_scp_rm = CHG_IRQ_EN;
	di->charge_irq_mask2_reg.reg.flash_led_short_rm = CHG_IRQ_EN;
	di->charge_irq_mask2_reg.reg.flash_led_open_rm = CHG_IRQ_EN;
	di->charge_irq_mask3_reg.reg.lcd_ncp_scp_rm = CHG_IRQ_EN;
	di->charge_irq_mask3_reg.reg.lcd_bst_uvp_rm = CHG_IRQ_EN;
	di->charge_irq_mask3_reg.reg.lcd_bst_ovp_rm = CHG_IRQ_EN;
	di->charge_irq_mask3_reg.reg.lcd_bst_scp_rm = CHG_IRQ_EN;
	di->charge_irq_mask3_reg.reg.lcd_ldo_ocp_rm = CHG_IRQ_EN;
	di->charge_irq_mask3_reg.reg.ldo1_200ma_ocp_rm = CHG_IRQ_EN;
	di->charge_irq_mask3_reg.reg.ldo2_200ma_ocp_rm = CHG_IRQ_EN;
	di->charge_irq_mask4_reg.reg.otg_uvp_rm = CHG_IRQ_EN;
	di->charge_irq_mask4_reg.reg.otg_scp_rm = CHG_IRQ_EN;

	hi6522_config_opt_param();

	/* interrput init */
	(void)hi6522_write_byte(CHG_IRQ_MASK_REG0,
			di->charge_irq_mask0_reg.value);
	(void)hi6522_write_byte(CHG_IRQ_MASK_REG1,
			di->charge_irq_mask1_reg.value);
	(void)hi6522_write_byte(CHG_IRQ_MASK_REG2,
			di->charge_irq_mask2_reg.value);
	(void)hi6522_write_byte(CHG_IRQ_MASK_REG3,
			di->charge_irq_mask3_reg.value);
	(void)hi6522_write_byte(CHG_IRQ_MASK_REG4,
			di->charge_irq_mask4_reg.value);

	INIT_WORK(&di->irq_work, hi6522_irq_work_handle);
	INIT_DELAYED_WORK(&di->hi6522_dpm_check_work,
				hi6522_dpm_check_work);
	di->usb_nb.notifier_call = hi6522_dpm_switch_notifier_call;
}

static void gpio_ovlo_en_validate(struct hi6522_device_info *di)
{
	int ret;

	if (!!gpio_is_valid(di->gpio_ovlo_en)) {
		ret = gpio_request(di->gpio_ovlo_en, "ovlo_en");
		if (ret < 0) {
			scharger_err("%s failed %d\n", __func__, __LINE__);
			di->ovlo_flag = 1;
		}
	} else {
		scharger_err("%s failed %d\n", __func__, __LINE__);
		di->ovlo_flag = 1;
	}
}

static int hi6522_charger_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct hi6522_device_info *di = NULL;
	int ret;
	u8 read_reg = 0;
	struct device_node *np = NULL;
	enum hisi_charger_type type = chip_get_charger_type();

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (di == NULL) {
		scharger_err("hi6522_device_info is NULL!\n");
		return -ENOMEM;
	}
	g_hi6522_dev = di;
	scharger_di = di;
	sema_init(&di->charger_data_busy_lock, 1);
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	di->wled_ovp_vol = VOL_DEFAULT; /* init wled-ovp set val */
	i2c_set_clientdata(client, di);
	parse_dts(np, di);
	ret = hi6522_read_byte(SOC_SCHARGER_CHIP_VERSION_ADDR(0), &read_reg);
	if (ret < 0) {
		scharger_err("chip not present at address %x\n", client->addr);
		ret = -EINVAL;
		goto err_kfree;
	}
	irq_and_work_init(di);
	ret = chip_charger_type_notifier_register(&di->usb_nb);
	if (ret < 0) {
		scharger_err("chip_charger_type_notifier_register failed\n");
		goto err_io;
	}
	/* init batfet on-off flag */
	di->batfet_disable_flag = FALSE;

	di->gpio_int = of_get_named_gpio(np, "gpio_int", 0);
	if (!gpio_is_valid(di->gpio_int)) {
		scharger_err("%s failed %d\n", __func__, __LINE__);
		ret = -EFAULT;
		goto err_irq_gpio;
	}
	di->irq_int = gpio_to_irq(di->gpio_int);
	ret = gpio_request_one(di->gpio_int, GPIOF_IN, "Scharger_int");
	if (ret) {
		scharger_err("no card-detect pin available!\n");
		ret = -ENOMEM;
		goto err_irq_gpio;
	}
	ret = request_irq(di->irq_int, hi6522_irq_handle,
			  IRQF_TRIGGER_LOW | IRQF_NO_SUSPEND,
			  di->client->name, di);
	if (ret) {
		scharger_err("request Scharger irq error\n");
		goto err_irq_request;
	}
	di->ovlo_flag = 0;
	di->gpio_ovlo_en = of_get_named_gpio(np, "gpio_ovlo_en", 0);
	gpio_ovlo_en_validate(di);

	ret = charge_ops_register(&hi6522_ops);
	if (ret) {
		scharger_err("register charge ops failed!\n");
		goto err_sysfs;
	}

	ret = charger_otg_ops_register(&hi6522_otg_ops);
	if (ret) {
		scharger_err("register charger_otg ops failed\n");
		goto err_sysfs;
	}

	/*
	 * scharger v200 need to do dpm check when usb/charger is plugged on
	 * startup
	 */
	if (type < CHARGER_TYPE_NONE) {
		di->ico_en = 1;
		di->ico_iin = IIN_LIMIT_2A;
		queue_delayed_work(system_power_efficient_wq,
			&di->hi6522_dpm_check_work, msecs_to_jiffies(0));
	}
	ret = hi6522_sysfs_create_group(di);
	if (ret) {
		scharger_err("create sysfs entries failed!\n");
		goto err_create_group;
	}

	scharger_notify_prepare(); /* notify backlight open */
	scharger_inf("HI6522 probe ok!\n");
	hi6522_log_ops.dev_data = (void *)di;
	power_log_ops_register(&hi6522_log_ops);
	return 0;

err_create_group:
	hi6522_sysfs_remove_group(di);
err_sysfs:
	free_irq(di->irq_int, di);
	if (!di->ovlo_flag)
		gpio_free(di->gpio_ovlo_en);

err_irq_request:
	gpio_free(di->gpio_int);
err_irq_gpio:
err_io:
err_kfree:
	g_hi6522_dev = NULL;
	scharger_di = NULL;
	return ret;
}

static int hi6522_charger_remove(struct i2c_client *client)
{
	struct hi6522_device_info *di = i2c_get_clientdata(client);

	if (di == NULL) {
		scharger_err("%s:di is NULL\n", __func__);
		return 0;
	}
	chip_charger_type_notifier_unregister(&di->usb_nb);
	hi6522_sysfs_remove_group(di);
	cancel_delayed_work(&di->hi6522_dpm_check_work);
	free_irq(di->irq_int, di);
	gpio_free(di->gpio_int);
	if (!di->ovlo_flag)
		gpio_free(di->gpio_ovlo_en);

	return 0;
}

int hi6522_show_power_status(void)
{
	scharger_err("power_status:\n chg_en:%u\n otg_en:%u\n wled_en:%u\n ldo1_en:%u\n ldo2_en:%u\n",
		     g_hi6522_dev->chg_en, g_hi6522_dev->otg_en,
		     g_hi6522_dev->wled_en, g_hi6522_dev->ldo1_200ma_en,
		     g_hi6522_dev->ldo2_200ma_en);
	return 0;
}

static void hi6522_charger_shutdown(struct i2c_client *client)
{
	struct hi6522_device_info *di = i2c_get_clientdata(client);

	if (di == NULL) {
		scharger_err("%s:di is NULL\n", __func__);
		return;
	}
	(void)chip_charger_type_notifier_unregister(&di->usb_nb);
	(void)cancel_delayed_work(&di->hi6522_dpm_check_work);
}

static const struct i2c_device_id hi6522_id[] = {
	{ "hi6522_charger", 0 },
	{},
};

static int hi6522_charger_suspend(struct device *dev, pm_message_t state)
{
	struct hi6522_device_info *di = dev_get_drvdata(dev);
	int ret;

	if (di == NULL) {
		scharger_err("%s:di is NULL\n", __func__);
		return 0;
	}
	scharger_inf("%s:+\n", __func__);
	ret = hi6522_config_power_on_reg01(di);
	if (ret)
		scharger_err("%s:fail to config onff1\n", __func__);
	scharger_inf("%s:-\n", __func__);
	return 0;
}

static int hi6522_charger_resume(struct device *dev)
{
	struct hi6522_device_info *di = dev_get_drvdata(dev);
	int ret;

	if (di == NULL) {
		scharger_err("%s:di is NULL\n", __func__);
		return 0;
	}
	scharger_inf("%s:+\n", __func__);
	ret = hi6522_config_power_on_reg01(di);
	if (ret)
		scharger_err("%s:fail to config onff1\n", __func__);
	scharger_inf("%s:-\n", __func__);
	return 0;
}

MODULE_DEVICE_TABLE(i2c, hi6522);
const static struct of_device_id hi6522_charger_match_table[] = {
	{
		.compatible = "huawei,hi6522_charger",
		.data = NULL,
	}, {
	},
};

static const struct i2c_device_id hi6522_i2c_id[] = {
	{ "hi6522_charger", 0 },
	{ }
};

static struct i2c_driver hi6522_charger_driver = {
	.probe = hi6522_charger_probe,
	.remove = hi6522_charger_remove,
	.shutdown = hi6522_charger_shutdown,
	.id_table = hi6522_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "hi6522_charger",
		.suspend = hi6522_charger_suspend,
		.resume = hi6522_charger_resume,
		.of_match_table = of_match_ptr(hi6522_charger_match_table),
	},
};

static int __init hi6522_charger_init(void)
{
	int ret;

	ret = i2c_add_driver(&hi6522_charger_driver);
	if (ret)
		scharger_err("%s: i2c_add_driver error!!!\n", __func__);

	return ret;
}

module_init(hi6522_charger_init);

static void __exit hi6522_charger_exit(void)
{
	(void)i2c_del_driver(&hi6522_charger_driver);
}

module_exit(hi6522_charger_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HI6522 charger driver");
MODULE_AUTHOR("Wen Qi <qiwen4@hisilicon.com>");
