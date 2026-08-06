/*
 * au_pa.h
 *
 * au pa driver
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

#ifndef __AU_PA_H__
#define __AU_PA_H__

#include <sound/soc.h>

enum au_pa_mode {
	NORMAL_MODE = 0,
	RESET_MODE,
};

enum au_pa_callback_type {
	MODIFY_GAIN = 0,
	RESTORE_GAIN,
	CALLBACK_COUNT,
};

typedef int (*au_pa_callback)(void);

int au_pa_modify_gain(void);

int au_pa_restore_gain(void);

int au_pa_cfg_iomux(enum au_pa_mode mode);

int au_pa_func_register(enum au_pa_callback_type func_type,
	au_pa_callback func);

int au_pa_set_default_func(struct snd_soc_component *codec);

#endif /* AU_PA_H */

