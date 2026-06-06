

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_workqueue.h"

#include "oal_main.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_WORKQUEUE_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_workqueue_stru *oal_workqueue; /* oal工作队列全局变量 */

oal_int32 oal_workqueue_schedule(oal_work_stru *pst_work)
{
    if (oal_workqueue == OAL_PTR_NULL) {
        OAL_IO_PRINT("oal_workqueue is null.\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    return oal_queue_work(oal_workqueue, pst_work);
}

oal_int32 oal_workqueue_delay_schedule(oal_delayed_work *pst_work, oal_ulong delay)
{
    if (oal_workqueue == OAL_PTR_NULL) {
        OAL_IO_PRINT("oal_workqueue is null.\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    return oal_queue_delayed_work(oal_workqueue, pst_work, delay);
}

/*
 * 函 数 名  : oal_workqueue_init
 * 功能描述  : oal workqueue初始化
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : OAL_SUCC
 */
oal_uint32 oal_workqueue_init(oal_void)
{
    oal_workqueue = OAL_CREATE_SINGLETHREAD_WORKQUEUE("oal_workqueue");

    if (oal_workqueue == OAL_PTR_NULL) {
        OAL_IO_PRINT("oal_workqueue_init: create oal workqueue failed.\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_workqueue_exit
 * 功能描述  : oal workqueue退出函数
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : OAL_SUCC
 */
oal_uint32 oal_workqueue_exit(oal_void)
{
    /* 删除工作队列 */
    oal_destroy_workqueue(oal_workqueue);

    return OAL_SUCC;
}
oal_module_symbol(oal_workqueue_schedule);

oal_module_symbol(oal_workqueue_delay_schedule);

oal_module_license("GPL");

#endif


