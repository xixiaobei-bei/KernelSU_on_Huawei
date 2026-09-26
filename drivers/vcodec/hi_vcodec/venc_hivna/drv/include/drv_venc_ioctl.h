/*
 * drv_venc_ioctl.h
 *
 * This is for drv_venc_ioctl.
 *
 * Copyright (c) 2010-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __DRV_VENC_IOCTL_H__
#define __DRV_VENC_IOCTL_H__

#include "soc_venc_reg_interface.h"
#include "hi_type.h"

#define MAX_SLICE_NUM  16
#define QP_HISTOGRAM_NUM 52
#define CMDLIST_BUFFER_NUM 32

enum reg_cfg_mode {
	VENC_SET_CFGREG = 100,
	VENC_SET_CFGREGSIMPLE
};

typedef enum {
#ifdef HIVCODECV500
	VENC_CLK_RATE_LOWER = 0,
#endif
	VENC_CLK_RATE_LOW,
	VENC_CLK_RATE_NORMAL,
	VENC_CLK_RATE_HIGH,
	VENC_CLK_BUTT,
} venc_clk_t;

typedef struct {
	int32_t shared_fd;
	uint32_t iova;
	uint64_t virt_addr;
	uint32_t iova_size;
} venc_buffer_record_t;

typedef struct {
	int32_t   interal_shared_fd;
	int32_t   image_shared_fd;
	int32_t   stream_shared_fd[MAX_SLICE_NUM];
	int32_t   stream_head_shared_fd;
} venc_fd_info_t;

typedef struct {
	uint32_t rec_luma_size;
	uint32_t rec_chroma_size;
	uint32_t rec_luma_head_size;
	uint32_t rec_chroma_head_size;
	uint32_t qpgld_size;
	uint32_t nbi_size;
	uint32_t pme_size;
	uint32_t pme_info_size;
	uint32_t vedu_src_y_length;
	uint32_t vedu_src_c_length;
	uint32_t vedu_src_v_length;
	uint32_t vedu_src_yh_length;
	uint32_t vedu_src_ch_length;
	UADDR master_stream_start;
	UADDR master_stream_end;
} venc_buffer_alloc_info_t;


struct channel_info {
	uint32_t id;          // channel id of this frame
	uint32_t frame_number;     // the frame number of this channel
	uint32_t frame_type;   // the frame type of this frame
	uint32_t buf_index;   // the buffer index of this frame
};

enum block_size_type {
	BLOCK_4X4,
	BLOCK_8X8,
	BLOCK_16X16,
	BLOCK_32X32,
	BLOCK_BUTT
};

struct stream_info {
	bool is_buf_full;
	bool is_eop;
	uint32_t pic_stream_size;
	uint32_t block_cnt[BLOCK_BUTT];
	uint32_t ave_madi;
	uint32_t frame_madp;
	uint32_t slice_len[MAX_SLICE_NUM];
	uint32_t aligned_slice_len[MAX_SLICE_NUM];
	uint32_t slice_num;
	uint32_t luma_qp_cnt[QP_HISTOGRAM_NUM];
	uint32_t average_luma_qp;
	uint32_t average_lcu_mse;
};

struct clock_info {
	bool is_set_clock;
	venc_clk_t clock_type;
	uint32_t core_num;
};

struct encode_done_info {
	bool is_timeout;
	struct channel_info channel_info;   // the channel info of this frame
	struct stream_info stream_info;  // the register info for encode done process
};

struct encode_info {
	bool is_block;            // true: sync mode(waiting for encoding done); false: async mode
	bool is_reset;            // whether reset the hardware
	bool is_protected;
	struct channel_info channel; // the channel info of this frame
	enum reg_cfg_mode reg_cfg_mode;
	S_HEVC_AVC_REGS_TYPE_CFG all_reg;
	venc_fd_info_t mem_info;
	venc_buffer_alloc_info_t venc_inter_buffer;
	struct clock_info clock_info;
	struct encode_done_info encode_done_info;  // the read back register info
	bool enable_power_control_per_frame;
};

#define CMD_VENC_ENCODE                 _IOWR(IOC_TYPE_VENC, 0x32, struct encode_info)
#define CMD_VENC_GET_ENCODE_DONE_INFO   _IOR(IOC_TYPE_VENC, 0x36, struct encode_done_info)

#define CMD_VENC_IOMMU_MAP              _IOWR(IOC_TYPE_VENC, 0x33, venc_buffer_record_t)
#define CMD_VENC_IOMMU_UNMAP            _IOW(IOC_TYPE_VENC, 0x34, venc_buffer_record_t)

#endif

