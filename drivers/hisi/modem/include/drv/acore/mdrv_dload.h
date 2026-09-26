/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

/* *
 * @brief   dload模块在acore的对外头文件
 * @file    mdrv_dload.h
 * @author  zhaixiaojun
 * @version v1.0
 * @date    2019.12.04
 * <ul><li>v1.0|2019.12.04|创建文件</li></ul>
 * @since
 */
#ifndef _MDRV_DLOAD_H_
#define _MDRV_DLOAD_H_

#ifdef _cplusplus
extern "C" {
#endif

/* *
 * @brief 下载模式枚举
 */
typedef enum tagDLOAD_MODE_E {
    DLOAD_MODE_DOWNLOAD = 0,
    DLOAD_MODE_NORMAL,
    DLOAD_MODE_DATA,
    DLOAD_MODE_MAX
} DLOAD_MODE_E;

/* *
 * @brief version信息结构
 */
typedef struct tagDLOAD_VER_INFO_S {
    unsigned char *productname;
    unsigned int productnamelen;
    unsigned char *softwarever;
    unsigned int softwareverlen;
    unsigned char *isover;
    unsigned int isoverlen;
    unsigned char *dloadver;
    unsigned int dloadverlen;
} DLOAD_VER_INFO_S;

/* *
 * @brief 设置当前工作模式
 *
 * @par 描述:
 * 设置当前工作模式
 *
 * @param[in]  eDloadMode, 工作模式
 *
 * <ul><li>mdrv_dload.h：该接口声明所在的头文件。</li></ul>
 *
 * @see DLOAD_MODE_E
 */
void mdrv_dload_set_curmode(DLOAD_MODE_E eDloadMode);

/* *
 * @brief 获取当前工作模式
 *
 * @par 描述:
 * 获取当前工作模式
 *
 * @retval 当前工作模式。
 * <ul><li>mdrv_dload.h：该接口声明所在的头文件。</li></ul>
 *
 * @see DLOAD_MODE_E
 */
DLOAD_MODE_E mdrv_dload_get_curmode(void);

/* *
 * @brief 设置启动标志
 *
 * @par 描述:
 * 设置启动标志
 *
 * @param[in]  bSoftLoad, 是否软件加载
 *
 */
void mdrv_dload_set_softload(int bSoftLoad);

/* *
 * @brief 获取启动标志
 *
 * @par 描述:
 * 获取启动标志
 *
 * @retval 当前启动标志。
 *
 */
int mdrv_dload_get_softload(void);

/* *
 * @brief 获取升级版本信息
 *
 * @par 描述:
 * 获取升级版本信息
 *
 * @retval 升级版本信息。
 * <ul><li>mdrv_dload.h：该接口声明所在的头文件。</li></ul>
 *
 * @see DLOAD_VER_INFO_S
 */
DLOAD_VER_INFO_S * mdrv_dload_get_info(void);


#ifdef _cplusplus
}
#endif
#endif
