/*
 * zrhung_wp_sochalt.h
 *
 * This file is the header file for zero hung_wp sochalt
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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


#ifndef __ZRHUNG_WP_SOCHALT_H_
#define __ZRHUNG_WP_SOCHALT_H_

#include <chipset_common/hwzrhung/zrhung.h>

#ifdef __cplusplus
extern "C" {
#endif

int wp_get_sochalt(struct zrhung_write_event *we);
void get_sr_position_from_fastboot(char *dst, unsigned int max_dst_size);
void zrhung_get_longpress_event(void);
void zrhung_report_endrecovery(void);

#ifdef __cplusplus
}
#endif
#endif /* __ZRHUNG_WP_SOCHALT_H_ */

