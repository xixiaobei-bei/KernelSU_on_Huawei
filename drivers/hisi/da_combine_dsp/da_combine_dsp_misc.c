/*
 * da_combine_dsp_msic.c
 *
 * misc driver for da_combine codecdsp
 *
 * Copyright (c) 2015 Huawei Technologies Co., Ltd.
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

#include <linux/hisi/da_combine_dsp/da_combine_dsp_misc.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/suspend.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/hisi/rdr_pub.h>

#include <linux/hisi/da_combine/da_combine_dsp_regs.h>
#include <linux/hisi/da_combine/da_combine_vad.h>
#include <linux/hisi/da_combine/da_combine_utils.h>
#include <linux/hisi/audio_log.h>

#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif
#include "rdr_audio_codec.h"
#include "rdr_audio_adapter.h"
#include "da_combine_algo_interface.h"
#include "da_combine_dsp_interface.h"
#include "soundtrigger_dma_drv.h"
#include "download_image.h"
#include "audio_file.h"
#include "dsp_utils.h"
#include "slimbus.h"
#include "om_hook.h"
#include "om_beta.h"
#include "om.h"

#define LOG_TAG "DA_combine_msic"

#define DA_COMBINE_EXCEPTION_RETRY 3
#define DA_COMBINE_GET_STATE_MUTEX_RETRY 20
#define DA_COMBINE_IFOPEN_WAIT4DPCLK 1000
#define DA_COMBINE_CMD_VLD_BITS 0xf
#define DA_COMBINE_DSP_TO_AP_MSG_ACTIVE 0x5a5a5a5a
#define DA_COMBINE_DSP_TO_AP_MSG_DEACTIVE 0xa5a5a5a5
#define MAX_OUT_PARA_SIZE ((RESULT_SIZE) + (MAX_PARA_SIZE))
#define MAX_USR_INPUT_SIZE ((MAX_MSG_SIZE) + (MAX_PARA_SIZE))
#define MAX_MSG_SIZE 128
#define MAX_PARA_SIZE 4096
#define WAKEUP_SCENE_DSP_CLOSE 1
#define WAKEUP_SCENE_PLL_CLOSE 2
#define WAKEUP_SCENE_PLL_OPEN 3
#define INTERVAL_TIMEOUT_MS 1000
#define INTERRUPTED_SIGNAL_DELAY_MS 10
#define VERSION_CHIPID_REG_NUM 5
#define VERSION_DETECT_RETRY_COUNT 3
#define REQUEST_MUTEX_SLEEP_TIME 60
#define PARA_4BYTE_ALINED 0x3
#define FAKE_MSG_MAX_LEN 120
#define SAMPLE_RATE_8K 8000
#define SAMPLE_RATE_16K 16000
#define SAMPLE_RATE_32K 32000
#define SAMPLE_RATE_48K 48000
#define SAMPLE_RATE_96K 96000
#define SAMPLE_RATE_192K 192000
#define CMD_FUN_NAME_LEN 50
#define DUMP_DIR_LEN 128
#define SYSTEM_GID 1000
#define NOTIFY_DSP_TIMES 3
#define RESULT_SIZE 4
#define ROOT_UID 0

#define MSG_FUNC(id, func) {id, func, #func}
typedef int (*cmd_process_func)(const struct da_combine_param_io_buf *);

union dsp_msg_status {
	unsigned int val;
	struct {
		unsigned int sync_msg: 1;
		unsigned int pll_switch: 1;
		unsigned int poweron: 1;
		unsigned int msg_with_content: 1;
		unsigned int reserved: 28;
	};
};

struct misc_io_async_param {
	unsigned int in_l;
	unsigned int in_h;
	unsigned int in_size;
};

struct misc_io_sync_param {
	unsigned int in_l;
	unsigned int in_h;
	unsigned int in_size;

	unsigned int out_l;
	unsigned int out_h;
	unsigned int out_size;
};

struct dsp_fuzzer {
	unsigned int reserved;
	unsigned int id;
	char buf[FAKE_MSG_MAX_LEN];
};

struct ap_fuzzer {
	unsigned short id;
	unsigned short reserved;
	unsigned int param1;
	unsigned int param2;
	unsigned int param3;
};

struct cmd_func_pair {
	unsigned int id;
	cmd_process_func func;
	char name[CMD_FUN_NAME_LEN];
};

struct dsp_msg_proc_info {
	unsigned int id;
	int (*proc)(const void *data);
	char name[CMD_FUN_NAME_LEN];
};

struct dsp_msg_rcv {
	unsigned int state;
	unsigned int id;
	unsigned char data[MAX_MSG_SIZE];
};

struct request_pll_ops {
	unsigned int scene;
	unsigned int freq;
};

struct sample_rate_to_index {
	unsigned int sample_rate;
	unsigned int index;
};

struct da_combine_dsp_priv {
	struct da_combine_irq *irq;
	struct da_combine_resmgr *resmgr;
	struct da_combine_dsp_config dsp_config;
	struct snd_soc_component *codec;
	struct notifier_block resmgr_nb;
	struct mutex msg_mutex;
	struct mutex state_mutex;
	struct workqueue_struct *msg_proc_wq;
	struct delayed_work msg_proc_work;
	union da_combine_low_freq_status low_freq_status;
	union da_combine_high_freq_status high_freq_status;
	enum pll_state pll_state;
	unsigned int sync_msg_ret;
	wait_queue_head_t sync_msg_wq;
	unsigned int dsp_pllswitch_done;
	wait_queue_head_t dsp_pllswitch_wq;
	bool is_watchdog_coming;
	bool is_sync_write_timeout;
};

static const char * const state_name[] = {
	"PLL_DOWN",
	"PLL_HIGH_FREQ",
	"PLL_LOW_FREQ",
};

#define DA_COMBINE_IOCTL_ASYNCMSG _IOWR('A', 0x90, struct misc_io_async_param)
#define DA_COMBINE_IOCTL_SYNCMSG _IOWR('A', 0x91, struct misc_io_sync_param)
#define DA_COMBINE_IOCTL_MLIB_TEST_MSG _IOWR('A', 0x92, struct misc_io_sync_param)
#define DA_COMBINE_IOCTL_CODEC_DISPLAY_MSG _IOWR('A', 0x93, struct da_combine_dump_param_io_buf)
#define DA_COMBINE_IOCTL_KCOV_FAKE_DSP2AP_MSG _IOWR('A', 0x94, struct dsp_fuzzer)
#define DA_COMBINE_IOCTL_KCOV_FAKE_SYNCMSG _IOWR('A', 0x95, struct ap_fuzzer)
#define DA_COMBINE_IOCTL_KCOV_FAKE_DUMP_LOG _IOW('A', 0x96, unsigned int)
#define DA_COMBINE_IOCTL_DMESG_DUMP_LOG _IOWR('A', 0x97, unsigned int)

static const struct sample_rate_to_index sample_rate_index_table[] = {
	{ SAMPLE_RATE_8K, DA_COMBINE_SAMPLE_RATE_INDEX_8K },
	{ SAMPLE_RATE_16K, DA_COMBINE_SAMPLE_RATE_INDEX_16K },
	{ SAMPLE_RATE_32K, DA_COMBINE_SAMPLE_RATE_INDEX_32K },
	{ SAMPLE_RATE_48K, DA_COMBINE_SAMPLE_RATE_INDEX_48K },
	{ SAMPLE_RATE_96K, DA_COMBINE_SAMPLE_RATE_INDEX_96K },
	{ SAMPLE_RATE_192K, DA_COMBINE_SAMPLE_RATE_INDEX_192K },
};

static struct da_combine_dsp_priv *dsp_priv;

static inline long long timeval_to_ms(struct timeval tv)
{
	return 1000 * tv.tv_sec + tv.tv_usec / 1000;
}

static void update_cmd_status(void)
{
	int count = DA_COMBINE_EXCEPTION_RETRY;
	uint32_t status;

	status = da_combine_dsp_read_reg(DA_COMBINE_DSP_SUB_CMD_STATUS);

	while (count) {
		count--;
		da_combine_dsp_write_reg(DA_COMBINE_DSP_SUB_CMD_STATUS,
			status & DA_COMBINE_CMD_VLD_BITS);
		status = da_combine_dsp_read_reg(DA_COMBINE_DSP_SUB_CMD_STATUS);

		AUDIO_LOGW("cmd status:%d, retry %d", status,
			DA_COMBINE_EXCEPTION_RETRY - count);

		if ((status & (~(uint32_t)DA_COMBINE_CMD_VLD_BITS)) == 0) {
			da_combine_save_log();
			return;
		}
	}

	da_combine_save_log();
	da_combine_wtdog_send_event();
}

static irqreturn_t wtdog_irq_handler(int irq, void *data)
{
	da_combine_dsp_write_reg(DA_COMBINE_DSP_WATCHDOG_LOCK,
		DA_COMBINE_DSP_WATCHDOG_UNLOCK_WORD);
	da_combine_dsp_write_reg(DA_COMBINE_DSP_WATCHDOG_INTCLR,
		DA_COMBINE_DSP_WATCHDOG_INTCLR_WORD);
	da_combine_dsp_write_reg(DA_COMBINE_DSP_WATCHDOG_CONTROL,
		DA_COMBINE_DSP_WATCHDOG_CONTROL_DISABLE);
	da_combine_dsp_write_reg(DA_COMBINE_DSP_WATCHDOG_LOCK,
		DA_COMBINE_DSP_WATCHDOG_LOCK_WORD);

	if (dsp_priv->dsp_config.dsp_ops.wtd_enable)
		dsp_priv->dsp_config.dsp_ops.wtd_enable(false);

	dsp_priv->is_watchdog_coming = true;


	da_combine_wtdog_process();

	return IRQ_HANDLED;
}

static void sync_msg_irq_handler(void)
{
	AUDIO_LOGI("hifi msg cnf");

	if (dsp_priv->dsp_config.msg_state_addr != 0)
		da_combine_dsp_write_reg(dsp_priv->dsp_config.msg_state_addr,
			DA_COMBINE_AP_RECEIVE_MSG_CNF);

	da_combine_clr_cmd_status_bit(DA_COMBINE_DSP_MSG_BIT);
	dsp_priv->sync_msg_ret = 1;

	wake_up_interruptible_all(&dsp_priv->sync_msg_wq);
}

static void pll_switch_irq_handler(void)
{
	AUDIO_LOGI("hifi pll switch done cnf");

	if (dsp_priv->dsp_config.msg_state_addr != 0)
		da_combine_dsp_write_reg(dsp_priv->dsp_config.msg_state_addr,
			DA_COMBINE_AP_RECEIVE_PLL_SW_CNF);

	da_combine_clr_cmd_status_bit(DA_COMBINE_DSP_PLLSWITCH_BIT);
	dsp_priv->dsp_pllswitch_done = 1;

	wake_up_interruptible_all(&dsp_priv->dsp_pllswitch_wq);
}

static void msg_with_content_irq_handler(void)
{
	AUDIO_LOGI("hifi msg come");

	if (dsp_priv->msg_proc_wq == NULL) {
		AUDIO_LOGE("message workqueue doesn't init");
		da_combine_clr_cmd_status_bit(DA_COMBINE_DSP_MSG_WITH_CONTENT_BIT);
		return;
	}


	if (!queue_delayed_work(dsp_priv->msg_proc_wq,
		&dsp_priv->msg_proc_work, msecs_to_jiffies(0))) {
		AUDIO_LOGW("lost dsp msg");
		da_combine_dsp_write_reg(dsp_priv->dsp_config.rev_msg_addr,
			DA_COMBINE_DSP_TO_AP_MSG_DEACTIVE);
	}

	da_combine_clr_cmd_status_bit(DA_COMBINE_DSP_MSG_WITH_CONTENT_BIT);
}

static irqreturn_t msg_irq_handler(int irq, void *data)
{
	union dsp_msg_status status;

	IN_FUNCTION;

	if (!da_combine_dsp_read_reg(DA_COMBINE_DSP_CMD_STATUS_VLD)) {
		AUDIO_LOGE("cmd invalid");
		return IRQ_HANDLED;
	}

	status.val = da_combine_dsp_read_reg(DA_COMBINE_DSP_CMD_STATUS);
	AUDIO_LOGI("cmd status:%d", status.val);

	if (status.reserved != 0)
		update_cmd_status();

	if (dsp_priv->dsp_config.cmd4_addr)
		AUDIO_LOGI("timestamp:%u update cnf",
			da_combine_dsp_read_reg(dsp_priv->dsp_config.cmd4_addr));

	if (status.sync_msg)
		sync_msg_irq_handler();
	if (status.pll_switch)
		pll_switch_irq_handler();
	if (status.poweron)
		da_combine_dsp_poweron_irq_handler();
	if (status.msg_with_content)
		msg_with_content_irq_handler();

	OUT_FUNCTION;

	return IRQ_HANDLED;
}

static int check_algo_para(const unsigned char *arg, const unsigned int len)
{
	if (arg == NULL || len == 0) {
		AUDIO_LOGE("input buf or len error, len: %u", len);
		return -EINVAL;
	}

	if (len > MAX_PARA_SIZE) {
		AUDIO_LOGE("msg length: %u exceed limit", len);
		return -EINVAL;
	}

	if ((len & PARA_4BYTE_ALINED) != 0) {
		AUDIO_LOGE("msg length: %u is not 4 byte aligned", len);
		return -EINVAL;
	}

	return 0;
}

static int write_algo_para(const unsigned char *arg,
	const unsigned int len)
{
	if (check_algo_para(arg, len)) {
		AUDIO_LOGE("input buf or len error, len: %u", len);
		return -EINVAL;
	}

	da_combine_write(dsp_priv->dsp_config.para_addr, (unsigned int *)arg, len);

	return 0;
}

static int read_algo_para(unsigned char *arg, const unsigned int len)
{
	if (check_algo_para(arg, len)) {
		AUDIO_LOGE("input buf or len error, len: %u", len);
		return -EINVAL;
	}

	da_combine_read(dsp_priv->dsp_config.para_addr, arg, len);

	return 0;
}

static bool request_state_mutex(void)
{
	int count = DA_COMBINE_GET_STATE_MUTEX_RETRY;

	while (count) {
		if (mutex_trylock(&dsp_priv->state_mutex)) {
			AUDIO_LOGI("state mutex lock");
			return true;
		}
		msleep(REQUEST_MUTEX_SLEEP_TIME);
		count--;
	}

	AUDIO_LOGE("request state mutex timeout %dms",
		REQUEST_MUTEX_SLEEP_TIME * DA_COMBINE_GET_STATE_MUTEX_RETRY);

	return false;
}

static void release_state_mutex(void)
{
	mutex_unlock(&dsp_priv->state_mutex);
	AUDIO_LOGI("state mutex unlock");
}

static int sync_write_msg(const void *msg, const unsigned int len)
{
	IN_FUNCTION;

	if ((len == 0) || (len > MAX_MSG_SIZE)) {
		AUDIO_LOGE("msg length exceed limit");
		return -EINVAL;
	}

	if ((len & PARA_4BYTE_ALINED) != 0) {
		AUDIO_LOGE("msg length: %u is not 4 byte aligned", len);
		return -EINVAL;
	}

	da_combine_write(dsp_priv->dsp_config.msg_addr, (unsigned int *)msg, len);

	dsp_priv->sync_msg_ret = 0;

	AUDIO_LOGI("current pll state: %u, new pll state: %u",
		da_combine_dsp_read_reg(dsp_priv->dsp_config.cmd1_addr), dsp_priv->pll_state);
	da_combine_dsp_write_reg(dsp_priv->dsp_config.cmd1_addr,
		dsp_priv->pll_state);
	if (dsp_priv->dsp_config.msg_state_addr != 0)
		da_combine_dsp_write_reg(dsp_priv->dsp_config.msg_state_addr,
			DA_COMBINE_AP_SEND_MSG);
	if (dsp_priv->dsp_config.dsp_ops.notify_dsp)
		dsp_priv->dsp_config.dsp_ops.notify_dsp();

	OUT_FUNCTION;

	return 0;
}

static int check_sync_write_status(void)
{
	union dsp_msg_status status;

	if (!da_combine_dsp_read_reg(DA_COMBINE_DSP_CMD_STATUS_VLD)) {
		AUDIO_LOGE("cmd invalid");
		goto out;
	}

	status.val = da_combine_dsp_read_reg(DA_COMBINE_DSP_CMD_STATUS);
	AUDIO_LOGI("cmd status:%d, sync msg ret:%d",
		status.val, dsp_priv->sync_msg_ret);

	if (status.sync_msg) {
		AUDIO_LOGW("hifi msg cnf, but ap donot handle this irq");
		if (dsp_priv->dsp_config.msg_state_addr != 0)
			da_combine_dsp_write_reg(dsp_priv->dsp_config.msg_state_addr,
				DA_COMBINE_AP_RECEIVE_MSG_CNF);

		da_combine_clr_cmd_status_bit(DA_COMBINE_DSP_MSG_BIT);

		return 0;
	}

out:
	/* can't get codec version,reset system */
	if (da_combine_error_detect())
		rdr_system_error(RDR_AUDIO_CODEC_ERR_MODID, 0, 0);

	AUDIO_LOGE("cmd timeout");

	if (!(dsp_priv->is_watchdog_coming) &&
		!(dsp_priv->is_sync_write_timeout)) {
		AUDIO_LOGE("save log and reset media ");
		dsp_priv->is_sync_write_timeout = true;
		da_combine_wtdog_process();
	}

	return -EBUSY;
}

static int wait_sync_msg_result(void)
{
	long ret;
	long long begin, update;
	struct timeval time_begin, time_update;
	unsigned int interrupt_count = 0;

	do_gettimeofday(&time_begin);
	begin = timeval_to_ms(time_begin);
wait_interrupt:
	ret = wait_event_interruptible_timeout(dsp_priv->sync_msg_wq,
		(dsp_priv->sync_msg_ret == 1), HZ);
	if (ret > 0) {
		if (interrupt_count > 0)
			AUDIO_LOGW("sync cmd is interrupted %u times",
				interrupt_count);
		return 0;
	} else if (ret == -ERESTARTSYS) {
		interrupt_count++;
		do_gettimeofday(&time_update);
		update = timeval_to_ms(time_update);
		if (update - begin > INTERVAL_TIMEOUT_MS) {
			AUDIO_LOGW("cmd is interrupted %u times", interrupt_count);
			return -EINVAL;
		} else {
			if (update < begin)
				do_gettimeofday(&time_begin);
			if (dsp_priv->sync_msg_ret == 0)
				goto wait_interrupt;
		}
	} else {
		AUDIO_LOGE("cmd is interrupted %u times, ret: %ld",
			interrupt_count, ret);
		return -EINVAL;
	}

	return 0;
}

static void notify_dsp_pllswitch(unsigned char state)
{
	long ret = 0;
	unsigned int interrupt_count = 0;
	unsigned long long begin, update;
	struct timeval time_begin, time_update;

	/* notify dsp stop dma and close dspif */
	dsp_priv->dsp_pllswitch_done = 0;
	da_combine_dsp_write_reg(dsp_priv->dsp_config.cmd0_addr, state);

	AUDIO_LOGI("cmd[0x%x]reg[0x%x]pll state[0x%x]", state,
		da_combine_dsp_read_reg(dsp_priv->dsp_config.cmd0_addr),
		da_combine_dsp_read_reg(dsp_priv->dsp_config.cmd1_addr));

	if (dsp_priv->dsp_config.dsp_ops.notify_dsp)
		dsp_priv->dsp_config.dsp_ops.notify_dsp();

	do_gettimeofday(&time_begin);
	begin = timeval_to_ms(time_begin);

	do {
		if (dsp_priv->is_watchdog_coming) {
			AUDIO_LOGE("watchdog have already come, can't send msg");
			break;
		}

		ret = wait_event_interruptible_timeout(dsp_priv->dsp_pllswitch_wq,
			(dsp_priv->dsp_pllswitch_done == 1), HZ);

		if (dsp_priv->dsp_pllswitch_done) {
			AUDIO_LOGI("pll switch done, interrupt count %u",
				interrupt_count);
			break;
		}

		if (ret == -ERESTARTSYS) {
			interrupt_count++;
			msleep(INTERRUPTED_SIGNAL_DELAY_MS);
		}

		do_gettimeofday(&time_update);
		update = timeval_to_ms(time_update);
	} while ((update - begin) < NOTIFY_DSP_TIMES * INTERVAL_TIMEOUT_MS);

	AUDIO_LOGI("pll switch is interrupted %u times, ret: %ld",
		interrupt_count, ret);
}

static void notify_dsp_pause(void)
{
	IN_FUNCTION;

	AUDIO_LOGI("state mutex lock");
	mutex_lock(&dsp_priv->state_mutex);

	if (da_combine_get_dsp_state() &&
		!(dsp_priv->is_sync_write_timeout) &&
		!(dsp_priv->is_watchdog_coming)) {
		AUDIO_LOGI("notify dsp close dma");

		notify_dsp_pllswitch(HIFI_POWER_CLK_OFF);
		da_combine_pause_dsp();
	}

	OUT_FUNCTION;
}

static void show_codec_chip_info(void)
{
	unsigned int regval[VERSION_CHIPID_REG_NUM];

	regval[0] = da_combine_dsp_read_reg(DA_COMBINE_VERSION_REG);
	regval[1] = da_combine_dsp_read_reg(DA_COMBINE_CHIP_ID_REG0);
	regval[2] = da_combine_dsp_read_reg(DA_COMBINE_CHIP_ID_REG1);
	regval[3] = da_combine_dsp_read_reg(DA_COMBINE_CHIP_ID_REG2);
	regval[4] = da_combine_dsp_read_reg(DA_COMBINE_CHIP_ID_REG3);

	AUDIO_LOGI("version:0x%x", regval[0]);
}

static void notify_dsp_resume(enum pll_state state)
{
	IN_FUNCTION;

	show_codec_chip_info();

	AUDIO_LOGI("%s->%s, dsp is runing:%d", state_name[dsp_priv->pll_state],
		state_name[state], da_combine_get_dsp_state());

	dsp_priv->pll_state = state;

	AUDIO_LOGI("current pll state: %u, new pll state: %u",
		da_combine_dsp_read_reg(dsp_priv->dsp_config.cmd1_addr), dsp_priv->pll_state);
	da_combine_dsp_write_reg(dsp_priv->dsp_config.cmd1_addr,
		dsp_priv->pll_state);
	if (dsp_priv->dsp_config.dsp_ops.set_dsp_div) {
		dsp_priv->dsp_config.dsp_ops.set_dsp_div(dsp_priv->pll_state);
		AUDIO_LOGI("switch dsp pll source");
	}

	if (da_combine_get_dsp_state() &&
		!(dsp_priv->is_sync_write_timeout) &&
		!(dsp_priv->is_watchdog_coming)) {
		da_combine_resume_dsp(dsp_priv->pll_state);
		notify_dsp_pllswitch(HIFI_POWER_CLK_ON);
	}
	mutex_unlock(&dsp_priv->state_mutex);
	AUDIO_LOGI("state mutex unlock");

	OUT_FUNCTION;
}

/* check for parameters used by misc, only for if_open/if_close */
static int check_scene_para(const struct da_combine_param_io_buf *param)
{
	unsigned char index;
	unsigned int i;
	unsigned int max_if_id;
	unsigned int message_size;
	const struct dsp_if_open_req_stru *dsp_if_open_req = NULL;
	const struct pcm_process_dma_msg_stru *dma_msg_stru = NULL;
	const struct pcm_if_msg_stru *pcm_if_msg = NULL;

	if (da_combine_check_msg_para(param, sizeof(struct dsp_if_open_req_stru))) {
		AUDIO_LOGE("param check err");
		return -EINVAL;
	}

#ifdef CONFIG_SND_SOC_DA_COMBINE_V5
	if (dsp_priv->dsp_config.codec_type == DA_COMBINE_CODEC_TYPE_V5)
		max_if_id = DA_COMBINE_DSP_IF_PORT_9;
	else
#endif
		max_if_id = DA_COMBINE_DSP_IF_PORT_8;

	dsp_if_open_req = (struct dsp_if_open_req_stru *)(param->buf_in);
	dma_msg_stru = &dsp_if_open_req->process_dma;

	if (dma_msg_stru->if_count > max_if_id) {
		AUDIO_LOGE("try to open too many ifs");
		return -EINVAL;
	}

	message_size = sizeof(struct pcm_if_msg_stru) * (dma_msg_stru->if_count)
		+ sizeof(struct dsp_if_open_req_stru);
	if (param->buf_size_in < message_size) {
		AUDIO_LOGE("input size: %u invalid", param->buf_size_in);
		return -EINVAL;
	}

	for (i = 0; i < dma_msg_stru->if_count; i++) {
		pcm_if_msg = &dma_msg_stru->if_cfg_list[i];

		if (pcm_if_msg->if_id > max_if_id) {
			AUDIO_LOGE("dsp if id %d is out of range",
				pcm_if_msg->if_id);
			return -EINVAL;
		}

		if (da_combine_get_sample_rate_index(pcm_if_msg->sample_ratein,
			&index) == false)
			return -EINVAL;
	}

	return 0;
}

/* now we'v alread check the para, so don't do it again */
static void set_dsp_if_sample_rate(const char *arg)
{
	unsigned int i;
	const struct dsp_if_open_req_stru *dsp_if_open_req = NULL;
	const struct pcm_process_dma_msg_stru *dma_msg_stru = NULL;
	const struct pcm_if_msg_stru *pcm_if_msg = NULL;

	IN_FUNCTION;

	dsp_if_open_req = (struct dsp_if_open_req_stru *)arg;
	dma_msg_stru = &dsp_if_open_req->process_dma;

	for (i = 0; i < dma_msg_stru->if_count; i++) {
		pcm_if_msg = &dma_msg_stru->if_cfg_list[i];
		if (dsp_priv->dsp_config.dsp_ops.set_sample_rate)
			dsp_priv->dsp_config.dsp_ops.set_sample_rate(pcm_if_msg->if_id,
				pcm_if_msg->sample_ratein,
				pcm_if_msg->sample_rateout);
	}

	OUT_FUNCTION;
}

static const struct dsp_msg_proc_info dsp_msg_map[] = {
	MSG_FUNC(ANC_START_HOOK, da_combine_anc_beta_start_hook),
	MSG_FUNC(ANC_STOP_HOOK, da_combine_anc_beta_stop_hook),
	MSG_FUNC(ANC_TRIGGER_DFT, da_combine_anc_beta_upload_log),
	MSG_FUNC(DSM_MONO_STATIC0, da_combine_dsm_report_with_creat_dir),
	MSG_FUNC(DSM_DUAL_STATIC0, da_combine_dsm_dump_file_without_creat_dir),
	MSG_FUNC(DSM_DUAL_STATIC1, da_combine_dsm_report_without_creat_dir),
	MSG_FUNC(PA_BUFFER_REVERSE, da_combine_report_pa_buffer_reverse),
	MSG_FUNC(VIRTUAL_BTN_MONITOR, da_combine_upload_virtual_btn_beta),
	MSG_FUNC(WAKEUP_ERR_MSG, da_combine_wakeup_err_msg),
};

static void dsp_to_ap_msg_proc(struct dsp_msg_rcv *msg_buff)
{
	int ret;
	unsigned int i;
	void *msg_body = (void *)msg_buff->data;

	AUDIO_LOGI("msg id:0x%x", msg_buff->id);

	for (i = 0; i < ARRAY_SIZE(dsp_msg_map); i++) {
		if (dsp_msg_map[i].id == msg_buff->id) {
			ret = dsp_msg_map[i].proc(msg_body);
			if (ret != 0)
				AUDIO_LOGE("process err");

			break;
		}
	}
}

static void dsp_msg_proc_work(struct work_struct *work)
{
	struct dsp_msg_rcv rev_msg = {0};

	UNUSED_PARAMETER(work);

	/*
	 * |~active flag~|~msg body~|
	 * |~~~~4 byte~~~|~124 byte~|
	 */
	if (!dsp_priv->dsp_config.rev_msg_addr)
		return;

	if (da_combine_request_low_pll_resource(LOW_FREQ_SCENE_MSG_PROC))
		return;

	da_combine_read(dsp_priv->dsp_config.rev_msg_addr,
		(unsigned char *)&rev_msg, MAX_MSG_SIZE);

	if (rev_msg.state != DA_COMBINE_DSP_TO_AP_MSG_ACTIVE) {
		AUDIO_LOGE("msg proc status err:0x%x", rev_msg.state);
		da_combine_release_low_pll_resource(LOW_FREQ_SCENE_MSG_PROC);
		return;
	}

	dsp_to_ap_msg_proc(&rev_msg);
	da_combine_dsp_write_reg(dsp_priv->dsp_config.rev_msg_addr,
		DA_COMBINE_DSP_TO_AP_MSG_DEACTIVE);
	da_combine_release_low_pll_resource(LOW_FREQ_SCENE_MSG_PROC);
}

static int request_wakeup_pll(void)
{
	int ret;

	ret = da_combine_request_low_pll_resource(LOW_FREQ_SCENE_WAKE_UP);
	if (ret == -EAGAIN) {
		if (dsp_priv->low_freq_status.multi_wake_up) {
			AUDIO_LOGW("scene wakeup is already opened");
			return -EINVAL;
		}

		dsp_priv->low_freq_status.multi_wake_up = 1;
		return 0;
	}

	return 0;
}

static const struct request_pll_ops scene_freq_map[] = {
	{ MLIB_PATH_WAKEUP,      LOW_FREQ_SCENE_WAKE_UP      },
	{ MLIB_PATH_ANC,         HIGH_FREQ_SCENE_ANC         },
	{ MLIB_PATH_ANC_DEBUG,   HIGH_FREQ_SCENE_ANC_DEBUG   },
	{ MLIB_PATH_SMARTPA,     HIGH_FREQ_SCENE_PA          },
	{ MLIB_PATH_IR_LEARN,    HIGH_FREQ_SCENE_IR_LEARN    },
	{ MLIB_PATH_IR_TRANS,    HIGH_FREQ_SCENE_IR_TRANS    },
	{ MLIB_PATH_VIRTUAL_BTN, HIGH_FREQ_SCENE_VIRTUAL_BTN },
	{ MLIB_PATH_ULTRASONIC,  HIGH_FREQ_SCENE_ULTRASONIC  },
};

static int request_scene_pll(unsigned int proc_id)
{
	int ret = -EINVAL;
	unsigned int i;
	unsigned int freq;

	if (proc_id == MLIB_PATH_WAKEUP) {
		ret = request_wakeup_pll();
	} else {
		for (i = 0; i < ARRAY_SIZE(scene_freq_map); i++) {
			if (scene_freq_map[i].scene == proc_id) {
				freq = scene_freq_map[i].freq;
				ret = da_combine_request_pll_resource(freq);
				AUDIO_LOGI("request scene %d", proc_id);
				return ret;
			}
		}
	}

	return ret;
}

static void release_scene_pll(unsigned int proc_id)
{
	unsigned int i;
	unsigned int freq;

	if (proc_id == MLIB_PATH_WAKEUP) {
		if (dsp_priv->low_freq_status.multi_wake_up)
			dsp_priv->low_freq_status.multi_wake_up = 0;
		else
			da_combine_release_low_pll_resource(LOW_FREQ_SCENE_WAKE_UP);
	} else {
		for (i = 0; i < ARRAY_SIZE(scene_freq_map); i++) {
			if (scene_freq_map[i].scene == proc_id) {
				freq = scene_freq_map[i].freq;
				da_combine_release_pll_resource(freq);
				AUDIO_LOGI("release scene %d", proc_id);
				return;
			}
		}
	}
}

static void mad_path_enable(void)
{
	if (!dsp_priv->low_freq_status.multi_wake_up &&
		dsp_priv->low_freq_status.wake_up) {
		if (dsp_priv->dsp_config.dsp_ops.set_if_bypass)
			dsp_priv->dsp_config.dsp_ops.set_if_bypass(DA_COMBINE_DSP_IF_PORT_1, false);

		if (dsp_priv->dsp_config.dsp_ops.mad_enable)
			dsp_priv->dsp_config.dsp_ops.mad_enable();
	}
}

static void mad_path_disable(void)
{
	if (!dsp_priv->low_freq_status.multi_wake_up &&
		dsp_priv->low_freq_status.wake_up) {
		if (dsp_priv->dsp_config.dsp_ops.set_if_bypass)
			dsp_priv->dsp_config.dsp_ops.set_if_bypass(DA_COMBINE_DSP_IF_PORT_1, true);

		if (dsp_priv->dsp_config.dsp_ops.mad_disable)
			dsp_priv->dsp_config.dsp_ops.mad_disable();
	}
}

static void stop_hook_by_scene(unsigned int proc_id)
{
	switch (proc_id) {
	case MLIB_PATH_WAKEUP:
	case MLIB_PATH_SMARTPA:
		if (dsp_priv->low_freq_status.wake_up &&
			dsp_priv->high_freq_status.pa &&
			dsp_priv->high_freq_status.om_hook) {
			AUDIO_LOGW("pa & wake up exist, can not hook");
			da_combine_stop_dsp_hook();
		}
		break;
	case MLIB_PATH_ANC:
	case MLIB_PATH_ANC_DEBUG:
		break;
	case MLIB_PATH_IR_LEARN:
	case MLIB_PATH_IR_TRANS:
	case MLIB_PATH_VIRTUAL_BTN:
	case MLIB_PATH_ULTRASONIC:
		if (dsp_priv->high_freq_status.om_hook) {
			AUDIO_LOGW("in scene %u, can not hook", proc_id);
			da_combine_stop_dsp_hook();
		}
		break;
	default:
		AUDIO_LOGE("scene: %u unsupport\n", proc_id);
		break;
	}
}

static bool is_high_pll_scene_opened(unsigned int proc_id)
{
	unsigned int i;
	unsigned int scene_id = 0;

	if (proc_id < MLIB_PATH_SMARTPA || proc_id >= MLIB_PATH_BUTT) {
		AUDIO_LOGE("scene: %u unsupport", proc_id);
		return false;
	}

	for (i = 0; i < ARRAY_SIZE(scene_freq_map); i++) {
		if (scene_freq_map[i].scene == proc_id) {
			scene_id = scene_freq_map[i].freq;
			break;
		}
	}

	if ((dsp_priv->high_freq_status.val & (1 << scene_id)) == 0) {
		AUDIO_LOGW("scene %u is not opened", scene_id);
		return false;
	}

	return true;
}

static bool is_wakeup_opened(void)
{
	if (dsp_priv->low_freq_status.multi_wake_up)
		return true;

	if (!dsp_priv->low_freq_status.wake_up) {
		AUDIO_LOGW("scene wakeup is not opened");
		return false;
	}

	return true;
}

static bool is_scene_opened(unsigned int proc_id)
{
	if (proc_id == MLIB_PATH_WAKEUP) {
		if (!is_wakeup_opened())
			return false;
	} else {
		if (!is_high_pll_scene_opened(proc_id))
			return false;
	}

	return true;
}

static int open_scene(const struct da_combine_param_io_buf *param)
{
	int ret;
	const struct dsp_if_open_req_stru *dsp_if_open_req = NULL;
	const struct pcm_process_dma_msg_stru *dma_msg_stru = NULL;

	IN_FUNCTION;

	ret = check_scene_para(param);
	if (ret != 0) {
		AUDIO_LOGE("dsp if parameter invalid");
		return ret;
	}

	dsp_if_open_req = (struct dsp_if_open_req_stru *)(param->buf_in);
	dma_msg_stru = &dsp_if_open_req->process_dma;

	da_combine_stop_dspif_hook();

	if (dma_msg_stru->process_id == MLIB_PATH_ANC)
		da_combine_set_voice_hook_switch(dsp_if_open_req->perms);

	ret = request_scene_pll(dma_msg_stru->process_id);
	if (ret != 0) {
		AUDIO_LOGE("if open pll process err");
		return ret;
	}

	if (dma_msg_stru->process_id == MLIB_PATH_WAKEUP)
		mad_path_enable();

	stop_hook_by_scene(dma_msg_stru->process_id);

	set_dsp_if_sample_rate((char *)param->buf_in);

	AUDIO_LOGI("current pll state: %u, new pll state: %u",
		da_combine_dsp_read_reg(dsp_priv->dsp_config.cmd1_addr), dsp_priv->pll_state);
	da_combine_dsp_write_reg(dsp_priv->dsp_config.cmd1_addr,
		dsp_priv->pll_state);

	ret = da_combine_sync_write(param->buf_in, param->buf_size_in);
	if (ret != 0) {
		AUDIO_LOGE("sync write err");
		return ret;
	}

	OUT_FUNCTION;

	return ret;
}

static int close_scene(const struct da_combine_param_io_buf *param)
{
	int ret;
	const struct dsp_if_open_req_stru *dsp_if_open_req = NULL;
	const struct pcm_process_dma_msg_stru *dma_msg_stru = NULL;

	IN_FUNCTION;

	ret = check_scene_para(param);
	if (ret != 0) {
		AUDIO_LOGE("dsp if parameter invalid");
		return -EINVAL;
	}

	dsp_if_open_req = (struct dsp_if_open_req_stru *)(param->buf_in);
	dma_msg_stru = &dsp_if_open_req->process_dma;

	if (!is_scene_opened(dma_msg_stru->process_id)) {
		AUDIO_LOGW("scene is not opened");
		if (dma_msg_stru->process_id == MLIB_PATH_WAKEUP)
			return 0;

		return -EINVAL;
	}

	if (dma_msg_stru->process_id == MLIB_PATH_WAKEUP)
		mad_path_disable();

	AUDIO_LOGI("current pll state: %u, new pll state: %u",
		da_combine_dsp_read_reg(dsp_priv->dsp_config.cmd1_addr), dsp_priv->pll_state);
	da_combine_dsp_write_reg(dsp_priv->dsp_config.cmd1_addr,
		dsp_priv->pll_state);

	ret = da_combine_sync_write(param->buf_in, param->buf_size_in);
	if (ret != 0) {
		AUDIO_LOGE("sync write ret: %d", ret);
		return -EINVAL;
	}

	release_scene_pll(dma_msg_stru->process_id);

	OUT_FUNCTION;

	return ret;
}

static int start_fast_trans(const struct da_combine_param_io_buf *param)
{
	int ret;
	bool fast_mode_enable = false;
	const struct fast_trans_msg_stru *fast_mode_msg = NULL;

	IN_FUNCTION;

	if (param->buf_size_in < sizeof(struct fast_trans_msg_stru)) {
		AUDIO_LOGE("input size:%u invalid", param->buf_size_in);
		return -EINVAL;
	}

	fast_mode_msg = (struct fast_trans_msg_stru *)param->buf_in;

	if (fast_mode_msg->fast_trans_enable != 0)
		fast_mode_enable = true;

	ret = da_combine_fast_mode_set(fast_mode_enable);
	if (ret != 0)
		return -EPERM;

	ret = da_combine_request_low_pll_resource(LOW_FREQ_SCENE_FAST_TRANS_SET);
	if (ret != 0)
		return -EPERM;

	ret = da_combine_sync_write(param->buf_in, param->buf_size_in);
	if (ret != 0)
		return -EPERM;

	da_combine_release_low_pll_resource(LOW_FREQ_SCENE_FAST_TRANS_SET);

	return ret;
}

static int set_param(const struct da_combine_param_io_buf *param)
{
	int ret;
	struct mlib_para_set_req_stru *mlib_para = NULL;
	struct mlib_parameter_st *mlib_para_content = NULL;

	IN_FUNCTION;

	if (da_combine_check_msg_para(param, sizeof(struct mlib_para_set_req_stru)))
		return -EINVAL;

	mlib_para = (struct mlib_para_set_req_stru *)param->buf_in;
	if (mlib_para->process_id == MLIB_PATH_ULTRASONIC) {
		if (dsp_priv->high_freq_status.ultrasonic == 0)
			return -EPERM;
	}

	if (mlib_para->size != param->buf_size_in - sizeof(struct mlib_para_set_req_stru) ||
			mlib_para->size < sizeof(int)) { /* type of mlib_para_content->key is int */
		AUDIO_LOGE("mlib para size %u is out of range, buffer in size %u",
			mlib_para->size, param->buf_size_in);
		AUDIO_LOGE("request size %lu, key size %lu",
			sizeof(struct mlib_para_set_req_stru), sizeof(int));
		return -EINVAL;
	}

	mlib_para_content = (struct mlib_parameter_st *)mlib_para->auc_data;

	if (mlib_para_content->key == MLIB_ST_PARA_TRANSACTION)
		ret = da_combine_request_low_pll_resource(LOW_FREQ_SCENE_SET_PARA);
	else
		ret = da_combine_request_pll_resource(HIGH_FREQ_SCENE_SET_PARA);
	if (ret != 0) {
		AUDIO_LOGE("request pll failed");
		return -EPERM;
	}

	ret = write_algo_para(mlib_para->auc_data, mlib_para->size);
	if (ret != 0) {
		AUDIO_LOGE("write mlib para failed");
		goto end;
	}

	ret = da_combine_sync_write(param->buf_in, sizeof(struct mlib_para_set_req_stru));
	if (ret != 0) {
		AUDIO_LOGE("sync write failed");
		goto end;
	}

end:
	if (mlib_para_content->key == MLIB_ST_PARA_TRANSACTION)
		da_combine_release_low_pll_resource(LOW_FREQ_SCENE_SET_PARA);
	else
		da_combine_release_pll_resource(HIGH_FREQ_SCENE_SET_PARA);

	OUT_FUNCTION;

	return ret;
}

static int get_param(const struct da_combine_param_io_buf *param)
{
	int ret;
	struct mlib_para_get_req_stru *mlib_para = NULL;

	if (da_combine_check_msg_para(param, sizeof(struct mlib_para_get_req_stru)))
		return -EINVAL;

	if (param->buf_out == NULL || param->buf_size_out == 0) {
		AUDIO_LOGE("input buf out or buf size out[%u] error",
			param->buf_size_out);
		return -EINVAL;
	}

	mlib_para = (struct mlib_para_get_req_stru *)param->buf_in;
	if (mlib_para->process_id == MLIB_PATH_ULTRASONIC) {
		if (dsp_priv->high_freq_status.ultrasonic == 0)
			return -EPERM;
	}

	ret = da_combine_request_pll_resource(HIGH_FREQ_SCENE_GET_PARA);
	if (ret != 0) {
		AUDIO_LOGE("request pll failed");
		goto end;
	}

	if (param->buf_size_in > sizeof(struct mlib_para_get_req_stru)) {
		ret = write_algo_para(mlib_para->auc_data,
			param->buf_size_in -
			(unsigned int)sizeof(struct mlib_para_get_req_stru));
		if (ret != 0) {
			AUDIO_LOGE("write mlib para failed");
			goto end;
		}
	}

	ret = da_combine_sync_write(param->buf_in,
		(unsigned int)sizeof(struct mlib_para_get_req_stru));
	if (ret != 0) {
		AUDIO_LOGE("sync write failed");
		goto end;
	}

	if (param->buf_size_out <= RESULT_SIZE) {
		AUDIO_LOGE("not enough space for para get");
		goto end;
	}

	/* skip buffer that record result */
	ret = read_algo_para(param->buf_out + RESULT_SIZE,
		param->buf_size_out - RESULT_SIZE);
	if (ret != 0) {
		AUDIO_LOGE("read para failed");
		goto end;
	}

end:
	da_combine_release_pll_resource(HIGH_FREQ_SCENE_GET_PARA);

	return ret;
}

static int config_fast_trans(const struct da_combine_param_io_buf *param)
{
	int ret;
	const struct fast_trans_cfg_req_stru *pfastcfg = NULL;

	if (da_combine_check_msg_para(param, sizeof(struct fast_trans_cfg_req_stru)))
		return -EINVAL;

	pfastcfg = (struct fast_trans_cfg_req_stru *)(param->buf_in);

	AUDIO_LOGI("func fasttrans config [%d]", pfastcfg->msg_id);

	if (pfastcfg->msg_id == ID_AP_DSP_FASTTRANS_OPEN)
		dsp_priv->dsp_config.dsp_ops.set_fasttrans_enable(true);
	else
		dsp_priv->dsp_config.dsp_ops.set_fasttrans_enable(false);

	ret = da_combine_sync_write(param->buf_in, param->buf_size_in);
	if (ret != 0)
		AUDIO_LOGE("sync write failed");

	return ret;
}

static const struct cmd_func_pair cmd_func_map[] = {
	MSG_FUNC(ID_AP_DSP_IF_OPEN, open_scene),
	MSG_FUNC(ID_AP_DSP_IF_CLOSE, close_scene),
	MSG_FUNC(ID_AP_DSP_PARAMETER_SET, set_param),
	MSG_FUNC(ID_AP_DSP_PARAMETER_GET, get_param),
	MSG_FUNC(ID_AP_DSP_FASTMODE, start_fast_trans),
	MSG_FUNC(ID_AP_DSP_FASTTRANS_OPEN, config_fast_trans),
	MSG_FUNC(ID_AP_DSP_FASTTRANS_CLOSE, config_fast_trans),
	MSG_FUNC(ID_AP_DSP_HOOK_START, da_combine_start_hook),
	MSG_FUNC(ID_AP_DSP_HOOK_STOP, da_combine_stop_hook),
	MSG_FUNC(ID_AP_IMGAE_DOWNLOAD, da_combine_fw_download),
};

static cmd_process_func select_cmd_func(const unsigned char *arg,
	const struct cmd_func_pair *func_map, const unsigned int func_map_size)
{
	unsigned int i;
	unsigned short msg_id;

	IN_FUNCTION;

	if (arg == NULL) {
		AUDIO_LOGE("arg is null");
		return NULL;
	}

	msg_id = *(unsigned short *)arg;

	for (i = 0; i < func_map_size; i++) {
		if (func_map[i].id == msg_id) {
			AUDIO_LOGI("find cmd func id: %hu", msg_id);
			return func_map[i].func;
		}
	}

	AUDIO_LOGE("cmd process func for id: %hu not found", msg_id);

	OUT_FUNCTION;

	return NULL;
}

static int put_user_data(unsigned int usr_para_size,
	void __user *usr_para_addr, unsigned int krn_para_size,
	const void *krn_para_addr)
{
	int ret = 0;

	IN_FUNCTION;

	if (usr_para_size == 0 || !usr_para_addr) {
		AUDIO_LOGE("input usr para size: %d or usr para addr error",
			usr_para_size);
		return -EINVAL;
	}

	if (krn_para_size == 0 || !krn_para_addr) {
		AUDIO_LOGE("input error krn para size: %u", krn_para_size);
		return -EINVAL;
	}

	if (krn_para_size != usr_para_size) {
		AUDIO_LOGE("krn para size: %d diff from usr para size: %d",
			krn_para_size, usr_para_size);
		return -EINVAL;
	}

	AUDIO_LOGD("user para size:%d", usr_para_size);
	ret = copy_to_user(usr_para_addr, krn_para_addr, usr_para_size);
	if (ret != 0) {
		AUDIO_LOGE("copy to user fail, ret is %d", ret);
		return -EPERM;
	}

	OUT_FUNCTION;

	return 0;
}

static int get_output_param(unsigned int usr_para_size,
	const void __user *usr_para_addr, unsigned int *krn_para_size,
	void **krn_para_addr)
{
	if (*krn_para_addr != NULL) {
		AUDIO_LOGE("*krn para addr has already alloc memory");
		return -EINVAL;
	}

	AUDIO_LOGD("malloc size: %u", usr_para_size);
	if (usr_para_size == 0 || usr_para_size > MAX_OUT_PARA_SIZE) {
		AUDIO_LOGE("usr space size invalid");
		return -EINVAL;
	}

	if (usr_para_addr == NULL) {
		AUDIO_LOGE("usr para addr is NULL");
		return -EINVAL;
	}

	*krn_para_addr = kzalloc(usr_para_size, GFP_KERNEL);
	if (*krn_para_addr == NULL) {
		AUDIO_LOGE("kzalloc fail");
		return -ENOMEM;
	}

	*krn_para_size = usr_para_size;

	return 0;
}

static int get_input_param(unsigned int usr_para_size,
	const void __user *usr_para_addr, unsigned int *krn_para_size,
	void **krn_para_addr)
{
	int ret = 0;
	void *para_in = NULL;
	unsigned int para_size_in;

	IN_FUNCTION;

	if (usr_para_addr == NULL) {
		AUDIO_LOGE("usr para addr is null no user data");
		ret = -EINVAL;
		goto ERR;
	}

	if ((usr_para_size == 0) || (usr_para_size > MAX_USR_INPUT_SIZE)) {
		AUDIO_LOGE("usr buffer size:%u out of range", usr_para_size);
		ret = -EINVAL;
		goto ERR;
	}

	para_size_in = roundup(usr_para_size, 4);

	para_in = kzalloc(para_size_in, GFP_KERNEL);
	if (para_in == NULL) {
		AUDIO_LOGE("kzalloc para in fail");
		ret = -ENOMEM;
		goto ERR;
	}

	if (copy_from_user(para_in, usr_para_addr, usr_para_size)) {
		AUDIO_LOGE("copy from user fail");
		ret = -EINVAL;
		goto ERR;
	}

	*krn_para_size = para_size_in;
	*krn_para_addr = para_in;

	OUT_FUNCTION;

	return ret;

ERR:
	if (para_in != NULL) {
		kfree(para_in);
		para_in = NULL;
	}

	OUT_FUNCTION;

	return ret;
}

static int get_user_data(struct da_combine_param_io_buf *krn_param,
	struct misc_io_sync_param *param, uintptr_t arg)
{
	int ret;

	IN_FUNCTION;

	if (copy_from_user(param, (void __user *)arg, sizeof(*param))) {
		AUDIO_LOGE("copy from user fail");
		return -EFAULT;
	}

	ret = get_output_param(param->out_size,
		INT_TO_ADDR(param->out_l, param->out_h),
		&krn_param->buf_size_out, (void **)&krn_param->buf_out);
	if (ret != 0) {
		AUDIO_LOGE("alloc output buffer failed");
		return ret;
	}

	ret = get_input_param(param->in_size,
		INT_TO_ADDR(param->in_l, param->in_h),
		&(krn_param->buf_size_in), (void **)&krn_param->buf_in);
	if (ret != 0) {
		AUDIO_LOGE("get input param ret: %d", ret);
		return ret;
	}

	return 0;
}

static int check_msgid(const unsigned char *buf_in)
{
	unsigned short msg_id = *(unsigned short *)buf_in;

	if ((msg_id != ID_AP_IMGAE_DOWNLOAD && msg_id != ID_AP_DSP_UARTMODE) &&
		(da_combine_get_dsp_poweron_state() == HIFI_STATE_UNINIT)) {
		AUDIO_LOGE("dsp firmware not load,cmd:%d not send", msg_id);
		return -EFAULT;
	}

	return 0;
}

static int process_cmd(struct da_combine_param_io_buf *krn_param)
{
	cmd_process_func func = NULL;
	int ret;

	func = select_cmd_func(krn_param->buf_in, cmd_func_map,
		ARRAY_SIZE(cmd_func_map));
	if (func == NULL) {
		AUDIO_LOGE("select_func error");
		return -EINVAL;
	}

	ret = func(krn_param);
	if (ret != 0) {
		/* don't print err if redundant cmd was received */
		if (ret != -EAGAIN)
			AUDIO_LOGE("func process error");

		return ret;
	}

	/* write result to out buf */
	if (krn_param->buf_size_out >= sizeof(int)) {
		*(int *)krn_param->buf_out = ret;
	} else {
		AUDIO_LOGE("not enough space to save result");
		return -EINVAL;
	}

	return 0;
}

static int process_sync_cmd(uintptr_t arg)
{
	int ret;
	struct misc_io_sync_param param;
	struct da_combine_param_io_buf krn_param;

	IN_FUNCTION;

	memset(&param, 0, sizeof(param)); /* unsafe_function_ignore: memset */
	memset(&krn_param, 0, sizeof(krn_param)); /* unsafe_function_ignore: memset */

	ret = get_user_data(&krn_param, &param, arg);
	if (ret != 0) {
		AUDIO_LOGE("sync cmd get user data err, ret: %d", ret);
		goto end;
	}

	ret = check_msgid(krn_param.buf_in);
	if (ret != 0) {
		AUDIO_LOGE("sync cmd check msgid err");
		goto end;
	}

	ret = process_cmd(&krn_param);
	if (ret != 0) {
		AUDIO_LOGE("sync data proc err");
		goto end;
	}

	ret = put_user_data(param.out_size,
		INT_TO_ADDR(param.out_l, param.out_h),
		krn_param.buf_size_out, (void *)krn_param.buf_out);
	if (ret != 0) {
		AUDIO_LOGE("sync cmd send result2user err");
		goto end;
	}

end:
	if (krn_param.buf_in != NULL) {
		kfree(krn_param.buf_in);
		krn_param.buf_in = NULL;
	}

	if (krn_param.buf_out != NULL) {
		kfree(krn_param.buf_out);
		krn_param.buf_out = NULL;
	}

	OUT_FUNCTION;

	return ret;
}


static int resmgr_notifier(struct notifier_block *this,
	unsigned long event, void *ptr)
{
	struct pll_switch_event *switch_event = (struct pll_switch_event *)ptr;
	enum pll_state pll_state = PLL_RST;

	show_codec_chip_info();

	AUDIO_LOGI("event:%lu, from:%d, to:%d", event, switch_event->from,
		switch_event->to);

	switch (switch_event->to) {
	case PLL_HIGH:
		pll_state = PLL_HIGH_FREQ;
		break;
	case PLL_LOW:
		pll_state = PLL_LOW_FREQ;
		break;
	case PLL_NONE:
		pll_state = PLL_PD;
		break;
	default:
		AUDIO_LOGE("unsupport pll state: %d", switch_event->to);
		return 0;
	}

	switch (event) {
	case PRE_PLL_SWITCH:
		notify_dsp_pause();
		break;
	case POST_PLL_SWITCH:
		notify_dsp_resume(pll_state);
		break;
	default:
		AUDIO_LOGE("err pll swtich event: %lu", event);
		break;
	}

	return 0;
}

static int misc_irq_init(void)
{
	int ret;

	ret = da_combine_irq_request_irq(dsp_priv->irq,
		dsp_priv->dsp_config.vld_irq_num, msg_irq_handler,
		"cmd_valid", dsp_priv);
	if (ret < 0) {
		AUDIO_LOGE("request msg irq failed");
		return ret;
	}

	ret = da_combine_irq_request_irq(dsp_priv->irq,
		dsp_priv->dsp_config.wtd_irq_num, wtdog_irq_handler,
		"wd_irq", dsp_priv);
	if (ret < 0) {
		AUDIO_LOGE("request wtd irq failed");
		da_combine_irq_free_irq(dsp_priv->irq,
			dsp_priv->dsp_config.vld_irq_num, dsp_priv);
		return ret;
	}

	return ret;
}

static void misc_irq_deinit(void)
{
	if (dsp_priv->irq != NULL) {
		da_combine_irq_free_irq(dsp_priv->irq,
			dsp_priv->dsp_config.vld_irq_num, dsp_priv);
		da_combine_irq_free_irq(dsp_priv->irq,
			dsp_priv->dsp_config.wtd_irq_num, dsp_priv);

		dsp_priv->irq = NULL;
	}
}

static int misc_workqueue_init(void)
{
	dsp_priv->msg_proc_wq = create_singlethread_workqueue("msg_proc_wq");
	if (dsp_priv->msg_proc_wq == NULL) {
		AUDIO_LOGE("workqueue create failed");
		return -ENOENT;
	}
	INIT_DELAYED_WORK(&dsp_priv->msg_proc_work, dsp_msg_proc_work);

	return 0;
}

static void misc_workqueue_deinit(void)
{
	if (dsp_priv->msg_proc_wq != NULL) {
		cancel_delayed_work(&dsp_priv->msg_proc_work);
		flush_workqueue(dsp_priv->msg_proc_wq);
		destroy_workqueue(dsp_priv->msg_proc_wq);
		dsp_priv->msg_proc_wq = NULL;
	}
}

static int misc_init(const struct da_combine_dsp_config *dsp_config,
	struct snd_soc_component *codec, struct da_combine_resmgr *resmgr,
	struct da_combine_irq *irqmgr)
{
	int ret;

	dsp_priv->irq = irqmgr;
	dsp_priv->resmgr = resmgr;
	dsp_priv->codec = codec;
	dsp_priv->is_watchdog_coming = false;
	dsp_priv->is_sync_write_timeout = false;
	dsp_priv->dsp_pllswitch_done = 0;
	dsp_priv->sync_msg_ret = 0;

	memcpy(&dsp_priv->dsp_config, dsp_config, sizeof(*dsp_config)); /* unsafe_function_ignore: memcpy */

	mutex_init(&dsp_priv->msg_mutex);
	mutex_init(&dsp_priv->state_mutex);

	init_waitqueue_head(&dsp_priv->dsp_pllswitch_wq);
	init_waitqueue_head(&dsp_priv->sync_msg_wq);

	dsp_priv->resmgr_nb.notifier_call = resmgr_notifier;
	dsp_priv->resmgr_nb.priority = -1;
	ret = da_combine_resmgr_register_notifier(dsp_priv->resmgr, &dsp_priv->resmgr_nb);
	if (ret != 0) {
		AUDIO_LOGE("register resmgr notifier call init err, ret %d", ret);
		goto register_notifier_call_err;
	}

	ret = misc_irq_init();
	if (ret != 0) {
		AUDIO_LOGE("misc irq init err, ret %d", ret);
		goto misc_irq_init_err;
	}

	ret = misc_workqueue_init();
	if (ret != 0) {
		AUDIO_LOGE("misc workqueue init err, ret %d", ret);
		goto misc_workqueue_init_err;
	}

	return 0;

misc_workqueue_init_err:
	misc_irq_deinit();
misc_irq_init_err:
	da_combine_resmgr_unregister_notifier(dsp_priv->resmgr,
		&dsp_priv->resmgr_nb);
register_notifier_call_err:
	mutex_destroy(&dsp_priv->state_mutex);

	if (dsp_priv->dsp_config.dsp_ops.deinit)
		dsp_priv->dsp_config.dsp_ops.deinit();

	return ret;
}

static void misc_deinit(void)
{
	misc_workqueue_deinit();
	misc_irq_deinit();
	da_combine_resmgr_unregister_notifier(dsp_priv->resmgr, &dsp_priv->resmgr_nb);

	mutex_destroy(&dsp_priv->msg_mutex);
	mutex_destroy(&dsp_priv->state_mutex);

	if (dsp_priv->dsp_config.dsp_ops.deinit)
		dsp_priv->dsp_config.dsp_ops.deinit();
}


bool da_combine_get_sample_rate_index(unsigned int sample_rate,
	unsigned char *index)
{
	unsigned int i;

	if (index == NULL) {
		AUDIO_LOGE("index address is null");
		return false;
	}

	if (sample_rate == 0) {
		AUDIO_LOGI("data hook process, sample rate is 0");
		return true;
	}

	for (i = 0; i < ARRAY_SIZE(sample_rate_index_table); i++) {
		if (sample_rate == sample_rate_index_table[i].sample_rate) {
			*index = sample_rate_index_table[i].index;
			return true;
		}
	}

	/* shouldn't be here */
	AUDIO_LOGE("unsupport sample rate %d", sample_rate);

	return false;
}

void da_combine_soundtrigger_close_codec_dma(void)
{
	struct fast_trans_cfg_req_stru fastcfg = {0};

	IN_FUNCTION;

	AUDIO_LOGI("soundtrigger exception, release dma");

	fastcfg.msg_id = ID_AP_DSP_FASTTRANS_CLOSE;
	if (dsp_priv != NULL && dsp_priv->dsp_config.dsp_ops.set_fasttrans_enable)
		dsp_priv->dsp_config.dsp_ops.set_fasttrans_enable(false);

	if (da_combine_sync_write(&fastcfg, sizeof(fastcfg)) != 0)
		AUDIO_LOGE("sync write failed");

	OUT_FUNCTION;
}

bool da_combine_check_i2s2_clk(void)
{
	if (dsp_priv == NULL)
		return false;

	if (dsp_priv->dsp_config.dsp_ops.check_i2s2_clk)
		return dsp_priv->dsp_config.dsp_ops.check_i2s2_clk();

	return false;
}

int da_combine_sync_write(const void *arg, const unsigned int len)
{
	int ret = 0;
	int count = DA_COMBINE_EXCEPTION_RETRY;

	IN_FUNCTION;

	/* can't get codec version,reset system */
	if (da_combine_error_detect())
		rdr_system_error(RDR_AUDIO_CODEC_ERR_MODID, 0, 0);

	if (!request_state_mutex()) {
		AUDIO_LOGE("state mutex not release");
		return -EBUSY;
	}

	dsp_priv->sync_msg_ret = 0;

	da_combine_resmgr_pm_get_clk();

	while (count) {
		if (dsp_priv->is_watchdog_coming) {
			AUDIO_LOGE("watchdog have already come, can't send msg");
			ret = -EBUSY;
			goto out;
		}
		ret = sync_write_msg(arg, len);
		if (ret != 0) {
			AUDIO_LOGE("send msg failed");
			goto out;
		}
		ret = wait_sync_msg_result();
		if (ret == 0) {
			goto out;
		} else {
			count--;
			AUDIO_LOGE("cmd is interrupted, retry %d times, ret: %d",
				DA_COMBINE_EXCEPTION_RETRY - count, ret);
		}
		AUDIO_LOGE("CMD_STAT:0x%x, CMD_STAT_VLD:0x%x",
			da_combine_dsp_read_reg(DA_COMBINE_DSP_SUB_CMD_STATUS),
			da_combine_dsp_read_reg(DA_COMBINE_DSP_SUB_CMD_STATUS_VLD));
	}
	if (count == 0)
		ret = (check_sync_write_status() == 0) ? ret : -EBUSY;

out:
	da_combine_resmgr_pm_put_clk();
	release_state_mutex();
	OUT_FUNCTION;

	return ret;
}

bool da_combine_error_detect(void)
{
	unsigned int version;
	unsigned int retry_count = 0;

	while (retry_count < VERSION_DETECT_RETRY_COUNT) {
		version = da_combine_dsp_read_reg(DA_COMBINE_VERSION_REG);
		if (version != DA_COMBINE_VERSION_CS &&
			version != DA_COMBINE_VERSION_ES) {
			AUDIO_LOGE("err, ver 0x%x, pmu mclk status:0x%x",
				version, hi_cdcctrl_get_pmu_mclk_status());
		} else {
			return false;
		}

		udelay(1000);
		retry_count++;
	}


	return true;
}

void da_combine_dsp_state_lock(void)
{
	if (dsp_priv != NULL)
		mutex_lock(&dsp_priv->state_mutex);
}

void da_combine_dsp_state_unlock(void)
{
	if (dsp_priv != NULL)
		mutex_unlock(&dsp_priv->state_mutex);
}

void da_combine_wtdog_send_event(void)
{
	char *envp[2] = {"codechifi_watchdog", NULL};

	AUDIO_LOGE("now reset mediaserver");
	if (dsp_priv != NULL)
		kobject_uevent_env(&dsp_priv->irq->dev->kobj, KOBJ_CHANGE, envp);
}

void da_combine_wtdog_process(void)
{
	/* stop codec om asp dma */
	da_combine_stop_hook_route();

	/* stop soundtrigger asp dma */
	da_combine_soundtrigger_dma_close();

#ifdef CONFIG_HISI_HIFI_BB
	rdr_codec_dsp_watchdog_process();
#endif
}

void da_combine_clr_cmd_status_bit(uint8_t cmd)
{
	int count = DA_COMBINE_EXCEPTION_RETRY;
	uint32_t status;

	da_combine_dsp_reg_clr_bit(DA_COMBINE_DSP_SUB_CMD_STATUS, cmd);
	status = da_combine_dsp_read_reg(DA_COMBINE_DSP_SUB_CMD_STATUS);

	while (count) {
		if (((status >> cmd) & 1) == 0) {
			if (count != DA_COMBINE_EXCEPTION_RETRY)
				da_combine_save_log();
			return;
		}

		count--;
		da_combine_dsp_reg_clr_bit(DA_COMBINE_DSP_SUB_CMD_STATUS, cmd);
		status = da_combine_dsp_read_reg(DA_COMBINE_DSP_SUB_CMD_STATUS);
		AUDIO_LOGW("cmd status:%d, retry %d", status,
			DA_COMBINE_EXCEPTION_RETRY - count);
	}

	da_combine_save_log();
	da_combine_wtdog_send_event();
}

bool da_combine_get_wtdog_state(void)
{
	if (dsp_priv == NULL)
		return false;

	return dsp_priv->is_watchdog_coming;
}

void da_combine_set_wtdog_state(bool state)
{
	if (dsp_priv == NULL)
		return;

	dsp_priv->is_watchdog_coming = state;
}

bool da_combine_get_sync_write_state(void)
{
	if (dsp_priv == NULL)
		return false;

	return dsp_priv->is_sync_write_timeout;
}

void da_combine_set_sync_write_state(bool state)
{
	if (dsp_priv == NULL)
		return;

	dsp_priv->is_sync_write_timeout = state;
}

unsigned int da_combine_get_high_freq_status(void)
{
	if (dsp_priv != NULL)
		return dsp_priv->high_freq_status.val;

	return 0;
}

void da_combine_set_high_freq_status(unsigned int scene_id,
	bool enable)
{
	if (dsp_priv == NULL)
		return;

	if (enable)
		dsp_priv->high_freq_status.val |= (1 << scene_id);
	else
		dsp_priv->high_freq_status.val &= ~(1 << scene_id);
}

unsigned int da_combine_get_low_freq_status(void)
{
	if (dsp_priv != NULL)
		return dsp_priv->low_freq_status.val;

	return 0;
}

void da_combine_set_low_freq_status(unsigned int scene_id,
	bool enable)
{
	if (dsp_priv == NULL)
		return;

	if (enable)
		dsp_priv->low_freq_status.val |= (1 << scene_id);
	else
		dsp_priv->low_freq_status.val &= ~(1 << scene_id);
}

enum pll_state da_combine_get_pll_state(void)
{
	if (dsp_priv != NULL)
		return dsp_priv->pll_state;

	return 0;
}

static int da_combine_dsp_misc_open(struct inode *finode, struct file *fd)
{
	return 0;
}

static int da_combine_dsp_misc_release(struct inode *finode, struct file *fd)
{
	return 0;
}

static long da_combine_dsp_misc_ioctl(struct file *fd, unsigned int cmd,
	unsigned long arg)
{
	long ret = 0;

	if ((void __user *)(uintptr_t)arg == NULL) {
		AUDIO_LOGE("input error: arg is NULL");
		return -EINVAL;
	}

	mutex_lock(&dsp_priv->msg_mutex);
	switch (cmd) {
	case DA_COMBINE_IOCTL_SYNCMSG:
		AUDIO_LOGD("ioctl: HIFI_MISC_IOCTL_SYNCMSG");
		ret = process_sync_cmd(arg);
		break;
	case DA_COMBINE_IOCTL_DMESG_DUMP_LOG:
		AUDIO_LOGD("ioctl: DA_COMBINE_IOCTL_DMESG_DUMP_LOG");
		da_combine_set_only_printed_enable(true);
		da_combine_dsp_dump_no_path();
		break;
	default:
		AUDIO_LOGE("ioctl: invalid cmd: 0x%x", cmd);
		ret = -EINVAL;
		break;
	}
	mutex_unlock(&dsp_priv->msg_mutex);

	AUDIO_LOGI("ioctl: ret %ld", ret);

	return ret;
}

static long da_combine_dsp_misc_ioctl32(struct file *fd,
	unsigned int cmd, unsigned long arg)
{
	void __user *user_ptr = (void __user *)compat_ptr(arg);

	return da_combine_dsp_misc_ioctl(fd, cmd, (uintptr_t)user_ptr);
}

static const struct file_operations da_combine_dsp_misc_fops = {
	.owner = THIS_MODULE,
	.open = da_combine_dsp_misc_open,
	.release = da_combine_dsp_misc_release,
	.unlocked_ioctl = da_combine_dsp_misc_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = da_combine_dsp_misc_ioctl32,
#endif
};

static struct miscdevice da_combine_dsp_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "hi6402_hifi_misc",
	.fops = &da_combine_dsp_misc_fops,
};

int da_combine_dsp_misc_suspend(void)
{
	int ret = 0;

	AUDIO_LOGI("suspend+");

	/* mad */
	if ((dsp_priv != NULL) && (dsp_priv->pll_state == PLL_LOW_FREQ) &&
		da_combine_get_dsp_state()) {
		if (dsp_priv->dsp_config.dsp_ops.suspend)
			ret = dsp_priv->dsp_config.dsp_ops.suspend();
	}

	AUDIO_LOGI("suspend-");

	return ret;
}

int da_combine_dsp_misc_resume(void)
{
	int ret = 0;

	AUDIO_LOGI("resume+");

	if ((dsp_priv != NULL) && (dsp_priv->pll_state == PLL_LOW_FREQ) &&
		da_combine_get_dsp_state()) {
		if (dsp_priv->dsp_config.dsp_ops.resume)
			ret = dsp_priv->dsp_config.dsp_ops.resume();
	}

	AUDIO_LOGI("resume-");

	return ret;
}

int da_combine_dsp_misc_init(struct snd_soc_component *codec,
	struct da_combine_resmgr *resmgr, struct da_combine_irq *irqmgr,
	const struct da_combine_dsp_config *dsp_config)
{
	int ret;

	IN_FUNCTION;

	dsp_priv = kzalloc(sizeof(*dsp_priv), GFP_KERNEL);
	if (dsp_priv == NULL) {
		AUDIO_LOGE("dsp priv kzalloc error!");
		return -ENOMEM;
	}

	ret = misc_register(&da_combine_dsp_misc_device);
	if (ret != 0) {
		AUDIO_LOGE("misc register failed, ret %d", ret);
		ret = -EBUSY;
		goto misc_register_err_exit;
	}

	ret = misc_init(dsp_config, codec, resmgr, irqmgr);
	if (ret != 0) {
		AUDIO_LOGE("misc priv init err, ret %d", ret);
		goto priv_init_err;
	}

	ret = da_combine_dsp_utils_init(dsp_config, codec, resmgr);
	if (ret != 0) {
		AUDIO_LOGE("utils init err, ret %d", ret);
		goto utils_init_err;
	}

	ret = da_combine_dsp_om_init(dsp_config, irqmgr);
	if (ret != 0) {
		AUDIO_LOGE("misc om init err, ret %d", ret);
		goto om_init_err;
	}

	OUT_FUNCTION;

	return ret;

om_init_err:
	da_combine_dsp_utils_deinit();
utils_init_err:
	misc_deinit();
priv_init_err:
	(void)misc_deregister(&da_combine_dsp_misc_device);
misc_register_err_exit:
	kfree(dsp_priv);
	dsp_priv = NULL;

	return ret;
}

void da_combine_dsp_misc_deinit(void)
{
	IN_FUNCTION;

	if (dsp_priv == NULL)
		return;

	da_combine_dsp_om_deinit();
	da_combine_dsp_utils_deinit();
	misc_deinit();

	(void)misc_deregister(&da_combine_dsp_misc_device);
	kfree(dsp_priv);
	dsp_priv = NULL;

	OUT_FUNCTION;
}

