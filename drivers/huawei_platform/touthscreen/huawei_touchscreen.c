/*
 * Huawei Touchscreen Driver
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

#include <huawei_platform/touthscreen/huawei_touchscreen.h>
#include <linux/sched.h>
#include <linux/cpumask.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/debugfs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/regulator/consumer.h>
#include <linux/string.h>
#include <linux/of_gpio.h>
#include <linux/kthread.h>
#include <linux/uaccess.h>
#include <linux/sched/rt.h>
#include <linux/fb.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <linux/fb.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <uapi/linux/sched/types.h>
#include <huawei_platform/touthscreen/huawei_ts_kit_misc_dev.h>

#define MAX_LOTUS_NUM 6
#define	EDGE_WIDTH_DEFAULT 10
#define BUFFER1_LEN 6
#define BUFFER2_LEN 50
#define PRIORITY_SETTING 99
#define TWO_BIT 2

struct ts_data g_ts_data;
atomic_t g_data_report_over = ATOMIC_INIT(1);
u8 g_ts_log_cfg;

static struct ts_cmd_node ping_cmd_buff;
static struct ts_cmd_node pang_cmd_buff;
static struct work_struct tp_init_work;
static struct mutex easy_wake_guesure_lock;
int tp_gpio_num;
EXPORT_SYMBOL(tp_gpio_num);

static struct ts_device_data g_ts_device_map[] = {
	[0] = {
		.chip_name = "synaptics",
		.irq_gpio = TS_IO_UNDEFINE,
		.irq_config = TS_IRQ_CFG_UNDEFINE,
		.reset_gpio = TS_IO_UNDEFINE,
		.ops = &ts_synaptics_ops,
	},
};

static int ts_i2c_read(u8 *reg_addr, u16 reg_len, u8 *buf, u16 len)
{
	int count = 0;
	int ret;
	struct i2c_msg xfer[TWO_BIT];

	if (g_ts_data.chip_data->is_i2c_one_byte) {
		/* Read data */
		xfer[0].addr = g_ts_data.client->addr;
		xfer[0].flags = I2C_M_RD;
		xfer[0].len = len;
		xfer[0].buf = buf;
		do {
			ret = i2c_transfer(g_ts_data.client->adapter, xfer, 1);
			if (ret == 1)
				return NO_ERR;
			msleep(I2C_WAIT_TIME);
		} while (++count < I2C_RW_TRIES);
	} else {
		/* register addr */
		xfer[0].addr = g_ts_data.client->addr;
		xfer[0].flags = 0;
		xfer[0].len = reg_len;
		xfer[0].buf = reg_addr;

		/* Read data */
		xfer[1].addr = g_ts_data.client->addr;
		xfer[1].flags = I2C_M_RD;
		xfer[1].len = len;
		xfer[1].buf = buf;

		do {
			ret = i2c_transfer(g_ts_data.client->adapter,
				xfer, TWO_BIT);
			if (ret == TWO_BIT)
				return NO_ERR;
			msleep(I2C_WAIT_TIME);
		} while (++count < I2C_RW_TRIES);
	}

	ts_log_err("%s failed\n", __func__);
	return -EIO;
}

static int ts_i2c_write(u8 *buf, u16 length)
{
	int count = 0;
	int ret;

	do {
		ret = i2c_master_send(g_ts_data.client, buf, length);
		if (ret == length)
			return NO_ERR;
		msleep(I2C_WAIT_TIME);
	} while (++count < I2C_RW_TRIES);

	ts_log_err("%s: failed\n", __func__);
	return -EIO;
}

static struct ts_bus_info ts_bus_i2c_info = {
	.btype = TS_BUS_I2C,
	.bus_write = ts_i2c_write,
	.bus_read = ts_i2c_read,
};

static int ts_spi_read(u8 *reg_addr, u16 reg_len, u8 *buf, u16 len)
{
	return NO_ERR;
}

static int ts_spi_write(u8 *buf, u16 length)
{
	return NO_ERR;
}

static struct ts_bus_info ts_bus_spi_info = {
	.btype = TS_BUS_SPI,
	.bus_write = ts_spi_write,
	.bus_read = ts_spi_read,
};

int put_one_cmd_direct_sync(struct ts_cmd_node *cmd, int timeout)
{
	int error = NO_ERR;

	ts_log_info("%s Enter\n", __func__);

	if (atomic_read(&g_ts_data.state) == TS_UNINIT) {
		error = -EIO;
		return error;
	}
	if ((atomic_read(&g_ts_data.state) == TS_SLEEP) ||
		(atomic_read(&g_ts_data.state) == TS_WORK_IN_SLEEP)) {
		ts_log_info("%s In Sleep State\n", __func__);
		error = -EIO;
	}
	return error;
}

int put_cmd_process(struct ts_cmd_queue *q, struct ts_cmd_sync *sync,
	unsigned long flags, struct ts_cmd_node *cmd, int timeout)
{
	spin_lock_irqsave(&q->spin_lock, flags);
	smp_wmb();
	if (q->cmd_count == q->queue_size) {
		spin_unlock_irqrestore(&q->spin_lock, flags);
		ts_log_err("queue is full\n");
		WARN_ON(1);
		return NO_ERR;
	}
	memcpy(&q->ring_buff[q->wr_index], cmd, sizeof(struct ts_cmd_node));
	q->cmd_count++;
	q->wr_index++;
	q->wr_index %= q->queue_size;
	smp_mb();
	spin_unlock_irqrestore(&q->spin_lock, flags);
	ts_log_debug("put one cmd :%d in ring buff\n", cmd->command);
	wake_up_process(g_ts_data.ts_task); /* wakeup process */

	if (timeout &&
		!(wait_for_completion_timeout(&sync->done,
		abs(timeout) * HZ))) {
		atomic_set(&sync->timeout_flag, TS_TIMEOUT);
		ts_log_err("wait for cmd respone timeout\n");
		return -EBUSY;
	}
	smp_wmb();

	return NO_ERR;
}

int put_one_cmd(struct ts_cmd_node *cmd, int timeout)
{
	int error = -EIO;
	unsigned long flags;
	struct ts_cmd_queue *q = NULL;
	struct ts_cmd_sync *sync = NULL;

	if (!cmd) {
		ts_log_err("find null pointer\n");
		goto out;
	}

	if (atomic_read(&g_ts_data.state) == TS_UNINIT) {
		ts_log_err("ts module not initialize\n");
		goto out;
	}

	if (timeout) {
		sync = (struct ts_cmd_sync *)kzalloc(sizeof(struct ts_cmd_sync),
			GFP_KERNEL);
		if (sync == NULL) {
			ts_log_err("failed to kzalloc completion\n");
			error = -ENOMEM;
			goto out;
		}
		init_completion(&sync->done);
		atomic_set(&sync->timeout_flag, TS_NOT_TIMEOUT);
		cmd->sync = sync;
	} else {
		cmd->sync = NULL;
	}

	if ((g_ts_data.chip_data->is_direct_proc_cmd) &&
		(g_ts_data.chip_data->is_can_device_use_int == false)) {
		if (cmd->command == TS_INT_PROCESS)
			goto out;
		q = &g_ts_data.no_int_queue;
	} else {
		q = &g_ts_data.queue;
	}

	error = put_cmd_process(q, sync, flags, cmd, timeout);
	if (error)
		goto out;

	kfree(sync);
out:
	return error;
}

static int get_one_cmd(struct ts_cmd_node *cmd)
{
	unsigned long flags;
	int error = -EIO;
	struct ts_cmd_queue *q = NULL;

	if (unlikely(!cmd)) {
		ts_log_err("find null pointer\n");
		goto out;
	}

	q = &g_ts_data.queue;

	spin_lock_irqsave(&q->spin_lock, flags);
	smp_wmb();
	if (!q->cmd_count) {
		ts_log_debug("queue is empty\n");
		spin_unlock_irqrestore(&q->spin_lock, flags);
		goto out;
	}
	memcpy(cmd, &q->ring_buff[q->rd_index], sizeof(struct ts_cmd_node));
	q->cmd_count--;
	q->rd_index++;
	q->rd_index %= q->queue_size;
	smp_mb();
	spin_unlock_irqrestore(&q->spin_lock, flags);
	ts_log_debug("get one cmd :%d from ring buff\n", cmd->command);
	error = NO_ERR;

out:
	return error;
}

int ts_get_esd_status(void)
{
	int ret;

	ret = atomic_read(&g_ts_data.ts_esd_state);

	return ret;
}
EXPORT_SYMBOL(ts_get_esd_status);

void ts_clear_esd_status(void)
{
	atomic_set(&g_ts_data.ts_esd_state, TS_NO_ESD);
}
EXPORT_SYMBOL(ts_clear_esd_status);

static void ts_send_init_cmd(void)
{
	int error;
	struct ts_cmd_node cmd;

	ts_log_info("%s Enter\n", __func__);

	cmd.command = TS_TP_INIT;
	if (g_ts_data.chip_data->is_direct_proc_cmd) {
		g_ts_data.chip_data->is_can_device_use_int = true;
		error = put_one_cmd(&cmd, NO_SYNC_TIMEOUT);
		if (error) {
			ts_log_err("put cmd error :%d\n", error);
			return;
		}
	} else {
		ts_log_err("%s, nothing to do\n", __func__);
	}
}

static void proc_init_cmd(void)
{
	schedule_work(&tp_init_work);
}

static ssize_t ts_loglevel_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int error = NO_ERR;

	ts_log_info("%s called\n", __func__);
	if (dev == NULL) {
		ts_log_err("dev is null\n");
		error = -EINVAL;
		goto out;
	}

	error = snprintf(buf, MAX_STR_LEN, "%d\n", g_ts_log_cfg);

out:
	ts_log_info("%s:done\n", __func__);
	return error;
}

static ssize_t ts_loglevel_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int value;
	int error;
	struct ts_device_data *dev_data = g_ts_data.chip_data;

	ts_log_info("%s:called\n", __func__);

	if (dev == NULL) {
		ts_log_err("dev is null\n");
		error = -EINVAL;
		goto out;
	}

	error = sscanf(buf, "%u", &value);
	if (!error) {
		ts_log_err("sscanf return invaild :%d\n", error);
		error = -EINVAL;
		goto out;
	}
	ts_log_debug("sscanf value is %u\n", value);
	g_ts_log_cfg = value;
	error = count;

	if (dev_data->ops->chip_debug_switch)
		dev_data->ops->chip_debug_switch(g_ts_log_cfg);

out:
	ts_log_info("%s:done\n", __func__);
	return error;
}

static DEVICE_ATTR(touch_oem_info, 0644, NULL, NULL);
static DEVICE_ATTR(touch_chip_info, 0644, NULL, NULL);
static DEVICE_ATTR(calibration_info, 0644, NULL, NULL);
static DEVICE_ATTR(calibrate, 0440, NULL, NULL);
static DEVICE_ATTR(calibrate_wakeup_gesture, 0400, NULL, NULL);
static DEVICE_ATTR(touch_glove, 0600, NULL, NULL);
static DEVICE_ATTR(touch_sensitivity, 0600, NULL, NULL);
static DEVICE_ATTR(hand_detect, 0400, NULL, NULL);
static DEVICE_ATTR(loglevel, 0600, ts_loglevel_show,
	ts_loglevel_store);
static DEVICE_ATTR(supported_func_indicater, 0440, NULL, NULL);
static DEVICE_ATTR(touch_window, 0600, NULL, NULL);
static DEVICE_ATTR(fw_update_sd, 0200, NULL, NULL);
static DEVICE_ATTR(reset, 0200, NULL, NULL);
static DEVICE_ATTR(easy_wakeup_gesture, 0600, NULL, NULL);
static DEVICE_ATTR(wakeup_gesture_enable, 0600, NULL, NULL);
static DEVICE_ATTR(touch_dsm_debug, 0444, NULL, NULL);
static DEVICE_ATTR(easy_wakeup_control, 0200, NULL, NULL);
static DEVICE_ATTR(easy_wakeup_position, 0400, NULL, NULL);
static DEVICE_ATTR(touch_register_operation, 0660, NULL, NULL);
static DEVICE_ATTR(roi_enable, 0660, NULL, NULL);
static DEVICE_ATTR(roi_data, 0440, NULL, NULL);
static DEVICE_ATTR(roi_data_debug, 0440, NULL, NULL);
static DEVICE_ATTR(tp_capacitance_test_type, 0600, NULL, NULL);
static DEVICE_ATTR(tp_capacitance_test_config, 0400, NULL, NULL);
static DEVICE_ATTR(touch_rawdata_debug, 0660, NULL, NULL);
static DEVICE_ATTR(touch_special_hardware_test, 0660, NULL, NULL);
static DEVICE_ATTR(touch_wideth, 0660, NULL, NULL);
static DEVICE_ATTR(anti_false_touch_param, 0660, NULL, NULL);

static struct attribute *ts_attributes[] = {
	&dev_attr_touch_oem_info.attr,
	&dev_attr_touch_chip_info.attr,
	&dev_attr_calibration_info.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_wakeup_gesture.attr,
	&dev_attr_touch_glove.attr,
	&dev_attr_touch_sensitivity.attr,
	&dev_attr_hand_detect.attr,
	&dev_attr_loglevel.attr,
	&dev_attr_supported_func_indicater.attr,
	&dev_attr_touch_window.attr,
	&dev_attr_fw_update_sd.attr,
	&dev_attr_reset.attr,
	&dev_attr_easy_wakeup_gesture.attr,
	&dev_attr_wakeup_gesture_enable.attr,
	&dev_attr_touch_dsm_debug.attr,
	&dev_attr_easy_wakeup_control.attr,
	&dev_attr_easy_wakeup_position.attr,
	&dev_attr_touch_register_operation.attr,
	&dev_attr_roi_enable.attr,
	&dev_attr_roi_data.attr,
	&dev_attr_roi_data_debug.attr,
	&dev_attr_tp_capacitance_test_type.attr,
	&dev_attr_tp_capacitance_test_config.attr,
	&dev_attr_touch_rawdata_debug.attr,
	&dev_attr_touch_special_hardware_test.attr,
	&dev_attr_touch_wideth.attr,
	&dev_attr_anti_false_touch_param.attr,
	NULL
};

static const struct attribute_group g_ts_attr_group = {
	.attrs = ts_attributes,
};

static void ts_watchdog_work(struct work_struct *work)
{
	int error;
	struct ts_cmd_node cmd;

	ts_log_debug("%s\n", __func__);
	cmd.command = TS_CHECK_STATUS;

	error = put_one_cmd(&cmd, NO_SYNC_TIMEOUT);
	if (error)
		ts_log_err("put TS_CHECK_STATUS cmd error :%d\n", error);
}

static void ts_watchdog_timer(unsigned long data)
{
	struct ts_data *cd = (struct ts_data *)data;

	ts_log_debug("Timer triggered\n");

	if (!cd)
		return;

	if (!work_pending(&cd->watchdog_work))
		schedule_work(&cd->watchdog_work);
}

static void ts_start_wd_timer(struct ts_data *cd)
{
	if (!cd->chip_data->need_wd_check_status)
		return;

	if (!TS_WATCHDOG_TIMEOUT)
		return;

	ts_log_debug("start wd\n");
	if (cd->chip_data->watchdog_timeout) {
		mod_timer(&cd->watchdog_timer, jiffies +
			msecs_to_jiffies(cd->chip_data->watchdog_timeout));
	} else {
		mod_timer(&cd->watchdog_timer, jiffies +
			msecs_to_jiffies(TS_WATCHDOG_TIMEOUT));
	}
}

static void ts_stop_wd_timer(struct ts_data *cd)
{
	if (!cd->chip_data->need_wd_check_status)
		return;

	if (!TS_WATCHDOG_TIMEOUT)
		return;

	ts_log_debug("stop wd\n");
	del_timer(&cd->watchdog_timer);
	cancel_work_sync(&cd->watchdog_work);
	del_timer(&cd->watchdog_timer);
}

void ts_power_control_notify(enum ts_pm_type pm_type, int timeout)
{
	int error;
	struct ts_cmd_node cmd;

	if (atomic_read(&g_ts_data.state) == TS_UNINIT) {
		ts_log_info("ts is not init");
		return;
	}
	cmd.command = TS_POWER_CONTROL;
	cmd.cmd_param.pub_params.pm_type = pm_type;
	error = put_one_cmd(&cmd, timeout);
	if (error)
		ts_log_err("%s:put cmd error :%d\n",
			__func__, error);

	return;
}

static void ts_power_resume(unsigned long event, const int *blank)
{
	unsigned char ts_state;
	int times = 0;

	while (1) {
		ts_state = atomic_read(&g_ts_data.state);
		if ((ts_state == TS_SLEEP) || (ts_state == TS_WORK_IN_SLEEP)) {
			ts_power_control_notify(TS_RESUME_DEVICE,
				SHORT_SYNC_TIMEOUT);
			ts_power_control_notify(TS_AFTER_RESUME,
				NO_SYNC_TIMEOUT);
			break;
		} else {
			msleep(TS_FB_WAIT_TIME);
			if (times++ > TS_FB_LOOP_COUNTS) {
				times = 0;
				ts_log_err("no resume");
				ts_log_err("blank: %d, event:%lu, state: %d\n",
					*blank, event, ts_state);
				break;
			}
		}
	}
}

static void ts_power_suspend(unsigned long event, const int *blank)
{
	unsigned char ts_state;
	int times = 0;

	switch (event) {
	case FB_EARLY_EVENT_BLANK:
		while (times < TS_FB_LOOP_COUNTS) {
			ts_state = atomic_read(&g_ts_data.state);
			if ((ts_state == TS_WORK) ||
				(ts_state == TS_WORK_IN_SLEEP)) {
				ts_power_control_notify(
					TS_BEFORE_SUSPEND,
					SHORT_SYNC_TIMEOUT);
					break;
			} else {
				msleep(TS_FB_WAIT_TIME);
				times++;
			}
		}
		break;
	case FB_EVENT_BLANK:
		while (times < TS_FB_LOOP_COUNTS) {
			ts_state = atomic_read(&g_ts_data.state);
			if ((ts_state == TS_WORK) ||
				(ts_state == TS_WORK_IN_SLEEP)) {
				ts_power_control_notify(
					TS_SUSPEND_DEVICE,
					NO_SYNC_TIMEOUT);
				break;
			} else {
				msleep(TS_FB_WAIT_TIME);
				times++;
			}
		}
		break;
	default:
		break;
	}
}

static int fb_notifier_callback(struct notifier_block *self,
	unsigned long event, void *data)
{
	int error = NO_ERR;
	int i;
	struct fb_event *fb_event = data;
	int *blank = fb_event->data;

	/* only need process event FB_EARLY_EVENT_BLANK\FB_EVENT_BLANK */
	if (!(event == FB_EARLY_EVENT_BLANK || event == FB_EVENT_BLANK)) {
		ts_log_debug("event(%lu) do not need process\n", event);
		return NO_ERR;
	}

	for (i = 0; i < FB_MAX; i++) {
		if (registered_fb[i] == fb_event->info) {
			if (i == 0) {
				ts_log_debug("Ts-index:%d, go on !\n", i);
				break;
			}
			ts_log_info("Ts-index:%d, exit !\n", i);
			return error;
		}
	}
	ts_log_info("%s, blank: %d, event:%lu, state: %d\n", __func__,
		*blank, event, atomic_read(&g_ts_data.state));
	switch (*blank) {
	case FB_BLANK_UNBLANK:
		/* resume device */
		switch (event) {
		case FB_EARLY_EVENT_BLANK:
			ts_log_debug("resume: event = %lu, not care\n", event);
			break;
		case FB_EVENT_BLANK:
			ts_power_resume(event, blank);
			break;
		default:
			break;
		}
		break;
	case FB_BLANK_VSYNC_SUSPEND:
	case FB_BLANK_HSYNC_SUSPEND:
	case FB_BLANK_NORMAL:
	case FB_BLANK_POWERDOWN:
	default:
		ts_power_suspend(event, blank);
		break;
	}

	return NO_ERR;
}

static irqreturn_t ts_irq_handler(int irq, void *dev_id)
{
	int error = NO_ERR;
	struct ts_cmd_node cmd;

	if (g_ts_data.chip_data->ops->chip_irq_top_half)
		error = g_ts_data.chip_data->ops->chip_irq_top_half(&cmd);

	/* unexpected error happen, put err cmd to ts thread */
	if (error)
		cmd.command = TS_INT_ERR_OCCUR;
	else
		cmd.command = TS_INT_PROCESS;

	disable_irq_nosync(g_ts_data.irq_id);

	if (put_one_cmd(&cmd, NO_SYNC_TIMEOUT) &&
		(atomic_read(&g_ts_data.state) != TS_UNINIT))
		enable_irq(g_ts_data.irq_id);

	return IRQ_HANDLED;
}

static void ts_proc_bottom_half(struct ts_cmd_node *in_cmd,
	struct ts_cmd_node *out_cmd)
{
	struct ts_device_data *dev = g_ts_data.chip_data;

	ts_log_debug("bottom half called\n");
	atomic_set(&g_data_report_over, 0);
	/* related event need process, use out cmd to notify */
	if (dev->ops->chip_irq_bottom_half)
		dev->ops->chip_irq_bottom_half(in_cmd, out_cmd);
}

static void ts_work_after_input(void)
{
	struct ts_device_data *dev = g_ts_data.chip_data;

	if (dev->ops->chip_work_after_input)
		dev->ops->chip_work_after_input();
}

static void ts_film_touchplus(struct ts_fingers *finger, int finger_num,
	struct input_dev *input_dev)
{
	static int pre_special_button_key = TS_TOUCHPLUS_INVALID;
	int key_max = TS_TOUCHPLUS_KEY2;
	int key_min = TS_TOUCHPLUS_KEY3;
	unsigned char ts_state;

	ts_log_debug("%s called\n", __func__);

	/* discard touchplus report in gesture wakeup mode */
	ts_state = atomic_read(&g_ts_data.state);
	if ((ts_state == TS_SLEEP) || (ts_state == TS_WORK_IN_SLEEP))
		return;
	if ((finger_num != 0) || (finger->special_button_key > key_max) ||
		(finger->special_button_key < key_min)) {
		if (finger->special_button_flag != 0) {
			input_report_key(input_dev,
				finger->special_button_key, 0);
			input_sync(input_dev);
		}
		return;
	}

	if (finger->special_button_flag == 1) {
		input_report_key(input_dev, finger->special_button_key,
			finger->special_button_flag);
		input_sync(input_dev);
	} else if ((finger->special_button_flag == 0) &&
		(pre_special_button_key == finger->special_button_key)) {
		input_report_key(input_dev, finger->special_button_key,
			finger->special_button_flag);
		input_sync(input_dev);
	} else if ((finger->special_button_flag == 0) &&
		(pre_special_button_key != finger->special_button_key)) {
		input_report_key(input_dev, pre_special_button_key, 0);
		input_sync(input_dev);
	}
	pre_special_button_key = finger->special_button_key;
}

static void ts_report_input(struct ts_cmd_node *in_cmd,
	struct ts_cmd_node *out_cmd)
{
	struct ts_fingers *finger = &in_cmd->cmd_param.pub_params.report_info;
	struct input_dev *input_dev = g_ts_data.input_dev;
	int finger_num = 0;
	int id;

	ts_log_debug("%s enter\n", __func__);
	for (id = 0; id < TS_MAX_FINGER; id++) {
		if (finger->fingers[id].status == 0) {
			ts_log_debug("never touch before: id is %d\n", id);
			continue;
		}
		if (finger->fingers[id].status == TS_FINGER_PRESS) {
			finger_num++;
			input_report_abs(input_dev, ABS_MT_PRESSURE,
				finger->fingers[id].pressure);
			input_report_abs(input_dev, ABS_MT_POSITION_X,
				finger->fingers[id].x);
			input_report_abs(input_dev, ABS_MT_POSITION_Y,
				finger->fingers[id].y);
			input_report_abs(input_dev, ABS_MT_TRACKING_ID, id);
			input_mt_sync(input_dev);
		} else if (finger->fingers[id].status == TS_FINGER_RELEASE) {
			ts_log_debug("up: id is %d, status = %d\n", id,
				finger->fingers[id].status);
			input_mt_sync(input_dev);
		}
	}

	input_report_key(input_dev, BTN_TOUCH, finger_num);
	input_sync(input_dev);

	ts_film_touchplus(finger, finger_num, input_dev);
	ts_log_debug("report input done, finger_num = %d\n", finger_num);
	ts_work_after_input(); /* do some delayed works */
	atomic_set(&g_data_report_over, 1);
}

void send_up_msg_in_resume(void)
{
	struct input_dev *input_dev = g_ts_data.input_dev;

	input_report_key(input_dev, BTN_TOUCH, 0);
	input_mt_sync(input_dev);
	input_sync(input_dev);
	ts_log_debug("%s\n", __func__);
}

static int ts_power_control(int irq_id,
	struct ts_cmd_node *in_cmd,
	struct ts_cmd_node *out_cmd)
{
	int error = NO_ERR;
	enum ts_pm_type pm_type = in_cmd->cmd_param.pub_params.pm_type;
	struct ts_device_data *dev = g_ts_data.chip_data;

	switch (pm_type) {
	case TS_BEFORE_SUSPEND:
		ts_stop_wd_timer(&g_ts_data);
		disable_irq(irq_id);
		if (dev->ops->chip_before_suspend)
			error = dev->ops->chip_before_suspend();
		break;
	case TS_SUSPEND_DEVICE:
		atomic_set(&g_ts_data.state, TS_SLEEP);
		if (dev->ops->chip_suspend)
			error = dev->ops->chip_suspend();
		break;
	case TS_IC_SHUT_DOWN:
		disable_irq(irq_id);
		if (dev->ops->chip_shutdown)
			dev->ops->chip_shutdown();
		break;
	case TS_RESUME_DEVICE:
		if (dev->ops->chip_resume)
			error = dev->ops->chip_resume();
		break;
	case TS_AFTER_RESUME:
		if (dev->ops->chip_after_resume)
			error = dev->ops->chip_after_resume(
				(void *)&g_ts_data.feature_info);
		send_up_msg_in_resume();
		atomic_set(&g_ts_data.state, TS_WORK);
		enable_irq(irq_id);
		ts_start_wd_timer(&g_ts_data);
		break;
	default:
		ts_log_err("pm_type = %d\n", pm_type);
		error = -EINVAL;
		break;
	}
	return error;
}

static int ts_fw_update_boot(struct ts_cmd_node *in_cmd,
	struct ts_cmd_node *out_cmd)
{
	char *fw_name = in_cmd->cmd_param.pub_params.firmware_info.fw_name;
	int error = NO_ERR;
	struct ts_device_data *dev = g_ts_data.chip_data;

	if (dev->ops->chip_fw_update_boot)
		error = dev->ops->chip_fw_update_boot(fw_name);
	ts_log_info("process firmware update boot, return value:%d\n", error);

	return error;
}

static int ts_int_err_process(struct ts_cmd_node *in_cmd,
	struct ts_cmd_node *out_cmd)
{
	int error = NO_ERR;
	struct ts_device_data *dev = g_ts_data.chip_data;

	if (dev->ops->chip_reset)
		error = dev->ops->chip_reset();

	if (error) {
		out_cmd->command = TS_ERR_OCCUR;
		goto out;
	}

out:
	return error;
}

static int ts_err_process(struct ts_cmd_node *in_cmd,
	struct ts_cmd_node *out_cmd)
{
	static int error_count;
	int error = NO_ERR;
	struct ts_device_data *dev = g_ts_data.chip_data;

	ts_log_info("error process\n");

	if (dev->ops->chip_reset)
		error = dev->ops->chip_reset();
	if (error) {
		out_cmd->command = TS_ERR_OCCUR;
		goto out;
	}

	error_count = 0;
out:
	return error;
}

static void ts_algo_calibrate(struct ts_cmd_node *in_cmd,
	struct ts_cmd_node *out_cmd)
{
	int id;
	int algo_size = g_ts_data.chip_data->algo_size;
	u32 order = in_cmd->cmd_param.pub_params.algo_param.algo_order;
	struct ts_fingers *in_finger =
		&in_cmd->cmd_param.pub_params.algo_param.info;
	struct ts_fingers *out_finger =
		&out_cmd->cmd_param.pub_params.algo_param.info;
	struct ts_algo_func *algo = NULL;

	if (!algo_size) {
		ts_log_info("no algo handler, direct report\n");
		goto out;
	}

	ts_log_debug("algo order: %d, algo_size :%d\n", order, algo_size);
	for (id = 0; id < algo_size; id++) {
		if (order & BIT_MASK(id)) {
			ts_log_debug("algo id:%d is setted\n", id);
			list_for_each_entry(algo,
					&g_ts_data.chip_data->algo_head,
					node) {
				/* found the right algo func */
				if (algo->algo_index == id) {
					ts_log_debug("algo :%s called\n",
						algo->algo_name);
					algo->chip_algo_func(
						g_ts_data.chip_data,
						in_finger,
						out_finger);
					memcpy(in_finger, out_finger,
						sizeof(struct ts_fingers));
					break;
				}
			}
		}
	}

out:
	memcpy(&out_cmd->cmd_param.pub_params.report_info, in_finger,
		sizeof(struct ts_fingers));
	out_cmd->command = TS_REPORT_INPUT;
}

static int ts_check_status(struct ts_cmd_node *in_cmd,
	struct ts_cmd_node *out_cmd)
{
	int error = NO_ERR;
	struct ts_device_data *dev = g_ts_data.chip_data;

	if (dev->ops->chip_check_status)
		error = dev->ops->chip_check_status();
	if (error)
		out_cmd->command = TS_ERR_OCCUR;

	ts_start_wd_timer(&g_ts_data);
	return error;
}

static void ts_proc_command_process(struct ts_cmd_node *proc_cmd,
	struct ts_cmd_node *out_cmd)
{
	switch (proc_cmd->command) {
	case TS_INT_PROCESS:
		ts_proc_bottom_half(proc_cmd, out_cmd);
		enable_irq(g_ts_data.irq_id);
		break;
	case TS_INPUT_ALGO:
		ts_algo_calibrate(proc_cmd, out_cmd);
		break;
	case TS_REPORT_INPUT:
		ts_report_input(proc_cmd, out_cmd);
		break;
	case TS_POWER_CONTROL:
		ts_power_control(g_ts_data.irq_id, proc_cmd, out_cmd);
		break;
	case TS_FW_UPDATE_BOOT:
		disable_irq(g_ts_data.irq_id);
		ts_fw_update_boot(proc_cmd, out_cmd);
		enable_irq(g_ts_data.irq_id);
		break;
	case TS_CHECK_STATUS:
		ts_check_status(proc_cmd, out_cmd);
		break;
	case TS_TP_INIT:
		proc_init_cmd();
		break;
	case TS_ERR_OCCUR:
		disable_irq(g_ts_data.irq_id);
		ts_stop_wd_timer(&g_ts_data);
		ts_err_process(proc_cmd, out_cmd);
		ts_start_wd_timer(&g_ts_data);
		enable_irq(g_ts_data.irq_id);
		break;
	case TS_INT_ERR_OCCUR:
		ts_stop_wd_timer(&g_ts_data);
		ts_int_err_process(proc_cmd, out_cmd);
		enable_irq(g_ts_data.irq_id);
		ts_start_wd_timer(&g_ts_data);
		break;
	default:
		break;
	}
}

static bool ts_cmd_need_process(struct ts_cmd_node *cmd)
{
	bool is_need_process = true;
	struct ts_cmd_sync *sync;
	enum ts_pm_type pm_type = cmd->cmd_param.pub_params.pm_type;

	sync = cmd->sync;

	if (unlikely(atomic_read(&g_ts_data.state) == TS_SLEEP)) {
		switch (cmd->command) {
		case TS_POWER_CONTROL:
			if ((pm_type != TS_RESUME_DEVICE) &&
			(pm_type != TS_AFTER_RESUME))
				is_need_process = false;
			break;
		case TS_INT_PROCESS:
		case TS_INT_ERR_OCCUR:
			is_need_process = false;
			break;
		default:
			break;
		}
	} else if (unlikely(atomic_read(&g_ts_data.state) ==
		TS_WORK_IN_SLEEP)) {
		switch (cmd->command) {
		case TS_POWER_CONTROL:
			if ((pm_type != TS_RESUME_DEVICE) &&
				(pm_type != TS_AFTER_RESUME))
				is_need_process = false;
			break;
		default:
			is_need_process = true;
			break;
		}
	}

	if (!is_need_process && sync) {
		if (atomic_read(&sync->timeout_flag) == TS_TIMEOUT)
			kfree(sync);
		else
			complete(&sync->done);
	}

	return is_need_process;
}

static int ts_proc_command_directly(struct ts_cmd_node *cmd)
{
	int error = NO_ERR;
	struct ts_cmd_node outcmd;

	ts_log_info("%s Enter\n", __func__);
	/* Do not use cmd->sync in this func, setting it as null */
	cmd->sync = NULL;
	if (!ts_cmd_need_process(cmd)) {
		ts_log_info("%s, no need to process cmd:%d",
			__func__, cmd->command);
		error = -EIO;
		goto out;
	}
	mutex_lock(&g_ts_data.chip_data->device_call_lock);
	switch (cmd->command) {
	case TS_INT_PROCESS:
	case TS_INPUT_ALGO:
	case TS_REPORT_INPUT:
	case TS_POWER_CONTROL:
	case TS_FORCE_RESET:
	case TS_INT_ERR_OCCUR:
	case TS_ERR_OCCUR:
	case TS_CHECK_STATUS:
		ts_log_err("%s, command %d does not support direct call!",
			__func__, cmd->command);
		break;
	case TS_FW_UPDATE_BOOT:
		error = ts_fw_update_boot(cmd, &outcmd);
		break;
	default:
		ts_log_err("%s, command %d unknown!", __func__, cmd->command);
		break;
	}
	mutex_unlock(&g_ts_data.chip_data->device_call_lock);
	ts_log_debug("%s:command :%d process result:%d\n",
		__func__, cmd->command, error);
out:
	return error;
}

static void ts_proc_command(struct ts_cmd_node *cmd)
{
	int error = NO_ERR;
	struct ts_cmd_sync *sync = cmd->sync;
	struct ts_cmd_node *proc_cmd = cmd;
	struct ts_cmd_node *out_cmd = &pang_cmd_buff;

	/* discard timeout cmd to fix panic */
	if (sync && atomic_read(&sync->timeout_flag) == TS_TIMEOUT) {
		kfree(sync);
		return;
	}

	if (!ts_cmd_need_process(proc_cmd)) {
		ts_log_info("no need to process cmd:%d", proc_cmd->command);
		return;
	}

related_proc:
	out_cmd->command = TS_INVAILD_CMD;
	ts_proc_command_process(proc_cmd, out_cmd);
	ts_log_debug("command :%d process result:%d\n", proc_cmd->command,
		error);

	if (out_cmd->command != TS_INVAILD_CMD) {
		ts_log_debug("related command :%d need process\n",
			out_cmd->command);
		swap(proc_cmd, out_cmd); /* ping - pang */
		goto related_proc;
	}

	if (sync) { /* notify wait threads by completion */
		smp_mb();
		ts_log_debug("wakeup threads in waitqueue\n");
		if (atomic_read(&sync->timeout_flag) == TS_TIMEOUT)
			kfree(sync);
		else
			complete(&sync->done);
	}
}

static bool ts_task_continue(void)
{
	bool task_continue = true;
	unsigned long flags;

repeat:
	if (unlikely(kthread_should_stop())) {
		task_continue = false;
		goto out;
	}
	spin_lock_irqsave(&g_ts_data.queue.spin_lock, flags);
	smp_wmb();
	if (g_ts_data.queue.cmd_count) {
		set_current_state(TASK_RUNNING);
		ts_log_debug("ts task state to  TASK_RUNNING\n");
		goto out_unlock;
	} else {
		set_current_state(TASK_INTERRUPTIBLE);
		ts_log_debug("ts task state to  TASK_INTERRUPTIBLE\n");
		spin_unlock_irqrestore(&g_ts_data.queue.spin_lock, flags);
		schedule();
		goto repeat;
	}

out_unlock:
	spin_unlock_irqrestore(&g_ts_data.queue.spin_lock, flags);
out:
	return task_continue;
}

static int ts_parse_config(void)
{
	int error = NO_ERR;
	int rc;
	int index;
	char *tmp_buff = NULL;

	if (g_ts_data.node) {
		rc = of_property_read_string(g_ts_data.node, "product",
			(const char **)&tmp_buff);
		if (rc) {
			ts_log_err("product read failed:%d\n", rc);
			error = -EINVAL;
			goto out;
		}

		rc = of_property_read_u32(g_ts_data.node, "fpga_flag",
			&g_ts_data.fpga_flag);
		if (rc) {
			ts_log_err("fpga flag read failed:%d, set to 0\n", rc);
			g_ts_data.fpga_flag = 0;
		}
	}

	if (!tmp_buff) {
		ts_log_err("tmp_buff failed\n");
		error = -EINVAL;
		goto out;
	}

	for (index = 0; tmp_buff[index] && index < strlen(tmp_buff); index++)
		g_ts_data.product_name[index] = tolower(tmp_buff[index]);

	if (index == MAX_STR_LEN)
		g_ts_data.product_name[MAX_STR_LEN - 1] = '\0';

	ts_log_info("parse product name :%s\n", g_ts_data.product_name);

out:
	return error;
}

static void tp_init_work_fn(struct work_struct *work)
{
	struct ts_cmd_node use_cmd;
	struct ts_cmd_queue *q;
	struct ts_cmd_node *cmd = &use_cmd;
	struct ts_device_data *dev = g_ts_data.chip_data;
	int i = TS_CMD_QUEUE_SIZE;
	int error = NO_ERR;
	unsigned long flags;

	q = &g_ts_data.no_int_queue;
	mutex_lock(&g_ts_data.chip_data->device_call_lock);
	if (dev->ops->chip_init) {
		ts_log_info("%s, call chip init\n", __func__);
		error = dev->ops->chip_init();
	}
	mutex_unlock(&g_ts_data.chip_data->device_call_lock);
	if (error != NO_ERR) {
		ts_log_err("%s,chip init fail with error:%d\n",
			__func__, error);
		return;
	}
	ts_log_info("%s, chip init done\n", __func__);
	while (i-- > 0) {
		spin_lock_irqsave(&q->spin_lock, flags);
		smp_wmb();
		if (!q->cmd_count) {
			ts_log_debug("queue is empty\n");
			spin_unlock_irqrestore(&q->spin_lock, flags);
			break;
		}
		q->cmd_count--;
		q->rd_index++;
		q->rd_index %= q->queue_size;
		smp_mb();
		spin_unlock_irqrestore(&q->spin_lock, flags);
		error = ts_proc_command_directly(cmd);
		if (error != NO_ERR)
			ts_log_info("%s process init cmd %d error",
				__func__, cmd->command);
	}
}

static int ts_chip_init(void)
{
	int error = NO_ERR;
	struct ts_device_data *dev = g_ts_data.chip_data;

	ts_log_info("%s:called\n", __func__);
	mutex_init(&easy_wake_guesure_lock);
	if (g_ts_data.chip_data->is_direct_proc_cmd == 0) {
		if (dev->ops->chip_init)
			error = dev->ops->chip_init();
	}
	if (error)
		ts_log_err("chip init failed\n");

	if (g_ts_data.chip_data->is_direct_proc_cmd) {
		g_ts_data.chip_data->is_can_device_use_int = false;
		g_ts_data.no_int_queue.rd_index = 0;
		g_ts_data.no_int_queue.wr_index = 0;
		g_ts_data.no_int_queue.cmd_count = 0;
		g_ts_data.no_int_queue.queue_size = TS_CMD_QUEUE_SIZE;
		spin_lock_init(&g_ts_data.no_int_queue.spin_lock);
		INIT_WORK(&tp_init_work, tp_init_work_fn);
	}
	return error;
}

int register_algo_func(struct ts_device_data *chip_data,
	struct ts_algo_func *fn)
{
	int error = -EIO;

	if (!chip_data || !fn)
		goto out;

	fn->algo_index = chip_data->algo_size;
	list_add_tail(&fn->node, &chip_data->algo_head);
	chip_data->algo_size++;
	smp_mb();
	error = NO_ERR;

out:
	return error;
}

static int ts_register_algo(void)
{
	int error;
	struct ts_device_data *dev = g_ts_data.chip_data;

	ts_log_info("register algo called\n");

	error = ts_register_algo_func(dev);

	return error;
}

static void ts_ic_shutdown(void)
{
	struct ts_device_data *dev = g_ts_data.chip_data;

	if (dev->ops->chip_shutdown)
		dev->ops->chip_shutdown();
}

static int ts_set_irq(void)
{
	int ret;
	unsigned int irq_flags;

	g_ts_data.irq_id = g_ts_data.client->irq =
		gpio_to_irq(g_ts_data.chip_data->irq_gpio);
	tp_gpio_num = g_ts_data.chip_data->irq_gpio;
	switch (g_ts_data.chip_data->irq_config) {
	case TS_IRQ_LOW_LEVEL:
		irq_flags = IRQF_TRIGGER_LOW;
		break;
	case TS_IRQ_HIGH_LEVEL:
		irq_flags = IRQF_TRIGGER_HIGH;
		break;
	case TS_IRQ_RAISE_EDGE:
		irq_flags = IRQF_TRIGGER_RISING;
		break;
	case TS_IRQ_FALL_EDGE:
		irq_flags = IRQF_TRIGGER_FALLING;
		break;
	default:
		ts_log_err("ts irq_config invaild\n");
		return -EINVAL;
	}

	atomic_set(&g_ts_data.state, TS_WORK);
	ret = request_irq(g_ts_data.irq_id, ts_irq_handler,
		irq_flags | IRQF_NO_SUSPEND, "ts", &g_ts_data);
	if (ret) {
		ts_log_err("ts request_irq failed\n");
		return -EINVAL;
	}

	return NO_ERR;
}

static int ts_input_open(struct input_dev *dev)
{
	ts_log_debug("input_open called:%d\n", dev->users);
	return NO_ERR;
}

static void ts_input_close(struct input_dev *dev)
{
	ts_log_debug("input_close called:%d\n", dev->users);
}

static int ts_register_input(struct input_dev *input_dev)
{
	int error = NO_ERR;

	input_dev = input_allocate_device();
	if (!input_dev) {
		ts_log_err("failed to allocate memory for input dev\n");
		error = -ENOMEM;
		return -EINVAL;
	}

	input_dev->name = TS_DEV_NAME;
	if (g_ts_data.bops->btype == TS_BUS_I2C)
		input_dev->id.bustype = BUS_I2C;
	else if (g_ts_data.bops->btype == TS_BUS_SPI)
		input_dev->id.bustype = BUS_SPI;
	input_dev->dev.parent = &g_ts_data.ts_dev->dev;
	input_dev->open = ts_input_open;
	input_dev->close = ts_input_close;
	g_ts_data.input_dev = input_dev;

	if (g_ts_data.chip_data->ops->chip_input_config)
		error = g_ts_data.chip_data->ops->chip_input_config(
			g_ts_data.input_dev);
	if (error)
		return -EINVAL;

	input_set_drvdata(input_dev, &g_ts_data);

	error = input_register_device(input_dev);
	if (error) {
		ts_log_err("input dev register failed : %d\n", error);
		return -EINVAL;
	}

	return NO_ERR;
}

static int ts_create_dev_and_sysnode(void)
{
	ts_log_info("%s start\n", __func__);
	atomic_set(&g_ts_data.state, TS_UNINIT);
	atomic_set(&g_ts_data.ts_esd_state, TS_NO_ESD);
	g_ts_data.edge_wideth = EDGE_WIDTH_DEFAULT;
	g_ts_data.queue.rd_index = 0;
	g_ts_data.queue.wr_index = 0;
	g_ts_data.queue.cmd_count = 0;
	g_ts_data.queue.queue_size = TS_CMD_QUEUE_SIZE;
	spin_lock_init(&g_ts_data.queue.spin_lock);

	if (ts_parse_config()) {
		ts_log_err("ts parse config failed\n");
		return -EINVAL;
	}

	g_ts_data.ts_dev = platform_device_alloc("huawei_touch", -1);
	if (!g_ts_data.ts_dev) {
		ts_log_err("platform device malloc failed\n");
		goto err_out;
	}

	if (platform_device_add(g_ts_data.ts_dev)) {
		ts_log_err("platform device add failed\n");
		goto err_put_platform_dev;
	}

	if (sysfs_create_group(&g_ts_data.ts_dev->dev.kobj, &g_ts_attr_group)) {
		ts_log_err("can't create ts's sysfs\n");
		goto err_del_platform_dev;
	}

	if (sysfs_create_link(NULL, &g_ts_data.ts_dev->dev.kobj,
		"touchscreen")) {
		ts_log_err("%s: Fail create link\n", __func__);
		goto err_free_sysfs;
	}
	ts_log_info("%s end\n", __func__);
	return NO_ERR;

err_free_sysfs:
	sysfs_remove_group(&g_ts_data.ts_dev->dev.kobj, &g_ts_attr_group);
err_del_platform_dev:
	platform_device_del(g_ts_data.ts_dev);
err_put_platform_dev:
	platform_device_put(g_ts_data.ts_dev);
err_out:
	atomic_set(&g_ts_data.state, TS_UNINIT);
	return -EINVAL;
}

static void ts_start_watchdog(void)
{
	if (g_ts_data.chip_data->need_wd_check_status) {
		ts_log_info("This chip need watch dog to check status\n");
		INIT_WORK(&(g_ts_data.watchdog_work), ts_watchdog_work);
		setup_timer(&(g_ts_data.watchdog_timer), ts_watchdog_timer,
			(unsigned long)(&g_ts_data));
		ts_start_wd_timer(&g_ts_data);
	}
	ts_log_info("%s:called out\n", __func__);
}

static int ts_chip_detect_process(const char *chip_name,
	struct ts_device_data *ptr, struct device_node *child)
{
	int index;
	int ret = 0;

	for (index = 0; ptr != NULL && index < ARRAY_SIZE(g_ts_device_map);
		ptr++, index++) {
		if (!strcmp(chip_name, ptr->chip_name)) {
			if (g_ts_data.bops->btype == TS_BUS_I2C)
				ptr->slave_addr = g_ts_data.client->addr;

			ptr->bops = g_ts_data.bops;
			INIT_LIST_HEAD(&ptr->algo_head);
			g_ts_data.chip_data = ptr;
			g_ts_data.chip_data->is_i2c_one_byte = 0;
			g_ts_data.chip_data->is_new_oem_structure = 0;

			if (ptr->ops->chip_parse_config &&
				ptr->ops->chip_parse_config(child, ptr)) {
				ts_log_err("%s:parse config failed\n",
					chip_name);
				ret = -EINVAL;
				return ret;
			}

			if (ptr->ops->chip_detect) {
				ts_log_debug("call chip self init handle\n");
				ret = ptr->ops->chip_detect(child, ptr,
						g_ts_data.ts_dev);
			}
			return ret;
		}
	}
	if (index == ARRAY_SIZE(g_ts_device_map)) {
		ts_log_err("we can not find %s's chip data in device map\n",
			chip_name);
		ret = -EINVAL;
	}
	return ret;
}

static int chip_detect(const char *chip_name)
{
	bool found = false;
	int error = -EIO;
	int len;
	const __be32 *addr = NULL;
	struct device_node *child = NULL;
	struct ts_device_data *ptr = &g_ts_device_map[0];

	for_each_child_of_node(g_ts_data.node, child) {
		if (of_device_is_compatible(child, chip_name)) {
			found = true;
			break;
		}
	}

	if (!found) {
		ts_log_err("not find chip :%s's node\n", chip_name);
		goto out;
	}
	g_ts_data.dev_node = child;
	if (g_ts_data.bops->btype == TS_BUS_I2C) {
		addr = of_get_property(child, "slave_address", &len);
		if (!addr || (len < sizeof(int))) {
			ts_log_err("invalid slave_address on %s, len:%d\n",
				chip_name, len);
			goto out;
		}
		if (g_ts_data.client->addr != be32_to_cpup(addr)) {
			error =
				i2c_check_addr_busy(g_ts_data.client->adapter,
					be32_to_cpup(addr));
			if (error) {
				ts_log_err("0x%x slave addr conflict\n", *addr);
				goto out;
			}
			ts_log_debug("slave addr :0x%x not occupied\n", *addr);
			g_ts_data.client->addr = be32_to_cpup(addr);
		}
	}
	error = ts_chip_detect_process(chip_name, ptr, child);
out:
	return error;
}

static int get_support_chip(void)
{
	struct device_node *np = g_ts_data.node;
	const char *support_chip_name = NULL;
	int rc = 0;
	int index;
	int found = -ENXIO;
	int array_len;

	if (np) {
		array_len = of_property_count_strings(np, "support_chip_name");
		if (array_len <= 0) {
			ts_log_err("chip name length invaild:%d\n", array_len);
			return -EINVAL;
		}
		/* try to detect active ts ic */
		for (index = 0; index < array_len; index++) {
			rc = of_property_read_string_index(np,
				"support_chip_name",
				index,
				&support_chip_name);
			if (rc) {
				ts_log_err("read %d - chip name :%s, err:%d\n",
					index, support_chip_name, rc);
				continue;
			}

			if (!chip_detect(support_chip_name)) {
				found = NO_ERR;
				ts_log_info("chip: %s found success\n",
					support_chip_name);
				break;
			}
		}
	} else {
		ts_log_err("dts feature not support: %d\n", rc);
	}
	return found;
}

static int ts_init(void)
{
	int error = NO_ERR;
	struct input_dev *input_dev = NULL;

	if (ts_create_dev_and_sysnode())
		goto err_out;

	if (get_support_chip()) {
		ts_log_err("get support chip failed : %d\n", error);
		goto err_remove_sysfs_link;
	}

	if (ts_chip_init())
		ts_log_err("chip init failed:%d,try fwupdate again\n", error);

	if (ts_register_algo()) {
		ts_log_err("ts register algo failed : %d\n", error);
		goto err_remove_sysfs_link;
	}

	if (ts_register_input(input_dev)) {
		ts_log_err("ts register input failed : %d\n", error);
		goto err_free_input_dev;
	}

	g_ts_data.fb_notify.notifier_call = fb_notifier_callback;
	if (fb_register_client(&g_ts_data.fb_notify)) {
		ts_log_err("unable to register fb_notifier: %d\n", error);
		goto err_free_input_dev;
	}

	if (ts_set_irq()) {
		ts_log_err("set irq fialed: %d\n", error);
		goto err_unregister_suspend;
	}

	ts_send_init_cmd();
	ts_start_watchdog();
	ts_log_info("g_ts_data.state:%d\n", atomic_read(&g_ts_data.state));
	return NO_ERR;

err_unregister_suspend:
	if (fb_unregister_client(&g_ts_data.fb_notify))
		ts_log_err("error occurred while unregistering fb_notifier.\n");
err_free_input_dev:
	input_unregister_device(g_ts_data.input_dev);
	input_free_device(g_ts_data.input_dev);
err_remove_sysfs_link:
	sysfs_remove_link(NULL, "touchscreen");
	sysfs_remove_group(&g_ts_data.ts_dev->dev.kobj, &g_ts_attr_group);
	platform_device_del(g_ts_data.ts_dev);
	platform_device_put(g_ts_data.ts_dev);
err_out:
	atomic_set(&g_ts_data.state, TS_UNINIT);
	ts_log_info("g_ts_data.state:%d\n", atomic_read(&g_ts_data.state));
	return -EINVAL;
}

static int get_ts_bus_info(void)
{
	const char *bus_type = NULL;

	int rc;
	int error = NO_ERR;
	u32 bus_id = 0;

	g_ts_data.node = of_find_compatible_node(NULL, NULL, TS_DEV_NAME);
	if (!g_ts_data.node) {
		ts_log_err("can't find ts module node\n");
		error = -EINVAL;
		goto out;
	}

	rc = of_property_read_string(g_ts_data.node, "bus_type", &bus_type);
	if (rc) {
		ts_log_err("bus type read failed:%d\n", rc);
		error = -EINVAL;
		goto out;
	}
	if (!strcmp(bus_type, "i2c")) {
		g_ts_data.bops = &ts_bus_i2c_info;
	} else if (!strcmp(bus_type, "spi")) {
		g_ts_data.bops = &ts_bus_spi_info;
	} else {
		ts_log_err("bus type invaild:%s\n", bus_type);
		error = -EINVAL;
	}

	rc = of_property_read_u32(g_ts_data.node, "bus_id", &bus_id);
	if (rc) {
		ts_log_err("bus id read failed\n");
		error = -EINVAL;
		goto out;
	}

	g_ts_data.bops->bus_id = bus_id;
	ts_log_debug("bus id :%d\n", bus_id);
out:
	return error;
}

static int ts_creat_i2c_client(void)
{
	struct i2c_adapter *adapter = NULL;
	struct i2c_client *client = NULL;
	struct i2c_board_info board_info;

	memset(&board_info, 0, sizeof(struct i2c_board_info));
	strncpy(board_info.type, TS_DEV_NAME, I2C_NAME_SIZE);
	board_info.addr = I2C_DEFAULT_ADDR;
	board_info.flags = true;

	adapter = i2c_get_adapter(g_ts_data.bops->bus_id);
	if (!adapter) {
		ts_log_err("i2c_get_adapter failed\n");
		return -EIO;
	}

	client = i2c_new_device(adapter, &board_info);
	if (!client) {
		ts_log_err("i2c_new_device failed\n");
		return -EIO;
	}
	g_ts_data.client = client;
	i2c_set_clientdata(client, &g_ts_data);

	return NO_ERR;
}

static int ts_destroy_i2c_client(void)
{
	ts_log_err("destroy i2c device\n");
	i2c_unregister_device(g_ts_data.client);
	return NO_ERR;
}

static int ts_create_client(void)
{
	int error = -EINVAL;

	switch (g_ts_data.bops->btype) {
	case TS_BUS_I2C:
		ts_log_debug("create ts's i2c device\n");
		error = ts_creat_i2c_client();
		break;
	case TS_BUS_SPI:
		ts_log_debug("create ts's spi device\n");
		break;
	default:
		ts_log_err("unknown ts's device\n");
		break;
	}

	return error;
}

static void ts_destroy_client(void)
{
	ts_log_err("destroy touchscreen device\n");

	switch (g_ts_data.bops->btype) {
	case TS_BUS_I2C:
		ts_log_debug("destroy ts's i2c device\n");
		ts_destroy_i2c_client();
		break;
	case TS_BUS_SPI:
		ts_log_debug("destroy ts's spi device\n");
		break;
	default:
		ts_log_err("unknown ts's device\n");
		break;
	}
}

static int ts_thread(void *p)
{
	static const struct sched_param param = {
		.sched_priority = PRIORITY_SETTING,
	};
	smp_wmb();
	if (ts_init()) {
		ts_log_err("ts_init  failed\n");
		goto out;
	}
	memset(&ping_cmd_buff, 0, sizeof(struct ts_cmd_node));
	memset(&pang_cmd_buff, 0, sizeof(struct ts_cmd_node));
	smp_mb();
	sched_setscheduler(current, SCHED_RR, &param);

	while (ts_task_continue()) {
		while (!get_one_cmd(&ping_cmd_buff)) {
			ts_proc_command(&ping_cmd_buff);
			memset(&ping_cmd_buff, 0, sizeof(struct ts_cmd_node));
			memset(&pang_cmd_buff, 0, sizeof(struct ts_cmd_node));
		}
	}

	ts_log_err("ts thread stop\n");
	atomic_set(&g_ts_data.state, TS_UNINIT);
	disable_irq(g_ts_data.irq_id);
	ts_ic_shutdown();
	free_irq(g_ts_data.irq_id, &g_ts_data);
	sysfs_remove_group(&g_ts_data.ts_dev->dev.kobj, &g_ts_attr_group);
	if (fb_unregister_client(&g_ts_data.fb_notify))
		ts_log_err("error occurred while unregistering fb_notifier.\n");
	input_unregister_device(g_ts_data.input_dev);
	input_free_device(g_ts_data.input_dev);
	platform_device_unregister(g_ts_data.ts_dev);
out:
	ts_destroy_client();
	memset(&g_ts_data, 0, sizeof(struct ts_data));
	atomic_set(&g_ts_data.state, TS_UNINIT);
	ts_log_err("%s exited\n", __func__);
	return NO_ERR;
}

static int __init ts_module_init(void)
{
	int error = NO_ERR;

	ts_log_info("%s:called here\n", __func__);
	memset(&g_ts_data, 0, sizeof(struct ts_data));
	g_ts_data.dev_id = 0;
	error = get_ts_bus_info();
	if (error) {
		ts_log_err("get bus info failed:%d\n", error);
		memset(&g_ts_data, 0, sizeof(struct ts_data));
		goto out;
	}

	error = ts_create_client();
	if (error) {
		ts_log_err("create device failed:%d\n", error);
		goto out;
	}

	g_ts_data.ts_task =
		kthread_create(ts_thread, &g_ts_data, "ts_thread:%d",
			g_ts_data.dev_id);
	if (IS_ERR(g_ts_data.ts_task)) {
		ts_log_err("create ts_thread failed\n");
		ts_destroy_client();
		memset(&g_ts_data, 0, sizeof(struct ts_data));
		error = -EINVAL;
		goto out;
	}
	/*
	 * Attention about smp_mb/rmb/wmb
	 * Add these driver to avoid  data consistency problem
	 * ts_thread/ts_probe/irq_handler/put_one_cmd/get_one_cmd
	 * may run in different cpus and L1/L2 cache data consistency need
	 * to conside. We use barrier to make sure data consistently
	 */
	smp_mb();
	wake_up_process(g_ts_data.ts_task);

out:
	return error;
}

static void __exit ts_module_exit(void)
{
	ts_log_info("%s:called here\n", __func__);
	if (g_ts_data.ts_task)
		kthread_stop(g_ts_data.ts_task);
}

late_initcall(ts_module_init);
module_exit(ts_module_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Huawei TouchScreen Driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd");
