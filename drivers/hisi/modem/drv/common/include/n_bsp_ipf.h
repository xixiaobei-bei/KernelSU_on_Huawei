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

#ifndef    _BSP_IPF_H_
#define    _BSP_IPF_H_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**************************************************************************
  头文件包含
**************************************************************************/
#include "hi_ipf.h"
#include <bsp_shared_ddr.h>
#ifndef __CMSIS_RTOS
#include "mdrv_ipf.h"

#else
#include "mdrv_ipf_comm.h"
#endif


#include <bsp_trace.h>
#include <bsp_dump.h>
#ifdef CONFIG_BALONG_MODEM_RESET
#include "mdrv_sysboot.h"
#endif

/***************************************/
#define SET1(reg,type,a1,b1)        ((type##_T*)&reg)->bits.a1 = (b1)
#define SET2(reg,type,a1,b1,a2,b2)     SET1(reg,type,a1,b1);SET1(reg,type,a2,b2)
#define SET3(reg,type,a1,b1,a2,b2,a3,b3)     SET2(reg,type,a1,b1,a2,b2);SET1(reg,type,a3,b3)
#define SET4(reg,type,a1,b1,a2,b2,a3,b3,a4,b4)     SET3(reg,type,a1,b1,a2,b2,a3,b3);SET1(reg, type,a4,b4)
#define SET5(reg,type,a1,b1,a2,b2,a3,b3,a4,b4,a5,b5)     SET4(reg,type,a1,b1,a2,b2,a3,b3,a4,b4);SET1(reg, type,a5,b5)

#define UPDATE0(reg, type)    do{                        \
                            reg = ipf_readl(type##_OFFSET);   \
                            ipf_writel(reg, type##_OFFSET); \
                            (void)reg;	\
                        }while(0)

#define UPDATE1(reg, type, a1, b1)    do{                        \
                            reg = ipf_readl(type##_OFFSET);   \
                            SET1(reg, type,a1,b1);                  \
                            ipf_writel(reg, type##_OFFSET); \
                            (void)reg;	\
                        }while(0)
#define UPDATE2(reg, type, a1, b1, a2, b2)    do{                        \
                            reg = ipf_readl(type##_OFFSET);   \
                            SET2(reg, type,a1, b1, a2, b2);                  \
                            ipf_writel(reg, type##_OFFSET); \
                            (void)reg;	\
                        }while(0)
#define UPDATE3(reg, type, a1, b1, a2, b2, a3, b3)    do{                        \
                            reg = ipf_readl(type##_OFFSET);   \
                            SET3(reg, type,a1, b1, a2, b2, a3, b3);                  \
                            ipf_writel(reg, type##_OFFSET); \
                            (void)reg;	\
                        }while(0)
#define UPDATE4(reg, type, a1, b1, a2, b2, a3, b3, a4, b4)    do{                        \
                            reg = ipf_readl(type##_OFFSET);   \
                            SET4(reg, type,a1, b1, a2, b2, a3, b3,a4,b4);                  \
                            ipf_writel(reg, type##_OFFSET); \
                            (void)reg;	\
                        }while(0)
#define UPDATE5(reg, type, a1, b1, a2, b2, a3, b3, a4, b4, a5, b5)    do{                        \
							reg = ipf_readl(type##_OFFSET);   \
							SET5(reg, type,a1,b1,a2,b2,a3,b3,a4,b4,a5,b5);				   \
							ipf_writel(reg, type##_OFFSET); \
							(void)reg;	\
						}while(0)


/***************************************/


/* #define __BSP_IPF_DEBUG__*/

#define IPF_MEMBLOCK_CNT                (20)

/*IPF描述符地址定义*/

#define IPF_SHAREMEM_BASE				(unsigned long)((unsigned long)SHM_BASE_ADDR + SHM_OFFSET_IPF)

#define IPF_DLBD_MEM_SIZE				(IPF_DLBD_DESC_SIZE * sizeof(ipf_bd_s))
#define IPF_DLRD_MEM_SIZE				(IPF_DLRD_DESC_SIZE * sizeof(ipf_rd_s))
#define IPF_DLAD0_MEM_SIZE				(IPF_DLAD0_DESC_SIZE * sizeof(ipf_ad_s))
#define IPF_DLAD1_MEM_SIZE				(IPF_DLAD1_DESC_SIZE * sizeof(ipf_ad_s))
#define IPF_DLCD_MEM_SIZE               (IPF_DLCD_DESC_SIZE * sizeof(ipf_cd_s))
#define IPF_ULBD_MEM_SIZE               (IPF_ULBD_DESC_SIZE * sizeof(ipf_bd_s))
#define IPF_ULRD_MEM_SIZE               (IPF_ULRD_DESC_SIZE * sizeof(ipf_rd_s))
#define IPF_ULAD0_MEM_SIZE				(IPF_ULAD0_DESC_SIZE * sizeof(ipf_ad_s))
#define IPF_ULAD1_MEM_SIZE				(IPF_ULAD1_DESC_SIZE * sizeof(ipf_ad_s))
#define IPF_PWRCTL_BASIC_FILTER_SIZE	(IPF_BF_NUM * sizeof(ipf_filter_node_s))
#define IPF_EXT_FILTER_SIZE				(EXFLITER_NUM * sizeof(ipf_filter_node_s))
#define IPF_PWRCTL_INFO_SIZE			128
#define IPF_PWRCTL_ADQ_WPTR_INFO_SIZE	32
#define IPF_PWRCTL_ADQ_RPTR_CHCTRL_SIZE	64
#define IPF_PWRCTL_ADDR_LIMIT_SIZE		64
#define IPF_INIT_SIZE					16
#define IPF_ULBD_IDLENUM_SIZE			16
#define IPF_DLCDRPTR_MEM_SIZE			sizeof(unsigned int)
#define IPF_DEBUG_INFO_SIZE				(2*sizeof(struct ipf_debug))
#define IPF_TRANS_LIMIT_SIZE			32
#define IPF_OUT_ADDR_LIMIT_SIZE			sizeof(unsigned int)
#define IPF_ADDR_MEMBLOCK_SIZE			(IPF_MEMBLOCK_CNT*4)
#define IPF_DLDESC_SIZE					(IPF_DLBD_MEM_SIZE + IPF_DLRD_MEM_SIZE + IPF_DLAD0_MEM_SIZE + IPF_DLAD1_MEM_SIZE + IPF_DLCD_MEM_SIZE)
#define EXFLITER_NUM					(IPF_TOTAL_FILTER_NUM-IPF_BF_NUM_EX) /* 扩展过滤器数目 */
#define IPF_DISABLE_FILTER				0  /* filter disable*/

#define IPF_BF_NUM						64  /* 基本过滤器数目 */
#define IPF_TOTAL_FILTER_NUM            256  /* 过滤器总数目先使用一半 */
#define IPF_TAIL_INDEX					511  /* filter标识最后一个filter */

#define IPF_BF_NUM_EX					128  /* 基本过滤器数目 */
#define IPF_TOTAL_FILTER_NUM_EX			IPF_TOTAL_FILTER_NUM  /* 过滤器总数目先使用一半 */
#define IPF_TAIL_INDEX_EX				1023  /* filter标识最后一个filter */

#define TIME_OUT_CFG					65           /* 默认超时时间,时钟频率166MHZ下为0.1ms */
#define IPF_BDWPTR_MUX					0x100  /*BD写指针互斥量*/
#define IPF_INT_OPEN0					0x80400C03        /*上行通道中断屏蔽*/
#define IPF_INT_OPEN1					0x3D030000 /*下行通道中断屏蔽*/

#define IPF_CHANNEL_STATE_IDLE       	0x14 /*通道状态IDLE*/
#define IPF_ADQ_BUF_EPT_MASK            0x00000004
#define IPF_ADQ_BUF_FULL_MASK           0x00000008
#define IPF_ULADQ_PLEN_TH               404
#define IPF_ACORE_INIT_SUCCESS          0x10000

#define IPF_UL_RPT_INT0					0x1            /*上行结果上报中断0*/
#define IPF_UL_RDQ_FULL					(0x1 << 8)

#define IPF_UL_TIMEOUT_INT0             0x2            /*上行结果上报超时中断0*/
#define IPF_UL_ADQ0_EPTY_INT0			0x400         /*上行ADQ0空中断0*/
#define IPF_UL_ADQ1_EPTY_INT0			0x800         /*上行ADQ1空中断0*/
#define	IPF_UL_BDQ_EPTY_INT1			0x200			/*Upload BD empty interrupt*/

#define IPF_DL_RPT_INT1					0x10000       /*下行结果上报中断0*/
#define IPF_DL_RDQ_FULL					(0x1 << 24)

#define IPF_DL_TIMEOUT_INT1             0x20000       /*下行结果上报超时中断0*/
#define IPF_DL_RD_FULL_INT1         	0x1000000	  /*下行rd满中断1*/
#define IPF_DL_ADQ0_EPTY_INT1			0x4000000    /*下行ADQ0空中断0*/
#define IPF_DL_ADQ1_EPTY_INT1			0x8000000    /*下行ADQ1空中断0*/

#define SCH_ALL_DESC					1
#define	IPF_TIMER_RATIO					(32768)					/*one second*/
#define	IPF_RD_TMOUT					(2 * IPF_TIMER_RATIO)   /*must less than 16 second*/
#define	IPF_LEN_RATIO					(1024*1024)
#define MAX_PM_OM_SAVE_SIZE				40				/*dpm first packet print cnt*/

#define SAVE_NONE						0
#define SAVE_PASS						1
#define SAVE_DONE						2
#define IPF_VER_120A					(0x31323061)
#define IPF_VER_130A					(0x31333061)
#define IPF_VER_130B					(0x31333062)
#define IPF_VER_140A					(0x31343061)
#define IPF_VER_150A    				(0x31353061)
#define IPF_VER_160    				    (0x31363061)
#define IPF_VER_170    				    (0x31373061)

#define IPF_PWR_UP						0xb
#define IPF_PWR_DOWN					0xa
#define IPF_FAST_TIMEOUT    			1


#define IPF_REG_BAK_NUM		0x100
#define IPF_FILTER_BAK_NUM		0x20*128

#ifdef __KERNEL__
#define IPF_PRINT     printk
#else
#define IPF_PRINT	SRE_Printf
#endif

/*******************************/
typedef union ipf_desc_attr
{
	struct
	{
		unsigned short	int_en			: 1;    //lint !e46
		unsigned short	mode			: 2;    //lint !e46
		unsigned short	cd_en			: 1;    //lint !e46
		unsigned short	dl_direct_set	: 1;    //lint !e46
		unsigned short	fc_head			: 4;    //lint !e46
		unsigned short	reserved		: 7;    //lint !e46
	} bits;

	unsigned short u16;
}ipf_desc_attr_t;

/* RD描述符 */
typedef struct
{
    ipf_desc_attr_t u16Attribute;
    unsigned short u16PktLen;
    unsigned int u32InPtr;
    unsigned int u32OutPtr;
    unsigned short u16Result;
    unsigned short u16UsrField1;
    unsigned int u32UsrField2;
    unsigned int u32UsrField3;
    unsigned int u32UsrField4;
    unsigned int u32UsrField5;
}ipf_rd_s;

/* BD描述符 */
typedef struct
{
    ipf_desc_attr_t u16Attribute;
    unsigned short u16PktLen;
    unsigned int u32InPtr;
    unsigned int u32OutPtr;
    unsigned short u16Result;
    unsigned short u16UsrField1;
    unsigned int u32UsrField2;
    unsigned int u32UsrField3;
    unsigned int u32UsrField4;
    unsigned int u32UsrField5;
} ipf_bd_s;

typedef struct
{
    unsigned int u32OutPtr0;
    unsigned int u32OutPtr1;
} ipf_ad_s;

typedef struct
{
    unsigned short u16Attribute;
    unsigned short u16PktLen;
    unsigned int u32Ptr;
}ipf_cd_s;

union ipf_addr{
    struct{
        unsigned long long ptr  :48;        //lint !e46
        unsigned long long reserved :16;    //lint !e46
    }bits;
    unsigned long long addr;
};

typedef struct
{
    ipf_desc_attr_t attribute;
    unsigned short pkt_len;
    union ipf_addr input_ptr;
    unsigned short reserved;
	unsigned short user_field1;
    unsigned int user_field2;
    unsigned int user_field3;
	unsigned int user_field4;
	unsigned int user_field5;
    union ipf_addr output_ptr;
} __attribute__((packed, aligned(4))) ipf64_bd_s;

typedef struct
{
    ipf_desc_attr_t attribute;
    unsigned short pkt_len;
    union ipf_addr input_ptr;
    unsigned short result;
	unsigned short user_field1;
	unsigned int user_field2;
	unsigned int user_field3;
	union {
		struct{
    		unsigned int field4;
    		unsigned int field5;
		}user;
		unsigned long long ptr;
	} virt;
    union ipf_addr output_ptr;
} __attribute__((packed, aligned(4))) ipf64_rd_s;

typedef struct
{
	unsigned long long blk_len	:15;        //lint !e46
	unsigned long long cd_last	:1;         //lint !e46
	unsigned long long input_ptr	:48;    //lint !e46
} __attribute__((packed, aligned(4))) ipf64_cd_s;

typedef struct
{
    union ipf_addr output_ptr0;
    union ipf_addr output_ptr1;
} __attribute__((packed, aligned(4))) ipf64_ad_s;

typedef struct {
    unsigned char u8SrcAddr[16];
    unsigned char u8DstAddr[16];
    unsigned char u8DstMsk[16];
    union{
        struct{
        	unsigned int u16SrcPortLo:16;
        	unsigned int u16SrcPortHi:16;
        }Bits;
	    unsigned int u32SrcPort;
    } unSrcPort;
    union{
        struct
        {
            unsigned int u16DstPortLo:16;
        	unsigned int u16DstPortHi:16;
        }Bits;
    	unsigned int u32DstPort;
    } unDstPort;
    union{
        struct
        {
            unsigned int u8TrafficClass:8;
        	unsigned int u8TrafficClassMask:8;
        	unsigned int u16Reserve:16;
        }Bits;
    	unsigned int u32TrafficClass;
    }unTrafficClass;
    union{
        struct
        {
            unsigned int fltn_local_addr_mask:7;
        	unsigned int Reserve:25;
        }Bits;
    	unsigned int u32LocalAddressMsk;
    }unLocalAddressMsk;
    union{
        struct
        {
            unsigned int    fltn_next_header:8;
            unsigned int    reserved:24;
        }u32NextHeader;
    	unsigned int u32Protocol;
    }unNextHeader;
    union
    {
        struct
        {
            unsigned int    fltn_fow_lable             : 20; /* [19..0] 过滤表流标签配置寄存器，字节小端排列 */
            unsigned int    reserved                   : 12; /* [31..20] 保留。 */
        } bits;
        unsigned int    u32;
    }u32FlowLable;
    union{
        struct{
            unsigned int    fltn_type                  : 8; /* [7..0] 过滤器type域配置值 */
            unsigned int    reserved_1                 : 8; /* [15..8] 保留。 */
            unsigned int    fltn_code                  : 8; /* [23..16] 过滤器code域配置值 */
            unsigned int    reserved_0                 : 8; /* [31..24] 保留。 */
        }Bits;
    	unsigned int u32CodeType;
    } unFltCodeType;
    union{
        struct{
            unsigned int    fltn_next_index            : 10; /* [9..0] 下一个过滤器对应的index */
            unsigned int    reserved_1                 : 6; /* [15..10] 保留。 */
            unsigned int    fltn_pri                   : 9; /* [24..16] 过滤器优先级，fltn_pri数值越小优先级越高 */
            unsigned int    reserved_0                 : 7; /* [31..25] 保留。 */
        }Bits;
    	unsigned int u32FltChain;
    } unFltChain;
    unsigned int u32FltSpi;
    union{
    	struct{
    		unsigned int FltEn:1;
    		unsigned int FltType:1;
    		unsigned int Resv1:2;
            unsigned int FltSpiEn:1;
            unsigned int FltCodeEn:1;
            unsigned int FltTypeEn:1;
            unsigned int FltFlEn:1;
            unsigned int FltNhEn:1;
            unsigned int FltTosEn:1;
            unsigned int FltRPortEn:1;
            unsigned int FltLPortEn:1;
            unsigned int FltRAddrEn:1;
            unsigned int FltLAddrEn:1;
            unsigned int Resv2:2;
            unsigned int FltBid:6;
            unsigned int Resv3:10;
    	}Bits;
    	unsigned int u32FltRuleCtrl;
    }unFltRuleCtrl;

}ipf_filter_node_s;


enum ipf_version{
	IPF_VERSION_0000  = 0x0,
	IPF_VERSION_120a  = 0x1,		
	IPF_VERSION_130a  = 0x2,
	IPF_VERSION_130b  = 0x3,
	IPF_VERSION_140a  = 0x4,
	IPF_VERSION_150a  = 0x5,
	IPF_VERSION_160   = 0x6,
	IPF_VERSION_170   = 0x7,
	IPF_VERSTION_BOTTOM
};

typedef enum ipf_enable
{
    ipf_disable, 
    ipf_enable, 
    ipf_en_bottom
}ipf_enable_t;

typedef enum tagIPF_FORRESET_CONTROL_E
{
    IPF_FORRESET_CONTROL_ALLOW = 0xabcd1234,     /*允许发送*/
    IPF_FORRESET_CONTROL_FORBID,    /*禁止发送*/
    IPF_FORRESET_CONTROL_MAX
}IPF_FORRESET_CONTROL_E;

#ifdef CONFIG_MPERF
struct ipf_runtime_info{
    u32 eclapse_time;
    u32 bd_cfg_cnt;
    u32 rd_rcv_cnt;
    u32 adq0_depth;
    u32 adq1_depth;
    u32 bdq_depth;
    u32 rdq_depth;
    u32 byte_cnt;
};
#endif

typedef struct ipf_ddr {
    unsigned int    start;
    unsigned int    ul_start;
    unsigned int    filter_pwc_start;
    unsigned int    pwc_info_start;
    unsigned int    dlcdrptr;
    unsigned int    debug_dlcd_start;
    unsigned int    debug_dlcd_size;
    unsigned int    end;
    unsigned int    len;
} ipf_ddr_t;

struct ipf_debug
{
    unsigned int    init_ok;
    unsigned int    mdrv_called_not_init;
    unsigned int    ad_thred;
    unsigned int    flt_chain_loop;
    unsigned int    get_rd_times;
    unsigned int    get_rd_num_times;
    unsigned int    get_rd_cnt[IPF_AD_MAX];
    unsigned int    get_ad_num_times;
    unsigned int    cfg_ad_times;
    unsigned int    cfg_ad_cnt[IPF_AD_MAX];
    unsigned int    ad_out_ptr_null[IPF_AD_MAX];
    unsigned int    busy_cnt;
    unsigned int    occupy_cnt;
    unsigned int    get_bd_num_times;
    unsigned int    cfg_bd_times;
    unsigned int    cfg_bd_cnt;
    unsigned int    bd_full;
    unsigned int    bd_len_zero_err;
    unsigned int    cd_not_enough;
    unsigned int    suspend;
    unsigned int    resume;
    unsigned int    resume_with_intr;
    unsigned int    timestamp_en;
    unsigned int    rd_len_update;
    unsigned int    rd_len;
    unsigned int    rd_ts;
    unsigned int    rate_en;
    unsigned int    dump_mem_alloc_ok;
    unsigned int    invalid_para;
    ipf_ddr_t   share_ddr_info;
    unsigned int    reg_scur_wr_err;
    unsigned int    reg_scur_rd_err;
    unsigned int    ccore_rst_err;
    unsigned int    ccore_reset;
    unsigned int    core_rst_done;
    unsigned int    cp_flag;
    unsigned int    rst_ts;
    unsigned int    rsr_suspend_begin;
    unsigned int    rsr_suspend_end;
    unsigned int    rsr_resume_begin;
    unsigned int    rsr_resume_end;
};

#ifdef __KERNEL__
struct ipf_desc_handler_s{
    char* name;
	unsigned int bdq_rptr_offset;
	unsigned int bdq_wptr_offset;
	void (*config)(void);
	void (*cd_en_set)(void* cd_base, unsigned int index, unsigned short en);
	int (*cd_en_get)(void* cd_base, unsigned int index);
	u64 dma_mask;
	void* (*cd_pos_get)(void* cd_base, unsigned int index);
	void (*cd_s2h)(void* header, void* cd_base, unsigned int index);
	void (*cd_clear)(void* cd_base, unsigned int index);
	void (*acpu_wake_ccpu)(void);
	void (*bd_s2h)(IPF_CONFIG_PARAM_S* param, void* bd_base, unsigned int index);
	void (*bd_h2s)(IPF_CONFIG_PARAM_S* param, void* bd_base, unsigned int index);
	void (*rd_h2s)(IPF_RD_DESC_S* param, void* bd_base, unsigned int index);
	void (*rd_s2h)(IPF_RD_DESC_S* param, void* bd_base, unsigned int index);
	int (*ad_s2h)(IPF_AD_TYPE_E type, unsigned int n, IPF_AD_DESC_S * param);
	void (*ad_h2s)(IPF_AD_DESC_S* param, void* ad_base, unsigned int index);
	void (*cd_last_set)(void* cd_base, unsigned int index, unsigned short en);
	unsigned int (*cd_last_get)(void* cd_base, unsigned int index);
	void (*chn_cfg)(int dir, IPF_CHL_CTRL_S *pstCtrl);
	void (*ad_threshold_cfg)(unsigned int ad0, unsigned int ad1);
	void (*enable_chn)(IPF_CHANNEL_TYPE_E eChanType, int bFlag);
	unsigned int (*get_dlrd_num)(void);
	unsigned int (*get_ulrd_num)(void);
	void (*get_rd)(unsigned int* pu32Num, IPF_RD_DESC_S *pstRd);
	void (*get_ad_num)(unsigned int* AD0Num,unsigned int* AD1Num);
	unsigned int (*get_bd_num)(void);
	void (*config_bd)(unsigned int u32Num, IPF_CONFIG_ULPARAM_S* pstUlPara);
	int (*is_busy)(void);
    void (*save_reg)(unsigned int*);
    void (*restore_reg)(unsigned int*);
    void (*dump)(void);
#ifdef CONFIG_MPERF
    unsigned int (*get_rd_num)(void);
#endif
};
#else
struct ipf_desc_handler_s{
    char* name;
    unsigned int bdq_rptr_offset;
    unsigned int bdq_wptr_offset;
    void (*config)(void);
    void* (*cd_pos_get)(void* cd_base, unsigned int index);
    void (*cd_s2h)(void* header, void* cd_base, unsigned int index);
    void (*cd_h2s)(IPF_CD_DESC_S* para, void* cd_base, unsigned int index);
    int (*bd_s2h)(IPF_CONFIG_PARAM_S* param, void* bd_base, unsigned int index, unsigned int wake);
    void (*bd_h2s)(IPF_CONFIG_PARAM_S* param, void* bd_base, unsigned int index);
    void (*rd_h2s)(IPF_RD_DESC_S* param, void* bd_base, unsigned int index);
    void (*rd_s2h)(IPF_RD_DESC_S* param, void* bd_base, unsigned int index);
    int (*ad_s2h)(IPF_AD_TYPE_E type, unsigned int n, IPF_AD_DESC_S * param);
    void (*ad_h2s)(IPF_AD_DESC_S* param, void* ad_base, unsigned int index);
    void (*cd_last_set)(void* cd_base, unsigned int index, unsigned short en);
    unsigned int (*cd_last_get)(void* cd_base, unsigned int index);
    void (*chn_cfg)(int dir, IPF_CHL_CTRL_S *pstCtrl);
    void (*ad_threshold_cfg)(unsigned int ad0, unsigned int ad1);
    void (*enable_chn)(IPF_CHANNEL_TYPE_E eChanType, int bFlag);
    unsigned int (*get_ulrd_num)(void);
    void (*get_rd)(unsigned int* pu32Num, IPF_RD_DESC_S *pstRd);
    void (*get_ad_num)(unsigned int* AD0Num,unsigned int* AD1Num);
    unsigned int (*get_bd_num)(void);
    void (*config_bd)(unsigned int u32Num, IPF_CONFIG_ULPARAM_S* pstUlPara);
    int (*is_busy)(void);
    void (*save_reg)(unsigned int*);
    void (*restore_reg)(unsigned int*);
    void (*timeout)(void);
    void (*rx)(void);
    void (*dump)(void);
    void (*ccpu_wake_acpu)(void);
#ifdef CONFIG_MPERF
    unsigned int (*get_rd_num)(void);
#endif
};
#endif
typedef void (*ipf_bd_empty)(void);
typedef int (*BSP_IPF_WakeupUlCb)(void);
typedef int (*BSP_IPF_AdqEmptyUlCb)(IPF_ADQ_EMPTY_E eAdqEmpty);

typedef struct tagIPF_DL_S
{
    void* pstIpfBDQ;
    void* pstIpfRDQ;
    void* pstIpfADQ0;
    void* pstIpfADQ1;
    void* pstIpfCDQ;
	
#ifdef __KERNEL__
    BSP_IPF_WakeupDlCb pFnDlIntCb; /* 中断中唤醒的PS任务 */
    BSP_IPF_AdqEmptyDlCb pAdqEmptyDlCb;/*下行AD空回调，A核心*/

/*ACore硬件使用的物理地址*/
    dma_addr_t pstIpfPhyBDQ;
    dma_addr_t pstIpfPhyRDQ;
    dma_addr_t pstIpfPhyADQ0;
    dma_addr_t pstIpfPhyADQ1;
#endif
    unsigned int *u32IpfCdRptr; /* 当前可以读出的位置 */
    unsigned int u32IpfCdWptr;
    unsigned int u32IdleBd; /* 记录上一次获取的空闲BD 数 */
}IPF_DL_S;

typedef struct tagIPF_LIMIT_ADDR{
	unsigned int block_sum;
	unsigned int block1_start;
	unsigned int block1_end;
	unsigned int block2_start;
	unsigned int block2_end;
	unsigned int block_err;
}IPF_LIMIT_ADDR_S;

typedef struct filter_map_node
{
    unsigned int ps_id;
    unsigned int index;
    struct filter_map_node* next;
}filter_map;

struct ipf_limit_handler{
    void (*region_set)(unsigned int region, unsigned int start, unsigned int end);
    void (*secctrl_set)(unsigned int ctrl);
    unsigned int (*secctrl_get)(void);
    void (*default_set)(unsigned int outlimit);
    void (*sideband_set)(unsigned int sideband);
};

typedef union
{
	ipf_bd_s	bd32[IPF_DLBD_DESC_SIZE];
	ipf64_bd_s	bd64[IPF_DLBD_DESC_SIZE];
}ipf_dbd_u;

typedef union
{
	ipf_bd_s	bd32[IPF_ULBD_DESC_SIZE];
	ipf64_bd_s	bd64[IPF_ULBD_DESC_SIZE];
}ipf_ubd_u;

typedef union
{
	ipf_rd_s	rd32[IPF_ULRD_DESC_SIZE];
	ipf64_rd_s	rd64[IPF_ULRD_DESC_SIZE];
}ipf_urd_u;

typedef union
{
	ipf_rd_s	rd32[IPF_DLRD_DESC_SIZE];
	ipf64_rd_s	rd64[IPF_DLRD_DESC_SIZE];
}ipf_drd_u;

typedef union
{
	ipf_ad_s	ad32[IPF_ULAD0_DESC_SIZE];
	ipf64_ad_s	ad64[IPF_ULAD0_DESC_SIZE];
}ipf_uad_u;

typedef union
{
	ipf_ad_s	ad32[IPF_DLAD0_DESC_SIZE];
	ipf64_ad_s	ad64[IPF_DLAD0_DESC_SIZE];
}ipf_dad_u;

typedef union
{
	ipf_cd_s	cd32[IPF_DLCD_DESC_SIZE];
	ipf64_cd_s	cd64[IPF_DLCD_DESC_SIZE];
}ipf_dcd_u;

enum ipf_pm_status
{
    ipf_flag_up = 0,
    ipf_flag_down = 1,
    ipf_flag_reset = 2,
};

struct ipf_share_mem_map
{
    ipf_dcd_u dcd;
    unsigned int ipf_ccore_reg_size;
    unsigned int ipf_acore_reg_size;
    unsigned int psam_ccore_reg_size;
    unsigned int psam_acore_reg_size;
    unsigned int reg_save[IPF_REG_BAK_NUM];
    union{
        unsigned char init[IPF_INIT_SIZE];
        struct {
            unsigned int acore;
            unsigned int ccore;
            unsigned int modem;
            unsigned int save;
        } status;
    }init;
    int rsracc_valid;
    unsigned int idle;
    unsigned int dcd_rptr;
    struct ipf_debug debug[2];
    unsigned char trans_limit[IPF_TRANS_LIMIT_SIZE];
    unsigned int out_addr_limit;
    unsigned char memlock[IPF_MEMBLOCK_CNT*4];
    
//    ipf_ubd_u ubd;
    ipf_drd_u drd;
//    ipf_dad_u dad0;
//    ipf_dad_u dad1;
    
    unsigned int bottom[0];     //lint !e43
}__attribute__((packed, aligned(1)));

#define IPF_ACORE_MAP(sm)   0
#define PSAM_ACORE_MAP(sm)  (sm->ipf_acore_reg_size)
#define IPF_CCORE_MAP(sm)   (sm->ipf_acore_reg_size + sm->psam_acore_reg_size)
#define PSAM_CCORE_MAP(sm)  (sm->ipf_acore_reg_size + sm->psam_acore_reg_size + sm->ipf_ccore_reg_size)

#define bsp_ipf_get_sharemem() /*lint -save -e826 */(struct ipf_share_mem_map*)(((unsigned char*)SHM_BASE_ADDR + SHM_OFFSET_IPF))/*lint -restore */

struct ipf_mannul_unit{
    char* name;
    char* desc;
};

typedef union 
{
	u64 addr;
	struct
	{
		u32 addr_l;
		u32 addr_h;
	} u32_t;
}phy_addr_uion;

static inline void phy_addr_write(u64 phy_addr, void* reg_h, void* reg_l)
{
	phy_addr_uion pa;

	pa.addr = phy_addr;

	writel(pa.u32_t.addr_h, reg_h);
	writel(pa.u32_t.addr_l, reg_l);
}

static inline u64 phy_addr_read(void* reg_h, void* reg_l)
{
	phy_addr_uion pa;

	pa.u32_t.addr_h = readl(reg_h);
	pa.u32_t.addr_l = readl(reg_l);

	return pa.addr;
}

static inline void phy_addr_write32(u64 phy_addr,void* reg_l)
{
	phy_addr_uion pa;

	pa.addr = phy_addr;

	writel(pa.u32_t.addr_l, reg_l);
}

static inline u64 phy_addr_read32(void* reg_l)
{
	phy_addr_uion pa;

	pa.u32_t.addr_l = readl(reg_l);

	return pa.addr;
}

static inline void phy_addr_write64(u64 phy_addr, void* reg_l)
{
	phy_addr_uion pa;

	pa.addr = phy_addr;

	writel(pa.u32_t.addr_h, (unsigned char*)reg_l+4);
	writel(pa.u32_t.addr_l, reg_l);
}

static inline u64 phy_addr_read64(void* reg_l)
{
	phy_addr_uion pa;

	pa.u32_t.addr_h = readl((unsigned char*)reg_l+4);
	pa.u32_t.addr_l = readl(reg_l);

	return pa.addr;
}
#define U32_TO_ADDR(x) (modem_phy_addr)(((unsigned long)x)&0xffffffff)
#define ADDR_TO_U32(x) ((unsigned int)(unsigned long)x)

int bsp_ipf_srest(void);
void bsp_ipf_set_debug_para(void ** psam_get_debug);
void bsp_set_init_status(int stat);
int bsp_get_init_status(void);
#ifdef CONFIG_MPERF
int ipf_perf_info(struct ipf_runtime_info* info);
#endif
#if defined(__OS_VXWORKS__) || defined(__OS_RTOSCK__)||defined(__OS_RTOSCK_SMP__)
int ipf_config_cd(void* pstTtf, unsigned short* pu16TotalDataLen, unsigned int* pu32BdInPtr);
void bsp_ipf_dl_rpt1_switch(int value);
int ipf_init(void);
IPF_FORRESET_CONTROL_E bsp_ipf_get_control_flag_for_ccore_reset(void);
void ipf_clear_ptr(void);
void bsp_ipf_stop(void);
#endif

#ifdef __cplusplus
} /* allow C++ to use these headers */
#endif /* __cplusplus */

#endif    /* End #define _BSP_DMAC_H_ */

