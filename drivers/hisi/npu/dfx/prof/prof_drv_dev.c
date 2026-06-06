#include <linux/err.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/barrier.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include "drv_log.h"
#include "npu_dfx.h"
#include "npu_dfx_cq.h"
#include "npu_dfx_sq.h"
#include "prof_drv_dev.h"
#include "npu_manager_common.h"
#include "npu_mailbox_msg.h"
#include "npu_ioctl_services.h"
#include <linux/io.h>

prof_char_dev_global_info_t g_prof_dev_info;

int prof_tscpu_sync_rw_ptr(int channel_id, int read1_write0);

void flush_cache(unsigned char *base, unsigned int len)
{
	int i;
	int num = len / CACHE_LINE_LEN;

	i = ((len % CACHE_LINE_LEN) > 0) ? num++ : num;
	num = i;

	asm volatile ("dsb st" : : : "memory");
	for (i = 0; i < num; i++) {
		asm volatile ("DC CIVAC ,%x0" ::"r" (base + i * CACHE_LINE_LEN));
		mb();
	}

	asm volatile ("dsb st" : : : "memory");
}

int prof_get_channel_index(int channel_id, unsigned int *channel_idx)
{
	int ret = 0;
	if (channel_id == CHANNEL_AICPU0) {
		*channel_idx  = PROF_CHANNEL_AICPU0_IDX;
	} else if (channel_id == CHANNEL_AICORE) {
		*channel_idx  = PROF_CHANNEL_AICORE_IDX;
	} else if (channel_id == CHANNEL_TSFW) {
		*channel_idx  = PROF_CHANNEL_TSFW_IDX;
	} else {
		ret = -1;
	}

	return ret;
}

STATIC char* prof_devnode(struct device *dev, umode_t *mode)
{
	if(mode != NULL)
		*mode = 0666;
	return NULL;
}


int prof_ts_save_file(struct prof_channel_info *channel_info)
{
	int ret;
	unsigned int size = 0;
	unsigned int read_ptr = 0;
	unsigned int write_ptr = 0;
	struct prof_device_info *prof_device = NULL;
	struct file *filp = NULL;
	struct prof_data_head *data_head = NULL;
	unsigned char *base = NULL;
	loff_t offset = 0;
	unsigned int   all_size = 0;

	if (channel_info->channel_state != CHANNEL_ENABLE ||
		channel_info->vir_addr == NULL) {
		prof_err("channel id(%u) is disable.\n", channel_info->channel_id);
		return PROF_ERROR;
	}

	data_head = (struct prof_data_head *)channel_info->vir_addr;

	if (channel_info->real_time == PROF_REAL)
		return PROF_OK;

	prof_device = &g_prof_dev_info.prof_device;
	mutex_lock(&prof_device->ts_file_mutex);

	flush_cache(channel_info->vir_addr, channel_info->buf_len);

	read_ptr  = data_head->read_ptr;
	write_ptr = atomic_read((atomic_t *)&data_head->write_ptr);

	if (write_ptr == read_ptr) {
		prof_err("channel id(%u) file(%s) write_ptr(%u) = read_ptr.\n",
			 channel_info->channel_id, channel_info->prof_file, write_ptr);
		mutex_unlock(&prof_device->ts_file_mutex);
		return PROF_OK;
	}

	filp = filp_open(channel_info->prof_file, O_RDWR | O_APPEND | O_CREAT, 0644);
	if (IS_ERR(filp)) {
		prof_err("unable to open file: %s (%ld)\n", channel_info->prof_file, PTR_ERR(filp));
		mutex_unlock(&prof_device->ts_file_mutex);
		return PROF_ERROR;
	}

	if (write_ptr > read_ptr) {
		size = write_ptr - read_ptr;
		base = channel_info->vir_addr + DATA_BUF_RESERVED + read_ptr;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0))
		ret = kernel_write(filp, (char *)base, size, offset);
#else
		ret = kernel_write(filp, (char *)base, size, &offset);
#endif
		if (ret < 0) {
			prof_err("file(%s) write failed(%d).\n", channel_info->prof_file, ret);
			filp_close(filp, NULL);
			mutex_unlock(&prof_device->ts_file_mutex);
			return ret;
		}

		filp_close(filp, NULL);
		atomic_set((atomic_t *)&data_head->read_ptr, write_ptr);
		flush_cache(channel_info->vir_addr, CACHE_LINE_LEN);
		mutex_unlock(&prof_device->ts_file_mutex);

		prof_info("file(%s) write len(%d).\n", channel_info->prof_file, size);

		return PROF_OK;
	}

	if (read_ptr < (channel_info->buf_len - DATA_BUF_RESERVED)) {
		size = channel_info->buf_len - DATA_BUF_RESERVED - read_ptr;
		base = channel_info->vir_addr + DATA_BUF_RESERVED + read_ptr;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0))
		ret = kernel_write(filp, (char *)base, size, offset);
#else
		ret = kernel_write(filp, (char *)base, size, &offset);
#endif
		if (ret < 0) {
			prof_err("file(%s) write failed(%d).\n", channel_info->prof_file, ret);
			filp_close(filp, NULL);
			mutex_unlock(&prof_device->ts_file_mutex);
			return ret;
		}

		all_size = size;
	}

	offset = 0;
	if (write_ptr > 0) {
		size = write_ptr;
		base = channel_info->vir_addr + DATA_BUF_RESERVED;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0))
		ret = kernel_write(filp, (char *)base, size, offset);
#else
		ret = kernel_write(filp, (char *)base, size, &offset);
#endif
		if (ret < 0) {
			prof_err("file(%s) write failed(%d).\n", channel_info->prof_file, ret);
			filp_close(filp, NULL);
			mutex_unlock(&prof_device->ts_file_mutex);
			return ret;
		}
	}

	filp_close(filp, NULL);
	atomic_set((atomic_t *)&data_head->read_ptr, write_ptr);
	flush_cache(channel_info->vir_addr, CACHE_LINE_LEN);
	mutex_unlock(&prof_device->ts_file_mutex);

	all_size += size;
	prof_info("file(%s) write len(%d).\n", channel_info->prof_file, all_size);

	return PROF_OK;
}

void prof_sq_report_profile(const unsigned char *cq_buf, const unsigned char *sq_buf)
{
	unsigned int channel_id;
	struct prof_cq_scheduler *cq_scheduler = NULL;
	struct prof_channel_info *channel_info = NULL;
	unsigned int channel_idx = PROF_CHANNEL_NUM;

	if ((cq_buf == NULL) || (sq_buf == NULL)) {
		prof_err("cq_buf or sq_buf is null.\n");
		return;
	}

	cq_scheduler = (struct prof_cq_scheduler *)cq_buf;
	channel_id = cq_scheduler->channel_id;

	if (channel_id >= PROF_CHANNEL_MAX) {
		prof_err("channel_id is fail. channel_id: %u\n", channel_id);
		return;
	}

	if (prof_get_channel_index(channel_id, &channel_idx)) {
		prof_err("prof_get_channel_index fail. channel_id: %u\n", channel_id);
		return;
	}

	channel_info = (struct prof_channel_info *)&g_prof_dev_info.prof_channel[channel_idx];
	mutex_lock(&channel_info->cmd_mutex);
	prof_debug("report cmd_verify: %d, channel_id: %d, channel_cmd: %d, ret_val: %d.\n"
			, cq_scheduler->cmd_verify
			, cq_scheduler->channel_id
			, cq_scheduler->channel_cmd
			, cq_scheduler->ret_val);

	if ((cq_scheduler->channel_cmd == TS_SYNC_READ_PTR) ||
		(cq_scheduler->channel_cmd == TS_SYNC_WRITE_PTR)) {
		mutex_unlock(&channel_info->cmd_mutex);
		return;
		}

	if (channel_info->cmd_verify != cq_scheduler->cmd_verify) {
		prof_err("cmd_verify is fail. channel cmd_verify: %u, cq cmd_verify: %u.\n",
			 channel_info->cmd_verify, cq_scheduler->cmd_verify);
		mutex_unlock(&channel_info->cmd_mutex);
		return;
	}

	channel_info->ret_val = cq_scheduler->ret_val;
	mutex_unlock(&channel_info->cmd_mutex);

	up(&channel_info->sync_wait_sema);

	return;
}

void prof_cq_callback_profile(const unsigned char *cq_buf, const unsigned char *sq_buf)
{
	int ret;
	unsigned int channel_id;
	unsigned int poll_head  = 0;
	struct prof_cq_scheduler *cq_scheduler = NULL;
	struct prof_channel_info *channel_info = NULL;
	unsigned int channel_idx = PROF_CHANNEL_NUM;
	struct prof_device_info *prof_device  = &g_prof_dev_info.prof_device;

	if (cq_buf == NULL) {
		prof_err("cq_buf is null.\n");
		return;
	}

	cq_scheduler = (struct prof_cq_scheduler *)cq_buf;
	channel_id = cq_scheduler->channel_id;

	if (channel_id >= PROF_CHANNEL_MAX) {
		prof_err("channel_id is fail. channel_id: %u\n", channel_id);
		return;
	}

	if (prof_get_channel_index(channel_id, &channel_idx)) {
		prof_err("prof_get_channel_index fail. channel_id: %u\n", channel_id);
		return;
	}

	channel_info = (struct prof_channel_info *)&g_prof_dev_info.prof_channel[channel_idx];

	mutex_lock(&channel_info->cmd_mutex);
	prof_info("callback cmd_verify: %d, channel_id: %d, channel_cmd: %d, ret_val: %d.\n"
			, cq_scheduler->cmd_verify
			, cq_scheduler->channel_id
			, cq_scheduler->channel_cmd
			, cq_scheduler->ret_val);

	if (channel_info->real_time == PROF_REAL) {
		if (channel_info->poll_flag == POLL_INVALID) {
			mutex_lock(&prof_device->cq1_mutex);
			poll_head = prof_device->poll_head;
			g_prof_dev_info.poll_box[poll_head].device_id =
				prof_device->device_id;
			g_prof_dev_info.poll_box[poll_head].channel_id = channel_id;
			prof_device->poll_head++;
			prof_device->poll_head %= PROF_POLL_DEPTH;
			channel_info->poll_flag = POLL_VALID;
			mutex_unlock(&prof_device->cq1_mutex);
			wake_up(&prof_device->cq1_wq);
		}
	}
	else {
		ret = prof_ts_save_file(channel_info);
		if (ret != PROF_OK) {
			prof_err("prof_agent_ts save file error, ret = %d\n", ret);
		}
	}
	mutex_unlock(&channel_info->cmd_mutex);

	return;
}

void prof_cq_callback_aicpu(const unsigned char *cq_buf, const unsigned char *sq_buf)
{
	prof_info("prof_cq_callback_aicpu() is reserved!\n");
	return;
}

void prof_all_dfx_channel_free(void)
{
	int i;
	struct prof_channel_info *channel_info = NULL;

	for (i = 0; i < PROF_CHANNEL_NUM; i++) {
		channel_info = &g_prof_dev_info.prof_channel[i];

		mutex_lock(&channel_info->cmd_mutex);
		if (channel_info->vir_addr != NULL) {
			iounmap(channel_info->vir_addr);
			channel_info->vir_addr = NULL;
			channel_info->phy_addr = 0;
		}
		mutex_unlock(&channel_info->cmd_mutex);
	}

}

int prof_poll_init(void)
{
	int channel_idx;
	if (g_prof_dev_info.poll_box != NULL) {
		return PROF_OK;
	}

	g_prof_dev_info.poll_box = (prof_poll_info_t *)kzalloc(
		sizeof(prof_poll_info_t) * PROF_POLL_DEPTH, GFP_KERNEL);
	if (g_prof_dev_info.poll_box == NULL) {
		prof_err("pollbox kzalloc is error\n");
		return PROF_ERROR;
	}

	for (channel_idx = 0; channel_idx < PROF_CHANNEL_NUM; channel_idx ++) {
		mutex_init(&g_prof_dev_info.prof_channel[channel_idx].cmd_mutex);
	}

	return PROF_OK;
}

void prof_poll_free(void)
{

	if (g_prof_dev_info.poll_box != NULL) {
		kfree(g_prof_dev_info.poll_box);
		g_prof_dev_info.poll_box = NULL;
	}

	return;
}

int prof_dfx_cqsq_init(unsigned int dev_id)
{
	struct prof_device_info *prof_device  = &g_prof_dev_info.prof_device;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;
	struct devdrv_dfx_create_sq_para sq_para = {0};
	struct devdrv_dfx_create_cq_para cq_para = {0};
	int ret;
	unsigned int sq_0_index = DFX_PROF_SQ;
	unsigned int cq_0_index = DFX_PROF_REPORT_CQ;
	unsigned int cq_1_index = DFX_PROF_AICORE_CALLBACK_CQ;
	unsigned int cq_2_index = DFX_PROF_AICPU_CALLBACK_CQ;
	u64 sq_0_addr = 0;
	u64 cq_0_addr = 0;
	u64 cq_1_addr = 0;
	u64 cq_2_addr = 0;

	if (dev_id >= PROF_DEVICE_NUM) {
		prof_err("dev_id = %u fail.\n", dev_id);
		return PROF_ERROR;
	}

	if (prof_device->device_state == DEV_USED)
		return PROF_OK;

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if(cur_dev_ctx == NULL){
		devdrv_drv_err("cur_dev_ctx %d is null\n",dev_id);
		return -1;
	}

	sq_para.slot_len = PROF_SQ_SLOT_LEN;
	sq_para.sq_index = sq_0_index;
	sq_para.addr = (unsigned long long *)&sq_0_addr;
	sq_para.function = DEVDRV_MAX_CQSQ_FUNC;

	ret = devdrv_create_dfx_sq(cur_dev_ctx, &sq_para);
	if (ret != PROF_OK) {
		prof_err("devdrv_create_functinal_sq fail.ret = %d.\n", ret);
		return ret;
	}

	/*cq0's length is 128 Byte */
	cq_para.cq_type = DFX_DETAILED_CQ;
	cq_para.cq_index = cq_0_index;
	cq_para.function = DEVDRV_MAX_CQSQ_FUNC;
	cq_para.slot_len = PROF_SQ_SLOT_LEN;
	cq_para.callback = prof_sq_report_profile;
	cq_para.addr =(unsigned long long *)&cq_0_addr;

	ret = devdrv_create_dfx_cq(cur_dev_ctx, &cq_para);
	if (ret != PROF_OK) {
		prof_err("devdrv_create_functinal_cq fail.ret = %d.\n", ret);
		devdrv_destroy_dfx_sq(cur_dev_ctx, sq_0_index);
		return ret;
	}

	cq_para.cq_type = DFX_BRIEF_CQ;
	cq_para.cq_index = cq_1_index;
	cq_para.function = DEVDRV_MAX_CQSQ_FUNC;
	cq_para.slot_len = PROF_CQ_SLOT_LEN;
	cq_para.callback = prof_cq_callback_profile;
	cq_para.addr =(unsigned long long *)&cq_1_addr;

	ret = devdrv_create_dfx_cq(cur_dev_ctx, &cq_para);
	if (ret != PROF_OK) {
		prof_err("devdrv_create_functinal_cq fail.ret = %d.\n", ret);
		devdrv_destroy_dfx_sq(cur_dev_ctx, sq_0_index);
		devdrv_destroy_dfx_cq(cur_dev_ctx, cq_0_index);
		return ret;
	}

	cq_para.cq_type = DFX_BRIEF_CQ;
	cq_para.cq_index = cq_2_index;
	cq_para.function = DEVDRV_MAX_CQSQ_FUNC;
	cq_para.slot_len = PROF_CQ_SLOT_LEN;
	cq_para.callback = prof_cq_callback_aicpu;
	cq_para.addr =(unsigned long long *)&cq_2_addr;

	ret = devdrv_create_dfx_cq(cur_dev_ctx, &cq_para);
	if (ret != PROF_OK) {
		prof_err("devdrv_create_functinal_cq fail.ret = %d.\n", ret);
		devdrv_destroy_dfx_sq(cur_dev_ctx, sq_0_index);
		devdrv_destroy_dfx_cq(cur_dev_ctx, cq_0_index);
		devdrv_destroy_dfx_cq(cur_dev_ctx, cq_1_index);
		return ret;
	}

	prof_device->device_state = DEV_USED;
	prof_device->sq_0_index = sq_0_index;
	prof_device->cq_0_index = cq_0_index;
	prof_device->cq_1_index = cq_1_index;
	prof_device->cq_2_index = cq_2_index;
	prof_device->sq_0_addr  = sq_0_addr;
	prof_device->cq_0_addr  = cq_0_addr;
	prof_device->cq_1_addr  = cq_1_addr;
	prof_device->cq_2_addr  = cq_2_addr;

	prof_info("prof_cqsq_init init ok.\n");
	devdrv_drv_debug("prof_cqsq_init init ok.\n");//for test

	return ret;
}

int prof_ts_dfx_cqsq_create(void)
{
	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_mailbox_cqsq mailbox_cqsq = {0};
	int ret = 0;
	struct prof_device_info *prof_device  = &g_prof_dev_info.prof_device;

	if (prof_device->device_state != DEV_USED) {
		//devdrv_drv_err("device_state %d is error\n",prof_device->device_state);
		//return PROF_ERROR;
		prof_dfx_cqsq_init(prof_device->device_id);
	}

	dev_ctx  = get_dev_ctx_by_id(prof_device->device_id);
	if(dev_ctx == NULL){
		devdrv_drv_err("cur_dev_ctx %d is null\n",prof_device->device_id);
		return PROF_ERROR;
	};

	//ts_work_status 0: power down
	if (dev_ctx->ts_work_status == 0) {
	//if (devdrv_wait_tscpu_ready_status() != 0) {
		devdrv_drv_err("device is not working.\n");
		return PROF_ERROR;
	}

	/*only use cq2 for profile; log and debug don't use cq2 for driver and TS*/
	//memset(&mailbox_cqsq, 0, sizeof(struct devdrv_mailbox_cqsq));
	mailbox_cqsq.cmd_type  = DEVDRV_MAILBOX_CREATE_CQSQ_PROFILE;
	mailbox_cqsq.sq_index  = prof_device->sq_0_index;
	mailbox_cqsq.sq_addr   = prof_device->sq_0_addr;
	mailbox_cqsq.cq0_index = prof_device->cq_0_index;
	mailbox_cqsq.cq0_addr  = prof_device->cq_0_addr;
	mailbox_cqsq.cq1_index = prof_device->cq_1_index;
	mailbox_cqsq.cq1_addr  = prof_device->cq_1_addr;
	mailbox_cqsq.cq2_index = prof_device->cq_2_index;
	mailbox_cqsq.cq2_addr  = prof_device->cq_2_addr;

	ret = devdrv_dfx_send_mailbox(dev_ctx, &mailbox_cqsq);
	if (ret != PROF_OK) {
		prof_err("devdrv_mailbox_send_cqsq fail, ret=%d\n", ret);
		devdrv_destroy_dfx_sq(dev_ctx, prof_device->sq_0_index);
		devdrv_destroy_dfx_cq(dev_ctx, prof_device->cq_0_index);
		devdrv_destroy_dfx_cq(dev_ctx, prof_device->cq_1_index);
		devdrv_destroy_dfx_cq(dev_ctx, prof_device->cq_2_index);
		prof_device->device_state = DEV_UNUSED;
	}
	return ret;
}

int prof_ts_dfx_cqsq_release(void)
{
	struct prof_device_info *prof_device  = &g_prof_dev_info.prof_device;
	struct devdrv_mailbox_cqsq mailbox_cqsq = {0};
	struct devdrv_dev_ctx *dev_ctx = NULL;
	int ret;

	if (prof_device->device_state == DEV_UNUSED)
		return PROF_OK;

	dev_ctx  = get_dev_ctx_by_id(prof_device->device_id);
	if(dev_ctx == NULL){
		devdrv_drv_err("cur_dev_ctx %d is null\n",prof_device->device_id);
		return PROF_ERROR;
	};

	//ts_work_status 0: power down
	if (dev_ctx->ts_work_status == 0) {
		devdrv_drv_err("device is not working.\n");
		return PROF_ERROR;
	}

	//memset(&mailbox_cqsq, 0, sizeof(struct devdrv_mailbox_cqsq));

	mailbox_cqsq.cmd_type  = DEVDRV_MAILBOX_RELEASE_CQSQ_PROFILE;
	mailbox_cqsq.sq_index  = (u16)prof_device->sq_0_index;
	mailbox_cqsq.cq0_index = (u16)prof_device->cq_0_index;
	mailbox_cqsq.cq1_index = (u16)prof_device->cq_1_index;
	mailbox_cqsq.cq2_index = (u16)prof_device->cq_2_index;

	ret = devdrv_dfx_send_mailbox(dev_ctx, &mailbox_cqsq);
	if (ret != PROF_OK) {
		prof_err("devdrv_mailbox_send_cqsq fail, ret=%d\n", ret);
	}
	return ret;
}

int prof_dfx_cqsq_destroy(unsigned int dev_id)
{
	struct prof_device_info *prof_device  = &g_prof_dev_info.prof_device;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	if (prof_device->device_state == DEV_UNUSED)
		return PROF_OK;

	prof_device->device_state = DEV_UNUSED;

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if(cur_dev_ctx == NULL){
		devdrv_drv_err("cur_dev_ctx %d is null\n",dev_id);
		return PROF_ERROR;
	}

	devdrv_destroy_dfx_sq(cur_dev_ctx, prof_device->sq_0_index);
	devdrv_destroy_dfx_cq(cur_dev_ctx, prof_device->cq_0_index);
	devdrv_destroy_dfx_cq(cur_dev_ctx, prof_device->cq_1_index);
	devdrv_destroy_dfx_cq(cur_dev_ctx, prof_device->cq_2_index);

	return PROF_OK;
}

int prof_get_platform_info(struct prof_ioctl_para *para)
{
	prof_err("user out_buf is null, please check.\n");
	return PROF_ERROR;
}

int prof_get_devnum(struct prof_ioctl_para *para)
{
	prof_err("user out_buf is null, please check.\n");
	return PROF_ERROR;
}

int prof_get_devids(struct prof_ioctl_para *para)
{
	prof_err("user out_buf is null, please check.\n");
	return PROF_ERROR;
}

/* 0-input para valid; !0-input para invalid */
static int prof_check_start_para_valid(struct prof_ioctl_para *para)
{
	if (para->channel_id == CHANNEL_AICORE) {
		if ((para->buf_len <= DATA_BUF_RESERVED) || (para->buf_len > PROFILING_AICORE_SIZE)) {
			return PROF_ERROR;
		} else {
			return PROF_OK;
		}
	} else if (para->channel_id == CHANNEL_AICPU0) {
		if ((para->buf_len <= DATA_BUF_RESERVED) || (para->buf_len > PROFILING_AICPU_SIZE)) {
			return PROF_ERROR;
		} else {
			return PROF_OK;
		}
	} else if (para->channel_id == CHANNEL_TSFW) {
		if ((para->buf_len <= DATA_BUF_RESERVED) || (para->buf_len > PROFILING_TSFW_SIZE)) {
			return PROF_ERROR;
		} else {
			return PROF_OK;
		}
	} else {
		return PROF_ERROR;
	}
}

int prof_tscpu_sync_rw_ptr(int channel_id, int read1_write0)
{
	struct prof_device_info *prof_device  = &g_prof_dev_info.prof_device;
	int ret = 0;
	struct prof_channel_info *channel_info = NULL;
	struct prof_sq_scheduler sq_send_cmd = {0};
	unsigned int channel_idx = PROF_CHANNEL_NUM;

	if (channel_id >= PROF_CHANNEL_MAX) {
		prof_err("channel id fail.channel_id = %u.\n", channel_id);
		return PROF_ERROR;
	}

	if (prof_get_channel_index(channel_id, &channel_idx)) {
		prof_err("prof_get_channel_index fail. channel_id: %u\n", channel_id);
		return PROF_ERROR;
	}

	/*global memory need memset() for single channel*/
	channel_info = (struct prof_channel_info *)&g_prof_dev_info.prof_channel[channel_idx];
	mutex_lock(&channel_info->cmd_mutex);
	if (channel_info->channel_state == CHANNEL_DISABLE) {
		mutex_unlock(&channel_info->cmd_mutex);
		prof_err("TS cpu channel(%u) is disable.\n", channel_info->channel_id);
		return PROF_ERROR;
	}

	sq_send_cmd.cmd_verify = 0;
	sq_send_cmd.channel_id = channel_id;
	sq_send_cmd.channel_cmd = read1_write0? TS_SYNC_READ_PTR: TS_SYNC_WRITE_PTR;
	sq_send_cmd.data_size = 0;

	/*only send actual data for sq_send_cmd*/
	ret = devdrv_dfx_send_sq(prof_device->device_id, prof_device->sq_0_index,
					(unsigned char *)&sq_send_cmd, sizeof(struct prof_sq_scheduler));
	if (ret != PROF_OK) {
		prof_err("devdrv_functional_send_sq error: %d, channel_id = %u, cmd_verify = %u.\n",
			 ret, channel_info->channel_id, channel_info->cmd_verify);
	}
	mutex_unlock(&channel_info->cmd_mutex);

	return ret;
}



static int prof_get_channel_share_memory(struct prof_ioctl_para *para,
	struct prof_channel_info *channel_info)
{
	struct devdrv_dfx_desc *dfx_desc = NULL;
	struct devdrv_platform_info *plat_info = NULL;
	unsigned int i = 0;

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_info.\n");
		return PROF_ERROR;
	}
	dfx_desc = &DEVDRV_PLAT_GET_DFX_DESC(plat_info, DEVDRV_DFX_DEV_PROFILE);
	if (para->channel_id == CHANNEL_AICPU0) {
		channel_info->phy_addr = dfx_desc->bufs->base;
	} else if (para->channel_id == CHANNEL_AICORE) {
		channel_info->phy_addr = dfx_desc->bufs->base + PROFILING_AICPU_SIZE;
	} else if (para->channel_id == CHANNEL_TSFW) {
		channel_info->phy_addr = dfx_desc->bufs->base + PROFILING_AICPU_SIZE + PROFILING_AICORE_SIZE;
	}else {
		channel_info->phy_addr = 0;
		prof_err("error para->channel_id = %d\n", para->channel_id);
		return PROF_ERROR;
	}

	channel_info->vir_addr = (unsigned char *)ioremap_nocache(
		channel_info->phy_addr, para->buf_len);
	if (channel_info->vir_addr == NULL) {
		devdrv_drv_err("ioremap_nocache failed.\n");
		return PROF_ERROR;
	}

	for (i = 0; i < para->buf_len; i+=4) {
		writel(0, &channel_info->vir_addr[i]);
	}

	devdrv_drv_debug("phy_addr = %pK, vir_addr = %pK,.buf_size : %u\n",
		(void *)(uintptr_t)channel_info->phy_addr, (void *)(uintptr_t)channel_info->vir_addr,
		para->buf_len);

	return PROF_OK;
}

bool prof_ts_dfx_cqsq_is_created(void)
{
	unsigned int channel_idx;

	for (channel_idx = 0; channel_idx < PROF_CHANNEL_NUM; channel_idx ++) {
		if (g_prof_dev_info.prof_channel[channel_idx].channel_state == CHANNEL_ENABLE)
			return true;
	}
	return false;
}

bool prof_ts_dfx_cqsq_need_release(void)
{
	unsigned int channel_idx;

	for (channel_idx = 0; channel_idx < PROF_CHANNEL_NUM; channel_idx ++) {
		if (g_prof_dev_info.prof_channel[channel_idx].channel_state == CHANNEL_ENABLE)
			return false;
	}
	return true;
}

int prof_tscpu_start(struct prof_ioctl_para *para)
{
	struct prof_device_info *prof_device  = &g_prof_dev_info.prof_device;
	int ret = 0;
	unsigned int sq_size;
	struct prof_data_head *data_head = NULL;
	struct prof_channel_info *channel_info = NULL;
	struct prof_sq_scheduler *sq_send_cmd = NULL;
	unsigned int channel_idx = PROF_CHANNEL_NUM;
	para->ret_val = PROF_ERROR;

	sq_size = PROF_SQ_SLOT_LEN - sizeof(struct prof_sq_scheduler);
	if (para->ts_data_size > sq_size) {
		prof_err("ts_data_size(%u) overstep(%u).\n", para->ts_data_size, sq_size);
		return PROF_ERROR;
	}

	if (prof_check_start_para_valid(para)) {
		prof_err("para is error. channel_id=%d, buf_len = %d\n",
			para->channel_id, para->buf_len);
		return PROF_ERROR;
	}

	if (prof_get_channel_index(para->channel_id, &channel_idx)) {
		prof_err("prof_get_channel_index fail. channel_id: %u\n", para->channel_id);
		return PROF_ERROR;
	}

	/*global memory need memset() for single channel*/
	if (channel_idx >= PROF_CHANNEL_NUM) {
		prof_err(" channel_idx: %u\n", channel_idx);
		return PROF_ERROR;
	}

	channel_info = (struct prof_channel_info *)&g_prof_dev_info.prof_channel[channel_idx];
	mutex_lock(&channel_info->cmd_mutex);
	channel_info->used = PROF_CHANNEL_UNUSED;
	if (channel_info->channel_state == CHANNEL_ENABLE) {
		mutex_unlock(&channel_info->cmd_mutex);
		prof_err("TS cpu channel(%u) is enable.\n", channel_info->channel_id);
		para->ret_val = PROF_OK;
		return PROF_OK;
	}

	if (channel_info->vir_addr == NULL) {
		ret = prof_get_channel_share_memory(para,channel_info);
		if (ret != PROF_OK) {
			mutex_unlock(&channel_info->cmd_mutex);
			prof_err("get shm_memory failed!\n");
			return PROF_ERROR;
		}
		if (channel_info->vir_addr == NULL) {
			mutex_unlock(&channel_info->cmd_mutex);
			prof_err("channel_info->vir_addr memory req fail.\n");
			return PROF_ERROR;
		}
	}

	channel_info->buf_len	 = para->buf_len;
	channel_info->real_time	 = para->real_time;
	channel_info->channel_id = para->channel_id;
	channel_info->device_id	 = para->device_id;

	/*init data head , buffer and length Ïà¶ÔÓ¦*/
	data_head = (struct prof_data_head *)channel_info->vir_addr;

    flush_cache(channel_info->vir_addr, CACHE_LINE_LEN);
	data_head->write_ptr = 0;
	data_head->read_ptr  = 0;
	data_head->buf_len = para->buf_len;

    flush_cache(channel_info->vir_addr, CACHE_LINE_LEN);

	sq_size = sizeof(struct prof_sq_scheduler) + para->ts_data_size;
	sq_send_cmd = (struct prof_sq_scheduler *)kzalloc(sq_size, GFP_KERNEL);
	if (sq_send_cmd == NULL) {
		prof_err("kzalloc error, sq_send_cmd is null.\n");
		iounmap(channel_info->vir_addr);
		channel_info->phy_addr = 0;
		channel_info->vir_addr = NULL;
		mutex_unlock(&channel_info->cmd_mutex);
		return PROF_ERROR;
	}

	sq_send_cmd->cmd_verify = channel_info->cmd_verify;
	sq_send_cmd->channel_id = para->channel_id;
	sq_send_cmd->channel_cmd = TS_START;
	sq_send_cmd->buf_len   = para->buf_len - DATA_BUF_RESERVED;
	sq_send_cmd->phy_addr  = channel_info->phy_addr;
	sq_send_cmd->data_size = para->ts_data_size;

	memcpy(sq_send_cmd->ts_cpu_data, para->ts_data, para->ts_data_size);

	sema_init(&channel_info->sync_wait_sema, 0);
	sema_init(&channel_info->sema_channel_stopping, 0);

	/*only send actual data for sq_send_cmd*/
	ret = devdrv_dfx_send_sq(prof_device->device_id, prof_device->sq_0_index,
					(unsigned char *)sq_send_cmd, sq_size);
	if (ret != PROF_OK) {
		prof_err("devdrv_functional_send_sq error: %d, channel_id = %u, cmd_verify = %u.\n",
			 ret, channel_info->channel_id, channel_info->cmd_verify);
		kfree(sq_send_cmd);
		sq_send_cmd = NULL;
		iounmap(channel_info->vir_addr);
		channel_info->phy_addr = 0;
		channel_info->vir_addr = NULL;
		para->ret_val = ret;
		mutex_unlock(&channel_info->cmd_mutex);

		return ret;
	}

	ret = down_timeout(&channel_info->sync_wait_sema, TS2DRV_TIMEOUT);
	if (ret != PROF_OK) {
		prof_err("prof_tscpu_start down_timeout. channel_id: %u, cmd_verify: %u\n",
			 para->channel_id, channel_info->cmd_verify);

		channel_info->cmd_verify++;
		para->ret_val = PROF_TIMEOUT;
		kfree(sq_send_cmd);
		sq_send_cmd = NULL;
		mutex_unlock(&channel_info->cmd_mutex);

		return PROF_TIMEOUT;
	}

	channel_info->cmd_verify++;
	channel_info->channel_state = CHANNEL_ENABLE;

	/*no real mode, only save file path; */
	if (channel_info->real_time == PROF_NON_REAL)
		memcpy(channel_info->prof_file, para->prof_file, strlen(para->prof_file) + 1);

	kfree(sq_send_cmd);
	sq_send_cmd = NULL;
	para->ret_val = PROF_OK;
	prof_err("prof_tscpu_start channel_id: %u success.\n",
		 para->channel_id);
	mutex_unlock(&channel_info->cmd_mutex);

	return PROF_OK;
}

int prof_tscpu_stop(struct prof_ioctl_para *para)
{
	struct prof_device_info *prof_device  = &g_prof_dev_info.prof_device;
	int ret = 0;
	struct prof_channel_info *channel_info = NULL;
	struct prof_sq_scheduler  sq_send_cmd = {0};
	struct prof_cq_scheduler  cq_callback_cmd = {0};
	unsigned int channel_idx = PROF_CHANNEL_NUM;
	para->ret_val = PROF_ERROR;

	if (para->channel_id >= PROF_CHANNEL_MAX) {
		prof_err("channel id fail. channel_id = %u.\n", para->channel_id);
		return PROF_ERROR;
	}

	if (prof_get_channel_index(para->channel_id, &channel_idx)) {
		prof_err("prof_get_channel_index fail. channel_id: %u\n", para->channel_id);
		return PROF_ERROR;
	}

	if (channel_idx >= PROF_CHANNEL_NUM) {
		prof_err(" channel_idx: %u\n", channel_idx);
		return PROF_ERROR;
	}

	channel_info = (struct prof_channel_info *)&g_prof_dev_info.prof_channel[channel_idx];
	mutex_lock(&channel_info->cmd_mutex);
	if (channel_info->channel_state != CHANNEL_ENABLE) {
		mutex_unlock(&channel_info->cmd_mutex);
		prof_err("TS cpu channel is not exist. device_id = %u, channel_id = %u.\n",
			 channel_info->device_id, para->channel_id);
		return PROF_ERROR;
	}

	sq_send_cmd.cmd_verify = channel_info->cmd_verify;
	sq_send_cmd.channel_id = channel_info->channel_id;
	sq_send_cmd.channel_cmd = TS_STOP;

	ret = devdrv_dfx_send_sq(prof_device->device_id, prof_device->sq_0_index,
					(unsigned char *)&sq_send_cmd,
					sizeof(struct prof_sq_scheduler));
	if (ret != PROF_OK) {
		mutex_unlock(&channel_info->cmd_mutex);
		prof_err("devdrv_functional_send_sq error: %d, channel_id = %u, cmd_verify = %u.\n",
			ret, channel_info->channel_id, channel_info->cmd_verify);
		para->ret_val = ret;

		return ret;
	}

	ret = down_timeout(&channel_info->sync_wait_sema, TS2DRV_TIMEOUT);
	if (ret != PROF_OK) {
		prof_err("prof_tscpu_stop down_timeout. channel_id: %u, cmd_verify: %u\n",
			 para->channel_id, channel_info->cmd_verify);
		channel_info->cmd_verify++;
		channel_info->channel_state = CHANNEL_DISABLE;
		para->ret_val = PROF_TIMEOUT;
		mutex_unlock(&channel_info->cmd_mutex);

		return PROF_TIMEOUT;
	}

	/*callback for the last data*/
	channel_info->used = PROF_CHANNEL_STOPPING;
	cq_callback_cmd.channel_id = para->channel_id;
	prof_cq_callback_profile((unsigned char *)&cq_callback_cmd, NULL);

	channel_info->channel_state = CHANNEL_DISABLE;
	channel_info->used = PROF_CHANNEL_UNUSED;

	if (prof_ts_dfx_cqsq_need_release() == true)
	{
		(void)prof_ts_dfx_cqsq_release();
	}

	iounmap(channel_info->vir_addr);

	channel_info->phy_addr = 0;
	channel_info->vir_addr = NULL;
	channel_info->cmd_verify++;
	mutex_unlock(&channel_info->cmd_mutex);

	para->ret_val = PROF_OK;
	return PROF_OK;
}

void prof_tscpu_all_stop(unsigned int stop_type)
{
	int ret;
	int i;
	struct prof_channel_info *channel_info = NULL;
	struct prof_sq_scheduler  sq_send_cmd = {0};
	struct prof_device_info *prof_device  = &g_prof_dev_info.prof_device;

	mutex_lock(&prof_device->cmd_mutex);

	for (i = 0; i < PROF_CHANNEL_NUM; i++) {
		channel_info = (struct prof_channel_info *)&g_prof_dev_info.prof_channel[i];

		if (channel_info->channel_state != CHANNEL_ENABLE)
			continue;
		mutex_lock(&channel_info->cmd_mutex);

		channel_info->channel_state = CHANNEL_DISABLE;

		sq_send_cmd.cmd_verify = channel_info->cmd_verify;
		sq_send_cmd.channel_id = channel_info->channel_id;
		sq_send_cmd.channel_cmd = TS_STOP;

		ret = devdrv_dfx_send_sq(prof_device->device_id, prof_device->sq_0_index,
						(unsigned char *)&sq_send_cmd,
						sizeof(struct prof_sq_scheduler));
		if (ret != PROF_OK) {
			prof_err("sq_send fail error: %d, device_id = %u, channel_id = %u\n",
				ret, channel_info->device_id, channel_info->channel_id);
		}

		if (channel_info->vir_addr != NULL) {
			iounmap(channel_info->vir_addr);
			channel_info->phy_addr = 0;
			channel_info->vir_addr = NULL;
		}
		channel_info->cmd_verify++;
		mutex_unlock(&channel_info->cmd_mutex);
	}

	mutex_unlock(&prof_device->cmd_mutex);

	if (stop_type == PROF_DEV_STOP) {
		prof_ts_dfx_cqsq_release();
	}

	return;
}

int prof_tscpu_ringbuffer(struct prof_ioctl_para *para,
	unsigned int read_ptr, unsigned int write_ptr, struct prof_channel_info *channel_info)
{
	unsigned int buf_size = 0;
	unsigned int all_size = 0;

	struct prof_data_head *data_head = NULL;
	void *out_buf = NULL;
	unsigned char *base = NULL;

	out_buf = para->out_buf;
	if (out_buf == NULL) {
		prof_err("para->out_buf is null.\n");
		return PROF_ERROR;
	}

	data_head = (struct prof_data_head *)channel_info->vir_addr;

	if (write_ptr > read_ptr) {
		buf_size = write_ptr - read_ptr;
		base = channel_info->vir_addr + DATA_BUF_RESERVED + read_ptr;
		buf_size = buf_size > para->buf_len ? para->buf_len : buf_size;

		if (copy_to_user_safe((void __user *)out_buf, (void *)base, buf_size) != 0)
			return PROF_ERROR;

		atomic_set((atomic_t *)&data_head->read_ptr, (read_ptr + buf_size));
		flush_cache(channel_info->vir_addr, CACHE_LINE_LEN);
		para->ret_val = buf_size;

		return PROF_OK;
	}

	if (read_ptr < (channel_info->buf_len - DATA_BUF_RESERVED)) {
		buf_size = channel_info->buf_len - DATA_BUF_RESERVED - read_ptr;
		base = channel_info->vir_addr + DATA_BUF_RESERVED + read_ptr;
		all_size = buf_size > para->buf_len ? para->buf_len : buf_size;

		if (copy_to_user_safe((void __user *)out_buf, (void *)base, all_size) != 0)
			return PROF_ERROR;

		if ((all_size >= para->buf_len) || (write_ptr == 0)) {
			atomic_set((atomic_t *)&data_head->read_ptr, (read_ptr + all_size));
			flush_cache(channel_info->vir_addr, CACHE_LINE_LEN);
			para->ret_val = all_size;

			return PROF_OK;
		}
	}

	buf_size = write_ptr;
	base = channel_info->vir_addr + DATA_BUF_RESERVED;
	buf_size = (buf_size + all_size) > para->buf_len ? para->buf_len - all_size : buf_size;

	/*copy_to_user_secure() is self defined */
	if (copy_to_user_safe((void *)(out_buf + all_size), (void *)base, buf_size) != 0)
		return PROF_ERROR;

	atomic_set((atomic_t *)&data_head->read_ptr, buf_size);
	flush_cache(channel_info->vir_addr, CACHE_LINE_LEN);
	para->ret_val = all_size + buf_size;

	return PROF_OK;
}

int prof_tscpu_read(struct prof_ioctl_para *para)
{
	int ret = 0;
	unsigned int write_ptr = 0;
	unsigned int read_ptr = 0;
	struct prof_channel_info *channel_info = NULL;
	struct prof_data_head *data_head = NULL;
	unsigned int channel_idx = PROF_CHANNEL_NUM;

	if (prof_get_channel_index(para->channel_id, &channel_idx)) {
		prof_err("prof_get_channel_index fail. channel_id: %u\n", para->channel_id);
		return PROF_ERROR;
	}

	if (channel_idx >= PROF_CHANNEL_NUM) {
		prof_err(" channel_idx: %u\n", channel_idx);
		return PROF_ERROR;
	}

	channel_info = (struct prof_channel_info *)&g_prof_dev_info.prof_channel[channel_idx];
	mutex_lock(&channel_info->cmd_mutex);
	if (channel_info->channel_state != CHANNEL_ENABLE ||
		channel_info->vir_addr == NULL) {
		prof_err("TS cpu channel is not exist. device_id = %u, channel_id = %u.\n",
			 channel_info->device_id, para->channel_id);
		mutex_unlock(&channel_info->cmd_mutex);
		return PROF_ERROR;
	}

	/*check real mode */
	if (channel_info->real_time != PROF_REAL) {
		prof_err("the real mode error, please check. real_time = %u, channel_id = %u.\n",
			 channel_info->real_time, para->channel_id);
		mutex_unlock(&channel_info->cmd_mutex);
		return PROF_ERROR;
	}

	data_head = (struct prof_data_head *)channel_info->vir_addr;

	flush_cache(channel_info->vir_addr, channel_info->buf_len);

	read_ptr  = data_head->read_ptr;
	write_ptr = atomic_read((atomic_t *)&data_head->write_ptr);

	para->ret_val = 0;

	/*at top-level, read interface is thread; only no data, should free channel's memory.*/
	if (write_ptr == read_ptr) {
		if (channel_info->used == PROF_CHANNEL_STOPPING) {
			up(&channel_info->sema_channel_stopping);
        }
		mutex_unlock(&channel_info->cmd_mutex);
		return PROF_OK;
	}

	ret = prof_tscpu_ringbuffer(para, read_ptr, write_ptr, channel_info);
	if (ret != PROF_OK) {
		mutex_unlock(&channel_info->cmd_mutex);
		prof_err("channel_id = %d, read_ptr = %d, write_ptr = %d.\n", para->channel_id, read_ptr, write_ptr);
		return PROF_ERROR;
	}
	mutex_unlock(&channel_info->cmd_mutex);

	return PROF_OK;
}

int prof_drv_read(struct prof_ioctl_para *prof_para)
{
	return prof_tscpu_read(prof_para);
}

int prof_drv_stop(struct prof_ioctl_para *prof_para)
{
	return prof_tscpu_stop(prof_para);
}

void prof_pollflag_init(int start, int end)
{
	int index;
	unsigned int channel_idx = PROF_CHANNEL_NUM;

	for (index = start; index < end; index++) {
		if (prof_get_channel_index(g_prof_dev_info.poll_box[index].channel_id,
			&channel_idx)) {
			prof_err("prof_get_channel_index fail. channel_id: %u\n",
				g_prof_dev_info.poll_box[index].channel_id);
			continue;
		}

		if (channel_idx >= PROF_CHANNEL_NUM) {
			prof_err("channel_idx: %u\n", channel_idx);
			continue;
		}
		g_prof_dev_info.prof_channel[channel_idx].poll_flag = POLL_INVALID;
	}

	return;
}

int prof_poll_copy(struct prof_ioctl_para *para)
{
	int ret;
	int size = 0;
	int all_size = 0;
	unsigned int copy_size = 0;
	struct prof_device_info *prof_device  = &g_prof_dev_info.prof_device;

	if (para->out_buf == NULL) {
		prof_err("user out_buf is null, please check.\n");
		return PROF_ERROR;
	}

	mutex_lock(&prof_device->cq1_mutex);

	if (prof_device->poll_head == prof_device->poll_tail) {
		para->ret_val = 0;
		mutex_unlock(&prof_device->cq1_mutex);
		return PROF_OK;
	}

	if (prof_device->poll_head > prof_device->poll_tail) {
		size = prof_device->poll_head - prof_device->poll_tail;
		copy_size = sizeof(prof_poll_info_t) * size;
		if(copy_size > sizeof(prof_poll_info_t) * para->poll_number)
		{
			copy_size = sizeof(prof_poll_info_t) * para->poll_number;
			size = para->poll_number;
		}

		ret = copy_to_user_safe((void __user *)para->out_buf,
				   (void *)(&g_prof_dev_info.poll_box[prof_device->poll_tail]),
				   copy_size);
		if (ret != PROF_OK) {
			prof_err("copy_to_user error.size = %d\n", size);
			para->ret_val = 0;
			mutex_unlock(&prof_device->cq1_mutex);

			return PROF_ERROR;
		}

		prof_pollflag_init(prof_device->poll_tail, prof_device->poll_tail + size);
		prof_device->poll_tail = prof_device->poll_tail + size;
		para->ret_val= size;
		mutex_unlock(&prof_device->cq1_mutex);

		return PROF_OK;
	}

	all_size = 0;
	if (prof_device->poll_tail < PROF_POLL_DEPTH) {
		size = PROF_POLL_DEPTH - prof_device->poll_tail;

		if (size > para->poll_number) {
			size = para->poll_number;
		}

		ret = copy_to_user_safe((void __user *)para->out_buf,
				   (void *)(&g_prof_dev_info.poll_box[prof_device->poll_tail]),
				   (unsigned long)(unsigned int)(sizeof(prof_poll_info_t) * size));
		if (ret != PROF_OK) {
			prof_err("copy_to_user error.size = %d\n", size);
			para->ret_val = 0;
			mutex_unlock(&prof_device->cq1_mutex);

			return PROF_ERROR;
		}

		prof_pollflag_init(prof_device->poll_tail, prof_device->poll_tail + size);

		if ((size >= para->poll_number) || (prof_device->poll_head == 0)) {
			prof_device->poll_tail = (prof_device->poll_tail + size) % PROF_POLL_DEPTH;
			para->ret_val = size;
			mutex_unlock(&prof_device->cq1_mutex);

			return PROF_OK;
		}

		all_size = size;
	}

	size = prof_device->poll_head;

	//enhance condition for static check
	if ((size + all_size) > para->poll_number) {
		size = para->poll_number - all_size;
	}

	/*notice user buffer's offset*/
	ret  = copy_to_user_safe((void *)(para->out_buf + (unsigned int)(
			sizeof(prof_poll_info_t) * all_size)), (void *)g_prof_dev_info.poll_box,
			(unsigned long)(unsigned int)(sizeof(prof_poll_info_t) * size));
	if (ret != PROF_OK) {
		prof_err("copy_to_user error.size = %d, already size = %d, "
			"poll_number = %d, poll_head = %d, poll_tail = %d\n",
			size, all_size, para->poll_number, prof_device->poll_head,
			prof_device->poll_tail);
		para->ret_val = all_size;
		mutex_unlock(&prof_device->cq1_mutex);

		return PROF_ERROR;
	}

	prof_pollflag_init(0, size);

	prof_device->poll_tail = size;
	para->ret_val = all_size + size;
	mutex_unlock(&prof_device->cq1_mutex);

	return PROF_OK;
}

int prof_poll(struct prof_ioctl_para *para)
{
	int ret = PROF_ERROR;
	long time_remain = (long)para->timeout * TIME_UNIT;
	struct prof_device_info *prof_device  = &g_prof_dev_info.prof_device;

	if ((para->poll_number > PROF_CHANNEL_MAX) || (para->poll_number <= 0)) {
		prof_err("log_para->number is wrong, number = %d\n", para->poll_number);
		return PROF_ERROR;
	}

	if (time_remain > 0) {
		(void)wait_event_timeout(prof_device->cq1_wq,
			(prof_device->poll_head != prof_device->poll_tail), time_remain);
	}

	ret = prof_poll_copy(para);
	if (ret != PROF_OK) {
		prof_err("prof_poll_copy error, ret = %d\n", ret);
		return ret;
	}

	return ret;
}

int prof_dfx_resource_recycle(unsigned int dev_id)
{
	prof_dfx_cqsq_destroy(dev_id);

	return PROF_OK;
}

long prof_drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct prof_ioctl_para prof_para = {0};
	void __user *parg = NULL;
	int ret;
	unsigned int drv_cmd;

	parg = (void __user *)(uintptr_t)arg;
	if (parg == NULL) {
		prof_err("user arg error.parg is null\n");
		return PROF_ERROR;
	}

	if (copy_from_user(&prof_para, parg, sizeof(struct prof_ioctl_para))) {
		prof_err("copy_from_user error.\n");
		return PROF_ERROR;
	}

	prof_para.ret_val = PROF_ERROR;

	drv_cmd = _IOC_NR(cmd);

	switch (drv_cmd) {
		case PROF_READ:
			ret = prof_drv_read(&prof_para);
			break;
		case PROF_POLL:
			ret = prof_poll(&prof_para);
			break;
		default:
			prof_err("log ioctl cmd(%d) illegal.\n", drv_cmd);
			return PROF_ERROR;
	}

	if (ret != PROF_OK) {
		prof_err("drv_cmd error. drv_cmd = %d\n", drv_cmd);
		return PROF_ERROR;
	}

	if (copy_to_user_safe(parg, (void *)&prof_para,
			 sizeof(struct prof_ioctl_para))) {
		prof_err("copy_to_user error.\n");
		return PROF_ERROR;
	}

	return PROF_OK;
}

int prof_drv_open(struct inode *inode, struct file *filp)
{
	return PROF_OK;
}

int prof_drv_release(struct inode *inode, struct file *filp)
{
	return PROF_OK;
}

const struct file_operations prof_drv_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl = prof_drv_ioctl,
	.open		= prof_drv_open,
	.release	= prof_drv_release,
};

int prof_drv_register_cdev(void)
{
	int ret;
	unsigned int major;
	dev_t devno;
	struct char_device *priv = (struct char_device *)&g_prof_dev_info.char_dev;

	priv->devno = 0;
	ret = alloc_chrdev_region(&priv->devno, 0, 1, CHAR_DRIVER_NAME);
	if (ret < 0) {
		prof_err("alloc_chrdev_region error, ret = %d\n", ret);
		return PROF_ERROR;
	}

	/* init and add char device */
	major = MAJOR(priv->devno);
	devno = MKDEV(major, 0);
	cdev_init(&priv->cdev, &prof_drv_fops);
	priv->cdev.owner = THIS_MODULE;
	priv->cdev.ops = &prof_drv_fops;

	if (cdev_add(&priv->cdev, devno, 1)) {
		prof_err("cdev_add error, devno = %d\n", devno);
		unregister_chrdev_region(devno, 1);
		return PROF_ERROR;
	}

	priv->dev_class = class_create(THIS_MODULE, CHAR_DRIVER_NAME);
	if (IS_ERR(priv->dev_class)) {
		prof_err("class_create error, devno = %d\n", devno);
		unregister_chrdev_region(devno, 1);
		cdev_del(&priv->cdev);
		return PROF_ERROR;
	}
	if(priv->dev_class != NULL)
		priv->dev_class->devnode = prof_devnode;

	g_prof_dev_info.char_dev.device = device_create(priv->dev_class, NULL,
			devno, NULL, CHAR_DRIVER_NAME);

	return PROF_OK;
}

void prof_drv_free_cdev(void)
{
	struct char_device *priv = &g_prof_dev_info.char_dev;

	device_destroy(priv->dev_class, priv->devno);
	class_destroy(priv->dev_class);
	unregister_chrdev_region(priv->devno, 1);
	cdev_del(&priv->cdev);
}

int __init prof_drv_module_init(void)
{
	int ret;
	struct prof_device_info *prof_device  = &g_prof_dev_info.prof_device;

	memset(&g_prof_dev_info, 0, sizeof(prof_char_dev_global_info_t));

	//memset(&prof_device, 0, sizeof(struct prof_device_info));

	mutex_init(&prof_device->ts_file_mutex);
	mutex_init(&prof_device->ext_file_mutex);
	mutex_init(&prof_device->cmd_mutex);
	mutex_init(&prof_device->cq1_mutex);
	init_waitqueue_head(&prof_device->cq1_wq);

	prof_device->device_id = 0;
	prof_device->device_state = DEV_UNUSED;
	g_prof_dev_info.op_cb_idx = DEVDRV_CALLBACK_REG_NUM;
	g_prof_dev_info.rl_cb_idx = DEVDRV_CALLBACK_REG_NUM;
	g_prof_dev_info.poll_box = NULL;

	ret = prof_drv_register_cdev();
	if (ret != PROF_OK) {
		prof_err("create character device fail.\n");
		return PROF_ERROR;
	}

	ret = prof_poll_init();
	if (ret != PROF_OK) {
		prof_drv_free_cdev();
		prof_err("prof_poll_init fail.\n");
		return PROF_ERROR;
	}

	ret = prof_dfx_cqsq_init(prof_device->device_id);
	if (ret != PROF_OK) {
		prof_drv_free_cdev();
		prof_poll_free();
		prof_err("call prof_cqsq_init fail.\n");
		return PROF_ERROR;
	}

	prof_info("prof_drv load ok.\n");

	return PROF_OK;
}

void __exit prof_drv_module_exit(void)
{
	prof_dfx_cqsq_destroy(g_prof_dev_info.prof_device.device_id);
	prof_poll_free();
	prof_all_dfx_channel_free();
	prof_drv_free_cdev();

	prof_info("prof_drv unload ok.\n");
}

module_init(prof_drv_module_init);

module_exit(prof_drv_module_exit);

MODULE_DESCRIPTION("prof driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_VERSION(DRV_MODE_VERSION);
