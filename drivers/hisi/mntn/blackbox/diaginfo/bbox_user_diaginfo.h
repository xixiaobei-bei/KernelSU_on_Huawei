/*
 * bbox_user_diaginfo.h
 *
 * DMD driver for user-mode application
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
#ifndef __BBOX_USER_DIAGINFO_H
#define __BBOX_USER_DIAGINFO_H

#define MAX_USER_DATA_LEN 97

struct hisi_user_diaginfo {
	unsigned int err_id;
	char err_msg[MAX_USER_DATA_LEN];
};

#endif /* __BBOX_USER_DIAGINFO_H */
