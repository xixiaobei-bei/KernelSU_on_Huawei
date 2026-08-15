/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Contexthub share memory driver
 * Author: Huawei
 * Create: 2016-04-01
 */
#include <linux/version.h>
#include <linux/init.h>
#include <linux/notifier.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include "shmem.h"
#include "inputhub_api.h"
#include "common.h"
#include <libhwsecurec/securec.h>

#define SHMEM_AP_RECV_PHY_ADDR            DDR_SHMEM_CH_SEND_ADDR_AP
#define SHMEM_AP_RECV_PHY_SIZE            DDR_SHMEM_CH_SEND_SIZE
#define SHMEM_AP_SEND_PHY_ADDR            DDR_SHMEM_AP_SEND_ADDR_AP
#define SHMEM_AP_SEND_PHY_SIZE            DDR_SHMEM_AP_SEND_SIZE
#define SHMEM_INIT_OK                     (0x0aaaa5555)
#define MODULE_NAME                       "sharemem"
#define SHMEM_SMALL_PIECE_SZ              1024
enum {
	SHMEM_MSG_TYPE_NORMAL,
	SHMEM_MSG_TYPE_SHORT,
};

#define SHMEM_IMPROVEMENT_SHORT_BLK

static LIST_HEAD(shmem_client_list);
static DEFINE_MUTEX(shmem_recv_lock); /*lint !e651 !e708 !e570 !e64 !e785*/

struct shmem_ipc_data {
	unsigned int module_id;	/*enum is different between M7 & A53, so use "unsigned int" */
	unsigned int buf_size;
#ifdef SHMEM_IMPROVEMENT_SHORT_BLK
	unsigned int offset;
	int msg_type;
	int checksum;
	unsigned int priv;
#endif
};

struct shmem_ipc {
	pkt_header_t hd;
	struct shmem_ipc_data data;
};

struct shmem {
	unsigned int init_flag;
	void __iomem *recv_addr_base;
	void __iomem *send_addr_base;
	void __iomem *recv_addr;
	void __iomem *send_addr;
	struct semaphore send_sem;
};

struct receive_response_work_t {
	struct shmem_ipc_data data;
	struct work_struct worker;
};

struct workqueue_struct *g_receive_response_wq;
struct receive_response_work_t g_receive_response_work;
static struct shmem g_shmem_gov;
static struct wake_lock g_shmem_lock;

/*
 * IPC消息封装，调用inputhub的接口发送IPC消息
 */
static int shmem_ipc_send(unsigned char cmd, obj_tag_t module_id,
			  unsigned int size, bool is_lock)
{
	struct shmem_ipc pkt;
	write_info_t winfo;

	if (memset_s(&pkt, sizeof(pkt), 0, sizeof(pkt)) != EOK) {
		pr_err("%s memset_s fail\n", __func__);
	}
	pkt.data.module_id = module_id;
	pkt.data.buf_size = size;
#ifdef SHMEM_IMPROVEMENT_SHORT_BLK
	pkt.data.offset = 0;
	pkt.data.msg_type = SHMEM_MSG_TYPE_NORMAL;
	pkt.data.checksum = 0;
#endif
#ifdef CONFIG_INPUTHUB_20
	winfo.tag = TAG_SHAREMEM;
	winfo.cmd = cmd;
	winfo.wr_buf = &pkt.data;
	winfo.wr_len = sizeof(struct shmem_ipc_data);
	if (is_lock) {
		return write_customize_cmd(&winfo, NULL, is_lock);
	} else {
		pkt.hd.tag = TAG_SHAREMEM;
		pkt.hd.cmd = cmd;
		pkt.hd.length = sizeof(struct shmem_ipc_data);
		return inputhub_mcu_write_cmd(&pkt, sizeof(pkt)); //send msg no lock no resp
	}
#else
	winfo.tag = TAG_SHAREMEM;
	winfo.cmd = cmd;
	winfo.wr_buf = &pkt.data;
	winfo.wr_len = sizeof(struct shmem_ipc_data);
	return write_customize_cmd(&winfo, NULL);
#endif
}

#ifdef SHMEM_IMPROVEMENT_SHORT_BLK
/*
 * 计算数据包的checksum
 */
static int shmem_get_checksum(void *buf_addr, unsigned int buf_size)
{
	unsigned char *p = buf_addr;
	unsigned int sum = 0;
	if (!buf_addr || buf_size > SHMEM_SMALL_PIECE_SZ) {
		return -1;
	}

	while (buf_size) {
		sum += *p;
		p++;
		buf_size--;
	}
	return sum;
}
#endif

/*
 * 消息回复的workqueue, 发送IPC消息确认sharemem消息已处理
 */
static void receive_response_work_handler(struct work_struct *work)
{
	struct receive_response_work_t *p =
	    container_of(work, struct receive_response_work_t, worker); /*lint !e826*/
	if (p == NULL) {
		pr_err("%s NULL pointer\n", __func__);
		return;
	}
	shmem_ipc_send(CMD_SHMEM_AP_RECV_RESP, (obj_tag_t)p->data.module_id,
		       p->data.buf_size, false);
	pr_info("[%s]\n", __func__);
}

/*
 * 将sharemem包中的数据从DDR中取出，如果是大数据包消息同时启动workqueue回复IPC确认
 */
const pkt_header_t *shmempack(const char *buf, unsigned int length)
{
	int ret;
	struct shmem_ipc *msg = (struct shmem_ipc *)buf;
	static char recv_buf[SHMEM_AP_RECV_PHY_SIZE] = { 0, };
	const pkt_header_t *head = (const pkt_header_t *)recv_buf;

	if (NULL == buf)
		return NULL;

#ifdef SHMEM_IMPROVEMENT_SHORT_BLK
	if (msg->data.offset > SHMEM_AP_RECV_PHY_SIZE ||\
		msg->data.buf_size > SHMEM_AP_RECV_PHY_SIZE ||\
		msg->data.offset + msg->data.buf_size > SHMEM_AP_RECV_PHY_SIZE) {
		pr_err("[%s] data invalid; offset %x, len %x\n", __func__, msg->data.offset, msg->data.buf_size);
		return NULL;
	}

	g_shmem_gov.recv_addr = g_shmem_gov.recv_addr_base + msg->data.offset;

	ret = memcpy_s(recv_buf, sizeof(recv_buf), g_shmem_gov.recv_addr, (size_t)msg->data.buf_size);
	if (ret != EOK) {
		pr_err("[%s] memset_s fail[%d]\n", __func__, ret);
	}
	if (msg->data.module_id != head->tag) {
		pr_warn("[%s] module id invalid; %x, %x\n", __func__, (int)msg->data.module_id, (int)head->tag);
	}

	switch (msg->data.msg_type) {
	case SHMEM_MSG_TYPE_NORMAL:
		wake_lock_timeout(&g_shmem_lock, HZ / 2);
		ret = memcpy_s(&g_receive_response_work.data, sizeof(g_receive_response_work.data), &msg->data, sizeof(g_receive_response_work.data));
		if (ret != EOK) {
			pr_err("[%s] memset_s fail[%d]\n", __func__, ret);
		}
		queue_work(g_receive_response_wq, &g_receive_response_work.worker);
		break;
	case SHMEM_MSG_TYPE_SHORT:
		if (msg->data.checksum != shmem_get_checksum(g_shmem_gov.recv_addr, msg->data.buf_size)) {
			pr_err("[%s] checksum is invalid; module %x, tag %x\n", __func__, (int)msg->data.module_id, (int)head->tag);
			return NULL; /*git it up*/
		}
		break;
	default:
		pr_err("[%s] unknow msg type;\n", __func__);
		return NULL; /*git it up*/
	}
#else
	ret = memcpy_s(recv_buf, sizeof(recv_buf), g_shmem_gov.recv_addr_base, (size_t)msg->data.buf_size);
	if (ret != EOK) {
		pr_err("[%s] memset_s fail[%d]\n", __func__, ret);
	}
	ret = memcpy_s(&g_receive_response_work.data, sizeof(g_receive_response_work.data), &msg->data, sizeof(g_receive_response_work.data));
	if (ret != EOK) {
		pr_err("[%s] memset_s fail[%d]\n", __func__, ret);
	}
	queue_work(g_receive_response_wq, &g_receive_response_work.worker);
#endif
	return head;
} /*lint !e715*/

/*
 * sharemem接收模块初始化
 */
static int shmem_recv_init(void)
{
	g_receive_response_wq = alloc_ordered_workqueue("sharemem_receive_response", __WQ_LEGACY | WQ_MEM_RECLAIM | WQ_FREEZABLE);
	if (g_receive_response_wq == NULL) {
		pr_err("failed to create sharemem_receive_response workqueue\n");
		return -1;
	}

	g_shmem_gov.recv_addr_base =
	    ioremap_wc((ssize_t)SHMEM_AP_RECV_PHY_ADDR, (unsigned long)SHMEM_AP_RECV_PHY_SIZE);
	if (g_shmem_gov.recv_addr_base == NULL) {
		pr_err("[%s] ioremap err\n", __func__);
		return -ENOMEM;
	}

	INIT_WORK(&g_receive_response_work.worker, receive_response_work_handler);

	return 0;
}

#ifdef CONFIG_HISI_DEBUG_FS
#define SHMEM_TEST_TAG (TAG_END-1)
/*
 * sharemem测试代码，不商用
 */
void shmem_recv_test(const void __iomem *buf_addr, unsigned int size)
{
	pr_info("%s: get size %d, send back;\n", __func__, size);
	shmem_send(SHMEM_TEST_TAG, buf_addr, size);
}

int shmem_notify_test(const pkt_header_t *head)
{
	shmem_recv_test((void __iomem *)head, (unsigned int)(head->length + sizeof(pkt_header_t)));
	return 0;
}

int shmem_start_test(void)
{
	register_mcu_event_notifier(SHMEM_TEST_TAG, CMD_SHMEM_AP_RECV_REQ, shmem_notify_test);
	pr_info("%s: ok;\n", __func__);
	return 0;
}
// Add the following sentence here to enable test: late_initcall_sync(shmem_start_test);
#endif

/*
 * sharemem消息发送接口，将数据复制到DDR中，并发送IPC通知contexthub处理
 */
int shmem_send(obj_tag_t module_id, const void *usr_buf,
	       unsigned int usr_buf_size)
{
	int ret;
	if ((NULL == usr_buf) || (usr_buf_size > SHMEM_AP_SEND_PHY_SIZE)) {
		return -EINVAL;
	}
	if (SHMEM_INIT_OK != g_shmem_gov.init_flag) {
		return -EPERM;
	}
	ret = down_timeout(&g_shmem_gov.send_sem, (long)msecs_to_jiffies(500));
	if (ret != 0) {
		pr_warning("[%s]down_timeout 500\n", __func__);
	}
	ret = memcpy_s((void *)g_shmem_gov.send_addr_base, (size_t)SHMEM_AP_SEND_PHY_SIZE, usr_buf, (unsigned long)usr_buf_size);
	if (ret != EOK) {
		pr_err("[%s] memset_s fail[%d]\n", __func__, ret);
	}
	return shmem_ipc_send(CMD_SHMEM_AP_SEND_REQ, module_id, usr_buf_size, true);
}

/*
 * 获得sharemem数据发送的size上限
 */
unsigned int shmem_get_capacity(void)
{
	return (unsigned int)SHMEM_AP_SEND_PHY_SIZE;
}

/*
 * sharemem消息发送后，收到contexthub的回复确认后up信号量
 */
int shmem_send_resp(const pkt_header_t *head)
{
	if (!g_shmem_gov.send_sem.count) {
		up(&g_shmem_gov.send_sem);
	} else {
		pr_info("%s:%d\n", __func__, g_shmem_gov.send_sem.count);
	}
	return 0;
} /*lint !e715*/

/*
 * sharemem发送模块初始化
 */
static int shmem_send_init(void)
{
	g_shmem_gov.send_addr_base =
	    ioremap_wc((ssize_t)SHMEM_AP_SEND_PHY_ADDR, (unsigned long)SHMEM_AP_SEND_PHY_SIZE);
	if (g_shmem_gov.send_addr_base == NULL) {
		pr_err("[%s] ioremap err\n", __func__);
		return -ENOMEM;
	}

	sema_init(&g_shmem_gov.send_sem, 1);
	return 0;
}

/*
 * Contexthub sharemem驱动初始化
 */
int contexthub_shmem_init(void)
{
	int ret;
	ret = get_contexthub_dts_status();
	if (ret != 0) {
		return ret;
	}

	ret = shmem_recv_init();
	if (ret != 0) {
		return ret;
	}
	ret = shmem_send_init();
	if (ret != 0) {
		return ret;
	}
	g_shmem_gov.init_flag = SHMEM_INIT_OK;
	wake_lock_init(&g_shmem_lock, WAKE_LOCK_SUSPEND, "ch_shmem_lock");
	return ret;
}

/*lint -e753*/
MODULE_ALIAS("platform:contexthub" MODULE_NAME);
MODULE_LICENSE("GPL v2");

