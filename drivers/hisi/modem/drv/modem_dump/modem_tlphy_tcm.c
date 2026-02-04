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
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/kdebug.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <asm/string.h>
#include <product_config.h>
#include "osl_types.h"
#include "osl_bio.h"
#include "bsp_rfile.h"
#include "osl_io.h"
#include "bsp_dspload.h"
#include "bsp_dsp.h"
#include "bsp_dump.h"
#include "modem_tlphy_tcm.h"
#include "modem_dump_base.h"
#include "bsp_dump_drv.h"
#include "modem_dump.h"
/*****************************************************************************
* 函 数 名  : om_save_lphy_tcm
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
    u8 * data;
    char file_name[128];
    bool flag = false;
    int fd;
    int ret;
    struct dsp_dump_proc_flag *tl_flag;
    u32 is_beta = 0;

    is_beta = (u32)(bbox_check_edition() == EDITION_INTERNAL_BETA);


    data = (u8 *)ioremap_wc(DDR_TLPHY_IMAGE_ADDR, DDR_TLPHY_IMAGE_SIZE);
    if(NULL == data)
    {
        dump_fetal("ioremap DDR_TLPHY_IMAGE_ADDR fail\n");
        return;
    }

    tl_flag = (struct dsp_dump_proc_flag *)((unsigned long)SHM_BASE_ADDR + SHM_OFFSET_DSP_FLAG);
    if(SAVE_TCM_BEGIN == tl_flag->dsp_dump_flag)
    {
        dump_fetal("carry tldsp tcm to ddr not finished!\n");
        flag = true;
    }
    else if(SAVE_TCM_END == tl_flag->dsp_dump_flag)
    {
        flag = true;
    }
    else
    {
        flag = false;
    }

    /*DSP DDR内存分布参考hi_dsp.h*/
    if(flag == true)
    {
        /* DTCM PUB */
        memset(file_name, 0, sizeof(file_name));
        snprintf(file_name, sizeof(file_name), "%slphy_pub_dtcm.bin", dst_path);

        dump_save_file(file_name,data,LPHY_PUB_DTCM_SIZE);
        dump_fetal("[dump]: save %s finished!\n",file_name);

        /*DTCM MOD*/
        memset(file_name, 0, sizeof(file_name));
        snprintf(file_name, sizeof(file_name), "%slphy_mode_dtcm.bin", dst_path);

        dump_save_file(file_name,data+LPHY_PUB_DTCM_SIZE,LPHY_PRV_DTCM_SIZE);
        dump_fetal("[dump]: save %s finished!\n",file_name);

        if(TRUE == is_beta)
        {
            /* ITCM PUB */
            memset(file_name, 0, sizeof(file_name));
            snprintf(file_name, sizeof(file_name), "%slphy_pub_itcm.bin", dst_path);

            dump_save_file(file_name,data+LPHY_PUB_DTCM_SIZE+LPHY_PRV_DTCM_SIZE,LPHY_PUB_ITCM_SIZE);
            dump_fetal("[dump]: save %s finished!\n",file_name);

            /*ITCM MOD*/
            memset(file_name, 0, sizeof(file_name));
            snprintf(file_name, sizeof(file_name), "%slphy_mode_itcm.bin", dst_path);

            dump_save_file(file_name,data+LPHY_PUB_DTCM_SIZE+LPHY_PUB_ITCM_SIZE+LPHY_PRV_DTCM_SIZE,LPHY_PRV_ITCM_SIZE);
            dump_fetal("[dump]: save %s finished!\n",file_name);
        }

    }
    else if(TRUE == is_beta)
    {
        /*MBB与PHONE均保存DTCM*/
        memset(file_name, 0, sizeof(file_name));
        snprintf(file_name, sizeof(file_name), "%slphy_dtcm.bin", dst_path);
        fd = bsp_open(file_name,RFILE_RDWR|RFILE_CREAT,0660);
        if(fd<0){
            dump_fetal("[dump]:open %s failed ,save lphy_dtcm failed!\n",file_name);
            iounmap(data);
            return;
        }
        ret = bsp_write(fd,data,LPHY_PUB_DTCM_SIZE);
        if(ret != LPHY_PUB_DTCM_SIZE)
            goto err0;
        ret = bsp_write(fd,data+LPHY_PUB_DTCM_SIZE + LPHY_PUB_ITCM_SIZE,LPHY_PRV_DTCM_SIZE);
        if(ret != LPHY_PRV_DTCM_SIZE)
            goto err0;
        ret = bsp_write(fd,data+LPHY_PUB_DTCM_SIZE + LPHY_PUB_ITCM_SIZE + LPHY_PRV_DTCM_SIZE + LPHY_PRV_ITCM_SIZE,LPHY_PRV_DTCM_SIZE);
        if(ret != LPHY_PRV_DTCM_SIZE)
            goto err0;
        dump_fetal("[dump]: save %s finished!\n",file_name);
    err0:
        bsp_close(fd);

        /*如果是PHONE产品则同时保存LPHY ITCM，MBB受空间限制则只保存DTCM部分*/
        if(DUMP_PHONE == dump_get_product_type())
        {
            memset(file_name, 0, sizeof(file_name));
            snprintf(file_name, sizeof(file_name), "%slphy_itcm.bin", dst_path);
            fd = bsp_open(file_name,RFILE_RDWR|RFILE_CREAT,0660);
            if(fd<0){
                dump_fetal("[dump]:open %s failed ,save lphy_itcm failed!\n",file_name);
                iounmap(data);
                return;
            }
            ret = bsp_write(fd,data+LPHY_PUB_DTCM_SIZE,LPHY_PUB_ITCM_SIZE);
            if(ret != LPHY_PUB_ITCM_SIZE)
                goto err1;
            ret = bsp_write(fd,data+LPHY_PUB_DTCM_SIZE+LPHY_PUB_ITCM_SIZE+LPHY_PRV_DTCM_SIZE,LPHY_PRV_ITCM_SIZE);
            if(ret != LPHY_PRV_ITCM_SIZE)
                goto err1;
            ret = bsp_write(fd,data+LPHY_PUB_DTCM_SIZE+LPHY_PUB_ITCM_SIZE+LPHY_PRV_DTCM_SIZE+LPHY_PRV_ITCM_SIZE+LPHY_PRV_DTCM_SIZE,LPHY_PRV_ITCM_SIZE);
            if(ret != LPHY_PRV_ITCM_SIZE)
                goto err1;
            dump_fetal("[dump]: save %s finished!\n",file_name);
        err1:
            bsp_close(fd);
        }
    }
    tl_flag->dsp_dump_flag = 0;

    iounmap(data);
    return;
}

