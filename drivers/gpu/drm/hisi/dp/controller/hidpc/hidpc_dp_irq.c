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
#include <drm/drm_crtc_helper.h>
#include <drm/drm_connector.h>

#include "hisi_dp_drv.h"
#include "hisi_defs.h"
#include "hidpc_dp_avgen.h"
#include "hidpc_dp_core.h"
#include "hidpc_dp_irq.h"
#include "hidpc_reg.h"
#include "../dp_core_interface.h"
#include "../../link/dp_irq.h"
#include "../../link/dp_aux.h"


/*lint -save -e* */
int handle_test_link_phy_pattern(struct dp_ctrl *dptx)
{
	int retval;
	uint8_t pattern = 0;
	uint32_t phy_pattern = 0;

	drm_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] NULL Pointer\n");

	retval = dptx_read_dpcd(dptx, DP_TEST_PHY_PATTERN, &pattern);
	if (retval)
		return retval;

	pattern &= DPTX_PHY_TEST_PATTERN_SEL_MASK;

	switch (pattern) {
	case DPTX_NO_TEST_PATTERN_SEL:
		HISI_DRM_INFO("[DP] DPTX_NO_TEST_PATTERN_SEL\n");
		phy_pattern = DPTX_PHYIF_CTRL_TPS_NONE;
		break;
	case DPTX_D102_WITHOUT_SCRAMBLING:
		HISI_DRM_INFO("[DP] DPTX_D102_WITHOUT_SCRAMBLING\n");
		phy_pattern = DPTX_PHYIF_CTRL_TPS_1;
		break;
	case DPTX_SYMBOL_ERROR_MEASUREMENT_COUNT:
		HISI_DRM_INFO("[DP] DPTX_SYMBOL_ERROR_MEASUREMENT_COUNT\n");
		phy_pattern = DPTX_PHYIF_CTRL_TPS_SYM_ERM;
		break;
	case DPTX_TEST_PATTERN_PRBS7:
		HISI_DRM_INFO("[DP] DPTX_TEST_PATTERN_PRBS7\n");
		phy_pattern = DPTX_PHYIF_CTRL_TPS_PRBS7;
		break;
	case DPTX_80BIT_CUSTOM_PATTERN_TRANS:
		HISI_DRM_INFO("[DP] DPTX_80BIT_CUSTOM_PATTERN_TRANS\n");
		dptx_writel(dptx, DPTX_CUSTOMPAT0, 0x3e0f83e0);
		dptx_writel(dptx, DPTX_CUSTOMPAT1, 0x3e0f83e0);
		dptx_writel(dptx, DPTX_CUSTOMPAT2, 0xf83e0);
		phy_pattern = DPTX_PHYIF_CTRL_TPS_CUSTOM80;
		break;
	case DPTX_CP2520_HBR2_COMPLIANCE_EYE_PATTERN:
		HISI_DRM_INFO("[DP] DPTX_CP2520_HBR2_COMPLIANCE_EYE_PATTERN\n");
		phy_pattern = DPTX_PHYIF_CTRL_TPS_CP2520_1;
		break;
	case DPTX_CP2520_HBR2_COMPLIANCE_PATTERN_2:
		HISI_DRM_INFO("[DP] DPTX_CP2520_HBR2_COMPLIANCE_EYE_PATTERN\n");
		phy_pattern = DPTX_PHYIF_CTRL_TPS_CP2520_2;
		break;
	case DPTX_CP2520_HBR2_COMPLIANCE_PATTERN_3:
		HISI_DRM_INFO("[DP] DPTX_CP2520_HBR2_COMPLIANCE_EYE_PATTERN\n");
		phy_pattern = DPTX_PHYIF_CTRL_TPS_4;
		break;
	default:
		HISI_DRM_INFO("[DP] Invalid TEST_PATTERN\n");
		return -EINVAL;
	}
	if (dptx->dptx_phy_set_pattern)
		dptx->dptx_phy_set_pattern(dptx, phy_pattern);

	return 0;
}

irqreturn_t dptx_threaded_irq(int irq, void *dev)
{
	int retval = 0;
	struct hisi_dp_device *dp_dev = NULL;
	struct dp_ctrl *dptx = NULL;
	u32 hpdsts;
	struct drm_connector *connector = NULL;
	enum drm_connector_status old_status;
	enum drm_connector_status new_status;

	if (dev == NULL) {
		HISI_DRM_ERR("[DP] dp_dev is NULL!\n");
		return IRQ_HANDLED;
	}

	dp_dev = dev;

	dptx = &(dp_dev->dp);
	connector = &dptx->dp_dev->connector;

	HISI_DRM_DEBUG("[DP] +");

	mutex_lock(&dptx->dptx_mutex);
	if (!dptx->dptx_enable) {
		HISI_DRM_WARN("[DP] dptx has already off!\n");
		mutex_unlock(&dptx->dptx_mutex);
		return IRQ_HANDLED;
	}

	/*
	 * this should be set after all AUX transactions that are
	 * queued are aborted. Currently we don't queue AUX and AUX is
	 * only started from this function.
	 */
	atomic_set(&dptx->aux.abort, 0);
	if (atomic_read(&dptx->c_connect)) {
		atomic_set(&dptx->c_connect, 0);

		hpdsts = dptx_readl(dptx, DPTX_HPDSTS);
		HISI_DRM_INFO("[DP] HPDSTS = 0x%08x.\n", hpdsts);

		if (hpdsts & DPTX_HPDSTS_STATUS) {
			if (dptx->handle_hotplug)
				retval = dptx->handle_hotplug(dp_dev);
			dp_imonitor_set_param(DP_PARAM_HOTPLUG_RETVAL, &retval);
			if (retval)
				HISI_DRM_ERR("[DP] DP Device Hotplug error %d\n", retval);
		} else {
			if (dptx->handle_hotunplug)
				retval = dptx->handle_hotunplug(dp_dev);
			if (retval)
				HISI_DRM_ERR("[DP] DP Device Hotplug error %d\n", retval);
		}
	}

	mutex_unlock(&dptx->dptx_mutex);

	old_status = connector->status;
	new_status = dptx->video_transfer_enable ?
				connector_status_connected :
				connector_status_disconnected;
	HISI_DRM_INFO("[DP] connector old_status %d, cur_status %d",
		      old_status, new_status);
	if (old_status != new_status)
		drm_kms_helper_hotplug_event(dptx->dp_dev->drm_dev);

	HISI_DRM_DEBUG("[DP] -");

	return IRQ_HANDLED;
}

void dptx_hpd_handler(struct dp_ctrl *dptx, bool plugin, uint8_t dp_lanes)
{
	u32 reg;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	HISI_DRM_INFO("[DP] DP Plug Type:%d, Lanes:%d\n", plugin, dp_lanes);

	/* need to check dp lanes */
	dptx->max_lanes = dp_lanes;

	reg = dptx_readl(dptx, DPTX_CCTL);
	if (plugin)
		reg |= DPTX_CCTL_FORCE_HPD;
	else
		reg &= ~DPTX_CCTL_FORCE_HPD;

	dptx_writel(dptx, DPTX_CCTL, reg);
}

void dptx_hpd_irq_handler(struct dp_ctrl *dptx)
{
	int retval;

	drm_check_and_void_return((dptx == NULL), "[DP] NULL Pointer\n");

	HISI_DRM_INFO("[DP] DP Shot Plug!\n");

	if (!dptx->video_transfer_enable) {
		HISI_DRM_ERR("[DP] DP link train is useless!\n");
		return;
	}

	dptx_notify(dptx);
	retval = handle_sink_request(dptx);
	if (retval)
		HISI_DRM_ERR("[DP] Unable to handle sink request %d\n", retval);
}

static int dptx_hpd_irq(struct dp_ctrl *dptx, u32 ists, u32 hpdsts)
{
	int retval;

	if (ists & DPTX_ISTS_HPD) {
		if (hpdsts & DPTX_HPDSTS_IRQ) {
			dptx_writel(dptx, DPTX_HPDSTS, DPTX_HPDSTS_IRQ);
			atomic_set(&dptx->sink_request, 1);
			dptx_notify(dptx);
			retval = IRQ_WAKE_THREAD;
		}

		if (hpdsts & DPTX_HPDSTS_HOT_PLUG) {
			dptx_writel(dptx, DPTX_HPDSTS, DPTX_HPDSTS_HOT_PLUG);

			atomic_set(&dptx->aux.abort, 1);
			atomic_set(&dptx->c_connect, 1);
			dptx_notify(dptx);
			retval = IRQ_WAKE_THREAD;
		}

		if (hpdsts & DPTX_HPDSTS_HOT_UNPLUG) {
			dptx_writel(dptx, DPTX_HPDSTS, DPTX_HPDSTS_HOT_UNPLUG);
			atomic_set(&dptx->aux.abort, 1);
			atomic_set(&dptx->c_connect, 1);
			dptx_notify(dptx);
			retval = IRQ_WAKE_THREAD;
		}

		if (hpdsts & 0x80) {
			HISI_DRM_INFO("[DP] DPTX_HPDSTS[7] HOTPLUG DEBUG INTERRUPT!\n");
			dptx_writel(dptx, DPTX_HPDSTS, 0x80 | DPTX_HPDSTS_HOT_UNPLUG);
			retval = 0;
		}
		return retval;
	}
	return 0;
}

irqreturn_t dptx_irq(int irq, void *dev)
{
	irqreturn_t retval = IRQ_HANDLED;
	struct hisi_dp_device *dp_dev = NULL;
	struct dp_ctrl *dptx = NULL;
	u32 ists;
	u32 ien;
	u32 hpdsts;

	if (dev == NULL) {
		HISI_DRM_ERR("[DP] dp_dev is NULL!\n");
		return IRQ_HANDLED;
	}

	dp_dev = (struct hisi_dp_device *)dev;

	dptx = &(dp_dev->dp);

	ists = dptx_readl(dptx, DPTX_ISTS);
	ien = dptx_readl(dptx, DPTX_IEN);
	hpdsts = dptx_readl(dptx, DPTX_HPDSTS);
	HISI_DRM_DEBUG("[DP] ISTS=0x%08x, IEN=0x%08x, HPDSTS=0x%08x.\n",
		ists, ien, hpdsts);

	if (!(ists & DPTX_ISTS_ALL_INTR)) {
		HISI_DRM_INFO("[DP] IRQ_NONE, DPTX_ISTS=0x%08x.\n", ists);
		retval = IRQ_NONE;
		return retval;
	}

	if (ists & DPTX_ISTS_HDCP) {
#ifdef CONFIG_DP_HDCP_ENABLE
		handle_hdcp_intr(dptx);
		dptx_writel(dptx, DPTX_ISTS, DPTX_ISTS_HDCP);
#endif
	}

	/* Handle and clear */
	if (ists & DPTX_ISTS_SDP)
		HISI_DRM_DEBUG("[DP] DPTX_ISTS_SDP!");


	if (ists & DPTX_ISTS_AUDIO_FIFO_OVERFLOW) {
		if (ien & DPTX_IEN_AUDIO_FIFO_OVERFLOW) {
			HISI_DRM_INFO("[DP] DPTX_ISTS_AUDIO_FIFO_OVERFLOW!\n");
			dptx_writel(dptx, DPTX_ISTS, DPTX_ISTS_AUDIO_FIFO_OVERFLOW);
		}
	}

	if (ists & DPTX_ISTS_VIDEO_FIFO_OVERFLOW) {
		if (ien & DPTX_IEN_VIDEO_FIFO_OVERFLOW) {
			HISI_DRM_ERR("[DP] DPTX_ISTS_VIDEO_FIFO_OVERFLOW!\n");
			dptx_writel(dptx, DPTX_ISTS, DPTX_ISTS_VIDEO_FIFO_OVERFLOW);
		}
	}

	if (ists & DPTX_ISTS_TYPE_C) {
		/* Handle and clear */
		dptx_writel(dptx, DPTX_TYPE_C_CTRL, DPTX_TYPEC_INTERRUPT_STATUS);

		HISI_DRM_DEBUG("\n [DP] DPTX_TYPE_C_CTRL: [%02x] PRE", dptx_readl(dptx, DPTX_TYPE_C_CTRL));
		dptx_typec_reset_ack(dptx);
		HISI_DRM_DEBUG("\n [DP] DPTX_TYPE_C_CTRL: [%02x] AFT", dptx_readl(dptx, DPTX_TYPE_C_CTRL));
	}

	retval = dptx_hpd_irq(dptx, ists, hpdsts);
	return retval;
}
/*lint -restore*/
