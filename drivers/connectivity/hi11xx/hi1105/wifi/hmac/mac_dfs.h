
#ifndef __MAC_DFS_H__
#define __MAC_DFS_H__

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_DFS_H

#ifdef _PRE_WLAN_FEATURE_DFS
typedef struct {
    oal_bool_enum_uint8 en_dfs_switch; /* DFS使能开关 bit0:dfs使能,bit1:标示AP因为DFS特性暂时处于邋wait_start */
    oal_bool_enum_uint8 en_cac_switch;
    oal_bool_enum_uint8 en_offchan_cac_switch;
    uint8_t uc_debug_level;  /* bit0:打印雷达带业务，bit1:打印雷达无业务 */
    uint8_t uc_offchan_flag; /* bit0:0表示homechan,1表示offchan; bit1:0表示普通,1表示offchancac */
    uint8_t uc_offchan_num;
    uint8_t uc_timer_cnt;
    uint8_t uc_timer_end_cnt;
    uint8_t uc_cts_duration;
    uint8_t uc_dmac_channel_flag; /* dmac用于标示当前信道off or home */
    oal_bool_enum_uint8 en_dfs_init;
    uint8_t uc_custom_next_chnum;                   /* 应用层指定的DFS下一跳信道号 */
    uint32_t dfs_cac_outof_5600_to_5650_time_ms; /* CAC检测时长，5600MHz ~ 5650MHz频段外，默认60秒 */
    uint32_t dfs_cac_in_5600_to_5650_time_ms;    /* CAC检测时长，5600MHz ~ 5650MHz频段内，默认10分钟 */
    /* Off-Channel CAC检测时长，5600MHz ~ 5650MHz频段外，默认6分钟 */
    uint32_t off_chan_cac_outof_5600_to_5650_time_ms;
    /* Off-Channel CAC检测时长，5600MHz ~ 5650MHz频段内，默认60分钟 */
    uint32_t off_chan_cac_in_5600_to_5650_time_ms;
    uint16_t us_dfs_off_chan_cac_opern_chan_dwell_time; /* Off-channel CAC在工作信道上驻留时长 */
    uint16_t us_dfs_off_chan_cac_off_chan_dwell_time;   /* Off-channel CAC在Off-Channel信道上驻留时长 */
    uint32_t radar_detected_timestamp;
    int32_t l_radar_th;                  // 设置的雷达检测门限，单位db
    uint32_t custom_chanlist_bitmap;  // 应用层同步下来的信道列表
    // (0x0000000F) /* 36--48 */
    // (0x000000F0) /* 52--64 */
    // (0x000FFF00) /* 100--144 */
    // (0x01F00000) /* 149--165 */
    wlan_channel_bandwidth_enum_uint8 en_next_ch_width_type;  // 设置的下一跳信道的带宽模式
    uint8_t uac_resv[NUM_3_BYTES];
    uint32_t dfs_non_occupancy_period_time_ms;
    uint8_t uc_radar_type;
    uint8_t uc_band;
    uint8_t uc_channel_num;
    uint8_t uc_flag;
} mac_dfs_info_stru;

typedef struct {
    frw_timeout_stru st_dfs_cac_timer;          /* CAC定时器 */
    frw_timeout_stru st_dfs_off_chan_cac_timer; /* Off-Channel CAC定时器 */
    frw_timeout_stru st_dfs_chan_dwell_timer;   /* 信道驻留定时器，定时器到期，切离该信道 */
    frw_timeout_stru st_dfs_radar_timer;
    mac_dfs_info_stru st_dfs_info;
    oal_dlist_head_stru st_dfs_nol;
} mac_dfs_core_stru;
#endif

#endif /* end of mac_dfs.h */
