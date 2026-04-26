/**
 * @file testcase.c
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>

#include "Kunit.h"
#include "Stub.h"

#include "mipi_dsi_drv.h"
#include "dkmd_connector.h"

extern struct composer_dev_private_data composer_dev_data;

void test_dsi_init(void)
{
	struct composer_dev_private_data *pdata = &composer_dev_data;

	pr_info("test_dsi_init, pdata->pdev = %p!\n", pdata->pdev);

	CHECK_NOT_EQUAL(pdata->pdev, NULL);

	CHECK_EQUAL(pdata->base.on_func(pdata->pdev), 0);

	msleep(1000);

	CHECK_EQUAL(pdata->base.off_func(pdata->pdev), 0);

	msleep(1000);

	CHECK_EQUAL(pdata->base.on_func(pdata->pdev), 0);
}

KU_TestInfo testcase[] = {
	{ "test_dsi_init", test_dsi_init },
	KU_TEST_INFO_NULL
};

int testcase_suite_init(void)
{
	pr_info("testcase device init!\n");
	return 0;
}

int testcase_suite_clean(void)
{
	pr_info("testcase device deinit!\n");
	return 0;
}

KU_SuiteInfo testcase_suites[]=
{
	{"testcase_init", testcase_suite_init, testcase_suite_clean, testcase, KU_TRUE},
	KU_SUITE_INFO_NULL
};

static int testcase_init(void)
{
	pr_info("+++++++++++++++++++++++++++++++++++++++ hello, testcase kunit ++++++++++++++++++++++++++++++++++!\n");

	RUN_ALL_TESTS(testcase_suites,"/data/log/testcase");
	return 0;
}

static void testcase_exit(void)
{
	pr_info("--------------------------------------- bye, testcase kunit ----------------------------------!\n");
}

module_init(testcase_init);
module_exit(testcase_exit);
