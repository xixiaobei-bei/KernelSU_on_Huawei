/*
 * da_separate_fpga.h is used to work by da_separate_fpga
 * * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __DA_SEPARATE_FPGA_H__
#define __DA_SEPARATE_FPGA_H__

#include <sound/soc.h>

void sif0_io_init(struct snd_soc_component *codec);
bool is_pmu_version_valid(struct snd_soc_component *codec);
#endif
