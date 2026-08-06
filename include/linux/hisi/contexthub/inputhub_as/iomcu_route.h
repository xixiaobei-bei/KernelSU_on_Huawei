/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019. All rights reserved.
 * Description: iomcu_route.h.
 * Create: 2019/11/05
 */
#ifndef __IOMCU_ROUTE_H
#define __IOMCU_ROUTE_H
#include <linux/completion.h>
#include <linux/mutex.h>
#include <linux/hisi/contexthub/protocol_as.h>

#define LENGTH_SIZE sizeof(unsigned int)
#define HEAD_SIZE (LENGTH_SIZE + TIMESTAMP_SIZE)

struct t_head {
	int for_alignment;
	union {
		char effect_addr[sizeof(int)];
		int pkg_length;
	};
	s64 timestamp;
};

struct inputhub_buffer_pos {
	char *pos;
	unsigned int buffer_size;
};

struct inputhub_route_table {
	char *channel_name;
	struct inputhub_buffer_pos p_head; /* point to the head of buffer */
	struct inputhub_buffer_pos p_read; /* point to read position of buffer */
	struct inputhub_buffer_pos p_write; /* point to write position of buffer */
	wait_queue_head_t read_wait; /* to block read when no data in buffer */
	bool is_reading; /* for multiple thread read buffer */
	spinlock_t buffer_spin_lock; /* for read write buffer */
};

/*
 * Function    : inputhub_route_open
 * Description : alloc route item kernel space
 * Input       : [route_item] manager for kernel buf
 * Output      : none
 * Return      : 0 OK, other error
 */
int inputhub_route_open(struct inputhub_route_table *route_item);

/*
 * Function    : inputhub_route_close
 * Description : free route item kernel space
 * Input       : [route_item] manager for kernel buf
 * Output      : none
 * Return      : 0 OK, other error
 */
void inputhub_route_close(struct inputhub_route_table *route_item);

/*
 * Function    : inputhub_route_read
 * Description : read report data from kernel buffer, copy to user buffer
 * Input       : [route_item] manager for kernel buf
 *             : [buf] user buffer
 *             : [count] user buffer size
 * Output      : none
 * Return      : read buffer bytes actually
 */
ssize_t inputhub_route_read(struct inputhub_route_table *route_item,
			    char __user *buf, size_t count);

/*
 * Function    : inputhub_route_write
 * Description : write report data to kernel buffer, use internal timestamp
 * Input       : [route_item] manager for kernel buf
 *             : [buf] data buffer
 *             : [count] data buffer size
 * Output      : none
 * Return      : write buffer bytes actually
 */
ssize_t inputhub_route_write(struct inputhub_route_table *route_item,
			     const char *buf, size_t count);

/*
 * Function    : inputhub_route_write_batch
 * Description : write report data to kernel buffer, use external timestamp
 * Input       : [route_item] manager for kernel buf
 *             : [buf] data buffer
 *             : [count] data buffer size
 *             : [timestamp] external timestamp
 * Output      : none
 * Return      : write buffer bytes actually
 */
ssize_t inputhub_route_write_batch(struct inputhub_route_table *route_item,
				   const char *buf, size_t count, int64_t timestamp);
#endif /* __IOMCU_ROUTE_H */
