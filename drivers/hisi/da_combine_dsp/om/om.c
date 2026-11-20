/*
 * om.c
 *
 * om module for da_combine codec
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

#include "om.h"

#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/vmalloc.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/rtc.h>
#include <linux/hisi/da_combine/da_combine_utils.h>
#include <linux/hisi/da_combine/da_combine_dsp_regs.h>
#include "linux/hisi/audio_log.h"
#include <rdr_audio_codec.h>
#include <rdr_audio_adapter.h>

#include "da_combine_dsp_misc.h"
#include "dsp_utils.h"
#include "audio_file.h"
#include "om_hook.h"
#include "om_beta.h"

#define LOG_TAG "DA_combine_om"

#define OM_DA_COMBINE_LOG_PATH "codechifi_logs/"
#define DUMP_LOG_FILE_NAME "codec_reg.bin"
#define MAX_DUMP_REG_SIZE 12000
#define DUMP_DIR_LEN 128
#define REG_ROW_NUM 200
#define REG_ROW_LEN 60

struct da_combine_om_priv {
	struct task_struct *kda_combine_dump_task;
	struct semaphore da_combine_dump_sema;
	struct da_combine_dsp_config dsp_config;
	bool only_dump_print;
};

static struct da_combine_om_priv *om_priv;

struct reg_dump {
	unsigned int addr;
	unsigned int len;
	const char *name;
};

static const struct reg_dump regs_map[] = {
	{ DA_COMBINE_DUMP_CFG_SUB_ADDR1,       DA_COMBINE_DUMP_CFG_SUB_SIZE1,      "page_cfg_subsys:" },
	{ DA_COMBINE_DUMP_CFG_SUB_ADDR2,       DA_COMBINE_DUMP_CFG_SUB_SIZE2,      "page_cfg_subsys:" },
	{ DA_COMBINE_DUMP_AUDIO_SUB_ADDR,      DA_COMBINE_DUMP_AUDIO_SUB_SIZE,     "aud_reg:" },
	{ DA_COMBINE_DUMP_DSP_EDMA_ADDR1,      DA_COMBINE_DUMP_DSP_EDMA_SIZE1,     "DMA:" },
	{ DA_COMBINE_DUMP_DSP_EDMA_ADDR2,      DA_COMBINE_DUMP_DSP_EDMA_SIZE2,     "DMA:" },
	{ DA_COMBINE_DUMP_DSP_EDMA_ADDR3,      DA_COMBINE_DUMP_DSP_EDMA_SIZE3,     "DMA:" },
	{ DA_COMBINE_DUMP_DSP_WATCHDOG_ADDR1,  DA_COMBINE_DUMP_DSP_WATCHDOG_SIZE1, "WTD:" },
	{ DA_COMBINE_DUMP_DSP_WATCHDOG_ADDR2,  DA_COMBINE_DUMP_DSP_WATCHDOG_SIZE2, "WTD:" },
	{ DA_COMBINE_DUMP_DSP_SCTRL_ADDR1,     DA_COMBINE_DUMP_DSP_SCTRL_SIZE1,    "SCTRL:" },
	{ DA_COMBINE_DUMP_DSP_SCTRL_ADDR2,     DA_COMBINE_DUMP_DSP_SCTRL_SIZE2,    "SCTRL:" },
	/* 0x20007038, 0x20007039 should always read in the end */
	{ DA_COMBINE_DUMP_CFG_SUB_ADDR3,       DA_COMBINE_DUMP_CFG_SUB_SIZE3,      "page_cfg_subsys:" },
};

static void read_regs2buf(char *buf, const size_t size)
{
	size_t i;
	int ret;
	size_t buffer_used = 0;

	if (buf == NULL) {
		AUDIO_LOGE("input buf is null");
		return;
	}

	ret = da_combine_request_low_pll_resource(LOW_FREQ_SCENE_DUMP);
	if (ret != 0) {
		AUDIO_LOGE("dump reg request pll failed");
		return;
	}

	for (i = 0; i < ARRAY_SIZE(regs_map); i++) {
		if (buffer_used + regs_map[i].len > size) {
			AUDIO_LOGE("buffer size error, index:%zd, used size:%zd",
				i, buffer_used);
			AUDIO_LOGE("current reg size:%d, total size:%zd",
				regs_map[i].len, size);

			continue;
		}
		da_combine_read(regs_map[i].addr,
			(unsigned char *)(buf + buffer_used), regs_map[i].len);
		buffer_used += regs_map[i].len;
	}

	da_combine_release_low_pll_resource(LOW_FREQ_SCENE_DUMP);
}

static int check_regs_para(const char *codec_reg_addr,
	const char *dump_reg_addr, const size_t dump_reg_size)
{
	if (codec_reg_addr == NULL) {
		AUDIO_LOGE("reg addr is null");
		return -EINVAL;
	}

	if (dump_reg_addr == NULL) {
		AUDIO_LOGE("dump reg addr is null");
		return -EINVAL;
	}

	if (dump_reg_size > MAX_DUMP_REG_SIZE) {
		AUDIO_LOGE("dump reg size is err, size is %zd", dump_reg_size);
		return -EINVAL;
	}

	return 0;
}

void format_aligned_regs(const unsigned char *offset,
	char *reg_addr, const size_t reg_size,
	const struct reg_dump *s_reg_dump)
{
	unsigned int j;
	unsigned int dump_row_num = 4;
	unsigned int dump_data_bytes = 4;

	snprintf(reg_addr + strlen(reg_addr),
		reg_size - strlen(reg_addr),
		"\n%s 0x%08x (0x%08x) ",
		s_reg_dump->name,
		s_reg_dump->addr,
		s_reg_dump->len);

	for (j = 0; j < (s_reg_dump->len / dump_data_bytes); j++) {
		if (j % dump_row_num == 0)
			snprintf(reg_addr + strlen(reg_addr),
				reg_size - strlen(reg_addr),
				"\n0x%08x:",
				s_reg_dump->addr + j * dump_data_bytes);

		snprintf(reg_addr + strlen(reg_addr),
			reg_size - strlen(reg_addr),
			"0x%08x ",
			*((unsigned int *)offset + j));
	}
}

void format_last_regs(const unsigned char *reg_offset_addr,
	char *dump_reg_addr, const size_t dump_reg_size,
	const struct reg_dump *s_reg_dump)
{
	unsigned int dump_data_bytes = 4;
	unsigned int cur_offset;

	if (s_reg_dump->len % dump_data_bytes != 0) {
		cur_offset = s_reg_dump->len / dump_data_bytes * dump_data_bytes;

		switch (s_reg_dump->len % dump_data_bytes) {
		case 1:
			snprintf(dump_reg_addr + strlen(dump_reg_addr),
				dump_reg_size - strlen(dump_reg_addr),
				"0x%02x",
				*(reg_offset_addr + cur_offset));
			break;
		case 2:
			snprintf(dump_reg_addr + strlen(dump_reg_addr),
				dump_reg_size - strlen(dump_reg_addr),
				"0x%02x%02x",
				*(reg_offset_addr + cur_offset + 1),
				*(reg_offset_addr + cur_offset));
			break;
		case 3:
			snprintf(dump_reg_addr + strlen(dump_reg_addr),
				dump_reg_size - strlen(dump_reg_addr),
				"0x%02x%02x%02x",
				*(reg_offset_addr + cur_offset + 2),
				*(reg_offset_addr + cur_offset + 1),
				*(reg_offset_addr + cur_offset));
			break;
		default:
			return;
		}
	}
}

static void format_regs(char *codec_reg_addr, char *dump_reg_addr,
	const size_t dump_reg_size)
{
	unsigned int reg_offset = 0;
	unsigned int i;
	unsigned char *cur_reg_offset = NULL;

	if (check_regs_para(codec_reg_addr, dump_reg_addr, dump_reg_size))
		return;

	memset(dump_reg_addr, 0, dump_reg_size); /* unsafe_function_ignore: memset */

	for (i = 0; i < ARRAY_SIZE(regs_map); i++) {
		cur_reg_offset = codec_reg_addr + reg_offset;

		format_aligned_regs(cur_reg_offset, dump_reg_addr,
			dump_reg_size, &regs_map[i]);

		format_last_regs(cur_reg_offset, dump_reg_addr,
			dump_reg_size, &regs_map[i]);

		reg_offset += regs_map[i].len;
	}
}

size_t get_dump_regs_size(void)
{
	unsigned int i;
	size_t size = 0;

	for (i = 0; i < ARRAY_SIZE(regs_map); i++)
		size += regs_map[i].len;

	AUDIO_LOGI("dump regs size is 0x%lx", size);

	return size;
}

static int save_reg_file(const char *path, unsigned int size)
{
	int ret = 0;
	size_t reg_size;
	char *reg_buf = NULL;
	char *dump_reg_buf = NULL;
	size_t dump_reg_size = REG_ROW_LEN * REG_ROW_NUM;

	if (path == NULL) {
		AUDIO_LOGE("path is null, can not dump reg");
		return -EINVAL;
	}

	reg_size = get_dump_regs_size();

	reg_buf = vzalloc(reg_size);
	if (reg_buf == NULL) {
		AUDIO_LOGE("alloc reg_buf failed");
		return -ENOMEM;
	}
	read_regs2buf(reg_buf, reg_size);

	dump_reg_buf = vzalloc(dump_reg_size);
	if (dump_reg_buf == NULL) {
		AUDIO_LOGE("alloc dump_reg_buf failed");
		vfree(reg_buf);
		return -ENOMEM;
	}

	format_regs(reg_buf, dump_reg_buf, dump_reg_size);

#ifdef CONFIG_HISI_HIFI_BB
	ret = rdr_audio_write_file(path, dump_reg_buf,
		(unsigned int)strlen(dump_reg_buf) + 1);
	if (ret != 0)
		AUDIO_LOGE("write reg file fail");
#endif
	vfree(reg_buf);
	vfree(dump_reg_buf);

	return ret;
}

void da_combine_save_log(void)
{

}

void da_combine_dsp_dump_with_path(const char *dirname)
{
	int ret;
	char path[DUMP_DIR_LEN] = {0};

	if (dirname == NULL) {
		AUDIO_LOGE("path is null, can not dump");
		return;
	}

	if (om_priv == NULL) {
		AUDIO_LOGE("om priv is null");
		return;
	}

	snprintf(path, sizeof(path) - 1, "%s%s%s",
		dirname, OM_DA_COMBINE_DUMP_RAM_LOG_PATH,
		OM_DA_COMBINE_DUMP_OCRAM_NAME);

	ret = da_combine_save_log_file(path, DUMP_TYPE_PRINT_LOG,
		om_priv->dsp_config.dump_log_addr,
		om_priv->dsp_config.dump_log_size);
	if (ret != 0)
		AUDIO_LOGE("dump ocram log failed");

	if (om_priv->only_dump_print == false) {
		snprintf(path, sizeof(path) - 1, "%s%s%s",
			dirname, OM_DA_COMBINE_DUMP_RAM_LOG_PATH,
			OM_DA_COMBINE_DUMP_OCRAM_NAME);

		ret = da_combine_save_log_file(path, DUMP_TYPE_PANIC_LOG,
			om_priv->dsp_config.dump_ocram_addr,
			om_priv->dsp_config.dump_ocram_size);
		if (ret != 0)
			AUDIO_LOGE("dump panic stack log failed");

		snprintf(path, sizeof(path) - 1, "%s%s%s",
			dirname, OM_DA_COMBINE_LOG_PATH,
			DUMP_LOG_FILE_NAME);

		ret = save_reg_file(path, sizeof(path));
		if (ret != 0)
			AUDIO_LOGE("save reg file fail, ret:%d\n", ret);
	}

	om_priv->only_dump_print = false;
}

void da_combine_dsp_dump_no_path(void)
{
	if (om_priv == NULL) {
		AUDIO_LOGE("om priv is null");
		return;
	}

	up(&om_priv->da_combine_dump_sema);
}

void da_combine_set_only_printed_enable(bool enable)
{
	if (om_priv == NULL) {
		AUDIO_LOGE("om priv is null");
		return;
	}

	om_priv->only_dump_print = enable;
}

static int da_combine_dump_dsp_thread(void *p)
{
	int ret;
	char *log_path = NULL;

	while (!kthread_should_stop()) {
		if (down_interruptible(&om_priv->da_combine_dump_sema) != 0)
			AUDIO_LOGE("dump dsp thread wake up err");

		if (bbox_check_edition() != EDITION_INTERNAL_BETA) {
			AUDIO_LOGE("not beta, do not dump hifi");
			om_priv->only_dump_print = false;
			continue;
		}

		log_path = kzalloc((unsigned long)DUMP_DIR_LEN, GFP_ATOMIC);
		if (log_path == NULL) {
			AUDIO_LOGE("log path alloc fail");
			om_priv->only_dump_print = false;
			continue;
		}

		ret = audio_create_rootpath(log_path, DUMP_DIR_ROOT,
			DUMP_DIR_LEN - 1);
		if (ret != 0) {
			AUDIO_LOGE("create log path failed, do not dump");
			om_priv->only_dump_print = false;
			kfree(log_path);
			log_path = NULL;
			continue;
		}

		da_combine_dsp_dump_with_path(log_path);

		kfree(log_path);
		log_path = NULL;
	}

	return 0;
}

int da_combine_dsp_om_init(const struct da_combine_dsp_config *config,
	struct da_combine_irq *irqmgr)
{
	int ret;

	om_priv = kzalloc(sizeof(*om_priv), GFP_KERNEL);
	if (om_priv == NULL) {
		AUDIO_LOGE("dsp debug init: failed to kzalloc priv debug");
		return -ENOMEM;
	}

	memcpy(&om_priv->dsp_config, config, sizeof(*config));

	sema_init(&om_priv->da_combine_dump_sema, 1);

	om_priv->kda_combine_dump_task = kthread_create(da_combine_dump_dsp_thread, 0, "hi64xxdumplog");
	if (IS_ERR(om_priv->kda_combine_dump_task)) {
		AUDIO_LOGE("create dsp dump log thread fail");
		kfree(om_priv);
		om_priv = NULL;
		return -EINVAL;
	}

	wake_up_process(om_priv->kda_combine_dump_task);
	om_priv->only_dump_print = false;

	ret = da_combine_dsp_beta_init();
	if (ret != 0) {
		AUDIO_LOGE("dsp beta init err, ret %d", ret);
		goto beta_init_err;
	}

	ret = da_combine_dsp_hook_init(irqmgr, config->codec_type);
	if (ret != 0) {
		AUDIO_LOGE("dsp hook init err, ret %d", ret);
		goto hook_init_err;
	}


	return ret;

hook_init_err:
	da_combine_dsp_beta_deinit();
beta_init_err:
	up(&om_priv->da_combine_dump_sema);
	kthread_stop(om_priv->kda_combine_dump_task);
	om_priv->kda_combine_dump_task = NULL;

	kfree(om_priv);
	om_priv = NULL;

	return ret;
}

void da_combine_dsp_om_deinit(void)
{
	da_combine_dsp_hook_deinit();
	da_combine_dsp_beta_deinit();

	up(&om_priv->da_combine_dump_sema);
	kthread_stop(om_priv->kda_combine_dump_task);
	om_priv->kda_combine_dump_task = NULL;

	kfree(om_priv);
	om_priv = NULL;
}

