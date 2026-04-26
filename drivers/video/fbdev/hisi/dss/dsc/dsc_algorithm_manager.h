/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: dsc algorithm manager
 * Author: yangbr
 * Create: 2020-01-22
 * Notes: null
 */
#ifndef __DPTX_DSC_MANAGER_H__
#define __DPTX_DSC_MANAGER_H__
#include "dsc_algorithm.h"

typedef struct dsc_algorithm_manager {
	void (*vesa_dsc_info_calc)(const struct input_dsc_info *input_para,
		struct dsc_info *output_para, struct dsc_info *exist_para);
} dsc_algorithm_manager_t;

dsc_algorithm_manager_t *get_dsc_algorithm_manager_instance(void);

#endif
