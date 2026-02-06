/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: Hisilicon hwcam intf header file.
 * Author: yangkai
 * Create: 2016-03-05
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

#ifndef __HW_ALAN_KERNEL_CAMERA_OBJ_MDL_H__
#define __HW_ALAN_KERNEL_CAMERA_OBJ_MDL_H__

#include <linux/dma-buf.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/videodev2.h>
#include <media/huawei/camera.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>

#define CAM_CFG_ERR(fmt, arg...) \
	printk(KERN_ERR "%s(%d, %d): " fmt "\n", \
		__FUNCTION__, __LINE__, current->pid, ## arg)

#define CAM_CFG_INFO(fmt, arg...) \
	printk(KERN_INFO "%s(%d, %d): " fmt "\n", \
		__FUNCTION__, __LINE__, current->pid, ## arg)

#define CAM_CFG_DEBUG(fmt, arg...) \
	printk(KERN_DEBUG "%s(%d, %d): " fmt "\n", \
		__FUNCTION__, __LINE__, current->pid, ## arg)

#define AVAIL_NAME_LENGTH 31

typedef struct _tag_cam_vbuf {
	struct vb2_buffer buf;
	struct list_head node;
} cam_vbuf_t;

struct _tag_cam_cfgstream_intf;
struct _tag_cam_cfgpipeline_intf;
struct _tag_cam_dev_intf;
struct _tag_cam_user_intf;
struct _tag_cam_cfgreq_mount_stream_intf;
struct _tag_cam_cfgreq_mount_pipeline_intf;
struct _tag_cam_cfgreq_intf;
struct _tag_cam_cfgack;

/* cam_cfgreq interface definition begin */
typedef struct _tag_cam_cfgreq_vtbl {
	void (*get)(struct _tag_cam_cfgreq_intf *intf);
	int (*put)(struct _tag_cam_cfgreq_intf *intf);
	int (*on_req)(struct _tag_cam_cfgreq_intf *intf,
		struct v4l2_event *ev);
	int (*on_cancel)(struct _tag_cam_cfgreq_intf *intf,
		int reason);
	int (*on_ack)(struct _tag_cam_cfgreq_intf *intf,
		struct _tag_cam_cfgack *ack);
} cam_cfgreq_vtbl_t;

typedef struct _tag_cam_cfgreq_intf {
	struct _tag_cam_cfgreq_vtbl const *vtbl;
} cam_cfgreq_intf_t;

static inline void cam_cfgreq_intf_get(cam_cfgreq_intf_t *intf)
{
	return intf->vtbl->get(intf);
}

static inline int cam_cfgreq_intf_put(cam_cfgreq_intf_t *intf)
{
	return intf->vtbl->put(intf);
}

static inline int cam_cfgreq_intf_on_req(cam_cfgreq_intf_t *intf,
	struct v4l2_event *ev)
{
	return intf->vtbl->on_req(intf, ev);
}

static inline int cam_cfgreq_intf_on_cancel(cam_cfgreq_intf_t *intf,
	int reason)
{
	return intf->vtbl->on_cancel(intf, reason);
}

static inline int cam_cfgreq_intf_on_ack(cam_cfgreq_intf_t *intf,
	struct _tag_cam_cfgack *ack)
{
	return intf->vtbl->on_ack(intf, ack);
}

static inline int cam_cfgreq_on_ack_noop(cam_cfgreq_intf_t *pintf,
	struct _tag_cam_cfgack *ack)
{
	return 0;
}

typedef void (*pfn_cam_cfgdev_release_ack)(struct _tag_cam_cfgack *ack);

typedef struct _tag_cam_cfgack {
	struct list_head node;
	pfn_cam_cfgdev_release_ack release;
	struct v4l2_event ev;
} cam_cfgack_t;

static inline int cam_cfgack_result(struct _tag_cam_cfgack *ack)
{
	return ((cam_cfgreq_t *)ack->ev.u.data)->rc;
}

/* cam_usr interface definition begin */
typedef struct _tag_cam_user_vtbl {
	void (*get)(struct _tag_cam_user_intf *intf);
	int (*put)(struct _tag_cam_user_intf *intf);
	void (*wait_begin)(struct _tag_cam_user_intf *intf);
	void (*wait_end)(struct _tag_cam_user_intf *intf);
	void (*notify)(struct _tag_cam_user_intf *intf,
		struct v4l2_event *ev);
} cam_user_vtbl_t;

typedef struct _tag_cam_user_intf {
	cam_user_vtbl_t const *vtbl;
} cam_user_intf_t;

static inline void cam_user_intf_get(cam_user_intf_t *intf)
{
	intf->vtbl->get(intf);
}

static inline int cam_user_intf_put(cam_user_intf_t *intf)
{
	return intf->vtbl->put(intf);
}

static inline void cam_user_intf_wait_begin(cam_user_intf_t *intf)
{
	intf->vtbl->wait_begin(intf);
}

static inline void cam_user_intf_wait_end(cam_user_intf_t *intf)
{
	intf->vtbl->wait_end(intf);
}

static inline void cam_user_intf_notify(cam_user_intf_t *intf,
	struct v4l2_event *ev)
{
	intf->vtbl->notify(intf, ev);
}
/* cam_usr interface definition end */

/* cam_dev interface definition begin */
extern int cam_dev_create(struct device *dev, int *dev_num);
/* cam_dev interface definition end */

/* cam_cfgdev interface definition begin */
extern void cam_cfgdev_lock(void);
extern void cam_cfgdev_unlock(void);
extern int cam_cfgdev_queue_ack(struct v4l2_event *ev);
extern int cam_cfgdev_send_req(cam_user_intf_t *user,
	struct v4l2_event *ev,
	struct v4l2_fh *target,
	int one_way,
	int *rc);
extern int cam_cfgdev_register_subdev(struct v4l2_subdev *subdev,
	cam_device_id_constants_t dev_const);
extern int cam_cfgdev_unregister_subdev(struct v4l2_subdev *subdev);
/* cam_cfgdev interface definition end */

extern int hw_is_binderized(void);
extern char *gen_media_prefix(char *media_ent,
	cam_device_id_constants_t dev_const, size_t dst_size);
extern int init_subdev_media_entity(struct v4l2_subdev *subdev,
	cam_device_id_constants_t dev_const);

#endif /* __HW_ALAN_KERNEL_CAMERA_OBJ_MDL_H__ */
