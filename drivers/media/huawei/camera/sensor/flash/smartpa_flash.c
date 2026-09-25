/*
 * smartpa_flash.c
 *
 * the smartpa flash driver file
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/consumer.h>
#include <linux/power/hisi_hi6521_charger_power.h>
#include <linux/bitops.h>
#include <sound/hisi_pa.h>
#include <securec.h>
#include "hw_flash.h"
#include "hisi_smartpa_regulator.h"

#define FLASH_LED_MAX 50
#define TORCH_LED_MAX 18
#define FLASH_LEV_MAX 44
#define TORCH_LEV_MAX 14
#define FLASH_TIMEOUT_MS 600

#define SMARTPA_FLASH_MAX_CUR 1200 // 1200ma
#define SMARTPA_TORCH_MAX_CUR 300 // 300ma
#define SMARTPA_TORCH_DEFAULT_CUR 140 // 140ma
#define SMARTPA_TORCH_MAX_RT_CUR 180 // 180mA
#define SMARTPA_MA_TO_UM 1000
#define SMARTPA_FLASH_VOLTAGE 4600000 // 4.6v

#define SMARTPA_BOOST_REGULATOR "pvdd-classd"
#define SMARTPA_FLASH_MODE_REGULATOR "flash-led"
#define SMARTPA_TORCH_MODE_REGULATOR "torch-led"
#define CLE_IRQ 1
#define REL_IRQ 0
#define SMARTPA_CHECK_PARAM_NUM 2
#define BASE_DEC 10
#define BASE_HEX 16

struct hw_smartpa_private_data_t {
	unsigned int flash_led[FLASH_LED_MAX];
	unsigned int torch_led[TORCH_LED_MAX];
	unsigned int flash_led_num;
	unsigned int torch_led_num;
	struct regulator *flash_inter_ldo;
	struct regulator *flash_mode_ldo;
	struct regulator *torch_mode_ldo;
	unsigned int status;
};

static int flash_arry[FLASH_LEV_MAX] = { 125, 150, 175, 200, 225, 250, 275, 300,
	325, 350, 375, 400, 425, 450, 475, 500, 525, 550,
	575, 600, 625, 650, 675, 700, 725, 750, 775, 800,
	825, 850, 875, 900, 925, 950, 975, 1000, 1025,
	1050, 1075, 1100, 1125, 1150, 1175, 1200 };
static int torch_arry[TORCH_LEV_MAX] = { 40, 60, 80, 100, 120,
	140, 160, 180, 200, 220,
	240, 260, 280, 300 };

/* Internal varible define */
static struct hw_smartpa_private_data_t hw_smartpa_pdata;
static struct hw_flash_ctrl_t hw_smartpa_ctrl;

static const char *smartpa_flash_tag = "smartpa_flash";

DEFINE_HISI_FLASH_MUTEX(smartpa);

#ifdef CAMERA_FLASH_FACTORY_TEST
extern int register_camerafs_attr(struct device_attribute *attr);
#endif

static int hw_smartpa_find_match_flash_current(int cur_flash)
{
	int cur_level = 0;
	int i;

	cam_info("%s ernter cur_flash %d.\n", __func__, cur_flash);
	if (cur_flash <= 0) {
		cam_err("%s current set is error", __func__);
		return -EINVAL;
	}

	if (cur_flash >= SMARTPA_FLASH_MAX_CUR) {
		cam_warn("%s current set is %d", __func__, cur_flash);
		return SMARTPA_FLASH_MAX_CUR;
	}

	for (i = 1; i < FLASH_LEV_MAX - 1; i++) {
		if (cur_flash <= flash_arry[i]) {
			cam_info("%s  i %d.\n", __func__, i);
			break;
		}
	}

	if (cur_flash - flash_arry[i - 1] < flash_arry[i] - cur_flash) {
		cur_level =  flash_arry[i - 1];
	} else {
		cur_level = flash_arry[i];
	}

	return cur_level;
}

static int hw_smartpa_find_match_torch_current(int cur_torch)
{
	int cur_level = 0;
	int i;

	cam_info("%s ernter cur_flash %d.\n", __func__, cur_torch);
	if (cur_torch <= 0) {
		cam_err("%s current set is error", __func__);
		return -EINVAL;
	}

	if (cur_torch >= SMARTPA_TORCH_MAX_CUR) {
		cam_warn("%s current set is %d", __func__, cur_torch);
		return SMARTPA_TORCH_MAX_CUR;
	}

	for (i = 1; i < TORCH_LEV_MAX - 1; i++) {
		if (cur_torch <= torch_arry[i]) {
			cam_info("%s  i %d.\n", __func__, i);
			break;
		}
	}

	if (cur_torch - torch_arry[i - 1] < torch_arry[i] - cur_torch) {
		cur_level =  torch_arry[i - 1];
	} else {
		cur_level = torch_arry[i];
	}

	return cur_level;
}

static int hw_smartpa_init(struct hw_flash_ctrl_t *flash_ctrl)
{
	if (!flash_ctrl) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -1;
	}

	struct hw_smartpa_private_data_t *pdata =
		(struct hw_smartpa_private_data_t *)flash_ctrl->pdata;
	if (!pdata) {
		cam_err("%s-%s pdata is NULL", smartpa_flash_tag, __func__);
		return -1;
	}

	pdata->status = STANDBY_MODE;

	return 0;
}

static int hw_smartpa_exit(struct hw_flash_ctrl_t *flash_ctrl)
{
	if (!flash_ctrl) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -1;
	}

	return 0;
}

static int hw_smartpa_flash_mode(struct hw_flash_ctrl_t *flash_ctrl, int data)
{
	struct hw_smartpa_private_data_t *pdata = NULL;
	int ret;
	int cur_flash = data;

	data = hw_smartpa_find_match_flash_current(cur_flash);
	cam_info("%s-%s data=%d", smartpa_flash_tag, __func__, data);

	if (!flash_ctrl) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -1;
	}

	pdata = (struct hw_smartpa_private_data_t *)flash_ctrl->pdata;

	if (data <= 0) {
		cam_err("set the flash_lum_level: %d", data);
		return -1;
	}

	if (data > SMARTPA_FLASH_MAX_CUR) {
		cam_warn("set the max flash_lum_level: %d", data);
		data = SMARTPA_FLASH_MAX_CUR;
	}

	if ((!pdata->flash_inter_ldo) || (!pdata->flash_mode_ldo)) {
		cam_err("%s regulator is NULL", __func__);
		return -1;
	}

	/* if flash has already on do nothing */
	if (BIT(FLASH_MODE) & pdata->status) {
		cam_info("%s already in flash mode, do nothing", __func__);
		return 0;
	}

	hisi_pa_modify_gain();

	ret = hisi_smartpa_flash_init();
	if (ret < 0) {
		cam_err("%s hisi_smartpa_flash_init fail ret = %d", __func__, ret);
		goto err_out;
	}
	ret = hisi_smartpa_flash_irq_enable(CLE_IRQ);
	if (ret < 0) {
		cam_err("%s hisi_smartpa_flash_irq_enable fail ret = %d", __func__, ret);
		goto err_out;
	}

	ret = regulator_set_voltage(pdata->flash_inter_ldo, SMARTPA_FLASH_VOLTAGE, SMARTPA_FLASH_VOLTAGE);
	if (ret < 0) {
		cam_err("%s regulator_set_voltage  fail ret = %d", __func__, ret);
		goto err_out;
	}

	// the smartpa current unit is ua
	data = data * SMARTPA_MA_TO_UM;

	ret = regulator_set_current_limit(pdata->flash_mode_ldo, data, data);
	if (ret < 0) {
		cam_err("%s regulator_set_current_limit  fail ret = %d current is %d", __func__, ret, data);
		goto err_out;
	}

	ret = regulator_enable(pdata->flash_inter_ldo);
	if (ret < 0) {
		cam_err("%s regulator_enable flash_inter_ldo fail ret = %d", __func__, ret);
		goto err_out;
	}
	udelay(500);

	ret = regulator_enable(pdata->flash_mode_ldo);
	if (ret < 0) {
		cam_err("%s regulator_enable torch_mode_ldo fail ret = %d", __func__, ret);
		if (regulator_disable(pdata->flash_inter_ldo) != 0)
			cam_notice("Failed: flash_inter_ldo regulator_disable");
		goto err_out;
	}
	pdata->status |= BIT(FLASH_MODE);
	return 0;

err_out:
	hisi_pa_restore_gain();
	return ret;

}

static int hw_smartpa_torch_mode(struct hw_flash_ctrl_t *flash_ctrl, int data)
{
	struct hw_smartpa_private_data_t *pdata = NULL;
	int ret;
	int cur_torch = data;

	data = hw_smartpa_find_match_torch_current(cur_torch);
	cam_info("%s-%s data=%d", smartpa_flash_tag, __func__, data);

	if (!flash_ctrl) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -1;
	}

	pdata = (struct hw_smartpa_private_data_t *)flash_ctrl->pdata;

	if (data <= 0) {
		cam_err("set the torch_lum_level: %d", data);
		return -1;
	}

	if (data > SMARTPA_TORCH_MAX_CUR) {
		cam_warn("set the max torch_lum_level: %d", data);
		data = SMARTPA_TORCH_MAX_CUR;
	}

	if ((!pdata->flash_inter_ldo) || (!pdata->torch_mode_ldo)) {
		cam_err("%s regulator is NULL", __func__);
		return -1;
	}
	/* if flash has already on do nothing */
	if (BIT(TORCH_MODE) & pdata->status) {
		cam_info("%s already in torch mode, do nothing", __func__);
		return 0;
	}

	udelay(500);

	ret = hisi_smartpa_flash_irq_enable(CLE_IRQ); // torch irq on
	if (ret < 0) {
		cam_err("%s hisi_smartpa_flash_irq_enable fail ret = %d", __func__, ret);
		return ret;
	}

	// the smartpa current unit is ua
	data = data * SMARTPA_MA_TO_UM;

	ret = regulator_set_current_limit(pdata->torch_mode_ldo, data, data);
	if (ret < 0) {
		cam_err("%s regulator_set_current_limit  fail ret = %d current is %d", __func__, ret, data);
		return ret;
	}

	ret = regulator_enable(pdata->torch_mode_ldo);
	if (ret < 0) {
		cam_err("%s regulator_enable torch_mode_ldo fail ret = %d", __func__, ret);
		if (regulator_disable(pdata->flash_inter_ldo) != 0)
			cam_notice("Failed: flash_inter_ldo regulator_disable");
		return ret;
	}

	pdata->status |= BIT(TORCH_MODE);
	return 0;
}

static int hw_smartpa_on(struct hw_flash_ctrl_t *flash_ctrl, void *data)
{
	struct hw_flash_cfg_data *cdata = (struct hw_flash_cfg_data *)data;
	int rc = -1;

	if ((!flash_ctrl) || (!cdata)) {
		cam_err("%s-%s flash_ctrl or cdata is NULL",
			smartpa_flash_tag, __func__);
		return rc;
	}

	cam_info("%s-%s mode = %d, level = %d", __func__, smartpa_flash_tag,
		cdata->mode, cdata->data);

	mutex_lock(flash_ctrl->hw_flash_mutex);

	if (cdata->mode == FLASH_MODE || cdata->mode == FLASH_STROBE_MODE)
		rc = hw_smartpa_flash_mode(flash_ctrl, cdata->data);
	else
		rc = hw_smartpa_torch_mode(flash_ctrl, cdata->data);

	flash_ctrl->state.mode = cdata->mode;
	flash_ctrl->state.data = cdata->data;
	mutex_unlock(flash_ctrl->hw_flash_mutex);

	return rc;
}

static int hw_smartpa_off(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_smartpa_private_data_t *pdata = NULL;
	int ret = 0;

	cam_info("%s-%s enter.\n", smartpa_flash_tag, __func__);

	if (!flash_ctrl) {
		cam_err("%s-%s flash_ctrl is NULL", smartpa_flash_tag, __func__);
		return -1;
	}

	if (flash_ctrl->state.mode == STANDBY_MODE) {
		cam_notice("%s-%s flash led has been powered off",
			smartpa_flash_tag, __func__);
		return 0;
	}

	pdata = (struct hw_smartpa_private_data_t *)flash_ctrl->pdata;

	if ((!pdata->flash_inter_ldo)
		|| (!pdata->flash_mode_ldo)
		|| (!pdata->torch_mode_ldo)) {
		cam_err("%s regulator is NULL", __func__);
		return -1;
	}

	mutex_lock(flash_ctrl->hw_flash_mutex);

	if (BIT(TORCH_MODE) & pdata->status) {
		if (regulator_disable(pdata->torch_mode_ldo) != 0)
			cam_notice("Failed: torch_mode_ldo regulator_disable");
		pdata->status &= ~BIT(TORCH_MODE);

		ret = hisi_smartpa_flash_irq_enable(REL_IRQ); // torch irq off
		if (ret < 0) {
			cam_err("%s hisi_smartpa_flash_irq_enable fail ret = %d", __func__, ret);
			mutex_unlock(flash_ctrl->hw_flash_mutex);
			return ret;
		}
	}

	if (BIT(FLASH_MODE) & pdata->status) {
		if (regulator_disable(pdata->flash_mode_ldo) != 0)
			cam_notice("Failed: flash_mode_ldo regulator_disable");
		if (regulator_disable(pdata->flash_inter_ldo) != 0)
			cam_notice("Failed: flash_inter_ldo regulator_disable");
		pdata->status &= ~BIT(FLASH_MODE);
		ret = hisi_smartpa_flash_irq_enable(REL_IRQ);
		if (ret < 0) {
			cam_err("%s hisi_smartpa_flash_irq_enable fail ret = %d", __func__, ret);
			mutex_unlock(flash_ctrl->hw_flash_mutex);
			return ret;
		}
	}
	hisi_pa_restore_gain();

	if (ret == 0) {
		flash_ctrl->state.mode = STANDBY_MODE;
		flash_ctrl->state.data = 0;
	}

	mutex_unlock(flash_ctrl->hw_flash_mutex);

	return ret;
}

static int hw_smartpa_get_dt_data(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_smartpa_private_data_t *pdata = NULL;
	struct device_node *dev_node = NULL;
	unsigned int i;
	int rc = -1;

	cam_info("%s-%s enter", smartpa_flash_tag, __func__);

	if (!flash_ctrl) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return rc;
	}

	pdata = (struct hw_smartpa_private_data_t *)flash_ctrl->pdata;
	dev_node = flash_ctrl->dev->of_node;

	rc = of_property_read_u32(dev_node, "huawei,flash_led_num",
		&pdata->flash_led_num);
	if (rc < 0) {
		cam_err("%s failed %d", __func__, __LINE__);
		return rc;
	}

	rc = of_property_read_u32(dev_node, "huawei,torch_led_num",
		&pdata->torch_led_num);
	if (rc < 0) {
		cam_err("%s failed %d", __func__, __LINE__);
		return rc;
	}

	rc = of_property_read_u32_array(dev_node, "huawei,flash_led",
		pdata->flash_led, pdata->flash_led_num);
	if (rc < 0) {
		cam_err("%s failed line %d", __func__, __LINE__);
		return rc;
	} else {
		for (i = 0; i < pdata->flash_led_num; i++)
			cam_debug("%s flash_led[%d] = 0x%x", __func__, i,
				pdata->flash_led[i]);
	}

	rc = of_property_read_u32_array(dev_node, "huawei,torch_led",
		pdata->torch_led, pdata->torch_led_num);
	if (rc < 0) {
		cam_err("%s failed line %d", __func__, __LINE__);
		return rc;
	} else {
		for (i = 0; i < pdata->torch_led_num; i++)
			cam_debug("%s torch_led[%d] = 0x%x", __func__, i,
				pdata->torch_led[i]);
	}

	return rc;
}

#ifdef CAMERA_FLASH_FACTORY_TEST
static ssize_t hw_smartpa_lightness_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int rc = 0;
	int ret;

	cam_info("%s-%s enter", smartpa_flash_tag, __func__);

	if (!buf) {
		cam_err("%s buf is NULL", __func__);
		return rc;
	}

	ret = snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE,
		MAX_ATTRIBUTE_BUFFER_SIZE - 1, "mode = %d, data = %d",
		hw_smartpa_ctrl.state.mode, hw_smartpa_ctrl.state.data);
	if (ret < 0)
		cam_err("%s snprintf_s fail", __func__);

	rc = strlen(buf) + 1;

	return rc;
}

static int hw_smartpa_param_check(char *buf, unsigned long *param, int num_of_par)
{
	char *token = NULL;
	int base;
	int cnt;

	cam_info("%s-%s enter", smartpa_flash_tag, __func__);

	if (!buf) {
		cam_err("%s buf is NULL", __func__);
		return -EINVAL;
	}

	token = strsep(&buf, " ");

	for (cnt = 0; cnt < num_of_par; cnt++) {
		if (token != NULL) {
			if ((token[1] == 'x') || (token[1] == 'X'))
				base = BASE_HEX;
			else
				base = BASE_DEC;

			if (kstrtoul(token, base, &param[cnt]) != 0)
				return -EINVAL;

			token = strsep(&buf, " ");
		} else {
			return -EINVAL;
		}
	}

	return 0;
}

static ssize_t hw_smartpa_lightness_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hw_flash_cfg_data cdata = {0};
	unsigned long param[SMARTPA_CHECK_PARAM_NUM] = {0};
	int rc = 0;
	int flash_id;
	struct hw_smartpa_private_data_t *pdata =
		(struct hw_smartpa_private_data_t *)(hw_smartpa_ctrl.pdata);

	cam_info("%s-%s enter", smartpa_flash_tag, __func__);

	if (!buf) {
		cam_err("%s buf is NULL", __func__);
		return rc;
	}

	rc = hw_smartpa_param_check((char *)buf, param, SMARTPA_CHECK_PARAM_NUM);
	if (rc < 0) {
		cam_err("%s failed to check param", __func__);
		return rc;
	}

	flash_id = (int)param[0];
	cdata.mode = (int)param[1];

	cam_info("%s flash_id = %d, cdata.mode = %d", __func__, flash_id,
		cdata.mode);
	if (flash_id != 1) {
		cam_err("%s smartpa wrong flash_id = %d", __func__, flash_id);
		return -1;
	}

	if (cdata.mode == STANDBY_MODE) {
		rc = hw_smartpa_off(&hw_smartpa_ctrl);
		if (rc < 0) {
			cam_err("%s smartpa flash off error", __func__);
			return rc;
		}
	} else if (cdata.mode == TORCH_MODE) {
		cdata.data = SMARTPA_TORCH_MAX_RT_CUR;
		cam_info("%s mode = %d, max_current = %d", __func__, cdata.mode,
			cdata.data);

		rc = hw_smartpa_on(&hw_smartpa_ctrl, &cdata);
		if (rc < 0) {
			cam_err("%s smartpa flash on error", __func__);
			return rc;
		}
	} else {
		cam_err("%s smartpa wrong mode = %d", __func__, cdata.mode);
		return -1;
	}

	return count;
}
#endif

static ssize_t hw_smartpa_flash_mask_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int rc = 0;
	int ret;

	cam_info("%s-%s enter", smartpa_flash_tag, __func__);

	if (!buf) {
		cam_err("%s buf is NULL", __func__);
		return rc;
	}

	ret = snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE,
		MAX_ATTRIBUTE_BUFFER_SIZE - 1, "flash_mask_disabled = %d",
		hw_smartpa_ctrl.flash_mask_enable);
	if (ret < 0)
		cam_err("%s snprintf_s fail", __func__);

	rc = strlen(buf) + 1;

	return rc;
}

static ssize_t hw_smartpa_flash_mask_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int rc = -1;
	if (!buf) {
		cam_err("%s buf is NULL", __func__);
		return rc;
	}

	if (buf[0] == '0')
		hw_smartpa_ctrl.flash_mask_enable = false;
	else
		hw_smartpa_ctrl.flash_mask_enable = true;

	cam_info("%s flash_mask_enable = %d", __func__,
		hw_smartpa_ctrl.flash_mask_enable);
	return count;
}

static void hw_smartpa_torch_brightness_set(struct led_classdev *cdev,
	enum led_brightness brightness)
{
	struct hw_flash_cfg_data cdata = {0};
	int rc;
	unsigned int led_bright = brightness;

	cam_info("%s-%s enter", smartpa_flash_tag, __func__);

	if (led_bright == STANDBY_MODE) {
		rc = hw_smartpa_off(&hw_smartpa_ctrl);
		if (rc < 0) {
			cam_err("%s smartpa off error", __func__);
			return;
		}
	} else {
		cdata.mode = TORCH_MODE;
		cdata.data = SMARTPA_TORCH_DEFAULT_CUR;
		rc = hw_smartpa_on(&hw_smartpa_ctrl, &cdata);
		if (rc < 0) {
			cam_err("%s smartpa on error", __func__);
			return;
		}
	}
}

#ifdef CAMERA_FLASH_FACTORY_TEST
static struct device_attribute hw_smartpa_lightness =
	__ATTR(flash_lightness, 0664, hw_smartpa_lightness_show,
	hw_smartpa_lightness_store);
#endif

static struct device_attribute hw_smartpa_flash_mask =
	__ATTR(flash_mask, 0664, hw_smartpa_flash_mask_show,
	hw_smartpa_flash_mask_store);

static int hw_smartpa_register_attribute(struct hw_flash_ctrl_t *flash_ctrl,
	struct device *dev)
{
	int rc;
	struct hw_smartpa_private_data_t *pdata = NULL;

	cam_info("%s-%s enter", smartpa_flash_tag, __func__);

	if ((!flash_ctrl) || (!dev)) {
		cam_err("%s flash_ctrl or dev is NULL", __func__);
		return -1;
	}

	pdata = (struct hw_smartpa_private_data_t *)(flash_ctrl->pdata);
	if (!pdata) {
		cam_err("%s pdata is NULL", __func__);
		return -1;
	}

	flash_ctrl->cdev_torch.name = "torch";
	flash_ctrl->cdev_torch.max_brightness =
		(enum led_brightness)pdata->torch_led_num;
	flash_ctrl->cdev_torch.brightness_set = hw_smartpa_torch_brightness_set;

	rc = led_classdev_register((struct device *)dev, &flash_ctrl->cdev_torch);
	if (rc < 0) {
		cam_err("%s failed to register torch classdev", __func__);
		goto err_out;
	}

#ifdef CAMERA_FLASH_FACTORY_TEST
	rc = device_create_file(dev, &hw_smartpa_lightness);
	if (rc < 0) {
		cam_err("%s failed to creat lightness attribute", __func__);
		goto err_create_lightness_file;
	}
#endif
	rc = device_create_file(dev, &hw_smartpa_flash_mask);
	if (rc < 0) {
		cam_err("%s failed to creat flash_mask attribute", __func__);
		goto err_create_flash_mask_file;
	}

	return 0;

err_create_flash_mask_file:
#ifdef CAMERA_FLASH_FACTORY_TEST
	device_remove_file(dev, &hw_smartpa_lightness);
err_create_lightness_file:
#endif
	led_classdev_unregister(&flash_ctrl->cdev_torch);
err_out:

	return rc;
}

static int hw_smartpa_match(struct hw_flash_ctrl_t *flash_ctrl)
{
	int rc = -1;
	cam_info("%s-%s enter", smartpa_flash_tag, __func__);

	if (!flash_ctrl) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return rc;
	}
#ifdef CAMERA_FLASH_FACTORY_TEST
	register_camerafs_attr(&hw_smartpa_lightness);
#endif
	return 0;
}

static const struct of_device_id hw_smartpa_match_id[] = {
	{ .compatible = "huawei,smartpa_flash" },
	{}
};

MODULE_DEVICE_TABLE(of, smartpa_dt_match);

static int hw_smartpa_probe(struct platform_device *pdev)
{
	int ret = -1;
	struct regulator *ldo = NULL;

	cam_info("%s-%s: enter", smartpa_flash_tag, __func__);

	if (!pdev) {
		cam_err("%s pdev is NULL", __func__);
		return ret;
	}

	ret = memset_s(&hw_smartpa_pdata, sizeof(hw_smartpa_pdata), 0,
		sizeof(hw_smartpa_pdata));
	if (ret != EOK)
		cam_err("%s-%s. failed to memset_s write_data",
			smartpa_flash_tag, __func__);

	ldo = devm_regulator_get(&pdev->dev, SMARTPA_BOOST_REGULATOR);
	if (IS_ERR_OR_NULL(ldo)) {
		cam_err("%s-%s: Could not get regulator : %s", smartpa_flash_tag, __func__, SMARTPA_BOOST_REGULATOR);
		ret = -ENXIO;
		goto fail;
	}

	hw_smartpa_pdata.flash_inter_ldo = ldo;

	/* get flash mode regulator */
	ldo = devm_regulator_get(&pdev->dev, SMARTPA_FLASH_MODE_REGULATOR);
	if (IS_ERR_OR_NULL(ldo)) {
		cam_err("%s-%s: Could not get regulator : %s", smartpa_flash_tag, __func__, SMARTPA_FLASH_MODE_REGULATOR);
		ret = -ENXIO;
		goto fail;
	}
	hw_smartpa_pdata.flash_mode_ldo = ldo;

	/* get torch mode regulator */
	ldo = devm_regulator_get(&pdev->dev, SMARTPA_TORCH_MODE_REGULATOR);
	if (IS_ERR_OR_NULL(ldo)) {
		cam_err("%s-%s: Could not get regulator : %s", smartpa_flash_tag, __func__, SMARTPA_TORCH_MODE_REGULATOR);
		ret = -ENXIO;
		goto fail;
	}

	hw_smartpa_pdata.torch_mode_ldo = ldo;
	hw_smartpa_ctrl.pdata = &hw_smartpa_pdata;
	platform_set_drvdata(pdev, &hw_smartpa_ctrl);
	cam_info("%s-%s: end success", smartpa_flash_tag, __func__);
	return hw_flash_platform_probe(pdev, &hw_smartpa_ctrl);

fail:
	if (hw_smartpa_pdata.flash_inter_ldo) {
		devm_regulator_put(hw_smartpa_pdata.flash_inter_ldo);
		hw_smartpa_pdata.flash_inter_ldo = NULL;
	}
	if (hw_smartpa_pdata.flash_mode_ldo) {
		devm_regulator_put(hw_smartpa_pdata.flash_mode_ldo);
		hw_smartpa_pdata.flash_mode_ldo = NULL;
	}
	return ret;
}

static int hw_smartpa_remove(struct platform_device *pdev)
{
	return hw_smartpa_ctrl.func_tbl->flash_exit(&hw_smartpa_ctrl);
}

static struct platform_driver hw_smartpa_platform_driver = {
	.probe = hw_smartpa_probe,
	.remove = hw_smartpa_remove,
	.driver = {
		.name = "smartpa",
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(hw_smartpa_match_id),
#endif
	},
};

static struct hw_flash_fn_t hw_smartpa_func_tbl = {
	.flash_config = hw_flash_config,
	.flash_init = hw_smartpa_init,
	.flash_exit = hw_smartpa_exit,
	.flash_on = hw_smartpa_on,
	.flash_off = hw_smartpa_off,
	.flash_match = hw_smartpa_match,
	.flash_get_dt_data = hw_smartpa_get_dt_data,
	.flash_register_attribute = hw_smartpa_register_attribute,
};

static struct hw_flash_ctrl_t hw_smartpa_ctrl = {
	.func_tbl = &hw_smartpa_func_tbl,
	.hw_flash_mutex = &flash_mut_smartpa,
	.pdata = (void *)&hw_smartpa_pdata,
	.flash_mask_enable = true,
	.state = {
		.mode = STANDBY_MODE,
	},
};

module_platform_driver(hw_smartpa_platform_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("SMARTPA CAMERA FLASH");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");