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

/*lint --e{959}*/
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
        signed int        nvStatus;
        signed char         nv_version_info[30];
    }NV_SW_VER_STRU;
#pragma pack(pop)

/*NV ID  = 0xd10b*/

typedef struct ST_PWC_SWITCH_STRU_S {

    /*以下NV用于低功耗的整体控制，其中有些BIT暂时未用，做他用时，请更正为准确的名称*/
    unsigned int deepsleep  :1; /*bit0*/
    unsigned int lightsleep :1; /*bit1*/
    unsigned int dfs        :1; /*bit2*/
    unsigned int hifi       :1; /*bit3*/
    unsigned int drxAbb     :1; /*bit4*/
    unsigned int drxZspCore :1; /*bit5*/
    unsigned int drxZspPll  :1; /*bit6*/
    unsigned int drxWLBbpPll  :1; /*bit7*/
    unsigned int drxGBbpPll   :1; /*bit8*/
    unsigned int drxRf      :1; /*bit9*/
    unsigned int drxPa      :1; /*bit10*/
    unsigned int drxGuBbpPd   :1; /*bit11*/
    unsigned int drxDspPd     :1; /*bit12*/
    unsigned int drxLBbpPd    :1; /*bit13*/
    unsigned int drxPmuEco    :1; /*bit14*/
    unsigned int drxPeriPd    :1; /*bit15*/
    unsigned int l2cache_mntn  :1; /*bit16*/
    unsigned int bugChk     :1; /*bit17*/
    unsigned int pmuSwitch     :1; /*bit18*/
    unsigned int drxLdsp      :1;  /*bit 19*/
    unsigned int matserTDSpd  :1; /*bit20*/
    unsigned int tdsClk    :1;  /*bit21*/
    unsigned int slaveTDSpd   :1; /*bit22*/
    unsigned int slow         :1;/*bit23*/
    unsigned int ccpu_hotplug_suspend      :1;/*bit24 此方案拔核时，被拔出的cpu做suspend操作，耗时较长*/
    unsigned int ccpu_hotplug_crg      :1;/*bit25 此方案拔核时，被拔核仅作l1cache和remove smp操作，耗时叫短，但是低功耗睡眠时需要先把被拔出的cpu插入*/
    unsigned int ccpu_tickless          :1;/*bit26 ccpu tickless*/
    unsigned int reserved  :4; /*bit27-30*/
    unsigned int ddrdfs    :1; /*bit31*/

    /*以下NV用于DEBUG上下电和开关钟*/
    unsigned int drx_pa_pd        :1; /*bit0 用于控制PA的上下电*/
    unsigned int drx_rfic_pd      :1; /*bit1 用于控制RFIC的上下电*/
    unsigned int drx_rfclk_pd     :1; /*bit2 用于控制RFIC CLK的上下电*/
    unsigned int drx_fem_pd       :1; /*bit3 用于控制FEM的上下电*/
    unsigned int drx_cbbe16_pd    :1; /*bit4 用于控制CBBE16的上下电*/
    unsigned int drx_bbe16_pd     :1; /*bit5 用于控制BBE16的上下电*/
    unsigned int drx_abb_pd       :1; /*bit6 用于控制ABB的上下电*/
    unsigned int drx_bbp_init     :1; /*bit7 用于控制BBP默认将钟电全部开启*/
    unsigned int drx_bbppwr_pd    :1; /*bit8 用于控制BBP电源阈的上下电*/
    unsigned int drx_bbpclk_pd    :1; /*bit9 用于控制BBP时钟阈的开关钟*/
    unsigned int drx_bbp_pll      :1; /*bit10 用于控制BBP_PLL的开关钟*/
    unsigned int drx_cbbe16_pll   :1; /*bit11 用于控制CBBE16_PLL的开关钟*/
    unsigned int drx_bbe16_pll    :1; /*bit12 用于控制BBE16_PLL的开关钟*/
    unsigned int drx_bbp_reserved :1; /*bit13 bbp预留*/
    unsigned int drx_abb_gpll     :1; /*bit14 用于控制ABB_GPLL的开关钟*/
    unsigned int drx_abb_scpll    :1; /*bit15 用于控制ABB_SCPLL的开关钟*/
    unsigned int drx_abb_reserved1:1;
    unsigned int drx_abb_reserved2:1;
    unsigned int reserved2        :14; /*bit18-31 未用*/
}ST_PWC_SWITCH_STRU;

typedef struct ST_PWC_NR_POWERCTRL_STRU_S {

    /*以下NV用于NR低功耗的整体控制，其中有些BIT暂时未用，做他用时，请更正为准确的名称*/
    unsigned char nrcpudeepsleep;/*lixiaojie*/
    unsigned char l2hacdeepsleep;/*lixiaojie*/
    unsigned char cpubusdfs;/*yangqiang*/
    unsigned char hotplug;/*lixiaojie*/
    unsigned char tickless;/*lixiaojie*/
    unsigned char drxnrbbpinit;/*ningzhaoke*/
    unsigned char dxrbbppwr;/*ningzhaoke*/
    unsigned char drxnrbbpclk;/*ningzhaoke*/
    unsigned char drxnrbbppll;/*ningzhaoke*/
    unsigned char drxnrbbppmu;/*ningzhaoke*/
    unsigned char drxbbainit;/*ningzhaoke*/
    unsigned char drxbbapwr;/*ningzhaoke*/
    unsigned char drxbbaclk;/*ningzhaoke*/
    unsigned char drxbbapll;/*ningzhaoke*/
    unsigned char drxbbapmu;/*ningzhaoke*/
    unsigned char drxl1c;/*chenquan*/
    unsigned char drxl1cpll;/*chenquan*/
    unsigned char reserved;
}ST_PWC_NR_POWERCTRL_STRU;

typedef struct ST_DDR_TMON_PROTECT_STRU_S{
	unsigned int ddr_tmon_protect_on :1;
	unsigned int ddr_vote_dvfs_alone  :1;
	unsigned int reserved1 : 6;
	unsigned int ddr_tmon_protect_enter_threshold :3; //温保进入的threshold
	unsigned int ddr_tmon_protect_exit_threshold :3; //温保退出threshold
	unsigned int ddr_tmon_protect_freq_threshold:2;//进入温保后DDR的最大工作频点
	unsigned int ddr_tmon_protect_upper:3; //ddr 高温复位门限
	unsigned int ddr_tmon_protect_downer:3; //ddr低温复位门限
	unsigned int reserved2:10;
}ST_DDR_TMON_PROTECT_STRU;

typedef struct ST_CHR_REPORT_STRU_S {
    unsigned int pm_monitor_time;/*pm monitor time,Unit:minute,1 means if cp not sleep ,CHR will report the pm state*/
    unsigned int reserved1;
    unsigned int reserved2;
    unsigned int reserved3;
}ST_CHR_REPORT_STRU;

/*NV ID = 0xd10c*/
typedef struct ST_PWC_DFS_STRU_S {
    unsigned int CcpuUpLimit;
    unsigned int CcpuDownLimit;
    unsigned int CcpuUpNum;
    unsigned int CcpuDownNum;
    unsigned int AcpuUpLimit;
    unsigned int AcpuDownLimit;
    unsigned int AcpuUpNum;
    unsigned int AcpuDownNum;
    unsigned int DFSTimerLen;
    unsigned int DFSHifiLoad;
    unsigned int Strategy;/*使用何种策略bit0:1->200ms负载检测,bit0:0->4s负载检测;bit1:1/0打开/关闭辅助DDR调频*/
    unsigned int DFSDdrUpLimit;
    unsigned int DFSDdrDownLimit;
    unsigned int DFSDdrprofile;
    unsigned int reserved;
}ST_PWC_DFS_STRU;

/*ID=0xd111 begin */
typedef struct
{
    unsigned int dump_switch    : 2; /* 00: excdump, 01: usbdump, 1x: no dump */
    unsigned int ARMexc         : 1; /* 2 ARM异常检测开关*/
    unsigned int stackFlow      : 1; /* 3 堆栈溢出检测开关*/
    unsigned int taskSwitch     : 1; /* 4 任务切换记录开关*/
    unsigned int intSwitch      : 1; /* 5 中断记录开关*/
    unsigned int intLock        : 1; /* 6 锁中断记录开关*/
    unsigned int appRegSave1    : 1; /* 7 寄存器组1记录开关 */
    unsigned int appRegSave2    : 1; /* 8 寄存器组2记录开关*/
    unsigned int appRegSave3    : 1; /* 9 寄存器组3记录开关*/
    unsigned int commRegSave1   : 1; /* 10 寄存器组1记录开关 */
    unsigned int commRegSave2   : 1; /* 11 寄存器组2记录开关*/
    unsigned int commRegSave3   : 1; /* 12 寄存器组3记录开关*/
    unsigned int sysErrReboot   : 1; /* 13 systemError复位开关*/
    unsigned int reset_log      : 1; /* 14 强制记录开关，暂未使用*/
    unsigned int fetal_err      : 1; /* 15 强制记录开关，暂未使用*/
    unsigned int log_ctrl       : 2; /* bsp_trsce 输出控制*/
    unsigned int dumpTextClip   : 1; /* ddr保存时text段裁剪特性*/
	unsigned int secDump        : 1;
    unsigned int reserved1      : 12;
} DUMP_CFG_STRU;

typedef struct
{
    union
    {
        unsigned int         uintValue;
        DUMP_CFG_STRU   Bits;
    } dump_cfg;

    unsigned int appRegAddr1;    /* ACORE保存寄存器组地址1*/
    unsigned int appRegSize1;    /* ACORE保存寄存器组长度1*/
    unsigned int appRegAddr2;    /* ACORE保存寄存器组地址2*/
    unsigned int appRegSize2;    /* ACORE保存寄存器组长度2*/
    unsigned int appRegAddr3;    /* ACORE保存寄存器组地址3*/
    unsigned int appRegSize3;    /* ACORE保存寄存器组长度3*/

    unsigned int commRegAddr1;   /* CCORE保存寄存器组地址1*/
    unsigned int commRegSize1;   /* CCORE保存寄存器组长度1*/
    unsigned int commRegAddr2;   /* CCORE保存寄存器组地址2*/
    unsigned int commRegSize2;   /* CCORE保存寄存器组长度2*/
    unsigned int commRegAddr3;   /* CCORE保存寄存器组地址3*/
    unsigned int commRegSize3;   /* CCORE保存寄存器组长度3*/

    unsigned int traceOnstartFlag;           /* 0:开机启动Trace, 非0:开机不启动Trace */
    unsigned int traceCoreSet;               /* 0:采集A核Trace, 1:采集C核Trace, 2:采集双核Trace */
    unsigned int busErrFlagSet;             /* 0:开机不启动防总线挂死功能, 非0:开机启动防总线挂死功能 */
} NV_DUMP_STRU;
/*ID=0xd111 end */

/*NV ID = 0xd115 start*/
typedef struct {
    unsigned int index;           /*硬件版本号数值(大版本号1+大版本号2)，区分不同产品*/
    unsigned int hwIdSub;        /*硬件子版本号，区分产品的不同的版本*/
    char  name[32];           /*内部产品名*/
    char    namePlus[32];       /*内部产品名PLUS*/
    char    hwVer[32];          /*硬件版本名称*/
    char    dloadId[32];        /*升级中使用的名称*/
    char    productId[32];      /*外部产品名*/
}PRODUCT_INFO_NV_STRU;

/* product support module nv define */
typedef struct
{
    unsigned int sdcard      : 1;//1: support; 0: not support
    unsigned int charge      : 1;
    unsigned int wifi        : 1;
    unsigned int oled        : 1;
    unsigned int hifi        : 1;
    unsigned int onoff       : 1;
    unsigned int hsic        : 1;
    unsigned int localflash  : 1;
    unsigned int reserved    : 24;
} DRV_MODULE_SUPPORT_STRU;

/*库仑计电压，电流校准参数*/
typedef struct
{
    unsigned int v_offset_a;         /* 电压校准线性参数 */
    signed int v_offset_b;         /* 电压校准线性偏移*/
    unsigned int c_offset_a;         /* 电流校准线性参数 */
    signed int c_offset_b;         /* 电流校准线性偏移 */
}COUL_CALI_NV_TYPE;

/*E5串口复用*/
typedef struct
{
    unsigned int wait_usr_sele_uart : 1;//[bit 0-0]1: await user's command for a moment; 0: do not wait
    unsigned int a_core_uart_num    : 2;//[bit 1-2]the number of uart used by a core
    unsigned int c_core_uart_num    : 2;//[bit 3-4]the number of uart used by c core
    unsigned int m_core_uart_num    : 2;//[bit 5-6]the number of uart used by m core
    unsigned int a_shell            : 1;//[bit 7-7]0:ashell is not used; 1:ashell is used
    unsigned int c_shell            : 1;//[bit 8-8]0:cshell is not used; 1:cshell is used
    unsigned int uart_at            : 1;//[bit 9-9]uart at control
    unsigned int extendedbits       : 22;//[b00]1:open cshell_auart; 0:close
}DRV_UART_SHELL_FLAG;

typedef struct
{
    unsigned int wdt_enable;
    unsigned int wdt_timeout;
    unsigned int wdt_keepalive_ctime;
    unsigned int wdt_suspend_timerout;
    unsigned int wdt_reserve;
}DRV_WDT_INIT_PARA_STRU;

/* pm om控制 NV项 = 0xd145 */
typedef struct
{
    unsigned int mem_ctrl:1;       /* 控制bit位:使用哪种类型的memory */
    unsigned int reserved:31;      /* 控制bit位:保留 */
    unsigned char  log_threshold[4]; /* acore/ccore/mcore触发写文件的阀值, 数值表示百分比 */
    unsigned char  mod_sw[8];        /* 各个模块log功能控制开关 */
}DRV_PM_OM_CFG_STRU;

/* watchpoint = 0xd148 begin */
typedef struct
{
    unsigned int enable;     /* 使能标志，0，去使能；1，使能， 如果该标志为去使能，配置将直接忽略 */
    unsigned int type;       /* 监控类型: 1，读；2，写；3，读写 */
    unsigned int start_addr; /* 监控起始地址 */
    unsigned int end_addr;   /* 监控结束地址 */
}WATCHPOINT_CFG;

typedef struct
{
    WATCHPOINT_CFG  ap_cfg[4];  /* A核配置，最多支持4个watchpoint */
    WATCHPOINT_CFG  cp_cfg[4];  /* A核配置，最多支持4个watchpoint */
}DRV_WATCHPOINT_CFG_STRU;
/* watchpoint = 0xd148 end */

/* 0xD194, for tsensor start*/
#define TSENSOR_ID_MAX 4
struct DRV_TSENSOR_NV_TEMP_UNIT {
    signed int high_temp;
    signed int low_temp;
};

struct DRV_TSENSOR_NV_TEMP_INFO {
    unsigned int temp_unit_nums;                                     /*高温温度阈值实际单元数*/
    struct DRV_TSENSOR_NV_TEMP_UNIT temp_threshold_init[5]; /*最多支持5组高温度阈值*/
};

typedef struct
{
    unsigned int enable_reset_hreshold;                                         /*使能复位门限标志,0,去使能；1,使能*/
    unsigned int reset_threshold_temp;                                          /*复位温度阈值*/
    unsigned int timer_value_s;                                                 /*定时器超时值*/
    unsigned int enable_print_temp;                                             /*使能10s打印一次温度*/
    unsigned int enable_htemp_protect;                                          /*高温温保使能标志, 0, 去使能;  1, 使能*/
    struct DRV_TSENSOR_NV_TEMP_INFO htemp_threshold[TSENSOR_ID_MAX];  /*高温温保阈值*/
    unsigned int enable_ltemp_protect;                                          /*低温温保使能标志, 0, 去使能;  1, 使能*/
    struct DRV_TSENSOR_NV_TEMP_INFO ltemp_threshold[TSENSOR_ID_MAX];  /*低温温保阈值*/
}DRV_TSENSOR_TEMP_PROTECT_CONTROL_NV_STRU;
/* 0xD194, for tsensor end*/

typedef struct
{
    unsigned int DialupEnableCFG;
    unsigned int DialupACShellCFG;
}DRV_DIALUP_HSUART_STRU;

/* TEST support module nv define */
typedef struct
{
    unsigned int lcd         : 1;//1: support; 0: not support
    unsigned int emi         : 1;
    unsigned int pwm         : 1;
    unsigned int i2c         : 1;
    unsigned int leds        : 1;
    unsigned int reserved    : 27;
} DRV_MODULE_TEST_STRU;

typedef struct
{
    unsigned int ulResumeMode;       /*在NV数据出现错误时的处理方式,1:产线模式,2:用户模式*/
}NV_SELF_CTRL_STRU;

/*0xd158*/
typedef struct
{
    unsigned int pmu_enable_cfg;     /*pmu 打开关闭控制nv*/
    unsigned int protocol_sel;       /*pmu ssi spmi协议选择*/
    unsigned int reserved;
}DRV_PMU_CFG_STRU;

/*0xd183*/
typedef struct
{
	unsigned int sim_volt_flag;
	unsigned int reserved;
}DRV_NV_PMU_TYPE;

/*0xd168*/
typedef struct
{
    unsigned int ap_enable:1;        /*AP coresight enable flag*/
    unsigned int cp_enable:1;        /*CP coresight enable flag*/
    unsigned int ap_store :1;        /*AP coresight store flag*/
    unsigned int cp_store :1;        /*CP coresight store flag*/
    unsigned int reserved :28;
}DRV_CORESIGHT_CFG_STRU;

typedef struct
{
    unsigned char normalwfi_flag;
    unsigned char deepsleep_flag;
    unsigned char buck3off_flag;
    unsigned char peridown_flag;
    unsigned int deepsleep_Tth;
    unsigned int TLbbp_Tth;
}DRV_NV_PM_TYPE;

typedef struct
{
    unsigned int enPortNum;         /* 维测使用的物理通道类型 */
}DIAG_CHANNLE_PORT_CFG_STRU;

typedef struct {
unsigned int deflate_enable:1;      /*0 deflate特性关闭 1 deflate特性打开*/
unsigned int reservd:31;
}DRV_DEFLATE_CFG_STRU;

typedef struct {
    unsigned int iqi_enable:1;      /*0 iqi特性关闭 1 iqi特性打开*/
    unsigned int serial_enable:1;   /*serial 开关 1打开 0 关闭*/
    //unsigned int debug_enable:1;    /*debug功能使能*/
    unsigned int reservd:30;
}DRV_IQI_CFG_STRU;

typedef struct
{
    unsigned char  cMasterSwitch;  /* Range:[0,1] *//* 开机log总开关1:开0:关 */
    unsigned char  cBufUsable;     /* Range:[0,1] *//* 开机log内存可用1:开0:关 */
    unsigned char  cBufEnable;     /* Range:[0,1] *//* 开机log内存使能1使能 0:不使能 */
    unsigned char  cswACPUBsp;     /* Range:[0,3] *//* ACPU Bsp开机log profile */
    unsigned char  cswACPUDiag;    /* Range:[0,3] *//* ACPU Diag开机log profile */
    unsigned char  cswACPUHifi;    /* Range:[0,3] *//* ACPU Hifi开机log profile */
    unsigned char  cswLRMBsp;      /* Range:[0,3] *//* 4G Modem Bsp开机log profile */
    unsigned char  cswLRMDiag;     /* Range:[0,3] *//* 4G Modem Diag开机log profile */
    unsigned char  cswLRMTLPhy;    /* Range:[0,3] *//* 4G Modem LPHY开机log profile */
    unsigned char  cswLRMGUPhy;    /* Range:[0,3] *//* 4G Modem GUPHY开机log profile */
    unsigned char  cswLRMCPhy;     /* Range:[0,3] *//* 4G Modem CPHY开机log profile */
    unsigned char  cswLRMEasyRf;   /* Range:[0,3] *//* 4G Modem easyRf开机log profile */
    unsigned char  cswNRMBsp;      /* Range:[0,3] *//* 5G Modem Bsp开机log profile */
    unsigned char  cswNRMDiag;     /* Range:[0,3] *//* 5G Modem CCPU开机log profile */
    unsigned char  cswNRMHAC;      /* Range:[0,3] *//* 5G Modem L2HAC开机log profile */
    unsigned char  cswNRMPhy;      /* Range:[0,3] *//* 5G Modem NRPHY开机log profile */
    unsigned char  cswNRMHL1C;     /* Range:[0,3] *//* 5G Modem HL1C开机log profile */
    unsigned char  cswNRMPDE;      /* Range:[0,3] *//* 5G Modem PDE开机log profile */
    unsigned char  cReserved[6];   /* 保留字段 */
}DRV_NV_POWER_ON_LOG_SWITCH_STRU;


typedef struct
{
    signed int temperature;
    unsigned short code;
    unsigned short reserved;
} DRV_CONVERT_TABLE_TYPE;
#define XO_TABLE_SIZE 166
typedef struct
{
    DRV_CONVERT_TABLE_TYPE convert_table[XO_TABLE_SIZE];
} DRV_XO_CONVERT_TABLE;
#define PA_TABLE_SIZE 32
typedef struct
{
    DRV_CONVERT_TABLE_TYPE convert_table[PA_TABLE_SIZE];
} DRV_PA_CONVERT_TABLE;


#ifndef LPHY_UT_MODE//lint !e553
typedef struct convert_table
{
    signed int temperature;
    unsigned short code;
    unsigned short reserved;
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
