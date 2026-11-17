

#include "hmac_soft_retry.h"
#include "hmac_tx_msdu_dscr.h"
#include "hmac_host_tx_data.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SOFT_RETRY_C

#define BA_INFO_BITMAP_WROD_SIZE 32 /* MAC上报的BA BITMAP每个word的大小, BITMAP共8 * 32 bits */
#define BA_INFO_BITMAP_WROD_NUM 8


uint32_t hmac_tx_soft_retry_process(hmac_tid_info_stru *tx_tid_info, oal_netbuf_stru *netbuf, uint16_t seq_num)
{
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);

    if (MAC_GET_CB_RETRIED_NUM(tx_ctl) == 0 || tx_tid_info->bitmap_mgmt.ba_bitmap[seq_num] != 0) {
        /* 重传剩余次数0或该帧发送成功, 则无需重传 */
        return OAL_FAIL;
    }

    MAC_GET_CB_RETRIED_NUM(tx_ctl)--;
    oal_atomic_dec(&g_tid_schedule_mgmt.ring_mpdu_num);

    oam_warning_log4(0, OAM_SF_TX, "{hmac_tx_soft_retry_process::user[%d] tid[%d] retry frame type[%d] subtype[%d]",
        tx_tid_info->user_index, tx_tid_info->tid_no, MAC_GET_CB_FRAME_TYPE(tx_ctl), MAC_GET_CB_FRAME_SUBTYPE(tx_ctl));

    hmac_host_tx_tid_enqueue(tx_tid_info, netbuf, OPERATE_TYPE_TAIL);

    return OAL_SUCC;
}

void hmac_tx_tid_update_ba_bitmap(hmac_tid_info_stru *tid_info, mac_tx_ba_info_stru *tx_ba_info)
{
    hmac_ba_bitmap_mgmt_stru *ba_bitmap_mgmt = &tid_info->bitmap_mgmt;
    uint16_t seqnum;
    uint16_t sn_index;
    uint8_t ba_info_bitmap_word;
    uint8_t ba_info_seqnum_bit;

    if (!tx_ba_info->ba_info_vld) {
        return;
    }

    for (sn_index = 0; sn_index < BA_INFO_BITMAP_WROD_SIZE * BA_INFO_BITMAP_WROD_NUM; sn_index++) {
        ba_info_bitmap_word = sn_index / BA_INFO_BITMAP_WROD_SIZE;
        ba_info_seqnum_bit = sn_index % BA_INFO_BITMAP_WROD_SIZE;

        seqnum = (tx_ba_info->ba_ssn + sn_index) % HMAC_BA_BITMAP_MAX_SIZE;
        ba_bitmap_mgmt->ba_bitmap[seqnum] =
            (tx_ba_info->ba_bitmap[ba_info_bitmap_word] & BIT(ba_info_seqnum_bit)) != 0;
    }
}
