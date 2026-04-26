/*
 * audio codec debug driver.
 *
 * Copyright (c) 2017 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#ifdef CONFIG_SWITCH
#include <linux/switch.h>
#endif
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include "hicodec_debug.h"
#include "asoc_adapter.h"

#define REG_CACHE_NUM_MAX       800
#define TIME_STAMP_MAX          (24*60*60)
#define HICODEC_DBG_SIZE_PAGE   (1024*40)
#define HICODEC_DBG_SIZE_WIDGET 8192
#define HICODEC_DBG_SIZE_PATH   16384
#define HICODEC_DBG_SIZE_CTRL   10240
#define HICODEC_DBG_SIZE_CACHE  (28 * REG_CACHE_NUM_MAX)

#define ALLOC_FAIL_MSG      "buffer alloc failed\n"
#define LOG_TAG "hicodec: "
#define AUDIO_DEBUG_DIR                "audio"

struct hicodec_rr_cache_node
{
	int rw;
	unsigned int reg;
	unsigned int val;
	unsigned int time;
};

struct hicodec_rr_cache
{
	struct hicodec_rr_cache_node cache[REG_CACHE_NUM_MAX];
	unsigned int cache_idx;
	spinlock_t lock;
};

static struct snd_soc_component *g_codec[ASP_CODEC_CNT];
static struct proc_dir_entry *audio_debug_dir = NULL;
static struct hicodec_rr_cache *rr_cache = NULL;
static struct hicodec_dump_reg_info *reg_info[ASP_CODEC_CNT];

static bool isReadRegAll = true;
static unsigned int g_vs_reg;

static bool is_valid_codec(struct snd_soc_component **codec)
{
	int i;

	for (i = 0; i < ASP_CODEC_CNT; i++)
		if (codec[i])
			return true;

	return false;
}

static bool is_valid_reg_info(struct hicodec_dump_reg_info **info)
{
	int i;

	for (i = 0; i < ASP_CODEC_CNT; i++)
		if (info[i])
			return true;

	return false;
}
/*******************************************************************************
 * SECTION for register history dump
 *******************************************************************************/
/*
 * record reg read/write op
 * loop range (0-1023)
 * catch atomic ensured by read/write function
 */
void hicodec_debug_reg_rw_cache(unsigned int reg, unsigned int val, int rw)
{
	u64 sec;
	unsigned int idx_wr;

	if (!is_valid_codec(g_codec) || !rr_cache) {
		return;
	}

	idx_wr = rr_cache->cache_idx;

	idx_wr %= REG_CACHE_NUM_MAX;
	rr_cache->cache[idx_wr].rw = rw;
	rr_cache->cache[idx_wr].reg = reg;
	rr_cache->cache[idx_wr].val = val;

	sec = hisi_getcurtime();
	do_div(sec, NSEC_PER_SEC);
	sec %= TIME_STAMP_MAX;
	rr_cache->cache[idx_wr].time = (unsigned long int)sec;
	idx_wr++;
	rr_cache->cache_idx = idx_wr % REG_CACHE_NUM_MAX;
}

static void hicodec_debug_reg_history_dp(char *buf, size_t buf_len)
{
	unsigned int idx_wr_now, idx_wr_latter;
	unsigned int idx, idx_start = 0, idx_stop = 0;
	unsigned long time, time_next;
	unsigned int reg = 0, val = 0, rw = 0;
	unsigned long flags = 0;
	size_t content_len;
	u64 sec;

	if (!is_valid_codec(g_codec)) {
		snprintf(buf, buf_len, "g_codec is null\n"); /*lint !e747 */
		return;
	}

	if (!rr_cache) {
		snprintf(buf, buf_len, "rr_cache is null\n"); /*lint !e747 */
		return;
	}

	spin_lock_irqsave(&rr_cache->lock, flags); /*lint !e550 */
	idx_wr_now = rr_cache->cache_idx;
	time_next = rr_cache->cache[(idx_wr_now + 1) % REG_CACHE_NUM_MAX].time; /*lint !e679 */
	spin_unlock_irqrestore(&rr_cache->lock, flags);

	if (idx_wr_now >= REG_CACHE_NUM_MAX) {
		snprintf(buf, buf_len, "rr_idx(%d) err\n", idx_wr_now); /*lint !e747 */
		return;
	}

	if (0 == idx_wr_now) {
		snprintf(buf, buf_len, "no register cached now\n"); /*lint !e747 */
		return;
	}

	/* parameters: idx_wr_now */
	sec = hisi_getcurtime();
	do_div(sec, NSEC_PER_SEC);
	snprintf(buf, buf_len, "time=%lu s, idx now=%d, BEGIN\n", (unsigned long int)sec, idx_wr_now); /*lint !e747 */

	/* judge the position of idx in order to loop*/
	if (0 == time_next) {
		idx_start = 0;
		idx_stop = idx_wr_now;
	} else {
		/* loop */
		idx_start = idx_wr_now + 1;
		idx_stop = idx_wr_now + REG_CACHE_NUM_MAX;
	}

	for (idx = idx_start; idx < idx_stop; idx++) {
		spin_lock_irqsave(&rr_cache->lock, flags); /*lint !e550 */

		rw = rr_cache->cache[idx % REG_CACHE_NUM_MAX].rw;
		reg = rr_cache->cache[idx % REG_CACHE_NUM_MAX].reg;
		val = rr_cache->cache[idx % REG_CACHE_NUM_MAX].val;
		time = rr_cache->cache[idx % REG_CACHE_NUM_MAX].time;

		content_len = strlen(buf);
		if (HICODEC_DEBUG_FLAG_READ == rw)
			/* read */
			snprintf(buf + content_len, buf_len - content_len, "%lu.r 0x%04X 0x%08X\n", time, reg, val);
		else if (HICODEC_DEBUG_FLAG_WRITE == rw)
			/* write */
			snprintf(buf + content_len, buf_len - content_len, "%lu.w 0x%04X 0x%08X\n", time, reg, val);
		else
			/* error branch */
			snprintf(buf + content_len, buf_len - content_len, "%lu.e %x %x\n", time, reg, val);
		spin_unlock_irqrestore(&rr_cache->lock, flags);
	}

	/* dump */
	spin_lock_irqsave(&rr_cache->lock, flags); /*lint !e550 */
	idx_wr_latter = rr_cache->cache_idx;
	spin_unlock_irqrestore(&rr_cache->lock, flags);
	content_len = strlen(buf);
	snprintf(buf + content_len, buf_len - content_len, "idx=%d -- %d, END\n", idx_wr_now, idx_wr_latter);
}

/*
 * read history register
 * cat rh
 */
static ssize_t hicodec_debug_rh_read(struct file *file, char __user *user_buf,
		size_t count, loff_t *ppos)
{
	char *kn_buf;
	ssize_t byte_read;

	kn_buf = vmalloc(HICODEC_DBG_SIZE_CACHE); /*lint !e747 */
	if (NULL == kn_buf) {
		pr_err(LOG_TAG"kn_buf is null\n");
		return simple_read_from_buffer(user_buf, count, ppos, ALLOC_FAIL_MSG, sizeof(ALLOC_FAIL_MSG));
	}

	memset(kn_buf, 0, HICODEC_DBG_SIZE_CACHE);/* unsafe_function_ignore: memset */
	hicodec_debug_reg_history_dp(kn_buf, HICODEC_DBG_SIZE_CACHE);

	byte_read = simple_read_from_buffer(user_buf, count, ppos, kn_buf, strlen(kn_buf));
	vfree(kn_buf);

	return byte_read;
}

static const struct file_operations hicodec_debug_rh_fops = {
	.read  = hicodec_debug_rh_read,
};

static void dump_dapm_widget(char *buf, struct snd_soc_component *codec, size_t buf_len)
{
	struct list_head *l = NULL;
	struct snd_soc_dapm_widget *w = NULL;
	struct snd_soc_dapm_context *dc = NULL;
	size_t content_len;
	int i = -1;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
	dc = snd_soc_component_get_dapm(codec);
#else
	dc = &(codec->dapm);
#endif
	if (!dc || !(dc->card)) {
		snprintf(buf, buf_len, "dc or dc->card or dc->card->widgets is null\n"); /*lint !e747 */
		return;
	}
	l = &(dc->card->widgets);
	list_for_each_entry(w, l, list) { /*lint !e64 !e826 */
		i++;
		content_len = strlen(buf);
		snprintf(buf + content_len, buf_len - content_len, "<%02d> pwr= %d \"%s\"\n", i, w->power, w->name);
	}
}

static void hicodec_debug_dapm_widget_dump(char *buf, size_t buf_len)
{
	size_t content_len;
	int i;

	if (!is_valid_codec(g_codec)) {
		snprintf(buf, buf_len, "g_codec is null\n"); /*lint !e747 */
		return;
	}
	snprintf(buf, buf_len, "BEGIN widget\n"); /*lint !e747 */

	for (i = 0; i < ASP_CODEC_CNT; i++)
		if(g_codec[i])
			dump_dapm_widget(buf, g_codec[i], buf_len);

	content_len = strlen(buf);
	snprintf(buf + content_len, buf_len - content_len, "END\n");
}

/*
 * read dapm widget
 * cat dwidget
 */
static ssize_t hicodec_debug_dwidget_read(struct file *file, char __user *user_buf,
		size_t count, loff_t *ppos)
{
	char *kn_buf;
	ssize_t byte_read;

	kn_buf = vmalloc(HICODEC_DBG_SIZE_WIDGET); /*lint !e747 */
	if (NULL == kn_buf) {
		pr_err(LOG_TAG"kn_buf is null\n");
		return simple_read_from_buffer(user_buf, count, ppos, ALLOC_FAIL_MSG, sizeof(ALLOC_FAIL_MSG));
	}

	memset(kn_buf, 0, HICODEC_DBG_SIZE_WIDGET);/* unsafe_function_ignore: memset */
	hicodec_debug_dapm_widget_dump(kn_buf, HICODEC_DBG_SIZE_WIDGET);

	byte_read = simple_read_from_buffer(user_buf, count, ppos, kn_buf, strlen(kn_buf));
	vfree(kn_buf);

	return byte_read;
}

static const struct file_operations hicodec_debug_dwidget_fops = {
	.read  = hicodec_debug_dwidget_read,
};

static void dump_dapm_path(char *buf, struct snd_soc_component *codec, size_t buf_len)
{
	struct list_head *l = NULL;
	struct snd_soc_dapm_path *p = NULL;
	struct snd_soc_dapm_widget *source = NULL;
	struct snd_soc_dapm_widget *sink = NULL;
	struct snd_soc_dapm_context *dc = NULL;

	int i = -1;
	size_t content_len;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
	dc = snd_soc_component_get_dapm(codec);
#else
	dc = &(codec->dapm);
#endif
	if (!dc || !(dc->card)) {
		snprintf(buf, HICODEC_DBG_SIZE_WIDGET, "dc or dc->card or dc->card->paths is null\n"); /*lint !e747 */
		return;
	}
	l = &(dc->card->paths);
	list_for_each_entry(p, l, list) { /*lint !e64 !e826 */
		i++;
		content_len = strlen(buf);
		snprintf(buf + content_len, buf_len - content_len, "<%d>cn_stat: %d\n", i, p->connect);
		if (p->source == NULL || p->sink == NULL) {
			snprintf(buf + strlen(buf), buf_len - strlen(buf), "<%d>end\n", i);
			continue;
		}

		source = p->source;
		sink   = p->sink;

		content_len = strlen(buf);
		snprintf(buf + content_len, buf_len - content_len, "src w \"%s\"\n", source->name);
		content_len = strlen(buf);
		snprintf(buf + content_len, buf_len - content_len, "sink w \"%s\"\n", sink->name);
	}
}

static void hicodec_debug_dapm_path_dump(char *buf, size_t buf_len)
{
	size_t content_len;
	int i;

	if (!is_valid_codec(g_codec)) {
		snprintf(buf, buf_len, "g_codec is null\n"); /*lint !e747 */
		return;
	}
	snprintf(buf, buf_len, "BEGIN path\n"); /*lint !e747 */

	for (i = 0; i < ASP_CODEC_CNT; i++)
		if(g_codec[i])
			dump_dapm_path(buf, g_codec[i], buf_len);

	content_len = strlen(buf);
	snprintf(buf + content_len, buf_len - content_len, "END\n");
}

/*
 * read dapm path
 * cat dpath
 */
static ssize_t hicodec_debug_dpath_read(struct file *file, char __user *user_buf,
		size_t count, loff_t *ppos)
{
	char *kn_buf;
	ssize_t byte_read;

	kn_buf = vmalloc(HICODEC_DBG_SIZE_PATH); /*lint !e747 */
	if (NULL == kn_buf) {
		pr_err(LOG_TAG"kn_buf is null\n");
		return simple_read_from_buffer(user_buf, count, ppos, ALLOC_FAIL_MSG, sizeof(ALLOC_FAIL_MSG));
	}

	memset(kn_buf, 0, HICODEC_DBG_SIZE_PATH);/* unsafe_function_ignore: memset */
	hicodec_debug_dapm_path_dump(kn_buf, HICODEC_DBG_SIZE_PATH);

	byte_read = simple_read_from_buffer(user_buf, count, ppos, kn_buf, strlen(kn_buf));
	vfree(kn_buf);

	return byte_read;
}

static const struct file_operations hicodec_debug_dpath_fops = {
	.read  = hicodec_debug_dpath_read,
};

static void dump_kcontrol(char *buf, struct snd_soc_component *codec, size_t buf_len)
{
	struct list_head *l = NULL;
	struct snd_kcontrol *ctrl = NULL;
	struct snd_ctl_elem_info info;
	struct snd_ctl_elem_value value;
	struct snd_soc_dapm_context *dc = NULL;

	int i = -1;
	size_t content_len;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
	dc = snd_soc_component_get_dapm(codec);
#else
	dc = &(codec->dapm);
#endif
	if (!dc || !(dc->card) || !(dc->card->snd_card)) {
		snprintf(buf, HICODEC_DBG_SIZE_WIDGET, "dc or dc->card or dc->card->snd_card is null\n"); /*lint !e747 */
		return;
	}
	l = &(dc->card->snd_card->controls);
	content_len = strlen(buf);
	snprintf(buf + content_len, buf_len - content_len, "controls_count=%d\n", dc->card->snd_card->controls_count);
	list_for_each_entry(ctrl, l, list) {
		i++;
		content_len = strlen(buf);
		snprintf(buf + content_len, buf_len - content_len, "%d\t%-48s ", i, ctrl->id.name);
		memset(&info, 0, sizeof(info));
		memset(&value, 0, sizeof(value));
		ctrl->info(ctrl, &info);
		ctrl->get(ctrl, &value);
		content_len = strlen(buf);
		switch (info.type) {
		case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
			snprintf(buf + content_len, buf_len - content_len, "%s\n", value.value.integer.value[0]? "On" : "Off");
			break;
		case SNDRV_CTL_ELEM_TYPE_INTEGER:
			snprintf(buf + content_len, buf_len - content_len, "%ld\n", value.value.integer.value[0]);
			break;
		case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
			info.value.enumerated.item = value.value.enumerated.item[0];
			ctrl->info(ctrl, &info);
			snprintf(buf + content_len, buf_len - content_len, "%s\n", info.value.enumerated.name);
			break;
		default:
			snprintf(buf + content_len, buf_len - content_len, "unkown type\n");
			break;
		}
	}
}

static void hicodec_debug_kcontrol_dump(char *buf, size_t buf_len)
{
	size_t content_len;
	int i;

	if (!is_valid_codec(g_codec)) {
		snprintf(buf, buf_len, "g_codec is null\n"); /*lint !e747 */
		return;
	}
	snprintf(buf, buf_len, "BEGIN control\n"); /*lint !e747 */

	for (i = 0; i < ASP_CODEC_CNT; i++)
		if(g_codec[i])
			dump_kcontrol(buf, g_codec[i], buf_len);

	content_len = strlen(buf);
	snprintf(buf + content_len, buf_len - content_len, "END\n");
}

/*
 * read kcontrol
 * cat dctrl
 */
static ssize_t hicodec_debug_dctrl_read(struct file *file, char __user *user_buf,
		size_t count, loff_t *ppos)
{
	char *kn_buf;
	ssize_t byte_read;

	kn_buf = vmalloc(HICODEC_DBG_SIZE_CTRL); /*lint !e747 */
	if (NULL == kn_buf) {
		pr_err(LOG_TAG"kn_buf is null\n");
		return simple_read_from_buffer(user_buf, count, ppos, ALLOC_FAIL_MSG, sizeof(ALLOC_FAIL_MSG));
	}

	memset(kn_buf, 0, HICODEC_DBG_SIZE_CTRL);/* unsafe_function_ignore: memset */
	hicodec_debug_kcontrol_dump(kn_buf, HICODEC_DBG_SIZE_CTRL);

	byte_read = simple_read_from_buffer(user_buf, count, ppos, kn_buf, strlen(kn_buf));
	vfree(kn_buf);

	return byte_read;
}

static const struct file_operations hicodec_debug_dctrl_fops = {
	.read  = hicodec_debug_dctrl_read,
};

static void dump_rr_page(char *buf, size_t length, struct snd_soc_component *codec, const struct hicodec_dump_reg_info *info)
{
	unsigned int regi;
	unsigned int i;
	size_t content_len;
	struct hicodec_dump_reg_entry *entry = NULL;

	for (i = 0; i < info->count; ++i) {
		entry = info->entry + i;
		if (entry->seg_name) {
			content_len = strlen(buf);
			snprintf(buf + content_len, length - content_len, "%s\n", entry->seg_name);
		}
		for (regi = entry->start; regi <= entry->end; regi += entry->reg_size) {
			content_len = strlen(buf);
			snprintf(buf + content_len, length - content_len,
				"w 0x%04X 0x%08X\n", regi, snd_soc_component_read32(codec, regi));
		}
	}
}

static void hicodec_debug_rr_page_dump(char *buf, size_t buf_len)
{
	size_t length;
	size_t content_len;
	int i;

#ifdef CONFIG_SND_SOC_ASP_CODEC_CODECLESS
	length = buf_len * ASP_CODEC_CNT;
#else
	length = buf_len;
#endif

	if (!is_valid_codec(g_codec) || (!is_valid_reg_info(reg_info))) {
		snprintf(buf, length, "g_codec or reg_info is null\n"); /*lint !e747 */
		return;
	}
	snprintf(buf, length, "BEGIN rr\n"); /*lint !e747 */

	for (i = 0; i < ASP_CODEC_CNT; i++)
		if(g_codec[i])
			dump_rr_page(buf, length, g_codec[i], reg_info[i]);

	content_len = strlen(buf);
	snprintf(buf + content_len, length - content_len, "\nEND\n");
}

/*
 * read codec register
 * cat rr
 */
static ssize_t hicodec_debug_rr_read(struct file *file, char __user *user_buf,
		size_t count, loff_t *ppos)
{
	char *kn_buf = NULL;
	ssize_t byte_read;
	struct snd_soc_component *codec = NULL;
	size_t length;

	if (!g_codec[ASP_CODECLESS] && !g_codec[ASP_CODEC]) {
		pr_err(LOG_TAG"g_codec is null\n");
		return -EINVAL;
	}

	length = HICODEC_DBG_SIZE_PAGE;
	codec = (struct snd_soc_component *)g_codec[ASP_CODEC];

#ifdef CONFIG_SND_SOC_ASP_CODEC_CODECLESS
	length = HICODEC_DBG_SIZE_PAGE * ASP_CODEC_CNT;
	if((g_vs_reg >= reg_info[ASP_CODECLESS]->entry->start) && (g_vs_reg <= reg_info[ASP_CODECLESS]->entry->end))
		codec = (struct snd_soc_component *)g_codec[ASP_CODECLESS];
#endif

	kn_buf = vmalloc(length); /*lint !e747 */
	if (NULL == kn_buf) {
		pr_err(LOG_TAG"kn_buf is null\n");
		return simple_read_from_buffer(user_buf, count, ppos, ALLOC_FAIL_MSG, sizeof(ALLOC_FAIL_MSG));
	}

	memset(kn_buf, 0, length);/* unsafe_function_ignore: memset */
	if (isReadRegAll) {
		hicodec_debug_rr_page_dump(kn_buf, HICODEC_DBG_SIZE_PAGE);
	} else {
		snprintf(kn_buf + strlen(kn_buf), HICODEC_DBG_SIZE_PAGE - strlen(kn_buf),
				"%#04x:%#010x\n", g_vs_reg, snd_soc_component_read32(codec, g_vs_reg));
	}

	byte_read = simple_read_from_buffer(user_buf, count, ppos, kn_buf, strlen(kn_buf));
	vfree(kn_buf);

	return byte_read;
}
#ifdef CONFIG_HISI_DEBUG_FS
/*
 * Write or read a single register.
 *
 * 1 default cat will output page registers: cat rr
 * 2 write single register: echo "w reg val" > rr
 * 3 read single register:
 * echo "r reg" > rr
 * cat rr
 */
static ssize_t hicodec_debug_rr_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
	char *kn_buf = NULL;
	ssize_t byte_writen;
	int num = 0;
	unsigned int vs_reg = 0;
	unsigned int vs_val = 0;
	struct snd_soc_component *codec = NULL;

	if (!g_codec[ASP_CODECLESS] && !g_codec[ASP_CODEC]) {
		pr_err(LOG_TAG"g_codec is null\n");
		return -EINVAL;
	}

	kn_buf = kzalloc(HICODEC_DBG_SIZE_PAGE, GFP_KERNEL); /*lint !e747 */
	if (NULL == kn_buf) {
		pr_err(LOG_TAG"kn_buf is null\n");
		return -EFAULT;
	}

	byte_writen = simple_write_to_buffer(kn_buf, HICODEC_DBG_SIZE_PAGE, ppos, user_buf, count); /*lint !e747 */
	if (byte_writen != count) { /*lint !e737 */
		pr_err(LOG_TAG"simple_write_to_buffer err:%zd\n", byte_writen);
		byte_writen = -EINVAL;
		kfree(kn_buf);
		return byte_writen;
	}

	switch (kn_buf[0]) {
	case 'w':
		/* write single reg */
		num = sscanf(kn_buf, "w 0x%x 0x%x", &vs_reg, &vs_val);
		codec = (struct snd_soc_component *)g_codec[ASP_CODEC];
#ifdef CONFIG_SND_SOC_ASP_CODEC_CODECLESS
	if((vs_reg >= reg_info[ASP_CODECLESS]->entry->start) && (vs_reg <= reg_info[ASP_CODECLESS]->entry->end))
		codec = (struct snd_soc_component *)g_codec[ASP_CODECLESS];
#endif
		if (2 == num) {
			snd_soc_component_write(codec, vs_reg, vs_val);
			pr_info(LOG_TAG"write single reg, vs_reg:0x%x, value:0x%x", vs_reg, vs_val);
		} else {
			byte_writen = -EINVAL;
			pr_err(LOG_TAG"write single reg failed");
		}
		isReadRegAll = true;

		break;
	case 'r':
		/* read single reg */
		num = sscanf(kn_buf, "r 0x%x", &vs_reg);
		if (1 == num) {
			isReadRegAll = false;
			g_vs_reg = vs_reg;
			pr_info(LOG_TAG"read single reg,vs_reg:0x%x",vs_reg);
		} else {
			isReadRegAll = true;
			byte_writen = -EINVAL;
			pr_err(LOG_TAG"read single reg failed");
		}
		break;
	case 'p':
		/* read PAGE */
		isReadRegAll = true;
		break;
	default:
		/* abnormal */
		isReadRegAll = true;
		byte_writen = -EINVAL;
		break;
	}

	kfree(kn_buf);
	return byte_writen;
}
#endif
static const struct file_operations hicodec_debug_rr_fops = {
	.read  = hicodec_debug_rr_read,
#ifdef CONFIG_HISI_DEBUG_FS
	.write = hicodec_debug_rr_write,
#endif
};

/*******************************************************************************
 * SECTION for init and uninit
 *******************************************************************************/

int hicodec_debug_init(struct snd_soc_component *codec, const struct hicodec_dump_reg_info *info)
{
	if (!codec || !info) {
		pr_err(LOG_TAG"%s: codec or info is null\n", __func__);
		return -EINVAL;
	}

#ifdef CONFIG_SND_SOC_ASP_CODEC_CODECLESS
	if (info->entry->start == DBG_CODECLESS_START_ADDR && info->entry->end == DBG_CODECLESS_END_ADDR) {
		g_codec[ASP_CODECLESS] = codec;
		reg_info[ASP_CODECLESS] = (struct hicodec_dump_reg_info *)info;
	} else {
		g_codec[ASP_CODEC] = codec;
		reg_info[ASP_CODEC] = (struct hicodec_dump_reg_info *)info;
	}
#else
	g_codec[ASP_CODEC] = codec;
	reg_info[ASP_CODEC] = (struct hicodec_dump_reg_info *)info;
#endif

	if (!audio_debug_dir) {
		audio_debug_dir = proc_mkdir(AUDIO_DEBUG_DIR, NULL);
		if (!audio_debug_dir) {
			pr_err(LOG_TAG"%s: fail to create audio proc_fs dir\n", __func__);
			return -ENOMEM;
		}
	} else {
		pr_info(LOG_TAG"proc_fs dir already exists, codec is dual codec\n");
		return 0;
	}
	/* register */
	if (!proc_create("rr", 0640, audio_debug_dir, &hicodec_debug_rr_fops)) {
		pr_err(LOG_TAG"fail to create audio proc_fs rr\n");
	}

	/* register history */
	if (!proc_create("rh", 0640, audio_debug_dir, &hicodec_debug_rh_fops)) {
		pr_err(LOG_TAG"fail to create audio proc_fs cache\n");
	}

	/* dump widget */
	if (!proc_create("dwidget", 0640, audio_debug_dir, &hicodec_debug_dwidget_fops)) {
		pr_err(LOG_TAG"fail to create audio proc_fs dwidget\n");
	}

	/* dump path */
	if (!proc_create("dpath", 0640, audio_debug_dir, &hicodec_debug_dpath_fops)) {
		pr_err(LOG_TAG"fail to create audio proc_fs dpath\n");
	}

	/* dump kcontrol */
	if (!proc_create("dctrl", 0640, audio_debug_dir, &hicodec_debug_dctrl_fops)) {
		pr_err(LOG_TAG"fail to create audio proc_fs dctrl\n");
	}

	rr_cache = (struct hicodec_rr_cache *)kzalloc(sizeof(struct hicodec_rr_cache), GFP_KERNEL);
	if (!rr_cache) {
		pr_err(LOG_TAG"fail to kzalloc rr_cache\n");
		remove_proc_entry(AUDIO_DEBUG_DIR,0);
		audio_debug_dir = NULL;
		return -ENOMEM;
	}

	return 0;
}

void hicodec_debug_uninit(struct snd_soc_component *codec)
{
	if (audio_debug_dir) {
		remove_proc_entry(AUDIO_DEBUG_DIR,0);
		audio_debug_dir = NULL;
	}

	if (rr_cache) {
		kfree(rr_cache);
		rr_cache = NULL;
	}
}
