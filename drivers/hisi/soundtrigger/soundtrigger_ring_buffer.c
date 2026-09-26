/*
 * ring_buffer.c
 *
 * soundtrigger ring_buffer implement
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

#include <linux/vmalloc.h>
#include <linux/hisi/audio_log.h>
#include "soundtrigger_ring_buffer.h"

#define LOG_TAG "soundtrigger"

struct ring_buffer {
	struct ring_buffer_itef itef;
	uint32_t size;
	uint32_t start;
	uint32_t count;
	uint32_t element_size;
	char buffer[0];
};

static bool ring_buffer_is_empty(const struct ring_buffer_itef *itef)
{
	const struct ring_buffer *rb = (const struct ring_buffer *)itef;

	return (rb->count == 0);
}

static bool ring_buffer_is_full(const struct ring_buffer_itef *itef)
{
	const struct ring_buffer *rb = (const struct ring_buffer *)itef;

	return (rb->count == rb->size);
}

static uint32_t ring_buffer_get(struct ring_buffer_itef *itef, void *buffer, uint32_t size)
{
	uint32_t ret = 0;
	struct ring_buffer *rb = (struct ring_buffer *)itef;

	if (size != rb->element_size) {
		AUDIO_LOGE("buffer size: %u not equal to element size: %u", size, rb->element_size);
		return 0;
	}

	if (!ring_buffer_is_empty(&rb->itef)) {
		memcpy(buffer, rb->buffer + rb->start * rb->element_size, rb->element_size);/*lint !e679*/
		rb->start = (rb->start + 1) % rb->size;
		--rb->count;
		ret = rb->element_size;
	}

	return ret;
}

static void ring_buffer_put(struct ring_buffer_itef *itef, const void *buffer, uint32_t size)
{
	struct ring_buffer *rb = (struct ring_buffer *)itef;
	uint32_t end = (rb->start + rb->count) % rb->size;

	if (size != rb->element_size) {
		AUDIO_LOGE("buffer size: %u not equal to element size: %u", size, rb->element_size);
		return;
	}

	memcpy(rb->buffer + end * rb->element_size, buffer, rb->element_size);/*lint !e679*/

	if (ring_buffer_is_full(&rb->itef)) {
		rb->start = (rb->start + 1) % rb->size; /* full, overwrite */
	} else {
		++rb->count;
	}
}

static void ring_buffer_deinit(struct ring_buffer_itef *itef)
{
	struct ring_buffer *rb = (struct ring_buffer *)itef;

	memset(rb, 0, sizeof(struct ring_buffer) + rb->element_size * rb->size);

	vfree(rb);
}

struct ring_buffer_itef *st_ring_buffer_init(uint32_t element_size, uint32_t element_cnt)
{
	uint32_t size = sizeof(struct ring_buffer) + element_size * element_cnt;
	struct ring_buffer *rb = NULL;

	if (element_size == 0 || element_cnt == 0) {
		AUDIO_LOGE("invalid element para, size: %u, cnt: %u", element_size, element_cnt);
		return NULL;
	}

	rb = vzalloc(size);
	if (rb == NULL)
		return NULL;

	rb->element_size = element_size;
	rb->size = element_cnt;
	rb->start = 0;
	rb->count = 0;

	rb->itef.empty = ring_buffer_is_empty;
	rb->itef.full = ring_buffer_is_full;
	rb->itef.get = ring_buffer_get;
	rb->itef.put = ring_buffer_put;
	rb->itef.deinit = ring_buffer_deinit;

	return &rb->itef;
}
