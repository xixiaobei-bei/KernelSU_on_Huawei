/*
 * sw_core.c
 *
 * single-wire driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include "sw_core.h"
#include <linux/module.h>
#include <linux/reboot.h>
#include <linux/workqueue.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <securec.h>
#include <huawei_platform/inputhub/kbhub.h>
#include <crypto/hash.h>
#include <crypto/skcipher.h>
#include <linux/crypto.h>

#include "sw_debug.h"
#include "sw_detect.h"

#define KB_DEVICE_NAME         "hwsw_kb"
#define VERSION                "1.0"
#define SCREEN_ON              0
#define SCREEN_OFF             1
#define RECV_DATA_DELAY_MS     5
#define MACBT_RETRY_DELAY_MS   100
#define KB_POWERKEY_DATA_COUNT 7
#define SW_STATE_UNKNOWN       (-1)
#define SW_STATE_ENABLE        0

#define SN_LEN                 16
#define NFC_INFO_LEN           42
#define SIGNED_SN_LEN          32
#define MODULES_ID_LEN         3
#define KB_VID_PID_LEN         20
#define MAC_RECV_LEN           20
#define MAC_SEND_LEN           6
#define MAC_STEP               2
#define SN_START               4
#define MAC_BT_START           36
#define SUB_MODULES_ID_START   3
#define HEX_LEN                16
#define MAC_RETRY_TIMES        3

struct hw_sw_core_data {
	struct sk_buff_head tx_data_seq;
	struct sk_buff_head rx_data_seq;
	struct work_struct events;

	void *pm_data;

	struct sw_device *device;
	u16 kb_state;
	u32 product;
	u32 modules_id[MODULES_ID_LEN];
	u32 sub_modules_id;
	unsigned char mac_send[MAC_SEND_LEN];
	unsigned char mac_recv[MAC_RECV_LEN];
	unsigned char signed_sn_info[SIGNED_SN_LEN];
	char pid[KB_VID_PID_LEN];
	char vid[KB_VID_PID_LEN];
};

struct hw_sw_disc_data {
	struct platform_device *pm_pdev;
	struct hw_sw_core_data *core_data;
	struct wakeup_source kb_wakelock;
	struct delayed_work kb_channel_work;
	struct completion kb_comm_complete;
	struct sw_detector_ops detect_ops;

	int kb_online;
	int fb_state;
	int wait_fb_on;
};

struct platform_device *hw_sw_device;
static int g_sw_state = SW_STATE_UNKNOWN;
static struct class *hwkb_class;
static struct device *hwkb_device;

static struct shash_desc *init_shash_desc(struct crypto_shash *alg)
{
	int size = sizeof(struct shash_desc) + crypto_shash_descsize(alg);
	struct shash_desc *shash_desc = kmalloc(size, GFP_KERNEL);

	if (!shash_desc)
		return ERR_PTR(-ENOMEM);
	shash_desc->tfm = alg;
	shash_desc->flags = 0x0; /* default val */
	return shash_desc;
}

static int calc_hash(struct crypto_shash *alg, const unsigned char *data,
	unsigned int datalen, unsigned char *digest)
{
	int ret;
	struct shash_desc *shash_desc = init_shash_desc(alg);

	if (IS_ERR(shash_desc)) {
		sw_print_err("can't alloc shash_desc\n");
		return PTR_ERR(shash_desc);
	}

	ret = crypto_shash_digest(shash_desc, data, datalen, digest);
	if (ret)
		sw_print_err("can't crypto_shash_digest\n");
	kfree(shash_desc);
	return ret;
}

static int gen_hash256(const unsigned char *data, unsigned int datalen,
	unsigned char *digest)
{
	int ret;
	char *hash_alg_name = "sha256";
	struct crypto_shash *alg = crypto_alloc_shash(hash_alg_name, 0, 0);

	if (IS_ERR(alg)) {
		sw_print_err("can't alloc alg %s\n", hash_alg_name);
		return PTR_ERR(alg);
	}
	ret = calc_hash(alg, data, datalen, digest);
	if (ret)
		sw_print_err("can't calc_hash\n");
	crypto_free_shash(alg);
	return ret;
}

static int sw_get_sn(char *buf, int len)
{
	char *pstr = NULL;
	char *dstr = NULL;
	int ret;
	unsigned long tmp_len;

	pstr = strstr(saved_command_line, "androidboot.serialno=");
	if (!pstr) {
		sw_print_err("no fastboot serialno info\n");
		return -EINVAL;
	}
	pstr += strlen("androidboot.serialno=");
	dstr = strstr(pstr, " ");
	if (!dstr) {
		sw_print_err("no find the serialno end\n");
		return -EINVAL;
	}
	tmp_len = (unsigned long)(dstr - pstr);
	if (tmp_len > len) {
		sw_print_err("command_line format error\n");
		return -EINVAL;
	}
	ret = memcpy_s(buf, tmp_len, pstr, tmp_len);
	if (ret < 0) {
		sw_print_err("fail to copy tmp_len to buf\n");
		return -EINVAL;
	}
	buf[dstr - pstr] = '\0';

	return 0;
}

static int sw_get_signed_sn(struct hw_sw_core_data *psw_core_data)
{
	int ret;
	unsigned char sn_info[SN_LEN] = { 0 };

	if (!psw_core_data)
		return -EINVAL;

	ret = sw_get_sn(sn_info, SN_LEN);
	if (ret) {
		sw_print_err("fail to get sn\n");
		return -EINVAL;
	}
	ret = gen_hash256(sn_info, SN_LEN, psw_core_data->signed_sn_info);
	if (ret) {
		sw_print_err("fail to get hash256 sn\n");
		return -EINVAL;
	}

	return 0;
}

/* workqueue to process sw events */
static struct workqueue_struct *sw_wq;
static void sw_core_event(struct work_struct *work);

static struct hw_sw_disc_data *get_disc_data(void)
{
	struct platform_device *pdev = NULL;
	struct hw_sw_disc_data *pdisc_data = NULL;

	pdev = hw_sw_device;
	if (!pdev) {
		sw_print_err("pdev is null\n");
		return NULL;
	}

	pdisc_data = dev_get_drvdata(&pdev->dev);
	if (!pdisc_data) {
		sw_print_err("pdisc_data is null\n");
		return NULL;
	}

	return pdisc_data;
}

void send_nfc_info_to_sensorhub(struct hw_sw_core_data *psw_core_data)
{
	int i, ret;
	unsigned char kb_nfc_info[NFC_INFO_LEN] = { 0 };

	if (!psw_core_data)
		return;

	sw_print_info("enter\n");

	for (i = 0; i < MODULES_ID_LEN; i++)
		kb_nfc_info[i] = (unsigned char)psw_core_data->modules_id[i];

	ret = sw_get_signed_sn(psw_core_data);
	if (ret < 0)
		sw_print_err("get sn fail\n");

	kb_nfc_info[SUB_MODULES_ID_START] =
		(unsigned char)psw_core_data->sub_modules_id;

	for (i = 0; i < SIGNED_SN_LEN; i++)
		kb_nfc_info[SN_START + i] = psw_core_data->signed_sn_info[i];

	for (i = 0; i < MAC_SEND_LEN; i++)
		kb_nfc_info[MAC_BT_START + i] = psw_core_data->mac_send[i];

	ret = kernel_send_kb_report_event(KBHB_IOCTL_SEND_DATA,
		kb_nfc_info, NFC_INFO_LEN);
	if (ret < 0)
		sw_print_err("send nfc info fail\n");
}

static ssize_t stateinfo_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int len = 0;
	int ret;
	int buf_len = PAGE_SIZE;
	char *cur = buf;
	struct hw_sw_disc_data *pdisc_data = get_disc_data();
	struct hw_sw_core_data *pcore_data = NULL;

	SW_PRINT_FUNCTION_NAME;

	if (!pdisc_data)
		goto exit_nosupport;

	pcore_data = pdisc_data->core_data;
	if (!pcore_data)
		goto exit;

	ret = snprintf_s(cur, buf_len, buf_len - 1,
		"keyboard is %s\n",
		(pcore_data->kb_state == KBSTATE_ONLINE) ?
		"online" : "offline");
	if (ret < 0)
		goto exit;

	buf_len -= ret;
	cur += ret;
	len += ret;

	if (pcore_data->kb_state == KBSTATE_ONLINE) {
		ret = snprintf_s(cur, buf_len, buf_len - 1,
			"keyboard vendor id is 0x%04X\n",
			pcore_data->device->vendor);
		if (ret < 0)
			goto exit;

		buf_len -= ret;
		cur += ret;
		len += ret;
		ret = snprintf_s(cur, buf_len, buf_len - 1,
			"keyboard version is %d.%d\n",
			pcore_data->device->mainver,
			pcore_data->device->subver);
		if (ret < 0)
			return 0;

		buf_len -= ret;
		cur += ret;
		len += ret;
	}
	goto exit;

exit_nosupport:
	len = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "no support\n");
exit:
	return (len >= PAGE_SIZE) ? (PAGE_SIZE - 1) : len;
}

static DEVICE_ATTR_RO(stateinfo);

static ssize_t kb_init_nfc_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int len;
	struct hw_sw_disc_data *pdisc_data = get_disc_data();

	sw_print_info("enter\n");

	if (!pdisc_data || !pdisc_data->core_data)
		goto exit_nosupport;

	len = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
		"%s", pdisc_data->core_data->mac_recv);
	if (len < 0)
		return 0;
	goto exit;

exit_nosupport:
	len = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "no support\n");
exit:
	return (len >= PAGE_SIZE) ? (PAGE_SIZE - 1) : len;
}

static int mac_string_to_hex(char *mac_string,
	unsigned char *mac_data, int size)
{
	int i = 0;
	long val = 0;
	char *ptr = NULL;
	char *p = NULL;

	if (!mac_string || !mac_data)
		return -EINVAL;

	p = mac_string;
	ptr = strsep(&p, ":");
	while (ptr) {
		if (kstrtol(ptr, HEX_LEN, &val) < 0)
			return -EINVAL;

		if (i < size)
			mac_data[i] = (unsigned char)val;
		else
			return -EINVAL;

		i++;
		ptr = strsep(&p, ":");
	}

	return 0;
}

static ssize_t kb_init_nfc_info_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	struct hw_sw_core_data *pcore_data = NULL;
	struct hw_sw_disc_data *pdisc_data = get_disc_data();

	if (!buf || !pdisc_data)
		return -EINVAL;

	pcore_data = pdisc_data->core_data;
	if (!pcore_data)
		return -EINVAL;

	sw_print_info("count is %d\n", count);

	ret = strcpy_s(pcore_data->mac_recv, MAC_RECV_LEN, buf);
	if (ret != EOK) {
		sw_print_err("strcpy_s err %x\n", ret);
		return -EINVAL;
	}

	ret = mac_string_to_hex(pcore_data->mac_recv,
		pcore_data->mac_send, MAC_SEND_LEN);
	if (ret < 0) {
		sw_print_err("mac to string error\n");
		return -EINVAL;
	}

	return count;
}

static DEVICE_ATTR_RW(kb_init_nfc_info);

static struct attribute *sw_dev_attrs[] = {
	&dev_attr_stateinfo.attr,
	&dev_attr_kb_init_nfc_info.attr,
	NULL,
};

static const struct attribute_group sw_platform_group = {
	.attrs = sw_dev_attrs,
};

static int sw_get_core_reference(struct hw_sw_core_data **core_data)
{
	struct platform_device *pdev = hw_sw_device;
	struct hw_sw_disc_data *pdisc_data = NULL;

	if (!pdev) {
		*core_data = NULL;
		sw_print_err("pdev is null\n");
		return -FAILURE;
	}

	pdisc_data = dev_get_drvdata(&pdev->dev);
	if (!pdisc_data) {
		*core_data = NULL;
		sw_print_err("pdisc_data is null\n");
		return -FAILURE;
	}

	*core_data = pdisc_data->core_data;

	return SUCCESS;
}

static void sw_recv_data_frame(struct hw_sw_core_data *core_data,
	struct sk_buff *skb)
{
	if (!core_data || !skb)
		return;

	if (!core_data->device)
		return;

	if (!core_data->device->drv ||
		!core_data->device->drv->recvframe)
		return;

	core_data->device->drv->recvframe(core_data->device, skb);
}

static int sw_connect_device(struct hw_sw_core_data *core_data,
	struct sk_buff *skb)
{
	int ret;
	u32 vendorid;
	struct sw_device *device = NULL;

	if (!skb || skb->len < KB_HANDSHAKE_LEN)
		return -FAILURE;

	vendorid = (u32)((skb->data[KB_VID_HIGH_BYTE] << BITS_NUM_PER_BYTE) |
		skb->data[KB_VID_LOW_BYTE]);

	device = sw_create_new_device(core_data->product, vendorid);
	if (!device)
		return -FAILURE;

	device->mainver = (u8)skb->data[KB_MAIN_VERSION_BYTE];
	device->subver = (u8)skb->data[KB_SUB_VERSION_BYTE];
	device->dev.parent = &hw_sw_device->dev;

	ret = device_register(&device->dev);
	if (ret)
		sw_print_err("device_add %x\n", ret);

	core_data->device = device;
	core_data->kb_state = KBSTATE_ONLINE;

	return ret;
}

static int sw_disconnect_device(struct hw_sw_core_data *core_data)
{
	struct sw_device *device = NULL;
	int err = SUCCESS;

	if (!core_data)
		return SUCCESS;

	if (core_data->device) {
		device = core_data->device;
		core_data->device = NULL;
		err = sw_release_device(device);
	}

	return err;
}

static void swkb_wake_lock_timeout(struct hw_sw_disc_data *pdisc_data,
	long timeout)
{
	if (!pdisc_data->kb_wakelock.active) {
		__pm_wakeup_event(&pdisc_data->kb_wakelock,
			jiffies_to_msecs(timeout));
		sw_print_info("swkb wake lock\n");
	}
}

static void swkb_wake_lock(struct hw_sw_disc_data *pdisc_data)
{
	if (!pdisc_data->kb_wakelock.active) {
		__pm_stay_awake(&pdisc_data->kb_wakelock);
		sw_print_info("swkb wake lock\n");
	}
}

static void swkb_wake_unlock(struct hw_sw_disc_data *pdisc_data)
{
	if (pdisc_data->kb_wakelock.active) {
		__pm_relax(&pdisc_data->kb_wakelock);
		sw_print_info("swkb wake unlock\n");
	}
}

static void sw_update_pid_vid(char *uevent[])
{
	int ret;
	struct hw_sw_core_data *pcore_data = NULL;
	struct hw_sw_disc_data *pdisc_data = get_disc_data();

	if (!pdisc_data)
		return;

	pcore_data = pdisc_data->core_data;
	if (!pcore_data)
		return;

	if ((pcore_data->product != 0) && (pcore_data->device->vendor != 0)) {
		ret = snprintf_s(pcore_data->vid, KB_VID_PID_LEN,
			KB_VID_PID_LEN - 1, "KB_VID=%4x",
			pcore_data->device->vendor);
		if (ret < 0)
			sw_print_err("failed to parse vid\n");
		else
			uevent[1] = pcore_data->vid; /* vid uevent */

		ret = snprintf_s(pcore_data->pid, KB_VID_PID_LEN,
			KB_VID_PID_LEN - 1, "KB_PID=%4x",
			pcore_data->product);
		if (ret < 0)
			sw_print_err("failed to parse pid\n");
		else
			uevent[2] = pcore_data->pid; /* pid uevent */
	}
}

static void sw_notify_android_uevent(int is_conn_or_disconn)
{
	char *disconnected[KB_STATE_MAX] = { "KB_STATE=DISCONNECTED", NULL };
	char *connected[KB_STATE_CONNECT_MAX] = { "KB_STATE=CONNECTED",
		NULL, NULL, NULL };
	char *nfc_req[KB_STATE_MAX] = { "KB_STATE=RECV_NFC_REQ", NULL };
	char *nfc_near[KB_STATE_NFC_NEAR_MAX] = { "KB_NFC_STATE=NEAR",
		NULL, NULL, NULL };
	char *nfc_far[KB_STATE_MAX] = { "KB_NFC_STATE=FAR", NULL };

	sw_print_info("enter\n");

	if (IS_ERR(hwkb_device)) {
		sw_print_info("device uninit\n");
		return;
	}
	switch (is_conn_or_disconn) {
	case KBSTATE_ONLINE:
		sw_update_pid_vid(connected);
		kobject_uevent_env(&hwkb_device->kobj, KOBJ_CHANGE, connected);
		sw_print_info("connected\n");
		break;
	case KBSTATE_OFFLINE:
		kobject_uevent_env(&hwkb_device->kobj,
			KOBJ_CHANGE, disconnected);
		kb_extented_dmd_report(KB_DISCONNECTED, KB_NFC_NOT_WRITE);
		sw_print_info("disconnected\n");
		break;
	case KBSTATE_NFC_INFO_REQ:
		kobject_uevent_env(&hwkb_device->kobj, KOBJ_CHANGE, nfc_req);
		sw_print_info("recv nfc info req\n");
		break;
	case KBSTATE_NFC_NEAR:
		sw_update_pid_vid(nfc_near);
		kobject_uevent_env(&hwkb_device->kobj, KOBJ_CHANGE, nfc_near);
		sw_print_info("recv nfc near\n");
		break;
	case KBSTATE_NFC_FAR:
		kobject_uevent_env(&hwkb_device->kobj, KOBJ_CHANGE, nfc_far);
		sw_print_info("recv nfc far\n");
		break;
	default:
		sw_print_err("is_conn_or_disconn = %d, parameter is invalid\n",
			is_conn_or_disconn);
		break;
	}
}

static int sw_core_init(struct hw_sw_core_data **core_data)
{
	int ret;
	u32 val = 0;
	struct hw_sw_core_data *psw_core_data = NULL;
	struct device_node *np = NULL;

	SW_PRINT_FUNCTION_NAME;

	psw_core_data = kzalloc(sizeof(*psw_core_data), GFP_KERNEL);
	if (!psw_core_data)
		return -ENOMEM;

	skb_queue_head_init(&psw_core_data->rx_data_seq);
	skb_queue_head_init(&psw_core_data->tx_data_seq);

	psw_core_data->device = NULL;
	/* product default cmr keyboard product id */
	psw_core_data->product = CMR_KEYBOARD_PID;

	INIT_WORK(&psw_core_data->events, sw_core_event);

	np = of_find_compatible_node(NULL, NULL, "huawei,sw_kb");
	if (!np) {
		sw_print_err("unable to find %s\n", "huawei, sw_kb");
		return -ENOENT;
	}

	if (of_property_read_u32(np, "product_id", &val))
		sw_print_err("dts:can not get product_id\n");
	else
		psw_core_data->product = val;

	ret = of_property_read_u32_array(np, "modules_id",
		psw_core_data->modules_id, MODULES_ID_LEN);
	if (ret)
		sw_print_err("dts:can not get modules_id\n");

	if (of_property_read_u32(np, "sub_modules_id", &val))
		sw_print_err("dts:can not get sub_modules_id\n");
	else
		psw_core_data->sub_modules_id = val;

	*core_data = psw_core_data;
	return SUCCESS;
}

static int sw_skb_enqueue(struct hw_sw_core_data *psw_core_data,
	struct sk_buff *skb, u8 type)
{
	if (unlikely(!psw_core_data)) {
		sw_print_err("psw_core_data is null\n");
		return -EINVAL;
	}

	if (unlikely(!skb)) {
		sw_print_err("skb is null\n");
		return -EINVAL;
	}

	switch (type) {
	case TX_DATA_QUEUE:
		skb_queue_tail(&psw_core_data->tx_data_seq, skb);
		break;
	case RX_DATA_QUEUE:
		skb_queue_tail(&psw_core_data->rx_data_seq, skb);
		break;
	default:
		sw_print_err("queue type is error, type=%d\n", type);
		break;
	}

	return SUCCESS;
}

static int sw_push_skb_queue(struct hw_sw_core_data *psw_core_data,
	u8 *buf_ptr, int pkt_len, u8 type)
{
	struct sk_buff *skb = NULL;
	int ret;

	if (!psw_core_data) {
		sw_print_err("psw_core_data is null\n");
		return -EINVAL;
	}

	if (!buf_ptr) {
		sw_print_err("buf_ptr is null\n");
		return -EINVAL;
	}

	skb = alloc_skb(pkt_len, GFP_ATOMIC);
	if (!skb)
		return -ENOMEM;

	skb_put(skb, pkt_len);
	ret = memcpy_s(skb->data, pkt_len, buf_ptr, pkt_len);
	if (ret < 0) {
		sw_print_err("fail to copy buf_ptr to skb->data\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	return sw_skb_enqueue(psw_core_data, skb, type);
}

static struct sk_buff *sw_skb_dequeue(struct hw_sw_core_data *psw_core_data,
	u8 type)
{
	struct sk_buff *curr_skb = NULL;

	if (!psw_core_data) {
		sw_print_err("psw_core_data is null\n");
		return NULL;
	}

	switch (type) {
	case TX_DATA_QUEUE:
		curr_skb = skb_dequeue(&psw_core_data->tx_data_seq);
		break;
	case RX_DATA_QUEUE:
		curr_skb = skb_dequeue(&psw_core_data->rx_data_seq);
		break;
	default:
		sw_print_err("queue type is error, type=%d\n", type);
		break;
	}

	return curr_skb;
}

static void sw_kfree_skb(struct hw_sw_core_data *psw_core_data, u8 type)
{
	struct sk_buff *skb = NULL;

	SW_PRINT_FUNCTION_NAME;
	if (!psw_core_data) {
		sw_print_err("psw_core_data is null");
		return;
	}

	skb = sw_skb_dequeue(psw_core_data, type);
	while (skb) {
		kfree_skb(skb);
		skb = sw_skb_dequeue(psw_core_data, type);
	}

	switch (type) {
	case TX_DATA_QUEUE:
		skb_queue_purge(&psw_core_data->tx_data_seq);
		break;
	case RX_DATA_QUEUE:
		skb_queue_purge(&psw_core_data->rx_data_seq);
		break;
	default:
		sw_print_err("queue type is error, type=%d\n", type);
		break;
	}
}

static int sw_core_exit(struct hw_sw_core_data *pcore_data)
{
	SW_PRINT_FUNCTION_NAME;

	if (pcore_data) {
		sw_disconnect_device(pcore_data);
		/* free sw tx and rx queue */
		sw_kfree_skb(pcore_data, TX_DATA_QUEUE);
		sw_kfree_skb(pcore_data, RX_DATA_QUEUE);
		kfree(pcore_data);
	}
	return SUCCESS;
}

static void kick_sw_wq(void)
{
	struct hw_sw_core_data *sw_core_data = NULL;
	int ret;

	ret = sw_get_core_reference(&sw_core_data);
	if ((ret != SUCCESS) || !sw_core_data)
		return;

	if (work_pending(&sw_core_data->events))
		return;

	if (queue_work(sw_wq, &sw_core_data->events))
		return;
}

static void sw_send_power_key(struct hw_sw_core_data *core_data)
{
	char powerkey_down[] = { 0x0, 0x7, 0x0, 0x0, 0x0, 0x30, 0x0 };
	char powerkey_up[] = { 0x0, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0 };
	struct sk_buff *skb = NULL;
	int ret;

	if (!core_data)
		return;
	sw_print_info("enter\n");

	skb = alloc_skb(KB_POWERKEY_DATA_COUNT, GFP_ATOMIC);
	if (!skb)
		return;

	skb_put(skb, KB_POWERKEY_DATA_COUNT);
	ret = memcpy_s(skb->data, KB_POWERKEY_DATA_COUNT, powerkey_down,
		KB_POWERKEY_DATA_COUNT);
	if (ret < 0) {
		sw_print_err("fail to copy powerkey_down to skb->data\n");
		kfree_skb(skb);
		return;
	}

	sw_recv_data_frame(core_data, skb);
	msleep(RECV_DATA_DELAY_MS);
	ret = memcpy_s(skb->data, KB_POWERKEY_DATA_COUNT, powerkey_up,
		KB_POWERKEY_DATA_COUNT);
	if (ret < 0) {
		sw_print_err("fail to copy powerkey_up to skb->data\n");
		kfree_skb(skb);
		return;
	}

	sw_recv_data_frame(core_data, skb);
	kfree_skb(skb);
	sw_print_info("exit\n");
}

/*
 * Note:handshake data format
 * cmd(1 byte) + devno(1 byte) + len(1 byte) + crc(2 bytes) +
 * payload(1 byte) + vendor(2 bytes)+ mainver(1 byte) +  subver(1 byte)
 */
static int sw_process_cmd_handshake(struct hw_sw_core_data *core_data,
	struct sk_buff *skb)
{
	int ret = SUCCESS;

	sw_print_dbg("sw_core_event connect keyboard\n");

	if (!core_data->device) {
		ret = sw_connect_device(core_data, skb);
		sw_notify_android_uevent(KBSTATE_ONLINE);
	}

	return ret;
}

static int sw_process_cmd_disconnect(struct hw_sw_core_data *core_data,
	struct sk_buff *skb)
{
	int ret;
	struct hw_sw_disc_data *pdisc_data =
		(struct hw_sw_disc_data *)(core_data->pm_data);
	if (!pdisc_data)
		return -FAILURE;

	sw_print_dbg("sw_core_event disconnect keyboard\n");

	/* disable kb device */
	ret = sw_disconnect_device(core_data);
	core_data->kb_state = KBSTATE_OFFLINE;

	if (pdisc_data->detect_ops.notify)
		pdisc_data->detect_ops.notify(pdisc_data->detect_ops.detector,
			SW_NOTIFY_EVENT_DISCONNECT, pdisc_data);

	if (ret == SUCCESS) {
		swkb_wake_lock_timeout(pdisc_data, DEFAULT_WAKE_TIMEOUT);
		kernel_send_kb_cmd(KBHB_IOCTL_STOP, 0);
		if (pdisc_data->kb_online == DEVSTAT_CHGDEV_ONLINE) {
			pdisc_data->kb_online = DEVSTAT_NONEDEV;
			sw_print_info("charge device remove\n");
			return ret;
		}
		sw_notify_android_uevent(KBSTATE_OFFLINE);
	}
	return ret;
}

static int sw_process_cmd_default(struct hw_sw_core_data *core_data,
	struct sk_buff *skb)
{
	struct hw_sw_disc_data *pdisc_data =
		(struct hw_sw_disc_data *)(core_data->pm_data);
	if (!pdisc_data)
		return -FAILURE;

	if (pdisc_data->fb_state == SCREEN_OFF) {
		if (pdisc_data->wait_fb_on == SCREEN_ON) {
			pdisc_data->wait_fb_on = SCREEN_OFF;
			sw_send_power_key(core_data);
		}
	}

	if (pdisc_data->fb_state == SCREEN_ON) {
		if (pdisc_data->wait_fb_on == SCREEN_OFF)
			pdisc_data->wait_fb_on = SCREEN_ON;
		sw_recv_data_frame(core_data, skb);
	}

	return SUCCESS;
}

static void sw_core_event(struct work_struct *work)
{
	struct hw_sw_core_data *core_data = NULL;
	struct sk_buff *skb = NULL;
	u8 hdr;

	SW_PRINT_FUNCTION_NAME;

	core_data = container_of(work, struct hw_sw_core_data, events);

	skb = sw_skb_dequeue(core_data, RX_DATA_QUEUE);
	while (skb) {
		skb_orphan(skb);
		if (!skb_linearize(skb)) {
			hdr = (u8)skb->data[0];
			switch (hdr) {
			case PROTO_CMD_HANDSHAKE:
				sw_process_cmd_handshake(core_data, skb);
				break;
			case PROTO_CMD_DISCONNECT:
				sw_process_cmd_disconnect(core_data, skb);
				break;
			case PROTO_CMD_WORKMODE:
				sw_recv_data_frame(core_data, skb);
				break;
			default:
				sw_process_cmd_default(core_data, skb);
				break;
			}
		}

		kfree_skb(skb);
		skb = sw_skb_dequeue(core_data, RX_DATA_QUEUE);
	}
}

static int sw_core_recv(struct hw_sw_core_data *sw_core_data, u8 *data,
	int count)
{
	int ret;

	SW_PRINT_FUNCTION_NAME;
	if (!sw_core_data) {
		sw_print_err("core_data is null\n");
		return -EINVAL;
	}

	if (unlikely(!data)) {
		sw_print_err("received null\n");
		return -EINVAL;
	}

	if (count < 1) {
		sw_print_err("received count error\n");
		return -EINVAL;
	}

	ret = sw_push_skb_queue(sw_core_data, (u8 *)data, count, RX_DATA_QUEUE);
	if (ret == SUCCESS)
		kick_sw_wq();

	return ret;
}

static int sw_poweroff_notify_sys(struct notifier_block *this,
	unsigned long code, void *unused)
{
	return SUCCESS;
}

static struct notifier_block plat_poweroff_notifier = {
	.notifier_call = sw_poweroff_notify_sys,
};

static void sw_fb_notifier_action(int screen_on_off)
{
	struct hw_sw_disc_data *pdisc_data = get_disc_data();

	if (!pdisc_data)
		return;

	pdisc_data->fb_state = screen_on_off;
	if ((pdisc_data->fb_state == SCREEN_ON) &&
		(pdisc_data->wait_fb_on == SCREEN_OFF))
		pdisc_data->wait_fb_on = SCREEN_ON;
}

static int sw_fb_notifier(struct notifier_block *nb,
	unsigned long action, void *data)
{
	int *blank = NULL;
	struct fb_event *event = data;

	if (!event) {
		sw_print_err("event is null\n");
		return NOTIFY_OK;
	}

	blank = event->data;
	if (!blank) {
		sw_print_err("blank is null\n");
		return NOTIFY_OK;
	}

	switch (action) {
	case FB_EVENT_BLANK: /* change finished */
		/* only notify main screen on/off info */
		if (event->info != registered_fb[0]) {
			sw_print_info("not main screen info, just return\n");
			return NOTIFY_OK;
		}

		switch (*blank) {
		case FB_BLANK_UNBLANK: /* screen on */
			sw_print_info("screen on\n");
			sw_fb_notifier_action(SCREEN_ON);
			break;

		case FB_BLANK_POWERDOWN: /* screen off */
			sw_print_info("screen off\n");
			sw_fb_notifier_action(SCREEN_OFF);
			break;

		default:
			sw_print_err("unknown---> lcd unknown\n");
			break;
		}
		break;

	default:
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block fb_notify = {
	.notifier_call = sw_fb_notifier,
};

static void swkb_channel_com_work(struct work_struct *work)
{
	unsigned long timeout;
	struct hw_sw_disc_data *pdisc_data = container_of(work,
		struct hw_sw_disc_data, kb_channel_work.work);

	sw_print_info("start\n");
	swkb_wake_lock(pdisc_data);
	timeout = wait_for_completion_timeout(
		&pdisc_data->kb_comm_complete,
		msecs_to_jiffies(KB_COMM_COMPLETE_TIMEOUT_MS));
	if (timeout == 0) {
		sw_print_err("wait_for_completion_timeout timeout\n");
		goto err_timeout;
	}

	reinit_completion(&pdisc_data->kb_comm_complete);

err_timeout:
	swkb_wake_unlock(pdisc_data);
	sw_print_info("end\n");
}

static void sw_destroy_monitor_device(struct platform_device *pdev)
{
	if (!pdev)
		return;

	if (!IS_ERR(hwkb_device))
		device_destroy(hwkb_device->class, hwkb_device->devt);

	if (!IS_ERR(hwkb_class))
		class_destroy(hwkb_class);

	hwkb_device = NULL;
	hwkb_class = NULL;
}

static void sw_init_monitor_device(struct platform_device *pdev)
{
	int ret;

	if (hwkb_device || hwkb_class)
		sw_destroy_monitor_device(pdev);

	hwkb_class = class_create(THIS_MODULE, KB_DEVICE_NAME);
	if (IS_ERR(hwkb_class)) {
		ret = PTR_ERR(hwkb_class);
		goto err_init;
	}

	hwkb_device = device_create(hwkb_class, NULL, 0, NULL, "hwkb");
	if (IS_ERR(hwkb_device)) {
		ret = PTR_ERR(hwkb_device);
		goto err_init;
	}

	return;

err_init:
	sw_print_err("failed %x\n", ret);
	sw_destroy_monitor_device(pdev);
}

/*
 * if dts config hwsw_kb g_sw_state = 0
 * in inputhub  kbhub_channel model will init when  g_sw_state = 0
 */
static int swkb_get_statue_callback(void)
{
	return g_sw_state;
}

static void swkb_notify_event_callback(unsigned int event)
{
	int i;
	struct hw_sw_disc_data *pdisc_data = get_disc_data();

	if (!pdisc_data)
		return;
	if (pdisc_data->detect_ops.notify) {
		if (event == NOTIFY_EVENT_DETECT) {
			pdisc_data->detect_ops.notify(
				pdisc_data->detect_ops.detector,
				SW_NOTIFY_EVENT_REDETECT,
				pdisc_data);
		} else if (event == NOTIFY_EVENT_RECV_NFC_REQ) {
			for (i = 0; i < MAC_RETRY_TIMES; i++) {
				/*
				 * bluetooth mac third is ':', retry 3 times
				 * for waiting service write raw bluetooth mac
				 * to node before kernel send mac to sensorhub
				 */
				if (pdisc_data->core_data->mac_recv[2] == ':')
					break;
				msleep(MACBT_RETRY_DELAY_MS);
			}
			send_nfc_info_to_sensorhub(pdisc_data->core_data);
		} else if (event == NOTIFY_EVENT_NEAR) {
			sw_notify_android_uevent(KBSTATE_NFC_NEAR);
		} else if (event == NOTIFY_EVENT_FAR) {
			sw_notify_android_uevent(KBSTATE_NFC_FAR);
		}
	}
}

static int swkb_process_kbdata_callback(char *data, int count)
{
	struct hw_sw_core_data *sw_core_data = NULL;
	struct sw_driver *swdriver = NULL;
	int ret;

	ret = sw_get_core_reference(&sw_core_data);
	if ((ret != SUCCESS) || !sw_core_data)
		return -FAILURE;

	if (sw_core_data->device) {
		swdriver = sw_core_data->device->drv;
		if (swdriver && swdriver->pre_recvframe) {
			/* if ret != 0 , this frame need ignore */
			ret = swdriver->pre_recvframe(sw_core_data->device,
				data, count);
			if (ret)
				return -FAILURE;
		}
	}

	ret = sw_core_recv(sw_core_data, (u8 *)data, count);
	return ret;
}

static struct kb_dev_ops sw_kbhubops = {
	.process_kbdata = swkb_process_kbdata_callback,
	.get_status = swkb_get_statue_callback,
	.notify_event = swkb_notify_event_callback,
};

/*
 * Note: CMD_KB_EVENT data format to sensorhub
 * led(1 byte) + len(1 byte) + data_type(1 byte) + reserved(1 byte) +
 * data (12 bytes)
 * sensohub used led byte for Caps Lock, other must set it to 0xff
 * sw may detect kb or charger insert, only use byte[4] to indicate it
 */
static void sw_detect_notify_connect_device(enum sw_connect_dev_num dev)
{
	u8 buffer[SW_REPORT_DATA_SIZE] = { 0 };
	int i = 0;
	/*
	 * buffer[0]: not led control data
	 * buffer[1]: data is 1byte
	 * buffer[2]: connect type data
	 * buffer[3]: reserved
	 * buffer[4]: kb or charge connect
	 */
	buffer[i++] = SW_KB_EVENT_LED_INVALID;
	buffer[i++] = 1;
	buffer[i++] = SW_KB_EVENT_TYPE_CONNECT_DEV;
	buffer[i++] = 0;
	buffer[i++] = dev;

	kernel_send_kb_report_event(KBHB_IOCTL_CMD, buffer,
		SW_REPORT_DATA_SIZE);
}

static int sw_detect_notifier_call(struct notifier_block *detect_nb,
	unsigned long event, void *data)
{
	struct hw_sw_disc_data *pdisc_data = get_disc_data();

	if (pdisc_data) {
		sw_print_err("event %lx\n", event);

		if (event == DEVSTAT_KBDEV_ONLINE) {
			swkb_wake_lock_timeout(pdisc_data,
				DEFAULT_WAKE_TIMEOUT);
			/* notify sensorhub start work */
			kernel_send_kb_cmd(KBHB_IOCTL_START, 0);
			sw_detect_notify_connect_device(SW_DEV_KB);
		} else if (event == DEVSTAT_CHGDEV_ONLINE) {
			swkb_wake_lock_timeout(pdisc_data,
				DEFAULT_WAKE_TIMEOUT);
			kernel_send_kb_cmd(KBHB_IOCTL_START, 0);
			sw_detect_notify_connect_device(SW_DEV_CHARGER);
		}
		pdisc_data->kb_online = event;
	}

	return NOTIFY_OK;
}

static void sw_detectops_destroy(struct sw_detector_ops *ops)
{
	if (ops && ops->detector && ops->notify) {
		ops->notify(ops->detector,
			SW_NOTIFY_EVENT_DESTROY, ops);
		ops->detector = NULL;
	}
}

static int sw_probe(struct platform_device *pdev)
{
	struct hw_sw_disc_data *pdisc_data = NULL;
	int ret;

	SW_PRINT_FUNCTION_NAME;

	hw_sw_device = pdev;

	pdisc_data = kzalloc(sizeof(*pdisc_data), GFP_KERNEL);
	if (!pdisc_data)
		return -ENOMEM;

	dev_set_drvdata(&pdev->dev, pdisc_data);

	pdisc_data->detect_ops.detect_nb.notifier_call =
		sw_detect_notifier_call;
	pdisc_data->detect_ops.notify = NULL;

	ret = sw_gpio_detect_register(pdev, &pdisc_data->detect_ops);
	if (ret < 0) {
		sw_print_err("sw_gpio_detect_register failed\n");
		goto err_core_init;
	}

	ret = sw_core_init(&pdisc_data->core_data);
	if (ret != 0) {
		sw_print_err("sw core init failed\n");
		goto err_core_init;
	}
	/* refer to itself */
	pdisc_data->core_data->pm_data = pdisc_data;
	/* get reference of pdev */
	pdisc_data->pm_pdev = pdev;

	wakeup_source_init(&pdisc_data->kb_wakelock,
		"swkb_wakelock");
	INIT_DELAYED_WORK(&pdisc_data->kb_channel_work, swkb_channel_com_work);

	init_completion(&pdisc_data->kb_comm_complete);

	pdisc_data->kb_online = KBSTATE_OFFLINE;

	ret = sysfs_create_group(&pdev->dev.kobj, &sw_platform_group);
	if (ret)
		sw_print_err("sysfs_create_group error ret = %d\n", ret);

	sw_init_monitor_device(pdev);

	pdisc_data->fb_state = SCREEN_ON;
	pdisc_data->wait_fb_on = SCREEN_ON;

	fb_register_client(&fb_notify);

	ret = register_reboot_notifier(&plat_poweroff_notifier);
	if (ret) {
		sw_print_err("register_reboot_notifier fail (err=%d)\n", ret);
		goto err_exception;
	}

	ret = kbdev_proxy_register(&sw_kbhubops);
	if (ret) {
		sw_print_err("kbdev_proxy_register fail err=%d\n", ret);
		goto err_exception;
	}

	g_sw_state = SW_STATE_ENABLE;
	return ret;

err_exception:
	sw_core_exit(pdisc_data->core_data);
err_core_init:
	wakeup_source_trash(&pdisc_data->kb_wakelock);
	sw_detectops_destroy(&pdisc_data->detect_ops);
	kfree(pdisc_data);
	return ret;
}

static int sw_remove(struct platform_device *pdev)
{
	struct hw_sw_disc_data *pdisc_data = NULL;

	SW_PRINT_FUNCTION_NAME;

	pdisc_data = dev_get_drvdata(&pdev->dev);
	if (!pdisc_data) {
		sw_print_err("pdisc_data is null\n");
	} else {
		pdisc_data->pm_pdev = NULL;
		sw_core_exit(pdisc_data->core_data);
		sw_detectops_destroy(&pdisc_data->detect_ops);

		kfree(pdisc_data);
		pdisc_data = NULL;
	}

	sw_destroy_monitor_device(pdev);
	return SUCCESS;
}

static int sw_suspend(struct platform_device *pdev, pm_message_t state)
{
	sw_print_dbg("enter\n");
	return SUCCESS;
}

static int sw_resume(struct platform_device *pdev)
{
	sw_print_dbg("enter\n");
	return SUCCESS;
}

#define HW_SW_NAME "huawei,sw_kb"
static const struct of_device_id sw_match_table[] = {
	{
		.compatible = HW_SW_NAME,
		.data = NULL,
	},
	{ },
};

static struct platform_driver sw_platform_driver = {
	.probe = sw_probe,
	.remove = sw_remove,
	.suspend = sw_suspend,
	.resume = sw_resume,
	.driver = {
		.name = "hwsw_kb",
		.owner = THIS_MODULE,
		.of_match_table = sw_match_table,
	},
};

static int __init sw_init(void)
{
	int ret;

	sw_print_info("huawei 3 popo-pin by uart to hid\n");

	ret = sw_drivers_init();
	if (ret) {
		sw_print_err("can't register sw bus\n");
		goto err_ret;
	}

	ret = platform_driver_register(&sw_platform_driver);
	if (ret) {
		sw_print_err("unable to register platform sw driver\n");
		goto err_bus;
	}

	sw_wq = alloc_workqueue("sw_core_wq", WQ_FREEZABLE, 0);
	if (!sw_wq) {
		sw_print_err("unable to alloc sw_core_wq workqueue\n");
		goto err_bus;
	}

	return SUCCESS;

err_bus:
	sw_drivers_exit();
err_ret:
	return ret;
}

static void __exit sw_exit(void)
{
	platform_driver_unregister(&sw_platform_driver);
	sw_drivers_exit();
}

module_init(sw_init);
module_exit(sw_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Huawei 3 Popo-pin core driver by UART");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
