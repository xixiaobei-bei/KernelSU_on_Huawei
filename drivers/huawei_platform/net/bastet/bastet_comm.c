/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014. All rights reserved.
 * Description: Provide kernel device information for bastet.
 *
 * Author: pengyu@huawei.com
 * Create: 2014-06-21
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <linux/if.h>
#include <huawei_platform/net/bastet/bastet_comm.h>
#include <huawei_platform/net/bastet/bastet_utils.h>
#include <huawei_platform/net/bastet/bastet.h>
#include <huawei_platform/net/bastet/bastet_interface.h>

#define BASTET_MODEM_DEV "/dev/bastet_modem"

#define DEV_OPEN_MAX_RETRY 60

struct buffer {
	struct list_head list;
	uint8_t data[BST_MAX_READ_PAYLOAD];
};

static struct file *dev_filp;

static int kernel_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	mm_segment_t oldfs = get_fs();
	int error = -ENOTTY;

	if (!filp || !filp->f_op || !filp->f_op->unlocked_ioctl)
		goto out;

	set_fs(KERNEL_DS);
	error = filp->f_op->unlocked_ioctl(filp, cmd, arg);
	set_fs(oldfs);

	if (error == -ENOIOCTLCMD)
		error = -ENOTTY;
out:
	return error;
}

#ifdef CONFIG_HUAWEI_BASTET_COMM
/**
 * get_modem_rab_id() - get the rab id.
 * @info, pointer of modem rab structer.
 *
 * use the current device name to get the modem rab info.
 * then fill the bst_modem_rab_id structer.
 *
 * Return: ret > 0, it means success.
 * Otherwise is fail.
 */
int get_modem_rab_id(struct bst_modem_rab_id *info)
{
	union bst_rab_id_ioc_arg ioc_arg = {};
	int ret;

	if (!info)
		return -EINVAL;

	if (IS_ERR_OR_NULL(dev_filp))
		return -EBUSY;

	memcpy(ioc_arg.in, cur_netdev_name, IFNAMSIZ);
	ioc_arg.in[IFNAMSIZ - 1] = '\0';

	ret = kernel_ioctl(dev_filp, BST_MODEM_IOC_GET_MODEM_RAB_ID,
		(unsigned long)&ioc_arg);
	if (ret >= 0) {
		info->modem_id = ioc_arg.out.modem_id;
		info->rab_id = ioc_arg.out.rab_id;
	}

	return ret;
}

int get_ipv6_modem_rab_id(struct bst_modem_rab_id *info)
{
	union bst_rab_id_ioc_arg ioc_arg = {};
	int ret;

	if (!info)
		return -EINVAL;

	if (IS_ERR_OR_NULL(dev_filp))
		return -EBUSY;

	memcpy(ioc_arg.in, cur_netdev_name, IFNAMSIZ);
	ioc_arg.in[IFNAMSIZ - 1] = '\0';

	ret = kernel_ioctl(dev_filp, BST_MODEM_IOC_GET_IPV6_MODEM_RAB_ID,
		(unsigned long)&ioc_arg);
	if (ret >= 0) {
		info->modem_id = ioc_arg.out.modem_id;
		info->rab_id = ioc_arg.out.rab_id;
	}

	return ret;
}
#endif /* CONFIG_HUAWEI_BASTET_COMM */

/**
 * bastet_modem_dev_write() - write message into dev.
 * @msg: the message need to write.
 * @len: message length.
 *
 * write the message to device.
 *
 * Return: ret > 0, it means success.
 * Otherwise is fail.
 */
int bastet_modem_dev_write(uint8_t *msg, uint32_t len)
{
	loff_t offset = 0;
	ssize_t ret;

	if (!msg)
		return -EINVAL;

	if (IS_ERR_OR_NULL(dev_filp))
		return -EBUSY;

	bastet_logi("len %u", len);

	if (BASTET_DEBUG)
		print_hex_dump(KERN_ERR,
			"bstmsg:", 0, 16, 1,
			(void *)msg, len, 1);

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
	ret = kernel_write(dev_filp, msg, len, &offset);
#else
	ret = kernel_write(dev_filp, msg, len, offset);
#endif
	bastet_logi("ret %ld", ret);

	return ret;
}

/**
 * bastet_comm_write() - write the common info.
 * @msg: the message need to write.
 * @len: message length.
 * @type: message type.
 *
 * write the message to device, invoke the bastet_modem_dev_write.
 *
 * Return: ret > 0, it means success.
 * Otherwise is fail.
 */
int bastet_comm_write(uint8_t *msg, uint32_t len, uint32_t type)
{
	struct bst_acom_msg *p_msg = NULL;
	uint8_t *buf = NULL;
	uint32_t ul_length;
	int ret;

	if (len > BST_MAX_WRITE_PAYLOAD) {
		bastet_loge(
			"write length over BST_MAX_WRITE_PAYLOAD!");
		return -EINVAL;
	}

	buf = kmalloc(sizeof(*p_msg) +
		BST_MAX_WRITE_PAYLOAD, GFP_KERNEL);
	if (buf == NULL)
		return -ENOMEM;

	p_msg = (struct bst_acom_msg *)buf;
	p_msg->en_msg_type = type;
	p_msg->ul_len     = len;
	memcpy(p_msg->auc_value, msg, len);
	ul_length = sizeof(*p_msg) + len;

	ret = bastet_modem_dev_write((uint8_t *)p_msg, ul_length);
	kfree(buf);

	return ret;
}

#ifdef CONFIG_HUAWEI_EMCOM
/**
 * bastet_comm_keypsInfo_write() - write the common key ps info.
 * @ulState: the message state.
 *
 * write the message to device, invoke the bastet_modem_dev_write.
 *
 * Return: ret > 0, it means success.
 * Otherwise is fail.
 */
int bastet_comm_keypsInfo_write(uint32_t ulState)
{
	struct bst_key_psinfo key_ps_info = {};
	int ret;

	bastet_logi("state: %d", ulState);

	key_ps_info.en_msg_type =
		BST_ACORE_CORE_MSG_TYPE_EMCOM_KEY_PSINFO;
	key_ps_info.en_state = ulState;

	ret = bastet_modem_dev_write((uint8_t *)&key_ps_info,
		sizeof(key_ps_info));
	return ret;
}
#endif /* CONFIG_HUAWEI_EMCOM */

#ifdef CONFIG_HW_CROSSLAYER_OPT
static void bastet_aspen_pkt_drop_proc(uint8_t *msg, uint32_t len)
{
	struct bst_aspen_pkt_drop *aspen_msg =
		(struct bst_aspen_pkt_drop *)msg;

	if (aspen_msg == NULL) {
		bastet_loge("aspen msg is empty");
		return;
	}

	if (len < (sizeof(*aspen_msg) - sizeof(aspen_msg->st_pkt))) {
		bastet_loge("aspen msg size too small %u", len);
		return;
	}

	switch (aspen_msg->ul_aspen_info_type) {
	case BST_ASPEN_INFO_PKT_DROP:
	{
		struct aspen_cdn_info *p_aspen_cdn_info = NULL;
		if (aspen_msg->ul_pkt_num >
			(BST_MAX_READ_PAYLOAD / sizeof(*p_aspen_cdn_info))) {
			bastet_loge("aspen ul_pkt_num oversize %d",
				aspen_msg->ul_pkt_num);
			break;
		}
		if (len != (sizeof(*aspen_msg) -
			(BST_ASPEN_PKT_DROP_SIZE - aspen_msg->ul_pkt_num) *
			sizeof(*p_aspen_cdn_info))) {
			bastet_loge("aspen msg size wrong %u", len);
			break;
		}
		aspen_crosslayer_recovery((void *)aspen_msg->st_pkt,
			(int)aspen_msg->ul_pkt_num);
		break;
	}
	default:
	{
		bastet_loge("aspen info type is wrong %u",
			aspen_msg->ul_aspen_info_type);
		break;
	}
	}
}
#endif /* CONFIG_HW_CROSSLAYER_OPT */

#ifdef CONFIG_HW_CROSSLAYER_OPT
static void handle_bst_acore_core_msg_type_aspen(uint8_t *msg, uint32_t len)
{
	struct bst_acom_msg *acom_msg = (struct bst_acom_msg *)msg;

	long hdrlen = (uint8_t *)(acom_msg->auc_value) - (uint8_t *)acom_msg;

	if (acom_msg->ul_len > BST_MAX_READ_PAYLOAD ||
		len != hdrlen + acom_msg->ul_len) {
		bastet_loge("aspen msg len error %u %u", len,
			acom_msg->ul_len);
		return;
	}
	bastet_aspen_pkt_drop_proc(acom_msg->auc_value, acom_msg->ul_len);
}
#endif

static void handle_bst_acore_core_msg_type_dspp(uint8_t *msg, uint32_t len)
{
	struct bst_acom_msg *acom_msg = (struct bst_acom_msg *)msg;
	long hdrlen = (uint8_t *)(acom_msg->auc_value) - (uint8_t *)acom_msg;

	if (len != hdrlen + acom_msg->ul_len) {
		bastet_loge("dspp msg len error %u %u", len, acom_msg->ul_len);
		return;
	}
	ind_hisi_com(acom_msg->auc_value, acom_msg->ul_len);
}

#ifdef CONFIG_HUAWEI_EMCOM
static void handle_bst_acore_core_msg_type_emcom_support(uint8_t *msg,
	uint32_t len)
{
	struct bst_emcom_support_msg *emcom_support_msg =
		(struct bst_emcom_support_msg *)msg;
	if (len != sizeof(*emcom_support_msg)) {
		bastet_loge("emcom support msg len error %u %lu", len,
			sizeof(*emcom_support_msg));
		return;
	}
	Emcom_Ind_Modem_Support(emcom_support_msg->en_state);
}
#endif

static void handle_bst_acore_core_msg_type_reset_info(uint8_t *msg,
	uint32_t len)
{
	struct bst_acom_msg *acom_msg = (struct bst_acom_msg *)msg;
	long hdrlen = (uint8_t *)(acom_msg->auc_value) - (uint8_t *)acom_msg;

	if (acom_msg->ul_len > BST_MAX_READ_PAYLOAD ||
		len != hdrlen + acom_msg->ul_len) {
		bastet_loge("reset info msg len error %u %u", len,
			acom_msg->ul_len);
		return;
	}
	ind_modem_reset(acom_msg->auc_value, acom_msg->ul_len);
}

static int handle_event(uint8_t *msg, uint32_t len)
{
	struct bst_common_msg *bst_msg = NULL;

	if (msg == NULL) {
		bastet_loge("msg is null");
		return 0;
	}

	if (len < sizeof(*bst_msg)) {
		bastet_loge("msg len error %u", len);
		return 0;
	}

	bst_msg = (struct bst_common_msg *)msg;

	bastet_logi("bst msg type %u len %u", bst_msg->en_msg_type, len);

	switch (bst_msg->en_msg_type) {
	#ifdef CONFIG_HW_CROSSLAYER_OPT
	case BST_ACORE_CORE_MSG_TYPE_ASPEN:
	{
		handle_bst_acore_core_msg_type_aspen(msg, len);
		break;
	}
	#endif
	case BST_ACORE_CORE_MSG_TYPE_DSPP:
	{
		handle_bst_acore_core_msg_type_dspp(msg, len);
		break;
	}
	#ifdef CONFIG_HUAWEI_EMCOM
	case BST_ACORE_CORE_MSG_TYPE_EMCOM_SUPPORT:
	{
		handle_bst_acore_core_msg_type_emcom_support(msg, len);
		break;
	}
	#endif
	case BST_ACORE_CORE_MSG_TYPE_RESET_INFO:
	{
		handle_bst_acore_core_msg_type_reset_info(msg, len);
		break;
	}
	default:
		bastet_loge("bst msg type error %u",
			bst_msg->en_msg_type);
		break;
	}
	return 0;
}

static void free_buffers(struct list_head *buffers)
{
	struct buffer *buffer_entry = NULL;
	struct buffer *tmp = NULL;

	if (!buffers)
		return;

	list_for_each_entry_safe(buffer_entry, tmp, buffers, list) {
		list_del(&buffer_entry->list);
		kfree(buffer_entry);
	}
}

static void copy_from_buffers(uint8_t *data,
	struct list_head *buffers,
	uint32_t total)
{
	struct buffer *buffer_entry = NULL;
	uint8_t *pdata = data;
	int remain = total;
	int size;

	if (!data || !buffers)
		return;

	list_for_each_entry(buffer_entry, buffers, list) {
		if (remain <= 0)
			break;
		size = remain > BST_MAX_READ_PAYLOAD ?
		BST_MAX_READ_PAYLOAD : remain;
		memcpy(pdata, buffer_entry->data, size);
		pdata += size;
		remain -= size;
	}
}

static int read_data_from_file(uint32_t *len, struct list_head *buffers)
{
	int32_t ret = 0;
	int ret_code = 0;
	struct buffer *buf = NULL;
	loff_t offset = 0;

	while (1) {
		buf = kmalloc(sizeof(*buf), GFP_KERNEL);
		if (buf == NULL) {
			free_buffers(buffers);
			ret_code = -ENOMEM;
			break;
		}

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
		ret = kernel_read(dev_filp, buf->data,
			BST_MAX_READ_PAYLOAD, &offset);
#else
		ret = kernel_read(dev_filp, offset, buf->data,
			BST_MAX_READ_PAYLOAD);
#endif
		bastet_logi("read %d", ret);

		if (-EBUSY == ret) {
			msleep(5000);
			kfree(buf);
			continue;
		}

		if (ret > 0 && ret < BST_MAX_READ_PAYLOAD) {
			if (BASTET_DEBUG)
				print_hex_dump(KERN_ERR, "bstmsg:", 0, 16, 1,
					(void *)buf, ret, 1);
			*len += ret;
			list_add(&buf->list, buffers);
			break;
		} else if (ret == BST_MAX_READ_PAYLOAD) {
			if (BASTET_DEBUG)
				print_hex_dump(KERN_ERR, "bstmsg:", 0, 16, 1,
					(void *)buf, ret, 1);
			*len += ret;
			list_add(&buf->list, buffers);
		} else if (ret == 0) {
			kfree(buf);
			break;
		}

		free_buffers(buffers);
		kfree(buf);
		ret_code = -EINVAL;
		break;
	}
	return ret_code;
}

static int get_more_data(uint8_t *firstdata, uint32_t buffer_size)
{
	uint32_t len = 0;
	uint8_t *data = NULL;
	struct list_head buffers = LIST_HEAD_INIT(buffers);

	int ret = read_data_from_file(&len, &buffers);

	if (ret == -EINVAL || ret == -ENOMEM)
		return ret;

		if (len == 0) {
			handle_event(firstdata, buffer_size);
			return 0;
		}

		data = kmalloc(len + BST_MAX_READ_PAYLOAD,
			GFP_KERNEL);
		if (data == NULL) {
			free_buffers(&buffers);
			return -EINVAL;
		}

		memcpy(data, firstdata, buffer_size);
		copy_from_buffers(data + BST_MAX_READ_PAYLOAD, &buffers, len);
		free_buffers(&buffers);

		handle_event(data, len + BST_MAX_READ_PAYLOAD);

		if (data != NULL)
			kfree(data);

		return 0;
}

static int get_event(void)
{
	int32_t size = 0;
	int ret = 0;
	uint8_t *buf = NULL;
	loff_t offset = 0;

	if (IS_ERR_OR_NULL(dev_filp))
		return -EINVAL;

	buf = kmalloc(BST_MAX_READ_PAYLOAD, GFP_KERNEL);
	if (buf == NULL)
		return -ENOMEM;

	while (1) {
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
		size = kernel_read(dev_filp, buf,
			BST_MAX_READ_PAYLOAD, &offset);
#else
		size = kernel_read(dev_filp, offset, buf,
			BST_MAX_READ_PAYLOAD);
#endif
		bastet_logi("read %d", size);
		if (-EBUSY == size) {
			msleep(5000);
			continue;
		} else {
			break;
		}
	}

	if (size > 0 && size < BST_MAX_READ_PAYLOAD) {
		if (BASTET_DEBUG) {
			print_hex_dump(KERN_ERR,
			"bstmsg:", 0, 16, 1,
			(void *)buf, size, 1);
			}
		handle_event(buf, size);
	} else if (size == BST_MAX_READ_PAYLOAD) {
		if (BASTET_DEBUG) {
			print_hex_dump(KERN_ERR,
			"bstmsg:", 0, 16, 1,
			(void *)buf, size, 1);
			}
			ret = get_more_data(buf, BST_MAX_READ_PAYLOAD);
	} else {
		bastet_logi("read error %d", size);
		ret = -EINVAL;
	}

	kfree(buf);
	return ret;
}

static int bastet_modem_dev_open(void)
{
	struct file *filp = NULL;
	int try = DEV_OPEN_MAX_RETRY;

	while (try--) {
		filp = filp_open(BASTET_MODEM_DEV, O_RDWR, 0);

		if (IS_ERR(filp)) {
			bastet_logi("open err=%ld",
				PTR_ERR(filp));

				msleep(1000);
				continue;
			} else {
				bastet_logi("open succ");
				break;
			}
		}

		if (IS_ERR(filp))
			return PTR_ERR(filp);

		dev_filp = filp;
		return 0;
}

static int bastet_modem_reader(void *data)
{
	int err;

	err = bastet_modem_dev_open();
	if (err < 0) {
		bastet_loge("error open bastet modem dev");
		return 0;
	}

	while (!kthread_should_stop()) {
		err = get_event();
		if (err < 0) {
			bastet_logi("get_event err=%d",
				err);
			break;
		}
	}

	bastet_logi("kthread exit");
	return 0;
}

/**
 * bastet_comm_init() - initialize the bastet.
 *
 * invoke the kthread to run the kernel thread. no parameters.
 * bastet_modem_reader was running on the thread.
 *
 * Return: 0, it means success.
 * Otherwise is fail.
 */
int bastet_comm_init(void)
{
	struct task_struct *task = NULL;

	bastet_logi("bastet_comm_init");

	task = kthread_run(bastet_modem_reader, NULL, "bastet_modem_rd");
	if (IS_ERR(task)) {
		bastet_loge(
		"create reader thread failed err=%ld",
		PTR_ERR(task));
		return -1;
	}

	return 0;
}
