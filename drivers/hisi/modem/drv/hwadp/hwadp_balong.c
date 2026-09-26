/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#include <bsp_dt.h>
#include <linux/module.h>
#include <product_config.h>
#include <mdrv_memory.h>
#include <osl_types.h>
#include <bsp_sram.h>
#include <bsp_shared_ddr.h>
#include <bsp_sysctrl.h>
#include <bsp_hwadp.h>
#include <bsp_print.h>

#undef THIS_MODU
#define THIS_MODU mod_hwadp

#define hwadp_printf(fmt, ...) bsp_err(fmt, ##__VA_ARGS__)

static void hwadp_dts_register_base_addr(void)
{
    device_node_s *node = NULL;
    device_node_s *np = NULL;
    int retval;

    node = bsp_dt_find_compatible_node(NULL, NULL, "hisilicon,hardware_adapt");
    if (node == NULL) {
        hwadp_printf("dts node not found!\n");
        return;
    }

    bsp_dt_for_each_available_child_of_node(node, np) {
        unsigned int ip_type = ~0U;
        unsigned int int_type = ~0U;
        unsigned int irq_num;
        int na, ns;
        int len = 0;
        unsigned long base_addr = 0;
        const __be32 *prop = NULL;

        if (!np->name || !strlen(np->name)) {
            continue;
        }

        /* try to obtain ip base address */
        na = bsp_dt_n_addr_cells(np);
        ns = bsp_dt_n_size_cells(np);
        prop = bsp_dt_get_property(np, "reg", &len);
        retval = bsp_dt_property_read_u32(np, "ip_type", &ip_type);
        if (prop && (len == (na + ns) * (int)sizeof(*prop))) {
            base_addr = (unsigned long)bsp_dt_read_number(prop, na);
            (void)bsp_dt_iomap(np, 0);
            if (retval) {
                ip_type = ~0U;
            }
            retval = bsp_hwadp_register_base_addr((bsp_ip_type_e)ip_type, (const char *)np->name,
                                                  (void *)(uintptr_t)base_addr);
            if (retval) {
                hwadp_printf("hwadp:failed to register base addr!ip_type=%d base_addr=%lx name=%s\n",
                             (int)ip_type, base_addr, np->name);
            }
            continue;
        }

        irq_num = bsp_dt_irq_parse_and_map(np, 0);
        retval = bsp_dt_property_read_u32(np, "int_type", &int_type);
        if (retval) {
            int_type = ~0U;
        }

        if (bsp_hwadp_register_irq_num((bsp_int_type_e)int_type, (const char *)np->name, irq_num)) {
            hwadp_printf("hwadp:failed to register irq number!int_type=%d irq_num=%d name=%s\n",
                         (int)int_type, irq_num, np->name);
        }
    }
}

int hwadp_init(void)
{
    int ret = MDRV_OK;
    void *addr_ao = NULL;
    void *addr_pd = NULL;
    void *addr_mdm = NULL;
    void *addr_ap_peri = NULL;

    hwadp_dts_register_base_addr();
    /* IP base */
    addr_ao = bsp_sysctrl_addr_phy_byindex(SYSCTRL_AO);
    addr_pd = bsp_sysctrl_addr_phy_byindex(SYSCTRL_PD);
    addr_mdm = bsp_sysctrl_addr_phy_byindex(SYSCTRL_MDM);
    addr_ap_peri = bsp_sysctrl_addr_phy_byindex(SYSCTRL_AP_PERI);

#ifdef DDR_ZSP_UP_ADDR
    ret += bsp_hwadp_register_base_addr(BSP_IP_TYPE_AHB, "AHB", (void *)(uintptr_t)DDR_ZSP_UP_ADDR);
#endif
    ret += bsp_hwadp_register_base_addr(BSP_IP_TYPE_SYSCTRL, "SYSCTRL", addr_ao);
    if (addr_ao != addr_pd) {
        ret += bsp_hwadp_register_base_addr(BSP_IP_TYPE_SYSCTRL_PD, "SYSCTRL_PD", addr_pd);
    }
    if (addr_mdm != NULL) {
        ret += bsp_hwadp_register_base_addr(BSP_IP_TYPE_SYSCTRL_MDM, "SYSCTRL_MDM", addr_mdm);
    }
    if (addr_ap_peri != NULL) {
        ret += bsp_hwadp_register_base_addr(BSP_IP_TYPE_PERI_SYSCTRL, "PERI_SYSCTRL", addr_ap_peri);
    }
#ifdef SHM_OFFSET_WAN
    ret += bsp_hwadp_register_ddr(BSP_DDR_SECT_TYPE_WAN, BSP_DDR_SECT_ATTR_NONCACHEABLE,
                                  (void *)(uintptr_t)(SHM_BASE_ADDR + SHM_OFFSET_WAN),
                                  (void *)SHD_DDR_V2P((unsigned long)SHM_BASE_ADDR + SHM_OFFSET_WAN),
                                  SHM_SIZE_WAN);
#endif
    if (!ret) {
        hwadp_printf("hwadp init ok.\n");
    } else {
        hwadp_printf("hwadp init fail.ret=%d\n", ret);
    }

    return ret;
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
subsys_initcall(hwadp_init);
#endif
