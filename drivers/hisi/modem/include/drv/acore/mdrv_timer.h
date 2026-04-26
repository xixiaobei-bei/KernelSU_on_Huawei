/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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
 *  @brief   timer模块在ACORE上的对外头文件
 *  @file    mdrv_timer.h
 *  @author  baihaoyue
 *  @version v1.0
 *  @date    2019.11.29
 *  @note    修改记录(版本号|修订日期|说明) 
 *  <ul><li>v1.0|2012.11.29|创建文件</li></ul>
 *  <ul><li>v2.0|2019.11.29|接口自动化</li></ul>
 *  @since   
 */

#ifndef __MDRV_ACORE_TIMER_H__
#define __MDRV_ACORE_TIMER_H__

#include "mdrv_public.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
* @brief ACORE定时器枚举
*/
typedef enum {
    ACORE_SOFTTIMER_ID,         /**< acore A core 唤醒源软timer id */
    ACORE_WDT_TIMER_ID,         /**< acore A CORE看门狗 */
    TSP_RESERVE_TIMER0_ID,      /**< acore 保留 */
    ACORE_SOFTTIMER_NOWAKE_ID,  /**< acore A core 非唤醒源软timer */
    TSP_RESERVE_TIMER1_ID,      /**< acore 保留 */
    TSP_RESERVE_TIMER2_ID,      /**< acore 保留 */
    TSP_RESERVE_TIMER3_ID,      /**< acore 保留 */
    TIMER_ACPU_OSA_ID,          /**< acore A CORE软件定时 */
    TIMER_ACPU_OM_TCXO_ID,      /**< acore 解决vos timer频繁唤醒 */
    TIMER_HIFI1_ID,             /**< acore hifi使用 */
    TIMER_HIFI2_ID,             /**< acore hifi使用 */
    TIMER_HRTIMER_ID,           /**< acore hrtimer */
    TIMER_ID_MAX
} drv_timer_id_e;

/**
* @brief 定时器计数模式
*/
typedef enum {
    TIMER_ONCE_COUNT = 0,        /**< acore 单次定时器模式 */
    TIMER_PERIOD_COUNT,          /**< acore 周期定时器模式 */
    TIMER_COUNT_BUTT
} drv_timer_mode_e;

/**
* @brief 定时器单位
*/
typedef enum {
    TIMER_UNIT_MS = 0,           /**< acore 0表示单位ms模式 */
    TIMER_UNIT_US,               /**< acore 1表示单位us模式 */
    TIMER_UNIT_NONE,             /**< acore 2表示单位1，即直接操作load寄存器模式 */
    TIMER_UNIT_BUTT
} drv_timer_unit_e;

/**
 * @brief timer回调
 */
typedef int (*timer_handle_cb)(int);

/**
 * @brief 封装给上层注册timer唤醒系统时的OM回调
 *
 * @par 描述:
 * 封装给上层注册timer唤醒系统时的OM回调
 *
 * @param[in]  timer_id , 定时器ID。
 * @param[in]  routinue, 回调函数。
 * @param[in]  arg, 回调参数。
 *
 * @retval 无。

 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 * @see drv_timer_id_e timer_handle_cb
 */
void mdrv_timer_debug_register(drv_timer_id_e timer_id, timer_handle_cb routinue, int arg);

/**
 * @brief 定时器启动函数
 *
 * @par 描述:
 * 根据传入的回调函数、回调参数、计数值、定时器模式及单位，设置并启动对应的定时器。
 *
 * @param[in]  timer_id , 定时器ID。
 * @param[in]  routinue, 回调函数。
 * @param[in]  arg, 回调参数。
 * @param[in]  timer_value, 计数值。
 * @param[in]  mode, 定时器计数模式。
 * @param[in]  unit_type, 计数单位。
 *
 * @retval 0,表示函数执行成功。
 * @retval ！=0,表示函数执行失败。
 
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 * @see drv_timer_id_e timer_handle_cb drv_timer_mode_e drv_timer_unit_e
 */
int mdrv_timer_start(drv_timer_id_e timer_id, timer_handle_cb routine, int arg,
                     unsigned int timer_value, drv_timer_mode_e mode, drv_timer_unit_e unit_type);

/**
 * @brief 定时器停止函数
 *
 * @par 描述:
 * 定时器停止函数。
 *
 * @param[in]  timer_id , 定时器ID。
 *
 * @retval 0,表示函数执行成功。
 * @retval ！=0,表示函数执行失败。
 
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 * @see drv_timer_id_e
 */
int mdrv_timer_stop(drv_timer_id_e timer_id);

/**
 * @brief 获取定时器的剩余时间
 *
 * @par 描述:
 * 获取定时器的剩余时间，该时间根据传入的单位进行计算。
 *
 * @param[in]  timer_id , 定时器ID。
 * @param[in]  unit_type, 计数单位。
 * @param[out]  p_rest_time, 剩余时间。
 *
 * @retval 0,表示函数执行成功。
 * @retval ！=0,表示函数执行失败。
 
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 * @see drv_timer_id_e drv_timer_unit_e
 */
int mdrv_timer_get_rest_time(drv_timer_id_e timer_id, drv_timer_unit_e unit_type, unsigned int *p_rest_time);

/**
 * @brief 获取BBP定时器的值。
 *
 * @par 描述:
 * 获取BBP定时器的值，用于OAM 时戳。
 *
 * @param[out]  p_high32bit_value , 高位值。
 * @param[out]  p_low32bit_value, 低位值。
 *
 * @retval 0,表示函数执行成功。
 * @retval ！=0,表示函数执行失败。
 
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_timer_get_accuracy_timestamp(unsigned int *p_high32bit_value, unsigned int *p_low32bit_value);

/**
 * @brief 获取32KHz时间戳
 *
 * @par 描述:
 * 获取32KHz时间戳，时间戳返回值为递增值，一个数值代表30.5us。
 *
 * @retval 32KHz时间戳。
 
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 * 
 * @see mdrv_get_normal_timestamp_freq
 */
unsigned int mdrv_timer_get_normal_timestamp(void);

/**
 * @brief 获取19.2MHz时间戳
 *
 * @par 描述:
 * 获取19.2MHz高精度时间戳，系统深睡会停止计时，一个数值代表52ns。
 *
 * @retval 高精度时间戳。
 
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_get_hrt_timestamp_freq
 */
unsigned int mdrv_timer_get_hrt_timestamp(void);

/**
 * @brief 获取32KHz时间戳频率
 *
 * @par 描述:
 * 获取32KHz时间戳频率，单位HZ
 *
 * @retval 时间戳频率。
 
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 */
unsigned int mdrv_get_normal_timestamp_freq(void);

/**
 * @brief 获取19.2MHz时间戳频率
 *
 * @par 描述:
 * 获取19.2MHz时间戳频率，单位HZ
 *
 * @retval 时间戳频率。
 
 * @par 依赖:
 * <ul><li>mdrv_timer.h：该接口声明所在的头文件。</li></ul>
 *
 */
unsigned int mdrv_get_hrt_timestamp_freq(void);

#ifdef __cplusplus
}
#endif

#endif
