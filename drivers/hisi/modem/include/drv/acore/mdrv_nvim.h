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

 /**
 *  @brief   nv模块在acore上的对外头文件
 *  @file    mdrv_nvim.h
 *  @author  zhaoxiaofeng
 *  @version v1.0
 *  @date    2019.11.26
 *  @note    修改记录(版本号|修订日期|说明)
 *  <ul><li>v1.0|2019.11.26|创建文件</li></ul>
 *  @since
 */
#ifndef __MDRV_ACORE_NVIM_H__
#define __MDRV_ACORE_NVIM_H__
#ifdef __cplusplus
extern "C"
{
#endif

#include "mdrv_nvim_comm.h"

/**
* @brief 
*/
enum nv_factory_check_e
{
    NV_FACTORY_CHECK_M1_GU = 0,     /*GU主卡*/
    NV_FACTORY_CHECK_M1_TL,         /*TL主卡*/
    NV_FACTORY_CHECK_M2_GU,         /*GU副卡*/
    NV_FACTORY_CHECK_BUTT
};

/**
 * @brief 校验校准NV的CRC校验码
 *
 * @param[in]  mode, mode取值0: GU主卡 1:TL主卡 2:GU副卡

 * @retval 0,表示校验通过。
 * @retval 0x100f0002, 参数有误。
 * @retval 0x100f0027, 校验错误。

 */
unsigned int mdrv_nv_check_factorynv(unsigned int mode);

/**
 * @brief 获取NV镜像支持的modem个数
 *
 * @retval 支持的modem个数
 */
unsigned int mdrv_nv_get_modem_num(void);

/**
 * @brief 更新NV备份分区
 *
 * @attention
 * <ul><li>只更新备份分区</li></ul>

 * @retval 0,表示函数执行成功。
 * @retval 0x100f0035, 获取信号量超时。
 * @retval 0x100f0036, 释放信号量错误。
 */
unsigned int mdrv_nv_backup(void);

/**
 * @brief 获取单个NV数据的地址
 *
 * @attention
 * <ul><li>modem ID无效或者NV ID找不到返回NULL</li></ul>

 * @param[in]  modemid, modem ID。
 * @param[in]  itemid, NV ID。

 * @retval 有效地址或NULL

 * @par 依赖:
 * <ul><li>mdrv_nvim.h：该接口声明所在的头文件。</li></ul>
 */
void * mdrv_nv_get_addr(unsigned int modemid, unsigned int itemid);

#ifndef FEATURE_NV_SEC_ON
/**
 * @brief 备份NV数据到出场区
 *
 * @attention
 * <ul><li>更新备份区和出厂区</li></ul>

 * @retval 0,表示函数执行成功。
 * @retval 0x100f0035, 获取信号量超时。
 * @retval 0x100f0036, 释放信号量错误。
 */
unsigned int mdrv_nv_backup_factorynv(void);

/**
 * @brief 恢复出厂区NV
 *
 * @retval 0,表示函数执行成功。
 * @retval 0x100f0035, 获取信号量超时。
 * @retval 0x100f0036, 释放信号量错误。
 */
unsigned int mdrv_nv_revert_factorynv(void);
#endif
/*****************************************************************************
* 函 数 名  : mdrv_nv_restore
*
* 功能描述  :  恢复生产NV项
*
* 输入参数  :
* 输出参数  : 无
*
* 返 回 值  : OK
*
*****************************************************************************/
/**
 * @brief mdrv_nv_restore
 * @todo 历史无用桩接口，待删除
 */
unsigned int mdrv_nv_restore(void);
/**
 * @brief mdrv_nv_restore_result
 * @todo 历史无用桩接口，待删除
 */
unsigned int mdrv_nv_restore_result(void);

#ifdef __cplusplus
}
#endif
#endif
