/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: hitrace interface test
 * Author: lc.luchao@huawei.com
 * Create: 2019-06-08
 */
#include "chipset_common/hitrace/tracec.h"
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/scatterlist.h>
#include <linux/swap.h>
#include <linux/list.h>

#define HITRACE_TEST_FAIL 1
#define HITRACE_TEST_OK 0
#define HITRACE_INVALID_INDEX (-1)
#define HITRACE_DECIMAL_BASE 10

struct hitrace_test_case {
	const char *name;
	int (*run)(void);
	int result;
};
static struct dentry *hitrace_root;
static struct dentry *hitrace_file;
static int test_result = HITRACE_TEST_FAIL;
static long test_index = HITRACE_INVALID_INDEX;
static DEFINE_MUTEX(hitrace_test_lock);
static DEFINE_MUTEX(hitrace_result_lock);

static int hitrace_test_intftest_001(void)
{
	const uint64_t chain_id = 0xABCDEF;
	const uint64_t span_id = 0x12345;
	const uint64_t parent_span_id = 0x67890;
	const int flags = HITRACE_FLAG_INCLUDE_ASYNC |
		HITRACE_FLAG_DONOT_CREATE_SPAN;
	struct hitrace_id_struct id = { HITRACE_ID_VALID, HITRACE_VER_1,
		chain_id, flags, span_id, parent_span_id };
	uint8_t bytes[HITRACE_ID_LEN + 1];
	struct hitrace_id_struct bytes_to_id = { 0, 0, 0, 0, 0, 0 };

	if (hitrace_is_valid(&id) == 0)
		return HITRACE_TEST_FAIL;
	/* test return value when set the len to different value
	 * we check the return value when the value of param len
	 * less than,larger than or equal to HITRACE_ID_LEN
	 */
	if (hitrace_id_to_bytes(&id, bytes, HITRACE_ID_LEN - 1) != 0)
		return HITRACE_TEST_FAIL;
	if (hitrace_id_to_bytes(&id, bytes, HITRACE_ID_LEN + 1) !=
		HITRACE_ID_LEN)
		return HITRACE_TEST_FAIL;
	if (hitrace_id_to_bytes(&id, bytes, HITRACE_ID_LEN) != HITRACE_ID_LEN)
		return HITRACE_TEST_FAIL;
	bytes_to_id = hitrace_bytes_to_id(bytes, HITRACE_ID_LEN - 1);
	if (hitrace_is_valid(&bytes_to_id) != 0)
		return HITRACE_TEST_FAIL;
	bytes_to_id = hitrace_bytes_to_id(bytes, HITRACE_ID_LEN + 1);
	if (hitrace_is_valid(&bytes_to_id) != 0)
		return HITRACE_TEST_FAIL;
	bytes_to_id = hitrace_bytes_to_id(bytes, HITRACE_ID_LEN);
	if (hitrace_is_valid(&bytes_to_id) == 0)
		return HITRACE_TEST_FAIL;
	if (hitrace_get_chain_id(&bytes_to_id) != chain_id)
		return HITRACE_TEST_FAIL;
	if (hitrace_get_flags(&bytes_to_id) !=
		(HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_DONOT_CREATE_SPAN))
		return HITRACE_TEST_FAIL;
	if (hitrace_get_span_id(&bytes_to_id) != span_id)
		return HITRACE_TEST_FAIL;
	if (hitrace_get_parent_span_id(&bytes_to_id) != parent_span_id)
		return HITRACE_TEST_FAIL;
	return HITRACE_TEST_OK;
}

static int hitrace_test_intftest_002(void)
{
	struct hitrace_id_struct begin_id = { 0, 0, 0, 0, 0, 0 };
	struct hitrace_id_struct end_id = { 0, 0, 0, 0, 0, 0 };

	hitrace_clear_id();
	begin_id = hitrace_begin("test",
		HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_NO_BE_INFO);
	if (!hitrace_is_valid(&begin_id))
		return HITRACE_TEST_FAIL;
	if (!hitrace_is_flag_enabled(&begin_id, HITRACE_FLAG_INCLUDE_ASYNC))
		return HITRACE_TEST_FAIL;
	if (!hitrace_is_flag_enabled(&begin_id, HITRACE_FLAG_NO_BE_INFO))
		return HITRACE_TEST_FAIL;
	hitrace_end(&begin_id);
	end_id = hitrace_get_id();
	if (hitrace_is_valid(&end_id))
		return HITRACE_TEST_FAIL;
	return HITRACE_TEST_OK;
}

static int hitrace_test_intftest_003(void)
{
	struct hitrace_id_struct begin_id = { 0, 0, 0, 0, 0, 0 };
	struct hitrace_id_struct rebegin_id = { 0, 0, 0, 0, 0, 0 };
	struct hitrace_id_struct end_id = { 0, 0, 0, 0, 0, 0 };
	struct hitrace_id_struct reend_id = { 0, 0, 0, 0, 0, 0 };

	hitrace_clear_id();
	begin_id = hitrace_begin("test", HITRACE_FLAG_INCLUDE_ASYNC);
	rebegin_id = hitrace_begin("test reenter", HITRACE_FLAG_TP_INFO);
	if (hitrace_is_valid(&rebegin_id))
		return HITRACE_TEST_FAIL;
	if (hitrace_get_chain_id(&rebegin_id) ==
		hitrace_get_chain_id(&begin_id))
		return HITRACE_TEST_FAIL;
	if (hitrace_is_flag_enabled(&rebegin_id, HITRACE_FLAG_INCLUDE_ASYNC))
		return HITRACE_TEST_FAIL;
	if (hitrace_is_flag_enabled(&rebegin_id, HITRACE_FLAG_TP_INFO))
		return HITRACE_TEST_FAIL;
	hitrace_end(&rebegin_id);
	end_id = hitrace_get_id();
	if (!hitrace_is_valid(&end_id))
		return HITRACE_TEST_FAIL;
	if (hitrace_get_chain_id(&end_id) != hitrace_get_chain_id(&begin_id))
		return HITRACE_TEST_FAIL;
	if (!hitrace_is_flag_enabled(&end_id, HITRACE_FLAG_INCLUDE_ASYNC))
		return HITRACE_TEST_FAIL;
	if (hitrace_is_flag_enabled(&end_id, HITRACE_FLAG_TP_INFO))
		return HITRACE_TEST_FAIL;
	hitrace_end(&begin_id);
	reend_id = hitrace_get_id();
	if (hitrace_is_valid(&reend_id))
		return HITRACE_TEST_FAIL;
	return HITRACE_TEST_OK;
}

static int hitrace_test_idtest_001(void)
{
	const uint64_t chain_id = 0xABCDEF;
	const uint64_t span_id = 0x12345;
	const uint64_t parent_span_id = 0x67890;
	const int flags = HITRACE_FLAG_INCLUDE_ASYNC |
		HITRACE_FLAG_DONOT_CREATE_SPAN;
	struct hitrace_id_struct set_id = { 0, 0, 0, 0, 0, 0 };
	struct hitrace_id_struct get_id = { 0, 0, 0, 0, 0, 0 };
	struct hitrace_id_struct clear_id = { 0, 0, 0, 0, 0, 0 };
	struct hitrace_id_struct init_id = { 0, 0, 0, 0, 0, 0 };

	hitrace_clear_id();
	init_id = hitrace_get_id();
	if (hitrace_is_valid(&init_id))
		return HITRACE_TEST_FAIL;
	hitrace_set_chain_id(&set_id, chain_id);
	hitrace_set_flags(&set_id, flags);
	hitrace_set_span_id(&set_id, span_id);
	hitrace_set_parent_span_id(&set_id, parent_span_id);
	hitrace_set_id(&set_id);
	get_id = hitrace_get_id();
	if (!hitrace_is_valid(&get_id))
		return HITRACE_TEST_FAIL;
	if (hitrace_get_chain_id(&get_id) != chain_id)
		return HITRACE_TEST_FAIL;
	if (hitrace_get_flags(&get_id) !=
		(HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_DONOT_CREATE_SPAN))
		return HITRACE_TEST_FAIL;
	if (hitrace_get_span_id(&get_id) != span_id)
		return HITRACE_TEST_FAIL;
	if (hitrace_get_parent_span_id(&get_id) != parent_span_id)
		return HITRACE_TEST_FAIL;
	hitrace_clear_id();
	clear_id = hitrace_get_id();
	if (hitrace_is_valid(&clear_id))
		return HITRACE_TEST_FAIL;
	return HITRACE_TEST_OK;
}

static int hitrace_test_spantest_001(void)
{
	struct hitrace_id_struct id = { 0, 0, 0, 0, 0, 0 };
	struct hitrace_id_struct child_id = { 0, 0, 0, 0, 0, 0 };
	struct hitrace_id_struct grand_child_id = { 0, 0, 0, 0, 0, 0 };

	hitrace_clear_id();
	id = hitrace_begin("test", HITRACE_FLAG_DONOT_CREATE_SPAN);
	if (!hitrace_is_flag_enabled(&id, HITRACE_FLAG_DONOT_CREATE_SPAN))
		return HITRACE_TEST_FAIL;
	if (hitrace_get_span_id(&id))
		return HITRACE_TEST_FAIL;
	if (hitrace_get_parent_span_id(&id))
		return HITRACE_TEST_FAIL;
	child_id = hitrace_create_span();
	if (!hitrace_is_valid(&child_id))
		return HITRACE_TEST_FAIL;
	if (hitrace_get_flags(&child_id) != hitrace_get_flags(&id))
		return HITRACE_TEST_FAIL;
	if (hitrace_get_chain_id(&child_id) != hitrace_get_chain_id(&id))
		return HITRACE_TEST_FAIL;
	if (hitrace_get_parent_span_id(&child_id) != hitrace_get_span_id(&id))
		return HITRACE_TEST_FAIL;
	hitrace_set_id(&child_id);
	grand_child_id = hitrace_create_span();
	if (!hitrace_is_valid(&grand_child_id))
		return HITRACE_TEST_FAIL;
	if (hitrace_get_flags(&grand_child_id) != hitrace_get_flags(&id))
		return HITRACE_TEST_FAIL;
	if (hitrace_get_chain_id(&grand_child_id) != hitrace_get_chain_id(&id))
		return HITRACE_TEST_FAIL;
	if (hitrace_get_parent_span_id(&grand_child_id) !=
		hitrace_get_span_id(&child_id))
		return HITRACE_TEST_FAIL;
	hitrace_end(&id);
	return HITRACE_TEST_OK;
}

static int hitrace_test_spantest_002(void)
{
	struct hitrace_id_struct child_id = { 0, 0, 0, 0, 0, 0 };
	struct hitrace_id_struct id = hitrace_begin("test",
		HITRACE_FLAG_DONOT_CREATE_SPAN);

	if (!hitrace_is_flag_enabled(&id, HITRACE_FLAG_DONOT_CREATE_SPAN))
		return HITRACE_TEST_FAIL;

	child_id = hitrace_create_span();
	if (!hitrace_is_valid(&child_id))
		return HITRACE_TEST_FAIL;
	if (hitrace_get_flags(&child_id) != hitrace_get_flags(&id))
		return HITRACE_TEST_FAIL;
	if (hitrace_get_chain_id(&child_id) != hitrace_get_chain_id(&id))
		return HITRACE_TEST_FAIL;
	if (hitrace_get_parent_span_id(&child_id) !=
		hitrace_get_parent_span_id(&id))
		return HITRACE_TEST_FAIL;

	hitrace_end(&id);
	return HITRACE_TEST_OK;
}

static int hitrace_test_tracepointtest_001(void)
{
	struct hitrace_id_struct id = { 0, 0, 0, 0, 0, 0 };
	int msg_conent = 12;

	hitrace_clear_id();
	id = hitrace_begin("test tp flag", HITRACE_FLAG_TP_INFO);
	if (!hitrace_is_flag_enabled(&id, HITRACE_FLAG_TP_INFO))
		return HITRACE_TEST_FAIL;
	hitrace_trace_point(HITRACE_TP_CS,
		&id, "client send msg content %d", msg_conent);
	hitrace_trace_point(HITRACE_TP_CS,
		NULL, "client send msg content %d", msg_conent);
	hitrace_end(&id);
	return HITRACE_TEST_OK;
}

static int hitrace_test_tracepointtest_002(void)
{
	struct hitrace_id_struct id = { 0, 0, 0, 0, 0, 0 };

	hitrace_clear_id();
	id = hitrace_begin("test no tp flag",
		HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_NO_BE_INFO);
	if (hitrace_is_flag_enabled(&id, HITRACE_FLAG_TP_INFO))
		return HITRACE_TEST_FAIL;
	hitrace_begin("test no tp flag",
		HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_NO_BE_INFO);
	hitrace_end(&id);
	return HITRACE_TEST_OK;
}

static int hitrace_test_syncasynctest_001(void)
{
	struct hitrace_id_struct sync_id = { 0, 0, 0, 0, 0, 0 };
	struct hitrace_id_struct async_id = { 0, 0, 0, 0, 0, 0 };

	hitrace_clear_id();
	sync_id = hitrace_begin("test sync only",
		HITRACE_FLAG_TP_INFO | HITRACE_FLAG_NO_BE_INFO);
	if (hitrace_is_flag_enabled(&sync_id, HITRACE_FLAG_INCLUDE_ASYNC))
		return HITRACE_TEST_FAIL;
	hitrace_trace_point(HITRACE_TP_CS,
		&sync_id, "client send msg: %s", "sync");
	hitrace_end(&sync_id);
	async_id = hitrace_begin("test sync+async",
		HITRACE_FLAG_INCLUDE_ASYNC |
		HITRACE_FLAG_TP_INFO | HITRACE_FLAG_NO_BE_INFO);
	if (!hitrace_is_flag_enabled(&async_id, HITRACE_FLAG_INCLUDE_ASYNC))
		return HITRACE_TEST_FAIL;
	hitrace_trace_point(HITRACE_TP_CS,
		&async_id, "client send msg: %s", "async");
	hitrace_end(&async_id);
	return HITRACE_TEST_OK;
}

static int hitrace_test_invalid_param_test_001(void)
{
	struct hitrace_id_struct invalid_flag_id = { 0, 0, 0, 0, 0, 0 };
	struct hitrace_id_struct invalid_name_id = { 0, 0, 0, 0, 0, 0 };
	const int invalid_large_flag = HITRACE_FLAG_MAX;
	const int invalid_little_flag = -1;

	hitrace_clear_id();
	/* begin with invalid flag */
	invalid_flag_id = hitrace_begin("test invalid flag",
		invalid_large_flag);
	if (hitrace_is_valid(&invalid_flag_id))
		return HITRACE_TEST_FAIL;
	invalid_flag_id = hitrace_begin("test invalid flag",
		invalid_little_flag);
	if (hitrace_is_valid(&invalid_flag_id))
		return HITRACE_TEST_FAIL;
	hitrace_end(&invalid_flag_id);
	/* begin with invalid name */
	invalid_name_id = hitrace_begin(NULL, HITRACE_FLAG_TP_INFO);
	if (!hitrace_is_valid(&invalid_name_id))
		return HITRACE_TEST_FAIL;
	hitrace_end(&invalid_name_id);
	return HITRACE_TEST_OK;
}

static const struct hitrace_test_case hitrace_test_cases[] = {
	{
		.name = "HitraceKernelTest_IntfTest_001",
		.run = hitrace_test_intftest_001,
	},
	{
		.name = "HitraceKernelTest_IntfTest_002",
		.run = hitrace_test_intftest_002,
	},
	{
		.name = "HitraceKernelTest_IntfTest_003",
		.run = hitrace_test_intftest_003,
	},
	{
		.name = "HitraceKernelTest_IdTest_001",
		.run = hitrace_test_idtest_001,
	},
	{
		.name = "HitraceKernelTest_SpanTest_001",
		.run = hitrace_test_spantest_001,
	},
	{
		.name = "HitraceKernelTest_SpanTest_002",
		.run = hitrace_test_spantest_002,
	},
	{
		.name = "HitraceKernelTest_TracepointTest_001",
		.run = hitrace_test_tracepointtest_001,
	},
	{
		.name = "HitraceKernelTest_TracepointTest_002",
		.run = hitrace_test_tracepointtest_002,
	},
	{
		.name = "HitraceKernelTest_SyncAsyncTest_001",
		.run = hitrace_test_syncasynctest_001,
	},
	{
		.name = "HitraceKernelTest_InvalidParamTest_001",
		.run = hitrace_test_invalid_param_test_001,
	},
};

static void hitrace_test_run(long testcase)
{
	long i;

	for (i = 0; i < ARRAY_SIZE(hitrace_test_cases); i++) {
		/* testcase:1-9, i:0-8, so we use i+1 when we find testcase */
		if (testcase && ((i + 1) != testcase))
			continue;
		test_result = hitrace_test_cases[i].run();
		test_index = i;
	}
}


static int hitrace_test_show(struct seq_file *sf, void *data)
{
	mutex_lock(&hitrace_result_lock);
	if (test_result == HITRACE_TEST_OK) {
		seq_printf(sf, "Test case:%s\nTest result: %s\n",
			hitrace_test_cases[test_index].name, "OK");
	} else {
		seq_printf(sf, "Test case:%s\nTest result: %s\n",
			hitrace_test_cases[test_index].name, "FAIL");
	}
	test_result = HITRACE_TEST_FAIL;
	test_index = HITRACE_INVALID_INDEX;
	mutex_unlock(&hitrace_result_lock);
	return 0;
}

static int hitrace_test_open(struct inode *inode, struct file *file)
{
	return single_open(file, hitrace_test_show, inode->i_private);
}

static ssize_t hitrace_test_write(struct file *file, const char __user *buf,
	size_t count, loff_t *pos)
{
	long testcase;
	int ret;

	ret = kstrtol_from_user(buf, count, HITRACE_DECIMAL_BASE, &testcase);
	if (ret)
		return ret;
	mutex_lock(&hitrace_test_lock);
	hitrace_test_run(testcase);
	mutex_unlock(&hitrace_test_lock);
	return count;
}

static const struct file_operations hitrace_test_fops = {
	.open		= hitrace_test_open,
	.read		= seq_read,
	.write		= hitrace_test_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init hitrace_debug_init(void)
{
	pr_info("hitrace debug init\n");
	hitrace_root = debugfs_create_dir("hitrace", NULL);
	hitrace_file = debugfs_create_file("test", 0644,
		hitrace_root, NULL, &hitrace_test_fops);
	return 0;
}

static void __exit hitrace_debug_exit(void)
{
	pr_info("hitrace debug exit\n");
	if (hitrace_file != NULL) {
		debugfs_remove(hitrace_file);
		hitrace_file = NULL;
	}
	if (hitrace_root != NULL) {
		debugfs_remove_recursive(hitrace_root);
		hitrace_root = NULL;
	}
}

module_init(hitrace_debug_init);
module_exit(hitrace_debug_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("debug for hitrace");
