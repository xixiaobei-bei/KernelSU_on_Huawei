/*
 * om_hook.c
 *
 * hook module for da_combine codec
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

#include "om_hook.h"

#include <linux/err.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/sched/rt.h>
#include <linux/pm_wakeup.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/rtc.h>
#include <linux/time.h>
#include <linux/stat.h>
#include <linux/version.h>
#include <linux/types.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <uapi/linux/sched/types.h>
#endif
#include <linux/hisi/da_combine/da_combine_v3_dsp_regs.h>
#include <linux/hisi/rdr_pub.h>
#include "linux/hisi/audio_log.h"
#include <da_combine_dsp_interface.h>

#include "slimbus.h"
#ifdef CONFIG_SND_SOC_DA_COMBINE_V5
#include "slimbus_da_combine_v5.h"
#endif
#include "da_combine_dsp_misc.h"
#include "download_image.h"
#include "audio_file.h"
#include "dsp_misc.h"
#include "dsp_utils.h"
#include "om_beta.h"

/*lint -e655 -e838 -e730*/
#define LOG_TAG "om_hook"

#define HOOK_PATH_VISUAL_TOOL "/data/log/codec_dsp/visual_tool/"
#define HOOK_PATH_DEFAULT "/data/log/codec_dsp/default/"
#define HOOK_RECORD_FILE "da_combine_dsp_hook.data"
#define VERIFY_FRAME_HEAD_MAGIC_NUM 0x5a5a5a5a
#define VERIFY_FRAME_BODY_MAGIC_NUM 0xa5a5a5a5
#define LEFT_DMA_ADDR (CODEC_DSP_OM_DMA_BUFFER_ADDR)
#define LEFT_DMA_SIZE (CODEC_DSP_OM_DMA_BUFFER_SIZE / 2)
#define LEFT_DMA_CFG_ADDR (CODEC_DSP_OM_DMA_CONFIG_ADDR)
#define LEFT_DMA_CFG_SIZE (CODEC_DSP_OM_DMA_CONFIG_SIZE / 2)
#define RIGHT_DMA_ADDR (LEFT_DMA_ADDR + LEFT_DMA_SIZE)
#define RIGHT_DMA_SIZE (LEFT_DMA_SIZE)
#define RIGHT_DMA_CFG_ADDR (LEFT_DMA_CFG_ADDR + LEFT_DMA_CFG_SIZE)
#define RIGHT_DMA_CFG_SIZE (LEFT_DMA_CFG_SIZE)
#define HOOK_AP_DMA_INTERVAL 20 /* ms */
#define HOOK_DSP_DMA_INTERVAL 5 /* ms */
#define HOOK_AP_DSP_DMA_TIMES ((HOOK_AP_DMA_INTERVAL) / (HOOK_DSP_DMA_INTERVAL))
#define HOOK_RECORD_MAX_LENGTH 200
#define HOOK_FILE_NAME_MAX_LENGTH 100
#define HOOK_VISUAL_TOOL_MAX_FILE_SIZE (192 * 1024)
#define HOOK_MAX_FILE_SIZE 0x40000000 /* 1G */
#define HOOK_MAX_DIR_SIZE 0x100000000 /* 4G */
#define HOOK_DEFAULT_SUBDIR_CNT 20
#define HOOK_MAX_SUBDIR_CNT 5000
#define PRINT_MAX_CNT 3
#define HOOK_THREAD_PRIO 20
#define ROOT_UID 0
#define SYSTEM_GID 1000
#define MAX_PARA_SIZE 4096
#define MAGIC_NUM_LOCATION 1
#define INFO_MSG_LOCATION 2

enum {
	VERIFY_DEFAULT,
	VERIFY_START,
	VERIFY_ADJUSTING,
	VERIFY_ADJUSTED,
	VERIFY_END,
};

enum {
	PCM_SWAP_BUFF_A = 0,
	PCM_SWAP_BUFF_B,
	PCM_SWAP_BUFF_CNT,
};

enum {
	HOOK_LEFT = 0,
	HOOK_RIGHT,
	HOOK_CNT,
};

enum hook_status {
	HOOK_VALID = 0,
	HOOK_POS_UNSUPPORT,
	HOOK_CONFIG_ERR,
	HOOK_BANDWIDTH_LIMIT,
	HOOK_DUPLICATE_POS,
	HOOK_IF_BUSY,
	HOOK_MULTI_SCENE,
	HOOK_STATUS_BUTT,
};

struct hook_pos_name {
	enum hook_pos pos;
	char *name;
};

struct linux_dirent {
	unsigned long d_ino;
	unsigned long d_off;
	unsigned short d_reclen;
	char d_name[1];
};

struct data_cfg {
	unsigned int sample_rate;
	unsigned short resolution;
	unsigned short channels;
};

struct pos_info {
	enum hook_pos pos;
	struct data_cfg config;
	unsigned int size;
	enum hook_status hook_status;
};

struct pos_infos {
	struct list_head node;
	struct pos_info info;
};

struct data_flow {
	struct list_head node;
	void *addr;
};

struct hook_dma_cfg {
	unsigned int port; /* slimbus port address */
	unsigned int config; /* dma config number */
	unsigned short channel;	/* dma channel number */
};

struct hook_runtime {
	void __iomem *lli_cfg[PCM_SWAP_BUFF_CNT]; /* must be aligned to 32byte */
	void *lli_cfg_phy[PCM_SWAP_BUFF_CNT]; /* lli cfg, physical addr */
	void __iomem *buffer[PCM_SWAP_BUFF_CNT]; /* dma buffer */
	void *buffer_phy[PCM_SWAP_BUFF_CNT]; /* dma buffer, physical addr */
	struct semaphore hook_proc_sema;
	struct semaphore hook_stop_sema;
	struct wakeup_source wake_lock;
	struct task_struct *kthread;
	struct pos_infos info_list;
	struct data_flow data_list;
	unsigned int size; /* dma buffer size */
	unsigned short channel; /* dma channel number */
	unsigned int kthread_should_stop;
	unsigned int verify_state;
	unsigned int hook_id;
	unsigned int idle_id;
	unsigned int hook_file_size;
	unsigned int hook_file_cnt;
	bool parsed;
};

struct da_combine_hook_priv {
	struct clk *asp_subsys_clk;
	struct hook_runtime runtime[HOOK_CNT];
	char hook_path[HOOK_PATH_MAX_LENGTH];
	bool standby;
	bool started;
	bool is_eng;
	bool should_deactive_slimbus;
	bool is_dspif_hooking;
	unsigned short bandwidth;
	unsigned short sponsor;
	unsigned int dir_count;
	unsigned int codec_type;
	unsigned short open_file_failed_cnt;
	unsigned int slimbus_track_type;
	enum slimbus_device slimbus_device_type;
};

struct hook_dma_addr {
	unsigned int dma_addr;
	unsigned int dma_size;
	unsigned int dma_config_addr;
	unsigned int dma_config_size;
};


static const struct hook_pos_name pos_name[] = {
	{ HOOK_POS_IF0,              "IF0" },
	{ HOOK_POS_IF1,              "IF1" },
	{ HOOK_POS_IF2,              "IF2" },
	{ HOOK_POS_IF3,              "IF3" },
	{ HOOK_POS_IF4,              "IF4" },
	{ HOOK_POS_IF5,              "IF5" },
	{ HOOK_POS_IF6,              "IF6" },
	{ HOOK_POS_IF7,              "IF7" },
	{ HOOK_POS_IF8,              "IF8" },
	{ HOOK_POS_ANC_CORE,         "ANC_CORE" },
	{ HOOK_POS_LOG,              "LOG" },
	{ HOOK_POS_ANC_PCM_RX_VOICE, "ANC_PCM_RX_VOICE" },
	{ HOOK_POS_ANC_PCM_REF,      "ANC_PCM_REF" },
	{ HOOK_POS_ANC_PCM_ERR,      "ANC_PCM_ERR" },
	{ HOOK_POS_ANC_PCM_ANTI,     "ANC_PCM_ANTI" },
	{ HOOK_POS_ANC_BETA_CSINFO,  "ANC_BETA_CSINFO" },
	{ HOOK_POS_ANC_PCM_TX_VOICE, "ANC_PCM_TX_VOICE" },
	{ HOOK_POS_ANC_ALG_INDICATE, "ANC_ALG_INDICATE" },
	{ HOOK_POS_ANC_COEF,         "ANC_COEF" },
	{ HOOK_POS_PA_EFFECTIN,      "PA_EFFECTIN" },
	{ HOOK_POS_PA_EFFECTOUT,     "PA_EFFECTOUT" },
	{ HOOK_POS_WAKEUP_MICIN,     "WAKEUP_MICIN" },
	{ HOOK_POS_WAKEUP_EFFECTOUT, "WAKEUP_EFFECTOUT" },
	{ HOOK_POS_PA1_I,            "PA1_I" },
	{ HOOK_POS_PA1_V,            "PA1_V" },
	{ HOOK_POS_PA2_I,            "PA2_I" },
	{ HOOK_POS_PA2_V,            "PA2_V" },
	{ HOOK_POS_ULTRASONIC,       "ULTRASONIC" },
};

static struct da_combine_hook_priv *hook_priv;

static unsigned int get_idle_buffer_id(const struct hook_runtime *runtime)
{
	unsigned int used_dma_addr;
	unsigned int src_addr_a;

	src_addr_a =
		(unsigned int)((uintptr_t)runtime->buffer_phy[PCM_SWAP_BUFF_A]);

	used_dma_addr = asp_dma_get_des(runtime->channel);
	if (used_dma_addr == src_addr_a)
		return PCM_SWAP_BUFF_B;
	else
		return PCM_SWAP_BUFF_A;
}

static void deactivate_slimbus(void)
{
	int ret = 0;

	if (hook_priv->should_deactive_slimbus) {
		ret = slimbus_deactivate_track(hook_priv->slimbus_device_type,
			hook_priv->slimbus_track_type, NULL);
		if (ret)
			AUDIO_LOGW("deactivate debug return ret %d", ret);
	}

	clk_disable_unprepare(hook_priv->asp_subsys_clk);
}

/* stop hook dma and release hook data buffer */
static void stop_hook(unsigned int hook_id)
{
	int ret;
	struct hook_runtime *runtime = NULL;
	struct pos_infos *pos_infos = NULL;
	struct da_combine_hook_priv *priv = hook_priv;

	runtime = &priv->runtime[hook_id];

	runtime->hook_file_cnt  = 0;
	runtime->hook_file_size = 0;

	asp_dma_stop(runtime->channel);

	ret = down_interruptible(&runtime->hook_stop_sema);
	if (ret == -ETIME)
		AUDIO_LOGE("down interruptible error -ETIME");

	iounmap(runtime->buffer[PCM_SWAP_BUFF_A]);
	iounmap(runtime->lli_cfg[PCM_SWAP_BUFF_A]);

	while (!list_empty(&runtime->info_list.node)) {
		pos_infos = list_entry(runtime->info_list.node.next,
			struct pos_infos, node);
		list_del(&pos_infos->node);
		kfree(pos_infos);
	}

	priv->started = false;
	priv->standby = true;

	up(&runtime->hook_stop_sema);
}


static int check_param_and_scene(const struct da_combine_param_io_buf *param,
	unsigned int in_size)
{
	int ret;
	union da_combine_high_freq_status high_status;

	ret = da_combine_check_msg_para(param, in_size);
	if (ret != 0)
		return ret;

	if (param->buf_size_in != in_size) {
		AUDIO_LOGE("err param size:%d!", param->buf_size_in);
		return -EINVAL;
	}

	high_status.val = da_combine_get_high_freq_status();
	if (high_status.om_hook) {
		AUDIO_LOGW("om hook is running");
		return -EINVAL;
	}

	return 0;
}

static const char *get_pos_name(enum hook_pos pos)
{
	unsigned int i;
	unsigned int array_size;

	array_size = ARRAY_SIZE(pos_name);
	for (i = 0; i < array_size; i++) {
		if (pos_name[i].pos == pos)
			return pos_name[i].name;
	}

	return "UNKNOW";
}

static void dump_dsp_data_for_tool(enum hook_pos pos, const void *data,
	unsigned int size, unsigned int hook_id)
{
	char path[HOOK_PATH_MAX_LENGTH + HOOK_FILE_NAME_MAX_LENGTH] = {0};
	char new_path[HOOK_PATH_MAX_LENGTH + HOOK_FILE_NAME_MAX_LENGTH] = {0};
	struct da_combine_hook_priv *priv = hook_priv;
	struct hook_runtime *runtime = NULL;

	if (pos == HOOK_POS_LOG)
		return;

	if (hook_id == HOOK_LEFT) {
		runtime = &priv->runtime[HOOK_LEFT];
		snprintf(path, sizeof(path) - 1, "%s%s_L_WRITING.data",
			priv->hook_path, get_pos_name(pos));
		snprintf(new_path, sizeof(new_path) - 1, "%s%s_L_%d.data",
			priv->hook_path, get_pos_name(pos),
			runtime->hook_file_cnt);
	} else {
		runtime = &priv->runtime[HOOK_RIGHT];
		snprintf(path, sizeof(path) - 1, "%s%s_R_WRITING.data",
			priv->hook_path, get_pos_name(pos));
		snprintf(new_path, sizeof(new_path) - 1, "%s%s_R_%d.data",
			priv->hook_path, get_pos_name(pos),
			runtime->hook_file_cnt);
	}

	da_combine_dsp_dump_to_file(data, size, path);

	runtime->hook_file_size += size;

	if (runtime->hook_file_size >= HOOK_VISUAL_TOOL_MAX_FILE_SIZE) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
		(void)ksys_rename(path, new_path);
#else
		(void)sys_rename(path, new_path);
#endif
		runtime->hook_file_cnt++;
		runtime->hook_file_size = 0;
	}
}

static void dump_dsp_data(enum hook_pos pos, const void *data,
	unsigned int size, unsigned int hook_id)
{
	char path[HOOK_PATH_MAX_LENGTH + HOOK_FILE_NAME_MAX_LENGTH] = {0};
	struct da_combine_hook_priv *priv = hook_priv;

	if (pos == HOOK_POS_LOG)
		return;

	if (priv->sponsor == OM_SPONSOR_BETA) {
		da_combine_anc_beta_generate_path(pos, priv->hook_path,
			path, sizeof(path));
	} else {
		if (hook_id == HOOK_LEFT)
			snprintf(path, sizeof(path), "%s%s_L.data",
				priv->hook_path, get_pos_name(pos));
		else
			snprintf(path, sizeof(path), "%s%s_R.data",
				priv->hook_path, get_pos_name(pos));
	}
	da_combine_dsp_dump_to_file(data, size, path);
}

static void parse_dsp_data(const void *data, const struct hook_runtime *runtime)
{
	struct da_combine_hook_priv *priv = hook_priv;
	const struct pos_infos *pos_infos = NULL;
	const unsigned int *buffer = (unsigned int *)data;
	const unsigned int *dsp_buffer = NULL;
	unsigned int data_size = 0;
	unsigned int i;

	if (*buffer == VERIFY_FRAME_HEAD_MAGIC_NUM) {
		AUDIO_LOGW("throw away verify data");
		return;
	}

	/*
	 * data arrange:
	 * |dsp buffer1|dsp buffer2|dsp buffer3|dsp buffer4|
	 */
	for (i = 0; i < HOOK_AP_DSP_DMA_TIMES; i++) {
		dsp_buffer = buffer;

		list_for_each_entry(pos_infos, &runtime->info_list.node, node) {
			data_size = pos_infos->info.size;

			if (((char *)dsp_buffer + data_size) >
				((char *)data + runtime->size)) {
				AUDIO_LOGE("om buffer will overflow, forbid dump data");
				return;
			}

			if (priv->sponsor == OM_SPONSOR_TOOL)
				dump_dsp_data_for_tool(pos_infos->info.pos,
					dsp_buffer, data_size,
					runtime->hook_id);
			else
				dump_dsp_data(pos_infos->info.pos, dsp_buffer,
						data_size, runtime->hook_id);

			dsp_buffer += data_size / sizeof(unsigned int);
		}

		buffer += (runtime->size / HOOK_AP_DSP_DMA_TIMES) /
			sizeof(unsigned int);
	}
}

static int parse_pos_info(const void *data, struct hook_runtime *runtime)
{
	struct da_combine_hook_priv *priv = hook_priv;
	const struct pos_info *pos_info = NULL;
	struct pos_infos *pos_infos = NULL;
	unsigned int parsed_size = 0;
	const unsigned int *buffer = (unsigned int *)data;
	char log[HOOK_RECORD_MAX_LENGTH] = {0};
	char path[HOOK_PATH_MAX_LENGTH + HOOK_FILE_NAME_MAX_LENGTH] = {0};

	/*
	 * pos info arrange:
	 * |5a5a5a5a|a5a5a5a5|......|a5a5a5a5|
	 * |  head  |  body  | info |  body  |
	 */
	if (buffer[MAGIC_NUM_LOCATION] != VERIFY_FRAME_BODY_MAGIC_NUM) {
		AUDIO_LOGE("err pos info:%d", buffer[1]);
		return -EINVAL;
	}

	pos_info = (struct pos_info *) (&buffer[INFO_MSG_LOCATION]);
	while ((*(unsigned int *)pos_info != VERIFY_FRAME_BODY_MAGIC_NUM) &&
		(parsed_size <= runtime->size)) {
		if (pos_info->hook_status != HOOK_VALID ||
			(pos_info->size > (runtime->size / HOOK_AP_DSP_DMA_TIMES))) {
			AUDIO_LOGE("invalid hook pos:0x%x, size:%d, reason:%d",
				pos_info->pos, pos_info->size,
				pos_info->hook_status);
			snprintf(log, sizeof(log) - 1,
				"invalid hook pos:0x%x, size:%d, reason:%d\n",
				pos_info->pos, pos_info->size,
				pos_info->hook_status);
			snprintf(path, sizeof(path) - 1,
				"%s%s", priv->hook_path, HOOK_RECORD_FILE);
			da_combine_dsp_dump_to_file(log, strlen(log), path);
			pos_info++;
			parsed_size += sizeof(struct pos_info);
			break;
		}

		pos_infos = kzalloc(sizeof(struct pos_infos), GFP_KERNEL);
		if (pos_infos == NULL) {
			AUDIO_LOGE("malloc failed");
			break;
		}

		memcpy(&pos_infos->info, pos_info, sizeof(struct pos_info));

		AUDIO_LOGI("pos:0x%x, size:%d, state:%d, rate:%d",
			pos_infos->info.pos, pos_infos->info.size,
			pos_infos->info.hook_status,
			pos_infos->info.config.sample_rate);
		AUDIO_LOGI("resolution:%d, channels:%d",
			pos_infos->info.config.resolution,
			pos_infos->info.config.channels);

		list_add_tail(&pos_infos->node, &runtime->info_list.node);

		pos_info++;

		parsed_size += sizeof(struct pos_info);
	}

	return 0;
}

static unsigned int get_verify_pos(void *data, unsigned int size)
{
	unsigned int i;

	unsigned int *buffer = (unsigned int *)data;

	for (i = 0; i < size / sizeof(unsigned int); i++)
		if (buffer[i] == VERIFY_FRAME_HEAD_MAGIC_NUM)
			break;

	return i * sizeof(unsigned int);
}

static void verify_data(struct data_flow *data, struct hook_runtime *runtime)
{
	unsigned int pos;
	unsigned int id;

	if (runtime->verify_state == VERIFY_END)
		return;

	if (runtime->verify_state == VERIFY_DEFAULT) {
		pos = get_verify_pos(data->addr, runtime->size);
		if (pos == runtime->size)
			return;

		id = get_idle_buffer_id(runtime);
		if (pos)
			((struct dma_lli_cfg *)(runtime->lli_cfg[id]))->a_count = pos;

		AUDIO_LOGI("verify pos:%d", pos);
		runtime->verify_state = VERIFY_START;
		AUDIO_LOGI("verify start");
	} else if (runtime->verify_state == VERIFY_START) {
		((struct dma_lli_cfg *)(runtime->lli_cfg[PCM_SWAP_BUFF_A]))->a_count =
			runtime->size;
		((struct dma_lli_cfg *)(runtime->lli_cfg[PCM_SWAP_BUFF_B]))->a_count =
			runtime->size;
		runtime->verify_state = VERIFY_ADJUSTING;
		AUDIO_LOGI("verify adjusting");
	} else if (runtime->verify_state == VERIFY_ADJUSTING) {
		runtime->verify_state = VERIFY_ADJUSTED;
		AUDIO_LOGI("verify adjusted");
	} else {
		runtime->verify_state = VERIFY_END;
		AUDIO_LOGI("verify end");
	}
}

static void parse_data(struct hook_runtime *runtime)
{
	struct data_flow *data = NULL;

	if (list_empty(&runtime->data_list.node)) {
		AUDIO_LOGE("data list is empty");
		return;
	}
	data = list_entry(runtime->data_list.node.next, struct data_flow, node);

	verify_data(data, runtime);
	if (runtime->verify_state != VERIFY_END)
		goto free_list_node;

	if (!runtime->parsed) {
		if (!parse_pos_info(data->addr, runtime))
			runtime->parsed = true;
	} else {
		parse_dsp_data(data->addr, runtime);
	}

free_list_node:
	list_del(&data->node);
	kfree(data->addr);
	kfree(data);
}

static void hook_should_stop(void)
{
	enum slimbus_framer framer;

	framer = slimbus_debug_get_framer();
	if ((framer != SLIMBUS_FRAMER_CODEC) || da_combine_check_i2s2_clk())
		da_combine_stop_dsp_hook();
}

static int add_data_to_list(struct hook_runtime *runtime)
{
	int ret = 0;
	void *buffer = NULL;
	unsigned int id;
	struct data_flow *data = NULL;

	buffer = kzalloc(runtime->size, GFP_KERNEL);
	if (buffer == NULL) {
		AUDIO_LOGE("malloc failed");
		return -ENOMEM;
	}

	data = kzalloc(sizeof(struct data_flow), GFP_KERNEL);
	if (data == NULL) {
		AUDIO_LOGE("malloc failed");
		ret = -ENOMEM;
		goto err_exit;
	}

	id = get_idle_buffer_id(runtime);
	memcpy(buffer, runtime->buffer[id], runtime->size);

	if (runtime->idle_id != get_idle_buffer_id(runtime))
		AUDIO_LOGW("dma buffer is changed");

	data->addr = buffer;

	list_add_tail(&data->node, &runtime->data_list.node);

	return 0;

err_exit:
	kfree(buffer);

	return ret;
}

static int left_data_parse_thread(void *p)
{
	int ret;
	struct da_combine_hook_priv *priv = hook_priv;
	struct hook_runtime *runtime = NULL;

	runtime = &priv->runtime[HOOK_LEFT];

	while (!runtime->kthread_should_stop) {
		ret = down_interruptible(&runtime->hook_proc_sema);
		if (ret == -ETIME)
			AUDIO_LOGE("proc sema down int err -ETIME");

		ret = down_interruptible(&runtime->hook_stop_sema);
		if (ret == -ETIME)
			AUDIO_LOGE("stop sema down int err -ETIME");

		if (!priv->started || priv->standby) {
			up(&runtime->hook_stop_sema);
			AUDIO_LOGE("parse data when hook stopped");
			continue;
		}

		__pm_stay_awake(&runtime->wake_lock);

		add_data_to_list(runtime);
		parse_data(runtime);
		if (runtime->idle_id != get_idle_buffer_id(runtime))
			AUDIO_LOGW("dma buffer is changed");

		__pm_relax(&runtime->wake_lock);

		up(&runtime->hook_stop_sema);

		hook_should_stop();
	}

	return 0;
}

static int right_data_parse_thread(void *p)
{
	int ret;
	struct da_combine_hook_priv *priv = hook_priv;
	struct hook_runtime *runtime = NULL;

	runtime = &priv->runtime[HOOK_RIGHT];

	while (!runtime->kthread_should_stop) {
		ret = down_interruptible(&runtime->hook_proc_sema);
		if (ret == -ETIME)
			AUDIO_LOGE("proc sema down int err -ETIME");

		ret = down_interruptible(&runtime->hook_stop_sema);
		if (ret == -ETIME)
			AUDIO_LOGE("stop sema down int err -ETIME");

		if (!priv->started || priv->standby) {
			up(&runtime->hook_stop_sema);
			AUDIO_LOGE("parse data when hook stopped");
			continue;
		}

		__pm_stay_awake(&runtime->wake_lock);

		add_data_to_list(runtime);
		parse_data(runtime);
		if (runtime->idle_id != get_idle_buffer_id(runtime))
			AUDIO_LOGW("dma buffer is changed");

		__pm_relax(&runtime->wake_lock);

		up(&runtime->hook_stop_sema);
	}

	return 0;
}

static int init_para(unsigned int codec_type)
{
	strncpy(hook_priv->hook_path, HOOK_PATH_DEFAULT,
		sizeof(hook_priv->hook_path));
	hook_priv->sponsor = OM_SPONSOR_DEFAULT;
	hook_priv->bandwidth = OM_BANDWIDTH_6144;
	hook_priv->dir_count = HOOK_DEFAULT_SUBDIR_CNT;
	hook_priv->codec_type = codec_type;

	if (codec_type == DA_COMBINE_CODEC_TYPE_V3) {
		hook_priv->slimbus_device_type = SLIMBUS_DEVICE_DA_COMBINE_V3;
		hook_priv->slimbus_track_type =
			(unsigned int)SLIMBUS_TRACK_DEBUG;
	}
#ifdef CONFIG_SND_SOC_DA_COMBINE_V5
	else if (codec_type == DA_COMBINE_CODEC_TYPE_V5) {
		hook_priv->slimbus_device_type = SLIMBUS_DEVICE_DA_COMBINE_V5;
		hook_priv->slimbus_track_type =
			(unsigned int)SLIMBUS_DA_COMBINE_V5_TRACK_DEBUG;
	}
#endif
	else {
		AUDIO_LOGE("do not support type %d", codec_type);
		WARN_ON(true);
		return -EINVAL;
	}

	hook_priv->is_eng = false;
	hook_priv->started = false;
	hook_priv->standby = true;
	hook_priv->should_deactive_slimbus = true;

	return 0;
}

static int init_thread(void)
{
	int i;
	int ret;
	struct sched_param param;

	for (i = 0; i < HOOK_CNT; i++) {
		hook_priv->runtime[i].hook_id = i;

		INIT_LIST_HEAD(&hook_priv->runtime[i].info_list.node);
		INIT_LIST_HEAD(&hook_priv->runtime[i].data_list.node);

		wakeup_source_init(&hook_priv->runtime[i].wake_lock,
			"da_combine_dsp_hook");

		sema_init(&hook_priv->runtime[i].hook_proc_sema, 0);
		sema_init(&hook_priv->runtime[i].hook_stop_sema, 1);

		if (i == HOOK_RIGHT)
			hook_priv->runtime[i].kthread = kthread_create(
				right_data_parse_thread, 0,
				"right_data_parse_thread");
		else
			hook_priv->runtime[i].kthread = kthread_create(
				left_data_parse_thread, 0,
				"left_data_parse_thread");

		if (IS_ERR(hook_priv->runtime[i].kthread)) {
			AUDIO_LOGE("create data parse thread fail:%d", i);
			return -EINVAL;
		}

		hook_priv->runtime[i].kthread_should_stop = 0;

		/* set high prio */
		memset(&param, 0, sizeof(struct sched_param));
		param.sched_priority = MAX_RT_PRIO - HOOK_THREAD_PRIO;
		ret = sched_setscheduler(hook_priv->runtime[i].kthread,
			SCHED_RR, &param);
		if (ret)
			AUDIO_LOGE("set thread schedule priorty failed:%d", i);
	}

	wake_up_process(hook_priv->runtime[HOOK_LEFT].kthread);
	wake_up_process(hook_priv->runtime[HOOK_RIGHT].kthread);

	return 0;
}

static void deinit_thread(void)
{
	int i;

	for (i = 0; i < HOOK_CNT; i++) {
		if (hook_priv->runtime[i].kthread) {
			hook_priv->runtime[i].kthread_should_stop = 1;
			up(&hook_priv->runtime[i].hook_proc_sema);
			kthread_stop(hook_priv->runtime[i].kthread);
			hook_priv->runtime[i].kthread = NULL;
		}

		wakeup_source_trash(&hook_priv->runtime[i].wake_lock);
	}
}

static bool is_file_valid(const char *buf, const char *path, unsigned int size)
{
	struct da_combine_hook_priv *priv = hook_priv;

	if (priv == NULL) {
		AUDIO_LOGE("hook priv is null");
		return false;
	}

	if (path == NULL || buf == NULL || size == 0) {
		AUDIO_LOGE("para err");
		return false;
	}

	return true;
}

int da_combine_dsp_create_hook_dir(const char *path)
{
	char tmp_path[HOOK_PATH_MAX_LENGTH] = {0};

	if (path == NULL) {
		AUDIO_LOGE("path is null");
		return -EINVAL;
	}

	strncpy(tmp_path, path,  sizeof(tmp_path) - 1);

	if (audio_create_dir(tmp_path, HOOK_PATH_MAX_LENGTH - 1))
		return -EFAULT;

	return 0;
}

void da_combine_dsp_dump_to_file(const char *buf, unsigned int size,
	const char *path)
{
	unsigned int file_flag = O_RDWR;
	mm_segment_t fs;
	struct file *fp = NULL;
	struct kstat file_stat;
	struct da_combine_hook_priv *priv = hook_priv;

	if(!is_file_valid(buf, path, size))
		return;

	fs = get_fs();
	set_fs(KERNEL_DS);

	if (vfs_stat(path, &file_stat) == 0 &&
		file_stat.size > HOOK_MAX_FILE_SIZE) {
		/* delete file */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
		ksys_unlink(path);
#else
		sys_unlink(path);
#endif

		AUDIO_LOGI("delete too large file");
	}

	if (vfs_stat(path, &file_stat) < 0) {
		if (priv->open_file_failed_cnt < PRINT_MAX_CNT)
			AUDIO_LOGI("create file");

		file_flag |= O_CREAT;
	}

	fp = filp_open(path, file_flag, 0660);
	if (IS_ERR(fp)) {
		if (priv->open_file_failed_cnt < PRINT_MAX_CNT) {
			AUDIO_LOGE("open file fail");
			priv->open_file_failed_cnt++;
		}
		fp = NULL;
		goto END;
	}

	vfs_llseek(fp, 0L, SEEK_END);

	if (vfs_write(fp, buf, size, &fp->f_pos) < 0) /*lint !e613*/
		AUDIO_LOGE("write file fail");

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
	if (ksys_chown((const char __user *)path, ROOT_UID, SYSTEM_GID))
#else
	if (sys_chown((const char __user *)path, ROOT_UID, SYSTEM_GID))
#endif
		AUDIO_LOGE("chown path failed");

END:
	if (fp != NULL)
		filp_close(fp, 0);

	set_fs(fs);
}

void da_combine_stop_hook_route(void)
{
	struct da_combine_hook_priv *priv = hook_priv;

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return;
	}

	if (!priv->started || priv->standby)
		return;

	stop_hook(HOOK_LEFT);
	AUDIO_LOGI("left hook stoped");

	if (priv->bandwidth == OM_BANDWIDTH_12288) {
		stop_hook(HOOK_RIGHT);
		AUDIO_LOGI("right hook stoped");
	}

	deactivate_slimbus();

	priv->sponsor = OM_SPONSOR_DEFAULT;
}

/* stop hook route and notify dsp */
void da_combine_stop_dsp_hook(void)
{
	int ret;
	struct om_stop_hook_msg stop_hook_msg;
	union da_combine_high_freq_status high_status;

	if (hook_priv == NULL) {
		AUDIO_LOGE("priv is null");
		return;
	}

	high_status.val = da_combine_get_high_freq_status();
	if (high_status.om_hook == 0) {
		AUDIO_LOGW("om hook is not opened");
		return;
	}

	stop_hook_msg.msg_id = ID_AP_DSP_HOOK_STOP;

	da_combine_stop_hook_route();

	ret = da_combine_sync_write(&stop_hook_msg,
		(unsigned int)sizeof(struct om_stop_hook_msg));
	if (ret != 0) {
		AUDIO_LOGE("sync write failed");
		goto end;
	}

	hook_priv->is_dspif_hooking = false;
	AUDIO_LOGI("hook stopped");

end:
	(void)da_combine_release_pll_resource(HIGH_FREQ_SCENE_OM_HOOK);
}

void da_combine_stop_dspif_hook(void)
{
	if (hook_priv == NULL) {
		AUDIO_LOGE("priv is null");
		return;
	}

	if (hook_priv->is_dspif_hooking) {
		AUDIO_LOGW("dsp scene will start, stop hooking dspif data");
		da_combine_stop_dsp_hook();
	}
}

/*
 * this function is called by sync ap msg proc or dsp msg proc and
 * needs to check the scene and param
 */
int da_combine_stop_hook(const struct da_combine_param_io_buf *param)
{

	return 0;
}

int da_combine_start_hook(const struct da_combine_param_io_buf *param)
{
	int ret = 0;

	return ret;
}

int da_combine_set_dsp_hook_bw(unsigned short bandwidth)
{
	struct da_combine_hook_priv *priv = hook_priv;

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return -EINVAL;
	}

	if (bandwidth >= OM_BANDWIDTH_BUTT) {
		AUDIO_LOGE("err om bw:%d", bandwidth);
		return -EINVAL;
	}

	if (priv->started || !priv->standby) {
		AUDIO_LOGE("om is running, forbid set bw:%d", bandwidth);
		return -EBUSY;
	}

	priv->bandwidth = bandwidth;

	AUDIO_LOGI("set om bw:%d", bandwidth);

	return 0;
}

int da_combine_set_dsp_hook_sponsor(unsigned short sponsor)
{
	struct da_combine_hook_priv *priv = hook_priv;

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return -EINVAL;
	}

	if (sponsor >= OM_SPONSOR_BUTT) {
		AUDIO_LOGE("err om sponsor:%d", sponsor);
		return -EINVAL;
	}

	if (priv->started || !priv->standby) {
		AUDIO_LOGE("hook is running, forbid set sponsor:%d", sponsor);
		return -EBUSY;
	}

	priv->sponsor = sponsor;

	AUDIO_LOGI("set om sponsor:%d", sponsor);

	return 0;
}

int da_combine_set_hook_path(const struct da_combine_param_io_buf *param)
{
	int ret;
	struct om_set_hook_path_msg *set_path = NULL;
	struct da_combine_hook_priv *priv = hook_priv;

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return -EINVAL;
	}

	ret = check_param_and_scene(param,
		sizeof(struct om_set_hook_path_msg));
	if (ret != 0)
		return ret;

	set_path = (struct om_set_hook_path_msg *)param->buf_in;
	if (set_path->size > (HOOK_PATH_MAX_LENGTH - 2)) {
		AUDIO_LOGE("err para");
		return -EINVAL;
	}

	if (priv->started || !priv->standby) {
		AUDIO_LOGE("om is running, forbid set path");
		return -EBUSY;
	}

	AUDIO_LOGI("ok");

	return ret;
}


int da_combine_dsp_hook_init(struct da_combine_irq *irqmgr,
	unsigned int codec_type)
{
	int ret;
	struct device *dev = irqmgr->dev;

	AUDIO_LOGI("init begin");

	hook_priv = kzalloc(sizeof(*hook_priv), GFP_KERNEL);
	if (hook_priv == NULL) {
		AUDIO_LOGE("dsp hook malloc failed");
		return -ENOMEM;
	}

	ret = init_para(codec_type);
	if (ret != 0)
		goto err_exit;

	hook_priv->asp_subsys_clk = devm_clk_get(dev, "clk_asp_subsys");
	if (IS_ERR(hook_priv->asp_subsys_clk)) {
		AUDIO_LOGE("asp subsys clk fail");
		ret = -EINVAL;
		goto err_exit;
	}

	ret = init_thread();
	if (ret != 0)
		goto err_exit;

	AUDIO_LOGI("init end");

	return 0;

err_exit:
	kfree(hook_priv);
	hook_priv = NULL;
	AUDIO_LOGE("init failed");

	return ret;
}

void da_combine_dsp_hook_deinit(void)
{
	if (hook_priv == NULL)
		return;

	deinit_thread();

	kfree(hook_priv);
	hook_priv = NULL;
}

