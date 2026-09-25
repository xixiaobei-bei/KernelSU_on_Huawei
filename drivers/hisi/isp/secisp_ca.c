/*
 * TEEC for secisp
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/dma-buf.h>
#include <linux/err.h>
#include <linux/ion.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/platform_data/remoteproc-hisi.h>
#include <teek_client_api.h>
#include <teek_client_id.h>
#include <securec.h>

#include "secisp_ca.h"

/* teec info */
static TEEC_Session g_secisp_session;
static TEEC_Context g_secisp_context;
static int g_secisp_creat_teec = 0;
static struct mutex g_secisp_tee_lock;

static int get_ion_mem_info(int fd, tz_sg_list **sglist,
			unsigned int *sgl_size, struct device *dev)
{
	struct scatterlist *sg = NULL;
	struct scatterlist *sgl = NULL;
	struct page *page = NULL;
	struct dma_buf *dmabuf = NULL;
	struct sg_table *table = NULL;
	struct dma_buf_attachment *attach = NULL;
	tz_sg_list *tmp_sgl = NULL;
	unsigned int nents = 0;
	int i = 0;

	dmabuf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(dmabuf)) {
		secisp_print_err("dma_buf_get fail, fd=%d, dma_buf=%pK\n", fd, dmabuf);
		return -EINVAL;
	}

	attach = dma_buf_attach(dmabuf, dev);
	if (IS_ERR_OR_NULL(attach)) {
		secisp_print_err("dma_buf_attach error, attach=%pK\n", attach);
		dma_buf_put(dmabuf);
		return -EINVAL;
	}

	table = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR_OR_NULL(table)) {
		secisp_print_err("dma_buf_map_attachment error, table=%pK\n", table);
		dma_buf_detach(dmabuf, attach);
		dma_buf_put(dmabuf);
		return -EINVAL;
	}

	sgl = table->sgl;
	if (sgl == NULL) {
		secisp_print_err("Failed : sgl.NULL\n");
		dma_buf_unmap_attachment(attach, table, DMA_BIDIRECTIONAL);
		dma_buf_detach(dmabuf, attach);
		dma_buf_put(dmabuf);
		return -EFAULT;
	}

	nents = table->nents;
	*sgl_size = sizeof(tz_sg_list) + nents * sizeof(struct ion_page_info);
	tmp_sgl = kzalloc(*sgl_size, GFP_KERNEL);
	if (tmp_sgl == NULL) {
		secisp_print_err("kzalloc tmp_sgl mem failed!\n");
		dma_buf_unmap_attachment(attach, table, DMA_BIDIRECTIONAL);
		dma_buf_detach(dmabuf, attach);
		dma_buf_put(dmabuf);
		return -ENOMEM;
	}

	tmp_sgl->sglist_size = *sgl_size;
	tmp_sgl->info_length = nents;

	for_each_sg(sgl, sg, tmp_sgl->info_length, i) {
		page = sg_page(sg);
		tmp_sgl->page_info[i].phys_addr = page_to_phys(page);
		tmp_sgl->page_info[i].npages = sg->length / PAGE_SIZE;
		secisp_print_err("for_each_sg i=%d, addr=0x%llx, npages=%d\n",
			i, tmp_sgl->page_info[i].phys_addr,
			tmp_sgl->page_info[i].npages);
	}

	*sglist = tmp_sgl;

	dma_buf_unmap_attachment(attach, table, DMA_BIDIRECTIONAL);
	dma_buf_detach(dmabuf, attach);
	dma_buf_put(dmabuf);

	return 0;
}


/*
 * Function name:teek_secisp_open.
 * Discription:Init the TEEC and get the context
 * Parameters:
 *      @ session: the bridge from unsec world to sec world.
 *      @ context: context.
 * return value:
 *      @ 0-->success, others-->failed.
 */
int teek_secisp_open(void)
{
	TEEC_Result result;
	TEEC_UUID svc_uuid = TEE_SERVICE_SECISP_ID;/* Tzdriver id for secisp */
	TEEC_Operation operation = {0};
	char package_name[] = TEE_SECISP_PACKAGE_NAME;
	u32 root_id = TEE_SECISP_UID;
	int ret = 0;

	mutex_lock(&g_secisp_tee_lock);
	result = TEEK_InitializeContext(
			 NULL,
			 &g_secisp_context);
	if (result != TEEC_SUCCESS) {
		secisp_print_err("TEEK_InitializeContext failed, result=0x%x!\n", result);
		ret = -EPERM;
		goto error;
	}

	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(
				       TEEC_NONE,
				       TEEC_NONE,
				       TEEC_MEMREF_TEMP_INPUT,
				       TEEC_MEMREF_TEMP_INPUT);

	operation.params[2].tmpref.buffer	= (void *)(&root_id);
	operation.params[2].tmpref.size		= sizeof(root_id);
	operation.params[3].tmpref.buffer	= (void *)(package_name);
	operation.params[3].tmpref.size		= strlen(package_name) + 1;

	result = TEEK_OpenSession(
			 &g_secisp_context,
			 &g_secisp_session,
			 &svc_uuid,
			 TEEC_LOGIN_IDENTIFY,
			 NULL,
			 &operation,
			 NULL);

	if (result != TEEC_SUCCESS) {
		secisp_print_err("TEEK_OpenSession failed, result=0x%x!\n", result);
		ret = -EPERM;
		TEEK_FinalizeContext(&g_secisp_context);
	}
	g_secisp_creat_teec = 1;
error:
	mutex_unlock(&g_secisp_tee_lock);
	return ret;
}

/*
 * Function name:teek_secisp_close
 * Discription: close secisp ta
 * Parameters:
 * return value:
 *      @ 0-->success, others-->failed.
 */
int teek_secisp_close(void)
{
	int ret = 0;

	mutex_lock(&g_secisp_tee_lock);
	TEEK_CloseSession(&g_secisp_session);
	TEEK_FinalizeContext(&g_secisp_context);
	g_secisp_creat_teec = 0;
	mutex_unlock(&g_secisp_tee_lock);

	return ret;
}

/*
 * Function name:teek_secisp_disreset
 * Discription: load isp img and ispcpu disreset
 * Parameters:
 *      @ buffer:  isp mem info
 * return value:
 *      @ 0-->success, others-->failed.
 */
int teek_secisp_disreset(int sharefd, unsigned int size)
{
	TEEC_Result result;
	TEEC_Operation operation = {0};
	int ret = 0;
	u32 origin = 0;

	if (g_secisp_creat_teec == 0) {
		secisp_print_err("secisp teec not create\n");
		return -EPERM;
	}

	mutex_lock(&g_secisp_tee_lock);
	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(
				       TEEC_ION_SGLIST_INPUT,
				       TEEC_NONE,
				       TEEC_NONE,
				       TEEC_NONE);

	operation.params[0].value.a = sharefd;
	operation.params[0].value.b = size;
	result = TEEK_InvokeCommand(
			 &g_secisp_session,
			 TEE_SECISP_CMD_IMG_DISRESET,
			 &operation,
			 &origin);
	if (result != TEEC_SUCCESS) {
		secisp_print_err("TEEK_InvokeCommand .%d failed, result is 0x%x!\n",
					TEE_SECISP_CMD_IMG_DISRESET, result);
		ret = -EPERM;
	}

	mutex_unlock(&g_secisp_tee_lock);
	return ret;
}

/*
 * Function name:teek_secisp_reset
 * Discription: unmap sec mem and reset
 * Parameters:
 *      @ session: the bridge from unsec world to sec world.
 *      @ buffer:  isp mem info
 * return value:
 *      @ 0-->success, others-->failed.
 */
int teek_secisp_reset(void)
{
	TEEC_Result result;
	TEEC_Operation operation = {0};
	u32 origin = 0;
	int ret = 0;

	if (g_secisp_creat_teec == 0) {
		secisp_print_err("secisp teec not create\n");
		return -EPERM;
	}

	mutex_lock(&g_secisp_tee_lock);
	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(
				       TEEC_NONE,
				       TEEC_NONE,
				       TEEC_NONE,
				       TEEC_NONE);

	result = TEEK_InvokeCommand(
			 &g_secisp_session,
			 TEE_SECISP_CMD_RESET,
			 &operation,
			 &origin);
	if (result != TEEC_SUCCESS) {
		secisp_print_err("TEEK_InvokeCommand .%d failed, result is 0x%x!\n",
					TEE_SECISP_CMD_RESET, result);
		ret = -EPERM;
	}

	mutex_unlock(&g_secisp_tee_lock);
	return ret;
}
/*
 * Function name:teek_secisp_static_mem_map
 * Discription: mem cfg and map
 * Parameters:
 *      @ session: the bridge from unsec world to sec world.
 *      @ buffer:  isp mem info
 * return value:
 *      @ 0-->success, others-->failed.
 */
int teek_secisp_static_mem_map(secisp_mem_info *buffer,
					   unsigned int *iova)
{
	TEEC_Result result;
	TEEC_Operation operation = {0};
	u32 origin = 0;
	int ret = 0;

	if (g_secisp_creat_teec == 0) {
		secisp_print_err("secisp teec not create\n");
		return -EPERM;
	}

	if (buffer == NULL) {
		secisp_print_err("secisp_mem_info is NULL\n");
		return -EPERM;
	}

	mutex_lock(&g_secisp_tee_lock);
	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(
				       TEEC_MEMREF_TEMP_INPUT,
				       TEEC_VALUE_OUTPUT,
				       TEEC_NONE,
				       TEEC_NONE);

	operation.params[0].tmpref.buffer	= buffer;
	operation.params[0].tmpref.size		= sizeof(secisp_mem_info);

	secisp_print_info("do map +");
	result = TEEK_InvokeCommand(
			 &g_secisp_session,
			 TEE_SECISP_STATIC_MEM_MAP,
			 &operation,
			 &origin);
	if (result != TEEC_SUCCESS) {
		secisp_print_err("TEEK_InvokeCommand .%d failed, result is 0x%x!\n",
					TEE_SECISP_STATIC_MEM_MAP, result);
		ret = -EPERM;
	} else {
		*iova = operation.params[1].value.a;
	}

	secisp_print_info("do map -");
	mutex_unlock(&g_secisp_tee_lock);
	return ret;
}

/*
 * Function name:teek_secisp_static_mem_unmap
 * Discription: mem cfg and unmap
 * Parameters:
 *      @ session: the bridge from unsec world to sec world.
 *      @ buffer:  isp mem info
 * return value:
 *      @ 0-->success, others-->failed.
 */

int teek_secisp_static_mem_unmap(secisp_mem_info *buffer)
{
	TEEC_Result result;
	TEEC_Operation operation = {0};
	u32 origin = 0;
	int ret = 0;

	if (g_secisp_creat_teec == 0) {
		secisp_print_err("secisp teec not create\n");
		return -EPERM;
	}

	if (buffer == NULL) {
		secisp_print_err("secisp_mem_info is NULL\n");
		return -EPERM;
	}

	mutex_lock(&g_secisp_tee_lock);
	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(
				       TEEC_MEMREF_TEMP_INPUT,
				       TEEC_NONE,
				       TEEC_NONE,
				       TEEC_NONE);

	operation.params[0].tmpref.buffer	= buffer;
	operation.params[0].tmpref.size		= sizeof(secisp_mem_info);

	secisp_print_info("do unmap +");
	result = TEEK_InvokeCommand(
			 &g_secisp_session,
			 TEE_SECISP_STATIC_MEM_UNMAP,
			 &operation,
			 &origin);
	if (result != TEEC_SUCCESS) {
		secisp_print_err("TEEK_InvokeCommand .%d failed, result is 0x%x!\n",
					TEE_SECISP_STATIC_MEM_UNMAP, result);
		ret = -EPERM;
	}

	secisp_print_info("do unmap -");
	mutex_unlock(&g_secisp_tee_lock);
	return ret;
}

/*
 * Function name:teek_secisp_mem_map
 * Discription: mem cfg and map
 * Parameters:
 *      @ session: the bridge from unsec world to sec world.
 *      @ buffer:  isp mem info
 * return value:
 *      @ 0-->success, others-->failed.
 */
int teek_secisp_mem_map(secisp_mem_info *buffer, unsigned int *iova)
{
	TEEC_Result result;
	TEEC_Operation operation = {0};
	u32 origin = 0;
	int ret = 0;

	if (g_secisp_creat_teec == 0) {
		secisp_print_err("secisp teec not create\n");
		return -EPERM;
	}

	if (buffer == NULL) {
		secisp_print_err("secisp_mem_info is NULL\n");
		return -EPERM;
	}

	mutex_lock(&g_secisp_tee_lock);
	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(
				       TEEC_ION_SGLIST_INPUT,
				       TEEC_MEMREF_TEMP_INPUT,
				       TEEC_VALUE_OUTPUT,
				       TEEC_NONE);

	operation.params[0].ionref.ion_share_fd	= buffer->sharefd;
	operation.params[0].ionref.ion_size	= buffer->size;
	operation.params[1].tmpref.buffer	= buffer;
	operation.params[1].tmpref.size		= sizeof(secisp_mem_info);

	secisp_print_info("do map +");
	result = TEEK_InvokeCommand(
			 &g_secisp_session,
			 TEE_SECISP_MEM_CFG_AND_MAP,
			 &operation,
			 &origin);
	if (result != TEEC_SUCCESS) {
		secisp_print_err("TEEK_InvokeCommand .%d failed, result is 0x%x!\n",
					TEE_SECISP_MEM_CFG_AND_MAP, result);
		ret = -EPERM;
	} else {
		*iova = operation.params[2].value.a;
	}

	secisp_print_info("do map -");
	mutex_unlock(&g_secisp_tee_lock);
	return ret;
}

/*
 * Function name:teek_secisp_mem_unmap
 * Discription: mem cfg and unmap
 * Parameters:
 *      @ session: the bridge from unsec world to sec world.
 *      @ buffer:  isp mem info
 * return value:
 *      @ 0-->success, others-->failed.
 */

int teek_secisp_mem_unmap(secisp_mem_info *buffer)
{
	TEEC_Result result;
	TEEC_Operation operation = {0};
	u32 origin = 0;
	int ret = 0;

	if (g_secisp_creat_teec == 0) {
		secisp_print_err("secisp teec not create\n");
		return -EPERM;
	}

	if (buffer == NULL) {
		secisp_print_err("secisp_mem_info is NULL\n");
		return -EPERM;
	}

	mutex_lock(&g_secisp_tee_lock);
	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(
				       TEEC_ION_SGLIST_INPUT,
				       TEEC_MEMREF_TEMP_INPUT,
				       TEEC_NONE,
				       TEEC_NONE);

	operation.params[0].ionref.ion_share_fd	= buffer->sharefd;
	operation.params[0].ionref.ion_size	= buffer->size;
	operation.params[1].tmpref.buffer	= buffer;
	operation.params[1].tmpref.size		= sizeof(secisp_mem_info);

	secisp_print_info("do unmap +");
	result = TEEK_InvokeCommand(
			 &g_secisp_session,
			 TEE_SECISP_MEM_CFG_AND_UNMAP,
			 &operation,
			 &origin);
	if (result != TEEC_SUCCESS) {
		secisp_print_err("TEEK_InvokeCommand .%d failed, result is 0x%x!\n",
					TEE_SECISP_MEM_CFG_AND_UNMAP, result);
		ret = -EPERM;
	}

	secisp_print_info("do unmap -");
	mutex_unlock(&g_secisp_tee_lock);
	return ret;
}

/*
 * Function name:teek_secisp_dynamic_mem_map
 * Discription: mem cfg and map
 * Parameters:
 *      @ session: the bridge from unsec world to sec world.
 *      @ buffer:  isp mem info
 * return value:
 *      @ 0-->success, others-->failed.
 */
int teek_secisp_dynamic_mem_map(secisp_mem_info *buffer,
				unsigned int *iova, struct device *dev)
{
	TEEC_Result result;
	TEEC_Operation operation = {0};
	u32 origin = 0;
	int ret = 0;
	unsigned int dynamic_sgl_size = 0;
	tz_sg_list *dynamic_sgl = NULL;

	if (g_secisp_creat_teec == 0) {
		secisp_print_err("secisp teec not create\n");
		return -EPERM;
	}

	if (buffer == NULL) {
		secisp_print_err("secisp_mem_info is NULL\n");
		return -EPERM;
	}

	if (dev == NULL) {
		secisp_print_err("dev is NULL\n");
		return -EPERM;
	}

	ret = get_ion_mem_info(buffer->sharefd, &dynamic_sgl,
				&dynamic_sgl_size, dev);
	if (ret < 0) {
		secisp_print_err("get_ion_mem_info fail\n");
		return -EPERM;
	}

	dynamic_sgl->ion_size = buffer->size;

	mutex_lock(&g_secisp_tee_lock);
	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(
				       TEEC_MEMREF_TEMP_INPUT,
				       TEEC_MEMREF_TEMP_INPUT,
				       TEEC_VALUE_OUTPUT,
				       TEEC_NONE);

	operation.params[0].tmpref.buffer	= dynamic_sgl;
	operation.params[0].tmpref.size		= dynamic_sgl->sglist_size;
	operation.params[1].tmpref.buffer	= buffer;
	operation.params[1].tmpref.size		= sizeof(secisp_mem_info);

	secisp_print_info("do map +");
	result = TEEK_InvokeCommand(
			 &g_secisp_session,
			 TEE_SECISP_DYNAMIC_MEM_MAP,
			 &operation,
			 &origin);
	if (result != TEEC_SUCCESS) {
		secisp_print_err("TEEK_InvokeCommand .%d failed, result is 0x%x!\n",
					TEE_SECISP_DYNAMIC_MEM_MAP, result);
		ret = -EPERM;
	} else {
		*iova = operation.params[2].value.a;
	}

	kfree(dynamic_sgl);
	dynamic_sgl = NULL;
	secisp_print_info("do map -");
	mutex_unlock(&g_secisp_tee_lock);
	return ret;
}

/*
 * Function name:teek_secisp_dynamic_mem_unmap
 * Discription: mem cfg and unmap
 * Parameters:
 *      @ session: the bridge from unsec world to sec world.
 *      @ buffer:  isp mem info
 * return value:
 *      @ 0-->success, others-->failed.
 */
int teek_secisp_dynamic_mem_unmap(secisp_mem_info *buffer,
						struct device *dev)
{
	TEEC_Result result;
	TEEC_Operation operation = {0};
	u32 origin = 0;
	int ret = 0;
	unsigned int dynamic_sgl_size = 0;
	tz_sg_list *dynamic_sgl = NULL;

	if (g_secisp_creat_teec == 0) {
		secisp_print_err("secisp teec not create\n");
		return -EPERM;
	}
	if (buffer == NULL) {
		secisp_print_err("secisp_mem_info is NULL\n");
		return -EPERM;
	}

	if (dev == NULL) {
		secisp_print_err("dev is NULL\n");
		return -EPERM;
	}

	ret = get_ion_mem_info(buffer->sharefd, &dynamic_sgl,
				&dynamic_sgl_size, dev);
	if (ret < 0) {
		secisp_print_err("get_ion_mem_info fail\n");
		return -EPERM;
	}

	dynamic_sgl->ion_size = buffer->size;

	mutex_lock(&g_secisp_tee_lock);
	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(
				       TEEC_MEMREF_TEMP_INPUT,
				       TEEC_MEMREF_TEMP_INPUT,
				       TEEC_NONE,
				       TEEC_NONE);

	operation.params[0].tmpref.buffer	= dynamic_sgl;
	operation.params[0].tmpref.size		= dynamic_sgl->sglist_size;
	operation.params[1].tmpref.buffer	= buffer;
	operation.params[1].tmpref.size		= sizeof(secisp_mem_info);

	secisp_print_info("do unmap +");
	result = TEEK_InvokeCommand(
			 &g_secisp_session,
			 TEE_SECISP_DYNAMIC_MEM_UNMAP,
			 &operation,
			 &origin);
	if (result != TEEC_SUCCESS) {
		secisp_print_err("TEEK_InvokeCommand .%d failed, result is 0x%x!\n",
					TEE_SECISP_DYNAMIC_MEM_UNMAP, result);
		ret = -EPERM;
	}

	kfree(dynamic_sgl);
	dynamic_sgl = NULL;
	secisp_print_info("do unmap -");
	mutex_unlock(&g_secisp_tee_lock);
	return ret;
}

/*
 * Function name:teek_secisp_ca_probe
 * Discription: sec isp ca init
 * Parameters:
 * return value:
 */
void teek_secisp_ca_probe(void)
{
	mutex_init(&g_secisp_tee_lock);
	g_secisp_creat_teec = 0;
}
/*
 * Function name:teek_secisp_ca_remove
 * Discription: sec isp ca remove
 * Parameters:
 * return value:
 */
void teek_secisp_ca_remove(void)
{
	g_secisp_creat_teec = 0;
	mutex_destroy(&g_secisp_tee_lock);
}
