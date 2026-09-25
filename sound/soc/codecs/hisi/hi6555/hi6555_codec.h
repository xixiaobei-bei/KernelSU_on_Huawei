/*
 * hi6555_codec.h -- codec driver
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _HI6555_CODEC_H_
#define _HI6555_CODEC_H_
#include "hi6555_type.h"

void hi6555_codec_driver_init(const struct codec_ops *ops);

#endif

