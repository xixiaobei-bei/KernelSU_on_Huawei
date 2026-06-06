

#include <linux/kernel.h>
#include <linux/module.h>
#include <huawei_platform/log/hw_log.h>
#include "pcie_config.h"


static power_config_fp g_pcie_power_level_config = NULL;
static power_level_info g_power_level_info = { 0, 0 };

/* ap send pcie_power_config to modem */
void config_pcie_power_level(int power_level, int timeout)
{
	g_power_level_info.power_level = power_level;
	g_power_level_info.timeout = timeout;
	if (g_pcie_power_level_config == NULL) {
		pr_info("%s: power level config func is not registed\n", __func__);
		return;
	}
	g_pcie_power_level_config(power_level, timeout);
}

/* modem register callback func */
void kirin_pm_mode_notify_register(power_config_fp handler)
{
	if (handler == NULL) {
		pr_err("%s: try to register NULL func\n", __func__);
		return;
	}

	g_pcie_power_level_config = handler;
	pr_info("%s: init power level config\n", __func__);
}

/* modem unregister callback func */
void kirin_pm_mode_notify_unregister(void)
{
	pr_info("%s\n", __func__);
	g_pcie_power_level_config = NULL;
	return;
}

/* modem get pcie_power_level_info */
power_level_info get_power_level_info(void)
{
	return g_power_level_info;
}
EXPORT_SYMBOL(kirin_pm_mode_notify_register);
EXPORT_SYMBOL(kirin_pm_mode_notify_unregister);
EXPORT_SYMBOL(get_power_level_info);
