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

#ifndef __RNIC_ONDEMAND_H__
#define __RNIC_ONDEMAND_H__

/*****************************************************************************
  1. Other file included
*****************************************************************************/

#include <linux/types.h>
#include "rnic_ondemand_i.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*****************************************************************************
  2. Macro definitions
*****************************************************************************/

#if (defined(LLT_OS_VER))
#define STATIC
#else
#define STATIC static
#endif

#define RNIC_PROC_BUF_SIZE			(16)

#define RNIC_PROC_FILE_PATH			"dial"
#define RNIC_PROC_FILE_ONDEMAND		"ondemand"
#define RNIC_PROC_FILE_IDEL_TIME	"idle_timeout"
#define RNIC_PROC_FILE_EVENT_REPORT	"dial_event_report"

#define RNIC_ONDEMAND_ISDIGIT(c)	(((c) >= '0') && ((c) <= '9'))


/*****************************************************************************
  3. Enumerations declatations
*****************************************************************************/

enum rnic_ondemand_config_type_e {
	RNIC_CFG_TYPE_DIAL_MODE = 0,
	RNIC_CFG_TYPE_IDLE_TIME,
	RNIC_CFG_TYPE_EVENT_REPORT,
	RNIC_CFG_TYPE_MAX
};


/*****************************************************************************
  4. STRUCT and UNION declatations
*****************************************************************************/

struct rnic_ondemand_context_s {
	uint32_t cfg_value[RNIC_CFG_TYPE_MAX];
	rnic_ondemand_config_func cfg_func[RNIC_CFG_TYPE_MAX];
};


/*****************************************************************************
 5. Global declaration
*****************************************************************************/


/*******************************************************************************
  6. Function declarations
*******************************************************************************/


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __RNIC_ONDEMAND_H__ */
