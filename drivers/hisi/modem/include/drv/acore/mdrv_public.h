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

#ifndef __MDRV_ACORE_PUBLIC_H__
#define __MDRV_ACORE_PUBLIC_H__
#ifdef __cplusplus
extern "C"
{
#endif

/**
 *  @brief   acore对外公共头文件
 *  @file    mdrv_public.h
 *  @author  tongyuan
 *  @version v1.0
 *  @date    2019.11.26
 *  @note    修改记录(版本号|修订日期|说明)
 *  <ul><li>v1.0|2019.11.26|接口自动化整改</li></ul>
 *  <ul><li>v1.0|2020.1.15|删除无用的函数指针接口</li></ul>
 *  @since   DRV1.0
 */

/**
* @brief 底层驱动返回OK
*/
#define MDRV_OK                  (0)

/**
* @brief 底层驱动返回ERROR
*/
#define MDRV_ERROR               (-1)

/**
 * @brief 参数为int型的函数指针接口
 *
 * @par 描述:
 * 参数为int型的函数指针接口
 *
 * @attention
 *
 * @param[in] int型的函数入参，含义取决于具体函数定义
 *
 * @retval int型，返回值具体取决于函数定义
 *
 * @par 依赖:
 * <ul><li>mdrv_public.h：该接口声明所在的头文件。</li></ul>
 *
 * @see
 */
typedef int (*funcptr_1)(int);

#ifndef __VOIDFUNCPTR
/**
* @brief 定义空函数指针
*/
#define __VOIDFUNCPTR

/**
 * @brief 参数为无符号整型的函数指针接口
 *
 * @par 描述:
 * 参数为无符号整型的函数指针接口
 *
 * @attention
 *
 * @param unsigned int型，具体含义取决于函数定义
 *
 * @retval void
 *
 * @par 依赖:
 * <ul><li>mdrv_public.h：该接口声明所在的头文件。</li></ul>
 *
 * @see
 */
typedef void (*voidfuncptr)(unsigned int);
#endif

#ifdef __cplusplus
}
#endif
#endif
