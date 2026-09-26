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
#ifndef __BSP_PMCTRL_H__
#define __BSP_PMCTRL_H__

#define PMCTRL_ERR -1
#define PMCTRL_OK 0

/**
 * @brief 电压档位 枚举类型，支持的电压档位由低到高
 */
typedef enum {
    PROFILE_ID_0, /**< 支持档位的最低档 */
    PROFILE_ID_1,
    PROFILE_ID_2,
    PROFILE_ID_3,
    PROFILE_ID_4,
    PROFILE_ID_MAX,
} profile_id_e;

/**
 * @brief 调压目标枚举类型
 */
typedef enum {
    PERI_PMCTRL,
} pmctrl_core_e;

/**
 * @brief 当前profile 状态枚举类型
 */
typedef enum {
    VREADY,
    WAITING,
} status_id_e;

/**
 * @brief 获取PMCTRL 句柄
 * @par 描述:
 * 根据目标枚举获取句柄
 *
 * @attention
 * <ul>
 * <li>targe_id必须正确，传入错误的target函数会执行失败。</li>
 * <li>name 命名最长 32 Byte，命名要求以组件名开头，下划线跟后续描述，传入超过32Byte的字符串会被截断。</li>
 * <li>name 命名举例 phy_xxx、drv_xxx、ps_xxx </li>
 * <li>name 命名用作维测使用，如果传入重复的名字，会分配新的句柄。</li>
 * <li>name 因为不利于维测，调用者不能多次传入重复名字。</li>
 * <li>name 当该目标的投票获取数超过了上限，调用该接口无法分配出新的pmctrl句柄，会返回失败</li>
 * </ul>
 * @param[in]  target_id, 调压目标型枚举号
 * @param[in]  name, 用户名称
 * @param[out]  pmctrl_handle, pmctrl句柄
 *
 * @retval 0,表示函数执行成功
 * @retval !=0,表示函数执行失败
 * @par 依赖:
 * <ul><li>mdrv_pmu.h：该接口声明所在的头文件。</li></ul>
 * @see target_id_e target_id
 */

int bsp_pmctrl_create(pmctrl_core_e target_id, const char *name, unsigned int *pmctrl_handle);

/**
 * @brief 设置电压档位异步接口
 * @par 描述:
 * 根据pmctrl句柄和电压目标档位异步设置电压档位投票
 *
 * @attention
 * <ul>
 * <li>pmctrl_handle 需要使用 mdrv_pmctrl_create 获取</li>
 * <li>profile 需要满足版本的调压档位，超出调压档位会报错。</li>
 * <li>设置电压之后，需要等一段时间才能认为电压稳定，该接口是异步接口，接口内不会等待延时</li>
 * <li>调用异步接口后，需要调用 mdrv_pmctrl_check_profile 查询当前电源状态。</li>
 * <li>当avs电压更新之后，根据当前投票profile ，对pmctrl寄存器中的电压值重新设置。</li>
 * <li>设置profile 投票 与 avs电压更新互斥。</li>
 * </ul>
 * @param[in]  pmctrl_handle, pmctrl句柄
 * @param[in]  profile, 目标电压档位
 *
 * @retval 0,表示函数执行成功
 * @retval !=0,表示函数执行失败
 * @par 依赖:
 * <ul><li>mdrv_pmu.h：该接口声明所在的头文件。</li></ul>
 * @see profile_id_e profile
 */
int bsp_pmctrl_set_profile_async(unsigned int pmctrl_handle, profile_id_e profile);

/**
 * @brief 查询电源状态
 * @par 描述:
 * 根据pmctrl句柄和电压目标档位查询当前pmu电压是否满足电压投票需求
 *
 * @attention
 * <ul>
 * <li>pmu_handle 需要使用 mdrv_pmctrl_create 获取</li>
 * <li>profile, 需要满足版本的调压档位，超出调压档位会报错。</li>
 * </ul>
 * @param[in]  pmctrl_handle, pmctrl句柄
 * @param[in]  profile, 目标电压档位
 * @param[out]  profile_ready, 当前profile 是否满足需求， READY表示满足，WAITING表示不满足
 *
 * @retval 0,表示函数执行成功
 * @retval !=0,表示入参检查失败
 * @par 依赖:
 * <ul><li>mdrv_pmu.h：该接口声明所在的头文件。</li></ul>
 * @see profile_id_e profile status_id_e *profile_ready
 */

int bsp_pmctrl_check_profile(unsigned int pmctrl_handle, profile_id_e profile, status_id_e *profile_ready);

/**
 * @brief 设置电压同步接口
 * @par 描述:
 * 根据pmu句柄和电压目标档位同步设置投票电压
 *
 * @attention
 * <ul>
 * <li>pmctrl_handle 需要使用 mdrv_pmctrl_create 获取</li>
 * <li>volt_profile 需要满足版本的调压档位，超出调压档位会报错。</li>
 * <li>id 设置电压之后，需要等一段时间才能能为电压稳定</li>
 * <li>该接口是同步接口，接口内会等待延时，延时大小核ramp档位 和 调压范围有关。</li>
 * <li>如果当前电压已满足投票电压需求，无额外延时。</li>
 * <li>如果当前电压不满足投票电压需求，延时在50us - 100us 之间。</li>
 * <li>调用同步接口返回后，既可认为当前pmu电压是否满足电压投票需求。</li>
 * <li>当avs电压更新之后，根据当前投票profile ，对pmctrl寄存器中的电压值重新设置。</li>
 * <li>设置profile 投票 与 avs电压更新互斥。</li>
 * </ul>
 * @param[in]  pmctrl_handle, pmctrl句柄
 * @param[in]  profile, 目标电压档位
 *
 * @retval 0,表示函数执行成功
 * @retval !=0,表示函数执行失败
 * @par 依赖:
 * <ul><li>mdrv_pmu.h：该接口声明所在的头文件。</li></ul>
 * @see profile_id_e profile
 */
int bsp_pmctrl_set_profile_sync(unsigned int pmctrl_handle, profile_id_e profile);

#endif
