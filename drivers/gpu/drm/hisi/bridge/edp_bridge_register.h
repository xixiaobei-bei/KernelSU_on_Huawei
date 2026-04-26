#ifndef EDP_BRIDGE_REGISTER_H
#define EDP_BRIDGE_REGISTER_H

#include <linux/list.h>

enum mipi2edp_product_series {
	PRODUCT_SERIES_LAPTOP = 0,
	PRODUCT_SERIES_DESKTOP,
	PRODUCT_SERIES_INVALID
};

enum mipi2edp_product_type {
	PRODUCT_TYPE_LAPTOP = 0,
	PRODUCT_TYPE_DESKTOP_V3 = 1,
	PRODUCT_TYPE_DESKTOP_V4 = 2,
	PRODUCT_TYPE_LAPTOP_KV = 3,
	PRODUCT_TYPE_INVALID
};

enum mipi2edp_chip_addr {
	CHIP_I2C_ADDR_SN65DSIX6 = 0x2c,
	CHIP_I2C_ADDR_LT9711A = 0x41,
	CHIP_ADDR_INVALID
};

struct mipi2edp;
struct drm_display_mode;
struct drm_connector;
struct i2c_client;
struct drm_dp_aux;
struct drm_dp_aux_msg;
struct regmap_config;
struct clk;

struct mipi2edp_bridge_chip_info {
	struct list_head list;
	enum mipi2edp_chip_addr chip_addr;
	const char *name;
	const struct regmap_config *regmap_config;

	int (*is_chip_onboard)(struct mipi2edp *pdata);
	void (*pre_enable)(struct mipi2edp *pdata);
	int (*enable)(struct mipi2edp *pdata);
	void (*disable)(struct mipi2edp *pdata);
	void (*post_disable)(struct mipi2edp *pdata);
	int (*get_edid)(char *edid, u32 len);
	ssize_t (*transfer)(struct drm_dp_aux *aux, struct drm_dp_aux_msg *msg);
	void (*set_clk)(struct clk *clock, int enable);
};

int mipi2edp_bridge_chip_info_register(struct mipi2edp_bridge_chip_info *chip_info);
int mipi2edp_bridge_chip_info_get(enum mipi2edp_chip_addr chip_addr, struct mipi2edp_bridge_chip_info *chip_info);

struct bridge_product_funcs {
	int (*bridge_init_private_data)(struct mipi2edp *pdata, struct i2c_client *client);
	int (*bridge_probe)(struct mipi2edp *pdata, struct i2c_client *client);
	int (*bridge_remove)(struct i2c_client *client);
	int (*bridge_shutdown)(struct mipi2edp *pdata);

	int (*bridge_attach)(struct mipi2edp *pdata);
	void (*bridge_pre_enable)(struct mipi2edp *pdata);
	void (*bridge_enable)(struct mipi2edp *pdata);
	void (*bridge_disable)(struct mipi2edp *pdata);
	void (*bridge_post_disable)(struct mipi2edp *pdata);
	void (*bridge_mode_set)(struct mipi2edp *pdata, struct drm_display_mode *mode, struct drm_display_mode *adjusted_mode);

	int (*get_edid)(struct mipi2edp *pdata);
};

struct bridge_connector_help_funcs {
	enum drm_connector_status (*drm_connector_detect)(struct mipi2edp *pdata, bool force);

	/**
	 * @drm_connector_get_mode:
	 *
	 * RETURNS:
	 *
	 * The number of modes added by calling drm_mode_probed_add().
	 */
	int (*drm_connector_get_mode)(struct mipi2edp *pdata);

	enum drm_mode_status (*drm_connector_mode_valid)(struct drm_connector *connector, struct drm_display_mode *mode);
};

struct bridge_pm_func {
	int (*pm_runtime_suspend)(struct mipi2edp *pdata);
	int (*pm_runtime_resume)(struct mipi2edp *pdata);
	int (*pm_system_suspend)(struct mipi2edp *pdata);
	int (*pm_system_resume)(struct mipi2edp *pdata);
};

struct mipi2edp_bridge_product_info {
	struct list_head list;
	enum mipi2edp_product_type product_type;
	enum mipi2edp_product_series product_series;
	const char* desc;
	int connector_type;
	const struct bridge_product_funcs *bridge_ops;
	const struct bridge_connector_help_funcs *bridge_connector_ops;
	const struct bridge_pm_func *bridge_pm_ops;
};

int mipi2edp_bridge_product_info_register(struct mipi2edp_bridge_product_info *product_info);
int mipi2edp_bridge_product_info_get(enum mipi2edp_product_type product_type, struct mipi2edp_bridge_product_info *product_info);

#endif
