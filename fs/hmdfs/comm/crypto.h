/* SPDX-License-Identifier: GPL-2.0 */
/*
 * crypto.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Description: Crypto for MDFS communication
 * Author: wangminmin4@huawei.com
 *	   liuxuesong3@huawei.com
 * Create: 2020-06-08
 *
 */

#ifndef HMDFS_CRYPTO_H
#define HMDFS_CRYPTO_H

#include "transport.h"

#define MAX_LABEL_SIZE	   30
#define CRYPTO_IV_OFFSET   0
#define CRYPTO_SALT_OFFSET (CRYPTO_IV_OFFSET + TLS_CIPHER_AES_GCM_128_IV_SIZE)
#define CRYPTO_SEQ_OFFSET                                                      \
	(CRYPTO_SALT_OFFSET + TLS_CIPHER_AES_GCM_128_SALT_SIZE)
#define REKEY_LIFETIME (60 * 60 * HZ)

enum HKDF_TYPE {
	HKDF_TYPE_KEY_INITIATOR  = 0,
	HKDF_TYPE_KEY_ACCEPTER   = 1,
	HKDF_TYPE_REKEY          = 2,
	HKDF_TYPE_IV             = 3,
#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	HKDF_TYPE_D2DP_INITIATOR = 4,
	HKDF_TYPE_D2DP_ACCEPTER  = 5,
	HKDF_TYPE_D2DP_REKEY     = 6,
#endif
	HKDF_TYPE_MAX,
};

enum SET_CRYPTO_TYPE {
	SET_CRYPTO_SEND = 0,
	SET_CRYPTO_RECV = 1,
};

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
int d2dp_encrypt(struct d2d_security *self, void *src, size_t src_len,
		 void *dst, size_t dst_len);
int d2dp_decrypt(struct d2d_security *self, void *src, size_t src_len,
		 void *dst, size_t dst_len);
#endif /* CONFIG_HMDFS_D2DP_TRANSPORT */

int tls_crypto_info_init(struct connection *conn_impl);
int set_crypto_info(struct connection *conn_impl, int set_type);
int update_key(__u8 *old_key, __u8 *new_key, int type);

#endif
