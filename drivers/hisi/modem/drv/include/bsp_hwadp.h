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
#ifndef    _BSP_HWADP_H_
#define    _BSP_HWADP_H_

#include <mdrv_misc.h>
#include <mdrv_int.h>
#include <mdrv_memory.h>

#ifdef __cplusplus /* __cplusplus */
extern "C"
{
#endif /* __cplusplus */

#define HWADP_OK    (0)
#define HWADP_ERROR (-1)

struct HI_HWADP_MEMMAP {
    void* addr_virt;
    void* addr_phy;
    unsigned int mem_size;
};

#if defined(CONFIG_HWADP) || defined(CONFIG_HW_IP_BASE_ADDR)
extern int hwadp_init(void);
extern int bsp_hwadp_register_base_addr(bsp_ip_type_e type, const char *name, void *base_addr);
extern int bsp_hwadp_register_irq_num(bsp_int_type_e type, const char *name, unsigned int irq_num);
extern int bsp_hwadp_register_ddr(bsp_ddr_sect_type_e type, bsp_ddr_sect_attr_e attr, void *ddr_virt,
    void *ddr_phy, unsigned int size);
#else
static inline int bsp_hwadp_register_base_addr(bsp_ip_type_e type, const char *name, void *base_addr)
{
    return 0;
}
static inline int bsp_hwadp_register_irq_num(bsp_int_type_e type, const char *name, unsigned int irq_num)
{
    return 0;
}
static inline int bsp_hwadp_register_ddr(bsp_ddr_sect_type_e type, bsp_ddr_sect_attr_e attr,
    void *ddr_virt, void *ddr_phy, unsigned int size)
{
    return 0;
}
#endif

#ifdef __cplusplus
}
#endif
#endif    /* End of HWADP_H */

