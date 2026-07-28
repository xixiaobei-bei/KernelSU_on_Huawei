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

/*lint --e{528,537,715} */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>

#include <product_config.h>

#include <mdrv_module_init.h>

/* modem driver init in kernel thread to make sure it runs in a kernel domain */
#define MODEM_INIT_IN_KERNEL_THREAD


DECLARE_COMPLETION(modem_module_init_done_complete);


int modem_module_init_start(void *data)
{
    u32 i;
    int ret = 0;

    for (i = 0; i < sizeof(g_modem_module_init_func)/sizeof(g_modem_module_init_func[0]); i++) {
        ret = g_modem_module_init_func[i]();
        if (ret) {
            printk(KERN_ERR"%pF return error, ret = %d\n", g_modem_module_init_func[i], ret);
        }
    }

    printk(KERN_ERR"balong modem init done\n");

    complete(&modem_module_init_done_complete);

    return ret;
}

__init int bsp_modem_module_init(void)
{
    int ret = 0;

#ifdef MODEM_INIT_IN_KERNEL_THREAD
    struct task_struct *modem_module_kthread;

    modem_module_kthread = kthread_run(modem_module_init_start, NULL, "modem_module_init");
    if (!IS_ERR(modem_module_kthread)) {
        printk(KERN_ERR"succeed to start balong modem module thread\n");
    } else {
        ret = (int)PTR_ERR(modem_module_kthread);
        printk(KERN_ERR"fail to start balong modem module thread, ret = %d\n", ret);
    }

    wait_for_completion(&modem_module_init_done_complete);
#else
    ret = modem_module_init_start(NULL);
#endif

    return ret;
}

module_init(bsp_modem_module_init);

