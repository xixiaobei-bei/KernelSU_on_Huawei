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

#ifndef __DIAG_VCOM_H__
#define __DIAG_VCOM_H__

/*****************************************************************************
  1. Other file included
*****************************************************************************/

#include <linux/types.h>


/*****************************************************************************
  2. Macro definitions
*****************************************************************************/

#define DIAG_VCOM_OK	0
#define DIAG_VCOM_ERROR 1


/*****************************************************************************
  3. Enumerations declatations
*****************************************************************************/

enum diag_vcom_event_e {
	DIAG_VCOM_EVT_CHAN_CLOSE = 0,
	DIAG_VCOM_EVT_CHAN_OPEN,
	DIAG_VCOM_EVT_MAX
};

enum diag_vcom_channel_e {
	DIAG_VCOM_CHAN_CTRL = 0,
	DIAG_VCOM_CHAN_DATA,
	DIAG_VCOM_CHAN_MAX
};

enum diag_vcom_om_data_mode_e {
	DIAG_VCOM_OM_DATA_MODE_TRANSPARENT = 0,
	DIAG_VCOM_OM_DATA_MODE_COMPRESSED
};


/*****************************************************************************
  4. STRUCT and UNION declatations
*****************************************************************************/

struct diag_vcom_cb_ops_s {
	void (*event_cb)(uint32_t channel, uint32_t event);
	uint32_t (*data_rx_cb)(uint32_t channel, uint8_t *data, uint32_t length);
};


/*******************************************************************************
  5. Function declarations
*******************************************************************************/

#ifdef CONFIG_DIAG_NETLINK
extern int diag_vcom_register_ops(uint32_t channel, struct diag_vcom_cb_ops_s *ops);

extern int diag_vcom_report_om_data(uint32_t channel, uint8_t *data, uint32_t len, uint8_t mode);

#else
static inline int diag_vcom_register_ops(uint32_t channel, struct diag_vcom_cb_ops_s *ops){return 0;};
static inline int diag_vcom_report_om_data(uint32_t channel, uint8_t *data, uint32_t len, uint8_t mode){return 0;};
static inline int diag_vcom_init(void){return 0;};

#endif

#endif /* __DIAG_VCOM_H__ */
