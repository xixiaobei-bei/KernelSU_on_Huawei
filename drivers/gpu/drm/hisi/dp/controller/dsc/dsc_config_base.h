#ifndef __DPTX_DP_DSC_CFG_H__
#define __DPTX_DP_DSC_CFG_H__
#include "hisi_dp_drv.h"

void dptx_program_pps_sdps(struct dp_ctrl *dptx);
void dptx_dsc_enable(struct dp_ctrl *dptx);

int dptx_dsc_get_clock_div(struct dp_ctrl *dptx);
int dptx_slice_height_limit(struct dp_ctrl *dptx, u32 pic_height);
int dptx_line_buffer_depth_limit(struct dp_ctrl *dptx, u8 line_buf_depth);
void dptx_dsc_notice_rx(struct dp_ctrl *dptx);
void dptx_dsc_cfg(struct dp_ctrl *dptx);
void dptx_dsc_sdp_manul_send(struct dp_ctrl *dptx, bool enable);
#endif
