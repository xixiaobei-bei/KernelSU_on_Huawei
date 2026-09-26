/**
 * @file cmdlist_test.c
 * @brief test unit for cmdlist
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "dkmd_cmdlist.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "Kunit.h"
#include "Stub.h"

extern int cmdlist_scene_client_commit(int scene_id);

void test_cmdlist_flush(void)
{
	CHECK_EQUAL(cmdlist_scene_client_commit(0), 0);
}

KU_TestInfo test_cmdlist[] = {
	{ "test_cmdlist_flush", test_cmdlist_flush },
	KU_TEST_INFO_NULL
};

int cmdlist_test_suite_init(void)
{
	pr_info("cmdlist device init!\n");
	return 0;
}

int cmdlist_test_suite_clean(void)
{
	pr_info("cmdlist device deinit!\n");
	return 0;
}

KU_SuiteInfo cmdlist_test_suites[]=
{
	{"cmdlist_test_init", cmdlist_test_suite_init, cmdlist_test_suite_clean, test_cmdlist, KU_TRUE},
	KU_SUITE_INFO_NULL
};

static int cmdlist_test_init(void)
{
	pr_info("+++++++++++++++++++++++++++++++++++++++ hello, cmdlist kunit test ++++++++++++++++++++++++++++++++++!\n");

	RUN_ALL_TESTS(cmdlist_test_suites,"/data/log/cmdlist");
	return 0;
}

static void cmdlist_test_exit(void)
{
	pr_info("--------------------------------------- bye, cmdlist kunit test ----------------------------------!\n");
}

module_init(cmdlist_test_init);
module_exit(cmdlist_test_exit);
