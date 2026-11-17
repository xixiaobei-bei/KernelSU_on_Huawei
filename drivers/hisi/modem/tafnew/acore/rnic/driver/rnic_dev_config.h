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

#ifndef __RNIC_DEV_CONFIG_H__
#define __RNIC_DEV_CONFIG_H__

/*****************************************************************************
  1. Other file included
*****************************************************************************/

#include <linux/types.h>
#include "rnic_dev_i.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#ifdef WIN32
#pragma warning(disable:4200) /* zero-sized array in struct/union */
#endif

/*****************************************************************************
  2. Macro definitions
*****************************************************************************/

#define RNIC_CFG_MSG_MAX_LEN		(4096)
#define RNIC_DEV_REQUEST_MSG_BASE	(100)
#define RNIC_DEV_UNSOL_MSG_BASE		(200)

#define RNIC_DEV_MSG_OFFSETOF(_m)	(offsetof(struct rnic_ctrl_msg_s, _m))
#define RNIC_DEV_MSG_ARGS_SIZE(_m)	(sizeof(((struct rnic_ctrl_msg_s *)0)->_m))

#define RNIC_DEV_MSG_HDR_LEN 		RNIC_DEV_MSG_OFFSETOF(args_start)
#define RNIC_DEV_MSG_MAX_PAYLOAD	(sizeof(struct rnic_ctrl_msg_s) - RNIC_DEV_MSG_HDR_LEN)


/*****************************************************************************
  3. Enumerations declatations
*****************************************************************************/

enum rnic_vcom_message_types_e {
	RNIC_REQUEST_SET_NAPI_BASE_CONFIG = RNIC_DEV_REQUEST_MSG_BASE,
	RNIC_REQUEST_SET_NAPI_WEIGHT,
	RNIC_REQUEST_SET_NAPI_LB_CONFIG,
	RNIC_REQUEST_SET_NAPI_LB_LEVEL,
	RNIC_REQUEST_SET_PS_IFACE_UP,
	RNIC_REQUEST_SET_PS_IFACE_DOWN,

	RNIC_UNSOL_REPORT_DATA_STATS = RNIC_DEV_UNSOL_MSG_BASE,
	RNIC_UNSOL_REPORT_NAPI_STATS,
	RNIC_UNSOL_REPORT_PETH_STATS,
};


/*****************************************************************************
  4. STRUCT and UNION declatations
*****************************************************************************/

struct rnic_ctrl_msg_s {
	uint16_t message_type;
	uint16_t reserved;
	uint32_t args_length;
	union {
		uint32_t args_start;
		struct rnic_napi_config_s napi_base_config;
		struct rnic_napi_lb_config_s napi_lb_config;
		struct {
			uint8_t devid;
			uint8_t weight;
		} napi_weight;
		struct {
			uint8_t devid;
			uint8_t level;
		} napi_lb_level;
		struct {
			uint8_t devid;
			uint8_t ip_family;
		} iface_config;
		struct {
			uint8_t devid;
			struct rnic_data_stats_s stats;
		} data_stats;
		struct {
			uint8_t devid;
			struct rnic_napi_stats_s stats;
		} napi_stats;
	};
};

struct rnic_icc_debug_ctx_s {
	uint32_t rcv_total_cnt;
	uint32_t rcv_abnormal_cnt;
	uint32_t snd_total_cnt;
	uint32_t snd_fail_cnt;
};


/*****************************************************************************
 5. Global declaration
*****************************************************************************/

/*******************************************************************************
  6. Function declarations
*******************************************************************************/

#if (defined(CONFIG_HISI_BALONG_EXTRA_MODEM))
int rnic_rc_report_data_stats(uint8_t devid);
int rnic_rc_report_napi_stats(uint8_t devid);
#endif /* CONFIG_HISI_BALONG_EXTRA_MODEM */


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __RNIC_DEV_CONFIG_H__ */
