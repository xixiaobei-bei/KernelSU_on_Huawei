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

#ifndef __BSP_IQI_H__
#define __BSP_IQI_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__VXWORKS__) || defined(__OS_RTOSCK__) ||defined(__OS_RTOSCK_SMP__)
#include <mdrv_iqi.h>
#endif
#include <osl_types.h>
#include <osl_common.h>
#include <bsp_dump.h>
#include <bsp_icc.h>
#include <bsp_trace.h>


#define iqi_debug(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_DEBUG,   BSP_MODU_IQI, "<%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#define iqi_warning(fmt, ...)  (bsp_trace(BSP_LOG_LEVEL_WARNING, BSP_MODU_IQI, "<%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#define iqi_error(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_ERROR,   BSP_MODU_IQI, "<%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#define iqi_crit(fmt, ...)     (bsp_trace(BSP_LOG_LEVEL_CRIT,   BSP_MODU_IQI, "<%s> "fmt, __FUNCTION__, ##__VA_ARGS__))


#define IQI_ICC_CHANNID         (ICC_CHN_IQI << 16 | IQI_LOGIC_CHAN_AC)
#define IQI_IFC_CHANNID         (ICC_CHN_IFC << 16 | IFC_RECV_FUNC_IQI)


typedef struct
{
    u32 send_slice;         /*发送时间*/
    u32 msg_type;           /*消息类型*/
}iqi_icc_msg_t;

typedef enum tagIQI_MSG_OPEN_STATE
{
    IQI_MSG_OPEN        = 0x1,
    IQI_MSG_CLOSE       = 0x2,
    IQI_MSG_WRITEABLE   = 0x3,
    IQI_MSG_OPEN_COMPLETE = 0x5,
    IQI_MSG_CLOSE_COMPLETE   = 0x4,
    IQI_MSG_BUTT,
}IQI_MSG_OPEN_STATE;

typedef struct
{
    u32 master_bridge:1;
    u32 slave_bridge:1;
    u32 resered:30;
}iqi_bridge_mark_t;

typedef enum tagIQI_BRIDGE_STATE
{
    IQI_BRIDGE_CLOSE = 0,
    IQI_BRIDGE_OPEN = 1,
    IQI_BRIDGE_BUTT,
}IQI_BRIDGE_STATE;


#define IQI_OK                              (0)
#define IQI_ERROR                           (1)

#define BSP_ERR_IQI_BASE                    BSP_DEF_ERR(BSP_MODU_IQI, 0)

#define IQI_ERR_NV_CLOSE                    (BSP_ERR_IQI_BASE + 0x1)
#define IQI_ERR_CREATE_BACKEND_TASK         (BSP_ERR_IQI_BASE + 0x2)
#define IQI_ERR_CREATE_FRONTEND_TASK        (BSP_ERR_IQI_BASE + 0x3)
#define IQI_ERR_BUFFER_NOT_ENOUGH           (BSP_ERR_IQI_BASE + 0x4)
#define IQI_ERR_CHANNEL_ERROR               (BSP_ERR_IQI_BASE + 0x5)
#define IQI_ERR_ICC_SEND_ERROR              (BSP_ERR_IQI_BASE + 0x6)
#define IQI_ERR_ICC_READ_ERROR              (BSP_ERR_IQI_BASE + 0x7)
#define IQI_ERR_OPEN_PORT_FAIL              (BSP_ERR_IQI_BASE + 0x8)
#define IQI_ERR_CLOSE_PORT_FAIL             (BSP_ERR_IQI_BASE + 0x9)
#define IQI_ERR_CREATE_QUEUE_FAIL           (BSP_ERR_IQI_BASE + 0xA)
#define IQI_ERR_ADD_QUEUE_NODE_FAIL         (BSP_ERR_IQI_BASE + 0xB)
#define IQI_ERR_BUFF_ERROR                  (BSP_ERR_IQI_BASE + 0xC)
#define IQI_ERR_MALLOC_FAIL                 (BSP_ERR_IQI_BASE + 0xD)
#define IQI_ERR_SUBMIT_FAIL                 (BSP_ERR_IQI_BASE + 0xE)
#define IQI_ERR_NOT_ENGOUGH_SPACE           (BSP_ERR_IQI_BASE + 0xF)
#define IQI_ERR_WRITE_QUEU_FAIL             (BSP_ERR_IQI_BASE + 0x10)
#define IQI_ERR_TASK_CREATE_FAIL            (BSP_ERR_IQI_BASE + 0x11)
#define IQI_ERR_NOT_ACTIVE                  (BSP_ERR_IQI_BASE + 0x12)
#define IQI_ERR_REPEAT_ACTIVE               (BSP_ERR_IQI_BASE + 0x13)
#define IQI_ERR_EXCUTE_ERR                  (BSP_ERR_IQI_BASE + 0x14)
#define IQI_ERR_DELETE_TIMER_FAIL           (BSP_ERR_IQI_BASE + 0x15)
#define IQI_ERR_INIT_FIAL                   (BSP_ERR_IQI_BASE + 0x16)
#define IQI_ERR_BACKEND_SHUTDOWN_FAIL       (BSP_ERR_IQI_BASE + 0x17)
#define IQI_ERR_SHUTDOWN_NOT_COMMPLETE      (BSP_ERR_IQI_BASE + 0x18)
#define IQI_ERR_READ_NV_FAIL                (BSP_ERR_IQI_BASE + 0x19)
#define IQI_ERR_IO_ASYNC_PENDING            (BSP_ERR_IQI_BASE + 0x1A)
#define IQI_ERR_GET_IDLE_SPACE              (BSP_ERR_IQI_BASE + 0x1B)
#define IQI_ERR_ICC_MSG_ERROR               (BSP_ERR_IQI_BASE + 0x1C)
#define IQI_ERR_BRIDGE_RESET                (BSP_ERR_IQI_BASE + 0x1D)
#define IQI_ERR_METRIC_SWTICH_OFF           (BSP_ERR_IQI_BASE + 0x1E)
#define IQI_ERR_GET_TIME_STAMP_FAIL         (BSP_ERR_IQI_BASE + 0x1F)
#define IQI_ERR_ALLOC_MEM_FAIL              (BSP_ERR_IQI_BASE + 0x20)
#define IQI_ERR_MODIFY_SOFT_TIMER_FAIL      (BSP_ERR_IQI_BASE + 0x21)

#if defined(__VXWORKS__) || defined(__OS_RTOSCK__) ||defined(__OS_RTOSCK_SMP__)
/*****************************************************************************
* 函 数 名  :bsp_iqi_report_metric
* 功能描述  : 提供给外部组件上报消息接口
* 输入参数  : id、消息内容、长度
* 输出参数  : 无
* 返 回 值  : 是否成功标志
*****************************************************************************/
s32 bsp_iqi_report_metric( unsigned int metric_id, const void *payload,  unsigned long payload_size );
/*****************************************************************************
* 函 数 名  :bsp_iqi_report_metrict
* 功能描述  : 提供给外部组件上报消息带时间戳接口
* 输入参数  : id、时间戳、消息内容、长度
* 输出参数  : 无
* 返 回 值  : 是否成功标志
*****************************************************************************/
s32 bsp_iqi_report_metrict (unsigned int metric_id, IQI_TIMESTAMP_T *timestamp, const void *payload, unsigned long payload_size );
/*****************************************************************************
* 函 数 名  :bsp_iqi_report_metric_v
* 功能描述  : 提供给外部组件上报块式消息接口
* 输入参数  : id、消息块
* 输出参数  : 无
* 返 回 值  : 是否成功标志
*****************************************************************************/
s32 bsp_iqi_report_metric_v (unsigned int metric_id, const IQI_IOVEC_T *blocks );
/*****************************************************************************
* 函 数 名  :bsp_iqi_report_metric_vt
* 功能描述  : 提供给外部组件上报块式消息带时间戳接口
* 输入参数  : id、时间戳、消息块
* 输出参数  : 无
* 输出参数  : 无
* 返 回 值  : 是否成功标志
*****************************************************************************/
s32 bsp_iqi_report_metric_vt (unsigned int metric_id, IQI_TIMESTAMP_T *timestamp,const IQI_IOVEC_T *blocks );
/*****************************************************************************
* 函 数 名  :bsp_iqi_report_metric_vt
* 功能描述  : 提供给外部组件上报块式消息带时间戳接口
* 输入参数  : id、时间戳、消息块
* 输出参数  : 无
* 输出参数  : 无
* 返 回 值  : 是否成功标志
*****************************************************************************/
s32 bsp_iqi_get_timestamp(IQI_TIMESTAMP_T * timestamp);
/*****************************************************************************
* 函 数 名  :bsp_iqi_report_metric_vt
* 功能描述  : 提供给外部组件上报块式消息带时间戳接口
* 输入参数  : id、时间戳、消息块
* 输出参数  : 无
* 输出参数  : 无
* 返 回 值  : 是否成功标志
*****************************************************************************/
s32 bsp_iqi_get_metric_switch(void);
/*****************************************************************************
* 函 数 名      :mdrv_iqi_active
* 功能描述  : 激活IQI特性
* 输入参数  :
* 输出参数  : 无
* 返 回 值     : 是否成功标志
*****************************************************************************/
s32 iqi_active(void);
/*****************************************************************************
* 函 数 名      :iqi_deactive
* 功能描述  : 激活IQI特性
* 输入参数  :
* 输出参数  : 无
* 返 回 值     : 是否成功标志
*****************************************************************************/
s32 iqi_deactive(void);
#endif
/*****************************************************************************
* 函 数 名  :bsp_iqi_report_metric_vt
* 功能描述  : 提供给外部组件上报块式消息带时间戳接口
* 输入参数  : id、时间戳、消息块
* 输出参数  : 无
* 输出参数  : 无
* 返 回 值  : 是否成功标志
*****************************************************************************/
s32 bsp_get_plv_switch(void);


#ifdef __cplusplus
}
#endif

#endif

