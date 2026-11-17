/*
 * copyright: Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * file: bfr_chipsets.h
 *
 * define the chipsets' external public enum/macros/interface for BFR
 * (Boot Fail Recovery)
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

#ifndef BFR_CHIPSETS_H
#define BFR_CHIPSETS_H

#include <linux/types.h>
#include "chipset_common/bfmr/public/bfmr_public.h"

#ifdef __cplusplus
extern "C" {
#endif

bool bfr_bopd_has_been_enabled(void);
bool bfr_safe_mode_has_been_enabled(void);
int bfr_get_full_path_of_rrecord_part(char **path_buf);

#ifdef __cplusplus
}
#endif

#endif /* BFR_CHIPSETS_H */


