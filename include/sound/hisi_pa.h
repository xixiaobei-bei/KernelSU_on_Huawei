/*
 * hisi_pa.h
 *
 * hisi pa driver
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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

#ifndef __HISI_PA_H__
#define __HISI_PA_H__

#include <sound/soc.h>

enum hisi_pa_mode {
	NORMAL_MODE = 0,
	RESET_MODE,
};

enum hisi_pa_callback_type {
	MODIFY_GAIN = 0,
	RESTORE_GAIN,
	CALLBACK_COUNT,
};

typedef int (*hisi_pa_callback)(void);

int hisi_pa_modify_gain(void);

int hisi_pa_restore_gain(void);

int hisi_pa_cfg_iomux(enum hisi_pa_mode mode);

int hisi_pa_func_register(enum hisi_pa_callback_type func_type,
	hisi_pa_callback func);

int hisi_pa_set_default_func(struct snd_soc_codec *codec);

#endif /* __HISI_PA_H__ */

