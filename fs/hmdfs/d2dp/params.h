/* SPDX-License-Identifier: GPL-2.0 */
/*
 * D2D protocol customization parameters
 */

#ifndef D2D_PARAMS_H
#define D2D_PARAMS_H

#include <linux/time.h>
#include <linux/types.h>

/* Number of RTO retries before delay for resend starts increasing */
#define D2D_RETRY_LIMIT_MIN 3
/* Number of RTO retries which leads to connection drop */
#define D2D_RETRY_LIMIT_MAX 10

/* Packet format */
#define UDP_DATAGRAM_SIZE 1472
#define SACK_PAIRS_MAX    100

/* Buffer size params */
#define D2D_DEFAULT_BUFFER_SIZE (6  * (1UL << 20))          /* 6M  */
#define D2D_BUFFER_SIZE_MIN     (4  * (1UL << 10))          /* 4K  */
#define D2D_BUFFER_SIZE_MAX     (16 * (1UL << 20))          /* 16M */

/* RX buffer allowed fill percentage */
#define D2D_BUFFER_ALLOWED_FILL_PERCENTAGE 90

/* ACK timer period params */
#define D2D_DEFAULT_ACK_PERIOD_USEC (5U   * USEC_PER_MSEC)  /* 5ms  */
#define D2D_ACK_PERIOD_MIN_USEC     (1U   * USEC_PER_MSEC)  /* 1ms  */
#define D2D_ACK_PERIOD_MAX_USEC     (100U * USEC_PER_SEC)   /* 100s */

/* RTO timer period params (10 times longer) */
#define D2D_DEFAULT_RTO_PERIOD_USEC (10U * D2D_DEFAULT_ACK_PERIOD_USEC)
#define D2D_RTO_PERIOD_MIN_USEC     (10U * D2D_ACK_PERIOD_MIN_USEC)
#define D2D_RTO_PERIOD_MAX_USEC     (10U * D2D_ACK_PERIOD_MAX_USEC)

/* How much time we wait until protocol can be forcefully destroyed */
#define D2D_DESTROY_TIMEOUT_MSEC (10U * MSEC_PER_SEC)

#define FAST_CORE_6 6
#define FAST_CORE_7 7

/* Protocol version */
#define D2D_PROTOCOL_VERSION_MASK	0xF000  /* first 4 flag bits */
#define D2D_PROTOCOL_VERSION		0

#endif /* D2D_PARAMS_H */
