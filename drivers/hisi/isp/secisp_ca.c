/*
 * TEEC for secisp
 *
 * Copyright (c) 2019 ISP Technologies CO., Ltd.
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
#include <linux/platform_data/remoteproc_hisp.h>
#include <teek_client_api.h>
#include <teek_client_id.h>
#include <securec.h>
#include <dynamic_mem.h>

/* info */
#define secisp_err(fmt, ...)     (printk(KERN_ERR "[sec]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#define secisp_info(fmt, ...)    (printk(KERN_INFO "[sec]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#define secisp_debug(fmt, ...)   (printk(KERN_DEBUG "[sec]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
/* CA info */
#define TEE_SERVICE_SECISP_ID           TEE_SERVICE_SECISP
#define TEE_SECISP_PACKAGE_NAME         "/vendor/bin/CameraDaemon"
#define TEE_SECISP_UID                  1013

/* CA cmd */
enum svc_secisp_cmd_id {
	TEE_SECISP_CMD_IMG_DISRESET        = 0,
	TEE_SECISP_CMD_RESET               = 1,
	TEE_SECISP_SEC_MEM_CFG_AND_MAP     = 2,
	TEE_SECISP_SEC_MEM_CFG_AND_UNMAP   = 3,
	TEE_SECISP_NONSEC_MEM_MAP_SEC      = 4,
	TEE_SECISP_NONSEC_MEM_UNMAP_SEC    = 5,
	TEE_SECISP_BOOT_MEM_CFG_AND_MAP    = 6,
	TEE_SECISP_BOOT_MEM_CFG_AND_UNMAP  = 7,
	TEE_SECISP_CMD_MAX
};

struct secisp_img_mem_info {
	struct secisp_mem_info info;
	unsigned int sfd;
};

struct secisp_boot_mem_info {
	struct secisp_mem_info rsv_info[HISP_SEC_RSV_MAX_TYPE];
	struct secisp_img_mem_info img_info[HISP_SEC_BOOT_MAX_TYPE];
};

struct secisp_dynamic_mem_info {
	struct dma_buf *dmabuf;
	struct sg_table *table;
	struct dma_buf_attachment *attach;
	unsigned int nents;
};

/* teec info */
static TEEC_Session g_secisp_session;
static TEEC_Context g_secisp_context;
static int g_secisp_creat_teec;
static struct mutex g_secisp_tee_lock;
static struct secisp_dynamic_mem_info g_info;
static struct sglist *g_sgl;
static struct secisp_boot_mem_info *g_buffer;
static u64 g_pool_sfd[SECISP_MAX_TYPE];

static int secisp_sgl_info_get(int fd, struct device *dev,
	struct scatterlist **sgl, struct secisp_dynamic_mem_info *info)
{
	info->dmabuf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(info->dmabuf)) {
		secisp_err("dma_buf_get fail, fd=%d, dma_buf=%pK\n",
			fd, info->dmabuf);
		return -EINVAL;
	}

	info->attach = dma_buf_attach(info->dmabuf, dev);
	if (IS_ERR_OR_NULL(info->attach)) {
		secisp_err("dma_buf_attach error, attach=%pK\n", info->attach);
		dma_buf_put(info->dmabuf);
		return -EINVAL;
	}

	info->table = dma_buf_map_attachment(info->attach, DMA_BIDIRECTIONAL);
	if (IS_ERR_OR_NULL(info->table)) {
		secisp_err("dma_buf_map_attachment error\n");
		dma_buf_detach(info->dmabuf, info->attach);
		dma_buf_put(info->dmabuf);
		return -EINVAL;
	}

	if (info->table->sgl == NULL) {
		secisp_err("Failed : sgl.NULL\n");
		dma_buf_unmap_attachment(info->attach, info->table,
			DMA_BIDIRECTIONAL);
		dma_buf_detach(info->dmabuf, info->attach);
		dma_buf_put(info->dmabuf);
		return -EINVAL;
	}

	*sgl = info->table->sgl;
	info->nents = info->table->nents;

	return 0;
}

static void secisp_sgl_info_put(struct secisp_dynamic_mem_info *info)
{
	if (info == NULL) {
		secisp_err("Failed : info.NULL\n");
		return;
	}

	dma_buf_unmap_attachment(info->attach, info->table, DMA_BIDIRECTIONAL);
	dma_buf_detach(info->dmabuf, info->attach);
	dma_buf_put(info->dmabuf);
}

static int get_ion_mem_info(int fd, struct sglist **sglist,
		struct device *dev)
{
	struct scatterlist *sg = NULL;
	struct scatterlist *sgl = NULL;
	struct sglist *tmp_sgl = NULL;
	unsigned int sgl_size;
	int i = 0;
	int ret;

	ret = secisp_sgl_info_get(fd, dev, &sgl, &g_info);
	if (ret) {
		secisp_err("Failed : secisp_get_sgl_info\n");
		return -EFAULT;
	}

	sgl_size = sizeof(struct sglist) +
		g_info.nents * sizeof(struct ion_page_info);
	tmp_sgl = vmalloc(sgl_size);
	if (tmp_sgl == NULL) {
		secisp_err("alloc tmp_sgl mem failed!\n");
		secisp_sgl_info_put(&g_info);
		return -ENOMEM;
	}

	tmp_sgl->sglist_size = sgl_size;
	tmp_sgl->info_length = g_info.nents;

	for_each_sg(sgl, sg, tmp_sgl->info_length, i) {
		tmp_sgl->page_info[i].phys_addr = page_to_phys(sg_page(sg));
		tmp_sgl->page_info[i].npages = sg->length / PAGE_SIZE;
	}

	*sglist = tmp_sgl;

	return 0;
}

static int secisp_boot_rsv_mem_cfg(
	struct secisp_boot_mem_info *buffer, struct hisp_sec *dev)
{
	int i, ret;

	for (i = 0; i < HISP_SEC_RSV_MAX_TYPE; i++) {
		ret = memcpy_s(&buffer->rsv_info[i],
			sizeof(struct secisp_mem_info),
			&dev->rsv_mem[i], sizeof(struct secisp_mem_info));
		if (ret != 0) {
			secisp_err("memcpy_s rsv memory fail, index.%d!\n", i);
			return ret;
		}
	}

	return 0;
}

static int secisp_boot_img_mem_cfg(
	struct secisp_boot_mem_info *buffer, struct hisp_sec *dev)
{
	struct sg_table *table = NULL;
	enum SEC_SVC type = 0;
	u64 sfd = 0;
	int i, ret;

	for (i = 0; i < HISP_SEC_BOOT_MAX_TYPE; i++) {
		ret = memcpy_s(&buffer->img_info[i].info,
			sizeof(struct secisp_mem_info),
			&dev->boot_mem[i], sizeof(struct secisp_mem_info));
		if (ret != 0) {
			secisp_err("memcpy_s boot memory fail, index.%d!\n", i);
			return ret;
		}

		ret = secmem_get_buffer(buffer->img_info[i].info.sharefd,
			&table, &sfd, &type);
		if (ret != 0) {
			secisp_err("secmem_get_buffer fail. ret.%d!\n", ret);
			return -EFAULT;
		}
		buffer->img_info[i].sfd = (unsigned int)sfd;
	}

	return 0;
}

static int secisp_boot_mem_info_cfg(
	struct secisp_boot_mem_info *buffer, struct hisp_sec *dev)
{
	int ret;

	if (buffer == NULL || dev == NULL) {
		secisp_err("buffer or dev is NULL!\n");
		return -EPERM;
	}

	ret = secisp_boot_rsv_mem_cfg(buffer, dev);
	if (ret < 0) {
		secisp_err("secisp_boot_rsv_mem_cfg fail. ret.%d!\n", ret);
		return ret;
	}

	ret = secisp_boot_img_mem_cfg(buffer, dev);
	if (ret < 0)
		secisp_err("secisp_boot_img_mem_cfg fail. ret.%d!\n", ret);

	return ret;
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
	result = TEEK_InitializeContext(NULL, &g_secisp_context);
	if (result != TEEC_SUCCESS) {
		secisp_err("TEEK_InitializeContext failed.0x%x!\n", result);
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

	operation.params[2].tmpref.buffer = (void *)(&root_id);
	operation.params[2].tmpref.size   = sizeof(root_id);
	operation.params[3].tmpref.buffer = (void *)(package_name);
	operation.params[3].tmpref.size   = strlen(package_name) + 1;

	result = TEEK_OpenSession(
			&g_secisp_context,
			&g_secisp_session,
			&svc_uuid,
			TEEC_LOGIN_IDENTIFY,
			NULL,
			&operation,
			NULL);

	if (result != TEEC_SUCCESS) {
		secisp_err("TEEK_OpenSession failed, result=0x%x!\n", result);
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
 * return value:
 *      @ 0-->success, others-->failed.
 */
int teek_secisp_disreset(void)
{
	TEEC_Result result;
	TEEC_Operation operation = {0};
	int ret = 0;
	u32 origin = 0;

	if (g_secisp_creat_teec == 0) {
		secisp_err("secisp teec not create\n");
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
			TEE_SECISP_CMD_IMG_DISRESET,
			&operation,
			&origin);
	if (result != TEEC_SUCCESS) {
		secisp_err("TEEK_InvokeCommand .%d failed, result is 0x%x!\n",
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
		secisp_err("secisp teec not create\n");
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
		secisp_err("TEEK_InvokeCommand .%d failed, result is 0x%x!\n",
			TEE_SECISP_CMD_RESET, result);
		ret = -EPERM;
	}

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
int teek_secisp_mem_map(struct secisp_mem_info *buffer)
{
	TEEC_Result result;
	TEEC_Operation operation = {0};
	u32 origin = 0;
	struct sg_table *table = NULL;
	enum SEC_SVC type = 0;
	u64 sfd = 0;
	int ret = 0;

	if (g_secisp_creat_teec == 0) {
		secisp_err("secisp teec not create\n");
		return -EPERM;
	}

	if (buffer == NULL) {
		secisp_err("secisp_mem_info is NULL\n");
		return -EPERM;
	}

	ret = secmem_get_buffer(buffer->sharefd, &table, &sfd, &type);
	if (ret != 0) {
		secisp_err("secmem_get_buffer fail. ret.%d!\n", ret);
		return -EFAULT;
	}

	g_pool_sfd[buffer->type] = sfd;
	mutex_lock(&g_secisp_tee_lock);
	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(
					TEEC_VALUE_INPUT,
					TEEC_MEMREF_TEMP_INPUT,
					TEEC_NONE,
					TEEC_NONE);

	operation.params[0].value.a       = (unsigned int)sfd;
	operation.params[0].value.b       = buffer->size;
	operation.params[1].tmpref.buffer = buffer;
	operation.params[1].tmpref.size   = sizeof(struct secisp_mem_info);

	secisp_info("do map +");
	result = TEEK_InvokeCommand(
			&g_secisp_session,
			TEE_SECISP_SEC_MEM_CFG_AND_MAP,
			&operation,
			&origin);
	if (result != TEEC_SUCCESS) {
		secisp_err("TEEK_InvokeCommand .%d failed, result is 0x%x!\n",
			TEE_SECISP_SEC_MEM_CFG_AND_MAP, result);
		ret = -EPERM;
	}

	secisp_info("do map -");
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

int teek_secisp_mem_unmap(struct secisp_mem_info *buffer)
{
	TEEC_Result result;
	TEEC_Operation operation = {0};
	u32 origin = 0;
	u64 sfd = 0;
	int ret = 0;

	if (g_secisp_creat_teec == 0) {
		secisp_err("secisp teec not create\n");
		return -EPERM;
	}

	if (buffer == NULL) {
		secisp_err("secisp_mem_info is NULL\n");
		return -EPERM;
	}

	sfd = g_pool_sfd[buffer->type];
	mutex_lock(&g_secisp_tee_lock);
	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(
					TEEC_VALUE_INPUT,
					TEEC_MEMREF_TEMP_INPUT,
					TEEC_NONE,
					TEEC_NONE);

	operation.params[0].value.a       = (unsigned int)sfd;
	operation.params[0].value.b       = buffer->size;
	operation.params[1].tmpref.buffer = buffer;
	operation.params[1].tmpref.size   = sizeof(struct secisp_mem_info);

	secisp_info("do unmap +");
	result = TEEK_InvokeCommand(
			&g_secisp_session,
			TEE_SECISP_SEC_MEM_CFG_AND_UNMAP,
			&operation,
			&origin);
	if (result != TEEC_SUCCESS) {
		secisp_err("TEEK_InvokeCommand .%d failed, result is 0x%x!\n",
			TEE_SECISP_SEC_MEM_CFG_AND_UNMAP, result);
		ret = -EPERM;
	}

	secisp_info("do unmap -");
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
int teek_secisp_dynamic_mem_map(
	struct secisp_mem_info *buffer, struct device *dev)
{
	TEEC_Result result;
	TEEC_Operation operation = {0};
	u32 origin = 0;
	int ret = 0;

	if (g_secisp_creat_teec == 0) {
		secisp_err("secisp teec not create\n");
		return -EPERM;
	}

	if (buffer == NULL || dev == NULL) {
		secisp_err("secisp_mem_info or dev is NULL\n");
		return -EPERM;
	}

	ret = get_ion_mem_info(buffer->sharefd, &g_sgl, dev);
	if (ret < 0) {
		secisp_err("get_ion_mem_info fail\n");
		return -EPERM;
	}

	g_sgl->ion_size = buffer->size; /*lint !e613 */

	mutex_lock(&g_secisp_tee_lock);
	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
		TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE);

	operation.params[0].tmpref.buffer = g_sgl;
	operation.params[0].tmpref.size   = g_sgl->sglist_size;
	operation.params[1].tmpref.buffer = buffer;
	operation.params[1].tmpref.size	  = sizeof(struct secisp_mem_info);

	secisp_info("do map +");
	result = TEEK_InvokeCommand(&g_secisp_session,
		TEE_SECISP_NONSEC_MEM_MAP_SEC, &operation, &origin);
	if (result != TEEC_SUCCESS) {
		secisp_err("TEEK_InvokeCommand .%d failed, result is 0x%x!\n",
			TEE_SECISP_NONSEC_MEM_MAP_SEC, result);
		vfree(g_sgl);
		g_sgl = NULL;
		secisp_sgl_info_put(&g_info);
		ret = -EPERM;
	}

	secisp_info("do map -");
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
int teek_secisp_dynamic_mem_unmap(
	struct secisp_mem_info *buffer, struct device *dev)
{
	TEEC_Result result;
	TEEC_Operation operation = {0};
	u32 origin = 0;
	int ret = 0;

	if (g_secisp_creat_teec == 0) {
		secisp_err("secisp teec not create\n");
		return -EPERM;
	}

	if (buffer == NULL || dev == NULL) {
		secisp_err("secisp_mem_info or dev is NULL\n");
		return -EPERM;
	}

	mutex_lock(&g_secisp_tee_lock);
	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
		TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE);

	operation.params[0].tmpref.buffer = g_sgl;
	operation.params[0].tmpref.size   = g_sgl->sglist_size;
	operation.params[1].tmpref.buffer = buffer;
	operation.params[1].tmpref.size   = sizeof(struct secisp_mem_info);

	secisp_info("do unmap +");
	result = TEEK_InvokeCommand(&g_secisp_session,
		TEE_SECISP_NONSEC_MEM_UNMAP_SEC, &operation, &origin);
	if (result != TEEC_SUCCESS) {
		secisp_err("TEEK_InvokeCommand .%d failed, result is 0x%x!\n",
			TEE_SECISP_NONSEC_MEM_UNMAP_SEC, result);
		ret = -EPERM;
	}

	vfree(g_sgl);
	g_sgl = NULL;
	secisp_sgl_info_put(&g_info);
	secisp_info("do unmap -");
	mutex_unlock(&g_secisp_tee_lock);
	return ret;
}

/*
 * Function name:teek_secisp_boot_mem_map
 * Discription: boot mem map
 * Parameters:
 *      @ session: the bridge from unsec world to sec world.
 *      @ buffer:  boot mem info
 * return value:
 *      @ 0-->success, others-->failed.
 */
static int teek_secisp_boot_mem_map(struct secisp_boot_mem_info *buffer)
{
	TEEC_Result result;
	TEEC_Operation operation = {0};
	u32 origin = 0;
	int ret = 0;

	if (g_secisp_creat_teec == 0) {
		secisp_err("secisp teec not create\n");
		return -EPERM;
	}

	if (buffer == NULL) {
		secisp_err("secisp_boot_mem_info is NULL\n");
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

	operation.params[0].tmpref.buffer = buffer;
	operation.params[0].tmpref.size   = sizeof(struct secisp_boot_mem_info);

	secisp_info("do map +");
	result = TEEK_InvokeCommand(
			&g_secisp_session,
			TEE_SECISP_BOOT_MEM_CFG_AND_MAP,
			&operation,
			&origin);
	if (result != TEEC_SUCCESS) {
		secisp_err("TEEK_InvokeCommand .%d failed, result is 0x%x!\n",
			TEE_SECISP_BOOT_MEM_CFG_AND_MAP, result);
		ret = -EPERM;
	}

	secisp_info("do map -");
	mutex_unlock(&g_secisp_tee_lock);
	return ret;
}

/*
 * Function name:teek_secisp_boot_mem_unmap
 * Discription: boot mem unmap
 * Parameters:
 *      @ session: the bridge from unsec world to sec world.
 *      @ buffer:  boot mem info
 * return value:
 *      @ 0-->success, others-->failed.
 */
static int teek_secisp_boot_mem_unmap(struct secisp_boot_mem_info *buffer)
{
	TEEC_Result result;
	TEEC_Operation operation = {0};
	u32 origin = 0;
	int ret = 0;

	if (g_secisp_creat_teec == 0) {
		secisp_err("secisp teec not create\n");
		return -EPERM;
	}

	if (buffer == NULL) {
		secisp_err("secisp_boot_mem_info is NULL\n");
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

	operation.params[0].tmpref.buffer = buffer;
	operation.params[0].tmpref.size   = sizeof(struct secisp_boot_mem_info);

	secisp_info("do unmap +");
	result = TEEK_InvokeCommand(
			&g_secisp_session,
			TEE_SECISP_BOOT_MEM_CFG_AND_UNMAP,
			&operation,
			&origin);
	if (result != TEEC_SUCCESS) {
		secisp_err("TEEK_InvokeCommand .%d failed, result is 0x%x!\n",
			TEE_SECISP_BOOT_MEM_CFG_AND_UNMAP, result);
		ret = -EPERM;
	}

	secisp_info("do unmap -");
	mutex_unlock(&g_secisp_tee_lock);
	return ret;
}

/*
 * Function name:teek_secisp_boot_mem_cfg
 * Discription: boot mem cfg
 * Parameters:
 *      @ session: the bridge from unsec world to sec world.
 *      @ dev:  hisp_sec dev
 * return value:
 *      @ 0-->success, others-->failed.
 */
int teek_secisp_boot_mem_config(struct hisp_sec *dev)
{
	int ret;

	if (dev == NULL) {
		secisp_err("hisp_sec dev is NULL\n");
		return -EPERM;
	}

	g_buffer = kzalloc(sizeof(struct secisp_boot_mem_info), GFP_KERNEL);
	if (g_buffer == NULL)
		return -ENOMEM;

	ret = secisp_boot_mem_info_cfg(g_buffer, dev);
	if (ret < 0) {
		secisp_err("secisp_boot_mem_info_cfg fail, ret.%d\n", ret);
		kfree(g_buffer);
		g_buffer = NULL;
		return ret;
	}

	ret = teek_secisp_boot_mem_map(g_buffer);
	if (ret < 0) {
		secisp_err("secisp_boot_mem_info_cfg fail, ret.%d\n", ret);
		kfree(g_buffer);
		g_buffer = NULL;
		return ret;
	}

	return 0;
}

/*
 * Function name:teek_secisp_boot_mem_deconfig
 * Discription: boot mem decfg
 * Parameters:
 *      @ session: the bridge from unsec world to sec world.
 *      @ dev:  hisp_sec dev
 * return value:
 *      @ 0-->success, others-->failed.
 */
int teek_secisp_boot_mem_deconfig(struct hisp_sec *dev)
{
	int ret;

	if (dev == NULL) {
		secisp_err("hisp_sec dev is NULL\n");
		return -EPERM;
	}

	ret = teek_secisp_boot_mem_unmap(g_buffer);
	if (ret < 0) {
		secisp_err("teek_secisp_boot_mem_unmap fail, ret.%d\n", ret);
		kfree(g_buffer);
		g_buffer = NULL;
		return ret;
	}

	kfree(g_buffer);
	g_buffer = NULL;
	return 0;
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
