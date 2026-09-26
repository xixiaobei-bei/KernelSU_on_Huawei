/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: Implement of cvdr config.
 * Author: lixiuhua
 * Create: 2017-02-28
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

#include <linux/iommu.h>
#include <asm/io.h>
#include "cvdr_cfg.h"
#include "cam_log.h"

#if 1

void wr_port_25_update_fail_logs_dbg(void __iomem *viraddr)
{
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_SRT_VP_WR_CFG_25_OFFSET_UPDATE,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_SRT_VP_WR_CFG_25_OFFSET_UPDATE)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_SRT_VP_WR_AXI_FS_25_OFFSET_UPDATE,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_SRT_VP_WR_AXI_FS_25_OFFSET_UPDATE)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET_UPDATE,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET_UPDATE)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_SRT_VP_WR_IF_CFG_25_OFFSET_UPDATE,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_SRT_VP_WR_IF_CFG_25_OFFSET_UPDATE)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_NR_RD_CFG_4,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_NR_RD_CFG_4)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_LIMITER_NR_RD_4,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_LIMITER_NR_RD_4)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_LIMITER_VP_WR_25_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_LIMITER_VP_WR_25_OFFSET)));
}

void wr_port_25_update_suc_logs_dbg(void __iomem *viraddr)
{
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_SRT_VP_WR_CFG_25_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_SRT_VP_WR_CFG_25_OFFSET)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_SRT_VP_WR_AXI_FS_25_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_SRT_VP_WR_AXI_FS_25_OFFSET)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_SRT_VP_WR_IF_CFG_25_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_SRT_VP_WR_IF_CFG_25_OFFSET)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_SRT_NR_RD_CFG_1_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_SRT_NR_RD_CFG_1_OFFSET)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_SRT_LIMITER_NR_RD_1_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_SRT_LIMITER_NR_RD_1_OFFSET)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_SRT_LIMITER_VP_WR_25_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_SRT_LIMITER_VP_WR_25_OFFSET)));
}

void wr_port_id0_logs_dbg(void __iomem *viraddr)
{
	int i;
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_VP_WR_CFG_0_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_VP_WR_CFG_0_OFFSET)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_VP_WR_AXI_FS_0_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_VP_WR_AXI_FS_0_OFFSET)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_VP_WR_AXI_LINE_0_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_VP_WR_AXI_LINE_0_OFFSET)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_VP_WR_IF_CFG_0_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_VP_WR_IF_CFG_0_OFFSET)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_LIMITER_VP_WR_0_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_LIMITER_VP_WR_0_OFFSET)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_NR_RD_CFG_0_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_NR_RD_CFG_0_OFFSET)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_NR_RD_DEBUG_0_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_NR_RD_DEBUG_0_OFFSET)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_LIMITER_NR_RD_0_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_LIMITER_NR_RD_0_OFFSET)));

	for (i = CVDR_AXI_JPEG_DEBUG_0_OFFSET;
		/* 4: step */
		i <= CVDR_AXI_JPEG_DEBUG_3_OFFSET; i += 4) {
		cam_debug("%s: CVDR debug_x_offset reg: 0x%08x=0x%08x",
			__FUNCTION__, i,
			get_reg_val((void __iomem*)((char*)viraddr + i)));
	}

	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_VP_WR_DEBUG_0_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_VP_WR_DEBUG_0_OFFSET)));
}

void wr_port_id30_logs_dbg(void __iomem *viraddr)
{
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_VP_WR_CFG_30_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_VP_WR_CFG_30_OFFSET)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_VP_WR_AXI_FS_30_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_VP_WR_AXI_FS_30_OFFSET)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_VP_WR_AXI_LINE_30_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_VP_WR_AXI_LINE_30_OFFSET)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_VP_WR_IF_CFG_30_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_VP_WR_IF_CFG_30_OFFSET)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_NR_RD_CFG_4,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_NR_RD_CFG_4)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_LIMITER_NR_RD_4,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_LIMITER_NR_RD_4)));
	cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_LIMITER_VP_WR_30_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_LIMITER_VP_WR_30_OFFSET)));
}

static int dump_cvdr_reg(hjpeg_hw_ctl_t *hw_ctl)
{
	void __iomem *viraddr = hw_ctl->cvdr_viraddr;
	if (!viraddr) {
		cam_err("%s: viraddr is NULL", __FUNCTION__);
		return -1;
	}

	if (hw_ctl->cvdr_prop.wr_port == WR_PORT_25) {
		if (get_hjpeg_wr_port_addr_update() != 0)
			wr_port_25_update_fail_logs_dbg(viraddr);
		else
			wr_port_25_update_suc_logs_dbg(viraddr);
	} else if (hw_ctl->cvdr_prop.wr_port == WR_PORT_0) {
		wr_port_id0_logs_dbg(viraddr);
	} else if (hw_ctl->cvdr_prop.wr_port == WR_PORT_30) {
		wr_port_id30_logs_dbg(viraddr);
	}
	return 0;
}
#endif

void wr_port_25_update_fail_logs_info(void __iomem *viraddr)
{
	cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_SRT_VP_WR_CFG_25_OFFSET_UPDATE,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_SRT_VP_WR_CFG_25_OFFSET_UPDATE)));
	cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_SRT_VP_WR_AXI_FS_25_OFFSET_UPDATE,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_SRT_VP_WR_AXI_FS_25_OFFSET_UPDATE)));
	cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET_UPDATE,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET_UPDATE)));
	cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_SRT_VP_WR_IF_CFG_25_OFFSET_UPDATE,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_SRT_VP_WR_IF_CFG_25_OFFSET_UPDATE)));
	cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_LIMITER_VP_WR_25_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_LIMITER_VP_WR_25_OFFSET)));
	cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_VP_WR_DEBUG_25_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_VP_WR_DEBUG_25_OFFSET)));
}

void wr_port_id30_logs_info(void __iomem *viraddr)
{
	cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_VP_WR_CFG_30_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_VP_WR_CFG_30_OFFSET)));
	cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_VP_WR_AXI_FS_30_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_VP_WR_AXI_FS_30_OFFSET)));
	cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_VP_WR_AXI_LINE_30_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_VP_WR_AXI_LINE_30_OFFSET)));
	cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_VP_WR_IF_CFG_30_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_VP_WR_IF_CFG_30_OFFSET)));
	cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_LIMITER_VP_WR_30_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_LIMITER_VP_WR_30_OFFSET)));
	cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_VP_WR_DEBUG_30_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_VP_WR_DEBUG_30_OFFSET)));
}

void wr_port_id4_logs_info(void __iomem *viraddr)
{
	cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_NR_RD_CFG_4,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_NR_RD_CFG_4)));
	cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_NR_RD_DEBUG_4_OFFSET,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_NR_RD_DEBUG_4_OFFSET)));
	cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_LIMITER_NR_RD_4,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_LIMITER_NR_RD_4)));
}

int dump_cvdr_debug_reg(hjpeg_hw_ctl_t *hw_ctl)
{
	unsigned int i;
	void __iomem *viraddr = NULL;
	if (!hw_ctl) {
		cam_err("%s:hw_ctl is NULL", __FUNCTION__);
		return -1;
	}

	viraddr = hw_ctl->cvdr_viraddr;
	if (!viraddr) {
		cam_err("%s:cvdr viraddr is NULL", __FUNCTION__);
		return -1;
	}
	cam_info("%s: CVDR reg:\n 0x%08x=0x%08x\n 0x%08x=0x%08x\n 0x%08x=0x%08x",
		__FUNCTION__,
		CVDR_AXI_JPEG_CVDR_CFG,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_CVDR_CFG)),
		CVDR_AXI_JPEG_CVDR_WR_QOS_CFG,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_CVDR_WR_QOS_CFG)),
		CVDR_AXI_JPEG_CVDR_RD_QOS_CFG,
		get_reg_val((void __iomem*)((char*)viraddr +
		CVDR_AXI_JPEG_CVDR_RD_QOS_CFG)));

	if (hw_ctl->cvdr_prop.wr_port == WR_PORT_25) {
		if (get_hjpeg_wr_port_addr_update() != 0)
			wr_port_25_update_fail_logs_info(viraddr);
	} else if (hw_ctl->cvdr_prop.wr_port == WR_PORT_30) {
		wr_port_id30_logs_info(viraddr);
	}

	if (hw_ctl->cvdr_prop.rd_port == RD_PORT_4)
		wr_port_id4_logs_info(viraddr);

	if (hw_ctl->smmu_type == ST_SMMUV3) {
		for (i = CVDR_AXI_JPEG_DEBUG_0; i <= CVDR_AXI_JPEG_DEBUG_15;
			i += CVDR_AXI_JPEG_DEBUG_INTERTVAL)
			cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__, i,
				get_reg_val((void __iomem*)(
				(char*)viraddr + i)));
	}
	return 0;
}

static int cvdr_fmt_desc_vp_wr(u32 wr_port, jpgenc_config_t *config,
	cvdr_wr_fmt_desc_t *desc)
{
	uint32_t width, height;
	uint32_t buf_addr, buf_size;

	width = config->buffer.width;
	height = config->buffer.height;

	buf_addr = config->buffer.output_buffer + JPGENC_HEAD_SIZE;
	buf_size = config->buffer.output_size - JPGENC_HEAD_SIZE;

	if (width == 0) {
		cam_err("width cannot be zero");
		return -1;
	}

	desc->pix_fmt = DF_D64;
	desc->pix_expan = EXP_PIX;
	desc->last_page = (buf_addr + buf_size) >> 15; /* 15: offset */

	desc->access_limiter = 16; /* 16: the value of access_limiter */

	switch (wr_port) {
	case WR_PORT_0: {
		desc->access_limiter = ACCESS_LIMITER_VP_WR_0;
		desc->line_stride = 0x3F;
		desc->line_wrap = 0x3FFF;
		break;
	}
	case WR_PORT_25: {
		if (get_hjpeg_wr_port_addr_update() != 0) {
			desc->access_limiter = ACCESS_LIMITER_VP_WR_0;
			desc->line_stride = 0x3F;
			desc->line_wrap = 0x3FFF;
		} else {
			/* 2, 16: ratio */
			desc->line_stride =
				(unsigned short)((width * 2) / 16 - 1);
			desc->line_wrap = (unsigned short)height;
		}
		break;
	}
	case WR_PORT_30: {
		desc->access_limiter = ACCESS_LIMITER_VP_WR_0;
		desc->line_stride = CVDR_AXI_JPEG_LINE_STRIDE_VAL;
		desc->line_wrap = 0x0;
		break;
	}
	default:
		cam_err("%s, invalid wr port[%d]", __FUNCTION__, wr_port);
		return -1;
	}

	cam_debug("%s acess_limiter = %d, last_page =0x%x, line_stride = %d, "
		"width = %d, height = %d",
		__FUNCTION__, desc->access_limiter, desc->last_page,
		desc->line_stride, width, height);

	return 0;
}

/* register addr changed,but no struct change */
void hjpeg_vp_wr_axi_line_config(void __iomem *cvdr_srt_base,
	cvdr_wr_fmt_desc_t *desc, u32 offset)
{
	u_vp_wr_axi_line tmp_line;

	if (!cvdr_srt_base || !desc) {
		cam_err("%s, desc or cvdr_srt_base is null", __FUNCTION__);
		return;
	}

	tmp_line.reg32 =
		get_reg_val((void __iomem*)((char*)cvdr_srt_base + offset));
	tmp_line.bits.vpwr_line_stride = desc->line_stride;
	tmp_line.bits.vpwr_line_wrap = desc->line_wrap;
	set_reg_val((void __iomem*)((char*)cvdr_srt_base + offset),
		tmp_line.reg32);
}

/* axi line register changed; */
void hjpeg_vp_wr_axi_line_update(void __iomem *cvdr_srt_base,
	cvdr_wr_fmt_desc_t *desc, u32 offset)
{
	u_vp_wr_axi_line_update_2 tmp_line;

	if (!cvdr_srt_base || !desc) {
		cam_err("%s, desc or cvdr_srt_base is null", __FUNCTION__);
		return;
	}

	tmp_line.reg32 =
		get_reg_val((void __iomem*)((char*)cvdr_srt_base + offset));
	tmp_line.bits.vpwr_line_stride = desc->line_stride;
	tmp_line.bits.vpwr_line_start_wstrb_4 = 0xF;
	tmp_line.bits.vpwr_line_wrap = desc->line_wrap;
	set_reg_val((void __iomem*)((char*)cvdr_srt_base + offset),
		tmp_line.reg32);
}

/* axi line register change :stride's bit add */
void hjpeg_vp_wr_axi_line_update3(void __iomem *cvdr_srt_base,
	cvdr_wr_fmt_desc_t *desc, u32 offset)
{
	u_vp_wr_axi_line_update_3 tmp_line;

	if (!cvdr_srt_base || !desc) {
		cam_err("%s, desc or cvdr_srt_base is null", __FUNCTION__);
		return;
	}

	tmp_line.reg32 =
		get_reg_val((void __iomem*)((char*)cvdr_srt_base + offset));
	tmp_line.bits.vpwr_line_stride = desc->line_stride;
	tmp_line.bits.vpwr_line_start_wstrb =
		CVDR_AXI_JPEG_LINE_START_WSTRB_VAL;
	tmp_line.bits.vpwr_line_wrap = desc->line_wrap;
	set_reg_val((void __iomem*)((char*)cvdr_srt_base + offset),
		tmp_line.reg32);
}

void hjpeg_config_axi_line(hjpeg_hw_ctl_t *hw_ctl, cvdr_wr_fmt_desc_t *desc)
{
	int wr_port_flag;
	if (!hw_ctl || !desc) {
		cam_err("%s, hw_ctl or desc is null", __FUNCTION__);
		return;
	}

	wr_port_flag = get_hjpeg_wr_port_addr_update();
	switch (hw_ctl->cvdr_prop.wr_port) {
	case WR_PORT_0: {
		hjpeg_vp_wr_axi_line_config(hw_ctl->cvdr_viraddr, desc,
			CVDR_AXI_JPEG_VP_WR_AXI_LINE_0_OFFSET);
		break;
	}
	case WR_PORT_25: {
		if (wr_port_flag == CVDR_WR_PORT_ADDR_AXILINE_UPDATE)
			hjpeg_vp_wr_axi_line_update(hw_ctl->cvdr_viraddr, desc,
				CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET_UPDATE);
		else if (wr_port_flag == CVDR_WR_PORT_ADDR_UPDATE)
			hjpeg_vp_wr_axi_line_config(hw_ctl->cvdr_viraddr, desc,
				CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET_UPDATE);
		else
			hjpeg_vp_wr_axi_line_config(hw_ctl->cvdr_viraddr, desc,
				CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET);
		break;
	}
	case WR_PORT_30: {
		hjpeg_vp_wr_axi_line_update3(hw_ctl->cvdr_viraddr, desc,
			CVDR_AXI_JPEG_VP_WR_AXI_LINE_30_OFFSET);
		break;
	}
	default: {
		cam_err("%s, invalid rd port[%d]", __FUNCTION__,
			hw_ctl->cvdr_prop.wr_port);
		return;
	}
	}
}

/* frame_start[31:4] for 32bit addr */
void hjpeg_config_vp_wr_axi_fs_for32(void __iomem *cvdr_srt_base,
	u32 offset, u32 addr)
{
	u_vp_wr_axi_fs tmp_fs;
	if (!cvdr_srt_base) {
		cam_err("%s, cvdr_srt_base is null", __FUNCTION__);
		return;
	}

	tmp_fs.reg32 =
		get_reg_val((void __iomem*)((char*)cvdr_srt_base + offset));
	tmp_fs.bits.vpwr_address_frame_start = addr >> 4; /* 4:offset */
	set_reg_val((void __iomem*)((char*)cvdr_srt_base + offset),
		tmp_fs.reg32);
}

/* frame_start[31:2] for 34bit addr */
void hjpeg_config_vp_wr_axi_fs_for34(void __iomem *cvdr_srt_base,
	u32 offset, u32 addr)
{
	u_vp_wr_axi_fs_update tmp_fs;
	if (!cvdr_srt_base) {
		cam_err("%s, cvdr_srt_base is null", __FUNCTION__);
		return;
	}

	tmp_fs.reg32 =
		get_reg_val((void __iomem*)((char*)cvdr_srt_base + offset));
	tmp_fs.bits.vpwr_address_frame_start = addr >> 4; /* 4:offset */
	set_reg_val((void __iomem*)((char*)cvdr_srt_base + offset),
		tmp_fs.reg32);
}

void hjpeg_config_axi_fs(hjpeg_hw_ctl_t *hw_ctl, jpgenc_config_t *config)
{
	u32 buf_addr;
	u32 wr_port;
	int iova_update_flag;
	if (!hw_ctl || !config) {
		cam_err("%s, hw_ctl or config is null", __FUNCTION__);
		return;
	}

	if (!hw_ctl->cvdr_viraddr) {
		cam_err("%s, hw_ctl cvdr_viraddr is null", __FUNCTION__);
		return;
	}

	wr_port = hw_ctl->cvdr_prop.wr_port;
	iova_update_flag = get_hjpeg_iova_update();
	buf_addr = config->buffer.output_buffer + JPGENC_HEAD_SIZE;

	switch (wr_port) {
	case WR_PORT_0: {
		hjpeg_config_vp_wr_axi_fs_for32(hw_ctl->cvdr_viraddr,
			CVDR_AXI_JPEG_VP_WR_AXI_FS_0_OFFSET, buf_addr);
		break;
	}
	case WR_PORT_25: {
		if (iova_update_flag == CVDR_IOVA_ADDR_34BITS)
			hjpeg_config_vp_wr_axi_fs_for34(hw_ctl->cvdr_viraddr,
				CVDR_SRT_VP_WR_AXI_FS_25_OFFSET_UPDATE,
				buf_addr);
		else
			hjpeg_config_vp_wr_axi_fs_for32(hw_ctl->cvdr_viraddr,
				CVDR_SRT_VP_WR_AXI_FS_25_OFFSET, buf_addr);
		break;
	}
	case WR_PORT_30: {
		hjpeg_config_vp_wr_axi_fs_for34(hw_ctl->cvdr_viraddr,
			CVDR_AXI_JPEG_VP_WR_AXI_FS_30_OFFSET, buf_addr);
		break;
	}
	default: {
		cam_err("%s, invalid rd port[%d]", __FUNCTION__, wr_port);
		return;
	}
	}
}

/* last_page[31:15]for 32bit addr pix_fmt[3:0] */
void hjpeg_vp_wr_cfg_for32(void __iomem *cvdr_srt_base,
	cvdr_wr_fmt_desc_t *desc, u32 offset)
{
	u_vp_wr_cfg tmp_cfg;
	if (!cvdr_srt_base || !desc) {
		cam_err("%s, desc or cvdr_srt_base is null", __FUNCTION__);
		return;
	}

	tmp_cfg.reg32 =
		get_reg_val((void __iomem*)((char*)cvdr_srt_base + offset));
	tmp_cfg.bits.vpwr_pixel_format = desc->pix_fmt;
	tmp_cfg.bits.vpwr_pixel_expansion = desc->pix_expan;
	tmp_cfg.bits.vpwr_last_page = desc->last_page;
	set_reg_val((void __iomem*)((char*)cvdr_srt_base + offset),
		tmp_cfg.reg32);
}

/* last_page[31:13] for 34bit addr and  pix_fmt[3:0] */
void hjpeg_vp_wr_cfg_for34(void __iomem *cvdr_srt_base,
	cvdr_wr_fmt_desc_t *desc, u32 offset)
{
	u_vp_wr_cfg_update tmp_cfg_update;
	if (!cvdr_srt_base || !desc) {
		cam_err("%s, desc or cvdr_srt_base is null", __FUNCTION__);
		return;
	}

	tmp_cfg_update.reg32 =
		get_reg_val((void __iomem*)((char*)cvdr_srt_base + offset));
	tmp_cfg_update.bits.vpwr_pixel_format = desc->pix_fmt;
	tmp_cfg_update.bits.vpwr_pixel_expansion = desc->pix_expan;
	tmp_cfg_update.bits.vpwr_last_page = desc->last_page;
	tmp_cfg_update.bits.reserved_0 = 0x0;
	set_reg_val((void __iomem*)((char*)cvdr_srt_base + offset),
		tmp_cfg_update.reg32);
}

/* lastpage[31:13] for 34bit  and pix_fmt[4:0] */
void hjpeg_vp_wr_cfg_for34_fmt_update(void __iomem *cvdr_srt_base,
	cvdr_wr_fmt_desc_t *desc, u32 offset)
{
	u_vp_wr_cfg_update_2 tmp_cfg_update;
	if (!cvdr_srt_base || !desc) {
		cam_err("%s, desc or cvdr_srt_base is null", __FUNCTION__);
		return;
	}

	tmp_cfg_update.reg32 =
		get_reg_val((void __iomem*)((char*)cvdr_srt_base + offset));
	tmp_cfg_update.bits.vpwr_pixel_format = PIX_FMT_D64;
	tmp_cfg_update.bits.vpwr_pixel_expansion = desc->pix_expan;
	tmp_cfg_update.bits.vpwr_last_page = desc->last_page;
	tmp_cfg_update.bits.reserved_0 = 0x0;
	set_reg_val((void __iomem*)((char*)cvdr_srt_base + offset),
		tmp_cfg_update.reg32);
}

void hjpeg_config_vp_wr(hjpeg_hw_ctl_t *hw_ctl, cvdr_wr_fmt_desc_t *desc)
{
	int iova_update_flag;
	u32 wr_port;
	u32 vp_wr_cfg_offset;
	if (!hw_ctl || !desc) {
		cam_err("%s, hw_ctl or desc is null", __FUNCTION__);
		return;
	}

	wr_port = hw_ctl->cvdr_prop.wr_port;
	iova_update_flag = get_hjpeg_iova_update();

	switch (wr_port) {
	case WR_PORT_0: {
		vp_wr_cfg_offset = CVDR_AXI_JPEG_VP_WR_CFG_0_OFFSET;
		break;
	}
	case WR_PORT_25: {
		if (get_hjpeg_wr_port_addr_update() != 0)
			vp_wr_cfg_offset = CVDR_SRT_VP_WR_CFG_25_OFFSET_UPDATE;
		else
			vp_wr_cfg_offset = CVDR_SRT_VP_WR_CFG_25_OFFSET;
		break;
	}
	case WR_PORT_30: {
		vp_wr_cfg_offset = CVDR_AXI_JPEG_VP_WR_CFG_30_OFFSET;
		break;
	}
	default:
		cam_err("%s, invalid wr port[%d]", __FUNCTION__, wr_port);
		return;
	}

	if (iova_update_flag == CVDR_IOVA_ADDR_32BITS) {
		cam_info("%s, for 32bits", __FUNCTION__);
		hjpeg_vp_wr_cfg_for32(hw_ctl->cvdr_viraddr, desc,
			vp_wr_cfg_offset);
	} else if (iova_update_flag == CVDR_IOVA_ADDR_34BITS) {
		if (is_pixel_fmt_update() != 0) {
			cam_info("%s, for 34bits and pixel format changed",
				__FUNCTION__);
			hjpeg_vp_wr_cfg_for34_fmt_update(hw_ctl->cvdr_viraddr,
				desc, vp_wr_cfg_offset);
		} else {
			cam_info("%s, for 34bits", __FUNCTION__);
			hjpeg_vp_wr_cfg_for34(hw_ctl->cvdr_viraddr, desc,
				vp_wr_cfg_offset);
		}
	} else {
		cam_err("%s, for error iova update value", __FUNCTION__);
	}
}

static void hjpeg_config_axi_if_cfg(hjpeg_hw_ctl_t *hw_ctl,
	u32 vp_wr_if_cfg_offset)
{
	void __iomem *cvdr_srt_base = hw_ctl->cvdr_viraddr;
	bool flag = false;

	u_vp_wr_axi_if_cfg axi_if_cfg;
	axi_if_cfg.reg32 =get_reg_val((void __iomem*)(
		(char*)cvdr_srt_base + vp_wr_if_cfg_offset));
	/* smmu bypass */
	if (hw_ctl->smmu_bypass == BYPASS_YES ||
		is_hjpeg_prefetch_bypass() == PREFETCH_BYPASS_YES) {
		axi_if_cfg.bits.prefetch_bypass = 1;
		flag = true;
	}

	if (hw_ctl->smmu_type == ST_SMMUV3) {
		axi_if_cfg.bits.stop_enable_flux_ctrl = 0;
		flag = true;
	}

	if (flag)
		set_reg_val((void __iomem*)(
			(char*)cvdr_srt_base + vp_wr_if_cfg_offset),
			axi_if_cfg.reg32);
}

#define PREFETCH_BY_PASS (1 << 31)
static int srt_limiter_offset_init(hjpeg_hw_ctl_t *hw_ctl,
	u32 *vp_wr_if_cfg_offset, u32 *cvdr_axi_limiter_vp_wr_offset)
{
	u32 wr_port = hw_ctl->cvdr_prop.wr_port;
	switch (wr_port) {
	case WR_PORT_0: {
		/* for smmu bypass */
		*vp_wr_if_cfg_offset = CVDR_AXI_JPEG_VP_WR_IF_CFG_0_OFFSET;
		*cvdr_axi_limiter_vp_wr_offset =
			CVDR_AXI_JPEG_LIMITER_VP_WR_0_OFFSET;
		break;
	}
	case WR_PORT_25: {
		if (get_hjpeg_wr_port_addr_update() != 0)
			/* for smmu bypass */
			*vp_wr_if_cfg_offset =
				CVDR_SRT_VP_WR_IF_CFG_25_OFFSET_UPDATE;
		else
			/* for smmu bypass */
			*vp_wr_if_cfg_offset = CVDR_SRT_VP_WR_IF_CFG_25_OFFSET;

		*cvdr_axi_limiter_vp_wr_offset =
			CVDR_AXI_JPEG_LIMITER_VP_WR_25_OFFSET;
		break;
	}
	case WR_PORT_30: {
		*vp_wr_if_cfg_offset = CVDR_AXI_JPEG_VP_WR_IF_CFG_30_OFFSET;
		*cvdr_axi_limiter_vp_wr_offset =
			CVDR_AXI_JPEG_LIMITER_VP_WR_30_OFFSET;
		break;
	}
	default:
		cam_err("%s, invalid wr port[%d]", __FUNCTION__, wr_port);
		return -1;
	}

	return 0;
}

static int set_vp_wr_ready(hjpeg_hw_ctl_t *hw_ctl, jpgenc_config_t *config)
{
	int ret;
	u_cvdr_srt_limiter_vp_wr_0 lmt;

	u32 cfg_offset;
	u32 axi_limiter_offset;

	cvdr_wr_fmt_desc_t desc;

	u32 wr_port = hw_ctl->cvdr_prop.wr_port;
	void __iomem *cvdr_srt_base = hw_ctl->cvdr_viraddr;

	ret = cvdr_fmt_desc_vp_wr(wr_port, config, &desc);
	if (ret != 0) {
		cam_err("%s() %d format for cvdr vpwr failed",
			__FUNCTION__, __LINE__);
		return -1;
	}

	ret = srt_limiter_offset_init(hw_ctl,
		&cfg_offset, &axi_limiter_offset);
	if (ret != 0)
		return ret;

	if (hw_ctl->cvdr_prop.flag) {
		desc.access_limiter = hw_ctl->cvdr_prop.wr_limiter;
		cam_debug("wr limiter:%d", desc.access_limiter);
	}

	/* set axi limiter */
	lmt.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base +
		axi_limiter_offset));
	lmt.bits.vpwr_access_limiter_0_0 = desc.access_limiter;
	lmt.bits.vpwr_access_limiter_1_0 = desc.access_limiter;
	lmt.bits.vpwr_access_limiter_2_0 = desc.access_limiter;
	lmt.bits.vpwr_access_limiter_3_0 = desc.access_limiter;
	set_reg_val((void __iomem*)((char*)cvdr_srt_base +
		axi_limiter_offset), lmt.reg32);

	/* config vp wr cfg */
	hjpeg_config_vp_wr(hw_ctl, &desc);

	/* config vp wr axi line */
	hjpeg_config_axi_line(hw_ctl, &desc);

	hjpeg_config_axi_if_cfg(hw_ctl, cfg_offset);

	/* config vp wr axi fs */
	hjpeg_config_axi_fs(hw_ctl, config);

	return 0;
}

static int get_nr_rd_pos(hjpeg_hw_ctl_t *hw_ctl,
	u32 *nr_rd_cfg_offset, u32 *limiter_nr_rd_offset,
	unsigned char *access_limiter, unsigned char *allocated_du)
{
	u32 rd_port = hw_ctl->cvdr_prop.rd_port;
	switch (rd_port) {
	case RD_PORT_0: {
		*access_limiter = ACCESS_LIMITER_NR_RD_0;

		if (is_hjpeg_qos_update() != 0)
			*allocated_du =
				(unsigned char)ALLOCATED_DU_NR_RD_0_UPDATE;
		else
			*allocated_du = (unsigned char)ALLOCATED_DU_NR_RD_0;

		*nr_rd_cfg_offset = CVDR_AXI_JPEG_NR_RD_CFG_0_OFFSET;
		*limiter_nr_rd_offset = CVDR_AXI_JPEG_LIMITER_NR_RD_0_OFFSET;
		break;
	}
	case RD_PORT_1: {
		*nr_rd_cfg_offset = CVDR_SRT_NR_RD_CFG_1_OFFSET;
		*limiter_nr_rd_offset = CVDR_SRT_LIMITER_NR_RD_1_OFFSET;
		break;
	}
	case RD_PORT_4: {
		*access_limiter = ACCESS_LIMITER_NR_RD_0;

		if (is_hjpeg_qos_update() != 0)
			*allocated_du =
				(unsigned char)ALLOCATED_DU_NR_RD_0_UPDATE;
		else
			*allocated_du = (unsigned char)ALLOCATED_DU_NR_RD_0;

		*nr_rd_cfg_offset = CVDR_AXI_JPEG_NR_RD_CFG_4;
		*limiter_nr_rd_offset = CVDR_AXI_JPEG_LIMITER_NR_RD_4;
		break;
	}
	default:
		cam_err("%s, invalid rd port[%d]", __FUNCTION__, rd_port);
		return -1;
	}

	if (hw_ctl->cvdr_prop.flag) {
		*access_limiter = hw_ctl->cvdr_prop.rd_limiter;
		*allocated_du = hw_ctl->cvdr_prop.allocated_du;
		cam_debug("rd limiter:%d du [%u]",
			*access_limiter, *allocated_du);
	}

	return 0;
}

static int set_nr_rd_config(hjpeg_hw_ctl_t *hw_ctl)
{
	int rc;
	u_cvdr_srt_nr_rd_cfg_1 tmp;
	u_cvdr_srt_limiter_nr_rd_1 lmt;
	u32 nr_rd_cfg_offset;
	u32 limiter_nr_rd_offset;

	u32 rd_port = hw_ctl->cvdr_prop.rd_port;
	void __iomem *cvdr_srt_base = hw_ctl->cvdr_viraddr;

	unsigned char access_limiter = ACCESS_LIMITER;
	unsigned char allocated_du = ALLOCATED_DU;

	rc = get_nr_rd_pos(hw_ctl, &nr_rd_cfg_offset, &limiter_nr_rd_offset,
		&access_limiter, &allocated_du);
	if (rc != 0)
		return rc;

	tmp.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base +
		nr_rd_cfg_offset));
	cam_info("%s: get CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		nr_rd_cfg_offset, tmp.reg32);
	lmt.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base +
		limiter_nr_rd_offset));
	cam_info("%s: get CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		limiter_nr_rd_offset, lmt.reg32);
	if (hw_ctl->cvdr_prop.flag && rd_port == RD_PORT_4) {
		/* check platform default value */
		tmp.reg32 = CVDR_AXI_JPEG_NR_RD_CFG_4_DEFAULT_VALUE;
		lmt.reg32 = CVDR_AXI_JPEG_LIMITER_NR_RD_4_DEFAULT_VALUE;
	}
	tmp.bits.nrrd_allocated_du_1 = allocated_du;
	tmp.bits.nrrd_enable_1 = 1;
	tmp.bits.nr_rd_stop_enable_pressure_1 = 1;
	tmp.bits.nr_rd_stop_enable_flux_ctrl_1 =
		(hw_ctl->smmu_type == ST_SMMUV3) ? 0 : 1;
	set_reg_val((void __iomem*)((char*)cvdr_srt_base + nr_rd_cfg_offset),
		tmp.reg32);
	cam_info("%s: set CVDR reg: 0x%08x=0x%08x",
		__FUNCTION__, nr_rd_cfg_offset, tmp.reg32);

	lmt.bits.nrrd_access_limiter_0_1 = access_limiter;
	lmt.bits.nrrd_access_limiter_1_1 = access_limiter;
	lmt.bits.nrrd_access_limiter_2_1 = access_limiter;
	lmt.bits.nrrd_access_limiter_3_1 = access_limiter;
	lmt.bits.nrrd_access_limiter_reload_1 = 0xF;
	set_reg_val((void __iomem*)(
		(char*)cvdr_srt_base + limiter_nr_rd_offset), lmt.reg32);
	cam_info("%s: set CVDR reg: 0x%08x=0x%08x",
		__FUNCTION__, limiter_nr_rd_offset, lmt.reg32);

	return 0;
}

void config_cvdr_max_wr_id(void __iomem *cvdr_base_addr)
{
	u32 tmpVal;

	tmpVal = get_reg_val((void __iomem*)(
		(char*)cvdr_base_addr + CVDR_AXI_JPEG_CVDR_CFG));
	if (is_cvdr_cfg_update() == 0) {
		/*
		 * max number of AXI write transactionsReturnToLine
		 * the maximum expected number is 0x8
		 */
		REG_SET_FIELD(tmpVal, AXI_JPEG_CVDR_CFG_max_axiwrite_id, 0x7);
		/*
		 * Maximum value allowed on the ARID to access the
		 * memory maximum expected number is 0x10
		 */
		REG_SET_FIELD(tmpVal, AXI_JPEG_CVDR_CFG_max_axiread_id, 0xF);
		set_reg_val((void __iomem*)(
			(char*)cvdr_base_addr + CVDR_AXI_JPEG_CVDR_CFG),
			tmpVal);
	}

	cam_info("%s: CVDR reg: 0x%08x=0x%08x",
		__FUNCTION__, CVDR_AXI_JPEG_CVDR_CFG,
		get_reg_val((void __iomem*)(
		(char*)cvdr_base_addr + CVDR_AXI_JPEG_CVDR_CFG)));
}

void config_cvdr_wr_qos(void __iomem *cvdr_base_addr)
{
	u_cvdr_wr_qos_cfg wr_qos;

	wr_qos.reg32 = get_reg_val((void __iomem*)(
		(char*)cvdr_base_addr + CVDR_AXI_JPEG_CVDR_WR_QOS_CFG));
	wr_qos.bits.wr_qos_max = 0x1;
	wr_qos.bits.wr_qos_threshold_01_start = 0x1;
	wr_qos.bits.wr_qos_threshold_01_stop = 0x1;
	wr_qos.bits.wr_qos_threshold_11_start = 0x3;
	wr_qos.bits.wr_qos_threshold_11_stop = 0x3;
	wr_qos.bits.wr_qos_threshold_10_start = 0x3;
	wr_qos.bits.wr_qos_threshold_10_stop = 0x3;
	if (is_hjpeg_qos_update() != 0)
		wr_qos.bits.wr_qos_sr = 0x0;

	set_reg_val((void __iomem*)(
		(char*)cvdr_base_addr + CVDR_AXI_JPEG_CVDR_WR_QOS_CFG),
		wr_qos.reg32);
	cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_CVDR_WR_QOS_CFG,
		get_reg_val((void __iomem*)((char*)cvdr_base_addr +
		CVDR_AXI_JPEG_CVDR_WR_QOS_CFG)));
}

void config_cvdr_rd_qos(void __iomem *cvdr_base_addr)
{
	U_CVDR_RD_QOS_CFG rd_qos;
	rd_qos.reg32 = get_reg_val((void __iomem*)(
		(char*)cvdr_base_addr + CVDR_AXI_JPEG_CVDR_RD_QOS_CFG));
	rd_qos.bits.rd_qos_max = 0x1;
	rd_qos.bits.rd_qos_threshold_11_start = 0x3;
	rd_qos.bits.rd_qos_threshold_11_stop = 0x3;
	rd_qos.bits.rd_qos_threshold_10_start = 0x3;
	rd_qos.bits.rd_qos_threshold_10_stop = 0x3;
	rd_qos.bits.rd_qos_threshold_01_start = 0x1;
	rd_qos.bits.rd_qos_threshold_01_stop = 0x1;
	if (is_hjpeg_qos_update() != 0)
		rd_qos.bits.rd_qos_sr = 0x0;

	set_reg_val((void __iomem*)(
		(char*)cvdr_base_addr + CVDR_AXI_JPEG_CVDR_RD_QOS_CFG),
		rd_qos.reg32);
	cam_info("%s: CVDR reg: 0x%08x=0x%08x", __FUNCTION__,
		CVDR_AXI_JPEG_CVDR_RD_QOS_CFG,
		get_reg_val((void __iomem*)((char*)cvdr_base_addr +
		CVDR_AXI_JPEG_CVDR_RD_QOS_CFG)));
}

void hjpeg_config_cvdr(hjpeg_hw_ctl_t *hw_ctl, jpgenc_config_t *config)
{
	int ret;
	void __iomem *cvdr_base_addr = NULL;

	if (!hw_ctl) {
		cam_err("%s() %d: invalid parameter", __FUNCTION__, __LINE__);
		return;
	}

	if (!config) {
		cam_err("%s() %d: config is null", __FUNCTION__, __LINE__);
		return;
	}

	cam_info("%s enter", __FUNCTION__);

	if (hw_ctl->chip_type == CT_CS) {
		cvdr_base_addr = hw_ctl->cvdr_viraddr;
		if (cvdr_base_addr) {
			config_cvdr_max_wr_id(cvdr_base_addr);
			config_cvdr_wr_qos(cvdr_base_addr);
			config_cvdr_rd_qos(cvdr_base_addr);
		}
	}

	ret = set_vp_wr_ready(hw_ctl, config);
	if (ret != 0) {
		cam_err("%s() %d set vp wr ready fail", __FUNCTION__, __LINE__);
		return;
	}

	ret = set_nr_rd_config(hw_ctl);
	if (ret != 0) {
		cam_err("%s() %d set nr rd config fail",
			__FUNCTION__, __LINE__);
		return;
	}

	dump_cvdr_reg(hw_ctl);
}