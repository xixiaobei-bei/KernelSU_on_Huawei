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

#ifndef __DRV_NV_DEF_ACORE_H__
#define __DRV_NV_DEF_ACORE_H__

#include "acore_nv_id_drv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*
 * 结构名  : NV_SECBOOT_ENABLE_FLAG
 * ID说明  : ID=50201
 * 结构说明  : 标识软件版本是否需要开启安全特性，16字节的码流。
 */
typedef struct {
    /*
     * Range:[0,1]
     * 0：非安全加密版本，不执行EFUSE烧写；
     * 1：安全加密版本，可执行EFUSE烧写。
     */
    u16 usNVSecBootEnableFlag;
} NV_SECBOOT_ENABLE_FLAG;

/* USB Feature for usb id:50075 */
/* 结构说明  : USB软硬件特性控制NV项。仅支持MBB产品，Kirin960中只有P531-bbit形态产品使用。 */
typedef struct {
    /*
     * bit 0：wwan_flags。判断PC网卡是否为无线网卡。
     * 0：no；1：yes。
     * bit 1：bypass_mode。选择网络模式。
     * 0：stick模式；1：E5 模式。
     * bit 2：vbus connect，vbus连接芯片的vbus引脚（通常为0，部分车载为1）。
     * 0：disconnect；1：connect。
     * bit 3：sys_err_on_disable。USB重枚举可维可测，请勿开启。
     * 0：no；1：yes。
     * bit 4：phy_auto_pd。不使用的usb phy，随枚举完成关闭特性，不能和hibernation_support同时使用。
     * bit5：force_highspeed。USB是否强制配置USB2.0和切换到SGMII功能（SGMII仅V765支持）。
     * 0：no；1：yes。
     * bit6：usb shell。是否使用usb shell。
     * 0：no；1：yes。
     * bit7：resv。
     */
    u8 flags;
    /*
     * bit 0: 0：usb正插，1：usb强制反插
     * bit 1: 0：linux rndis网卡使用默认2K组包水线，1：linux rndis网卡使用16K水线
     */
    u8 reserve1;
    /*
     * usb hibernation功能。支持usb hibernation可以令acore在usb连接时进入睡眠。
     * 0：no；1：yes。
     */
    u8 hibernation_support;
    /*
     * PC驱动的类型。
     * 0；ECM网卡需要使用此驱动; 1：NCM网卡需要使用此驱动；2：保留; 3: 部分Linux PC原生ECM驱动需要支持上报subclass 0x3。
     */
    u8 pc_driver;
    /*
     * USB插拔检测模式。
     * 0：pmu detect 通过PMU检测USB插拔；
     * 1：no detect 不进行检测，USB保持常开；
     * 2：car module detect车载模块T-box检测模式，GPIO检测插入，芯片vbus检测拔出。
     * 3: 对接linux pc
     */
    u8 detect_mode;
    /*
     * 部分版本的usb ip存在在使用USB3.0的U1U2省电模式时会重枚举的芯片逻辑问题，为了规避这个问题添加此宏。
     * 0：关闭u1u2功能；
     * 1：使能u1u2，并使能synopsis提供的规避方案；
     * 2：使能u1u2功能，不使用规避方案。
     */
    u8 enable_u1u2_workaround;
    /*
     * 使用gpio端口进行usb id高低电平检测。用于OTG模式中的形态识别。
     * 0：关闭检测；
     * 1：打开检测。
     */
    u8 usb_gpio_support : 4;
    /*
     * USB的模式选择。
     * 0：device模式；
     * 1：host模式；
     * 2：otg模式。
     */
    u8 usb_mode : 4;
    /*
     * bit[0:3]:网卡模式选择。
     * 0：NCM网卡自动协商模式，可以与主机协商为nth16或者nth32；
     * 1：RNDIS网卡模式；
     * 2：ECM网卡模式；
     * 3：NCM网卡强制模式，强制单板支持nth16。
     * bit[4:7]:网卡个数选择，默认值0，使用1个网卡，只有stick模式下的NCM/ECM支持最多4个网卡。
     * 0：默认值，使用1个网卡；
     * 1/2/3/4：枚举对应个数网卡。
     */
    u8 network_card;
} NV_USB_FEATURE;

/* SPE Feature for usb id HEX:0xd12c DEC:53548 */
/* 结构说明：内部使用 */
typedef struct {
    u32 smp_sch : 1;
    u32 smp_hp : 1;
    u32 mulwan : 1;
    u32 ipf_extdesc : 1;
    u32 ack_extdesc : 1;
    u32 direct_fw : 1;
    u32 reserve : 26;
} DRV_SPE_FEATURE;

/* C核单独复位按需开关特性配置NV项 = 0xd134
 * 结构说明  : 控制C核单独复位特性是否打开，以及是否与接收来自ril的单独复位请求。
 */
typedef struct {
    /*
     * 是否开启C核单独复位特性。
     * 0：关
     * 1：开
     */
    u32 is_feature_on : 1;
    /*
     * 是否接收来自ril的C核单独复位请求。
     * 0：否
     * 1：是
     */
    u32 is_connected_ril : 1;
    u32 reserve : 30; /* bit 2-31 */
} DRV_CCORE_RESET_STRU;

/* NVID=0xd144 begin */
typedef struct {
    u32 mdm_dump : 1;     /* modem_dump.bin */
    u32 mdm_sram : 1;     /* modem_sram.bin */
    u32 mdm_share : 1;    /* modem_share.bin */
    u32 mdm_ddr : 1;      /* modem_ddr.bin */
    u32 lphy_tcm : 1;     /* lphy_tcm.bin */
    u32 lpm3_tcm : 1;     /* lpm3_tcm.bin */
    u32 ap_etb : 1;       /* ap_etb.bin */
    u32 mdm_etb : 1;      /* modem_etb.bin */
    u32 reset_log : 1;    /* reset.log */
    u32 cphy_tcm : 1;     /* cphy_tcm.bin */
    u32 mdm_secshare : 1; /* secshare.bin */
    u32 mdm_dts : 1;      /* modem_dts.bin */
    u32 llram_share : 1;  /* modem_llram.bin */
    u32 reserved2 : 19;
} DUMP_FILE_BITS;

/*
 * 结构说明  : 可维可测设置当前需要保存的异常文件NV。
 * 该NV在B5000平台修改为可定制属性，终端按需修改定制；
 */
typedef struct {
    union {
        u32 file_value;
        /*
         * bit0：是否保存modem_dump.bin异常文件。
         * bit1：是否保存modem_sram.bin。
         * bit2：是否保存modem_share.bin异常文件。
         * bit3：是否保存modem_ddr.bin异常文件。
         * bit4：是否保存lphy_tcm.bin异常文件。
         * bit5：是否保存lpm3_tcm.bin异常文件。
         * bit6：是否保存ap_etb.bin异常文件。
         * bit7：是否保存modem _etb.bin异常文件。
         * bit8: 是否保存reset.log文件
         * bit9：是否保存cphy_tcm.bin异常文件。
         * bit10：是否保存modem_secshare.bin异常文件。
         * 其它保留。
         */
        DUMP_FILE_BITS file_bits;
    } file_list;
    u32 file_cnt; /* 记录异常文件份数 */
} DUMP_FILE_CFG_STRU;
/* 结构说明：控制总线防挂死检测功能是否开启以及开启后是否复位 */
typedef struct {
    u32 enable;       /* 总线挂死寄存器解析使能标志, 0, 去使能;  1, 使能 */
    u32 reset_enable; /* 复位使能标志, 0, 去使能; 1, 使能 */
} DRV_PDLOCK_AP_CFG_STRU;

/* dlock  0xd155 */
/* 结构说明：废弃 */
typedef struct {
    u32 enable;       /* 总线挂死寄存器解析使能标志, 0, 去使能;  1, 使能 */
    u32 reset_enable; /* 复位使能标志, 0, 去使能; 1, 使能 */
} DRV_DLOCK_CFG_STRU;

/* 0xD14d
 * 结构说明  : 控制NV log是否保存，NV异常时处理的方式。
 */
typedef struct {
    /*
     * 区分当前版本为MBB产品还是Phone产品。
     * 0：Phone产品；
     * 1：MBB产品。
     */
    u32 product : 2;
    /*
     * 如果NV出错是否需要复位单板。
     * 0：不复位；
     * 1：复位。
     */
    u32 reset : 1;
    /*
     * 在备份分区数据出错的情况下是否保存备份分区数据
     * 0：不保存
     * 1：保存
     */
    u32 save_bakup : 1;
    /*
     * 在工作分区数据出错的情况下是否保存备份分区数据
     * 0：不保存
     * 1：保存
     */
    u32 save_image : 1;
    /*
     * 在NV DDR数据出错的情况下是否保存备份分区数据
     * 0：不保存
     * 1：保存
     */
    u32 save_ddr : 1;
    /*
     * 在NV DDR 数据出错且工作分区数据仍然不正确的的情况下是否进行紧急恢复
     * 0：不恢复
     * 1：进行恢复
     */
    u32 resume_bakup : 1;
    /*
     * 在NV DDR 数据出错的情况下是否进行紧急恢复
     * 0：不恢复
     * 1：进行恢复
     */
    u32 resume_img : 1;
    u32 reserved : 24; /* 预留。 */
} NV_DEBUG_CFG_STRU;

/* NV ID 0xd157 DVS使能控制及hpm读值配置
 * [NV elf编译项目] 结构定义重复 xuziheng/306651 20171209
 */
#ifndef NV_DEFINE
typedef struct {
    u32 dvs_en;
    u32 hpm_delay;
} ST_DVS_CONFIG_STRU;
#endif

/* 结构说明  : mbb产品控制noc err检测功能是否打开以及noc异常是否复位（phone形态关闭，mbb形态打开）。 */
typedef struct {
    u32 enable;       /* mbb软件noc异常检测功能是否打开，0：关闭；1：打开 */
    u32 reset_enable; /* Mbb noc中断处理后是否复位，0：不复位；1：复位 */
} DRV_NOC_AP_CFG_STRU;
/* 结构说明：控制总线检测功能是否开启以及开启后是否复位 */
typedef struct {
    u32 enable;       /* mbb ap侧bus err probe解析使能标志, 0, 去使能;  1, 使能 */
    u32 reset_enable; /* mbb ap侧复位使能标志, 0, 去使能; 1, 使能 */
} DRV_BUS_ERR_AP_CFG_STRU;

/*
 * 结构说明  : 本NV项的功能是实现DMSS中断处理流程的使能控制和触发DMSS中断后是否复位的控制；
 * 本NV项使用在有DMSS模块的系统中，目前只支持V765平台，手机平台不支持。
 */
typedef struct {
    u32 enable; /* enable用于使能DMSS中断处理流程，若要使能，配置enable为0x1，不使能配置为0x0 */
    u32 reset_enable; /* reset_enable用于控制触发DMSS中断后是否复位，如要复位，配置reset_enable为0x1，不复位则配置为0x0
                       */
} DRV_DMSS_CFG_STRU;

/* amon监控ID NV配置 0xd16e */
typedef struct {
    u32 id;
    u32 id_enable; /* 是否启动该id的监控功能 */
    u32 port;      /* 监控端口 */
    u32 master_id; /* 监控masterid */
    char master_name[0x10];
    u32 start_addr; /* 监控起始地址 */
    u32 end_addr;
    u32 opt_type; /* 监控的操作类型，01:读，10:写，11:读写，其余值:不监控 */
    u32 cnt_type; /* 复位标志，匹配到监控条件是否复位 */
} AMON_AP_NV_ID_CFG;

typedef struct {
    u32 amon_enable;
    AMON_AP_NV_ID_CFG id_cfg[0x8];
} AMON_AP_NV_CFG;

/* 结构说明  : 内部使用 */
typedef struct {
    AMON_AP_NV_CFG amon_nv_cfg[1];
} DRV_AMON_AP_NV_CFG_STRU;

/* socp rate ctrl */
/* 结构说明  : NV项含义或功能描述：SOCP限流信息NV。 */
typedef struct {
    u32 chan_enable;      /* 限流功能是否打开 */
    u32 rate_limits;      /* 所有通道总的限流上限，单位MB/s */
    u32 rev;              /* 保留 */
    u32 chan_rate_bits_l; /* SOCP 源通道低32个通道限流使能位 */
    u32 chan_rate_bits_h; /* SOCP 源通道高32个通道限流使能位 */
} DRV_DIAG_RATE_STRU;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
