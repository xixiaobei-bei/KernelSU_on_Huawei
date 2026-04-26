/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
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
 * @brief   sysboot模块在acore上的对外头文件
 * @file    mdrv_sysboot.h
 * @author  panjunzhou
 * @version v1.0
 * @date    2019.11.29
 * @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|2019.11.29|创建文件</li></ul>
 * @since   非融合版本
 */

#ifndef __MDRV_ACORE_SYSBOOT_H__
#define __MDRV_ACORE_SYSBOOT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
     *                           Attention                           *
 * Description : Driver for sysboot
 * Core        : Acore、Ccore
 * Header File : the following head files need to be modified at the same time
 * : /acore/mdrv_sysboot.h
 * : /ccore/ccpu/mdrv_sysboot.h
 * : /ccore/fusion/mdrv_sysboot.h
 ******************************************************************************/
/**
 * @brief 复位回调函数种类
 */
typedef enum {
    MDRV_RESET_CB_BEFORE, /**< 复位前的回调 */
    MDRV_RESET_CB_AFTER,  /**< 复位完成后的回调 */
    MDRV_RESET_RESETTING, /**< 复位过程中的回调 */
    MDRV_RESET_CB_INVALID,
} drv_reset_cb_moment_e;

/**
 * @brief RAT模式
 */
typedef enum {
    DRV_RAT_MODE_UMTS_FDD, /**< FDD */
    DRV_RAT_MODE_UMTS_TDD, /**< TDD */
    DRV_RAT_MODE_BUTT
} drv_rat_mode_e;

/**
 * @brief 关机原因
 */
typedef enum DRV_SHUTDOWN_REASON_tag_s {
    DRV_SHUTDOWN_POWER_KEY,           /**< 长按 Power 键关机 */
    DRV_SHUTDOWN_BATTERY_ERROR,       /**< 电池异常 */
    DRV_SHUTDOWN_LOW_BATTERY,         /**< 电池电量低 */
    DRV_SHUTDOWN_TEMPERATURE_PROTECT, /**< 过温保护关机 */
    DRV_SHUTDOWN_CHARGE_REMOVE,       /**< 关机充电模式下，拔除充电器 */
    DRV_SHUTDOWN_UPDATE,              /**< 关机并进入升级模式 */
    DRV_SHUTDOWN_RESET,               /**< 系统软复位 */
    DRV_SHUTDOWN_BUTT
} drv_shutdown_reason_e;

/**
 * @brief 开机原因枚举值定义
 */
typedef enum {
    POWER_ON_REASON_START = 0x45462310,             /**< start magic number */
    POWER_ON_REASON_CHARGE = POWER_ON_REASON_START, /**< code boot: include USB/DC */
    POWER_ON_REASON_POWER_KEY = 0x45462311,         /**< code boot: power key */
    POWER_ON_REASON_NORMAL_REBOOT = 0x45462312,     /**< warm boot: normal reboot */
    POWER_ON_REASON_ACORE_WDT = 0x45462313,         /**< warm boot: A core watch dog */
    POWER_ON_REASON_CCORE_WDT = 0x45462314,         /**< warm boot: C core watch dog */
    POWER_ON_REASON_MCORE_WDT = 0x45462315,         /**< warm boot: M core watch dog */
    POWER_ON_REASON_ACORE_EXCEPTION = 0x45462316,   /**< warm boot: A core Other anomalies */
    POWER_ON_REASON_CCORE_EXCEPTION = 0x45462317,   /**< warm boot: C core Other anomalies */
    POWER_ON_REASON_MCORE_EXCEPTION = 0x45462318,   /**< warm boot: M core Other anomalies */
    POWER_ON_REASON_UNKNOW_EXCEPTION = 0x45462319,  /**< unkwon reason, such as Hardware reset */
    POWER_ON_REASON_BUTT = 0x4546231a
} power_on_start_reason_e;

/**
 * @brief 开机模式枚举值定义
 * 注意：该枚举定义与mbb上层应用保持一致，为保证上层应用的前向兼容，各模式枚举值不能变更
 */
typedef enum {
    DRV_START_MODE_EXCEPTION,                     /**< 异常开机模式，此时大部分业务都不启动 */
    DRV_START_MODE_NORMAL,                        /**< 正常开机模式，所有业务启动 */
    DRV_START_MODE_CHARGING,                      /**< 充电开机模式, ps 等业务不启动 */
    DRV_START_MODE_NOAPP,                         /**< SD卡升级第二次重启后不启动APP */
    DRV_START_MODE_UPDATE = DRV_START_MODE_NOAPP, /**< 升级模式 */
    DRV_START_MODE_MODEM,
    DRV_START_MODE_BUTT
} drv_start_mode_e;
/*****************************************************************************
     *                           Attention                           *
 *****************************************************************************/
/**
 * @brief Modem子系统关机接口。
 *
 * @par 描述:
 * Modem子系统关机接口。
 *
 * @attention:
 * <ul><li>1. PS/TAF上层只需关注过温关机场景。</li></ul>
 * <ul><li>2.
 * 对于按键关机、低电关机、关机充电时拔出充电器关机等MBB解决方案的场景，使用底软内部接口，不要调用mdrv_sysboot_shutdown接口。</li></ul>
 *
 * @par 依赖:
 * <ul><li>mdrv_sysboot.h：该接口声明所在的头文件。</li></ul>
 */
void mdrv_sysboot_shutdown(void);

/**
 * @brief 复位Modem子系统。
 *
 * @par 描述:
 * 复位Modem子系统。
 *
 * @attention:
 * <ul><li>1. P在phone形态下，该函数实现对Modem子系统（C核）的单独复位；MBB形态下，则是对全系统复位。</li></ul>
 * <ul><li>2. ccore和acore都可以调用，最终结果一致，内部函数实现有差异。</li></ul>
 * <ul><li>3. 异常情况下，如果需要重启modem时，请使用system_error接口；严禁直接调用该接口。</li></ul>
 *
 * @par 依赖:
 * <ul><li>mdrv_sysboot.h：该接口声明所在的头文件。</li></ul>
 */
void mdrv_sysboot_restart(void);

/**
 * @brief 设置Modem状态。
 *
 * @par 描述:
 * 设置Modem状态（Ready或者Off）。
 *
 * @param[in]  state Modem状态，MODEM_NOT_READY(0), MODEM_READY(1)
 *
 * @retval =0，表示设置成功。
 * @retval =1，表示设置失败。
 *
 * @attention:
 * <ul><li>1. taf通过该接口设置Modem状态。</li></ul>
 * <ul><li>2.
 * 同时存在多个Modem时，上层必须在所有Modem状态均确认后再调用该接口设置Modem状态。上层调用该接口设置MODEM_READY前，应用层获取到的状态为Modem
 * Off。</li></ul>
 *
 * @par 依赖:
 * <ul><li>mdrv_sysboot.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_set_modem_state(unsigned int state);

/**
 * @brief reset回调函数格式
 */
typedef int (*pdrv_reset_cbfun)(drv_reset_cb_moment_e enparam, int userdata);

/**
 * @brief 设置Modem状态。
 *
 * @par 描述:
 * 设置Modem状态（Ready或者Off）。
 *
 * @param[in]  pname 上层组件注册的名字，最长9个字符（不包括结束符），底软负责存储
 * @param[in]  pcbfun 回调函数指针
 * @param[in]  userdata 上层组件数据，在调用回调函数时，作为入参传给注册者
 * @param[in]  priolevel 回调函数调用优先级，参考emum DRV_RESET_CALLCBFUN_PRIO定义，值越小优先级越高
 *
 * @retval =0，表示注册成功。
 * @retval =1，表示注册失败。
 *
 * @par 依赖:
 * <ul><li>mdrv_sysboot.h：该接口声明所在的头文件。</li></ul>
 *
 * @see pdrv_reset_cbfun
 */
int mdrv_sysboot_register_reset_notify(const char *pname, pdrv_reset_cbfun pcbfun, int userdata, int priolevel);

/**
 * @brief 5GCPU状态读取接口函数。
 *
 * @par 描述:
 * 5GCPU状态读取接口函数。
 *
 * @retval =0，表示未初始化成功。
 * @retval =1，表示初始化成功。
 *
 * @par 依赖:
 * <ul><li>mdrv_sysboot.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_sysboot_is_nr_modem_ready(void);

/**
 * @brief 4GCPU状态读取接口函数。
 *
 * @par 描述:
 * 4GCPU状态读取接口函数。
 *
 * @retval =0，表示未初始化成功。
 * @retval =1，表示初始化成功。
 *
 * @par 依赖:
 * <ul><li>mdrv_sysboot.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_sysboot_is_modem_ready(void);

/**
 * @brief 获取开机模式,该接口只在mbb平台生效，phone平台接口打桩。
 *
 * @par 描述:
 * 获取开机模式信息。
 *
 * @retval drv_start_mode_e,开机模式枚举值
 *
 * @par 依赖:
 * <ul><li>mdrv_sysboot.h：该接口声明所在的头文件。</li></ul>
 */
drv_start_mode_e mdrv_sysboot_get_bootmode(void);

/**
 * @brief 获取开机原因，该接口只在mbb平台生效，phone平台接口打桩。
 *
 * @par 描述:
 * 获取开机原因信息。
 *
 * @retval power_on_start_reason_e,开机原因枚举值。
 *
 * @par 依赖:
 * <ul><li>mdrv_sysboot.h：该接口声明所在的头文件。</li></ul>
 */
power_on_start_reason_e mdrv_sysboot_get_power_on_reason(void);

/**
 * @brief power_off回调注册，该接口只在mbb平台生效，phone平台接口打桩。
 *
 * @par 描述:
 * 用于注册关机前处理的回调注册函数
 * 1. 供MBB模块在关机前执行必要的处理
 * 2. 注册的回调函数原型必须是void (*hook)(void)
 *
 * @retval void
 *
 * @par 依赖:
 * <ul><li>mdrv_sysboot.h：该接口声明所在的头文件。</li></ul>
 */
void mdrv_sysboot_power_off_cb_register(void (*hook)(void));

/**
 * @brief reboot回调注册，该接口只在mbb平台生效，phone平台接口打桩。
 *
 * @par 描述:
 * 用于注册设备重启前处理的回调注册函数
 * 1. 供MBB模块在设备重启前执行必要的处理
 * 2. 注册的回调函数原型必须是void (*hook)(void)
 *
 * @retval void
 *
 * @par 依赖:
 * <ul><li>mdrv_sysboot.h：该接口声明所在的头文件。</li></ul>
 */
void mdrv_sysboot_reboot_cb_register(void (*hook)(void));

#ifdef __cplusplus
}
#endif
#endif
