/*
 * audio_pcm_hifi.c -- ALSA SoC PCM HIFI driver
 *
 * Copyright (c) 2013 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/sched/rt.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
#include <linux/ion.h>
#endif
#include <linux/dma-buf.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/types.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/hwdep.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <uapi/linux/sched/types.h>
#endif
#include "drv_mailbox_cfg.h"

#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif

#include <linux/hisi/audio_log.h>
#include "audio_pcm_hifi.h"
#include "pcm_debug.h"

#ifndef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "audio pcm hifi"
#endif

/*lint -e750 -e785 -e838 -e749 -e747 -e611 -e570 -e647 -e574*/

#define UNUSED_PARAMETER(x) (void)(x)

#define AUIDO_PCM_HIFI "asp-pcm-hifi"
#define AUIDO_PCM_HIFI_COMPONENT_NAME "asp-pcm-hifi-platform"

/*
 * PLAYBACK SUPPORT FORMATS
 * BITS : 8/16/24  18/20
 * LITTLE_ENDIAN / BIG_ENDIAN
 * MONO / STEREO
 * UNSIGNED / SIGNED
 */
#define AUIDO_PCM_PB_FORMATS  (SNDRV_PCM_FMTBIT_S8 | \
		SNDRV_PCM_FMTBIT_U8 | \
		SNDRV_PCM_FMTBIT_S16_LE | \
		SNDRV_PCM_FMTBIT_S16_BE | \
		SNDRV_PCM_FMTBIT_U16_LE | \
		SNDRV_PCM_FMTBIT_U16_BE | \
		SNDRV_PCM_FMTBIT_S24_LE | \
		SNDRV_PCM_FMTBIT_S24_BE | \
		SNDRV_PCM_FMTBIT_U24_LE | \
		SNDRV_PCM_FMTBIT_U24_BE)

/*
 * PLAYBACK SUPPORT RATES
 * 8/11.025/16/22.05/32/44.1/48/88.2/96kHz
 */
#define AUIDO_PCM_PB_RATES    (SNDRV_PCM_RATE_8000_48000 | \
		SNDRV_PCM_RATE_44100 | \
		SNDRV_PCM_RATE_88200 | \
		SNDRV_PCM_RATE_96000 | \
		SNDRV_PCM_RATE_176400 | \
		SNDRV_PCM_RATE_192000 | \
		SNDRV_PCM_RATE_384000)

#define AUIDO_PCM_PB_MIN_CHANNELS  ( 1 )
#define AUIDO_PCM_PB_MAX_CHANNELS  ( 4 )
/* Assume the FIFO size */
#define AUIDO_PCM_PB_FIFO_SIZE     ( 16 )

/* CAPTURE SUPPORT FORMATS : SIGNED 16/24bit */
#define AUIDO_PCM_CP_FORMATS  ( SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE)

/* CAPTURE SUPPORT RATES : 48/96kHz */
#define AUIDO_PCM_CP_RATES    ( SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 )

#define AUIDO_PCM_CP_MIN_CHANNELS  ( 1 )
#define AUIDO_PCM_CP_MAX_CHANNELS 13
/* Assume the FIFO size */
#define AUIDO_PCM_CP_FIFO_SIZE     ( 32 )
#define AUIDO_PCM_MODEM_RATES      ( SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000)
#define AUIDO_PCM_BT_RATES         ( SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 )
#define AUIDO_PCM_FM_RATES         ( SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 )

#define AUIDO_PCM_MAX_BUFFER_SIZE  ( 192 * 1024 )    /* 0x30000 */
#define AUIDO_PCM_BUFFER_SIZE_MM   ( 32 * 1024 )
#define AUIDO_PCM_MIN_BUFFER_SIZE  ( 32 )
#define AUIDO_PCM_MAX_PERIODS      ( 32 )
#define AUIDO_PCM_MIN_PERIODS      ( 2 )
#define AUIDO_PCM_WORK_DELAY_1MS   ( 33 )    /* 33 equals 1ms */
#define AUIDO_PCM_CYC_SUB(Cur, Pre, CycLen)                    \
	(((Cur) < (Pre)) ? (((CycLen) - (Pre)) + (Cur)) : ((Cur) - (Pre)))

#ifndef OK
#define OK              0
#endif
#ifndef ERROR
#define ERROR           -1
#endif

#undef NULL
#define NULL ((void *)0)

#define ALSA_TIMEOUT_MILLISEC 40
#define CHECK_FLAG_TIMEOUT    (160)     /* 5ms */
#define CHECK_UPDATE_TIMEOUT  (350)     /* 10ms */
#define FREQUENTLY_LOG_INTERVAL 100

PCM_DMA_BUF_CONFIG  g_pcm_dma_buf_config[PCM_DEVICE_TOTAL][PCM_STREAM_MAX] = {
	{/*normal*/
		{PCM_DMA_BUF_0_PLAYBACK_BASE, PCM_DMA_BUF_0_PLAYBACK_LEN},
		{PCM_DMA_BUF_0_CAPTURE_BASE, PCM_DMA_BUF_0_CAPTURE_LEN}
	},
	{ {0, 0}, {0, 0} },/*modem*/
	{ {0, 0}, {0, 0} },/*fm*/
	{ {0, 0}, {0, 0} },/*offload*/
	{/*direct*/
		{PCM_DMA_BUF_1_PLAYBACK_BASE, PCM_DMA_BUF_1_PLAYBACK_LEN},
		{PCM_DMA_BUF_1_CAPTURE_BASE, PCM_DMA_BUF_1_CAPTURE_LEN}
	},
	{/*lowlatency*/
		{PCM_DMA_BUF_2_PLAYBACK_BASE, PCM_DMA_BUF_2_PLAYBACK_LEN},
		{PCM_DMA_BUF_2_CAPTURE_BASE, PCM_DMA_BUF_2_CAPTURE_LEN}
	},
	{/*mmap*/
		{PCM_DMA_BUF_3_PLAYBACK_BASE, PCM_DMA_BUF_3_PLAYBACK_LEN},
		{PCM_DMA_BUF_3_CAPTURE_BASE, PCM_DMA_BUF_3_CAPTURE_LEN}
	},
};

/* supported sample rates */
static const unsigned int freq[] = {
	8000,   11025,  12000,  16000,
	22050,  24000,  32000,  44100,
	48000,  88200,  96000,  176400,
	192000, 384000,
};

static const struct snd_soc_component_driver audio_pcm_component = {
	.name   = AUIDO_PCM_HIFI,
};

static u64 audio_pcm_dmamask           = (u64)(0xffffffff);

static struct snd_soc_dai_driver audio_pcm_dai[] =
{
	{
		.name = "asp-pcm-mm",
		.playback = {
			.stream_name  = "asp-pcm-mm Playback",
			.channels_min = AUIDO_PCM_PB_MIN_CHANNELS,
			.channels_max = AUIDO_PCM_PB_MAX_CHANNELS,
			.rates        = AUIDO_PCM_PB_RATES,
			.formats      = AUIDO_PCM_PB_FORMATS
		},
		.capture = {
			.stream_name  = "asp-pcm-mm Capture",
			.channels_min = AUIDO_PCM_CP_MIN_CHANNELS,
			.channels_max = AUIDO_PCM_CP_MAX_CHANNELS,
			.rates        = AUIDO_PCM_CP_RATES,
			.formats      = AUIDO_PCM_CP_FORMATS
		},
	},
	{
		.name = "asp-pcm-modem",
		.playback = {
			.stream_name  = "asp-pcm-modem Playback",
			.channels_min = AUIDO_PCM_PB_MIN_CHANNELS,
			.channels_max = AUIDO_PCM_PB_MAX_CHANNELS,
			.rates        = AUIDO_PCM_MODEM_RATES,
			.formats      = AUIDO_PCM_PB_FORMATS
		},
		.capture = {
			.stream_name  = "asp-pcm-modem Capture",
			.channels_min = AUIDO_PCM_CP_MIN_CHANNELS,
			.channels_max = AUIDO_PCM_CP_MAX_CHANNELS,
			.rates        = AUIDO_PCM_MODEM_RATES,
			.formats      = AUIDO_PCM_CP_FORMATS
		},
	},
	{
		.name = "asp-pcm-fm",
		.playback = {
			.stream_name  = "asp-pcm-fm Playback",
			.channels_min = AUIDO_PCM_PB_MIN_CHANNELS,
			.channels_max = AUIDO_PCM_PB_MAX_CHANNELS,
			.rates        = AUIDO_PCM_FM_RATES,
			.formats      = AUIDO_PCM_PB_FORMATS
		},
	},
	{
		.name = "asp-pcm-lpp",
		.playback = {
			.stream_name  = "asp-pcm-lpp Playback",
			.channels_min = AUIDO_PCM_PB_MIN_CHANNELS,
			.channels_max = AUIDO_PCM_PB_MAX_CHANNELS,
			.rates        = AUIDO_PCM_PB_RATES,
			.formats      = AUIDO_PCM_PB_FORMATS
		},
	},
	{
		.name = "asp-pcm-direct",
		.playback = {
			.stream_name  = "asp-pcm-direct Playback",
			.channels_min = AUIDO_PCM_PB_MIN_CHANNELS,
			.channels_max = AUIDO_PCM_PB_MAX_CHANNELS,
			.rates        = AUIDO_PCM_PB_RATES,
			.formats      = AUIDO_PCM_PB_FORMATS
		},
	},
	{
		.name = "asp-pcm-fast",
		.playback = {
			.stream_name  = "asp-pcm-fast Playback",
			.channels_min = AUIDO_PCM_PB_MIN_CHANNELS,
			.channels_max = AUIDO_PCM_PB_MAX_CHANNELS,
			.rates        = AUIDO_PCM_PB_RATES,
			.formats      = AUIDO_PCM_PB_FORMATS
		},
		.capture = {
			.stream_name  = "asp-pcm-fast Capture",
			.channels_min = AUIDO_PCM_CP_MIN_CHANNELS,
			.channels_max = AUIDO_PCM_CP_MAX_CHANNELS,
			.rates        = AUIDO_PCM_CP_RATES,
			.formats      = AUIDO_PCM_CP_FORMATS
		},
	},
	{
		.name = "audio-pcm-mmap",
		.playback = {
			.stream_name  = "audio-pcm-mmap Playback",
			.channels_min = AUIDO_PCM_PB_MIN_CHANNELS,
			.channels_max = AUIDO_PCM_PB_MAX_CHANNELS,
			.rates        = AUIDO_PCM_PB_RATES,
			.formats      = AUIDO_PCM_PB_FORMATS
		},
		.capture = {
			.stream_name  = "audio-pcm-mmap Capture",
			.channels_min = AUIDO_PCM_CP_MIN_CHANNELS,
			.channels_max = AUIDO_PCM_CP_MAX_CHANNELS,
			.rates        = AUIDO_PCM_CP_RATES,
			.formats      = AUIDO_PCM_CP_FORMATS
		},
	},
};

/* define the capability of playback channel */
static const struct snd_pcm_hardware audio_pcm_hardware_playback =
{
	.info             = SNDRV_PCM_INFO_INTERLEAVED
		| SNDRV_PCM_INFO_NONINTERLEAVED
		| SNDRV_PCM_INFO_MMAP
		| SNDRV_PCM_INFO_MMAP_VALID
		| SNDRV_PCM_INFO_PAUSE,
	.formats          = SNDRV_PCM_FMTBIT_S24_LE,
	.channels_min     = AUIDO_PCM_PB_MIN_CHANNELS,
	.channels_max     = AUIDO_PCM_PB_MAX_CHANNELS,
	.buffer_bytes_max = PCM_DMA_BUF_PLAYBACK_PRIMARY_LEN,
	.period_bytes_min = AUIDO_PCM_MIN_BUFFER_SIZE,
	.period_bytes_max = PCM_DMA_BUF_PLAYBACK_PRIMARY_LEN,
	.periods_min      = AUIDO_PCM_MIN_PERIODS,
	.periods_max      = AUIDO_PCM_MAX_PERIODS,
	.fifo_size        = AUIDO_PCM_PB_FIFO_SIZE,
};

/* define the capability of capture channel */
static const struct snd_pcm_hardware audio_pcm_hardware_capture =
{
	.info             = SNDRV_PCM_INFO_INTERLEAVED,
	.formats          = SNDRV_PCM_FMTBIT_S16_LE,
	.rates            = SNDRV_PCM_RATE_48000,
	.channels_min     = AUIDO_PCM_CP_MIN_CHANNELS,
	.channels_max     = AUIDO_PCM_CP_MAX_CHANNELS,
	.buffer_bytes_max = AUIDO_PCM_MAX_BUFFER_SIZE,
	.period_bytes_min = AUIDO_PCM_MIN_BUFFER_SIZE,
	.period_bytes_max = AUIDO_PCM_MAX_BUFFER_SIZE,
	.periods_min      = AUIDO_PCM_MIN_PERIODS,
	.periods_max      = AUIDO_PCM_MAX_PERIODS,
	.fifo_size        = AUIDO_PCM_CP_FIFO_SIZE,
};

/* define the capability of playback channel for direct*/
static const struct snd_pcm_hardware audio_pcm_hardware_direct_playback =
{
	.info             = SNDRV_PCM_INFO_INTERLEAVED
		| SNDRV_PCM_INFO_NONINTERLEAVED
		| SNDRV_PCM_INFO_MMAP
		| SNDRV_PCM_INFO_MMAP_VALID
		| SNDRV_PCM_INFO_PAUSE,
	.formats          = SNDRV_PCM_FMTBIT_S24_LE,
	.channels_min     = AUIDO_PCM_PB_MIN_CHANNELS,
	.channels_max     = AUIDO_PCM_PB_MAX_CHANNELS,
	.buffer_bytes_max = PCM_DMA_BUF_PLAYBACK_DIRECT_LEN,
	.period_bytes_min = AUIDO_PCM_MIN_BUFFER_SIZE,
	.period_bytes_max = PCM_DMA_BUF_PLAYBACK_DIRECT_LEN,
	.periods_min      = AUIDO_PCM_MIN_PERIODS,
	.periods_max      = AUIDO_PCM_MAX_PERIODS,
	.fifo_size        = AUIDO_PCM_PB_FIFO_SIZE,
};

/* define the capability of playback channel for Modem */
static const struct snd_pcm_hardware audio_pcm_hardware_modem_playback =
{
	.info             = SNDRV_PCM_INFO_INTERLEAVED
		| SNDRV_PCM_INFO_NONINTERLEAVED
		| SNDRV_PCM_INFO_BLOCK_TRANSFER
		| SNDRV_PCM_INFO_PAUSE,
	.formats          = SNDRV_PCM_FMTBIT_S16_LE,
	.channels_min     = AUIDO_PCM_PB_MIN_CHANNELS,
	.channels_max     = AUIDO_PCM_PB_MAX_CHANNELS,
	.buffer_bytes_max = AUIDO_PCM_MAX_BUFFER_SIZE,
	.period_bytes_min = AUIDO_PCM_MIN_BUFFER_SIZE,
	.period_bytes_max = AUIDO_PCM_MAX_BUFFER_SIZE,
	.periods_min      = AUIDO_PCM_MIN_PERIODS,
	.periods_max      = AUIDO_PCM_MAX_PERIODS,
	.fifo_size        = AUIDO_PCM_PB_FIFO_SIZE,
};

/* define the capability of playback channel for lowlatency */
static const struct snd_pcm_hardware audio_pcm_hardware_lowlatency_playback =
{
	.info             = SNDRV_PCM_INFO_INTERLEAVED
		| SNDRV_PCM_INFO_NONINTERLEAVED
		| SNDRV_PCM_INFO_MMAP
		| SNDRV_PCM_INFO_MMAP_VALID
		| SNDRV_PCM_INFO_PAUSE,
	.formats          = SNDRV_PCM_FMTBIT_S24_LE,
	.channels_min     = AUIDO_PCM_PB_MIN_CHANNELS,
	.channels_max     = AUIDO_PCM_PB_MAX_CHANNELS,
	.buffer_bytes_max = PCM_DMA_BUF_PLAYBACK_PRIMARY_LEN,
	.period_bytes_min = AUIDO_PCM_MIN_BUFFER_SIZE,
	.period_bytes_max = PCM_DMA_BUF_PLAYBACK_PRIMARY_LEN,
	.periods_min      = AUIDO_PCM_MIN_PERIODS,
	.periods_max      = AUIDO_PCM_MAX_PERIODS,
	.fifo_size        = AUIDO_PCM_PB_FIFO_SIZE,
};

/* define the capability of capture channel for lowlatency */
static const struct snd_pcm_hardware audio_pcm_hardware_lowlatency_capture =
{
	.info             = SNDRV_PCM_INFO_INTERLEAVED
		| SNDRV_PCM_INFO_NONINTERLEAVED
		| SNDRV_PCM_INFO_MMAP
		| SNDRV_PCM_INFO_MMAP_VALID
		| SNDRV_PCM_INFO_PAUSE,
	.formats          = SNDRV_PCM_FMTBIT_S16_LE,
	.rates            = SNDRV_PCM_RATE_48000,
	.channels_min     = AUIDO_PCM_CP_MIN_CHANNELS,
	.channels_max     = AUIDO_PCM_CP_MAX_CHANNELS,
	.buffer_bytes_max = AUIDO_PCM_MAX_BUFFER_SIZE,
	.period_bytes_min = AUIDO_PCM_MIN_BUFFER_SIZE,
	.period_bytes_max = AUIDO_PCM_MAX_BUFFER_SIZE,
	.periods_min      = AUIDO_PCM_MIN_PERIODS,
	.periods_max      = AUIDO_PCM_MAX_PERIODS,
	.fifo_size        = AUIDO_PCM_CP_FIFO_SIZE,
};

/* define the capability of playback channel for mmap device */
static const struct snd_pcm_hardware audio_pcm_hardware_mmap_playback = {
	.info             = SNDRV_PCM_INFO_INTERLEAVED
		| SNDRV_PCM_INFO_NONINTERLEAVED
		| SNDRV_PCM_INFO_MMAP
		| SNDRV_PCM_INFO_MMAP_VALID
		| SNDRV_PCM_INFO_PAUSE,
	.formats          = SNDRV_PCM_FMTBIT_S16_LE,
	.channels_min     = AUIDO_PCM_PB_MIN_CHANNELS,
	.channels_max     = AUIDO_PCM_PB_MAX_CHANNELS,
	.buffer_bytes_max = PCM_DMA_BUF_MMAP_MAX_SIZE,
	.period_bytes_min = AUIDO_PCM_MIN_BUFFER_SIZE,
	.period_bytes_max = PCM_DMA_BUF_MMAP_MAX_SIZE,
	.periods_min      = AUIDO_PCM_MIN_PERIODS,
	.periods_max      = AUIDO_PCM_MAX_PERIODS,
	.fifo_size        = AUIDO_PCM_PB_FIFO_SIZE,
};

/* define the capability of capture channel for mmap device */
static const struct snd_pcm_hardware audio_pcm_hardware_mmap_capture = {
	.info             = SNDRV_PCM_INFO_INTERLEAVED
		| SNDRV_PCM_INFO_NONINTERLEAVED
		| SNDRV_PCM_INFO_MMAP
		| SNDRV_PCM_INFO_MMAP_VALID
		| SNDRV_PCM_INFO_PAUSE,
	.formats          = SNDRV_PCM_FMTBIT_S16_LE,
	.rates            = SNDRV_PCM_RATE_48000,
	.channels_min     = AUIDO_PCM_CP_MIN_CHANNELS,
	.channels_max     = AUIDO_PCM_CP_MAX_CHANNELS,
	.buffer_bytes_max = PCM_DMA_BUF_MMAP_MAX_SIZE,
	.period_bytes_min = AUIDO_PCM_MIN_BUFFER_SIZE,
	.period_bytes_max = PCM_DMA_BUF_MMAP_MAX_SIZE,
	.periods_min      = AUIDO_PCM_MIN_PERIODS,
	.periods_max      = AUIDO_PCM_MAX_PERIODS,
	.fifo_size        = AUIDO_PCM_CP_FIFO_SIZE,
};

static const struct snd_pcm_hardware *audio_pcm_hw_info[PCM_DEVICE_TOTAL][PCM_STREAM_MAX] = {
	{&audio_pcm_hardware_playback, &audio_pcm_hardware_capture}, /* PCM_DEVICE_NORMAL */
	{NULL, NULL}, /* PCM_DEVICE_MODEM */
	{NULL, NULL}, /* PCM_DEVICE_FM */
	{NULL, NULL}, /* PCM_DEVICE_OFFLOAD */
	{&audio_pcm_hardware_direct_playback, NULL}, /* PCM_DEVICE_DIRECT */
	{&audio_pcm_hardware_lowlatency_playback, &audio_pcm_hardware_lowlatency_capture}, /* PCM_DEVICE_LOW_LATENCY */
	{&audio_pcm_hardware_mmap_playback, &audio_pcm_hardware_mmap_capture}, /* PCM_DEVICE_MMAP */
};

struct pcm_thread_stream_info {
	/* communction paras */
	volatile uint32_t finish_flag;          /* is hifi updated play buffer */
	volatile uint32_t data_buf_offset;      /* new frame data addr offset */
	volatile uint32_t data_buf_size;        /* new frame buffer size */
	/* om info */
	volatile uint32_t set_time;             /* when kernel set new buffer */
	volatile uint32_t get_time;             /* when hifi get new buffer */
	volatile uint32_t check_get_time;       /* thread check finish_flag changed time */
	volatile uint32_t last_get_time;        /* last update buffer time */
	volatile uint32_t data_avail;           /* alsa queued space */
};

struct pcm_hifi_share_data {
	struct pcm_thread_stream_info stream_share_data[PCM_STREAM_MAX];
};

struct pcm_thread_info {
	struct task_struct *pcm_run_thread;
	struct pcm_hifi_share_data *hifi_share_data;  /* communication share mem with hifi */
	atomic_t using_thread_cnt;	/* record the number of substreams using thread */
	struct audio_pcm_runtime_data *runtime_data[PCM_DEVICE_TOTAL][PCM_STREAM_MAX];
};

struct audio_pcm_data {
	struct pcm_thread_info thread_info;	/* currently only lowlatency use */
	int mmap_shared_fd; /* only for mmap use */
	bool low_power_support; /* read from dts */
};

extern int mailbox_get_timestamp(void);
static int audio_pcm_notify_set_buf(struct snd_pcm_substream *substream);
static void audio_pcm_isr_handle(struct snd_pcm_substream *substream,
	uint16_t frame_num, bool discard);

static bool _is_valid_pcm_device(int pcm_device)
{
	switch (pcm_device) {
	case PCM_DEVICE_NORMAL:
	case PCM_DEVICE_DIRECT:
	case PCM_DEVICE_LOW_LATENCY:
	case PCM_DEVICE_MMAP:
		return true;
	default:
		return false;
	}
}

static bool _is_valid_substream(struct snd_pcm_substream *substream)
{
	int pcm_device = 0;
	int pcm_mode = 0;

	if (substream == NULL) {
		AUDIO_LOGE("substream is NULL");
		return false;
	}

	if (substream->runtime == NULL) {
		if (dsp_misc_get_platform_type() == DSP_PLATFORM_ASIC) {
			AUDIO_LOGE("substream runtime is NULL");
		}
		return false;
	}

	pcm_device = substream->pcm->device;
	pcm_mode = substream->stream;

	if (!_is_valid_pcm_device(pcm_device)) {
		AUDIO_LOGE("pcm_device: %d is not support", pcm_device);
		return false;
	}

	if ((pcm_mode != SNDRV_PCM_STREAM_PLAYBACK) &&
		(pcm_mode != SNDRV_PCM_STREAM_CAPTURE)) {
		AUDIO_LOGE("pcm_mode: %d is invalid", pcm_mode);
		return false;
	}

	return true;
}

static bool _is_pcm_device_using_thread(uint16_t pcm_device)
{
	switch (pcm_device) {
	case PCM_DEVICE_LOW_LATENCY:
		return true;
	default:
		return false;
	}
}

static void _dump_thread_info(struct pcm_thread_info *thread_info,
	struct snd_pcm_substream *substream, const char *str)
{
	uint32_t pcm_mode = substream->stream;
	struct pcm_hifi_share_data *share_data = thread_info->hifi_share_data;
	struct pcm_thread_stream_info *stream_info = &share_data->stream_share_data[pcm_mode];

	if (stream_info == NULL) {
		AUDIO_LOGE("pcm mode: %d, thread_share_data is null!", pcm_mode);
		return;
	}

	AUDIO_LOGW("%s: finish_flag-%d, get_time-%d, set_time:%d, last_get_time-%d, check_get_time-%d",
		str,
		stream_info->finish_flag,
		stream_info->get_time,
		stream_info->set_time,
		stream_info->last_get_time,
		stream_info->check_get_time);
}

static bool _is_one_frame_finished(struct pcm_thread_info *thread_info,
	struct snd_pcm_substream *substream)
{
	uint32_t pcm_mode = substream->stream;
	struct pcm_hifi_share_data *share_data = thread_info->hifi_share_data;
	struct pcm_thread_stream_info *stream_info = &share_data->stream_share_data[pcm_mode];
	const char *timeout_str[PCM_STREAM_MAX] = {
		"thread check play timeout",
		"thread check capture timeout"};
	int get_data_interval = 0;

	if (stream_info == NULL) {
		AUDIO_LOGE("pcm thread share data is null!");
		return false;
	}

	if (stream_info->finish_flag) {
		stream_info->finish_flag = false;
		stream_info->check_get_time = mailbox_get_timestamp();

		get_data_interval = stream_info->check_get_time - stream_info->get_time;
		if (get_data_interval > CHECK_FLAG_TIMEOUT) {
			_dump_thread_info(thread_info, substream, timeout_str[pcm_mode]);
		}

		return true;
	}

	return false;
}

static void _thread_update_frame(struct pcm_thread_info *thread_info,
	bool *should_schedule)
{
	uint32_t stream = 0;
	uint32_t device = 0;
	struct audio_pcm_runtime_data *prtd = NULL;

	for (device = 0; device < PCM_DEVICE_TOTAL; device++) {
		for (stream = 0; stream < PCM_STREAM_MAX; stream++) {
			if (!_is_pcm_device_using_thread(device))
				continue;

			prtd = thread_info->runtime_data[device][stream];
			if (prtd == NULL)
				continue;

			if (prtd->status != STATUS_RUNNING)
				continue;

			if (_is_one_frame_finished(thread_info, prtd->substream)) {
				audio_pcm_isr_handle(prtd->substream, 1, false);
				*should_schedule = false;
			}
		}
	}
}

static int pcm_check_frame_thread(void *data)
{
	struct pcm_thread_info *thread_info = data;
	bool should_schedule = false;

	while (!kthread_should_stop()) {
		/* if playback & capture stream are all closed, thread go to sleep,
		 * and wait to be wakeup until next trigger start.
		 */
		if (atomic_read(&thread_info->using_thread_cnt) == 0) {
			AUDIO_LOGI("set lowlatency check frame thread to sleep state!");
			set_current_state(TASK_INTERRUPTIBLE); /*lint !e446 !e666*/
			schedule();
		}

		should_schedule = true;

		_thread_update_frame(thread_info, &should_schedule);

		if (should_schedule)
			usleep_range(1000, 1100);
	}

	return 0;
}

static void pcm_thread_stop(struct snd_pcm *pcm)
{
	struct snd_soc_pcm_runtime *rtd = pcm->private_data;
	struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AUIDO_PCM_HIFI_COMPONENT_NAME);
	struct audio_pcm_data *pdata = snd_soc_component_get_drvdata(component);
	struct pcm_thread_info *thread_info = &pdata->thread_info;
	bool is_stream_open = pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream_opened ||
		pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream_opened;

	if (is_stream_open)
		return;

	if (thread_info->pcm_run_thread) {
		kthread_stop(thread_info->pcm_run_thread);
		thread_info->pcm_run_thread = NULL;

		if (thread_info->hifi_share_data) {
			iounmap(thread_info->hifi_share_data);
			thread_info->hifi_share_data = NULL;
		}

		AUDIO_LOGI("pcm thread has stopped");
	}
}

static int pcm_thread_start(struct snd_pcm *pcm)
{
	int ret;
	struct sched_param param = {.sched_priority = MAX_RT_PRIO - 1};
	struct pcm_hifi_share_data *thread_share_data = NULL;
	struct snd_soc_pcm_runtime *rtd = pcm->private_data;
	struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AUIDO_PCM_HIFI_COMPONENT_NAME);
	struct audio_pcm_data *pdata = snd_soc_component_get_drvdata(component);

	struct pcm_thread_info *thread_info = &pdata->thread_info;

	if (thread_info->pcm_run_thread) {
		AUDIO_LOGI("pcm thread has already running, not need init");
		return 0;
	}

	/* share memory remap */
	thread_share_data = ioremap_wc(DRV_DSP_PCM_THREAD_DATA_ADDR, sizeof(struct pcm_hifi_share_data));
	if (!thread_share_data) {
		AUDIO_LOGE("pcm share mem iormap failed!");
		return -ENOMEM;
	}
	memset(thread_share_data, 0, sizeof(*thread_share_data));/* unsafe_function_ignore: memset */

	/* create pcm thread for communication with hifi */
	thread_info->pcm_run_thread = kthread_create(pcm_check_frame_thread,
		(void *)thread_info, "pcm_run_thread");
	if (IS_ERR(thread_info->pcm_run_thread)) {
		AUDIO_LOGE("create check frame thread failed!");
		thread_info->pcm_run_thread = NULL;
		iounmap(thread_share_data);
		return -ENOMEM;
	}

	thread_info->hifi_share_data = thread_share_data;

	/* set highest rt prio */
	ret = sched_setscheduler(thread_info->pcm_run_thread, SCHED_FIFO, &param);
	if (ret)
		AUDIO_LOGE("set thread schedule priority failed");

	/* do not wakeup process in this stage, wakeup when lowlatency stream start */
	AUDIO_LOGI("create pcm_run_thread success");

	return 0;
}

static int pcm_set_share_data(
	struct snd_pcm_substream *substream, uint32_t data_addr, uint32_t data_len)
{
	int pcm_mode = substream->stream;
	struct snd_soc_pcm_runtime *soc_prtd = substream->private_data;
	struct snd_soc_component *component = snd_soc_rtdcom_lookup(soc_prtd, AUIDO_PCM_HIFI_COMPONENT_NAME);
	struct audio_pcm_data *pdata = snd_soc_component_get_drvdata(component);
	struct pcm_thread_info *thread_info = &pdata->thread_info;
	struct pcm_hifi_share_data *share_data = thread_info->hifi_share_data;
	struct pcm_thread_stream_info *stream_info = &share_data->stream_share_data[pcm_mode];
	const char *timeout_str[PCM_STREAM_MAX] = {
		"set play addr timeout",
		"set capture addr timeout"};
	int set_data_interval = 0;

	if (stream_info == NULL) {
		AUDIO_LOGE("pcm mode: %d, thread_share_data is null!", pcm_mode);
		return -EINVAL;
	}

	stream_info->data_buf_offset = data_addr;
	stream_info->data_buf_size = data_len;
	stream_info->last_get_time = stream_info->set_time;
	stream_info->set_time = mailbox_get_timestamp();

	set_data_interval = stream_info->set_time - stream_info->last_get_time;
	if (set_data_interval > CHECK_UPDATE_TIMEOUT)
		_dump_thread_info(thread_info, substream, timeout_str[pcm_mode]);

	return 0;
}

static int audio_pcm_get_mmap_buf_phys(struct device *dev, int shared_fd,
	dma_addr_t *addr)
{
	struct sg_table *table = NULL;
	struct dma_buf *buf = NULL;
	struct dma_buf_attachment *attach = NULL;

	if (shared_fd < 0) {
		AUDIO_LOGE("share fd is invalid: %d", shared_fd);
		return -EFAULT;
	}

	buf = dma_buf_get(shared_fd);
	if (IS_ERR(buf)) {
		AUDIO_LOGE("buf can not be get from fd: %d", shared_fd);
		return -EFAULT;
	}

	attach = dma_buf_attach(buf, dev);
	if (IS_ERR(attach)) {
		AUDIO_LOGE("dmabuf attach failed");
		dma_buf_put(buf);
		return -EFAULT;
	}

	table = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR_OR_NULL(table)) {
		AUDIO_LOGE("dmabuf map attachment failed");
		dma_buf_detach(buf, attach);
		dma_buf_put(buf);
		return -EFAULT;
	}

	*addr = sg_phys(table->sgl);

	dma_buf_unmap_attachment(attach, table, DMA_BIDIRECTIONAL);
	dma_buf_detach(buf, attach);
	dma_buf_put(buf);

	return 0;
}

static int audio_pcm_get_mmap_share_buf(struct snd_pcm_substream *substream,
	uint32_t buf_size)
{
	int ret = -EINVAL;
	struct audio_pcm_runtime_data *prtd = substream->runtime->private_data;
	struct snd_soc_pcm_runtime *soc_prtd = substream->private_data;
	struct snd_soc_component *component = snd_soc_rtdcom_lookup(soc_prtd, AUIDO_PCM_HIFI_COMPONENT_NAME);
	struct audio_pcm_data *pdata = snd_soc_component_get_drvdata(component);
	struct device *dev = component->dev;
	int shared_fd = pdata->mmap_shared_fd;
	struct audio_pcm_mmap_buf *mmap_buf = NULL;
	int pcm_device = substream->pcm->device;
	int pcm_mode = substream->stream;

	if (buf_size == 0 || buf_size > PCM_DMA_BUF_MMAP_MAX_SIZE) {
		AUDIO_LOGE("buf size %d error", buf_size);
		return ret;
	}

	if (shared_fd < 0) {
		AUDIO_LOGE("mmap share fd is invalid: %d", shared_fd);
		return -EBADFD;
	}

	mmap_buf = kzalloc(sizeof(*mmap_buf), GFP_KERNEL);
	if (mmap_buf == NULL) {
		AUDIO_LOGE("mmap buf malloc fail");
		return -ENOMEM;
	}

	mmap_buf->buf_size = buf_size;

	/* get share buffer phy address */
	ret = audio_pcm_get_mmap_buf_phys(dev, shared_fd, &mmap_buf->phy_addr);
	if (ret) {
		AUDIO_LOGE("failed to get buf phys");
		goto err_buf_addr;
	}

	mmap_buf->dmabuf = dma_buf_get(shared_fd);
	if (IS_ERR(mmap_buf->dmabuf)) {
		AUDIO_LOGE("dmabuf get failed");
		ret = - ENOMEM;
		goto err_buf_addr;
	}
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	ret = dma_buf_begin_cpu_access(mmap_buf->dmabuf, DMA_BIDIRECTIONAL);
	if (ret) {
		AUDIO_LOGE("dma buf begin cpu access failed !");
		goto err_access_dmabuf;
	}
#endif
	/* get buffer virt address */
	mmap_buf->buf_addr = dma_buf_kmap(mmap_buf->dmabuf, 0);
	if (mmap_buf->buf_addr == NULL) {
		AUDIO_LOGE("device:%d mode:%d failed to map dma buf",
			pcm_device, pcm_mode);
		ret = - ENOMEM;
		goto err_buf_map;
	}

	memset((void *)mmap_buf->buf_addr, 0, mmap_buf->buf_size);/* unsafe_function_ignore: memset */
	prtd->mmap_buf = mmap_buf;

	return ret;

err_buf_map:
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	dma_buf_end_cpu_access(mmap_buf->dmabuf, DMA_BIDIRECTIONAL);
err_access_dmabuf:
#endif
	dma_buf_put(mmap_buf->dmabuf);
	mmap_buf->dmabuf = NULL;
err_buf_addr:
	kfree(mmap_buf);

	return ret;
}

static int audio_pcm_free_mmap_share_buf(struct snd_pcm_substream *substream)
{
	struct audio_pcm_runtime_data *prtd = substream->runtime->private_data;
	struct snd_soc_pcm_runtime *soc_prtd = substream->private_data;
	struct snd_soc_component *component = snd_soc_rtdcom_lookup(soc_prtd, AUIDO_PCM_HIFI_COMPONENT_NAME);
	struct audio_pcm_data *pdata = snd_soc_component_get_drvdata(component);
	struct audio_pcm_mmap_buf *mmap_buf = prtd->mmap_buf;
	int ret = 0;

	if (mmap_buf == NULL) {
		AUDIO_LOGE("mmap buf is invalid");
		return -EINVAL;
	}

	if (mmap_buf->buf_addr == NULL) {
		AUDIO_LOGE("mmap buf addr is invalid");
		return -EINVAL;
	}

	if (mmap_buf->dmabuf == NULL) {
		AUDIO_LOGE("dma buf is invalid");
		return -EINVAL;
	}

	dma_buf_kunmap(mmap_buf->dmabuf, 0, mmap_buf->buf_addr);
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	ret = dma_buf_end_cpu_access(mmap_buf->dmabuf, DMA_BIDIRECTIONAL);
	if (ret < 0)
		AUDIO_LOGE("dma buf end cpu access failed");
#endif
	dma_buf_put(mmap_buf->dmabuf);

	mmap_buf->dmabuf = NULL;
	mmap_buf->buf_addr = NULL;

	kfree(prtd->mmap_buf);
	prtd->mmap_buf = NULL;
	pdata->mmap_shared_fd = -1;

	return ret;
}

static int audio_pcm_hwdep_ioctl_shared_fd(struct snd_pcm *pcm,
	unsigned long arg)
{
	struct audio_pcm_mmap_fd mmap_fd;
	struct snd_soc_pcm_runtime *rtd = pcm->private_data;
	struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AUIDO_PCM_HIFI_COMPONENT_NAME);
	struct audio_pcm_data *pdata = snd_soc_component_get_drvdata(component);
	unsigned int buffer_size = 0;
	int pcm_mode = -1;

	memset(&mmap_fd, 0, sizeof(mmap_fd));/* unsafe_function_ignore: memset */
	if (copy_from_user(&mmap_fd, (void __user *)(uintptr_t)arg,
			   sizeof(mmap_fd))) {
		AUDIO_LOGE("copying mmap_fd from user fail");
		return -EFAULT;
	}

	pcm_mode = mmap_fd.stream_direction;
	if (pcm_mode != SNDRV_PCM_STREAM_PLAYBACK &&
		pcm_mode != SNDRV_PCM_STREAM_CAPTURE) {
		AUDIO_LOGE("pcm mode is invalid: %d", pcm_mode);
		return -EINVAL;
	}

	pdata->mmap_shared_fd = mmap_fd.shared_fd;
	if (pdata->mmap_shared_fd < 0) {
		AUDIO_LOGE("mmap share fd is invalid");
		return -EFAULT;
	}

	buffer_size = mmap_fd.buf_size;
	if (buffer_size == 0 || buffer_size > PCM_DMA_BUF_MMAP_MAX_SIZE) {
		AUDIO_LOGE("buffer_size is invalid");
		return -EFAULT;
	}

	AUDIO_LOGD("device: %d mode: %d share fd: %d, buf size: %d",
		pcm->device, pcm_mode, pdata->mmap_shared_fd, buffer_size);

	return 0;
}

/* add custom pcm ioctl cmd here */
static int audio_pcm_hwdep_ioctl(struct snd_hwdep *hw, struct file *file,
			       unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct snd_pcm *pcm = hw->private_data;

	if (!(void __user *)(uintptr_t)arg) {
		AUDIO_LOGE("input buff is NULL");
		return -EINVAL;
	}

	switch (cmd) {
	case AUIDO_PCM_IOCTL_MMAP_SHARED_FD:
		return audio_pcm_hwdep_ioctl_shared_fd(pcm, arg);
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int audio_pcm_add_hwdep_dev(struct snd_soc_pcm_runtime *runtime)
{
	struct snd_hwdep *hwdep = NULL;
	int ret = 0;
	char device[] = "MMAP_xx";

	snprintf(device, sizeof(device), "MMAP_%d", runtime->pcm->device);
	AUDIO_LOGD("add hwdep node for pcm device %d", runtime->pcm->device);

	ret = snd_hwdep_new(runtime->card->snd_card,
			   &device[0], runtime->pcm->device, &hwdep);
	if (!hwdep || ret < 0) {
		AUDIO_LOGE("hwdep intf failed to create %s - hwdep", device);
		return ret;
	}

	hwdep->iface = SND_HWDEP_IFACE_PCM;
	hwdep->private_data = runtime->pcm;
	hwdep->ops.ioctl = audio_pcm_hwdep_ioctl;

	return 0;
}

static int map_hifi_share_buffer(struct snd_pcm_substream *substream)
{
	struct audio_pcm_runtime_data *prtd = substream->runtime->private_data;
	struct pcm_ap_hifi_buf *buf = &prtd->hifi_buf;
	int pcm_device = substream->pcm->device;
	int pcm_mode = substream->stream;

	if (pcm_device != PCM_DEVICE_MMAP)
		return 0;

	buf->addr = g_pcm_dma_buf_config[pcm_device][pcm_mode].pcm_dma_buf_base;
	buf->bytes = g_pcm_dma_buf_config[pcm_device][pcm_mode].pcm_dma_buf_len;
	buf->area = ioremap(buf->addr, buf->bytes);

	if (!buf->area) {
		AUDIO_LOGE("ap-hifi buf area error");
		return -ENOMEM;
	}

	return 0;
}

static void unmap_hifi_share_buffer(struct snd_pcm_substream *substream)
{
	struct audio_pcm_runtime_data *prtd = substream->runtime->private_data;
	struct pcm_ap_hifi_buf *buf = &prtd->hifi_buf;
	int pcm_device = substream->pcm->device;

	if (pcm_device != PCM_DEVICE_MMAP)
		return;

	if (!buf->area)
		return;

	iounmap(buf->area);

	buf->area = NULL;
	buf->addr = 0;
}

static int substream_param_check(struct snd_pcm_substream *substream)
{
	struct audio_pcm_runtime_data *prtd = NULL;

	if (substream == NULL) {
		AUDIO_LOGE("substream is null");
		return IRQ_HDD_PTR;
	}

	if (substream->runtime == NULL) {
		AUDIO_LOGE("runtime is null");
		return IRQ_HDD_PTR;
	}

	if (substream->runtime->private_data == NULL) {
		AUDIO_LOGE("private data is null");
		return IRQ_HDD_PTR;
	}

	prtd = substream->runtime->private_data;
	if (prtd->status != STATUS_RUNNING) {
		AUDIO_LOGD("status %d error", prtd->status);
		return IRQ_HDD_STATUS;
	}

	return 0;
}

static void mmap_update_appl_ptr(struct snd_pcm_substream *substream)
{
	struct audio_pcm_runtime_data *prtd = NULL;
	int pcm_mode;
	int pcm_device;
	snd_pcm_uframes_t rt_period_size;

	prtd = substream->runtime->private_data;
	pcm_mode = substream->stream;
	pcm_device = substream->pcm->device;
	rt_period_size = substream->runtime->period_size;

	if (pcm_device != PCM_DEVICE_MMAP) {
		return;
	}

	switch (pcm_mode) {
	case SNDRV_PCM_STREAM_PLAYBACK:
		prtd->frame_counter += rt_period_size;
		substream->runtime->control->appl_ptr = prtd->frame_counter + rt_period_size;
		break;
	case SNDRV_PCM_STREAM_CAPTURE:
		prtd->frame_counter += rt_period_size;
		substream->runtime->control->appl_ptr = prtd->frame_counter - rt_period_size;
		break;
	default:
		AUDIO_LOGE("pcm mode error");
		break;
	}

	substream->runtime->control->appl_ptr %= substream->runtime->boundary;
}

static void update_pcm_buff(struct snd_pcm_substream *substream, bool discard)
{
	struct audio_pcm_runtime_data *prtd = NULL;
	int pcm_mode;
	int pcm_device;
	snd_pcm_sframes_t rt_period_size;
	unsigned int num_period;
	snd_pcm_sframes_t avail;
	int ret;

	prtd = substream->runtime->private_data;
	pcm_mode = substream->stream;
	pcm_device = substream->pcm->device;
	rt_period_size = (snd_pcm_sframes_t)substream->runtime->period_size;
	num_period = substream->runtime->periods;

	if (SNDRV_PCM_STREAM_CAPTURE == pcm_mode) {
		avail = snd_pcm_capture_hw_avail(substream->runtime);
	} else {
		avail = snd_pcm_playback_hw_avail(substream->runtime);

		if (prtd->is_support_low_power && avail == rt_period_size)
			return;
	}

	if (avail < rt_period_size) {
		if (dsp_misc_get_platform_type() == DSP_PLATFORM_ASIC)
			AUDIO_LOGW("underrun, device is %d, mode is %d, avail[%ld] < period size[%ld]",
				pcm_device, pcm_mode, avail, rt_period_size);
		return;
	}

	spin_lock(&prtd->lock);
	prtd->period_cur = (prtd->period_cur + 1) % num_period;
	spin_unlock(&prtd->lock);

	snd_pcm_period_elapsed(substream);

	if (!discard) {
		ret = audio_pcm_notify_set_buf(substream);
		if (ret < 0)
			AUDIO_LOGE("mode is %d, device is %d: notify set buf fail, ret is %d",
				pcm_mode, pcm_device, ret);
	}

	spin_lock(&prtd->lock);
	prtd->period_next = (prtd->period_next + 1) % num_period;
	spin_unlock(&prtd->lock);
}

static void audio_pcm_isr_handle(struct snd_pcm_substream *substream,
	uint16_t frame_num, bool discard)
{
	uint16_t i;

	if (substream_param_check(substream) != 0)
		return;

	if (frame_num > UPDATE_FRAME_NUM_MAX) {
		AUDIO_LOGE("frame_num error, %u", frame_num);
		return;
	}

	for (i = 0; i < frame_num; i++) {
		mmap_update_appl_ptr(substream);
		update_pcm_buff(substream, discard);
	}
}

static int audio_pcm_mailbox_send_data(const void *pmsg_body, unsigned int msg_len,
		unsigned int msg_priority)
{
	unsigned int ret = 0;
	static unsigned int err_count = 0;

	ret = DRV_MAILBOX_SENDMAIL(MAILBOX_MAILCODE_ACPU_TO_HIFI_AUDIO, pmsg_body, msg_len);
	if (ret != 0) {
		if (err_count % 50 == 0)
			AUDIO_LOGE("send mail error, %u", ret);

		err_count++;
	} else {
		err_count = 0;
	}

	return (int)ret;
}

static int audio_pcm_notify_set_buf(struct snd_pcm_substream *substream)
{
	int ret = 0;
	unsigned int period_size;
	struct hifi_channel_set_buffer msg_body = {0};
	unsigned short pcm_mode = (unsigned short)substream->stream;
	int pcm_device = substream->pcm->device;
	struct audio_pcm_runtime_data *prtd = (struct audio_pcm_runtime_data *)substream->runtime->private_data;
	struct audio_pcm_mmap_buf *mmap_buf = NULL;
	uint32_t data_phy_addr = 0;
	uint32_t data_offset_addr = 0;

	IN_FUNCTION;

	if (NULL == prtd) {
		AUDIO_LOGE("prtd is null, error");
		return -EINVAL;
	}

	if ((SNDRV_PCM_STREAM_PLAYBACK != pcm_mode) && (SNDRV_PCM_STREAM_CAPTURE != pcm_mode)) {
		AUDIO_LOGE("pcm mode %d invalid", pcm_mode);
		return -EINVAL;
	}

	/* transfer frame data between dma buffer and hifi buffer */
	mmap_buf = prtd->mmap_buf;
	if (pcm_device == PCM_DEVICE_MMAP) {
		if (prtd->hifi_buf.bytes < mmap_buf->buf_size) {
			AUDIO_LOGE("mmap buf size %d is larger than hifi buf %zu",
				mmap_buf->buf_size, prtd->hifi_buf.bytes);
			return -EINVAL;
		}

		if (pcm_mode == SNDRV_PCM_STREAM_PLAYBACK)
			memcpy(prtd->hifi_buf.area, mmap_buf->buf_addr, mmap_buf->buf_size);
		else
			memcpy(mmap_buf->buf_addr, prtd->hifi_buf.area, mmap_buf->buf_size);
	}

	period_size = prtd->period_size;
	if (pcm_device == PCM_DEVICE_MMAP) {
		data_phy_addr = prtd->hifi_buf.addr + prtd->period_next * period_size;
	} else {
		data_phy_addr = substream->runtime->dma_addr + prtd->period_next * period_size;
	}
	data_offset_addr = data_phy_addr - PCM_DMA_BUF_0_PLAYBACK_BASE;

	msg_body.msg_type   = (unsigned short)HI_CHN_MSG_PCM_SET_BUF;
	msg_body.pcm_mode   = pcm_mode;
	msg_body.pcm_device = (unsigned short)pcm_device;
	msg_body.data_addr = data_offset_addr;
	msg_body.data_len  = period_size;

	if (STATUS_RUNNING != prtd->status) {
		AUDIO_LOGD("pcm status %d error", prtd->status);
		return -EINVAL;
	}

	if (prtd->using_thread_flag) {
		ret = pcm_set_share_data(substream, msg_body.data_addr, msg_body.data_len);
	} else {
		ret = audio_pcm_mailbox_send_data(&msg_body, sizeof(msg_body), 0);
	}
	if (ret != OK)
		ret = -EBUSY;

	OUT_FUNCTION;

	return ret;
}

static void print_pcm_timeout(unsigned int pre_time, const char *print_type, unsigned int time_delay)
{
	unsigned int delay_time;
	unsigned int curr_time;

	if (dsp_misc_get_platform_type() != DSP_PLATFORM_ASIC)
		return;

	curr_time = (unsigned int)mailbox_get_timestamp();
	delay_time = curr_time - pre_time;

	if (delay_time > (AUIDO_PCM_WORK_DELAY_1MS * time_delay))
		AUDIO_LOGW("%s, delay time stamp diff is %u", print_type, delay_time);

}

static void audio_pcm_notify_recv_proc(const void *usr_para,
	struct mb_queue *mail_handle, unsigned int mail_len)
{
	struct snd_pcm_substream * substream = NULL;
	struct audio_pcm_runtime_data *prtd = NULL;
	struct hifi_chn_pcm_period_elapsed mail_buf;
	unsigned int mail_size = mail_len;
	unsigned int ret_mail;
	unsigned int start_time;

	UNUSED_PARAMETER(usr_para);

	memset(&mail_buf, 0, sizeof(struct hifi_chn_pcm_period_elapsed));/* unsafe_function_ignore: memset */
	/* get the data from mailbox */
	ret_mail = DRV_MAILBOX_READMAILDATA(mail_handle, (unsigned char*)&mail_buf, &mail_size);
	if ((ret_mail != 0) ||
		(mail_size == 0) ||
		(mail_size > sizeof(struct hifi_chn_pcm_period_elapsed))) {
		AUDIO_LOGE("mailbox read error, read result:%u, read mail size:%u",
			ret_mail, mail_size);
		return;
	}

	substream = INT_TO_ADDR(mail_buf.substream_l32,mail_buf.substream_h32);
	if (!_is_valid_substream(substream))
		return;

	prtd = (struct audio_pcm_runtime_data *)substream->runtime->private_data;
	if (NULL == prtd) {
		AUDIO_LOGE("prtd is NULL");
		return;
	}

	if (STATUS_STOP == prtd->status) {
		AUDIO_LOGI("process has stopped");
		return;
	}

	switch(mail_buf.msg_type) {
		case HI_CHN_MSG_PCM_PERIOD_ELAPSED:
			print_pcm_timeout(mail_buf.msg_timestamp, "ap respond elapsed msg timeout", RESPOND_MAX_TIME);
			start_time = (unsigned int)mailbox_get_timestamp();
			audio_pcm_isr_handle(substream, mail_buf.frame_num, (bool)mail_buf.discard);
			print_pcm_timeout(start_time, "ap process elapsed msg timeout", PROCESS_MAX_TIME);
			break;
		case HI_CHN_MSG_PCM_PERIOD_STOP:
			if (STATUS_STOPPING == prtd->status) {
				prtd->status = STATUS_STOP;
				AUDIO_LOGI("device %d mode %d stop now !", substream->pcm->device, mail_buf.pcm_mode);
			}
			break;
		default:
			AUDIO_LOGE("msg_type 0x%x", mail_buf.msg_type);
			break;
	}
}

static int audio_pcm_notify_isr_register(mb_msg_cb receive_func)
{
	unsigned int ret = 0;

	if (receive_func == NULL) {
		AUDIO_LOGE("receive func is null");
		return ERROR;
	}

	ret = DRV_MAILBOX_REGISTERRECVFUNC(MAILBOX_MAILCODE_HIFI_TO_ACPU_AUDIO,
		receive_func, NULL);
	if (ret != 0) {
		AUDIO_LOGE("register receive func error, ret:%u, mailcode:0x%x",
			ret, MAILBOX_MAILCODE_HIFI_TO_ACPU_AUDIO);
		return ERROR;
	}

	return 0;
}

static int audio_pcm_notify_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	int ret = 0;
	struct hifi_chn_pcm_open open_msg = {0};
	unsigned int params_value = 0;
	unsigned int infreq_index = 0;
	struct audio_pcm_runtime_data *prtd = substream->runtime->private_data;

	IN_FUNCTION;

	open_msg.msg_type = (unsigned short)HI_CHN_MSG_PCM_OPEN;
	open_msg.pcm_mode = (unsigned short)substream->stream;
	open_msg.pcm_device = (unsigned short)substream->pcm->device;
	open_msg.is_support_low_power = (unsigned short)prtd->is_support_low_power;

	/* check channels  : mono or stereo */
	params_value = params_channels(params);
	if ((AUIDO_PCM_CP_MIN_CHANNELS <= params_value) && (AUIDO_PCM_CP_MAX_CHANNELS >= params_value)) {
		open_msg.config.channels = params_value;
	} else {
		AUDIO_LOGE("DAC not support %d channels", params_value);
		return -EINVAL;
	}

	/* check samplerate */
	params_value = params_rate(params);

	for (infreq_index = 0; infreq_index < ARRAY_SIZE(freq); infreq_index++) {
		if(params_value == freq[infreq_index])
			break;
	}

	if (ARRAY_SIZE(freq) <= infreq_index) {
		AUDIO_LOGE("rate %d not support", params_value);
		return -EINVAL;
	}

	open_msg.config.rate = params_value;

	/* check format */
	params_value = (unsigned int)params_format(params);
	if (params_value == SNDRV_PCM_FORMAT_S24_LE) {
		params_value = PCM_FORMAT_S24_LE_LA;
	} else {
		params_value = PCM_FORMAT_S16_LE;
	}

	open_msg.config.format = params_value;
	open_msg.config.period_size = params_period_size(params);
	open_msg.config.period_count = params_periods(params);

	AUDIO_LOGI("device %u, mode %u, low power support %u,"
		"channels %u, rate %u, period size %u, period count %u, format %u",
		open_msg.pcm_device, open_msg.pcm_mode, open_msg.is_support_low_power,
		open_msg.config.channels, open_msg.config.rate,
		open_msg.config.period_size, open_msg.config.period_count,
		open_msg.config.format);

	ret = audio_pcm_mailbox_send_data(&open_msg, sizeof(open_msg), 0);

	OUT_FUNCTION;

	return ret;
}

static int audio_pcm_notify_hw_free(struct snd_pcm_substream *substream)
{
	int ret = 0;

	UNUSED_PARAMETER(substream);

	return ret;
}

static int audio_pcm_notify_prepare(struct snd_pcm_substream *substream)
{
	int ret = OK;

	UNUSED_PARAMETER(substream);

	return ret;
}

static int audio_pcm_notify_trigger(int cmd, struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct hifi_chn_pcm_trigger msg_body = {0};
	uint32_t period_size = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct audio_pcm_runtime_data *prtd = runtime->private_data;
	struct snd_soc_pcm_runtime *soc_prtd = substream->private_data;
	struct snd_soc_component *component = snd_soc_rtdcom_lookup(soc_prtd, AUIDO_PCM_HIFI_COMPONENT_NAME);
	struct audio_pcm_data *pdata = snd_soc_component_get_drvdata(component);
	struct pcm_thread_info *thread_info = &pdata->thread_info;
	struct audio_pcm_mmap_buf *mmap_buf = NULL;
	int pcm_mode = substream->stream;
	int pcm_device = substream->pcm->device;
	uint32_t data_phy_addr = 0;
	uint32_t data_offset_addr = 0;

	IN_FUNCTION;

	if (NULL == prtd) {
		AUDIO_LOGE("prtd is null");
		return -EINVAL;
	}

	msg_body.msg_type   	= (unsigned short)HI_CHN_MSG_PCM_TRIGGER;
	msg_body.pcm_mode   	= (unsigned short)pcm_mode;
	msg_body.pcm_device   	= (unsigned short)pcm_device;
	msg_body.tg_cmd     	= (unsigned short)cmd;
	msg_body.substream_l32  = GET_LOW32((uintptr_t)substream);
	msg_body.substream_h32  = GET_HIG32((uintptr_t)substream);

	if ((SNDRV_PCM_TRIGGER_START == cmd)
		|| (SNDRV_PCM_TRIGGER_RESUME == cmd)
		|| (SNDRV_PCM_TRIGGER_PAUSE_RELEASE == cmd)) {

		/* transfer frame data between dma buffer and hifi buffer */
		mmap_buf = prtd->mmap_buf;
		if (pcm_device == PCM_DEVICE_MMAP) {
			if (pcm_mode == SNDRV_PCM_STREAM_PLAYBACK)
				memcpy(prtd->hifi_buf.area, mmap_buf->buf_addr, mmap_buf->buf_size);
			else
				memcpy(mmap_buf->buf_addr, prtd->hifi_buf.area, mmap_buf->buf_size);

			spin_lock(&prtd->lock);
			prtd->period_cur = (prtd->period_cur + 1) % substream->runtime->periods;
			spin_unlock(&prtd->lock);

			runtime->status->hw_ptr += substream->runtime->period_size;
		}

		period_size = prtd->period_size;
		if (pcm_device == PCM_DEVICE_MMAP) {
			data_phy_addr = prtd->hifi_buf.addr + prtd->period_next * period_size;
		} else {
			data_phy_addr = runtime->dma_addr + prtd->period_next * period_size;
		}
		data_offset_addr = data_phy_addr - PCM_DMA_BUF_0_PLAYBACK_BASE;

		msg_body.data_addr = data_offset_addr;
		msg_body.data_len  = period_size;
	}

	/* update share memory info */
	if (prtd->using_thread_flag && (cmd == SNDRV_PCM_TRIGGER_START)) {
		ret = wake_up_process(thread_info->pcm_run_thread);
		AUDIO_LOGI("lowlatency check frame thread wake up %s!", ret ? "success" : "fail");

		ret = pcm_set_share_data(substream, msg_body.data_addr, msg_body.data_len);
		if (ret)
			AUDIO_LOGE("set share data fail, ret:%d!", ret);
	}

	ret = audio_pcm_mailbox_send_data(&msg_body, sizeof(msg_body), 0);

	OUT_FUNCTION;

	return ret;
}

static int audio_pcm_notify_open(struct snd_pcm_substream *substream)
{
	int ret = 0;

	UNUSED_PARAMETER(substream);

	return ret;
}

static int audio_pcm_notify_close(struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct hifi_chn_pcm_close msg_body  = {0};

	IN_FUNCTION;

	msg_body.msg_type = (unsigned short)HI_CHN_MSG_PCM_CLOSE;
	msg_body.pcm_mode = (unsigned short)substream->stream;
	msg_body.pcm_device = (unsigned short)substream->pcm->device;
	ret = audio_pcm_mailbox_send_data(&msg_body, sizeof(msg_body), 0);
	if (ret)
		ret = -EBUSY;

	OUT_FUNCTION;

	return ret;
}

static int audio_pcm_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	int ret = 0;
	struct audio_pcm_runtime_data *prtd = substream->runtime->private_data;
	size_t bytes = params_buffer_bytes(params);
	int device = substream->pcm->device;
	struct snd_dma_buffer *dma_buf = &substream->dma_buffer;
	struct audio_pcm_mmap_buf *mmap_buf = NULL;

	if (!_is_valid_pcm_device(device)) {
		return ret;
	}

	if (NULL == prtd) {
		AUDIO_LOGE("prtd is null");
		return -EINVAL;
	}

	if (device == PCM_DEVICE_MMAP) {
		ret = map_hifi_share_buffer(substream);
		if (ret) {
			AUDIO_LOGE("prealloc hifi share buffer fail, ret %d", ret);
			return ret;
		}

		ret = audio_pcm_get_mmap_share_buf(substream, bytes);
		if (ret) {
			AUDIO_LOGE("alloc mmap share buffer size %zd fail, ret %d", bytes, ret);
			unmap_hifi_share_buffer(substream);
			return ret;
		}

		mmap_buf = prtd->mmap_buf;
		dma_buf->dev.type = SNDRV_DMA_TYPE_DEV;
		dma_buf->dev.dev = substream->pcm->card->dev;
		dma_buf->private_data = NULL;
		dma_buf->area = mmap_buf->buf_addr;
		dma_buf->addr = mmap_buf->phy_addr;
		dma_buf->bytes = mmap_buf->buf_size;
		snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
	} else {
		ret = snd_pcm_lib_malloc_pages(substream, bytes);
		if (ret < 0) {
			AUDIO_LOGE("snd_pcm_lib_malloc_pages ret : %d", ret);
			return ret;
		}
	}

	spin_lock(&prtd->lock);
	prtd->period_size = params_period_bytes(params);
	prtd->period_next = 0;
	spin_unlock(&prtd->lock);

	ret = audio_pcm_notify_hw_params(substream, params);
	if (ret < 0) {
		AUDIO_LOGE("pcm mode %d notify hw_params error", substream->stream);
		if (device != PCM_DEVICE_MMAP)
			snd_pcm_lib_free_pages(substream);
		else
			unmap_hifi_share_buffer(substream);
	}

	return ret;
}

static int audio_pcm_hw_free(struct snd_pcm_substream *substream)
{
	int ret = 0;
	int i   = 0;
	struct audio_pcm_runtime_data *prtd = NULL;
	int device = substream->pcm->device;

	prtd = (struct audio_pcm_runtime_data *)substream->runtime->private_data;

	if (!_is_valid_pcm_device(device)) {
		return ret;
	}

	if (NULL == prtd) {
		AUDIO_LOGE("prtd is null");
		return -EINVAL;
	}

	for(i = 0; i < 30 ; i++) {  /* wait for dma ok */
		if (STATUS_STOP == prtd->status) {
			break;
		} else {
			msleep(10);
		}
	}
	if (30 == i) {
		AUDIO_LOGI("timeout for waiting for stop info from other");
	}

	ret = audio_pcm_notify_hw_free(substream);
	if (ret < 0) {
		AUDIO_LOGE("free fail device %d", substream->pcm->device);
	}

	if (device != PCM_DEVICE_MMAP)
		ret = snd_pcm_lib_free_pages(substream);

	return ret;
}

static int audio_pcm_prepare(struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct audio_pcm_runtime_data *prtd = (struct audio_pcm_runtime_data *)substream->runtime->private_data;
	int device = substream->pcm->device;

	if (!_is_valid_pcm_device(device))
		return ret;

	if (NULL == prtd) {
		AUDIO_LOGE("prtd is null");
		return -EINVAL;
	}

	/* init prtd */
	spin_lock(&prtd->lock);
	prtd->status        = STATUS_STOP;
	prtd->period_next   = 0;
	prtd->period_cur    = 0;
	prtd->frame_counter = 0;
	spin_unlock(&prtd->lock);

	ret = audio_pcm_notify_prepare(substream);

	return ret;
}

static int audio_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct audio_pcm_runtime_data *prtd = (struct audio_pcm_runtime_data *)substream->runtime->private_data;
	unsigned int num_periods = runtime->periods;
	int device = substream->pcm->device;
	struct audio_pcm_mmap_buf *mmap_buf = NULL;

	if (!_is_valid_pcm_device(device)) {
		return ret;
	}

	if (NULL == prtd) {
		AUDIO_LOGE("prtd is null");
		return -EINVAL;
	}

	AUDIO_LOGI("device %d mode %d trigger %d ", substream->pcm->device, substream->stream, cmd);
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		ret = audio_pcm_notify_trigger(cmd, substream);
		if (ret < 0) {
			AUDIO_LOGE("trigger %d failed, ret : %d", cmd, ret);
		} else {
			spin_lock(&prtd->lock);
			prtd->status = STATUS_RUNNING;
			prtd->period_next = (prtd->period_next + 1) % num_periods;
			prtd->frame_counter = 0;
			spin_unlock(&prtd->lock);
		}
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		spin_lock(&prtd->lock);
		prtd->status = STATUS_STOPPING;
		spin_unlock(&prtd->lock);

		ret = audio_pcm_notify_trigger(cmd, substream);
		if (ret < 0) {
			AUDIO_LOGE("audio_pcm_notify_pcm_trigger ret : %d", ret);
		}

		if (device == PCM_DEVICE_MMAP) {
			mmap_buf = prtd->mmap_buf;
			if (mmap_buf && mmap_buf->buf_addr)
				memset(mmap_buf->buf_addr, 0, mmap_buf->buf_size);/* unsafe_function_ignore: memset */
		}
		break;
	default:
		AUDIO_LOGE("trigger cmd error : %d", cmd);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static snd_pcm_uframes_t audio_pcm_pointer(struct snd_pcm_substream *substream)
{
	long frame = 0L;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct audio_pcm_runtime_data *prtd = runtime->private_data;
	int device = substream->pcm->device;

	if (!_is_valid_pcm_device(device)) {
		return (snd_pcm_uframes_t)frame;
	}

	if (NULL == prtd) {
		AUDIO_LOGE("prtd is null");
		return -EINVAL;
	}

	frame = bytes_to_frames(runtime, prtd->period_cur * prtd->period_size);
	if (frame >= runtime->buffer_size)
		frame = 0;

	return (snd_pcm_uframes_t)frame;
}

static int audio_pcm_open(struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct audio_pcm_runtime_data *prtd = NULL;
	uint32_t pcm_mode = substream->stream;
	uint32_t pcm_device = substream->pcm->device;
	const struct snd_pcm_hardware *hw_info = NULL;
	struct snd_pcm *pcm = substream->pcm;
	struct snd_soc_pcm_runtime *rtd = pcm->private_data;
	struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AUIDO_PCM_HIFI_COMPONENT_NAME);
	struct audio_pcm_data *pdata = snd_soc_component_get_drvdata(component);
	struct pcm_thread_info *thread_info = &pdata->thread_info;

	AUDIO_LOGI("device %d, mode %d open", substream->pcm->device, substream->stream);

	if (WARN_ON(pcm_device >= PCM_DEVICE_TOTAL || pcm_mode >= PCM_STREAM_MAX))
		return -EINVAL;

	hw_info = audio_pcm_hw_info[pcm_device][pcm_mode];
	if (!hw_info) {
		AUDIO_LOGW("pcm device: %d, pcm_mode:%d hw_info is not found, set default para", pcm_device, pcm_mode);
		return snd_soc_set_runtime_hwparams(substream, &audio_pcm_hardware_modem_playback);
	}

	snd_soc_set_runtime_hwparams(substream, hw_info);

	prtd = kzalloc(sizeof(*prtd), GFP_KERNEL);
	if (prtd == NULL) {
		AUDIO_LOGE("Failed to allocate memory for audio_pcm_runtime_data");
		return -ENOMEM;
	}

	prtd->substream = substream;
	spin_lock_init(&prtd->lock);

	thread_info->runtime_data[pcm_device][pcm_mode] = prtd;

	/* init substream private data */
	spin_lock(&prtd->lock);
	prtd->period_cur  = 0;
	prtd->period_next = 0;
	prtd->period_size = 0;
	prtd->frame_counter = 0;
	prtd->status = STATUS_STOP;
	prtd->using_thread_flag = _is_pcm_device_using_thread(pcm_device);
	/* now only primary playback support low power */
	prtd->is_support_low_power = (pdata->low_power_support &&
		pcm_mode == SNDRV_PCM_STREAM_PLAYBACK && pcm_device == PCM_DEVICE_NORMAL);
	substream->runtime->private_data = prtd;
	spin_unlock(&prtd->lock);

	if (prtd->using_thread_flag)
		atomic_inc(&thread_info->using_thread_cnt);

	ret = audio_pcm_notify_open(substream);
	if (ret) {
		AUDIO_LOGE("notify open fail, ret %d", ret);
		goto fail_open;
	}

	return 0;

fail_open:
	kfree(prtd);
	return ret;
}

static int audio_pcm_close(struct snd_pcm_substream *substream)
{
	int ret = 0;
	int device = substream->pcm->device;
	struct snd_pcm *pcm = substream->pcm;
	struct snd_soc_pcm_runtime *rtd = pcm->private_data;
	struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AUIDO_PCM_HIFI_COMPONENT_NAME);
	struct audio_pcm_data *pdata = snd_soc_component_get_drvdata(component);
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct audio_pcm_runtime_data *prtd = runtime->private_data;
	struct pcm_thread_info *thread_info = NULL;

	if (!_is_valid_pcm_device(device))
		return 0;

	if (prtd == NULL) {
		AUDIO_LOGE("prtd is null");
		return -EINVAL;
	}

	if (pdata == NULL) {
		AUDIO_LOGE("platform data is null");
		return -EINVAL;
	}

	thread_info = &pdata->thread_info;
	if (prtd->using_thread_flag) {
		atomic_dec(&thread_info->using_thread_cnt);
	}

	AUDIO_LOGI("device %d, mode %d close", device, substream->stream);
	ret = audio_pcm_notify_close(substream);
	if (ret)
		AUDIO_LOGE("pcm notify hifi close fail, ret %d", ret);

	if (device == PCM_DEVICE_MMAP) {
		ret = audio_pcm_free_mmap_share_buf(substream);
		if (ret)
			AUDIO_LOGE("free mmap share buf fail");
		unmap_hifi_share_buffer(substream);
	}

	runtime->private_data = NULL;
	kfree(prtd);

	if (substream->stream < PCM_STREAM_MAX) {
		thread_info->runtime_data[device][substream->stream] = NULL;
	}

	return ret;
}

static int audio_pcm_ioctl(struct snd_pcm_substream *substream,
	unsigned int cmd, void *arg)
{
	struct audio_pcm_runtime_data *prtd = substream->runtime->private_data;
	struct audio_pcm_mmap_buf *mmap_buf = NULL;
	int pcm_device = substream->pcm->device;

	switch (cmd) {
	case SNDRV_PCM_IOCTL1_RESET:
		if (pcm_device == PCM_DEVICE_MMAP) {
			mmap_buf = prtd->mmap_buf;
			if (mmap_buf && mmap_buf->buf_addr)
				memset(mmap_buf->buf_addr, 0, mmap_buf->buf_size);/* unsafe_function_ignore: memset */
		}
		break;
	default:
		break;
	}

	return snd_pcm_lib_ioctl(substream, cmd, arg);
}

/* define all pcm ops of audio pcm */
static struct snd_pcm_ops audio_pcm_ops = {
	.open       = audio_pcm_open,
	.close      = audio_pcm_close,
	.ioctl      = audio_pcm_ioctl,
	.hw_params  = audio_pcm_hw_params,
	.hw_free    = audio_pcm_hw_free,
	.prepare    = audio_pcm_prepare,
	.trigger    = audio_pcm_trigger,
	.pointer    = audio_pcm_pointer,
};

static int preallocate_dma_buffer(struct snd_pcm *pcm, int stream)
{
	struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = &substream->dma_buffer;

	if ((pcm->device >= PCM_DEVICE_TOTAL) || (stream >= PCM_STREAM_MAX)) {
		AUDIO_LOGE("Invalid argument: device %d stream %d", pcm->device, stream);
		return -EINVAL;
	}

	if (pcm->device == PCM_DEVICE_MMAP)
		return 0;

	buf->dev.type = SNDRV_DMA_TYPE_DEV;
	buf->dev.dev = pcm->card->dev;
	buf->private_data = NULL;
	buf->addr = g_pcm_dma_buf_config[pcm->device][stream].pcm_dma_buf_base;
	buf->bytes = g_pcm_dma_buf_config[pcm->device][stream].pcm_dma_buf_len;
	buf->area = ioremap(buf->addr, buf->bytes);

	if (!buf->area) {
		AUDIO_LOGE("dma buf area error");
		return -ENOMEM;
	}

	return 0;
}

static void free_dma_buffers(struct snd_pcm *pcm)
{
	struct snd_pcm_substream *substream = NULL;
	struct snd_dma_buffer *buf = NULL;
	int stream;

	IN_FUNCTION;

	if (pcm->device == PCM_DEVICE_MMAP)
		return;

	for (stream = 0; stream < 2; stream++) {
		substream = pcm->streams[stream].substream;
		if (!substream)
			continue;

		buf = &substream->dma_buffer;
		if (!buf->area)
			continue;

		iounmap(buf->area);

		buf->area = NULL;
		buf->addr = 0;
	}
}

static int audio_pcm_new(struct snd_soc_pcm_runtime *rtd)
{
	int ret = 0;
	struct snd_card *card = rtd->card->snd_card;
	struct snd_pcm *pcm = rtd->pcm;

	if (!card->dev->dma_mask) {
		AUDIO_LOGI("dev->dma_mask not set");
		card->dev->dma_mask = &audio_pcm_dmamask;
	}

	if (!card->dev->coherent_dma_mask) {
		AUDIO_LOGI("dev->coherent_dma_mask not set");
		card->dev->coherent_dma_mask = audio_pcm_dmamask;
	}

	AUDIO_LOGI("PLATFORM machine set pcm-device %d", pcm->device);

	if (!_is_valid_pcm_device(pcm->device)) {
		AUDIO_LOGI("just alloc space for pcm device %d", pcm->device);
		return 0;
	}

	/* register callback */
	ret = audio_pcm_notify_isr_register(audio_pcm_notify_recv_proc);
	if (ret) {
		AUDIO_LOGE("notify isr register error : %d", ret);
		return ret;
	}

	/* init lowlatency stream thread */
	if (_is_pcm_device_using_thread(pcm->device)) {
		ret = pcm_thread_start(pcm);
		if (ret) {
			AUDIO_LOGE("pcm thread start fail, ret %d", ret);
			return ret;
		}
	}

	if (pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream) {
		ret = preallocate_dma_buffer(pcm, SNDRV_PCM_STREAM_PLAYBACK);
		if (ret) {
			AUDIO_LOGE("playback preallocate dma buffer fail");
			goto err;
		}
	}

	if (pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream) {
		ret = preallocate_dma_buffer(pcm, SNDRV_PCM_STREAM_CAPTURE);
		if (ret) {
			AUDIO_LOGE("capture preallocate dma buffer fail");
			goto err;
		}
	}

	if (pcm->device == PCM_DEVICE_MMAP) {
		/* add a hwdep node for pcm device */
		ret = audio_pcm_add_hwdep_dev(rtd);
		if (ret) {
			AUDIO_LOGE("add hw dep node for pcm device %d fail, ret %d", pcm->device, ret);
			goto err;
		}
	}

	return 0;

err:
	if (_is_pcm_device_using_thread(pcm->device)) {
		pcm_thread_stop(pcm);
	}

	free_dma_buffers(pcm);

	return ret;
}

static void audio_pcm_free(struct snd_pcm *pcm)
{
	IN_FUNCTION;
	AUDIO_LOGI("audio_pcm_free pcm-device %d", pcm->device);

	if (_is_pcm_device_using_thread(pcm->device))
		pcm_thread_stop(pcm);

	free_dma_buffers(pcm);

	OUT_FUNCTION;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
struct snd_soc_component_driver audio_pcm_platform = {
	.name     =  AUIDO_PCM_HIFI_COMPONENT_NAME,
	.ops      =  &audio_pcm_ops,
	.pcm_new  =  audio_pcm_new,
	.pcm_free =  audio_pcm_free,
};
#else
struct snd_soc_platform_driver audio_pcm_platform = {
	.component_driver.name    =  AUIDO_PCM_HIFI_COMPONENT_NAME,
	.ops      =  &audio_pcm_ops,
	.pcm_new  =  audio_pcm_new,
	.pcm_free =  audio_pcm_free,
};
#endif

/*lint -e429*/
static int  audio_pcm_platform_probe(struct platform_device *pdev)
{
	int ret = -ENODEV;
	struct device *dev = &pdev->dev;
	struct audio_pcm_data *pdata = NULL;
	struct pcm_thread_info *pcm_thread_para = NULL;
	unsigned int dma_buffer_end_addr = PCM_DMA_BUF_END_ADDR;

	IN_FUNCTION;

	if (dma_buffer_end_addr > PCM_OM_BUFF_LOCATION) {
		AUDIO_LOGE("data memory across the border");
		goto probe_failed;
	}

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);

	if (!pdata) {
		AUDIO_LOGE("allocate audio pcm platform data fail");
		goto probe_failed;
	}

	pcm_thread_para = &pdata->thread_info;

	atomic_set(&pcm_thread_para->using_thread_cnt, 0);
	pdata->mmap_shared_fd = -1;

	if (of_property_read_bool(pdev->dev.of_node, "low_power_support")) {
		pdata->low_power_support = true;
	} else {
		pdata->low_power_support = false;
	}

	platform_set_drvdata(pdev, pdata);

	ret = devm_snd_soc_register_component(&pdev->dev, &audio_pcm_component,
			audio_pcm_dai, ARRAY_SIZE(audio_pcm_dai));
	if (ret) {
		AUDIO_LOGE("snd_soc_register_dai return %d", ret);
		goto probe_failed;
	}

	dev_set_name(&pdev->dev, AUIDO_PCM_HIFI);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
	ret = devm_snd_soc_register_component(dev, &audio_pcm_platform, NULL, 0);
#else
	ret = devm_snd_soc_register_platform(dev, &audio_pcm_platform);
#endif
	if (ret) {
		AUDIO_LOGE("snd_soc_register_platform return %d", ret);
		goto probe_failed;
	}
	OUT_FUNCTION;

	return ret;

probe_failed:
	OUT_FUNCTION;
	return ret;
}
/*lint +e429*/

static int audio_pcm_platform_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id audio_pcm_hifi_match_table[] = {
	{.compatible = AUIDO_PCM_HIFI, },
	{ },
};
static struct platform_driver audio_pcm_platform_driver = {
	.driver = {
		.name  = AUIDO_PCM_HIFI,
		.owner = THIS_MODULE,
		.of_match_table = audio_pcm_hifi_match_table,
	},
	.probe  = audio_pcm_platform_probe,
	.remove = audio_pcm_platform_remove,
};

static int __init audio_pcm_init(void)
{
	IN_FUNCTION;
	return platform_driver_register(&audio_pcm_platform_driver);
}
module_init(audio_pcm_init);

static void __exit audio_pcm_exit(void)
{
	platform_driver_unregister(&audio_pcm_platform_driver);
}
module_exit(audio_pcm_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("audio pcm hifi platform driver");

