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
 * @brief
 * @file    mdrv_crypto.h
 * @author  wangbao
 * @version v1.0
 * @date    2019.11.27
 * @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|2019.11.27|创建文件</li></ul>
 * @since
 */

#ifndef __MDRV_ACORE_CRYPTO_H__
#define __MDRV_ACORE_CRYPTO_H__
#ifdef __cplusplus
extern "C" {
#endif
/* *
 * @brief 当前版本是否支持安全启动
 *
 * @par 描述:
 * 当前版本是否支持安全启动
 *
 * @param[out]  unsigned char *pu8Data 是否支持安全启动标志[1:支持   0:不支持]
 *
 * @retval 0，成功。
 * @retval -1，失败
 *
 */
int mdrv_crypto_secboot_supported(unsigned char * pu8Data);
/* *
 * @brief 查询当前版本是否已经启用安全启动
 *
 * @par 描述:
 * 查询当前版本是否已经启用安全启动
 *
 * @param[out]  unsigned char *pu8Data 是否已经启用安全启动标志[1:启用   0:未启用]
 *
 * @retval 0，成功。
 * @retval -1，失败
 *
 */
int mdrv_crypto_sec_started(unsigned char * pu8Data);
/* *
 * @brief 启用安全启动
 *
 * @par 描述:
 * 启用安全启动
 *
 * @retval 0，成功。
 * @retval -1，失败
 *
 */
int mdrv_crypto_start_secure(void);
/* *
 * @brief 安全镜像判断
 *
 * @par 描述:
 * 安全镜像判断
 *
 * @retval 1，安全镜像
 * @retval 0，非安全镜像
 *
 */
unsigned int mdrv_crypto_check_sec(void);

#ifdef __cplusplus
}
#endif
#endif
