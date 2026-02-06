

#ifndef __HMAC_FRAG_H__
#define __HMAC_FRAG_H__

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "hmac_main.h"
#include "hmac_tx_data.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_FRAG_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

oal_netbuf_stru *hmac_defrag_process(hmac_user_stru *pst_hmac_user,
                                     oal_netbuf_stru *pst_netbuf,
                                     uint32_t hrdsize);
uint32_t hmac_frag_process(hmac_vap_stru *pst_hmac_vap,
                           oal_netbuf_stru *pst_netbuf_original,
                           mac_tx_ctl_stru *pst_tx_ctl,
                           uint32_t cip_hdrsize,
                           uint32_t max_tx_unit);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_frag.h */
