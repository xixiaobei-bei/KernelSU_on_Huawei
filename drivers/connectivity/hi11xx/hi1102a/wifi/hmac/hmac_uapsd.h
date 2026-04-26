
#ifndef __HMAC_UAPSD_H__
#define __HMAC_UAPSD_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "mac_user.h"
#include "hmac_ext_if.h"
#include "dmac_ext_if.h"
#include "hmac_user.h"
#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_UAPSD_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_UAPSD_SEND_ALL 0xff /* 发送队列中所有报文,设置为UINT8变量最大值 */
#define HMAC_UAPSD_WME_LEN  8
extern oal_void hmac_uapsd_update_user_para(oal_uint8 *puc_mac_hdr,
                                            oal_uint8 uc_sub_type,
                                            oal_uint32 ul_msg_len,
                                            hmac_user_stru *pst_hmac_user);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_uapsd.h */
