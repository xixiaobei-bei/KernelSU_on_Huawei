/*
 * copyright: Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * file: bfm_hisi_dmd_info_priv.h
 *
 * define the dmd private modules' external
 *          public enum/macros/interface for BFM (Boot Fail Monitor)
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

#ifndef BFM_HISI_DMD_INFO_H
#define BFM_HISI_DMD_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief: get the most err number from DMD file about boot fail.
 *
 * @param: err_num     the dmd err number
 * @param: count       the times of err occur
 * @param: err_name    the mod name of this err
 *
 * @return: 0 - find adapt err number; -1 - not find.
 */
int get_dmd_err_num(unsigned int *err_num, unsigned int *count, char *err_name);

#ifdef __cplusplus
}
#endif

#endif

