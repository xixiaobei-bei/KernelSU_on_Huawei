/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2019. All rights reserved.
 * Description: This file describe HISI GPU hardware related features
 * Author: Huawei Technologies Co., Ltd.
 * Create: 2014-2-24
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * A copy of the licence is included with the program, and can also be obtained
 * from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

/* AUTOMATICALLY GENERATED FILE. If you want to amend the issues/features,
 * please update base/tools/hwconfig_generator/hwc_{issues,features}.py
 * For more information see base/tools/hwconfig_generator/README
 */

#ifndef KBASE_CONFIG_HI_FEATURES_H
#define KBASE_CONFIG_HI_FEATURES_H

enum kbase_hi_feature {
	/* use external reset instead of internal gpu hardware reset */
	KBASE_FEATURE_EXTERNAL_RESET,

	/* mtcos be implemented, shader core poweron ready period */
	KBASE_FEATURE_CORE_READY_PERIOD,

	/* close buck bwtween each frame */
	KBASE_FEATURE_BUCKOFF_PER_FRAME,

	/* gpu auto clock gating feature, triger by L2 power off */
	KBASE_FEATURE_AUTO_CLOCK_DIV,

	/* gpu mem auto shutdown feature */
	KBASE_FEATURE_AUTO_MEM_SHUTDOWN,

	/* enable striping granul setting */
	KBASE_FEATURE_STRIPING_GRANUL_SETTING,

	/* filtering some mmu fault caused by application */
	KBASE_FEATURE_MMU_FAULT_FILTERING,

	/* enable always on power policy */
	KBASE_FEATURE_USE_ALWAYS_ON,

	/* enable ARM workaroud patch */
	KBASE_FEATURE_ENABLE_ARM_WA_JOB,

	KBASE_FEATURE_MAX_COUNT
};

/* define features be used by tHex */
static const enum kbase_hi_feature kbase_hi_feature_thex_r0p0[] = {
	KBASE_FEATURE_BUCKOFF_PER_FRAME,
	KBASE_FEATURE_AUTO_MEM_SHUTDOWN,
	KBASE_FEATURE_MAX_COUNT
};

/* define features used by tSix */
static const enum kbase_hi_feature kbase_hi_feature_tsix_r1p1[] = {
	KBASE_FEATURE_BUCKOFF_PER_FRAME,
	KBASE_FEATURE_AUTO_CLOCK_DIV,
	KBASE_FEATURE_AUTO_MEM_SHUTDOWN,
	KBASE_FEATURE_MAX_COUNT
};

/* define features be uesed by tNox */
static const enum kbase_hi_feature kbase_hi_feature_tnox_r0p0[] = {
	KBASE_FEATURE_BUCKOFF_PER_FRAME,
	KBASE_FEATURE_STRIPING_GRANUL_SETTING,
#ifdef CONFIG_MALI_NORR_PHX
	KBASE_FEATURE_MMU_FAULT_FILTERING,
#endif
	KBASE_FEATURE_MAX_COUNT
};

/* define features used by tGox */
static const enum kbase_hi_feature kbase_hi_feature_tgox_r1p0[] = {
	KBASE_FEATURE_BUCKOFF_PER_FRAME,
	KBASE_FEATURE_STRIPING_GRANUL_SETTING,
	KBASE_FEATURE_MAX_COUNT
};

/* define features uesed by tTrex */
static const enum kbase_hi_feature kbase_hi_feature_ttrx_r0p1[] = {
	KBASE_FEATURE_BUCKOFF_PER_FRAME,
	KBASE_FEATURE_AUTO_MEM_SHUTDOWN,
	KBASE_FEATURE_STRIPING_GRANUL_SETTING,
	KBASE_FEATURE_ENABLE_ARM_WA_JOB,
	KBASE_FEATURE_MAX_COUNT
};

/* define features used by tBex */
static const enum kbase_hi_feature kbase_hi_feature_tbex_r0p0[] = {
	KBASE_FEATURE_AUTO_MEM_SHUTDOWN,
	KBASE_FEATURE_STRIPING_GRANUL_SETTING,
	KBASE_FEATURE_ENABLE_ARM_WA_JOB,
	KBASE_FEATURE_MAX_COUNT
};

/* define features used by tBex */
static const enum kbase_hi_feature kbase_hi_feature_tbex_r0p1[] = {
	KBASE_FEATURE_AUTO_MEM_SHUTDOWN,
	KBASE_FEATURE_STRIPING_GRANUL_SETTING,
	KBASE_FEATURE_MAX_COUNT
};

/* define features uesed by tNax */
static const enum kbase_hi_feature kbase_hi_feature_tnax_r0p1[] = {
	KBASE_FEATURE_BUCKOFF_PER_FRAME,
	KBASE_FEATURE_STRIPING_GRANUL_SETTING,
	KBASE_FEATURE_MAX_COUNT
};

#endif /* KBASE_CONFIG_HI_FEATURES_H */
