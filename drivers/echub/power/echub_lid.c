/*
 * Button.c
 *
 * Powerbutton/LID Button driver for echub
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pm_wakeup.h>
#include <asm/irq.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/i2c.h>
#include <linux/timer.h>
#include "../echub.h"
#include "../echub_i2c.h"

#ifdef CONFIG_HUAWEI_ECHUB_LID_TEST
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#endif

#define TRUE                           (1)
#define FALSE                          (0)

#define GPIO_KEY_PRESS                 (1)
#define GPIO_KEY_RELEASE               (0)

#define GPIO_HIGH_VOLTAGE              (1)
#define GPIO_LOW_VOLTAGE               (0)
#define TIMER_DEBOUNCE                 (900)
#define WAKE_DEBOUNCE                  (1000)

#define EC_ADDR   0x38
#define EC_LID_FLAG_ADDR   0x0479

#ifdef CONFIG_HUAWEI_ECHUB_LID_TEST
#define DEVICE_NAME                 "lid"
#define LID_MAGIC                   0xEF
#define OPEN_LID                    (_IO(LID_MAGIC, 0x1))
#define CLOSE_LID                   (_IO(LID_MAGIC, 0x0))
#define READ_LID_STATUS             (_IO(LID_MAGIC, 0x2))
#define READ_SLEEP_COUNT            (_IO(LID_MAGIC, 0x3))
#endif

static struct wakeup_source lid_lock;

struct hw_gpio_key {
	struct input_dev        *input_dev;
	struct delayed_work     lid_work;
	struct timer_list       key_lid_timer;
	int                     gpio_lid;
	int                     lid_irq;

	struct pinctrl          *pctrl;
	struct pinctrl_state    *pins_default;
	struct pinctrl_state    *pins_idle;
};

#ifdef CONFIG_HUAWEI_ECHUB_LID_TEST
static struct hw_gpio_key *g_hw_gpio_key = NULL;
static int dis_irq_flag = 0;
extern int g_sleep_count;
#endif

int echub_i2c_write_to_ec(char devaddr, int regoffset, char regdata)
{
	struct i2c_adapter *adap;
	int ret;
	struct i2c_msg msg[2];
	unsigned char data[4];
	unsigned char value[2];
	adap = i2c_get_adapter(ECHUB_I2C_ADAPTER);
	if (!adap)
		return -ENODEV;

	/* write data buffer */
	msg[0].addr = devaddr;
	msg[0].flags = 0;
	msg[0].len = ECHUB_SEND_BYTE_LEN;
	msg[0].buf = data;

	data[0] = (u8)(regoffset >> BIT_OFFSET);
	data[1] = (u8)(regoffset & 0xFF);
	data[2] = REG_DATE_LEN;
	data[3] = regdata;

	/* read data buffer */
	msg[1].addr = devaddr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = ECHUB_SEND_TRANS_LEN;
	msg[1].buf = value;

	ret = i2c_transfer(adap, msg, ECHUB_SEND_TRANS_LEN);

	/* check i2c transfer result */
	if (ret != ECHUB_SEND_TRANS_LEN)
		return -ENODEV;

	return 0;
}

int echub_i2c_read_from_ec(char devaddr, int regoffset, char *value, int len)
{
	struct i2c_adapter *adap;
	int ret;
	struct i2c_msg msg[2];
	unsigned char data[2];
	adap = i2c_get_adapter(ECHUB_I2C_ADAPTER);
	if (!adap)
		return -ENODEV;

	/* write data buffer */
	msg[0].addr = devaddr;
	msg[0].flags = 0;
	msg[0].len = ECHUB_SEND_BYTE_LEN;
	msg[0].buf = data;

	data[0] = (u8)(regoffset >> BIT_OFFSET);
	data[1] = (u8)(regoffset & 0xFF);

	/* read data buffer */
	msg[1].addr = devaddr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = len;
	msg[1].buf = value;

	ret = i2c_transfer(adap, msg, ECHUB_SEND_TRANS_LEN);

	/* check i2c transfer result */
	if (ret != ECHUB_SEND_TRANS_LEN)
		return -ENODEV;

	if (value[1] != REG_DATE_LEN)
		return -EINVAL;

	return 0;
}

/**********************************************************
*  Function:       of_get_key_gpio
*  Discription:    get gpio property
*  Parameters:     struct device_node *np, const char *propname,
				   int prop_index, int gpio_index, enum of_gpio_flags *flags
*  return value:   0-sucess or others-fail
**********************************************************/
static int of_get_key_gpio(struct device_node *np, const char *propname,
					int prop_index, int gpio_index, enum of_gpio_flags *flags)
{
	int ret = 0;

#ifdef CONFIG_GPIO_LOGIC
	ret = of_get_gpio_by_prop(np, propname, prop_index, gpio_index, flags);
#else
	ret = of_get_named_gpio(np, propname, prop_index);
#endif

	return ret;
}

/**********************************************************
*  Function:       hw_gpio_key_open
*  Discription:    open gpiokey
*  Parameters:     input_dev *dev
*  return value:   0-sucess or others-fail
**********************************************************/
static int hw_gpio_key_open(struct input_dev *dev)
{
	/* add input device open function */
	echub_info("%s\n", __func__);
	return 0;
}

/**********************************************************
*  Function:       hw_gpio_key_close
*  Discription:    close gpiokey
*  Parameters:     input_dev *dev
*  return value:   0-sucess or others-fail
**********************************************************/
static void hw_gpio_key_close(struct input_dev *dev)
{
	/* add input device close function */
	echub_info("%s\n", __func__);
}

/**********************************************************
*  Function:       hw_lid_work
*  Discription:    lid main work
*  Parameters:     work_struct *work
*  return value:   0-sucess or others-fail
**********************************************************/
static void hw_lid_work(struct work_struct *work)
{
	struct hw_gpio_key *gpio_key = container_of(work,
		struct hw_gpio_key, lid_work.work);

	unsigned int keyup_value = 0;
	unsigned int report_action = GPIO_KEY_RELEASE;

	keyup_value = gpio_get_value((unsigned int)gpio_key->gpio_lid);
	/*judge key is pressed or released.*/
	if (keyup_value == GPIO_LOW_VOLTAGE) {
		report_action = GPIO_KEY_PRESS;
	} else if (keyup_value == GPIO_HIGH_VOLTAGE) {
		report_action = GPIO_KEY_RELEASE;
	} else {
		echub_err("[gpiokey][%s]invalid gpio key_value.\n", __func__);
		return;
	}

	input_report_switch(gpio_key->input_dev, SW_LID, report_action);
	input_sync(gpio_key->input_dev);
}

/**********************************************************
*  Function:       gpio_lid_timer
*  Discription:    gpio timer
*  Parameters:     long data
*  return value:   void
**********************************************************/
static void gpio_lid_timer(struct timer_list *t)
{
	int keyup_value;
	struct hw_gpio_key *gpio_key = from_timer(gpio_key, t, key_lid_timer);

	keyup_value = gpio_get_value((unsigned int)gpio_key->gpio_lid);
	schedule_delayed_work(&(gpio_key->lid_work), 0);
}

/**********************************************************
*  Function:       hw_gpio_key_irq_handler
*  Discription:    gpio irq handler
*  Parameters:     int irq, void *dev_id
*  return value:   void
**********************************************************/
static irqreturn_t hw_gpio_key_irq_handler(int irq, void *dev_id)
{
	struct hw_gpio_key *gpio_key = (struct hw_gpio_key *)dev_id;
	int key_value = 0;

	if (irq == gpio_key->lid_irq) {
		mod_timer(&(gpio_key->key_lid_timer), jiffies + msecs_to_jiffies(TIMER_DEBOUNCE));
		key_value = gpio_get_value((unsigned int)gpio_key->gpio_lid);
	} else {
		echub_err("[gpiokey] [%s]invalid irq %d!\n", __func__, irq);
	}
	return IRQ_HANDLED;
}

/**********************************************************
*  Function:       set_lid_status
*  Discription:    Enable or disable the LID irq based on the MFG mode and LID flag.
*  Parameters:     struct hw_gpio_key *gpio_key
*  return value:   0-sucess or others-fail
**********************************************************/
static int set_lid_status(struct hw_gpio_key *gpio_key)
{
	int ret = 0;
	char value[4]={0};

	/* Enter MFG test mode */
	ret = echub_i2c_write_to_ec(0x38, 0x0408, 1);
	if (ret) {
		echub_err("Enter MFG test mode failed: %d\n", ret);
		return ret;
	}

	/* Read MFG mode */
	ret = echub_i2c_read_from_ec(0x38, 0x0489, value, 4);
	if (ret)
		echub_err("Read MFG mode failed: %d\n", ret);

	echub_info("Read MFG mode: ret = %d value = %*ph\n", ret, 4, value);

	/* Exist MFG test mode */
	ret = echub_i2c_write_to_ec(0x38, 0x0408, 0);
	if (ret)
		echub_err("Exist MFG test mode failed: %d\n", ret);

	if (value[2] == 1) {
		echub_info("The current mode is MFG mode\n");
		/* Read lid flag */
		ret = echub_i2c_read_from_ec(0x38, 0x04e9, value, 4);
		echub_info("Get lid flag form EC. ret = %d value = %*ph\n", ret, 4, value);
		if (ret) {
			echub_info("Read lid flag failed: %d\n", ret);
			return ret;
		}
		if (value[2] == 1) {
			echub_info("Disable lid irq\n");
			disable_irq(gpio_key->lid_irq);
#ifdef CONFIG_HUAWEI_ECHUB_LID_TEST
			dis_irq_flag = 1;
#endif
		}
	}

	return ret;
}

#ifdef CONFIG_OF
static const struct of_device_id hw_gpio_key_match[] = {
	{ .compatible = "huawei,echub-lid" },
	{},
};
MODULE_DEVICE_TABLE(of, hw_gpio_key_match);
#endif

/**********************************************************
*  Function:       hw_gpio_key_probe
*  Discription:    module probe
*  Parameters:     pdev:platform_device
*  return value:   0-sucess or others-fail
**********************************************************/
static int hw_gpio_key_probe(struct platform_device *pdev)
{
	struct hw_gpio_key *gpio_key = NULL;
	struct input_dev *input_dev = NULL;
	enum of_gpio_flags flags;
	int err = 0;

	if (pdev == NULL) {
		echub_err("[gpiokey]parameter error!\n");
		return -EINVAL;
	}

	echub_info("huawei gpio key driver probes start!\n");
#ifdef CONFIG_OF
	if (!of_match_node(hw_gpio_key_match, pdev->dev.of_node)) {
		echub_err("dev node is not match. exiting.\n");
		return -ENODEV;
	}
#endif

	gpio_key = devm_kzalloc(&pdev->dev, sizeof(struct hw_gpio_key), GFP_KERNEL);
	if (gpio_key == NULL) {
		echub_info("Failed to allocate struct hw_gpio_key!\n");
		return -ENOMEM;
	}

	input_dev = input_allocate_device();
	if (input_dev == NULL) {
		echub_err("Failed to allocate struct input_dev!\n");
		return -ENOMEM;
	}

	input_dev->name = pdev->name;
	input_dev->id.bustype = BUS_HOST;
	input_dev->dev.parent = &pdev->dev;
	input_set_drvdata(input_dev, gpio_key);

	set_bit(EV_SW, input_dev->evbit);
	set_bit(EV_SYN, input_dev->evbit);
	set_bit(SW_LID, input_dev->swbit);

	input_dev->open = hw_gpio_key_open;
	input_dev->close = hw_gpio_key_close;

	gpio_key->input_dev = input_dev;

	/*initial work before we use it.*/
	INIT_DELAYED_WORK(&(gpio_key->lid_work), hw_lid_work);
	wakeup_source_init(&lid_lock, "key_lid_wake_lock");

	/* inital gpio_lid */
	err = of_property_read_u32(pdev->dev.of_node, "gpio-key-lid", (u32 *)&gpio_key->gpio_lid);
	if (err) {
		gpio_key->gpio_lid = 0;
		echub_info("%s: Not support gpio_key lid\n", __func__);
		goto err_get_gpio;
	} else {
		echub_info("%s: Support gpio_key lid: %d\n", __func__, gpio_key->gpio_lid);
	}

	if (gpio_is_valid(gpio_key->gpio_lid)) {
		err = devm_gpio_request(&pdev->dev, (unsigned int)gpio_key->gpio_lid, "gpio_lid");
		if (err < 0) {
			echub_err("Fail request gpio:%d\n", gpio_key->gpio_lid);
			goto err_get_gpio;
		}

		err = gpio_direction_input((unsigned int)gpio_key->gpio_lid);
		if (err < 0) {
			echub_err("Failed to set gpio_lid directoin!\n");
			goto err_get_gpio;
		}

		gpio_key->lid_irq = gpio_to_irq((unsigned int)gpio_key->gpio_lid);
		if (gpio_key->lid_irq < 0) {
			echub_err("Failed to get gpio key press irq!\n");
			err = gpio_key->lid_irq;
			goto err_gpio_to_irq;
		}
	} else {
		echub_err("%s: gpio of lid is not valid, check DTS\n", __func__);
	}

	gpio_key->pctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(gpio_key->pctrl)) {
		echub_err("failed to devm pinctrl get\n");
		err = -EINVAL;
		goto err_pinctrl;
	}

	timer_setup(&(gpio_key->key_lid_timer), gpio_lid_timer, 0);

	if (gpio_is_valid(gpio_key->gpio_lid)) {
		err = request_irq(gpio_key->lid_irq, hw_gpio_key_irq_handler, IRQF_NO_SUSPEND | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, pdev->name, gpio_key);
		if (err) {
			echub_err("Failed to request press interupt handler!\n");
			goto err_lid_irq_req;
		}
	}

	err = input_register_device(gpio_key->input_dev);
	if (err) {
		echub_err("Failed to register input device!\n");
		goto err_register_dev;
	}

	device_init_wakeup(&pdev->dev, TRUE);
	platform_set_drvdata(pdev, gpio_key);

#ifdef CONFIG_HUAWEI_ECHUB_LID_TEST
	g_hw_gpio_key = gpio_key;
#endif
	err = set_lid_status(gpio_key);
	if (err)
		echub_info("Can't set lid status\n");

	echub_info("huawei gpio key driver probes successfully!\n");
	return 0;

err_register_dev:
	free_irq(gpio_key->lid_irq, gpio_key);
err_lid_irq_req:
err_pinctrl_put:
	devm_pinctrl_put(gpio_key->pctrl);
err_pinctrl:
err_gpio_to_irq:
err_get_gpio:
	input_free_device(input_dev);
	wakeup_source_trash(&lid_lock);
	echub_err("[gpiokey] gpio key probe failed! ret = %d.\n", err);
	return err;
}

/**********************************************************
*  Function:       hw_gpio_key_remove
*  Discription:    module remove
*  Parameters:     pdev:platform_device
*  return value:   0-sucess or others-fail
**********************************************************/
static int hw_gpio_key_remove(struct platform_device *pdev)
{
	struct hw_gpio_key *gpio_key = platform_get_drvdata(pdev);

	if (gpio_key == NULL) {
		echub_err("get invalid gpio_key pointer\n");
		return -EINVAL;
	}

	free_irq(gpio_key->lid_irq, gpio_key);
	gpio_free((unsigned int)gpio_key->gpio_lid);
	devm_pinctrl_put(gpio_key->pctrl);
	cancel_delayed_work(&(gpio_key->lid_work));
	wakeup_source_trash(&lid_lock);

	input_unregister_device(gpio_key->input_dev);
	platform_set_drvdata(pdev, NULL);

	kfree(gpio_key);
	gpio_key = NULL;
	return 0;
}

#ifdef CONFIG_PM
/**********************************************************
*  Function:       hw_gpio_key_suspend
*  Discription:    module remove
*  Parameters:     pdev:platform_device pm_message_t state
*  return value:   0-sucess or others-fail
**********************************************************/
static int hw_gpio_key_suspend(struct platform_device *pdev, pm_message_t state)
{
	int err;
	struct hw_gpio_key *gpio_key = platform_get_drvdata(pdev);

	echub_info("%s: suspend +\n", __func__);
	irq_set_irq_type(gpio_key->lid_irq, IRQF_TRIGGER_RISING);

	echub_info("%s: suspend -\n", __func__);
	return 0;
}

/**********************************************************
*  Function:       hw_gpio_key_resume
*  Discription:    module remove
*  Parameters:     pdev:platform_device
*  return value:   0-sucess or others-fail
**********************************************************/
static int hw_gpio_key_resume(struct platform_device *pdev)
{
	int err;
	struct hw_gpio_key *gpio_key = platform_get_drvdata(pdev);

	echub_info("%s: resume +\n", __func__);
	irq_set_irq_type(gpio_key->lid_irq, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING);

	echub_info("%s: resume -\n", __func__);
	return 0;
}
#endif

struct platform_driver hw_gpio_key_driver = {
	.probe = hw_gpio_key_probe,
	.remove = hw_gpio_key_remove,
	.driver = {
		.name = "hw_gpio_key",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hw_gpio_key_match),
	},
#ifdef CONFIG_PM
	.suspend = hw_gpio_key_suspend,
	.resume = hw_gpio_key_resume,
#endif
};

#ifdef CONFIG_HUAWEI_ECHUB_LID_TEST
static int open_lid(struct hw_gpio_key *gpio_key)
{
	int ret;
	if (!dis_irq_flag)
		return -1;
	enable_irq(gpio_key->lid_irq);

	ret = echub_i2c_write_to_ec(EC_ADDR, EC_LID_FLAG_ADDR, 0);
	if (ret)
		printk("write 0x0 to EC failed\n");

	return 0;
}

static int close_lid(struct hw_gpio_key *gpio_key)
{
	int ret;
	if (dis_irq_flag)
		return -1;
	disable_irq(gpio_key->lid_irq);
	ret = echub_i2c_write_to_ec(EC_ADDR, EC_LID_FLAG_ADDR, 1);
	if (ret)
		printk("write 0x1 to EC failed\n");
	return 0;
}

static int read_lid_status(struct hw_gpio_key *gpio_key)
{
	int lid_status;

	lid_status = gpio_get_value((unsigned int)gpio_key->gpio_lid);
	return !!lid_status;
}

static int read_sleep_count(void)
{
	return g_sleep_count;
}

static int lid_open(struct inode *inode, struct file *flip)
{
	flip->private_data = g_hw_gpio_key;
	return 0;
}

static int lid_release(struct inode *inode, struct file *flip)
{
	return 0;
}

static long lid_ioctl(struct file *flip, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	switch (cmd) {
	case OPEN_LID:
		ret = open_lid(g_hw_gpio_key);
		if (!ret)
			dis_irq_flag = 0;
		break;
	case CLOSE_LID:
		ret = close_lid(g_hw_gpio_key);
		if (!ret)
			dis_irq_flag = 1;
		break;
	case READ_LID_STATUS:
		ret = read_lid_status(g_hw_gpio_key);
		if (ret != 0 && ret != 1)
			echub_err("lid_dev: read lid status error\n");
		break;
	case READ_SLEEP_COUNT:
		ret = read_sleep_count();
		if (ret < 0)
			echub_err("read sleep count error\n");
		break;

	default:
		echub_err("lid_dev ioctl cmd error: open: 1; close: 0; read: 2\n");
		return -EINVAL;

	}
	return ret;
}

static struct file_operations lid_dev_fops = {
	.owner          = THIS_MODULE,
	.unlocked_ioctl = lid_ioctl,
	.open			= lid_open,
	.release		= lid_release,
};

static struct miscdevice lid_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = DEVICE_NAME,
	.fops  = &lid_dev_fops,
};
#endif

/**********************************************************
*  Function:       hw_gpio_key_init
*  Discription:    module initialization
*  Parameters:     pdev:platform_device
*  return value:   0-sucess or others-fail
**********************************************************/
int __init hw_gpio_key_init(void)
{
	int ret;

	echub_info("%s\n", __func__);
	ret = platform_driver_register(&hw_gpio_key_driver);
	if (ret)
		echub_err("lid_dev misc_register err\n");

#ifdef CONFIG_HUAWEI_ECHUB_LID_TEST
	ret = misc_register(&lid_dev);
	if (ret)
		echub_err("lid_dev misc_register err\n");
#endif
	return ret;
}

/**********************************************************
*  Function:       hw_gpio_key_exit
*  Discription:    module exit
*  Parameters:     NULL
*  return value:   NULL
**********************************************************/
void __exit hw_gpio_key_exit(void)
{
	echub_info("%s\n", __func__);
#ifdef CONFIG_HUAWEI_ECHUB_LID_TEST
	misc_deregister(&lid_dev);
#endif
	platform_driver_unregister(&hw_gpio_key_driver);
}

module_init(hw_gpio_key_init);
module_exit(hw_gpio_key_exit);

MODULE_ALIAS("huawei echub button module");
MODULE_DESCRIPTION("huawei echub button driver");
MODULE_LICENSE("GPL");
