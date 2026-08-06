/**
 * @file
 * Copyright (c) 2021-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#include "dpu_utils.h"
#include "dpu_vsync.h"
#include "comp_drv.h"

static inline int vsync_timestamp_changed(struct dpu_vsync *vsync_ctrl, ktime_t prev_timestamp)
{
	dpu_assert_if_cond(vsync_ctrl == NULL);

	return !(prev_timestamp == vsync_ctrl->timestamp);
}

static ssize_t vsync_event_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct composer_private *comp = NULL;
	struct dpu_vsync *vsync_ctrl = NULL;
	ssize_t ret;
	ktime_t prev_timestamp;

	if (!dev || !buf) {
		dpu_pr_err("NULL Pointer\n");
		return -1;
	}

	comp = dev_get_drvdata(dev);
	if (!comp) {
		dpu_pr_err("NULL Pointer\n");
		return -1;
	}

	vsync_ctrl = &comp->vsync_ctrl;
	prev_timestamp = vsync_ctrl->timestamp;
	ret = wait_event_interruptible(vsync_ctrl->wait, (vsync_timestamp_changed(vsync_ctrl, prev_timestamp) && vsync_ctrl->enabled));
	if (ret) {
		dpu_pr_err("vsync wait event be interrupted abnormal!!!");
		return -1;
	}

	ret = snprintf(buf, PAGE_SIZE, "VSYNC=%llu\n", ktime_to_ns(vsync_ctrl->timestamp));
	buf[strlen(buf) + 1] = '\0';
	return ret;
}

static ssize_t vsync_timestamp_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct composer_private *comp = NULL;
	struct dpu_vsync *vsync_ctrl = NULL;
	ssize_t ret;

	if (!dev || !buf) {
		dpu_pr_err("NULL Pointer\n");
		return -1;
	}

	comp = dev_get_drvdata(dev);
	if (!comp) {
		dpu_pr_err("NULL Pointer\n");
		return -1;
	}

	vsync_ctrl = &comp->vsync_ctrl;

	dpu_pr_err(" ++++ ");

	ret = snprintf(buf, PAGE_SIZE, "%llu\n", ktime_to_ns(vsync_ctrl->timestamp));
	buf[strlen(buf) + 1] = '\0';

	dpu_pr_err(" ---- ");
	return ret;
}

static ssize_t vsync_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct composer_private *comp = NULL;
	struct dpu_vsync *vsync_ctrl = NULL;
	int ret;
	int enable;

	if (!dev || !buf) {
		dpu_pr_err("NULL Pointer.\n");
		return -1;
	}

	comp = dev_get_drvdata(dev);
	if (!comp) {
		dpu_pr_err("NULL Pointer.\n");
		return -1;
	}

	vsync_ctrl = &comp->vsync_ctrl;

	ret = sscanf(buf, "%d", &enable);
	if (!ret) {
		dpu_pr_err("get buf (%s) enable fail\n", buf);
		return -1;
	}

	dpu_pr_info("vsync enable=%d", enable);

	dpu_vsync_enable(vsync_ctrl, enable);
	return count;
}

static DEVICE_ATTR(vsync_event, 0444, vsync_event_show, NULL);
static DEVICE_ATTR(vsync_enable, 0444, NULL, vsync_enable_store);
static DEVICE_ATTR(vsync_timestamp, 0444, vsync_timestamp_show, NULL);

static int vsync_isr_notify(struct notifier_block *self, unsigned long action, void *data)
{
	struct dpu_vsync *vsync_ctrl = (struct dpu_vsync *)data;

	if ((action & vsync_ctrl->listening_isr_bit) == 0)
		return 0;

	vsync_ctrl->timestamp = ktime_get();

	if (dpu_vsync_is_enabled(vsync_ctrl))
		wake_up_interruptible_all(&(vsync_ctrl->wait));

	// TODO: other vsync function

	return 0;
}

static struct notifier_block vsync_isr_notifier = {
	.notifier_call = vsync_isr_notify,
};

void dpu_vsync_init(struct dpu_vsync *vsync_ctrl, struct dpu_attr *attrs, uint32_t listening_isr_bit)
{
	dpu_assert_if_cond(vsync_ctrl == NULL);
	dpu_assert_if_cond(attrs == NULL);

	spin_lock_init(&(vsync_ctrl->spin_enable));
	init_waitqueue_head(&vsync_ctrl->wait);

	vsync_ctrl->enabled = 0;
	vsync_ctrl->notifier = &vsync_isr_notifier;
	vsync_ctrl->listening_isr_bit = listening_isr_bit;

	dpu_sysfs_attrs_append(attrs, &dev_attr_vsync_event.attr);
	dpu_sysfs_attrs_append(attrs, &dev_attr_vsync_enable.attr);
	dpu_sysfs_attrs_append(attrs, &dev_attr_vsync_timestamp.attr);
}


