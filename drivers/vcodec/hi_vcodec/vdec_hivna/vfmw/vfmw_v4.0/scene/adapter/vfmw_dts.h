/*
 * vfmw_dts.h
 *
 * This is vfmw handle dts config interface.
 *
 * Copyright (c) 2017-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __VFMW_DTS_H__
#define __VFMW_DTS_H__
#include "vfmw.h"

SINT32 vfmw_set_dts_config(vfmw_dts_config_s *dts_config);
SINT32 vfmw_get_dts_config(vfmw_dts_config_s *dts_config);

#endif
