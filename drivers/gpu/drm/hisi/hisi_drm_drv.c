/* Copyright (c) 2018-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
/*lint -e86 -e574*/

#include "hisi_drm_drv.h"

#include <linux/of_platform.h>
#include <linux/component.h>
#include <linux/of_graph.h>
#include <linux/console.h>

#include <drm/drmP.h>
#include <drm/drm_gem_cma_helper.h>
#include <drm/drm_fb_cma_helper.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_auth.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
#include <drm/drm_gem_framebuffer_helper.h>
#endif

#include "hisi_drm_gem.h"
#include "hisi_drm.h"
#include "hisi_gem_dmabuf.h"
#include "hisi_drm_debug.h"
#include "hisi_dp_drv.h"

#ifdef CONFIG_DRM_FBDEV_EMULATION
static bool fbdev = true;
MODULE_PARM_DESC(fbdev, "Enable fbdev compat layer");
module_param(fbdev, bool, 0600);
#endif

static void hisi_fbdev_output_poll_changed(struct drm_device *dev)
{
	struct hisi_drm_private *priv = NULL;

	HISI_DRM_INFO("+");

	if (dev == NULL) {
		HISI_DRM_ERR("dev is nullptr!");
		return;
	}

	priv = dev->dev_private;

	if (!priv) {
		HISI_DRM_ERR("dev_private is nullptr!");
		return;
	}

	mutex_lock(&priv->suspend_mutex);
	if (priv->suspend) {
		HISI_DRM_INFO("in suspend");
		priv->need_poll_change = true;
		mutex_unlock(&priv->suspend_mutex);
		return;
	}

	if (!priv->fb_helper)
		HISI_DRM_ERR("priv->fb_helper is NULL");
	else
		drm_fb_helper_hotplug_event(priv->fb_helper);
	mutex_unlock(&priv->suspend_mutex);

	HISI_DRM_INFO("-");
}

static const struct drm_mode_config_funcs hisi_drm_mode_config_funcs = {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
	.fb_create = drm_fb_cma_create,
#else
	.fb_create = drm_gem_fb_create,
#endif
	.output_poll_changed = hisi_fbdev_output_poll_changed,
	.atomic_check = drm_atomic_helper_check,
	.atomic_commit = drm_atomic_helper_commit,
};

static struct drm_mode_config_helper_funcs hisi_drm_mode_config_helpers = {
	.atomic_commit_tail = drm_atomic_helper_commit_tail_rpm,
};

static const struct file_operations hisi_drm_fops = {
	.owner = THIS_MODULE,
	.open = drm_open,
	.release = drm_release,
	.unlocked_ioctl = drm_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = drm_compat_ioctl,
#endif
	.poll = drm_poll,
	.read = drm_read,
	.llseek = no_llseek,
	.mmap = hisi_drm_gem_mmap,
};

struct drm_ioctl_desc hisi_drm_ioctls[] = {
	DRM_IOCTL_DEF_DRV(HISI_GEM_CREATE, hisi_drm_gem_create_ioctl,
		DRM_AUTH | DRM_RENDER_ALLOW),
};

static const struct vm_operations_struct hisi_drm_gem_vm_ops = {
	.open = drm_gem_vm_open,
	.close = drm_gem_vm_close,
};


static struct drm_driver hisi_drm_driver = {
	.driver_features = DRIVER_GEM | DRIVER_MODESET | DRIVER_PRIME |
		DRIVER_ATOMIC | DRIVER_RENDER,
	.fops = &hisi_drm_fops,
	.gem_free_object_unlocked = hisi_drm_gem_free_object,
	.gem_vm_ops = &hisi_drm_gem_vm_ops,
	.dumb_create = hisi_drm_gem_dumb_create,
	.gem_prime_get_sg_table = hisi_drm_gem_prime_get_sg_table,
	.gem_prime_import_sg_table = hisi_drm_gem_prime_import_sg_table,
	.gem_prime_vmap = hisi_drm_gem_prime_vmap,
	.gem_prime_vunmap = hisi_drm_gem_prime_vunmap,
	.gem_prime_mmap = hisi_drm_gem_prime_mmap,

	.prime_handle_to_fd = drm_gem_prime_handle_to_fd,
	.prime_fd_to_handle = drm_gem_prime_fd_to_handle,
	.gem_prime_export = hisi_gem_dmabuf_prime_export,
	.gem_prime_import = hisi_gem_dmabuf_prime_import,

	.ioctls = hisi_drm_ioctls,
	.num_ioctls = ARRAY_SIZE(hisi_drm_ioctls),

	.name = "hisi",
	.desc = "Hisilicon DRM Driver",
	.date = "20190309",
	.major = 1,
	.minor = 0,
};

static void hisi_drm_mode_config_init(struct drm_device *drm_dev)
{
	drm_dev->mode_config.min_width = 0;
	drm_dev->mode_config.min_height = 0;

	drm_dev->mode_config.max_width = MAX_PIXEL_WIDE;
	drm_dev->mode_config.max_height = MAX_PIXEL_HEIGHT;

	drm_dev->mode_config.funcs = &hisi_drm_mode_config_funcs;
	drm_dev->mode_config.helper_private = &hisi_drm_mode_config_helpers;

	drm_dev->mode_config.allow_fb_modifiers = true;
}

static int priv_alloc(struct drm_device *drm_dev, struct hisi_drm_private **priv)
{
	*priv = devm_kzalloc(drm_dev->dev, sizeof(**priv), GFP_KERNEL);

	if (!(*priv)) {
		HISI_DRM_ERR("failed to alloc!");
		return -ENOMEM;
	}
	return 0;
}

static int hisi_drm_kms_init(struct drm_device *drm_dev)
{
	struct hisi_drm_private *priv = NULL;
	int ret;

	HISI_DRM_INFO("+");

	if (!drm_dev) {
		HISI_DRM_ERR("drm_dev is nullptr!");
		return -EINVAL;
	}

	ret = priv_alloc(drm_dev, &priv);
	if (ret)
		return ret;

	drm_dev->dev_private = priv;
	dev_set_drvdata(drm_dev->dev, drm_dev);
	mutex_init(&priv->suspend_mutex);
	priv->suspend = false;
	priv->need_poll_change = false;

	/* dev->mode_config initialization */
	drm_mode_config_init(drm_dev);
	hisi_drm_mode_config_init(drm_dev);

	/* bind and init sub drivers */
	ret = component_bind_all(drm_dev->dev, drm_dev);
	if (ret) {
		HISI_DRM_ERR("failed to bind all component!");
		goto err_bind_all;
	}

	/* vblank init */
	ret = drm_vblank_init(drm_dev, drm_dev->mode_config.num_crtc);
	if (ret) {
		HISI_DRM_ERR("failed to initialize vblank!");
		goto err_drm_vblank_init;
	}

	/* with irq_enabled = true, we can use the vblank feature. */
	drm_dev->irq_enabled = true;

#ifndef CONFIG_DRM_DSS_IOMMU
	arch_setup_dma_ops(drm_dev->dev, 0, 0, NULL, true);
#endif

	/* reset all the states of crtc/plane/encoder/connector */
	drm_mode_config_reset(drm_dev);

	/* init kms poll for handling hpd */
	drm_kms_helper_poll_init(drm_dev);

	priv->fb_helper = hisi_drm_fbdev_init(drm_dev);
	if (!priv->fb_helper) {
		HISI_DRM_ERR("failed to init fbdev!");
		goto err_fbdev_init;
	}

	/* force detection after connectors init */
	(void)drm_helper_hpd_irq_event(drm_dev);

	HISI_DRM_INFO("-");

	return 0;

err_fbdev_init:
err_drm_vblank_init:
	component_unbind_all(drm_dev->dev, drm_dev);

err_bind_all:
	drm_mode_config_cleanup(drm_dev);
	devm_kfree(drm_dev->dev, priv);
	drm_dev->dev_private = NULL;

	return ret;
}

static int hisi_drm_kms_cleanup(struct drm_device *drm_dev)
{
	struct hisi_drm_private *priv = NULL;

	HISI_DRM_INFO("+");

	if (!drm_dev) {
		HISI_DRM_ERR("drm_dev is NULL!");
		return -EINVAL;
	}

	priv = drm_dev->dev_private;
	if (!priv) {
		HISI_DRM_ERR("priv is NULL!");
		return -EINVAL;
	}

	if (priv->fb_helper) {
		hisi_drm_fbdev_fini(priv->fb_helper);
		priv->fb_helper = NULL;
	}

	drm_kms_helper_poll_fini(drm_dev);
	component_unbind_all(drm_dev->dev, drm_dev);
	drm_mode_config_cleanup(drm_dev);

	devm_kfree(drm_dev->dev, priv);
	drm_dev->dev_private = NULL;

	HISI_DRM_INFO("-");

	return 0;
}

static int hisi_drm_connectors_register(struct drm_device *dev)
{
	struct drm_connector *connector = NULL;
	struct drm_connector *failed_connector = NULL;
	struct drm_connector_list_iter conn_iter;
	int ret;

	mutex_lock(&dev->mode_config.mutex);
	drm_connector_list_iter_begin(dev, &conn_iter);

	drm_for_each_connector_iter(connector, &conn_iter) {
		ret = drm_connector_register(connector);
		if (ret) {
			failed_connector = connector;
			goto err_out;
		}
	}

	drm_connector_list_iter_end(&conn_iter);
	mutex_unlock(&dev->mode_config.mutex);

	return 0;

err_out:
	drm_connector_list_iter_begin(dev, &conn_iter);
	drm_for_each_connector_iter(connector, &conn_iter) {
		if (failed_connector == connector)
			break;
		drm_connector_unregister(connector);
	}
	drm_connector_list_iter_end(&conn_iter);
	mutex_unlock(&dev->mode_config.mutex);

	return ret;
}

static int hisi_drm_bind(struct device *dev)
{
	struct drm_driver *driver = &hisi_drm_driver;
	struct drm_device *drm_dev = NULL;
	int ret;

	HISI_DRM_INFO("+");

	if (dev == NULL) {
		HISI_DRM_ERR("dev is NULL!");
		return -EINVAL;
	}

	drm_dev = drm_dev_alloc(driver, dev);
	if (IS_ERR(drm_dev)) {
		HISI_DRM_ERR("faild to allocATE drm_device!");
		return PTR_ERR(drm_dev);
	}

	ret = hisi_drm_kms_init(drm_dev);
	if (ret) {
		HISI_DRM_ERR("failed to int drm kms! ret=%d!", ret);
		goto err_drm_kms_init;
	}

	ret = drm_dev_register(drm_dev, 0);
	if (ret) {
		HISI_DRM_ERR("failed to register drm_dev! ret=%d!", ret);
		goto err_drm_dev_register;
	}

	/* connectors should be registered after drm device register */
	ret = hisi_drm_connectors_register(drm_dev);
	if (ret) {
		HISI_DRM_ERR("failed to register drm_connectors! ret=%d!", ret);
		goto err_drm_connectors_register;
	}

	dp_wakeup();

	HISI_DRM_INFO("Initialized %s %d.%d.%d %s on minor %d",
		driver->name, driver->major, driver->minor, driver->patchlevel,
		driver->date, drm_dev->primary->index);

	HISI_DRM_INFO("-");

	return 0;

err_drm_connectors_register:
	drm_dev_unregister(drm_dev);
err_drm_dev_register:
	hisi_drm_kms_cleanup(drm_dev);
err_drm_kms_init:
	drm_dev_unref(drm_dev);

	return ret;
}

static void hisi_drm_unbind(struct device *dev)
{
	HISI_DRM_INFO("+");

	if (dev == NULL) {
		HISI_DRM_ERR("dev is nullptr!");
		return;
	}

	drm_put_dev(dev_get_drvdata(dev));

	HISI_DRM_INFO("-");
}

static const struct component_master_ops hisi_drm_ops = {
	.bind = hisi_drm_bind,
	.unbind = hisi_drm_unbind,
};

static struct platform_driver *hisi_drm_drivers[] = {
	&hisi_dpe_driver,
	&hisi_mipi_dsi_driver,
	&hisi_dp_driver,
};

static int of_dev_node_match(struct device *dev, void *data)
{
	if (dev == NULL) {
		HISI_DRM_ERR("dev is nullptr!");
		return -EINVAL;
	}

	return dev->of_node == data;
}

static int compare_dev(struct device *dev, void *data)
{
	return dev == (struct device *)data;
}

static struct component_match *hisi_drm_match_add(struct device *dev)
{
	struct component_match *match = NULL;
	int i;

	if (dev == NULL) {
		HISI_DRM_ERR("dev is nullptr!");
		return NULL;
	}

	for (i = 0; i < ARRAY_SIZE(hisi_drm_drivers); ++i) {
		struct platform_driver *driver = hisi_drm_drivers[i];
		struct device *p = NULL;
		struct device *d = NULL;
		struct device_node *node = NULL;

		if (!driver) {
			HISI_DRM_ERR("driver[%d] is NULL!", i);
			continue;
		}

		node = of_find_compatible_node(NULL, NULL,
			driver->driver.of_match_table->compatible);
		while ((d = bus_find_device(&platform_bus_type, p,
			(void *)node, of_dev_node_match))) {
			put_device(p);
			component_match_add(dev, &match, compare_dev, d);
			p = d;
		}

		put_device(p);
	}

	return match ?: ERR_PTR(-ENODEV);
}

static int hisi_drm_platform_probe(struct platform_device *pdev)
{
	struct component_match *match = NULL;
	int ret;

	HISI_DRM_INFO("+");

	if (pdev == NULL) {
		HISI_DRM_ERR("pdev is nullptr!");
		return -EINVAL;
	}

	match = hisi_drm_match_add(&pdev->dev);
	if (IS_ERR(match)) {
		HISI_DRM_ERR("1 failed to match! ret=%d!", IS_ERR(match));
		return PTR_ERR(match);
	}

	ret = component_master_add_with_match(&pdev->dev,
		&hisi_drm_ops, match);
	if (ret) {
		HISI_DRM_ERR("2 failed to mach! ret=%d!", ret);
		return ret;
	}

	HISI_DRM_INFO("-");

	return ret;
}

static int hisi_drm_platform_remove(struct platform_device *pdev)
{
	HISI_DRM_INFO("+");

	if (pdev == NULL) {
		HISI_DRM_ERR("pdev is nullptr!");
		return -EINVAL;
	}

	component_master_del(&pdev->dev, &hisi_drm_ops);

	HISI_DRM_INFO("-");

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static void hisi_drm_fb_suspend(struct drm_device *drm)
{
	struct hisi_drm_private *priv = drm->dev_private;

	if (priv == NULL) {
		HISI_DRM_ERR("priv is nullptr!");
		return;
	}

	console_lock();
	drm_fb_helper_set_suspend(priv->fb_helper, 1);
	console_unlock();
}

static void hisi_drm_fb_resume(struct drm_device *drm)
{
	struct hisi_drm_private *priv = drm->dev_private;

	if (priv == NULL) {
		HISI_DRM_ERR("priv is nullptr!");
		return;
	}

	console_lock();
	drm_fb_helper_set_suspend(priv->fb_helper, 0);
	console_unlock();
}

static int hisi_drm_sys_suspend(struct device *dev)
{
	int ret = 0;
	struct drm_device *drm = NULL;
	struct hisi_drm_private *priv = NULL;

	HISI_DRM_INFO("+");
	if (!dev) {
		HISI_DRM_INFO("### dev is nullptr");
		return -EINVAL;
	}
	drm = dev_get_drvdata(dev);
	if (drm == NULL) {
		HISI_DRM_ERR("drm is nullptr!");
		return -EINVAL;
	}

	priv = drm->dev_private;
	if (priv == NULL) {
		HISI_DRM_ERR("priv is nullptr!");
		return -EINVAL;
	}
	hisi_wait_dp_off();

	drm_kms_helper_poll_disable(drm);
	hisi_drm_fb_suspend(drm);
	HISI_DRM_INFO("poll_disable and suspend");

	mutex_lock(&priv->suspend_mutex);
	priv->suspend = true;
	priv->suspend_state = drm_atomic_helper_suspend(drm);

	if (IS_ERR(priv->suspend_state)) {
		ret = PTR_ERR(priv->suspend_state);
		drm_kms_helper_poll_enable(drm);
		HISI_DRM_ERR("failed to suspend! ret=%d!", ret);
		priv->suspend = false;
	}

	mutex_unlock(&priv->suspend_mutex);

	HISI_DRM_INFO("-");
	return ret;
}

static int hisi_drm_sys_resume(struct device *dev)
{
	struct drm_device *drm = NULL;
	struct hisi_drm_private *priv = NULL;

	HISI_DRM_INFO("+");

	drm = dev_get_drvdata(dev);
	if (drm == NULL) {
		HISI_DRM_ERR("drm is nullptr!");
		return -EINVAL;
	}

	priv = drm->dev_private;
	if (priv == NULL) {
		HISI_DRM_ERR("priv is nullptr!");
		return -EINVAL;
	}

	mutex_lock(&priv->suspend_mutex);
	drm_atomic_helper_resume(drm, priv->suspend_state);
	priv->suspend = false;

	if (priv->need_poll_change) {
		if (!priv->fb_helper)
			HISI_DRM_ERR("priv->fb_helper is NULL");
		else
			drm_fb_helper_hotplug_event(priv->fb_helper);

		priv->need_poll_change = false;
	}
	mutex_unlock(&priv->suspend_mutex);

	hisi_drm_fb_resume(drm);
	drm_kms_helper_poll_enable(drm);

	HISI_DRM_INFO("-");

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(hisi_drm_pm_ops,
	hisi_drm_sys_suspend,
	hisi_drm_sys_resume);

static struct platform_driver hisi_drm_platform_driver = {
	.probe = hisi_drm_platform_probe,
	.remove = hisi_drm_platform_remove,
	.driver = {
		.name = "hisi-drm",
		.pm = &hisi_drm_pm_ops,
	},
};

static inline int match_dev_name(struct device *dev, void *data)
{
	if (dev == NULL || data == NULL) {
		HISI_DRM_ERR("invalid input!");
		return -EINVAL;
	}

	return !strcmp(dev_name(dev), data);
}

static void hisi_drm_unregister_devices(void)
{
	struct device *dev = NULL;

	while ((dev = bus_find_device(&platform_bus_type, NULL,
		(void *)hisi_drm_platform_driver.driver.name,
			match_dev_name))) {
		put_device(dev);
		platform_device_unregister(to_platform_device(dev));
	}
}

static void hisi_drm_unregister_drivers(void)
{
	int i;

	for (i = ARRAY_SIZE(hisi_drm_drivers) - 1; i >= 0; i--) {
		struct platform_driver *driver = hisi_drm_drivers[i];

		if (!driver)
			continue;

		platform_driver_unregister(driver);
	}

	platform_driver_unregister(&hisi_drm_platform_driver);
}

static int hisi_drm_register_devices(void)
{
	struct platform_device *pdev = NULL;

	pdev = platform_device_register_simple(
		hisi_drm_platform_driver.driver.name, -1, NULL, 0);
	if (IS_ERR(pdev)) {
		HISI_DRM_ERR("failed to register device[%s]!",
			hisi_drm_platform_driver.driver.name);
		return PTR_ERR(pdev);
	}

	return 0;
}

static int hisi_drm_register_drivers(void)
{
	int ret;
	int i;

	ret = platform_driver_register(&hisi_drm_platform_driver);
	if (ret) {
		HISI_DRM_ERR("failed to register drm platform driver!");
		return ret;
	}

	for (i = 0; i < ARRAY_SIZE(hisi_drm_drivers); i++) {
		struct platform_driver *driver = hisi_drm_drivers[i];

		if (!driver)
			continue;

		ret = platform_driver_register(driver);
		if (ret) {
			HISI_DRM_ERR("failed to register drm drivers [%d]!", i);
			goto err_out;
		}
	}

	return 0;

err_out:
	hisi_drm_unregister_drivers();
	return ret;
}

static int hisi_drm_init(void)
{
	int ret;

	HISI_DRM_INFO("+");

	ret = hisi_drm_register_devices();
	if (ret) {
		HISI_DRM_ERR("failed to register devices!");
		return ret;
	}

	ret = hisi_drm_register_drivers();
	if (ret)
		goto err_out;

	HISI_DRM_INFO("-");

	return ret;

err_out:
	hisi_drm_unregister_devices();
	return ret;
}

static void hisi_drm_exit(void)
{
	HISI_DRM_INFO("+");
	hisi_drm_unregister_drivers();
	hisi_drm_unregister_devices();
	HISI_DRM_INFO("-");
}

module_init(hisi_drm_init);
module_exit(hisi_drm_exit);

MODULE_DESCRIPTION("Hisilicon DRM master driver");
MODULE_LICENSE("GPL v2");

/*lint +e86 +e574*/
