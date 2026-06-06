/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: Alloc global operation and pass params to TEE.
 * Author: qiqingchao  q00XXXXXX
 * Create: 2016-06-21
 */
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/version.h>
#include <asm/memory.h>

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/dma-buf.h>
#endif

#include <linux/hisi/hisi_ion.h>
#include <linux/ion.h>
#include "securec.h"

#include "smc.h"
#include "teek_client_constants.h"
#include "tc_ns_client.h"
#include "teek_ns_client.h"
#include "agent.h"
#include "tc_ns_log.h"
#include "gp_ops.h"
#include "mem.h"
#include "mailbox_mempool.h"
#include "dynamic_mem.h"
#include "tlogger.h"
#include "cfc.h"
#include "tc_client_driver.h"


#include "security_auth_enhance.h"
#define AES_LOGIN_MAXLEN   ((MAX_PUBKEY_LEN) > (MAX_PACKAGE_NAME_LEN) ? \
	(MAX_PUBKEY_LEN) : (MAX_PACKAGE_NAME_LEN))
static int do_encryption(uint8_t *buffer, uint32_t buffer_size,
	uint32_t payload_size, uint8_t *key);
static int encrypt_login_info(uint8_t flags, uint32_t cmd_id, int32_t index,
	uint32_t login_info_size, uint8_t *buffer, uint8_t *key);

typedef struct {
	tc_ns_dev_file *dev_file;
	tc_ns_client_context *client_context;
	tc_ns_session *session;
	tc_ns_operation *operation;
	tc_ns_temp_buf *local_temp_buffer;
	unsigned int *trans_paramtype_to_tee;
} alloc_params;

#define MAX_SHARED_SIZE 0x100000	/* 1 MiB */
#define TEEC_PARAM_TYPES(param0_type, param1_type, param2_type, param3_type) \
	((param3_type) << 12 | (param2_type) << 8 | \
	 (param1_type) << 4 | (param0_type))

#define TEEC_PARAM_TYPE_GET(param_types, index) \
	(((param_types) >> (4*(index))) & 0x0F)

#define ROUND_UP(N, S) (((N)+(S)-1)&(~((S)-1)))

static int free_operation(const tc_ns_client_context *client_context,
	tc_ns_operation *operation, tc_ns_temp_buf local_temp_buffer[4]);

static int check_user_param_value(tc_ns_client_context *client_context,
	int n)
{
	bool check_value = (n < 0) || (n > 3);

	if (client_context == NULL) {
		tloge("client_context is null.\n");
		return -EINVAL;
	}
	if (check_value == true) {
		tloge("n is invalid.\n");
		return -EINVAL;
	}
	return 0;
}

int tc_user_param_valid(tc_ns_client_context *client_context, int n)
{
	tc_ns_client_param *client_param = NULL;
	unsigned int param_type;
	bool check_mem = false;
	bool check_value = false;
	int check_result = check_user_param_value(client_context, n);

	if (check_result != 0)
		return check_result;
	client_param = &(client_context->params[n]);
	param_type = TEEC_PARAM_TYPE_GET(client_context->param_types, n);
	tlogd("Param %d type is %x\n", n, param_type);
	if (param_type == TEEC_NONE) {
		tlogd("param_type is TEEC_NONE.\n");
		return 0;
	}
	/* temp buffers we need to allocate/deallocate for every operation */
	check_mem = (param_type == TEEC_MEMREF_TEMP_INPUT) ||
		(param_type == TEEC_MEMREF_TEMP_OUTPUT) ||
		(param_type == TEEC_MEMREF_TEMP_INOUT) ||
		(param_type == TEEC_MEMREF_PARTIAL_INPUT) ||
		(param_type == TEEC_MEMREF_PARTIAL_OUTPUT) ||
		(param_type == TEEC_MEMREF_PARTIAL_INOUT);
	check_value = (param_type == TEEC_VALUE_INPUT) ||
		(param_type == TEEC_VALUE_OUTPUT) ||
		(param_type == TEEC_VALUE_INOUT) ||
		(param_type == TEEC_ION_INPUT);
	if (check_mem == true) {
		uint32_t size;
		/* Check the size and buffer addresses  have valid userspace addresses */
		if (!access_ok(VERIFY_READ, client_param->memref.size_addr,
			sizeof(uint32_t)))
			return -EFAULT;
		get_user(size, (uint32_t *) client_param->memref.size_addr);
		/* Check if the buffer address is valid user space address */
		if (!access_ok(VERIFY_READ, client_param->memref.buffer, size))
			return -EFAULT;
	} else if (check_value == true) {
		if (!access_ok(VERIFY_READ, client_param->value.a_addr,
			sizeof(uint32_t)))
			return -EFAULT;
		if (!access_ok(VERIFY_READ, client_param->value.b_addr,
			sizeof(uint32_t)))
			return -EFAULT;
	} else {
		tloge("param_types is not supported.\n");
		return -EFAULT;
	}
	return 0;
}

static int copy_from_client_for_kernel(const void __user *src,
	size_t src_size, void *dest, size_t dst_size)
{
	int s_ret = 0;
	bool check_value = false;

	check_value = virt_addr_valid(src) || vmalloc_addr_valid(src) ||
		modules_addr_valid(src);
	if (check_value == true) {
		s_ret = memcpy_s(dest, dst_size, src, src_size);
		if (s_ret != EOK) {
			tloge("copy_from_client _s fail. line=%d, s_ret=%d.\n",
				__LINE__, s_ret);
			return s_ret;
		}
		return 0;
	}
	tloge("copy_from_client check kernel addr %llx failed.\n",
		(uint64_t)src);
	return  -EFAULT;
}

static int copy_from_client_for_usr(const void __user *src, size_t size,
	void *dest)
{
	if (copy_from_user(dest, src, size)) {
		tloge("copy_from_user failed.\n");
		return -EFAULT;
	}
	return 0;
}

/* These 2 functions handle copying from client. Because client here can be
 * kernel client or user space client, we must use the proper copy function
 */
static inline int copy_from_client(void *dest, const void __user *src,
	size_t size, uint8_t kernel_api)
{
	int ret = 0;
	bool check_value = false;
	size_t src_size;
	size_t dst_size;

	check_value = (dest == NULL) || (src == NULL);
	if (check_value == true) {
		tloge("src or dest is NULL input buffer\n");
		return -EINVAL;
	}
	/*
	 * size is checked before calling copy_from_client
	 * to be sure that size is <= dest's buffer size.
	 */
	if (size == 0)
		return 0;
	src_size = size;
	dst_size = size;

	if (kernel_api) {
		/* Source is kernel valid address */
		ret = copy_from_client_for_kernel(src, src_size, dest, dst_size);
	} else {
		/* buffer is in user space(CA call TEE API) */
		ret = copy_from_client_for_usr(src, size, dest);
	}
	return ret;
}

static int copy_to_client_for_kernel(const void __user *src,
	size_t src_size, void *dest, size_t dst_size)
{
	int s_ret = 0;
	bool check_value = false;

	check_value = virt_addr_valid(dest) || vmalloc_addr_valid(dest) ||
		modules_addr_valid(dest);
	if (check_value == true) {
		s_ret = memcpy_s(dest, dst_size, src, src_size);
		if (s_ret != EOK) {
			tloge("copy_to_client _s fail. line=%d, s_ret=%d.\n",
			      __LINE__, s_ret);
			return s_ret;
		}
	} else {
		tloge("copy_to_client check dst addr %llx failed.\n",
			(uint64_t)dest);
		return -EFAULT;
	}
	return 0;
}

static int copy_to_client_for_usr(const void __user *src, size_t size,
	void *dest)
{
	if (copy_to_user(dest, src, size))
		return -EFAULT;
	return 0;
}

static inline int copy_to_client(void __user *dest, const void *src,
	size_t size, uint8_t kernel_api)
{
	int ret = 0;
	bool check_value = false;
	size_t src_size;
	size_t dst_size;

	check_value = (dest == NULL) || (src == NULL);
	if (check_value == true) {
		tloge("src or dest is NULL input buffer\n");
		return -EINVAL;
	}
	if (size == 0)
		return ret;
	src_size = size;
	dst_size = size;
	/* Dest is kernel valid address */
	if (kernel_api) {
		ret = copy_to_client_for_kernel(src, src_size, dest, dst_size);
	} else {
		/* buffer is in user space(CA call TEE API) */
		ret = copy_to_client_for_usr(src, size, dest);
	}
	return ret;
}

static int check_params_for_alloc(tc_ns_dev_file *dev_file,
	tc_ns_session *session, tc_ns_operation *operation,
	tc_ns_temp_buf *local_temp_buffer)
{
	if (dev_file == NULL) {
		tloge("dev_file is null");
		return -EINVAL;
	}
	if (session == NULL) {
		tloge("session is null\n");
		return -EINVAL;
	}
	if (operation == NULL) {
		tloge("operation is null\n");
		return -EINVAL;
	}
	if (local_temp_buffer == NULL) {
		tloge("local_temp_buffer is null");
		return -EINVAL;
	}
	return 0;
}

static int check_context_for_alloc(tc_ns_client_context *client_context)
{
	if (client_context == NULL) {
		tloge("client_context is null");
		return -EINVAL;
	}
	if (!client_context->param_types) {
		tloge("invalid param type\n");
		return -EINVAL;
	}
	return 0;
}

static bool judge_for_open_session(uint8_t flags, int index)
{
	/*
	 * Normally kernel_params = kernel_api
	 * But when TC_CALL_LOGIN, params 2/3 will
	 * be filled by kernel. so under this circumstance,
	 * params 2/3 has to be set to kernel mode; and
	 * param 0/1 will keep the same with kernel_api.
	 */
	bool check_value = (flags & TC_CALL_LOGIN) && (index >= 2);
	return check_value;
}

static int alloc_for_tmp_mem(alloc_params params_in,
	uint8_t kernel_params, unsigned int param_type, uint8_t flags, int i)
{
	tc_ns_client_context *client_context = params_in.client_context;
	tc_ns_temp_buf *local_temp_buffer = params_in.local_temp_buffer;
	tc_ns_operation *operation = params_in.operation;
	tc_ns_session *session = params_in.session;
	unsigned int *trans_paramtype_to_tee = params_in.trans_paramtype_to_tee;
	tc_ns_client_param *client_param = NULL;
	void *temp_buf = NULL;
	uint32_t buffer_size = 0;
	bool check_value = 0;
	int ret = 0;
	/* For interface compatibility sake we assume buffer size to be 32bits */
	client_param = &(client_context->params[i]);
	if (copy_from_client(&buffer_size,
		(uint32_t __user *) client_param->memref.size_addr,
		sizeof(uint32_t), kernel_params)) {
		tloge("copy memref.size_addr failed\n");
		return -EFAULT;
	}
	/* Don't allow unbounded malloc requests */
	if (buffer_size > MAX_SHARED_SIZE) {
		tloge("buffer_size %u from user is too large\n", buffer_size);
		return -EFAULT;
	}
	temp_buf = (void *)mailbox_alloc(buffer_size, MB_FLAG_ZERO);
	/* If buffer size is zero or malloc failed */
	if (temp_buf == NULL) {
		tloge("temp_buf malloc failed, i = %d.\n", i);
		return -ENOMEM;
	} else {
		tlogd("temp_buf malloc ok, i = %d.\n", i);
	}
	local_temp_buffer[i].temp_buffer = temp_buf;
	local_temp_buffer[i].size = buffer_size;
	check_value = (param_type == TEEC_MEMREF_TEMP_INPUT) ||
		(param_type == TEEC_MEMREF_TEMP_INOUT);
	if (check_value == true) {
		tlogv("client_param->memref.buffer=0x%llx\n",
			client_param->memref.buffer);
		/* Kernel side buffer */
		if (copy_from_client(temp_buf,
			(void *)client_param->memref.buffer, buffer_size,
			kernel_params)) {
			tloge("copy memref.buffer failed\n");
			return -EFAULT;
		}
	}
	ret = encrypt_login_info(flags, client_context->cmd_id, i,
		buffer_size, temp_buf, session->secure_info.crypto_info.key);
	if (ret != 0) {
		tloge("SECURITY_AUTH_ENHANCE:encry failed\n");
		return ret;
	}
	operation->params[i].memref.buffer = virt_to_phys((void *)temp_buf);
	operation->buffer_h_addr[i] = virt_to_phys((void *)temp_buf) >> 32;
	operation->params[i].memref.size = buffer_size;
	/* TEEC_MEMREF_TEMP_INPUT equal to TEE_PARAM_TYPE_MEMREF_INPUT */
	trans_paramtype_to_tee[i] = param_type;
	return ret;
}

static int check_buffer_for_ref(uint32_t *buffer_size,
	tc_ns_client_param *client_param, uint8_t kernel_params)
{
	if (copy_from_client(buffer_size,
		(uint32_t __user *)client_param->memref.size_addr,
		sizeof(uint32_t), kernel_params)) {
		tloge("copy memref.size_addr failed\n");
		return -EFAULT;
	}
	if (*buffer_size == 0) {
		tloge("buffer_size from user is 0\n");
		return -ENOMEM;
	}
	return 0;
}
static int alloc_for_ref_mem(alloc_params params_in,
	uint8_t kernel_params, unsigned int param_type, int i)
{
	tc_ns_client_context *client_context = params_in.client_context;
	tc_ns_operation *operation = params_in.operation;
	unsigned int *trans_paramtype_to_tee = params_in.trans_paramtype_to_tee;
	tc_ns_dev_file *dev_file = params_in.dev_file;
	tc_ns_client_param *client_param = NULL;
	tc_ns_shared_mem *shared_mem = NULL;
	uint32_t buffer_size = 0;
	bool check_value = 0;
	int ret = 0;

	client_param = &(client_context->params[i]);
	ret = check_buffer_for_ref(&buffer_size, client_param, kernel_params);
	if (ret != 0)
		return ret;
	operation->params[i].memref.buffer = 0;
	/* find kernel addr refered to user addr */
	mutex_lock(&dev_file->shared_mem_lock);
	list_for_each_entry(shared_mem, &dev_file->shared_mem_list, head) {
		if (shared_mem->user_addr ==
			(void *)client_param->memref.buffer) {
			/* arbitrary CA can control offset by ioctl, so in here
			 * offset must be checked, and avoid integer overflow.
			 */
			check_value = ((shared_mem->len - client_param->memref.offset) >= buffer_size) &&
				(shared_mem->len > client_param->memref.offset);
			if (check_value == true) {
				void *buffer_addr =
					(void *)((unsigned long)shared_mem->kernel_addr +
					client_param->memref.offset);
				if (!shared_mem->from_mailbox) {
					buffer_addr =
						mailbox_copy_alloc(buffer_addr,
						buffer_size);
					if (buffer_addr == NULL) {
						tloge("alloc mailbox copy failed\n");
						ret = -ENOMEM;
						break;
					}
					operation->mb_buffer[i] = buffer_addr;
				}
				operation->params[i].memref.buffer =
					virt_to_phys(buffer_addr);
				operation->buffer_h_addr[i] =
					virt_to_phys(buffer_addr) >> 32;
				/* save shared_mem in operation
				 * so that we can use it while free_operation
				 */
				operation->sharemem[i] = shared_mem;
				get_sharemem_struct(shared_mem);
			} else {
				tloge("Unexpected size %u vs %u",
					shared_mem->len, buffer_size);
			}
			break;
		}
	}
	mutex_unlock(&dev_file->shared_mem_lock);
	/* for 8G physical memory device, there is a chance that
	 * operation->params[i].memref.buffer could be all 0,
	 * buffer_h_addr cannot be 0 in the same time.
	 */
	check_value = (!operation->params[i].memref.buffer) &&
		(!operation->buffer_h_addr[i]);
	if (check_value == true) {
		tloge("can not find shared buffer, exit\n");
		return -EINVAL;
	}
	operation->params[i].memref.size = buffer_size;
	/* Change TEEC_MEMREF_PARTIAL_XXXXX  to TEE_PARAM_TYPE_MEMREF_XXXXX */
	trans_paramtype_to_tee[i] = param_type -
		(TEEC_MEMREF_PARTIAL_INPUT - TEE_PARAM_TYPE_MEMREF_INPUT);
	return ret;
}

static int copy_for_value(alloc_params params_in, uint8_t kernel_params,
	unsigned int param_type, int i)
{
	tc_ns_operation *operation = params_in.operation;
	unsigned int *trans_paramtype_to_tee = params_in.trans_paramtype_to_tee;
	tc_ns_client_context *client_context = params_in.client_context;
	int ret = 0;
	tc_ns_client_param *client_param = NULL;

	client_param = &(client_context->params[i]);
	if (copy_from_client(&operation->params[i].value.a,
		client_param->value.a_addr, sizeof(operation->params[i].value.a),
		kernel_params)) {
		tloge("copy value.a_addr failed\n");
		return -EFAULT;
	}
	if (copy_from_client(&operation->params[i].value.b,
		client_param->value.b_addr, sizeof(operation->params[i].value.b),
		kernel_params)) {
		tloge("copy value.b_addr failed\n");
		return -EFAULT;
	}
	/* TEEC_VALUE_INPUT equal
	 * to TEE_PARAM_TYPE_VALUE_INPUT
	 */
	trans_paramtype_to_tee[i] = param_type;
	return ret;
}

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
static int ion_get_handle(struct ion_handle **drm_ion_handle,
	unsigned int ion_shared_fd)
{
#if (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE)
	*drm_ion_handle = ion_import_dma_buf_fd(g_drm_ion_client, ion_shared_fd);
#else
	*drm_ion_handle = ion_import_dma_buf(g_drm_ion_client, ion_shared_fd);
#endif
	if (IS_ERR(*drm_ion_handle)) {
		tloge("in %s err: fd=%d\n", __func__, ion_shared_fd);
		return -EFAULT;
	}
	return 0;
}
#endif

static int alloc_for_ion(alloc_params params_in, uint8_t kernel_params,
	unsigned int param_type, int i)
{
	tc_ns_operation *operation = params_in.operation;
	unsigned int *trans_paramtype_to_tee = params_in.trans_paramtype_to_tee;
	tc_ns_client_context *client_context = params_in.client_context;
	size_t drm_ion_size = 0;
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
	phys_addr_t drm_ion_phys = 0x0;
	struct dma_buf *drm_dma_buf = NULL;
#else
	ion_phys_addr_t drm_ion_phys = 0x0;
	struct ion_handle *drm_ion_handle = NULL;
#endif
	int ret = 0;
	tc_ns_client_param *client_param = NULL;
	unsigned int ion_shared_fd = 0;

	client_param = &(client_context->params[i]);
	if (copy_from_client(&operation->params[i].value.a,
		client_param->value.a_addr, sizeof(operation->params[i].value.a),
		kernel_params)) {
		tloge("value.a_addr copy failed\n");
		return -EFAULT;
	}
	if (copy_from_client(&operation->params[i].value.b,
		client_param->value.b_addr, sizeof(operation->params[i].value.b),
		kernel_params)) {
		tloge("value.b_addr copy failed\n");
		return -EFAULT;
	}
	if ((int)operation->params[i].value.a >= 0) {
		ion_shared_fd = operation->params[i].value.a;

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
		drm_dma_buf = dma_buf_get(ion_shared_fd);
		if (IS_ERR(drm_dma_buf)) {
			tloge("in %s err:drm_dma_buf is ERR, ret=%d fd=%d\n",
				__func__, ret, ion_shared_fd);
			return -EFAULT;
		}

		ret = ion_secmem_get_phys(drm_dma_buf, &drm_ion_phys,
			&drm_ion_size);
		if (ret) {
			tloge("in %s err:ret=%d fd=%d\n", __func__,
				ret, ion_shared_fd);
			return -EFAULT;
		}
#else
		ret = ion_get_handle(&drm_ion_handle, ion_shared_fd);
		if (ret != 0)
			return ret;
#if (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE)
		ret = ion_secmem_get_phys(g_drm_ion_client, drm_ion_handle,
			(phys_addr_t *)&drm_ion_phys, &drm_ion_size);
#else
		ret = ion_phys(g_drm_ion_client, drm_ion_handle, &drm_ion_phys,
			&drm_ion_size);
#endif
		if (ret) {
			ion_free(g_drm_ion_client, drm_ion_handle);
			tloge("in %s err:ret=%d fd=%d\n", __func__, ret,
				ion_shared_fd);
			return -EFAULT;
		}
#endif
		if (drm_ion_size > operation->params[i].value.b)
			drm_ion_size = operation->params[i].value.b;
		operation->params[i].memref.buffer = (unsigned int)drm_ion_phys;
		operation->params[i].memref.size = (unsigned int)drm_ion_size;
		trans_paramtype_to_tee[i] = param_type;

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
		ion_free(g_drm_ion_client, drm_ion_handle);
#else
		dma_buf_put(drm_dma_buf);
#endif
	} else {
		tloge("in %s err: drm ion handle invaild!\n", __func__);
		return -EFAULT;
	}
	return ret;
}

static int alloc_operation(tc_ns_dev_file *dev_file, tc_ns_session *session,
	tc_ns_operation *operation, tc_ns_client_context *client_context,
	tc_ns_temp_buf local_temp_buffer[4], uint8_t flags)
{
	int ret = 0;
	int i;
	unsigned int trans_paramtype_to_tee[4];
	uint8_t kernel_params;
	unsigned int param_type;
	tc_ns_client_param *client_param = NULL;
	alloc_params params_in = {
		dev_file, client_context, session, operation,
		local_temp_buffer, trans_paramtype_to_tee
	};

	ret = check_params_for_alloc(dev_file, session, operation,
		local_temp_buffer);
	if (ret != 0)
		return ret;
	ret = check_context_for_alloc(client_context);
	if (ret != 0)
		return ret;
	kernel_params = dev_file->kernel_api;
	tlogd("Allocating param types %08X\n", client_context->param_types);
	/* Get the 4 params from the client context */
	for (i = 0; i < 4; i++) {
		/*
		 * Normally kernel_params = kernel_api
		 * But when TC_CALL_LOGIN(open session), params 2/3 will
		 * be filled by kernel for authentication. so under this circumstance,
		 * params 2/3 has to be set to kernel mode for  authentication; and
		 * param 0/1 will keep the same with user_api.
		 */
		if (judge_for_open_session(flags, i))
			kernel_params = TEE_REQ_FROM_KERNEL_MODE;
		client_param = &(client_context->params[i]);
		param_type = TEEC_PARAM_TYPE_GET(client_context->param_types, i);
		tlogd("Param %d type is %x\n", i, param_type);
		if (teec_tmpmem_type(param_type, 2)) {
			/* temp buffers we need to allocate/deallocate
			 * for every operation
			 */
			ret = alloc_for_tmp_mem(params_in, kernel_params,
				param_type, flags, i);
			if (ret != 0)
				break;
		} else if (teec_memref_type(param_type, 2)) {
			/* MEMREF_PARTIAL buffers are already allocated so we just
			 * need to search for the shared_mem ref;
			 * For interface compatibility sake we assume buffer size to
			 * be 32bits
			 */
			ret = alloc_for_ref_mem(params_in, kernel_params,
				param_type, i);
			if (ret != 0)
				break;
		} else if (teec_value_type(param_type, 2)) {
			ret = copy_for_value(params_in, kernel_params,
				param_type, i);
			if (ret != 0)
				break;
		} else if (param_type == TEEC_ION_INPUT) {
			ret = alloc_for_ion(params_in, kernel_params,
				param_type, i);
			if (ret != 0)
				break;
		} else {
			tlogd("param_type = TEEC_NONE\n");
			trans_paramtype_to_tee[i] = TEE_PARAM_TYPE_NONE;
		}
	}
	if (!ret) {
		operation->paramtypes =
			TEEC_PARAM_TYPES(trans_paramtype_to_tee[0],
				trans_paramtype_to_tee[1],
				trans_paramtype_to_tee[2],
				trans_paramtype_to_tee[3]);
		return 0;
	}
	(void)free_operation(client_context, operation, local_temp_buffer);
	return ret;
}

static int check_params_for_update(tc_ns_dev_file *dev_file,
	tc_ns_temp_buf *local_temp_buffer)
{
	if (dev_file == NULL) {
		tloge("dev_file is null");
		return -EINVAL;
	}
	if (local_temp_buffer == NULL) {
		tloge("local_temp_buffer is null");
		return -EINVAL;
	}
	return 0;
}

static int update_for_tmp_mem(const tc_ns_dev_file *dev_file,
	tc_ns_client_context *client_context, tc_ns_operation *operation,
	tc_ns_temp_buf *local_temp_buffer, bool is_complete, int i)
{
	tc_ns_client_param *client_param = NULL;
	uint32_t buffer_size = 0;
	/* temp buffer */
	buffer_size = operation->params[i].memref.size;
	client_param = &(client_context->params[i]);
	/* Size is updated all the time */
	if (copy_to_client((void *)client_param->memref.size_addr,
		&buffer_size, sizeof(buffer_size),
		dev_file->kernel_api)) {
		tloge("copy tempbuf size failed\n");
		return -EFAULT;
	}
	/* Only update the buffer when the buffer size is valid in
	 * complete case, when the buffer size is invalid see next param
	 */
	if (buffer_size > local_temp_buffer[i].size) {
		if (!is_complete) {
			return 0;
		} else {
			/* operation is allocated from mailbox and share with gtask,
			 * so it's possible to be changed
			 */
			tloge("client_param->memref.size has been changed larger than the initial\n");
			return -EFAULT;
		}
	}
	if (copy_to_client((void *)client_param->memref.buffer,
		local_temp_buffer[i].temp_buffer,
		operation->params[i].memref.size, dev_file->kernel_api)) {
		tloge("copy tempbuf failed\n");
		return -ENOMEM;
	}
	return 0;
}

static int update_for_ref_mem(const tc_ns_dev_file *dev_file,
	tc_ns_client_context *client_context, tc_ns_operation *operation, int i)
{
	tc_ns_client_param *client_param = NULL;
	uint32_t buffer_size = 0;
	bool check_value = false;
	unsigned int orig_size = 0;
	/* update size */
	buffer_size = operation->params[i].memref.size;
	client_param = &(client_context->params[i]);
	if (copy_from_client(&orig_size,
		(uint32_t __user *)client_param->memref.size_addr,
		sizeof(orig_size), dev_file->kernel_api)) {
		tloge("copy orig memref.size_addr failed\n");
		return -EFAULT;
	}
	if (copy_to_client((void *)client_param->memref.size_addr,
		&buffer_size, sizeof(buffer_size), dev_file->kernel_api)) {
		tloge("copy buf size failed\n");
		return -EFAULT;
	}
	/* copy from mb_buffer to sharemem */
	check_value = !operation->sharemem[i]->from_mailbox &&
		operation->mb_buffer[i] &&
		orig_size >= buffer_size;
	if (check_value == true) {
		void *buffer_addr =
			(void *)((unsigned long)operation->sharemem[i]->kernel_addr +
			client_param->memref.offset);
		if (memcpy_s(buffer_addr,
			operation->sharemem[i]->len - client_param->memref.offset,
			operation->mb_buffer[i], buffer_size)) {
			tloge("copy to sharemem failed\n");
			return -EFAULT;
		}
	}
	return 0;
}

static int update_for_value(const tc_ns_dev_file *dev_file,
	tc_ns_client_context *client_context, tc_ns_operation *operation, int i)
{
	tc_ns_client_param *client_param = NULL;

	client_param = &(client_context->params[i]);
	if (copy_to_client(client_param->value.a_addr,
		&operation->params[i].value.a,
		sizeof(operation->params[i].value.a),
		dev_file->kernel_api)) {
		tloge("inc copy value.a_addr failed\n");
		return -EFAULT;
	}
	if (copy_to_client(client_param->value.b_addr,
		&operation->params[i].value.b,
		sizeof(operation->params[i].value.b),
		dev_file->kernel_api)) {
		tloge("inc copy value.b_addr failed\n");
		return -EFAULT;
	}
	return 0;
}

static int update_client_operation(tc_ns_dev_file *dev_file,
	tc_ns_client_context *client_context,
	tc_ns_operation *operation, tc_ns_temp_buf local_temp_buffer[4],
	bool is_complete)
{
	tc_ns_client_param *client_param = NULL;
	int ret = 0;
	unsigned int param_type;
	int i;

	ret = check_params_for_update(dev_file, local_temp_buffer);
	if (ret != 0)
		return -EINVAL;
	if (client_context == NULL) {
		tloge("client_context is null");
		return -EINVAL;
	}
	/* if paramTypes is NULL, no need to update */
	if (!client_context->param_types)
		return 0;
	for (i = 0; i < 4; i++) {
		client_param = &(client_context->params[i]);
		param_type = TEEC_PARAM_TYPE_GET(client_context->param_types, i);
		if (teec_tmpmem_type(param_type, 1)) {
			ret = update_for_tmp_mem(dev_file, client_context,
				operation, local_temp_buffer, is_complete, i);
			if (ret != 0)
				break;
		} else if (teec_memref_type(param_type, 1)) {
			ret = update_for_ref_mem(dev_file, client_context,
				operation, i);
			if (ret != 0)
				break;
		} else if (is_complete && teec_value_type(param_type, 1)) {
			ret = update_for_value(dev_file, client_context,
				operation, i);
			if (ret != 0)
				break;
		} else {
			tlogd("param_type:%d don't need to update.\n",
				param_type);
		}
	}
	return ret;
}

static int free_operation(const tc_ns_client_context *client_context,
	tc_ns_operation *operation, tc_ns_temp_buf local_temp_buffer[4])
{
	int ret = 0;
	unsigned int param_type;
	int i;
	void *temp_buf = NULL;
	bool check_temp_mem = false;
	bool check_part_mem = false;

	if (operation == NULL)
		return -EFAULT;
	if (client_context == NULL)
		return -EFAULT;
	if (local_temp_buffer == NULL) {
		tloge("local_temp_buffer is null");
		return -EINVAL;
	}
	for (i = 0; i < 4; i++) {
		param_type = TEEC_PARAM_TYPE_GET(client_context->param_types, i);
		check_temp_mem = param_type == TEEC_MEMREF_TEMP_INPUT ||
			param_type == TEEC_MEMREF_TEMP_OUTPUT ||
			param_type == TEEC_MEMREF_TEMP_INOUT;
		check_part_mem = param_type == TEEC_MEMREF_PARTIAL_INPUT ||
			param_type == TEEC_MEMREF_PARTIAL_OUTPUT ||
			param_type == TEEC_MEMREF_PARTIAL_INOUT;
		if (check_temp_mem == true) {
			/* free temp buffer */
			temp_buf = local_temp_buffer[i].temp_buffer;
			tlogd("Free temp buf, i = %d\n", i);
			if (virt_addr_valid(temp_buf) &&
				!ZERO_OR_NULL_PTR(temp_buf)) {
				mailbox_free(temp_buf);
				temp_buf = NULL;
			}
		} else if (check_part_mem == true) {
			put_sharemem_struct(operation->sharemem[i]);
			if (operation->mb_buffer[i])
				mailbox_free(operation->mb_buffer[i]);
		}
	}
	return ret;
}

unsigned char g_ca_auth_hash_buf[HASH_PLAINTEXT_ALIGNED_SIZE + IV_BYTESIZE];

static int32_t save_token_info(void *dst_teec, uint8_t *src_buf,
	uint8_t kernel_api)
{
	uint8_t temp_teec_token[TOKEN_SAVE_LEN];

	if (dst_teec == NULL || src_buf == NULL) {
		tloge("dst data or src data is invalid.\n");
		return -EINVAL;
	}
	/* copy libteec_token && timestamp to libteec */
	if (memmove_s(temp_teec_token, sizeof(temp_teec_token),
		src_buf, TIMESTAMP_SAVE_INDEX) != EOK) {
		tloge("copy teec token failed.\n");
		return -EFAULT;
	}
	if (memmove_s(&temp_teec_token[TIMESTAMP_SAVE_INDEX],
		TIMESTAMP_LEN_DEFAULT, &src_buf[TIMESTAMP_BUFFER_INDEX],
		TIMESTAMP_LEN_DEFAULT) != EOK) {
		tloge("copy teec timestamp failed.\n");
		return -EFAULT;
	}
	/* copy libteec_token to libteec */
	if (copy_to_client(dst_teec, temp_teec_token, TOKEN_SAVE_LEN,
		kernel_api) != EOK) {
		tloge("copy teec token & timestamp failed.\n");
		return -EFAULT;
	}
	/* clear libteec(16byte) */
	if (memset_s(src_buf, TIMESTAMP_SAVE_INDEX, 0,
		TIMESTAMP_SAVE_INDEX) != EOK) {
		tloge("clear libteec failed.\n");
		return -EFAULT;
	}
	return EOK;
}

static int check_params_for_fill_token(tc_ns_smc_cmd *smc_cmd,
	tc_ns_token *tc_token, uint8_t *mb_pack_token,
	uint32_t mb_pack_token_size, tc_ns_client_context *client_context)
{

	if (smc_cmd == NULL || tc_token == NULL || mb_pack_token == NULL ||
		client_context == NULL || mb_pack_token_size < TOKEN_BUFFER_LEN) {
		tloge("in parameter is ivalid.\n");
		return -EFAULT;
	}

	if (client_context->teec_token == NULL ||
		tc_token->token_buffer == NULL) {
		tloge("teec_token or token_buffer is NULL, error!\n");
		return -EFAULT;
	}
	return 0;
}

static int fill_token_info(tc_ns_smc_cmd *smc_cmd,
	tc_ns_client_context *client_context, tc_ns_token *tc_token,
	tc_ns_dev_file *dev_file, bool global, uint8_t *mb_pack_token,
	uint32_t mb_pack_token_size)
{
	uint8_t temp_libteec_token[TOKEN_SAVE_LEN] = {0};
	errno_t ret_s;
	int ret = 0;

	ret = check_params_for_fill_token(smc_cmd, tc_token, mb_pack_token,
		mb_pack_token_size, client_context);
	if (ret != 0)
		return ret;
	if ((client_context->cmd_id == GLOBAL_CMD_ID_CLOSE_SESSION) ||
		(!global)) {
		if (copy_from_client(temp_libteec_token,
			client_context->teec_token, TOKEN_SAVE_LEN,
			dev_file->kernel_api)) {
			tloge("copy libteec token failed!\n");
			return -EFAULT;
		}
		if (memcmp(&temp_libteec_token[TIMESTAMP_SAVE_INDEX],
			&tc_token->token_buffer[TIMESTAMP_BUFFER_INDEX],
			TIMESTAMP_LEN_DEFAULT)) {
			tloge("timestamp compare failed!\n");
			return -EFAULT;
		}
		/* combine token_buffer */
		/* teec_token, 0-15byte */
		if (memmove_s(tc_token->token_buffer,
			TIMESTAMP_SAVE_INDEX, temp_libteec_token,
			TIMESTAMP_SAVE_INDEX) != EOK) {
			tloge("copy buffer failed!\n");
			ret_s = memset_s(tc_token->token_buffer,
				TOKEN_BUFFER_LEN, 0, TOKEN_BUFFER_LEN);
			if (ret_s != EOK)
				tloge("memset_s buffer error=%d\n", ret_s);
			return -EFAULT;
		}
		/* kernal_api, 40byte */
		if (memmove_s((tc_token->token_buffer + KERNAL_API_INDEX),
			KERNAL_API_LEN, &dev_file->kernel_api,
			KERNAL_API_LEN) != EOK) {
			tloge("copy KERNAL_API_LEN failed!\n");
			ret_s = memset_s(tc_token->token_buffer,
				TOKEN_BUFFER_LEN, 0, TOKEN_BUFFER_LEN);
			if (ret_s != EOK)
				tloge("fill info memset_s error=%d\n", ret_s);
			return -EFAULT;
		}
	} else { /* open_session, set token_buffer 0 */
		if (memset_s(tc_token->token_buffer, TOKEN_BUFFER_LEN,
			0, TOKEN_BUFFER_LEN) != EOK) {
			tloge("alloc tc_ns_token->token_buffer error.\n");
			return -EFAULT;
		}
	}
	if (memcpy_s(mb_pack_token, mb_pack_token_size, tc_token->token_buffer,
		TOKEN_BUFFER_LEN)) {
		tloge("copy token failed\n");
		return -EFAULT;
	}
	smc_cmd->pid = current->tgid;
	smc_cmd->token_phys = virt_to_phys(mb_pack_token);
	smc_cmd->token_h_phys = virt_to_phys(mb_pack_token) >> 32;
	return EOK;
}

static int load_security_enhance_info(tc_ns_smc_cmd *smc_cmd,
	tc_ns_client_context *client_context, tc_ns_session *session,
	tc_ns_token *tc_token, tc_ns_dev_file *dev_file,
	struct mb_cmd_pack *mb_pack, bool global, bool is_token_work)
{
	int ret = 0;
	bool is_open_session_cmd = false;

	if (smc_cmd == NULL || mb_pack == NULL) {
		tloge("in parameter is invalid.\n");
		return -EFAULT;
	}
	if (is_token_work == true) {
		ret = fill_token_info(smc_cmd, client_context, tc_token,
			dev_file, global, mb_pack->token, sizeof(mb_pack->token));
		if (ret != EOK) {
			tloge("fill info failed. global=%d, cmd_id=%d, session_id=%d\n",
				global, smc_cmd->cmd_id, smc_cmd->context_id);
			return -EFAULT;
		}
	}
	is_open_session_cmd = global &&
		(smc_cmd->cmd_id == GLOBAL_CMD_ID_OPEN_SESSION);
	if (is_open_session_cmd) {
		if (session == NULL) {
			tloge("invalid session when load secure info\n");
			return -EFAULT;
		}
		if (generate_encrypted_session_secure_params(
			&session->secure_info, mb_pack->secure_params,
			sizeof(mb_pack->secure_params))) {
			tloge("Can't get encrypted session parameters buffer!");
			return -EFAULT;
		}
		smc_cmd->params_phys =
			virt_to_phys((void *)mb_pack->secure_params);
		smc_cmd->params_h_phys =
			virt_to_phys((void *)mb_pack->secure_params) >> 32;
	}
	return EOK;
}

static int check_params_for_append_token(
	const tc_ns_client_context *client_context,
	tc_ns_token *tc_token, const tc_ns_dev_file *dev_file)
{
	if (client_context == NULL || dev_file == NULL || tc_token == NULL) {
		tloge("in parameter is invalid.\n");
		return -EFAULT;
	}
	if (client_context->teec_token == NULL ||
	    tc_token->token_buffer == NULL) {
		tloge("teec_token or token_buffer is NULL, error!\n");
		return -EFAULT;
	}
	return 0;
}

static int append_teec_token(const tc_ns_client_context *client_context,
	tc_ns_token *tc_token, const tc_ns_dev_file *dev_file, bool global,
	uint8_t *mb_pack_token, uint32_t mb_pack_token_size)
{
	uint8_t temp_libteec_token[TOKEN_SAVE_LEN] = {0};
	int sret;
	int ret;

	ret = check_params_for_append_token(client_context, tc_token, dev_file);
	if (ret)
		return ret;
	if (!global) {
		if (copy_from_client(temp_libteec_token,
			client_context->teec_token, TOKEN_SAVE_LEN,
			dev_file->kernel_api)) {
			tloge("copy libteec token failed!\n");
			return -EFAULT;
		}
		/* combine token_buffer ,teec_token, 0-15byte */
		if (memmove_s(tc_token->token_buffer, TOKEN_BUFFER_LEN,
			temp_libteec_token, TIMESTAMP_SAVE_INDEX) != EOK) {
			tloge("copy temp_libteec_token failed!\n");
			sret = memset_s(tc_token->token_buffer,
				TOKEN_BUFFER_LEN, 0, TOKEN_BUFFER_LEN);
			if (sret != 0) {
				tloge("memset_s failed!\n");
				return -EFAULT;
			}
			return -EFAULT;
		}
		if (memcpy_s(mb_pack_token, mb_pack_token_size,
			tc_token->token_buffer, TOKEN_BUFFER_LEN)) {
			tloge("copy token failed\n");
			return -EFAULT;
		}
	}
	return EOK;
}

static int post_process_token(tc_ns_smc_cmd *smc_cmd,
	tc_ns_client_context *client_context, tc_ns_token *tc_token,
	uint8_t *mb_pack_token, uint32_t mb_pack_token_size,
	uint8_t kernel_api, bool global)
{
	int ret = 0;

	if (mb_pack_token == NULL || tc_token == NULL ||
	    client_context == NULL || tc_token->token_buffer == NULL ||
	    mb_pack_token_size < TOKEN_BUFFER_LEN) {
		tloge("in parameter is invalid.\n");
		return -EINVAL;
	}
	if (memcpy_s(tc_token->token_buffer, TOKEN_BUFFER_LEN, mb_pack_token,
		mb_pack_token_size)) {
		tloge("copy token failed\n");
		return -EFAULT;
	}
	if (memset_s(mb_pack_token, mb_pack_token_size, 0, mb_pack_token_size)) {
		tloge("memset mb_pack token error.\n");
		return -EFAULT;
	}
	sync_timestamp(smc_cmd, tc_token->token_buffer, client_context->uuid,
		global);
	ret = save_token_info(client_context->teec_token, tc_token->token_buffer,
		kernel_api);
	if (ret != EOK) {
		tloge("save_token_info  failed.\n");
		return -EFAULT;
	}
	return EOK;
}

#define TEE_TZMP \
{ \
	0xf8028dca,\
	0xaba0,\
	0x11e6,\
	{ \
		0x80, 0xf5, 0x76, 0x30, 0x4d, 0xec, 0x7e, 0xb7 \
	} \
}
#define INVALID_TZMP_UID   0xffffffff
static DEFINE_MUTEX(tzmp_lock);
static unsigned int tzmp_uid = INVALID_TZMP_UID;

int tzmp2_uid(const tc_ns_client_context *client_context,
	tc_ns_smc_cmd *smc_cmd, bool global)
{
	teec_uuid uuid_tzmp = TEE_TZMP;
	bool check_value = false;

	if (client_context == NULL || smc_cmd == NULL) {
		tloge("client_context or smc_cmd is null! ");
		return -EINVAL;
	}
	if (memcmp(client_context->uuid, (unsigned char *)&uuid_tzmp,
		sizeof(client_context->uuid)) == 0) {
		check_value = smc_cmd->cmd_id == GLOBAL_CMD_ID_OPEN_SESSION &&
			global;
		if (check_value == true) {
			/* save tzmp_uid */
			mutex_lock(&tzmp_lock);
			tzmp_uid = 0; /* for multisesion, we use same uid */
			smc_cmd->uid = 0;
			tlogv("openSession , tzmp_uid.uid is %d", tzmp_uid);
			mutex_unlock(&tzmp_lock);
			return EOK;
		}
		mutex_lock(&tzmp_lock);
		if (tzmp_uid == INVALID_TZMP_UID) {
			tloge("tzmp_uid.uid error!");
			mutex_unlock(&tzmp_lock);
			return -EFAULT;
		}
		smc_cmd->uid = tzmp_uid;
		tlogv("invokeCommand or closeSession , tzmp_uid is %d, global is %d",
			tzmp_uid, global);
		mutex_unlock(&tzmp_lock);
		return EOK;
	}
	return -EFAULT;
}

static int check_params_for_client_call(tc_ns_dev_file *dev_file,
	tc_ns_client_context *client_context)
{
	if (dev_file == NULL) {
		tloge("dev_file is null");
		return -EINVAL;
	}
	if (client_context == NULL) {
		tloge("client_context is null");
		return -EINVAL;
	}
	return 0;
}

static int alloc_for_client_call(tc_ns_smc_cmd **smc_cmd,
	struct mb_cmd_pack **mb_pack)
{
	*smc_cmd = kzalloc(sizeof(**smc_cmd), GFP_KERNEL);
	if (*smc_cmd == NULL) {
		tloge("smc_cmd malloc failed.\n");
		return -ENOMEM;
	}
	*mb_pack = mailbox_alloc_cmd_pack();
	if (*mb_pack == NULL) {
		kfree(*smc_cmd);
		return -ENOMEM;
	}
	return 0;
}

static void init_smc_cmd(tc_ns_dev_file *dev_file,
	tc_ns_client_context *client_context, tc_ns_smc_cmd *smc_cmd,
	struct mb_cmd_pack *mb_pack, bool global)
{
	smc_cmd->uuid_phys = virt_to_phys((void *)mb_pack->uuid);
	smc_cmd->uuid_h_phys = virt_to_phys((void *)mb_pack->uuid) >> 32;
	smc_cmd->cmd_id = client_context->cmd_id;
	smc_cmd->dev_file_id = dev_file->dev_file_id;
	smc_cmd->context_id = client_context->session_id;
	smc_cmd->err_origin = client_context->returns.origin;
	smc_cmd->started = client_context->started;
	tzmp2_uid(client_context, smc_cmd, global);
	tlogv("current uid is %d\n", smc_cmd->uid);
	if (client_context->param_types != 0) {
		smc_cmd->operation_phys = virt_to_phys(&mb_pack->operation);
		smc_cmd->operation_h_phys =
			virt_to_phys(&mb_pack->operation) >> 32;
	} else {
		smc_cmd->operation_phys = 0;
		smc_cmd->operation_h_phys = 0;
	}
	smc_cmd->login_method = client_context->login.method;
	return;
}

static int check_login_for_encrypt(tc_ns_client_context *client_context,
	tc_ns_session *session, tc_ns_smc_cmd *smc_cmd,
	struct mb_cmd_pack *mb_pack, int need_check_login)
{
	int ret = 0;

	if (need_check_login && session != NULL) {
		ret = do_encryption(session->ca_auth_hash_buf,
			sizeof(session->ca_auth_hash_buf),
			MAX_SHA_256_SZ,
			session->secure_info.crypto_info.key);
		if (ret) {
			tloge("hash encryption failed ret=%d\n", ret);
			return ret;
		}
		if (memcpy_s(mb_pack->login_data, sizeof(mb_pack->login_data),
			session->ca_auth_hash_buf,
			sizeof(session->ca_auth_hash_buf))) {
			tloge("copy login data failed\n");
			return -EFAULT;
		}
		smc_cmd->login_data_phy = virt_to_phys(mb_pack->login_data);
		smc_cmd->login_data_h_addr =
			virt_to_phys(mb_pack->login_data) >> 32;
		smc_cmd->login_data_len = MAX_SHA_256_SZ;
	} else {
		smc_cmd->login_data_phy = 0;
		smc_cmd->login_data_h_addr = 0;
		smc_cmd->login_data_len = 0;
	}
	return 0;
}

static void get_uid_for_cmd(uint32_t *uid)
{
#if (KERNEL_VERSION(3, 13, 0) <= LINUX_VERSION_CODE)
	kuid_t kuid;

	kuid.val = 0;
	kuid = current_uid(); /*lint !e666*/
	*uid = kuid.val;
#else
	*uid = current_uid();
#endif
}

static int proc_check_login_for_open_session(
	tc_ns_client_context *client_context, tc_ns_dev_file *dev_file,
	tc_ns_session *session, struct mb_cmd_pack *mb_pack, bool global,
	tc_ns_smc_cmd *smc_cmd)
{
	int ret = 0;
	int need_check_login = 0;
	int s_ret = 0;

	mb_pack->uuid[0] = ((true == global) ? 1 : 0);
	s_ret = memcpy_s(mb_pack->uuid + 1, UUID_LEN, client_context->uuid, 16);
	if (s_ret != EOK) {
		tloge("alloc_operation _s error\n");
		return -EFAULT;
	}
	init_smc_cmd(dev_file, client_context, smc_cmd, mb_pack, global);
	need_check_login = sizeof(uint32_t) == dev_file->pub_key_len &&
		GLOBAL_CMD_ID_OPEN_SESSION == smc_cmd->cmd_id &&
		(current->mm != NULL) && global;
	ret = check_login_for_encrypt(client_context, session, smc_cmd,
		mb_pack, need_check_login);
	if (ret != 0)
		return ret;
	smc_cmd->ca_pid = current->pid;
	smc_cmd->real_pid = current->pid;
	return ret;
}

static void reset_session_id(tc_ns_client_context *client_context,
	bool global, tc_ns_smc_cmd *smc_cmd, int tee_ret)
{
	int need_reset = 0;

	client_context->session_id = smc_cmd->context_id;
	// if tee_ret error except TEEC_PENDING,but context_id is seted,need to reset to 0.
	need_reset = global &&
		client_context->cmd_id == GLOBAL_CMD_ID_OPEN_SESSION &&
		tee_ret != 0 && TEEC_PENDING != tee_ret;
	if (need_reset)
		client_context->session_id = 0;
	return;
}

static void pend_ca_thread(tc_ns_session *session, tc_ns_smc_cmd *smc_cmd)
{
	struct tc_wait_data *wq = NULL;

	if (session != NULL)
		wq = &session->wait_data;
	if (wq != NULL) {
		tlogv("before wait event\n");
		/* use wait_event instead of wait_event_interruptible so
		 * that ap suspend will not wake up the TEE wait call
		 */
		wait_event(wq->send_cmd_wq, wq->send_wait_flag);
		wq->send_wait_flag = 0;
	}
	tlogv("operation start is :%d\n", smc_cmd->started);
	return;
}


static void proc_error_situation(tc_ns_client_context *client_context,
	struct mb_cmd_pack *mb_pack, tc_ns_smc_cmd *smc_cmd,
	int tee_ret, bool operation_init, tc_ns_temp_buf *local_temp_buffer)
{
	uint64_t phys_addr = smc_cmd->uuid_phys |
		(uint64_t)(((uint64_t)smc_cmd->uuid_h_phys) << 32);
	unsigned char *uuid_char = (void *)phys_to_virt(phys_addr);
	teec_uuid *uuid = (teec_uuid*)(uuid_char + 1);

	/* kfree(NULL) is safe and this check is probably not required */
	client_context->returns.code = tee_ret;
	client_context->returns.origin = smc_cmd->err_origin;
	/* when CA invoke command and crash,
	 * Gtask happen to release service node ,tzdriver need to kill ion;
	 * ta crash ,tzdriver also need to kill ion
	 */
	if (tee_ret == TEE_ERROR_TAGET_DEAD || tee_ret == TEEC_ERROR_GENERIC) {
		tloge("ta_crash or ca is killed or some error happen\n");
		kill_ion_by_uuid(uuid);
	}
	kfree(smc_cmd);
	if (operation_init)
		free_operation(client_context, &mb_pack->operation,
			local_temp_buffer);
	mailbox_free(mb_pack);
	return;
}

static void proc_short_buffer_situation(tc_ns_client_context *client_context,
	struct mb_cmd_pack *mb_pack, tc_ns_smc_cmd *smc_cmd,
	tc_ns_dev_file *dev_file, bool operation_init,
	tc_ns_temp_buf *local_temp_buffer)
{
	int ret = 0;
	/* update size */
	if (operation_init) {
		ret = update_client_operation(dev_file, client_context,
			&mb_pack->operation, local_temp_buffer, false);
		if (ret) {
			smc_cmd->err_origin = TEEC_ORIGIN_COMMS;
			return;
		}
	}
	return;
}

int tc_client_call(tc_ns_client_context *client_context,
	tc_ns_dev_file *dev_file, tc_ns_session *session, uint8_t flags)
{
	int ret = 0;
	int tee_ret = 0;
	tc_ns_smc_cmd *smc_cmd = NULL;
	tc_ns_temp_buf local_temp_buffer[4] = {
		{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
	};
	bool global = flags & TC_CALL_GLOBAL;
	uint32_t uid = 0;
	bool is_token_work = false;
	tc_ns_token *tc_token = session ? &session->tc_ns_token : NULL;
	struct mb_cmd_pack *mb_pack = NULL;
	bool operation_init = false;

	get_uid_for_cmd(&uid);
	ret = check_params_for_client_call(dev_file, client_context);
	if (ret != 0)
		return ret;
	if (client_context->cmd_id == GLOBAL_CMD_ID_OPEN_SESSION &&
		global == TC_CALL_GLOBAL)
		CFC_FUNC_ENTRY(tc_client_call);
	ret = alloc_for_client_call(&smc_cmd, &mb_pack);
	smc_cmd->uid = uid;
	if (ret != 0)
		return ret;
	if (client_context->param_types != 0) {
		ret = alloc_operation(dev_file, session, &mb_pack->operation,
			client_context, local_temp_buffer, flags);
		if (ret) {
			tloge("alloc_operation malloc failed");
			goto error;
		}
		operation_init = true;
	}
	ret = proc_check_login_for_open_session(client_context, dev_file,
		session, mb_pack, global, smc_cmd);
	if (ret != 0)
		goto error;
	is_token_work = (!global) ||
		(smc_cmd->cmd_id == GLOBAL_CMD_ID_OPEN_SESSION);
	ret = load_security_enhance_info(smc_cmd, client_context, session,
		tc_token, dev_file, mb_pack, global, is_token_work);
	if (ret != EOK) {
		tloge("load_security_enhance_info  failed.\n");
		goto error;
	}
	/* send smc to secure world */
	tee_ret = tc_ns_smc(smc_cmd);
	reset_session_id(client_context, global, smc_cmd, tee_ret);
	if (is_token_work) {
		ret = post_process_token(smc_cmd, client_context, tc_token,
			mb_pack->token, sizeof(mb_pack->token),
			dev_file->kernel_api, global);
		if (ret != EOK) {
			tloge("post_process_token failed.\n");
			smc_cmd->err_origin = TEEC_ORIGIN_COMMS;
			goto error;
		}
	}
	if (tee_ret != 0) {
		while (tee_ret == TEEC_PENDING) {
			pend_ca_thread(session, smc_cmd);
			if (is_token_work) {
				ret = append_teec_token(client_context,
					tc_token, dev_file, global,
					mb_pack->token, sizeof(mb_pack->token));
				if (ret != EOK) {
					tloge("append teec's member failed. global=%d, cmd_id=%d, session_id=%d\n",
					      global, smc_cmd->cmd_id, smc_cmd->context_id);
					goto error;
				}
			}
			tee_ret = tc_ns_smc_with_no_nr(smc_cmd);
			if (is_token_work) {
				ret = post_process_token(smc_cmd,
					client_context, tc_token,
					mb_pack->token, sizeof(mb_pack->token),
					dev_file->kernel_api, global);
				if (ret != EOK) {
					tloge("NO NR, post_process_token  failed.\n");
					goto error;
				}
			}
		}
		/* Client was interrupted, return and let it handle it's own signals first then retry */
		if (tee_ret == TEEC_CLIENT_INTR) {
			ret = -ERESTARTSYS;
			goto error;
		} else if (tee_ret) {
			tloge("smc_call returns error ret 0x%x\n", tee_ret);
			tloge("smc_call smc cmd ret 0x%x\n", smc_cmd->ret_val);
			goto short_buffer;
		}
		client_context->session_id = smc_cmd->context_id;
	}
	/* wake_up tee log reader */
	tz_log_write();
	if (operation_init) {
		ret = update_client_operation(dev_file, client_context,
			&mb_pack->operation, local_temp_buffer, true);
		if (ret) {
			smc_cmd->err_origin = TEEC_ORIGIN_COMMS;
			goto error;
		}
	}
	ret = 0;
	goto error;
short_buffer:
	if (tee_ret == TEEC_ERROR_SHORT_BUFFER)
		proc_short_buffer_situation(client_context, mb_pack, smc_cmd,
			dev_file, operation_init, local_temp_buffer);

	ret = EFAULT;
error:
	proc_error_situation(client_context, mb_pack, smc_cmd, tee_ret,
		operation_init, local_temp_buffer);
	return ret;
}

static bool is_opensession_by_index(uint8_t flags, uint32_t cmd_id,
	int i)
{
	/* params[2] for apk cert or native ca uid;
	 * params[3] for pkg name; therefore we set i>= 2
	 */
	bool global = flags & TC_CALL_GLOBAL;
	bool login_en = (global && (i >= 2) &&
		(cmd_id == GLOBAL_CMD_ID_OPEN_SESSION));
	return login_en;
}

static bool is_valid_size(uint32_t buffer_size, uint32_t temp_size)
{
	bool over_flow = false;

	if (buffer_size > AES_LOGIN_MAXLEN) {
		tloge("SECURITY_AUTH_ENHANCE: buffer_size is not right\n");
		return false;
	}
	over_flow = (temp_size > ROUND_UP(buffer_size, SZ_4K)) ? true : false;
	if (over_flow) {
		tloge("SECURITY_AUTH_ENHANCE: input data exceeds limit\n");
		return false;
	}
	return true;
}
static int check_param_for_encryption(uint8_t *buffer,
	uint32_t buffer_size, uint8_t **plaintext)
{
	if (buffer == NULL) {
		tloge("Do encryption buffer is null!\n");
		return -EINVAL;
	}
	*plaintext = buffer;
#if (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
	*plaintext = kzalloc(buffer_size, GFP_KERNEL);
	if (*plaintext == NULL) {
		tloge("malloc plaintext failed\n");
		return -ENOMEM;
	}
	if (memcpy_s(*plaintext, buffer_size, buffer, buffer_size)) {
		tloge("memcpy failed\n");
		kfree(*plaintext);
		return -EINVAL;
	}
#else

#endif
#endif
	return 0;
}

static int handle_end(uint8_t *plaintext, uint8_t *cryptotext, int ret)
{
#if (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE)

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
	kfree(plaintext);
#else
#endif

#endif
	if (cryptotext != NULL)
		kfree(cryptotext);
	return ret;
}

static int get_total_and_check(uint32_t *plaintext_size,
	uint32_t payload_size, uint32_t buffer_size,
	uint32_t *plaintext_aligned_size, uint32_t *total_size)
{
	int ret = 0;
	/* Payload + Head + Padding */
	*plaintext_size = payload_size + sizeof(struct encryption_head);
	*plaintext_aligned_size =
		ROUND_UP(*plaintext_size, CIPHER_BLOCK_BYTESIZE);
	/* Need 16 bytes to store AES-CBC iv */
	*total_size = *plaintext_aligned_size + IV_BYTESIZE;
	if (*total_size > buffer_size) {
		tloge("Do encryption buffer is not enough!\n");
		ret = -ENOMEM;
	}
	return ret;
}

static int do_encryption(uint8_t *buffer, uint32_t buffer_size,
	uint32_t payload_size, uint8_t *key)
{
	uint32_t plaintext_size, plaintext_aligned_size, total_size;
	uint8_t *cryptotext = NULL;
	uint8_t *plaintext = NULL;
	struct encryption_head head;
	int ret = check_param_for_encryption(buffer, buffer_size, &plaintext);

	if (ret != 0)
		return ret;
	ret = get_total_and_check(&plaintext_size, payload_size, buffer_size,
		&plaintext_aligned_size, &total_size);
	if (ret != 0)
		goto end;
	cryptotext = kzalloc(total_size, GFP_KERNEL);
	if (cryptotext == NULL) {
		tloge("Malloc failed when doing encryption!\n");
		ret = -ENOMEM;
		goto end;
	}
	/* Setting encryption head */
	ret = set_encryption_head(&head, plaintext, payload_size);
	if (ret) {
		tloge("Set encryption head failed, ret = %d.\n", ret);
		goto end;
	}
	ret = memcpy_s((void *)(plaintext + payload_size),
		buffer_size - payload_size, (void *)&head, sizeof(head));
	if (ret) {
		tloge("Copy encryption head failed, ret = %d.\n", ret);
		goto end;
	}
	/* Setting padding data */
	ret = crypto_aescbc_cms_padding(plaintext, plaintext_aligned_size,
		plaintext_size);
	if (ret) {
		tloge("Set encryption padding data failed, ret = %d.\n", ret);
		goto end;
	}
	ret = crypto_session_aescbc_key256(plaintext, plaintext_aligned_size,
		cryptotext, total_size, key, NULL, ENCRYPT);
	if (ret) {
		tloge("Encrypt failed, ret=%d.\n", ret);
		goto end;
	}
	ret = memcpy_s((void *)buffer, buffer_size, (void *)cryptotext,
		total_size);
	if (ret)
		tloge("Copy cryptotext failed, ret=%d.\n", ret);
end:
	return handle_end(plaintext, cryptotext, ret);
}

static int encrypt_login_info(uint8_t flags, uint32_t cmd_id, int32_t index,
	uint32_t login_info_size, uint8_t *buffer, uint8_t *key)
{
	uint32_t payload_size;
	uint32_t plaintext_size;
	uint32_t plaintext_aligned_size;
	uint32_t total_size;
	bool login_en;

	if (buffer == NULL) {
		tloge("Login information buffer is null!\n");
		return -EINVAL;
	}
	login_en = is_opensession_by_index(flags, cmd_id, index);
	if (!login_en)
		return 0;
	/* Need adding the termination null byte ('\0') to the end. */
	payload_size = login_info_size + sizeof(char);

	/* Payload + Head + Padding */
	plaintext_size = payload_size + sizeof(struct encryption_head);
	plaintext_aligned_size = ROUND_UP(plaintext_size, CIPHER_BLOCK_BYTESIZE);
	/* Need 16 bytes to store AES-CBC iv */
	total_size = plaintext_aligned_size + IV_BYTESIZE;
	if (!is_valid_size(login_info_size, total_size)) {
		tloge("Login information encryption size is invalid!\n");
		return -EFAULT;
	}
	return do_encryption(buffer, total_size, payload_size, key);
}
