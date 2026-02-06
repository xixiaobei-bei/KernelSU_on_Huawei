#ifndef RTD2172_H
#define RTD2172_H

#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <drm/drm_crtc.h>
#include <drm/drm_dp_helper.h>
#include <linux/regulator/consumer.h>

#include "hisi_panel.h"
#include "hisi_drm_drv.h"
#include "hisi_drm_dsi.h"


struct rtd2172_bridge {
	struct device *dev;
	struct i2c_client *client;
	struct regulator *ec_hdmi_vdd_on;
	struct regulator *ec_1v1_en;
};

bool is_dp_bridge_suspend(void);

#endif

