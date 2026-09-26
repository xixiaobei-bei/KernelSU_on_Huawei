/*
 * wireless_rx_auth.h
 *
 * authenticate for wireless rx charge
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _WIRELESS_RX_AUTH_H_
#define _WIRELESS_RX_AUTH_H_

#define WLRX_AUTH_RANDOM_LEN       8
#define WLRX_AUTH_TX_KEY_LEN       8
#define WLRX_AUTH_HASH_LEN         (WLRX_AUTH_RANDOM_LEN + WLRX_AUTH_TX_KEY_LEN)
#define WLRX_AUTH_WAIT_TIMEOUT     2000
#define WLRX_AUTH_GENL_OPS_NUM     1

#ifdef CONFIG_WIRELESS_CHARGER
bool wlrx_auth_get_srv_state(void);
int wlrx_auth_wait_completion(void);
void wlrx_auth_clean_hash_data(void);
u8 *wlrx_auth_get_hash_data_header(void);
unsigned int wlrx_auth_get_hash_data_size(void);
#else
static inline bool wlrx_auth_get_srv_state(void)
{
	return false;
}

static inline void wlrx_auth_clean_hash_data(void)
{
}

static inline u8 *wlrx_auth_get_hash_data_header(void)
{
	return NULL;
}

static inline unsigned int wlrx_auth_get_hash_data_size(void)
{
	return 0;
}

static inline int wlrx_auth_wait_completion(void)
{
	return -1;
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_RX_AUTH_H_ */
