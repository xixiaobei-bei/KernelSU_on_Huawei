/*
 * f_mass_storage_hisi.c
 *
 * mass_storage support for mac system and configfs
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * either version 2 of that License or (at your option) any later version.
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <linux/hisi/usb/android_gadget.h>

#define FILE_MODE 0644

#ifdef CONFIG_USB_MASS_STORAGE_SUPPORT_MAC
/* Added for support mac system begin */
/* usbsdms_read_toc_data1 rsp packet */
static u8 usbsdms_read_toc_data1[] = {
	0x00, 0x0A, 0x01, 0x01,
	0x00, 0x14, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00
};

/* usbsdms_read_toc_data1_format0000 rsp packet */
static u8 usbsdms_read_toc_data1_format0000[] = {
	0x00, 0x12, 0x01, 0x01,
	0x00, 0x14, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* the last four bytes:32MB */
	0x00, 0x14, 0xAA, 0x00, 0x00, 0x00, 0xFF, 0xFF
};

/* usbsdms_read_toc_data1_format0001 rsp packet */
static u8 usbsdms_read_toc_data1_format0001[] = {
	0x00, 0x0A, 0x01, 0x01,
	0x00, 0x14, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* usbsdms_read_toc_data2 rsp packet */
static u8 usbsdms_read_toc_data2[] = {
	0x00, 0x2e, 0x01, 0x01,
	0x01, 0x14, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
	0x01, 0x14, 0x00, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
	0x01, 0x14, 0x00, 0xa2, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x3c,
	/* ^ CDROM size from this byte */
	0x01, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00
};

/* usbsdms_read_toc_data3 rsp packet */
static u8 usbsdms_read_toc_data3[] = {
	0x00, 0x12, 0x01, 0x01,
	0x00, 0x14, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* READ_TOC command structure */
struct usbsdms_read_toc_cmd_type {
	u8  op_code;
	/*
	 * bit1 is MSF,
	 * 0: address format is LBA form
	 * 1: address format is MSF form
	 */
	u8  msf;
	/*
	 * bit3~bit0,   MSF Field   Track/Session Number
	 * 0000b:       Valid       Valid as a Track Number
	 * 0001b:       Valid       Ignored by Drive
	 * 0010b:       Ignored     Valid as a Session Number
	 * 0011b~0101b: Ignored     Ignored by Drive
	 * 0110b~1111b: Reserved
	 */
	u8  format;
	u8  reserved1;
	u8  reserved2;
	u8  reserved3;
	u8  session_num; /* a specific session or a track */
	u8  allocation_length_msb;
	u8  allocation_length_lsb;
	u8  control;
};
/* description : response for command READ TOC */
static int do_read_toc(struct fsg_common *common, struct fsg_buffhd *bh)
{
	u8 *buf = (u8 *) bh->buf;
	struct usbsdms_read_toc_cmd_type *read_toc_cmd = NULL;
	unsigned long response_length = 0;
	u8 *response_ptr = NULL;

	read_toc_cmd = (struct usbsdms_read_toc_cmd_type *)common->cmnd;

	/*
	 * When TIME is set to one, the address fields in some returned
	 * data formats shall be in TIME form.
	 * 2 is time form mask.
	 */
	if (read_toc_cmd->msf == 2) {
		response_ptr = usbsdms_read_toc_data2;
		response_length = sizeof(usbsdms_read_toc_data2);
	} else if (read_toc_cmd->allocation_length_msb != 0) {
		response_ptr = usbsdms_read_toc_data3;
		response_length = sizeof(usbsdms_read_toc_data3);
	} else {
		/*
		 * When TIME is set to zero, the address fields in some returned
		 * data formats shall be in LBA form.
		 */
		if (read_toc_cmd->format == 0) {
			/* 0 is mean to valid as a Track Number */
			response_ptr = usbsdms_read_toc_data1_format0000;
			response_length =
				sizeof(usbsdms_read_toc_data1_format0000);
		} else if (read_toc_cmd->format == 1) {
			/* 1 is mean to ignored by Logical Unit */
			response_ptr = usbsdms_read_toc_data1_format0001;
			response_length =
				sizeof(usbsdms_read_toc_data1_format0001);
		} else {
			/* Valid as a Session Number */
			response_ptr = usbsdms_read_toc_data1;
			response_length = sizeof(usbsdms_read_toc_data1);
		}
	}

	memcpy(buf, response_ptr, response_length);

	if (response_length < common->data_size_from_cmnd)
		common->data_size_from_cmnd = response_length;

	common->data_size = common->data_size_from_cmnd;

	common->residue = common->usb_amount_left = common->data_size;

	return response_length;
}
#endif

#ifdef CONFIG_HISI_USB_CONFIGFS

static ssize_t mass_storage_inquiry_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct usb_function_instance *fi = dev_get_drvdata(dev);
	struct fsg_opts *opts = fsg_opts_from_func_inst(fi);

	return snprintf(buf, PAGE_SIZE, "%s\n", opts->common->inquiry_string);
}

static ssize_t mass_storage_inquiry_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct usb_function_instance *fi = dev_get_drvdata(dev);
	struct fsg_opts *opts = fsg_opts_from_func_inst(fi);
	int len;

	len = min(size, sizeof(opts->common->inquiry_string) - 1);

	strncpy(opts->common->inquiry_string, buf, len);

	opts->common->inquiry_string[len] = 0;

	return size;
}

static DEVICE_ATTR(inquiry_string, FILE_MODE,
					mass_storage_inquiry_show,
					mass_storage_inquiry_store);

static struct device_attribute *mass_storage_function_attributes[] = {
	&dev_attr_inquiry_string,
	NULL
};

static int create_mass_storage_device(struct usb_function_instance *fi)
{
	struct device *dev = NULL;
	struct device_attribute **attrs = NULL;
	struct device_attribute *attr = NULL;
	int err = 0;

	dev = create_function_device("f_mass_storage");
	if (IS_ERR(dev))
		return PTR_ERR(dev);

	attrs = mass_storage_function_attributes;
	if (attrs != NULL) {
		attr = *attrs;
		while ((attr != NULL) && !err) {
			err = device_create_file(dev, attr);
			attrs++;
			attr = *attrs;
		}
		if (err) {
			device_destroy(dev->class, dev->devt);
			return -EINVAL;
		}
	}
	dev_set_drvdata(dev, fi);
	return 0;
}
#endif
