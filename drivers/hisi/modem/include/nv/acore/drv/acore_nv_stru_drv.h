/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __DRV_NV_DEF_ACORE_H__
#define __DRV_NV_DEF_ACORE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "acore_nv_id_drv.h"

/*****************************************************************************
 结构名    : NV_SECBOOT_ENABLE_FLAG
 结构说明  : NV_SECBOOT_ENABLE_FLAG结构 ID=50201
*****************************************************************************/
typedef struct
{
    u16 usNVSecBootEnableFlag; /*Range:[0,1]*/
}NV_SECBOOT_ENABLE_FLAG;

/* USB Feature for usb id:50075 */
typedef struct
{
    u8 flags;
    u8 reserve1;
    u8 hibernation_support;/**/
    u8 pc_driver;
    u8 detect_mode;/*0=pmu detect; 1=no detect(for fpga&cpe); 2=car module vbus detect */
    u8 enable_u1u2_workaround;
    u8 usb_gpio_support:4;
    u8 usb_mode:4;
    u8 network_card;
} NV_USB_FEATURE;

typedef struct
{
    u32 smp_sch; 
}DRV_USB_DBG_STRU;

/* C核单独复位按需开关特性配置NV项 = 0xd134 */
typedef struct
{
    u32 is_feature_on:1;         /* bit0, 整个C核单独复位功能是否打开 */
    u32 is_connected_ril:1;      /* bit1, 是否和RIL对接 */
    u32 reserve:30;              /* bit 2-31*/
} DRV_CCORE_RESET_STRU;

/* NVID=0xd144 begin */
typedef struct
{
    u32 mdm_dump     :1;     /* modem_dump.bin */
    u32 mdm_sram     :1;     /* modem_sram.bin */
    u32 mdm_share    :1;     /* modem_share.bin */
    u32 mdm_ddr      :1;     /* modem_ddr.bin */
    u32 lphy_tcm     :1;     /* lphy_tcm.bin */
    u32 lpm3_tcm     :1;     /* lpm3_tcm.bin */
    u32 ap_etb       :1;     /* ap_etb.bin */
    u32 mdm_etb      :1;     /* modem_etb.bin */
    u32 reset_log    :1;     /*reset.log*/
    u32 cphy_tcm     :1;     /* cphy_tcm.bin */
    u32 mdm_secshare :1;     /* secshare.bin */
    u32 mdm_dts      :1;     /* modem_dts.bin */    
    u32 llram_share  :1;     /* modem_llram.bin */
    u32 reserved2   :19;
}DUMP_FILE_BITS;

typedef struct
{
    union
    {
        u32         file_value;
        DUMP_FILE_BITS  file_bits;
    } file_list;
    u32             file_cnt; /* 记录异常文件份数 */
}DUMP_FILE_CFG_STRU;

typedef struct
{
    u32 enable;        /*总线挂死寄存器解析使能标志, 0, 去使能;  1, 使能*/
    u32 reset_enable;  /*复位使能标志, 0, 去使能; 1, 使能*/
}DRV_DLOCK_CFG_STRU;
/* dlock = 0xd155 */

/*end rse mipi 配置方式*/

/* 0xD14d */
typedef struct
{
    u32 product         :2;
    u32 reset           :1;
    u32 save_bakup      :1;
    u32 save_image      :1;
    u32 save_ddr        :1;
    u32 resume_bakup    :1;  /*Range:[0,1]*/
    u32 resume_img      :1;  /*Range:[0,1]*/
    u32 reserved        :24;
}NV_DEBUG_CFG_STRU;

/*NV ID = 0xd157 DVS使能控制及hpm读值配置*/
//[NV elf编译项目] 结构定义重复 xuziheng/306651 20171209
#ifndef NV_DEFINE
typedef struct {
    u32 dvs_en;
    u32 hpm_delay;
}ST_DVS_CONFIG_STRU;
#endif

typedef struct
{
    u32 enable;         /*mbb ap侧noc err probe解析使能标志, 0, 去使能;  1, 使能*/
    u32 reset_enable;   /*mbb ap侧复位使能标志, 0, 去使能; 1, 使能*/
}DRV_NOC_AP_CFG_STRU;
typedef struct
{
    u32 enable;        /*mbb ap侧dmss解析使能标志, 0, 去使能;  1, 使能*/
    u32 reset_enable;  /*mbb ap侧dmss复位使能标志, 0, 不使能;  1, 使能*/
}DRV_DMSS_CFG_STRU;

typedef struct {
    u32 ulRatePeriod;   /* 流控周期单位ms , 取值必须1~0xFFFFFFFF */
    u32 ulRateEn[2];    /* 流量控制使能，每个bit对应1个通道，ulFlowEn[0]对应channel 0~31 bit0 :channel 0，ulFlowEn[1]对应channel 32~63 bit32:chan32，0不使能，1使能 */
    u32 ulRateThr[64];  /* 流量控制阈值，对应64个通道，取值0~0xFFFFFFFF，0：通道始终进行限流，通道优先级始终保持最低 */
}DRV_DIAG_RATE_STRU;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

