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
#ifndef HI_MAA_V2_H
#define HI_MAA_V2_H

#define MAA_DDR_RLS_INT_MASK BIT(16)
#define MAA_MDM_DDR_RLS_INT_MASK BIT(18)
#define MAA_RLS_FULL_WR_INT_MASK BIT(17)
#define MAA_MDM_RLS_FULL_WR_INT_MASK BIT(19)
#define MAA_SAFE_CHK_INT_MASK BIT(20)

#define MAA_GLB_CTRL (0x0)             /* 全局控制寄存器 */
#define MAA_AXI_BUS_CFG (0x4)          /* AXI_MST OS配置寄存器 */
#define MAA_VERSION (0x8)              /* 版本寄存器 */
#define MAA_LP_EN (0xC)                /* 低功耗使能寄存器 */
#define MAA_LP_STAT (0x10)             /* 低功耗状态寄存器 */
#define MAA_AUTOCLK_RESP_BYPASS (0x14) /* 低功耗状态寄存器 */
#define MAA_OPIPE_INT_MSK (0x18)       /* MAA中断屏蔽寄存器。连给ACORE */
#define MAA_OPIPE_INT_STAT (0x1C)      /* MAA中断屏蔽寄存器。连给ACORE */
#define MAA_OPIPE_INT_RAW (0x20)       /* MAA中断屏蔽寄存器。连给ACORE */
#define MAA_INT_MSK_DISPATCH (0x24)    /* MAA中断屏蔽寄存器。连给ACORE */
#define MAA_INT_STAT_DISPATCH (0x28)   /* MAA中断屏蔽寄存器。连给ACORE */
#define MAA_IPIPE_INT_RAW0 (0x30)      /* MAA中断原始状态寄存器。连给ACORE */
#define MAA_IPIPE_INT_MSK0 (0x34)      /* MAA中断原始状态寄存器。连给ACORE */
#define MAA_IPIPE_INT_STAT0 (0x38)     /* MAA中断原始状态寄存器。连给ACORE */
#define MAA_IPIPE_INT_RAW1 (0x3C)      /* MAA中断原始状态寄存器。连给ACORE */
#define MAA_IPIPE_INT_MSK1 (0x40)      /* MAA中断原始状态寄存器。连给ACORE */
#define MAA_IPIPE_INT_STAT1 (0x44)     /* MAA中断原始状态寄存器。连给ACORE */
#define MAA_IPIPE_INT_RAW2 (0x48)      /* MAA中断原始状态寄存器。连给L2HAC */
#define MAA_IPIPE_INT_MSK2 (0x4C)      /* MAA中断原始状态寄存器。连给L2HAC */
#define MAA_IPIPE_INT_STAT2 (0x50)     /* MAA中断原始状态寄存器。连给L2HAC */
#define MAA_IPIPE_INT_RAW3 (0x54)      /* MAA中断原始状态寄存器。连给4G CCORE */
#define MAA_IPIPE_INT_MSK3 (0x58)      /* MAA中断原始状态寄存器。连给4G CCORE */
#define MAA_IPIPE_INT_STAT3 (0x5C)     /* MAA中断原始状态寄存器。连给4G CCORE */
#define MAA_IPIPE_INT_RAW4 (0x60)      /* MAA中断原始状态寄存器。连给5G CCORE */
#define MAA_IPIPE_INT_MSK4 (0x64)      /* MAA中断原始状态寄存器。连给5G CCORE */
#define MAA_IPIPE_INT_STAT4 (0x68)     /* MAA中断原始状态寄存器。连给5G CCORE */
#define MAA_IPIPE_INT_RAW5 (0x6C)      /* MAA中断原始状态寄存器。连给HL1C */
#define MAA_IPIPE_INT_MSK5 (0x70)      /* MAA中断原始状态寄存器。连给HL1C */
#define MAA_IPIPE_INT_STAT5 (0x74)     /* MAA中断原始状态寄存器。连给HL1C */
#define MAA_IPIPE_INT_RAW6 (0x78)      /* MAA中断原始状态寄存器。连给HL1C */
#define MAA_IPIPE_INT_MSK6 (0x7C)      /* MAA中断原始状态寄存器。连给HL1C */
#define MAA_IPIPE_INT_STAT6 (0x80)     /* MAA中断原始状态寄存器。连给HL1C */
#define MAA_IPIPE_INT_RAW7 (0x84)      /* MAA中断原始状态寄存器。连给HL1C */
#define MAA_IPIPE_INT_MSK7 (0x88)      /* MAA中断原始状态寄存器。连给HL1C */
#define MAA_IPIPE_INT_STAT7 (0x8C)     /* MAA中断原始状态寄存器。连给HL1C */
#define MAA_HAC_GIF_128SPLIT (0x180)   /* MAA HAC_GIF通道读写128拆分控制 */
#define MAA_HAC_GIF_MAX_LEN (0x184)    /* MAA HAC_GIF通道DMA MAX_LEN */
#define MAA_HAC_GIF_PRIOR (0x188)      /* MAA HAC_GIF通道优先级 */
#define MAA_TIMER_EN (0x190)           /* MAA使能fifo计时timer */
#define MAA_TIMER_WATCHCNT (0x194)     /* MAA 配置TIMER计数上限值 */
#define MAA_ALLOC_ADDR_L(opipes, src_ptrs) (0x200 + 0x80 * (opipes) + 0x8 * (src_ptrs))
#define MAA_ALLOC_ADDR_H(opipes, src_ptrs) (0x204 + 0x80 * (opipes) + 0x8 * (src_ptrs))
#define MAA_RLS_POOL_ADDR_L (0x800)   /* 释放目的内存池基地址低位寄存器 */
#define MAA_RLS_POOL_ADDR_H (0x804)   /* 释放目的内存池基地址高位寄存器 */
#define MAA_RLS_POOL_DEPTH (0x808)    /* 释放目的内存池深度寄存器 */
#define MAA_RLS_POOL_WPTR (0x80C)     /* 释放目的内存池写指针寄存器 */
#define MAA_RLS_POOL_RPTR (0x810)     /* 释放目的内存池读指针寄存器 */
#define MAA_RLS_POOL_UP_THRH (0x814)  /* 释放目的内存池上水线寄存器 */
#define MAA_RLS1_POOL_ADDR_L (0x820)  /* 释放目的内存池基地址低位寄存器 */
#define MAA_RLS1_POOL_ADDR_H (0x824)  /* 释放目的内存池基地址高位寄存器 */
#define MAA_RLS1POOL_DEPTH (0x828)    /* 释放目的内存池深度寄存器 */
#define MAA_RLS1_POOL_WPTR (0x82C)    /* 释放目的内存池写指针寄存器 */
#define MAA_RLS1_POOL_RPTR (0x830)    /* 释放目的内存池读指针寄存器 */
#define MAA_RLS1_POOL_UP_THRH (0x834) /* 释放目的内存池上水线寄存器 */

#define MAA_ACORE_ALLOC_CNT(levels) (0x900 + 0x80 * (levels))
#define MAA_ALLOC_FIFO_CNT(levels) (0x904 + 0x80 * (levels))
#define MAA_ALLOC_MAA_CNT(levels) (0x90C + 0x80 * (levels))
#define MAA_ALLOC_DESTN_CNT(destns, levels) (0x910 + 0x4 * (destns) + 0x80 * (levels))
#define MAA_DESTN_ALLOC_CNT(destns, levels) (0x930 + 0x4 * (destns) + 0x80 * (levels))
#define MAA_RLS_CNT(levels) (0x960 + 0x80 * (levels))

#define MAA_RLS0_ABANDON_CNT (0xEF8)       /* RELEASE POOL满写时丢弃的指针个数 */
#define MAA_RLS1_ABANDON_CNT (0xEFC)       /* RELEASE POOL满写时丢弃的指针个数 */
#define MAA_CNT_CLK_EN (0xF00)             /* 维测计数器时钟门控使能寄存器 */
#define MAA_OPIPE_STAT (0xF04)             /* OPIPE状态寄存器 */
#define MAA_IPIPE_STAT0 (0xF08)            /* IPIPE状态寄存器 */
#define MAA_IPIPE_STAT1 (0xF0C)            /* IPIPE状态寄存器 */
#define MAA_IPIPE_STAT2 (0xF10)            /* IPIPE状态寄存器 */
#define MAA_AP_MDM_OPIPE_NUM (0xF20)       /* 内存粒度配置寄存器。 */
#define MAA_HAC_BP_DBG (0x1000)            /* MAA BP debug寄存器 */
#define MAA_HAC_DATA_DBG (0x1004)          /* MAA AXI_MST DATA debug寄存器 */
#define MAA_HAC_PUSH_DBG (0x1008)          /* MAA AXI_MST PUSH debug寄存器 */
#define MAA_HAC_DBG (0x100C)               /* MAA AXI_MST debug寄存器 */
#define MAA_GS_DBG (0x1010)                /* MAA AXI_GS debug寄存器 */
#define MAA_HAC_BP_INVALID_DATA_L (0x1018) /* HAC_BP_INVALID_DATA_L寄存器 */
#define MAA_HAC_BP_INVALID_DATA_H (0x101C) /* HAC_BP_INVALID_DATA_H寄存器 */

#define MAA_ALLOC_ADDR_RD_L(opipes, src_ptrs) (0x1100 + 0x100 * (opipes) + 0x8 * (src_ptrs))
#define MAA_ALLOC_ADDR_RD_H(opipes, src_ptrs) (0x1104 + 0x100 * (opipes) + 0x8 * (src_ptrs))

#define MAA_OPIPE_SECCTRL (0x3000)  /* opipe安全配置寄存器 */
#define MAA_IPIPE_SECCTRL0 (0x3008) /* ipipe安全配置寄存器 */
#define MAA_IPIPE_SECCTRL1 (0x300C) /* ipipe安全配置寄存器 */
#define MAA_IPIPE_SECCTRL2 (0x3010) /* ipipe安全配置寄存器 */
#define MAA_IPIPE_SECCTRL3 (0x3014) /* ipipe安全配置寄存器 */
#define MAA_IPIPE_SECCTRL4 (0x3018) /* ipipe安全配置寄存器 */

#define MAA_ALLOC_ADDR_STR_L(sec_rgns) (0x3020 + 0x10 * (sec_rgns))
#define MAA_ALLOC_ADDR_STR_H(sec_rgns) (0x3024 + 0x10 * (sec_rgns))
#define MAA_ALLOC_ADDR_END_L(sec_rgns) (0x3028 + 0x10 * (sec_rgns))
#define MAA_ALLOC_ADDR_END_H(sec_rgns) (0x302C + 0x10 * (sec_rgns))

#define MAA_ALLOC_ADDR_PT_CTRL (0x3100) /* 地址指针安全范围控制寄存器。 */
#define MAA_PTR_SIZE0 (0x3104)          /* 内存粒度配置寄存器。 */
#define MAA_PTR_SIZE1 (0x3108)          /* 内存粒度配置寄存器。 */
#define MAA_PTR_SIZE2 (0x310C)          /* 内存粒度配置寄存器。 */
#define MAA_PTR_SIZE3 (0x3110)          /* 内存粒度配置寄存器。 */
#define MAA_PTR_SIZE4 (0x3114)          /* 内存粒度配置寄存器。 */
#define MAA_PTR_SIZE5 (0x3118)          /* 内存粒度配置寄存器。 */
#define MAA_PTR_SIZE6 (0x311C)          /* 内存粒度配置寄存器。 */
#define MAA_PTR_SIZE7 (0x3120)          /* 内存粒度配置寄存器。 */
#define MAA_PTR_SIZE8 (0x3124)          /* 内存粒度配置寄存器。 */
#define MAA_PTR_SIZE9 (0x3128)          /* 内存粒度配置寄存器。 */
#define MAA_AP_MID (0x3200)             /* OPIPE的MID寄存器 */
#define MAA_MDM_MID (0x3204)            /* OPIPE的MID寄存器 */
#define MAA_IPIPE_MID0 (0x3208)         /* IPIPE的MID寄存器 */
#define MAA_IPIPE_MID1 (0x320C)         /* IPIPE的MID寄存器 */

#define MAA_OPIPE_BASE_ADDR_L(opipes) (0x4000 + 0x40 * (opipes))
#define MAA_OPIPE_BASE_ADDR_H(opipes) (0x4004 + 0x40 * (opipes))
#define MAA_OPIPE_DEPTH(opipes) (0x4008 + 0x40 * (opipes))
#define MAA_OPIPE_UP_THRH(opipes) (0x400c + 0x40 * (opipes))
#define MAA_OPIPE_DN_THRH(opipes) (0x4010 + 0x40 * (opipes))
#define MAA_OPIPE_RPTR(opipes) (0x4014 + 0x40 * (opipes))
#define MAA_OPIPE_WPTR(opipes) (0x4018 + 0x40 * (opipes))
#define MAA_OPIPE_SPACE(opipes) (0x401c + 0x40 * (opipes))
#define MAA_OPIPE_EN(opipes) (0x4020 + 0x40 * (opipes))
#define MAA_OPIPE_CFG_DONE(opipes) (0x4024 + 0x40 * (opipes))
#define MAA_OPIPE_DBG1(opipes) (0x4028 + 0x40 * (opipes))
#define MAA_OPIPE_DBG0(opipes) (0x402c + 0x40 * (opipes))
#define MAA_IPIPE_BASE_ADDR_L(ipipes) (0x6000 + 0x40 * (ipipes))
#define MAA_IPIPE_BASE_ADDR_H(ipipes) (0x6004 + 0x40 * (ipipes))
#define MAA_IPIPE_DEPTH(ipipes) (0x6008 + 0x40 * (ipipes))
#define MAA_IPIPE_WPTR(ipipes) (0x600C + 0x40 * (ipipes))
#define MAA_IPIPE_RPTR(ipipes) (0x6010 + 0x40 * (ipipes))
#define MAA_IPIPE_PTR_ADDR_L(ipipes) (0x6018 + 0x40 * (ipipes))
#define MAA_IPIPE_PTR_ADDR_H(ipipes) (0x601C + 0x40 * (ipipes))
#define MAA_IPIPE_UP_THRH(ipipes) (0x6020 + 0x40 * (ipipes))
#define MAA_IPIPE_DN_THRH(ipipes) (0x6024 + 0x40 * (ipipes))
#define MAA_IPIPE_CTRL(ipipes) (0x6028 + 0x40 * (ipipes))
#define MAA_IPIPE_DBG1(ipipes) (0x602C + 0x40 * (ipipes))
#endif