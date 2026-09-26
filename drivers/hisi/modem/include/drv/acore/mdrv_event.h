/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
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

/**
 *  @brief   msg模块在acore上的对外头文件
 *  @file    mdrv_event.h
 *  @author  wangzaiwei
 *  @version v1.0
 *  @date    2019.11.15
 *  @note
 *  -v1.0|2019.11.15|创建文件
 *  @since   DRV2.0
 */

#ifndef __MDRV_EVENT_H__
#define __MDRV_EVENT_H__
#ifdef __cplusplus
extern "C" {
#endif

/** @brief 在事件接收函数中，表示阻塞式接收 */
#define MDRV_EVENT_WAIT_FOREVER 0xFFFFFFFF

/** @brief 在事件接收函数中,对任意所需事件到达敏感 */
#define MDRV_EVENT_ANY 0x00000001

/** @brief 在事件接收函数中,对所有所需事件到达敏感 */
#define MDRV_EVENT_ALL 0x00000010

/**
 * @brief 事件发送接口
 *
 * @par 描述:
 * 对指定任务发送一个或者多个事件，event按照32bit使用，每个bit代表一个事件
 *
 * @attention
 * <ul><li>目标任务必须存在，且创建了队列后才能成功发送过去</li></ul>
 * <ul><li>Bit9事件 在协议栈任务上有专用用途，所以不要给协议栈任务发送Bit9事件</li></ul>
 *
 * @param[in]  task_id, 操作系统任务id，在kernel下即pid，可以从task_struct中的pid得到
 * @param[in]  event, 发送的事件，可以一次发送多个事件(多个bit)，不可为0
 *
 * @retval 0,发送成功
 * @retval 非0,发送失败。
 * @par 依赖:
 * <ul><li>mdrv_event.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_event_recv
 */
MODULE_EXPORTED int mdrv_event_send(unsigned task_id, unsigned event);
/**
 * @brief 事件接收接口
 *
 * @par 描述:
 * 使用当前任务接收一个或者多个事件，event按照bit使用，每个bit代表一个事件，事件意义由收发双方自行定义
 *
 * @attention
 * <ul><li>任务必须创建了队列后才能调用该接口收取事件</li></ul>
 * <ul><li>只能在任务上下文中使用，在禁止调度的任务上下文中不允许阻塞</li></ul>
 *
 * @param[in]  req_event, 要接收的事件集合，按bit使用，不可为0
 * @param[in]  opt, MDRV_EVENT_ANY/MDRV_EVENT_ALL，
 * 当MDRV_EVENT_ANY时，任一req_event中的事件到达，本函数满足返回条件
 * 当MDRV_EVENT_ALL时，所有req_event中的事件到达，本函数满足返回条件
 * 当opt为其他值时，行为非法
 * @param[in]  time_out, 超时时间，按照系统tick计数，kernel下单位为HZ(时间计算与系统相关)
 * 当超时时间为0时，代表非阻塞RECV,可以在CR和禁止调度的任务上下文中使用
 * 当超时时间为MDRV_EVENT_WAIT_FOREVER时，代表永不超时
 * 当超时时间为其他值时，按照tick计数，若计时到达前事件满足，则正常返回，否则超时返回
 * @param[out]  rsp_event, 返回实际收到属于req_event的事件。
 * 当opt为MDRV_EVENT_ANY，rsp_event为req_event的子集;
 * 当opt为MDRV_EVENT_ALL，rsp_event为req_event。
 *
 * @retval 0,接收成功返回
 * @retval 非0,接收失败返回。
 * @par 依赖:
 * <ul><li>mdrv_event.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_event_send
 */
MODULE_EXPORTED int mdrv_event_recv(unsigned req_event, unsigned opt, unsigned time_out, unsigned *rsp_event);

/**
 * @brief V事件发送接口
 *
 * @par 描述:
 * 对指定任务发送一个或者多个事件，event按照32bit使用，每个bit代表一个事件
 *
 * @attention
 * <ul><li>根据PS需求定制，其他组件不要使用！(目前实现几乎等同mdrv_event_send)</li></ul>
 * <ul><li>目标任务必须存在，且创建了队列后才能成功发送过去</li></ul>
 * <ul><li>Bit9事件 在协议栈任务上有专用用途，所以不要给协议栈任务发送Bit9事件</li></ul>
 *
 * @param[in]  task_id, 操作系统任务id，在kernel下即pid，可以从task_struct中的pid得到
 * @param[in]  event, 发送的事件，可以一次发送多个事件(多个bit)，不可为0
 *
 * @retval 0,发送成功
 * @retval 非0,发送失败。
 * @par 依赖:
 * <ul><li>mdrv_event.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_event_vrecv
 */
MODULE_EXPORTED int mdrv_event_vsend(unsigned task_id, unsigned event);
/**
 * @brief V事件接收接口
 *
 * @par 描述:
 * 接收一个或者多个事件，event按照bit使用，每个bit代表一个事件。事件意义由收发双方自行定义
 * 该接口为协议栈定制接口，会根据协议栈的需求修改，目前的行为类似mdrv_event_recv。
 * 1，该接口可以通过消息唤醒，且返回特殊事件
 * 2，该接口在返回特殊事件时，不可以组合返回其他普通事件
 * 3，当同时有消息和事件存在时，优先返回普通事件
 * 4，普通事件可以组合返回
 * 5，当收到特殊事件后，调用者应采取措施处理消息，否则再次调用该接口，会继续返回特殊事件
 * 6，对执行该接口的任务发送特殊事件，会引起未定义的行为
 *
 * @attention
 * <ul><li>根据PS需求定制，其他组件不要使用！(允许消息唤醒，并返回协议栈保留事件)</li></ul>
 * <ul><li>任务必须创建了队列后才能调用该接口收取事件</li></ul>
 * <ul><li>只能在普通任务上下文中使用，在禁止调度的任务上下文中不允许阻塞</li></ul>
 *
 * @param[in]  req_event, 要接收的事件集合，按bit使用，不可为0
 * @param[in]  opt, 仅支持MDRV_EVENT_ANY, 其他输入均为非法
 * @param[in]  time_out, 超时时间，按照系统tick计数(时间计算与系统相关)
 * 当超时时间为0时，代表非阻塞RECV,可以禁止调度的任务上下文中使用
 * 当超时时间为MDRV_EVENT_WAIT_FOREVER时，代表永不超时
 * 当超时时间为其他值时，按照tick计数，若计时到达前事件满足，则正常返回，否则超时返回
 * @param[out]  rsp_event, 返回返回实际收到的事件(可能返回PS定义的特殊事件)
 *
 * @retval 0,接收成功返回
 * @retval 非0,接收失败失败。
 * @par 依赖:
 * <ul><li>mdrv_event.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_event_vsend
 */
MODULE_EXPORTED int mdrv_event_vrecv(unsigned req_event, unsigned opt, unsigned time_out, unsigned *rsp_event);

#ifdef __cplusplus
}
#endif
#endif
