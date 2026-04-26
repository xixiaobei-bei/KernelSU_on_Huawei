/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: Define for hievent interface for kernel
 * Author: xiaocong
 * Create: 2019-10-17
 */

#ifndef HIVIEW_HIEVENT_H
#define HIVIEW_HIEVENT_H

#ifdef __cplusplus
extern "C" {
#endif

#define RAW_FORMAT_STRING_INDEX 2
#define RAW_FIRST_PARAM_INDEX 3

struct hiview_hievent;

struct hiview_hievent *hiview_hievent_create(unsigned int event_id);

int hiview_hievent_report(struct hiview_hievent *event);

void hiview_hievent_destroy(struct hiview_hievent *event);

int hiview_hievent_put_integral(struct hiview_hievent *event, const char *key, long long value);

int hiview_hievent_put_string(struct hiview_hievent *event, const char *key, const char *value);

int hiview_hievent_set_time(struct hiview_hievent *event, long long seconds);

int hiview_hievent_add_file_path(struct hiview_hievent *event, const char *path);

struct hiview_hievent *hiview_build_raw_event(int event_id, const char *fmt, ...)
				__attribute__((__format__(__printf__, RAW_FORMAT_STRING_INDEX, RAW_FIRST_PARAM_INDEX)));

int hiview_send_raw_event(int event_id, const char *fmt, ...)
				__attribute__((__format__(__printf__, RAW_FORMAT_STRING_INDEX, RAW_FIRST_PARAM_INDEX)));

#ifdef __cplusplus
}
#endif

#endif
