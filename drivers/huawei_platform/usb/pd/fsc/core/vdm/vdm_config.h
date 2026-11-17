/*
 * vdm_config.h
 *
 * vdm_config driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifdef FSC_HAVE_VDM

#ifndef __FSC_VDM_CONFIG_H__
#define __FSC_VDM_CONFIG_H__
#include "../platform.h"

/* General VDM Things */
extern FSC_BOOL svid_enable;
extern FSC_BOOL mode_enable;
extern FSC_U16 my_svid;
extern FSC_U32 my_mode;
extern FSC_BOOL mode_entered;

/* Functions to configure VDM code! */
void ConfigureVdmResponses(FSC_U8* bytes);
void ReadVdmConfiguration(FSC_U8* data);

/* Functions specifically to configure DisplayPort code! */
#ifdef FSC_HAVE_DP
void configDp (FSC_BOOL enabled, FSC_U32 status);
void configAutoDpModeEntry (FSC_BOOL enabled, FSC_U32 mask, FSC_U32 value);
void WriteDpControls(FSC_U8* data);
void ReadDpControls(FSC_U8* data);
void ReadDpStatus(FSC_U8* data);
#endif // FSC_HAVE_DP

#endif // __FSC_VDM_CONFIG_H__

#endif // FSC_HAVE_VDM
