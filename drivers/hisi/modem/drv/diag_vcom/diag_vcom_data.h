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

#ifndef __DIAG_VCOM_DATA_H__
#define __DIAG_VCOM_DATA_H__

/*****************************************************************************
  1. Other file included
*****************************************************************************/

#include <linux/types.h>


/*****************************************************************************
  2. Macro definitions
*****************************************************************************/

#define DIAG_VCOM_NETLINK_CTRL 31
#define DIAG_VCOM_NETLINK_DATA 29

#define DIAG_VCOM_MSG_COMMAND	  0
#define DIAG_VCOM_MSG_RETURN_CODE 1
#define DIAG_VCOM_MSG_RETURN_DATA 2

#define DIAG_VCOM_LOG_PORT_USB	0
#define DIAG_VCOM_LOG_PORT_VCOM 1

#define DIAG_VCOM_LOG_MODE_DIRECT 0
#define DIAG_VCOM_LOG_MODE_DELAY  1
#define DIAG_VCOM_LOG_MODE_CYCLE  2

#define DIAG_VCOM_LOG_NVE_DISABLE 0
#define DIAG_VCOM_LOG_NVE_ENABLE  1

#define DIAG_VCOM_DATA_TRANSPARENT 0
#define DIAG_VCOM_DATA_COMPROSSED  1

#define DIAG_VCOM_REQUEST_COMM_MSG_BASE   0
#define DIAG_VCOM_REQUEST_CONFIG_MSG_BASE 100
#define DIAG_VCOM_REQUEST_PROXY_MSG_BASE  300
#define DIAG_VCOM_UNSOL_MSG_BASE		  500


/*****************************************************************************
  3. Enumerations declatations
*****************************************************************************/

enum diag_vcom_message_types_e {
	DIAG_VCOM_REQUEST_ASSOCIATE_PORT = DIAG_VCOM_REQUEST_COMM_MSG_BASE + 1,
	DIAG_VCOM_REQUEST_UNASSOCIATE_PORT,

	DIAG_VCOM_REQUEST_SET_LOG_PORT_CONFIG = DIAG_VCOM_REQUEST_CONFIG_MSG_BASE +1,
	DIAG_VCOM_REQUEST_GET_LOG_PORT_CONFIG,
	DIAG_VCOM_REQUEST_SET_LOG_CONFIG,
	DIAG_VCOM_REQUEST_GET_LOG_CONFIG,
	DIAG_VCOM_REQUEST_SET_LOG_NVE_CONFIG,
	DIAG_VCOM_REQUEST_GET_LOG_NVE_CONFIG,

	DIAG_VCOM_REQUEST_SEND_OM_COMMAND = DIAG_VCOM_REQUEST_PROXY_MSG_BASE + 1,

	DIAG_VCOM_UNSOL_REPORT_OM_DATA = DIAG_VCOM_UNSOL_MSG_BASE + 1,
};

enum diag_vcom_error_code_e {
	DIAG_VCOM_E_SUCCESS = 0,
	DIAG_VCOM_E_NOT_READY,
	DIAG_VCOM_E_GENERIC_FAILURE,
	DIAG_VCOM_E_UNKNOWN_MESSAGE,
	DIAG_VCOM_E_INCORRECT_PARAMETERS
};


/*****************************************************************************
  4. STRUCT and UNION declatations
*****************************************************************************/

struct diag_vcom_msg_s {
	uint16_t message_type;
	uint16_t op;
	union {
		uint32_t args_length;
		uint32_t return_code;
	};
	union {
		uint32_t args_start;
		struct {
			uint32_t pid;
		} port;
		struct {
			uint32_t port;
		} log_port_config;
		struct {
			uint32_t mode;
		} log_config;
		struct {
			uint32_t enable;
		} log_nve_config;
		struct {
			uint32_t length;
			uint8_t data[0];
		} om_command;
	};
};

struct diag_vcom_unsol_msg_s {
	uint16_t message_type;
	uint16_t op;
	uint32_t args_length;
	union {
		uint32_t args_start;
		struct {
			uint32_t msg_seq;
			uint32_t blk_num;
			uint32_t blk_seq;
			uint8_t mode;
			uint8_t reserved[3];
			uint32_t length;
			uint8_t data[0];
		} om_data;
	};
};


#endif /*__DIAG_VCOM_DATA_H__ */
