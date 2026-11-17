/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: Provide bastet utilities.
 * Author: zhuweichen@huawei.com
 * Create: 2014-09-01
 */

#include <linux/sched.h>
#include <linux/fdtable.h>
#include <linux/file.h>
#include <linux/jiffies.h>
#include <linux/inetdevice.h>
#include <linux/of.h>
#include <linux/pm_wakeup.h>
#include <linux/fb.h>
#include <linux/mm.h>
#include <linux/timer.h>
#include <uapi/linux/if.h>
#include <linux/atomic.h>
#include <net/tcp.h>
#include <huawei_platform/net/bastet/bastet_utils.h>
#include <huawei_platform/net/bastet/bastet.h>
#include <huawei_platform/net/bastet/bastet_interface.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/mm.h>
#include <linux/sched/task.h>
#endif

#include <net/inet6_hashtables.h>

#define BASTET_DEFAULT_NET_DEV "rmnet0"

/* minimum uid number */
#define MIN_UID 0
/* maximum uid number */
#define MAX_UID 65535

#define SCREEN_ON 1
#define SCREEN_OFF 0

#define CHANNEL_OCCUPIED_TIMEOUT (5 * HZ)

#define DETECT_PEROID (1 * HZ)
#define PROHIBIT_PEROID (300 * HZ)
#define MAX_COUNT 10

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
#ifndef tcp_jiffies32
#define tcp_jiffies32 tcp_time_stamp
#endif
#endif

static void channel_occupied_timeout(unsigned long data);

static bool bastet_cfg_en;
static DEFINE_TIMER(channel_timer, channel_occupied_timeout, 0, 0);

bool bastet_dev_en;
char cur_netdev_name[IFNAMSIZ] = BASTET_DEFAULT_NET_DEV;
atomic_t proxy = ATOMIC_INIT(0);
atomic_t buffer = ATOMIC_INIT(0);
atomic_t channel = ATOMIC_INIT(0);
atomic_t cp_reset = ATOMIC_INIT(0);
#ifdef CONFIG_HUAWEI_BASTET_COMM_NEW
atomic_t reg_ccore_reset = ATOMIC_INIT(0);
#endif
/* set 1 for adjusting to non-wifi-proxy situation */
atomic_t channel_en = ATOMIC_INIT(1);
uid_t hrt_uid = -1;
int count = 0;
unsigned long detect_timestamp = 0;
unsigned long prohibit_timestamp = 0;

bool is_ipv6_addr(struct sock *sk)
{
#if IS_ENABLED(CONFIG_IPV6)
	if (sk == NULL) {
		bastet_logi("input null");
		return false;
	}

	if (sk->sk_family == AF_INET6 &&
		!(ipv6_addr_type(&sk->sk_v6_rcv_saddr) & IPV6_ADDR_MAPPED))
		return true;
#endif
	return false;
}

void print_bastet_sock_seq(struct bst_sock_sync_prop *prop)
{
	if (prop == NULL) {
		bastet_logi("input null");
		return;
	}

	if (!BASTET_DEBUG)
		return;

	bastet_logi("prop->seq=%u,prop->rcv_nxt=%u,prop->snd_wnd=%u,"
		"prop->mss=%u,prop->snd_wscale=%u,prop->rcv_wscale=%u",
		prop->seq, prop->rcv_nxt, prop->snd_wnd, prop->mss,
		prop->snd_wscale, prop->rcv_wscale);
	return;
}

inline bool is_bastet_enabled(void)
{
	return bastet_cfg_en && bastet_dev_en;
}

inline bool is_proxy_available(void)
{
	return atomic_read(&proxy) != 0;
}

inline bool is_buffer_available(void)
{
	return atomic_read(&buffer) != 0;
}

inline bool is_channel_available(void)
{
	return atomic_read(&channel) == 0;
}

inline bool is_cp_reset(void)
{
	return atomic_read(&cp_reset) != 0;
}

/* check priority channel enable or disable */
inline bool is_channel_enable(void)
{
	return atomic_read(&channel_en) != 0;
}

inline bool is_sock_foreground(struct sock *sk)
{
	return hrt_uid == sock_i_uid(sk).val;
}

/*
 * is_wifi_proxy() - check the proxy type.
 * @sk: struct sock pointer
 *
 * check whether socket is wifi network type
 *
 * Return: true, wifi network type
 *         false, not wifi network type
 */
inline bool is_wifi_proxy(struct sock *sk)
{
	return sk && sk->bastet && sk->bastet->is_wifi;
}

void set_channel_occupied(void)
{
	atomic_inc(&channel);
	mod_timer(&channel_timer, jiffies + CHANNEL_OCCUPIED_TIMEOUT);
}

void clear_channel_occupied(void)
{
	atomic_dec(&channel);
	if (atomic_read(&channel) == 0 && timer_pending(&channel_timer))
		del_timer(&channel_timer);
}

static void channel_occupied_timeout(unsigned long data)
{
	bastet_logi("set channel available");
	atomic_set(&channel, 0);
}

static void bastet_modem_reset_notify(void)
{
	if (!is_bastet_enabled())
		return;

	post_indicate_packet(BST_IND_MODEM_RESET, NULL, 0);
	atomic_set(&cp_reset, 1);
}

#ifdef CONFIG_HUAWEI_BASTET_COMM_NEW
static void reg_ccore_reset_notify(void)
{
	atomic_set(&reg_ccore_reset, 1);
}

static void unreg_ccore_reset_notify(void)
{
	atomic_set(&reg_ccore_reset, 0);
}

static bool is_reg_ccore_reset_notify(void)
{
	return atomic_read(&reg_ccore_reset) != 0;
}

/*
 * ind_modem_reset() - notify the modem to reset.
 * @value: reset command.
 * @len: commend length.
 *
 * parse the reset command, if equals to 0x1, the indication
 * the modem to reset. Otherwise, do nothing.
 *
 * Return: No
 */
void ind_modem_reset(uint8_t *value, uint32_t len)
{
	uint8_t reset_info;

	if (len != IPV4_ADDR_LENGTH) {
		bastet_loge("error msg len %u", len);
		return;
	}

	reset_info = value[0];

	switch (reset_info) {
	case 0x1:
		bastet_logi("ccore reset");
		if (is_reg_ccore_reset_notify()) {
			bastet_loge("bastet_modem_reset_notify");
			bastet_modem_reset_notify();
		}
		break;
	default:
		bastet_loge("error reset info value %u", reset_info);
		break;
	}
}
#endif

#if defined CONFIG_MSM_SUBSYSTEM_RESTART
#include <soc/qcom/subsystem_notif.h>

static void *subsys_h;

static int bastet_mss_reset_notifier_cb(struct notifier_block *this,
	unsigned long code, void *data)
{
	if (code == SUBSYS_AFTER_SHUTDOWN) {
		bastet_logi("SUBSYS_AFTER_SHUTDOWN");
		bastet_modem_reset_notify();
	}

	return 0;
}

static struct notifier_block mss_reset_notifier = {
	.notifier_call = bastet_mss_reset_notifier_cb,
};

static void reg_mss_reset_notify(void)
{
	bastet_logi("register msm mss reset notification");
	subsys_h = subsys_notif_register_notifier("modem", &mss_reset_notifier);
	if (IS_ERR(subsys_h))
		bastet_loge("failed to register for ssr rc: %d\n",
			(int)PTR_ERR(subsys_h));
}

static void unreg_mss_reset_notify(void)
{
	if (subsys_h != NULL)
		subsys_notif_unregister_notifier(subsys_h,
			&mss_reset_notifier);
}

#elif defined CONFIG_HUAWEI_BASTET_COMM_NEW
static void reg_mss_reset_notify(void)
{
	bastet_logi("register ccore reset notification");
	reg_ccore_reset_notify();
}

static void unreg_mss_reset_notify(void)
{
	bastet_logi("unregister ccore reset notification");
	unreg_ccore_reset_notify();
}

#elif defined CONFIG_BALONG_MODEM_RESET
#include <linux/hisi/reset.h>

#if defined(CONFIG_HISI_BALONG_MODEM_HI3XXX) || \
	defined(CONFIG_HISI_BALONG_MODEM_HI6XXX)
extern int ccorereset_regcbfunc(const char *pname,
	pdrv_reset_cbfun pcbfun, int userdata, int priolevel);
#elif defined CONFIG_HISI_BALONG_MODEM_HI3650
extern int bsp_reset_cb_func_register(const char *pname,
	pdrv_reset_cbfun pcbfun, int userdata, int priolevel);
#endif

static int bastet_ccorereset_cb(DRV_RESET_CB_MOMENT_E eparam, int userdata)
{
	if (eparam == MDRV_RESET_CB_BEFORE) {
		bastet_logi("MDRV_RESET_CB_BEFORE");
		bastet_modem_reset_notify();
	}

	return BSP_RESET_OK;
}

static void reg_mss_reset_notify(void)
{
	bastet_logi("register balong mss reset notification");
#if defined(CONFIG_HISI_BALONG_MODEM_HI3XXX) || \
	defined(CONFIG_HISI_BALONG_MODEM_HI6XXX)
	ccorereset_regcbfunc("BASTET", bastet_ccorereset_cb,
		0, BSP_DRV_CBFUN_PRIOLEVEL);
#elif defined CONFIG_HISI_BALONG_MODEM_HI3650
	bsp_reset_cb_func_register("BASTET", bastet_ccorereset_cb,
		0, BSP_DRV_CBFUN_PRIOLEVEL);
#endif
}

static void unreg_mss_reset_notify(void)
{
	bastet_logi("unreg_mss_reset_notify nothing to do");
}

#else
static void reg_mss_reset_notify(void)
{
	bastet_loge("mss reset notify is not supported");
}

static void unreg_mss_reset_notify(void)
{
	bastet_loge("mss reset notify is not supported");
}
#endif

static void post_screen_state_indicate(int state)
{
	int prohibit_state = SCREEN_OFF;

	if (!detect_timestamp)
		detect_timestamp = jiffies;

	if (prohibit_timestamp) {
		if (!time_after(jiffies, prohibit_timestamp + PROHIBIT_PEROID))
			return;
		else
			prohibit_timestamp = 0;
	}

	post_indicate_packet(BST_IND_SCREEN_STATE, &state, sizeof(state));
	count++;
	if (count == MAX_COUNT) {
		if (!time_after(jiffies, detect_timestamp + DETECT_PEROID)) {
			prohibit_timestamp = jiffies;
			post_indicate_packet(BST_IND_SCREEN_STATE,
				&prohibit_state, sizeof(prohibit_state));
			bastet_logi("screen state change is prohibit now");
		}
	}

	if (count > MAX_COUNT) {
		count = 0;
		detect_timestamp = jiffies;
	}
}

static int bastet_fb_event(struct notifier_block *self,
	unsigned long event, void *data)
{
	struct fb_event *fb_event = NULL;
	int *blank = NULL;
	int state = SCREEN_OFF;

	if (data == NULL)
		return NOTIFY_DONE;

	fb_event = (struct fb_event *)data;
	if (fb_event->data == NULL)
		return NOTIFY_DONE;

	blank = (int *)fb_event->data;
	if (event == FB_EVENT_BLANK) {
		switch (*blank) {
		case FB_BLANK_UNBLANK:
			state = SCREEN_ON;
			post_screen_state_indicate(state);
			break;
		case FB_BLANK_POWERDOWN:
			state = SCREEN_OFF;
			post_screen_state_indicate(state);
			break;
		default:
			break;
		}
	}
	return NOTIFY_DONE;
}

static struct notifier_block bastet_fb_notifier = {
	.notifier_call = bastet_fb_event,
};

static void init_fb_notification(void)
{
	fb_register_client(&bastet_fb_notifier);
}

static void deinit_fb_notification(void)
{
	fb_unregister_client(&bastet_fb_notifier);
}

static struct file *fget_by_pid(unsigned int fd, pid_t pid)
{
	struct file *file = NULL;
	struct task_struct *task = NULL;
	struct files_struct *files = NULL;

	rcu_read_lock();
	task = find_task_by_vpid(pid);
	if (!task) {
		rcu_read_unlock();
		return NULL;
	}
	get_task_struct(task);
	rcu_read_unlock();
	files = task->files;

	/* process is removed, task isn't null, but files is null */
	if (files == NULL) {
		put_task_struct(task);
		return NULL;
	}
	put_task_struct(task);
	rcu_read_lock();
	file = fcheck_files(files, fd);
	if (file) {
		/* File object ref couldn't be taken */
		if ((file->f_mode & FMODE_PATH) ||
			!atomic_long_inc_not_zero(&file->f_count))
			file = NULL;
	}
	rcu_read_unlock();

	return file;
}

struct socket *sockfd_lookup_by_fd_pid(int fd, pid_t pid, int *err)
{
	struct file *file = NULL;
	struct socket *sock = NULL;

	file = fget_by_pid(fd, pid);
	if (!file) {
		*err = -EBADF;
		return NULL;
	}

	sock = sock_from_file(file, err);
	if (!sock)
		fput(file);

	return sock;
}

/*
 * get_sock_by_fd_pid() - get the sock.
 * @fd: file descriptor.
 * @pid: process id.
 *
 * invoke the sockfd_lookup_by_fd_pid() to get the sock.
 *
 * Return: sock, success.
 * NULL means fail.
 */
struct sock *get_sock_by_fd_pid(int fd, pid_t pid)
{
	int err = 0;
	struct socket *sock = NULL;
	struct sock *sk = NULL;

	sock = sockfd_lookup_by_fd_pid(fd, pid, &err);
	if (sock == NULL)
		return NULL;

	sk = sock->sk;
	if (sk != NULL)
		sock_hold(sk);

	if (sock->file != NULL)
		sockfd_put(sock);

	return sk;
}

void print_abnormal_infomation_for_find_fd(int count)
{
	if (count == 0)
		bastet_loge("No socket with given address exist");
	else
		bastet_loge("More than one socket with same address exist");
}

/*
 * get_fd_by_addr() - get the fd.
 * @guide: addr stru, include the ip and fd.
 *
 * check the file and task, iterator the fd tables.
 * to find the fd.
 *
 * Return: 0 success.
 * Otherwise is fail.
 */
int get_fd_by_addr(struct addr_to_fd *guide)
{
	struct task_struct *task = NULL;
	struct fdtable *fdt = NULL;
	struct sock *sk = NULL;
	struct inet_sock *inet = NULL;
	unsigned int i;
	int count = 0;
	int fd = -1;

	if (guide == NULL)
		return -EFAULT;
	rcu_read_lock();
	task = find_task_by_vpid(guide->pid);
	if (!task) {
		rcu_read_unlock();
		return -EFAULT;
	}
	get_task_struct(task);
	rcu_read_unlock();

	/* process is removed, task isn't null, but files is null */
	if (task->files == NULL) {
		put_task_struct(task);
		return -EFAULT;
	}

	fdt = files_fdtable(task->files);
	for (i = 0; i <= fdt->max_fds; i++) {
		sk = get_sock_by_fd_pid(i, guide->pid);
		if (!sk)
			continue;

		inet = inet_sk(sk);
		if (!inet) {
			sock_put(sk);
			continue;
		}
		if (inet->inet_daddr == guide->remote_ip
			&& inet->inet_dport == guide->remote_port) {
			count++;
			fd = i;
		}
		sock_put(sk);
	}
	put_task_struct(task);
	if (count == 1) {
		guide->fd = fd;
		bastet_loge("fd=%d", fd);
	} else {
		print_abnormal_infomation_for_find_fd(count);
	}

	return 0;
}

/*
 * get_pid_cmdline() - get the fd.
 * @cmdline: command line data.
 *
 * get the task, then get the task mm,
 * use these two parameters to invoke the access_process_vm()
 *
 * Return: 0 success.
 * Otherwise is fail.
 */
int get_pid_cmdline(struct get_cmdline *cmdline)
{
	struct task_struct *task = NULL;
	struct mm_struct *mm = NULL;
	char buffer[MAX_PID_NAME_LEN];
	int res;
	unsigned int len;

	if (cmdline == NULL)
		return -1;
	rcu_read_lock();
	task = find_task_by_vpid(cmdline->pid);
	if (!task) {
		rcu_read_unlock();
		return -1;
	}
	get_task_struct(task);
	rcu_read_unlock();
	mm = get_task_mm(task);
	if (!mm)
		goto out;
	if (!mm->arg_end)
		goto out_mm; /* Shh! No looking before we're done */

	len = mm->arg_end - mm->arg_start;

	if (len > MAX_PID_NAME_LEN)
		len = MAX_PID_NAME_LEN;

	memset(buffer, '\0', MAX_PID_NAME_LEN);
	res = access_process_vm(task, mm->arg_start, buffer, len, 0);
	/*
	 * If the nul at the end of args has been overwritten, then
	 * assume application is using setproctitle(3).
	 */
	if (res > 0 && buffer[res - 1] != '\0' && len < MAX_PID_NAME_LEN) {
		len = strnlen(buffer, res);
		if (len < res) {
			res = len;
		} else {
			len = mm->env_end - mm->env_start;
			if (len > (unsigned int)(MAX_PID_NAME_LEN - res))
				len = (unsigned int)(MAX_PID_NAME_LEN - res);
			res += access_process_vm(task,
				mm->env_start, buffer + res, len, 0);
			res = strnlen(buffer, res);
		}
	}
	if (res > 0 && res < MAX_PID_NAME_LEN)
		strncpy(cmdline->name, buffer, res);
out_mm:
	mmput(mm);
out:
	put_task_struct(task);
	return 0;
}

/*
 * bastet_get_comm_prop() - get the comm prop.
 * @sk: point to the sock data.
 * @prop: comm prop data pointer.
 *
 * check the sk and prop,if not null, assign the information
 * from inet_sock to prop
 *
 * Return: 0 success.
 * Otherwise is fail.
 */
int bastet_get_comm_prop(struct sock *sk,
	struct bst_sock_comm_prop *prop)
{
	struct inet_sock *inet = inet_sk(sk);

	if (inet == NULL || prop == NULL) {
		bastet_loge("invalid patameter");
		return -1;
	}
	prop->local_ip = inet->inet_saddr;
	prop->local_port = inet->inet_sport;
	prop->remote_ip = inet->inet_daddr;
	prop->remote_port = inet->inet_dport;
	return 0;
}

int bastet_get_comm_prop_ipv6(struct sock *sk,
	struct bst_sock_comm_prop_ipv6 *prop)
{
	struct inet_sock *inet = inet_sk(sk);

	if (inet == NULL || prop == NULL) {
		bastet_loge("invalid patameter");
		return -1;
	}
#if IS_ENABLED(CONFIG_IPV6)
	memcpy((void*)prop->local_ip,
		(const void*)sk->sk_v6_rcv_saddr.s6_addr, IPV6_ADDR_LENGTH);
	memcpy((void*)prop->remote_ip,
		(const void*)sk->sk_v6_daddr.s6_addr, IPV6_ADDR_LENGTH);
#else
	memset((void*)prop->local_ip, 0, IPV6_ADDR_LENGTH);
	memset((void*)prop->remote_ip, 0, IPV6_ADDR_LENGTH);
#endif
	prop->local_port = inet->inet_sport;
	prop->remote_port = inet->inet_dport;

	return 0;
}

/*
 * bastet_get_sock_prop() - get the sync sock prop.
 * @sk: point to the sock.
 * @prop: sync prop data pointer.
 *
 * check the sk and prop,if not null, assign the information
 * from tcp_sock to prop
 *
 * Return: 0 success.
 * Otherwise is fail.
 */
void bastet_get_sock_prop(struct sock *sk, struct bst_sock_sync_prop *prop)
{
	struct tcp_sock *tp = tcp_sk(sk);

	if (IS_ERR_OR_NULL(tp) || IS_ERR_OR_NULL(prop)) {
		bastet_loge("invalid patameter");
		return;
	}

	prop->seq = tp->write_seq;
	prop->rcv_nxt = tp->rcv_nxt;
	prop->snd_wnd = tp->snd_wnd;
	prop->mss = tp->advmss;
	prop->snd_wscale = tp->rx_opt.snd_wscale;
	prop->rcv_wscale = tp->rx_opt.rcv_wscale;
	prop->tx = 0;
	prop->rx = 0;

	if (likely(tp->rx_opt.tstamp_ok)) {
		tcp_mstamp_refresh(tp);
		prop->ts_current = tcp_time_stamp(tp) + tp->tsoffset;
		prop->ts_recent = tp->rx_opt.ts_recent;
		prop->ts_recent_tick = 0;
	}

	bastet_logi("prop->seq=%u,prop->rcv_nxt=%u,prop->snd_wnd=%u,"
		"prop->mss=%u,prop->snd_wscale=%u,prop->rcv_wscale=%u",
		prop->seq, prop->rcv_nxt, prop->snd_wnd, prop->mss,
		prop->snd_wscale, prop->rcv_wscale);
}

/*
 * get_sock_by_comm_prop() - get the sock from prop.
 * @guide: point to the comm prop data.
 *
 * Get sock by ip and port.
 *
 * Return: sock data, success
 * NULL, fail.
 */
struct sock *get_sock_by_comm_prop(struct bst_sock_comm_prop *guide)
{
	struct net *net = NULL;
	struct net_device *dev = NULL;

	dev = dev_get_by_name(&init_net, cur_netdev_name);
	if (dev == NULL || guide == NULL)
		return NULL;

	net = dev_net(dev);

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
	return __inet_lookup_established(net,
		&tcp_hashinfo, guide->remote_ip, guide->remote_port,
		guide->local_ip, ntohs(guide->local_port), dev->ifindex, 0);
#else
	return __inet_lookup_established(net,
		&tcp_hashinfo, guide->remote_ip, guide->remote_port,
		guide->local_ip, ntohs(guide->local_port), dev->ifindex);
#endif
}

struct sock *get_sock_by_comm_prop_ipv6(
	struct bst_sock_comm_prop_ipv6 *guide)
{
	struct net *net = NULL;
	struct net_device *dev = NULL;

	dev = dev_get_by_name(&init_net, cur_netdev_name);
	if (dev == NULL || guide == NULL)
		return NULL;

	net = dev_net(dev);
#if IS_ENABLED(CONFIG_IPV6)
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
	return __inet6_lookup_established(net, &tcp_hashinfo,
		(struct in6_addr *)guide->remote_ip, guide->remote_port,
		(struct in6_addr *)guide->local_ip, ntohs(guide->local_port),
		dev->ifindex, 0);
#else
	return __inet6_lookup_established(net, &tcp_hashinfo,
		(struct in6_addr *)guide->remote_ip, guide->remote_port,
		(struct in6_addr *)guide->local_ip, ntohs(guide->local_port),
		dev->ifindex);
#endif
#endif
	return NULL;
}

void ind_hisi_com(const void *info, u32 len)
{
	post_indicate_packet(BST_IND_HISICOM, info, len);
}
inline bool is_uid_valid(__u32 uid)
{
	return uid >= MIN_UID && uid <= MAX_UID;
}

int set_current_net_device_name(const char *iface)
{
	if (iface == NULL)
		return -EINVAL;

	memcpy(cur_netdev_name, iface, IFNAMSIZ);
	cur_netdev_name[IFNAMSIZ - 1] = '\0';
	return 0;
}

/*
 * get_uid_by_pid() - get uid.
 * @info: pid_to_uid pid and uid
 *
 * get Uid by Pid, get it from task->real_cred->uid.val
 *
 * Return:0 is success
 * Otherwise is fail.
 */
int get_uid_by_pid(struct set_process_info *info)
{
	struct task_struct *task = NULL;

	if (info == NULL) {
		bastet_loge("invalid parameter");
		return -EFAULT;
	}
	rcu_read_lock();
	task = find_task_by_vpid(info->pid);
	if (!task) {
		bastet_loge("find task by pid failed");
		rcu_read_unlock();
		return -EFAULT;
	}
	get_task_struct(task);
	rcu_read_unlock();
	if (!(task->real_cred)) {
		bastet_loge("task->real_cred is NULL");
		put_task_struct(task);
		return -EFAULT;
	}
	info->uid = (int32_t)task->real_cred->uid.val;
	bastet_logi("uid=%d", info->uid);
	put_task_struct(task);
	return 0;
}

/*
 * get_sock_net_dev_name() - get the dev name.
 * @dev_name: pid and fd to find sock
 *
 * get the network device name by invoke the get_sock_by_fd_pid.
 *
 * Return: 0, success
 *         negative, failed
 */
int get_sock_net_dev_name(struct get_netdev_name *dev_name)
{
	int ret = 0;
	struct sock *sk = NULL;
	struct dst_entry *dst = NULL;

	if (dev_name == NULL) {
		bastet_loge("dev_name is null");
		return -EINVAL;
	}
	sk = get_sock_by_fd_pid(dev_name->guide.fd, dev_name->guide.pid);
	if (sk == NULL) {
		bastet_loge("can not find sock by fd: %d, pid: %d",
			dev_name->guide.fd, dev_name->guide.pid);
		return -ENOENT;
	}
	/* struct net_device is in struct dst_entry */
	dst = __sk_dst_get(sk);
	if (dst && dst->dev) {
		bastet_logi("net device name: %s", dst->dev->name);
		memcpy(dev_name->netdev_name, dst->dev->name, IFNAMSIZ);
	} else {
		bastet_loge("failed to get net device name");
		ret = -ENOENT;
	}
	sock_put(sk);

	return ret;
}

/*
 * bastet_set_freezer() - set the freezer
 * @freezer: process freeze state
 *
 * use atomic_set to set the sk->bastet->hbm.frozen flags.
 *
 * Return: 0, success
 *         negative, failed
 */
int bastet_set_freezer(struct freezer_state freezer)
{
	struct sock *sk = NULL;

	sk = get_sock_by_fd_pid(freezer.guide.fd, freezer.guide.pid);
	if (sk == NULL) {
		bastet_loge("can not find sock by fd: %d, pid: %d",
			freezer.guide.fd, freezer.guide.pid);
		return -ENOENT;
	}
	if (is_wifi_proxy(sk)) {
		bastet_logi("frozen: %d", freezer.frozen);
		atomic_set(&sk->bastet->hbm.frozen, freezer.frozen ? 1 : 0);
	}
	sock_put(sk);

	return 0;
}

void bastet_utils_init(void)
{
	bastet_logi("bastet feature enabled");
	bastet_cfg_en = true;
	reg_mss_reset_notify();
	init_fb_notification();
}

void bastet_utils_exit(void)
{
	unreg_mss_reset_notify();
	deinit_fb_notification();
}
