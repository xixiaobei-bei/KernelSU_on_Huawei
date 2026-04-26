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
#ifndef __MDRV_VCOM_AGENT_H__
#define __MDRV_VCOM_AGENT_H__
#include <mdrv_vcom.h>

#ifdef  __cplusplus
#if  __cplusplus
extern "C"{
#endif
#endif

/**************************结构体定义***********************/
/**
* @brief vcom接收回调函数类型
*/
typedef int (*com_rx_cb_f)(enum com_id_e id, const unsigned char* data, unsigned int size);

/**
* @brief vcom发送回调函数类型
*/
typedef int (*com_tx_cb_f)(enum com_id_e id, unsigned char* data, unsigned int size);

/**************************函数定义*************************/
/**
 * @brief vcom接收回调函数注册接口
 *
 * @par 描述:
 * vcom接收回调函数注册接口
 *
 * @attention
 * <ul><li>NA。</li></ul>
 *
 * @param[in]  id , 通道id
 * @param[in]  func , 回调函数。
 *
 * @retval 0, 成功。
 * @retval !0, 失败。
 * @par 依赖:
 * <ul><li>mdrv_vcom.h：该接口声明所在的头文件。</li></ul>
 * <ul><li>mdrv_vcom_agent.h：该接口声明所在的头文件。</li></ul>
 *
 * @see enum com_id_e
 */
int mdrv_com_rxcb_reg(enum com_id_e id, com_rx_cb_f func);

/**
 * @brief vcom发送回调函数注册接口
 *
 * @par 描述:
 * vcom发送回调函数注册接口
 *
 * @attention
 * <ul><li>NA。</li></ul>
 *
 * @param[in]  id , 通道id
 * @param[in]  func , 回调函数。
 *
 * @retval 0, 成功。
 * @retval !0, 失败。
 * @par 依赖:
 * <ul><li>mdrv_vcom.h：该接口声明所在的头文件。</li></ul>
 * <ul><li>mdrv_vcom_agent.h：该接口声明所在的头文件。</li></ul>
 *
 * @see enum com_id_e
 */
int mdrv_com_txcb_reg(enum com_id_e id, com_tx_cb_f func);

/**
 * @brief vcom发送数据接口
 *
 * @par 描述:
 * vcom发送数据接口
 *
 * @attention
 * <ul><li>NA。</li></ul>
 *
 * @param[in]  id , 通道id
 * @param[in]  data , 数据基地址。
 * @param[in]  s , 数据长度字节数
 *
 * @retval 0, 成功。
 * @retval !0, 失败。
 * @par 依赖:
 * <ul><li>mdrv_vcom.h：该接口声明所在的头文件。</li></ul>
 * <ul><li>mdrv_vcom_agent.h：该接口声明所在的头文件。</li></ul>
 *
 * @see enum com_id_e
 */
int mdrv_com_write(enum com_id_e id, const unsigned char* data, unsigned int s);

#ifdef  __cplusplus
#if  __cplusplus
}
#endif
#endif

#endif
