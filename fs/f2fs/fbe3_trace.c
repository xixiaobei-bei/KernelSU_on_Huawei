/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: big data trace for fbe3
 * Author: LAI Xinyi
 * Create: 2020-09-01
 */

#include "fbe3_trace.h"
#include <log/imonitor.h>

static uint32_t get_gen_meta_data(const struct fbe3_metadata_stat *big_data,
				  struct imonitor_eventobj *obj)
{
	uint32_t stats = 0;

	if (!big_data || !obj) {
		pr_err("%s: param is invalid!", __func__);
		return EINVAL;
	}

	stats |= (uint32_t)imonitor_set_param_integer_v2(obj, "FBE3_Enable",
							 big_data->fbe3_enable);
	stats |= (uint32_t)imonitor_set_param_integer_v2(obj, "MSP_Enable",
							 big_data->msp_enable);
	stats |= (uint32_t)imonitor_set_param_integer_v2(obj, "Scene_Type",
							 big_data->scene_type);
	stats |= (uint32_t)imonitor_set_param_integer_v2(obj, "Result",
							 big_data->result);
	stats |= (uint32_t)imonitor_set_param_integer_v2(obj, "Hisi_delay",
							 big_data->hisi_delay);
	stats |= (uint32_t)imonitor_set_param_integer_v2(obj, "Profile_ID",
							 big_data->user_id);
	stats |= (uint32_t)imonitor_set_param_integer_v2(
		obj, "Hisi_Error_Code", big_data->hisi_err_code);
	stats |= (uint32_t)imonitor_set_param_integer_v2(
		obj, "Vold_Error_Code", big_data->vold_err_code);

	return stats;
}

static void upload_gen_meta_data(const struct fbe3_metadata_stat *big_data,
				 uint64_t errno)
{
	struct imonitor_eventobj *obj = NULL;
	uint32_t stats;
	int err;

	if (!big_data) {
		pr_err("%s: invalid big data!\n", __func__);
		return;
	}

	obj = imonitor_create_eventobj(errno);
	if (!obj) {
		pr_err("%s: get event obj failed!\n", __func__);
		return;
	}

	stats = get_gen_meta_data(big_data, obj);
	if (stats != 0) {
		pr_err("%s: set param failed!\n", __func__);
		imonitor_destroy_eventobj(obj);
		return;
	}
	if ((err = imonitor_send_event(obj)) < 0)
		pr_err("%s: data send failed! err=%d\n", __func__, err);

	imonitor_destroy_eventobj(obj);
	return;
}

void gen_meta_upload_bigdata(int type, int ret, uint64_t hisi_delay)
{
	struct fbe3_metadata_stat stat_data = { 0 };
	uint64_t errno = 0;

	if (type == ECE_GEN_METADATA)
		errno = ECE_BIG_DATA_UPLOAD_CODE;
	else if (type == SECE_GEN_METADATA)
		errno = SECE_BIG_DATA_UPLOAD_CODE;

	/* fill context of big data */
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V3
	stat_data.fbe3_enable = 1;
#endif
	stat_data.msp_enable = 0;
	stat_data.scene_type = type;
	stat_data.result = ret ? GEN_METADATA_FAIL : GEN_METADATA_SUC;
	stat_data.hisi_delay = ret ? hisi_delay : 0;
	stat_data.user_id = USER_ID;
	stat_data.hisi_err_code = ret;
	stat_data.vold_err_code = 0;

	upload_gen_meta_data(&stat_data, errno);

	return;
}
