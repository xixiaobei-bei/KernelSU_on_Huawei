/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: codec reg check in call
 * Author: lijinkui
 * Create: 2020-01-10
 */

#ifndef __CODEC_CHECK_H__
#define __CODEC_CHECK_H__

#define int_to_addr(low, high) ((void *)(uintptr_t)((unsigned long long)(low) | ((unsigned long long)(high) << 32)))

#define IOCTL_CODEC_CHECK_SET_PARAM  _IOWR('C', 0x00, struct drv_io_async_param)
#define IOCTL_CODEC_CHECK_TEST       _IOWR('C', 0x01, struct drv_io_async_param)
#define MAX_REG_NUM    20

struct codec_check_node {
	unsigned int reg;
	unsigned int val;
	unsigned int mask;
};

struct codec_check_data {
	int scene_num;
	struct codec_check_node node[MAX_REG_NUM];
};

struct error_reg {
	unsigned int reg;
	unsigned int val;
	unsigned int real_val;
};

struct drv_io_async_param {
	unsigned int para_in_l;        /* input buffer addr low */
	unsigned int para_in_h;        /* input buffer addr high */
	unsigned int para_size_in;     /* input buffer length */
	unsigned int para_out_l;       /* output buffer addr low */
	unsigned int para_out_h;       /* output buffer addr high */
	unsigned int para_size_out;    /* output buffer length */
};

#endif // __CODEC_CHECK_H__
