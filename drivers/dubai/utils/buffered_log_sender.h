#ifndef DUBAI_BUFFERED_LOG_SENDER_H
#define DUBAI_BUFFERED_LOG_SENDER_H

#define cal_log_entry_len(head, info, count)	((long long)sizeof(head) + (long long)(count) * (long long)sizeof(info))

enum {
	BUFFERED_LOG_MAGIC_PROC_CPUTIME = 0,
	BUFFERED_LOG_MAGIC_KWORKER,
	BUFFERED_LOG_MAGIC_UEVENT,
	BUFFERED_LOG_MAGIC_BINDER_STATS,
	BUFFERED_LOG_MAGIC_WS_LASTING_NAME,
	BUFFERED_LOG_MAGIC_SENSORHUB_TYPE_LIST,
};

struct buffered_log_entry {
	int length;
	int magic;
	unsigned char data[0];
} __packed;

extern int (*send_buffered_log)(const struct buffered_log_entry *entry);
struct buffered_log_entry *create_buffered_log_entry(long long size, int magic);
void free_buffered_log_entry(struct buffered_log_entry *entry);
void buffered_log_release(void);

#endif // DUBAI_BUFFERED_LOG_SENDER_H
