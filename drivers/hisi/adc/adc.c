/*
 * adc.c
 *
 * for the hkadc driver.
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include <linux/hisi/hisi_adc.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/notifier.h>
#include <linux/delay.h>
#include <linux/completion.h>
#include <linux/mutex.h>
#include <linux/debugfs.h>
#include <linux/hisi/hisi_rproc.h>
#include <securec.h>
#define MODULE_NAME		"adc"
/* adc maybe IPC timeout */
#define ADC_IPC_TIMEOUT		1500
#define ADC_IPC_MAX_CNT		3
#define DEBUG_ON		1

#define AP_ID			0x00
#define ADC_OBJ_ID		0x0b
#define ADC_IPC_CMD_TYPE0	0x02
#define ADC_IPC_CMD_TYPE1	0x0c
#define ADC_IPC_DATA		((AP_ID << 24) | (ADC_OBJ_ID << 16) | \
				 (ADC_IPC_CMD_TYPE0 << 8) | ADC_IPC_CMD_TYPE1)
#define HADC_IPC_RECV_HEADER	((0x08 << 24) | (ADC_OBJ_ID << 16) | \
				 (ADC_IPC_CMD_TYPE0 << 8) | ADC_IPC_CMD_TYPE1)
#define adc_data_bit_r(d, n)	((d) >> (n))
#define ADC_DATA_MASK		0xff
#define ADC_RESULT_ERR		(-EINVAL)
#define ADC_CHN_MAX		18
#define ADC_RET_BIT		8
#define ADC_VALUE_BIT		16

#define ADC_CHANNEL_XOADC	15
#define VREF_VOLT		1800
#define AD_RANGE		32767

#define HKADC_VREF		1800
#define HKADC_ACCURACY		0xFFF
#define ADC_IPC_CMD_TYPE_CURRENT	0x14
#define ADC_IPC_CURRENT		((AP_ID << 24) | (ADC_OBJ_ID << 16) | \
				 (ADC_IPC_CMD_TYPE0 << 8) | \
				 ADC_IPC_CMD_TYPE_CURRENT)
#define HADC_IPC_RECV_CURRENT_HEADER	((0x08 << 24) | (ADC_OBJ_ID << 16) | \
					 (ADC_IPC_CMD_TYPE0 << 8) | \
					 ADC_IPC_CMD_TYPE_CURRENT)

enum {
	ADC_IPC_CMD_TYPE = 0,
	ADC_IPC_CMD_CHANNEL,
	ADC_IPC_CMD_LEN
};

struct adc_info {
	int channel;
	int value;
};

struct adc_device {
	struct adc_info info;
	mbox_msg_t tx_msg[ADC_IPC_CMD_LEN];
	struct notifier_block *nb;
	struct mutex mutex; /* Mutex for devices */
	struct completion completion;
};

static struct adc_device *g_adc_dev;

int g_hkadc_debug;

void hkadc_debug(int onoff)
{
	g_hkadc_debug = onoff;
}

/* notifiers AP when LPM3 sends msgs */
static int adc_dev_notifier(struct notifier_block *nb,
			    unsigned long len, void *msg)
{
	u32 *_msg = (u32 *)msg;
	unsigned long i;

	if (g_hkadc_debug == DEBUG_ON) {
		for (i = 0; i < len; i++)
			pr_info("%s_debug:[notifier] msg[%lu] = 0x%x\n",
				MODULE_NAME, i, _msg[i]);
	}

	if (_msg[0] != HADC_IPC_RECV_HEADER &&
	    _msg[0] != HADC_IPC_RECV_CURRENT_HEADER)
		return 0;

	if ((adc_data_bit_r(_msg[1], ADC_RET_BIT) & ADC_DATA_MASK) != 0) {
		g_adc_dev->info.value = ADC_RESULT_ERR;
		complete(&g_adc_dev->completion);
		pr_err("%s:ret error msg[1][0x%x]\n", MODULE_NAME, _msg[1]);
		return 0;
	}
	if ((_msg[1] & ADC_DATA_MASK) == g_adc_dev->info.channel) {
		g_adc_dev->info.value = adc_data_bit_r(_msg[1],
							    ADC_VALUE_BIT);
		pr_debug("%s: msg[1][0x%x]\n", MODULE_NAME, _msg[1]);
		complete(&g_adc_dev->completion);
	} else {
		pr_err("%s:channel error msg[1][0x%x]\n", MODULE_NAME, _msg[1]);
	}

	return 0;
}

/* AP sends msgs to LPM3 for adc sampling&converting. */
static int adc_send_ipc_to_lpm3(int channel, int ipc_header)
{
	int loop = ADC_IPC_MAX_CNT;
	int ret;

	if (channel > ADC_CHN_MAX) {
		pr_err("%s: invalid channel!\n", MODULE_NAME);
		ret = -EINVAL;
		return ret;
	}

	if (g_adc_dev == NULL) {
		pr_err("%s: adc dev is not initialized yet!\n", MODULE_NAME);
		ret = -ENODEV;
		return ret;
	}

	g_adc_dev->tx_msg[ADC_IPC_CMD_TYPE] = (mbox_msg_t)ipc_header;
	g_adc_dev->info.channel = channel;
	g_adc_dev->tx_msg[ADC_IPC_CMD_CHANNEL] = (mbox_msg_t)channel;

	do {
		ret = RPROC_ASYNC_SEND(HISI_ACPU_LPM3_MBX_4,
				       (mbox_msg_t *)g_adc_dev->tx_msg,
				       ADC_IPC_CMD_LEN);
		loop--;
	} while (ret == -ENOMEM && loop > 0);
	if (ret != 0) {
		pr_err("%s:fail to send msg,ret = %d!\n", MODULE_NAME, ret);
		goto err_msg_async;
	}

	return ret;

err_msg_async:
	g_adc_dev->info.channel = ADC_RESULT_ERR;
	return ret;
}

/*
 * Function name:adc_to_volt.
 * Discription:calculate volt from hkadc.
 * Parameters:
 *      @ adc
 * return value:
 *      @ volt(mv): negative-->failed, other-->succeed.
 */
int adc_to_volt(int adc)
{
	int volt;

	if (adc < 0)
		return ADC_RESULT_ERR;
	volt = adc * HKADC_VREF / HKADC_ACCURACY;
	return volt;
}

int xoadc_to_volt(int adc)
{
	int volt;

	if (adc < 0)
		return ADC_RESULT_ERR;
	volt = adc * VREF_VOLT / AD_RANGE;
	return volt;
}

/*
 * Function name:adc_get_value.
 * Discription:get volt from hkadc.
 * Parameters:
 *      @ adc_channel
 * return value:
 *      @ channel volt(mv): negative-->failed, other-->succeed.
 */
int hisi_adc_get_value(int adc_channel)
{
	int ret;
	int volt;

	ret = hisi_adc_get_adc(adc_channel);

	if (ret < 0)
		return ret;

	if (adc_channel == ADC_CHANNEL_XOADC)
		volt = xoadc_to_volt(ret);
	else
		volt = adc_to_volt(ret);

	return volt;
}
EXPORT_SYMBOL(hisi_adc_get_value);

static int adc_send_wait(int adc_channel, int ipc_header)
{
	int ret;

	reinit_completion(&g_adc_dev->completion);
	ret = adc_send_ipc_to_lpm3(adc_channel, ipc_header);
	if (ret != 0)
		return ret;

	ret = wait_for_completion_timeout(&g_adc_dev->completion,
					  msecs_to_jiffies(ADC_IPC_TIMEOUT));
	if (ret == 0) {
		pr_err("%s: channel-%d timeout!\n", MODULE_NAME, adc_channel);
		ret =  -ETIMEOUT;
	} else {
		ret = 0;
	}
	return ret;
}

/*
 * Function name:adc_get_adc.
 * Discription:get adc value from hkadc.
 * Parameters:
 *      @ adc_channel
 * return value:
 *      @ adc value(12 bits): negative-->failed, other-->succeed.
 */
int hisi_adc_get_adc(int adc_channel)
{
	int ret, value;

	mutex_lock(&g_adc_dev->mutex);

	ret = adc_send_wait(adc_channel, ADC_IPC_DATA);
	value = g_adc_dev->info.value;
	g_adc_dev->info.channel = ADC_RESULT_ERR;

	mutex_unlock(&g_adc_dev->mutex);
	if (g_hkadc_debug == DEBUG_ON)
		pr_info("%s value %d;ret %d\n", __func__, value, ret);

	return ret ? ret : value;
}
EXPORT_SYMBOL(hisi_adc_get_adc);

int hisi_adc_get_current(int adc_channel)
{
	int ret, value;

	mutex_lock(&g_adc_dev->mutex);

	ret = adc_send_wait(adc_channel, ADC_IPC_CURRENT);
	value = g_adc_dev->info.value;
	g_adc_dev->info.channel = ADC_RESULT_ERR;

	mutex_unlock(&g_adc_dev->mutex);

	return ret ? ret : value;
}
EXPORT_SYMBOL(hisi_adc_get_current);

static void adc_init_device_debugfs(void)
{
}

static void adc_remove_device_debugfs(void)
{
}

/* adc init function */
static int __init adc_driver_init(void)
{
	int ret = 0;

	g_adc_dev = kzalloc(sizeof(struct adc_device), GFP_KERNEL);
	if (g_adc_dev == NULL) {
		ret = -ENOMEM;
		goto err_adc_dev;
	}

	g_adc_dev->nb = kzalloc(sizeof(struct notifier_block), GFP_KERNEL);
	if (g_adc_dev->nb == NULL) {
		ret = -ENOMEM;
		goto err_adc_nb;
	}

	g_adc_dev->nb->notifier_call = adc_dev_notifier;

	/* register the rx notify callback */
	ret = RPROC_MONITOR_REGISTER(HISI_LPM3_ACPU_MBX_1, g_adc_dev->nb);
	if (ret) {
		pr_info("%s:RPROC_MONITOR_REGISTER failed", __func__);
		goto err_get_mbox;
	}

	g_adc_dev->tx_msg[ADC_IPC_CMD_TYPE] = ADC_IPC_DATA;
	g_adc_dev->tx_msg[ADC_IPC_CMD_CHANNEL] = (mbox_msg_t)ADC_RESULT_ERR;

	mutex_init(&g_adc_dev->mutex);
	init_completion(&g_adc_dev->completion);
	adc_init_device_debugfs();

	pr_info("%s: init ok!\n", MODULE_NAME);
	return ret;

err_get_mbox:
	kfree(g_adc_dev->nb);
	g_adc_dev->nb = NULL;
err_adc_nb:
	kfree(g_adc_dev);
	g_adc_dev = NULL;
err_adc_dev:
	return ret;
}

/* adc exit function */
static void __exit adc_driver_exit(void)
{
	if (g_adc_dev != NULL) {
		kfree(g_adc_dev->nb);
		g_adc_dev->nb = NULL;

		kfree(g_adc_dev);
		g_adc_dev = NULL;
	}
	adc_remove_device_debugfs();
}

subsys_initcall(adc_driver_init);
module_exit(adc_driver_exit);
