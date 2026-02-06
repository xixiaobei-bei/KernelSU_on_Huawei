/*
 * ufs-hisi.c
 *
 * ufs configuration for pisces
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define pr_fmt(fmt) "ufshcd :" fmt

#include "ufs-hisi.h"
#include <linux/bootdevice.h>
#include <linux/dma-mapping.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/vmalloc.h>
#include <soc_crgperiph_interface.h>
#include <soc_sctrl_interface.h>
#include <soc_ufs_sysctrl_interface.h>
#include <soc_ufs_dme_interface.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/cpuhotplug.h>

#include "dsm_ufs.h"
#include "ufs-kirin.h"
#include "ufshcd.h"
#include "ufshcd-kirin-interface.h"
#include "ufs-hisi-dump.h"
#include "ufshci.h"
#include "unipro.h"

#ifdef CONFIG_HISI_DEBUG_FS
#define HUFS_BUG() BUG()
#else
#define HUFS_BUG()                                                         \
	do {                                                                   \
	} while (0)
#endif

#define UFS_DME_LMDM_ATTR_WR_EN_PEER UFS_BIT(28)
#define UFS_DME_LMDM_ATTR_VALID_PEER UFS_BIT(31)
#define COMPLETION_TIMEOUT_RETRY_TIMES 2000
#define COMPLETION_TIMEOUT_RETRY_UDELAY 10

u32 g_deemp_n3_5_db[RT_LEVEL_NUM] = { 0x3, 0x3, 0x3, 0x3, 0x7, 0x7, 0x7,
				0x7, 0x7, 0x7, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF };

u32 g_deemp_n6_db[RT_LEVEL_NUM] = { 0x7, 0x7, 0x7, 0xF, 0xF, 0xF, 0xF,
			0x1F, 0x1F, 0x1F, 0x1F, 0x3F, 0x3F, 0x3F, 0x3F, 0x7F };

/* PMC to fast/fast auto Gear 1 */
static const struct ufs_attr_cfg hufs_mphy_v200_tc_pre_pmc_fsg1_attr[] = {
	{ 0x00000050, 0x00000000 },
	{ 0x00010050, 0x00000000 },
	{ 0x00000051, 0x00000000 },
	{ 0x00010051, 0x00000000 },
	{ 0x00000052, 0x00000000 },
	{ 0x00010052, 0x00000000 },
	{ 0x00000056, 0x00000000 },
	{ 0x00010056, 0x00000000 },
	{ 0x00000057, 0x00000000 },
	{ 0x00010057, 0x00000000 },
	{ 0x00000058, 0x0000000F },
	{ 0x00010058, 0x0000000F },
	{ 0x000000A1, 0x00000000 },
	{ 0x000100A1, 0x00000000 },
	{ 0x000000A4, 0x00000000 },
	{ 0x000100A4, 0x00000000 },
	{ 0x000000A2, 0x0000000B },
	{ 0x000100A2, 0x0000000B },
	{ 0x000000A5, 0x00000000 },
	{ 0x000100A5, 0x00000000 },
	{ 0x000000A3, 0x0000001F },
	{ 0x000100A3, 0x0000001F },
	{ 0x000000A6, 0x00000009 },
	{ 0x000100A6, 0x00000009 },
	{ 0, 0 },
};

/* PMC to fast/fast auto Gear 2 */
static const struct ufs_attr_cfg hufs_mphy_v200_tc_pre_pmc_fsg2_attr[] = {
	{ 0x00000050, 0x00000010 },
	{ 0x00010050, 0x00000010 },
	{ 0x00000051, 0x00000000 },
	{ 0x00010051, 0x00000000 },
	{ 0x00000052, 0x00000000 },
	{ 0x00010052, 0x00000000 },
	{ 0x00000056, 0x00000000 },
	{ 0x00010056, 0x00000000 },
	{ 0x00000057, 0x00000001 },
	{ 0x00010057, 0x00000001 },
	{ 0x00000058, 0x0000000F },
	{ 0x00010058, 0x0000000F },
	{ 0x000000A1, 0x00000000 },
	{ 0x000100A1, 0x00000000 },
	{ 0x000000A4, 0x00000000 },
	{ 0x000100A4, 0x00000000 },
	{ 0x000000A2, 0x0000000B },
	{ 0x000100A2, 0x0000000B },
	{ 0x000000A5, 0x00000000 },
	{ 0x000100A5, 0x00000000 },
	{ 0x000000A3, 0x0000001F },
	{ 0x000100A3, 0x0000001F },
	{ 0x000000A6, 0x0000000D },
	{ 0x000100A6, 0x0000000D },
	{ 0, 0 },
};

/* PMC to fast/fast auto Gear 3 */
static const struct ufs_attr_cfg hufs_mphy_v200_tc_pre_pmc_fsg3_attr[] = {
	{ 0x00000050, 0x00000010 },
	{ 0x00010050, 0x00000010 },
	{ 0x00000051, 0x00000000 },
	{ 0x00010051, 0x00000000 },
	{ 0x00000052, 0x00000000 },
	{ 0x00010052, 0x00000000 },
	{ 0x00000056, 0x00000000 },
	{ 0x00010056, 0x00000000 },
	{ 0x00000057, 0x00000007 },
	{ 0x00010057, 0x00000007 },
	{ 0x00000058, 0x0000000F },
	{ 0x00010058, 0x0000000F },
	{ 0x000000A1, 0x00000000 },
	{ 0x000100A1, 0x00000000 },
	{ 0x000000A4, 0x00000002 },
	{ 0x000100A4, 0x00000002 },
	{ 0x000000A2, 0x0000000B },
	{ 0x000100A2, 0x0000000B },
	{ 0x000000A5, 0x00000000 },
	{ 0x000100A5, 0x00000000 },
	{ 0x000000A3, 0x0000001F },
	{ 0x000100A3, 0x0000001F },
	{ 0x000000A6, 0x0000001F },
	{ 0x000100A6, 0x0000001F },
	{ 0, 0 },
};

/* PMC to fast/fast auto Gear 4 */
static const struct ufs_attr_cfg hufs_mphy_v200_tc_pre_pmc_fsg4_attr[] = {
	{ 0x00000050, 0x00000010 },
	{ 0x00010050, 0x00000010 },
	{ 0x00000051, 0x00000000 },
	{ 0x00010051, 0x00000000 },
	{ 0x00000052, 0x00000000 },
	{ 0x00010052, 0x00000000 },
	{ 0x00000056, 0x00000000 },
	{ 0x00010056, 0x00000000 },
	{ 0x00000057, 0x0000001F },
	{ 0x00010057, 0x0000001F },
	{ 0x00000058, 0x0000000F },
	{ 0x00010058, 0x0000000F },
	{ 0x000000A1, 0x00000000 },
	{ 0x000100A1, 0x00000000 },
	{ 0x000000A4, 0x00000023 },
	{ 0x000100A4, 0x00000023 },
	{ 0x000000A2, 0x0000000B },
	{ 0x000100A2, 0x0000000B },
	{ 0x000000A5, 0x00000000 },
	{ 0x000100A5, 0x00000000 },
	{ 0x000000A3, 0x0000001F },
	{ 0x000100A3, 0x0000001F },
	{ 0x000000A6, 0x0000001F },
	{ 0x000100A6, 0x0000001F },
	{ 0, 0 },
};

/* PMC to slow/slow */
static const struct ufs_attr_cfg hufs_mphy_v200_tc_pre_pmc_slow_attr[] = {
	{ 0x00000050, 0x00000000 },
	{ 0x00010050, 0x00000000 },
	{ 0x00000051, 0x00000000 },
	{ 0x00010051, 0x00000000 },
	{ 0x00000052, 0x00000000 },
	{ 0x00010052, 0x00000000 },
	{ 0x00000056, 0x00000000 },
	{ 0x00010056, 0x00000000 },
	{ 0x00000057, 0x00000000 },
	{ 0x00010057, 0x00000000 },
	{ 0x00000058, 0x0000000F },
	{ 0x00010058, 0x0000000F },
	{ 0x000000A1, 0x00000000 },
	{ 0x000100A1, 0x00000000 },
	{ 0x000000A4, 0x00000000 },
	{ 0x000100A4, 0x00000000 },
	{ 0x000000A2, 0x0000000B },
	{ 0x000100A2, 0x0000000B },
	{ 0x000000A5, 0x00000000 },
	{ 0x000100A5, 0x00000000 },
	{ 0x000000A3, 0x0000001F },
	{ 0x000100A3, 0x0000001F },
	{ 0x000000A6, 0x0000001f },
	{ 0x000100A6, 0x0000001f },
	{ 0, 0 },
};

/* PMC to Fast/Fast auto G1 */
static const struct ufs_attr_cfg hufs_mphy_v300_pre_pmc_fsg1_attr[] = {
	{ 0x00000057, 0x00000000 }, /* RG_TX0_SL_EN_POST[7:0] TX 0x57 */
	{ 0x00010057, 0x00000000 }, /* RG_TX1_SL_EN_POST[7:0] TX 0x57 */
	{ 0x00020013, 0x00000012 }, /* RX0 CDR KI/KP, RX 0x13 */
	{ 0x00030013, 0x00000012 }, /* RX1 CDR KI/KP, RX 0x13 */
	{ 0x000200F3, 0x00000000 }, /* RX0 RX_EQ_SEL_I, RX 0xF3 */
	{ 0x000300F3, 0x00000000 }, /* RX1 RX_EQ_SEL_I, RX 0xF3 */
	{ 0x0002000F, 0x0000000F }, /* RX0 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
	{ 0x0003000F, 0x0000000F }, /* RX1 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
	{ 0, 0 },
};

/* PMC to Fast/Fast auto G2 */
static const struct ufs_attr_cfg hufs_mphy_v300_pre_pmc_fsg2_attr[] = {
	{ 0x00000057, 0x00000000 }, /* RG_TX0_SL_EN_POST[7:0] TX 0x57 */
	{ 0x00010057, 0x00000000 }, /* RG_TX1_SL_EN_POST[7:0] TX 0x57 */
	{ 0x00020013, 0x00000012 }, /* RX0 CDR KI/KP, RX 0x13 */
	{ 0x00030013, 0x00000012 }, /* RX1 CDR KI/KP, RX 0x13 */
	{ 0x000200F3, 0x00000000 }, /* RX0 RX_EQ_SEL_I, RX 0xF3 */
	{ 0x000300F3, 0x00000000 }, /* RX1 RX_EQ_SEL_I, RX 0xF3 */
	{ 0x0002000F, 0x0000000E }, /* RX0 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
	{ 0x0003000F, 0x0000000E }, /* RX1 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
	{ 0, 0 },
};

/* PMC to Fast/Fast auto G3 */
static const struct ufs_attr_cfg hufs_mphy_v300_pre_pmc_fsg3_attr[] = {
	{ 0x00000057, 0x00000000 }, /* RG_TX0_SL_EN_POST[7:0] TX 0x57 */
	{ 0x00010057, 0x00000000 }, /* RG_TX1_SL_EN_POST[7:0] TX 0x57 */
	{ 0x00020013, 0x00000012 }, /* RX0 CDR KI/KP, RX 0x13 */
	{ 0x00030013, 0x00000012 }, /* RX1 CDR KI/KP, RX 0x13 */
	{ 0x000200F3, 0x00000000 }, /* RX0 RX_EQ_SEL_I, RX 0xF3 */
	{ 0x000300F3, 0x00000000 }, /* RX1 RX_EQ_SEL_I, RX 0xF3 */
	{ 0x0002000F, 0x00000001 }, /* RX0 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
	{ 0x0003000F, 0x00000001 }, /* RX1 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
	{ 0, 0 },
};

/* PMC to Fast/Fast auto G4 */
static const struct ufs_attr_cfg hufs_mphy_v300_pre_pmc_fsg4_attr[] = {
	{ 0x00000057, 0x00000000 }, /* RG_TX0_SL_EN_POST[7:0] TX 0x57 */
	{ 0x00010057, 0x00000000 }, /* RG_TX1_SL_EN_POST[7:0] TX 0x57 */
	{ 0x00020013, 0x00000012 }, /* RX0 CDR KI/KP, RX 0x13 */
	{ 0x00030013, 0x00000012 }, /* RX1 CDR KI/KP, RX 0x13 */
	{ 0x000200F3, 0x00000001 }, /* RX0 RX_EQ_SEL_I, RX 0xF3 */
	{ 0x000300F3, 0x00000001 }, /* RX1 RX_EQ_SEL_I, RX 0xF3 */
	{ 0x0002000F, 0x00000000 }, /* RX0 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
	{ 0x0003000F, 0x00000000 }, /* RX1 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
	{ 0, 0 },
};

/* cdr bw value for gear1~gear4 */
static const hufs_mphy_v300_pre_pmc_cdr_bw[] = { 0x32, 0x22, 0x10, 0x00 };
#define SAMSUNG_CDR_BW 0x2

/* PMC to slow/slow auto */
static const struct ufs_attr_cfg hufs_mphy_v300_pre_pmc_slow_attr[] = {
	{ 0x00000057, 0x00000000 }, /* RG_TX0_SL_EN_POST[7:0] TX 0x57 */
	{ 0x00010057, 0x00000000 }, /* RG_TX1_SL_EN_POST[7:0] TX 0x57 */
	{ 0x00020013, 0x00000000 }, /* RX0 CDR KI/KP, RX 0x13 */
	{ 0x00030013, 0x00000000 }, /* RX1 CDR KI/KP, RX 0x13 */
	{ 0x000200F3, 0x00000000 }, /* RX0 RX_EQ_SEL_I, RX 0xF3 */
	{ 0x000300F3, 0x00000000 }, /* RX1 RX_EQ_SEL_I, RX 0xF3 */
	{ 0x0002000F, 0x00000000 }, /* RX0 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
	{ 0x0003000F, 0x00000000 }, /* RX1 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
	{ 0, 0 },
};

int hufs_set_each_cfg_attr(struct ufs_hba *hba, struct ufs_attr_cfg *cfg)
{
	if (!cfg)
		return 0;

	while (cfg->addr != 0) {
		hufs_uic_write_reg(hba, cfg->addr, cfg->val);
		cfg++;
	}
	return 0;
}

static bool is_dme_reg(u32 reg_offset)
{
	/* check if BIT(12)-BIT(15) is D */
	if (((reg_offset >> 12) & 0xF) == 0xD)
		return true;
	else
		return false;
}

int hufs_uic_write_reg(struct ufs_hba *hba, u32 reg_offset, u32 value)
{
	int ret;
	unsigned int h8_state;
	int retries = HUFS_UIC_ACCESS_REG_RETRIES;
	bool auto_h8_disabled = false;

	if (hba->autoh8_disable_depth == 0 && !is_dme_reg(reg_offset)) {
		ret = ufshcd_hufs_disable_auto_hibern8(hba);
		if (ret) {
			dev_err(hba->dev,
				"disable autoh8 fail, skip uic reg access\n");
			return ret;
		}
		auto_h8_disabled = 1;
	}
	h8_state = ufshcd_readl(hba, UFS_AUTO_H8_STATE_OFF);
	if ((h8_state & UFS_HC_AH8_STATE) != AH8_XFER)
		dev_info(hba->dev, "ufs auto_h8 status 0x%x\n", h8_state);

	do {
		writel(value, hba->ufs_unipro_base +
				      (reg_offset << HUFS_UNIPRO_BIT_SHIFT));
		ret = (u32)readl(
			hba->ufs_unipro_base +
			(DME_LOCAL_OPC_STATE << HUFS_UNIPRO_BIT_SHIFT));
		ufsdbg_error_inject_dispatcher(hba, ERR_INJECT_UIC_CMD_ERR,
					       UIC_CMD_RESULT_SUCCESS, &ret);
	} while ((ret == LOCAL_ATTR_BUSY) && retries--);

	if (ret && ret != INVALID_MIB_ATTRIBUTE)
		dev_err(hba->dev, "hufs_uic_write [0x%x] error : 0x%x\n",
			reg_offset, ret);
	else if (ret == INVALID_MIB_ATTRIBUTE && hba->invalid_attr_print)
		dev_err(hba->dev, "invalid mib attr 0x%x\n", reg_offset);
	if (auto_h8_disabled)
		ufshcd_hufs_enable_auto_hibern8(hba);

	return ret;
}

int hufs_uic_read_reg(struct ufs_hba *hba, u32 reg_offset, u32 *value)
{
	int ret;
	unsigned int h8_state;
	int retries = HUFS_UIC_ACCESS_REG_RETRIES;
	bool auto_h8_disabled = false;

	if (!value) {
		pr_err("%s: Invalid value!\n", __func__);
		return -EINVAL;
	}

	/* only when auto hibern8 is enabled, disable auto hibern8 is needed */
	if (hba->autoh8_disable_depth == 0 && !is_dme_reg(reg_offset)) {
		ret = ufshcd_hufs_disable_auto_hibern8(hba);
		if (ret) {
			dev_err(hba->dev,
				"disable autoh8 fail, skip uic reg access\n");
			return ret;
		}
		auto_h8_disabled = 1;
	}
	h8_state = ufshcd_readl(hba, UFS_AUTO_H8_STATE_OFF);
	if ((h8_state & UFS_HC_AH8_STATE) != AH8_XFER)
		dev_info(hba->dev, "ufs auto_h8 status 0x%x\n", h8_state);

	do {
		*value = (u32)readl(hba->ufs_unipro_base +
				    (reg_offset << HUFS_UNIPRO_BIT_SHIFT));
		ret = (u32)readl(
			hba->ufs_unipro_base +
			(DME_LOCAL_OPC_STATE << HUFS_UNIPRO_BIT_SHIFT));
		ufsdbg_error_inject_dispatcher(hba, ERR_INJECT_UIC_CMD_ERR,
			UIC_CMD_RESULT_SUCCESS, &ret);
	} while ((ret == LOCAL_ATTR_BUSY) && retries--);
	if (ret && ret != INVALID_MIB_ATTRIBUTE)
		dev_err(hba->dev, "hufs_uic_read [0x%x] error : 0x%x\n",
			reg_offset, ret);
	else if (ret == INVALID_MIB_ATTRIBUTE && hba->invalid_attr_print)
		dev_err(hba->dev, "invalid mib attr 0x%x\n", reg_offset);
	if (auto_h8_disabled)
		ufshcd_hufs_enable_auto_hibern8(hba);

	return ret;
}

int hufs_uic_peer_set(struct ufs_hba *hba, u32 offset, u32 value)
{
	u32 ctrl_dw;

	hufs_uic_write_reg(hba, DME_PEER_OPC_WDATA, value);

	ctrl_dw = offset | UFS_DME_LMDM_ATTR_WR_EN_PEER |
		  UFS_DME_LMDM_ATTR_VALID_PEER;
	return hufs_uic_write_reg(hba, DME_PEER_OPC_CTRL, ctrl_dw);
}

int hufs_uic_peer_get(struct ufs_hba *hba, u32 offset, u32 *value)
{
	u32 ctrl_dw;
	int ret = 0;

	ctrl_dw = offset | UFS_DME_LMDM_ATTR_VALID_PEER;

	init_completion(&hba->uic_peer_get_done);
	hufs_uic_write_reg(hba, DME_PEER_OPC_CTRL, ctrl_dw);

	if (wait_for_completion_timeout(
		    &hba->uic_peer_get_done,
		    msecs_to_jiffies(HUFS_UIC_ACCESS_REG_TIMEOUT)))
		hufs_uic_read_reg(hba, DME_PEER_OPC_RDATA, value);
	else
		ret = -ETIMEDOUT;

	return ret;
}

/***************************************************************
 *
 * snps_to_hufs_addr
 * Description: the address transltation is different between
 *     HUFS and SNPS UFS. For backward compatible, use this function
 *     to translate the address.
 *
 ***************************************************************/
/* SNPS addr coding : 0x REG_addr | select
 * example: TX 0x41 lane0 --> 0x00410000
 *      TX 0x41 lane1 --> 0x00410001
 *      RX 0xC1 lane0 --> 0x00C10004
 *      RX 0xC1 lane1 --> 0x00C10005
 *      HUFS addr coding : 0x select | Reg_addr
 *      example: TX 0x41 lane0 --> 0x00000041
 *      TX 0x41 lane1 --> 0x00010041
 *      RX 0xC1 lane0 --> 0x000200C1
 *      RX 0xC1 lane1 --> 0x000300C1
 */
u32 snps_to_hufs_addr(u32 cmd, u32 arg1)
{
	u32 temp;
	u32 rx0_sel = HUFS_RX0_SEL;
	u32 rx1_sel = HUFS_RX1_SEL;

	/* 0xD0850000 is vendor private register */
	if (arg1 == 0xD0850000) {
		temp = 0x0000D014;
		return temp;
	}
	if (cmd == UIC_CMD_DME_PEER_SET || cmd == UIC_CMD_DME_PEER_GET) {
		/* If peer operation, we should use spec-defined SEL */
		rx0_sel = SPEC_RX0_SEL;
		rx1_sel = SPEC_RX1_SEL;
	}

	temp = ((arg1 & UIC_ADDR_MASK) >> UIC_SHIFT);
	temp &= UIC_SLE_MASK;
	if ((arg1 & UIC_SLE_MASK) == TX0_SEL)
		temp |= (HUFS_TX0_SEL << UIC_SHIFT);
	else if ((arg1 & UIC_SLE_MASK) == TX1_SEL)
		temp |= (HUFS_TX1_SEL << UIC_SHIFT);
	else if ((arg1 & UIC_SLE_MASK) == RX0_SEL)
		temp |= (rx0_sel << UIC_SHIFT);
	else if ((arg1 & UIC_SLE_MASK) == RX1_SEL)
		temp |= (rx1_sel << UIC_SHIFT);
	else
		pr_err("invalid mphy gen_selector_index\n");

	return temp;
}

int hufs_dme_link_startup(struct ufs_hba *hba)
{
	int ret = 0;
	u32 value = 0;
	u32 link_state;

	hufs_uic_read_reg(hba, DME_UNIPRO_STATE, &value);
	link_state = value & MASK_DEBUG_UNIPRO_STATE;
	if (link_state != LINK_DOWN) {
		dev_err(hba->dev, "unipro is not linkdown, error\n");
		return -1;
	}

	init_completion(&hba->uic_linkup_done);
	hufs_uic_write_reg(hba, DME_LINKSTARTUPREQ, 0x1);
	if (wait_for_completion_timeout(
		    &hba->uic_linkup_done,
		    msecs_to_jiffies(HUFS_DME_LINKUP_TIMEOUT))) {
		/* Read clear the Linereset caused by link startup */
		(void)ufshcd_readl(hba, REG_UIC_ERROR_CODE_PHY_ADAPTER_LAYER);
		(void)hufs_uic_read_reg(hba, DME_UECPA, &value);
		return ret;
	}

	dev_err(hba->dev, "%s fail\n", __func__);
	hufs_ufs_dme_reg_dump(hba, HUFS_UIC_LINKUP_FAIL);
	ret = -ETIMEDOUT;
	return ret;
}

int ufshcd_wait_for_unipro_register_poll(
	struct ufs_hba *hba, u32 reg, u32 mask, u32 val, int timeout_ms)
{
	int ret = 0;
	u32 reg_val;

	timeout_ms = timeout_ms * 10; /* read reg once 100 us */
	while (timeout_ms-- > 0) {
		hufs_uic_read_reg(hba, reg, &reg_val);
		if ((reg_val & mask) == (val & mask))
			return ret;
		udelay(100); /* wait 100 us */
	}
	ret = -ETIMEDOUT;
	return ret;
}

static void ufshcd_hufs_update_upiu_prdt_offset(
	struct hufs_utp_transfer_req_desc *hufs_utrdlp, struct ufs_hba *hba,
	u16 response_offset, u16 prdt_offset, u16 ucdl_index)
{
#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
	__le16 upiu_size = ALIGNED_UPIU_SIZE * MAX_DATA_USED_SPACE;
	if (ucdl_index == HI1861_VCMD_QUERY_TAG)
		upiu_size = VCMD_RESP_UPIU_SIZE;
#else
	__le16 upiu_size = ALIGNED_UPIU_SIZE;
#endif
	if (hba->quirks & UFSHCD_QUIRK_PRDT_BYTE_GRAN) {
		hufs_utrdlp->response_upiu_offset =
			cpu_to_le16(response_offset);
		hufs_utrdlp->prd_table_offset = cpu_to_le16(prdt_offset);
		hufs_utrdlp->response_upiu_length = cpu_to_le16(upiu_size);
	} else { /* switch from byte to dword */
		hufs_utrdlp->response_upiu_offset =
			cpu_to_le16((response_offset >> 2));
		hufs_utrdlp->prd_table_offset = cpu_to_le16((prdt_offset >> 2));
		hufs_utrdlp->response_upiu_length = cpu_to_le16(upiu_size >> 2);
	}
}

static void
ufshcd_update_upiu_prdt_addr(struct ufshcd_lrb *lrbp,
			     struct utp_transfer_cmd_desc *cmd_descp,
			     u16 ucdl_index)
{
	lrbp->ucd_rsp_ptr =
		(struct utp_upiu_rsp *)cmd_descp[ucdl_index].response_upiu;
	lrbp->ucd_prdt_ptr =
		(struct ufshcd_sg_entry *)cmd_descp[ucdl_index].prd_table;
}

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
static void
ufshcd_update_vcmd_upiu_prdt_addr(struct ufshcd_lrb *lrbp,
				  struct utp_transfer_cmd_desc *cmd_descp,
				  u16 ucdl_index)
{
	struct vcmd_utp_transfer_cmd_desc *vcmd_cmd_descp =
		(struct vcmd_utp_transfer_cmd_desc *)(cmd_descp + ucdl_index);
	lrbp->ucd_rsp_ptr =
		(struct utp_upiu_rsp *)vcmd_cmd_descp->response_upiu;
	lrbp->ucd_prdt_ptr =
		(struct ufshcd_sg_entry *)vcmd_cmd_descp->prd_table;
}
#endif

static void config_lrbp_hufs_utrd(struct ufs_hba *hba)
{
	bool is_hufs_utrd = false;
	int i;

	if (ufshcd_is_hufs_hc(hba))
		is_hufs_utrd = true;
	for (i = 0; i < hba->nutrs; i++)
		hba->lrb[i].is_hufs_utrd = is_hufs_utrd;
}

/**
 * ufshcd_hufs_host_memory_configure - configure local reference block with
 *				memory offsets
 * hba: per adapter instance
 *
 * Configure Host memory space
 * 1. Update Corresponding UTRD.UCDBA and UTRD.UCDBAU with UCD DMA
 * address.
 * 2. Update each UTRD with Response UPIU offset, Response UPIU length
 * and PRDT offset.
 * 3. Save the corresponding addresses of UTRD, UCD.CMD, UCD.RSP and UCD.PRDT
 * into local reference block.
 */
void ufshcd_hufs_host_memory_configure(struct ufs_hba *hba)
{
	struct utp_transfer_cmd_desc *cmd_descp = hba->ucdl_base_addr;
	struct hufs_utp_transfer_req_desc *hufs_utrdlp =
		hba->hufs_utrdl_base_addr;
	dma_addr_t cmd_desc_dma_addr;
	dma_addr_t cmd_desc_element_addr;
	u16 response_offset;
	u16 prdt_offset;
	u16 ucdl_index;
	int slot_begin;
	int i;

	response_offset =
		(u16)offsetof(struct utp_transfer_cmd_desc, response_upiu);
	prdt_offset = (u16)offsetof(struct utp_transfer_cmd_desc, prd_table);

	cmd_desc_dma_addr = hba->ucdl_dma_addr;
#if defined(CONFIG_HISI_UFS_HC_CORE_UTR) && defined(CONFIG_HISI_DEBUG_FS)
	if (hba->nutrs != (SPEC_SLOT_NUM + CORE_SLOT_NUM))
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE, 0ull,
					  0ull);
#endif
	config_lrbp_hufs_utrd(hba);
#ifdef CONFIG_HISI_UFS_HC_CORE_UTR
	slot_begin = SPEC_SLOT_NUM;
#else
	slot_begin = 0;
#endif
	for (i = slot_begin; i < hba->nutrs; i++) {
		/* Configure UTRD with command descriptor base address */
		ucdl_index = i - slot_begin;
		cmd_desc_element_addr = (cmd_desc_dma_addr +
					 (sizeof(struct utp_transfer_cmd_desc) *
					  (unsigned int)ucdl_index));
#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
		if (ucdl_index == HI1861_VCMD_QUERY_TAG) {
			response_offset =
				(u16)offsetof(struct vcmd_utp_transfer_cmd_desc,
					      response_upiu);
			prdt_offset = (u16)offsetof(
				struct vcmd_utp_transfer_cmd_desc, prd_table);
		}
#endif

		hufs_utrdlp[i].command_desc_base_addr_lo =
			cpu_to_le32(lower_32_bits(cmd_desc_element_addr));
		hufs_utrdlp[i].command_desc_base_addr_hi =
			cpu_to_le32(upper_32_bits(cmd_desc_element_addr));

		/* Response upiu and prdt offset should be in double words */
		ufshcd_hufs_update_upiu_prdt_offset(&hufs_utrdlp[i], hba,
						    response_offset,
						    prdt_offset, ucdl_index);

		hba->lrb[i].hufs_utr_descriptor_ptr = (hufs_utrdlp + i);
		hba->lrb[i].utrd_dma_addr =
			hba->utrdl_dma_addr +
			(i * sizeof(struct hufs_utp_transfer_req_desc));
		hba->lrb[i].ucd_req_dma_addr = cmd_desc_element_addr;
		hba->lrb[i].ucd_req_ptr =
			(struct utp_upiu_req *)(cmd_descp + ucdl_index);
		hba->lrb[i].ucd_rsp_dma_addr =
			cmd_desc_element_addr + response_offset;
		hba->lrb[i].ucd_prdt_dma_addr =
			cmd_desc_element_addr + prdt_offset;
#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
		if (ucdl_index == HI1861_VCMD_QUERY_TAG) {
			ufshcd_update_vcmd_upiu_prdt_addr(
				&hba->lrb[i], cmd_descp, ucdl_index);
		} else {
			ufshcd_update_upiu_prdt_addr(&hba->lrb[i], cmd_descp,
						     ucdl_index);
		}
#else
		ufshcd_update_upiu_prdt_addr(&hba->lrb[i], cmd_descp,
					     ucdl_index);
#endif
	}

#ifdef CONFIG_HISI_UFS_HC_CORE_UTR
	hba->nutrs = CORE_SLOT_NUM;
	hba->lrb = hba->lrb + SPEC_SLOT_NUM;
#endif
}

int ufshcd_hufs_uic_change_pwr_mode(struct ufs_hba *hba, u8 mode)
{
	int ret;
	u32 reg_val = 0;
	int retries = HUFS_UIC_ACCESS_REG_RETRIES;

	if (hba->quirks & UFSHCD_QUIRK_BROKEN_PA_RXHSUNTERMCAP) {
		ret = ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_RXHSUNTERMCAP, 0),
				     1);
		if (ret) {
			dev_err(hba->dev,
				"%s: failed to enable PA_RXHSUNTERMCAP ret %d\n",
				__func__, ret);
			return ret;
		}
	}
	init_completion(&hba->uic_pmc_done);

retry:
	ret = hufs_uic_write_reg(hba, PA_PWRMODE, mode);
	if (ret)
		return ret;
	if (wait_for_completion_timeout(
		    &hba->uic_pmc_done,
		    msecs_to_jiffies(HUFS_UIC_ACCESS_REG_TIMEOUT)))
		return ret;

	dev_err(hba->dev, "hufs pmc timetout\n");
	hufs_uic_read_reg(hba, DME_POWERMODEIND, &reg_val);
	if ((reg_val & DME_PWR_BUSY) && retries--)
		goto retry;

	hufs_ufs_dme_reg_dump(hba, HUFS_UIC_PMC_FAIL);
	return -ETIMEDOUT;
}

static void clear_hufs_hisi_h8_intr(struct ufs_hba *hba, bool h8_op)
{
	if (h8_op == HUFS_H8_OP_ENTER) {
		clear_unipro_intr(hba, DME_HIBERN_ENTER_CNF_INTR |
					       DME_HIBERN_ENTER_IND_INTR);
		ufshcd_writel(hba, UIC_HIBERNATE_ENTER, REG_INTERRUPT_STATUS);
	} else {
		clear_unipro_intr(hba, DME_HIBERN_EXIT_CNF_INTR |
					       DME_HIBERN_EXIT_IND_INTR);
		ufshcd_writel(hba, UIC_HIBERNATE_EXIT, REG_INTERRUPT_STATUS);
	}
	/* clear uic error intr raised during h8 */
	ufshcd_writel(hba, UIC_ERROR, REG_INTERRUPT_STATUS);
}

static int ufshcd_hibern8_enter_req(struct ufs_hba *hba)
{
	int ret;
	u32 value = 0;
	int retries = HUFS_UIC_ACCESS_REG_RETRIES;
	const int timeout_ms = HUFS_UIC_ACCESS_REG_TIMEOUT;

retry:
	hufs_uic_read_reg(hba, DME_HIBERNATE_ENTER_STATE, &value);
	hufs_uic_read_reg(hba, DME_HIBERNATE_EXIT_STATE, &value);
	hufs_uic_write_reg(hba, DME_HIBERNATE_ENTER, 0x1);

	ret = ufshcd_wait_for_unipro_register_poll(
		hba, DME_HIBERNATE_ENTER_STATE, DME_HIBERNATE_REQ_RECEIVED,
		DME_HIBERNATE_REQ_RECEIVED, timeout_ms);
	if (ret) {
		hufs_uic_read_reg(hba, DME_HIBERNATE_ENTER_STATE, &value);

		if (value == DME_HIBERNATE_REQ_DENIED && retries--)
			goto retry;

		return ret;
	}

	ret = ufshcd_wait_for_unipro_register_poll(
		hba, DME_HIBERNATE_ENTER_IND, DME_HIBERNATE_ENTER_LOCAL_SUCC,
		DME_HIBERNATE_ENTER_LOCAL_SUCC, timeout_ms);
	if (ret) {
		hufs_uic_read_reg(hba, DME_HIBERNATE_ENTER_IND, &value);
		if (value == DME_HIBERNATE_ENTER_BUSY && retries--)
			goto retry;
		return ret;
	}
	dev_info(hba->dev, "Enter hibernate success\n");
	return ret;
}

static int ufshcd_hibern8_exit_req(struct ufs_hba *hba)
{
	int ret;
	u32 value = 0;
	int retries = HUFS_UIC_ACCESS_REG_RETRIES;
	const int timeout_ms = HUFS_UIC_ACCESS_REG_TIMEOUT;

retry:
	hufs_uic_write_reg(hba, DME_HIBERNATE_EXIT, 0x1);

	ret = ufshcd_wait_for_unipro_register_poll(
		hba, DME_HIBERNATE_EXIT_STATE, DME_HIBERNATE_REQ_RECEIVED,
		DME_HIBERNATE_REQ_RECEIVED, timeout_ms);
	if (ret) {
		hufs_uic_read_reg(hba, DME_HIBERNATE_EXIT_STATE, &value);
		if (value == DME_HIBERNATE_REQ_DENIED && retries--)
			goto retry;

		return ret;
	}

	ret = ufshcd_wait_for_unipro_register_poll(
		hba, DME_HIBERNATE_EXIT_IND, DME_HIBERNATE_EXIT_LOCAL_SUCC,
		DME_HIBERNATE_EXIT_LOCAL_SUCC, timeout_ms);
	if (ret) {
		hufs_uic_read_reg(hba, DME_HIBERNATE_EXIT_IND, &value);
		if (value == DME_HIBERNATE_EXIT_BUSY && retries--)
			goto retry;
		return ret;
	}
	dev_info(hba->dev, "Exit hibernate success\n");
	return ret;
}

static int hufshcd_hibern8_req(struct ufs_hba *hba, bool h8_op)
{
	int ret;

	if (h8_op == HUFS_H8_OP_ENTER)
		ret = ufshcd_hibern8_enter_req(hba);
	else
		ret = ufshcd_hibern8_exit_req(hba);
	return ret;
}

static int check_unipro_state_before_hibern8(struct ufs_hba *hba, bool h8_op)
{
	int ret = 0;
	u32 value = 0;

	hufs_uic_read_reg(hba, DME_UNIPRO_STATE, &value);
	if (h8_op == HUFS_H8_OP_ENTER) {
		if ((value & MASK_DEBUG_UNIPRO_STATE) != LINK_UP) {
			dev_err(hba->dev,
				"unipro state is not up, can't do hibernate enter\n");
			ret = -EPERM;
		}
	} else {
		if ((value & MASK_DEBUG_UNIPRO_STATE) != LINK_HIBERN) {
			dev_err(hba->dev,
				"unipro state is not hibernate, can't do hibernate exit\n");
			ret = -EPERM;
		}
	}
	return ret;
}

#ifdef CONFIG_SCSI_UFS_HS_ERROR_RECOVER
static void ufs_run_hba_recover(struct ufs_hba *hba)
{
	if (!work_busy(&hba->recover_hs_work))
		schedule_work(&hba->recover_hs_work);
	else
		dev_err(hba->dev, "%s:recover_hs_work is running\n", __func__);
}

static void ufs_hba_error_recover(bool h8_op, struct ufs_hba *hba)
{
	u32 value;

	if (h8_op || !hba->check_pwm_after_h8) {
		dev_err(hba->dev, "no need to check\n");
		return;
	}

	if (!hba->vops->get_pwr_by_debug_register) {
		dev_err(hba->dev, "no check pwm op\n");
		hba->check_pwm_after_h8 = 0;
	} else {
		value = hba->vops->get_pwr_by_debug_register(hba);
		if (value == SLOW) {
			dev_err(hba->dev, "ufs pwr = 0x%x after H8\n", value);
			hba->check_pwm_after_h8 = 0;
			ufshcd_init_pwr_info(hba);
			ufs_run_hba_recover(hba);
		} else {
			hba->check_pwm_after_h8--;
		}
		dev_err(hba->dev, "check pwr after H8, %d times remain\n",
			hba->check_pwm_after_h8);
	}
}
#endif

int ufshcd_hibern8_op_irq_safe(struct ufs_hba *hba, bool h8_op)
{
	int ret;
	u32 ie_value;
	u32 dme_intr_value = 0;

	/* disable auto h8 when we use software h8 */
	ret = ufshcd_hufs_disable_auto_hibern8(hba);
	if (ret) {
		dev_err(hba->dev, "disable autoh8 fail before enter h8\n");
		return ret;
	}
	/* step 1: close interrupt and save interrupt value */
	ie_value = ufshcd_readl(hba, REG_INTERRUPT_ENABLE);
	ufshcd_writel(hba, 0, REG_INTERRUPT_ENABLE);

	/* close DME hibernate interrupt */
	hufs_uic_read_reg(hba, DME_INTR_ENABLE, &dme_intr_value);
	hufs_uic_write_reg(hba, DME_INTR_ENABLE, 0);

	ret = check_unipro_state_before_hibern8(hba, h8_op);
	if (ret)
		return ret;

	ret = hufshcd_hibern8_req(hba, h8_op);
	if (ret)
		goto out;

	clear_hufs_hisi_h8_intr(hba, h8_op);

#ifdef CONFIG_SCSI_UFS_HS_ERROR_RECOVER
	ufs_hba_error_recover(h8_op, hba);
#endif

out:
	ufsdbg_error_inject_dispatcher(hba, ERR_INJECT_PWR_MODE_CHANGE_ERR, 0,
				       &ret);

	if (ret) {
		hufs_ufs_dme_reg_dump(hba, h8_op);
		ufshcd_writel(hba, UIC_ERROR, REG_INTERRUPT_STATUS);
		dsm_ufs_update_error_info(hba, DSM_UFS_ENTER_OR_EXIT_H8_ERR);
		schedule_ufs_dsm_work(hba);
	}

	ufshcd_writel(hba, ie_value, REG_INTERRUPT_ENABLE);
	hufs_uic_write_reg(hba, DME_INTR_ENABLE, dme_intr_value);
	return ret;
}

void ufshcd_hufs_enable_auto_hibern8(struct ufs_hba *hba)
{
	u32 value;

	value = ufshcd_readl(hba, UFS_AHIT_CTRL_OFF);
	if (value & UFS_AUTO_HIBERN_EN)
		return;

	value |= UFS_AUTO_HIBERN_EN;
	value &= ~UFS_HIBERNATE_EXIT_MODE;
	ufshcd_writel(hba, value, UFS_AHIT_CTRL_OFF);
}

int ufshcd_hufs_disable_auto_hibern8(struct ufs_hba *hba)
{
	u32 h8_reg;
	int retry = HUFS_AUTO_H8_XFER_TIMEOUT;

	h8_reg = ufshcd_readl(hba, UFS_AHIT_CTRL_OFF);
	if (h8_reg & UFS_AUTO_HIBERN_EN) {
		h8_reg &= (~UFS_AUTO_HIBERN_EN);
		ufshcd_writel(hba, h8_reg, UFS_AHIT_CTRL_OFF);
	} else {
		return 0;
	}

	do {
		h8_reg = ufshcd_readl(hba, UFS_AUTO_H8_STATE_OFF);
		if ((h8_reg & UFS_HC_AH8_STATE) == 0x1)
			return 0;

		mdelay(1);
	} while (retry--);

	dev_err(hba->dev, "disable auto hibern fail\n");
	return -ETIMEDOUT;
}

void hufs_ufs_dme_reg_dump(
	struct ufs_hba *hba, enum hufs_uic_reg_dump_type dump_type)
{
	u32 reg_val = 0;

	hufs_uic_read_reg(hba, DME_UNIPRO_STATE, &reg_val);
	dev_err(hba->dev, "DME_UNIPRO_STATE is 0x%x\n", reg_val);
	if (dump_type == HUFS_UIC_HIBERNATE_ENTER) {
		hufs_uic_read_reg(hba, DME_HIBERNATE_ENTER_STATE, &reg_val);
		dev_err(hba->dev, "DME_HIBERNATEENTER_STATE is 0x%x\n",
			reg_val);
		hufs_uic_read_reg(hba, DME_HIBERNATE_ENTER_IND, &reg_val);
		dev_err(hba->dev, "DME_HIBERNATE_ENTER_IND is 0x%x\n", reg_val);
	} else if (dump_type == HUFS_UIC_HIBERNATE_EXIT) {
		hufs_uic_read_reg(hba, DME_HIBERNATE_EXIT_STATE, &reg_val);
		dev_err(hba->dev, "DME_HIBERNATEEXIT_STATE is 0x%x\n", reg_val);
		hufs_uic_read_reg(hba, DME_HIBERNATE_EXIT_IND, &reg_val);
		dev_err(hba->dev, "DME_HIBERNATE_EXIT_IND is 0x%x\n", reg_val);
	} else if (dump_type == HUFS_UIC_LINKUP_FAIL) {
		hufs_uic_read_reg(hba, PA_FSM_STATUS, &reg_val);
		dev_err(hba->dev, "PA_FSM_STATUS is 0x%x\n", reg_val);
		hufs_uic_read_reg(hba, PA_STATUS, &reg_val);
		dev_err(hba->dev, "PA_STATUS is 0x%x\n", reg_val);
	} else if (dump_type == HUFS_UIC_PMC_FAIL) {
		hufs_uic_read_reg(hba, DME_POWERMODEIND, &reg_val);
		dev_err(hba->dev, "hufs DME_POWERMODEIND is 0x%x\n",
			reg_val);
	}
}

void clear_unipro_intr(struct ufs_hba *hba, int dme_intr_clr)
{
	unsigned int value = 0;

	hufs_uic_read_reg(hba, DME_INTR_CLR, &value);
	value |= (unsigned int)dme_intr_clr;
	hufs_uic_write_reg(hba, DME_INTR_CLR, value);
}

void ufshcd_enable_vs_intr(struct ufs_hba *hba, u32 intrs)
{
	u32 set;

	set = ufshcd_readl(hba, UFS_VS_IE);

	set |= intrs;
	ufshcd_writel(hba, set, UFS_VS_IE);
}

void ufshcd_disable_vs_intr(struct ufs_hba *hba, u32 intrs)
{
	u32 set;

	set = ufshcd_readl(hba, UFS_VS_IE);

	set &= ~intrs;
	ufshcd_writel(hba, set, UFS_VS_IE);
}

static void ufshcd_enable_autodiv(struct ufs_hba *hba)
{
	u32 value;
	struct hufs_host *host = (struct hufs_host *)hba->priv;

	writel(UFS_AUTODIV_VAL, SOC_SCTRL_SCUFS_AUTODIV_ADDR(host->sysctrl));

	value = readl(SOC_SCTRL_SCUFS_AUTODIV_ADDR(host->sysctrl));
	value &= ~UFS_AUTODIV_LPM3_BYPASS;
	writel(value, SOC_SCTRL_SCUFS_AUTODIV_ADDR(host->sysctrl));

	value = readl(SOC_SCTRL_SCPERCLKEN4_ADDR(host->sysctrl));
	value |= BIT(SOC_SCTRL_SCPERCLKEN4_gt_clk_autodiv_ufs_subsys_START);
	writel(value, SOC_SCTRL_SCPERCLKEN4_ADDR(host->sysctrl));
}

/* enable clock gating and autodiv when ufshcd is idle */
void ufshcd_enable_clock_gating_autodiv(struct ufs_hba *hba)
{
	ufshcd_idle_auto_gating(hba);
	ufshcd_enable_autodiv(hba);
}

static void check_pa_granularity(struct ufs_hba *hba)
{
	u32 value = 0;

	if (strstarts(hba->model, UFS_MODEL_SS6100)) {
		hufs_uic_write_reg(hba, PA_TACTIVATE, PA_TACTIVATE_VAL);
		hufs_uic_write_reg(hba, PA_GRANULARITY, PA_GRANULARITY_VAL);
		pr_err("1861: Override PA_TACTIVATE/PA_GRANULARITY\n");
	} else {
		/* as previous procedure */
		hufs_uic_read_reg(hba, PA_GRANULARITY, &value);
		if (value < PA_GRANULARITY_VAL)
			hufs_uic_write_reg(hba, PA_GRANULARITY,
						PA_GRANULARITY_VAL);
	}
}

static void v300_set_deemphasis(struct ufs_hba *hba)
{
	u32 r_value = 0;
	u32 deemph_val;
	struct hufs_host *host = hba->priv;

	hufs_uic_read_reg(hba, ATTR_MTX0(TX_DMY1), &r_value);
	if (r_value > MAX_TX_R)
		r_value = MAX_TX_R;

	if (host->tx_equalizer == TX_EQUALIZER_35DB)
		deemph_val = g_deemp_n3_5_db[r_value];
	else if (host->tx_equalizer == TX_EQUALIZER_60DB)
		deemph_val = g_deemp_n6_db[r_value];
	else
		deemph_val = 0;

	pr_err("equalizer = [0x%x], r = [0x%x], deemph_val = [0x%x]\n",
			host->tx_equalizer, r_value, deemph_val);
	hufs_uic_write_reg(hba, ATTR_MTX0(TX_AFE_CONFIG_VI), deemph_val);
	hufs_uic_write_reg(hba, ATTR_MTX1(TX_AFE_CONFIG_VI), deemph_val);
}

static void mphy_cdr_bw_config(struct ufs_hba *hba, u32 gear)
{
	u32 cdr_bw;
	struct hufs_host *host = hba->priv;

	if (!(host->caps & USE_MPHY_CDR_BW_WORKROUND))
		return;
	if (gear < UFS_HS_G1 || gear > UFS_HS_G4)
		return;

	/* index 0~3 for gear 1~4 */
	cdr_bw = hufs_mphy_v300_pre_pmc_cdr_bw[gear - 1];
	/*
	 * for samsung devices, cdr bw value: 0x2(gear 1 and gear 2)
	 * 0x2 (gear3 and gear 4)
	 */
	if (hba->manufacturer_id == UFS_VENDOR_SAMSUNG)
		cdr_bw = SAMSUNG_CDR_BW;

	hufs_uic_write_reg(hba, ATTR_MRX0(RX_AFE_CTRL_III), cdr_bw);
	hufs_uic_write_reg(hba, ATTR_MRX1(RX_AFE_CTRL_III), cdr_bw);
}

static void
hufs_asic_mphy_pre_pmc_attr_config(struct ufs_hba *hba,
				   struct ufs_pa_layer_attr *dev_req_params)
{
	u32 value = 0;
	struct ufs_attr_cfg *cfg = NULL;
	struct hufs_host *host = hba->priv;
	u8 deemp_20t4_en = 0;

	if (dev_req_params->pwr_rx == FAST_MODE ||
	    dev_req_params->pwr_rx == FASTAUTO_MODE) {
		if (host->tx_equalizer != TX_EQUALIZER_0DB)
			deemp_20t4_en = 1;
		switch (dev_req_params->gear_rx) {
		case UFS_HS_G4:
			cfg = (struct ufs_attr_cfg *)
				hufs_mphy_v300_pre_pmc_fsg4_attr;
			break;
		case UFS_HS_G3:
			cfg = (struct ufs_attr_cfg *)
				hufs_mphy_v300_pre_pmc_fsg3_attr;
			break;
		case UFS_HS_G2:
			cfg = (struct ufs_attr_cfg *)
				hufs_mphy_v300_pre_pmc_fsg2_attr;
			break;
		case UFS_HS_G1:
			cfg = (struct ufs_attr_cfg *)
				hufs_mphy_v300_pre_pmc_fsg1_attr;
			break;
		default:
			dev_err(hba->dev, "unknown ufs gear\n");
			cfg = 0;
			break;
		}
	} else {
		cfg = (struct ufs_attr_cfg *)hufs_mphy_v300_pre_pmc_slow_attr;
	}

	hufs_set_each_cfg_attr(hba, cfg);

	mphy_cdr_bw_config(hba, dev_req_params->gear_rx);
	/* 0x00000050 is vendor private register */
	hufs_uic_read_reg(hba, 0x00000050, &value);
	if (deemp_20t4_en) {
		v300_set_deemphasis(hba);
		value |= MPHY_DEEMPH_20T4_EN;
	} else {
		value &= (~MPHY_DEEMPH_20T4_EN);
	}

	hufs_txpost_enable(dev_req_params, &value);
	hufs_uic_write_reg(hba, 0x00000050, value);
	hufs_uic_write_reg(hba, 0x00010050, value);
}

static void hufs_mphy_tc_pre_pmc_attr_config(
	struct ufs_hba *hba, struct ufs_pa_layer_attr *dev_req_params)
{
	struct ufs_attr_cfg *cfg = NULL;

	if (!is_v200_mphy(hba))
		return;
	if (dev_req_params->pwr_rx == FAST_MODE ||
	    dev_req_params->pwr_rx == FASTAUTO_MODE) {
		switch (dev_req_params->gear_rx) {
		case UFS_HS_G4:
			cfg = (struct ufs_attr_cfg *)
				hufs_mphy_v200_tc_pre_pmc_fsg4_attr;
			break;
		case UFS_HS_G3:
			cfg = (struct ufs_attr_cfg *)
				hufs_mphy_v200_tc_pre_pmc_fsg3_attr;
			break;
		case UFS_HS_G2:
			cfg = (struct ufs_attr_cfg *)
				hufs_mphy_v200_tc_pre_pmc_fsg2_attr;
			break;
		case UFS_HS_G1:
			cfg = (struct ufs_attr_cfg *)
				hufs_mphy_v200_tc_pre_pmc_fsg1_attr;
			break;
		default:
			dev_err(hba->dev, "unknown ufs gear\n");
			cfg = 0;
			break;
		}
	} else {
		cfg = (struct ufs_attr_cfg *)hufs_mphy_v200_tc_pre_pmc_slow_attr;
	}
	hufs_set_each_cfg_attr(hba, cfg);
}

void ufs_pwr_change_pre_change(
	struct ufs_hba *hba, struct ufs_pa_layer_attr *dev_req_params)
{
	struct hufs_host *host = NULL;

	host = hba->priv;

	pr_info("%s ++\n", __func__);
#ifdef CONFIG_HISI_DEBUG_FS
	pr_info("device manufacturer_id is 0x%x\n", hba->manufacturer_id);
#endif
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_TXSKIP), 0x0); /* PA_TxSkip */

	/* PA_PWRModeUserData0 = 8191, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA0), 8191);
	/* PA_PWRModeUserData1 = 65535, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA1), 65535);
	/* PA_PWRModeUserData2 = 32767, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA2), 32767);
	/* PA_PWRModeUserData3 = 8191, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA3), 8191);
	/* PA_PWRModeUserData4 = 65535, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA4), 65535);
	/* PA_PWRModeUserData5 = 32767, default is 0 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_PWRMODEUSERDATA5), 32767);

	check_pa_granularity(hba);

	/* for hufs asic mphy and emu, use USE_HUFS_MPHY_ASIC on ASIC later */
	if (!(host->caps & USE_HUFS_MPHY_TC) || hba->host->is_emulator)
		hufs_asic_mphy_pre_pmc_attr_config(hba, dev_req_params);

	if ((host->caps & USE_HUFS_MPHY_TC))
		hufs_mphy_tc_pre_pmc_attr_config(hba, dev_req_params);
	pr_info("%s --\n", __func__);
}

void ufshcd_hufs_enable_unipro_intr(struct ufs_hba *hba, u32 unipro_intrs)
{
	u32 set = 0;

	hufs_uic_read_reg(hba, DME_INTR_ENABLE, &set);
	set |= unipro_intrs;
	hufs_uic_write_reg(hba, DME_INTR_ENABLE, set);
}

void ufshcd_hufs_disable_unipro_intr(struct ufs_hba *hba, u32 unipro_intrs)
{
	u32 set = 0;

	hufs_uic_read_reg(hba, DME_INTR_ENABLE, &set);
	set &= ~unipro_intrs;
	hufs_uic_write_reg(hba, DME_INTR_ENABLE, set);
}

static void ufshcd_sl_unipro_intr(struct ufs_hba *__hba, u32 unipro_intr_status)
{
	u32 en_unipro_intr_status;
	u32 result;
	u32 reg_offset;
	u32 reg_value = 0;
	struct ufs_hba *hba = __hba;

	ufsdbg_error_inject_dispatcher(hba, ERR_INJECT_UNIPRO_INTR,
		unipro_intr_status, &unipro_intr_status);

	hufs_uic_read_reg(hba, DME_INTR_ENABLE, &reg_value);
	en_unipro_intr_status = unipro_intr_status & reg_value;

	if (en_unipro_intr_status & LINKUP_CNF_INTR) {
		hufs_uic_read_reg(hba, DME_LINKSTARTUP_STATE, &reg_value);
		reg_value &= LINK_STARTUP_CNF;
		if (reg_value == LINK_STARTUP_SUCC)
			complete(&hba->uic_linkup_done);
	}
	if (en_unipro_intr_status & PMC_IND_INTR) {
		hufs_uic_read_reg(hba, DME_POWERMODEIND, &reg_value);
		if (reg_value & DME_POWER_MODE_LOCAL_SUCC)
			complete(&hba->uic_pmc_done);
	}
	if (en_unipro_intr_status & LOCAL_ATTR_FAIL_INTR) {
		hufs_uic_read_reg(hba, DME_LOCAL_OPC_DBG, &reg_value);
		reg_offset = reg_value & ATTR_LOCAL_ERR_ADDR;
		result = reg_value & ATTR_LOCAL_ERR_RES;
		if (result && result != INVALID_MIB_ATTRIBUTE)
			dev_err(hba->dev,
				"local attr access fail, local_opc_dbg is 0x%x, reg_offset is 0x%x\n",
				result, reg_offset);
		else if (result == INVALID_MIB_ATTRIBUTE &&
			 hba->invalid_attr_print)
			dev_err(hba->dev, "invalid mib attr 0x%x\n",
				reg_offset);
	}
	if (en_unipro_intr_status & PEER_ATTR_COMPL_INTR) {
		hufs_uic_read_reg(hba, DME_PEER_OPC_STATE, &reg_value);
		if (reg_value & PEER_ATTR_RES)
			dev_err(hba->dev, "peer attr access fail 0x%x\n",
				reg_value);
		else
			complete(&hba->uic_peer_get_done);
	}
	if (en_unipro_intr_status & HSH8ENT_LR_INTR) {
		if (hba->ufshcd_state == UFSHCD_STATE_OPERATIONAL) {
			dev_err(hba->dev, "%s, unipro IS 0x%x\n", __func__,
				en_unipro_intr_status);
			__ufshcd_disable_pwm_cnt(hba);

			/* block commands from scsi mid-layer */
			scsi_block_requests(hba->host);

			hba->force_host_reset = true;
			hba->ufshcd_state = UFSHCD_STATE_EH_SCHEDULED;
			kthread_queue_work(&hba->eh_worker, &hba->eh_work);
		}
	}
	if (en_unipro_intr_status & DEBUG_COUNTER_OVER_FLOW_INTR)
		ufshcd_dme_set(hba, UIC_ARG_MIB((u32)DEBUGCOUNTER_CLR),
			       BIT_DBG_CNT0_CLR | BIT_DBG_CNT1_CLR);
}

irqreturn_t _ufshcd_hufs_unipro_intr(int unipro_irq, void *__hba)
{
	u32 intr_stat = 0;
	u32 en_intr_stat;
	u32 reg_value = 0;
	irqreturn_t retval;
	struct ufs_hba *hba = __hba;

	spin_lock(hba->host->host_lock);

	hufs_uic_read_reg(hba, DME_INTR_STATUS, &intr_stat);

	hufs_uic_read_reg(hba, DME_INTR_ENABLE, &reg_value);
	en_intr_stat = intr_stat & reg_value;

	hba->last_unipro_intr = intr_stat;
	hba->last_unipro_intr_time_stamp = ktime_get();
	if (intr_stat)
		clear_unipro_intr(hba, intr_stat);
	if (en_intr_stat)
		ufshcd_sl_unipro_intr(hba, en_intr_stat);

	retval = IRQ_HANDLED;

	spin_unlock(hba->host->host_lock);
	return retval;
}

#ifdef CONFIG_SCSI_UFS_INTR_HUB
irqreturn_t ufshcd_ufs_intr_hub_intr(int ufs_intr_hub_irq, void *__hba)
{
	u32 intr_stat;
	irqreturn_t retval = IRQ_NONE;
	struct ufs_hba *hba = (struct ufs_hba *)__hba;
	struct hufs_host *host = (struct hufs_host *)hba->priv;

	intr_stat = readl(host->hsdt0_intr_hub +
			  INTR_HUB_LEVEL2_INTR_STATUS_NS_N(INTR_HUB_UFS_GROUP));
	if (intr_stat & BIT(INTR_UNIPRO_GIC_BIT))
		retval = _ufshcd_hufs_unipro_intr(ufs_intr_hub_irq, hba);

	if (intr_stat & BIT(INTR_UFSHC_GIC_BIT))
		retval = ufshcd_intr(ufs_intr_hub_irq, hba);
	return retval;
}
#else
irqreturn_t ufshcd_hufs_unipro_intr(int unipro_irq, void *__hba)
{
	return _ufshcd_hufs_unipro_intr(unipro_irq, __hba);
}
#endif

void ufshcd_sl_fatal_intr(struct ufs_hba *hba, u32 intr_status)
{
	ufsdbg_error_inject_dispatcher(hba, ERR_INJECT_INTR, intr_status,
				       &intr_status);
	hba->errors = INT_FATAL_ERRORS & intr_status;

#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
	if (ufshcd_support_inline_encrypt(hba))
		hba->errors |= CRYPTO_ENGINE_FATAL_ERROR & intr_status;
#endif

	if (hba->errors)
		ufshcd_check_errors(hba);
}

irqreturn_t ufshcd_hufs_fatal_err_intr(int fatal_err_irq, void *__hba)
{
	u32 fatal_intr_status, enabled_fatal_intr_status;
	irqreturn_t retval;
	struct ufs_hba *hba = __hba;

	spin_lock(hba->host->host_lock);

	fatal_intr_status = ufshcd_readl(hba, REG_INTERRUPT_STATUS);
	ufshcd_hufs_is_intr_inject(hba, &fatal_intr_status);
	dev_err(hba->dev, "fatal_err intr status 0x%x\n", fatal_intr_status);

	hba->last_fatal_intr = fatal_intr_status;
	hba->last_fatal_intr_time_stamp = ktime_get();
	enabled_fatal_intr_status =
		fatal_intr_status & ufshcd_readl(hba, REG_INTERRUPT_ENABLE);
	if (fatal_intr_status)
		ufshcd_writel(hba, fatal_intr_status, REG_INTERRUPT_STATUS);
	if (enabled_fatal_intr_status & INT_FATAL_ERRORS)
		ufshcd_sl_fatal_intr(hba, enabled_fatal_intr_status);

	retval = IRQ_HANDLED;
	spin_unlock(hba->host->host_lock);
	return retval;
}

/* inject ufs IS interrupt */
void ufshcd_hufs_is_intr_inject(struct ufs_hba *hba, u32 *intr_status)
{
#ifdef CONFIG_SCSI_UFS_FAULT_INJECT
	u32 is_intr_set;

	is_intr_set = ufshcd_readl(hba, UFS_IS_INT_SET);
	if (is_intr_set && intr_status) {
		*intr_status |= is_intr_set;
		ufshcd_writel(hba, 0, UFS_IS_INT_SET);
		dev_err(hba->dev, "%s: intr_status:0x%x, intr_set:0x%x\n",
			__func__, *intr_status, is_intr_set);
	}
#endif
}

/* inject ufs VS interrupt */
static u32 hufshcd_vs_intr_inject(struct ufs_hba *hba, u32 vs_intr_status)
{
#ifdef CONFIG_SCSI_UFS_FAULT_INJECT
	u32 vs_intr_set;

	vs_intr_set = ufshcd_readl(hba, UFS_VS_IS_SET);
	if (vs_intr_set) {
		vs_intr_status |= vs_intr_set;
		ufshcd_writel(hba, 0, UFS_VS_IS_SET);
		dev_err(hba->dev, "%s: intr_status:0x%x, intr_set:0x%x\n",
			__func__, vs_intr_status, vs_intr_set);
	}
#endif
	return vs_intr_status;
}

static void ufshcd_timeout_handler(struct ufs_hba *hba)
{
	struct ufshcd_lrb *lrbp = NULL;
	int tag = 0;

	if (hba->ufshcd_state != UFSHCD_STATE_OPERATIONAL)
		return;

	__ufshcd_disable_dev_tmt_cnt(hba);

	/* block commands from scsi mid-layer */
	scsi_block_requests(hba->host);

	hba->force_host_reset = true;
	hba->ufshcd_state = UFSHCD_STATE_EH_SCHEDULED;
	for_each_set_bit (tag, &hba->outstanding_reqs, hba->nutrs) {
		lrbp = &hba->lrb[tag];
		dev_err(hba->dev,
			"UPIU[%d] - tag %d - opcode 0x%x - issue time %lld - complete time %lld\n",
			tag, lrbp->req_tag, lrbp->opcode,
			ktime_to_us(lrbp->issue_time_stamp),
			ktime_to_us(lrbp->complete_time_stamp));
	}
	dsm_ufs_update_upiu_info(hba, -1, DSM_UFS_TIMEOUT_ERR);
	schedule_ufs_dsm_work(hba);
	kthread_queue_work(&hba->eh_worker, &hba->eh_work);
}

static void ufshcd_sl_vs_intr(struct ufs_hba *hba, u32 vs_intr_status)
{
	u32 vs_errors;

	ufsdbg_error_inject_dispatcher(
		hba, ERR_INJECT_VS_INTR, vs_intr_status, &vs_intr_status);
	if (!vs_intr_status)
		return;

	if (ufshcd_is_auto_hibern8_allowed(hba)) {
		vs_errors = (vs_intr_status & (AH8_EXIT_REQ_CNF_FAIL_INTR |
					       AH8_ENTER_REQ_CNF_FAIL_INTR));
		/*
		 * If enter auto H8 fail, it not affects normal function,
		 * just print. Exit fail, do error hanle.
		 */
		if (vs_errors & AH8_ENTER_REQ_CNF_FAIL_INTR)
			dev_err(hba->dev, "unipro reject ah8 enter req\n");
		if (vs_errors) {
			hba->errors |= vs_errors;
			ufshcd_check_errors(hba);
			hba->errors &= ~vs_errors;
		}
	}

	/* Device Timeout handler */
	if ((vs_intr_status & UFS_RX_CPORT_TIMEOUT_INTR) ||
	    (vs_intr_status & VS_IS_IO_TIMEOUT)) {
		hba->io_timeout_cnt ++;
		hba->errors |= UFS_RX_CPORT_TIMEOUT_INTR;
		dev_err(hba->dev, "%s, UFS_VS_IS 0x%x\n", __func__,
			vs_intr_status);
		hufs_key_reg_dump(hba);
		ufshcd_timeout_handler(hba);
	}

	/* Idle Timeout handler */
	if (likely(hba->ufs_idle_intr_en)) {
		if (!hba->idle_intr_disabled) {
			if (vs_intr_status & IDLE_PREJUDGE_INTR)
				ufshcd_idle_handler(hba);
		}
	}
}

/* ufs VS interrupt handle */
irqreturn_t ufshcd_hufs_vs_intr(struct ufs_hba *hba)
{
	u32 vs_intr_status, enabled_vs_intr_status;
	irqreturn_t retval = IRQ_NONE;

	vs_intr_status = ufshcd_readl(hba, UFS_VS_IS);
	vs_intr_status = hufshcd_vs_intr_inject(hba, vs_intr_status);
	enabled_vs_intr_status = vs_intr_status & ufshcd_readl(hba, UFS_VS_IE);

	hba->last_vs_intr = vs_intr_status;
	hba->last_vs_intr_time_stamp = ktime_get();
	if (vs_intr_status)
		ufshcd_writel(hba, vs_intr_status, UFS_VS_IS);

	if (enabled_vs_intr_status) {
		ufshcd_sl_vs_intr(hba, enabled_vs_intr_status);
		retval = IRQ_HANDLED;
	}

	return retval;
}

int ufs_access_register_and_check(struct ufs_hba *hba, u32 access_mode)
{
	int ret;

	ufshcd_writel(hba, access_mode, UFS_CFG_RAM_CTRL);

	ret = ufshcd_wait_for_register_poll(hba, UFS_CFG_RAM_STATUS,
					    access_mode,
					    COMPLETION_TIMEOUT_RETRY_TIMES,
					    COMPLETION_TIMEOUT_RETRY_UDELAY);
	if (ret)
		dev_err(hba->dev, "UFS registers access is not finished\n");

	return ret;
}

int wait_mphy_init_done(struct ufs_hba *hba)
{
	int ret;
	const u32 timeout_ms = MPHY_INIT_TIMEOUT;

	if (unlikely(hba->host->is_emulator))
		return 0;

	ret = ufshcd_wait_for_unipro_register_poll(
		hba, MPHY_INIT, MASK_MPHY_INIT, MPHY_INIT_DONE, timeout_ms);
	if (ret < 0)
		dev_err(hba->dev, "Wait_mphy_init_done failed %d\n", ret);

	return ret;
}

void ufshcd_hufs_enable_dev_tmt_intr(struct ufs_hba *hba)
{
	ufshcd_enable_vs_intr(hba, UFS_RX_CPORT_TIMEOUT_INTR);
}

void ufshcd_hufs_enable_pwm_intr(struct ufs_hba *hba)
{
	ufshcd_hufs_enable_unipro_intr(hba, HSH8ENT_LR_INTR);
}

void ufshcd_hufs_enable_idle_tmt_cnt(struct ufs_hba *hba)
{
	u32 reg_val;

	reg_val = ufshcd_readl(hba, UFS_CFG_IDLE_ENABLE);
	reg_val |= IDLE_PREJUDGE_TIMTER_EN;
	ufshcd_writel(hba, reg_val, UFS_CFG_IDLE_ENABLE);
}

void ufshcd_hufs_disable_idle_tmt_cnt(struct ufs_hba *hba)
{
	u32 reg_val;

	reg_val = ufshcd_readl(hba, UFS_CFG_IDLE_ENABLE);
	reg_val &= ~IDLE_PREJUDGE_TIMTER_EN;
	ufshcd_writel(hba, reg_val, UFS_CFG_IDLE_ENABLE);
}

/*
 * when auto k is enable, if pmc from slowauto gear 1 lane 1 to fast gear 4
 * lane 2, we need to pmc to slowauto gear 1 lane 2 first to avoid bug
 */
int auto_k_enable_pmc_check(
	struct ufs_hba *hba, struct ufs_pa_layer_attr final_params)
{
	u32 value = 0;
	struct ufs_pa_layer_attr temp_params;
	struct hufs_host *host = hba->priv;

	if (!(host->caps & USE_MPHY_AUTO_K_WORKROUND))
		return 0;
	/* RX_K_OFFSET_CTRL XIII */
	hufs_uic_read_reg(hba, 0x00020042, &value);
	/* BIT 0: Auto_k_OS_EN */
	if (!(value & BIT(0)))
		return 0;
	if (hba->pwr_info.pwr_rx != SLOWAUTO_MODE ||
	    hba->pwr_info.lane_rx != 1 || hba->pwr_info.gear_rx != 1)
		return 0;
	if (final_params.pwr_rx != FAST_MODE || final_params.gear_rx != 4 ||
	    final_params.lane_rx != 2) /* lane_rx2 or gear_rx4 */
		return 0;
	temp_params.pwr_rx = SLOWAUTO_MODE;
	temp_params.pwr_tx = SLOWAUTO_MODE;
	temp_params.gear_rx = 1;
	temp_params.gear_tx = 1;
	temp_params.lane_rx = 2; /* lane_rx2 */
	temp_params.lane_tx = 2; /* lane_tx2 */
	temp_params.hs_rate = final_params.hs_rate;

	return ufshcd_change_power_mode(hba, &temp_params);
}

#ifdef CONFIG_HISI_UFS_HC_CORE_UTR
/* given a qos lvl, get its max outstanding */
static unsigned int qos_outstd_map[NUM_OF_QOS_LVL] = {
	QOS_LVL0_DFT_OUTSTD,
	QOS_LVL1_DFT_OUTSTD,
	QOS_LVL2_DFT_OUTSTD,
	QOS_LVL3_DFT_OUTSTD,
	QOS_LVL4_DFT_OUTSTD,
	QOS_LVL5_DFT_OUTSTD,
	QOS_LVL6_DFT_OUTSTD,
	QOS_LVL7_DFT_OUTSTD,
};

static unsigned int qos_prmt_outstd_map[NUM_OF_PROMOTE_QOS_LVL] = {
	QOS_LVL0_PRMT_DFT_OUTSTD,
	QOS_LVL1_PRMT_DFT_OUTSTD,
	QOS_LVL2_PRMT_DFT_OUTSTD,
	QOS_LVL3_PRMT_DFT_OUTSTD,
	QOS_LVL4_PRMT_DFT_OUTSTD,
	QOS_LVL5_PRMT_DFT_OUTSTD,
	QOS_LVL6_PRMT_DFT_OUTSTD,
};

static unsigned int qos_incrs_outstd_map[NUM_OF_PROMOTE_QOS_LVL] = {
	QOS_LVL0_INCRS_DFT_OUTSTD,
	QOS_LVL1_INCRS_DFT_OUTSTD,
	QOS_LVL2_INCRS_DFT_OUTSTD,
	QOS_LVL3_INCRS_DFT_OUTSTD,
	QOS_LVL4_INCRS_DFT_OUTSTD,
	QOS_LVL5_INCRS_DFT_OUTSTD,
	QOS_LVL6_INCRS_DFT_OUTSTD,
};

int get_tag_per_cpu(struct ufs_hba *hba, unsigned int cpu)
{
	u64 tmp = ~(0xFFUL << (SLOT_NUM_EACH_CORE * cpu)) | hba->lrb_in_use;
	if (unlikely(tmp == ~0UL))
		return -1;

	return ffz(tmp);
}

void ufshcd_enable_core_run_stop_reg(struct ufs_hba *hba)
{
	uint8_t core_num;

	for (core_num = 0; core_num < UTR_CORE_NUM; core_num++)
		ufshcd_writel(hba, CORE_UTP_TRANSFER_REQ_LIST_RUN_STOP_BIT,
			      UFS_CORE_UTRLRSR(core_num));
}

void ufshcd_disable_core_run_stop_reg(struct ufs_hba *hba)
{
	uint8_t core_num;

	for (core_num = 0; core_num < UTR_CORE_NUM; core_num++)
		ufshcd_writel(hba, 0, UFS_CORE_UTRLRSR(core_num));
}

/*
 * timeout in ms, range from 100ms to 1000ms,
 * must be multiples of 100ms.
 */
void set_core_utr_qos_starve_timeout(struct ufs_hba *hba, unsigned int timeout)
{
	uint32_t reg_val;

	/* timeout must be multiples of 100s and can't be larger than 1000ms */
	if (timeout < 100 || timeout > 1000 || timeout % 100) {
		dev_err(hba->dev, "Invalid QoS timeout: %ums, default to %dms\n",
				timeout, DEFAULT_CORE_TR_TIMEOUT);
		timeout = DEFAULT_CORE_TR_TIMEOUT;
	}

	reg_val = ufshcd_readl(hba, UFS_MUTILQ_ARB_TO_THSH);
	/* only lower 7bits count */
	reg_val &= ~0x7F;

	/*
	 * timeout is multiple of 100ms,
	 * and need to mius 1 according to reg excel.
	 */
	reg_val |= (timeout / 100) - 1;

	/* 0x4 mean times 100ms, timer scale bits 06:04 */
	reg_val |= (0x4 << 4);
	ufshcd_writel(hba, reg_val, UFS_MUTILQ_ARB_TO_THSH);
}

u64 read_core_utr_doorbells(struct ufs_hba *hba)
{
	unsigned int i;
	u64 doorbell = 0;
	u32 a_doorbell;

	for (i = 0; i < UTR_CORE_NUM; i++) {
		a_doorbell = ufshcd_readl(hba, UFS_CORE_UTRLDBR(i));
		a_doorbell &= 0xFF;
		doorbell |= (u64)a_doorbell << (SLOT_NUM_EACH_CORE * i);
	}

	return doorbell;
}

void core_utr_qos_ctrl_init(struct ufs_hba *hba)
{
	unsigned int i;

	hba->qos_ctrl.hufs_qos_en = true;
	for (i = 0; i < NUM_OF_QOS_LVL; i++)
		hba->qos_ctrl.core_qos_outstd_num[i] = qos_outstd_map[i];
	for (i = 0; i < NUM_OF_PROMOTE_QOS_LVL; i++)
		hba->qos_ctrl.core_qos_prmt_outstd_num[i] = qos_prmt_outstd_map[i];
	for (i = 0; i < NUM_OF_PROMOTE_QOS_LVL; i++)
		hba->qos_ctrl.core_qos_incrs_outstd_num[i] = qos_incrs_outstd_map[i];

	hba->qos_ctrl.hufs_qos_inited = true;
}

void set_core_utr_slot_qos(struct ufs_hba *hba,
		unsigned int task_tag, unsigned int qos)
{
	unsigned int cpu;
	unsigned int core_slot;
	unsigned int reg_val;

#ifdef CONFIG_HISI_DEBUG_FS
	if (qos > QOS_MAX_LVL)
		rdr_syserr_process_for_ap(
				(u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
#endif
	cpu = task_tag / SLOT_NUM_EACH_CORE;
	core_slot = task_tag % SLOT_NUM_EACH_CORE;

	reg_val = ufshcd_readl(hba, UFS_CORE_DOORBELL_QOS(cpu));
	reg_val &= ~(0xFU << (BITS_EACH_SLOT_QOS * core_slot));
	reg_val |= (qos << (BITS_EACH_SLOT_QOS * core_slot));
	ufshcd_writel(hba, reg_val, UFS_CORE_DOORBELL_QOS(cpu));
}

static void ufs_set_spec_dbl_qos(struct ufs_hba *hba)
{
	unsigned int reg_val;
	unsigned int slot, index;
	unsigned int spec_doorbell_qos[SPEC_SLOT_NUM] = {0};

	reg_val = 0;
	for (slot = 0; slot < SLOT_NUM_EACH_QOS_REG; slot++)
		reg_val |= (spec_doorbell_qos[slot] << (BITS_EACH_SLOT_QOS * slot));
	ufshcd_writel(hba, reg_val, UFS_DOORBELL_0_7_QOS);

	reg_val = 0;
	for (slot = QOS_SLOT_8; slot < QOS_SLOT_8 + SLOT_NUM_EACH_QOS_REG;
	     slot++) {
		index = slot - QOS_SLOT_8;
		reg_val |= (spec_doorbell_qos[slot] << (BITS_EACH_SLOT_QOS * index));
	}
	ufshcd_writel(hba, reg_val, UFS_DOORBELL_8_15_QOS);

	reg_val = 0;
	for (slot = QOS_SLOT_16; slot < QOS_SLOT_16 + SLOT_NUM_EACH_QOS_REG;
	     slot++) {
		index = slot - QOS_SLOT_16;
		reg_val |= (spec_doorbell_qos[slot] << (BITS_EACH_SLOT_QOS * index));
	}
	ufshcd_writel(hba, reg_val, UFS_DOORBELL_16_23_QOS);

	reg_val = 0;
	for (slot = QOS_SLOT_24; slot < QOS_SLOT_24 + SLOT_NUM_EACH_QOS_REG;
	     slot++) {
		index = slot - QOS_SLOT_24;
		reg_val |= (spec_doorbell_qos[slot] << (BITS_EACH_SLOT_QOS * index));
	}
	ufshcd_writel(hba, reg_val, UFS_DOORBELL_24_31_QOS);
}

static void ufs_enable_doorbell_qos(struct ufs_hba *hba)
{
	unsigned int reg_val;
	unsigned int slot, core;
	unsigned int core_doorbell_qos[CORE_SLOT_NUM] = {0};

	/* Enable Qos */
	reg_val = ufshcd_readl(hba, UFS_PROC_MODE_CFG);
	reg_val |= MASK_CFG_UTR_QOS_EN;
	ufshcd_writel(hba, reg_val, UFS_PROC_MODE_CFG);

	ufs_set_spec_dbl_qos(hba);

	/* set core doorbell default Qos */
	for (core = 0; core < UTR_CORE_NUM; core++) {
		reg_val = 0;
		for (slot = 0; slot < SLOT_NUM_EACH_CORE; slot++)
			reg_val |= ((unsigned int)core_doorbell_qos
					    [SLOT_NUM_EACH_CORE * core + slot]
				    << (BITS_EACH_SLOT_QOS * slot));

		ufshcd_writel(hba, reg_val, UFS_CORE_DOORBELL_QOS(core));
	}
}

static void ufs_config_qos_outstanding(struct ufs_hba *hba)
{
	unsigned int i;
	unsigned int reg_val;
	unsigned int qos_outstd_num[NUM_OF_QOS_LVL];

	for (i = 0; i < NUM_OF_QOS_LVL; i++)
		qos_outstd_num[i] = hba->qos_ctrl.core_qos_outstd_num[i] - 1;

	reg_val = (qos_outstd_num[0] | (qos_outstd_num[1] << MASK_QOS_1) |
		   (qos_outstd_num[2] << MASK_QOS_2) |
		   (qos_outstd_num[3] << MASK_QOS_3));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_0_3_OUTSTANDING);

	reg_val = (qos_outstd_num[4] |
		  (qos_outstd_num[5] << MASK_QOS_5) |
		  (qos_outstd_num[6] << MASK_QOS_6) |
		  (qos_outstd_num[7] << MASK_QOS_7));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_4_7_OUTSTANDING);
}

static void ufs_config_qos_promote(struct ufs_hba *hba)
{
	unsigned int i;
	unsigned int reg_val;
	unsigned int qos_prmt_num[QOS_PROMOTE_NUM];

	for (i = 0; i < NUM_OF_PROMOTE_QOS_LVL; i++)
		qos_prmt_num[i] =  hba->qos_ctrl.core_qos_prmt_outstd_num[i] - 1;

	reg_val = (qos_prmt_num[0] | (qos_prmt_num[1] << MASK_QOS_1) |
		  (qos_prmt_num[2] << MASK_QOS_2) |
		  (qos_prmt_num[3] << MASK_QOS_3));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_0_3_PROMOTE);

	reg_val = (qos_prmt_num[4] | (qos_prmt_num[5] << MASK_QOS_5) |
		  (qos_prmt_num[6] << MASK_QOS_6));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_4_6_PROMOTE);
}

static void ufs_config_qos_increase(struct ufs_hba *hba)
{
	unsigned int i;
	unsigned int reg_val;
	unsigned int qos_incrs_num[QOS_INCREASE_NUM];

	for (i = 0; i < NUM_OF_PROMOTE_QOS_LVL; i++)
		qos_incrs_num[i] = hba->qos_ctrl.core_qos_incrs_outstd_num[i] - 1;

	reg_val = (qos_incrs_num[0] | (qos_incrs_num[1] << MASK_QOS_1) |
		  (qos_incrs_num[2] << MASK_QOS_2) |
		  (qos_incrs_num[3] << MASK_QOS_3));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_0_3_INCREASE);

	reg_val = (qos_incrs_num[4] | (qos_incrs_num[5] << MASK_QOS_5) |
		  (qos_incrs_num[6] << MASK_QOS_6));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_4_6_INCREASE);
}

int config_hufs_tr_qos(struct ufs_hba *hba)
{
	int ret;
	unsigned long flags;

#ifdef CONFIG_HISI_DEBUG_FS
	if (!hba->qos_ctrl.hufs_qos_inited)
		rdr_syserr_process_for_ap(
				(u32)MODID_AP_S_PANIC_STORAGE, 0ull, 0ull);
#endif

	spin_lock_irqsave(hba->host->host_lock, flags);
	ret = __ufshcd_wait_for_doorbell_clr(hba);
	if (ret) {
		dev_err(hba->dev,
			"wait doorbell clear timeout before config hufs qos\n");
		spin_unlock_irqrestore(hba->host->host_lock, flags);
		return ret;
	}

	ufs_config_qos_outstanding(hba);
	ufs_config_qos_promote(hba);
	ufs_config_qos_increase(hba);

	ufshcd_writel(hba, OUTSTANDING_NUM, UFS_TR_OUTSTANDING_NUM);

	ufs_enable_doorbell_qos(hba);

	spin_unlock_irqrestore(hba->host->host_lock, flags);
	return ret;
}

static inline void ufshcd_sl_core_intr(struct ufs_hba *hba, u32 intr_status,
				unsigned int cpu);
static irqreturn_t ufshcd_hufs_core_intr(int irq, void *__hba)
{
#ifdef CONFIG_SCSI_UFS_FAULT_INJECT
	u32 is_int_set;
#endif
	u32 intr_status, enabled_intr_status;
	irqreturn_t retval = IRQ_NONE;
	struct ufs_hba *hba = __hba;
	unsigned int queue_num;

	queue_num = irq - hba->core_irq[0];
	spin_lock(hba->host->host_lock);
	intr_status = ufshcd_readl(hba, UFS_CORE_IS(queue_num));
#ifdef CONFIG_SCSI_UFS_FAULT_INJECT
	is_int_set = ufshcd_readl(hba, UFS_CORE_IS_INT_SET(queue_num));
	if (is_int_set) {
		intr_status |= is_int_set;
		ufshcd_writel(hba, 0, UFS_CORE_IS_INT_SET(queue_num));
	}
#endif
	enabled_intr_status = intr_status & ufshcd_readl(hba, UFS_CORE_IE(queue_num));

	hba->last_core = queue_num;
	hba->last_core_intr = intr_status;
	hba->last_core_intr_time_stamp = ktime_get();
	if (intr_status)
		ufshcd_writel(hba, intr_status, UFS_CORE_IS(queue_num));

	if (likely(enabled_intr_status)) {
		ufshcd_sl_core_intr(hba, enabled_intr_status, queue_num);
		retval = IRQ_HANDLED;
	}
	spin_unlock(hba->host->host_lock);

	return retval;
}

static int ufshcd_starting_cpu(unsigned int cpu, struct hlist_node *node)
{
	int ret;
	struct ufs_hba *hba =
		hlist_entry_safe(node, struct ufs_hba, node);

	if (!hba || cpu >= UTR_CORE_NUM) {
		pr_err("%s cpu%u invalid para\n", __func__, cpu);
		return 0;
	}

	ret = irq_force_affinity((u32)hba->core_irq[cpu], cpumask_of(cpu));
	if (ret)
		dev_err(hba->dev, "%s set ufs irq %d affinity cpu%u fail %d\n",
			__func__, hba->core_irq[cpu], cpu, ret);

	return 0;
}

static int ufshcd_irq_cpuhp_setup(struct ufs_hba *hba, struct device *dev)
{
	int ret;

	ret = cpuhp_setup_state_multi(CPUHP_AP_IRQ_UFS_STARTING,
				     "scsi/ufs:starting", ufshcd_starting_cpu,
				     NULL);
	if (ret) {
		dev_err(dev, "Failed to register cpuhp notifier for UFS: %d\n",
			ret);
		return ret;
	}

	ret = cpuhp_state_add_instance(CPUHP_AP_IRQ_UFS_STARTING,
				      &hba->node);
	if (ret) {
		dev_err(dev, "Failed to add cpuhp instace: %d\n", ret);
		goto out_cpuhp;
	}

	return 0;
out_cpuhp:
	cpuhp_remove_multi_state(CPUHP_AP_IRQ_UFS_STARTING);
	return ret;
}

void ufshcd_irq_cpuhp_remove(struct ufs_hba *hba)
{
	cpuhp_state_remove_instance(CPUHP_AP_IRQ_UFS_STARTING, &hba->node);
	cpuhp_remove_multi_state(CPUHP_AP_IRQ_UFS_STARTING);
}

int request_irq_for_core_irq(struct ufs_hba *hba, struct device *dev)
{
	unsigned int i;
	int err = 0;

	for (i = 0; i < UTR_CORE_NUM; i++) {
		if (hba->core_irq[i] >= 0) {
			err = devm_request_irq(dev, hba->core_irq[i], ufshcd_hufs_core_intr,
				IRQF_SHARED, hba->core_irq_name[i], hba);
			if (err) {
				dev_err(hba->dev, "request core %u irq failed\n", i);
				return err;
			}
		} else {
			dev_err(hba->dev, "irq for core%u is not available!\n", i);
		}
	}

	for (i = 0; i < UTR_CORE_NUM; i++) {
		err = irq_set_affinity((u32)hba->core_irq[i], cpumask_of(i));
		if (err && cpu_online(i))
			dev_err(hba->dev,
				"set ufs irq affinity cpu%u fail %d\n", i, err);
	}

	return ufshcd_irq_cpuhp_setup(hba, dev);
}

void ufshcd_core_transfer_req_compl(struct ufs_hba *hba,
					   unsigned int queue_num)
{
	unsigned long completed_reqs;
	u32 tr_doorbell;
	const unsigned int core_mask = SLOT_NUM_EACH_CORE * queue_num;

	tr_doorbell = ufshcd_readl(hba, UFS_CORE_UTRLDBR(queue_num));
	completed_reqs =
		((u64)tr_doorbell << (core_mask)) ^ hba->outstanding_reqs;
	completed_reqs &= 0xFFUL << core_mask;

	if (likely(completed_reqs))
		__ufshcd_transfer_req_compl(hba, completed_reqs);
}

static void ufshcd_sl_core_intr(struct ufs_hba *hba, u32 intr_status,
				unsigned int queue_num)
{
	if (unlikely(intr_status & MASK_CORE_IE_IO_TIMEOUT)) {
		dev_err(hba->dev, " UFS_CORE timeout 0x%x\n", intr_status);
		hba->errors |= UFS_CORE_IO_TIMEOUT;
		hufs_key_reg_dump(hba);
		ufshcd_timeout_handler(hba);
	}

	if (likely(intr_status & MASK_CORE_IE_IO_CMPL))
		ufshcd_core_transfer_req_compl(hba, queue_num);
}

void ufshcd_enable_core_utr_intr(struct ufs_hba *hba)
{
	uint8_t core_num;
	u32 set;

	for (core_num = 0; core_num < UTR_CORE_NUM; core_num++) {
		set = ufshcd_readl(hba, UFS_CORE_IE(core_num));
		set |= (MASK_CORE_IE_IO_CMPL | MASK_CORE_IE_IO_TIMEOUT);
		ufshcd_writel(hba, set, UFS_CORE_IE(core_num));
	}
}
#endif /* CONFIG_HISI_UFS_HC_CORE_UTR */

static void ufs_dump_all_mphy_regs(struct ufs_hba *hba)
{
	unsigned int index;
	u32 reg = 0;
	u32 reg_lane1 = 0;

	dev_err(hba->dev,
		": ---------------------------------------------------\n");
	dev_err(hba->dev, "MPHY dump starts\n");
	for (index = 0; index <= MPHY_END_OFF; index++) {
		/* don't print more info if one hufs uic cmd failed */
		if (hufs_uic_read_reg(hba, ATTR_MTX0(index), &reg))
			return;
		(void)hufs_uic_read_reg(hba, ATTR_MTX1(index), &reg_lane1);

		dev_err(hba->dev, "TX 0x%02x, Lane0 = 0x%08x, Lane1 = 0x%08x\n",
			index, reg, reg_lane1);

		(void)hufs_uic_read_reg(hba, ATTR_MRX0(index), &reg);
		(void)hufs_uic_read_reg(hba, ATTR_MRX1(index), &reg_lane1);

		dev_err(hba->dev, "RX 0x%02x, Lane0 = 0x%08x, Lane1 = 0x%08x\n",
			index, reg, reg_lane1);
	}

	dev_err(hba->dev, "MPHY dump Ends\n");
	dev_err(hba->dev,
		"---------------------------------------------------\n");
}

static void ufs_dump_all_l15_regs(struct ufs_hba *hba)
{
	int index;
	int ret;
	u32 reg = 0;

	dev_err(hba->dev,
		"---------------------------------------------------\n");
	dev_err(hba->dev, "L1.5 dump starts\n");
	for (index = L15_START_OFF_1; index < L15_END_OFF_1; index++) {
		ret = hufs_uic_read_reg(hba, index, &reg);
		if (ret != INVALID_MIB_ATTRIBUTE)
			dev_err(hba->dev, "L1.5 0x%x = 0x%x\n", index, reg);
	}

	for (index = L15_START_OFF_2; index < L15_END_OFF_2; index++) {
		ret = hufs_uic_read_reg(hba, index, &reg);
		if (ret != INVALID_MIB_ATTRIBUTE)
			dev_err(hba->dev, "L1.5 0x%x = 0x%x\n", index, reg);
	}

	dev_err(hba->dev, "L1.5 dump Ends\n");
	dev_err(hba->dev,
		"---------------------------------------------------\n");
}

static void ufs_dump_all_hci_regs(struct ufs_hba *hba)
{
	int index;

	dev_err(hba->dev,
		"---------------------------------------------------\n");
	dev_err(hba->dev, "HCI dump Starts\n");
	for (index = 0; index < HCI_END_OFF; index += HCI_OFFSET)
		dev_err(hba->dev, "HCI 0x%x = 0x%x\n", index,
			ufshcd_readl(hba, index));
	dev_err(hba->dev, "HCI dump Ends\n");
	dev_err(hba->dev,
		"---------------------------------------------------\n");
}

static void ufs_dump_all_misc(struct ufs_hba *hba)
{
	int index;
	int ret;
	u32 reg = 0;

	dev_err(hba->dev,
		"---------------------------------------------------\n");
	dev_err(hba->dev, "misc dump starts\n");
	for (index = MISC_START_OFF_1; index < MISC_END_OFF_1; index += 1) {
		ret = hufs_uic_read_reg(hba, index, &reg);
		if (ret != INVALID_MIB_ATTRIBUTE)
			dev_err(hba->dev, "DME 0x%x = 0x%x\n", index, reg);
	}
	for (index = MISC_START_OFF_2; index < MISC_END_OFF_2; index += 1) {
		ret = hufs_uic_read_reg(hba, index, &reg);
		if (ret != INVALID_MIB_ATTRIBUTE)
			dev_err(hba->dev, "DME 0x%x = 0x%x\n", index, reg);
	}
	for (index = MISC_START_OFF_3; index < MISC_END_OFF_3; index += 1) {
		ret = hufs_uic_read_reg(hba, index, &reg);
		if (ret != INVALID_MIB_ATTRIBUTE)
			dev_err(hba->dev, "L2 0x%x = 0x%x\n", index, reg);
	}
	for (index = MISC_START_OFF_4; index < MISC_END_OFF_4; index += 1) {
		ret = hufs_uic_read_reg(hba, index, &reg);
		if (ret != INVALID_MIB_ATTRIBUTE)
			dev_err(hba->dev, "L2 0x%x = 0x%x\n", index, reg);
	}
	dev_err(hba->dev, "misc dump ends\n");
	dev_err(hba->dev,
		"---------------------------------------------------\n");
}

void hufs_error_reg_dump(struct ufs_hba *hba)
{
	/*
	 * reg dump all addr will result in some addr not exists, set print
	 * invalid attr to false to avoid too much useless print.
	 */
	hba->invalid_attr_print = false;
	ufs_dump_all_mphy_regs(hba);
	ufs_dump_all_l15_regs(hba);
	ufs_dump_all_hci_regs(hba);
	ufs_dump_all_misc(hba);
	hba->invalid_attr_print = true;
}

int get_peer_rx_activate_time(struct ufs_hba *hba, u32 *value)
{
	return hufs_uic_peer_get(hba, PEER_MRX0(RX_MIN_ACTIVATETIME_CAPABILITY),
				 value);
}

typedef void (*irq_op_ch)(unsigned int irq);
void hufs_all_irq_op(struct ufs_hba *hba, bool enable)
{
	irq_op_ch irq_op;
	int i;

	if (enable)
		irq_op = enable_irq;
	else
		irq_op = disable_irq;

	irq_op(hba->irq);
#ifndef CONFIG_SCSI_UFS_INTR_HUB
	irq_op(hba->fatal_err_irq);
	irq_op(hba->unipro_irq);
#endif

#ifdef CONFIG_HISI_UFS_HC_CORE_UTR
	for (i = 0; i < UTR_CORE_NUM; i++)
		irq_op(hba->core_irq[i]);
#endif
}

void hufs_enable_all_irq(struct ufs_hba *hba)
{
	hufs_all_irq_op(hba, true);
}

void hufs_disable_all_irq(struct ufs_hba *hba)
{
	hufs_all_irq_op(hba, false);
}

void ufs_rx_eq_sel_ctrl(struct ufs_hba *hba, bool flag)
{
	u32 value = 0;

	if (!hba) {
		pr_err("hba is NULL\n");
		return;
	}

	if (flag) {
		hufs_uic_read_reg(hba, ATTR_MRX0(RX_EQ_SEL_R), &value);
		value |= SW_RX_EQ_SEL_CTRL;
		hufs_uic_write_reg(hba, ATTR_MRX0(RX_EQ_SEL_R), value);
		hufs_uic_read_reg(hba, ATTR_MRX1(RX_EQ_SEL_R), &value);
		value |= SW_RX_EQ_SEL_CTRL;
		hufs_uic_write_reg(hba, ATTR_MRX1(RX_EQ_SEL_R), value);
	} else {
		hufs_uic_read_reg(hba, ATTR_MRX0(RX_EQ_SEL_R), &value);
		value &= ~SW_RX_EQ_SEL_CTRL;
		hufs_uic_write_reg(hba, ATTR_MRX0(RX_EQ_SEL_R), value);
		hufs_uic_read_reg(hba, ATTR_MRX1(RX_EQ_SEL_R), &value);
		value &= ~SW_RX_EQ_SEL_CTRL;
		hufs_uic_write_reg(hba, ATTR_MRX1(RX_EQ_SEL_R), value);
	}
}
