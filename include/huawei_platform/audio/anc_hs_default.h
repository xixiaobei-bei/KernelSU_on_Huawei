/*
 * anc_hs_default.h
 *
 * analog headset default driver
 *
 * Copyright (c) 2014-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef ANC_DEFAULT
#define ANC_DEFAULT

#include "huawei_platform/audio/anc_hs_interface.h"

#ifdef CONFIG_ANC_DEFAULT
void anc_max14744_refresh_headset_type(int headset_type);
#else
inline void anc_max14744_refresh_headset_type(int headset_type)
{
}
#endif

#endif /* ANC_DEFAULT */
