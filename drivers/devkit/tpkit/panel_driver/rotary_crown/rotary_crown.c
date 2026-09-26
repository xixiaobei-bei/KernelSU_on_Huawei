/*
 * Rotary Crown driver
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

#include "rotary_crown.h"
#include "securec.h"
#include <huawei_platform/log/log_jank.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/pinctrl/pinctrl-state.h>
#include <linux/platform_device.h>

#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif

#define CROWN_RECOVERY_TIME 5

typedef int (*crown_callback)(struct i2c_client *client);

struct event_work_struct {
	struct work_struct event_work;
	struct i2c_client *client;
	crown_callback callback;
};

static struct timer_list crown_recovery_timer;
static struct work_struct crown_recovery_work;
static struct mutex work_lock;

/* global values */
struct rotary_crown_data *g_rotary_crown_pdata;
static int rotary_crown_read_res_from_nv(void);

int rc_i2c_read(uint8_t *reg_addr, uint16_t reg_len, uint8_t *buf, uint16_t len)
{
	int count = 0;
	int ret;
	int msg_len;
	struct i2c_msg *msg_addr = NULL;
	struct i2c_msg xfer[ROTARY_I2C_MSG_LEN];
	struct i2c_client *client = g_rotary_crown_pdata->client;

	if (g_rotary_crown_pdata->is_one_byte) {
		/* Read data */
		xfer[ROTARY_I2C_MSG_INDEX0].addr = client->addr;
		xfer[ROTARY_I2C_MSG_INDEX0].flags = I2C_M_RD;
		xfer[ROTARY_I2C_MSG_INDEX0].len = len;
		xfer[ROTARY_I2C_MSG_INDEX0].buf = buf;
		do {
			ret = i2c_transfer(client->adapter, xfer,
				ROTARY_I2C_MSG_INDEX1);
			if (ret == ROTARY_I2C_MSG_INDEX1)
				return NO_ERR;

			msleep(I2C_WAIT_TIME);
		} while (++count < I2C_RW_TRIES);
	} else {
		/* register addr */
		xfer[ROTARY_I2C_MSG_INDEX0].addr = client->addr;
		xfer[ROTARY_I2C_MSG_INDEX0].flags = 0;
		xfer[ROTARY_I2C_MSG_INDEX0].len = reg_len;
		xfer[ROTARY_I2C_MSG_INDEX0].buf = reg_addr;

		/* Read data */
		xfer[ROTARY_I2C_MSG_INDEX1].addr = client->addr;
		xfer[ROTARY_I2C_MSG_INDEX1].flags = I2C_M_RD;
		xfer[ROTARY_I2C_MSG_INDEX1].len = len;
		xfer[ROTARY_I2C_MSG_INDEX1].buf = buf;

		if (reg_len > 0) {
			msg_len = ROTARY_I2C_MSG_LEN;
			msg_addr = &xfer[ROTARY_I2C_MSG_INDEX0];
		} else {
			msg_len = ROTARY_I2C_MSG_INDEX1;
			msg_addr = &xfer[ROTARY_I2C_MSG_INDEX1];
		}

		do {
			ret = i2c_transfer(client->adapter, msg_addr, msg_len);
			if (ret == msg_len)
				return NO_ERR;

			msleep(I2C_WAIT_TIME);
		} while (++count < I2C_RW_TRIES);
	}

	rc_err("%s: failed\n", __func__);
	return -EIO;
}

int rc_i2c_write(uint8_t *buf, uint16_t length)
{
	int count = 0;
	int ret;

	do {
		ret = i2c_master_send(g_rotary_crown_pdata->client,
			(const char *) buf, length);
		if (ret == length)
			return NO_ERR;

		msleep(I2C_WAIT_TIME);
	} while (++count < I2C_RW_TRIES);
#if defined(CONFIG_HUAWEI_DSM)
	// repot dmd
#endif

	rc_err("%s: failed", __func__);
	return -EIO;
}

static int rotary_crown_write_reg(uint8_t addr, uint8_t value)
{
	int ret;
	uint8_t cmd[ROTARY_REG_WRITE_LEN] = {0};

	// write reg
	cmd[ROTARY_I2C_MSG_INDEX0] = addr;
	cmd[ROTARY_I2C_MSG_INDEX1] = value;

	// soft reset do not give ack back
	if ((addr == PA_DEVICE_CONTROL) && (value == PA_CHIP_RESET)) {
		ret = i2c_master_send(g_rotary_crown_pdata->client,
			(const char *)cmd, ROTARY_REG_WRITE_LEN);
		rc_info("%s:soft reset send", __func__);
		return 0;
	}

	ret = rc_i2c_write(cmd, ROTARY_REG_WRITE_LEN);
	if (ret != NO_ERR)
		rc_err("set value(0x%02x) to reg(0x%02x) failed", value, addr);

	return ret;
}

static int rotary_crown_parse_dts(void)
{
	struct device_node *np = g_rotary_crown_pdata->dev->of_node;
	int ret;
	uint32_t value = 0;

	rc_debug("%s enter\n", __func__);
	if (!g_rotary_crown_pdata) {
		rc_err("%s: parameters invalid !\n", __func__);
		return -EINVAL;
	}

	g_rotary_crown_pdata->i2c_switch_gpio = of_get_named_gpio(
		np, "i2c_switch_gpio", 0);
	if (!gpio_is_valid(g_rotary_crown_pdata->i2c_switch_gpio)) {
		rc_err("%s:i2c_switch_gpio is not valid", __func__);
		return -EINVAL;
	}

	g_rotary_crown_pdata->irq_gpio = of_get_named_gpio(np, "irq_gpio", 0);
	if (!gpio_is_valid(g_rotary_crown_pdata->irq_gpio)) {
		rc_err("%s:irq_gpio is not valid !\n", __func__);
		return -EINVAL;
	}

	rc_debug("%s,irq_gpio=%d\n", __func__,
		g_rotary_crown_pdata->irq_gpio);
	// i2c one byte or not
	ret = of_property_read_u32(np, ROTARY_I2C_TYPE, &value);
	if (ret) {
		g_rotary_crown_pdata->is_one_byte = false;
		rc_err("get is_one_btye failed\n");
	} else {
		g_rotary_crown_pdata->is_one_byte = value ? true : false;
	}
	rc_debug("%s,one byte=%d\n", __func__,
		g_rotary_crown_pdata->is_one_byte);

	return 0;
}

static int rotary_crown_pinctrl_init(void)
{
	int ret;

	if (!g_rotary_crown_pdata) {
		rc_err("%s: parameters invalid !\n", __func__);
		return -EINVAL;
	}

	g_rotary_crown_pdata->rc_pinctrl =
		devm_pinctrl_get(g_rotary_crown_pdata->dev);
	if (IS_ERR(g_rotary_crown_pdata->rc_pinctrl)) {
		rc_err("%s:failed to devm pinctrl get\n", __func__);
		ret = -EINVAL;
		return ret;
	}

	do {
		g_rotary_crown_pdata->pinctrl_state_active =
			pinctrl_lookup_state(g_rotary_crown_pdata->rc_pinctrl,
				PINCTRL_STATE_DEFAULT);
		if (IS_ERR(g_rotary_crown_pdata->pinctrl_state_active)) {
			rc_err("%s:failed to lookup state active\n",
				__func__);
			ret = -EINVAL;
			break;
		}

		g_rotary_crown_pdata->pinctrl_state_suspend =
			pinctrl_lookup_state(g_rotary_crown_pdata->rc_pinctrl,
				PINCTRL_STATE_IDLE);
		if (IS_ERR(g_rotary_crown_pdata->pinctrl_state_suspend)) {
			rc_err("%s:failed to lookup suspend state\n",
				__func__);
			ret = -EINVAL;
			break;
		}

		/*
		 * Pinctrl handle is optional. If pinctrl handle
		 * is found, let pins to be configured in active state.
		 * If not found continue further without error.
		 */
		ret = pinctrl_select_state(g_rotary_crown_pdata->rc_pinctrl,
			g_rotary_crown_pdata->pinctrl_state_active);
		if (ret) {
			rc_err("%s:failed to set pinctr active\n",
				__func__);
			ret = -EINVAL;
			break;
		}

		return 0;
	} while (0);

	devm_pinctrl_put(g_rotary_crown_pdata->rc_pinctrl);
	return ret;
}

static void event_callback_work(struct work_struct *work)
{
	struct event_work_struct *event_work =
	container_of(work, struct event_work_struct, event_work);

	if (!event_work)
		return;
	mutex_lock(&work_lock);
	event_work->callback(event_work->client);

	rc_debug("%s event callback", __func__);
	kfree(event_work);
	mutex_unlock(&work_lock);
}

static void crown_handle_event(struct i2c_client *client,
				crown_callback callback)
{
	struct event_work_struct *ws = NULL;
	bool ret = false;
	/* free after callback work has done */
	ws = kmalloc(sizeof(*ws), GFP_KERNEL);
	if (!ws)
		return;
	INIT_WORK(&ws->event_work, event_callback_work);
	ws->client = client;
	ws->callback = callback;
	ret = schedule_work(&ws->event_work);
	if (!ret)
		kfree(ws);
}

static int rotary_crown_check_chip_id(void)
{
	uint8_t addr = PA_PRODUCT_ID1;
	uint8_t value = 0;
	int ret;

	rc_debug("%s:called\n", __func__);
	if (!g_rotary_crown_pdata) {
		rc_err("%s: parameters invalid !\n", __func__);
		return -EINVAL;
	}

	ret = rc_i2c_read(&addr, ROTARY_REG_ADDR_LEN,
		&value, ROTARY_REG_VALUE_LEN);
	if (ret != NO_ERR) {
		rc_err("%s: call rc_i2c_read failed", __func__);
		return ret;
	}

	rc_debug("%s: chip id(0x%02x)\n", __func__, value);
	if (value != PA_DEFAULT_CHIP_ID1)
		return -EINVAL;

	return NO_ERR;
}

static void rotary_crown_clear_interrupt(void)
{
	int ret;
	uint8_t addr;
	uint8_t value;

	addr = PA_MOTION_STATUS;
	ret = rc_i2c_read(&addr, ROTARY_REG_ADDR_LEN, &value,
		ROTARY_REG_VALUE_LEN);
	if (ret != NO_ERR)
		rc_err("%s: read motion status failed", __func__);

	addr = PA_DELTA_X_LOW;
	ret = rc_i2c_read(&addr, ROTARY_REG_ADDR_LEN, &value,
		ROTARY_REG_VALUE_LEN);
	if (ret != NO_ERR)
		rc_err("%s: read low delta x failed", __func__);

	addr = PA_DELTA_Y_LOW;
	ret = rc_i2c_read(&addr, ROTARY_REG_ADDR_LEN, &value,
		ROTARY_REG_VALUE_LEN);
	if (ret != NO_ERR)
		rc_err("%s: read low delta y failed", __func__);
}

static int rotary_crown_chip_init(void)
{
	int ret;

	// switch to bank 0
	ret = rotary_crown_write_reg(PA_PAGE_BANK_CONTROL, PA_PAGE_BANK_0);
	if (ret != NO_ERR)
		return ret;

	// software reset
	ret = rotary_crown_write_reg(PA_DEVICE_CONTROL, PA_CHIP_RESET);
	if (ret != NO_ERR)
		rc_err("%s: enter software reset failed", __func__);
	msleep(ROTARY_DELAY_1_MS);

	// exit software reset
	ret = rotary_crown_write_reg(PA_DEVICE_CONTROL, PA_CHIP_EXIT_RESET);
	if (ret != NO_ERR)
		return ret;

	ret = rotary_crown_write_reg(PA_OPERATION_MODE, PA_ENTER_SLEEP1_MODE);
	if (ret != NO_ERR)
		return ret;

	// disable write protect
	ret = rotary_crown_write_reg(PA_WRITE_PROTECT, PA_DISABLE_WR_PROTECT);
	if (ret != NO_ERR)
		return ret;

	// set X-axis resolution
	ret = rotary_crown_write_reg(PA_RESOLUTION_X_CONFIG,
		g_rotary_crown_pdata->res_x_nv);
	if (ret != NO_ERR)
		return ret;

	// set Y-axis resolution
	ret = rotary_crown_write_reg(PA_RESOLUTION_Y_CONFIG, PA_Y_RES_VALUE);
	if (ret != NO_ERR)
		return ret;

	// set 12-bit X/Y data format
	ret = rotary_crown_write_reg(PA_DELTA_DATA_FORMAT, PA_USE_12_BIT_FMT);
	if (ret != NO_ERR)
		return ret;

	// set power saving
	ret = rotary_crown_write_reg(PA_LOW_VOL_CONFIG, PA_SET_POWER_SAVING);
	if (ret != NO_ERR)
		return ret;

	// trace performance config
	ret = rotary_crown_write_reg(PA_TRACE_PERF_CONFIG3, PA_PERF_CFG3_VAL);
	if (ret != NO_ERR)
		return ret;

	ret = rotary_crown_write_reg(PA_TRACE_PERF_CONFIG1, PA_PERF_CFG1_VAL);
	if (ret != NO_ERR)
		return ret;

	ret = rotary_crown_write_reg(PA_TRACE_PERF_CONFIG2, PA_PERF_CFG2_VAL);
	if (ret != NO_ERR)
		return ret;

	// enable write protect
	ret = rotary_crown_write_reg(PA_WRITE_PROTECT, PA_ENABLE_WR_PROTECT);
	if (ret != NO_ERR)
		return ret;

	// clear interrupt
	rotary_crown_clear_interrupt();

	rc_info("%s:crown chip init success", __func__);

	return NO_ERR;
}

static irqreturn_t rotary_crown_irq_handler(int irq, void *dev_id)
{
	struct rotary_crown_data *rc = dev_id;

	rc_debug("%s: enter", __func__);
	if (!rc)
		return IRQ_HANDLED;

	schedule_work(&rc->work);
	return IRQ_HANDLED;
}

static int rotary_crown_read_motion(int16_t *dx)
{
	int ret;
	uint8_t addr;
	int16_t delta_x_low = 0;
	int16_t delta_x_high;
	int16_t delta_xy_high = 0;
	uint8_t value;

	if (dx == NULL)
		return -EINVAL;

	// read delta x low 8-bit
	addr = PA_DELTA_X_LOW;
	value = 0;
	ret = rc_i2c_read(&addr, ROTARY_REG_ADDR_LEN,
		&value, ROTARY_REG_VALUE_LEN);
	if (ret != NO_ERR) {
		rc_err("%s: read low delta x failed", __func__);
		return ret;
	}
	rc_debug("%s: low delta x reg 0x%02x", __func__, value);
	delta_x_low |= value;

	// read delta xy high 4-bit
	addr = PA_DELTA_XY_HIGH;
	value = 0;
	ret = rc_i2c_read(&addr, ROTARY_REG_ADDR_LEN,
		&value, ROTARY_REG_VALUE_LEN);
	if (ret != NO_ERR) {
		rc_err("%s: read high delta xy failed", __func__);
		return ret;
	}
	rc_debug("%s: high delta reg 0x%02x", __func__, value);
	delta_xy_high |= value;

	delta_x_high = (delta_xy_high << ROTARY_HIGH_DELTA_SHIFT)
		& ROTARY_HIGH_DELTA_MASK;
	if (delta_x_high & ROTARY_HIGH_DELTA_SIGH_MASK)
		delta_x_high |= ROTARY_HIGH_DELTA_SIGH;

	*dx = delta_x_high | delta_x_low;
	return NO_ERR;
}

static void rotary_crown_work_func(struct work_struct *work)
{
	int ret;
	uint8_t addr = PA_MOTION_STATUS;
	uint8_t motion_valid;
	uint8_t i;
	uint8_t no_motion_cnt;
	int16_t delta_x = 0;
	struct rotary_crown_data *rc = container_of(work,
		struct rotary_crown_data, work);

	if (rc->is_suspend) {
		rc_info("%s:in suspend, return", __func__);
		return;
	}

	if (!gpio_get_value(rc->i2c_switch_gpio)) {
		rc_info("%s:i2c is not at AP, exit", __func__);
		return;
	}

	mutex_lock(&work_lock);
	rc_debug("%s: enter", __func__);

	if (rc->is_init_ok && rc->irq_is_use) {
		disable_irq_nosync(rc->client->irq);
		rc_info("%s:crown irq disable", __func__);
	} else {
		rc_info("%s:irq not init, return", __func__);
		mutex_unlock(&work_lock);
		return;
	}

	do {
		no_motion_cnt = 0;
		for (i = 0; i < ROTARY_READ_MOTION_TIMES; i++) {
			motion_valid = 0;
			ret = rc_i2c_read(&addr, ROTARY_REG_ADDR_LEN,
				&motion_valid, ROTARY_REG_VALUE_LEN);
			if (ret != NO_ERR) {
				rc_err("%s: read motion status failed",
					__func__);
				break;
			}

			motion_valid &= ROTARY_MOTION_VALID_MASK;
			if (motion_valid)
				break;

			no_motion_cnt++;
		}

		// no motion for ROTARY_READ_MOTION_TIMES times
		if (no_motion_cnt >= ROTARY_READ_MOTION_TIMES) {
			rc_debug("%s: no motion for %d times",
				__func__, no_motion_cnt);
			break;
		}

		if (!gpio_get_value(rc->i2c_switch_gpio)) {
			rc_info("%s:i2c switch to mcu, break", __func__);
			break;
		}
		no_motion_cnt = 0;
		ret = rotary_crown_read_motion(&delta_x);
		if (ret != NO_ERR) {
			rc_err("%s: read motion failed", __func__);
			continue;
		}

		rc_debug("%s: delta x 0x%04x", __func__, delta_x);
		input_report_rel(rc->input_dev, REL_WHEEL, delta_x);
		input_sync(rc->input_dev);

		msleep(ROTARY_READ_DELTA_DELAY);
	} while (motion_valid);

	if (rc->irq_is_use) {
		enable_irq(rc->client->irq);
		rc_info("%s:crown irq enable", __func__);
	}
	rc_debug("%s: exit", __func__);
	mutex_unlock(&work_lock);
}

static int rotary_crown_run_mode_set(bool enable)
{
	int ret;

	rc_debug("%s: enter, enable = %d", __func__, enable);
	if (enable) {
		// exit power down mode
		ret = rotary_crown_write_reg(PA_DEVICE_CONTROL, PA_CHIP_EXIT_RESET);
		if (ret != NO_ERR) {
			rc_err("%s: exit power down mode failed, value = %d",
				__func__, PA_CHIP_EXIT_RESET);
			return ret;
		}

		ret = rotary_crown_write_reg(PA_OPERATION_MODE, PA_ENTER_SLEEP1_MODE);
		if (ret != NO_ERR) {
			rc_err("%s: set operation mode failed, value = %d",
				__func__, PA_ENTER_SLEEP1_MODE);
			return ret;
		}

		return NO_ERR;
	}

	// enter power down mode
	ret = rotary_crown_write_reg(PA_DEVICE_CONTROL, PA_CHIP_POWER_DOWN);
	if (ret != NO_ERR) {
		rc_err("%s: enter power down mode failed, value = %d",
			__func__, PA_CHIP_POWER_DOWN);
		return ret;
	}

	return NO_ERR;
}

static int rotary_crown_suspend(struct i2c_client *client)
{
	int ret;
	struct rotary_crown_data *rc = i2c_get_clientdata(client);

	rc_info("%s: enter", __func__);
	if (rc->is_suspend == true) {
		rc_info("%s:already in suspend, skip", __func__);
		return 0;
	}

	if (!IS_ERR(rc->rc_pinctrl)) {
		ret = pinctrl_select_state(rc->rc_pinctrl,
			rc->pinctrl_state_suspend);
		if (ret < 0)
			rc_err("%s:failed to set suspend state", __func__);
	}

	if (gpio_get_value(g_rotary_crown_pdata->i2c_switch_gpio)) {
		rc_info("%s:i2c is at AP, set mode", __func__);
		ret = rotary_crown_run_mode_set(false);
		if (ret)
			rc_err("%s: enter sleep 2 mode failed", __func__);

		// clear interrupt
		rotary_crown_clear_interrupt();
	}

	// disable interrupt
	if (rc->irq_is_use) {
		disable_irq_nosync(rc->client->irq);
		rc_info("%s:crown irq disable", __func__);
		rc->is_suspend = true;
	}

	rc_info("%s: exit\n", __func__);

	return 0;
}

static int rotary_crown_resume(struct i2c_client *client)
{
	int ret;
	struct rotary_crown_data *rc = i2c_get_clientdata(client);

	rc_info("%s enter", __func__);
	if (rc->is_suspend == false) {
		rc_info("%s:already in resume, skip", __func__);
		return 0;
	}

	if (!gpio_get_value(g_rotary_crown_pdata->i2c_switch_gpio)) {
		rc_info("%s:i2c is not at AP, exit", __func__);
		return 0;
	}

	if (!IS_ERR(rc->rc_pinctrl)) {
		ret = pinctrl_select_state(rc->rc_pinctrl,
			rc->pinctrl_state_active);
		if (ret < 0)
			rc_err("%s:failed to set active state", __func__);
	}

	if (g_rotary_crown_pdata->nv_status_count < CROWN_RECOVERY_TIME) {
		rotary_crown_read_res_from_nv();
		g_rotary_crown_pdata->nv_status_count++;
		rc_info("%s:res_x_nv = %d", __func__, g_rotary_crown_pdata->res_x_nv);
	}

	ret = rotary_crown_chip_init();
	if (ret)
		rc_err("%s: enter run mode failed", __func__);

	// enable irq
	if (rc->irq_is_use) {
		enable_irq(rc->client->irq);
		rc_info("%s:crown irq enable", __func__);
		rc->is_suspend = false;
	}

	rc_info("%s exit", __func__);

	return 0;
}

#if defined(CONFIG_FB)
static int fb_notifier_callback(
	struct notifier_block *self, unsigned long event, void *data)
{
	struct rotary_crown_data *rc_data = container_of(self,
		struct rotary_crown_data, fb_notif);
	rc_info("%s:enter, event = %d, suspend=%d", __func__, event, rc_data->is_suspend);
	if (rc_data && rc_data->client) {
		if (event == LCDKIT_TS_RESUME_DEVICE)
			crown_handle_event(rc_data->client, rotary_crown_resume);
		else if (event == LCDKIT_TS_SUSPEND_DEVICE)
			crown_handle_event(rc_data->client, rotary_crown_suspend);
	}
	rc_info("%s:exit", __func__);

	return 0;
}
#elif defined(CONFIG_HAS_EARLYSUSPEND)
static void rotary_crown_early_suspend(struct early_suspend *h)
{
	struct rotary_crown_data *rc;

	rc_debug("%s: enter", __func__);
	rc = container_of(h, struct rotary_crown_data, early_suspend);
	rotary_crown_suspend(rc->client);
}

static void rotary_crown_late_resume(struct early_suspend *h)
{
	struct rotary_crown_data *rc;

	rc_debug("%s: enter", __func__);
	rc = container_of(h, struct rotary_crown_data, early_suspend);
	rotary_crown_resume(rc->client);
}
#endif

#if (!defined(CONFIG_FB) && !defined(CONFIG_HAS_EARLYSUSPEND))

static int rotary_crown_pm_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);

	rc_debug("%s: enter", __func__);
	rotary_crown_suspend(client);

	return 0;
}

static int rotary_crown_pm_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);

	rc_debug("%s: enter", __func__);
	rotary_crown_resume(client);

	return 0;
}

static const struct dev_pm_ops rotary_crown_dev_pm_ops = {
	.suspend = rotary_crown_pm_suspend,
	.resume = rotary_crown_pm_resume,
};
#endif

static int rotary_crown_irq_init(void)
{
	int ret;

	// interrupt setting
	do {
		ret = gpio_request(g_rotary_crown_pdata->irq_gpio,
			"RC_INT_IRQ");
		if (ret < 0) {
			rc_err("Failed to request GPIO:%d, ERRNO:%d",
				(s32) g_rotary_crown_pdata->irq_gpio, ret);
			ret = -ENODEV;
			break;
		}

		ret = gpio_direction_input(g_rotary_crown_pdata->irq_gpio);
		if (ret < 0) {
			rc_err("Failed to set GPIO direction:%d, ERRNO:%d",
				(s32) g_rotary_crown_pdata->irq_gpio, ret);
			ret = -ENODEV;
			break;
		}

		g_rotary_crown_pdata->client->irq =
			gpio_to_irq(g_rotary_crown_pdata->irq_gpio);
		g_rotary_crown_pdata->irq_is_use = true;
		rc_debug("irq num:%d\n", g_rotary_crown_pdata->client->irq);

		INIT_WORK(&(g_rotary_crown_pdata->work),
			rotary_crown_work_func);

		if (g_rotary_crown_pdata->irq_is_use) {
			// IRQF_TRIGGER_NONE 0x00000000
			// IRQF_TRIGGER_RISING   0x00000001
			// IRQF_TRIGGER_FALLING  0x00000002
			// IRQF_TRIGGER_HIGH 0x00000004
			// IRQF_TRIGGER_LOW  0x00000008
			ret = request_irq(g_rotary_crown_pdata->client->irq,
				rotary_crown_irq_handler,
				IRQF_TRIGGER_FALLING,
				g_rotary_crown_pdata->client->name,
				g_rotary_crown_pdata);
			if (ret != 0) {
				rc_err("request_irq failed");
				break;
			}
			disable_irq_nosync(g_rotary_crown_pdata->client->irq);
			rc_info("%s:crown irq disable", __func__);
			g_rotary_crown_pdata->is_suspend = true;
		}
		return NO_ERR;
	} while (0);

	if (gpio_is_valid(g_rotary_crown_pdata->irq_gpio)) {
		gpio_free(g_rotary_crown_pdata->irq_gpio);
		g_rotary_crown_pdata->irq_is_use = false;
	}

	return ret;
}

static int rotary_crown_input_init(void)
{
	int ret;

	// interrupt setting
	do {
		// pin control setting
		ret = rotary_crown_pinctrl_init();
		if (ret != NO_ERR) {
			rc_err("failed to init pin control");
			ret = -ENODEV;
			break;
		}

		// input setting
		g_rotary_crown_pdata->input_dev = input_allocate_device();
		if (g_rotary_crown_pdata->input_dev == NULL) {
			rc_err("Can not allocate memory for input device");
			ret = -ENOMEM;
			break;
		}

		g_rotary_crown_pdata->input_dev->name =
			ROTARY_CROWN_INPUT_DEV_NAME;
		g_rotary_crown_pdata->input_dev->dev.parent =
			&(g_rotary_crown_pdata->client->dev);
		g_rotary_crown_pdata->input_dev->id.bustype = BUS_I2C;
		input_set_capability(g_rotary_crown_pdata->input_dev,
			EV_REL, REL_WHEEL);
		ret = input_register_device(g_rotary_crown_pdata->input_dev);
		if (ret < 0) {
			rc_err("Can not register input device");
			ret = -ENODEV;
			break;
		}

		return NO_ERR;
	} while (0);

	if (g_rotary_crown_pdata->input_dev) {
		input_unregister_device(g_rotary_crown_pdata->input_dev);
		input_free_device(g_rotary_crown_pdata->input_dev);
		g_rotary_crown_pdata->input_dev = NULL;
	}

	return ret;
}

static ssize_t rotary_crown_self_test(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	int ret;

	rc_debug("%s: called\n", __func__);
	if ((dev == NULL) || (buf == NULL)) {
		rc_err("%s: parameter is null\n", __func__);
		return -EINVAL;
	}

	ret = rotary_crown_check_chip_id();
	if (ret != NO_ERR)
		ret = snprintf_s(buf, ROTARY_STR_LEN, ROTARY_STR_LEN - 1,
			"%s\n", "Fail");
	else
		ret = snprintf_s(buf, ROTARY_STR_LEN, ROTARY_STR_LEN - 1,
			"%s\n", "Success");

	return ret;
}

static int rotary_crown_read_res_from_nv(void)
{
	int ret;
	struct hisi_nve_info_user user_info;

	memset_s(&user_info, sizeof(user_info), 0, sizeof(user_info));
	user_info.nv_operation = NV_READ_OPERATION;
	user_info.nv_number = NV_NUMBER_VALUE;
	user_info.valid_size = RC_RES_NV_DATA_SIZE;
	strncpy_s(user_info.nv_name, sizeof(user_info.nv_name), "RCRES",
		RC_NV_NAME_LEN);
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret) {
		rc_err("hisi_nve_direct_access read error %d\n", ret);
		return -EINVAL;
	}

	if ((user_info.nv_data[0] <= ROTARY_MIN_RES_X) ||
	    (user_info.nv_data[0] >= ROTARY_MAX_RES_X)) {
		rc_err("nv value is not invalid %d\n", user_info.nv_data[0]);
		return -EINVAL;
	}

	g_rotary_crown_pdata->res_x_nv = user_info.nv_data[0];
	rc_debug("%s: 0x%02x\n", __func__, g_rotary_crown_pdata->res_x_nv);
	return 0;
}

// calibration defition
static int rotary_crown_set_res(void)
{
	int ret;

	rc_debug("%s:called\n", __func__);
	// disable write protect
	ret = rotary_crown_write_reg(PA_WRITE_PROTECT, PA_DISABLE_WR_PROTECT);
	if (ret != NO_ERR)
		return ret;

	// set X-axis resolution
	ret = rotary_crown_write_reg(PA_RESOLUTION_X_CONFIG,
		g_rotary_crown_pdata->res_x_nv);
	if (ret != NO_ERR)
		return ret;

	// set Y-axis resolution
	ret = rotary_crown_write_reg(PA_RESOLUTION_Y_CONFIG, PA_Y_RES_VALUE);
	if (ret != NO_ERR)
		return ret;

	// enable write protect
	ret = rotary_crown_write_reg(PA_WRITE_PROTECT, PA_ENABLE_WR_PROTECT);
	if (ret != NO_ERR)
		return ret;

	return NO_ERR;
}

static ssize_t rotary_crown_calibrate_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	unsigned long val = 0;
	int ret;

	rc_debug("%s: called\n", __func__);
	if ((dev == NULL) || (buf == NULL)) {
		rc_err("%s: parameter is null\n", __func__);
		return -EINVAL;
	}

	ret = kstrtoul(buf, 0, &val);
	if (ret) {
		rc_err("%s: call strtoul failed :%d\n", __func__, ret);
		return -EINVAL;
	}

	rc_debug("%s: val(%lu)\n", __func__, val);
	if (val) {
		g_rotary_crown_pdata->res_x_nv = (u8)val;
	} else {
		ret = rotary_crown_read_res_from_nv();
		if (ret) {
			rc_err("%s: failed to read nv\n", __func__);
			return ret;
		}
	}

	ret = rotary_crown_set_res();
	if (ret) {
		rc_err("%s: write nv to chip failed :%d\n",
			__func__, ret);
		return ret;
	}

	return count;
}

static ssize_t rotary_crown_calibrate_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	int ret;
	uint8_t addr = PA_RESOLUTION_X_CONFIG;
	uint8_t value = 0;
	uint8_t offset = 0;

	rc_debug("%s: called\n", __func__);
	if ((dev == NULL) || (buf == NULL)) {
		rc_err("%s: parameter is null\n", __func__);
		return -EINVAL;
	}

	ret = rc_i2c_read(&addr, ROTARY_REG_ADDR_LEN, &value,
		ROTARY_REG_VALUE_LEN);
	if (ret != NO_ERR)
		rc_err("%s: read x resolution failed", __func__);

	rc_debug("%s: x resoluttion(0x%02x)\n", __func__, value);
	offset += snprintf_s(buf + offset, ROTARY_STR_LEN - offset,
		ROTARY_STR_LEN - offset - 1, "%d\n", value);

	return offset;
}

#ifdef RC_FACTORY_MODE
// enter calibration
static int rc_calibrate_enter(void)
{
	int ret;

	rc_debug("%s: enter\n", __func__);
	if (g_rotary_crown_pdata == NULL)
		return -ENOMEM;

	g_rotary_crown_pdata->res_x_nv = PA_X_RES_VALUE;
	ret = rotary_crown_set_res();
	if (ret) {
		rc_err("%s: set chip resolution failed", __func__);
		return ret;
	}

	ret = rotary_crown_run_mode_set(true);
	if (ret) {
		rc_err("%s: enter run mode failed", __func__);
		return ret;
	}

	// disable interrupt
	if (g_rotary_crown_pdata->irq_is_use)
		disable_irq_nosync(g_rotary_crown_pdata->client->irq);

	// clear interrupt
	rotary_crown_clear_interrupt();

	return NO_ERR;
}

// exit calibration
static int rc_calibrate_exit(void)
{
	int ret;

	rc_debug("%s: enter\n", __func__);
	ret = rotary_crown_run_mode_set(false);
	if (ret) {
		rc_err("%s: enter run mode failed", __func__);
		return ret;
	}

	// clear interrupt
	rotary_crown_clear_interrupt();

	if (g_rotary_crown_pdata->irq_is_use)
		enable_irq(g_rotary_crown_pdata->client->irq);

	return NO_ERR;
}

// enter nv result test
static int rc_nv_test_enter(void)
{
	int ret;

	rc_debug("%s: enter\n", __func__);
	if (g_rotary_crown_pdata == NULL)
		return -ENOMEM;

	ret = rotary_crown_run_mode_set(true);
	if (ret) {
		rc_err("%s: enter run mode failed", __func__);
		return ret;
	}

	// disable interrupt
	if (g_rotary_crown_pdata->irq_is_use)
		disable_irq_nosync(g_rotary_crown_pdata->client->irq);

	// clear interrupt
	rotary_crown_clear_interrupt();

	return NO_ERR;
}

static ssize_t rotary_crown_calibrate_mode_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	unsigned long val = 0;
	int ret;

	rc_debug("%s: called\n", __func__);
	if ((dev == NULL) || (buf == NULL)) {
		rc_err("%s: parameter is null\n", __func__);
		return -EINVAL;
	}

	ret = kstrtoul(buf, 0, &val);
	if (ret) {
		rc_err("%s: call strtoul failed :%d\n", __func__, ret);
		return -EINVAL;
	}

	rc_debug("%s: val(%lu)\n", __func__, val);
	if (val)
		ret = rc_calibrate_enter();
	else
		ret = rc_calibrate_exit();

	if (ret) {
		rc_err("%s: calibrate control failed :%d\n", __func__, ret);
		return ret;
	}

	return count;
}

static ssize_t rotary_crown_calibrate_mode_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	int ret;
	uint8_t addr = PA_RESOLUTION_X_CONFIG;
	uint8_t value = 0;
	uint8_t offset = 0;

	rc_debug("%s: called\n", __func__);
	if ((dev == NULL) || (buf == NULL)) {
		rc_err("%s: parameter is null\n", __func__);
		return -EINVAL;
	}

	ret = rc_i2c_read(&addr, ROTARY_REG_ADDR_LEN, &value,
		ROTARY_REG_VALUE_LEN);
	if (ret != NO_ERR)
		rc_err("%s: read x resolution failed", __func__);

	rc_debug("%s: x resoluttion(0x%02x)\n", __func__, value);
	offset += snprintf_s(buf + offset, ROTARY_STR_LEN - offset,
		ROTARY_STR_LEN - offset - 1, "RES_X:%d, DELTA_X:%d\n",
		value, ROTARY_STD_DELTA_VAL);

	return offset;
}

static ssize_t rotary_crown_test_mode_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	unsigned long val = 0;
	int ret;

	rc_debug("%s: called\n", __func__);
	if ((dev == NULL) || (buf == NULL)) {
		rc_err("%s: parameter is null\n", __func__);
		return -EINVAL;
	}

	ret = kstrtoul(buf, 0, &val);
	if (ret) {
		rc_err("%s: call strtoul failed :%d\n", __func__, ret);
		return -EINVAL;
	}

	rc_debug("%s: val(%lu)\n", __func__, val);
	if (val)
		ret = rc_nv_test_enter();
	else
		ret = rc_calibrate_exit();

	if (ret) {
		rc_err("%s: nv test control failed :%d\n", __func__, ret);
		return ret;
	}

	return count;
}

static ssize_t rotary_crown_delta_motion_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	int ret;
	uint8_t addr;
	uint8_t value = 0;
	uint8_t offset = 0;
	uint16_t delta_x = 0;

	rc_debug("%s: called\n", __func__);
	if ((dev == NULL) || (buf == NULL)) {
		rc_err("%s: parameter is null\n", __func__);
		return -EINVAL;
	}

	addr = PA_MOTION_STATUS;
	ret = rc_i2c_read(&addr, ROTARY_REG_ADDR_LEN, &value,
		ROTARY_REG_VALUE_LEN);
	if (ret != NO_ERR) {
		rc_err("%s: read motion status failed", __func__);
		return ret;
	}

	// read delta x
	ret = rotary_crown_read_motion(&delta_x);
	if (ret != NO_ERR) {
		rc_err("%s: read x resolution failed", __func__);
		return ret;
	}
	rc_debug("%s: delta x(0x%04x)\n", __func__, delta_x);
	offset = snprintf_s(buf + offset, ROTARY_STR_LEN - offset,
		ROTARY_STR_LEN - offset - 1, "%d\n", delta_x);

	return offset;
}
#endif // RC_FACTORY_MODE

// device file node create
static DEVICE_ATTR(rotary_crown_self_test, 0444, rotary_crown_self_test, NULL);
static DEVICE_ATTR(rotary_crown_calibrate, 0664, rotary_crown_calibrate_show,
	rotary_crown_calibrate_store);
#ifdef RC_FACTORY_MODE
static DEVICE_ATTR(rotary_crown_calibrate_mode, 0664,
	rotary_crown_calibrate_mode_show,
	rotary_crown_calibrate_mode_store);
static DEVICE_ATTR(rotary_crown_test_mode, 0664, NULL,
	rotary_crown_test_mode_store);
static DEVICE_ATTR(rotary_crown_delta_motion, 0444,
	rotary_crown_delta_motion_show, NULL);
#endif // RC_FACTORY_MODE

static struct attribute *rc_attributes[] = {
	&dev_attr_rotary_crown_self_test.attr,
	&dev_attr_rotary_crown_calibrate.attr,
#ifdef RC_FACTORY_MODE
	&dev_attr_rotary_crown_calibrate_mode.attr,
	&dev_attr_rotary_crown_test_mode.attr,
	&dev_attr_rotary_crown_delta_motion.attr,
#endif // RC_FACTORY_MODE
	NULL
};

static struct attribute_group g_rc_attr_group = {
	.attrs = rc_attributes,
};

static int rotary_crown_remove(struct i2c_client *client)
{
	struct rotary_crown_data *rc = i2c_get_clientdata(client);
	int ret;

	ret = rotary_crown_run_mode_set(false);
	if (ret)
		rc_err("%s: enter sleep 2 mode failed", __func__);
#if defined(CONFIG_FB)
	if (fb_unregister_client(&rc->fb_notif))
		rc_err("%s: unregistering fb_notifier failed", __func__);
#elif defined(CONFIG_HAS_EARLYSUSPEND)
	unregister_early_suspend(&rc->early_suspend);
#endif

	if (!IS_ERR(g_rotary_crown_pdata->rc_pinctrl)) {
		ret = pinctrl_select_state(g_rotary_crown_pdata->rc_pinctrl,
			g_rotary_crown_pdata->pinctrl_state_suspend);
		if (ret)
			rc_err("%s:failed to set release state", __func__);
		devm_pinctrl_put(g_rotary_crown_pdata->rc_pinctrl);
	}
	g_rotary_crown_pdata->rc_pinctrl = NULL;
	i2c_set_clientdata(client, NULL);
	if (rc->irq_is_use)
		free_irq(client->irq, rc);
	if (rc->input_dev)
		input_unregister_device(rc->input_dev);

	if (g_rotary_crown_pdata->sys_file_ok)
		sysfs_remove_group(&client->dev.kobj, &g_rc_attr_group);

	if (g_rotary_crown_pdata != NULL) {
		devm_kfree(&client->dev, g_rotary_crown_pdata);
		g_rotary_crown_pdata = NULL;
	}

	return 0;
}

static void rotary_crown_clear(struct i2c_client *client)
{
	int ret;

	if (g_rotary_crown_pdata->input_dev) {
		input_unregister_device(g_rotary_crown_pdata->input_dev);
		input_free_device(g_rotary_crown_pdata->input_dev);
		g_rotary_crown_pdata->input_dev = NULL;
	}

	if (gpio_is_valid(g_rotary_crown_pdata->irq_gpio)) {
		gpio_free(g_rotary_crown_pdata->irq_gpio);
		g_rotary_crown_pdata->irq_is_use = false;
		free_irq(client->irq, g_rotary_crown_pdata);
	}

#if defined(CONFIG_FB)
	if (g_rotary_crown_pdata->fb_notif.notifier_call)
		if (lcdkit_unregister_notifier(&g_rotary_crown_pdata->fb_notif))
			rc_err("%s: unregister fb_notifier failed", __func__);
#elif defined(CONFIG_HAS_EARLYSUSPEND)
	if (g_rotary_crown_pdata->early_suspend.suspend)
		unregister_early_suspend(&g_rotary_crown_pdata->early_suspend);
#endif
	mutex_destroy(&work_lock);
	i2c_set_clientdata(client, NULL);
	if (!IS_ERR(g_rotary_crown_pdata->rc_pinctrl)) {
		ret = pinctrl_select_state(g_rotary_crown_pdata->rc_pinctrl,
			g_rotary_crown_pdata->pinctrl_state_suspend);
		if (ret)
			rc_err("failed to select release pinctrl state");
		devm_pinctrl_put(g_rotary_crown_pdata->rc_pinctrl);
		g_rotary_crown_pdata->rc_pinctrl = NULL;
	}

	if (g_rotary_crown_pdata->sys_file_ok)
		sysfs_remove_group(&client->dev.kobj, &g_rc_attr_group);

	if (g_rotary_crown_pdata != NULL) {
		devm_kfree(&client->dev, g_rotary_crown_pdata);
		g_rotary_crown_pdata = NULL;
	}
}

static void rotary_crown_pm_init(void)
{
	int ret;

	if (g_rotary_crown_pdata == NULL)
		return;

#if defined(CONFIG_FB)
	g_rotary_crown_pdata->fb_notif.notifier_call = fb_notifier_callback;
		ret = lcdkit_register_notifier(&g_rotary_crown_pdata->fb_notif);
		if (ret)
			rc_err("Unable to register fb_notifier: %d\n", ret);
#elif defined(CONFIG_HAS_EARLYSUSPEND)
	g_rotary_crown_pdata->early_suspend.level =
		EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
		g_rotary_crown_pdata->early_suspend.suspend =
			rotary_crown_early_suspend;
		g_rotary_crown_pdata->early_suspend.resume =
			rotary_crown_late_resume;
		register_early_suspend(&g_rotary_crown_pdata->early_suspend);
#endif // CONFIG_HAS_EARLYSUSPEND
}

static int rotary_crown_calibrate_init(void)
{
	int ret;

	rc_debug("%s:called\n", __func__);
	do {
		g_rotary_crown_pdata->sys_file_ok = false;
		ret = sysfs_create_group(&g_rotary_crown_pdata->dev->kobj,
			&g_rc_attr_group);
		if (ret) {
			rc_err("%s: failed to create device attr, ret=%d\n",
				__func__, ret);
			return -ENODEV;
		}

		g_rotary_crown_pdata->sys_file_ok = true;
		ret = sysfs_create_link(NULL, &g_rotary_crown_pdata->dev->kobj,
			"rotarycrown");
		if (ret < 0) {
			pr_err("%s: Failed to sysfs_create_link, %d\n",
				__func__, ret);
			ret = -ENODEV;
			break;
		}

		g_rotary_crown_pdata->res_x_nv = PA_X_RES_VALUE;

		return NO_ERR;
	} while (0);

	sysfs_remove_group(&g_rotary_crown_pdata->dev->kobj, &g_rc_attr_group);
	return ret;
}

static int rotary_crown_detect(void)
{
	int ret;

	ret = rotary_crown_check_chip_id();
	if (ret)
		return ret;

	// calibration init
	ret = rotary_crown_calibrate_init();
	if (ret)
		return ret;

	if (g_rotary_crown_pdata->nv_status_count < CROWN_RECOVERY_TIME) {
		rotary_crown_read_res_from_nv();
		g_rotary_crown_pdata->nv_status_count++;
		rc_info("%s:res_x_nv = %d", __func__, g_rotary_crown_pdata->res_x_nv);
	}

	ret = rotary_crown_chip_init();
	if (ret)
		return ret;

	if (g_rotary_crown_pdata->irq_is_use && g_rotary_crown_pdata->is_suspend) {
		enable_irq(g_rotary_crown_pdata->client->irq);
		rc_info("%s:crown irq enable", __func__);
		g_rotary_crown_pdata->is_suspend = false;
	}

	g_rotary_crown_pdata->is_init_ok = true;
	return 0;
}

static void crown_recovery_work_callback(struct work_struct *work)
{
	int ret;
	unsigned long expires;

	rc_info("%s:time on, crown recovery", __func__);
	ret = rotary_crown_detect();
	if (!ret) {
		del_timer(&crown_recovery_timer);
		cancel_work_sync(&crown_recovery_work);
		rc_info("%s:crown recovery success", __func__);
	}
	expires = jiffies + (CROWN_RECOVERY_TIME * HZ);
	mod_timer(&crown_recovery_timer, expires);
}

static void crown_recovery_func(unsigned long data)
{
	schedule_work(&crown_recovery_work);
}

static int rotary_crown_probe(
	struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	int size;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -ENODEV;

	g_rotary_crown_pdata = NULL;
	size = sizeof(*g_rotary_crown_pdata);
	g_rotary_crown_pdata = devm_kzalloc(&client->dev, size, GFP_KERNEL);
	if (!g_rotary_crown_pdata)
		return -ENOMEM;

	ret = memset_s(g_rotary_crown_pdata, size, 0, size);
	if (ret)
		goto err_free;

	i2c_set_clientdata(client, g_rotary_crown_pdata);
	g_rotary_crown_pdata->client = client;
	g_rotary_crown_pdata->dev = &client->dev;
	g_rotary_crown_pdata->nv_status_count = 0;
	ret = rotary_crown_parse_dts();
	if (ret)
		goto err_free;

	// irq setting
	ret = rotary_crown_irq_init();
	if (ret)
		goto err_free;

	// input and pin control setting
	ret = rotary_crown_input_init();
	if (ret)
		goto err_free;

	rotary_crown_pm_init();
	mutex_init(&work_lock);
	ret = rotary_crown_detect();
	if (!ret)
		return 0;

	rc_err("%s: init failed", __func__);
	INIT_WORK(&crown_recovery_work, crown_recovery_work_callback);
	init_timer(&crown_recovery_timer);
	crown_recovery_timer.expires = jiffies + (CROWN_RECOVERY_TIME * HZ);
	crown_recovery_timer.function = &crown_recovery_func;
	crown_recovery_timer.data = 0;
	add_timer(&crown_recovery_timer);

	return 0;

err_free:
	devm_kfree(&client->dev, g_rotary_crown_pdata);
	return ret;
}

static const struct i2c_device_id rotary_crown_id[] = {
	{ROTARY_CROWN_DRV_NAME, 0},
	{}
};

static const struct of_device_id rotary_crown_match_table[] = {
	{.compatible = "pixart,pat9126ja", },
	{},
};

static struct i2c_driver rotary_crown_driver = {
	.probe      = rotary_crown_probe,
	.remove     = rotary_crown_remove,
	.id_table   = rotary_crown_id,
	.driver = {
		.name   = ROTARY_CROWN_DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = rotary_crown_match_table,
#if (!defined(CONFIG_FB) && !defined(CONFIG_HAS_EARLYSUSPEND))
		.suspend = NULL,
		.resume = NULL,
		.pm     = &rotary_crown_dev_pm_ops,
#endif
	},
};

static int __init rotary_crown_module_init(void)
{
	rc_debug("%s: enter", __func__);
	return i2c_add_driver(&rotary_crown_driver);
}

static void __exit rotary_crown_module_exit(void)
{
	rc_debug("%s: enter", __func__);
	i2c_del_driver(&rotary_crown_driver);
}

late_initcall(rotary_crown_module_init);
module_exit(rotary_crown_module_exit);
MODULE_AUTHOR("Huawei Device Company");
MODULE_DESCRIPTION("Huawei RotaryCrown Driver");
MODULE_LICENSE("GPL");
