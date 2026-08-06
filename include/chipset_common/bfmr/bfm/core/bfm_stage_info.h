/*
 * copyright: Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * file: bfm_trace_boottime.h
 *
 * define the trace_boottime's external public enum/macros/interface for BFM
 * (Boot Fail Monitor)
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef BFM_TRACE_STAGETIME_H
#define BFM_TRACE_STAGETIME_H

#include "chipset_common/bfmr/public/bfmr_public.h"
#include "chipset_common/bfmr/common/bfmr_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BMFR_MAX_BOOTUP_STAGE_COUNT 256
#define STAGE_INFO_RESERVED 32

struct bfm_stage_info {
	enum bfmr_detail_stage stage;
	unsigned int start_time; /* seconds */
	char reserved[STAGE_INFO_RESERVED];
};

struct bfm_stages {
	unsigned int stage_count;
	struct bfm_stage_info stage_info_list[BMFR_MAX_BOOTUP_STAGE_COUNT];
};

/*
 * @brief: record the stage information into list
 *
 * @param: pStage[in], the stageinfo pointer
 */
void bfm_add_stage_info(struct bfm_stage_info *pstage);

/*
 * @brief: append the stages info into bs_data
 *
 * @param: buffer[inout], the buffer to record information
 * @param: buffer_len[in], the size of buffer
 *
 * @return:the length to need print
 */
size_t bfm_save_stages_info_txt(char *buffer, unsigned int buffer_len);

/*
 * @brief: init trace time module resource
 *
 * @return: 0 - succeeded; -1 - failed.
 *
 * @note: it need be initialized in kernel.
 */
int bfm_init_stage_info(void);

/*
 * @brief: release trace time module resource
 *
 * @note: it need be called after bootsuccess & save log.
 */
void bfm_deinit_stage_info(void);

#ifdef __cplusplus
}
#endif

#endif

