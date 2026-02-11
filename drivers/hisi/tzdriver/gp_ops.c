/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 * Description: Alloc global operation and pass params to TEE.
 * Author: qiqingchao  q00XXXXXX
 * Create: 2016-06-21
 */
#include "gp_ops.h"
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
#include <linux/types.h>
#include <asm/memory.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/dma-buf.h>
#endif
#include <linux/hisi/hisi_ion.h>
#include <linux/ion.h>
#include <securec.h>
#include "teek_client_constants.h"
#include "agent.h"
#include "tc_ns_log.h"
#include "smc.h"
#include "mem.h"
#include "mailbox_mempool.h"
#include "dynamic_mem.h"
#include "tc_client_sub_driver.h"
#ifdef CONFIG_TEELOG
#include "tlogger.h"
#endif
#include "tc_client_driver.h"


#ifdef SECURITY_AUTH_ENHANCE
#include "security_auth_enhance.h"
#define AES_LOGIN_MAXLEN   ((MAX_PUBKEY_LEN) > (MAX_PACKAGE_NAME_LEN) ? \
	(MAX_PUBKEY_LEN) : (MAX_PACKAGE_NAME_LEN))
static int do_encryption(uint8_t *buffer, uint32_t buffer_size,
	uint32_t payload_size, uint8_t *key);
static int encrypt_login_info(uint32_t login_info_size, uint8_t *buffer,
	uint8_t *key);
#endif

typedef struct {
	tc_ns_dev_file *dev_file;
	tc_ns_client_context *client_context;
	tc_ns_session *session;
	tc_ns_temp_buf *local_temp_buffer;
	unsigned int tmp_buf_size;
} tc_call_params;

typedef struct {
	tc_ns_dev_file *dev_file;
	tc_ns_client_context *client_context;
	tc_ns_session *session;
	tc_ns_operation *operation;
	tc_ns_temp_buf *local_temp_buffer;
	unsigned int tmp_buf_size;
	unsigned int *trans_paramtype_to_tee;
	unsigned int trans_paramtype_size;
} alloc_params;

typedef struct {
	tc_ns_dev_file *dev_file;
	tc_ns_client_context *client_context;
	tc_ns_operation *operation;
	tc_ns_temp_buf *local_temp_buffer;
	unsigned int tmp_buf_size;
	bool is_complete;
} update_params;

#define MAX_SHARED_SIZE 0x100000      /* 1 MiB */
#define TEEC_PARAM_TYPES(param0_type, param1_type, param2_type, param3_type) \
	((param3_type) << 12 | (param2_type) << 8 | \
	 (param1_type) << 4 | (param0_type))

#define TEEC_PARAM_TYPE_GET(param_types, index) \
	(((param_types) >> ((index) << 2)) & 0x0F)

#define ROUND_UP(N, S) (((N)+(S)-1)&(~((S)-1)))

static void free_operation(tc_call_params *params, tc_ns_operation *operation);

/* dir: 0-inclue input, 1-include output, 2-both */
static inline bool teec_value_type(unsigned int type, int dir)
{
	return (((dir == 0 || dir == 2) && type == TEEC_VALUE_INPUT) ||
		((dir == 1 || dir == 2) && type == TEEC_VALUE_OUTPUT) ||
		type == TEEC_VALUE_INOUT) ? true : false;
}

static inline bool teec_tmpmem_type(unsigned int type, int dir)
{
	return (((dir == 0 || dir == 2) && type == TEEC_MEMREF_TEMP_INPUT) ||
		((dir == 1 || dir == 2) && type == TEEC_MEMREF_TEMP_OUTPUT) ||
		type == TEEC_MEMREF_TEMP_INOUT) ? true : false;
}

static inline bool teec_memref_type(unsigned int type, int dir)
{
	return (((dir == 0 || dir == 2) && type == TEEC_MEMREF_PARTIAL_INPUT) ||
		((dir == 1 || dir == 2) && type == TEEC_MEMREF_PARTIAL_OUTPUT) ||
		type == TEEC_MEMREF_PARTIAL_INOUT) ? true : false;
}

static int check_user_param_value(tc_ns_client_context *client_context,
	unsigned int index)
{
	if (client_context == NULL) {
		tloge("client_context is null.\n");
		return -EINVAL;
	}
	if (index > TEE_PARAM_FOUR) {
		tloge("index is invalid, index:%x.\n", index);
		return -EINVAL;
	}
	return 0;
}

int tc_user_param_valid(tc_ns_client_context *client_context,
	unsigned int index)
{
	tc_ns_client_param *client_param = NULL;
	unsigned int param_type;
	bool check_mem = false;
	bool check_value = false;
	int check_result = check_user_param_value(client_context, index);

	if (check_result != 0)
		return check_result;
	client_param = &(client_context->params[index]);
	param_type = TEEC_PARAM_TYPE_GET(client_context->param_types, index);
	tlogd("Param %d type is %x\n", index, param_type);
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
		(param_type == TEEC_ION_INPUT) ||
		(param_type == TEEC_ION_SGLIST_INPUT);
	if (check_mem == true) {
		uint32_t size;
		/* Check the size and buffer addresses  have valid userspace addresses */
		if (!access_ok(VERIFY_READ,
			(unsigned long)(uintptr_t)client_param->memref.size_addr,
			sizeof(uint32_t)))
			return -EFAULT;
		get_user(size,
			(uint32_t *)(uintptr_t)client_param->memref.size_addr);
		/* Check if the buffer address is valid user space address */
		if (!access_ok(VERIFY_READ,
			(unsigned long)(uintptr_t)client_param->memref.buffer,
			size))
			return -EFAULT;
	} else if (check_value == true) {
		if (!access_ok(VERIFY_READ,
			(unsigned long)(uintptr_t)client_param->value.a_addr,
			sizeof(uint32_t)))
			return -EFAULT;
		if (!access_ok(VERIFY_READ,
			(unsigned long)(uintptr_t)client_param->value.b_addr,
			sizeof(uint32_t)))
			return -EFAULT;
	} else {
		tloge("param_types is not supported.\n");
		return -EFAULT;
	}
	return 0;
}

static int copy_from_client_for_kernel(void *dest, size_t dst_size,
	const void __user *src, size_t size)
{
	int s_ret = 0;
	bool check_value = false;

	/* Source is kernel valid address */
	check_value = virt_addr_valid((unsigned long)(uintptr_t)src) ||
		vmalloc_addr_valid(src) || modules_addr_valid(src);
	if (check_value == true) {
		s_ret = memcpy_s(dest, dst_size, src, size);
		if (s_ret != EOK) {
			tloge("copy_from_client _s fail. line=%d, s_ret=%d.\n",
				__LINE__, s_ret);
			return s_ret;
		}
		return 0;
	}
	tloge("copy_from_client check kernel addr %llx failed.\n",
		(uint64_t)(uintptr_t)src);
	return  -EFAULT;
}

static int copy_from_client_for_usr(void *dest, const void __user *src,
	size_t size, size_t dest_size)
{
	if (size > dest_size) {
		tloge("size is larger than dest_size\n");
		return -EINVAL;
	}
	if (copy_from_user(dest, src, size)) {
		tloge("copy_from_user failed.\n");
		return -EFAULT;
	}
	return 0;
}

/* These 2 functions handle copying from client. Because client here can be
 * kernel client or user space client, we must use the proper copy function
 */
static inline int copy_from_client(void *dest, size_t dest_size,
	const void __user *src, size_t size, uint8_t kernel_api)
{
	int ret;
	bool check_value = false;

	check_value = (dest == NULL) || (src == NULL);
	if (check_value == true) {
		tloge("src or dest is NULL input buffer\n");
		return -EINVAL;
	}
	/*  to be sure that size is <= dest's buffer size. */
	if (size > dest_size) {
		tloge("size is larger than dest_size or size is 0\n");
		return -EINVAL;
	}
	if (size == 0)
		return 0;

	if (kernel_api) {
		ret = copy_from_client_for_kernel(dest, dest_size, src, size);
	} else {
		/* buffer is in user space(CA call TEE API) */
		ret = copy_from_client_for_usr(dest, src, size, dest_size);
	}
	return ret;
}

static int copy_to_client_for_kernel(void *dest, size_t dst_size,
	const void __user *src, size_t size)
{
	int s_ret = 0;
	bool check_value = false;
	/* Dest is kernel valid address */
	check_value = virt_addr_valid((unsigned long)(uintptr_t)dest) ||
		vmalloc_addr_valid(dest) || modules_addr_valid(dest);
	if (check_value == true) {
		s_ret = memcpy_s(dest, dst_size, src, size);
		if (s_ret != EOK) {
			tloge("copy_to_client _s fail. line=%d, s_ret=%d.\n",
			      __LINE__, s_ret);
			return s_ret;
		}
	} else {
		tloge("copy_to_client check dst addr %llx failed.\n",
			(uint64_t)(uintptr_t)dest);
		return -EFAULT;
	}
	return 0;
}

static int copy_to_client_for_usr(void *dest,
	const void __user *src, size_t size)
{
	if (copy_to_user(dest, src, size))
		return -EFAULT;
	return 0;
}

static inline int copy_to_client(void __user *dest, size_t dest_size,
	const void *src, size_t size, uint8_t kernel_api)
{
	int ret;
	bool check_value = false;

	check_value = (dest == NULL) || (src == NULL);
	if (check_value == true) {
		tloge("src or dest is NULL input buffer\n");
		return -EINVAL;
	}
	/*  to be sure that size is <= dest's buffer size. */
	if (size > dest_size) {
		tloge("size is larger than dest_size\n");
		return -EINVAL;
	}
	if (size == 0)
		return 0;

	if (kernel_api) {
		ret = copy_to_client_for_kernel(dest, dest_size, src, size);
	} else {
		/* buffer is in user space(CA call TEE API) */
		ret = copy_to_client_for_usr(dest, src, size);
	}
	return ret;
}

static int check_params_for_alloc(tc_call_params *params,
	tc_ns_operation *operation)
{
	if (params->dev_file == NULL) {
		tloge("dev_file is null");
		return -EINVAL;
	}
	if (params->session == NULL) {
		tloge("session is null\n");
		return -EINVAL;
	}
	if (operation == NULL) {
		tloge("operation is null\n");
		return -EINVAL;
	}
	if (params->local_temp_buffer == NULL) {
		tloge("local_temp_buffer is null");
		return -EINVAL;
	}
	if (params->tmp_buf_size != (unsigned int)TEE_PARAM_NUM) {
		tloge("tmp_buf_size is wrong");
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
	if (client_context->param_types == 0) {
		tloge("invalid param type\n");
		return -EINVAL;
	}
	return 0;
}

static void set_kernel_params_for_open_session(uint8_t flags,
	int index, uint8_t *kernel_params)
{
	/*
	 * Normally kernel_params = kernel_api
	 * But when TC_CALL_LOGIN, params 2/3 will
	 * be filled by kernel. so under this circumstance,
	 * params 2/3 has to be set to kernel mode; and
	 * param 0/1 will keep the same with kernel_api.
	 */
	bool check_value = (flags & TC_CALL_LOGIN) && (index >= TEE_PARAM_THREE);
	if (check_value)
		*kernel_params = TEE_REQ_FROM_KERNEL_MODE;
	return;
}

#ifdef SECURITY_AUTH_ENHANCE
static bool is_opensession_by_index(uint8_t flags, uint32_t cmd_id,
	int index);
#endif

static int check_size_for_alloc(alloc_params *params_in, unsigned int index)
{
	bool check_value = false;

	check_value = (params_in->trans_paramtype_size != TEE_PARAM_NUM ||
		params_in->tmp_buf_size != TEE_PARAM_NUM ||
		index >= TEE_PARAM_NUM);
	if (check_value == true) {
		tloge("buf size or params type or index is invalid.\n");
		return -EFAULT;
	}
	return 0;
}

static int alloc_for_tmp_mem(alloc_params *params_in,
	uint8_t kernel_params, unsigned int param_type, uint8_t flags,
	unsigned int index)
{
	tc_ns_client_context *client_context = params_in->client_context;
	tc_ns_temp_buf *local_temp_buffer = params_in->local_temp_buffer;
	tc_ns_operation *operation = params_in->operation;
	tc_ns_session *session = params_in->session;
	unsigned int *trans_paramtype_to_tee = params_in->trans_paramtype_to_tee;
	tc_ns_client_param *client_param = NULL;
	void *temp_buf = NULL;
	uint32_t buffer_size = 0;
	bool check_value = false;
	int ret = 0;

	if (check_size_for_alloc(params_in, index) != 0)
		return -EFAULT;
	/* For interface compatibility sake we assume buffer size to be 32bits */
	client_param = &(client_context->params[index]);
	if (copy_from_client(&buffer_size, sizeof(buffer_size),
		(uint32_t __user *)(uintptr_t)client_param->memref.size_addr,
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
		tloge("temp_buf malloc failed, i = %d.\n", index);
		return -ENOMEM;
	} else {
		tlogd("temp_buf malloc ok, i = %d.\n", index);
	}
	local_temp_buffer[index].temp_buffer = temp_buf;
	local_temp_buffer[index].size = buffer_size;
	check_value = (param_type == TEEC_MEMREF_TEMP_INPUT) ||
		(param_type == TEEC_MEMREF_TEMP_INOUT);
	if (check_value == true) {
		tlogv("client_param->memref.buffer=0x%llx\n",
			client_param->memref.buffer);
		/* Kernel side buffer */
		if (copy_from_client(temp_buf, buffer_size,
			(void *)(uintptr_t)client_param->memref.buffer,
			buffer_size, kernel_params)) {
			tloge("copy memref.buffer failed\n");
			return -EFAULT;
		}
	}
#ifdef SECURITY_AUTH_ENHANCE
	if (is_opensession_by_index(flags, client_context->cmd_id, index)
		== true) {
		ret = encrypt_login_info(buffer_size,
			temp_buf, session->secure_info.crypto_info.key);
		if (ret != 0) {
			tloge("SECURITY_AUTH_ENHANCE:encry failed\n");
			return ret;
		}
	}
#endif
	operation->params[index].memref.buffer = virt_to_phys((void *)temp_buf);
	operation->buffer_h_addr[index] =
		virt_to_phys((void *)temp_buf) >> ADDR_TRANS_NUM;
	operation->params[index].memref.size = buffer_size;
	/* TEEC_MEMREF_TEMP_INPUT equal to TEE_PARAM_TYPE_MEMREF_INPUT */
	trans_paramtype_to_tee[index] = param_type;
	return ret;
}

static int check_buffer_for_ref(uint32_t *buffer_size,
	tc_ns_client_param *client_param, uint8_t kernel_params)
{
	if (copy_from_client(buffer_size, sizeof(*buffer_size),
		(uint32_t __user *)(uintptr_t)client_param->memref.size_addr,
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
static int alloc_for_ref_mem(alloc_params *params_in,
	uint8_t kernel_params, unsigned int param_type, int index)
{
	tc_ns_client_context *client_context = params_in->client_context;
	tc_ns_operation *operation = params_in->operation;
	unsigned int *trans_paramtype_to_tee = params_in->trans_paramtype_to_tee;
	tc_ns_dev_file *dev_file = params_in->dev_file;
	tc_ns_client_param *client_param = NULL;
	tc_ns_shared_mem *shared_mem = NULL;
	uint32_t buffer_size = 0;
	bool check_value = false;
	int ret = 0;

	if (check_size_for_alloc(params_in, index) != 0)
		return -EFAULT;
	client_param = &(client_context->params[index]);
	ret = check_buffer_for_ref(&buffer_size, client_param, kernel_params);
	if (ret != 0)
		return ret;
	operation->params[index].memref.buffer = 0;
	/* find kernel addr refered to user addr */
	mutex_lock(&dev_file->shared_mem_lock);
	list_for_each_entry(shared_mem, &dev_file->shared_mem_list, head) {
		if (shared_mem->user_addr ==
			(void *)(uintptr_t)client_param->memref.buffer) {
			/* arbitrary CA can control offset by ioctl, so in here
			 * offset must be checked, and avoid integer overflow.
			 */
			check_value = ((shared_mem->len -
				client_param->memref.offset) >= buffer_size) &&
				(shared_mem->len > client_param->memref.offset);
			if (check_value == true) {
				void *buffer_addr =
					(void *)(uintptr_t)(
					(uintptr_t)shared_mem->kernel_addr +
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
					operation->mb_buffer[index] = buffer_addr;
				}
				operation->params[index].memref.buffer =
					virt_to_phys(buffer_addr);
				operation->buffer_h_addr[index] =
					virt_to_phys(buffer_addr) >> ADDR_TRANS_NUM;
				/* save shared_mem in operation
				 * so that we can use it while free_operation
				 */
				operation->sharemem[index] = shared_mem;
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
	check_value = (!operation->params[index].memref.buffer) &&
		(!operation->buffer_h_addr[index]);
	if (check_value == true) {
		tloge("can not find shared buffer, exit\n");
		return -EINVAL;
	}
	operation->params[index].memref.size = buffer_size;
	/* Change TEEC_MEMREF_PARTIAL_XXXXX  to TEE_PARAM_TYPE_MEMREF_XXXXX */
	trans_paramtype_to_tee[index] = param_type -
		(TEEC_MEMREF_PARTIAL_INPUT - TEE_PARAM_TYPE_MEMREF_INPUT);
	return ret;
}

static int copy_for_value(alloc_params *params_in, uint8_t kernel_params,
	unsigned int param_type, int index)
{
	tc_ns_operation *operation = params_in->operation;
	unsigned int *trans_paramtype_to_tee = params_in->trans_paramtype_to_tee;
	tc_ns_client_context *client_context = params_in->client_context;
	int ret = 0;
	tc_ns_client_param *client_param = NULL;

	if (check_size_for_alloc(params_in, index) != 0)
		return -EFAULT;
	client_param = &(client_context->params[index]);
	if (copy_from_client(&operation->params[index].value.a,
		sizeof(operation->params[index].value.a),
		(void *)(uintptr_t)client_param->value.a_addr,
		sizeof(operation->params[index].value.a),
		kernel_params)) {
		tloge("copy value.a_addr failed\n");
		return -EFAULT;
	}
	if (copy_from_client(&operation->params[index].value.b,
		sizeof(operation->params[index].value.b),
		(void *)(uintptr_t)client_param->value.b_addr,
		sizeof(operation->params[index].value.b),
		kernel_params)) {
		tloge("copy value.b_addr failed\n");
		return -EFAULT;
	}
	/* TEEC_VALUE_INPUT equal
	 * to TEE_PARAM_TYPE_VALUE_INPUT
	 */
	trans_paramtype_to_tee[index] = param_type;
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
	if (IS_ERR_OR_NULL(*drm_ion_handle)) {
		tloge("in %s err: fd=%d\n", __func__, ion_shared_fd);
		return -EFAULT;
	}
	return 0;
}
#endif

static int get_ion_sg_list_from_fd(uint32_t ion_shared_fd,
	uint32_t ion_alloc_size, phys_addr_t *sglist_table,
	size_t *ion_sglist_size)
{
	struct sg_table *ion_table = NULL;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	struct sglist *tmp_sglist = NULL;
	uint64_t ion_id = 0;
	enum SEC_SVC ion_type = 0;
	uint32_t ion_list_num = 0;
	uint32_t sglist_size;
	uint32_t i;

	int ret = secmem_get_buffer(ion_shared_fd, &ion_table, &ion_id, &ion_type);
	if (ret != 0) {
		tloge("get ion table failed. \n");
		return -EFAULT;
	}

	if (ion_type != SEC_DRM_TEE) {
		bool stat = (ion_table->nents <= 0 || ion_table->nents > MAX_ION_NENTS);
		if (stat)
			return -EFAULT;
		ion_list_num = (uint32_t)(ion_table->nents & INT_MAX);
		for_each_sg(ion_table->sgl, sg, ion_list_num, i) {
			if (sg == NULL) {
				tloge("an error sg when get ion sglist \n");
				return -EFAULT;
			}
		}
	}
	// ion_list_num is less than 1024, so sglist_size won't flow
	sglist_size = sizeof(struct ion_page_info) * ion_list_num +
		sizeof(*tmp_sglist);
	tmp_sglist = (tz_sg_list *)mailbox_alloc(sglist_size, MB_FLAG_ZERO);
	if (tmp_sglist == NULL) {
		tloge("sglist mem alloc failed.\n");
		return -ENOMEM;
	}
	tmp_sglist->sglist_size = (uint64_t)sglist_size;
	tmp_sglist->ion_size = (uint64_t)ion_alloc_size;
	tmp_sglist->info_length = (uint64_t)ion_list_num;
	if (ion_type != SEC_DRM_TEE) {
		for_each_sg(ion_table->sgl, sg, ion_list_num, i) {
			page = sg_page(sg);
			tmp_sglist->page_info[i].phys_addr = page_to_phys(page);
			tmp_sglist->page_info[i].npages = sg->length / PAGE_SIZE;
		}
	} else
		tmp_sglist->ion_id = ion_id;

	*sglist_table = virt_to_phys((void *)tmp_sglist);
	*ion_sglist_size = sglist_size;
	return 0;
}

static int alloc_for_ion_sglist(alloc_params *params_in, uint8_t kernel_params,
	unsigned int param_type, int index)
{
	tc_ns_operation *operation = params_in->operation;
	tc_ns_temp_buf *local_temp_buffer = params_in->local_temp_buffer;
	unsigned int *param_type_to_tee = params_in->trans_paramtype_to_tee;
	tc_ns_client_context *client_context = params_in->client_context;
	size_t ion_sglist_size = 0;
	phys_addr_t ion_sglist_addr = 0x0;
	int ret = 0;
	tc_ns_client_param *client_param = NULL;
	unsigned int ion_shared_fd = 0;
	unsigned int ion_alloc_size = 0;

	if (check_size_for_alloc(params_in, index) != 0)
		return -EFAULT;
	client_param = &(client_context->params[index]);
	if (copy_from_client(&operation->params[index].value.a,
		sizeof(operation->params[index].value.a),
		(void *)(uintptr_t)client_param->value.a_addr,
		sizeof(operation->params[index].value.a), kernel_params)) {
		tloge("value.a_addr copy failed\n");
		return -EFAULT;
	}
	if (copy_from_client(&operation->params[index].value.b,
		sizeof(operation->params[index].value.b),
		(void *)(uintptr_t)client_param->value.b_addr,
		sizeof(operation->params[index].value.b), kernel_params)) {
		tloge("value.b_addr copy failed\n");
		return -EFAULT;
	}
	if ((int)operation->params[index].value.a < 0) {
		tloge("drm ion handle invaild!\n");
		return -EFAULT;
	}
	ion_shared_fd = operation->params[index].value.a;
	ion_alloc_size = operation->params[index].value.b;

	ret = get_ion_sg_list_from_fd(ion_shared_fd, ion_alloc_size,
		&ion_sglist_addr, &ion_sglist_size);
	if (ret < 0) {
		tloge("get ion sglist failed, fd=%d\n", ion_shared_fd);
		return -EFAULT;
	}
	local_temp_buffer[index].temp_buffer = phys_to_virt(ion_sglist_addr);
	local_temp_buffer[index].size = ion_sglist_size;

	operation->params[index].memref.buffer = (unsigned int)ion_sglist_addr;
	operation->buffer_h_addr[index] =
		(unsigned int)(ion_sglist_addr >> ADDR_TRANS_NUM);
	operation->params[index].memref.size = (unsigned int)ion_sglist_size;
	param_type_to_tee[index] = param_type;
	return ret;
}

static int alloc_for_ion(alloc_params *params_in, uint8_t kernel_params,
	unsigned int param_type, int index)
{
	tc_ns_operation *operation = params_in->operation;
	unsigned int *trans_paramtype_to_tee = params_in->trans_paramtype_to_tee;
	tc_ns_client_context *client_context = params_in->client_context;
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

	if (check_size_for_alloc(params_in, index) != 0)
		return -EFAULT;
	client_param = &(client_context->params[index]);
	if (copy_from_client(&operation->params[index].value.a,
		sizeof(operation->params[index].value.a),
		(void *)(uintptr_t)client_param->value.a_addr,
		sizeof(operation->params[index].value.a),
		kernel_params)) {
		tloge("value.a_addr copy failed\n");
		return -EFAULT;
	}
	if (copy_from_client(&operation->params[index].value.b,
		sizeof(operation->params[index].value.b),
		(void *)(uintptr_t)client_param->value.b_addr,
		sizeof(operation->params[index].value.b),
		kernel_params)) {
		tloge("value.b_addr copy failed\n");
		return -EFAULT;
	}
	if ((int)operation->params[index].value.a >= 0) {
		ion_shared_fd = operation->params[index].value.a;

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
		drm_dma_buf = dma_buf_get(ion_shared_fd);
		if (IS_ERR_OR_NULL(drm_dma_buf)) {
			tloge("in %s err:drm_dma_buf is ERR, ret=%d fd=%d\n",
				__func__, ret, ion_shared_fd);
			return -EFAULT;
		}

		ret = ion_secmem_get_phys(drm_dma_buf, &drm_ion_phys,
			&drm_ion_size);
		if (ret) {
			tloge("in %s err:ret=%d fd=%d\n", __func__,
				ret, ion_shared_fd);
			dma_buf_put(drm_dma_buf);
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
		if (drm_ion_size > operation->params[index].value.b)
			drm_ion_size = operation->params[index].value.b;
		operation->params[index].memref.buffer = (unsigned int)drm_ion_phys;
		operation->params[index].memref.size = (unsigned int)drm_ion_size;
		trans_paramtype_to_tee[index] = param_type;

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

static int alloc_operation(tc_call_params *params,
	tc_ns_operation *operation, uint8_t flags)
{
	int ret;
	unsigned int index;
	unsigned int trans_paramtype_to_tee[TEE_PARAM_NUM] = { TEE_PARAM_TYPE_NONE };
	uint8_t kernel_params;
	unsigned int param_type;
	tc_ns_client_param *client_param = NULL;
	alloc_params params_in = {
		params->dev_file, params->client_context, params->session,
		operation, params->local_temp_buffer, TEE_PARAM_NUM,
		trans_paramtype_to_tee, TEE_PARAM_NUM,
	};
	ret = check_params_for_alloc(params, operation);
	if (ret != 0)
		return ret;
	ret = check_context_for_alloc(params->client_context);
	if (ret != 0)
		return ret;
	kernel_params = params->dev_file->kernel_api;
	tlogd("Allocating param types %08X\n",
		params->client_context->param_types);
	/* Get the 4 params from the client context */
	for (index = 0; index < TEE_PARAM_NUM; index++) {
		/*
		 * Normally kernel_params = kernel_api
		 * But when TC_CALL_LOGIN(open session), params 2/3 will
		 * be filled by kernel for authentication. so under this circumstance,
		 * params 2/3 has to be set to kernel mode for  authentication; and
		 * param 0/1 will keep the same with user_api.
		 */
		set_kernel_params_for_open_session(flags, index, &kernel_params);
		client_param = &(params->client_context->params[index]);
		param_type = TEEC_PARAM_TYPE_GET(
			params->client_context->param_types, index);
		tlogd("Param %d type is %x\n", index, param_type);
		if (teec_tmpmem_type(param_type, TEE_PARAM_THREE))
			/* temp buffers we need to allocate/deallocate
			 * for every operation
			 */
			ret = alloc_for_tmp_mem(&params_in, kernel_params,
				param_type, flags, index);
		else if (teec_memref_type(param_type, TEE_PARAM_THREE))
			/* MEMREF_PARTIAL buffers are already allocated so we just
			 * need to search for the shared_mem ref;
			 * For interface compatibility we assume buffer size to be 32bits
			 */
			ret = alloc_for_ref_mem(&params_in, kernel_params,
				param_type, index);
		else if (teec_value_type(param_type, TEE_PARAM_THREE))
			ret = copy_for_value(&params_in, kernel_params,
				param_type, index);
		else if (param_type == TEEC_ION_INPUT)
			ret = alloc_for_ion(&params_in, kernel_params,
				param_type, index);
		else if (param_type == TEEC_ION_SGLIST_INPUT)
			ret = alloc_for_ion_sglist(&params_in, kernel_params,
				param_type, index);
		else
			tlogd("param_type = TEEC_NONE\n");

		if (ret != 0)
			break;
	}
	if (ret != 0) {
		free_operation(params, operation);
		return ret;
	}
	operation->paramtypes =
		TEEC_PARAM_TYPES(trans_paramtype_to_tee[TEE_PARAM_ONE],
		trans_paramtype_to_tee[TEE_PARAM_TWO],
		trans_paramtype_to_tee[TEE_PARAM_THREE],
		trans_paramtype_to_tee[TEE_PARAM_FOUR]);
	return ret;
}

static int check_params_for_update(tc_call_params *in_params)
{
	if (in_params->dev_file == NULL) {
		tloge("dev_file is null");
		return -EINVAL;
	}
	if (in_params->client_context == NULL) {
		tloge("client_context is null");
		return -EINVAL;
	}
	if (in_params->local_temp_buffer == NULL) {
		tloge("local_temp_buffer is null");
		return -EINVAL;
	}
	if (in_params->tmp_buf_size != TEE_PARAM_NUM) {
		tloge("tmp_buf_size is invalid");
		return -EINVAL;
	}
	return 0;
}

static int update_for_tmp_mem(update_params *params_in, int index)
{
	tc_ns_client_param *client_param = NULL;
	uint32_t buffer_size;
	tc_ns_dev_file *dev_file = params_in->dev_file;
	tc_ns_client_context *client_context = params_in->client_context;
	tc_ns_operation *operation = params_in->operation;
	tc_ns_temp_buf *local_temp_buffer = params_in->local_temp_buffer;
	bool is_complete = params_in->is_complete;
	bool check_value = params_in->tmp_buf_size != TEE_PARAM_NUM ||
		index >= TEE_PARAM_NUM;

	if (check_value == true) {
		tloge("tmp_buf_size or index is invalid\n");
		return -EFAULT;
	}
	/* temp buffer */
	buffer_size = operation->params[index].memref.size;
	client_param = &(client_context->params[index]);
	/* Size is updated all the time */
	if (copy_to_client((void *)(uintptr_t)client_param->memref.size_addr,
		sizeof(buffer_size),
		&buffer_size, sizeof(buffer_size),
		dev_file->kernel_api)) {
		tloge("copy tempbuf size failed\n");
		return -EFAULT;
	}
	if (buffer_size > local_temp_buffer[index].size) {
		/* incomplete case, when the buffer size is invalid see next param*/
		if (!is_complete)
			return 0;
		/* complete case, operation is allocated from mailbox
		 *  and share with gtask, so it's possible to be changed
		 */
		tloge("client_param->memref.size has been changed larger than the initial\n");
		return -EFAULT;
	}
	/* Only update the buffer when the buffer size is valid in complete case */
	if (copy_to_client((void *)(uintptr_t)client_param->memref.buffer,
		operation->params[index].memref.size,
		local_temp_buffer[index].temp_buffer,
		operation->params[index].memref.size, dev_file->kernel_api)) {
		tloge("copy tempbuf failed\n");
		return -ENOMEM;
	}
	return 0;
}

static int update_for_ref_mem(update_params *params_in, int index)
{
	tc_ns_client_param *client_param = NULL;
	uint32_t buffer_size;
	bool check_value = false;
	unsigned int orig_size = 0;
	tc_ns_dev_file *dev_file = params_in->dev_file;
	tc_ns_client_context *client_context = params_in->client_context;
	tc_ns_operation *operation = params_in->operation;

	if (index >= TEE_PARAM_NUM) {
		tloge("index is invalid\n");
		return -EFAULT;
	}
	/* update size */
	buffer_size = operation->params[index].memref.size;
	client_param = &(client_context->params[index]);

	if (copy_from_client(&orig_size,
		sizeof(orig_size),
		(uint32_t __user *)(uintptr_t)client_param->memref.size_addr,
		sizeof(orig_size), dev_file->kernel_api)) {
		tloge("copy orig memref.size_addr failed\n");
		return -EFAULT;
	}
	if (copy_to_client((void *)(uintptr_t)client_param->memref.size_addr,
		sizeof(buffer_size),
		&buffer_size, sizeof(buffer_size), dev_file->kernel_api)) {
		tloge("copy buf size failed\n");
		return -EFAULT;
	}
	/* copy from mb_buffer to sharemem */
	check_value = !operation->sharemem[index]->from_mailbox &&
		operation->mb_buffer[index] && orig_size >= buffer_size;
	if (check_value == true) {
		void *buffer_addr =
			(void *)(uintptr_t)((uintptr_t)
			operation->sharemem[index]->kernel_addr +
			client_param->memref.offset);
		if (memcpy_s(buffer_addr,
			operation->sharemem[index]->len -
			client_param->memref.offset,
			operation->mb_buffer[index], buffer_size)) {
			tloge("copy to sharemem failed\n");
			return -EFAULT;
		}
	}
	return 0;
}

static int update_for_value(update_params *params_in, int index)
{
	tc_ns_client_param *client_param = NULL;
	tc_ns_dev_file *dev_file = params_in->dev_file;
	tc_ns_client_context *client_context = params_in->client_context;
	tc_ns_operation *operation = params_in->operation;

	if (index >= TEE_PARAM_NUM) {
		tloge("index is invalid\n");
		return -EFAULT;
	}
	client_param = &(client_context->params[index]);
	if (copy_to_client((void *)(uintptr_t)client_param->value.a_addr,
		sizeof(operation->params[index].value.a),
		&operation->params[index].value.a,
		sizeof(operation->params[index].value.a),
		dev_file->kernel_api)) {
		tloge("inc copy value.a_addr failed\n");
		return -EFAULT;
	}
	if (copy_to_client((void *)(uintptr_t)client_param->value.b_addr,
		sizeof(operation->params[index].value.b),
		&operation->params[index].value.b,
		sizeof(operation->params[index].value.b),
		dev_file->kernel_api)) {
		tloge("inc copy value.b_addr failed\n");
		return -EFAULT;
	}
	return 0;
}

static int update_client_operation(tc_call_params *params,
	tc_ns_operation *operation, bool is_complete)
{
	tc_ns_client_param *client_param = NULL;
	int ret;
	unsigned int param_type;
	unsigned int index;
	update_params params_in = { params->dev_file, params->client_context,
		operation, params->local_temp_buffer, TEE_PARAM_NUM,
		is_complete
	};
	ret = check_params_for_update(params);
	if (ret != 0)
		return -EINVAL;
	/* if paramTypes is NULL, no need to update */
	if (params->client_context->param_types == 0)
		return 0;
	for (index = 0; index < TEE_PARAM_NUM; index++) {
		client_param = &(params->client_context->params[index]);
		param_type = TEEC_PARAM_TYPE_GET(
			params->client_context->param_types, index);
		if (teec_tmpmem_type(param_type, TEE_PARAM_TWO))
			ret = update_for_tmp_mem(&params_in, index);
		else if (teec_memref_type(param_type, TEE_PARAM_TWO))
			ret = update_for_ref_mem(&params_in, index);
		else if (is_complete &&
			teec_value_type(param_type, TEE_PARAM_TWO))
			ret = update_for_value(&params_in, index);
		else
			tlogd("param_type:%d don't need to update.\n",
				param_type);
		if (ret != 0)
			break;
	}
	return ret;
}

static void free_operation(tc_call_params *params, tc_ns_operation *operation)
{
	unsigned int param_type;
	unsigned int index;
	void *temp_buf = NULL;
	bool check_temp_mem = false;
	bool check_part_mem = false;
	tc_ns_temp_buf *local_temp_buffer = params->local_temp_buffer;
	tc_ns_client_context *client_context = params->client_context;

	if (params->tmp_buf_size != TEE_PARAM_NUM)
		tloge("tmp_buf_size is invalid %x.\n", params->tmp_buf_size);
	for (index = 0; index < TEE_PARAM_NUM; index++) {
		param_type = TEEC_PARAM_TYPE_GET(
			client_context->param_types, index);
		check_temp_mem = param_type == TEEC_MEMREF_TEMP_INPUT ||
			param_type == TEEC_MEMREF_TEMP_OUTPUT ||
			param_type == TEEC_MEMREF_TEMP_INOUT;
		check_part_mem = param_type == TEEC_MEMREF_PARTIAL_INPUT ||
			param_type == TEEC_MEMREF_PARTIAL_OUTPUT ||
			param_type == TEEC_MEMREF_PARTIAL_INOUT;
		if (check_temp_mem == true) {
			/* free temp buffer */
			temp_buf = local_temp_buffer[index].temp_buffer;
			tlogd("Free temp buf, i = %d\n", index);
			if (virt_addr_valid((unsigned long)(uintptr_t)temp_buf) &&
				!ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)temp_buf)) {
				mailbox_free(temp_buf);
				temp_buf = NULL;
			}
		} else if (check_part_mem == true) {
			put_sharemem_struct(operation->sharemem[index]);
			if (operation->mb_buffer[index])
				mailbox_free(operation->mb_buffer[index]);
		} else if (param_type == TEEC_ION_SGLIST_INPUT) {
			temp_buf = local_temp_buffer[index].temp_buffer;
			tlogd("Free ion sglist buf, i = %d\n", index);
			if (virt_addr_valid((uint64_t)(uintptr_t)temp_buf) &&
				!ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)temp_buf)) {
				mailbox_free(temp_buf);
				temp_buf = NULL;
			}
		}
	}
}

#ifdef SECURITY_AUTH_ENHANCE
unsigned char g_auth_hash_buf[MAX_SHA_256_SZ * NUM_OF_SO + HASH_PLAINTEXT_ALIGNED_SIZE + IV_BYTESIZE];
#else
unsigned char g_auth_hash_buf[MAX_SHA_256_SZ * NUM_OF_SO + MAX_SHA_256_SZ];
#endif

#ifdef SECURITY_AUTH_ENHANCE
static int32_t save_token_info(void *dst_teec, uint32_t dst_size,
	uint8_t *src_buf, uint32_t src_size, uint8_t kernel_api)
{
	uint8_t temp_teec_token[TOKEN_SAVE_LEN] = {0};
	bool check_value = (dst_teec == NULL || src_buf == NULL ||
		dst_size != TOKEN_SAVE_LEN || src_size == 0);

	if (check_value == true) {
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
	if (copy_to_client(dst_teec, dst_size,
		temp_teec_token, TOKEN_SAVE_LEN,
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
	int ret;
	bool check_value = true;

	ret = check_params_for_fill_token(smc_cmd, tc_token, mb_pack_token,
		mb_pack_token_size, client_context);
	if (ret != 0)
		return ret;
	check_value = (client_context->cmd_id == GLOBAL_CMD_ID_CLOSE_SESSION) ||
		(!global);
	if (check_value == true) {
		if (copy_from_client(temp_libteec_token,
			TOKEN_SAVE_LEN,
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
		/* combine token_buffer teec_token, 0-15byte */
		if (memmove_s(tc_token->token_buffer,
			TIMESTAMP_SAVE_INDEX, temp_libteec_token,
			TIMESTAMP_SAVE_INDEX) != EOK) {
			tloge("copy buffer failed!\n");
			ret_s = memset_s(tc_token->token_buffer,
				tc_token->token_len, 0, TOKEN_BUFFER_LEN);
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
				tc_token->token_len, 0, TOKEN_BUFFER_LEN);
			if (ret_s != EOK)
				tloge("fill info memset_s error=%d\n", ret_s);
			return -EFAULT;
		}
	} else { /* open_session, set token_buffer 0 */
		if (memset_s(tc_token->token_buffer, tc_token->token_len,
			0, TOKEN_BUFFER_LEN) != EOK) {
			tloge("alloc tc_ns_token->token_buffer error.\n");
			return -EFAULT;
		}
	}
	if (memcpy_s(mb_pack_token, mb_pack_token_size, tc_token->token_buffer,
		tc_token->token_len)) {
		tloge("copy token failed\n");
		return -EFAULT;
	}
	smc_cmd->pid = current->tgid;
	smc_cmd->token_phys = virt_to_phys(mb_pack_token);
	smc_cmd->token_h_phys = virt_to_phys(mb_pack_token) >> ADDR_TRANS_NUM;
	return EOK;
}

static int load_security_enhance_info(tc_call_params *params,
	tc_ns_smc_cmd *smc_cmd, tc_ns_token *tc_token,
	struct mb_cmd_pack *mb_pack, bool global, bool is_token_work)
{
	int ret;
	bool is_open_session_cmd = false;
	tc_ns_dev_file *dev_file = params->dev_file;
	tc_ns_client_context *client_context = params->client_context;
	tc_ns_session *session = params->session;

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
			virt_to_phys((void *)mb_pack->secure_params) >> ADDR_TRANS_NUM;
	}
	return EOK;
}

static int check_params_for_append_token(
	const tc_ns_client_context *client_context,
	tc_ns_token *tc_token, const tc_ns_dev_file *dev_file,
	uint32_t mb_pack_token_size)
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
	if (mb_pack_token_size < TOKEN_BUFFER_LEN) {
		tloge("mb_pack_token_size is invalid.\n");
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

	ret = check_params_for_append_token(client_context, tc_token,
		dev_file, mb_pack_token_size);
	if (ret)
		return ret;
	if (!global) {
		if (copy_from_client(temp_libteec_token,
			TOKEN_SAVE_LEN,
			client_context->teec_token, TOKEN_SAVE_LEN,
			dev_file->kernel_api)) {
			tloge("copy libteec token failed!\n");
			return -EFAULT;
		}
		/* combine token_buffer ,teec_token, 0-15byte */
		if (memmove_s(tc_token->token_buffer, tc_token->token_len,
			temp_libteec_token, TIMESTAMP_SAVE_INDEX) != EOK) {
			tloge("copy temp_libteec_token failed!\n");
			sret = memset_s(tc_token->token_buffer,
				tc_token->token_len, 0, TOKEN_BUFFER_LEN);
			if (sret != 0) {
				tloge("memset_s failed!\n");
				return -EFAULT;
			}
			return -EFAULT;
		}
		if (memcpy_s(mb_pack_token, mb_pack_token_size,
			tc_token->token_buffer, tc_token->token_len)) {
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
	int ret;
	bool check_value = false;

	check_value = (mb_pack_token == NULL || tc_token == NULL ||
		client_context == NULL || client_context->teec_token == NULL ||
		tc_token->token_buffer == NULL ||
		mb_pack_token_size < TOKEN_BUFFER_LEN);
	if (check_value == true) {
		tloge("in parameter is invalid.\n");
		return -EINVAL;
	}
	if (memcpy_s(tc_token->token_buffer, tc_token->token_len, mb_pack_token,
		mb_pack_token_size)) {
		tloge("copy token failed\n");
		return -EFAULT;
	}
	if (memset_s(mb_pack_token, mb_pack_token_size, 0, mb_pack_token_size)) {
		tloge("memset mb_pack token error.\n");
		return -EFAULT;
	}
	if (sync_timestamp(smc_cmd, tc_token->token_buffer, tc_token->token_len, global)
		!= TEEC_SUCCESS) {
		tloge("sync time stamp error.\n");
		return -EFAULT;
	}

	ret = save_token_info(client_context->teec_token, client_context->token_len,
		tc_token->token_buffer, tc_token->token_len, kernel_api);
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
static unsigned int g_tzmp_uid = INVALID_TZMP_UID;

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
			g_tzmp_uid = 0; /* for multisesion, we use same uid */
			smc_cmd->uid = 0;
			tlogv("openSession , tzmp_uid.uid is %d", g_tzmp_uid);
			mutex_unlock(&tzmp_lock);
			return EOK;
		}
		mutex_lock(&tzmp_lock);
		if (g_tzmp_uid == INVALID_TZMP_UID) {
			tloge("tzmp_uid.uid error!");
			mutex_unlock(&tzmp_lock);
			return -EFAULT;
		}
		smc_cmd->uid = g_tzmp_uid;
		tlogv("invokeCommand or closeSession , tzmp_uid is %d, global is %d",
			g_tzmp_uid, global);
		mutex_unlock(&tzmp_lock);
		return EOK;
	}
	return EOK;
}
#endif

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
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)(*smc_cmd))) {
		tloge("smc_cmd malloc failed.\n");
		return -ENOMEM;
	}
	*mb_pack = mailbox_alloc_cmd_pack();
	if (*mb_pack == NULL) {
		kfree(*smc_cmd);
		*smc_cmd = NULL;
		return -ENOMEM;
	}
	return 0;
}

static int init_smc_cmd(tc_ns_dev_file *dev_file,
	tc_ns_client_context *client_context, tc_ns_smc_cmd *smc_cmd,
	struct mb_cmd_pack *mb_pack, bool global)
{
	smc_cmd->global_cmd = global;
	if (memcpy_s(smc_cmd->uuid, sizeof(smc_cmd->uuid),
		client_context->uuid, UUID_LEN)) {
		tloge("memcpy_s uuid error.\n");
		return -EFAULT;
	}
	smc_cmd->cmd_id = client_context->cmd_id;
	smc_cmd->dev_file_id = dev_file->dev_file_id;
	smc_cmd->context_id = client_context->session_id;
	smc_cmd->err_origin = client_context->returns.origin;
	smc_cmd->started = client_context->started;
#ifdef SECURITY_AUTH_ENHANCE
	if (tzmp2_uid(client_context, smc_cmd, global) != EOK)
		tloge("caution! tzmp uid failed !\n\n");
#endif
	tlogv("current uid is %d\n", smc_cmd->uid);
	if (client_context->param_types != 0) {
		smc_cmd->operation_phys = virt_to_phys(&mb_pack->operation);
		smc_cmd->operation_h_phys =
			virt_to_phys(&mb_pack->operation) >> ADDR_TRANS_NUM;
	} else {
		smc_cmd->operation_phys = 0;
		smc_cmd->operation_h_phys = 0;
	}
	smc_cmd->login_method = client_context->login.method;
	return 0;
}

static int check_login_for_encrypt(tc_ns_client_context *client_context,
	tc_ns_session *session, tc_ns_smc_cmd *smc_cmd,
	struct mb_cmd_pack *mb_pack, int need_check_login)
{
	int ret = 0;

	if (need_check_login && session != NULL) {
#ifdef SECURITY_AUTH_ENHANCE
		ret = do_encryption(session->auth_hash_buf,
			sizeof(session->auth_hash_buf),
			MAX_SHA_256_SZ * (NUM_OF_SO + 1),
			session->secure_info.crypto_info.key);
		if (ret) {
			tloge("hash encryption failed ret=%d\n", ret);
			return ret;
		}
#endif
		if (memcpy_s(mb_pack->login_data, sizeof(mb_pack->login_data),
			session->auth_hash_buf,
			sizeof(session->auth_hash_buf))) {
			tloge("copy login data failed\n");
			return -EFAULT;
		}
		smc_cmd->login_data_phy = virt_to_phys(mb_pack->login_data);
		smc_cmd->login_data_h_addr =
			virt_to_phys(mb_pack->login_data) >> ADDR_TRANS_NUM;
		smc_cmd->login_data_len = MAX_SHA_256_SZ * (NUM_OF_SO + 1);
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
	kuid = current_uid();
	*uid = kuid.val;
#else
	*uid = current_uid();
#endif
}

static int proc_check_login_for_open_session(
	tc_call_params *params, struct mb_cmd_pack *mb_pack,
	bool global, tc_ns_smc_cmd *smc_cmd)
{
	int ret;
	int need_check_login;
	tc_ns_dev_file *dev_file = params->dev_file;
	tc_ns_client_context *client_context = params->client_context;
	tc_ns_session *session = params->session;

	ret = init_smc_cmd(dev_file, client_context,
		smc_cmd, mb_pack, global);
	if (ret != 0)
		return ret;
	need_check_login = dev_file->pub_key_len == sizeof(uint32_t) &&
		smc_cmd->cmd_id == GLOBAL_CMD_ID_OPEN_SESSION &&
		(current->mm != NULL) && global;
	ret = check_login_for_encrypt(client_context, session,
		smc_cmd, mb_pack, need_check_login);
	if (ret != 0)
		return ret;
#ifdef CONFIG_TEE_SMP
	smc_cmd->ca_pid = current->pid;
#endif
	return ret;
}

static void reset_session_id(tc_ns_client_context *client_context,
	bool global, tc_ns_smc_cmd *smc_cmd, int tee_ret)
{
	int need_reset;

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


static void proc_error_situation(tc_call_params *params,
	struct mb_cmd_pack *mb_pack, tc_ns_smc_cmd *smc_cmd,
	int tee_ret, bool operation_init)
{
	/* kfree(NULL) is safe and this check is probably not required */
	params->client_context->returns.code = tee_ret;
	params->client_context->returns.origin = smc_cmd->err_origin;
	/* when CA invoke command and crash,
	 * Gtask happen to release service node ,tzdriver need to kill ion;
	 * ta crash ,tzdriver also need to kill ion
	 */
	if (tee_ret == TEE_ERROR_TAGET_DEAD || tee_ret == TEEC_ERROR_GENERIC) {
		tloge("ta_crash or ca is killed or some error happen\n");
#ifdef DYNAMIC_ION
		kill_ion_by_uuid((teec_uuid *)smc_cmd->uuid);
#endif
	}
	if (operation_init && mb_pack != NULL) {
		free_operation(params, &mb_pack->operation);
	}
	kfree(smc_cmd);
	mailbox_free(mb_pack);
	return;
}

static void proc_short_buffer_situation(tc_call_params *params,
	tc_ns_operation *operation, tc_ns_smc_cmd *smc_cmd,
	bool operation_init)
{
	int ret = 0;
	/* update size */
	if (operation_init) {
		ret = update_client_operation(params, operation, false);
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
	int ret;
	int tee_ret = 0;
	tc_ns_smc_cmd *smc_cmd = NULL;
	tc_ns_temp_buf local_temp_buffer[TEE_PARAM_NUM] = {
		{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
	};
	bool global = flags & TC_CALL_GLOBAL;
	uint32_t uid = 0;
#ifdef SECURITY_AUTH_ENHANCE
	bool is_token_work = false;
	tc_ns_token *tc_token = (session != NULL) ? &session->tc_ns_token : NULL;
#endif
	struct mb_cmd_pack *mb_pack = NULL;
	bool operation_init = false;
	tc_call_params in_params = { dev_file, client_context, session,
		local_temp_buffer, TEE_PARAM_NUM
	};

	get_uid_for_cmd(&uid);
	ret = check_params_for_client_call(dev_file, client_context);
	if (ret != 0)
		return ret;
	ret = alloc_for_client_call(&smc_cmd, &mb_pack);
	if (ret != 0)
		return ret;
	smc_cmd->uid = uid;
	if (client_context->param_types != 0) {
		ret = alloc_operation(&in_params, &mb_pack->operation, flags);
		if (ret) {
			tloge("alloc_operation malloc failed");
			goto error;
		}
		operation_init = true;
	}
	ret = proc_check_login_for_open_session(&in_params,
		mb_pack, global, smc_cmd);
	if (ret != 0)
		goto error;
#ifdef SECURITY_AUTH_ENHANCE
	/* invoke cmd(global is false) or open session */
	is_token_work = (!global) ||
		(smc_cmd->cmd_id == GLOBAL_CMD_ID_OPEN_SESSION);
	ret = load_security_enhance_info(&in_params, smc_cmd,
		tc_token, mb_pack, global, is_token_work);
	if (ret != EOK) {
		tloge("load_security_enhance_info  failed.\n");
		goto error;
	}
#endif
	/* send smc to secure world */
	tee_ret = tc_ns_smc(smc_cmd);
	reset_session_id(client_context, global, smc_cmd, tee_ret);
#ifdef SECURITY_AUTH_ENHANCE
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
#endif
	if (tee_ret != 0) {
#ifdef TC_ASYNC_NOTIFY_SUPPORT
		while (tee_ret == TEEC_PENDING) {
			pend_ca_thread(session, smc_cmd);
#ifdef SECURITY_AUTH_ENHANCE
			if (is_token_work) {
				ret = append_teec_token(client_context,
					tc_token, dev_file, global,
					mb_pack->token, sizeof(mb_pack->token));
				if (ret != EOK) {
					tloge("append teec's member failed. global=%d, cmd_id=%d, session_id=%d\n",
					      global, smc_cmd->cmd_id,
					      smc_cmd->context_id);
					goto error;
				}
			}
#endif
			tee_ret = tc_ns_smc_with_no_nr(smc_cmd);
#ifdef SECURITY_AUTH_ENHANCE
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
#endif
		}
#endif
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
#ifdef CONFIG_TEELOG
	tz_log_write();
#endif
	if (operation_init) {
		ret = update_client_operation(&in_params,
			&mb_pack->operation, true);
		if (ret) {
			smc_cmd->err_origin = TEEC_ORIGIN_COMMS;
			goto error;
		}
	}
	ret = 0;
	goto error;
short_buffer:
	if (tee_ret == TEEC_ERROR_SHORT_BUFFER)
		proc_short_buffer_situation(&in_params, &mb_pack->operation,
			smc_cmd, operation_init);
	ret = EFAULT;
error:
	proc_error_situation(&in_params, mb_pack, smc_cmd, tee_ret, operation_init);
	return ret;
}

#ifdef SECURITY_AUTH_ENHANCE
static bool is_opensession_by_index(uint8_t flags, uint32_t cmd_id,
	int index)
{
	/* params[2] for apk cert or native ca uid;
	 * params[3] for pkg name; therefore we set i>= 2
	 */
	bool global = flags & TC_CALL_GLOBAL;
	bool login_en = (global && (index >= TEE_PARAM_THREE) &&
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
	uint32_t buffer_size, uint8_t **plaintext,
	uint32_t *plaintext_buffer_size)
{
	if (buffer == NULL || buffer_size == 0) {
		tloge("bad params before encryption!\n");
		return -EINVAL;
	}
	*plaintext = buffer;
	*plaintext_buffer_size = buffer_size;
#if (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
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
	uint32_t plaintext_size;
	uint32_t plaintext_aligned_size;
	uint32_t total_size;
	uint8_t *cryptotext = NULL;
	uint8_t *plaintext = NULL;
	uint32_t plaintext_buffer_size;
	struct encryption_head head;
	int ret = check_param_for_encryption(buffer,
		buffer_size, &plaintext, &plaintext_buffer_size);

	if (ret != 0)
		return ret;
	ret = get_total_and_check(&plaintext_size, payload_size, buffer_size,
		&plaintext_aligned_size, &total_size);
	if (ret != 0)
		goto end;
	cryptotext = kzalloc(total_size, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)cryptotext)) {
		tloge("Malloc failed when doing encryption!\n");
		ret = -ENOMEM;
		goto end;
	}
	/* Setting encryption head */
	ret = set_encryption_head(&head, payload_size);
	if (ret) {
		tloge("Set encryption head failed, ret = %d.\n", ret);
		goto end;
	}
	ret = memcpy_s((void *)(plaintext + payload_size),
		plaintext_buffer_size - payload_size,
		(void *)&head, sizeof(head));
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

static int encrypt_login_info(uint32_t login_info_size,
	uint8_t *buffer, uint8_t *key)
{
	uint32_t payload_size;
	uint32_t plaintext_size;
	uint32_t plaintext_aligned_size;
	uint32_t total_size;

	if (buffer == NULL) {
		tloge("Login information buffer is null!\n");
		return -EINVAL;
	}
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
#endif
