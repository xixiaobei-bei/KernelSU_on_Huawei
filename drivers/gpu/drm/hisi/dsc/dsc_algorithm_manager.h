
#ifndef __DPTX_DP_DSC_MANAGER_H__
#define __DPTX_DP_DSC_MANAGER_H__
#include "dsc_algorithm.h"

struct dsc_algorithm_manager {
	void (*vesa_dsc_info_calc)(const struct input_dsc_info *input_para,
		struct dsc_info *output_para, struct dsc_info *exist_para);
};

struct dsc_algorithm_manager *get_dsc_algorithm_manager_instance(void);
#endif
