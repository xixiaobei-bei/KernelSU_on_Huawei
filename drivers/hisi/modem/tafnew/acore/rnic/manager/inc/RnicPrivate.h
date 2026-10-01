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

#ifndef __RNIC_PRIVATE_H__
#define __RNIC_PRIVATE_H__

/*****************************************************************************
 1. Other files included
*****************************************************************************/
#if (VOS_OS_VER == VOS_LINUX)
#include <linux/kernel.h>
#else
#include "PsCommonDef.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*****************************************************************************
 2. Macro definitions
*****************************************************************************/
/*
#undef  pr_fmt
#define pr_fmt(fmt) "["HISI_LOG_TAG"]:" fmt

#define NAS_RNIC_TAG "NAS_RNIC"

#if (VOS_OS_VER == VOS_LINUX)
#define HISI_LOG_TAG NAS_RNIC_TAG
#endif
*/

#if (VOS_OS_VER == VOS_LINUX)
/*lint -emacro({717}, RNIC_PR_LOGH)*/
#define RNIC_PR_LOGH(fmt, ...) \
			do { \
				pr_err("[NAS_RNIC]: [HIGH] %s "fmt"\n", __func__, ##__VA_ARGS__); \
			} while(0)

/*lint -emacro({717}, RNIC_PR_LOGE)*/
#define RNIC_PR_LOGE(fmt, ...) \
			do { \
				pr_err("[NAS_RNIC]: [ERR] %s "fmt"\n", __func__, ##__VA_ARGS__); \
			} while(0)

/*lint -emacro({717}, RNIC_PR_LOGI)*/
#define RNIC_PR_LOGI(fmt, ...) \
			do { \
				pr_info("[NAS_RNIC]: [INFO] %s "fmt"\n", __func__, ##__VA_ARGS__); \
			} while(0)

/*lint -emacro({717}, RNIC_PR_LOGD)*/
#define RNIC_PR_LOGD(fmt, ...) \
			do { \
				pr_debug("[NAS_RNIC]: [DBG] %s "fmt"\n", __func__, ##__VA_ARGS__); \
			} while(0)
#else
#define RNIC_PR_LOGH                                    PS_PRINTF
#define RNIC_PR_LOGE                                    PS_PRINTF
#define RNIC_PR_LOGI                                    PS_PRINTF
#define RNIC_PR_LOGD                                    PS_PRINTF
#endif


/*****************************************************************************
 3. Enumerations declarations
*****************************************************************************/


/*****************************************************************************
 4. STRUCT and UNION declaration
*****************************************************************************/


/*****************************************************************************
 5. Global declaration
*****************************************************************************/


/*****************************************************************************
 6. Function declarations
*****************************************************************************/


/*****************************************************************************
 7. OTHER declarations
*****************************************************************************/


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __RNIC_PRIVATE_H__ */
