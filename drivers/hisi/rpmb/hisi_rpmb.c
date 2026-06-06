

#include <linux/version.h>
#include <linux/cpumask.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <asm-generic/fcntl.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/slab.h>
#include <linux/smp.h>
#include <linux/types.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/types.h>
#endif
#include <linux/bootmem.h>
#include <linux/mm.h>
#include <linux/printk.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/wakelock.h>
#include <linux/compiler.h>
#include <global_ddr_map.h>
#include <linux/time.h>

#include <asm/byteorder.h>
#include <asm/compiler.h>
#include <asm/hardirq.h>

#include <linux/hisi/rpmb.h>
#include "hisi_rpmb.h"
#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <linux/bootdevice.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/hisi/hisi_tele_mntn.h>

static u64 g_hisee_start_time = 0;
static u64 g_hisee_end_time = 0;
static u64 g_work_queue_start = 0;
u64 g_rpmb_ufs_start_time = 0;

struct hisi_rpmb {
	struct rpmb_request *rpmb_request;
	struct device *dev;
	struct block_device *blkdev;
	struct task_struct *rpmb_task;
	int wake_up_condition;
	wait_queue_head_t wait;
	struct wake_lock wake_lock;
	enum rpmb_version dev_ver;
	struct rpmb_operation *ops;
};

struct {
	struct rpmb_operation *ops;
	enum bootdevice_type type;
} rpmb_device[BOOT_DEVICE_MAX];

static struct hisi_rpmb hisi_rpmb;
static enum bootdevice_type rpmb_support_device = BOOT_DEVICE_EMMC;
static int rpmb_drivers_init_status = RPMB_DRIVER_IS_NOT_READY;
static int rpmb_device_init_status = RPMB_DEVICE_IS_NOT_READY;
static int rpmb_key_status = KEY_NOT_READY;

DEFINE_MUTEX(rpmb_counter_lock);

void rpmb_print_frame_buf(char *name, const void *buf, int len, int format)
{
	pr_err("%s \n", name);
	print_hex_dump(KERN_ERR, "", DUMP_PREFIX_OFFSET, format, 1, buf, len,
		       false);
}

noinline int atfd_hisi_rpmb_smc(u64 _function_id, u64 _arg0, u64 _arg1,
				u64 _arg2)
{
	register u64 function_id asm("x0") = _function_id;
	register u64 arg0 asm("x1") = _arg0;
	register u64 arg1 asm("x2") = _arg1;
	register u64 arg2 asm("x3") = _arg2;

	asm volatile(__asmeq("%0", "x0")
		     __asmeq("%1", "x1")
		     __asmeq("%2", "x2")
		     __asmeq("%3", "x3")

		     "smc    #0\n"
		     : "+r"(function_id)
		     : "r"(arg0), "r"(arg1), "r"(arg2));

	return (int)function_id;
}

/*
 * Return the member of hisi_rpmb, rpmb_request. This function only is used in
 * rpmb folder, and not for other module.
 */
struct rpmb_request *rpmb_get_request(void)
{
	if (hisi_rpmb.rpmb_request)
		return hisi_rpmb.rpmb_request;
	else
		return NULL;
}

int rpmb_get_dev_ver(enum rpmb_version *ver)
{
	enum rpmb_version version = hisi_rpmb.dev_ver;

	if (!ver)
		return RPMB_ERR_DEV_VER;

	if (version <= RPMB_VER_INVALID || version >= RPMB_VER_MAX) {
		pr_err("Error: invalid rpmb dev ver: 0x%x\n", version);
		return RPMB_ERR_DEV_VER;
	}

	*ver = version;

	return RPMB_OK;
}

/*
 * @ops: driver's rpmb operation pointer.
 * @device_type: enum value of bootdevice_type.
 * This function is used to register ufs/mmc rpmb operation.
 */
int rpmb_operation_register(struct rpmb_operation *ops,
			    enum bootdevice_type device_type)
{
	if (!ops) {
		pr_err("%s: ops is null\n", __func__);
		return -EINVAL;
	}

	if (BOOT_DEVICE_MAX <= device_type) {
		pr_err("%s: device_type(%d) exceed max type\n", __func__,
		       device_type);
		return -EDOM;
	}

	if (rpmb_device[device_type].ops) {
		pr_err("%s: ops index(%d) has already been register\n",
		       __func__, device_type);
		return -EINVAL;
	}

	rpmb_device[device_type].type = device_type;
	rpmb_device[device_type].ops = ops;

	return 0;
}

static struct rpmb_operation *
rpmb_operation_get(enum bootdevice_type device_type)
{
	if (BOOT_DEVICE_MAX <= device_type) {
		pr_err("%s: device_type(%d) exceed max type\n", __func__,
		       device_type);
		return NULL;
	}

	if (!rpmb_device[device_type].ops) {
		pr_err("%s: ops index(%d) is not exist\n", __func__,
		       device_type);
		return NULL;
	}

	return rpmb_device[device_type].ops;
}

void hisi_hisee_time_stamp(enum rpmb_state state, unsigned int blks)
{
	u64 temp_cost_time;
	uint16_t debug_enable = 0;


	g_hisee_end_time = hisi_getcurtime();
	temp_cost_time = g_hisee_end_time - g_hisee_start_time;
	if (temp_cost_time > RPMB_TIMEOUT_TIME_IN_KERNEL || debug_enable) {
		pr_err("[%s]rpmb access cost time is more than 800ms, "
		       "start[%llu], workqueue start time[%llu], "
		       "ufs start time[%llu],end[%llu], state[%d]!\n",
		       __func__, g_hisee_start_time, g_work_queue_start,
		       g_rpmb_ufs_start_time, g_hisee_end_time, (int32_t)state);
	}

	return;
}

int wait_hisee_rpmb_request_is_finished(void)
{
	int i;
	struct rpmb_request *request = hisi_rpmb.rpmb_request;

	/* wait 20s timeout (2000 * 10ms) */
	for (i = 0; i < 2000; i++) {
		if (request->rpmb_request_status == 0) {
			break;
		}
		msleep(10);
	}

	if (i == 2000) {
		pr_err("rpmb request get result from device timeout\n");
		return -1;
	}

	return 0;
}

int check_hisee_rpmb_request_is_cleaned(void)
{
	int i;
	struct rpmb_request *request = hisi_rpmb.rpmb_request;

	/* wait 10-15ms timeout (1000 * 10us) */
	for (i = 0; i < 1000; i++) {
		if (0 != request->rpmb_request_status) {
			pr_err("rpmb request is not cleaned in 100-150ms\n");
			return -1;
		}
		usleep_range((unsigned long)10, (unsigned long)15);
	}

	return 0;
}

int32_t hisee_exception_to_reset_rpmb(void)
{
	int i;
	struct rpmb_request *request = hisi_rpmb.rpmb_request;

	request->rpmb_exception_status = RPMB_MARK_EXIST_STATUS;
	for (i = 0; i < 3; i++) {
		if (wait_hisee_rpmb_request_is_finished())
			return -1;
		/* if request finished status is not cleaned in 100-150ms,
		 * we will retry to wait for hisee request finished
		 */
		if (check_hisee_rpmb_request_is_cleaned())
			pr_err("Front rpmb request is done, but request is "
			       "not cleaned\n");
		else
			break;
	}
	request->rpmb_exception_status = 0;

	return 0;
}

static void rpmb_work_routine(void)
{
	int result = RPMB_ERR_DEV_VER;
	int ret;
	struct rpmb_request *request = hisi_rpmb.rpmb_request;
	struct rpmb_operation *rpmb_ops = hisi_rpmb.ops;

	/* No key, we do not send read and write request
	 * (hynix device not support no key to read more than 4K)
	 */
	if (rpmb_key_status == KEY_NOT_READY) {
		result = RPMB_ERR_KEY;
		pr_err("[%s]:rpmb key is not ready, do not transfer read and"
		       "write request to device, result = %d\n",
		       __func__, result);
		goto out;
	}

	g_work_queue_start = hisi_getcurtime();
	wake_lock_timeout(&hisi_rpmb.wake_lock, (long)2 * HZ);
	if (rpmb_ops && rpmb_ops->issue_work)
		result = rpmb_ops->issue_work(request);
	if (result)
		pr_err("[%s]:rpmb request done failed,result = %d\n", __func__,
		       result);

out:
	/* mark hisee rpmb request end time */
	hisi_hisee_time_stamp(request->info.state,
			      request->info.current_rqst.blks);
	hisi_rpmb.wake_up_condition = 0;

	ret = atfd_hisi_rpmb_smc((u64)RPMB_SVC_REQUEST_DONE, (u64)(long)result,
				 (u64)0, (u64)0);
	if (ret) {
		pr_err("[%s]:state %d, region %d, trans blks %d, "
		       "rpmb request done from bl31 failed, ret 0x%x\n",
		       __func__, request->info.state,
		       request->info.rpmb_region_num,
		       request->info.current_rqst.blks, ret);
		rpmb_print_frame_buf("frame failed",
				     (void *)&request->error_frame, 512, 16);
	}
}

/*
 * hisi_rpmb_active - handle rpmb request from ATF
 */
void hisi_rpmb_active(void)
{
	/*mark hisee rpmb request start time*/
	g_hisee_start_time = hisi_getcurtime();
	hisi_rpmb.wake_up_condition = 1;
	wake_up(&hisi_rpmb.wait);
}
EXPORT_SYMBOL(hisi_rpmb_active);


static void rpmb_key_status_check(void)
{
	int ret = RPMB_ERR_DEV_VER;
	struct rpmb_operation *rpmb_ops = hisi_rpmb.ops;

	/* This is just for region 0 */
	hisi_rpmb.rpmb_request->info.rpmb_region_num = 0;

	if (rpmb_ops && rpmb_ops->key_status)
		ret = rpmb_ops->key_status();

	if (!ret)
		rpmb_key_status = KEY_READY;
	else
		rpmb_key_status = KEY_NOT_READY;
}

u32 get_rpmb_support_key_num(void)
{
	int i;
	u32 key_num = 0;
	u8 rpmb_region_enable;

	rpmb_region_enable = get_rpmb_region_enable();

	for (i = 0; i < MAX_RPMB_REGION_NUM; i++) {
		if (((rpmb_region_enable >> i) & 0x1))
			key_num++;
	}

	if (key_num == 0) {
		pr_err("failed: get_rpmb_support_key_num is zero, rpmb is not "
		       "support\n");
	}

	return key_num;
}

int get_rpmb_key_status(void)
{
	int result;

	if (get_rpmb_support_key_num() == 1) {
		return rpmb_key_status;
	} else {
		result = atfd_hisi_rpmb_smc((u64)RPMB_SVC_MULTI_KEY_STATUS,
					    (u64)0x0, (u64)0x0, (u64)0x0);
		if (result == KEY_NOT_READY || result == KEY_READY) {
			return result;
		} else {
			pr_err("get_rpmb_key_status failed, result = %d\n",
			       result);
			return KEY_REQ_FAILED;
		}
	}
}

int get_rpmb_init_status_quick(void)
{
	return rpmb_drivers_init_status;
}

static ssize_t hisi_rpmb_key_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	ssize_t ret = RPMB_ERR_DEV_VER;
	struct rpmb_operation *rpmb_ops = hisi_rpmb.ops;
	struct rpmb_request *request = hisi_rpmb.rpmb_request;

	ret = strncmp(buf, "set_key", count);
	if (ret) {
		pr_err("invalid key set command\n");
		return ret;
	}

	if (rpmb_ops && rpmb_ops->key_store)
		ret = rpmb_ops->key_store(dev, request);

	if (!ret)
		rpmb_key_status = KEY_READY;

	return ret;
}

/* according to rpmb_key_status to check the key is ready */
static ssize_t hisi_rpmb_key_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	rpmb_key_status_check();
	if (rpmb_key_status == KEY_READY)
		strncpy(buf, "true", sizeof("true")); /* unsafe_function_ignore: strncpy */
	else
		strncpy(buf, "false", sizeof("false")); /* unsafe_function_ignore: strncpy */
	return (ssize_t)strlen(buf);
}

#define WAIT_INIT_TIMES 3000
int get_rpmb_init_status(void)
{
	int i;
	int32_t ret = 0;
	/*lint -e501*/
	mm_segment_t oldfs = get_fs();
	set_fs(get_ds());
	/*lint +e501*/
	for (i = 0; i < WAIT_INIT_TIMES; i++) {
		if (rpmb_support_device == BOOT_DEVICE_EMMC)
			ret = (int32_t)sys_access(EMMC_RPMB_BLOCK_DEVICE_NAME,
						  0);
		else
			ret = (int32_t)sys_access(UFS_RPMB_BLOCK_DEVICE_NAME,
						  0);
		if (!ret && rpmb_device_init_status)
			break;

		usleep_range((unsigned long)3000, (unsigned long)5000);
	}
	if (i == WAIT_INIT_TIMES) {
		pr_err("wait for device init timeout, ret = %d\n", ret);
		rpmb_drivers_init_status = RPMB_DRIVER_IS_NOT_READY;
	} else {
		rpmb_drivers_init_status = RPMB_DRIVER_IS_READY;
	}

	set_fs(oldfs);
	/* when device is OK and rpmb key is not ready, we will check
	 * the key status
	 */
	if (rpmb_drivers_init_status == RPMB_DRIVER_IS_READY &&
	    rpmb_key_status == KEY_NOT_READY) {
		rpmb_key_status_check();
		pr_err("[%s]:rpmb key status{%d}\n", __func__, rpmb_key_status);
	}

	return rpmb_drivers_init_status;
}

static DEVICE_ATTR(rpmb_key, (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP),
		   hisi_rpmb_key_show, hisi_rpmb_key_store);


int hisi_rpmb_ioctl_cmd(enum func_id id, enum rpmb_op_type operation,
			struct storage_blk_ioc_rpmb_data *storage_data)
{
	int ret = RPMB_ERR_DEV_VER;
	struct rpmb_operation *rpmb_ops = hisi_rpmb.ops;

	if (!storage_data)
		return RPMB_INVALID_PARA;

	if (get_rpmb_init_status() == RPMB_DRIVER_IS_NOT_READY) {
		pr_err("[%s]:rpmb init is not ready\n", __func__);
		ret = RPMB_ERR_INIT;
		goto out;
	}
	/* No key, we do not send read and write request(hynix device not
	 * support no key to read more than 4K)
	 */
	if ((rpmb_key_status == KEY_NOT_READY) &&
	    (operation != RPMB_OP_WR_CNT)) {
		ret = RPMB_ERR_KEY;
		pr_err("[%s]:rpmb key is not ready, do not transfer read and "
		       "write request to device, result = %d\n",
		       __func__, ret);
		goto out;
	}

	if (rpmb_ops && rpmb_ops->ioctl)
		ret = rpmb_ops->ioctl(id, operation, storage_data);
	if (ret)
		pr_err("%s: ret %d\n", __func__, ret);

out:
	return ret;
}

/* create a virtual device for dma_alloc */
#define SECURE_STORAGE_NAME "secure_storage"
#define RPMB_DEVICE_NAME "hisi_rpmb"
static int hisi_rpmb_device_init(void)
{
	struct device *pdevice;
	struct class *rpmb_class;
	struct device_node *np = NULL;
	enum rpmb_version version;
	dma_addr_t rpmb_request_phy = 0;
	unsigned long mem_size = 0;
	phys_addr_t bl31_smem_base =
		HISI_SUB_RESERVED_BL31_SHARE_MEM_PHYMEM_BASE;
	u32 data[2] = { 0 };

	rpmb_class = class_create(THIS_MODULE, SECURE_STORAGE_NAME);
	if (IS_ERR(rpmb_class))
		return (int)PTR_ERR(rpmb_class);

	pdevice = device_create(rpmb_class, NULL, MKDEV(0, 0), NULL,
				RPMB_DEVICE_NAME);
	if (IS_ERR(pdevice))
		goto err_class_destroy;

	hisi_rpmb.dev = pdevice;

	if (device_create_file(pdevice, &dev_attr_rpmb_key)) {
		pr_err("rpmb error: unable to create sysfs attributes\n");
		goto err_device_destroy;
	}

	np = of_find_compatible_node(NULL, NULL, "hisilicon,hisi-rpmb");
	if (!np) {
		pr_err("rpmb err of_find_compatible_node");
		goto err_device_remove_file;
	}

	if (of_property_read_u32_array(np, "hisi,bl31-share-mem", &data[0],
				       (unsigned long)2)) {
		pr_err("rpmb get share_mem_address failed\n");
		goto err_node;
	}

	rpmb_request_phy = bl31_smem_base + data[0];
	mem_size = data[1];

	hisi_rpmb.rpmb_request = ioremap_wc(rpmb_request_phy, mem_size);
	if (!hisi_rpmb.rpmb_request)
		goto err_node;
	if (atfd_hisi_rpmb_smc((u64)RPMB_SVC_REQUEST_ADDR, rpmb_request_phy,
			       (u64)rpmb_support_device, (u64)0x0)) {
		pr_err("rpmb set shared memory address failed\n");
		goto err_ioremap;
	}

	version = (enum rpmb_version)atfd_hisi_rpmb_smc(
		(u64)RPMB_SVC_GET_DEV_VER, (u64)0, (u64)0, (u64)0);
	if (version <= RPMB_VER_INVALID || version >= RPMB_VER_MAX) {
		pr_err("Error: invalid rpmb dev ver: 0x%x\n", version);
		goto err_ioremap;
	}

	hisi_rpmb.dev_ver = version;

	return 0;

err_ioremap:
	iounmap(hisi_rpmb.rpmb_request);
err_node:
	of_node_put(np);
err_device_remove_file:
	device_remove_file(pdevice, &dev_attr_rpmb_key);
err_device_destroy:
	device_destroy(rpmb_class, pdevice->devt);
err_class_destroy:
	class_destroy(rpmb_class);
	return -1;
}

static int hisi_rpmb_work_thread(void *arg)
{
	set_freezable();
	while (!kthread_should_stop()) {
		wait_event_freezable(hisi_rpmb.wait,
				     hisi_rpmb.wake_up_condition);
		rpmb_work_routine();
	}
	return 0;
}

static int __init hisi_rpmb_init(void)
{
	struct sched_param param;

	hisi_rpmb.wake_up_condition = 0;
	init_waitqueue_head(&hisi_rpmb.wait);
	hisi_rpmb.rpmb_task =
		kthread_create(hisi_rpmb_work_thread, NULL, "rpmb_task");
	param.sched_priority = 1;
	sched_setscheduler(hisi_rpmb.rpmb_task, SCHED_FIFO, &param);
	wake_up_process(hisi_rpmb.rpmb_task);

	rpmb_support_device = get_bootdevice_type();
	hisi_rpmb.ops = rpmb_operation_get(rpmb_support_device);
	if (!hisi_rpmb.ops) {
		pr_err("%s: cannot get rpmb ops\n", __func__);
		return -1;
	}

	if (hisi_rpmb_device_init())
		return -1;
	else
		rpmb_device_init_status = RPMB_DEVICE_IS_READY;
	wake_lock_init(&hisi_rpmb.wake_lock, WAKE_LOCK_SUSPEND,
		       "hisi-rpmb-wakelock");

	return 0;
}
late_initcall(hisi_rpmb_init);
MODULE_AUTHOR("qianziye");
MODULE_DESCRIPTION("Hisilicon Secure RPMB.");
MODULE_LICENSE("GPL v2");
