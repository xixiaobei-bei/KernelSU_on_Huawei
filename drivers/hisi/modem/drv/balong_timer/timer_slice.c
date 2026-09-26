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

/*lint --e{537}*/
#include <linux/module.h>
#include <osl_types.h>
#include <bsp_dt.h>
#include <bsp_version.h>
#include <bsp_hardtimer.h>
#include <bsp_slice.h>

#undef THIS_MODU
#define THIS_MODU mod_hardtimer

struct timerslice_control {
    void *timerslice_base_addr;
    void *timerslice_base_addr_phy; /* 时间戳实地址 */
    void *timerslice_hrt_base_addr; /* udelay timer stamp addr */
    u32 hrt_slice_is_increase;
    u32 slice_is_increase;
    u32 hrt_slice_offset[0x2]; /* udelay timer stamp 偏移 */
    u32 slice_offset[2];     /* 32k slice offset */
    u32 hrt_slice_clock_freq;
    u32 slice_clock_freq;
    u64 slice_judgetime;
    u32 version_id;
    u32 is_inited_flag;
};

struct timerslice_control g_slice_ctrl = {
    NULL, NULL, NULL, 0, 0, { 0, 0 }, { 0, 0 }, 0, 0, 0x50000, /* default 10s */
    0,    0,
};

void slice_set_judgetime(u64 time_s)
{
    if (g_slice_ctrl.is_inited_flag) {
        g_slice_ctrl.slice_judgetime = g_slice_ctrl.slice_clock_freq * time_s;
        return;
    } else {
        return;
    }
}

u32 bsp_get_slice_value(void)
{
    if (g_slice_ctrl.is_inited_flag) {
        u32 slice = readl((void *)((uintptr_t)g_slice_ctrl.timerslice_base_addr + g_slice_ctrl.slice_offset[0]));
        if (g_slice_ctrl.slice_is_increase) {
            return slice;
        } else {
            return (U32_MAX_VALUE - slice);
        }
    } else {
        return 0;
    }
}

/*
 * 函数  : bbp_get_curtime
 * 功能  : 被PS调用，用来获取系统精确时间
 */
static void _bsp_slice_getcurtime(u64 *pcurtime)
{
    /*lint -save -e958*/
    u64 timer_value[0x4];
    /*lint -restore*/

    timer_value[0] = (u64)readl((void *)((uintptr_t)g_slice_ctrl.timerslice_base_addr + g_slice_ctrl.slice_offset[0]));
    timer_value[1] = (u64)readl((void *)((uintptr_t)g_slice_ctrl.timerslice_base_addr + g_slice_ctrl.slice_offset[1]));
    if (timer_value[0] >= (U32_MAX_VALUE - g_slice_ctrl.slice_judgetime)) {
        timer_value[0x2] = (u64)readl(
            (void *)((uintptr_t)g_slice_ctrl.timerslice_base_addr + g_slice_ctrl.slice_offset[0]));
        timer_value[0x3] = (u64)readl(
            (void *)((uintptr_t)g_slice_ctrl.timerslice_base_addr + g_slice_ctrl.slice_offset[1]));

        if (timer_value[0x2] < timer_value[0]) {
            /* When high 32 bit and 32 bit low overflow at the same time,
               pcurtime is correct，timer_value[3] - 1 = 0xffffffff */
            (*pcurtime) = ((timer_value[0x3] - 1) << U32_BIT_MAX) | timer_value[0];
        } else {
            (*pcurtime) = (timer_value[1] << U32_BIT_MAX) | timer_value[0];
        }
    } else {
        (*pcurtime) = (timer_value[1] << U32_BIT_MAX) | timer_value[0];
    }
}

u32 bsp_slice_getcurtime(u64 *pcurtime)
{
    if (pcurtime != NULL && g_slice_ctrl.is_inited_flag) {
        if (!g_slice_ctrl.slice_is_increase) {
            *pcurtime = (u64)bsp_get_slice_value();
        } else {
            _bsp_slice_getcurtime(pcurtime);
        }
    }

    return 0;
}

void bsp_ab_sync_slice_getcurtime(u64 *pcurtime)
{
    u64 slice_get = 0;
    u32 ret;

    if (pcurtime == NULL) {
        return;
    }
    ret = bsp_slice_getcurtime(&slice_get);
    if (ret) {
        return;
    }
    *pcurtime = slice_get;
}

u32 bsp_slice_getcurtime_hrt(u64 *pcurtime)
{
    /*lint -save -e958*/
    u64 timer_value[0x4];
    /*lint -restore*/

    if (pcurtime != NULL && g_slice_ctrl.is_inited_flag) {
        timer_value[0] = (u64)readl(
            (void *)((uintptr_t)g_slice_ctrl.timerslice_hrt_base_addr + g_slice_ctrl.hrt_slice_offset[0]));
        timer_value[1] = (u64)readl(
            (void *)((uintptr_t)g_slice_ctrl.timerslice_hrt_base_addr + g_slice_ctrl.hrt_slice_offset[1]));
        if (timer_value[0] >= (U32_MAX_VALUE - g_slice_ctrl.slice_judgetime)) {
            timer_value[0x2] = (u64)readl(
                (void *)((uintptr_t)g_slice_ctrl.timerslice_hrt_base_addr + g_slice_ctrl.hrt_slice_offset[0]));
            timer_value[0x3] = (u64)readl(
                (void *)((uintptr_t)g_slice_ctrl.timerslice_hrt_base_addr + g_slice_ctrl.hrt_slice_offset[1]));

            if (timer_value[0x2] < timer_value[0]) {
                /* When high 32 bit and 32 bit low overflow at the same time,
                   pcurtime is correct，timer_value[3] - 1 = 0xffffffff */
                (*pcurtime) = ((timer_value[0x3] - 1) << U32_BIT_MAX) | timer_value[0];
            } else {
                (*pcurtime) = (timer_value[1] << U32_BIT_MAX) | timer_value[0];
            }
        } else {
            (*pcurtime) = (timer_value[1] << U32_BIT_MAX) | timer_value[0];
        }
    }
    return 0;
}

u32 bsp_get_slice_value_hrt(void)
{
    if (g_slice_ctrl.is_inited_flag) {
        u32 slice = readl(
            (void *)((uintptr_t)g_slice_ctrl.timerslice_hrt_base_addr + g_slice_ctrl.hrt_slice_offset[0]));
        if (g_slice_ctrl.hrt_slice_is_increase) {
            return slice;
        } else {
            return (U32_MAX_VALUE - slice);
        }
    } else {
        return 0;
    }
}

u32 bsp_get_elapse_ms(void)
{
    u64 tmp;
    unsigned long timer_get;

    if (g_slice_ctrl.is_inited_flag) {
        timer_get = bsp_get_slice_value();
        tmp = (u64)(timer_get & U32_MAX_VALUE);
        tmp = tmp * SLICE_CONVERT_DELTA;
        tmp = div_u64(tmp, g_slice_ctrl.slice_clock_freq);
        return (u32)tmp;
    } else {
        return 0;
    }
}

void *bsp_get_stamp_addr(void)
{
    if (g_slice_ctrl.is_inited_flag) {
        return (void *)((uintptr_t)g_slice_ctrl.timerslice_base_addr + g_slice_ctrl.slice_offset[0]);
    } else {
        return NULL;
    }
}

void *bsp_get_stamp_addr_phy(void)
{
    if (g_slice_ctrl.is_inited_flag) {
        return (void *)((uintptr_t)g_slice_ctrl.timerslice_base_addr_phy + g_slice_ctrl.slice_offset[0]);
    } else {
        return NULL;
    }
}

u32 bsp_get_slice_freq(void)
{
    return g_slice_ctrl.slice_clock_freq;
}

u32 bsp_get_hrtimer_freq(void)
{
    return g_slice_ctrl.hrt_slice_clock_freq;
}

int slice_info(device_node_s *node)
{
    void *vaddr = NULL;
    const __be32 *regaddr_p = NULL;
    u64 regaddr64;
    int na;
    u32 ret;

    regaddr_p = bsp_dt_get_property(node, "reg", NULL);
    if (regaddr_p == NULL) {
        hardtimer_print_error("timer_stamp get_address failed.\r\n");
        return BSP_ERROR;
    }
    na = bsp_dt_n_addr_cells(node);
    regaddr64 = bsp_dt_read_number(regaddr_p, na);

    g_slice_ctrl.timerslice_base_addr_phy = (void *)(uintptr_t)regaddr64;
    vaddr = bsp_dt_iomap(node, 0);
    if (vaddr == NULL) {
        hardtimer_print_error("timer slice iomap failed.\r\n");
        return BSP_ERROR;
    }
    g_slice_ctrl.timerslice_base_addr = vaddr;

    ret = bsp_dt_property_read_u32(node, "increase_count_flag", &g_slice_ctrl.slice_is_increase);
    /* 32k slice使用2个32位寄存器计数 */
    ret += bsp_dt_property_read_u32_array(node, "offset", g_slice_ctrl.slice_offset, 2);
    ret += bsp_dt_property_read_u32(node, "clock-frequency", &g_slice_ctrl.slice_clock_freq);
    if (ret) {
        hardtimer_print_error("timer slice property get failed.\r\n");
        return BSP_ERROR;
    }
    return BSP_OK;
}

int slice_init(void)
{
    device_node_s *node = NULL;

    const bsp_version_info_s *version = bsp_get_version_info();

    if (unlikely(version == NULL)) {
        hardtimer_print_error("version get failed.\r\n");
        return BSP_ERROR;
    }
    /*
     * p532 fpga 与p532 asic读取同一套dts，所以通过版本号区分slice来源;
     * porting timer slice also come from timer
     */
    if ((version->board_type == BSP_BOARD_TYPE_SOC) || (version->board_type == BSP_BOARD_TYPE_SFT) ||
        (version->cses_type == TYPE_ESL)) {
        node = bsp_dt_find_compatible_node(NULL, NULL, "hisilicon,timer_stamp");
    } else if (version->cses_type == TYPE_ESL_EMU) { /* ESL+EMU */
        node = bsp_dt_find_compatible_node(NULL, NULL, "hisilicon,timer_stamp_hybrid");
    } else {
        node = bsp_dt_find_compatible_node(NULL, NULL, "hisilicon,timer_slice");
    }
    if (node == NULL) {
        hardtimer_print_error("timer slice find_compatible_node failed.\r\n");
        return BSP_ERROR;
    }
    if (!bsp_dt_device_is_available(node)) {
        hardtimer_print_error("timer slice dts status is not ok.\n");
        return BSP_ERROR;
    }
    if (slice_info(node) == BSP_ERROR) {
        return BSP_ERROR;
    }

    return BSP_OK;
}

int hrt_slice_init(void)
{
    device_node_s *node = NULL;
    char *hrttimer_slice = "hisilicon,hrttimer_slice";
    void *iomap_node = NULL;
    s32 ret;

    node = bsp_dt_find_compatible_node(NULL, NULL, hrttimer_slice);
    if (node == NULL) {
        hardtimer_print_error("timer_hrtslice find_compatible_node failed.\n");
        return BSP_ERROR;
    }
    if (!bsp_dt_device_is_available(node)) {
        hardtimer_print_error("timer hrtslice dts status is not ok.\n");
        return BSP_ERROR;
    }
    /* 内存映射，获得基址 */
    iomap_node = bsp_dt_iomap(node, 0);
    if (iomap_node == NULL) {
        hardtimer_print_error("hrttimer_app iomap failed.\n");
        return BSP_ERROR;
    }
    g_slice_ctrl.timerslice_hrt_base_addr = iomap_node;
    ret = bsp_dt_property_read_u32(node, "increase_count_flag", &g_slice_ctrl.hrt_slice_is_increase);
    if (ret) {
        hardtimer_print_error("timer_hrtslice get increase_count_flag failed.\n");
        return BSP_ERROR;
    }
    ret = bsp_dt_property_read_u32(node, "clock-frequency", &g_slice_ctrl.hrt_slice_clock_freq);
    if (ret) {
        hardtimer_print_error("timer_hrtslice get frequency failed.\n");
        return BSP_ERROR;
    }
    ret = bsp_dt_property_read_u32_array(node, "offset", g_slice_ctrl.hrt_slice_offset, 0x2);
    if (ret) {
        hardtimer_print_error("timer_hrtslice get offset failed.\n");
        return BSP_ERROR;
    }

    return BSP_OK;
}

int bsp_slice_init(void)
{
    int ret;

    ret = slice_init();
    if (ret) {
        hardtimer_print_error("slice_init failed.\n");
        return BSP_ERROR;
    }
    ret = hrt_slice_init();
    if (ret) {
        hardtimer_print_error(" hrt_slice_initfailed.\n");
        return BSP_ERROR;
    }
    g_slice_ctrl.is_inited_flag = 1;
    hardtimer_print_error("ok.\n");
    return BSP_OK;
}

EXPORT_SYMBOL(bsp_get_slice_value);
EXPORT_SYMBOL(bsp_get_slice_value_hrt);
EXPORT_SYMBOL(bsp_get_elapse_ms);
EXPORT_SYMBOL(bsp_get_stamp_addr);
EXPORT_SYMBOL(bsp_get_stamp_addr_phy);
EXPORT_SYMBOL(bsp_slice_getcurtime);
EXPORT_SYMBOL(bsp_slice_getcurtime_hrt);
EXPORT_SYMBOL(slice_set_judgetime);
#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
core_initcall(bsp_slice_init);
#endif
