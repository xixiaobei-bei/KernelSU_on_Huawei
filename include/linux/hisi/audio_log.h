/*
 * audio_log.h -- audio log API define
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#ifndef __AUDIO_LOG_H__
#define __AUDIO_LOG_H__

#ifdef AUDIO_PRINT_DEBUG
#define IN_FUNCTION \
	pr_info(LOG_TAG"audio[I]:%s:%d: begin\n", __func__, __LINE__)
#define OUT_FUNCTION \
	pr_info(LOG_TAG"audio[I]:%s:%d: end\n", __func__, __LINE__)
#define AUDIO_LOGD(fmt, ...) \
	pr_info(LOG_TAG"audio[D]:%s:%d: "fmt"\n", __func__, __LINE__, ##__VA_ARGS__)
#else
#define IN_FUNCTION
#define OUT_FUNCTION
#define AUDIO_LOGD(fmt, ...)
#endif

#define AUDIO_LOGI(fmt, ...) \
	pr_info(LOG_TAG"audio[I]:%s:%d: "fmt"\n", __func__, __LINE__, ##__VA_ARGS__)

#define AUDIO_LOGW(fmt, ...) \
	pr_warn(LOG_TAG"audio[W]:%s:%d: "fmt"\n", __func__, __LINE__, ##__VA_ARGS__)

#define AUDIO_LOGE(fmt, ...) \
	pr_err(LOG_TAG"audio[E]:%s:%d: "fmt"\n", __func__, __LINE__, ##__VA_ARGS__)

#endif
