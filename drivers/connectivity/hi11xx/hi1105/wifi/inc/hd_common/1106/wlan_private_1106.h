

#ifndef __WLAN_PRIVATE_1106_H__
#define __WLAN_PRIVATE_1106_H__

typedef struct {
    uint8_t hdr_len;            /* 帧头长度 */
    uint8_t cali;               /* ftm 校准使能 */
    uint8_t band_cap;           /* 指示 带宽 */
    uint8_t prot_format;        /* 指示 协议类型 */
    uint8_t chain_selection;    /* 通道选择 */
    uint8_t divider;            /*  分频系数 */
    uint16_t frame_len;         /* 帧长度 */
} mac_ftm_action_event_stru;

#endif /* #ifndef __WLAN_PRIVATE_1106_H__ */
