/*
 * hi6555_smpa_path.h -- codec driver
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _HI6555_SMPA_PATH_H_
#define _HI6555_SMPA_PATH_H_

#include <sound/soc.h>
#include "hi6555_reg.h"

int add_smpa(struct snd_soc_codec *codec);

void hisi_pa_asp_soc_reg_init(struct snd_soc_codec *codec);

#endif
