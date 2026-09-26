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

#include "crypto.h"

#include <crypto/aead.h>
#include <crypto/hash.h>
#include <linux/tcp.h>
#include <net/inet_connection_sock.h>
#include <net/tcp_states.h>
#include <net/tls.h>

#include "hmdfs.h"

static void tls_crypto_set_key(struct connection *conn_impl, int tx)
{
	int rc = 0;
	struct tcp_handle *tcp = conn_impl->connect_handle;
	struct tls_context *ctx = tls_get_ctx(tcp->sock->sk);
	struct cipher_context *cctx = NULL;
	struct tls_sw_context_tx *sw_ctx_tx = NULL;
	struct tls_sw_context_rx *sw_ctx_rx = NULL;
	struct crypto_aead **aead = NULL;
	struct tls12_crypto_info_aes_gcm_128 *crypto_info = NULL;

	if (tx) {
		crypto_info = &conn_impl->send_crypto_info;
		cctx = &ctx->tx;
		sw_ctx_tx = tls_sw_ctx_tx(ctx);
		aead = &sw_ctx_tx->aead_send;
	} else {
		crypto_info = &conn_impl->recv_crypto_info;
		cctx = &ctx->rx;
		sw_ctx_rx = tls_sw_ctx_rx(ctx);
		aead = &sw_ctx_rx->aead_recv;
	}

	memcpy(cctx->iv, crypto_info->salt, TLS_CIPHER_AES_GCM_128_SALT_SIZE);
	memcpy(cctx->iv + TLS_CIPHER_AES_GCM_128_SALT_SIZE, crypto_info->iv,
	       TLS_CIPHER_AES_GCM_128_IV_SIZE);
	memcpy(cctx->rec_seq, crypto_info->rec_seq,
	       TLS_CIPHER_AES_GCM_128_REC_SEQ_SIZE);
	rc = crypto_aead_setkey(*aead, crypto_info->key,
				TLS_CIPHER_AES_GCM_128_KEY_SIZE);
	if (rc)
		hmdfs_err("crypto set key error");
}

int tls_crypto_info_init(struct connection *conn_impl)
{
	int ret = 0;
	u8 key_meterial[HMDFS_KEY_SIZE];
	struct tcp_handle *tcp =
		(struct tcp_handle *)(conn_impl->connect_handle);
	if (conn_impl->node->version < DFS_2_0 || !tcp)
		return -EINVAL;
	// send
	update_key(conn_impl->send_key, key_meterial, HKDF_TYPE_IV);
	ret = kernel_setsockopt(tcp->sock, SOL_TCP, TCP_ULP, "tls",
				sizeof("tls"));
	if (ret)
		hmdfs_err("set tls error %d", ret);
	tcp->connect->send_crypto_info.info.version = TLS_1_2_VERSION;
	tcp->connect->send_crypto_info.info.cipher_type =
		TLS_CIPHER_AES_GCM_128;

	memcpy(tcp->connect->send_crypto_info.key, tcp->connect->send_key,
	       TLS_CIPHER_AES_GCM_128_KEY_SIZE);
	memcpy(tcp->connect->send_crypto_info.iv,
	       key_meterial + CRYPTO_IV_OFFSET, TLS_CIPHER_AES_GCM_128_IV_SIZE);
	memcpy(tcp->connect->send_crypto_info.salt,
	       key_meterial + CRYPTO_SALT_OFFSET,
	       TLS_CIPHER_AES_GCM_128_SALT_SIZE);
	memcpy(tcp->connect->send_crypto_info.rec_seq,
	       key_meterial + CRYPTO_SEQ_OFFSET,
	       TLS_CIPHER_AES_GCM_128_REC_SEQ_SIZE);

	ret = kernel_setsockopt(tcp->sock, SOL_TLS, TLS_TX,
				(char *)&(tcp->connect->send_crypto_info),
				sizeof(tcp->connect->send_crypto_info));
	if (ret)
		hmdfs_err("set tls send_crypto_info error %d", ret);

	// recv
	update_key(tcp->connect->recv_key, key_meterial, HKDF_TYPE_IV);
	tcp->connect->recv_crypto_info.info.version = TLS_1_2_VERSION;
	tcp->connect->recv_crypto_info.info.cipher_type =
		TLS_CIPHER_AES_GCM_128;

	memcpy(tcp->connect->recv_crypto_info.key, tcp->connect->recv_key,
	       TLS_CIPHER_AES_GCM_128_KEY_SIZE);
	memcpy(tcp->connect->recv_crypto_info.iv,
	       key_meterial + CRYPTO_IV_OFFSET, TLS_CIPHER_AES_GCM_128_IV_SIZE);
	memcpy(tcp->connect->recv_crypto_info.salt,
	       key_meterial + CRYPTO_SALT_OFFSET,
	       TLS_CIPHER_AES_GCM_128_SALT_SIZE);
	memcpy(tcp->connect->recv_crypto_info.rec_seq,
	       key_meterial + CRYPTO_SEQ_OFFSET,
	       TLS_CIPHER_AES_GCM_128_REC_SEQ_SIZE);
	memset(key_meterial, 0, HMDFS_KEY_SIZE);

	ret = kernel_setsockopt(tcp->sock, SOL_TLS, TLS_RX,
				(char *)&(tcp->connect->recv_crypto_info),
				sizeof(tcp->connect->recv_crypto_info));
	if (ret)
		hmdfs_err("set tls recv_crypto_info error %d", ret);
	return ret;
}

static int tls_set_tx(struct tcp_handle *tcp)
{
	int ret = 0;
	u8 new_key[HMDFS_KEY_SIZE];
	u8 key_meterial[HMDFS_KEY_SIZE];

	ret = update_key(tcp->connect->send_key, new_key, HKDF_TYPE_REKEY);
	if (ret < 0)
		return ret;
	memcpy(tcp->connect->send_key, new_key, HMDFS_KEY_SIZE);
	ret = update_key(tcp->connect->send_key, key_meterial, HKDF_TYPE_IV);
	if (ret < 0)
		return ret;

	memcpy(tcp->connect->send_crypto_info.key, tcp->connect->send_key,
	       TLS_CIPHER_AES_GCM_128_KEY_SIZE);
	memcpy(tcp->connect->send_crypto_info.iv,
	       key_meterial + CRYPTO_IV_OFFSET, TLS_CIPHER_AES_GCM_128_IV_SIZE);
	memcpy(tcp->connect->send_crypto_info.salt,
	       key_meterial + CRYPTO_SALT_OFFSET,
	       TLS_CIPHER_AES_GCM_128_SALT_SIZE);
	memcpy(tcp->connect->send_crypto_info.rec_seq,
	       key_meterial + CRYPTO_SEQ_OFFSET,
	       TLS_CIPHER_AES_GCM_128_REC_SEQ_SIZE);
	memset(new_key, 0, HMDFS_KEY_SIZE);
	memset(key_meterial, 0, HMDFS_KEY_SIZE);

	tls_crypto_set_key(tcp->connect, 1);
	return 0;
}

static int tls_set_rx(struct tcp_handle *tcp)
{
	int ret = 0;
	u8 new_key[HMDFS_KEY_SIZE];
	u8 key_meterial[HMDFS_KEY_SIZE];

	ret = update_key(tcp->connect->recv_key, new_key, HKDF_TYPE_REKEY);
	if (ret < 0)
		return ret;
	memcpy(tcp->connect->recv_key, new_key, HMDFS_KEY_SIZE);
	ret = update_key(tcp->connect->recv_key, key_meterial, HKDF_TYPE_IV);
	if (ret < 0)
		return ret;

	memcpy(tcp->connect->recv_crypto_info.key, tcp->connect->recv_key,
	       TLS_CIPHER_AES_GCM_128_KEY_SIZE);
	memcpy(tcp->connect->recv_crypto_info.iv,
	       key_meterial + CRYPTO_IV_OFFSET, TLS_CIPHER_AES_GCM_128_IV_SIZE);
	memcpy(tcp->connect->recv_crypto_info.salt,
	       key_meterial + CRYPTO_SALT_OFFSET,
	       TLS_CIPHER_AES_GCM_128_SALT_SIZE);
	memcpy(tcp->connect->recv_crypto_info.rec_seq,
	       key_meterial + CRYPTO_SEQ_OFFSET,
	       TLS_CIPHER_AES_GCM_128_REC_SEQ_SIZE);
	memset(new_key, 0, HMDFS_KEY_SIZE);
	memset(key_meterial, 0, HMDFS_KEY_SIZE);
	tls_crypto_set_key(tcp->connect, 0);
	return 0;
}

int set_crypto_info(struct connection *conn_impl, int set_type)
{
	int ret = 0;
	__u8 version = conn_impl->node->version;
	struct tcp_handle *tcp =
		(struct tcp_handle *)(conn_impl->connect_handle);
	if (version < DFS_2_0 || !tcp)
		return -EINVAL;

	if (set_type == SET_CRYPTO_SEND) {
		ret = tls_set_tx(tcp);
		if (ret) {
			hmdfs_err("tls set tx fail");
			return ret;
		}
	}
	if (set_type == SET_CRYPTO_RECV) {
		ret = tls_set_rx(tcp);
		if (ret) {
			hmdfs_err("tls set rx fail");
			return ret;
		}
	}
	hmdfs_info("KTLS setting success");
	return ret;
}

static int hmac_sha256(u8 *key, u8 key_len, char *info, u8 info_len, u8 *output)
{
	struct crypto_shash *tfm = NULL;
	struct shash_desc *shash = NULL;
	int ret = 0;

	if (!key)
		return -EINVAL;

	tfm = crypto_alloc_shash("hmac(sha256)", 0, 0);
	if (IS_ERR(tfm)) {
		hmdfs_err("crypto_alloc_ahash failed: err %ld", PTR_ERR(tfm));
		return PTR_ERR(tfm);
	}

	ret = crypto_shash_setkey(tfm, key, key_len);
	if (ret) {
		hmdfs_err("crypto_ahash_setkey failed: err %d", ret);
		goto failed;
	}

	shash = kzalloc(sizeof(*shash) + crypto_shash_descsize(tfm),
			GFP_KERNEL);
	if (!shash) {
		ret = -ENOMEM;
		goto failed;
	}

	shash->tfm = tfm;

	ret = crypto_shash_digest(shash, info, info_len, output);

	kfree(shash);

failed:
	crypto_free_shash(tfm);
	return ret;
}

static const char *const g_key_lable[] = { "ktls key initiator",
					   "ktls key accepter",
					   "ktls key update", "ktls iv&salt" };
static const int g_key_lable_len[] = { 18, 17, 15, 12 };

int update_key(__u8 *old_key, __u8 *new_key, int type)
{
	int ret = 0;
	char lable[MAX_LABLE_SIZE];
	u8 lable_size;

	lable_size = g_key_lable_len[type] + sizeof(u16) + sizeof(char);
	*((u16 *)lable) = HMDFS_KEY_SIZE;
	memcpy(lable + sizeof(u16), g_key_lable[type], g_key_lable_len[type]);
	*(lable + sizeof(u16) + g_key_lable_len[type]) = 0x01;
	ret = hmac_sha256(old_key, HMDFS_KEY_SIZE, lable, lable_size, new_key);
	if (ret < 0)
		hmdfs_err("hmac sha256 error");
	return ret;
}
