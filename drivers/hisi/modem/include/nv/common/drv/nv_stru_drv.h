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


#ifndef __DRV_NV_DEF_H__
#define __DRV_NV_DEF_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "nv_id_drv.h"

#ifndef __u8_defined
#define __u8_defined
typedef signed char          s8;
typedef unsigned char        u8;
typedef signed short         s16;
typedef unsigned short       u16;
typedef signed int           s32;
typedef unsigned int         u32;
typedef signed long long     s64;
typedef unsigned long long   u64;
#endif

/*****************************************************************************
 结构名    : nv_protocol_base_type
 结构说明  : nv_protocol_base_type结构 ID= en_NV_Item_Modem_Log_Path 148
            MBB形态 modem log路径名，用于区分新老形态的modem log路径。
*****************************************************************************/
/*NV ID = 50018*/
#pragma pack(push)
#pragma pack(1)
    typedef struct
    {
        s32        nvStatus;
        s8         nv_version_info[30];
    }NV_SW_VER_STRU;
#pragma pack(pop)

/*NV ID  = 0xd10b*/

typedef struct ST_PWC_SWITCH_STRU_S {

    /*以下NV用于低功耗的整体控制，其中有些BIT暂时未用，做他用时，请更正为准确的名称*/
    u32 deepsleep  :1; /*bit0*/
    u32 lightsleep :1; /*bit1*/
    u32 dfs        :1; /*bit2*/
    u32 hifi       :1; /*bit3*/
    u32 drxAbb     :1; /*bit4*/
    u32 drxZspCore :1; /*bit5*/
    u32 drxZspPll  :1; /*bit6*/
    u32 drxWLBbpPll  :1; /*bit7*/
    u32 drxGBbpPll   :1; /*bit8*/
    u32 drxRf      :1; /*bit9*/
    u32 drxPa      :1; /*bit10*/
    u32 drxGuBbpPd   :1; /*bit11*/
    u32 drxDspPd     :1; /*bit12*/
    u32 drxLBbpPd    :1; /*bit13*/
    u32 drxPmuEco    :1; /*bit14*/
    u32 drxPeriPd    :1; /*bit15*/
    u32 l2cache_mntn  :1; /*bit16*/
    u32 bugChk     :1; /*bit17*/
    u32 pmuSwitch     :1; /*bit18*/
    u32 drxLdsp      :1;  /*bit 19*/
    u32 matserTDSpd  :1; /*bit20*/
    u32 tdsClk    :1;  /*bit21*/
    u32 slaveTDSpd   :1; /*bit22*/
    u32 slow         :1;/*bit23*/
    u32 ccpu_hotplug_suspend      :1;/*bit24 此方案拔核时，被拔出的cpu做suspend操作，耗时较长*/
    u32 ccpu_hotplug_crg      :1;/*bit25 此方案拔核时，被拔核仅作l1cache和remove smp操作，耗时叫短，但是低功耗睡眠时需要先把被拔出的cpu插入*/
    u32 ccpu_tickless          :1;/*bit26 ccpu tickless*/
    u32 sleep_monitor_time  :4; /*bit27-30*/
    u32 reserved    :1; /*bit31*/

    /*以下NV用于DEBUG上下电和开关钟*/
    u32 drx_pa_pd        :1; /*bit0 用于控制PA的上下电*/
    u32 drx_rfic_pd      :1; /*bit1 用于控制RFIC的上下电*/
    u32 drx_rfclk_pd     :1; /*bit2 用于控制RFIC CLK的上下电*/
    u32 drx_fem_pd       :1; /*bit3 用于控制FEM的上下电*/
    u32 drx_cbbe16_pd    :1; /*bit4 用于控制CBBE16的上下电*/
    u32 drx_bbe16_pd     :1; /*bit5 用于控制BBE16的上下电*/
    u32 drx_abb_pd       :1; /*bit6 用于控制ABB的上下电*/
    u32 drx_bbp_init     :1; /*bit7 用于控制BBP默认将钟电全部开启*/
    u32 drx_bbppwr_pd    :1; /*bit8 用于控制BBP电源阈的上下电*/
    u32 drx_bbpclk_pd    :1; /*bit9 用于控制BBP时钟阈的开关钟*/
    u32 drx_bbp_pll      :1; /*bit10 用于控制BBP_PLL的开关钟*/
    u32 drx_cbbe16_pll   :1; /*bit11 用于控制CBBE16_PLL的开关钟*/
    u32 drx_bbe16_pll    :1; /*bit12 用于控制BBE16_PLL的开关钟*/
    u32 drx_bbp_reserved :1; /*bit13 bbp预留*/
    u32 drx_abb_gpll     :1; /*bit14 用于控制ABB_GPLL的开关钟*/
    u32 drx_abb_scpll    :1; /*bit15 用于控制ABB_SCPLL的开关钟*/
    u32 drx_abb_reserved1:1;
    u32 drx_abb_reserved2:1;
    u32 reserved2        :14; /*bit18-31 未用*/
}ST_PWC_SWITCH_STRU;

/*NV ID = 0xd10c*/
typedef struct ST_PWC_DFS_STRU_S {
    u32 CcpuUpLimit;
    u32 CcpuDownLimit;
    u32 CcpuUpNum;
    u32 CcpuDownNum;
    u32 AcpuUpLimit;
    u32 AcpuDownLimit;
    u32 AcpuUpNum;
    u32 AcpuDownNum;
    u32 DFSTimerLen;
    u32 DFSHifiLoad;
    u32 Strategy;/*使用何种策略bit0:1->200ms负载检测,bit0:0->4s负载检测;bit1:1/0打开/关闭辅助DDR调频*/
    u32 DFSDdrUpLimit;
    u32 DFSDdrDownLimit;
    u32 DFSDdrprofile;
    u32 reserved;
}ST_PWC_DFS_STRU;

/*ID=0xd111 begin */
typedef struct
{
    u32 dump_switch    : 2; /* 00: excdump, 01: usbdump, 1x: no dump */
    u32 ARMexc         : 1; /* 2 ARM异常检测开关*/
    u32 stackFlow      : 1; /* 3 堆栈溢出检测开关*/
    u32 taskSwitch     : 1; /* 4 任务切换记录开关*/
    u32 intSwitch      : 1; /* 5 中断记录开关*/
    u32 intLock        : 1; /* 6 锁中断记录开关*/
    u32 appRegSave1    : 1; /* 7 寄存器组1记录开关 */
    u32 appRegSave2    : 1; /* 8 寄存器组2记录开关*/
    u32 appRegSave3    : 1; /* 9 寄存器组3记录开关*/
    u32 commRegSave1   : 1; /* 10 寄存器组1记录开关 */
    u32 commRegSave2   : 1; /* 11 寄存器组2记录开关*/
    u32 commRegSave3   : 1; /* 12 寄存器组3记录开关*/
    u32 sysErrReboot   : 1; /* 13 systemError复位开关*/
    u32 reset_log      : 1; /* 14 强制记录开关，暂未使用*/
    u32 fetal_err      : 1; /* 15 强制记录开关，暂未使用*/
    u32 log_ctrl       : 2; /* bsp_trsce 输出控制*/
    u32 dumpTextClip   : 1; /* ddr保存时text段裁剪特性*/
    u32 reserved1      : 13;
} DUMP_CFG_STRU;

typedef struct
{
    union
    {
        u32         uintValue;
        DUMP_CFG_STRU   Bits;
    } dump_cfg;

    u32 appRegAddr1;    /* ACORE保存寄存器组地址1*/
    u32 appRegSize1;    /* ACORE保存寄存器组长度1*/
    u32 appRegAddr2;    /* ACORE保存寄存器组地址2*/
    u32 appRegSize2;    /* ACORE保存寄存器组长度2*/
    u32 appRegAddr3;    /* ACORE保存寄存器组地址3*/
    u32 appRegSize3;    /* ACORE保存寄存器组长度3*/

    u32 commRegAddr1;   /* CCORE保存寄存器组地址1*/
    u32 commRegSize1;   /* CCORE保存寄存器组长度1*/
    u32 commRegAddr2;   /* CCORE保存寄存器组地址2*/
    u32 commRegSize2;   /* CCORE保存寄存器组长度2*/
    u32 commRegAddr3;   /* CCORE保存寄存器组地址3*/
    u32 commRegSize3;   /* CCORE保存寄存器组长度3*/

    u32 traceOnstartFlag;           /* 0:开机启动Trace, 非0:开机不启动Trace */
    u32 traceCoreSet;               /* 0:采集A核Trace, 1:采集C核Trace, 2:采集双核Trace */
    u32 busErrFlagSet;             /* 0:开机不启动防总线挂死功能, 非0:开机启动防总线挂死功能 */
} NV_DUMP_STRU;
/*ID=0xd111 end */

/*NV ID = 0xd115 start*/
typedef struct {
    u32 index;           /*硬件版本号数值(大版本号1+大版本号2)，区分不同产品*/
    u32 hwIdSub;        /*硬件子版本号，区分产品的不同的版本*/
    char  name[32];           /*内部产品名*/
    char    namePlus[32];       /*内部产品名PLUS*/
    char    hwVer[32];          /*硬件版本名称*/
    char    dloadId[32];        /*升级中使用的名称*/
    char    productId[32];      /*外部产品名*/
}PRODUCT_INFO_NV_STRU;

/* product support module nv define */
typedef struct
{
    u32 sdcard      : 1;//1: support; 0: not support
    u32 charge      : 1;
    u32 wifi        : 1;
    u32 oled        : 1;
    u32 hifi        : 1;
    u32 onoff       : 1;
    u32 hsic        : 1;
    u32 localflash  : 1;
    u32 reserved    : 24;
} DRV_MODULE_SUPPORT_STRU;

/*库仑计电压，电流校准参数*/
typedef struct
{
    u32 v_offset_a;         /* 电压校准线性参数 */
    s32 v_offset_b;         /* 电压校准线性偏移*/
    u32 c_offset_a;         /* 电流校准线性参数 */
    s32 c_offset_b;         /* 电流校准线性偏移 */
}COUL_CALI_NV_TYPE;

/*E5串口复用*/
typedef struct
{
    u32 wait_usr_sele_uart : 1;//[bit 0-0]1: await user's command for a moment; 0: do not wait
    u32 a_core_uart_num    : 2;//[bit 1-2]the number of uart used by a core
    u32 c_core_uart_num    : 2;//[bit 3-4]the number of uart used by c core
    u32 m_core_uart_num    : 2;//[bit 5-6]the number of uart used by m core
    u32 a_shell            : 1;//[bit 7-7]0:ashell is not used; 1:ashell is used
    u32 c_shell            : 1;//[bit 8-8]0:cshell is not used; 1:cshell is used
    u32 uart_at            : 1;//[bit 9-9]uart at control
    u32 extendedbits       : 22;//[b00]1:open cshell_auart; 0:close
}DRV_UART_SHELL_FLAG;

typedef struct
{
    u32 wdt_enable;
    u32 wdt_timeout;
    u32 wdt_keepalive_ctime;
    u32 wdt_suspend_timerout;
    u32 wdt_reserve;
}DRV_WDT_INIT_PARA_STRU;

/* pm om控制 NV项 = 0xd145 */
typedef struct
{
    u32 mem_ctrl:1;       /* 控制bit位:使用哪种类型的memory */
    u32 reserved:31;      /* 控制bit位:保留 */
    u8  log_threshold[4]; /* acore/ccore/mcore触发写文件的阀值, 数值表示百分比 */
    u8  mod_sw[8];        /* 各个模块log功能控制开关 */
}DRV_PM_OM_CFG_STRU;

/* watchpoint = 0xd148 begin */
typedef struct
{
    u32 enable;     /* 使能标志，0，去使能；1，使能， 如果该标志为去使能，配置将直接忽略 */
    u32 type;       /* 监控类型: 1，读；2，写；3，读写 */
    u32 start_addr; /* 监控起始地址 */
    u32 end_addr;   /* 监控结束地址 */
}WATCHPOINT_CFG;

typedef struct
{
    WATCHPOINT_CFG  ap_cfg[4];  /* A核配置，最多支持4个watchpoint */
    WATCHPOINT_CFG  cp_cfg[4];  /* A核配置，最多支持4个watchpoint */
}DRV_WATCHPOINT_CFG_STRU;
/* watchpoint = 0xd148 end */

typedef struct
{
    u32 DialupEnableCFG;
    u32 DialupACShellCFG;
}DRV_DIALUP_HSUART_STRU;

/* TEST support module nv define */
typedef struct
{
    u32 lcd         : 1;//1: support; 0: not support
    u32 emi         : 1;
    u32 pwm         : 1;
    u32 i2c         : 1;
    u32 leds        : 1;
    u32 reserved    : 27;
} DRV_MODULE_TEST_STRU;

typedef struct
{
    u32 ulResumeMode;       /*在NV数据出现错误时的处理方式,1:产线模式,2:用户模式*/
}NV_SELF_CTRL_STRU;

/*0xd158*/
typedef struct
{
    u32 pmu_enable_cfg;     /*pmu 打开关闭控制nv*/
    u32 protocol_sel;       /*pmu ssi spmi协议选择*/
    u32 reserved;
}DRV_PMU_CFG_STRU;

/*0xd168*/
typedef struct
{
    u32 ap_enable:1;        /*AP coresight enable flag*/
    u32 cp_enable:1;        /*CP coresight enable flag*/
    u32 ap_store :1;        /*AP coresight store flag*/
    u32 cp_store :1;        /*CP coresight store flag*/
    u32 reserved :28;
}DRV_CORESIGHT_CFG_STRU;

typedef struct
{
    u8 normalwfi_flag;
    u8 deepsleep_flag;
    u8 buck3off_flag;
    u8 peridown_flag;
    u32 deepsleep_Tth;
    u32 TLbbp_Tth;
}DRV_NV_PM_TYPE;

typedef struct
{
    u32 enPortNum;         /* 维测使用的物理通道类型 */
}DIAG_CHANNLE_PORT_CFG_STRU;

typedef struct {
    u32 iqi_enable:1;      /*0 iqi特性关闭 1 iqi特性打开*/
    u32 serial_enable:1;   /*serial 开关 1打开 0 关闭*/
    //u32 debug_enable:1;    /*debug功能使能*/
    u32 reservd:30;
}DRV_IQI_CFG_STRU;
#ifndef LPHY_UT_MODE//lint !e553
typedef struct convert_table
{
    s32 temperature;
    u16 code;
    u16 reserved;
} convert_table;
#define XO_TBL_SIZE 166
typedef struct xo_convert_table_array
{
    convert_table convert_table[XO_TBL_SIZE];
} xo_convert_table_array;
#define PA_TBL_SIZE 32
typedef struct pa_convert_table_array
{
    convert_table convert_table[PA_TBL_SIZE];
} pa_convert_table_array;
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
