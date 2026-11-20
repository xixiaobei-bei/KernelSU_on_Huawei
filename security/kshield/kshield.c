/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: the kshield.c implements the module init and exit code
 * Author: ducuixia <ducuixia@huawei.com>
 * Create: 2019-11-09
 */

#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/rcupdate.h>
#include <linux/slab.h>

#include "common.h"
#include "hook_charact.h"
#include "hook_sock_spray.h"
#include "hook_spray.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MOBILE SECURITY TEAM");

static int __init kshield_hook_init(void)
{
	int ret;

	ks_info("kshield hook cha init start\n");
	ret = hook_cha_init();
	if (ret)
		goto out_ret;

	ks_info("kshield hook spray init start\n");
	ret = hook_spray_init();
	if (ret)
		goto out_cha;

	ks_info("kshield hook sock spray init start\n");
	ret = hook_sock_spray_init();
	if (ret)
		goto out_spray;

	ks_info("kshield hook init success\n");
	return 0;

out_spray:
	hook_spray_deinit();
out_cha:
	hook_cha_deinit();
out_ret:
	ks_err("kshield hook init failed, ret = %d\n", ret);
	return ret;
}

static void __exit kshield_hook_deinit(void)
{
	hook_sock_spray_deinit();
	hook_spray_deinit();
	hook_cha_deinit();
	ks_debug("kshield hook deinit success\n");
}

device_initcall(kshield_hook_init);
module_exit(kshield_hook_deinit);
