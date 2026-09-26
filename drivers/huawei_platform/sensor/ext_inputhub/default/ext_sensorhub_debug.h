/*
 * ext_sensorhub_debug.h
 *
 * head file for debug external sensorhub
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

#ifndef EXT_SENSORHUB_DEBUG_H
#define EXT_SENSORHUB_DEBUG_H

#define MAX_CMD_BUF_ARGC 64

static const int tag_sendcmd = 1;
static const int tag_subcmd = 2;
static const int tag_mock_data = 3;

struct write_cmd_tag_map {
	const char *str;
	int tag;
};

#define ch_is_digit(ch) ('0' <= (ch) && (ch) <= '9')
#define ch_is_hex(ch) \
	(('A' <= (ch) && (ch) <= 'F') || ('a' <= (ch) && (ch) <= 'f'))
#define ch_is_hex_digit(ch) (ch_is_digit(ch) || ch_is_hex(ch))

#endif /* EXT_SENSORHUB_DEBUG_H */
