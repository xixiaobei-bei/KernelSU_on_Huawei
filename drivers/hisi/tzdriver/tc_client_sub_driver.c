

#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <asm/cacheflush.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_reserved_mem.h>
#include <linux/atomic.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/pid.h>
#include <linux/security.h>
#include <linux/cred.h>
#include <linux/namei.h>
#include <linux/thread_info.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/file.h>
#include <linux/scatterlist.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/security.h>
#include <linux/path.h>

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <crypto/skcipher.h>
#include <linux/sched/mm.h>
#include <linux/sched/signal.h>
#endif

#include "smc.h"
#include "teek_client_constants.h"
#include "tc_ns_client.h"
#include "mem.h"
#include "tee_rdr_register.h"
#include "tui.h"
#include "gp_ops.h"
#include "teek_client_type.h"
#include "libhwsecurec/securec.h"
#include "tc_ns_log.h"
#include "cfc.h"
#include "mailbox_mempool.h"
#include "tz_spi_notify.h"
#include "tc_client_driver.h"
#include "tc_client_sub_driver.h"
#include "smc.h"
#include "teek_client_constants.h"
#include "tc_ns_client.h"
#include "agent.h"
#include "mem.h"
#include "tee_rdr_register.h"
#include "tui.h"
#include "gp_ops.h"
#include "teek_client_type.h"
#include "libhwsecurec/securec.h"
#include "tc_ns_log.h"
#include "cfc.h"
#include "mailbox_mempool.h"
#include "tz_spi_notify.h"
#include "tc_client_driver.h"
#include "tc_client_sub_driver.h"

#include <linux/random.h>
#include <linux/crc32.h>
#include "security_auth_enhance.h"

#include "dynamic_mem.h"

static DEFINE_MUTEX(load_app_lock);


typedef struct {
	tc_ns_dev_file *dev_file;
	tc_ns_client_context *context;
	tc_ns_session *session;
} get_secure_info_params;

static int check_random_data(uint8_t *data, uint32_t size)
{
	uint32_t i;

	for (i = 0; i < size; i++) {
		if (data[i] != 0)
			break;
	}
	if (i >= size)
		return -1;
	return 0;
}

int generate_random_data(uint8_t *data, uint32_t size)
{
	if (data == NULL) {
		tloge("Bad parameters!\n");
		return -EFAULT;
	}
	if (memset_s((void *)data, size, 0, size)) {
		tloge("Clean the data buffer failed!\n");
		return -EFAULT;
	}
	get_random_bytes_arch((void *)data, size);
	if (check_random_data(data, size) != 0)
		return -EFAULT;
	return 0;
}

bool is_valid_encryption_head(const struct encryption_head *head,
	const uint8_t *data, uint32_t len)
{
	if (head == NULL || data == NULL || len == 0) {
		tloge("In parameters check failed.\n");
		return false;
	}
	if (strncmp(head->magic, MAGIC_STRING, sizeof(MAGIC_STRING))) {
		tloge("Magic string is invalid.\n");
		return false;
	}
	if (head->payload_len != len) {
		tloge("Payload length is invalid.\n");
		return false;
	}
	return true;
}

int generate_challenge_word(uint8_t *challenge_word, uint32_t size)
{
	if (challenge_word == NULL) {
		tloge("Parameter is null pointer!\n");
		return -EINVAL;
	}
	return generate_random_data(challenge_word, size);
}

int set_encryption_head(struct encryption_head *head, const uint8_t *data,
	uint32_t len)
{
	if (head == NULL || data == NULL || len == 0) {
		tloge("In parameters check failed.\n");
		return -EINVAL;
	}
	if (strncpy_s(head->magic, sizeof(head->magic),
		MAGIC_STRING, strlen(MAGIC_STRING) + 1)) {
		tloge("Copy magic string failed.\n");
		return -EFAULT;
	}
	head->payload_len = len;
	return 0;
}

static tc_ns_dev_file *tc_find_dev_file(unsigned int dev_file_id)
{
	tc_ns_dev_file *dev_file = NULL;

	list_for_each_entry(dev_file, &g_tc_ns_dev_list.dev_file_list, head) {
		if (dev_file->dev_file_id == dev_file_id)
			return dev_file;
	}
	return NULL;
}

tc_ns_session *tc_find_session2(unsigned int dev_file_id,
	unsigned int context_id, unsigned char *uuid)
{
	tc_ns_dev_file *dev_file = NULL;
	tc_ns_service *service = NULL;
	tc_ns_session *session = NULL;

	if (uuid == NULL) {
		tloge("Parameter is null pointer!\n");
		return NULL;
	}
	mutex_lock(&g_tc_ns_dev_list.dev_lock);
	dev_file = tc_find_dev_file(dev_file_id);
	mutex_unlock(&g_tc_ns_dev_list.dev_lock);
	if (dev_file == NULL) {
		tloge("Can't find dev file!\n");
		return NULL;
	}
	mutex_lock(&dev_file->service_lock);
	service = tc_find_service_in_dev(dev_file, uuid, false);
	get_service_struct(service);
	mutex_unlock(&dev_file->service_lock);
	if (service == NULL) {
		tloge(" Can't find service!\n");
		return NULL;
	}
	mutex_lock(&service->session_lock);
	session = tc_find_session_withowner(&service->session_list,
		context_id, dev_file);
	get_session_struct(session);
	mutex_unlock(&service->session_lock);
	put_service_struct(service);
	if (session == NULL) {
		tloge("can't find session[0x%x]!\n", context_id);
		return NULL;
	}
	return session;
}

#define ALIGN_UP(x, align) (((x) + ((align)-1)) & ~((align)-1))

void clean_session_secure_information(tc_ns_session *session)
{
	if (session != NULL) {
		if (memset_s((void *)&session->secure_info,
			sizeof(session->secure_info), 0, sizeof(session->secure_info)))
			tloge("Clean this session secure information failed!\n");
	}
}

static int alloc_secure_params(uint32_t secure_params_aligned_size,
	uint32_t params_size, struct session_secure_params **ree_secure_params,
	struct session_secure_params **tee_secure_params)
{
	*ree_secure_params = mailbox_alloc(params_size, 0);
	if (*ree_secure_params == NULL) {
		tloge("Malloc REE session secure parameters buffer failed.\n");
		return -ENOMEM;
	}
	*tee_secure_params = kzalloc(secure_params_aligned_size, GFP_KERNEL);
	if (*tee_secure_params == NULL) {
		mailbox_free(*ree_secure_params);
		tloge("Malloc TEE session secure parameters buffer failed.\n");
		return -ENOMEM;
	}
	return 0;
}

static int init_for_alloc_secure_params(get_secure_info_params params_in,
	struct mb_cmd_pack **mb_pack, uint32_t *secure_params_aligned_size,
	uint32_t *params_size)
{
	int ret = 0;

	ret = generate_challenge_word(
		(uint8_t *)&params_in.session->secure_info.challenge_word,
		sizeof(params_in.session->secure_info.challenge_word));
	if (ret) {
		tloge("Generate challenge word failed, ret = %d\n", ret);
		return ret;
	}
	*mb_pack = mailbox_alloc_cmd_pack();
	if (*mb_pack == NULL) {
		return -ENOMEM;
	}
	(*mb_pack)->uuid[0] = 1; /* TC_CALL_GLOBAL */
	ret = memcpy_s((*mb_pack)->uuid + 1, UUID_LEN,
		params_in.context->uuid, sizeof(params_in.context->uuid));
	if (ret) {
		tloge("Memcpy uuid failed, ret = %d,\n", ret);
		mailbox_free(*mb_pack);
		return ret;
	}
	*secure_params_aligned_size =
		ALIGN_UP(sizeof(struct session_secure_params), CIPHER_BLOCK_BYTESIZE);
	*params_size = *secure_params_aligned_size + IV_BYTESIZE;
	return 0;
}

static int send_smc_cmd_for_secure_params(get_secure_info_params params_in,
	struct session_secure_params *ree_secure_params, struct mb_cmd_pack *mb_pack)
{
	int ret = 0;
	tc_ns_smc_cmd smc_cmd = {0};
	kuid_t kuid;
	uint32_t uid;

#if (KERNEL_VERSION(3, 13, 0) <= LINUX_VERSION_CODE)
	kuid = current_uid(); /*lint !e666*/
	uid = kuid.val;
#else
	uid = current_uid();
#endif
	/* Transfer chanllenge word to secure world */
	ree_secure_params->payload.ree2tee.challenge_word =
		params_in.session->secure_info.challenge_word;
	smc_cmd.uuid_phys = virt_to_phys((void *)mb_pack->uuid);
	smc_cmd.uuid_h_phys = virt_to_phys((void *)mb_pack->uuid) >> 32;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_GET_SESSION_SECURE_PARAMS;
	smc_cmd.dev_file_id = params_in.dev_file->dev_file_id;
	smc_cmd.context_id = params_in.context->session_id;
	smc_cmd.operation_phys = 0;
	smc_cmd.operation_h_phys = 0;
	smc_cmd.login_data_phy = 0;
	smc_cmd.login_data_h_addr = 0;
	smc_cmd.login_data_len = 0;
	smc_cmd.err_origin = 0;
	smc_cmd.uid = uid;
	smc_cmd.started = params_in.context->started;
	smc_cmd.params_phys = virt_to_phys((void *)ree_secure_params);
	smc_cmd.params_h_phys = virt_to_phys((void *)ree_secure_params) >> 32;
	ret = tc_ns_smc(&smc_cmd);
	if (ret) {
		ree_secure_params->payload.ree2tee.challenge_word = 0;
		tloge("TC_NS_SMC returns error, ret = %d\n", ret);
		return ret;
	}
	return 0;
}

static int update_secure_params_from_tee(get_secure_info_params params_in,
	struct session_secure_params *ree_secure_params,
	struct session_secure_params *tee_secure_params,
	uint32_t secure_params_aligned_size,
	uint32_t params_size)
{
	int ret = 0;
	uint8_t *enc_secure_params = NULL;
	/* Get encrypted session secure parameters from secure world */
	enc_secure_params = (uint8_t *)ree_secure_params;
	ret = crypto_session_aescbc_key256(enc_secure_params, params_size,
		(uint8_t *)tee_secure_params, secure_params_aligned_size,
		g_session_root_key->key, NULL, DECRYPT);
	if (ret) {
		tloge("Decrypted session secure parameters failed, ret = %d.\n", ret);
		return ret;
	}
	/* Analyze encryption head */
	if (!is_valid_encryption_head(&tee_secure_params->head,
		(uint8_t *)&tee_secure_params->payload,
		sizeof(tee_secure_params->payload)))
		return -EFAULT;

	/* Store session secure parameters */
	ret = memcpy_s((void *)params_in.session->secure_info.scrambling,
		sizeof(params_in.session->secure_info.scrambling),
		(void *)&tee_secure_params->payload.tee2ree.scrambling,
		sizeof(tee_secure_params->payload.tee2ree.scrambling));
	if (ret) {
		tloge("Memcpy scrambling data failed, ret = %d.\n", ret);
		return ret;
	}
	ret = memcpy_s((void *)&params_in.session->secure_info.crypto_info,
		sizeof(struct session_crypto_info),
		(void *)&tee_secure_params->payload.tee2ree.crypto_info,
		sizeof(struct session_crypto_info));
	if (ret) {
		tloge("Memcpy session crypto information failed, ret = %d.\n", ret);
		return ret;
	}
	return 0;
}

int get_session_secure_params(tc_ns_dev_file *dev_file,
	tc_ns_client_context *context, tc_ns_session *session)
{
	int ret = 0;
	uint32_t params_size;
	uint32_t secure_params_aligned_size;
	struct session_secure_params *ree_secure_params = NULL;
	struct session_secure_params *tee_secure_params = NULL;
	struct mb_cmd_pack *mb_pack = NULL;
	bool check_value = false;
	get_secure_info_params params_in = { dev_file, context, session };

	check_value = (dev_file == NULL || context == NULL || session == NULL);
	if (check_value == true) {
		tloge("Parameter is null pointer!\n");
		return -EINVAL;
	}
	ret = init_for_alloc_secure_params(params_in, &mb_pack,
		&secure_params_aligned_size, &params_size);
	if (ret != 0)
		return ret;
	ret = alloc_secure_params(secure_params_aligned_size,
		params_size, &ree_secure_params, &tee_secure_params);
	if (ret != 0) {
		mailbox_free(mb_pack);
		return ret;
	}
	ret = send_smc_cmd_for_secure_params(params_in, ree_secure_params, mb_pack);
	if (ret != 0)
		goto free;

	ret = update_secure_params_from_tee(params_in, ree_secure_params,
		tee_secure_params, secure_params_aligned_size, params_size);
	if (memset_s((void *)tee_secure_params, secure_params_aligned_size,
		0, secure_params_aligned_size))
		tloge("Clean the secure parameters buffer failed!\n");
free:
	mailbox_free(mb_pack);
	mailbox_free(ree_secure_params);
	kfree(tee_secure_params);
	if (ret)
		clean_session_secure_information(session);
	return ret;
}

int generate_encrypted_session_secure_params(
	struct session_secure_info *secure_info,
	uint8_t *enc_secure_params, size_t enc_params_size)
{
	int ret = 0;
	uint32_t secure_params_aligned_size =
		ALIGN_UP(sizeof(struct session_secure_params), CIPHER_BLOCK_BYTESIZE);
	uint32_t params_size = secure_params_aligned_size + IV_BYTESIZE;
	struct session_secure_params *ree_secure_params = NULL;

	if (enc_secure_params == NULL || enc_params_size < params_size) {
		tloge("invalid enc params\n");
		return -EINVAL;
	}
	ree_secure_params = kzalloc(secure_params_aligned_size, GFP_KERNEL);
	if (ree_secure_params == NULL) {
		tloge("Malloc REE session secure parameters buffer failed.\n");
		return -ENOMEM;
	}
	/* Transfer chanllenge word to secure world */
	ree_secure_params->payload.ree2tee.challenge_word = secure_info->challenge_word;
	/* Setting encryption head */
	ret = set_encryption_head(&ree_secure_params->head,
		(uint8_t *)&ree_secure_params->payload,
		sizeof(ree_secure_params->payload));
	if (ret) {
		ree_secure_params->payload.ree2tee.challenge_word = 0;
		kfree(ree_secure_params);
		tloge("Set encryption head failed, ret = %d.\n", ret);
		return -EINVAL;
	}
	/* Setting padding data */
	ret = crypto_aescbc_cms_padding((uint8_t *)ree_secure_params,
		secure_params_aligned_size,
		sizeof(struct session_secure_params));
	if (ret) {
		ree_secure_params->payload.ree2tee.challenge_word = 0;
		kfree(ree_secure_params);
		tloge("Set encryption padding data failed, ret = %d.\n", ret);
		return -EINVAL;
	}
	/* Encrypt buffer with current session key */
	ret = crypto_session_aescbc_key256((uint8_t *)ree_secure_params,
		secure_params_aligned_size,
		enc_secure_params, params_size, secure_info->crypto_info.key,
		NULL, ENCRYPT);
	if (ret) {
		ree_secure_params->payload.ree2tee.challenge_word = 0;
		kfree(ree_secure_params);
		tloge("Encrypted session secure parameters failed, ret = %d.\n",
		      ret);
		return -EINVAL;
	}
	ree_secure_params->payload.ree2tee.challenge_word = 0;
	kfree(ree_secure_params);
	return 0;
}

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
static int crypto_aescbc_key256(uint8_t *output, const uint8_t *input,
	const uint8_t *iv, const uint8_t *key, int32_t size, uint32_t encrypto_type)
{
	struct scatterlist src;
	struct scatterlist dst;
	struct crypto_skcipher *skcipher = NULL;
	struct skcipher_request *req = NULL;
	int ret;
	uint8_t temp_iv[IV_BYTESIZE] = {0};

	skcipher = crypto_alloc_skcipher("cbc(aes)", 0, 0);
	if (IS_ERR(skcipher)) {
		tloge("crypto_alloc_skcipher() failed.\n");
		return -EFAULT;
	}
	req = skcipher_request_alloc(skcipher, GFP_KERNEL);
	if (!req) {
		tloge("skcipher_request_alloc() failed.\n");
		crypto_free_skcipher(skcipher);
		return -ENOMEM;
	}
	ret = crypto_skcipher_setkey(skcipher, key, CIPHER_KEY_BYTESIZE);
	if (ret) {
		tloge("crypto_skcipher_setkey failed. %d\n", ret);
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
	sg_init_table(&dst, 1);
	sg_init_table(&src, 1);
	sg_set_buf(&dst, output, size);
	sg_set_buf(&src, input, size);
	skcipher_request_set_crypt(req, &src, &dst, size, temp_iv);
	if (encrypto_type) {
		ret = crypto_skcipher_encrypt(req);
	} else {
		ret = crypto_skcipher_decrypt(req);
	}
	skcipher_request_free(req);
	crypto_free_skcipher(skcipher);
	return ret;
}
#else
/* size of [iv] is 16 and [key] must be 32 bytes.
 * [size] is the size of [output] and [input].
 * [size] must be multiple of 32.
 */
static int crypto_aescbc_key256(uint8_t *output, const uint8_t *input,
	const uint8_t *iv, const uint8_t *key, int32_t size,
	uint32_t encrypto_type)
{
	struct scatterlist src;
	struct scatterlist dst;
	struct blkcipher_desc desc;
	struct crypto_blkcipher *cipher = NULL;
	int ret;

	cipher = crypto_alloc_blkcipher("cbc(aes)", 0, 0);
	if (IS_ERR(cipher)) {
		tloge("crypto_alloc_blkcipher() failed.\n");
		return -EFAULT;
	}
	ret = crypto_blkcipher_setkey(cipher, key, CIPHER_KEY_BYTESIZE);
	if (ret) {
		tloge("crypto_blkcipher_setkey failed. %d\n", ret);
		crypto_free_blkcipher(cipher);
		return -EFAULT;
	}
	crypto_blkcipher_set_iv(cipher, iv, IV_BYTESIZE);
	sg_init_table(&dst, 1);
	sg_init_table(&src, 1);
	sg_set_buf(&dst, output, size);
	sg_set_buf(&src, input, size);
	desc.tfm = cipher;
	desc.flags = 0;
	if (encrypto_type) {
		ret = crypto_blkcipher_encrypt(&desc, &dst, &src, size);
	} else {
		ret = crypto_blkcipher_decrypt(&desc, &dst, &src, size);
	}
	crypto_free_blkcipher(cipher);
	return ret;
}
#endif

static int check_params_for_crypto_session(uint8_t *in, uint8_t *out,
	const uint8_t *key, uint32_t in_len, uint32_t out_len)
{
	if (in == NULL || out == NULL || key == NULL) {
		tloge("AES-CBC crypto parameters have null pointer.\n");
		return -EINVAL;
	}
	if (in_len < IV_BYTESIZE || out_len < IV_BYTESIZE) {
		tloge("AES-CBC crypto data length is invalid.\n");
		return -EINVAL;
	}
	return 0;
}

int crypto_session_aescbc_key256(uint8_t *in, uint32_t in_len, uint8_t *out,
	uint32_t out_len, const uint8_t *key, uint8_t *iv, uint32_t mode)
{
	int ret;
	uint32_t src_len;
	uint32_t dest_len;
	uint8_t *aescbc_iv = NULL;
	bool check_value = false;

	ret = check_params_for_crypto_session(in, out, key, in_len, out_len);
	if (ret)
		return ret;
	/* For iv variable is null, iv is the first 16 bytes
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
		tloge("AES-CBC crypto use error mode = %d.\n", mode);
		return -EINVAL;
	}

	/* IV is configured by user */
	if (iv) {
		src_len = in_len;
		dest_len = out_len;
		aescbc_iv = iv;
	}
	check_value = (src_len != dest_len) || (!src_len) ||
		(src_len % CIPHER_BLOCK_BYTESIZE);
	if (check_value == true) {
		tloge("AES-CBC, plaintext-len must be equal to cryptotext's. src_len=%d, dest_len=%d.\n",
			src_len, dest_len);
		return -EINVAL;
	}
	/* IV is configured in here */
	check_value = !iv && (mode == ENCRYPT);
	if (check_value == true) {
		ret = generate_random_data(aescbc_iv, IV_BYTESIZE);
		if (ret) {
			tloge("Generate AES-CBC iv failed, ret = %d.\n", ret);
			return ret;
		}
	}
	return crypto_aescbc_key256(out, in, aescbc_iv, key, src_len, mode);
}

/*lint -esym(429,*)*/
int crypto_aescbc_cms_padding(uint8_t *plaintext, uint32_t plaintext_len,
	uint32_t payload_len)
{
	uint32_t padding_len;
	uint8_t padding;
	bool check_value = false;

	if (plaintext == NULL) {
		tloge("Plaintext is NULL.\n");
		return -EINVAL;
	}
	check_value = (!plaintext_len) ||
		(plaintext_len % CIPHER_BLOCK_BYTESIZE) ||
		(plaintext_len < payload_len);
	if (check_value == true) {
		tloge("Plaintext length is invalid.\n");
		return -EINVAL;
	}
	padding_len = plaintext_len - payload_len;
	if (padding_len >= CIPHER_BLOCK_BYTESIZE) {
		tloge("Padding length is error.\n");
		return -EINVAL;
	}
	if (padding_len == 0) {
		/* No need padding */
		return 0;
	}
	padding = (uint8_t)padding_len;
	if (memset_s((void *)(plaintext + payload_len),
		padding_len, padding, padding_len)) {
		tloge("CMS-Padding is failed.\n");
		return -EFAULT;
	}
	return 0;
}
/*lint +esym(429,*)*/

int check_mm_struct(struct mm_struct *mm)
{
	if (mm == NULL)
		return -1;
	if (!mm->exe_file) {
		mmput(mm);
		return -1;
	}
	return 0;
}

char *get_process_path(struct task_struct *task, char *tpath)
{
	char *ret_ptr = NULL;
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	struct path base_path = {0};
#else
	struct path base_path;
#endif
	struct mm_struct *mm = NULL;
	struct file *exe_file = NULL;
	errno_t sret;
	bool check_value = false;

	check_value = (tpath == NULL || task == NULL);
	if (check_value == true)
		return NULL;
	sret = memset_s(tpath, MAX_PATH_SIZE, '\0', MAX_PATH_SIZE);
	if (sret != EOK) {
		tloge("memset_s error sret is %d\n", sret);
		return NULL;
	}
	mm = get_task_mm(task);
	if (check_mm_struct(mm) != 0)
		return NULL;
	exe_file = get_mm_exe_file(mm);
	if (exe_file) {
		base_path = exe_file->f_path;
		path_get(&base_path);
		ret_ptr = d_path(&base_path, tpath, MAX_PATH_SIZE);
		path_put(&base_path);
		fput(exe_file);
	}
	mmput(mm);
	return ret_ptr;
}

int calc_process_path_hash(unsigned char *data,
	unsigned long len, char *digest)
{
	int rc;
	struct sdesc {
		struct shash_desc shash;
		char ctx[];
	};
	struct sdesc *desc = NULL;
	bool check_value = false;

	check_value = (data == NULL || digest == NULL);
	if (check_value == true) {
		tloge("Bad parameters!\n");
		return -EFAULT;
	}
	if (tee_init_crypto("sha256")) {
		tloge("init tee crypto failed\n");
		return -EFAULT;
	}
	desc = kmalloc(sizeof(struct shash_desc) +
		crypto_shash_descsize(g_tee_shash_tfm), GFP_KERNEL);
	if (desc == NULL) {
		tloge("alloc desc failed\n");
		return -ENOMEM;
	}
	desc->shash.tfm = g_tee_shash_tfm;
	desc->shash.flags = 0;
	rc = crypto_shash_digest(&desc->shash, data, len, digest);
	kfree(desc);
	return rc;
}

int pack_ca_cert(int type, char *ca_cert, char *path,
	struct task_struct *ca_task, const struct cred *cred)
{
	int message_size = 0;

	if (ca_cert == NULL || path == NULL ||
	    ca_task == NULL || cred == NULL)
		return 0;
#if (KERNEL_VERSION(3, 13, 0) <= LINUX_VERSION_CODE)
	if (type == NON_HIDL_SIDE) {
		message_size = snprintf_s(ca_cert, BUF_MAX_SIZE - 1,
			BUF_MAX_SIZE - 1, "%s%s%u", ca_task->comm, path,
			cred->uid.val);
	} else {
		message_size = snprintf_s(ca_cert, BUF_MAX_SIZE - 1,
			BUF_MAX_SIZE - 1, "%s%u", path,
			cred->uid.val);
	}
#else
	if (type == NON_HIDL_SIDE) {
		message_size = snprintf_s(ca_cert, BUF_MAX_SIZE - 1,
			BUF_MAX_SIZE - 1, "%s%s%u", ca_task->comm, path,
			cred->uid);
	} else {
		message_size = snprintf_s(ca_cert, BUF_MAX_SIZE - 1,
			BUF_MAX_SIZE - 1, "%s%u", path,
			cred->uid);
	}
#endif
	return message_size;
}

int check_process_selinux_security(struct task_struct *ca_task, char *context)
{
	u32 sid;
	u32 tid;
	int rc = 0;
	bool check_value = false;

	check_value = (ca_task == NULL || context == NULL);
	if (check_value == true)
		return -EPERM;
	security_task_getsecid(ca_task, &sid);
	rc = security_context_str_to_sid(context, &tid, GFP_KERNEL);
	if (rc) {
		tloge("convert context to sid failed\n");
		return rc;
	}
	if (sid != tid) {
		tloge("invalid access process judged by selinux side\n");
		return -EPERM;
	}
	return 0;
}

tc_ns_service *tc_find_service_in_dev(tc_ns_dev_file *dev, unsigned char *uuid,
	bool ref)
{
	uint32_t i;

	if (dev == NULL || uuid == NULL)
		return NULL;
	for (i = 0; i < SERVICES_MAX_COUNT; i++) {
		if (dev->services[i] != NULL &&
			memcmp(dev->services[i]->uuid, uuid, 16) == 0) {
			if (ref)
				dev->service_ref[i]++;
			return dev->services[i];
		}
	}
	return NULL;
}

tc_ns_service *tc_find_service_from_all(unsigned char *uuid)
{
	tc_ns_service *service = NULL;

	if (uuid == NULL)
		return NULL;
	list_for_each_entry(service, &g_service_list, head) {
		if (memcmp(service->uuid, uuid, sizeof(service->uuid)) == 0)
			return service;
	}
	return NULL;
}

int add_service_to_dev(tc_ns_dev_file *dev, tc_ns_service *service)
{
	uint32_t i;

	if (dev == NULL || service == NULL)
		return -1;
	for (i = 0; i < SERVICES_MAX_COUNT; i++) {
		if (dev->services[i] == NULL) {
			tlogd("add service  %d to %p\n", i, dev);
			dev->services[i] = service;
			dev->service_ref[i] = 1;
			return 0;
		}
	}
	return -1;
}

void del_service_from_dev(tc_ns_dev_file *dev, tc_ns_service *service)
{
	uint32_t i;

	if (dev == NULL || service == NULL)
		return;
	for (i = 0; i < SERVICES_MAX_COUNT; i++) {
		if (dev->services[i] == service) {
			tlogd("dev->service_ref[%d] = %d\n", i, dev->service_ref[i]);
			dev->service_ref[i]--;
			if (!dev->service_ref[i]) {
				tlogd("del service  %d from %p\n", i, dev);
				dev->services[i] = NULL;
				put_service_struct(service);
			}
			break;
		}
	}
}

tc_ns_session *tc_find_session_withowner(struct list_head *session_list,
	unsigned int session_id, tc_ns_dev_file *dev_file)
{
	tc_ns_session *session = NULL;
	bool check_value = false;

	check_value = (session_list == NULL || dev_file == NULL);
	if (check_value == true) {
		tloge("session_list or dev_file is Null.\n");
		return ERR_PTR(-EINVAL);
	}
	list_for_each_entry(session, session_list, head) {
		check_value = (session->session_id == session_id &&
			session->owner == dev_file);
		if (check_value == true)
			return session;
	}
	return NULL;
}

void dump_services_status(char *param)
{
	tc_ns_service *service = NULL;

	(void)param;
	mutex_lock(&g_service_list_lock);
	tlogi("show service list:\n");
	list_for_each_entry(service, &g_service_list, head) {
		tlogi("uuid-%x, usage=%d\n", *(uint32_t *)service->uuid,
			atomic_read(&service->usage));
	}
	mutex_unlock(&g_service_list_lock);
}

errno_t init_context(tc_ns_client_context *context, unsigned char *uuid)
{
	errno_t sret;

	if (context == NULL || uuid == NULL)
		return -1;
	sret = memset_s(context, sizeof(tc_ns_client_context), 0,
		sizeof(tc_ns_client_context));
	if (sret != EOK)
		return -1;

	sret = memcpy_s(context->uuid, sizeof(context->uuid), uuid,
		sizeof(context->uuid));
	if (sret != EOK)
		return -1;
	return 0;
}

int close_session(tc_ns_dev_file *dev, tc_ns_session *session,
	unsigned char *uuid, unsigned int session_id)
{
	tc_ns_client_context context;
	int ret = 0;
	errno_t sret;
	bool check_value = false;

	check_value = (dev == NULL || session == NULL || uuid == NULL);
	if (check_value == true) {
		return TEEC_ERROR_GENERIC;
	}
	sret = init_context(&context, uuid);
	if (sret != 0)
		return TEEC_ERROR_GENERIC;
	context.session_id = session_id;
	context.cmd_id = GLOBAL_CMD_ID_CLOSE_SESSION;
	ret = tc_client_call(&context, dev, session, TC_CALL_GLOBAL | TC_CALL_SYNC);
	kill_ion_by_uuid((teec_uuid *)(context.uuid));
	if (ret)
		tloge("close session failed, ret=0x%x\n", ret);
	return ret;
}

void kill_session(tc_ns_dev_file *dev, unsigned char *uuid,
	unsigned int session_id)
{
	tc_ns_client_context context;
	int ret = 0;
	errno_t sret;

	if (dev == NULL || uuid == NULL)
		return;
	sret = init_context(&context, uuid);
	if (sret != 0)
		return;
	context.session_id = session_id;
	context.cmd_id = GLOBAL_CMD_ID_KILL_TASK;
	tlogd("dev_file_id=%d\n", dev->dev_file_id);
	/* do clear work in agent */
	tee_agent_clear_work(&context, dev->dev_file_id);
	ret = tc_client_call(&context, dev, NULL, TC_CALL_GLOBAL | TC_CALL_SYNC);
	kill_ion_by_uuid((teec_uuid *)context.uuid);
	if (ret)
		tloge("close session failed, ret=0x%x\n", ret);
	return;
}

int tc_ns_service_init(unsigned char *uuid, tc_ns_service **new_service)
{
	int ret = 0;
	tc_ns_service *service = NULL;
	errno_t sret;
	bool check_value = false;

	check_value = (uuid == NULL || new_service == NULL);
	if (check_value == true)
		return -ENOMEM;
	service = kzalloc(sizeof(tc_ns_service), GFP_KERNEL);
	if (service == NULL) {
		tloge("kmalloc failed\n");
		ret = -ENOMEM;
		return ret;
	}
	sret = memcpy_s(service->uuid, sizeof(service->uuid), uuid,
		sizeof(service->uuid));
	if (sret != EOK) {
		kfree(service);
		return -ENOMEM;
	}
	INIT_LIST_HEAD(&service->session_list);
	mutex_init(&service->session_lock);
	list_add_tail(&service->head, &g_service_list);
	tlogd("add service [0x%x] to service list\n", *(uint32_t *)uuid);
	atomic_set(&service->usage, 1);
	mutex_init(&service->operation_lock);
	*new_service = service;
	return ret;
}

uint32_t tc_ns_get_uid(void)
{
	struct task_struct *task = current;
	const struct cred *cred = NULL;
	uint32_t uid = 0;

	cred = get_task_cred(task);
	if (cred == NULL) {
		tloge("failed to get uid of the task\n");
		return (uint32_t)(-1);
	}

#if (KERNEL_VERSION(3, 13, 0) <= LINUX_VERSION_CODE)
	uid = cred->uid.val;
#else
	uid = cred->uid;
#endif
	put_cred(cred);
	tlogd("current uid is %d\n", uid);
	return uid;
}

int tee_init_crypto(char *hash_type)
{
	long rc = 0;

	if (hash_type == NULL) {
		tloge("tee init crypto: error input parameter.\n");
		return -5;
	}
	mutex_lock(&g_tee_crypto_hash_lock);
	if (g_tee_init_crypt_state) {
		mutex_unlock(&g_tee_crypto_hash_lock);
		return 0;
	}
	g_tee_shash_tfm = crypto_alloc_shash(hash_type, 0, 0);
	if (IS_ERR(g_tee_shash_tfm)) {
		rc = PTR_ERR(g_tee_shash_tfm);
		tloge("Can not allocate %s (reason: %ld)\n", hash_type, rc);
		mutex_unlock(&g_tee_crypto_hash_lock);
		return rc;
	}
	g_tee_init_crypt_state = 1;
	mutex_unlock(&g_tee_crypto_hash_lock);
	return 0;
}

int get_pack_name_len(tc_ns_dev_file *dev_file, uint8_t *cert_buffer)
{
	errno_t sret;

	if (dev_file == NULL || cert_buffer == NULL)
		return -ENOMEM;
	sret = memcpy_s(&dev_file->pkg_name_len, sizeof(dev_file->pkg_name_len),
		cert_buffer, sizeof(dev_file->pkg_name_len));
	if (sret != EOK)
		return -ENOMEM;
	tlogd("package_name_len is %u\n", dev_file->pkg_name_len);
	if (dev_file->pkg_name_len == 0 ||
	    dev_file->pkg_name_len >= MAX_PACKAGE_NAME_LEN) {
		tloge("Invalid size of package name len login info!\n");
		return -EINVAL;
	}
	return 0;

}

int get_public_key_len(tc_ns_dev_file *dev_file, uint8_t *cert_buffer)
{
	errno_t sret;

	if (dev_file == NULL || cert_buffer == NULL)
		return -ENOMEM;
	sret = memcpy_s(&dev_file->pub_key_len, sizeof(dev_file->pub_key_len),
		cert_buffer, sizeof(dev_file->pub_key_len));
	if (sret != EOK)
		return -ENOMEM;
	tlogd("publick_key_len is %d\n", dev_file->pub_key_len);
	if (dev_file->pub_key_len > MAX_PUBKEY_LEN) {
		tloge("Invalid public key length in login info!\n");
		return -EINVAL;
	}
	return 0;
}

bool is_valid_ta_size(char *file_buffer, unsigned int file_size)
{
	if (file_buffer == NULL || file_size == 0) {
		tloge("invalid load ta size\n");
		return false;
	}
	if (file_size > SZ_8M) {
		tloge("larger than 8M TA is not supportedi, size=%d\n", file_size);
		return false;
	}
	return true;
}

int tc_ns_need_load_image(unsigned int file_id, unsigned char *uuid)
{
	int ret = 0;
	int smc_ret = 0;
	tc_ns_smc_cmd smc_cmd = {0};
	struct mb_cmd_pack *mb_pack = NULL;
	char *mb_param = NULL;

	if (uuid == NULL) {
		tloge("invalid uuid\n");
		return -ENOMEM;
	}
	mb_pack = mailbox_alloc_cmd_pack();
	if (mb_pack == NULL) {
		tloge("alloc mb pack failed\n");
		return -ENOMEM;
	}
	mb_param = mailbox_copy_alloc((void *)uuid, sizeof(teec_uuid));
	if (mb_param == NULL) {
		tloge("alloc mb param failed\n");
		ret = -ENOMEM;
		goto clean;
	}
	mb_pack->operation.paramtypes = TEEC_MEMREF_TEMP_INOUT;
	mb_pack->operation.params[0].memref.buffer = virt_to_phys((void *)mb_param);
	mb_pack->operation.buffer_h_addr[0] = virt_to_phys((void *)mb_param) >> 32;
	mb_pack->operation.params[0].memref.size = SZ_4K;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_NEED_LOAD_APP;
	mb_pack->uuid[0] = 1;
	smc_cmd.uuid_phys = virt_to_phys((void *)mb_pack->uuid);
	smc_cmd.uuid_h_phys = virt_to_phys((void *)mb_pack->uuid) >> 32;
	smc_cmd.dev_file_id = file_id;
	smc_cmd.context_id = 0;
	smc_cmd.operation_phys = virt_to_phys(&mb_pack->operation);
	smc_cmd.operation_h_phys = virt_to_phys(&mb_pack->operation) >> 32;
	tlogd("secure app load smc command\n");
	smc_ret = tc_ns_smc(&smc_cmd);
	if (smc_ret != 0) {
		tloge("smc_call returns error ret 0x%x\n", smc_ret);
		ret = -1;
		goto clean;
	} else {
		ret = *(int *)mb_param;
	}
clean:
	if (mb_param != NULL)
		mailbox_free(mb_param);
	mailbox_free(mb_pack);

	return ret;
}

int load_ta_image(tc_ns_dev_file *dev_file, tc_ns_client_context *context)
{
	int ret = 0;

	if (dev_file == NULL || context == NULL)
		return -1;
	mutex_lock(&load_app_lock);
	ret = tc_ns_need_load_image(dev_file->dev_file_id, context->uuid);
	if (ret == 1) {
		if (context->file_buffer == NULL) {
			tloge("context's file_buffer is NULL");
			mutex_unlock(&load_app_lock);
			return -1;
		}
		ret = tc_ns_load_image(dev_file, context->file_buffer,
			context->file_size);
		if (ret) {
			tloge("load image failed, ret=%x", ret);
			mutex_unlock(&load_app_lock);
			return ret;
		}
	}
	mutex_unlock(&load_app_lock);
	return ret;
}

void release_free_session(tc_ns_dev_file *dev_file,
	tc_ns_client_context *context, tc_ns_session *session)
{
	int need_kill_session = 0;
	int need_free = 0;

	if (dev_file == NULL || context == NULL || session == NULL)
		return;
	need_kill_session = context->session_id != 0;
	if (need_kill_session)
		kill_session(dev_file, context->uuid, context->session_id);
	need_free = session && session->tc_ns_token.token_buffer;
	if (need_free) {
		kfree(session->tc_ns_token.token_buffer);
		session->tc_ns_token.token_buffer = NULL;
	}
}

void close_session_in_service_list(tc_ns_dev_file *dev, tc_ns_service *service,
	uint32_t i)
{
	tc_ns_session *tmp_session = NULL;
	tc_ns_session *session = NULL;
	errno_t ret_s = 0;
	int ret = 0;

	if (dev == NULL || service == NULL)
		return;
	list_for_each_entry_safe(session, tmp_session,
		&dev->services[i]->session_list, head) {
		if (session->owner != dev)
			continue;
		mutex_lock(&session->ta_session_lock);
		ret = close_session(dev, session, service->uuid, session->session_id);
		mutex_unlock(&session->ta_session_lock);
		if (TEEC_SUCCESS != ret)
			tloge("close session smc(when close fd) failed!\n");
		/* Clean session secure information */
		ret_s = memset_s((void *)&session->secure_info,
			sizeof(session->secure_info),
			0,
			sizeof(session->secure_info));
		if (ret_s != EOK)
			tloge("tc_ns_client_close memset_s error=%d\n", ret_s);
		list_del(&session->head);
		put_session_struct(session); /* pair with open session */
	}
}

void close_unclosed_session(tc_ns_dev_file *dev, uint32_t i)
{
	if (dev == NULL)
		return;
	if (dev->services[i] && !list_empty(&dev->services[i]->session_list)) {
		tc_ns_service *service = dev->services[i];
		/* reorder the oplock and sessin lock to avoid dead lock
		 * during opensession and clientclose
		 */
		mutex_lock(&service->operation_lock);
		mutex_lock(&service->session_lock);
		close_session_in_service_list(dev, service, i);
		mutex_unlock(&service->session_lock);
		mutex_unlock(&service->operation_lock);
		put_service_struct(service); /* pair with open session */
	}
}

void del_dev_node(tc_ns_dev_file *dev)
{
	if (dev == NULL)
		return;
	mutex_lock(&g_tc_ns_dev_list.dev_lock);
	/* del dev from the list */
	list_del(&dev->head);
	mutex_unlock(&g_tc_ns_dev_list.dev_lock);
}


static int param_check(tc_ns_dev_file *dev_file, void *argp)
{
	if (dev_file == NULL) {
		tloge("dev file id erro\n");
		return -EINVAL;
	}
	if (argp == NULL) {
		tloge("argp is NULL input buffer\n");
		return -EINVAL;
	}
	return 0;
}

int tc_ns_tui_event(tc_ns_dev_file *dev_file, void *argp)
{
	int ret = 0;
	teec_tui_parameter tui_param = {0};
	bool check_value = false;

	ret = param_check(dev_file, argp);
	if (ret != 0)
		return ret;
	if (copy_from_user(&tui_param, argp, sizeof(teec_tui_parameter))) {
		tloge("copy from user failed\n");
		ret = -ENOMEM;
		return ret;
	}
	check_value = (tui_param.event_type == TUI_POLL_CANCEL ||
		tui_param.event_type == TUI_POLL_NOTCH ||
		tui_param.event_type == TUI_POLL_FOLD);
	if (check_value == true) {
		ret = tui_send_event(tui_param.event_type, &tui_param);
	} else {
		tloge("no permission to send event\n");
		ret = -1;
	}
	return ret;
}

int ns_client_close_teecd_not_agent(tc_ns_dev_file *dev)
{
	if (dev == NULL) {
		tloge("invalid dev(null)\n");
		return TEEC_ERROR_GENERIC;
	}
	del_dev_node(dev);
	kfree(dev);
	return TEEC_SUCCESS;
}


