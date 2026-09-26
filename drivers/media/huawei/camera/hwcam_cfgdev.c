/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: Hisilicon hwcam config devive source file.
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

#include <linux/atomic.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/freezer.h>
#include <linux/pid.h>
#include <linux/slab.h>
#include <linux/videodev2.h>
#include <media/media-device.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-ioctl.h>
#include <securec.h>

#include "hwcam_intf.h"
#include "cam_log.h"
#include "hwcam_compat32.h"
#include <dsm/dsm_pub.h>
#include <linux/of.h>

#define CREATE_TRACE_POINTS

#include "trace_hwcam.h"

struct dsm_client_ops g_ops2 = {
	.poll_state = NULL,
	.dump_func = NULL,
};

struct dsm_dev g_dev_camera_user = {
	.name = "dsm_camera_user",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = &g_ops2,
	.buff_size = 4096, /* 4096, buf size */
};

struct dsm_client *client_camera_user;

typedef struct _tag_cam_cfgdev_vo {
	struct v4l2_device    v4l2;
	struct video_device   *vdev;
	struct media_device   *mdev;

	struct dentry         *debug_root;
	struct v4l2_fh        rq;
	__u8                  sbuf[64]; /* 64, buf len */
	struct mutex          lock;
} cam_cfgdev_vo_t;

typedef enum _tag_cam_cfgsvr_flags {
	CAM_CFGSVR_FLAG_UNPLAYED = 0,
	CAM_CFGSVR_FLAG_PLAYING = 1,
} cam_cfgsvr_flags_t;

static DEFINE_MUTEX(s_cfgdev_lock);
static cam_cfgdev_vo_t s_cfgdev;

static DEFINE_MUTEX(s_cfgsvr_lock);
static DECLARE_WAIT_QUEUE_HEAD(s_wait_cfgsvr);
static struct pid *s_pid_cfgsvr;
static int g_is_binderized = 0; /* default 0 is Passthrough */

void cam_cfgdev_lock(void)
{
	mutex_lock(&s_cfgdev_lock);
}

void cam_cfgdev_unlock(void)
{
	mutex_unlock(&s_cfgdev_lock);
}

static DEFINE_SPINLOCK(s_ack_queue_lock);
static atomic_t s_sequence = ATOMIC_INIT(0);
static cam_cfgsvr_flags_t s_cfgsvr_flags = CAM_CFGSVR_FLAG_UNPLAYED;
static struct list_head s_ack_queue = LIST_HEAD_INIT(s_ack_queue);
static DECLARE_WAIT_QUEUE_HEAD(s_wait_ack);

static void cam_cfgdev_work(struct work_struct *w);
static DECLARE_DELAYED_WORK(s_cfgdev_work, cam_cfgdev_work);

static ssize_t guard_thermal_show(struct device_driver *drv, char *buf);
static ssize_t guard_thermal_store(struct device_driver *drv,
	const char *buf, size_t count);
static DRIVER_ATTR_RW(guard_thermal);

static ssize_t dump_meminfo_show(struct device_driver *drv, char *buf);
static ssize_t dump_meminfo_store(struct device_driver *drv,
	const char *buf, size_t count);
static DRIVER_ATTR_RW(dump_meminfo);

/* set camera log level by device */
uint32_t cam_debug_mask = CAM_DEBUG_INFO | CAM_DEBUG_EMERG | \
	CAM_DEBUG_ALERT | CAM_DEBUG_CRIT | CAM_DEBUG_ERR | CAM_DEBUG_WARING;

#ifdef DEBUG_HISI_CAMERA
static ssize_t cam_log_show(struct device_driver *drv, char *buf);
static ssize_t cam_log_store(struct device_driver *drv,
	const char *buf, size_t count);
static DRIVER_ATTR_RW(cam_log);
#endif

static int cam_cfgdev_guard_thermal(void)
{
	struct v4l2_event ev = {
		.type = CAM_V4L2_EVENT_TYPE,
		.id = CAM_CFGDEV_REQUEST,
	};
	cam_cfgreq2dev_t *req = (cam_cfgreq2dev_t *)ev.u.data;

	req->req.intf = NULL;
	req->kind = CAM_CFGDEV_REQ_GUARD_THERMAL;
	return cam_cfgdev_send_req(NULL, &ev, &s_cfgdev.rq, 1, NULL);
}

static ssize_t guard_thermal_store(struct device_driver *drv,
	const char *buf, size_t count)
{
	int ret = 0;

	(void)drv;
	if (!buf) {
		cam_err("%s input buf is null", __FUNCTION__);
		return ret;
	}

	cam_info("%s enter", __FUNCTION__);
	if (count > 1) {
		errno_t err = memset_s(s_cfgdev.sbuf,
			sizeof(s_cfgdev.sbuf), 0, sizeof(s_cfgdev.sbuf));
		if (err != EOK)
			cam_warn("%s memset_s fail", __FUNCTION__);
		if (count <= sizeof(s_cfgdev.sbuf)) {
			err = memcpy_s(s_cfgdev.sbuf,
				sizeof(s_cfgdev.sbuf) - 1, buf, count - 1);
			if (err != EOK)
				cam_warn("%s mcmcpy fail", __FUNCTION__);
		} else {
			size_t sbuf_size = sizeof(s_cfgdev.sbuf);
			err = memcpy_s(s_cfgdev.sbuf,
				sbuf_size - 1, buf, sbuf_size - 1);
			if (err != EOK)
				cam_warn("%s mcmcpy fail", __FUNCTION__);
			s_cfgdev.sbuf[sbuf_size - 1] = '\0';
			cam_warn("%s count[%zu] is beyond sbuf size[%zu]",
				__FUNCTION__, count, sizeof(s_cfgdev.sbuf));
		}
		ret = cam_cfgdev_guard_thermal();
	}
	cam_debug("%s ret is %d", __FUNCTION__, ret);
	return count;
}

static ssize_t guard_thermal_show(struct device_driver *drv, char *buf)
{
	char *offset = buf;
	int ret;

	(void)drv;
	if (!offset) {
		cam_err("%s input buf is null", __FUNCTION__);
		return 0;
	}

	cam_info("%s enter", __FUNCTION__);
	ret = snprintf_s(offset, PAGE_SIZE, PAGE_SIZE - 1,
		"guard thermal:[%s]\n", s_cfgdev.sbuf);
	if (ret < 0)
		cam_warn("%s snprintf_s fail", __FUNCTION__);
	offset += ret;
	return (offset - buf);
}

static void cam_cfgdev_dump_meminfo(void)
{
	struct v4l2_event ev = {
		.type = CAM_V4L2_EVENT_TYPE,
		.id = CAM_CFGDEV_REQUEST,
	};
	cam_cfgreq2dev_t *req = (cam_cfgreq2dev_t *)ev.u.data;
	if (!req) {
		cam_err("%s config req is null", __FUNCTION__);
		return;
	}

	req->req.intf = NULL;
	req->kind = CAM_CFGDEV_REQ_DUMP_MEMINFO;
	(void)cam_cfgdev_send_req(NULL, &ev, &s_cfgdev.rq, 1, NULL);
}

static ssize_t dump_meminfo_store(struct device_driver *drv,
	const char *buf, size_t count)
{
	if (!buf)
		return 0;
	return strnlen(buf, count);
}

static ssize_t dump_meminfo_show(struct device_driver *drv, char *buf)
{
	(void)drv;
	(void)buf;
	cam_info("%s enter", __FUNCTION__);
	cam_cfgdev_dump_meminfo();
	return 0;
}

#ifdef DEBUG_HISI_CAMERA
static ssize_t cam_log_store(struct device_driver *drv,
	const char *buf, size_t count)
{
	char *p = (char *)buf;
	u32 val;

	if (!p) {
		cam_err("%s input param buf is null", __FUNCTION__);
		return 0;
	}

	if (p[1] == 'x' || p[1] == 'X' || p[0] == 'x' || p[0] == 'X') {
		p++;
		if (p[0] == 'x' || p[0] == 'X')
			p++;
		val = simple_strtoul(p, &p, 16); /* 16, len */
	} else {
		val = simple_strtoul(p, &p, 10); /* 10, len */
	}

	if (p == buf)
		printk(KERN_INFO ": %s is not in hex or decimal form.\n", buf);
	else
		cam_debug_mask = val;

	return strnlen(buf, count);
}

static ssize_t cam_log_show(struct device_driver *drv, char *buf)
{
	int ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
		"0x%08x\n", cam_debug_mask);
	if (ret < 0)
		cam_warn("%s snprintf_s fail", __FUNCTION__);
	(void)drv;
	return ret;
}
#endif

static void cam_cfgdev_work(struct work_struct *w)
{
	(void)w;
}

static void cam_cfgdev_release_ack(cam_cfgack_t *ack)
{
	kzfree(ack);
}

int cam_cfgdev_queue_ack(struct v4l2_event *ev)
{
	cam_cfgack_t *ack = NULL;
	cam_cfgreq_t *req = NULL;

	req = (cam_cfgreq_t *)ev->u.data;
	if (req && req->one_way != 0) {
		CAM_CFG_ERR("need NOT acknowledge an one way"
			"request 0x%pK, 0x%08x, %u",
			req->intf, (*(unsigned *)(req + 1)), req->seq);
		return -EINVAL;
	}

	ack = kzalloc(sizeof(cam_cfgack_t), GFP_KERNEL);
	if (!ack) {
		CAM_CFG_ERR("out of memory for ack");
		return -ENOMEM;
	}
	ack->ev = *ev;
	ack->release = cam_cfgdev_release_ack;

	spin_lock(&s_ack_queue_lock);
	list_add_tail(&ack->node, &s_ack_queue);
	wake_up_all(&s_wait_ack);
	spin_unlock(&s_ack_queue_lock);

	return 0;
}

static bool cam_cfgdev_check_ack(
	cam_cfgreq_t *req, cam_cfgack_t **ppack)
{
	bool ret = s_cfgsvr_flags == CAM_CFGSVR_FLAG_UNPLAYED;

	cam_cfgack_t *ack = NULL;
	cam_cfgack_t *tmp = NULL;
	cam_cfgreq_t *back = NULL;

	spin_lock(&s_ack_queue_lock);
	list_for_each_entry_safe(ack, tmp, &s_ack_queue, node) {
		back = (cam_cfgreq_t *)ack->ev.u.data;
		if (req->user == back->user &&
			req->intf == back->intf &&
			req->seq == back->seq) {
			ret = true;
			*ppack = ack;
			list_del(&ack->node);
			break;
		}
	}
	spin_unlock(&s_ack_queue_lock);
	return ret;
}

static void cam_cfgdev_flush_ack_queue(void)
{
	cam_cfgack_t *ack = NULL;
	cam_cfgack_t *tmp = NULL;

	spin_lock(&s_ack_queue_lock);
	s_cfgsvr_flags = CAM_CFGSVR_FLAG_UNPLAYED;
	list_for_each_entry_safe(ack, tmp, &s_ack_queue, node) {
		list_del(&ack->node);
		ack->release(ack);
	}
	wake_up_all(&s_wait_ack);
	spin_unlock(&s_ack_queue_lock);
}

enum {
	CAM_WAIT4ACK_TIME = 10000, /* 10s */
	CAM_WAIT4CFGSVR_TIME = 3000, /* 3s */
};

static int cam_cfgdev_ack_deal(cam_user_intf_t *user,
	cam_cfgreq_t *req, int timeout, cam_cfgack_t **pack)
{
	int rc = -ETIME;
	int retry = 3;
	cam_cfgack_t *ack = NULL;

	while (true) {
		if (user)
			cam_user_intf_wait_begin(user);
		rc = wait_event_freezable_timeout(s_wait_ack,
			cam_cfgdev_check_ack(req, &ack), timeout); /*lint !e666*/
		if (user)
			cam_user_intf_wait_end(user);

		if (ack) {
			rc = 0;
			break;
		}

		if (rc == 0) {
			CAM_CFG_ERR("request(0x%pK, 0x%08x, %u) is out of time for ACK",
				req->intf, (*(unsigned *)(req + 1)), req->seq);
			rc = -ETIME;
			break;
		} else if (rc == -ERESTARTSYS) {
			CAM_CFG_INFO("request(0x%pK, 0x%08x, %u) is interrupted! "
				"pid: %d, tgid: %d, pending.signal: 0x%lx",
				req->intf, (*(unsigned *)(req + 1)), req->seq,
				current->pid, current->tgid, current->pending.signal.sig[0]);
			if (retry > 0) {
				retry--;
				continue;
			} else {
				rc = -ERESTART;
				break;
			}
		}
	}
	*pack = ack;

	return rc;
}

static int cam_cfgdev_wait_ack(cam_user_intf_t *user,
	cam_cfgreq_t *req, int timeout, int *ret)
{
	int rc;
	cam_cfgack_t *ack = NULL;
	timeout = msecs_to_jiffies(timeout);

	if (ret)
		*ret = -EINVAL;

	rc = cam_cfgdev_ack_deal(user, req, timeout, &ack);

	if (req->intf) {
		if (ack)
			cam_cfgreq_intf_on_ack(req->intf, ack);
		else
			cam_cfgreq_intf_on_cancel(req->intf, rc);
	}

	if (ack) {
		if (ret)
			*ret = cam_cfgack_result(ack);
		ack->release(ack);
	}

	return rc;
}

static int cam_cfgdev_thermal_guard(struct v4l2_event *ev)
{
	char *buf = (char *)ev->u.data;
	if (buf) {
		errno_t err = memcpy_s(buf, sizeof(s_cfgdev.sbuf),
			s_cfgdev.sbuf, sizeof(s_cfgdev.sbuf));
		if (err != EOK)
			cam_warn("%s memcpy_s fail", __FUNCTION__);
	}
	return 0;
}

int cam_cfgdev_send_req(cam_user_intf_t *user, struct v4l2_event *ev,
	struct v4l2_fh *target, int one_way, int *ret)
{
	int rc = 0;
	cam_cfgreq_t *req = (cam_cfgreq_t *)ev->u.data;
	if (!req)
		return -ENOENT;

	req->user = user;
	req->seq = atomic_add_return(1, &s_sequence);
	req->one_way = one_way ? 1 : 0;

	trace_cam_cfgdev_send_req_begin(req);

	mutex_lock(&s_cfgdev_lock);
	if (target->vdev) {
		v4l2_event_queue_fh(target, ev);
		if (req->intf)
			cam_cfgreq_intf_get(req->intf);
	} else {
		CAM_CFG_ERR("the target is invalid");
		rc = -ENOENT;
	}
	mutex_unlock(&s_cfgdev_lock);

	if (ret)
		*ret = rc;

	if (rc == 0 && req->one_way == 0)
		rc = cam_cfgdev_wait_ack(user, req, CAM_WAIT4ACK_TIME, ret);

	trace_cam_cfgdev_send_req_end(req);

	return rc;
}

int hw_is_binderized(void)
{
	return g_is_binderized;
}

char *gen_media_prefix(char *media_ent,
	cam_device_id_constants_t dev_const, size_t dst_size)
{
	int rc;
	if (dst_size >= 1) {
		rc = snprintf_s(media_ent, dst_size, dst_size - 1,
			"%d", dev_const);
		if (rc < 0)
			CAM_CFG_ERR("snprintf_s media_ent failed");
		strlcat(media_ent, "-", dst_size);
	}

	return media_ent;
}

int init_subdev_media_entity(struct v4l2_subdev *subdev,
	cam_device_id_constants_t dev_const)
{
	int rc;

	if (!subdev) {
		cam_err("%s input subdev is null", __FUNCTION__);
		return -ENOENT;
	}
	rc = media_entity_pads_init(&subdev->entity, 0, NULL);
	subdev->entity.obj_type = MEDIA_ENTITY_TYPE_V4L2_SUBDEV;
	return rc;
}

static void cam_cfgdev_subdev_release(struct video_device *vdev)
{
	struct v4l2_subdev *sd = video_get_drvdata(vdev);
	if (!sd)
		return;
	sd->devnode = NULL;
	video_device_release(vdev);

	CAM_CFG_INFO("TODO");
}

static unsigned int cam_subdev_poll(struct file *file, poll_table *wait)
{
	struct video_device *vdev = NULL;
	struct v4l2_subdev *sd = NULL;
	struct v4l2_fh *fh = NULL;

	if (!file)
		return POLLERR;

	vdev = video_devdata(file);
	if (!vdev)
		return POLLERR;

	sd = vdev_to_v4l2_subdev(vdev);
	if (!sd)
		return POLLERR;

	fh = file->private_data;
	if (!fh || (sd->flags & V4L2_SUBDEV_FL_HAS_EVENTS) == 0)
		return POLLERR;

	poll_wait(file, &fh->wait, wait);

	if (v4l2_event_pending(fh) != 0)
		return POLLIN;

	return 0;
}

static struct v4l2_file_operations g_cam_v4l2_subdev_fops = {};

static void gen_vname_for_mprefix(struct video_device *vdev,
	const char *media_prefix, const char *cfgdev_name)
{
	int rc;

	rc = snprintf_s(vdev->name, sizeof(vdev->name),
		sizeof(vdev->name) - 1, "%s", media_prefix);
	if (rc < 0)
		CAM_CFG_ERR("snprintf_s video device name failed");
	(void)strlcpy(vdev->name + strlen(vdev->name), cfgdev_name,
		sizeof(vdev->name) - strlen(vdev->name));
}

static void gen_vdentity_name_for_mprefix(struct video_device *vdev,
	const char *media_prefix)
{
	int rc;

	rc = snprintf_s(vdev->name + strlen(vdev->name),
		sizeof(vdev->name) - strlen(vdev->name),
		sizeof(vdev->name) - strlen(vdev->name) - 1,
		"%s", video_device_node_name(vdev));
	if (rc < 0) {
		CAM_CFG_ERR("Truncation Occurred");
		(void)snprintf_s(vdev->name, sizeof(vdev->name),
			sizeof(vdev->name) - 1, "%s", media_prefix);
		(void)snprintf_s(vdev->name + strlen(vdev->name),
			sizeof(vdev->name) - strlen(vdev->name),
			sizeof(vdev->name) - strlen(vdev->name) - 1,
			"%s", video_device_node_name(vdev));
	}
}

int cam_cfgdev_register_subdev(
	struct v4l2_subdev *sd, cam_device_id_constants_t dev_const)
{
	int rc;
	char media_prefix[10]; /* 10,array len */
	struct video_device *vdev = NULL;
	struct v4l2_device *v4l2 = &s_cfgdev.v4l2;

	if (!sd || !sd->name[0]) {
		rc = -EINVAL;
		goto out;
	}

	rc = v4l2_device_register_subdev(v4l2, sd);
	if (rc < 0)
		goto out;

	if ((sd->flags & V4L2_SUBDEV_FL_HAS_DEVNODE) == 0)
		goto out;

	vdev = video_device_alloc();
	if (!vdev) {
		rc = -ENOMEM;
		goto video_alloc_fail;
	}

	video_set_drvdata(vdev, sd);
	gen_media_prefix(media_prefix, dev_const, sizeof(media_prefix));
	gen_vname_for_mprefix(vdev, media_prefix, sd->name);
	vdev->v4l2_dev = v4l2;
	vdev->fops = &g_cam_v4l2_subdev_fops;
	vdev->release = cam_cfgdev_subdev_release;
	rc = __video_register_device(vdev, VFL_TYPE_SUBDEV, -1, 1, sd->owner);
	if (rc < 0)
		goto video_register_fail;
	cam_debug("register video devices %s sucessful", sd->name);
	cam_debug("video dev name %s %s", vdev->dev.kobj.name, vdev->name);
	sd->entity.info.dev.major = VIDEO_MAJOR;
	sd->entity.info.dev.minor = vdev->minor;

	gen_vdentity_name_for_mprefix(vdev, media_prefix);
	rc = 0;
	sd->entity.name = vdev->name;
	sd->devnode = vdev;
	goto out;

video_register_fail:
	video_device_release(vdev);

video_alloc_fail:
	if (sd->devnode)
		v4l2_device_unregister_subdev(sd);

out:
	return rc;
}

int cam_cfgdev_unregister_subdev(struct v4l2_subdev *sd)
{
	if (!sd)
		return -EINVAL;
	v4l2_device_unregister_subdev(sd);
	return 0;
}

static unsigned int cam_cfgdev_vo_poll(
	struct file *filep, struct poll_table_struct *ptbl)
{
	unsigned int rc = 0;

	if (!filep) {
		CAM_CFG_ERR("%s: filep is NULL", __FUNCTION__);
		return POLLERR;
	}
	if (!ptbl) {
		CAM_CFG_ERR("%s: ptbl is NULL", __FUNCTION__);
		return POLLERR;
	}

	mutex_lock(&s_cfgdev.lock);
	poll_wait(filep, &s_cfgdev.rq.wait, ptbl);
	if (v4l2_event_pending(&s_cfgdev.rq) != 0)
		rc = POLLIN | POLLRDNORM;
	mutex_unlock(&s_cfgdev.lock);
	return rc;
}

static long cam_cfgdev_vo_dqevent(struct v4l2_event *ev, int nb)
{
	long rc;
	cam_cfgreq_t *req = NULL;

	if (!ev)
		return -ENOENT;

	while (true) {
		rc = v4l2_event_dequeue(&s_cfgdev.rq, ev, nb);
		if (rc != 0)
			break;

		req = (cam_cfgreq_t *)ev->u.data;
		if (req->intf) {
			rc = cam_cfgreq_intf_on_req(req->intf, ev);
			cam_cfgreq_intf_put(req->intf);
			if (rc != 0) {
				CAM_CFG_ERR("request 0x%pK, 0x%08x, %u skipped",
					req->intf, (*(unsigned *)(req + 1)),
					req->seq);
				/*
				 * to skip failed/timeouted/interrupted events
				 */
				continue;
			}
		}
		break;
	}
	return rc;
}

static void cam_cfgdev_subscribed_event_ops_merge(
	const struct v4l2_event *old, struct v4l2_event *new)
{
	cam_cfgreq2dev_t *req = (cam_cfgreq2dev_t *)&old->u.data;
	if (req->req.intf)
		cam_cfgreq_intf_put(req->req.intf);
	CAM_CFG_ERR("the event queue overflowed");
	(void)new;
}

static struct v4l2_subscribed_event_ops s_cam_subscribed_event_ops = {
	.merge = cam_cfgdev_subscribed_event_ops_merge,
};

static long cam_cfgdev_vo_do_ioctl(
	struct file *filep, unsigned int cmd, void *arg)
{
	long rc = -EINVAL;

	if (!filep || !arg)
		return rc;

	switch (cmd) {
	case VIDIOC_DQEVENT:
		rc = cam_cfgdev_vo_dqevent(arg,
			filep->f_flags & O_NONBLOCK);
		break;

	case VIDIOC_SUBSCRIBE_EVENT:
		rc = v4l2_event_subscribe(&s_cfgdev.rq,
			(struct v4l2_event_subscription *)arg,
			8, /* 8, arg len */
			&s_cam_subscribed_event_ops);
		break;

	case VIDIOC_UNSUBSCRIBE_EVENT:
		rc = v4l2_event_unsubscribe(&s_cfgdev.rq,
			(struct v4l2_event_subscription *)arg);
		break;

	case CAM_V4L2_IOCTL_REQUEST_ACK:
		rc = cam_cfgdev_queue_ack((struct v4l2_event *)arg);
		break;

	case CAM_V4L2_IOCTL_THERMAL_GUARD:
		rc = cam_cfgdev_thermal_guard((struct v4l2_event *)arg);
		break;

	default:
		CAM_CFG_ERR("invalid IOCTL CMD %d", cmd);
		break;
	}
	return rc;
}

static long cam_cfgdev_vo_ioctl(
	struct file *filep,
	unsigned int cmd,
	unsigned long arg)
{
	return video_usercopy(filep, cmd, arg, cam_cfgdev_vo_do_ioctl);
}

#ifdef CONFIG_COMPAT
static long cam_cfgdev_vo_ioctl32(
	struct file *filep,
	unsigned int cmd,
	unsigned long arg)
{
	long rc;
	void __user *up_p = NULL;
	void __user *kp = NULL;
	up_p = compat_ptr(arg);

	switch (cmd) {
	case CAM_V4L2_IOCTL_REQUEST_ACK32:
		cmd = CAM_V4L2_IOCTL_REQUEST_ACK;
		break;
	case CAM_V4L2_IOCTL_THERMAL_GUARD32:
		cmd = CAM_V4L2_IOCTL_THERMAL_GUARD;
		break;
	default:
		CAM_CFG_ERR("case err cmd %u", cmd);
		break;
	}

	switch (cmd) {
	case CAM_V4L2_IOCTL_REQUEST_ACK:
	case CAM_V4L2_IOCTL_THERMAL_GUARD: {
		kp = compat_alloc_user_space(sizeof(struct v4l2_event));
		if (!kp)
			return -EFAULT;
		rc = compat_get_v4l2_event_data(kp, up_p);
		if (rc != 0)
			return rc;
		rc = cam_cfgdev_vo_ioctl(filep, cmd,
			(unsigned long)(uintptr_t)(kp));
		if (rc != 0)
			return rc;
		rc = compat_put_v4l2_event_data(kp, up_p);
		break;
	}
	default:
		rc = cam_cfgdev_vo_ioctl(filep, cmd, arg);
		break;
	}
	return rc;
}
#endif

static int cam_cfgdev_vo_close(struct file *filep)
{
	void *fpd = NULL;

	mutex_lock(&s_cfgdev.lock);
	if (filep)
		swap(filep->private_data, fpd);
	mutex_unlock(&s_cfgdev.lock);
	if (fpd) {
		struct pid *pid = NULL;

		mutex_lock(&s_cfgsvr_lock);
		swap(s_pid_cfgsvr, pid);

		mutex_lock(&s_cfgdev_lock);
		v4l2_fh_del(&s_cfgdev.rq);
		v4l2_fh_exit(&s_cfgdev.rq);
		mutex_unlock(&s_cfgdev_lock);

		if (pid)
			put_pid(pid);

		mutex_unlock(&s_cfgsvr_lock);
		cam_cfgdev_flush_ack_queue();

		CAM_CFG_INFO("the server %d detached", current->pid);
	}
	return 0;
}

static int cam_cfgdev_vo_open(struct file *filep)
{
	int rc = 0;

	mutex_lock(&s_cfgsvr_lock);
	if (s_pid_cfgsvr) {
		mutex_unlock(&s_cfgsvr_lock);
		CAM_CFG_INFO("only one server can attach to cfgdev");
		rc = -EBUSY;
		goto exit_open;
	}
	s_pid_cfgsvr = get_pid(task_pid(current));

	mutex_lock(&s_cfgdev_lock);
	v4l2_fh_init(&s_cfgdev.rq, s_cfgdev.vdev);
	v4l2_fh_add(&s_cfgdev.rq);
	mutex_unlock(&s_cfgdev_lock);
	mutex_unlock(&s_cfgsvr_lock);

	spin_lock(&s_ack_queue_lock);
	s_cfgsvr_flags = CAM_CFGSVR_FLAG_PLAYING;
	spin_unlock(&s_ack_queue_lock);

	if (filep)
		filep->private_data = &s_cfgdev;
	wake_up_all(&s_wait_cfgsvr);

	CAM_CFG_INFO("the server %d attached", current->pid);

exit_open:
	return rc;
}

static int cam_cfgdev_get_dts(struct platform_device *pdev)
{
	struct device *ptmpdev = NULL;
	struct device_node *np = NULL;
	int rc;

	if (!pdev) {
		CAM_CFG_ERR("pDev NULL");
		return -ENOMEM;
	}

	ptmpdev = &(pdev->dev);
	if (!ptmpdev) {
		CAM_CFG_ERR("pdev NULL");
		return -ENOMEM;
	}

	np = ptmpdev->of_node;
	if (!np) {
		CAM_CFG_ERR("of node NULL");
		return -ENOMEM;
	}

	rc = of_property_read_u32(np, "huawei,binderized",
		(u32 *)(&g_is_binderized));
	if (rc < 0) {
		CAM_CFG_ERR("get binderized flag failed");
		return -ENOMEM;
	}

	CAM_CFG_INFO("binderized=%d", g_is_binderized);
	return 0;
}

static struct v4l2_file_operations s_fops_cfgdev = {
	.owner = THIS_MODULE,
	.open = cam_cfgdev_vo_open,
	.poll = cam_cfgdev_vo_poll,
	.unlocked_ioctl = cam_cfgdev_vo_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl32 = cam_cfgdev_vo_ioctl32,
#endif
	.release = cam_cfgdev_vo_close,
};

static void cam_cfgdev_vo_subdev_notify(
	struct v4l2_subdev *sd,
	unsigned int notification,
	void *arg)
{
	CAM_CFG_INFO("TODO");
	(void)sd;
	(void)notification;
	(void)arg;
}

static int create_media_device(struct platform_device *pdev,
	struct media_device **pmdev)
{
	int rc;
	struct media_device *mdev = NULL;

	mdev = kzalloc(sizeof(struct media_device), GFP_KERNEL);
	if (!mdev) {
		return -ENOMEM;
	}

	rc = cam_cfgdev_get_dts(pdev);
	if (rc < 0)
		CAM_CFG_DEBUG("get dts failed");

	media_device_init(mdev);
	(void)strlcpy(mdev->model, CAM_MODEL_CFG, sizeof(mdev->model));
	if (pdev)
		mdev->dev = &(pdev->dev);

	rc = media_device_register(mdev);
	if (rc < 0) {
		kzfree(mdev);
		return rc;
	}
	*pmdev = mdev;

	return 0;
}

static void destroy_media_device(struct media_device *mdev)
{
	media_device_unregister(mdev);
	kzfree(mdev);
}

static int video_device_init(struct video_device *vdev,
	struct v4l2_device *v4l2)
{
	int rc;
	char media_prefix[10]; /* 10, array len */

	vdev->v4l2_dev = v4l2;
	(void)gen_media_prefix(media_prefix, CAM_VNODE_GROUP_ID,
		sizeof(media_prefix));

	gen_vname_for_mprefix(vdev, media_prefix, "hwcam-cfgdev");
	vdev->entity.obj_type = MEDIA_ENTITY_TYPE_VIDEO_DEVICE;
	vdev->release = video_device_release_empty;
	vdev->fops = &s_fops_cfgdev;
	vdev->minor = -1;
	vdev->vfl_type = VFL_TYPE_GRABBER;
	rc = video_register_device(vdev, VFL_TYPE_GRABBER, -1);
	if (rc < 0)
		return rc;
	cam_debug("video dev name %s %s", vdev->dev.kobj.name, vdev->name);

	gen_vdentity_name_for_mprefix(vdev, media_prefix);
	vdev->entity.name = vdev->name;
	vdev->lock = &s_cfgdev_lock;

	return 0;
}

static int cam_cfgdev_vo_probe(struct platform_device *pdev)
{
	int rc;

	struct video_device *vdev = NULL;
	struct media_device *mdev = NULL;
	struct v4l2_device *v4l2 = &s_cfgdev.v4l2;

	vdev = video_device_alloc();
	if (!vdev) {
		rc = -ENOMEM;
		goto probe_end;
	}

	rc = create_media_device(pdev, &mdev);
	if (rc != 0)
		goto media_alloc_fail;

	rc = media_entity_pads_init(&vdev->entity, 0, NULL);
	if (rc < 0)
		goto entity_init_fail;

	v4l2->mdev = mdev;
	v4l2->notify = cam_cfgdev_vo_subdev_notify;
	rc = v4l2_device_register(&(pdev->dev), v4l2);
	if (rc < 0)
		goto v4l2_register_fail;

	rc = video_device_init(vdev, v4l2);
	if (rc < 0)
		goto video_register_fail;

	video_set_drvdata(vdev, &s_cfgdev);

	s_cfgdev.vdev = vdev;
	s_cfgdev.mdev = mdev;

	s_cfgdev.debug_root = debugfs_create_dir("hwcam", NULL);
	mutex_init(&s_cfgdev.lock);

	if (!client_camera_user)
		client_camera_user = dsm_register_client(&g_dev_camera_user);

	goto probe_end;

video_register_fail:
	v4l2_device_unregister(v4l2);

v4l2_register_fail:
	media_entity_cleanup(&vdev->entity);

entity_init_fail:
	destroy_media_device(mdev);

media_alloc_fail:
	video_device_release(vdev);

probe_end:
	CAM_CFG_INFO("exit");
	return rc;
}

static int cam_cfgdev_vo_remove(struct platform_device *pdev)
{
	(void)pdev;
	video_unregister_device(s_cfgdev.vdev);
	v4l2_device_unregister(&s_cfgdev.v4l2);
	media_entity_cleanup(&(s_cfgdev.vdev)->entity);
	media_device_unregister(s_cfgdev.mdev);
	kzfree(s_cfgdev.mdev);
	video_device_release(s_cfgdev.vdev);
	s_cfgdev.vdev = NULL;
	s_cfgdev.mdev = NULL;
	mutex_destroy(&s_cfgdev.lock);
	return 0;
}

static const struct of_device_id s_cfgdev_devtbl_match[] = {
	{ .compatible = "huawei,camcfgdev" },
	{},
}
MODULE_DEVICE_TABLE(of, s_cfgdev_devtbl_match);

static struct platform_driver s_cfgdev_driver = {
	.probe = cam_cfgdev_vo_probe,
	.remove = cam_cfgdev_vo_remove,
	.driver = {
		.name = "huawei,camcfgdev",
		.owner = THIS_MODULE,
		.of_match_table = s_cfgdev_devtbl_match,
	},
};

static int __init cam_cfgdev_vo_init(void)
{
	int ret;
	g_cam_v4l2_subdev_fops = v4l2_subdev_fops;
	g_cam_v4l2_subdev_fops.poll = cam_subdev_poll;
	ret = platform_driver_register(&s_cfgdev_driver);
	if (ret == 0) {
		if (driver_create_file(&s_cfgdev_driver.driver,
			&driver_attr_guard_thermal) != 0)
			cam_warn("%s create driver attr failed", __FUNCTION__);

		if (driver_create_file(&s_cfgdev_driver.driver,
			&driver_attr_dump_meminfo) != 0)
			cam_warn("%s create driver attr dump_meminfo failed",
				__FUNCTION__);

#ifdef DEBUG_HISI_CAMERA
		if (driver_create_file(&s_cfgdev_driver.driver,
			&driver_attr_cam_log) != 0)
			cam_warn("%s create driver attr cam_log failed",
				__FUNCTION__);
#endif
	}
	return ret;
}

static void __exit cam_cfgdev_vo_exit(void)
{
	platform_driver_unregister(&s_cfgdev_driver);
}

module_init(cam_cfgdev_vo_init);
module_exit(cam_cfgdev_vo_exit);
MODULE_DESCRIPTION("Huawei V4L2 Camera");
MODULE_LICENSE("GPL v2");
