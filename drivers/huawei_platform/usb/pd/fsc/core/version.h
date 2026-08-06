/*
 * version.h
 *
 * version driver
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

#ifdef FSC_DEBUG

#ifndef __FSC_VERSION_H__
#define __FSC_VERSION_H__

#include "platform.h"

/* Program Revision constant.  When updating firmware, change this.  */
#define FSC_TYPEC_CORE_FW_REV_UPPER  3
#define FSC_TYPEC_CORE_FW_REV_MIDDLE  3
#define FSC_TYPEC_CORE_FW_REV_LOWER  3

FSC_U8 core_get_rev_lower(void);
FSC_U8 core_get_rev_middle(void);
FSC_U8 core_get_rev_upper(void);

#endif // __FSC_VERSION_H_

#endif // FSC_DEBUG
