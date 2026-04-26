#ifndef _TAF_DSP_PHY_
#define _TAF_DSP_PHY_

#ifdef MODEM_FUSION_VERSION
#include "v_basic_type_def.h"

#ifdef LPS_RTT

/* 物理层使用消息头定义和头文件包含 */
#include "mdrv_msg.h"
#ifndef VOS_MSG_HEADER
#define VOS_MSG_HEADER MDRV_MSG_HEAD
#endif

#else
#include "vos.h"
#endif /* End def LPS_RTT */

#else
#include "vos.h"
#endif /* MODEM_FUSION_VERSION */

#endif
