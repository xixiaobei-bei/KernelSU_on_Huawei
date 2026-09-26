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

#ifndef HISI_DSS_TOOL_H
#define HISI_DSS_TOOL_H

#include "../hisi_fb.h"

int hisi_dss_handle_cur_ovl_req(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req);
int hisi_get_hal_format(struct fb_info *info);
bool hal_format_has_alpha(uint32_t format);
bool is_pixel_10bit2dma(int format);
bool is_pixel_10bit2dfc(int format);
bool is_yuv_package(uint32_t format);
bool is_yuv_semiplanar(uint32_t format);
bool is_yuv_plane(uint32_t format);
bool is_yuv(uint32_t format);
bool is_yuv_sp_420(uint32_t format);
bool is_yuv_sp_422(uint32_t format);
bool is_yuv_p_420(uint32_t format);
bool is_yuv_p_422(uint32_t format);
bool is_rgbx(uint32_t format);
uint32_t is_need_rdma_stretch_bit(struct dpu_fb_data_type *dpufd, dss_layer_t *layer);
bool is_arsr_post_need_padding(dss_overlay_t *pov_req, struct dpu_panel_info *pinfo, uint32_t ihinc);
int hisi_adjust_clip_rect(dss_layer_t *layer, dss_rect_ltrb_t *clip_rect);
int hisi_pixel_format_hal2dma(int format);
int hisi_transform_hal2dma(int transform, int chn_idx);


#endif /* HISI_DSS_TOOL_H */
