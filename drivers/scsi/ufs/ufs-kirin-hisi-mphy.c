/*
 * ufs-kirin-hisi-mphy.c
 *
 * config for ufs mhpy
 *
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd.
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

#define pr_fmt(fmt) "ufshcd :" fmt

#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include "ufshcd.h"
#include "ufs-kirin.h"

#define UFS_I2C_FMT_MSTNM_LEN 8
#define UFS_I2C_FMT_ADDR_LEN 2
#define HUFS_MPHY_CONFIG_SIZE 3
#define GEAR_RATE_CONFIG_SIZE 3

static struct i2c_board_info ufs_i2c_board_info = {
	.type = "i2c_ufs",
	.addr = UFS_I2C_SLAVE,
};

int ufs_i2c_readl(struct ufs_hba *hba, u32 *value, u32 addr)
{
	int ret = -1;
	u32 temp;
	struct hufs_host *host = NULL;

	host = hba->priv;
	if (unlikely(hba->host->is_emulator)) {
		dev_err(hba->dev, "not do %s on emu\n", __func__);
		return 0;
	}
	temp = cpu_to_be32(addr);
	if (host->i2c_client) {
		ret = i2c_master_send(
			host->i2c_client, (char *)(&temp), (int)sizeof(u32));
		if (ret < 0)
			pr_err("%s ufs_i2c_write fail\n", __func__);
		ret = i2c_master_recv(
			host->i2c_client, (char *)(&temp), (int)sizeof(u32));
		if (ret < 0)
			pr_err("%s fail\n", __func__);
	} else {
		pr_err("%s fail client empty\n", __func__);
	}
	*value = temp;
	return ret;
}

int ufs_i2c_writel(struct ufs_hba *hba, u32 val, u32 addr)
{
	int ret = -1;
	union i2c_fmt {
		unsigned char chars[UFS_I2C_FMT_MSTNM_LEN]; /* master name length */
		u32 addr_val[UFS_I2C_FMT_ADDR_LEN];
	} data;
	struct hufs_host *host = NULL;

	host = hba->priv;
	if (unlikely(hba->host->is_emulator)) {
		dev_err(hba->dev, "not do %s on emu\n", __func__);
		return 0;
	}
	data.addr_val[0] = cpu_to_be32(addr);
	data.addr_val[1] = val;
	if (host->i2c_client) {
		ret = i2c_master_send(host->i2c_client, (char *)data.chars,
		    (int)sizeof(union i2c_fmt));
		if (ret < 0)
			pr_err("%s ufs_i2c_write fail\n", __func__);
	} else {
		pr_err("%s ufs_i2c_write fail client empty\n", __func__);
	}
	return ret;
}

int create_i2c_client(struct ufs_hba *hba)
{
	struct hufs_host *host = NULL;
	struct i2c_adapter *adapter = NULL;

	host = hba->priv;
	adapter = i2c_get_adapter(UFS_I2C_BUS);
	if (!adapter) {
		pr_err("%s i2c_get_adapter error\n", __func__);
		return -EIO;
	}
	host->i2c_client = i2c_new_device(adapter, &ufs_i2c_board_info);
	if (!host->i2c_client) {
		pr_err("%s i2c_new_device error\n", __func__);
		return -EIO;
	}
	return 0;
}

/* select the I2C 's CS singnal for HUFS MPHY board in FPGA */
void i2c_chipsel_gpio_config(struct hufs_host *host, struct device *dev)
{
	int err;

	if (!host || !dev)
		return;

	host->chipsel_gpio = of_get_named_gpio(dev->of_node, "cs-gpios", 0);
	if (!gpio_is_valid(host->chipsel_gpio))
		pr_err("%s: gpio of host->chipsel_gpio is not valid,check DTS\n",
		    __func__);
	err = gpio_request((unsigned int)host->chipsel_gpio, "cs-gpio");
	if (err < 0)
		pr_err("Can`t request cs chipsel gpio %d\n",
		    host->chipsel_gpio);
	err = gpio_direction_output((unsigned int)host->chipsel_gpio, 0);
	if (err < 0)
		pr_err("%s: could not set gpio %d output push down\n", __func__,
		    host->chipsel_gpio);
}

void hufs_mphy_updata_temp_sqvref(
	struct ufs_hba *hba, struct hufs_host *host)
{
	if (!host)
		return;

	if ((host->caps & USE_HUFS_MPHY_TC)) {
		/*
		 * in low temperature to solve the PLL'S oscill
		 * RG_PLL_CP
		 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)RX_FSM_STATE, 0x0), 0x1);
		/* RG_PLL_DMY0 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)MC_HS_START_TIME_VAR_CAPABILITY, 0x0), 0x51);
		/*
		 * rate A->B's VC0 stable time
		 * RX enable lane 0
		 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00f0, 0x4), 0x1);
		/* RX enable lane 1 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00f0, 0x5), 0x1);
		/* H8's workaround */
		/* RX_SQ_VERF, lane 0 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00f1, 0x4), 0x7);
		/* RX_SQ_VERF, lane 1 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00f1, 0x5), 0x7);
	}
}

/* for hufs mphy v120 */
void hufs_mphy_updata_vswing_fsm_ocs5(
	struct ufs_hba *hba, struct hufs_host *host)
{
	uint32_t value = 0;

	if (!host)
		return;

	if ((host->caps & USE_HUFS_MPHY_TC)) {
		/* RX_MC_PRESENT */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00c2, 0x4), 0x1);
		/* RX_MC_PRESENT */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00c2, 0x5), 0x1);
		/*
		 * disable change
		 * meaure the power, can close it
		 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)0x00c7, 0x0), 0x3);
		/* meaure the power, can close it */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)0x00c8, 0x0), 0x3);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)0x007a, 0x0), 0x1c);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x007a, 0x1), 0x1c);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)0x007c, 0x0), 0xd4);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x007c, 0x1), 0xd4);
		/* RX_STALL */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00cf, 0x4), 0x2);
		/* RX_STALL */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00cf, 0x5), 0x2);
		/* RX_SLEEP */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00d0, 0x4), 0x2);
		/* RX_SLEEP */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00d0, 0x5), 0x2);
		/* RX_HS_CLK_EN */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00cc, 0x4), 0x3);
		/* RX_HS_CLK_EN */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00cc, 0x5), 0x3);
		/* RX_LS_CLK_EN */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00cd, 0x4), 0x3);
		/* RX_LS_CLK_EN */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00cd, 0x5), 0x3);
		/*
		 * enhance the accuracy of squelch detection
		 * RX_H8_EXIT
		 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00ce, 0x4), 0x3);
		/* RX_H8_EXIT */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00ce, 0x5), 0x3);

		/*
		 * try to solve the OCS=5
		 * RX_HS_DATA_VALID_TIMER_VAL0
		 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00e9, 0x4), 0x20);
		/* RX_HS_DATA_VALID_TIMER_VAL0 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00e9, 0x5), 0x20);
		/* RX_HS_DATA_VALID_TIMER_VAL1 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00ea, 0x4), 0x1);
		/* RX_HS_DATA_VALID_TIMER_VAL1 */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00ea, 0x5), 0x1);

		/* PA_TxHsG1SyncLength , can not set MPHY's register directly */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)PA_TXHSG1SYNCLENGTH, 0x0), 0x4F);
		/* PA_TxHsG2SyncLength , can not set MPHY's register directly */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)PA_TXHSG2SYNCLENGTH, 0x0), 0x4F);
		/* PA_TxHsG3SyncLength , can not set MPHY's register directly */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)PA_TXHSG3SYNCLENGTH, 0x0), 0x4F);

		/*
		 * enlarge TX_LS_PREPARE_LENGTH
		 * enable override
		 * Unipro VS_mphy_disable
		 */
		ufshcd_dme_get(hba, UIC_ARG_MIB_SEL((u32)0xd0f0, 0x0), &value);
		value |= (1 << TX_LS_PREPARE_LENGTH_OVREN);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)0xd0f0, 0x0), value);
		/* Set to max value 0xf */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)0xd0f4, 0x0), 0xf);
		/* update */
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)0xd085, 0x0), 0x1);
	}
}

void hufs_mphy_v200_updata_vswing_fsm(
	struct ufs_hba *hba, struct hufs_host *host)
{
	uint32_t value = 0;

	if (!host)
		return;

	/*
	 * set the HS-prepare length and sync length to MAX
	 * value, try to solve the data check error problem,
	 * the device seems not receive the write cmd.
	 */
	/* PA_TxHsG1SyncLength , can not set MPHY's register directly */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)PA_TXHSG1SYNCLENGTH, 0x0), 0x4F);
	/* PA_TxHsG2SyncLength , can not set MPHY's register directly */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)PA_TXHSG2SYNCLENGTH, 0x0), 0x4F);
	/* A_TxHsG3SyncLength , can not set MPHY's register directly */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)PA_TXHSG3SYNCLENGTH, 0x0), 0x4F);
	/* enlarge TX_LS_PREPARE_LENGTH */ /* enable override */
	ufshcd_dme_get(hba, UIC_ARG_MIB_SEL((u32)0xd0f0, 0x0), &value);
	/* Unipro VS_mphy_disable */
	value |= (1 << TX_LS_PREPARE_LENGTH_OVREN);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)0xd0f0, 0x0), value);
	/* Set to max value 0x6 */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)0xd0f4, 0x0), 0x6);
	/* update */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL((u32)0xd085, 0x0), 0x1);
}

void hufs_mphy_v200_link_post_config(
	struct ufs_hba *hba, struct hufs_host *host)
{
	uint32_t tx_lane_num = 1;
	uint32_t rx_lane_num = 1;
	uint32_t value = 0;
	uint32_t value_bak = 0;

	if (!host)
		return;

	/*
	 * set the PA_TActivate to 128. need to check in ASIC
	 * H8's workaround
	 * following register config para is vendor private register
	 */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_TACTIVATE, 0x0), 0x5);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x80da, 0x0), 0x2d);

	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_CONNECTEDTXDATALANES), &tx_lane_num);
	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_CONNECTEDRXDATALANES), &rx_lane_num);

	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(TX_MIN_STALL_NOCONFIG_TIME_CAPABILITY, 0x4), 0x1e);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00eb, 0x4), 0x64);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(TX_PHY_EDITORIAL_RELEASE_CAPABILITY, 0x4), 0xf0);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(MC_PWMGEAR_CAPABILITY, 0x0), 0x4b);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(MC_LS_TERMINATED_LINE_DRIVE_CAPABILITY, 0x0), 0xcb);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00c2, 0x4), 0x0);
	if (tx_lane_num > 1 && rx_lane_num > 1) {
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(TX_MIN_STALL_NOCONFIG_TIME_CAPABILITY, 0x5), 0x1e);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00eb, 0x5), 0x64);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(TX_PHY_EDITORIAL_RELEASE_CAPABILITY, 0x5), 0xf0);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00c2, 0x5), 0x0);
	}

	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xD085, 0x0), 0x1);

	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_TXHSG1SYNCLENGTH), &value);
	if (value < 0x4B)
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_TXHSG1SYNCLENGTH, 0x0), 0x4B);
	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_TXHSG2SYNCLENGTH), &value);
	if (value < 0x4C)
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_TXHSG2SYNCLENGTH, 0x0), 0x4C);
	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_TXHSG3SYNCLENGTH), &value);
	if (value < 0x4D)
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_TXHSG3SYNCLENGTH, 0x0), 0x4D);
	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_TXHSG4SYNCLENGTH), &value);
	if (value < 0x4E)
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_TXHSG4SYNCLENGTH, 0x0), 0x4E);

	ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(0x00ba, 0x0), &value_bak);
	ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(0x00ae, 0x0), &value);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00ae, 0x0), value | BIT(7));
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x00ba, 0x0), value_bak);
}

static const unsigned int hufs_mphy_v200_updata_config[][HUFS_MPHY_CONFIG_SIZE] = {
	{ 0x0009, 0x4, 0x1 },  { 0x0009, 0x5, 0x1 },  { 0x00df, 0x0, 0x3 },
	{ 0x0023, 0x0, 0x4 },  { 0x0023, 0x1, 0x4 },  { 0x00a3, 0x4, 0x4 },
	{ 0x00a3, 0x5, 0x4 },  { 0x00f1, 0x4, 0x4 },  { 0x00f1, 0x5, 0x4 },
	{ 0x0003, 0x4, 0x0a }, { 0x0003, 0x5, 0x0a }, { 0x0004, 0x4, 0x64 },
	{ 0x0004, 0x5, 0x64 }, { 0x00cf, 0x4, 0x2 },  { 0x00cf, 0x5, 0x2 },
	{ 0x00d0, 0x4, 0x2 },  { 0x00d0, 0x5, 0x2 },  { 0x00f0, 0x4, 0x1 },
	{ 0x00f0, 0x5, 0x1 },  { 0x0059, 0x0, 0x0f }, { 0x0059, 0x1, 0x0f },
	{ 0x005a, 0x0, 0x0 },  { 0x005a, 0x1, 0x0 },  { 0x005b, 0x0, 0x0f },
	{ 0x005b, 0x1, 0x0f }, { 0x005c, 0x0, 0x0 },  { 0x005c, 0x1, 0x0 },
	{ 0x005d, 0x0, 0x0 },  { 0x005d, 0x1, 0x0 },  { 0x005e, 0x0, 0x0a },
	{ 0x005e, 0x1, 0x0a }, { 0x005f, 0x0, 0x0a }, { 0x005f, 0x1, 0x0a },
	{ 0x007a, 0x0, 0x0 },  { 0x007a, 0x1, 0x0 },  { 0x007b, 0x0, 0x0 },
	{ 0x007b, 0x1, 0x0 },  { 0x00c3, 0x4, 0x4 },  { 0x00c3, 0x5, 0x4 },
	{ 0x00f6, 0x4, 0x1 },  { 0x00f6, 0x5, 0x1 },  { 0x00c7, 0x0, 0x3 },
	{ 0x00c8, 0x0, 0x3 },  { 0x00c5, 0x0, 0x3 },  { 0x00c6, 0x0, 0x3 },
	{ 0x00e9, 0x4, 0x0 },  { 0x00e9, 0x5, 0x0 },  { 0x00ea, 0x4, 0x10 },
	{ 0x00ea, 0x5, 0x10 }, { 0x00f4, 0x4, 0x0 },  { 0x00f4, 0x5, 0x0 },
	{ 0x00f3, 0x4, 0x0 },  { 0x00f3, 0x5, 0x0 },  { 0x00f2, 0x4, 0x3 },
	{ 0x00f2, 0x5, 0x3 },  { 0x00f6, 0x4, 0x2 },  { 0x00f6, 0x5, 0x2 },
	{ 0x00f5, 0x4, 0x0 },  { 0x00f5, 0x5, 0x0 },  { 0x00fc, 0x4, 0x1f },
	{ 0x00fc, 0x5, 0x1f }, { 0x00fd, 0x4, 0x0 },  { 0x00fd, 0x5, 0x0 },
	{ 0x00fb, 0x4, 0x5 },  { 0x00fb, 0x5, 0x5 },  { 0x0011, 0x4, 0x11 },
	{ 0x0011, 0x5, 0x11 }, { 0xd085, 0x0, 0x1 }
};
void hufs_mphy_v200_updata(struct ufs_hba *hba, struct hufs_host *host)
{
	unsigned int i;

	if (!host)
		return;

	/* in low temperature to solve the PLL'S oscill */
	for (i = 0; i < ARRAY_SIZE(hufs_mphy_v200_updata_config); i++)
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(
					    hufs_mphy_v200_updata_config[i][0],
					    hufs_mphy_v200_updata_config[i][1]),
			       hufs_mphy_v200_updata_config[i][2]); /* arr size is 3 */
	/* Trigger UniPro update */
	mdelay(40); /* wait 40ms */
}

static const unsigned int hufs_mphy_v200_pwr_change_fast[][HUFS_MPHY_CONFIG_SIZE] = {
	{ 0x0050, 0x0, 0x0 },  { 0x0050, 0x1, 0x0 },  { 0x0051, 0x0, 0x0 },
	{ 0x0051, 0x1, 0x0 },  { 0x0052, 0x0, 0x0 },  { 0x0052, 0x1, 0x0 },
	{ 0x0056, 0x0, 0x0 },  { 0x0056, 0x1, 0x0 },  { 0x0057, 0x0, 0x0 },
	{ 0x0057, 0x1, 0x0 },  { 0x0058, 0x0, 0x0 },  { 0x0058, 0x1, 0x0 },
	{ 0x00a1, 0x0, 0x0 },  { 0x00a1, 0x1, 0x0 },  { 0x00a4, 0x0, 0x0 },
	{ 0x00a4, 0x1, 0x0 },  { 0x00a2, 0x0, 0x0b }, { 0x00a2, 0x1, 0x0b },
	{ 0x00a5, 0x0, 0x0 },  { 0x00a5, 0x1, 0x0 },  { 0x00a3, 0x0, 0x1f },
	{ 0x00a3, 0x1, 0x1f }, { 0x00a6, 0x0, 0x9 },  { 0x00a6, 0x1, 0x9 }
};

static const unsigned int hufs_mphy_v200_pwr_change_slow[][HUFS_MPHY_CONFIG_SIZE] = {
	{ 0x0050, 0x0, 0x0 },  { 0x0050, 0x1, 0x0 },  { 0x0051, 0x0, 0x0 },
	{ 0x0051, 0x1, 0x0 },  { 0x0052, 0x0, 0x0 },  { 0x0052, 0x1, 0x0 },
	{ 0x0056, 0x0, 0x0 },  { 0x0056, 0x1, 0x0 },  { 0x0057, 0x0, 0x0 },
	{ 0x0057, 0x1, 0x0 },  { 0x0058, 0x0, 0xf },  { 0x0058, 0x1, 0xf },
	{ 0x00a1, 0x0, 0x0 },  { 0x00a1, 0x1, 0x0 },  { 0x00a4, 0x0, 0x0 },
	{ 0x00a4, 0x1, 0x0 },  { 0x00a2, 0x0, 0x0b }, { 0x00a2, 0x1, 0x0b },
	{ 0x00a5, 0x0, 0x0 },  { 0x00a5, 0x1, 0x0 },  { 0x00a3, 0x0, 0x1f },
	{ 0x00a3, 0x1, 0x1f }, { 0x00a6, 0x0, 0x1f }, { 0x00a6, 0x1, 0x1f }
};

void hufs_mphy_v200_pwr_change_pre_config(
	struct ufs_hba *hba, struct hufs_host *host)
{
	unsigned int i;

	if (!host)
		return;

	if (hba->pwr_info.pwr_rx == FAST_MODE ||
		hba->pwr_info.pwr_rx == FASTAUTO_MODE) {
		/* for HS Gear 1 (max HS gear 1 on FPGA) only */
		for (i = 0; i < ARRAY_SIZE(hufs_mphy_v200_pwr_change_fast); i++)
			ufshcd_dme_set(hba,
				UIC_ARG_MIB_SEL(
					hufs_mphy_v200_pwr_change_fast[i][0],
					hufs_mphy_v200_pwr_change_fast[i][1]),
				hufs_mphy_v200_pwr_change_fast[i][2]); /* arr size is 3 */
	} else {
		for (i = 0; i < ARRAY_SIZE(hufs_mphy_v200_pwr_change_slow); i++)
			ufshcd_dme_set(hba,
				UIC_ARG_MIB_SEL(
					hufs_mphy_v200_pwr_change_slow[i][0],
					hufs_mphy_v200_pwr_change_slow[i][1]),
				hufs_mphy_v200_pwr_change_slow[i][2]); /* arr size is 3 */
	}

	/* set PA_HIBERN8TIME to be 0xa for sandisk 2.1 */
	if (hba->manufacturer_id == UFS_VENDOR_SANDISK &&
		hba->ufs_device_spec_version == UFSHCI_VERSION_21)
		ufshcd_dme_set(hba, UIC_ARG_MIB((u32)PA_HIBERN8TIME), 0xA);
}

static const unsigned int gear1_rateb_config[][GEAR_RATE_CONFIG_SIZE] = {
	{ 0x00c2, 0x0, 0x0 },  { 0x00c9, 0x0, 0x0 }, /* RG_PLL_SWC_EN */
	{ 0x00c4, 0x0, 0x1 }, /* RG_PLL_FBK_S */
	{ 0x00c3, 0x0, 0x4c }, /* RG_PLL_FBK_P */
	{ 0x00cf, 0x0, 0x2 }, /* RG_PLL_TXHSGR */
	{ 0x00cd, 0x0, 0x2 } /* RG_PLL_RXHSGR */
};

static const unsigned int gear1_ratea_config[][GEAR_RATE_CONFIG_SIZE] = {
	{ 0x00c2, 0x0, 0x0 }, /* RG_PLL_PRE_DIV */
	{ 0x00c9, 0x0, 0x0 }, /* RG_PLL_SWC_EN */
	{ 0x00c4, 0x0, 0x1 }, /* RG_PLL_FBK_S */
	{ 0x00c3, 0x0, 0x41 }, /* RG_PLL_FBK_P */
	{ 0x00cf, 0x0, 0x2 }, /* RG_PLL_TXHSGR */
	{ 0x00cd, 0x0, 0x2 } /* RG_PLL_RXHSGR */
};

static const unsigned int gear2_rateb_config[][GEAR_RATE_CONFIG_SIZE] = {
	{ 0x00c2, 0x0, 0x0 },   { 0x00c9, 0x0, 0x0 }, { 0x00c4, 0x1, 0x1 },
	{ 0x00c3, 0x4c, 0x4c }, { 0x00cf, 0x1, 0x1 }, { 0x00cd, 0x1, 0x1 }
};

static const unsigned int gear2_ratea_config[][GEAR_RATE_CONFIG_SIZE] = {
	{ 0x00c2, 0x0, 0x0 },   { 0x00c9, 0x0, 0x0 }, { 0x00c4, 0x1, 0x1 },
	{ 0x00c3, 0x41, 0x41 }, { 0x00cf, 0x1, 0x1 }, { 0x00cd, 0x1, 0x1 }
};

static const unsigned int gear3_rateb_config[][GEAR_RATE_CONFIG_SIZE] = {
	{ 0x00c2, 0x0, 0x0 },   { 0x00c9, 0x0, 0x0 }, { 0x00c4, 0x1, 0x1 },
	{ 0x00c3, 0x4c, 0x4c }, { 0x00cf, 0x1, 0x0 }, { 0x00cd, 0x1, 0x0 }
};

static const unsigned int gear3_ratea_config[][GEAR_RATE_CONFIG_SIZE] = {
	{ 0x00c2, 0x0, 0x0 },   { 0x00c9, 0x0, 0x0 }, { 0x00c4, 0x1, 0x1 },
	{ 0x00c3, 0x41, 0x41 }, { 0x00cf, 0x1, 0x0 }, { 0x00cd, 0x1, 0x0 }
};

#define ufs_dme_set_gear_rate(gear_rate_config)                                \
	do {                                                                   \
		for (i = 0; i < ARRAY_SIZE(gear_rate_config); i++)           \
			ufshcd_dme_set(hba,                                    \
				UIC_ARG_MIB_SEL((gear_rate_config)[i][0],      \
					(gear_rate_config)[i][1]),             \
				(gear_rate_config)[i][2]);                     \
	} while (0)

void adapt_pll_to_power_mode(struct ufs_hba *hba)
{
	int i;
	struct hufs_host *host = NULL;

	host = hba->priv;
	if (host->caps & USE_HS_GEAR1) {
		if (host->caps & USE_RATE_B)
			/* RG_PLL_PRE_DIV */
			ufs_dme_set_gear_rate(gear1_rateb_config);
		else
			ufs_dme_set_gear_rate(gear1_ratea_config);
	} else if (host->caps & USE_HS_GEAR2) {
		if (host->caps & USE_RATE_B)
			ufs_dme_set_gear_rate(gear2_rateb_config);
		else
			ufs_dme_set_gear_rate(gear2_ratea_config);
	} else if (host->caps & USE_HS_GEAR3) {
		if (host->caps & USE_RATE_B)
			ufs_dme_set_gear_rate(gear3_rateb_config);
		else
			ufs_dme_set_gear_rate(gear3_ratea_config);
	}
}

/* modify different from fastbooo */
void deemphasis_config(
	struct hufs_host *host, struct ufs_hba *hba,
	struct ufs_pa_layer_attr *dev_req_params)
{
	uint32_t value = 0;

	if (!host || !dev_req_params)
		return;

	if (host->caps & USE_HUFS_MPHY_TC) {
		/*
		 * the de-emphasis level you want to select, for example ,
		 * value = 0x5, it's 3.67 dB
		 * following register config para is vendor private register
		 */
		if (host->caps & USE_HS_GEAR3) {
			value = 0x26; /* 4.44 db */
			ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x007e, 0x0), 0x5);
			ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(TX_AMPLITUDE, 0x0), 0x22);
			ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x007d, 0x0), 0x22);
			if ((dev_req_params->lane_tx > 1) &&
				(dev_req_params->lane_rx > 1)) {
				ufshcd_dme_set(
					hba, UIC_ARG_MIB_SEL(0x007e, 0x1), 0x5);
				ufshcd_dme_set(hba,
					UIC_ARG_MIB_SEL(0x0025, 0x1), 0x22);
				ufshcd_dme_set(hba,
					UIC_ARG_MIB_SEL(0x007d, 0x1), 0x22);
			}
		} else {
			value = 0x6f; /* 6.85 db */
			ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x007e, 0x0), 0x5);
			ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(TX_AMPLITUDE, 0x0), 0x15);
			ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x007d, 0x0), 0x15);
			if ((dev_req_params->lane_tx > 1) &&
				(dev_req_params->lane_rx > 1)) {
				ufshcd_dme_set(
					hba, UIC_ARG_MIB_SEL(0x007e, 0x1), 0x5);
				ufshcd_dme_set(hba,
					UIC_ARG_MIB_SEL(0x0025, 0x1), 0x15);
				ufshcd_dme_set(hba,
					UIC_ARG_MIB_SEL(0x007d, 0x1), 0x15);
			}
		}
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(TX_HS_EQUALIZER_SETTING, 0x0), value);
		ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0x007b, 0x0), value);
		if ((dev_req_params->lane_tx > 1) &&
			(dev_req_params->lane_rx > 1)) {
			ufshcd_dme_set(
				hba, UIC_ARG_MIB_SEL(0x0037, 0x1), value);
			ufshcd_dme_set(
				hba, UIC_ARG_MIB_SEL(0x007b, 0x1), value);
		}
	}
}
