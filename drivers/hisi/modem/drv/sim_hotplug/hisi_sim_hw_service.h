/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#ifndef __HISI_SIM_SERVICE_H
#define __HISI_SIM_SERVICE_H

#include <bsp_icc.h>
#include "mdrv_msg.h"

#ifdef CONFIG_EXTRA_MODEM_SIM
#define SIM_ICC_TO_MODEM ICC_CPU_APLUSB
#define SIM0_CHANNEL_ID ((ICC_CHN_APLUSB_IFC << 16) | APLUSB_IFC_FUNC_SIM0)
#define SIM1_CHANNEL_ID ((ICC_CHN_APLUSB_IFC << 16) | APLUSB_IFC_FUNC_SIM1)
#else
#define SIM_ICC_TO_MODEM ICC_CPU_MODEM
#define SIM0_CHANNEL_ID ((ICC_CHN_IFC << 16) | IFC_RECV_FUNC_SIM0)
#define SIM1_CHANNEL_ID ((ICC_CHN_IFC << 16) | IFC_RECV_FUNC_SIM1)
#endif

typedef struct {
    u32 msg_type;
    u32 msg_value;
    u32 msg_sn;
    u32 time_stamp;
} sci_msg_data_s;

struct sci_msg_pkt {
    struct msg_head hdr;
    sci_msg_data_s content;
};

typedef enum {
    SIM_MSG_HOTPLUG_TYPE = 0x0,
    SIM_MSG_INQUIRE_TYPE = 0x1,
    SIM_MSG_POWER_TYPE = 0x2,
    SIM_MSG_NOTIFIER_TYPE = 0x3,
    SIM_MSG_SWITCH_TYPE = 0x4,
    SIM_MSG_LEGACY_TYPE = 0x5,
    SIM_MSG_TYPE_BUTT
} sci_msg_type_e;

typedef enum {
    SIM_CARD_TYPE_INQUIRE = 0x1,           /**< 查询当前SIM卡类型 */
    SIM_CARD_DET_WHILE_ESIM_INQUIRE = 0x2, /**< 请求ESIM时实卡插入 */
} sci_msg_inquire_type_e;

typedef enum {
    SIM_POWER_ON_CLASS_C_REQUEST = 0x1, /**< 请求设置1.8V电压 */
    SIM_POWER_ON_CLASS_B_REQUEST = 0x2, /**< 请求设置3.0V电压 */
    SIM_POWER_OFF_REQUEST = 0x3,        /**< 请求关闭电源 */
    SIM_POWER_MSG_BUTT
} sci_msg_power_type_e;

typedef enum {
    SIM_DEACTIVE_NOTIFIER = 0x1, /**< 通知去激活操作 */
} sci_msg_notifier_type_e;

typedef enum {
    SIM_SET_ESIM_SWITCH = 0x1, /**< 切换ESIM */
    SIM_SET_USIM_SWITCH = 0x2, /**< 切换USIM */
} sci_msg_switch_type_e;

typedef enum {
    SIM_INVALID_INQUIRE_MSG = 0x0,
    SIM_CARD_TYPE_SIM = 0x1,            /**< 卡类型为SIM卡 */
    SIM_CARD_TYPE_MUXSIM = 0x2,         /**< 卡类型为SIM卡 */
    SIM_CARD_TYPE_SD = 0x3,             /**< 卡类型为SD */
    SIM_CARD_TYPE_JTAG = 0x4,           /**< 卡类型为JTAG */
    SIM_CARD_TYPE_FAIL = 0x5,           /**< 卡类型查询失败 */
    SIM_CARD_EXIST_WHILE_ESIM = 0x6,    /**< ESIM时副卡在位 */
    SIM_CARD_NO_EXIST_WHILE_ESIM = 0x7, /**< ESIM时副卡不在位 */
    SIM_MSG_INQUIRE_FAIL = 0x8,
} sci_msg_inquire_result_e;

typedef enum {
    SIM_INVALID_POWER_MSG = 0x0,
    SIM_POWER_ON_SUCC = 0x1,  /**< 上电成功 */
    SIM_POWER_ON_FAIL = 0x2,  /**< 上电失败 */
    SIM_POWER_OFF_SUCC = 0x3, /**< 下电成功 */
    SIM_POWER_OFF_FAIL = 0x4, /**< 下电失败 */
    SIM_MSG_POWER_INVALID = 0x5,
} sci_msg_power_result_e;

typedef enum {
    SIM_INVALID_NOTIFIER_MSG = 0x0,
    SIM_DEACTIVE_SUCC = 0x1, /**< 去激活准备 */
    SIM_DEACTIVE_FAIL = 0x2, /**< 去激活失败 */
} sci_msg_notifier_result_e;

typedef enum {
    SIM_INVALID_SWITCH_MSG = 0x0,
    SIM_ESIM_SWITCH_SUCC = 0x1, /**< 切换ESIM成功 */
    SIM_ESIM_SWITCH_FAIL = 0x2, /**< 切换ESIM失败 */
    SIM_USIM_SWITCH_SUCC = 0x3, /**< 切换USIM成功 */
    SIM_USIM_SWITCH_FAIL = 0x4, /**< 切换USIM失败 */
    SIM_MSG_SWITCH_INVALID = 0x5,
} sci_msg_switch_result_e;

typedef enum {
    SIM_INVALID_LEGACY_MSG = 0x0,
    SIM_LEGACY_MSG_SUCC = 0x1, /**< legacy消息处理成功 */
    SIM_LEGACY_MSG_FAIL = 0x2, /**< legacy消息处理失败 */
    SIM_MSG_LEGACY_INVALID = 0x3,
} sci_msg_legacy_result_e;

#define SIM_HW_OPS_PER_TYPE 0x10
#define SIM_HW_ERR_CODE_PER_TYPE 0x100

struct hisi_sim_platform_info *hisi_sim_get_platinfo(unsigned int sim_id);

#endif /* __HISI_SIM_HW_SERVICE_H */
