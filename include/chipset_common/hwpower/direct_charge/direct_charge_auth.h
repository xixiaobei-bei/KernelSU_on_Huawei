/*
 * direct_charge_auth.h
 *
 * authenticate for direct charge
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

#ifndef _DIRECT_CHARGE_AUTH_H_
#define _DIRECT_CHARGE_AUTH_H_

#define DC_AUTH_DIGEST_LEN         16
#define DC_AUTH_HASH_LEN           (DC_AUTH_DIGEST_LEN * 2 + 1)
#define DC_AUTH_WAIT_TIMEOUT       1000
#define DC_AUTH_GENL_OPS_NUM       1

#ifdef CONFIG_DIRECT_CHARGER
bool dc_auth_get_srv_state(void);
int dc_auth_wait_completion(void);
void dc_auth_clean_hash_data(void);
u8 *dc_auth_get_hash_data_header(void);
unsigned int dc_auth_get_hash_data_size(void);
#else
static inline bool dc_auth_get_srv_state(void)
{
	return false;
}

static inline void dc_auth_clean_hash_data(void)
{
}

static inline u8 *dc_auth_get_hash_data_header(void)
{
	return NULL;
}

static inline unsigned int dc_auth_get_hash_data_size(void)
{
	return 0;
}

static inline int dc_auth_wait_completion(void)
{
	return -1;
}
#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGE_AUTH_H_ */
