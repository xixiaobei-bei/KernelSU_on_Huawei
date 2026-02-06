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

#ifndef __BSP_OM_ENUM_H__
#define __BSP_OM_ENUM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* field id定义只允许添加，不允许删除，枚举定义删除之后，id值需要保留，新添加id需要跳过删除id */
/* 后续新增id放到DYNAMIC区域，添加AP新的field，放到DUMP_AP_FIELD_END之前，添加CP新的field，放到DUMP_CP_FIELD_END之前 */
/* 添加M3新的field，放到DUMP_M3_FIELD_END之前 */
typedef enum{
    /*AP FIELD IDs*/
    DUMP_MODEMAP_FIELD_BEGIN     = (0x010F0000),
     /* FIX */
    DUMP_MODEMAP_BASE_INFO       = (DUMP_MODEMAP_FIELD_BEGIN),
    DUMP_MODEMAP_TASK_SWITCH     = (0x010F0001),
    DUMP_MODEMAP_INTLOCK         = (0x010F0002),
    DUMP_MODEMAP_TASK_STACK      = (0x010F0003),
    DUMP_MODEMAP_INT_STACK       = (0x010F0004),
    DUMP_MODEMAP_ALLTASK         = (0x010F0005),
    DUMP_MODEMAP_ALLTASK_TCB     = (0x010F0006),
    DUMP_MODEMAP_PRINT           = (0x010F0007),
    DUMP_MODEMAP_REGS            = (0x010F0008),
    DUMP_MODEMAP_CPUVIEW         = (0x010F0009),
    DUMP_MODEMAP_USER_DATA       = (0x010F000B),
    DUMP_MODEMAP_LP              = (0x01100003),
    DUMP_MODEMAP_LP_BUSERROR     = (0x01100004),
    DUMP_MODEMAP_ICC             = (0x01100005),
    DUMP_MODEMAP_KERNEL_LOG      = (0x01100007),
    DUMP_MODEMAP_PM_OM           = (0x01100008),
    DUMP_MODEMAP_NV              = (0x01100009),
    DUMP_MODEMAP_CPUFREQ         = (0x0110000a),
    DUMP_MODEMAP_BASE_INFO_SMP   = (0x0110000b),
    DUMP_MODEMAP_CPRESET         = (0x0110000c),
    DUMP_MODEMAP_IPF             = (0x0110000d),
    DUMP_MODEMAP_COLD_PATCH      = (0x0110000e),
    DUMP_MODEMAP_KO_DUMP         = (0x0110000f),

    DUMP_MODEMAP_FIELD_END       = (0x01ffffff),

    /*CP FIELD IDs*/
    DUMP_LRCCPU_FIELD_BEGIN          = (0x02000000),
    DUMP_LRCCPU_BASE_INFO            = DUMP_LRCCPU_FIELD_BEGIN,
    DUMP_LRCCPU_TASK_SWITCH          = (0x02000001),
    DUMP_LRCCPU_INTLOCK              = (0x02000002),
    DUMP_LRCCPU_TASK_STACK           = (0x02000003),
    DUMP_LRCCPU_SYSTEM_STACK         = (0x02000004),
    DUMP_LRCCPU_ALLTASK_NAME         = (0x02000005),
    DUMP_LRCCPU_ALLTASK_TCB          = (0x02000006),
    DUMP_LRCCPU_PRINT                = (0x02000007),
    DUMP_LRCCPU_DMESG                = (0x02000008),
    DUMP_LRCCPU_REGS                 = (0x02000009),
    DUMP_LRCCPU_CPUVIEW              = (0x0200000A),
    DUMP_LRCCPU_USER_DATA            = (0x0200000C),
    DUMP_LRCCPU_BBE16_TCM            = (0x0200000D),
    DUMP_LRCCPU_DRX                  = (0x0200000E),
    DUMP_LRCCPU_TASK_TCB             = (0x0200000F),
    DUMP_LRCCPU_RTOSCK_CDA           = (0x02000010),
    DUMP_LRCCPU_SIM0                 = (0x02000064),
    DUMP_LRCCPU_SIM1                 = (0x02000065),
    DUMP_LRCCPU_LP                   = (0x02000067),
    DUMP_LRCCPU_LP_BUSERROR          = (0x02000068),
    DUMP_LRCCPU_UTRACE0              = (0x02000069),
    DUMP_LRCCPU_UTRACE1              = (0x0200006a),
    DUMP_LRCCPU_UTRACE2              = (0x0200006b),
    DUMP_LRCCPU_UTRACE3              = (0x0200006c),
    DUMP_LRCCPU_ICC                  = (0x0200006E),
    DUMP_LRCCPU_AMON                 = (0x02000070),
    DUMP_LRCCPU_RUN_TRACE            = (0x02000072),
    DUMP_LRCCPU_SYSCTRL              = (0x02000074),
    DUMP_LRCCPU_PM_OM                = (0x02000075),
    DUMP_LRCCPU_SAVE_MOD_DUAL_MODEM     = (0x02000076),
    DUMP_LRCCPU_CIPHER               = (0x02000077),
    DUMP_LRCCPU_IPF                  = (0x02000078),
    DUMP_LRCCPU_PSAM                 = (0x02000079),
    DUMP_LRCCPU_PDLOCK               = (0x0200007a),
    DUMP_LRCCPU_NV                   = (0x0200007b),
    DUMP_LRCCPU_OS_SYS               = (0x0200007C),
    DUMP_LRCCPU_TASK_SWITCH1         = (0x0200007d),
    DUMP_LRCCPU_TASK_SWITCH2         = (0x0200007e),
    DUMP_LRCCPU_TASK_SWITCH3         = (0x0200007f),
    DUMP_LRCCPU_SYSTEM_STACK1        = (0x02000080),
    DUMP_LRCCPU_SYSTEM_STACK2        = (0x02000081),
    DUMP_LRCCPU_SYSTEM_STACK3        = (0x02000082),
    DUMP_LRCCPU_SEM_INFO             = (0x02000083),
    DUMP_LRCCPU_NOC                  = (0x02000084),
    DUMP_LRCCPU_EDMA                 = (0x02000085),
    DUMP_LRCCPU_CPM_LTE0             = (0x02000086),
    DUMP_LRCCPU_CPM_LTE1             = (0x02000087),
    DUMP_LRCCPU_CPM_TDS              = (0x02000088),
    DUMP_LRCCPU_CPM_NXP              = (0x02000089),
    DUMP_LRCCPU_CPM_1X               = (0x0200008A),
    DUMP_LRCCPU_DUMP_DEBUG           = (0x02000090),
    DUMP_LRCCPU_EXCINFO              = (0x02000091),
    DUMP_LRCCPU_REBOOTCONTEX         = (0x02000092),
    DUMP_LRCCPU_WDT                  = (0x02000093),
    DUMP_LRCCPU_FREQ                 = (0x02000094),
    DUMP_LRCCPU_PMU                  = (0x02000095),
    DUMP_LRCCPU_FIQ                  = (0x02000096),
    DUMP_LRCCPU_DSP0                 = (0x02000097),

    DUMP_LRCCPU_BBP                  = (0x02000098),
    DDUMP_LRCCPU_DSP1                = (0x02000099),
    DDUMP_LRCCPU_LTEV                = (0x0200009a),

    /*0x02F00000-- 专用于smp修改新增的特定格式段其他模块不得使用*/
    DUMP_LRCCPU_BASE_INFO_SMP        = (0x02f00000),
    DUMP_LRCCPU_CPUINFO              = (0x02f00001),
    DUMP_LRCCPU_CPUINFO1             = (0x02f00002),
    DUMP_LRCCPU_CPUINFO2             = (0x02f00003),
    DUMP_LRCCPU_CPUINFO3             = (0x02f00004),
    DUMP_LRCCPU_TASK_TCB_SMP         = (0x02f00005),
    DUMP_LRCCPU_DEBUGREG_CPU         = (0x02f00006),/*0x02f00006 ~ 0x02f0000d :8个预留给Debug Reg保存使用*/
    DUMP_LRCCPU_DEBUGREG_CPU_END     = (0x02f0000d),
    DUMP_LRCCPU_TASK_TCB_SMP_NEW     = (0x02f0000e),
    DUMP_LRCCPU_SEC_DUMP_TARNS       = (0x02f0000f),
	DUMP_LRCCPU_NRCPM_NR             = (0x02f00010),
    DUMP_LRCCPU_NRCPM_DSP            = (0x02f00011),

    DUMP_LRCCPU_FIELD_LLT            = (0x02fffffe),
    DUMP_LRCCPU_FIELD_END            = (0x02ffffff),

    /*AP kernel FIELD IDs*/
    DUMP_KERNEL_FIELD_BEGIN     = (0x03000000),
    DUMP_KERNEL_BASE_INFO       = (DUMP_KERNEL_FIELD_BEGIN),
    DUMP_KERNEL_TASK_SWITCH     = (0x030F0001),
    DUMP_KERNEL_INTLOCK         = (0x030F0002),
    DUMP_KERNEL_TASK_STACK      = (0x030F0003),
    DUMP_KERNEL_INT_STACK       = (0x030F0004),
    DUMP_KERNEL_TASK_NAME       = (0x030F0005),
    DUMP_KERNEL_ALLTASK_TCB     = (0x030F0006),
    DUMP_KERNEL_PRINT           = (0x030F0007),
    DUMP_KERNEL_REGS            = (0x030F0008),
    DUMP_KERNEL_CPUVIEW         = (0x030F0009),
    DUMP_KERNEL_USER_DATA       = (0x030F000B),
    DUMP_KERNEL_AMON            = (0x030F000C),
    DUMP_KERNEL_RUNNING_TRACE   = (0x030F000D),/*0x030F000D ~ 0x030F0014 :8个预留给running trace使用*/
    DUMP_KERNEL_CPU_INFO        = (0x030F0015),/*0x030F0015 ~ 0x030F001D :8个预留给cpu info使用*/
    DUMP_KERNEL_BASE_INFO1      = (0x030F001E),/*SMP 系统新增扩展baseinfo ID*/
    DUMP_KERNEL_TASK_TCB        = (0x030F001F),
    DUMP_KERNEL_DEBUGREG_CPU    = (0x030F0020),/*0x030F0020 ~ 0x030F0027 :8个预留给Debug Reg保存使用*/
    DUMP_KERNEL_DEBUGREG_CPU_END= (0x030F0027),
    DUMP_KERNEL_USB             = (0x03100000),
    DUMP_KERNEL_UTRACE          = (0x03100001),
    DUMP_KERNEL_DRX             = (0x03100002),
    DUMP_KERNEL_UTRACE0         = (0x03100003),
    DUMP_KERNEL_UTRACE1         = (0x03100004),
    DUMP_KERNEL_UTRACE2         = (0x03100005),
    DUMP_KERNEL_UTRACE3         = (0x03100006),
    DUMP_KERNEL_NOC             = (0x03100007),
    DUMP_KERNEL_PDLOCK          = (0x03100008),
    DUMP_KERNEL_FIELD_END       = (0x03ffffff),

    /*M3 FIELD IDs*/
    DUMP_M3_FIELD_BEGIN         = (0x04000000),
    DUMP_M3_LP_BUSERROR         = (DUMP_M3_FIELD_BEGIN),
    DUMP_M3_ICC                 = (0x04000001),
    DUMP_M3_PM_OM               = (0x04000002),
    DUMP_M3_BASE_INFO           = (0x04000020),
    DUMP_M3_TASK_SWITCH         = (0x04000021),
    DUMP_M3_INTLOCK             = (0x04000022),
    DUMP_M3_TASK_STACK          = (0x04000023),
    DUMP_M3_INT_STACK           = (0x04000024),
    DUMP_M3_ALLTASK             = (0x04000025),
    DUMP_M3_ALLTASK_TCB         = (0x04000026),
    DUMP_M3_LOG             = (0x04000027),
    DUMP_M3_PRINT           = (0x04000028),
    DUMP_M3_REGS                = (0x04000029),
    DUMP_M3_TCM0                = (0x0400002A),
    DUMP_M3_TCM1                = (0x0400002B),
    DUMP_M3_DDRMNT              = (0x0400002C),
    DUMP_M3_APB_SPI              = (0x0400002d),
    DUMP_M3_BOOST               = (0x04000064),

    /*0x04100000--0x04200000 专用于smp修改新增的段其他模块不得使用*/
    DUMP_M3_SMP_START           = (0x04100000),
    DUMP_M3_CPUINFO             = (DUMP_M3_SMP_START),
    DUMP_M3_BASEINFO_SMP        = (DUMP_M3_SMP_START+1),
    DUMP_M3_SMP_END             = (0x041FFFFF),

    DUMP_M3_FIELD_END           = (0x04FFFFFF),

    /* SHARE FIELD IDs */
    DUMP_SHARE_FIELD_BEGIN  = (0x05000000),
    DUMP_SHARE_GLOBAL_INFO  = (DUMP_SHARE_FIELD_BEGIN),
    DUMP_SHARE_LOAD_INFO    = (0x05000001),
    DUMP_SHARE_FASTBOOT_INFO= (0x05000002),
    DUMP_SHARE_FIELD_END,

    /* TEEOS FIELD IDs */
    DUMP_TEE_FIELD_BEGIN  = (0x06000000),
    DUMP_TEE_FIELD_LOG    = (0x06000001),
    DUMP_TEE_FIELD_END,
    DUMP_NRCCPU_FIELD_BEGIN  = (0x07000000),

    /*CP FIELD IDs*/
    DUMP_NRCCPU_BASE_INFO            = DUMP_NRCCPU_FIELD_BEGIN,
    DUMP_NRCCPU_TASK_SWITCH          = (0x07000001),
    DUMP_NRCCPU_INTLOCK              = (0x07000002),
    DUMP_NRCCPU_TASK_STACK           = (0x07000003),
    DUMP_NRCCPU_SYSTEM_STACK         = (0x07000004),
    DUMP_NRCCPU_ALLTASK_NAME         = (0x07000005),
    DUMP_NRCCPU_ALLTASK_TCB          = (0x07000006),
    DUMP_NRCCPU_PRINT                = (0x07000007),
    DUMP_NRCCPU_DMESG                = (0x07000008),
    DUMP_NRCCPU_REGS                 = (0x07000009),
    DUMP_NRCCPU_CPUVIEW              = (0x0700000A),
    DUMP_NRCCPU_USER_DATA            = (0x0700000C),
    DUMP_NRCCPU_BBE16_TCM            = (0x0700000D),
    DUMP_NRCCPU_DRX                  = (0x0700000E),
    DUMP_NRCCPU_TASK_TCB             = (0x0700000F),
    DUMP_NRCCPU_RTOSCK_CDA           = (0x07000010),
    DUMP_NRCCPU_SIM0                 = (0x07000064),
    DUMP_NRCCPU_SIM1                 = (0x07000065),
    DUMP_NRCCPU_LP                   = (0x07000067),
    DUMP_NRCCPU_LP_BUSERROR          = (0x07000068),
    DUMP_NRCCPU_UTRACE0              = (0x07000069),
    DUMP_NRCCPU_UTRACE1              = (0x0700006a),
    DUMP_NRCCPU_UTRACE2              = (0x0700006b),
    DUMP_NRCCPU_UTRACE3              = (0x0700006c),
    DUMP_NRCCPU_ICC                  = (0x0700006E),
    DUMP_NRCCPU_AMON                 = (0x07000070),
    DUMP_NRCCPU_RUN_TRACE            = (0x07000072),
    DUMP_NRCCPU_SYSCTRL              = (0x07000074),
    DUMP_NRCCPU_PM_OM                = (0x07000075),
    DUMP_NRCCPU_SAVE_MOD_DUAL_MODEM     = (0x07000076),
    DUMP_NRCCPU_CIPHER               = (0x07000077),
    DUMP_NRCCPU_IPF                  = (0x07000078),
    DUMP_NRCCPU_PSAM                 = (0x07000079),
    DUMP_NRCCPU_PDLOCK               = (0x0700007a),
    DUMP_NRCCPU_NV                   = (0x0700007b),
    DUMP_NRCCPU_OS_SYS               = (0x0700007C),
    DUMP_NRCCPU_TASK_SWITCH1         = (0x0700007d),
    DUMP_NRCCPU_TASK_SWITCH2         = (0x0700007e),
    DUMP_NRCCPU_TASK_SWITCH3         = (0x0700007f),
    DUMP_NRCCPU_SYSTEM_STACK1        = (0x07000080),
    DUMP_NRCCPU_SYSTEM_STACK2        = (0x07000081),
    DUMP_NRCCPU_SYSTEM_STACK3        = (0x07000082),
    DUMP_NRCCPU_SEM_INFO             = (0x07000083),
    DUMP_NRCCPU_NOC                  = (0x07000084),
    DUMP_NRCCPU_EDMA                 = (0x07000085),
    DUMP_NRCCPU_CPM_LTE0             = (0x07000086),
    DUMP_NRCCPU_CPM_LTE1             = (0x07000087),
    DUMP_NRCCPU_CPM_TDS              = (0x07000088),
    DUMP_NRCCPU_CPM_NXP              = (0x07000089),
    DUMP_NRCCPU_CPM_1X               = (0x0700008A),
    DUMP_NRCCPU_DUMP_DEBUG           = (0x07000090),
    DUMP_NRCCPU_EXCINFO              = (0x07000091),
    DUMP_NRCCPU_REBOOTCONTEX         = (0x07000092),
    DUMP_NRCCPU_WDT                  = (0x07000093),
    DUMP_NRCCPU_FREQ                 = (0x07000094),
    DUMP_NRCCPU_PMU                  = (0x07000095),
    DUMP_NRCCPU_FIQ                  = (0x07000096),
    DUMP_NRCCPU_DSP0                 = (0x07000097),

    DUMP_NRCCPU_BBP                  = (0x07000098),

    DUMP_NRCCPU_NRCPM_NR             = (0x07000099),
    DUMP_NRCCPU_NRCPM_NXP0           = (0x0700009a),

    /*0x02F00000-- 专用于smp修改新增的特定格式段其他模块不得使用*/
    DUMP_NRCCPU_BASE_INFO_SMP        = (0x07f00000),
    DUMP_NRCCPU_CPUINFO              = (0x07f00001),
    DUMP_NRCCPU_CPUINFO1             = (0x07f00002),
    DUMP_NRCCPU_CPUINFO2             = (0x07f00003),
    DUMP_NRCCPU_CPUINFO3             = (0x07f00004),
    DUMP_NRCCPU_TASK_TCB_SMP         = (0x07f00005),
    DUMP_NRCCPU_DEBUGREG_CPU         = (0x07f00006),/*0x02f00006 ~ 0x02f0000d :8个预留给Debug Reg保存使用*/
    DUMP_NRCCPU_DEBUGREG_CPU_END     = (0x07f0000d),
    DUMP_NRCCPU_TASK_TCB_SMP_NEW     = (0x07f0000e),
    DUMP_NRCCPU_SEC_DUMP_TARNS       = (0x07f0000f),
    DUMP_NRCCPU_FIELD_LLT            = (0x07fffffe),
    DUMP_NRCCPU_FIELD_END            = (0x07ffffff),

    /*CP FIELD IDs*/
    DUMP_NRL2HAC_FIELD_BEGIN          = (0x08000000),
    DUMP_NRL2HAC_BASE_INFO            = DUMP_NRL2HAC_FIELD_BEGIN,
    DUMP_NRL2HAC_TASK_SWITCH          = (0x08000001),
    DUMP_NRL2HAC_INTLOCK              = (0x08000002),
    DUMP_NRL2HAC_TASK_STACK           = (0x08000003),
    DUMP_NRL2HAC_SYSTEM_STACK         = (0x08000004),
    DUMP_NRL2HAC_ALLTASK_NAME         = (0x08000005),
    DUMP_NRL2HAC_ALLTASK_TCB          = (0x08000006),
    DUMP_NRL2HAC_PRINT                = (0x08000007),
    DUMP_NRL2HAC_DMESG                = (0x08000008),
    DUMP_NRL2HAC_REGS                 = (0x08000009),
    DUMP_NRL2HAC_CPUVIEW              = (0x0800000A),
    DUMP_NRL2HAC_USER_DATA            = (0x0800000C),
    DUMP_NRL2HAC_BBE16_TCM            = (0x0800000D),
    DUMP_NRL2HAC_DRX                  = (0x0800000E),
    DUMP_NRL2HAC_TASK_TCB             = (0x0800000F),
    DUMP_NRL2HAC_RTOSCK_CDA           = (0x08000010),
    DUMP_NRL2HAC_SIM0                 = (0x08000064),
    DUMP_NRL2HAC_SIM1                 = (0x08000065),
    DUMP_NRL2HAC_LP                   = (0x08000067),
    DUMP_NRL2HAC_LP_BUSERROR          = (0x08000068),
    DUMP_NRL2HAC_UTRACE0              = (0x08000069),
    DUMP_NRL2HAC_UTRACE1              = (0x0800006a),
    DUMP_NRL2HAC_UTRACE2              = (0x0800006b),
    DUMP_NRL2HAC_UTRACE3              = (0x0800006c),
    DUMP_NRL2HAC_ICC                  = (0x0800006E),
    DUMP_NRL2HAC_AMON                 = (0x08000070),
    DUMP_NRL2HAC_RUN_TRACE            = (0x08000072),
    DUMP_NRL2HAC_SYSCTRL              = (0x08000074),
    DUMP_NRL2HAC_PM_OM                = (0x08000075),
    DUMP_NRL2HAC_SAVE_MOD_DUAL_MODEM     = (0x08000076),
    DUMP_NRL2HAC_CIPHER               = (0x08000077),
    DUMP_NRL2HAC_IPF                  = (0x08000078),
    DUMP_NRL2HAC_PSAM                 = (0x08000079),
    DUMP_NRL2HAC_PDLOCK               = (0x0800007a),
    DUMP_NRL2HAC_NV                   = (0x0800007b),
    DUMP_NRL2HAC_OS_SYS               = (0x0800007C),
    DUMP_NRL2HAC_TASK_SWITCH1         = (0x0800007d),
    DUMP_NRL2HAC_TASK_SWITCH2         = (0x0800007e),
    DUMP_NRL2HAC_TASK_SWITCH3         = (0x0800007f),
    DUMP_NRL2HAC_SYSTEM_STACK1        = (0x08000080),
    DUMP_NRL2HAC_SYSTEM_STACK2        = (0x08000081),
    DUMP_NRL2HAC_SYSTEM_STACK3        = (0x08000082),
    DUMP_NRL2HAC_SEM_INFO             = (0x08000083),
    DUMP_NRL2HAC_NOC                  = (0x08000084),
    DUMP_NRL2HAC_EDMA                 = (0x08000085),
    DUMP_NRL2HAC_CPM_LTE0             = (0x08000086),
    DUMP_NRL2HAC_CPM_LTE1             = (0x08000087),
    DUMP_NRL2HAC_CPM_TDS              = (0x08000088),
    DUMP_NRL2HAC_CPM_NXP              = (0x08000089),
    DUMP_NRL2HAC_CPM_1X               = (0x0800008A),
    DUMP_NRL2HAC_DUMP_DEBUG           = (0x08000090),
    DUMP_NRL2HAC_EXCINFO              = (0x08000091),
    DUMP_NRL2HAC_REBOOTCONTEX         = (0x08000092),
    DUMP_NRL2HAC_WDT                  = (0x08000093),
    DUMP_NRL2HAC_FREQ                 = (0x08000094),
    DUMP_NRL2HAC_PMU                  = (0x08000095),
    DUMP_NRL2HAC_FIQ                  = (0x08000096),
    DUMP_NRL2HAC_DSP0                 = (0x08000097),
    /*0x02F00000-- 专用于smp修改新增的特定格式段其他模块不得使用*/
    DUMP_NRL2HAC_BASE_INFO_SMP        = (0x08f00000),
    DUMP_NRL2HAC_CPUINFO              = (0x08f00001),
    DUMP_NRL2HAC_CPUINFO1             = (0x08f00002),
    DUMP_NRL2HAC_CPUINFO2             = (0x08f00003),
    DUMP_NRL2HAC_CPUINFO3             = (0x08f00004),
    DUMP_NRL2HAC_TASK_TCB_SMP         = (0x08f00005),
    DUMP_NRL2HAC_DEBUGREG_CPU         = (0x08f00006),/*0x02f00006 ~ 0x02f0000d :8个预留给Debug Reg保存使用*/
    DUMP_NRL2HAC_DEBUGREG_CPU_END     = (0x08f0000d),
    DUMP_NRL2HAC_TASK_TCB_SMP_NEW     = (0x08f0000e),
    DUMP_NRL2HAC_FIELD_LLT            = (0x08fffffe),
    DUMP_NRL2HAC_FIELD_END            = (0x08FFFFFF),


    DUMP_NRHL1C_FIELD_BEGIN  = (0x09000000),
    DUMP_NRHL1C_FIELD_END    = (0x09FFFFFF),
    DUMP_NRPHY_FIELD_BEGIN  = (0x0a000000),
    DUMP_NRPHY_FIELD_END    = (0x0aFFFFFF),
    DUMP_RF_FIELD_BEGIN  = (0x0b000000),
    DUMP_RF_FIELD_END    = (0x0bFFFFFF),
}DUMP_SAVE_MOD_ENUM;


#define AP_TRACE_ID(id)                 (DUMP_KERNEL_UTRACE##id)

#ifdef __OS_NRCCPU__
#define CP_TRACE_ID(id)                 (DUMP_NRCCPU_UTRACE0+id)
#else
#define CP_TRACE_ID(id)                 (DUMP_LRCCPU_UTRACE0+id)
#endif

#define HAC_TRACE_ID(id)                 (DUMP_NRL2HAC_UTRACE0+id)


typedef enum _teeos_errno_e{
    TEEOS_ERRNO_LOAD_SEC_IMAGE  = 0x83000000,   /* modem单独复位中,加载安全镜像失败 */
    TEEOS_ERRNO_BUTT            = 0x83ffffff
}dump_teeos_errno_t;

typedef enum _hifi_errno_e{
    HIFI_ERRNO_MODEM_RESET      = 0x84000000,   /* modem单独复位中HIFI回调失败 */
    HIFI_ERRNO_BUTT             = 0x84ffffff
}dump_hifi_errno_t;

typedef enum _mcore_errno_e{
    LPM3_ERRNO_MODEM_RESET      = 0x85000000,   /* modem单独复位中M3异常 */
    LPM3_ERRNO_BUTT             = 0x85ffffff
}dump_mcore_errno_t;

#if defined(__OS_RTOSCK__) || defined(__OS_RTOSCK_SMP__) ||defined(__OS_RTOSCK_TVP__) ||defined(__OS_RTOSCK_TSP__)

#ifndef __OS_NRCCPU__
#define  DUMP_CP_FIELD_BEGIN                  (DUMP_LRCCPU_FIELD_BEGIN)
#define  DUMP_CP_BASE_INFO                    (DUMP_LRCCPU_BASE_INFO)
#define  DUMP_CP_TASK_SWITCH                  (DUMP_LRCCPU_TASK_SWITCH)
#define  DUMP_CP_INTLOCK                      (DUMP_LRCCPU_INTLOCK)
#define  DUMP_CP_TASK_STACK                   (DUMP_LRCCPU_TASK_STACK)
#define  DUMP_CP_SYSTEM_STACK                 (DUMP_LRCCPU_SYSTEM_STACK)
#define  DUMP_CP_ALLTASK_NAME                 (DUMP_LRCCPU_ALLTASK_NAME)
#define  DUMP_CP_ALLTASK_TCB                  (DUMP_LRCCPU_ALLTASK_TCB)
#define  DUMP_CP_PRINT                        (DUMP_LRCCPU_PRINT)
#define  DUMP_CP_DMESG                        (DUMP_LRCCPU_DMESG)
#define  DUMP_CP_REGS                         (DUMP_LRCCPU_REGS)
#define  DUMP_CP_CPUVIEW                      (DUMP_LRCCPU_CPUVIEW)
#define  DUMP_CP_USER_DATA                    (DUMP_LRCCPU_USER_DATA)
#define  DUMP_CP_BBE16_TCM                    (DUMP_LRCCPU_BBE16_TCM)
#define  DUMP_CP_DRX                          (DUMP_LRCCPU_DRX)
#define  DUMP_CP_TASK_TCB                     (DUMP_LRCCPU_TASK_TCB )
#define  DUMP_CP_RTOSCK_CDA                   (DUMP_LRCCPU_RTOSCK_CDA)
#define  DUMP_CP_SIM0                         (DUMP_LRCCPU_SIM0)
#define  DUMP_CP_SIM1                         (DUMP_LRCCPU_SIM1)
#define  DUMP_CP_LP                           (DUMP_LRCCPU_LP)
#define  DUMP_CP_LP_BUSERROR                  (DUMP_LRCCPU_LP_BUSERROR)
#define  DUMP_CP_UTRACE0                      (DUMP_LRCCPU_UTRACE0)
#define  DUMP_CP_UTRACE1                      (DUMP_LRCCPU_UTRACE1)
#define  DUMP_CP_UTRACE2                      (DUMP_LRCCPU_UTRACE2)
#define  DUMP_CP_UTRACE3                      (DUMP_LRCCPU_UTRACE3)
#define  DUMP_CP_ICC                          (DUMP_LRCCPU_ICC)
#define  DUMP_CP_AMON                         (DUMP_LRCCPU_AMON)
#define  DUMP_CP_RUN_TRACE                    (DUMP_LRCCPU_RUN_TRACE)
#define  DUMP_CP_SYSCTRL                      (DUMP_LRCCPU_SYSCTRL)
#define  DUMP_CP_PM_OM                        (DUMP_LRCCPU_PM_OM)
#define  DUMP_SAVE_MOD_DUAL_MODEM             (DUMP_LRCCPU_SAVE_MOD_DUAL_MODEM)
#define  DUMP_CP_CIPHER                       (DUMP_LRCCPU_CIPHER)
#define  DUMP_CP_IPF                          (DUMP_LRCCPU_IPF)
#define  DUMP_CP_PSAM                         (DUMP_LRCCPU_PSAM)
#define  DUMP_CP_PDLOCK                       (DUMP_LRCCPU_PDLOCK)
#define  DUMP_CP_NV                           (DUMP_LRCCPU_NV)
#define  DUMP_CP_OS_SYS                       (DUMP_LRCCPU_OS_SYS)
#define  DUMP_CP_TASK_SWITCH1                 (DUMP_LRCCPU_TASK_SWITCH1)
#define  DUMP_CP_TASK_SWITCH2                 (DUMP_LRCCPU_TASK_SWITCH2)
#define  DUMP_CP_TASK_SWITCH3                 (DUMP_LRCCPU_TASK_SWITCH3 )
#define  DUMP_CP_SYSTEM_STACK1                (DUMP_LRCCPU_SYSTEM_STACK1)
#define  DUMP_CP_SYSTEM_STACK2                (DUMP_LRCCPU_SYSTEM_STACK2)
#define  DUMP_CP_SYSTEM_STACK3                (DUMP_LRCCPU_SYSTEM_STACK3)
#define  DUMP_CP_SEM_INFO                     (DUMP_LRCCPU_SEM_INFO)
#define  DUMP_CP_NOC                          (DUMP_LRCCPU_NOC)
#define  DUMP_CP_EDMA                         (DUMP_LRCCPU_EDMA)
#define  DUMP_CP_CPM_LTE0                     (DUMP_LRCCPU_CPM_LTE0)
#define  DUMP_CP_CPM_LTE1                     (DUMP_LRCCPU_CPM_LTE1)
#define  DUMP_CP_CPM_TDS                      (DUMP_LRCCPU_CPM_TDS)
#define  DUMP_CP_CPM_NXP                      (DUMP_LRCCPU_CPM_NXP)
#define  DUMP_CP_CPM_1X                       (DUMP_LRCCPU_CPM_1X)
#define  DUMP_CP_DUMP_DEBUG                   (DUMP_LRCCPU_DUMP_DEBUG)
#define  DUMP_CP_EXCINFO                      (DUMP_LRCCPU_EXCINFO)
#define  DUMP_CP_REBOOTCONTEX                 (DUMP_LRCCPU_REBOOTCONTEX )
#define  DUMP_CP_WDT                          (DUMP_LRCCPU_WDT)
#define  DUMP_CP_FREQ                         (DUMP_LRCCPU_FREQ)
#define  DUMP_CP_PMU                          (DUMP_LRCCPU_PMU)
#define  DUMP_CP_FIQ                          (DUMP_LRCCPU_FIQ)
#define  DUMP_CP_DSP0                         (DUMP_LRCCPU_DSP0)
#define  DUMP_CP_BASE_INFO_SMP                (DUMP_LRCCPU_BASE_INFO_SMP)
#define  DUMP_CP_CPUINFO                      (DUMP_LRCCPU_CPUINFO)
#define  DUMP_CP_CPUINFO1                     (DUMP_LRCCPU_CPUINFO1)
#define  DUMP_CP_CPUINFO2                     (DUMP_LRCCPU_CPUINFO2)
#define  DUMP_CP_CPUINFO3                     (DUMP_LRCCPU_CPUINFO3)
#define  DUMP_CP_TASK_TCB_SMP                 (DUMP_LRCCPU_TASK_TCB_SMP)
#define  DUMP_CP_DEBUGREG_CPU                 (DUMP_LRCCPU_DEBUGREG_CPU)
#define  DUMP_CP_DEBUGREG_CPU_END             (DUMP_LRCCPU_DEBUGREG_CPU_END)
#define  DUMP_CP_TASK_TCB_SMP_NEW             (DUMP_LRCCPU_TASK_TCB_SMP_NEW)
#define  DUMP_CP_FIELD_LLT                    (DUMP_LRCCPU_FIELD_LLT)
#define  DUMP_CP_FIELD_END                    (DUMP_LRCCPU_FIELD_END)
#define  DUMP_CP_BBP                          (DUMP_LRCCPU_BBP)
#define  DUMP_CP_SEC_DUMP_TARNS               (DUMP_LRCCPU_SEC_DUMP_TARNS)
#define  DUMP_CP_DSP1                         (DDUMP_LRCCPU_DSP1)
#define  DUMP_CP_CPM_LTEV                     (DDUMP_LRCCPU_LTEV)

#else
#define  DUMP_CP_FIELD_BEGIN                  (DUMP_NRCCPU_FIELD_BEGIN)
#define  DUMP_CP_BASE_INFO                    (DUMP_NRCCPU_BASE_INFO)
#define  DUMP_CP_TASK_SWITCH                  (DUMP_NRCCPU_TASK_SWITCH)
#define  DUMP_CP_INTLOCK                      (DUMP_NRCCPU_INTLOCK)
#define  DUMP_CP_TASK_STACK                   (DUMP_NRCCPU_TASK_STACK)
#define  DUMP_CP_SYSTEM_STACK                 (DUMP_NRCCPU_SYSTEM_STACK)
#define  DUMP_CP_ALLTASK_NAME                 (DUMP_NRCCPU_ALLTASK_NAME)
#define  DUMP_CP_ALLTASK_TCB                  (DUMP_NRCCPU_ALLTASK_TCB)
#define  DUMP_CP_PRINT                        (DUMP_NRCCPU_PRINT)
#define  DUMP_CP_DMESG                        (DUMP_NRCCPU_DMESG)
#define  DUMP_CP_REGS                         (DUMP_NRCCPU_REGS)
#define  DUMP_CP_CPUVIEW                      (DUMP_NRCCPU_CPUVIEW)
#define  DUMP_CP_USER_DATA                    (DUMP_NRCCPU_USER_DATA)
#define  DUMP_CP_BBE16_TCM                    (DUMP_NRCCPU_BBE16_TCM)
#define  DUMP_CP_DRX                          (DUMP_NRCCPU_DRX)
#define  DUMP_CP_TASK_TCB                     (DUMP_NRCCPU_TASK_TCB )
#define  DUMP_CP_RTOSCK_CDA                   (DUMP_NRCCPU_RTOSCK_CDA)
#define  DUMP_CP_SIM0                         (DUMP_NRCCPU_SIM0)
#define  DUMP_CP_SIM1                         (DUMP_NRCCPU_SIM1)
#define  DUMP_CP_LP                           (DUMP_NRCCPU_LP)
#define  DUMP_CP_LP_BUSERROR                  (DUMP_NRCCPU_LP_BUSERROR)
#define  DUMP_CP_UTRACE0                      (DUMP_NRCCPU_UTRACE0)
#define  DUMP_CP_UTRACE1                      (DUMP_NRCCPU_UTRACE1)
#define  DUMP_CP_UTRACE2                      (DUMP_NRCCPU_UTRACE2)
#define  DUMP_CP_UTRACE3                      (DUMP_NRCCPU_UTRACE3)
#define  DUMP_CP_ICC                          (DUMP_NRCCPU_ICC)
#define  DUMP_CP_AMON                         (DUMP_NRCCPU_AMON)
#define  DUMP_CP_RUN_TRACE                    (DUMP_NRCCPU_RUN_TRACE)
#define  DUMP_CP_SYSCTRL                      (DUMP_NRCCPU_SYSCTRL)
#define  DUMP_CP_PM_OM                        (DUMP_NRCCPU_PM_OM)
#define  DUMP_SAVE_MOD_DUAL_MODEM             (DUMP_NRCCPU_SAVE_MOD_DUAL_MODEM)
#define  DUMP_CP_CIPHER                       (DUMP_NRCCPU_CIPHER)
#define  DUMP_CP_IPF                          (DUMP_NRCCPU_IPF)
#define  DUMP_CP_PSAM                         (DUMP_NRCCPU_PSAM)
#define  DUMP_CP_PDLOCK                       (DUMP_NRCCPU_PDLOCK)
#define  DUMP_CP_NV                           (DUMP_NRCCPU_NV)
#define  DUMP_CP_OS_SYS                       (DUMP_NRCCPU_OS_SYS)
#define  DUMP_CP_TASK_SWITCH1                 (DUMP_NRCCPU_TASK_SWITCH1)
#define  DUMP_CP_TASK_SWITCH2                 (DUMP_NRCCPU_TASK_SWITCH2)
#define  DUMP_CP_TASK_SWITCH3                 (DUMP_NRCCPU_TASK_SWITCH3 )
#define  DUMP_CP_SYSTEM_STACK1                (DUMP_NRCCPU_SYSTEM_STACK1)
#define  DUMP_CP_SYSTEM_STACK2                (DUMP_NRCCPU_SYSTEM_STACK2)
#define  DUMP_CP_SYSTEM_STACK3                (DUMP_NRCCPU_SYSTEM_STACK3)
#define  DUMP_CP_SEM_INFO                     (DUMP_NRCCPU_SEM_INFO)
#define  DUMP_CP_NOC                          (DUMP_NRCCPU_NOC)
#define  DUMP_CP_EDMA                         (DUMP_NRCCPU_EDMA)
#define  DUMP_CP_CPM_LTE0                     (DUMP_NRCCPU_CPM_LTE0)
#define  DUMP_CP_CPM_LTE1                     (DUMP_NRCCPU_CPM_LTE1)
#define  DUMP_CP_CPM_TDS                      (DUMP_NRCCPU_CPM_TDS)
#define  DUMP_CP_CPM_NXP                      (DUMP_NRCCPU_CPM_NXP)
#define  DUMP_CP_CPM_1X                       (DUMP_NRCCPU_CPM_1X)
#define  DUMP_CP_DUMP_DEBUG                   (DUMP_NRCCPU_DUMP_DEBUG)
#define  DUMP_CP_EXCINFO                      (DUMP_NRCCPU_EXCINFO)
#define  DUMP_CP_REBOOTCONTEX                 (DUMP_NRCCPU_REBOOTCONTEX )
#define  DUMP_CP_WDT                          (DUMP_NRCCPU_WDT)
#define  DUMP_CP_FREQ                         (DUMP_NRCCPU_FREQ)
#define  DUMP_CP_PMU                          (DUMP_NRCCPU_PMU)
#define  DUMP_CP_FIQ                          (DUMP_NRCCPU_FIQ)
#define  DUMP_CP_DSP0                         (DUMP_NRCCPU_DSP0)
#define  DUMP_CP_BASE_INFO_SMP                (DUMP_NRCCPU_BASE_INFO_SMP)
#define  DUMP_CP_CPUINFO                      (DUMP_NRCCPU_CPUINFO)
#define  DUMP_CP_CPUINFO1                     (DUMP_NRCCPU_CPUINFO1)
#define  DUMP_CP_CPUINFO2                     (DUMP_NRCCPU_CPUINFO2)
#define  DUMP_CP_CPUINFO3                     (DUMP_NRCCPU_CPUINFO3)
#define  DUMP_CP_TASK_TCB_SMP                 (DUMP_NRCCPU_TASK_TCB_SMP)
#define  DUMP_CP_DEBUGREG_CPU                 (DUMP_NRCCPU_DEBUGREG_CPU)
#define  DUMP_CP_DEBUGREG_CPU_END             (DUMP_NRCCPU_DEBUGREG_CPU_END)
#define  DUMP_CP_TASK_TCB_SMP_NEW             (DUMP_NRCCPU_TASK_TCB_SMP_NEW)
#define  DUMP_CP_SEC_DUMP_TARNS               (DUMP_NRCCPU_SEC_DUMP_TARNS)
#define  DUMP_CP_BBP                          (DUMP_NRCCPU_BBP)

#define  DUMP_CP_FIELD_LLT                    (DUMP_NRCCPU_FIELD_LLT)
#define  DUMP_CP_FIELD_END                    (DUMP_NRCCPU_FIELD_END)
#endif
#endif

#ifdef __cplusplus
}
#endif


#endif /* __BSP_OM_ENUM_H__ */


