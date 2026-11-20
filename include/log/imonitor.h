/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: Define for imonitor interface for kernel
 * Author: liwei
 * Create: 2018-11-21
 */

#ifndef IMONITOR_H
#define IMONITOR_H

#include "imonitor_keys.h"

#define IMONITOR_NEW_API
#define IMONITOR_NEW_API_V2

#ifdef __cplusplus
extern "C" {
#endif

struct imonitor_eventobj;

struct imonitor_eventobj *imonitor_create_eventobj(unsigned int event_id);

/* deprecated */
int imonitor_set_param(struct imonitor_eventobj *event_obj, unsigned short param_id, long value);

#ifdef IMONITOR_NEW_API
int imonitor_set_param_integer(struct imonitor_eventobj *event_obj, unsigned short param_id, long value);

int imonitor_set_param_string(struct imonitor_eventobj *event_obj, unsigned short param_id, const char *value);
#endif

int imonitor_unset_param(struct imonitor_eventobj *event_obj, unsigned short param_id);

#ifdef IMONITOR_NEW_API_V2
int imonitor_set_param_integer_v2(struct imonitor_eventobj *event_obj, const char *param, long value);

int imonitor_set_param_string_v2(struct imonitor_eventobj *event_obj, const char *param, const char *value);

int imonitor_unset_param_v2(struct imonitor_eventobj *event_obj, const char *param);
#endif

int imonitor_set_time(struct imonitor_eventobj *event_obj, long long seconds);

int imonitor_add_dynamic_path(struct imonitor_eventobj *event_obj, const char *path);

int imonitor_add_and_del_dynamic_path(struct imonitor_eventobj *event_obj, const char *path);

int imonitor_send_event(struct imonitor_eventobj *event_obj);

void imonitor_destroy_eventobj(struct imonitor_eventobj *event_obj);

#ifdef __cplusplus
}
#endif

#endif
