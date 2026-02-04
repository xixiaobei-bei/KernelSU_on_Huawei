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


#include <product_config.h>

#include <osl_sem.h>
#include <osl_malloc.h>

#include "bsp_dump.h"
#include "modem_dump_hook.h"
#include "mdrv_om_common.h"
#include "bsp_dump_drv.h"

struct dump_hook_ctrl_s g_dump_hook_ctrl;

dump_handle bsp_dump_register_hook(char * name, dump_hook func)
{
    struct dump_hook_s* pfieldhook = NULL;
    unsigned long flags;

    if(!g_dump_hook_ctrl.ulInitflag)
    {
        (void)bsp_dump_hook_init();
    }

    pfieldhook = (struct dump_hook_s*)osl_malloc(sizeof(struct dump_hook_s));
    if(pfieldhook == NULL)
    {
        printk("malloc failed!\n");
        return -1;
    }

    memset(pfieldhook,0,sizeof(struct dump_hook_s));

    pfieldhook->pfunc = func;
    memcpy(pfieldhook->name,name,(sizeof(pfieldhook->name)>strlen(name))?strlen(name):sizeof(pfieldhook->name));

    spin_lock_irqsave(&g_dump_hook_ctrl.spinlock, flags);
    list_add(&pfieldhook->hook_list, &g_dump_hook_ctrl.list_hook);
    spin_unlock_irqrestore(&g_dump_hook_ctrl.spinlock, flags);

    return (dump_handle)((unsigned long)pfieldhook);
}

void bsp_dump_hook_callback(void)
{
    struct list_head *p,*n;
    struct dump_hook_s* pfieldhook = NULL;

    list_for_each_safe(p,n, &g_dump_hook_ctrl.list_hook)
    {
        pfieldhook = list_entry(p, struct dump_hook_s, hook_list);
        if((pfieldhook)&&(pfieldhook->pfunc))
        {
            pfieldhook->pfunc();
        }
    }

    dump_fetal("save all hook finish\n");
}


s32 bsp_dump_hook_init(void)
{
    if(true == g_dump_hook_ctrl.ulInitflag)
    {
        return 0;
    }
    spin_lock_init(&g_dump_hook_ctrl.spinlock);

    INIT_LIST_HEAD(&g_dump_hook_ctrl.list_hook);

    g_dump_hook_ctrl.ulInitflag = true;

    return 0;
}

s32 bsp_dump_unregister_hook(dump_handle handle)
{
    struct dump_hook_s * pfieldhook = NULL;
    struct dump_hook_s * hook_node = NULL;
    unsigned long flags;

    if(!g_dump_hook_ctrl.ulInitflag)
    {
        printk("init failed!\n");
        return -1;
    }

    if(!handle)
    {
        printk("invalid parameter!\n");
        return -1;
    }

    spin_lock_irqsave(&g_dump_hook_ctrl.spinlock, flags);
    list_for_each_entry(pfieldhook, &g_dump_hook_ctrl.list_hook, hook_list)
    {
        if((dump_handle)((unsigned long)pfieldhook) == handle)
        {
            hook_node = pfieldhook;
        }
    }

    if(!hook_node)
    {
        spin_unlock_irqrestore(&g_dump_hook_ctrl.spinlock, flags);
        return -1;
    }

    list_del(&hook_node->hook_list);
    osl_free(hook_node);
    spin_unlock_irqrestore(&g_dump_hook_ctrl.spinlock, flags);

    return 0;
}

void bsp_dump_show_exthook(void)
{
    struct list_head *p,*n;
    struct dump_hook_s* pfieldhook = NULL;


    list_for_each_safe(p,n, &g_dump_hook_ctrl.list_hook)
    {
        pfieldhook = list_entry(p, struct dump_hook_s, hook_list);
        if((pfieldhook) && (pfieldhook->pfunc))
        {
            printk("name:%s,Function :%pS\n", pfieldhook->name, pfieldhook->pfunc);
        }
    }
}






