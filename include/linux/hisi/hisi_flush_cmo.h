/*
 * hisi_flush_cmo.h
 *
 * header file of hisi_flush_cmo module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef __HISI_FLUSH_CMO_H__
#define __HISI_FLUSH_CMO_H__

#ifdef CONFIG_NAMSTO_FLUSH_CMO
/* must not use it when dcache is disabled(c bit of SCTLR_EL1) */
void flush_cluster_cmo(void);
#else
static inline void flush_cluster_cmo(void) {}
#endif

#if defined(CONFIG_NAMSTO_DEBUG) && defined(CONFIG_NAMSTO_FLUSH_CMO)
void __flush_cluster_cmo(void);
#else
static inline void __flush_cluster_cmo(void) {}
#endif

#endif /* __HISI_FLUSH_CMO_H__ */
