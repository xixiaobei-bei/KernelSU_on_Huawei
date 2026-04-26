/*
 * ring_buffer.c
 *
 * soundtrigger ring buffer implement
 *
 * Copyright (c) 2014-2020 Huawei Technologies Co., Ltd.
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

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <linux/types.h>

struct ring_buffer_itef {
	bool (*empty)(const struct ring_buffer_itef *rb);
	bool (*full)(const struct ring_buffer_itef *rb);
	uint32_t (*get)(struct ring_buffer_itef *rb, void *element, uint32_t size);
	void (*put)(struct ring_buffer_itef *rb, const void *element, uint32_t size);
	void (*deinit)(struct ring_buffer_itef *rb);
};

struct ring_buffer_itef *st_ring_buffer_init(uint32_t element_size, uint32_t element_cnt);

#endif /* __RING_BUFFER_H__ */
