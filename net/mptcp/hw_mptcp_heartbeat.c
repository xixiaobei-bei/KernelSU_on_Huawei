/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
* Description: MPTCP heartbeat implementation
*	heartbeat to MP-GW will only be triggered when no data is transfered in a period of time
*	mechanism may need to be improved, currently we use 2 threads
*	one thread for msg handling
*	one thread for heartbeat reply reception
*	due to msg-handling design, heartbeat might not be stopped timely
* Author: jiangwenhao jiangwenhao1@huawei.com
* Create: 2019-01-04
*/

#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/ip.h>
#include <linux/skbuff.h>
#include <linux/inetdevice.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/syscalls.h>
#include <linux/socket.h>
#include <linux/kernel.h>
#include <linux/inet.h>
#include <net/mptcp.h>
#include <net/sock.h>
#include <net/hw_mptcp_heartbeat.h>

/* marco and constant definitions */
#define HB_LOGE(fmt, args...)					\
	do \
		pr_err("mpheartbeat : " fmt, ##args);	\
	while (0)

#define HB_LOGD(fmt, args...)					\
	do	\
		pr_err("mpheartbeat : " fmt, ##args);	\
	while (0)

#define CHECK_MSG_TYPE(x) ((x > MIN_MSG) && (x < MAX_MSG))
#define CHECK_DETECTION_TYPE(x) (((x) & LINK_MASK) > 0)
#define CHECK_DETECTION_RESULT(x) ((x == LINK_OK) || (x == LINK_BROKEN))
#define CONVERT_BITSHIFT_TO_MSG(bitshift) (bitshift) // due to the tricky design
#define LINK_MASK ((1 << LINK_NUM) - 1)

#define HEARTBEAT_REQUEST 11
#define HEARTBEAT_RESPONSE 12

#define HEARTBEAT_DEFAULT_RETRY_INTERVAL 1
#define HEARTBEAT_DEFAULT_TIMEOUT 5 // in second

#define INVALID_VALUE (-1)
#define MAX_BUFF_SIZE 200

/* structure definitions */
typedef struct __reception_thread_parameters {
	struct socket *sock;
	char *buffer;
	int interval;
	int retry_count;
} reception_thread_parameters;

typedef enum __heartbeat_queue_msg_type {
	MIN_MSG = -1,
/* it is a little tricky here, remember to set the values in accordance with detection_type */
	WIFI_HEARTBEAT_TRIGGERED,
	LTE_HEARTBEAT_TRIGGERED,
	STOP, // put STOP above MAX_MSG
	MAX_MSG,
} heartbeat_queue_msg_type;

typedef struct __heartbeat_msg {
	heartbeat_queue_msg_type msg_type;
	void* para;
	struct __heartbeat_msg* next;
} heartbeat_msg;

/* global variables */
static struct sockaddr_in g_link_addrs[LINK_NUM];
static bool g_heartbeat_initialized  = false;

static spinlock_t g_heartbeat_parameter_lock; // lock g_link_addrs
static heartbeat_msg* g_msg_queue = NULL;
static struct semaphore g_msg_queue_sema;
static spinlock_t g_msg_queue_lock;

static struct task_struct* g_heartbeat_msg_task = NULL;

static const char* IF_NAMES[LINK_NUM] = IF_NAME_LIST;

/* functions */
static int heartbeat_msg_loop(void* para);
static void handle_msg(heartbeat_msg* msg);
static int detect(unsigned int type, int retry_count, int timeout, char* payload, int payload_size);
static void report_detection_result(unsigned int type, int result,
		heartbeat_detection_callback cb);
static void insert_msg(int msg_type, const void* para, int size, bool prior);
static void clear_msg_queue(void);
static int prepare_socket(struct socket* sock, struct sockaddr_in* dest, const char* ifname);
static int bind_to_device(struct socket* sock, const char* ifname);
static int wait_for_reply(struct socket* sock, int wait_time);

/*
 *	initialization function, called by emcom kernel module
 *	input param : link_addrs the addresses of all links, size LINK_NUM
 *	            retry_count,
 */
void heartbeat_init(struct sockaddr_in* link_addrs)
{
	if (!link_addrs) {
		HB_LOGE("illegal parameters, init failed!");
		return;
	}
	if (g_heartbeat_initialized) {
		HB_LOGE("try to reinitialize heartbeat, abort");
		return;
	}

	/* init locks */
	spin_lock_init(&g_heartbeat_parameter_lock); // essentially sets lock to UNLOCK status
	sema_init(&g_msg_queue_sema, 0);
	spin_lock_init(&g_msg_queue_lock);

	/* record the parameters */
	spin_lock_bh(&g_heartbeat_parameter_lock);
	memcpy(g_link_addrs, link_addrs, sizeof(g_link_addrs));
	spin_unlock_bh(&g_heartbeat_parameter_lock);

	/* start the heartbeat thread */
	if (!g_heartbeat_msg_task)
		g_heartbeat_msg_task = kthread_run(heartbeat_msg_loop, NULL, "heartbeat_process");

	g_heartbeat_initialized = true;

	HB_LOGD("heartbeat initialized!");
}
EXPORT_SYMBOL(heartbeat_init); //lint !e580

/*
 *	de-initialization function, called by emcom kernel module
 *    intuitively this is needed
 */
void heartbeat_deinit(void)
{
	if (!g_heartbeat_initialized) {
		HB_LOGE("try to deinit an un-initialized heartbeat,abort");
		return;
	}
	spin_lock_bh(&g_heartbeat_parameter_lock);
	memset(g_link_addrs, 0, sizeof(g_link_addrs));
	spin_unlock_bh(&g_heartbeat_parameter_lock);

	/* the msg thread CANNOT be stopped by kthread_stop() since it would block this thread */
	/* so send a stop msg to the queue */
	insert_msg(STOP, NULL, 0, true); // put STOP msg in the head
	g_heartbeat_msg_task = NULL;
	g_heartbeat_initialized = false;
}
EXPORT_SYMBOL(heartbeat_deinit); //lint !e580

/*
 *	trigger one heartbeat detection, called by emcom kernel module
 *	input param :
 *		type, can be a combination of multiple links (each bit indicates a link)
 *		para, detection parameters
 */
void heartbeat_trigger(unsigned int type, detection_para* para)
{
	unsigned int bit_shift = 0;
	if (!CHECK_DETECTION_TYPE(type) || !para) {
		HB_LOGE("illegal detection type or null para in heartbeat_trigger()");
		return;
	}
	if (!g_heartbeat_initialized) {
		HB_LOGE("try to trigger an un-initialized heartbeat, abort");
		return;
	}

	/* no need to record this type */
	/* this loop design applies even if LINK_NUM increases */
	for (bit_shift = 0; bit_shift < LINK_NUM; bit_shift++) {
		if (((type >> bit_shift) & 1) == 1)
			insert_msg(CONVERT_BITSHIFT_TO_MSG(bit_shift),
					(void*)para, sizeof(detection_para), false);
	}

	HB_LOGD("heartbeat triggered with type %d", type);
}
EXPORT_SYMBOL(heartbeat_trigger); //lint !e580

/*
 *	heartbeat thread entry function
 */
static int heartbeat_msg_loop(void* para)
{
	heartbeat_msg* current_msg = NULL;
	HB_LOGD("entered heartbeat_msg_loop");

	/* message loop */
	while(true) { // should NOT be stopped by kthread_should_stop()
		/* if all messages are handled, sleep and wait for new message */
		HB_LOGD("try to lock in heartbeat_msg_loop");
		spin_lock_bh(&g_msg_queue_lock);
		while(!g_msg_queue) {
			spin_unlock_bh(&g_msg_queue_lock);
			down(&g_msg_queue_sema);
			HB_LOGD("wake up in heartbeat_msg_loop");
			spin_lock_bh(&g_msg_queue_lock);
		}
		/* at this point, the lock is obtained */
		if (g_msg_queue->msg_type == STOP) {
			HB_LOGD("heartbeat_msg_loop handles a STOP msg");
			spin_unlock_bh(&g_msg_queue_lock); // note this, do not cause dead lock
			break;
		}
		current_msg = g_msg_queue;
		g_msg_queue = g_msg_queue->next;
		HB_LOGD("try to unlock in heartbeat_msg_loop");
		spin_unlock_bh(&g_msg_queue_lock);
		handle_msg(current_msg);
		kfree(current_msg); // free this message, otherwise memery leak may occur
	}

	clear_msg_queue();
	HB_LOGD("heartbeat_msg_loop exits gracefully");
	return 0;
}

/*
 *	handle messages in msg queue
 */
static void handle_msg(heartbeat_msg* msg)
{
	heartbeat_queue_msg_type type = MIN_MSG;
	void* para = NULL;
	int status = INVALID_VALUE;
	detection_para* detection_param = NULL;

	if (!msg) {
		HB_LOGE("null msg in handle_msg");
		return;
	}
	type = msg->msg_type;
	para = msg->para;
	HB_LOGD("about to handle a msg %d", msg->msg_type);
	switch(type) {
	case WIFI_HEARTBEAT_TRIGGERED:
		detection_param = para;
		status = detect(WIFI_DETECTION, detection_param->retry_count,
					detection_param->timeout, detection_param->payload,
					detection_param->payload_len);
		report_detection_result(WIFI_DETECTION, status, detection_param->cb_fun);
		kfree(detection_param);
		break;
	case LTE_HEARTBEAT_TRIGGERED:
		detection_param = para;
		status = detect(LTE_DETECTION, detection_param->retry_count,
					detection_param->timeout, detection_param->payload,
					detection_param->payload_len);
		report_detection_result(LTE_DETECTION, status, detection_param->cb_fun);
		kfree(detection_param);
		break;
	default:
		HB_LOGE("impossible branch in handle_msg");
		break;
	}
}

/*
 *	report the detection result to the callback function, no matter the link is broken or not
 */
static void report_detection_result(unsigned int link, int result,
		heartbeat_detection_callback cb_fun)
{
	if (!CHECK_DETECTION_TYPE(link) || !CHECK_DETECTION_RESULT(result) || !cb_fun) {
		HB_LOGE("illegal parameter in report_detection_result");
		return;
	}

	(*cb_fun)(link, result);
	HB_LOGD("reported a heartbeat detection, type %d, result %d, callback %p",
			link, result, cb_fun);
}

/*
 *	insert message to the tail of the msg queue
 */
static void insert_msg(int msg_type, const void* para, int size, bool prior)
{
	heartbeat_msg* msg = NULL;
	heartbeat_msg* search = NULL;
	void* para_copy = NULL;
	if (!CHECK_MSG_TYPE(msg_type)) {
		HB_LOGE("illegal msg type %d", msg_type);
		return;
	}

	if (para) {
		if (size <= 0) {
			HB_LOGE("insert_msg received a non-null para and a non-positive size");
			return;
		}
		/* if this message has a para, copy it since the message will be handled in another thread */
		para_copy = (void*)kmalloc(size, GFP_KERNEL);
		if (!para_copy) {
			HB_LOGE("error in kmalloc for para_copy");
			return;
		}
		memcpy(para_copy, para, size); // remember to free para_copy when its process is done
	}

	msg = (heartbeat_msg*)kmalloc(sizeof(heartbeat_msg), GFP_KERNEL);
	if (!msg) {
		HB_LOGE("error in kmalloc for msg");
		if (para_copy)
			kfree(para_copy);
		return;
	}
	msg->msg_type = msg_type;
	msg->para = para_copy;
	msg->next = NULL;

	/* to the head of the queue */
	if (prior) {
		spin_lock_bh(&g_msg_queue_lock);
		if (!g_msg_queue) {
			g_msg_queue = msg;
			spin_unlock_bh(&g_msg_queue_lock);
			up(&g_msg_queue_sema); // remeber to trigger the sema
			HB_LOGD("inserted a msg %d to the head of the queue", msg_type);
			return;
		} else {
			msg->next = g_msg_queue->next;
			g_msg_queue = msg;
			spin_unlock_bh(&g_msg_queue_lock);
			up(&g_msg_queue_sema); // remeber to trigger the sema
			HB_LOGD("inserted a msg %d to the head of the queue", msg_type);
			return;
		}
	}

	/* to the tail of the queue */
	spin_lock_bh(&g_msg_queue_lock);
	if (!g_msg_queue) {
		g_msg_queue = msg;
		spin_unlock_bh(&g_msg_queue_lock);
		up(&g_msg_queue_sema); // remeber to trigger the sema
		HB_LOGD("inserted a msg %d to the tail of the queue", msg_type);
		return;
	}
	/* search for the tail and insert */
	search = g_msg_queue;
	while(search->next)
		search = search->next;
	search->next = msg;
	spin_unlock_bh(&g_msg_queue_lock);
	up(&g_msg_queue_sema); // remeber to trigger the sema
	HB_LOGD("inserted a msg %d to the tail of the queue", msg_type);
}

/*
 *	clear the msg queue, should only be called when the msg loop is exited
 */
static void clear_msg_queue(void)
{
	heartbeat_msg* search = NULL;
	spin_lock_bh(&g_msg_queue_lock);
	search = g_msg_queue;
	while (g_msg_queue) {
		search = g_msg_queue;
		g_msg_queue = g_msg_queue->next;
		if (search->para)
			kfree(search->para);
		kfree(search);
	}
	spin_unlock_bh(&g_msg_queue_lock);
	HB_LOGD("cleared the msg queue");
}

/*
 *	detect the corresponding link, one at a time
 *	currently type is either WIFI_DETECTION (value 1) or LTE_DETECTION (value 2)
 */
static int detect(unsigned int type, int retry_count, int timeout, char* payload, int payload_size)
{
	int status = INVALID_VALUE;
	long err = INVALID_VALUE;
	int len = 0;
	int index = 0;
	struct sockaddr_in dest_addr = {0};
	const char* ifname = NULL;
	struct sockaddr_in link_addrs[LINK_NUM];
	struct socket* sock = NULL;
	struct kvec vec;
	struct msghdr msg;

	if ((retry_count < 0) || (timeout <= 0) || !payload || (payload_size <= 0)) {
		HB_LOGE("illegal parameters in detect");
		return INVALID_VALUE;
	}
	/* it is neccessary to check the type first */
	if ((type < 0) || (type > LINK_MASK) || (type % 2 != 0 && type != 1) ) {
		HB_LOGE("illgal type in detect(), abort");
		return INVALID_VALUE;
	}
	/* read the global parameters */
	spin_lock_bh(&g_heartbeat_parameter_lock);
	memcpy(link_addrs, g_link_addrs, sizeof(struct sockaddr_in)*LINK_NUM);
	spin_unlock_bh(&g_heartbeat_parameter_lock);

	/* fill the dest_addr according to the type */
	for (index = 0; index < LINK_NUM; index++) {
		if (((type >> index) & 1) == 1) {
			memcpy(&dest_addr, &link_addrs[index], sizeof(dest_addr));
			ifname = IF_NAMES[index];
			break;
		}
	}

	/* prepare msg to be sent */
	vec.iov_base = payload;
	vec.iov_len = payload_size;
	memset(&msg,0,sizeof(msg));
	msg.msg_name = &dest_addr;
	msg.msg_namelen = sizeof(dest_addr);

	/* prepare socket */
	HB_LOGD("about to create socket");
	err = sock_create_kern(&init_net, AF_INET, SOCK_DGRAM, IPPROTO_UDP,&sock);
	if (err != 0) {
		HB_LOGE("failed to create socket, error code %ld", err);
		return INVALID_VALUE;
	}
	err = prepare_socket(sock, &dest_addr, ifname);
	if (err != 0) {
		HB_LOGE("failed to prepare socket, error code %ld", err);
		sock_release(sock);
		return INVALID_VALUE;
	}

	/* start to send 1 heartbeat packet */
	HB_LOGD("about to send first heartbeat packet to destination");
	len = kernel_sendmsg(sock, &msg, &vec, 1, payload_size);
	if (payload_size != len) {
		HB_LOGE("kernel_sendmsg() error, length is %d", len);
		sock_release(sock);
		return INVALID_VALUE;
	}
	/* wait for reply */
	status = wait_for_reply(sock, timeout);
	if (status == LINK_OK) {
		HB_LOGD("heartbeat triggered and LINK_OK detected, link type %d", type);
		sock_release(sock);
		return status;
	}

	HB_LOGD("result of first detection is LINK_BROKEN");
	status = INVALID_VALUE;
	/* if the first heartbeat failed, do retry count times more */
	/* continuously send multiple heartbeat packets */
	/* however, abort once a reply is received by the reception thread */
	index = 0;
	len = 0;
	while (index < retry_count) {
		len = kernel_sendmsg(sock, &msg, &vec, 1, payload_size);
		HB_LOGD("sent a heartbeat to destination, index %d, total retry %d, error code %ld",
				index, retry_count, err);
		/* wait HEARTBEAT_DEFAULT_RETRY_INTERVAL second */
		status = wait_for_reply(sock, timeout);
		if (status == LINK_OK) {
			HB_LOGD("heartbeat triggered and LINK_OK detected, link %d, retry index %d",
					type, index);
			sock_release(sock);
			return status;
		}
		index++;
	}
	HB_LOGD("the heartbeat detection result in final stage is %d", status);
	sock_release(sock);

	return status;
}

/*
 *	udp heartbeat socket preparation
 *	input : socket_fd, ifname
 *	returns error code, 0 if succeeded
 */
static int prepare_socket(struct socket* sock, struct sockaddr_in* dest, const char* ifname)
{
	int err = INVALID_VALUE;
	if (!sock || !ifname || !dest) {
		HB_LOGE("null sock, ifname or dest addr in prepare_socket(), critical!");
		return INVALID_VALUE;
	}

	sock->sk->sk_reuse = 1;

	/* bind to device */
	err = bind_to_device(sock, ifname);
	if (err < 0) {
		HB_LOGE("bind to %s err, err=%d\n", ifname, err);
		return INVALID_VALUE;
	}

	return 0;
}

static int bind_to_device(struct socket* sock, const char* ifname)
{
	int err;
	if (!sock || !ifname) {
		HB_LOGE("illegal parameter in bind_to_device");
		return INVALID_VALUE;
	}

	err = kernel_setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (char*)ifname, strlen(ifname));
	if (err != 0) {
		HB_LOGE("failed to bind device, err %d", err);
		return INVALID_VALUE;
	}
	return 0;
}

/*
 *	receive reply
 *	input: socket_fd, ifname
 *	return: LINK_OK if reply received,
 *			LINK_BROKEN if time out,
 *			INVALID_VALUE if errors occur
 */
static int wait_for_reply(struct socket* sock, int wait_time)
{
	char buffer[MAX_BUFF_SIZE];
	int err = INVALID_VALUE;
	int recv_len = 0;
	int ret = INVALID_VALUE;
	struct timeval timeout = {0, 0};
	struct kvec vec;
	struct msghdr msg;

	if ((!sock) || (wait_time <= 0)) {
		HB_LOGE("illegal parameters in reception_thread(), CRITICAL");
		return INVALID_VALUE;
	}
	memset(&vec,0,sizeof(vec));
	memset(&msg,0,sizeof(msg));
	vec.iov_base = buffer;
	vec.iov_len = MAX_BUFF_SIZE;
	/* wait for reply */
	HB_LOGD("wait for reply %d seconds", wait_time);
	timeout.tv_sec = wait_time;
	err = kernel_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
	if (err != 0) {
		HB_LOGE("failed to set socket option");
		return INVALID_VALUE;
	}
	recv_len = kernel_recvmsg(sock, &msg, &vec, 1, MAX_BUFF_SIZE, 0);
	HB_LOGD("received a reply, recv_len is %d", recv_len);
	if (recv_len > 0) {
		HB_LOGD("received a reply in reception_thread()");
		/* a validation should be applied */
		ret = LINK_OK;
	} else {
		HB_LOGD("reception_thread() timeout! no reply received");
		ret = LINK_BROKEN;
	}
	return ret;
}
