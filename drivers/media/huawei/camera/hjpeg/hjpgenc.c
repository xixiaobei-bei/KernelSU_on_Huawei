/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: Hisilicon hjpgenc source file.
 * Author: yangkai
 * Create: 2016-04-01
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/compiler.h>
#include <linux/gpio.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <media/v4l2-subdev.h>
#include <media/huawei/hjpeg_cfg.h>
#include <securec.h>

#include "hjpeg_intf.h"
#include "hwcam_intf.h"
#include "cam_log.h"

typedef struct _tag_hjpeg {
	struct v4l2_subdev        subdev;
	struct platform_device    *pdev;
	struct mutex              lock;
	hjpeg_intf_t              *intf;
	unsigned int              jpeg_power_ref;
	unsigned int              jpeg_open_ref;
} hjpeg_t;

struct mutex g_jpeg_ref_lock;
#define sd_2_hjpeg(sd) container_of(sd, hjpeg_t, subdev)
#define i_2_hjpeg(jpeg_intf) container_of(jpeg_intf, hjpeg_t, intf)

extern int memset_s(void *dest, size_t destMax, int c, size_t count);

static bool is_hjpeg_encode_power_on(hjpeg_t *hjpeg)
{
	bool rc = false;

	if (!hjpeg) {
		cam_err("%s hjpeg == NULL.%d", __FUNCTION__, __LINE__);
		return rc;
	}

	if (hjpeg->jpeg_power_ref == 0) {
		cam_err("%s hjpeg do not power on.%d", __FUNCTION__, __LINE__);
		return rc;
	}

	return true;
}

static long hjpeg_encode_process(hjpeg_t *hjpeg, void *arg)
{
	long rc = -EINVAL;

	if (!is_hjpeg_encode_power_on(hjpeg))
		return rc;

	return hjpeg->intf->vtbl->encode_process(hjpeg->intf, arg);
}

static long hjpeg_encode_power_on(hjpeg_t *hjpeg)
{
	long rc = 0;

	if (!hjpeg) {
		cam_err("%s hjpeg is NULL %d", __FUNCTION__, __LINE__);
		return -EINVAL;
	}

	if (hjpeg->jpeg_power_ref == 0) {
		rc = hjpeg->intf->vtbl->power_on(hjpeg->intf);
		if (rc != 0) {
			cam_err("%s hjpeg power on fail.%d",
				__FUNCTION__, __LINE__);
			return -EINVAL;
		}
	}

	hjpeg->jpeg_power_ref++;
	return rc;
}

static long hjpeg_encode_power_down(hjpeg_t *hjpeg)
{
	long rc = 0;

	if (!is_hjpeg_encode_power_on(hjpeg))
		return -EINVAL;

	if (hjpeg->jpeg_power_ref == 1) {
		rc = hjpeg->intf->vtbl->power_down(hjpeg->intf);
		if (rc != 0) {
			cam_err("%s hjpeg power down fail.%d",
				__FUNCTION__, __LINE__);
			return rc;
		}
	}

	if (hjpeg->jpeg_power_ref > 0)
		hjpeg->jpeg_power_ref--;

	return rc;
}

static long hjpeg_vo_subdev_ioctl(struct v4l2_subdev *sd, unsigned int cmd,
	void *arg)
{
	long rc = -EINVAL;
	hjpeg_t *hjpeg = NULL;

	if (!sd) {
		cam_err("%s: sd is NULL", __FUNCTION__);
		return rc;
	}

	hjpeg = sd_2_hjpeg(sd);
	mutex_lock(&g_jpeg_ref_lock);
	switch (cmd) {
	case HJPEG_ENCODE_PROCESS:
		rc = hjpeg_encode_process(hjpeg, arg);
		break;
	case HJPEG_ENCODE_POWERON:
		rc = hjpeg_encode_power_on(hjpeg);
		break;
	case HJPEG_ENCODE_POWERDOWN:
		rc = hjpeg_encode_power_down(hjpeg);
		break;
	default:
		cam_info("%s: invalid ioctl cmd for hjpeg!!!cmd is %d",
			__FUNCTION__, cmd);
		break;
	}
	mutex_unlock(&g_jpeg_ref_lock);
	return rc;
}

#ifdef CONFIG_COMPAT
static long hjpeg_usercopy(struct v4l2_subdev *sd, unsigned int cmd, void *kp,
	void __user *up)
{
	long rc;
	rc = memset_s(kp, _IOC_SIZE(cmd), 0, _IOC_SIZE(cmd));
	if (rc != 0) {
		cam_err("%s memset_s return fail", __FUNCTION__);
		return -EINVAL;
	}

	if (_IOC_DIR(cmd) & _IOC_WRITE) {
		if (copy_from_user(kp, up, _IOC_SIZE(cmd))) {
			cam_err("%s: copy in arg failed", __FUNCTION__);
			return -EFAULT;
		}
	}

	rc = hjpeg_vo_subdev_ioctl(sd, cmd, kp);
	if (rc < 0) {
		cam_err("%s subdev_ioctl failed", __FUNCTION__);
		return -EFAULT;
	}

	switch (_IOC_DIR(cmd)) {
	case _IOC_READ:
	case (_IOC_WRITE | _IOC_READ):
		if (copy_to_user(up, kp, _IOC_SIZE(cmd))) {
			cam_err("%s: copy back arg failed", __FUNCTION__);
			return -EFAULT;
		}
		break;
	default:
		break;
	}

	return rc;
}

static long hjpeg_vo_subdev_compat_ioctl32(
	struct v4l2_subdev *sd,
	unsigned int cmd,
	unsigned long arg)
{
	long rc = -EINVAL;
	switch (cmd) {
	case HJPEG_ENCODE_PROCESS:
	case HJPEG_ENCODE_SETREG:
	case HJPEG_ENCODE_GETREG: {
		jpgenc_config_t data;
		if (_IOC_SIZE(cmd) > sizeof(data)) {
			cam_err("%s: cmd size too large", __FUNCTION__);
			return -EINVAL;
		}

		rc = hjpeg_usercopy(sd, cmd, (void*)&data, (void __user*)arg);
		if (rc < 0)
			cam_err("%s: hjpeg_usercopy fail", __FUNCTION__);
		break;
	}
	case HJPEG_ENCODE_POWERON:
	case HJPEG_ENCODE_POWERDOWN: {
		rc = hjpeg_vo_subdev_ioctl(sd, cmd, NULL);
		if (rc != 0)
			cam_err("%s: hjpeg encode poweron and powerdonw failed",
				__FUNCTION__);
		break;
	}
	default:
		cam_info("%s: invalid ioctl cmd for hjpeg!!!cmd is %d",
			__FUNCTION__, cmd);
		break;
	}

	return rc;
}
#endif

static int hjpeg_vo_power(struct v4l2_subdev *sd, int on)
{
	return 0;
}

static int hjpeg_subdev_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	hjpeg_t *hjpeg = NULL;
	if (!sd) {
		cam_err("%s: sd is NULL", __FUNCTION__);
		return -1;
	}
	hjpeg = sd_2_hjpeg(sd);
	mutex_lock(&g_jpeg_ref_lock);
	hjpeg->jpeg_open_ref++;
	mutex_unlock(&g_jpeg_ref_lock);
	cam_info("%s hjpeg_sbudev open,OpenRef = %d",__FUNCTION__,
		hjpeg->jpeg_open_ref);
	return 0;
}

static int hjpeg_subdev_close(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	int rc = 0;
	hjpeg_t *hjpeg = NULL;
	if (!sd) {
		cam_err("%s: sd is NULL", __FUNCTION__);
		return -1;
	}
	hjpeg = sd_2_hjpeg(sd);
	mutex_lock(&g_jpeg_ref_lock);
	if (hjpeg->jpeg_open_ref)
		hjpeg->jpeg_open_ref--;
	if ((hjpeg->jpeg_open_ref == 0) && (is_hjpeg_encode_power_on(hjpeg))) {
		cam_warn("%s hjpeg is still on power-on state, power off it",
			__FUNCTION__);
		rc = hjpeg_encode_power_down(hjpeg);
		if (rc != 0)
			cam_err("failed to hjpeg power off");
	}
	mutex_unlock(&g_jpeg_ref_lock);
	cam_info("%s hjpeg_sbudev close!,OpenRef = %d",
		__FUNCTION__, hjpeg->jpeg_open_ref);
	return 0;
}

static struct v4l2_subdev_internal_ops s_subdev_internal_ops_hjpeg = {
	.open = hjpeg_subdev_open,
	.close = hjpeg_subdev_close,
};

static struct v4l2_subdev_core_ops s_subdev_core_ops_hjpeg = {
	.ioctl = hjpeg_vo_subdev_ioctl,
#ifdef  CONFIG_COMPAT
	.compat_ioctl32 = hjpeg_vo_subdev_compat_ioctl32,
#endif
	.s_power = hjpeg_vo_power,
};

static struct v4l2_subdev_ops s_subdev_ops_hjpeg = {
	.core = &s_subdev_core_ops_hjpeg,
};

int hjpeg_register(struct platform_device *pdev, hjpeg_intf_t *si)
{
	int rc = 0;
	int count;

	struct v4l2_subdev *subdev = NULL;
	hjpeg_t *jpeg = (hjpeg_t*)kzalloc(sizeof(hjpeg_t), GFP_KERNEL);
	if (!jpeg) {
		rc = -ENOMEM;
		goto register_fail;
	}

	subdev = &jpeg->subdev;
	mutex_init(&jpeg->lock);
	mutex_init(&g_jpeg_ref_lock);
	v4l2_subdev_init(subdev, &s_subdev_ops_hjpeg);
	subdev->internal_ops = &s_subdev_internal_ops_hjpeg;
	count = snprintf_s(subdev->name, sizeof(subdev->name),
		sizeof(subdev->name) - 1, "%s", "hwcam-cfg-jpeg");
	if (count < 0)
		cam_err("snprintf_s fail");
	subdev->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	v4l2_set_subdevdata(subdev, pdev);

	init_subdev_media_entity(subdev, CAM_SUBDEV_HJPEG);
	cam_cfgdev_register_subdev(subdev, CAM_SUBDEV_HJPEG);
	subdev->devnode->lock = &jpeg->lock;

	platform_set_drvdata(pdev, subdev);
	jpeg->intf = si;
	jpeg->pdev = pdev;
	jpeg->jpeg_power_ref = 0;
	jpeg->jpeg_open_ref = 0;
register_fail:
	return rc;
}

void hjpeg_unregister(struct platform_device *pdev)
{
	struct v4l2_subdev *subdev = platform_get_drvdata(pdev);
	hjpeg_t *jpeg = sd_2_hjpeg(subdev);

	media_entity_cleanup(&subdev->entity);
	cam_cfgdev_unregister_subdev(subdev);

	kzfree(jpeg);
}
