/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: analog headset switch driver
 * Author: lijinkui
 * Create: 2019-7-16
 */

#include "ana_hs_core.h"
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <chipset_common/hwusb/hw_usb.h>
#include <huawei_platform/log/hw_log.h>
#include "securec.h"
#ifdef CONFIG_HUAWEI_SENSORS_INPUT_INFO
#include <huawei_platform/sensor/hw_comm_pmic.h>
#endif

#define HWLOG_TAG ana_hs_core
HWLOG_REGIST();

#define GPIO_NUM_MAX                     10
#define NAME_LEN_MAX                     20
#define RETRY_TIMES                      3
#define GPIO_CFG_WAIT_TIME               20
#define WAKE_LOCK_TIMEOUT                1000
#define PLUG_IN_STATUS_STABLE_DELAY      800
#define DEFAULT_MIC_SWITCH_DELAY_TIME    0
#define SD_GPIO_SCTRL_REG                0xfff0a314
#define USB_PMIC_VOLTAGE                 5000000
#define LR_CHANNEL_VOLTAGE               2950000
#define ANA_HS_HIGH                      1
#define ANA_HS_LOW                       0

struct ana_hs_core_data {
	int registed;
	int init_flag;
	int ana_hs_in;
	int bus_type;
	int gpio_type;
	int gpio_num;
	/* save all used gpio */
	int gpio_list[GPIO_NUM_MAX];
	int gpio_init_val[GPIO_NUM_MAX];
	int connect_linein_r;
	int support_cc;
	int using_superswitch;
	int switch_antenna;
	int sd_gpio_used;
	int mic_gnd_gpio_num;
	int pmic_num;
	unsigned int *mic_gnd_indexs;
	struct wakeup_source wake_lock;
	struct mutex mutex;
	struct workqueue_struct *plugin_delay_wq;
	struct delayed_work plugin_delay_work;
	struct workqueue_struct *plugout_delay_wq;
	struct delayed_work plugout_delay_work;
	struct ana_hs_codec_dev *codec_ops;

	/* data sequence in each scene, config in dts */
	struct ana_hs_sequence_data *init_seq;
	struct ana_hs_sequence_data *plug_in_seq;
	struct ana_hs_sequence_data *plug_out_seq;
	struct ana_hs_sequence_data *mic_gnd_sequence;
	struct ana_hs_sequence_data *dp_charger_in_seq;
	struct ana_hs_sequence_data *dp_cross_seq;
	struct ana_hs_sequence_data *dp_charger_out_seq;
	struct ana_hs_sequence_data *fb_connect_seq;
	struct ana_hs_sequence_data *fb_disconnect_seq;

	/* read/write ops for i2c or the other bus */
	struct ana_hs_bus_ctl_ops *ops;
	void *private_data;
	void *bus_data;

	/* use regulator to control mos */
	bool regulator_used;
	struct regulator *phy_ldo;
	int phy_ldo_vol;
	struct device *dev;
};

static struct ana_hs_core_data *g_data;

int ana_hs_get_misc_init_flag(void)
{
	if (g_data == NULL || g_data->init_flag == 0)
		return 0;
	return 1;
}

static int hs_gpio_get_value(int gpio)
{
	if (g_data->gpio_type == ANA_HS_GPIO_CODEC)
		return gpio_get_value_cansleep(gpio);
	return gpio_get_value(gpio);
}

static void hs_gpio_set_value(int gpio, int value)
{
	if (gpio <= 0)
		return;
	if (g_data->gpio_type == ANA_HS_GPIO_CODEC)
		gpio_set_value_cansleep(gpio, value);
	else
		gpio_set_value(gpio, value);
}

int ana_hs_register_bus_ops(struct ana_hs_bus_ctl_ops *ops, void *data)
{
	if (g_data == NULL || ops == NULL || data == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	g_data->ops = ops;
	g_data->bus_data = data;

	return 0;
}

int ana_hs_bus_read(unsigned int addr, unsigned int *value)
{
	int ret = -1;

	if (g_data == NULL || g_data->ops == NULL ||
		g_data->bus_data == NULL || value == NULL) {
		hwlog_err("%s: data NULL\n", __func__);
		return -EINVAL;
	}
	if (g_data->bus_type == ANA_HS_I2C) {
		struct ana_hs_i2c_data *data =
			(struct ana_hs_i2c_data *)g_data->bus_data;

		ret = g_data->ops->read_regs(
			data->regmap_cfg->regmap, addr, value);
	}

	return ret;
}

int ana_hs_bus_write(unsigned int addr, unsigned int value)
{
	int ret = -1;

	if (g_data == NULL || g_data->ops == NULL ||
		g_data->bus_data == NULL) {
		hwlog_err("%s: data NULL\n", __func__);
		return -EINVAL;
	}
	if (g_data->bus_type == ANA_HS_I2C) {
		struct ana_hs_i2c_data *data =
			(struct ana_hs_i2c_data *)g_data->bus_data;

		value = value & data->regmap_cfg->value_mask;
		ret = g_data->ops->write_regs(
			data->regmap_cfg->regmap, addr, value);
	}

	return ret;
}

static int ana_hs_bus_update_bits(unsigned int addr,
	unsigned int mask, unsigned int value)
{
	int ret = -1;

	if (g_data == NULL || g_data->ops == NULL ||
		g_data->bus_data == NULL) {
		hwlog_err("%s: data NULL\n", __func__);
		return -EINVAL;
	}
	if (g_data->bus_type == ANA_HS_I2C) {
		struct ana_hs_i2c_data *data =
			(struct ana_hs_i2c_data *)g_data->bus_data;

		value = value & data->regmap_cfg->value_mask;
		ret = g_data->ops->update_bits(
			data->regmap_cfg->regmap, addr, mask, value);
	}

	return ret;
}

static bool read_property_setting(struct device *dev,
	const char *property_name, bool default_setting)
{
	bool setting = false;
	int value = 0;

	if (!of_property_read_u32(dev->of_node, property_name, &value)) {
		if (value != 0)
			setting = true;
		else
			setting = false;
	} else {
		hwlog_info("%s: missing %s, set default value %s\n",
			__func__, property_name,
			default_setting ? "true" : "false");
		setting = default_setting;
	}
	return setting;
}

int ana_hs_get_u32_array(struct device_node *node,
	const char *propname, u32 **value, int *num)
{
	u32 *array = NULL;
	int ret;
	int count = 0;

	if ((node == NULL) || (propname == NULL) || (value == NULL) ||
		(num == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (of_property_read_bool(node, propname))
		count = of_property_count_elems_of_size(node,
			propname, (int)sizeof(u32));
	if (count <= 0) {
		hwlog_debug("%s: %s not existed, skip\n", __func__, propname);
		return 0;
	}

	array = kzalloc(sizeof(u32) * count, GFP_KERNEL);
	if (array == NULL)
		return -ENOMEM;

	ret = of_property_read_u32_array(node, propname, array,
		(size_t)(long)count);
	if (ret < 0) {
		hwlog_err("%s: get %s array failed\n", __func__, propname);
		ret = -EFAULT;
		goto err_out;
	}

	*value = array;
	*num = count;
	return 0;

err_out:
	ana_hs_kfree_ops(array);
	return ret;
}

static void ana_hs_seq_free(struct ana_hs_sequence_data *data)
{
	if (data != NULL) {
		ana_hs_kfree_ops(data->nodes);
		kfree(data);
		data = NULL;
	}
}

int ana_hs_parse_sequence_data(
	struct ana_hs_sequence_data **data, struct device_node *node,
	const char *seq_str)
{
	struct ana_hs_sequence_data *data_temp = NULL;
	int count = 0;
	int val_num;
	int ret;

	if ((node == NULL) || (seq_str == NULL)) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	data_temp = kzalloc(sizeof(*data_temp), GFP_KERNEL);
	if (data_temp == NULL)
		return -ENOMEM;

	ret = ana_hs_get_u32_array(node, seq_str,
		(u32 **)&data_temp->nodes, &count);
	if ((count <= 0) || (ret < 0)) {
		hwlog_err("%s: get %s failed or count is 0\n",
			__func__, seq_str);
		ret = -EFAULT;
		goto err_out;
	}

	val_num = sizeof(struct ana_hs_node) / sizeof(unsigned int);
	if ((count % val_num) != 0) {
		hwlog_err("%s: count %d %% val_num %d != 0\n",
			__func__, count, val_num);
		ret = -EFAULT;
		goto err_out;
	}

	data_temp->num = (unsigned int)(count / val_num);
	*data = data_temp;
	return 0;

err_out:
	ana_hs_seq_free(data_temp);
	return ret;
}

static void ana_hs_free_gpio(void)
{
	int i;

	if (!g_data)
		return;

	for (i = 0; i < g_data->gpio_num; i++) {
		if (g_data->gpio_list[i] > 0)
			gpio_free(g_data->gpio_list[i]);
	}
}

static int ana_hs_load_gpio_data(struct device *dev)
{
	unsigned int temp = 0;
	int ret;
	int i;
	char name[NAME_LEN_MAX] = {0};

	if (!of_property_read_u32(dev->of_node, "gpio_num", &temp))
		g_data->gpio_num = temp;
	else
		g_data->gpio_num = 0;

	for (i = 0; i < g_data->gpio_num; i++) {
		g_data->gpio_list[i] = of_get_named_gpio(dev->of_node, "gpios", i);
		if (g_data->gpio_list[i] < 0) {
			hwlog_err("%s load gpio list failed\n", __func__);
			return g_data->gpio_list[i];
		}
	}

	ret = of_property_read_u32_array(dev->of_node, "gpio_init_val",
		g_data->gpio_init_val, g_data->gpio_num);
	if (ret < 0) {
		hwlog_err("%s load gpio init val failed\n", __func__);
		return ret;
	}

	for (i = 0; i < g_data->gpio_num; i++) {
		ret = snprintf_s(name, NAME_LEN_MAX, NAME_LEN_MAX - 1, "ana_hs_gpio%d", i);
		if (ret < 0)
			hwlog_err("%s snprintf_s is failed\n", __func__);

		if (!gpio_is_valid(g_data->gpio_list[i])) {
			hwlog_err("%s gpio %d is unvalid \n",
				__func__, g_data->gpio_list[i]);
			return -ENOENT;
		}
		ret = gpio_request(g_data->gpio_list[i], name);
		if (ret < 0) {
			hwlog_err("%s request GPIO for %d fail %d\n",
				__func__, g_data->gpio_list[i], ret);
			return ret;
		} else {
			ret = gpio_direction_output(g_data->gpio_list[i],
				g_data->gpio_init_val[i]);
			if (ret < 0) {
				hwlog_err("%s set output for %d fail %d\n",
					__func__, g_data->gpio_list[i], ret);
				return ret;
			}
		}
	}

	return 0;
}

static void ana_hs_gpio_sequence_update(struct ana_hs_sequence_data *data)
{
	int i;

	if (!data)
		return;

	for (i = 0; i < data->num; i++) {
		hs_gpio_set_value(g_data->gpio_list[data->nodes[i].key],
			data->nodes[i].value);
		if (data->nodes[i].delay > 0)
			msleep(data->nodes[i].delay);
	}
}

static void ana_hs_i2c_sequence_update(struct ana_hs_sequence_data *data)
{
	int i;
	int ret = 0;

	if (!data)
		return;

	for (i = 0; i < data->num; i++) {
		if (data->nodes[i].mask != 0)
			ret = ana_hs_bus_update_bits(data->nodes[i].key,
				data->nodes[i].mask, data->nodes[i].value);
		else
			ret = ana_hs_bus_write(data->nodes[i].key,
				data->nodes[i].value);
		if (ret < 0) {
			hwlog_err("%s: write reg 0x%x error\n",
				__func__, data->nodes[i].key);
			break;
		}

		if (data->nodes[i].delay > 0)
			msleep(data->nodes[i].delay);
	}
}

static void ana_hs_sequence_update(struct ana_hs_sequence_data *data)
{
	if (g_data->bus_type == ANA_HS_GPIO)
		ana_hs_gpio_sequence_update(data);
	else
		ana_hs_i2c_sequence_update(data);
}

static void load_bus_type(struct device_node *node)
{
	unsigned int temp = ANA_HS_GPIO;

	if (!of_property_read_u32(node, "bus_type", &temp))
		g_data->bus_type = temp;
	else
		g_data->bus_type = ANA_HS_GPIO;
}

static void load_gpio_type_config(struct device_node *node)
{
	unsigned int temp = ANA_HS_GPIO_SOC;

	if (!of_property_read_u32(node, "gpio_type", &temp))
		g_data->gpio_type = temp;
	else
		g_data->gpio_type = ANA_HS_GPIO_SOC;
}

static void get_mic_gnd_gpios(struct device_node *node)
{
	int count = 0;
	int ret;

	ret = ana_hs_get_u32_array(node, "mic_gnd_indexs",
		(u32 **)&g_data->mic_gnd_indexs, &count);
	if (ret < 0 || count <= 0) {
		hwlog_err("%s:get mic_gnd_indexs fail or count is 0\n",
			__func__);
		return;
	}
	g_data->mic_gnd_gpio_num = count;
}

#ifdef CONFIG_HUAWEI_SENSORS_INPUT_INFO
static void ana_hs_audio_pmic_ldo_set_state(int state)
{
	struct hw_comm_pmic_cfg_t audio_pmic_ldo_set;

	audio_pmic_ldo_set.pmic_num = g_data->pmic_num;
	audio_pmic_ldo_set.pmic_power_type = VOUT_BOOST_EN;
	audio_pmic_ldo_set.pmic_power_voltage = USB_PMIC_VOLTAGE;
	audio_pmic_ldo_set.pmic_power_state = state;
	hw_pmic_power_cfg(AUDIO_PMIC_REQ, &audio_pmic_ldo_set);
}
#endif

static bool ana_hs_prepare_regulator(void)
{
	int ret;

	g_data->phy_ldo =
		devm_regulator_get(g_data->dev, "ana_hs_switch");
	if (IS_ERR(g_data->phy_ldo)) {
		hwlog_err("phy_ldo regulator dts read failed\n");
		g_data->phy_ldo = NULL;
		return false;
	}

	ret = regulator_set_voltage(g_data->phy_ldo,
		g_data->phy_ldo_vol, g_data->phy_ldo_vol);
	if (ret != 0) {
		hwlog_err("phy_ldo regulator set voltage failed\n");
		devm_regulator_put(g_data->phy_ldo);
		g_data->phy_ldo = NULL;
		return false;
	}

	return true;
}

static void plugin_regulator_config(void)
{
	if (!g_data || !g_data->regulator_used) {
		hwlog_info("%s: g_data is null or ldo not used\n", __func__);
		return;
	}

	if ((g_data->phy_ldo == NULL) && !ana_hs_prepare_regulator()) {
		hwlog_err("%s: regulator used but get failed\n", __func__);
		return;
	}

	hwlog_info("%s: regulator config\n", __func__);
#ifdef CONFIG_HUAWEI_SENSORS_INPUT_INFO
	ana_hs_audio_pmic_ldo_set_state(ANA_HS_LOW);
#endif
	if (regulator_enable(g_data->phy_ldo)) {
		hwlog_err("error: regulator enable failed\n");
#ifdef CONFIG_HUAWEI_SENSORS_INPUT_INFO
		ana_hs_audio_pmic_ldo_set_state(ANA_HS_HIGH);
#endif
	}
}

static void plugin_work(struct work_struct *work)
{
	__pm_stay_awake(&g_data->wake_lock);

	/*
	 * change codec hs resistence from 70ohm to 3Kohm,
	 * to reduce the pop sound in hs when usb analog hs plug in.
	 */
	g_data->codec_ops->ops.hs_high_resistence_enable(
		g_data->private_data, true);

	/* to avoid TDD-noise */
	if (g_data->switch_antenna)
		hw_usb_send_event(ANA_AUDIO_IN_EVENT);

	plugin_regulator_config();

	ana_hs_sequence_update(g_data->plug_in_seq);
	msleep(GPIO_CFG_WAIT_TIME);
	g_data->codec_ops->ops.plug_in_detect(
		g_data->private_data);
	mutex_lock(&g_data->mutex);
	g_data->ana_hs_in = ANA_HS_PLUG_IN;

	mutex_unlock(&g_data->mutex);

	/* recovery codec hs resistence to 70ohm. */
	g_data->codec_ops->ops.hs_high_resistence_enable(
		g_data->private_data, false);
	__pm_relax(&g_data->wake_lock);
}

static void plugout_regulator_config(void)
{
	if (!g_data || !g_data->regulator_used || !g_data->phy_ldo) {
		hwlog_info("%s: g_data is null or ldo is null\n",
			__func__);
		return;
	}

	hwlog_info("%s: regulator config\n", __func__);
	if (regulator_disable(g_data->phy_ldo)) {
		hwlog_err("error: regulator enable failed\n");
		return;
	}
#ifdef CONFIG_HUAWEI_SENSORS_INPUT_INFO
	ana_hs_audio_pmic_ldo_set_state(ANA_HS_HIGH);
#endif
}

static void plugout_work(struct work_struct *work)
{
	__pm_stay_awake(&g_data->wake_lock);
	if (g_data->plugin_delay_wq) {
		hwlog_info("%s remove plugin work, insert-remov too fast\n",
			__func__);
		cancel_delayed_work(
			&g_data->plugin_delay_work);
		flush_workqueue(g_data->plugin_delay_wq);
	}
	if (g_data->ana_hs_in == ANA_HS_PLUG_IN) {
		hwlog_info("%s usb analog hs plug out act\n", __func__);
		g_data->codec_ops->ops.plug_out_detect(
			g_data->private_data);
		mutex_lock(&g_data->mutex);
		g_data->ana_hs_in = ANA_HS_PLUG_OUT;
		mutex_unlock(&g_data->mutex);

		plugout_regulator_config();

		ana_hs_sequence_update(g_data->plug_out_seq);

		if (g_data->switch_antenna)
			/* notify the phone: usb analog hs plug out. */
			hw_usb_send_event(ANA_AUDIO_OUT_EVENT);
	}
	__pm_relax(&g_data->wake_lock);
}

static void ana_hs_remove_workqueue(void)
{
	if (g_data->plugout_delay_wq) {
		cancel_delayed_work(&g_data->plugout_delay_work);
		flush_workqueue(g_data->plugout_delay_wq);
		destroy_workqueue(g_data->plugout_delay_wq);
	}
	if (g_data->plugin_delay_wq) {
		cancel_delayed_work(&g_data->plugin_delay_work);
		flush_workqueue(g_data->plugin_delay_wq);
		destroy_workqueue(g_data->plugin_delay_wq);
	}
}

static int ana_hs_create_workqueue(void)
{
	int ret = 0;

	/* create workqueue */
	g_data->plugin_delay_wq = create_singlethread_workqueue("ana_hs_plugin_delay_wq");
	if (!(g_data->plugin_delay_wq)) {
		hwlog_err("%s plugin wq create failed\n", __func__);
		return -ENOMEM;
	}
	INIT_DELAYED_WORK(&g_data->plugin_delay_work, plugin_work);

	g_data->plugout_delay_wq = create_singlethread_workqueue("ana_hs_plugout_delay_wq");
	if (!(g_data->plugout_delay_wq)) {
		hwlog_err("%s plugout wq create failed\n", __func__);
		ret = -ENOMEM;
		goto err_create_wq;
	}
	INIT_DELAYED_WORK(&g_data->plugout_delay_work, plugout_work);

	return ret;
err_create_wq:
	ana_hs_remove_workqueue();
	return ret;
}

static int ana_hs_chip_init(void)
{
	if (g_data == NULL) {
		hwlog_err("%s: g_data NULL\n", __func__);
		return -1;
	}
	ana_hs_sequence_update(g_data->init_seq);

	return 0;
}

static int ana_hs_core_pluged_state(void)
{
	int ana_hs_state = pd_dpm_get_analog_hs_state();

	hwlog_info("%s ana_hs_state =%d\n", __func__, ana_hs_state);

	if (ana_hs_state)
		return ANA_HS_PLUG_IN;
	return ANA_HS_PLUG_OUT;
}

static int ana_hs_core_codec_dev_register(
	struct ana_hs_codec_dev *dev, void *codec_data)
{
	/* analog headset driver not be probed, just return */
	if (!g_data) {
		hwlog_err("%s g_data is NULL\n", __func__);
		return -ENODEV;
	}

	/* only support one codec to be registed */
	if (g_data->registed == ANA_HS_ALREADY_REGISTER) {
		hwlog_err("%s codec has registed, no more permit\n",
			__func__);
		return -EEXIST;
	}

	if (!dev->ops.plug_in_detect || !dev->ops.plug_out_detect) {
		hwlog_err("%s codec ops function must be all registed\n",
			__func__);
		return -EINVAL;
	}

	mutex_lock(&g_data->mutex);
	g_data->codec_ops = dev;
	g_data->private_data = codec_data;
	g_data->registed = ANA_HS_ALREADY_REGISTER;
	mutex_unlock(&g_data->mutex);

	hwlog_info("%s analog hs register sucessful\n", __func__);

	return 0;
}

static bool ana_hs_usb_hs_support(void)
{
	if (!g_data)
		return false;

	if (g_data->registed == ANA_HS_NOT_REGISTER)
		return false;
	return true;
}

static void ana_hs_core_plug_handle(int hs_state)
{
	/* analog headset driver not be probed, just return */
	if (!g_data) {
		hwlog_err("%s pdata is NULL\n", __func__);
		return;
	}
	if (g_data->registed == ANA_HS_NOT_REGISTER) {
		hwlog_err("%s codec_ops_dev is not registed\n", __func__);
		return;
	}

	__pm_wakeup_event(&g_data->wake_lock,
		WAKE_LOCK_TIMEOUT);

	switch (hs_state) {
	case ANA_HS_PLUG_IN:
		queue_delayed_work(g_data->plugin_delay_wq,
			&g_data->plugin_delay_work,
			msecs_to_jiffies(PLUG_IN_STATUS_STABLE_DELAY));
		break;
	case ANA_HS_PLUG_OUT:
		queue_delayed_work(g_data->plugout_delay_wq,
			&g_data->plugout_delay_work, 0);
		break;
	case DP_PLUG_IN:
		ana_hs_sequence_update(g_data->dp_charger_in_seq);
		break;
	case DP_PLUG_IN_CROSS:
		ana_hs_sequence_update(g_data->dp_cross_seq);
		break;
	case DP_PLUG_OUT:
		ana_hs_sequence_update(g_data->dp_charger_out_seq);
		break;
	case DIRECT_CHARGE_IN:
		ana_hs_sequence_update(g_data->dp_charger_in_seq);
		break;
	case DIRECT_CHARGE_OUT:
		ana_hs_sequence_update(g_data->dp_charger_out_seq);
		break;
	default:
		hwlog_err("%s unknown state\n", __func__);
		break;
	}
	hwlog_info("%s hs_state is %d\n", __func__, hs_state);
}

static int set_mic_sel_val(int sel_value)
{
	int ret = sel_value ? 0 : 1;

	return ret;
}

static void gpio_mic_gnd_swap(void)
{
	unsigned int i;
	int value;
	struct ana_hs_sequence_data *data = g_data->mic_gnd_sequence;
	struct ana_hs_node *node = NULL;

	if (data == NULL) {
		hwlog_err("%s: mic_gnd_sequence NULL, do nothing\n", __func__);
		return;
	}
	for (i = 0; i < data->num; i++) {
		node = &data->nodes[i];
		if (node->key >= (unsigned int)(g_data->gpio_num)) {
			hwlog_err("%s: invalid gpio index\n", __func__);
			return;
		}
		value = hs_gpio_get_value(g_data->gpio_list[node->key]);
		hwlog_info("%s: gpio %d before value %d\n", __func__,
			g_data->gpio_list[node->key], value);
		value = set_mic_sel_val(value);
		hs_gpio_set_value(g_data->gpio_list[node->key], value);
		if (node->delay > 0)
			msleep(node->delay);
	}
}

static void i2c_mic_gnd_swap(void)
{
	int i;
	int ret = 0;
	unsigned int value = 0;
	struct ana_hs_sequence_data *data = g_data->mic_gnd_sequence;

	for (i = 0; i < data->num; i++) {
		ret = ana_hs_bus_read(data->nodes->key, &value);
		if (ret < 0) {
			hwlog_err("%s: read reg 0x%x fail\n",
				__func__, data->nodes->key);
			return;
		}
		/* mask should not be 0, set in dts */
		if ((value & data->nodes->mask) == data->nodes->mask)
			value = value & (~data->nodes->mask);
		else
			value = value | data->nodes->mask;

		ret = ana_hs_bus_update_bits(data->nodes->key,
			data->nodes->mask, value);
		if (ret < 0) {
			hwlog_err("%s: write reg 0x%x error\n",
				__func__, data->nodes->key);
			break;
		}
		if (data->nodes->delay > 0)
			msleep(data->nodes->delay);
	}
}

void ana_hs_core_mic_gnd_swap(void)
{
	/* analog headset driver not be probed, just return */
	if (!g_data) {
		hwlog_err("%s g_data is NULL\n", __func__);
		return;
	}

	if (g_data->registed == ANA_HS_NOT_REGISTER) {
		hwlog_err("%s codec_ops_dev is not registed\n", __func__);
		return;
	}

	if (g_data->bus_type == ANA_HS_I2C) {
		i2c_mic_gnd_swap();
	} else {
		gpio_mic_gnd_swap();
	}
}

/*
 * used by superswitch
 */
void ana_hs_set_gpio_state(int enable)
{
	if (g_data == NULL)
		return;

	switch (enable) {
	case ANA_HS_ENABLE:
		ana_hs_sequence_update(g_data->dp_charger_in_seq);
		break;
	case ANA_HS_DISABLE:
		ana_hs_sequence_update(g_data->dp_charger_out_seq);
		break;
	default:
		hwlog_err("%s: unknown arg\n", __func__);
	}
}

static long ana_hs_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int gpio_mic_sel_val = 0;
	uintptr_t arg_tmp = (uintptr_t)arg;
	unsigned int __user *p_user = (unsigned int __user *)arg_tmp;

	if (!g_data)
		return -EBUSY;

	if (g_data->registed == ANA_HS_NOT_REGISTER)
		return -EBUSY;

	switch (cmd) {
	case IOCTL_ANA_HS_GET_MIC_SWITCH_STATE:
		if (g_data->bus_type == ANA_HS_GPIO) {
			/* return first gpio value for mic gnd state test */
			gpio_mic_sel_val = hs_gpio_get_value(
				g_data->gpio_list[g_data->mic_gnd_indexs[0]]);
			hwlog_info("%s gpio_mic_sel_val = %d\n",
				__func__, gpio_mic_sel_val);
		}
		ret = put_user((__u32)(gpio_mic_sel_val), p_user);
		break;
	case IOCTL_ANA_HS_GET_CONNECT_LINEIN_R_STATE:
		hwlog_info("%s connect_linein_r = %d\n",
			__func__, g_data->connect_linein_r);
		ret = put_user((__u32)(g_data->connect_linein_r),
			p_user);
		break;
	case IOCTL_ANA_HS_GND_FB_CONNECT:
		hwlog_info("%s gnd fb connect\n", __func__);
		ana_hs_sequence_update(g_data->fb_connect_seq);
		break;
	case IOCTL_ANA_HS_GND_FB_DISCONNECT:
		hwlog_info("%s ioctl gnd fb disconnect\n", __func__);
		ana_hs_sequence_update(g_data->fb_disconnect_seq);
		break;
	default:
		hwlog_err("%s unsupport cmd\n", __func__);
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

static const struct file_operations ana_hs_fops = {
	.owner               = THIS_MODULE,
	.open                = simple_open,
	.unlocked_ioctl      = ana_hs_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl        = ana_hs_ioctl,
#endif
};

static struct miscdevice ana_hs_device = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = "ana_hs_core",
	.fops   = &ana_hs_fops,
};

struct ana_hs_dev ana_hs_ops = {
	.tag = TAG_ANA_HS_CORE,
	.ops.ana_hs_pluged_state = ana_hs_core_pluged_state,
	.ops.ana_hs_dev_register = ana_hs_core_codec_dev_register,
	.ops.ana_hs_support_usb_sw = ana_hs_usb_hs_support,
	.ops.ana_hs_plug_handle = ana_hs_core_plug_handle,
	.ops.ana_hs_mic_gnd_swap = ana_hs_core_mic_gnd_swap,
};

static void parse_sequence_data(struct device_node *node)
{
	int ret;

	if (g_data == NULL)
		return;

	ret = ana_hs_parse_sequence_data(
		&g_data->init_seq, node, "init_seq");
	if (ret < 0)
		hwlog_err("%s: parse dts init_seq fail\n", __func__);
	ret = ana_hs_parse_sequence_data(
		&g_data->plug_in_seq, node, "plug_in_seq");
	if (ret < 0)
		hwlog_err("%s: parse dts plug_in_seq fail\n", __func__);
	ret = ana_hs_parse_sequence_data(
		&g_data->plug_out_seq, node, "plug_out_seq");
	if (ret < 0)
		hwlog_err("%s: parse dts plug_out_seq fail\n", __func__);
	ret = ana_hs_parse_sequence_data(
		&g_data->mic_gnd_sequence, node, "mic_gnd_sequence");
	if (ret < 0)
		hwlog_err("%s: parse dts mic_gnd_sequence fail\n", __func__);
	ret = ana_hs_parse_sequence_data(
		&g_data->dp_charger_in_seq, node, "dp_charger_in_seq");
	if (ret < 0)
		hwlog_err("%s: parse dts dp_charger_in_seq fail\n", __func__);
	ret = ana_hs_parse_sequence_data(
		&g_data->dp_cross_seq, node, "dp_cross_seq");
	if (ret < 0)
		hwlog_err("%s: parse dts dp_cross_seq fail\n", __func__);
	ret = ana_hs_parse_sequence_data(
		&g_data->dp_charger_out_seq, node, "dp_charger_out_seq");
	if (ret < 0)
		hwlog_err("%s: parse dts dp_charger_out_seq fail\n", __func__);
	ret = ana_hs_parse_sequence_data(
		&g_data->fb_connect_seq, node, "fb_connect_seq");
	if (ret < 0)
		hwlog_err("%s: parse dts fb_connect_seq fail\n", __func__);
	ret = ana_hs_parse_sequence_data(
		&g_data->fb_disconnect_seq, node, "fb_disconnect_seq");
	if (ret < 0)
		hwlog_err("%s: parse dts fb_disconnect_seq fail\n", __func__);
}

static void ana_hs_get_prop(struct device *dev)
{
	if (g_data == NULL)
		return;

	g_data->connect_linein_r = read_property_setting(dev,
		"connect_linein_r", true);
	g_data->support_cc = read_property_setting(dev,
		"support_cc", false);
	g_data->using_superswitch = read_property_setting(dev,
		"using_superswitch", false);
	g_data->switch_antenna = read_property_setting(dev,
		"switch_antenna", false);
}

static void free_sequence_data(void)
{
	if (g_data->init_seq != NULL)
		ana_hs_seq_free(g_data->init_seq);
	if (g_data->plug_in_seq != NULL)
		ana_hs_seq_free(g_data->plug_in_seq);
	if (g_data->plug_out_seq != NULL)
		ana_hs_seq_free(g_data->plug_out_seq);
	if (g_data->mic_gnd_sequence != NULL)
		ana_hs_seq_free(g_data->mic_gnd_sequence);
	if (g_data->dp_charger_in_seq != NULL)
		ana_hs_seq_free(g_data->dp_charger_in_seq);
	if (g_data->dp_cross_seq != NULL)
		ana_hs_seq_free(g_data->dp_cross_seq);
	if (g_data->dp_charger_out_seq != NULL)
		ana_hs_seq_free(g_data->dp_charger_out_seq);
	if (g_data->fb_connect_seq != NULL)
		ana_hs_seq_free(g_data->fb_connect_seq);
	if (g_data->fb_disconnect_seq != NULL)
		ana_hs_seq_free(g_data->fb_disconnect_seq);
}

static int ana_hs_get_regulator(struct device *dev)
{
	int ret;
	int temp = 0;
	struct device_node *node = dev->of_node;

	if (!g_data) {
		hwlog_err("%s error: g_data is null\n", __func__);
		return -1;
	}

	if (of_property_read_bool(node, "regulator_used"))
		g_data->regulator_used = true;

	if (!g_data->regulator_used) {
		hwlog_info("%s: phy_ldo regulator not used\n", __func__);
		return -1;
	}

	ret = of_property_read_s32(node, "ana_hs_switch_vol",
		&g_data->phy_ldo_vol);
	if (ret != 0) {
		hwlog_warn("phy_ldo_vol use default\n");
		g_data->phy_ldo_vol = LR_CHANNEL_VOLTAGE;
	}

	g_data->phy_ldo =
		devm_regulator_get(dev, "ana_hs_switch");
	if (IS_ERR(g_data->phy_ldo)) {
		hwlog_err("phy_ldo regulator dts read failed\n");
		g_data->phy_ldo = NULL;
		return -1;
	}

	ret = regulator_set_voltage(g_data->phy_ldo,
		g_data->phy_ldo_vol, g_data->phy_ldo_vol);
	if (ret != 0) {
		hwlog_err("phy_ldo regulator set voltage failed\n");
		return -1;
	}

	ret = regulator_get_voltage(g_data->phy_ldo);
	hwlog_info("%s phy_ldo_vol=%d\n", __func__, ret);

	if (!of_property_read_u32(dev->of_node, "pmic_num", &temp))
		g_data->pmic_num = temp;
	else
		g_data->pmic_num = 0;

	return 0;
}

static int sd_gpio_config(void)
{
	unsigned int tmp_reg_value;
	unsigned long *virtual_addr = (unsigned long *)ioremap(
		SD_GPIO_SCTRL_REG, sizeof(unsigned long));

	/* 1: use sd gpio, need reset gpio status */
	if (g_data->sd_gpio_used != 1)
		return 0;
	if (!virtual_addr) {
		hwlog_err("%s sd gpio config fail\n", __func__);
		return -1;
	}
	/* BIT2: reset sd gpio status */
	tmp_reg_value = *(unsigned long *)virtual_addr | 0x4;
	*(unsigned long *)virtual_addr = tmp_reg_value;

	iounmap(virtual_addr);

	return 0;
}

static int ana_hs_core_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	int ret;

	g_data = kzalloc(sizeof(*g_data), GFP_KERNEL);
	if (!g_data) {
		hwlog_err("%s can not allocate ana hs dev data\n", __func__);
		return -ENOMEM;
	}

	g_data->init_flag = 1;
	g_data->dev = dev;
	ana_hs_get_prop(dev);

	wakeup_source_init(&g_data->wake_lock, "ana_hs_core");
	mutex_init(&g_data->mutex);

	load_bus_type(node);
	if (g_data->bus_type == ANA_HS_GPIO) {
		ret = ana_hs_load_gpio_data(dev);
		if (ret < 0) {
			hwlog_err("%s get gpios failed, ret =%d\n", __func__, ret);
			goto ana_hs_err_out;
		}
		load_gpio_type_config(node);
		get_mic_gnd_gpios(node);
		ret = sd_gpio_config();
		if (ret < 0)
			goto ana_hs_err_out;
	}

	parse_sequence_data(node);

	g_data->registed = ANA_HS_NOT_REGISTER;
	g_data->ana_hs_in = ANA_HS_PLUG_OUT;
	ret = ana_hs_chip_init();
	if (ret < 0) {
		hwlog_err("%s ana hs chip init failed, ret =%d\n", __func__, ret);
		goto ana_hs_err_out;
	}

	g_data->regulator_used = false;
	ret = ana_hs_get_regulator(dev);
	if (ret < 0)
		hwlog_info("%s: get regulator fail, maybe not needed\n", __func__);

	/* create workqueue */
	ret = ana_hs_create_workqueue();
	if (ret)
		goto ana_hs_err_out;

	ana_hs_dev_register(&ana_hs_ops, ANA_PRIORITY_H);

	/* register misc device for userspace */
	ret = misc_register(&ana_hs_device);
	if (ret) {
		hwlog_err("%s misc device register failed\n", __func__);
		goto err_misc_register;
	}

	return 0;

err_misc_register:
	ana_hs_remove_workqueue();
ana_hs_err_out:
	free_sequence_data();
	ana_hs_free_gpio();
	wakeup_source_trash(&g_data->wake_lock);
	g_data->init_flag = 0;
	kfree(g_data);
	g_data = NULL;

	return ret;
}

static int ana_hs_core_remove(struct platform_device *pdev)
{
	hwlog_info("%s in remove\n", __func__);

	if (!g_data)
		return 0;

	ana_hs_dev_deregister(TAG_ANA_HS_CORE);
	ana_hs_remove_workqueue();
	ana_hs_free_gpio();
	free_sequence_data();
	misc_deregister(&ana_hs_device);

	g_data->init_flag = 0;
	kfree(g_data);
	g_data = NULL;

	return 0;
}

static const struct of_device_id ana_hs_core_of_match[] = {
	{ .compatible = "huawei,ana_hs_core", },
	{},
};
MODULE_DEVICE_TABLE(of, ana_hs_core_of_match);

static struct platform_driver ana_hs_core_driver = {
	.driver = {
		.name   = "ana_hs_core",
		.owner  = THIS_MODULE,
		.of_match_table = ana_hs_core_of_match,
	},
	.probe  = ana_hs_core_probe,
	.remove = ana_hs_core_remove,
};

static int __init ana_hs_core_init(void)
{
	return platform_driver_register(&ana_hs_core_driver);
}

static void __exit ana_hs_core_exit(void)
{
	platform_driver_unregister(&ana_hs_core_driver);
}

subsys_initcall_sync(ana_hs_core_init);
module_exit(ana_hs_core_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("headset connect switch driver for Type-C analog headset");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
