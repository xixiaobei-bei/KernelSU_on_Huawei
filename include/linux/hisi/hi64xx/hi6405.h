/*
 * hi6405.h -- ALSA SoC hi6405 codec driver
 *
 * Copyright (c) 2015 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __HI6405_H__
#define __HI6405_H__

#define INVALID_REG 0xE000
#define INVALID_REG_VALUE 0xFFFFFFFF

#define HI6405_PB_MIN_CHANNELS (2)
#define HI6405_PB_MAX_CHANNELS (2)
#define HI6405_CP_MIN_CHANNELS (1)
#define HI6405_CP_MAX_CHANNELS (6)
#define HI6405_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | \
			SNDRV_PCM_FMTBIT_S16_BE | \
			SNDRV_PCM_FMTBIT_S24_LE | \
			SNDRV_PCM_FMTBIT_S24_BE)
#define HI6405_RATES SNDRV_PCM_RATE_8000_384000

/* #define HI6405_DEBUG */
#ifdef HI6405_DEBUG
#define IN_FUNCTION \
	pr_info(LOG_TAG"[I]:%s:%d: Begin\n", __FUNCTION__, __LINE__);
#define OUT_FUNCTION \
	pr_info(LOG_TAG"[I]:%s:%d: End.\n", __FUNCTION__, __LINE__);

#define HI6405_LOGD(fmt, ...) \
	pr_info(LOG_TAG"[D]:%s:%d: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define HI6405_LOGE(fmt, ...) \
	pr_err(LOG_TAG"[E]:%s:%d: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#define IN_FUNCTION
#define OUT_FUNCTION
#define HI6405_LOGD(fmt, ...)
#define HI6405_LOGE(fmt, ...) \
	pr_err(LOG_TAG"[E]:%s:%d: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#endif

#define HI6405_LOGI(fmt, ...) \
	pr_info(LOG_TAG"[I]:%s:%d: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);

#define HI6405_LOGW(fmt, ...) \
	pr_warn(LOG_TAG"[W]:%s:%d: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);

#ifdef CONFIG_HISI_DIEID
int hi6405_codec_get_dieid(char *dieid, unsigned int len);
#endif

#endif /* __HI6405_H__ */
