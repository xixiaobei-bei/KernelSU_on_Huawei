/*
 * cvdr_cfg.c
 *
 * implement for config cvdr. different implement by different platform.
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * lixiuhua <aimee.li@hisilicon.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/iommu.h>
#include <asm/io.h>

#include "cvdr_cfg.h"
#include "cam_log.h"

#if 1
static int dump_cvdr_reg(hjpeg_hw_ctl_t *hw_ctl)
{
    unsigned int i;
    void __iomem * viraddr = hw_ctl->cvdr_viraddr;
    if (NULL == viraddr) {
        cam_err("%s: viraddr is NULL", __func__);
        return -1;
    }

    if (hw_ctl->cvdr_prop.wr_port == WR_PORT_25) {
        if (get_hjpeg_wr_port_addr_update()) {
            cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
                CVDR_SRT_VP_WR_CFG_25_OFFSET_UPDATE,
                get_reg_val((void __iomem*)((char*)viraddr+CVDR_SRT_VP_WR_CFG_25_OFFSET_UPDATE)));
            cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
                CVDR_SRT_VP_WR_AXI_FS_25_OFFSET_UPDATE,
                get_reg_val((void __iomem*)((char*)viraddr+CVDR_SRT_VP_WR_AXI_FS_25_OFFSET_UPDATE)));
            cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
                CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET_UPDATE,
                get_reg_val((void __iomem*)((char*)viraddr+CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET_UPDATE)));
            cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
                CVDR_SRT_VP_WR_IF_CFG_25_OFFSET_UPDATE,
                get_reg_val((void __iomem*)((char*)viraddr+CVDR_SRT_VP_WR_IF_CFG_25_OFFSET_UPDATE)));
            cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
                CVDR_AXI_JPEG_NR_RD_CFG_4,
                get_reg_val((void __iomem*)((char*)viraddr+CVDR_AXI_JPEG_NR_RD_CFG_4)));
            cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
                CVDR_AXI_JPEG_LIMITER_NR_RD_4,
                get_reg_val((void __iomem*)((char*)viraddr+CVDR_AXI_JPEG_LIMITER_NR_RD_4)));
            cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
                CVDR_AXI_JPEG_LIMITER_VP_WR_25_OFFSET,
                get_reg_val((void __iomem*)((char*)viraddr+CVDR_AXI_JPEG_LIMITER_VP_WR_25_OFFSET)));
        } else {
            cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
                CVDR_SRT_VP_WR_CFG_25_OFFSET,
                get_reg_val((void __iomem*)((char*)viraddr+CVDR_SRT_VP_WR_CFG_25_OFFSET)));
            cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
                CVDR_SRT_VP_WR_AXI_FS_25_OFFSET,
                get_reg_val((void __iomem*)((char*)viraddr+CVDR_SRT_VP_WR_AXI_FS_25_OFFSET)));
            cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
                CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET,
                get_reg_val((void __iomem*)((char*)viraddr+CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET)));
            cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
                CVDR_SRT_VP_WR_IF_CFG_25_OFFSET,
                get_reg_val((void __iomem*)((char*)viraddr+CVDR_SRT_VP_WR_IF_CFG_25_OFFSET)));
            cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
                CVDR_SRT_NR_RD_CFG_1_OFFSET,
                get_reg_val((void __iomem*)((char*)viraddr+CVDR_SRT_NR_RD_CFG_1_OFFSET)));
            cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
                CVDR_SRT_LIMITER_NR_RD_1_OFFSET,
                get_reg_val((void __iomem*)((char*)viraddr+CVDR_SRT_LIMITER_NR_RD_1_OFFSET)));
            cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
                CVDR_SRT_LIMITER_VP_WR_25_OFFSET,
                get_reg_val((void __iomem*)((char*)viraddr+CVDR_SRT_LIMITER_VP_WR_25_OFFSET)));
        }
    } else if (hw_ctl->cvdr_prop.wr_port == WR_PORT_0) {
        cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
            CVDR_AXI_JPEG_VP_WR_CFG_0_OFFSET,
            get_reg_val((void __iomem*)((char*)viraddr+CVDR_AXI_JPEG_VP_WR_CFG_0_OFFSET)));
        cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
            CVDR_AXI_JPEG_VP_WR_AXI_FS_0_OFFSET,
            get_reg_val((void __iomem*)((char*)viraddr+CVDR_AXI_JPEG_VP_WR_AXI_FS_0_OFFSET)));
        cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
            CVDR_AXI_JPEG_VP_WR_AXI_LINE_0_OFFSET,
            get_reg_val((void __iomem*)((char*)viraddr+CVDR_AXI_JPEG_VP_WR_AXI_LINE_0_OFFSET)));
        cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
            CVDR_AXI_JPEG_VP_WR_IF_CFG_0_OFFSET,
            get_reg_val((void __iomem*)((char*)viraddr+CVDR_AXI_JPEG_VP_WR_IF_CFG_0_OFFSET)));
        cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
            CVDR_AXI_JPEG_LIMITER_VP_WR_0_OFFSET,
            get_reg_val((void __iomem*)((char*)viraddr+CVDR_AXI_JPEG_LIMITER_VP_WR_0_OFFSET)));
        cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
            CVDR_AXI_JPEG_NR_RD_CFG_0_OFFSET,
            get_reg_val((void __iomem*)((char*)viraddr+CVDR_AXI_JPEG_NR_RD_CFG_0_OFFSET)));
        cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
            CVDR_AXI_JPEG_NR_RD_DEBUG_0_OFFSET,
            get_reg_val((void __iomem*)((char*)viraddr+CVDR_AXI_JPEG_NR_RD_DEBUG_0_OFFSET)));
        cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
            CVDR_AXI_JPEG_LIMITER_NR_RD_0_OFFSET,
            get_reg_val((void __iomem*)((char*)viraddr+CVDR_AXI_JPEG_LIMITER_NR_RD_0_OFFSET)));

        for (i = CVDR_AXI_JPEG_DEBUG_0_OFFSET; i <= CVDR_AXI_JPEG_DEBUG_3_OFFSET; i+=4)
        {
            cam_debug("%s: CVDR debug_x_offset reg: 0x%08x=0x%08x", __func__,
            i, get_reg_val((void __iomem*)((char*)viraddr+i)));
        }

        cam_debug("%s: CVDR reg: 0x%08x=0x%08x", __func__,
            CVDR_AXI_JPEG_VP_WR_DEBUG_0_OFFSET,
            get_reg_val((void __iomem*)((char*)viraddr+CVDR_AXI_JPEG_VP_WR_DEBUG_0_OFFSET)));
    }
    return 0;
}
#endif

static int cvdr_fmt_desc_vp_wr(u32 wr_port, jpgenc_config_t* config, cvdr_wr_fmt_desc_t *desc)
{
    uint32_t width;
    uint32_t height;
    uint32_t buf_addr;
    uint32_t buf_size;

    width    = config->buffer.width;
    height   = config->buffer.height;

    buf_addr = config->buffer.output_buffer + JPGENC_HEAD_SIZE;
    buf_size = config->buffer.output_size - JPGENC_HEAD_SIZE;

    if(0 == width){
       cam_err("width cannot be zero");
       return -1;
    }

    desc->pix_fmt        = DF_D64;
    desc->pix_expan      = EXP_PIX;
    desc->last_page      = (buf_addr + buf_size) >> 15;

    desc->access_limiter = 16;

    switch(wr_port) {
        case WR_PORT_0:
        {
            desc->access_limiter = ACCESS_LIMITER_VP_WR_0;
            desc->line_stride = 0x3F;
            desc->line_wrap   = 0x3FFF;
            break;
        }
        case WR_PORT_25:
        {
            if (get_hjpeg_wr_port_addr_update()) {
                desc->access_limiter = ACCESS_LIMITER_VP_WR_0;
                desc->line_stride = 0x3F;
                desc->line_wrap   = 0x3FFF;
            } else {
                desc->line_stride = (unsigned short)((width * 2) / 16 - 1);
                desc->line_wrap   = (unsigned short)height;
            }
            break;
        }
        default:
            cam_err("%s, invalid wr port[%d]",__func__, wr_port);
            return -1;
    }

    cam_debug("%s acess_limiter = %d, last_page =0x%x, line_stride = %d, width = %d,  height = %d",
        __func__, desc->access_limiter, desc->last_page, desc->line_stride, width, height);

    return 0;
}

// register addr changed,but no struct change
void hjpeg_vp_wr_axi_line_config(void __iomem *cvdr_srt_base, cvdr_wr_fmt_desc_t *desc, u32 offset)
{
    U_VP_WR_AXI_LINE tmp_line;

    if (NULL == cvdr_srt_base || NULL == desc) {
        cam_err("%s, desc or cvdr_srt_base is null", __func__);
        return;
    }

    tmp_line.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base + offset));
    tmp_line.bits.vpwr_line_stride = desc->line_stride;
    tmp_line.bits.vpwr_line_wrap   = desc->line_wrap;
    set_reg_val((void __iomem*)((char*)cvdr_srt_base + offset), tmp_line.reg32);
}

// axi line register changed;
void hjpeg_vp_wr_axi_line_update(void __iomem *cvdr_srt_base, cvdr_wr_fmt_desc_t *desc, u32 offset)
{
    U_VP_WR_AXI_LINE_UPDATE_2 tmp_line;

    if (NULL == cvdr_srt_base || NULL == desc) {
        cam_err("%s, desc or cvdr_srt_base is null", __func__);
        return;
    }

    tmp_line.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base + offset));
    tmp_line.bits.vpwr_line_stride = desc->line_stride;
    tmp_line.bits.vpwr_line_start_wstrb_4 = 0xF;
    tmp_line.bits.vpwr_line_wrap   = desc->line_wrap;
    set_reg_val((void __iomem*)((char*)cvdr_srt_base + offset), tmp_line.reg32);
}

void hjpeg_config_axi_line(hjpeg_hw_ctl_t *hw_ctl, cvdr_wr_fmt_desc_t *desc)
{
    int wr_port_flag;

    if (NULL == hw_ctl || NULL == desc) {
        cam_err("%s, hw_ctl or desc is null", __func__);
        return;
    }

    wr_port_flag  = get_hjpeg_wr_port_addr_update();

    switch(hw_ctl->cvdr_prop.wr_port) {
        case WR_PORT_0:
        {
            hjpeg_vp_wr_axi_line_config(hw_ctl->cvdr_viraddr, desc, CVDR_AXI_JPEG_VP_WR_AXI_LINE_0_OFFSET);
            break;
        }
        case WR_PORT_25:
        {
            if (CVDR_WR_PORT_ADDR_AXILINE_UPDATE == wr_port_flag) {
                hjpeg_vp_wr_axi_line_update(hw_ctl->cvdr_viraddr, desc, CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET_UPDATE);
            } else if (CVDR_WR_PORT_ADDR_UPDATE == wr_port_flag) {
                hjpeg_vp_wr_axi_line_config(hw_ctl->cvdr_viraddr, desc, CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET_UPDATE);
            } else {
                hjpeg_vp_wr_axi_line_config(hw_ctl->cvdr_viraddr, desc, CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET);
            }
            break;
        }
        default:
        {
            cam_err("%s, invalid rd port[%d]", __func__, hw_ctl->cvdr_prop.wr_port);
            return;
        }
    }
}
// frame_start[31:4] for 32bit addr
void hjpeg_config_vp_wr_axi_fs_for32(void __iomem *cvdr_srt_base, u32 offset, u32 addr)
{
    U_VP_WR_AXI_FS tmp_fs;
    if (NULL == cvdr_srt_base) {
        cam_err("%s, cvdr_srt_base is null", __func__);
        return;
    }

    tmp_fs.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base + offset));
    tmp_fs.bits.vpwr_address_frame_start = addr >> 4;
    set_reg_val((void __iomem*)((char*)cvdr_srt_base + offset), tmp_fs.reg32);
}

// frame_start[31:2] for 34bit addr
void hjpeg_config_vp_wr_axi_fs_for34(void __iomem *cvdr_srt_base, u32 offset, u32 addr)
{
    U_VP_WR_AXI_FS_UPDATE tmp_fs;
    if (NULL == cvdr_srt_base) {
        cam_err("%s, cvdr_srt_base is null", __func__);
        return;
    }

    tmp_fs.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base + offset));
    tmp_fs.bits.vpwr_address_frame_start = addr >> 4;
    set_reg_val((void __iomem*)((char*)cvdr_srt_base + offset), tmp_fs.reg32);
}

void hjpeg_config_axi_fs(hjpeg_hw_ctl_t *hw_ctl, jpgenc_config_t* config)
{
    u32 buf_addr;
    u32 wr_port;
    int iova_update_flag;
    if (NULL == hw_ctl || NULL == config) {
        cam_err("%s, hw_ctl or config is null", __func__);
        return;
    }

    if (NULL == hw_ctl->cvdr_viraddr) {
        cam_err("%s, hw_ctl cvdr_viraddr is null", __func__);
        return;
    }

    wr_port          = hw_ctl->cvdr_prop.wr_port;
    iova_update_flag = get_hjpeg_iova_update();
    buf_addr = config->buffer.output_buffer + JPGENC_HEAD_SIZE;

    switch(wr_port) {
        case WR_PORT_0:
        {
            hjpeg_config_vp_wr_axi_fs_for32(hw_ctl->cvdr_viraddr, CVDR_AXI_JPEG_VP_WR_AXI_FS_0_OFFSET, buf_addr);
            break;
        }
        case WR_PORT_25:
        {
            if (CVDR_IOVA_ADDR_34BITS == iova_update_flag) {
                hjpeg_config_vp_wr_axi_fs_for34(hw_ctl->cvdr_viraddr, CVDR_SRT_VP_WR_AXI_FS_25_OFFSET_UPDATE, buf_addr);
            } else {
                hjpeg_config_vp_wr_axi_fs_for32(hw_ctl->cvdr_viraddr, CVDR_SRT_VP_WR_AXI_FS_25_OFFSET, buf_addr);
            }
            break;
        }
        default:
        {
            cam_err("%s, invalid rd port[%d]", __func__, wr_port);
            return;
        }
    }
}

// last_page[31:15]for 32bit addr pix_fmt[3:0]
void hjpeg_vp_wr_cfg_for32(void __iomem *cvdr_srt_base, cvdr_wr_fmt_desc_t *desc, u32 offset)
{
    U_VP_WR_CFG tmp_cfg;
    if (NULL == cvdr_srt_base || NULL == desc) {
        cam_err("%s, desc or cvdr_srt_base is null", __func__);
        return;
    }

    tmp_cfg.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base + offset));
    tmp_cfg.bits.vpwr_pixel_format    = desc->pix_fmt;
    tmp_cfg.bits.vpwr_pixel_expansion = desc->pix_expan;
    tmp_cfg.bits.vpwr_last_page       = desc->last_page;
    set_reg_val((void __iomem*)((char*)cvdr_srt_base + offset), tmp_cfg.reg32);
}
// last_page[31:13] for 34bit addr and  pix_fmt[3:0]
void hjpeg_vp_wr_cfg_for34(void __iomem *cvdr_srt_base, cvdr_wr_fmt_desc_t *desc, u32 offset)
{
    U_VP_WR_CFG_UPDATE tmp_cfg_update;
    if (NULL == cvdr_srt_base || NULL == desc) {
        cam_err("%s, desc or cvdr_srt_base is null", __func__);
        return;
    }

    tmp_cfg_update.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base + offset));
    tmp_cfg_update.bits.vpwr_pixel_format    = desc->pix_fmt;
    tmp_cfg_update.bits.vpwr_pixel_expansion = desc->pix_expan;
    tmp_cfg_update.bits.vpwr_last_page       = desc->last_page;
    tmp_cfg_update.bits.reserved_0           = 0x0;
    set_reg_val((void __iomem*)((char*)cvdr_srt_base + offset), tmp_cfg_update.reg32);
}
// lastpage[31:13] for 34bit  and pix_fmt[4:0]
void hjpeg_vp_wr_cfg_for34_fmt_update(void __iomem *cvdr_srt_base, cvdr_wr_fmt_desc_t *desc, u32 offset)
{
    U_VP_WR_CFG_UPDATE_2 tmp_cfg_update;
    if (NULL == cvdr_srt_base || NULL == desc) {
        cam_err("%s, desc or cvdr_srt_base is null", __func__);
        return;
    }

    tmp_cfg_update.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base + offset));
    tmp_cfg_update.bits.vpwr_pixel_format    = PIX_FMT_D64;
    tmp_cfg_update.bits.vpwr_pixel_expansion = desc->pix_expan;
    tmp_cfg_update.bits.vpwr_last_page       = desc->last_page;
    tmp_cfg_update.bits.reserved_0           = 0x0;
    set_reg_val((void __iomem*)((char*)cvdr_srt_base + offset), tmp_cfg_update.reg32);
}

void hjpeg_config_vp_wr(hjpeg_hw_ctl_t *hw_ctl, cvdr_wr_fmt_desc_t *desc)
{
    int iova_update_flag;
    u32 wr_port;
    u32 vp_wr_cfg_offset;
    if (NULL == hw_ctl || NULL == desc) {
        cam_err("%s, hw_ctl or desc is null", __func__);
        return;
    }

    wr_port          = hw_ctl->cvdr_prop.wr_port;
    iova_update_flag = get_hjpeg_iova_update();

    switch(wr_port) {
        case WR_PORT_0:
        {
            vp_wr_cfg_offset = CVDR_AXI_JPEG_VP_WR_CFG_0_OFFSET;
            break;
        }
        case WR_PORT_25:
        {
            if (get_hjpeg_wr_port_addr_update()) {
                vp_wr_cfg_offset      = CVDR_SRT_VP_WR_CFG_25_OFFSET_UPDATE;
            } else {
                vp_wr_cfg_offset      = CVDR_SRT_VP_WR_CFG_25_OFFSET;
            }
            break;
        }
        default:
            cam_err("%s, invalid wr port[%d]", __func__, wr_port);
            return;
    }

    if (CVDR_IOVA_ADDR_32BITS == iova_update_flag) {
        cam_info("%s, for 32bits", __func__);
        hjpeg_vp_wr_cfg_for32(hw_ctl->cvdr_viraddr, desc, vp_wr_cfg_offset);
    } else if (CVDR_IOVA_ADDR_34BITS == iova_update_flag) {
        if (is_pixel_fmt_update())    {
            cam_info("%s, for 34bits and pixel format changed", __func__);
            hjpeg_vp_wr_cfg_for34_fmt_update(hw_ctl->cvdr_viraddr, desc, vp_wr_cfg_offset);
        } else {
            cam_info("%s, for 34bits", __func__);
            hjpeg_vp_wr_cfg_for34(hw_ctl->cvdr_viraddr, desc, vp_wr_cfg_offset);
        }
    } else {
        cam_err("%s, for error iova update value", __func__);
    }
}

#define PREFETCH_BY_PASS (1 << 31)
static int set_vp_wr_ready(hjpeg_hw_ctl_t *hw_ctl, jpgenc_config_t* config)
{
    int ret;
    U_CVDR_SRT_LIMITER_VP_WR_0 lmt;

    u32 vp_wr_if_cfg_offset;
    u32 cvdr_axi_limiter_vp_wr_offset;

    cvdr_wr_fmt_desc_t desc;

    u32 wr_port                 = hw_ctl->cvdr_prop.wr_port;
    void __iomem *cvdr_srt_base = hw_ctl->cvdr_viraddr;


    ret = cvdr_fmt_desc_vp_wr(wr_port, config, &desc);
    if(0 != ret){
        cam_err("%s (%d) format for cvdr vpwr failed", __func__, __LINE__);
        return -1;
    }

    switch(wr_port) {
        case WR_PORT_0:
        {
            // for smmu bypass
            vp_wr_if_cfg_offset   = CVDR_AXI_JPEG_VP_WR_IF_CFG_0_OFFSET;
            cvdr_axi_limiter_vp_wr_offset = CVDR_AXI_JPEG_LIMITER_VP_WR_0_OFFSET;
            break;
        }
        case WR_PORT_25:
        {
            if (get_hjpeg_wr_port_addr_update()) {
                // for smmu bypass
                vp_wr_if_cfg_offset   = CVDR_SRT_VP_WR_IF_CFG_25_OFFSET_UPDATE;
            } else {
                // for smmu bypass
                vp_wr_if_cfg_offset   = CVDR_SRT_VP_WR_IF_CFG_25_OFFSET;
            }

            cvdr_axi_limiter_vp_wr_offset = CVDR_AXI_JPEG_LIMITER_VP_WR_25_OFFSET;
            break;
        }
        default:
            cam_err("%s, invalid wr port[%d]", __func__, wr_port);
            return -1;
    }

    if (hw_ctl->cvdr_prop.flag) {
        desc.access_limiter = hw_ctl->cvdr_prop.wr_limiter;
        cam_debug("wr limiter:%d",desc.access_limiter);
    }

    // set axi limiter
    lmt.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base + cvdr_axi_limiter_vp_wr_offset));
    lmt.bits.vpwr_access_limiter_0_0 = desc.access_limiter;
    lmt.bits.vpwr_access_limiter_1_0 = desc.access_limiter;
    lmt.bits.vpwr_access_limiter_2_0 = desc.access_limiter;
    lmt.bits.vpwr_access_limiter_3_0 = desc.access_limiter;
    set_reg_val((void __iomem*)((char*)cvdr_srt_base + cvdr_axi_limiter_vp_wr_offset), lmt.reg32);

    // config vp wr cfg
    hjpeg_config_vp_wr(hw_ctl, &desc);

    // config vp wr axi line
    hjpeg_config_axi_line(hw_ctl, &desc);

    // smmu bypass
    if (hw_ctl->smmu_bypass == BYPASS_YES)
    {
        set_reg_val((void __iomem*)((char*)cvdr_srt_base + vp_wr_if_cfg_offset),
            (get_reg_val((void __iomem*)((char*)cvdr_srt_base + vp_wr_if_cfg_offset))|(PREFETCH_BY_PASS)));  /*lint !e648*/
    }

    // config vp wr axi fs
    hjpeg_config_axi_fs(hw_ctl, config);

    return 0;
}

static int set_nr_rd_config(hjpeg_hw_ctl_t *hw_ctl)
{
    U_CVDR_SRT_NR_RD_CFG_1 tmp;
    U_CVDR_SRT_LIMITER_NR_RD_1 lmt;
    u32 nr_rd_cfg_offset;
    u32 limiter_nr_rd_offset;

    u32 rd_port                 = hw_ctl->cvdr_prop.rd_port;
    void __iomem* cvdr_srt_base = hw_ctl->cvdr_viraddr;

    //FIXME:use ceil align up float to int
    unsigned char access_limiter = ACCESS_LIMITER;
    unsigned char allocated_du   = ALLOCATED_DU;

    switch(rd_port) {
        case RD_PORT_0:
        {
            access_limiter = ACCESS_LIMITER_NR_RD_0;

            if (is_hjpeg_qos_update()) {
                allocated_du   = ALLOCATED_DU_NR_RD_0_UPDATE;
            } else {
                allocated_du   = ALLOCATED_DU_NR_RD_0;
            }

            nr_rd_cfg_offset     = CVDR_AXI_JPEG_NR_RD_CFG_0_OFFSET;
            limiter_nr_rd_offset = CVDR_AXI_JPEG_LIMITER_NR_RD_0_OFFSET;
            break;
        }
        case RD_PORT_1:
        {
            nr_rd_cfg_offset     = CVDR_SRT_NR_RD_CFG_1_OFFSET;
            limiter_nr_rd_offset = CVDR_SRT_LIMITER_NR_RD_1_OFFSET;
            break;
        }
        case RD_PORT_4:
        {
            access_limiter = ACCESS_LIMITER_NR_RD_0;

            if (is_hjpeg_qos_update()) {
                allocated_du   = ALLOCATED_DU_NR_RD_0_UPDATE;
            } else {
                allocated_du   = ALLOCATED_DU_NR_RD_0;
            }

            nr_rd_cfg_offset     = CVDR_AXI_JPEG_NR_RD_CFG_4;
            limiter_nr_rd_offset = CVDR_AXI_JPEG_LIMITER_NR_RD_4;
            break;
        }
        default:
            cam_err("%s, invalid rd port[%d]", __func__, rd_port);
            return -1;
    }

    if (hw_ctl->cvdr_prop.flag) {
        access_limiter = hw_ctl->cvdr_prop.rd_limiter;
        allocated_du   = hw_ctl->cvdr_prop.allocated_du;
        cam_debug("rd limiter:%d du [%u].", access_limiter, allocated_du);
    }

    tmp.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base + nr_rd_cfg_offset));
    tmp.bits.nrrd_allocated_du_1 = allocated_du;
    tmp.bits.nrrd_enable_1 = 1;
    tmp.bits.nr_rd_stop_enable_pressure_1 = 1;
    tmp.bits.nr_rd_stop_enable_flux_ctrl_1 = 1;
    set_reg_val((void __iomem*)((char*)cvdr_srt_base + nr_rd_cfg_offset) , tmp.reg32);
    cam_info("%s: set CVDR reg: 0x%08x=0x%08x", __func__, nr_rd_cfg_offset, tmp.reg32);

    lmt.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base + limiter_nr_rd_offset));
    lmt.bits.nrrd_access_limiter_0_1 = access_limiter;
    lmt.bits.nrrd_access_limiter_1_1 = access_limiter;
    lmt.bits.nrrd_access_limiter_2_1 = access_limiter;
    lmt.bits.nrrd_access_limiter_3_1 = access_limiter;
    lmt.bits.nrrd_access_limiter_reload_1 = 0xF;
    set_reg_val((void __iomem*)((char*)cvdr_srt_base + limiter_nr_rd_offset) , lmt.reg32);
    cam_info("%s: set CVDR reg: 0x%08x=0x%08x", __func__, limiter_nr_rd_offset, lmt.reg32);

    return 0;
}

void hjpeg_config_cvdr(hjpeg_hw_ctl_t *hw_ctl, jpgenc_config_t* config)
{
    int ret;
    u32 tmpVal;

    void __iomem *  cvdr_base_addr;

    U_CVDR_WR_QOS_CFG wr_qos;
    U_CVDR_RD_QOS_CFG rd_qos;

    if (NULL == hw_ctl)
    {
        cam_err("%s(%d): invalid parameter",__func__, __LINE__);
        return;
    }

    if (NULL == config) {
        cam_err("%s(%d): config is null!",__func__, __LINE__);
        return;
    }

    cam_info("%s enter ",__func__);

    if (CT_CS==hw_ctl->chip_type) {
        cvdr_base_addr = hw_ctl->cvdr_viraddr;

        tmpVal = get_reg_val((void __iomem*)((char*)cvdr_base_addr + CVDR_AXI_JPEG_CVDR_CFG));
        if (!is_cvdr_cfg_update()) {
            // max number of AXI write transactionsReturnToLine
            // the maximum expected number is 0x8
            REG_SET_FIELD(tmpVal, AXI_JPEG_CVDR_CFG_max_axiwrite_id, 0x7);
            // Maximum value allowed on the ARID to access the memory
            // maximum expected number is 0x10
            REG_SET_FIELD(tmpVal, AXI_JPEG_CVDR_CFG_max_axiread_id, 0xF);
            set_reg_val((void __iomem*)((char*)cvdr_base_addr+CVDR_AXI_JPEG_CVDR_CFG), tmpVal);
        }

        cam_info("%s: CVDR reg: 0x%08x=0x%08x", __func__,
                 CVDR_AXI_JPEG_CVDR_CFG,
                 get_reg_val((void __iomem*)((char*)cvdr_base_addr+CVDR_AXI_JPEG_CVDR_CFG)));

        wr_qos.reg32 = get_reg_val((void __iomem*)((char*)cvdr_base_addr+CVDR_AXI_JPEG_CVDR_WR_QOS_CFG));
        wr_qos.bits.wr_qos_max = 0x1;
        wr_qos.bits.wr_qos_threshold_01_start = 0x1;
        wr_qos.bits.wr_qos_threshold_01_stop = 0x1;
        wr_qos.bits.wr_qos_threshold_11_start = 0x3;
        wr_qos.bits.wr_qos_threshold_11_stop = 0x3;
        wr_qos.bits.wr_qos_threshold_10_start = 0x3;
        wr_qos.bits.wr_qos_threshold_10_stop = 0x3;
        if (is_hjpeg_qos_update()) {
            wr_qos.bits.wr_qos_sr = 0x0;
        }
        set_reg_val((void __iomem*)((char*)cvdr_base_addr+CVDR_AXI_JPEG_CVDR_WR_QOS_CFG), wr_qos.reg32);
        cam_info("%s: CVDR reg: 0x%08x=0x%08x", __func__,
                CVDR_AXI_JPEG_CVDR_WR_QOS_CFG,
                get_reg_val((void __iomem*)((char*)cvdr_base_addr+CVDR_AXI_JPEG_CVDR_WR_QOS_CFG)));
        rd_qos.reg32 = get_reg_val((void __iomem*)((char*)cvdr_base_addr+CVDR_AXI_JPEG_CVDR_RD_QOS_CFG));
        rd_qos.bits.rd_qos_max = 0x1;
        rd_qos.bits.rd_qos_threshold_11_start = 0x3;
        rd_qos.bits.rd_qos_threshold_11_stop = 0x3;
        rd_qos.bits.rd_qos_threshold_10_start = 0x3;
        rd_qos.bits.rd_qos_threshold_10_stop = 0x3;
        rd_qos.bits.rd_qos_threshold_01_start = 0x1;
        rd_qos.bits.rd_qos_threshold_01_stop = 0x1;
        if (is_hjpeg_qos_update()) {
            rd_qos.bits.rd_qos_sr = 0x0;
        }
        set_reg_val((void __iomem*)((char*)cvdr_base_addr+CVDR_AXI_JPEG_CVDR_RD_QOS_CFG), rd_qos.reg32);
        cam_info("%s: CVDR reg: 0x%08x=0x%08x", __func__,
                CVDR_AXI_JPEG_CVDR_RD_QOS_CFG,
                get_reg_val((void __iomem*)((char*)cvdr_base_addr+CVDR_AXI_JPEG_CVDR_RD_QOS_CFG)));
    }

    ret = set_vp_wr_ready(hw_ctl, config);
    if(0 != ret){
        cam_err("%s (%d) set vp wr ready fail", __func__, __LINE__);
        return;
    }

    ret = set_nr_rd_config(hw_ctl);
    if(0 != ret){
        cam_err("%s (%d) set nr rd config fail", __func__, __LINE__);
        return;
    }

    dump_cvdr_reg(hw_ctl);
}
