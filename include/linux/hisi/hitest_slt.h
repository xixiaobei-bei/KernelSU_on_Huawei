/*
 * hitest_slt.h
 *
 * running kernel at slt.
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
#ifndef __HITEST_SLT_H__
#define __HITEST_SLT_H__

#ifdef CONFIG_HISILICON_PLATFORM_HITEST
extern int is_running_kernel_slt(void);
#else
static inline int is_running_kernel_slt(void)
{
	return 0;
}
#endif
#endif /* __HITEST_SLT_H__ */
