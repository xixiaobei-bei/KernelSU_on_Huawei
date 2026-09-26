/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: drivers to generate imonitor event struct
 * Author: wangtanyun
 * Create: 2017-10-09
 */

#include <huawei_platform/log/imonitor_events.h>

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <securec.h>

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/log/imonitor.h>
#include <huawei_platform/log/log_exception.h>

#define BIT_TYPE      1
#define TINYINT_TYPE  2
#define SMALLINT_TYPE 3
#define INT_TYPE      4
#define FLOAT_TYPE    5
#define DATETIME_TYPE 6
#define CLASS_TYPE    7
#define BASE_TYPE_MAX CLASS_TYPE
#define VARCHAR_TYPE  100

#define BIT_TYPE_LEN      1
#define TINYINT_TYPE_LEN  1
#define SMALLINT_TYPE_LEN 2
#define INT_TYPE_LEN      4
#define FLOAT_TYPE_LEN    4
#define DATETIME_TYPE_LEN 20
#define INT_TYPE_MAX_LEN  21

#define MASK_OFFSET_GET_OFFSET(m) ((m) & (0x7FFFFFFF))
#define MASK_OFFSET_SET_MASK(m)   ((m) | (1 << 31))
#define MASK_OFFSET_GET_MASK(m)   ((m) >> 31)

#define MAX_PATH_NUMBER 10
#define MAX_PATH_LEN    256
#define MAX_STR_LEN     (10 * 1024)

/* 64K is max length of /dev/hwlog_exception */
#define EVENT_INFO_BUF_LEN      (64 * 1024)
#define EVENT_INFO_PACK_BUF_LEN (2 * 1024)

#define INVALID_KEY (-1)
#define INVALID_VERSION (-1)

#define HWLOG_TAG imonitor
HWLOG_REGIST();

#define BUF_POINTER_FORWARD                              \
	do {                                             \
		if (tmp_len >= 0 && tmp_len < len) {     \
			tmp += tmp_len;                  \
			len -= tmp_len;                  \
		} else {                                 \
			hwlog_err("string over length"); \
			tmp += len;                      \
			len = 0;                         \
		}                                        \
	} while (0)

struct imonitor_param;
struct imonitor_param {
	int key;
	char *key_v2;
	char *value;
	struct imonitor_param *next;
};

/* event obj struct */
struct imonitor_eventobj {
	unsigned int event_id;
	const unsigned short *desc;
	unsigned int params_count;
	char api_version;

	/* record params linked list */
	struct imonitor_param *head;
	/* use to -t from set time function */
	long long time;
	/* dynamic file path should be packed and uploaded */
	char *dynamic_path[MAX_PATH_NUMBER];
	/* dynamic file path should be packed and uploaded, deleted after packing and uploading */
	char *dynamic_path_delete[MAX_PATH_NUMBER];
};

#define check_v1_api(obj)                                                      \
do {                                                                           \
	if ((obj)->api_version == 0) {                                         \
		(obj)->api_version = 1;                                        \
	} else if ((obj)->api_version == 2) { /* 2 means api version */        \
		hwlog_err("cannot use v1 api %s after v2 api used", __func__); \
		return INVALID_VERSION;                                        \
	}                                                                      \
} while (0)

#define check_v2_api(obj)                                                      \
do {                                                                           \
	if ((obj)->api_version == 0) {                                         \
		(obj)->api_version = 2; /* 2 means api version */              \
	} else if ((obj)->api_version == 1) {                                  \
		hwlog_err("cannot use v2 api %s after v1 api used", __func__); \
		return INVALID_VERSION;                                        \
	}                                                                      \
} while (0)

#ifndef IMONITOR_NEW_API
int imonitor_set_param_integer(struct imonitor_eventobj *event_obj, unsigned short param_id, long value);
int imonitor_set_param_string(struct imonitor_eventobj *event_obj, unsigned short param_id, const char *value);
#endif

int imonitor_unset_param(struct imonitor_eventobj *event_obj, unsigned short param_id);

#ifndef IMONITOR_NEW_API_V2
int imonitor_set_param_integer_v2(struct imonitor_eventobj *event_obj, const char *param, long value);
int imonitor_set_param_string_v2(struct imonitor_eventobj *event_obj, const char *param, const char *value);
int imonitor_unset_param_v2(struct imonitor_eventobj *event_obj, const char *param);
#endif

static int imonitor_convert_string(struct imonitor_eventobj *event_obj, char **buf_ptr);
static struct imonitor_param *create_imonitor_param(void);
static void destroy_imonitor_param(struct imonitor_param *param);
static struct imonitor_param *get_imonitor_param(struct imonitor_param *head, int key, const char *key_s);
static void del_imonitor_param(struct imonitor_eventobj *event_obj, int key, const char *key_s);
static void add_imonitor_param(struct imonitor_eventobj *event_obj, struct imonitor_param *param);

static struct imonitor_param *create_imonitor_param(void)
{
	struct imonitor_param *param = NULL;

	param = vmalloc(sizeof(*param));
	if (!param)
		return NULL;
	param->key = INVALID_KEY;
	param->key_v2 = NULL;
	param->value = NULL;
	param->next = NULL;

	return param;
}

static void destroy_imonitor_param(struct imonitor_param *param)
{
	if (!param)
		return;
	if (param->value)
		vfree(param->value);
	kfree(param->key_v2);
	vfree(param);
}

static struct imonitor_param *get_imonitor_param(struct imonitor_param *head, int key, const char *key_s)
{
	struct imonitor_param *param = head;

	while (param) {
		if (key_s && param->key_v2) {
			if (strcmp(param->key_v2, key_s) == 0)
				return param;
		} else if (param->key == key) {
			return param;
		}
		param = param->next;
	}

	return NULL;
}

static void del_imonitor_param(struct imonitor_eventobj *obj, int key, const char *key_s)
{
	struct imonitor_param *prev = NULL;
	struct imonitor_param *param = obj->head;

	while (param) {
		int is_found = 0;

		if (key_s && param->key_v2) {
			if (strcmp(param->key_v2, key_s) == 0)
				is_found = 1;
		} else if (param->key == key) {
			is_found = 1;
		}
		if (is_found) {
			if (!prev)
				obj->head = param->next;
			else
				prev->next = param->next;
			destroy_imonitor_param(param);
			break;
		}
		prev = param;
		param = param->next;
	}
}

static void add_imonitor_param(struct imonitor_eventobj *event_obj, struct imonitor_param *param)
{
	if (!event_obj->head) {
		event_obj->head = param;
	} else {
		struct imonitor_param *temp = event_obj->head;

		while (temp->next)
			temp = temp->next;
		temp->next = param;
	}
}

struct imonitor_eventobj *imonitor_create_eventobj(unsigned int event_id)
{
	/* Look for module */
	unsigned int i;
	const struct imonitor_module_index *module_index = NULL;
	const struct imonitor_event_index *event_index = NULL;
	struct imonitor_eventobj *event_obj = NULL;

	for (i = 0; i < imonitor_modules_count; i++) {
		if ((event_id >= imonitor_modules_table[i].min_eventid) && (event_id <= imonitor_modules_table[i].max_eventid)) {
			module_index = &imonitor_modules_table[i];
			break;
		}
	}
	if (module_index) {
		/* Look for event_id description */
		for (i = 0; i < module_index->events_count; i++) {
			if (event_id == module_index->events[i].eventid) {
				event_index = &module_index->events[i];
				break;
			}
		}
	}
	/* combined event obj struct */
	event_obj = vmalloc(sizeof(*event_obj));
	if (!event_obj)
		return NULL;

	if (memset_s(event_obj, sizeof(*event_obj), 0, sizeof(*event_obj)) != EOK) {
		vfree(event_obj);
		return NULL;
	}

	event_obj->event_id = event_id;
	hwlog_info("%s : %u", __func__, event_id);

	/* below are parameters related , no parameters for this event */
	if ((!event_index) || (event_index->params_count == 0)) {
		event_obj->params_count = 0;
		return (void *)event_obj;
	}

	event_obj->params_count = event_index->params_count;
	event_obj->desc = event_index->desc;

	return (void *)event_obj;
}
EXPORT_SYMBOL(imonitor_create_eventobj);

int imonitor_set_param(struct imonitor_eventobj *event_obj, unsigned short param_id, long value)
{
	unsigned short type;

	if (!event_obj) {
		hwlog_err("Bad param for %s", __func__);
		return -EINVAL;
	}

	if (param_id >= event_obj->params_count) {
		hwlog_err("Bad param for %s, param_id: %d", __func__, param_id);
		return -EINVAL;
	}

	type = event_obj->desc[param_id];
	switch (type) {
	case BIT_TYPE:
		if (((char)value) > 1) {
			hwlog_err("Invalid value for bit type, value: %d", (int)value);
			return -EINVAL;
		}
		/* go through down */
	case TINYINT_TYPE:
	case SMALLINT_TYPE:
	case INT_TYPE:
		imonitor_set_param_integer(event_obj, param_id, value);
		break;
	case FLOAT_TYPE:
		/* kernel not support float */
		break;
	case DATETIME_TYPE:
		if (value == 0) {
			hwlog_err("Invalid value for datetime type");
		} else {
			imonitor_set_param_string(event_obj, param_id, (char *)value);
		}
		break;
	case CLASS_TYPE:
		/* kernel not support class */
		break;
	default:
		if ((type > BASE_TYPE_MAX) && (type <= VARCHAR_TYPE)) {
			hwlog_err("type error: %d", type);
			return -EINVAL;
		}
		if (value == 0) {
			hwlog_err("Invalid value for varchar type");
		} else {
			imonitor_set_param_string(event_obj, param_id, (char *)value);
		}
		break;
	}

	return 0;
}
EXPORT_SYMBOL(imonitor_set_param);

int imonitor_set_param_integer(struct imonitor_eventobj *event_obj, unsigned short param_id, long value)
{
	struct imonitor_param *param = NULL;

	if (!event_obj) {
		hwlog_err("Bad param for %s", __func__);
		return -EINVAL;
	}

	check_v1_api(event_obj);
	param = get_imonitor_param(event_obj->head, (int)param_id, NULL);
	if (!param) {
		param = create_imonitor_param();
		if (!param)
			return -ENOMEM;
		param->key = param_id;
		add_imonitor_param(event_obj, param);
	}
	if (param->value)
		vfree(param->value);
	param->value = vmalloc(INT_TYPE_MAX_LEN);
	if (!param->value)
		return -ENOMEM;
	if (memset_s(param->value, INT_TYPE_MAX_LEN, 0, INT_TYPE_MAX_LEN) != EOK)
		return -ENOMEM;
	if (snprintf_s(param->value, INT_TYPE_MAX_LEN, INT_TYPE_MAX_LEN - 1, "%d", (int)value) < 0)
		return -ENOMEM;

	return 0;
}
EXPORT_SYMBOL(imonitor_set_param_integer);

int imonitor_set_param_string(struct imonitor_eventobj *event_obj, unsigned short param_id, const char *value)
{
	struct imonitor_param *param = NULL;
	int len;

	if ((!event_obj) || (!value)) {
		hwlog_err("Bad param for %s", __func__);
		return -EINVAL;
	}

	check_v1_api(event_obj);
	param = get_imonitor_param(event_obj->head, (int)param_id, NULL);
	if (!param) {
		param = create_imonitor_param();
		if (!param)
			return -ENOMEM;
		param->key = param_id;
		add_imonitor_param(event_obj, param);
	}
	if (param->value)
		vfree(param->value);
	len = strlen(value);
	/* prevent length larger than MAX_STR_LEN */
	if (len > MAX_STR_LEN)
		len = MAX_STR_LEN;
	param->value = vmalloc(len + 1);
	if (!param->value)
		return -ENOMEM;
	if (memset_s(param->value, len + 1, 0, len + 1) != EOK)
		return -ENOMEM;
	if (strncpy_s(param->value, len + 1, value, len) != EOK)
		return -ENOMEM;

	param->value[len] = '\0';

	return 0;
}
EXPORT_SYMBOL(imonitor_set_param_string);

int imonitor_unset_param(struct imonitor_eventobj *event_obj, unsigned short param_id)
{
	if (!event_obj) {
		hwlog_err("Bad param for %s", __func__);
		return -EINVAL;
	}

	check_v1_api(event_obj);
	del_imonitor_param(event_obj, (int)param_id, NULL);

	return 0;
}
EXPORT_SYMBOL(imonitor_unset_param);

int imonitor_set_param_integer_v2(struct imonitor_eventobj *event_obj, const char *param, long value)
{
	struct imonitor_param *i_param = NULL;

	if ((!event_obj) || (!param)) {
		hwlog_err("Bad param for %s", __func__);
		return -EINVAL;
	}

	check_v2_api(event_obj);
	i_param = get_imonitor_param(event_obj->head, INVALID_KEY, param);
	if (!i_param) {
		i_param = create_imonitor_param();
		if (!i_param)
			return -ENOMEM;
		i_param->key_v2 = kstrdup(param, GFP_ATOMIC);
		add_imonitor_param(event_obj, i_param);
	}
	if (i_param->value)
		vfree(i_param->value);
	i_param->value = vmalloc(INT_TYPE_MAX_LEN);
	if (!i_param->value)
		return -ENOMEM;
	if (memset_s(i_param->value, INT_TYPE_MAX_LEN, 0, INT_TYPE_MAX_LEN) != EOK)
		return -ENOMEM;
	if (snprintf_s(i_param->value, INT_TYPE_MAX_LEN, INT_TYPE_MAX_LEN - 1, "%d", (int)value) < 0)
		return -ENOMEM;

	return 0;
}
EXPORT_SYMBOL(imonitor_set_param_integer_v2);

int imonitor_set_param_string_v2(struct imonitor_eventobj *event_obj, const char *param, const char *value)
{
	struct imonitor_param *i_param = NULL;
	int len;

	if ((!event_obj) || (!param) || (!value)) {
		hwlog_err("Bad param for %s", __func__);
		return -EINVAL;
	}

	check_v2_api(event_obj);
	i_param = get_imonitor_param(event_obj->head, INVALID_KEY, param);
	if (!i_param) {
		i_param = create_imonitor_param();
		if (!i_param)
			return -ENOMEM;
		i_param->key_v2 = kstrdup(param, GFP_ATOMIC);
		add_imonitor_param(event_obj, i_param);
	}
	if (i_param->value)
		vfree(i_param->value);
	len = strlen(value);
	/* prevent length larger than MAX_STR_LEN */
	if (len > MAX_STR_LEN)
		len = MAX_STR_LEN;
	i_param->value = vmalloc(len + 1);
	if (!i_param->value)
		return -ENOMEM;
	if (memset_s(i_param->value, len + 1, 0, len + 1) != EOK)
		return -ENOMEM;
	if (strncpy_s(i_param->value, len + 1, value, len) != EOK)
		return -ENOMEM;

	i_param->value[len] = '\0';

	return 0;
}
EXPORT_SYMBOL(imonitor_set_param_string_v2);

int imonitor_unset_param_v2(struct imonitor_eventobj *event_obj, const char *param)
{
	if ((!event_obj) || (!param)) {
		hwlog_err("Bad param for %s", __func__);
		return -EINVAL;
	}
	check_v2_api(event_obj);
	del_imonitor_param(event_obj, INVALID_KEY, param);
	return 0;
}
EXPORT_SYMBOL(imonitor_unset_param_v2);

int imonitor_set_time(struct imonitor_eventobj *event_obj, long long seconds)
{
	if ((!event_obj) || (seconds == 0)) {
		hwlog_err("Bad param for %s", __func__);
		return -EINVAL;
	}
	event_obj->time = seconds;
	return 0;
}
EXPORT_SYMBOL(imonitor_set_time);

static int add_path(char **pool, int pool_len, const char *path)
{
	int i;

	if ((!path) || (path[0] == 0)) {
		hwlog_err("Bad param %s", __func__);
		return -EINVAL;
	}

	if ((pool_len <= 0) || (pool_len > MAX_PATH_NUMBER)) {
		hwlog_err("pool len is illegal");
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

int imonitor_add_dynamic_path(struct imonitor_eventobj *event_obj, const char *path)
{
	if (!event_obj) {
		hwlog_err("Bad param %s", __func__);
		return -EINVAL;
	}
	return add_path(event_obj->dynamic_path, MAX_PATH_NUMBER, path);
}
EXPORT_SYMBOL(imonitor_add_dynamic_path);

int imonitor_add_and_del_dynamic_path(struct imonitor_eventobj *event_obj, const char *path)
{
	if (!event_obj) {
		hwlog_err("Bad param %s", __func__);
		return -EINVAL;
	}
	return add_path(event_obj->dynamic_path_delete, MAX_PATH_NUMBER, path);
}
EXPORT_SYMBOL(imonitor_add_and_del_dynamic_path);

/*
 * make string ":" to "::", ";" to ";;", and remove newline character
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
	if (memset_s(regular, len + count * 2 + 1, 0, len + count * 2 + 1) != EOK) {
		vfree(regular);
		return NULL;
	}
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

static int imonitor_convert_string(struct imonitor_eventobj *event_obj, char **buf_ptr)
{
	int len;
	char *tmp = NULL;
	int tmp_len;
	unsigned int i;
	unsigned int key_count;
	struct imonitor_param *param = NULL;

	char *buf = vmalloc(EVENT_INFO_BUF_LEN);
	if (!buf) {
		*buf_ptr = NULL;
		return 0;
	}
	if (memset_s(buf, EVENT_INFO_BUF_LEN, 0, EVENT_INFO_BUF_LEN) != EOK) {
		vfree(buf);
		*buf_ptr = NULL;
		return 0;
	}

	len = EVENT_INFO_BUF_LEN - 1;
	tmp = buf;

	/* fill event_id */
	tmp_len = snprintf_s(tmp, len + 1, len, "eventid %d", event_obj->event_id);
	BUF_POINTER_FORWARD;

	/* fill the path */
	for (i = 0; i < MAX_PATH_NUMBER; i++) {
		if (!event_obj->dynamic_path[i])
			break;
		tmp_len = snprintf_s(tmp, len + 1, len, " -i %s", event_obj->dynamic_path[i]);
		BUF_POINTER_FORWARD;
	}

	for (i = 0; i < MAX_PATH_NUMBER; i++) {
		if (!event_obj->dynamic_path_delete[i])
			break;
		tmp_len = snprintf_s(tmp, len + 1, len, " -d %s", event_obj->dynamic_path_delete[i]);
		BUF_POINTER_FORWARD;
	}

	/* fill time */
	if (event_obj->time) {
		tmp_len = snprintf_s(tmp, len + 1, len, " -t %lld", event_obj->time);
		BUF_POINTER_FORWARD;
	}

	/* fill the param info */
	key_count = 0;
	param = event_obj->head;
	while (param) {
		char *value = NULL;
		char *regular_value = NULL;
		int need_free = 1;

		if (!param->value) {
			param = param->next;
			continue;
		}
		if (key_count == 0) {
			tmp_len = snprintf_s(tmp, len + 1, len, " --extra ");
			BUF_POINTER_FORWARD;
		}
		key_count++;

		/* fill key */
		if (param->key_v2)
			tmp_len = snprintf_s(tmp, len + 1, len, "%s:", param->key_v2);
		else
			tmp_len = snprintf_s(tmp, len + 1, len, "%d:", param->key);
		BUF_POINTER_FORWARD;

		/* fill value */
		tmp_len = 0;
		value = param->value;
		regular_value = make_regular(value);
		if (!regular_value) {
			regular_value = "NULL";
			need_free = 0;
		}
		tmp_len = snprintf_s(tmp, len + 1, len, "%s;", regular_value);
		if ((value != regular_value) && need_free)
			vfree(regular_value);
		BUF_POINTER_FORWARD;
		param = param->next;
	}

	*buf_ptr = buf;

	return (EVENT_INFO_BUF_LEN - len);
}

static int imonitor_write_log_exception(char *str, const int str_len)
{
	char temp_chr;
	char *str_ptr = str;
	int left_buf_len = str_len + 1;
	int sent_cnt = 0;

	while (left_buf_len > 0) {
		if (left_buf_len > EVENT_INFO_PACK_BUF_LEN) {
			temp_chr = str_ptr[EVENT_INFO_PACK_BUF_LEN - 1];
			str_ptr[EVENT_INFO_PACK_BUF_LEN - 1] = '\0';
			logbuf_to_exception(0, 0, IDAP_LOGTYPE_CMD, 1, str_ptr, EVENT_INFO_PACK_BUF_LEN);
			left_buf_len -= (EVENT_INFO_PACK_BUF_LEN - 1);
			str_ptr += (EVENT_INFO_PACK_BUF_LEN - 1);
			str_ptr[0] = temp_chr;
			sent_cnt++;
		} else {
			logbuf_to_exception(0, 0, IDAP_LOGTYPE_CMD, 0, str_ptr, left_buf_len);
			sent_cnt++;
			break;
		}
	}

	return sent_cnt;
}

static void imonitor_file_lock(struct file *file_ptr, int cmd)
{
	struct file_lock *fl = NULL;

	fl = locks_alloc_lock();
	if (!fl) {
		hwlog_err("%s alloc error", __func__);
		return;
	}
	fl->fl_file = file_ptr;
	fl->fl_owner = file_ptr;
	fl->fl_pid = 0;
	fl->fl_flags = FL_FLOCK;
	fl->fl_type = cmd;
	fl->fl_end = OFFSET_MAX;

#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 4, 0))
	locks_lock_file_wait(file_ptr, fl);
#else
	flock_lock_file_wait(file_ptr, fl);
#endif
	locks_free_lock(fl);
}

int imonitor_send_event(struct imonitor_eventobj *event_obj)
{
	char *str = NULL;
	int buf_len;
	int sent_packet;
	struct file *fp = NULL;

	if (!event_obj) {
		hwlog_err("Bad param %s", __func__);
		return -EINVAL;
	}

	buf_len = imonitor_convert_string(event_obj, &str);
	if (!str)
		return -EINVAL;
	fp = filp_open("/dev/hwlog_exception", O_WRONLY, 0);
	if (IS_ERR(fp)) {
		hwlog_info("%s open fail", __func__);
		sent_packet = imonitor_write_log_exception(str, buf_len);
	} else {
		imonitor_file_lock(fp, F_WRLCK);
		sent_packet = imonitor_write_log_exception(str, buf_len);
		imonitor_file_lock(fp, F_UNLCK);
		filp_close(fp, 0);
	}

	hwlog_info("%s : %u", __func__, event_obj->event_id);
	vfree(str);

	return sent_packet;
}
EXPORT_SYMBOL(imonitor_send_event);

void imonitor_destroy_eventobj(struct imonitor_eventobj *event_obj)
{
	int i;
	struct imonitor_param *payload = NULL;

	if (!event_obj)
		return;

	payload = event_obj->head;
	while (payload) {
		struct imonitor_param *del = payload;

		payload = payload->next;
		destroy_imonitor_param(del);
	}
	event_obj->head = NULL;
	for (i = 0; i < MAX_PATH_NUMBER; i++) {
		kfree(event_obj->dynamic_path[i]);
		event_obj->dynamic_path[i] = NULL;
		kfree(event_obj->dynamic_path_delete[i]);
		event_obj->dynamic_path_delete[i] = NULL;
	}
	vfree(event_obj);
}
EXPORT_SYMBOL(imonitor_destroy_eventobj);
