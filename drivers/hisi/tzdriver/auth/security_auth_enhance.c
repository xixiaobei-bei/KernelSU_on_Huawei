/*
 * security_auth_enhance.c
 *
 * function for token decry, update, verify and so on.
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
#include "security_auth_enhance.h"
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/crypto.h>
#include <linux/scatterlist.h>
#include <linux/version.h>
#include <linux/err.h>
#include <linux/random.h>
#include <linux/crc32.h>
#include <crypto/skcipher.h>
#include <securec.h>
#include "teek_client_constants.h"
#include "tc_ns_log.h"
#include "securectype.h"
#include "tc_client_driver.h"
#include "mailbox_mempool.h"
#include "smc_smp.h"
#include "session_manager.h"

#if !defined(UINT64_MAX)
#define UINT64_MAX ((uint64_t)0xFFFFFFFFFFFFFFFFULL)
#endif

#define GLOBAL_CMD_ID_SSA               0x2DCB /* SSA cmd_id 11723 */
#define GLOBAL_CMD_ID_MT                0x2DCC /* MT cmd_id 11724 */
#define GLOBAL_CMD_ID_MT_UPDATE         0x2DCD /* MT_IPDATE cmd_id 11725 */
#define TEEC_PENDING2_AGENT             0xFFFF2001
#define INVALID_TZMP_UID                0xffffffff
#define ALIGN_BIT                       0x3

#define SCRAMBLING_KEY_LEN      4
#define TIMESTAMP_BUFFER_INDEX  32
#define KERNAL_API_INDEX        40
#define SYNC_INDEX              41
#define ENCRYPT                 1
#define DECRYPT                 0

#define TIMESTAMP_LEN_DEFAULT \
	((KERNAL_API_INDEX) - (TIMESTAMP_BUFFER_INDEX))
#define KERNAL_API_LEN \
	((TOKEN_BUFFER_LEN) - (KERNAL_API_INDEX))
#define TIMESTAMP_SAVE_INDEX    16

static DEFINE_MUTEX(g_tzmp_lock);
static unsigned int g_tzmp_uid = INVALID_TZMP_UID;

#define AES_LOGIN_MAXLEN   ((MAX_PUBKEY_LEN) > (MAX_PACKAGE_NAME_LEN) ? \
	(MAX_PUBKEY_LEN) : (MAX_PACKAGE_NAME_LEN))

#define TEE_TZMP \
{ \
	0xf8028dca, \
	0xaba0, \
	0x11e6, \
	{ \
		0x80, 0xf5, 0x76, 0x30, 0x4d, 0xec, 0x7e, 0xb7 \
	} \
}

struct session_crypto_info *g_session_root_key;

#define align_up(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

enum SCRAMBLING_ID {
	SCRAMBLING_TOKEN = 0,
	SCRAMBLING_OPERATION = 1,
	SCRAMBLING_MAX = SCRAMBLING_NUMBER
};

#define MAGIC_STRING "Trusted-magic"

struct get_secure_info_params {
	struct tc_ns_dev_file *dev_file;
	struct tc_ns_client_context *context;
	struct tc_ns_session *session;
};

static bool is_token_empty(const uint8_t *token, uint32_t token_len)
{
	uint32_t i;

	if (!token) {
		tloge("bad parameters, token is null\n");
		return true;
	}
	for (i = 0; i < token_len; i++) {
		if (*(token + i))
			return false;
	}
	return true;
}

static int32_t scrambling_timestamp(const void *in, void *out,
	uint32_t data_len, const void *key, uint32_t key_len)
{
	uint32_t i;
	bool check_value = false;

	if (!in || !out || !key) {
		tloge("bad parameters, input_data is null\n");
		return -EFAULT;
	}
	check_value = (!data_len || data_len > SECUREC_MEM_MAX_LEN ||
			key_len > SECUREC_MEM_MAX_LEN || !key_len);
	if (check_value) {
		tloge("bad parameters, data_len is %u, scrambling_len is %u\n",
		      data_len, key_len);
		return -EFAULT;
	}
	for (i = 0; i < data_len; i++)
		*((uint8_t *)out + i) =
			*((uint8_t *)in + i) ^ *((uint8_t *)key + i % key_len);

	return EOK;
}

static int32_t change_time_stamp(uint8_t flag, uint64_t *time_stamp)
{
	if (flag == INC) {
		if (*time_stamp < UINT64_MAX) {
			(*time_stamp)++;
		} else {
			tloge("val overflow\n");
			return -EFAULT;
		}
	} else if (flag == DEC) {
		if (*time_stamp > 0) {
			(*time_stamp)--;
		} else {
			tloge("val down overflow\n");
			return -EFAULT;
		}
	} else {
		tloge("flag error, 0x%x\n", flag);
		return -EFAULT;
	}
	return EOK;
}

static int32_t descrambling_timestamp(uint8_t *in_token_buf,
	const struct session_secure_info *secure_info, uint8_t flag)
{
	uint64_t time_stamp = 0;
	int32_t ret;

	if (in_token_buf == NULL || secure_info == NULL) {
		tloge("invalid params!\n");
		return -EINVAL;
	}
	if (scrambling_timestamp(&in_token_buf[TIMESTAMP_BUFFER_INDEX],
				 &time_stamp, TIMESTAMP_LEN_DEFAULT,
				 secure_info->scrambling, SCRAMBLING_KEY_LEN)) {
		tloge("descrambling_timestamp failed\n");
		return -EFAULT;
	}
	ret = change_time_stamp(flag, &time_stamp);
	if (ret) {
		return ret;
	}

	tlogd("timestamp is %llu\n", time_stamp);
	if (scrambling_timestamp(&time_stamp,
				 &in_token_buf[TIMESTAMP_BUFFER_INDEX],
				 TIMESTAMP_LEN_DEFAULT,
				 secure_info->scrambling,
				 SCRAMBLING_KEY_LEN)) {
		tloge("descrambling_timestamp failed\n");
		return -EFAULT;
	}
	return EOK;
}

int32_t update_timestamp(const struct tc_ns_smc_cmd *cmd)
{
	struct tc_ns_session *session = NULL;
	struct session_secure_info *secure_info = NULL;
	uint8_t *token_buffer = NULL;
	bool filter_flag = false;
	bool need_check_flag = false;

	if (!cmd) {
		tloge("cmd is NULL, error\n");
		return TEEC_ERROR_BAD_PARAMETERS;
	}
	/* if cmd is agent, not check uuid. and sometime uuid canot access it */
	filter_flag = (cmd->agent_id) ||
			(cmd->ret_val == TEEC_PENDING2_AGENT);
	if (filter_flag)
		return TEEC_SUCCESS;

	need_check_flag = (!cmd->global_cmd) && (!cmd->agent_id) &&
		(cmd->ret_val != TEEC_PENDING2_AGENT);
	if (need_check_flag) {
		token_buffer = phys_to_virt((phys_addr_t)(cmd->token_h_phys) <<
					    ADDR_TRANS_NUM | (cmd->token_phys));
		if (!token_buffer ||
			is_token_empty(token_buffer, TOKEN_BUFFER_LEN)) {
			tloge("token is NULL or token is empyt, error\n");
			return -EFAULT;
		}

		session = tc_find_session_by_uuid(cmd->dev_file_id, cmd);
		if (!session) {
			tlogd("tc_find_session_key find session FAILURE\n");
			return -EFAULT;
		}
		secure_info = &session->secure_info;
		if (descrambling_timestamp(token_buffer,
						secure_info, INC)) {
			put_session_struct(session);
			tloge("update token_buffer error\n");
			return -EFAULT;
		}
		put_session_struct(session);
		token_buffer[SYNC_INDEX] = UN_SYNCED;
	} else {
		tlogd("global cmd or agent, do not update timestamp\n");
	}
	return EOK;
}

int32_t sync_timestamp(const struct tc_ns_smc_cmd *cmd, uint8_t *token,
	uint32_t token_len, bool is_global)
{
	struct tc_ns_session *session = NULL;
	bool check_val = false;

	check_val = (!cmd || !token || token_len <= SYNC_INDEX);
	if (check_val) {
		tloge("parameters is NULL, error\n");
		return -EFAULT;
	}
	if (cmd->cmd_id == GLOBAL_CMD_ID_OPEN_SESSION && is_global) {
		tlogd("OpenSession would not need sync timestamp\n");
		return EOK;
	}
	if (token[SYNC_INDEX] == UN_SYNCED) {
		tlogd("flag is UN_SYNC, to sync timestamp!\n");

		session = tc_find_session_by_uuid(cmd->dev_file_id, cmd);
		if (!session) {
			tloge("sync_timestamp find session FAILURE\n");
			return -EFAULT;
		}
		if (descrambling_timestamp(token,
			&session->secure_info, DEC)) {
			put_session_struct(session);
			tloge("sync token_buffer error\n");
			return -EFAULT;
		}
		put_session_struct(session);
	} else if (token[SYNC_INDEX] == IS_SYNCED) {
		tlogd("token is synced\n");
	} else {
		tloge("sync flag error! 0x%x\n", token[SYNC_INDEX]);
		return -EFAULT;
	}
	return EOK;
}

/* scrambling operation and pid */
static void scrambling_operation(struct tc_ns_smc_cmd *cmd, uint32_t scrambler)
{
	if (cmd == NULL)
		return;
	if (cmd->operation_phys || cmd->operation_h_phys) {
		cmd->operation_phys = cmd->operation_phys ^ scrambler;
		cmd->operation_h_phys = cmd->operation_h_phys ^ scrambler;
	}
	cmd->pid = cmd->pid ^ scrambler;
}

static bool agent_msg(uint32_t cmd_id)
{
	bool agent = (cmd_id == GLOBAL_CMD_ID_SSA) ||
		 (cmd_id == GLOBAL_CMD_ID_MT) ||
		 (cmd_id == GLOBAL_CMD_ID_MT_UPDATE);

	return agent;
}

/* calculate cmd checksum and scrambling operation */
int32_t update_chksum(struct tc_ns_smc_cmd *cmd)
{
	struct tc_ns_session *session = NULL;
	struct session_secure_info *secure_info = NULL;
	uint32_t scrambler_oper;
	bool check_value = false;

	if (!cmd) {
		tloge("cmd is NULL, error\n");
		return -EFAULT;
	}
	/*
	 * if cmd is agent, do not check uuid.
	 * and sometimes uuid cannot access it
	 */
	check_value = (cmd->agent_id ||
				cmd->ret_val == TEEC_PENDING2_AGENT);
	if (check_value)
		return EOK;

	if (agent_msg(cmd->cmd_id)) {
		tlogd("SSA cmd, no need to update_chksum\n");
		return EOK;
	}
	/* cmd is invoke command */
	check_value = (!cmd->global_cmd) && (!cmd->agent_id) &&
		(cmd->ret_val != TEEC_PENDING2_AGENT);

	if (check_value) {
		session = tc_find_session_by_uuid(cmd->dev_file_id, cmd);
		if (session) {
			secure_info = &session->secure_info;
			scrambler_oper =
				secure_info->scrambling[SCRAMBLING_OPERATION];
			scrambling_operation(cmd, scrambler_oper);
			put_session_struct(session);
		}
	}
	return EOK;
}

int32_t verify_chksum(const struct tc_ns_smc_cmd *cmd)
{
	struct tc_ns_session *session = NULL;
	struct session_secure_info *secure_info = NULL;
	bool check_flag = false;

	if (!cmd) {
		tloge("cmd is NULL, error\n");
		return -EFAULT;
	}
	if (agent_msg(cmd->cmd_id)) {
		tlogd("SSA cmd, no need to update_chksum\n");
		return EOK;
	}

	/* cmd is invoke command */
	check_flag = cmd->global_cmd == false &&
			cmd->cmd_id != GLOBAL_CMD_ID_CLOSE_SESSION &&
			cmd->cmd_id != GLOBAL_CMD_ID_KILL_TASK &&
			cmd->agent_id == 0;
	if (check_flag) {
		session = tc_find_session_by_uuid(cmd->dev_file_id, cmd);
		if (session) {
			secure_info = &session->secure_info;
			put_session_struct(session);
		}
	}
	return EOK;
}

static int check_random_data(const uint8_t *data, uint32_t size)
{
	uint32_t i;

	for (i = 0; i < size; i++) {
		if (data[i])
			break;
	}
	if (i >= size)
		return -1;
	return 0;
}

static int generate_random_data(uint8_t *data, uint32_t size)
{
	if (!data || !size) {
		tloge("Bad parameters!\n");
		return -EFAULT;
	}
	if (memset_s((void *)data, size, 0, size)) {
		tloge("Clean the data buffer failed\n");
		return -EFAULT;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
	get_random_bytes_arch((void *)data, size);
#else
	if (get_random_bytes_arch((void *)data, size))
		tloge("get random byte failed\n");
#endif
	if (check_random_data(data, size))
		tlogd("random arch generate failed\n");
	else
		return 0;

	/* using soft random generator */
	get_random_bytes((void *)data, size);
	if (check_random_data(data, size))
		return -EFAULT;

	return 0;
}

static int generate_challenge_word(uint8_t *challenge_word, uint32_t size)
{
	if (!challenge_word) {
		tloge("Parameter is null pointer\n");
		return -EINVAL;
	}
	return generate_random_data(challenge_word, size);
}

bool is_opensession_by_index(uint8_t flags, uint32_t cmd_id, int index)
{
	/* params[2] for apk cert or native ca uid;
	 * params[3] for pkg name; therefore we set i>= 2
	 */
	bool is_global = flags & TC_CALL_GLOBAL;
	bool login_en = (is_global && (index >= 2) &&
		(cmd_id == GLOBAL_CMD_ID_OPEN_SESSION));
	return login_en;
}

static bool is_valid_size(uint32_t buffer_size, uint32_t temp_size)
{
	bool over_flow = false;

	if (buffer_size > AES_LOGIN_MAXLEN) {
		tloge("CONFIG_AUTH_ENHANCE: buffer_size is not right\n");
		return false;
	}
	over_flow = (temp_size > round_up(buffer_size, SZ_4K)) ? true : false;
	if (over_flow) {
		tloge("CONFIG_AUTH_ENHANCE: input data exceeds limit\n");
		return false;
	}
	return true;
}
static int check_param_for_encryption(uint8_t *buffer,
	uint32_t buffer_size, uint8_t **plaintext,
	uint32_t *plaintext_buffer_size, const uint8_t *key)
{
	if (!buffer || !buffer_size || !key) {
		tloge("bad params before encryption\n");
		return -EINVAL;
	}

	*plaintext_buffer_size = buffer_size;
	*plaintext = kzalloc(*plaintext_buffer_size, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)(*plaintext))) {
		tloge("malloc plaintext failed\n");
		return -ENOMEM;
	}
	if (memcpy_s(*plaintext, *plaintext_buffer_size,
		buffer, buffer_size)) {
		tloge("memcpy failed\n");
		kfree(*plaintext);
		return -EINVAL;
	}
	return 0;
}

static int handle_end(const uint8_t *plaintext, const uint8_t *cryptotext, int ret)
{
	kfree(plaintext);
	if (cryptotext)
		kfree(cryptotext);
	return ret;
}

static int calc_plaintext_size(uint32_t *plaintext_size,
	uint32_t payload_size, uint32_t buffer_size,
	uint32_t *plaintext_aligned_size, uint32_t *total_size)
{
	int ret = 0;

	/* Payload + Head + Padding */
	*plaintext_size = payload_size + sizeof(struct encryption_head);
	*plaintext_aligned_size =
		round_up(*plaintext_size, CIPHER_BLOCK_BYTESIZE);
	/* Need 16 bytes to store AES-CBC iv */
	*total_size = *plaintext_aligned_size + IV_BYTESIZE;
	if (*total_size > buffer_size) {
		tloge("Do encryption buffer is not enough\n");
		ret = -ENOMEM;
	}
	return ret;
}

static int set_encryption_head(struct encryption_head *head,
	uint32_t len)
{
	if (!head || !len) {
		tloge("In parameters check failed\n");
		return -EINVAL;
	}
	if (strncpy_s(head->magic, sizeof(head->magic),
		MAGIC_STRING, strlen(MAGIC_STRING) + 1)) {
		tloge("Copy magic string failed\n");
		return -EFAULT;
	}
	head->payload_len = len;
	return 0;
}

static int crypto_aescbc_cms_padding(uint8_t *plaintext, uint32_t plaintext_len,
	uint32_t payload_len)
{
	uint32_t padding_len;
	uint8_t padding;
	bool check_value = false;

	if (!plaintext) {
		tloge("Plaintext is NULL\n");
		return -EINVAL;
	}

	check_value = (!plaintext_len) ||
		(plaintext_len % CIPHER_BLOCK_BYTESIZE) ||
		(plaintext_len < payload_len);
	if (check_value) {
		tloge("Plaintext length is invalid\n");
		return -EINVAL;
	}

	padding_len = plaintext_len - payload_len;
	if (padding_len >= CIPHER_BLOCK_BYTESIZE) {
		tloge("Padding length is error\n");
		return -EINVAL;
	}

	/* No need padding */
	if (!padding_len)
		return 0;

	padding = (uint8_t)padding_len;
	if (memset_s((void *)(plaintext + payload_len),
		padding_len, padding, padding_len)) {
		tloge("CMS-Padding is failed\n");
		return -EFAULT;
	}
	return 0;
}

/* size of [iv] is 16 and [key] must be 32 bytes.
 * [size] is the size of [output] and [input].
 * [size] must be multiple of 32.
 */
static int crypto_aescbc_key256(uint8_t *output, const uint8_t *input,
	const uint8_t *iv, const uint8_t *key, int32_t size, uint32_t encrypto_type)
{
	struct scatterlist src = {0};
	struct scatterlist dst = {0};
	struct crypto_skcipher *skcipher = NULL;
	struct skcipher_request *req = NULL;
	int ret;
	uint8_t temp_iv[IV_BYTESIZE] = {0};

	skcipher = crypto_alloc_skcipher("cbc(aes)", 0, 0);
	if (IS_ERR_OR_NULL(skcipher)) {
		tloge("crypto_alloc_skcipher() failed\n");
		return -EFAULT;
	}
	req = skcipher_request_alloc(skcipher, GFP_KERNEL);
	if (!req) {
		tloge("skcipher_request_alloc() failed\n");
		crypto_free_skcipher(skcipher);
		return -ENOMEM;
	}
	ret = crypto_skcipher_setkey(skcipher, key, CIPHER_KEY_BYTESIZE);
	if (ret) {
		tloge("crypto_skcipher_setkey failed, %d\n", ret);
		skcipher_request_free(req);
		crypto_free_skcipher(skcipher);
		return -EFAULT;
	}
	if (memcpy_s(temp_iv, IV_BYTESIZE, iv, IV_BYTESIZE) != EOK) {
		tloge("memcpy_s failed\n");
		skcipher_request_free(req);
		crypto_free_skcipher(skcipher);
		return -EFAULT;
	}
	sg_init_table(&dst, 1); /* init table to 1 */
	sg_init_table(&src, 1); /* init table to 1 */
	sg_set_buf(&dst, output, size);
	sg_set_buf(&src, input, size);
	skcipher_request_set_crypt(req, &src, &dst, size, temp_iv);
	if (encrypto_type)
		ret = crypto_skcipher_encrypt(req);
	else
		ret = crypto_skcipher_decrypt(req);
	skcipher_request_free(req);
	crypto_free_skcipher(skcipher);
	return ret;
}

static int check_params_for_crypto_session(const uint8_t *in, const uint8_t *out,
	const uint8_t *key, uint32_t in_len, uint32_t out_len)
{
	if (!in || !out || !key) {
		tloge("AES-CBC crypto parameters have null pointer\n");
		return -EINVAL;
	}
	if (in_len < IV_BYTESIZE || out_len < IV_BYTESIZE) {
		tloge("AES-CBC crypto data length is invalid\n");
		return -EINVAL;
	}
	return 0;
}

static int crypto_session_aescbc_key256(uint8_t *in, uint32_t in_len, uint8_t *out,
	uint32_t out_len, const uint8_t *key, uint8_t *iv, uint32_t mode)
{
	int ret;
	uint32_t src_len;
	uint32_t dest_len;
	uint8_t *aescbc_iv = NULL;

	ret = check_params_for_crypto_session(in, out, key, in_len, out_len);
	if (ret)
		return ret;

	/*
	 * For iv variable is null, iv is the first 16 bytes
	 * in cryptotext buffer.
	 */
	switch (mode) {
	case ENCRYPT:
		src_len = in_len;
		dest_len = out_len - IV_BYTESIZE;
		aescbc_iv = out + dest_len;
		break;
	case DECRYPT:
		src_len = in_len - IV_BYTESIZE;
		dest_len = out_len;
		aescbc_iv = in + src_len;
		break;
	default:
		tloge("AES-CBC crypto use error mode = %u\n", mode);
		return -EINVAL;
	}

	/* IV is configured by user */
	if (iv != NULL) {
		src_len = in_len;
		dest_len = out_len;
		aescbc_iv = iv;
	}

	if (src_len != dest_len || !src_len ||
		src_len % CIPHER_BLOCK_BYTESIZE) {
		tloge("AES-CBC, plaintext-len must be equal to cryptotext's, src_len=%u, dest_len=%u\n",
			src_len, dest_len);
		return -EINVAL;
	}
	/* IV is configured here */
	if (!iv && mode == ENCRYPT) {
		ret = generate_random_data(aescbc_iv, IV_BYTESIZE);
		if (ret) {
			tloge("Generate AES-CBC iv failed, ret = %d\n", ret);
			return ret;
		}
	}
	return crypto_aescbc_key256(out, in, aescbc_iv, key, src_len, mode);
}

static int generate_encrypted_session_secure_params(
	const struct session_secure_info *secure_info,
	uint8_t *enc_secure_params, size_t enc_params_size)
{
	int ret;
	struct session_secure_params *ree_secure_params = NULL;
	uint32_t secure_aligned_size =
		align_up(sizeof(*ree_secure_params), CIPHER_BLOCK_BYTESIZE);
	uint32_t params_size = secure_aligned_size + IV_BYTESIZE;

	if (!secure_info || !enc_secure_params ||
		enc_params_size < params_size) {
		tloge("invalid enc params\n");
		return -EINVAL;
	}
	ree_secure_params = kzalloc(secure_aligned_size, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)ree_secure_params)) {
		tloge("Malloc REE session secure parameters buffer failed\n");
		return -ENOMEM;
	}
	/* Transfer chanllenge word to secure world */
	ree_secure_params->payload.ree2tee.challenge_word = secure_info->challenge_word;
	/* Setting encryption head */
	ret = set_encryption_head(&ree_secure_params->head,
		sizeof(ree_secure_params->payload));
	if (ret) {
		tloge("Set encryption head failed, ret = %d\n", ret);
		ree_secure_params->payload.ree2tee.challenge_word = 0;
		kfree(ree_secure_params);
		return -EINVAL;
	}
	/* Setting padding data */
	ret = crypto_aescbc_cms_padding((uint8_t *)ree_secure_params,
		secure_aligned_size,
		sizeof(struct session_secure_params));
	if (ret) {
		tloge("Set encryption padding data failed, ret = %d\n", ret);
		ree_secure_params->payload.ree2tee.challenge_word = 0;
		kfree(ree_secure_params);
		return -EINVAL;
	}
	/* Encrypt buffer with current session key */
	ret = crypto_session_aescbc_key256((uint8_t *)ree_secure_params,
		secure_aligned_size,
		enc_secure_params, params_size, secure_info->crypto_info.key,
		NULL, ENCRYPT);
	if (ret) {
		tloge("Encrypted session secure parameters failed, ret = %d\n",
		      ret);
		ree_secure_params->payload.ree2tee.challenge_word = 0;
		kfree(ree_secure_params);
		return -EINVAL;
	}
	ree_secure_params->payload.ree2tee.challenge_word = 0;
	kfree(ree_secure_params);
	return 0;
}

struct get_plaintext_size {
	uint32_t pt_size;
	uint32_t pta_size;
	uint32_t total_size;
};

int do_encryption(uint8_t *buffer, uint32_t buffer_size,
	uint32_t payload_size, const uint8_t *key)
{
	struct get_plaintext_size local_text_size;
	uint8_t *cryptotext = NULL;
	uint8_t *plaintext = NULL;
	uint32_t ptb_size;
	struct encryption_head head;
	int ret;

	if (check_param_for_encryption(buffer, buffer_size,
		&plaintext, &ptb_size, key))
		return -EINVAL;

	ret = calc_plaintext_size(&local_text_size.pt_size, payload_size, buffer_size,
		&local_text_size.pta_size, &local_text_size.total_size);
	if (ret)
		goto end;
	cryptotext = kzalloc(local_text_size.total_size, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)cryptotext)) {
		tloge("Malloc failed when doing encryption\n");
		ret = -ENOMEM;
		goto end;
	}
	/* Setting encryption head */
	ret = set_encryption_head(&head, payload_size);
	if (ret) {
		tloge("Set encryption head failed, ret = %d\n", ret);
		goto end;
	}
	ret = memcpy_s((void *)(plaintext + payload_size),
		ptb_size - payload_size, &head, sizeof(head));
	if (ret) {
		tloge("Copy encryption head failed, ret = %d\n", ret);
		goto end;
	}
	/* Setting padding data */
	ret = crypto_aescbc_cms_padding(plaintext, local_text_size.pta_size, local_text_size.pt_size);
	if (ret) {
		tloge("Set encryption padding data failed, ret = %d\n", ret);
		goto end;
	}
	ret = crypto_session_aescbc_key256(plaintext, local_text_size.pta_size,
		cryptotext, local_text_size.total_size, key, NULL, ENCRYPT);
	if (ret) {
		tloge("Encrypt failed, ret = %d\n", ret);
		goto end;
	}
	ret = memcpy_s((void *)buffer, buffer_size, (void *)cryptotext,
		local_text_size.total_size);
end:
	return handle_end(plaintext, cryptotext, ret);
}

int encrypt_login_info(uint32_t login_info_size,
	uint8_t *buffer, const uint8_t *key)
{
	uint32_t payload_size;
	uint32_t plaintext_size;
	uint32_t plaintext_aligned_size;
	uint32_t total_size;

	if (!buffer || !key) {
		tloge("Login information buffer is null\n");
		return -EINVAL;
	}
	/* Need adding the termination null byte ('\0') to the end. */
	payload_size = login_info_size + sizeof(char);

	/* Payload + Head + Padding */
	plaintext_size = payload_size + sizeof(struct encryption_head);
	plaintext_aligned_size = round_up(plaintext_size, CIPHER_BLOCK_BYTESIZE);
	/* Need 16 bytes to store AES-CBC iv */
	total_size = plaintext_aligned_size + IV_BYTESIZE;
	if (!is_valid_size(login_info_size, total_size)) {
		tloge("Login information encryption size is invalid\n");
		return -EFAULT;
	}
	return do_encryption(buffer, total_size, payload_size, key);
}

static int32_t save_token_info(void *dst_teec, uint32_t dst_size,
	uint8_t *src_buf, uint32_t src_size, uint8_t kernel_api)
{
	uint8_t temp_teec_token[TOKEN_SAVE_LEN] = {0};
	bool check_value = (!dst_teec || !src_buf ||
		dst_size != TOKEN_SAVE_LEN || !src_size);

	if (check_value) {
		tloge("dst data or src data is invalid\n");
		return -EINVAL;
	}
	/* copy libteec_token && timestamp to libteec */
	if (memmove_s(temp_teec_token, sizeof(temp_teec_token),
		src_buf, TIMESTAMP_SAVE_INDEX)) {
		tloge("copy teec token failed\n");
		return -EFAULT;
	}
	if (memmove_s(&temp_teec_token[TIMESTAMP_SAVE_INDEX],
		TIMESTAMP_LEN_DEFAULT, &src_buf[TIMESTAMP_BUFFER_INDEX],
		TIMESTAMP_LEN_DEFAULT)) {
		tloge("copy teec timestamp failed\n");
		return -EFAULT;
	}
	/* copy libteec_token to libteec */
	if (write_to_client(dst_teec, dst_size,
		temp_teec_token, TOKEN_SAVE_LEN,
		kernel_api)) {
		tloge("copy teec token & timestamp failed\n");
		return -EFAULT;
	}
	/* clear libteec(16byte) */
	if (memset_s(src_buf, TIMESTAMP_SAVE_INDEX, 0,
		TIMESTAMP_SAVE_INDEX)) {
		tloge("clear libteec failed\n");
		return -EFAULT;
	}
	return 0;
}

static int combine_temp_token(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, struct tc_ns_token *tc_token)
{
	uint8_t temp_libteec_token[TOKEN_SAVE_LEN] = {0};

	if (read_from_client(temp_libteec_token, TOKEN_SAVE_LEN,
		call_params->context->teec_token, TOKEN_SAVE_LEN,
		call_params->dev->kernel_api)) {
			tloge("copy libteec token failed\n");
			return -EFAULT;
	}

	if (memcmp(&temp_libteec_token[TIMESTAMP_SAVE_INDEX],
		&tc_token->token_buffer[TIMESTAMP_BUFFER_INDEX],
		TIMESTAMP_LEN_DEFAULT)) {
		tloge("timestamp compare failed\n");
		return -EFAULT;
	}

	/* combine token_buffer teec_token, 0-15byte */
	if (memmove_s(tc_token->token_buffer, TIMESTAMP_SAVE_INDEX,
		temp_libteec_token, TIMESTAMP_SAVE_INDEX)) {
		tloge("copy buffer failed\n");
		if (memset_s(tc_token->token_buffer, tc_token->token_len,
			0, TOKEN_BUFFER_LEN))
			tloge("memset buffer failed\n");
		return -EFAULT;
	}

	/* kernal_api, 40byte */
	if (memmove_s((tc_token->token_buffer + KERNAL_API_INDEX),
		KERNAL_API_LEN, &call_params->dev->kernel_api,
		KERNAL_API_LEN)) {
		tloge("copy KERNAL_API_LEN failed\n");
		if (memset_s(tc_token->token_buffer,
			tc_token->token_len, 0, TOKEN_BUFFER_LEN))
			tloge("fill info memset failed\n");
		return -EFAULT;
	}

	return 0;
}

static int fill_token_info(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, bool is_global)
{
	struct tc_ns_token *tc_token = (call_params->sess) ?
		&(call_params->sess->teec_token) : NULL;

	if (!call_params->context->teec_token || !tc_token ||
		!tc_token->token_buffer) {
		tloge("token or token_buffer is NULL\n");
		return -EINVAL;
	}

	if (call_params->context->cmd_id == GLOBAL_CMD_ID_CLOSE_SESSION ||
		!is_global) {
		if (combine_temp_token(call_params, op_params, tc_token))
			return -EFAULT;
	} else { /* open_session, set token_buffer 0 */
		if (memset_s(tc_token->token_buffer, tc_token->token_len,
			0, TOKEN_BUFFER_LEN)) {
			tloge("memset token_buffer fail\n");
			return -EFAULT;
		}
	}

	if (memcpy_s(op_params->mb_pack->token, TOKEN_BUFFER_LEN, tc_token->token_buffer,
		tc_token->token_len)) {
		tloge("copy token failed\n");
		return -EFAULT;
	}

	op_params->smc_cmd->pid = current->tgid;
	op_params->smc_cmd->token_phys =
		virt_to_phys(op_params->mb_pack->token);
	op_params->smc_cmd->token_h_phys =
		virt_to_phys(op_params->mb_pack->token) >> ADDR_TRANS_NUM;

	return 0;
}

static bool is_token_work(bool is_global, const struct tc_ns_smc_cmd *smc_cmd)
{
	/* invoke cmd(global is false) or open session */
	return (!is_global || smc_cmd->cmd_id == GLOBAL_CMD_ID_OPEN_SESSION);
}

static bool is_load_info_params_valid(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	if (!call_params->dev || !call_params->context ||
		!op_params->mb_pack) {
		tloge("parameter is invalid\n");
		return false;
	}

	return true;
}

int load_security_enhance_info(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	int ret;
	struct tc_ns_session *session = NULL;
	struct tc_ns_smc_cmd *smc_cmd = NULL;
	struct mb_cmd_pack *mb_pack = NULL;
	bool is_global = false;

	if (!call_params || !op_params || !op_params->smc_cmd)
		return -EINVAL;

	is_global = call_params->flags & TC_CALL_GLOBAL;
	if (!is_token_work(is_global, op_params->smc_cmd))
		return 0;

	if (!is_load_info_params_valid(call_params, op_params))
		return -EFAULT;

	session = call_params->sess;
	smc_cmd = op_params->smc_cmd;
	mb_pack = op_params->mb_pack;

	ret = fill_token_info(call_params, op_params, is_global);
	if (ret) {
		tloge("fill info failed. global=%d, cmd id=%u, session id=%u\n",
			is_global, smc_cmd->cmd_id, smc_cmd->context_id);
		return -EFAULT;
	}

	if (!(is_global && smc_cmd->cmd_id == GLOBAL_CMD_ID_OPEN_SESSION))
		return 0;

	if (!session) {
		tloge("invalid session when load secure info\n");
		return -EFAULT;
	}
	if (generate_encrypted_session_secure_params(
		&session->secure_info, mb_pack->secure_params,
		sizeof(mb_pack->secure_params))) {
		tloge("Can't get encrypted session parameters buffer");
		return -EFAULT;
	}
	smc_cmd->params_phys =
		virt_to_phys((void *)mb_pack->secure_params);
	smc_cmd->params_h_phys =
		virt_to_phys((void *)mb_pack->secure_params) >> ADDR_TRANS_NUM;

	return 0;
}

static bool is_token_params_valid(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	if (!op_params || !call_params->dev ||
		!call_params->context || !call_params->context->teec_token ||
		!op_params->mb_pack || !op_params->smc_cmd) {
		tloge("parameter is invalid\n");
		return false;
	}

	return true;
}

int append_teec_token(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	uint8_t temp_libteec_token[TOKEN_SAVE_LEN] = {0};
	struct tc_ns_token *tc_token = NULL;

	if (!call_params)
		return -EINVAL;

	/* only send cmd, we append token */
	if (call_params->flags & TC_CALL_GLOBAL)
		return 0;

	if (!is_token_params_valid(call_params, op_params))
		return -EINVAL;

	tc_token = (call_params->sess) ?
		&(call_params->sess->teec_token) : NULL;
	if (!tc_token || !tc_token->token_buffer) {
		tloge("token or token_buffer is null\n");
		return -EINVAL;
	}

	if (read_from_client(temp_libteec_token, TOKEN_SAVE_LEN,
		call_params->context->teec_token, TOKEN_SAVE_LEN,
		call_params->dev->kernel_api)) {
		tloge("copy libteec token failed\n");
		return -EFAULT;
	}

	/* combine token_buffer ,teec_token, 0-15byte */
	if (memmove_s(tc_token->token_buffer, tc_token->token_len,
		temp_libteec_token, TIMESTAMP_SAVE_INDEX)) {
		tloge("copy temp_libteec_token failed\n");
		if (memset_s(tc_token->token_buffer, tc_token->token_len,
			0, TOKEN_BUFFER_LEN))
			tloge("memset failed\n");
		return -EFAULT;
	}

	if (memcpy_s(op_params->mb_pack->token, TOKEN_BUFFER_LEN,
		tc_token->token_buffer, tc_token->token_len)) {
		tloge("copy token failed\n");
		return -EFAULT;
	}
	return 0;
}

int post_process_token(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	int ret;
	struct tc_ns_token *tc_token = NULL;
	bool is_global = false;

	if (!call_params) {
		tloge("invalid param\n");
		return -EINVAL;
	}

	is_global = call_params->flags & TC_CALL_GLOBAL;
	if (!is_token_work(is_global, op_params->smc_cmd))
		return 0;

	if (!is_token_params_valid(call_params, op_params))
		return -EINVAL;

	tc_token = (call_params->sess) ?
		&(call_params->sess->teec_token) : NULL;
	if (!tc_token || !tc_token->token_buffer) {
		tloge("token_buffer is null\n");
		return -EINVAL;
	}

	if (memcpy_s(tc_token->token_buffer, tc_token->token_len,
		op_params->mb_pack->token, TOKEN_BUFFER_LEN)) {
		tloge("copy token failed\n");
		return -EFAULT;
	}

	if (memset_s(op_params->mb_pack->token, TOKEN_BUFFER_LEN,
		0, TOKEN_BUFFER_LEN)) {
		tloge("memset mb_pack token error\n");
		return -EFAULT;
	}

	if (sync_timestamp(op_params->smc_cmd, tc_token->token_buffer,
		tc_token->token_len, is_global)) {
		tloge("sync time stamp error\n");
		return -EFAULT;
	}

	ret = save_token_info(call_params->context->teec_token,
		call_params->context->token_len, tc_token->token_buffer,
		tc_token->token_len, call_params->dev->kernel_api);
	if (ret) {
		tloge("save token info failed\n");
		return -EFAULT;
	}
	return 0;
}

int tzmp2_uid(const struct tc_ns_client_context *client_context,
	struct tc_ns_smc_cmd *smc_cmd, bool is_global)
{
	struct tc_uuid uuid_tzmp = TEE_TZMP;
	bool check_value = false;

	if (!client_context || !smc_cmd) {
		tloge("client_context or smc_cmd is null\n");
		return -EINVAL;
	}
	if (memcmp(client_context->uuid, (unsigned char *)&uuid_tzmp,
		sizeof(client_context->uuid)))
		return 0;

	check_value = smc_cmd->cmd_id == GLOBAL_CMD_ID_OPEN_SESSION && is_global;
	if (check_value) {
		/* save tzmp_uid */
		mutex_lock(&g_tzmp_lock);
		g_tzmp_uid = 0; /* for multisesion, we use same uid */
		smc_cmd->uid = 0;
		tlogv("openSession, tzmp_uid.uid is %u\n", g_tzmp_uid);
		mutex_unlock(&g_tzmp_lock);
		return EOK;
	}
	mutex_lock(&g_tzmp_lock);
	if (g_tzmp_uid == INVALID_TZMP_UID) {
		tloge("tzmp_uid.uid error\n");
		mutex_unlock(&g_tzmp_lock);
		return -EFAULT;
	}
	smc_cmd->uid = g_tzmp_uid;
	tlogv("invokeCommand or closeSession , tzmp_uid is %u, global is %d\n",
		g_tzmp_uid, is_global);
	mutex_unlock(&g_tzmp_lock);

	return 0;
}

void clean_session_secure_information(struct tc_ns_session *session)
{
	if (session) {
		if (memset_s((void *)&session->secure_info,
			sizeof(session->secure_info), 0, sizeof(session->secure_info)))
			tloge("Clean this session secure information failed\n");
	}
}

static int alloc_secure_params(uint32_t secure_aligned_size,
	uint32_t params_size, struct session_secure_params **ree_secure_params,
	struct session_secure_params **tee_secure_params)
{
	if (!secure_aligned_size) {
		tloge("secure_aligned_size is invalid\n");
		return -ENOMEM;
	}
	*ree_secure_params = mailbox_alloc(params_size, 0);
	if (!*ree_secure_params) {
		tloge("Malloc REE session secure parameters buffer failed\n");
		return -ENOMEM;
	}
	*tee_secure_params = kzalloc(secure_aligned_size, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)(*tee_secure_params))) {
		tloge("Malloc TEE session secure parameters buffer failed\n");
		mailbox_free(*ree_secure_params);
		return -ENOMEM;
	}
	return 0;
}

static int init_for_alloc_secure_params(
	struct get_secure_info_params *params_in,
	uint32_t *secure_aligned_size, uint32_t *params_size)
{
	int ret;

	ret = generate_challenge_word(
		(uint8_t *)&params_in->session->secure_info.challenge_word,
		sizeof(params_in->session->secure_info.challenge_word));
	if (ret) {
		tloge("Generate challenge word failed, ret = %d\n", ret);
		return ret;
	}
	*secure_aligned_size =
		align_up(sizeof(struct session_secure_params), CIPHER_BLOCK_BYTESIZE);
	*params_size = *secure_aligned_size + IV_BYTESIZE;
	return 0;
}

static int send_smc_cmd_for_secure_params(
	const struct get_secure_info_params *params_in,
	struct session_secure_params *ree_secure_params)
{
	struct tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	kuid_t kuid;
	uint32_t uid;

	kuid = current_uid();
	uid = kuid.val;
	/* Transfer chanllenge word to secure world */
	ree_secure_params->payload.ree2tee.challenge_word =
		params_in->session->secure_info.challenge_word;
	smc_cmd.global_cmd = true;
	if (memcpy_s(smc_cmd.uuid, sizeof(smc_cmd.uuid),
		params_in->context->uuid, UUID_LEN)) {
		tloge("memcpy_s uuid error\n");
		return -EFAULT;
	}
	smc_cmd.cmd_id = GLOBAL_CMD_ID_GET_SESSION_SECURE_PARAMS;
	smc_cmd.dev_file_id = params_in->dev_file->dev_file_id;
	smc_cmd.context_id = params_in->context->session_id;
	smc_cmd.operation_phys = 0;
	smc_cmd.operation_h_phys = 0;
	smc_cmd.login_data_phy = 0;
	smc_cmd.login_data_h_addr = 0;
	smc_cmd.login_data_len = 0;
	smc_cmd.err_origin = 0;
	smc_cmd.uid = uid;
	smc_cmd.started = params_in->context->started;
	smc_cmd.params_phys = virt_to_phys((void *)ree_secure_params);
	smc_cmd.params_h_phys = virt_to_phys((void *)ree_secure_params) >> ADDR_TRANS_NUM;

	if (tc_ns_smc(&smc_cmd)) {
		ree_secure_params->payload.ree2tee.challenge_word = 0;
		tloge("tc ns smc returns error, ret = %d\n", smc_cmd.ret_val);
		return -EPERM;
	}
	return 0;
}

static bool is_valid_encryption_head(const struct encryption_head *head,
	const uint8_t *data, uint32_t len)
{
	if (!head || !data || !len) {
		tloge("In parameters check failed\n");
		return false;
	}
	if (strncmp(head->magic, MAGIC_STRING, sizeof(MAGIC_STRING))) {
		tloge("Magic string is invalid\n");
		return false;
	}
	if (head->payload_len != len) {
		tloge("Payload length is invalid\n");
		return false;
	}
	return true;
}

static int update_secure_params_from_tee(
	struct get_secure_info_params *params_in,
	struct session_secure_params *ree_secure_params,
	struct session_secure_params *tee_secure_params,
	uint32_t secure_aligned_size,
	uint32_t params_size)
{
	int ret;
	uint8_t *enc_secure_params = NULL;
	/* Get encrypted session secure parameters from secure world */
	enc_secure_params = (uint8_t *)ree_secure_params;
	ret = crypto_session_aescbc_key256(enc_secure_params, params_size,
		(uint8_t *)tee_secure_params, secure_aligned_size,
		g_session_root_key->key, NULL, DECRYPT);
	if (ret) {
		tloge("Decrypted session secure parameters failed, ret = %d\n", ret);
		return ret;
	}
	/* Analyze encryption head */
	if (!is_valid_encryption_head(&tee_secure_params->head,
		(uint8_t *)&tee_secure_params->payload,
		sizeof(tee_secure_params->payload)))
		return -EFAULT;

	/* Store session secure parameters */
	ret = memcpy_s((void *)params_in->session->secure_info.scrambling,
		sizeof(params_in->session->secure_info.scrambling),
		(void *)tee_secure_params->payload.tee2ree.scrambling,
		sizeof(tee_secure_params->payload.tee2ree.scrambling));
	if (ret) {
		tloge("Memcpy scrambling data failed, ret = %d\n", ret);
		return ret;
	}
	ret = memcpy_s((void *)&params_in->session->secure_info.crypto_info,
		sizeof(params_in->session->secure_info.crypto_info),
		(void *)&tee_secure_params->payload.tee2ree.crypto_info,
		sizeof(tee_secure_params->payload.tee2ree.crypto_info));
	if (ret) {
		tloge("Memcpy session crypto information failed, ret = %d\n", ret);
		return ret;
	}
	return 0;
}

int get_session_secure_params(struct tc_ns_dev_file *dev_file,
	struct tc_ns_client_context *context, struct tc_ns_session *session)
{
	int ret;
	uint32_t params_size;
	uint32_t secure_aligned_size;
	struct session_secure_params *ree_secure_params = NULL;
	struct session_secure_params *tee_secure_params = NULL;
	bool check_value = false;
	struct get_secure_info_params params_in = { dev_file, context, session };

	check_value = (!dev_file || !context || !session);
	if (check_value) {
		tloge("Parameter is null pointer\n");
		return -EINVAL;
	}
	ret = init_for_alloc_secure_params(&params_in,
		&secure_aligned_size, &params_size);
	if (ret)
		return ret;
	ret = alloc_secure_params(secure_aligned_size,
		params_size, &ree_secure_params, &tee_secure_params);
	if (ret)
		return ret;
	ret = send_smc_cmd_for_secure_params(&params_in, ree_secure_params);
	if (ret)
		goto free;

	ret = update_secure_params_from_tee(&params_in, ree_secure_params,
		tee_secure_params, secure_aligned_size, params_size);
	if (memset_s((void *)tee_secure_params, secure_aligned_size,
		0, secure_aligned_size))
		tloge("Clean the secure parameters buffer failed\n");
free:
	mailbox_free(ree_secure_params);
	ree_secure_params = NULL;
	kfree(tee_secure_params);
	tee_secure_params = NULL;
	if (ret)
		clean_session_secure_information(session);
	return ret;
}

void free_root_key(void)
{
	if (IS_ERR_OR_NULL(g_session_root_key))
		return;

	if (memset_s((void *)g_session_root_key, sizeof(*g_session_root_key),
		0, sizeof(*g_session_root_key)))
		tloge("memset mem failed\n");
	kfree(g_session_root_key);
	g_session_root_key = NULL;
}

int get_session_root_key(void)
{
	int ret;
	uint32_t *buffer = NULL;
	struct tc_ns_smc_queue *cmd_data = NULL;

	cmd_data = get_cmd_data_buffer();
	if (!cmd_data) {
		tloge("cmd data buffer is null\n");
		return -EFAULT;
	}

	buffer = (uint32_t *)(cmd_data->in);
	if (!buffer || ((uintptr_t)buffer & ALIGN_BIT)) {
		tloge("Session root key must be 4bytes aligned\n");
		return -EFAULT;
	}

	g_session_root_key = kzalloc(sizeof(*g_session_root_key),
		GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)g_session_root_key)) {
		tloge("No memory to store session root key\n");
		return -ENOMEM;
	}

	if (memcpy_s(g_session_root_key, sizeof(*g_session_root_key),
		(void *)(buffer + 1), sizeof(*g_session_root_key))) {
		tloge("Copy session root key from TEE failed\n");
		ret = -EFAULT;
		goto free_mem;
	}
	if (memset_s((void *)(cmd_data->in), sizeof(cmd_data->in),
		0, sizeof(cmd_data->in))) {
		tloge("Clean the command buffer failed\n");
		ret = -EFAULT;
		goto free_mem;
	}
	return 0;
free_mem:
	free_root_key();
	return ret;
}
