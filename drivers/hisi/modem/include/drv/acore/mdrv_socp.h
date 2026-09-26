/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * @brief   socp模块在ACORE上的对外头文件
 * @file    mdrv_socp.h
 * @author  yaningbo
 * @version v1.0
 * @date    2019.11.29
 * @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|2012.11.29|创建文件</li></ul>
 * <ul><li>v2.0|2019.11.29|接口自动化</li></ul>
 * @since
 */

#ifndef __MDRV_ACORE_SOCP_H__
#define __MDRV_ACORE_SOCP_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief socp coder source channel type
 */
#define SOCP_CODER_SRC_CHAN 0x00
/**
 * @brief socp coder dest channel type
 */
#define SOCP_CODER_DEST_CHAN 0x01
/**
 * @brief socp decoder source channel type
 */
#define SOCP_DECODER_SRC_CHAN 0x02
/**
 * @brief socp decoder dest channel type
 */
#define SOCP_DECODER_DEST_CHAN 0x03

/**
 * @brief socp source channel packet max length
 */
#define SOCP_CODER_SRC_PKT_LEN_MAX (4 * 1024)

/**
 * @brief socp channel type define
 */
#define SOCP_CHAN_DEF(chan_type, chan_id) ((chan_type << 16) | chan_id)
/**
 * @brief get socp channel id by socp channel
 */
#define SOCP_REAL_CHAN_ID(unique_chan_id) (unique_chan_id & 0xFFFF)
/**
 * @brief get socp channel type by socp channel
 */
#define SOCP_REAL_CHAN_TYPE(unique_chan_id) (unique_chan_id >> 16)

/**
 * @brief socp dest channel dsm state:enable
 */
#define SOCP_DEST_DSM_ENABLE 0x01
/**
 * @brief socp dest channel dsm state:disable
 */
#define SOCP_DEST_DSM_DISABLE 0x00


typedef enum {
    SOCP_BD_DATA = 0, /**< data type  */
    SOCP_BD_LIST = 1, /**< list type */
    SOCP_BD_BUTT      /**< hdlc flag butt */
} socp_bd_type_e;

/**
 * @brief socp hdlc state
 */
typedef enum {
    SOCP_HDLC_ENABLE = 0,  /**< socp hdlc enable */
    SOCP_HDLC_DISABLE = 1, /**< socp hdlc disable */
    SOCP_HDLC_FLAG_BUTT
} socp_hdlc_flag_e;
/**
 * @brief socp state
 */
typedef enum {
    SOCP_IDLE = 0,    /**< socp is in idle */
    SOCP_BUSY,        /**< socp is busy */
    SOCP_UNKNOWN_BUTT /**< socp is in unknown state */
} socp_state_e;


/* 编码目的通道ID枚举定义 */
typedef enum {
    SOCP_CODER_DST_OM_CNF = SOCP_CHAN_DEF(SOCP_CODER_DEST_CHAN, 0),    /**< diag cnf channel */
    SOCP_CODER_DST_OM_IND = SOCP_CHAN_DEF(SOCP_CODER_DEST_CHAN, 1),    /**< diag ind channel */
    SOCP_CODER_DST_DUMP = SOCP_CHAN_DEF(SOCP_CODER_DEST_CHAN, 2),      /**< secure dump channel */
    SOCP_CODER_DST_LOGSERVER = SOCP_CHAN_DEF(SOCP_CODER_DEST_CHAN, 3), /**< log server channel */
    SOCP_CODER_DST_BUTT
} socp_coder_dst_e;

/**
 * @brief socp event state
 */
typedef enum {
    SOCP_EVENT_PKT_HEADER_ERROR = 0x1,               /**< socp packet header is error */
    SOCP_EVENT_OUTBUFFER_OVERFLOW = 0x2,             /**< socp dest channel is overflow */
    SOCP_EVENT_RDBUFFER_OVERFLOW = 0x4,              /**< socp dest channel rd buffer is overflow */
    SOCP_EVENT_DECODER_UNDERFLOW = 0x8,              /**< socp deccode source channel is underflow */
    SOCP_EVENT_PKT_LENGTH_ERROR = 0x10,              /**< socp deccode packet length error */
    SOCP_EVENT_CRC_ERROR = 0x20,                     /**< socp deccode packet crc is error */
    SOCP_EVENT_DATA_TYPE_ERROR = 0x40,               /**< socp deccode data type is error */
    SOCP_EVENT_HDLC_HEADER_ERROR = 0x80,             /**< socp deccode hdlc header is error */
    SOCP_EVENT_OUTBUFFER_THRESHOLD_OVERFLOW = 0x100, /**< socp dest channel is threshold overflow */
    SOCP_EVENT_BUTT
} socp_event_e;

/**
 * @brief socp coder source channel mode
 */
typedef enum {
    SOCP_ENCSRC_CHNMODE_CTSPACKET = 0, /**< coder source block ring buffer */
    SOCP_ENCSRC_CHNMODE_FIXPACKET,     /**< coder source fix length ring buffer */
    SOCP_ENCSRC_CHNMODE_LIST,          /**< coder source list ring buffer */
    SOCP_ENCSRC_CHNMODE_BUTT
} socp_encsrc_chnmode_e;
/**
 * @brief socp source channel priority
 */
typedef enum {
    SOCP_CHAN_PRIORITY_0 = 0, /**< lowest priority */
    SOCP_CHAN_PRIORITY_1,     /**< lower priority */
    SOCP_CHAN_PRIORITY_2,     /**< higher priority */
    SOCP_CHAN_PRIORITY_3,     /**< highest priority */
    SOCP_CHAN_PRIORITY_BUTT   /**< priority butt */
} socp_chan_priority_e;

/**
 * @brief socp data type
 */
typedef enum {
    SOCP_DATA_TYPE_0 = 0, /**< lte oam data type */
    SOCP_DATA_TYPE_1,     /**< gu oam data type */
    SOCP_DATA_TYPE_2,     /**< reserve */
    SOCP_DATA_TYPE_3,     /**< reserve */
    SOCP_DATA_TYPE_BUTT   /**< data type butt */
} socp_data_type_e;

/**
 * @brief socp source channel data type state
 */
typedef enum {
    SOCP_DATA_TYPE_EN = 0, /**< data type enable, default */
    SOCP_DATA_TYPE_DIS,    /**< data type disable */
    SOCP_DATA_TYPE_EN_BUTT /**< data type butt */
} socp_data_type_en_e;

/**
 * @brief socp source channel trans id state
 */
typedef enum {
    SOCP_TRANS_ID_DIS = 0, /**< trans id disable, default */
    SOCP_TRANS_ID_EN,      /**< trans id enable */
    SOCP_TRANS_ID_EN_BUTT
} socp_trans_id_en_e;

/**
 * @brief socp source channel pointer imgae state
 */
typedef enum {
    SOCP_PTR_IMG_DIS = 0, /**< pointer image disable, default */
    SOCP_PTR_IMG_EN,      /**< pointer image enable */
    SOCP_PTR_IMG_EN_BUTT  /**< pointer image butt */
} socp_ptr_img_en_e;

/**
 * @brief socp decoder source channel mode
 */
typedef enum {
    SOCP_DECSRC_CHNMODE_BYTES = 0, /**< decoder source bytes */
    SOCP_DECSRC_CHNMODE_LIST,      /**< decoder source list */
    SOCP_DECSRC_CHNMODE_BUTT       /**< decoder souce channel mode butt */
} socp_decsrc_chnmode_e;

/**
 * @brief socp source channel debug state
 */
typedef enum {
    SOCP_ENC_DEBUG_DIS = 0, /**< debug disable, default */
    SOCP_ENC_DEBUG_EN,      /**< debug enable */
    SOCP_ENC_DEBUG_EN_BUTT  /**< debug state butt */
} socp_enc_debug_en_e;

/**
 * @brief socp dest ind channel, log report mode
 */
typedef enum {
    SOCP_IND_MODE_DIRECT, /* *< direct report mode */
    SOCP_IND_MODE_DELAY,  /* *< delay report mode, buffers for buffer */
    SOCP_IND_MODE_CYCLE   /* *< cycle report mode, buffer override */
} socp_ind_mode_e;

/**
 * @brief deflate ind compress state
 */
typedef enum {
    DEFLATE_IND_NO_COMPRESS, /**< not compress */
    DEFLATE_IND_COMPRESS     /**< compress mode */
} deflate_ind_compress_e;

/**
 * @brief bbp data sample mode
 */
typedef enum {
    SOCP_BBP_DS_MODE_DROP = 0, /**< data drop mode */
    SOCP_BBP_DS_MODE_OVERRIDE, /**< data override mode */
    SOCP_BBP_DS_MODE_BUTT      /**< data sample mode butt */
} socp_bbp_ds_mode_e;

/**
 * @brief socp data type
 */
typedef enum {
    SCM_DATA_TYPE_TL = 0, /**< tl data type */
    SCM_DATA_TYPE_GU,     /**< gu data type */
    SCM_DATA_TYPE_BUTT    /**< data type butt */
} scm_data_type_e;


/**
 * @brief deflate compress enable state
 */
typedef enum {
    COMPRESS_DISABLE_STATE = 0, /**< compress disable */
    COMPRESS_ENABLE_STATE,      /**< compress enable */
} deflate_compress_state_e;

/**
 * @brief deflate dest channel buffer mode
 */
typedef enum {
    DEFLATE_DST_CHAN_NOT_CFG = 0, /**< not config */
    DEFLATE_DST_CHAN_DELAY,       /**< delay report mode, need more dest channel buffer */
    DEFLATE_DST_CHAN_DTS          /**< config dest channel buffer bye dts */
} deflate_dst_chan_cfg_type_e;

/**
 * @brief socp event callback function pointer
 */
typedef int (*socp_event_cb)(unsigned int chan_id, unsigned int event, unsigned int param);
/**
 * @brief socp read callback function pointer
 */
typedef int (*socp_read_cb)(unsigned int chan_id);
/**
 * @brief socp rd read callback function pointer
 */
typedef int (*socp_rd_cb)(unsigned int chan_id);

/**
 * @brief socp rd descriptor structure
 */
typedef struct {
    unsigned long long data_addr; /**< bd data buffer address */
    unsigned short data_len;      /**< bd data buffer length */
    unsigned short data_type;     /**< bd data buffer type */
    unsigned int reservd;         /**< reservd */
} socp_bd_data_s;

/**
 * @brief socp rd descriptor structure
 */
typedef struct {
    unsigned long long data_addr; /**< rd data buffer address */
    unsigned short data_len;      /**< rd data buffer length */
    unsigned short data_type;     /**< rd data buffer type */
    unsigned int reservd;         /**< reservd */
} socp_rd_data_s;

/**
 * @brief socp src channel set buffer
 */
typedef struct {
    unsigned char *input_start;    /**< block buffer start address */
    unsigned char *input_end;      /**< block buffer end address */
    unsigned char *rd_input_start; /**< rd buffer start address */
    unsigned char *rd_input_end;   /**< rd buffer end address */
    unsigned int rd_threshold;     /**< rd buffer report threshold */
    unsigned short rd_timeout;     /**< rd buffer time out cost */
    unsigned short rsv;            /**< reserve */
} socp_src_setbuf_s;

#pragma pack(push)
#pragma pack(4)
/**
 * @brief socp 环形buffer结构
 */
typedef struct {
    char *buffer;
    char *rb_buffer;
    unsigned int size;
    unsigned int rb_size;
} socp_buffer_rw_s;

/**
 * @brief socp src channel struct
 */
typedef struct {
    unsigned int dest_chan_id;               /**< dest channel id */
    socp_hdlc_flag_e bypass_en;              /**< bypass enable or not */
    socp_trans_id_en_e trans_id_en;          /**< trans id enable or not */
    socp_ptr_img_en_e rptr_img_en;           /**< pointer image enable or not */
    socp_data_type_e data_type;              /**< data type */
    socp_data_type_en_e data_type_en;        /**< data type enable or not */
    socp_enc_debug_en_e debug_en;            /**< debug enable or not */
    socp_encsrc_chnmode_e mode;              /**< channel mode */
    socp_chan_priority_e priority;           /**< channel priority */
    unsigned long read_ptr_img_phy_addr;     /**< block, read pointer image phyical address */
    unsigned long read_ptr_img_vir_addr;     /**< block, read pointer image virtual address */
    unsigned int rd_write_ptr_img_addr_low;  /**< rd, write pointer image low address  */
    unsigned int rd_write_ptr_img_addr_high; /**< rd, write pointer image high address */
    unsigned int chan_group;                 /**< socp source channel group number */
    socp_src_setbuf_s coder_src_setbuf;      /**< source channel buffer set for */
} socp_src_chan_cfg_s;
#pragma pack(pop)

/**
 * @brief socp dest channel buffer struct
 */
typedef struct {
    unsigned char *output_start; /**< output buffer start address */
    unsigned char *output_end;   /**< output buffer end address */
    unsigned int threshold;      /**< buffer report threshold */
    unsigned int rsv;            /**< reserve */
} socp_dst_setbuf_s;

/**
 * @brief socp coder dest channel  struct
 */
typedef struct {
    unsigned int encdst_thrh;           /**< coder dest channel buffer threshold */
    unsigned int encdst_timeout_mode;   /**< coder dest channel timeout mode, after socp2.06 use */
    socp_dst_setbuf_s coder_dst_setbuf; /**< dest channel buffer set for */
} socp_dst_chan_cfg_s;

/**
 * @brief socp coder dest buff config
 */
typedef struct {
    void *vir_buffer;              /**< socp coder dest channel buffer virtual address */
    unsigned long phy_buffer_addr; /**< socp coder dest channel buffer phyical address */
    unsigned int buffer_size;      /**< socp coder dest channel buffer size */
    unsigned int over_time;        /**< socp coder dest channel buffer timeout */
    unsigned int log_on_flag;      /**< socp coder dest channel buffer log is on flag */
    unsigned int cur_time_out;     /**< socp coder dest channel buffer current timeout cost */
} socp_encdst_buf_log_cfg_s;

/**
 * @brief socp deflate压缩功能去使能
 * @par 描述:
 * socp deflate压缩功能去使能
 * @attention
 * <ul><li>NA</li></ul>
 * @param[in]  dst_chan_id , 目的通道ID。
 * @retval 0:  操作成功。
 * @retval 非0:  操作失败。
 * @par 依赖:
 * <ul><li>NA</li></ul>
 * @see
 */
int mdrv_deflate_compress_disable(unsigned int dst_chan_id);

/**
 * @brief socp deflate压缩功能去使能
 * @par 描述:
 * socp deflate压缩功能去使能
 * @attention
 * <ul><li>NA</li></ul>
 * @param[in]  dst_chan_id , 目的通道ID。
 * @retval 0:  操作成功。
 * @retval 非0:  操作失败。
 * @par 依赖:
 * <ul><li>NA</li></ul>
 * @see
 */
int mdrv_deflate_set_ind_mode(socp_ind_mode_e mode);

/**
 * @brief bbp采数内存开关使能
 * @par 描述:
 * bbp采数内存开关使能
 * @attention
 * <ul><li>NA</li></ul>
 * @retval 0:  操作成功。
 * @retval 非0:  操作失败。
 * @par 依赖:
 * <ul><li>NA</li></ul>
 * @see
 */
int mdrv_socp_bbpmemenable(void);

/**
 * @brief 获取socp目的端中断计数
 * @par 描述:
 * 获取socp ind目的端传输中断和上溢中断计数
 * @param[out]  trf_info, 传输中断计数
 * @param[out]  trh_ovf_info, 上溢中断计数
 * @attention
 * <ul><li>NA</li></ul>
 * @retval 无
 * @par 依赖:
 * <ul><li>NA</li></ul>
 * @see
 */
void mdrv_socp_mntn_enc_dst_int_info(unsigned int *trf_info, unsigned int *trh_ovf_info);

/**
 * @brief 清除socp目的端中断信息
 * @par 描述:
 * 清除socp目的端中断信息
 * @attention
 * <ul><li>NA</li></ul>
 * @retval 无
 * @par 依赖:
 * <ul><li>NA</li></ul>
 * @see
 */
void mdrv_clear_socp_encdst_int_info(void);

/**
 * @brief 本接口用于设置ind目的端上报模式
 * @par 描述:
 * 本接口用于设置ind目的端上报模式
 * @attention
 * <ul><li>NA</li></ul>
 * @param[out]  mode   上报模式
 * @retval 0:  设置成功。
 * @retval 非0：设置失败。
 * @par 依赖:
 * <ul><li>NA</li></ul>
 * @see
 */
int mdrv_socp_set_ind_mode(socp_ind_mode_e mode);

/**
 * @brief 本接口用于启动socp源通道
 * @par 描述:
 * 本接口用于启动socp源通道
 * @attention
 * <ul><li>NA</li></ul>
 * @param[in]  src_chan_id 源通道ID
 * @retval 0:  启动源通道成功。
 * @retval 非0：启动源通道失败。
 * @par 依赖:
 * <ul><li>NA</li></ul>
 * @see
 */
int mdrv_socp_start(unsigned int src_chan_id);

/**
 * @brief 本接口用于停止socp源通道
 * @par 描述:
 * 本接口用于停止socp源通道
 * @attention
 * <ul><li>NA</li></ul>
 * @param[in]  src_chan_id 源通道ID
 * @retval 0:  停止源通道成功。
 * @retval 非0：停止源通道失败。
 * @par 依赖:
 * <ul><li>NA</li></ul>
 * @see
 */
int mdrv_socp_stop(unsigned int src_chan_id);

/**
 * @brief 本接口用于控制SOCP目的端数据上报管理状态机
 * @par 描述:
 * 本接口用于控制SOCP目的端数据上报管理状态机
 * @attention
 * <ul><li>NA</li></ul>
 * @param[in]  dest_chan_id 编码目的通道号
 * @param[in]  enable    是否打开socp目的端中断
 * @retval 0:  操作成功。
 * @retval 非0：操作失败。
 * @par 依赖:
 * <ul><li>NA</li></ul>
 * @see
 */
void mdrv_socp_send_data_manager(unsigned int dest_chan_id, unsigned int enable);

/**
 * @brief 本接口用于获取读数据缓冲区指针。
 * @par 描述:
 * 本接口用于获取读数据缓冲区指针。
 * @attention
 * <ul><li>NA</li></ul>
 * @param[in]  dst_chan_id  目的通道ID
 * @param[out]  pBuffer   读取大小
 * @retval 0:  获取成功。
 * @retval 非0：获取失败。
 * @par 依赖:
 * <ul><li>NA</li></ul>
 * @see
 */
int mdrv_socp_get_read_buffer(unsigned int dst_chan_id, socp_buffer_rw_s *buffer);

/**
 * @brief 清除socp目的端中断信息
 * @par 描述:
 * 清除socp目的端中断信息
 * @attention
 * <ul><li>NA</li></ul>
 * @retval 无
 * @par 依赖:
 * <ul><li>NA</li></ul>
 * @see
 */
int mdrv_clear_socp_buff(unsigned int src_chan_id);

/**
 * @brief 本接口用于查询刷新后LOG2.0 SOCP超时配置信息
 * @par 描述:
 * 本接口用于查询刷新后LOG2.0 SOCP超时配置信息
 * @attention
 * <ul><li>NA</li></ul>
 * @param[out]  cfg   log配置的信息
 * @retval 0:  获取成功。
 * @retval 非0：获取失败。
 * @par 依赖:
 * <ul><li>NA</li></ul>
 * @see
 */
unsigned int mdrv_socp_get_sd_logcfg(socp_encdst_buf_log_cfg_s *cfg);

/**
 * @brief 本接口用于查询刷新后LOG2.0 SOCP超时配置信息
 *
 * @par 描述:
 * 本接口用于查询刷新后LOG2.0 SOCP超时配置信息
 *
 * @attention
 * <ul><li>NA</li></ul>
 *
 * @param[out]  cfg   log配置的信息
 *
 * @retval 0:  获取成功。
 * @retval 非0：获取失败。
 *
 * @par 依赖:
 * <ul><li>NA</li></ul>
 *
 * @see
 */
unsigned int mdrv_socp_get_log_cfg(socp_encdst_buf_log_cfg_s *cfg);

/**
 * @brief 本接口用于获取写数据buffer
 * @par 描述:
 * 本接口用于获取写数据buffer
 * @attention
 * <ul><li>NA</li></ul>
 * @param[in]  src_chan_id 源通道ID
 * @param[in]  pBuff  写入buffer大小
 * @retval 0:  获取可用buffer成功。
 * @retval 非0：获取可用buffer失败。
 * @par 依赖:
 * <ul><li>NA</li></ul>
 * @see
 */
int mdrv_socp_get_write_buff(unsigned int src_chan_id, socp_buffer_rw_s *pBuff);

/**
 * @brief 本接口用于表明写入socp源通道数据完成
 * @par 描述:
 * 本接口用于表明写入socp源通道数据完成及大小
 * @attention
 * <ul><li>NA</li></ul>
 * @param[in]  src_chan_id 源通道ID
 * @param[in]  write_size  写入buffer大小
 * @retval 0:  获取可用buffer成功。
 * @retval 非0：获取可用buffer失败。
 * @par 依赖:
 * <ul><li>NA</li></ul>
 * @see
 */
int mdrv_socp_write_done(unsigned int src_chan_id, unsigned int write_size);

/**
 * @brief 本接口用于设置socp源通道属性
 * @par 描述:
 * 本接口用于设置socp源通道属性
 * @attention
 * <ul><li>NA</li></ul>
 * @param[in]  src_chan_id 源通道ID
 * @param[in]  src_attr    源通道属性
 * @retval 0:  源通道属性设置成功。
 * @retval 非0：源通道属性设置失败。
 * @par 依赖:
 * <ul><li>NA</li></ul>
 * @see
 */
int mdrv_socp_corder_set_src_chan(unsigned int src_chan_id, socp_src_chan_cfg_s *src_attr);

/**
 * @brief 清除socp源端buffer
 * @par 描述:
 * 清除socp源端buffer
 * @attention
 * <ul><li>NA</li></ul>
 * @retval 无
 * @par 依赖:
 * <ul><li>NA</li></ul>
 * @see
 */
int mdrv_clear_socp_buff(unsigned int src_chan_id);

/**
 * @brief 清除socp目的端buffer
 * @par 描述:
 * 清除socp目的端buffer
 * @attention
 * <ul><li>NA</li></ul>
 * @retval 无
 * @par 依赖:
 * <ul><li>NA</li></ul>
 * @see
 */
int mdrv_clear_socp_dst_buffer(unsigned int dst_chan_id);

#ifdef __cplusplus
}
#endif
#endif
