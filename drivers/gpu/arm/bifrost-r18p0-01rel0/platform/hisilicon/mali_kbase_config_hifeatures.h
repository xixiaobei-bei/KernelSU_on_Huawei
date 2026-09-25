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
	KBASE_FEATURE_HI0001,
	KBASE_FEATURE_HI0002,
	KBASE_FEATURE_HI0003,
	KBASE_FEATURE_HI0004,
	KBASE_FEATURE_HI0005,
	KBASE_FEATURE_HI0006,
	KBASE_FEATURE_HI0007,
	KBASE_FEATURE_HI0008,
	/* for avs benchmark collect, When an exception occurs , do BUG_ON */
	KBASE_FEATURE_HI0009,
	KBASE_FEATURE_HI0010,
	KBASE_FEATURE_HI0011,
	/* for tSIx mem auto shutdown */
	KBASE_FEATURE_HI0012,
	/* use always on as power policy in FPGA */
	KBASE_FEATURE_HI0013,
	/* norr es gpu deep sleep and auto shutdown */
	KBASE_FEATURE_HI0014,
	/* norr cs,trym,gondul gpu deep sleep and auto shutdown */
	KBASE_FEATURE_HI0015,
	/* open bug on for gpu steadiness */
	KBASE_FEATURE_HI0016,
	/* trym es set gpu striping granule to hash function with 256 byte */
	KBASE_FEATURE_HI0017,
	/* enable after TR4A */
	KBASE_FEATURE_HI0018,
	KBASE_HI_FEATURE_END
};

static const enum kbase_hi_feature kbase_hi_feature_t880_r0p2[] = {
	KBASE_FEATURE_HI0002,
	KBASE_FEATURE_HI0004,
	KBASE_FEATURE_HI0008,
	KBASE_HI_FEATURE_END
};

static const enum kbase_hi_feature kbase_hi_feature_t830_r2p0[] = {
	KBASE_FEATURE_HI0004,
	KBASE_FEATURE_HI0007,
	KBASE_FEATURE_HI0008,
	KBASE_HI_FEATURE_END
};

static const enum kbase_hi_feature kbase_hi_feature_t880_r2p0[] = {
	KBASE_FEATURE_HI0002,
	KBASE_FEATURE_HI0003,
	KBASE_FEATURE_HI0004,
	KBASE_FEATURE_HI0005,
	KBASE_FEATURE_HI0006,
	KBASE_FEATURE_HI0008,
	KBASE_HI_FEATURE_END
};
// MIx
static const enum kbase_hi_feature kbase_hi_feature_tmix_r0p0[] = {
	KBASE_FEATURE_HI0004,
	KBASE_FEATURE_HI0006,
	KBASE_FEATURE_HI0008,
#ifdef CONFIG_HISI_ENABLE_HPM_DATA_COLLECT
	KBASE_FEATURE_HI0009,
#endif
	KBASE_HI_FEATURE_END
};
// HEx
static const enum kbase_hi_feature kbase_hi_feature_thex_r0p0[] = {
	KBASE_FEATURE_HI0007,
	KBASE_FEATURE_HI0008,
	KBASE_FEATURE_HI0010,
#ifdef CONFIG_MALI_NORR
	KBASE_FEATURE_HI0014,
#endif
	KBASE_FEATURE_HI0016,
	KBASE_HI_FEATURE_END
};
// NOx
static const enum kbase_hi_feature kbase_hi_feature_tnox_r0p0[] = {
	KBASE_FEATURE_HI0007,
	KBASE_FEATURE_HI0008,
#ifdef CONFIG_HISI_ENABLE_HPM_DATA_COLLECT
	KBASE_FEATURE_HI0009,
#endif
	KBASE_FEATURE_HI0015,
	KBASE_FEATURE_HI0016,
#ifdef CONFIG_MALI_NORR_PHX
	KBASE_FEATURE_HI0017,
#endif
	KBASE_FEATURE_HI0018,
	KBASE_HI_FEATURE_END
};
// GOx
static const enum kbase_hi_feature kbase_hi_feature_tgox_r1p0[] = {
	KBASE_FEATURE_HI0007,
	KBASE_FEATURE_HI0008,
#ifdef CONFIG_HISI_ENABLE_HPM_DATA_COLLECT
	KBASE_FEATURE_HI0009,
#endif
	KBASE_FEATURE_HI0015,
	KBASE_FEATURE_HI0016,
	KBASE_HI_FEATURE_END
};
// SIx
static const enum kbase_hi_feature kbase_hi_feature_tsix_r1p1[] = {
	KBASE_FEATURE_HI0002,
	KBASE_FEATURE_HI0005,
	KBASE_FEATURE_HI0007,
	KBASE_FEATURE_HI0008,
#ifdef CONFIG_HISI_ENABLE_HPM_DATA_COLLECT
	KBASE_FEATURE_HI0009,
#endif
	KBASE_FEATURE_HI0010,
	KBASE_FEATURE_HI0012,
	KBASE_HI_FEATURE_END
};
#endif /* BASE_HWCONFIG_ISSUES_H */
