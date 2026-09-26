/*
 * vbus_channel_error_handle.c
 *
 * error handle for vbus channel driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/hardware_channel/vbus_channel_error_handle.h>
#include <chipset_common/hwpower/hardware_channel/vbus_channel.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG vbus_channel_eh
HWLOG_REGIST();

#define MAX_USB_STATE_CNT         5
#define VBUS_CH_EH_DMD_BUF_SIZE   128

static struct vbus_ch_eh_dev *g_vbus_ch_eh_dev;
static bool g_otg_sc_happen_flag;

static struct vbus_ch_eh_dev *vbus_ch_eh_get_dev(void)
{
	if (!g_vbus_ch_eh_dev) {
		hwlog_err("g_vbus_ch_eh_dev is null\n");
		return NULL;
	}

	return g_vbus_ch_eh_dev;
}

static int vbus_ch_eh_check_usb_state(struct vbus_ch_eh_dev *l_dev)
{
	if (power_platform_usb_state_is_host())
		l_dev->usb_state_count = 0;
	else
		l_dev->usb_state_count++;

	if (l_dev->usb_state_count >= MAX_USB_STATE_CNT) {
		hwlog_info("not in usb_state_host, close check work\n");
		return -1;
	}

	return 0;
}

static void vbus_ch_eh_dmd_report(enum vbus_ch_eh_dmd_type type,
	const void *data)
{
	char buff[VBUS_CH_EH_DMD_BUF_SIZE] = {0};

	if (!data) {
		hwlog_err("data is null\n");
		return;
	}

	switch (type) {
	case BOOST_GPIO_SC_DMD_REPORT:
		snprintf(buff, sizeof(buff), "vusb voltage: %d\n",
			*(int *)data);
		power_dsm_report_dmd(POWER_DSM_BATTERY,
			DSM_OTG_VBUS_SHORT, buff);
		break;
	case BOOST_GPIO_OTG_OCP_DMD_REPORT:
		snprintf(buff, sizeof(buff), "otg ocp: gpio%d\n",
			*(int *)data);
		power_dsm_report_dmd(POWER_DSM_BATTERY,
			DSM_OTG_VBUS_SHORT, buff);
		break;
	default:
		hwlog_err("type is invaild\n");
		break;
	}
}

bool vbus_ch_eh_get_otg_sc_flag(void)
{
	return g_otg_sc_happen_flag;
}

static void vbus_ch_eh_set_otg_sc_flag(bool flag)
{
	g_otg_sc_happen_flag = flag;
}

static void boost_gpio_otg_sc_check_work(struct work_struct *work)
{
	struct vbus_ch_eh_dev *l_dev = vbus_ch_eh_get_dev();
	struct otg_sc_info *info = NULL;
	struct otg_sc_para *para = NULL;
	int value = 0;

	if (!l_dev)
		return;

	info = &l_dev->boost_gpio_otg_sc_info;
	para = l_dev->boost_gpio_otg_sc_info.para;
	if (!info || !para) {
		hwlog_err("info or para is null\n");
		return;
	}

	if (power_platform_get_vusb_status(&value)) {
		hwlog_err("get vusb value failed\n");
		return;
	}

	if (value < (int)para->vol_mv) {
		info->fault_count++;
		hwlog_info("value=%d, fault_count=%u\n", value, info->fault_count);
	} else {
		info->fault_count = 0;
	}

	if (info->fault_count > para->check_times) {
		info->fault_count = 0;
		if (info->work_flag) {
			vbus_ch_eh_set_otg_sc_flag(true);
			if (power_platform_pogopin_is_support() &&
				power_platform_pogopin_otg_from_buckboost())
				vbus_ch_close(VBUS_CH_USER_PD,
					VBUS_CH_TYPE_POGOPIN_BOOST, true, true);
			else
				vbus_ch_close(VBUS_CH_USER_PD,
					VBUS_CH_TYPE_BOOST_GPIO, true, true);
			vbus_ch_eh_dmd_report(BOOST_GPIO_SC_DMD_REPORT, &value);
		}

		hwlog_err("otg short circuit happen\n");
		return;
	}

	if (vbus_ch_eh_check_usb_state(l_dev))
		return;

	if (info->work_flag)
		schedule_delayed_work(&info->work,
			msecs_to_jiffies(para->delayed_time));
}

static void boost_gpio_otg_sc_check_start(struct vbus_ch_eh_dev *l_dev,
	void *data)
{
	u32 delayed_time;

	if (!data) {
		hwlog_err("data is null\n");
		return;
	}

	l_dev->boost_gpio_otg_sc_info.para = (struct otg_sc_para *)data;
	delayed_time = l_dev->boost_gpio_otg_sc_info.para->delayed_time;
	l_dev->boost_gpio_otg_sc_info.work_flag = true;
	l_dev->usb_state_count = 0;
	cancel_delayed_work(&l_dev->boost_gpio_otg_sc_info.work);
	schedule_delayed_work(&l_dev->boost_gpio_otg_sc_info.work,
		msecs_to_jiffies(delayed_time));
}

static void boost_gpio_otg_sc_check_stop(struct vbus_ch_eh_dev *l_dev)
{
	l_dev->boost_gpio_otg_sc_info.fault_count = 0;
	l_dev->boost_gpio_otg_sc_info.work_flag = false;
	l_dev->usb_state_count = 0;
	cancel_delayed_work(&l_dev->boost_gpio_otg_sc_info.work);
	vbus_ch_eh_set_otg_sc_flag(false);
}

static void boost_gpio_otg_ocp_handle(struct vbus_ch_eh_dev *l_dev,
	void *data)
{
	if (power_platform_pogopin_is_support() &&
		power_platform_pogopin_otg_from_buckboost())
		vbus_ch_close(VBUS_CH_USER_PD,
			VBUS_CH_TYPE_POGOPIN_BOOST, true, true);
	else
		vbus_ch_close(VBUS_CH_USER_PD,
			VBUS_CH_TYPE_BOOST_GPIO, true, true);
	vbus_ch_eh_dmd_report(BOOST_GPIO_OTG_OCP_DMD_REPORT, data);
}

int vbus_ch_eh_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct vbus_ch_eh_dev *l_dev = vbus_ch_eh_get_dev();

	if (!l_dev)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_OTG_SC_CHECK_STOP:
		boost_gpio_otg_sc_check_stop(l_dev);
		break;
	case POWER_NE_OTG_SC_CHECK_START:
		boost_gpio_otg_sc_check_start(l_dev, data);
		break;
	case POWER_NE_OTG_OCP_HANDLE:
		boost_gpio_otg_ocp_handle(l_dev, data);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int __init vbus_ch_eh_init(void)
{
	int ret;
	struct vbus_ch_eh_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_vbus_ch_eh_dev = l_dev;
	l_dev->nb.notifier_call = vbus_ch_eh_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_OTG, &l_dev->nb);
	if (ret)
		goto fail_free_mem;

	INIT_DELAYED_WORK(&l_dev->boost_gpio_otg_sc_info.work,
		boost_gpio_otg_sc_check_work);
	return 0;

fail_free_mem:
	kfree(l_dev);
	g_vbus_ch_eh_dev = NULL;
	return ret;
}

static void __exit vbus_ch_eh_exit(void)
{
	struct vbus_ch_eh_dev *l_dev = vbus_ch_eh_get_dev();

	if (!l_dev)
		return;

	cancel_delayed_work(&l_dev->boost_gpio_otg_sc_info.work);
	power_event_bnc_unregister(POWER_BNT_OTG, &l_dev->nb);
	kfree(l_dev);
	g_vbus_ch_eh_dev = NULL;
}

subsys_initcall_sync(vbus_ch_eh_init);
module_exit(vbus_ch_eh_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("vbus channel error check driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

