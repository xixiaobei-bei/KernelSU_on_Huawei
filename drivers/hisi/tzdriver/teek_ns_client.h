/*
 * teek_ns_client.h
 *
 * define structures and IOCTLs.
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef TEEK_NS_CLIENT_H
#define TEEK_NS_CLIENT_H

#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/completion.h>
#include <securec.h>
#include "tc_ns_client.h"
#include "tc_ns_log.h"

#define TC_NS_CLIENT_IOC_MAGIC  't'
#define TC_NS_CLIENT_DEV        "tc_ns_client"
#define TC_NS_CLIENT_DEV_NAME   "/dev/tc_ns_client"

#define EXCEPTION_MEM_SIZE (8*1024) /* mem for exception handling */
#define TSP_REQUEST        0xB2000008
#define TSP_RESPONSE       0xB2000009
#define TSP_REE_SIQ        0xB200000A
#define TSP_CRASH          0xB200000B
#define TSP_PREEMPTED      0xB2000005
#define TC_CALL_GLOBAL     0x01
#define TC_CALL_SYNC       0x02
#define TC_CALL_LOGIN            0x04
#define TEE_REQ_FROM_USER_MODE   0U
#define TEE_REQ_FROM_KERNEL_MODE 1U
#define TEE_PARAM_NUM            4

/* Max sizes for login info buffer comming from teecd */
#define MAX_PACKAGE_NAME_LEN 255
/* The apk certificate format is as follows:
  * modulus_size(4 bytes) + modulus buffer(512 bytes)
  * + exponent size(4 bytes) + exponent buffer(1 bytes)
  */
#define MAX_PUBKEY_LEN 1024

struct tc_ns_dev_list {
	struct mutex dev_lock; /* for dev_file_list */
	struct list_head dev_file_list;
};

struct tc_uuid {
	uint32_t time_low;
	uint16_t time_mid;
	uint16_t timehi_and_version;
	uint8_t clockseq_and_node[8]; /* clock len is 8 */
};

struct tc_ns_shared_mem {
	void *kernel_addr;
	void *user_addr;
	void *user_addr_ca; /* for ca alloc share mem */
	unsigned int len;
	struct list_head head;
	atomic_t usage;
	atomic_t offset;
};

struct tc_ns_service {
	unsigned char uuid[UUID_LEN];
	struct mutex session_lock; /* for session_list */
	struct list_head session_list;
	struct list_head head;
	struct mutex operation_lock; /* for session's open/close */
	atomic_t usage;
};

#define SERVICES_MAX_COUNT 32 /* service limit can opened on 1 fd */
struct tc_ns_dev_file {
	unsigned int dev_file_id;
	struct mutex service_lock; /* for service_ref[], services[] */
	uint8_t service_ref[SERVICES_MAX_COUNT]; /* a judge if set services[i]=NULL */
	struct tc_ns_service *services[SERVICES_MAX_COUNT];
	struct mutex shared_mem_lock; /* for shared_mem_list */
	struct list_head shared_mem_list;
	struct list_head head;
	/* Device is linked to call from kernel */
	uint8_t kernel_api;
	/* client login info provided by teecd, can be either package name and public
	 * key or uid(for non android services/daemons)
	 * login information can only be set once, dont' allow subsequent calls
	 */
	bool login_setup;
	struct mutex login_setup_lock; /* for login_setup */
	uint32_t pkg_name_len;
	uint8_t pkg_name[MAX_PACKAGE_NAME_LEN];
	uint32_t pub_key_len;
	uint8_t pub_key[MAX_PUBKEY_LEN];
	int load_app_flag;
	struct completion close_comp; /* for kthread close unclosed session */
};

union tc_ns_parameter {
	struct {
		unsigned int buffer;
		unsigned int size;
	} memref;
	struct {
		unsigned int a;
		unsigned int b;
	} value;
};

struct tc_ns_login {
	unsigned int method;
	unsigned int mdata;
};

struct tc_ns_operation {
	unsigned int paramtypes;
	union tc_ns_parameter params[TEE_PARAM_NUM];
	unsigned int    buffer_h_addr[TEE_PARAM_NUM];
	struct tc_ns_shared_mem *sharemem[TEE_PARAM_NUM];
	void *mb_buffer[TEE_PARAM_NUM];
};

struct tc_ns_temp_buf {
	void *temp_buffer;
	unsigned int size;
};

struct tc_ns_smc_cmd {
	uint8_t      uuid[sizeof(struct tc_uuid)];
	bool         global_cmd; /* mark it's a gtask cmd */
	unsigned int cmd_id;
	unsigned int dev_file_id;
	unsigned int context_id;
	unsigned int agent_id;
	unsigned int operation_phys;
	unsigned int operation_h_phys;
	unsigned int login_method;
	unsigned int login_data_phy;
	unsigned int login_data_h_addr;
	unsigned int login_data_len;
	unsigned int err_origin;
	int          ret_val;
	unsigned int event_nr;
	unsigned int uid;
	unsigned int ca_pid;
#ifdef CONFIG_AUTH_ENHANCE
	unsigned int token_phys;
	unsigned int token_h_phys;
	unsigned int pid;
	unsigned int params_phys;
	unsigned int params_h_phys;
	unsigned int eventindex;     /* tee audit event index for upload */
#endif
	bool started;
} __attribute__((__packed__));

/*
 * @brief
 */
struct tc_wait_data {
	wait_queue_head_t send_cmd_wq;
	int send_wait_flag;
};

#ifdef CONFIG_AUTH_ENHANCE

#define TOKEN_SAVE_LEN	        24
/* token(32byte) + timestamp(8byte) + kernal_api(1byte) + sync(1byte) */
#define TOKEN_BUFFER_LEN        42

/* Using AES-CBC algorithm to encrypt communication between secure world and
 * normal world.
 */
#define CIPHER_KEY_BYTESIZE   32   /* AES-256 key size */
#define IV_BYTESIZE           16  /* AES-CBC encryption initialization vector size */
#define CIPHER_BLOCK_BYTESIZE 16 /* AES-CBC cipher block size */
#define SCRAMBLING_NUMBER     3
#define MAGIC_SIZE            16

/* One encrypted block, which is aligned with CIPHER_BLOCK_BYTESIZE bytes
 * Head + Payload + Padding
 */
struct encryption_head {
	int8_t magic[MAGIC_SIZE];
	uint32_t payload_len;
};

#define HASH_PLAINTEXT_SIZE (MAX_SHA_256_SZ + sizeof(struct encryption_head))
#define HASH_PLAINTEXT_ALIGNED_SIZE \
	ALIGN(HASH_PLAINTEXT_SIZE, CIPHER_BLOCK_BYTESIZE)

struct session_crypto_info {
	uint8_t key[CIPHER_KEY_BYTESIZE]; /* AES-256 key */
	uint8_t iv[IV_BYTESIZE]; /* AES-CBC encryption initialization vector */
};

struct session_secure_info {
	uint32_t challenge_word;
	uint32_t scrambling[SCRAMBLING_NUMBER];
	struct session_crypto_info crypto_info;
};

struct tc_ns_token {
	/* 42byte, token_32byte + timestamp_8byte + kernal_api_1byte + sync_1byte */
	uint8_t *token_buffer;
	uint32_t token_len;
};

struct session_secure_params {
	struct encryption_head head;
	union {
		struct {
			uint32_t challenge_word;
		} ree2tee;
		struct {
			uint32_t scrambling[SCRAMBLING_NUMBER];
			struct session_crypto_info crypto_info;
		} tee2ree;
	} payload;
};
#endif

#define NUM_OF_SO 1
#ifdef CONFIG_CMS_CAHASH_AUTH
#define KIND_OF_SO 1
#else
#define KIND_OF_SO 2
#endif
struct tc_ns_session {
	unsigned int session_id;
	struct list_head head;
	struct tc_wait_data wait_data;
	struct mutex ta_session_lock; /* for open/close/invoke on 1 session */
	struct tc_ns_dev_file *owner;
#ifdef CONFIG_AUTH_ENHANCE
	/* Session secure enhanced information */
	struct session_secure_info secure_info;
	struct tc_ns_token teec_token;
	/* when CONFIG_AUTH_ENHANCE enabled, hash of the same CA and
	 * SO library will be encrypted by different session key,
	 * so put auth_hash_buf in struct tc_ns_session.
	 * the first MAX_SHA_256_SZ size stores SO hash,
	 * the next HASH_PLAINTEXT_ALIGNED_SIZE stores CA hash and cryptohead,
	 * the last IV_BYTESIZE size stores aes iv
	 */
	uint8_t auth_hash_buf[MAX_SHA_256_SZ * NUM_OF_SO + HASH_PLAINTEXT_ALIGNED_SIZE + IV_BYTESIZE];
#else
	uint8_t auth_hash_buf[MAX_SHA_256_SZ * NUM_OF_SO + MAX_SHA_256_SZ];
#endif
	atomic_t usage;
};

struct mb_cmd_pack {
	struct tc_ns_operation operation;
#ifdef CONFIG_AUTH_ENHANCE
	unsigned char login_data[MAX_SHA_256_SZ * NUM_OF_SO + HASH_PLAINTEXT_ALIGNED_SIZE + IV_BYTESIZE];
	unsigned char token[TOKEN_BUFFER_LEN];
	unsigned char secure_params[ALIGN(sizeof(struct session_secure_params),
		CIPHER_BLOCK_BYTESIZE) + IV_BYTESIZE];
#else
	unsigned char login_data[MAX_SHA_256_SZ * NUM_OF_SO + MAX_SHA_256_SZ];
#endif
};

#endif
