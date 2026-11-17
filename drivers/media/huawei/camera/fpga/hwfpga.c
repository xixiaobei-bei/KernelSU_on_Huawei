/*
 * hwfpga.c
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
#include "hwfpga.h"

static struct mutex g_fpga_fd_lock;
static int g_fpga_fd_open_times;

struct hwfpga_t {
	struct v4l2_subdev subdev;
	struct platform_device *pdev;
	struct mutex lock;
	const struct hwfpga_intf_t *hw;
	cam_data_table_t *cfg;
	struct hwfpga_notify_intf_t notify;
};

#define sdev_to_hwfpga(sd) container_of(sd, struct hwfpga_t, subdev)

static long hwfpga_subdev_config(struct hwfpga_t *s,
	hwfpga_config_data_t *data);

static int hwfpga_v4l2_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	hwfpga_config_data_t edata;
	struct hwfpga_t *s = NULL;

	if (!sd || !fh) {
		CAM_CFG_ERR("sd or fh is null");
		return -EINVAL;
	}

	s = sdev_to_hwfpga(sd);
	CAM_CFG_INFO("instance(0x%pK)", s);

	mutex_lock(&g_fpga_fd_lock);
	if (g_fpga_fd_open_times == 0) {
		edata.cfgtype = CAM_FPGA_INITIAL;
		hwfpga_subdev_config(s, &edata);
	}
	g_fpga_fd_open_times++;
	mutex_unlock(&g_fpga_fd_lock);
	CAM_CFG_INFO("g_fpga_fd_open_times = %d", g_fpga_fd_open_times);
	return 0;
}

static int hwfpga_v4l2_close(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct hwfpga_t *s = NULL;
	hwfpga_config_data_t edata;
	cam_data_table_t *cfg = NULL;

	if (!sd || !fh) {
		CAM_CFG_ERR("sd or fh is null");
		return -EINVAL;
	}
	s = sdev_to_hwfpga(sd);
	CAM_CFG_INFO("instance(0x%pK)", s);

	mutex_lock(&g_fpga_fd_lock);
	g_fpga_fd_open_times--;

	if (g_fpga_fd_open_times == 0)
		edata.cfgtype = CAM_FPGA_CLOSE;
	else
		edata.cfgtype = CAM_FPGA_POWEROFF;

	hwfpga_subdev_config(s, &edata);
	mutex_unlock(&g_fpga_fd_lock);

	swap(s->cfg, cfg);

	CAM_CFG_INFO("g_fpga_fd_open_times = %d.", g_fpga_fd_open_times);
	return 0;
}

static struct v4l2_subdev_internal_ops g_hwfpga_subdev_internal_ops = {
	.open  = hwfpga_v4l2_open,
	.close = hwfpga_v4l2_close,
};

static long hwfpga_subdev_get_info(struct hwfpga_t *fpga, hwfpga_info_t *info)
{
	int ret;

	if (!fpga || !info) {
		CAM_CFG_ERR("fpga or info is null");
		return -EINVAL;
	}

	ret = memcpy_s(info->name,
		HWFPGA_NAME_SIZE,
		hwfpga_intf_get_name(fpga->hw),
		HWFPGA_NAME_SIZE);
	if (ret != 0) {
		CAM_CFG_ERR("memcpy_s failed");
		return -EINVAL;
	}

	CAM_CFG_INFO("fpga name(%s)\n", info->name);
	return 0;
}

static long hwfpga_subdev_config(struct hwfpga_t *s, hwfpga_config_data_t *data)
{
	long rc = -EINVAL;

	CAM_CFG_INFO("%s cfgtype(%d)", __func__, data->cfgtype);

	switch (data->cfgtype) {
	case CAM_FPGA_POWERON:
		if (s->hw->vtbl->power_on)
			rc = s->hw->vtbl->power_on(s->hw);
		break;
	case CAM_FPGA_POWEROFF:
		if (s->hw->vtbl->power_off)
			rc = s->hw->vtbl->power_off(s->hw);
		break;
	case CAM_FPGA_LOADFW:
		if (s->hw->vtbl->load_firmware)
			rc = s->hw->vtbl->load_firmware(s->hw);
		break;
	case CAM_FPGA_ENABLE:
		if (s->hw->vtbl->enable)
			rc = s->hw->vtbl->enable(s->hw);
		break;
	case CAM_FPGA_DISABLE:
		if (s->hw->vtbl->disable)
			rc = s->hw->vtbl->disable(s->hw);
		break;
	case CAM_FPGA_INITIAL:
		if (s->hw->vtbl->init)
			rc = s->hw->vtbl->init(s->hw);
		break;
	case CAM_FPGA_CLOSE:
		if (s->hw->vtbl->close)
			rc = s->hw->vtbl->close(s->hw);
		break;
	case CAM_FPGA_CHECKDEVICE:
		if (s->hw->vtbl->checkdevice)
			rc = s->hw->vtbl->checkdevice(s->hw);
		break;
	default:
		CAM_CFG_ERR("invalid cfgtype(%d)\n", data->cfgtype);
		break;
	}

	return rc;
}

static long hwfpga_subdev_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg)
{
	long rc;
	struct hwfpga_t *s = NULL;

	if (!sd || !arg) {
		CAM_CFG_ERR("sd or arg is null");
		return -EINVAL;
	}

	s = sdev_to_hwfpga(sd);
	switch (cmd) {
	case HWFPGA_IOCTL_GET_INFO:
		rc = hwfpga_subdev_get_info(s, arg);
		break;
	case HWFPGA_IOCTL_CONFIG:
		CAM_CFG_INFO("%s CMD(%u)", __func__, cmd);
		rc = hwfpga_subdev_config(s, arg);
		break;
	default:
		CAM_CFG_ERR("invalid IOCTL CMD(%u)\n", cmd);
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int hwfpga_subdev_subscribe_event(struct v4l2_subdev *sd,
	struct v4l2_fh *fh, struct v4l2_event_subscription *sub)
{
	/* 128:element size */
	return v4l2_event_subscribe(fh, sub, 128, NULL);
}

static int hwfpga_subdev_unsubscribe_event(struct v4l2_subdev *sd,
	struct v4l2_fh *fh, struct v4l2_event_subscription *sub)
{
	return v4l2_event_unsubscribe(fh, sub);
}

static int hwfpga_power(struct v4l2_subdev *sd, int on)
{
	return 0;
}

#define notify_to_hwfpga(i) container_of(i, struct hwfpga_t, notify)

static void hwfpga_notify_error(struct hwfpga_notify_intf_t *i,
	hwfpga_event_t *fpga_ev)
{
	struct hwfpga_t *fpga = NULL;
	struct v4l2_event ev;
	struct video_device *vdev = NULL;
	hwfpga_event_t *req = (hwfpga_event_t *)ev.u.data;

	if (!fpga_ev || !i) {
		CAM_CFG_ERR("invalid arguments");
		return;
	}

	fpga = notify_to_hwfpga(i);
	vdev = fpga->subdev.devnode;

	ev.type = HWFPGA_V4L2_EVENT_TYPE;
	ev.id = HWFPGA_HIGH_PRIO_EVENT;

	req->kind = fpga_ev->kind;
	req->data.error.id = fpga_ev->data.error.id;

	v4l2_event_queue(vdev, &ev);
}

static struct hwfpga_notify_vtbl_t g_notify_hwfpga = {
	.error = hwfpga_notify_error,
};

static struct v4l2_subdev_core_ops g_hwfpga_subdev_core_ops = {
	.ioctl = hwfpga_subdev_ioctl,
	.subscribe_event = hwfpga_subdev_subscribe_event,
	.unsubscribe_event = hwfpga_subdev_unsubscribe_event,
	.s_power = hwfpga_power,
};

static struct v4l2_subdev_ops g_hwfpga_subdev_ops = {
	.core = &g_hwfpga_subdev_core_ops,
};

int32_t hwfpga_register(struct platform_device *pdev,
	const struct hwfpga_intf_t *i, struct hwfpga_notify_intf_t **notify)
{
	int rc = 0;
	struct v4l2_subdev *subdev = NULL;
	struct hwfpga_t *fpga = NULL;

	if (!pdev || !i || !notify) {
		CAM_CFG_ERR("invalid arguments");
		return -EINVAL;
	}
	fpga = kzalloc(sizeof(struct hwfpga_t), GFP_KERNEL);
	if (!fpga) {
		rc = -ENOMEM;
		goto alloc_fail;
	}

	subdev = &fpga->subdev;
	v4l2_subdev_init(subdev, &g_hwfpga_subdev_ops);
	subdev->internal_ops = &g_hwfpga_subdev_internal_ops;
	rc = snprintf_s(subdev->name, sizeof(subdev->name),
		sizeof(subdev->name)-1, "%s", hwfpga_intf_get_name(i));
	if (rc < 0) {
		CAM_CFG_ERR("isnprintf_s fail");
		kzfree(fpga);
		return -1;
	}
	/* can be access in user space */
	subdev->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	subdev->flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
	v4l2_set_subdevdata(subdev, pdev);
	platform_set_drvdata(pdev, subdev);

	mutex_init(&fpga->lock);
	mutex_init(&g_fpga_fd_lock);

	init_subdev_media_entity(subdev, CAM_SUBDEV_FPGA);
	cam_cfgdev_register_subdev(subdev, CAM_SUBDEV_FPGA);

	subdev->devnode->lock = &fpga->lock;
	fpga->hw = pdev->dev.driver->of_match_table->data;
	fpga->pdev = pdev;
	fpga->notify.vtbl = &g_notify_hwfpga;
	*notify = &(fpga->notify);

	if (i->vtbl->fpga_get_dt_data)
		rc = i->vtbl->fpga_get_dt_data(i, pdev->dev.of_node);

	if (i->vtbl->fpga_register_attribute)
		rc = i->vtbl->fpga_register_attribute(i, &subdev->devnode->dev);

alloc_fail:
	return rc;
}

/* added for memory struct hwfpga_t leak */
void hwfpga_unregister(struct platform_device *pdev)
{
	struct hwfpga_t *fpga = NULL;
	struct v4l2_subdev *subdev = NULL;

	if (!pdev) {
		CAM_CFG_ERR("pdev is null");
		return;
	}
	subdev = platform_get_drvdata(pdev);
	fpga = sdev_to_hwfpga(subdev);
	media_entity_cleanup(&subdev->entity);
	cam_cfgdev_unregister_subdev(subdev);

	kzfree(fpga);
}


