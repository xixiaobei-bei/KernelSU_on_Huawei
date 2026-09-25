

#ifndef __OAM_EVENT_H__
#define __OAM_EVENT_H__

/* 其他头文件包含 */
#include "oal_ext_if.h"
#include "oam_ext_if.h"

/* 宏定义 */
#define OAM_OTA_DATA_TO_STD_MAX_LEN  300
#define OAM_OTA_FRAME_TO_SDT_MAX_LEN 1200
#define OAM_SKB_CB_LEN               OAL_NETBUF_CB_SIZE()

/* 函数声明 */
extern oal_uint32 oam_event_init(oal_void);

#endif /* end of oam_event.h */
