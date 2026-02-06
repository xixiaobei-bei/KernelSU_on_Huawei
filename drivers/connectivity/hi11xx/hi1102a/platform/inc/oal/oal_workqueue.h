

#ifndef __OAL_WORKQUEUE_H__
#define __OAL_WORKQUEUE_H__

/* 其他头文件包含 */
#include "oal_types.h"
#include "oal_hardware.h"
#include "oal_mm.h"
#include "arch/oal_workqueue.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_WORKQUEUE_H

/* 函数声明 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
extern oal_int32 oal_workqueue_schedule(oal_work_stru *pst_work);
extern oal_int32 oal_workqueue_delay_schedule(oal_delayed_work *pst_work, oal_ulong delay);
extern oal_uint32 oal_workqueue_init(oal_void);
extern oal_uint32 oal_workqueue_exit(oal_void);
#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
OAL_STATIC OAL_INLINE oal_int32 oal_workqueue_schedule(oal_work_stru *pst_work)
{
    return 0;
}
OAL_STATIC OAL_INLINE oal_int32 oal_workqueue_delay_schedule(oal_delayed_work *pst_work, oal_ulong delay)
{
    return 0;
}

OAL_STATIC OAL_INLINE oal_uint32 oal_workqueue_init(oal_void)
{
    return 0;
}
OAL_STATIC OAL_INLINE oal_uint32 oal_workqueue_exit(oal_void)
{
    return 0;
}
#endif

#endif /* end of oal_workqueue.h */
