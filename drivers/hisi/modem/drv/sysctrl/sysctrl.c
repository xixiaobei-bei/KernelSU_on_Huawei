/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#include <linux/io.h>
#include <osl_module.h>
#include <bsp_dt.h>
#include <bsp_sysctrl.h>

#undef THIS_MODU
#define THIS_MODU mod_sysctrl

struct sysctrl_tag {
    void *addr_virt[SYSCTRL_MAX];
    unsigned long addr_phy[SYSCTRL_MAX];
    unsigned int reg_sum;
};
struct sysctrl_tag g_sysctrl;

static unsigned long reg_node_info_get(device_node_s *np, unsigned int sc_index)
{
    const __be32 *spec = NULL;
    int len = 0;
    int na;
    int ns;

    na = bsp_dt_n_addr_cells(np);
    ns = bsp_dt_n_size_cells(np);

    spec = bsp_dt_get_property(np, "reg", &len);
    if (spec == NULL || (unsigned int)len < (unsigned int)(na + ns) * sizeof(*spec)) {
        sc_err_func("index = %d is out of range\n", sc_index);
        return 0;
    }

    return (unsigned long)bsp_dt_read_number(spec + sc_index * (unsigned int)(na + ns), na);
}

static void *virt_addr_lookup(unsigned long phy_addr)
{
    u32 i;
    unsigned long addr = phy_addr & ~0xfff;

    for (i = 0; i < g_sysctrl.reg_sum; i++) {
        if (addr == g_sysctrl.addr_phy[i]) {
            return (void *)(uintptr_t)((uintptr_t)g_sysctrl.addr_virt[i] + phy_addr - g_sysctrl.addr_phy[i]);
        }
    }

    return NULL;
}

void *bsp_sysctrl_addr_get(const void *phy_addr)
{
    void *virt_addr = virt_addr_lookup((uintptr_t)phy_addr);

    if (virt_addr == NULL) {
        sc_err("phy_addr[0x%x] is not exit in sysctrl\n", (unsigned int)(uintptr_t)phy_addr);
    }

    return virt_addr;
}
EXPORT_SYMBOL_GPL(bsp_sysctrl_addr_get);

void *bsp_sysctrl_addr_byindex(sysctrl_index_e sc_index)
{
    if (sc_index >= SYSCTRL_MAX) {
        sc_err("index %d is error,largest is %d\n", sc_index, SYSCTRL_MAX - 1);
        return NULL;
    }

    if (g_sysctrl.addr_virt[sc_index] == NULL) {
        sc_err("index %d addr is no exist\n", sc_index);
    }

    return g_sysctrl.addr_virt[sc_index];
}
EXPORT_SYMBOL_GPL(bsp_sysctrl_addr_byindex);

void *bsp_sysctrl_addr_phy_byindex(sysctrl_index_e sc_index)
{
    if (sc_index >= SYSCTRL_MAX) {
        sc_err("index %d is error,largest is %d\n", sc_index, SYSCTRL_MAX - 1);
        return NULL;
    }

    if (g_sysctrl.addr_phy[sc_index] == 0) {
        sc_err("index %d addr_phy is no exist\n", sc_index);
    }

    return (void *)(uintptr_t)g_sysctrl.addr_phy[sc_index];
}
EXPORT_SYMBOL_GPL(bsp_sysctrl_addr_phy_byindex);

int sysctrl_init(void)
{
    u32 i;
    device_node_s *node = NULL;
    int ret;

    sc_err("[init]start\n");

    /* 初始化时将结构体中的数组赋值"0"或"NULL"，当底软使用者传入枚举大于reg_num时，返回0地址 */
    for (i = 0; i < SYSCTRL_MAX; i++) {
        g_sysctrl.addr_virt[i] = NULL;
        g_sysctrl.addr_phy[i] = 0;
    }

    node = bsp_dt_find_compatible_node(NULL, NULL, "hisilicon,sysctrl_app");
    if (node == NULL) {
        sc_err_func("can't find dts node\n");
        return -1;
    }

    ret = bsp_dt_property_read_u32(node, "reg_sum", &g_sysctrl.reg_sum);
    if (ret) {
        sc_err_func("can't read reg_sum from dts, ret = %d\n", ret);
        return -1;
    }

    g_sysctrl.reg_sum = (g_sysctrl.reg_sum <= SYSCTRL_MAX) ? g_sysctrl.reg_sum : SYSCTRL_MAX;
    for (i = 0; i < g_sysctrl.reg_sum; i++) {
        g_sysctrl.addr_phy[i] = reg_node_info_get(node, i);
        if (g_sysctrl.addr_phy[i] != 0) {
            g_sysctrl.addr_virt[i] = bsp_dt_iomap(node, (int)i);
            if (g_sysctrl.addr_virt[i] == NULL) {
                sc_err_func("iomap error, index = %d\n", i);
                return -1;
            }
        } else {
            sc_info("index %d addr_phy is no exist", i);
        }
    }

    sc_err("[init]ok\n");

    return 0;
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
core_initcall(sysctrl_init);
#endif
