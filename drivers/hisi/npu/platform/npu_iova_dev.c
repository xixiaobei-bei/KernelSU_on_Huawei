#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/errno.h>

#include "npu_log.h"
#include "npu_platform.h"

int npu_iova_probe(struct platform_device *pdev)
{
	struct npu_platform_info *platform_info = NULL;

	npu_drv_debug("npu_smmu_probe start\n");

	platform_info = npu_plat_get_info();
	if (platform_info == NULL) {
		npu_drv_err("npu_plat_get_info failed.\n");
		return -ENODEV;
	}

	platform_info->p_ts_subsys_dev = &pdev->dev;
	return 0;
}

int npu_iova_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id npu_iova_of_match[] = {
	{ .compatible = "hisilicon,npu_iova",},
	{},
};
MODULE_DEVICE_TABLE(of, npu_iova_of_match);

static struct platform_driver npu_iova_driver = {
	.probe = npu_iova_probe,
	.remove = npu_iova_remove,
	.driver = {
		.name = "npu_iova",
		.of_match_table = npu_iova_of_match,
	},
};

int npu_iova_init(void)
{
	int ret;

	npu_drv_debug("started\n");

	ret = platform_driver_register(&npu_iova_driver);
	if (ret) {
		npu_drv_err("insmod devdrv npu_iova driver fail\n");
		return ret;
	}
	npu_drv_debug("succeed\n");

	return ret;
}

void npu_iova_exit(void)
{
	platform_driver_unregister(&npu_iova_driver);
}
