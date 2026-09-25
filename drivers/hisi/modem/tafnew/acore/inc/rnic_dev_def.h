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

#ifndef __RNIC_DEV_DEF_H__
#define __RNIC_DEV_DEF_H__

/*****************************************************************************
  1. Other file included
*****************************************************************************/

#include "product_config.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*****************************************************************************
  2. Macro definitions
*****************************************************************************/

/*****************************************************************************
  3. Enumerations declatations
*****************************************************************************/

enum rnic_dev_id_e {
	RNIC_DEV_ID_RMNET0 = 0,
	RNIC_DEV_ID_RMNET1,
	RNIC_DEV_ID_RMNET2,
	RNIC_DEV_ID_RMNET3,
	RNIC_DEV_ID_RMNET4,
	RNIC_DEV_ID_RMNET5,
	RNIC_DEV_ID_RMNET6,
	RNIC_DEV_ID_DATA_MAX = RNIC_DEV_ID_RMNET6,

#if ((FEATURE_ON == FEATURE_IMS) && (FEATURE_ON == FEATURE_DELAY_MODEM_INIT))
	RNIC_DEV_ID_RMNET_IMS00,
	RNIC_DEV_ID_RMNET_IMS10,
	RNIC_DEV_ID_RMNET_EMC0,
	RNIC_DEV_ID_RMNET_EMC1,

	RNIC_DEV_ID_RMNET_R_IMS00,
	RNIC_DEV_ID_RMNET_R_IMS01,
	RNIC_DEV_ID_RMNET_R_IMS10,
	RNIC_DEV_ID_RMNET_R_IMS11,

	RNIC_DEV_ID_RMNET_TUN00,
	RNIC_DEV_ID_RMNET_TUN01,
	RNIC_DEV_ID_RMNET_TUN02,
	RNIC_DEV_ID_RMNET_TUN03,
	RNIC_DEV_ID_RMNET_TUN04,

	RNIC_DEV_ID_RMNET_TUN10,
	RNIC_DEV_ID_RMNET_TUN11,
	RNIC_DEV_ID_RMNET_TUN12,
	RNIC_DEV_ID_RMNET_TUN13,
	RNIC_DEV_ID_RMNET_TUN14,
	RNIC_DEV_ID_IMS_MAX = RNIC_DEV_ID_RMNET_TUN14,
#endif /* FEATURE_ON == FEATURE_IMS && FEATURE_ON == FEATURE_DELAY_MODEM_INIT */

	RNIC_DEV_ID_BUTT
};
typedef unsigned char RNIC_DEV_ID_ENUM_UINT8;



#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */


#endif /* __RNIC_DEV_DEF_H__ */

