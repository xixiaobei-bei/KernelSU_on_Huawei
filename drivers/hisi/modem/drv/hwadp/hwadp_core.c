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

#include <linux/types.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <product_config.h>
#include <osl_irq.h>
#include <soc_interrupts_comm.h>
#include <mdrv_public.h>
#include <mdrv_misc.h>
#include <mdrv_int.h>
#include <mdrv_memory.h>
#include <bsp_shared_ddr.h>
#include <bsp_vic.h>
#include <securec.h>

#define HWADP_MAX_IP_BASE_NUM 256U
#define HWADP_MAX_IRQ_NUM 256U
#define HWADP_MAX_NAME_LEN 127U
#define HWADP_ERR_IRQ_NUM (-1)

#ifndef INT_LVL_MAX
#define INT_LVL_MAX 224
#endif

struct ul_base_addr_info {
    struct list_head node;
    bsp_ip_type_e en_ip_type;
    char ip_name[HWADP_MAX_NAME_LEN + 1];
    void *ul_base_addr;
};

struct ul_int_info {
    struct list_head node;
    bsp_int_type_e en_ip_type;
    char int_name[HWADP_MAX_NAME_LEN + 1];
    unsigned int irq_num;
};

struct ul_base_addr_info *g_ip_addrs[HWADP_MAX_IP_BASE_NUM] = {
    [0 ... HWADP_MAX_IP_BASE_NUM - 1] = NULL,
};

struct ul_int_info *g_irqs[HWADP_MAX_IRQ_NUM] = {
    [0 ... HWADP_MAX_IRQ_NUM - 1] = NULL,
};

bsp_ddr_sect_info_s g_ddr_info[BSP_DDR_SECT_TYPE_BUTTOM] = {
    [0 ... BSP_DDR_SECT_TYPE_BUTTOM - 1] = {BSP_DDR_SECT_TYPE_BUTTOM, BSP_DDR_SECT_ATTR_BUTTOM, NULL, NULL, 0},
};

LIST_HEAD(hwadp_ip_addrs);
LIST_HEAD(hwadp_irq_nums);

void *mdrv_misc_get_ip_baseaddr(bsp_ip_type_e en_ip_type)
{
    if ((en_ip_type < 0) || (en_ip_type >= HWADP_MAX_IP_BASE_NUM)) {
        return NULL;
    }
    /*lint -e650 */
    return g_ip_addrs[(int)en_ip_type] ? g_ip_addrs[(int)en_ip_type]->ul_base_addr : NULL;
    /*lint +e650 */
}

EXPORT_SYMBOL(mdrv_misc_get_ip_baseaddr);

void *mdrv_misc_get_ip_baseaddr_byname(const char *ip_name)
{
    void *ip_base = NULL;
    struct ul_base_addr_info *pos = (struct ul_base_addr_info *)hwadp_ip_addrs.next;

    if (ip_name == NULL) {
        return ip_base;
    }

    list_for_each_entry(pos, &hwadp_ip_addrs, node) {
        if (!strncmp(ip_name, pos->ip_name, (HWADP_MAX_NAME_LEN + 1))) {
            ip_base = pos->ul_base_addr;
            break;
        }
    }
    return ip_base;
}

EXPORT_SYMBOL(mdrv_misc_get_ip_baseaddr_byname);

int mdrv_int_get_num(bsp_int_type_e enIntType)
{
    if ((enIntType < 0) || (enIntType >= HWADP_MAX_IRQ_NUM)) {
        return -1;
    }
    /*lint -e650 */
    return g_irqs[(int)enIntType] ? (int)g_irqs[(int)enIntType]->irq_num : HWADP_ERR_IRQ_NUM;
    /*lint +e650 */
}

EXPORT_SYMBOL(mdrv_int_get_num);

int mdrv_int_get_num_byname(const char *int_name)
{
    int irq_num = HWADP_ERR_IRQ_NUM;
    struct ul_int_info *pos = (struct ul_int_info *)hwadp_irq_nums.next;

    if (int_name == NULL) {
        return irq_num;
    }

    list_for_each_entry(pos, &hwadp_irq_nums, node) {
        if (!strncmp(int_name, pos->int_name, (HWADP_MAX_NAME_LEN + 1))) {
            irq_num = (int)pos->irq_num;
            break;
        }
    }
    return irq_num;
}

EXPORT_SYMBOL(mdrv_int_get_num_byname);

#define VIC_INT_NUM  (32 * 4)
int mdrv_int_enable(int ul_lev)
{
    int retval = MDRV_OK;

    if (ul_lev >= 0 && ul_lev < INT_LVL_MAX) {
        /* linux request_irq includes "enable" */
    } else if (ul_lev >= INT_LVL_MAX && ul_lev < INT_LVL_MAX + VIC_INT_NUM) {
        retval = bsp_vic_enable(ul_lev - INT_LVL_MAX);
    } else {
        retval = MDRV_ERROR;
    }

    return retval;
}

EXPORT_SYMBOL(mdrv_int_enable);

int mdrv_int_disable(int ul_lev)
{
    int retval = MDRV_OK;

    if (ul_lev >= 0 && ul_lev < INT_LVL_MAX) {
    } else if (ul_lev >= INT_LVL_MAX && ul_lev < INT_LVL_MAX + VIC_INT_NUM) {
        retval = bsp_vic_disable(ul_lev - INT_LVL_MAX);
    } else {
        retval = MDRV_ERROR;
    }

    return retval;
}

EXPORT_SYMBOL(mdrv_int_disable);
mdrv_int_handler_t g_irq_handler[INT_LVL_MAX] = {NULL};

irqreturn_t hwadp_int_handler(int irq_no, void *parameter)
{
    if (g_irq_handler[irq_no] != NULL) {
        return (irqreturn_t)g_irq_handler[irq_no](irq_no, parameter);
    } else {
        return IRQ_NONE;
    }
}

int mdrv_int_connect(int level, mdrv_int_handler_t routine, unsigned int parameter)
{
    int retval = MDRV_OK;

    if (level < INT_LVL_MAX && level >= 0) {
        g_irq_handler[level] = routine;
        return request_irq((unsigned int)level, hwadp_int_handler, 0, "other", (void *)(uintptr_t)parameter);
    } else {
        retval = MDRV_ERROR;
    }

    return retval;
}

EXPORT_SYMBOL(mdrv_int_connect);

int mdrv_int_disconnect(int level, unsigned int parameter)
{
    int retval = MDRV_OK;

    if (level < INT_LVL_MAX && level >= 0) {
        free_irq((unsigned int)level, (void *)(uintptr_t)parameter);
    } else if (level >= INT_LVL_MAX && level < INT_LVL_MAX + VIC_INT_NUM) {
        retval = bsp_vic_disconnect(level - INT_LVL_MAX);
    } else {
        retval = MDRV_ERROR;
    }

    return retval;
}

EXPORT_SYMBOL(mdrv_int_disconnect);

int mdrv_get_fix_ddr_addr(bsp_ddr_sect_query_s *pst_sect_query, bsp_ddr_sect_info_s *pst_sect_info)
{
    if ((pst_sect_query == NULL) || (pst_sect_info == NULL)) {
        return MDRV_ERROR;
    }
    if (pst_sect_query->enSectType >= BSP_DDR_SECT_TYPE_BUTTOM) {
        return MDRV_ERROR;
    }
    if (g_ddr_info[pst_sect_query->enSectType].pSectPhysAddr == NULL) {
        return MDRV_ERROR;
    }

    if (memcpy_s((void *)pst_sect_info, sizeof(bsp_ddr_sect_info_s),
        (const void *)(&g_ddr_info[pst_sect_query->enSectType]), sizeof(bsp_ddr_sect_info_s))) {
        return -1;
    }

    return MDRV_OK;
}

EXPORT_SYMBOL(mdrv_get_fix_ddr_addr);

static inline void *drv_ddr_virt_to_phy(const void *ulvaddr)
{
    if ((ulvaddr >= (void *)SHM_BASE_ADDR)
        && (ulvaddr < (void *)(uintptr_t)(SHM_BASE_ADDR + DDR_SHARED_MEM_SIZE))) {
        return (void *)SHD_DDR_V2P((uintptr_t) ulvaddr);
    }
    return NULL;
}

static inline void *drv_axi_virt_to_phy(const void *ulvaddr)
{
    if ((ulvaddr >= g_mem_ctrl.sram_virt_addr)
        && (ulvaddr < (void *)(uintptr_t)((uintptr_t)g_mem_ctrl.sram_virt_addr + g_mem_ctrl.sram_mem_size))) {
        return (void *)((uintptr_t)ulvaddr -
            (uintptr_t)g_mem_ctrl.sram_virt_addr + (uintptr_t)g_mem_ctrl.sram_phy_addr);
    }
    return NULL;
}

void *mdrv_virt_to_phy(mem_mode_type_e mode, const void *lpaddr)
{
    void *phy_addr = NULL;

    switch (mode) {
        case MEM_DDR_MODE:
            phy_addr = (void *)drv_ddr_virt_to_phy(lpaddr);
            break;
        case MEM_AXI_MODE:
            phy_addr = (void *)drv_axi_virt_to_phy(lpaddr);
            break;
        default:
            break;
    }

    return phy_addr;
}

EXPORT_SYMBOL(mdrv_virt_to_phy);

static inline void *drv_ddr_phy_to_virt(const void *ulpaddr)
{
    if ((ulpaddr >= (void *)g_mem_ctrl.sddr_phy_addr) &&
        (ulpaddr < (void *)(uintptr_t)((uintptr_t)g_mem_ctrl.sddr_phy_addr + g_mem_ctrl.sddr_mem_size))) {
        return (void *)SHD_DDR_P2V((uintptr_t)ulpaddr);
    }
    return NULL;
}

static inline void *drv_axi_phy_to_virt(const void *ulpaddr)
{
    if ((ulpaddr >= g_mem_ctrl.sram_phy_addr) &&
        (ulpaddr < (void *)(uintptr_t)((uintptr_t)g_mem_ctrl.sram_phy_addr + g_mem_ctrl.sram_mem_size))) {
        return (void *)(uintptr_t)((uintptr_t)ulpaddr - (uintptr_t)g_mem_ctrl.sram_phy_addr
            + (uintptr_t)g_mem_ctrl.sram_virt_addr);
    }
    return NULL;
}

void *mdrv_phy_to_virt(mem_mode_type_e mode, const void *pmem)
{
    void *virt_addr = NULL;

    switch (mode) {
        case MEM_DDR_MODE:
            virt_addr = (void *)drv_ddr_phy_to_virt(pmem);
            break;
        case MEM_AXI_MODE:
            virt_addr = (void *)drv_axi_phy_to_virt(pmem);
            break;
        default:
            break;
    }

    return virt_addr;
}

EXPORT_SYMBOL(mdrv_phy_to_virt);

int bsp_hwadp_register_base_addr(bsp_ip_type_e type, const char *ip_name, void *base_addr)
{
    struct ul_base_addr_info *ip_base = NULL;
    void *base = NULL;

    base = mdrv_misc_get_ip_baseaddr_byname(ip_name);
    if (base != NULL) {
        return -1;
    }

    if (strlen(ip_name) > HWADP_MAX_NAME_LEN) {
        return -1;
    }

    ip_base = (struct ul_base_addr_info *)kzalloc(sizeof(struct ul_base_addr_info), GFP_KERNEL);
    if (ip_base == NULL) {
        return -1;
    }

    ip_base->en_ip_type = type;
    ip_base->ul_base_addr = base_addr;
    if (strncpy_s((void *)ip_base->ip_name, HWADP_MAX_NAME_LEN + 1, ip_name, HWADP_MAX_NAME_LEN)) {
        kfree(ip_base);
        return -1;
    }

    list_add(&ip_base->node, &hwadp_ip_addrs);

    if ((type < 0) || (type >= HWADP_MAX_IP_BASE_NUM)) {
        return 0;
    }
    /*lint -e650 */
    if (!g_ip_addrs[(int)type]) {
        g_ip_addrs[(int)type] = ip_base;
    } /*lint +e650 */

    return 0; /*lint !e593 */
}

int bsp_hwadp_register_irq_num(bsp_int_type_e type, const char *irq_name, unsigned int irq_num)
{
    struct ul_int_info *irq_info = NULL;
    int int_num;

    int_num = mdrv_int_get_num_byname(irq_name);
    if (HWADP_ERR_IRQ_NUM != int_num) {
        return -1;
    }

    if (strlen(irq_name) > HWADP_MAX_NAME_LEN) {
        return -1;
    }

    irq_info = (struct ul_int_info *)kzalloc(sizeof(struct ul_int_info), GFP_KERNEL);
    if (irq_info == NULL) {
        return -1;
    }

    irq_info->en_ip_type = type;
    irq_info->irq_num = irq_num;
    if (strncpy_s((void *)irq_info->int_name, HWADP_MAX_NAME_LEN + 1, irq_name, HWADP_MAX_NAME_LEN)) {
        kfree(irq_info);
        return -1;
    }
    list_add(&irq_info->node, &hwadp_irq_nums);

    if ((type < 0) || (type >= HWADP_MAX_IRQ_NUM)) {
        return 0;
    }
    /*lint -e650 */
    if (!g_irqs[(int)type]) {
        g_irqs[(int)type] = irq_info;
    } /*lint +e650 */

    return 0;  /*lint !e593 */
}

int bsp_hwadp_register_ddr(bsp_ddr_sect_type_e type, bsp_ddr_sect_attr_e attr,
                           void *ddr_virt, void *ddr_phy, unsigned int size)
{
    if (type >= BSP_DDR_SECT_TYPE_BUTTOM) {
        return MDRV_ERROR;
    }

    g_ddr_info[type].enSectType = type;
    g_ddr_info[type].enSectAttr = attr;
    g_ddr_info[type].pSectVirtAddr = ddr_virt;
    g_ddr_info[type].pSectPhysAddr = ddr_phy;
    g_ddr_info[type].ulSectSize = size;

    return MDRV_OK;
}
