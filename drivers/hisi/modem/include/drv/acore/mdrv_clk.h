/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 */

 /**
 *  @brief   clk模块在acore上的对外头文件
 *  @file    mdrv_clk.h
 *  @author  maluping
 *  @version v1.0
 *  @date    2019.11.27
 *  @note    修改记录(版本号|修订日期|说明)
 *  <ul><li>v1.0|2019.11.27|创建文件</li></ul>
 *  @since
 */
#ifndef __MDRV_CLK_H__
#define __MDRV_CLK_H__

#include "mdrv_public.h"

#ifdef __cplusplus
extern "C" {
#endif

 /**
* @brief  acc 类型枚举
*/
enum mdrv_clk_acc_type {
    CLK_HDLC,
    CLK_ACC_TYPE_MAX
};

/**
 * @brief 打开时钟
 *
 * @par 描述:
 * 根据acc类型打开对应时钟
 *
 * @attention
 * <ul><li>超出acc类型范围会报错。</li></ul>
 *
 * @param[in]  acc_clk_index , acc类型值。
 *
 * @retval 0,表示操作成功。
 * @retval -1,表示操作失败。
 *
 * @par 依赖:
 * <ul><li>mdrv_clk.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_clk_acc_type
 */
int mdrv_acc_clk_enable(enum mdrv_clk_acc_type acc_clk_index);
/**
 * @brief 关闭时钟
 *
 * @par 描述:
 * 根据acc类型关闭对应时钟
 *
 * @attention
 * <ul><li>超出acc类型范围会报错。</li></ul>
 *
 * @param[in]  acc_clk_index , acc类型值。
 *
 * @par 依赖:
 * <ul><li>mdrv_clk.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_clk_acc_type
 */
void mdrv_acc_clk_disable(enum mdrv_clk_acc_type acc_clk_index);
/**
 * @brief 查询时钟状态
 *
 * @par 描述:
 * 根据acc类型查询时钟状态
 *
 * @attention
 * <ul><li>超出acc类型范围会报错。</li></ul>
 *
 * @param[in]  acc_clk_index , acc类型值。
 *
 * @retval 1,时钟是打开的。
 * @retval 0,时钟是关闭的。
 * @retval -1,无该时钟节点。
 *
 * @par 依赖:
 * <ul><li>mdrv_clk.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_clk_acc_type
 */
int mdrv_acc_clk_is_enabled(enum mdrv_clk_acc_type acc_clk_index);

#ifdef __cplusplus
}
#endif

#endif
