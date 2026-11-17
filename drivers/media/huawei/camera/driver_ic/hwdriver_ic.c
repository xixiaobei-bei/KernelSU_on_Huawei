/*
 * hwdriver_ic.c
 *
 * camera driver source file
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
#include <securec.h>
#include "hwcam_intf.h"
#include "hwdriver_ic.h"

#define EVENT_MAX_ELEMENT_SIZE  128

struct hwdriveric_t {
	struct v4l2_subdev                  subdev;
	struct platform_device              *pdev;
	struct mutex                        lock;
	const struct hwdriveric_intf_t      *hw;
	cam_data_table_t                  *cfg;
	struct hwdriveric_notify_intf_t     notify;
};

#define sdev_to_hwdriveric(sd) container_of(sd, struct hwdriveric_t, subdev)
#define intf_to_driveric(i) container_of(i, struct driveric_t, intf)

static long hwdriveric_subdev_config(
	 struct hwdriveric_t *s,
	 hwdriveric_config_data_t *data);

static int hwdriveric_v4l2_open(
	struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh)
{
	struct hwdriveric_t *s = sdev_to_hwdriveric(sd);

	CAM_CFG_INFO("instance(0x%pK)", s);
	return 0;
}

static int hwdriveric_v4l2_close(
	struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh)
{
	struct hwdriveric_t *s = sdev_to_hwdriveric(sd);
	hwdriveric_config_data_t edata;
	cam_data_table_t *cfg = NULL;

	CAM_CFG_INFO("instance(0x%pK)", s);
	edata.cfgtype = CAM_DRIVERIC_POWEROFF;
	hwdriveric_subdev_config(s, &edata);

	swap(s->cfg, cfg);
	return 0;
}

static struct v4l2_subdev_internal_ops g_hwdriveric_subdev_internal_ops = {
	.open  = hwdriveric_v4l2_open,
	.close = hwdriveric_v4l2_close,
};

static long hwdriveric_subdev_get_info(
	struct hwdriveric_t *driveric,
	hwdriveric_info_t *info)
{
	struct driveric_t *ic = NULL;
	int ret;

	ret = memset_s(info->name,
		HWDRIVERIC_NAME_SIZE,
		0,
		HWDRIVERIC_NAME_SIZE);
	if (ret != 0) {
		CAM_CFG_ERR("%s.memset_s failed", __func__);
		return -EINVAL;
	}
	ret = strncpy_s(info->name,
		HWDRIVERIC_NAME_SIZE - 1,
		hwdriveric_intf_get_name(driveric->hw),
		strlen(hwdriveric_intf_get_name(driveric->hw)) + 1);
	if (ret != 0) {
		CAM_CFG_ERR("%s.strncpy_s failed", __func__);
		return -EINVAL;
	}
	ic = intf_to_driveric(driveric->hw);

	info->i2c_idx  = ic->i2c_index;
	info->position = ic->position;

	CAM_CFG_INFO("ic name[%s], i2c_index[%d], postion[%d]!\n",
		info->name, info->i2c_idx, info->position);

	return 0;
}

static long hwdriveric_subdev_config(
	struct hwdriveric_t *s,
	hwdriveric_config_data_t *data)
{
	long rc = -EINVAL;

	switch (data->cfgtype) {
	case CAM_DRIVERIC_POWERON:
		if (s->hw->vtbl->power_on)
			rc = s->hw->vtbl->power_on(s->hw);
		break;
	case CAM_DRIVERIC_POWEROFF:
		if (s->hw->vtbl->power_off)
			rc = s->hw->vtbl->power_off(s->hw);
		break;
	default:
		CAM_CFG_ERR("invalid cfgtype[%d]!\n", data->cfgtype);
		break;
	}

	return rc;
}

static long hwdriveric_subdev_ioctl(
	struct v4l2_subdev *sd,
	unsigned int cmd,
	void *arg)
{
	long rc;
	struct hwdriveric_t *s = sdev_to_hwdriveric(sd);

	switch (cmd) {
	case HWDRIVERIC_IOCTL_GET_INFO:
		rc = hwdriveric_subdev_get_info(s, arg);
		break;
	case HWDRIVERIC_IOCTL_CONFIG:
		rc = hwdriveric_subdev_config(s, arg);
		break;
	default:
		CAM_CFG_ERR("invalid IOCTL CMD[%u]!\n", cmd);
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int hwdriveric_subdev_subscribe_event(
	struct v4l2_subdev *sd,
	struct v4l2_fh *fh,
	struct v4l2_event_subscription *sub)
{
	return v4l2_event_subscribe(fh, sub, EVENT_MAX_ELEMENT_SIZE, NULL);
}

static int hwdriveric_subdev_unsubscribe_event(
	struct v4l2_subdev *sd,
	struct v4l2_fh *fh,
	struct v4l2_event_subscription *sub)
{
	return v4l2_event_unsubscribe(fh, sub);
}

static int hwdriveric_power(struct v4l2_subdev *sd, int on)
{
	return 0;
}

#define notify_to_hwdriveric(i) container_of(i, struct hwdriveric_t, notify)

static void hwdriveric_notify_error(struct hwdriveric_notify_intf_t *i,
	hwdriveric_event_t *driveric_ev)
{
	struct hwdriveric_t *driveric = NULL;
	struct v4l2_event ev;
	struct video_device *vdev = NULL;
	hwdriveric_event_t *req = (hwdriveric_event_t *)ev.u.data;

	driveric = notify_to_hwdriveric(i);
	vdev = driveric->subdev.devnode;

	ev.type = HWDRIVERIC_V4L2_EVENT_TYPE;
	ev.id   = HWDRIVERIC_HIGH_PRIO_EVENT;

	req->kind = driveric_ev->kind;
	req->data.error.id = driveric_ev->data.error.id;

	v4l2_event_queue(vdev, &ev);
}

static struct hwdriveric_notify_vtbl_t g_notify_hwdriveric = {
	.error = hwdriveric_notify_error,
};

static struct v4l2_subdev_core_ops g_hwdriveric_subdev_core_ops = {
	.ioctl = hwdriveric_subdev_ioctl,
	.subscribe_event = hwdriveric_subdev_subscribe_event,
	.unsubscribe_event = hwdriveric_subdev_unsubscribe_event,
	.s_power = hwdriveric_power,
};

static struct v4l2_subdev_ops g_hwdriveric_subdev_ops = {
	.core = &g_hwdriveric_subdev_core_ops,
};

int32_t hwdriveric_register(struct platform_device *pdev,
	const struct hwdriveric_intf_t *i,
	struct hwdriveric_notify_intf_t **notify)
{
	int rc = 0;
	int ret;
	struct v4l2_subdev *subdev = NULL;
	struct hwdriveric_t *driveric =
		kzalloc(sizeof(struct hwdriveric_t), GFP_KERNEL);

	if (!driveric) {
		rc = -ENOMEM;
		goto alloc_fail;
	}

	subdev = &driveric->subdev;
	v4l2_subdev_init(subdev, &g_hwdriveric_subdev_ops);
	subdev->internal_ops = &g_hwdriveric_subdev_internal_ops;
	ret = snprintf_s(subdev->name, sizeof(subdev->name),
		sizeof(subdev->name)-1, "%s",
		hwdriveric_intf_get_name(i));
	if (ret < 0) {
		CAM_CFG_ERR("snprintf_s failed %s()%d", __func__, __LINE__);
		kzfree(driveric);
		rc = -EINVAL;
		goto alloc_fail;
	}
	/* can be access in user space */
	subdev->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	subdev->flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
	v4l2_set_subdevdata(subdev, pdev);
	platform_set_drvdata(pdev, subdev);

	mutex_init(&driveric->lock);

	init_subdev_media_entity(subdev, CAM_SUBDEV_DRIVER_IC);
	cam_cfgdev_register_subdev(subdev, CAM_SUBDEV_DRIVER_IC);

	subdev->devnode->lock = &driveric->lock;
	driveric->hw = i;
	driveric->pdev = pdev;
	driveric->notify.vtbl = &g_notify_hwdriveric;
	*notify = &(driveric->notify);

	if (i->vtbl->driveric_get_dt_data)
		rc = i->vtbl->driveric_get_dt_data(i, pdev->dev.of_node);

	if (i->vtbl->init)
		rc = i->vtbl->init(i);

	if (i->vtbl->driveric_register_attribute)
		rc = i->vtbl->driveric_register_attribute(i,
			&subdev->devnode->dev);

alloc_fail:
	return rc;
}

/* added for memory hwdriveric_t leak * */
void hwdriveric_unregister(struct platform_device *pdev)
{
	struct v4l2_subdev *subdev = platform_get_drvdata(pdev);
	struct hwdriveric_t *driveric = NULL;

	driveric = sdev_to_hwdriveric(subdev);
	media_entity_cleanup(&subdev->entity);
	cam_cfgdev_unregister_subdev(subdev);

	kzfree(driveric);
}

