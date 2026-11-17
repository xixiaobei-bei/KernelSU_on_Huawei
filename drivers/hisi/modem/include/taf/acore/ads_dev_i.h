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

#ifndef __ADS_DEV_I_H__
#define __ADS_DEV_I_H__

/*****************************************************************************
  1. Other file included
*****************************************************************************/

#include <linux/skbuff.h>


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

/*****************************************************************************
  4. STRUCT and UNION declatations
*****************************************************************************/

typedef int (*ads_iface_rx_func_t)(unsigned long user_data, struct sk_buff *skb);
typedef void (*ads_iface_rx_cmplt_func_t)(unsigned long user_data);

struct ads_iface_rx_handler_s {
	unsigned long user_data;
	ads_iface_rx_func_t rx_func;
	ads_iface_rx_cmplt_func_t rx_cmplt_func;
};


/*******************************************************************************
  5. Function declarations
*******************************************************************************/

/**
 * ads_iface_tx - IFACE data tx handler
 * @iface_id: iface to send the packet to
 * @skb: packet to send (require 14 bytes headroom reserved)
 *
 * For errors, client does not needs to free the skb.
 *
 * The protocol of skb shall be set before calling the function.
 * IPv4 - htons(ETH_P_IP)
 * IPv6 - htons(ETH_P_IPV6)
 * Others - 0
 *
 * Returns: on success, negative on failure
 */
int ads_iface_tx(unsigned char iface_id, struct sk_buff *skb);

/**
 * ads_iface_register_rx_handler - Register IFACE data rx handler
 * @iface_id: iface to register the rx callback
 * @rx_handler: rx callback
 *
 * The protocol of skb is set before calling the rx callback function.
 * IPv4 - htons(ETH_P_IP)
 * IPv6 - htons(ETH_P_IPV6)
 * Others - 0
 *
 * Returns: on success, negative on failure
 */
int ads_iface_register_rx_handler(unsigned char iface_id,
					struct ads_iface_rx_handler_s *rx_handler);

/**
 * ads_iface_deregister_rx_handler - Deregister IFACE data rx handler
 * @iface_id: iface to register the rx callback
 *
 * Returns: on success, negative on failure
 */
int ads_iface_deregister_rx_handler(unsigned char iface_id);



#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __ADS_DEV_I_H__ */

