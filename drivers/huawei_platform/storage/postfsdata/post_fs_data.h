/*
 * post_fs_data.h
 *
 * post_fs_data header
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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

#ifndef POST_FS_DATA_H
#define POST_FS_DATA_H

int wait_for_postfsdata(unsigned int timeout);
unsigned int get_postfsdata_flag(void);
void set_postfsdata_flag(int value);

#endif
