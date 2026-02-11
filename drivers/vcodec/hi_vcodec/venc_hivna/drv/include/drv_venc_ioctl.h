
#ifndef __DRV_VENC_IOCTL_H__
#define __DRV_VENC_IOCTL_H__

#include "soc_venc_reg_interface.h"
#include "hi_type.h"

#define MAX_SLICE_NUM  (16)

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
	HI_S32 shared_fd;
	HI_U32 iova;
	HI_U64 vir_addr;
	HI_U32 iova_size;
} venc_buffer_record_t;

typedef struct {
	HI_S32   interal_shared_fd;
	HI_S32   image_shared_fd;
	HI_S32   stream_shared_fd[MAX_SLICE_NUM];
	HI_S32   stream_head_shared_fd;
} venc_fd_info_t;

typedef struct {
	HI_U32 rec_luma_size;
	HI_U32 rec_chroma_size;
	HI_U32 rec_luma_head_size;
	HI_U32 rec_chroma_head_size;
	HI_U32 qpgld_size;
	HI_U32 nbi_size;
	HI_U32 pme_size;
	HI_U32 pme_info_size;
	HI_U32 vedu_src_y_length;
	HI_U32 vedu_src_c_length;
	HI_U32 vedu_src_v_length;
	HI_U32 vedu_src_yh_length;
	HI_U32 vedu_src_ch_length;
	UADDR master_stream_start;
	UADDR master_stream_end;
} venc_buffer_alloc_info_t;


struct channel_info {
	HI_U32 id;          // channel id of this frame
	HI_U32 frame_no;     // the frame number of this channel
	HI_U32 frame_type;   // the frame type of this frame
};

enum block_size_type {
	BLOCK_4X4,
	BLOCK_8X8,
	BLOCK_16x16,
	BLOCK_32x32,
	BLOCK_BUTT
};

struct stream_info {
	HI_BOOL is_buf_full;
	HI_BOOL is_eop;
	HI_U32 pic_stream_size;
	HI_U32 block_cnt[BLOCK_BUTT];
	HI_U32 ave_madi;
	HI_U32 frame_madp;
	HI_U32 slice_len[MAX_SLICE_NUM];
	HI_U32 slice_num;
};

struct clock_info {
	HI_BOOL is_set_clock;
	venc_clk_t clock_type;
	HI_U32 core_num;
};

struct encode_done_info {
	HI_BOOL is_timeout;
	struct channel_info channel_info;   // the channel info of this frame
	struct stream_info stream_info;  // the register info for encode done process
};

struct encode_info {
	HI_BOOL is_block;            // true: sync mode(waiting for encoding done); false: async mode
	HI_BOOL is_reset;            // whether reset the hardware
	struct channel_info channel; // the channel info of this frame
	enum reg_cfg_mode reg_cfg_mode;
	S_HEVC_AVC_REGS_TYPE_CFG all_reg;
	venc_fd_info_t mem_info;
	venc_buffer_alloc_info_t venc_inter_buffer;
	struct clock_info clock_info;
	struct encode_done_info encode_done_info;  // the read back register info
};

#define CMD_VENC_ENCODE                 _IOWR(IOC_TYPE_VENC, 0x32, struct encode_info)
#define CMD_VENC_GET_ENCODE_DONE_INFO   _IOR(IOC_TYPE_VENC, 0x36, struct encode_done_info)

#define CMD_VENC_IOMMU_MAP              _IOWR(IOC_TYPE_VENC, 0x33, venc_buffer_record_t)
#define CMD_VENC_IOMMU_UNMAP            _IOW(IOC_TYPE_VENC, 0x34, venc_buffer_record_t)

#endif //__HI_DRV_VENC_H__

