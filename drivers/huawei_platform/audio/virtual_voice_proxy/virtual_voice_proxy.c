/*
 * virtual_voice_proxy.c
 *
 * virtual voice proxy driver
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */



#include "drv_mailbox_msg.h"
#include "bsp_drv_ipc.h"
#include "hifi_lpp.h"

#include "huawei_platform/audio/virtual_voice_proxy.h"

#include <linux/of_platform.h>
#include <linux/kthread.h>

#define DTS_COMP_VIR_VOICE_PROXY_NAME "hisilicon,virtual_voice_proxy"

#define TIME_OUT_MSEC 20
#define TIME_OUT_MSEC_PRINTF 21
#define MESSAGE_CALLBACKS_SIZE 10
#define COMMAND_CALLBACKS_SIZE 10
#define INIT_CALLBACKS_SIZE 10
#define TIME_OF_THOUSAND_FACTOR 1000

/* receive message from hifi, the size of msg_id (bytes) */
#define VOICE_PROXY_MSG_ID_SIZE 4

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif

/* this queue is used for sending data confirm message to hifi */
LIST_HEAD(virtual_proxy_confirm_queue);

/* this queue is used for tell the write thread that the type of new message */
LIST_HEAD(virtual_proxy_command_queue);

/* this queue is used for handle mail message */
LIST_HEAD(virtual_proxy_mail_msg_queue);

/* the message callback is used for handling message from hifi */
static struct voice_proxy_msg_handle message_callbacks[MESSAGE_CALLBACKS_SIZE];

/* the command callback is used for get data from data queue by command */
static struct voice_proxy_cmd_handle command_callbacks[COMMAND_CALLBACKS_SIZE];

/* the sign init callback is used for init write sign */
static struct voice_proxy_sign_init sign_init_callbacks[INIT_CALLBACKS_SIZE];

struct virtual_voice_proxy_priv {
	struct device *dev;
	/* this lock is used for handling the queue of command_queue */
	spinlock_t command_lock;
	spinlock_t cnf_queue_lock;
	spinlock_t mail_lock;
	wait_queue_head_t command_waitq;
	wait_queue_head_t mail_waitq;
	int32_t command_wait_flag;
	int32_t mail_wait_flag;
	struct workqueue_struct *send_mailbox_cnf_wq;
	struct work_struct send_mailbox_cnf_work;
	struct task_struct *write_thread;
	struct task_struct *mail_thread;
	mailbox_send_msg_cb send_mailbox_msg;
	read_mailbox_msg_cb read_mailbox_msg;
	struct virtual_voice_memory_pool command_pool;
};

static struct virtual_voice_proxy_priv porxy_priv;

int virtual_voice_get_free_node_num(struct virtual_voice_memory_pool *mem_pool)
{
	int count = 0;
	struct vir_free_node *free_node = NULL;

	if (mem_pool == NULL)
		return 0;

	free_node = mem_pool->free_node_header;
	while (free_node != NULL) {
		count++;
		free_node = free_node->next;
	}

	return count;
}

static int virtual_voice_malloc_mem_pool_block_data(
	struct vir_mem_block **mem_block, uint32_t size)
{
	int i;
	int index;
	struct vir_mem_block *new_block = NULL;

	if (mem_block == NULL || *mem_block == NULL)
		return -ENOMEM;

	new_block = *mem_block;
	for (i = 0; i < VIR_MEM_BLOCK_NUM; i++) {
		new_block->data[i].data = kzalloc(size, GFP_ATOMIC);
		if (new_block->data[i].data == NULL) {
			loge("vir_free_node data malloc failed\n");
			index = i - 1;
			while (index >= 0) {
				if (new_block->data[index].data != NULL) {
					kfree(new_block->data[index].data);
					new_block->data[index].data = NULL;
				}
				index--;
			}
			return -ENOMEM;
		}
		new_block->data[i].next = NULL;
	}

	for (i = 1; i < VIR_MEM_BLOCK_NUM; i++)
		new_block->data[i - 1].next = &new_block->data[i];

	new_block->data[VIR_MEM_BLOCK_NUM - 1].next = NULL;
	return 0;
}

void *virtual_voice_malloc(struct virtual_voice_memory_pool *mem_pool,
	uint32_t size, int *ret)
{
	struct vir_free_node *free_node = NULL;
	struct vir_mem_block *new_block = NULL;
	int temp;

	if (mem_pool == NULL || ret == NULL)
		return NULL;

	*ret = VIR_MEM_BLOCK_POOL_NOT_USE;
	if (size != mem_pool->elem_size) {
		logw("size:%u, is not equal to mem pool elem_size:%u\n",
			size, mem_pool->elem_size);
		return kzalloc(size, GFP_ATOMIC);
	}
	spin_lock_bh(&mem_pool->mem_lock);
	if (mem_pool->free_node_header == NULL) {
		if (mem_pool->block_num > VIT_MEM_BLOCK_MAXNUM_LIMIT) {
			loge("block_num is exceed limit\n");
			spin_unlock_bh(&mem_pool->mem_lock);
			return NULL;
		}
		new_block = kzalloc(sizeof(*new_block), GFP_ATOMIC);
		if (new_block == NULL) {
			loge("new_block malloc failed\n");
			spin_unlock_bh(&mem_pool->mem_lock);
			return NULL;
		}
		new_block->next = NULL;
		mem_pool->free_node_header = &new_block->data[0];
		temp = virtual_voice_malloc_mem_pool_block_data(&new_block,
			size);
		if (temp < 0) {
			if (new_block != NULL)
				kfree(new_block);
			spin_unlock_bh(&mem_pool->mem_lock);
			return NULL;
		}
		mem_pool->block_num++;
		if (mem_pool->mem_block_header == NULL) {
			mem_pool->mem_block_header = new_block;
		} else {
			new_block->next = mem_pool->mem_block_header;
			mem_pool->mem_block_header = new_block;
		}
	}
	*ret = VIR_MEM_BLOCK_POOL_USE;
	free_node = mem_pool->free_node_header;
	mem_pool->free_node_header = mem_pool->free_node_header->next;
	free_node->next = NULL;
	spin_unlock_bh(&mem_pool->mem_lock);
	return (void *)free_node;
}

void virtual_voice_free(struct virtual_voice_memory_pool *mem_pool,
	void **mem, int mem_status)
{
	struct vir_free_node *free_node = NULL;

	if (mem_pool == NULL || mem == NULL || *mem == NULL)
		return;

	if (mem_status == VIR_MEM_BLOCK_POOL_NOT_USE) {
		kfree(*mem);
		*mem = NULL;
		return;
	}

	free_node = (struct vir_free_node *)(*mem);
	spin_lock_bh(&mem_pool->mem_lock);
	free_node->next = mem_pool->free_node_header;
	mem_pool->free_node_header = free_node;
	*mem = NULL;
	spin_unlock_bh(&mem_pool->mem_lock);
}

void virtual_voice_memory_pool_free(struct virtual_voice_memory_pool *mem_pool)
{
	int i;
	struct vir_mem_block *mem_ptr = NULL;

	if (mem_pool == NULL)
		return;

	spin_lock_bh(&mem_pool->mem_lock);
	while (mem_pool->mem_block_header != NULL) {
		mem_ptr = mem_pool->mem_block_header->next;
		for (i = 0; i < VIR_MEM_BLOCK_NUM; i++) {
			if (mem_pool->mem_block_header->data[i].data == NULL)
				continue;

			kfree(mem_pool->mem_block_header->data[i].data);
			mem_pool->mem_block_header->data[i].data = NULL;
		}
		kfree(mem_pool->mem_block_header);
		mem_pool->mem_block_header = mem_ptr;
	}
	mem_pool->free_node_header = NULL;
	mem_pool->block_num = 0;
	spin_unlock_bh(&mem_pool->mem_lock);
}

void virtual_voice_memory_pool_init(struct virtual_voice_memory_pool *mem_pool,
	uint32_t elem_size)
{
	if (mem_pool == NULL)
		return;

	mem_pool->block_num = 0;
	mem_pool->free_node_header = NULL;
	mem_pool->mem_block_header = NULL;
	mem_pool->elem_size = elem_size;
	spin_lock_init(&mem_pool->mem_lock);
}

bool virtual_voice_memory_pool_free_node_full_idle(
	struct virtual_voice_memory_pool *mem_pool)
{
	int free_node_count;

	if (mem_pool == NULL)
		return false;

	free_node_count =
		virtual_voice_get_free_node_num(mem_pool);

	if ((free_node_count == mem_pool->block_num * VIR_MEM_BLOCK_NUM) &&
		(free_node_count > 0))
		return true;

	return false;

}

void virtual_voice_cmd_memory_pool_free(void)
{
	if (!list_empty_careful(&virtual_proxy_confirm_queue))
		return;

	if (!list_empty_careful(&virtual_proxy_command_queue))
		return;

	if (virtual_voice_memory_pool_free_node_full_idle(
		&porxy_priv.command_pool)) {
		logi("cmd mem pool clear\n");
		virtual_voice_memory_pool_free(&porxy_priv.command_pool);
	}
}

void virtual_voice_proxy_register_msg_callback(uint16_t msg_id,
	voice_proxy_msg_cb callback)
{
	int32_t i;

	if (callback == NULL) {
		loge("register_msg_callback fail, param callback is NULL\n");
		return;
	}

	for (i = 0; i < MESSAGE_CALLBACKS_SIZE; i++) {
		if (!message_callbacks[i].msg_id) {
			message_callbacks[i].msg_id = msg_id;
			message_callbacks[i].callback = callback;
			return;
		}
	}

	loge("register_msg_callback fail, message_callbacks is full\n");
}

void virtual_voice_proxy_deregister_msg_callback(uint16_t msg_id)
{
	int32_t i;

	for (i = 0; i < MESSAGE_CALLBACKS_SIZE; i++) {
		if (message_callbacks[i].msg_id == msg_id) {
			message_callbacks[i].msg_id = 0;
			message_callbacks[i].callback = NULL;
			return;
		}
	}

	loge("deregister_msg_callback fail, msg_id is invalid\n");
}

void virtual_voice_proxy_register_cmd_callback(uint16_t msg_id,
	voice_proxy_cmd_cb callback)
{
	int32_t i;

	if (callback == NULL) {
		loge("register_cmd_callback fail, param callback is NULL\n");
		return;
	}

	for (i = 0; i < COMMAND_CALLBACKS_SIZE; i++) {
		if (!command_callbacks[i].msg_id) {
			command_callbacks[i].msg_id = msg_id;
			command_callbacks[i].callback = callback;
			return;
		}
	}

	loge("register_cmd_callback fail, command_callbacks is full\n");
}

void virtual_voice_proxy_deregister_cmd_callback(uint16_t msg_id)
{
	int32_t i;

	for (i = 0; i < COMMAND_CALLBACKS_SIZE; i++) {
		if (command_callbacks[i].msg_id == msg_id) {
			command_callbacks[i].msg_id = 0;
			command_callbacks[i].callback = 0;
			return;
		}
	}

	loge("deregister_cmd_callback fail, invalid msg_id\n");
}

void virtual_voice_proxy_register_sign_init_callback(
	voice_proxy_sign_init_cb callback)
{
	int32_t i = 0;

	if (callback == NULL) {
		loge("register_sign_init_callback fail, callback is NULL\n");
		return;
	}

	for (i = 0; i < INIT_CALLBACKS_SIZE; i++) {
		if (sign_init_callbacks[i].callback == NULL) {
			sign_init_callbacks[i].callback = callback;
			return;
		}
	}

	loge("register init_callback fail, sign_init_callbacks is full\n");
}

void virtual_voice_proxy_deregister_sign_init_callback(
	voice_proxy_sign_init_cb callback)
{
	int32_t i = 0;

	if (callback == NULL) {
		loge("deregister init_callback, param callback is NULL\n");
		return;
	}

	for (i = 0; i < INIT_CALLBACKS_SIZE; i++) {
		if (sign_init_callbacks[i].callback == callback) {
			sign_init_callbacks[i].callback = NULL;
			return;
		}
	}

	loge("deregister init_callback fail, sign_init_callbacks is full");
}

int64_t virtual_voice_proxy_get_time_ms(void)
{
	int64_t timems;
	struct timeval time;

	do_gettimeofday(&time);
	timems = TIME_OF_THOUSAND_FACTOR * time.tv_sec +
		time.tv_usec / TIME_OF_THOUSAND_FACTOR;

	return timems;
}

void virtual_voice_proxy_set_send_sign(bool first,
	bool *cnf, int64_t *timestamp)
{
	int64_t cur_timestamp;
	int64_t spend_time;

	if (cnf == NULL || timestamp == NULL) {
		loge("set_send_sign fail, param is NULL\n");
		return;
	}

	if (first) {
		*timestamp = virtual_voice_proxy_get_time_ms();
	} else {
		cur_timestamp = virtual_voice_proxy_get_time_ms();
		spend_time = cur_timestamp - *timestamp;
		if (spend_time > TIME_OUT_MSEC) {
			if (spend_time > TIME_OUT_MSEC_PRINTF)
				logd("time spend %lld, it exceed limit %d\n",
					spend_time, TIME_OUT_MSEC);
			*cnf = 1;
		}
		*timestamp = cur_timestamp;
	}
}

static struct virtual_voice_proxy_cmd_node *virtual_voice_proxy_cmd_malloc(
	uint32_t size)
{
	int ret;
	void *ptr = NULL;
	struct virtual_voice_proxy_cmd_node *command = NULL;
	struct vir_free_node *mem_node = NULL;

	ptr = virtual_voice_malloc(&porxy_priv.command_pool,
		size, &ret);
	if (ptr == NULL) {
		loge("command malloc in pool failed\n");
		return NULL;
	}

	if (ret == VIR_MEM_BLOCK_POOL_NOT_USE) {
		command = ptr;
		/* only for virtual_voice_free use the node_cmd->mem_node */
		command->mem_node = ptr;
	} else {
		mem_node = ptr;
		command = (struct virtual_voice_proxy_cmd_node *)(mem_node->data);
		if (command == NULL) {
			loge("command is NULL\n");
			return NULL;
		}
		command->mem_node = mem_node;
	}
	command->mem_status = ret;
	return command;
}

static void virtual_voice_cmd_node_free(
	struct virtual_voice_memory_pool *mem_pool,
	struct virtual_voice_proxy_cmd_node **command)
{
	struct virtual_voice_proxy_cmd_node *node_cmd = NULL;

	if (command == NULL || *command == NULL || mem_pool == NULL)
		return;

	node_cmd = *command;
	virtual_voice_free(mem_pool,
		(void **)(&node_cmd->mem_node),
		node_cmd->mem_status);
}

int32_t virtual_voice_proxy_add_cmd(uint16_t msg_id)
{

	struct virtual_voice_proxy_cmd_node *command =
		virtual_voice_proxy_cmd_malloc(sizeof(*command));
	if (command == NULL) {
		loge("command malloc failed\n");
		return -ENOMEM;
	}

	command->msg_id = msg_id;
	spin_lock_bh(&porxy_priv.command_lock);
	list_add_tail(&command->list_node, &virtual_proxy_command_queue);
	porxy_priv.command_wait_flag++;
	spin_unlock_bh(&porxy_priv.command_lock);
	wake_up(&porxy_priv.command_waitq);
	return 0;
}


int32_t virtual_voice_proxy_add_data(voice_proxy_add_data_cb callback,
	int8_t *data, uint32_t size, uint16_t msg_id)
{
	int32_t ret;
	struct virtual_voice_proxy_cmd_node *command = NULL;

	if (callback == NULL || data == NULL) {
		loge("proxy_add_data fail, param is NULL\n");
		return -EINVAL;
	}

	command = virtual_voice_proxy_cmd_malloc(sizeof(*command));
	if (command == NULL) {
		loge("command malloc failed\n");
		return -ENOMEM;
	}

	command->msg_id = msg_id;
	spin_lock_bh(&porxy_priv.command_lock);
	ret = callback(data, size);
	if (ret < 0) {
		spin_unlock_bh(&porxy_priv.command_lock);
		virtual_voice_cmd_node_free(&porxy_priv.command_pool,
			(struct virtual_voice_proxy_cmd_node **)(&command));
		command = NULL;
		return ret;
	}

	list_add_tail(&command->list_node, &virtual_proxy_command_queue);
	porxy_priv.command_wait_flag++;
	spin_unlock_bh(&porxy_priv.command_lock);
	wake_up(&porxy_priv.command_waitq);
	return ret;
}

static int32_t send_mailbox_cnf_msg(uint16_t msg_id)
{
	int32_t ret = 0;
	struct virtual_voice_proxy_confirm cmd_cnf = {0};

	cmd_cnf.msg_id = msg_id;
	if (porxy_priv.send_mailbox_msg != NULL) {
		/* call the mailbox to send the message to hifi */
		ret = porxy_priv.send_mailbox_msg(
			MAILBOX_MAILCODE_ACPU_TO_HIFI_VIRTUAL_VOICE,
			msg_id,
			(void *)&cmd_cnf,
			(uint32_t)sizeof(cmd_cnf));
	} else {
		loge("send_mailbox_msg is null\n");
		ret = -EINVAL;
	}

	return ret;
}

static void vir_voc_send_mailbox_cnf_work_queue(struct work_struct *work)
{
	int32_t ret = 0;
	struct virtual_voice_proxy_cmd_node *command = NULL;

	UNUSED_PARAMETER(work);

	spin_lock_bh(&porxy_priv.cnf_queue_lock);
	while (!list_empty_careful(&virtual_proxy_confirm_queue)) {
		command = list_first_entry(&virtual_proxy_confirm_queue,
					struct virtual_voice_proxy_cmd_node,
					list_node);
		if (command == NULL) {
			loge("command is NULL\n");
			spin_unlock_bh(&porxy_priv.cnf_queue_lock);
			return;
		}

		list_del_init(&command->list_node);
		spin_unlock_bh(&porxy_priv.cnf_queue_lock);

		ret = send_mailbox_cnf_msg(command->msg_id);
		if (ret != MAILBOX_OK)
			loge("send mailbox cnf msg fail\n");

		virtual_voice_cmd_node_free(&porxy_priv.command_pool,
			(struct virtual_voice_proxy_cmd_node **)(&command));
		command = NULL;

		spin_lock_bh(&porxy_priv.cnf_queue_lock);
	}
	spin_unlock_bh(&porxy_priv.cnf_queue_lock);
}

int32_t virtual_voice_proxy_add_work_queue_cmd(uint16_t msg_id)
{
	struct virtual_voice_proxy_cmd_node *command =
		virtual_voice_proxy_cmd_malloc(sizeof(*command));

	if (command == NULL) {
		loge("command malloc fail\n");
		return -ENOMEM;
	}
	command->msg_id = msg_id;

	spin_lock_bh(&porxy_priv.cnf_queue_lock);
	list_add_tail(&command->list_node, &virtual_proxy_confirm_queue);
	spin_unlock_bh(&porxy_priv.cnf_queue_lock);

	if (!queue_work(porxy_priv.send_mailbox_cnf_wq,
		&porxy_priv.send_mailbox_cnf_work))
		loge("msg_id 0x%x no send mailbox cnf queue work\n", msg_id);

	return 0;
}

int32_t virtual_voice_proxy_mailbox_send_msg_cb(uint32_t mailcode,
	uint16_t msg_id, const void *buf, uint32_t size)
{
	int32_t ret;

	UNUSED_PARAMETER(msg_id);
	if (buf == NULL) {
		loge("mailbox_send_msg_cb fail, param buf is NULL\n");
		return -EINVAL;
	}

	ret = (int)mailbox_send_msg((size_t)mailcode, buf, (size_t)size);
	if (ret)
		loge("mailbox_send_msg fail\n");

	return ret;
}

static int32_t virtual_voice_read_mailbox_msg_cb(void *mail_handle,
		int8_t *buf, int32_t *size)
{
	int32_t ret;

	if (mail_handle == NULL || buf == NULL || size == NULL) {
		loge("mailbox_msg_data_cb fail, param is NULL!\n");
		return -EINVAL;
	}

	ret = (int)mailbox_read_msg_data(mail_handle,
		(char *)buf, (unsigned int *)size);
	if (ret)
		loge("mailbox_read_msg_data fail\n");

	return ret;
}

/* the interrupt handle function for receiving mailbox data */
static void handle_voice_mail(const void *usr_para,
		struct mb_queue *mail_handle, uint32_t mail_len)
{
	int32_t ret_mail;
	struct virtual_voice_proxy_rev_msg rev_msg = {0};
	unsigned int mail_size = mail_len;
	struct virtual_voice_proxy_cmd_node *mail_msg = NULL;

	UNUSED_PARAMETER(usr_para);
	if (mail_handle == NULL) {
		loge("mail_handle is NULL\n");
		return;
	}

	if (porxy_priv.read_mailbox_msg != NULL) {
		ret_mail = porxy_priv.read_mailbox_msg(mail_handle,
			(unsigned char *)&rev_msg, &mail_size);
	} else {
		loge("read_mailbox_msg is null\n");
		return;
	}

	if (ret_mail != MAILBOX_OK) {
		loge("read mailbox msg fail! ret=0x%x\n",
			(unsigned int)ret_mail);
		return;
	}

	if ((mail_size == 0) || (mail_size > sizeof(rev_msg))) {
		loge("data length error! size: %d, sizeof(rev_msg):%lu\n",
				mail_size, sizeof(rev_msg));
		return;
	}

	mail_msg = virtual_voice_proxy_cmd_malloc(sizeof(*mail_msg));
	if (mail_msg == NULL) {
		loge("mail_msg malloc failed\n");
		return;
	}
	spin_lock_bh(&porxy_priv.mail_lock);
	memcpy(&mail_msg->rev_msg, &rev_msg, sizeof(rev_msg));
	list_add_tail(&mail_msg->list_node, &virtual_proxy_mail_msg_queue);
	porxy_priv.mail_wait_flag++;
	spin_unlock_bh(&porxy_priv.mail_lock);
	wake_up(&porxy_priv.mail_waitq);
}

static int32_t register_mailbox_msg_cb(mb_msg_cb callback)
{
	int32_t ret;

	ret = (int32_t)mailbox_reg_msg_cb(
		(size_t)MAILBOX_MAILCODE_HIFI_TO_ACPU_VIRTUAL_VOICE,
		callback, NULL);

	if (ret)
		loge("hifi mailbox handle func register fail\n");

	return ret;
}

static int32_t virtual_voice_proxy_mailbox_cb_init(void)
{
	int32_t ret;

	porxy_priv.send_mailbox_msg = virtual_voice_proxy_mailbox_send_msg_cb;
	porxy_priv.read_mailbox_msg =
		(read_mailbox_msg_cb)virtual_voice_read_mailbox_msg_cb;
	ret = register_mailbox_msg_cb(handle_voice_mail);

	return ret;
}

static void virtual_voice_proxy_mailbox_cb_deinit(void)
{
	porxy_priv.send_mailbox_msg = NULL;
	porxy_priv.read_mailbox_msg = NULL;

	register_mailbox_msg_cb(NULL);
}

static void voice_write_sign_init(void)
{
	int32_t i;

	for (i = 0; i < INIT_CALLBACKS_SIZE; i++) {
		if (sign_init_callbacks[i].callback)
			sign_init_callbacks[i].callback();
	}
}

/* this function will get data from callback which register by other module */
static int32_t write_thread_get_data(uint32_t *size, uint16_t *msg_id)
{
	int32_t ret = 0;
	int32_t i;
	struct virtual_voice_proxy_cmd_node *command = NULL;

	if (size == NULL || msg_id == NULL) {
		loge("write thread get_data fail, param is NULL\n");
		return -EINVAL;
	}

	if (list_empty_careful(&virtual_proxy_command_queue)) {
		logw("proxy_command_queue is empty\n");
		return -EINVAL;
	}

	command = list_first_entry(&virtual_proxy_command_queue,
		struct virtual_voice_proxy_cmd_node, list_node);
	if (command == NULL) {
		loge("command is NULL\n");
		return -EINVAL;
	}
	for (i = 0; i < COMMAND_CALLBACKS_SIZE; i++) {
		if (command_callbacks[i].msg_id == command->msg_id) {
			command_callbacks[i].callback(size, msg_id);
			break;
		}
	}

	if (i == COMMAND_CALLBACKS_SIZE) {
		loge("write thread get data fail, invalid msg id:0x%x:\n",
			command->msg_id);
		ret = -EINVAL;
	}

	if (*size > VOICE_PROXY_LIMIT_PARAM_SIZE) {
		loge("data size error, size:%d > %d\n",
			*size, VOICE_PROXY_LIMIT_PARAM_SIZE);
		ret = -EINVAL;
	}
	list_del_init(&command->list_node);
	virtual_voice_cmd_node_free(&porxy_priv.command_pool,
		(struct virtual_voice_proxy_cmd_node **)(&command));
	command = NULL;
	return ret;
}

/* send the voice data to hifi */
static int virtual_voice_proxy_write_thread(void *arg)
{
	int32_t ret = 0;
	uint16_t msg_id = 0;
	uint32_t data_size = 0;

	UNUSED_PARAMETER(arg);

	voice_write_sign_init();

	while (!kthread_should_stop()) {
		ret = wait_event_interruptible(porxy_priv.command_waitq,
			porxy_priv.command_wait_flag != 0);
		if (ret) {
			loge("wait event failed, 0x%x.\n", ret);
			continue;
		}

		spin_lock_bh(&porxy_priv.command_lock);
		porxy_priv.command_wait_flag = 0;

		while (!list_empty_careful(&virtual_proxy_command_queue)) {
			data_size = VOICE_PROXY_LIMIT_PARAM_SIZE;

			/* voice data in not in shared memory */
			ret = write_thread_get_data(&data_size, &msg_id);

			spin_unlock_bh(&porxy_priv.command_lock);

			if ((ret) || (!data_size)) {
				spin_lock_bh(&porxy_priv.command_lock);
				continue;
			}

			ret = send_mailbox_cnf_msg(msg_id);
			if (ret)
				loge("send_mailbox_msg fail, ret:%d\n", ret);

			spin_lock_bh(&porxy_priv.command_lock);
		}
		spin_unlock_bh(&porxy_priv.command_lock);
	}

	return 0;
}

/* handle mail message */
static int virtual_voice_proxy_handle_mail_thread(void *arg)
{
	int32_t ret;
	uint16_t msg_id;
	uint32_t i;
	struct virtual_voice_proxy_rev_msg rev_msg;
	struct virtual_voice_proxy_cmd_node *mail_msg = NULL;

	UNUSED_PARAMETER(arg);
	while (!kthread_should_stop()) {
		ret = wait_event_interruptible(porxy_priv.mail_waitq,
			porxy_priv.mail_wait_flag != 0);
		if (ret) {
			loge("wait event failed, 0x%x.\n", ret);
			continue;
		}

		spin_lock_bh(&porxy_priv.mail_lock);
		porxy_priv.mail_wait_flag = 0;
		while (!list_empty_careful(&virtual_proxy_mail_msg_queue)) {
			mail_msg = list_first_entry(&virtual_proxy_mail_msg_queue,
				struct virtual_voice_proxy_cmd_node, list_node);
			if (mail_msg == NULL) {
				loge("mail_msg is NULL\n");
				break;
			}
			memcpy(&rev_msg, &mail_msg->rev_msg, sizeof(rev_msg));
			msg_id = mail_msg->rev_msg.msg_id;
			list_del_init(&mail_msg->list_node);
			virtual_voice_cmd_node_free(&porxy_priv.command_pool,
				(struct virtual_voice_proxy_cmd_node **)(&mail_msg));
			spin_unlock_bh(&porxy_priv.mail_lock);
			for (i = 0; i < MESSAGE_CALLBACKS_SIZE; i++) {
				if (message_callbacks[i].msg_id == msg_id) {
					message_callbacks[i].callback(
						(int8_t *)&rev_msg,
						(uint32_t)sizeof(rev_msg));
					break;
				}
			}

			if (i == MESSAGE_CALLBACKS_SIZE)
				loge("handle mail msg_id is invalid\n");

			spin_lock_bh(&porxy_priv.mail_lock);
		}
		spin_unlock_bh(&porxy_priv.mail_lock);
	}
	return 0;
}

static int32_t virtual_voice_proxy_create_thread(void)
{
	porxy_priv.write_thread = kthread_run(virtual_voice_proxy_write_thread,
			NULL, "virtual_voice proxy write");

	if (IS_ERR_OR_NULL(porxy_priv.write_thread)) {
		loge("write_thread call kthread_run fail\n");
		return -EBUSY;
	}

	porxy_priv.mail_thread = kthread_run(
		virtual_voice_proxy_handle_mail_thread,
		NULL, "virtual_voice handle mail thread");

	if (IS_ERR_OR_NULL(porxy_priv.mail_thread)) {
		loge("mail_thread call kthread_run fail\n");
		return -EBUSY;
	}
	return 0;
}

static void destroy_thread(void)
{
	if (!IS_ERR_OR_NULL(porxy_priv.write_thread)) {
		kthread_stop(porxy_priv.write_thread);
		spin_lock_bh(&porxy_priv.command_lock);
		porxy_priv.command_wait_flag++;
		spin_unlock_bh(&porxy_priv.command_lock);
		wake_up(&porxy_priv.command_waitq);
		porxy_priv.write_thread = NULL;
	}

	if (!IS_ERR_OR_NULL(porxy_priv.mail_thread)) {
		kthread_stop(porxy_priv.mail_thread);
		spin_lock_bh(&porxy_priv.mail_lock);
		porxy_priv.mail_wait_flag++;
		spin_unlock_bh(&porxy_priv.mail_lock);
		wake_up(&porxy_priv.mail_waitq);
		porxy_priv.mail_thread = NULL;
	}

}

static int virtual_voice_proxy_probe(struct platform_device *pdev)
{
	int32_t ret;

	memset(&porxy_priv, 0, sizeof(porxy_priv));
	porxy_priv.dev = &pdev->dev;
	porxy_priv.command_wait_flag = 0;
	virtual_voice_memory_pool_init(&porxy_priv.command_pool,
		sizeof(struct virtual_voice_proxy_cmd_node));

	spin_lock_init(&porxy_priv.command_lock);
	spin_lock_init(&porxy_priv.mail_lock);
	spin_lock_init(&porxy_priv.cnf_queue_lock);
	init_waitqueue_head(&porxy_priv.command_waitq);
	init_waitqueue_head(&porxy_priv.mail_waitq);

	porxy_priv.send_mailbox_cnf_wq =
		create_singlethread_workqueue("vir_voice_send_mailbox_cnf_wq");
	if (porxy_priv.send_mailbox_cnf_wq == NULL) {
		loge("workqueue create failed\n");
		return -EFAULT;
	}

	INIT_WORK(&porxy_priv.send_mailbox_cnf_work,
		vir_voc_send_mailbox_cnf_work_queue);

	ret = virtual_voice_proxy_mailbox_cb_init();
	if (ret) {
		loge("hifi mailbox handle func register fail\n");
		goto mailbox_cb_init_err;
	}

	ret = virtual_voice_proxy_create_thread();
	if (ret) {
		loge("voice_proxy_create_thread fail\n");
		goto thread_create_err;
	}

	of_platform_populate(pdev->dev.of_node, NULL, NULL, &pdev->dev);

	return (int)ret;

thread_create_err:
	destroy_thread();
	virtual_voice_proxy_mailbox_cb_deinit();
mailbox_cb_init_err:
	if (porxy_priv.send_mailbox_cnf_wq != NULL) {
		flush_workqueue(porxy_priv.send_mailbox_cnf_wq);
		destroy_workqueue(porxy_priv.send_mailbox_cnf_wq);
		porxy_priv.send_mailbox_cnf_wq = NULL;
	}
	return (int)ret;
}

static int virtual_voice_proxy_remove(struct platform_device *pdev)
{
	UNUSED_PARAMETER(pdev);

	destroy_thread();
	virtual_voice_proxy_mailbox_cb_deinit();
	virtual_voice_cmd_memory_pool_free();

	if (porxy_priv.send_mailbox_cnf_wq != NULL) {
		flush_workqueue(porxy_priv.send_mailbox_cnf_wq);
		destroy_workqueue(porxy_priv.send_mailbox_cnf_wq);
		porxy_priv.send_mailbox_cnf_wq = NULL;
	}

	return 0;
}


static const struct of_device_id virtual_voice_proxy_match_table[] = {
	{
		.compatible = DTS_COMP_VIR_VOICE_PROXY_NAME,
		.data = NULL,
	},
	{}
};

static struct platform_driver virtual_voice_proxy_driver = {
	.driver = {
		.name = "virtual_voice proxy",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(virtual_voice_proxy_match_table),
	},
	.probe = virtual_voice_proxy_probe,
	.remove = virtual_voice_proxy_remove,
};

static int __init virtual_voice_proxy_init(void)
{
	int ret;

	ret = platform_driver_register(&virtual_voice_proxy_driver);
	if (ret)
		loge("virtual voice proxy register fail,ERROR is %d\n", ret);

	return ret;
}

static void __exit virtual_voice_proxy_exit(void)
{
	platform_driver_unregister(&virtual_voice_proxy_driver);
}

module_init(virtual_voice_proxy_init);
module_exit(virtual_voice_proxy_exit);

MODULE_DESCRIPTION("virtual_voice_proxy driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");

