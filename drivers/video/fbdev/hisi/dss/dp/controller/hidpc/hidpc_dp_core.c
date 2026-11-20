/*
 * Copyright (c) 2016 Synopsys, Inc.
 *
 * Synopsys DP TX Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/*
 * Copyright (c) 2017 Hisilicon Tech. Co., Ltd. Integrated into the Hisilicon display system.
 */

#include "hidpc_dp_core.h"
#include "../dp_avgen_base.h"
#include "hidpc_dp_avgen.h"
#include "hidpc_dp_aux.h"
#include "hidpc_dp_irq.h"
#include "hidpc_reg.h"
#include "../dp_core_interface.h"
#include "../../phy/dp_phy_config_interface.h"
#include "hisi_dp.h"
#include "hisi_fb.h"
#include "hisi_fb_def.h"
#include "../dsc/dsc_config_base.h"
#if defined(CONFIG_DP_HDCP)
#include "hdcp/hdcp_common.h"
#endif

/*
 * Core Access Layer
 *
 * Provides low-level register access to the DPTX core.
 */

/**
 * dptx_intr_en() - Enables interrupts
 * @dptx: The dptx struct
 * @bits: The interrupts to enable
 *
 * This function enables (unmasks) all interrupts in the INTERRUPT
 * register specified by @bits.
 */
/*lint -save -e* */
static void dptx_intr_en(struct dp_ctrl *dptx, uint32_t bits)
{
	uint32_t ien;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	DPU_FB_INFO("[DP] Enable DPTX Intr: %x\n", bits);
	ien = dptx_readl(dptx, DPTX_IEN);
	ien |= bits;
	dptx_writel(dptx, DPTX_IEN, ien);
}

/**
 * dptx_intr_dis() - Disables interrupts
 * @dptx: The dptx struct
 * @bits: The interrupts to disable
 *
 * This function disables (masks) all interrupts in the INTERRUPT
 * register specified by @bits.
 */
static void dptx_intr_dis(struct dp_ctrl *dptx, uint32_t bits)
{
	uint32_t ien;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	DPU_FB_INFO("[DP] Disable DPTX Intr: %x\n", bits);
	ien = dptx_readl(dptx, DPTX_IEN);
	ien &= ~bits;
	dptx_writel(dptx, DPTX_IEN, ien);
}

/**
 * dptx_global_intr_en() - Enables top-level interrupts
 * @dptx: The dptx struct
 *
 * Enables (unmasks) all top-level interrupts.
 */
void dptx_global_intr_en(struct dp_ctrl *dptx)
{
	uint32_t hpd_ien;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");
	dptx_intr_en(dptx, DPTX_IEN_ALL_INTR);

	dptx_writel(dptx, DPTX_WRAP_REG_DPC_INT_MASK, 0x3);

	/* Enable all HPD interrupts */
	hpd_ien = dptx_readl(dptx, DPTX_HPD_IEN);
	hpd_ien |= (DPTX_HPD_IEN_IRQ_EN |
		    DPTX_HPD_IEN_HOT_PLUG_EN |
		    DPTX_HPD_IEN_HOT_UNPLUG_EN);
	dptx_writel(dptx, DPTX_HPD_IEN, hpd_ien);

#if defined(CONFIG_DP_HDCP)
	hdcp_intr_en(dptx);
#endif
}

/**
 * dptx_global_intr_dis() - Disables top-level interrupts
 * @dptx: The dptx struct
 *
 * Disables (masks) all top-level interrupts.
 */
void dptx_global_intr_dis(struct dp_ctrl *dptx)
{
	uint32_t hpd_ien;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	dptx_intr_dis(dptx, DPTX_IEN_ALL_INTR);
	dptx_writel(dptx, DPTX_WRAP_REG_DPC_INT_MASK, 0x0);

	/* all HPD interrupts */
	hpd_ien = dptx_readl(dptx, DPTX_HPD_IEN);
	hpd_ien &= ~(DPTX_HPD_IEN_IRQ_EN |
		    DPTX_HPD_IEN_HOT_PLUG_EN |
		    DPTX_HPD_IEN_HOT_UNPLUG_EN);
	dptx_writel(dptx, DPTX_HPD_IEN, hpd_ien);

#if defined(CONFIG_DP_HDCP)
	hdcp_intr_dis(dptx);
#endif
}

/**
 * dptx_clear_intr() - cleat top-level interrupts
 * @dptx: The dptx struct
 *
 * clear all top-level interrupts.
 */
void dptx_global_intr_clear(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	dptx_writel(dptx, DPTX_ISTS, DPTX_ISTS_ALL_INTR);

	/* all HPD interrupts */
	dptx_writel(dptx, DPTX_HPDSTS, (DPTX_HPDSTS_IRQ | DPTX_HPDSTS_HOT_PLUG | DPTX_HPDSTS_HOT_UNPLUG));
}

static int dp_dis_reset(struct dpu_fb_data_type *dpufd, bool benable)
{

	dpu_check_and_return((dpufd == NULL), -EINVAL, ERR, "[DP] dpufd is NULL!\n");

	if (benable) {
		/* dp dis reset */
		outp32(dpufd->peri_crg_base + PERRSTDIS0, 0x00000400);
	} else {
		/* dp reset */
		outp32(dpufd->peri_crg_base + PERRSTEN0, 0x00000400);
	}

	return 0;
}

/**
 * dptx_soft_reset() - Performs a core soft reset
 * @dptx: The dptx struct
 * @bits: The components to reset
 *
 * Resets specified parts of the core by writing @bits into the core
 * soft reset control register and clearing them 10-20 microseconds
 * later.
 */
void dptx_soft_reset(struct dp_ctrl *dptx, uint32_t bits)
{
	uint32_t rst;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	bits &= (DPTX_SRST_CTRL_ALL);

	/* Set reset bits */
	rst = dptx_readl(dptx, DPTX_SRST_CTRL);
	rst |= bits;
	dptx_writel(dptx, DPTX_SRST_CTRL, rst);

	usleep_range(10, 20);

	/* Clear reset bits */
	rst = dptx_readl(dptx, DPTX_SRST_CTRL);
	rst &= ~bits;
	dptx_writel(dptx, DPTX_SRST_CTRL, rst);
}

/**
 * dptx_soft_reset_all() - Reset all core modules
 * @dptx: The dptx struct
 */
void dptx_soft_reset_all(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	dptx_soft_reset(dptx, DPTX_SRST_CTRL_ALL);
}

void dptx_phy_soft_reset(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	dptx_soft_reset(dptx, DPTX_SRST_CTRL_PHY);
}

void dptx_typec_reset_ack(struct dp_ctrl *dptx)
{
	uint32_t dp_status;
	uint32_t reg;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	reg = dptx_readl(dptx, DPTX_TYPE_C_CTRL);
	dp_status = (reg & DPTX_TYPEC_DISABLE_STATUS) >> 1;
	reg &= ~DPTX_TYPEC_DISABLE_ACK;
	reg |= dp_status; /* DPTX_TYPEC_DISABLE_ACK */
	dptx_writel(dptx, DPTX_TYPE_C_CTRL, reg);
}

/**
 * dptx_core_init_phy() - Initializes the DP TX PHY module
 * @dptx: The dptx struct
 *
 * Initializes the PHY layer of the core. This needs to be called
 * whenever the PHY layer is reset.
 */
void dptx_core_init_phy(struct dp_ctrl *dptx)
{
	uint32_t phyifctrl;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	/* Set 40-bit PHY width */
	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	if (g_fpga_flag == 1)
		phyifctrl |= DPTX_PHYIF_CTRL_WIDTH; /* FPGA need 40 bit */
	else
		phyifctrl &= ~DPTX_PHYIF_CTRL_WIDTH; /* UDP need 20 bit */

	dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);
}

/**
 * dptx_check_dptx_id() - Check value of DPTX_ID register
 * @dptx: The dptx struct
 *
 * Returns True if DPTX core correctly identifyed.
 */
bool dptx_check_dptx_id(struct dp_ctrl *dptx)
{
	uint32_t dptx_id;

	dpu_check_and_return((dptx == NULL), false, ERR, "[DP] NULL Pointer\n");

	dptx_id = dptx_readl(dptx, DPTX_ID);
	if (dptx_id != ((DPTX_ID_DEVICE_ID << DPTX_ID_DEVICE_ID_SHIFT) |
			DPTX_ID_VENDOR_ID))
		return false;

	return true;
}
/**
 * dptx_core_init() - Initializes the DP TX core
 * @dptx: The dptx struct
 *
 * Initialize the DP TX core and put it in a known state.
 */
int dptx_core_init(struct dp_ctrl *dptx)
{
	char str[11];
	uint32_t version;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	/* Reset the core */
	dptx_soft_reset_all(dptx);
	dptx_typec_reset_ack(dptx);


	dptx_phy_set_lanes_power(dptx, false);

	dptx_writel(dptx, DPTX_CCTL, 0x0);

	/* Check the core version */
	memset(str, 0, sizeof(str));
	version = dptx_readl(dptx, DPTX_VER_NUMBER);
	str[0] = (char)(version >> 24) & 0xff;
	str[1] = '.';
	str[2] = (char)(version >> 16) & 0xff;
	str[3] = (char)(version >> 8) & 0xff;
	str[4] = (char)(version & 0xff);

	version = dptx_readl(dptx, DPTX_VER_TYPE);
	str[5] = '-';
	str[6] = (char)(version >> 24) & 0xff;
	str[7] = (char)(version >> 16) & 0xff;
	str[8] = (char)(version >> 8) & 0xff;
	str[9] = (char)(version & 0xff);
	str[10] = '\0';

	DPU_FB_DEBUG("Core version: %s\n", str);
	dptx->version = version;

	dptx_core_init_phy(dptx);

	return 0;
}

/**
 * dptx_core_deinit() - Deinitialize the core
 * @dptx: The dptx struct
 *
 * Disable the core in preparation for module shutdown.
 */
int dptx_core_deinit(struct dp_ctrl *dptx)
{

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	dptx_global_intr_dis(dptx);
	dptx_soft_reset_all(dptx);


	if (dptx->dptx_aux_disreset)
		dptx->dptx_aux_disreset(dptx, false);
	return 0;
}

int dptx_core_on(struct dp_ctrl *dptx)
{
	int ret = 0;
	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL\n");
	dpu_check_and_return((!dptx_check_dptx_id(dptx)), -EINVAL, ERR, "[DP] DPTX_ID not match !\n");

	ret = dptx_core_init(dptx);
	if (ret) {
		DPU_FB_ERR("[DP] DPTX core init failed!\n");
		return ret;
	}

	/* FIXME: clear intr */
	dptx_global_intr_dis(dptx);
	/* dptx_global_intr_clear(dptx); */
	enable_irq(dptx->irq);
	/* Enable all top-level interrupts */
	dptx_global_intr_en(dptx);

	return 0;
}

void dptx_core_off(struct dpu_fb_data_type *dpufd, struct dp_ctrl *dptx)
{

	dpu_check_and_no_retval((dpufd == NULL), ERR, "[DP] dpufd is NULL\n");
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] dptx is NULL\n");

	/* FIXME: clear intr */
	dptx_global_intr_dis(dptx);
	disable_irq_nosync(dptx->irq);

	dptx_core_deinit(dptx);

}

void dptx_core_remove(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	dptx_core_deinit(dptx);
}

void dptx_free_lanes(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	if (dptx->dptx_disable_default_video_stream)
		dptx->dptx_disable_default_video_stream(dptx, 0);
	dptx_typec_reset_ack(dptx);
	dptx_phy_set_lanes_power(dptx, false);
}

/*
 * PHYIF core access functions
 */

int dptx_phy_get_lanes(struct dp_ctrl *dptx)
{
	uint32_t phyifctrl;
	uint32_t val;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	val = (phyifctrl & DPTX_PHYIF_CTRL_LANES_MASK) >>
		DPTX_PHYIF_CTRL_LANES_SHIFT;

	return (1 << val);
}

void dptx_phy_set_lanes(struct dp_ctrl *dptx, uint32_t lanes)
{
	uint32_t phyifctrl;
	uint32_t val;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	DPU_FB_INFO("[DP] DPTX set lanes =%d\n", lanes);

	switch (lanes) {
	case 1:
		val = 0;
		break;
	case 2:
		val = 1;
		break;
	case 4:
		val = 2;
		break;
	default:
		DPU_FB_ERR("[DP] Invalid number of lanes %d\n", lanes);
		return;
	}

	phyifctrl = 0;
	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	phyifctrl &= ~DPTX_PHYIF_CTRL_LANES_MASK;
	phyifctrl |= (val << DPTX_PHYIF_CTRL_LANES_SHIFT);
	dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);

}

void dptx_phy_set_rate(struct dp_ctrl *dptx, uint32_t rate)
{
	uint32_t phyifctrl;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	DPU_FB_INFO("[DP] DPTX set rate=%d\n", rate);

	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);

	switch (rate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
	case DPTX_PHYIF_CTRL_RATE_HBR:
	case DPTX_PHYIF_CTRL_RATE_HBR2:
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		break;
	default:
		DPU_FB_ERR("[DP] Invalid PHY rate %d\n", rate);
		break;
	}

	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	phyifctrl &= ~DPTX_PHYIF_CTRL_RATE_MASK;
	phyifctrl |= rate << DPTX_PHYIF_CTRL_RATE_SHIFT;
	dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);
}

int dwc_phy_get_rate(struct dp_ctrl *dptx)
{
	uint32_t phyifctrl;
	uint32_t rate;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	rate = (phyifctrl & DPTX_PHYIF_CTRL_RATE_MASK) >>
		DPTX_PHYIF_CTRL_RATE_SHIFT;

	return rate;
}

int dptx_phy_wait_busy(struct dp_ctrl *dptx, uint32_t lanes)
{
	unsigned long dw_jiffies = 0;
	uint32_t phyifctrl;
	uint32_t mask = 0;

	DPU_FB_INFO("[DP] lanes=%d\n", lanes);

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	/* FPGA don't need config phy */
	if (g_fpga_flag == 1)
		return 0;

	switch (lanes) {
	/* This case (value 4) is not terminated by a 'break' statement */
	case 4:
		mask |= DPTX_PHYIF_CTRL_BUSY(3);
		mask |= DPTX_PHYIF_CTRL_BUSY(2);
	/* This case (value 2) is not terminated by a 'break' statement */
	case 2:
		mask |= DPTX_PHYIF_CTRL_BUSY(1);
	case 1:
		mask |= DPTX_PHYIF_CTRL_BUSY(0);
		break;
	default:
		DPU_FB_ERR("[DP] Invalid number of lanes %d\n", lanes);
		break;
	}

	dw_jiffies = jiffies + HZ / 10;

	do {
		phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);

		if (!(phyifctrl & mask))
			return 0;

		udelay(1);
	} while (time_after(dw_jiffies, jiffies));

	DPU_FB_ERR("PHY BUSY timed out\n");
	return -EBUSY;
}

void dptx_phy_set_pre_emphasis(struct dp_ctrl *dptx, uint32_t lane,
	uint32_t level)
{
	uint32_t phytxeq;

	DPU_FB_INFO("[DP] lane=%d, level=0x%x\n", lane, level);

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	if (lane > 3) {
		DPU_FB_ERR("[DP] Invalid lane %d\n", lane);
		return;
	}

	if (level > 3) {
		DPU_FB_ERR("[DP] Invalid pre-emphasis level %d, using 3", level);
		level = 3;
	}

	phytxeq = dptx_readl(dptx, DPTX_PHY_TX_EQ);
	phytxeq &= ~DPTX_PHY_TX_EQ_PREEMP_MASK(lane);
	phytxeq |= (level << DPTX_PHY_TX_EQ_PREEMP_SHIFT(lane)) &
		DPTX_PHY_TX_EQ_PREEMP_MASK(lane);
	dptx_writel(dptx, DPTX_PHY_TX_EQ, phytxeq);
}

void dptx_phy_set_vswing(struct dp_ctrl *dptx, uint32_t lane, uint32_t level)
{
	uint32_t phytxeq;

	DPU_FB_INFO("[DP] lane=%d, level=0x%x\n", lane, level);

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	if (lane > 3) {
		DPU_FB_ERR("[DP] Invalid lane %d\n", lane);
		return;
	}

	if (level > 3) {
		DPU_FB_ERR("[DP] Invalid vswing level %d, using 3", level);
		level = 3;
	}

	phytxeq = dptx_readl(dptx, DPTX_PHY_TX_EQ);
	phytxeq &= ~DPTX_PHY_TX_EQ_VSWING_MASK(lane);
	phytxeq |= (level << DPTX_PHY_TX_EQ_VSWING_SHIFT(lane)) &
		DPTX_PHY_TX_EQ_VSWING_MASK(lane);

	dptx_writel(dptx, DPTX_PHY_TX_EQ, phytxeq);
}

void dptx_phy_set_pattern(struct dp_ctrl *dptx, uint32_t pattern)
{
	uint32_t phyifctrl = 0;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	DPU_FB_INFO("[DP] Setting PHY pattern=0x%x\n", pattern);

	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	phyifctrl &= ~DPTX_PHYIF_CTRL_TPS_SEL_MASK;
	phyifctrl |= ((pattern << DPTX_PHYIF_CTRL_TPS_SEL_SHIFT) &
		      DPTX_PHYIF_CTRL_TPS_SEL_MASK);
	dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);
}

void dptx_phy_enable_xmit(struct dp_ctrl *dptx, uint32_t lanes, bool enable)
{
	uint32_t phyifctrl;
	uint32_t mask = 0;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	DPU_FB_INFO("[DP] lanes=%d, enable=%d\n", lanes, enable);

	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);

	switch (lanes) {
	/* This case (value 4) is not terminated by a 'break' statement */
	case 4:
		mask |= DPTX_PHYIF_CTRL_XMIT_EN(3);
		mask |= DPTX_PHYIF_CTRL_XMIT_EN(2);
	/* This case (value 2) is not terminated by a 'break' statement */
	case 2:
		mask |= DPTX_PHYIF_CTRL_XMIT_EN(1);
	case 1:
		mask |= DPTX_PHYIF_CTRL_XMIT_EN(0);
		break;
	default:
		DPU_FB_ERR("[DP] Invalid number of lanes %d\n", lanes);
		break;
	}

	if (enable)
		phyifctrl |= mask;
	else
		phyifctrl &= ~mask;

	dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);
}

void dptx_phy_set_lanes_power(struct dp_ctrl *dptx, bool bopen)
{
	uint32_t phyifctrl;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	if (bopen) {
		phyifctrl &= ~DPTX_PHYIF_CTRL_PWRDOWN_MASK;
	} else {
		phyifctrl |= (3 << 17);
	}
	dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);
#if CONFIG_DP_SETTING_COMBOPHY && defined(CONFIG_HISI_FB_970)
	if (dptx->dptx_combophy_set_lanes_power)
		dptx->dptx_combophy_set_lanes_power(bopen);
#endif
}

static int dptx_ssc_dis_switch(struct dp_ctrl *dptx, bool downspread_support)
{
	uint32_t phyifctrl;
	int retval;
	uint8_t byte = 0;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

#if CONFIG_DP_SETTING_COMBOPHY && defined(CONFIG_HISI_FB_970)
	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	if (downspread_support)
		phyifctrl &= ~DPTX_PHYIF_CTRL_SSC_DIS;
	else
		phyifctrl |= DPTX_PHYIF_CTRL_SSC_DIS;
	dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);

	if (dptx->dptx_combophy_set_ssc_dis)
		dptx->dptx_combophy_set_ssc_dis(dptx, downspread_support);
#else
		phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
		if (downspread_support)
			phyifctrl &= ~DPTX_PHYIF_CTRL_SSC_DIS;
		else
			phyifctrl |= DPTX_PHYIF_CTRL_SSC_DIS;
		dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);
#endif

	retval = dptx_read_dpcd(dptx, DP_DOWNSPREAD_CTRL, &byte);
	if (retval) {
		DPU_FB_ERR("[DP] Read DPCD error\n");
		return retval;
	}

	if (downspread_support)
		byte |= DP_SPREAD_AMP_0_5;
	else
		byte &= ~DP_SPREAD_AMP_0_5;
	retval = dptx_write_dpcd(dptx, DP_DOWNSPREAD_CTRL, byte);
	if (retval) {
		DPU_FB_ERR("[DP] write DPCD error\n");
		return retval;
	}

	return 0;
}

static int dptx_set_ssc(struct dp_ctrl *dptx, bool bswitchphy)
{
	int retval = 0;
	uint8_t byte = 0;
	uint32_t phyifctrl;

	/* FPGA don't need config phy */
	if (g_fpga_flag == 1)
		return 0;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	if (bswitchphy)	{
		dptx_phy_set_lanes_power(dptx, false);

		/* Wait for PHY busy */
		retval = dptx_phy_wait_busy(dptx, dptx->link.lanes);
		if (retval) {
			DPU_FB_ERR("[DP] Timed out waiting for PHY BUSY\n");
			return retval;
		}
	}

	retval = dptx_read_dpcd(dptx, DP_MAX_DOWNSPREAD, &byte);
	if (retval) {
		DPU_FB_ERR("[DP] Read DPCD error\n");
		return retval;
	}

	DPU_FB_INFO("[DP] SSC setting:  %x\n", byte);

	phyifctrl = dptx_readl(dptx, DPTX_PHYIF_CTRL);
	phyifctrl |= DPTX_PHYIF_CTRL_SSC_DIS;
	dptx_writel(dptx, DPTX_PHYIF_CTRL, phyifctrl);

	retval = dptx_ssc_dis_switch(dptx, byte & DP_MAX_DOWNSPREAD_0_5);
	if (retval)
		return retval;

	if (bswitchphy) {
		/* Wait for PHY busy */
		retval = dptx_phy_wait_busy(dptx, dptx->link.lanes);
		if (retval) {
			DPU_FB_ERR("[DP] Timed out waiting for PHY BUSY\n");
			return retval;
		}

		dptx_phy_set_lanes_power(dptx, true);

		/* Wait for PHY busy */
		retval = dptx_phy_wait_busy(dptx, dptx->link.lanes);
		if (retval) {
			DPU_FB_ERR("[DP] Timed out waiting for PHY BUSY\n");
			return retval;
		}

		if (dptx->dptx_phy_enable_xmit)
			dptx->dptx_phy_enable_xmit(dptx, dptx->link.lanes, true);
	}

	return 0;
}

void dptx_phy_set_ssc(struct dp_ctrl *dptx, bool bswitchphy)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	dptx_set_ssc(dptx, bswitchphy);
}

/**
 * Get params decided by core
 * Called by dp_device_init, dptx's initial default params decided by core
 */
static void dptx_default_params_from_core(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	if (g_fpga_flag == 1) {
		dptx->max_rate = DPTX_PHYIF_CTRL_RATE_HBR;
	} else {
#ifdef CONFIG_HISI_FB_970
		dptx->max_rate = DPTX_PHYIF_CTRL_RATE_HBR2;
#else
		dptx->max_rate = DPTX_PHYIF_CTRL_RATE_HBR2;
#endif
	}

	dptx->multipixel = DPTX_MP_DUAL_PIXEL;
	dptx->max_lanes = DPTX_DEFAULT_LINK_LANES;
}

int dptx_set_enhanced_framing_mode(struct dp_ctrl *dptx)
{
	int retval;
	uint32_t cctl;
	uint8_t byte;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	byte = dptx->link.lanes;
	cctl = dptx_readl(dptx, DPTX_CCTL);

	if (dptx->efm_en && drm_dp_enhanced_frame_cap(dptx->rx_caps)) {
		byte |= DP_ENHANCED_FRAME_CAP;
		cctl |= DPTX_CCTL_ENH_FRAME_EN;
	} else {
		cctl &= ~DPTX_CCTL_ENH_FRAME_EN;
	}

	dptx_writel(dptx, DPTX_CCTL, cctl);

	retval = dptx_write_dpcd(dptx, DP_LANE_COUNT_SET, byte);
	if (retval)
		return retval;

	return 0;
}

/* Check sink device is connected or not */
static bool dptx_sink_device_connected(struct dp_ctrl *dptx)
{
	u32 hpd_sts;

	dpu_check_and_return(!dptx, false, ERR, "[DP] NULL Pointer\n");

	hpd_sts = dptx_readl(dptx, DPTX_HPDSTS);

	return (hpd_sts & DPTX_HPDSTS_STATUS);
}

void dptx_link_core_reset(struct dp_ctrl *dptx)
{

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	dptx_soft_reset(dptx,
		DPTX_SRST_CTRL_PHY | DPTX_SRST_CTRL_HDCP | DPTX_SRST_CTRL_AUX);


	/* Enable AUX Block */
	if (dptx->dptx_aux_disreset)
		dptx->dptx_aux_disreset(dptx, true);
	dptx_core_init_phy(dptx);
}

void dptx_link_set_preemp_vswing(struct dp_ctrl *dptx)
{
	uint32_t i;
	uint8_t pe;
	uint8_t vs;
#if CONFIG_DP_SETTING_COMBOPHY
	int retval;
#endif

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	/* FPGA don't need config phy */
	if (g_fpga_flag == 1)
		return;

	for (i = 0; i < dptx->link.lanes; i++) {
		huawei_dp_imonitor_set_param_vs_pe(i, &(dptx->link.vswing_level[i]), &(dptx->link.preemp_level[i]));
		pe = dptx->link.preemp_level[i];
		vs = dptx->link.vswing_level[i];
		DPU_FB_INFO("[DP] lane: %d, vs: %d, pe: %d\n", i, vs, pe);
#if CONFIG_DP_SETTING_COMBOPHY
			if (dptx->dptx_combophy_set_preemphasis_swing)
				dptx->dptx_combophy_set_preemphasis_swing(dptx, i, vs, pe);
#else
			dptx_phy_set_pre_emphasis(dptx, i, pe);
			dptx_phy_set_vswing(dptx, i, vs);
#endif
	}

#if CONFIG_DP_SETTING_COMBOPHY
	/* Wait for PHY busy */
	retval = dptx_phy_wait_busy(dptx, dptx->link.lanes);
	if (retval) {
		DPU_FB_ERR("[DP] Timed out 1 waiting for PHY BUSY\n");
		return;
	}
#endif
}

static int dptx_set_sink_lane_status(struct dp_ctrl *dptx)
{
	int retval = 0;
	uint8_t byte;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	retval = dptx_phy_rate_to_bw(dptx->link.rate);
	if (retval < 0)
		return retval;

	byte = (uint8_t)retval;
	retval = dptx_write_dpcd(dptx, DP_LINK_BW_SET, byte);
	if (retval)
		return retval;


	byte = DP_SPREAD_AMP_0_5;
	retval = dptx_write_dpcd(dptx, DP_DOWNSPREAD_CTRL, byte);
	if (retval)
		return retval;

	byte = 1;
	retval = dptx_write_dpcd(dptx, DP_MAIN_LINK_CHANNEL_CODING_SET, byte);
	if (retval)
		return retval;

	return 0;
}

static int dptx_link_set_lane_status(struct dp_ctrl *dptx)
{
	int retval = 0;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	dptx_phy_set_lanes_power(dptx, false);

	/* Wait for PHY busy */
	retval = dptx_phy_wait_busy(dptx, dptx->link.lanes);
	if (retval) {
		DPU_FB_ERR("[DP] Timed out 1 waiting for PHY BUSY\n");
		return retval;
	}

	/* Initialize PHY */
	dptx_phy_set_lanes(dptx, dptx->link.lanes);
	dptx_phy_set_rate(dptx, dptx->link.rate);
	if (dptx->dptx_phy_set_ssc)
		dptx->dptx_phy_set_ssc(dptx, false);

	/* Wait for PHY busy */
	retval = dptx_phy_wait_busy(dptx, dptx->link.lanes);
	if (retval) {
		DPU_FB_ERR("[DP] Timed out 2 waiting for PHY BUSY\n");
		return retval;
	}


	dptx_phy_set_lanes_power(dptx, true);

	/* Wait for PHY busy */
	retval = dptx_phy_wait_busy(dptx, dptx->link.lanes);
	if (retval) {
		DPU_FB_ERR("[DP] Timed out 3 waiting for PHY BUSY\n");
		return retval;
	}

	/* Set PHY_TX_EQ */
	if (dptx->dptx_link_set_preemp_vswing)
		dptx->dptx_link_set_preemp_vswing(dptx);

	if (dptx->dptx_phy_set_pattern)
		dptx->dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_NONE);
	retval = dptx_write_dpcd(dptx, DP_TRAINING_PATTERN_SET, DP_TRAINING_PATTERN_DISABLE);
	if (retval)
		return retval;

	if (dptx->dptx_phy_enable_xmit)
		dptx->dptx_phy_enable_xmit(dptx, dptx->link.lanes, true);

	retval = dptx_set_enhanced_framing_mode(dptx);
	if (retval)
		return retval;

	retval = dptx_set_sink_lane_status(dptx);
	if (retval)
		return retval;

	return 0;
}

void dptx_initiate_mst_act(struct dp_ctrl *dptx)
{
	uint32_t reg;
	int count = 0;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	reg = dptx_readl(dptx, DPTX_CCTL);
	reg |= DPTX_CCTL_INITIATE_MST_ACT;
	dptx_writel(dptx, DPTX_CCTL, reg);

	while (1) {
		reg = dptx_readl(dptx, DPTX_CCTL);
		if (!(reg & DPTX_CCTL_INITIATE_MST_ACT))
			break;

		count++;
		if (WARN(count > 100, "CCTL.ACT timeout\n"))
			break;

		mdelay(10);
	}
}

void dptx_clear_vcpid_table(struct dp_ctrl *dptx)
{
	int i;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	for (i = 0; i < 8; i++)
		dptx_writel(dptx, DPTX_MST_VCP_TABLE_REG_N(i), 0);
}

void dptx_set_vcpid_table_slot(struct dp_ctrl *dptx, uint32_t slot, uint32_t stream)
{
	uint32_t offset;
	uint32_t reg;
	uint32_t lsb;
	uint32_t mask;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] dptx is NULL\n");
	dpu_check_and_no_retval((slot > 63), ERR, "Invalid slot number > 63");

	offset = DPTX_MST_VCP_TABLE_REG_N(slot >> 3);
	reg = dptx_readl(dptx, offset);

	lsb = (slot & 0x7) * 4;
	mask = GENMASK(lsb + 3, lsb);

	reg &= ~mask;
	reg |= (stream << lsb) & mask;

	DPU_FB_DEBUG("[DP] Writing 0x%08x val=0x%08x\n", offset, reg);
	dptx_writel(dptx, offset, reg);
}

void dptx_ctrl_layer_init(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] NULL Pointer\n");

	dptx->aux_rw = dptx_aux_rw;

	dptx->dptx_hpd_handler = dptx_hpd_handler;
	dptx->dptx_hpd_irq_handler = dptx_hpd_irq_handler;
	dptx->dptx_irq = dptx_irq;
	dptx->dptx_threaded_irq = dptx_threaded_irq;

	dptx->dp_dis_reset = dp_dis_reset;
	dptx->dptx_core_on = dptx_core_on;
	dptx->dptx_core_off = dptx_core_off;
	dptx->dptx_core_remove = dptx_core_remove;

	dptx->dptx_free_lanes = dptx_free_lanes;
	dptx->dptx_link_core_reset = dptx_link_core_reset;
	dptx->dptx_default_params_from_core = dptx_default_params_from_core;
	dptx->dptx_sink_device_connected = dptx_sink_device_connected;

	dptx->dptx_hdr_infoframe_set_reg = dptx_hdr_infoframe_set_reg;

	dptx->dptx_audio_config = dptx_audio_config;
	dptx->dptx_video_core_config = dptx_video_core_config;
	dptx->dptx_link_timing_config = dptx_link_timing_config;
	dptx->dptx_video_ts_change = dptx_video_ts_change;
	dptx->dptx_video_ts_calculate = dptx_video_ts_calculate;
	dptx->dptx_link_close_stream = dptx_link_close_stream;

	dptx->dptx_fec_enable = NULL; /* plateform before V510 do not support fec */

	dptx->dptx_link_set_lane_status = dptx_link_set_lane_status;
	dptx->dptx_link_set_lane_after_training = dptx_link_set_lane_after_training;
	dptx->dptx_phy_set_pattern = dptx_phy_set_pattern;
	dptx->dptx_link_set_preemp_vswing = dptx_link_set_preemp_vswing;

	dptx->dptx_enable_default_video_stream = dptx_enable_default_video_stream;
	dptx->dptx_disable_default_video_stream = dptx_disable_default_video_stream;
	dptx->dptx_triger_media_transfer = dptx_triger_media_transfer;
	dptx->dptx_resolution_switch = dptx_resolution_switch;
	dptx->dptx_phy_enable_xmit = dptx_phy_enable_xmit;

	dptx->dptx_dsc_cfg = dptx_dsc_cfg;
	dptx->dptx_slice_height_limit = dptx_slice_height_limit;
	dptx->dptx_line_buffer_depth_limit = dptx_line_buffer_depth_limit;
	dptx->dptx_dsc_sdp_manul_send = NULL;

	/* for test */
	dptx->dptx_audio_num_ch_change = dptx_audio_num_ch_change;
	dptx->dptx_video_timing_change = dptx_video_timing_change;
	dptx->dptx_video_bpc_change = dptx_video_bpc_change;
	dptx->dptx_audio_infoframe_sdp_send = dptx_audio_infoframe_sdp_send;
	dptx->handle_test_link_phy_pattern = handle_test_link_phy_pattern;
	dptx->dptx_phy_set_ssc = dptx_phy_set_ssc;

	dptx_phy_layer_init(dptx);
}
/*lint -restore*/
