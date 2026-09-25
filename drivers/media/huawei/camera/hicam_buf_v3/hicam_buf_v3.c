

#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>

#include <hicam_buf.h>
#include <cam_log.h>
#include <hicam_buf_v3_priv.h>
#include <securec.h>


struct hicam_buf_v3_device {
	struct miscdevice dev;
	struct platform_device *pdev;
	atomic_t ref_count;
	enum hicam_buf_dev_type devtype;
	int padding_support;
	char filename[HICAM_BUF_FILE_MAX_LEN];
	void *private;
};

int hicam_buf_v3_map_iommu(struct device *dev, int fd, struct iommu_format *fmt, int padding_support)
{
	if (IS_ERR_OR_NULL(dev)) {
		cam_err("%s: hicam_buf_dev is null", __func__);
		return -EINVAL;
	}

	if (fd < 0 || !fmt) {
		cam_err("%s: fd or fmt invalid", __func__);
		return -EINVAL;
	}

	return hicam_v3_internal_map_iommu(dev, fd, fmt, padding_support);
}

void hicam_buf_v3_unmap_iommu(struct device *dev, int fd, struct iommu_format *fmt, int padding_support)
{
	if (IS_ERR_OR_NULL(dev)) {
		cam_err("%s: hicam_buf_dev is null", __func__);
		return ;
	}

	if (fd < 0 || !fmt) {
		cam_err("%s: fd or fmt invalid", __func__);
		return ;
	}

	hicam_v3_internal_unmap_iommu(dev, fd, fmt, padding_support);
}

static long hicam_config_v3(struct hicam_buf_v3_device *hicam_buf_v3_dev, struct hicam_buf_cfg *cfg)
{
	long ret = 0;
	if (IS_ERR_OR_NULL(hicam_buf_v3_dev)) {
		cam_err("%s: hicam_buf_dev is null", __func__);
		return -ENODEV;
	}

	switch (cfg->type) {
	case HICAM_BUF_MAP_IOMMU:
		ret = hicam_buf_v3_map_iommu(&hicam_buf_v3_dev->pdev->dev, cfg->fd,
				&cfg->iommu_format, hicam_buf_v3_dev->padding_support);
		break;
	case HICAM_BUF_UNMAP_IOMMU:
		hicam_buf_v3_unmap_iommu(&hicam_buf_v3_dev->pdev->dev, cfg->fd,
				&cfg->iommu_format, hicam_buf_v3_dev->padding_support);
		break;
	default:
		cam_err("%s: can't handle type[%d]", __func__, cfg->type);
	}

	return ret;
}

static long hicam_buf_v3_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	struct hicam_buf_cfg data;
	struct miscdevice *mdev = NULL;
	struct hicam_buf_v3_device * idev = NULL;

	if (_IOC_SIZE(cmd) > sizeof(data)) {
		cam_err("%s: cmd size too large!\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user(&data, (void __user *)arg, _IOC_SIZE(cmd))){
		cam_err("%s: copy in arg failed!\n", __func__);
		return -EFAULT;
	}

	mdev = filp->private_data;
	idev = container_of(mdev, struct hicam_buf_v3_device, dev);

	switch (cmd) {
	case HICAM_BUF_IOC_CFG:
		ret = hicam_config_v3(idev, &data);
		break;
	default:
		cam_info("%s: invalid command %d", __func__, cmd);
		return -EINVAL;
	}

	if (copy_to_user((void __user *)arg, &data, _IOC_SIZE(cmd))){
		cam_err("%s: copy back arg failed!\n", __func__);
		return -EFAULT;
	}

	return ret;
}

static int hicam_buf_v3_release(struct inode *inode, struct file *file)
{
	int ref_count;
	struct hicam_buf_v3_device *idev = file->private_data;
	struct miscdevice *mdev = &idev->dev;

	ref_count = atomic_dec_return(&idev->ref_count);
	cam_info("%s: %s device closed, ref:%d",
			__func__, mdev->name, ref_count);
	return 0;
}

static int hicam_buf_v3_open(struct inode *inode, struct file *file)
{
	int ref_count;
	struct miscdevice *mdev = file->private_data;
	struct hicam_buf_v3_device *idev = container_of(mdev, struct hicam_buf_v3_device, dev);

	ref_count = atomic_inc_return(&idev->ref_count);
	cam_info("%s: %s device opened, ref:%d",
			__func__, mdev->name, ref_count);
	return 0;
}

static const struct file_operations hicam_buf_v3_fops = {
	.owner = THIS_MODULE,
	.open = hicam_buf_v3_open,
	.release = hicam_buf_v3_release,
	.unlocked_ioctl = hicam_buf_v3_ioctl,
#if CONFIG_COMPAT
	.compat_ioctl = hicam_buf_v3_ioctl,
#endif
};

static int hicam_buf_v3_parse_of_node(struct hicam_buf_v3_device *idev)
{
	int ret;
	struct device_node *np = idev->pdev->dev.of_node;
	if (!np) {
		return -ENODEV;
	}

	ret = of_property_read_u32(np, "huawei,devtype", &(idev->devtype));
	if (ret < 0)
		cam_err("%s: get devtype failed", __func__);

	idev->padding_support = 0;  // default not support iova padding mode
	ret = of_property_read_u32(np, "huawei,support-iova-padding", &(idev->padding_support));
	cam_info("%s: get huawei,support-iova-padding %d", __func__, idev->padding_support);

	return 0;
}

struct hicam_buf_v3_device *hicam_buf_v3_device_create(struct platform_device *pdev)
{
	int ret;
	struct hicam_buf_v3_device *idev;

	idev = kzalloc(sizeof(*idev), GFP_KERNEL);
	if (!idev)
		return ERR_PTR(-ENOMEM);

	atomic_set(&idev->ref_count, 0);
	idev->pdev = pdev;

	ret = hicam_buf_v3_parse_of_node(idev);
	if (ret) {
		cam_err("%s: failed to parse device of_node", __func__);
		goto err_out;
	}
	ret = sprintf_s(idev->filename, HICAM_BUF_FILE_MAX_LEN, "hicam_buf%d", idev->devtype);
	if (ret < 0) {
		cam_err("%s: failed to sprintf_s filename, devtype %d", __func__, idev->devtype);
		goto err_out;
	}

	idev->dev.minor = MISC_DYNAMIC_MINOR;
	idev->dev.name = idev->filename; /* dev name under /dev */
	idev->dev.fops = &hicam_buf_v3_fops;
	idev->dev.parent = NULL;
	ret = misc_register(&idev->dev);
	if (ret) {
		cam_err("%s: failed to register misc device", __func__);
		goto err_out;
	}
	cam_info("%s: create %s success", __func__, idev->dev.name);

	return idev;
err_out:
	kfree(idev);
	return ERR_PTR(ret);
}

void hicam_buf_v3_device_destroy(struct hicam_buf_v3_device *idev)
{
	misc_deregister(&idev->dev);
	kfree(idev);
}

static int32_t hicam_buf_v3_probe(struct platform_device* pdev)
{
	int rc = 0;
	struct hicam_buf_v3_device* hicam_buf_v3_dev = NULL;

	if (!pdev) {
		cam_err("%s: null pdev", __func__);
		return -ENODEV;
	}

	dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64)); /*lint !e598 !e648 */
	hicam_buf_v3_dev = hicam_buf_v3_device_create(pdev);
	if (IS_ERR(hicam_buf_v3_dev)) {
		cam_err("%s: fail to create hicam ion device", __func__);
		rc = PTR_ERR(hicam_buf_v3_dev);
		goto err_create_device;
	}

	platform_set_drvdata(pdev, hicam_buf_v3_dev);
	return 0;

err_create_device:
	hicam_buf_v3_dev = NULL;
	return rc;
}

static int32_t hicam_buf_v3_remove(struct platform_device* pdev)
{
	struct hicam_buf_v3_device* hicam_buf_v3_dev = platform_get_drvdata(pdev);

	if (!hicam_buf_v3_dev) {
		cam_err("%s: hicam_buf_dev is not inited", __func__);
		return -EINVAL;
	}

	hicam_buf_v3_device_destroy(hicam_buf_v3_dev);

	return 0;
}

static const struct of_device_id hicam_buf_v3_dt_match[] = {
	{ .compatible = "huawei,hicam_buf_v3", },
	{ },
};
MODULE_DEVICE_TABLE(of, hicam_buf_v3_dt_match);

static struct platform_driver hicam_buf_v3_platform_driver = {
	.driver = {
		.name = "huawei,hicam_buf_v3",
		.owner = THIS_MODULE,
		.of_match_table = hicam_buf_v3_dt_match,
	},

	.probe = hicam_buf_v3_probe,
	.remove = hicam_buf_v3_remove,
};

static int __init hicam_buf_v3_init_module(void)
{
	cam_info("enter %s", __func__);
	return platform_driver_register(&hicam_buf_v3_platform_driver);
}

static void __exit hicam_buf_v3_exit_module(void)
{
	platform_driver_unregister(&hicam_buf_v3_platform_driver);
}

rootfs_initcall(hicam_buf_v3_init_module);
module_exit(hicam_buf_v3_exit_module);

MODULE_DESCRIPTION("hicam_buf_v3");
MODULE_LICENSE("GPL v2");
