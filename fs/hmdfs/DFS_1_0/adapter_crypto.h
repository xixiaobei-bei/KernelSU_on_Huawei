/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_crypto.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Description: Crypto for fusion communication
 * Author: wangminmin4@huawei.com
 * Create: 2020-03-31
 *
 */

#ifndef HMDFS_ADAPTER_CRYPTO_H
#define HMDFS_ADAPTER_CRYPTO_H

#include "comm/transport.h"
#include <linux/types.h>

int aeadcipher_encrypt_buffer(struct connection *con, __u8 *src_buf,
			      size_t src_len, __u8 *dst_buf, size_t dst_len);
int aeadcipher_decrypt_buffer(struct connection *con, __u8 *src_buf,
			      size_t src_len, __u8 *dst_buf, size_t dst_len);
#endif
