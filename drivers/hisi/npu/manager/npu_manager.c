#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/swap.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/wakelock.h>
#include <linux/atomic.h>
#include <dsm/dsm_pub.h>

#include "npu_manager.h"
#include "npu_proc_ctx.h"
#include "npu_manager_ioctl_services.h"
#include "npu_ioctl_services.h"
#include "devdrv_user_common.h"
#include "npu_calc_channel.h"
#include "npu_calc_cq.h"
#include "npu_stream.h"
#include "npu_shm.h"
#include "npu_common.h"
#include "npu_devinit.h"
#include "drv_log.h"
#include "npu_pm.h"
#include "npu_firmware.h"
#include "npu_recycle.h"
#include "bbox/npu_black_box.h"
#include "npu_mailbox_msg.h"
#include "npu_manager_common.h"

static unsigned int g_npu_manager_major;
static struct class *g_npu_manager_class = NULL;
static struct devdrv_manager_info *g_dev_manager_info = NULL;

static int (*devdrv_npu_ioctl_call[DEVDRV_MAX_CMD])
    (struct devdrv_proc_ctx *proc_ctx, unsigned long arg) = {NULL};

struct dsm_dev dev_davinci = {
    .name = "dsm_ai",
    .device_name = NULL,
    .ic_name = NULL,
    .module_name = NULL,
    .fops = NULL,
    .buff_size = 1024,
};
struct dsm_client *davinci_dsm_client = NULL;

static int devdrv_manager_open(struct inode *inode, struct file *filep)
{
	devdrv_drv_debug("devdrv_manager_open start\n");

	return 0;
}

static int devdrv_manager_release(struct inode *inode, struct file *filep)
{
	devdrv_drv_debug("devdrv_manager_release start\n");

	return 0;
}

struct devdrv_manager_info *devdrv_get_manager_info(void)
{
	return g_dev_manager_info;/*dev_manager_info*/
}
EXPORT_SYMBOL(devdrv_get_manager_info);

const struct file_operations devdrv_manager_fops = {
	.owner		= THIS_MODULE,
	.open		= devdrv_manager_open,
	.release	= devdrv_manager_release,
	.unlocked_ioctl	= devdrv_manager_ioctl,
};

static int devdrv_npu_map(struct file *filep, struct vm_area_struct *vma)
{
	unsigned long vm_pgoff = 0;
	struct devdrv_proc_ctx *proc_ctx = NULL;
	u32 map_type = 0;
	u32 share_num = 0;
	u8 dev_id = 0;
	int ret = 0;

	if ((vma == NULL) || (filep == NULL))
	{
		devdrv_drv_err("invalid vma or filep\n");
		return -EFAULT;
	}

	proc_ctx = (struct devdrv_proc_ctx *)filep->private_data;
	if(proc_ctx == NULL){
		devdrv_drv_err("proc_ctx is NULL\n");
		return -EFAULT;
	}

	dev_id = proc_ctx->devid;
	vm_pgoff = vma->vm_pgoff;
	map_type = MAP_GET_TYPE(vm_pgoff);
	mutex_lock(&proc_ctx->map_mutex);
	devdrv_drv_debug("map_type = %d memory mmap start ~\n",map_type);
	switch(map_type)
	{
		case MAP_RESERVED:
				ret = -EINVAL;
				break;
		case MAP_L2_BUFF:
				ret = devdrv_map_l2_buff(filep, vma, dev_id);
				break;
		case MAP_CONTIGUOUS_MEM:
				share_num = MAP_GET_SHARE_NUM(vm_pgoff);
				ret = devdrv_map_cm(filep, vma, share_num,dev_id);
				break;
		case MAP_INFO_SQ_CQ_MEM:
				ret = devdrv_info_sq_cq_mmap(dev_id,filep,vma);
				break;
		default:
				ret = -EINVAL;
				break;
	}
	mutex_unlock(&proc_ctx->map_mutex);
	if(ret != 0){
		devdrv_drv_err("map_type = %d memory mmap failed\n",map_type);
	}
	return ret;
}

static int devdrv_npu_open(struct inode *inode, struct file* filep)
{
	struct devdrv_dev_ctx* cur_dev_ctx = NULL;
	struct devdrv_proc_ctx *proc_ctx = NULL;
	unsigned int minor = iminor(inode);
	struct devdrv_ts_cq_info *cq_info = NULL;
	//stub
	const u8 dev_id = 0; //should get from manager info
	int ret = 0;

	devdrv_drv_warn("devdrv_open start. minor = %d \n",minor);

	if (minor >= DEVDRV_MAX_DAVINCI_NUM) {
		devdrv_drv_err("invalid npu minor num, minor = %d\n", minor);
		return -ENODEV;
	}

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if(cur_dev_ctx == NULL){
		devdrv_drv_err("cur_dev_ctx %d is null\n",dev_id);
		return  -ENODEV;
	}

	mutex_lock(&cur_dev_ctx->npu_open_release_mutex);

	proc_ctx = kzalloc(sizeof(struct devdrv_proc_ctx), GFP_KERNEL);
	if(proc_ctx == NULL) {
		mutex_unlock(&cur_dev_ctx->npu_open_release_mutex);
		devdrv_drv_err("alloc memory for proc_ctx failed.\n");
		return -ENOMEM;
	}

	devdrv_proc_ctx_init(proc_ctx);

	proc_ctx->devid = cur_dev_ctx->devid;
	proc_ctx->pid = current->tgid;
	filep->private_data = (void *)proc_ctx;

	//alloc cq for current process
	mutex_lock(&cur_dev_ctx->cq_mutex_t);
	cq_info = devdrv_proc_alloc_cq(proc_ctx);
	if (cq_info == NULL) {
		mutex_unlock(&cur_dev_ctx->cq_mutex_t);
		devdrv_drv_err("alloc persistent cq for proc_context failed.\n");
		ret = -ENOMEM;
		goto proc_alloc_cq_failed;
	}
	devdrv_drv_debug("alloc persistent cq for proc_context success.\n");
	mutex_unlock(&cur_dev_ctx->cq_mutex_t);

	//add proc_ctx to cur dev_ctx proc list
	(void)devdrv_add_proc_ctx(&proc_ctx->dev_ctx_list,dev_id);

	//bind proc_ctx to cq report int ctx
	devdrv_bind_proc_ctx_with_cq_int_ctx(proc_ctx);

	ret = npu_open(cur_dev_ctx);
	if (ret != 0) {
		devdrv_drv_err("npu open failed.\n");
		goto npu_open_failed;
	}

	mutex_unlock(&cur_dev_ctx->npu_open_release_mutex);
	devdrv_cm_resource_recycle(cur_dev_ctx);

	devdrv_drv_debug("devdrv_open succeed.\n");
	return 0; //lint !e454

npu_open_failed:
	devdrv_unbind_proc_ctx_with_cq_int_ctx(proc_ctx);
	(void)devdrv_remove_proc_ctx(&proc_ctx->dev_ctx_list,proc_ctx->devid);
	(void)devdrv_proc_free_cq(proc_ctx);
proc_alloc_cq_failed:
	mutex_unlock(&cur_dev_ctx->npu_open_release_mutex);
	kfree(proc_ctx);
	proc_ctx = NULL;
	return ret;
}

static int devdrv_npu_release(struct inode *inode, struct file *filep)
{
	int ret = 0;
	struct devdrv_dev_ctx* cur_dev_ctx = NULL;
	struct devdrv_proc_ctx *proc_ctx = NULL;
	u8 dev_id = 0; //should get from manager info
	proc_ctx = (struct devdrv_proc_ctx*)filep->private_data;
	if (proc_ctx == NULL) {
		devdrv_drv_err("get proc_ctx fail.\n");
		return -EINVAL;
	}
	devdrv_drv_warn("devdrv_release start. minor = %d,dev_id = %d\n",
		iminor(inode), proc_ctx->devid);

	dev_id = proc_ctx->devid;
	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (cur_dev_ctx == NULL) {
		devdrv_drv_err("cur_dev_ctx %d is null\n",dev_id);
		return -EINVAL;
	}

	devdrv_cm_resource_recycle(cur_dev_ctx);

	//callback char dev release rs before npu powerdown
	devdrv_release_npu_callback_proc(dev_id);

	mutex_lock(&cur_dev_ctx->npu_open_release_mutex);

	//resource leak happened
	if (devdrv_is_proc_resource_leaks(proc_ctx) == true) {

		devdrv_resource_leak_print(proc_ctx);
		//to be done, should judge is ts alive to decide whether inform ts
		devdrv_recycle_npu_resources(proc_ctx);

		mutex_lock(&cur_dev_ctx->npu_power_up_off_mutex);

		ret = npu_powerdown(cur_dev_ctx);
		if (ret != -EBUSY && ret !=0) {
			devdrv_drv_err("npu powerdown failed.\n");
		}
		mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);

		ret = npu_release(cur_dev_ctx);
		if (ret != 0) {
			devdrv_drv_err("npu release failed.\n");
		}

		kfree(proc_ctx);
		proc_ctx = NULL;
		mutex_unlock(&cur_dev_ctx->npu_open_release_mutex); //lint !e455

		return 0;
	}

	//normal case
	devdrv_unbind_proc_ctx_with_cq_int_ctx(proc_ctx);
	(void)devdrv_remove_proc_ctx(&proc_ctx->dev_ctx_list,proc_ctx->devid);
	(void)devdrv_proc_free_cq(proc_ctx);

	mutex_lock(&cur_dev_ctx->npu_power_up_off_mutex);

	ret = npu_powerdown(cur_dev_ctx);
	if (ret != -EBUSY && ret !=0) {
		devdrv_drv_err("npu powerdown failed.\n");
	}
	mutex_unlock(&cur_dev_ctx->npu_power_up_off_mutex);

	ret = npu_release(cur_dev_ctx);
	if (ret != 0) {
		devdrv_drv_err("npu release failed.\n");
	}

	mutex_lock(&cur_dev_ctx->npu_wake_lock_mutex);
	wake_unlock(&cur_dev_ctx->wakelock);
	mutex_unlock(&cur_dev_ctx->npu_wake_lock_mutex);

	kfree(proc_ctx);
	proc_ctx = NULL;
	mutex_unlock(&cur_dev_ctx->npu_open_release_mutex); //lint !e455

	devdrv_drv_debug("devdrv_npu_release succeed.\n");
	return ret;
}

const struct file_operations npu_dev_fops = {
	.owner		= THIS_MODULE,
	.open		= devdrv_npu_open,
	.release	= devdrv_npu_release,
	.unlocked_ioctl	= devdrv_npu_ioctl,
	.mmap = devdrv_npu_map,
};


static char* devdrv_manager_devnode(struct device *dev, umode_t *mode)
{
	if(mode != NULL)
		*mode = 0666;
	return NULL;
}

static int devdrv_register_manager_chrdev(dev_t npu_manager_dev,dev_t devno)
{
	struct device *i_device = NULL;
	int ret = 0;

	ret = alloc_chrdev_region(&npu_manager_dev, 0,
			DEVDRV_MAX_DAVINCI_NUM, "npu_manager");
	if (ret != 0) {
		devdrv_drv_err("npu manager alloc_chrdev_region error.\n");
		return -1;
	}

	g_dev_manager_info->cdev.owner = THIS_MODULE;
	g_npu_manager_major = MAJOR(npu_manager_dev);
	devno = MKDEV(g_npu_manager_major, 0);

	g_npu_manager_class = class_create(THIS_MODULE, "npu_manager");
	if (IS_ERR(g_npu_manager_class)) {
		devdrv_drv_err("npu_manager_class create error.\n");
		ret = -EINVAL;
		goto class_fail;
	}

	g_npu_manager_class->devnode = devdrv_manager_devnode;

	cdev_init(&g_dev_manager_info->cdev, &devdrv_manager_fops);
	ret = cdev_add(&g_dev_manager_info->cdev, devno,DEVDRV_MAX_DAVINCI_NUM);
	if (ret != 0) {
		devdrv_drv_err("npu manager cdev_add error.\n");
		goto cdev_setup_fail;
	}

	i_device = device_create(	g_npu_manager_class, NULL,
								devno, NULL,"davinci_manager");
	if (i_device == NULL) {
		devdrv_drv_err("npu manager device_create error.\n");
		ret = -ENODEV;
		goto device_create_fail;
	}

	return ret;
device_create_fail:
	cdev_del(&g_dev_manager_info->cdev);
cdev_setup_fail:
	class_destroy(g_npu_manager_class);
class_fail:
	unregister_chrdev_region(npu_manager_dev, DEVDRV_MAX_DAVINCI_NUM);
	return ret;
}

static void devdrv_unregister_manager_chrdev(dev_t npu_manager_dev,dev_t devno)
{

	device_destroy(g_npu_manager_class, devno);

	cdev_del(&g_dev_manager_info->cdev);

	class_destroy(g_npu_manager_class);

	unregister_chrdev_region(npu_manager_dev, DEVDRV_MAX_DAVINCI_NUM);

}

static void devdrv_npu_ioctl_call_init(void)
{
	int i = 0;
	for(i = 0; i< DEVDRV_MAX_CMD;i++){
		devdrv_npu_ioctl_call[i] = NULL;
	}

	devdrv_npu_ioctl_call[_IOC_NR(DEVDRV_ALLOC_STREAM_ID)] =
		devdrv_ioctl_alloc_stream;
	devdrv_npu_ioctl_call[_IOC_NR(DEVDRV_FREE_STREAM_ID)] =
		devdrv_ioctl_free_stream;
	devdrv_npu_ioctl_call[_IOC_NR(DEVDRV_GET_OCCUPY_STREAM_ID)] =
		devdrv_ioctl_get_occupy_stream_id;
	devdrv_npu_ioctl_call[_IOC_NR(DEVDRV_ALLOC_EVENT_ID)] =
		devdrv_ioctl_alloc_event;
	devdrv_npu_ioctl_call[_IOC_NR(DEVDRV_FREE_EVENT_ID)] =
		devdrv_ioctl_free_event;
	devdrv_npu_ioctl_call[_IOC_NR(DEVDRV_ALLOC_MODEL_ID)] =
		devdrv_ioctl_alloc_model;
	devdrv_npu_ioctl_call[_IOC_NR(DEVDRV_FREE_MODEL_ID)] =
		devdrv_ioctl_free_model;
	devdrv_npu_ioctl_call[_IOC_NR(DEVDRV_ALLOC_TASK_ID)] =
		devdrv_ioctl_alloc_task;
	devdrv_npu_ioctl_call[_IOC_NR(DEVDRV_FREE_TASK_ID)] =
		devdrv_ioctl_free_task;
	devdrv_npu_ioctl_call[_IOC_NR(DEVDRV_MAILBOX_SEND)] =
		devdrv_ioctl_mailbox_send;
	devdrv_npu_ioctl_call[_IOC_NR(DEVDRV_REPORT_WAIT)] =
		devdrv_ioctl_report_wait;
	devdrv_npu_ioctl_call[_IOC_NR(DEVDRV_GET_TS_TIMEOUT_ID)] =
		devdrv_ioctl_get_ts_timeout;
	devdrv_npu_ioctl_call[_IOC_NR(DEVDRV_CUSTOM_IOCTL)] =
		devdrv_ioctl_custom;
}

int devdrv_proc_npu_ioctl_call(struct devdrv_proc_ctx *proc_ctx,
	unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	if(cmd < _IO(DEVDRV_ID_MAGIC, 1) ||
		cmd >= _IO(DEVDRV_ID_MAGIC, DEVDRV_MAX_CMD)) {
		devdrv_drv_err("parameter,arg = 0x%lx,cmd = %d\n", arg, cmd);
		return -EINVAL;
	}

	devdrv_drv_debug("IOC_NR = %d  cmd = %d\n",_IOC_NR(cmd),cmd);

	if(devdrv_npu_ioctl_call[_IOC_NR(cmd)] == NULL) {
		devdrv_drv_err("devdrv_proc_npu_ioctl_call invalid cmd = %d\n",cmd);
		return -EINVAL;
	}

	// porcess ioctl
	ret = devdrv_npu_ioctl_call[_IOC_NR(cmd)](proc_ctx, arg);
	if(ret != 0) {
		devdrv_drv_err("process failed,arg = %d\n", cmd);
		return -EINVAL;
	}

	return ret;
}

static int __init devdrv_manager_init(void)
{
	const dev_t npu_manager_dev = 0;
	const dev_t npu_manager_devno = 0;
	const u8 dev_id = 0;
	int ret = 0;
	struct devdrv_platform_info *plat_info = NULL;

	devdrv_drv_debug("npu dev %d drv_manager_init start \n", dev_id);

	plat_info = devdrv_plat_get_info();
	if (plat_info == NULL) {
		devdrv_drv_err("devdrv_plat_get_info failed.\n");
		return -ENODEV;
	}

	/* bbox black box init */
	ret = npu_black_box_init();
	if (ret != 0) {
		devdrv_drv_err("Failed npu_black_box_init ! ret = %d\n", ret);
		return ret;
	}

	g_dev_manager_info = kzalloc(sizeof(struct devdrv_manager_info), GFP_KERNEL);
	if(g_dev_manager_info == NULL){
		devdrv_drv_err("kzalloc npu g_dev_manager_info failed.\n");
		return -ENOMEM;
	}

	g_dev_manager_info->plat_info = DEVDRV_MANAGER_DEVICE_ENV;
	ret = devdrv_register_manager_chrdev(npu_manager_dev,npu_manager_devno);
	if(ret != 0){
		devdrv_drv_err("register npu manager chrdev failed ret = %d \n", ret);
		ret = -ENODEV;
		goto register_manager_chrdev_failed;
	}

	devdrv_npu_ioctl_call_init();
	//init npu powerup or powerdown register info
	devdrv_register_callbak_info_init();

	if (davinci_dsm_client == NULL) {
		davinci_dsm_client = dsm_register_client(&dev_davinci);
		if (davinci_dsm_client == NULL) {
			devdrv_drv_err("dsm_register_client register fail.\n");
		}
	}
	//npu device resoure init
	ret =  devdrv_devinit(THIS_MODULE,dev_id,&npu_dev_fops);
	if(ret != 0){
		devdrv_drv_err("npu dev %d devdrv_devinit failed\n",dev_id);
		ret = -ENODEV;
		goto npu_devinit_failed;
	}

	ret = devdrv_request_cq_report_irq_bh();
	if(ret != 0){
		devdrv_drv_err("devdrv_request_cq_report_irq_bh failed \n");
		goto request_cq_report_irq_failed;
	}

	devdrv_drv_debug("devdrv_manager_init success\n");
	return ret;

request_cq_report_irq_failed:
npu_devinit_failed:
	devdrv_unregister_manager_chrdev(npu_manager_dev,npu_manager_devno);
register_manager_chrdev_failed:
	kfree(g_dev_manager_info);
	g_dev_manager_info = NULL;
	devdrv_drv_err("npu devdrv_manager_init failed\n");
	return ret;
}

module_init(devdrv_manager_init);

static void __exit devdrv_manager_exit(void)
{
	dev_t npu_manager_dev = 0;
	dev_t npu_manager_devno = 0;
	const u8 dev_id = 0;//default npu 0 on lite plat

	if (davinci_dsm_client != NULL)
	{
		dsm_unregister_client(davinci_dsm_client, &dev_davinci);
		davinci_dsm_client = NULL;
	}

	//free irq and kill tasklet
	(void)devdrv_free_cq_report_irq_bh();

	//dev resource unregister
	(void)devdrv_devexit(dev_id);

	//unregister npu manager char dev
	devdrv_unregister_manager_chrdev(npu_manager_dev,npu_manager_devno);
	//free mem
	if (g_dev_manager_info != NULL) {
		kfree(g_dev_manager_info);
		g_dev_manager_info = NULL;
	}

	/* bbox black box uninit */
	(void)npu_black_box_exit();
}

module_exit(devdrv_manager_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("DAVINCI driver");
MODULE_VERSION("V1.0");

