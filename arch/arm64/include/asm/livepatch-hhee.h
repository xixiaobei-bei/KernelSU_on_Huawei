/*
 * livepatch-hhee.h - hhee interface for arm64-specific Kernel Live Patching
 *
 * Copyright (c) 2014-2018, Huawei Tech. Co., Ltd. All rights reserved.
 * Author: Li Bin <huawei.libin@huawei.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __ASM_LIVEPATCH_HHEE_H
#define __ASM_LIVEPATCH_HHEE_H

#include <linux/arm-smccc.h>
#include <asm/cacheflush.h>
#include <linux/of.h>
#include <asm/insn.h>
#ifdef CONFIG_HISI_HHEE
#include <linux/hisi/hisi_hhee.h>
#endif

#ifdef CONFIG_LIVEPATCH
extern unsigned long hkip_token;
#else
static unsigned long hkip_token;
#endif

enum aarch64_reloc_stage {
	RELOC_MODULE_LOADING,
	RELOC_MODULE_LOADED,
};

static inline bool is_hkip_enabled(void)
{
	bool ret = false;
#ifdef CONFIG_HISI_HHEE_TOKEN
	if (hhee_check_enable() == HHEE_ENABLE)
		ret = true;
#endif
	return ret;
}
#endif

