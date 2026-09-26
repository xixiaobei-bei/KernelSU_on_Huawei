/*
 * hwevext_codec_interface.h
 *
 * hwevext_codec_interface header file
 *
 * Copyright (c) 2021 Huawei Technologies Co., Ltd.
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

#ifndef __HWEVEXT_CODEC_INTERFACE_H__
#define __HWEVEXT_CODEC_INTERFACE_H__

#include <sound/soc.h>

#ifdef CONFIG_HWEVEXT_CODEC_INTERFACE
void hwevext_codec_register_in_machine_driver(struct platform_device *pdev,
	struct snd_soc_card **card,
	struct snd_soc_dai_link *dai_link, unsigned int links_num);

#else
static inline void hwevext_codec_register_in_machine_driver(
	struct platform_device *pdev,
	struct snd_soc_card **card,
	struct snd_soc_dai_link *dai_link, unsigned int links_num)
{
}
#endif

#endif /* hwevext_codec_interface.h */

