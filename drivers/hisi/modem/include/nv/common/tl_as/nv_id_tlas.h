/******************************************************************************

    Copyright(C)2008,Hisilicon Co. LTD.

 ******************************************************************************
  File Name       : LNvCommon.h
  Description     : LNvCommon.h header file
  History           :
     1.Name+ID      yyyy-mm-dd  Draft Enact

******************************************************************************/

#ifndef __LNVCOMMON_H__
#define __LNVCOMMON_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
/* this file is included by drv, submit by zhouluojun, reviewed by hujianbo */
/* #pragma pack(4) */

/*****************************************************************************
  2 macro
*****************************************************************************/

/*****************************************************************************
  3 Massage Declare
*****************************************************************************/


/*****************************************************************************
  4 Enum
    COMMON TLAS NV值使用范围
*****************************************************************************/
enum NV_ITEM_ID_ENUM
{
    EN_NV_ID_BEGIN                                  = 0,
    EN_NV_ID_UE_CAPABILITY                          = 0xD22c,
    EN_NV_ID_SWITCH_PARA                            = 0xD275,
    EN_NV_ID_BANDNon1_BAND_INFO                     = 0xD2ED,/* 非标频段的频段信息begin */
    EN_NV_ID_BANDNon16_BAND_INFO                    = 0xD2FC,/* 非标频段的频段信息end */
    EN_NV_ID_POWER_ON_LOG_SWITCH                    = 0xD33b,
};
typedef unsigned long NV_ITEM_ID_ENUM_UINT32;

/*****************************************************************************

*****************************************************************************/


/* this file is included by drv, submit by zhouluojun, reviewed by hujianbo */
/*
#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif
*/



#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of PsNvInterface.h */

