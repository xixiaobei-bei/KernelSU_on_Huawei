/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef HISI_OVERLAY_UTILS_STRUCT_H
#define HISI_OVERLAY_UTILS_STRUCT_H

#include "../hisi_dss.h"

struct hisi_ov_compose_rect {
	dss_rect_t *wb_dst_rect;
	dss_rect_t *wb_ov_block_rect;
	dss_rect_ltrb_t *clip_rect;
	dss_rect_t *aligned_rect;
};

struct hisi_ov_compose_flag {
	bool rdma_stretch_enable;
	bool has_base;
	bool csc_needed;
	bool enable_cmdlist;
};

struct ov_module_set_regs_flag {
	bool enable_cmdlist;
	bool is_first_ov_block;
	int task_end;
	int last;
};

#endif  /* HISI_OVERLAY_UTILS_STRUCT_H */
