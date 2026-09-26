#ifndef HISI_DRM_DSS_GLOBAL_H_INCLUDED
#define HISI_DRM_DSS_GLOBAL_H_INCLUDED

#include "hisi_drm_dpe_utils.h"

/* number of smrx idx for each channel */
extern const uint32_t g_dss_chn_sid_num[DSS_CHN_MAX_DEFINE];

/* start idx of each channel */
/* smrx_idx = g_dss_smmu_smrx_idx[chn_idx] + (0 ~ g_dss_chn_sid_num[chn_idx]) */
extern const uint32_t g_dss_smmu_smrx_idx[DSS_CHN_MAX_DEFINE];

extern const uint32_t g_dss_module_ovl_base[DSS_MCTL_IDX_MAX][MODULE_OVL_MAX];

extern const uint32_t g_dss_module_base[DSS_CHN_MAX_DEFINE][MODULE_CHN_MAX];

bool check_yuv_planar(uint32_t format);

bool check_yuv_semi_planar(uint32_t format);

#endif /*HISI_DRM_DSS_GLOBAL_H_INCLUDED*/

/********************************* END ***********************************************/

