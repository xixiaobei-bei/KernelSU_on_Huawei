/*
 * voice_proxy.c - HW voice proxy in kernel, it is used for pass through
 * voice data between AP and hifi.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include "voice_proxy.h"

#include <linux/of_platform.h>
#include <linux/kthread.h>

#include "drv_mailbox_msg.h"
#include "bsp_drv_ipc.h"

/*lint -e528 -e753 -e754*/
#define LOG_TAG "voice_proxy"
#define TIME_OUT_MSEC 20
#define MESSAGE_CALLBACKS_SIZE 20
#define COMMAND_CALLBACKS_SIZE 20
#define SIGN_INIT_CALLBACKS_SIZE 20

/* receive message from hifi£¬the size of msg_id (bytes ) */
#define VOICE_PROXY_MSG_ID_SIZE (4)

#define DTS_COMP_VOICE_PROXY_NAME "hisilicon,voice_proxy"

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif

struct hifi_msg_para {
	uint16_t msg_id;
};

/* handle the reply message from hifi,record msg_id and data */
struct voice_proxy_rev_msg_buf {
	int8_t *mail_buf; /* received data buff from mailbox */
	int32_t mail_buf_len;
	int32_t msg_id; /* the first 4 bytes is msg_id in the received data from mailbox */
	int8_t *out_buf_ptr; /* pointer to the behind of cmd_id */
	int32_t out_buf_len;
};

/* this queue is used for sending data confirm message to hifi */
LIST_HEAD(confirm_queue);

/*
 * this queue is used for telling the write thread that the type
 * of new message
 */
LIST_HEAD(proxy_command_queue);

/* the message callback is used for handling message from hifi */
static struct voice_proxy_msg_handle message_callbacks[MESSAGE_CALLBACKS_SIZE];

/* the command callback is used for get data from data queue by command */
static struct voice_proxy_cmd_handle command_callbacks[COMMAND_CALLBACKS_SIZE];

/* the sign init callback is used for init write sign */
static struct voice_proxy_sign_init sign_init_callbacks[SIGN_INIT_CALLBACKS_SIZE];

struct voice_proxy_priv {
	struct device *dev;
	/* this lock is used for handling the queue of proxy_command_queue */
	spinlock_t command_lock;
	spinlock_t cnf_queue_lock;

	wait_queue_head_t command_waitq;

	int32_t command_wait_flag;

	struct workqueue_struct *send_mailbox_cnf_wq;
	struct work_struct send_mailbox_cnf_work;

	struct task_struct *write_thread;

	mailbox_send_msg_cb send_mailbox_msg;
	read_mailbox_msg_cb read_mailbox_msg;
};

static struct voice_proxy_priv priv;

void voice_proxy_register_msg_callback(uint16_t msg_id, voice_proxy_msg_cb callback)
{
	int32_t i;

	if (!callback) {
		AUDIO_LOGE("register_msg_callback fail, param callback is NULL");
		return;
	}

	for (i = 0; i < MESSAGE_CALLBACKS_SIZE; i++) {
		if (!message_callbacks[i].msg_id) {
			message_callbacks[i].msg_id = msg_id;
			message_callbacks[i].callback = callback;
			return;
		}
	}

	AUDIO_LOGE("register_msg_callback fail, message_callbacks is full");
}

void voice_proxy_deregister_msg_callback(uint16_t msg_id)
{
	int32_t i;

	for (i = 0; i < MESSAGE_CALLBACKS_SIZE; i++) {
		if (message_callbacks[i].msg_id == msg_id) {
			message_callbacks[i].msg_id = 0;
			message_callbacks[i].callback = 0;
			return;
		}
	}

	AUDIO_LOGE("deregister_msg_callback fail, msg_id is invalid");
}

void voice_proxy_register_cmd_callback(uint16_t msg_id, voice_proxy_cmd_cb callback)
{
	int32_t i;

	if (!callback) {
		AUDIO_LOGE("register_cmd_callback fail, param callback is NULL");
		return;
	}

	for (i = 0; i < COMMAND_CALLBACKS_SIZE; i++) {
		if (!command_callbacks[i].msg_id) {
			command_callbacks[i].msg_id = msg_id;
			command_callbacks[i].callback = callback;
			return;
		}
	}

	AUDIO_LOGE("register_cmd_callback fail, command_callbacks is full");
}

void voice_proxy_deregister_cmd_callback(uint16_t msg_id)
{
	int32_t i;

	for (i = 0; i < COMMAND_CALLBACKS_SIZE; i++) {
		if (command_callbacks[i].msg_id == msg_id) {
			command_callbacks[i].msg_id = 0;
			command_callbacks[i].callback = 0;
			return;
		}
	}

	AUDIO_LOGE("deregister_cmd_callback fail, invalid msg_id");
}

void voice_proxy_register_sign_init_callback(voice_proxy_sign_init_cb callback)
{
	int32_t i = 0;

	if (!callback) {
		AUDIO_LOGE("register_sign_init_callback fail, param callback is NULL");
		return;
	}

	for (i = 0; i < SIGN_INIT_CALLBACKS_SIZE; i++) {
		if (!sign_init_callbacks[i].callback) {
			sign_init_callbacks[i].callback = callback;
			return;
		}
	}

	AUDIO_LOGE("register_sign_init_callback fail, sign_init_callbacks is full");
}

void voice_proxy_deregister_sign_init_callback(voice_proxy_sign_init_cb callback)
{
	int32_t i = 0;

	if (!callback) {
		AUDIO_LOGE("deregister_sign_init_callback, param callback is NULL");
		return;
	}

	for (i = 0; i < SIGN_INIT_CALLBACKS_SIZE; i++) {
		if (sign_init_callbacks[i].callback == callback) {
			sign_init_callbacks[i].callback = NULL;
			return;
		}
	}

	AUDIO_LOGE("deregister_sign_init_callback fail, sign_init_callbacks is full");
}

static void write_sign_init(void)
{
	int32_t i;

	for (i = 0; i < SIGN_INIT_CALLBACKS_SIZE; i++) {
		if (sign_init_callbacks[i].callback)
			sign_init_callbacks[i].callback();
	}
}

int64_t voice_proxy_get_time_ms(void)
{
	int64_t timems;
	struct timeval time;

	do_gettimeofday(&time);
	timems = 1000 * time.tv_sec + time.tv_usec / 1000;

	return timems;
}

void voice_proxy_set_send_sign(bool first, bool *cnf, int64_t *timestamp)
{
	int64_t cur_timestamp = 0;

	if (!cnf || !timestamp) {
		AUDIO_LOGE("set_send_sign fail, param is NULL");
		return;
	}

	if (first) {
		*timestamp = voice_proxy_get_time_ms();
	} else {
		cur_timestamp = voice_proxy_get_time_ms();
		if (cur_timestamp - *timestamp > TIME_OUT_MSEC) {
			*cnf = 1;
			*timestamp = cur_timestamp;
		}
	}
}

int32_t voice_proxy_create_data_node(struct voice_proxy_data_node **node,
	int8_t *data, int32_t size)
{
	struct voice_proxy_data_node *n = NULL;

	if (node == NULL || data == NULL) {
		AUDIO_LOGE("input parameter invalid");
		return -EINVAL;
	}

	n = kzalloc(sizeof(*n) + size, GFP_ATOMIC); /* lint !e737 */
	if (n == NULL) {
		AUDIO_LOGE("kzalloc failed");
		return -ENOMEM;
	}

	memcpy(n->list_data.data, data, size); /* lint !e732 !e747 */ /* unsafe_function_ignore: memcpy */
	n->list_data.size = size;
	*node = n;

	return 0;
}

int32_t voice_proxy_mailbox_send_msg_cb(uint32_t mailcode, uint16_t msg_id, const void *buf, uint32_t size)
{
	int32_t ret;

	if (!buf) {
		AUDIO_LOGE("mailbox_send_msg_cb fail, param buf is NULL");
		return -EINVAL;
	}

	UNUSED_PARAMETER(msg_id);

	ret = (int)mailbox_send_msg((size_t)mailcode, buf, (size_t)size);
	if (ret)
		AUDIO_LOGE("mailbox_send_msg fail");

	return ret;
}

static int32_t read_mailbox_msg_data_cb(struct mb_queue *mail_handle,
	int8_t *buf, int32_t *size)
{
	int32_t ret;

	if (!mail_handle || !buf || !size) {
		AUDIO_LOGE("mailbox_msg_data_cb fail, param is NULL");
		return -EINVAL;
	}

	ret = (int)mailbox_read_msg_data(mail_handle, (char *)buf, (unsigned int *)size);
	if (ret)
		AUDIO_LOGE("mailbox_read_msg_data fail");

	return ret;
}

static int32_t send_mailbox_cnf_msg(uint16_t msg_id, uint16_t modem_no, uint32_t channel_id)
{
	int32_t ret = 0;
	struct voice_proxy_confirm cmd_cnf;

	memset(&cmd_cnf, 0, sizeof(cmd_cnf)); /* unsafe_function_ignore: memset  */
	cmd_cnf.msg_id = msg_id;
	cmd_cnf.modem_no = modem_no;
	cmd_cnf.channel_id = channel_id;
	if (priv.send_mailbox_msg) {
		/* call the mailbox to send the message to hifi */
		ret = priv.send_mailbox_msg(MAILBOX_MAILCODE_ACPU_TO_HIFI_VOICE_RT,
			msg_id, (void *)&cmd_cnf, (uint32_t)sizeof(cmd_cnf));
	} else {
		AUDIO_LOGE("send_mailbox_msg is null");
		ret = -EINVAL;
	}

	return ret;
}

static void send_mailbox_cnf_work_queue(struct work_struct *work)
{
	int32_t ret = 0;
	struct proxy_voice_cnf_cmd_code *command = NULL;

	UNUSED_PARAMETER(work);

	spin_lock_bh(&priv.cnf_queue_lock);
	while (!list_empty_careful(&confirm_queue)) {
		command = list_first_entry(&confirm_queue, struct proxy_voice_cnf_cmd_code, list_node); /* lint !e826 */

		list_del_init(&command->list_node);
		spin_unlock_bh(&priv.cnf_queue_lock);

		ret = send_mailbox_cnf_msg(command->msg_id, command->modem_no, command->channel_id);
		if (ret)
			AUDIO_LOGE("send mailbox cnf msg fail");

		kfree(command);
		command = NULL;

		spin_lock_bh(&priv.cnf_queue_lock);
	}
	spin_unlock_bh(&priv.cnf_queue_lock);
}

int32_t voice_proxy_add_work_queue_cmd(uint16_t msg_id, uint16_t modem_no, uint32_t channel_id)
{
	struct proxy_voice_cnf_cmd_code *command;

	command = kzalloc(sizeof(*command), GFP_ATOMIC);
	if (command == NULL) {
		AUDIO_LOGE("command kzalloc fail");
		return -ENOMEM;
	}

	command->msg_id = msg_id;
	command->modem_no = modem_no;
	command->channel_id = channel_id;

	spin_lock_bh(&priv.cnf_queue_lock);
	list_add_tail(&command->list_node, &confirm_queue);
	spin_unlock_bh(&priv.cnf_queue_lock);

	if (!queue_work(priv.send_mailbox_cnf_wq, &priv.send_mailbox_cnf_work))
		AUDIO_LOGE("msg_id 0x%x no send mailbox cnf queue work", msg_id);

	return 0; /*lint !e429*/
}

int32_t voice_proxy_add_data(voice_proxy_add_data_cb callback, int8_t *data, uint32_t size, uint16_t msg_id)
{
	int32_t ret;
	struct voice_proxy_cmd_node *command = NULL;

	if (!callback || !data) {
		AUDIO_LOGE("proxy_add_data fail, param is NULL");
		return -EINVAL;
	}

	command = kzalloc(sizeof(*command), GFP_ATOMIC);
	if (command == NULL) {
		AUDIO_LOGE("kzalloc failed");
		return -ENOMEM;
	}

	command->msg_id = msg_id;

	spin_lock_bh(&priv.command_lock);
	ret = callback(data, size);
	if (ret < 0) {
		spin_unlock_bh(&priv.command_lock);
		kfree(command);
		return ret;
	}

	list_add_tail(&command->list_node, &proxy_command_queue);
	priv.command_wait_flag++;
	spin_unlock_bh(&priv.command_lock);
	wake_up(&priv.command_waitq);

	return ret; /*lint !e429*/
}

int32_t voice_proxy_add_cmd(uint16_t msg_id)
{
	struct voice_proxy_cmd_node *command;

	command = kzalloc(sizeof(*command), GFP_ATOMIC);
	if (command == NULL) {
		AUDIO_LOGE("command kzalloc failed");
		return -ENOMEM;
	}

	command->msg_id = msg_id;
	spin_lock_bh(&priv.command_lock);

	list_add_tail(&command->list_node, &proxy_command_queue);
	priv.command_wait_flag++;

	spin_unlock_bh(&priv.command_lock);
	wake_up(&priv.command_waitq);

	return 0; /*lint !e429*/
}

/*
 * this function will get data from callback which register by other module,
 * the size is in&out param, as a in param,it show the size of data,
 * as a out param, it show the substantial data size which get from callback.
 */
static int32_t write_thread_get_data(int8_t *data, uint32_t *size, uint16_t *msg_id)
{
	int32_t ret = 0;
	int32_t i;
	struct voice_proxy_cmd_node *command = NULL;

	if (!data || !size || !msg_id) {
		AUDIO_LOGE("input param is NULL");
		return -EINVAL;
	}

	if (list_empty_careful(&proxy_command_queue)) {
		AUDIO_LOGE("proxy_command_queue is empty");
		return -EINVAL;
	}

	command = list_first_entry(&proxy_command_queue, struct voice_proxy_cmd_node, list_node); /* lint !e826 */

	for (i = 0; i < COMMAND_CALLBACKS_SIZE; i++) {
		if (command_callbacks[i].msg_id == command->msg_id) {
			command_callbacks[i].callback(data, size, msg_id);
			break;
		}
	}

	if (i == COMMAND_CALLBACKS_SIZE) {
		AUDIO_LOGE("invalid msg id:0x%x", command->msg_id);
		ret = -EINVAL;
	}

	if (*size > VOICE_PROXY_LIMIT_PARAM_SIZE) {
		AUDIO_LOGE("data size error, size(%d)>(%d)", *size, VOICE_PROXY_LIMIT_PARAM_SIZE);
		ret = -EINVAL;
	}

	list_del_init(&command->list_node);
	kfree(command);
	command = NULL;

	return ret; /* lint !e438 */
}

/*
 * send the voice data to hifi
 */
static int voice_proxy_write_thread(void *arg)
{
	int32_t ret = 0;
	uint16_t msg_id = 0;
	uint32_t mailcode = 0;
	uint32_t size = VOICE_PROXY_LIMIT_PARAM_SIZE;
	int8_t *data = NULL;

	UNUSED_PARAMETER(arg);

	data = kzalloc((size_t)size, GFP_ATOMIC);
	if (!data) {
		AUDIO_LOGE("kzalloc failed");
		return -ENOMEM;
	}

	write_sign_init();

	while (!kthread_should_stop()) {
		ret = wait_event_interruptible(priv.command_waitq, priv.command_wait_flag != 0);
		if (ret) {
			if (ret != -ERESTARTSYS)
				AUDIO_LOGE("wait event interruptible fail, 0x%x", ret);
			else
				AUDIO_LOGE("wait event failed,ERESTARTSYS");
			continue;
		}

		spin_lock_bh(&priv.command_lock);
		priv.command_wait_flag = 0;

		while (!list_empty_careful(&proxy_command_queue)) {
			size = VOICE_PROXY_LIMIT_PARAM_SIZE;

			ret = write_thread_get_data(data, &size, &msg_id);

			spin_unlock_bh(&priv.command_lock);

			if ((ret) || (!size)) {
				spin_lock_bh(&priv.command_lock);
				continue;
			}

			mailcode = (msg_id == ID_PROXY_VOICE_LTE_TX_NTF) ?
				MAILBOX_MAILCODE_ACPU_TO_HIFI_VOICE :
				MAILBOX_MAILCODE_ACPU_TO_HIFI_VOICE_RT;

			if (priv.send_mailbox_msg)
				ret = priv.send_mailbox_msg(mailcode, msg_id, data, size);
			else
				AUDIO_LOGE("send_mailbox_msg is null");

			if (ret)
				AUDIO_LOGE("send_mailbox_msg fail, ret:%d", ret);

			spin_lock_bh(&priv.command_lock);
		}
		spin_unlock_bh(&priv.command_lock);
	}

	kfree(data);
	return 0; /* lint !e438 */
}

/*
 * the interrupt handle function for receiving mailbox data
 */
static bool is_input_param_valid(struct mb_queue *mail_handle, uint32_t mail_len)
{
	if (mail_handle == NULL) {
		AUDIO_LOGE("mail_handle is NULL");
		return false;
	}

	/* mail len must not less msg id size, it is 4 byte */
	if (mail_len < VOICE_PROXY_MSG_ID_SIZE || mail_len > VOICE_PROXY_LIMIT_PARAM_SIZE) {
		AUDIO_LOGE("param error, mail len: %d", mail_len);
		return false;
	}

	return true;
}

static void handle_mail(const void *usr_para, struct mb_queue *mail_handle, uint32_t mail_len)
{
	int32_t i = 0;
	int32_t ret_mail = 0;
	uint16_t msg_id = 0;
	struct voice_proxy_rev_msg_buf *rev_msg = NULL;
	struct hifi_msg_para *msg_para = NULL;

	if (!is_input_param_valid(mail_handle, mail_len))
		return;

	rev_msg = kzalloc(sizeof(*rev_msg), GFP_ATOMIC);
	if (rev_msg == NULL) {
		AUDIO_LOGE("rev_msg kzalloc failed");
		return;
	}

	/*
	 * mail_buf_len is a in para and a out para, the mail_len show the size
	 * which the mailbox request to read. the function of mailbox_read_msg_data
	 * need a preplanned allocated buffer to copy the mailbox buffer, and
	 * return the practical size of buffer which copy from mailbox by
	 * mail_buf_len.
	 */
	rev_msg->mail_buf_len = (int)mail_len;

	rev_msg->mail_buf = kzalloc((size_t)VOICE_PROXY_LIMIT_PARAM_SIZE, GFP_ATOMIC);
	if (rev_msg->mail_buf == NULL) {
		AUDIO_LOGE("malloc mail_buf failed");
		goto EXIT;
	}

	if (priv.read_mailbox_msg) {
		ret_mail = priv.read_mailbox_msg(mail_handle, rev_msg->mail_buf, &rev_msg->mail_buf_len);
	} else {
		AUDIO_LOGE("read_mailbox_msg is null");
		goto EXIT;
	}

	if (ret_mail != MAILBOX_OK) {
		AUDIO_LOGE("read mailbox msg fail! ret=0x%x", (unsigned int)ret_mail);
		goto EXIT;
	}

	msg_para = (struct hifi_msg_para *)rev_msg->mail_buf; /* lint !e826 */
	msg_id = msg_para->msg_id;

	for (i = 0; i < MESSAGE_CALLBACKS_SIZE; i++) {
		if (message_callbacks[i].msg_id == msg_id) {
			message_callbacks[i].callback((int8_t *)rev_msg->mail_buf, (uint32_t)rev_msg->mail_buf_len);
			break;
		}
	}

	if (i == MESSAGE_CALLBACKS_SIZE)
		AUDIO_LOGE("callback fail, msg_id is invalid");

EXIT:
	if (rev_msg) {
		if (rev_msg->mail_buf) {
			kfree(rev_msg->mail_buf);
			rev_msg->mail_buf = NULL;
		}
		kfree(rev_msg);
		rev_msg = NULL;
	}
}

static int32_t register_mailbox_msg_cb(mb_msg_cb callback)
{
	int32_t ret;

	ret = (int)mailbox_reg_msg_cb((size_t)MAILBOX_MAILCODE_HIFI_TO_ACPU_VOICE, callback, NULL);

	return ret;
}

void voice_proxy_mailbox_cb_register(mailbox_send_msg_cb send_cb,
	read_mailbox_msg_cb read_cb, register_mailbox_cb register_cb)
{
	if (send_cb)
		priv.send_mailbox_msg = send_cb;

	if (read_cb)
		priv.read_mailbox_msg = read_cb;

	if (register_cb)
		register_cb((mb_msg_cb)handle_mail);
}

static int32_t voice_proxy_mailbox_cb_init(void)
{
	int32_t ret;

	priv.send_mailbox_msg = voice_proxy_mailbox_send_msg_cb;
	priv.read_mailbox_msg = (read_mailbox_msg_cb)read_mailbox_msg_data_cb;
	ret = register_mailbox_msg_cb(handle_mail);

	return ret;
}

static void voice_proxy_mailbox_cb_deinit(void)
{
	priv.send_mailbox_msg = NULL;
	priv.read_mailbox_msg = NULL;

	register_mailbox_msg_cb(NULL);
}

static void destory_thread(void)
{
	if (!IS_ERR(priv.write_thread)) {
		kthread_stop(priv.write_thread);
		spin_lock_bh(&priv.command_lock);
		priv.command_wait_flag++;
		spin_unlock_bh(&priv.command_lock);
		wake_up(&priv.command_waitq);
		priv.write_thread = NULL;
	}
}

static int32_t voice_proxy_create_thread(void)
{
	int32_t ret = 0;

	priv.write_thread = kthread_run(voice_proxy_write_thread, NULL, "voice proxy write");

	if (IS_ERR(priv.write_thread)) {
		AUDIO_LOGE("call kthread_run fail");
		ret = -EBUSY;
	}

	return ret;
}

static int voice_proxy_probe(struct platform_device *pdev)
{
	int32_t ret;

	memset(&priv, 0, sizeof(priv)); /* unsafe_function_ignore: memset */

	priv.dev = &pdev->dev;

	priv.command_wait_flag = 0;

	spin_lock_init(&priv.command_lock);
	spin_lock_init(&priv.cnf_queue_lock);
	init_waitqueue_head(&priv.command_waitq);

	priv.send_mailbox_cnf_wq = create_singlethread_workqueue("send_mailbox_cnf_wq");
	if (!priv.send_mailbox_cnf_wq) {
		AUDIO_LOGE("workqueue create failed");
		return -EFAULT;
	}

	INIT_WORK(&priv.send_mailbox_cnf_work, send_mailbox_cnf_work_queue);

	ret = voice_proxy_mailbox_cb_init();
	if (ret) {
		AUDIO_LOGE("hifi mailbox handle func register fail");
		goto ERR;
	}

	ret = voice_proxy_create_thread();
	if (ret) {
		AUDIO_LOGE("voice_proxy_create_thread fail");
		goto ERR1;
	}

	of_platform_populate(pdev->dev.of_node, NULL, NULL, &pdev->dev);

	return ret;

ERR1:
	voice_proxy_mailbox_cb_deinit();
ERR:
	if (priv.send_mailbox_cnf_wq) {
		flush_workqueue(priv.send_mailbox_cnf_wq);
		destroy_workqueue(priv.send_mailbox_cnf_wq);
		priv.send_mailbox_cnf_wq = NULL;
	}

	return ret;
}

static int voice_proxy_remove(struct platform_device *pdev)
{
	UNUSED_PARAMETER(pdev);

	destory_thread();
	voice_proxy_mailbox_cb_deinit();

	if (priv.send_mailbox_cnf_wq) {
		flush_workqueue(priv.send_mailbox_cnf_wq);
		destroy_workqueue(priv.send_mailbox_cnf_wq);
		priv.send_mailbox_cnf_wq = NULL;
	}

	return 0;
}

static const struct of_device_id hifi_voice_proxy_match_table[] = {
	{
		.compatible = DTS_COMP_VOICE_PROXY_NAME,
		.data = NULL,
	},
	{
	} /* lint !e785 */
};

static struct platform_driver voice_proxy_driver = {
	.driver = {
		.name  = "voice proxy",
		.owner = THIS_MODULE, /* lint !e64 */
		.of_match_table = of_match_ptr(hifi_voice_proxy_match_table),
	}, /* lint !e785 */
	.probe = voice_proxy_probe,
	.remove = voice_proxy_remove,
}; /* lint !e785 */

static int __init voice_proxy_init(void)
{
	int32_t ret;

	ret = platform_driver_register(&voice_proxy_driver); /* lint !e64 */
	if (ret)
		AUDIO_LOGE("voice proxy driver register fail,ERROR is %d", ret);

	return ret;
}

static void __exit voice_proxy_exit(void)
{
	platform_driver_unregister(&voice_proxy_driver);
}

module_init(voice_proxy_init);
module_exit(voice_proxy_exit);

MODULE_DESCRIPTION("voice_proxy driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL");
