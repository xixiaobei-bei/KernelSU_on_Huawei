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

#ifndef __DIAG_VCOM_DEBUG_H__
#define __DIAG_VCOM_DEBUG_H__

/*****************************************************************************
 1. Other files included
*****************************************************************************/

#include <linux/kernel.h>


/*****************************************************************************
 2. Macro definitions
*****************************************************************************/

#define DIAG_VCOM_LOG_LVL_HIGH (1 << 0)
#define DIAG_VCOM_LOG_LVL_ERR  (1 << 1)
#define DIAG_VCOM_LOG_LVL_INFO (1 << 2)
#define DIAG_VCOM_LOG_LVL_DBG  (1 << 3)

/*lint -emacro({717}, DIAG_VCOM_LOGH)*/
#define DIAG_VCOM_LOGH(fmt, ...) \
			do { \
				if (diag_vcom_log_level & DIAG_VCOM_LOG_LVL_HIGH) \
					pr_err("[DIAG:HIGH] %s: " fmt "\n", __func__, ##__VA_ARGS__); \
			} while(0)

/*lint -emacro({717}, DIAG_VCOM_LOGE)*/
#define DIAG_VCOM_LOGE(fmt, ...) \
			do { \
				if (diag_vcom_log_level & DIAG_VCOM_LOG_LVL_ERR) \
					pr_err("[DIAG:ERR] %s: " fmt "\n", __func__, ##__VA_ARGS__); \
			} while(0)

/*lint -emacro({717}, DIAG_VCOM_LOGI)*/
#define DIAG_VCOM_LOGI(fmt, ...)\
			do {\
				if (diag_vcom_log_level & DIAG_VCOM_LOG_LVL_INFO) \
					pr_err("[DIAG:INFO] %s: " fmt "\n", __func__, ##__VA_ARGS__); \
			} while(0)

/*lint -emacro({717}, DIAG_VCOM_LOGD)*/
#define DIAG_VCOM_LOGD(fmt, ...) \
			do {\
				if (diag_vcom_log_level & DIAG_VCOM_LOG_LVL_DBG) \
					pr_err("[DIAG:DBG] %s: " fmt "\n", __func__, ##__VA_ARGS__); \
			} while(0)


/*****************************************************************************
 3. Enumerations declarations
*****************************************************************************/


/*****************************************************************************
 4. STRUCT and UNION declaration
*****************************************************************************/


/*****************************************************************************
 5. Global declaration
*****************************************************************************/

extern unsigned int diag_vcom_log_level;


/*****************************************************************************
 6. Function declarations
*****************************************************************************/


/*****************************************************************************
 7. OTHER declarations
*****************************************************************************/



#endif /* end of diag_vcom_debug.h */
