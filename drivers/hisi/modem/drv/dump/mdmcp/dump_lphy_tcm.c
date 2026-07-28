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

#include <linux/timer.h>
#include <linux/thread_info.h>
#include <linux/syslog.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <asm/string.h>
#include "product_config.h"
#include "securec.h"
#include "osl_types.h"
#include "osl_bio.h"
#include "osl_io.h"
#include "bsp_rfile.h"
#include "bsp_dspload.h"
#include "bsp_dsp.h"
#include "bsp_dump.h"
#include "bsp_ddr.h"
#include "dump_file.h"
#include "dump_config.h"
#include <bsp_slice.h>
#include <bsp_nvim.h>
#include <dump_lphy_tcm.h>
#undef	THIS_MODU
#define THIS_MODU mod_dump


char * lphy_image_ddr_addr = NULL;
char * lphy1_image_ddr_addr = NULL;
/*****************************************************************************
* 函 数 名  : dump_save_lphy_log
* 功能描述  : 保存lphy全部的log文件
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :
*****************************************************************************/
void dump_save_lphy_log(char* data,char* dst_path, u32 file_size)
{
    char file_name[128] = {0};
    if(file_size > (u32)DDR_TLPHY_IMAGE_SIZE)
    {
        dump_error("save %s failed, file_size: 0x%x\n", file_name, file_size);
        file_size = (u32)DDR_TLPHY_IMAGE_SIZE;
    }
    /* coverity[HUAWEI DEFECT] */
    memset_s(file_name, sizeof(file_name), 0, sizeof(file_name));
    /* coverity[HUAWEI DEFECT] */
    snprintf_s(file_name, sizeof(file_name), (sizeof(file_name)-1), "%slphy_dump.bin", dst_path);
    file_name[127]='\0';
    dump_save_file(file_name, data, file_size);
    dump_ok("save %s ok\n", file_name);
}
/*****************************************************************************
* 函 数 名  : dump_save_all_tcm
* 功能描述  : 保存全部的tcm文件
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_all_tcm(char* data,char* dst_path)
{
}

/*****************************************************************************
* 函 数 名  : dump_save_some_tcm
* 功能描述  : 保存全部的dtcm和itcm文件
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_some_tcm(char* data,char* dst_path)
{
}
/*****************************************************************************
* 函 数 名  : dump_save_lphy_tcm
* 功能描述  : 保存tldsp的镜像
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_lphy_tcm(char * dst_path)
{
    DUMP_FILE_CFG_STRU* cfg = dump_get_file_cfg();

    if(DUMP_PHONE == dump_get_product_type()
        && DUMP_ACCESS_MDD_DDR_NON_SEC != dump_get_access_mdmddr_type())
    {
        return;
    }

    if(cfg->file_list.file_bits.lphy_tcm == 0)
    {
        return;
    }

    lphy_image_ddr_addr = (char *)ioremap_wc(NXDSP_MDDR_FAMA(DDR_TLPHY_IMAGE_ADDR), DDR_TLPHY_IMAGE_SIZE);
    if(NULL == lphy_image_ddr_addr)
    {
        dump_error("ioremap DDR_TLPHY_IMAGE_ADDR fail\n");
        return;
    }
    dump_save_lphy_log(lphy_image_ddr_addr,dst_path,(u32)readl(lphy_image_ddr_addr+0x18));
/* ltev存log */
    iounmap(lphy_image_ddr_addr);
    return;
}
void dump_lphy_init(void)
{
}
