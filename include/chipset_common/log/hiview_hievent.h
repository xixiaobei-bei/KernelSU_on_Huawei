/*
 * hiview_hievent.h
 *
 * interfaces to generate hiview event struct and convert it
 * to regular string which can be analysed by hiview engine
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#ifndef HIVIEW_HIEVENT_H
#define HIVIEW_HIEVENT_H

#ifdef __cplusplus
extern "C" {
#endif

struct hiview_hievent;

struct hiview_hievent *hiview_hievent_create(unsigned int eventid);

int hiview_hievent_report(struct hiview_hievent *event);

void hiview_hievent_destroy(struct hiview_hievent *event);

int hiview_hievent_put_integral(
				struct hiview_hievent *event,
				const char *key,
				long value);

int hiview_hievent_put_string(
				struct hiview_hievent *event,
				const char *key,
				const char *value);

int hiview_hievent_set_time(
				struct hiview_hievent *event,
				long long seconds);

int hiview_hievent_add_file_path(
				struct hiview_hievent *event,
				const char *path);

#ifdef __cplusplus
}
#endif

#endif
