/*lint -e529 -e578*/
#include <drm/drmP.h>

#include "drm_mipi_dsi.h"
#include "hisi_drm_dpe_utils.h"
#include "hisi_panel.h"

#include "hisi_drm_debug.h"

void set_reg(char __iomem *addr, uint32_t val, uint8_t bw, uint8_t bs)
{
	uint32_t mask = (1UL << bw) - 1UL;
	uint32_t tmp;

	tmp = inp32(addr);
	tmp &= ~(mask << bs);

	outp32(addr, tmp | ((val & mask) << bs));

	HISI_DRM_PRINT_REG("writel: [%pK] = 0x%x\n",
		addr, tmp | ((val & mask) << bs));
}

uint32_t set_bits32(uint32_t old_val, uint32_t val, uint8_t bw, uint8_t bs)
{
	uint32_t mask = (1UL << bw) - 1UL;
	uint32_t tmp;

	tmp = old_val;
	tmp &= ~(mask << bs);

	return (tmp | ((val & mask) << bs));
}

void hisi_set_reg(struct hisi_drm_crtc *hisi_crtc,
	char __iomem *addr, uint32_t val, uint8_t bw, uint8_t bs)
{
	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is null\n");
		return;
	}

	if (hisi_crtc->enable_cmdlist)
		hisi_cmdlist_set_reg(hisi_crtc, addr, val, bw, bs);
	else
		set_reg(addr, val, bw, bs);
}

void hisi_drm_get_timestamp(struct timeval *tv)
{
	struct timespec ts;

	if (!tv) {
		HISI_DRM_ERR("tv is null");
		return;
	}

	ktime_get_ts(&ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
}

uint32_t hisi_drm_timestamp_diff(
	struct timeval *lasttime,
	struct timeval *curtime)
{
	uint32_t ret;

	if (!lasttime || !curtime) {
		HISI_DRM_ERR("lasttime or curtime is nullptr!");
		return false;
	}

	ret = (curtime->tv_usec >= lasttime->tv_usec) ?
		curtime->tv_usec - lasttime->tv_usec :
		1000000 - (lasttime->tv_usec - curtime->tv_usec);

	return ret;
}

bool is_mipi_cmd_mode(struct hisi_drm_crtc *hisi_crtc)
{
	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is nullptr!");
		return false;
	}

	if (hisi_crtc->encoder_type & (PANEL_MIPI_CMD |
		PANEL_DUAL_MIPI_CMD)) {
		return true;
	}

	return false;
}

bool is_mipi_video_mode(struct hisi_drm_crtc *hisi_crtc)
{
	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is nullptr!");
		return false;
	}

	if (hisi_crtc->encoder_type & (PANEL_MIPI_VIDEO |
		PANEL_DUAL_MIPI_VIDEO | PANEL_RGB2MIPI)) {
		return true;
	}

	return false;
}

bool is_mipi_mode(struct hisi_drm_crtc *hisi_crtc)
{
	return (is_mipi_cmd_mode(hisi_crtc) ||
		is_mipi_video_mode(hisi_crtc));
}

bool is_dual_mipi_mode(struct hisi_drm_crtc *hisi_crtc)
{
	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is nullptr!");
		return false;
	}

	if (hisi_crtc->encoder_type & (PANEL_DUAL_MIPI_VIDEO |
		PANEL_DUAL_MIPI_CMD)) {
		return true;
	}

	return false;
}

bool is_dp(struct hisi_drm_crtc *hisi_crtc)
{
	if (!hisi_crtc) {
		HISI_DRM_ERR("hisi_crtc is nullptr!");
		return false;
	}

	if (hisi_crtc->encoder_type & PANEL_DP)
		return true;

	return false;
}

/*lint +e529 +e578*/
