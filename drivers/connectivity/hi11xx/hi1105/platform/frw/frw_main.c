

/* 头文件包含 */
#include "frw_main.h"
#include "frw_task.h"
#include "frw_event_main.h"
#include "frw_event_deploy.h"
#include "frw_ext_if.h"
#include "frw_timer.h"

#include "oal_hcc_host_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FRW_MAIN_C

/* 全局变量定义 */
OAL_STATIC uint16_t g_en_wlan_driver_init_state = FRW_INIT_STATE_BUTT;
OAL_STATIC frw_event_sub_table_item_stru g_frw_timeout_event_sub_table[FRW_TIMEOUT_SUB_TYPE_BUTT];

/*
  3 函数实现
 */
void frw_event_fsm_register(void)
{
    g_frw_timeout_event_sub_table[FRW_TIMEOUT_TIMER_EVENT].p_func = frw_timer_timeout_proc;
    frw_event_table_register(FRW_EVENT_TYPE_TIMEOUT, FRW_EVENT_PIPELINE_STAGE_0, g_frw_timeout_event_sub_table);
}

int32_t frw_main_init(void)
{
    uint32_t ul_ret;

    frw_set_init_state(FRW_INIT_STATE_START);

    /* 事件管理模块初始化 */
    ul_ret = frw_event_init();
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_FRW, "{frw_main_init:: frw_event_init return err code: %d}", ul_ret);
        return -OAL_EFAIL;
    }

    frw_task_init();

#if (_PRE_MULTI_CORE_MODE_PIPELINE_AMP == _PRE_MULTI_CORE_MODE)
    /* 事件核间部署模块初始化 */
    ul_ret = frw_event_deploy_init();
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_FRW, "{frw_main_init:: frw_event_deploy_init return err code: %d}", ul_ret);
        return -OAL_EFAIL;
    }
#endif

    frw_timer_init(FRW_TIMER_DEFAULT_TIME, frw_timer_timeout_proc_event, 0);

    frw_event_fsm_register();

    /* 启动成功后，输出打印 设置状态始终放最后 */
    frw_set_init_state(FRW_INIT_STATE_FRW_SUCC);

    return OAL_SUCC;
}

void frw_main_exit(void)
{
    /* 卸载定时器 */
    frw_timer_exit();

    frw_task_exit();

    /* 卸载事件管理模块 */
    frw_event_exit();

#if (_PRE_MULTI_CORE_MODE_PIPELINE_AMP == _PRE_MULTI_CORE_MODE)
    /* 事件核间部署模块卸载 */
    frw_event_deploy_exit();
#endif

    /* 卸载成功后在置状态位 */
    frw_set_init_state(FRW_INIT_STATE_START);

    /* 卸载成功后，输出打印 */
    return;
}

/*
 * 函 数 名  : frw_set_init_state
 * 功能描述  : 设置初始化状态
 * 输入参数  : 初始化状态
 */
void frw_set_init_state(uint16_t en_init_state)
{
    if (en_init_state >= FRW_INIT_STATE_BUTT) {
        oam_warning_log1(0, OAM_SF_FRW, "{frw_set_init_state:: en_init_state >= FRW_INIT_STATE_BUTT: %d}",
                         en_init_state);
        return;
    }

    g_en_wlan_driver_init_state = en_init_state;

    return;
}

/*
 * 函 数 名  : frw_get_init_state
 * 功能描述  : 获取初始化状态
 */
uint16_t frw_get_init_state(void)
{
    return g_en_wlan_driver_init_state;
}

/*lint -e578*/ /*lint -e19*/
oal_module_symbol(frw_main_init);
oal_module_symbol(frw_main_exit);

oal_module_symbol(frw_set_init_state);
oal_module_symbol(frw_get_init_state);
