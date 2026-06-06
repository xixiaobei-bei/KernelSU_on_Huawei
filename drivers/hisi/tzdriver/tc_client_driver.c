/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: Function definition for proc open,close session and invoke.
 * Author: qiqingchao  q00XXXXXX
 * Create: 2016-06-21
 */
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

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/mm.h>
#include <linux/sched/signal.h>
#endif

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

#define TEEC_PARAM_TYPES(param0_type, param1_type, param2_type, param3_type) \
	((param3_type) << 12 | (param2_type) << 8 | \
	(param1_type) << 4 | (param0_type))

#define TEEC_PARAM_TYPE_GET(param_types, index) \
	(((param_types) >> (4*(index))) & 0x0F)

//global reference start
static dev_t g_tc_ns_client_devt;
static struct class *g_driver_class = NULL;
static struct cdev g_tc_ns_client_cdev;
static struct device_node *g_np = NULL;
static unsigned int g_device_file_cnt = 1;
static struct task_struct *g_teecd_task = NULL;
static DEFINE_MUTEX(g_device_file_cnt_lock);

struct tc_ns_dev_list g_tc_ns_dev_list;

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
struct ion_client *g_drm_ion_client = NULL;
#endif

struct crypto_shash *g_tee_shash_tfm = NULL;
int g_tee_init_crypt_state;
struct mutex g_tee_crypto_hash_lock;
struct list_head g_service_list;
DEFINE_MUTEX(g_service_list_lock);
/* hash code for /vendor/bin/teecd0 */

static unsigned char g_non_hidl_auth_hash[SHA256_DIGEST_LENTH] = {
	0xc5, 0x6e, 0x2b, 0x89,
	0xce, 0x9e, 0xeb, 0x63,
	0xe7, 0x42, 0xfb, 0x2b,
	0x9d, 0x48, 0xff, 0x52,
	0xb2, 0x2f, 0xa7, 0xd5,
	0x87, 0xc6, 0x1f, 0x95,
	0x84, 0x5c, 0x0e, 0x96,
	0x9e, 0x18, 0x81, 0x51,
};
static unsigned char g_hidl_auth_hash[SHA256_DIGEST_LENTH] = {
	0x92, 0xe8, 0x79, 0x91,
	0xc2, 0xad, 0x5c, 0x8a,
	0xfe, 0x86, 0x6f, 0x93,
	0xd5, 0xc5, 0x5a, 0xa8,
	0x8a, 0x60, 0x87, 0x39,
	0x81, 0x50, 0x34, 0x29,
	0x8e, 0x23, 0xb2, 0x50,
	0xd0, 0x36, 0xad, 0xdc,
};

typedef struct {
	tc_ns_dev_file *dev_file;
	char *file_buffer;
	unsigned int file_size;
} load_image_params;

static unsigned char g_teecd_hash[SHA256_DIGEST_LENTH] = {0};
static unsigned char g_hidl_hash[SHA256_DIGEST_LENTH] = {0};
static bool g_teecd_hash_enable = false;
static bool g_hidl_hash_enable = false;

/*
 * Calculate hash of task's text.
 * @cfc_rehash: if generate a random number and hash the resulting hash again.
 * The random number is passed to TEE through CoreSight.
 * For TEECD code hash checking, cfc_rehash should be false.
 */
static int tee_calc_task_hash(unsigned char *digest, bool cfc_rehash,
	struct task_struct *cur_struct);

static int check_teecd_hash(int type)
{
	bool check_value = false;
	unsigned char digest[SHA256_DIGEST_LENTH] = {0};

	check_value = (type != NON_HIDL_SIDE && type != HIDL_SIDE);
	if (check_value == true) {
		tloge("type error! type is %d\n", type);
		return -EFAULT;
	}
	check_value = (g_teecd_hash_enable && type == NON_HIDL_SIDE);
	if (check_value == true) {
		if (tee_calc_task_hash(digest, false, current) ||
			memcmp(digest, g_teecd_hash, SHA256_DIGEST_LENTH)) {
			tloge("compare teecd hash error!\n");
			return CHECK_CODE_HASH_FAIL;
		}
	}
	check_value = (g_hidl_hash_enable && type == HIDL_SIDE);
	if (check_value == true) {
		if (tee_calc_task_hash(digest, false, current) ||
			memcmp(digest, g_hidl_hash, SHA256_DIGEST_LENTH)) {
			tloge("compare libteec hidl serivce hash error!\n");
			return CHECK_CODE_HASH_FAIL;
		}
	}
	return 0;
}

static void free_cred(const struct cred *cred)
{
	if (cred != NULL)
		put_cred(cred);
}

static int get_mem_space(char **ca_cert, char **tpath,
	const struct cred *cred)
{

	*tpath = kmalloc(MAX_PATH_SIZE, GFP_KERNEL);
	if (*tpath == NULL) {
		tloge("tpath kmalloc fail\n");
		free_cred(cred);
		return -EPERM;
	}
	*ca_cert = kmalloc(BUF_MAX_SIZE, GFP_KERNEL);
	if (*ca_cert == NULL) {
		tloge("ca_cert kmalloc fail\n");
		kfree(*tpath);
		free_cred(cred);
		return -EPERM;
	}
	return 0;
}

static int check_path_and_access(struct task_struct *ca_task,
	char *ca_cert, unsigned char *digest, int message_size, int type)
{
	int ret = 0;

	ret = calc_process_path_hash(ca_cert, message_size, digest);
	if (ret != 0)
		return ret;
	if (type == NON_HIDL_SIDE) {
		if (memcmp(digest, g_non_hidl_auth_hash,
			SHA256_DIGEST_LENTH) == 0) {
			if (check_process_selinux_security(
				ca_task, "u:r:tee:s0"))
				tloge("[teecd] check seclabel failed\n");
		} else {
			tloge("[teecd]check process path failed \n");
			return CHECK_PATH_HASH_FAIL;
		}
	} else if (type == HIDL_SIDE) {
		if (memcmp(digest, g_hidl_auth_hash,
			SHA256_DIGEST_LENTH) == 0) {
			if (check_process_selinux_security(
				ca_task, "u:r:hal_libteec_default:s0")) {
				tloge("[libteec hidl service] check seclabel failed\n");
				return CHECK_SECLABEL_FAIL;
			}
		} else {
			tlogd("access process is not libteec hidl service ,please going on\n");
			return ENTER_BYPASS_CHANNEL;
		}
	} else {
		tloge("%d. is invalid type\n", type);
		return INVALID_TYPE;
	}
	ret = check_teecd_hash(type);
	return ret;
}


static int check_process_access(struct task_struct *ca_task, int type)
{
	char *ca_cert = NULL;
	char *path = NULL;
	char *tpath = NULL;
	unsigned char digest[SHA256_DIGEST_LENTH] = {0};
	const struct cred *cred = NULL;
	int message_size;
	int ret = 0;

	if (ca_task == NULL) {
		tloge("task_struct is NULL\n");
		return -EPERM;
	}
	if (!ca_task->mm) {
		tlogd("kernel thread need not check\n");
		ret = ENTER_BYPASS_CHANNEL;
		return ret;
	}
	cred = get_task_cred(ca_task);
	if (cred == NULL) {
		tloge("cred is NULL\n");
		return -EPERM;
	}
	ret = get_mem_space(&ca_cert, &tpath, cred);
	if (ret != 0)
		return ret;
	path = get_process_path(ca_task, tpath);
	if (!IS_ERR_OR_NULL(path)) {
		errno_t sret;

		sret = memset_s(ca_cert, BUF_MAX_SIZE, 0x00, BUF_MAX_SIZE);
		if (sret != EOK) {
			tloge("memset_s error sret is %d\n", sret);
			kfree(tpath);
			kfree(ca_cert);
			free_cred(cred);
			return -EPERM;
		}
		message_size = pack_ca_cert(type, ca_cert, path, ca_task, cred);
		if (message_size > 0)
			ret = check_path_and_access(ca_task, ca_cert,
				digest, message_size, type);
	} else {
		ret = -EPERM;
	}
	kfree(tpath);
	kfree(ca_cert);
	free_cred(cred);
	return ret;
}

static inline int tee_cfc_rehash(struct shash_desc *shash,
	unsigned char *digest)
{
	return 0;
}

static int update_task_hash(struct mm_struct *mm,
	struct task_struct *cur_struct, unsigned char *digest,
	struct shash_desc *shash)
{
	int rc;
	unsigned long start_code, end_code, code_size, in_size;
	struct page *ptr_page = NULL;
	void *ptr_base = NULL;
#if (KERNEL_VERSION(4, 9, 0) > LINUX_VERSION_CODE)
	int locked = 1;
#endif

	start_code = mm->start_code;
	end_code = mm->end_code;
	code_size = end_code - start_code;
	down_read(&mm->mmap_sem);
	while (start_code < end_code) {
		/* Get a handle of the page we want to read */
#if (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE)

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
		rc = get_user_pages_remote(cur_struct, mm, start_code,
			1, 0, &ptr_page, NULL);
#else
		rc = get_user_pages_remote(cur_struct, mm, start_code,
			1, 0, &ptr_page, NULL, NULL);
#endif

#else
		rc = get_user_pages_locked(cur_struct, mm, start_code,
			1, 0, 0, &ptr_page, &locked);
#endif
		if (rc != 1) {
			tloge("get user pages locked error[0x%x]\n", rc);
			rc = -EFAULT;
			break;
		}
		ptr_base = kmap_atomic(ptr_page);
		if (ptr_base == NULL) {
			rc = -3;
			put_page(ptr_page);
			break;
		}
		in_size = (code_size > PAGE_SIZE) ? PAGE_SIZE : code_size;
		rc = crypto_shash_update(shash, ptr_base, in_size);
		if (rc) {
			kunmap_atomic(ptr_base);
			put_page(ptr_page);
			break;
		}
		kunmap_atomic(ptr_base);
		put_page(ptr_page);
		start_code += in_size;
		code_size = end_code - start_code;
	}
	return rc;
}


/* Calculate the SHA256 file digest */
static int tee_calc_task_hash(unsigned char *digest, bool cfc_rehash,
	struct task_struct *cur_struct)
{
	bool check_value = false;
	struct mm_struct *mm = NULL;
	int rc;
	struct {
		struct shash_desc shash;
		char ctx[crypto_shash_descsize(g_tee_shash_tfm)];
	} desc;

	if (digest == NULL) {
		tloge("tee hash: input param is error!\n");
		return -2;
	}
	mm = get_task_mm(cur_struct);
	if (mm == NULL) {
		errno_t sret;

		sret = memset_s(digest, MAX_SHA_256_SZ, 0, MAX_SHA_256_SZ);
		if (sret != EOK)
			return -2;
		if (cfc_is_enabled && cfc_rehash)
			CFC_SEND_DATA(tee_calc_task_hash_fix_val, 0);
		return 0;
	}
	desc.shash.tfm = g_tee_shash_tfm;
	desc.shash.flags = 0;
	rc = crypto_shash_init(&desc.shash);
	if (rc != 0)
		return rc;
	rc = update_task_hash(mm, cur_struct, digest, &desc.shash);
	up_read(&mm->mmap_sem);
	mmput(mm);
	if (!rc) {
		rc = crypto_shash_final(&desc.shash, digest);
		check_value = rc || !cfc_is_enabled || !cfc_rehash;
		if (check_value == true)
			return rc;
	}
	return rc;
}

/* Modify the client context so params id 2 and 3 contain temp pointers to the
 * public key and package name for the open session. This is used for the
 * TEEC_LOGIN_IDENTIFY open session method
 */
static int set_login_information(tc_ns_dev_file *dev_file,
	tc_ns_client_context *context)
{
	/* The daemon has failed to get login information or not supplied */
	if (dev_file->pkg_name_len == 0)
		return -1;
	/* The 3rd parameter buffer points to the pkg name buffer in the
	 * device file pointer
	 * get package name len and package name
	 */
	context->params[3].memref.size_addr = (__u64)&dev_file->pkg_name_len;
	context->params[3].memref.buffer = (__u64)dev_file->pkg_name;
	/* Set public key len and public key */
	if (dev_file->pub_key_len != 0) {
		context->params[2].memref.size_addr =
			(__u64)&dev_file->pub_key_len;
		context->params[2].memref.buffer =
			(__u64)dev_file->pub_key;
	} else {
		/* If get public key failed, then get uid in kernel */
		uint32_t ca_uid = tc_ns_get_uid();

		if (ca_uid == -1) {
			tloge("Failed to get uid of the task\n");
			goto error;
		}
		dev_file->pub_key_len = sizeof(ca_uid);
		context->params[2].memref.size_addr =
			(__u64)&dev_file->pub_key_len;
		if (memcpy_s(dev_file->pub_key, MAX_PUBKEY_LEN, &ca_uid,
			dev_file->pub_key_len)) {
			tloge("Failed to copy pubkey, pub_key_len=%u\n",
			      dev_file->pub_key_len);
			goto error;
		}
		context->params[2].memref.buffer = (__u64) dev_file->pub_key;
	}
	/* Now we mark the 2 parameters as input temp buffers */
	context->param_types = TEEC_PARAM_TYPES(
		TEEC_PARAM_TYPE_GET(context->param_types, 0),
		TEEC_PARAM_TYPE_GET(context->param_types, 1),
		TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT);
	return 0;
error:
	return -1;
}

static int check_process_and_params(tc_ns_dev_file *dev_file,
	uint8_t **buf, uint8_t **cert_buffer, int *cert_buffer_size)
{
	int ret = -EINVAL;

	ret = check_process_access(current, NON_HIDL_SIDE);
	if (ret) {
		tloge(KERN_ERR "tc client login: teecd verification failed ret 0x%x!\n", ret);
		return -EPERM;
	}
	if (dev_file->login_setup) {
		tloge("Login information cannot be set twice!\n");
		return -EINVAL;
	}
	*cert_buffer_size = (MAX_PACKAGE_NAME_LEN + MAX_PUBKEY_LEN +
		sizeof(dev_file->pkg_name_len) + sizeof(dev_file->pub_key_len));
	*buf = *cert_buffer = kmalloc(*cert_buffer_size, GFP_KERNEL);
	if (*cert_buffer == NULL) {
		tloge("Failed to allocate login buffer!");
		return -EFAULT;
	}

	return 0;
}

static int tc_ns_client_login_func(tc_ns_dev_file *dev_file,
	void __user *buffer)
{
	int ret = -EINVAL;
	uint8_t *cert_buffer = NULL;
	uint8_t *buf = NULL;
	errno_t sret;
	int cert_buffer_size = 0;

	ret = check_process_and_params(dev_file, &buf, &cert_buffer, &cert_buffer_size);
	if (ret != 0)
		return ret;
	dev_file->login_setup = true;
	if (buffer == NULL) {
		/* We accept no debug information because the daemon might  have failed */
		tlogd("No debug information\n");
		dev_file->pkg_name_len = 0;
		dev_file->pub_key_len = 0;
		return 0;
	}
	/* GET PACKAGE NAME AND APP CERTIFICATE:
	 * The proc_info format is as follows:
	 * package_name_len(4 bytes) || package_name ||
	 * apk_cert_len(4 bytes) || apk_cert.
	 * or package_name_len(4 bytes) || package_name
	 * || exe_uid_len(4 bytes) || exe_uid.
	 * The apk certificate format is as follows:
	 * modulus_size(4bytes) ||modulus buffer
	 * || exponent size || exponent buffer
	 */
	if (copy_from_user(cert_buffer, buffer, cert_buffer_size)) {
		tloge("Failed to get user login info!\n");
		ret = -EINVAL;
		goto error;
	}
	/* get package name len */
	ret = get_pack_name_len(dev_file, cert_buffer);
	if (ret != 0)
		goto error;
	cert_buffer += sizeof(dev_file->pkg_name_len);
	/* get package name */
	sret = strncpy_s(dev_file->pkg_name, MAX_PACKAGE_NAME_LEN, cert_buffer,
		dev_file->pkg_name_len);
	if (sret != EOK) {
		ret = -ENOMEM;
		goto error;
	}
	tlogd("package name is %s\n", dev_file->pkg_name);
	cert_buffer += dev_file->pkg_name_len;
	/* get public key len */
	ret = get_public_key_len(dev_file, cert_buffer);
	if (ret != 0)
		goto error;
	/* get public key */
	if (dev_file->pub_key_len != 0) {
		cert_buffer += sizeof(dev_file->pub_key_len);
		if (memcpy_s(dev_file->pub_key, MAX_PUBKEY_LEN, cert_buffer,
			dev_file->pub_key_len)) {
			tloge("Failed to copy cert, pub_key_len=%u\n",
				dev_file->pub_key_len);
			ret = -EINVAL;
			goto error;
		}
		cert_buffer += dev_file->pub_key_len;
	}
	ret = 0;
error:
	kfree(buf);
	return ret;
}

static int alloc_for_load_image(unsigned int *mb_load_size,
	unsigned int file_size, char **mb_load_mem,
	struct mb_cmd_pack **mb_pack, teec_uuid **uuid_return)
{
	/* we will try any possible to alloc mailbox mem to load TA */
	for (; *mb_load_size > 0; *mb_load_size >>= 1) {
		*mb_load_mem = mailbox_alloc(*mb_load_size, 0);
		if (*mb_load_mem != NULL) {
			break;
		} else {
			tlogw("alloc mem(size=%d) for TA load mem fail, will retry\n", *mb_load_size);
		}
	}
	if (*mb_load_mem == NULL) {
		tloge("alloc TA load mem failed\n");
		return -ENOMEM;
	}
	*mb_pack = mailbox_alloc_cmd_pack();
	if (*mb_pack == NULL) {
		mailbox_free(*mb_load_mem);
		tloge("alloc mb pack failed\n");
		return -ENOMEM;
	}
	*uuid_return = mailbox_alloc(sizeof(teec_uuid), 0);
	if (*uuid_return == NULL) {
		mailbox_free(*mb_load_mem);
		mailbox_free(*mb_pack);
		tloge("alloc uuid failed\n");
		return -ENOMEM;
	}
	return 0;
}

static void pack_data_for_smc_cmd(uint32_t load_size, char *mb_load_mem,
	struct mb_cmd_pack *mb_pack, teec_uuid *uuid_return,
	tc_ns_smc_cmd *smc_cmd)
{
	mb_pack->operation.params[0].memref.buffer =
		virt_to_phys((void *)mb_load_mem);
	mb_pack->operation.buffer_h_addr[0] =
		virt_to_phys((void *)mb_load_mem) >> 32;
	mb_pack->operation.params[0].memref.size = load_size + sizeof(int);
	mb_pack->operation.params[2].memref.buffer =
		virt_to_phys((void *)uuid_return);
	mb_pack->operation.buffer_h_addr[2] =
		virt_to_phys((void *)uuid_return) >> 32;
	mb_pack->operation.params[2].memref.size = sizeof(*uuid_return);
	mb_pack->operation.paramtypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT,
		TEEC_VALUE_OUTPUT, TEEC_MEMREF_TEMP_OUTPUT, TEEC_VALUE_INPUT);
	mb_pack->uuid[0] = 1;
	/* load image smc command */
	smc_cmd->cmd_id = GLOBAL_CMD_ID_LOAD_SECURE_APP;
	smc_cmd->uuid_phys = virt_to_phys((void *)mb_pack->uuid);
	smc_cmd->uuid_h_phys = virt_to_phys((void *)mb_pack->uuid) >> 32;
	smc_cmd->context_id = 0;
	smc_cmd->operation_phys = virt_to_phys(&mb_pack->operation);
	smc_cmd->operation_h_phys =
		virt_to_phys(&mb_pack->operation) >> 32;
}

static int load_image_for_ion(struct mb_cmd_pack *mb_pack,
	load_image_params params_in, teec_uuid *uuid_return)
{
	int ret = 0;
	/* check need to add ionmem  */
	uint32_t configid = mb_pack->operation.params[1].value.a;
	uint32_t ion_size = mb_pack->operation.params[1].value.b;
	int32_t check_result = (configid != 0 && ion_size != 0);

	tloge("check load by ION result %d : configid =%d,ion_size =%d,uuid=%x\n",
		check_result, configid, ion_size, uuid_return->time_low);
	if (check_result) {
		ret = load_app_use_configid(configid, params_in.dev_file->dev_file_id,
			uuid_return, ion_size);
		if (ret != 0) {
			tloge("load_app_use_configid failed ret =%d\n", ret);
			return -1;
		}
	}
	return ret;
}

static int load_image_by_frame(load_image_params params_in,
	unsigned int mb_load_size, char *mb_load_mem,
	struct mb_cmd_pack *mb_pack, teec_uuid *uuid_return,
	unsigned int load_times)
{
	char *p = mb_load_mem;
	uint32_t load_size;
	int load_flag = 1; /* 0:it's last block, 1:not last block */
	uint32_t loaded_size = 0;
	unsigned int index;
	tc_ns_smc_cmd smc_cmd = {0};
	int ret = 0;
	int smc_ret = 0;

	for (index = 0; index < load_times; index++) {
		if (index == (load_times - 1)) {
			load_flag = 0;
			load_size = params_in.file_size - loaded_size;
		} else {
			load_size = mb_load_size - sizeof(load_flag);
		}
		*(int *)p = load_flag;
		if (load_size > mb_load_size - sizeof(load_flag)) {
			tloge("invalid load size %d/%d\n", load_size,
				mb_load_size);
			return  -1;
		}
		if (copy_from_user(mb_load_mem + sizeof(load_flag),
			(void __user *)params_in.file_buffer +
			loaded_size, load_size)) {
			tloge("file buf get fail\n");
			return  -1;
		}
		pack_data_for_smc_cmd(load_size, mb_load_mem, mb_pack,
			uuid_return, &smc_cmd);
		mb_pack->operation.params[3].value.a = index;
		smc_cmd.dev_file_id = params_in.dev_file->dev_file_id;
		smc_ret = tc_ns_smc(&smc_cmd);
		tlogd("smc cmd ret %d\n", smc_ret);
		tlogd("configid=%d, ret=%d, load_flag=%d, index=%d\n",
			mb_pack->operation.params[1].value.a, smc_ret,
			load_flag, index);

		if (smc_ret != 0) {
			tloge("smc_call returns error ret 0x%x\n", smc_ret);
			return -1;
		}
		if (smc_ret == 0 && load_flag == 0) {
			ret = load_image_for_ion(mb_pack, params_in,
				uuid_return);
			if (ret != 0)
				return -1;
		}
		loaded_size += load_size;
	}
	return 0;
}

int tc_ns_load_image(tc_ns_dev_file *dev_file, char *file_buffer,
	unsigned int file_size)
{
	int ret = 0;
	struct mb_cmd_pack *mb_pack = NULL;
	unsigned int mb_load_size;
	char *mb_load_mem = NULL;
	teec_uuid *uuid_return = NULL;
	unsigned int load_times;
	load_image_params params_in = {dev_file, file_buffer, file_size};
	bool check_value = false;

	check_value = (dev_file == NULL || file_buffer == NULL);
	if (check_value) {
		tloge("dev_file or file_buffer is NULL!\n");
		return -EINVAL;
	}
	if (!is_valid_ta_size(file_buffer, file_size))
		return -EINVAL;
	mb_load_size = file_size > (SZ_1M - sizeof(int)) ?
		SZ_1M : ALIGN(file_size, SZ_4K);
	ret = alloc_for_load_image(&mb_load_size, file_size, &mb_load_mem,
		&mb_pack, &uuid_return);
	if (ret != 0)
		return ret;
	load_times = file_size / (mb_load_size - sizeof(int));
	if (file_size % (mb_load_size - sizeof(int)))
		load_times += 1;
	ret = load_image_by_frame(params_in, mb_load_size, mb_load_mem,
		mb_pack, uuid_return, load_times);
	mailbox_free(mb_load_mem);
	mailbox_free(mb_pack);
	mailbox_free(uuid_return);
	return ret;
}

static int check_task_state(bool hidl_access, struct task_struct **hidl_struct,
	tc_ns_client_context *context)
{
	bool check_value = false;

	if (hidl_access) {
		rcu_read_lock();
		*hidl_struct = pid_task(find_vpid(context->callingPid),
			PIDTYPE_PID);
		check_value = *hidl_struct == NULL ||
			(*hidl_struct)->state == TASK_DEAD;
		if (check_value == true) {
			tloge("task is dead!\n");
			rcu_read_unlock();
			return -EFAULT;
		}
		get_task_struct(*hidl_struct);
		rcu_read_unlock();
	} else {
		check_value = (context->callingPid && current->mm);
		if (check_value == true) {
			tloge("non hidl service pass non-zero callingpid , reject please!!!\n");
			return -EFAULT;
		}
	}
	return 0;
}

static int calc_task_hash(tc_ns_session *session, tc_ns_service *service,
	struct task_struct *cur_struct)
{
	mutex_lock(&g_tee_crypto_hash_lock);
	if (tee_calc_task_hash(session->ca_auth_hash_buf, true, cur_struct)) {
		mutex_unlock(&g_tee_crypto_hash_lock);
		tloge("tee calc task hash failed\n");
		return -EFAULT;
	}
	mutex_unlock(&g_tee_crypto_hash_lock);
	return 0;
}

static int check_hidl_access(bool hidl_access)
{
	if (hidl_access) {
		if (!g_hidl_hash_enable) {
			if (memset_s((void *)g_hidl_hash,
				sizeof(g_hidl_hash), 0x00, sizeof(g_hidl_hash))) {
				tloge("tc_client_open memset failed!\n");
				return -EFAULT;
			}
			g_hidl_hash_enable =
				(current->mm &&
				!tee_calc_task_hash(g_hidl_hash, false, current));
			if (!g_hidl_hash_enable) {
				tloge("calc libteec hidl hash failed\n");
				return -EFAULT;
			}
		}
	}
	return 0;
}

static int check_login_method(tc_ns_dev_file *dev_file,
	tc_ns_client_context *context, uint8_t *flags)
{
	bool check_value = false;
	int ret = 0;

	if (dev_file == NULL || context == NULL || flags == NULL)
		return -EFAULT;
	if (context->login.method == TEEC_LOGIN_IDENTIFY) {
		tlogd("login method is IDENTIFY\n");
		/* Check if params 0 and 1 are valid */
		check_value = dev_file->kernel_api == TEE_REQ_FROM_USER_MODE &&
			(tc_user_param_valid(context, 0) ||
			tc_user_param_valid(context, 1));
		if (check_value == true)
			return -EFAULT;
		ret = set_login_information(dev_file, context);
		if (ret != 0) {
			tloge("set_login_information failed ret =%d\n", ret);
			return ret;
		}
		*flags |= TC_CALL_LOGIN;
	} else {
		tlogd("login method is not supported\n");
		return -EINVAL;
	}
	return 0;
}

static tc_ns_service *find_service(tc_ns_dev_file *dev_file,
	tc_ns_client_context *context)
{
	int ret = -EINVAL;
	tc_ns_service *service = NULL;

	mutex_lock(&dev_file->service_lock);
	service = tc_find_service_in_dev(dev_file, context->uuid, true);
	mutex_unlock(&dev_file->service_lock);
	/* if service has been opened in this dev */
	if (service != NULL) {
		/* If service has been reference by this dev, tc_find_service_in_dev
		 * will increase a ref count to declaim there's how many callers to
		 * this service from the dev_file, instead of increase service->usage.
		 * While close session, dev->service_ref[i] will decrease and till
		 * it get to 0 put_service_struct will be called.
		 */
		return service;
	}
	mutex_lock(&g_service_list_lock);
	service = tc_find_service_from_all(context->uuid);
	/* if service has been opened in other dev */
	if (service != NULL) {
		get_service_struct(service);
		mutex_unlock(&g_service_list_lock);
		goto add_service;
	}
	/* Create a new service if we couldn't find it in list */
	ret = tc_ns_service_init(context->uuid, &service);
	/* Put unlock after tc_ns_serviceInit to make sure tc_find_service_from_all * is correct */
	mutex_unlock(&g_service_list_lock);
	if (ret) {
		tloge("service init failed");
		return NULL;
	}
add_service:
	mutex_lock(&dev_file->service_lock);
	ret = add_service_to_dev(dev_file, service);
	mutex_unlock(&dev_file->service_lock);
	if (ret) {
		tloge("fail to add service to dev\n");
		return NULL;
	}
	return service;
}

static int proc_calc_task_hash(tc_ns_dev_file *dev_file,
	tc_ns_client_context *context, tc_ns_session *session,
	tc_ns_service *service, uint8_t *flags, bool hidl_access)
{
	int ret = 0;
	struct task_struct *cur_struct = NULL;
	struct task_struct *hidl_struct = NULL;

	ret = check_login_method(dev_file, context, flags);
	if (ret != 0)
		return ret;
	context->cmd_id = GLOBAL_CMD_ID_OPEN_SESSION;
	mutex_init(&session->ta_session_lock);
	if (tee_init_crypto("sha256")) {
		tloge("init code hash error!!!\n");
		return -EFAULT;
	}
	ret = check_task_state(hidl_access, &hidl_struct, context);
	if (ret != 0)
		return ret;
	if (hidl_struct != NULL)
		cur_struct = hidl_struct;
	else
		cur_struct = current;
	/* lock reason:
	 * tee_calc_task_hash will use the global value g_tee_shash_tfm
	 */
	ret = calc_task_hash(session, service, cur_struct);
	if (hidl_struct != NULL)
		put_task_struct(hidl_struct);
	return ret;
}

static void proc_after_smc_cmd(tc_ns_dev_file *dev_file,
	tc_ns_client_context *context, tc_ns_service *service,
	tc_ns_session *session)
{
	session->session_id = context->session_id;
	session->wait_data.send_wait_flag = 0;
	init_waitqueue_head(&session->wait_data.send_cmd_wq);
	atomic_set(&session->usage, 1);
	session->owner = dev_file;
	mutex_lock(&service->session_lock);
	list_add_tail(&session->head, &service->session_list);
	mutex_unlock(&service->session_lock);
}


static int proc_send_smc_cmd(tc_ns_dev_file *dev_file,
	tc_ns_client_context *context, tc_ns_service *service,
	tc_ns_session *session, uint8_t flags)
{
	int ret = 0;

	mutex_lock(&service->operation_lock);
	ret = load_ta_image(dev_file, context);
	if (ret != 0) {
		mutex_unlock(&service->operation_lock);
		return ret;
	}
	/* send smc */
	ret = get_session_secure_params(dev_file, context, session);
	if (ret) {
		tloge("Get session secure parameters failed, ret = %d.\n", ret);
		/* Clean this session secure information */
		clean_session_secure_information(session);
		mutex_unlock(&service->operation_lock);
		return ret;
	}
	session->tc_ns_token.token_buffer =
		kzalloc(TOKEN_BUFFER_LEN, GFP_KERNEL);
	if (session->tc_ns_token.token_buffer == NULL) {
		tloge("kzalloc %d bytes token failed.\n", TOKEN_BUFFER_LEN);
		/* Clean this session secure information */
		clean_session_secure_information(session);
		mutex_unlock(&service->operation_lock);
		return -ENOMEM;
	}
	ret = tc_client_call(context, dev_file, session, flags);
	if (ret != 0) {
		/* Clean this session secure information */
		clean_session_secure_information(session);
	}
	if (ret != 0) {
		kill_ion_by_uuid((teec_uuid *)context->uuid);
		mutex_unlock(&service->operation_lock);
		tloge("smc_call returns error, ret=0x%x\n", ret);
		return ret;
	}
	proc_after_smc_cmd(dev_file, context, service, session);
	/* session_id in tee is unique, but in concurrency scene
	 * same session_id may appear in tzdriver, put session_list
	 * add/del in service->operation_lock can avoid it.
	 */
	mutex_unlock(&service->operation_lock);
	return ret;
}

static void proc_error_situation(tc_ns_dev_file *dev_file,
	tc_ns_client_context *context, tc_ns_service *service,
	tc_ns_session *session)
{
	mutex_lock(&service->session_lock);
	release_free_session(dev_file, context, session);
	mutex_unlock(&service->session_lock);

	mutex_lock(&dev_file->service_lock);
	del_service_from_dev(dev_file, service);
	mutex_unlock(&dev_file->service_lock);
	kfree(session);
	return;
}

int tc_ns_open_session(tc_ns_dev_file *dev_file,
	tc_ns_client_context *context)
{
	int ret = -EINVAL;
	tc_ns_service *service = NULL;
	tc_ns_session *session = NULL;
	uint8_t flags = TC_CALL_GLOBAL;
	bool hidl_access = false;
	bool check_value = (dev_file == NULL || context == NULL);

	CFC_FUNC_ENTRY(tc_ns_open_session);
	if (check_value == true) {
		tloge("invalid dev_file or context\n");
		return -EINVAL;
	}
	ret = check_process_access(current, HIDL_SIDE);
	if (!ret) {
		hidl_access = true;
	} else if (ret && ret != ENTER_BYPASS_CHANNEL) {
		tloge("libteec hidl service may be exploited ret 0x%x\n", ret);
		return -EPERM;
	}
	ret = check_hidl_access(hidl_access);
	if (ret != 0)
		return ret;
	service = find_service(dev_file, context);
	if (service == NULL) {
		tloge("find service failed\n");
		return -ENOMEM;
	}
	session = kzalloc(sizeof(tc_ns_session), GFP_KERNEL);
	if (session == NULL) {
		tloge("kmalloc failed\n");
		del_service_from_dev(dev_file, service);
		return -ENOMEM;
	}
	ret = proc_calc_task_hash(dev_file, context, session, service,
		&flags, hidl_access);
	if (ret != 0)
		goto error;
	ret = proc_send_smc_cmd(dev_file, context, service, session, flags);
	if (ret == 0)
		return ret;
error:
	proc_error_situation(dev_file, context, service, session);
	return ret;
}

static tc_ns_service *get_service(tc_ns_dev_file *dev_file,
	tc_ns_client_context *context)
{
	tc_ns_service *service = NULL;

	mutex_lock(&dev_file->service_lock);
	service = tc_find_service_in_dev(dev_file, context->uuid, false);
	get_service_struct(service);
	mutex_unlock(&dev_file->service_lock);
	return service;
}

static tc_ns_session *get_session(tc_ns_service *service,
	tc_ns_dev_file *dev_file, tc_ns_client_context *context)
{
	tc_ns_session *session = NULL;

	mutex_lock(&service->session_lock);
	session = tc_find_session_withowner(&service->session_list,
		context->session_id, dev_file);
	get_session_struct(session);
	mutex_unlock(&service->session_lock);
	return session;
}

static tc_ns_session *get_and_del_session(tc_ns_service *service,
	tc_ns_dev_file *dev_file, tc_ns_client_context *context)
{
	tc_ns_session *session = NULL;

	mutex_lock(&service->session_lock);
	session = tc_find_session_withowner(&service->session_list,
		context->session_id, dev_file);
	if (session != NULL)
		list_del(&session->head);
	get_session_struct(session);
	mutex_unlock(&service->session_lock);
	return session;
}

int tc_ns_close_session(tc_ns_dev_file *dev_file,
	tc_ns_client_context *context)
{
	int ret = -EINVAL;
	errno_t ret_s = 0;
	tc_ns_service *service = NULL;
	tc_ns_session *session = NULL;

	if (dev_file == NULL || context == NULL) {
		tloge("invalid dev_file or context\n");
		return ret;
	}
	service = get_service(dev_file, context);
	if (service == NULL) {
		tloge("invalid service\n");
		return ret;
	}
	session = get_and_del_session(service, dev_file, context);
	if (session != NULL) {
		int ret2;

		mutex_lock(&service->operation_lock);
		mutex_lock(&session->ta_session_lock);
		ret2 = close_session(dev_file, session,
			context->uuid, context->session_id);
		mutex_unlock(&session->ta_session_lock);
		mutex_unlock(&service->operation_lock);
		if (ret2 != TEEC_SUCCESS)
			tloge("close session smc failed!\n");
		/* Clean this session secure information */
		ret_s = memset_s((void *)&session->secure_info,
			sizeof(session->secure_info),
			0, sizeof(session->secure_info));
		if (ret_s != EOK)
			tloge("tc_ns_close_session memset_s error=%d\n", ret_s);
		put_session_struct(session);
		put_session_struct(session); /* pair with open session */
		ret = TEEC_SUCCESS;
		mutex_lock(&dev_file->service_lock);
		del_service_from_dev(dev_file, service);
		mutex_unlock(&dev_file->service_lock);
	} else {
		tloge("invalid session\n");
	}
	put_service_struct(service);
	return ret;
}

int tc_ns_send_cmd(tc_ns_dev_file *dev_file,
	tc_ns_client_context *context)
{
	int ret = -EINVAL;
	tc_ns_service *service = NULL;
	tc_ns_session *session = NULL;
	bool check_value = (dev_file == NULL || context == NULL);

	if (check_value == true) {
		tloge("invalid dev_file or context\n");
		return ret;
	}
	tlogd("session id :%x\n", context->session_id);
	service = get_service(dev_file, context);
	/* check sessionid is validated or not */
	if (service != NULL) {
		session = get_session(service, dev_file, context);
		put_service_struct(service);
		if (session != NULL) {
			tlogd("send cmd find session id %x\n",
				context->session_id);
			goto find_session;
		}
	} else {
		tloge("can't find service\n");
	}
	tloge("send cmd can not find session id %d\n", context->session_id);
	return ret;
find_session:
	/* send smc */
	mutex_lock(&session->ta_session_lock);
	ret = tc_client_call(context, dev_file, session, 0);
	mutex_unlock(&session->ta_session_lock);
	put_session_struct(session);
	if (ret != 0)
		tloge("smc_call returns error, ret=0x%x\n", ret);
	return ret;
}

int tc_ns_client_open(tc_ns_dev_file **dev_file, uint8_t kernel_api)
{
	int ret = TEEC_ERROR_GENERIC;
	tc_ns_dev_file *dev = NULL;

	tlogd("tc_client_open\n");
	if (dev_file == NULL) {
		tloge("dev_file is NULL");
		return -EFAULT;
	}
	dev = kzalloc(sizeof(tc_ns_dev_file), GFP_KERNEL);
	if (dev == NULL) {
		tloge("dev malloc failed");
		return ret;
	}
	mutex_lock(&g_tc_ns_dev_list.dev_lock);
	list_add_tail(&dev->head, &g_tc_ns_dev_list.dev_file_list);
	mutex_unlock(&g_tc_ns_dev_list.dev_lock);
	mutex_lock(&g_device_file_cnt_lock);
	dev->dev_file_id = g_device_file_cnt;
	g_device_file_cnt++;
	mutex_unlock(&g_device_file_cnt_lock);
	INIT_LIST_HEAD(&dev->shared_mem_list);
	dev->login_setup = 0;
	dev->kernel_api = kernel_api;
	dev->load_app_flag = 0;
	mutex_init(&dev->service_lock);
	mutex_init(&dev->shared_mem_lock);
	*dev_file = dev;
	ret = TEEC_SUCCESS;
	return ret;
}

int tc_ns_client_close(tc_ns_dev_file *dev, int flag)
{
	int ret = TEEC_ERROR_GENERIC;

	if (dev == NULL) {
		tloge("invalid dev(null)\n");
		return ret;
	}
	if (flag == 0) {
		uint32_t i = 0;
		mutex_lock(&dev->service_lock);
		for (i = 0; i < SERVICES_MAX_COUNT; i++) {
			/* close unclosed session */
			close_unclosed_session(dev, i);
		}
		mutex_unlock(&dev->service_lock);
		if (dev->dev_file_id == tui_attach_device()) {
			do_ns_tui_release();
			free_tui_caller_info();
		}
		kill_ion_by_cafd(dev->dev_file_id);
	}
	if (!flag)
		tc_ns_unregister_agent_client(dev);
	del_dev_node(dev);
	ret = TEEC_SUCCESS;
	tee_agent_clear_dev_owner(dev);
	kfree(dev);
	return ret;
}

void shared_vma_open(struct vm_area_struct *vma)
{
	(void)vma;
}

static void release_vma_shared_mem(tc_ns_dev_file *dev_file,
	struct vm_area_struct *vma)
{
	tc_ns_shared_mem *shared_mem = NULL;
	tc_ns_shared_mem *shared_mem_temp = NULL;
	bool find = false;

	mutex_lock(&dev_file->shared_mem_lock);
	list_for_each_entry_safe(shared_mem, shared_mem_temp,
			&dev_file->shared_mem_list, head) {
		if (shared_mem != NULL) {
			if (shared_mem->user_addr ==
				(void *)(uintptr_t)vma->vm_start) {
				shared_mem->user_addr = NULL;
				find = true;
			} else if (shared_mem->user_addr_ca ==
				(void *)(uintptr_t)vma->vm_start) {
				shared_mem->user_addr_ca = NULL;
				find = true;
			}

			if ((shared_mem->user_addr == NULL) &&
				(shared_mem->user_addr_ca == NULL))
				list_del(&shared_mem->head);
			/* pair with tc_client_mmap */
			if (find == true) {
				put_sharemem_struct(shared_mem);
				break;
			}
		}
	}
	mutex_unlock(&dev_file->shared_mem_lock);
}

void shared_vma_close(struct vm_area_struct *vma)
{
	tc_ns_dev_file *dev_file = NULL;
	bool check_value = false;

	if (vma == NULL) {
		tloge("vma is null\n");
		return;
	}
	dev_file = vma->vm_private_data;
	if (dev_file == NULL) {
		tloge("vma->vm_private_data is null\n");
		return;
	}
	check_value = (g_teecd_task == current->group_leader) && (!tc_ns_get_uid()) &&
		(g_teecd_task->flags & PF_EXITING || current->flags & PF_EXITING);
	if (check_value == true) {
		tlogd("teecd is killed, just return in vma close\n");
		return;
	}
	release_vma_shared_mem(dev_file, vma);
}

static struct vm_operations_struct shared_remap_vm_ops = {
	.open = shared_vma_open,
	.close = shared_vma_close,
};

static struct smc_event_data *find_event_control_from_vma_pgoff(
	unsigned long vm_pgoff)
{
	struct smc_event_data *event_control = NULL;

	if (vm_pgoff == 1) {
		event_control = find_event_control(AGENT_FS_ID);
	} else if (vm_pgoff  == 2) {
		event_control = find_event_control(AGENT_MISC_ID);
	} else if (vm_pgoff == 3) {
		event_control = find_event_control(AGENT_SOCKET_ID);
	}
	return event_control;
}

static void check_is_teecd(struct vm_area_struct *vma,
	struct smc_event_data **event_control,
	tc_ns_shared_mem **shared_mem, bool *is_teecd)
{
	bool check_value = false;

	check_value = (g_teecd_task == current->group_leader) &&
		(!tc_ns_get_uid());
	if (check_value == true) {
		*event_control =
			find_event_control_from_vma_pgoff(vma->vm_pgoff);
		if (*event_control != NULL)
			*shared_mem = (*event_control)->buffer;
		*is_teecd = true;
	}
	/* for thirdparty ca agent,we just set is_teecd */
	if (!check_ext_agent_access(current))
		*is_teecd = true;
	return;
}

static void alloc_shared_mem_for_ca_and_agent(
	struct vm_area_struct *vma, tc_ns_shared_mem **shared_mem,
	tc_ns_dev_file *dev_file, bool *only_remap, bool is_teecd)
{
	bool check_value = false;
	tc_ns_shared_mem *shm_tmp = NULL;
	unsigned long len = vma->vm_end - vma->vm_start;

	if (!is_teecd) {
		/* using vma->vm_pgoff as share_mem index */
		/* check if aready allocated */
		list_for_each_entry(shm_tmp, &dev_file->shared_mem_list, head) {
			if (atomic_read(&shm_tmp->offset) == vma->vm_pgoff) {
				tlogd("share_mem already allocated, shm_tmp->offset=%d\n",
					atomic_read(&shm_tmp->offset));
				/* if this shared mem is already maped, just let shared_mem be NULL*/
				if (shm_tmp->user_addr_ca != NULL) {
					*only_remap = true;
					break;
				}
				/* do remap to client */
				*only_remap = true;
				*shared_mem = shm_tmp;
				get_sharemem_struct(shm_tmp);
				break;
			}
		}
	}
	check_value = (*shared_mem == NULL && !(*only_remap));
	if (check_value == true) {
		/* alloc mem for ca and third-party agent(not event_control) */
		*shared_mem = tc_mem_allocate(len, is_teecd);
	}
	return;
}

static int remap_shared_mem(struct vm_area_struct *vma,
	struct smc_event_data *event_control,
	tc_ns_shared_mem *shared_mem, bool only_remap)
{
	int ret = 0;
	unsigned long pfn;

	if (shared_mem->from_mailbox) {
		pfn = virt_to_phys(shared_mem->kernel_addr) >> PAGE_SHIFT;
		if (!valid_mmap_phys_addr_range(pfn,
			(unsigned long)shared_mem->len)) {
			/* for teecd, no need to free mem allocated form mailbox  */
			if (event_control != NULL) {
				put_agent_event(event_control);
				return -1;
			}
			tc_mem_free(shared_mem);
			return -1;
		}
		ret = remap_pfn_range(vma, vma->vm_start,
			virt_to_phys(shared_mem->kernel_addr) >> PAGE_SHIFT,
			(unsigned long)shared_mem->len, vma->vm_page_prot);
	} else {
		if (only_remap)
			vma->vm_flags |= VM_USERMAP;
		ret = remap_vmalloc_range(vma, shared_mem->kernel_addr, 0);
	}
	if (ret) {
		tloge("can't remap %s to user, ret = %d\n",
		      shared_mem->from_mailbox ? "pfn" : "vmalloc", ret);
		if (event_control != NULL) {
			/* for teecd, no need to free mem allocated form mailbox  */
			put_agent_event(event_control);
			return -1;
		}
		if (!only_remap)
			tc_mem_free(shared_mem);
		return -1;
	}
	return ret;
}

static int tc_client_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret = 0;
	tc_ns_dev_file *dev_file = NULL;
	tc_ns_shared_mem *shared_mem = NULL;
	struct smc_event_data *event_control = NULL;
	bool is_teecd = false;
	bool only_remap = false;
	bool check_value = false;

	if (filp == NULL || vma == NULL) {
		tloge("filp or vma is null\n");
		return -1;
	}
	dev_file = filp->private_data;
	if (dev_file == NULL) {
		tloge("can not find dev in malloc shared buffer!\n");
		return -1;
	}
	/* in this func, we need to deal with five cases:
	 * teecd (alloc and remap);  third-party ca(alloc and remap);
	 * vendor CA alloc sharedmem (alloc and remap);
	 * HIDL alloc sharedmem (alloc and remap);
	 * system CA alloc sharedmem (only just remap);
	 */
	check_is_teecd(vma, &event_control, &shared_mem, &is_teecd);
	mutex_lock(&dev_file->shared_mem_lock);
	alloc_shared_mem_for_ca_and_agent(vma, &shared_mem,
		dev_file, &only_remap, is_teecd);
	/* code runs here, for teecd, thirdparty ca agent and ca,
	 * shared_mem is allocated;
	 * for only teecd, event_control is assigned value;
	 */
	if (IS_ERR_OR_NULL(shared_mem)) {
		put_agent_event(event_control);
		mutex_unlock(&dev_file->shared_mem_lock);
		return -1;
	}
	/* when remap, check the len of remap mem */
	check_value = (only_remap &&
		vma->vm_end - vma->vm_start != shared_mem->len);
	if (check_value) {
		tloge("len of map memory is invalid!\n");
		put_sharemem_struct(shared_mem);
		put_agent_event(event_control);
		mutex_unlock(&dev_file->shared_mem_lock);
		return -1;
	}

	ret = remap_shared_mem(vma, event_control, shared_mem, only_remap);
	if (ret != 0) {
		if (only_remap)
			put_sharemem_struct(shared_mem);
		mutex_unlock(&dev_file->shared_mem_lock);
		return ret;
	}

	vma->vm_flags |= VM_DONTCOPY;
	vma->vm_ops = &shared_remap_vm_ops;
	shared_vma_open(vma);
	vma->vm_private_data = (void *)dev_file;

	if (only_remap) {
		tloge("only need remap to client\n");
		shared_mem->user_addr_ca = (void *)vma->vm_start;
		mutex_unlock(&dev_file->shared_mem_lock);
		return ret;
	}
	shared_mem->user_addr = (void *)vma->vm_start;
	atomic_set(&shared_mem->offset, vma->vm_pgoff);
	get_sharemem_struct(shared_mem);
	list_add_tail(&shared_mem->head, &dev_file->shared_mem_list);
	mutex_unlock(&dev_file->shared_mem_lock);
	put_agent_event(event_control);
	return ret;
}

static int ioctl_session_send_cmd(tc_ns_dev_file *dev_file,
	tc_ns_client_context *context, void *argp)
{
	int ret = 0;

	ret = tc_ns_send_cmd(dev_file, context);
	if (ret)
		tloge("tc_ns_send_cmd Failed ret is %d\n", ret);
	if (copy_to_user(argp, context, sizeof(tc_ns_client_context))) {
		if (ret == 0)
			ret = -EFAULT;
	}
	return ret;
}

static int tc_client_session_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	int ret = TEEC_ERROR_GENERIC;
	void *argp = (void __user *)arg;
	tc_ns_dev_file *dev_file = file->private_data;
	tc_ns_client_context context;

	if (argp == NULL) {
		tloge("argp is NULL input buffer\n");
		ret = -EINVAL;
		return ret;
	}
	if (copy_from_user(&context, argp, sizeof(tc_ns_client_context))) {
		tloge("copy from user failed\n");
		return -EFAULT;
	}
	context.returns.origin = TEEC_ORIGIN_COMMS;
	switch (cmd) {
	case TC_NS_CLIENT_IOCTL_SES_OPEN_REQ: {
		CFC_FUNC_ENTRY(tc_client_session_ioctl);
		ret = tc_ns_open_session(dev_file, &context);
		if (ret)
			tloge("tc_ns_open_session Failed ret is %d\n", ret);
		if (copy_to_user(argp, &context, sizeof(context)) && ret == 0)
			ret = -EFAULT;
		break;
	}
	case TC_NS_CLIENT_IOCTL_SES_CLOSE_REQ: {
		ret = tc_ns_close_session(dev_file, &context);
		break;
	}
	case TC_NS_CLIENT_IOCTL_SEND_CMD_REQ: {
		ret = ioctl_session_send_cmd(dev_file, &context, argp);
		break;
	}
	default:
		tloge("invalid cmd:0x%x!", cmd);
		return ret;
	}
	/*
	 * Don't leak ERESTARTSYS to user space.
	 *
	 * CloseSession is not reentrant, so convert to -EINTR.
	 * In other case, restart_syscall().
	 *
	 * It is better to call it right after the error code
	 * is generated (in tc_client_call), but kernel CAs are
	 * still exist when these words are written. Setting TIF
	 * flags for callers of those CAs is very hard to analysis.
	 *
	 * For kernel CA, when ERESTARTSYS is seen, loop in kernel
	 * instead of notifying user.
	 *
	 * P.S. ret code in this function is in mixed naming space.
	 * See the definition of ret. However, this function never
	 * return its default value, so using -EXXX is safe.
	 */
	if (ret == -ERESTARTSYS) {
		if (cmd == TC_NS_CLIENT_IOCTL_SES_CLOSE_REQ) {
			ret = -EINTR;
		} else {
			return restart_syscall();
		}
	}
	return ret;
}

static tc_ns_shared_mem *find_shared_mem(tc_ns_dev_file *dev_file,
	int *find_flag)
{
	tc_ns_shared_mem *tmp_mem = NULL;

	/* find sharedmem */
	mutex_lock(&dev_file->shared_mem_lock);
	list_for_each_entry(tmp_mem, &dev_file->shared_mem_list, head) {
		if (tmp_mem != NULL) {
			*find_flag = 1;
			get_sharemem_struct(tmp_mem);
			mutex_unlock(&dev_file->shared_mem_lock);
			return tmp_mem;
		}
	}
	mutex_unlock(&dev_file->shared_mem_lock);
	return tmp_mem;
}

static int ioctl_register_agent(tc_ns_dev_file *dev_file, unsigned long arg)
{
	int ret = 0;
	tc_ns_shared_mem *shared_mem = NULL;
	int find_flag = 0;

	shared_mem = find_shared_mem(dev_file, &find_flag);
	ret = tc_ns_register_agent(dev_file, (unsigned int)arg, shared_mem);
	if (find_flag)
		put_sharemem_struct(shared_mem);
	return ret;
}

static int ioctl_unregister_agent(tc_ns_dev_file *dev_file, unsigned long arg)
{
	int ret = 0;
	struct smc_event_data *event_data = NULL;

	event_data = find_event_control((unsigned int)arg);
	if (event_data == NULL) {
		tloge("invalid agent id\n");
		return TEEC_ERROR_GENERIC;
	}
	if (event_data->owner != dev_file) {
		tloge("invalid unregister request\n");
		put_agent_event(event_data);
		return TEEC_ERROR_GENERIC;
	}
	put_agent_event(event_data);
	ret = tc_ns_unregister_agent((unsigned int)arg);
	return ret;
}

static long tc_agent_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = TEEC_ERROR_GENERIC;
	tc_ns_dev_file *dev_file = file->private_data;

	if (dev_file == NULL) {
		tloge("invalid params\n");
		return ret;
	}
	switch (cmd) {
	case TC_NS_CLIENT_IOCTL_WAIT_EVENT: {
		ret = tc_ns_wait_event((unsigned int)arg);
		break;
	}
	case TC_NS_CLIENT_IOCTL_SEND_EVENT_RESPONSE: {
		ret = tc_ns_send_event_response((unsigned int)arg);
		break;
	}
	case TC_NS_CLIENT_IOCTL_REGISTER_AGENT: {
		ret = ioctl_register_agent(dev_file, arg);
		break;
	}
	case TC_NS_CLIENT_IOCTL_UNREGISTER_AGENT: {
		ret = ioctl_unregister_agent(dev_file, arg);
		break;
	}
	case TC_NS_CLIENT_IOCTL_SYC_SYS_TIME: {
		ret = tc_ns_sync_sys_time((tc_ns_client_time *)arg);
		break;
	}
	case TC_NS_CLIENT_IOCTL_SET_NATIVE_IDENTITY: {
		ret = tc_ns_set_nativeca_hash(arg);
		break;
	}
	case TC_NS_CLIENT_IOCTL_LOAD_TTF_FILE_AND_NOTCH_HEIGHT: {
		ret = load_tui_font_file(normal, arg);
		break;
	}
	case TC_NS_CLIENT_IOCTL_LOW_TEMPERATURE_MODE: {
		ret = switch_low_temperature_mode((unsigned int)arg);
		break;
	}
	case TC_NS_CLIENT_IOCTL_LATEINIT: {
		ret = tc_ns_late_init((unsigned int)arg);
		break;
	}
	default:
		tloge("invalid cmd!");
		return ret;
	}
	tlogd("TC_NS_ClientIoctl ret = 0x%x\n", ret);
	return ret;
}

static long tc_client_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = TEEC_ERROR_GENERIC;
	void *argp = (void __user *)arg;
	tc_ns_dev_file *dev_file = file->private_data;
	tc_ns_client_context client_context = { { 0 } };

	switch (cmd) {
	/* IOCTLs for the CAs */
	case TC_NS_CLIENT_IOCTL_SES_OPEN_REQ:
		/* Upvote for peripheral zone votage, needed by Coresight.
		 * Downvote will be processed inside CFC_RETURN_PMCLK_ON_COND
		 */
		cfc_prepare_clk_pm();
		CFC_FUNC_ENTRY(tc_client_ioctl);
	/* Fall through */
	case TC_NS_CLIENT_IOCTL_SES_CLOSE_REQ:
	case TC_NS_CLIENT_IOCTL_SEND_CMD_REQ:
		ret = tc_client_session_ioctl(file, cmd, arg);
		break;
	case TC_NS_CLIENT_IOCTL_CANCEL_CMD_REQ:
		if (argp == NULL) {
			tloge("argp is NULL input buffer\n");
			ret = -EINVAL;
			break;
		}
		if (copy_from_user(&client_context, argp,
			sizeof(tc_ns_client_context))) {
			tloge("copy from user failed\n");
			ret = -ENOMEM;
			break;
		}
		ret = tc_ns_send_cmd(dev_file, &client_context);
		break;
	/* This is the login information
	 * and is set teecd when client opens a new session
	 */
	case TC_NS_CLIENT_IOCTL_LOGIN: {
		ret = tc_ns_client_login_func(dev_file, argp);
		break;
	}
	/* IOCTLs for the secure storage daemon */
	case TC_NS_CLIENT_IOCTL_WAIT_EVENT:
	case TC_NS_CLIENT_IOCTL_SEND_EVENT_RESPONSE:
	case TC_NS_CLIENT_IOCTL_REGISTER_AGENT:
	case TC_NS_CLIENT_IOCTL_UNREGISTER_AGENT:
	case TC_NS_CLIENT_IOCTL_SYC_SYS_TIME:
	case TC_NS_CLIENT_IOCTL_LOAD_TTF_FILE_AND_NOTCH_HEIGHT:
	case TC_NS_CLIENT_IOCTL_SET_NATIVE_IDENTITY:
	case TC_NS_CLIENT_IOCTL_LOW_TEMPERATURE_MODE:
	case TC_NS_CLIENT_IOCTL_LATEINIT:
		ret = tc_agent_ioctl(file, cmd, arg);
		break;
	/* for tui service inform TUI TA  event type */
	case TC_NS_CLIENT_IOCTL_TUI_EVENT: {
		ret = tc_ns_tui_event(dev_file, argp);
		break;
	}
	default:
		tloge("invalid cmd 0x%x!", cmd);
		break;
	}

	tlogd("tc_client_ioctl ret = 0x%x\n", ret);
	CFC_RETURN_PMCLK_ON_COND(tc_client_ioctl, 0, ret,
		cmd == TC_NS_CLIENT_IOCTL_SES_OPEN_REQ);
}

static int tc_client_open(struct inode *inode, struct file *file)
{
	int ret = TEEC_ERROR_GENERIC;
	tc_ns_dev_file *dev = NULL;

	ret = check_process_access(current, NON_HIDL_SIDE);
	if (ret) {
		tloge(KERN_ERR "teecd service may be exploited 0x%x\n", ret);
		return -EPERM;
	}
	if (!g_teecd_hash_enable) {
		if (memset_s((void *)g_teecd_hash, sizeof(g_teecd_hash),
			0x00, sizeof(g_teecd_hash))) {
			tloge("tc_client_open memset failed!\n");
			return -EFAULT;
		}
		g_teecd_hash_enable = (current->mm &&
			!tee_calc_task_hash(g_teecd_hash, false, current));
		if (!g_teecd_hash_enable) {
			tloge("calc teecd hash failed\n");
			return -EFAULT;
		}
	}
	g_teecd_task = current->group_leader;
	file->private_data = NULL;
	ret = tc_ns_client_open(&dev, TEE_REQ_FROM_USER_MODE);
	if (ret == TEEC_SUCCESS)
		file->private_data = dev;
	return ret;
}

static int tc_client_close(struct inode *inode, struct file *file)
{
	int ret = TEEC_ERROR_GENERIC;
	tc_ns_dev_file *dev = file->private_data;
	bool check_value = false;

	/* release tui resource */
	if (dev->dev_file_id == tui_attach_device())
		tui_send_event(TUI_POLL_CANCEL, 0);
	check_value = (g_teecd_task == current->group_leader) &&
		(!tc_ns_get_uid());
	if (check_value == true) {
		/* for teecd close fd */
		check_value = g_teecd_task->flags & PF_EXITING ||
			current->flags & PF_EXITING;
		if (check_value == true) {
			/* when teecd is be killed or crash */
			tloge("teecd is killed, something bad must be happened!!!\n");
			tc_ns_send_event_response_all();
			if (tc_ns_is_system_agent_client(dev)) {
				/* for teecd agent close fd */
				ret = tc_ns_client_close(dev, 1);
			} else {
				/* for ca damon close fd */
				ret = ns_client_close_teecd_not_agent(dev);
			}
		} else {
			/* for ca damon close fd when ca damon close fd
			 *  later than HIDL thread
			 */
			ret = tc_ns_client_close(dev, 0);
		}
	} else {
		/* for CA(HIDL thread) close fd */
		ret = tc_ns_client_close(dev, 0);
	}
	file->private_data = NULL;
	return ret;
}

long tc_compat_client_ioctl(struct file *flie, unsigned int cmd,
	unsigned long arg)
{
	long ret = -ENOIOCTLCMD;

	arg = (unsigned long)compat_ptr(arg);
	ret = tc_client_ioctl(flie, cmd, arg);
	return ret;
}

static const struct file_operations TC_NS_ClientFops = {
	.owner = THIS_MODULE,
	.open = tc_client_open,
	.release = tc_client_close,
	.unlocked_ioctl = tc_client_ioctl,
	.mmap = tc_client_mmap,
	.compat_ioctl = tc_compat_client_ioctl,
};

static int tui_flag = 0;

static int tc_ns_client_init(struct device **class_dev)
{
	int ret = 0;

	tlogd("tc_ns_client_init");
	g_np = of_find_compatible_node(NULL, NULL, "trusted_core");

	if (g_np == NULL) {
		tloge("No trusted_core compatible node found.\n");
		return -ENODEV;
	}
	ret = alloc_chrdev_region(&g_tc_ns_client_devt, 0, 1, TC_NS_CLIENT_DEV);
	if (ret < 0) {
		tloge("alloc_chrdev_region failed %d", ret);
		return -EFAULT;
	}
	g_driver_class = class_create(THIS_MODULE, TC_NS_CLIENT_DEV);
	if (IS_ERR(g_driver_class)) {
		ret = -ENOMEM;
		tloge("class_create failed %d", ret);
		goto chrdev_region_unregister;
	}
	*class_dev = device_create(g_driver_class, NULL, g_tc_ns_client_devt,
		NULL, TC_NS_CLIENT_DEV);
	if (IS_ERR(*class_dev)) {
		tloge("class_device_create failed");
		ret = -ENOMEM;
		goto destroy_class;
	}
	(*class_dev)->of_node = g_np;
	cdev_init(&g_tc_ns_client_cdev, &TC_NS_ClientFops);
	g_tc_ns_client_cdev.owner = THIS_MODULE;
	ret = cdev_add(&g_tc_ns_client_cdev,
		MKDEV(MAJOR(g_tc_ns_client_devt), 0), 1);
	if (ret < 0) {
		tloge("cdev_add failed %d", ret);
		goto class_device_destroy;
	}
	ret = memset_s(&g_tc_ns_dev_list, sizeof(g_tc_ns_dev_list), 0,
		sizeof(g_tc_ns_dev_list));
	if (ret != EOK)
		goto class_device_destroy;
	INIT_LIST_HEAD(&g_tc_ns_dev_list.dev_file_list);
	mutex_init(&g_tc_ns_dev_list.dev_lock);
	mutex_init(&g_tee_crypto_hash_lock);
	INIT_LIST_HEAD(&g_service_list);
	return ret;

class_device_destroy:
	device_destroy(g_driver_class, g_tc_ns_client_devt);
destroy_class:
	class_destroy(g_driver_class);
chrdev_region_unregister:
	unregister_chrdev_region(g_tc_ns_client_devt, 1);
	return ret;
}

static int tc_teeos_init(struct device *class_dev)
{
	int ret = 0;

	ret = smc_init_data(class_dev);
	if (ret < 0)
		return ret;
	ret = tc_mem_init();
	if (ret < 0)
		goto smc_data_free;
	agent_init();
	ret = tc_ns_register_ion_mem();
	if (ret < 0) {
		pr_err("Failed to register ion mem in tee.\n");
		goto free_agent;
	}
	ret = tc_ns_register_rdr_mem();
	if (ret)
		tloge("TC_NS_register_rdr_mem failed %x\n", ret);
	ret = teeos_register_exception();   /* 0:error */
	if (ret != 0)
		tloge("teeos_register_exception to rdr failed\n");
	ret = tz_spi_init(class_dev, g_np);
	if (ret)
		goto free_agent;
	return ret;

free_agent:
	agent_exit();
	tc_mem_destroy();
smc_data_free:
	smc_free_data();
	return ret;
}

static int tc_tui_init(struct device *class_dev)
{
	int ret = 0;

	ret = init_tui(class_dev);
	if (ret) {
		tloge("init_tui failed 0x%x\n", ret);
		tui_flag = 1;
		/* go on init, skip tui init fail. */
	}

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	g_drm_ion_client = hisi_ion_client_create("DRM_ION");
	if (IS_ERR(g_drm_ion_client)) {
		tloge("in %s err: drm ion client create failed!\n", __func__);
		ret = -EFAULT;
		goto free_tui;
	}
#endif
	if (init_smc_svc_thread()) {
		ret = -EFAULT;
		goto free_tui;
	}
	if (init_dynamic_mem()) {
		tloge("init_dynamic_mem Failed\n");
		ret = -EFAULT;
		goto free_tui;
	}
	return ret;

free_tui:
	/* if error happens */
	if (!tui_flag)
		tui_exit();
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	if (g_drm_ion_client != NULL) {
		ion_client_destroy(g_drm_ion_client);
		g_drm_ion_client = NULL;
	}
#endif
	return ret;
}

static __init int tc_init(void)
{
	struct device *class_dev = NULL;
	int ret = 0;

	ret = tc_ns_client_init(&class_dev);
	if (ret != 0)
		return ret;

	ret = tc_teeos_init(class_dev);
	if (ret != 0)
		goto class_device_destroy;

	ret = tc_tui_init(class_dev);
	if (ret != 0)
		goto free_teeos;
	return ret;

free_teeos:
	tz_spi_exit();
	agent_exit();
	tc_mem_destroy();
	smc_free_data();
class_device_destroy:
	device_destroy(g_driver_class, g_tc_ns_client_devt);
	class_destroy(g_driver_class);
	unregister_chrdev_region(g_tc_ns_client_devt, 1);

	return ret;
}

static void tc_exit(void)
{
	tlogd("otz_client exit");
	if (!tui_flag)
		tui_exit();
	tz_spi_exit();
	device_destroy(g_driver_class, g_tc_ns_client_devt);
	class_destroy(g_driver_class);
	unregister_chrdev_region(g_tc_ns_client_devt, 1);
	smc_free_data();
	agent_exit();
	tc_mem_destroy();
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	if (g_drm_ion_client != NULL) {
		ion_client_destroy(g_drm_ion_client);
		g_drm_ion_client = NULL;
	}
#endif
	if (g_tee_shash_tfm != NULL) {
		crypto_free_shash(g_tee_shash_tfm);
		g_tee_init_crypt_state = 0;
		g_tee_shash_tfm = NULL;
	}
	exit_dynamic_mem();
}

MODULE_AUTHOR("q00209673");
MODULE_DESCRIPTION("TrustCore ns-client driver");
MODULE_VERSION("1.10");
fs_initcall_sync(tc_init);
module_exit(tc_exit);
