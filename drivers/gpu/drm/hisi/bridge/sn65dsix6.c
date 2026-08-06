/*
 * sn65dsix6.c
 *
 * operate function for sn65dsix6
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
#include <linux/mfd/hisi_pmic.h>
#include "edp_bridge.h"

#define SN_DEVICE_REV_REG 0x08
#define SN_DPPLL_SRC_REG 0x0A
#define DPPLL_CLK_SRC_DSICLK BIT(0)
#define REFCLK_FREQ_MASK GENMASK(3, 1)
#define refclk_freq(x) ((x) << 1)
#define DPPLL_SRC_DP_PLL_LOCK BIT(7)
#define SN_PLL_ENABLE_REG 0x0D
#define SN_DSI_LANES_REG 0x10
#define CHA_DSI_LANES_MASK GENMASK(4, 3)
#define cha_dsi_lanes(x) ((x) << 3)
#define SN_DSIA_CLK_FREQ_REG 0x12
#define SN_CHA_ACTIVE_LINE_LENGTH_LOW_REG 0x20
#define SN_CHA_VERTICAL_DISPLAY_SIZE_LOW_REG 0x24
#define SN_CHA_HSYNC_PULSE_WIDTH_LOW_REG 0x2C
#define SN_CHA_HSYNC_PULSE_WIDTH_HIGH_REG 0x2D
#define CHA_HSYNC_POLARITY BIT(7)
#define SN_CHA_VSYNC_PULSE_WIDTH_LOW_REG 0x30
#define SN_CHA_VSYNC_PULSE_WIDTH_HIGH_REG 0x31
#define CHA_VSYNC_POLARITY BIT(7)
#define SN_CHA_HORIZONTAL_BACK_PORCH_REG 0x34
#define SN_CHA_VERTICAL_BACK_PORCH_REG 0x36
#define SN_CHA_HORIZONTAL_FRONT_PORCH_REG 0x38
#define SN_CHA_VERTICAL_FRONT_PORCH_REG 0x3A
#define SN_ENH_FRAME_REG 0x5A
#define VSTREAM_ENABLE BIT(3)
#define SN_DATA_FORMAT_REG 0x5B
#define SN_HPD_DISABLE_REG 0x5C
#define HPD_DISABLE BIT(0)
#define sn_aux_wdata_reg(x) (0x64 + (x))
#define SN_AUX_ADDR_19_16_REG 0x74
#define SN_AUX_ADDR_15_8_REG 0x75
#define SN_AUX_ADDR_7_0_REG 0x76
#define SN_AUX_LENGTH_REG 0x77
#define SN_AUX_CMD_REG 0x78
#define AUX_CMD_SEND BIT(0)
#define aux_cmd_req(x) ((x) << 4)
#define sn_aux_rdata_reg(x) (0x79 + (x))
#define SN_SSC_CONFIG_REG 0x93
#define DP_NUM_LANES_MASK GENMASK(5, 4)
#define dp_num_lanes(x) ((x) << 4)
#define SN_DATARATE_CONFIG_REG 0x94
#define DP_DATARATE_MASK GENMASK(7, 5)
#define dp_datarate(x) ((x) << 5)
#define SN_ML_TX_MODE_REG 0x96
#define ML_TX_MAIN_LINK_OFF 0
#define ML_TX_NORMAL_MODE BIT(0)
#define SN_AUX_CMD_STATUS_REG 0xF4
#define AUX_IRQ_STATUS_AUX_RPLY_TOUT BIT(3)
#define AUX_IRQ_STATUS_AUX_SHORT BIT(5)
#define AUX_IRQ_STATUS_NAT_I2C_FAIL BIT(6)
#define SN_I2C_CLAIM_1_REG 0x60
#define SN_I2C_CLAIM_2_REG 0x61

#define delay_ms(x) (x)
/* Matches DP_AUX_MAX_PAYLOAD_BYTES (for now) */
#define SN_AUX_MAX_PAYLOAD_BYTES 16
#define SN65DSI86_CONFIG_LENGTH 30

#define I2C_CONTROLLER 4
#define I2C_TRANSFER_LEN 2

#define MAX_CHIP_PROBE_TIMES 3

struct i2c_config {
	unsigned int registers;
	unsigned int value;
	unsigned int delayms;
};

struct i2c_config_table {
	int64_t key;
	struct i2c_config *config;
};

static struct i2c_config g_sn65dsi86_1920_config[] = {
	{0xFF, 0x07, delay_ms(0)},
	{0x16, 0x01, delay_ms(0)},
	{0xFF, 0x00, delay_ms(0)},
	{0x0A, 0x08, delay_ms(0)},
	{0x10, 0x26, delay_ms(0)},
	{0x12, 0x59, delay_ms(0)},
	{0x13, 0x59, delay_ms(0)},
	{0x94, 0x82, delay_ms(0)},
	{0x0D, 0x01, delay_ms(10)},
	{0x5A, 0x04, delay_ms(0)},
	{0x93, 0x20, delay_ms(0)},
	{0x96, 0x0A, delay_ms(20)},
	{0x20, 0x80, delay_ms(0)},
	{0x21, 0x07, delay_ms(0)},
	{0x22, 0x00, delay_ms(0)},
	{0x23, 0x00, delay_ms(0)},
	{0x24, 0x38, delay_ms(0)},
	{0x25, 0x04, delay_ms(0)},
	{0x2C, 0x2C, delay_ms(0)},
	{0x2D, 0x00, delay_ms(0)},
	{0x30, 0x05, delay_ms(0)},
	{0x31, 0x00, delay_ms(0)},
	{0x34, 0x94, delay_ms(0)},
	{0x36, 0x24, delay_ms(0)},
	{0x38, 0x58, delay_ms(0)},
	{0x3A, 0x04, delay_ms(0)},
	{0x5B, 0x00, delay_ms(0)},
	{0x3C, 0x02, delay_ms(0)},
	{0x5A, 0x0C, delay_ms(100)},
	{0x96, 0x0A, delay_ms(0)},
};

static struct i2c_config g_sn65dsi86_1600_config[] = {
	{0xFF, 0x07, delay_ms(0)},
	{0x16, 0x01, delay_ms(0)},
	{0xFF, 0x00, delay_ms(0)},
	{0x0A, 0x08, delay_ms(0)},
	{0x10, 0x26, delay_ms(0)},
	{0x12, 0x40, delay_ms(0)},
	{0x13, 0x40, delay_ms(0)},
	{0x94, 0x22, delay_ms(0)},
	{0x0D, 0x01, delay_ms(10)},
	{0x5A, 0x04, delay_ms(0)},
	{0x93, 0x20, delay_ms(0)},
	{0x96, 0x0A, delay_ms(20)},
	{0x20, 0x40, delay_ms(0)},
	{0x21, 0x06, delay_ms(0)},
	{0x22, 0x00, delay_ms(0)},
	{0x23, 0x00, delay_ms(0)},
	{0x24, 0x84, delay_ms(0)},
	{0x25, 0x03, delay_ms(0)},
	{0x2C, 0x50, delay_ms(0)},
	{0x2D, 0x00, delay_ms(0)},
	{0x30, 0x03, delay_ms(0)},
	{0x31, 0x00, delay_ms(0)},
	{0x34, 0x60, delay_ms(0)},
	{0x36, 0x60, delay_ms(0)},
	{0x38, 0x18, delay_ms(0)},
	{0x3A, 0x01, delay_ms(0)},
	{0x5B, 0x00, delay_ms(0)},
	{0x3C, 0x02, delay_ms(0)},
	{0x5A, 0x0C, delay_ms(100)},
	{0x96, 0x0A, delay_ms(0)},
};

static struct i2c_config g_sn65dsi86_1920_1200_config[] = {
	{0xFF, 0x07, delay_ms(0)},
	{0x16, 0x01, delay_ms(0)},
	{0xFF, 0x00, delay_ms(0)},
	{0x0A, 0x08, delay_ms(0)},
	{0x10, 0x26, delay_ms(0)},
	{0x12, 0x5C, delay_ms(0)},
	{0x13, 0x5C, delay_ms(0)},
	{0x94, 0x82, delay_ms(0)},
	{0x0D, 0x01, delay_ms(10)},
	{0x5A, 0x04, delay_ms(0)},
	{0x93, 0x20, delay_ms(0)},
	{0x96, 0x0A, delay_ms(20)},
	{0x20, 0x80, delay_ms(0)},
	{0x21, 0x07, delay_ms(0)},
	{0x22, 0x00, delay_ms(0)},
	{0x23, 0x00, delay_ms(0)},
	{0x24, 0xB0, delay_ms(0)},
	{0x25, 0x04, delay_ms(0)},
	{0x2C, 0x20, delay_ms(0)},
	{0x2D, 0x00, delay_ms(0)},
	{0x30, 0x06, delay_ms(0)},
	{0x31, 0x00, delay_ms(0)},
	{0x34, 0x50, delay_ms(0)},
	{0x36, 0x1A, delay_ms(0)},
	{0x38, 0x30, delay_ms(0)},
	{0x3A, 0x03, delay_ms(0)},
	{0x5B, 0x00, delay_ms(0)},
	{0x3C, 0x02, delay_ms(0)},
	{0x5A, 0x0C, delay_ms(100)},
	{0x96, 0x0A, delay_ms(0)},
};

static struct i2c_config g_sn65dsi86_2560_config[] = {
	{0xFF, 0x07, delay_ms(0)},
	{0x16, 0x01, delay_ms(0)},
	{0xFF, 0x00, delay_ms(0)},
	{0x0A, 0x08, delay_ms(0)},
	{0x10, 0x26, delay_ms(0)},
	{0x12, 0x76, delay_ms(0)},
	{0x13, 0x76, delay_ms(0)},
	{0x94, 0xE2, delay_ms(0)},
	{0x0D, 0x01, delay_ms(10)},
	{0x5A, 0x04, delay_ms(0)},
	{0x93, 0x20, delay_ms(0)},
	{0x96, 0x0A, delay_ms(20)},
	{0x20, 0x00, delay_ms(0)},
	{0x21, 0x0A, delay_ms(0)},
	{0x22, 0x00, delay_ms(0)},
	{0x23, 0x00, delay_ms(0)},
	{0x24, 0x38, delay_ms(0)},
	{0x25, 0x04, delay_ms(0)},
	{0x2C, 0x2C, delay_ms(0)},
	{0x2D, 0x00, delay_ms(0)},
	{0x30, 0x05, delay_ms(0)},
	{0x31, 0x00, delay_ms(0)},
	{0x34, 0x94, delay_ms(0)},
	{0x36, 0x0B, delay_ms(0)},
	{0x38, 0xF8, delay_ms(0)},
	{0x3A, 0x04, delay_ms(0)},
	{0x5B, 0x00, delay_ms(0)},
	{0x3C, 0x02, delay_ms(0)},
	{0x5A, 0x0C, delay_ms(100)},
	{0x96, 0x0A, delay_ms(0)},
};

static struct i2c_config g_sn65dsi86_1680_config[] = {
	{0xFF, 0x07, delay_ms(0)},
	{0x16, 0x01, delay_ms(0)},
	{0xFF, 0x00, delay_ms(0)},
	{0x0A, 0x08, delay_ms(0)},
	{0x10, 0x26, delay_ms(0)},
	{0x12, 0x57, delay_ms(0)},
	{0x13, 0x57, delay_ms(0)},
	{0x94, 0x82, delay_ms(0)},
	{0x0D, 0x01, delay_ms(10)},
	{0x5A, 0x04, delay_ms(0)},
	{0x93, 0x20, delay_ms(0)},
	{0x96, 0x0A, delay_ms(20)},
	{0x20, 0x90, delay_ms(0)},
	{0x21, 0x06, delay_ms(0)},
	{0x22, 0x00, delay_ms(0)},
	{0x23, 0x00, delay_ms(0)},
	{0x24, 0x1A, delay_ms(0)},
	{0x25, 0x04, delay_ms(0)},
	{0x2C, 0xB0, delay_ms(0)},
	{0x2D, 0x00, delay_ms(0)},
	{0x30, 0x06, delay_ms(0)},
	{0x31, 0x00, delay_ms(0)},
	{0x34, 0xFF, delay_ms(0)},
	{0x36, 0x1E, delay_ms(0)},
	{0x38, 0x68, delay_ms(0)},
	{0x3A, 0x03, delay_ms(0)},
	{0x5B, 0x00, delay_ms(0)},
	{0x3C, 0x02, delay_ms(0)},
	{0x5A, 0x0C, delay_ms(100)},
	{0x96, 0x0A, delay_ms(0)},
};

static struct i2c_config g_sn65dsi86_1152_config[] = {
	{0xFF, 0x07, delay_ms(0)},
	{0x16, 0x01, delay_ms(0)},
	{0xFF, 0x00, delay_ms(0)},
	{0x0A, 0x08, delay_ms(0)},
	{0x10, 0x26, delay_ms(0)},
	{0x12, 0x40, delay_ms(0)},
	{0x13, 0x40, delay_ms(0)},
	{0x94, 0x22, delay_ms(0)},
	{0x0D, 0x01, delay_ms(10)},
	{0x5A, 0x04, delay_ms(0)},
	{0x93, 0x20, delay_ms(0)},
	{0x96, 0x0A, delay_ms(20)},
	{0x20, 0x80, delay_ms(0)},
	{0x21, 0x04, delay_ms(0)},
	{0x22, 0x00, delay_ms(0)},
	{0x23, 0x00, delay_ms(0)},
	{0x24, 0x60, delay_ms(0)},
	{0x25, 0x03, delay_ms(0)},
	{0x2C, 0x80, delay_ms(0)},
	{0x2D, 0x00, delay_ms(0)},
	{0x30, 0x03, delay_ms(0)},
	{0x31, 0x00, delay_ms(0)},
	{0x34, 0xFF, delay_ms(0)},
	{0x36, 0x20, delay_ms(0)},
	{0x38, 0x40, delay_ms(0)},
	{0x3A, 0x01, delay_ms(0)},
	{0x5B, 0x00, delay_ms(0)},
	{0x3C, 0x02, delay_ms(0)},
	{0x5A, 0x0C, delay_ms(100)},
	{0x96, 0x0A, delay_ms(0)},
};

static struct i2c_config g_sn65dsi86_1024_75_config[] = {
	{0xFF, 0x07, delay_ms(0)},
	{0x16, 0x01, delay_ms(0)},
	{0xFF, 0x00, delay_ms(0)},
	{0x0A, 0x08, delay_ms(0)},
	{0x10, 0x26, delay_ms(0)},
	{0x12, 0x2F, delay_ms(0)},
	{0x13, 0x2F, delay_ms(0)},
	{0x94, 0x82, delay_ms(0)},
	{0x0D, 0x01, delay_ms(10)},
	{0x5A, 0x04, delay_ms(0)},
	{0x93, 0x10, delay_ms(0)},
	{0x96, 0x0A, delay_ms(20)},
	{0x20, 0x00, delay_ms(0)},
	{0x21, 0x04, delay_ms(0)},
	{0x22, 0x00, delay_ms(0)},
	{0x23, 0x00, delay_ms(0)},
	{0x24, 0x00, delay_ms(0)},
	{0x25, 0x03, delay_ms(0)},
	{0x2C, 0x60, delay_ms(0)},
	{0x2D, 0x00, delay_ms(0)},
	{0x30, 0x03, delay_ms(0)},
	{0x31, 0x00, delay_ms(0)},
	{0x34, 0xB0, delay_ms(0)},
	{0x36, 0x1C, delay_ms(0)},
	{0x38, 0x10, delay_ms(0)},
	{0x3A, 0x01, delay_ms(0)},
	{0x5B, 0x00, delay_ms(0)},
	{0x3C, 0x02, delay_ms(0)},
	{0x5A, 0x0C, delay_ms(100)},
	{0x96, 0x0A, delay_ms(0)},
};

static struct i2c_config g_sn65dsi86_1024_60_config[] = {
	{0xFF, 0x07, delay_ms(0)},
	{0x16, 0x01, delay_ms(0)},
	{0xFF, 0x00, delay_ms(0)},
	{0x0A, 0x08, delay_ms(0)},
	{0x10, 0x26, delay_ms(0)},
	{0x12, 0x27, delay_ms(0)},
	{0x13, 0x27, delay_ms(0)},
	{0x94, 0x82, delay_ms(0)},
	{0x0D, 0x01, delay_ms(10)},
	{0x5A, 0x04, delay_ms(0)},
	{0x93, 0x10, delay_ms(0)},
	{0x96, 0x0A, delay_ms(20)},
	{0x20, 0x00, delay_ms(0)},
	{0x21, 0x04, delay_ms(0)},
	{0x22, 0x00, delay_ms(0)},
	{0x23, 0x00, delay_ms(0)},
	{0x24, 0x00, delay_ms(0)},
	{0x25, 0x03, delay_ms(0)},
	{0x2C, 0x88, delay_ms(0)},
	{0x2D, 0x00, delay_ms(0)},
	{0x30, 0x06, delay_ms(0)},
	{0x31, 0x00, delay_ms(0)},
	{0x34, 0xA0, delay_ms(0)},
	{0x36, 0x1D, delay_ms(0)},
	{0x38, 0x18, delay_ms(0)},
	{0x3A, 0x03, delay_ms(0)},
	{0x5B, 0x00, delay_ms(0)},
	{0x3C, 0x02, delay_ms(0)},
	{0x5A, 0x0C, delay_ms(100)},
	{0x96, 0x0A, delay_ms(0)},
};

static struct i2c_config g_sn65dsi86_1280_60_config[] = {
	{0xFF, 0x07, delay_ms(0)},
	{0x16, 0x01, delay_ms(0)},
	{0xFF, 0x00, delay_ms(0)},
	{0x0A, 0x08, delay_ms(0)},
	{0x10, 0x26, delay_ms(0)},
	{0x12, 0x40, delay_ms(0)},
	{0x13, 0x40, delay_ms(0)},
	{0x94, 0x22, delay_ms(0)},
	{0x0D, 0x01, delay_ms(10)},
	{0x5A, 0x04, delay_ms(0)},
	{0x93, 0x20, delay_ms(0)},
	{0x96, 0x0A, delay_ms(20)},
	{0x20, 0x00, delay_ms(0)},
	{0x21, 0x05, delay_ms(0)},
	{0x22, 0x00, delay_ms(0)},
	{0x23, 0x00, delay_ms(0)},
	{0x24, 0x00, delay_ms(0)},
	{0x25, 0x04, delay_ms(0)},
	{0x2C, 0x70, delay_ms(0)},
	{0x2D, 0x00, delay_ms(0)},
	{0x30, 0x03, delay_ms(0)},
	{0x31, 0x00, delay_ms(0)},
	{0x34, 0xF8, delay_ms(0)},
	{0x36, 0x26, delay_ms(0)},
	{0x38, 0x30, delay_ms(0)},
	{0x3A, 0x01, delay_ms(0)},
	{0x5B, 0x00, delay_ms(0)},
	{0x3C, 0x02, delay_ms(0)},
	{0x5A, 0x0C, delay_ms(100)},
	{0x96, 0x0A, delay_ms(0)},
};

static struct i2c_config g_sn65dsi86_1280_75_config[] = {
	{0xFF, 0x07, delay_ms(0)},
	{0x16, 0x01, delay_ms(0)},
	{0xFF, 0x00, delay_ms(0)},
	{0x0A, 0x08, delay_ms(0)},
	{0x10, 0x26, delay_ms(0)},
	{0x12, 0x51, delay_ms(0)},
	{0x13, 0x51, delay_ms(0)},
	{0x94, 0x82, delay_ms(0)},
	{0x0D, 0x01, delay_ms(10)},
	{0x5A, 0x04, delay_ms(0)},
	{0x93, 0x20, delay_ms(0)},
	{0x96, 0x0A, delay_ms(20)},
	{0x20, 0x00, delay_ms(0)},
	{0x21, 0x05, delay_ms(0)},
	{0x22, 0x00, delay_ms(0)},
	{0x23, 0x00, delay_ms(0)},
	{0x24, 0x00, delay_ms(0)},
	{0x25, 0x04, delay_ms(0)},
	{0x2C, 0x90, delay_ms(0)},
	{0x2D, 0x00, delay_ms(0)},
	{0x30, 0x03, delay_ms(0)},
	{0x31, 0x00, delay_ms(0)},
	{0x34, 0xF8, delay_ms(0)},
	{0x36, 0x26, delay_ms(0)},
	{0x38, 0x10, delay_ms(0)},
	{0x3A, 0x01, delay_ms(0)},
	{0x5B, 0x00, delay_ms(0)},
	{0x3C, 0x02, delay_ms(0)},
	{0x5A, 0x0C, delay_ms(100)},
	{0x96, 0x0A, delay_ms(0)},
};

static struct i2c_config_table i2c_config_info_table[] = {
	{((int64_t)1920 << 40) + ((int64_t)1080 << 16) + (int64_t)60, g_sn65dsi86_1920_config},
	{((int64_t)1920 << 40) + ((int64_t)1200 << 16) + (int64_t)60, g_sn65dsi86_1920_1200_config},
	{((int64_t)2560 << 40) + ((int64_t)1080 << 16) + (int64_t)60, g_sn65dsi86_2560_config},
	{((int64_t)1600 << 40) + ((int64_t)900 << 16) + (int64_t)60, g_sn65dsi86_1600_config},
	{((int64_t)1680 << 40) + ((int64_t)1050 << 16) + (int64_t)60, g_sn65dsi86_1680_config},
	{((int64_t)1152 << 40) + ((int64_t)864 << 16) + (int64_t)75, g_sn65dsi86_1152_config},
	{((int64_t)1024 << 40) + ((int64_t)768 << 16) + (int64_t)75, g_sn65dsi86_1024_75_config},
	{((int64_t)1024 << 40) + ((int64_t)768 << 16) + (int64_t)60, g_sn65dsi86_1024_60_config},
	{((int64_t)1280 << 40) + ((int64_t)1024 << 16) + (int64_t)75, g_sn65dsi86_1280_75_config},
	{((int64_t)1280 << 40) + ((int64_t)1024 << 16) + (int64_t)60, g_sn65dsi86_1280_60_config},
};

static int ti_i2c_send_byte(char devaddr, const char regoffset, const char value)
{
	struct i2c_adapter *adap;
	int ret;
	char data[I2C_TRANSFER_LEN];
	struct i2c_msg msg[1];
	int retries = 3;
	unsigned char xfers = XFER_ONE;

	adap = i2c_get_adapter(I2C_CONTROLLER);
	if (!adap) {
		HISI_DRM_ERR("ti_i2c_send_byte i2c_get_adapter err");
		return -ENODEV;
	}

	msg->addr = devaddr;
	msg->flags = 0;
	msg->len = I2C_TRANSFER_LEN;
	msg->buf = data;
	data[0] = regoffset;
	data[1] = value;

	do {
		ret = i2c_transfer(adap, msg, xfers);
	} while (ret != xfers && --retries);
	HISI_DRM_DEBUG("ti_i2c_send_byte regoffset = %d value=%d ret = %d \n", regoffset, value, ret);
	i2c_put_adapter(adap);
	if (ret == 1) {
		return 0;
	}
	return ret;
}

static int ti_i2c_recv_byte(char devaddr, const char regoffset, char *value)
{
	struct i2c_adapter *adap;
	int ret;
	struct i2c_msg msg[2];
	int retries = 3;
	unsigned char xfers = XFER_TWO;

	adap = i2c_get_adapter(I2C_CONTROLLER);
	if (!adap) {
		HISI_DRM_ERR("ti_i2c_recv_byte i2c_get_adapter err");
		return -ENODEV;
	}

	msg[0].addr = devaddr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = (__u8 *)&regoffset;

	msg[1].addr = devaddr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = value;
	do {
		ret = i2c_transfer(adap, msg, xfers);
	} while (ret != xfers && --retries);
	HISI_DRM_DEBUG("ti_i2c_send_byte regoffset=%d value=%d ret = %d \n", regoffset, value[0], ret);
	i2c_put_adapter(adap);
	if (ret == 2) {
		return 0;
	}
	return ret;
}

static int is_sn65dsix6_onboard(struct mipi2edp *pdata)
{
	int i, ret, reg;

	/* check whether sn65dsix6 chip is on board */
	ret = 0;
	reg = 0;
	for (i = 0; i < MAX_CHIP_PROBE_TIMES; i++) {
		ret = regmap_read(pdata->regmap, SN_DEVICE_REV_REG, &reg);
		if (ret)
			continue;

		HISI_DRM_INFO("sn65dsix6 is onboard! device_rev is 0x%x\n", reg);
		return ret;
	}

	HISI_DRM_ERR("sn65dsix6 isn't onboard!\n");
	return ret;
}

static void sn65dsi86_pre_enable(struct mipi2edp *pdata)
{
	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return;
	}

	if (pdata->bridge_product_info.product_series != PRODUCT_SERIES_LAPTOP) {
		HISI_DRM_INFO("not PRODUCT_SERIES_LAPTOP no need to pre enable on chip sn65dsi86!");
		return;
	}

	/* wait for sn65dsi86 power up! */
	mdelay(1);
	/* configure bridge ref_clk */
	regmap_write(pdata->regmap, SN_DPPLL_SRC_REG, 0x08);

	regmap_update_bits(pdata->regmap, SN_HPD_DISABLE_REG, HPD_DISABLE, HPD_DISABLE);
	/* wait for LCD inital finish! */
	mdelay(100);
	/* *
	 * The SN65DSI86 only supports ASSR Display Authentication method and
	 * this method is enabled by default. An eDP panel must support this
	 * authentication method. We need to enable this method in the eDP panel
	 * at DisplayPort address 0x0010A prior to link training.
	 */
	regmap_write(pdata->regmap, sn_aux_wdata_reg(0), 0x01);
	regmap_write(pdata->regmap, SN_AUX_ADDR_19_16_REG, 0x00);
	regmap_write(pdata->regmap, SN_AUX_ADDR_15_8_REG, 0x01);
	regmap_write(pdata->regmap, SN_AUX_ADDR_7_0_REG, 0x0A);
	regmap_write(pdata->regmap, SN_AUX_LENGTH_REG, 0x01);
	regmap_write(pdata->regmap, SN_AUX_CMD_REG, 0x81);
}

static void video_param_config(struct mipi2edp *pdata)
{
	regmap_write(pdata->regmap, 0x20, 0x70);
	regmap_write(pdata->regmap, 0x21, 0x08);
	regmap_write(pdata->regmap, 0x22, 0x00);
	regmap_write(pdata->regmap, 0x23, 0x00);
	regmap_write(pdata->regmap, 0x24, 0xA0);
	regmap_write(pdata->regmap, 0x25, 0x05);
	regmap_write(pdata->regmap, 0x2C, 0x20);
	regmap_write(pdata->regmap, 0x2D, 0x00);
	regmap_write(pdata->regmap, 0x30, 0x0A);
	regmap_write(pdata->regmap, 0x31, 0x00);
	regmap_write(pdata->regmap, 0x34, 0x50);
	regmap_write(pdata->regmap, 0x36, 0x1B);
	regmap_write(pdata->regmap, 0x38, 0x30);
	regmap_write(pdata->regmap, 0x3A, 0x03);
	regmap_write(pdata->regmap, 0x5B, 0x00);
	regmap_write(pdata->regmap, 0x3C, 0x00);
}

static int sn65dsi86_config_i2c(struct mipi2edp *pdata, struct i2c_config *sn65dsi86_config, int len)
{
	int count;
	mdelay(delay_ms(100));
	for (count = 0; count < len; count++) {
		regmap_write(pdata->regmap, sn65dsi86_config[count].registers, sn65dsi86_config[count].value);
		HISI_DRM_INFO("TI : sn65dsi86_config_i2c count =%d registers=%d value=%d", count,
			sn65dsi86_config[count].registers, sn65dsi86_config[count].value);
		mdelay(sn65dsi86_config[count].delayms);
		if (sn65dsi86_config[count].delayms == delay_ms(10))
			HISI_DRM_INFO("TI - sn65dsi86_enable! \n");
	}
	return 0;
}

static int sn65dsi86_config(struct mipi2edp *pdata)
{
	int64_t key;
	int count;
	int len;
	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return -EINVAL;
	}
	key = screen_resolution_key(pdata->output_hdisplay, pdata->output_vdisplay, pdata->output_vrefresh);
	len = ARRAY_SIZE(i2c_config_info_table);
	HISI_DRM_INFO("TI : sn65dsi86_hdisplay=%d_vrefresh=%d", pdata->output_hdisplay, pdata->output_vrefresh);
	for (count = 0; count < len; count++) {
		if (key == i2c_config_info_table[count].key) {
			sn65dsi86_config_i2c(pdata, i2c_config_info_table[count].config, SN65DSI86_CONFIG_LENGTH);
			HISI_DRM_INFO("TI : sn65dsi86_config_i2c");
			return 0;
		}
	}
	HISI_DRM_WARN("output_mode invalid");
	return -EINVAL;
}

static int sn65dsi86_enable(struct mipi2edp *pdata)
{
	int val = 0;
	int ret = 0;
	if (!pdata) {
		HISI_DRM_ERR("pdata is nullptr!");
		return -EINVAL;
	}

	if (pdata->bridge_product_info.product_series == PRODUCT_SERIES_DESKTOP) {
		mutex_lock(&pdata->lock);
		ret = sn65dsi86_config(pdata);

		HISI_DRM_INFO("set i2c_claimx regs for edid(0x50) and ddc(0x37)\n");
		/* I2C_CLAIM1_EN. prepare for address 0x50 (for edid) operation*/
		regmap_write(pdata->regmap, SN_I2C_CLAIM_1_REG, 0xa1);

		/* I2C_CLAIM2_EN. prepare for address 0x37 (for ddc) operation*/
		regmap_write(pdata->regmap, SN_I2C_CLAIM_2_REG, 0x6f);

		mutex_unlock(&pdata->lock);
		return ret;
	}

	/* DSI_A lane config */
	regmap_update_bits(pdata->regmap, SN_DSI_LANES_REG, CHA_DSI_LANES_MASK, 0x26);
	/* EQ for improve signal quality! */
	regmap_write(pdata->regmap, 0x11, 0xff);

	/* DP lane config */
	regmap_update_bits(pdata->regmap, SN_SSC_CONFIG_REG, DP_NUM_LANES_MASK, 0x34);
	regmap_update_bits(pdata->regmap, SN_DATARATE_CONFIG_REG, DP_DATARATE_MASK, 0x20);

	/* set dsi/dp clk frequency value */
	regmap_write(pdata->regmap, SN_DSIA_CLK_FREQ_REG, 0x7C); /* 0x96 */

	/* regmap_update_bits(pdata->regmap, SN_DATARATE_CONFIG_REG,
	 * DP_DATARATE_MASK, dp_datarate(i))
	 */

	/* enable DP PLL */
	regmap_write(pdata->regmap, SN_PLL_ENABLE_REG, 1);

	ret = regmap_read_poll_timeout(pdata->regmap, SN_DPPLL_SRC_REG, val, val & DPPLL_SRC_DP_PLL_LOCK, 1000, 100 * 1000);
	if (ret) {
		HISI_DRM_ERR("DP_PLL_LOCK polling failed, ret=%d!", ret);
		return ret;
	}

	regmap_write(pdata->regmap, 0x95, 0x00);

	/* Semi auto link training mode */
	regmap_write(pdata->regmap, 0x5A, 0x05);
	regmap_write(pdata->regmap, 0x96, 0x02);
	regmap_write(pdata->regmap, 0x93, 0x34);

	regmap_write(pdata->regmap, 0x96, 0x0A);
	mdelay(10);
	regmap_write(pdata->regmap, 0x96, 0x0A);
	mdelay(10);

	ret = regmap_read_poll_timeout(pdata->regmap, SN_ML_TX_MODE_REG, val,
		val == ML_TX_MAIN_LINK_OFF || val == ML_TX_NORMAL_MODE, 1000, 500 * 1000);
	if (ret) {
		HISI_DRM_ERR("Training complete polling failed, ret=%d!", ret);
		return ret;
	} else if (val == ML_TX_MAIN_LINK_OFF) {
		HISI_DRM_ERR("Link training failed, link is off, ret=%d!", ret);
		return ret;
	}

	HISI_DRM_INFO("Link training OK!");

	/* config video parameters */
	video_param_config(pdata);
	/* enable video stream */
	regmap_write(pdata->regmap, 0x5A, 0x0D);

	return ret;
}

static void sn65dsi86_disable(struct mipi2edp *pdata)
{
	/* disable video stream */
	regmap_update_bits(pdata->regmap, SN_ENH_FRAME_REG, VSTREAM_ENABLE, 0);
	/* semi auto link training mode OFF */
	regmap_write(pdata->regmap, SN_ML_TX_MODE_REG, 0);
	/* disable DP PLL */
	regmap_write(pdata->regmap, SN_PLL_ENABLE_REG, 0);
}

static int ti_get_edid(char *edid, u32 len)
{
	char res;
	int i;
	int top = 0;
	int ret = 0;

	mdelay(20);
	ti_i2c_send_byte(0x2C, 0x60, 0xA1);

	for (i = 0; i < len; i++) {
		res = 0;
		ret = ti_i2c_recv_byte(0x50, i, &res);
		edid[top++] = res;
		if (ret) {
			HISI_DRM_ERR("edid regoffset=%d value=0x%x ret=%x\n", i, res, ret);
		}
	}
	HISI_DRM_INFO("EDID:\n");
	print_hex_dump(KERN_NOTICE,
		" \t", DUMP_PREFIX_NONE, 16, 1,
			edid, EDID_LEN, false);
	return 0;
}

static void ti_set_clk(struct clk *clock, int enable)
{
	int ret = 0;
	int value = 0;

	if (enable) {
		/* enable 38M4 clock for bridge from pmu */
		ret = clk_prepare_enable(clock);
		if (ret)
			HISI_DRM_INFO("clk enable failed! ret:%d\n", ret);
	} else {
		value = clk_get_enable_count(clock);
		HISI_DRM_INFO("clk_get_enable_count = %d \n", value);
		while (value > 0) {
			/* disable 38M4 clock for bridge from pmu */
			clk_disable_unprepare(clock);
			value--;
		}
	}
	HISI_DRM_INFO("pmic_read_reg = %d \n", pmic_read_reg(0x3e));
}

static const struct regmap_config sn65dsix6_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.reg_stride = 1,
};
/* "ti_sn_aux_trnasfer" should be deleted,because we actually use /dev/i2c-4 to
 * get edid and set/get brightness.
 */
static struct mipi2edp_bridge_chip_info sn65dsix6_bridge_chip_info = {
	.chip_addr = CHIP_I2C_ADDR_SN65DSIX6,
	.name = "ti-sn65dsi86-aux",
	.regmap_config = &sn65dsix6_regmap,
	.is_chip_onboard = is_sn65dsix6_onboard,
	.pre_enable = sn65dsi86_pre_enable,
	.enable = sn65dsi86_enable,
	.disable = sn65dsi86_disable,
	.post_disable = NULL,
	.get_edid = ti_get_edid,
	.transfer = NULL,
	.set_clk = ti_set_clk
};

static int __init sn65dsix6_chip_register(void)
{
	int ret = 0;

	printk(KERN_DEBUG "sn65dsix6_chip_register +");

	ret = mipi2edp_bridge_chip_info_register(&sn65dsix6_bridge_chip_info);

	printk(KERN_DEBUG "sn65dsix6_chip_register -");

	return ret;
}

fs_initcall(sn65dsix6_chip_register);
