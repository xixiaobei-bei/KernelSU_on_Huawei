

#ifndef _TEEK_NS_CLIENT_H_
#define _TEEK_NS_CLIENT_H_

#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <securec.h>
#include "tc_ns_client.h"
#include "tc_ns_log.h"

#define TC_NS_CLIENT_IOC_MAGIC  't'
#define TC_NS_CLIENT_DEV        "tc_ns_client"
#define TC_NS_CLIENT_DEV_NAME   "/dev/tc_ns_client"

#ifdef CONFIG_SECURE_EXTENSION
#define TC_ASYNC_NOTIFY_SUPPORT
#endif

#define EXCEPTION_MEM_SIZE (8*1024) /* mem for exception handling */
#define TSP_REQUEST        0xB2000008
#define TSP_RESPONSE       0xB2000009
#define TSP_REE_SIQ        0xB200000A
#define TSP_CRASH          0xB200000B
#define TSP_PREEMPTED      0xB2000005
#define TC_CALL_GLOBAL     0x01
#define TC_CALL_SYNC       0x02
#define TC_CALL_LOGIN            0x04
#define TEE_REQ_FROM_USER_MODE   0x0
#define TEE_REQ_FROM_KERNEL_MODE 0x1

/* Max sizes for login info buffer comming from teecd */
#define MAX_PACKAGE_NAME_LEN 255
/* The apk certificate format is as follows:
  * modulus_size(4 bytes) + modulus buffer(512 bytes)
  * + exponent size(4 bytes) + exponent buffer(1 bytes)
  */
#define MAX_PUBKEY_LEN 1024

struct tag_tc_ns_shared_mem;
struct tag_tc_ns_service;

struct tc_ns_dev_list {
	struct mutex dev_lock; /* for dev_file_list */
	struct list_head dev_file_list;
};

extern struct tc_ns_dev_list g_tc_ns_dev_list;
extern struct mutex g_service_list_lock;

#define SERVICES_MAX_COUNT 32 /* service limit can opened on 1 fd */
typedef struct tag_tc_ns_dev_file {
	unsigned int dev_file_id;
	struct mutex service_lock; /* for service_ref[], services[] */
	uint8_t service_ref[SERVICES_MAX_COUNT]; /* a judge if set services[i]=NULL */
	struct tag_tc_ns_service *services[SERVICES_MAX_COUNT];
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
} tc_ns_dev_file;

typedef union {
	struct {
		unsigned int buffer;
		unsigned int size;
	} memref;
	struct {
		unsigned int a;
		unsigned int b;
	} value;
} tc_ns_parameter;

typedef struct tag_tc_ns_login {
	unsigned int method;
	unsigned int mdata;
} tc_ns_login;

typedef struct tag_tc_ns_operation {
	unsigned int paramtypes;
	tc_ns_parameter params[TEE_PARAM_NUM];
	unsigned int    buffer_h_addr[TEE_PARAM_NUM];
	struct tag_tc_ns_shared_mem *sharemem[TEE_PARAM_NUM];
	void *mb_buffer[TEE_PARAM_NUM];
} tc_ns_operation;

typedef struct tag_tc_ns_temp_buf {
	void *temp_buffer;
	unsigned int size;
} tc_ns_temp_buf;

typedef struct  tag_tc_ns_smc_cmd {
	uint8_t      uuid[sizeof(teec_uuid)];
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
#ifdef CONFIG_TEE_SMP
	unsigned int ca_pid;
#endif
#ifdef SECURITY_AUTH_ENHANCE
	unsigned int token_phys;
	unsigned int token_h_phys;
	unsigned int pid;
	unsigned int params_phys;
	unsigned int params_h_phys;
	unsigned int eventindex;     // tee audit event index for upload
#endif
	bool started;
}__attribute__((__packed__))tc_ns_smc_cmd;

typedef struct tag_tc_ns_shared_mem {
	void *kernel_addr;
	void *user_addr;
	void *user_addr_ca; /* for ca alloc share mem */
	unsigned int len;
	bool from_mailbox;
	struct list_head head;
	atomic_t usage;
	atomic_t offset;
} tc_ns_shared_mem;

typedef struct tag_tc_ns_service {
	unsigned char uuid[UUID_LEN];
	struct mutex session_lock; /* for session_list */
	struct list_head session_list;
	struct list_head head;
	struct mutex operation_lock; /* for session's open/close */
	atomic_t usage;
} tc_ns_service;

/*
 * @brief
 */
struct tc_wait_data {
	wait_queue_head_t send_cmd_wq;
	int send_wait_flag;
};

#ifdef SECURITY_AUTH_ENHANCE
/* Using AES-CBC algorithm to encrypt communication between secure world and
 * normal world.
 */
#define CIPHER_KEY_BYTESIZE 32   /* AES-256 key size */
#define IV_BYTESIZE   16  /* AES-CBC encryption initialization vector size */
#define CIPHER_BLOCK_BYTESIZE 16 /* AES-CBC cipher block size */
#define SCRAMBLING_NUMBER 3
#define CHKSUM_LENGTH  (sizeof(tc_ns_smc_cmd) - sizeof(uint32_t))

#define HASH_PLAINTEXT_SIZE (MAX_SHA_256_SZ + sizeof(struct encryption_head))
#define HASH_PLAINTEXT_ALIGNED_SIZE \
	ALIGN(HASH_PLAINTEXT_SIZE, CIPHER_BLOCK_BYTESIZE)

enum SCRAMBLING_ID {
	SCRAMBLING_TOKEN = 0,
	SCRAMBLING_OPERATION = 1,
	SCRAMBLING_MAX = SCRAMBLING_NUMBER
};

struct session_crypto_info {
	uint8_t key[CIPHER_KEY_BYTESIZE]; /* AES-256 key */
	uint8_t iv[IV_BYTESIZE]; /* AES-CBC encryption initialization vector */
};

struct session_secure_info {
	uint32_t challenge_word;
	uint32_t scrambling[SCRAMBLING_NUMBER];
	struct session_crypto_info crypto_info;
};

#define MAGIC_SIZE 16
#define MAGIC_STRING "Trusted-magic"

/* One encrypted block, which is aligned with CIPHER_BLOCK_BYTESIZE bytes
 * Head + Payload + Padding
 */
struct encryption_head {
	int8_t magic[MAGIC_SIZE];
	uint32_t payload_len;
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

#ifdef SECURITY_AUTH_ENHANCE
typedef struct tag_tc_ns_token {
	/* 42byte, token_32byte + timestamp_8byte + kernal_api_1byte + sync_1byte */
	uint8_t *token_buffer;
	uint32_t token_len;
} tc_ns_token;
#endif

#define NUM_OF_SO 1
#define KIND_OF_SO 2
typedef struct tag_tc_ns_session {
	unsigned int session_id;
	struct list_head head;
	struct tc_wait_data wait_data;
	struct mutex ta_session_lock; /* for open/close/invoke on 1 session */
	tc_ns_dev_file *owner;
#ifdef SECURITY_AUTH_ENHANCE
	/* Session secure enhanced information */
	struct session_secure_info secure_info;
	tc_ns_token tc_ns_token;
	/* when SECURITY_AUTH_ENHANCE enabled, hash of the same CA and
	 * SO library will be encrypted by different session key,
	 * so put auth_hash_buf in tc_ns_session.
	 * the first MAX_SHA_256_SZ size stores SO hash,
	 * the next HASH_PLAINTEXT_ALIGNED_SIZE stores CA hash and cryptohead,
	 * the last IV_BYTESIZE size stores aes iv
	 */
	uint8_t auth_hash_buf[MAX_SHA_256_SZ * NUM_OF_SO + HASH_PLAINTEXT_ALIGNED_SIZE + IV_BYTESIZE];
#else
	uint8_t auth_hash_buf[MAX_SHA_256_SZ * NUM_OF_SO + MAX_SHA_256_SZ];
#endif
	atomic_t usage;
} tc_ns_session;

static inline void get_service_struct(struct tag_tc_ns_service *service)
{
	if (service != NULL) {
		atomic_inc(&service->usage);
		tlogd("service->usage = %d\n", atomic_read(&service->usage));
	}
}

static inline void put_service_struct(struct tag_tc_ns_service *service)
{
	if (service != NULL) {
		tlogd("service->usage = %d\n", atomic_read(&service->usage));
		mutex_lock(&g_service_list_lock);
		if (atomic_dec_and_test(&service->usage)) {
			tlogd("del service [0x%x] from service list\n",
				*(uint32_t *)service->uuid);
			list_del(&service->head);
			kfree(service);
		}
		mutex_unlock(&g_service_list_lock);
	}
}

static inline void get_session_struct(struct tag_tc_ns_session *session)
{
	if (session != NULL) {
		atomic_inc(&session->usage);
	}
}

static inline void put_session_struct(struct tag_tc_ns_session *session)
{
	if (session != NULL) {
		if (atomic_dec_and_test(&session->usage)) {
#ifdef SECURITY_AUTH_ENHANCE
			if (session->tc_ns_token.token_buffer != NULL) {
				if (memset_s(
					(void *)session->tc_ns_token.token_buffer,
					session->tc_ns_token.token_len,
					0,
					session->tc_ns_token.token_len) != EOK)
					tloge("Caution, memset failed!\n");
				kfree(session->tc_ns_token.token_buffer);
				session->tc_ns_token.token_buffer = NULL;
				(void)session->tc_ns_token.token_buffer; /* avoid Codex warning */
			}
#endif
			if (memset_s((void *)session, sizeof(*session), 0, sizeof(*session)) != EOK)
				tloge("Caution, memset failed!\n");
			kfree(session);
		}
	}
}

tc_ns_session *tc_find_session_withowner(struct list_head *session_list,
	unsigned int session_id, tc_ns_dev_file *dev);

#ifdef SECURITY_AUTH_ENHANCE
int generate_encrypted_session_secure_params(
	struct session_secure_info *secure_info,
	uint8_t *enc_secure_params, size_t enc_params_size);
#define ENCRYPT 1
#define DECRYPT 0

int crypto_session_aescbc_key256(uint8_t *in, uint32_t in_len,
	uint8_t *out, uint32_t out_len,
	const uint8_t *key, uint8_t *iv,
	uint32_t mode);
int crypto_aescbc_cms_padding(uint8_t *plaintext, uint32_t plaintext_len,
	uint32_t payload_len);
#endif

int tc_ns_client_open(tc_ns_dev_file **dev_file, uint8_t kernel_api);
int tc_ns_client_close(tc_ns_dev_file *dev, int flag);
int is_agent_alive(unsigned int agent_id);
int tc_ns_open_session(tc_ns_dev_file *dev_file, tc_ns_client_context *context);
int tc_ns_close_session(tc_ns_dev_file *dev_file, tc_ns_client_context *context);
int tc_ns_send_cmd(tc_ns_dev_file *dev_file, tc_ns_client_context *context);
uint32_t tc_ns_get_uid(void);

#endif
