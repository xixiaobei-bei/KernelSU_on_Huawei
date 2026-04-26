/*
 * wireless_tx_auth.h
 *
 * authenticate for wireless tx charge
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

#ifndef _WIRELESS_TX_AUTH_H_
#define _WIRELESS_TX_AUTH_H_

#define WLTX_AUTH_RANDOM_LEN       8
#define WLTX_AUTH_HASH_LEN         8
#define WLTX_AUTH_WAIT_TIMEOUT     2000
#define WLTX_AUTH_GENL_OPS_NUM     1

#ifdef CONFIG_WIRELESS_CHARGER
bool wltx_auth_get_srv_state(void);
int wltx_auth_wait_completion(void);
void wltx_auth_clean_hash_data(void);
u8 *wltx_auth_get_hash_data_header(void);
unsigned int wltx_auth_get_hash_data_size(void);
#else
static inline bool wltx_auth_get_srv_state(void)
{
	return false;
}

static inline void wltx_auth_clean_hash_data(void)
{
}

static inline u8 *wltx_auth_get_hash_data_header(void)
{
	return NULL;
}

static inline unsigned int wltx_auth_get_hash_data_size(void)
{
	return 0;
}

static inline int wltx_auth_wait_completion(void)
{
	return -1;
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_TX_AUTH_H_ */
