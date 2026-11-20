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


#ifndef __DIAG_SYSTEM_DEBUG_H__
#define __DIAG_SYSTEM_DEBUG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include <product_config.h>
#include <mdrv.h>
#include <mdrv_diag_system.h>
#include <osl_types.h>
#include <bsp_slice.h>
#include <bsp_diag_frame.h>
#include <bsp_socp.h>

#pragma pack(push)
#pragma pack(4)


#define DIAG_DEBUG_START        (0xaaaa5555)
#define DIAG_DEBUG_END          (0x5555aaaa)

/* debug版本 V1.00 */
#define DIAG_DEBUG_VERSION      (0x00010000)

/* debug提示信息的长度 */
#define DIAG_DEBUG_INFO_LEN     (32)

/* 两次数据采集的时间间隔 */
#define DIAG_DEBUG_DEALAY       (5000)

/* 复杂的数据结构，存储信息前先存储信息长度，高8位是0xa5，低24位是数据长度 */
#define DIAG_DEBUG_SIZE_FLAG    (0xa5000000)


#define diag_system_printf(fmt, ...) \
    bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_DIAG_SYSTEM,"<%s %d> "fmt, __FUNCTION__,__LINE__, ##__VA_ARGS__)





/* PTR ***********************************************************************/
/*
初步估计每个诊断命令由10-20个点能覆盖整个处理流程，
1000个点能缓存50-100个诊断命令的处理流程
*/
#define DIAG_PTR_NUMBER             (2000)

typedef struct
{
    u16     enStep;     /*事件枚举*/
    u16     paraMark;   /*是否携带参数, 1携带参数 0不携带参数*/
    u32     ulTime;     /*当前时间*/
    u32     para[2];    /*携带的参数，仅支持0个或者2个，不支持其他*//*lint !e43*/
} DIAG_PTR_ELEMENT_STRU;

typedef struct
{
    u32              ulCur;
    DIAG_PTR_ELEMENT_STRU   stPtr[DIAG_PTR_NUMBER];
} DIAG_PTR_INFO_STRU;

/* 吞吐率统计 ***********************************************************************/
#define     DIAG_THRPUT_DEBUG_NUM       (100)       /* 缓存100个点 */

#define     DIAG_THRPUT_DEBUG_LEN       (5*32768)   /* 每次统计吞吐率最少间隔5秒以上 */

typedef struct
{
    unsigned int      ulSlice;            /* 当前时间戳时间戳 */
    unsigned int      ulThroughput;       /* 吞吐率(Bytes/s) */
}DIAG_THRPUT_NODE_STRU;

typedef struct
{
    unsigned int              ulSlice;            /* 最近一次统计的时间戳 */
    unsigned int              ulBytes;            /* 当前已累计的字节数 */
    unsigned int              ulMax;              /* 吞吐率峰值 */
    unsigned int              ulPtr;              /* 当前指针 */
    DIAG_THRPUT_NODE_STRU   stNode[DIAG_THRPUT_DEBUG_NUM];
}DIAG_THRPUT_INFO_STRU;


extern DIAG_MNTN_DST_INFO_STRU g_ind_dst_mntn_info;
extern u32              g_ulSendUSBStartSlice;

static inline void diag_system_debug_ind_dst_lost(u32 type, u32 len)
{
    g_ind_dst_mntn_info.ulDeltaLostTimes++;
    g_ind_dst_mntn_info.ulDeltaLostLen += len;
    g_ind_dst_mntn_info.aulCurFailNum[type]++;
    g_ind_dst_mntn_info.aulCurFailLen[type] += len;
}

static inline void diag_system_debug_rev_socp_data(u32 len)
{
    g_ind_dst_mntn_info.ulDeltaSocpLen += len;
}

static inline void  diag_system_debug_usb_len(u32 len)
{
    g_ind_dst_mntn_info.ulDeltaUSBLen += len;

}
static inline void  diag_system_debug_usb_ok_len(u32 len)
{
    g_ind_dst_mntn_info.ulDeltaUSBOKLen += len;

}

static inline void  diag_system_debug_usb_free_len(u32 len)
{
    g_ind_dst_mntn_info.ulDeltaUSBFreeLen += len;

}
static inline void  diag_system_debug_usb_fail_len(u32 len)
{
    g_ind_dst_mntn_info.ulDeltaUSBFailLen += len;
}

static inline void diag_system_debug_vcom_len(u32 len)
{
    g_ind_dst_mntn_info.ulDeltaVcomLen += len;

}
static inline void diag_system_debug_vcom_fail_len(u32 len)
{
    g_ind_dst_mntn_info.ulDeltaVcomFailLen += len;

}
static inline void diag_system_debug_vcom_sucess_len(u32 len)
{
    g_ind_dst_mntn_info.ulDeltaVcomSucessLen += len;

}
static inline void diag_system_debug_socket_len(u32 len)
{
    g_ind_dst_mntn_info.ulDeltaSocketLen += len;
}
static inline void diag_system_debug_socket_sucess_len(u32 len)
{
    g_ind_dst_mntn_info.ulDeltaSocketSucessLen += len;
}
static inline void diag_system_debug_socket_fail_len(u32 len)
{
    g_ind_dst_mntn_info.ulDeltaSocketFailLen += len;
}

static inline void diag_system_debug_send_data_end(void)
{
    u32 curent_slice;
    u32 start_slice;
    u32 delta;

    curent_slice = bsp_get_slice_value();
    start_slice = bsp_get_socp_ind_dst_int_slice();
    delta =  get_timer_slice_delta(start_slice, curent_slice);

    g_ind_dst_mntn_info.ulDeltaSocpIntToPortTime += delta;
}
static inline void diag_system_debug_send_usb_end(void)
{
    u32 curent_slice;
    u32 start_slice;
    u32 delta;

    curent_slice    = bsp_get_slice_value();
    start_slice     = g_ulSendUSBStartSlice;

    delta = get_timer_slice_delta(start_slice, curent_slice);
    g_ind_dst_mntn_info.ulDeltaUsbSendTime += delta;
}
static inline void diag_system_debug_send_usb_start(void)
{
    g_ulSendUSBStartSlice = bsp_get_slice_value();
}

/* 函数声明*******************************************/
void DIAG_DebugPTR(void);
void DIAG_DebugShowPTR(u32 ulnum);
void diag_PTR(DIAG_PTR_ID_ENUM enType, u32 paraMark, u32 para0, u32 para1);
u32 DIAG_SystemDebugFileHeader(u32 pFile);
void diag_ThroughputSave(DIAG_THRPUT_ID_ENUM enChn, u32 bytes);
void DIAG_ShowThroughput(u32 ulIndex);
u32 DIAG_GetThrputInfo(DIAG_THRPUT_ID_ENUM type);
void DIAG_SystemDebugFileTail(u32 pFile, s8 *FilePath);
s32 diag_system_debug_event_cb(unsigned int u32ChanID, SOCP_EVENT_ENUM_UIN32 u32Event, unsigned int u32Param);
void DIAG_Throughput(void);
void diag_debug_get_dst_mntn_info(DIAG_MNTN_DST_INFO_STRU * dst_mntn);
void diag_reset_dst_mntn_info(void);
void diag_save_soft_decode_info(void);

#pragma pack(pop)

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of diag_Debug.h */

