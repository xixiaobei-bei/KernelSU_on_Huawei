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

/*lint --e{528,715} */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/printk.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/semaphore.h>
#include <linux/regulator/consumer.h>

#include "product_config.h"
#include "osl_bio.h"
#include "osl_sem.h"
#include "osl_thread.h"
#include "osl_spinlock.h"
#include "soc_clk.h"
#include "hi_efuse.h"
#include "bsp_memmap.h"

#include "bsp_icc.h"
#include "bsp_llt.h"
#include "bsp_slice.h"
#include "bsp_efuse.h"

#include "securec.h"

#define THIS_MODU mod_efuse
#ifdef HI_K3_EFUSE
#include <mdrv.h>
#include "../../adrv/adrv.h"
#endif

struct work_struct g_modem_efuse_work;

EFUSE_DATA_STRU g_modem_efuse_msg = {0};

EFUSE_DEBUG_INFO g_modem_efuse_debug_info;

struct semaphore g_modem_efuse_extra_op_semaphore;

void bsp_efuse_agent_opt_read(EFUSE_DATA_STRU *msg)
{
    efuse_print_info("efuse read start group = %d, length = %d\n", msg->start, msg->len);
#ifdef HI_K3_EFUSE
    if ((msg->start == EFUSE_GRP_AVS) && (msg->len == EFUSE_AVS_GROUP_SIZE)) {
#ifdef EFUSE_AVS_VALUE
        msg->ret = get_efuse_avs_value((u8 *)msg->buf, msg->len, 0);
#endif
    } else {
        msg->ret = EFUSE_ERROR_ARGS;
        efuse_print_error("error group for efuse read, group = %d\n", msg->start);
    }
#else
    if ((msg->start == EFUSE_GRP_AVS) && (msg->len == EFUSE_AVS_GROUP_SIZE)) {
        msg->ret = bsp_efuse_read(msg->buf, EFUSE_AVS_GROUP_START, EFUSE_AVS_GROUP_SIZE);
    } else {
        msg->ret = bsp_efuse_read(msg->buf, msg->start, msg->len);
    }
#endif
    efuse_print_info("efuse read finish, ret = %d\n", msg->ret);
}

void bsp_efuse_agent_opt_write(EFUSE_DATA_STRU *msg)
{

    efuse_print_info("efuse write start group = %d, length = %d\n", msg->start, msg->len);
#ifdef HI_K3_EFUSE
    if ((msg->start == EFUSE_GRP_HUK) && (!is_in_llt())) {
        msg->ret = set_efuse_kce_value((unsigned char *)msg->buf, (unsigned int)msg->len * 4, 0);
    } else {
        msg->ret = EFUSE_ERROR_ARGS;
        efuse_print_error("error group for efuse write, group = %d\n", msg->start);
    }
#else
    msg->ret = bsp_efuse_write(msg->buf, msg->start,msg->len);
#endif
    efuse_print_info("efuse write finish, ret = %d\n", msg->ret);
}

#ifndef HI_K3_EFUSE
void bsp_efuse_agent_opt_sec_read(EFUSE_DATA_STRU *msg)
{
    efuse_print_info("efuse sec read start group = %d, length = %d\n", msg->start, msg->len);
    msg->ret = bsp_efuse_sec_read(msg->buf, msg->start,msg->len);
    efuse_print_info("efuse sec read finish, ret = %d\n", msg->ret);
}

void bsp_efuse_agent_opt_sec_write(EFUSE_DATA_STRU *msg)
{
    efuse_print_info("efuse sec write start group = %d, length = %d\n", msg->start, msg->len);

    efuse_print_info("efuse sec write finish, ret = %d\n", msg->ret);
}
#endif

struct efuse_agent_opt {
    u32 opt_code;
    void (*func_agent_opt)(EFUSE_DATA_STRU *);
};

struct efuse_agent_opt g_efuse_agent_opt[] = {
    {EFUSE_OPT_READ,        bsp_efuse_agent_opt_read},
    {EFUSE_OPT_WRITE,       bsp_efuse_agent_opt_write},
#ifndef HI_K3_EFUSE
    {EFUSE_OPT_SEC_READ,    bsp_efuse_agent_opt_sec_read},
    {EFUSE_OPT_SEC_WRITE,   bsp_efuse_agent_opt_sec_write},
#endif
    {0xFFFFFFFF,            NULL},
};


void efuse_handle_work(struct work_struct *work)
{
    u32 i;
    int length;
    u32 channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_EFUSE;

    EFUSE_DATA_STRU *msg = &g_modem_efuse_msg;

    g_modem_efuse_debug_info.rw_start_stamp = bsp_get_slice_value();

    for (i = 0; i < sizeof(g_efuse_agent_opt)/sizeof(g_efuse_agent_opt[0]); i++) {
        if (msg->opt == g_efuse_agent_opt[i].opt_code) {
            g_efuse_agent_opt[i].func_agent_opt(msg);
            break;
        }
    }

    if (i == sizeof(g_efuse_agent_opt)/sizeof(g_efuse_agent_opt[0])) {
        msg->ret = EFUSE_ERROR_ARGS;
        efuse_print_error("error opt, opt = 0x%08X\n", msg->opt);
    }

    g_modem_efuse_debug_info.rw_end_stamp = bsp_get_slice_value();
    if ((msg->opt != EFUSE_OPT_SEC_READ) && (msg->opt != EFUSE_OPT_SEC_WRITE)) {
        length = bsp_icc_send(ICC_CPU_MODEM, channel_id, (unsigned char*)msg, (u32)sizeof(EFUSE_DATA_STRU));
        if (length != (int)sizeof(EFUSE_DATA_STRU)) {
            msg->ret = EFUSE_ERROR_ICC_SEND_FAIL;
            efuse_print_error("send len(%d) != expected len(%d)\n", length, (int)sizeof(EFUSE_DATA_STRU));
            return;
        }
    }

    g_modem_efuse_debug_info.icc_send_stamp = bsp_get_slice_value();
}

int bsp_efuse_data_receive(u32 channel_id , u32 len, void* context)
{
    int ret;
    int length;

    length = bsp_icc_read(channel_id, (u8*)&g_modem_efuse_msg, (u32)sizeof(EFUSE_DATA_STRU));
    if (length != (int)sizeof(EFUSE_DATA_STRU)) {
        efuse_print_error("read len(%d) != expected len(%d)\n", length, (int)sizeof(EFUSE_DATA_STRU));
        return EFUSE_ERROR_ICC_READ_FAIL;
    }

    g_modem_efuse_debug_info.icc_callback_stamp = bsp_get_slice_value();

    ret = (int)schedule_work(&g_modem_efuse_work);

    efuse_print_info("efuse schedule_work begin, ret = %d\n", ret);

    return EFUSE_OK;
}

int bsp_efuse_extra_data_receive(u32 channel_id , u32 len, void* context)
{
    int length;

    length = bsp_icc_read(channel_id, (u8*)&g_modem_efuse_msg, (u32)sizeof(EFUSE_DATA_STRU));
    if (length != (int)sizeof(EFUSE_DATA_STRU)) {
        efuse_print_error("read len(%d) != expected len(%d)\n", length, (int)sizeof(EFUSE_DATA_STRU));
        return EFUSE_ERROR_ICC_READ_FAIL;
    }

    up(&g_modem_efuse_extra_op_semaphore);

    efuse_print_info("efuse read extra data ok\n");

    return EFUSE_OK;
}

int bsp_efuse_agent_init(void)
{
    int ret;
    u32 channel_id;

    INIT_WORK(&g_modem_efuse_work, efuse_handle_work);

    sema_init(&g_modem_efuse_extra_op_semaphore, 0);

    ret = memset_s((void*)&g_modem_efuse_debug_info, sizeof(g_modem_efuse_debug_info), 0, sizeof(g_modem_efuse_debug_info));
    if (ret != EOK) {
        efuse_print_error("failed to memset_s in bsp_efuse_agent_init!\n");
        return ret;
    }

    channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_EFUSE;
    ret = bsp_icc_event_register(channel_id, bsp_efuse_data_receive, NULL, NULL, NULL);
    if (ret) {
        efuse_print_error("fail to register efuse icc event, ret = %d\n", ret);
        return EFUSE_ERROR;
    }


    efuse_print_info("modem efuse agent init ok\n");

    return EFUSE_OK;
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(bsp_efuse_agent_init);
#endif

