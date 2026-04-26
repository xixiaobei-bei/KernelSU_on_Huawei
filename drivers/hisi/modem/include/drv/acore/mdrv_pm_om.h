/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 */

/**
 *  @brief   pmom mdrv interface for ps
 *  @file    mdrv_pm_om.h
 *  @author  lixiaojie
 *  @version v1.0
 *  @date    2019.12.17
 *  @note
 *  <ul><li>v1.0|2012.11.29|create file</li></ul>
 *  <ul><li>v2.0|2019.12.17|modify for interface automation</li></ul>
 *  @since
 */

#ifndef MDRV_PM_OM_H
#define MDRV_PM_OM_H

#ifdef __cplusplus
extern "C"
{
#endif
/**
* @brief pmom moid enum for all
*/
enum pm_om_mod_id {
    PM_MOD_CP_OSA    =  18,
    PM_MOD_CP_MSP    =  19,
    PM_MOD_BEGIN     =  32,
    PM_OM_CAS_1X     =  33,
    PM_OM_CPROC_1X   =  34,
    PM_OM_CAS_EVDO   =  35,
    PM_OM_CPROC_EVDO =  36,
    PM_OM_TLPHY      =  37,
    PM_OM_TRRC       =  38,
    PM_OM_LRRC       =  39,
    PM_OM_WPHY       =  40,
    PM_OM_GPHY       =  41,
    PM_OM_WRRC       =  42,
    PM_MOD_AP_OSA    =  43,
    PM_OM_GUCNAS     =  44,
    PM_OM_GRRC       =  45,
    PM_OM_LL2        =  46,
    PM_OM_NPHY       =  47,
    PM_OM_NRRC       =  48,
    PM_OM_EASYRF     =  49,
    PM_MOD_END       =  50,
    PM_OM_MOD_ID_ENUM_MAX = 64,   /**< max is 64, as nv is 64 bit */
};

/**
* @brief pmom ascii
*/
enum pm_om_magic {
    PM_OM_MAGIC_AOSA = 0x41534F41, /**< AOSA : OSA ACORE */
    PM_OM_MAGIC_COSA = 0x41534F43, /**< COSA : OSA CCORE */
    PM_OM_MAGIC_CMSP = 0x50534D43, /**< CMSP : MSP CCORE */
    PM_OM_MAGIC_CASX = 0x58534143, /**< CASX : CAS 1X */
    PM_OM_MAGIC_CPRX = 0x58525043, /**< CPRX : CPROC 1X */
    PM_OM_MAGIC_CASE = 0x45534143, /**< CASE : CAS EVDO */
    PM_OM_MAGIC_CPRE = 0x45525043, /**< CPRE : CPROC EVDO */
    PM_OM_MAGIC_TLPY = 0x48504C54, /**< TLPH : TL PHY */
    PM_OM_MAGIC_TRRC = 0x43525254, /**< TRRC : T RRC */
    PM_OM_MAGIC_LRRC = 0x4352524C, /**< LRRC : L RRC */
    PM_OM_MAGIC_WPHY = 0x59485057, /**< WPHY : W PHY */
    PM_OM_MAGIC_GPHY = 0x59485047, /**< GPHY : G PHY */
    PM_OM_MAGIC_GUCN = 0x4E435547, /**< GUCN : GUC NAS */
    PM_OM_MAGIC_WRRC = 0x43525257, /**< WRRC : W AS */
    PM_OM_MAGIC_GRRC = 0x43525247, /**< GRRC : G AS */
    PM_OM_MAGIC_TLL2 = 0x324C4C54, /**< TLL2 : T LL2 */
    PM_OM_MAGIC_NPHY = 0x5948504E, /**< NPHY : N PHY */
    PM_OM_MAGIC_NRRC = 0x4352524E, /**< NRRC : NR RRC */
    PM_OM_MAGIC_BBRF = 0x46524242, /**< BBRF : EASYRF */
};

/**
 * @brief struct pm_info_usr_data for user data
 */
struct pm_info_usr_data {
    unsigned int mod_id; /**< PM_OM_MOD_ID_ENUM */
    unsigned int magic; /**< PM_OM_MAGIC_ENUM */
    void         *buf; /**< 组件记录功耗状态信息的的内存起始地址 */
    unsigned int buf_len; /**< 组件记录功耗状态信息的的内存大小 */
    void         *context; /**< 用户私有数据区 */
};

/**
 * @brief pm_info_cbfun modem功耗状态信息收集回调函数指针
 */
typedef int (*pm_info_cbfun)(struct pm_info_usr_data *usr_data);

/**
 * @brief pm info 注册接口
 *
 * @par 描述:
 * 注册modem功耗状态信息收集回调函数和用户信息
 *
 * @attention
 * <ul><li>usr_data 用户数据, 不允许传入NULL指针</li></ul>
 *
 * @param[in]  pcbfun   modem功耗状态信息收集回调函数, 需调用mdrv_pm_log将数据上报
 * @param[in]  usr_data 用户数据, 不允许传入NULL指针
 *
 * @retval 0,表示函数执行成功。
 * @par 依赖:
 * <ul><li>NA</li></ul>
 *
 * @see
 */
int mdrv_pm_info_stat_register(pm_info_cbfun pcbfun, struct pm_info_usr_data *usr_data);


/**
 * @brief pm info 输出接口
 *
 * @par 描述:
 * 输出流程信息到log区
 *
 * @attention
 * <ul><li>usr_data 用户数据, 不允许传入NULL指针</li></ul>
 *
 * @param[in]  mod_id    模块id，使用enum PM_OM_MOD_ID_ENUM枚举类型定义的值
 * @param[out]  data_len    输出数据长度
 * @param[out]  data         输出数据
 * @retval 0,表示函数执行成功。
 * @par 依赖:
 * <ul><li>NA</li></ul>
 *
 * @see
 */
int mdrv_pm_log(int mod_id, unsigned int data_len, const void *data);

#ifdef __cplusplus
}
#endif
#endif
