/*
 * soundtrigger_socdsp_pcm.c
 *
 * soundtrigger pcm buffer driver
 *
 * Copyright (c) 2014-2020 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/hisi/audio_log.h>

#include "dsp_misc.h"
#include "soundtrigger_ring_buffer.h"
#include "soundtrigger_socdsp_pcm.h"
#include "soundtrigger_socdsp_mailbox.h"
#include "audio_pcm_hifi.h"

#define LOG_TAG "soundtrigger"

#define FAST_BUFFER_SIZE (AP_AUDIO_WAKEUP_RINGBUFEER_SIZE)
#define FAST_SOCDSP_UPLOAD_SIZE 61440 /* 16K*1ch*2byte*1920ms */
#define RINGBUF_FRAME_LEN 640
#define RINGBUF_FRAME_COUNT 100

enum fast_transfer_state {
	FAST_TRANS_NOT_COMPLETE = 0,
	FAST_TRANS_COMPLETE,
};

enum fast_read_state {
	FAST_READ_NOT_COMPLETE = 0,
	FAST_READ_COMPLETE,
};

struct soundtrigger_socdsp_pcm {
	struct ring_buffer_itef *rb;
	char *fast_buffer;
	char *elapsed_buffer;
	int fast_complete_flag;
	uint32_t fast_len;
	int fast_read_complete_flag; /* flag to decide whether HAL read cpmlete */
};

static uint32_t g_sochifi_wakeup_upload_chn_num = ONEMIC_CHN;
static struct soundtrigger_socdsp_pcm g_socdsp_pcm[WAKEUP_MODE_BUTT];

static int soundtrtgger_socdsp_pcm_buffer_init(uint32_t wakeup_mode)
{
	int ret;
	uint32_t fast_buf, fast_size, elapsed_buf, elapsed_size;
	uint32_t rb_frame_len = RINGBUF_FRAME_LEN * g_sochifi_wakeup_upload_chn_num;
	struct soundtrigger_socdsp_pcm *socdsp_pcm = &g_socdsp_pcm[wakeup_mode];

	if (wakeup_mode == NORMAL_WAKEUP_MODE) {
		fast_buf = AP_AUDIO_WAKEUP_RINGBUFFER_ADDR;
		fast_size = AP_AUDIO_WAKEUP_RINGBUFEER_SIZE;
		elapsed_buf = AP_AUDIO_WAKEUP_CAPTURE_ADDR;
		elapsed_size = AP_AUDIO_WAKEUP_CAPTURE_SIZE;
	} else {
		fast_buf = AP_AUDIO_LP_WAKEUP_RINGBUFFER_ADDR;
		fast_size = AP_AUDIO_LP_WAKEUP_RINGBUFEER_SIZE;
		elapsed_buf = AP_AUDIO_LP_WAKEUP_CAPTURE_ADDR;
		elapsed_size = AP_AUDIO_LP_WAKEUP_CAPTURE_SIZE;
	}

	socdsp_pcm->rb = st_ring_buffer_init(rb_frame_len, RINGBUF_FRAME_COUNT);
	if (socdsp_pcm->rb == NULL) {
		AUDIO_LOGE("ring buffer init fail");
		return -ENOMEM;
	}

	socdsp_pcm->fast_buffer = ioremap_wc(fast_buf, fast_size);
	if (socdsp_pcm->fast_buffer == NULL) {
		AUDIO_LOGE("fast buffer ioremap fail");
		ret = -EFAULT;
		goto err_fast_buf_remap;
	}

	socdsp_pcm->elapsed_buffer = ioremap_wc(elapsed_buf, elapsed_size);
	if (socdsp_pcm->elapsed_buffer == NULL) {
		AUDIO_LOGE("elapsed buffer ioremap fail");
		ret = -EFAULT;
		goto err_elapsed_buf_remap;
	}

	return 0;

err_elapsed_buf_remap:
	iounmap(socdsp_pcm->fast_buffer);
	socdsp_pcm->fast_buffer = NULL;
err_fast_buf_remap:
	socdsp_pcm->rb->deinit(socdsp_pcm->rb);
	socdsp_pcm->rb = NULL;

	return ret;
}

static void soundtrtgger_socdsp_pcm_buffer_deinit(uint32_t wakeup_mode)
{
	struct soundtrigger_socdsp_pcm *socdsp_pcm = &g_socdsp_pcm[wakeup_mode];

	if (socdsp_pcm->rb != NULL) {
		socdsp_pcm->rb->deinit(socdsp_pcm->rb);
		socdsp_pcm->rb = NULL;
	}

	if (socdsp_pcm->fast_buffer != NULL) {
		iounmap(socdsp_pcm->fast_buffer);
		socdsp_pcm->fast_buffer = NULL;
	}

	if (socdsp_pcm->elapsed_buffer != NULL) {
		iounmap(socdsp_pcm->elapsed_buffer);
		socdsp_pcm->elapsed_buffer = NULL;
	}
}

static int soundtrigger_socdsp_pcm_get_element(struct soundtrigger_socdsp_pcm *socdsp_pcm, char *element, uint32_t size)
{
	int ret = 0;

	uint32_t rb_frame_len = RINGBUF_FRAME_LEN * g_sochifi_wakeup_upload_chn_num;

	if (socdsp_pcm->rb == NULL)
		return -EFAULT;

	if (!socdsp_pcm->rb->empty(socdsp_pcm->rb)) {
		ret = socdsp_pcm->rb->get(socdsp_pcm->rb, element, size);
		if (ret != rb_frame_len)
			return -EFAULT;
	}

	return ret;
}

static int check_read_para(const struct soundtrigger_socdsp_pcm *socdsp_pcm,
	const char __user *buf, size_t count, size_t rb_frame_len)
{
	if (buf == NULL) {
		AUDIO_LOGE("buf is null");
		return -EFAULT;
	}

	if (socdsp_pcm->fast_buffer == NULL) {
		AUDIO_LOGE("fast buffer is null");
		return -EFAULT;
	}

	if (rb_frame_len == 0) {
		AUDIO_LOGE("rb_frame_len is 0");
		return -EFAULT;
	}

	if (count < rb_frame_len) {
		AUDIO_LOGE("count:%zu < rb frame len:%zu", count, rb_frame_len);
		return -EFAULT;
	}

	return 0;
}

static ssize_t soundtrigger_socdsp_pcm_read(struct soundtrigger_socdsp_pcm *socdsp_pcm,
	char __user *buf, size_t count)
{
	int ret, rest_bytes, read_bytes;
	size_t max_read_len;
	char *element = NULL;
	size_t fast_upload_size = FAST_SOCDSP_UPLOAD_SIZE * g_sochifi_wakeup_upload_chn_num;
	size_t rb_frame_len = RINGBUF_FRAME_LEN * g_sochifi_wakeup_upload_chn_num;

	ret = check_read_para(socdsp_pcm, buf, count, rb_frame_len);
	if (ret != 0) {
		return ret;
	}

	max_read_len = count >= fast_upload_size ? fast_upload_size : count;
	if (socdsp_pcm->fast_read_complete_flag == FAST_READ_NOT_COMPLETE) {
		if (socdsp_pcm->fast_complete_flag != FAST_TRANS_COMPLETE) {
			return -EFAULT;
		}

		if (max_read_len > socdsp_pcm->fast_len) {
			max_read_len = socdsp_pcm->fast_len;
			AUDIO_LOGI("fastlen less than 64K %u", socdsp_pcm->fast_len);
		}

		rest_bytes = copy_to_user(buf, socdsp_pcm->fast_buffer, max_read_len);
		if (rest_bytes) {
			AUDIO_LOGE("copy to user fail");
			return -EFAULT;
		}

		socdsp_pcm->fast_read_complete_flag = FAST_READ_COMPLETE;
		return max_read_len;
	}

	element = kzalloc(rb_frame_len, GFP_ATOMIC);
	if (element == NULL) {
		AUDIO_LOGE("element malloc failed");
		return -EFAULT;
	}

	read_bytes = soundtrigger_socdsp_pcm_get_element(socdsp_pcm, element, rb_frame_len);
	if (read_bytes <= 0) {
		kfree(element);
		return -EFAULT;
	}

	rest_bytes = copy_to_user(buf, element, read_bytes);
	if (rest_bytes) {
		AUDIO_LOGE("copy to user fail");
		kfree(element);
		return -EFAULT;
	}

	kfree(element);
	return read_bytes;
}

static ssize_t soundtrigger_socdsp_lp_read(struct file *file, char __user *buf,
	size_t count, loff_t *ppos)
{
	struct soundtrigger_socdsp_pcm *socdsp_pcm = &g_socdsp_pcm[LP_WAKEUP_MODE];
	ssize_t ret;

	ret = soundtrigger_socdsp_pcm_read(socdsp_pcm, buf, count);

	return ret;
}

static ssize_t soundtrigger_socdsp_read(struct file *file, char __user *buf,
	size_t count, loff_t *ppos)
{
	struct soundtrigger_socdsp_pcm *socdsp_pcm = &g_socdsp_pcm[NORMAL_WAKEUP_MODE];
	ssize_t ret;

	ret = soundtrigger_socdsp_pcm_read(socdsp_pcm, buf, count);

	return ret;
}

static const struct file_operations soundtrigger_socdsp_pcm_read_fops = {
	.owner = THIS_MODULE,
	.read = soundtrigger_socdsp_read,
};

static struct miscdevice soundtrigger_socdsp_pcm_drv_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "soundtrigger_socdsp_pcm_drv",
	.fops = &soundtrigger_socdsp_pcm_read_fops,
};

static const struct file_operations soundtrigger_socdsp_lp_pcm_read_fops = {
	.owner = THIS_MODULE,
	.read = soundtrigger_socdsp_lp_read,
};

static struct miscdevice soundtrigger_socdsp_lp_pcm_drv_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "soundtrigger_socdsp_lp_pcm_drv",
	.fops = &soundtrigger_socdsp_lp_pcm_read_fops,
};

void soundtrigger_socdsp_pcm_flag_init(uint32_t wakeup_mode)
{
	struct soundtrigger_socdsp_pcm *socdsp_pcm = NULL;

	if (wakeup_mode >= WAKEUP_MODE_BUTT) {
		AUDIO_LOGE("invalid wake mode: %u", wakeup_mode);
		return;
	}

	socdsp_pcm = &g_socdsp_pcm[wakeup_mode];

	socdsp_pcm->fast_complete_flag = FAST_TRANS_NOT_COMPLETE;
	socdsp_pcm->fast_len = 0;
	socdsp_pcm->fast_read_complete_flag = FAST_READ_NOT_COMPLETE;
}

int soundtrigger_socdsp_pcm_fastbuffer_filled(uint32_t wakeup_mode, uint32_t fast_len)
{
	struct soundtrigger_socdsp_pcm *socdsp_pcm = NULL;

	if (wakeup_mode >= WAKEUP_MODE_BUTT) {
		AUDIO_LOGE("invalid wake mode: %u", wakeup_mode);
		return -EINVAL;
	}

	socdsp_pcm = &g_socdsp_pcm[wakeup_mode];

	if (socdsp_pcm->fast_buffer == NULL)
		return -ENOMEM;

	socdsp_pcm->fast_len = fast_len;
	socdsp_pcm->fast_complete_flag = FAST_TRANS_COMPLETE;

	return 0;
}

int soundtrigger_socdsp_pcm_elapsed(uint32_t wakeup_mode, uint32_t start, int buffer_len)
{
	uint32_t rb_frame_len;
	struct soundtrigger_socdsp_pcm *socdsp_pcm = NULL;

	if (wakeup_mode >= WAKEUP_MODE_BUTT) {
		AUDIO_LOGE("invalid wake mode: %u", wakeup_mode);
		return -EINVAL;
	}

	socdsp_pcm = &g_socdsp_pcm[wakeup_mode];
	if (socdsp_pcm->fast_buffer == NULL ||
		socdsp_pcm->elapsed_buffer == NULL ||
		socdsp_pcm->rb == NULL)
		return -EFAULT;

	rb_frame_len = RINGBUF_FRAME_LEN * g_sochifi_wakeup_upload_chn_num;

	if ((buffer_len != rb_frame_len) || (start > rb_frame_len))
		return -EFAULT;

	socdsp_pcm->rb->put(socdsp_pcm->rb, (void*)(socdsp_pcm->elapsed_buffer + start), rb_frame_len);

	return 0;
}

int soundtrigger_socdsp_pcm_init(uint32_t channel_num)
{
	int ret;

	if (channel_num > MAX_MICNUM_CHN || channel_num < ONEMIC_CHN) {
		g_sochifi_wakeup_upload_chn_num = ONEMIC_CHN;
		AUDIO_LOGE("channel_num: %u error, set default: %u",
			channel_num, g_sochifi_wakeup_upload_chn_num);
	} else {
		g_sochifi_wakeup_upload_chn_num = channel_num;
	}

	ret = misc_register(&soundtrigger_socdsp_pcm_drv_device);
	if (ret) {
		AUDIO_LOGE("misc register fail, ret: %d", ret);
		return -EFAULT;
	}

	ret = misc_register(&soundtrigger_socdsp_lp_pcm_drv_device);
	if (ret) {
		AUDIO_LOGE("lp misc register fail, ret: %d", ret);
		goto err_lp_misc_register;
	}

	ret = soundtrtgger_socdsp_pcm_buffer_init(NORMAL_WAKEUP_MODE);
	if (ret) {
		AUDIO_LOGE("pcm buffer init fail, ret: %d", ret);
		goto err_normal_pcm_buf_init;
	}

	ret = soundtrtgger_socdsp_pcm_buffer_init(LP_WAKEUP_MODE);
	if (ret) {
		AUDIO_LOGE("lp pcm buffer init fail, ret: %d", ret);
		goto err_lp_pcm_buf_init;
	}

	return 0;

err_lp_pcm_buf_init:
	soundtrtgger_socdsp_pcm_buffer_deinit(NORMAL_WAKEUP_MODE);
err_normal_pcm_buf_init:
	misc_deregister(&soundtrigger_socdsp_lp_pcm_drv_device);
err_lp_misc_register:
	misc_deregister(&soundtrigger_socdsp_pcm_drv_device);

	return ret;
}

int soundtrigger_socdsp_pcm_deinit(void)
{
	soundtrtgger_socdsp_pcm_buffer_deinit(NORMAL_WAKEUP_MODE);
	soundtrtgger_socdsp_pcm_buffer_deinit(LP_WAKEUP_MODE);

	misc_deregister(&soundtrigger_socdsp_pcm_drv_device);
	misc_deregister(&soundtrigger_socdsp_lp_pcm_drv_device);

	return 0;
}

