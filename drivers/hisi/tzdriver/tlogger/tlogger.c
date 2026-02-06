/*
 * tlogger.c
 *
 * TEE Logging Subsystem, read the tee os log from log memory
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "tlogger.h"
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <asm/ioctls.h>
#include <linux/syscalls.h>
#include <securec.h>
#include "smc_smp.h"
#include "mailbox_mempool.h"
#include "teek_client_constants.h"
#include "tc_ns_client.h"
#include "teek_ns_client.h"
#include "log_cfg_api.h"
#include "tc_ns_log.h"
#include "ko_adapt.h"

/* for log item ----------------------------------- */
#define LOG_ITEM_MAGIC          0x5A5A
#define LOG_ITEM_LEN_ALIGN      64
#define LOG_ITEM_MAX_LEN        1024
#define LOG_READ_STATUS_ERROR   0x000FFFF

/* =================================================== */
#define LOGGER_LOG_TEEOS        "hisi_teelog" /* tee os log */
#define __TEELOGGERIO           0xBE /* for ioctl */


#define GET_VERSION_BASE       5
#define SET_READERPOS_CUR_BASE 6
#define SET_TLOGCAT_STAT_BASE  7
#define GET_TLOGCAT_STAT_BASE  8

/* get tee verison */
#define MAX_TEE_VERSION_LEN     256
#define TEELOGGER_GET_VERSION \
	_IOR(__TEELOGGERIO, GET_VERSION_BASE, char[MAX_TEE_VERSION_LEN])
/* set the log reader pos to current pos */
#define TEELOGGER_SET_READERPOS_CUR \
	_IO(__TEELOGGERIO, SET_READERPOS_CUR_BASE)
#define TEELOGGER_SET_TLOGCAT_STAT \
	_IO(__TEELOGGERIO, SET_TLOGCAT_STAT_BASE)
#define TEELOGGER_GET_TLOGCAT_STAT \
	_IO(__TEELOGGERIO, GET_TLOGCAT_STAT_BASE)

#ifdef CONFIG_TEE_KTRACE
#define GET_EVENT_KTRACE_BASE       9
#define TEELOGGER_GET_EVENT_KTRACE \
	_IOR(__TEELOGGERIO, GET_EVENT_KTRACE_BASE, char *)

#define TEEOS_KEVENT_TRACE_SIZE      (8 * SZ_2K)

char *g_event_buffer_start = NULL;
#endif

int g_tlogcat_f = 0;

#ifndef CONFIG_TEE_LOG_ACHIVE_PATH
#define CONFIG_TEE_LOG_ACHIVE_PATH "/data/log/tee/last_teemsg"
#endif

u32 g_last_read_offset = 0;

#define NEVER_USED_LEN 32U
#define LOG_ITEM_RESERVED_LEN 2U

/* 64 byte head + user log */
struct log_item {
	u8 never_used[NEVER_USED_LEN];
	u16 magic;
	u16 reserved0;
	u32 serial_no;
	s16 real_len; /* log real len */
	u16 buffer_len; /* log buffer's len, multiple of 32 bytes */
	u8 uuid[UUID_LEN];
	u8 log_source_type;
	u8 reserved[LOG_ITEM_RESERVED_LEN];
	u8 new_line; /* '\n' char, easy viewing log in bbox.bin file */
	u8 log_buffer[0];
};

/* --- for log mem --------------------------------- */
#define TEMP_LOG_MEM_SIZE          (10 * SZ_1K)

#define LOG_BUFFER_RESERVED_LEN    11U
#define VERSION_INFO_LEN           156U

/*
 * Log's buffer flag info, size: 64 bytes head + 156 bytes's version info.
 * For filed description:
 * last_pos : current log's end position, last log's start position.
 * write_loops: Write cyclically. Init value is 0, when memory is used
 *              up, the value add 1.
 */
struct log_buffer_flag {
	u32 reserved0;
	u32 last_pos;
	u32 write_loops;
	u32 log_level;
	u32 reserved[LOG_BUFFER_RESERVED_LEN];
	u32 max_len;
	u8 version_info[VERSION_INFO_LEN];
};

struct log_buffer {
	struct log_buffer_flag flag;
	u8 buffer_start[0];
};

static struct log_buffer *g_log_buffer = NULL;

struct tlogger_log {
	unsigned char *buffer_info; /* ring buffer info */
	struct mutex mutex_info; /* this mutex protects buffer_info */
	wait_queue_head_t wait_queue_head; /* wait queue head for reader */
	struct list_head logs; /* log channels list */
	struct miscdevice misc_device; /* misc device log */
	struct list_head readers; /* log's readers */
};

static LIST_HEAD(m_log_list);

struct tlogger_reader {
	struct tlogger_log *log; /* tlogger_log info data */
	struct list_head list; /* log entry in tlogger_log's list */
	/* Current reading position, start position of next read again */
	u32 r_off;
	u32 r_loops;
	u32 r_sn;
	u32 r_failtimes;
	u32 r_from_cur;
	u32 r_is_tlogf;
	bool r_all; /* whether this reader can read all entries */
	unsigned int r_ver;
};

static u32 g_log_mem_len = 0;
static u32 g_tlogcat_count = 0;
static struct tlogger_log *g_log;

static struct tlogger_log *get_reader_log(const struct file *file)
{
	struct tlogger_reader *reader = NULL;

	reader = file->private_data;
	if (!reader)
		return NULL;

	return reader->log;
}

static bool check_log_item_validite(struct log_item *item,
	u32 item_max_size)
{
	bool con = (item && (item->magic == LOG_ITEM_MAGIC) &&
		(item->buffer_len > 0) &&
		(item->real_len > 0) &&
		(item->buffer_len % LOG_ITEM_LEN_ALIGN == 0) &&
		(item->real_len <= item->buffer_len) &&
		((item->buffer_len - item->real_len) < LOG_ITEM_LEN_ALIGN) &&
		(item->buffer_len + sizeof(*item) <= item_max_size));

	return con;
}

static struct log_item *get_next_log_item(const u8 *buffer_start,
	u32 max_len, u32 read_pos, u32 scope_len, u32 *pos)
{
	u32 i = 0;
	struct log_item *item = NULL;
	u32 max_size;

	if ((read_pos + scope_len) > max_len)
		return NULL;

	while ((i + sizeof(*item) + LOG_ITEM_LEN_ALIGN) <= scope_len) {
		*pos = read_pos + i;
		item = (struct log_item *)(buffer_start + read_pos + i);
		max_size = (((scope_len - i) > LOG_ITEM_MAX_LEN) ?
			LOG_ITEM_MAX_LEN : (scope_len - i));
		if (check_log_item_validite(item, max_size))
			break;

		i += LOG_ITEM_LEN_ALIGN;
		item = NULL;
	}

	return item;
}

struct reader_position {
	const u8 *buffer_start;
	u32 max_len;
	u32 start_pos;
	u32 end_pos;
};

static u32 parse_log_item(char __user *buf, size_t count,
	struct reader_position *position, u32 *read_off,
	bool *user_buffer_left)
{
	struct log_item *next_item = NULL;
	u32 buf_left;
	u32 buf_written;
	u32 item_len;
	bool con = false;
	u32 start_pos = position->start_pos;

	buf_written = 0;
	buf_left = count;

	con = (!read_off || !position->buffer_start);
	if (con)
		return buf_written;

	*user_buffer_left = true;
	while (start_pos < position->end_pos) {
		next_item = get_next_log_item(position->buffer_start,
			position->max_len, start_pos,
			position->end_pos - start_pos, &start_pos);
		if (!next_item)
			break;

		/* copy to user */
		item_len = next_item->buffer_len + sizeof(*next_item);
		if (buf_left < item_len) {
			*user_buffer_left = false;
			break;
		}

		start_pos += item_len;
		if (copy_to_user(buf + buf_written,
			(void *)next_item, item_len))
			tloge("copy failed, item len %u\n", item_len);

		buf_written += item_len;
		buf_left -= item_len;
	}

	*read_off = start_pos;
	return buf_written;
}

static ssize_t get_buffer_info(struct tlogger_reader *reader,
	struct log_buffer_flag *buffer_flag, struct log_buffer **log_buffer)
{
	struct tlogger_log *log = NULL;
	errno_t ret;
	struct log_buffer *buffer_tmp = NULL;

	log = reader->log;
	if (!log)
		return -EINVAL;

	buffer_tmp = (struct log_buffer*)log->buffer_info;
	if (!buffer_tmp)
		return -EINVAL;

	__asm__ volatile ("isb");
	__asm__ volatile ("dsb sy");

	mutex_lock(&log->mutex_info);
	ret = memcpy_s(buffer_flag, sizeof(*buffer_flag), &buffer_tmp->flag,
		sizeof(buffer_tmp->flag));
	mutex_unlock(&log->mutex_info);
	if (ret) {
		tloge("memcpy failed %d\n", ret);
		return -EAGAIN;
	}

	*log_buffer = buffer_tmp;
	return 0;
}

#define LOG_BUFFER_MAX_LEN      0x100000

static ssize_t get_last_read_pos(struct log_buffer_flag *log_flag,
	struct tlogger_reader *reader, u32 *log_last_pos, u32 *is_read)
{
	u32 buffer_max_len = g_log_mem_len - sizeof(*g_log_buffer);

	*is_read = 0;

	if (buffer_max_len > LOG_BUFFER_MAX_LEN)
		return -EINVAL;

	*log_last_pos = log_flag->last_pos;
	if (*log_last_pos == reader->r_off &&
		log_flag->write_loops == reader->r_loops)
		return 0;

	if (log_flag->max_len < *log_last_pos ||
		log_flag->max_len > buffer_max_len) {
		tloge("invalid data maxlen %x pos %x\n",
			log_flag->max_len , *log_last_pos);
		return -EFAULT;
	}

	if (reader->r_off > log_flag->max_len) {
		tloge("invalid data roff %x maxlen %x\n",
			reader->r_off , log_flag->max_len);
		return -EFAULT;
	}

	*is_read = 1;
	return 0;
}

static void set_reader_position(struct reader_position *position,
	const u8 *buffer_start, u32 max_len, u32 start_pos, u32 end_pos)
{
	position->buffer_start = buffer_start;
	position->max_len = max_len;
	position->start_pos = start_pos;
	position->end_pos = end_pos;
}

static ssize_t proc_read_ret(u32 buf_written,
	const struct tlogger_reader *reader)
{
	ssize_t ret;

	if (!buf_written) {
		ret = LOG_READ_STATUS_ERROR;
	} else {
		ret = buf_written;
		tlogd("read length %u\n", buf_written);
		g_last_read_offset = reader->r_off;
	}
	return ret;
}

static ssize_t check_read_params(struct file *file,
	char __user *buf, size_t count)
{
	if (count < LOG_ITEM_MAX_LEN)
		return -EINVAL;

	if (!file || !buf)
		return -EINVAL;

	return 0;
}

/*
 * If the sequence number of the last read position is smaller
 * than the current minimum sequence number, the last read
 * position is overwritten. And this time read data from
 * minimum number, or read data from last position.
 */
static ssize_t trigger_parse_log(char __user *buf, size_t count,
	u32 log_last_pos, struct log_buffer *log_buffer,
	struct tlogger_reader *reader)
{
	bool user_buffer_left = false;
	u32 buf_written;
	struct reader_position position = {0};
	struct log_buffer_flag *buffer_flag = &(log_buffer->flag);

	if (buffer_flag->write_loops == reader->r_loops) {
		set_reader_position(&position, log_buffer->buffer_start,
			buffer_flag->max_len, reader->r_off, log_last_pos);

		buf_written = parse_log_item(buf, count, &position,
			&reader->r_off, &user_buffer_left);

		return proc_read_ret(buf_written, reader);
	}

	if (buffer_flag->write_loops > (reader->r_loops +1) ||
		((buffer_flag->write_loops == (reader->r_loops + 1)) &&
		(reader->r_off < log_last_pos))) {
		reader->r_off = log_last_pos;
		reader->r_loops = buffer_flag->write_loops - 1;
	}

	set_reader_position(&position, log_buffer->buffer_start,
		buffer_flag->max_len, reader->r_off, buffer_flag->max_len);

	buf_written = parse_log_item(buf, count, &position,
		&reader->r_off, &user_buffer_left);

	if (count > buf_written && user_buffer_left) {
		set_reader_position(&position, log_buffer->buffer_start,
			buffer_flag->max_len, 0, log_last_pos);

		buf_written += parse_log_item(buf + buf_written,
			count - buf_written, &position,
			&reader->r_off, &user_buffer_left);

		reader->r_loops = buffer_flag->write_loops;
	}

	return proc_read_ret(buf_written, reader);
}

static ssize_t process_tlogger_read(struct file *file,
	char __user *buf, size_t count, loff_t *pos)
{
	struct tlogger_reader *reader = NULL;
	struct log_buffer *log_buffer = NULL;
	ssize_t ret;
	u32 last_pos;
	u32 is_read;
	struct log_buffer_flag buffer_flag;

	(void)pos;

	ret = check_read_params(file, buf, count);
	if (ret)
		return ret;

	reader = file->private_data;
	if (!reader)
		return -EINVAL;

	ret = get_buffer_info(reader, &buffer_flag, &log_buffer);
	if (ret)
		return ret;

	ret = get_last_read_pos(&buffer_flag, reader, &last_pos, &is_read);
	if (!is_read)
		return ret;

	return trigger_parse_log(buf, count, last_pos, log_buffer, reader);
}

void tz_log_write(void)
{
	struct log_buffer *log_buffer = NULL;

	if (!g_log)
		return;

	log_buffer = (struct log_buffer*)g_log->buffer_info;
	if (!log_buffer)
		return;

	if (g_last_read_offset != log_buffer->flag.last_pos) {
		tlogd("wake up write tz log\n");
		wake_up_interruptible(&g_log->wait_queue_head);
	}

	return;
}

static struct tlogger_log *get_tlogger_log_by_minor(int minor)
{
	struct tlogger_log *log = NULL;

	list_for_each_entry(log, &m_log_list, logs) {
		if (log->misc_device.minor == minor)
			return log;
	}

	return NULL;
}

static int process_tlogger_open(struct inode *inode,
	struct file *file)
{
	struct tlogger_log *log = NULL;
	int ret;
	struct tlogger_reader *reader = NULL;

	tlogd("open logger open ++\n");
	/* not support seek */
	ret = nonseekable_open(inode, file);
	if (ret)
		return ret;

	tlogd("Before get log from minor\n");
	log = get_tlogger_log_by_minor(MINOR(inode->i_rdev));
	if (!log)
		return -ENODEV;

	reader = kmalloc(sizeof(*reader), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)reader))
		return -ENOMEM;

	reader->log = log;
	reader->r_all = true;
	reader->r_off = 0;
	reader->r_loops = 0;
	reader->r_sn = 0;
	reader->r_failtimes = 0;
	reader->r_is_tlogf = 0;
	reader->r_from_cur = 0;

	INIT_LIST_HEAD(&reader->list);

	mutex_lock(&log->mutex_info);
	list_add_tail(&reader->list, &log->readers);
	g_tlogcat_count++;
	mutex_unlock(&log->mutex_info);

	file->private_data = reader;
	tlogd("tlogcat count %u\n", g_tlogcat_count);
	return 0;
}

static int process_tlogger_release(struct inode *ignored,
	struct file *file)
{
	struct tlogger_reader *reader = NULL;
	struct tlogger_log *log = NULL;

	(void)ignored;

	tlogd("logger_release ++\n");

	if (!file)
		return -1;

	reader = file->private_data;
	if (!reader) {
		tloge("reader is null\n");
		return -1;
	}

	log = reader->log;
	if (!log) {
		tloge("log is null\n");
		return -1;
	}

	mutex_lock(&log->mutex_info);
	list_del(&reader->list);
	if (g_tlogcat_count >= 1)
		g_tlogcat_count--;
	mutex_unlock(&log->mutex_info);

	tlogi("logger_release r_is_tlogf-%u\n", reader->r_is_tlogf);
	if (reader->r_is_tlogf)
		g_tlogcat_f = 0;

	kfree(reader);
	tlogd("tlogcat count %u\n", g_tlogcat_count);
	return 0;
}

static unsigned int process_tlogger_poll(struct file *file,
	poll_table *wait)
{
	struct tlogger_reader *reader = NULL;
	struct tlogger_log *log = NULL;
	struct log_buffer *buffer = NULL;
	unsigned int ret = POLLOUT | POLLWRNORM;

	tlogd("logger_poll ++\n");
	if (!file) {
		tloge("file is null\n");
		return ret;
	}

	reader = file->private_data;
	if (!reader) {
		tloge("the private data is null\n");
		return ret;
	}

	log = reader->log;
	if (!log) {
		tloge("log is null\n");
		return ret;
	}

	buffer = (struct log_buffer*)log->buffer_info;
	if (!buffer) {
		tloge("buffer is null\n");
		return ret;
	}

	poll_wait(file, &log->wait_queue_head, wait);

	if (buffer->flag.last_pos != reader->r_off)
		ret |= POLLIN | POLLRDNORM;

	return ret;
}

#define SET_READ_POS   1U
static void set_reader_cur_pos(const struct file *file)
{
	struct tlogger_reader *reader = NULL;
	struct tlogger_log *log = NULL;
	struct log_buffer *buffer = NULL;

	reader = file->private_data;
	if (!reader)
		return;

	log = reader->log;
	if (!log)
		return;

	buffer = (struct log_buffer*)log->buffer_info;
	if (!buffer)
		return;

	reader->r_from_cur = SET_READ_POS;
	reader->r_off = buffer->flag.last_pos;
	reader->r_loops = buffer->flag.write_loops;
}

static void set_tlogcat_f_stat(const struct file *file)
{
	struct tlogger_reader *reader = NULL;

	if (!file)
		return;

	reader = file->private_data;
	if (!reader)
		return;

	reader->r_is_tlogf = 1;
	g_tlogcat_f = 1;

	tlogi("set tlogcat_f-%u\n", g_tlogcat_f);
	return;
}

static int get_tlogcat_f_stat(void)
{
	tlogi("get tlogcat_f-%u\n", g_tlogcat_f);
	return g_tlogcat_f;
}

static int check_user_arg(unsigned long arg, size_t arg_len)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 18) || \
	LINUX_VERSION_CODE == KERNEL_VERSION(4, 19, 71))
	return (int)access_ok(VERIFY_READ,
		(void __user *)(uintptr_t)arg, arg_len);
#else
	return (int)access_ok((void __user *)(uintptr_t)arg, arg_len);
#endif
}

static int get_teeos_version(unsigned int cmd, unsigned long arg)
{
	int ret;

	if (!(_IOC_DIR(cmd) & _IOC_READ)) {
		tloge("check get version cmd failed\n");
		return -1;
	}

	ret = check_user_arg(arg,
		sizeof(g_log_buffer->flag.version_info));
	if (!ret) {
		tloge("check version info arg failed\n");
		return -1;
	}

	if (copy_to_user((void __user *)(uintptr_t)arg,
		(void *)g_log_buffer->flag.version_info,
		sizeof(g_log_buffer->flag.version_info))) {
		tloge("version info copy failed\n");
		return -1;
	}

	return 0;
}

#ifdef CONFIG_TEE_KTRACE
static int get_event_trace(unsigned int cmd, unsigned long arg)
{
	int ret;

	if (!(_IOC_DIR(cmd) & _IOC_READ)) {
		tloge("check get event trace cmd failed\n");
		return -1;
	}

	ret = check_user_arg(arg, TEEOS_KEVENT_TRACE_SIZE);
	if (!ret || !g_event_buffer_start) {
		tloge("invalid event buffer or accesss failed\n");
		return -1;
	}

	if (copy_to_user((void __user *)(uintptr_t)arg,
		(void *)g_event_buffer_start, TEEOS_KEVENT_TRACE_SIZE)) {
		tloge("kernel trace copy failed ret %d\n", ret);
		return -1;
	}
	return 0;
}
#endif

static long process_tlogger_ioctl(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	struct tlogger_log *log = NULL;
	long ret = -EINVAL;

	if (!file)
		return -1;

	log = get_reader_log(file);
	if (!log) {
		tloge("log is null\n");
		return -1;
	}

	tlogd("logger_ioctl start ++\n");
	mutex_lock(&log->mutex_info);

	switch (cmd) {
	case TEELOGGER_GET_VERSION:
		if (!get_teeos_version(cmd, arg))
			ret = 0;
		break;
	case TEELOGGER_SET_READERPOS_CUR:
		set_reader_cur_pos(file);
		ret = 0;
		break;
	case TEELOGGER_SET_TLOGCAT_STAT:
		set_tlogcat_f_stat(file);
		ret = 0;
		break;
	case TEELOGGER_GET_TLOGCAT_STAT:
		ret = get_tlogcat_f_stat();
		break;
#ifdef CONFIG_TEE_KTRACE
	case TEELOGGER_GET_EVENT_KTRACE:
		if (!get_event_trace(cmd, arg))
			ret = 0;
		break;
#endif
	default:
		tloge("ioctl error default\n");
		break;
	}

	mutex_unlock(&log->mutex_info);
	return ret;
}

#ifdef CONFIG_COMPAT
static long process_tlogger_compat_ioctl(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	tlogd("logger_compat_ioctl ++\n");
	arg = (unsigned long)(uintptr_t)compat_ptr(arg);
	return process_tlogger_ioctl(file, cmd, arg);
}
#endif

static const struct file_operations g_logger_fops = {
	.owner = THIS_MODULE,
	.read = process_tlogger_read,
	.poll = process_tlogger_poll,
	.unlocked_ioctl = process_tlogger_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = process_tlogger_compat_ioctl,
#endif
	.open = process_tlogger_open,
	.release = process_tlogger_release,
};

static int __init register_device(const char *log_name,
	uintptr_t addr, int size)
{
	int ret;
	struct tlogger_log *log = NULL;
	unsigned char *buffer = (unsigned char *)addr;

	log = kzalloc(sizeof(*log), GFP_KERNEL);
	if (!log) {
		tloge("kzalloc is failed\n");
		return -ENOMEM;
	}
	log->buffer_info = buffer;
	log->misc_device.minor = MISC_DYNAMIC_MINOR;
	log->misc_device.name = kstrdup(log_name, GFP_KERNEL);
	if (!log->misc_device.name) {
		ret = -ENOMEM;
		tloge("kstrdup is failed\n");
		goto out_free_log;
	}
	log->misc_device.fops = &g_logger_fops;
	log->misc_device.parent = NULL;

	init_waitqueue_head(&log->wait_queue_head);
	INIT_LIST_HEAD(&log->readers);
	mutex_init(&log->mutex_info);
	INIT_LIST_HEAD(&log->logs);
	list_add_tail(&log->logs, &m_log_list);

	/* register misc device for this log */
	ret = misc_register(&log->misc_device);
	if (unlikely(ret)) {
		tloge("failed to register misc device:%s\n",
			log->misc_device.name);
		goto out_free_log;
	}
	g_log = log;
	return 0;

out_free_log:
	if (log->misc_device.name)
		kfree(log->misc_device.name);

	kfree(log);
	return ret;
}

static struct log_item *lastmsg_get_next(const u8 *buffer_start,
	u32 read_pos, int scope_len, u32 max_len)
{
	int i = 0;
	struct log_item *item = NULL;
	u32 item_max_size;
	u32 len;

	while (i <= scope_len &&
		((read_pos + i + sizeof(*item)) < max_len)) {
		len = (u32)(scope_len - i);
		item_max_size =
			((len > LOG_ITEM_MAX_LEN) ? LOG_ITEM_MAX_LEN : len);
		item = (struct log_item *)(buffer_start + read_pos + i);

		if (check_log_item_validite(item, item_max_size)) {
			if ((read_pos + i + sizeof(*item) +
				item->buffer_len) > max_len) {
				tloge("check item len error\n");
				return NULL;
			}

			return item;
		}

		i += LOG_ITEM_LEN_ALIGN;
		item = NULL;
	}

	return NULL;
}

#define OPEN_FILE_MODE          0640U
#define ROOT_UID                0
#define ROOT_GID                0

static int tlogger_chown(void)
{
	uid_t user = ROOT_UID;
	gid_t group = ROOT_GID;
	int ret;

	get_log_chown(&user, &group);

	/* not need modify chown attr */
	if (group == ROOT_GID && user == ROOT_UID)
		return 0;

	ret = (int)koadpt_sys_chown(
		(const char __user *)CONFIG_TEE_LOG_ACHIVE_PATH, user, group);
	if (ret) {
		tloge("sys chown for last teemsg file error\n");
		return -1;
	}

	return 0;
}

static int write_version_to_lastmsg(struct file *filep,
	loff_t *pos)
{
	ssize_t write_len;

	/* first write tee versino info */
	write_len = koadpt_vfs_write(filep, g_log_buffer->flag.version_info,
		strlen(g_log_buffer->flag.version_info), pos);
	if (write_len < 0) {
		tloge("Failed to write to last teemsg version\n");
		return -1;
	}

	tlogd("Succeed to Write to last teemsg version, len=%ld\n", write_len);
	return 0;
}

static int write_log_to_lastmsg(struct file *filep,
	const u8 *buffer, u32 buffer_max_len, loff_t *pos)
{
	u32 read_off = 0;
	struct log_item *next_item = NULL;
	ssize_t item_len;
	ssize_t total_len = 0;
	ssize_t write_len;

	next_item = lastmsg_get_next(buffer, read_off,
		LOG_ITEM_MAX_LEN, buffer_max_len);

	while (next_item) {
		item_len = next_item->buffer_len + sizeof(*next_item);
		write_len = koadpt_vfs_write(filep, next_item->log_buffer,
			next_item->real_len, pos);
		if (write_len < 0) {
			tloge("Failed to write last teemsg %zd\n", write_len);
			return -1;
		}

		tlogd("Succeed to Write last teemsg, len=%ld\n", write_len);
		total_len += item_len;
		read_off = (u8 *)next_item - buffer + item_len;
		if (total_len >= buffer_max_len)
			break;

		next_item = lastmsg_get_next(buffer, read_off,
			LOG_ITEM_MAX_LEN, buffer_max_len);
	}

	return 0;
}

static int get_lastmsg_buffer(u8 **buffer, u32 *buffer_max_len)
{
	errno_t rc;
	int ret;
	u8 *addr = NULL;

	if (!g_log_buffer)
		return -1;

	*buffer_max_len = g_log_mem_len - sizeof(*g_log_buffer);

	if (*buffer_max_len > LOG_BUFFER_MAX_LEN)
		return 0;

	addr = kmalloc(*buffer_max_len, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)addr)) {
		ret = -ENOMEM;
		goto free_res;
	}

	rc = memcpy_s(addr, *buffer_max_len, g_log_buffer->buffer_start,
		*buffer_max_len);
	if (rc) {
		tloge("memcpy failed %d\n", rc);
		ret = -EAGAIN;
		goto free_res;
	}

	*buffer = addr;
	return 0;

free_res:
	if (addr)
		kfree(addr);

	return ret;
}

static int open_lastmsg_file(struct file **file)
{
	struct file *filep = NULL;

	filep = filp_open(CONFIG_TEE_LOG_ACHIVE_PATH,
		O_CREAT | O_RDWR | O_TRUNC, OPEN_FILE_MODE);
	if (!filep || IS_ERR(filep)) {
		tloge("open last teemsg file err %ld\n", PTR_ERR(filep));
		return -1;
	}

	*file = filep;
	return 0;
}

int tlogger_store_lastmsg(void)
{
	struct file *filep = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;
	int ret;
	u32 buffer_max_len;
	u8 *buffer = NULL;

	if (!g_tlogcat_count) {
		tlogd("tlogcat count %u\n", g_tlogcat_count);
		return 0;
	}

	/* copy logs from log memory, then parse the logs */
	ret = get_lastmsg_buffer(&buffer, &buffer_max_len);
	if (ret)
		return ret;

	/* exception handling, store trustedcore exception info to file */
	ret = open_lastmsg_file(&filep);
	if (ret)
		goto free_res;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	ret = tlogger_chown();
	if (ret)
		goto clear_fs;

	ret = write_version_to_lastmsg(filep, &pos);
	if (ret)
		goto clear_fs;

	ret = write_log_to_lastmsg(filep, buffer, buffer_max_len, &pos);

clear_fs:
	set_fs(old_fs);
free_res:
	if (buffer) {
		kfree(buffer);
		buffer = NULL;
	}

	if (filep)
		filp_close(filep, 0);

	/* trigger write teeos log */
	tz_log_write();
	return ret;
}

int register_mem_to_teeos(uint64_t mem_addr, u32 mem_len, bool is_cache_mem)
{
	struct tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	struct mb_cmd_pack *mb_pack = NULL;
	int ret;

	mb_pack = mailbox_alloc_cmd_pack();
	if (!mb_pack) {
		tloge("mailbox alloc failed\n");
		return -ENOMEM;
	}

	smc_cmd.global_cmd = true;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_REGISTER_LOG_MEM;
	mb_pack->operation.paramtypes = teec_param_types(
		TEE_PARAM_TYPE_VALUE_INPUT,
		TEE_PARAM_TYPE_VALUE_INPUT,
		TEE_PARAM_TYPE_VALUE_INPUT,
		TEE_PARAM_TYPE_NONE);
	mb_pack->operation.params[0].value.a = mem_addr;
	mb_pack->operation.params[0].value.b = mem_addr >> ADDR_TRANS_NUM;
	mb_pack->operation.params[1].value.a = mem_len;
	/*
	 * is_cache_mem: true, teeos map this memory for cache
	 * style; or else map to no cache style
	 */
	mb_pack->operation.params[2].value.a = is_cache_mem;

	smc_cmd.operation_phys = virt_to_phys(&mb_pack->operation);
	smc_cmd.operation_h_phys =
		virt_to_phys(&mb_pack->operation) >> ADDR_TRANS_NUM;
	ret = tc_ns_smc(&smc_cmd);
	mailbox_free(mb_pack);
	if (ret)
		tloge("Send log mem info failed\n");

	return ret;
}

static int register_mem_cfg(uint64_t *addr, u32 *len)
{
	int ret;
	ret = register_log_mem(addr, len);
	if (ret)
		tloge("register log mem failed %x\n", ret);

	ret = register_log_exception();
	if (ret)
		tloge("teeos register exception to log module failed\n");

	return ret;
}

static int check_log_mem(uint64_t mem_addr, u32 mem_len)
{
#ifdef CONFIG_TEE_KTRACE
	if (mem_len < TEMP_LOG_MEM_SIZE ||
		mem_len - TEMP_LOG_MEM_SIZE < TEEOS_KEVENT_TRACE_SIZE)  {
		tloge("log mem init error, too small len:0x%x\n", mem_len);
		return -1;
	}
#else
	if (mem_len < TEMP_LOG_MEM_SIZE) {
		tloge("log mem init error, too small len:0x%x\n", mem_len);
		return -1;
	}
#endif

	if (!mem_addr) {
		tloge("mem init failed!!! addr is 0\n");
		return -1;
	}
	return 0;
}

static int register_tloger(void)
{
	int ret;
	uint64_t mem_addr = 0;

	ret = register_mem_cfg(&mem_addr, &g_log_mem_len);
	if (ret)
		return ret;

	ret = check_log_mem(mem_addr, g_log_mem_len);
	if (ret)
		return ret;

	g_log_buffer =
		(struct log_buffer *)map_log_mem(mem_addr, g_log_mem_len);
	if (!g_log_buffer)
		return -ENOMEM;

#ifdef CONFIG_TEE_KTRACE
	g_log_mem_len = g_log_mem_len - TEEOS_KEVENT_TRACE_SIZE;
	g_event_buffer_start = ((char *)g_log_buffer) + g_log_mem_len;
#endif

	g_log_buffer->flag.max_len = g_log_mem_len - sizeof(*g_log_buffer);

	tloge("tlogcat verison 1.0.0\n");
	ret = register_device(LOGGER_LOG_TEEOS, (uintptr_t)g_log_buffer,
		sizeof(*g_log_buffer) + g_log_buffer->flag.max_len);
	if (ret) {
		unmap_log_mem((int *)g_log_buffer);
		g_log_buffer = NULL;
#ifdef CONFIG_TEE_KTRACE
		g_event_buffer_start = NULL;
#endif
		g_log_mem_len = 0;
	}

	return ret;
}

static void unregister_mem_cfg(void)
{
	if (g_log_buffer)
		unmap_log_mem((int *)g_log_buffer);

	unregister_log_exception();
}

static void unregister_tlogger(void)
{
	struct tlogger_log *current_log = NULL;
	struct tlogger_log *next_log = NULL;

	list_for_each_entry_safe(current_log, next_log, &m_log_list, logs) {
		/* we have to delete all the entry inside m_log_list */
		misc_deregister(&current_log->misc_device);
		kfree(current_log->misc_device.name);
		list_del(&current_log->logs);
		kfree(current_log);
	}

	unregister_mem_cfg();
	g_log_buffer = NULL;
	g_log_mem_len = 0;
#ifdef CONFIG_TEE_KTRACE
	g_event_buffer_start = NULL;
#endif
}

static int __init init_tlogger_service(void)
{
	return register_tloger();
}

static void __exit exit_tlogger_service(void)
{
	unregister_tlogger();
}

#ifdef CONFIG_TZDRIVER
device_initcall(init_tlogger_service);
module_exit(exit_tlogger_service);

MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_DESCRIPTION("TrustCore Logger");
MODULE_VERSION("1.00");
#endif
