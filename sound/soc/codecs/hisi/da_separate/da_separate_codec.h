/*
 * da_separate_codec.h -- codec driver
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __DA_SEPARATE_CODEC_H__
#define __DA_SEPARATE_CODEC_H__
#include "da_separate_type.h"

void da_separate_codec_driver_init(const struct codec_ops *ops);

#endif

