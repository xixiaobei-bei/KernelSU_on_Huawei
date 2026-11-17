/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: supply a feature to track the specific tcp sockets and record
 *              the statistics to /proc/net/tcp_socket node
 * Author: zhangge
 * Create: 2020-07-17
 */

#include "track_tcp_socket.h"

#include <linux/errno.h>
#include <linux/tcp.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <linux/netfilter_ipv4.h>
#include <linux/rtc.h>
#include <linux/time.h>
#include <linux/proc_fs.h>
#include <../fs/proc/internal.h>
#include <trace/events/irq.h>

#ifdef CONFIG_HUAWEI_TRACK_SOCKET_DEBUG

#define __init_systrace(irq) struct irqaction irq, *_irq
#define __start_systrace(irq) do {	\
	_irq = &irq;	\
	_irq->name = #irq;	\
	trace_irq_handler_entry(10086, _irq);	\
} while (0)
#define init_systrace(name) __init_systrace(_systrace_##name)
#define start_systrace(name)	__start_systrace(_systrace_##name)
#define stop_systrace(name)	trace_irq_handler_exit(10086, _irq, 0)

size_t g_total_memory;
static DEFINE_SPINLOCK(g_memory_lock);

static void inc_memory(mem_size)
{
	spin_lock_bh(&g_memory_lock);
	g_total_memory += (mem_size);
	spin_unlock_bh(&g_memory_lock);
	pr_info("[TRACK_TCP_%d] total: %u, inc: %u",
		__LINE__, g_total_memory, (mem_size));
}

static void dec_memory(mem_size)
{
	spin_lock_bh(&g_memory_lock);
	if (g_total_memory >= (mem_size))
		g_total_memory -= (mem_size);
	else
		pr_info("[TRACK_TCP_%d] Wrong! total: %u, dec: %u",
			__LINE__, g_total_memory, (mem_size));
	spin_unlock_bh(&g_memory_lock);
	pr_info("[TRACK_TCP_%d] total: %u, dec: %u",
		__LINE__, g_total_memory, (mem_size));
}

static inline void print_socket_info(struct socket_info *info)
{
	pr_info("[TRACK_TCP_%d] sk=%p, fd=%d, uid=%u, role=%u, id=%s",
		__LINE__, info->sk, info->fd, info->uid,
		info->role, info->app_name);
}

static inline void print_realtime_stats(struct socket_realtime_stats *stats)
{
	pr_info("[TRACK_TCP_%d] state:%u, sync:%u, win:%u",
		__LINE__, stats->socket_state, stats->sync_send_num,
		stats->window_size);
}

static inline void print_period_stats(struct socket_period_stats *stats)
{
	pr_info("[TRACK_TCP_%d] rtt(%u, [%u, %u]), send:%u, resend%u, recv:%u",
		__LINE__, stats->rtt, stats->rtt_min, stats->rtt_max,
		stats->tx_packets, stats->re_tx_packets, stats->rx_packets);
}

#else
#define init_systrace(name)
#define start_systrace(name)
#define stop_systrace(name)

#define print_socket_info(info)
#define print_realtime_stats(stats)
#define print_period_stats(stats)

#define inc_memory(size)
#define dec_memory(size)
#define show_struct_size()
#endif

#define JIFFIES_MS (1000 / HZ)
static inline u32 ms_convert_jiffies(u32 cycle)
{
	return cycle / JIFFIES_MS;
}

#define get_member_value(base, member_type, offset) ({	\
	void *_tmp = (void *)(base);			\
	(*(member_type *)(_tmp + offset)); })

#define member_offset(member) offsetof(struct socket_period_stats, member)

struct tcp_socket_stats *g_tcp_socket_stats;
struct record_period_para g_record_level[] = {
	{1000, 10}, {500, 20}, {1000, 30}, {500, 60}
};

static inline void proc_mutex_lock(void)
{
	mutex_lock(&g_tcp_socket_stats->proc_mutex);
}

static inline void proc_mutex_unlock(void)
{
	mutex_unlock(&g_tcp_socket_stats->proc_mutex);
}

static inline bool is_alphabet(char ch)
{
	return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

static inline bool is_digit(char ch)
{
	return ch >= '0' && ch <= '9';
}

static bool app_name_is_valid(const char *name, u32 size)
{
	u32 i;

	if (size == 0 || size >= TASK_COMM_LEN)
		return false;
	if (*name == '_')
		return false;
	for (i = 0; i < size; i++) {
		if (is_alphabet(name[i]))
			continue;
		if (is_digit(name[i]))
			continue;
		if (name[i] == '_')
			continue;
		return false;
	}
	if (name[size] != 0)
		return false;

	return true;
}

static inline void trans_to_state(struct socket_stats_cb *cb, u32 state)
{
	cb->state = state;
}

static inline bool is_stats_cb_valid(struct socket_stats_cb *cb)
{
	return cb->state == IDLE;
}

static inline bool is_stats_cb_working(struct socket_stats_cb *cb)
{
	return cb->state == WORKING;
}

static inline bool stats_data_can_show(struct socket_stats_cb *cb)
{
	return cb->state == WORKING || cb->state == WAITING_TO_DELETE;
}

static void update_rtt(struct socket_period_stats *stats, u32 srtt_us)
{
	stats->rtt = (srtt_us >> 3) / USEC_PER_MSEC;
	if (stats->rtt < stats->rtt_min)
		stats->rtt_min = stats->rtt;
	if (stats->rtt > stats->rtt_max)
		stats->rtt_max = stats->rtt;
}

static inline void reset_rtt(struct socket_period_stats *stats, u32 value)
{
	stats->rtt = value;
	stats->rtt_min = value;
	stats->rtt_max = value;
}

static struct socket_stats_saved *alloc_socket_stats_saved(u32 level)
{
	struct record_period_para *record_level = g_record_level;
	struct socket_stats_saved *stat_saved = NULL;
	size_t size;

	if (level < ARRAY_SIZE(g_record_level))
		record_level = &g_record_level[level];

	size = sizeof(struct socket_stats_saved) +
	       sizeof(struct socket_period_stats) * record_level->cnt;
	stat_saved = kzalloc(size, GFP_KERNEL);
	if (!stat_saved)
		return NULL;

	inc_memory(size);

	stat_saved->first = stat_saved->period_stats;
	stat_saved->head = stat_saved->period_stats;
	stat_saved->cur = stat_saved->head;
	stat_saved->end = stat_saved->head + (record_level->cnt - 1);

	return stat_saved;
}

static void destroy_saved_stats(struct socket_stats_saved *stat_saved)
{
	size_t size;

	if (!stat_saved)
		return;
	size = sizeof(struct socket_stats_saved) +
		   sizeof(struct socket_period_stats) *
		   (stat_saved->end - stat_saved->head + 1);
	kfree(stat_saved);
	dec_memory(size);
}

static void delete_global_timer(void)
{
	int i;

	spin_lock_bh(&g_tcp_socket_stats->lock);
	for (i = 0; i < MAX_TRACK_SOCKET_NUM; i++)
		if (g_tcp_socket_stats->used[i] == CB_USED_WITH_GLOBAL_TIMER)
			break;

	if (i == MAX_TRACK_SOCKET_NUM) {
		spin_unlock_bh(&g_tcp_socket_stats->lock);
		return;
	}
	spin_unlock_bh(&g_tcp_socket_stats->lock);

	del_timer_sync(&g_tcp_socket_stats->timer);
}

static void delete_self_timer(struct socket_stats_cb *stats_cb)
{
	spin_lock_bh(&stats_cb->lock);
	if (stats_cb->jcycle == 0) {
		spin_unlock_bh(&stats_cb->lock);
		return;
	}
	stats_cb->jcycle = 0;
	spin_unlock_bh(&stats_cb->lock);

	del_timer_sync(&stats_cb->timer);
}

static void init_stats_list_node(struct socket_stats_cb *stats_cb,
				 struct sock *sk,
				 const struct track_socket_value *value,
				 struct socket_stats_saved *stats_saved,
				 struct proc_list *proc_node)
{
	struct socket_info *info = NULL;

	spin_lock_bh(&stats_cb->lock);
	if (!is_stats_cb_valid(stats_cb)) {
		spin_unlock_bh(&stats_cb->lock);
		pr_err("[TRACK_TCP_%d] CB is used, check codes", __LINE__);
		return;
	}

	trans_to_state(stats_cb, WORKING);
	info = &stats_cb->socket_info;

	info->sk = sk;
	info->fd = value->fd;
	info->uid = sk->sk_uid.val;
	strncpy(info->app_name, value->app_name, TASK_COMM_LEN - 1);
	info->app_name[TASK_COMM_LEN - 1] = '\0';
	info->role = value->cmd_value.sub_cmds.role;
	info->sub_cmd_bits = value->cmd_value.sub_cmds.sub_cmd_bits;

	stats_cb->stats_saved = stats_saved;
	stats_cb->proc_node = proc_node;
#ifdef CONFIG_HUAWEI_TRACK_SOCKET_DEBUG
	pr_info("[TRACK_TCP_%d] sk:%p,fd:%d,uid:%u,id:%s,role:%u,subs:%d,proc:%p",
		__LINE__, info->sk, info->fd, info->uid, info->app_name,
		info->role, info->sub_cmd_bits, proc_node);
#endif
	spin_unlock_bh(&stats_cb->lock);
}

static void clear_socket_stats_info(int index)
{
	struct socket_stats_cb *tmp =
		&g_tcp_socket_stats->stats_cb[index];

	delete_self_timer(tmp);

	spin_lock_bh(&tmp->lock);
	if (is_stats_cb_valid(tmp)) {
		spin_unlock_bh(&tmp->lock);
		pr_err("[TRACK_TCP_%d] CB is valid, check codes", __LINE__);
		return;
	}

	destroy_saved_stats(tmp->stats_saved);

	tmp->jcycle = 0;
	tmp->proc_node = NULL;
	memset(&tmp->socket_info, 0, sizeof(struct socket_info));
	memset(&tmp->socket_stats, 0, sizeof(struct single_socket_stats));
	tmp->state = 0;

	trans_to_state(tmp, IDLE);
	spin_unlock_bh(&tmp->lock);

	spin_lock_bh(&g_tcp_socket_stats->lock);
	g_tcp_socket_stats->used[index] = CB_IDLE;
	g_tcp_socket_stats->cnt--;
	spin_unlock_bh(&g_tcp_socket_stats->lock);
}

static struct proc_list *find_proc_node(const struct proc_dir_entry *proc_entry)
{
	struct proc_list *pos = NULL;

	list_for_each_entry(pos, &g_tcp_socket_stats->proc_head, list) {
		if (pos->proc_entry == proc_entry)
			return pos;
	}

	return NULL;
}

static void show_socket_info(struct seq_file *seq,
			     const struct socket_info *info)
{
	struct timeval time;
	struct rtc_time rtc;

	seq_puts(seq, "Time: ");
	do_gettimeofday(&time);
	rtc_time_to_tm(time.tv_sec, &rtc);
	seq_printf(seq, "%d:%d:%d\n", rtc.tm_hour + 8, rtc.tm_min, rtc.tm_sec);

	seq_printf(seq, "uid:%u, app:%s, fd:%d, role:%u, localPort:%u, remotePort:%u\n",
		   info->uid, info->app_name, info->fd, info->role,
		   info->local_port, info->remote_port);
}

static void show_realtime_stats(
	struct seq_file *seq, const struct socket_stats_cb *stats_cb)
{
	const struct socket_realtime_stats *stats = NULL;
	u16 sub_cmd_bits;
	u16 mask = 0;

	stats = &stats_cb->socket_stats.realtime_stats;
	sub_cmd_bits = stats_cb->socket_info.sub_cmd_bits;

	if (sub_cmd_bits == 0) {
		seq_printf(seq, "state:%u, sync_tx:%u, sync_rx:%u, window:%u\n",
			   stats->socket_state, stats->sync_send_num,
			   stats->sync_recv_num, stats->window_size);
		return;
	}

	if (sub_cmd_bits & SUB_CMD_STATE)
		seq_printf(seq, "state: %u", stats->socket_state);
	if (sub_cmd_bits & SUB_CMD_SYNC_SEND)
		seq_printf(seq, "sync_send: %u", stats->sync_send_num);
	if (sub_cmd_bits & SUB_CMD_SYNC_RECV)
		seq_printf(seq, "sync_recv: %u", stats->sync_recv_num);
	if (sub_cmd_bits & SUB_CMD_WINDOW)
		seq_printf(seq, "window: %u", stats->window_size);

	mask |= SUB_CMD_STATE | SUB_CMD_SYNC_SEND |
	       SUB_CMD_SYNC_RECV | SUB_CMD_WINDOW;
	if (sub_cmd_bits & mask)
		seq_puts(seq, "\n");
}

static void show_each_stat(struct seq_file *seq, const char *name,
			   const struct socket_stats_saved *stats, int offset)
{
	struct socket_period_stats *tmp = NULL;

	seq_printf(seq, "%s\t", name);
	tmp = stats->first;
	while (tmp != stats->cur) {
		seq_printf(seq, "%u ", get_member_value(tmp, u32, offset));
		tmp = (tmp == stats->end) ? stats->head : (++tmp);
	}
	seq_printf(seq, "%u\n", get_member_value(tmp, u32, offset));
}

static void show_period_stats(struct seq_file *seq,
			      const struct socket_stats_cb *stats_cb)
{
	struct socket_stats_saved *stats = stats_cb->stats_saved;
	u16 sub_cmd_bits;

	sub_cmd_bits = stats_cb->socket_info.sub_cmd_bits;
	if (sub_cmd_bits == SUB_CMD_DEFAULT)
		sub_cmd_bits = 0xffff;
	if (sub_cmd_bits & SUB_CMD_RTT)
		show_each_stat(seq, "rtt(ms)", stats, member_offset(rtt));
	if (sub_cmd_bits & SUB_CMD_RTT_MIN)
		show_each_stat(seq, "rtt_min", stats, member_offset(rtt_min));
	if (sub_cmd_bits & SUB_CMD_RTT_MAX)
		show_each_stat(seq, "rtt_max", stats, member_offset(rtt_max));
	if (sub_cmd_bits & SUB_CMD_SEND)
		show_each_stat(seq, "tx", stats, member_offset(tx_packets));
	if (sub_cmd_bits & SUB_CMD_RESEND)
		show_each_stat(seq, "resend", stats,
			       member_offset(re_tx_packets));
	if (sub_cmd_bits & SUB_CMD_RECV)
		show_each_stat(seq, "rx", stats, member_offset(rx_packets));
}

static void show_each_socket_stats(struct seq_file *seq,
				   struct socket_stats_cb *stats_cb)
{
	init_systrace(show_each_socket_stats);

	start_systrace(show_each_socket_stats);

	show_socket_info(seq, &stats_cb->socket_info);
	show_realtime_stats(seq, stats_cb);
	show_period_stats(seq, stats_cb);

	stop_systrace(show_each_socket_stats);
}

static int track_tcp_socket_show(struct seq_file *seq, void *v)
{
	struct inode *inode = (struct inode *)seq->private;
	struct proc_dir_entry *proc_entry = PDE(inode);
	struct proc_list *proc_node = NULL;
	struct socket_stats_cb *pos = NULL;

	init_systrace(track_tcp_socket_show);

	proc_mutex_lock();
	proc_node = find_proc_node(proc_entry);
	if (!proc_node) {
		proc_mutex_unlock();
		pr_warn("[TRACK_TCP_%d] Can not find the proc node!", __LINE__);
		return 0;
	}

	start_systrace(track_tcp_socket_show);
	list_for_each_entry(pos, &proc_node->sk_head, sk_list) {
		spin_lock_bh(&pos->lock);
		if (stats_data_can_show(pos))
			show_each_socket_stats(seq, pos);
		spin_unlock_bh(&pos->lock);
	}
	stop_systrace(track_tcp_socket_show);
	proc_mutex_unlock();

	return 0;
}

static int track_tcp_socket_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, track_tcp_socket_show, inode);
}

static const struct file_operations g_track_tcp_socket_fops = {
	.owner	 = THIS_MODULE,
	.open	 = track_tcp_socket_seq_open,
	.read	 = seq_read,
};

static int generate_proc_info(const char *comm,
			      struct socket_stats_cb *stats_cb,
			      struct proc_dir_entry **proc_entry,
			      struct proc_list **proc_node)
{
	char name[MAX_PROC_NAME_LEN] = "app_";
	struct proc_list *pos = NULL;
	struct proc_dir_entry *entry = NULL;
	struct proc_list *node = NULL;
	size_t entry_size;

	strncat(name, comm, TASK_COMM_LEN);

	proc_mutex_lock();
	list_for_each_entry(pos, &g_tcp_socket_stats->proc_head, list) {
		if (strcmp(pos->proc_name, name) == 0) {
			*proc_entry = pos->proc_entry;
			*proc_node = pos;
			list_add_tail(&stats_cb->sk_list,
				      &pos->sk_head);
			proc_mutex_unlock();
			return 0;
		}
	}

	pr_info("[TRACK_TCP_%d] start to create proc %s", __LINE__, name);
	entry = proc_create(name, 0400, g_tcp_socket_stats->proc_parent,
			    &g_track_tcp_socket_fops);
	if (!entry) {
		proc_mutex_unlock();
		pr_err("[TRACK_TCP_%d] create proc failed:%s", __LINE__, name);
		return -ENOMEM;
	}
	entry_size = sizeof(struct proc_dir_entry) + sizeof(name);
	inc_memory(entry_size);

	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (unlikely(!node)) {
		pr_err("[TRACK_TCP_%d] fail to create proc: %s",
		       __LINE__, name);
		remove_proc_entry(name, g_tcp_socket_stats->proc_parent);
		proc_mutex_unlock();

		dec_memory(entry_size);
		return -ENOMEM;
	}

	inc_memory(sizeof(struct proc_list));

	*proc_entry = entry;
	*proc_node = node;
	pr_info("[TRACK_TCP_%d] create proc kzalloc %p, %s", __LINE__,
		node, name);
	strncpy(node->proc_name, name, MAX_PROC_NAME_LEN - 1);
	node->proc_name[MAX_PROC_NAME_LEN - 1] = '\0';
	node->proc_entry = entry;
	INIT_LIST_HEAD(&node->sk_head);
	list_add_tail(&node->list, &g_tcp_socket_stats->proc_head);
	list_add_tail(&stats_cb->sk_list, &node->sk_head);

	proc_mutex_unlock();

	return 0;
}

static void clear_proc_info(struct socket_stats_cb *stats_cb)
{
	struct proc_list *proc_node = NULL;

	proc_mutex_lock();
	proc_node = stats_cb->proc_node;
	pr_info("[TRACK_TCP_%d] proc_node = %p", __LINE__, proc_node);

	list_del_init(&stats_cb->sk_list);

	if (list_empty(&proc_node->sk_head)) {
		list_del(&proc_node->list);
		proc_node->proc_entry = NULL;
		dec_memory(sizeof(struct proc_dir_entry) +
			   sizeof(proc_node->proc_name));
		dec_memory(sizeof(struct proc_list));
		remove_proc_entry(proc_node->proc_name,
				  g_tcp_socket_stats->proc_parent);
		pr_info("[TRACK_TCP_%d] proc_name = %s", __LINE__,
			proc_node->proc_name);
		kfree(proc_node);
		stats_cb->proc_node = NULL;
	}

	proc_mutex_unlock();
}

static void update_proc_stats(struct socket_stats_cb *stats_cb)
{
	struct socket_stats_saved *stats_saved = stats_cb->stats_saved;

	print_socket_info(&stats_cb->socket_info);
	print_realtime_stats(&stats_cb->socket_stats.realtime_stats);
	print_period_stats(&stats_cb->socket_stats.period_stats);

	if (stats_saved->first == stats_saved->head) {
		if (stats_saved->cur != stats_saved->end) {
			stats_saved->cur++;
		} else {
			stats_saved->cur = stats_saved->head;
			stats_saved->first++;
		}
	} else if (stats_saved->first != stats_saved->end) {
		stats_saved->first++;
		stats_saved->cur++;
	} else {
		stats_saved->cur = stats_saved->end;
		stats_saved->first = stats_saved->head;
	}

	memcpy(&stats_saved->realtime_stats,
	       &stats_cb->socket_stats.realtime_stats,
	       sizeof(struct socket_realtime_stats));
	memcpy(stats_saved->cur,
	       &stats_cb->socket_stats.period_stats,
	       sizeof(struct socket_period_stats));

	reset_rtt(&stats_cb->socket_stats.period_stats,
		  stats_cb->socket_stats.period_stats.rtt);
}

static void update_global_proc_stats(unsigned long timer_addr)
{
	int i;
	struct timer_list *timer = NULL;
	struct socket_stats_cb *tmp = NULL;

#ifdef CONFIG_HUAWEI_TRACK_SOCKET_DEBUG
	pr_info("[TRACK_TCP_%d] global timer running", __LINE__);
#endif
	for (i = 0; i < MAX_TRACK_SOCKET_NUM; i++) {
		if (g_tcp_socket_stats->used[i] == CB_IDLE ||
		    g_tcp_socket_stats->used[i] == CB_USED_WITH_SELF_TIMER)
			continue;
		tmp = &g_tcp_socket_stats->stats_cb[i];
		spin_lock_bh(&tmp->lock);
		//lint -e{548}
		if (tmp->jcycle == 0 && is_stats_cb_working(tmp))
			update_proc_stats(tmp);
		spin_unlock_bh(&tmp->lock);
	}

	spin_lock_bh(&g_tcp_socket_stats->lock);
	timer = &g_tcp_socket_stats->timer;
	WARN_ON(timer_addr != (unsigned long)(uintptr_t)timer);

	timer->function = update_global_proc_stats;
	timer->data = timer_addr;
	mod_timer(timer, jiffies + g_tcp_socket_stats->jcycle);
	spin_unlock_bh(&g_tcp_socket_stats->lock);
}

static void start_global_timer(int index)
{
	struct timer_list *timer = NULL;

	spin_lock_bh(&g_tcp_socket_stats->lock);
	g_tcp_socket_stats->used[index] = CB_USED_WITH_GLOBAL_TIMER;
	timer = &g_tcp_socket_stats->timer;
	if (timer_pending(timer)) {
		spin_unlock_bh(&g_tcp_socket_stats->lock);
		return;
	}

	g_tcp_socket_stats->jcycle = ms_convert_jiffies(
					g_record_level[0].cycle);
	timer->expires = jiffies + g_tcp_socket_stats->jcycle;
	timer->function = update_global_proc_stats;
	timer->data = (unsigned long)(uintptr_t)timer;
	add_timer(timer);
	pr_info("[TRACK_TCP_%d] successfully to start global timer", __LINE__);
	spin_unlock_bh(&g_tcp_socket_stats->lock);
}

static void update_self_proc_stats(unsigned long stats_list_addr)
{
	struct timer_list *timer = NULL;
	struct socket_stats_cb *tmp =
		(struct socket_stats_cb *)stats_list_addr;

	spin_lock_bh(&tmp->lock);
	if (!is_stats_cb_working(tmp)) {
		spin_unlock_bh(&tmp->lock);
		return;
	}
	timer = &tmp->timer;

	timer->function = update_self_proc_stats;
	timer->data = stats_list_addr;
	mod_timer(timer, jiffies + tmp->jcycle);

	update_proc_stats(tmp);
	spin_unlock_bh(&tmp->lock);
}

static void start_self_timer(struct socket_stats_cb *stats_cb, u32 level)
{
	struct timer_list *timer = NULL;

	spin_lock_bh(&stats_cb->lock);
	if (!is_stats_cb_working(stats_cb)) {
		spin_unlock_bh(&stats_cb->lock);
		pr_err("[TRACK_TCP_%d] CB is not working!", __LINE__);
		return;
	}

	timer = &stats_cb->timer;
	if (timer_pending(timer)) {
		spin_unlock_bh(&stats_cb->lock);
		pr_warn("[TRACK_TCP_%d] self timer is working?!", __LINE__);
		return;
	}

	stats_cb->jcycle =
		ms_convert_jiffies(g_record_level[level].cycle);
	timer->expires = jiffies + stats_cb->jcycle;
	timer->function = update_self_proc_stats;
	timer->data = (unsigned long)(uintptr_t)stats_cb;
	add_timer(timer);
	pr_info("[TRACK_TCP_%d] successfully to start self timer", __LINE__);
	spin_unlock_bh(&stats_cb->lock);
}

static void start_record_timer(int index, u32 level)
{
	u32 tmp = (level < ARRAY_SIZE(g_record_level)) ? level : 0;
	struct socket_stats_cb *stats_cb = NULL;

	stats_cb = &g_tcp_socket_stats->stats_cb[index];
	if (tmp == 0)
		start_global_timer(index);
	else
		start_self_timer(stats_cb, tmp);
}

static void stop_record_timer(struct socket_stats_cb *stats_cb, u32 level)
{
	u32 tmp = (level < ARRAY_SIZE(g_record_level)) ? level : 0;

	if (tmp == 0)
		delete_global_timer();
	else
		delete_self_timer(stats_cb);
}

static void delete_stats_list_func(unsigned long timer_addr)
{
	int i;
	struct socket_stats_cb *stats_cb = NULL;

#ifdef CONFIG_HUAWEI_TRACK_SOCKET_DEBUG
	pr_info("[TRACK_TCP_%d] delete timer running", __LINE__);
#endif

	for (i = 0; i < MAX_TRACK_SOCKET_NUM; i++) {
		spin_lock_bh(&g_tcp_socket_stats->lock);
		if (g_tcp_socket_stats->used[i] == CB_IDLE) {
			spin_unlock_bh(&g_tcp_socket_stats->lock);
			continue;
		}
		spin_unlock_bh(&g_tcp_socket_stats->lock);

		stats_cb = &g_tcp_socket_stats->stats_cb[i];
		if (stats_cb->state != WAITING_TO_DELETE)
			continue;

		clear_proc_info(stats_cb);
		clear_socket_stats_info(i);
	}

	delete_global_timer();
}

static void start_delete_timer(void)
{
	struct timer_list *timer = NULL;

	spin_lock_bh(&g_tcp_socket_stats->lock);
	timer = &g_tcp_socket_stats->del_timer;
	timer->expires = jiffies + ms_convert_jiffies(DELETE_EXPIRE_TIME);
	timer->function = delete_stats_list_func;
	timer->data = (unsigned long)(uintptr_t)timer;

	mod_timer(timer, timer->expires);
	spin_unlock_bh(&g_tcp_socket_stats->lock);
}

static int is_track_socket_available(struct sock *sk,
				     const struct track_socket_value *value)
{
	if (!app_name_is_valid(value->app_name, value->name_size)) {
		pr_err("[TRACK_TCP_%d] invalid name!", __LINE__);
		return -EINVAL;
	}

	if (g_tcp_socket_stats->cnt >= MAX_TRACK_SOCKET_NUM) {
		pr_err("[TRACK_TCP_%d] can not track more, current num: %d",
		       __LINE__, g_tcp_socket_stats->cnt);
		return -ENOBUFS;
	}

	lock_sock(sk);
	if (sk->track_flag == 1) {
		pr_warn("[TRACK_TCP_%d] the socket:%d has been tracked, current num: %u",
			__LINE__, value->fd, g_tcp_socket_stats->cnt);
		release_sock(sk);
		return -EEXIST;
	}
	release_sock(sk);

	return 0;
}

static int choose_valid_stats_list_node(void)
{
	int i;

	spin_lock_bh(&g_tcp_socket_stats->lock);
	for (i = 0; i < MAX_TRACK_SOCKET_NUM; i++)
		if (g_tcp_socket_stats->used[i] == CB_IDLE)
			break;

	if (i == MAX_TRACK_SOCKET_NUM) {
		spin_unlock_bh(&g_tcp_socket_stats->lock);
		pr_warn("[TRACK_TCP_%d]reach the MAXMIUM, return, count = %u",
			__LINE__, g_tcp_socket_stats->cnt);
		return -1;
	}

	g_tcp_socket_stats->used[i] = CB_USED_WITH_SELF_TIMER;
	g_tcp_socket_stats->cnt++;
	spin_unlock_bh(&g_tcp_socket_stats->lock);

	return i;
}

struct socket_stats_cb *match_stats_list(struct sock *sk)
{
	int i;

	spin_lock_bh(&g_tcp_socket_stats->lock);
	for (i = 0; i < MAX_TRACK_SOCKET_NUM; i++) {
		if (g_tcp_socket_stats->used[i] == CB_IDLE)
			continue;
		if (g_tcp_socket_stats->stats_cb[i].socket_info.sk == sk) {
			spin_unlock_bh(&g_tcp_socket_stats->lock);
			return &g_tcp_socket_stats->stats_cb[i];
		}
	}
	spin_unlock_bh(&g_tcp_socket_stats->lock);

	pr_err("[TRACK_TCP_%d] CANNOT find the record!", __LINE__);
	return NULL;
}

static int start_track_tcp_socket(struct sock *sk,
				  const struct track_socket_value *value)
{
	int index;
	int err;
	struct socket_stats_cb *tmp = NULL;
	struct proc_dir_entry *proc_entry = NULL;
	struct socket_stats_saved *stat_saved = NULL;
	struct proc_list *proc_node = NULL;
	int level;

	pr_info("[TRACK_TCP_%d] start to track tcp socket of comm(%s)",
		__LINE__, current->comm);
	err = is_track_socket_available(sk, value);
	if (err != 0)
		return err;

	index = choose_valid_stats_list_node();
	if (index == -1)
		return -ENOBUFS;

	level = value->cmd_value.sub_cmds.level;
	stat_saved = alloc_socket_stats_saved(level);
	if (!stat_saved) {
		pr_err("[TRACK_TCP_%d] alloc failed", __LINE__);
		err = -ENOMEM;
		goto alloc_error;
	}

	tmp = &g_tcp_socket_stats->stats_cb[index];
	pr_info("[TRACK_TCP_%d] tmp(%p)", __LINE__, tmp);
	err = generate_proc_info(value->app_name, tmp,
				 &proc_entry, &proc_node);
	if (err != 0) {
		pr_err("[TRACK_TCP_%d] generate proc failed", __LINE__);
		goto proc_error;
	}

	init_stats_list_node(tmp, sk, value, stat_saved, proc_node);

	start_record_timer(index, level);

	lock_sock(sk);
	if (sk->track_flag == 1) {
		release_sock(sk);
		pr_err("[TRACK_TCP_%d] fail", __LINE__);
		err = -EEXIST;
		goto sock_error;
	}
	sk->track_flag = 1;
	sk->track_index = index;
	release_sock(sk);

	pr_info("[TRACK_TCP_%d] successful to track tcp socket of comm(%s), level:%d",
		__LINE__, current->comm, level);

	return err;

sock_error:
	stop_record_timer(tmp, level);

	clear_proc_info(tmp);
	clear_socket_stats_info(index);
	stat_saved = NULL;

proc_error:
	destroy_saved_stats(stat_saved);

alloc_error:
	spin_lock_bh(&g_tcp_socket_stats->lock);
	g_tcp_socket_stats->used[index] = CB_IDLE;
	g_tcp_socket_stats->cnt--;
	spin_unlock_bh(&g_tcp_socket_stats->lock);

	return err;
}

int stop_track_tcp_socket(struct sock *sk)
{
	int index = sk->track_index;
	struct socket_stats_cb *stats_cb = NULL;

	pr_info("[TRACK_TCP_%d] stop track sk:%p", __LINE__, sk);
	lock_sock(sk);
	if (sk->track_flag == 0) {
		release_sock(sk);
		return -EINVAL;
	}

	sk->track_flag = 0;
	sk->track_index = -1;
	release_sock(sk);

	stats_cb = match_stats_list(sk);
	if (stats_cb) {
		pr_info("[TRACK_TCP_%d] stop track sk:%p, stats_cb %p, proc_node %p",
			__LINE__, sk, stats_cb, stats_cb->proc_node);
		clear_proc_info(stats_cb);
	}
	clear_socket_stats_info(index);

	delete_global_timer();
	return 0;
}

int process_track_socket_cmd(struct sock *sk, const char __user *optval,
			     int optlen)
{
	int err = -EINVAL;
	struct track_socket_value value;

	if (optlen != sizeof(value))
		return -EINVAL;
	memset(&value, 0, sizeof(value));
	if (copy_from_user(&value, optval, optlen))
		return -EFAULT;
	pr_info("[TRACK_TCP_%d] process command, len=%d, type=%u",
		__LINE__, optlen, value.cmd_type);
	switch (value.cmd_type) {
	case START_TRACK_SOCKET:
		err = start_track_tcp_socket(sk, &value);
		break;
	case START_RECORD_STATS:
		err = 0;
		break;
	case STOP_TRACK_SOCKET:
		err = stop_track_tcp_socket(sk);
		break;
	default:
		err = -EINVAL;
		break;
	}

	return err;
}

void release_tcp_socket_tracking(struct sock *sk)
{
	int index;
	struct socket_stats_cb *stats_cb = NULL;

	if (!sk)
		return;

	lock_sock(sk);
	if (sk->track_flag == 0) {
		release_sock(sk);
		return;
	}
	index = sk->track_index;
	sk->track_flag = 0;
	sk->track_index = -1;
	release_sock(sk);

	stats_cb = &g_tcp_socket_stats->stats_cb[index];

	pr_info("[TRACK_TCP_%d]stop track sk:%p", __LINE__, sk);

	spin_lock_bh(&stats_cb->lock);
	trans_to_state(stats_cb, WAITING_TO_DELETE);
	spin_unlock_bh(&stats_cb->lock);

	start_delete_timer();
}

static void update_tcp_data(const struct sock *sk,
			    const struct sk_buff *skb, u8 hook,
			    struct socket_stats_cb *stats_cb)
{
	struct single_socket_stats *stats = &stats_cb->socket_stats;
	struct tcphdr *tcp_header = tcp_hdr(skb);
	u32 seq = ntohl(tcp_header->seq);
	struct inet_sock *inet = inet_sk(sk);
	struct tcp_sock *skt = NULL;

#ifdef CONFIG_HUAWEI_TRACK_SOCKET_DEBUG
	pr_info("[TRACK_TCP_%d] sk = %p, hook = %u, seq = %u",
		__LINE__, sk, hook, seq);
#endif
	stats->realtime_stats.socket_state = sk->sk_state;

	if (inet) {
		stats_cb->socket_info.local_port = inet->inet_sport;
		stats_cb->socket_info.remote_port = inet->inet_dport;
	}

	stats->realtime_stats.window_size = tcp_header->window;

	if (hook == NF_INET_POST_ROUTING) {
		if (sk->sk_state == TCP_ESTABLISHED) {
			if (seq <= stats->realtime_stats.max_send_seq)
				stats->period_stats.re_tx_packets++;
			else
				stats->realtime_stats.max_send_seq = seq;
			stats->period_stats.tx_packets++;
		} else if (sk->sk_state == TCP_SYN_SENT) {
			if (tcp_header->syn == 1)
				stats->realtime_stats.sync_send_num++;
		}

		return;
	}

	if (hook == NF_INET_LOCAL_IN) {
		if (sk->sk_state == TCP_ESTABLISHED) {
			stats->period_stats.rx_packets++;
			skt = (struct tcp_sock *)sk;
			update_rtt(&stats->period_stats, skt->srtt_us);
			return;
		}

		if (sk->sk_state == TCP_SYN_SENT && tcp_header->syn == 1)
			stats->realtime_stats.sync_recv_num++;
	}
}

static void update_tcp_socket_track_data(const struct sock *sk,
					 const struct sk_buff *skb,
					 const struct nf_hook_state *state)
{
	u8 hook = (u8)state->hook;
	struct socket_stats_cb *tmp = NULL;

	init_systrace(update_track_data);

	if (hook != NF_INET_LOCAL_IN && hook != NF_INET_POST_ROUTING)
		return;

	tmp = &g_tcp_socket_stats->stats_cb[sk->track_index];
	if (tmp->socket_info.sk != sk) {
		pr_err("[TRACK_TCP_%d] something is wrong!", __LINE__);
		return;
	}

	start_systrace(update_track_data);
	spin_lock_bh(&tmp->lock);
	if (is_stats_cb_working(tmp))
		update_tcp_data(sk, skb, hook, tmp);
	spin_unlock_bh(&tmp->lock);
	stop_systrace(update_track_data);
}

static unsigned int track_socket_hook_v4(void *priv,
					 struct sk_buff *skb,
					 const struct nf_hook_state *state)
{
	struct iphdr *iph = NULL;
	struct sock *sk = NULL;

	if (!skb)
		return NF_ACCEPT;
	sk = skb_to_full_sk(skb);
	if (!sk || !sk_fullsock(sk))
		return NF_ACCEPT;

	if (sk->track_flag != 1)
		return NF_ACCEPT;

	if (sk->track_index < 0 || sk->track_index >= MAX_TRACK_SOCKET_NUM)
		return NF_ACCEPT;

	iph = ip_hdr(skb);
	if (!iph)
		return NF_ACCEPT;
	if (iph->protocol == IPPROTO_TCP)
		update_tcp_socket_track_data(sk, skb, state);

	return NF_ACCEPT;
}

static struct nf_hook_ops g_net_hooks[] = {
	{
		.hook = track_socket_hook_v4,
		.pf = PF_INET,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP_PRI_FILTER,
	},
	{
		.hook = track_socket_hook_v4,
		.pf = PF_INET,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER,
	},
};

static int __init track_tcp_module_init(void)
{
	int i;
	int ret;
	struct socket_stats_cb *tmp = NULL;
	size_t g_tcp_socket_stats_size, stats_cbs_size, hooks_size;

	g_tcp_socket_stats_size = sizeof(struct tcp_socket_stats);
	g_tcp_socket_stats = kzalloc(g_tcp_socket_stats_size, GFP_KERNEL);
	if (!g_tcp_socket_stats) {
		ret = -ENOMEM;
		goto init_error;
	}
	inc_memory(g_tcp_socket_stats_size);

	spin_lock_init(&g_tcp_socket_stats->lock);
	init_timer(&g_tcp_socket_stats->timer);
	init_timer(&g_tcp_socket_stats->del_timer);
	INIT_LIST_HEAD(&g_tcp_socket_stats->proc_head);
	mutex_init(&g_tcp_socket_stats->proc_mutex);
	g_tcp_socket_stats->proc_parent = NULL;

	stats_cbs_size = sizeof(struct socket_stats_cb) * MAX_TRACK_SOCKET_NUM;
	g_tcp_socket_stats->stats_cb = kzalloc(stats_cbs_size, GFP_KERNEL);
	if (!g_tcp_socket_stats->stats_cb) {
		ret = -ENOMEM;
		goto init_error;
	}
	inc_memory(stats_cbs_size);

	for (i = 0; i < MAX_TRACK_SOCKET_NUM; i++) {
		tmp = &g_tcp_socket_stats->stats_cb[i];
		spin_lock_init(&tmp->lock);
		INIT_LIST_HEAD(&tmp->sk_list);
		tmp->socket_info.sk = NULL;
		tmp->proc_node = NULL;
		tmp->stats_saved = NULL;
		init_timer(&tmp->timer);
		tmp->state = IDLE;
	}

	ret = nf_register_net_hooks(&init_net, g_net_hooks,
				    ARRAY_SIZE(g_net_hooks));
	if (ret)
		goto init_error;
	hooks_size = sizeof(struct nf_hook_entries) +
		     sizeof(struct nf_hook_entry) * ARRAY_SIZE(g_net_hooks) +
		     sizeof(struct nf_hook_ops *) * ARRAY_SIZE(g_net_hooks);
	inc_memory(hooks_size);

	g_tcp_socket_stats->proc_parent = proc_mkdir("tcp_socket",
						     init_net.proc_net);
	if (!g_tcp_socket_stats->proc_parent) {
		ret = -ENOMEM;
		goto proc_error;
	}
	inc_memory(sizeof(struct proc_dir_entry) + sizeof("tcp_socket"));

	return ret;

proc_error:
	nf_unregister_net_hooks(&init_net, g_net_hooks,
				ARRAY_SIZE(g_net_hooks));
	dec_memory(hooks_size);

init_error:
	if (g_tcp_socket_stats->stats_cb) {
		for (i = 0; i < MAX_TRACK_SOCKET_NUM; i++)
			del_timer(&g_tcp_socket_stats->stats_cb[i].timer);
		kfree(g_tcp_socket_stats->stats_cb);
		dec_memory(stats_cbs_size);
		g_tcp_socket_stats->stats_cb = NULL;
	}

	if (g_tcp_socket_stats) {
		mutex_destroy(&g_tcp_socket_stats->proc_mutex);
		del_timer(&g_tcp_socket_stats->timer);
		del_timer(&g_tcp_socket_stats->del_timer);
		kfree(g_tcp_socket_stats);
		dec_memory(g_tcp_socket_stats_size);
		g_tcp_socket_stats = NULL;
	}

	return ret;
}

static void __exit track_tcp_module_exit(void)
{
	int i;
	size_t hooks_size;

	pr_info("[TRACK_TCP] module_exit, start.");

	nf_unregister_net_hooks(&init_net, g_net_hooks,
				ARRAY_SIZE(g_net_hooks));
	hooks_size = sizeof(struct nf_hook_entries) +
		     sizeof(struct nf_hook_entry) * ARRAY_SIZE(g_net_hooks) +
		     sizeof(struct nf_hook_ops *) * ARRAY_SIZE(g_net_hooks);
	dec_memory(hooks_size);

	del_timer_sync(&g_tcp_socket_stats->timer);
	del_timer_sync(&g_tcp_socket_stats->del_timer);

	for (i = 0; i < MAX_TRACK_SOCKET_NUM; i++) {
		del_timer(&g_tcp_socket_stats->stats_cb[i].timer);
		if (g_tcp_socket_stats->used[i] == CB_IDLE)
			continue;
		stop_track_tcp_socket(
			g_tcp_socket_stats->stats_cb[i].socket_info.sk);
	}

	if (g_tcp_socket_stats->stats_cb) {
		dec_memory(sizeof(struct socket_stats_cb) *
			   MAX_TRACK_SOCKET_NUM);
	}
	kfree(g_tcp_socket_stats->stats_cb);
	g_tcp_socket_stats->stats_cb = NULL;

	if (g_tcp_socket_stats->proc_parent) {
		remove_proc_entry("tcp_socket", init_net.proc_net);
		dec_memory(sizeof(struct proc_dir_entry) +
			   sizeof("tcp_socket"));
		g_tcp_socket_stats->proc_parent = NULL;
	}

	if (g_tcp_socket_stats) {
		mutex_destroy(&g_tcp_socket_stats->proc_mutex);
		kfree(g_tcp_socket_stats);
		dec_memory(sizeof(struct tcp_socket_stats));
		g_tcp_socket_stats = NULL;
	}
	pr_info("[TRACK_TCP] module_exit, end.");
}

module_init(track_tcp_module_init);
module_exit(track_tcp_module_exit);
