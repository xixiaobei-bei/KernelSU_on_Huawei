/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: Hisilicon hwcam usr dev source file.
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

#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/videodev2.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-ioctl.h>
#include <media/videobuf2-core.h>
#include <securec.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)) /* 4: kernel version */
#include <media/videobuf2-v4l2.h>
#endif

#include "hwcam_intf.h"
#include "cam_log.h"

typedef struct _tag_cam_dev {
	struct v4l2_device v4l2;
	struct mutex lock;
	struct video_device *vdev;
	struct media_device *mdev;
	struct mutex devlock;
} cam_dev_t;

static struct v4l2_file_operations s_fops_cam_dev = {
	.owner = THIS_MODULE,
	.open = NULL,
	.poll = NULL,
	.release = NULL,
	.unlocked_ioctl = NULL,
#ifdef CONFIG_COMPAT
	.compat_ioctl32 = NULL,
#endif
};

int create_media_device(struct device *dev, struct video_device *vdev,
		struct media_device **pmdev)
{
	int rc;
	struct media_device *mdev = NULL;

	mdev = kzalloc(sizeof(struct media_device), GFP_KERNEL);
	if (!mdev) {
		return -ENOMEM;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)) /* 4,9: kernel version */
	media_device_init(mdev);
#endif
	strlcpy(mdev->model, CAM_MODEL_USER, sizeof(mdev->model));
	mdev->dev = dev;
	rc = media_device_register(mdev);
	if (rc < 0) {
		kzfree(mdev);
		return rc;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)) /* 4,9: kernel version */
	rc = media_entity_pads_init(&vdev->entity, 0, NULL);
#else
	rc = media_entity_init(&vdev->entity, 0, NULL, 0);
#endif
	if (rc < 0) {
		media_device_unregister(mdev);
		kzfree(mdev);
		return rc;
	}

	*pmdev = mdev;
	return rc;
}

void destroy_media_device(struct video_device *vdev,
	struct media_device *mdev)
{
	media_entity_cleanup(&vdev->entity);
	media_device_unregister(mdev);
	kzfree(mdev);
}

void cam_dev_format_name(struct video_device *vdev, const char *prefix)
{
	int rc;
	rc = snprintf_s(vdev->name + strlen(vdev->name),
		sizeof(vdev->name) - strlen(vdev->name),
		sizeof(vdev->name) - strlen(vdev->name) - 1,
		"%s", video_device_node_name(vdev));
	if (rc < 0) {
		CAM_CFG_ERR("Truncation Occurred");
		(void)snprintf_s(vdev->name, sizeof(vdev->name),
			sizeof(vdev->name) - 1, "%s", prefix);
		(void)snprintf_s(vdev->name + strlen(vdev->name),
			sizeof(vdev->name) - strlen(vdev->name),
			sizeof(vdev->name) - strlen(vdev->name) - 1,
			"%s", video_device_node_name(vdev));
	}
}

int video_device_init(struct video_device *vdev,
	struct v4l2_device *v4l2, cam_dev_t *cam)
{
	int rc;
	char media_prefix[10]; /* 10, array len */

	gen_media_prefix(media_prefix, CAM_DEVICE_GROUP_ID,
		sizeof(media_prefix));
	rc = snprintf_s(vdev->name, sizeof(vdev->name),
		sizeof(vdev->name) - 1, "%s", media_prefix);
	if (rc < 0)
		CAM_CFG_ERR("snprintf_s media_prefix failed");
	strlcpy(vdev->name + strlen(vdev->name), "hwcam-userdev",
		sizeof(vdev->name) - strlen(vdev->name));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)) /* 4,9: kernel version */
	vdev->entity.obj_type = MEDIA_ENTITY_TYPE_VIDEO_DEVICE;
#else
	vdev->entity.type = MEDIA_ENT_T_DEVNODE_V4L;
	vdev->entity.group_id = CAM_DEVICE_GROUP_ID;
#endif
	vdev->v4l2_dev = v4l2;
	vdev->release = video_device_release;
	vdev->fops = &s_fops_cam_dev;
	vdev->minor = -1;
	vdev->vfl_type = VFL_TYPE_GRABBER;
	vdev->vfl_dir = VFL_DIR_TX;
	rc = video_register_device(vdev, VFL_TYPE_GRABBER, -1);
	if (rc < 0)
		return rc;
	cam_debug("video dev name %s %s", vdev->dev.kobj.name, vdev->name);
	mutex_init(&cam->lock);
	mutex_init(&cam->devlock);
	vdev->lock = &cam->lock;
	cam_dev_format_name(vdev, media_prefix);
	vdev->entity.name = vdev->name;

	return 0;
}

int create_video_device(struct device *dev, struct video_device **pvdev,
	struct v4l2_device *v4l2, cam_dev_t *cam,
	struct media_device *mdev)
{
	int rc;
	struct video_device *vdev = NULL;

	vdev = video_device_alloc();
	if (!vdev)
		return -ENOMEM;

	rc = create_media_device(dev, vdev, &mdev);
	if (rc < 0)
		goto media_alloc_fail;

	v4l2->mdev = mdev;
	v4l2->notify = NULL;
	rc = v4l2_device_register(dev, v4l2);
	if (rc < 0)
		goto v4l2_register_fail;

	rc = video_device_init(vdev, v4l2, cam);
	if (rc < 0)
		goto video_init_fail;

	*pvdev = vdev;
	goto create_end;

video_init_fail:
	v4l2_device_unregister(v4l2);

v4l2_register_fail:
	destroy_media_device(vdev, mdev);

media_alloc_fail:
	video_device_release(vdev);

create_end:
	return rc;
}

int cam_dev_create(struct device *dev, int *dev_num)
{
	int rc;
	struct v4l2_device *v4l2 = NULL;
	struct video_device *vdev = NULL;
	struct media_device *mdev = NULL;
	cam_dev_t *cam = NULL;

	cam = kzalloc(sizeof(cam_dev_t), GFP_KERNEL);
	if (!cam) {
		rc = -ENOMEM;
		goto init_end;
	}
	v4l2 = &cam->v4l2;

	rc = create_video_device(dev, &vdev, v4l2, cam, mdev);
	if (rc < 0)
		goto video_create_fail;

	video_set_drvdata(vdev, cam);
	cam->vdev = vdev;
	cam->mdev = mdev;
	if (dev_num)
		*dev_num = vdev->num;

	goto init_end;

video_create_fail:
	kzfree(cam);

init_end:
	return rc;
}
