/*
 * power_bigdata.h
 *
 * bigdata for power module
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

#ifndef _POWER_BIGDATA_H_
#define _POWER_BIGDATA_H_

#include <huawei_platform/log/imonitor.h>
#include <huawei_platform/log/imonitor_keys.h>

#define POWER_BIGDATA_RD_BUF_SIZE  64

enum power_bigdata_type {
	POWER_BIGDATA_TYPE_BEGIN = 0,
	POWER_BIGDATA_TYPE_COUL = POWER_BIGDATA_TYPE_BEGIN,
	POWER_BIGDATA_TYPE_END,
};

enum power_bigdata_event_id {
	POWER_BIGDATA_EVENT_ID_COUL = 930001007,
};

typedef int (*power_bigdata_send_cb)(struct imonitor_eventobj *, void *);

struct power_bigdata_test_data {
	unsigned long integer;
	char *string;
};

struct power_bigdata_info {
	const char *name;
	enum power_bigdata_type type;
	unsigned int event_id;
};

#ifdef CONFIG_LOG_EXCEPTION
int power_bigdata_report(enum power_bigdata_type type,
	power_bigdata_send_cb send_cb, void *data);
int power_bigdata_send_integer(struct imonitor_eventobj *obj,
	const char *para, long value);
int power_bigdata_send_string(struct imonitor_eventobj *obj,
	const char *para, const char *value);
#else
static inline int power_bigdata_report(enum power_bigdata_type type,
	power_bigdata_send_cb send_cb, void *data)
{
	return -1;
}

static inline int power_bigdata_send_integer(struct imonitor_eventobj *obj,
	const char *para, long value)
{
	return -1;
}

static inline int power_bigdata_send_string(struct imonitor_eventobj *obj,
	const char *para, const char *value)
{
	return -1;
}
#endif /* CONFIG_LOG_EXCEPTION */

#endif /* _POWER_BIGDATA_H_ */
