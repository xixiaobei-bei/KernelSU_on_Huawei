

#ifndef _HI1106_HOST_CTRL_RB_REGS_H_
#define _HI1106_HOST_CTRL_RB_REGS_H_

/* This file is generated automatically. Do not modify it */
#define HOST_CTRL_RB_BASE    (0x40113000)

/* SYS CTL ID寄存器 0x000 */
typedef union {
    struct {
        unsigned int host_ctrl_sys_ctl_id : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_host_ctrl_sys_ctl_id;
#define HOST_CTRL_RB_HOST_CTRL_SYS_CTL_ID_OFF    0x000
#define HOST_CTRL_RB_HOST_CTRL_SYS_CTL_ID_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_HOST_CTRL_SYS_CTL_ID_OFF)

/* 通用寄存器 0x0010 */
typedef union {
    struct {
        unsigned int host_ctrl_gp_reg0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_host_ctrl_gp_reg0;
#define HOST_CTRL_RB_HOST_CTRL_GP_REG0_OFF    0x0010
#define HOST_CTRL_RB_HOST_CTRL_GP_REG0_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_HOST_CTRL_GP_REG0_OFF)

/* 通用寄存器 0x0014 */
typedef union {
    struct {
        unsigned int host_ctrl_gp_reg1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_host_ctrl_gp_reg1;
#define HOST_CTRL_RB_HOST_CTRL_GP_REG1_OFF    0x0014
#define HOST_CTRL_RB_HOST_CTRL_GP_REG1_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_HOST_CTRL_GP_REG1_OFF)

/* 测试寄存器 0x100 */
typedef union {
    struct {
        unsigned int host_ctrl_test_value : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_host_ctrl_test_value;
#define HOST_CTRL_RB_HOST_CTRL_TEST_VALUE_OFF    0x100
#define HOST_CTRL_RB_HOST_CTRL_TEST_VALUE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_HOST_CTRL_TEST_VALUE_OFF)

/* 测试结果寄存器 0x104 */
typedef union {
    struct {
        unsigned int host_ctrl_test_result : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_host_ctrl_test_result;
#define HOST_CTRL_RB_HOST_CTRL_TEST_RESULT_OFF    0x104
#define HOST_CTRL_RB_HOST_CTRL_TEST_RESULT_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_HOST_CTRL_TEST_RESULT_OFF)

/* TX通道0配置 0x200 */
typedef union {
    struct {
        unsigned int ete_clk_en : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_ete_clk_en;
#define HOST_CTRL_RB_ETE_CLK_EN_OFF    0x200
#define HOST_CTRL_RB_ETE_CLK_EN_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_ETE_CLK_EN_OFF)

/* LMI配置 0x300 */
typedef union {
    struct {
        unsigned int wch3_qos : 2; /* 0:1 */
        unsigned int wch3_pri : 2; /* 2:3 */
        unsigned int wch2_qos : 2; /* 4:5 */
        unsigned int wch2_pri : 2; /* 6:7 */
        unsigned int wch1_qos : 2; /* 8:9 */
        unsigned int wch1_pri : 2; /* 10:11 */
        unsigned int wch0_qos : 2; /* 12:13 */
        unsigned int wch0_pri : 2; /* 14:15 */
        unsigned int rch3_qos : 2; /* 16:17 */
        unsigned int rch3_pri : 2; /* 18:19 */
        unsigned int rch2_qos : 2; /* 20:21 */
        unsigned int rch2_pri : 2; /* 22:23 */
        unsigned int rch1_qos : 2; /* 24:25 */
        unsigned int rch1_pri : 2; /* 26:27 */
        unsigned int rch0_qos : 2; /* 28:29 */
        unsigned int rch0_pri : 2; /* 30:31 */
    } bits;
    unsigned int as_dword;
} hreg_lmi;
#define HOST_CTRL_RB_LMI_OFF    0x300
#define HOST_CTRL_RB_LMI_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_LMI_OFF)

/* TX通道0配置 0x400 */
typedef union {
    struct {
        unsigned int tx_ch0_en : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_en;
#define HOST_CTRL_RB_TX_CH0_EN_OFF    0x400
#define HOST_CTRL_RB_TX_CH0_EN_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_EN_OFF)

/* TX通道0配置 0x404 */
typedef union {
    struct {
        unsigned int tx_ch0_mode : 1; /* 0 */
        unsigned int tx_ch0_mcy_start : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_mode;
#define HOST_CTRL_RB_TX_CH0_MODE_OFF    0x404
#define HOST_CTRL_RB_TX_CH0_MODE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_MODE_OFF)

/* TX通道0配置 0x408 */
typedef union {
    struct {
        unsigned int tx_ch0_pri : 3; /* 0:2 */
        unsigned int tx_ch0_fc : 1; /* 3 */
        unsigned int reserved0 : 28; /* 4:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_pri;
#define HOST_CTRL_RB_TX_CH0_PRI_OFF    0x408
#define HOST_CTRL_RB_TX_CH0_PRI_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_PRI_OFF)

/* TX通道0配置 0x40C */
typedef union {
    struct {
        unsigned int tx_ch0_sr_base_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_sr_base_addr_h;
#define HOST_CTRL_RB_TX_CH0_SR_BASE_ADDR_H_OFF    0x40C
#define HOST_CTRL_RB_TX_CH0_SR_BASE_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_SR_BASE_ADDR_H_OFF)

/* TX通道0配置 0x410 */
typedef union {
    struct {
        unsigned int tx_ch0_sr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_sr_base_addr;
#define HOST_CTRL_RB_TX_CH0_SR_BASE_ADDR_OFF    0x410
#define HOST_CTRL_RB_TX_CH0_SR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_SR_BASE_ADDR_OFF)

/* TX通道0配置 0x414 */
typedef union {
    struct {
        unsigned int tx_ch0_sr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_sr_entry_num;
#define HOST_CTRL_RB_TX_CH0_SR_ENTRY_NUM_OFF    0x414
#define HOST_CTRL_RB_TX_CH0_SR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_SR_ENTRY_NUM_OFF)

/* TX通道0配置 0x418 */
typedef union {
    struct {
        unsigned int tx_ch0_sr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_sr_head_ptr;
#define HOST_CTRL_RB_TX_CH0_SR_HEAD_PTR_OFF    0x418
#define HOST_CTRL_RB_TX_CH0_SR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_SR_HEAD_PTR_OFF)

/* TX通道0配置 0x41C */
typedef union {
    struct {
        unsigned int tx_ch0_sr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_sr_tail_ptr;
#define HOST_CTRL_RB_TX_CH0_SR_TAIL_PTR_OFF    0x41C
#define HOST_CTRL_RB_TX_CH0_SR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_SR_TAIL_PTR_OFF)

/* TX通道0配置 0x420 */
typedef union {
    struct {
        unsigned int tx_ch0_sr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_sr_afull_th;
#define HOST_CTRL_RB_TX_CH0_SR_AFULL_TH_OFF    0x420
#define HOST_CTRL_RB_TX_CH0_SR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_SR_AFULL_TH_OFF)

/* TX通道0配置 0x424 */
typedef union {
    struct {
        unsigned int tx_ch0_sr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_sr_aempty_th;
#define HOST_CTRL_RB_TX_CH0_SR_AEMPTY_TH_OFF    0x424
#define HOST_CTRL_RB_TX_CH0_SR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_SR_AEMPTY_TH_OFF)

/* TX通道0配置 0x428 */
typedef union {
    struct {
        unsigned int tx_ch0_dr_buff_size : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_dr_buff_size;
#define HOST_CTRL_RB_TX_CH0_DR_BUFF_SIZE_OFF    0x428
#define HOST_CTRL_RB_TX_CH0_DR_BUFF_SIZE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_DR_BUFF_SIZE_OFF)

/* TX通道0配置 0x430 */
typedef union {
    struct {
        unsigned int tx_ch0_dr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_dr_base_addr;
#define HOST_CTRL_RB_TX_CH0_DR_BASE_ADDR_OFF    0x430
#define HOST_CTRL_RB_TX_CH0_DR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_DR_BASE_ADDR_OFF)

/* TX通道0配置 0x434 */
typedef union {
    struct {
        unsigned int tx_ch0_dr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_dr_entry_num;
#define HOST_CTRL_RB_TX_CH0_DR_ENTRY_NUM_OFF    0x434
#define HOST_CTRL_RB_TX_CH0_DR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_DR_ENTRY_NUM_OFF)

/* TX通道0配置 0x438 */
typedef union {
    struct {
        unsigned int tx_ch0_dr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_dr_head_ptr;
#define HOST_CTRL_RB_TX_CH0_DR_HEAD_PTR_OFF    0x438
#define HOST_CTRL_RB_TX_CH0_DR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_DR_HEAD_PTR_OFF)

/* TX通道0配置 0x43C */
typedef union {
    struct {
        unsigned int tx_ch0_dr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_dr_tail_ptr;
#define HOST_CTRL_RB_TX_CH0_DR_TAIL_PTR_OFF    0x43C
#define HOST_CTRL_RB_TX_CH0_DR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_DR_TAIL_PTR_OFF)

/* TX通道0配置 0x440 */
typedef union {
    struct {
        unsigned int tx_ch0_dr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_dr_afull_th;
#define HOST_CTRL_RB_TX_CH0_DR_AFULL_TH_OFF    0x440
#define HOST_CTRL_RB_TX_CH0_DR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_DR_AFULL_TH_OFF)

/* TX通道0配置 0x444 */
typedef union {
    struct {
        unsigned int tx_ch0_dr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_dr_aempty_th;
#define HOST_CTRL_RB_TX_CH0_DR_AEMPTY_TH_OFF    0x444
#define HOST_CTRL_RB_TX_CH0_DR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_DR_AEMPTY_TH_OFF)

/* TX通道0配置 0x448 */
typedef union {
    struct {
        unsigned int tx_ch0_status : 2; /* 0:1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_status;
#define HOST_CTRL_RB_TX_CH0_STATUS_OFF    0x448
#define HOST_CTRL_RB_TX_CH0_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_STATUS_OFF)

/* TX通道1配置 0x450 */
typedef union {
    struct {
        unsigned int tx_ch1_en : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_en;
#define HOST_CTRL_RB_TX_CH1_EN_OFF    0x450
#define HOST_CTRL_RB_TX_CH1_EN_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_EN_OFF)

/* TX通道1配置 0x454 */
typedef union {
    struct {
        unsigned int tx_ch1_mode : 1; /* 0 */
        unsigned int tx_ch1_mcy_start : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_mode;
#define HOST_CTRL_RB_TX_CH1_MODE_OFF    0x454
#define HOST_CTRL_RB_TX_CH1_MODE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_MODE_OFF)

/* TX通道1配置 0x458 */
typedef union {
    struct {
        unsigned int tx_ch1_pri : 3; /* 0:2 */
        unsigned int tx_ch1_fc : 1; /* 3 */
        unsigned int reserved0 : 28; /* 4:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_pri;
#define HOST_CTRL_RB_TX_CH1_PRI_OFF    0x458
#define HOST_CTRL_RB_TX_CH1_PRI_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_PRI_OFF)

/* TX通道1配置 0x45C */
typedef union {
    struct {
        unsigned int tx_ch1_sr_base_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_sr_base_addr_h;
#define HOST_CTRL_RB_TX_CH1_SR_BASE_ADDR_H_OFF    0x45C
#define HOST_CTRL_RB_TX_CH1_SR_BASE_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_SR_BASE_ADDR_H_OFF)

/* TX通道1配置 0x460 */
typedef union {
    struct {
        unsigned int tx_ch1_sr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_sr_base_addr;
#define HOST_CTRL_RB_TX_CH1_SR_BASE_ADDR_OFF    0x460
#define HOST_CTRL_RB_TX_CH1_SR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_SR_BASE_ADDR_OFF)

/* TX通道1配置 0x464 */
typedef union {
    struct {
        unsigned int tx_ch1_sr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_sr_entry_num;
#define HOST_CTRL_RB_TX_CH1_SR_ENTRY_NUM_OFF    0x464
#define HOST_CTRL_RB_TX_CH1_SR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_SR_ENTRY_NUM_OFF)

/* TX通道1配置 0x468 */
typedef union {
    struct {
        unsigned int tx_ch1_sr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_sr_head_ptr;
#define HOST_CTRL_RB_TX_CH1_SR_HEAD_PTR_OFF    0x468
#define HOST_CTRL_RB_TX_CH1_SR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_SR_HEAD_PTR_OFF)

/* TX通道1配置 0x46C */
typedef union {
    struct {
        unsigned int tx_ch1_sr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_sr_tail_ptr;
#define HOST_CTRL_RB_TX_CH1_SR_TAIL_PTR_OFF    0x46C
#define HOST_CTRL_RB_TX_CH1_SR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_SR_TAIL_PTR_OFF)

/* TX通道1配置 0x470 */
typedef union {
    struct {
        unsigned int tx_ch1_sr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_sr_afull_th;
#define HOST_CTRL_RB_TX_CH1_SR_AFULL_TH_OFF    0x470
#define HOST_CTRL_RB_TX_CH1_SR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_SR_AFULL_TH_OFF)

/* TX通道1配置 0x474 */
typedef union {
    struct {
        unsigned int tx_ch1_sr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_sr_aempty_th;
#define HOST_CTRL_RB_TX_CH1_SR_AEMPTY_TH_OFF    0x474
#define HOST_CTRL_RB_TX_CH1_SR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_SR_AEMPTY_TH_OFF)

/* TX通道0配置 0x478 */
typedef union {
    struct {
        unsigned int tx_ch1_dr_buff_size : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_dr_buff_size;
#define HOST_CTRL_RB_TX_CH1_DR_BUFF_SIZE_OFF    0x478
#define HOST_CTRL_RB_TX_CH1_DR_BUFF_SIZE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_DR_BUFF_SIZE_OFF)

/* TX通道1配置 0x480 */
typedef union {
    struct {
        unsigned int tx_ch1_dr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_dr_base_addr;
#define HOST_CTRL_RB_TX_CH1_DR_BASE_ADDR_OFF    0x480
#define HOST_CTRL_RB_TX_CH1_DR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_DR_BASE_ADDR_OFF)

/* TX通道1配置 0x484 */
typedef union {
    struct {
        unsigned int tx_ch1_dr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_dr_entry_num;
#define HOST_CTRL_RB_TX_CH1_DR_ENTRY_NUM_OFF    0x484
#define HOST_CTRL_RB_TX_CH1_DR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_DR_ENTRY_NUM_OFF)

/* TX通道1配置 0x488 */
typedef union {
    struct {
        unsigned int tx_ch1_dr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_dr_head_ptr;
#define HOST_CTRL_RB_TX_CH1_DR_HEAD_PTR_OFF    0x488
#define HOST_CTRL_RB_TX_CH1_DR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_DR_HEAD_PTR_OFF)

/* TX通道1配置 0x48C */
typedef union {
    struct {
        unsigned int tx_ch1_dr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_dr_tail_ptr;
#define HOST_CTRL_RB_TX_CH1_DR_TAIL_PTR_OFF    0x48C
#define HOST_CTRL_RB_TX_CH1_DR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_DR_TAIL_PTR_OFF)

/* TX通道1配置 0x490 */
typedef union {
    struct {
        unsigned int tx_ch1_dr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_dr_afull_th;
#define HOST_CTRL_RB_TX_CH1_DR_AFULL_TH_OFF    0x490
#define HOST_CTRL_RB_TX_CH1_DR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_DR_AFULL_TH_OFF)

/* TX通道1配置 0x494 */
typedef union {
    struct {
        unsigned int tx_ch1_dr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_dr_aempty_th;
#define HOST_CTRL_RB_TX_CH1_DR_AEMPTY_TH_OFF    0x494
#define HOST_CTRL_RB_TX_CH1_DR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_DR_AEMPTY_TH_OFF)

/* TX通道1配置 0x498 */
typedef union {
    struct {
        unsigned int tx_ch1_status : 2; /* 0:1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_status;
#define HOST_CTRL_RB_TX_CH1_STATUS_OFF    0x498
#define HOST_CTRL_RB_TX_CH1_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_STATUS_OFF)

/* TX通道2配置 0x4A0 */
typedef union {
    struct {
        unsigned int tx_ch2_en : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_en;
#define HOST_CTRL_RB_TX_CH2_EN_OFF    0x4A0
#define HOST_CTRL_RB_TX_CH2_EN_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_EN_OFF)

/* TX通道2配置 0x4A4 */
typedef union {
    struct {
        unsigned int tx_ch2_mode : 1; /* 0 */
        unsigned int tx_ch2_mcy_start : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_mode;
#define HOST_CTRL_RB_TX_CH2_MODE_OFF    0x4A4
#define HOST_CTRL_RB_TX_CH2_MODE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_MODE_OFF)

/* TX通道2配置 0x4A8 */
typedef union {
    struct {
        unsigned int tx_ch2_pri : 3; /* 0:2 */
        unsigned int tx_ch2_fc : 1; /* 3 */
        unsigned int reserved0 : 28; /* 4:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_pri;
#define HOST_CTRL_RB_TX_CH2_PRI_OFF    0x4A8
#define HOST_CTRL_RB_TX_CH2_PRI_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_PRI_OFF)

/* TX通道2配置 0x4AC */
typedef union {
    struct {
        unsigned int tx_ch2_sr_base_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_sr_base_addr_h;
#define HOST_CTRL_RB_TX_CH2_SR_BASE_ADDR_H_OFF    0x4AC
#define HOST_CTRL_RB_TX_CH2_SR_BASE_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_SR_BASE_ADDR_H_OFF)

/* TX通道2配置 0x4B0 */
typedef union {
    struct {
        unsigned int tx_ch2_sr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_sr_base_addr;
#define HOST_CTRL_RB_TX_CH2_SR_BASE_ADDR_OFF    0x4B0
#define HOST_CTRL_RB_TX_CH2_SR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_SR_BASE_ADDR_OFF)

/* TX通道2配置 0x4B4 */
typedef union {
    struct {
        unsigned int tx_ch2_sr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_sr_entry_num;
#define HOST_CTRL_RB_TX_CH2_SR_ENTRY_NUM_OFF    0x4B4
#define HOST_CTRL_RB_TX_CH2_SR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_SR_ENTRY_NUM_OFF)

/* TX通道2配置 0x4B8 */
typedef union {
    struct {
        unsigned int tx_ch2_sr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_sr_head_ptr;
#define HOST_CTRL_RB_TX_CH2_SR_HEAD_PTR_OFF    0x4B8
#define HOST_CTRL_RB_TX_CH2_SR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_SR_HEAD_PTR_OFF)

/* TX通道2配置 0x4BC */
typedef union {
    struct {
        unsigned int tx_ch2_sr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_sr_tail_ptr;
#define HOST_CTRL_RB_TX_CH2_SR_TAIL_PTR_OFF    0x4BC
#define HOST_CTRL_RB_TX_CH2_SR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_SR_TAIL_PTR_OFF)

/* TX通道2配置 0x4C0 */
typedef union {
    struct {
        unsigned int tx_ch2_sr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_sr_afull_th;
#define HOST_CTRL_RB_TX_CH2_SR_AFULL_TH_OFF    0x4C0
#define HOST_CTRL_RB_TX_CH2_SR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_SR_AFULL_TH_OFF)

/* TX通道2配置 0x4C4 */
typedef union {
    struct {
        unsigned int tx_ch2_sr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_sr_aempty_th;
#define HOST_CTRL_RB_TX_CH2_SR_AEMPTY_TH_OFF    0x4C4
#define HOST_CTRL_RB_TX_CH2_SR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_SR_AEMPTY_TH_OFF)

/* TX通道0配置 0x4C8 */
typedef union {
    struct {
        unsigned int tx_ch2_dr_buff_size : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_dr_buff_size;
#define HOST_CTRL_RB_TX_CH2_DR_BUFF_SIZE_OFF    0x4C8
#define HOST_CTRL_RB_TX_CH2_DR_BUFF_SIZE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_DR_BUFF_SIZE_OFF)

/* TX通道2配置 0x4D0 */
typedef union {
    struct {
        unsigned int tx_ch2_dr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_dr_base_addr;
#define HOST_CTRL_RB_TX_CH2_DR_BASE_ADDR_OFF    0x4D0
#define HOST_CTRL_RB_TX_CH2_DR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_DR_BASE_ADDR_OFF)

/* TX通道2配置 0x4D4 */
typedef union {
    struct {
        unsigned int tx_ch2_dr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_dr_entry_num;
#define HOST_CTRL_RB_TX_CH2_DR_ENTRY_NUM_OFF    0x4D4
#define HOST_CTRL_RB_TX_CH2_DR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_DR_ENTRY_NUM_OFF)

/* TX通道2配置 0x4D8 */
typedef union {
    struct {
        unsigned int tx_ch2_dr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_dr_head_ptr;
#define HOST_CTRL_RB_TX_CH2_DR_HEAD_PTR_OFF    0x4D8
#define HOST_CTRL_RB_TX_CH2_DR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_DR_HEAD_PTR_OFF)

/* TX通道2配置 0x4DC */
typedef union {
    struct {
        unsigned int tx_ch2_dr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_dr_tail_ptr;
#define HOST_CTRL_RB_TX_CH2_DR_TAIL_PTR_OFF    0x4DC
#define HOST_CTRL_RB_TX_CH2_DR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_DR_TAIL_PTR_OFF)

/* TX通道2配置 0x4E0 */
typedef union {
    struct {
        unsigned int tx_ch2_dr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_dr_afull_th;
#define HOST_CTRL_RB_TX_CH2_DR_AFULL_TH_OFF    0x4E0
#define HOST_CTRL_RB_TX_CH2_DR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_DR_AFULL_TH_OFF)

/* TX通道2配置 0x4E4 */
typedef union {
    struct {
        unsigned int tx_ch2_dr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_dr_aempty_th;
#define HOST_CTRL_RB_TX_CH2_DR_AEMPTY_TH_OFF    0x4E4
#define HOST_CTRL_RB_TX_CH2_DR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_DR_AEMPTY_TH_OFF)

/* TX通道2配置 0x4E8 */
typedef union {
    struct {
        unsigned int tx_ch2_status : 2; /* 0:1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_status;
#define HOST_CTRL_RB_TX_CH2_STATUS_OFF    0x4E8
#define HOST_CTRL_RB_TX_CH2_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_STATUS_OFF)

/* TX通道3配置 0x4F0 */
typedef union {
    struct {
        unsigned int tx_ch3_en : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_en;
#define HOST_CTRL_RB_TX_CH3_EN_OFF    0x4F0
#define HOST_CTRL_RB_TX_CH3_EN_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_EN_OFF)

/* TX通道3配置 0x4F4 */
typedef union {
    struct {
        unsigned int tx_ch3_mode : 1; /* 0 */
        unsigned int tx_ch3_mcy_start : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_mode;
#define HOST_CTRL_RB_TX_CH3_MODE_OFF    0x4F4
#define HOST_CTRL_RB_TX_CH3_MODE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_MODE_OFF)

/* TX通道3配置 0x4F8 */
typedef union {
    struct {
        unsigned int tx_ch3_pri : 3; /* 0:2 */
        unsigned int tx_ch3_fc : 1; /* 3 */
        unsigned int reserved0 : 28; /* 4:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_pri;
#define HOST_CTRL_RB_TX_CH3_PRI_OFF    0x4F8
#define HOST_CTRL_RB_TX_CH3_PRI_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_PRI_OFF)

/* TX通道3配置 0x4FC */
typedef union {
    struct {
        unsigned int tx_ch3_sr_base_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_sr_base_addr_h;
#define HOST_CTRL_RB_TX_CH3_SR_BASE_ADDR_H_OFF    0x4FC
#define HOST_CTRL_RB_TX_CH3_SR_BASE_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_SR_BASE_ADDR_H_OFF)

/* TX通道3配置 0x500 */
typedef union {
    struct {
        unsigned int tx_ch3_sr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_sr_base_addr;
#define HOST_CTRL_RB_TX_CH3_SR_BASE_ADDR_OFF    0x500
#define HOST_CTRL_RB_TX_CH3_SR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_SR_BASE_ADDR_OFF)

/* TX通道3配置 0x504 */
typedef union {
    struct {
        unsigned int tx_ch3_sr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_sr_entry_num;
#define HOST_CTRL_RB_TX_CH3_SR_ENTRY_NUM_OFF    0x504
#define HOST_CTRL_RB_TX_CH3_SR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_SR_ENTRY_NUM_OFF)

/* TX通道3配置 0x508 */
typedef union {
    struct {
        unsigned int tx_ch3_sr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_sr_head_ptr;
#define HOST_CTRL_RB_TX_CH3_SR_HEAD_PTR_OFF    0x508
#define HOST_CTRL_RB_TX_CH3_SR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_SR_HEAD_PTR_OFF)

/* TX通道3配置 0x50C */
typedef union {
    struct {
        unsigned int tx_ch3_sr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_sr_tail_ptr;
#define HOST_CTRL_RB_TX_CH3_SR_TAIL_PTR_OFF    0x50C
#define HOST_CTRL_RB_TX_CH3_SR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_SR_TAIL_PTR_OFF)

/* TX通道3配置 0x510 */
typedef union {
    struct {
        unsigned int tx_ch3_sr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_sr_afull_th;
#define HOST_CTRL_RB_TX_CH3_SR_AFULL_TH_OFF    0x510
#define HOST_CTRL_RB_TX_CH3_SR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_SR_AFULL_TH_OFF)

/* TX通道3配置 0x514 */
typedef union {
    struct {
        unsigned int tx_ch3_sr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_sr_aempty_th;
#define HOST_CTRL_RB_TX_CH3_SR_AEMPTY_TH_OFF    0x514
#define HOST_CTRL_RB_TX_CH3_SR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_SR_AEMPTY_TH_OFF)

/* TX通道0配置 0x518 */
typedef union {
    struct {
        unsigned int tx_ch3_dr_buff_size : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_dr_buff_size;
#define HOST_CTRL_RB_TX_CH3_DR_BUFF_SIZE_OFF    0x518
#define HOST_CTRL_RB_TX_CH3_DR_BUFF_SIZE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_DR_BUFF_SIZE_OFF)

/* TX通道3配置 0x520 */
typedef union {
    struct {
        unsigned int tx_ch3_dr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_dr_base_addr;
#define HOST_CTRL_RB_TX_CH3_DR_BASE_ADDR_OFF    0x520
#define HOST_CTRL_RB_TX_CH3_DR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_DR_BASE_ADDR_OFF)

/* TX通道3配置 0x524 */
typedef union {
    struct {
        unsigned int tx_ch3_dr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_dr_entry_num;
#define HOST_CTRL_RB_TX_CH3_DR_ENTRY_NUM_OFF    0x524
#define HOST_CTRL_RB_TX_CH3_DR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_DR_ENTRY_NUM_OFF)

/* TX通道3配置 0x528 */
typedef union {
    struct {
        unsigned int tx_ch3_dr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_dr_head_ptr;
#define HOST_CTRL_RB_TX_CH3_DR_HEAD_PTR_OFF    0x528
#define HOST_CTRL_RB_TX_CH3_DR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_DR_HEAD_PTR_OFF)

/* TX通道3配置 0x52C */
typedef union {
    struct {
        unsigned int tx_ch3_dr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_dr_tail_ptr;
#define HOST_CTRL_RB_TX_CH3_DR_TAIL_PTR_OFF    0x52C
#define HOST_CTRL_RB_TX_CH3_DR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_DR_TAIL_PTR_OFF)

/* TX通道3配置 0x530 */
typedef union {
    struct {
        unsigned int tx_ch3_dr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_dr_afull_th;
#define HOST_CTRL_RB_TX_CH3_DR_AFULL_TH_OFF    0x530
#define HOST_CTRL_RB_TX_CH3_DR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_DR_AFULL_TH_OFF)

/* TX通道3配置 0x534 */
typedef union {
    struct {
        unsigned int tx_ch3_dr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_dr_aempty_th;
#define HOST_CTRL_RB_TX_CH3_DR_AEMPTY_TH_OFF    0x534
#define HOST_CTRL_RB_TX_CH3_DR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_DR_AEMPTY_TH_OFF)

/* TX通道3配置 0x538 */
typedef union {
    struct {
        unsigned int tx_ch3_status : 2; /* 0:1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_status;
#define HOST_CTRL_RB_TX_CH3_STATUS_OFF    0x538
#define HOST_CTRL_RB_TX_CH3_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_STATUS_OFF)

/* TX通道4配置 0x540 */
typedef union {
    struct {
        unsigned int tx_ch4_en : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch4_en;
#define HOST_CTRL_RB_TX_CH4_EN_OFF    0x540
#define HOST_CTRL_RB_TX_CH4_EN_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH4_EN_OFF)

/* TX通道4配置 0x544 */
typedef union {
    struct {
        unsigned int tx_ch4_mode : 1; /* 0 */
        unsigned int tx_ch4_mcy_start : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch4_mode;
#define HOST_CTRL_RB_TX_CH4_MODE_OFF    0x544
#define HOST_CTRL_RB_TX_CH4_MODE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH4_MODE_OFF)

/* TX通道4配置 0x548 */
typedef union {
    struct {
        unsigned int tx_ch4_pri : 3; /* 0:2 */
        unsigned int tx_ch4_fc : 1; /* 3 */
        unsigned int reserved0 : 28; /* 4:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch4_pri;
#define HOST_CTRL_RB_TX_CH4_PRI_OFF    0x548
#define HOST_CTRL_RB_TX_CH4_PRI_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH4_PRI_OFF)

/* TX通道4配置 0x588 */
typedef union {
    struct {
        unsigned int tx_ch4_status : 2; /* 0:1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch4_status;
#define HOST_CTRL_RB_TX_CH4_STATUS_OFF    0x588
#define HOST_CTRL_RB_TX_CH4_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH4_STATUS_OFF)

/* RX通道0配置 0x590 */
typedef union {
    struct {
        unsigned int rx_ch0_en : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_en;
#define HOST_CTRL_RB_RX_CH0_EN_OFF    0x590
#define HOST_CTRL_RB_RX_CH0_EN_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_EN_OFF)

/* RX通道0配置 0x594 */
typedef union {
    struct {
        unsigned int rx_ch0_mode : 1; /* 0 */
        unsigned int rx_ch0_mcy_start : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_mode;
#define HOST_CTRL_RB_RX_CH0_MODE_OFF    0x594
#define HOST_CTRL_RB_RX_CH0_MODE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_MODE_OFF)

/* RX通道0配置 0x598 */
typedef union {
    struct {
        unsigned int rx_ch0_pri : 3; /* 0:2 */
        unsigned int reserved0 : 29; /* 3:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_pri;
#define HOST_CTRL_RB_RX_CH0_PRI_OFF    0x598
#define HOST_CTRL_RB_RX_CH0_PRI_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_PRI_OFF)

/* RX通道0配置 0x59C */
typedef union {
    struct {
        unsigned int rx_ch0_fc : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_fc;
#define HOST_CTRL_RB_RX_CH0_FC_OFF    0x59C
#define HOST_CTRL_RB_RX_CH0_FC_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_FC_OFF)

/* RX通道0配置 0x5A0 */
typedef union {
    struct {
        unsigned int rx_ch0_sr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_sr_base_addr;
#define HOST_CTRL_RB_RX_CH0_SR_BASE_ADDR_OFF    0x5A0
#define HOST_CTRL_RB_RX_CH0_SR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_SR_BASE_ADDR_OFF)

/* RX通道0配置 0x5A4 */
typedef union {
    struct {
        unsigned int rx_ch0_sr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_sr_entry_num;
#define HOST_CTRL_RB_RX_CH0_SR_ENTRY_NUM_OFF    0x5A4
#define HOST_CTRL_RB_RX_CH0_SR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_SR_ENTRY_NUM_OFF)

/* RX通道0配置 0x5A8 */
typedef union {
    struct {
        unsigned int rx_ch0_sr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_sr_head_ptr;
#define HOST_CTRL_RB_RX_CH0_SR_HEAD_PTR_OFF    0x5A8
#define HOST_CTRL_RB_RX_CH0_SR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_SR_HEAD_PTR_OFF)

/* RX通道0配置 0x5AC */
typedef union {
    struct {
        unsigned int rx_ch0_sr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_sr_tail_ptr;
#define HOST_CTRL_RB_RX_CH0_SR_TAIL_PTR_OFF    0x5AC
#define HOST_CTRL_RB_RX_CH0_SR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_SR_TAIL_PTR_OFF)

/* RX通道0配置 0x5B0 */
typedef union {
    struct {
        unsigned int rx_ch0_sr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_sr_afull_th;
#define HOST_CTRL_RB_RX_CH0_SR_AFULL_TH_OFF    0x5B0
#define HOST_CTRL_RB_RX_CH0_SR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_SR_AFULL_TH_OFF)

/* RX通道0配置 0x5B4 */
typedef union {
    struct {
        unsigned int rx_ch0_sr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_sr_aempty_th;
#define HOST_CTRL_RB_RX_CH0_SR_AEMPTY_TH_OFF    0x5B4
#define HOST_CTRL_RB_RX_CH0_SR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_SR_AEMPTY_TH_OFF)

/* RX通道0配置 0x5BC */
typedef union {
    struct {
        unsigned int rx_ch0_dr_base_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_dr_base_addr_h;
#define HOST_CTRL_RB_RX_CH0_DR_BASE_ADDR_H_OFF    0x5BC
#define HOST_CTRL_RB_RX_CH0_DR_BASE_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_DR_BASE_ADDR_H_OFF)

/* RX通道0配置 0x5C0 */
typedef union {
    struct {
        unsigned int rx_ch0_dr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_dr_base_addr;
#define HOST_CTRL_RB_RX_CH0_DR_BASE_ADDR_OFF    0x5C0
#define HOST_CTRL_RB_RX_CH0_DR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_DR_BASE_ADDR_OFF)

/* RX通道0配置 0x5C4 */
typedef union {
    struct {
        unsigned int rx_ch0_dr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_dr_entry_num;
#define HOST_CTRL_RB_RX_CH0_DR_ENTRY_NUM_OFF    0x5C4
#define HOST_CTRL_RB_RX_CH0_DR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_DR_ENTRY_NUM_OFF)

/* RX通道0配置 0x5C8 */
typedef union {
    struct {
        unsigned int rx_ch0_dr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_dr_head_ptr;
#define HOST_CTRL_RB_RX_CH0_DR_HEAD_PTR_OFF    0x5C8
#define HOST_CTRL_RB_RX_CH0_DR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_DR_HEAD_PTR_OFF)

/* RX通道0配置 0x5CC */
typedef union {
    struct {
        unsigned int rx_ch0_dr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_dr_tail_ptr;
#define HOST_CTRL_RB_RX_CH0_DR_TAIL_PTR_OFF    0x5CC
#define HOST_CTRL_RB_RX_CH0_DR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_DR_TAIL_PTR_OFF)

/* RX通道0配置 0x5D0 */
typedef union {
    struct {
        unsigned int rx_ch0_dr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_dr_afull_th;
#define HOST_CTRL_RB_RX_CH0_DR_AFULL_TH_OFF    0x5D0
#define HOST_CTRL_RB_RX_CH0_DR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_DR_AFULL_TH_OFF)

/* RX通道0配置 0x5D4 */
typedef union {
    struct {
        unsigned int rx_ch0_dr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_dr_aempty_th;
#define HOST_CTRL_RB_RX_CH0_DR_AEMPTY_TH_OFF    0x5D4
#define HOST_CTRL_RB_RX_CH0_DR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_DR_AEMPTY_TH_OFF)

/* RX通道0配置 0x5D8 */
typedef union {
    struct {
        unsigned int rx_ch0_status : 2; /* 0:1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_status;
#define HOST_CTRL_RB_RX_CH0_STATUS_OFF    0x5D8
#define HOST_CTRL_RB_RX_CH0_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_STATUS_OFF)

/* RX通道1配置 0x5E0 */
typedef union {
    struct {
        unsigned int rx_ch1_en : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_en;
#define HOST_CTRL_RB_RX_CH1_EN_OFF    0x5E0
#define HOST_CTRL_RB_RX_CH1_EN_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_EN_OFF)

/* RX通道1配置 0x5E4 */
typedef union {
    struct {
        unsigned int rx_ch1_mode : 1; /* 0 */
        unsigned int rx_ch1_mcy_start : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_mode;
#define HOST_CTRL_RB_RX_CH1_MODE_OFF    0x5E4
#define HOST_CTRL_RB_RX_CH1_MODE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_MODE_OFF)

/* RX通道1配置 0x5E8 */
typedef union {
    struct {
        unsigned int rx_ch1_pri : 3; /* 0:2 */
        unsigned int reserved0 : 29; /* 3:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_pri;
#define HOST_CTRL_RB_RX_CH1_PRI_OFF    0x5E8
#define HOST_CTRL_RB_RX_CH1_PRI_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_PRI_OFF)

/* RX通道1配置 0x5EC */
typedef union {
    struct {
        unsigned int rx_ch1_fc : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_fc;
#define HOST_CTRL_RB_RX_CH1_FC_OFF    0x5EC
#define HOST_CTRL_RB_RX_CH1_FC_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_FC_OFF)

/* RX通道1配置 0x5F0 */
typedef union {
    struct {
        unsigned int rx_ch1_sr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_sr_base_addr;
#define HOST_CTRL_RB_RX_CH1_SR_BASE_ADDR_OFF    0x5F0
#define HOST_CTRL_RB_RX_CH1_SR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_SR_BASE_ADDR_OFF)

/* RX通道1配置 0x5F4 */
typedef union {
    struct {
        unsigned int rx_ch1_sr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_sr_entry_num;
#define HOST_CTRL_RB_RX_CH1_SR_ENTRY_NUM_OFF    0x5F4
#define HOST_CTRL_RB_RX_CH1_SR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_SR_ENTRY_NUM_OFF)

/* RX通道1配置 0x5F8 */
typedef union {
    struct {
        unsigned int rx_ch1_sr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_sr_head_ptr;
#define HOST_CTRL_RB_RX_CH1_SR_HEAD_PTR_OFF    0x5F8
#define HOST_CTRL_RB_RX_CH1_SR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_SR_HEAD_PTR_OFF)

/* RX通道1配置 0x5FC */
typedef union {
    struct {
        unsigned int rx_ch1_sr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_sr_tail_ptr;
#define HOST_CTRL_RB_RX_CH1_SR_TAIL_PTR_OFF    0x5FC
#define HOST_CTRL_RB_RX_CH1_SR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_SR_TAIL_PTR_OFF)

/* RX通道1配置 0x600 */
typedef union {
    struct {
        unsigned int rx_ch1_sr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_sr_afull_th;
#define HOST_CTRL_RB_RX_CH1_SR_AFULL_TH_OFF    0x600
#define HOST_CTRL_RB_RX_CH1_SR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_SR_AFULL_TH_OFF)

/* RX通道1配置 0x604 */
typedef union {
    struct {
        unsigned int rx_ch1_sr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_sr_aempty_th;
#define HOST_CTRL_RB_RX_CH1_SR_AEMPTY_TH_OFF    0x604
#define HOST_CTRL_RB_RX_CH1_SR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_SR_AEMPTY_TH_OFF)

/* RX通道1配置 0x60C */
typedef union {
    struct {
        unsigned int rx_ch1_dr_base_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_dr_base_addr_h;
#define HOST_CTRL_RB_RX_CH1_DR_BASE_ADDR_H_OFF    0x60C
#define HOST_CTRL_RB_RX_CH1_DR_BASE_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_DR_BASE_ADDR_H_OFF)

/* RX通道1配置 0x610 */
typedef union {
    struct {
        unsigned int rx_ch1_dr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_dr_base_addr;
#define HOST_CTRL_RB_RX_CH1_DR_BASE_ADDR_OFF    0x610
#define HOST_CTRL_RB_RX_CH1_DR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_DR_BASE_ADDR_OFF)

/* RX通道1配置 0x614 */
typedef union {
    struct {
        unsigned int rx_ch1_dr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_dr_entry_num;
#define HOST_CTRL_RB_RX_CH1_DR_ENTRY_NUM_OFF    0x614
#define HOST_CTRL_RB_RX_CH1_DR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_DR_ENTRY_NUM_OFF)

/* RX通道1配置 0x618 */
typedef union {
    struct {
        unsigned int rx_ch1_dr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_dr_head_ptr;
#define HOST_CTRL_RB_RX_CH1_DR_HEAD_PTR_OFF    0x618
#define HOST_CTRL_RB_RX_CH1_DR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_DR_HEAD_PTR_OFF)

/* RX通道1配置 0x61C */
typedef union {
    struct {
        unsigned int rx_ch1_dr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_dr_tail_ptr;
#define HOST_CTRL_RB_RX_CH1_DR_TAIL_PTR_OFF    0x61C
#define HOST_CTRL_RB_RX_CH1_DR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_DR_TAIL_PTR_OFF)

/* RX通道1配置 0x620 */
typedef union {
    struct {
        unsigned int rx_ch1_dr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_dr_afull_th;
#define HOST_CTRL_RB_RX_CH1_DR_AFULL_TH_OFF    0x620
#define HOST_CTRL_RB_RX_CH1_DR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_DR_AFULL_TH_OFF)

/* RX通道1配置 0x624 */
typedef union {
    struct {
        unsigned int rx_ch1_dr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_dr_aempty_th;
#define HOST_CTRL_RB_RX_CH1_DR_AEMPTY_TH_OFF    0x624
#define HOST_CTRL_RB_RX_CH1_DR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_DR_AEMPTY_TH_OFF)

/* RX通道1配置 0x628 */
typedef union {
    struct {
        unsigned int rx_ch1_status : 2; /* 0:1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_status;
#define HOST_CTRL_RB_RX_CH1_STATUS_OFF    0x628
#define HOST_CTRL_RB_RX_CH1_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_STATUS_OFF)

/* RX通道2配置 0x630 */
typedef union {
    struct {
        unsigned int rx_ch2_en : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_en;
#define HOST_CTRL_RB_RX_CH2_EN_OFF    0x630
#define HOST_CTRL_RB_RX_CH2_EN_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_EN_OFF)

/* RX通道2配置 0x634 */
typedef union {
    struct {
        unsigned int rx_ch2_mode : 1; /* 0 */
        unsigned int rx_ch2_mcy_start : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_mode;
#define HOST_CTRL_RB_RX_CH2_MODE_OFF    0x634
#define HOST_CTRL_RB_RX_CH2_MODE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_MODE_OFF)

/* RX通道2配置 0x638 */
typedef union {
    struct {
        unsigned int rx_ch2_pri : 3; /* 0:2 */
        unsigned int reserved0 : 29; /* 3:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_pri;
#define HOST_CTRL_RB_RX_CH2_PRI_OFF    0x638
#define HOST_CTRL_RB_RX_CH2_PRI_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_PRI_OFF)

/* RX通道2配置 0x63C */
typedef union {
    struct {
        unsigned int rx_ch2_fc : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_fc;
#define HOST_CTRL_RB_RX_CH2_FC_OFF    0x63C
#define HOST_CTRL_RB_RX_CH2_FC_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_FC_OFF)

/* RX通道2配置 0x640 */
typedef union {
    struct {
        unsigned int rx_ch2_sr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_sr_base_addr;
#define HOST_CTRL_RB_RX_CH2_SR_BASE_ADDR_OFF    0x640
#define HOST_CTRL_RB_RX_CH2_SR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_SR_BASE_ADDR_OFF)

/* RX通道2配置 0x644 */
typedef union {
    struct {
        unsigned int rx_ch2_sr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_sr_entry_num;
#define HOST_CTRL_RB_RX_CH2_SR_ENTRY_NUM_OFF    0x644
#define HOST_CTRL_RB_RX_CH2_SR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_SR_ENTRY_NUM_OFF)

/* RX通道2配置 0x648 */
typedef union {
    struct {
        unsigned int rx_ch2_sr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_sr_head_ptr;
#define HOST_CTRL_RB_RX_CH2_SR_HEAD_PTR_OFF    0x648
#define HOST_CTRL_RB_RX_CH2_SR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_SR_HEAD_PTR_OFF)

/* RX通道2配置 0x64C */
typedef union {
    struct {
        unsigned int rx_ch2_sr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_sr_tail_ptr;
#define HOST_CTRL_RB_RX_CH2_SR_TAIL_PTR_OFF    0x64C
#define HOST_CTRL_RB_RX_CH2_SR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_SR_TAIL_PTR_OFF)

/* RX通道2配置 0x650 */
typedef union {
    struct {
        unsigned int rx_ch2_sr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_sr_afull_th;
#define HOST_CTRL_RB_RX_CH2_SR_AFULL_TH_OFF    0x650
#define HOST_CTRL_RB_RX_CH2_SR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_SR_AFULL_TH_OFF)

/* RX通道2配置 0x654 */
typedef union {
    struct {
        unsigned int rx_ch2_sr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_sr_aempty_th;
#define HOST_CTRL_RB_RX_CH2_SR_AEMPTY_TH_OFF    0x654
#define HOST_CTRL_RB_RX_CH2_SR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_SR_AEMPTY_TH_OFF)

/* RX通道2配置 0x65C */
typedef union {
    struct {
        unsigned int rx_ch2_dr_base_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_dr_base_addr_h;
#define HOST_CTRL_RB_RX_CH2_DR_BASE_ADDR_H_OFF    0x65C
#define HOST_CTRL_RB_RX_CH2_DR_BASE_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_DR_BASE_ADDR_H_OFF)

/* RX通道2配置 0x660 */
typedef union {
    struct {
        unsigned int rx_ch2_dr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_dr_base_addr;
#define HOST_CTRL_RB_RX_CH2_DR_BASE_ADDR_OFF    0x660
#define HOST_CTRL_RB_RX_CH2_DR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_DR_BASE_ADDR_OFF)

/* RX通道2配置 0x664 */
typedef union {
    struct {
        unsigned int rx_ch2_dr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_dr_entry_num;
#define HOST_CTRL_RB_RX_CH2_DR_ENTRY_NUM_OFF    0x664
#define HOST_CTRL_RB_RX_CH2_DR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_DR_ENTRY_NUM_OFF)

/* RX通道2配置 0x668 */
typedef union {
    struct {
        unsigned int rx_ch2_dr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_dr_head_ptr;
#define HOST_CTRL_RB_RX_CH2_DR_HEAD_PTR_OFF    0x668
#define HOST_CTRL_RB_RX_CH2_DR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_DR_HEAD_PTR_OFF)

/* RX通道2配置 0x66C */
typedef union {
    struct {
        unsigned int rx_ch2_dr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_dr_tail_ptr;
#define HOST_CTRL_RB_RX_CH2_DR_TAIL_PTR_OFF    0x66C
#define HOST_CTRL_RB_RX_CH2_DR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_DR_TAIL_PTR_OFF)

/* RX通道2配置 0x670 */
typedef union {
    struct {
        unsigned int rx_ch2_dr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_dr_afull_th;
#define HOST_CTRL_RB_RX_CH2_DR_AFULL_TH_OFF    0x670
#define HOST_CTRL_RB_RX_CH2_DR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_DR_AFULL_TH_OFF)

/* RX通道2配置 0x674 */
typedef union {
    struct {
        unsigned int rx_ch2_dr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_dr_aempty_th;
#define HOST_CTRL_RB_RX_CH2_DR_AEMPTY_TH_OFF    0x674
#define HOST_CTRL_RB_RX_CH2_DR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_DR_AEMPTY_TH_OFF)

/* RX通道2配置 0x678 */
typedef union {
    struct {
        unsigned int rx_ch2_status : 2; /* 0:1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_status;
#define HOST_CTRL_RB_RX_CH2_STATUS_OFF    0x678
#define HOST_CTRL_RB_RX_CH2_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_STATUS_OFF)

/* RX通道3配置 0x680 */
typedef union {
    struct {
        unsigned int rx_ch3_en : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_en;
#define HOST_CTRL_RB_RX_CH3_EN_OFF    0x680
#define HOST_CTRL_RB_RX_CH3_EN_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_EN_OFF)

/* RX通道3配置 0x684 */
typedef union {
    struct {
        unsigned int rx_ch3_mode : 1; /* 0 */
        unsigned int rx_ch3_mcy_start : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_mode;
#define HOST_CTRL_RB_RX_CH3_MODE_OFF    0x684
#define HOST_CTRL_RB_RX_CH3_MODE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_MODE_OFF)

/* RX通道3配置 0x688 */
typedef union {
    struct {
        unsigned int rx_ch3_pri : 3; /* 0:2 */
        unsigned int reserved0 : 29; /* 3:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_pri;
#define HOST_CTRL_RB_RX_CH3_PRI_OFF    0x688
#define HOST_CTRL_RB_RX_CH3_PRI_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_PRI_OFF)

/* RX通道3配置 0x68C */
typedef union {
    struct {
        unsigned int rx_ch3_fc : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_fc;
#define HOST_CTRL_RB_RX_CH3_FC_OFF    0x68C
#define HOST_CTRL_RB_RX_CH3_FC_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_FC_OFF)

/* RX通道3配置 0x690 */
typedef union {
    struct {
        unsigned int rx_ch3_sr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_sr_base_addr;
#define HOST_CTRL_RB_RX_CH3_SR_BASE_ADDR_OFF    0x690
#define HOST_CTRL_RB_RX_CH3_SR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_SR_BASE_ADDR_OFF)

/* RX通道3配置 0x694 */
typedef union {
    struct {
        unsigned int rx_ch3_sr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_sr_entry_num;
#define HOST_CTRL_RB_RX_CH3_SR_ENTRY_NUM_OFF    0x694
#define HOST_CTRL_RB_RX_CH3_SR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_SR_ENTRY_NUM_OFF)

/* RX通道3配置 0x698 */
typedef union {
    struct {
        unsigned int rx_ch3_sr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_sr_head_ptr;
#define HOST_CTRL_RB_RX_CH3_SR_HEAD_PTR_OFF    0x698
#define HOST_CTRL_RB_RX_CH3_SR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_SR_HEAD_PTR_OFF)

/* RX通道3配置 0x69C */
typedef union {
    struct {
        unsigned int rx_ch3_sr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_sr_tail_ptr;
#define HOST_CTRL_RB_RX_CH3_SR_TAIL_PTR_OFF    0x69C
#define HOST_CTRL_RB_RX_CH3_SR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_SR_TAIL_PTR_OFF)

/* RX通道3配置 0x6A0 */
typedef union {
    struct {
        unsigned int rx_ch3_sr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_sr_afull_th;
#define HOST_CTRL_RB_RX_CH3_SR_AFULL_TH_OFF    0x6A0
#define HOST_CTRL_RB_RX_CH3_SR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_SR_AFULL_TH_OFF)

/* RX通道3配置 0x6A4 */
typedef union {
    struct {
        unsigned int rx_ch3_sr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_sr_aempty_th;
#define HOST_CTRL_RB_RX_CH3_SR_AEMPTY_TH_OFF    0x6A4
#define HOST_CTRL_RB_RX_CH3_SR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_SR_AEMPTY_TH_OFF)

/* RX通道3配置 0x6AC */
typedef union {
    struct {
        unsigned int rx_ch3_dr_base_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_dr_base_addr_h;
#define HOST_CTRL_RB_RX_CH3_DR_BASE_ADDR_H_OFF    0x6AC
#define HOST_CTRL_RB_RX_CH3_DR_BASE_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_DR_BASE_ADDR_H_OFF)

/* RX通道3配置 0x6B0 */
typedef union {
    struct {
        unsigned int rx_ch3_dr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_dr_base_addr;
#define HOST_CTRL_RB_RX_CH3_DR_BASE_ADDR_OFF    0x6B0
#define HOST_CTRL_RB_RX_CH3_DR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_DR_BASE_ADDR_OFF)

/* RX通道3配置 0x6B4 */
typedef union {
    struct {
        unsigned int rx_ch3_dr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_dr_entry_num;
#define HOST_CTRL_RB_RX_CH3_DR_ENTRY_NUM_OFF    0x6B4
#define HOST_CTRL_RB_RX_CH3_DR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_DR_ENTRY_NUM_OFF)

/* RX通道3配置 0x6B8 */
typedef union {
    struct {
        unsigned int rx_ch3_dr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_dr_head_ptr;
#define HOST_CTRL_RB_RX_CH3_DR_HEAD_PTR_OFF    0x6B8
#define HOST_CTRL_RB_RX_CH3_DR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_DR_HEAD_PTR_OFF)

/* RX通道3配置 0x6BC */
typedef union {
    struct {
        unsigned int rx_ch3_dr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_dr_tail_ptr;
#define HOST_CTRL_RB_RX_CH3_DR_TAIL_PTR_OFF    0x6BC
#define HOST_CTRL_RB_RX_CH3_DR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_DR_TAIL_PTR_OFF)

/* RX通道3配置 0x6C0 */
typedef union {
    struct {
        unsigned int rx_ch3_dr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_dr_afull_th;
#define HOST_CTRL_RB_RX_CH3_DR_AFULL_TH_OFF    0x6C0
#define HOST_CTRL_RB_RX_CH3_DR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_DR_AFULL_TH_OFF)

/* RX通道3配置 0x6C4 */
typedef union {
    struct {
        unsigned int rx_ch3_dr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_dr_aempty_th;
#define HOST_CTRL_RB_RX_CH3_DR_AEMPTY_TH_OFF    0x6C4
#define HOST_CTRL_RB_RX_CH3_DR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_DR_AEMPTY_TH_OFF)

/* RX通道3配置 0x6C8 */
typedef union {
    struct {
        unsigned int rx_ch3_status : 2; /* 0:1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_status;
#define HOST_CTRL_RB_RX_CH3_STATUS_OFF    0x6C8
#define HOST_CTRL_RB_RX_CH3_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_STATUS_OFF)

/* RX通道4配置 0x6D0 */
typedef union {
    struct {
        unsigned int rx_ch4_en : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_en;
#define HOST_CTRL_RB_RX_CH4_EN_OFF    0x6D0
#define HOST_CTRL_RB_RX_CH4_EN_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_EN_OFF)

/* RX通道4配置 0x6D4 */
typedef union {
    struct {
        unsigned int rx_ch4_mode : 1; /* 0 */
        unsigned int rx_ch4_mcy_start : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_mode;
#define HOST_CTRL_RB_RX_CH4_MODE_OFF    0x6D4
#define HOST_CTRL_RB_RX_CH4_MODE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_MODE_OFF)

/* RX通道4配置 0x6D8 */
typedef union {
    struct {
        unsigned int rx_ch4_pri : 3; /* 0:2 */
        unsigned int reserved0 : 29; /* 3:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_pri;
#define HOST_CTRL_RB_RX_CH4_PRI_OFF    0x6D8
#define HOST_CTRL_RB_RX_CH4_PRI_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_PRI_OFF)

/* RX通道4配置 0x6DC */
typedef union {
    struct {
        unsigned int rx_ch4_fc : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_fc;
#define HOST_CTRL_RB_RX_CH4_FC_OFF    0x6DC
#define HOST_CTRL_RB_RX_CH4_FC_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_FC_OFF)

/* RX通道4配置 0x6E0 */
typedef union {
    struct {
        unsigned int rx_ch4_sr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_sr_base_addr;
#define HOST_CTRL_RB_RX_CH4_SR_BASE_ADDR_OFF    0x6E0
#define HOST_CTRL_RB_RX_CH4_SR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_SR_BASE_ADDR_OFF)

/* RX通道4配置 0x6E4 */
typedef union {
    struct {
        unsigned int rx_ch4_sr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_sr_entry_num;
#define HOST_CTRL_RB_RX_CH4_SR_ENTRY_NUM_OFF    0x6E4
#define HOST_CTRL_RB_RX_CH4_SR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_SR_ENTRY_NUM_OFF)

/* RX通道4配置 0x6E8 */
typedef union {
    struct {
        unsigned int rx_ch4_sr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_sr_head_ptr;
#define HOST_CTRL_RB_RX_CH4_SR_HEAD_PTR_OFF    0x6E8
#define HOST_CTRL_RB_RX_CH4_SR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_SR_HEAD_PTR_OFF)

/* RX通道4配置 0x6EC */
typedef union {
    struct {
        unsigned int rx_ch4_sr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_sr_tail_ptr;
#define HOST_CTRL_RB_RX_CH4_SR_TAIL_PTR_OFF    0x6EC
#define HOST_CTRL_RB_RX_CH4_SR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_SR_TAIL_PTR_OFF)

/* RX通道4配置 0x6F0 */
typedef union {
    struct {
        unsigned int rx_ch4_sr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_sr_afull_th;
#define HOST_CTRL_RB_RX_CH4_SR_AFULL_TH_OFF    0x6F0
#define HOST_CTRL_RB_RX_CH4_SR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_SR_AFULL_TH_OFF)

/* RX通道4配置 0x6F4 */
typedef union {
    struct {
        unsigned int rx_ch4_sr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_sr_aempty_th;
#define HOST_CTRL_RB_RX_CH4_SR_AEMPTY_TH_OFF    0x6F4
#define HOST_CTRL_RB_RX_CH4_SR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_SR_AEMPTY_TH_OFF)

/* RX通道4配置 0x6FC */
typedef union {
    struct {
        unsigned int rx_ch4_dr_base_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_dr_base_addr_h;
#define HOST_CTRL_RB_RX_CH4_DR_BASE_ADDR_H_OFF    0x6FC
#define HOST_CTRL_RB_RX_CH4_DR_BASE_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_DR_BASE_ADDR_H_OFF)

/* RX通道4配置 0x700 */
typedef union {
    struct {
        unsigned int rx_ch4_dr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_dr_base_addr;
#define HOST_CTRL_RB_RX_CH4_DR_BASE_ADDR_OFF    0x700
#define HOST_CTRL_RB_RX_CH4_DR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_DR_BASE_ADDR_OFF)

/* RX通道4配置 0x704 */
typedef union {
    struct {
        unsigned int rx_ch4_dr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_dr_entry_num;
#define HOST_CTRL_RB_RX_CH4_DR_ENTRY_NUM_OFF    0x704
#define HOST_CTRL_RB_RX_CH4_DR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_DR_ENTRY_NUM_OFF)

/* RX通道4配置 0x708 */
typedef union {
    struct {
        unsigned int rx_ch4_dr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_dr_head_ptr;
#define HOST_CTRL_RB_RX_CH4_DR_HEAD_PTR_OFF    0x708
#define HOST_CTRL_RB_RX_CH4_DR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_DR_HEAD_PTR_OFF)

/* RX通道4配置 0x70C */
typedef union {
    struct {
        unsigned int rx_ch4_dr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_dr_tail_ptr;
#define HOST_CTRL_RB_RX_CH4_DR_TAIL_PTR_OFF    0x70C
#define HOST_CTRL_RB_RX_CH4_DR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_DR_TAIL_PTR_OFF)

/* RX通道4配置 0x710 */
typedef union {
    struct {
        unsigned int rx_ch4_dr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_dr_afull_th;
#define HOST_CTRL_RB_RX_CH4_DR_AFULL_TH_OFF    0x710
#define HOST_CTRL_RB_RX_CH4_DR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_DR_AFULL_TH_OFF)

/* RX通道4配置 0x714 */
typedef union {
    struct {
        unsigned int rx_ch4_dr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_dr_aempty_th;
#define HOST_CTRL_RB_RX_CH4_DR_AEMPTY_TH_OFF    0x714
#define HOST_CTRL_RB_RX_CH4_DR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_DR_AEMPTY_TH_OFF)

/* RX通道4配置 0x718 */
typedef union {
    struct {
        unsigned int rx_ch4_status : 2; /* 0:1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_status;
#define HOST_CTRL_RB_RX_CH4_STATUS_OFF    0x718
#define HOST_CTRL_RB_RX_CH4_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_STATUS_OFF)

/* RX通道5配置 0x720 */
typedef union {
    struct {
        unsigned int rx_ch5_en : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_en;
#define HOST_CTRL_RB_RX_CH5_EN_OFF    0x720
#define HOST_CTRL_RB_RX_CH5_EN_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_EN_OFF)

/* RX通道5配置 0x724 */
typedef union {
    struct {
        unsigned int rx_ch5_mode : 1; /* 0 */
        unsigned int rx_ch5_mcy_start : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_mode;
#define HOST_CTRL_RB_RX_CH5_MODE_OFF    0x724
#define HOST_CTRL_RB_RX_CH5_MODE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_MODE_OFF)

/* RX通道5配置 0x728 */
typedef union {
    struct {
        unsigned int rx_ch5_pri : 3; /* 0:2 */
        unsigned int reserved0 : 29; /* 3:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_pri;
#define HOST_CTRL_RB_RX_CH5_PRI_OFF    0x728
#define HOST_CTRL_RB_RX_CH5_PRI_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_PRI_OFF)

/* RX通道5配置 0x72C */
typedef union {
    struct {
        unsigned int rx_ch5_fc : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_fc;
#define HOST_CTRL_RB_RX_CH5_FC_OFF    0x72C
#define HOST_CTRL_RB_RX_CH5_FC_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_FC_OFF)

/* RX通道5配置 0x730 */
typedef union {
    struct {
        unsigned int rx_ch5_sr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_sr_base_addr;
#define HOST_CTRL_RB_RX_CH5_SR_BASE_ADDR_OFF    0x730
#define HOST_CTRL_RB_RX_CH5_SR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_SR_BASE_ADDR_OFF)

/* RX通道5配置 0x734 */
typedef union {
    struct {
        unsigned int rx_ch5_sr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_sr_entry_num;
#define HOST_CTRL_RB_RX_CH5_SR_ENTRY_NUM_OFF    0x734
#define HOST_CTRL_RB_RX_CH5_SR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_SR_ENTRY_NUM_OFF)

/* RX通道5配置 0x738 */
typedef union {
    struct {
        unsigned int rx_ch5_sr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_sr_head_ptr;
#define HOST_CTRL_RB_RX_CH5_SR_HEAD_PTR_OFF    0x738
#define HOST_CTRL_RB_RX_CH5_SR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_SR_HEAD_PTR_OFF)

/* RX通道5配置 0x73C */
typedef union {
    struct {
        unsigned int rx_ch5_sr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_sr_tail_ptr;
#define HOST_CTRL_RB_RX_CH5_SR_TAIL_PTR_OFF    0x73C
#define HOST_CTRL_RB_RX_CH5_SR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_SR_TAIL_PTR_OFF)

/* RX通道5配置 0x740 */
typedef union {
    struct {
        unsigned int rx_ch5_sr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_sr_afull_th;
#define HOST_CTRL_RB_RX_CH5_SR_AFULL_TH_OFF    0x740
#define HOST_CTRL_RB_RX_CH5_SR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_SR_AFULL_TH_OFF)

/* RX通道5配置 0x744 */
typedef union {
    struct {
        unsigned int rx_ch5_sr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_sr_aempty_th;
#define HOST_CTRL_RB_RX_CH5_SR_AEMPTY_TH_OFF    0x744
#define HOST_CTRL_RB_RX_CH5_SR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_SR_AEMPTY_TH_OFF)

/* RX通道5配置 0x74C */
typedef union {
    struct {
        unsigned int rx_ch5_dr_base_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_dr_base_addr_h;
#define HOST_CTRL_RB_RX_CH5_DR_BASE_ADDR_H_OFF    0x74C
#define HOST_CTRL_RB_RX_CH5_DR_BASE_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_DR_BASE_ADDR_H_OFF)

/* RX通道5配置 0x750 */
typedef union {
    struct {
        unsigned int rx_ch5_dr_base_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_dr_base_addr;
#define HOST_CTRL_RB_RX_CH5_DR_BASE_ADDR_OFF    0x750
#define HOST_CTRL_RB_RX_CH5_DR_BASE_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_DR_BASE_ADDR_OFF)

/* RX通道5配置 0x754 */
typedef union {
    struct {
        unsigned int rx_ch5_dr_entry_num : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_dr_entry_num;
#define HOST_CTRL_RB_RX_CH5_DR_ENTRY_NUM_OFF    0x754
#define HOST_CTRL_RB_RX_CH5_DR_ENTRY_NUM_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_DR_ENTRY_NUM_OFF)

/* RX通道5配置 0x758 */
typedef union {
    struct {
        unsigned int rx_ch5_dr_head_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_dr_head_ptr;
#define HOST_CTRL_RB_RX_CH5_DR_HEAD_PTR_OFF    0x758
#define HOST_CTRL_RB_RX_CH5_DR_HEAD_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_DR_HEAD_PTR_OFF)

/* RX通道5配置 0x75C */
typedef union {
    struct {
        unsigned int rx_ch5_dr_tail_ptr : 11; /* 0:10 */
        unsigned int reserved0 : 21; /* 11:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_dr_tail_ptr;
#define HOST_CTRL_RB_RX_CH5_DR_TAIL_PTR_OFF    0x75C
#define HOST_CTRL_RB_RX_CH5_DR_TAIL_PTR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_DR_TAIL_PTR_OFF)

/* RX通道5配置 0x760 */
typedef union {
    struct {
        unsigned int rx_ch5_dr_afull_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_dr_afull_th;
#define HOST_CTRL_RB_RX_CH5_DR_AFULL_TH_OFF    0x760
#define HOST_CTRL_RB_RX_CH5_DR_AFULL_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_DR_AFULL_TH_OFF)

/* RX通道5配置 0x764 */
typedef union {
    struct {
        unsigned int rx_ch5_dr_aempty_th : 10; /* 0:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_dr_aempty_th;
#define HOST_CTRL_RB_RX_CH5_DR_AEMPTY_TH_OFF    0x764
#define HOST_CTRL_RB_RX_CH5_DR_AEMPTY_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_DR_AEMPTY_TH_OFF)

/* RX通道5配置 0x768 */
typedef union {
    struct {
        unsigned int rx_ch5_status : 2; /* 0:1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_status;
#define HOST_CTRL_RB_RX_CH5_STATUS_OFF    0x768
#define HOST_CTRL_RB_RX_CH5_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_STATUS_OFF)

/* TX通道状态 0x770 */
typedef union {
    struct {
        unsigned int tx_ch_sr_afull : 4; /* 0:3 */
        unsigned int tx_ch_sr_full : 4; /* 4:7 */
        unsigned int reserved0 : 24; /* 8:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch_sr_full;
#define HOST_CTRL_RB_TX_CH_SR_FULL_OFF    0x770
#define HOST_CTRL_RB_TX_CH_SR_FULL_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH_SR_FULL_OFF)

/* TX通道状态 0x774 */
typedef union {
    struct {
        unsigned int tx_ch_sr_aempty : 4; /* 0:3 */
        unsigned int tx_ch_sr_empty : 4; /* 4:7 */
        unsigned int reserved0 : 24; /* 8:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch_sr_empty;
#define HOST_CTRL_RB_TX_CH_SR_EMPTY_OFF    0x774
#define HOST_CTRL_RB_TX_CH_SR_EMPTY_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH_SR_EMPTY_OFF)

/* TX通道状态 0x778 */
typedef union {
    struct {
        unsigned int tx_ch_dr_afull : 4; /* 0:3 */
        unsigned int tx_ch_dr_full : 4; /* 4:7 */
        unsigned int reserved0 : 24; /* 8:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch_dr_full;
#define HOST_CTRL_RB_TX_CH_DR_FULL_OFF    0x778
#define HOST_CTRL_RB_TX_CH_DR_FULL_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH_DR_FULL_OFF)

/* TX通道状态 0x77C */
typedef union {
    struct {
        unsigned int tx_ch_dr_aempty : 4; /* 0:3 */
        unsigned int tx_ch_dr_empty : 4; /* 4:7 */
        unsigned int reserved0 : 24; /* 8:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch_dr_empty;
#define HOST_CTRL_RB_TX_CH_DR_EMPTY_OFF    0x77C
#define HOST_CTRL_RB_TX_CH_DR_EMPTY_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH_DR_EMPTY_OFF)

/* RX通道状态 0x780 */
typedef union {
    struct {
        unsigned int rx_ch_sr_afull : 6; /* 0:5 */
        unsigned int rx_ch_sr_full : 6; /* 6:11 */
        unsigned int reserved0 : 20; /* 12:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch_sr_full;
#define HOST_CTRL_RB_RX_CH_SR_FULL_OFF    0x780
#define HOST_CTRL_RB_RX_CH_SR_FULL_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH_SR_FULL_OFF)

/* RX通道状态 0x784 */
typedef union {
    struct {
        unsigned int rx_ch_sr_aempty : 6; /* 0:5 */
        unsigned int rx_ch_sr_empty : 6; /* 6:11 */
        unsigned int reserved0 : 20; /* 12:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch_sr_empty;
#define HOST_CTRL_RB_RX_CH_SR_EMPTY_OFF    0x784
#define HOST_CTRL_RB_RX_CH_SR_EMPTY_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH_SR_EMPTY_OFF)

/* RX通道状态 0x788 */
typedef union {
    struct {
        unsigned int rx_ch_dr_afull : 6; /* 0:5 */
        unsigned int rx_ch_dr_full : 6; /* 6:11 */
        unsigned int reserved0 : 20; /* 12:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch_dr_full;
#define HOST_CTRL_RB_RX_CH_DR_FULL_OFF    0x788
#define HOST_CTRL_RB_RX_CH_DR_FULL_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH_DR_FULL_OFF)

/* RX通道状态 0x78C */
typedef union {
    struct {
        unsigned int rx_ch_dr_aempty : 6; /* 0:5 */
        unsigned int rx_ch_dr_empty : 6; /* 6:11 */
        unsigned int reserved0 : 20; /* 12:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch_dr_empty;
#define HOST_CTRL_RB_RX_CH_DR_EMPTY_OFF    0x78C
#define HOST_CTRL_RB_RX_CH_DR_EMPTY_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH_DR_EMPTY_OFF)

/* Data buffer门限 0x7A8 */
typedef union {
    struct {
        unsigned int rx_rdat_buf_ampt_th : 7; /* 0:6 */
        unsigned int reserved0 : 1; /* 7 */
        unsigned int rx_rdat_buf_aful_th : 7; /* 8:14 */
        unsigned int reserved1 : 1; /* 15 */
        unsigned int tx_rdat_buf_ampt_th : 7; /* 16:22 */
        unsigned int reserved2 : 1; /* 23 */
        unsigned int tx_rdat_buf_aful_th : 7; /* 24:30 */
        unsigned int reserved3 : 1; /* 31 */
    } bits;
    unsigned int as_dword;
} hreg_txrx_dat_buf_aful_ampt_th;
#define HOST_CTRL_RB_TXRX_DAT_BUF_AFUL_AMPT_TH_OFF    0x7A8
#define HOST_CTRL_RB_TXRX_DAT_BUF_AFUL_AMPT_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TXRX_DAT_BUF_AFUL_AMPT_TH_OFF)

/* cnta buffer门限 0x7AC */
typedef union {
    struct {
        unsigned int rx_rcnt_buf_ampt_th : 4; /* 0:3 */
        unsigned int reserved0 : 4; /* 4:7 */
        unsigned int rx_rcnt_buf_aful_th : 4; /* 8:11 */
        unsigned int reserved1 : 4; /* 12:15 */
        unsigned int tx_rcnt_buf_ampt_th : 4; /* 16:19 */
        unsigned int reserved2 : 4; /* 20:23 */
        unsigned int tx_rcnt_buf_aful_th : 4; /* 24:27 */
        unsigned int reserved3 : 4; /* 28:31 */
    } bits;
    unsigned int as_dword;
} hreg_txrx_cnt_buf_aful_ampt_th;
#define HOST_CTRL_RB_TXRX_CNT_BUF_AFUL_AMPT_TH_OFF    0x7AC
#define HOST_CTRL_RB_TXRX_CNT_BUF_AFUL_AMPT_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TXRX_CNT_BUF_AFUL_AMPT_TH_OFF)

/* 描述符 buffer门限 0x7B0 */
typedef union {
    struct {
        unsigned int rx_desc_ampt_th : 5; /* 0:4 */
        unsigned int reserved0 : 3; /* 5:7 */
        unsigned int rx_desc_aful_th : 5; /* 8:12 */
        unsigned int reserved1 : 3; /* 13:15 */
        unsigned int tx_desc_ampt_th : 5; /* 16:20 */
        unsigned int reserved2 : 3; /* 21:23 */
        unsigned int tx_desc_aful_th : 5; /* 24:28 */
        unsigned int reserved3 : 3; /* 29:31 */
    } bits;
    unsigned int as_dword;
} hreg_txrx_desc_buf_aful_ampt_th;
#define HOST_CTRL_RB_TXRX_DESC_BUF_AFUL_AMPT_TH_OFF    0x7B0
#define HOST_CTRL_RB_TXRX_DESC_BUF_AFUL_AMPT_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TXRX_DESC_BUF_AFUL_AMPT_TH_OFF)

/* 描述符读取个数 0x7B8 */
typedef union {
    struct {
        unsigned int rx_ch0_desc_num_th : 5; /* 0:4 */
        unsigned int rx_ch1_desc_num_th : 5; /* 5:9 */
        unsigned int rx_ch2_desc_num_th : 5; /* 10:14 */
        unsigned int rx_ch3_desc_num_th : 5; /* 15:19 */
        unsigned int rx_ch4_desc_num_th : 5; /* 20:24 */
        unsigned int rx_ch5_desc_num_th : 5; /* 25:29 */
        unsigned int reserved0 : 2; /* 30:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_desc_num_th;
#define HOST_CTRL_RB_RX_DESC_NUM_TH_OFF    0x7B8
#define HOST_CTRL_RB_RX_DESC_NUM_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_DESC_NUM_TH_OFF)

/* 描述符读取个数 0x7BC */
typedef union {
    struct {
        unsigned int tx_ch0_desc_num_th : 5; /* 0:4 */
        unsigned int tx_ch1_desc_num_th : 5; /* 5:9 */
        unsigned int tx_ch2_desc_num_th : 5; /* 10:14 */
        unsigned int tx_ch3_desc_num_th : 5; /* 15:19 */
        unsigned int reserved0 : 12; /* 20:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_desc_num_th;
#define HOST_CTRL_RB_TX_DESC_NUM_TH_OFF    0x7BC
#define HOST_CTRL_RB_TX_DESC_NUM_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_DESC_NUM_TH_OFF)

/* MEMORY COPY配置 0x7C0 */
typedef union {
    struct {
        unsigned int tx_mcpy_src_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_mcpy_src_addr;
#define HOST_CTRL_RB_TX_MCPY_SRC_ADDR_OFF    0x7C0
#define HOST_CTRL_RB_TX_MCPY_SRC_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_MCPY_SRC_ADDR_OFF)

/* MEMORY COPY配置 0x7C4 */
typedef union {
    struct {
        unsigned int tx_mcpy_src_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_mcpy_src_addr_h;
#define HOST_CTRL_RB_TX_MCPY_SRC_ADDR_H_OFF    0x7C4
#define HOST_CTRL_RB_TX_MCPY_SRC_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_MCPY_SRC_ADDR_H_OFF)

/* MEMORY COPY配置 0x7C8 */
typedef union {
    struct {
        unsigned int tx_mcpy_dst_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_mcpy_dst_addr;
#define HOST_CTRL_RB_TX_MCPY_DST_ADDR_OFF    0x7C8
#define HOST_CTRL_RB_TX_MCPY_DST_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_MCPY_DST_ADDR_OFF)

/* MEMORY COPY配置 0x7CC */
typedef union {
    struct {
        unsigned int tx_mcpy_src_len : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_mcpy_src_len;
#define HOST_CTRL_RB_TX_MCPY_SRC_LEN_OFF    0x7CC
#define HOST_CTRL_RB_TX_MCPY_SRC_LEN_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_MCPY_SRC_LEN_OFF)

/* MEMORY COPY配置 0x7D0 */
typedef union {
    struct {
        unsigned int rx_mcpy_src_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_mcpy_src_addr;
#define HOST_CTRL_RB_RX_MCPY_SRC_ADDR_OFF    0x7D0
#define HOST_CTRL_RB_RX_MCPY_SRC_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_MCPY_SRC_ADDR_OFF)

/* MEMORY COPY配置 0x7D4 */
typedef union {
    struct {
        unsigned int rx_mcpy_dst_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_mcpy_dst_addr;
#define HOST_CTRL_RB_RX_MCPY_DST_ADDR_OFF    0x7D4
#define HOST_CTRL_RB_RX_MCPY_DST_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_MCPY_DST_ADDR_OFF)

/* MEMORY COPY配置 0x7D8 */
typedef union {
    struct {
        unsigned int rx_mcpy_dst_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_mcpy_dst_addr_h;
#define HOST_CTRL_RB_RX_MCPY_DST_ADDR_H_OFF    0x7D8
#define HOST_CTRL_RB_RX_MCPY_DST_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_MCPY_DST_ADDR_H_OFF)

/* MEMORY COPY配置 0x7DC */
typedef union {
    struct {
        unsigned int rx_mcpy_src_len : 16; /* 0:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_mcpy_src_len;
#define HOST_CTRL_RB_RX_MCPY_SRC_LEN_OFF    0x7DC
#define HOST_CTRL_RB_RX_MCPY_SRC_LEN_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_MCPY_SRC_LEN_OFF)

/* 状态机状态上报 0x818 */
typedef union {
    struct {
        unsigned int rx_desc_state : 3; /* 0:2 */
        unsigned int reserved0 : 1; /* 3 */
        unsigned int tx_desc_state : 3; /* 4:6 */
        unsigned int reserved1 : 25; /* 7:31 */
    } bits;
    unsigned int as_dword;
} hreg_txrx_state_machine_report;
#define HOST_CTRL_RB_TXRX_STATE_MACHINE_REPORT_OFF    0x818
#define HOST_CTRL_RB_TXRX_STATE_MACHINE_REPORT_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TXRX_STATE_MACHINE_REPORT_OFF)

/* 状态机状态上报 0x81C */
typedef union {
    struct {
        unsigned int rx_wr_state : 4; /* 0:3 */
        unsigned int rx_rd_state : 4; /* 4:7 */
        unsigned int tx_wr_state : 4; /* 8:11 */
        unsigned int tx_rd_state : 4; /* 12:15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_txrx_dstate_machine_report;
#define HOST_CTRL_RB_TXRX_DSTATE_MACHINE_REPORT_OFF    0x81C
#define HOST_CTRL_RB_TXRX_DSTATE_MACHINE_REPORT_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TXRX_DSTATE_MACHINE_REPORT_OFF)

/* 状态机状态上报 0x820 */
typedef union {
    struct {
        unsigned int rx_ch0_desc_waddr : 5; /* 0:4 */
        unsigned int rx_ch0_desc_raddr : 5; /* 5:9 */
        unsigned int rx_ch0_desc_afull : 1; /* 10 */
        unsigned int rx_ch0_desc_aempty : 1; /* 11 */
        unsigned int rx_ch0_desc_full : 1; /* 12 */
        unsigned int rx_ch0_desc_empty : 1; /* 13 */
        unsigned int reserved0 : 18; /* 14:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_desc_buff_status;
#define HOST_CTRL_RB_RX_CH0_DESC_BUFF_STATUS_OFF    0x820
#define HOST_CTRL_RB_RX_CH0_DESC_BUFF_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_DESC_BUFF_STATUS_OFF)

/* 状态机状态上报 0x824 */
typedef union {
    struct {
        unsigned int rx_ch1_desc_waddr : 5; /* 0:4 */
        unsigned int rx_ch1_desc_raddr : 5; /* 5:9 */
        unsigned int rx_ch1_desc_afull : 1; /* 10 */
        unsigned int rx_ch1_desc_aempty : 1; /* 11 */
        unsigned int rx_ch1_desc_full : 1; /* 12 */
        unsigned int rx_ch1_desc_empty : 1; /* 13 */
        unsigned int reserved0 : 18; /* 14:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_desc_buff_status;
#define HOST_CTRL_RB_RX_CH1_DESC_BUFF_STATUS_OFF    0x824
#define HOST_CTRL_RB_RX_CH1_DESC_BUFF_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_DESC_BUFF_STATUS_OFF)

/* 状态机状态上报 0x828 */
typedef union {
    struct {
        unsigned int rx_ch2_desc_waddr : 5; /* 0:4 */
        unsigned int rx_ch2_desc_raddr : 5; /* 5:9 */
        unsigned int rx_ch2_desc_afull : 1; /* 10 */
        unsigned int rx_ch2_desc_aempty : 1; /* 11 */
        unsigned int rx_ch2_desc_full : 1; /* 12 */
        unsigned int rx_ch2_desc_empty : 1; /* 13 */
        unsigned int reserved0 : 18; /* 14:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_desc_buff_status;
#define HOST_CTRL_RB_RX_CH2_DESC_BUFF_STATUS_OFF    0x828
#define HOST_CTRL_RB_RX_CH2_DESC_BUFF_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_DESC_BUFF_STATUS_OFF)

/* 状态机状态上报 0x82C */
typedef union {
    struct {
        unsigned int rx_ch3_desc_waddr : 5; /* 0:4 */
        unsigned int rx_ch3_desc_raddr : 5; /* 5:9 */
        unsigned int rx_ch3_desc_afull : 1; /* 10 */
        unsigned int rx_ch3_desc_aempty : 1; /* 11 */
        unsigned int rx_ch3_desc_full : 1; /* 12 */
        unsigned int rx_ch3_desc_empty : 1; /* 13 */
        unsigned int reserved0 : 18; /* 14:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_desc_buff_status;
#define HOST_CTRL_RB_RX_CH3_DESC_BUFF_STATUS_OFF    0x82C
#define HOST_CTRL_RB_RX_CH3_DESC_BUFF_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_DESC_BUFF_STATUS_OFF)

/* 状态机状态上报 0x830 */
typedef union {
    struct {
        unsigned int rx_ch4_desc_waddr : 5; /* 0:4 */
        unsigned int rx_ch4_desc_raddr : 5; /* 5:9 */
        unsigned int rx_ch4_desc_afull : 1; /* 10 */
        unsigned int rx_ch4_desc_aempty : 1; /* 11 */
        unsigned int rx_ch4_desc_full : 1; /* 12 */
        unsigned int rx_ch4_desc_empty : 1; /* 13 */
        unsigned int reserved0 : 18; /* 14:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_desc_buff_status;
#define HOST_CTRL_RB_RX_CH4_DESC_BUFF_STATUS_OFF    0x830
#define HOST_CTRL_RB_RX_CH4_DESC_BUFF_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_DESC_BUFF_STATUS_OFF)

/* 状态机状态上报 0x834 */
typedef union {
    struct {
        unsigned int rx_ch5_desc_waddr : 5; /* 0:4 */
        unsigned int rx_ch5_desc_raddr : 5; /* 5:9 */
        unsigned int rx_ch5_desc_afull : 1; /* 10 */
        unsigned int rx_ch5_desc_aempty : 1; /* 11 */
        unsigned int rx_ch5_desc_full : 1; /* 12 */
        unsigned int rx_ch5_desc_empty : 1; /* 13 */
        unsigned int reserved0 : 18; /* 14:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_desc_buff_status;
#define HOST_CTRL_RB_RX_CH5_DESC_BUFF_STATUS_OFF    0x834
#define HOST_CTRL_RB_RX_CH5_DESC_BUFF_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_DESC_BUFF_STATUS_OFF)

/* 状态机状态上报 0x838 */
typedef union {
    struct {
        unsigned int tx_ch0_desc_waddr : 5; /* 0:4 */
        unsigned int tx_ch0_desc_raddr : 5; /* 5:9 */
        unsigned int tx_ch0_desc_afull : 1; /* 10 */
        unsigned int tx_ch0_desc_aempty : 1; /* 11 */
        unsigned int tx_ch0_desc_full : 1; /* 12 */
        unsigned int tx_ch0_desc_empty : 1; /* 13 */
        unsigned int reserved0 : 18; /* 14:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_desc_buff_status;
#define HOST_CTRL_RB_TX_CH0_DESC_BUFF_STATUS_OFF    0x838
#define HOST_CTRL_RB_TX_CH0_DESC_BUFF_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_DESC_BUFF_STATUS_OFF)

/* 状态机状态上报 0x83C */
typedef union {
    struct {
        unsigned int tx_ch1_desc_waddr : 5; /* 0:4 */
        unsigned int tx_ch1_desc_raddr : 5; /* 5:9 */
        unsigned int tx_ch1_desc_afull : 1; /* 10 */
        unsigned int tx_ch1_desc_aempty : 1; /* 11 */
        unsigned int tx_ch1_desc_full : 1; /* 12 */
        unsigned int tx_ch1_desc_empty : 1; /* 13 */
        unsigned int reserved0 : 18; /* 14:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_desc_buff_status;
#define HOST_CTRL_RB_TX_CH1_DESC_BUFF_STATUS_OFF    0x83C
#define HOST_CTRL_RB_TX_CH1_DESC_BUFF_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_DESC_BUFF_STATUS_OFF)

/* 状态机状态上报 0x840 */
typedef union {
    struct {
        unsigned int tx_ch2_desc_waddr : 5; /* 0:4 */
        unsigned int tx_ch2_desc_raddr : 5; /* 5:9 */
        unsigned int tx_ch2_desc_afull : 1; /* 10 */
        unsigned int tx_ch2_desc_aempty : 1; /* 11 */
        unsigned int tx_ch2_desc_full : 1; /* 12 */
        unsigned int tx_ch2_desc_empty : 1; /* 13 */
        unsigned int reserved0 : 18; /* 14:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_desc_buff_status;
#define HOST_CTRL_RB_TX_CH2_DESC_BUFF_STATUS_OFF    0x840
#define HOST_CTRL_RB_TX_CH2_DESC_BUFF_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_DESC_BUFF_STATUS_OFF)

/* 状态机状态上报 0x844 */
typedef union {
    struct {
        unsigned int tx_ch3_desc_waddr : 5; /* 0:4 */
        unsigned int tx_ch3_desc_raddr : 5; /* 5:9 */
        unsigned int tx_ch3_desc_afull : 1; /* 10 */
        unsigned int tx_ch3_desc_aempty : 1; /* 11 */
        unsigned int tx_ch3_desc_full : 1; /* 12 */
        unsigned int tx_ch3_desc_empty : 1; /* 13 */
        unsigned int reserved0 : 18; /* 14:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_desc_buff_status;
#define HOST_CTRL_RB_TX_CH3_DESC_BUFF_STATUS_OFF    0x844
#define HOST_CTRL_RB_TX_CH3_DESC_BUFF_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_DESC_BUFF_STATUS_OFF)

/* 状态机状态上报 0x848 */
typedef union {
    struct {
        unsigned int rx_rdat_buf_waddr : 7; /* 0:6 */
        unsigned int rx_rdat_buf_raddr : 7; /* 7:13 */
        unsigned int rx_rdat_buf_afull : 1; /* 14 */
        unsigned int rx_rdat_buf_aempty : 1; /* 15 */
        unsigned int rx_rdat_buf_full : 1; /* 16 */
        unsigned int rx_rdat_buf_empty : 1; /* 17 */
        unsigned int reserved0 : 14; /* 18:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_data_buff_status;
#define HOST_CTRL_RB_RX_DATA_BUFF_STATUS_OFF    0x848
#define HOST_CTRL_RB_RX_DATA_BUFF_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_DATA_BUFF_STATUS_OFF)

/* 状态机状态上报 0x84C */
typedef union {
    struct {
        unsigned int rx_rcnt_buf_waddr : 4; /* 0:3 */
        unsigned int rx_rcnt_buf_raddr : 4; /* 4:7 */
        unsigned int rx_rcnt_buf_afull : 1; /* 8 */
        unsigned int rx_rcnt_buf_aempty : 1; /* 9 */
        unsigned int rx_rcnt_buf_full : 1; /* 10 */
        unsigned int rx_rcnt_buf_empty : 1; /* 11 */
        unsigned int reserved0 : 20; /* 12:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_rcnt_buff_status;
#define HOST_CTRL_RB_RX_RCNT_BUFF_STATUS_OFF    0x84C
#define HOST_CTRL_RB_RX_RCNT_BUFF_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_RCNT_BUFF_STATUS_OFF)

/* 状态机状态上报 0x850 */
typedef union {
    struct {
        unsigned int tx_rdat_buf_waddr : 7; /* 0:6 */
        unsigned int tx_rdat_buf_raddr : 7; /* 7:13 */
        unsigned int tx_rdat_buf_afull : 1; /* 14 */
        unsigned int tx_rdat_buf_aempty : 1; /* 15 */
        unsigned int tx_rdat_buf_full : 1; /* 16 */
        unsigned int tx_rdat_buf_empty : 1; /* 17 */
        unsigned int reserved0 : 14; /* 18:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_data_buff_status;
#define HOST_CTRL_RB_TX_DATA_BUFF_STATUS_OFF    0x850
#define HOST_CTRL_RB_TX_DATA_BUFF_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_DATA_BUFF_STATUS_OFF)

/* 状态机状态上报 0x854 */
typedef union {
    struct {
        unsigned int tx_rcnt_buf_waddr : 4; /* 0:3 */
        unsigned int tx_rcnt_buf_raddr : 4; /* 4:7 */
        unsigned int tx_rcnt_buf_afull : 1; /* 8 */
        unsigned int tx_rcnt_buf_aempty : 1; /* 9 */
        unsigned int tx_rcnt_buf_full : 1; /* 10 */
        unsigned int tx_rcnt_buf_empty : 1; /* 11 */
        unsigned int reserved0 : 20; /* 12:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_rcnt_buff_status;
#define HOST_CTRL_RB_TX_RCNT_BUFF_STATUS_OFF    0x854
#define HOST_CTRL_RB_TX_RCNT_BUFF_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_RCNT_BUFF_STATUS_OFF)

/* 状态机状态上报 0x858 */
typedef union {
    struct {
        unsigned int rx_ch_end_buf_waddr : 4; /* 0:3 */
        unsigned int rx_ch_end_buf_raddr : 4; /* 4:7 */
        unsigned int rx_ch_end_buf_afull : 1; /* 8 */
        unsigned int rx_ch_end_buf_aempty : 1; /* 9 */
        unsigned int rx_ch_end_buf_full : 1; /* 10 */
        unsigned int rx_ch_end_buf_empty : 1; /* 11 */
        unsigned int rx_src_addr_buf_waddr : 3; /* 12:14 */
        unsigned int rx_src_addr_buf_raddr : 3; /* 15:17 */
        unsigned int rx_src_addr_buf_afull : 1; /* 18 */
        unsigned int rx_src_addr_buf_aempty : 1; /* 19 */
        unsigned int rx_src_addr_buf_full : 1; /* 20 */
        unsigned int rx_src_addr_buf_empty : 1; /* 21 */
        unsigned int reserved0 : 10; /* 22:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch_end_buff_status;
#define HOST_CTRL_RB_RX_CH_END_BUFF_STATUS_OFF    0x858
#define HOST_CTRL_RB_RX_CH_END_BUFF_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH_END_BUFF_STATUS_OFF)

/* 状态机状态上报 0x85C */
typedef union {
    struct {
        unsigned int tx_ch_end_buf_waddr : 4; /* 0:3 */
        unsigned int tx_ch_end_buf_raddr : 4; /* 4:7 */
        unsigned int tx_ch_end_buf_afull : 1; /* 8 */
        unsigned int tx_ch_end_buf_aempty : 1; /* 9 */
        unsigned int tx_ch_end_buf_full : 1; /* 10 */
        unsigned int tx_ch_end_buf_empty : 1; /* 11 */
        unsigned int tx_src_addr_buf_waddr : 3; /* 12:14 */
        unsigned int tx_src_addr_buf_raddr : 3; /* 15:17 */
        unsigned int tx_src_addr_buf_afull : 1; /* 18 */
        unsigned int tx_src_addr_buf_aempty : 1; /* 19 */
        unsigned int tx_src_addr_buf_full : 1; /* 20 */
        unsigned int tx_src_addr_buf_empty : 1; /* 21 */
        unsigned int reserved0 : 10; /* 22:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch_end_buff_status;
#define HOST_CTRL_RB_TX_CH_END_BUFF_STATUS_OFF    0x85C
#define HOST_CTRL_RB_TX_CH_END_BUFF_STATUS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH_END_BUFF_STATUS_OFF)

/* RX错误地址上报 0x860 */
typedef union {
    struct {
        unsigned int rx_err_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_err_addr;
#define HOST_CTRL_RB_RX_ERR_ADDR_OFF    0x860
#define HOST_CTRL_RB_RX_ERR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_ERR_ADDR_OFF)

/* TX错误地址上报 0x864 */
typedef union {
    struct {
        unsigned int tx_err_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_err_addr;
#define HOST_CTRL_RB_TX_ERR_ADDR_OFF    0x864
#define HOST_CTRL_RB_TX_ERR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_ERR_ADDR_OFF)

/*  0x86C */
typedef union {
    struct {
        unsigned int tx_err_intr_mask : 5; /* 0:4 */
        unsigned int reserved0 : 3; /* 5:7 */
        unsigned int tx_done_intr_mask : 5; /* 8:12 */
        unsigned int reserved1 : 3; /* 13:15 */
        unsigned int rx_err_intr_mask : 6; /* 16:21 */
        unsigned int reserved2 : 2; /* 22:23 */
        unsigned int rx_done_intr_mask : 6; /* 24:29 */
        unsigned int reserved3 : 2; /* 30:31 */
    } bits;
    unsigned int as_dword;
} hreg_ete_intr_mask;
#define HOST_CTRL_RB_ETE_INTR_MASK_OFF    0x86C
#define HOST_CTRL_RB_ETE_INTR_MASK_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_ETE_INTR_MASK_OFF)

/*  0x870 */
typedef union {
    struct {
        unsigned int tx_err_intr_clr : 5; /* 0:4 */
        unsigned int reserved0 : 3; /* 5:7 */
        unsigned int tx_done_intr_clr : 5; /* 8:12 */
        unsigned int reserved1 : 3; /* 13:15 */
        unsigned int rx_err_intr_clr : 6; /* 16:21 */
        unsigned int reserved2 : 2; /* 22:23 */
        unsigned int rx_done_intr_clr : 6; /* 24:29 */
        unsigned int reserved3 : 2; /* 30:31 */
    } bits;
    unsigned int as_dword;
} hreg_ete_intr_clr;
#define HOST_CTRL_RB_ETE_INTR_CLR_OFF    0x870
#define HOST_CTRL_RB_ETE_INTR_CLR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_ETE_INTR_CLR_OFF)

/*  0x874 */
typedef union {
    struct {
        unsigned int tx_err_intr_status : 5; /* 0:4 */
        unsigned int reserved0 : 3; /* 5:7 */
        unsigned int tx_done_intr_status : 5; /* 8:12 */
        unsigned int reserved1 : 3; /* 13:15 */
        unsigned int rx_err_intr_status : 6; /* 16:21 */
        unsigned int reserved2 : 2; /* 22:23 */
        unsigned int rx_done_intr_status : 6; /* 24:29 */
        unsigned int reserved3 : 2; /* 30:31 */
    } bits;
    unsigned int as_dword;
} hreg_ete_intr_sts;
#define HOST_CTRL_RB_ETE_INTR_STS_OFF    0x874
#define HOST_CTRL_RB_ETE_INTR_STS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_ETE_INTR_STS_OFF)

/*  0x878 */
typedef union {
    struct {
        unsigned int tx_err_intr_raw_status : 5; /* 0:4 */
        unsigned int reserved0 : 3; /* 5:7 */
        unsigned int tx_done_intr_raw_status : 5; /* 8:12 */
        unsigned int reserved1 : 3; /* 13:15 */
        unsigned int rx_err_intr_raw_status : 6; /* 16:21 */
        unsigned int reserved2 : 2; /* 22:23 */
        unsigned int rx_done_intr_raw_status : 6; /* 24:29 */
        unsigned int reserved3 : 2; /* 30:31 */
    } bits;
    unsigned int as_dword;
} hreg_ete_intr_raw_sts;
#define HOST_CTRL_RB_ETE_INTR_RAW_STS_OFF    0x878
#define HOST_CTRL_RB_ETE_INTR_RAW_STS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_ETE_INTR_RAW_STS_OFF)

/*  0x87C */
typedef union {
    struct {
        unsigned int tx_ch_dr_empty_intr_mask : 4; /* 0:3 */
        unsigned int rx_ch_dr_empty_intr_mask : 6; /* 4:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_ete_ch_dr_empty_intr_mask;
#define HOST_CTRL_RB_ETE_CH_DR_EMPTY_INTR_MASK_OFF    0x87C
#define HOST_CTRL_RB_ETE_CH_DR_EMPTY_INTR_MASK_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_ETE_CH_DR_EMPTY_INTR_MASK_OFF)

/*  0x880 */
typedef union {
    struct {
        unsigned int tx_ch_dr_empty_intr_clr : 4; /* 0:3 */
        unsigned int rx_ch_dr_empty_intr_clr : 6; /* 4:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_ete_ch_dr_empty_intr_clr;
#define HOST_CTRL_RB_ETE_CH_DR_EMPTY_INTR_CLR_OFF    0x880
#define HOST_CTRL_RB_ETE_CH_DR_EMPTY_INTR_CLR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_ETE_CH_DR_EMPTY_INTR_CLR_OFF)

/*  0x884 */
typedef union {
    struct {
        unsigned int tx_ch_dr_empty_intr_status : 4; /* 0:3 */
        unsigned int rx_ch_dr_empty_intr_status : 6; /* 4:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_ete_ch_dr_empty_intr_sts;
#define HOST_CTRL_RB_ETE_CH_DR_EMPTY_INTR_STS_OFF    0x884
#define HOST_CTRL_RB_ETE_CH_DR_EMPTY_INTR_STS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_ETE_CH_DR_EMPTY_INTR_STS_OFF)

/*  0x888 */
typedef union {
    struct {
        unsigned int tx_ch_dr_empty_intr_raw_status : 4; /* 0:3 */
        unsigned int rx_ch_dr_empty_intr_raw_status : 6; /* 4:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_ete_ch_dr_empty_intr_raw_sts;
#define HOST_CTRL_RB_ETE_CH_DR_EMPTY_INTR_RAW_STS_OFF    0x888
#define HOST_CTRL_RB_ETE_CH_DR_EMPTY_INTR_RAW_STS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_ETE_CH_DR_EMPTY_INTR_RAW_STS_OFF)

/* TXRX拆分一次性读最大长度 0x88C */
typedef union {
    struct {
        unsigned int tx_rd_max_num : 11; /* 0:10 */
        unsigned int rx_rd_max_num : 11; /* 11:21 */
        unsigned int reserved0 : 10; /* 22:31 */
    } bits;
    unsigned int as_dword;
} hreg_txrx_rd_max;
#define HOST_CTRL_RB_TXRX_RD_MAX_OFF    0x88C
#define HOST_CTRL_RB_TXRX_RD_MAX_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TXRX_RD_MAX_OFF)

/* TXRX拆分一次性写最大长度 0x890 */
typedef union {
    struct {
        unsigned int tx_wr_max_num : 11; /* 0:10 */
        unsigned int rx_wr_max_num : 11; /* 11:21 */
        unsigned int reserved0 : 10; /* 22:31 */
    } bits;
    unsigned int as_dword;
} hreg_txrx_wr_max;
#define HOST_CTRL_RB_TXRX_WR_MAX_OFF    0x890
#define HOST_CTRL_RB_TXRX_WR_MAX_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TXRX_WR_MAX_OFF)

/* TXRX_DATA_FIFO水线满阈值 0x894 */
typedef union {
    struct {
        unsigned int tx_dat_fifo_th : 11; /* 0:10 */
        unsigned int rx_dat_fifo_th : 11; /* 11:21 */
        unsigned int reserved0 : 10; /* 22:31 */
    } bits;
    unsigned int as_dword;
} hreg_txrx_data_fifo_th;
#define HOST_CTRL_RB_TXRX_DATA_FIFO_TH_OFF    0x894
#define HOST_CTRL_RB_TXRX_DATA_FIFO_TH_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TXRX_DATA_FIFO_TH_OFF)

/* ETE的SEGMODE模式 0x898 */
typedef union {
    struct {
        unsigned int cfg_ete_seg_mode : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_ete_seg_mode;
#define HOST_CTRL_RB_ETE_SEG_MODE_OFF    0x898
#define HOST_CTRL_RB_ETE_SEG_MODE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_ETE_SEG_MODE_OFF)

/* TX FIFO CNT上报 0x8A0 */
typedef union {
    struct {
        unsigned int tx_wr_fifo_cnt : 16; /* 0:15 */
        unsigned int tx_fifo_cnt : 11; /* 16:26 */
        unsigned int reserved0 : 5; /* 27:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_fifo_cnt;
#define HOST_CTRL_RB_TX_FIFO_CNT_OFF    0x8A0
#define HOST_CTRL_RB_TX_FIFO_CNT_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_FIFO_CNT_OFF)

/* RX FIFO CNT上报 0x8A4 */
typedef union {
    struct {
        unsigned int rx_wr_fifo_cnt : 16; /* 0:15 */
        unsigned int rx_fifo_cnt : 11; /* 16:26 */
        unsigned int reserved0 : 5; /* 27:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_fifo_cnt;
#define HOST_CTRL_RB_RX_FIFO_CNT_OFF    0x8A4
#define HOST_CTRL_RB_RX_FIFO_CNT_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_FIFO_CNT_OFF)

/* FIFO_ERR_CLR 0x8A8 */
typedef union {
    struct {
        unsigned int soc_tx_ch_end_rd_empty_err_clr : 1; /* 0 */
        unsigned int soc_tx_wr_fifo_cnt_rd_empty_err_clr : 1; /* 1 */
        unsigned int soc_tx_rd_offset_rd_empty_err_clr : 1; /* 2 */
        unsigned int soc_tx_wr_offset_rd_empty_err_clr : 1; /* 3 */
        unsigned int soc_tx_desc_offset_rd_empty_err_clr : 1; /* 4 */
        unsigned int soc_tx_ch_end_wr_full_err_clr : 1; /* 5 */
        unsigned int soc_tx_wr_fifo_cnt_wr_full_err_clr : 1; /* 6 */
        unsigned int soc_tx_rd_offset_wr_full_err_clr : 1; /* 7 */
        unsigned int soc_tx_wr_offset_wr_full_err_clr : 1; /* 8 */
        unsigned int soc_tx_desc_offset_wr_full_err_clr : 1; /* 9 */
        unsigned int soc_tx_rd_cnt_zero_err_clr : 1; /* 10 */
        unsigned int soc_tx_wr_cnt_zero_err_clr : 1; /* 11 */
        unsigned int soc_tx_rd_desc_cnt_zero_err_clr : 1; /* 12 */
        unsigned int soc_rx_ch_end_rd_empty_err_clr : 1; /* 13 */
        unsigned int soc_rx_wr_fifo_cnt_rd_empty_err_clr : 1; /* 14 */
        unsigned int soc_rx_rd_offset_rd_empty_err_clr : 1; /* 15 */
        unsigned int soc_rx_wr_offset_rd_empty_err_clr : 1; /* 16 */
        unsigned int soc_rx_desc_offset_rd_empty_err_clr : 1; /* 17 */
        unsigned int soc_rx_ch_end_wr_full_err_clr : 1; /* 18 */
        unsigned int soc_rx_wr_fifo_cnt_wr_full_err_clr : 1; /* 19 */
        unsigned int soc_rx_rd_offset_wr_full_err_clr : 1; /* 20 */
        unsigned int soc_rx_wr_offset_wr_full_err_clr : 1; /* 21 */
        unsigned int soc_rx_desc_offset_wr_full_err_clr : 1; /* 22 */
        unsigned int soc_rx_rd_cnt_zero_err_clr : 1; /* 23 */
        unsigned int soc_rx_wr_cnt_zero_err_clr : 1; /* 24 */
        unsigned int soc_rx_rd_desc_cnt_zero_err_clr : 1; /* 25 */
        unsigned int reserved0 : 6; /* 26:31 */
    } bits;
    unsigned int as_dword;
} hreg_fifo_err_clr;
#define HOST_CTRL_RB_FIFO_ERR_CLR_OFF    0x8A8
#define HOST_CTRL_RB_FIFO_ERR_CLR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_FIFO_ERR_CLR_OFF)

/* RX_RING_FIFO_ERR_CLR 0x8AC */
typedef union {
    struct {
        unsigned int soc_rx_dr_ring_full_err_clr : 6; /* 0:5 */
        unsigned int soc_rx_sr_ring_full_err_clr : 6; /* 6:11 */
        unsigned int soc_rx_dr_ring_empty_err_clr : 6; /* 12:17 */
        unsigned int soc_rx_sr_ring_empty_err_clr : 6; /* 18:23 */
        unsigned int soc_rx_rdat_buf_wr_full_err_clr : 1; /* 24 */
        unsigned int soc_rx_rcnt_buf_wr_full_err_clr : 1; /* 25 */
        unsigned int soc_rx_rdat_buf_rd_empty_err_clr : 1; /* 26 */
        unsigned int soc_rx_rcnt_buf_rd_empty_err_clr : 1; /* 27 */
        unsigned int reserved0 : 4; /* 28:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ring_fifo_err_clr;
#define HOST_CTRL_RB_RX_RING_FIFO_ERR_CLR_OFF    0x8AC
#define HOST_CTRL_RB_RX_RING_FIFO_ERR_CLR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_RING_FIFO_ERR_CLR_OFF)

/* TX_RING_FIFO_ERR_CLR 0x8B0 */
typedef union {
    struct {
        unsigned int soc_tx_dr_ring_full_err_clr : 4; /* 0:3 */
        unsigned int soc_tx_sr_ring_full_err_clr : 4; /* 4:7 */
        unsigned int soc_tx_dr_ring_empty_err_clr : 4; /* 8:11 */
        unsigned int soc_tx_sr_ring_empty_err_clr : 4; /* 12:15 */
        unsigned int soc_tx_rdat_buf_wr_full_err_clr : 1; /* 16 */
        unsigned int soc_tx_rcnt_buf_wr_full_err_clr : 1; /* 17 */
        unsigned int soc_tx_rdat_buf_rd_empty_err_clr : 1; /* 18 */
        unsigned int soc_tx_rcnt_buf_rd_empty_err_clr : 1; /* 19 */
        unsigned int reserved0 : 12; /* 20:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ring_fifo_err_clr;
#define HOST_CTRL_RB_TX_RING_FIFO_ERR_CLR_OFF    0x8B0
#define HOST_CTRL_RB_TX_RING_FIFO_ERR_CLR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_RING_FIFO_ERR_CLR_OFF)

/* FIFO_ERR_CLR 0x8B4 */
typedef union {
    struct {
        unsigned int soc_tx_ch_end_rd_empty_err : 1; /* 0 */
        unsigned int soc_tx_wr_fifo_cnt_rd_empty_err : 1; /* 1 */
        unsigned int soc_tx_rd_offset_rd_empty_err : 1; /* 2 */
        unsigned int soc_tx_wr_offset_rd_empty_err : 1; /* 3 */
        unsigned int soc_tx_desc_offset_rd_empty_err : 1; /* 4 */
        unsigned int soc_tx_ch_end_wr_full_err : 1; /* 5 */
        unsigned int soc_tx_wr_fifo_cnt_wr_full_err : 1; /* 6 */
        unsigned int soc_tx_rd_offset_wr_full_err : 1; /* 7 */
        unsigned int soc_tx_wr_offset_wr_full_err : 1; /* 8 */
        unsigned int soc_tx_desc_offset_wr_full_err : 1; /* 9 */
        unsigned int soc_tx_rd_cnt_zero_err : 1; /* 10 */
        unsigned int soc_tx_wr_cnt_zero_err : 1; /* 11 */
        unsigned int soc_tx_rd_desc_cnt_zero_err : 1; /* 12 */
        unsigned int soc_rx_ch_end_rd_empty_err : 1; /* 13 */
        unsigned int soc_rx_wr_fifo_cnt_rd_empty_err : 1; /* 14 */
        unsigned int soc_rx_rd_offset_rd_empty_err : 1; /* 15 */
        unsigned int soc_rx_wr_offset_rd_empty_err : 1; /* 16 */
        unsigned int soc_rx_desc_offset_rd_empty_err : 1; /* 17 */
        unsigned int soc_rx_ch_end_wr_full_err : 1; /* 18 */
        unsigned int soc_rx_wr_fifo_cnt_wr_full_err : 1; /* 19 */
        unsigned int soc_rx_rd_offset_wr_full_err : 1; /* 20 */
        unsigned int soc_rx_wr_offset_wr_full_err : 1; /* 21 */
        unsigned int soc_rx_desc_offset_wr_full_err : 1; /* 22 */
        unsigned int soc_rx_rd_cnt_zero_err : 1; /* 23 */
        unsigned int soc_rx_wr_cnt_zero_err : 1; /* 24 */
        unsigned int soc_rx_rd_desc_cnt_zero_err : 1; /* 25 */
        unsigned int reserved0 : 6; /* 26:31 */
    } bits;
    unsigned int as_dword;
} hreg_fifo_err_sts;
#define HOST_CTRL_RB_FIFO_ERR_STS_OFF    0x8B4
#define HOST_CTRL_RB_FIFO_ERR_STS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_FIFO_ERR_STS_OFF)

/* RX_RING_FIFO_ERR_CLR 0x8B8 */
typedef union {
    struct {
        unsigned int soc_rx_dr_ring_full_err : 6; /* 0:5 */
        unsigned int soc_rx_sr_ring_full_err : 6; /* 6:11 */
        unsigned int soc_rx_dr_ring_empty_err : 6; /* 12:17 */
        unsigned int soc_rx_sr_ring_empty_err : 6; /* 18:23 */
        unsigned int soc_rx_rdat_buf_wr_full_err : 1; /* 24 */
        unsigned int soc_rx_rcnt_buf_wr_full_err : 1; /* 25 */
        unsigned int soc_rx_rdat_buf_rd_empty_err : 1; /* 26 */
        unsigned int soc_rx_rcnt_buf_rd_empty_err : 1; /* 27 */
        unsigned int soc_rx_src_addr_buf_wr_full_err : 1; /* 28 */
        unsigned int soc_rx_src_addr_buf_rd_empty_err : 1; /* 29 */
        unsigned int soc_rx_ete_mon_err : 1; /* 30 */
        unsigned int reserved0 : 1; /* 31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ring_fifo_err_sts;
#define HOST_CTRL_RB_RX_RING_FIFO_ERR_STS_OFF    0x8B8
#define HOST_CTRL_RB_RX_RING_FIFO_ERR_STS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_RING_FIFO_ERR_STS_OFF)

/* TX_RING_FIFO_ERR_CLR 0x8BC */
typedef union {
    struct {
        unsigned int soc_tx_dr_ring_full_err : 4; /* 0:3 */
        unsigned int soc_tx_sr_ring_full_err : 4; /* 4:7 */
        unsigned int soc_tx_dr_ring_empty_err : 4; /* 8:11 */
        unsigned int soc_tx_sr_ring_empty_err : 4; /* 12:15 */
        unsigned int soc_tx_rdat_buf_wr_full_err : 1; /* 16 */
        unsigned int soc_tx_rcnt_buf_wr_full_err : 1; /* 17 */
        unsigned int soc_tx_rdat_buf_rd_empty_err : 1; /* 18 */
        unsigned int soc_tx_rcnt_buf_rd_empty_err : 1; /* 19 */
        unsigned int soc_tx_src_addr_buf_wr_full_err : 1; /* 20 */
        unsigned int soc_tx_src_addr_buf_rd_empty_err : 1; /* 21 */
        unsigned int soc_tx_ete_mon_err : 1; /* 22 */
        unsigned int reserved0 : 9; /* 23:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ring_fifo_err_sts;
#define HOST_CTRL_RB_TX_RING_FIFO_ERR_STS_OFF    0x8BC
#define HOST_CTRL_RB_TX_RING_FIFO_ERR_STS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_RING_FIFO_ERR_STS_OFF)

/* TX_RING_FIFO_ERR_CLR 0x8C0 */
typedef union {
    struct {
        unsigned int tx_ch_vir_dr_empty : 4; /* 0:3 */
        unsigned int rx_ch_vir_dr_empty : 6; /* 4:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_vir_fifo_sts;
#define HOST_CTRL_RB_VIR_FIFO_STS_OFF    0x8C0
#define HOST_CTRL_RB_VIR_FIFO_STS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_VIR_FIFO_STS_OFF)

/* TX_RING_FIFO_ERR_CLR 0x8C4 */
typedef union {
    struct {
        unsigned int tx_ch_vir_dr_empty_err : 4; /* 0:3 */
        unsigned int rx_ch_vir_dr_empty_err : 6; /* 4:9 */
        unsigned int reserved0 : 22; /* 10:31 */
    } bits;
    unsigned int as_dword;
} hreg_vir_fifo_err_sts;
#define HOST_CTRL_RB_VIR_FIFO_ERR_STS_OFF    0x8C4
#define HOST_CTRL_RB_VIR_FIFO_ERR_STS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_VIR_FIFO_ERR_STS_OFF)

/* ETE_CH_PRI_PENDING_MASK 0x8C8 */
typedef union {
    struct {
        unsigned int soc_rx_ch_pri_pending_mask : 1; /* 0 */
        unsigned int soc_tx_ch_pri_pending_mask : 1; /* 1 */
        unsigned int reserved0 : 30; /* 2:31 */
    } bits;
    unsigned int as_dword;
} hreg_ete_pri_pending_mask;
#define HOST_CTRL_RB_ETE_PRI_PENDING_MASK_OFF    0x8C8
#define HOST_CTRL_RB_ETE_PRI_PENDING_MASK_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_ETE_PRI_PENDING_MASK_OFF)

/* RX_CH0包个数总数计数 0xC00 */
typedef union {
    struct {
        unsigned int rx_ch0_total_cnt : 11; /* 0:10 */
        unsigned int reserved0 : 5; /* 11:15 */
        unsigned int host_rx_ch0_total_cnt : 11; /* 16:26 */
        unsigned int reserved1 : 5; /* 27:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_done_total_cnt;
#define HOST_CTRL_RB_RX_CH0_DONE_TOTAL_CNT_OFF    0xC00
#define HOST_CTRL_RB_RX_CH0_DONE_TOTAL_CNT_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_DONE_TOTAL_CNT_OFF)

/* RX_ch1包个数总数计数 0xC04 */
typedef union {
    struct {
        unsigned int rx_ch1_total_cnt : 11; /* 0:10 */
        unsigned int reserved0 : 5; /* 11:15 */
        unsigned int host_rx_ch1_total_cnt : 11; /* 16:26 */
        unsigned int reserved1 : 5; /* 27:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_done_total_cnt;
#define HOST_CTRL_RB_RX_CH1_DONE_TOTAL_CNT_OFF    0xC04
#define HOST_CTRL_RB_RX_CH1_DONE_TOTAL_CNT_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_DONE_TOTAL_CNT_OFF)

/* RX_ch2包个数总数计数 0xC08 */
typedef union {
    struct {
        unsigned int rx_ch2_total_cnt : 11; /* 0:10 */
        unsigned int reserved0 : 5; /* 11:15 */
        unsigned int host_rx_ch2_total_cnt : 11; /* 16:26 */
        unsigned int reserved1 : 5; /* 27:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_done_total_cnt;
#define HOST_CTRL_RB_RX_CH2_DONE_TOTAL_CNT_OFF    0xC08
#define HOST_CTRL_RB_RX_CH2_DONE_TOTAL_CNT_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_DONE_TOTAL_CNT_OFF)

/* RX_ch3包个数总数计数 0xC0C */
typedef union {
    struct {
        unsigned int rx_ch3_total_cnt : 11; /* 0:10 */
        unsigned int reserved0 : 5; /* 11:15 */
        unsigned int host_rx_ch3_total_cnt : 11; /* 16:26 */
        unsigned int reserved1 : 5; /* 27:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_done_total_cnt;
#define HOST_CTRL_RB_RX_CH3_DONE_TOTAL_CNT_OFF    0xC0C
#define HOST_CTRL_RB_RX_CH3_DONE_TOTAL_CNT_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_DONE_TOTAL_CNT_OFF)

/* RX_ch4包个数总数计数 0xC10 */
typedef union {
    struct {
        unsigned int rx_ch4_total_cnt : 11; /* 0:10 */
        unsigned int reserved0 : 5; /* 11:15 */
        unsigned int host_rx_ch4_total_cnt : 11; /* 16:26 */
        unsigned int reserved1 : 5; /* 27:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_done_total_cnt;
#define HOST_CTRL_RB_RX_CH4_DONE_TOTAL_CNT_OFF    0xC10
#define HOST_CTRL_RB_RX_CH4_DONE_TOTAL_CNT_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_DONE_TOTAL_CNT_OFF)

/* RX_ch5包个数总数计数 0xC14 */
typedef union {
    struct {
        unsigned int rx_ch5_total_cnt : 11; /* 0:10 */
        unsigned int reserved0 : 5; /* 11:15 */
        unsigned int host_rx_ch5_total_cnt : 11; /* 16:26 */
        unsigned int reserved1 : 5; /* 27:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_done_total_cnt;
#define HOST_CTRL_RB_RX_CH5_DONE_TOTAL_CNT_OFF    0xC14
#define HOST_CTRL_RB_RX_CH5_DONE_TOTAL_CNT_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_DONE_TOTAL_CNT_OFF)

/* tx_CH0包个数总数计数 0xC18 */
typedef union {
    struct {
        unsigned int tx_ch0_total_cnt : 11; /* 0:10 */
        unsigned int reserved0 : 5; /* 11:15 */
        unsigned int host_tx_ch0_total_cnt : 11; /* 16:26 */
        unsigned int reserved1 : 5; /* 27:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch0_done_total_cnt;
#define HOST_CTRL_RB_TX_CH0_DONE_TOTAL_CNT_OFF    0xC18
#define HOST_CTRL_RB_TX_CH0_DONE_TOTAL_CNT_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH0_DONE_TOTAL_CNT_OFF)

/* tx_ch1包个数总数计数 0xC1C */
typedef union {
    struct {
        unsigned int tx_ch1_total_cnt : 11; /* 0:10 */
        unsigned int reserved0 : 5; /* 11:15 */
        unsigned int host_tx_ch1_total_cnt : 11; /* 16:26 */
        unsigned int reserved1 : 5; /* 27:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch1_done_total_cnt;
#define HOST_CTRL_RB_TX_CH1_DONE_TOTAL_CNT_OFF    0xC1C
#define HOST_CTRL_RB_TX_CH1_DONE_TOTAL_CNT_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH1_DONE_TOTAL_CNT_OFF)

/* tx_ch2包个数总数计数 0xC20 */
typedef union {
    struct {
        unsigned int tx_ch2_total_cnt : 11; /* 0:10 */
        unsigned int reserved0 : 5; /* 11:15 */
        unsigned int host_tx_ch2_total_cnt : 11; /* 16:26 */
        unsigned int reserved1 : 5; /* 27:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch2_done_total_cnt;
#define HOST_CTRL_RB_TX_CH2_DONE_TOTAL_CNT_OFF    0xC20
#define HOST_CTRL_RB_TX_CH2_DONE_TOTAL_CNT_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH2_DONE_TOTAL_CNT_OFF)

/* tx_ch3包个数总数计数 0xC24 */
typedef union {
    struct {
        unsigned int tx_ch3_total_cnt : 11; /* 0:10 */
        unsigned int reserved0 : 5; /* 11:15 */
        unsigned int host_tx_ch3_total_cnt : 11; /* 16:26 */
        unsigned int reserved1 : 5; /* 27:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch3_done_total_cnt;
#define HOST_CTRL_RB_TX_CH3_DONE_TOTAL_CNT_OFF    0xC24
#define HOST_CTRL_RB_TX_CH3_DONE_TOTAL_CNT_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH3_DONE_TOTAL_CNT_OFF)

/* tx_ch4包个数总数计数 0xC28 */
typedef union {
    struct {
        unsigned int tx_ch4_total_cnt : 11; /* 0:10 */
        unsigned int reserved0 : 5; /* 11:15 */
        unsigned int host_tx_ch4_total_cnt : 11; /* 16:26 */
        unsigned int reserved1 : 5; /* 27:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ch4_done_total_cnt;
#define HOST_CTRL_RB_TX_CH4_DONE_TOTAL_CNT_OFF    0xC28
#define HOST_CTRL_RB_TX_CH4_DONE_TOTAL_CNT_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_CH4_DONE_TOTAL_CNT_OFF)

/* 当前的源端描述符地址 0xC2C */
typedef union {
    struct {
        unsigned int tx_dst_desc_curr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_dst_desc_curr_addr;
#define HOST_CTRL_RB_TX_DST_DESC_CURR_ADDR_OFF    0xC2C
#define HOST_CTRL_RB_TX_DST_DESC_CURR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_DST_DESC_CURR_ADDR_OFF)

/* 当前的源端地址 0xC30 */
typedef union {
    struct {
        unsigned int tx_src_curr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_src_curr_addr;
#define HOST_CTRL_RB_TX_SRC_CURR_ADDR_OFF    0xC30
#define HOST_CTRL_RB_TX_SRC_CURR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_SRC_CURR_ADDR_OFF)

/* 当前的源端地址 0xC34 */
typedef union {
    struct {
        unsigned int tx_src_curr_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_src_curr_addr_h;
#define HOST_CTRL_RB_TX_SRC_CURR_ADDR_H_OFF    0xC34
#define HOST_CTRL_RB_TX_SRC_CURR_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_SRC_CURR_ADDR_H_OFF)

/* 当前的目的端地址 0xC38 */
typedef union {
    struct {
        unsigned int tx_dst_curr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_dst_curr_addr;
#define HOST_CTRL_RB_TX_DST_CURR_ADDR_OFF    0xC38
#define HOST_CTRL_RB_TX_DST_CURR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_DST_CURR_ADDR_OFF)

/* 当前的源端描述符地址 0xC3C */
typedef union {
    struct {
        unsigned int rx_src_desc_curr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_src_desc_curr_addr;
#define HOST_CTRL_RB_RX_SRC_DESC_CURR_ADDR_OFF    0xC3C
#define HOST_CTRL_RB_RX_SRC_DESC_CURR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_SRC_DESC_CURR_ADDR_OFF)

/* 当前的源端地址 0xC40 */
typedef union {
    struct {
        unsigned int rx_src_curr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_src_curr_addr;
#define HOST_CTRL_RB_RX_SRC_CURR_ADDR_OFF    0xC40
#define HOST_CTRL_RB_RX_SRC_CURR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_SRC_CURR_ADDR_OFF)

/* 当前的目的端地址 0xC44 */
typedef union {
    struct {
        unsigned int rx_dst_curr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_dst_curr_addr;
#define HOST_CTRL_RB_RX_DST_CURR_ADDR_OFF    0xC44
#define HOST_CTRL_RB_RX_DST_CURR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_DST_CURR_ADDR_OFF)

/* 当前的目的端地址 0xC48 */
typedef union {
    struct {
        unsigned int rx_dst_curr_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_dst_curr_addr_h;
#define HOST_CTRL_RB_RX_DST_CURR_ADDR_H_OFF    0xC48
#define HOST_CTRL_RB_RX_DST_CURR_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_DST_CURR_ADDR_H_OFF)

/* TX方向数采控制 0xC4C */
typedef union {
    struct {
        unsigned int tx_ete_mon_en : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_tx_ete_mon_cfg;
#define HOST_CTRL_RB_TX_ETE_MON_CFG_OFF    0xC4C
#define HOST_CTRL_RB_TX_ETE_MON_CFG_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_TX_ETE_MON_CFG_OFF)

/* RX方向数采控制 0xC54 */
typedef union {
    struct {
        unsigned int rx_ete_mon_en : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ete_mon_cfg;
#define HOST_CTRL_RB_RX_ETE_MON_CFG_OFF    0xC54
#define HOST_CTRL_RB_RX_ETE_MON_CFG_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_ETE_MON_CFG_OFF)

/* host端包计数写地址 0xC60 */
typedef union {
    struct {
        unsigned int rx_ch0_pcie0_host_ddr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_pcie0_host_ddr_addr;
#define HOST_CTRL_RB_RX_CH0_PCIE0_HOST_DDR_ADDR_OFF    0xC60
#define HOST_CTRL_RB_RX_CH0_PCIE0_HOST_DDR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_PCIE0_HOST_DDR_ADDR_OFF)

/* host端包计数写地址 0xC64 */
typedef union {
    struct {
        unsigned int rx_ch0_pcie0_host_ddr_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_pcie0_host_ddr_addr_h;
#define HOST_CTRL_RB_RX_CH0_PCIE0_HOST_DDR_ADDR_H_OFF    0xC64
#define HOST_CTRL_RB_RX_CH0_PCIE0_HOST_DDR_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_PCIE0_HOST_DDR_ADDR_H_OFF)

/* host端包计数写地址 0xC68 */
typedef union {
    struct {
        unsigned int rx_ch1_pcie0_host_ddr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_pcie0_host_ddr_addr;
#define HOST_CTRL_RB_RX_CH1_PCIE0_HOST_DDR_ADDR_OFF    0xC68
#define HOST_CTRL_RB_RX_CH1_PCIE0_HOST_DDR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_PCIE0_HOST_DDR_ADDR_OFF)

/* host端包计数写地址 0xC6C */
typedef union {
    struct {
        unsigned int rx_ch1_pcie0_host_ddr_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_pcie0_host_ddr_addr_h;
#define HOST_CTRL_RB_RX_CH1_PCIE0_HOST_DDR_ADDR_H_OFF    0xC6C
#define HOST_CTRL_RB_RX_CH1_PCIE0_HOST_DDR_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_PCIE0_HOST_DDR_ADDR_H_OFF)

/* host端包计数写地址 0xC70 */
typedef union {
    struct {
        unsigned int rx_ch2_pcie0_host_ddr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_pcie0_host_ddr_addr;
#define HOST_CTRL_RB_RX_CH2_PCIE0_HOST_DDR_ADDR_OFF    0xC70
#define HOST_CTRL_RB_RX_CH2_PCIE0_HOST_DDR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_PCIE0_HOST_DDR_ADDR_OFF)

/* host端包计数写地址 0xC74 */
typedef union {
    struct {
        unsigned int rx_ch2_pcie0_host_ddr_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_pcie0_host_ddr_addr_h;
#define HOST_CTRL_RB_RX_CH2_PCIE0_HOST_DDR_ADDR_H_OFF    0xC74
#define HOST_CTRL_RB_RX_CH2_PCIE0_HOST_DDR_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_PCIE0_HOST_DDR_ADDR_H_OFF)

/* host端包计数写地址 0xC78 */
typedef union {
    struct {
        unsigned int rx_ch3_pcie0_host_ddr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_pcie0_host_ddr_addr;
#define HOST_CTRL_RB_RX_CH3_PCIE0_HOST_DDR_ADDR_OFF    0xC78
#define HOST_CTRL_RB_RX_CH3_PCIE0_HOST_DDR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_PCIE0_HOST_DDR_ADDR_OFF)

/* host端包计数写地址 0xC7C */
typedef union {
    struct {
        unsigned int rx_ch3_pcie0_host_ddr_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_pcie0_host_ddr_addr_h;
#define HOST_CTRL_RB_RX_CH3_PCIE0_HOST_DDR_ADDR_H_OFF    0xC7C
#define HOST_CTRL_RB_RX_CH3_PCIE0_HOST_DDR_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_PCIE0_HOST_DDR_ADDR_H_OFF)

/* host端包计数写地址 0xC80 */
typedef union {
    struct {
        unsigned int rx_ch4_pcie0_host_ddr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_pcie0_host_ddr_addr;
#define HOST_CTRL_RB_RX_CH4_PCIE0_HOST_DDR_ADDR_OFF    0xC80
#define HOST_CTRL_RB_RX_CH4_PCIE0_HOST_DDR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_PCIE0_HOST_DDR_ADDR_OFF)

/* host端包计数写地址 0xC84 */
typedef union {
    struct {
        unsigned int rx_ch4_pcie0_host_ddr_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_pcie0_host_ddr_addr_h;
#define HOST_CTRL_RB_RX_CH4_PCIE0_HOST_DDR_ADDR_H_OFF    0xC84
#define HOST_CTRL_RB_RX_CH4_PCIE0_HOST_DDR_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_PCIE0_HOST_DDR_ADDR_H_OFF)

/* host端包计数写地址 0xC88 */
typedef union {
    struct {
        unsigned int rx_ch5_pcie0_host_ddr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_pcie0_host_ddr_addr;
#define HOST_CTRL_RB_RX_CH5_PCIE0_HOST_DDR_ADDR_OFF    0xC88
#define HOST_CTRL_RB_RX_CH5_PCIE0_HOST_DDR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_PCIE0_HOST_DDR_ADDR_OFF)

/* host端包计数写地址 0xC8C */
typedef union {
    struct {
        unsigned int rx_ch5_pcie0_host_ddr_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_pcie0_host_ddr_addr_h;
#define HOST_CTRL_RB_RX_CH5_PCIE0_HOST_DDR_ADDR_H_OFF    0xC8C
#define HOST_CTRL_RB_RX_CH5_PCIE0_HOST_DDR_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_PCIE0_HOST_DDR_ADDR_H_OFF)

/* host端包计数写地址 0xC90 */
typedef union {
    struct {
        unsigned int rx_ch0_pcie1_host_ddr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_pcie1_host_ddr_addr;
#define HOST_CTRL_RB_RX_CH0_PCIE1_HOST_DDR_ADDR_OFF    0xC90
#define HOST_CTRL_RB_RX_CH0_PCIE1_HOST_DDR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_PCIE1_HOST_DDR_ADDR_OFF)

/* host端包计数写地址 0xC94 */
typedef union {
    struct {
        unsigned int rx_ch0_pcie1_host_ddr_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch0_pcie1_host_ddr_addr_h;
#define HOST_CTRL_RB_RX_CH0_PCIE1_HOST_DDR_ADDR_H_OFF    0xC94
#define HOST_CTRL_RB_RX_CH0_PCIE1_HOST_DDR_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH0_PCIE1_HOST_DDR_ADDR_H_OFF)

/* host端包计数写地址 0xC98 */
typedef union {
    struct {
        unsigned int rx_ch1_pcie1_host_ddr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_pcie1_host_ddr_addr;
#define HOST_CTRL_RB_RX_CH1_PCIE1_HOST_DDR_ADDR_OFF    0xC98
#define HOST_CTRL_RB_RX_CH1_PCIE1_HOST_DDR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_PCIE1_HOST_DDR_ADDR_OFF)

/* host端包计数写地址 0xC9C */
typedef union {
    struct {
        unsigned int rx_ch1_pcie1_host_ddr_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch1_pcie1_host_ddr_addr_h;
#define HOST_CTRL_RB_RX_CH1_PCIE1_HOST_DDR_ADDR_H_OFF    0xC9C
#define HOST_CTRL_RB_RX_CH1_PCIE1_HOST_DDR_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH1_PCIE1_HOST_DDR_ADDR_H_OFF)

/* host端包计数写地址 0xCA0 */
typedef union {
    struct {
        unsigned int rx_ch2_pcie1_host_ddr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_pcie1_host_ddr_addr;
#define HOST_CTRL_RB_RX_CH2_PCIE1_HOST_DDR_ADDR_OFF    0xCA0
#define HOST_CTRL_RB_RX_CH2_PCIE1_HOST_DDR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_PCIE1_HOST_DDR_ADDR_OFF)

/* host端包计数写地址 0xCA4 */
typedef union {
    struct {
        unsigned int rx_ch2_pcie1_host_ddr_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch2_pcie1_host_ddr_addr_h;
#define HOST_CTRL_RB_RX_CH2_PCIE1_HOST_DDR_ADDR_H_OFF    0xCA4
#define HOST_CTRL_RB_RX_CH2_PCIE1_HOST_DDR_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH2_PCIE1_HOST_DDR_ADDR_H_OFF)

/* host端包计数写地址 0xCA8 */
typedef union {
    struct {
        unsigned int rx_ch3_pcie1_host_ddr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_pcie1_host_ddr_addr;
#define HOST_CTRL_RB_RX_CH3_PCIE1_HOST_DDR_ADDR_OFF    0xCA8
#define HOST_CTRL_RB_RX_CH3_PCIE1_HOST_DDR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_PCIE1_HOST_DDR_ADDR_OFF)

/* host端包计数写地址 0xCAC */
typedef union {
    struct {
        unsigned int rx_ch3_pcie1_host_ddr_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch3_pcie1_host_ddr_addr_h;
#define HOST_CTRL_RB_RX_CH3_PCIE1_HOST_DDR_ADDR_H_OFF    0xCAC
#define HOST_CTRL_RB_RX_CH3_PCIE1_HOST_DDR_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH3_PCIE1_HOST_DDR_ADDR_H_OFF)

/* host端包计数写地址 0xCB0 */
typedef union {
    struct {
        unsigned int rx_ch4_pcie1_host_ddr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_pcie1_host_ddr_addr;
#define HOST_CTRL_RB_RX_CH4_PCIE1_HOST_DDR_ADDR_OFF    0xCB0
#define HOST_CTRL_RB_RX_CH4_PCIE1_HOST_DDR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_PCIE1_HOST_DDR_ADDR_OFF)

/* host端包计数写地址 0xCB4 */
typedef union {
    struct {
        unsigned int rx_ch4_pcie1_host_ddr_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch4_pcie1_host_ddr_addr_h;
#define HOST_CTRL_RB_RX_CH4_PCIE1_HOST_DDR_ADDR_H_OFF    0xCB4
#define HOST_CTRL_RB_RX_CH4_PCIE1_HOST_DDR_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH4_PCIE1_HOST_DDR_ADDR_H_OFF)

/* host端包计数写地址 0xCB8 */
typedef union {
    struct {
        unsigned int rx_ch5_pcie1_host_ddr_addr : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_pcie1_host_ddr_addr;
#define HOST_CTRL_RB_RX_CH5_PCIE1_HOST_DDR_ADDR_OFF    0xCB8
#define HOST_CTRL_RB_RX_CH5_PCIE1_HOST_DDR_ADDR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_PCIE1_HOST_DDR_ADDR_OFF)

/* host端包计数写地址 0xCBC */
typedef union {
    struct {
        unsigned int rx_ch5_pcie1_host_ddr_addr_h : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch5_pcie1_host_ddr_addr_h;
#define HOST_CTRL_RB_RX_CH5_PCIE1_HOST_DDR_ADDR_H_OFF    0xCBC
#define HOST_CTRL_RB_RX_CH5_PCIE1_HOST_DDR_ADDR_H_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH5_PCIE1_HOST_DDR_ADDR_H_OFF)

/* host端包计数写使能 0xCC0 */
typedef union {
    struct {
        unsigned int rx_ch_wr_end_sel : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_rx_ch_wr_end_sel;
#define HOST_CTRL_RB_RX_CH_WR_END_SEL_OFF    0xCC0
#define HOST_CTRL_RB_RX_CH_WR_END_SEL_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_RX_CH_WR_END_SEL_OFF)

/*  0xE00 */
typedef union {
    struct {
        unsigned int host_aux_mac_div_num : 7; /* 0:6 */
        unsigned int reserved0 : 1; /* 7 */
        unsigned int host_aux_phy_div_num : 7; /* 8:14 */
        unsigned int reserved1 : 1; /* 15 */
        unsigned int host_aux_div_en : 1; /* 16 */
        unsigned int reserved2 : 3; /* 17:19 */
        unsigned int host_aux_mac_tcxo_rtc_clkb_on_sts : 1; /* 20 */
        unsigned int host_aux_mac_tcxo_rtc_clka_on_sts : 1; /* 21 */
        unsigned int reserved3 : 2; /* 22:23 */
        unsigned int host_aux_phy_tcxo_rtc_clkb_on_sts : 1; /* 24 */
        unsigned int host_aux_phy_tcxo_rtc_clka_on_sts : 1; /* 25 */
        unsigned int reserved4 : 6; /* 26:31 */
    } bits;
    unsigned int as_dword;
} hreg_host_sub_crg_aux_div_ctl;
#define HOST_CTRL_RB_HOST_SUB_CRG_AUX_DIV_CTL_OFF    0xE00
#define HOST_CTRL_RB_HOST_SUB_CRG_AUX_DIV_CTL_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_HOST_SUB_CRG_AUX_DIV_CTL_OFF)

/*  0xE04 */
typedef union {
    struct {
        unsigned int pcie_phy_div_num : 3; /* 0:2 */
        unsigned int reserved0 : 1; /* 3 */
        unsigned int pcie_phy_div_en : 1; /* 4 */
        unsigned int reserved1 : 3; /* 5:7 */
        unsigned int pcie_phy_pclk_en : 1; /* 8 */
        unsigned int reserved2 : 23; /* 9:31 */
    } bits;
    unsigned int as_dword;
} hreg_host_sub_crg_dft_pclk_div_ctl;
#define HOST_CTRL_RB_HOST_SUB_CRG_DFT_PCLK_DIV_CTL_OFF    0xE04
#define HOST_CTRL_RB_HOST_SUB_CRG_DFT_PCLK_DIV_CTL_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_HOST_SUB_CRG_DFT_PCLK_DIV_CTL_OFF)

/*  0xE08 */
typedef union {
    struct {
        unsigned int soc_slv_araddr_m0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_slv_addr_cfg0_m0;
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_M0_OFF    0xE08
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_M0_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_M0_OFF)

/*  0xE0C */
typedef union {
    struct {
        unsigned int soc_slv_awaddr_m0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_slv_addr_cfg1_m0;
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_M0_OFF    0xE0C
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_M0_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_M0_OFF)

/*  0xE10 */
typedef union {
    struct {
        unsigned int soc_slv_araddr_w0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_slv_addr_cfg0_w0;
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_W0_OFF    0xE10
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_W0_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_W0_OFF)

/*  0xE14 */
typedef union {
    struct {
        unsigned int soc_slv_awaddr_w0 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_slv_addr_cfg1_w0;
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_W0_OFF    0xE14
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_W0_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_W0_OFF)

/*  0xE18 */
typedef union {
    struct {
        unsigned int soc_slv_araddr_w1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_slv_addr_cfg0_w1;
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_W1_OFF    0xE18
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_W1_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_W1_OFF)

/*  0xE1C */
typedef union {
    struct {
        unsigned int soc_slv_awaddr_w1 : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_slv_addr_cfg1_w1;
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_W1_OFF    0xE1C
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_W1_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_W1_OFF)

/*  0xE20 */
typedef union {
    struct {
        unsigned int soc_slv_araddr_aon : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_slv_addr_cfg0_aon;
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_AON_OFF    0xE20
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_AON_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_AON_OFF)

/*  0xE24 */
typedef union {
    struct {
        unsigned int soc_slv_awaddr_aon : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_slv_addr_cfg1_aon;
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_AON_OFF    0xE24
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_AON_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_AON_OFF)

/*  0xE28 */
typedef union {
    struct {
        unsigned int soc_slv_araddr_dma : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_slv_addr_cfg0_dma;
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_DMA_OFF    0xE28
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_DMA_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_DMA_OFF)

/*  0xE2C */
typedef union {
    struct {
        unsigned int soc_slv_awaddr_dma : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_slv_addr_cfg1_dma;
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_DMA_OFF    0xE2C
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_DMA_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_DMA_OFF)

/*  0xE30 */
typedef union {
    struct {
        unsigned int soc_slv_araddr_sdio : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_slv_addr_cfg0_sdio;
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_SDIO_OFF    0xE30
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_SDIO_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_PCIE_SLV_ADDR_CFG0_SDIO_OFF)

/*  0xE34 */
typedef union {
    struct {
        unsigned int soc_slv_awaddr_sdio : 32; /* 0:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_slv_addr_cfg1_sdio;
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_SDIO_OFF    0xE34
#define HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_SDIO_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_PCIE_SLV_ADDR_CFG1_SDIO_OFF)

/* HOST_AXI总线优先级配置 0xE60 */
typedef union {
    struct {
        unsigned int host_axi_m1_priority : 2; /* 0:1 */
        unsigned int host_axi_m2_priority : 2; /* 2:3 */
        unsigned int host_axi_m3_priority : 2; /* 4:5 */
        unsigned int reserved0 : 2; /* 6:7 */
        unsigned int host_axi_s1_priority : 2; /* 8:9 */
        unsigned int host_axi_s2_priority : 2; /* 10:11 */
        unsigned int host_axi_s3_priority : 2; /* 12:13 */
        unsigned int reserved1 : 18; /* 14:31 */
    } bits;
    unsigned int as_dword;
} hreg_host_axi_priority_cfg;
#define HOST_CTRL_RB_HOST_AXI_PRIORITY_CFG_OFF    0xE60
#define HOST_CTRL_RB_HOST_AXI_PRIORITY_CFG_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_HOST_AXI_PRIORITY_CFG_OFF)

/*  0xE64 */
typedef union {
    struct {
        unsigned int cfg_grant_cnt : 5; /* 0:4 */
        unsigned int rid_grant_sel : 1; /* 5 */
        unsigned int reserved0 : 26; /* 6:31 */
    } bits;
    unsigned int as_dword;
} hreg_remap_rd_cfg;
#define HOST_CTRL_RB_REMAP_RD_CFG_OFF    0xE64
#define HOST_CTRL_RB_REMAP_RD_CFG_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_REMAP_RD_CFG_OFF)

/*  0xE68 */
typedef union {
    struct {
        unsigned int awbuf_flow_clr : 1; /* 0 */
        unsigned int arbuf_flow_clr : 1; /* 1 */
        unsigned int slv1_fifo_flow_clr : 1; /* 2 */
        unsigned int slv2_fifo_flow_clr : 1; /* 3 */
        unsigned int reserved0 : 28; /* 4:31 */
    } bits;
    unsigned int as_dword;
} hreg_remap_fifo_flow_clr;
#define HOST_CTRL_RB_REMAP_FIFO_FLOW_CLR_OFF    0xE68
#define HOST_CTRL_RB_REMAP_FIFO_FLOW_CLR_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_REMAP_FIFO_FLOW_CLR_OFF)

/*  0xE6C */
typedef union {
    struct {
        unsigned int awbuf_full : 1; /* 0 */
        unsigned int awbuf_empty : 1; /* 1 */
        unsigned int awbuf_wflow : 1; /* 2 */
        unsigned int awbuf_rflow : 1; /* 3 */
        unsigned int arbuf_full : 1; /* 4 */
        unsigned int arbuf_empty : 1; /* 5 */
        unsigned int arbuf_wflow : 1; /* 6 */
        unsigned int arbuf_rflow : 1; /* 7 */
        unsigned int slv1_fifo_full : 1; /* 8 */
        unsigned int slv1_fifo_empty : 1; /* 9 */
        unsigned int slv1_fifo_wflow : 1; /* 10 */
        unsigned int slv1_fifo_rflow : 1; /* 11 */
        unsigned int slv2_fifo_full : 1; /* 12 */
        unsigned int slv2_fifo_empty : 1; /* 13 */
        unsigned int slv2_fifo_wflow : 1; /* 14 */
        unsigned int slv2_fifo_rflow : 1; /* 15 */
        unsigned int reserved0 : 16; /* 16:31 */
    } bits;
    unsigned int as_dword;
} hreg_remap_fifo_flow_sts;
#define HOST_CTRL_RB_REMAP_FIFO_FLOW_STS_OFF    0xE6C
#define HOST_CTRL_RB_REMAP_FIFO_FLOW_STS_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_REMAP_FIFO_FLOW_STS_OFF)

/*  0xE70 */
typedef union {
    struct {
        unsigned int awid_sel0 : 12; /* 0:11 */
        unsigned int reserved0 : 4; /* 12:15 */
        unsigned int awid_sel1 : 12; /* 16:27 */
        unsigned int reserved1 : 4; /* 28:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_remap_wr_cfg0;
#define HOST_CTRL_RB_PCIE_REMAP_WR_CFG0_OFF    0xE70
#define HOST_CTRL_RB_PCIE_REMAP_WR_CFG0_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_PCIE_REMAP_WR_CFG0_OFF)

/*  0xE74 */
typedef union {
    struct {
        unsigned int awid_sel_enable : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_pcie_remap_wr_cfg1;
#define HOST_CTRL_RB_PCIE_REMAP_WR_CFG1_OFF    0xE74
#define HOST_CTRL_RB_PCIE_REMAP_WR_CFG1_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_PCIE_REMAP_WR_CFG1_OFF)

/* HOST工作模式配置 0xE78 */
typedef union {
    struct {
        unsigned int cfg_host_mode : 1; /* 0 */
        unsigned int reserved0 : 31; /* 1:31 */
    } bits;
    unsigned int as_dword;
} hreg_cfg_host_mode;
#define HOST_CTRL_RB_CFG_HOST_MODE_OFF    0xE78
#define HOST_CTRL_RB_CFG_HOST_MODE_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_CFG_HOST_MODE_OFF)

/*  0xE7C */
typedef union {
    struct {
        unsigned int host_axi_csysreq : 1; /* 0 */
        unsigned int reserved0 : 3; /* 1:3 */
        unsigned int host_axi_clken_sel : 1; /* 4 */
        unsigned int reserved1 : 3; /* 5:7 */
        unsigned int remap_clken : 1; /* 8 */
        unsigned int reserved2 : 23; /* 9:31 */
    } bits;
    unsigned int as_dword;
} hreg_host_axi_low_power_cfg;
#define HOST_CTRL_RB_HOST_AXI_LOW_POWER_CFG_OFF    0xE7C
#define HOST_CTRL_RB_HOST_AXI_LOW_POWER_CFG_REG \
    (HOST_CTRL_RB_BASE + HOST_CTRL_RB_HOST_AXI_LOW_POWER_CFG_OFF)

#endif
