
#include <linux/device.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/hisi-iommu.h>
#include <linux/dma-mapping.h>
#include <linux/dma-iommu.h>
#include <linux/dma-buf.h>
#include <linux/iommu.h>
#include "drv_venc_osal.h"
#include "drv_venc.h"
#include "venc_regulator.h"
/*lint -e750 -e838 -e715*/
#ifndef VM_RESERVED /* for kernel up to 3.7.0 version */
# define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif

typedef struct {
	dev_t dev;
	struct device *venc_device;
	struct cdev cdev;
	struct class *venc_class;
} venc_entry_t;

typedef enum {
	UDP_DEFAULT,
	FPGA_ES,
	FPGA_CS,
} venc_platform_t;

struct semaphore g_venc_sem;

HI_BOOL g_venc_open_flag = HI_FALSE;
HI_BOOL g_venc_dev_detected = HI_FALSE;

// VENC device open times
atomic_t g_venc_count = ATOMIC_INIT(0);


static HI_S32  venc_drv_setup_cdev(venc_entry_t *venc, const struct file_operations *fops);
static HI_S32  venc_drv_cleanup_cdev(venc_entry_t *venc);
static HI_S32  venc_drv_probe(struct platform_device *pdev);
static HI_S32  venc_drv_remove(struct platform_device *pdev);

struct platform_device *g_venc_pdev;

static long venc_drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	HI_S32 ret;
	void __user *user_arg = (void __user *)(uintptr_t)arg;
	HI_VOID *kernel_arg = NULL;
	struct encode_info *encode_info =  NULL;
	struct encode_done_info info = {0};

	venc_buffer_record_t buf_node = {0};

	if (!user_arg) {
		HI_FATAL_VENC("uarg is NULL\n");
		return HI_FAILURE;
	}

	switch (cmd) {
	case CMD_VENC_ENCODE:
		ret = hi_venc_down_interruptible(&g_venc_sem);
		if (ret != HI_SUCCESS) {
			HI_FATAL_VENC("encode: down interruptible failed");
			return ret;
		}

		ret = drv_mem_copy_from_user(cmd, user_arg, &kernel_arg);
		if (ret != HI_SUCCESS) {
			HI_FATAL_VENC("encode: copy from user failed");
			hi_venc_up_interruptible(&g_venc_sem);
			return ret;
		}

		encode_info = (struct encode_info *)kernel_arg;
		ret = venc_drv_encode(encode_info, (struct venc_fifo_buffer *)file->private_data);
		if (ret != HI_SUCCESS) {
			HI_FATAL_VENC("encode failed");
			hi_venc_up_interruptible(&g_venc_sem);
			return ret;
		}

		if (encode_info->is_block) {
			ret = copy_to_user(user_arg, kernel_arg, sizeof(*encode_info));
			if (ret)
				HI_FATAL_VENC("encode copy to user failed");
		}
		hi_venc_up_interruptible(&g_venc_sem);
		break;
	case CMD_VENC_GET_ENCODE_DONE_INFO:
		ret = venc_drv_get_encode_done_info((struct venc_fifo_buffer *)file->private_data, &info);
		if (ret != HI_SUCCESS) {
			HI_FATAL_VENC("encode done info buffer is empty");
			return HI_FAILURE;
		}

		if (copy_to_user(user_arg, &info, sizeof(info))) {
			HI_FATAL_VENC("get encode done info: copy to user failed");
			return HI_FAILURE;
		}
		break;
	case CMD_VENC_IOMMU_MAP:
		if (copy_from_user(&buf_node, user_arg, sizeof(buf_node))) {
			HI_FATAL_VENC("iommu map: copy frome user failed");
			return HI_FAILURE;
		}

		ret = drv_mem_iommumap(&buf_node, g_venc_pdev);
		if (ret != HI_SUCCESS) {
			HI_FATAL_VENC("iommu map failed");
			return HI_FAILURE;
		}

		if (copy_to_user(user_arg, &buf_node, sizeof(buf_node))) {
			HI_FATAL_VENC("iommu map: copy to user failed");
			return HI_FAILURE;
		}
		break;
	case CMD_VENC_IOMMU_UNMAP:
		if (copy_from_user(&buf_node, user_arg, sizeof(buf_node))) {
			HI_FATAL_VENC("iommu unmap: copy frome user failed");
			return HI_FAILURE;
		}

		ret = drv_mem_iommuunmap(buf_node.shared_fd, buf_node.iova, g_venc_pdev);
		if (ret != HI_SUCCESS) {
			HI_FATAL_VENC("iommu unmap failed");
			return HI_FAILURE;
		}
		break;
	default:
		HI_ERR_VENC("venc cmd unknown:0x%x", cmd);
		ret = HI_FAILURE;
		break;
	}

	return ret;
}

static HI_S32 venc_drv_open(struct inode *finode, struct file  *ffile)
{
	HI_S32 ret;

	ret = hi_venc_down_interruptible(&g_venc_sem);
	if (ret) {
		HI_FATAL_VENC("Open down interruptible failed\n");
		return HI_FAILURE;
	}

	if (atomic_read(&g_venc_count) == MAX_OPEN_COUNT) {
		HI_FATAL_VENC("open venc too much\n");
		hi_venc_up_interruptible(&g_venc_sem);
		return -EAGAIN;
	}

	ret = venc_drv_alloc_encode_done_info_buffer(ffile);
	if (ret != 0) {
		HI_FATAL_VENC("malloc buffer fail\n");
		hi_venc_up_interruptible(&g_venc_sem);
		return HI_FAILURE;
	}

	if (atomic_inc_return(&g_venc_count) == 1) {
		ret = venc_drv_board_init();
		if (ret != HI_SUCCESS) {
			HI_FATAL_VENC("board init failed, ret value is %d\n", ret);
			atomic_dec(&g_venc_count);
			venc_drv_free_encode_done_info_buffer(ffile);
			hi_venc_up_interruptible(&g_venc_sem);
			return HI_FAILURE;
		}
		ret = venc_drv_open_vedu();
		if (ret != HI_SUCCESS) {
			HI_FATAL_VENC("venc firmware layer open failed, ret value is %d\n", ret);
			atomic_dec(&g_venc_count);
			venc_drv_board_deinit();
			venc_drv_free_encode_done_info_buffer(ffile);
			hi_venc_up_interruptible(&g_venc_sem);
			return HI_FAILURE;
		}
	}

	g_venc_open_flag = HI_TRUE;
	hi_venc_up_interruptible(&g_venc_sem);

	HI_INFO_VENC("Open venc device successfully");
	return HI_SUCCESS;
}

static HI_S32 venc_drv_close(struct inode *finode, struct file  *ffile)
{
	HI_S32 ret;

	ret = hi_venc_down_interruptible(&g_venc_sem);
	if (ret) {
		HI_FATAL_VENC("Close down interruptible failed\n");
		return HI_FAILURE;
	}

	if (atomic_dec_and_test(&g_venc_count)) {
		venc_drv_close_vedu();
		venc_drv_board_deinit();
		g_venc_open_flag = HI_FALSE;
	}

	ret = venc_drv_free_encode_done_info_buffer(ffile);
	if (ret != 0) {
		HI_FATAL_VENC("free buffer fail\n");
		hi_venc_up_interruptible(&g_venc_sem);
		return HI_FAILURE;
	}

	hi_venc_up_interruptible(&g_venc_sem);

	HI_INFO_VENC("Close venc device successfully");
	return HI_SUCCESS;
}

HI_S32 venc_drv_suspend(struct platform_device *pdev, pm_message_t state)
{
	HI_S32 ret;
	HI_INFO_VENC("+");

	ret = hi_venc_down_interruptible(&g_venc_sem);
	if (ret) {
		HI_ERR_VENC("Suspend down interruptible failed\n");
		return HI_FAILURE;
	}

	if (!g_venc_open_flag)
		goto exit;

	ret = venc_drv_suspend_vedu();
	if (ret != HI_SUCCESS) {
		HI_FATAL_VENC("venc firmware layer suspend failed, ret value is %d\n", ret);
		hi_venc_up_interruptible(&g_venc_sem);
		return HI_FAILURE;
	}

	venc_drv_board_deinit();
exit:
	hi_venc_up_interruptible(&g_venc_sem);
	HI_INFO_VENC("-");
	return HI_SUCCESS;
} /*lint !e715*/

HI_S32 venc_drv_resume(struct platform_device *pdev)
{
	HI_S32 ret;
	HI_INFO_VENC("+");

	ret = hi_venc_down_interruptible(&g_venc_sem);
	if (ret) {
		HI_FATAL_VENC("Resume down interruptible failed\n");
		return HI_FAILURE;
	}

	if (!g_venc_open_flag) {
		goto exit;
	}
	ret = venc_drv_board_init();
	if (ret != HI_SUCCESS) {
		HI_FATAL_VENC("board init failed, ret value is %d\n", ret);
		atomic_dec(&g_venc_count);
		hi_venc_up_interruptible(&g_venc_sem);
		return HI_FAILURE;
	}
	ret = venc_drv_resume_vedu();
	if (ret != HI_SUCCESS) {
		HI_FATAL_VENC("venc firmware layer resume failed, ret value is %d\n", ret);
		atomic_dec(&g_venc_count);
		hi_venc_up_interruptible(&g_venc_sem);
		return HI_FAILURE;
	}
exit:
	hi_venc_up_interruptible(&g_venc_sem);
	HI_INFO_VENC("-");
	return HI_SUCCESS;
} /*lint !e715*/

static struct file_operations venc_fops = {
	.owner          = THIS_MODULE, /*lint !e64*/
	.open           = venc_drv_open,
	.unlocked_ioctl = venc_drv_ioctl,
	.compat_ioctl   = venc_drv_ioctl,
	.release        = venc_drv_close,
}; /*lint !e785*/

static const struct of_device_id venc_of_match[] = {
	{ .compatible = VENC_COMPATIBLE, }, /*lint !e785 !e651*/
	{ } /*lint !e785*/
};


static struct platform_driver venc_driver = {
	.probe   = venc_drv_probe,
	.remove  = venc_drv_remove,
	.suspend = venc_drv_suspend,
	.resume  = venc_drv_resume,
	.driver  = {
		.name    = "hi_venc",
		.owner   = THIS_MODULE, /*lint !e64*/
		.of_match_table = venc_of_match
	}, /*lint !e785*/
}; /*lint !e785*/

static ssize_t venc_drv_show(struct device *dev, struct device_attribute *attr, char *buf)
{
#ifdef VENC_DEBUG_ENABLE
	if (buf == HI_NULL) {
		HI_ERR_VENC("%s buf is null\n", __func__);
		return 0;
	}

	return snprintf(buf, PAGE_SIZE, "0x%pK\n", (void *)(uintptr_t)venc_get_smmu_ttb());    // unsafe_function_ignore: snprintf
#else
	return 0;
#endif
}

static DEVICE_ATTR(omxvenc_misc, 0444, venc_drv_show, NULL);

static HI_S32 venc_drv_setup_cdev(venc_entry_t *venc, const struct file_operations *fops)
{
	HI_S32 err;

	HI_INFO_VENC("enter %s()\n", __func__);
	err = alloc_chrdev_region(&venc->dev, 0, 1, "hi_venc");
	if (err < 0)
		return HI_FAILURE;

	memset((HI_VOID *) & (venc->cdev), 0, sizeof(struct cdev));    // unsafe_function_ignore: memset
	cdev_init(&(venc->cdev), &venc_fops);

	venc->cdev.owner = THIS_MODULE; /*lint !e64*/
	venc->cdev.ops = &venc_fops;
	err = cdev_add(&(venc->cdev), venc->dev, 1);

	/* Create the device category directory hi_venc in the /sys/class/ directory */
	venc->venc_class = class_create(THIS_MODULE, "hi_venc"); /*lint !e64*/
	if (IS_ERR(venc->venc_class)) {
		err = PTR_ERR(venc->venc_class); /*lint !e712*/
		HI_ERR_VENC("Fail to create hi_venc class\n");
		goto unregister_region;
	}

	/* Create the device file hi_venc in /dev/ and /sys/class/hi_venc respectively */
	venc->venc_device = device_create(venc->venc_class, NULL, venc->dev, "%s", "hi_venc");
	if (IS_ERR(venc->venc_device)) {
		err = PTR_ERR(venc->venc_device); /*lint !e712*/
		HI_ERR_VENC("Fail to create hi_venc device\n");
		goto cls_destroy;
	}

	err = device_create_file(venc->venc_device, &dev_attr_omxvenc_misc);
	if (err) {
		HI_ERR_VENC("%s, failed to create device file\n", __func__);
		goto dev_destroy;
	}

	HI_INFO_VENC("exit %s()\n", __func__);
	return HI_SUCCESS;

dev_destroy:
	device_destroy(venc->venc_class, venc->dev);
cls_destroy:
	class_destroy(venc->venc_class);
	venc->venc_class = HI_NULL;
unregister_region:
	unregister_chrdev_region(venc->dev, 1);
	return HI_FALSE;
} /*lint !e550*/

static HI_S32 venc_drv_cleanup_cdev(venc_entry_t *venc)
{
	if (venc->venc_class) {
		device_remove_file(venc->venc_device, &dev_attr_omxvenc_misc);
		device_destroy(venc->venc_class, venc->dev);
		class_destroy(venc->venc_class);
	}

	cdev_del(&(venc->cdev));
	unregister_chrdev_region(venc->dev, 1);

	return 0;
}

static HI_U32 venc_drv_device_idle(venc_platform_t platform_type)
{
	HI_U32 idle   = 0;
	HI_U32 *platform_ctrl = HI_NULL;
	switch (platform_type) {
#ifdef VENC_ES_SUPPORT
	case FPGA_ES:
		platform_ctrl = (HI_U32 *)ioremap(VENC_PCTRL_PERI_ES, (unsigned long)4); /*lint !e747*/
		if (platform_ctrl)
			idle = readl(platform_ctrl) & VENC_EXISTBIT_ES; /*lint !e50 !e64*/
		break;
#endif

#ifdef VENC_CS_SUPPORT
	case FPGA_CS:
		platform_ctrl = (HI_U32 *)ioremap(VENC_PCTRL_PERI_CS, (unsigned long)4); /*lint !e747*/
		if (platform_ctrl)
			idle = readl(platform_ctrl) & VENC_EXISTBIT_CS; /*lint !e50 !e64*/ //b[18]
		break;
#endif

	default:
		printk(KERN_ERR "unkown platform_type %d\n", platform_type);
		break;
	}

	if (!platform_ctrl) {
		printk(KERN_ERR "ioremap failed\n");
		return HI_FALSE;
	} else {
		iounmap(platform_ctrl);
	}

	return (HI_U32)((idle != 0) ? HI_TRUE : HI_FALSE);
}

static HI_S32 venc_drv_probe(struct platform_device *pdev)
{
	HI_S32 ret;
	venc_entry_t *venc = HI_NULL;
	HI_U32 fpga_cs_flag = 0;
	HI_U32 fpga_es_flag = 0;
	venc_platform_t platform_name;
	struct device *dev = HI_NULL;
	if (!pdev) {
		HI_FATAL_VENC("invalid argument");
		return HI_FAILURE;
	}

	dev = &pdev->dev;
	platform_name = UDP_DEFAULT;

	HI_INFO_VENC("omxvenc prepare to probe\n");
	hi_venc_init_mutex(&g_venc_sem);

#ifdef VENC_ES_SUPPORT
	ret = of_property_read_u32(dev->of_node, VENC_FPGAFLAG_ES, &fpga_es_flag);
	if (ret)
		HI_INFO_VENC("read failed, but fpga_es has defualt value\n");
	if (fpga_es_flag == 1)
		platform_name = FPGA_ES;
#endif

#ifdef VENC_CS_SUPPORT
	ret = of_property_read_u32(dev->of_node, VENC_FPGAFLAG_CS, &fpga_cs_flag);
	if (ret)
		HI_INFO_VENC("read failed, but fpga_cs has defualt value\n");
	if (fpga_cs_flag == 1)
		platform_name = FPGA_CS;
#endif

	if ((fpga_cs_flag == 1) || (fpga_es_flag == 1)) {
		printk(KERN_INFO "fpga platform\n");
		if (venc_drv_device_idle(platform_name) == HI_FALSE) {
			printk(KERN_ERR "venc is not exist\n");
			return HI_FAILURE;
		}
	} else {
		printk(KERN_INFO "not fpga platform\n");
	}

	if (g_venc_dev_detected) {
		HI_INFO_VENC("venc device detected already\n");
		return HI_SUCCESS;
	}

	ret = venc_drv_init_lock();
	if (ret != HI_SUCCESS) {
		HI_ERR_VENC("init lock failed");
		return ret;
	}

	venc = hi_mem_valloc(sizeof(venc_entry_t)); /*lint !e747*/
	if (!venc) {
		HI_FATAL_VENC("call vmalloc failed\n");
		goto destroy_lock;
	}

	memset((HI_VOID *)venc, 0, sizeof(venc_entry_t));    // unsafe_function_ignore: memset
	ret = venc_drv_setup_cdev(venc, &venc_fops);
	if (ret < 0) {
		HI_ERR_VENC("setup char device failed\n");
		goto free;
	}

	platform_set_drvdata(pdev, venc);
	g_venc_pdev = pdev;
	g_venc_dev_detected = HI_TRUE;

	/* 1 read venc dts info from dts */
	ret = get_dts_config_info(pdev);
	if (ret != HI_SUCCESS) {
		HI_FATAL_VENC("get venc DTS config info failed");
		goto cleanup;
	}

	/* 2 get regulator interface */
	ret = get_regulator_info(pdev);
	if (ret != HI_SUCCESS) {
		HI_FATAL_VENC("get venc regulator failed");
		goto cleanup;
	}

	HI_INFO_VENC("omxvenc probe successfully\n");
	return ret;

cleanup:
	venc_drv_cleanup_cdev(venc);
free:
	hi_mem_vfree(venc);
destroy_lock:
	venc_drv_destroy_lock();

	return ret;
}

static HI_S32 venc_drv_remove(struct platform_device *pdev)
{
	venc_entry_t *venc = NULL;
	HI_INFO_VENC("omxvenc prepare to remove\n");

	venc = platform_get_drvdata(pdev);
	if (venc) {
		venc_drv_cleanup_cdev(venc);
	} else {
		HI_ERR_VENC("get platform drvdata err\n");
	}

	platform_set_drvdata(pdev, NULL);
	hi_mem_vfree(venc);
	venc_drv_destroy_lock();
	g_venc_dev_detected = HI_FALSE;

	HI_INFO_VENC("remove omxvenc successfully\n");
	return 0;
}


struct platform_device *venc_get_device(HI_VOID)
{
    return g_venc_pdev;
}

HI_S32 __init venc_drv_mod_init(HI_VOID)
{
	HI_S32 ret;

	HI_INFO_VENC("enter %s()\n", __func__);

	ret = platform_driver_register(&venc_driver); /*lint !e64*/
	if (ret) {
		HI_ERR_VENC("register platform driver failed\n");
		return ret;
	}
	HI_INFO_VENC("success\n");

#ifdef MODULE
	HI_INFO_VENC("Load hi_venc.ko success\t(%s)\n", VERSION_STRING);
#endif

	HI_INFO_VENC("exit %s()\n", __func__);

	return ret;
}

HI_VOID venc_drv_mod_exit(HI_VOID)
{
	HI_INFO_VENC("enter %s()\n", __func__);

	platform_driver_unregister(&venc_driver);

	HI_INFO_VENC("exit %s()\n", __func__);
	return;
}
/*lint -e528*/
module_init(venc_drv_mod_init); /*lint !e528*/
module_exit(venc_drv_mod_exit); /*lint !e528*/
/*lint -e753*/
MODULE_LICENSE("Dual BSD/GPL"); /*lint !e753*/

