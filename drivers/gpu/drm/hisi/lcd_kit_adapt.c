/*lint -e546 -e565 -e580*/
#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wvisibility"

#include <securec.h>

#include <linux/stddef.h>
#include <linux/string.h>
#include <linux/module.h>

#include "hisi_defs.h"


int g_tskit_pt_station_flag;
EXPORT_SYMBOL(g_tskit_pt_station_flag);


int lcdkit_register_notifier(struct notifier_block *nb)
{
	UNUSED(nb);
	return 0;
}
EXPORT_SYMBOL(lcdkit_register_notifier);

int lcdkit_unregister_notifier(struct notifier_block *nb)
{
	UNUSED(nb);
	return 0;
}
EXPORT_SYMBOL(lcdkit_unregister_notifier);

int hostprocessing_get_project_id_for_udp(char *out, int len)
{
	char sharp[] = "ALPS491600";
	int ret;

	if (out == NULL)
		return -1;

	ret = memcpy_s(out, len, sharp, strlen(sharp) + 1);
	drm_check_and_return(ret != EOK, -1, ERR, "memcpy failed!");
	return 0;
}
EXPORT_SYMBOL(hostprocessing_get_project_id_for_udp);

int hostprocessing_get_project_id(char *out, int len)
{
	char sharp[] = "ALPS491600";
	int ret;

	if (out == NULL) {
		HISI_DRM_ERR("out is nullptr!");
		return -EINVAL;
	}

	ret = memcpy_s(out, len, sharp, strlen(sharp) + 1);
	drm_check_and_return(ret != EOK, -EINVAL, ERR, "memcpy failed!");

	return 0;
}
EXPORT_SYMBOL(hostprocessing_get_project_id);

void lcd_huawei_thp_register(struct tp_thp_device_ops *tp_thp_device_ops)
{
	UNUSED(tp_thp_device_ops);
}
EXPORT_SYMBOL(lcd_huawei_thp_register);

int hisifb_esd_recover_disable(int value)
{
	UNUSED(value);
	return 0;
}
EXPORT_SYMBOL(hisifb_esd_recover_disable);


/*
 * lcd kit ops, provide to ts kit module register.
 */
struct ts_kit_ops {
	int (*ts_power_notify)(enum lcd_kit_ts_pm_type type, int sync);
	int (*get_tp_status_by_type)(int type, int *status);
	int (*read_otp_gamma)(u8 *buf, int len);
	bool (*get_tp_proxmity)(void);
};

/*
 * lcd kit ops, provide to lcd kit module register.
 */
struct lcd_kit_ops {
	bool (*lcd_kit_support)(void);
	int (*create_sysfs)(struct kobject *obj);
	int (*get_project_id)(char *buff);
	int (*get_status_by_type)(int type, int *status);
	int (*get_pt_station_status)(void);
	int (*get_lcd_status)(void);
	int (*get_panel_power_status)(void);
	int (*power_monitor_on)(void);
	int (*power_monitor_off)(void);
	int (*set_vss_by_thermal)(void);
	int (*write_otp_gamma)(u8 *buf);
};

struct ts_kit_ops *g_ts_kit_ops;
struct lcd_kit_ops *g_lcd_kit_ops;

struct lcd_kit_ops *lcd_kit_get_ops(void)
{
	return g_lcd_kit_ops;
}

int ts_kit_ops_register(struct ts_kit_ops *ops)
{
	if (g_ts_kit_ops == NULL) {
		g_ts_kit_ops = ops;
		return 0;
	}

	return -EINVAL;
}

#pragma GCC diagnostic pop
/*lint +e546 +e565 +e580*/

