/*
 * session_manager.c
 *
 * function for session management
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
#include "session_manager.h"
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/file.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/spinlock_types.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <asm/cacheflush.h>
#include <linux/kthread.h>
#include <linux/atomic.h>
#include <linux/vmalloc.h>
#include <linux/pid.h>
#include <linux/cred.h>
#include <linux/thread_info.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/sched/mm.h>
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#include <linux/completion.h>
#include "smc_smp.h"
#include "mem.h"
#include "gp_ops.h"
#include "tc_ns_log.h"
#include "teek_client_constants.h"
#include "client_hash_auth.h"
#include "security_auth_enhance.h"
#include "mailbox_mempool.h"
#include "tc_client_driver.h"
#include "dynamic_ion_mem.h"

static DEFINE_MUTEX(g_load_app_lock);
#define MAX_REF_COUNT 255

/* record all service node and need mutex to avoid race */
struct list_head g_service_list;
DEFINE_MUTEX(g_service_list_lock);

struct load_img_params {
	struct tc_ns_dev_file *dev_file;
	const char *file_buffer;
	unsigned int file_size;
	struct mb_cmd_pack *mb_pack;
	char *mb_load_mem;
	struct tc_uuid *uuid_return;
	unsigned int mb_load_size;
};

void init_srvc_list(void)
{
	INIT_LIST_HEAD(&g_service_list);
}

void get_session_struct(struct tc_ns_session *session)
{
	if (!session)
		return;

	atomic_inc(&session->usage);
}

void put_session_struct(struct tc_ns_session *session)
{
	if (!session || !atomic_dec_and_test(&session->usage))
		return;

#ifdef CONFIG_AUTH_ENHANCE
	if (session->teec_token.token_buffer) {
		if (memset_s(
			session->teec_token.token_buffer,
			session->teec_token.token_len, 0,
			session->teec_token.token_len))
			tloge("Caution, memset failed!\n");
		kfree(session->teec_token.token_buffer);
		session->teec_token.token_buffer = NULL;
	}
#endif
	if (memset_s(session, sizeof(*session), 0, sizeof(*session)))
		tloge("Caution, memset failed!\n");
	kfree(session);
}

void get_service_struct(struct tc_ns_service *service)
{
	if (!service)
		return;

	atomic_inc(&service->usage);
	tlogd("service->usage = %d\n", atomic_read(&service->usage));
}

void put_service_struct(struct tc_ns_service *service)
{
	if (!service)
		return;

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

static int add_service_to_dev(struct tc_ns_dev_file *dev,
	struct tc_ns_service *service)
{
	uint32_t i;

	if (!dev || !service)
		return -EINVAL;

	for (i = 0; i < SERVICES_MAX_COUNT; i++) {
		if (!dev->services[i]) {
			tlogd("add service %u to %u\n", i, dev->dev_file_id);
			dev->services[i] = service;
			dev->service_ref[i] = 1;
			return 0;
		}
	}
	return -EFAULT;
}

void dump_services_status(const char *param)
{
	struct tc_ns_service *service = NULL;

	(void)param;
	mutex_lock(&g_service_list_lock);
	tlogi("show service list:\n");
	list_for_each_entry(service, &g_service_list, head) {
		tlogi("uuid-%x, usage=%d\n", *(uint32_t *)service->uuid,
			atomic_read(&service->usage));
	}
	mutex_unlock(&g_service_list_lock);
}

static void del_service_from_dev(struct tc_ns_dev_file *dev,
	struct tc_ns_service *service)
{
	uint32_t i;

	for (i = 0; i < SERVICES_MAX_COUNT; i++) {
		if (dev->services[i] == service) {
			tlogd("dev service ref-%u = %u\n", i,
				dev->service_ref[i]);
			if (dev->service_ref[i] == 0) {
				tloge("Caution! No service to be deleted!\n");
				break;
			}
			dev->service_ref[i]--;
			if (!dev->service_ref[i]) {
				tlogd("del service %u from %u\n",
					i, dev->dev_file_id);
				dev->services[i] = NULL;
				put_service_struct(service);
			}
			break;
		}
	}
}

struct tc_ns_session *tc_find_session_withowner(
	const struct list_head *session_list,
	unsigned int session_id, struct tc_ns_dev_file *dev_file)
{
	struct tc_ns_session *session = NULL;

	if (!session_list || !dev_file) {
		tloge("session list or dev is null\n");
		return NULL;
	}

	list_for_each_entry(session, session_list, head) {
		if (session->session_id == session_id &&
			session->owner == dev_file)
			return session;
	}
	return NULL;
}

struct tc_ns_service *tc_find_service_in_dev(const struct tc_ns_dev_file *dev,
	const unsigned char *uuid, int uuid_size)
{
	uint32_t i;

	if (!dev || !uuid || uuid_size != UUID_LEN)
		return NULL;

	for (i = 0; i < SERVICES_MAX_COUNT; i++) {
		if (dev->services[i] != NULL &&
			!memcmp(dev->services[i]->uuid, uuid, UUID_LEN))
			return dev->services[i];
	}
	return NULL;
}

struct tc_ns_session *tc_find_session_by_uuid(unsigned int dev_file_id,
	const struct tc_ns_smc_cmd *cmd)
{
	struct tc_ns_dev_file *dev_file = NULL;
	struct tc_ns_service *service = NULL;
	struct tc_ns_session *session = NULL;

	if (!cmd) {
		tloge("parameter is null pointer!\n");
		return NULL;
	}

	dev_file = tc_find_dev_file(dev_file_id);
	if (!dev_file) {
		tloge("can't find dev file!\n");
		return NULL;
	}

	mutex_lock(&dev_file->service_lock);
	service = tc_find_service_in_dev(dev_file, cmd->uuid, UUID_LEN);
	get_service_struct(service);
	mutex_unlock(&dev_file->service_lock);
	if (!service) {
		tloge("can't find service!\n");
		return NULL;
	}

	mutex_lock(&service->session_lock);
	session = tc_find_session_withowner(&service->session_list,
		cmd->context_id, dev_file);
	get_session_struct(session);
	mutex_unlock(&service->session_lock);
	put_service_struct(service);
	if (!session) {
		tloge("can't find session-0x%x!\n", cmd->context_id);
		return NULL;
	}
	return session;
}

static int tc_ns_need_load_image(unsigned int file_id,
	const unsigned char *uuid, unsigned int uuid_len)
{
	int ret;
	int smc_ret;
	struct tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	struct mb_cmd_pack *mb_pack = NULL;
	char *mb_param = NULL;

	if (!uuid || uuid_len != UUID_LEN) {
		tloge("invalid uuid\n");
		return -ENOMEM;
	}
	mb_pack = mailbox_alloc_cmd_pack();
	if (!mb_pack) {
		tloge("alloc mb pack failed\n");
		return -ENOMEM;
	}
	mb_param = mailbox_copy_alloc(uuid, uuid_len);
	if (!mb_param) {
		tloge("alloc mb param failed\n");
		ret = -ENOMEM;
		goto clean;
	}
	mb_pack->operation.paramtypes = TEEC_MEMREF_TEMP_INOUT;
	mb_pack->operation.params[0].memref.buffer =
		virt_to_phys(mb_param);
	mb_pack->operation.buffer_h_addr[0] =
		virt_to_phys(mb_param) >> ADDR_TRANS_NUM;
	mb_pack->operation.params[0].memref.size = SZ_4K;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_NEED_LOAD_APP;
	smc_cmd.global_cmd = true;
	smc_cmd.dev_file_id = file_id;
	smc_cmd.context_id = 0;
	smc_cmd.operation_phys = virt_to_phys(&mb_pack->operation);
	smc_cmd.operation_h_phys =
		virt_to_phys(&mb_pack->operation) >> ADDR_TRANS_NUM;

	smc_ret = tc_ns_smc(&smc_cmd);
	if (smc_ret) {
		tloge("smc call returns error ret 0x%x\n", smc_ret);
		ret = -EFAULT;
		goto clean;
	} else {
		ret = *(int *)mb_param;
	}
clean:
	if (mb_param)
		mailbox_free(mb_param);
	mailbox_free(mb_pack);

	return ret;
}

int tc_ns_load_secfile(struct tc_ns_dev_file *dev_file,
	const void __user *argp)
{
	int ret;
	struct load_secfile_ioctl_struct ioctl_arg = { 0, {0}, 0, {NULL} };

	if (!dev_file || !argp) {
		tloge("Invalid params !\n");
		return -EINVAL;
	}

	if (copy_from_user(&ioctl_arg, argp, sizeof(ioctl_arg))) {
		tloge("copy from user failed\n");
		ret = -ENOMEM;
		return ret;
	}

	mutex_lock(&g_load_app_lock);
	if (ioctl_arg.secfile_type == LOAD_TA) {
		ret = tc_ns_need_load_image(dev_file->dev_file_id,
			ioctl_arg.uuid, (unsigned int)UUID_LEN);
		if (ret == 1) /* 1 means we need to load image */
			ret = tc_ns_load_image(dev_file, ioctl_arg.file_buffer,
				ioctl_arg.file_size);
	} else if (ioctl_arg.secfile_type == LOAD_LIB) {
		ret = tc_ns_load_image(dev_file,
			ioctl_arg.file_buffer, ioctl_arg.file_size);
	} else {
		tloge("invalid secfile type: %d!", ioctl_arg.secfile_type);
		ret = -EINVAL;
	}
	if (ret)
		tloge("load TA secfile: %d failed, ret = %x",
			ioctl_arg.secfile_type, ret);
	mutex_unlock(&g_load_app_lock);
	return ret;
}

/*
 * Modify the client context so params id 2 and 3 contain temp pointers to the
 * public key and package name for the open session. This is used for the
 * TEEC_LOGIN_IDENTIFY open session method
 */
static int set_login_information(struct tc_ns_dev_file *dev_file,
	struct tc_ns_client_context *context)
{
	/* The daemon has failed to get login information or not supplied */
	if (!dev_file->pkg_name_len)
		return -EINVAL;
	/*
	 * The 3rd parameter buffer points to the pkg name buffer in the
	 * device file pointer
	 * get package name len and package name
	 */
	context->params[3].memref.size_addr =
		(__u64)(uintptr_t)&dev_file->pkg_name_len;
	context->params[3].memref.buffer =
		(__u64)(uintptr_t)dev_file->pkg_name;
	/* Set public key len and public key */
	if (dev_file->pub_key_len) {
		context->params[2].memref.size_addr =
			(__u64)(uintptr_t)&dev_file->pub_key_len;
		context->params[2].memref.buffer =
			(__u64)(uintptr_t)dev_file->pub_key;
	} else {
		/* If get public key failed, then get uid in kernel */
		uint32_t ca_uid = tc_ns_get_uid();
		if (ca_uid == (uint32_t)(-1)) {
			tloge("failed to get uid of the task\n");
			goto error;
		}
		dev_file->pub_key_len = sizeof(ca_uid);
		context->params[2].memref.size_addr =
			(__u64)(uintptr_t)&dev_file->pub_key_len;
		if (memcpy_s(dev_file->pub_key, MAX_PUBKEY_LEN, &ca_uid,
			dev_file->pub_key_len)) {
			tloge("failed to copy pubkey, pub key len=%u\n",
			      dev_file->pub_key_len);
			goto error;
		}
		context->params[2].memref.buffer =
			(__u64)(uintptr_t)dev_file->pub_key;
	}
	/* Now we mark the 2 parameters as input temp buffers */
	context->param_types = teec_param_types(
		teec_param_type_get(context->param_types, 0),
		teec_param_type_get(context->param_types, 1),
		TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT);
	return 0;
error:
	return -EFAULT;
}

static int check_login_method(struct tc_ns_dev_file *dev_file,
	struct tc_ns_client_context *context, uint8_t *flags)
{
	int ret;

	if (!dev_file || !context || !flags)
		return -EFAULT;

	if (context->login.method != TEEC_LOGIN_IDENTIFY) {
		tlogd("login method is not supported\n");
		return -EINVAL;
	}

	tlogd("login method is IDENTIFY\n");
	/* check if usr params 0 and 1 are valid */
	if (dev_file->kernel_api == TEE_REQ_FROM_USER_MODE &&
		(!tc_user_param_valid(context, (unsigned int)0) ||
		!tc_user_param_valid(context, (unsigned int)1)))
		return -EINVAL;

	ret = set_login_information(dev_file, context);
	if (ret) {
		tloge("set login information failed ret =%d\n", ret);
		return ret;
	}
	*flags |= TC_CALL_LOGIN;

	return 0;
}

static struct tc_ns_service *tc_ref_service_in_dev(struct tc_ns_dev_file *dev,
	const unsigned char *uuid, int uuid_size, bool *is_full)
{
	uint32_t i;

	if (uuid_size != UUID_LEN)
		return NULL;

	for (i = 0; i < SERVICES_MAX_COUNT; i++) {
		if (dev->services[i] != NULL &&
			!memcmp(dev->services[i]->uuid, uuid, UUID_LEN)) {
			if (dev->service_ref[i] == MAX_REF_COUNT) {
				*is_full = true;
				return NULL;
			}
			dev->service_ref[i]++;
			return dev->services[i];
		}
	}
	return NULL;
}

static int tc_ns_service_init(const unsigned char *uuid, uint32_t uuid_len,
	struct tc_ns_service **new_service)
{
	int ret = 0;
	struct tc_ns_service *service = NULL;

	if (!uuid || !new_service || uuid_len != UUID_LEN)
		return -EINVAL;

	service = kzalloc(sizeof(*service), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)service)) {
		tloge("kzalloc failed\n");
		ret = -ENOMEM;
		return ret;
	}

	if (memcpy_s(service->uuid, sizeof(service->uuid), uuid, uuid_len)) {
		kfree(service);
		return -EFAULT;
	}

	INIT_LIST_HEAD(&service->session_list);
	mutex_init(&service->session_lock);
	list_add_tail(&service->head, &g_service_list);
	tlogd("add service: 0x%x to service list\n", *(uint32_t *)uuid);
	atomic_set(&service->usage, 1);
	mutex_init(&service->operation_lock);
	*new_service = service;

	return ret;
}

static struct tc_ns_service *tc_find_service_from_all(
	const unsigned char *uuid, uint32_t uuid_len)
{
	struct tc_ns_service *service = NULL;

	if (!uuid || uuid_len != UUID_LEN)
		return NULL;

	list_for_each_entry(service, &g_service_list, head) {
		if (!memcmp(service->uuid, uuid, sizeof(service->uuid)))
			return service;
	}

	return NULL;
}

static struct tc_ns_service *find_service(struct tc_ns_dev_file *dev_file,
	const struct tc_ns_client_context *context)
{
	int ret;
	struct tc_ns_service *service = NULL;
	bool is_full = false;

	mutex_lock(&dev_file->service_lock);
	service = tc_ref_service_in_dev(dev_file, context->uuid,
		UUID_LEN, &is_full);
	/* if service has been opened in this dev or ref cnt is full */
	if (service || is_full) {
		/*
		 * If service has been reference by this dev, find service in dev
		 * will incre ref count to declaim there's how many callers to
		 * this service from the dev, instead of incre service->usage.
		 * While close session, dev->service_ref[i] will decre and till
		 * it get to 0, put service struct will be called.
		 */
		mutex_unlock(&dev_file->service_lock);
		return service;
	}
	mutex_lock(&g_service_list_lock);
	service = tc_find_service_from_all(context->uuid, UUID_LEN);
	/* if service has been opened in other dev */
	if (service) {
		get_service_struct(service);
		mutex_unlock(&g_service_list_lock);
		goto add_service;
	}
	/* Create a new service if we couldn't find it in list */
	ret = tc_ns_service_init(context->uuid, UUID_LEN, &service);
	/* unlock after init to make sure find service from all is correct */
	mutex_unlock(&g_service_list_lock);
	if (ret) {
		tloge("service init failed");
		mutex_unlock(&dev_file->service_lock);
		return NULL;
	}
add_service:
	ret = add_service_to_dev(dev_file, service);
	mutex_unlock(&dev_file->service_lock);
	if (ret) {
		/*
		 * for new srvc, match init usage to 1;
		 * for srvc already exist, match get;
		 */
		put_service_struct(service);
		service = NULL;
		tloge("fail to add service to dev\n");
		return NULL;
	}
	return service;
}

static bool is_valid_ta_size(const char *file_buffer, unsigned int file_size)
{
	if (!file_buffer || !file_size) {
		tloge("invalid load ta size\n");
		return false;
	}

	if (file_size > SZ_8M) {
		tloge("not support TA larger than 8M, size=%u\n", file_size);
		return false;
	}
	return true;
}

static int alloc_for_load_image(struct load_img_params *params)
{
	/* we will try any possible to alloc mailbox mem to load TA */
	for (; params->mb_load_size > 0; params->mb_load_size >>= 1) {
		params->mb_load_mem = mailbox_alloc(params->mb_load_size, 0);
		if (params->mb_load_mem)
			break;
		tlogw("alloc mem size=%u for TA load mem fail\n",
			params->mb_load_size);
	}

	if (!params->mb_load_mem) {
		tloge("alloc TA load mem failed\n");
		return -ENOMEM;
	}

	params->mb_pack = mailbox_alloc_cmd_pack();
	if (!params->mb_pack) {
		mailbox_free(params->mb_load_mem);
		params->mb_load_mem = NULL;
		tloge("alloc mb pack failed\n");
		return -ENOMEM;
	}

	params->uuid_return = mailbox_alloc(sizeof(*(params->uuid_return)), 0);
	if (!params->uuid_return) {
		mailbox_free(params->mb_load_mem);
		params->mb_load_mem = NULL;
		mailbox_free(params->mb_pack);
		params->mb_pack = NULL;
		tloge("alloc uuid failed\n");
		return -ENOMEM;
	}
	return 0;
}

static void pack_load_frame_cmd(uint32_t load_size,
	const struct load_img_params *params, struct tc_ns_smc_cmd *smc_cmd)
{
	struct mb_cmd_pack *mb_pack = params->mb_pack;
	char *mb_load_mem = params->mb_load_mem;
	struct tc_uuid *uuid_return = params->uuid_return;

	mb_pack->operation.params[0].memref.buffer =
		virt_to_phys(mb_load_mem);
	mb_pack->operation.buffer_h_addr[0] =
		virt_to_phys(mb_load_mem) >> ADDR_TRANS_NUM;
	mb_pack->operation.params[0].memref.size = load_size + sizeof(int);
	mb_pack->operation.params[2].memref.buffer =
		virt_to_phys(uuid_return);
	mb_pack->operation.buffer_h_addr[2] =
		virt_to_phys(uuid_return) >> ADDR_TRANS_NUM;
	mb_pack->operation.params[2].memref.size = sizeof(*uuid_return);
	mb_pack->operation.paramtypes = teec_param_types(TEEC_MEMREF_TEMP_INOUT,
		TEEC_VALUE_OUTPUT, TEEC_MEMREF_TEMP_OUTPUT, TEEC_VALUE_INPUT);

	smc_cmd->global_cmd = true;
	smc_cmd->cmd_id = GLOBAL_CMD_ID_LOAD_SECURE_APP;
	smc_cmd->context_id = 0;
	smc_cmd->operation_phys = virt_to_phys(&mb_pack->operation);
	smc_cmd->operation_h_phys =
		virt_to_phys(&mb_pack->operation) >> ADDR_TRANS_NUM;
}

#ifdef CONFIG_DYNAMIC_ION
static int load_image_for_ion(const struct load_img_params *params)
{
	int ret = 0;
	/* check need to add ionmem  */
	uint32_t configid = params->mb_pack->operation.params[1].value.a;
	uint32_t ion_size = params->mb_pack->operation.params[1].value.b;
	int32_t check_result = (configid != 0 && ion_size != 0);

	tloge("check load result=%d, cfgid =%d, ion_size =%d, uuid=%x\n",
		check_result, configid, ion_size,
		params->uuid_return->time_low);
	if (check_result) {
		ret = load_app_use_configid(configid,
			params->dev_file->dev_file_id,
			params->uuid_return, ion_size);
		if (ret) {
			tloge("load app use configid failed ret =%d\n", ret);
			return -EFAULT;
		}
	}
	return ret;
}
#endif

static int load_image_by_frame(struct load_img_params *params,
	unsigned int load_times)
{
	char *p = params->mb_load_mem;
	uint32_t load_size;
	int load_flag = 1; /* 0:it's last block, 1:not last block */
	uint32_t loaded_size = 0;
	unsigned int index;
	struct tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	int smc_ret;

	for (index = 0; index < load_times; index++) {
		if (index == (load_times - 1)) {
			load_flag = 0;
			load_size = params->file_size - loaded_size;
		} else {
			load_size = params->mb_load_size - sizeof(load_flag);
		}
		*(int *)p = load_flag;
		if (load_size > params->mb_load_size - sizeof(load_flag)) {
			tloge("invalid load size %u/%u\n", load_size,
				params->mb_load_size);
			return  -EINVAL;
		}
		if (copy_from_user(params->mb_load_mem + sizeof(load_flag),
			(void __user *)params->file_buffer +
			loaded_size, load_size)) {
			tloge("file buf get fail\n");
			return  -EFAULT;
		}
		pack_load_frame_cmd(load_size, params, &smc_cmd);
		params->mb_pack->operation.params[3].value.a = index;
		smc_cmd.dev_file_id = params->dev_file->dev_file_id;
		smc_ret = tc_ns_smc(&smc_cmd);
		tlogd("configid=%u, ret=%d, load_flag=%d, index=%u\n",
			params->mb_pack->operation.params[1].value.a, smc_ret,
			load_flag, index);

		if (smc_ret)
			return -EFAULT;

#ifdef CONFIG_DYNAMIC_ION
		if (!smc_ret && !load_flag) {
			if (load_image_for_ion(params))
				return -EPERM;
		}
#endif
		loaded_size += load_size;
	}
	return 0;
}

int tc_ns_load_image_with_lock(struct tc_ns_dev_file *dev, const char *file_buffer,
	unsigned int file_size)
{
	int ret;

	if (!dev || !file_buffer) {
		tloge("dev or file buffer is NULL!\n");
		return -EINVAL;
	}

	mutex_lock(&g_load_app_lock);
	ret = tc_ns_load_image(dev, file_buffer, file_size);
	mutex_unlock(&g_load_app_lock);

	return ret;
}

int tc_ns_load_image(struct tc_ns_dev_file *dev, const char *file_buffer,
	unsigned int file_size)
{
	int ret;
	unsigned int load_times;
	struct load_img_params params = {
		dev, file_buffer, file_size, NULL, NULL, NULL, 0
	};

	if (!dev || !file_buffer) {
		tloge("dev or file buffer is NULL!\n");
		return -EINVAL;
	}

	if (!is_valid_ta_size(file_buffer, file_size))
		return -EINVAL;

	params.mb_load_size = (file_size > (SZ_1M - sizeof(int))) ?
		SZ_1M : ALIGN(file_size, SZ_4K);
	ret = alloc_for_load_image(&params);
	if (ret)
		return ret;

	if (params.mb_load_size <= sizeof(int)) {
		tloge("mb load size is too small!\n");
		ret = -ENOMEM;
		goto free_mem;
	}
	load_times = file_size / (params.mb_load_size - sizeof(int));
	if (file_size % (params.mb_load_size - sizeof(int)))
		load_times += 1;
	ret = load_image_by_frame(&params, load_times);
free_mem:
	mailbox_free(params.mb_load_mem);
	mailbox_free(params.mb_pack);
	mailbox_free(params.uuid_return);
	return ret;
}

static int load_ta_image(struct tc_ns_dev_file *dev_file,
	const struct tc_ns_client_context *context)
{
	int ret;

	mutex_lock(&g_load_app_lock);
	ret = tc_ns_need_load_image(dev_file->dev_file_id, context->uuid,
		(unsigned int)UUID_LEN);
	if (ret == 1) { /* 1 means we need to load image */
		if (!context->file_buffer) {
			tloge("context's file_buffer is NULL");
			mutex_unlock(&g_load_app_lock);
			return -1;
		}
		ret = tc_ns_load_image(dev_file, context->file_buffer,
			context->file_size);
		if (ret) {
			tloge("load image failed, ret=%x", ret);
			mutex_unlock(&g_load_app_lock);
			return ret;
		}
	}
	mutex_unlock(&g_load_app_lock);
	return ret;
}

static void init_new_sess_node(struct tc_ns_dev_file *dev_file,
	const struct tc_ns_client_context *context,
	struct tc_ns_service *service,
	struct tc_ns_session *session)
{
	session->session_id = context->session_id;
	atomic_set(&session->usage, 1);
	session->owner = dev_file;

	session->wait_data.send_wait_flag = 0;
	init_waitqueue_head(&session->wait_data.send_cmd_wq);

	mutex_lock(&service->session_lock);
	list_add_tail(&session->head, &service->session_list);
	mutex_unlock(&service->session_lock);
}

static int proc_open_session(struct tc_ns_dev_file *dev_file,
	struct tc_ns_client_context *context, struct tc_ns_service *service,
	struct tc_ns_session *session, uint8_t flags)
{
	int ret;
	struct tc_call_params params = {
		dev_file, context, session, flags
	};

	mutex_lock(&service->operation_lock);
	ret = load_ta_image(dev_file, context);
	if (ret) {
		tloge("load ta image failed\n");
		mutex_unlock(&service->operation_lock);
		return ret;
	}

	ret = get_session_secure_params(dev_file, context, session);
	if (ret) {
		tloge("Get session secure parameters failed, ret = %d\n", ret);
		/* Clean this session secure information */
		clean_session_secure_information(session);
		mutex_unlock(&service->operation_lock);
		return ret;
	}
#ifdef CONFIG_AUTH_ENHANCE
	session->teec_token.token_buffer =
		kzalloc(TOKEN_BUFFER_LEN, GFP_KERNEL);
	session->teec_token.token_len = TOKEN_BUFFER_LEN;
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)
		session->teec_token.token_buffer)) {
		tloge("kzalloc %d bytes token failed\n", TOKEN_BUFFER_LEN);
		/* Clean this session secure information */
		clean_session_secure_information(session);
		mutex_unlock(&service->operation_lock);
		return -ENOMEM;
	}
#endif
	ret = tc_client_call(&params);
	if (ret) {
		/* Clean this session secure information */
		clean_session_secure_information(session);
		kill_ion_by_uuid((struct tc_uuid *)context->uuid);
		mutex_unlock(&service->operation_lock);
		tloge("smc call returns error, ret=0x%x\n", ret);
		return ret;
	}
	init_new_sess_node(dev_file, context, service, session);
	/*
	 * session_id in tee is unique, but in concurrency scene
	 * same session_id may appear in tzdriver, put session_list
	 * add/del in service->operation_lock can avoid it.
	 */
	mutex_unlock(&service->operation_lock);
	return ret;
}

void free_session_token_buf(struct tc_ns_session *session)
{
#ifdef CONFIG_AUTH_ENHANCE
	if (session &&
		session->teec_token.token_buffer) {
		if (memset_s(session->teec_token.token_buffer,
			session->teec_token.token_len,
			0, session->teec_token.token_len))
			tloge("Caution, memset failed!\n");
		kfree(session->teec_token.token_buffer);
		session->teec_token.token_buffer = NULL;
	}
#else
	(void)session;
#endif
}

int tc_ns_open_session(struct tc_ns_dev_file *dev_file,
	struct tc_ns_client_context *context)
{
	int ret;
	struct tc_ns_service *service = NULL;
	struct tc_ns_session *session = NULL;
	uint8_t flags = TC_CALL_GLOBAL;

	if (!dev_file || !context) {
		tloge("invalid dev_file or context\n");
		return -EINVAL;
	}

	ret = check_login_method(dev_file, context, &flags);
	if (ret)
		return ret;

	context->cmd_id = GLOBAL_CMD_ID_OPEN_SESSION;

	service = find_service(dev_file, context);
	if (!service) {
		tloge("find service failed\n");
		return -ENOMEM;
	}

	session = kzalloc(sizeof(*session), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)session)) {
		tloge("kzalloc failed\n");
		mutex_lock(&dev_file->service_lock);
		del_service_from_dev(dev_file, service);
		mutex_unlock(&dev_file->service_lock);
		return -ENOMEM;
	}
	mutex_init(&session->ta_session_lock);

	ret = calc_client_auth_hash(dev_file, context, session);
	if (ret) {
		tloge("calc client auth hash failed\n");
		goto err_free_rsrc;
	}

	ret = proc_open_session(dev_file, context, service, session, flags);
	if (!ret)
		return ret;
err_free_rsrc:
	free_session_token_buf(session);

	mutex_lock(&dev_file->service_lock);
	del_service_from_dev(dev_file, service);
	mutex_unlock(&dev_file->service_lock);

	kfree(session);
	return ret;
}

static struct tc_ns_session *get_session(struct tc_ns_service *service,
	struct tc_ns_dev_file *dev_file,
	const struct tc_ns_client_context *context)
{
	struct tc_ns_session *session = NULL;

	mutex_lock(&service->session_lock);
	session = tc_find_session_withowner(&service->session_list,
		context->session_id, dev_file);
	get_session_struct(session);
	mutex_unlock(&service->session_lock);

	return session;
}

static struct tc_ns_service *get_service(struct tc_ns_dev_file *dev_file,
	const struct tc_ns_client_context *context)
{
	struct tc_ns_service *service = NULL;

	mutex_lock(&dev_file->service_lock);
	service = tc_find_service_in_dev(dev_file, context->uuid, UUID_LEN);
	get_service_struct(service);
	mutex_unlock(&dev_file->service_lock);

	return service;
}

static int close_session(struct tc_ns_dev_file *dev,
	struct tc_ns_session *session, const unsigned char *uuid,
	unsigned int uuid_len, unsigned int session_id)
{
	struct tc_ns_client_context context;
	int ret;
	struct tc_call_params params = {
		dev, &context, session, 0
	};

	if (uuid_len != UUID_LEN)
		return -EINVAL;

	if (memset_s(&context, sizeof(context), 0, sizeof(context)))
		return -EFAULT;

	if (memcpy_s(context.uuid, sizeof(context.uuid), uuid, uuid_len))
		return -EFAULT;

	context.session_id = session_id;
	context.cmd_id = GLOBAL_CMD_ID_CLOSE_SESSION;
	params.flags = TC_CALL_GLOBAL | TC_CALL_SYNC;
	ret = tc_client_call(&params);
	if (ret)
		tloge("close session failed, ret=0x%x\n", ret);

	kill_ion_by_uuid((struct tc_uuid *)context.uuid);
	return ret;
}

static void close_session_in_service_list(struct tc_ns_dev_file *dev,
	struct tc_ns_service *service)
{
	struct tc_ns_session *tmp_session = NULL;
	struct tc_ns_session *session = NULL;
	int ret;

	list_for_each_entry_safe(session, tmp_session,
		&service->session_list, head) {
		if (session->owner != dev)
			continue;
		ret = close_session(dev, session, service->uuid,
			(unsigned int)UUID_LEN, session->session_id);
		if (ret)
			tloge("close session smc failed when close fd!\n");
#ifdef CONFIG_AUTH_ENHANCE
		/* Clean session secure information */
		if (memset_s(&session->secure_info,
			sizeof(session->secure_info), 0,
			sizeof(session->secure_info)))
			tloge("memset error\n");
#endif
		mutex_lock(&service->session_lock);
		list_del(&session->head);
		mutex_unlock(&service->session_lock);

		put_session_struct(session); /* pair with open session */
	}
}

static bool if_exist_unclosed_session(struct tc_ns_dev_file *dev)
{
	uint32_t index;

	for (index = 0; index < SERVICES_MAX_COUNT; index++) {
		if (dev->services[index] != NULL &&
			!list_empty(&dev->services[index]->session_list))
			return true;
	}
	return false;
}

static int close_session_thread_fn(void *arg)
{
	struct tc_ns_dev_file *dev = arg;
	uint32_t index;
	struct tc_ns_service *service = NULL;

	/* close unclosed session */
	for (index = 0; index < SERVICES_MAX_COUNT; index++) {
		if (dev->services[index] != NULL &&
				!list_empty(&dev->services[index]->session_list)) {
				service = dev->services[index];

				mutex_lock(&service->operation_lock);
				close_session_in_service_list(dev, service);
				mutex_unlock(&service->operation_lock);

				put_service_struct(service); /* pair with open session */
			}
	}

	tlogd("complete close all unclosed session\n");
	complete(&dev->close_comp);
	return 0;
}

void close_unclosed_session_in_kthread(struct tc_ns_dev_file *dev)
{
	struct task_struct *close_thread = NULL;

	if (!dev) {
		tloge("dev is invalid\n");
		return;
	}

	if (!if_exist_unclosed_session(dev))
		return;

	close_thread = kthread_create(close_session_thread_fn,
		dev, "close_fn_%6d", dev->dev_file_id);
	if (unlikely(IS_ERR_OR_NULL(close_thread))) {
		tloge("fail to create close session thread\n");
		return;
	}

	wake_up_process(close_thread);
	wait_for_completion(&dev->close_comp);
	tlogd("wait for completion success\n");
}

int tc_ns_close_session(struct tc_ns_dev_file *dev_file,
	const struct tc_ns_client_context *context)
{
	int ret = -EINVAL;
	struct tc_ns_service *service = NULL;
	struct tc_ns_session *session = NULL;

	if (!dev_file || !context) {
		tloge("invalid dev_file or context\n");
		return ret;
	}
	service = get_service(dev_file, context);
	if (!service) {
		tloge("invalid service\n");
		return ret;
	}
	/*
	 * session_id in tee is unique, but in concurrency scene
	 * same session_id may appear in tzdriver, put session_list
	 * add/del in service->operation_lock can avoid it.
	 */
	mutex_lock(&service->operation_lock);
	session = get_session(service, dev_file, context);
	if (session) {
		int ret2;
		mutex_lock(&session->ta_session_lock);
		ret2 = close_session(dev_file, session, context->uuid,
			(unsigned int)UUID_LEN, context->session_id);
		mutex_unlock(&session->ta_session_lock);
		if (ret2)
			tloge("close session smc failed!\n");
#ifdef CONFIG_AUTH_ENHANCE
		/* Clean this session secure information */
		if (memset_s(&session->secure_info,
			sizeof(session->secure_info),
			0, sizeof(session->secure_info)))
			tloge("close session memset error\n");
#endif
		mutex_lock(&service->session_lock);
		list_del(&session->head);
		mutex_unlock(&service->session_lock);

		put_session_struct(session);
		put_session_struct(session); /* pair with open session */

		ret = 0;
		mutex_lock(&dev_file->service_lock);
		del_service_from_dev(dev_file, service);
		mutex_unlock(&dev_file->service_lock);
	} else {
		tloge("invalid session\n");
	}
	mutex_unlock(&service->operation_lock);
	put_service_struct(service);
	return ret;
}

int tc_ns_send_cmd(struct tc_ns_dev_file *dev_file,
	struct tc_ns_client_context *context)
{
	int ret = -EINVAL;
	struct tc_ns_service *service = NULL;
	struct tc_ns_session *session = NULL;
	struct tc_call_params params = {
		dev_file, context, NULL, 0
	};

	if (!dev_file || !context) {
		tloge("invalid dev_file or context\n");
		return ret;
	}

	service = get_service(dev_file, context);
	if (service) {
		session = get_session(service, dev_file, context);
		put_service_struct(service);
		if (session) {
			tlogd("send cmd find session id %x\n",
				context->session_id);
			goto find_session;
		}
		tloge("can't find session\n");
	} else {
		tloge("can't find service\n");
	}

	return ret;
find_session:
	mutex_lock(&session->ta_session_lock);
	params.sess = session;
	ret = tc_client_call(&params);
	mutex_unlock(&session->ta_session_lock);
	put_session_struct(session);
	if (ret)
		tloge("smc call returns error, ret=0x%x\n", ret);
	return ret;
}

static int ioctl_session_send_cmd(struct tc_ns_dev_file *dev_file,
	struct tc_ns_client_context *context, void *argp)
{
	int ret;

	ret = tc_ns_send_cmd(dev_file, context);
	if (ret)
		tloge("send cmd failed ret is %d\n", ret);
	if (copy_to_user(argp, context, sizeof(*context))) {
		if (!ret)
			ret = -EFAULT;
	}
	return ret;
}

int tc_client_session_ioctl(struct file *file, unsigned int cmd,
	 unsigned long arg)
{
	int ret = -EINVAL;
	void *argp = (void __user *)(uintptr_t)arg;
	struct tc_ns_dev_file *dev_file = NULL;
	struct tc_ns_client_context context;

	if (!argp || !file) {
		tloge("invalid params\n");
		return -EINVAL;
	}

	dev_file = file->private_data;
	if (copy_from_user(&context, argp, sizeof(context))) {
		tloge("copy from user failed\n");
		return -EFAULT;
	}

	context.returns.origin = TEEC_ORIGIN_COMMS;
	switch (cmd) {
	case TC_NS_CLIENT_IOCTL_SES_OPEN_REQ:
		ret = tc_ns_open_session(dev_file, &context);
		if (ret)
			tloge("open session failed ret is %d\n", ret);
		if (copy_to_user(argp, &context, sizeof(context)) && !ret)
			ret = -EFAULT;
		break;
	case TC_NS_CLIENT_IOCTL_SES_CLOSE_REQ:
		ret = tc_ns_close_session(dev_file, &context);
		break;
	case TC_NS_CLIENT_IOCTL_SEND_CMD_REQ:
		ret = ioctl_session_send_cmd(dev_file, &context, argp);
		break;
	default:
		tloge("invalid cmd:0x%x!\n", cmd);
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
		if (cmd == TC_NS_CLIENT_IOCTL_SES_CLOSE_REQ)
			ret = -EINTR;
		else
			return restart_syscall();
	}
	return ret;
}
