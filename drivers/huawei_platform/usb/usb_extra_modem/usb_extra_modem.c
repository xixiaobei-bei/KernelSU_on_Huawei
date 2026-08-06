/*
 * usb_extra_modem.c
 *
 * file for usb_extra_modem driver
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/usb/usb_extra_modem.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_uvdm.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_wakeup.h>
#include <huawei_platform/usb/hw_pd_dev.h>

#define HWLOG_TAG usb_extra_modem
HWLOG_REGIST();

struct uem_dev_info *g_uem_di;

struct uem_dev_info *uem_get_dev_info(void)
{
	if (!g_uem_di)
		hwlog_err("dev_info is null\n");

	return g_uem_di;
}

unsigned int uem_get_charge_resistance(void)
{
	struct uem_dev_info *di = uem_get_dev_info();

	if (!di)
		return 0;

	if (!uem_check_online_status())
		return 0;

	return di->charge_resistance;
}

unsigned int uem_get_charge_leak_current(void)
{
	struct uem_dev_info *di = uem_get_dev_info();

	if (!di)
		return 0;

	if (!uem_check_online_status())
		return 0;

	return di->charge_leak_current;
}

static void uem_set_online_status(bool flag)
{
	struct uem_dev_info *di = uem_get_dev_info();

	if (!di)
		return;

	di->attach_status = flag;
	hwlog_info("uem online status: %d\n", di->attach_status);
}

bool uem_check_online_status(void)
{
	struct uem_dev_info *di = uem_get_dev_info();

	if (!di)
		return false;

	return di->attach_status;
}

static void uem_check_usb_speed(struct uem_dev_info *di)
{
	if (!uem_check_online_status() || di->otg_status)
		return;

	cancel_delayed_work(&di->ncm_enumeration_work);
	power_wakeup_unlock(di->uem_lock);
}

static u32 uem_uvdm_package_header_data(u32 cmd)
{
	return (UVDM_CMD_DIRECTION_INITIAL << UVDM_HDR_SHIFT_CMD_DIRECTTION) |
		(cmd << UVDM_HDR_SHIFT_CMD) |
		(UVDM_FUNCTION_USB_EXT_MODEM << UVDM_HDR_SHIFT_FUNCTION) |
		(UVDM_VERSION << UVDM_HDR_SHIFT_VERSION) |
		(UVDM_VDM_TYPE << UVDM_HDR_SHIFT_VDM_TYPE) |
		(UVDM_VID << UVDM_HDR_SHIFT_VID);
}

static int uem_send_uvdm_command(u32 cmd)
{
	u32 data[UVDM_VDOS_COUNT_ONE] = { 0 };

	/* data[0]: header */
	data[0] = uem_uvdm_package_header_data(cmd);

	return uvdm_protocol_handle_vdo_data(data, UVDM_VDOS_COUNT_ONE, false, 0);
}

void uem_set_product_id_info(unsigned int vid, unsigned int pid)
{
	struct uem_dev_info *di = uem_get_dev_info();

	if (!di)
		return;

	hwlog_info("vid = 0x%x, pid = 0x%x\n", vid, pid);
	if (vid != UEM_VID || pid != UEM_PID) {
		hwlog_info("usb extra modem not find\n");
		return;
	}

	power_wakeup_lock(di->uem_lock);
	schedule_delayed_work(&di->ncm_enumeration_work, msecs_to_jiffies(UEM_NCM_ENUMERATION_TIME_OUT));
	uem_set_online_status(true);
	di->modem_active_status = true;
	pd_dpm_set_emark_detect_enable(false);
	chip_usb_set_hifi_ip_first(0);
	schedule_delayed_work(&di->attach_work, msecs_to_jiffies(UEM_ATTACH_WORK_DELAY));
	schedule_delayed_work(&di->charge_info_work, msecs_to_jiffies(UEM_CHARGE_INFO_DELAY));
}

void uem_handle_pr_swap_end(void)
{
	struct uem_dev_info *di = uem_get_dev_info();

	if (!di)
		return;

	if (!uem_check_online_status())
		return;

	pd_dpm_set_ignore_bc12_event_when_vbuson(false);
	hwlog_info("uem pr swap end, do dr swap\n");
	pd_dpm_start_data_role_swap();
	power_wakeup_unlock(di->uem_lock);
}

static void uem_ext_otg_insert_uevent(struct uem_dev_info *di)
{
	char *envp[UEM_UEVENT_SIZE] = { "UEM=OTGINSERT", NULL };
	kobject_uevent_env(&di->dev->kobj, KOBJ_CHANGE, envp);
	hwlog_info("uem send otg insert uevent\n");
}

static void uem_ext_otg_disconnect_uevent(struct uem_dev_info *di)
{
	char *envp[UEM_UEVENT_SIZE] = { "UEM=OTGDISCONNECT", NULL };
	kobject_uevent_env(&di->dev->kobj, KOBJ_CHANGE, envp);
	hwlog_info("uem send otg disconnect uevent\n");
}

static void uem_ext_vbus_insert_uevent(struct uem_dev_info *di)
{
	char *envp[UEM_UEVENT_SIZE] = { "UEM=EXTVBUSINSERT", NULL };
	kobject_uevent_env(&di->dev->kobj, KOBJ_CHANGE, envp);
	hwlog_info("uem send ext vbus insert uevent\n");
}

static void uem_detach_uevent(struct uem_dev_info *di)
{
	char *envp[UEM_UEVENT_SIZE] = { "UEM=DETACH", NULL };
	kobject_uevent_env(&di->dev->kobj, KOBJ_CHANGE, envp);
	hwlog_info("uem send detach uevent\n");
}

static void uem_attach_uevent(struct uem_dev_info *di)
{
	char *envp[UEM_UEVENT_SIZE] = { "UEM=ATTACH", "MODULEID=2999", NULL };
	kobject_uevent_env(&di->dev->kobj, KOBJ_CHANGE, envp);
	hwlog_info("uem send attach uevent\n");
}

void uem_handle_detach_event(void)
{
	struct uem_dev_info *di = uem_get_dev_info();

	if (!di)
		return;

	if (!uem_check_online_status())
		return;

	uem_set_online_status(false);
	pd_dpm_set_emark_detect_enable(true);
	chip_usb_set_hifi_ip_first(1);
	di->modem_active_status = false;
	di->charge_resistance = 0;
	di->charge_leak_current = 0;
	di->otg_status = 0;
	di->vbus_status = 0;
	uem_detach_uevent(di);
	power_wakeup_unlock(di->uem_lock);
}

static void uem_otg_insert_work(struct work_struct *work)
{
	hwlog_info("uem send uvdm command: open q3\n");
	uem_send_uvdm_command(UEM_HWUVDM_CMD_CMOS_Q3_OPEN);
}

static void uem_vbus_insert_work(struct work_struct *work)
{
	hwlog_info("uem send uvdm command: charge ready\n");
	uem_send_uvdm_command(UEM_HWUVDM_CMD_CHARGE_READY);
}

static void uem_charge_info_work(struct work_struct *work)
{
	hwlog_info("uem request charge info\n");
	uem_send_uvdm_command(UEM_HWUVDM_CMD_REQUEST_CHARGE_INFO);
}

static void uem_attach_work(struct work_struct *work)
{
	struct uem_dev_info *di = container_of(work, struct uem_dev_info,
		attach_work.work);

	if (!di)
		return;

	hwlog_info("uem send uvdm command: close q2\n");
	uem_send_uvdm_command(UEM_HWUVDM_CMD_CMOS_Q2_CLOSE);
	uem_attach_uevent(di);
}

static void uem_ncm_enumeration_work(struct work_struct *work)
{
	struct uem_dev_info *di = container_of(work, struct uem_dev_info,
		ncm_enumeration_work.work);

	if (!di)
		return;

	if (di->otg_status)
		return;

	hwlog_info("uem ncm enumeration time out\n");
	power_wakeup_unlock(di->uem_lock);
}

static int uem_handle_receive_uvdm_data(struct uem_dev_info *di,
	u32 *vdo)
{
	u32 cmd;
	u32 vdo_hdr = vdo[0];

	cmd = (vdo_hdr >> UVDM_HDR_SHIFT_CMD) & UVDM_MASK_CMD;
	switch (cmd) {
	case UEM_HWUVDM_CMD_VBUS_INSERT:
		power_wakeup_lock(di->uem_lock);
		di->vbus_status = true;
		hwlog_info("uem vbus insert\n");
		uem_ext_vbus_insert_uevent(di);
		schedule_delayed_work(&di->vbus_insert_work, msecs_to_jiffies(UEM_VBUS_INSERT_DELAY));
		break;
	case UEM_HWUVDM_CMD_OTG_INSERT:
		power_wakeup_lock(di->uem_lock);
		di->otg_status = true;
		hwlog_info("uem otg insert\n");
		uem_ext_otg_insert_uevent(di);
		schedule_delayed_work(&di->otg_insert_work, msecs_to_jiffies(UEM_OTG_INSERT_DELAY));
		break;
	case UEM_HWUVDM_CMD_OTG_DISCONNECT:
		power_wakeup_unlock(di->uem_lock);
		di->otg_status = false;
		hwlog_info("uem otg disconnect\n");
		uem_ext_otg_disconnect_uevent(di);
		break;
	case UEM_HWUVDM_CMD_REQUEST_CHARGE_INFO:
		di->charge_resistance = vdo[1];
		di->charge_leak_current = vdo[2];
		hwlog_info("uem get charge info, Resistance: 0x%x, leak current: 0x%x\n",
			di->charge_resistance, di->charge_leak_current);
		break;
	default:
		break;
	}
}

static int uem_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct uem_dev_info *di = container_of(nb, struct uem_dev_info, nb);

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_UEM_RECEIVE_UVDM_DATA:
		if (!data)
			return NOTIFY_OK;

		uem_handle_receive_uvdm_data(di, (u32 *)data);
		break;
	case POWER_NE_UEM_USB_SUPER_SPEED:
		uem_check_usb_speed(di);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int uem_modem_enable_control(int val)
{
	struct uem_dev_info *di = uem_get_dev_info();

	if (!di)
		return;

	if (!uem_check_online_status())
		return;

	if (val) {
		hwlog_info("uem modem boot up\n");
		di->modem_active_status = true;
		uem_send_uvdm_command(UEM_HWUVDM_CMD_PMU_ENABLE);
	} else {
		hwlog_info("uem modem shutdown\n");
		di->modem_active_status = false;
		uem_send_uvdm_command(UEM_HWUVDM_CMD_PMU_DISABLE);
	}
}

static void uem_usb_power_control(int val)
{
	struct uem_dev_info *di = uem_get_dev_info();

	if (!di)
		return;

	if (!uem_check_online_status() || di->vbus_status)
		return;

	if (val) {
		hwlog_info("uem send uvdm command: usb power on\n");
		uem_send_uvdm_command(UEM_HWUVDM_CMD_USB_POWERON);
	} else {
		hwlog_info("uem send uvdm command: usb power off\n");
		uem_send_uvdm_command(UEM_HWUVDM_CMD_USB_POWEROFF);
	}
}

static void uem_modem_wakeup_control(int val)
{
	struct uem_dev_info *di = uem_get_dev_info();

	if (!di)
		return;

	if (!uem_check_online_status() || di->vbus_status)
		return;

	if (val) {
		hwlog_info("uem send uvdm command: modem wakeup\n");
		uem_send_uvdm_command(UEM_HWUVDM_CMD_MODEM_WAKEUP);
	}
}

static void uem_host_controller_enable(int val)
{
	struct uem_dev_info *di = uem_get_dev_info();

	if (!di)
		return;

	if (!uem_check_online_status() || di->vbus_status)
		return;

	if (val) {
		hwlog_info("usb host on and pd wake lock\n");
		pd_dpm_usb_host_on();
		pd_dpm_wakelock_ctrl(PD_WAKE_LOCK);
	} else {
		hwlog_info("usb host off and pd wake unlock\n");
		pd_dpm_usb_host_off();
		pd_dpm_wakelock_ctrl(PD_WAKE_UNLOCK);
	}
}

static ssize_t uem_modem_active_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct uem_dev_info *di = uem_get_dev_info();

	if (!di)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "%u\n", di->modem_active_status);
}
static DEVICE_ATTR(modem_active_status, 0664, uem_modem_active_status_show, NULL);

static ssize_t uem_attach_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct uem_dev_info *di = uem_get_dev_info();

	if (!di)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "%u\n", di->attach_status);
}
static DEVICE_ATTR(attach_status, 0664, uem_attach_status_show, NULL);

static ssize_t uem_otg_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct uem_dev_info *di = uem_get_dev_info();

	if (!di)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "%u\n", di->otg_status);
}
static DEVICE_ATTR(otg_status, 0664, uem_otg_status_show, NULL);

static ssize_t uem_vbus_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct uem_dev_info *di = uem_get_dev_info();

	if (!di)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "%u\n", di->vbus_status);
}
static DEVICE_ATTR(vbus_status, 0664, uem_vbus_status_show, NULL);

static ssize_t uem_modem_enable_control_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;

	if (kstrtol(buf, UEM_BASE_DEC, &val) < 0) {
		hwlog_err("modem_enable control val invalid\n");
		return -EINVAL;
	}

	uem_modem_enable_control((int)val);
	return count;
}
static DEVICE_ATTR(modem_enable, 0664, NULL, uem_modem_enable_control_store);

static ssize_t uem_modem_wakeup_control_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;

	if (kstrtol(buf, UEM_BASE_DEC, &val) < 0) {
		hwlog_err("modem_wakeup control val invalid\n");
		return -EINVAL;
	}

	uem_modem_wakeup_control((int)val);
	return count;
}
static DEVICE_ATTR(modem_wakeup, 0664, NULL, uem_modem_wakeup_control_store);

static ssize_t uem_usb_power_control_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;

	if (kstrtol(buf, UEM_BASE_DEC, &val) < 0) {
		hwlog_err("usb_power control val invalid\n");
		return -EINVAL;
	}

	uem_usb_power_control((int)val);
	return count;
}
static DEVICE_ATTR(usb_power, 0664, NULL, uem_usb_power_control_store);

static ssize_t uem_host_controller_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;

	if (kstrtol(buf, UEM_BASE_DEC, &val) < 0) {
		hwlog_err("host_controller_enable val invalid\n");
		return -EINVAL;
	}

	uem_host_controller_enable((int)val);
	return count;
}
static DEVICE_ATTR(host_controller_enable, 0664, NULL, uem_host_controller_enable_store);

static struct attribute *uem_sysfs_attributes[] = {
	&dev_attr_modem_active_status.attr,
	&dev_attr_attach_status.attr,
	&dev_attr_otg_status.attr,
	&dev_attr_vbus_status.attr,
	&dev_attr_modem_enable.attr,
	&dev_attr_modem_wakeup.attr,
	&dev_attr_usb_power.attr,
	&dev_attr_host_controller_enable.attr,
	NULL,
};

static const struct attribute_group uem_attr_group = {
	.attrs = uem_sysfs_attributes,
};

static int uem_probe(struct platform_device *pdev)
{
	struct uem_dev_info *di = NULL;
	int ret;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_uem_di = di;
	di->dev = &pdev->dev;
	platform_set_drvdata(pdev, di);
	INIT_DELAYED_WORK(&di->charge_info_work, uem_charge_info_work);
	INIT_DELAYED_WORK(&di->vbus_insert_work, uem_vbus_insert_work);
	INIT_DELAYED_WORK(&di->otg_insert_work, uem_otg_insert_work);
	INIT_DELAYED_WORK(&di->attach_work, uem_attach_work);
	INIT_DELAYED_WORK(&di->ncm_enumeration_work, uem_ncm_enumeration_work);
	di->uem_lock = power_wakeup_source_register(di->dev, "uem_wakelock");
	di->nb.notifier_call = uem_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_USB_EXT_MODEM, &di->nb);
	if (ret)
		goto notifier_regist_fail;

	power_sysfs_create_group("hw_usb", "usb_extra_modem", &uem_attr_group);
	hwlog_info("probe end\n");
	return 0;

notifier_regist_fail:
	power_wakeup_source_unregister(di->uem_lock);
	g_uem_di = NULL;
	platform_set_drvdata(pdev, NULL);
	kfree(di);
	return ret;
}

static const struct of_device_id uem_match_table[] = {
	{
		.compatible = "huawei,usb_extra_modem",
		.data = NULL,
	},
	{},
};

static struct platform_driver uem_driver = {
	.probe = uem_probe,
	.driver = {
		.name = "huawei,usb_extra_modem",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(uem_match_table),
	},
};

static int __init uem_init(void)
{
	return platform_driver_register(&uem_driver);
}

static void __exit uem_exit(void)
{
	platform_driver_unregister(&uem_driver);
}

module_init(uem_init);
module_exit(uem_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei usb extra modem driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
