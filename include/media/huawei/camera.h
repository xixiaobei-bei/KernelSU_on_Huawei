/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: camera header file.
 * Author: yangkai
 * Create: 2016-03-15
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __HW_ALAN_MEDIA_CAMERA_H__
#define __HW_ALAN_MEDIA_CAMERA_H__

#if !defined(__KERNEL__)
#include <stdlib.h>
#else /* defined(__KERNEL__) */
#include <linux/bsearch.h>
#endif
#include <linux/videodev2.h>
#include <linux/compat.h>
#include <securec.h>

#define CAM_MODEL_USER     "hwcam_userdev"
#define CAM_MODEL_CFG      "hwcam_cfgdev"
#define HWSWINGCAM_MODEL_CFG "hwswingcam_cfgdev"

struct _tag_cam_cfgreq_intf;
struct _tag_cam_user_intf;

typedef enum _tag_cam_device_id_constants {
	CAM_DEVICE_GROUP_ID      = 0x10,

	CAM_VNODE_GROUP_ID       = 0x8000,

	CAM_SUBDEV_SENSOR        = 0x8010,
	CAM_SUBDEV_EXTISP        = 0x8020,
	CAM_SUBDEV_IDI           = 0x8030,
	CAM_SUBDEV_PMIC          = 0x8040,
	CAM_SUBDEV_FLASH0        = 0x8050,
	CAM_SUBDEV_FLASH1        = 0x8060,
	CAM_SUBDEV_FLASH2        = 0x8070,
	CAM_SUBDEV_ISP           = 0x8080,
	CAM_SUBDEV_VCM0          = 0x8090,
	CAM_SUBDEV_VCM1          = 0x80a0,
	CAM_SUBDEV_OIS           = 0X80b0,
	CAM_SUBDEV_HISP          = 0X80c0,
	CAM_SUBDEV_LASER         = 0x80d0,
	CAM_SUBDEV_DEPTHISP      = 0x80e0,
	CAM_SUBDEV_HJPEG         = 0x80f0,
	CAM_SUBDEV_GAZELED       = 0x8100,
	CAM_SUBDEV_FPGA          = 0x8110,
	CAM_SUBDEV_DRIVER_IC     = 0x8120,
	CAM_SUBDEV_DOT_PROJECTOR = 0x8130,
	CAM_SUBDEV_SWING         = 0x8140,
} cam_device_id_constants_t;

typedef enum _tag_cam_buf_kind {
	CAM_BUF_KIND_PIPELINE_CAPABILITY,
	CAM_BUF_KIND_STREAM_PARAM,
	CAM_BUF_KIND_STREAM,
} cam_buf_kind_t;

enum {
	CAM_GRAPHIC_BUF_INFO_LENGTH = 100,
};

enum {
	CAM_DATA_TABLE_ENTRY_UNUSED = 0,
	CAM_DATA_TABLE_ENTRY_USED = 1,
};

typedef struct _tag_cam_buf_info {
	cam_buf_kind_t kind;
	int              fd;
	uint32_t         size;
	uint32_t         frame;
	int              plane;
} cam_buf_info_t;

typedef struct _tag_cam_stream_info {
	cam_buf_info_t info;
	union {
		void *handle;
		int64_t _handle;
	};
} cam_stream_info_t;

typedef struct _tag_cam_graphic_buf_info {
	union {
		struct {
			uint32_t index;
			int      num_fds;
			int      num_ints;
			int      data[0];
		};
		int reserved[CAM_GRAPHIC_BUF_INFO_LENGTH];
	};
} cam_graphic_buf_info_t;

typedef struct _tag_cam_buf_status {
	int id;
	int buf_status;
	struct timeval tv;
} cam_buf_status_t;

enum {
	CAM_DATA_TABLE_NAME_SIZE = 32,
	CAM_DATA_ALIGNMENT = sizeof(uint32_t),
};

typedef struct _tag_cam_data_entry {
	int32_t        prev;
	int32_t        next;

	uint32_t const id;
	uint32_t const size;
	uint32_t const dim;
	uint32_t const offset;
	uint32_t       used;
} cam_data_entry_t;

/**
 * @brief a self-sufficient data table, ALL VARIABLES IN IT MUST BE SCALAR TYPE.
 */
typedef struct _tag_cam_data_table {
	char const         name[CAM_DATA_TABLE_NAME_SIZE];
	uint32_t const     total_size;
	uint32_t const     entry_count;

	cam_data_entry_t used_list;
	cam_data_entry_t unused_list;
	cam_data_entry_t entries[0];
} cam_data_table_t;

/* cdphy-CSI2 info */
typedef enum _phy_id_e {
	CDPHY_A = 0,
	CDPHY_B,
	CDPHY_C,
	CDPHY_D,
	CDPHY_MAX,
} phy_id_e;

typedef enum _phy_mode_e {
	PHY_MODE_DPHY = 0,
	PHY_MODE_CPHY,
	PHY_MODE_MAX,
} phy_mode_e;

typedef enum _phy_freq_mode_e {
	PHY_AUTO_FREQ = 0,
	PHY_MANUAL_FREQ,
	PHY_FREQ_MODE_MAX,
} phy_freq_mode_e;

typedef enum _phy_work_mode_e {
	PHY_SINGLE_MODE = 0,
	PHY_DUAL_MODE_SENSORA, /* dphy use DL1&3,cphy use DL2 */
	PHY_DUAL_MODE_SENSORB, /* dphy use DL0&2,cphy use DL0&1 */
	PHY_WORK_MODE_MAX,
} phy_work_mode_e;

#define PHY_INFO_IS_MASTER_SENSOR_LEN 2
#define PHY_INFO_IS_ID_LEN 2
#define PHY_INFO_IS_MODE_LEN 2
#define PHY_INFO_IS_FREQ_MODE_LEN 2
#define PHY_INFO_IS_FREQ_LEN 2
#define PHY_INFO_IS_WORD_MODE_LEN 2

typedef struct _phy_info_t {
	unsigned int is_master_sensor[PHY_INFO_IS_MASTER_SENSOR_LEN];
	phy_id_e phy_id[PHY_INFO_IS_ID_LEN];
	phy_mode_e phy_mode[PHY_INFO_IS_MODE_LEN];
	phy_freq_mode_e phy_freq_mode[PHY_INFO_IS_FREQ_MODE_LEN];
	unsigned int phy_freq[PHY_INFO_IS_FREQ_LEN];
	phy_work_mode_e phy_work_mode[PHY_INFO_IS_WORD_MODE_LEN];
} phy_info_t;

typedef void (*pfn_data_table_entry_handler)
	(cam_data_entry_t *entry, void *data, uint32_t dim);

static inline bool cam_data_table_empty(cam_data_table_t *tbl)
{
	return tbl->used_list.next == tbl->used_list.prev;
}

static inline bool cam_data_table_full(cam_data_table_t *tbl)
{
	return tbl->unused_list.next == tbl->unused_list.prev;
}

static inline bool cam_data_table_is_entry_valid(
	cam_data_table_t const *tbl, cam_data_entry_t const *entry)
{
	return &(tbl->used_list) <= entry &&
		entry < tbl->entries + tbl->entry_count;
}

static inline cam_data_entry_t *cam_data_table_get_first_entry(
	cam_data_table_t *tbl)
{
	cam_data_entry_t *next = (cam_data_entry_t *)
		(((uint8_t *)tbl) + tbl->used_list.next);
	return !cam_data_table_is_entry_valid(tbl, next) ||
		next == &tbl->used_list ? NULL : next;
}

static inline cam_data_entry_t *cam_data_table_get_next_entry(
	cam_data_table_t *tbl, cam_data_entry_t *cur)
{
	cam_data_entry_t *next = (cam_data_entry_t *)
		(((uint8_t *)tbl) + cur->next);
	return !cam_data_table_is_entry_valid(tbl, next) ||
		next == &tbl->used_list ? NULL : next;
}

static inline uint32_t cam_data_table_entry_index(cam_data_table_t *tbl,
	cam_data_entry_t *cur)
{
	return tbl->entries <= cur && cur < tbl->entries + tbl->entry_count ?
		cur - tbl->entries : -1u;
}

static inline bool cam_data_table_offset_is_valid(
	cam_data_table_t const *tbl, uint32_t offset)
{
	return sizeof(cam_data_table_t) + tbl->entry_count *
		sizeof(cam_data_entry_t) <= offset &&
		offset < tbl->total_size;
}

static inline void *cam_data_table_data_ref(cam_data_table_t *tbl,
	cam_data_entry_t const *cur, uint32_t *dim)
{
	uint32_t offset = cur ? cur->offset : 0;
	if (cam_data_table_offset_is_valid(tbl, offset)) {
		void *data = (uint8_t *)tbl + offset;
		if (dim)
			*dim = 1 < cur->dim ? *((uint32_t *)data - 1) : 1;
		return data;
	} else {
		return NULL;
	}
}

static inline void cam_data_table_enumerate(cam_data_table_t *tbl,
	pfn_data_table_entry_handler handler)
{
	cam_data_entry_t *cur = NULL;
	if (!tbl)
		return;
	cur = cam_data_table_get_first_entry(tbl);
	while (cur) {
		uint32_t dim = 1;
		void *data = cam_data_table_data_ref(tbl, cur, &dim);
		handler(cur, data, dim);
		cur = cam_data_table_get_next_entry(tbl, cur);
	}
}

static inline int cam_data_entry_compare(void const *e1, void const *e2)
{
	uint32_t i1 = ((cam_data_entry_t const *)e1)->id;
	uint32_t i2 = ((cam_data_entry_t const *)e2)->id;
	if (i1 < i2)
		return -1;
	else if (i2 < i1)
		return 1;
	else
		return 0;
}

static inline cam_data_entry_t *cam_data_table_find_entry_by_id(
	cam_data_table_t *tbl, uint32_t id)
{
	cam_data_entry_t eid = { .prev = 0, .next = 0,
		.id = id, .size = 0, .dim = 0, .offset = 0, };
	return (cam_data_entry_t *)
		bsearch(&eid, tbl->entries, tbl->entry_count,
			sizeof(cam_data_entry_t), cam_data_entry_compare);
}

static inline cam_data_entry_t *cam_data_table_get_entry_by_index(
	cam_data_table_t *tbl, uint32_t index)
{
	return index < tbl->entry_count ? tbl->entries + index : NULL;
}

static inline void cam_data_table_reset_used_list_status(
	cam_data_table_t *tbl,
	cam_data_entry_t *fu,
	cam_data_entry_t *lu)
{
	cam_data_entry_t *cur = fu;
	if (!tbl)
		return;
	while (cur && cur != lu) {
		cur->used = CAM_DATA_TABLE_ENTRY_UNUSED;
		cur = cam_data_table_get_next_entry(tbl, cur);
	}
	if (cur)
		cur->used = CAM_DATA_TABLE_ENTRY_UNUSED;
}

static inline void cam_data_table_reset(cam_data_table_t *tbl)
{
	cam_data_entry_t *fu =
		(cam_data_entry_t *)((uint8_t *)tbl + tbl->used_list.next);
	cam_data_entry_t *lu =
		(cam_data_entry_t *)((uint8_t *)tbl + tbl->used_list.prev);
	cam_data_entry_t *ln = NULL;

	if (fu != &tbl->used_list && lu != &tbl->used_list) {
		cam_data_table_reset_used_list_status(tbl, fu, lu);
		ln = (cam_data_entry_t *)((uint8_t *)tbl +
			tbl->unused_list.prev);

		fu->prev = tbl->unused_list.prev;
		lu->next = ln->next;
		tbl->unused_list.prev = tbl->used_list.prev;
		ln->next = tbl->used_list.next;
	}

	tbl->used_list.prev = tbl->used_list.next =
		(uint8_t *)&tbl->used_list - (uint8_t *)tbl;
}

static inline int cam_data_table_set_entry_data(cam_data_table_t *tbl,
	cam_data_entry_t *tgt, void const *buf, uint32_t size, uint32_t dim)
{
	if (tbl->entries <= tgt &&
		tgt < tbl->entries + tbl->entry_count &&
		buf && tgt->size == size && dim <= tgt->dim &&
		cam_data_table_offset_is_valid(tbl, tgt->offset)) {
		void *data = (uint8_t *)tbl + tgt->offset;
		cam_data_entry_t *lu = NULL;

		cam_data_entry_t *prev =
			(cam_data_entry_t *)((uint8_t *)tbl + tgt->prev);
		cam_data_entry_t *next =
			(cam_data_entry_t *)((uint8_t *)tbl + tgt->next);

		prev->next = tgt->next;
		next->prev = tgt->prev;

		lu = (cam_data_entry_t *)((uint8_t *)tbl +
			tbl->used_list.prev);

		tgt->prev = tbl->used_list.prev;
		tgt->next = lu->next;
		lu->next = tbl->used_list.prev =
			(uint8_t *)tgt - (uint8_t *)tbl;
		tgt->used = CAM_DATA_TABLE_ENTRY_USED;

		if (memcpy_s(data, dim * size, buf, dim * size) != EOK)
			return -EINVAL;

		if (tgt->dim > 1)
			*((uint32_t *)data - 1) = dim;

		return 0;
	}
	return -EINVAL;
}

static inline int cam_data_table_set_data(cam_data_table_t *tbl,
	uint32_t id, void const *buf, uint32_t size, uint32_t dim)
{
	cam_data_entry_t *tgt = cam_data_table_find_entry_by_id(tbl, id);
	return cam_data_table_set_entry_data(tbl, tgt, buf, size, dim);
}

static inline int cam_data_table_merge_data(cam_data_table_t *tbl_to,
	cam_data_table_t *tbl_from)
{
	if (tbl_to && tbl_from && tbl_to != tbl_from) {
		if (strncmp(tbl_to->name, tbl_from->name,
			CAM_DATA_TABLE_NAME_SIZE)) {
			cam_data_entry_t *src =
				cam_data_table_get_first_entry(tbl_from);
			while (src) {
				uint32_t dim = 1;
				void const *data = cam_data_table_data_ref(
					tbl_from, src, &dim);
				uint32_t index = cam_data_table_entry_index(
					tbl_from, src);
				cam_data_entry_t *tgt =
					cam_data_table_get_entry_by_index(
						tbl_to, index);
				cam_data_table_set_entry_data(tbl_to, tgt,
					data, src->size, dim);
				src = cam_data_table_get_next_entry(tbl_from,
					src);
			}
		} else {
			cam_data_entry_t *src =
				cam_data_table_get_first_entry(tbl_from);
			while (src) {
				uint32_t dim = 1;
				void const *data =
					cam_data_table_data_ref(tbl_from,
						src, &dim);
				cam_data_table_set_data(tbl_to, src->id,
					data, src->size, dim);
				src = cam_data_table_get_next_entry(tbl_from,
					src);
			}
		}
		return 0;
	}
	return -EINVAL;
}

static inline int cam_data_table_set_entry_as_used(cam_data_table_t *tbl,
	cam_data_entry_t *tgt, int used)
{
	if (tbl->entries <= tgt && tgt < tbl->entries + tbl->entry_count) {
		cam_data_entry_t *prev =
			(cam_data_entry_t *)((uint8_t *)tbl + tgt->prev);
		cam_data_entry_t *next =
			(cam_data_entry_t *)((uint8_t *)tbl + tgt->next);

		prev->next = tgt->next;
		next->prev = tgt->prev;

		if (used != 0) {
			cam_data_entry_t *lu =
				(cam_data_entry_t *)((uint8_t *)tbl +
				tbl->used_list.prev);

			tgt->prev = tbl->used_list.prev;
			tgt->next = lu->next;
			lu->next = tbl->used_list.prev = (uint8_t *)tgt -
				(uint8_t *)tbl;
			tgt->used = CAM_DATA_TABLE_ENTRY_USED;
		} else {
			cam_data_entry_t *ln =
				(cam_data_entry_t *)((uint8_t *)tbl +
				tbl->unused_list.prev);

			tgt->prev = tbl->unused_list.prev;
			tgt->next = ln->next;
			ln->next = tbl->unused_list.prev = (uint8_t *)tgt -
				(uint8_t *)tbl;
			tgt->used = CAM_DATA_TABLE_ENTRY_UNUSED;
		}
		return 0;
	}
	return -EINVAL;
}

static inline int cam_data_table_set_as_used(cam_data_table_t *tbl,
	uint32_t id, int used)
{
	cam_data_entry_t *tgt = cam_data_table_find_entry_by_id(tbl, id);
	return cam_data_table_set_entry_as_used(tbl, tgt, used);
}

#define CAM_V4L2_IOCTL_GET_BUF \
	_IOR('A', BASE_VIDIOC_PRIVATE + 0x06, cam_buf_status_t)

#define CAM_V4L2_IOCTL_PUT_BUF \
	_IOW('A', BASE_VIDIOC_PRIVATE + 0x07, cam_buf_status_t)

#define CAM_V4L2_IOCTL_BUF_DONE \
	_IOW('A', BASE_VIDIOC_PRIVATE + 0x08, cam_buf_status_t)

#define CAM_V4L2_IOCTL_REQUEST_ACK \
	_IOW('A', BASE_VIDIOC_PRIVATE + 0x20, struct v4l2_event)

#define CAM_V4L2_IOCTL_NOTIFY \
	_IOW('A', BASE_VIDIOC_PRIVATE + 0x21, struct v4l2_event)

#define CAM_V4L2_IOCTL_THERMAL_GUARD \
	_IOWR('A', BASE_VIDIOC_PRIVATE + 0x22, struct v4l2_event)

typedef enum _tag_cam_v4l2_cid_kind {
	CAM_V4L2_CID_PIPELINE_MIN = V4L2_CID_PRIVATE_BASE,
	CAM_V4L2_CID_PIPELINE_PARAM,
	CAM_V4L2_CID_PIPELINE_MAX,

	CAM_V4L2_CID_STREAM_MIN = V4L2_CID_PRIVATE_BASE + 0x1000,
	CAM_V4L2_CID_STREAM_PARAM,
	CAM_V4L2_CID_STREAM_MAX,
} cam_v4l2_cid_kind_t;

typedef enum _tag_cam_cfgreq_constants {
	CAM_V4L2_EVENT_TYPE     = V4L2_EVENT_PRIVATE_START + 0x00001000,
	CAM_CFGDEV_REQUEST      = 0x1000,
	CAM_CFGPIPELINE_REQUEST = 0x2000,
	CAM_CFGSTREAM_REQUEST   = 0x3000,
	CAM_SERVER_CRASH        = 0x4000,
	CAM_HARDWARE_SUSPEND    = 0x5001,
	CAM_HARDWARE_RESUME     = 0x5002,
	CAM_NOTIFY_USER         = 0x6000,
} cam_cfgreq_constants_t;

typedef enum _tag_cam_cfgreq2dev_kind {
	CAM_CFGDEV_REQ_MIN = CAM_CFGDEV_REQUEST,
	CAM_CFGDEV_REQ_MOUNT_PIPELINE,
	CAM_CFGDEV_REQ_GUARD_THERMAL,
	CAM_CFGDEV_REQ_DUMP_MEMINFO,
	CAM_CFGDEV_REQ_MAX,
} cam_cfgreq2dev_kind_t;

/* add for 32+64 */
#if 1
typedef struct _tag_cam_cfgreq {
	union {
		struct _tag_cam_user_intf *user;
		int64_t _user;
	};
	union {
		struct _tag_cam_cfgreq_intf *intf;
		int64_t _intf;
	};
	uint32_t seq;
	int rc;
	uint32_t one_way : 1;
} cam_cfgreq_t;
#else
typedef struct _tag_cam_cfgreq {
	cam_user_intf_t   *user;
	cam_cfgreq_intf_t *intf;
	unsigned long       seq;
	int                 rc;
	unsigned long       one_way : 1;
} cam_cfgreq_t;
#endif

typedef struct _tag_cam_cfgreq2dev {
	cam_cfgreq_t req;
	cam_cfgreq2dev_kind_t kind;
	union { /* can ONLY place 10 int fields here. */
		struct {
			int fd;
			int moduleID;
		} pipeline;
	};
} cam_cfgreq2dev_t;

typedef enum _tag_cam_cfgreq2pipeline_kind {
	CAM_CFGPIPELINE_REQ_MIN = CAM_CFGPIPELINE_REQUEST,
	CAM_CFGPIPELINE_REQ_UNMOUNT,
	CAM_CFGPIPELINE_REQ_MOUNT_STREAM,

	CAM_CFGPIPELINE_REQ_MOUNT_BUF,
	CAM_CFGPIPELINE_REQ_UNMOUNT_BUF,

	CAM_CFGPIPELINE_REQ_ENUM_FMT,
	CAM_CFGPIPELINE_REQ_QUERY_CAPABILITY,

	CAM_CFGPIPELINE_REQ_QUERY_PARAM,
	CAM_CFGPIPELINE_REQ_CHANGE_PARAM,

	CAM_CFGPIPELINE_REQ_MAX,
} cam_cfgreq2pipeline_kind_t;

typedef struct _tag_cam_cfgreq2pipeline {
	cam_cfgreq_t req;
	cam_cfgreq2pipeline_kind_t kind;
	union { /* can ONLY place 10 int fields here. */
		struct {
			int fd;
			cam_buf_info_t info;
			union {
				void *handle;
				int64_t _handle;
			};
		} stream;
		cam_buf_info_t buf;
	};
} cam_cfgreq2pipeline_t;

typedef enum _tag_cam_cfgreq2stream_kind {
	CAM_CFGSTREAM_REQ_MIN = CAM_CFGSTREAM_REQUEST,
	CAM_CFGSTREAM_REQ_UNMOUNT,

	CAM_CFGSTREAM_REQ_MOUNT_BUF,
	CAM_CFGSTREAM_REQ_UNMOUNT_BUF,
	CAM_CFGSTREAM_REQ_MOUNT_GRAPHIC_BUF,
	CAM_CFGSTREAM_REQ_UNMOUNT_GRAPHIC_BUF,

	CAM_CFGSTREAM_REQ_QUERY_PARAM,
	CAM_CFGSTREAM_REQ_CHANGE_PARAM,

	CAM_CFGSTREAM_REQ_TRY_FMT,

	CAM_CFGSTREAM_REQ_START,
	CAM_CFGSTREAM_REQ_STOP,

	CAM_CFGSTREAM_REQ_MAX,
} cam_cfgreq2stream_kind_t;

typedef struct _tag_cam_cfgreq2stream {
	cam_cfgreq_t req;
	cam_cfgreq2stream_kind_t kind;
	union { /* can ONLY place 10 int fields here. */
		cam_buf_info_t buf;
		struct {
			int index;
		} unmount_graphic_buf;
	};
} cam_cfgreq2stream_t;

#endif /* __HW_ALAN_MEDIA_CAMERA_H__ */
