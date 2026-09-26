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
 * Description :file format:UTF-8_format
 */


#ifndef __DRV_NV_DEF_H__
#define __DRV_NV_DEF_H__

#include "nv_id_drv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifndef __KERNEL__

#ifndef __u8_defined
#define __u8_defined
typedef signed char s8;         //lint !e761
typedef unsigned char u8;       //lint !e761
typedef signed short s16;       //lint !e761
typedef unsigned short u16;     //lint !e761
typedef signed int s32;         //lint !e761
typedef unsigned int u32;       //lint !e761
typedef signed long long s64;   //lint !e761
typedef unsigned long long u64; //lint !e761
#endif

#endif
/*lint --e{959}*/
/*
 * 结构名    : nv_protocol_base_type
 * 结构说明  : nv_protocol_base_type结构 ID= en_NV_Item_Modem_Log_Path 148
            MBB形态 modem log路径名，用于区分新老形态的modem log路径。
 */
/* NV ID  50018 */
#pragma pack(push)
#pragma pack(1)

#define NV_VERSION_INFO_LEN 30
/* 结构说明  : 软件版本号。 */
typedef struct {
    s32 nvStatus;           /* 当前NV是否有效。 */
    s8 nv_version_info[NV_VERSION_INFO_LEN]; /* 软件版本号信息。 */
} NV_SW_VER_STRU;
#pragma pack(pop)

/* NV ID  0xd10b */
/* 结构说明  : 控制hotplug特性开关，第一个32bit的后边reserved的8个bit中，使用一个bit，使用后reserved的bit减少为7。 */
typedef struct ST_PWC_SWITCH_STRU_S {
    /* 以下NV用于低功耗的整体控制，其中有些BIT暂时未用，做他用时，请更正为准确的名称 */
    u32 deepsleep : 1;            /* Ccpu深睡开关 1：使能，0：不使能 */
    u32 lightsleep : 1;           /* 未使用。 */
    u32 dfs : 1;                  /* 调频开关 1：使能，0：不使能 */
    u32 hifi : 1;                 /* 未使用。 */
    u32 drxAbb : 1;               /* 未使用。 */
    u32 drxZspCore : 1;           /* 未使用。 */
    u32 drxZspPll : 1;            /* 未使用。 */
    u32 drxWLBbpPll : 1;          /* 未使用。 */
    u32 drxGBbpPll : 1;           /* 未使用。 */
    u32 drxRf : 1;                /* 未使用。 */
    u32 drxPa : 1;                /* 未使用。 */
    u32 drxGuBbpPd : 1;           /* 未使用。 */
    u32 drxDspPd : 1;             /* 未使用。 */
    u32 drxLBbpPd : 1;            /* 未使用。 */
    u32 drxPmuEco : 1;            /* 未使用。 */
    u32 drxPeriPd : 1;            /* 未使用。 */
    u32 l2cache_mntn : 1;         /* 未使用。 */
    u32 bugChk : 1;               /* 未使用。 */
    u32 pmuSwitch : 1;            /* 未使用。 */
    u32 drxLdsp : 1;              /* 未使用。 */
    u32 matserTDSpd : 1;          /* 未使用。 */
    u32 tdsClk : 1;               /* 未使用。 */
    u32 slaveTDSpd : 1;           /* 未使用。 */
    u32 slow : 1;                 /* 未使用。 */
    u32 ccpu_hotplug_suspend : 1; /* bit24 此方案拔核时，被拔出的cpu做suspend操作，耗时较长 */
    u32 ccpu_hotplug_crg : 1; /* Ccpu热插拔方案2开关.拔掉cpu时，被拔出的cpu仅做关钟和刷cache操作，较节省时间。但是低功耗睡眠时需要先把被拔出的cpu插入
                               */
    u32 ccpu_tickless : 1;    /* Ccpu的tickless开关。 */
    u32 rsv : 4;
    u32 ddrdfs : 1; /* Ddrdfs控制开关。 */

    /* 以下NV用于DEBUG上下电和开关钟 */
    u32 drx_pa_pd : 1;         /* bit0 用于控制PA的上下电 */
    u32 drx_rfic_pd : 1;       /* bit1 用于控制RFIC的上下电 */
    u32 drx_rfclk_pd : 1;      /* bit2 用于控制RFIC CLK的上下电 */
    u32 drx_fem_pd : 1;        /* bit3 用于控制FEM的上下电 */
    u32 drx_cbbe16_pd : 1;     /* bit4 用于控制CBBE16的上下电 */
    u32 drx_bbe16_pd : 1;      /* bit5 用于控制BBE16的上下电 */
    u32 drx_abb_pd : 1;        /* bit6 用于控制ABB的上下电 */
    u32 drx_bbp_init : 1;      /* bit7 用于控制BBP默认将钟电全部开启 */
    u32 drx_bbppwr_pd : 1;     /* bit8 用于控制BBP电源阈的上下电 */
    u32 drx_bbpclk_pd : 1;     /* bit9 用于控制BBP时钟阈的开关钟 */
    u32 drx_bbp_pll : 1;       /* bit10 用于控制BBP_PLL的开关钟 */
    u32 drx_cbbe16_pll : 1;    /* bit11 用于控制CBBE16_PLL的开关钟 */
    u32 drx_bbe16_pll : 1;     /* bit12 用于控制BBE16_PLL的开关钟 */
    u32 drx_bbp_reserved : 1;  /* bit13 bbp预留 */
    u32 drx_abb_gpll : 1;      /* bit14 用于控制ABB_GPLL的开关钟 */
    u32 drx_abb_scpll : 1;     /* bit15 用于控制ABB_SCPLL的开关钟 */
    u32 drx_abb_reserved1 : 1; /* abb预留，未使用。 */
    u32 drx_abb_reserved2 : 1; /* abb预留，未使用。 */
    u32 reserved2 : 14;        /* bit18-31 未用 */
} ST_PWC_SWITCH_STRU;

/* 结构说明  : 该NV实现对nr低功耗特性的开关控制。 */
typedef struct ST_PWC_NR_POWERCTRL_STRU_S {
    u8 nrcpudeepsleep; /* 控制NR R8 CPU低功耗特性开关 */
    u8 l2hacdeepsleep; /* 控制L2HAC R8 CPU低功耗特性开关 */
    u8 cpubusdfs;      /* 调频调压开关 */
    u8 hotplug;        /* CPU 热插拔，当前特性不用打开 */
    u8 tickless;       /* Tickless，当前特性不打开 */
    u8 drxnrbbpinit;   /* nrbbp回片初始化打桩 */
    u8 dxrbbpaxidfs;   /* nrbbp自动降频打桩 */
    u8 drxnrbbpclk;    /* 保留 */
    u8 drxnrbbppll;    /* bbppll开关打桩 */
    u8 drxnrbbppmu;    /* nrbbp上下电打桩 */
    u8 drxbbainit;     /* 保留 */
    u8 drxbbapwr;      /* bba上电打桩 */
    u8 drxbbaclk;      /* 保留 */
    u8 drxbbapll;      /* 保留 */
    u8 drxbbapmu;      /* 保留 */
    u8 drxl1c;         /* 保留 */
    u8 drxl1cpll;      /* 保留 */
    u8 reserved;       /* 保留 */
} ST_PWC_NR_POWERCTRL_STRU;

/* 结构说明：DDR温保NV */
typedef struct ST_DDR_TMON_PROTECT_STRU_S {
    u32 ddr_tmon_protect_on : 1; /* DDR温保开关，1:开，0:关 */
    u32 ddr_vote_dvfs_alone : 1; /* DDR调频是否受温保影响，1:温保不影响，0:调频考虑温保因素 */
    u32 reserved1 : 6;
    u32 ddr_tmon_protect_enter_threshold : 3; // 温保进入的threshold
    u32 ddr_tmon_protect_exit_threshold : 3;  // 温保退出threshold
    u32 ddr_tmon_protect_freq_threshold : 2;  // 进入温保后DDR的最大工作频点
    u32 ddr_tmon_protect_upper : 3;           // ddr 高温复位门限
    u32 ddr_tmon_protect_downer : 3;          // ddr低温复位门限
    u32 reserved2 : 10;
} ST_DDR_TMON_PROTECT_STRU;

/* 结构说明：DRV PM CHR上报特性开关NV */
typedef struct ST_CHR_REPORT_STRU_S {
    /*
     * 0：DRV PM CHR上报特性关闭；
     * 其他：不睡眠上报异常阈值，单位为分钟。
     */
    u32 pm_monitor_time; /* pm monitor time,Unit:minute,1 means if cp not sleep ,CHR will report the pm state */
    u32 reserved1;
    u32 reserved2;
    u32 reserved3;
} ST_CHR_REPORT_STRU;

/* NV ID 0xd10c */
/* 结构说明  : 用于低功耗动态调频。 */
typedef struct ST_PWC_DFS_STRU_S {
    u32 CcpuUpLimit;   /* C核负载上调限值，默认80 ，超过它需要把频率上调。 */
    u32 CcpuDownLimit; /* C核负载下调限值，默认55 ，低于它需要把频率下调。 */
    u32 CcpuUpNum;     /* C核连续高于负载上限次数，默认1,超过它需要把频率上调。 */
    u32 CcpuDownNum;   /* C核连续低于负载下限次数，默认3,超过它需要把频率下调。 */
    u32 AcpuUpLimit;   /* A核负载上调限值，默认80 ，超过它需要把频率上调。 */
    u32 AcpuDownLimit; /* A核负载下调限值，默认55 ，超过它需要把频率下调。 */
    u32 AcpuUpNum;     /* A核连续高于负载上限次数，默认1，超过它需要把频率上调。 */
    u32 AcpuDownNum;   /* A核连续低于负载下限次数，默认3，低于它需要把频率下调。 */
    u32 DFSTimerLen;   /* 定时查询负载时间，默认400 个tick，1tick=10ms。 */
    u32 DFSHifiLoad;   /* 保留 */
    /*
     * Dfs使用哪种策略，仅在Phone形态使用。
     * bit0:1使用200ms调频检测策略；bit0:0使用4s调频检测策略；
     * bit1:0：关闭DDR调频；1：打开DDR调频。
     */
    u32 Strategy;
    u32 DFSDdrUpLimit;   /* DDR调频负载上限，仅在Phone形态使用。 */
    u32 DFSDdrDownLimit; /* DDR调频负载下限，仅在Phone形态使用。 */
    u32 DFSDdrprofile;   /* DDR调频最大profile值，仅在Phone形态使用。 */
    u32 reserved; /* 保留扩展NV，现用于表示睡眠唤醒后的设置的profile值，仅在MBB相关产品使用。 */
} ST_PWC_DFS_STRU;

/* ID=0xd111 begin */
typedef struct {
    /*
     * Dump模式。
     * 00: excdump；
     * 01: usbdump；
     * 1x: no dump。
     */
    u32 dump_switch : 2;
    u32 ARMexc : 1;       /* ARM异常检测开关。 */
    u32 stackFlow : 1;    /* 堆栈溢出检测开关。 */
    u32 taskSwitch : 1;   /* 任务切换记录开关。 */
    u32 intSwitch : 1;    /* 中断记录开关。 */
    u32 intLock : 1;      /* 锁中断记录开关。 */
    u32 appRegSave1 : 1;  /* ACORE寄存器组1记录开关。 */
    u32 appRegSave2 : 1;  /* ACORE寄存器组2记录开关。 */
    u32 appRegSave3 : 1;  /* ACORE寄存器组3记录开关。 */
    u32 commRegSave1 : 1; /* CCORE寄存器组1记录开关。 */
    u32 commRegSave2 : 1; /* CCORE寄存器组2记录开关。 */
    u32 commRegSave3 : 1; /* CCORE寄存器组3记录开关。 */
    u32 sysErrReboot : 1; /* SystemError复位开关。 */
    u32 reset_log : 1;    /* 强制记录开关，暂未使用。 */
    u32 fetal_err : 1;    /* 强制记录开关，暂未使用。 */
    u32 log_ctrl : 2;     /* log开关。 */
    u32 dumpTextClip : 1; /* ddr保存时text段裁剪特性 */
    u32 secDump : 1;
    u32 mccnt : 1;
    u32 pccnt : 1;
    u32 rrt : 1;
    u32 reserved1 : 9; /* 保留。 */
} DUMP_CFG_STRU;

/*
 * 结构名    : NV_DCXO_C1C2_CAL_RESULT_STRU
 * 结构说明  : NVID DCXO C1C2校准结果
 */
typedef struct {
    u16 shwC2fix; /* 粗调电容值，档位0-15有效，约3ppm/码字 */
    u16 shwC1fix; /* 细调电容值，档位0-255有效，约0.3ppm/码字 */
} NV_FAC_DCXO_C1C2_CAL_RESULT_STRU;

/* 结构说明  : 用来控制可维可测功能。 */
typedef struct {
    u32 thread0_channel : 1;
    u32 thread1_channel : 1;
    u32 thread2_channel : 1;
    u32 thread3_channel : 1;
    u32 thread4_channel : 1;
    u32 thread5_channel : 1;
    u32 thread6_channel : 1;
    u32 thread7_channel : 1;
    u32 thread8_channel : 1;
    u32 thread9_channel : 1;
    u32 thread10_channel : 1;
    u32 thread11_channel : 1;
    u32 thread12_channel : 1;
    u32 thread13_channel : 1;
    u32 thread14_channel : 1;
    u32 thread15_channel : 1;
    u32 reserved : 16; /* 保留。 */
} DUMP_SCHDULE_CTRL_STRU;


/* 结构说明  : 用来控制可维可测功能。 */
typedef struct {
    union {
        u32 uintValue;
        DUMP_CFG_STRU Bits;
    } dump_cfg;
    union {
        u32 thread_cfg;
        DUMP_SCHDULE_CTRL_STRU thread_bits;
    } schudule_cfg;

    u32 appRegAddr1; /* ACORE保存寄存器组地址1 */
    u32 appRegSize1; /* ACORE保存寄存器组长度1 */
    u32 appRegAddr2; /* ACORE保存寄存器组地址2 */
    u32 appRegSize2; /* ACORE保存寄存器组长度2 */
    u32 appRegAddr3; /* ACORE保存寄存器组地址3 */
    u32 appRegSize3; /* ACORE保存寄存器组长度3 */

    u32 commRegAddr1; /* CCORE保存寄存器组地址1 */
    u32 commRegSize1; /* CCORE保存寄存器组长度1 */
    u32 commRegAddr2; /* CCORE保存寄存器组地址2 */
    u32 commRegSize2; /* CCORE保存寄存器组长度2 */
    u32 commRegAddr3; /* CCORE保存寄存器组地址3 */
    u32 commRegSize3; /* CCORE保存寄存器组长度3 */

    /*
     * Trace采集开关。
     * 1:开机启动Trace；
     * 非1:开机不启动Trace。
     */
    u32 traceOnstartFlag;
    /*
     * Trace采集配置。
     * bit1：采集app核；
     * bit2：采集modem核。
     */
    u32 traceCoreSet;
    /*
     * 防总线挂四功能开关。
     * 0：开机不启动防总线挂死功能；
     * 非0：开机启动防总线挂死功能。
     */
    u32 busErrFlagSet;
} NV_DUMP_STRU; /* ID 0xd111 end */

#define NV_STRU_NAME_LEN 32
/* NV ID 0xd115 start */
/* 结构说明  : 控制硬件版本号。 */
typedef struct {
    u32 index;          /* 硬件版本号数值(大版本号1+大版本号2)，区分不同产品 */
    u32 hwIdSub;        /* 硬件子版本号，区分产品的不同的版本 */
    char name[NV_STRU_NAME_LEN];      /* 内部产品名 */
    char namePlus[NV_STRU_NAME_LEN];  /* 内部产品名PLUS */
    char hwVer[NV_STRU_NAME_LEN];     /* 硬件版本名称 */
    char dloadId[NV_STRU_NAME_LEN];   /* 升级中使用的名称 */
    char productId[NV_STRU_NAME_LEN]; /* 外部产品名 */
} PRODUCT_INFO_NV_STRU;

/* product support module nv define */
/* 结构说明  : 用来标识产品板各硬件模块是否支持，1：支持，0不支持。
 * 各产品形态可以在该产品的NV DIFF文件中，根据硬件ID号来定制是否支持某个模块，也可据此扩展对应产品形态特有的其他模块。
 */
typedef struct {
    /*
     * 是否支持SD卡。
     * 1：支持；0：不支持。
     */
    u32 sdcard : 1;
    u32 charge : 1;     /* 是否支持充电。 */
    u32 wifi : 1;       /* 是否支持Wi-Fi。 */
    u32 oled : 1;       /* 是否支持LCD、OLED等屏。 */
    u32 hifi : 1;       /* 是否支持Hi-Fi语音模块。 */
    u32 onoff : 1;      /* 是否支持开关机操作。 */
    u32 hsic : 1;       /* 是否HSIC接口。 */
    u32 localflash : 1; /* 是否支持本地Flash存储。 */
    u32 reserved : 24;  /* 保留bit位，供扩展使用。 */
} DRV_MODULE_SUPPORT_STRU;

/* 结构说明：库仑计电压，电流校准参数 */
typedef struct {
    u32 v_offset_a; /* 电压校准线性参数 */
    s32 v_offset_b; /* 电压校准线性偏移 */
    u32 c_offset_a; /* 电流校准线性参数 */
    s32 c_offset_b; /* 电流校准线性偏移 */
} COUL_CALI_NV_TYPE;

/* E5串口复用 */
/* 结构说明  : 各核串口号分配，虚拟串口和物理AT口开关标志。 */
typedef struct {
    u32 wait_usr_sele_uart : 1; /* [bit 0-0]1: await user's command for a moment; 0: do not wait */
    u32 a_core_uart_num : 2;    /* 分配给A核的物理串口号。 */
    u32 c_core_uart_num : 2;    /* 分配给C核的物理串口号。 */
    u32 m_core_uart_num : 2;    /* 分配给M核的物理串口号。 */
    /*
     * A-Shell开关标志。
     * 0：关；1：开。
     */
    u32 a_shell : 1;
    /*
     * C-Shell开关标志。
     * 0：关；1：开。
     */
    u32 c_shell : 1;
    /*
     * 物理AT口开关标志。
     * 0：关；1：开。
     */
    u32 uart_at : 1;
    /*
     * 控制是否输出cshell的log到A核物理串口。
     * 0：关闭cshell到auart的路径；
     * 1：打开cshell到auart路径。
     */
    u32 extendedbits : 22;
} DRV_UART_SHELL_FLAG;

/*
 * 结构说明  : 看门狗初始化配置NV。
 * 该NV在B5000平台修改为可定制属性，终端按需修改定制；
 */
typedef struct {
    /*
     * 控制WDT是否打开。
     * 1：打开；0：关闭。
     */
    u32 wdt_enable;
    u32 wdt_timeout;          /* WDT超时时间。默认30s。 */
    u32 wdt_keepalive_ctime;  /* WDT喂狗时间间隔，默认15s。 */
    u32 wdt_suspend_timerout; /* Suspend中WDT超时时间默认150s。 */
    u32 wdt_reserve;          /* WDT保留项。 */
} DRV_WDT_INIT_PARA_STRU;

/* pm om控制 NV项 = 0xd145 */
/*
 * 结构说明  : 1、配置控制低功耗可维可测使用的memory类型；
 * 2、低功耗可维可测功能是否开启
 * 3、配置各个核的log触发文件记录的阀值。
 */
typedef struct {
    /*
     * 控制缓存是使用共享内存还是其他类型的memory。
     * 0：使用预先分配的256KB共享DDR；
     * 1：复用其他可维可测内存（如socp使用的DDR空间）。
     */
    u32 mem_ctrl : 1;
    u32 reserved : 31;   /* 控制bit位:保留 */
    u8 log_threshold[0x4]; /* 各个核的log触发文件记录的阀值，其中数组index对应各个核的ipc编号，阀值以百分比表示。 */
    /*
     * 控制各模块是否开启低功耗log记录功能，每个bit表示该模块状态。
     * 0：关闭
     * 1：打开
     */
    u8 mod_sw[0x8];
} DRV_PM_OM_CFG_STRU;

/* watchpoint = 0xd148 begin */
typedef struct {
    u32 enable; /* 使能标志，0，去使能；1，使能， 如果该标志为去使能，配置将直接忽略 */
    u32 type;   /* 监控类型: 1，读；2，写；3，读写 */
    u32 start_addr; /* 监控起始地址 */
    u32 end_addr;   /* 监控结束地址 */
} WATCHPOINT_CFG;
typedef struct {
    u32 enable; /* 使能标志，0，去使能；1，使能， 如果该标志为去使能，配置将直接忽略 */
    u32 type;   /* 监控类型: 1，读；2，写；3，读写 */
    u32 start_addr_low;  /* 监控起始地址低32bit */
    u32 start_addr_high; /* 监控起始地址高32bit */
    u32 end_addr_low;    /* 监控结束地址低32bit */
    u32 end_addr_high;   /* 监控结束地址高32bit */
} WATCHPOINT_CFG64;
/* 结构说明：配置watchpoint。保留NV项禁止修改 */
typedef struct {
    WATCHPOINT_CFG64 ap_cfg[0x4]; /* A核配置，最多支持4个watchpoint */
    WATCHPOINT_CFG cp_cfg[0x4];   /* C核配置，最多支持4个watchpoint */
} DRV_WATCHPOINT_CFG_STRU; /* watchpoint = 0xd148 end */

/* 0xD194, for tsensor start */
struct DRV_TSENSOR_NV_TEMP_UNIT {
    s32 high_temp;
    s32 low_temp;
};

struct DRV_TSENSOR_NV_TEMP_INFO {
    u32 temp_unit_nums;                                     /* 高温温度阈值实际单元数 */
    struct DRV_TSENSOR_NV_TEMP_UNIT temp_threshold_init[0x5]; /* 最多支持5组高温度阈值 */
};
/* 结构说明：温保特性NV控制 */
typedef struct {
    u32 enable_reset_hreshold;                       /* 使能复位门限标志,0,去使能；1,使能 */
    int reset_threshold_temp;                        /* 复位温度阈值 */
    u32 timer_value_s;                               /* 定时器超时值 */
    u32 ltemp_out_time;                              /* 低温温保退出时间 */
    u32 enable_print_temp;                           /* 使能10s打印一次温度 */
    u32 enable_htemp_protect;                        /* 高温温保使能标志, 0, 去使能;  1, 使能 */
    struct DRV_TSENSOR_NV_TEMP_INFO htemp_threshold; /* 高温温保阈值 */
    u32 enable_ltemp_protect;                        /* 低温温保使能标志, 0, 去使能;  1, 使能 */
    struct DRV_TSENSOR_NV_TEMP_INFO ltemp_threshold; /* 低温温保阈值 */
} DRV_TSENSOR_TEMP_PROTECT_CONTROL_NV_STRU; /* 0xD194, for tsensor end */

/* 0xd19a, for pcie speed ctrl  start 结构说明：PCIE调速和低功耗控制的NV */
typedef struct {
    u32 pcie_speed_ctrl_flag;  /* PCIE 速度控制标志,0,去使能；1,使能 */
    u32 pcie_pm_ctrl_flag;     /* PCIE 低功耗控制标志,0,去使能；1,使能 */
    u32 pcie_init_speed_mode;  /* 初始化时的PCIE 速度 */
    u32 pcie_pm_speed_mode;    /* 低功耗恢复时PCIE配置的速度 */
    u32 pcie_speed_work_delay; /* 当降低PCIE 速度的延时work 的延时时间时间是ms */

    u32 gen1_volume;           /* PCIE GEN1模式的流量阈值大小 */
    u32 gen2_volume;           /* PCIE GEN2模式的流量阈值大小 */
    u32 gen3_volume;           /* PCIE GEN3模式的流量阈值大小 */
    u32 pcie_l1ss_enable_flag; /* PCIE L1SS Enable ,  0,去使能；1,使能 */
} DRV_PCIE_SPEED_CTRL_STRU; /* 0xd19a, for pcie speed ctrl  end */

/* 结构说明： 控制高速uart拨号特性是否打开，数据端口转成ashell或cshell。主线不再使用 */
typedef struct {
    /*
     * 控制高速uart拨号特性是否打开。
     * 0：关闭；
     * 1：打开。
     */
    u32 DialupEnableCFG;
    /*
     * 该NV项用于711 hsuart拨号业务，NAS将命令参数写入到NV项中，软复位后
     * hsuart层会读取NV值，将hsuart端口切换成相应端口。
     * 0：hsuart为数传端口；
     * 0x5A5A5A5A：hsuart为AShell端口；
     * 0xA5A5A5A5：hsuart为CShell端口。
     */
    u32 DialupACShellCFG;
} DRV_DIALUP_HSUART_STRU;

/* TEST support module nv define */
/* 结构说明  : 该NV实现控制各个模块是否打开的功能，1：打开；0：关闭。 */
typedef struct {
    u32 lcd : 1;  /* 是否支持lcd模块，1: support; 0: not support */
    u32 emi : 1;  /* 是否支持emi模块 */
    u32 pwm : 1;  /* 是否支持pwm模块 */
    u32 i2c : 1;  /* 是否支持i2c模块 */
    u32 leds : 1; /* 是否支持leds模块 */
    u32 reserved : 27;
} DRV_MODULE_TEST_STRU;

/* 结构说明  : NV自管理。 */
typedef struct {
    /*
     * 在NV数据出现错误时的处理方式。
     * 1：产线模式；
     * 2：用户模式。
     */
    u32 ulResumeMode;
} NV_SELF_CTRL_STRU;

/* 0xd158 */
/* 结构说明  : 控制pmu接口类型是ssi接口还是spmi接口。 */
typedef struct {
    u32 pmu_enable_cfg; /* pmu 打开关闭控制NV。 */
    /*
     * pmu ssi spmi协议选择。
     * 0：ssi接口；1：spmi接口
     */
    u32 protocol_sel;
    u32 reserved; /* 保留。 */
} DRV_PMU_CFG_STRU;


/* 0xd159 */
/* 结构说明  : 该NV实现在modem 启动阶段配置 serdes 时钟的波形与驱动能力功能。 */
typedef struct {
    /*
     * Bit 7 最高位 0：通过nv 配置 serdes 时钟波形 mask，如果要修改 波形，需要同时修改 mask 为 1；
     * Bit 0 最低位 0：serdes配置为方波；1： serdes配置为正弦波
     */
    u8 clk_type;
    /*
     * Bit 7 最高位 0：通过nv 配置 serdes 时钟驱动能力 mask，如果要修改 波形，需要同时修改 mask 1；
     * Bit 0-bit6 配置serdes 时钟驱动能力的值
     */
    u8 serdes_clk_drv;
    u8 reserved1;
    u8 reserved2;
} DRV_PMU_CLK_STRU;

/* 0xd183 */
typedef struct {
    u32 sim_volt_flag;
    u32 reserved;
} DRV_NV_PMU_TYPE;

/* 0xd168 */
/* 结构说明  : （C10/C20  ）控制双核coresight功能的使能与ETB数据保存是否使能。 */
typedef struct {
    u32 ap_enable : 1; /* Ap coresight使能控制 1：使能，0：不使能 */
    u32 cp_enable : 1; /* CP coersight使能控制1：使能，0：不使能 */
    u32 ap_store : 1;  /* AP coresight store flag */
    u32 cp_store : 1;  /* Cp etb数据保存控制1：使能，0：不使能 */
    u32 reserved : 28;
} DRV_CORESIGHT_CFG_STRU;

/* 结构说明  : M3上控制低功耗的NV。 */
typedef struct {
    /*
     * M3是否进Normal wfi。
     * 0表示不进；
     * 1表示进。
     */
    u8 normalwfi_flag;
    /*
     * M3是否进深睡的标记。
     * 0表示不进；
     * 1表示进。
     */
    u8 deepsleep_flag;
    /*
     * M3 是否走Buck3掉电的流程。
     * 0表示Buck3不掉电；
     * 1表示Buck3掉电。
     */
    u8 buck3off_flag;
    /*
     * M3 如果Buck3不掉电的话，外设是否掉电。
     * 0表示外设不掉电；
     * 1表示外设掉电（注意，如果Buck3掉电的话，此NV配置不起作用，Buck3掉电，外设一定掉电）。
     */
    u8 peridown_flag;
    u32 deepsleep_Tth; /* C核深睡时间阈值。 */
    u32 TLbbp_Tth;     /* C核深睡，BBP寄存器的阈值。 */
} DRV_NV_PM_TYPE;

/* 结构说明  : 该NV实现对R8 mem retention特性的开关控制。 */
typedef struct {
    u32 lr_retention_enable;    /* 4G R8 MEM retention开关控制 */
    u32 nr_retention_enable;    /* 5G  R8 MEM retention开关控制 */
    u32 l2hac_retention_enable; /* L2HAC  R8 MEM retention开关控制,当前暂未启用，HAC mem与NR的使用nr_retention_enable统一控制 */
    u32 default_mode; /* Nv未打开情况下，默认模式，如果为1，则默认为shut down模式；如果为1，则默认为retention模式 */
    u32 reserved1;
} ST_PWC_RETENTION_STRU;

/* 结构说明  : 该NV实现对低功耗维测debug特性的开关控制。 */
typedef struct {
    /* if lockirq time length greater than threshold,this cfg decide whether system error */
    u32 lpmcu_lockirq_syserr : 1;
    u32 lrcpu_pminfo_report2hids : 1; /* if set,report lr pm info to hids,include dpm,pm,wakelock,rsracc etc */
    u32 nrcpu_pminfo_report2hids : 1; /* if set,report nr pm info to hids,include dpm,pm,wakelock,rsracc etc */
    u32 l2hac_pminfo_report2hids : 1; /* if set,report l2hac pm info to hids,include dpm,pm,wakelock,rsracc etc */
    u32 rsv : 28;
    u32 lpmcu_irq_monitor_threshold; /* record lpmcu lockirq time length threshold,slice number */
    u32 pm_stay_wake_time_threshold; /* ccpu保持唤醒时间阈值；超过该阈值，往hids打印维测信息 */
    u32 ccpu_sleep_check_time;       /* M3 check if response ccpu pd irq late,unit is slice */
    u32 nrwakeup_time_threshold;     /* M3 check if nrwakeup time over threshold,unit is slice */
    u32 lrwakeup_time_threshold;     /* M3 check if lrwakeup time over threshold,unit is slice */
    u32 rsv5;
    u32 rsv6;
    u32 rsv7;
    u32 rsv8;
} ST_PMOM_DEBUG_STRU;

/* 结构说明  : 该NV实现控制HKADC校准策略版本号 */
typedef struct {
    u8 version; /* 当版本号为1时，hkadc使用新的校准策略 */
    u8 rsv1;
    u8 rsv2;
    u8 rsv3;
} DRV_HKADC_CAL_VERSION;


/* 结构说明  : 用于指示维测（DIAG）使用的默认通道类型。。 */
typedef struct {
    /*
     * 0：USB通道
     * 1：VCOM
     */
    u32 enPortNum;
} DIAG_CHANNLE_PORT_CFG_STRU;
/* 结构说明：deflate 特性开关 */
typedef struct {
    u32 deflate_enable : 1; /* 0 deflate特性关闭 1 deflate特性打开 */
    u32 reservd : 31;
} DRV_DEFLATE_CFG_STRU;

/* 结构说明：废弃不用 */
typedef struct {
    u32 iqi_enable : 1;    /* 0 iqi特性关闭 1 iqi特性打开 */
    u32 serial_enable : 1; /* serial 开关 1打开 0 关闭 */
    // u32 debug_enable:1;    /*debug功能使能*/
    u32 reservd : 30;
} DRV_IQI_CFG_STRU;

/*
 * 结构说明  : 该NV实现对各组件进行开机log通知和各组件开机log级别配置的功能
 */
typedef struct {
    /*
     * 0：关闭开机LOG特性；
     * 1：打开开机LOG特性。
     */
    u8 cMasterSwitch;
    /*
     * 0：开机LOG内存不可用；
     * 1：开机LOG内存可用。
     */
    u8 cBufUsable;
    /*
     * 0：开机LOG内存不使能；
     * 1：开机LOG内存使能。
     */
    u8 cBufEnable;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    u8 cswACPUBsp;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    u8 cswACPUDiag;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    u8 cswACPUHifi;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    u8 cswLRMBsp;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    u8 cswLRMDiag;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    u8 cswLRMTLPhy;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    u8 cswLRMGUPhy;
    u8 cswLRMCPhy; /* Range:[0,3] 4G Modem CPHY开机log profile */
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    u8 cswLRMEasyRf;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    u8 cswNRMBsp;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    u8 cswNRMDiag;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    u8 cswNRMHAC;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    u8 cswNRMPhy;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    u8 cswNRMHL1C;
    /*
     * 0：开机LOG关闭
     * 1：精简级别LOG；
     * 2：正常级别LOG
     * 3：全LOG
     */
    u8 cswNRMPDE;
    u8 cReserved[0x6]; /* 保留字段 */
} DRV_NV_POWER_ON_LOG_SWITCH_STRU;


typedef struct {
    s32 temperature;
    u16 code;
    u16 reserved;
} DRV_CONVERT_TABLE_TYPE;
#define XO_TABLE_SIZE 166
/* 结构说明：内部使用 */
typedef struct {
    DRV_CONVERT_TABLE_TYPE convert_table[XO_TABLE_SIZE];
} DRV_XO_CONVERT_TABLE;
#define PA_TABLE_SIZE 32
/* 结构说明：内部使用 */
typedef struct {
    DRV_CONVERT_TABLE_TYPE convert_table[PA_TABLE_SIZE];
} DRV_PA_CONVERT_TABLE;


#ifndef LPHY_UT_MODE //lint !e553
typedef struct convert_table {
    s32 temperature;
    u16 code;
    u16 reserved;
} convert_table;
#define XO_TBL_SIZE 166
typedef struct xo_convert_table_array {
    convert_table convert_table[XO_TBL_SIZE];
} xo_convert_table_array;
#define PA_TBL_SIZE 32
typedef struct pa_convert_table_array {
    convert_table convert_table[PA_TBL_SIZE];
} pa_convert_table_array;
#endif
/* NVID 50019 for HiBurn tool update */
typedef struct {
    /*
    0 : fastboot直接启动到kernel
    1 : 启动停留在fastboot
    */
    u8 update_mode_flag;
    u8 reserved1;
    u8 reserved2;
    u8 reserved3;
} NV_HIBURN_CONFIG_STRU;

/* 结构说明：该 NV 实现控制 digital power monitor 功耗拟合功能开关定制 */
typedef struct {
    /* 功耗拟合开关,0关闭,1开启,默认关闭 */
    u32 enable;
    /* 功耗拟合寄存器读取周期,单位是ms,默认是1000 */
    u32 task_delay_ms;
    u32 reserved1;
    u32 reserved2;
} DRV_POWER_MONITOR_CFG;

/*
 * 结构说明：NV状态标识
 */
typedef struct {
    unsigned char factory_reset_flag; /* 出厂恢复标志 1:出厂恢复 0:没有出厂恢复 */
    unsigned char reserve[0x3];         /* 保留字段 */
} NVM_OPT_MODE_STRU;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
