/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Contexthub flp driver.
 * Create: 2017-02-18
 */
#include "smart_flp.h"
#include <clocksource/arm_arch_timer.h>
#include <linux/delay.h>
#include <linux/compat.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/hisi/contexthub/iomcu_dump.h>
#include <linux/hisi/hisi_syscounter.h>
#include <linux/io.h>
#include <linux/init.h>
#include <net/genetlink.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/pm_wakeup.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/time.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#include <securec.h>
#include <soc_acpu_baseaddr_interface.h>
#include <soc_syscounter_interface.h>
#include "../inputhub_wrapper.h"
#include "smart_softtimer.h"
#include "../shmem.h"
#include "../common.h"

/*lint -e750 -esym(750,*) */
#define FLP_BATCHING        (BIT(0))
#define FLP_GEOFENCE        (BIT(1))
#define FLP_CELLFENCE       (BIT(2))
#define FLP_CELLTRAJECTORY  (BIT(3))
#define FLP_OFFLINEDB       (BIT(4))
#define FLP_WIFIFENCE       (BIT(5))
#define FLP_CELLBATCHING    (BIT(7))
#define FLP_DIAG            (BIT(9))
#define FLP_ERR_TIMEOUT     222

#define IOMCU_APP_FLP       (FLP_BATCHING|FLP_GEOFENCE|FLP_CELLFENCE|FLP_CELLTRAJECTORY|FLP_WIFIFENCE|FLP_CELLBATCHING|FLP_DIAG)
#define FLP_MAX_GET_DATA   (1024 * 200)
#ifdef CONFIG_SMART_FLP_DIAG
#define FLP_DIAG_MAX_CMD_LEN 64
#endif

#if defined (DDR_SHMEMEXT_ADDR_AP) && defined (DDR_SHMEMEXT_SIZE)
#define OFFLINE_INDOOR_MMAP_ADDR  (DDR_SHMEMEXT_ADDR_AP + DDR_SHMEMEXT_SIZE - OFFLINE_INDOOR_MMAP_SIZE)
#define OFFLINE_INDOOR_MMAP_SIZE  (0x80000)
#define OFFLINE_CELL_MMAP_SIZE    (0x100000)
#define MMAP_OFFSET_CHECK_BIT     (8)
#endif
enum {
	OFFLINE_TYPE_INDOOR = 1,
	OFFLINE_TYPE_CELL,
	OFFLINE_TYPE_INVALID
};

enum {
	LOC_STATUS_STOP,
	LOC_STATUS_START,
	LOC_STATUS_OFF,
	LOC_STATUS_ON
};

/*lint +e750 +esym(750,*) */

typedef struct flp_device {
	struct list_head        list;
	unsigned int            service_type ;
	struct mutex            lock;
	unsigned int            denial_sevice;
	struct mutex            recovery_lock;
	struct completion       shmem_completion;
	int                     shmem_resp_errno;
	struct workqueue_struct *wq;
} flp_device_t;

typedef struct flp_data_buf{
	char                  *data_buf;
	unsigned int          buf_size;
	unsigned int          read_index;
	unsigned int          write_index;
	unsigned int          data_count;
} flp_data_buf_t;

typedef struct flp_port {
	struct list_head        list;
	unsigned int            channel_type;
	struct softtimer_list   sleep_timer ;
	batching_config_t       gps_batching_config;
	unsigned int            portid;
	unsigned long           total_count ;
	struct work_struct      work;
	unsigned int            work_para;
	unsigned int            need_awake;
	unsigned int            need_report;
	struct wakeup_source    wlock;
	unsigned int            need_hold_wlock;
	loc_source_status       location_status;
} flp_port_t;

struct flp_report_data_handle_work {
	struct work_struct worker;
	void *data;
};

flp_device_t  g_flp_dev;
static unsigned int g_flp_shmem_transid;

#if (defined CONFIG_SMART_FLP_GEOFENCE) || (defined CONFIG_SMART_FLP_WIFIFENCE) || (defined CONFIG_SMART_FLP_CELLFENCE)
static int flp_fence_shmem(flp_port_t *flp_port, hal_config_t *config,
	unsigned short int shmem_cmd, unsigned int type);
#endif

/*lint -e785 */
static struct genl_family flp_genl_family = {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0))
	.id         = GENL_ID_GENERATE,
#endif
	.name       = FLP_GENL_NAME,
	.version    = TASKFLP_GENL_VERSION,
	.maxattr    = FLP_GENL_ATTR_MAX,
};
/*lint +e785 */
/*lint +e655*/
static int flp_genlink_checkin(flp_port_t *flp_port, unsigned int count, unsigned char cmd_type)
{
	if (flp_port == NULL) {
		pr_err("[%s] flp_port NULL\n", __func__);
		return -EINVAL;
	}

	if (flp_port->portid == 0) {
		pr_err("[%s]no portid error\n", __func__);
		return -EBUSY;
	}

	return 0;
}

/*lint -e826 -e834 -e776*/

/*
 * Generate netlink data packet and send to HAL
 */
static int flp_generate_netlink_packet(flp_port_t *flp_port, const char *buf,
		unsigned int count, unsigned char cmd_type)
{
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh = NULL;
	void *msg_header = NULL;
	char *data = NULL ;
	int result;
	static unsigned int flp_event_seqnum;

	result = flp_genlink_checkin(flp_port, count, cmd_type);
	if (result != 0) {
		pr_err("[%s]flp_genlink_checkin[%d]\n", __func__, result);
		return result;
	}

	skb = genlmsg_new((size_t)count, GFP_ATOMIC);
	if (skb == NULL)
		return -ENOMEM;

	/* add the genetlink message header */
	msg_header = genlmsg_put(skb, 0, flp_event_seqnum++,
	&flp_genl_family, 0, cmd_type);
	if (msg_header == NULL) {
		nlmsg_free(skb);
		return -ENOMEM;
	}

	/* fill the data */
	data = nla_reserve_nohdr(skb, (int)count);
	if (data == NULL) {
		nlmsg_free(skb);
		return -EINVAL;
	}

	if (buf != NULL && count) {
		result = memcpy_s(data, (size_t)count, buf, (size_t)count);
		if (result != EOK) {
			nlmsg_free(skb);
			pr_err("[%s]memset_s fail[%d]\n", __func__, result);
			return result;
		}
	}

	/* if aligned, just set real count */
	nlh = (struct nlmsghdr *)((unsigned char *)msg_header - GENL_HDRLEN - NLMSG_HDRLEN);
	nlh->nlmsg_len = count + GENL_HDRLEN + NLMSG_HDRLEN;

	pr_info("%s %d:%d\n", __func__,  cmd_type, nlh->nlmsg_len);
	/* send unicast genetlink message */
	result = genlmsg_unicast(&init_net, skb, flp_port->portid);
	if (result != 0)
		pr_err("flp:Failed to send netlink event:%d", result);

	return result;
}
/*lint -e845*/
/*lint +e826 +e834 +e776 +e845*/
/*lint +e838*/

/*lint -e834 -e776*/

/*lint -e845 -e826*/

/*
 * Send cellbatching data reported by contexthub to HAL
 */
/*lint +e845 */
static void flp_resort_cellbatching_data(char *data, unsigned int len)
{
	cell_batching_data_t *temp1 = (cell_batching_data_t *)data;
	cell_batching_data_t *temp2 = (cell_batching_data_t *)data;

	int index = 0;
	int ret;
	unsigned long time1 = 0;
	unsigned long time2 = 0;
	bool is_resort = false;
	unsigned int temp_len = len;

	if (data == NULL || len == 0 || (len % sizeof(cell_batching_data_t)) != 0) {
		pr_err("flp:%s invalid data len[%d]\n", __func__, len);
		return;
	}
	while ((temp_len - sizeof(cell_batching_data_t)) != 0) {
		index++;
		time1 = temp1[index-1].timestamplow;
		time1 |= (unsigned long) (temp1[index-1].timestamphigh) << 32;
		time2 = temp2[index].timestamplow;
		time2 |= (unsigned long) (temp2[index].timestamphigh) << 32;
		if (time1 > time2) {
			is_resort = true;
			pr_info("flp: %s resort cellbatching index %d\n", __func__, index);
			break;
		}
		temp_len -= sizeof(cell_batching_data_t);
	}
	if (is_resort) {
		char *begin_index = data + index * sizeof(cell_batching_data_t);
		char *buff = kzalloc(len, GFP_KERNEL);
		if (buff == NULL) {
			pr_info("flp: %s kmalloc fail\n", __func__);
			return ;
		}
		ret = memcpy_s(buff, len, begin_index, len - index * sizeof(cell_batching_data_t));
		if (ret != EOK) {
			pr_err("%s memcpy_s error\n", __func__);
			kfree(buff);
			return;
		}
		ret = memcpy_s(buff + len - index * sizeof(cell_batching_data_t), index * sizeof(cell_batching_data_t), data, index * sizeof(cell_batching_data_t));
		if (ret != EOK) {
			pr_err("%s memcpy_s error\n", __func__);
			kfree(buff);
			return;
		}
		ret = memcpy_s(data, len, buff, len);
		if (ret != EOK) {
			pr_err("%s memcpy_s error\n", __func__);
			kfree(buff);
			return;
		}
		kfree(buff);
		pr_info("flp: %s done\n", __func__);
	}
}

static void flp_send_data_to_uplayer(flp_port_t *flp_port, const char *msg, unsigned int len, unsigned int check_type, unsigned int cmd)
{
	if (flp_port->channel_type & check_type) {
		flp_generate_netlink_packet(flp_port, msg, len, cmd);
	}
}

/*
 * Send the data reported by contexthub to HAL
 */
static int get_common_data_from_mcu(const struct pkt_header *head)
{
	unsigned int len;
	char *data = NULL;
	flp_port_t *flp_port = NULL;
	struct list_head *pos = NULL;

	if (!head) {
		pr_err("%s, head is null\n", __func__);
		return -EINVAL;
	}

	if (head->length < sizeof(unsigned int)) {
		pr_err("%s, length is too short\n", __func__);
		return -EINVAL;
	}
	len = head->length - sizeof(unsigned int);
	data = (char *)((pkt_subcmd_req_t *)head + 1);//lint !e838

	pr_info("flp:%s cmd:%d: len:%d\n", __func__, head->cmd, len);
	mutex_lock(&g_flp_dev.lock);
	list_for_each(pos, &g_flp_dev.list) {//lint !e838
		flp_port = container_of(pos, flp_port_t, list);
		pr_info("flp:%s, type[%d], subcmd[%d]\n", __func__, flp_port->channel_type, ((pkt_subcmd_req_t *)head)->subcmd);
		switch (((pkt_subcmd_req_t *)head)->subcmd) {
		case SUB_CMD_FLP_LOCATION_UPDATE_REQ:
			flp_send_data_to_uplayer(flp_port, data, len, FLP_BATCHING, FLP_GENL_CMD_GNSS_LOCATION);
			break;
		case SUB_CMD_FLP_GEOF_TRANSITION_REQ:
			flp_send_data_to_uplayer(flp_port, data, len, FLP_GEOFENCE, FLP_GENL_CMD_GEOFENCE_TRANSITION);
			break;
		case SUB_CMD_FLP_GEOF_MONITOR_STATUS_REQ:
			flp_send_data_to_uplayer(flp_port, data, len, FLP_GEOFENCE, FLP_GENL_CMD_GEOFENCE_MONITOR);
			break;
		case SUB_CMD_FLP_RESET_RESP:
			if (IOMCU_APP_FLP & flp_port->channel_type) {
				flp_port->channel_type &= ~IOMCU_APP_FLP;
				g_flp_dev.service_type &= ~IOMCU_APP_FLP;
				flp_generate_netlink_packet(flp_port, data, (unsigned int)sizeof(unsigned int), FLP_GENL_CMD_IOMCU_RESET);
			}
			break;

		case SUB_CMD_FLP_CELLFENCE_TRANSITION_REQ:
			flp_send_data_to_uplayer(flp_port, data, len, FLP_CELLFENCE, FLP_GENL_CMD_CELLTRANSITION);
			break;
		case SUB_CMD_FLP_CELLTRAJECTORY_REPORT_REQ:
			flp_resort_cellbatching_data(data, len);
			flp_send_data_to_uplayer(flp_port, data, len, FLP_CELLTRAJECTORY, FLP_GENL_CMD_TRAJECTORY_REPORT);
			break;
		case SUB_CMD_FLP_CELLDB_LOCATION_REPORT_REQ:
			flp_send_data_to_uplayer(flp_port, data, len, FLP_OFFLINEDB, FLP_GENL_CMD_REQUEST_CELLDB);
			break;
		case SUB_CMD_FLP_GEOF_GET_LOCATION_REPORT_REQ:
			flp_send_data_to_uplayer(flp_port, data, len, FLP_GEOFENCE, FLP_GENL_CMD_GEOFENCE_LOCATION);
			break;
		case SUB_CMD_FLP_WIFENCE_TRANSITION_REQ:
			flp_send_data_to_uplayer(flp_port, data, len, FLP_WIFIFENCE, FLP_GENL_CMD_WIFIFENCE_TRANSITION);
			break;
		case SUB_CMD_FLP_WIFENCE_STATUS_REQ:
			flp_send_data_to_uplayer(flp_port, data, len, FLP_WIFIFENCE, FLP_GENL_CMD_WIFIFENCE_MONITOR);
			break;
		case SUB_CMD_FLP_DIAG_DATA_REPORT_REQ:
			flp_send_data_to_uplayer(flp_port, data, len, FLP_DIAG, FLP_GENL_CMD_DIAG_DATA_REPORT);
			break;
		case SUB_CMD_FLP_CELL_CELLBATCHING_REPORT_REQ:
			flp_resort_cellbatching_data(data, len);
			flp_send_data_to_uplayer(flp_port, data, len, FLP_CELLBATCHING, FLP_GENL_CMD_CELLBATCHING_REPORT);
			break;
		case SUB_CMD_FLP_GEOF_GET_SIZE_REPORT_REQ:
			flp_send_data_to_uplayer(flp_port, data, len, FLP_GEOFENCE, FLP_GENL_CMD_GEOFENCE_SIZE);
			break;
		case SUB_CMD_FLP_CELLFENCE_GET_SIZE_REPORT_REQ:
			flp_send_data_to_uplayer(flp_port, data, len, FLP_CELLFENCE, FLP_GENL_CMD_CELLFENCE_SIZE);
			break;
		case SUB_CMD_FLP_WIFENCE_GET_SIZE_REPORT_REQ:
			flp_send_data_to_uplayer(flp_port, data, len, FLP_WIFIFENCE, FLP_GENL_CMD_WIFIFENCE_SIZE);
			break;
		default:
			pr_err("flp:%s cmd[0x%x] error\n", __func__, head->cmd);
			mutex_unlock(&g_flp_dev.lock);
			return -EFAULT;
		}
	}
	mutex_unlock(&g_flp_dev.lock);
	return (int)len;
}

/*
 * Send the data of known ID reported by contexthub to HAL
 */
static int flp_handle_report_data(const struct pkt_header *head)
{
	if (head == NULL) {
		pr_err("[%s] head is NULL\n", __func__);
		return -EINVAL;
	}
	switch (((pkt_subcmd_req_t *)head)->subcmd) {
	case SUB_CMD_FLP_LOCATION_UPDATE_REQ:
	case SUB_CMD_FLP_GEOF_TRANSITION_REQ:
	case SUB_CMD_FLP_GEOF_MONITOR_STATUS_REQ:
	case SUB_CMD_FLP_CELLFENCE_TRANSITION_REQ:
	case SUB_CMD_FLP_CELLTRAJECTORY_REPORT_REQ:
	case SUB_CMD_FLP_CELLDB_LOCATION_REPORT_REQ:
	case SUB_CMD_FLP_GEOF_GET_LOCATION_REPORT_REQ:
	case SUB_CMD_FLP_WIFENCE_TRANSITION_REQ:
	case SUB_CMD_FLP_WIFENCE_STATUS_REQ:
	case SUB_CMD_FLP_CELL_CELLBATCHING_REPORT_REQ:
	case SUB_CMD_FLP_DIAG_DATA_REPORT_REQ:
	case SUB_CMD_FLP_GEOF_GET_SIZE_REPORT_REQ:
	case SUB_CMD_FLP_CELLFENCE_GET_SIZE_REPORT_REQ:
	case SUB_CMD_FLP_WIFENCE_GET_SIZE_REPORT_REQ:
		return get_common_data_from_mcu(head);
	default:
		pr_err("FLP[%s]uncorrect subcmd 0x%x.\n", __func__, ((pkt_subcmd_req_t *)head)->subcmd);
	}
	return -EFAULT;
}

/*
 * Report data to work handler
 */
/*lint -e838*/
static void flp_report_data_handle_work_handler(struct work_struct *work)
{
	int ret;
	struct flp_report_data_handle_work *w = NULL;

	if (work == NULL) {
		pr_err("[%s] w or data is NULL\n", __func__);
		return;
	}

	w = container_of(work, struct flp_report_data_handle_work, worker);
	if (w == NULL || w->data == NULL) {
		pr_err("[%s] w or data is NULL\n", __func__);
		return;
	}

	ret = flp_handle_report_data((const struct pkt_header *)w->data);
	if (ret < 0)
		pr_warn("FLP[%s]handle report data fail\n", __func__);

	kfree(w->data);
	kfree(w);
}

/*
 * Process data sent from the sensor hub in a new thread for prevent deadlock
 */
static int get_data_from_mcu(const struct pkt_header *head)
{
	size_t len;
	struct flp_report_data_handle_work *wk = NULL;

	if (!head) {
		pr_err("[%s] head is NULL\n", __func__);
		return -EINVAL;
	}

	len = head->length + sizeof(struct pkt_header);
	if (len >= FLP_MAX_GET_DATA) {
		pr_err("[%s] date len too big, len:%zu\n", __func__, len);
		return -EINVAL;
	}

	wk = kzalloc(sizeof(struct flp_report_data_handle_work), GFP_KERNEL);
	if (!wk) {
		pr_err("[%s] alloc work error is err.\n", __func__);
		return -ENOMEM;
	}

	wk->data = kzalloc(len, GFP_KERNEL);
	if (!wk->data) {
		kfree(wk);
		pr_err("[%s] kzalloc wk data error\n", __func__);
		return -ENOMEM;
	}

	if (memcpy_s(wk->data, len, head, len) != EOK) {
		kfree(wk->data);
		kfree(wk);
		pr_err("%s memcpy_s error\n", __func__);
		return -EFAULT;
	}

	INIT_WORK(&wk->worker, flp_report_data_handle_work_handler);
	queue_work(g_flp_dev.wq, &wk->worker);
	return 0;
}
/*lint +e838*/

/*
 * Business recovery after contexthub reset
 */
static void  flp_service_recovery(void)
{
	flp_port_t *flp_port = NULL;
	unsigned int response = FLP_IOMCU_RESET;
	struct list_head *pos = NULL; // coverity is conflict with pclint for valiable init, disable pclint 838 warning below
	struct list_head *tmp = NULL;

	mutex_lock(&g_flp_dev.recovery_lock);
	list_for_each_safe(pos, tmp, &g_flp_dev.list) {//lint !e838
		flp_port = container_of(pos, flp_port_t, list);
		if (flp_port->channel_type & IOMCU_APP_FLP) {
			flp_port->channel_type &= ~IOMCU_APP_FLP;
			g_flp_dev.service_type &= ~IOMCU_APP_FLP;
			flp_generate_netlink_packet(flp_port, (char *)&response, (unsigned int)sizeof(unsigned int), FLP_GENL_CMD_IOMCU_RESET);
		}
	}
	mutex_unlock(&g_flp_dev.recovery_lock);
}
/*lint -e715*/
static int flp_notifier(struct notifier_block *nb,
			unsigned long action, void *data)
{
	switch (action) {
	case IOM3_RECOVERY_3RD_DOING:
		flp_service_recovery();
		break;
	default:
		pr_err("register_iom3_recovery_notifier err\n");
		break;
	}
	return 0;
}
/*lint +e715*/
static struct notifier_block sensor_reboot_notify = {
	.notifier_call = flp_notifier,
	.priority = -1,
};

static void flp_timerout_work(struct work_struct *wk)
{
	flp_port_t *flp_port  = container_of(wk, flp_port_t, work);

	flp_generate_netlink_packet(flp_port, NULL, 0, (unsigned char)flp_port->work_para);
}
static void flp_sleep_timeout(unsigned long data)
{
	flp_port_t *flp_port = (flp_port_t *)((uintptr_t)data);
	pr_info("flp_sleep_timeout \n");
	if (flp_port != NULL) {
		flp_port->work_para = FLP_GENL_CMD_NOTIFY_TIMEROUT;
		queue_work(system_power_efficient_wq, &flp_port->work);
		if (flp_port->need_hold_wlock)
			__pm_wakeup_event(&flp_port->wlock, jiffies_to_msecs(2 * HZ));
	}
	return ;
}

void flp_port_resume(void)
{
	struct list_head *pos = NULL;
	flp_port_t *flp_port = NULL;

	mutex_lock(&g_flp_dev.lock);
	list_for_each(pos, &g_flp_dev.list) {//lint !e838
		flp_port = container_of(pos, flp_port_t, list);
		if (flp_port->need_awake) {
			flp_port->work_para = FLP_GENL_CMD_AWAKE_RET;
			queue_work(system_power_efficient_wq, &flp_port->work);
		}
	}
	mutex_unlock(&g_flp_dev.lock);
}
/*
 * FLP command ID check
 */
static bool flp_check_cmd(flp_port_t *flp_port, unsigned int cmd, int type)
{
	switch (type) {
	case FLP_GEOFENCE:
	case FLP_BATCHING:
	case FLP_CELLFENCE:
	case FLP_DIAG:
	case FLP_WIFIFENCE:
		if ((!(flp_port->channel_type & IOMCU_APP_FLP)) && (g_flp_dev.service_type & IOMCU_APP_FLP)) {
			pr_err("FLP[%s] ERR: FLP APP not support multi process!\n", __func__);
			return false;
		}
		if ((cmd & FLP_IOCTL_TAG_MASK) == FLP_IOCTL_TAG_FLP) {
			return true;
		}
		break ;
	default:
		break ;
	}
	return false;
}

/*lint -e715 -e502*/
/*
 * Send start message to contexthub
 */
static int flp_common_ioctl_open_service(flp_port_t *flp_port)
{
	unsigned int response = FLP_IOMCU_RESET;
	struct read_info rd;

	(void)memset_s((void *)&rd, sizeof(rd), 0, sizeof(struct read_info));
	if (g_flp_dev.service_type & IOMCU_APP_FLP) {
		g_flp_dev.service_type &= ~IOMCU_APP_FLP;
		flp_generate_netlink_packet(flp_port, (char *)&response, (unsigned int)sizeof(unsigned int),
					    FLP_GENL_CMD_IOMCU_RESET);
	}
	return 0;
}
/*
 * Send a closed message to contexthub
 */
static int flp_common_ioctl_close_service(flp_port_t *flp_port)
{
	if (g_flp_dev.service_type & IOMCU_APP_FLP)
		inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0, NULL);
	return 0;
}

/*lint -e530*/
/*
 * Send a service stop message to contexthub
 */
static int flp_stop_service_type(flp_port_t *flp_port, unsigned long arg)
{
	int service_id;
	int ret;
	unsigned int status;
	struct read_info rd = {0};

	if (get_user(service_id, (int __user *)((uintptr_t)arg)))
		return -EFAULT;

	status = (unsigned int)service_id;
	if ((flp_port->channel_type & status) == 0 || (g_flp_dev.service_type & status) == 0) {
		pr_err("[%s]service has closed, service_id[%d]\n", __func__, service_id);
		return -EACCES;
	}
	if ((flp_port->channel_type & IOMCU_APP_FLP) == 0) {
		pr_err("[%s]iomcu flp had closed, service_id[%d]\n", __func__, service_id);
		return -EACCES;
	}

	if ((g_flp_dev.service_type & IOMCU_APP_FLP) == 0) {
		pr_err("[%s]global service_type has cleanup\n", __func__);
		flp_port->channel_type &= ~IOMCU_APP_FLP;
		return -EACCES;
	}

	ret = inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_COMMON_STOP_SERVICE_REQ,
					    (char *)&service_id, sizeof(service_id), &rd);
	if (ret != 0) {
		pr_err("[%s]iomcu err[%d]\n", __func__, rd.errno);
		return ret;
	}

	flp_port->channel_type &= ~status;

	/* here global service_type is bad design,I just obey the original design */
	g_flp_dev.service_type &= ~status;
	pr_info("%s current open service[0x%x], close[0x%x]\n",
		__func__, g_flp_dev.service_type, service_id);
	if ((g_flp_dev.service_type & IOMCU_APP_FLP) == 0)
		(void)inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0, NULL);

	return ret;
}
/*lint +e530*/

static int flp_iomcu_template (flp_port_t *flp_port, unsigned long arg, unsigned int iomcu_sub_cmd)
{
	CELLFENCE_IOCTRL_HDR_TYPE cfence_hdr;
	char usr[MAX_PKT_LENGTH];

	if ((flp_port->channel_type & IOMCU_APP_FLP) == 0) {
		pr_err("[%s] ERR: you must add cellfence first error\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user(&cfence_hdr, (void *)((uintptr_t)arg), sizeof(CELLFENCE_IOCTRL_HDR_TYPE))) {
		pr_err("[%s]copy_from_user error\n", __func__);
		return -EFAULT;
	}

	if (cfence_hdr.len == 0 || MAX_PKT_LENGTH < cfence_hdr.len) {
		pr_err("[%s] cfence_hdr.len  [%u]error\n", __func__, cfence_hdr.len);
		return -EPERM;
	}

	if (copy_from_user(usr, (void *)cfence_hdr.buf, (unsigned long)cfence_hdr.len)) {
		pr_err("[%s]copy_from_user usr error\n", __func__);
		return -EFAULT;
	}

	return inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, iomcu_sub_cmd,
				    usr, (unsigned long)cfence_hdr.len, NULL);
}
/*
 * cellfence command processing
 */
int cellfence_operate (flp_port_t *flp_port, unsigned long arg)
{
	unsigned int sub_cmd = SUB_CMD_FLP_CELLFENCE_OPT_REQ;

	return flp_iomcu_template(flp_port, arg, sub_cmd);
}
/*
 * cellfence data injection
 */
int cellfence_inject_result(flp_port_t *flp_port, unsigned long arg)
{
	unsigned int sub_cmd = SUB_CMD_FLP_CELLFENCE_INJECT_RESULT_REQ;

	return flp_iomcu_template(flp_port, arg, sub_cmd);
}

/*
 * Send wifi message to contexthub
 */
static int flp_wifi_cfg(flp_port_t *flp_port, unsigned long arg)
{
	unsigned int sub_cmd = SUB_CMD_FLP_COMMON_WIFI_CFG_REQ;

	return flp_iomcu_template(flp_port, arg, sub_cmd);
}
/*
 * flp service switch
 */
static inline void flp_service_ops(flp_port_t *flp_port, unsigned int data)
{
	g_flp_dev.denial_sevice = data;
	pr_info("%s 0x%x\n", __func__, g_flp_dev.denial_sevice);

	if (g_flp_dev.denial_sevice) {
		flp_common_ioctl_close_service(flp_port);
	} else {
		flp_common_ioctl_open_service(flp_port);
	}
}

/*
 * Location information injection
 */
static int flp_common_inject_location(unsigned long arg)
{
	iomcu_location inject_data;
	if (copy_from_user(&inject_data, (void *)((uintptr_t)arg), sizeof(iomcu_location))) {
		pr_err("%s copy_from_user error\n", __func__);
		return -EFAULT;
	}
	return inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_INJECT_LOCATION_REQ, (char *)&inject_data, sizeof(iomcu_location), NULL);
}

#if (defined CONFIG_SMART_FLP_GEOFENCE) || (defined CONFIG_SMART_FLP_WIFIFENCE) || (defined CONFIG_SMART_FLP_CELLFENCE)
/*
 * Public data injection
 */
static int flp_common_inject_data(flp_port_t *flp_port, unsigned long arg)
{
#ifdef CONFIG_HISI_DEBUG_FS
	hal_config_t config;
	int ret;

	if (copy_from_user(&config, (void *)((uintptr_t)arg), sizeof(hal_config_t))) {
		pr_err("[%s] copy hal config error\n", __func__);
		return -EFAULT;
	}
	ret = flp_fence_shmem(flp_port, &config, FLP_SHMEM_CMD_INJECT_DATA, 0);
	return ret;
#else
	return 0;
#endif
}
#endif
/*
 * Send push location status message to contexthub
 */
static int flp_common_push_location_status(flp_port_t *flp_port, unsigned long arg)
{
	loc_source_status_hal hal_status;
	int ret;
	unsigned int clean_bit;

	if (copy_from_user(&hal_status, (void *)((uintptr_t)arg), sizeof(loc_source_status_hal))) {
		pr_err("[flperr][%s] copy_from_user error\n", __func__);
		return -EFAULT;
	}

	if (hal_status.source >= LOCATION_SOURCE_SUPPORT || hal_status.status > LOC_STATUS_ON) {
		pr_err("[%s]:line[%d] ERR: Invalid source[0x%x] status[0x%x]\n", __func__, __LINE__, hal_status.source, hal_status.status);
		return -EINVAL;
	}

	flp_port->location_status.source |= (1 << hal_status.source);
	clean_bit = hal_status.status ^ 1;
	flp_port->location_status.status[hal_status.source] |= (1 << hal_status.status);
	flp_port->location_status.status[hal_status.source] &= (~(1 << clean_bit));

	if (!(flp_port->channel_type & IOMCU_APP_FLP)) {
		pr_info("[%s]:line[%d]you must have flp first source[0x%x] status[0x%x], ap store source[0x%x] status[0x%x]\n",
			__func__, __LINE__, hal_status.source, hal_status.status, flp_port->location_status.source,
			flp_port->location_status.status[hal_status.source]);
		return 0;
	}

	ret = inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_BATCH_PUSH_GNSS_REQ,
					(char *)&flp_port->location_status, sizeof(loc_source_status), NULL);

	pr_info("[%s]:line[%d] hal send source[0x%x] status[0x%x], ap will send source[0x%x] status[0x%x] result[%d]\n",
	       __func__, __LINE__, hal_status.source, hal_status.status, flp_port->location_status.source,
	       flp_port->location_status.status[hal_status.source], ret);
	return ret;
}

/*
 * Send debug config message to contexthub
 */
/*lint -e603*/
static int flp_common_set_debug_config(unsigned long arg)
{
	int cfg;
	int ret;

	if (get_user(cfg, (int __user *)((uintptr_t)arg))) {
		pr_err("[%s]:line[%d] open or send error\n", __func__, __LINE__);
		return -EFAULT;
	}

	ret = inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_COMMON_DEBUG_CONFIG_REQ,
				   (char *)&cfg, sizeof(cfg), NULL);

	pr_info("[%s]:line[%d] dbg cfg[%d] result[%d]\n", __func__, __LINE__, cfg, ret);
	return ret;
}
/*lint +e603*/

/*
 * hifence ioctl message processing
 */
static int flp_hifence_common_ioctl(flp_port_t *flp_port, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case FLP_IOCTL_COMMON_STOP_SERVICE_TYPE:
		return flp_stop_service_type(flp_port, arg);
	case FLP_IOCTL_COMMON_WIFI_CFG:
		return flp_wifi_cfg(flp_port, arg);
	case FLP_IOCTL_COMMON_INJECT:
		return flp_common_inject_location(arg);
	case FLP_IOCTL_COMMON_LOCATION_STATUS:
		return flp_common_push_location_status(flp_port, arg);
	case FLP_IOCTL_COMMON_DEBUG_CONFIG:
		return flp_common_set_debug_config(arg);
#if (defined CONFIG_SMART_FLP_GEOFENCE) || (defined CONFIG_SMART_FLP_WIFIFENCE) || (defined CONFIG_SMART_FLP_CELLFENCE)
	case FLP_IOCTL_COMMON_INJECT_DATA:
		return flp_common_inject_data(flp_port, arg);
#endif
	default:
		pr_err("flp_hifence_common_ioctl input cmd[0x%x] error\n", cmd);
		break;
	}
	return -EFAULT;
}

/*
 * flp common ioctl message processing
 */
static int flp_common_ioctl(flp_port_t *flp_port, unsigned int cmd, unsigned long arg)
{
	unsigned int data = 0;
	int ret = 0;

	if (FLP_IOCTL_COMMON_RELEASE_WAKELOCK != cmd) {
		if (copy_from_user(&data, (void *)((uintptr_t)arg), sizeof(unsigned int))) {
			pr_err("flp_ioctl copy_from_user error[%d]\n", ret);
			return -EFAULT;
		}
	}

	switch (cmd) {
	case FLP_IOCTL_COMMON_SLEEP:
		pr_info("flp:start timer %d\n",  data);
		/* if timer is running just delete it ,then restart it */
		smart_softtimer_delete(&flp_port->sleep_timer);
		ret = smart_softtimer_modify(&flp_port->sleep_timer, data);
		if (ret == 0)
			smart_softtimer_add(&flp_port->sleep_timer);
		break ;
	case FLP_IOCTL_COMMON_AWAKE_RET:
		flp_port->need_awake = data;
		break ;
	case FLP_IOCTL_COMMON_SETPID:
		flp_port->portid = data;
		break ;
	case FLP_IOCTL_COMMON_CLOSE_SERVICE:
		flp_service_ops(flp_port, data);
		break;
	case FLP_IOCTL_COMMON_HOLD_WAKELOCK:
		flp_port->need_hold_wlock = data;
		pr_info("%s 0x%x\n", __func__, flp_port->need_hold_wlock);
		break;
	case FLP_IOCTL_COMMON_RELEASE_WAKELOCK:
		if (flp_port->need_hold_wlock) {
			__pm_relax(&flp_port->wlock);/*lint !e455*/
		}
		break;
	default:
			ret = flp_hifence_common_ioctl(flp_port, cmd, arg);
			return ret;
	}
	return 0;
}

struct flp_shmem_resp_pkt {
	struct pkt_header hd;
	uint32_t transid;
	int32_t errno ;
} __packed;

static int flp_shmem_resp(const struct pkt_header *head)
{
	struct flp_shmem_resp_pkt *resp = (struct flp_shmem_resp_pkt *)head;

	if (resp->transid != g_flp_shmem_transid) {
		pr_warn("%s shmem response transid:%u, but expect:%u\n",
			__func__, resp->transid, g_flp_shmem_transid);
		return 0;
	}

	pr_info("%s get shmem resp transid:%u\n", __func__, resp->transid);

	g_flp_dev.shmem_resp_errno = resp->errno;
	complete(&g_flp_dev.shmem_completion);
	return 0;
}

int flp_shmem_send_async(unsigned short int cmd, const void __user *buf,
			unsigned int length)
{
	struct flp_shmem_hdr *iomcu_data = NULL;
	unsigned int iomcu_len;
	int ret;

	if (!buf || length == 0) {
		pr_err("%s user buf is null or length:%u fail\n",
			__func__, length);
		return -ENOMEM;
	}

	if (length > shmem_get_capacity()) {
		pr_err("%s length:%u too large, capacity:%u\n",
			__func__, length, shmem_get_capacity());
		return -ENOMEM;
	}

	iomcu_len = sizeof(struct flp_shmem_hdr) + length;
	iomcu_data = kzalloc((size_t)iomcu_len, GFP_KERNEL); //lint !e838
	if (IS_ERR_OR_NULL(iomcu_data)) {
		pr_err("%s kmalloc fail\n", __func__);
		return -ENOMEM;
	}

	ret = (int)copy_from_user(iomcu_data->data, (const void __user *)buf,
				(unsigned long)length);
	if (ret) {
		pr_err("%s copy_from_user length:%u ret:%d error\n", __func__,
			length, ret);
		ret = -EFAULT;
		goto OUT;
	}

	iomcu_data->tag = FLP_IOMCU_SHMEM_TAG;
	iomcu_data->cmd = cmd;
	iomcu_data->transid = ++g_flp_shmem_transid;
	iomcu_data->data_len = length;

	pr_info("%s cmd:0x%x,transid:%u iomcu_len:%u\n", __func__, cmd, iomcu_data->transid, iomcu_len);

#ifdef CONFIG_INPUTHUB_30
	if (iomcu_len > INPUTHUB_WRAPPER_MAX_LEN)
		ret = shmem_send(TAG_FLP, iomcu_data, (unsigned int)iomcu_len);
	else
		ret = inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ,
			SUB_CMD_FLP_SHMEM_REQ, (const char*)iomcu_data, iomcu_len, NULL);
#else /* CONFIG_INPUTHUB_20 */
	ret = shmem_send(TAG_FLP, iomcu_data, (unsigned int)iomcu_len);
#endif
	if (ret) {
		pr_err("%s shmem_send error\n", __func__);
		ret = -EFAULT;
		goto OUT;
	}
OUT:
	if (!IS_ERR_OR_NULL(iomcu_data))
		kfree(iomcu_data);

	return ret;
}

int flp_shmem_send_sync(unsigned short int cmd, const void __user *buf,
			unsigned int length)
{
	int ret;

	reinit_completion(&g_flp_dev.shmem_completion);

	ret = flp_shmem_send_async(cmd, buf, length);
	if (ret) {
		pr_err("[%s]:[%d] flp_shmem_send_async send fail\n", __func__, __LINE__);
		return ret;
	}

	// wait for sending result
	if (!wait_for_completion_timeout(&g_flp_dev.shmem_completion, msecs_to_jiffies(2000))) {
		pr_err("[%s]:[%d] shmem send timeout\n", __func__, __LINE__);
		return FLP_ERR_TIMEOUT;
	}

	ret = g_flp_dev.shmem_resp_errno;

	return ret;
}

/*lint +e715*/
/*lint -e438*/
static int flp_open_send_status_flp(flp_port_t *flp_port)
{
	int ret = 0;

	if (!(flp_port->channel_type & IOMCU_APP_FLP)) {
		ret = inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_OPEN_REQ, 0, NULL, (size_t)0, NULL);
		if (ret != 0) {
			pr_err("[flperr][%s]:line[%d] iomcu open err \n", __func__, __LINE__);
			return ret;
		}
	}
	return ret;
}

#if (defined CONFIG_SMART_FLP_GEOFENCE) || (defined CONFIG_SMART_FLP_WIFIFENCE) || (defined CONFIG_SMART_FLP_CELLFENCE)
/*
 * Send fence message to contexthub through share memory
 */
static int flp_fence_shmem(flp_port_t *flp_port, hal_config_t *config,
				unsigned short int shmem_cmd, unsigned int type)
{
	int ret;

	ret = flp_open_send_status_flp(flp_port);
	if (ret != 0) {
		return ret;
	}

	ret = flp_shmem_send_sync(shmem_cmd, (const void __user *)config->buf,
		(unsigned long)config->length);
	if (ret != 0 && ret != FLP_ERR_TIMEOUT)
		return ret;

	flp_port->channel_type |= type;
	g_flp_dev.service_type |= type;

	return ret;
}
#endif

#if (defined CONFIG_SMART_FLP_GEOFENCE) || (defined CONFIG_SMART_FLP_WIFIFENCE)
/*
 * Read fence status
 */
/*lint -e603*/
static int flp_fence_status_cmd(flp_port_t *flp_port, unsigned long arg, unsigned int cmd)
{
	struct read_info rd = {0};
	int ret;
	int id;

	if (flp_port == NULL)
		return -EFAULT;

	if (get_user(id, (int __user *)((uintptr_t)arg)))
		return -EFAULT;

	ret = inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ,
										cmd, (char *)&id, sizeof(id), &rd);
	if (ret != 0) {
		pr_err("[%s]iomcu err[%d]\n", __func__, rd.errno);
		ret = -ENOENT;
		goto STATUS_FIN;
	}

	if (sizeof(int) != rd.data_length) {
		pr_err("[%s]iomcu len err[%d]\n", __func__, rd.data_length);
		ret = -EPERM;
		goto STATUS_FIN;
	}

	ret = *(int *)rd.data;

STATUS_FIN:
	return ret;
}
/*lint +e603*/
#endif

#ifdef CONFIG_SMART_FLP_GEOFENCE
/*
 * Send a fence message to contexthub
 */
static int flp_ioctl_add_geofence(flp_port_t *flp_port, unsigned long arg)
{
	hal_config_t config;
	int ret;

	if (copy_from_user(&config, (void *)((uintptr_t)arg), sizeof(hal_config_t))) {
		pr_err("[flperr][%s] copy_from_user hal_config error\n", __func__);
		return -EFAULT;
	}

	if (config.length > FLP_GEOFENCE_MAX_NUM * sizeof(geofencing_useful_data_t) || config.length == 0) {
		pr_err("[flperr]geofence number overflow %u\n", config.length);
		return -EFAULT;
	}

	ret = flp_fence_shmem(flp_port, &config, FLP_SHMEM_ADD_GEOFENCE, (unsigned int)FLP_GEOFENCE);
	return ret;
}

/*
 * Read fence status
 */
static int flp_geofence_status_cmd(flp_port_t *flp_port, unsigned long arg)
{
	int ret;
	if ((flp_port->channel_type & FLP_GEOFENCE) == 0) {
		pr_err("[%s]you must add geo first\n", __func__);
		return -EIO;
	}

	ret = flp_fence_status_cmd(flp_port, arg, SUB_CMD_FLP_GEOF_GET_TRANSITION_REQ);
	return ret;
}

/*
 * Send delete geofence message to contexthub
 */
static int flp_ioctl_remove_geofence(flp_port_t *flp_port, unsigned long arg)
{
	hal_config_t config;
	int ret;

	if ((flp_port->channel_type & FLP_GEOFENCE) == 0) {
		pr_err("[%s] not start \n", __func__);
		return -EPERM;
	}

	if (copy_from_user(&config, (const void __user *)((uintptr_t)arg), sizeof(hal_config_t))) {
		pr_err("[%s] copy_from_user hal_config error\n", __func__);
		return -EFAULT;
	}

	if (config.length > FLP_GEOFENCE_MAX_NUM * sizeof(int) || config.length == 0) {
		pr_err("[%s] geofence number %u overflow\n", __func__, config.length);
		return -EFAULT;
	}

	ret = flp_fence_shmem(flp_port, &config, FLP_SHMEM_REMOVE_GEOFENCE, (unsigned int)FLP_GEOFENCE);
	return ret;
}

/*
 * Send modified geofence message to contexthub
 */
static int flp_ioctl_modify_geofence(flp_port_t *flp_port, unsigned long arg)
{
	geofencing_option_info_t modify_config;
	int ret;

	if ((flp_port->channel_type & FLP_GEOFENCE) == 0) {
		pr_err("%s not start \n", __func__);
		return -EPERM;
	}
	if (copy_from_user(&modify_config, (const void __user *)((uintptr_t)arg), sizeof(geofencing_option_info_t))) {
		pr_err("%s copy_from_user error\n", __func__);
		return -EFAULT;
	}

	ret = inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_MODIFY_GEOF_REQ,
			     (char *)&modify_config, sizeof(geofencing_option_info_t), NULL);
	return ret;
}

/*
 * Send a message to get geofence location to contexthub
 */
static int flp_geofence_get_location(flp_port_t *flp_port, unsigned long arg)
{
	if ((flp_port->channel_type & FLP_GEOFENCE) == 0) {
		pr_err("%s not start\n", __func__);
		return -EPERM;
	}

	inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_GEOF_GET_LOCATION_REQ, NULL, (size_t)0, NULL);
	return 0;
}

static int flp_geofence_get_size(flp_port_t *flp_port, unsigned long arg)
{
	int ret;

	if ((flp_port->channel_type & FLP_GEOFENCE) == 0) {
		pr_err("%s not start\n", __func__);
		return -EPERM;
	}
	ret = inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_GEOF_GET_SIZE_REQ,
		NULL, (size_t)0, NULL);
	return ret;
}

/*
 * geofence IOCTL message processing
 */
static int flp_geofence_ioctl(flp_port_t *flp_port, unsigned int cmd, unsigned long arg)
{
	int ret;
	switch (cmd) {
	case FLP_IOCTL_GEOFENCE_ADD:
		ret = flp_ioctl_add_geofence(flp_port, arg);
		break;
	case FLP_IOCTL_GEOFENCE_REMOVE:
		ret = flp_ioctl_remove_geofence(flp_port, arg);
		break;
	case FLP_IOCTL_GEOFENCE_MODIFY:
		ret = flp_ioctl_modify_geofence(flp_port, arg);
		break;
	case FLP_IOCTL_GEOFENCE_STATUS:
		ret = flp_geofence_status_cmd(flp_port, arg);
		break;
	case FD_IOCTL_GEOFENCE_GET_LOCATION:
		ret = flp_geofence_get_location(flp_port, arg);
		break;
	case FD_IOCTL_GEOFENCE_GET_SIZE:
		ret = flp_geofence_get_size(flp_port, arg);
		break;
	default:
		pr_err("%s input cmd[0x%x] error\n", __func__, cmd);
		ret = -EFAULT;
		break;
	}
	return ret;
}
#endif /* CONFIG_SMART_FLP_GEOFENCE end */

/*lint +e438*/
#ifdef CONFIG_SMART_FLP_BATCHING
/* max complexiy must less than 15 */
/*
 * Send location BATCHING command to contexthub
 */
static int __flp_location_ioctl(flp_port_t *flp_port, unsigned int cmd, unsigned long arg)
{
	int data;

	if (!(flp_port->channel_type & FLP_BATCHING)) {
		pr_err("%s not start \n", __func__);
		return -EPERM;
	}
	switch (cmd) {
	case FLP_IOCTL_BATCHING_STOP:
		if (copy_from_user(&data, (void *)((uintptr_t)arg), sizeof(int))) {
			pr_err("%s copy_from_user error\n", __func__);
			return -EFAULT;
		}
		inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_STOP_BATCHING_REQ, (char *)&data, sizeof(int), NULL);
		flp_port->channel_type &= ~FLP_BATCHING;
		g_flp_dev.service_type &= ~FLP_BATCHING;
		break ;
	case FLP_IOCTL_BATCHING_UPDATE:
		if (copy_from_user(&flp_port->gps_batching_config, (void *)((uintptr_t)arg), sizeof(batching_config_t))) {
			pr_err("%s copy_from_user error\n", __func__);
			return -EFAULT;
		}
		inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_UPDATE_BATCHING_REQ,
		(char *)&flp_port->gps_batching_config, sizeof(batching_config_t), NULL);
		break;
	case FLP_IOCTL_BATCHING_LASTLOCATION:
		if (copy_from_user(&data, (void *)((uintptr_t)arg), sizeof(int))) {
			pr_err("%s copy_from_user error\n", __func__);
			return -EFAULT;
		}
		inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_GET_BATCHED_LOCATION_REQ, (char *)&data, sizeof(int), NULL);
		break ;
	case FLP_IOCTL_BATCHING_FLUSH:
		inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_FLUSH_LOCATION_REQ, NULL, (size_t)0, NULL);
		break;

	case FLP_IOCTL_BATCHING_MULT_LASTLOCATION:
		if (copy_from_user(&data, (void *)((uintptr_t)arg), sizeof(int))) {
			pr_err("%s copy_from_user error\n", __func__);
			return -EFAULT;
		}
		inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_BATCHING_MULT_LASTLOCATION_REQ, (char *)&data, sizeof(int), NULL);
		break ;
	case FLP_IOCTL_BATCHING_MULT_FLUSH:
		if (copy_from_user(&data, (void *)((uintptr_t)arg), sizeof(int))) {
			pr_err("%s copy_from_user error\n", __func__);
			return -EFAULT;
		}
		inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_BATCHING_MULT_FLUSH_REQ, (char *)&data, sizeof(int), NULL);
		break ;

	default:
		return -EFAULT;
	}
	return 0;
}

/*
 * location batching ioctl message processing
 */
static int flp_location_ioctl(flp_port_t *flp_port, unsigned int cmd, unsigned long arg)
{
	struct read_info rd = {0};
	int ret;

	switch (cmd) {
	case FLP_IOCTL_BATCHING_START:
		ret = flp_open_send_status_flp(flp_port);
		if (ret != 0)
			return ret;

		if (copy_from_user(&flp_port->gps_batching_config, (void *)((uintptr_t)arg), sizeof(batching_config_t))) {
			pr_err("%s copy_from_user error\n", __func__);
			return -EFAULT;
		}
		ret = inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_START_BATCHING_REQ,
				     (char *)&flp_port->gps_batching_config, sizeof(batching_config_t), NULL);
		flp_port->channel_type |= FLP_BATCHING;
		g_flp_dev.service_type |= FLP_BATCHING;
		break ;
	case FLP_IOCTL_BATCHING_STOP:
	case FLP_IOCTL_BATCHING_UPDATE:
	case FLP_IOCTL_BATCHING_LASTLOCATION:
	case FLP_IOCTL_BATCHING_FLUSH:
	case FLP_IOCTL_BATCHING_MULT_LASTLOCATION:
	case FLP_IOCTL_BATCHING_MULT_FLUSH:
		ret = __flp_location_ioctl(flp_port, cmd, arg);
		break ;
	case FLP_IOCTL_BATCHING_GET_SIZE:
		ret = inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_GET_BATCH_SIZE_REQ, NULL, (size_t)0, &rd);
		if (ret == 0) {
			if (copy_to_user((void *)((uintptr_t)arg), rd.data, sizeof(u16))) {
				pr_err("%s copy_to_user error\n", __func__);
				return -EFAULT;
			}
		}
		break;
	default:
		pr_err("%s input cmd[0x%x] error\n", __func__, cmd);
		return -EFAULT;
	}
	return ret;
}
#endif /* CONFIG_SMART_FLP_BATCHING end */

/*lint -e438*/
#ifdef CONFIG_SMART_FLP_CELLFENCE
/*
 * Send a message to add cellfence to contexthub
 */
static int cellfence_add(flp_port_t *flp_port, unsigned long arg, unsigned short int cmd)
{
	hal_config_t config;
	int ret;

	if (copy_from_user(&config, (void *)((uintptr_t)arg), sizeof(hal_config_t))) {
		pr_err("[%s] copy_from_user hal_config error\n", __func__);
		return -EFAULT;
	}

	if (config.length > CELLFENCE_ADD_INFO_BUF_SIZE || config.length == 0) {
		pr_err("[%s] length %u overflow\n", __func__, config.length);
		return -EFAULT;
	}
	ret = flp_fence_shmem(flp_port, &config, cmd, (unsigned int)FLP_CELLFENCE);
	return ret;
}

 /*
 * Send the base station trace message to contexthub
 */
/*lint -e603*/
static int celltrajectory_cfg(flp_port_t *flp_port, unsigned long arg)
{
	int cfg = 0;
	int ret;

	if (get_user(cfg, (int __user *)((uintptr_t)arg)))
		return -EFAULT;

	ret = flp_open_send_status_flp(flp_port);
	if (ret != 0) {
		pr_err("[%s]open or send error\n", __func__);
		return ret;
	}
	ret = inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ,
									SUB_CMD_FLP_CELLTRAJECTORY_CFG_REQ, (char *)&cfg, sizeof(cfg), NULL);
	if (ret != 0) {
		pr_err("[%s]GEOFENCE_RM_REQ error\n", __func__);
		return ret;
	}
	pr_info("flp:[%s]cfg[%d]\n", __func__, cfg);
	if (cfg != 0) {
		flp_port->channel_type |= FLP_CELLTRAJECTORY;
		g_flp_dev.service_type |= FLP_CELLTRAJECTORY;
	} else {
		flp_port->channel_type &= ~FLP_CELLTRAJECTORY;
		g_flp_dev.service_type &= ~FLP_CELLTRAJECTORY;
	}

	return ret;
}
/*lint +e603*/

 /*
 * Send the message of the base station trajectory request to contexthub
 */
static int celltrajectory_request(flp_port_t *flp_port)
{
	if ((flp_port->channel_type & FLP_CELLTRAJECTORY) == 0) {
		pr_err("[%s] ERR: you must celltrajectory_cfg first \n", __func__);
		return -EINVAL;
	}

	return inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ,
				    SUB_CMD_FLP_CELLTRAJECTORY_REQUEST_REQ, NULL, (size_t)0, NULL);
}

 /*
 * Send cellbatching config to contexthub
 */
static int flp_cellbatching_cfg(flp_port_t *flp_port, unsigned long arg)
{
	int ret;
	cell_batching_start_config_t ct_cfg;

	if (arg == 0) {
		pr_err("[flperr][%s] invalid param\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(&ct_cfg, (void *)((uintptr_t)arg), sizeof(cell_batching_start_config_t))) {
		pr_err("[%s]copy_from_user error\n", __func__);
		return -EIO;
	}

	if ((flp_port->channel_type & IOMCU_APP_FLP) == 0) {
		ret = inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_OPEN_REQ, 0, NULL, (size_t)0, NULL);
		if (ret != 0) {
			pr_err("[%s]CMD_CMN_OPEN_REQ error\n", __func__);
			return ret;
		}
	}

	ret = inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ,
	SUB_CMD_FLP_CELL_CELLBATCHING_CFG_REQ, (char *)&ct_cfg, sizeof(cell_batching_start_config_t), NULL);
	if (ret != 0) {
		if (!(g_flp_dev.service_type & IOMCU_APP_FLP))
			(void)inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0, NULL);
		pr_err("[%s]GEOFENCE_RM_REQ error\n", __func__);
		return ret;
	}

	pr_info("flp:[%s]cfg[%d]\n", __func__, ct_cfg.cmd);
	if (ct_cfg.cmd) {
		flp_port->channel_type |= FLP_CELLBATCHING;
		g_flp_dev.service_type |= FLP_CELLBATCHING;
	} else {
		flp_port->channel_type &= ~FLP_CELLBATCHING;
		g_flp_dev.service_type &= ~FLP_CELLBATCHING;
	}

	return ret;
}

 /*
 * Send cellbatching request to contexthub
 */
static int flp_cellbatching_request(flp_port_t *flp_port)
{

	if ((flp_port->channel_type & FLP_CELLBATCHING) == 0) {
		pr_err("[%s] ERR: you must cellbatching_cfg first \n", __func__);
		return -EINVAL;
	}
	return inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ,
				    SUB_CMD_FLP_CELL_CELLBATCHING_REQ, NULL, (size_t)0, NULL);
}

static int flp_cellfence_get_size(flp_port_t *flp_port, unsigned long arg)
{
	int ret;

	if ((flp_port->channel_type & FLP_CELLFENCE) == 0) {
		pr_err("%s not start\n", __func__);
		return -EPERM;
	}
	ret = inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_CELLFENCE_GET_SIZE_REQ,
	NULL, (size_t)0, NULL);
	return ret;
}

/*
 * cellfence ioctl message processing
 */
static int cellfence_ioctl(flp_port_t *flp_port, unsigned int cmd, unsigned long arg)
{
	if (!flp_check_cmd(flp_port, (int)cmd, (int)FLP_CELLFENCE)) {
		return -EPERM;
	}
	switch (cmd) {
	case FD_IOCTL_CELLFENCE_ADD:
		return cellfence_add(flp_port, arg, FLP_SHMEM_ADD_CELLFENCE);
	case FD_IOCTL_CELLFENCE_ADD_V2:
		return cellfence_add(flp_port, arg, FLP_SHMEM_ADD_CELLFENCE_V2);
	case FD_IOCTL_CELLFENCE_OPERATE:
		return cellfence_operate(flp_port, arg);
	case FD_IOCTL_TRAJECTORY_CONFIG:
		return celltrajectory_cfg(flp_port, arg);
	case FD_IOCTL_TRAJECTORY_REQUEST:
		return celltrajectory_request(flp_port);
	case FD_IOCTL_CELLFENCE_INJECT_RESULT:
		return cellfence_inject_result(flp_port, arg);
	case FD_IOCTL_CELLBATCHING_CONFIG:
		return flp_cellbatching_cfg(flp_port, arg);
	case FD_IOCTL_CELLBATCHING_REQUEST:
		return flp_cellbatching_request(flp_port);
	case FD_IOCTL_CELLFENCE_GET_SIZE:
		return flp_cellfence_get_size(flp_port, arg);
	default:
		pr_err("[%s]cmd err[%u] \n", __func__, cmd);
		return -EINVAL;
	}
}
#endif /* CONFIG_SMART_FLP_CELLFENCE */

#ifdef CONFIG_SMART_FLP_WIFIFENCE
/*
 * Send a message to add wififence to contexthub
 */
static int flp_add_wififence(flp_port_t *flp_port, unsigned long arg)
{
	hal_config_t config;
	int ret;

	if (copy_from_user(&config, (void *)((uintptr_t)arg), sizeof(hal_config_t))) {
		pr_err("[flperr][%s] copy_from_user hal_config error\n", __func__);
		return -EFAULT;
	}

	if (config.length > MAX_ADD_WIFI_NUM * sizeof(wififence_req_t) || config.length == 0) {
		pr_err("[flperr]wififence number error %u\n", config.length);
		return -EFAULT;
	}
	ret = flp_fence_shmem(flp_port, &config, FLP_SHMEM_ADD_WIFIFENCE, (unsigned int)FLP_WIFIFENCE);
	return ret;
}

/*
 * Send operation (including pause, resume, delete) WIFI fence message to contexthub
 */
static int flp_oper_wififence(flp_port_t *flp_port, unsigned long arg, unsigned int cmd)
{
	hal_config_t config;
	char *cmd_data = NULL; //lint !e838

	if ((flp_port->channel_type & FLP_WIFIFENCE) == 0) {
		pr_err("%s not start \n", __func__);
		return -EPERM;
	}
	if (copy_from_user(&config, (const void __user *)((uintptr_t)arg), sizeof(hal_config_t))) {
		pr_err("%s copy_from_user error\n", __func__);
		return -EFAULT;
	}
	if (config.length > FLP_MAX_WIFIFENCE_SIZE || config.length == 0) {
		pr_err("flp geofence number overflow %d\n", config.length);
		return -EFAULT;
	}
	cmd_data = kzalloc((size_t)config.length, GFP_KERNEL);//lint !e838
	if (cmd_data == NULL) {
		pr_err("%s kmalloc fail\n", __func__);
		return -ENOMEM;
	}
	if (copy_from_user(cmd_data, (const void __user *)config.buf, (unsigned long)config.length)) {
		pr_err("%s copy_from_user error\n", __func__);
		kfree(cmd_data);
		return -EFAULT;
	}

	inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, cmd,
			     (char *)cmd_data, (size_t)config.length, NULL);
	kfree(cmd_data);
	return 0;
}

/*
 * Send query WIFI fence status command to contexthub
 */
static int flp_wififence_status_cmd(flp_port_t *flp_port, unsigned long arg)
{
	int ret;

	if ((flp_port->channel_type & FLP_WIFIFENCE) == 0) {
		pr_err("[%s]you must add wifence first\n", __func__);
		return -EIO;
	}
	ret = flp_fence_status_cmd(flp_port, arg, SUB_CMD_FLP_GET_WIFENCE_STATUS_REQ);
	return ret;
}

static int flp_wififence_get_size(flp_port_t *flp_port, unsigned long arg)
{
	int ret;

	if ((flp_port->channel_type & FLP_WIFIFENCE) == 0) {
		pr_err("%s not start\n", __func__);
		return -EPERM;
	}
	ret = inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_WIFENCE_GET_SIZE_REQ,
		NULL, (size_t)0, NULL);
	return ret;
}

/*
 * wififence ioctl message processing
 */
static int flp_wififence_ioctl(flp_port_t *flp_port, unsigned int cmd, unsigned long arg)
{
	if (flp_check_cmd(flp_port, cmd, (int)FLP_WIFIFENCE) == 0)
		return -EPERM;

	switch (cmd) {
	case FD_IOCTL_WIFIFENCE_ADD:
		return flp_add_wififence(flp_port, arg);
	case FD_IOCTL_WIFIFENCE_REMOVE:
		return flp_oper_wififence(flp_port, arg, SUB_CMD_FLP_REMOVE_WIFENCE_REQ);
	case FD_IOCTL_WIFIFENCE_PAUSE:
		return flp_oper_wififence(flp_port, arg, SUB_CMD_FLP_PAUSE_WIFENCE_REQ);
	case FD_IOCTL_WIFIFENCE_RESUME:
		return flp_oper_wififence(flp_port, arg, SUB_CMD_FLP_RESUME_WIFENCE_REQ);
	case FD_IOCTL_WIFIFENCE_GET_STATUS:
		return flp_wififence_status_cmd(flp_port, arg);
	case FD_IOCTL_WIFIFENCE_GET_SIZE:
		return flp_wififence_get_size(flp_port, arg);
	default:
		pr_err("[%s]cmd err[%u] \n", __func__, cmd);
		return -EINVAL;
	}
}
#endif

#ifdef CONFIG_SMART_FLP_DIAG
/*
 * Send diag command to contexthub
 */
static int flp_diag_send_cmd(flp_port_t *flp_port, unsigned long arg)
{
	unsigned int data_len = 0;
	char *data_buf = NULL;
	int ret = 0;

	if (arg == 0) {
		pr_err("[flperr][%s] invalid param\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(&data_len, (void *)((uintptr_t)arg), sizeof(data_len))) {
		pr_err("[flperr][%s] copy_from_user error\n", __func__);
		return -EFAULT;
	}

	if (data_len > FLP_DIAG_MAX_CMD_LEN || !data_len) {
		pr_err("[flperr][%s] invalid data_len=%d\n", __func__, data_len);
		return -EFAULT;
	}
	data_buf = kzalloc(data_len + sizeof(unsigned int), GFP_KERNEL);//lint !e838
	if (data_buf == NULL) {
		pr_err("[flperr]%s kzalloc fail\n", __func__);
		return -ENOMEM;
	}
	if (copy_from_user(data_buf, (void *)((uintptr_t)arg), data_len + sizeof(unsigned int))) {
		pr_err("[flperr][%s] copy_from_user error\n", __func__);
		ret = -EFAULT;
		goto FLP_DIAG_EXIT;
	}

	if ((flp_port->channel_type & IOMCU_APP_FLP) == 0) {
		ret = inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_OPEN_REQ, 0, NULL, (size_t)0, NULL);
		if (ret != 0) {
			pr_err("[%s]CMD_CMN_OPEN_REQ error\n", __func__);
			goto FLP_DIAG_EXIT;
		}
	}

	ret = inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CONFIG_REQ, SUB_CMD_FLP_DIAG_SEND_CMD_REQ,
				(char *)(data_buf + sizeof(unsigned int)), (size_t)data_len, NULL);
	if (ret != 0) {
		pr_err("[%s]GEOFENCE_RM_REQ error\n", __func__);
		goto FLP_DIAG_EXIT;
	}

	pr_info("flp:[%s]data_len[%d]\n", __func__, data_len);
	flp_port->channel_type |= FLP_DIAG;
	g_flp_dev.service_type |= FLP_DIAG;

FLP_DIAG_EXIT:
	if ((g_flp_dev.service_type & IOMCU_APP_FLP) == 0)
		(void)inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0, NULL);
	kfree(data_buf);
	return ret;
}

/*
 * diag ioctl message processing
 */
static int flp_diag_ioctl(flp_port_t *flp_port, unsigned int cmd, unsigned long arg)
{
	if (flp_check_cmd(flp_port, (int)cmd, (int)FLP_DIAG) == 0) {
		return -EPERM;
	}
	switch (cmd) {
	case FD_IOCTL_DIAG_SEND_CMD:
		return flp_diag_send_cmd(flp_port, arg);
	default:
		pr_err("[%s]cmd err[%u] \n", __func__, cmd);
		return -EINVAL;
	}
}
#endif /* CONFIG_SMART_FLP_DIAG end */
/*lint -e732*/
/*
 * flp ioctl message processing
 */
static long flp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret;
	flp_port_t *flp_port  = (flp_port_t *)file->private_data;
	if (flp_port == NULL) {
		pr_err("flp_ioctl parameter error\n");
		return -EINVAL;
	}
	pr_info("[%s]cmd[0x%x]\n", __func__, cmd & 0x0FFFF);
	mutex_lock(&g_flp_dev.lock);
	if ((g_flp_dev.denial_sevice != 0) && (cmd != FLP_IOCTL_COMMON_CLOSE_SERVICE)) {
		mutex_unlock(&g_flp_dev.lock);
		return 0;
	}

	switch (cmd & FLP_IOCTL_TYPE_MASK) {
#ifdef CONFIG_SMART_FLP_GEOFENCE
	case FLP_IOCTL_TYPE_GEOFENCE:
		if (!flp_check_cmd(flp_port, (int)cmd, (int)FLP_GEOFENCE)) {
			ret = -EPERM;
			break;
		}
		ret = (long)flp_geofence_ioctl(flp_port, cmd, arg);
		break;
#endif
#ifdef CONFIG_SMART_FLP_BATCHING
	case FLP_IOCTL_TYPE_BATCHING:
		if (!flp_check_cmd(flp_port, (int)cmd, (int)FLP_BATCHING)) {
			ret = -EPERM;
			break;
		}
		ret = (long)flp_location_ioctl(flp_port, cmd, arg);
		break;
#endif
#ifdef CONFIG_SMART_FLP_CELLFENCE
	case FLP_IOCTL_TYPE_CELLFENCE:
		ret = (long)cellfence_ioctl(flp_port, cmd, arg);
		break;
#endif
#ifdef CONFIG_SMART_FLP_WIFIFENCE
	case FLP_IOCTL_TYPE_WIFIFENCE:
		ret = (long)flp_wififence_ioctl(flp_port, cmd, arg);
		break;
#endif
#ifdef CONFIG_SMART_FLP_DIAG
	case FLP_IOCTL_TYPE_DIAG:
		ret = (long)flp_diag_ioctl(flp_port, cmd, arg);
		break;
#endif

	case FLP_IOCTL_TYPE_COMMON:
		ret = (long)flp_common_ioctl(flp_port, cmd, arg);
		break;
	default:
		pr_err("flp_ioctl input cmd[0x%x] error\n", cmd);
		ret = -EFAULT;
		break;
	}
	mutex_unlock(&g_flp_dev.lock);
	pr_info("[%s]cmd[0x%x] has processed ret[%ld]\n", __func__, cmd&0x0FFFF, ret);
	return ret;
}
/*lint +e732*/
/*lint -e438*/
/*
 * Device node open function, internal resource initialization
 */
static int flp_open(struct inode *inode, struct file *filp)/*lint -e715*/
{
	int ret = 0;
	flp_port_t *flp_port = NULL;
	struct list_head *pos = NULL;
	int count = 0;

	mutex_lock(&g_flp_dev.lock);
		list_for_each(pos, &g_flp_dev.list) {//lint !e838
		count++;
	}

	if (count > 100) {
		pr_err("flp_open clinet limit\n");
		ret = -EACCES;
		goto FLP_OPEN_ERR;
	}

	flp_port  = (flp_port_t *) kmalloc(sizeof(flp_port_t), GFP_KERNEL);
	if (flp_port == NULL) {
		pr_err("flp_open no mem\n");
		ret = -ENOMEM;
		goto FLP_OPEN_ERR;
	}
	(void)memset_s(flp_port, sizeof(flp_port_t), 0, sizeof(flp_port_t));
	INIT_LIST_HEAD(&flp_port->list);
	smart_softtimer_create(&flp_port->sleep_timer,
			flp_sleep_timeout, (unsigned long)((uintptr_t)flp_port), 0);
	INIT_WORK(&flp_port->work, flp_timerout_work);
	mutex_lock(&g_flp_dev.recovery_lock);
	list_add_tail(&flp_port->list, &g_flp_dev.list);
	mutex_unlock(&g_flp_dev.recovery_lock);
	mutex_unlock(&g_flp_dev.lock);
	wakeup_source_init(&flp_port->wlock, "smart_flp");
	filp->private_data = flp_port;
	pr_info("%s %d: v1.4 enter\n", __func__, __LINE__);
	return 0;
FLP_OPEN_ERR:
	mutex_unlock(&g_flp_dev.lock);
	return ret;
}

static void __flp_release(flp_port_t *flp_port)
{
	kfree(flp_port);
}

/*
 * Device node close function, internal resource release
 */
static int flp_release(struct inode *inode, struct file *file)/*lint -e715*/
{
	flp_port_t *flp_port  = (flp_port_t *)file->private_data;
	pr_info("[%s]\n", __func__);

	if (flp_port == NULL) {
		pr_err("flp_close parameter error\n");
		return -EINVAL;
	}

	smart_softtimer_delete(&flp_port->sleep_timer);
	cancel_work_sync(&flp_port->work);
	wakeup_source_trash(&flp_port->wlock);

	mutex_lock(&g_flp_dev.lock);
	mutex_lock(&g_flp_dev.recovery_lock);
	list_del(&flp_port->list);
	mutex_unlock(&g_flp_dev.recovery_lock);

	/* if start batching or Geofence function ever */
	if (flp_port->channel_type & IOMCU_APP_FLP) {
		inputhub_wrapper_pack_and_send_cmd(TAG_FLP, CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0, NULL);
		flp_port->channel_type &= ~IOMCU_APP_FLP;
		g_flp_dev.service_type &= ~IOMCU_APP_FLP;
	}
	__flp_release(flp_port);
	file->private_data = NULL ;
	mutex_unlock(&g_flp_dev.lock);
	return 0;
}

#if defined (DDR_SHMEMEXT_ADDR_AP) && defined (DDR_SHMEMEXT_SIZE)
/*
 * Device node mmap function
 */
static int flp_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long size;
	int ret;
	flp_port_t *flp_port;
	unsigned long page = 0;
	unsigned long offset;

	flp_port = (flp_port_t *)filp->private_data;
	if (flp_port == NULL) {
		pr_err("[%s]:line[%d] flp parameter error\n", __func__, __LINE__);
		return -EINVAL;
	}

	if (vma->vm_start == 0) {
		pr_err("[%s]:line[%d] Failed : vm_start.0x%lx\n", __func__, __LINE__, vma->vm_start);
		return -EINVAL;
	}

	size = vma->vm_end - vma->vm_start;
	offset = vma->vm_pgoff;
	vma->vm_pgoff = 0;
	pr_info("[%s]:line[%d] enter, vm_start.0x%lx, size.0x%lx, vm_end.0x%lx, offset.0x%lx\n",
	       __func__, __LINE__, vma->vm_start, size, vma->vm_end, offset);

	offset = offset >> MMAP_OFFSET_CHECK_BIT;
	if (OFFLINE_TYPE_CELL == offset) {
		if (OFFLINE_CELL_MMAP_SIZE < size) {
			pr_err("[%s]:line[%d] size.0x%lx.\n", __func__, __LINE__, size);
			return -EINVAL;
		}
		page = DDR_SHMEMEXT_ADDR_AP;
	} else if (OFFLINE_TYPE_INDOOR == offset) {
		if (OFFLINE_INDOOR_MMAP_SIZE < size) {
			pr_err("[%s]:line[%d] current indoor is not set, use cell size[0x%lx].\n", __func__, __LINE__, size);
			return -EINVAL;
		}
		page = OFFLINE_INDOOR_MMAP_ADDR;
	} else {
		pr_err("[%s]:line[%d] Failed : offset[0x%lx] is invalid.\n", __func__, __LINE__, offset);
		return -EINVAL;
	}

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	ret = remap_pfn_range(vma, vma->vm_start, (page >> PAGE_SHIFT), size, vma->vm_page_prot);
	if (ret != 0) {
		pr_err("[%s]:line[%d] remap_pfn_range failed, ret.%d\n", __func__, __LINE__, ret);
		return -EINVAL;
	}
	flp_port->channel_type |= FLP_OFFLINEDB;
	return 0;
}
#else
/*
 * Device node mmap function
 */
static int flp_mmap(struct file *filp, struct vm_area_struct *vma)
{
	flp_port_t *flp_port;

	flp_port = (flp_port_t *)filp->private_data;
	if (flp_port == NULL) {
		pr_err("[%s]:line[%d] flp parameter error\n", __func__, __LINE__);
		return -EINVAL;
	}
	pr_info("[%s]:line[%d] enter, vm_start.0x%lx, vm_end0x%lx, mmap dose not support\n",
	       __func__, __LINE__, vma->vm_start, vma->vm_end);
	return -EINVAL;
}
#endif
/*lint +e438*/
/*lint +e826*/
/*lint -e785 -e64*/
static const struct file_operations flp_fops = {
	.owner          = THIS_MODULE,
	.llseek         = no_llseek,
	.unlocked_ioctl = flp_ioctl,
	.open           = flp_open,
	.release        = flp_release,
	.mmap           = flp_mmap,
#ifdef CONFIG_COMPAT
	.compat_ioctl = flp_ioctl,
#endif
};

/*******************************************************************************************
Description:   miscdevice to motion
*******************************************************************************************/
static struct miscdevice flp_miscdev = {
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     "flp",
	.fops =     &flp_fops,
};
/*lint +e785 +e64*/

/*******************************************************************************************
Function:       flp_register
Data Accessed:  no
Data Updated:   no
Input:          void
Output:        void
Return:        result of function, 0 successed, else false
*******************************************************************************************/
/*lint -e655*/
int flp_register(void)
{
	int ret;

	if (get_contexthub_dts_status() != 0) {
		pr_err("[%s] contexthub disabled!\n", __func__);
		return -EINVAL;
	}

	(void)memset_s((void *)&g_flp_dev, sizeof(g_flp_dev), 0, sizeof(g_flp_dev));
	ret = genl_register_family(&flp_genl_family);
	if (ret != 0) {
		return ret ;
	}
	INIT_LIST_HEAD(&g_flp_dev.list);
	ret = inputhub_wrapper_register_event_notifier(TAG_FLP,
			CMD_CMN_CONFIG_REQ, get_data_from_mcu);
	if (ret != 0) {
		pr_err("[%s] register data report process failed!\n", __func__);
		goto register_failed;
	}
	ret = inputhub_wrapper_register_event_notifier(TAG_FLP,
			CMD_SHMEM_AP_RECV_REQ, flp_shmem_resp);
	if (ret != 0) {
		pr_err("[%s] register flp_shmem_resp failed!\n", __func__);
		goto unregister_config_resp;
	}

	register_iom3_recovery_notifier(&sensor_reboot_notify);
	mutex_init(&g_flp_dev.lock);
	mutex_init(&g_flp_dev.recovery_lock);
	init_completion(&g_flp_dev.shmem_completion);

	g_flp_dev.wq = create_freezable_workqueue("flp_wq");
	if (!g_flp_dev.wq) {
		pr_err("[%s] create workqueue fail!\n", __func__);
		ret = -EFAULT;
		goto unregister_shmem_resp;
	}

	ret = misc_register(&flp_miscdev);
	if (ret != 0)    {
		pr_err("cannot register smart flp err=%d\n", ret);
		goto destroy_wq;
	}
	pr_info("flp_register success\n");
	return 0;

destroy_wq:
	destroy_workqueue(g_flp_dev.wq);
unregister_shmem_resp:
	(void)inputhub_wrapper_unregister_event_notifier(TAG_FLP,
		CMD_SHMEM_AP_RECV_REQ, flp_shmem_resp);
unregister_config_resp:
	(void)inputhub_wrapper_unregister_event_notifier(TAG_FLP,
		CMD_CMN_CONFIG_REQ, get_data_from_mcu);
register_failed:
	genl_unregister_family(&flp_genl_family);
	return ret;
}
EXPORT_SYMBOL_GPL(flp_register);
/*lint +e655*/

/*******************************************************************************************
Function:       flp_unregister
Description:    unregister for Fused Location Provider function
Data Accessed:  no
Data Updated:   no
Input:          void
Output:        void
Return:        void
*******************************************************************************************/
int flp_unregister(void)
{
	destroy_workqueue(g_flp_dev.wq);
	(void)inputhub_wrapper_unregister_event_notifier(TAG_FLP,
		CMD_SHMEM_AP_RECV_REQ, flp_shmem_resp);
	(void)inputhub_wrapper_unregister_event_notifier(TAG_FLP, CMD_CMN_CONFIG_REQ, get_data_from_mcu);
	genl_unregister_family(&flp_genl_family);
	misc_deregister(&flp_miscdev);
	return 0;
}
EXPORT_SYMBOL_GPL(flp_unregister);
