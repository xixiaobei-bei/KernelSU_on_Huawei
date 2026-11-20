/*
 * hisi_mntn_bc_panic.h
 *
 * head of record data for panic.
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
#ifndef __MNTN_BC_PANIC_H
#define __MNTN_BC_PANIC_H

#ifdef CONFIG_HISI_RECOVERY_BIGCORE_PANIC
extern void record_bc_panic(void);
#else
void record_bc_panic(void)
{
	return;
}
#endif
#endif
