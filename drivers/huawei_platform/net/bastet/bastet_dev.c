/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014. All rights reserved.
 * Description: Bastet Driver Device.
 *
 * Author: zhuweichen@huawei.com
 * Create: 2014-06-21
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif
#include <huawei_platform/net/bastet/bastet.h>
#include <huawei_platform/net/bastet/bastet_utils.h>
#include <huawei_platform/net/bastet/bastet_interface.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/task.h>
#endif

#define BASTET_NAME						"bastet"

#define BST_FIRST_MINOR					0
#define BST_DEVICES_NUMBER				1
#define BST_MAX_REPLY_LEN				1024
/* Current Max Traffic report task number is 32 */
#define BST_TRAFFIC_LEN(len)			(len > 0XFF ? 0XFF : len)
#define BST_MAX_PROXY_NUM               32
#define APP_MAX_PID_NUM                 500
#define FIXED_CTNT_MAX_LEN              (1024)
#ifdef CONFIG_HUAWEI_BASTET_COMM
#define BST_ACORE_CORE_MSG_TYPE_DSPP    0
#endif
#define BST_MAX_INDICATE_PACKET_LEN  (5000)
dev_t bastet_dev;
struct cdev bastet_cdev;
struct class *bastet_class;

struct data_packet {
	struct list_head list;
	struct bst_device_ind data;
};

struct bastet_driver_data {
	wait_queue_head_t read_wait;
	spinlock_t read_lock;
	struct list_head read_queue;
};

static struct bastet_driver_data bastet_data;

struct bst_ind_type_name {
	enum bst_ind_type type;
	const char *name;
};

static struct bst_ind_type_name ind_type_name[] = {
	{ BST_IND_INVALID, "BST_IND_INVALID" },
	{ BST_IND_HISICOM, "BST_IND_HISICOM" },
	{ BST_IND_SOCK_SYNC_REQ, "BST_IND_SOCK_SYNC_REQ" },
	{ BST_IND_SOCK_SYNC_PROP, "BST_IND_SOCK_SYNC_PROP" },
	{ BST_IND_SOCK_CLOSED, "BST_IND_SOCK_CLOSED" },
	{ BST_IND_MODEM_RESET, "BST_IND_MODEM_RESET" },
	{ BST_IND_NETFILTER_SYNC_UID, "BST_IND_NETFILTER_SYNC_UID" },
	{ BST_IND_TRAFFIC_FLOW_REQ, "BST_IND_TRAFFIC_FLOW_REQ" },
	{ BST_IND_PRIORITY_DATA, "BST_IND_PRIORITY_DATA" },
	{ BST_IND_SOCK_DISCONNECT, "BST_IND_SOCK_DISCONNECT" },
	{ BST_IND_SOCK_TIMEDOUT, "BST_IND_SOCK_TIMEDOUT" },
	{ BST_IND_SOCK_EST, "BST_IND_SOCK_EST" },
	{ BST_IND_SOCK_RENEW_CLOSE, "BST_IND_SOCK_RENEW_CLOSE" },
	{ BST_IND_SOCK_NORMAL_CLOSE, "BST_IND_SOCK_NORMAL_CLOSE" },
	{ BST_IND_SOCK_ERR_CLOSE, "BST_IND_SOCK_ERR_CLOSE" },
	{ BST_IND_PRIO_SOCK_CLOSE, "BST_IND_PRIO_SOCK_CLOSE" },
	{ BST_IND_SCREEN_STATE, "BST_IND_SCREEN_STATE" },
	{ BST_IND_UID_SOCK_PROP, "BST_IND_UID_SOCK_PROP" },
	{ BST_IND_TRIGGER_THAW, "BST_IND_TRIGGER_THAW" },
	{ BST_IND_SOCK_STATE_CHANGED, "BST_IND_SOCK_STATE_CHANGED" },
	{ BST_IND_PRIORITY_UID, "BST_IND_PRIORITY_UID" },
	{ BST_IND_FG_KEY_MSG, "BST_IND_FG_KEY_MSG" },
	{ BST_IND_FG_UID_SOCK_CHG, "BST_IND_FG_UID_SOCK_CHG" },
	{ BST_IND_HB_REPLY_RECV, "BST_IND_HB_REPLY_RECV" },
	{ BST_IND_RRC_KEEP, "BST_IND_RRC_KEEP" },
	{ BST_IND_RCVQUEUE_FULL, "BST_IND_RCVQUEUE_FULL" },
	{ BST_IND_SKSTATE_NOT_UPDATING, "BST_IND_SKSTATE_NOT_UPDATING" },
	{ BST_IND_SOCK_SYNC_FAILED, "BST_IND_SOCK_SYNC_FAILED" },
	{ BST_IND_GET_SK_FAILED, "BST_IND_GET_SK_FAILED" },
	{ BST_IND_GET_BSK_FAILED, "BST_IND_GET_BSK_FAILED" },
	{ BST_IND_PENDING_SK_SET, "BST_IND_PENDING_SK_SET" },
	{ BST_IND_NOPENDING_SK_SET, "BST_IND_NOPENDING_SK_SET" },
	{ BST_IND_SOCK_STATE_WAIT, "BST_IND_SOCK_STATE_WAIT" },
	{ BST_IND_SEND_DATA_NOTACK, "BST_IND_SEND_DATA_NOTACK" },
	{ BST_IND_RECV_DATA_INQUEUE, "BST_IND_RECV_DATA_INQUEUE" },
	{ BST_IND_SOCK_SYNC_PROP_IPV6, "BST_IND_SOCK_SYNC_PROP_IPV6" },
	{ BST_IND_SOCK_CLOSED_IPV6, "BST_IND_SOCK_CLOSED_IPV6" },
	{ BST_IND_SOCK_SYNC_REQ_IPV6, "BST_IND_SOCK_SYNC_REQ_IPV6" },
	{ BST_IND_PRIORITY_DATA_IPV6, "BST_IND_PRIORITY_DATA_IPV6" },
	{ BST_IND_SOCK_DISCONNECT_IPV6, "BST_IND_SOCK_DISCONNECT_IPV6" },
	{ BST_IND_SOCK_TIMEDOUT_IPV6, "BST_IND_SOCK_TIMEDOUT_IPV6" },
	{ BST_IND_SOCK_EST_IPV6, "BST_IND_SOCK_EST_IPV6" },
	{ BST_IND_SOCK_NORMAL_CLOSE_IPV6, "BST_IND_SOCK_NORMAL_CLOSE_IPV6" },
	{ BST_IND_SOCK_ERR_CLOSE_IPV6, "BST_IND_SOCK_ERR_CLOSE_IPV6" },
	{ BST_IND_PRIO_SOCK_CLOSE_IPV6, "BST_IND_PRIO_SOCK_CLOSE_IPV6" },
	{ BST_IND_UID_SOCK_PROP_IPV6, "BST_IND_UID_SOCK_PROP_IPV6" },
	{ BST_IND_SN_INVALID, "BST_IND_SN_INVALID" },
};

static int handle_bst_ioc_sock_sync_start(void __user *argp);
static int handle_bst_ioc_sock_sync_stop(void __user *argp);
static int handle_bst_ioc_sock_sync_set(void __user *argp);
static int handle_bst_ioc_sock_sync_set_ipv6(void __user *argp);
static int handle_bst_ioc_sock_comm_get(void __user *argp);
static int handle_bst_ioc_sock_comm_get_ipv6(void __user *argp);
static int handle_bst_ioc_sock_close_set(void __user *argp);
static int handle_bst_ioc_sock_close_set_ipv6(void __user *argp);
static int handle_bst_ioc_sock_state_get(void __user *argp);
static int handle_bst_ioc_apply_local_port(void __user *argp);
static int handle_bst_ioc_release_local_port(void __user *argp);
static int handle_bst_ioc_set_traffic_flow(void __user *argp);
static int handle_bst_ioc_get_timestamp_info(void __user *argp);
static int handle_bst_ioc_set_net_dev_name(void __user *argp);
static int handle_bst_ioc_get_modem_rab_id(void __user *argp);
static int handle_bst_ioc_get_ipv6_modem_rab_id(void __user *argp);
static int handle_bst_ioc_sock_reconn_set(void __user *argp);
static int handle_bst_ioc_sock_reconn_fail(void __user *argp);
static int handle_bst_ioc_set_reconn_proxyid(void __user *argp);
static int handle_bst_ioc_sync_prop_start(void __user *argp);
static int handle_bst_ioc_sync_prop_start_ipv6(void __user *argp);
static int handle_bst_ioc_sync_prop_stop(void __user *argp);
static int handle_bst_ioc_sync_prop_stop_ipv6(void __user *argp);
static int handle_bst_ioc_proxy_available(void __user *argp);
static int handle_bst_ioc_buf_available(void __user *argp);
static int handle_bst_ioc_prio_send_suc(void __user *argp);
static int handle_bst_ioc_sock_sync_prepare(void __user *argp);
static int handle_bst_ioc_get_fd_by_addr(void __user *argp);
static int handle_bst_ioc_get_cmdline(void __user *argp);
static int handle_bst_ioc_sync_hold_time(void __user *argp);
static int handle_bst_ioc_net_dev_reset(void __user *argp);
static int handle_bst_ioc_uid_prop_monitor(void __user *argp);
static int handle_bst_ioc_set_hrtapp_activity(void __user *argp);
static int handle_bst_ioc_set_proc_info(void __user *argp);
static int handle_bst_ioc_set_special_uid(void __user *argp);
static int handle_bst_ioc_nf_control(void __user *argp);
static int handle_bst_ioc_get_sock_netdev_name(void __user *argp);
static int handle_bst_ioc_send_hb_data(void __user *argp);
static int handle_bst_ioc_set_hb_reply(void __user *argp);
static int handle_bst_ioc_filter_hb_reply(void __user *argp);
static int handle_bst_ioc_set_freezer_state(void __user *argp);
static int handle_bst_ioc_set_prio_ch_enable(void __user *argp);
static int handle_bst_ioc_fg_io_ctrl(void __user *argp);

typedef int (*bst_ioc_fp)(void __user *argp);

typedef struct {
	int bst_ioc_id;
	bst_ioc_fp bst_ioc_handle;
	const char *id_name;
} bst_ioc_cmd;

static const bst_ioc_cmd bst_ioc_cmd_tb[] = {
#include <huawei_platform/net/bastet/bastet_ioctl_handler.h>
};

/**
 * post_indicate_packet() - post_indicate_packet
 * @bst_ind_type: bastet indication type.
 * @info: the pointer to the packet
 * @len: the length of packet.
 *
 * check the bastet device status. if ok then copy
 * data to pkt, add it to the list.
 *
 * Return: 0 - indicate ok.
 *         ENIENT bastet not opened or len is too long
 *         ENOMEM kmalloc is fail.
 */
int post_indicate_packet(enum bst_ind_type type,
	const void *info, unsigned int len)
{
	struct data_packet *pkt = NULL;

	if (!bastet_dev_en) {
		bastet_loge("bastet is not opened");
		return -ENOENT;
	}
	if (len > BST_MAX_INDICATE_PACKET_LEN) {
		bastet_loge("len is too long");
		return -ENOENT;
	}
	pkt = kmalloc(sizeof(*pkt) + len, GFP_ATOMIC);
	if (pkt == NULL) {
		bastet_loge("failed to kmalloc");
		return -ENOMEM;
	}
	memset(pkt, 0, sizeof(*pkt) + len);

	pkt->data.cons = 0;
	pkt->data.type = type;
	pkt->data.len = len;
	if (info != NULL)
		memcpy(pkt->data.value, info, len);

	if (type >= BST_IND_INVALID && type <= BST_IND_UID_SOCK_PROP_IPV6)
		bastet_logi("bst_ind_type:%s", ind_type_name[type].name);

	spin_lock_bh(&bastet_data.read_lock);
	list_add_tail(&pkt->list, &bastet_data.read_queue);
	spin_unlock_bh(&bastet_data.read_lock);

	wake_up_interruptible_sync_poll(&bastet_data.read_wait,
		POLLIN | POLLRDNORM | POLLRDBAND);

	return 0;
}

static int handle_bst_ioc_sock_sync_start(void __user *argp)
{
	int rc = -EFAULT;
	struct bst_set_sock_sync_delay init_p;

	if (copy_from_user(&init_p, argp, sizeof(init_p)))
		return rc;

	rc = start_bastet_sock(&init_p);
	return rc;
}

static int handle_bst_ioc_sock_sync_stop(void __user *argp)
{
	int rc = -EFAULT;
	struct bst_sock_id guide;

	if (copy_from_user(&guide, argp, sizeof(guide)))
		return rc;

	rc = stop_bastet_sock(&guide);
	return rc;
}

static int handle_bst_ioc_sock_sync_set(void __user *argp)
{
	int rc = -EFAULT;
	struct bst_set_sock_sync_prop set_p;

	if (copy_from_user(&set_p, argp, sizeof(set_p)))
		return rc;

	rc = set_tcp_sock_sync_prop(&set_p);
	return rc;
}

static int handle_bst_ioc_sock_sync_set_ipv6(void __user *argp)
{
	int rc = -EFAULT;
	struct bst_set_sock_sync_prop_ipv6 set_p;

	if (copy_from_user(&set_p, argp, sizeof(set_p)))
		return rc;

	rc = set_tcp_sock_sync_prop_ipv6(&set_p);
	return rc;
}

static int handle_bst_ioc_sock_comm_get(void __user *argp)
{
	int rc = -EFAULT;
	struct bst_get_sock_comm_prop get_p;

	if (copy_from_user(&get_p, argp, sizeof(get_p)))
		return rc;

	rc = get_tcp_sock_comm_prop(&get_p);
	if (rc < 0)
		return rc;

	if (copy_to_user(argp, &get_p, sizeof(get_p)))
		rc = -EFAULT;

	return rc;
}

static int handle_bst_ioc_sock_comm_get_ipv6(void __user *argp)
{
	int rc = -EFAULT;
	struct bst_get_sock_comm_prop_ipv6 get_p;

	if (copy_from_user(&get_p, argp, sizeof(get_p)))
		return rc;

	rc = get_tcp_sock_comm_prop_ipv6(&get_p);
	if (rc < 0)
		return rc;

	if (copy_to_user(argp, &get_p, sizeof(get_p)))
		rc = -EFAULT;

	return rc;
}

static int handle_bst_ioc_sock_close_set(void __user *argp)
{
	int rc = -EFAULT;
	struct bst_sock_comm_prop guide;

	if (copy_from_user(&guide, argp, sizeof(guide)))
		return rc;

	rc = set_tcp_sock_closed(&guide);

	return rc;
}

static int handle_bst_ioc_sock_close_set_ipv6(void __user *argp)
{
	int rc = -EFAULT;
	struct bst_sock_comm_prop_ipv6 guide;

	if (copy_from_user(&guide, argp, sizeof(guide)))
		return rc;

	rc = set_tcp_sock_closed_ipv6(&guide);

	return rc;
}

static int handle_bst_ioc_sock_state_get(void __user *argp)
{
	int rc = -EFAULT;
	struct bst_get_bastet_sock_state get_p;

	if (copy_from_user(&get_p, argp, sizeof(get_p)))
		return rc;

	rc = get_tcp_bastet_sock_state(&get_p);
	if (rc < 0)
		return rc;

	if (copy_to_user(argp, &get_p, sizeof(get_p)))
		rc = -EFAULT;

	return rc;
}

static int handle_bst_ioc_apply_local_port(void __user *argp)
{
	int rc = -EFAULT;
	u16 local_port = 0;

	rc = bind_local_ports(&local_port);

	if (copy_to_user(argp, &local_port, sizeof(local_port)))
		rc = -EFAULT;

	return rc;
}

static int handle_bst_ioc_release_local_port(void __user *argp)
{
	int rc = -EFAULT;
	u16 local_port;

	if (copy_from_user(&local_port, argp, sizeof(local_port)))
		return rc;

	rc = unbind_local_ports(local_port);

	return rc;
}

static int handle_bst_ioc_set_traffic_flow(void __user *argp)
{
	uint8_t *buf = NULL;
	int buf_len = 0;
	int rc = -EFAULT;
	struct bst_traffic_flow_prop *p_bst_traffic_flow_prop = NULL;

	struct bst_traffic_flow_pkg flow_p;

	if (copy_from_user(&flow_p, argp, sizeof(flow_p)))
		return rc;

	if (flow_p.cnt == 0) {
		bastet_wake_up_traffic_flow();
		rc = 0;
		return rc;
	}

	if (flow_p.cnt > BST_MAX_PROXY_NUM) {
		bastet_loge("proxy count is exceed the max");
		return rc;
	}

	buf_len = BST_TRAFFIC_LEN(flow_p.cnt);

	buf_len *= sizeof(*p_bst_traffic_flow_prop);
	buf = kmalloc(buf_len, GFP_KERNEL);
	if (buf == NULL) {
		bastet_loge("kmalloc failed");
		rc = -ENOMEM;
		return rc;
	}

	if (copy_from_user(buf, argp
		+ sizeof(flow_p), buf_len)) {
		bastet_loge("pkg copy_from_user error");
		kfree(buf);
		return rc;
	}

	rc = adjust_traffic_flow_by_pkg(buf, flow_p.cnt);
	kfree(buf);

	return rc;
}

static int handle_bst_ioc_get_timestamp_info(void __user *argp)
{
	int rc = 0;
	struct bst_timestamp_info info;

	info.time_now = ((uint32_t)(jiffies));
	info.time_freq = HZ;

	if (copy_to_user(argp, &info, sizeof(info)))
		rc = -EFAULT;
	return rc;
}

static int handle_bst_ioc_set_net_dev_name(void __user *argp)
{
	int rc = -EFAULT;
	struct bst_net_dev_name info;

	if (copy_from_user(&info, argp, sizeof(info)))
		return rc;

	rc = set_current_net_device_name(info.iface);
	return rc;
}

#ifdef CONFIG_HUAWEI_BASTET_COMM
static int handle_bst_ioc_get_modem_rab_id(void __user *argp)
{
	int rc = -EFAULT;
	struct bst_modem_rab_id info;

	rc = get_modem_rab_id(&info);
	if (rc < 0)
		return rc;

	if (copy_to_user(argp, &info, sizeof(info)))
		rc = -EFAULT;
	return rc;
}

static int handle_bst_ioc_get_ipv6_modem_rab_id(void __user *argp)
{
	int rc = -EFAULT;
	struct bst_modem_rab_id info;

	memset(&info, 0, sizeof(info));
	rc = get_ipv6_modem_rab_id(&info);
	if (rc < 0)
		return rc;

	if (copy_to_user(argp, &info, sizeof(info)))
		rc = -EFAULT;
	return rc;
}
#else
static int handle_bst_ioc_get_modem_rab_id(void __user *argp)
{
	int rc = -EFAULT;
	bastet_loge("CONFIG_HUAWEI_BASTET_COMM not defined");
	return rc;
}

static int handle_bst_ioc_get_ipv6_modem_rab_id(void __user *argp)
{
	int rc = -EFAULT;
	bastet_loge("CONFIG_HUAWEI_BASTET_COMM not defined");
	return rc;
}
#endif

static int handle_bst_ioc_sock_reconn_set(void __user *argp)
{
	struct bst_get_sock_comm_prop get_prop;

	if (copy_from_user(&get_prop, argp, sizeof(get_prop)))
		return -EFAULT;
	bastet_logi("BST_IOC_SOCK_RECONN_SET ABANDON");
	return -EFAULT;
}

static int handle_bst_ioc_sock_reconn_fail(void __user *argp)
{
	int rc = -EFAULT;
	struct bst_sock_comm_prop comm_prop;

	if (copy_from_user(&comm_prop, argp, sizeof(comm_prop)))
		return rc;
	bastet_logi("BST_IOC_SOCK_RECONN_FAIL");
	bastet_reconn_failed(comm_prop);
	rc = 0;
	return rc;
}

static int handle_bst_ioc_set_reconn_proxyid(void __user *argp)
{
	int rc = -EFAULT;
	struct reconn_id id;

	if (copy_from_user(&id, argp, sizeof(id)))
		return rc;
	bastet_logi("fd=%d, pid=%d, proxy_id=0x%08X",
		id.guide.fd, id.guide.pid, id.proxy_id);
	rc = bastet_reconn_proxyid_set(id);
	return rc;
}

static int handle_bst_ioc_sync_prop_start(void __user *argp)
{
	int rc = -EFAULT;
	struct bst_set_sock_sync_prop set_p;

	if (copy_from_user(&set_p, argp, sizeof(set_p)))
		return rc;

	rc = bastet_sync_prop_start(&set_p);
	return rc;
}

static int handle_bst_ioc_sync_prop_start_ipv6(void __user *argp)
{
	int rc = -EFAULT;
	struct bst_set_sock_sync_prop_ipv6 set_p;

	if (copy_from_user(&set_p, argp, sizeof(set_p)))
		return rc;

	rc = bastet_sync_prop_start_ipv6(&set_p);
	return rc;
}

static int handle_bst_ioc_sync_prop_stop(void __user *argp)
{
	int rc = -EFAULT;
	struct bst_sock_comm_prop comm_prop;

	if (copy_from_user(&comm_prop, argp, sizeof(comm_prop)))
		return rc;

	rc = bastet_sync_prop_stop(&comm_prop);
	return rc;
}

static int handle_bst_ioc_sync_prop_stop_ipv6(void __user *argp)
{
	int rc = -EFAULT;
	struct bst_sock_comm_prop_ipv6 comm_prop;

	if (copy_from_user(&comm_prop, argp, sizeof(comm_prop)))
		return rc;

	rc = bastet_sync_prop_stop_ipv6(&comm_prop);
	return rc;
}

static int handle_bst_ioc_proxy_available(void __user *argp)
{
	int available = 0;
	int rc = -EFAULT;

	if (copy_from_user(&available, argp, sizeof(int32_t)))
		return rc;
	atomic_set(&proxy, available);
	if (!is_proxy_available()) {
		/* proxy is unavailable, release all uid list */
		bastet_partner_clear();
	}
	bastet_logi("proxy_available=%d",
		is_proxy_available());
	rc = 0;
	return rc;
}

static int handle_bst_ioc_buf_available(void __user *argp)
{
	int available = 0;
	int rc = -EFAULT;

	if (copy_from_user(&available, argp, sizeof(int32_t)))
		return rc;
	atomic_set(&buffer, available);
	bastet_logi("buffer_available=%d",
		is_buffer_available());
	rc = 0;
	return rc;
}

static int handle_bst_ioc_prio_send_suc(void __user *argp)
{
	int rc = -EFAULT;

	clear_channel_occupied();
	bastet_logi("channel_available=%d", is_channel_available());
	rc = 0;
	return rc;
}

static int handle_bst_ioc_sock_sync_prepare(void __user *argp)
{
	int rc = -EFAULT;

	struct bst_set_sock_sync_delay sync_prop;

	if (copy_from_user(&sync_prop, argp, sizeof(sync_prop)))
		return rc;

	rc = prepare_bastet_sock(&sync_prop);
	return rc;
}

static int handle_bst_ioc_get_fd_by_addr(void __user *argp)
{
	struct addr_to_fd guide;
	int rc = -EFAULT;

	if (copy_from_user(&guide, argp, sizeof(guide)))
		return rc;

	rc = get_fd_by_addr(&guide);
	if (rc < 0) {
		bastet_loge("failed to get fd by address");
		return rc;
	}
	if (copy_to_user(argp, &guide, sizeof(guide)))
		rc = -EFAULT;

	return rc;
}

static int handle_bst_ioc_get_cmdline(void __user *argp)
{
	struct get_cmdline cmdline;
	int rc = -EFAULT;

	if (copy_from_user(&cmdline, argp, sizeof(cmdline)))
		return rc;

	rc = get_pid_cmdline(&cmdline);
	if (rc < 0) {
		bastet_loge("failed to get pid cmdline");
		return rc;
	}
	if (copy_to_user(argp, &cmdline, sizeof(cmdline)))
		rc = -EFAULT;

	return rc;
}

static int handle_bst_ioc_sync_hold_time(void __user *argp)
{
	struct bst_set_sock_sync_delay hold_delay;
	int rc = -EFAULT;

	if (copy_from_user(&hold_delay, argp, sizeof(hold_delay)))
		return rc;

	rc = set_sock_sync_hold_time(hold_delay);
	if (rc < 0) {
		bastet_loge("failed to sync hold time");
		return rc;
	}

	return rc;
}

static int handle_bst_ioc_net_dev_reset(void __user *argp)
{
	int reset = 0;
	int rc = -EFAULT;

	if (copy_from_user(&reset, argp, sizeof(int32_t)))
		return rc;
	atomic_set(&cp_reset, reset);
	bastet_logi("cp_reset=%d", is_cp_reset());
	return 0;
}

static int handle_bst_ioc_uid_prop_monitor(void __user *argp)
{
	struct app_monitor_prop prop;
	int32_t *tids = NULL;
	int rc = -EFAULT;

	if (copy_from_user(&prop, argp,
		sizeof(prop))) {
		rc = -EFAULT;
		return rc;
	}
	bastet_logi(
		"BST_IOC_UID_PROP_MONITOR: uid=%d, cmd=%d, count=%d",
		prop.uid, prop.cmd, prop.tid_count);

	if (prop.tid_count > APP_MAX_PID_NUM) {
		bastet_loge("tid_count exceed the max value");
		return rc;
	}

	if (prop.tid_count > 0) {
		tids = kmalloc_array(prop.tid_count,
			sizeof(int32_t),
			GFP_KERNEL);
		if (tids == NULL) {
			bastet_loge("kmalloc failed");
			rc = -ENOMEM;
			return rc;
		}
		if (copy_from_user(tids,
			argp + sizeof(prop),
			prop.tid_count * sizeof(int32_t))) {
			rc = -EFAULT;
			kfree(tids);
			return rc;
		}
	}
	rc = bastet_partner_process_cmd(&prop, tids);
	if (tids != NULL)
		kfree(tids);

	return rc;
}

static int handle_bst_ioc_set_hrtapp_activity(void __user *argp)
{
	int uid = 0;
	int rc = -EFAULT;

	if (copy_from_user(&uid, argp, sizeof(int32_t)))
		return rc;
	hrt_uid = uid;
	bastet_logi("hrt_uid=%d", hrt_uid);
	rc = 0;
	return rc;
}

static int handle_bst_ioc_set_proc_info(void __user *argp)
{
	int rc = -EFAULT;
	struct set_process_info info;

	if (copy_from_user(&info, argp, sizeof(info))) {
		rc = -EFAULT;
		return rc;
	}
	rc = set_proc_info(&info);
	if (rc < 0) {
		bastet_loge("failed to set proc info");
		return rc;
	}
	if (copy_to_user(argp, &info, sizeof(info)))
		rc = -EFAULT;

	return rc;
}

static int handle_bst_ioc_fg_io_ctrl(void __user *argp)
{
	bastet_logi("BST_IOC_FAST_GRAB_INFO Entered");
	bst_fg_io_ctrl((uintptr_t)argp);
	return 0;
}

static int handle_bst_ioc_set_special_uid(void __user *argp)
{
	int uid;
	int rc = -EFAULT;

	if (copy_from_user(&uid, argp, sizeof(int))) {
		bastet_loge("failed to copy uid from user");
		rc = -EFAULT;
		return rc;
	}
	rc = set_special_uid(uid);
	if (rc < 0) {
		bastet_loge("failed to set proc info");
		return rc;
	}

	return rc;
}

static int handle_bst_ioc_nf_control(void __user *argp)
{
	bool isRegister = true;
	int rc = -EFAULT;

	bastet_logi("BST_IOC_NF_CONTROL");
	if (copy_from_user(&isRegister, argp, sizeof(bool))) {
		bastet_loge("failed to get uid by pid");
		rc = -EFAULT;
		return rc;
	}
	rc = set_netfilter(isRegister);
	if (rc < 0) {
		bastet_loge("failed to set netfilter");
		return rc;
	}

	return rc;
}

static int handle_bst_ioc_get_sock_netdev_name(void __user *argp)
{
	int rc = -EFAULT;
	/* get network device name of socket for usersapce */
	struct get_netdev_name ndev_name;

	if (copy_from_user(&ndev_name, argp, sizeof(ndev_name)))
		return rc;

	/*
	 * find socket by pid&fd,
	 * and get network device name of the socket
	 */
	rc = get_sock_net_dev_name(&ndev_name);
	if (rc < 0) {
		bastet_loge("failed to get net device name");
		return rc;
	}
	/* feedback network device name to userspace */
	if (copy_to_user(argp, &ndev_name, sizeof(ndev_name)))
		rc = -EFAULT;
	return rc;
}

static int handle_bst_ioc_send_hb_data(void __user *argp)
{
	/* send data by socket which is found from pid and fd */
	struct heartbeat_content send;
	uint8_t *content = NULL;
	int rc = -EFAULT;

	if (copy_from_user(&send, argp, sizeof(send))) {
		rc = -EFAULT;
		return rc;
	}
	/*
	 * alloc memory for heartbeat send content, which length is from
	 * struct heartbeat_content
	 */
	if ((send.len == 0) || (send.len > FIXED_CTNT_MAX_LEN)) {
		bastet_loge("send.len is invalid");
		rc = -EINVAL;
		return rc;
	}
	content = kmalloc(send.len, GFP_ATOMIC);
	if (content == NULL) {
		bastet_loge("failed to kmalloc send data");
		rc = -ENOMEM;
		return rc;
	}
	/* continue to get heartbeat send content from userspace */
	if (copy_from_user(content, argp + sizeof(send), send.len)) {
		kfree(content);
		rc = -EFAULT;
		return rc;
	}
	rc = bastet_send_msg(&send.guide, content, send.len);
	/* after data be sent, free the heartbeat send content memory */
	kfree(content);
	return rc;
}

static int handle_bst_ioc_set_hb_reply(void __user *argp)
{
	/* store heartbeat reply content to struct sock */
	struct heartbeat_content reply;
	uint8_t *content = NULL;
	int rc = -EFAULT;

	if (copy_from_user(&reply, argp, sizeof(reply))) {
		rc = -EFAULT;
		return rc;
	}
	/*
	 * alloc memory for heartbeat reply content,
	 * which length is from struct heartbeat_content
	 */
	if ((reply.len > BST_MAX_REPLY_LEN) || (reply.len == 0))
		return rc;

	content = kmalloc(reply.len, GFP_ATOMIC);
	if (content == NULL) {
		bastet_loge("failed to kmalloc reply data");
		rc = -ENOMEM;
		return rc;
	}
	/* continue to get heartbeat reply content from userspace */
	if (copy_from_user(content, argp + sizeof(reply), reply.len)) {
		kfree(content);
		rc = -EFAULT;
		return rc;
	}
	/*
	 * set heartbeat reply content to struct sock,
	 * this memory will be free when sock released
	 */
	rc = bastet_set_hb_reply(&reply.guide, content, reply.len);
	return rc;
}

static int handle_bst_ioc_filter_hb_reply(void __user *argp)
{
	/*
	 * start heartbeat reply filter,
	 * it means there is a heartbeat packet following
	 */
	struct bst_sock_id guide;
	int rc = -EFAULT;

	if (copy_from_user(&guide, argp, sizeof(guide))) {
		rc = -EFAULT;
		return rc;
	}
	/* set heartbeat reply filter */
	rc = bastet_filter_hb_reply(&guide);
	return rc;
}

static int handle_bst_ioc_set_freezer_state(void __user *argp)
{
	/* set process freeze state to struct sock */
	struct freezer_state freezer;
	int rc = -EFAULT;

	if (copy_from_user(&freezer, argp, sizeof(freezer))) {
		rc = -EFAULT;
		return rc;
	}
	/* set process freeze state */
	rc = bastet_set_freezer(freezer);
	return rc;
}

static int handle_bst_ioc_set_prio_ch_enable(void __user *argp)
{
	int32_t enable = 0;
	int rc = -EFAULT;

	if (copy_from_user(&enable, argp, sizeof(int32_t)))
		return rc;
	/* set priority channel enable status */
	atomic_set(&channel_en, enable);
	bastet_logi("channel_en=%d", is_channel_enable());
	rc = 0;
	return rc;
}

/**
 * bastet_ioctl() - bastet ioctl method.
 * @flip: device descriptor.
 * @cmd: command string.
 * @arg: command args.
 *
 * this is main method to exchange data with user space,
 * including socket sync, get ip and port, adjust kernel flow.
 *
 * return deferent return code.
 *        -EFAULT - fail
 *        other - succ.
 */
static long bastet_ioctl(struct file *flip, unsigned int cmd, unsigned long arg)
{
	int rc = -EFAULT;
	void __user *argp = (void __user *)arg;
	int nr_cmd = _IOC_NR(cmd);

	if (nr_cmd < BST_IOC_SOCK_CMD_MIN || nr_cmd > BST_IOC_SOCK_CMD_MAX) {
		bastet_loge("unknown ioctl: %u", nr_cmd);
		return rc;
	}

	bastet_logi("cmd:%s", bst_ioc_cmd_tb[nr_cmd].id_name);
	rc = bst_ioc_cmd_tb[nr_cmd].bst_ioc_handle(argp);

	return rc;
}

/* support of 32bit userspace on 64bit platforms */
#ifdef CONFIG_COMPAT
static long compat_bastet_ioctl(struct file *flip,
	unsigned int cmd, unsigned long arg)
{
	return bastet_ioctl(flip, cmd, (unsigned long) compat_ptr(arg));
}
#endif

static int bastet_open(struct inode *inode, struct file *filp)
{
	spin_lock_bh(&bastet_data.read_lock);

	if (bastet_dev_en) {
		bastet_loge("bastet device has been opened");
		spin_unlock_bh(&bastet_data.read_lock);
		return -EPERM;
	}

	bastet_dev_en = true;

	spin_unlock_bh(&bastet_data.read_lock);
	bastet_logi("success");

	return 0;
}

static int bastet_packet_read(char __user *buf, size_t count)
{
	struct data_packet *pkt = NULL;
	struct bst_device_ind *p_bst_device_ind = NULL;
	uint8_t *data = NULL;
	bool isfree = false;
	int len = 0;
	int size = 0;

	if (buf == NULL)
		return -EINVAL;

	spin_lock_bh(&bastet_data.read_lock);
	if (list_empty(&bastet_data.read_queue)) {
		spin_unlock_bh(&bastet_data.read_lock);
		return -EAGAIN;
	}

	pkt = list_first_entry(&bastet_data.read_queue,
		struct data_packet, list);
	len = sizeof(*p_bst_device_ind)
		- sizeof(pkt->data.cons) + pkt->data.len;
	data = (uint8_t *)(&pkt->data) + sizeof(pkt->data.cons);

	if ((pkt->data.cons == 0) && (count > len)) {
		list_del(&pkt->list);
		size = len;
		isfree = true;
	} else if (((pkt->data.cons == 0) && (count <= len))
		|| ((pkt->data.cons != 0) && (pkt->data.cons + count <= len))) {
		size = count;
		isfree = false;
	} else {
		list_del(&pkt->list);
		size = len - pkt->data.cons;
		isfree = true;
	}

	spin_unlock_bh(&bastet_data.read_lock);
	if (copy_to_user(buf, data + pkt->data.cons, size)) {
		pkt->data.cons = 0;
		if (isfree)
			kfree(pkt);

		return -EFAULT;
	}
	pkt->data.cons += size;

	if (isfree)
		kfree(pkt);

	return size;
}

/**
 * bastet_read() - read the data.
 * @filp: file descriptor.
 * @buf: user space buffer.
 * @count: read numbers.
 * @ppos: read offset.
 *
 * blocked read, it will be waiting here until net device state is change.
 * standard arg is "const char __user *buf".
 *
 * Return: read the data size.
 *
 */
/*lint -e666*/
static ssize_t bastet_read(struct file *filp,
	char __user *buf, size_t count, loff_t *ppos)
{
	int ret = 0;

	spin_lock_bh(&bastet_data.read_lock);
	while (list_empty(&bastet_data.read_queue)) {
		spin_unlock_bh(&bastet_data.read_lock);
		ret = wait_event_interruptible(bastet_data.read_wait,
			!list_empty(&bastet_data.read_queue));
		if (ret)
			return ret;

		spin_lock_bh(&bastet_data.read_lock);
	}
	spin_unlock_bh(&bastet_data.read_lock);

	return bastet_packet_read(buf, count);
}
/*lint +e666*/

#ifdef CONFIG_HUAWEI_BASTET_COMM
static ssize_t bastet_write(struct file *filp,
	const char __user *buf, size_t count, loff_t *ppos)
{
	uint8_t *msg;
	int32_t ret = count;

	msg = kmalloc(BST_MAX_WRITE_PAYLOAD, GFP_KERNEL);

	if (msg == NULL)
		return -ENOMEM;

	if ((count > BST_MAX_WRITE_PAYLOAD) || (count <= 0)) {
		bastet_loge("write length over BST_MAX_WRITE_PAYLOAD!");
		ret = -EINVAL;
		goto write_end;
	}

	if (copy_from_user(msg, buf, count)) {
		bastet_loge("copy_from_user error");
		ret = -EFAULT;
		goto write_end;
	}

	bastet_comm_write(msg, count, BST_ACORE_CORE_MSG_TYPE_DSPP);

write_end:
	kfree(msg);
	return ret;
}
#endif

static unsigned int bastet_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;

	poll_wait(file, &bastet_data.read_wait, wait);
	mask = !list_empty(&bastet_data.read_queue) ? (POLLIN | POLLRDNORM) : 0;

	return mask;
}

static int bastet_release(struct inode *inode, struct file *filp)
{
	struct list_head *p = NULL;
	struct list_head *n = NULL;
	struct data_packet *pkt = NULL;

	spin_lock_bh(&bastet_data.read_lock);

	if (list_empty(&bastet_data.read_queue))
		goto out_release;

	list_for_each_safe(p, n, &bastet_data.read_queue) {
		pkt = list_entry(p, struct data_packet, list);
		list_del(&pkt->list);
		kfree(pkt);
	}

out_release:
	bastet_dev_en = false;
	spin_unlock_bh(&bastet_data.read_lock);
	bastet_partner_release();
	bastet_logi("success");

	return 0;
}

static const struct file_operations bastet_dev_fops = {
	.owner = THIS_MODULE,
	.open = bastet_open,
	.unlocked_ioctl = bastet_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = compat_bastet_ioctl,
#endif
	.read = bastet_read,
#ifdef CONFIG_HUAWEI_BASTET_COMM
	.write = bastet_write,
#endif
	.poll = bastet_poll,
	.release = bastet_release,
};

static void bastet_data_init(void)
{
	spin_lock_init(&bastet_data.read_lock);
	INIT_LIST_HEAD(&bastet_data.read_queue);
	init_waitqueue_head(&bastet_data.read_wait);
}

static int bastet_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = NULL;

	bastet_data_init();
	bastet_utils_init();
	bastet_traffic_flow_init();
#ifdef CONFIG_HUAWEI_BASTET_COMM
	bastet_comm_init();
#endif
	bastet_reconn_init();
	bastet_partner_init();
	bst_fg_init();
	bastet_filter_init();
	/* register bastet major and minor number */
	ret = alloc_chrdev_region(&bastet_dev,
		BST_FIRST_MINOR, BST_DEVICES_NUMBER, BASTET_NAME);
	if (ret) {
		bastet_loge("alloc_chrdev_region error");
		goto fail_region;
	}

	cdev_init(&bastet_cdev, &bastet_dev_fops);
	bastet_cdev.owner = THIS_MODULE;

	ret = cdev_add(&bastet_cdev, bastet_dev, BST_DEVICES_NUMBER);
	if (ret) {
		bastet_loge("cdev_add error");
		goto fail_cdev_add;
	}

	bastet_class = class_create(THIS_MODULE, BASTET_NAME);
	if (IS_ERR(bastet_class)) {
		bastet_loge("class_create error");
		goto fail_class_create;
	}

	dev = device_create(bastet_class, NULL, bastet_dev, NULL, BASTET_NAME);
	if (IS_ERR(dev)) {
		bastet_loge("device_create error");
		goto fail_device_create;
	}

	return 0;

fail_device_create:
	class_destroy(bastet_class);
fail_class_create:
	cdev_del(&bastet_cdev);
fail_cdev_add:
	unregister_chrdev_region(bastet_dev, BST_DEVICES_NUMBER);
fail_region:
	bastet_partner_release();
	return ret;
}

static int bastet_remove(struct platform_device *pdev)
{
	bastet_partner_release();
	if (bastet_class != NULL) {
		device_destroy(bastet_class, bastet_dev);
		class_destroy(bastet_class);
	}
	cdev_del(&bastet_cdev);
	unregister_chrdev_region(bastet_dev, BST_DEVICES_NUMBER);
	bastet_utils_exit();

	return 0;
}

#ifdef CONFIG_OF
static struct of_device_id of_bastet_match_tbl[] = {
	{
		.compatible = "huawei,bastet",
	},
	{ /* end */ }
};

MODULE_DEVICE_TABLE(of, of_bastet_match_tbl);
#endif

static struct platform_driver bastet_driver = {
	.probe	= bastet_probe,
	.remove	= bastet_remove,
	.driver = {
		.name	= "bastet",
		.owner	= THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(of_bastet_match_tbl),
#endif
	},
};

module_platform_driver(bastet_driver);

MODULE_AUTHOR("zhuxiaolong@huawei.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Bastet driver");
