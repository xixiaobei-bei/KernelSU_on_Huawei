/*
 * This file difine variable node dev-ivp
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _IVP_IOCTL_H_
#define _IVP_IOCTL_H_
#include <linux/fs.h>
#include "ivp_platform.h"

struct file_header {
	char name[4];  /* length of "IVP:" */
	char time[20]; /* length of time */
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

#ifdef MULTIPLE_ALGO
#define ELF_MAGIC0    0  /* 0-3 are magic '0x7f' */
#define ELF_MAGIC1    1  /* 'E' */
#define ELF_MAGIC2    2  /* 'L' */
#define ELF_MAGIC3    3  /* 'F' */

enum IVP_ALGO_SEGMENT {
	SEGMENT_DDR_TEXT = 0,
	SEGMENT_IRAM_TEXT,
	SEGMENT_DRAM0_DATA,
	SEGMENT_DRAM0_BSS,
	SEGMENT_MAX
};
#endif

void ivp_poweroff(struct ivp_device *pdev);
long ivp_ioctl(struct file *fd, unsigned int cmd, unsigned long args);
#endif /* _IVP_IOCTL_H_ */
