

#ifndef __CFG_TABLE_VBK_CS_H__
#define __CFG_TABLE_VBK_CS_H__

#define SIGMA2ALPHA_NUM (256)

typedef struct _vbk_ctrl_cfg_t {
	unsigned int to_use;

	unsigned int  op_mode;
	unsigned int  y_ds16_gauss_en;
	unsigned int  uv_ds16_gauss_en;
	unsigned int  sigma_gauss_en;
	unsigned int  sigma2alpha_en;
	unsigned int  in_width_ds4;
	unsigned int  in_height_ds4;
} vbk_ctrl_cfg_t;

typedef struct _vbk_y_sigma_gauss_coeff_t {
	unsigned int to_use;

	unsigned int g00;
	unsigned int g01;
	unsigned int g02;
	unsigned int g03;
	unsigned int g04;
	unsigned int inv;
} vbk_yds16_sigma_gauss_coeff_t;

typedef struct _vbk_uv_gauss_coeff_t {
	unsigned int to_use;

	unsigned int g00;
	unsigned int g01;
	unsigned int g02;
	unsigned int inv;
} vbk_uvds16_gauss_coeff_t;

typedef struct _vbk_addnoise_t {
	unsigned int to_use;

	unsigned int addnoise_th;
} vbk_addnoise_t;

typedef struct _vbk_sigma2alpha_t {
	unsigned int to_use;

	unsigned int sigma2alpha[SIGMA2ALPHA_NUM];
} vbk_sigma2alpha_t;

typedef struct _vbk_foremask_cfg_t {
	unsigned int to_use;

	unsigned int foremask_coeff;
	unsigned int foremask_th;
	unsigned int foremask_weighted_filter_en;
	unsigned int foremask_dilation_radius;
} vbk_foremask_cfg_t;

typedef struct _vbk_config_table_t {
	unsigned int to_use;

	vbk_ctrl_cfg_t vbk_ctrl;
	vbk_yds16_sigma_gauss_coeff_t yds16_gauss_coeff;
	vbk_yds16_sigma_gauss_coeff_t sigma_gauss_coeff;
	vbk_uvds16_gauss_coeff_t  uvds16_gauss_coeff;
	vbk_addnoise_t addnoise;
	vbk_sigma2alpha_t sigma2alpha;
	vbk_foremask_cfg_t  foremask_cfg;
} vbk_config_table_t;

#endif/*__CFG_TABLE_VBK_CS_H__*/

