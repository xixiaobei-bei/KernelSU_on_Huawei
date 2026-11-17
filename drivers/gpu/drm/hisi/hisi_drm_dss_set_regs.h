#ifndef HISI_DRM_DSS_SET_REGS_H_INCLUDED
#define HISI_DRM_DSS_SET_REGS_H_INCLUDED

void hisi_dss_aif_ch_set_reg(struct hisi_drm_crtc *hisi_crtc, char __iomem *aif_ch_base,
	dss_aif_t *s_aif);

int hisi_dss_ch_module_set_regs(struct hisi_drm_crtc *hisi_crtc, uint32_t chn_idx);

void hisi_dss_disable_channel(struct hisi_drm_crtc *hisi_crtc, int chn_idx);
#endif /*HISI_DRM_DSS_SET_REGS_H_INCLUDED*/

/********************************* END ***********************************************/
