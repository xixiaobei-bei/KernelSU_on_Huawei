/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: contexthub fmk implement.
 * Create: 2019/11/25
 */
#include "inputhub_wrapper.h"

#include <securec.h>

#include <linux/slab.h>
#include <linux/module.h>

#include "common.h"

#ifndef CONFIG_INPUTHUB_AS
int inputhub_wrapper_register_event_notifier(int tag, int cmd,
				int (*notify)(const struct pkt_header *head))
{
	// param check in register_mcu_event_notifier
	return register_mcu_event_notifier(tag, cmd, notify);
}

int inputhub_wrapper_unregister_event_notifier(int tag, int cmd,
				int (*notify)(const struct pkt_header *head))
{
	// param check in unregister_mcu_event_notifier
	return unregister_mcu_event_notifier(tag, cmd, notify);
}

int inputhub_wrapper_send_cmd(int app_tag, int cmd, const void *buf, int buf_size, struct read_info *rd)
{
	// param check in write_customize_cmd
	struct write_info wr;
	(void)memset_s(&wr, sizeof(wr), 0, sizeof(wr));

	wr.tag = app_tag;
	wr.cmd = cmd;
	wr.wr_buf = buf;
	wr.wr_len = buf_size;

	return write_customize_cmd(&wr, rd, false);
}

int inputhub_wrapper_get_app_tag(const struct pkt_header *head)
{
	if (head == NULL)
		return TAG_END;

	return head->tag;
}

#else // CONFIG_INPUTHUB_AS

struct inputhub_wrapper_notifier_node {
	int tag;
	int cmd;
	int subcmd;
	int (*match)(const struct pkt_header *data);
	int (*notify)(const struct pkt_header *data);
	struct list_head entry;
};

struct inputhub_wrapper_notifier_list {
	struct list_head head;
};

static DEFINE_MUTEX(notifier_list_lock);
static struct inputhub_wrapper_notifier_list g_inputhub_wrapper_list;
static int g_is_notifier_list_inited;

static void inputhub_wrapper_list_init(void)
{
	mutex_lock(&notifier_list_lock);
	if (g_is_notifier_list_inited == 0) {
		g_is_notifier_list_inited = 1;
		INIT_LIST_HEAD(&g_inputhub_wrapper_list.head);
	}
	mutex_unlock(&notifier_list_lock);
}

/*lint -e613 -e668*/
int inputhub_wrapper_register_event_notifier_ex(
				int (*match)(const struct pkt_header *head),
				int (*notify)(const struct pkt_header *head))
{
	struct inputhub_wrapper_notifier_node *pnode = NULL;
	struct inputhub_wrapper_notifier_node *n = NULL;
	int ret = 0;

	inputhub_wrapper_list_init();
	if (!match || !notify)
		return RET_FAIL;

	mutex_lock(&notifier_list_lock);
	/* avoid regist more than once */
	list_for_each_entry_safe(pnode, n, &g_inputhub_wrapper_list.head, entry) {
		if (match == pnode->match && notify == pnode->notify) {
			ctxhub_warn("%s, has already registed\n",
				  __func__);
			goto out; /* return when already registed */
		}
	}

	/* make inputhub_wrapper_notifier_node */
	pnode = kmalloc(sizeof(struct inputhub_wrapper_notifier_node), GFP_ATOMIC);
	if (pnode == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	(void)memset_s(pnode, sizeof(struct inputhub_wrapper_notifier_node), 0,
		sizeof(struct inputhub_wrapper_notifier_node));
	pnode->tag = 0xFF;
	pnode->cmd = 0xFF;
	pnode->subcmd = 0xFF;
	pnode->match = match;
	pnode->notify = notify;

	/* add to list */
	list_add(&pnode->entry, &g_inputhub_wrapper_list.head);
out:
	mutex_unlock(&notifier_list_lock);
	return ret;
}
/*lint +e613 +e668*/

int inputhub_wrapper_unregister_event_notifier_ex(
				int (*match)(const struct pkt_header *head),
				int (*notify)(const struct pkt_header *head))
{
	struct inputhub_wrapper_notifier_node *pos = NULL;
	struct inputhub_wrapper_notifier_node *n = NULL;

	if (!match || !notify)
		return -EINVAL;

	mutex_lock(&notifier_list_lock);
	list_for_each_entry_safe(pos, n, &g_inputhub_wrapper_list.head, entry) {
		if (match == pos->match && notify == pos->notify) {
			list_del(&pos->entry);
			kfree(pos);
			break;
		}
	}
	mutex_unlock(&notifier_list_lock);
	return 0;
}

/*lint -e613 -e668*/
int inputhub_wrapper_register_event_notifier(int tag, int cmd,
				int (*notify)(const struct pkt_header *head))
{
	struct inputhub_wrapper_notifier_node *pnode = NULL;
	struct inputhub_wrapper_notifier_node *n = NULL;
	int ret = 0;

	inputhub_wrapper_list_init();
	if ((!(tag >= TAG_BEGIN && tag < TAG_END)) || !notify)
		return RET_FAIL;

	mutex_lock(&notifier_list_lock);
	/* avoid regist more than once */
	list_for_each_entry_safe(pnode, n, &g_inputhub_wrapper_list.head, entry) {
		if ((tag == pnode->tag) && (cmd == pnode->cmd) &&
			(notify == pnode->notify)) {
			ctxhub_warn("%s:tag=%d, cmd=%d has already registed\n",
				  __func__, tag, cmd);
			goto out; /* return when already registed */
		}
	}

	/* make inputhub_wrapper_notifier_node */
	pnode = kmalloc(sizeof(struct inputhub_wrapper_notifier_node), GFP_ATOMIC);
	if (pnode == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	(void)memset_s(pnode, sizeof(struct inputhub_wrapper_notifier_node), 0,
		sizeof(struct inputhub_wrapper_notifier_node));
	pnode->tag = tag;
	pnode->cmd = cmd;
	pnode->match = NULL;
	pnode->notify = notify;

	/* add to list */
	list_add(&pnode->entry, &g_inputhub_wrapper_list.head);
out:
	mutex_unlock(&notifier_list_lock);
	return ret;
}
/*lint +e613 +e668*/

int inputhub_wrapper_unregister_event_notifier(int tag, int cmd,
				int (*notify)(const struct pkt_header *head))
{
	struct inputhub_wrapper_notifier_node *pos = NULL;
	struct inputhub_wrapper_notifier_node *n = NULL;

	if ((!(tag >= TAG_BEGIN && tag < TAG_END)) || !notify)
		return -EINVAL;

	mutex_lock(&notifier_list_lock);
	list_for_each_entry_safe(pos, n, &g_inputhub_wrapper_list.head, entry) {
		if ((tag == pos->tag) && (cmd == pos->cmd) &&
			(notify == pos->notify)) {
			list_del(&pos->entry);
			kfree(pos);
			break;
		}
	}
	mutex_unlock(&notifier_list_lock);
	return 0;
}

int inputhub_wrapper_send_cmd(int app_tag, int cmd, const void *buf, int buf_size, struct read_info *rd)
{
	// param check is in write_customize_cmd
	struct write_info wr;
	(void)memset_s(&wr, sizeof(wr), 0, sizeof(wr));

	wr.tag = TAG_AMF;
	wr.app_tag = app_tag;
	wr.cmd = cmd;
	wr.wr_buf = buf;
	wr.wr_len = buf_size;

	return write_customize_cmd(&wr, rd, false);
}

int inputhub_wrapper_get_app_tag(const struct pkt_header *head)
{
	if (head == NULL)
		return TAG_END;

	return head->app_tag;
}

static bool is_pnode_match(struct inputhub_wrapper_notifier_node *pnode,
						   const struct pkt_header *header)
{
	// static func, no need check param
	if (pnode->match != NULL)
		if (pnode->match(header) != 0)
			return true;

	if (pnode->tag == header->app_tag && pnode->cmd == header->cmd)
		return true;

	return false;
}
int inputhub_wrapper_packet_process(const struct pkt_header *header)
{
	/* context prove header is not null */
	struct inputhub_wrapper_notifier_node *pnode = NULL;
	struct inputhub_wrapper_notifier_node *n = NULL;
	bool found_notify_cb = false;
	int ret;

	if (header == NULL) {
		ctxhub_err("%s: header is null\n", __func__);
		return RET_FAIL;
	}
	/* even cmds are resp cmd */
	if ((header->cmd & 1) == 0)
		return RET_SUCC;

	mutex_lock(&notifier_list_lock);
	list_for_each_entry_safe(pnode, n, &g_inputhub_wrapper_list.head, entry) {
		if (is_pnode_match(pnode, header)) {
			if (pnode->notify == NULL) {
				ctxhub_warn("%s: tag %d, cmd %d notify is null\n",
					__func__, pnode->tag, pnode->cmd);
				continue;
			}

			found_notify_cb = true;
			ret = pnode->notify(header);
			if (ret)
				ctxhub_warn("%s: tag %d, cmd %d notify fail\n",
					__func__, pnode->tag, pnode->cmd);
		}
	}
	mutex_unlock(&notifier_list_lock);

	if (found_notify_cb)
		return RET_SUCC;

	ctxhub_err("%s: tag %d, cmd %d notify cb not found\n", __func__, pnode->tag, pnode->cmd);

	return RET_FAIL;
}

static int __init inputhub_wrapper_init(void)
{
	int ret;

	if (get_contexthub_dts_status())
		return -EINVAL;

	ret = register_mcu_event_notifier(TAG_AMF, CMD_ALL,
		inputhub_wrapper_packet_process);
	if (ret != 0) {
		ctxhub_err("%s, register mcu notify failed\n", __func__);
		return -EINVAL;
	}
	ctxhub_info("%s ok\n", __func__);
	return 0;
}

static void __exit inputhub_wrapper_exit(void)
{
	unregister_mcu_event_notifier(TAG_AMF, CMD_ALL, inputhub_wrapper_packet_process);
}

late_initcall_sync(inputhub_wrapper_init);
module_exit(inputhub_wrapper_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("inputhub wrapper driver");
MODULE_AUTHOR("Smart");

#endif // CONFIG_INPUTHUB_AS

/*
 * Function    : inputhub_wrapper_pack_and_send_cmd
 * Description : This interface support all scenarios and phone flp.
 *             : For all scenarios, no matter rd_info is nullptr or not, success is considered
 *             : when ipc interface return success and resp errno is 0.
 *             : But if rd_info is non-null pointer, it's considered that caller need resp value.
 *             : So variable rd will give rd_info.
 *             : For phone, returned error code and resp errno are not always required.
 *             : If caller need resp, will transport non-null pointer rd_info.
 *             : This function will return error code, how to use depend on caller.
 * Input       : [app_tag] app tag.
 *             : [cmd] cmd, CMD_CMN_CONFIG_REQ, CMD_CMN_OPEN_REQ and etc.
 *             : [buf] For CMD_CMN_CONFIG_REQ, this is customized buffer.
 *             :       For CMD_CMN_OPEN_REQ, CMD_CMN_CLOSE_REQ and etc, this is null.
 *             : [count] write buffer size.If buf is null, this will 0.
 *             : [rd] response info buffer
 * Output      : none
 * Return      : 0:successs, others: fail
 */
int inputhub_wrapper_pack_and_send_cmd(int app_tag, int cmd, int sub_cmd, const char *buf, size_t count, struct read_info *rd_info)
{
	char buffer[MAX_PKT_LENGTH];
	int ret;
	void *send_buf = NULL;
	int send_len = 0;

	if (count > INPUTHUB_WRAPPER_MAX_LEN) {
		pr_err("[%s], count %zu overflow limit %u\n", __func__, count, (unsigned int)INPUTHUB_WRAPPER_MAX_LEN);
		return -EINVAL;
	}

	/*
	 * Config packet need pack subcmd + buf
	 * For inputhub_as, success execute cmd need two point.
	 * One is ipc return success, the other is resp errno is 0.
	 */
	if (cmd == CMD_CMN_CONFIG_REQ) {
		((u32 *)buffer)[0] = sub_cmd;
		if (buf) {
			ret = memcpy_s(buffer + sizeof(u32), MAX_PKT_LENGTH - sizeof(u32), buf, count);
			if (ret != 0) {
				pr_err("[%s], memcpy failed!\n", __func__);
				return -EINVAL;
			}
		}
		send_buf = (void *)buffer;
		send_len = sizeof(u32) + count;
	}

	// this interface has different impl for inputhub as and no inputhub_as
	ret = inputhub_wrapper_send_cmd(app_tag, cmd, send_buf, send_len, rd_info);
	if (ret != 0) {
		pr_err("[%s], send cmd %d, sub cmd %d failed.Ret is %d\n",
			__func__, cmd, sub_cmd, ret);
		return -EINVAL;
	}

	if (rd_info) {
		if (rd_info->errno != 0) {
			pr_err("[%s], send cmd %d, sub cmd %d failed.Resp errno is %d\n",
				__func__, cmd, sub_cmd, rd_info->errno);
			return -EINVAL;
		}
	}
	return 0;
}
