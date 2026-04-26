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
 *  @brief   charger模块在acore上的对外头文件
 *  @file    mdrv_chg.h
 *  @author  tongyuan
 *  @version v1.0
 *  @date    2019.11.26
 *  @note    修改记录(版本号|修订日期|说明) 
 *  <ul><li>v1.0|2019.11.26|接口自动化整改</li></ul>
 *  @since   DRV1.0
 */

#ifndef __MDRV_ACORE_CHG_H__
#define __MDRV_ACORE_CHG_H__

#include "mdrv_public.h"
#ifdef __cplusplus
extern "C"
{
#endif

/**
* @brief 充电状态列表
*/
typedef enum CHARGING_STATE_E_tag {
        CHARGING_INIT = -1,    /**< 充电初始化 */
        NO_CHARGING_UP = 0,    /**< 开机未充电 */
        CHARGING_UP,           /**< 开机正充电 */
        NO_CHARGING_DOWN,      /**< 关机未充电 */
        CHARGING_DOWN          /**< 关机充电 */
}CHARGING_STATE_E;

/**
* @brief 电池电量列表
*/
typedef enum BATT_LEVEL_E_tag {
        BATT_INIT = -2,        /**< 异常值，初始化时使用 */
        BATT_LOW_POWER = -1,   /**< 电池低电 */
        BATT_LEVEL_0,          /**< 0格电池电量 */
        BATT_LEVEL_1,          /**< 1格电池电量 */
        BATT_LEVEL_2,          /**< 2格电池电量 */
        BATT_LEVEL_3,          /**< 3格电池电量 */
        BATT_LEVEL_4,          /**< 4格电池电量 */
        BATT_LEVEL_MAX
}BATT_LEVEL_E;

/**
* @brief 电池状态
*/
typedef struct BATT_STATE_tag {
    CHARGING_STATE_E  charging_state;  /**< 当前充电状态 */
    BATT_LEVEL_E      battery_level;   /**< 当前电池电量 */
}BATT_STATE_S;

/**
 * @brief 获取当前电池状态信息
 *
 * @par 描述:
 * 获取当前电池状态，包括电池电量和充电状态
 *
 * @attention
 * <ul><li>底软提供的是桩函数，函数具体实现在产品线代码中定义。</li></ul>
 *
 * @param[out] battery_state, BATT_STATE_S结构体变量，存储着当前充电状态以及电池电量信息
 *
 * @retval 0,表示操作成功。
 * @retval -1,表示操作失败。
 *
 * @par 依赖:
 * <ul><li>mdrv_chg.h：该接口声明所在的头文件。</li></ul>
 *
 * @see
 *
 */
int mdrv_misc_get_battery_state(BATT_STATE_S *battery_state);

/**
 * @brief 查询当前电池是否处在充电中
 *
 * @par 描述:
 * 查询当前电池是否处在充电中
 *
 * @attention
 * <ul><li>底软提供的是桩函数，函数具体实现在产品线代码中定义。</li></ul>
 *
 * @param void
 *
 * @retval 0,表示未充电。
 * @retval 1,表示充电中。
 *
 * @par 依赖:
 * <ul><li>mdrv_chg.h：该接口声明所在的头文件。</li></ul>
 *
 * @see
 *
 */
int mdrv_misc_get_charging_status(void);

/**
 * @brief 桩接口
 */
void mdrv_misc_set_charge_state(unsigned long ulState);

/**
 * @brief 桩接口
 */
int mdrv_misc_get_charge_state(void);

/**
 * @brief 桩接口
 */
int mdrv_misc_sply_battery(void);

/**
 * @brief 获取电池状态和电量
 *
 * @par 描述:
 * 获取电池状态和电量
 *
 * @attention
 * <ul><li>底软提供的是桩函数，函数具体实现在产品线代码中定义。</li></ul>
 *
 * @param[out] pusBcs 电池的连接、供电状态
 * @param[out] pucBcl 电量情况
 *
 * @retval  0 表示操作成功
 * @retval -1 表示操作失败
 *
 * @par 依赖:
 * <ul><li>mdrv_chg.h：该接口声明所在的头文件。</li></ul>
 *
 * @see
 *
 */
int mdrv_misc_get_cbc_state(unsigned char *pusBcs, unsigned char *pucBcl);


#ifdef __cplusplus
}
#endif
#endif

