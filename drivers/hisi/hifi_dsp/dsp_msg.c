/*
 * dsp msg.c
 *
 * dsp msg driver.
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "dsp_msg.h"

#include <linux/slab.h>
#include <linux/uaccess.h>

#include <dsm/dsm_pub.h>

#include "audio_hifi.h"
#include "drv_mailbox_msg.h"
#include "bsp_drv_ipc.h"
#include "dsp_om.h"
#include "om_debug.h"
#include "om_bigdata.h"
#include "usbaudio_ioctl.h"
#include "soundtrigger_socdsp_mailbox.h"

#include "hisi_lb.h"
#include "dsp_lpp.h"

#define FPGA_TIMEOUT_LEN_MS 10000
#define ASIC_TIMEOUT_LEN_MS 2000
#define SIZE_LIMIT_PARAM 256 /* IOCTL: io size */

#define SEND_MSG_TO_DSP mailbox_send_msg

LIST_HEAD(recv_sync_work_queue_head);

struct misc_msg_info {
	unsigned short msg_id;
	const char * const info;
};

struct dsp_msg_priv {
	spinlock_t recv_sync_lock;
	struct completion completion;
	unsigned int sn;
	unsigned char *base_virt;
	unsigned char *base_phy;
};

struct dsp_msg_priv g_msg_priv;

static const struct misc_msg_info g_msg_info[] = {
	{ ID_AP_AUDIO_SET_DTS_ENABLE_CMD, "ID_AP_AUDIO_SET_DTS_ENABLE_CMD" },
	{ ID_AP_AUDIO_SET_DTS_DEV_CMD, "ID_AP_AUDIO_SET_DTS_DEV_CMD" },
	{ ID_AP_AUDIO_SET_DTS_GEQ_CMD, "ID_AP_AUDIO_SET_DTS_GEQ_CMD" },
	{ ID_AP_AUDIO_SET_DTS_GEQ_ENABLE_CMD, "ID_AP_AUDIO_SET_DTS_GEQ_ENABLE_CMD" },
	{ ID_AP_DSP_VOICE_RECORD_START_CMD, "ID_AP_DSP_VOICE_RECORD_START_CMD" },
	{ ID_AP_DSP_VOICE_RECORD_STOP_CMD, "ID_AP_DSP_VOICE_RECORD_STOP_CMD" },
	{ ID_AP_VOICEPP_START_REQ, "ID_AP_VOICEPP_START_REQ" },
	{ ID_VOICEPP_AP_START_CNF, "ID_VOICEPP_AP_START_CNF" },
	{ ID_AP_VOICEPP_STOP_REQ, "ID_AP_VOICEPP_STOP_REQ" },
	{ ID_VOICEPP_AP_STOP_CNF, "ID_VOICEPP_AP_STOP_CNF" },
	{ ID_AP_AUDIO_PLAY_START_REQ, "ID_AP_AUDIO_PLAY_START_REQ" },
	{ ID_AUDIO_AP_PLAY_START_CNF, "ID_AUDIO_AP_PLAY_START_CNF" },
	{ ID_AP_AUDIO_PLAY_PAUSE_REQ, "ID_AP_AUDIO_PLAY_PAUSE_REQ" },
	{ ID_AUDIO_AP_PLAY_PAUSE_CNF, " ID_AUDIO_AP_PLAY_PAUSE_CNF" },
	{ ID_AUDIO_AP_PLAY_DONE_IND, "ID_AUDIO_AP_PLAY_DONE_IND" },
	{ ID_AP_AUDIO_PLAY_UPDATE_BUF_CMD, "ID_AP_AUDIO_PLAY_UPDATE_BUF_CMD" },
	{ ID_AP_AUDIO_PLAY_QUERY_TIME_REQ, "ID_AP_AUDIO_PLAY_QUERY_TIME_REQ" },
	{ ID_AP_AUDIO_PLAY_WAKEUPTHREAD_REQ, "ID_AP_AUDIO_PLAY_WAKEUPTHREAD_REQ" },
	{ ID_AUDIO_AP_PLAY_QUERY_TIME_CNF, "ID_AUDIO_AP_PLAY_QUERY_TIME_CNF" },
	{ ID_AP_AUDIO_PLAY_QUERY_STATUS_REQ, "ID_AP_AUDIO_PLAY_QUERY_STATUS_REQ" },
	{ ID_AUDIO_AP_PLAY_QUERY_STATUS_CNF, "ID_AUDIO_AP_PLAY_QUERY_STATUS_CNF" },
	{ ID_AP_AUDIO_PLAY_SEEK_REQ, "ID_AP_AUDIO_PLAY_SEEK_REQ" },
	{ ID_AUDIO_AP_PLAY_SEEK_CNF, "ID_AUDIO_AP_PLAY_SEEK_CNF" },
	{ ID_AP_AUDIO_PLAY_SET_VOL_CMD, "ID_AP_AUDIO_PLAY_SET_VOL_CMD" },
	{ ID_AP_AUDIO_RECORD_PCM_HOOK_CMD, "ID_AP_AUDIO_RECORD_PCM_HOOK_CMD" },
	{ ID_AP_DSP_ENHANCE_START_REQ, "ID_AP_DSP_ENHANCE_START_REQ" },
	{ ID_DSP_AP_ENHANCE_START_CNF, "ID_DSP_AP_ENHANCE_START_CNF" },
	{ ID_AP_DSP_ENHANCE_STOP_REQ, "ID_AP_DSP_ENHANCE_STOP_REQ" },
	{ ID_DSP_AP_ENHANCE_STOP_CNF, "ID_DSP_AP_ENHANCE_STOP_CNF" },
	{ ID_AP_DSP_ENHANCE_SET_DEVICE_REQ, "ID_AP_DSP_ENHANCE_SET_DEVICE_REQ" },
	{ ID_DSP_AP_ENHANCE_SET_DEVICE_CNF, "ID_DSP_AP_ENHANCE_SET_DEVICE_CNF" },
	{ ID_AP_AUDIO_ENHANCE_SET_DEVICE_IND, "ID_AP_AUDIO_ENHANCE_SET_DEVICE_IND" },
	{ ID_AP_AUDIO_MLIB_SET_PARA_IND, "ID_AP_AUDIO_MLIB_SET_PARA_IND" },
	{ ID_AP_AUDIO_CMD_SET_SOURCE_CMD, "ID_AP_AUDIO_CMD_SET_SOURCE_CMD" },
	{ ID_AP_AUDIO_CMD_SET_DEVICE_CMD, "ID_AP_AUDIO_CMD_SET_DEVICE_CMD" },
	{ ID_AP_AUDIO_CMD_SET_MODE_CMD, "ID_AP_AUDIO_CMD_SET_MODE_CMD" },
	{ ID_AP_AUDIO_CMD_SET_ANGLE_CMD, "ID_AP_AUDIO_CMD_SET_ANGLE_CMD" },
	{ ID_AP_AUDIO_ROUTING_COMPLETE_REQ, "ID_AP_AUDIO_ROUTING_COMPLETE_REQ" },
	{ ID_AUDIO_AP_OM_DUMP_CMD, "ID_AUDIO_AP_OM_DUMP_CMD" },
	{ ID_AUDIO_AP_FADE_OUT_REQ, "ID_AUDIO_AP_FADE_OUT_REQ" },
	{ ID_AP_ENABLE_MODEM_LOOP_REQ, "ID_AP_ENABLE_MODEM_LOOP_REQ" },
	{ ID_AP_ENABLE_AT_DSP_LOOP_REQ, "ID_AP_ENABLE_AT_DSP_LOOP_REQ" },
	{ ID_AP_AUDIO_DYN_EFFECT_GET_PARAM, "ID_AP_AUDIO_DYN_EFFECT_GET_PARAM" },
	{ ID_AP_AUDIO_DYN_EFFECT_GET_PARAM_CNF, "ID_AP_AUDIO_DYN_EFFECT_GET_PARAM_CNF" },
	{ ID_AP_AUDIO_DYN_EFFECT_TRIGGER, "ID_AP_AUDIO_DYN_EFFECT_TRIGGER" },
	{ ID_AP_DSP_REQUEST_SET_PARA_CMD, "ID_AP_DSP_REQUEST_SET_PARA_CMD" },
	{ ID_AP_DSP_REQUEST_GET_PARA_CMD, "ID_AP_DSP_REQUEST_GET_PARA_CMD" },
	{ ID_AP_DSP_REQUEST_GET_PARA_CNF, "ID_AP_DSP_REQUEST_GET_PARA_CNF" },
	{ ID_AP_DSP_I2S_TEST_POWER_REQ, "ID_AP_DSP_I2S_TEST_POWER_REQ" },
	{ ID_DSP_AP_I2S_TEST_POWER_CNF, "ID_DSP_AP_I2S_TEST_POWER_CNF" },
	{ ID_AP_DSP_SET_FM_CMD, "ID_AP_DSP_SET_FM_CMD" },
	{ ID_AP_SOCDSP_MADTEST_START, "ID_AP_SOCDSP_MADTEST_START" },
	{ ID_AP_SOCDSP_MADTEST_STOP, "ID_AP_SOCDSP_MADTEST_STOP" },
	{ ID_AP_HIFI_SET_VALUE_METER_SWITCH_CMD, "ID_AP_HIFI_SET_VALUE_METER_SWITCH_CMD" },
	{ ID_AP_HIFI_SET_WIRED_HEADSET_PARA_CMD, "ID_AP_HIFI_SET_WIRED_HEADSET_PARA_CMD" },
	{ ID_AP_HIFI_GET_VALUE_DATA_CMD, "ID_AP_HIFI_GET_VALUE_DATA_CMD" },
	{ ID_HIFI_AP_GET_VALUE_DATA_CNF, "ID_HIFI_AP_GET_VALUE_DATA_CNF" },
};

extern int hisi_dptx_set_aparam(unsigned int channel_num,
	unsigned int data_width, unsigned int sample_rate);
extern int hisi_dptx_get_spec(void *data, unsigned int size, unsigned int *ext_acount);

int rcv_dsp_sync_msg(struct list_head *node, unsigned int sn)
{
	if (sn == g_msg_priv.sn) {
		spin_lock_bh(&g_msg_priv.recv_sync_lock);
		list_add_tail(node, &recv_sync_work_queue_head);
		spin_unlock_bh(&g_msg_priv.recv_sync_lock);

		complete(&g_msg_priv.completion);
		return 0;
	}

	loge("msg priv sn: %u, cmd para sn: %u\n", g_msg_priv.sn, sn);

	return -EINVAL;
}

static void dsp_msg_info(const unsigned short msg_id)
{
	int i;
	int size;

	if ((msg_id == ID_AP_AUDIO_PLAY_QUERY_TIME_REQ) ||
			(msg_id == ID_AUDIO_AP_PLAY_QUERY_TIME_CNF))
		return;

	size = ARRAY_SIZE(g_msg_info);
	for (i = 0; i < size; i++) {
		if (g_msg_info[i].msg_id == msg_id) {
			logi("msg: %s\n", g_msg_info[i].info);
			break;
		}
	}

	if (i == size)
		logw("not defined msg id: 0x%x\n", msg_id);
}

static int async_write(const unsigned char *arg, unsigned int len)
{
	int ret;

	if (!arg) {
		loge("input arg is null\n");
		return ERROR;
	}

	ret = SEND_MSG_TO_DSP(MAILBOX_MAILCODE_ACPU_TO_HIFI_MISC, arg, len);
	if (ret != OK) {
		loge("msg send to dsp fail, ret is %d\n", ret);
		return ERROR;
	}

	return OK;
}

static int sync_write(const unsigned char  *buff, unsigned int len)
{
	int ret;
	unsigned long wait_result;
	static bool is_write_success;

	if (!buff) {
		loge("input arg is null\n");
		return ERROR;
	}

	reinit_completion(&g_msg_priv.completion);

	ret = SEND_MSG_TO_DSP(MAILBOX_MAILCODE_ACPU_TO_HIFI_MISC, buff, len);
	if (ret != OK) {
		loge("msg send to dsp fail, ret: %d\n", ret);
		return ERROR;
	}

	if (dsp_misc_get_platform_type() == DSP_PLATFORM_FPGA)
		wait_result = wait_for_completion_timeout(&g_msg_priv.completion,
			msecs_to_jiffies(FPGA_TIMEOUT_LEN_MS));
	else
		wait_result = wait_for_completion_timeout(&g_msg_priv.completion,
			msecs_to_jiffies(ASIC_TIMEOUT_LEN_MS));

	g_msg_priv.sn++;
	if (unlikely(g_msg_priv.sn & 0x10000000))
		g_msg_priv.sn = 0;

	if (!wait_result) {
		if (is_write_success) {
			loge("wait completion timeout\n");
			socdsp_dump_panic_log();
		}
		ret = ERROR;
	} else {
		is_write_success = true;
		ret = OK;
	}

	return ret;
}

static int get_input_param(unsigned int usr_para_size, const void *usr_para_addr,
	unsigned int *kernel_para_size, void **kernel_para_addr)
{
	void *para_in = NULL;
	unsigned int para_size_in;

	if ((usr_para_size == 0) ||
		(usr_para_size > SIZE_LIMIT_PARAM - SIZE_CMD_ID)) {
		loge("usr para size: %u exceed limit: 0/%u\n",
			usr_para_size, SIZE_LIMIT_PARAM - SIZE_CMD_ID);
		return ERROR;
	}

	para_size_in = usr_para_size + SIZE_CMD_ID;
	para_in = kzalloc(para_size_in, GFP_KERNEL);
	if (!para_in) {
		loge("kzalloc fail\n");
		return ERROR;
	}

	if (!usr_para_addr) {
		loge("usr para addr is null no user data\n");
		kfree(para_in);
		return ERROR;
	}

	if (try_copy_from_user(para_in, usr_para_addr, usr_para_size)) {
		loge("copy from user fail\n");
		kfree(para_in);
		return ERROR;
	}

	*kernel_para_size = para_size_in;
	*kernel_para_addr = para_in;

	dsp_msg_info(*(const unsigned short *)para_in);

	return OK;
}

static void input_param_free(void **kernel_para_addr)
{
	if (*kernel_para_addr) {
		kfree(*kernel_para_addr);
		*kernel_para_addr = NULL;
	} else {
		loge("kernel para addr to free is null\n");
	}
}

static int get_output_param(unsigned int kernel_para_size, const void *kernel_para_addr,
	unsigned int *usr_para_size, void __user *usr_para_addr)
{
	int ret = OK;
	void __user *para_to = NULL;
	unsigned int para_n;

	if (!kernel_para_addr) {
		loge("kernel para addr is null\n");
		return -EINVAL;
	}

	if (!usr_para_addr || !usr_para_size) {
		loge("usr size p: 0x%pK, usr addr: 0x%pK\n",
			usr_para_size, usr_para_addr);
		ret = -EINVAL;
		goto end;
	}

	para_to = usr_para_addr;
	para_n = kernel_para_size;
	if ((para_n == 0) || (para_n > SIZE_LIMIT_PARAM)) {
		loge("para n exceed limit: %u, %d\n", para_n, SIZE_LIMIT_PARAM);
		ret = -EINVAL;
		goto end;
	}

	if (para_n > *usr_para_size) {
		loge("para n exceed usr size: %u, %u\n", para_n, *usr_para_size);
		ret = -EINVAL;
		goto end;
	}

	/* Copy data from kernel space to user space: to from n */
	if (try_copy_to_user(para_to, kernel_para_addr, para_n)) {
		loge("copy to user fail\n");
		ret = COPYFAIL;
		goto end;
	}

	*usr_para_size = para_n;

end:
	dsp_msg_info(*(const unsigned short *)kernel_para_addr);
	return ret;
}

static int async_cmd(uintptr_t arg)
{
	int ret = OK;
	struct misc_io_async_param param;
	void *para_kernel_in = NULL;
	unsigned int para_kernel_size_in = 0;
	struct dsp_chn_cmd *cmd_para = NULL;
	void *para_addr_in = NULL;

	if (try_copy_from_user(&param, (void *)arg,
		sizeof(struct misc_io_async_param))) {
		loge("copy from user fail\n");
		ret = ERROR;
		goto end;
	}

	para_addr_in = INT_TO_ADDR(param.para_in_l, param.para_in_h);
	ret = get_input_param(param.para_size_in, para_addr_in,
		&para_kernel_size_in, &para_kernel_in);
	if (ret != OK) {
		loge("get ret: %d\n", ret);
		goto end;
	}
	/* add cmd id and sn  */
	cmd_para = (struct dsp_chn_cmd *)(para_kernel_in + para_kernel_size_in - SIZE_CMD_ID);
	cmd_para->cmd_type = HIFI_CHN_SYNC_CMD;
	cmd_para->sn = ACPU_TO_HIFI_ASYNC_CMD;

	ret = async_write(para_kernel_in, para_kernel_size_in);
	if (ret != OK) {
		loge("async write ret: %d\n", ret);
		goto end;
	}

	if (*(unsigned short *)para_kernel_in == ID_AP_AUDIO_PLAY_UPDATE_BUF_CMD)
		release_update_buff_wakelock();

end:
	input_param_free(&para_kernel_in);
	return ret;
}

static int rev_msg_get(unsigned char *mail_buf, unsigned int *mail_len)
{
	int ret = OK;
	struct recv_request *recv = NULL;
	unsigned int len;

	spin_lock_bh(&g_msg_priv.recv_sync_lock);

	if (!list_empty(&recv_sync_work_queue_head)) {
		recv = list_entry(recv_sync_work_queue_head.next,
			struct recv_request, recv_node);

		len = recv->rev_msg.mail_buff_len - SIZE_CMD_ID;
		if (len <= SIZE_LIMIT_PARAM) {
			memcpy(mail_buf, recv->rev_msg.mail_buff, len);
		} else {
			loge("mail buff error buf length: %u\n", len);
			ret = -EINVAL;
		}

		*mail_len = len;
		list_del(&recv->recv_node);

		kfree(recv->rev_msg.mail_buff);
		kfree(recv);
	}

	spin_unlock_bh(&g_msg_priv.recv_sync_lock);

	return ret;
}

static int cmd_para_init(struct misc_io_sync_param param,
	unsigned int *para_kernel_size_in, void **para_kernel_in)
{
	void __user *para_addr_in = NULL;
	struct dsp_chn_cmd *cmd_para = NULL;
	int ret;

	logi("para size in: %u\n", param.para_size_in);

	para_addr_in = INT_TO_ADDR(param.para_in_l, param.para_in_h);
	ret = get_input_param(param.para_size_in, para_addr_in,
		para_kernel_size_in, para_kernel_in);
	if (ret != OK) {
		loge("get input param fail: ret: %d\n", ret);
		return ret;
	}

	/* add cmd id and sn  */
	cmd_para = (struct dsp_chn_cmd *)(*para_kernel_in + *para_kernel_size_in - SIZE_CMD_ID);
	cmd_para->cmd_type = HIFI_CHN_SYNC_CMD;
	cmd_para->sn = g_msg_priv.sn;

	return OK;
}

static int sync_cmd(uintptr_t arg)
{
	int ret;
	struct misc_io_sync_param param;
	void *para_kernel_in = NULL;
	unsigned int para_kernel_size_in = 0;
	void __user *para_addr_out = NULL;
	unsigned char *mail_buf = NULL;
	unsigned int mail_len = 0;

	if (try_copy_from_user(&param, (void *)arg, sizeof(param))) {
		loge("copy from user fail\n");
		ret = ERROR;
		goto end;
	}

	para_addr_out = INT_TO_ADDR(param.para_out_l, param.para_out_h);

	ret = cmd_para_init(param, &para_kernel_size_in, &para_kernel_in);
	if (ret != OK)
		goto end;

	ret = sync_write(para_kernel_in, para_kernel_size_in);
	if (ret != OK) {
		loge("sync write ret: %d\n", ret);
		goto end;
	}

	mail_buf = kzalloc(SIZE_LIMIT_PARAM, GFP_KERNEL);
	if (!mail_buf) {
		loge("alloc mail buffer failed\n");
		ret = -ENOMEM;
		goto end;
	}

	ret = rev_msg_get(mail_buf, &mail_len);
	if (ret != OK)
		goto end;

	ret = get_output_param(mail_len, mail_buf, &param.para_size_out, para_addr_out);
	if (ret != OK) {
		loge("get output param fail, ret: %d\n", ret);
		goto end;
	}

	if (try_copy_to_user((void *)arg, &param, sizeof(param))) {
		loge("copy to_user fail\n");
		ret = COPYFAIL;
		goto end;
	}

end:
	if (mail_buf)
		kfree(mail_buf);

	input_param_free(&para_kernel_in);

	return ret;
}

static int get_phys_cmd(uintptr_t arg)
{
	int ret = OK;
	struct misc_io_get_phys_param param;
	unsigned long para_addr_in;

	if (try_copy_from_user(&param, (void *)arg, sizeof(param))) {
		loge("copy from user fail\n");
		return ERROR;
	}

	if (param.flag == 0) {
		para_addr_in = (uintptr_t)(g_msg_priv.base_phy -
			DSP_UNSEC_BASE_ADDR);
		param.phys_addr_l = GET_LOW32(para_addr_in);
		param.phys_addr_h = GET_HIG32(para_addr_in);
		logd("para addr in: 0x%pK\n", (void *)(uintptr_t)para_addr_in);
	} else {
		ret = ERROR;
		loge("invalid flag: %u\n", param.flag);
	}

	if (try_copy_to_user((void *)arg, &param,
			sizeof(struct misc_io_get_phys_param))) {
		loge("copy to user fail\n");
		ret = ERROR;
	}

	return ret;
}

static int write_param(uintptr_t arg)
{
	int ret = OK;
	void *vir_addr = NULL;
	void *para_addr_in = NULL;
	void *para_addr_out = NULL;
	struct misc_io_sync_param para;

	if (try_copy_from_user(&para, (void *)arg, sizeof(para))) {
		loge("copy from user fail\n");
		return ERROR;
	}

	para_addr_in  = INT_TO_ADDR(para.para_in_l, para.para_in_h);
	para_addr_out = INT_TO_ADDR(para.para_out_l, para.para_out_h);

	vir_addr = (unsigned char *)(g_msg_priv.base_virt +
		(DSP_AP_NV_DATA_ADDR - DSP_UNSEC_BASE_ADDR));

	logd("vir addr: 0x%pK vir data: 0x%x\n", vir_addr, (*(int *)vir_addr));
	logd("user addr: 0x%pK, size: %u\n", para_addr_in, para.para_size_in);

	if (para.para_size_in != NVPARAM_TOTAL_SIZE) {
		loge("the para size in: %u is not equal to: %u\n",
			para.para_size_in, (unsigned int)(NVPARAM_TOTAL_SIZE));
		return ERROR;
	}

	if (try_copy_from_user(vir_addr, (void __user *)para_addr_in,
			para.para_size_in)) {
		loge("copy data to dsp error\n");
		ret = ERROR;
	}

	if (para.para_size_out != sizeof(ret)) {
		loge("the para size out: %u is not equal to sizeof ret: %zu\n",
			para.para_size_out, sizeof(ret));
		return ERROR;
	}

	if (try_copy_to_user((void __user *)para_addr_out, &ret, sizeof(ret))) {
		loge("copy data to user fail\n");
		ret = ERROR;
	}

	return ret;
}

static int write_audio_effect_param(uintptr_t arg)
{
	int ret = OK;
	unsigned long copy_ret;
	void *vir_addr = NULL;
	void *para_addr_in = NULL;
	void *para_addr_out = NULL;
	struct misc_io_sync_param para;

	if (try_copy_from_user(&para, (void *)arg, sizeof(para))) {
		loge("copy from user fail\n");
		return ERROR;
	}

	if (para.para_size_in > DSP_AUDIO_EFFECT_PARAM_BUFF_SIZE) {
		loge("the para size in: %u is greater than: %u\n",
			para.para_size_in,
			(unsigned int)(DSP_AUDIO_EFFECT_PARAM_BUFF_SIZE));
		return ERROR;
	}

	if (para.para_size_out != sizeof(ret)) {
		loge("the para size out: %u is not equal to sizeof ret: %zu\n",
			para.para_size_out, sizeof(ret));
		return ERROR;
	}

	para_addr_in  = INT_TO_ADDR(para.para_in_l, para.para_in_h);
	para_addr_out = INT_TO_ADDR(para.para_out_l, para.para_out_h);
	vir_addr = (unsigned char *)(g_msg_priv.base_virt +
		(DSP_AUDIO_EFFECT_PARAM_ADDR - DSP_UNSEC_BASE_ADDR));
	logd("vir addr: 0x%pK vir data: 0x%x\n", vir_addr, (*(int *)vir_addr));
	logd("user addr: 0x%pK, size: %u\n", para_addr_in, para.para_size_in);

	copy_ret = try_copy_from_user(vir_addr,
		(void __user *)para_addr_in, para.para_size_in);
	if (copy_ret != 0) {
		loge("copy data to dsp error ret: %lu\n", copy_ret);
		ret = ERROR;
	}

	copy_ret = try_copy_to_user((void __user *)para_addr_out, &ret, sizeof(ret));
	if (copy_ret != 0) {
		loge("copy data to user fail ret: %lu\n", copy_ret);
		ret = ERROR;
	}

	return ret;
}

static int write_smartpa_param(uintptr_t arg)
{
	int ret = OK;
	unsigned long copy_ret = 0;
	void *vir_addr = NULL;
	void *para_addr_in = NULL;
	void *para_addr_out = NULL;
	struct misc_io_sync_param para;

	if (try_copy_from_user(&para, (void *)arg, sizeof(para))) {
		loge("copy from user fail\n");
		return ERROR;
	}

	if (para.para_size_in > AP_AUDIO_PA_BUFF_SIZE) {
		loge("the para size in: %u is greater than: %u\n",
		para.para_size_in, (unsigned int)(AP_AUDIO_PA_BUFF_SIZE));
		return ERROR;
	}

	if (para.para_size_out != sizeof(ret)) {
		loge("the para size out: %u is not equal to sizeof ret: %zu\n",
			para.para_size_out, sizeof(ret));
		return ERROR;
	}

	para_addr_in  = INT_TO_ADDR(para.para_in_l, para.para_in_h);
	para_addr_out = INT_TO_ADDR(para.para_out_l, para.para_out_h);
	vir_addr = (unsigned char *)(g_msg_priv.base_virt +
		(AP_AUDIO_PA_ADDR - DSP_UNSEC_BASE_ADDR));
	logd("vir addr: 0x%pK. vir data: 0x%x\n", vir_addr, (*(int *)vir_addr));
	logd("user addr: 0x%pK, size: %u\n", para_addr_in, para.para_size_in);

	copy_ret = try_copy_from_user(vir_addr,
		(void __user *)para_addr_in, para.para_size_in);
	if (copy_ret != 0) {
		loge("copy data to dsp error ret: %lu\n", copy_ret);
		ret = ERROR;
	}

	copy_ret = try_copy_to_user((void __user *)para_addr_out, &ret, sizeof(ret));
	if (copy_ret) {
		loge("copy data to user fail ret: %lu\n", copy_ret);
		ret = ERROR;
	}

	return ret;
}
static int usbaudio_cmd(uintptr_t arg)
{
	void *para_addr_in = NULL;
	void *para_addr_out = NULL;
	struct usbaudio_ioctl_input input;
	struct usbaudio_info output;
	struct misc_io_sync_param para;

	memset(&input, 0, sizeof(input));
	memset(&output, 0, sizeof(output));
	memset(&para, 0, sizeof(para));

	if (try_copy_from_user(&para, (void *)arg, sizeof(para))) {
		loge("copy from user fail\n");
		return ERROR;
	}

	if (para.para_size_out != sizeof(output)) {
		loge("wrong ioctl output size: %u should be %zu\n",
			para.para_size_out, sizeof(output));
		return ERROR;
	}

	para_addr_in  = INT_TO_ADDR(para.para_in_l, para.para_in_h);
	para_addr_out = INT_TO_ADDR(para.para_out_l, para.para_out_h);

	if (para.para_size_in != sizeof(input)) {
		loge("wrong ioctl input size.size: %u should be %zu\n",
			para.para_size_in, sizeof(input));
		return ERROR;
	}

	if (try_copy_from_user(&input, (void __user *)para_addr_in,
			para.para_size_in)) {
		loge("copy ioctl input data to kernel error\n");
		return ERROR;
	}

	if (input.msg_type == (unsigned int)USBAUDIO_QUERY_INFO) {
		/* query controller state */
		usbaudio_ctrl_query_info(&output);
	} else if (input.msg_type == (unsigned int)USBAUDIO_USB_POWER_RESUME) {
		output.sr_status = usbaudio_ctrl_usb_resume();
	} else if (input.msg_type == (unsigned int)USBAUDIO_NV_ISREADY) {
		usbaudio_set_nv_ready();
	} else {
		/* do nothing */
		loge("message type error %x\n", input.msg_type);
	}

	if (try_copy_to_user((void __user *)para_addr_out, &output, sizeof(output))) {
		loge("copy data to user fail\n");
		return ERROR;
	}

	return 0;
}

static int cmd_switch(const struct soundtrigger_sync_msg *input, int *output)
{
	switch (input->msg_type) {
	case WAKEUP_CHN_MSG_START:
		*output = start_recognition_msg(input->msg_type, input->module_id, false);
		break;
	case LP_WAKEUP_CHN_MSG_START:
		*output = start_recognition_msg(input->msg_type, input->module_id, true);
		break;
	case WAKEUP_CHN_MSG_PARAMETER_SET:
	case LP_WAKEUP_CHN_MSG_PARAMETER_SET:
		*output = parameter_set_msg(input->msg_type,
			input->module_id, &input->set_param);
		break;
	case WAKEUP_CHN_MSG_PARAMETER_GET:
	case LP_WAKEUP_CHN_MSG_PARAMETER_GET:
		get_handle_msg(input->msg_type, output);
		break;
	case WAKEUP_CHN_MSG_STOP:
		*output = stop_recognition_msg(input->msg_type, input->module_id, false);
		break;
	case LP_WAKEUP_CHN_MSG_STOP:
		*output = stop_recognition_msg(input->msg_type, input->module_id, true);
		break;
	default:
		/* do nothing */
		loge("message type error%x\n", input->msg_type);
		return ERROR;
	}

	return OK;
}

static int soundtrigger_cmd(uintptr_t arg)
{
	int ret;
	void *para_addr_in = NULL;
	void *para_addr_out = NULL;
	struct soundtrigger_sync_msg *input = NULL;
	int output = 0;
	struct misc_io_sync_param para;

	memset(&para, 0, sizeof(para));

	if (try_copy_from_user(&para, (void *)arg, sizeof(para))) {
		loge("copy from user fail\n");
		return ERROR;
	}

	if (para.para_size_out != sizeof(output)) {
		loge("wrong ioctl output size.size: %u should be: %zu\n",
			para.para_size_out, sizeof(output));
		return ERROR;
	}

	para_addr_in  = INT_TO_ADDR(para.para_in_l, para.para_in_h);
	para_addr_out = INT_TO_ADDR(para.para_out_l, para.para_out_h);

	if (para.para_size_in < sizeof(*input) ||
		para.para_size_in > (sizeof(*input) + AP_AUDIO_WAKEUP_MODEL_SIZE)) {
		loge("wrong ioctl input size.size: %u\n", para.para_size_in);
		return ERROR;
	}

	input = kzalloc(para.para_size_in, GFP_ATOMIC);
	if (!input) {
		loge("kzalloc fail\n");
		return ERROR;
	}

	if (try_copy_from_user((void *)input,
		(void __user *)para_addr_in, para.para_size_in)) {
		loge("copy ioctl input data to kernel error\n");
		kfree(input);
		return ERROR;
	}

	ret = cmd_switch(input, &output);

	if (try_copy_to_user((void __user *)para_addr_out, &output, sizeof(output))) {
		loge("copy data to user fail\n");
		ret = ERROR;
	}

	kfree(input);
	return ret;
}

#ifdef CONFIG_DP_MULTI_CHANNELS
static int get_dp_info(struct dp_edid_info *edid_info)
{
	int ret;

	if (!edid_info) {
		loge("edid info is null\n");
		return ERROR;
	}

	ret = hisi_dptx_get_spec(edid_info->spec, sizeof(edid_info->spec),
		&edid_info->ext_acount);
	if (ret) {
		loge("get spec failed ret: %d\n", ret);
		return ERROR;
	}

	logi("get spec success ext acount: %u\n", edid_info->ext_acount);

	return ret;
}

static int set_dp_info(const struct dp_edid_info *edid_info)
{
	int ret;

	if (!edid_info) {
		loge("edid info is null\n");
		return ERROR;
	}

	logi("set aparam channel num: %u data width: %u sample rate: %u\n",
		edid_info->aparam.channel_num, edid_info->aparam.data_width,
		edid_info->aparam.sample_rate);

	ret = hisi_dptx_set_aparam(edid_info->aparam.channel_num,
		edid_info->aparam.data_width, edid_info->aparam.sample_rate);
	if (ret) {
		loge("set aparam failed ret: %d\n", ret);
		return ERROR;
	}

	return ret;
}

static int get_dp_audio_cmd(uintptr_t arg)
{
	int ret;
	struct dp_edid_info info;

	memset(&info, 0, sizeof(info));

	ret = get_dp_info(&info);
	if (ret) {
		loge("get dpinfo failed ret: %d\n", ret);
		return ret;
	}

	if (try_copy_to_user((void __user *)arg, &info, sizeof(info))) {
		loge("copy data to user failed\n");
		return ERROR;
	}

	return OK;
}

static int set_dp_audio_cmd(uintptr_t arg)
{
	struct dp_edid_info info;

	memset(&info, 0, sizeof(info));

	if (try_copy_from_user(&info, (void __user *)arg, sizeof(info))) {
		loge("copy data to kernel fail\n");
		return ERROR;
	}

	return set_dp_info(&info);
}
#endif


#define msg_func(id, func) { id, func, #id }

static const struct dsp_ioctl_cmd cmd_func[] = {
	msg_func(HIFI_MISC_IOCTL_ASYNCMSG, async_cmd),
	msg_func(HIFI_MISC_IOCTL_GET_PHYS, get_phys_cmd),
	msg_func(HIFI_MISC_IOCTL_WRITE_PARAMS, write_param),
	msg_func(HIFI_MISC_IOCTL_AUDIO_EFFECT_PARAMS, write_audio_effect_param),
	msg_func(HIFI_MISC_IOCTL_DUMP_HIFI, socdsp_dump),
	msg_func(HIFI_MISC_IOCTL_GET_VOICE_BSD_PARAM, socdsp_om_get_voice_bsd_param),
	msg_func(HIFI_MISC_IOCTL_WAKEUP_THREAD, soc_dsp_wakeup_read_thread),
	msg_func(HIFI_MISC_IOCTL_WAKEUP_PCM_READ_THREAD, soc_dsp_wakeup_pcm_read_thread),
	msg_func(HIFI_MISC_IOCTL_WAKEUP_PARA_WRITE_THREAD, soc_dsp_wakeup_para_write_thread),
	msg_func(HIFI_MISC_IOCTL_USBAUDIO, usbaudio_cmd),
	msg_func(HIFI_MISC_IOCTL_SMARTPA_PARAMS, write_smartpa_param),
	msg_func(HIFI_MISC_IOCTL_SOUNDTRIGGER, soundtrigger_cmd),
#ifdef CONFIG_DP_MULTI_CHANNELS
	msg_func(HIFI_MISC_IOCTL_GET_DPAUDIO, get_dp_audio_cmd),
	msg_func(HIFI_MISC_IOCTL_SET_DPAUDIO, set_dp_audio_cmd),
#endif
};

long dsp_msg_process_cmd(unsigned int cmd, uintptr_t data32)
{
	long ret = ERROR;
	unsigned int i;

	if (ARRAY_SIZE(cmd_func) == 0) {
		loge("cmd proc is not register\n");
		return ret;
	}

	for (i = 0; i < ARRAY_SIZE(cmd_func); i++) {
		if (cmd == cmd_func[i].id) {
			logi("ioctl: %s\n", cmd_func[i].name);
			ret = cmd_func[i].func(data32);

			return ret;
		}
	}

	return ret;
}

void dsp_msg_init(struct dsp_misc_proc *misc_proc, unsigned char *base_virt,
	unsigned char *base_phy)
{
	memset(&g_msg_priv, 0, sizeof(g_msg_priv));

	spin_lock_init(&g_msg_priv.recv_sync_lock);
	init_completion(&g_msg_priv.completion);

	g_msg_priv.sn = 0;
	g_msg_priv.base_virt = base_virt;
	g_msg_priv.base_phy = base_phy;

	misc_proc->sync_msg_proc = sync_cmd;
}

void dsp_msg_deinit(void)
{
	logi("msg deinit\n");
}

