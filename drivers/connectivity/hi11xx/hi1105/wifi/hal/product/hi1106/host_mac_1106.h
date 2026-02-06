

#ifndef __HOST_MAC_1106_H__
#define __HOST_MAC_1106_H__

#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "hal_common.h"
#include "host_hal_device.h"

#if (_PRE_TEST_MODE == _PRE_TEST_MODE_ST)
#define HI1106_MAC_CFG_HOST_REG_BUTT       0xBC
#define HI1106_MAC_RPT_HOST_REG_BUTT       0xBC
#define HI1106_DFX_REG_BANK_BUTT           0xBC
#define HI1106_MAC_CFG_DMAC_BANK0_REG_BUTT 0xBC
#define HI1106_MAC_RPT_DMAC_REG_BUTT       0xBC
#define HI1106_MAC_CFG_DMAC_BANK4_REG_BUTT 0xBC
#define HI1106_RX_CHN_EST_REG_BANK_BUTT    0xBC

uint32_t g_st_host_hi1106_mac_cfg_host_reg_base[HI1106_MAC_CFG_HOST_REG_BUTT];
uint32_t g_st_host_hi1106_mac_rpt_host_reg_base[HI1106_MAC_RPT_HOST_REG_BUTT];
uint32_t g_st_host_hi1106_dfx_reg_bank_base[HI1106_DFX_REG_BANK_BUTT];
uint32_t g_st_host_hi1106_mac_cfg_dmac_bank0_reg_base[HI1106_MAC_CFG_DMAC_BANK0_REG_BUTT];
uint32_t g_st_host_hi1106_mac_rpt_dmac_reg_base[HI1106_MAC_RPT_DMAC_REG_BUTT];
uint32_t g_st_host_hi1106_mac_cfg_dmac_bank4_reg_base[HI1106_MAC_CFG_DMAC_BANK4_REG_BUTT];
uint32_t g_st_host_hi1106_rx_chn_est_reg_band_base[HI1106_RX_CHN_EST_REG_BANK_BUTT];
uint32_t hi1106_rf_ctrl_reg_bank_0_base[HI1106_RX_CHN_EST_REG_BANK_BUTT];
uint32_t hi1106_cali_test_reg_bank_0_base[HI1106_RX_CHN_EST_REG_BANK_BUTT];
uint32_t hi1106_phy_glb_reg_bank_base[HI1106_RX_CHN_EST_REG_BANK_BUTT];

#define HI1106_MAC_CFG_HOST_REG_BASE             ((uint32_t)&g_st_host_hi1106_mac_cfg_host_reg_base[0]) /* 06ST使用 */
#define HI1106_MAC_RPT_HOST_REG_BASE             ((uint32_t)&g_st_host_hi1106_mac_rpt_host_reg_base[0])
#define HI1106_DFX_REG_BANK_BASE                 ((uint32_t)&g_st_host_hi1106_dfx_reg_bank_base[0])
#define HI1106_MAC_CFG_DMAC_BANK0_REG_BASE       ((uint32_t)&g_st_host_hi1106_mac_cfg_dmac_bank0_reg_base[0])
#define HI1106_MAC_RPT_DMAC_REG_BASE             ((uint32_t)&g_st_host_hi1106_mac_rpt_dmac_reg_base[0])
#define HI1106_MAC_CFG_DMAC_BANK4_REG_BASE       ((uint32_t)&g_st_host_hi1106_mac_cfg_dmac_bank4_reg_base[0])
#define HI1106_RX_CHN_EST_REG_BANK_BASE          ((uint32_t)&g_st_host_hi1106_rx_chn_est_reg_band_base[0])
#define HI1106_RF_CTRL_REG_BANK_0_BASE           ((uint32_t)&hi1106_rf_ctrl_reg_bank_0_base[0])
#define HI1106_CALI_TEST_REG_BANK_0_BASE         ((uint32_t)&hi1106_cali_test_reg_bank_0_base[0])
#define HI1106_PHY_GLB_REG_BANK_BASE             ((uint32_t)&hi1106_phy_glb_reg_bank_base[0])
/* _PRE_TEST_MODE_ST */
#else

#define HI1106_MAC_CFG_HOST_REG_BASE                        0x40020000
#define HI1106_MAC_RPT_HOST_REG_BASE                        0x40030000
#define HI1106_DFX_REG_BANK_BASE                            0x40064000
#define HI1106_MAC_CFG_DMAC_BANK0_REG_BASE                  0x40022000
#define HI1106_MAC_RPT_DMAC_REG_BASE                        0x40032000
#define HI1106_MAC_CFG_DMAC_BANK4_REG_BASE                  0x4002A000
#define HI1106_RX_CHN_EST_REG_BANK_BASE                     0x40072800
#define HI1106_RF_CTRL_REG_BANK_0_BASE                      0x40064800
#define HI1106_CALI_TEST_REG_BANK_0_BASE                    0x40061000
#define HI1106_PHY_GLB_REG_BANK_BASE                        0x40060800

/* != _PRE_TEST_MODE_ST */
#endif

/* PHY 寄存器 */
#define HI1106_MAC_HOST_INTR_CLR_REG                        (HI1106_MAC_CFG_HOST_REG_BASE + 0x50)
#define HI1106_MAC_HOST_INTR_MASK_REG                       (HI1106_MAC_CFG_HOST_REG_BASE + 0x54)
#define HI1106_DFX_REG_BANK_PHY_INTR_RPT_REG                (HI1106_DFX_REG_BANK_BASE + 0xBC)     /* PHY中断状态 */
#define HI1106_DFX_REG_BANK_CFG_INTR_MASK_HOST_REG          (HI1106_DFX_REG_BANK_BASE + 0xB0)
#define HI1106_DFX_REG_BANK_CFG_INTR_CLR_REG                (HI1106_DFX_REG_BANK_BASE + 0xB8)
#define HI1106_PHY_GLB_REG_BANK_FTM_CFG_REG                 (HI1106_PHY_GLB_REG_BANK_BASE + 0x38) /* FTM配置寄存器 */
/* 信道估计通用寄存器 */
#define HI1106_RX_CHN_EST_REG_BANK_CHN_EST_COM_REG          (HI1106_RX_CHN_EST_REG_BANK_BASE + 0x1B0)
#define HI1106_RF_CTRL_REG_BANK_0_CFG_ONLINE_CALI_USE_RF_REG_EN_REG     (HI1106_RF_CTRL_REG_BANK_0_BASE + 0x34)
/* FIFO使能对应的寄存器配置地址 */
#define HI1106_CALI_TEST_REG_BANK_0_FIFO_FORCE_EN_REG       (HI1106_CALI_TEST_REG_BANK_0_BASE + 0x28)
#define HI1106_PHY_GLB_REG_BANK_PHY_BW_MODE_REG             (HI1106_PHY_GLB_REG_BANK_BASE + 0x4) /* 频宽模式配置寄存器 */


#define HI1106_MAC_HOST_INTR_STATUS_REG                     (HI1106_MAC_RPT_HOST_REG_BASE + 0x0)  /* Host的中断状态 */
/* BA INFO BUFFER的写指针上报 */
#define HI1106_MAC_BA_INFO_BUF_WPTR_REG                     (HI1106_MAC_RPT_HOST_REG_BASE + 0x4)
#define HI1106_MAC_RX_NORM_DATA_FREE_RING_RPTR_REG          (HI1106_MAC_RPT_HOST_REG_BASE + 0x8)  /* rx-ndata-frrptr */
#define HI1106_MAC_RX_SMALL_DATA_FREE_RING_RPTR_REG         (HI1106_MAC_RPT_HOST_REG_BASE + 0xC)  /* rx-sdata-frrptr */
#define HI1106_MAC_RX_DATA_CMP_RING_WPTR_REG                (HI1106_MAC_RPT_HOST_REG_BASE + 0x10)
#define HI1106_MAC_RPT_HOST_FREE_RING_STATUS_REG            (HI1106_MAC_RPT_HOST_REG_BASE + 0x14) /* F RING上报信息 */
#define HI1106_MAC_RX_PPDU_HOST_DESC_FREE_RING_RPTR_REG     (HI1106_MAC_RPT_HOST_REG_BASE + 0x18)
#define HI1106_MAC_RPT_PPDU_HOST_FREE_RING_STATUS_REG       (HI1106_MAC_RPT_HOST_REG_BASE + 0x1C)
/* TX BA INFO BUFFER深度配置寄存器 */
#define HI1106_MAC_TX_BA_INFO_BUF_DEPTH_REG                 (HI1106_MAC_CFG_HOST_REG_BASE + 0x0)
/* TX BA INFO BUFFER基地址配置寄存器 */
#define HI1106_MAC_TX_BA_INFO_BUF_ADDR_LSB_REG              (HI1106_MAC_CFG_HOST_REG_BASE + 0x4)
/* TX BA INFO BUFFER基地址配置寄存器 */
#define HI1106_MAC_TX_BA_INFO_BUF_ADDR_MSB_REG              (HI1106_MAC_CFG_HOST_REG_BASE + 0x8)
/* TX BA INFO WPTR BUFFER基地址配置寄存器 */
#define HI1106_MAC_TX_BA_INFO_WPTR_REG                      (HI1106_MAC_CFG_HOST_REG_BASE + 0xC)
/* TX BA INFO RPTR BUFFER基地址配置寄存器 */
#define HI1106_MAC_TX_BA_INFO_RPTR_REG                      (HI1106_MAC_CFG_HOST_REG_BASE + 0x10)
#define HI1106_MAC_RX_DATA_BUFF_LEN_REG                     (HI1106_MAC_CFG_HOST_REG_BASE + 0x1C)
#define HI1106_MAC_RX_NORM_DATA_FREE_RING_ADDR_LSB_REG      (HI1106_MAC_CFG_HOST_REG_BASE + 0x20)
#define HI1106_MAC_RX_NORM_DATA_FREE_RING_ADDR_MSB_REG      (HI1106_MAC_CFG_HOST_REG_BASE + 0x24)
#define HI1106_MAC_RX_SMALL_DATA_FREE_RING_ADDR_LSB_REG     (HI1106_MAC_CFG_HOST_REG_BASE + 0x28)
#define HI1106_MAC_RX_SMALL_DATA_FREE_RING_ADDR_MSB_REG     (HI1106_MAC_CFG_HOST_REG_BASE + 0x2C)
#define HI1106_MAC_RX_DATA_CMP_RING_ADDR_LSB_REG            (HI1106_MAC_CFG_HOST_REG_BASE + 0x30)
#define HI1106_MAC_RX_DATA_CMP_RING_ADDR_MSB_REG            (HI1106_MAC_CFG_HOST_REG_BASE + 0x34)
#define HI1106_MAC_RX_NORM_DATA_FREE_RING_SIZE_REG          (HI1106_MAC_CFG_HOST_REG_BASE + 0x38)
#define HI1106_MAC_RX_SMALL_DATA_FREE_RING_SIZE_REG         (HI1106_MAC_CFG_HOST_REG_BASE + 0x3C)
#define HI1106_MAC_RX_DATA_CMP_RING_SIZE_REG                (HI1106_MAC_CFG_HOST_REG_BASE + 0x40)
#define HI1106_MAC_RX_NORM_DATA_FREE_RING_WPTR_REG          (HI1106_MAC_CFG_HOST_REG_BASE + 0x44)
#define HI1106_MAC_RX_SMALL_DATA_FREE_RING_WPTR_REG         (HI1106_MAC_CFG_HOST_REG_BASE + 0x48)
#define HI1106_MAC_RX_DATA_CMP_RING_RPTR_REG                (HI1106_MAC_CFG_HOST_REG_BASE + 0x4C)
#define HI1106_MAC_HOST_INTR_CLR_REG                        (HI1106_MAC_CFG_HOST_REG_BASE + 0x50) /* Host中断清除 */
#define HI1106_MAC_HOST_INTR_MASK_REG                       (HI1106_MAC_CFG_HOST_REG_BASE + 0x54) /* Host中断屏蔽 */
#define HI1106_MAC_RX_DATA_CMP_RING_WPTR_CFG_REG            (HI1106_MAC_CFG_HOST_REG_BASE + 0x58)
#define HI1106_MAC_RX_NORM_DATA_FREE_RING_RPTR_CFG_REG      (HI1106_MAC_CFG_HOST_REG_BASE + 0x5C)
#define HI1106_MAC_RX_SMALL_DATA_FREE_RING_RPTR_CFG_REG     (HI1106_MAC_CFG_HOST_REG_BASE + 0x60)
#define HI1106_MAC_RX_FRAMEFILT_REG                         (HI1106_MAC_CFG_DMAC_BANK0_REG_BASE + 0x54)  /* RX过滤控制寄存器 */
#define HI1106_MAC_RX_FRAMEFILT2_REG                        (HI1106_MAC_CFG_DMAC_BANK0_REG_BASE + 0x58)  /* RX过滤控制寄存器2 */

#define HI1106_MAC_DMAC_INTR_MASK_REG                       (HI1106_MAC_CFG_DMAC_BANK0_REG_BASE + 0x4)   /* DMAC中断屏蔽寄存器 */
#define HI1106_MAC_RX_CTRL_REG                              (HI1106_MAC_CFG_DMAC_BANK0_REG_BASE + 0x20)  /* 接收控制寄存器 */
#define HI1106_MAC_CSI_PROCESS_REG                          (HI1106_MAC_CFG_DMAC_BANK4_REG_BASE + 0x0)   /* CSI采样上报配置寄存器 */
#define HI1106_MAC_CSI_BUF_BASE_ADDR_LSB_REG                (HI1106_MAC_CFG_DMAC_BANK4_REG_BASE + 0x4)   /* CSI采样上报片外循环BUF基地址LSB */
#define HI1106_MAC_CSI_BUF_BASE_ADDR_MSB_REG                (HI1106_MAC_CFG_DMAC_BANK4_REG_BASE + 0x8)   /* CSI采样上报片外循环BUF基地址MSB */
#define HI1106_MAC_CSI_BUF_SIZE_REG                         (HI1106_MAC_CFG_DMAC_BANK4_REG_BASE + 0xC)   /* CSI采样上报片外循环BUF大小 */
#define HI1106_MAC_CSI_WHITELIST_ADDR_LSB_0_REG             (HI1106_MAC_CFG_DMAC_BANK4_REG_BASE + 0x10)  /* CSI采样白名单低32位 */
#define HI1106_MAC_CSI_WHITELIST_ADDR_MSB_0_REG             (HI1106_MAC_CFG_DMAC_BANK4_REG_BASE + 0x14)  /* CSI采样白名单高16位 */

#define HI1106_MAC_LOCATION_INFO_MASK_REG                   (HI1106_MAC_RPT_DMAC_REG_BASE + 0x70)  /* 定位信息上报 */
#define HI1106_MAC_CSI_INFO_ADDR_MSB_REG                    (HI1106_MAC_RPT_DMAC_REG_BASE + 0x1B0) /* CSI上报地址高32bit */
#define HI1106_MAC_CSI_INFO_ADDR_LSB_REG                    (HI1106_MAC_RPT_DMAC_REG_BASE + 0x1B4) /* CSI上报地址低32bit */
#define HI1106_MAC_FTM_INFO_ADDR_MSB_REG                    (HI1106_MAC_RPT_DMAC_REG_BASE + 0x1B8) /* FTM上报地址高32bit */
#define HI1106_MAC_FTM_INFO_ADDR_LSB_REG                    (HI1106_MAC_RPT_DMAC_REG_BASE + 0x1BC) /* FTM上报地址低32bit */

/* FTM采样上报配置寄存器 */
#define HI1106_MAC_FTM_PROCESS_REG                          (HI1106_MAC_CFG_DMAC_BANK4_REG_BASE + 0x110)
/* FTM采样上报片外循环BUF基地址LSB */
#define HI1106_MAC_FTM_BUF_BASE_ADDR_LSB_REG                (HI1106_MAC_CFG_DMAC_BANK4_REG_BASE + 0x114)
/* FTM采样上报片外循环BUF基地址MSB */
#define HI1106_MAC_FTM_BUF_BASE_ADDR_MSB_REG                (HI1106_MAC_CFG_DMAC_BANK4_REG_BASE + 0x118)
/* FTM采样白名单低32位 */
#define HI1106_MAC_FTM_WHITELIST_ADDR_LSB_0_REG             (HI1106_MAC_CFG_DMAC_BANK4_REG_BASE + 0x12C)
/* FTM采样白名单高16位 */
#define HI1106_MAC_FTM_WHITELIST_ADDR_MSB_0_REG             (HI1106_MAC_CFG_DMAC_BANK4_REG_BASE + 0x130)

/* master: 0x4002_0000~0x4003_ffff slave: 0x4004_0000~0x4005_ffff */
#define HI1106_MAC_BANK_REG_OFFSET 0x20000
/* fpga 阶段暂未定主副偏移 后续修改适配 */
#define HI1106_PHY_BANK_REG_OFFSET 0x40000

#define HI1106_MAC_DMAC_COMMON_TIMER_MASK_REG       (HI1106_MAC_CFG_DMAC_BANK0_REG_BASE + 0x124) /* DMAC通用计数器中断屏蔽寄存器 */
#define HI1106_MAC_COMMON_TIMER_CTRL_0_REG          (HI1106_MAC_CFG_DMAC_BANK0_REG_BASE + 0x128) /* 通用定时器相关参数配置 */
#define HI1106_MAC_COMMON_TIMER_VAL_0_REG           (HI1106_MAC_CFG_DMAC_BANK0_REG_BASE + 0x1A8) /* 通用定时器定时值配置 */

#define HI1106_MAC_CFG_SMAC_REG_BASE          (0x4002E000)
#define HI1106_MAC_CFG_DMAC_BANK2_REG_BASE    (0x40026000)
#define HI1106_MAC_HWMAC_STARTUP_REG          (HI1106_MAC_CFG_SMAC_REG_BASE + 0x140) /* HWMAC开关配置寄存器 */
#define HI1106_MAC_COEX_ABORT_CTRL_REG        (HI1106_MAC_CFG_DMAC_BANK2_REG_BASE + 0x300) /* 共存控制寄存器 */
#define HI1106_MAC_CFG_COEX_ABORT_EN_MASK      0x1
#define HI1106_MAC_CFG_HWMAC_STARTUP_EN_MASK   0x1
#define HI1106_MAC_COMMON_TIMER_OFFSET         0x4

#define HI1106_MAC_MAX_COMMON_TIMER 31 /* 通用定时器个数 */

void  hi1106_host_mac_irq_mask(hal_host_device_stru *hal_device, uint32_t irq);
void  hi1106_host_mac_irq_unmask(hal_host_device_stru *hal_device, uint32_t irq);
void hi1106_clear_host_mac_int_status(hal_host_device_stru *hal_device, uint32_t status);
void hi1106_get_host_mac_int_mask(hal_host_device_stru *hal_device, uint32_t *p_mask);
void hi1106_get_host_mac_int_status(hal_host_device_stru *hal_device, uint32_t *p_status);
uint32_t hi1106_regs_addr_transfer(hal_host_device_stru *hal_device, uint32_t reg_addr);
void hi1106_clear_host_phy_int_status(hal_host_device_stru *hal_device, uint32_t status);
void hi1106_get_host_phy_int_mask(hal_host_device_stru *hal_device, uint32_t *p_mask);
void hi1106_get_host_phy_int_status(hal_host_device_stru *hal_device, uint32_t *p_status);
int32_t hi1106_host_regs_addr_init(hal_host_device_stru *hal_device);
uint32_t hi1106_get_dev_rx_filt_mac_status(uint64_t *reg_status);
uint32_t hi1106_regs_addr_get_offset(uint8_t device_id, uint32_t reg_addr);
#endif
