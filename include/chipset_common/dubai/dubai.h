#ifndef DUBAI_H
#define DUBAI_H

#include <linux/skbuff.h>

enum {
	WORK_ENTER = 0,
	WORK_EXIT,
};

extern void dubai_update_brightness(uint32_t brightness);
extern void dubai_log_kworker(unsigned long address, unsigned long long enter_time);
extern void dubai_log_uevent(const char *devpath, unsigned int action);
extern void dubai_log_binder_stats(int reply, uid_t c_uid, int c_pid, uid_t s_uid, int s_pid);
extern void dubai_log_packet_wakeup_stats(const char *tag, const char *key, int value);
extern void dubai_set_rtc_timer(const char *name, int pid);
extern void dubai_send_app_wakeup(const struct sk_buff *skb, unsigned int hook, unsigned int pf, int uid, int pid);
extern void dubai_update_suspend_abort_reason(const char *reason);

#endif // DUBAI_H
