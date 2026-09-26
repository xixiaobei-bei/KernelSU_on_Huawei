#ifndef HISI_DRM_DSS_RDMA_CONFIGURE_H_INCLUDED
#define HISI_DRM_DSS_RDMA_CONFIGURE_H_INCLUDED

int hisi_dss_rdma_config(struct dss_module_reg *dss_module,
	struct hisi_drm_plane_state *hisi_state, int chn_idx, int ovl_idx,
	struct dss_rect_ltrb *clip_rect, struct dss_rect *aligned_rect,
	bool *rdma_stretch_enable);

#endif /*HISI_DRM_DSS_RDMA_CONFIGURE_H_INCLUDED*/

/********************************* END ***********************************************/

