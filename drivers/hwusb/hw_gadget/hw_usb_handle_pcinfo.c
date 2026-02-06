/*
 * hw_usb_handle_pcinfo.c
 *
 * driver file for usb get pc info
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwusb/hw_usb_handle_pcinfo.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>

#define PCINFO_BYTE_OFFSET          8
#define ONE_BYTE                    1
#define TWO_BYTE                    2
#define ENVP_SIZE                   2
#define MAGIC_LEN                   8
#define PCINFO_VENDOR               0xFF
#define PCINFO_MODEL_ID_SIZE        3
#define PCINFO_DEVICE_ID_SIZE       32
#define PCINFO_BT_ADDRESS_SIZE      6
#define PCINFO_HEADER_SIZE          70
#define PCINFO_SIZE                 72
#define PCINFO_OS_INFO_SIZE         1
#define PCINFO_VERSION              0x0100

/* tag 5,6,7 is used for hicar,os info need start from 8 */
enum pcinfo_tlv_e {
	PCINFO_CMMU_TYPE,
	PCINFO_MODEL_ID,
	PCINFO_SUBMODEL_ID,
	PCINFO_DEVICE_ID,
	PCINFO_BTADDR,
	PCINFO_OS_INFO = 8,
	PCINFO_TLV_MAX,
};

struct pcinfo_parser_st {
	u8 cmmu_type;
	u8 type;
	u8 vendor;
	u8 sub_model;
	u8 os_info;
};

struct pcinfo_tlv_st {
	u8 tag;
	u8 len;
	u8 *val;
};

struct pcinfo_st {
	u8 *magic;
	u8 *total_len;
	u8 *type;
	u8 *version;
	u8 *sub_type;
	struct pcinfo_tlv_st tlv[PCINFO_TLV_MAX];
};

/* hilink tlv data */
/* 0x07 means pc type */
static u8 g_modelid[PCINFO_MODEL_ID_SIZE] = { 0x00, 0x00, 0x07 };
static u8 g_deviceid[PCINFO_DEVICE_ID_SIZE];
static u8 g_bt_addr[PCINFO_BT_ADDRESS_SIZE];
static u8 g_pcinfo_data[PCINFO_SIZE];
static u8 g_magic[MAGIC_LEN] = { 'h', 'w', 'n', 'e', 'a', 'r', 'b', 'y' };
static u8 g_total_len[TWO_BYTE] = { PCINFO_SIZE, 0x0 };

/* pcinfo version 0x0100 */
static u8 g_version[TWO_BYTE] = { 0x00, 0x01 };
static struct device *g_pcinfo_device;

static int send_data(void)
{
	int ret = -1;
	char *envp[ENVP_SIZE] = { "HW_USB_GET_PCINFO=SEND", NULL };

	if (!g_pcinfo_device) {
		pr_info("hw_usb_device is NULL\n");
		return ret;
	}

	ret = kobject_uevent_env(&g_pcinfo_device->kobj, KOBJ_CHANGE, envp);
	if (!ret)
		pr_info("sending HW_USB_GET_PCINFO=SEND ok\n");
	else
		pr_err("sending HW_USB_GET_PCINFO=SEND failed %d\n", ret);

	return ret;
}

static void send_data_wq(struct work_struct *data)
{
	(void *)data;
	send_data();
}
DECLARE_WORK(send_data_work, send_data_wq);

static inline void catenate_buf(u8 *buf, u8 len, u8 *offset)
{
	memcpy(g_pcinfo_data + *offset, buf, len);
	*offset += len;
}

static void assemble_pcinfo(struct pcinfo_st *pcinfo)
{
	int i;
	u8 offset = 0;

	memset(g_pcinfo_data, 0x0, PCINFO_SIZE);
	catenate_buf(pcinfo->magic, MAGIC_LEN, &offset);
	catenate_buf(pcinfo->total_len, TWO_BYTE, &offset);
	catenate_buf(pcinfo->type, TWO_BYTE, &offset);
	catenate_buf(pcinfo->version, TWO_BYTE, &offset);
	catenate_buf(pcinfo->sub_type, TWO_BYTE, &offset);

	for (i = 0; i < PCINFO_TLV_MAX; i++) {
		if (pcinfo->tlv[i].len == 0)
			continue;

		catenate_buf(&(pcinfo->tlv[i].tag), ONE_BYTE, &offset);
		catenate_buf(&(pcinfo->tlv[i].len), ONE_BYTE, &offset);
		catenate_buf(pcinfo->tlv[i].val, pcinfo->tlv[i].len, &offset);
	}

	for (i = 0; i < PCINFO_SIZE; i++)
		pr_info("%02X ", g_pcinfo_data[i]);

	pr_info("\n");
}

static int parse_pcinfo(u16 w_value, u16 w_index, u16 w_length)
{
	int i;
	struct pcinfo_st pcinfo;
	struct pcinfo_parser_st pcinfo_parser;

	pcinfo_parser.cmmu_type = (u8)w_length;
	pcinfo_parser.type = (u8)(w_value >> PCINFO_BYTE_OFFSET);
	pcinfo_parser.vendor = (u8)w_value;
	pcinfo_parser.sub_model = (u8)(w_index >> PCINFO_BYTE_OFFSET);
	pcinfo_parser.os_info = (u8)w_index;

	if (pcinfo_parser.vendor != PCINFO_VENDOR)
		return -1;

	memset(&pcinfo, 0x0, sizeof(pcinfo));
	pcinfo.magic = g_magic;
	pcinfo.total_len = g_total_len;
	pcinfo.type = &pcinfo_parser.type;
	pcinfo.version = g_version;
	pcinfo.sub_type = &pcinfo_parser.type;

	for (i = 0; i < PCINFO_TLV_MAX; i++)
		pcinfo.tlv[i].tag = i;

	pcinfo.tlv[PCINFO_CMMU_TYPE].len = ONE_BYTE;
	pcinfo.tlv[PCINFO_MODEL_ID].len = PCINFO_MODEL_ID_SIZE;
	pcinfo.tlv[PCINFO_SUBMODEL_ID].len = ONE_BYTE;
	pcinfo.tlv[PCINFO_DEVICE_ID].len = PCINFO_DEVICE_ID_SIZE;
	pcinfo.tlv[PCINFO_BTADDR].len = PCINFO_BT_ADDRESS_SIZE;
	pcinfo.tlv[PCINFO_OS_INFO].len = PCINFO_OS_INFO_SIZE;

	pcinfo.tlv[PCINFO_CMMU_TYPE].val = &pcinfo_parser.cmmu_type;
	pcinfo.tlv[PCINFO_MODEL_ID].val = g_modelid;
	pcinfo.tlv[PCINFO_SUBMODEL_ID].val = &pcinfo_parser.sub_model;
	pcinfo.tlv[PCINFO_DEVICE_ID].val = g_deviceid;
	pcinfo.tlv[PCINFO_BTADDR].val = g_bt_addr;
	pcinfo.tlv[PCINFO_OS_INFO].val = &pcinfo_parser.os_info;

	assemble_pcinfo(&pcinfo);
	return 0;
}

void hw_usb_handle_pcinfo(u16 w_value, u16 w_index, u16 w_length)
{
	int ret = parse_pcinfo(w_value, w_index, w_length);

	if (!ret)
		schedule_work(&send_data_work);
}
EXPORT_SYMBOL(hw_usb_handle_pcinfo);

static ssize_t pcinfo_read(struct file *file, char __user *buf, size_t count,
	loff_t *pos)
{
	ssize_t ret = copy_to_user(buf, (void *)g_pcinfo_data, PCINFO_SIZE);

	if (ret)
		pr_err("pcinfo read copy to user failed\n");
	else
		pr_info("pcinfo read copy to user success\n");

	return ret;
}

static const struct file_operations pcinfo_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = pcinfo_read,
};

static struct miscdevice pcinfo_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "usb_pcinfo_common",
	.fops = &pcinfo_fops,
};

int hw_pcinfo_create_device(struct device *dev, struct class *usb_class)
{
	int ret;

	g_pcinfo_device = dev;
	ret = misc_register(&pcinfo_miscdev);
	if (ret)
		pr_err("register pcinfo miscdev fail, ret:%d\n", ret);

	return 0;
}
EXPORT_SYMBOL(hw_pcinfo_create_device);
