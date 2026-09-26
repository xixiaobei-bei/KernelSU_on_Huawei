#include <linux/platform_device.h>
#include <drm/drm_crtc.h>
#include <drm/drm_dp_helper.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/kernel.h>
#include <linux/clk.h>

#include "hisi_panel.h"
#include "hisi_drm_drv.h"
#include "hisi_drm_dsi.h"
#include "edp_bridge_register.h"

#define  EDID_HEADER_LEN 8
#define  DEFAULT_MAX_BRIGHTNESS 120000
#define  EDID_LEN  128
#define  XFER_ONE  1
#define  XFER_TWO  2

struct mipi2edp {
	struct pinctrl_data *pinctrl;
	struct device *dev;
	struct i2c_client *client;
	struct regmap *regmap;
	struct drm_bridge bridge;
	struct drm_connector connector;
	struct drm_encoder *encoder;
	struct clk *mipi2edp_clk;
	/* DSI RX related params */
	struct device_node *host_node;
	struct mipi_dsi_device *dsi;
	u8 num_dsi_lanes;
	struct hisi_panel_info panel_info;
	struct backlight_device *bl;
	struct drm_dp_aux  aux;
	int output_hdisplay;
	int output_vdisplay;
	int output_vrefresh;
	char edid_array[EDID_LEN];
	struct mutex lock;

	enum mipi2edp_product_type product_type;
	int dsi_bit_clk;
	struct mipi2edp_bridge_product_info bridge_product_info;
	struct mipi2edp_bridge_chip_info bridge_chip_info;
	void *private_data;
};

struct mipi_info {
	uint32_t dsi_bit_clk;
	uint32_t hsa;
	uint32_t hbp;
	uint32_t hline_time;
	uint32_t vsa;
	uint32_t vbp;
	uint32_t vfp;
	uint32_t dpi_hsize;
	uint32_t vactive_line;
};

struct config_table {
	int64_t key;
	const struct mipi_info *mipi_config_info;
};

#define CHOOSE_GET_EDID                       (0X50<<16 | 1<<11 | 1<<10 | 1<<9 | 0<<8 | 16)
#define CHOOSE_SET_LUMINANCE                  (0X37<<16 | 0<<11 | 1<<10 | 1<<9 | 0<<8 | 7)
#define CHOOSE_RESPONSE_GETVCP                (0X37<<16 | 1<<11 | 1<<10 | 1<<9 | 0<<8 | 16)
#define CHOOSE_GET_VCP_OPCODE                 (0X37<<16 | 0<<11 | 1<<10 | 1<<9 | 0<<8 | 5)
#define CHOOSE_GET_ADDR                       (0X37<<16 | 1<<11 | 1<<10 | 1<<9 | 0<<8 | 1)
#define CHOOSE_RESPONSE_NODATA_1              (0X50<<16 | 1<<11 | 1<<10 | 1<<9 | 1<<8 | 1)
#define CHOOSE_RESPONSE_NODATA_2              (0X50<<16 | 1<<11 | 1<<10 | 0<<9 | 1<<8 | 1)
#define CHOOSE_RESPONSE_NODATA_3              (0X37<<16 | 1<<11 | 1<<10 | 1<<9 | 1<<8 | 1)
#define CHOOSE_RESPONSE_NODATA_4              (0X37<<16 | 1<<11 | 1<<10 | 0<<9 | 1<<8 | 1)
#define CHOOSE_RESPONSE_NODATA_5              (0X37<<16 | 1<<11 | 1<<10 | 1<<9 | 0<<8 | 4)
#define CHOOSE_EDID_ADDR                      (0X50<<16 | 1<<11 | 1<<10 | 1<<9 | 0<<8 | 1)

#define VCP_OPCODE_FOR_VCPVERSION    0XDF
#define VCP_OPCODE_FOR_LUMINANCE     0X10
#define UNKNOWN_VCP_OPCODE           -1
#define LT9711A_I2C_LUMINANCE_ADDR   0x37
#define NO_DATA                      0
#define EDID_LENGTH                  128
#define VCP_BUFFER_LENGTH            16
#define CHOOSE_UNKNOWN               -1
#define LIGHT_VALUE_POS              5
#define VCP_OPCODE_POS               3

void mipi2edp_panel_common_info_init(struct hisi_panel_info *pinfo);
int check_edid(char *edid, unsigned int len);
int set_bl_props(struct mipi2edp *pdata, struct device *dev);
int mipi2edp_get_edid(struct mipi2edp *pdata);
int read_dts_value_u32(struct device_node *dn, const char *propname, u32 *out_value);
void aux_register(struct mipi2edp *pdata);

#define screen_resolution_key(H, V, R) (((int64_t)(H) << 40) + ((int64_t)(V) << 16) + (int64_t)R)
