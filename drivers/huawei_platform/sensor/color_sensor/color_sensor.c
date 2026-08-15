#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/unistd.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/pm.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

#include "color_sensor.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG color_sensor
HWLOG_REGIST();

#define TOTAL_GAIN_NUM    5
#define DATA_NUM_PER_GAIN 4
#define TIME_OUT_DEFAULT 1500
#define ONE_SHOW_LEN     10
#define MAX_FLICK_DATA_LEN 1024
#define NV_READ_TAG	    1
#define NV_WRITE_TAG	0

static struct class *color_sensor_class;
struct hisi_nve_info_user nv_user_info;
int color_report_val[MAX_REPORT_LEN] = {0};
UINT32 g_flicker_support = 0;
static struct delayed_work color_sensor_dmd_work;
static struct color_priv_data_t color_priv_data = {
	.rgb_support = 0,
	.rgb_absent = 0,
};
#ifdef CONFIG_HUAWEI_DSM
extern struct dsm_client *shb_dclient;
#endif

int color_sensor_get_byte(const struct i2c_client *i2c, uint8_t reg,
	uint8_t *data)
{
	int ret = -EINVAL;

	if (!i2c || !data) {
		hwlog_err("%s data or handle Pointer is NULL\n", __func__);
		return ret;
	}

	ret = i2c_smbus_read_i2c_block_data(i2c, reg, 1, data);
	if (ret < 0)
		hwlog_err("%s failed\n", __func__);

	return ret;
}

int color_sensor_set_byte(const struct i2c_client *i2c, uint8_t reg,
	uint8_t data)
{
	int ret = -EINVAL;

	if (!i2c) {
		hwlog_err("%s data or handle Pointer is NULL\n", __func__);
		return ret;
	}

	ret = i2c_smbus_write_i2c_block_data(i2c, reg, 1, &data);
	if (ret < 0)
		hwlog_err("%s failed\n", __func__);

	return ret;
}

int color_sensor_read_fifo(struct i2c_client *client, uint8_t reg,
	void *buf, size_t len)
{
	struct i2c_msg msg[2]; // one set for reg, one for read buf

	if(!client || !buf) {
		hwlog_err("%s, client buf is NULL\n", __func__);
		return -EINVAL;
	}

	msg[0].addr = client->addr;
	msg[0].flags = 0; // 0 set for read
	msg[0].len = 1;   // reg len
	msg[0].buf = &reg;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = len;
	msg[1].buf = buf;

	if (i2c_transfer(client->adapter, msg, 2) != 2) { // 2 case i2c err
		hwlog_err("i2c transfer failed\n");
		return -EIO;
	}

	return 0;
}

#ifdef CONFIG_HUAWEI_DSM
static void color_dmd_work(struct work_struct *work)
{
	if (dsm_client_ocuppy(shb_dclient))
		return;

	if ((color_priv_data.rgb_support > 0) &&
		(color_priv_data.rgb_support == color_priv_data.rgb_absent)) {
		dsm_client_record(shb_dclient, "color detect fail\n");
		dsm_client_notify(shb_dclient,
			DSM_AP_ERR_COLORSENSOR_DETECT);
		hwlog_err("%s color sensor fail\n", __func__);
	}
}
#endif

int read_color_calibrate_data_from_nv(int nv_number, int nv_size, char *nv_name, char *temp)
{
	int ret = 0;
	if(NULL == nv_name || NULL == temp){
		hwlog_err("read_color_calibrate_data_from_nv INPUT PARA NULL\n");
		return -1;
	}
	hwlog_info("read_color_calibrate_data_from_nv %d in!!\n", nv_number);
	memset(&nv_user_info, 0, sizeof(nv_user_info));
	nv_user_info.nv_operation = NV_READ_TAG;
	nv_user_info.nv_number = nv_number;
	nv_user_info.valid_size = nv_size;
	strncpy(nv_user_info.nv_name, nv_name, sizeof(nv_user_info.nv_name));
	nv_user_info.nv_name[sizeof(nv_user_info.nv_name) - 1] = '\0';
	if ((ret = hisi_nve_direct_access(&nv_user_info))!=0)
	{
		hwlog_err("hisi_nve_direct_access read nv %d error(%d)\n", nv_number, ret);
		return -1;
	}
	memcpy(temp, nv_user_info.nv_data, sizeof(nv_user_info.nv_data) < nv_user_info.valid_size ?
		sizeof(nv_user_info.nv_data) : nv_user_info.valid_size);
	return 0;
}
EXPORT_SYMBOL_GPL(read_color_calibrate_data_from_nv);

int write_color_calibrate_data_to_nv(int nv_number, int nv_size, char *nv_name, char *temp)
{
	int ret = 0;
	if(NULL == nv_name || NULL == temp){
		hwlog_err("write_color_calibrate_data_to_nv INPUT PARA NULL\n");
		return -1;
	}
	hwlog_info("write_color_calibrate_data_to_nv %d in!!\n", nv_number);
	memset(&nv_user_info, 0, sizeof(nv_user_info));
	nv_user_info.nv_operation = NV_WRITE_TAG;
	nv_user_info.nv_number = nv_number;
	nv_user_info.valid_size = nv_size;
	strncpy(nv_user_info.nv_name, nv_name, sizeof(nv_user_info.nv_name));
	nv_user_info.nv_name[sizeof(nv_user_info.nv_name) - 1] = '\0';
	/*copy to nv by pass*/
	memcpy(nv_user_info.nv_data, temp, sizeof(nv_user_info.nv_data) < nv_user_info.valid_size ?
		sizeof(nv_user_info.nv_data) : nv_user_info.valid_size);
	if ((ret = hisi_nve_direct_access(&nv_user_info))!=0)
	{
		hwlog_err("hisi_nve_direct_access read nv %d error(%d)\n", nv_number, ret);
		return -1;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(write_color_calibrate_data_to_nv);

static ssize_t color_calibrate_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	color_sensor_output_para out_data ={0};
	struct colorDriver_chip *chip = NULL;
	int size = 1;
	if(NULL == dev || NULL == attr || NULL == buf)
	{
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}
	chip = dev_get_drvdata(dev);
	if(NULL == chip){
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}

	hwlog_info("[%s] in \n", __func__);
	size = sizeof(color_sensor_output_para);
	if (chip->color_show_calibrate_state == NULL) {
		hwlog_err("[%s] color_show_calibrate_state NULL!! \n", __func__);
		return -1;
	}
	chip->color_show_calibrate_state(chip, &out_data);
	memcpy(buf, &out_data, size);
	return size;
}

static ssize_t color_calibrate_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t size)
{
	color_sensor_input_para in_data;
	struct colorDriver_chip *chip = NULL;

	if(NULL == dev || NULL == attr || NULL == buf)
	{
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}
	chip = dev_get_drvdata(dev);
	if(NULL == chip){
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}

	hwlog_info("[%s] color_sensor store in!! \n", __func__);

	if (size >= sizeof(color_sensor_input_para)){
		memcpy(&in_data, buf, sizeof(color_sensor_input_para));
	}

	if (chip->color_store_calibrate_state == NULL) {
		hwlog_err("[%s] color_store_calibrate_state NULL!! \n", __func__);
		return -1;
	}

	hwlog_info("[%s] color_sensor store input enable = %d, data[%d, %d, %d, %d].\n", __func__,
		in_data.enable, in_data.tar_x, in_data.tar_y, in_data.tar_z, in_data.tar_ir);
	chip->color_store_calibrate_state(chip, &in_data);

	return size;
}
static ssize_t at_color_calibrate_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t size)
{
	at_color_sensor_input_para in_data;
	struct colorDriver_chip *chip = NULL;

	if(NULL == dev || NULL == attr || NULL == buf)
	{
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}
	chip = dev_get_drvdata(dev);
	if(NULL == chip){
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}
	if(NULL == chip->at_color_store_calibrate_state){
		hwlog_err("[%s] at_color_store_calibrate_state NULL!! \n", __func__);
		return -1;
	}
	hwlog_info("[%s] color_sensor store in!! \n", __func__);

	memcpy(&in_data, buf, min(size, sizeof(at_color_sensor_input_para)));

	hwlog_info("%s, target = %d, %d, %d, %d, %d, %d, %d, %d, %d", __func__, in_data.enable, \
		in_data.reserverd[0],in_data.reserverd[1],in_data.reserverd[2],in_data.reserverd[3],in_data.reserverd[4],in_data.reserverd[5],\
		in_data.reserverd[6],in_data.reserverd[7]);
	chip->at_color_store_calibrate_state(chip, &in_data);

	return size;
}

static ssize_t at_color_calibrate_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	at_color_sensor_output_para out_data ={0};
	struct colorDriver_chip *chip = NULL;
	int size = 1;
	if(NULL == dev || NULL == attr || NULL == buf)
	{
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}
	chip = dev_get_drvdata(dev);
	if(NULL == chip){
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}
	if(chip->at_color_show_calibrate_state == NULL){
		hwlog_err("[%s] at_color_show_calibrate_state NULL!! \n", __func__);
		return -1;
	}
	hwlog_info("[%s] in \n", __func__);
	size = sizeof(at_color_sensor_output_para);

	chip->at_color_show_calibrate_state(chip, &out_data);
	memcpy(buf, &out_data, size);
	hwlog_info("get cali result = %d, gain_arr=%d, color_array=%d\n", out_data.result, out_data.gain_arr, out_data.color_arr);
	hwlog_info("get cali gain = %d, %d, %d, %d %d\n", out_data.report_gain[0],out_data.report_gain[1],out_data.report_gain[2],\
		out_data.report_gain[3], out_data.report_gain[4]);
	return size;
}

static ssize_t color_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct colorDriver_chip *chip = NULL;
	int state;

	if(NULL == dev || NULL == attr || NULL == buf)
	{
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if(NULL == chip){
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}
	if (chip->color_enable_show_state == NULL) {
		hwlog_err("[%s] color_enable_show_state NULL!! \n", __func__);
		return -1;
	}
	chip->color_enable_show_state(chip, &state);

	return snprintf(buf, ONE_SHOW_LEN, "%d\n", state);
}
static ssize_t color_enable_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t size)
{
	struct colorDriver_chip *chip = NULL;
	bool state = true;

	if(NULL == dev || NULL == attr || NULL == buf)
	{
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if(NULL == chip){
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}

	if (strtobool(buf, &state)) {
		hwlog_err("[%s] Failed to strtobool enable state.\n", __func__);
		return -EINVAL;
	}
	if (chip->color_enable_store_state == NULL) {
		hwlog_err("[%s] color_enable_store_state NULL!! \n", __func__);
		return -1;
	}

	chip->color_enable_store_state(chip, (int)state);

	return size;
}
static ssize_t flicker_enable_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t size)
{
	struct colorDriver_chip *chip = NULL;
	long state = 0;

	if(NULL == dev || NULL == attr || NULL == buf)
	{
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if(NULL == chip){
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}
	if (strict_strtol(buf, ONE_SHOW_LEN, &state)){
		hwlog_err("[%s] Failed to strtobool enable state.\n", __func__);
		return -EINVAL;
	}
	if(chip->flicker_enable_store_state == NULL){
		hwlog_err("[%s] flicker_enable_store_state NULL!! \n", __func__);
		return -1;
	}
	if (g_flicker_support == 0) {
		hwlog_err("%s not support flicker\n", __func__);
		return -1;
	}
	hwlog_info("%s state = %d\n", __func__, (int)state);

	chip->flicker_enable_store_state(chip, (int)state);
	return size;
}

static ssize_t flicker_data_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{

	struct colorDriver_chip *chip = NULL;
	if(NULL == dev || NULL == attr || NULL == buf)
	{
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}
	chip = dev_get_drvdata(dev);
	if(NULL == chip){
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}
	if(chip->get_flicker_data == NULL){
		hwlog_err("[%s] get_flicker_data NULL!! \n", __func__);
		return -1;
	}

	chip->get_flicker_data(chip, buf);

	return MAX_FLICK_DATA_LEN;//max report flicker size
}

static ssize_t calibrate_timeout_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if(NULL == dev || NULL == attr || NULL == buf)
	{
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}

	return snprintf(buf, PAGE_SIZE, "%d\n", TIME_OUT_DEFAULT);
}

static ssize_t color_gain_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct colorDriver_chip *chip = NULL;
	int gain;

	if(NULL == dev || NULL == attr || NULL == buf)
	{
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if(NULL == chip){
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}
	if (chip->color_sensor_getGain == NULL) {
		hwlog_err("[%s] get_flicker_data NULL!! \n", __func__);
		return -1;
	}

	gain = chip->color_sensor_getGain(chip->deviceCtx);

	return snprintf(buf, PAGE_SIZE, "%d\n", gain);
}
static ssize_t color_gain_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct colorDriver_chip *chip = NULL;
	unsigned long value = 0L;
	int gain_value = 0;

	if(NULL == dev || NULL == attr || NULL == buf)
	{
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if(NULL == chip){
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}
	if (chip->color_sensor_setGain == NULL) {
		hwlog_err("[%s] color_sensor_setGain NULL!! \n", __func__);
		return -1;
	}
	if (strict_strtol(buf, ONE_SHOW_LEN, &value))
		return -EINVAL;
	gain_value = (int)value;
	chip->color_sensor_setGain(chip->deviceCtx, gain_value);
	return size;
}

static ssize_t color_data_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct colorDriver_chip *chip = NULL;

	if(NULL == dev || NULL == attr || NULL == buf)
	{
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if(NULL == chip){
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}
	//color_report_val
	memcpy(buf, color_report_val, MAX_REPORT_LEN*sizeof(int));
	hwlog_err("color_report_val = %d, %d, %d, %d, %d\n", color_report_val[0], color_report_val[1], color_report_val[2], color_report_val[3], color_report_val[4]);
	return MAX_REPORT_LEN*sizeof(int);
}

static ssize_t report_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
    struct colorDriver_chip *chip = NULL;
    int report_type;

    if(NULL == dev || NULL == attr || NULL == buf)
    {
        hwlog_err("[%s] input NULL!! \n", __func__);
        return -1;
    }

    chip = dev_get_drvdata(dev);
    if(NULL == chip){
        hwlog_err("[%s] input NULL!! \n", __func__);
        return snprintf(buf, PAGE_SIZE, "%d\n", AWB_SENSOR_RAW_SEQ_TYPE_INVALID);
    }

    if(chip->color_report_type == NULL){
	hwlog_err("%s, color_report_type NULL , return invalid report type\n", __func__);
    	return snprintf(buf, PAGE_SIZE, "%d\n", AWB_SENSOR_RAW_SEQ_TYPE_INVALID);
    }

    report_type = chip->color_report_type();
    if((report_type > AWB_SENSOR_RAW_SEQ_TYPE_INVALID)
        && (report_type < AWB_SENSOR_RAW_SEQ_TYPE_MAX)){
        hwlog_info("%s, color report sensor type = %d\n", __func__, report_type);
        return snprintf(buf, PAGE_SIZE, "%d\n", report_type);
    }

    return snprintf(buf, PAGE_SIZE, "%d\n", AWB_SENSOR_RAW_SEQ_TYPE_INVALID);
}

static ssize_t color_name_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct colorDriver_chip *chip = NULL;
	unsigned int len;
	char *color_name = NULL;

	if (!dev || !attr || !buf) {
		hwlog_err("%s input NULL\n", __func__);
		return -EFAULT;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("%s input NULL\n", __func__);
		return -EFAULT;
	}

	if (!chip->color_chip_name)
		goto unsupport_rgb;

	color_name = chip->color_chip_name();
	if (color_name == NULL)
		goto unsupport_rgb;

	len = strlen(color_name);
	hwlog_err("get color name len = %d\n", len);
	if (len >= MAX_NAME_STR_LEN)
		goto unsupport_rgb;

	return snprintf(buf, MAX_NAME_STR_LEN, "%s\n", color_name);

unsupport_rgb:
	return snprintf(buf, MAX_NAME_STR_LEN, "%s\n", "unsupport");
}


DEVICE_ATTR(calibrate, 0660, color_calibrate_show, color_calibrate_store);
DEVICE_ATTR(color_enable, 0660, color_enable_show, color_enable_store);
DEVICE_ATTR(gain, 0660, color_gain_show, color_gain_store);
DEVICE_ATTR(calibrate_timeout, 0440, calibrate_timeout_show, NULL);
DEVICE_ATTR(color_data, 0660, color_data_show, NULL);
DEVICE_ATTR(color_cali, 0660, at_color_calibrate_show, at_color_calibrate_store);
DEVICE_ATTR(flicker_enable, 0660, NULL, flicker_enable_store);
DEVICE_ATTR(flicker_data, 0660, flicker_data_show, NULL);
DEVICE_ATTR(report_type, 0660, report_type_show, NULL);
DEVICE_ATTR(color_name, 0660, color_name_show, NULL);

static struct attribute *color_sensor_attributes[] = {
	&dev_attr_calibrate.attr,
	&dev_attr_color_enable.attr,
	&dev_attr_gain.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_color_data.attr,
	&dev_attr_color_cali.attr,
	&dev_attr_flicker_enable.attr,
	&dev_attr_flicker_data.attr,
	&dev_attr_report_type.attr,
	&dev_attr_color_name.attr,
	NULL,
};
static const struct attribute_group color_sensor_attr_group = {
	.attrs = color_sensor_attributes,
};

static const struct attribute_group *color_sensor_attr_groups[] = {
	&color_sensor_attr_group,
	NULL,
};

int color_register(struct colorDriver_chip *chip)
{
	if(NULL == chip)
	{
		hwlog_err("[%s] input NULL!! \n", __func__);
		return -1;
	}
		
	chip->dev = device_create(color_sensor_class, NULL, 0, chip, "color_sensor");
	if (NULL == chip->dev) {
		hwlog_err("[%s] Failed to create color_sensor dev.", __func__);
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(color_register);

void color_notify_absent(void)
{
	if (color_priv_data.rgb_support == 0)
		return;

	// set ++ when rgb absent but config sensor support
	color_priv_data.rgb_absent++;
}

void color_notify_support(void)
{
	static bool sensor_dmd_check;
	color_priv_data.rgb_support++; // support rgb sensor for this product
	// start a work for check whether rgb sensor is exist
#ifdef CONFIG_HUAWEI_DSM
	if (sensor_dmd_check)
		return;
	INIT_DELAYED_WORK(&color_sensor_dmd_work, color_dmd_work);
	// check dmd after 30s
	schedule_delayed_work(&color_sensor_dmd_work, msecs_to_jiffies(30000));
	sensor_dmd_check = true;
#endif
}

void color_unregister(struct colorDriver_chip *chip)
{
	device_destroy(color_sensor_class, 0);
}
EXPORT_SYMBOL_GPL(color_unregister);

int (*color_default_enable)(bool enable) = NULL;
int color_sensor_enable(bool enable){
	if(color_default_enable == NULL){
		hwlog_err("ERR PARA\n");
		return 0;
	 }
	return color_default_enable(enable);
}
EXPORT_SYMBOL_GPL(color_sensor_enable);

static int color_sensor_init(void)
{
	color_sensor_class = class_create(THIS_MODULE, "ap_sensor");
	if (IS_ERR(color_sensor_class))
		return PTR_ERR(color_sensor_class);
	color_sensor_class->dev_groups = color_sensor_attr_groups;

	hwlog_info("[%s]color_sensor init\n", __func__);
	return 0;
}

static void color_sensor_exit(void)
{
	class_destroy(color_sensor_class);
}

subsys_initcall(color_sensor_init);
module_exit(color_sensor_exit);

MODULE_AUTHOR("Huawei");
MODULE_DESCRIPTION("Color class init");
MODULE_LICENSE("GPL");
