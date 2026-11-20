// SPDX-License-Identifier: GPL-2.0
/*
 * low_latency.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: petrov.maxim@huawei.com
 * Create: 2020-07-06
 *
 */

#include <linux/atomic.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/pm_qos.h>

#include "low_latency.h"

#define LOW_LATENCY_REQUEST_VALUE 0
#define LOW_LATENCY_TIMEOUT_USEC  8000000U  /* 8 seconds */

static unsigned int low_latency_ignore;
module_param(low_latency_ignore, uint, S_IRUGO | S_IWUSR);

/* Register the PM-QoS request with default value in the system */
void hmdfs_latency_create(struct hmdfs_latency_request *request)
{
	mutex_init(&request->lock);
	atomic64_set(&request->last_update, jiffies);
	pm_qos_add_request(&request->pmqr,
			   PM_QOS_NETWORK_LATENCY,
			   PM_QOS_DEFAULT_VALUE);
}

/* Remove the PM-QoS request from the system */
void hmdfs_latency_remove(struct hmdfs_latency_request *request)
{
	pm_qos_remove_request(&request->pmqr);
}

/* Prolong the low-latency request if exists or create one otherwise */
void hmdfs_latency_update(struct hmdfs_latency_request *request)
{
	unsigned long last = atomic64_read(&request->last_update);
	unsigned long curr = jiffies;

	if (low_latency_ignore)
		return;

	/* optimization: too early to update request */
	if (jiffies_to_usecs(curr - last) < LOW_LATENCY_TIMEOUT_USEC / 2)
		return;

	/*
	 * When locked, then another connection is already updating the request,
	 * and we don't need to notify PM-QoS subsystem twice.
	 */
	if (mutex_trylock(&request->lock)) {
		pm_qos_update_request_timeout(&request->pmqr,
					      LOW_LATENCY_REQUEST_VALUE,
					      LOW_LATENCY_TIMEOUT_USEC);
		atomic64_set(&request->last_update, curr);
		mutex_unlock(&request->lock);
	}
}

/* Force cancel the request by updating to the default value */
void hmdfs_latency_cancel(struct hmdfs_latency_request *request)
{
	mutex_lock(&request->lock);
	pm_qos_update_request(&request->pmqr, PM_QOS_DEFAULT_VALUE);
	mutex_unlock(&request->lock);
}
