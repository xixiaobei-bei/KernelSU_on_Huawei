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

#ifndef __BSP_DIAG_H__
#define __BSP_DIAG_H__

#include <product_config.h>
#include <stdarg.h>
#include "osl_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push)
#pragma pack(4)

/* 单帧最大长度 */
#define DIAG_FRAME_MAX_LEN (4 * 1024)
/* 单消息最大帧个数 */
#define DIAG_FRMAE_MAX_CNT (16)
/* 总长度最大值 */
#define DIAG_FRAME_SUM_LEN (DIAG_FRAME_MAX_LEN * DIAG_FRMAE_MAX_CNT)

/* 4字节对齐 */
#define ALIGN_DDR_WITH_4BYTE(len) (((len) + 3) & (~3))
/* 8字节对齐 */
#define ALIGN_DDR_WITH_8BYTE(len) (((len) + 7) & (~7))

#define DIAG_PRINT_HEAD_SIZE (sizeof(diag_print_ind_head_s))
#define DIAG_TRANS_HEAD_SIZE (sizeof(diag_trans_ind_head_s))

#define DIAG_HEAD_LEN sizeof(diag_frame_head_s) + sizeof(diag_data_head_s)
#define DIAG_SERVICE_HEAD_LEN sizeof(diag_service_head_s)
#define DIAG_MSG_HEAD_LEN sizeof(diag_msg_head_s)
#define DIAG_BSP_HEAD_LEN sizeof(diag_data_head_s)

#define DIAG_MEM_ALLOC_LEN_MAX (8 * 1024)

/* diag ps print最大允许字节数,包括前面文件名和行号长度，-1是预留\0结束符 */
#define DIAG_PS_PRINT_MAX_LEN (1000 - 1)
/* diag drv print最大允许字节数 */
#define DIAG_DRV_PRINT_MAX_LEN (255)

#define DIAG_HDS_NAME ("acpu[0]")
#define DIAG_DRV_HDS_PID (0x8003)

#define DIAG_SOCP_HEAD_MAGIC (0x48495349) /* HISI */
#define DIAG_SERVICE_HEAD_VER(data) (((diag_service_head_s *)data)->ver_4b)
#define DIAG_SERVCIE_GET_REAL_LEN(len) ((len)&0xffff)

/* diag_service_head_s:sessionid_8b */
#define MSP_SERVICE_SESSION_ID (0x1) /* 标识Service与Client之间的连接,固定为1 */

/* diag_service_head_s:ssid_4b */
typedef enum {
#ifdef DIAG_SYSTEM_5G
    DIAG_FRAME_SSID_DEFAULT = 0x0, /* 0x00：未指定（仅下发REQ使用，表示由UE确定分发目标，CNF和IND不能使用该值） */
    DIAG_FRAME_SSID_APP_CPU = 0x1,      /* 0x01：A-CPU */
    DIAG_FRAME_SSID_MODEM_CPU = 0x2,    /* 0x02：2G/3G/4.5G子系统C-CPU（底软、PS使用） */
    DIAG_FRAME_SSID_TLDSP_BBE_NX = 0x3, /* 0x03：BBE NX（TL DSP使用） */
    DIAG_FRAME_SSID_BBP_DEBUG = 0x4,    /* 0x04：2G/3G/4.5G BBP Debug   //原来是LTE BBP */
    DIAG_FRAME_SSID_GUC_BBE_NX = 0x5, /* 0x05：BBE NX（GUC DSP SDR使用）  //因GUC PHY与TL PHY没有融合，仍需单独分配 */
    DIAG_FRAME_SSID_HIFI = 0x6,
    DIAG_FRAME_SSID_LTE_V_DSP = 0x7, /* 0x07：LTE-V DSP（预留）//原来是TDS DSP */
    DIAG_FRAME_SSID_RESERVE0 = 0x8,
    DIAG_FRAME_SSID_MCU = 0x9,
    DIAG_FRAME_SSID_TEE = 0xA,      /* 0x0A：TEE        //原来是GPU */
    DIAG_FRAME_SSID_RESERVE1 = 0xB, /* 0x0B：保留       //原来是GUX BBP */
    DIAG_FRAME_SSID_IOM3 = 0xC,
    DIAG_FRAME_SSID_EASYRF0 = 0xD,
    DIAG_FRAME_SSID_X_DSP = 0xE,
    DIAG_FRAME_SSID_GUC_L1C = 0xE, /* 0x0E：2G/3G/4.5G子系统C-CPU(GUC L1C使用) */
    DIAG_FRAME_SSID_RESERVE2 = 0xF,
    DIAG_FRAME_SSID_5G_CCPU = 0x10,
    DIAG_FRAME_SSID_L2HAC = 0x11,
    DIAG_FRAME_SSID_HL1C = 0x12,
    DIAG_FRAME_SSID_LL1C_CORE0 = 0x13,
    DIAG_FRAME_SSID_LL1C_CORE1 = 0x14,
    DIAG_FRAME_SSID_LL1C_CORE2 = 0x15,
    DIAG_FRAME_SSID_LL1C_CORE3 = 0x16,
    DIAG_FRAME_SSID_LL1C_CORE4 = 0x17,
    DIAG_FRAME_SSID_LL1C_CORE5 = 0x18,
    DIAG_FRAME_SSID_SDR_CORE0 = 0x19,
    DIAG_FRAME_SSID_SDR_CORE1 = 0x1A,
    DIAG_FRAME_SSID_SDR_CORE2 = 0x1B,
    DIAG_FRAME_SSID_SDR_CORE3 = 0x1C,
    DIAG_FRAME_SSID_5G_BBP_DEBUG = 0x1D,
    DIAG_FRAME_SSID_EASYRF1 = 0x1E,
    DIAG_FRAME_SSID_BBP_ACCESS_DEBUG = 0x1F,
    DIAG_FRAME_SSID_BUTT,
#else
    DIAG_FRAME_SSID_APP_CPU = 0x1,
    DIAG_FRAME_SSID_MODEM_CPU,
    DIAG_FRAME_SSID_LTE_DSP,
    DIAG_FRAME_SSID_LTE_BBP,
    DIAG_FRAME_SSID_GU_DSP,
    DIAG_FRAME_SSID_HIFI,
    DIAG_FRAME_SSID_TDS_DSP,
    DIAG_FRAME_SSID_TDS_BBP,
    DIAG_FRAME_SSID_MCU,
    DIAG_FRAME_SSID_GPU,
    DIAG_FRAME_SSID_GU_BBP,
    DIAG_FRAME_SSID_IOM3,
    DIAG_FRAME_SSID_ISP,
    DIAG_FRAME_SSID_X_DSP,
    DIAG_FRAME_SSID_BUTT
#endif
} diag_frame_ssid_type_e;

/* diag_service_head_s:mt_2b */
typedef enum {
    DIAG_FRAME_MT_RSV = 0x0,
    DIAG_FRAME_MT_REQ = 0x1,
    DIAG_FRAME_MT_CNF = 0x2,
    DIAG_FRAME_MT_IND = 0x3
} diag_frame_msgtype_type_e;

/* MSP_DIAG_STID_STRU:pri_4b */
#ifdef DIAG_SYSTEM_5G
typedef enum {
    DIAG_FRAME_MSG_TYPE_RSV = 0x0,
    DIAG_FRAME_MSG_TYPE_MSP = 0x1,
    DIAG_FRAME_MSG_TYPE_PS = 0x2,
    DIAG_FRAME_MSG_TYPE_PHY = 0x3,
    DIAG_FRAME_MSG_TYPE_BBP = 0x4,
    DIAG_FRAME_MSG_TYPE_HSO = 0x5,
    DIAG_FRAME_MSG_TYPE_BSP = 0x9, /* MODEM BSP */
    DIAG_FRAME_MSG_TYPE_EASYRF = 0xa,
    DIAG_FRAME_MSG_TYPE_AP_BSP = 0xb, /* AP BSP */
    DIAG_FRAME_MSG_TYPE_AUDIO = 0xc,
    DIAG_FRAME_MSG_TYPE_APP = 0xe,
    DIAG_FRAME_MSG_TYPE_BUTT
} diag_message_type_e;
#else
typedef enum {
    DIAG_FRAME_MSG_TYPE_RSV = 0x0,
    DIAG_FRAME_MSG_TYPE_MSP = 0x1,
    DIAG_FRAME_MSG_TYPE_PS = 0x2,
    DIAG_FRAME_MSG_TYPE_PHY = 0x3,
    DIAG_FRAME_MSG_TYPE_BBP = 0x4,
    DIAG_FRAME_MSG_TYPE_HSO = 0x5,
    DIAG_FRAME_MSG_TYPE_BSP = 0x9,
    DIAG_FRAME_MSG_TYPE_ISP = 0xa,
    DIAG_FRAME_MSG_TYPE_AUDIO = 0xc,
    DIAG_FRAME_MSG_TYPE_APP = 0xe,
    DIAG_FRAME_MSG_TYPE_BUTT
} diag_frame_message_type_e;
#endif

/* MSP_DIAG_STID_STRU:mode_4b */
typedef enum {
    DIAG_FRAME_MODE_LTE = 0x0,
    DIAG_FRAME_MODE_TDS = 0x1,
    DIAG_FRAME_MODE_GSM = 0x2,
    DIAG_FRAME_MODE_UMTS = 0x3,
    DIAG_FRAME_MODE_1X = 0x4,
    DIAG_FRAME_MODE_HRPD = 0x5,

    DIAG_FRAME_MODE_NR = 0x6,
    DIAG_FRAME_MODE_LTEV = 0x7,

    DIAG_FRAME_MODE_COMM = 0xf,
} diag_frame_mode_type_e;

/* MSP_DIAG_STID_STRU:pri_4b */
typedef enum {
    DIAG_FRAME_MSG_CMD = 0x0,
    DIAG_FRAME_MSG_AIR = 0x1,
    DIAG_FRAME_MSG_LAYER = 0x2,
    DIAG_FRAME_MSG_PRINT = 0x3,
    DIAG_FRAME_MSG_EVENT = 0x4,
    DIAG_FRAME_MSG_USER = 0x5,
    DIAG_FRAME_MSG_VOLTE = 0x6,
    DIAG_FRAME_MSG_STRUCT = 0x7,
    DIAG_FRAME_MSG_DOT = 0x8,
    DIAG_FRAME_MSG_DSP_PRINT = 0x9,
    DIAG_FRAME_MSG_CNF = 0xa,
    DIAG_FRAME_MSG_IND = 0xb,
    DIAG_FRAME_MSG_DT = 0xF,
    DIAG_FRAME_MSG_STAT = 0x1f
} diag_frame_msg_sub_type_e;

typedef enum {
    DIAG_FRAME_MODEM_0 = 0x0,
    DIAG_FRAME_MODEM_1 = 0x1,
    DIAG_FRAME_MODEM_2 = 0x2
} diag_frame_modem_id_e;

/* diag_service_head_s:sid_8b */
typedef enum {
    DIAG_FRAME_MSP_SID_DEFAULT = 0x0,
    DIAG_FRAME_MSP_SID_AT_SERVICE,
    DIAG_FRAME_MSP_SID_DIAG_SERVICE,
    DIAG_FRAME_MSP_SID_DATA_SERVICE,
    DIAG_FRAME_MSP_SID_NV_SERVICE,
    DIAG_FRAME_MSP_SID_USIM_SERVICE,
    DIAG_FRAME_MSP_SID_DM_SERVICE,
    DIAG_FRAME_MSP_SID_CBT_SERVICE,
    DIAG_FRAME_MSP_SID_BUTT
} diag_frame_sid_type_e;

typedef enum {
    DIAG_FRAME_MSP_VER_4G = 0,
    DIAG_FRAME_MSP_VER_5G = 1,
    DIAG_FRAME_MSP_VER_BUTT
} diag_frame_ver_type_e;

#ifdef DIAG_SYSTEM_5G
/* 描述 :5G 一级头: service头 */
typedef struct {
    u32 sid_4b : 4;   /* service id, value:DIAG_SID_TYPE */
    u32 ver_4b : 4;   /* version , value:DIAG_SERVICE_HEAD_VER_TYPE */
    u32 mdmid_3b : 3; /* modem id dfd */
    u32 rsv_5b : 5;
    u32 ssid_8b : 8; /* sub system id , DIAG_SSID_TYPE, CCPU/ACPU/BBE NX/Audio Dsp/LTE-V DSP..... */
    u32 mt_2b : 2;
    u32 index_4b : 4;
    u32 eof_1b : 1;
    u32 ff_1b : 1;

    u16 msg_trans_id;
    u16 socp_encdst_id; /* SOCP填写 */

    u32 timestamp[1];
} diag_service_head_s;
#else
/* 描述 :一级头: service头 */
typedef struct {
    u32 sid_8b : 8;
    u32 mdmid_3b : 3;
    u32 rsv_1b : 1;
    u32 ssid_4b : 4;
    u32 sessionid_8b : 8;
    u32 mt_2b : 2;
    u32 index_4b : 4;
    u32 eof_1b : 1;
    u32 ff_1b : 1;
    u32 msg_trans_id;
    u32 timestamp[2];
} diag_service_head_s;
#endif
/* 描述 :二级头: DIAG消息头 */
typedef struct {
    u32 cmdid_19b : 19;
    u32 sec_5b : 5;
    u32 mode_4b : 4;
    u32 pri_4b : 4;
} diag_cmdid_s;

typedef struct {
    u32 msg_len : 16;
    u32 resv : 8;
    u32 msg_ver : 8;
} diag_msg_len_s;

/* 描述 :三级头: 工具软件信息头，用于REQ/CNF消息 */
typedef struct {
    u32 auid;                    /* 原AUID */
    u32 sn;                      /* HSO分发，插件命令管理 */
    u8 data[0]; /* 参数的数据 */ /*lint !e43 */
} diag_data_head_s;

/* 描述 :整体帧结构 */
typedef struct {
    diag_service_head_s srv_head;

    union {
        u32 cmd_id; /* 结构化ID */
        diag_cmdid_s cmdid_stru;
    };

    union {
        u32 msg_len;
        diag_msg_len_s msg_len_stru;
    };

    u8 data[0]; /* lint !e43 */
} diag_frame_head_s;

typedef struct {
    u32 auid; /* 原AUID */
    u32 sn;   /* HSO分发，插件命令管理 */
    u32 result;
} diag_comm_cnf_s;

typedef struct {
    u32 hisi_magic; /* "HISI" */
    u32 data_len;   /* 数据长度 */
} diag_socp_head_s;

typedef struct {
    diag_socp_head_s socp_header;
    diag_frame_head_s frame_header;
} diag_srv_head_s;

typedef struct {
    u32 module;  /* 打印信息所在的模块ID */
    u32 level;   /* 输出级别 */
    u32 msg_num; /* IND标号 */
} diag_print_ind_head_s;

/* diag event report上报信息的结构体 */
typedef struct {
    u32 msg_num;  /* 序号 */
    u32 event_id; /* 消息或者事件ID,主要针对消息,空口,事件,普通打印输出时该成员为零 */
    u32 module;   /* 打印信息所在的模块ID */
    s8 data[0]; /* 用户数据缓存区 */ /*lint !e43 */
} diag_event_ind_head_s;

/* diag air report上报信息的结构体 */
typedef struct {
    u32 module;                      /* 源模块ID */
    u32 direction;                   /* 1: NET-->UE, 2: UE-->NET */
    u32 msg_num;                     /* 序号 */
    u32 msg_id;                      /* ID */
    s8 data[0]; /* 用户数据缓存区 */ /*lint !e43 */
} diag_air_ind_head_s;

/* diag volte report上报信息的结构体 */
typedef struct {
    u32 module;                      /* 源模块ID */
    u32 direction;                   /* 1: NET-->UE, 2: UE-->NET */
    u32 msg_num;                     /* 序号 */
    u32 msg_id;                      /* ID */
    s8 data[0]; /* 用户数据缓存区 */ /*lint !e43 */
} diag_volte_ind_head_s;

/* diag userplane report上报信息的结构体 */
typedef struct {
    u32 module;                      /* 源模块ID */
    u32 msg_num;                     /* 序号 */
    u32 msg_id;                      /* ID */
    s8 data[0]; /* 用户数据缓存区 */ /*lint !e43 */
} diag_user_plane_ind_head_s;

/* diag 结构化数据上报信息的结构体 */
typedef struct {
    u32 module;                      /* 源模块ID */
    u32 msg_id;                      /* ID */
    u32 msg_num;                     /* 序号 */
    s8 data[0]; /* 用户数据缓存区 */ /*lint !e43 */
} diag_trans_ind_head_s;

/* diag drive数据上报信息的结构体和TRANS相同 */
typedef struct {
    u32 module;                      /* 源模块ID */
    u32 msg_id;                      /* ID */
    u32 msg_num;                     /* 序号 */
    s8 data[0]; /* 用户数据缓存区 */ /*lint !e43 */
} diag_dt_ind_head_s;

/* diag trace report上报信息的结构体 */
typedef struct {
    u32 module;                      /* 源模块ID */
    u32 dest_module;                 /* 目的模块ID */
    u32 msg_num;                     /* 序号 */
    u32 msg_id;                      /* ID */
    s8 data[0]; /* 用户数据缓存区 */ /*lint !e43 */
} diag_layer_ind_head_s;

/* 打点类型消息头 */
typedef struct {
    diag_socp_head_s socp_header;
    diag_frame_head_s frame_header;
} diag_srv_log_id_head_s;

/* 打印类型消息头 */
typedef struct {
    diag_socp_head_s socp_header;
    diag_frame_head_s frame_header;
    diag_print_ind_head_s print_head;
} diag_srv_log_head_s;

/* trans 类型消息头 */
typedef struct {
    diag_socp_head_s socp_header;
    diag_frame_head_s frame_header;
    diag_trans_ind_head_s trans_header;
} diag_srv_trans_head_s;

/* trans 类型消息头 */
typedef struct {
    diag_socp_head_s socp_header;
    diag_frame_head_s frame_header;
    diag_dt_ind_head_s dt_header;
} diag_srv_dt_head_s;

/* event 类型消息头 */
typedef struct {
    diag_socp_head_s socp_header;
    diag_frame_head_s frame_header;
    diag_event_ind_head_s event_header;
} diag_srv_event_head_s;

/* air 类型消息头 */
typedef struct {
    diag_socp_head_s socp_header;
    diag_frame_head_s frame_header;
    diag_air_ind_head_s air_header;
} diag_srv_air_head_s;

/* volte 类型消息头 */
typedef struct {
    diag_socp_head_s socp_header;
    diag_frame_head_s frame_header;
    diag_volte_ind_head_s volte_header;
} diag_srv_volte_head_s;

/* layer/trace 类型消息头 */
typedef struct {
    diag_socp_head_s socp_header;
    diag_frame_head_s frame_header;
    diag_layer_ind_head_s trace_header;
} diag_srv_layer_head_s;

/* user 类型消息头 */
typedef struct {
    diag_socp_head_s socp_header;
    diag_frame_head_s frame_header;
    diag_user_plane_ind_head_s user_header;
} diag_srv_user_plane_head_s;

/* drv log上报消息头 */
typedef struct {
    diag_socp_head_s socp_header;
    diag_frame_head_s frame_header;
    diag_print_ind_head_s print_head;
} diag_srv_print_head_s;

/* 此结构体与OSA的MsgBlock对应，不能随意修改 */
#pragma pack(1)
typedef struct {
    u32 sender_cpu;
    u32 sender_pid;
    u32 receiver_cpu;
    u32 receiver_pid;
    u32 length;
    u32 msg_id;
    u8 data[0]; /*lint !e43 */
} diag_osa_msg_head_s;
#pragma pack()
typedef struct {
    u32 header_size; /* 数据头的长度 */
    void *header;    /* 数据头 */
    u32 data_size;   /* ucData的长度 */
    void *data;      /* 数据 */
} diag_msg_report_head_s;

#define DIAG_SOCP_HEAD_SIZE (sizeof(diag_socp_head_s))
#define DIAG_FRAME_HEAD_SIZE (sizeof(diag_frame_head_s))

#ifdef DIAG_SYSTEM_FUSION
typedef enum {
    DIAG_MSG_CONN_REQ = 1,
    DIAG_MSG_DISCONN_REQ = 2,
    DIAG_MSG_MSP_REQ = 3,
    DIAG_MSG_POWERON_LOG_QUERY = 4, /* 开机LOG查询命令 */
    DIAG_MSG_CONN_AUTH_REQ = 5,
    DIAG_MSG_CONFIG_TRANS = 6, /* 转发工具下发的命令消息 */
    DIAG_MSG_RX_READY = 7,     /* 工具Rx Ready命令 */
    DIAG_MSG_PORT_COMMAND = 8,
    DIAG_MSG_PORT_COMMAND_RESP = 9,
    DIAG_INNER_MSG_TYPE_BUTT
} diag_msg_type_e;

typedef enum {
    DIAG_PORT_MSG_GET_IND_MODE = 0,
    DIAG_PORT_MSG_SET_IND_MODE = 1,
    DIAG_PORT_MSG_GET_CPS_MODE = 2,
    DIAG_PORT_MSG_SET_CPS_MODE = 3,
    DIAG_PORT_MSG_GET_LOG_CFG = 4,
    DIAG_PORT_MSG_GET_LOG_PORT = 5,
    DIAG_PORT_MSG_SET_LOG_PORT = 6,
    DIAG_PORT_MSG_TYPE_BUTT
} diag_at_msg_type_e;

typedef enum {
    DIAG_AUTH_TYPE_DEFAULT = 0,
    DIAG_AUTH_TYPE_AUTHING = 1,
    DIAG_AUTH_TYPE_SUCCESS = 2,
    DIAG_AUTH_TYPE_BUTT
} diag_auth_type_e;
#endif

typedef void (*soft_decode_dst_cb)(const unsigned char *buf_addr, unsigned int size, const unsigned char *rb_addr,
    unsigned int rb_size);

u32 bsp_soft_decode_dst_proc_reg(soft_decode_dst_cb func);

#if defined(CONFIG_DIAG_SYSTEM)
s32 bsp_diag_report_log(u32 module_id, u32 pid, char *file_name, u32 line_num, const char *fmt, va_list arg);
#else
static inline s32 bsp_diag_report_log(u32 module_id, u32 pid, char *file_name, u32 line_num, const char *fmt,
    va_list arg)
{
    return 0;
}
#endif

s32 bsp_diag_trans_report(void *trans_msg);
u32 bsp_diag_set_log_port(u32 phy_port);
u32 bsp_diag_get_log_port(u32 *phy_port);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
