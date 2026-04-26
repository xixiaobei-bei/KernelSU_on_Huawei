/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 */

#ifndef __MDRV_MEMORY_LAYOUT_H__
#define __MDRV_MEMORY_LAYOUT_H__

/**
 * @brief 获取DDR内存布局
 * @par 描述:
 * 获取DDR内存布局
 * @param[in] name: 要获取的DDR内存段名称（名称在DTS中注册）name长度不超过16字节
 * ------名称-------------说明--------------
 * "mntn_ddr"         维测内存空间
 * "share_mem_ddr"    非安全共享内存
 * "tlphy_ddr"        LRPHY镜像
 * "mdm_dts_ddr"      modem DTB空间
 * "mdm_ddr"          modem运行空间
 * "cbbe_ddr"         CBBE16
 * "socp_bbpds_ddr"   SOCP数采空间
 * "rf_sub6g_ddr"     RFIC sub6g镜像
 * "rf_hf_ddr"        RFIC高频镜像
 * "rf_dump_ddr"      RFIC dump空间
 * "sec_share_ddr"    安全共享内存
 * "nrphy_ddr"        NRPHY的镜像
 * "pde_ddr"          PDE镜像空间
 * "mdm_maa_ddr"      用户面数据Modem内存
 * "emu_hac_ddr"      EMU HAC加载空间
 * "mdm_nr_ddr"       NR_MDM CORE使用内存
 * "sharednr_ddr"     NR共享内存
 * "share_nsro_ddr"   非安全只读不可写共享内存
 * "share_unsec_ddr"  非安全共享内存
 * "share_sec_ddr"    安全共享内存
 * "tvp_ddr"          TVP运行空间
 * -----------------------------------------
 * @param[in] size: ddr内存段的大小
 * @retval 0 申请失败
 * @retval 其它 返回DDR内存段的物理地址
 */
unsigned long mdrv_mem_region_get(const char *name, unsigned int *size);

#endif /* __MDRV_MEMORY_LAYOUT_H__ */ 
