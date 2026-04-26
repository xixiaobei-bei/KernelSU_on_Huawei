/*
 * gp_op.c
 *
 * alloc global operation and pass params to TEE.
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
#include <linux/cred.h>
#include <asm/memory.h>
#include <securec.h>

#if ((defined CONFIG_ION_HISI) || (defined CONFIG_ION_HISI_SECSG))
#include <linux/hisi/hisi_ion.h>
#endif

#include "teek_client_constants.h"
#include "tc_ns_client.h"
#include "agent.h"
#include "tc_ns_log.h"
#include "smc_smp.h"
#include "mem.h"
#include "mailbox_mempool.h"
#include "tc_client_driver.h"
#include "dynamic_ion_mem.h"
#include "security_auth_enhance.h"
#include "tlogger.h"

#define MAX_SHARED_SIZE 0x100000      /* 1 MiB */

static void free_operation(const struct tc_call_params *call_params,
	struct tc_op_params *op_params);

/* dir: 0-inclue input, 1-include output, 2-both */
#define INPUT  0
#define OUTPUT 1
#define INOUT  2

static inline bool is_input_type(int dir)
{
	if (dir == INPUT || dir == INOUT)
		return true;

	return false;
}

static inline bool is_output_type(int dir)
{
	if (dir == OUTPUT || dir == INOUT)
		return true;

	return false;
}

static inline bool teec_value_type(unsigned int type, int dir)
{
	return ((is_input_type(dir) && type == TEEC_VALUE_INPUT) ||
		(is_output_type(dir) && type == TEEC_VALUE_OUTPUT) ||
		type == TEEC_VALUE_INOUT) ? true : false;
}

static inline bool teec_tmpmem_type(unsigned int type, int dir)
{
	return ((is_input_type(dir) && type == TEEC_MEMREF_TEMP_INPUT) ||
		(is_output_type(dir) && type == TEEC_MEMREF_TEMP_OUTPUT) ||
		type == TEEC_MEMREF_TEMP_INOUT) ? true : false;
}

static inline bool teec_memref_type(unsigned int type, int dir)
{
	return ((is_input_type(dir) && type == TEEC_MEMREF_PARTIAL_INPUT) ||
		(is_output_type(dir) && type == TEEC_MEMREF_PARTIAL_OUTPUT) ||
		type == TEEC_MEMREF_PARTIAL_INOUT) ? true : false;
}

static int check_user_param(const struct tc_ns_client_context *client_context,
	unsigned int index)
{
	if (!client_context) {
		tloge("client_context is null\n");
		return -EINVAL;
	}

	if (index >= PARAM_NUM) {
		tloge("index is invalid, index:%x\n", index);
		return -EINVAL;
	}
	return 0;
}

bool is_tmp_mem(uint32_t param_type)
{
	if (param_type == TEEC_MEMREF_TEMP_INPUT ||
		param_type == TEEC_MEMREF_TEMP_OUTPUT ||
		param_type == TEEC_MEMREF_TEMP_INOUT)
		return true;

	return false;
}

bool is_ref_mem(uint32_t param_type)
{
	if (param_type == TEEC_MEMREF_PARTIAL_INPUT ||
		param_type == TEEC_MEMREF_PARTIAL_OUTPUT ||
		param_type == TEEC_MEMREF_PARTIAL_INOUT)
		return true;

	return false;
}

bool is_val_param(uint32_t param_type)
{
	if (param_type == TEEC_VALUE_INPUT ||
		param_type == TEEC_VALUE_OUTPUT ||
		param_type == TEEC_VALUE_INOUT ||
		param_type == TEEC_ION_INPUT ||
		param_type == TEEC_ION_SGLIST_INPUT)
		return true;

	return false;
}

bool is_ion_param(uint32_t param_type)
{
	if (param_type == TEEC_ION_INPUT ||
		param_type == TEEC_ION_SGLIST_INPUT)
		return true;

	return false;
}

static bool is_mem_param(uint32_t param_type)
{
	if (is_tmp_mem(param_type) || is_ref_mem(param_type))
		return true;

	return false;
}

/* Check the size and buffer addresses  have valid userspace addresses */
static bool is_usr_refmem_valid(union tc_ns_client_param *client_param)
{
	uint32_t size = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 18) || \
	LINUX_VERSION_CODE == KERNEL_VERSION(4, 19, 71))
	if (!access_ok(VERIFY_READ,
		(void *)(uintptr_t)client_param->memref.size_addr,
		sizeof(uint32_t)))
#else
	if (!access_ok(
		(void *)(uintptr_t)client_param->memref.size_addr,
		sizeof(uint32_t)))
#endif
		return false;

	get_user(size, (uint32_t *)(uintptr_t)client_param->memref.size_addr);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 18) || \
	LINUX_VERSION_CODE == KERNEL_VERSION(4, 19, 71))
	if (!access_ok(VERIFY_READ,
		(void *)(uintptr_t)client_param->memref.buffer, size))
#else
	if (!access_ok((void *)(uintptr_t)client_param->memref.buffer, size))
#endif
		return false;

	return true;
}

static bool is_usr_valmem_valid(union tc_ns_client_param *client_param)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 18) || \
	LINUX_VERSION_CODE == KERNEL_VERSION(4, 19, 71))
	if (!access_ok(VERIFY_READ,
		(void *)(uintptr_t)client_param->value.a_addr,
		sizeof(uint32_t)))
#else
	if (!access_ok(
		(void *)(uintptr_t)client_param->value.a_addr,
		sizeof(uint32_t)))
#endif
		return false;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 18) || \
	LINUX_VERSION_CODE == KERNEL_VERSION(4, 19, 71))
	if (!access_ok(VERIFY_READ,
		(void *)(uintptr_t)client_param->value.b_addr,
		sizeof(uint32_t)))
#else
	if (!access_ok(
		(void *)(uintptr_t)client_param->value.b_addr,
		sizeof(uint32_t)))
#endif
		return false;

	return true;
}

bool tc_user_param_valid(struct tc_ns_client_context *client_context,
	unsigned int index)
{
	union tc_ns_client_param *client_param = NULL;
	unsigned int param_type;

	if (check_user_param(client_context, index))
		return false;

	client_param = &(client_context->params[index]);
	param_type = teec_param_type_get(client_context->param_types, index);
	tlogd("param %u type is %x\n", index, param_type);
	if (param_type == TEEC_NONE) {
		tlogd("param type is TEEC_NONE\n");
		return true;
	}

	if (is_mem_param(param_type)) {
		if (!is_usr_refmem_valid(client_param))
			return false;
	} else if (is_val_param(param_type)) {
		if (!is_usr_valmem_valid(client_param))
			return false;
	} else {
		tloge("param types is not supported\n");
		return false;
	}

	return true;
}

static bool is_kernel_addr_valid(const void *addr)
{
	if (virt_addr_valid((unsigned long)(uintptr_t)addr) ||
		vmalloc_addr_valid(addr) || modules_addr_valid(addr))
		return true;

	return false;
}

/*
 * These function handle read from client. Because client here can be
 * kernel client or user space client, we must use the proper function
 */
int read_from_client(void *dest, size_t dest_size,
	const void __user *src, size_t size, uint8_t kernel_api)
{
	int ret;

	if (!dest || !src) {
		tloge("src or dest is NULL input buffer\n");
		return -EINVAL;
	}

	if (size > dest_size) {
		tloge("size is larger than dest_size or size is 0\n");
		return -EINVAL;
	}
	if (!size)
		return 0;

	if (kernel_api) {
		if (!is_kernel_addr_valid(src)) {
			tloge("invalid addr\n");
			return -EFAULT;
		}
		ret = memcpy_s(dest, dest_size, src, size);
		if (ret != EOK) {
			tloge("memcpy fail. line=%d, s_ret=%d\n",
				__LINE__, ret);
			return ret;
		}
		return ret;
	}
	/* buffer is in user space(CA call TEE API) */
	if (copy_from_user(dest, src, size)) {
		tloge("copy from user failed\n");
		return -EFAULT;
	}

	return 0;
}

int write_to_client(void __user *dest, size_t dest_size,
	const void *src, size_t size, uint8_t kernel_api)
{
	int ret;

	if (!dest || !src) {
		tloge("src or dest is NULL input buffer\n");
		return -EINVAL;
	}

	if (size > dest_size) {
		tloge("size is larger than dest_size\n");
		return -EINVAL;
	}

	if (!size)
		return 0;

	if (kernel_api) {
		if (!is_kernel_addr_valid(dest)) {
			tloge("invalid addr\n");
			return -EFAULT;
		}
		ret = memcpy_s(dest, dest_size, src, size);
		if (ret != EOK) {
			tloge("write to client fail. line=%d, ret=%d\n",
			      __LINE__, ret);
			return ret;
		}
		return ret;
	}

	/* buffer is in user space(CA call TEE API) */
	if (copy_to_user(dest, src, size)) {
		tloge("copy to user failed\n");
		return -EFAULT;
	}
	return 0;
}

static bool is_input_tempmem(unsigned int param_type)
{
	if (param_type == TEEC_MEMREF_TEMP_INPUT ||
		param_type == TEEC_MEMREF_TEMP_INOUT)
		return true;

	return false;
}

static int update_input_data(const union tc_ns_client_param *client_param,
	uint32_t buffer_size, void *temp_buf,
	unsigned int param_type, uint8_t kernel_params)
{
	if (!is_input_tempmem(param_type))
		return 0;

	if (read_from_client(temp_buf, buffer_size,
		(void *)(uintptr_t)client_param->memref.buffer,
		buffer_size, kernel_params)) {
		tloge("copy memref buffer failed\n");
		return -EFAULT;
	}
	return 0;
}

static int do_opensess_auth_enhance(const struct tc_call_params *call_params,
	void *temp_buf, uint32_t buffer_size, unsigned int index)
{
	if (!is_opensession_by_index(call_params->flags,
		call_params->context->cmd_id, index))
		return 0;

#ifdef CONFIG_AUTH_ENHANCE
	if (encrypt_login_info(buffer_size, temp_buf,
		call_params->sess->secure_info.crypto_info.key)) {
		tloge("encrypt login info failed\n");
		return -EFAULT;
	}
#endif
	return 0;
}

/*
 * temp buffers we need to allocate/deallocate
 * for every operation
 */
static int alloc_for_tmp_mem(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index)
{
	union tc_ns_client_param *client_param = NULL;
	void *temp_buf = NULL;
	uint32_t buffer_size = 0;

	/* this never happens */
	if (index >= TEE_PARAM_NUM)
		return -EINVAL;

	/* For compatibility sake we assume buffer size to be 32bits */
	client_param = &(call_params->context->params[index]);
	if (read_from_client(&buffer_size, sizeof(buffer_size),
		(uint32_t __user *)(uintptr_t)client_param->memref.size_addr,
		sizeof(uint32_t), kernel_params)) {
		tloge("copy memref.size_addr failed\n");
		return -EFAULT;
	}

	if (buffer_size > MAX_SHARED_SIZE) {
		tloge("buffer size %u from user is too large\n", buffer_size);
		return -EFAULT;
	}

	temp_buf = mailbox_alloc(buffer_size, MB_FLAG_ZERO);
	if (!temp_buf) {
		tloge("temp buf malloc failed, i = %u\n", index);
		return -ENOMEM;
	}
	op_params->local_tmpbuf[index].temp_buffer = temp_buf;
	op_params->local_tmpbuf[index].size = buffer_size;

	if (update_input_data(client_param, buffer_size, temp_buf,
		param_type, kernel_params))
		return -EFAULT;

	if (do_opensess_auth_enhance(call_params, temp_buf,
		buffer_size, index))
		return -EFAULT;

	op_params->mb_pack->operation.params[index].memref.buffer =
		virt_to_phys(temp_buf);
	op_params->mb_pack->operation.buffer_h_addr[index] =
		virt_to_phys(temp_buf) >> ADDR_TRANS_NUM;
	op_params->mb_pack->operation.params[index].memref.size = buffer_size;
	/* TEEC_MEMREF_TEMP_INPUT equal to TEE_PARAM_TYPE_MEMREF_INPUT */
	op_params->trans_paramtype[index] = param_type;

	return 0;
}

static int check_buffer_for_ref(uint32_t *buffer_size,
	const union tc_ns_client_param *client_param, uint8_t kernel_params)
{
	if (read_from_client(buffer_size, sizeof(*buffer_size),
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

static bool is_refmem_offset_valid(struct tc_ns_shared_mem *shared_mem,
	union tc_ns_client_param *client_param, uint32_t buffer_size)
{
	/*
	 * arbitrary CA can control offset by ioctl, so in here
	 * offset must be checked, and avoid integer overflow.
	 */
	if (((shared_mem->len - client_param->memref.offset) >= buffer_size) &&
		(shared_mem->len > client_param->memref.offset))
		return true;
	tloge("Unexpected size %u vs %u", shared_mem->len, buffer_size);
	return false;
}

static bool is_phyaddr_valid(struct tc_ns_operation *operation, int index)
{
	/*
	 * for 8G physical memory device, there is a chance that
	 * operation->params[i].memref.buffer could be all 0,
	 * buffer_h_addr cannot be 0 in the same time.
	 */
	if ((!operation->params[index].memref.buffer) &&
		(!operation->buffer_h_addr[index])) {
		tloge("can not find shared buffer, exit\n");
		return false;
	}

	return true;
}

/*
 * MEMREF_PARTIAL buffers are already allocated so we just
 * need to search for the shared_mem ref;
 * For interface compatibility we assume buffer size to be 32bits
 */
static int alloc_for_ref_mem(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index)
{
	union tc_ns_client_param *client_param = NULL;
	struct tc_ns_shared_mem *shared_mem = NULL;
	uint32_t buffer_size = 0;
	void *buffer_addr = NULL;
	int ret = 0;

	/* this never happens */
	if (index >= TEE_PARAM_NUM)
		return -EINVAL;

	client_param = &(call_params->context->params[index]);
	if (check_buffer_for_ref(&buffer_size, client_param, kernel_params))
		return -EINVAL;

	op_params->mb_pack->operation.params[index].memref.buffer = 0;

	mutex_lock(&call_params->dev->shared_mem_lock);
	list_for_each_entry(shared_mem,
		&call_params->dev->shared_mem_list, head) {
		if (shared_mem->user_addr !=
			(void *)(uintptr_t)client_param->memref.buffer)
			continue;
		if (!is_refmem_offset_valid(shared_mem, client_param,
			buffer_size)) {
			break;
		}
		buffer_addr = (void *)(uintptr_t)(
			(uintptr_t)shared_mem->kernel_addr +
			client_param->memref.offset);
		buffer_addr = mailbox_copy_alloc(buffer_addr, buffer_size);
		if (!buffer_addr) {
			ret = -ENOMEM;
			break;
		}
		op_params->mb_pack->operation.mb_buffer[index] = buffer_addr;
		op_params->mb_pack->operation.params[index].memref.buffer =
			virt_to_phys(buffer_addr);
		op_params->mb_pack->operation.buffer_h_addr[index] =
			virt_to_phys(buffer_addr) >> ADDR_TRANS_NUM;
		op_params->mb_pack->operation.sharemem[index] = shared_mem;
		get_sharemem_struct(shared_mem);
		break;
	}
	mutex_unlock(&call_params->dev->shared_mem_lock);

	if (!is_phyaddr_valid(&op_params->mb_pack->operation, index))
		return -EINVAL;

	op_params->mb_pack->operation.params[index].memref.size = buffer_size;
	/* Change TEEC_MEMREF_PARTIAL_XXXXX  to TEE_PARAM_TYPE_MEMREF_XXXXX */
	op_params->trans_paramtype[index] = param_type -
		(TEEC_MEMREF_PARTIAL_INPUT - TEE_PARAM_TYPE_MEMREF_INPUT);
	return ret;
}

static int transfer_client_value(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index)
{
	struct tc_ns_operation *operation = &op_params->mb_pack->operation;
	union tc_ns_client_param *client_param = NULL;

	/* this never happens */
	if (index >= TEE_PARAM_NUM)
		return -EINVAL;

	client_param = &(call_params->context->params[index]);
	if (read_from_client(&operation->params[index].value.a,
		sizeof(operation->params[index].value.a),
		(void *)(uintptr_t)client_param->value.a_addr,
		sizeof(operation->params[index].value.a),
		kernel_params)) {
		tloge("copy valuea failed\n");
		return -EFAULT;
	}
	if (read_from_client(&operation->params[index].value.b,
		sizeof(operation->params[index].value.b),
		(void *)(uintptr_t)client_param->value.b_addr,
		sizeof(operation->params[index].value.b),
		kernel_params)) {
		tloge("copy valueb failed\n");
		return -EFAULT;
	}

	/* TEEC_VALUE_INPUT equal to TEE_PARAM_TYPE_VALUE_INPUT */
	op_params->trans_paramtype[index] = param_type;
	return 0;
}

#ifdef CONFIG_ION_HISI_SECSG
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

	if (secmem_get_buffer(ion_shared_fd, &ion_table, &ion_id, &ion_type)) {
		tloge("get ion table failed. \n");
		return -EFAULT;
	}

	if (ion_type != SEC_DRM_TEE) {
		if (ion_table->nents <= 0 || ion_table->nents > MAX_ION_NENTS)
			return -EFAULT;
		ion_list_num = (uint32_t)(ion_table->nents & INT_MAX);
		for_each_sg(ion_table->sgl, sg, ion_list_num, i) {
			if (!sg) {
				tloge("an error sg when get ion sglist \n");
				return -EFAULT;
			}
		}
	}
	/* ion_list_num is less than 1024, so sglist_size won't flow */
	sglist_size = sizeof(struct ion_page_info) * ion_list_num +
		sizeof(*tmp_sglist);
	tmp_sglist = (struct sglist *)mailbox_alloc(sglist_size, MB_FLAG_ZERO);
	if (!tmp_sglist) {
		tloge("sglist mem alloc failed\n");
		return -ENOMEM;
	}
	tmp_sglist->sglist_size = (uint64_t)sglist_size;
	tmp_sglist->ion_size = (uint64_t)ion_alloc_size;
	tmp_sglist->info_length = (uint64_t)ion_list_num;
	if (ion_type != SEC_DRM_TEE) {
		for_each_sg(ion_table->sgl, sg, ion_list_num, i) {
			page = sg_page(sg);
			tmp_sglist->page_info[i].phys_addr =
				page_to_phys(page);
			tmp_sglist->page_info[i].npages =
				sg->length / PAGE_SIZE;
		}
	} else {
		tmp_sglist->ion_id = ion_id;
	}

	*sglist_table = virt_to_phys(tmp_sglist);
	*ion_sglist_size = sglist_size;
	return 0;
}

static int alloc_for_ion_sglist(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index)
{
	struct tc_ns_operation *operation = &op_params->mb_pack->operation;
	size_t ion_sglist_size = 0;
	phys_addr_t ion_sglist_addr = 0x0;
	union tc_ns_client_param *client_param = NULL;
	unsigned int ion_shared_fd = 0;
	unsigned int ion_alloc_size;

	/* this never happens */
	if (index >= TEE_PARAM_NUM)
		return -EINVAL;

	client_param = &(call_params->context->params[index]);
	if (read_from_client(&operation->params[index].value.a,
		sizeof(operation->params[index].value.a),
		(void *)(uintptr_t)client_param->value.a_addr,
		sizeof(operation->params[index].value.a), kernel_params)) {
		tloge("valuea copy failed\n");
		return -EFAULT;
	}
	if (read_from_client(&operation->params[index].value.b,
		sizeof(operation->params[index].value.b),
		(void *)(uintptr_t)client_param->value.b_addr,
		sizeof(operation->params[index].value.b), kernel_params)) {
		tloge("valueb copy failed\n");
		return -EFAULT;
	}
	ion_shared_fd = operation->params[index].value.a;
	ion_alloc_size = operation->params[index].value.b;

	if (get_ion_sg_list_from_fd(ion_shared_fd, ion_alloc_size,
		&ion_sglist_addr, &ion_sglist_size)) {
		tloge("get ion sglist failed, fd=%u\n", ion_shared_fd);
		return -EFAULT;
	}
	op_params->local_tmpbuf[index].temp_buffer =
		phys_to_virt(ion_sglist_addr);
	op_params->local_tmpbuf[index].size = ion_sglist_size;

	operation->params[index].memref.buffer = (unsigned int)ion_sglist_addr;
	operation->buffer_h_addr[index] =
		(unsigned int)(ion_sglist_addr >> ADDR_TRANS_NUM);
	operation->params[index].memref.size = (unsigned int)ion_sglist_size;
	op_params->trans_paramtype[index] = param_type;

	return 0;
}
#else
static int alloc_for_ion_sglist(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index)
{
	tloge("not support hisi seg and releated feature!\n");
	return -1;
}
#endif

#ifdef CONFIG_ION_HISI
static int transfer_ion_params(struct tc_ns_operation *operation,
	union tc_ns_client_param *client_param, uint8_t kernel_params,
	unsigned int index)
{
	if (read_from_client(&operation->params[index].value.a,
		sizeof(operation->params[index].value.a),
		(void *)(uintptr_t)client_param->value.a_addr,
		sizeof(operation->params[index].value.a), kernel_params)) {
		tloge("value.a_addr copy failed\n");
		return -EFAULT;
	}

	if (read_from_client(&operation->params[index].value.b,
		sizeof(operation->params[index].value.b),
		(void *)(uintptr_t)client_param->value.b_addr,
		sizeof(operation->params[index].value.b), kernel_params)) {
		tloge("value.b_addr copy failed\n");
		return -EFAULT;
	}

	return 0;
}

static int alloc_for_ion(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index)
{
	struct tc_ns_operation *operation = &op_params->mb_pack->operation;
	size_t drm_ion_size = 0;
	phys_addr_t drm_ion_phys = 0x0;
	struct dma_buf *drm_dma_buf = NULL;
	union tc_ns_client_param *client_param = NULL;
	unsigned int ion_shared_fd = 0;
	int ret = 0;

	/* this never happens */
	if (index >= TEE_PARAM_NUM)
		return -EINVAL;

	client_param = &(call_params->context->params[index]);
	if (transfer_ion_params(operation, client_param, kernel_params, index))
		return -EFAULT;

	ion_shared_fd = operation->params[index].value.a;
	drm_dma_buf = dma_buf_get(ion_shared_fd);
	if (IS_ERR_OR_NULL(drm_dma_buf)) {
		tloge("drm dma buf is err, ret=%d fd=%u\n", ret, ion_shared_fd);
		return -EFAULT;
	}

	ret = ion_secmem_get_phys(drm_dma_buf, &drm_ion_phys, &drm_ion_size);
	if (ret) {
		tloge("in %s err:ret=%d fd=%u\n", __func__, ret, ion_shared_fd);
		dma_buf_put(drm_dma_buf);
		return -EFAULT;
	}

	if (drm_ion_size > operation->params[index].value.b)
		drm_ion_size = operation->params[index].value.b;
	operation->params[index].memref.buffer = (unsigned int)drm_ion_phys;
	operation->params[index].memref.size = (unsigned int)drm_ion_size;
	op_params->trans_paramtype[index] = param_type;
	dma_buf_put(drm_dma_buf);

	return ret;
}
#else
static inline int alloc_for_ion(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, uint8_t kernel_params,
	uint32_t param_type, unsigned int index)
{
	tloge("not support hisi ion and releated feature!\n");
	return -1;
}
#endif

static int alloc_operation(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	int ret = 0;
	uint32_t index;
	uint8_t kernel_params;
	uint32_t param_type;

	kernel_params = call_params->dev->kernel_api;
	for (index = 0; index < TEE_PARAM_NUM; index++) {
		/*
		 * Normally kernel_params = kernel_api
		 * But when TC_CALL_LOGIN, params 2/3 will
		 * be filled by kernel. so under this circumstance,
		 * params 2/3 has to be set to kernel mode; and
		 * param 0/1 will keep the same with kernel_api.
		 */
		if ((call_params->flags & TC_CALL_LOGIN) && (index >= 2))
			kernel_params = TEE_REQ_FROM_KERNEL_MODE;
		param_type = teec_param_type_get(
			call_params->context->param_types, index);

		tlogd("param %u type is %x\n", index, param_type);
		if (teec_tmpmem_type(param_type, INOUT))
			ret = alloc_for_tmp_mem(call_params, op_params,
				kernel_params, param_type, index);
		else if (teec_memref_type(param_type, INOUT))
			ret = alloc_for_ref_mem(call_params, op_params,
				kernel_params, param_type, index);
		else if (teec_value_type(param_type, INOUT))
			ret = transfer_client_value(call_params, op_params,
				kernel_params, param_type, index);
		else if (param_type == TEEC_ION_INPUT)
			ret = alloc_for_ion(call_params, op_params,
				kernel_params, param_type, index);
		else if (param_type == TEEC_ION_SGLIST_INPUT)
			ret = alloc_for_ion_sglist(call_params, op_params,
				kernel_params, param_type, index);
		else
			tlogd("param type = TEEC_NONE\n");

		if (ret)
			break;
	}
	if (ret) {
		free_operation(call_params, op_params);
		return ret;
	}
	op_params->mb_pack->operation.paramtypes =
		teec_param_types(op_params->trans_paramtype[0],
		op_params->trans_paramtype[1],
		op_params->trans_paramtype[2],
		op_params->trans_paramtype[3]);
	op_params->op_inited = true;

	return ret;
}

static int update_tmp_mem(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, unsigned int index, bool is_complete)
{
	union tc_ns_client_param *client_param = NULL;
	uint32_t buffer_size;
	struct tc_ns_operation *operation = &op_params->mb_pack->operation;

	if (index >= TEE_PARAM_NUM) {
		tloge("tmp buf size or index is invalid\n");
		return -EFAULT;
	}

	buffer_size = operation->params[index].memref.size;
	client_param = &(call_params->context->params[index]);
	/* Size is updated all the time */
	if (write_to_client((void *)(uintptr_t)client_param->memref.size_addr,
		sizeof(buffer_size),
		&buffer_size, sizeof(buffer_size),
		call_params->dev->kernel_api)) {
		tloge("copy tempbuf size failed\n");
		return -EFAULT;
	}
	if (buffer_size > op_params->local_tmpbuf[index].size) {
		/* incomplete case, when the buffer size is invalid see next param */
		if (!is_complete)
			return 0;
		/*
		 * complete case, operation is allocated from mailbox
		 *  and share with gtask, so it's possible to be changed
		 */
		tloge("memref.size has been changed larger than the initial\n");
		return -EFAULT;
	}
	/* Only update the buffer when the buffer size is valid in complete case */
	if (write_to_client((void *)(uintptr_t)client_param->memref.buffer,
		operation->params[index].memref.size,
		op_params->local_tmpbuf[index].temp_buffer,
		operation->params[index].memref.size,
		call_params->dev->kernel_api)) {
		tloge("copy tempbuf failed\n");
		return -ENOMEM;
	}
	return 0;
}

static int update_for_ref_mem(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, unsigned int index)
{
	union tc_ns_client_param *client_param = NULL;
	uint32_t buffer_size;
	unsigned int orig_size = 0;
	struct tc_ns_operation *operation = &op_params->mb_pack->operation;

	if (index >= TEE_PARAM_NUM) {
		tloge("index is invalid\n");
		return -EFAULT;
	}

	/* update size */
	buffer_size = operation->params[index].memref.size;
	client_param = &(call_params->context->params[index]);

	if (read_from_client(&orig_size,
		sizeof(orig_size),
		(uint32_t __user *)(uintptr_t)client_param->memref.size_addr,
		sizeof(orig_size), call_params->dev->kernel_api)) {
		tloge("copy orig memref.size_addr failed\n");
		return -EFAULT;
	}

	if (write_to_client((void *)(uintptr_t)client_param->memref.size_addr,
		sizeof(buffer_size),
		&buffer_size, sizeof(buffer_size),
		call_params->dev->kernel_api)) {
		tloge("copy buf size failed\n");
		return -EFAULT;
	}

	/* copy from mb_buffer to sharemem */
	if (operation->mb_buffer[index] && orig_size >= buffer_size) {
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

static int update_for_value(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, unsigned int index)
{
	union tc_ns_client_param *client_param = NULL;
	struct tc_ns_operation *operation = &op_params->mb_pack->operation;

	if (index >= TEE_PARAM_NUM) {
		tloge("index is invalid\n");
		return -EFAULT;
	}
	client_param = &(call_params->context->params[index]);
	if (write_to_client((void *)(uintptr_t)client_param->value.a_addr,
		sizeof(operation->params[index].value.a),
		&operation->params[index].value.a,
		sizeof(operation->params[index].value.a),
		call_params->dev->kernel_api)) {
		tloge("inc copy value.a_addr failed\n");
		return -EFAULT;
	}
	if (write_to_client((void *)(uintptr_t)client_param->value.b_addr,
		sizeof(operation->params[index].value.b),
		&operation->params[index].value.b,
		sizeof(operation->params[index].value.b),
		call_params->dev->kernel_api)) {
		tloge("inc copy value.b_addr failed\n");
		return -EFAULT;
	}
	return 0;
}

static int update_client_operation(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, bool is_complete)
{
	int ret = 0;
	uint32_t param_type;
	uint32_t index;

	if (!op_params->op_inited)
		return 0;

	/* if paramTypes is NULL, no need to update */
	if (!call_params->context->param_types)
		return 0;

	for (index = 0; index < TEE_PARAM_NUM; index++) {
		param_type = teec_param_type_get(
			call_params->context->param_types, index);
		if (teec_tmpmem_type(param_type, OUTPUT))
			ret = update_tmp_mem(call_params, op_params,
				index, is_complete);
		else if (teec_memref_type(param_type, OUTPUT))
			ret = update_for_ref_mem(call_params,
				op_params, index);
		else if (is_complete && teec_value_type(param_type, OUTPUT))
			ret = update_for_value(call_params, op_params, index);
		else
			tlogd("param_type:%u don't need to update\n", param_type);
		if (ret)
			break;
	}
	return ret;
}

static void free_operation(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	uint32_t param_type;
	uint32_t index;
	void *temp_buf = NULL;
	struct tc_ns_temp_buf *local_tmpbuf = op_params->local_tmpbuf;
	struct tc_ns_operation *operation = &op_params->mb_pack->operation;

	for (index = 0; index < TEE_PARAM_NUM; index++) {
		param_type = teec_param_type_get(
			call_params->context->param_types, index);
		if (is_tmp_mem(param_type)) {
			/* free temp buffer */
			temp_buf = local_tmpbuf[index].temp_buffer;
			tlogd("free temp buf, i = %u\n", index);
			if (virt_addr_valid(
				(unsigned long)(uintptr_t)temp_buf) &&
				!ZERO_OR_NULL_PTR(
				(unsigned long)(uintptr_t)temp_buf)) {
				mailbox_free(temp_buf);
				temp_buf = NULL;
			}
		} else if (is_ref_mem(param_type)) {
			put_sharemem_struct(operation->sharemem[index]);
			if (operation->mb_buffer[index])
				mailbox_free(operation->mb_buffer[index]);
		} else if (param_type == TEEC_ION_SGLIST_INPUT) {
			temp_buf = local_tmpbuf[index].temp_buffer;
			tlogd("free ion sglist buf, i = %u\n", index);
			if (virt_addr_valid((uint64_t)(uintptr_t)temp_buf) &&
				!ZERO_OR_NULL_PTR(
				(unsigned long)(uintptr_t)temp_buf)) {
				mailbox_free(temp_buf);
				temp_buf = NULL;
			}
		}
	}
}

static int is_clicall_params_vaild(const struct tc_call_params *call_params)
{
	if (!call_params) {
		tloge("call param is null");
		return false;
	}

	if (!call_params->dev) {
		tloge("dev file is null");
		return false;
	}

	if (!call_params->context) {
		tloge("client context is null");
		return false;
	}

	return true;
}

static int alloc_for_client_call(struct tc_op_params *op_params)
{
	op_params->smc_cmd = kzalloc(sizeof(*(op_params->smc_cmd)),
		GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)(op_params->smc_cmd))) {
		tloge("smc cmd malloc failed\n");
		return -ENOMEM;
	}

	op_params->mb_pack = mailbox_alloc_cmd_pack();
	if (!op_params->mb_pack) {
		kfree(op_params->smc_cmd);
		op_params->smc_cmd = NULL;
		return -ENOMEM;
	}

	return 0;
}

static int init_smc_cmd(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	struct tc_ns_smc_cmd *smc_cmd = op_params->smc_cmd;
	struct tc_ns_client_context *context = call_params->context;
	struct tc_ns_operation *operation = &op_params->mb_pack->operation;
	bool global = call_params->flags & TC_CALL_GLOBAL;

	smc_cmd->global_cmd = global;
	if (memcpy_s(smc_cmd->uuid, sizeof(smc_cmd->uuid),
		context->uuid, UUID_LEN)) {
		tloge("memcpy uuid error\n");
		return -EFAULT;
	}
	smc_cmd->cmd_id = context->cmd_id;
	smc_cmd->dev_file_id = call_params->dev->dev_file_id;
	smc_cmd->context_id = context->session_id;
	smc_cmd->err_origin = context->returns.origin;
	smc_cmd->started = context->started;
	smc_cmd->ca_pid = current->pid;

	if (tzmp2_uid(context, smc_cmd, global) != EOK)
		tloge("caution! tzmp uid failed !\n\n");

	tlogv("current uid is %u\n", smc_cmd->uid);
	if (context->param_types) {
		smc_cmd->operation_phys =
			virt_to_phys(operation);
		smc_cmd->operation_h_phys =
			virt_to_phys(operation) >> ADDR_TRANS_NUM;
	} else {
		smc_cmd->operation_phys = 0;
		smc_cmd->operation_h_phys = 0;
	}
	smc_cmd->login_method = context->login.method;

	return 0;
}

static bool need_check_login(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	if (call_params->dev->pub_key_len == sizeof(uint32_t) &&
		op_params->smc_cmd->cmd_id == GLOBAL_CMD_ID_OPEN_SESSION &&
		current->mm && (call_params->flags & TC_CALL_GLOBAL))
		return true;

	return false;
}

static int check_login_for_encrypt(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	struct tc_ns_session *sess = call_params->sess;
	struct tc_ns_smc_cmd *smc_cmd = op_params->smc_cmd;
	struct mb_cmd_pack *mb_pack = op_params->mb_pack;

	if (need_check_login(call_params, op_params) && sess) {
#ifdef CONFIG_AUTH_ENHANCE
		int ret = do_encryption(sess->auth_hash_buf,
			sizeof(sess->auth_hash_buf),
			MAX_SHA_256_SZ * (NUM_OF_SO + 1),
			sess->secure_info.crypto_info.key);
		if (ret) {
			tloge("hash encryption failed ret=%d\n", ret);
			return ret;
		}
#endif
		if (memcpy_s(mb_pack->login_data, sizeof(mb_pack->login_data),
			sess->auth_hash_buf,
			sizeof(sess->auth_hash_buf))) {
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

static uint32_t get_uid_for_cmd(void)
{
	kuid_t kuid;

	kuid = current_uid();
	return kuid.val;
}

static void reset_session_id(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, int tee_ret)
{
	bool need_reset = false;

	call_params->context->session_id = op_params->smc_cmd->context_id;
	/*
	 * if tee_ret error except TEEC_PENDING,
	 * but context_id is seted,need to reset to 0
	 */
	need_reset = ((call_params->flags & TC_CALL_GLOBAL) &&
		call_params->context->cmd_id == GLOBAL_CMD_ID_OPEN_SESSION &&
		tee_ret && tee_ret != TEEC_PENDING);
	if (need_reset)
		call_params->context->session_id = 0;
	return;
}

static void pend_ca_thread(struct tc_ns_session *session,
	const struct tc_ns_smc_cmd *smc_cmd)
{
	struct tc_wait_data *wq = NULL;

	if (session)
		wq = &session->wait_data;

	if (wq) {
		tlogv("before wait event\n");
		/*
		 * use wait_event instead of wait_event_interruptible so
		 * that ap suspend will not wake up the TEE wait call
		 */
		wait_event(wq->send_cmd_wq, wq->send_wait_flag);
		wq->send_wait_flag = 0;
	}
	tlogv("operation start is :%d\n", smc_cmd->started);
	return;
}


static void release_tc_call_resource(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, int tee_ret)
{
	/* kfree(NULL) is safe and this check is probably not required */
	call_params->context->returns.code = tee_ret;
	call_params->context->returns.origin = op_params->smc_cmd->err_origin;

	/*
	 * 1. when CA invoke command and crash, Gtask release service node
	 * then del ion won't be triggered, so here tzdriver need to kill ion;
	 * 2. when ta crash, tzdriver also need to kill ion;
	 */
	if (tee_ret == TEE_ERROR_TAGET_DEAD || tee_ret == TEEC_ERROR_GENERIC)
		kill_ion_by_uuid((struct tc_uuid *)op_params->smc_cmd->uuid);

	if (op_params->op_inited)
		free_operation(call_params, op_params);

	kfree(op_params->smc_cmd);
	mailbox_free(op_params->mb_pack);
}

static int config_smc_cmd_context(const struct tc_call_params *call_params,
	struct tc_op_params *op_params)
{
	int ret;

	ret = init_smc_cmd(call_params, op_params);
	if (ret)
		return ret;

	ret = check_login_for_encrypt(call_params, op_params);
	if (ret)
		return ret;

	ret = load_security_enhance_info(call_params, op_params);
	if (ret)
		tloge("load_security_enhance_info failed\n");

	return ret;
}

static int handle_ta_pending(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, int *tee_ret)
{
	int ret;

	if (*tee_ret != TEEC_PENDING)
		return 0;

	while (*tee_ret == TEEC_PENDING) {
		pend_ca_thread(call_params->sess, op_params->smc_cmd);
		ret = append_teec_token(call_params, op_params);
		if (ret) {
			tloge("append teec's member failed\n");
			return ret;
		}

		*tee_ret = tc_ns_smc_with_no_nr(op_params->smc_cmd);
		ret = post_process_token(call_params, op_params);
		if (ret) {
			tloge("no nr smc post proc token failed\n");
			return ret;
		}
	}

	return ret;
}

static int post_proc_smc_return(const struct tc_call_params *call_params,
	struct tc_op_params *op_params, int tee_ret)
{
	int ret;

	if (tee_ret) {
		tloge("smc call ret 0x%x, cmd ret val 0x%x\n", tee_ret,
			op_params->smc_cmd->ret_val);
		/* same as libteec_vendor, err from TEE, set ret positive */
		ret = EFAULT;
		if (tee_ret == TEEC_CLIENT_INTR)
			ret = -ERESTARTSYS;

		if (tee_ret == TEEC_ERROR_SHORT_BUFFER) {
			if (update_client_operation(call_params,
				op_params, false))
				op_params->smc_cmd->err_origin =
					TEEC_ORIGIN_COMMS;
		}
	} else {
		tz_log_write();
		ret = update_client_operation(call_params, op_params, true);
		if (ret)
			op_params->smc_cmd->err_origin = TEEC_ORIGIN_COMMS;
	}

	return ret;
}

int tc_client_call(const struct tc_call_params *call_params)
{
	int ret;
	int tee_ret = 0;
	struct tc_op_params op_params = { NULL, NULL, {{0}}, {0}, false };

	if (!is_clicall_params_vaild(call_params))
		return -EINVAL;

	if (alloc_for_client_call(&op_params))
		return -ENOMEM;

	op_params.smc_cmd->uid = get_uid_for_cmd();
	if (call_params->context->param_types) {
		ret = alloc_operation(call_params, &op_params);
		if (ret)
			goto free_src;
	}

	ret = config_smc_cmd_context(call_params, &op_params);
	if (ret)
		goto free_src;

	tee_ret = tc_ns_smc(op_params.smc_cmd);
	reset_session_id(call_params, &op_params, tee_ret);

	ret = post_process_token(call_params, &op_params);
	if (ret != EOK) {
		tloge("post process token failed\n");
		op_params.smc_cmd->err_origin = TEEC_ORIGIN_COMMS;
		goto free_src;
	}

	ret = handle_ta_pending(call_params, &op_params, &tee_ret);
	if (ret)
		goto free_src;

	ret = post_proc_smc_return(call_params, &op_params, tee_ret);

free_src:
	release_tc_call_resource(call_params, &op_params, tee_ret);
	return ret;
}
