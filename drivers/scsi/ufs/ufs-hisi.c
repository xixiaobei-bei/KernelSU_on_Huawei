/*
 * Copyright (c) 2018-2019, Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define pr_fmt(fmt) "ufshcd :" fmt

#include <linux/bootdevice.h>
#include <linux/dma-mapping.h>
#include <linux/gpio.h>
#include <linux/hisi/hisi_idle_sleep.h>
#include <linux/i2c.h>
#include <linux/kthread.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/vmalloc.h>
#include <soc_crgperiph_interface.h>
#include <soc_sctrl_interface.h>
#include <soc_ufs_sysctrl_interface.h>
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V2
#include <teek_client_api.h>
#include <teek_client_constants.h>
#include <teek_client_id.h>
#endif

#include "dsm_ufs.h"
#include "ufs-kirin.h"
#include "ufshcd.h"
#include "ufshci.h"
#include "unipro.h"
#ifdef CONFIG_HISI_UFS_MANUAL_BKOPS
#include "hisi_ufs_bkops.h"
#endif
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
extern bool ufshcd_support_inline_encrypt(struct ufs_hba *hba);
#endif

int UFS_DOORBELL_Core_0_7_QOS[64] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5,
	6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5,
	6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5,
	6, 7};

/* ufs doorbel  slot 0~31 Qos*/
int UFS_DOORBELL_QOS_VAL[32] = {
	0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
};

int hisi_set_each_cfg_attr(struct ufs_hba *hba, struct ufs_attr_cfg *cfg)
{
	if (!cfg)
		return 0;

	while (cfg->addr != 0) {
		hisi_uic_write_reg(hba, cfg->addr, cfg->val);
		cfg++;
	}
	return 0;
}

int hisi_auto_h8_op(struct ufs_hba *hba, bool auto_h8_op)
{
	uint32_t h8_reg = 0;
	int ret = 0;
	int retry = HISI_UIC_ACCESS_REG_RETRIES;

	if (!ufshcd_is_auto_hibern8_allowed(hba))
		return 0;
	if (auto_h8_op == UFS_HISI_AUTO_H8_DISABLE) {
		h8_reg = ufshcd_readl(hba, UFS_AHIT_CTRL_OFF);
		if (h8_reg & UFS_AUTO_HIBERN_EN) {
			h8_reg &= (~UFS_AUTO_HIBERN_EN);
			ufshcd_writel(hba, h8_reg, UFS_AHIT_CTRL_OFF);

			do {
				h8_reg = ufshcd_readl(
					hba, UFS_AUTO_H8_STATE_OFF);
				if (h8_reg & UFS_HC_AH8_STATE) {
					dev_info(hba->dev,
						"hisi ufs auto h8 disabled\n");
					goto out;
				}
				msleep(1);
			} while (retry--);

			if (retry < 0) {
				dev_err(hba->dev,
					"hisi ufs disable auto h8 timeout\n");
				ret = -ETIMEDOUT;
				goto out;
			}
		}
	} else {
		h8_reg = ufshcd_readl(hba, UFS_AHIT_CTRL_OFF);
		if (!(h8_reg & UFS_AUTO_HIBERN_EN)) {
			h8_reg |= UFS_AUTO_HIBERN_EN;
			ufshcd_writel(hba, h8_reg, UFS_AHIT_CTRL_OFF);
			dev_info(hba->dev, "hisi ufs auto h8 enabled\n");
		}
		goto out;
	}
out:
	return ret;
}

int hisi_uic_write_reg(struct ufs_hba *hba, uint32_t reg_offset, uint32_t value)
{
	int ret = 0;

	ret = hisi_auto_h8_op(hba, UFS_HISI_AUTO_H8_DISABLE);
	if (ret)
		return ret;
	/*lint -e679*/
	writel(value,
		hba->ufs_unipro_base + (reg_offset << HISI_UNIPRO_BIT_SHIFT));
	/*lint +e679*/

	ret = hisi_auto_h8_op(hba, UFS_HISI_AUTO_H8_ENABLE);
	return ret;
}
/*lint -e713*/
int hisi_uic_read_reg(struct ufs_hba *hba, uint32_t reg_offset, u32 *value)
{
	int ret = 0;
	uint32_t dbg_value = 0;

	ret = hisi_auto_h8_op(hba, UFS_HISI_AUTO_H8_DISABLE);
	if (ret)
		return ret;
	/*lint -e679*/
	*value = (u32)readl(
		hba->ufs_unipro_base + (reg_offset << HISI_UNIPRO_BIT_SHIFT));
	/*lint +e679*/
	dbg_value = (u32)readl(hba->ufs_unipro_base +
			       (DME_LOCAL_OPC_STATE << HISI_UNIPRO_BIT_SHIFT));
	if (dbg_value) {
		dev_err(hba->dev,
			"[Local OPC error] Read [0x%x] error : 0x%x\n",
			reg_offset, dbg_value);
		ret = dbg_value;
	}
	ret = hisi_auto_h8_op(hba, UFS_HISI_AUTO_H8_ENABLE);
	return ret;
}

int hisi_uic_peer_set(struct ufs_hba *hba, uint32_t offset, uint32_t value)
{
	uint32_t ctrl_dw = 0;
	int ret = 0;
	/*disable auto H8 first if enabled*/
	ret = hisi_auto_h8_op(hba, UFS_HISI_AUTO_H8_DISABLE);
	if (ret)
		return ret;

	hisi_uic_write_reg(hba, DME_PEER_OPC_WDATA, value);

	/*(MIBAttrib) | (GenSelectorIndex << 16) | (AttrType << 27) | (0x1 <<
	 * 28) | (0x1 << 31)*/
	ctrl_dw = offset | UFS_BIT(28) | UFS_BIT(31);
	hisi_uic_write_reg(hba, DME_PEER_OPC_CTRL, ctrl_dw);

	ret = hisi_auto_h8_op(hba, UFS_HISI_AUTO_H8_ENABLE);
	return ret;
}

int hisi_uic_peer_get(struct ufs_hba *hba, uint32_t offset, uint32_t *value)
{
	uint32_t ctrl_dw = 0;
	int ret = 0;
	struct completion attr_peer_get_done;
	/*disable auto H8 first if enabled*/
	ret = hisi_auto_h8_op(hba, UFS_HISI_AUTO_H8_DISABLE);
	if (ret)
		return ret;

	/*(MIBAttrib) | (GenSelectorIndex << 16) | (AttrType << 27) | (0x0 <<
	 * 28) | (0x1 << 31)*/
	ctrl_dw = offset | UFS_BIT(31);

	init_completion(&attr_peer_get_done);
	hba->uic_peer_get_done = &attr_peer_get_done;
	hisi_uic_write_reg(hba, DME_PEER_OPC_CTRL, ctrl_dw);

	if (wait_for_completion_timeout(hba->uic_peer_get_done,
		    msecs_to_jiffies(HISI_UIC_ACCESS_REG_TIMEOUT)))
		hisi_uic_read_reg(hba, DME_PEER_OPC_RDATA, value);
	else
		ret = -ETIMEDOUT;

	hisi_auto_h8_op(hba, UFS_HISI_AUTO_H8_ENABLE);
	return ret;
}
/***************************************************************
 *
 * snps_to_hisi_addr
 * Description: the address transltation is different between
 *     HISI and SNPS UFS. For backward compatible, use this function
 *     to translate the address.
 *
 ***************************************************************/
/*SNPS addr coding : 0x REG_addr | select
 example: TX 0x41 lane0 --> 0x00410000
	  TX 0x41 lane1 --> 0x00410001
	  RX 0xC1 lane0 --> 0x00C10004
	  RX 0xC1 lane1 --> 0x00C10005

HISI addr coding : 0x select | Reg_addr
 example: TX 0x41 lane0 --> 0x00000041
	  TX 0x41 lane1 --> 0x00010041
	  RX 0xC1 lane0 --> 0x000200C1
	  RX 0xC1 lane1 --> 0x000300C1  */
uint32_t snps_to_hisi_addr(uint32_t cmd, uint32_t arg1)
{
	uint32_t temp = 0;
	uint32_t rx0_sel = HISI_RX0_SEL;
	uint32_t rx1_sel = HISI_RX1_SEL;

	if (arg1 == 0xD0850000) {
		temp = 0x0000D014;
		return temp;
	}
	if ((cmd == UIC_CMD_DME_PEER_SET) || (cmd == UIC_CMD_DME_PEER_GET)) {
		/* If peer operation, we should use spec-defined SEL */
		rx0_sel = SPEC_RX0_SEL;
		rx1_sel = SPEC_RX1_SEL;
	}

	temp = ((arg1 & UIC_ADDR_MASK) >> UIC_SHIFT);
	temp &= UIC_SLE_MASK;
	if ((arg1 & 0xF) == SNPS_TX0_SEL) {
		temp |= (HISI_TX0_SEL << UIC_SHIFT);
	} else if ((arg1 & 0xF) == SNPS_TX1_SEL) {
		temp |= (HISI_TX1_SEL << UIC_SHIFT);
	} else if ((arg1 & 0xF) == SNPS_RX0_SEL) {
		temp |= (rx0_sel << UIC_SHIFT);
	} else if ((arg1 & 0xF) == SNPS_RX1_SEL) {
		temp |= (rx1_sel << UIC_SHIFT);
	}
	return temp;
}

int hisi_dme_link_startup(struct ufs_hba *hba)
{
	int retry = HISI_UIC_ACCESS_REG_RETRIES;
	int timeout = HISI_UFS_DME_LINKUP_TIMEOUT;
	int ret = 0;
	uint32_t value = 0;
	uint32_t link_state;
	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;

	hisi_uic_read_reg(hba, DME_UNIPRO_STATE, &value);
	link_state = value & MASK_DEBUG_UNIPRO_STATE;
	if (link_state != LINK_DOWN) {
		dev_err(hba->dev, "unipro is not linkdown, error\n");
		return ret;
	}

	while (retry--) {
		hisi_uic_write_reg(hba, DME_LINKSTARTUPREQ, 0x1);
		while (timeout--) {
			hisi_uic_read_reg(hba, DME_LINKSTARTUP_STATE, &value);
			value &= LINK_STARTUP_CNF;
			if (value == LINK_STARTUP_SUCC) {
				dev_info(
					hba->dev, "ufs link startup success\n");
				goto out;
			} else if (value == LINK_STARTUP_FAIL) {
				ufshcd_vops_device_reset(hba);
				writel(0x00010001,
					SOC_UFS_Sysctrl_UFS_RESET_CTRL_ADDR(
						host->sysctrl));
				ufshcd_hba_enable(hba);
				break;
			}
			msleep(1);
		}
	}
	if (retry < 0) {
		dev_err(hba->dev, "ufs link startup wait DME_LINKSTARTUP_STATE "
				  "timeout\n");
		hisi_uic_read_reg(hba, DME_UNIPRO_STATE, &value);
		dev_err(hba->dev, "DME_UNIPRO_STATE is 0x%x\n", value);
		hisi_uic_read_reg(hba, PA_FSM_STATUS, &value);
		dev_err(hba->dev, "PA_FSM_STATUS is 0x%x\n", value);
		hisi_uic_read_reg(hba, PA_STATUS, &value);
		dev_err(hba->dev, "PA_STATUS is 0x%x\n", value);

		ret = -ETIMEDOUT;
		goto out;
	}
out:
	return ret;
}
/*lint -e778*/
int ufshcd_hisi_wait_for_unipro_register_poll(
	struct ufs_hba *hba, u32 reg, u32 mask, u32 val, int timeout_ms)
{
	int ret = 0;
	uint32_t reg_val;
	timeout_ms = timeout_ms * 10;
	while (timeout_ms-- > 0) {
		hisi_uic_read_reg(hba, reg, &reg_val);
		if ((reg_val & mask) == (val & mask))
			return ret;
		udelay(100);
	}
	ret = -ETIMEDOUT;
	return ret;
}
/*lint +e778*/

int ufshcd_hisi_uic_change_pwr_mode(struct ufs_hba *hba, u8 mode)
{
	int ret = 0;
	u32 reg_val;
	struct completion hisi_uic_done;

	if (hba->quirks & UFSHCD_QUIRK_BROKEN_PA_RXHSUNTERMCAP) {
		ret = ufshcd_dme_set(
			hba, UIC_ARG_MIB_SEL(PA_RXHSUNTERMCAP, 0), 1);
		if (ret) {
			dev_err(hba->dev, "%s: failed to enable "
					  "PA_RXHSUNTERMCAP ret %d\n",
				__func__, ret);
			return ret;
		}
	}

	ret = hisi_uic_write_reg(hba, PA_PWRMODE, mode);
	if (ret)
		return ret;
	init_completion(&hisi_uic_done);
	hba->hisi_uic_done = &hisi_uic_done;
	if (wait_for_completion_timeout(hba->hisi_uic_done,
		    msecs_to_jiffies(HISI_UIC_ACCESS_REG_TIMEOUT))) {
		return ret;
	} else {
		hisi_uic_read_reg(hba, DME_POWERMODEIND, &reg_val);
		dev_err(hba->dev,
			"hisi ufs pmc fail, DME_POWERMODEIND is 0x%x\n",
			reg_val);
		hisi_uic_read_reg(hba, DME_UNIPRO_STATE, &reg_val);
		dev_err(hba->dev, "DME_UNIPRO_STATE is 0x%x\n", reg_val);
		return -ETIMEDOUT;
	}
}

int ufshcd_hisi_hibern8_op_irq_safe(struct ufs_hba *hba, bool h8_op)
{
	u32 value = 0;
	u32 ie_value;
	u32 dme_intr_value;
	int ret = 0;
	int retries = HISI_UIC_ACCESS_REG_RETRIES;
	int timeout_ms = HISI_UIC_ACCESS_REG_TIMEOUT;
	/* emulator have no real MPHY */
	if (unlikely(hba->host->is_emulator))
		return 0;

	/* disable auto h8 when we use software h8 */
	hisi_auto_h8_op(hba, UFS_HISI_AUTO_H8_DISABLE);
	hisi_uic_write_reg(hba, DME_CTRL1, UNIPRO_CLK_AUTO_GATE_WHEN_HIBERN);

	/* step 1: close interrupt and save interrupt value */
	ie_value = ufshcd_readl(hba, REG_INTERRUPT_ENABLE);
	ufshcd_writel(hba, 0, REG_INTERRUPT_ENABLE);

	/* close DME hibernate interrupt */
	hisi_uic_read_reg(hba, DME_INTR_ENABLE, &value);
	dme_intr_value = value;
	if (h8_op == UFS_HISI_H8_OP_ENTER) {
		value &= ~DME_HIBERN_ENTER_INTR;
		hisi_uic_write_reg(hba, DME_INTR_ENABLE, value);

		hisi_uic_read_reg(hba, DME_UNIPRO_STATE, &value);
		if ((value & MASK_DEBUG_UNIPRO_STATE) != LINK_UP) {
			dev_err(hba->dev, "unipro state is not up, can't do "
					  "hibernate enter\n");
			return -1;
		}

	} else {
		value &= ~DME_HIBERN_EXIT_INTR;
		hisi_uic_write_reg(hba, DME_INTR_ENABLE, value);

		hisi_uic_read_reg(hba, DME_UNIPRO_STATE, &value);
		if ((value & MASK_DEBUG_UNIPRO_STATE) != LINK_HIBERN) {
			dev_err(hba->dev, "unipro state is not hibernate, "
					  "can't do hibernate exit\n");
			return ret;
		}
	}

retry:
	hisi_uic_read_reg(hba, DME_HIBERNATE_ENTER_STATE, &value);
	hisi_uic_read_reg(hba, DME_HIBERNATE_EXIT_STATE, &value);

	if (h8_op == UFS_HISI_H8_OP_ENTER) {
		hisi_uic_write_reg(hba, DME_HIBERNATE_ENTER, 0x1);

		ret = ufshcd_hisi_wait_for_unipro_register_poll(hba,
			DME_HIBERNATE_ENTER_STATE, DME_HIBERNATE_REQ_RECEIVED,
			DME_HIBERNATE_REQ_RECEIVED, timeout_ms);
		if (ret) {
			hisi_uic_read_reg(
				hba, DME_HIBERNATE_ENTER_STATE, &value);

			if (value == DME_HIBERNATE_REQ_DENIED && retries--) {
				goto retry;
			}
			goto out;
		}

		ret = ufshcd_hisi_wait_for_unipro_register_poll(hba,
			DME_HIBERNATE_ENTER_IND, DME_HIBERNATE_ENTER_LOCAL_SUCC,
			DME_HIBERNATE_ENTER_LOCAL_SUCC, timeout_ms);
		if (ret) {
			hisi_uic_read_reg(hba, DME_HIBERNATE_ENTER_IND, &value);
			dev_err(hba->dev, "Enter hibernate fail, "
					  "DME_HIBERNATEENTERIND is 0x%x\n",
				value);
			goto out;
			// to add err handle
		}
		clear_dme_intr(hba,
			DME_HIBERN_ENTER_CNF_INTR | DME_HIBERN_ENTER_IND_INTR);
		ufshcd_writel(hba, UIC_HIBERNATE_ENTER, REG_INTERRUPT_STATUS);
		dev_err(hba->dev, "Enter hibernate success\n");

	} else {
		hisi_uic_write_reg(hba, DME_HIBERNATE_EXIT, 0x1);

		ret = ufshcd_hisi_wait_for_unipro_register_poll(hba,
			DME_HIBERNATE_EXIT_STATE, DME_HIBERNATE_REQ_RECEIVED,
			DME_HIBERNATE_REQ_RECEIVED, timeout_ms);
		if (ret) {
			hisi_uic_read_reg(
				hba, DME_HIBERNATE_EXIT_STATE, &value);
			if (value == DME_HIBERNATE_REQ_DENIED && retries--) {
				goto retry;
			}
			goto out;
		}

		ret = ufshcd_hisi_wait_for_unipro_register_poll(hba,
			DME_HIBERNATE_EXIT_IND, DME_HIBERNATE_EXIT_LOCAL_SUCC,
			DME_HIBERNATE_EXIT_LOCAL_SUCC, timeout_ms);
		if (ret) {
			hisi_uic_read_reg(hba, DME_HIBERNATE_EXIT_IND, &value);
			dev_err(hba->dev, "Exit hibernate fail, "
					  "DME_HIBERNATE_EXIT_IND is 0x%x\n",
				value);
			goto out;
			// to add err handle (modify)
		}
		clear_dme_intr(hba,
			DME_HIBERN_EXIT_CNF_INTR | DME_HIBERN_EXIT_IND_INTR);
		ufshcd_writel(hba, UIC_HIBERNATE_EXIT, REG_INTERRUPT_STATUS);
		dev_info(hba->dev, "Exit hibernate success\n");
	}

#ifdef CONFIG_SCSI_UFS_HS_ERROR_RECOVER
	if ((!h8_op) && hba->check_pwm_after_h8) {
		if (!hba->vops->get_pwr_by_debug_register) {
			dev_err(hba->dev, "no check pwm op\n");
			hba->check_pwm_after_h8 = 0;
		} else {
			value = hba->vops->get_pwr_by_debug_register(hba);
			if (value == SLOW) {
				dev_err(hba->dev, "ufs pwr = 0x%x after H8\n",
					value);
				hba->check_pwm_after_h8 = 0;
				ufshcd_init_pwr_info(hba);
				if (!work_busy(&hba->recover_hs_work))
					schedule_work(&hba->recover_hs_work);
				else
					dev_err(hba->dev, "%s:recover_hs_work "
							  "is runing \n",
						__func__);
			} else {
				hba->check_pwm_after_h8--;
			}
			dev_err(hba->dev,
				"check pwr after H8, %d times remain\n",
				hba->check_pwm_after_h8);
		}
	}
#endif
out:
	/* need modify? */
	ufsdbg_error_inject_dispatcher(
		hba, ERR_INJECT_PWR_MODE_CHANGE_ERR, 0, &ret);

	if (ret) {
		ufshcd_writel(hba, UIC_ERROR, REG_INTERRUPT_STATUS);
		dsm_ufs_update_error_info(hba, DSM_UFS_ENTER_OR_EXIT_H8_ERR);
		schedule_ufs_dsm_work(hba);
	}
	/* need modify? */

	ufshcd_writel(hba, ie_value, REG_INTERRUPT_ENABLE);
	ufshcd_writel(hba, dme_intr_value, DME_INTR_ENABLE);
	return ret;
}

int config_hisi_tr_qos(struct ufs_hba *hba)
{
	int ret;
	int slot = 0;
	int core = 0;
	unsigned int reg_val;
	int index = 0;
	unsigned long flags;

	spin_lock_irqsave(hba->host->host_lock, flags);
	ret = __ufshcd_wait_for_doorbell_clr(hba);
	if (ret) {
		dev_err(hba->dev,
			"wait doorbell clear timeout before config hisi qos\n");
		spin_unlock_irqrestore(hba->host->host_lock, flags);
		return ret;
	}

	reg_val =
		(QOS_0_OUTSTANDING_NUM | (QOS_1_OUTSTANDING_NUM << MASK_QOS_1) |
			(QOS_2_OUTSTANDING_NUM << MASK_QOS_2) |
			(QOS_3_OUTSTANDING_NUM << MASK_QOS_3));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_0_3_OUTSTANDING);
	reg_val = ufshcd_readl(hba, UFS_TR_QOS_0_3_OUTSTANDING);
	dev_err(hba->dev, "test UFS_TR_QOS_0_3_OUTSTANDING ufs is 0x%x\n",
		reg_val);

	reg_val =
		(QOS_4_OUTSTANDING_NUM | (QOS_5_OUTSTANDING_NUM << MASK_QOS_5) |
			(QOS_6_OUTSTANDING_NUM << MASK_QOS_6) |
			(QOS_7_OUTSTANDING_NUM << MASK_QOS_7));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_4_7_OUTSTANDING);
	reg_val = ufshcd_readl(hba, UFS_TR_QOS_4_7_OUTSTANDING);
	dev_err(hba->dev, "test UFS_TR_QOS_4_7_OUTSTANDING ufs is 0x%x\n",
		reg_val);

	reg_val = (QOS_0_PROMOTE_NUM | (QOS_1_PROMOTE_NUM << MASK_QOS_1) |
		   (QOS_2_PROMOTE_NUM << MASK_QOS_2) |
		   (QOS_3_PROMOTE_NUM << MASK_QOS_3));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_0_3_PROMOTE);
	reg_val = ufshcd_readl(hba, UFS_TR_QOS_0_3_PROMOTE);
	dev_err(hba->dev, "test UFS_TR_QOS_0_3_PROMOTE ufs is 0x%x\n", reg_val);

	reg_val = (QOS_4_PROMOTE_NUM | (QOS_5_PROMOTE_NUM << MASK_QOS_5) |
		   (QOS_6_PROMOTE_NUM << MASK_QOS_6));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_4_6_PROMOTE);
	reg_val = ufshcd_readl(hba, UFS_TR_QOS_4_6_PROMOTE);
	dev_err(hba->dev, "test UFS_TR_QOS_4_6_PROMOTE ufs is 0x%x\n", reg_val);

	reg_val = (QOS_0_INCREASE_NUM | (QOS_1_INCREASE_NUM << MASK_QOS_1) |
		   (QOS_2_INCREASE_NUM << MASK_QOS_2) |
		   (QOS_3_INCREASE_NUM << MASK_QOS_3));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_0_3_INCREASE);
	reg_val = ufshcd_readl(hba, UFS_TR_QOS_0_3_INCREASE);
	dev_err(hba->dev, "test UFS_TR_QOS_0_3_INCREASE ufs is 0x%x\n",
		reg_val);

	reg_val = (QOS_4_INCREASE_NUM | (QOS_5_INCREASE_NUM << MASK_QOS_5) |
		   (QOS_6_INCREASE_NUM << MASK_QOS_6));
	ufshcd_writel(hba, reg_val, UFS_TR_QOS_4_6_INCREASE);
	reg_val = ufshcd_readl(hba, UFS_TR_QOS_4_6_INCREASE);
	dev_err(hba->dev, "test UFS_TR_QOS_4_6_INCREASE ufs is 0x%x\n",
		reg_val);

	ufshcd_writel(hba, OUTSTANDING_NUM, UFS_TR_OUTSTANDING_NUM);

	/* Enable Qos */
	reg_val = ufshcd_readl(hba, UFS_PROC_MODE_CFG);
	reg_val |= MASK_CFG_UTR_QOS_EN;
	ufshcd_writel(hba, reg_val, UFS_PROC_MODE_CFG);

	/* set UFSHCI doorbell Qos */
	reg_val = 0;
	for (slot = 0; slot <= 7; slot++) {
		reg_val |= (UFS_DOORBELL_QOS_VAL[slot] << (4 * slot));
	}
	ufshcd_writel(hba, reg_val, UFS_DOORBELL_0_7_QOS);
	reg_val = 0;
	for (slot = 8; slot <= 15; slot++) {
		index = slot - 8;
		reg_val |= (UFS_DOORBELL_QOS_VAL[slot] << (4 * index));
	}
	ufshcd_writel(hba, reg_val, UFS_DOORBELL_8_15_QOS);
	reg_val = 0;
	for (slot = 16; slot <= 23; slot++) {
		index = slot - 16;
		reg_val |= (UFS_DOORBELL_QOS_VAL[slot] << (4 * index));
	}
	ufshcd_writel(hba, reg_val, UFS_DOORBELL_16_23_QOS);
	reg_val = 0;
	for (slot = 24; slot <= 31; slot++) {
		index = slot - 24;
		reg_val |= (UFS_DOORBELL_QOS_VAL[slot] << (4 * index));
	}
	ufshcd_writel(hba, reg_val, UFS_DOORBELL_24_31_QOS);

	/* set core 0~7 doorbell Qos */
    /*lint -e679*/
	reg_val = 0;
	for (core = 0; core <= 7; core++) {
		for (slot = 0; slot <= 7; slot++) {
			reg_val |= ((unsigned int)UFS_DOORBELL_Core_0_7_QOS[8 * core + slot]
				    << (4 * slot));
		}
		ufshcd_writel(hba, reg_val, 0x2010 + core * 0x80);
	}
    /*lint +e679*/

	spin_unlock_irqrestore(hba->host->host_lock, flags);
	return ret;
}

void ufshcd_hisi_enable_auto_hibern8(struct ufs_hba *hba)
{
	uint32_t value;
	unsigned long flags;

	spin_lock_irqsave(hba->host->host_lock, flags);
	if (hba->autoh8_disable_depth > 0) {
		hba->autoh8_disable_depth--;
	} else {
		dev_err(hba->dev, "unblance auto hibern8 enabled\n");
		// HISI_UFS_BUG(); //modify
	}
	if (hba->autoh8_disable_depth == 0) {
		hisi_uic_write_reg(
			hba, DME_CTRL1, UNIPRO_CLK_AUTO_GATE_WHEN_HIBERN);

		/* disable hibernate unipro intr */
		hisi_uic_read_reg(hba, DME_INTR_ENABLE, &value);
		value &= ~DME_HIBERN_INTR;
		hisi_uic_write_reg(hba, DME_INTR_ENABLE, value);

		value = ufshcd_readl(hba, REG_INTERRUPT_ENABLE);
		value |= (UIC_HIBERNATE_ENTER | UIC_HIBERNATE_EXIT);
		ufshcd_writel(hba, value, REG_INTERRUPT_ENABLE);

		value = ufshcd_readl(hba, UFS_AHIT_CTRL_OFF);
		value &= ~UFS_HIBERNATE_EXIT_MODE;
		ufshcd_writel(hba, value, UFS_AHIT_CTRL_OFF);

		ufshcd_writel(hba, hba->ahit, REG_CONTROLLER_AHIT);

		/* Enable auto hibernate8 */
		value = ufshcd_readl(hba, UFS_AHIT_CTRL_OFF);
		value |= UFS_AUTO_EN;
		ufshcd_writel(hba, value, UFS_AHIT_CTRL_OFF);
	}

	spin_unlock_irqrestore(hba->host->host_lock, flags);
}

void ufshcd_hisi_disable_auto_hibern8(struct ufs_hba *hba)
{
	uint32_t value;
	unsigned long flags;

	spin_lock_irqsave(hba->host->host_lock, flags);
	if (hba->autoh8_disable_depth > 0) {
		hba->autoh8_disable_depth++;
	} else {
		hba->autoh8_disable_depth++;
		/* Disable auto hibernate8 */
		value = ufshcd_readl(hba, UFS_AHIT_CTRL_OFF);
		value &= ~UFS_AUTO_EN;
		ufshcd_writel(hba, value, UFS_AHIT_CTRL_OFF);
	}
	spin_unlock_irqrestore(hba->host->host_lock, flags);
}
void hisi_ufs_dme_reg_dump(struct ufs_hba *hba)
{
	uint32_t reg_val;
	hisi_uic_read_reg(hba, DME_UNIPRO_STATE, &reg_val);
	dev_err(hba->dev, "DME_UNIPRO_STATE is 0x%x\n", reg_val);
	if (hba->errors & UIC_HIBERNATE_ENTER) {
		dev_err(hba->dev, "enter auto hibernate error\n");
		hisi_uic_read_reg(hba, DME_HIBERNATE_ENTER_STATE, &reg_val);
		dev_err(hba->dev, "DME_HIBERNATEENTER_STATE is 0x%x\n",
			reg_val);
		hisi_uic_read_reg(hba, DME_HIBERNATE_ENTER_IND, &reg_val);
		dev_err(hba->dev, "DME_HIBERNATE_ENTER_IND is 0x%x\n", reg_val);
	}
	if (hba->errors & UIC_HIBERNATE_EXIT) {
		dev_err(hba->dev, "exit auto hibernate error\n");
		hisi_uic_read_reg(hba, DME_HIBERNATE_EXIT_STATE, &reg_val);
		dev_err(hba->dev, "DME_HIBERNATEEXIT_STATE is 0x%x\n", reg_val);
		hisi_uic_read_reg(hba, DME_HIBERNATE_EXIT_IND, &reg_val);
		dev_err(hba->dev, "DME_HIBERNATE_EXIT_IND is 0x%x\n", reg_val);
	}
}

/* clear dme interrupt */
/*lint -e732*/
void clear_dme_intr(struct ufs_hba *hba, int dme_intr_clr)
{
	unsigned int value;
	hisi_uic_read_reg(hba, DME_INTR_CLR, &value);
	value |= (unsigned int)dme_intr_clr;
	hisi_uic_write_reg(hba, DME_INTR_CLR, value);
}
/*lint +e732*/

/* enable clock gating when ufshcd is idle*/
void ufshcd_enable_clock_gating(struct ufs_hba *hba)
{
	hisi_uic_write_reg(hba, UFS_BLOCK_CG_CFG, 0xF);
}

void ufs_hisi_kirin_pwr_change_pre_change(
	struct ufs_hba *hba, struct ufs_pa_layer_attr *dev_req_params)
{
	uint32_t value = 0;
	uint8_t deemp_20t4_en = 0;
	struct ufs_attr_cfg *cfg;
	struct ufs_kirin_host *host = hba->priv;

	pr_info("%s ++\n", __func__);
#ifdef CONFIG_HISI_DEBUG_FS
	pr_info("device manufacturer_id is 0x%x\n", hba->manufacturer_id);
#endif

	/* these for snps, delete? */

	/*ARIES platform need to set SaveConfigTime to 0x13, and change sync
	 * length to maximum value */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0xD0A0),
		0x13); /* VS_DebugSaveConfigTime */
	ufshcd_dme_set(
		hba, UIC_ARG_MIB((u32)0x1552), 0x4f); /* g1 sync length */
	ufshcd_dme_set(
		hba, UIC_ARG_MIB((u32)0x1554), 0x4f); /* g2 sync length */
	ufshcd_dme_set(
		hba, UIC_ARG_MIB((u32)0x1556), 0x4f); /* g3 sync length */
	if (value < 0xA)
		ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0x15a7),
			0xA); /* PA_Hibern8Time */
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0x15a8), 0xA); /* PA_Tactivate */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(0xd085, 0x0), 0x01);  //lint !e648

	ufshcd_dme_set(hba, UIC_ARG_MIB(0x155c), 0x0); /* PA_TxSkip */

	/*PA_PWRModeUserData0 = 8191, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b0), 8191);
	/*PA_PWRModeUserData1 = 65535, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b1), 65535);
	/*PA_PWRModeUserData2 = 32767, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b2), 32767);
	/*DME_FC0ProtectionTimeOutVal = 8191, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0xd041), 8191);
	/*DME_TC0ReplayTimeOutVal = 65535, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0xd042), 65535);
	/*DME_AFC0ReqTimeOutVal = 32767, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0xd043), 32767);
	/*PA_PWRModeUserData3 = 8191, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b3), 8191);
	/*PA_PWRModeUserData4 = 65535, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b4), 65535);
	/*PA_PWRModeUserData5 = 32767, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x15b5), 32767);
	/*DME_FC1ProtectionTimeOutVal = 8191, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0xd044), 8191);
	/*DME_TC1ReplayTimeOutVal = 65535, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0xd045), 65535);
	/*DME_AFC1ReqTimeOutVal = 32767, default is 0*/
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)0xd046), 32767);

	pr_info("%s --\n", __func__);
	/* for hisi asic mphy and emu, use USE_HISI_MPHY_ASIC on ASIC later */
	if (!(host->caps & USE_HISI_MPHY_TC) || hba->host->is_emulator) {
		if (dev_req_params->pwr_rx == FAST_MODE ||
			dev_req_params->pwr_rx == FASTAUTO_MODE) {
			deemp_20t4_en = 1;

			switch (dev_req_params->gear_rx) {
			case UFS_HS_G4:
				cfg = hisi_mphy_V300_pre_pmc_fsg4_attr;
				break;
			case UFS_HS_G3:
				cfg = hisi_mphy_V300_pre_pmc_fsg3_attr;
				break;
			case UFS_HS_G2:
				cfg = hisi_mphy_V300_pre_pmc_fsg2_attr;
				break;
			case UFS_HS_G1:
				deemp_20t4_en = 0;
				cfg = hisi_mphy_V300_pre_pmc_fsg1_attr;
				break;
			default:
				dev_err(hba->dev, "unkown ufs gear\n");
				cfg = 0;
				break;
			}
		} else {
			cfg = hisi_mphy_V300_pre_pmc_slow_attr;
		}
		hisi_set_each_cfg_attr(hba, cfg);

		/* read TX 0x50 */
		hisi_uic_read_reg(hba, 0x00000050, &value);
		if (deemp_20t4_en)
			value |= MPHY_DEEMPH_20T4_EN;
		else
			value &= (~MPHY_DEEMPH_20T4_EN);

		hisi_uic_write_reg(hba, 0x00000050, value); /*TX 0x50 Lane0*/
		hisi_uic_write_reg(hba, 0x00010050, value); /*TX 0x50 Lane1*/
	}

	if ((host->caps & USE_HISI_MPHY_TC)) {
		if (IS_V200_MPHY(hba)) {
			// hisi_mphy_V200_pwr_change_pre_config(hba, host);
			if (dev_req_params->pwr_rx == FAST_MODE ||
				dev_req_params->pwr_rx == FASTAUTO_MODE) {
				switch (dev_req_params->gear_rx) {
				case UFS_HS_G4:
					cfg = hisi_mphy_V200_tc_pre_pmc_fsg4_attr;
					break;
				case UFS_HS_G3:
					cfg = hisi_mphy_V200_tc_pre_pmc_fsg3_attr;
					break;
				case UFS_HS_G2:
					cfg = hisi_mphy_V200_tc_pre_pmc_fsg2_attr;
					break;
				case UFS_HS_G1:
					cfg = hisi_mphy_V200_tc_pre_pmc_fsg1_attr;
					break;
				default:
					dev_err(hba->dev, "unkown ufs gear\n");
					cfg = 0;
					break;
				}
			} else {
				cfg = hisi_mphy_V200_tc_pre_pmc_slow_attr;
			}
			hisi_set_each_cfg_attr(hba, cfg);
		}
	}
	return;
}
/*lint +e648 +e845*/
bool ufshcd_is_hisi_ufs_hc_used(struct ufs_hba *hba)
{
	if (hba->use_hisi_ufs_hc)
		return true;
	else
		return false;
}

/**
 *  * ufshcd_enable_intr - enable hisi unipro interrupts
 *   * @hba: per adapter instance
 *    * @intrs: interrupt bits
 *     */
void ufshcd_hisi_enable_unipro_intr(struct ufs_hba *hba, u32 unipro_intrs)
{
	u32 set = 0;
	hisi_uic_read_reg(hba, DME_INTR_ENABLE, &set);
	set |= unipro_intrs;
	hisi_uic_write_reg(hba, DME_INTR_ENABLE, set);
}

irqreturn_t ufshcd_hisi_unipro_intr(int unipro_irq, void *__hba)
{
	u32 unipro_intr_status, enabled_unipro_intr_status, value, result,
		reg_value, reg_offset;
	irqreturn_t retval = IRQ_NONE;
	struct ufs_hba *hba = __hba;

	spin_lock(hba->host->host_lock);

	hisi_uic_read_reg(hba, DME_INTR_STATUS, &unipro_intr_status);
	clear_dme_intr(hba, unipro_intr_status);

	hisi_uic_read_reg(hba, DME_INTR_ENABLE, &value);
	enabled_unipro_intr_status = unipro_intr_status & value;

	if (enabled_unipro_intr_status & MASK_DME_PMC_IND) {
		hisi_uic_read_reg(hba, DME_POWERMODEIND, &reg_value);
		if (reg_value & DME_POWER_MODE_LOCAL_SUCC) {
			complete(hba->hisi_uic_done);
			dev_err(hba->dev, "ufs pmc complete\n");
		}
	} else if (enabled_unipro_intr_status & MASK_LOCAL_ATTR_FAIL) {
		hisi_uic_read_reg(hba, DME_LOCAL_OPC_DBG, &value);
		if (value) {
			reg_offset = value & ATTR_LOCAL_ERR_ADDR;
			result = value & ATTR_LOCAL_ERR_RES;
			dev_err(hba->dev, "local attr access fail, "
					  "local_opc_dbg is 0x%x, reg_offset "
					  "is 0x%x\n",
				result, reg_offset);
			if (result == DME_LOCAL_OPC_BUSY) {
				dev_err(hba->dev, "hisi_uic_reg_access busy, "
						  "try again\n");
			}
		}
	} else if (enabled_unipro_intr_status & MASK_PEER_ATTR_COMPL) {
		hisi_uic_read_reg(hba, DME_PEER_OPC_STATE, &value);
		if (value & PEER_ATTR_RES)
			dev_err(hba->dev, "peer attr access fail\n");
		else
			complete(hba->uic_peer_get_done);
	} else
		dev_err(hba->dev, "ufs unipro intr status 0x%x\n",
			unipro_intr_status);

	retval = IRQ_HANDLED;

	spin_unlock(hba->host->host_lock);
	return retval;
}

void ufshcd_sl_fatal_intr(struct ufs_hba *hba, u32 intr_status)
{
	ufsdbg_error_inject_dispatcher(
		hba, ERR_INJECT_INTR, intr_status, &intr_status);
	hba->errors = INT_FATAL_ERRORS & intr_status;

#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
	if (ufshcd_support_inline_encrypt(hba))
		hba->errors |= CRYPTO_ENGINE_FATAL_ERROR & intr_status;
#endif

	if (hba->errors)
		ufshcd_check_errors(hba);
}

irqreturn_t ufshcd_hisi_fatal_err_intr(int fatal_err_irq, void *__hba)
{
	u32 fatal_intr_status, enabled_fatal_intr_status;
	irqreturn_t retval = IRQ_NONE;
	struct ufs_hba *hba = __hba;
	spin_lock(hba->host->host_lock);
	fatal_intr_status = ufshcd_readl(hba, REG_INTERRUPT_STATUS);
	dev_err(hba->dev, "fatal_err intr status 0x%x\n", fatal_intr_status);

	enabled_fatal_intr_status =
		fatal_intr_status & ufshcd_readl(hba, REG_INTERRUPT_ENABLE);
	if (enabled_fatal_intr_status & INT_FATAL_ERRORS) {
		ufshcd_sl_fatal_intr(hba, enabled_fatal_intr_status);
	}

	retval = IRQ_HANDLED;
	spin_unlock(hba->host->host_lock);
	return retval;
}

irqreturn_t ufshcd_hisi_core0_intr(int fatal_err_irq, void *__hba)
{
	irqreturn_t retval = IRQ_NONE;
	struct ufs_hba *hba = __hba;

	spin_lock(hba->host->host_lock);

	retval = IRQ_HANDLED;
	spin_unlock(hba->host->host_lock);
	return retval;
}

int wait_mphy_init_done(struct ufs_hba *hba)
{
	int ret = 0;
	u32 timeout_ms = MPHY_INIT_TIMEOUT;

	if (unlikely(hba->host->is_emulator))
		return 0;

	ret = ufshcd_hisi_wait_for_unipro_register_poll(
		hba, MPHY_INIT, MASK_MPHY_INIT, MPHY_INIT_DONE, timeout_ms);

	if (ret < 0) {
		dev_err(hba->dev, "Wait_mphy_init_done failed %d\n", ret);
	}
	return ret;
}
/*lint +e713*/
