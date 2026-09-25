/*
 * hiview_hievent.c
 *
 * drivers to generate hiview hievent struct and convert it
 * to regular string which can be analysed by hiview engine
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

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/vmalloc.h>

#include <log/hw_log.h>
#include <log/log_exception.h>

#define INT_TYPE_MAX_LEN	21

#define MAX_PATH_NUMBER		10
#define MAX_PATH_LEN		256
#define MAX_STR_LEN		(10 * 1024)

/* 64K is max length of /dev/hwlog_exception */
#define EVENT_INFO_BUF_LEN	(64 * 1024)
#define EVENT_INFO_PACK_BUF_LEN	(2 * 1024)

#define HWLOG_TAG		hiview_hievent
HWLOG_REGIST();

#define BUF_POINTER_FORWARD			\
	do {							\
		if (tmplen < len) {			\
			tmp += tmplen;			\
			len -= tmplen;			\
		} else {					\
			hwlog_err("string over length");	\
			tmp += len;				\
			len = 0;				\
		}							\
	} while (0)

struct hiview_hievent_payload;
struct hiview_hievent_payload {
	char *key;
	char *value;
	struct hiview_hievent_payload *next;
};

/* hievent struct */
struct hiview_hievent {
	unsigned int eventid;

	long long time;

	/* payload linked list */
	struct hiview_hievent_payload *head;

	/* file path needs uploaded */
	char *file_path[MAX_PATH_NUMBER];
};

static int hiview_hievent_convert_string(
				struct hiview_hievent *event,
				char **pbuf);

static struct hiview_hievent_payload *hiview_hievent_payload_create(void);

static void hiview_hievent_payload_destroy(
				struct hiview_hievent_payload *p);

static struct hiview_hievent_payload *hiview_hievent_get_payload(
				struct hiview_hievent_payload *head,
				const char *key);

static void hiview_hievent_add_payload(
				struct hiview_hievent *obj,
			    struct hiview_hievent_payload *payload);

static struct hiview_hievent_payload *hiview_hievent_payload_create(void)
{
	struct hiview_hievent_payload *payload = NULL;

	payload = vmalloc(sizeof(*payload));
	if (!payload)
		return NULL;
	payload->key= NULL;
	payload->value = NULL;
	payload->next = NULL;

	return payload;
}

static void hiview_hievent_payload_destroy(struct hiview_hievent_payload *p)
{
	if (!p)
		return;
	if (p->value)
		vfree(p->value);
	kfree(p->key);
	vfree(p);
}

static struct hiview_hievent_payload *hiview_hievent_get_payload(
					struct hiview_hievent_payload *head,
					const char *key)
{
	struct hiview_hievent_payload *p = head;

	while (p) {
		if (key) {
			if (strcmp(p->key, key) == 0)
				return p;
		}
		p = p->next;
	}

	return NULL;
}

static void hiview_hievent_add_payload(
					struct hiview_hievent *obj,
					struct hiview_hievent_payload *payload)
{
	if (!obj->head) {
		obj->head = payload;
	} else {
		struct hiview_hievent_payload *p = obj->head;

		while (p->next)
			p = p->next;
		p->next = payload;
	}
}

struct hiview_hievent *hiview_hievent_create(unsigned int eventid)
{
	struct hiview_hievent *event = NULL;

	/* combined event obj struct */
	event = vmalloc(sizeof(*event));
	if (!event)
		return NULL;
	memset(event, 0, sizeof(*event));
	event->eventid = eventid;
	hwlog_info("%s : %d\n", __func__, eventid);

	return (void *)event;
}
EXPORT_SYMBOL(hiview_hievent_create);

int hiview_hievent_put_integral(
					struct hiview_hievent *event,
					const char *key,
					long value)
{
	struct hiview_hievent_payload *payload = NULL;

	if ((!event) || (!key)) {
		hwlog_err("Bad input event or key for %s", __func__);
		return -EINVAL;
	}

	payload = hiview_hievent_get_payload(event->head, key);
	if (!payload) {
		payload = hiview_hievent_payload_create();
		if (!payload)
			return -ENOMEM;
		payload->key = kstrdup(key, GFP_ATOMIC);
		hiview_hievent_add_payload(event, payload);
	}
	if (payload->value)
		vfree(payload->value);
	payload->value = vmalloc(INT_TYPE_MAX_LEN);
	if (!payload->value)
		return -ENOMEM;
	memset(payload->value, 0, INT_TYPE_MAX_LEN);
	snprintf(payload->value, INT_TYPE_MAX_LEN, "%d", (int)value);

	return 0;
}
EXPORT_SYMBOL(hiview_hievent_put_integral);

int hiview_hievent_put_string(
					struct hiview_hievent *event,
					const char *key,
					const char *value)
{
	struct hiview_hievent_payload *payload = NULL;
	int len;

	if ((!event) || (!key) || (!value)) {
		hwlog_err("Bad key for %s", __func__);
		return -EINVAL;
	}

	payload = hiview_hievent_get_payload(event->head, key);
	if (!payload) {
		payload = hiview_hievent_payload_create();
		if (!payload)
			return -ENOMEM;
		payload->key = kstrdup(key, GFP_ATOMIC);
		hiview_hievent_add_payload(event, payload);
	}
	if (payload->value)
		vfree(payload->value);
	len = strlen(value);

	/* prevent length larger than MAX_STR_LEN */
	if (len > MAX_STR_LEN)
		len = MAX_STR_LEN;
	payload->value = vmalloc(len + 1);
	if (!payload->value)
		return -ENOMEM;
	memset(payload->value, 0, len + 1);
	strncpy(payload->value, value, len);
	payload->value[len] = '\0';

	return 0;
}
EXPORT_SYMBOL(hiview_hievent_put_string);

int hiview_hievent_set_time(struct hiview_hievent *event, long long seconds)
{
	if ((!event) || (seconds == 0)) {
		hwlog_err("Bad input for %s", __func__);
		return -EINVAL;
	}
	event->time = seconds;
	return 0;
}
EXPORT_SYMBOL(hiview_hievent_set_time);

static int append_array_item(char **pool, int pool_len, const char *path)
{
	int i;

	if ((!path) || (path[0] == 0)) {
		hwlog_err("Bad path %s", __func__);
		return -EINVAL;
	}

	if (strlen(path) > MAX_PATH_LEN) {
		hwlog_err("file path over max: %d", MAX_PATH_LEN);
		return -EINVAL;
	}

	for (i = 0; i < pool_len; i++) {
		if (pool[i] != 0)
			continue;
		pool[i] = kstrdup(path, GFP_ATOMIC);
		break;
	}

	if (i == MAX_PATH_NUMBER) {
		hwlog_err("Too many pathes");
		return -EINVAL;
	}

	return 0;
}

int hiview_hievent_add_file_path(
					struct hiview_hievent *event,
					const char *path)
{
	if (!event) {
		hwlog_err("Bad path %s", __func__);
		return -EINVAL;
	}
	return append_array_item(event->file_path, MAX_PATH_NUMBER, path);
}
EXPORT_SYMBOL(hiview_hievent_add_file_path);

/* make string ":" to "::", ";" to ";;", and remove newline character
 * for example: "abc:def;ghi" transfer to "abc::def;;ghi"
 */
static char *make_regular(char *value)
{
	int count = 0;
	int len = 0;
	char *temp = value;
	char *regular = NULL;
	char *regular_temp = NULL;

	while (*temp != '\0') {
		if (*temp == ':')
			count++;
		else if (*temp == ';')
			count++;
		else if ((*temp == '\n') || (*temp == '\r'))
			*temp = ' ';
		temp++;
		len++;
	}

	/* no need to transfer, just return old value */
	if (count == 0)
		return value;
	regular = vmalloc(len + count * 2 + 1);
	if (!regular)
		return NULL;
	memset(regular, 0, len + count * 2 + 1);
	regular_temp = regular;
	temp = value;
	while (*temp != 0) {
		if ((*temp == ':') || (*temp == ';'))
			*regular_temp++ = *temp;
		*regular_temp++ = *temp;
		temp++;
	}
	*regular_temp = '\0';

	return regular;
}

static int hiview_hievent_convert_string(
					struct hiview_hievent *event,
					char **pbuf)
{
	int len;
	char *tmp = NULL;
	int tmplen;
	unsigned int i;
	unsigned int keycount;
	struct hiview_hievent_payload *p = NULL;

	char *buf = vmalloc(EVENT_INFO_BUF_LEN);
	if (!buf)
		goto ERRORDONE;
	memset(buf, 0, EVENT_INFO_BUF_LEN);
	len = EVENT_INFO_BUF_LEN;
	tmp = buf;

	/* fill eventid */
	tmplen = snprintf(tmp, len, "eventid %d", event->eventid);
	BUF_POINTER_FORWARD;

	/* fill the path */
	for (i = 0; i < MAX_PATH_NUMBER; i++) {
		if (!event->file_path[i])
			break;
		tmplen = snprintf(tmp, len, " -i %s",
				  event->file_path[i]);
		BUF_POINTER_FORWARD;
	}

	/* fill time */
	if (event->time) {
		tmplen = snprintf(tmp, len, " -t %lld",  event->time);
		BUF_POINTER_FORWARD;
	}

	/* fill the payload info */
	keycount = 0;
	p = event->head;
	while (p) {
		char *value = NULL;
		char *regular_value = NULL;
		int need_free = 1;

		if (!p->value) {
			p = p->next;
			continue;
		}
		if (keycount == 0) {
			tmplen = snprintf(tmp, len, " --extra ");
			BUF_POINTER_FORWARD;
		}
		keycount++;

		/* fill key */
		if (p->key)
			tmplen = snprintf(tmp, len, "%s:", p->key);
		BUF_POINTER_FORWARD;
		/* fill value */
		tmplen = 0;

		value = p->value;
		regular_value = make_regular(value);
		if (!regular_value) {
			regular_value = "NULL";
			need_free = 0;
		}
		tmplen = snprintf(tmp, len, "%s;", regular_value);
		if ((value != regular_value) && need_free)
			vfree(regular_value);
		BUF_POINTER_FORWARD;
		p = p->next;
	}
	*pbuf = buf;
	return (EVENT_INFO_BUF_LEN - len);
ERRORDONE:
	if (buf)
		vfree(buf);
	*pbuf = NULL;
	return 0;
}

static int hiview_hievent_write_log_exception(char *str, const int strlen)
{
	char tempchr;
	char *strptr = str;
	int left_buf_len = strlen + 1;
	int sentcnt = 0;

	while (left_buf_len > 0) {
		if (left_buf_len > EVENT_INFO_PACK_BUF_LEN) {
			tempchr = strptr[EVENT_INFO_PACK_BUF_LEN - 1];
			strptr[EVENT_INFO_PACK_BUF_LEN - 1] = '\0';
			logbuf_to_exception(0, 0, IDAP_LOGTYPE_CMD, 1,
					    strptr, EVENT_INFO_PACK_BUF_LEN);
			left_buf_len -= (EVENT_INFO_PACK_BUF_LEN - 1);
			strptr += (EVENT_INFO_PACK_BUF_LEN - 1);
			strptr[0] = tempchr;
			sentcnt++;
		} else {
			logbuf_to_exception(0, 0, IDAP_LOGTYPE_CMD, 0,
					    strptr, left_buf_len);
			sentcnt++;
			break;
		}
	}

	return sentcnt;
}

static void hiview_hievent_file_lock(struct file *filp, int cmd)
{
	struct file_lock *fl = NULL;

	fl = locks_alloc_lock();
	if (!fl) {
		hwlog_err("%s alloc error", __func__);
		return;
	}
	fl->fl_file = filp;
	fl->fl_owner = filp;
	fl->fl_pid = 0;
	fl->fl_flags = FL_FLOCK;
	fl->fl_type = cmd;
	fl->fl_end = OFFSET_MAX;

#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 4, 0))
	locks_lock_file_wait(filp, fl);
#else
	flock_lock_file_wait(filp, fl);
#endif
	locks_free_lock(fl);
}

int hiview_hievent_report(struct hiview_hievent *obj)
{
	char *str = NULL;
	int buflen;
	int sent_packet;
	struct file *fp = NULL;

	if (!obj) {
		hwlog_err("Bad event %s", __func__);
		return -EINVAL;
	}

	buflen = hiview_hievent_convert_string(obj, &str);
	if (!str)
		return -EINVAL;
	fp = filp_open("/dev/hwlog_exception", O_WRONLY, 0);

	if (IS_ERR(fp)) {
		hwlog_info("%s open fail", __func__);
		sent_packet = hiview_hievent_write_log_exception(str, buflen);
	} else {
		hiview_hievent_file_lock(fp, F_WRLCK);
		sent_packet = hiview_hievent_write_log_exception(str, buflen);
		hiview_hievent_file_lock(fp, F_UNLCK);
		filp_close(fp, 0);
	}

	hwlog_info("report: %s", str);
	vfree(str);

	return sent_packet;
}
EXPORT_SYMBOL(hiview_hievent_report);

void hiview_hievent_destroy(struct hiview_hievent *event)
{
	int i;
	struct hiview_hievent_payload *p = NULL;

	if (!event)
		return;
	p = event->head;
	while (p) {
		struct hiview_hievent_payload *del = p;

		p = p->next;
		hiview_hievent_payload_destroy(del);
	}
	event->head = NULL;
	for (i = 0; i < MAX_PATH_NUMBER; i++) {
		kfree(event->file_path[i]);
		event->file_path[i] = NULL;
	}
	vfree(event);
}
EXPORT_SYMBOL(hiview_hievent_destroy);
