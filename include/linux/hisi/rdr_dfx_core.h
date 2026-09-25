/*
 * rdr_dfx_core.h
 *
 * blackbox. (kernel run data recorder.).
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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
#ifndef __RDR_DFX_CORE_H__
#define __RDR_DFX_CORE_H__

enum dfx_module_type {
	DFX_NOREBOOT = 0,
	DFX_ZEROHUNG,
	DFX_MAX_MODULE
};

#ifdef CONFIG_HISI_DFX_CORE
int dfx_read(u32 module, void *buffer, u32 size);
int dfx_write(u32 module, const void *buffer, u32 size);
#else
static inline int dfx_read(u32 module, void *buffer, u32 size) { return 0; }
static inline int dfx_write(u32 module, void *buffer, u32 size) { return 0; }
#endif

#endif
