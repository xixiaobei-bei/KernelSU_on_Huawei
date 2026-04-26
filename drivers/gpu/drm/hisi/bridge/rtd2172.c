#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_edid.h>
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_mipi_dsi.h>
#include <linux/component.h>
#include <linux/pm_runtime.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/of_gpio.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/regmap.h>
#include <linux/semaphore.h>
#include <linux/backlight.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include "linux/mfd/hisi_pmic.h"
#include "rtd2172.h"

static int rtd2172_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = NULL;
	struct rtd2172_bridge *pdata = NULL;
	struct device *pdev = NULL;
	int ret = 0;
	int error = 0;

	HISI_DRM_INFO("%s enter\n", __func__);

	if (!client) {
		HISI_DRM_ERR("rtd2172 client is nullptr!");
		return -EINVAL;
	}

	pdev = &client->dev;

	if (!pdev->of_node) {
		HISI_DRM_ERR("dev of_node is nullptr!");
		return -EINVAL;
	}

	adapter = client->adapter;
	if (!adapter) {
		HISI_DRM_ERR("adapter is nullptr!");
		return -EINVAL;
	}

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		HISI_DRM_ERR("failed to i2c_check_functionality!");
		return -EOPNOTSUPP;
	}

	pdata = devm_kzalloc(pdev, sizeof(struct rtd2172_bridge), GFP_KERNEL);
	if (!pdata) {
		HISI_DRM_ERR("failed to alloc i2c!");
		return -ENOMEM;
	}

	dev_set_drvdata(pdev, pdata);
	pdata->dev = pdev;

	pdata->client = client;
	i2c_set_clientdata(client, pdata);

	/* get rtd2172 regulator*/
	pdata->ec_hdmi_vdd_on = devm_regulator_get_optional(&client->dev, "EC_HDMI_VDD_ON");
	if (IS_ERR(pdata->ec_hdmi_vdd_on)) {
		error = PTR_ERR(pdata->ec_hdmi_vdd_on);
		if (error == -EPROBE_DEFER)
			ret = -EPROBE_DEFER;
		else
			ret = -EINVAL;
		HISI_DRM_ERR("Failed to get 'ec_hdmi_vdd_on' regulator: %d\n",error);
		goto err_out;
	}

	pdata->ec_1v1_en = devm_regulator_get_optional(&client->dev, "EC_1V1_EN");
	if (IS_ERR(pdata->ec_1v1_en)) {
		error = PTR_ERR(pdata->ec_1v1_en);
		if (error == -EPROBE_DEFER)
			ret = -EPROBE_DEFER;
		else
			ret = -EINVAL;
		HISI_DRM_ERR("Failed to get 'ec_1v1_en' regulator: %d\n",error);
		goto err_out;
	}
	/* power up */
	error = regulator_enable(pdata->ec_hdmi_vdd_on);
	if (error) {
		dev_err(&pdata->client->dev,"failed to enable ec_hdmi_vdd_on regulator: %d\n",error);
		return -EINVAL;
	}

	error = regulator_enable(pdata->ec_1v1_en);
	if (error) {
		dev_err(&pdata->client->dev,"failed to enable ec_1v1_en regulator: %d\n",error);
		return -EINVAL;
	}

	HISI_DRM_INFO("rtd2172_i2c_probe OK\n");
	return 0;
err_out:
	return ret;
}

static int rtd2172_i2c_remove(struct i2c_client *client)
{
	HISI_DRM_INFO("%s +\n", __func__);

	if (!client) {
		HISI_DRM_ERR("client is nullptr!");
		return -EINVAL;
	}
	HISI_DRM_INFO("%s -.\n", __func__);
	return 0;
}

static int  rtd2172_resume(struct device *dev)
{
	int ret = 0;
	int error = 0;
	struct rtd2172_bridge *pdata = dev_get_drvdata(dev);
	HISI_DRM_INFO("%s +", __func__);

	error = regulator_enable(pdata->ec_hdmi_vdd_on);
	if (error)
		dev_err(&pdata->client->dev,"failed to disable ec_hdmi_vdd_on regulator: %d\n",error);

	error = regulator_enable(pdata->ec_1v1_en);
	if (error)
		dev_err(&pdata->client->dev,"failed to disable ec_1v1_en regulator: %d\n",error);

	HISI_DRM_INFO("%s -", __func__);
	return ret;
}

static int  rtd2172_suspend(struct device *dev)
{
	int ret = 0;
	struct rtd2172_bridge *pdata = dev_get_drvdata(dev);
	int error = 0;

	HISI_DRM_INFO("%s +", __func__);
	error = regulator_disable(pdata->ec_1v1_en);
	if (error)
		dev_err(&pdata->client->dev,"failed to disable ec_1v1_en regulator: %d\n",error);
	mdelay(20);
	error = regulator_disable(pdata->ec_hdmi_vdd_on);
	if (error)
		dev_err(&pdata->client->dev,"failed to disable ec_hdmi_vdd_on regulator: %d\n",error);

	HISI_DRM_INFO("%s -", __func__);
	return ret;
}

static SIMPLE_DEV_PM_OPS(rtd2172_pm_ops, rtd2172_suspend, rtd2172_resume);

static const struct of_device_id rtd2172_i2c_of_match[] = {
	{
		.compatible = "rtd,rtd2172",
	},
	{}
};

MODULE_DEVICE_TABLE(of, rtd2172_i2c_of_match);

static const struct i2c_device_id rtd2172_i2c_id_table[] = {
	{ "rtd2172", 0 },
	{}
};

MODULE_DEVICE_TABLE(i2c, rtd2172_i2c_id_table);

static struct i2c_driver rtd2172_i2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "rtd2172",
		.of_match_table = of_match_ptr(rtd2172_i2c_of_match),
		.pm = &rtd2172_pm_ops,
	},
	.probe = rtd2172_i2c_probe,
	.remove = rtd2172_i2c_remove,
	.id_table = rtd2172_i2c_id_table,
};

static int __init rtd2172_init(void)
{
	int ret;

	HISI_DRM_INFO("%s +\n", __func__);

	ret = i2c_add_driver(&rtd2172_i2c_driver);
	if (ret) {
		HISI_DRM_ERR("failed to i2c_add_driver, ret=%d!\n", ret);
		return ret;
	}
	HISI_DRM_INFO("%s -\n", __func__);
	return ret;
}

module_init(rtd2172_init);

static void __exit rtd2172_exit(void)
{
	HISI_DRM_INFO("%s+\n", __func__);
	i2c_del_driver(&rtd2172_i2c_driver);
	HISI_DRM_INFO("%s-\n", __func__);
}

module_exit(rtd2172_exit);

MODULE_LICENSE("GPL");


