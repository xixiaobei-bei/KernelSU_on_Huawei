/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description:  this file difine variable node hisi-ivp
 * Author:  chenweiyu
 * Create:  2019-08-23
 */
#ifndef _IVP_IOCTL_H_
#define _IVP_IOCTL_H_
#include <linux/fs.h>
#include "ivp_platform.h"

struct file_header {
	char name[4]; // length of "IVP:"
	char time[20];// length of time
	unsigned int image_size;
	unsigned int sect_count;
};

struct image_section_header {
	unsigned short index;
	unsigned char type;
	unsigned char attribute;
	unsigned int offset;
	unsigned int vaddr;
	unsigned int size;
};

enum {
	IMAGE_SECTION_TYPE_EXEC = 0,
	IMAGE_SECTION_TYPE_DATA,
	IMAGE_SECTION_TYPE_BSS,
};

#define IVP_IMAGE_SUFFIX                  ".bin"
#define IVP_IMAGE_SUFFIX_LENGTH          (sizeof(IVP_IMAGE_SUFFIX) - 1)
void ivp_dev_poweroff(struct ivp_device *devp);
long ivp_ioctl(struct file *fd, unsigned int cmd, unsigned long args);
#endif /* IVP_IOCTL_H */
