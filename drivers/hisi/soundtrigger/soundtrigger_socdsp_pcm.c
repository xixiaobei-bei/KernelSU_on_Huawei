/*
 * soundtrigger_socdsp_pcm.c sound trigger socdsp pcm driver.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "soundtrigger_socdsp_pcm.h"
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/errno.h>
#include "hifi_lpp.h"
#include "mlib_ringbuffer.h"
#include "../../sound/soc/hisilicon/hisi_pcm_hifi.h"

#define FAST_BUFFER_SIZE  (HISI_AP_AUDIO_WAKEUP_RINGBUFEER_SIZE)
#define FAST_SOCDSP_UPLOAD_SIZE  61440 //16K*1ch*2byte*1920ms
#define RINGBUF_FRAME_LEN           (640)
#define RINGBUF_FRAME_COUNT (100)
#define RINGBUF_HEAD_SIZE  (20)

static unsigned int g_sochifi_wakeup_upload_chn_num = ONEMIC_CHN;

enum fast_transfer_state {
	FAST_TRANS_NOT_COMPLETE = 0,
	FAST_TRANS_COMPLETE,
};

enum fast_read_state {
	FAST_READ_NOT_COMPLETE = 0,
	FAST_READ_COMPLETE,
};

struct soundtrigger_socdsp_pcm {
	RingBuffer *ring_buffer;
	char *fast_buffer;
	char *elapsed_buffer;
	int fast_complete_flag; /* fast tansmit complete flag */
	unsigned int fast_len;
	int fast_read_complete_flag; /* flag to decide whether HAL read cpmlete */
};
struct soundtrigger_socdsp_pcm g_socdsp_pcm[WAKEUP_MODE_BUTT];

static int soundtrtgger_socdsp_pcm_buffer_init(unsigned int wakeup_mode)
{
	struct soundtrigger_socdsp_pcm *socdsp_pcm = &g_socdsp_pcm[wakeup_mode];
	unsigned int fast_buf;
	unsigned int fast_size;
	unsigned int elapsed_buf;
	unsigned int elapsed_size;
	unsigned int rb_frame_len = RINGBUF_FRAME_LEN * g_sochifi_wakeup_upload_chn_num;

	if (wakeup_mode == NORMAL_WAKEUP_MODE) {
		fast_buf = HISI_AP_AUDIO_WAKEUP_RINGBUFFER_ADDR;
		fast_size = HISI_AP_AUDIO_WAKEUP_RINGBUFEER_SIZE;
		elapsed_buf = HISI_AP_AUDIO_WAKEUP_CAPTURE_ADDR;
		elapsed_size = HISI_AP_AUDIO_WAKEUP_CAPTURE_SIZE;
	} else {
		fast_buf = HISI_AP_AUDIO_LP_WAKEUP_RINGBUFFER_ADDR;
		fast_size = HISI_AP_AUDIO_LP_WAKEUP_RINGBUFEER_SIZE;
		elapsed_buf = HISI_AP_AUDIO_LP_WAKEUP_CAPTURE_ADDR;
		elapsed_size = HISI_AP_AUDIO_LP_WAKEUP_CAPTURE_SIZE;
	}

	socdsp_pcm->ring_buffer = kzalloc(rb_frame_len * RINGBUF_FRAME_COUNT + RINGBUF_HEAD_SIZE, GFP_ATOMIC);
	if (socdsp_pcm->ring_buffer == NULL) {
		pr_err("%s ring_buffer malloc fail\n", __FUNCTION__);
		return -ENOMEM;
	}

	socdsp_pcm->fast_buffer = ioremap_wc(fast_buf, fast_size);
	if (socdsp_pcm->fast_buffer == NULL) {
		pr_err("%s fast buffer ioremap fail\n", __FUNCTION__);
		kfree(socdsp_pcm->ring_buffer);
		socdsp_pcm->ring_buffer = NULL;
		return -EFAULT;
	}

	socdsp_pcm->elapsed_buffer = ioremap_wc(elapsed_buf, elapsed_size);
	if (socdsp_pcm->elapsed_buffer == NULL) {
		pr_err("%s elapsed_buffer ioremap fail\n", __FUNCTION__);
		iounmap(socdsp_pcm->fast_buffer);
		socdsp_pcm->fast_buffer = NULL;
		kfree(socdsp_pcm->ring_buffer);
		socdsp_pcm->ring_buffer = NULL;
		return -EFAULT;
	}

	RingBuffer_Init(socdsp_pcm->ring_buffer, rb_frame_len, RINGBUF_FRAME_COUNT);

	return 0;
}

static void soundtrtgger_socdsp_pcm_buffer_deinit(unsigned int wakeup_mode)
{
	struct soundtrigger_socdsp_pcm *socdsp_pcm = &g_socdsp_pcm[wakeup_mode];

	if (socdsp_pcm->ring_buffer != NULL) {
		kfree(socdsp_pcm->ring_buffer);
		socdsp_pcm->ring_buffer = NULL;
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

static int soundtrigger_socdsp_pcm_get_element(struct soundtrigger_socdsp_pcm *socdsp_pcm, char *element)
{
	int ret = 0;

	unsigned int rb_frame_len = RINGBUF_FRAME_LEN * g_sochifi_wakeup_upload_chn_num;

	if (socdsp_pcm->ring_buffer == NULL)
		return -EFAULT;

	if (!RingBuffer_IsEmpty(socdsp_pcm->ring_buffer)) {
		ret = RingBuffer_Get(socdsp_pcm->ring_buffer, element);
		if (ret != rb_frame_len)
			return -EFAULT;
	}

	return ret;
}

static ssize_t soundtrigger_socdsp_pcm_read(struct soundtrigger_socdsp_pcm *socdsp_pcm,
						char __user *buf, size_t count)
{
	int rest_bytes = 0;
	int read_bytes = 0;
	size_t max_read_len = 0;

	unsigned int fast_upload_size = FAST_SOCDSP_UPLOAD_SIZE * g_sochifi_wakeup_upload_chn_num;
	unsigned int rb_frame_len = RINGBUF_FRAME_LEN * g_sochifi_wakeup_upload_chn_num;
	if (rb_frame_len == 0) {
		pr_err("%s rb_frame_len error", __FUNCTION__);
		return -EFAULT;
	}

	if (buf == NULL || socdsp_pcm->fast_buffer == NULL) {
		pr_err("%s buf is null", __FUNCTION__);
		return -EFAULT;
	}

	if (count < rb_frame_len) {
		pr_err("%s count is err %zu", __FUNCTION__, count);
		return -EFAULT;
	}

	max_read_len = count >= fast_upload_size ? fast_upload_size : count;
	if (socdsp_pcm->fast_read_complete_flag == FAST_READ_NOT_COMPLETE) {
		if (socdsp_pcm->fast_complete_flag == FAST_TRANS_COMPLETE) {
			if (max_read_len > socdsp_pcm->fast_len) {
				max_read_len = socdsp_pcm->fast_len;
				pr_info("fastlen less than 64K %u", socdsp_pcm->fast_len);
			}
			rest_bytes = copy_to_user(buf, socdsp_pcm->fast_buffer, max_read_len);
			if (rest_bytes) {
				pr_err("copy to user fail\n");
				return -EFAULT;
			}
			socdsp_pcm->fast_read_complete_flag = FAST_READ_COMPLETE;
			return max_read_len;
		} else {
			return -EFAULT;
		}
	} else {
		char *element = (char *)kzalloc(rb_frame_len, GFP_ATOMIC);
		if (element == NULL) {
			pr_err("%s element malloc failed", __FUNCTION__);
			return -EFAULT;
		}

		read_bytes = soundtrigger_socdsp_pcm_get_element(socdsp_pcm, element);
		if (read_bytes > 0) {
			rest_bytes = copy_to_user(buf, element, read_bytes);
			if (rest_bytes) {
				pr_err("copy to user fail\n");
				kfree(element);
				return -EFAULT;
			}
		} else {
			kfree(element);
			return -EFAULT;
		}
		kfree(element);
		return read_bytes;
	}
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

void soundtrigger_socdsp_pcm_flag_init(unsigned int wakeup_mode)
{
	struct soundtrigger_socdsp_pcm *socdsp_pcm = &g_socdsp_pcm[wakeup_mode];

	socdsp_pcm->fast_complete_flag = FAST_TRANS_NOT_COMPLETE;
	socdsp_pcm->fast_len = 0;
	socdsp_pcm->fast_read_complete_flag = FAST_READ_NOT_COMPLETE;
}

int soundtrigger_socdsp_pcm_fastbuffer_filled(unsigned int wakeup_mode, unsigned int fast_len)
{
	struct soundtrigger_socdsp_pcm *socdsp_pcm = &g_socdsp_pcm[wakeup_mode];

	if (socdsp_pcm->fast_buffer == NULL)
		return -ENOMEM;
	socdsp_pcm->fast_len = fast_len;
	socdsp_pcm->fast_complete_flag = FAST_TRANS_COMPLETE;
	return 0;
}

int soundtrigger_socdsp_pcm_elapsed(unsigned int wakeup_mode, unsigned int start, int buffer_len)
{
	unsigned int rb_frame_len;

	struct soundtrigger_socdsp_pcm *socdsp_pcm = &g_socdsp_pcm[wakeup_mode];

	rb_frame_len = RINGBUF_FRAME_LEN * g_sochifi_wakeup_upload_chn_num;

	if ((buffer_len != rb_frame_len) || (start > rb_frame_len))
		return -EFAULT;

	if (socdsp_pcm->fast_buffer == NULL ||
		socdsp_pcm->elapsed_buffer == NULL ||
		socdsp_pcm->ring_buffer == NULL)
		return -ENOMEM;

	RingBuffer_Put(socdsp_pcm->ring_buffer, (void*)(socdsp_pcm->elapsed_buffer + start));

	return 0;
}

int soundtrigger_socdsp_pcm_init(unsigned int sochifi_wakeup_upload_chn_num)
{
	int ret = 0;
	g_sochifi_wakeup_upload_chn_num = sochifi_wakeup_upload_chn_num;
	if (g_sochifi_wakeup_upload_chn_num > MAX_MICNUM_CHN ||
		g_sochifi_wakeup_upload_chn_num < ONEMIC_CHN) {
		g_sochifi_wakeup_upload_chn_num = ONEMIC_CHN;
		pr_err("%s g_sochifi_wakeup_upload_chn_num read error\n", __FUNCTION__);
	}

	ret = misc_register(&soundtrigger_socdsp_pcm_drv_device);
	if (ret) {
		pr_err("%s misc_register fail\n", __FUNCTION__);
		return -EFAULT;
	}

	ret = misc_register(&soundtrigger_socdsp_lp_pcm_drv_device);
	if (ret) {
		pr_err("%s lp_misc_register fail\n", __FUNCTION__);
		misc_deregister(&soundtrigger_socdsp_pcm_drv_device);
		return -EFAULT;
	}

	ret = soundtrtgger_socdsp_pcm_buffer_init(NORMAL_WAKEUP_MODE);
	if (ret) {
		pr_err("%s pcm_buffer_init fail\n", __FUNCTION__);
		misc_deregister(&soundtrigger_socdsp_pcm_drv_device);
		misc_deregister(&soundtrigger_socdsp_lp_pcm_drv_device);
		return ret;
	}

	ret = soundtrtgger_socdsp_pcm_buffer_init(LP_WAKEUP_MODE);
	if (ret) {
		pr_err("%s lp_pcm_buffer_init fail\n", __FUNCTION__);
		misc_deregister(&soundtrigger_socdsp_pcm_drv_device);
		misc_deregister(&soundtrigger_socdsp_lp_pcm_drv_device);
		soundtrtgger_socdsp_pcm_buffer_deinit(NORMAL_WAKEUP_MODE);
		return ret;
	}

	return 0;
}

int soundtrigger_socdsp_pcm_deinit(void)
{
	soundtrtgger_socdsp_pcm_buffer_deinit(NORMAL_WAKEUP_MODE);
	soundtrtgger_socdsp_pcm_buffer_deinit(LP_WAKEUP_MODE);

	misc_deregister(&soundtrigger_socdsp_pcm_drv_device);
	misc_deregister(&soundtrigger_socdsp_lp_pcm_drv_device);

	return 0;
}


