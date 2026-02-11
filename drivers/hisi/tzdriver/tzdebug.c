
#include "tzdebug.h"
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <stdarg.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/mm.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <securec.h>
#include "tc_ns_log.h"
#include "tc_client_sub_driver.h"
#include "teek_ns_client.h"
#include "smc.h"
#include "teek_client_constants.h"
#include "mailbox_mempool.h"
#include "dynamic_mem.h"
#include "tlogger.h"
#include "cmdmonitor.h"
#include "teek_client_api.h"

static struct dentry *tz_dbg_dentry = NULL;
typedef void (*tzdebug_opt_func)(const char *param);

struct opt_ops {
	char *name;
	tzdebug_opt_func func;
};

static DEFINE_MUTEX(g_meminfo_lock);
static struct tee_mem g_tee_meminfo = {0};
static int send_dump_mem(int flag, struct tee_mem *statmem);
static void tzmemdump(const char *param);

void tee_dump_mem(void)
{
	tzmemdump(NULL);
	if (tlogger_store_lastmsg() < 0)
		tloge("[cmd_monitor_tick]tlogger_store_lastmsg failed\n");
}

/* get meminfo (tee_mem + N * ta_mem < 4Kbyte) from tee */
static int get_tee_meminfo_cmd(void)
{
	int ret;
	int sret;
	struct tee_mem *mem = (struct tee_mem *)mailbox_alloc(sizeof(*mem),
	                      MB_FLAG_ZERO);

	if (mem == NULL)
		return -1;
	ret = send_dump_mem(0, mem);
	mutex_lock(&g_meminfo_lock);
	sret = memcpy_s((void *)&g_tee_meminfo, sizeof(g_tee_meminfo),
		mem, sizeof(*mem));
	if (sret != 0)
		tloge("sret=%d\n", sret);
	mutex_unlock(&g_meminfo_lock);
	mailbox_free(mem);
	return ret;
}

static atomic_t g_cmd_send = ATOMIC_INIT(1);

void set_cmd_send_state(void)
{
	atomic_set(&g_cmd_send, 1);
}

int get_tee_meminfo(struct tee_mem *meminfo)
{
	errno_t s_ret;

	if (meminfo == NULL)
		return -1;
	if (atomic_read(&g_cmd_send)) {
		if (get_tee_meminfo_cmd() != 0)
			return -1;
	} else {
		atomic_set(&g_cmd_send, 0);
	}
	mutex_lock(&g_meminfo_lock);
	s_ret = memcpy_s((void *)meminfo, sizeof(*meminfo),
	                 (void *)&g_tee_meminfo,
	                 sizeof(g_tee_meminfo));
	mutex_unlock(&g_meminfo_lock);
	if (s_ret != EOK)
		return -1;

	return 0;
}
EXPORT_SYMBOL(get_tee_meminfo);

static int send_dump_mem(int flag, struct tee_mem *statmem)
{
	tc_ns_smc_cmd smc_cmd = { {0}, 0 };
	struct mb_cmd_pack *mb_pack = NULL;
	int ret = 0;

	if (statmem == NULL) {
		tloge("statmem is NULL\n");
		return -1;
	}
	mb_pack = mailbox_alloc_cmd_pack();
	if (mb_pack == NULL)
		return -ENOMEM;
	smc_cmd.cmd_id = GLOBAL_CMD_ID_DUMP_MEMINFO;
	smc_cmd.global_cmd = true;
	mb_pack->operation.paramtypes = TEEC_PARAM_TYPES(
	                                        TEE_PARAM_TYPE_MEMREF_INOUT,
	                                        TEE_PARAM_TYPE_VALUE_INPUT,
	                                        TEE_PARAM_TYPE_NONE,
	                                        TEE_PARAM_TYPE_NONE);
	mb_pack->operation.params[TEE_PARAM_ONE].memref.buffer = virt_to_phys(statmem);
	mb_pack->operation.params[TEE_PARAM_ONE].memref.size = sizeof(*statmem);
	mb_pack->operation.buffer_h_addr[TEE_PARAM_ONE] =
		(virt_to_phys(statmem) >> ADDR_TRANS_NUM);
	mb_pack->operation.params[TEE_PARAM_TWO].value.a = flag;
	smc_cmd.operation_phys =
	        (unsigned int)virt_to_phys(&mb_pack->operation);
	smc_cmd.operation_h_phys = virt_to_phys(&mb_pack->operation) >> ADDR_TRANS_NUM;
	ret = tc_ns_smc(&smc_cmd);
	if (ret)
		tloge("send_dump_mem failed.\n");
	tz_log_write();
	mailbox_free(mb_pack);
	return ret;
}

static void archivelog(const char *param)
{
	(void)param;
	tzdebug_archivelog();
}

static void tzdump(const char *param)
{
	(void)param;
	show_cmd_bitmap();
	wakeup_tc_siq();
}

static void tzmemdump(const char *param)
{
	struct tee_mem *mem = NULL;

	(void)param;
	mem = (struct tee_mem *)mailbox_alloc(sizeof(*mem),
	                      MB_FLAG_ZERO);
	if (mem == NULL) {
		tloge("mailbox alloc failed\n");
		return;
	}
	(void)send_dump_mem(1, mem);
	mailbox_free(mem);
}

static void tzmemstat(const char *param)
{

	struct tee_mem *meminfo = NULL;
	uint32_t i;
	int ret;

	(void)param;
	meminfo = (struct tee_mem *)mailbox_alloc(sizeof(*meminfo),
	                      MB_FLAG_ZERO);
	if (meminfo == NULL) {
		tloge("mailbox alloc failed\n");
		return;
	}
	ret = get_tee_meminfo(meminfo);
	if (ret == 0) {
		tloge("total=%d,pmem=%d,free=%d,lowest=%d\n", meminfo->total_mem, meminfo->pmem,
			meminfo->free_mem, meminfo->free_mem_min);
		for (i = 0; i < meminfo->ta_num; i++) {
			tloge("i=%d,name=%s,mem=%d,memmax=%d,memlimit=%d\n", i,
				meminfo->ta_mem_info[i].ta_name, meminfo->ta_mem_info[i].pmem,
				meminfo->ta_mem_info[i].pmem_max, meminfo->ta_mem_info[i].pmem_limit);
		}
		report_imonitor(meminfo);
	}
	mailbox_free(meminfo);
}

static void tzlogwrite(const char *param)
{
	(void)param;
	(void)tz_log_write();
}

static void tzhelp(const char *param);

static struct opt_ops optArr[] = {
	{"help", tzhelp},
	{"archivelog", archivelog},
	{"dump", tzdump},
	{"memdump", tzmemdump},
	{"logwrite", tzlogwrite},
	{"dump_service", dump_services_status},
	{"memstat", tzmemstat},
};

static void tzhelp(const char *param)
{
	uint32_t i = 0;
	(void)param;

	for (i = 0; i < sizeof(optArr) / sizeof(struct opt_ops); i++) {
		tloge("cmd:%s\n", optArr[i].name);
	}
}

static ssize_t tz_dbg_opt_write(struct file *filp,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	char buf[128] = {0};
	char *value = NULL;
	char *p = NULL;
	uint32_t i = 0;

	if ((ubuf == NULL) || (filp == NULL) || (ppos == NULL))
		return -EINVAL;

	if (cnt >= sizeof(buf))
		return -EINVAL;

	if (cnt == 0)
		return -EINVAL;

	if (copy_from_user(buf, ubuf, cnt))
		return -EFAULT;
	buf[cnt] = 0;
	if (cnt > 0 && buf[cnt - 1] == '\n')
		buf[cnt - 1] = 0;
	value = buf;
	p = strsep(&value, ":");
	if (p == NULL)
		return -EINVAL;
	for (i = 0; i < sizeof(optArr) / sizeof(struct opt_ops); i++) {
		if (!strncmp(p, optArr[i].name, strlen(optArr[i].name)) &&
		    strlen(p) == strlen(optArr[i].name)) {
			optArr[i].func(value);
			return cnt;
		}
	}
	return -EFAULT;
}

static const struct file_operations tz_dbg_opt_fops = {
	.owner = THIS_MODULE,
	.write = tz_dbg_opt_write,
};

static int __init tzdebug_init(void)
{
	tz_dbg_dentry = debugfs_create_dir("tzdebug", NULL);
	if (tz_dbg_dentry == NULL)
		return 0;
	debugfs_create_file("opt", 0220, tz_dbg_dentry, NULL, &tz_dbg_opt_fops);
	return 0;
}

static void __exit tzdebug_exit(void)
{
	if (tz_dbg_dentry == NULL)
		return;
	debugfs_remove_recursive(tz_dbg_dentry);
	tz_dbg_dentry = NULL;
}

module_init(tzdebug_init);
module_exit(tzdebug_exit);
