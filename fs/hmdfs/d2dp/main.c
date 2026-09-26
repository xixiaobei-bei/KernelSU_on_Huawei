// SPDX-License-Identifier: GPL-2.0
/*
 * D2D-over-UDP protocol main entry point
 */

#include <linux/module.h>

#include "kobject.h"

static int __init d2d_protocol_init(void)
{
	int err = 0;

	err = d2dp_kobject_init();
	if (err) {
		pr_err("cannot init D2DP kobj subsystem: %d\n", err);
		goto out;
	}

	pr_info("D2D Protocol is initialized\n");
out:
	return err;
}

static void __exit d2d_protocol_exit(void)
{
	d2dp_kobject_deinit();
	pr_info("D2D Protocol is deinitialized\n");
}

module_init(d2d_protocol_init);
module_exit(d2d_protocol_exit);

MODULE_DESCRIPTION("D2D Protocol transport module");
MODULE_AUTHOR("Huawei RRI (Software OS lab) Networking team");
MODULE_LICENSE("GPL");
