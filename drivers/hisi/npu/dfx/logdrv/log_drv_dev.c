#include <linux/uaccess.h>
#include <linux/list.h>
#include "log_drv_dev.h"
#include "drv_log.h"
#include "npu_dfx.h"
#include "npu_dfx_cq.h"
#include "npu_dfx_sq.h"
#include "npu_common.h"
#include "npu_manager.h"
#include "npu_platform.h"
#include "npu_ioctl_services.h"
#include "npu_manager_common.h"
#include "npu_mailbox_msg.h"

#define MODULE_LOG			"NPU_DRV"
#define slog_drv_err(fmt ...)  	drv_err(MODULE_LOG, fmt)
#define slog_drv_warn(fmt ...)	drv_warn(MODULE_LOG, fmt)
#define slog_drv_info(fmt ...) 	drv_info(MODULE_LOG, fmt)
#define slog_drv_debug(fmt ...) drv_debug(MODULE_LOG, fmt)

#ifndef STATIC
#define STATIC static
#endif

log_char_dev_global_info_t g_log_dev_info;

extern int devdrv_get_devids(unsigned int *devices);

int flag_test = 0;

int log_get_channel_index(int channel_id, unsigned int *channel_idx)
{
	int ret = 0;
	if (channel_id == LOG_CHANNEL_TS_ID) {
		*channel_idx  = LOG_CHANNEL_TS_IDX;
	} else if (channel_id == LOG_CHANNEL_AICPU_ID) {
		*channel_idx  = LOG_CHANNEL_AICPU_IDX;
	} else {
		ret = -1;
	}

	return ret;
}

void log_cq0_report(const unsigned char *cq_buf, const unsigned char *sq_buf)
{
	struct log_channel_info_t *channel_info = NULL;
	struct log_cq_scheduler_t *cq_report = NULL;
	int channel_id = 0;
	unsigned int channel_idx = LOG_CHANNEL_NUM;
	struct log_device_info_t *log_dev_info = NULL;

	if (cq_buf == NULL) {
		slog_drv_err("cq_buf is null.\n");
		return;
	}

	cq_report  = (struct log_cq_scheduler_t *)cq_buf;
	channel_id = cq_report->channel_id;

	if (channel_id >= LOG_CHANNEL_MAX) {
		slog_drv_err("channel_id is fail. channel_id: %u\n", channel_id);
		return;
	}

	if (log_get_channel_index(channel_id, &channel_idx)) {
		slog_drv_err("log_get_channel_index fail. channel_id: %u\n", channel_id);
		return;
	}

	slog_drv_debug("channel_id: %d cmd_verify: %u channel_cmd: %u, ret_val: %u.\n", channel_id,
		      cq_report->cmd_verify, cq_report->channel_cmd, cq_report->ret_val);

	log_dev_info = &(g_log_dev_info.log_dev_info);

	mutex_lock(&log_dev_info->cq0_mutex);
	if (log_dev_info->cmd_verify != (unsigned char)cq_report->cmd_verify) {
		slog_drv_err("cmd_verify is fail. device cmd_verify: %u, cq cmd_verify: %u.\n",
			     log_dev_info->cmd_verify, cq_report->cmd_verify);

		mutex_unlock(&log_dev_info->cq0_mutex);
		return;
	}

	channel_info = (struct log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];
	channel_info->ret_val = cq_report->ret_val;
	log_dev_info->cmd_verify++;
	mutex_unlock(&log_dev_info->cq0_mutex);

	up(&log_dev_info->cq0_wait_sema);
}

STATIC char* log_devnode(struct device *dev, umode_t *mode)
{
	if(mode != NULL)
		*mode = 0666;
	return NULL;
}

void log_cq1_callback(const unsigned char *cq_buf, const unsigned char *sq_buf)
{
	struct log_channel_info_t *channel_info = NULL;
	struct log_cq_scheduler_t *cq_report = NULL;
	int channel_id = 0;
	unsigned int poll_head  = 0;
	unsigned int channel_idx  = LOG_CHANNEL_NUM;
	struct log_device_info_t *log_dev_info = NULL;

	if (cq_buf == NULL) {
		slog_drv_err("cq_buf is null.\n");
		return;
	}

	cq_report  = (struct log_cq_scheduler_t *)cq_buf;
	channel_id = cq_report->channel_id;

	if (channel_id >= LOG_CHANNEL_MAX) {
		slog_drv_err("channel_id is fail. channel_id: %u\n", channel_id);
		return;
	}

	if (log_get_channel_index(channel_id, &channel_idx)) {
		slog_drv_err("log_get_channel_index fail. channel_id: %u\n", channel_id);
		return;
	}

	channel_info = (struct log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];

	if (channel_info->poll_flag == POLL_VALID)
		return;

	log_dev_info = &(g_log_dev_info.log_dev_info);
	mutex_lock(&log_dev_info->cq1_mutex);
	poll_head = log_dev_info->poll_head;
	g_log_dev_info.poll_box[poll_head].device_id  = log_dev_info->device_id;
	g_log_dev_info.poll_box[poll_head].channel_idx = channel_idx;
	log_dev_info->poll_head++;
	log_dev_info->poll_head %= LOG_POLL_DEPTH;
	channel_info->poll_flag = POLL_VALID;
	mutex_unlock(&log_dev_info->cq1_mutex);
	wake_up(&log_dev_info->cq1_wq);

	return;
}

int log_dfx_cqsq_init(unsigned int dev_id)
{
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;
	struct devdrv_dfx_create_sq_para sq_para;
	struct devdrv_dfx_create_cq_para cq_para;
	unsigned int sq_0_index = DFX_LOG_SQ;
	unsigned int cq_0_index = DFX_LOG_REPORT_CQ;
	unsigned int cq_1_index = DFX_LOG_CALLBACK_CQ;
	u64 sq_0_addr = 0;
	u64 cq_0_addr = 0;
	u64 cq_1_addr = 0;
	int ret = 0;
	struct log_device_info_t *log_dev_info = &(g_log_dev_info.log_dev_info);

	if (log_dev_info->device_state == DEV_USED)
		return IDE_DRV_OK;

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if(cur_dev_ctx == NULL){
		devdrv_drv_err("cur_dev_ctx %d is null\n",dev_id);
		return -1;
	}

	sq_para.slot_len = LOG_SQ_SLOT_LEN;
	sq_para.sq_index = sq_0_index;
	sq_para.addr = (unsigned long long *)&sq_0_addr;
	sq_para.function = DEVDRV_MAX_CQSQ_FUNC;

	ret = devdrv_create_dfx_sq(cur_dev_ctx, &sq_para);
	if (ret != IDE_DRV_OK) {
		slog_drv_err("devdrv_create_dfx_sq fail.ret = %d.\n", ret);
		return ret;
	}

	/*the first cq length is 128 Byte; and the other cqs are 32 Byte;*/
	cq_para.cq_type = DFX_DETAILED_CQ;
	cq_para.cq_index = cq_0_index;
	cq_para.function = DEVDRV_MAX_CQSQ_FUNC;
	cq_para.slot_len = LOG_SQ_SLOT_LEN;
	cq_para.callback = log_cq0_report;
	cq_para.addr =(unsigned long long *)&cq_0_addr;

	ret = devdrv_create_dfx_cq(cur_dev_ctx, &cq_para);
	if (ret != IDE_DRV_OK) {
		slog_drv_err("devdrv_create_dfx_cq fail.ret = %d.\n", ret);
		devdrv_destroy_dfx_sq(cur_dev_ctx, sq_0_index);
		return ret;
	}

	cq_para.cq_type = DFX_BRIEF_CQ;
	cq_para.cq_index = cq_1_index;
	cq_para.function = DEVDRV_MAX_CQSQ_FUNC;
	cq_para.slot_len = LOG_CQ_SLOT_LEN;
	cq_para.callback = log_cq1_callback;
	cq_para.addr =(unsigned long long *)&cq_1_addr;

	ret = devdrv_create_dfx_cq(cur_dev_ctx, &cq_para);
	if (ret != IDE_DRV_OK) {
		slog_drv_err("devdrv_create_dfx_cq fail.ret = %d.\n", ret);
		devdrv_destroy_dfx_sq(cur_dev_ctx, sq_0_index);
		devdrv_destroy_dfx_cq(cur_dev_ctx, cq_0_index);
		return ret;
	}

	log_dev_info->device_state = DEV_USED;
	log_dev_info->sq_0_index = sq_0_index;
	log_dev_info->cq_0_index = cq_0_index;
	log_dev_info->cq_1_index = cq_1_index;
	log_dev_info->sq_addr    = sq_0_addr;
	log_dev_info->cq0_addr   = cq_0_addr;
	log_dev_info->cq1_addr   = cq_1_addr;

	slog_drv_info("log_cqsq_init init ok.\n");

	return ret;
}

static int log_ts_dfx_cqsq_create(void)
{
	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_mailbox_cqsq mailbox_cqsq = {0};
	int ret = 0;
	struct log_device_info_t *log_dev_info = &(g_log_dev_info.log_dev_info);
	const unsigned int dev_id = 0;

	if (log_dev_info->device_state != DEV_USED) {
		log_dfx_cqsq_init(dev_id);
	}

	dev_ctx  = get_dev_ctx_by_id(log_dev_info->device_id);
	if(dev_ctx == NULL){
		devdrv_drv_err("cur_dev_ctx %d is null\n",log_dev_info->device_id);
		return IDE_DRV_ERROR;
	};

	if (dev_ctx->ts_work_status == 0) {
		devdrv_drv_err("device is not working.\n");
		return DEVDRV_TS_DOWN;
	}

	mailbox_cqsq.cmd_type  = DEVDRV_MAILBOX_CREATE_CQSQ_LOG;
	mailbox_cqsq.sq_index  = (unsigned short)log_dev_info->sq_0_index;
	mailbox_cqsq.sq_addr   = log_dev_info->sq_addr;
	mailbox_cqsq.cq0_index = (unsigned short)log_dev_info->cq_0_index;
	mailbox_cqsq.cq0_addr  = log_dev_info->cq0_addr;
	mailbox_cqsq.cq1_index = (unsigned short)log_dev_info->cq_1_index;
	mailbox_cqsq.cq1_addr  = log_dev_info->cq1_addr;

	ret = devdrv_dfx_send_mailbox(dev_ctx, &mailbox_cqsq);
	if (ret != IDE_DRV_OK) {
		slog_drv_err("devdrv_mailbox_send_cqsq fail.ret = %d.\n", ret);
		devdrv_destroy_dfx_sq(dev_ctx, log_dev_info->sq_0_index);
		devdrv_destroy_dfx_cq(dev_ctx, log_dev_info->cq_0_index);
		devdrv_destroy_dfx_cq(dev_ctx, log_dev_info->cq_1_index);
		log_dev_info->device_state = DEV_UNUSED;
		return ret;
	}

	return ret;
}

int log_ts_dfx_cqsq_release(void)
{
	struct devdrv_dev_ctx *dev_ctx = NULL;
	struct devdrv_mailbox_cqsq mailbox_cqsq = {0};
	int ret;
	struct log_device_info_t *log_dev_info = &(g_log_dev_info.log_dev_info);

	if (log_dev_info->device_state == DEV_UNUSED)
		return IDE_DRV_OK;

	dev_ctx  = get_dev_ctx_by_id(log_dev_info->device_id);
	if(dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n",log_dev_info->device_id);
		return IDE_DRV_ERROR;
	};

	//ts_work_status 0: power down
	if (dev_ctx->ts_work_status == 0) {
		devdrv_drv_err("device is not working.\n");
		return DEVDRV_TS_DOWN;
	}

	mailbox_cqsq.cmd_type  = DEVDRV_MAILBOX_RELEASE_CQSQ_LOG;
	mailbox_cqsq.sq_index  = (unsigned short)log_dev_info->sq_0_index;
	mailbox_cqsq.cq0_index = (unsigned short)log_dev_info->cq_0_index;
	mailbox_cqsq.cq1_index = (unsigned short)log_dev_info->cq_1_index;

	ret = devdrv_dfx_send_mailbox(dev_ctx, &mailbox_cqsq);
	if (ret != IDE_DRV_OK) {
		slog_drv_err("devdrv_mailbox_send_cqsq fail.ret = %d.\n", ret);
	}

	return ret;
}

int log_dfx_cqsq_exit(unsigned int dev_id)
{
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;
	struct log_device_info_t *log_dev_info = &(g_log_dev_info.log_dev_info);

	if (dev_id >= LOG_DEVICE_MAX) {
		slog_drv_err("dev_id = %u fail.\n", dev_id);
		return IDE_DRV_ERROR;
	}

	if (log_dev_info->device_state == DEV_UNUSED)
		return IDE_DRV_OK;

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if(cur_dev_ctx == NULL){
		devdrv_drv_err("cur_dev_ctx %d is null\n",dev_id);
		return IDE_DRV_ERROR;
	}

	log_all_stop(LOG_MODELE_EXIT);

	log_dev_info->device_state = DEV_UNUSED;
	devdrv_destroy_dfx_sq(cur_dev_ctx, log_dev_info->sq_0_index);
	devdrv_destroy_dfx_cq(cur_dev_ctx, log_dev_info->cq_0_index);
	devdrv_destroy_dfx_cq(cur_dev_ctx, log_dev_info->cq_1_index);

	return IDE_DRV_OK;
}

int log_dfx_cqsq_destroy(unsigned int dev_id)
{
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;
	struct log_device_info_t *log_dev_info = &(g_log_dev_info.log_dev_info);

	if (dev_id >= LOG_DEVICE_MAX) {
		slog_drv_err("dev_id = %u fail.\n", dev_id);
		return IDE_DRV_ERROR;
	}

	if (log_dev_info->device_state == DEV_UNUSED)
		return IDE_DRV_OK;

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if(cur_dev_ctx == NULL){
		devdrv_drv_err("cur_dev_ctx %d is null\n",dev_id);
		return IDE_DRV_ERROR;
	}

	log_all_stop(LOG_NPU_POWERDOWN);

	log_dev_info->device_state = DEV_UNUSED;
	devdrv_destroy_dfx_sq(cur_dev_ctx, log_dev_info->sq_0_index);
	devdrv_destroy_dfx_cq(cur_dev_ctx, log_dev_info->cq_0_index);
	devdrv_destroy_dfx_cq(cur_dev_ctx, log_dev_info->cq_1_index);

	return IDE_DRV_OK;
}

int log_send_cmd_to_ts(log_sq_scheduler_t *sq_info)
{
	int ret = 0;
	struct log_device_info_t *log_dev_info = &(g_log_dev_info.log_dev_info);

	ret = devdrv_dfx_send_sq(log_dev_info->device_id, log_dev_info->sq_0_index,
					(unsigned char *)sq_info, sizeof(log_sq_scheduler_t));
	if (ret != IDE_DRV_OK) {
		slog_drv_err("send sq error: %d, channel id = %u, cmd_verify = %u.\n",
			     ret, sq_info->channel_id, sq_info->cmd_verify);
		log_dev_info->cmd_verify++;

		return ret;
	}

	ret = down_timeout(&log_dev_info->cq0_wait_sema, TS2DRV_TIMEOUT);
	if (ret != IDE_DRV_OK) {
		slog_drv_err("down_timeout. channel_id: %u, cmd_verify: %u\n",
			     sq_info->channel_id, sq_info->cmd_verify);
		log_dev_info->cmd_verify++;
		return IDE_DRV_TIMEOUT;
	}

	slog_drv_debug("channel_id: %u, cmd_verify: %u,channel_cmd: %u sq_0_index: %u\n",
		sq_info->channel_id, sq_info->cmd_verify,
		sq_info->channel_cmd,log_dev_info->sq_0_index);

	return IDE_DRV_OK;
}

static int log_get_channel_share_memory(struct log_channel_info_t *channel_info)
{
	struct devdrv_dfx_desc *dfx_desc = NULL;
	struct devdrv_platform_info *plat_info = NULL;
	unsigned int can_use_size = 0;

	if ((channel_info->channel_id != LOG_CHANNEL_TS_ID)
		&& (channel_info->channel_id != LOG_CHANNEL_AICPU_ID)) {
		devdrv_drv_err("channel_id(%u) is invalid.\n", channel_info->channel_id);
		return IDE_DRV_ERROR;
	}

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_info.\n");
		return IDE_DRV_ERROR;
	}

	dfx_desc = &DEVDRV_PLAT_GET_DFX_DESC(plat_info, DEVDRV_DFX_DEV_LOG);
	channel_info->phy_addr = dfx_desc->bufs->base;
	//a half memory use ts, other half memory use ai core
	can_use_size = (dfx_desc->bufs->len)>>1;
	if (channel_info->channel_id == LOG_CHANNEL_AICPU_ID) {
		channel_info->phy_addr += can_use_size;
	}

	channel_info->buf_size = can_use_size;
	channel_info->vir_addr = (unsigned char *)ioremap_wc(channel_info->phy_addr,
		can_use_size);
	if (channel_info->vir_addr == NULL) {
		devdrv_drv_err("ioremap_nocache failed.\n");
		return IDE_DRV_ERROR;
	}

	devdrv_drv_debug("phy_addr = %pK, vir_addr = %pK, buf_size : %x.\n",
		(void *)(uintptr_t)channel_info->phy_addr,
		(void *)(uintptr_t)channel_info->vir_addr,
		channel_info->buf_size);

	return IDE_DRV_OK;
}

bool log_dfx_cqsq_is_created(void)
{
	unsigned int channel_idx;

	for (channel_idx = 0; channel_idx < LOG_CHANNEL_NUM; channel_idx ++) {
		if (g_log_dev_info.log_channel[channel_idx].channel_state == LOG_CHANNEL_ENABLE)
			return true;
	}
	return false;
}

bool log_dfx_cqsq_need_release(void)
{
	unsigned int channel_idx;

	for (channel_idx = 0; channel_idx < LOG_CHANNEL_NUM; channel_idx ++) {
		if (g_log_dev_info.log_channel[channel_idx].channel_state == LOG_CHANNEL_ENABLE)
			return false;
	}
	return true;
}

int log_create(log_ioctl_para_t *log_para)
{
	log_channel_info_t *channel_info = NULL;
	log_sq_scheduler_t  sq_slot = {0};
	log_buf_ptr_t *buf_ptr = NULL;
	int channel_id = 0;
	int ret = 0;
	unsigned int channel_idx = LOG_CHANNEL_NUM;
	struct log_device_info_t *log_dev_info = &(g_log_dev_info.log_dev_info);

	ret = log_check_ioctl_para(log_para);
	if (ret != IDE_DRV_OK) {
		slog_drv_err("log_create para invalid.\n");
		return IDE_DRV_ERROR;
	}

	channel_id = log_para->channel_id;

	if (log_para->buf_size < LOG_BUF_MIN_SIZE || log_para->buf_size > LOG_BUF_MAX_SIZE) {
		slog_drv_err("buf_size(%d) create invalid.\n", log_para->buf_size);
		return IDE_DRV_ERROR;
	}

	if (log_get_channel_index(channel_id, &channel_idx)) {
		slog_drv_err("log_get_channel_index fail. channel_id: %u\n", channel_id);
		return IDE_DRV_ERROR;
	}

	channel_info = (log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];

	mutex_lock(&channel_info->cmd_mutex);
	//allowed creating repeatedly, reliability strategy
	if (channel_info->channel_state == LOG_CHANNEL_ENABLE) {
		slog_drv_err("Channel(%d) create repeatedly.\n", channel_id);
		log_para->ret = IDE_DRV_OK;
		mutex_unlock(&channel_info->cmd_mutex);
		return IDE_DRV_OK;
	}

	channel_info->channel_type = log_para->channel_type;
	channel_info->channel_id   = log_para->channel_id;

	if (log_dfx_cqsq_is_created() == false)
	{
		ret = log_ts_dfx_cqsq_create();
		if (ret != 0) {
			slog_drv_err("log_create_cqsq failed, ret = %d.\n", ret);
			mutex_unlock(&channel_info->cmd_mutex);
			return IDE_DRV_ERROR;
		}
	}

	if (channel_info->vir_addr == NULL) {
		ret = log_get_channel_share_memory(channel_info);
		if (ret != IDE_DRV_OK) {
			slog_drv_err("get shm_memory failed!\n");
			mutex_unlock(&channel_info->cmd_mutex);
			return IDE_DRV_ERROR;
		}
	}

	buf_ptr = (log_buf_ptr_t *)(channel_info->vir_addr);
	buf_ptr->buf_read  = 0;
	buf_ptr->buf_write = 0;
	buf_ptr->buf_len = channel_info->buf_size;

	/* write channel send cmd */
	sq_slot.cmd_verify = (int)log_dev_info->cmd_verify;
	sq_slot.channel_id  = log_para->channel_id;
	sq_slot.channel_cmd = LOG_CREATE;
	sq_slot.buf_len = log_para->buf_size - DATA_BUF_HEAD;
	sq_slot.phy_addr = channel_info->phy_addr;

	ret = log_send_cmd_to_ts(&sq_slot);
	if (ret != IDE_DRV_OK) {
		slog_drv_err("channel_id = %d, ret = %d.\n", log_para->channel_id, ret);
		iounmap((void *)channel_info->vir_addr);
		channel_info->vir_addr = NULL;
		mutex_unlock(&channel_info->cmd_mutex);
		return ret;
	}

	log_para->ret = channel_info->ret_val;

	if (log_para->ret != IDE_DRV_OK) {
		slog_drv_err("channel_id = %d, ts_cmd_val = %d.\n", log_para->channel_id,
			     log_para->ret);
		iounmap((void *)channel_info->vir_addr);
		channel_info->vir_addr = NULL;
		mutex_unlock(&channel_info->cmd_mutex);
		return IDE_DRV_ERROR;
	}

	channel_info->channel_state = LOG_CHANNEL_ENABLE;
	channel_info->poll_flag = POLL_INVALID;

	mutex_unlock(&channel_info->cmd_mutex);
	devdrv_drv_warn("log_create success, channel_id = %u.\n", channel_info->channel_id);
	return IDE_DRV_OK;
}

int log_channel_init(void)
{
	unsigned int channel_idx = LOG_CHANNEL_NUM;
	log_channel_info_t *channel_info = NULL;
	log_buf_ptr_t *buf_ptr = NULL;
	int ret = 0;

	for (channel_idx = 0; channel_idx < LOG_CHANNEL_NUM; channel_idx ++) {
		channel_info = (log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];

		mutex_lock(&channel_info->cmd_mutex);
		if (channel_idx == LOG_CHANNEL_TS_IDX) {
			channel_info->channel_id = LOG_CHANNEL_TS_ID;
		} else if (channel_idx == LOG_CHANNEL_AICPU_IDX) {
			channel_info->channel_id = LOG_CHANNEL_AICPU_ID;
		}

		if (channel_info->vir_addr == NULL) {
			ret = log_get_channel_share_memory(channel_info);
			if (ret != IDE_DRV_OK) {
				slog_drv_err("get shm_memory failed!\n");
				mutex_unlock(&channel_info->cmd_mutex);
				return IDE_DRV_ERROR;
			}
		}

		channel_info->channel_state = LOG_CHANNEL_ENABLE;
		channel_info->poll_flag = POLL_INVALID;
		channel_info->channel_type = 0;
		buf_ptr = (log_buf_ptr_t *)(channel_info->vir_addr);
		buf_ptr->buf_read  = 0;
		buf_ptr->buf_write = 0;
		buf_ptr->buf_len = channel_info->buf_size;
		mutex_unlock(&channel_info->cmd_mutex);
	}

	return IDE_DRV_OK;
}

void log_channel_release(void)
{
	unsigned int channel_idx = LOG_CHANNEL_NUM;
	log_channel_info_t *channel_info = NULL;

	for (channel_idx = 0; channel_idx < LOG_CHANNEL_NUM; channel_idx ++) {
		channel_info = (log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];

		mutex_lock(&channel_info->cmd_mutex);
		channel_info->channel_state = LOG_CHANNEL_DISABLE;

		if(channel_info->vir_addr != NULL) {
			iounmap((void *)channel_info->vir_addr);
			channel_info->vir_addr = NULL;
		}
		mutex_unlock(&channel_info->cmd_mutex);
	}

	return;
}

int log_delete(log_ioctl_para_t *log_para)
{
	log_channel_info_t *channel_info = NULL;
	log_sq_scheduler_t  sq_slot = {0};
	int ret = 0;
	unsigned int channel_idx = LOG_CHANNEL_NUM;
	struct log_device_info_t *log_dev_info = &(g_log_dev_info.log_dev_info);

	ret = log_check_ioctl_para(log_para);
	if (ret != IDE_DRV_OK) {
		slog_drv_err("log_delete para invalid.\n");
		return IDE_DRV_ERROR;
	}

	if (log_get_channel_index(log_para->channel_id, &channel_idx)) {
		slog_drv_err("log_get_channel_index fail. channel_id: %u\n",
			log_para->channel_id);
		return IDE_DRV_ERROR;
	}

	channel_info = (log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];

	mutex_lock(&channel_info->cmd_mutex);
	if (channel_info->channel_state != LOG_CHANNEL_ENABLE) {
		mutex_unlock(&channel_info->cmd_mutex);
		slog_drv_err("channel(%d) not enable.\n", log_para->channel_id);
		return IDE_DRV_ERROR;
	}

	if (channel_info->channel_type != LOG_CHANNEL_TYPE_TS) {
		mutex_unlock(&channel_info->cmd_mutex);
		return IDE_DRV_OK;
	}

	/*in delete(), set channel state firstly*/
	channel_info->channel_state = LOG_CHANNEL_DISABLE;

	/* write channel send cmd */
	sq_slot.cmd_verify  = (int)log_dev_info->cmd_verify;
	sq_slot.channel_id  = log_para->channel_id;
	sq_slot.channel_cmd = LOG_DELETE;

	ret = log_send_cmd_to_ts(&sq_slot);
	if (ret != IDE_DRV_OK) {
		slog_drv_err("delete fail,channel_id = %d, ts_cmd_val = %d.\n", log_para->channel_id, ret);
		mutex_unlock(&channel_info->cmd_mutex);
		return ret;
	}

	log_para->ret = channel_info->ret_val;
	if (log_para->ret != IDE_DRV_OK) {
		slog_drv_err("channel_id = %d, ts_cmd_val = %d.\n", log_para->channel_id,
			     log_para->ret);
		mutex_unlock(&channel_info->cmd_mutex);
		return IDE_DRV_ERROR;
	}

	if (channel_info->vir_addr != NULL) {
		iounmap((void *)channel_info->vir_addr);
		channel_info->vir_addr = NULL;
	}

	if(log_dfx_cqsq_need_release() == true)
	{
		ret = log_ts_dfx_cqsq_release();
		if (ret != IDE_DRV_OK) {
			slog_drv_err("log_destroy_cqsq = %d.\n", log_para->channel_id);
			mutex_unlock(&channel_info->cmd_mutex);
			return IDE_DRV_ERROR;
		}
	}

	slog_drv_err("log_delete success.\n");
	mutex_unlock(&channel_info->cmd_mutex);
	return IDE_DRV_OK;
}

int log_lpm3_set(log_ioctl_para_t *log_para)
{
	slog_drv_info("log_lpm3_set success, log_level = %d\n",
		log_para->attrib.log_level);
	return IDE_DRV_OK;
}

int log_set(log_ioctl_para_t *log_para)
{
	struct log_device_info_t *log_dev_info = &(g_log_dev_info.log_dev_info);
	log_channel_info_t *channel_info = NULL;
	log_sq_scheduler_t  sq_slot = {0};
	int ret = 0;
	unsigned int channel_idx = LOG_CHANNEL_NUM;

	ret = log_check_ioctl_para(log_para);
	if (ret != IDE_DRV_OK) {
		slog_drv_err("log_set para invalid.\n");
		return IDE_DRV_ERROR;
	}

	if (log_get_channel_index(log_para->channel_id, &channel_idx)) {
		slog_drv_err("log_get_channel_index fail. channel_id: %u\n",
			log_para->channel_id);
		return IDE_DRV_ERROR;
	}

	channel_info = (log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];

	mutex_lock(&channel_info->cmd_mutex);
	if (channel_info->channel_state != LOG_CHANNEL_ENABLE) {
		slog_drv_err("Channel(%d) not create.\n", log_para->channel_id);
		mutex_unlock(&channel_info->cmd_mutex);
		return IDE_DRV_ERROR;
	}

	/* write channel send cmd */
	sq_slot.cmd_verify  = (int)log_dev_info->cmd_verify;
	sq_slot.channel_id  = log_para->channel_id;
	sq_slot.channel_cmd = LOG_SET;

	memcpy((void *)&sq_slot.attrib, (void *)&log_para->attrib, sizeof(attribute_t));

	ret = log_send_cmd_to_ts(&sq_slot);
	if (ret != IDE_DRV_OK) {
		slog_drv_err("channel_id = %d, ts_cmd_val = %d.\n", log_para->channel_id, ret);
		mutex_unlock(&channel_info->cmd_mutex);
		return ret;
	}

	log_para->ret = channel_info->ret_val;
	if (log_para->ret != IDE_DRV_OK) {
		slog_drv_err("channel_id = %d, ts_cmd_val = %d.\n", log_para->channel_id,
			     log_para->ret);
		mutex_unlock(&channel_info->cmd_mutex);
		return IDE_DRV_ERROR;
	}

	memcpy((void *)&channel_info->ioctl_para.attrib, (void *)&log_para->attrib,
	       sizeof(attribute_t));

	mutex_unlock(&channel_info->cmd_mutex);
	return IDE_DRV_OK;
}

int log_show(log_ioctl_para_t *log_para)
{
	log_channel_info_t *channel_info = NULL;
	int ret = 0;
	unsigned int channel_idx = LOG_CHANNEL_NUM;

	ret = log_check_ioctl_para(log_para);
	if (ret != IDE_DRV_OK) {
		slog_drv_err("log_set para invalid.\n");
		return IDE_DRV_ERROR;
	}

	if (log_get_channel_index(log_para->channel_id, &channel_idx)) {
		slog_drv_err("log_get_channel_index fail. channel_id: %u\n",
			log_para->channel_id);
		return IDE_DRV_ERROR;
	}

	channel_info = (log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];

	if (channel_info->channel_state != LOG_CHANNEL_ENABLE) {
		slog_drv_err("Channel(%d) not create.\n", log_para->channel_id);
		return IDE_DRV_ERROR;
	}

	memcpy((void *)&log_para->attrib,
		(void *)&channel_info->ioctl_para.attrib, sizeof(attribute_t));

	log_para->ret = IDE_DRV_OK;

	return IDE_DRV_OK;
}

int log_read_ringbuffer(log_ioctl_para_t *log_para,
	unsigned int buf_read, unsigned int buf_write, log_channel_info_t *channel_info)
{
	char *out_buf = NULL;
	unsigned char *base = NULL;
	log_buf_ptr_t *buf_ptr = NULL;
	unsigned int   buf_len	 = 0;

	out_buf = log_para->out_buf;
	if ((out_buf == NULL) || (channel_info->vir_addr == NULL)) {
		slog_drv_err("para->out_buf is null.\n");
		return IDE_DRV_ERROR;
	}

	buf_ptr = (log_buf_ptr_t *)channel_info->vir_addr;

	if (buf_write > buf_read) {
		buf_len =(buf_write - buf_read) - ((buf_write - buf_read) % LOG_MEMORY_ALIGN);
		base = channel_info->vir_addr + DATA_BUF_HEAD + buf_read;

		if(buf_len == 0)
		{
			log_para->buf_size = 0;
			log_para->ret = IDE_DRV_OK;
			return IDE_DRV_OK;
		}
		if (copy_to_user_safe((void __user *)out_buf, (void *)base, buf_len) != 0) {
			slog_drv_warn("copy log fail.\n");
			return IDE_DRV_ERROR;
		}

		atomic_set((atomic_t *)&buf_ptr->buf_read, buf_write);
		log_para->buf_size = buf_len;
		log_para->ret = IDE_DRV_OK;
		return IDE_DRV_OK;
	}

	if (buf_read < (channel_info->buf_size - DATA_BUF_HEAD)) {
		buf_len =(channel_info->buf_size - DATA_BUF_HEAD - buf_read) -
				((channel_info->buf_size - DATA_BUF_HEAD - buf_read) % LOG_MEMORY_ALIGN);
		base = channel_info->vir_addr + DATA_BUF_HEAD + buf_read;

		if(buf_len == 0)
		{
			log_para->buf_size = 0;
			log_para->ret = IDE_DRV_OK;
			return IDE_DRV_OK;
		}
		if (copy_to_user_safe((void __user *)out_buf, (void *)base, buf_len) != 0) {
			slog_drv_warn("copy log fail.\n");
			return IDE_DRV_ERROR;
		}

		if (buf_write == 0) {
			atomic_set((atomic_t *)&buf_ptr->buf_read, buf_write);
			log_para->buf_size = buf_len;
			log_para->ret = IDE_DRV_OK;
			return IDE_DRV_OK;
		}
	}

	/*notice user buffer's offset; copy_to_user_safe() is defined in device manager ko*/
	base = channel_info->vir_addr + DATA_BUF_HEAD;
	if (copy_to_user_safe((void *)(out_buf + buf_len), (void *)base, buf_write) != 0) {
		slog_drv_warn("copy log fail.\n");
		return IDE_DRV_ERROR;
	}

	atomic_set((atomic_t *)&buf_ptr->buf_read, buf_write);
	log_para->buf_size = buf_len + buf_write;
	log_para->ret = IDE_DRV_OK;
	return IDE_DRV_OK;
}

int log_read(log_ioctl_para_t *log_para)
{
	log_channel_info_t *channel_info = NULL;
	log_buf_ptr_t *buf_ptr = NULL;
	unsigned int   buf_read  = 0;
	unsigned int   buf_write = 0;
	int channel_id;
	int ret = 0;
	unsigned int channel_idx = LOG_CHANNEL_NUM;

	ret = log_check_ioctl_para(log_para);
	if (ret != IDE_DRV_OK) {
		slog_drv_err("log_read para invalid.\n");
		return IDE_DRV_ERROR;
	}

	channel_id = log_para->channel_id;
	log_para->ret = IDE_DRV_ERROR;

	if (log_get_channel_index(log_para->channel_id, &channel_idx)) {
		slog_drv_err("log_get_channel_index fail. channel_id: %u\n", channel_id);
		return IDE_DRV_ERROR;
	}

	channel_info = (log_channel_info_t *)&g_log_dev_info.log_channel[channel_idx];

	mutex_lock(&channel_info->cmd_mutex);
	if (channel_info->channel_state != LOG_CHANNEL_ENABLE) {
		slog_drv_err("Channel(%d) not create.\n", log_para->channel_id);
		mutex_unlock(&channel_info->cmd_mutex);
		return IDE_DRV_ERROR;
	}

	if (channel_info->vir_addr == NULL) {
		slog_drv_err("Channel(%d) vir_addr is null.\n", log_para->channel_id);
		mutex_unlock(&channel_info->cmd_mutex);
		return IDE_DRV_ERROR;
	}

	log_para->buf_size = 0;
	buf_ptr = (log_buf_ptr_t *)channel_info->vir_addr;

	buf_read  = buf_ptr->buf_read;
	buf_write = atomic_read((atomic_t *)&buf_ptr->buf_write);

	/*print_num is set to control log's print for every channel; and always ret; */
	if (buf_write > (channel_info->buf_size - DATA_BUF_HEAD) ||
		buf_read > (channel_info->buf_size - DATA_BUF_HEAD)) {
		if (channel_info->print_num < LOG_PRINT_MAX) {
			slog_drv_err("log's buffer is error. channel_id = %d, len = %lu, read = %u, write = %u.\n",
					channel_id, (channel_info->buf_size - DATA_BUF_HEAD), buf_read, buf_write);
			channel_info->print_num ++;
			}
		mutex_unlock(&channel_info->cmd_mutex);
		return IDE_DRV_ERROR;
	}

	devdrv_drv_debug("log_read log_para->channel_id = %d, buf_write = %d, buf_read = %d.\n",
					log_para->channel_id, buf_write, buf_read);
	if (buf_write == buf_read)
	{
		log_para->ret = IDE_DRV_OK;
		mutex_unlock(&channel_info->cmd_mutex);
		return IDE_DRV_OK;
	}

	ret = log_read_ringbuffer(log_para, buf_read, buf_write, channel_info);
	if (ret != IDE_DRV_OK) {
		slog_drv_info("log_read_ringbuffer error, ret = %d, channel_id = %d, buf_read = %u, buf_write = %u\n",
			ret, channel_id, buf_read, buf_write);
		mutex_unlock(&channel_info->cmd_mutex);
		return ret;
	}

	mutex_unlock(&channel_info->cmd_mutex);
	return IDE_DRV_OK;
}

/*
flag = 0, when module exit, flag is 0.
flag = 1, when npu powerdown, flag is 1.
*/
void log_all_stop(int flag)
{
	struct log_device_info_t *log_dev_info = &(g_log_dev_info.log_dev_info);
	log_channel_info_t *channel_info = NULL;
	log_sq_scheduler_t  sq_slot = {0};
	int ret;
	int i;

	for (i = 0; i < LOG_CHANNEL_NUM; i++) {
		channel_info = (struct log_channel_info_t *)&g_log_dev_info.log_channel[i];

		if (channel_info->channel_state != LOG_CHANNEL_ENABLE) {
			continue;
		}

		if (flag > 0) {
			mutex_lock(&channel_info->cmd_mutex);
			if(channel_info->vir_addr != NULL) {
				iounmap((void *)channel_info->vir_addr);
				channel_info->vir_addr = NULL;
			}
			channel_info->channel_state = LOG_CHANNEL_DISABLE;
			mutex_unlock(&channel_info->cmd_mutex);
			continue;
		}

		sq_slot.cmd_verify  = (int)log_dev_info->cmd_verify;
		sq_slot.channel_id  = channel_info->channel_id;
		sq_slot.channel_cmd = LOG_DELETE;

		ret = log_send_cmd_to_ts(&sq_slot);
		if (ret != IDE_DRV_OK) {
			slog_drv_err("channel_id = %d, ts_cmd_val = %d.\n", i, ret);
		}
		mutex_lock(&channel_info->cmd_mutex);
		channel_info->channel_state = LOG_CHANNEL_DISABLE;
		log_dev_info->cmd_verify++;

		if (channel_info->vir_addr != NULL) {
			iounmap(channel_info->vir_addr);
			channel_info->vir_addr = NULL;
		}
		mutex_unlock(&channel_info->cmd_mutex);
	}

	ret = log_ts_dfx_cqsq_release();
	if (ret != IDE_DRV_OK) {
		slog_drv_err("channel_id = %d, ts_cmd_val = %d.\n", i, ret);
	}

	return;
}

int log_drv_get_devinfo(log_ioctl_para_t *log_para)
{

	slog_drv_err("devdrv_get_devinfo error.\n");
	return IDE_DRV_ERROR;
}

int log_drv_get_device_id(log_ioctl_para_t *log_para)
{
	int ret;
	unsigned int dev_id = 0;

	/*at device, devdrv_get_devids return the first member for device id;
	and at host , devdrv_get_devids require ids.*/
	ret = devdrv_get_devids(&dev_id);
	if (ret != IDE_DRV_OK) {
		slog_drv_err("devdrv_get_devids error: %d\n", ret);
		return IDE_DRV_ERROR;
	}

	log_para->device_id = dev_id;
	log_para->ret = IDE_DRV_OK;

	return IDE_DRV_OK;
}

int log_check_ioctl_para(struct log_ioctl_para_t* log_para)
{
	struct devdrv_platform_info *plat_info = NULL;
	struct devdrv_dfx_desc *dfx_desc = NULL;
	int i;

	if (log_para == NULL) {
		devdrv_drv_err("NULL para.\n");
		return IDE_DRV_ERROR;
	}

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL)
	{
		devdrv_drv_err("devdrv_plat_get_info fail.\n");
		return IDE_DRV_ERROR;
	}

	dfx_desc = &DEVDRV_PLAT_GET_DFX_DESC(plat_info, DEVDRV_DFX_DEV_LOG);
	if (dfx_desc->channel_num > DEVDRV_DFX_CHANNEL_MAX_RESOURCE) {
		slog_drv_err("channel_num(%d) cofig error.\n", dfx_desc->channel_num);
		return IDE_DRV_ERROR;
	}

	for ( i =0;i < dfx_desc->channel_num; i++) {
		if(dfx_desc->channels[i] == log_para->channel_id) break;
	}

	if (i == dfx_desc->channel_num) {
		slog_drv_err("channel_id(%d) create invalid.\n", log_para->channel_id);
		return IDE_DRV_ERROR;
	}

	return 0;
}


long log_drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct log_ioctl_para_t log_para = {0};
	unsigned int drv_cmd = 0;
	void __user *parg = NULL;
	int ret = 0;

	drv_cmd = _IOC_NR(cmd);
	parg = (void __user *)(uintptr_t)arg;
	if (parg == NULL) {
		slog_drv_err("user arg error.parg is null\n");
		return IDE_DRV_ERROR;
	}

	if (copy_from_user(&log_para, parg, sizeof(log_ioctl_para_t))) {
		slog_drv_err("copy_from_user error.\n");
		return IDE_DRV_ERROR;
	}

	switch (drv_cmd) {
	case LOG_TEST:
		flag_test++;
		break;
	case LOG_SET:
		ret = log_set((log_ioctl_para_t *)&log_para);
		break;
	case LOG_SHOW:
		ret = log_show((log_ioctl_para_t *)&log_para);
		break;
	case LOG_READ:
		ret = log_read((log_ioctl_para_t *)&log_para);
		break;
	case LOG_DEVICE_INFO:
		ret = log_drv_get_devinfo((log_ioctl_para_t *)&log_para);
		break;
	case LOG_DEVICE_ID:
		ret = log_drv_get_device_id((log_ioctl_para_t *)&log_para);
		break;
	default:
		slog_drv_err("log ioctl cmd(%d) illegal.\n", drv_cmd);
		return IDE_DRV_ERROR;
	}

	if (copy_to_user_safe(parg, (void *)&log_para,
			 sizeof(log_ioctl_para_t)) != 0) {
		slog_drv_err("copy_to_user error.\n");
		return IDE_DRV_ERROR;
	}

	return ret;
}

int log_drv_release(struct inode *inode, struct file *filp)
{
	if (flag_test-- > 0)
		return IDE_DRV_OK;


	return IDE_DRV_OK;
}

int log_drv_init(unsigned int dev_id)
{
	int channel_idx;
	struct log_device_info_t *log_dev_info = &(g_log_dev_info.log_dev_info);

	log_dev_info->device_state = DEV_UNUSED;
	log_dev_info->cmd_verify = 1;
	log_dev_info->device_id	 = dev_id;
	log_dev_info->poll_head	 = 0;
	log_dev_info->poll_tail	 = 0;
	sema_init(&log_dev_info->cq0_wait_sema, 0);
	sema_init(&log_dev_info->cq1_wait_sema, 0);
	mutex_init(&log_dev_info->cq0_mutex);
	mutex_init(&log_dev_info->cq1_mutex);
	init_waitqueue_head(&log_dev_info->cq1_wq);

	g_log_dev_info.poll_box = (log_poll_info_t *)kzalloc(
		sizeof(log_poll_info_t) * LOG_POLL_DEPTH, GFP_KERNEL);
	if (g_log_dev_info.poll_box == NULL) {
		slog_drv_err("pollbox kzalloc is error\n");
		return IDE_DRV_ERROR;
	}

	memset(g_log_dev_info.log_channel, 0, sizeof(log_channel_info_t)*LOG_CHANNEL_NUM);

	for (channel_idx = 0; channel_idx < LOG_CHANNEL_NUM; channel_idx ++) {
		mutex_init(&g_log_dev_info.log_channel[channel_idx].cmd_mutex);
	}

	g_log_dev_info.op_cb_idx = DEVDRV_CALLBACK_REG_NUM;
	g_log_dev_info.rl_cb_idx = DEVDRV_CALLBACK_REG_NUM;

	return IDE_DRV_OK;
}

void log_drv_uninit(void)
{
	memset(g_log_dev_info.log_channel, 0, sizeof(log_channel_info_t)*LOG_CHANNEL_NUM);

	if (g_log_dev_info.poll_box != NULL) {
		kfree((void *)g_log_dev_info.poll_box);
		g_log_dev_info.poll_box = NULL;
	}

	return;
}

const struct file_operations log_drv_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl = log_drv_ioctl,
	.release	= log_drv_release,
};

int log_drv_register_cdev(void)
{
	int ret;
	unsigned int major;
	dev_t devno;

	struct char_device *priv = (struct char_device *)&g_log_dev_info.char_dev_log;

	priv->devno = 0;
	ret = alloc_chrdev_region(&priv->devno, 0, 1, CHAR_DRIVER_NAME);
	if (ret < 0)
		return IDE_DRV_ERROR;

	/* init and add char device */
	major = MAJOR(priv->devno);
	devno = MKDEV(major, 0);
	cdev_init(&priv->cdev, &log_drv_fops);
	priv->cdev.owner = THIS_MODULE;
	priv->cdev.ops = &log_drv_fops;

	if (cdev_add(&priv->cdev, devno, 1)) {
		unregister_chrdev_region(devno, 1);
		return IDE_DRV_ERROR;
	}

	priv->dev_class = class_create(THIS_MODULE, CHAR_DRIVER_NAME);
	if (IS_ERR(priv->dev_class)) {
		(void)unregister_chrdev_region(devno, 1);
		(void)cdev_del(&priv->cdev);
		return IDE_DRV_ERROR;
	}
	if(priv->dev_class != NULL)
		priv->dev_class->devnode = log_devnode;

	g_log_dev_info.log_dev = device_create(priv->dev_class, NULL, devno, NULL,
			    CHAR_DRIVER_NAME);
	if (g_log_dev_info.log_dev == NULL) {
		slog_drv_err("device_create error.devno = %d\n", devno);
		(void)class_destroy(priv->dev_class);
		(void)unregister_chrdev_region(devno, 1);
		(void)cdev_del(&priv->cdev);
		return IDE_DRV_ERROR;
	}

	return IDE_DRV_OK;
}

void log_drv_free_cdev(void)
{
	struct char_device *priv = &(g_log_dev_info.char_dev_log);

	(void)device_destroy(priv->dev_class, priv->devno);
	(void)class_destroy(priv->dev_class);
	(void)unregister_chrdev_region(priv->devno, 1);
	(void)cdev_del(&priv->cdev);
}

int __init log_drv_module_init(void)
{
	int ret = IDE_DRV_OK;
	const unsigned int dev_id = 0;

	memset(&g_log_dev_info, 0, sizeof(log_char_dev_global_info_t));

	ret = log_drv_register_cdev();
	if (ret != IDE_DRV_OK) {
		slog_drv_err("create character device fail.\n");
		return IDE_DRV_ERROR;
	}

	ret = log_drv_init(dev_id);
	if (ret != IDE_DRV_OK) {
		log_drv_free_cdev();
		slog_drv_err("log drv init fail.\n");
		return IDE_DRV_ERROR;
	}

	ret = log_channel_init();
	if (ret != IDE_DRV_OK) {
			log_drv_uninit();
			log_drv_free_cdev();
			slog_drv_err("log drv init fail.\n");
			return IDE_DRV_ERROR;
	}

	slog_drv_info("device (%u) log drv load ok.\n", dev_id);

	return IDE_DRV_OK;
}

void __exit log_drv_module_exit(void)
{
	log_channel_release();

	log_drv_uninit();

	/*char device free at last.*/
	log_drv_free_cdev();

	slog_drv_info("device (%u) log drv exit.\n", g_log_dev_info.log_dev_info.device_id);
}

module_init(log_drv_module_init);

module_exit(log_drv_module_exit);

MODULE_DESCRIPTION("log driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_VERSION(LOG_DRV_MODE_VERSION);
