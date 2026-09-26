

#include <network_aware/network_aware.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/syslog.h>
#include <net/net_namespace.h>
#include <../fs/proc/internal.h>

/*
 * if happened in PERIOD_TIME jiffies, save in the same node
 * otherwise, create a new node
 */
#define PERIOD_TIME 1

/* when fg stat is beyond it, limit bg */
#define FG_RANGE_COUNT 0

/* us for bg to sleep */
#define SLEEP_TIME_MS 10

#define LIMIT_RATIO_SIZE 100
#define POSITION_INIT_NUM 100
#define AWARE_NET_CTRL_MODE_FIRSE_LEVEL 7
#define AWARE_NET_CTRL_MODE_SECOND_LEVEL 9
#define TIME_INTERVAL_INIT 5

#define STRTOL_SIZE 10
#define AWARE_FS_NET_PATH "aware"
#define AWARE_FS_FG_UIDS "fg_uids"
#define AWARE_FS_BG_UIDS "bg_uids"
#define AWARE_FS_CTRL "aware_ctrl"
#define TIME_MAX_SIZE (0xffffffff / TIME_LONG_OF_HZ)

static volatile unsigned long last_fg_time;
static volatile unsigned long cur_fg_index;

static unsigned long total_fg_rxtx_count;

static volatile unsigned long cur_bg_index;
static volatile unsigned long last_bg_time;

static unsigned long cur_bg_rate;
static unsigned long avg_bg_packet_long;
static unsigned long total_bg_rxtx_size;
static unsigned long total_bg_rxtx_count;

struct tag_aware_ctrl aware_net_ctrl = {
	.enable = 1,
	.mode = 1,
	.limit_rate = DEFAULT_LIMIT_RATE,
	.limit_ratio = LIMIT_RATIO_SIZE,
	.package_ratio = LIMIT_RATIO_SIZE,
};

struct network_info netinfo = {
	.fg_num = 0,
	.fg_uids = {0, },
	.bg_num = 0,
	.bg_uids = {0, },
	.fg_net_stat = {{0, }, },
	.bg_net_stat = {{0, }, },
};

struct bg_ctrl_policy_t {
	unsigned long package_count;
	unsigned int sleep_long;
};

static struct bg_ctrl_policy_t aware_net_bg_ctrl_policy[] = {
	{20, 1},
	{50, 10},
	{100, 25},
	{200, 10},
	{500, 25},
	{1000, 50},
	{1000, 100},
	{1000, 200},
	{1000, 300},
	{0x1fffffff, 100}
};

static bool is_bg_limit_enabled(void)
{
	bool ret = true;

	if (aware_net_ctrl.enable == 0)
		return false;
	spin_lock(&(netinfo.fg_lock));
	if (netinfo.fg_uids[0] < 0)
		ret = false;
	spin_unlock(&(netinfo.fg_lock));
	return ret;
}

static bool is_fg(int uid)
{
	int i;
	bool ret = false;

	spin_lock(&(netinfo.fg_lock));
	for (i = 0; i < netinfo.fg_num; i++) {
		if (uid == netinfo.fg_uids[i]) {
			ret = true;
			break;
		}
	}
	spin_unlock(&(netinfo.fg_lock));
	return ret;
}

static bool is_bg(int uid)
{
	int i;
	bool ret = false;

	spin_lock(&(netinfo.bg_lock));
	for (i = 0; i < netinfo.bg_num; i++) {
		if (uid == netinfo.bg_uids[i]) {
			ret = true;
			break;
		}
	}
	spin_unlock(&(netinfo.bg_lock));
	return ret;
}

static void updata_bg_data(unsigned long network_sum,
	unsigned long total_len_sum, unsigned long last_time,
	unsigned long oldest_time)
{
	total_bg_rxtx_count = network_sum;
	total_bg_rxtx_size = total_len_sum;

	avg_bg_packet_long = total_bg_rxtx_size / total_bg_rxtx_count;
	if (last_time >= oldest_time &&
		last_time - oldest_time < TIME_MAX_SIZE - 1)
		last_bg_time = (last_time - oldest_time + 1) *
			TIME_LONG_OF_HZ; /* time in ms */
	else
		return;
	cur_bg_rate = total_bg_rxtx_size / last_bg_time; /* rate in kbytes */
}

static void updata_network_data(const unsigned long *cur_time,
	unsigned long *network_sum, unsigned long *total_len_sum,
	unsigned long *oldest_time, unsigned long *last_time)
{
	int i;

	for (i = 0; i < MAX_FG_NET_STAT; i++) {
		struct net_stat_x *tmp_for_cnt = &netinfo.bg_net_stat[i];

		if (tmp_for_cnt->time + MAX_FG_NET_STAT < *cur_time)
			continue;

		*network_sum += tmp_for_cnt->read_count;
		*network_sum += tmp_for_cnt->write_count;
		*total_len_sum += tmp_for_cnt->total_read_len;
		*total_len_sum += tmp_for_cnt->total_write_len;
		if ((*oldest_time > tmp_for_cnt->time) &&
			(tmp_for_cnt->time != 0))
			*oldest_time = tmp_for_cnt->time;

		if (*last_time < tmp_for_cnt->time)
			*last_time = tmp_for_cnt->time;
	}
}

static void sleep_interruptible(unsigned int sleep_long)
{
	atomic_inc_unless_negative(&netinfo.bg_limit);
	msleep_interruptible(sleep_long);
	atomic_dec_if_positive(&netinfo.bg_limit);
	return;
}

/* if fg net read or write count is beyond RANGE_COUNT, limit bg network */
static void limit_bg(void)
{
	int i;
	unsigned long cur_time;
	unsigned long oldest_time;
	unsigned long network_sum = 0;
	unsigned int sleep_long = SLEEP_TIME_MS;
	unsigned long last_time = 0;
	unsigned long total_len_sum = 0;

	if (!is_bg_limit_enabled())
		return;

	cur_time = jiffies;
	/* mode 7: bg limite dynamically */
	if (aware_net_ctrl.mode == AWARE_NET_CTRL_MODE_FIRSE_LEVEL) {
		spin_lock(&(netinfo.fg_lock));
		for (i = 0; i < MAX_FG_NET_STAT; i++) {
			struct net_stat *tmp_for_cnt = &netinfo.fg_net_stat[i];

			if (tmp_for_cnt->time +
				MAX_FG_NET_STAT * PERIOD_TIME < cur_time)
				continue;

			network_sum += tmp_for_cnt->read_count;
			network_sum += tmp_for_cnt->write_count;
		}
		spin_unlock(&(netinfo.fg_lock));
		total_fg_rxtx_count = network_sum;
		if (total_fg_rxtx_count == 0)
			return;

		network_sum = 0;
		oldest_time = cur_time;
		spin_lock(&(netinfo.bg_lock));
		updata_network_data(&cur_time, &network_sum, &total_len_sum,
			&oldest_time, &last_time);
		spin_unlock(&(netinfo.bg_lock));
		if (network_sum < MIN_BG_PACKAGE_COUNT)
			return;

		updata_bg_data(network_sum, total_len_sum, last_time,
			oldest_time);

		if (cur_bg_rate < aware_net_ctrl.limit_rate)
			return;

		sleep_long =
			aware_net_bg_ctrl_policy
			[aware_net_ctrl.mode - 1].sleep_long;
	} else if (aware_net_ctrl.mode <= AWARE_NET_CTRL_MODE_SECOND_LEVEL) {
		sleep_long =
			aware_net_bg_ctrl_policy[aware_net_ctrl.mode - 1]
			.sleep_long;
	}
	sleep_interruptible(sleep_long);
}

static void verify_is_recving(bool is_recving, struct net_stat *p_fg_net_stat)
{
	if (is_recving) {
		p_fg_net_stat->read_count = 1;
		p_fg_net_stat->write_count = 0;
	} else {
		p_fg_net_stat->read_count = 0;
		p_fg_net_stat->write_count = 1;
	}
}

static void update_fg_net_stat(bool is_recving)
{
	struct net_stat *p_fg_net_stat = NULL;
	int i;
	unsigned long cur_time;
	unsigned long period;

	spin_lock(&(netinfo.fg_lock));
	cur_time = jiffies;

	/* first time, starts at 0 */
	if (last_fg_time == 0) {
		cur_fg_index = 0;
		p_fg_net_stat = &netinfo.fg_net_stat[0];
		p_fg_net_stat->time = cur_time;
		if (is_recving) {
			p_fg_net_stat->read_count = 1;
			p_fg_net_stat->write_count = 0;
		} else {
			p_fg_net_stat->read_count = 0;
			p_fg_net_stat->write_count = 1;
		}
		goto out;
	}
	period = (cur_time - last_fg_time) / PERIOD_TIME;
	if (period == 0) {
		/* update cur data */
		p_fg_net_stat = &netinfo.fg_net_stat[cur_fg_index];
		is_recving ? p_fg_net_stat->read_count++ :
			p_fg_net_stat->write_count++;
	} else {
		/* clear data during last time and cur time */
		if (period > MAX_FG_NET_STAT)
			period = MAX_FG_NET_STAT;
		for (i = 1; i < period; i++) {
			int tmp_index = (cur_fg_index + i) % MAX_FG_NET_STAT;

			p_fg_net_stat = &netinfo.fg_net_stat[tmp_index];
			p_fg_net_stat->read_count = 0;
			p_fg_net_stat->write_count = 0;
			p_fg_net_stat->time = 0;
		}
		/* update cur data */
		cur_fg_index = (cur_fg_index + period) % MAX_FG_NET_STAT;
		p_fg_net_stat = &netinfo.fg_net_stat[cur_fg_index];
		verify_is_recving(is_recving, p_fg_net_stat);
		p_fg_net_stat->time = cur_time;
	}

out:
	last_fg_time = cur_time;
	spin_unlock(&(netinfo.fg_lock));
	return;
}

void tcp_network_aware(bool is_recving)
{
	int cur_uid;

	if (aware_net_ctrl.enable == 0)
		return;

	cur_uid = current_uid().val;
	if (is_fg(cur_uid)) {
		update_fg_net_stat(is_recving);
		return;
	}

	if (is_bg(cur_uid))
		limit_bg();
}

void stat_bg_network_flow_x(bool is_recving, int len)
{
	unsigned long period;
	unsigned long cur_time;
	struct net_stat_x *temp_net_stat_x = NULL;

	/* first time, starts at 0 */
	spin_lock(&(netinfo.bg_lock));
	cur_time = jiffies;
	if (last_bg_time == 0)
		goto out;

	period = (cur_time - last_bg_time);
	if (period == 0) {
		/* update cur data */
		goto out_2;
	} else {
		int tmp_index;
		int i;
		/* clear data during last time and cur time */
		if (period > MAX_FG_NET_STAT)
			period = MAX_FG_NET_STAT;
		for (i = 1; i < period; i++) {
			tmp_index = (cur_bg_index + i) % MAX_FG_NET_STAT;
			temp_net_stat_x = &netinfo.bg_net_stat[tmp_index];
			memset(temp_net_stat_x, 0,
				sizeof(*temp_net_stat_x));
		}
		/* update cur data */
		cur_bg_index = (cur_bg_index + period) % MAX_FG_NET_STAT;
	}

out:
	last_bg_time = cur_time;
out_2:
	temp_net_stat_x = &netinfo.bg_net_stat[cur_bg_index];

	temp_net_stat_x->time = last_bg_time;
	if (is_recving) {
		temp_net_stat_x->read_count++;
		temp_net_stat_x->total_read_len += len;
	} else {
		temp_net_stat_x->write_count++;
		temp_net_stat_x->total_write_len += len;
	}
	spin_unlock(&(netinfo.bg_lock));
	return;
}

void stat_bg_network_flow(bool is_recving, int len)
{
	int cur_uid;

	cur_uid = current_uid().val;
	if (!is_bg(cur_uid))
		return;

	stat_bg_network_flow_x(is_recving, len);
}

void reinit_bg_stats(void)
{
	cur_bg_index = 0;
	last_bg_time = 0;
}


void reinit_fg_stats(void)
{
	last_fg_time = 0;
	cur_fg_index = 0;
}

void reinit_ctrl_policy(int limit_ratio, int package_ratio)
{
	int i;
	int count = ARRAY_SIZE(aware_net_bg_ctrl_policy);

	if (limit_ratio < 1)
		return;

	if (package_ratio < 1)
		return;

	for (i = 0; i < count; i++) {
		aware_net_bg_ctrl_policy[i].package_count =
			aware_net_bg_ctrl_policy[i].package_count *
			(package_ratio / aware_net_ctrl.package_ratio);
		if (aware_net_bg_ctrl_policy[i].package_count < MIN_BG_COUNT)
			aware_net_bg_ctrl_policy[i].package_count =
				MIN_BG_COUNT;
		else if (aware_net_bg_ctrl_policy[i].package_count >
			MAX_BG_COUNT)
			aware_net_bg_ctrl_policy[i].package_count =
			MAX_BG_COUNT;

		aware_net_ctrl.package_ratio = package_ratio;
		aware_net_bg_ctrl_policy[i].sleep_long =
			aware_net_bg_ctrl_policy[i].sleep_long *
			(limit_ratio / aware_net_ctrl.limit_ratio);

		if (aware_net_bg_ctrl_policy[i].sleep_long < MIN_BG_SLEEP)
			aware_net_bg_ctrl_policy[i].sleep_long = MIN_BG_SLEEP;
		else if (aware_net_bg_ctrl_policy[i].sleep_long >
			MAX_BG_SLEEP)
			aware_net_bg_ctrl_policy[i].sleep_long = MAX_BG_SLEEP;

		aware_net_ctrl.limit_ratio = limit_ratio;
	}
}

void dump_aware_net_stats(struct seq_file *m)
{
	unsigned long time = jiffies % (HZ * TIME_INTERVAL_INIT);
	int index;

	seq_printf(m, "fg_stats index:%lu  last_time:%lu\n",
		cur_fg_index, last_fg_time);

	seq_printf(m, "bg_stats rate:%lu  avg_long:%lu size:%lu count:(bg:%lu, fg:%lu) time_long:%lu\n",
		cur_bg_rate, avg_bg_packet_long, total_bg_rxtx_size,
		total_bg_rxtx_count, total_fg_rxtx_count, last_bg_time);

	/* print one  time in 5 */
	if (time > POSITION_INIT_NUM && time < (POSITION_INIT_NUM + HZ)) {
		for (index = 0; index < MAX_FG_NET_STAT; index++) {
			struct net_stat_x *tmp_for_cnt =
				&netinfo.bg_net_stat[index];

			if (tmp_for_cnt->time == 0)
				continue;
			seq_printf(m, "time:%lu rp:%lu rb:%lu tp:%lu tb:%lu\n",
				tmp_for_cnt->time, tmp_for_cnt->read_count,
				tmp_for_cnt->total_read_len,
				tmp_for_cnt->write_count,
				tmp_for_cnt->total_write_len);
		}
	}
}

static int fg_uids_show(struct seq_file *m, void *v)
{
	int i;

	for (i = 0; i < netinfo.fg_num; i++)
		seq_printf(m, "fg_uids[%d]: %d\n", i, netinfo.fg_uids[i]);

	return 0;
}

static int fg_uids_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, fg_uids_show, inode);
}

static ssize_t fg_uids_write(struct file *file, const char __user *buf,
	size_t count, loff_t *ppos)
{
	char buffer[MAX_ARRAY_LENGTH];
	char *cur = NULL;
	char *pos = NULL;
	int err = 0;
	int i = 0;

	memset(buffer, 0, sizeof(buffer));
	if (count > sizeof(buffer) - 1)
		count = sizeof(buffer) - 1;
	if (copy_from_user(buffer, buf, count)) {
		err = -EFAULT;
		goto out;
	}
	cur = buffer;

	spin_lock(&(netinfo.fg_lock));
	pos = strsep(&cur, ";");
	for (i = 0; i < MAX_UID_NUM; i++) {
		if (pos == NULL)
			break;
		netinfo.fg_uids[i] = simple_strtol(pos, NULL, 0);
		pos = strsep(&cur, ";");
	}
	netinfo.fg_num = i;
	for (i = 0; i < MAX_FG_NET_STAT; i++) {
		netinfo.fg_net_stat[i].read_count = 0;
		netinfo.fg_net_stat[i].write_count = 0;
		netinfo.fg_net_stat[i].time = 0;
	}

	reinit_fg_stats();

	spin_unlock(&(netinfo.fg_lock));

out:
	return err < 0 ? err : count;
}

static int bg_uids_show(struct seq_file *m, void *v)
{
	int i;

	for (i = 0; i < netinfo.bg_num; i++)
		seq_printf(m, "bg_uids[%d]: %d\n", i, netinfo.bg_uids[i]);

	seq_printf(m, "bg_is_limit:%d\n", atomic_read(&(netinfo.bg_limit)));
	return 0;
}

static int bg_uids_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, bg_uids_show, inode);
}

static ssize_t bg_uids_write(struct file *file, const char __user *buf,
	size_t count, loff_t *ppos)
{
	char buffer[MAX_ARRAY_LENGTH];
	char *cur = NULL;
	char *pos = NULL;

	int err = 0;
	int i = 0;

	memset(buffer, 0, sizeof(buffer));
	if (count > sizeof(buffer) - 1)
		count = sizeof(buffer) - 1;
	if (copy_from_user(buffer, buf, count)) {
		err = -EFAULT;
		goto out;
	}
	cur = buffer;

	spin_lock(&(netinfo.bg_lock));
	pos = strsep(&cur, ";");
	for (i = 0; i < MAX_UID_NUM; i++) {
		if (pos == NULL)
			break;
		netinfo.bg_uids[i] = simple_strtol(pos, NULL, 0);
		pos = strsep(&cur, ";");
	}
	netinfo.bg_num = i;
	memset(&netinfo.bg_net_stat, 0, sizeof(netinfo.bg_net_stat));
	reinit_bg_stats();
	spin_unlock(&(netinfo.bg_lock));

out:
	return err < 0 ? err : count;
}

static ssize_t aware_ctrl_write(struct file *file, const char __user *buf,
	size_t count, loff_t *ppos)
{
	int err = 0;
	char *cur = NULL;
	char *pos = NULL;
	char buffer[MAX_ARRAY_LENGTH];
	int new_limit_ratio = 100;
	int new_package_ratio = 100;

	memset(buffer, 0, sizeof(buffer));
	count = (count > sizeof(buffer) - 1) ? (sizeof(buffer) - 1) : count;

	if (copy_from_user(buffer, buf, count)) {
		err = -EFAULT;
		goto out;
	}

	cur = buffer;
	pos = strchr(cur, ':');
	if (pos == NULL)
		goto out;

	cur = pos + 1;
	aware_net_ctrl.mode = simple_strtol(cur, NULL, STRTOL_SIZE);
	aware_net_ctrl.enable = (aware_net_ctrl.mode == 0) ? 0 : 1;

	pos = strchr(cur, ':');
	if (pos == NULL) {
		aware_net_ctrl.limit_rate = DEFAULT_LIMIT_RATE;
		goto out;
	}

	cur = pos + 1;
	aware_net_ctrl.limit_rate = simple_strtol(cur, NULL, STRTOL_SIZE);
	if (aware_net_ctrl.limit_rate < 0)
		aware_net_ctrl.limit_rate = 0;

	pos = strchr(cur, ':');
	if (pos == NULL)
		goto out;

	cur = pos + 1;
	new_limit_ratio = simple_strtol(cur, NULL, STRTOL_SIZE);

	pos = strchr(cur, ':');
	if (pos == NULL)
		goto out_1;

	cur = pos + 1;
	new_package_ratio = simple_strtol(cur, NULL, STRTOL_SIZE);

out_1:
	reinit_ctrl_policy(new_limit_ratio, new_package_ratio);

out:
	return err < 0 ? err : count;
}

static int aware_ctrl_show(struct seq_file *m, void *v)
{
	seq_printf(m, "aware_ctrl enable:%d mode:%d limit_rate:%dk limit_ratio:%d package_ratio:%d\n",
		aware_net_ctrl.enable,
		aware_net_ctrl.mode,
		aware_net_ctrl.limit_rate,
		aware_net_ctrl.limit_ratio,
		aware_net_ctrl.package_ratio);
	dump_aware_net_stats(m);

	return 0;
}

static int aware_ctrl_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, aware_ctrl_show, inode);
}

static const struct file_operations proc_fg_uids_operations = {
	.open = fg_uids_open,
	.read = seq_read,
	.write = fg_uids_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations proc_bg_uids_operations = {
	.open = bg_uids_open,
	.read = seq_read,
	.write = bg_uids_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations proc_aware_ctrl_operations = {
	.open = aware_ctrl_open,
	.read = seq_read,
	.write = aware_ctrl_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static void __net_init aware_uids_proc_fs_init(struct proc_dir_entry *p_parent)
{
	struct proc_dir_entry *p_temp1 = NULL;
	struct proc_dir_entry *p_temp2 = NULL;
	struct proc_dir_entry *p_temp3 = NULL;

	if (p_parent == NULL)
		goto out_p_temp0;

	p_temp1 = proc_create(AWARE_FS_BG_UIDS, S_IRUSR | S_IWUSR, p_parent,
		&proc_bg_uids_operations);
	if (p_temp1 == NULL)
		goto out_p_temp0;

	p_temp2 = proc_create(AWARE_FS_FG_UIDS, S_IRUSR | S_IWUSR, p_parent,
		&proc_fg_uids_operations);
	if (p_temp2 == NULL)
		goto out_p_temp1;

	p_temp3 = proc_create(AWARE_FS_CTRL, S_IRUSR | S_IWUSR, p_parent,
		&proc_aware_ctrl_operations);
	if (p_temp3 == NULL)
		goto out_p_temp2;

	spin_lock_init(&(netinfo.fg_lock));
	spin_lock_init(&(netinfo.bg_lock));

	atomic_set(&netinfo.bg_limit, 0);

	return;
out_p_temp2:
	proc_remove(p_temp2);
out_p_temp1:
	proc_remove(p_temp1);
out_p_temp0:
	aware_net_ctrl.enable = 0;
	aware_net_ctrl.mode = 0;
	return;
}

static __net_init int aware_net_init(struct net *net)
{
	struct proc_dir_entry *p_parent = NULL;

	if (net == NULL)
		return 0;

	p_parent = proc_mkdir(AWARE_FS_NET_PATH, net->proc_net);
	if (p_parent == NULL)
		return -ENOMEM;

	aware_uids_proc_fs_init(p_parent);
	return 0;
}

static void aware_net_exit(struct net *net)
{
	if (net == NULL)
		return;

	remove_proc_entry(AWARE_FS_BG_UIDS, net->proc_net);
	remove_proc_entry(AWARE_FS_FG_UIDS, net->proc_net);
	remove_proc_entry(AWARE_FS_CTRL, net->proc_net);
	remove_proc_entry(AWARE_FS_NET_PATH, net->proc_net);
}

static struct pernet_operations aware_net_ops __net_initdata = {
	.init = aware_net_init,
	.exit = aware_net_exit,
};

static int __init aware_proc_init(void)
{
	return register_pernet_subsys(&aware_net_ops);
}

fs_initcall(aware_proc_init);

