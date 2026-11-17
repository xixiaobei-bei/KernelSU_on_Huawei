/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: camera trace header file
 * Author: yangkai
 * Create: 2016-03-15
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

#undef TRACE_SYSTEM
#define TRACE_SYSTEM hwcamcfgdrv

#if !defined(__TRACE_CAM_H__) || defined(TRACE_HEADER_MULTI_READ)
#define __TRACE_CAM_H__

#include <linux/tracepoint.h>
#include <media/huawei/camera.h>
#include <media/huawei/isp_cfg.h>

DECLARE_EVENT_CLASS(
	cam_cfgdev_req_internel, /* [false alarm]:will not cause error */
	TP_PROTO(cam_cfgreq_t *req),
	TP_ARGS(req),

	TP_STRUCT__entry(
		__field(uint32_t, kind)
		__field(uint32_t, seq)
	),

	TP_fast_assign(
		__entry->kind = (*(uint32_t *)(req + 1));
		__entry->seq = req->seq;
	),

	TP_printk("kind=0x%x seq=%d",
		__entry->kind, __entry->seq)
);

DEFINE_EVENT(cam_cfgdev_req_internel, cam_cfgdev_send_req_begin,
	TP_PROTO(cam_cfgreq_t *req), TP_ARGS(req));

DEFINE_EVENT(cam_cfgdev_req_internel, cam_cfgdev_send_req_end,
	TP_PROTO(cam_cfgreq_t *req), TP_ARGS(req));

#endif /* __TRACE_CAM_H__ */

/* This part must be outside protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE trace_hwcam
#include <trace/define_trace.h>