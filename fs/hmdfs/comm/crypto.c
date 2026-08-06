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
#include <linux/string.h>
#include <linux/tcp.h>
#include <net/inet_connection_sock.h>
#include <net/tcp_states.h>
#include <net/tls.h>

#include "hmdfs.h"

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
#include "DFS_1_0/adapter_crypto.h"
#include "comm/transport.h"
#include "d2dp/d2d.h"

int d2dp_encrypt(struct d2d_security *self, void *src, size_t src_len,
		 void *dst, size_t dst_len)
{
	int res;
	struct d2dp_handle *d2dp = self->key_info;

	if (dst_len < src_len + self->cr_overhead) {
		hmdfs_err("bad msg size [%ld < %ld]\n",
			  dst_len, src_len + self->cr_overhead);
		return -EINVAL;
	}

	res = aeadcipher_encrypt_buffer(d2dp->send_tfm, src, src_len,
					dst, dst_len);
	if (res) {
		hmdfs_err("encrypt_buf fail");
		return -EINVAL;
	}

	return src_len + self->cr_overhead;
}

int d2dp_decrypt(struct d2d_security *self, void *src, size_t src_len,
		 void *dst, size_t dst_len)
{
	int res;
	struct d2dp_handle *d2dp = self->key_info;

	if (src_len <= self->cr_overhead ||
	    (src_len - self->cr_overhead) > dst_len) {
		hmdfs_err("bad msg size [%ld < %ld or %ld > %ld]\n",
			  src_len, self->cr_overhead,
			  src_len - self->cr_overhead, dst_len);
		return -EINVAL;
	}

	res = aeadcipher_decrypt_buffer(d2dp->recv_tfm, src, src_len,
					dst, dst_len);
	if (res) {
		hmdfs_err("decrypt_buf fail");
		return -EINVAL;
	}

	return src_len - self->cr_overhead;
}
#endif /* CONFIG_HMDFS_D2DP_TRANSPORT */

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

static void init_tls_info(struct tls12_crypto_info_aes_gcm_128 *info,
			  u8 *key_meterial, u8 *key, int key_len)
{
	info->info.version = TLS_1_2_VERSION;
	info->info.cipher_type = TLS_CIPHER_AES_GCM_128;

	memcpy(info->key, key, TLS_CIPHER_AES_GCM_128_KEY_SIZE);
	memcpy(info->iv, key_meterial + CRYPTO_IV_OFFSET,
	       TLS_CIPHER_AES_GCM_128_IV_SIZE);
	memcpy(info->salt, key_meterial + CRYPTO_SALT_OFFSET,
	       TLS_CIPHER_AES_GCM_128_SALT_SIZE);
	memcpy(info->rec_seq, key_meterial + CRYPTO_SEQ_OFFSET,
	       TLS_CIPHER_AES_GCM_128_REC_SEQ_SIZE);
}

int tls_crypto_info_init(struct connection *conn_impl)
{
	int ret = 0;
	u8 key_meterial[HMDFS_KEY_SIZE];
	struct tcp_handle *tcp = conn_impl->connect_handle;

	if (conn_impl->node->version < DFS_2_0 || !tcp)
		return -EINVAL;
	// send
	update_key(conn_impl->send_key, key_meterial, HKDF_TYPE_IV);
	ret = kernel_setsockopt(tcp->sock, SOL_TCP, TCP_ULP, "tls",
				sizeof("tls"));
	if (ret)
		hmdfs_err("set tls error %d", ret);

	init_tls_info(&tcp->connect->send_crypto_info, key_meterial,
		      tcp->connect->send_key, HMDFS_KEY_SIZE);
	ret = kernel_setsockopt(tcp->sock, SOL_TLS, TLS_TX,
				(char *)&(tcp->connect->send_crypto_info),
				sizeof(tcp->connect->send_crypto_info));
	if (ret)
		hmdfs_err("set tls send_crypto_info error %d", ret);

	// recv
	update_key(tcp->connect->recv_key, key_meterial, HKDF_TYPE_IV);
	init_tls_info(&tcp->connect->recv_crypto_info, key_meterial,
		      tcp->connect->recv_key, HMDFS_KEY_SIZE);
	memzero_explicit(key_meterial, HMDFS_KEY_SIZE);

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

	memzero_explicit(new_key, HMDFS_KEY_SIZE);
	memzero_explicit(key_meterial, HMDFS_KEY_SIZE);

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

	memzero_explicit(new_key, HMDFS_KEY_SIZE);
	memzero_explicit(key_meterial, HMDFS_KEY_SIZE);

	tls_crypto_set_key(tcp->connect, 0);
	return 0;
}

static int set_crypto_info_tls(struct tcp_handle *tcp, int set_type)
{
	int ret = 0;

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

#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
static int d2dp_update_tx_key(struct d2dp_handle *d2dp)
{
	int ret = 0;
	u8 new_key[HMDFS_KEY_SIZE] = { 0 };
	struct connection *conn = d2dp->connect;

	ret = update_key(conn->send_key, new_key, HKDF_TYPE_D2DP_REKEY);
	if (ret < 0)
		goto out_memzero;

	/* TX lock can fail in D2DP */
	ret = d2d_crypto_tx_lock(d2dp->proto);
	if (ret) {
		hmdfs_err("D2DP error on lock");
		goto out_memzero;
	}
	ret = crypto_aead_setkey(d2dp->send_tfm, new_key, HMDFS_KEY_SIZE);
	d2d_crypto_tx_unlock(d2dp->proto);

	if (ret) {
		hmdfs_err("D2DP error on crypto aead");
		goto out_memzero;
	}

	memcpy(conn->send_key, new_key, HMDFS_KEY_SIZE);
out_memzero:
	memzero_explicit(new_key, HMDFS_KEY_SIZE);
	return ret;
}

static int d2dp_update_rx_key(struct d2dp_handle *d2dp)
{
	int ret = 0;
	u8 new_key[HMDFS_KEY_SIZE] = { 0 };
	struct connection *conn = d2dp->connect;

	ret = update_key(conn->recv_key, new_key, HKDF_TYPE_D2DP_REKEY);
	if (ret < 0)
		goto out_memzero;

	d2d_crypto_rx_lock(d2dp->proto);
	ret = crypto_aead_setkey(d2dp->recv_tfm, new_key, HMDFS_KEY_SIZE);
	d2d_crypto_rx_unlock(d2dp->proto);

	if (ret) {
		hmdfs_err("D2DP error on crypto aead");
		goto out_memzero;
	}

	memcpy(conn->recv_key, new_key, HMDFS_KEY_SIZE);
out_memzero:
	memzero_explicit(new_key, HMDFS_KEY_SIZE);
	return ret;
}

static int set_crypto_info_d2dp(struct d2dp_handle *d2dp, int set_type)
{
	int ret = 0;

	if (set_type == SET_CRYPTO_SEND) {
		ret = d2dp_update_tx_key(d2dp);
		if (ret) {
			hmdfs_err("D2DP update TX key failed: %d", ret);
			return ret;
		}
	}
	if (set_type == SET_CRYPTO_RECV) {
		ret = d2dp_update_rx_key(d2dp);
		if (ret) {
			hmdfs_err("D2DP update RX key failed: %d", ret);
			return ret;
		}
	}

	hmdfs_info("D2DP key updated successfully");
	return ret;
}
#endif /* CONFIG_HMDFS_D2DP_TRANSPORT */

int set_crypto_info(struct connection *conn, int set_type)
{
	__u8 version = conn->node->version;
	void *handle = conn->connect_handle;

	if (version < DFS_2_0 || !handle)
		return -EINVAL;

	switch (conn->type) {
	case CONNECT_TYPE_TCP:
		return set_crypto_info_tls(handle, set_type);
#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	case CONNECT_TYPE_D2DP:
		return set_crypto_info_d2dp(handle, set_type);
#endif
	default:
		hmdfs_err("wrong connection type: %d", conn->type);
		return -EINVAL;
	}
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

/*
 * NOTE: add new key labels with caution, keep the following rules:
 *
 * 1. Make sure that key label is not longer than (MAX_LABEL_SIZE - 3)
 * 2. Add the correct key label length to the `g_key_label_len` array
 * 3. Add the new element to the `HKDF_TYPE` enum to make label available
 */
static const char * const g_key_label[] = {
	[HKDF_TYPE_KEY_INITIATOR]  = "ktls key initiator",
	[HKDF_TYPE_KEY_ACCEPTER]   = "ktls key accepter",
	[HKDF_TYPE_REKEY]          = "ktls key update",
	[HKDF_TYPE_IV]             = "ktls iv&salt",
#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	[HKDF_TYPE_D2DP_INITIATOR] = "d2dp key initiator",
	[HKDF_TYPE_D2DP_ACCEPTER]  = "d2dp key accepter",
	[HKDF_TYPE_D2DP_REKEY]     = "d2dp key update",
#endif
};
static const int g_key_label_len[] = {
	[HKDF_TYPE_KEY_INITIATOR]  = 18,
	[HKDF_TYPE_KEY_ACCEPTER]   = 17,
	[HKDF_TYPE_REKEY]          = 15,
	[HKDF_TYPE_IV]             = 12,
#ifdef CONFIG_HMDFS_D2DP_TRANSPORT
	[HKDF_TYPE_D2DP_INITIATOR] = 18,
	[HKDF_TYPE_D2DP_ACCEPTER]  = 17,
	[HKDF_TYPE_D2DP_REKEY]     = 15,
#endif
};

/*
 * update scheme:
 *
 * 20|00|KK|EE|YY| ... |LL|AA|BB|EE|LL|01|??|??|?? (30 bytes total)
 * ----- ----------------------------- -- --------
 * 32    key label string              1  uninit
 */
int update_key(__u8 *old_key, __u8 *new_key, int type)
{
	int ret = 0;
	u16 key_size = HMDFS_KEY_SIZE;
	u8 label[MAX_LABEL_SIZE] = { 0 };
	u8 label_size = 0;

	if (type >= HKDF_TYPE_MAX || type < 0) {
		hmdfs_err("wrong type: %d", type);
		return -EINVAL;
	}

	/* calculate label size first, including label length and 0x01 byte */
	label_size = g_key_label_len[type] + sizeof(u16) + sizeof(u8);

	/* fill the label: key_size(2 bytes) + label string + 0x01 */
	memcpy(label, &key_size, sizeof(u16));
	memcpy(label + sizeof(u16), g_key_label[type], g_key_label_len[type]);
	label[sizeof(u16) + g_key_label_len[type]] = 0x01;

	ret = hmac_sha256(old_key, HMDFS_KEY_SIZE, label, label_size, new_key);
	if (ret < 0)
		hmdfs_err("hmac sha256 error: %d", ret);

	return ret;
}
