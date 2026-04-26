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

/**
 *  @brief   IPC模块在非融合版本上的资源枚举对外头文件
 *  @file    mdrv_ipc_comm.h
 *  @author  wangtongli
 *  @version v1.0
 *  @date    2019.11.27
 *  @note    修改记录
 *  <ul><li>v1.0|2019.11.27|支持接口自动化格式</li></ul>
 *  @since   始于DRV1.0（非融合版本）
 */

#ifndef __MDRV_IPC_COMMON_H__
#define __MDRV_IPC_COMMON_H__

#include "mdrv_public.h"
#include "mdrv_ipc_enum.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief ipc创建IPC信号量函数
 *
 * @par 描述:
 * 该接口用于创建IPC信号量
 *
 * @attention
 * <ul><li>暂无</li></ul>
 *
 * @param[in]  u32SignalNum , 信号量id
 *
 * @retval 0,表示函数执行成功。
 * @retval ！=0,表示函数执行失败。

 * @par 依赖:
 * <ul><li>mdrv_ipc_comm.h：该接口声明所在的头文件</li></ul>
 * <ul><li>mdrv_ipc_enum.h：该接口使用资源枚举对外头文件</li></ul>
 *
 * @see ipc_sem_id_e
 */
int mdrv_ipc_sem_create (unsigned int u32SignalNum);

/**
 * @brief ipc删除创建的IPC信号量函数
 *
 * @par 描述:
 * 该接口用于删除创建的IPC信号量
 *
 * @attention
 * <ul><li>暂无</li></ul>
 *
 * @param[in]  u32SignalNum , 信号量id
 *
 * @retval 0,表示函数执行成功。
 * @retval ！=0,表示函数执行失败。

 * @par 依赖:
 * <ul><li>mdrv_ipc_comm.h：该接口声明所在的头文件</li></ul>
 * <ul><li>mdrv_ipc_enum.h：该接口使用资源枚举对外头文件</li></ul>
 *
 * @see ipc_sem_id_e
 */
int mdrv_ipc_sem_delete(unsigned int u32SignalNum);

/**
 * @brief ipc使能某号中断
 *
 * @par 描述:
 * 该接口用于使能某号中断
 *
 * @attention
 * <ul><li>暂无</li></ul>
 *
 * @param[in]  ulLvl , ipc物理资源
 *
 * @retval 0,表示函数执行成功。
 * @retval ！=0,表示函数执行失败。

 * @par 依赖:
 * <ul><li>mdrv_ipc_comm.h：该接口声明所在的头文件</li></ul>
 * <ul><li>mdrv_ipc_enum.h：该接口使用资源枚举对外头文件</li></ul>
 *
 * @see ipc_int_lev_e
 */
int mdrv_ipc_int_enable (ipc_int_lev_e ulLvl);

/**
 * @brief ipc去使能某号中断
 *
 * @par 描述:
 * 该接口用于去使能某号中断
 *
 * @attention
 * <ul><li>暂无</li></ul>
 *
 * @param[in]  ulLvl , ipc物理资源
 *
 * @retval 0,表示函数执行成功。
 * @retval ！=0,表示函数执行失败。

 * @par 依赖:
 * <ul><li>mdrv_ipc_comm.h：该接口声明所在的头文件</li></ul>
 * <ul><li>mdrv_ipc_enum.h：该接口使用资源枚举对外头文件</li></ul>
 *
 * @see ipc_int_lev_e
 */
int mdrv_ipc_int_disable (ipc_int_lev_e ulLvl);

/**
 * @brief ipc往某号中断挂接服务程序
 *
 * @par 描述:
 * 该接口用于往某号中断挂接服务程序
 *
 * @attention
 * <ul><li>暂无</li></ul>
 *
 * @param[in]  ulLvl , ipc物理资源
 * @param[in]  routine , 中断服务程序
 * @param[in]  parameter , 中断服务程序参数
 *
 * @retval 0,表示函数执行成功。
 * @retval ！=0,表示函数执行失败。

 * @par 依赖:
 * <ul><li>mdrv_ipc_comm.h：该接口声明所在的头文件</li></ul>
 * <ul><li>mdrv_ipc_enum.h：该接口使用资源枚举对外头文件</li></ul>
 *
 * @see ipc_int_lev_e
 */
int mdrv_ipc_int_connect (ipc_int_lev_e ulLvl, voidfuncptr routine, unsigned int parameter);

/**
 * @brief ipc删除某号中断挂接的服务程序
 *
 * @par 描述:
 * 该接口用于删除某号中断挂接的服务程序
 *
 * @attention
 * <ul><li>暂无</li></ul>
 *
 * @param[in]  ulLvl , ipc物理资源
 *
 * @retval 0,表示函数执行成功。
 * @retval ！=0,表示函数执行失败。

 * @par 依赖:
 * <ul><li>mdrv_ipc_comm.h：该接口声明所在的头文件</li></ul>
 * <ul><li>mdrv_ipc_enum.h：该接口使用资源枚举对外头文件</li></ul>
 *
 * @see ipc_int_lev_e
 */
int mdrv_ipc_int_disconnect (ipc_int_lev_e ulLvl);

/**
 * @brief ipc往目标核发送中断
 *
 * @par 描述:
 * 该接口用于往目标核发送中断
 *
 * @attention
 * <ul><li>无</li></ul>
 *
 * @param[in]  enDstCore , 目的核号
 * @param[in]  ulLvl , ipc物理资源
 *
 * @retval 0,表示函数执行成功。
 * @retval ！=0,表示函数执行失败。

 * @par 依赖:
 * <ul><li>mdrv_ipc_comm.h：该接口声明所在的头文件</li></ul>
 * <ul><li>mdrv_ipc_enum.h：该接口使用资源枚举对外头文件</li></ul>
 *
 * @see ipc_int_lev_e
 */
int mdrv_ipc_int_send(ipc_int_core_e enDstCore, ipc_int_lev_e ulLvl);

/**
 * @brief 带睡眠功能的获取某个信号量
 *
 * @par 描述:
 * 该接口用于带睡眠功能的获取某个信号量
 *
 * @attention
 * <ul><li>无</li></ul>
 *
 * @param[in]  u32SignalNum , 信号量id
 * @param[in]  s32timeout , 超时时间
 *
 * @retval 0,表示函数执行成功。
 * @retval ！=0,表示函数执行失败。

 * @par 依赖:
 * <ul><li>mdrv_ipc_comm.h：该接口声明所在的头文件</li></ul>
 * <ul><li>mdrv_ipc_enum.h：该接口使用资源枚举对外头文件</li></ul>
 *
 * @see ipc_sem_id_e
 */
int mdrv_ipc_sem_take (unsigned int u32SignalNum, int s32timeout);

/**
 * @brief 释放take到的信号量
 *
 * @par 描述:
 * 该接口用于释放take到的信号量
 *
 * @attention
 * <ul><li>无</li></ul>
 *
 * @param[in]  u32SignalNum , 信号量id
 *
 * @retval 0,表示函数执行成功。
 * @retval ！=0,表示函数执行失败。

 * @par 依赖:
 * <ul><li>mdrv_ipc_comm.h：该接口声明所在的头文件</li></ul>
 * <ul><li>mdrv_ipc_enum.h：该接口使用资源枚举对外头文件</li></ul>
 *
 * @see ipc_sem_id_e
 */
int mdrv_ipc_sem_give(unsigned int u32SignalNum);

/**
 * @brief 循环等待获取某号ipc信号量
 *
 * @par 描述:
 * 该接口用于循环等待获取某号ipc信号量
 *
 * @attention
 * <ul><li>无</li></ul>
 *
 * @param[in]  u32SignalNum , 信号量id
 *
 * @retval 0,表示函数执行成功。
 * @retval ！=0,表示函数执行失败。

 * @par 依赖:
 * <ul><li>mdrv_ipc_comm.h：该接口声明所在的头文件</li></ul>
 * <ul><li>mdrv_ipc_enum.h：该接口使用资源枚举对外头文件</li></ul>
 *
 * @see ipc_sem_id_e
 */
void mdrv_ipc_spin_lock(unsigned int u32SignalNum);

/**
 * @brief 尝试获取某号ipc信号量，不等待
 *
 * @par 描述:
 * 该接口用于尝试获取某号ipc信号量，不等待
 *
 * @attention
 * <ul><li>无</li></ul>
 *
 * @param[in]  u32SignalNum , 信号量id
 *
 * @retval 0,表示函数执行成功。
 * @retval ！=0,表示函数执行失败。

 * @par 依赖:
 * <ul><li>mdrv_ipc_comm.h：该接口声明所在的头文件</li></ul>
 * <ul><li>mdrv_ipc_enum.h：该接口使用资源枚举对外头文件</li></ul>
 *
 * @see ipc_sem_id_e
 */
int mdrv_ipc_spin_trylock(unsigned int u32SignalNum);

/**
 * @brief 释放循环等待获取到的信号量
 *
 * @par 描述:
 * 该接口用于释放循环等待获取到的信号量
 *
 * @attention
 * <ul><li>无</li></ul>
 *
 * @param[in]  u32SignalNum , 信号量id
 *
 * @retval 0,表示函数执行成功。
 * @retval ！=0,表示函数执行失败。

 * @par 依赖:
 * <ul><li>mdrv_ipc_comm.h：该接口声明所在的头文件</li></ul>
 * <ul><li>mdrv_ipc_enum.h：该接口使用资源枚举对外头文件</li></ul>
 *
 * @see ipc_sem_id_e
 */
void mdrv_ipc_spin_unlock(unsigned int u32SignalNum);


#ifdef __cplusplus
}
#endif
#endif
