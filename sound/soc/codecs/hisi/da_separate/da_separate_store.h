/*
 * da_separate_store.h -- codec driver
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __DA_SEPARATE_STORE_H__
#define __DA_SEPARATE_STORE_H__

#include <sound/soc.h>

int da_separate_store_subscribe_path(struct snd_soc_component *codec, const char **name_list, unsigned int num);
#endif

