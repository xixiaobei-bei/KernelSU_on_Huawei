/*
 * Copyright (c) 2013-2014 NVIDIA Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/list.h>
#include <linux/completion.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#include <asm/page.h>
#include <linux/string.h>

#include "hhee.h"

static int hhee_logging_enabled;
struct circular_buffer *cb;
struct circular_buffer *crash_cb;
struct circular_buffer *monitor_cb;

/*
 * Initialize the shared buffer for HHEE logging.
 * The shared buffer is allocated in DMA memory to get uncached memory
 * since TLK directly writes to the physical address of the shared buffer.
 * The structure is declared in DMA memory too since it's members will
 * also be updated by the TLK directly to their physical addresses.
 */
static int circ_buf_init(struct circular_buffer *cir_buf)
{
	struct circular_buffer *tmp_cb = (struct circular_buffer *)cir_buf->virt_log_addr;

	if (0 == cir_buf->inited) {
		memset(cir_buf, 0, sizeof(struct circular_buffer));/* unsafe_function_ignore: memset */
		cir_buf->virt_log_addr = (uint64_t)tmp_cb;
		cir_buf->start = tmp_cb->start;
		cir_buf->inited = 1;
	} else {
		cir_buf->start = cir_buf->end;
	}

	cir_buf->size  = tmp_cb->size;
	cir_buf->end = tmp_cb->end;
	cir_buf->overflow = tmp_cb->overflow;
	cir_buf->buf = (char *)(tmp_cb);

	pr_info("[HHEE] Logging, start addr is %lx, buf size is %lx, end is %lx\n", cir_buf->start, cir_buf->size, cir_buf->end);
	return 0;
}

/*
 * Copy the contents of the circular buffer into a char buffer in order.
 * This helps to treat the buffer like a string and use it to tokenize it
 * into lines, tag and display it.
 */
ssize_t circ_buf_copy(struct circular_buffer *incb, char *text)
{
	size_t len = 0;
	unsigned long tmpsize;

	if (incb->end == incb->start)
		return 0;

	tmpsize = (unsigned long)(long long)abs((long)(incb->end - incb->start));
	if (tmpsize > incb->size) {
		pr_err("%s: cbuf pointers corrupted\n", __func__);
		pr_err("log end is %lx, start is %lx, size is %lx\n", incb->end, incb->start, incb->size);
		return -EINVAL;
	}

	if (incb->end > incb->start) {
		len += incb->end - incb->start;
		memcpy(text, incb->buf + incb->start, incb->end - incb->start);/* unsafe_function_ignore: memcpy */

	} else if (incb->start > incb->end) {
		memcpy(text, incb->buf + incb->start, incb->size - incb->start);/* unsafe_function_ignore: memcpy */
		len += incb->size - incb->start;
		memcpy(text + incb->size - incb->start, incb->buf, incb->end);/* unsafe_function_ignore: memcpy */
		len += incb->end;
	}

	/* Indicate that buffer is empty */
	incb->start = incb->end;

	return (ssize_t)len;
}

ssize_t hhee_copy_logs(char __user *buf, size_t count,
		       loff_t *offp, int logtype)
{
	ssize_t len;
	char *buffer;
	struct circular_buffer *incb;

	if (!hhee_logging_enabled)
		return 0;

	if (NORMAL_LOG == logtype)
		incb = cb;
	else if(CRASH_LOG == logtype)
		incb = crash_cb;
	else if(MONITOR_LOG == logtype)
		incb = monitor_cb;
	else {
		pr_err("logtype err\n");
		return -EINVAL;
	}

	circ_buf_init(incb);

	buffer = (char *)kzalloc(incb->size, GFP_KERNEL);
	if (!buffer) {
		pr_err("kzalloc failed\n");
		return -ENOMEM;
	}

	/* This detects if the buffer proved to be too small to hold the data.
	 * If buffer is not large enough, it overwrites it's oldest data,
	 * This warning serves to alert the user to possibly use a bigger buffer
	 */
	if (incb->overflow == 1) {
		pr_info("\n[HHEE] buffer overwritten.\n\n");
		incb->overflow = 0;
	}

	if ((len = circ_buf_copy(incb, buffer)) < 0) {
		kfree(buffer);
		return -EFAULT;
	}
	incb->buf[incb->end] = '\0';

	len = simple_read_from_buffer(buf, count, offp,
				buffer, (unsigned long)len);

	kfree(buffer);
	return len;
}

int cb_init(uint64_t inlog_addr, uint64_t inlog_size,
			struct circular_buffer **incb, unsigned int logtype)
{
	uint64_t log_addr;
	struct circular_buffer *tmp_cb;

	log_addr = (uint64_t)ioremap_cache(inlog_addr, inlog_size);
	if (!log_addr) {
		pr_err("fail to get virtal addr of hhee\n");
		return -EINVAL;
	}

	tmp_cb = (struct circular_buffer *) kzalloc(sizeof(struct circular_buffer),
						    GFP_KERNEL);
	if (!tmp_cb) {
		iounmap((void __iomem *)log_addr);
		pr_err("%s: no memory avaiable for circular buffer struct\n",
			__func__);
		return -ENOMEM;
	}

	tmp_cb->virt_log_addr = log_addr;
	tmp_cb->virt_log_size = inlog_size;
	tmp_cb->logtype = logtype;
	*incb = tmp_cb;
	return 0;
}

void cb_deinit(struct circular_buffer *incb)
{
	if (incb && incb->virt_log_addr)
		iounmap((void __iomem *)incb->virt_log_addr);
	if (incb)
		kfree(incb);
}
/*
 * Call function to initialize circular buffer.
 * An HVC is made to send the virtual address of the structure to
 * HHEE
 */
int hhee_logger_init(void)
{
	struct arm_smccc_res ret_res;
	int ret;

	pr_info("hhee logger init\n");
	/* get logging information*/
	/*normal buffer*/
	ret_res = hhee_fn_hvc((unsigned long)HHEE_LOGBUF_INFO, 0ul, 0ul, 0ul);
	ret = cb_init(ret_res.a0, ret_res.a1, &cb, NORMAL_LOG);
	if (ret)
		goto outfail1;

	/*crash buffer*/
	ret_res = hhee_fn_hvc((unsigned long)HHEE_CRASHLOG_INFO, 0ul, 0ul, 0ul);
	ret = cb_init(ret_res.a0, ret_res.a1, &crash_cb, CRASH_LOG);
	if (ret)
		goto outfail2;

	/*monitor buffer*/
	ret_res = hhee_fn_hvc((unsigned long)HHEE_PMFBUFLOG_INFO, 0ul, 0ul, 0ul);
	ret = cb_init(ret_res.a0, ret_res.a1, &monitor_cb, MONITOR_LOG);
	if (ret)
		goto outfail3;

	hhee_logging_enabled = 1;

	pr_info("HHEE logger init done\n");
	return 0;
outfail3:
	cb_deinit(crash_cb);
outfail2:
	cb_deinit(cb);
outfail1:
	pr_info("HHEE log init failed!\n");
	return -EINVAL;
}

