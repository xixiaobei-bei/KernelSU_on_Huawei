/*
 * pcm_debug.h
 *
 * pcm debug
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include <sound/soc.h>
#include <linux/version.h>

#ifndef __PCM_DEBUG_H__
#define __PCM_DEBUG_H__

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
struct snd_soc_component_driver audio_pcm_platform;
#else
struct snd_soc_platform_driver audio_pcm_platform;
#endif

#endif /* __PCM_DEBUG_H__ */

