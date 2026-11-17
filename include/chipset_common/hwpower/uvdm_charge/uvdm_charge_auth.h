/*
 * uvdm_charge_auth.h
 *
 * authenticate for uvdm charge
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

#ifndef _UVDM_CHARGE_AUTH_H_
#define _UVDM_CHARGE_AUTH_H_

#define UVDM_AUTH_HASH_LEN         16
#define UVDM_AUTH_WAIT_TIMEOUT     1000
#define UVDM_AUTH_GENL_OPS_NUM     1

#ifdef CONFIG_UVDM_CHARGER
bool uvdm_auth_get_srv_state(void);
int uvdm_auth_wait_completion(void);
void uvdm_auth_clean_hash_data(void);
u8 *uvdm_auth_get_hash_data_header(void);
unsigned int uvdm_auth_get_hash_data_size(void);
#else
static inline bool uvdm_auth_get_srv_state(void)
{
	return false;
}

static inline void uvdm_auth_clean_hash_data(void)
{
}

static inline u8 *uvdm_auth_get_hash_data_header(void)
{
	return NULL;
}

static inline unsigned int uvdm_auth_get_hash_data_size(void)
{
	return 0;
}

static inline int uvdm_auth_wait_completion(void)
{
	return -1;
}
#endif /* CONFIG_UVDM_CHARGER */

#endif /* _UVDM_CHARGE_AUTH_H_ */
