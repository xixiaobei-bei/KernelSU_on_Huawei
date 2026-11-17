/*
 * hwdot.c
 *
 * Description: hwdot source file
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

#include <linux/compiler.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <media/huawei/camera.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>

#include "hwcam_intf.h"
#include "hwdot.h"
#include "securec.h"

#define EVENT_MAX_ELEMENT_SIZE  128

struct hwdot_t {
	struct v4l2_subdev     subdev;
	struct platform_device *pdev;
	struct mutex           lock;
	const hwdot_intf_t     *hw;
	cam_data_table_t     *cfg;
	hwdot_notify_intf_t    notify;
};

#define sd_to_hwdot(sd) container_of(sd, struct hwdot_t, subdev)
#define intf_to_dot(i) container_of(i, dot_t, intf)

static long hwdot_subdev_config(struct hwdot_t *s, hwdot_config_data_t *data);

static int hwdot_v4l2_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct hwdot_t *s = sd_to_hwdot(sd);

	CAM_CFG_INFO("instance(0x%pK)", s);
	return 0;
}

static int hwdot_v4l2_close(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct hwdot_t *s = sd_to_hwdot(sd);
	hwdot_config_data_t edata;
	cam_data_table_t *cfg = NULL;

	CAM_CFG_INFO("instance(0x%pK)", s);
	edata.cfgtype = CAM_DOT_POWEROFF;
	hwdot_subdev_config(s, &edata);

	swap(s->cfg, cfg);
	return 0;
}

static struct v4l2_subdev_internal_ops g_hwdot_subdev_internal_ops = {
	.open  = hwdot_v4l2_open,
	.close = hwdot_v4l2_close,
};

static long hwdot_subdev_get_info(struct hwdot_t *dot, hwdot_info_t *info)
{
	dot_t *ic = NULL;
	int ret;

	ret = memset_s(info->name, HWDOT_NAME_SIZE, 0, HWDOT_NAME_SIZE);
	if (ret != 0) {
		CAM_CFG_ERR("%s.memset failed %d", __func__, __LINE__);
		return -EINVAL;
	}

	ret = strncpy_s(info->name,
		HWDOT_NAME_SIZE - 1,
		hwdot_intf_get_name(dot->hw),
		strlen(hwdot_intf_get_name(dot->hw)) + 1);
	if (ret != 0) {
		CAM_CFG_ERR("%s.strncpy failed %d", __func__, __LINE__);
		return -EINVAL;
	}

	ic = intf_to_dot(dot->hw);
	info->i2c_idx = ic->i2c_index;
	CAM_CFG_INFO("i2c_index %d", info->i2c_idx);

	return 0;
}

int hwdot_get_thermal(const hwdot_intf_t *i, void *data)
{
	return i->vtbl->get_thermal(i, data);
}


static long hwdot_subdev_config(struct hwdot_t *s, hwdot_config_data_t *data)
{
	long rc = -EINVAL;
	static bool hwdot_power_on;

	switch (data->cfgtype) {
	case CAM_DOT_POWERON:
		if (!hwdot_power_on) {
			rc = s->hw->vtbl->power_on(s->hw);
			hwdot_power_on = true;
		}
		break;
	case CAM_DOT_POWEROFF:
		if (hwdot_power_on) {
			rc = s->hw->vtbl->power_off(s->hw);
			hwdot_power_on = false;
		}
		break;
	default:
		CAM_CFG_ERR("invalid cfgtype %d!", data->cfgtype);
		break;
	}

	return rc;
}

static long hwdot_subdev_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg)
{
	long rc;
	struct hwdot_t *s = sd_to_hwdot(sd);

	switch (cmd) {
	case HWDOT_IOCTL_GET_INFO:
		rc = hwdot_subdev_get_info(s, arg);
		break;
	case HWDOT_IOCTL_CONFIG:
		rc = hwdot_subdev_config(s, arg);
		break;
	case HWDOT_IOCTL_GET_THERMAL:
		rc = hwdot_get_thermal(s->hw, arg);
		break;
	default:
		CAM_CFG_ERR("invalid IOCTL CMD %u!", cmd);
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int hwdot_subdev_subscribe_event(
	struct v4l2_subdev *sd,
	struct v4l2_fh *fh,
	struct v4l2_event_subscription *sub)
{
	return v4l2_event_subscribe(fh, sub, EVENT_MAX_ELEMENT_SIZE, NULL);
}

static int hwdot_subdev_unsubscribe_event(
	struct v4l2_subdev *sd,
	struct v4l2_fh *fh,
	struct v4l2_event_subscription *sub)
{
	return v4l2_event_unsubscribe(fh, sub);
}

static int hwdot_power(struct v4l2_subdev *sd, int on)
{
	return 0;
}

#define notify_to_hwdot(i) container_of(i, struct hwdot_t, notify)

static void hwdot_notify_error(hwdot_notify_intf_t *i, hwdot_event_t *dot_ev)
{
	struct hwdot_t *dot = NULL;
	struct v4l2_event ev;
	struct video_device *vdev = NULL;
	hwdot_event_t *req = (hwdot_event_t *)ev.u.data;

	dot = notify_to_hwdot(i);
	vdev = dot->subdev.devnode;

	ev.type = HWDOT_V4L2_EVENT_TYPE;
	ev.id = HWDOT_HIGH_PRIO_EVENT;

	req->kind = dot_ev->kind;
	req->data.error.id = dot_ev->data.error.id;

	v4l2_event_queue(vdev, &ev);
}

static hwdot_notify_vtbl_t g_notify_hwdot = {
	.error = hwdot_notify_error,
};

static struct v4l2_subdev_core_ops g_hwdot_subdev_core_ops = {
	.ioctl = hwdot_subdev_ioctl,
	.subscribe_event = hwdot_subdev_subscribe_event,
	.unsubscribe_event = hwdot_subdev_unsubscribe_event,
	.s_power = hwdot_power,
};

static struct v4l2_subdev_ops g_hwdot_subdev_ops = {
	.core = &g_hwdot_subdev_core_ops,
};

int32_t hwdot_register(struct platform_device *pdev, const hwdot_intf_t *i,
	hwdot_notify_intf_t **notify)
{
	int rc = 0;
	int ret;
	struct v4l2_subdev *subdev = NULL;
	struct hwdot_t *dot = NULL;

	dot = kzalloc(sizeof(*dot), GFP_KERNEL);

	if (!dot) {
		rc = -ENOMEM;
		goto alloc_fail;
	}

	subdev = &dot->subdev;
	v4l2_subdev_init(subdev, &g_hwdot_subdev_ops);
	subdev->internal_ops = &g_hwdot_subdev_internal_ops;
	ret = snprintf_s(subdev->name, sizeof(subdev->name),
		sizeof(subdev->name)-1, "%s", hwdot_intf_get_name(i));
	if (ret < 0) {
		CAM_CFG_ERR("%s.snprintf_s failed %d", __func__, __LINE__);
		kfree(dot);
		dot = NULL;
		return -EINVAL;
	}

	subdev->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	subdev->flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
	v4l2_set_subdevdata(subdev, pdev);
	platform_set_drvdata(pdev, subdev);

	mutex_init(&dot->lock);

	init_subdev_media_entity(subdev, CAM_SUBDEV_DOT_PROJECTOR);
	cam_cfgdev_register_subdev(subdev, CAM_SUBDEV_DOT_PROJECTOR);

	subdev->devnode->lock = &dot->lock;
	dot->hw = pdev->dev.driver->of_match_table->data;
	dot->pdev = pdev;
	dot->notify.vtbl = &g_notify_hwdot;
	*notify = &(dot->notify);

	if (i->vtbl->dot_get_dt_data)
		rc = i->vtbl->dot_get_dt_data(i, pdev->dev.of_node);

	if (i->vtbl->init)
		rc = i->vtbl->init(i);

alloc_fail:

	return rc;
}

void hwdot_unregister(struct platform_device *pdev)
{
	struct v4l2_subdev *subdev = platform_get_drvdata(pdev);
	struct hwdot_t *dot = NULL;

	dot = sd_to_hwdot(subdev);
	media_entity_cleanup(&subdev->entity);
	cam_cfgdev_unregister_subdev(subdev);

	kzfree(dot);
}
