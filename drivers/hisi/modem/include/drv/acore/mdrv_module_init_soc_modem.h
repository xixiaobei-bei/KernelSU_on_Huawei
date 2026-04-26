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
 *  @brief   module_init模块在soc平台acore上的对外头文件
 *  @file    mdrv_module_init_soc_modem.h
 *  @version v1.0
 *  @date    2019.12.04
 *  <ul><li>v1.0|2019.12.04|创建文件</li></ul>
 *  @since
 */
#ifndef __MDRV_MODULE_INIT_SOC_MODEM_H__
#define __MDRV_MODULE_INIT_SOC_MODEM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(DRV_BUILD_SEPARATE)
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
extern int APP_VCOM_Init(void);
extern int dms_file_init(void);
#endif
extern int nm_init(void);
extern int rnic_init(void);
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_OFF)
extern int ADS_PlatDevInit(void);
#endif
extern int act_cdev_init(void);
#endif

/**
* @brief modem初始化函数指针类型
*/
typedef int (*modem_module_init_func)(void);

/**
* @brief modem初始化函数列表
*/
static modem_module_init_func g_modem_ps_module_init_func[] = {
#if !defined(DRV_BUILD_SEPARATE)
    /* OAM GU begin */
    /* OAM GU end */
    /* OAM TL begin */
    /* OAM TL end */
    /* PS GU begin */
    /* PS GU end */
    /* PS TL begin */
    /* PS TL end */
    /* TAF begin */
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
    APP_VCOM_Init,
    dms_file_init,
#endif
    nm_init,
    rnic_init,
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_OFF)
    ADS_PlatDevInit,
#endif
    /* TAF end */
    /* IMS begin */
    act_cdev_init,
    /* IMS end */
#endif
};

#ifdef __cplusplus
}
#endif
#endif

