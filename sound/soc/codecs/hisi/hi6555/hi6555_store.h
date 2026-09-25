/*
 * hi6555_store.h -- codec driver
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _HI6555_STORE_H_
#define _HI6555_STORE_H_

#include <sound/soc.h>

int hi6555_store_subscribe_path(struct snd_soc_codec *codec, const char **name_list, unsigned int num);
#endif

