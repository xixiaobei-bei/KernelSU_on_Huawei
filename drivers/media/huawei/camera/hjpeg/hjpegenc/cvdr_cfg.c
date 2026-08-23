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
        if (is_hjpeg_wr_port_addr_update()) {
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
    //FIXME:this must be use ceil align up
    /* uint32_t num_DUs_per_line = width * pix_size / 8 / 128; */
    /* uint32_t total_num_bytes  = num_DUs_per_line * height * 128; */
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

    //FIXME: this must bue user ceil to align up float to int
    /* desc->access_limiter = 32 * pix_size / 128  ; */
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
            if (is_hjpeg_wr_port_addr_update()) {
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

    cam_debug("%s acess_limiter = %d, last_page = %d, line_stride = %d, width = %d,  height = %d",
        __func__, desc->access_limiter, desc->last_page, desc->line_stride, width, height);

    return 0;
}
#define PREFETCH_BY_PASS (1 << 31)
static int set_vp_wr_ready(hjpeg_hw_ctl_t *hw_ctl, jpgenc_config_t* config)
{
    int ret;
    U_VP_WR_CFG tmp_cfg;
    U_VP_WR_CFG_UPDATE tmp_cfg_update;
    U_VP_WR_AXI_LINE tmp_line;
    U_VP_WR_AXI_FS tmp_fs;
    U_CVDR_SRT_LIMITER_VP_WR_0 lmt;

    u32 vp_wr_cfg_offset;
    u32 vp_wr_axi_line_offset;
    u32 vp_wr_if_cfg_offset;
    u32 vp_wr_axi_fs_offset;
    u32 buf_addr;

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
            vp_wr_cfg_offset      = CVDR_AXI_JPEG_VP_WR_CFG_0_OFFSET;
            vp_wr_axi_line_offset = CVDR_AXI_JPEG_VP_WR_AXI_LINE_0_OFFSET;
            vp_wr_axi_fs_offset   = CVDR_AXI_JPEG_VP_WR_AXI_FS_0_OFFSET;
            // for smmu bypass
            vp_wr_if_cfg_offset   = CVDR_AXI_JPEG_VP_WR_IF_CFG_0_OFFSET;

            lmt.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base + CVDR_AXI_JPEG_LIMITER_VP_WR_0_OFFSET));
            lmt.bits.vpwr_access_limiter_0_0 = desc.access_limiter;
            lmt.bits.vpwr_access_limiter_1_0 = desc.access_limiter;
            lmt.bits.vpwr_access_limiter_2_0 = desc.access_limiter;
            lmt.bits.vpwr_access_limiter_3_0 = desc.access_limiter;
            set_reg_val((void __iomem*)((char*)cvdr_srt_base + CVDR_AXI_JPEG_LIMITER_VP_WR_0_OFFSET), lmt.reg32);

            break;
        }
        case WR_PORT_25:
        {
            if (is_hjpeg_wr_port_addr_update()) {
                vp_wr_cfg_offset      = CVDR_SRT_VP_WR_CFG_25_OFFSET_UPDATE;
                vp_wr_axi_line_offset = CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET_UPDATE;
                vp_wr_axi_fs_offset   = CVDR_SRT_VP_WR_AXI_FS_25_OFFSET_UPDATE;
                // for smmu bypass
                vp_wr_if_cfg_offset   = CVDR_SRT_VP_WR_IF_CFG_25_OFFSET_UPDATE;
            } else {
                vp_wr_cfg_offset      = CVDR_SRT_VP_WR_CFG_25_OFFSET;
                vp_wr_axi_line_offset = CVDR_SRT_VP_WR_AXI_LINE_25_OFFSET;
                vp_wr_axi_fs_offset   = CVDR_SRT_VP_WR_AXI_FS_25_OFFSET;
                // for smmu bypass
                vp_wr_if_cfg_offset   = CVDR_SRT_VP_WR_IF_CFG_25_OFFSET;
            }

            lmt.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base + CVDR_AXI_JPEG_LIMITER_VP_WR_25_OFFSET));
            lmt.bits.vpwr_access_limiter_0_0 = desc.access_limiter;
            lmt.bits.vpwr_access_limiter_1_0 = desc.access_limiter;
            lmt.bits.vpwr_access_limiter_2_0 = desc.access_limiter;
            lmt.bits.vpwr_access_limiter_3_0 = desc.access_limiter;
            set_reg_val((void __iomem*)((char*)cvdr_srt_base + CVDR_AXI_JPEG_LIMITER_VP_WR_25_OFFSET), lmt.reg32);

            break;
        }
        default:
            cam_err("%s, invalid wr port[%d]", __func__, wr_port);
            return -1;
    }
    // config vp wr cfg
    if (is_hjpeg_iova_update()) {
        tmp_cfg_update.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base + vp_wr_cfg_offset));
        tmp_cfg_update.bits.vpwr_pixel_format    = desc.pix_fmt;
        tmp_cfg_update.bits.vpwr_pixel_expansion = desc.pix_expan;
        tmp_cfg_update.bits.vpwr_last_page       = desc.last_page;
        tmp_cfg_update.bits.reserved_1 = 0x0;
        if (WR_PORT_25 == wr_port) {
           // FIXME:no vpwr_access_limiter in register table ON ES
           tmp_cfg_update.bits.vpwr_access_limiter = desc.access_limiter;
       }
        set_reg_val((void __iomem*)((char*)cvdr_srt_base + vp_wr_cfg_offset), tmp_cfg_update.reg32);
    } else {
        tmp_cfg.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base + vp_wr_cfg_offset));
        tmp_cfg.bits.vpwr_pixel_format    = desc.pix_fmt;
        tmp_cfg.bits.vpwr_pixel_expansion = desc.pix_expan;
        tmp_cfg.bits.vpwr_last_page       = desc.last_page;
        if (WR_PORT_25 == wr_port) {
            // FIXME:no vpwr_access_limiter in register table ON ES
            tmp_cfg.bits.vpwr_access_limiter = desc.access_limiter;
        }
        set_reg_val((void __iomem*)((char*)cvdr_srt_base + vp_wr_cfg_offset), tmp_cfg.reg32);
   }

    // config vp wr axi line
    tmp_line.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base + vp_wr_axi_line_offset));
    tmp_line.bits.vpwr_line_stride = desc.line_stride;
    tmp_line.bits.vpwr_line_wrap   = desc.line_wrap;
    set_reg_val((void __iomem*)((char*)cvdr_srt_base + vp_wr_axi_line_offset), tmp_line.reg32);

    // smmu bypass
    if (hw_ctl->smmu_bypass == BYPASS_YES)
    {
        set_reg_val((void __iomem*)((char*)cvdr_srt_base + vp_wr_if_cfg_offset),
            (get_reg_val((void __iomem*)((char*)cvdr_srt_base + vp_wr_if_cfg_offset))|(PREFETCH_BY_PASS)));  /*lint !e648*/
    }

    buf_addr = config->buffer.output_buffer + JPGENC_HEAD_SIZE;
    // config vp wr axi fs
    tmp_fs.reg32 = get_reg_val((void __iomem*)((char*)cvdr_srt_base + vp_wr_axi_fs_offset));

    tmp_fs.bits.vpwr_address_frame_start = buf_addr >> 4;

    if (is_hjpeg_iova_update()) {
        set_reg_val((void __iomem*)((char*)cvdr_srt_base + vp_wr_axi_fs_offset), tmp_fs.reg32 >> 2);
    } else {
        set_reg_val((void __iomem*)((char*)cvdr_srt_base + vp_wr_axi_fs_offset), tmp_fs.reg32);
    }

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

        // max number of AXI write transactionsReturnToLine
        // the maximum expected number is 0x8
        REG_SET_FIELD(tmpVal, AXI_JPEG_CVDR_CFG_max_axiwrite_id, 0x7);
        // Maximum value allowed on the ARID to access the memory
        // maximum expected number is 0x10
        REG_SET_FIELD(tmpVal, AXI_JPEG_CVDR_CFG_max_axiread_id, 0xF);
        set_reg_val((void __iomem*)((char*)cvdr_base_addr+CVDR_AXI_JPEG_CVDR_CFG), tmpVal);
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



