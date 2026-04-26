/*
 * da9313.c
 *
 * vsys sc da9313 driver
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
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/raid/pq.h>
#include <linux/pm_wakeup.h>
#include <huawei_platform/log/hw_log.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <huawei_platform/power/vsys_switch/vsys_switch.h>
#include "da9313.h"

#ifdef CONFIG_BCI_BATTERY
#include <linux/power/hisi/bci_battery.h>
#endif
#include <chipset_common/hwpower/common_module/power_i2c.h>
#ifdef CONFIG_HUAWEI_POWER_DEBUG
#include <chipset_common/hwpower/common_module/power_debug.h>
#endif

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG da9313
HWLOG_REGIST();

#define DELAY_TIME_FOR_AUTO_FREQUENCY_WORK 5

struct da9313_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct delayed_work frequency_work;
	struct wakeup_source wakelock;
	struct wakeup_source frequency_wakelock;
	int chip_already_init;
	int get_id_time;
	int device_id;
	int gpio_nonkey_en;
	int gpio_pwron;
	int gpio_int;
	int irq_int;
	int irq_active;
	int state;
	int support_pwrdown_mode;
	int single_sc_channel;
	int curr_frequency_mode;
	bool is_pg;
	struct mutex set_state_lock;
};

static struct da9313_device_info *g_da9313_di;

static int da9313_write_byte(u8 reg, u8 value)
{
	struct da9313_device_info *di = g_da9313_di;
	int i;

	if (!di || di->chip_already_init == 0) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!power_i2c_u8_write_byte(di->client, reg, value))
			return 0;
	}

	return -EIO;
}

static int da9313_read_byte(u8 reg, u8 *value)
{
	struct da9313_device_info *di = g_da9313_di;
	int i;

	if (!di || di->chip_already_init == 0) {
		hwlog_err("chip not init\n");
		return -EIO;
	}

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!power_i2c_u8_read_byte(di->client, reg, value))
			return 0;
	}

	return -EIO;
}

static int da9313_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = da9313_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	return da9313_write_byte(reg, val);
}

#ifdef POWER_MODULE_DEBUG_FUNCTION
static int da9313_read_mask(u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret;
	u8 val = 0;

	ret = da9313_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= mask;
	val >>= shift;
	*value = val;

	return 0;
}
#endif /* POWER_MODULE_DEBUG_FUNCTION */

static void da9313_wake_lock(void)
{
	struct da9313_device_info *di = g_da9313_di;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	if (!di->wakelock.active) {
		__pm_stay_awake(&di->wakelock);
		hwlog_info("wake_lock\n");
	}
}

static void da9313_wake_unlock(void)
{
	struct da9313_device_info *di = g_da9313_di;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	if (di->wakelock.active) {
		__pm_relax(&di->wakelock);
		hwlog_info("wake_unlock\n");
	}
}

#ifdef POWER_MODULE_DEBUG_FUNCTION
static void da9313_dump_register(void)
{
	u8 i;
	int ret;
	u8 val = 0;

	for (i = 0; i < DA9313_REG_TOTAL_NUM; i++) {
		ret = da9313_read_byte(i, &val);
		if (ret)
			hwlog_err("dump_register: read fail\n");

		hwlog_info("dump_register: reg[%x] = 0x%x\n", i, val);
	}
}
#endif /* POWER_MODULE_DEBUG_FUNCTION */

static int da9313_reset_cycle(void)
{
	int ret;
	u8 val = 0;

	ret = da9313_write_mask(DA9313_REG_MODECTRL,
		DA9313_REG_RESET_CYCLE_MASK,
		DA9313_REG_RESET_CYCLE_SHIFT,
		DA9313_RESET_CYCLE_ACTIVE);
	if (ret)
		return -1;

	ret = da9313_read_byte(DA9313_REG_MODECTRL, &val);
	if (ret)
		return -1;

	hwlog_info("reset_cycle reg[0x%x] = 0x%x\n", DA9313_REG_MODECTRL, val);

	return 0;
}

static int da9313_soft_reset(void)
{
	int ret;
	u8 val = 0;

	ret = da9313_write_mask(DA9313_REG_MODECTRL,
		DA9313_REG_SOFTRESET_MASK,
		DA9313_REG_SOFTRESET_SHIFT,
		DA9313_SOFTRESET_ASSERTED);
	if (ret)
		return -1;

	ret = da9313_read_byte(DA9313_REG_MODECTRL, &val);
	if (ret)
		return -1;

	hwlog_info("soft_reset reg[0x%x] = 0x%x\n", DA9313_REG_MODECTRL, val);

	return 0;
}

static int da9313_set_test_regs(void)
{
	int ret;

	ret = da9313_write_byte(DA9313_REG_CTRL_ACCESS_TEST_REGS,
		DA9313_EN_ACCESS_TEST_REGS);
	if (ret)
		return -1;

	ret = da9313_write_byte(DA9313_REG_CTRL_PVC_SWITCHING,
		DA9313_STOP_PVC_SWITCHING);
	if (ret)
		return -1;

	ret = da9313_write_byte(DA9313_REG_CTRL_ACCESS_TEST_REGS,
		DA9313_DIS_ACCESS_TEST_REGS);
	if (ret)
		return -1;

	return ret;
}

static int da9313_get_device_id(int *id)
{
	u8 part_info = 0;
	int ret;
	struct da9313_device_info *di = g_da9313_di;

	if (!di || !id) {
		hwlog_err("di or id is null\n");
		return -1;
	}

	if (di->get_id_time == DA9313_USED) {
		*id = di->device_id;
		return 0;
	}

	di->get_id_time = DA9313_USED;
	ret = da9313_read_byte(DA9313_REG_DEVICEID, &part_info);
	if (ret) {
		di->get_id_time = DA9313_NOT_USED;
		return -1;
	}

	hwlog_info("get_device_id [%x]=0x%x\n", DA9313_REG_DEVICEID, part_info);

	if (part_info == DA9313_DEVICEID_INFO) {
		di->device_id = VSYS_SC_DA9313;
	} else {
		di->device_id = -1;
		ret = -1;
		hwlog_err("get device id ERR\n");
	}

	*id = di->device_id;
	return ret;
}

static int da9313_get_state(void)
{
	struct da9313_device_info *di = g_da9313_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	return di->state;
}

static int da9313_get_pg_state(void)
{
	int val;
	struct da9313_device_info *di = g_da9313_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (!di->gpio_int) {
		hwlog_err("no device\n");
		return -ENODEV;
	}

	val = gpio_get_value(di->gpio_int);
	hwlog_info("get_pg_state: gpio_int val = %d\n", val);

	return val;
}


/*
 * While the PVC is in CP_FSW_CLK_EXT state,
 * all Critical Faults(UVLO,OC and OT) will be disabled.
 *
 * If the PG is deasserted (due to an UVLO condition) while PVC_MODE = Auto,
 * the CP_FSW_CLK_EXT state should be exited and the PVC reverted to "normal".
 * Automatic mode of operation with all Critical Faults enabled.
 */
static void da9313_set_normal_frequency_mode(struct da9313_device_info *di)
{
	int ret;
	int tmp;

	hwlog_info("current frequency mode %d\n", di->curr_frequency_mode);

	mutex_lock(&di->set_state_lock);
	tmp = di->chip_already_init;
	di->chip_already_init = 1;

	if (di->curr_frequency_mode != DA9313_AUTO_FREQUENCY_MODE) {
		hwlog_info("frequency mode %d, no switch\n", di->curr_frequency_mode);
		goto exit;
	}

	ret = da9313_write_byte(DA9313_REG_CTRL_ACCESS_TEST_REGS,
		DA9313_EN_ACCESS_TEST_REGS);
	if (ret) {
		hwlog_err("enable test regs fail\n");
		goto exit;
	}
	da9313_write_byte(DA9313_REG_FORCE_CP_FSW_CLK_EXT,
		DA9313_CP_FSW_CLK_EXT_RELEASE);
	da9313_write_byte(DA9313_REG_OSCILLATORS,
		DA9313_FORCE_OSCILLATOR_OFF);
	da9313_write_byte(DA9313_REG_CRITICAL_FAULTS,
		DA9313_CRITICAL_FAULTS_ENABLE);
	da9313_write_byte(DA9313_REG_CTRL_ACCESS_TEST_REGS,
		DA9313_DIS_ACCESS_TEST_REGS);

exit:
	di->chip_already_init = tmp;
	mutex_unlock(&di->set_state_lock);
}

static void da9313_set_auto_frequency_mode(struct da9313_device_info *di)
{
	int ret;

	ret = da9313_write_byte(DA9313_REG_CTRL_ACCESS_TEST_REGS,
		DA9313_EN_ACCESS_TEST_REGS);
	if (ret) {
		hwlog_err("enable test regs fail\n");
		return;
	}
	da9313_write_byte(DA9313_REG_CRITICAL_FAULTS,
		DA9313_CRITICAL_FAULTS_DISABLE);
	da9313_write_byte(DA9313_REG_OSCILLATORS,
		DA9313_FORCE_OSCILLATOR_ON);
	da9313_write_byte(DA9313_REG_CP_FSW_CLK_EXT,
		DA9313_CP_FSW_CLK_EXT_SELECT);
	da9313_write_byte(DA9313_REG_FORCE_CP_FSW_CLK_EXT,
		DA9313_CP_FSW_CLK_EXT_FORCE);
	da9313_write_mask(DA9313_REG_PVCCTRL, DA9313_REG_PVC_MODE_MASK,
		DA9313_REG_PVC_MODE_SHIFT, DA9313_AUTO_FREQUENCY_MODE);
	da9313_write_byte(DA9313_REG_CTRL_ACCESS_TEST_REGS,
		DA9313_DIS_ACCESS_TEST_REGS);

	di->curr_frequency_mode = DA9313_AUTO_FREQUENCY_MODE;
}

static void da9313_set_fix_frequency_mode(struct da9313_device_info *di)
{
	int ret;

	ret = da9313_write_byte(DA9313_REG_CTRL_ACCESS_TEST_REGS,
		DA9313_EN_ACCESS_TEST_REGS);
	if (ret) {
		hwlog_err("enable test regs fail\n");
		return;
	}
	da9313_write_mask(DA9313_REG_PVCCTRL, DA9313_REG_PVC_MODE_MASK,
		DA9313_REG_PVC_MODE_SHIFT, DA9313_FIX_FREQUENCY_MODE);
	da9313_write_byte(DA9313_REG_FORCE_CP_FSW_CLK_EXT,
		DA9313_CP_FSW_CLK_EXT_RELEASE);
	da9313_write_byte(DA9313_REG_OSCILLATORS,
		DA9313_FORCE_OSCILLATOR_OFF);
	da9313_write_byte(DA9313_REG_CRITICAL_FAULTS,
		DA9313_CRITICAL_FAULTS_ENABLE);
	da9313_write_byte(DA9313_REG_CTRL_ACCESS_TEST_REGS,
		DA9313_DIS_ACCESS_TEST_REGS);

	di->curr_frequency_mode = DA9313_FIX_FREQUENCY_MODE;
}

/*
 * When the auto frequency mode is quickly switched to the fixed mode,
 * the da9313 may not work and the phone will not turn on.
 * Therefore, when the current is in the fixed frequency mode,
 * the switch to the auto frequency mode requires a delay of 5ms,
 * which only takes effect for diversified products.
 * It is distinguished by single_sc_channel.
 */
static int da9313_set_frequency_mode(u8 mode)
{
	int ret = 0;
	int tmp;
	struct da9313_device_info *di = g_da9313_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (mode > 1) {
		hwlog_err("input mode is invalid\n");
		return -1;
	}

	mutex_lock(&di->set_state_lock);
	tmp = di->chip_already_init;
	di->chip_already_init = 1;

	if (di->single_sc_channel) {
		cancel_delayed_work(&di->frequency_work);
		if (di->frequency_wakelock.active)
			__pm_relax(&di->frequency_wakelock);

		if (mode == DA9313_AUTO_FREQUENCY_MODE) {
			__pm_wakeup_event(&di->frequency_wakelock,
				jiffies_to_msecs(HZ));
			schedule_delayed_work(&di->frequency_work,
				msecs_to_jiffies(DELAY_TIME_FOR_AUTO_FREQUENCY_WORK));
		} else {
			da9313_set_fix_frequency_mode(di);
		}
	} else {
		/* 0: fixed frequency mode; 1: auto frequency mode */
		ret = da9313_write_mask(DA9313_REG_PVCCTRL,
			DA9313_REG_PVC_MODE_MASK,
			DA9313_REG_PVC_MODE_SHIFT,
			mode);
	}

	di->chip_already_init = tmp;
	mutex_unlock(&di->set_state_lock);
	hwlog_info("channel state %d, set frequency mode %d\n", di->single_sc_channel, mode);
	return ret;
}

static int da9313_set_state(int enable)
{
	int ret = 0;
	struct da9313_device_info *di = g_da9313_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (!di->gpio_pwron || (!di->gpio_nonkey_en &&
		di->support_pwrdown_mode)) {
		hwlog_err("set_state: no device\n");
		return -ENODEV;
	}

	enable = !!enable;

	if (di->state == enable) {
		hwlog_info("set_state already %d\n", enable);
		return 0;
	}

	di->state = enable;

	if (enable && di->support_pwrdown_mode) {
		gpio_set_value(di->gpio_nonkey_en, enable);
		hwlog_info("set_state set gpio_nonkey_en %d\n", enable);

		/* high level lasts 20ms, from power down to active mode */
		usleep_range(20000, 21000);

		gpio_set_value(di->gpio_nonkey_en, !enable);
		hwlog_info("set_state set gpio_nonkey_en %d\n", !enable);

		/* wait for 20ms */
		usleep_range(20000, 21000);
	}

	mutex_lock(&di->set_state_lock);

	if (enable) {
		di->chip_already_init = 1;
		ret = da9313_set_test_regs();
		if (ret) {
			mutex_unlock(&di->set_state_lock);
			return VSYS_SC_SET_TEST_REGS_FAIL;
		}

		/* wait for 20ms */
		usleep_range(20000, 21000);
	}

	gpio_set_value(di->gpio_pwron, enable);
	hwlog_info("set_state set gpio_pwron %d\n", enable);

	if (!enable)
		di->is_pg = false;

	if (!enable && di->support_pwrdown_mode) {
		/* wait for 10ms */
		usleep_range(10000, 11000);

		ret = da9313_reset_cycle();
		ret |= da9313_soft_reset();
		di->chip_already_init = 0;
		if (ret) {
			mutex_unlock(&di->set_state_lock);
			return VSYS_SC_SET_MODE_CTRL_REG_FAIL;
		}
	}

	mutex_unlock(&di->set_state_lock);

	if (enable) {
		/* wait for 100ms to read power good signal */
		usleep_range(100000, 101000);

		if (da9313_get_pg_state() == DA9313_PG_ABNOR) {
			hwlog_err("set_state: sc is not power good\n");
			return VSYS_SC_NOT_PG;
		}
		di->is_pg = true;
	}

	return ret;
}

static void da9313_frequency_work(struct work_struct *work)
{
	struct da9313_device_info *di = g_da9313_di;
	int tmp;

	if (!di)
		return;

	mutex_lock(&di->set_state_lock);
	tmp = di->chip_already_init;
	di->chip_already_init = 1;

	da9313_set_auto_frequency_mode(di);

	di->chip_already_init = tmp;
	mutex_unlock(&di->set_state_lock);

	hwlog_info("set auto frequency work end\n");
}

static void da9313_irq_work(struct work_struct *work)
{
	struct da9313_device_info *di = NULL;

	if (!work) {
		hwlog_err("work is null\n");
		da9313_wake_unlock();
		return;
	}

	di = container_of(work, struct da9313_device_info, irq_work);
	if (!di) {
		hwlog_err("di is null\n");
		da9313_wake_unlock();
		return;
	}

	if (!di->single_sc_channel)
		atomic_notifier_call_chain(&vsys_sc_event_nh,
			VSYS_SC_INT_NOT_PG, NULL);

	if (di->single_sc_channel)
		da9313_set_normal_frequency_mode(di);

	if (di->irq_active == 0) {
		di->irq_active = 1;
		enable_irq(di->irq_int);
	}

	da9313_wake_unlock();
}

static irqreturn_t da9313_interrupt(int irq, void *_di)
{
	struct da9313_device_info *di = _di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	da9313_wake_lock();

	hwlog_info("da9313 int happened\n");

	if (!di->is_pg || !di->chip_already_init ||
		di->state == DA9313_CHIP_DISABLE) {
		hwlog_err("ignore interrupt\n");
		da9313_wake_unlock();
		return IRQ_HANDLED;
	}

	if (di->irq_active == 1) {
		di->irq_active = 0;
		disable_irq_nosync(di->irq_int);
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("irq is not enable, do nothing\n");
		da9313_wake_unlock();
	}

	return IRQ_HANDLED;
}

static int da9313_reg_init(struct da9313_device_info *di)
{
	int ret = 0;

	if (di->support_pwrdown_mode) {
		di->chip_already_init = 1;
		ret = da9313_reset_cycle();
		ret |= da9313_soft_reset();
		if (ret)
			hwlog_err("reg init failed\n");
		di->chip_already_init = 0;
	}

	return ret;
}

static int da9313_chip_init(void)
{
	struct da9313_device_info *di = g_da9313_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	return da9313_reg_init(di);
}

static void da9313_set_gpio_ctrl(u8 mode)
{
	int ret;

	ret = da9313_write_mask(DA9313_REG_GPIOCTRL,
		DA9313_REG_GPIO0_PIN_MASK,
		DA9313_REG_GPIO0_PIN_SHIFT,
		mode);
	hwlog_info("set gpio gpi mode %u, ret %d\n", mode, ret);
}

static void da9313_set_uv_ctrl(void)
{
	int ret;

	ret = da9313_write_byte(DA9313_REG_LOCK, DA9313_REG_VENDORID_ENABLE);
	if (ret) {
		hwlog_err("enable reg lock fail\n");
		return;
	}
	da9313_write_byte(DA9313_REG_CONFIGA, DA9313_REG_CONFIGA_FIX);
	da9313_write_byte(DA9313_REG_LOCK, DA9313_REG_VENDORID_DISABLE);
}

static void da9313_set_rampup_ctrl(void)
{
	int ret;

	ret = da9313_write_byte(DA9313_REG_CTRL_ACCESS_TEST_REGS,
		DA9313_EN_ACCESS_TEST_REGS);
	if (ret) {
		hwlog_err("enable test regs fail\n");
		return;
	}
	da9313_write_byte(DA9313_REG_RAMPUP_TIMER,
		DA9313_REG_RAMPUP_TIMER_DISABLE);
	da9313_write_byte(DA9313_REG_CTRL_ACCESS_TEST_REGS,
		DA9313_DIS_ACCESS_TEST_REGS);
}

static int da9313_gpio_init(struct da9313_device_info *di,
	struct device_node *np)
{
	int ret;

	if (di->support_pwrdown_mode) {
		/* 0: power down mode */
		ret = power_gpio_config_output(np,
			"gpio_nonkey_en", "gpio_nonkey_en",
			&di->gpio_nonkey_en, 0);
		if (ret)
			goto gpio_init_fail_0;
	}

	ret = power_gpio_config_output(np,
		"gpio_pwron", "gpio_pwron",
		&di->gpio_pwron, DA9313_CHIP_DISABLE);
	if (ret)
		goto gpio_init_fail_1;

	return 0;

gpio_init_fail_1:
	if (di->support_pwrdown_mode)
		gpio_free(di->gpio_nonkey_en);
gpio_init_fail_0:
	return ret;
}

static int da9313_irq_init(struct da9313_device_info *di,
	struct device_node *np)
{
	int ret;

	ret = power_gpio_config_interrupt(np,
		"gpio_irq", "gpio_irq", &di->gpio_int, &di->irq_int);
	if (ret)
		goto irq_init_fail_0;

	ret = request_irq(di->irq_int, da9313_interrupt, IRQF_TRIGGER_FALLING,
		"da9313_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		goto irq_init_fail_1;
	}

	enable_irq_wake(di->irq_int);
	di->irq_active = 1;
	INIT_WORK(&di->irq_work, da9313_irq_work);
	return 0;

irq_init_fail_1:
	gpio_free(di->gpio_int);
irq_init_fail_0:
	return ret;
}

static void da9313_para_init(struct da9313_device_info *di)
{
	di->chip_already_init = 0;
	di->state = DA9313_CHIP_DISABLE;
	di->get_id_time = DA9313_NOT_USED;
	di->is_pg = false;
	di->curr_frequency_mode = DA9313_AUTO_FREQUENCY_MODE;
}

static void da9313_board_init(struct da9313_device_info *di)
{
	/* open da9313 interrupt work state */
	di->chip_already_init = 1;
	di->is_pg = true;
	di->state = DA9313_CHIP_ENABLE;

	mutex_lock(&di->set_state_lock);

	da9313_set_auto_frequency_mode(di);
	da9313_set_gpio_ctrl(DA9313_GPIO0_PIN_GPI);
	da9313_set_uv_ctrl();
	da9313_set_rampup_ctrl();

	mutex_unlock(&di->set_state_lock);
}


static struct vsys_sc_device_ops da9313_ops = {
	.chip_name = "da9313",
	.get_state = da9313_get_state,
	.set_state = da9313_set_state,
	.get_id = da9313_get_device_id,
	.set_frequency_mode = da9313_set_frequency_mode,
	.chip_init = da9313_chip_init,
};

#ifdef CONFIG_HUAWEI_POWER_DEBUG
static ssize_t da9313_dbg_show_reg_value(void *dev_data,
	char *buf, size_t size)
{
	u8 val;
	int ret;
	int i;
	char rd_buf[DA9313_RD_BUF_SIZE] = {0};
	struct da9313_device_info *dev_p = dev_data;

	if (!buf || !dev_p) {
		hwlog_err("buf or dev_p is null\n");
		return scnprintf(buf, size, "buf or dev_p is null\n");
	}

	for (i = 0; i < DA9313_REG_TOTAL_NUM; i++) {
		val = 0;
		memset(rd_buf, 0, DA9313_RD_BUF_SIZE);
		ret = da9313_read_byte(i, &val);
		if (ret == 0)
			scnprintf(rd_buf, DA9313_RD_BUF_SIZE,
				"regaddr: 0x%x regval: 0x%x\n", i, val);
		else
			scnprintf(rd_buf, DA9313_RD_BUF_SIZE,
				"regaddr: 0x%x regval: invalid\n", i);

		strncat(buf, rd_buf, strlen(rd_buf));
	}

	return strlen(buf);
}

static ssize_t da9313_dbg_store_reg_value(void *dev_data,
	const char *buf, size_t size)
{
	int regaddr = 0;
	int regval = 0;
	int ret;
	struct da9313_device_info *dev_p = dev_data;

	if (!buf || !dev_p) {
		hwlog_err("buf or dev_p is null\n");
		return -EINVAL;
	}

	if (sscanf(buf, "%d %d", &regaddr, &regval) != 2) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	/* maximum value of 8-bit num is 255 */
	if (regaddr < 0 || regaddr >= DA9313_REG_TOTAL_NUM ||
		regval < 0 || regval > 255) {
		hwlog_err("regaddr 0x%x or regval 0x%x invalid\n",
			regaddr, regval);
		return -EINVAL;
	}

	ret = da9313_write_byte((u8)regaddr, (u8)regval);
	if (ret)
		return -EINVAL;

	hwlog_info("regaddr 0x%x regval 0x%x succeed\n", regaddr, regval);

	return size;
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static int da9313_parse_dts(struct device_node *np,
	struct da9313_device_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"single_sc_channel", &di->single_sc_channel, 0);

	return power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_pwrdown_mode", &di->support_pwrdown_mode, 1);
}

static int da9313_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct da9313_device_info *di = NULL;
	struct device_node *np = NULL;

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_da9313_di = di;
	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	da9313_parse_dts(np, di);

	ret = da9313_gpio_init(di, np);
	if (ret)
		goto da9313_fail_0;

	ret = da9313_irq_init(di, np);
	if (ret)
		goto da9313_fail_1;

	da9313_para_init(di);
	wakeup_source_init(&di->wakelock, "da9313_wakelock");
	wakeup_source_init(&di->frequency_wakelock, "da9313_frequency_wakelock");
	mutex_init(&di->set_state_lock);

	if (di->single_sc_channel) {
		hwlog_info("single sc channel operation\n");
		da9313_board_init(di);
	}

	ret = vsys_sc_ops_register(&da9313_ops);
	if (ret)
		goto da9313_fail_2;

#ifdef CONFIG_HUAWEI_POWER_DEBUG
	power_dbg_ops_register("da9313_regval", i2c_get_clientdata(client),
		(power_dbg_show)da9313_dbg_show_reg_value,
		(power_dbg_store)da9313_dbg_store_reg_value);
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

	INIT_DELAYED_WORK(&di->frequency_work, da9313_frequency_work);

	return 0;

da9313_fail_2:
	wakeup_source_trash(&di->wakelock);
	wakeup_source_trash(&di->frequency_wakelock);
	free_irq(di->irq_int, di);
	gpio_free(di->gpio_int);
da9313_fail_1:
	if (di->support_pwrdown_mode)
		gpio_free(di->gpio_nonkey_en);
	gpio_free(di->gpio_pwron);
da9313_fail_0:
	devm_kfree(&client->dev, di);
	g_da9313_di = NULL;

	return ret;
}

static int da9313_remove(struct i2c_client *client)
{
	struct da9313_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	/* reset da9313 */
	(void)da9313_set_state(DA9313_CHIP_DISABLE);

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	if (di->gpio_pwron)
		gpio_free(di->gpio_pwron);

	if (di->support_pwrdown_mode) {
		if (di->gpio_nonkey_en)
			gpio_free(di->gpio_nonkey_en);
	}

	wakeup_source_trash(&di->wakelock);
	wakeup_source_trash(&di->frequency_wakelock);
	g_da9313_di = NULL;

	return 0;
}

static void da9313_shutdown(struct i2c_client *client)
{
	struct da9313_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return;

	if (di->single_sc_channel)
		da9313_set_normal_frequency_mode(di);
}

MODULE_DEVICE_TABLE(i2c, da9313);

static const struct of_device_id da9313_of_match[] = {
	{
		.compatible = "huawei,da9313",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id da9313_i2c_id[] = {
	{ "da9313", 0 }, {}
};

static struct i2c_driver da9313_driver = {
	.probe = da9313_probe,
	.remove = da9313_remove,
	.shutdown = da9313_shutdown,
	.id_table = da9313_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "huawei,da9313",
		.of_match_table = of_match_ptr(da9313_of_match),
	},
};

static int __init da9313_init(void)
{
	return i2c_add_driver(&da9313_driver);
}

static void __exit da9313_exit(void)
{
	i2c_del_driver(&da9313_driver);
}

module_init(da9313_init);
module_exit(da9313_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("da9313 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
