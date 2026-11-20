/* SPDX-License-Identifier: GPL-2.0 */
/*
 * low_latency.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: petrov.maxim@huawei.com
 * Create: 2020-07-06
 *
 */

#ifndef LOW_LATENCY_H
#define LOW_LATENCY_H

#include <linux/atomic.h>
#include <linux/mutex.h>
#include <linux/pm_qos.h>

/*
 * The low-latency request handler is used for all the hmdfs connections to
 * mutiplex latency requests from multiple hmdfs peers.
 */
struct hmdfs_latency_request {
	/* protect against updates from same peer but different code paths */
	struct mutex lock;
	atomic64_t last_update;
	struct pm_qos_request pmqr;
};

/* Use `create` when creating peer connection and `remove` when destroying */
void hmdfs_latency_create(struct hmdfs_latency_request *request);
void hmdfs_latency_remove(struct hmdfs_latency_request *request);

/*
 * Use `update` function to prolong or activate low-latency request and use
 * `cancel` to force deactivation
 */
void hmdfs_latency_update(struct hmdfs_latency_request *request);
void hmdfs_latency_cancel(struct hmdfs_latency_request *request);

#endif /* LOW_LATENCY_H */
