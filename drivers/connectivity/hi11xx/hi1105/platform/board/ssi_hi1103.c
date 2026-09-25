

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG

/* 头文件包含 */
#include "ssi_hi1103.h"

#include "board.h"
#include "plat_debug.h"
#include "plat_pm.h"
#include "ssi_common.h"


#define TCXO_32K_DET_VALUE 10
#define TCXO_LIMIT_THRESHOLD 5
#define TCXO_GATING_CLK 76800000 /* 默认时钟 */
#define TCXO_NOMAL_CKL  38400000

static uint32 halt_det_cnt = 0; /* 检测soc异常次数 */
static ssi_cpu_infos st_ssi_cpu_infos;

static char *ssi_hi1103_pilot_cpu_st_str[] = {
    "OFF",              /* 0x0 */
    "BOOTING",          /* 0x1 */
    "SLEEPING",         /* 0x2 */
    "WORK",             /* 0x3 */
    "SAVING",           /* 0x4 */
    "PROTECT(ocp/scp)", /* 0x5 */
    "SLEEP",            /* 0x6 */
    "PROTECTING"        /* 0x7 */
};

static ssi_reg_info hi1103_glb_ctrl_full = { 0x50000000, 0x1000, SSI_RW_WORD_MOD };
static ssi_reg_info hi1103_glb_ctrl_extend1 = { 0x50001400, 0x10,   SSI_RW_WORD_MOD };
static ssi_reg_info hi1103_glb_ctrl_extend2 = { 0x50001540, 0xc,    SSI_RW_WORD_MOD };
static ssi_reg_info hi1103_glb_ctrl_extend3 = { 0x50001600, 0x4,    SSI_RW_WORD_MOD };
static ssi_reg_info hi1103_pmu_cmu_ctrl_full = { 0x50002000, 0xb00,  SSI_RW_WORD_MOD };
static ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_full = { 0x50003000, 0xa20,  SSI_RW_WORD_MOD };
static ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_tail = { 0x50003a80, 0xc,    SSI_RW_WORD_MOD };
static ssi_reg_info hi1103_coex_ctl_part1 = { 0x5000a000, 0x354,  SSI_RW_WORD_MOD }; /* coex ctl part1 */
static ssi_reg_info hi1103_coex_ctl_part2 = { 0x5000a500, 0x8,    SSI_RW_WORD_MOD };   /* coex ctl part2 */
static ssi_reg_info hi1103_w_ctrl_full = { 0x40000000, 0x408,  SSI_RW_WORD_MOD };
static ssi_reg_info hi1103_w_key_mem = { 0x2001e620, 0x80,   SSI_RW_DWORD_MOD };
static ssi_reg_info hi1103_b_except_mem  =    {0x8021ab80, 0x64, SSI_RW_DWORD_MOD};
static ssi_reg_info hi1103_b_ctrl_full = { 0x48000000, 0x40c,  SSI_RW_WORD_MOD };
static ssi_reg_info hi1103_pcie_ctrl_full = { 0x40007000, 0x4c8,  SSI_RW_DWORD_MOD };
static ssi_reg_info hi1103_pcie_dbi_full = { 0x40102000, 0x900,  SSI_RW_DWORD_MOD };         /* 没建链之前不能读 */
static ssi_reg_info hi1103_pcie_pilot_dma_full = { 0x40106000, 0x1000, SSI_RW_DWORD_MOD };  /* 4KB */
static ssi_reg_info hi1103_pcie_dma_ctrl_full = { 0x40008000, 0x34,   SSI_RW_DWORD_MOD };
static ssi_reg_info hi1103_pcie_sdio_ctrl_full = { 0x40101000, 0x180,  SSI_RW_DWORD_MOD };
static ssi_reg_info hi1103_WIFI_GPIO0_full = {0x50004000, 0x74, SSI_RW_WORD_MOD};
static ssi_reg_info hi1103_WIFI_GPIO1_full = {0x5000c000, 0x74, SSI_RW_WORD_MOD};
static ssi_reg_info hi1103_BFG_GPIO0_full = {0x50005000, 0x74, SSI_RW_WORD_MOD};
static ssi_reg_info hi1103_BFG_GPIO1_full = {0x5000d000, 0x74, SSI_RW_WORD_MOD};

static ssi_reg_info hi1103_rf_w_c0_full = {0x40040000, 0x314, SSI_RW_WORD_MOD};
static ssi_reg_info hi1103_rf_w_c1_full = {0x40042000, 0x314, SSI_RW_WORD_MOD};
static ssi_reg_info hi1103_rf_bfg_full = {0x4800C000, 0xB24, SSI_RW_WORD_MOD};

static ssi_reg_info hi1103_tcxo_detect_reg1 = { 0x50000040, 0x4,  SSI_RW_WORD_MOD };
static ssi_reg_info hi1103_tcxo_detect_reg2 = { 0x500000c0, 0x14, SSI_RW_WORD_MOD };
static ssi_reg_info hi1103_tcxo_detect_reg3 = { 0x50000700, 0xc,  SSI_RW_WORD_MOD };

static ssi_reg_info *hi1103_aon_reg_full[] = {
    &hi1103_glb_ctrl_full,
    &hi1103_glb_ctrl_extend1,
    &hi1103_glb_ctrl_extend2,
    &hi1103_glb_ctrl_extend3,
    &hi1103_pmu_cmu_ctrl_full,
    &hi1103_pmu2_cmu_ir_ctrl_full,
    &hi1103_pmu2_cmu_ir_ctrl_tail,
    &hi1103_WIFI_GPIO0_full,
    &hi1103_WIFI_GPIO1_full,
    &hi1103_BFG_GPIO0_full,
    &hi1103_BFG_GPIO1_full
};

static ssi_reg_info *hi1103_coex_ctl_full[] = {
    &hi1103_coex_ctl_part1,
    &hi1103_coex_ctl_part2,
};

static ssi_reg_info *hi1103_tcxo_detect_regs[] = {
    &hi1103_tcxo_detect_reg1,
    &hi1103_tcxo_detect_reg2,
    &hi1103_tcxo_detect_reg3
};

// 0x5000_0000~0x500000FC
static ssi_reg_info hi1103_glb_ctrl_cut1 = { 0x50000000, 0xfc, SSI_RW_WORD_MOD };
// 0x5000_0200~0x5000_020C
static ssi_reg_info hi1103_glb_ctrl_cut2 = { 0x50000200, 0xc, SSI_RW_WORD_MOD };
// 0x5000_0400~0x5000_043C
static ssi_reg_info hi1103_glb_ctrl_cut3 = { 0x50000400, 0x3c, SSI_RW_WORD_MOD };
// 0x5000_0500~0x5000_051C
static ssi_reg_info hi1103_glb_ctrl_cut4 = { 0x50000500, 0x1c, SSI_RW_WORD_MOD };
// 0x5000_0700~0x5000_070C
static ssi_reg_info hi1103_glb_ctrl_cut5 = { 0x50000700, 0xc, SSI_RW_WORD_MOD };
// 0x5000_0E00~0x5000_0E0C
static ssi_reg_info hi1103_glb_ctrl_cut6 = { 0x50000E00, 0xc, SSI_RW_WORD_MOD };
// 0x5000_1400~0x5000_140C
static ssi_reg_info hi1103_glb_ctrl_cut7 = { 0x50001400, 0x10, SSI_RW_WORD_MOD };
// 0x5000_1540~0x5000_1548
static ssi_reg_info hi1103_glb_ctrl_cut8 = { 0x50001540, 0xc, SSI_RW_WORD_MOD };
// 0x5000_1600~0x5000_1604
static ssi_reg_info hi1103_glb_ctrl_cut9 = { 0x50001600, 0x4, SSI_RW_WORD_MOD };

// PMU_CMU_CTRL
// 0x50002080~0x500021AC
static ssi_reg_info hi1103_pmu_cmu_ctrl_cut1 = { 0x50002080, 0x12c, SSI_RW_WORD_MOD };
// 0x50002200~0x5000220C
static ssi_reg_info hi1103_pmu_cmu_ctrl_cut2 = { 0x50002200, 0xc, SSI_RW_WORD_MOD };
// 0x50002380~0x5000239C
static ssi_reg_info hi1103_pmu_cmu_ctrl_cut3 = { 0x50002380, 0x1c, SSI_RW_WORD_MOD };
// 0x50002800~0x5000283C
static ssi_reg_info hi1103_pmu_cmu_ctrl_cut4 = { 0x50002800, 0x3c, SSI_RW_WORD_MOD };

// PMU2_CMU_IR_TS_EF_CTL
// 0x50003040~0x5000307C
static ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut1 = { 0x50003040, 0x3c, SSI_RW_WORD_MOD };
// 0x5000311C
static ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut2 = { 0x5000311C, 0x4, SSI_RW_WORD_MOD };
// 0x5000313C
static ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut3 = { 0x5000313C, 0x4, SSI_RW_WORD_MOD };
// 0x5000315C
static ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut4 = { 0x5000315C, 0x4, SSI_RW_WORD_MOD };
// 0x5000317C
static ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut5 = { 0x5000317C, 0x4, SSI_RW_WORD_MOD };
// 0x5000319C
static ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut6 = { 0x5000319C, 0x4, SSI_RW_WORD_MOD };
// 0x50003220~0x5000339C
static ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut7 = { 0x50003220, 0x17c, SSI_RW_WORD_MOD };
// 0x50003420~0x5000343C
static ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut8 = { 0x50003420, 0x1c, SSI_RW_WORD_MOD };
// 0x50003780~0x500037FC
static ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut9 = { 0x50003780, 0x7c, SSI_RW_WORD_MOD };
// 0x50003800~0x500038BF
static ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut10 = { 0x50003800, 0xc0, SSI_RW_WORD_MOD };
// 0x50003A80~0x50003A8C
static ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut11 = { 0x50003A80, 0xc, SSI_RW_WORD_MOD };

static ssi_reg_info *hi1103_aon_reg_cut[] = {
    &hi1103_glb_ctrl_cut1,
    &hi1103_glb_ctrl_cut2,
    &hi1103_glb_ctrl_cut3,
    &hi1103_glb_ctrl_cut4,
    &hi1103_glb_ctrl_cut5,
    &hi1103_glb_ctrl_cut6,
    &hi1103_glb_ctrl_cut7,
    &hi1103_glb_ctrl_cut8,
    &hi1103_glb_ctrl_cut9,
    &hi1103_pmu_cmu_ctrl_cut1,
    &hi1103_pmu_cmu_ctrl_cut2,
    &hi1103_pmu_cmu_ctrl_cut3,
    &hi1103_pmu_cmu_ctrl_cut4,
    &hi1103_pmu2_cmu_ir_ctrl_cut1,
    &hi1103_pmu2_cmu_ir_ctrl_cut2,
    &hi1103_pmu2_cmu_ir_ctrl_cut3,
    &hi1103_pmu2_cmu_ir_ctrl_cut4,
    &hi1103_pmu2_cmu_ir_ctrl_cut5,
    &hi1103_pmu2_cmu_ir_ctrl_cut6,
    &hi1103_pmu2_cmu_ir_ctrl_cut7,
    &hi1103_pmu2_cmu_ir_ctrl_cut8,
    &hi1103_pmu2_cmu_ir_ctrl_cut9,
    &hi1103_pmu2_cmu_ir_ctrl_cut10,
    &hi1103_pmu2_cmu_ir_ctrl_cut11
};

static ssi_reg_info hi1103_pcie_ctrl_cut1 = { 0x40007224, 0x4,  SSI_RW_DWORD_MOD };
static ssi_reg_info hi1103_pcie_ctrl_cut2 = { 0x400072d0, 0x4,  SSI_RW_DWORD_MOD };
static ssi_reg_info hi1103_pcie_ctrl_cut3 = { 0x40007430, 0x9c, SSI_RW_DWORD_MOD };

static ssi_reg_info *hi1103_pcie_cfg_reg_cut[] = {
    &hi1103_pcie_ctrl_cut1,
    &hi1103_pcie_ctrl_cut2,
    &hi1103_pcie_ctrl_cut3
};

static ssi_reg_info *hi1103_pcie_cfg_reg_full[] = {
    &hi1103_pcie_ctrl_full,
    &hi1103_pcie_dma_ctrl_full
};

static ssi_reg_info *hi1103_pcie_dbi_pilot_reg_full[] = {
    &hi1103_pcie_dbi_full,
    &hi1103_pcie_pilot_dma_full,
};

/* gnss_only uart_cfg */
static ssi_file_st aSsiFile[] = {
#ifdef BFGX_UART_DOWNLOAD_SUPPORT
    /* gnss only */
    { "/system/vendor/firmware/RAM_VECTOR.bin", 0x80100800 },
    { "/system/vendor/firmware/CPU_RAM_SCHED.bin", 0x80004000 },
#else
    /* mpw2 */
    { "/system/vendor/firmware/BCPU_ROM.bin",           0x80000000 },
    { "/system/vendor/firmware/VECTORS.bin",            0x80010000 },
    { "/system/vendor/firmware/RAM_VECTOR.bin",         0x80105c00 },
    { "/system/vendor/firmware/WCPU_ROM.bin",           0x4000 },
    { "/system/vendor/firmware/WL_ITCM.bin",            0x10000 },
    { "/system/vendor/firmware/PLAT_RAM_EXCEPTION.bin", 0x20002800 },
#endif
};

static void ssi_detect_tcxo_handle(uint32 tcxo_det_res_old,
                                        uint32 tcxo_det_res_new,
                                        oal_uint64 base_tcxo_clock,
                                        uint32 tcxo_det_value_target,
                                        oal_uint32 clock_32k)
{
    char *tcxo_str = "";
    int tcxo_is_abnormal = 0;
    oal_uint64 clock_tcxo = 0;
    oal_uint64 div_clock = 0;
    oal_uint64 tcxo_limit_low, tcxo_limit_high, tcxo_tmp;

    /* 为了计算误差范围 */
    tcxo_tmp = div_u64(base_tcxo_clock, 100);
    tcxo_limit_low = (tcxo_tmp * (100 - TCXO_LIMIT_THRESHOLD));
    tcxo_limit_high = (tcxo_tmp * (100 + TCXO_LIMIT_THRESHOLD));

    if (tcxo_det_res_new == tcxo_det_res_old) {
        /* tcxo 软件配置为打开此时应该有时钟 */
        PS_PRINT_ERR("tcxo don't change after detect, tcxo or 32k maybe abnormal, tcxo=0x%x,32k_clock=%u\n",
                       tcxo_det_res_new, clock_32k);
        if (tcxo_det_res_new == 0) {
            tcxo_is_abnormal = 1;
            tcxo_str = "non-tcxo";
        } else {
            /* 这里可能是无效的探测，要结合详细日志分析，此处DSM忽略改分支，不上报 */
            tcxo_is_abnormal = 0;
            tcxo_str = "tcxo-detect-invalid";
        }
    } else {
        /*
         * tcxo_det_res_new read from 16bit width register  <= 0xffff
         * (tcxo_det_res_new * 32768) = (检测到的计数周期数 * 32k时钟)
         */
        if (tcxo_det_value_target == 0) {
            PS_PRINT_ERR("tcxo_det_value_target is zero\n");
            return;
        }
        clock_tcxo = (oal_uint64)((tcxo_det_res_new * 32768) / (tcxo_det_value_target));
        div_clock = clock_tcxo;
        div_clock = div_u64(div_clock, 1000000); /* hz to Mhz */
        if ((clock_tcxo < tcxo_limit_low) || (clock_tcxo > tcxo_limit_high)) {
            /* 时钟误差超过阈值 */
            tcxo_is_abnormal = 2;
            tcxo_str = "tcxo clock-abnormal";
        } else {
            tcxo_is_abnormal = 0;
            tcxo_str = "tcxo normal";
        }
        PS_PRINT_ERR("%s real=%llu hz,%llu Mhz[base=%llu][limit:%llu~%llu]\n",
                     tcxo_str, clock_tcxo, div_clock, base_tcxo_clock, tcxo_limit_low, tcxo_limit_high);
    }

    /* tcxo detect abnormal, dmd report */
    if (hi11xx_kernel_crash == 0) {
        /* kernel is normal */
        if (tcxo_is_abnormal) {
#ifdef CONFIG_HUAWEI_DSM
            hw_1103_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_1103_TCXO_ERROR,
                "%s: tcxo=%llu[%llu][limit:%llu~%llu] 32k_clock=%lu,det_tick=0x%x value=0x%x\n",
                tcxo_str, clock_tcxo, base_tcxo_clock, tcxo_limit_low, tcxo_limit_high, clock_32k,
                tcxo_det_value_target, tcxo_det_res_new);
                halt_det_cnt++;
#endif
        }
    }
}

static int ssi_detect_tcxo_is_normal(void)
{
    /*
     * tcxo detect 依赖tcxo时钟，
     * 如果在启动后tcxo 异常那么tcxo_32k_det_result 为旧值
     * 如果在启动后32k异常 那么sytem_tick为旧值
     */
    int ret;
    uint32 reg;
    uint32 tcxo_enable;
    uint32 tcxo_det_value_src, tcxo_det_value_target;
    uint32 clock_32k = 0;
    uint32 sys_tick_old, sys_tick_new, pmu2_cmu_abb_sts_3, pmu2_cmu_abb_sts_2;
    uint32 tcxo_det_res_old, tcxo_det_res_new, cmu_reserve1;
    oal_uint64 base_tcxo_clock = TCXO_GATING_CLK;

    declare_time_cost_stru(cost);

    pmu2_cmu_abb_sts_3 = (uint32)ssi_read32_etc(HI1103_PMU2_CMU_IR_PMU2_CMU_ABB_STS_3_REG);
    pmu2_cmu_abb_sts_2 = (uint32)ssi_read32_etc(HI1103_PMU2_CMU_IR_PMU2_CMU_ABB_STS_2_REG);
    cmu_reserve1 = (uint32)ssi_read32_etc(HI1103_PMU2_CMU_IR_CMU_RESERVE1_REG);
    tcxo_det_value_src = (uint32)ssi_read32_etc(HI1103_GLB_CTL_TCXO_32K_DET_CNT_REG);

    if (cmu_reserve1 & (1 << 7)) { /* 0x50003338的bit7表示时钟频率选择 */
        base_tcxo_clock = TCXO_GATING_CLK;
    } else {
        base_tcxo_clock = TCXO_NOMAL_CKL;
    }

    if ((!(pmu2_cmu_abb_sts_3 & (1 << 7)))
        && (pmu2_cmu_abb_sts_2 & (1 << 14))) {
        /* tcxo enable */
        tcxo_enable = 1;
    } else {
        /* system maybe sleep, tcxo disable */
        tcxo_enable = 0;
        PS_PRINT_ERR("tcxo gating normal\n");
    }

    tcxo_det_value_target = TCXO_32K_DET_VALUE;
    if (tcxo_det_value_src == tcxo_det_value_target) {
        /* 刚做过detect,改变det_value，观测值是否改变 */
        tcxo_det_value_target = TCXO_32K_DET_VALUE + 2;
    }

    sys_tick_old = (uint32)ssi_read32_etc(HI1103_GLB_CTL_SYS_TICK_VALUE_W_0_REG);
    tcxo_det_res_old = (uint32)ssi_read32_etc(HI1103_GLB_CTL_TCXO_32K_DET_RESULT_REG);

    ssi_write32_etc(HI1103_GLB_CTL_SYS_TICK_CFG_W_REG, 0x2); /* 清零w systick */
    oal_get_time_cost_start(cost);

    if (tcxo_enable) {
        ssi_write32_etc(HI1103_GLB_CTL_TCXO_32K_DET_CNT_REG, tcxo_det_value_target); /* 设置计数周期 */
        ssi_write32_etc(HI1103_GLB_CTL_TCXO_DET_CTL_REG, 0x0);                       /* tcxo_det_en disable */

        /* to tcxo */
        ssi_switch_clk(SSI_AON_CLKSEL_TCXO);

        oal_udelay(150);

        /* to ssi */
        ssi_switch_clk(SSI_AON_CLKSEL_SSI);
        ssi_write32_etc(HI1103_GLB_CTL_TCXO_DET_CTL_REG, 0x1); /* tcxo_det_en enable */
        /* to tcxo */
        ssi_switch_clk(SSI_AON_CLKSEL_TCXO);
        oal_udelay(31 * tcxo_det_value_target * 2); /* wait detect done,根据设置的计数周期数等待 */

        /* to ssi */
        ssi_switch_clk(SSI_AON_CLKSEL_SSI);
    } else {
        oal_udelay(300);
    }

    ret = ssi_read_reg_info_arry(hi1103_tcxo_detect_regs, sizeof(hi1103_tcxo_detect_regs) / sizeof(ssi_reg_info *),
                                 ssi_is_logfile);
    if (ret) {
        return ret;
    }

    oal_udelay(1000); /* wait 32k count more */

    oal_get_time_cost_end(cost);
    oal_calc_time_cost_sub(cost);

    sys_tick_new = (uint32)ssi_read32_etc(HI1103_GLB_CTL_SYS_TICK_VALUE_W_0_REG);

    reg = (uint32)ssi_read32_etc(HI1103_GLB_CTL_TCXO_DET_CTL_REG);
    tcxo_det_res_new = (uint32)ssi_read32_etc(HI1103_GLB_CTL_TCXO_32K_DET_RESULT_REG);

    /* 32k detect */
    if (sys_tick_new == sys_tick_old) {
        PS_PRINT_ERR("32k sys_tick don't change after detect, 32k maybe abnormal, sys_tick=0x%x\n", sys_tick_new);
    } else {
        oal_uint64 us_to_s;
        us_to_s = time_cost_var_sub(cost);
        us_to_s += 1446; /* 经验值,误差1446us */
        clock_32k = (sys_tick_new * 1000) / (oal_uint32)us_to_s;
        PS_PRINT_ERR("32k runtime:%llu us , sys_tick:%u\n", us_to_s, sys_tick_new);
        PS_PRINT_ERR("32k realclock real= %u Khz[base=32768]\n", clock_32k);
    }

    /* tcxo enabled */
    if (tcxo_enable) {
        ssi_detect_tcxo_handle(tcxo_det_res_old, tcxo_det_res_new,
                               base_tcxo_clock, tcxo_det_value_target, clock_32k);
    }

    return ret;
}

int32 ssi_tcxo_mux(uint32 flag)
{
    int ret;

    if ((board_info_etc.ssi_gpio_clk == 0) || (board_info_etc.ssi_gpio_data == 0)) {
        PS_PRINT_ERR("reset aon, gpio ssi don't support\n");
        return -1;
    }

    ret = ssi_request_gpio_etc(board_info_etc.ssi_gpio_clk, board_info_etc.ssi_gpio_data);
    if (ret) {
        PS_PRINT_ERR("ssi_force_reset_aon request failed:%d, data:%d, ret=%d\n",
                     board_info_etc.ssi_gpio_clk, board_info_etc.ssi_gpio_data, ret);
        return ret;
    }

    PS_PRINT_INFO("SSI start set\n");

    ssi_switch_clk(SSI_AON_CLKSEL_SSI);

    if (flag == 1) {
        ssi_write16_etc(GPIO_SSI_REG(SSI_SSI_CTRL), 0x0);
        ssi_write16_etc(GPIO_SSI_REG(SSI_SEL_CTRL), 0x60);
        ssi_write16_etc(GPIO_SSI_REG(SSI_SSI_CTRL), 0x60);
        ssi_write32_etc(HI1103_PMU2_CMU_IR_CMU_RESERVE1_REG, 0x100);
        PS_PRINT_INFO("SSI set 0x50003338 to 0x100\n");
    } else {
        ssi_write16_etc(GPIO_SSI_REG(SSI_SEL_CTRL), 0x0);
    }

    ssi_switch_clk(SSI_AON_CLKSEL_TCXO);

    PS_PRINT_INFO("SSI set OK\n");

    ssi_free_gpio_etc();

    return 0;
}

static void dsm_cpu_info_dump(void)
{
    int32 i;
    int32 ret;
    int32 count = 0;
    char buf[DSM_CPU_INFO_SIZE];
    /* dsm cpu信息上报 */
    if (halt_det_cnt || (hi11xx_kernel_crash)) {
        PS_PRINT_INFO("halt_det_cnt=%u hi11xx_kernel_crash=%d dsm_cpu_info_dump return\n",
                      halt_det_cnt, hi11xx_kernel_crash);
        return;
    }

    /* 没有检测到异常，上报记录的CPU信息 */
    memset_s((void *)buf, sizeof(buf), 0, sizeof(buf));
    ret = snprintf_s(buf + count, sizeof(buf) - count, sizeof(buf) - count - 1,
                     "wcpu_state=0x%x %s, bcpu_state=0x%x %s ",
                     st_ssi_cpu_infos.wcpu_info.cpu_state,
                     (ssi_hi1103_pilot_cpu_st_str[st_ssi_cpu_infos.wcpu_info.cpu_state & 0x7]),
                     st_ssi_cpu_infos.bcpu_info.cpu_state,
                     (ssi_hi1103_pilot_cpu_st_str[st_ssi_cpu_infos.bcpu_info.cpu_state & 0x7]));
    if (ret < 0) {
        goto done;
    }
    count += ret;

    for (i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++) {
        if (st_ssi_cpu_infos.wcpu_info.reg_flag[i] == 0) {
            continue;
        }
        ret = snprintf_s(buf + count, sizeof(buf) - count, sizeof(buf) - count - 1,
                         "wcpu[%d] pc:0x%x lr:0x%x sp:0x%x ", i, st_ssi_cpu_infos.wcpu_info.pc[i],
                         st_ssi_cpu_infos.wcpu_info.lr[i], st_ssi_cpu_infos.wcpu_info.sp[i]);
        if (ret < 0) {
            goto done;
        }
        count += ret;
    }

done:
#ifdef CONFIG_HUAWEI_DSM
    hw_1103_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_1103_HALT, "%s\n", buf);
#else
    OAL_IO_PRINT("log str format err [non-dsm]%s\n", buf);
#endif
}

static void ssi_check_buck_scp_ocp_status(void)
{
    uint32 reg;

    /* buck ocp/acp */
    reg = (uint32)ssi_read32_etc(HI1103_PMU_CMU_CTL_PMU_PROTECT_STATUS_REG);
    if ((reg & (0xFFFFFFFC)) != 0) {
        /* bit 0,1 */
        PS_PRINT_INFO("buck protect status:0x%x invalid", reg);
        return;
    }

    PS_PRINT_INFO("buck protect status:0x%x %s %s \n", reg,
                  (reg & 0x1) ? "buck_scp_off" : "", (reg & 0x2) ? "buck_ocp_off" : "");
#ifdef CONFIG_HUAWEI_DSM
    if (reg & 0x3) {
        hw_1103_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_BUCK_PROTECTED, "%s: buck protect status:0x%x %s %s \n",
                                  __FUNCTION__, reg, (reg & 0x1) ? "buck_scp_off" : "",
                                  (reg & 0x2) ? "buck_ocp_off" : "");
        halt_det_cnt++;
    }
#endif
}

static int ssi_check_wcpu_is_working(void)
{
    uint32 reg, mask;

    /* pilot */
    reg = (uint32)ssi_read32_etc(HI1103_PMU_CMU_CTL_SYS_STATUS_0_REG);
    mask = reg & 0x7;
    PS_PRINT_INFO("cpu state=0x%8x, wcpu is %s\n", reg, ssi_hi1103_pilot_cpu_st_str[mask]);
    st_ssi_cpu_infos.wcpu_info.cpu_state = mask;
    if (mask == 0x5) {
        ssi_check_buck_scp_ocp_status();
    }
    return (mask == 0x3);
}

static int ssi_check_bcpu_is_working(void)
{
    uint32 reg, mask;

    /* pilot */
    reg = (uint32)ssi_read32_etc(HI1103_PMU_CMU_CTL_SYS_STATUS_0_REG);
    mask = (reg >> 3) & 0x7;
    PS_PRINT_INFO("cpu state=0x%8x, bcpu is %s\n", reg, ssi_hi1103_pilot_cpu_st_str[mask]);
    st_ssi_cpu_infos.bcpu_info.cpu_state = mask;
    if (mask == 0x5) {
        ssi_check_buck_scp_ocp_status();
    }
    return (mask == 0x3);
}

static int ssi_read_wpcu_pc_lr_sp(int trace_en)
{
    int i;
    uint32 reg_low, reg_high, pc, lr, sp;

    /* read pc twice check whether wcpu is runing */
    for (i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++) {
        ssi_write32_etc(HI1103_GLB_CTL_WCPU_LOAD_REG, 0x1);
        oal_mdelay(1);

        reg_low = (uint32)ssi_read32_etc(HI1103_GLB_CTL_WCPU_PC_L_REG);
        reg_high = (uint32)ssi_read32_etc(HI1103_GLB_CTL_WCPU_PC_H_REG);
        pc = reg_low | (reg_high << 16);

        reg_low = (uint32)ssi_read32_etc(HI1103_GLB_CTL_WCPU_LR_L_REG);
        reg_high = (uint32)ssi_read32_etc(HI1103_GLB_CTL_WCPU_LR_H_REG);
        lr = reg_low | (reg_high << 16);

        reg_low = (uint32)ssi_read32_etc(HI1103_GLB_CTL_WCPU_SP_L_REG);
        reg_high = (uint32)ssi_read32_etc(HI1103_GLB_CTL_WCPU_SP_H_REG);
        sp = reg_low | (reg_high << 16);

        PS_PRINT_INFO("gpio-ssi:read wcpu[%i], pc:0x%x, lr:0x%x, sp:0x%x \n", i, pc, lr, sp);
        if (!pc && !lr && !sp) {
            PS_PRINT_INFO("wcpu pc lr sp all zero\n");
            if (trace_en) {
                if (ssi_check_wcpu_is_working()) {
                    PS_PRINT_INFO("wcpu try to enable trace en\n");
                    ssi_write32_etc(HI1103_WCPU_PATCH_WCPU_CFG_TRACE_EN_REG, 0x1);
                    oal_mdelay(1);
                }
                trace_en = 0;
                i = -1;
            }
        } else {
            if (st_ssi_cpu_infos.wcpu_info.reg_flag[i] == 0) {
                st_ssi_cpu_infos.wcpu_info.reg_flag[i] = 1;
                st_ssi_cpu_infos.wcpu_info.pc[i] = pc;
                st_ssi_cpu_infos.wcpu_info.lr[i] = lr;
                st_ssi_cpu_infos.wcpu_info.sp[i] = sp;
            }
        }
        oal_mdelay(10);
    }

    return 0;
}

static int ssi_read_bpcu_pc_lr_sp(int trace_en)
{
    int i;
    uint32 reg_low, reg_high, pc, lr, sp;

    /* read pc twice check whether wcpu is runing */
    for (i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++) {
        ssi_write32_etc(HI1103_GLB_CTL_BCPU_LOAD_REG, 0x1);
        oal_mdelay(1);

        reg_low = (uint32)ssi_read32_etc(HI1103_GLB_CTL_BCPU_PC_L_REG);
        reg_high = (uint32)ssi_read32_etc(HI1103_GLB_CTL_BCPU_PC_H_REG);
        pc = reg_low | (reg_high << 16);

        reg_low = (uint32)ssi_read32_etc(HI1103_GLB_CTL_BCPU_LR_L_REG);
        reg_high = (uint32)ssi_read32_etc(HI1103_GLB_CTL_BCPU_LR_H_REG);
        lr = reg_low | (reg_high << 16);

        reg_low = (uint32)ssi_read32_etc(HI1103_GLB_CTL_BCPU_SP_L_REG);
        reg_high = (uint32)ssi_read32_etc(HI1103_GLB_CTL_BCPU_SP_H_REG);
        sp = reg_low | (reg_high << 16);

        PS_PRINT_INFO("gpio-ssi:read bcpu[%i], pc:0x%x, lr:0x%x, sp:0x%x \n", i, pc, lr, sp);
        if (!pc && !lr && !sp) {
            PS_PRINT_INFO("bcpu pc lr sp all zero\n");
            if (trace_en) {
                if (ssi_check_bcpu_is_working()) {
                    PS_PRINT_INFO("bcpu try to enable trace en\n");
                    ssi_write32_etc(HI1103_BCPU_PATCH_BCPU_CFG_TRACE_EN_REG, 0x1);
                    oal_mdelay(1);
                }
                trace_en = 0;
                i = -1;
            }
        } else {
            if (st_ssi_cpu_infos.bcpu_info.reg_flag[i] == 0) {
                st_ssi_cpu_infos.bcpu_info.reg_flag[i] = 1;
                st_ssi_cpu_infos.bcpu_info.pc[i] = pc;
                st_ssi_cpu_infos.bcpu_info.lr[i] = lr;
                st_ssi_cpu_infos.bcpu_info.sp[i] = sp;
            }
        }
        oal_mdelay(10);
    }

    return 0;
}

int ssi_read_device_arm_register(int trace_en)
{
    int32 ret;

    uint32 reg = (uint32)ssi_read32_etc(HI1103_PMU2_CMU_IR_SYSLDO_WL_C0_ABB_RF_PWR_EN_STS_REG);

    if (reg == 0x3) {
        PS_PRINT_ERR("0x50003a88 is 0x3, wifi chip maybe enter dft mode , please check!\n");
    }

    ret = ssi_check_wcpu_is_working();
    if (ret < 0) {
        return ret;
    }
    if (ret) {
        ssi_read_wpcu_pc_lr_sp(trace_en);
    }
    bfgx_print_subsys_state();
    ret = ssi_check_bcpu_is_working();
    if (ret < 0) {
        return ret;
    }
    if (ret) {
        ssi_read_bpcu_pc_lr_sp(trace_en);
    }

    return 0;
}

static int ssi_dump_device_aon_regs(unsigned long long module_set)
{
    int ret = OAL_SUCC;
    if (module_set & SSI_MODULE_MASK_AON) {
        ret = ssi_read_reg_info_arry(hi1103_aon_reg_full, sizeof(hi1103_aon_reg_full) / sizeof(ssi_reg_info *),
                                     ssi_is_logfile);
        if (ret) {
            return -OAL_EFAIL;
        }
    }

    if (module_set & SSI_MODULE_MASK_AON_CUT) {
        ret = ssi_read_reg_info_arry(hi1103_aon_reg_cut, sizeof(hi1103_aon_reg_cut) / sizeof(ssi_reg_info *),
                                     ssi_is_logfile);
        if (ret) {
            return -OAL_EFAIL;
        }
    }

    if (module_set & SSI_MODULE_MASK_COEX_CTL) {
        ret = ssi_read_reg_info_arry(hi1103_coex_ctl_full, sizeof(hi1103_coex_ctl_full) / sizeof(ssi_reg_info *),
                                     ssi_is_logfile);
        if (ret) {
            return -OAL_EFAIL;
        }
    }

    return OAL_SUCC;
}

static int ssi_dump_device_wcpu_key_mem(unsigned long long module_set)
{
    int ret;

    if (module_set & SSI_MODULE_MASK_WCPU_KEY_DTCM) {
        if (ssi_check_wcpu_is_working()) {
            ret = ssi_read_reg_info(&hi1103_w_key_mem, NULL, 0, ssi_is_logfile);
            if (ret) {
                PS_PRINT_INFO("wcpu key mem read failed, continue try aon\n");
            }
        } else {
            PS_PRINT_INFO("wctrl can't dump, wcpu down\n");
        }
    }

    return 0;
}

static int ssi_dump_device_wctrl_regs(unsigned long long module_set)
{
    int ret;

    if (module_set & SSI_MODULE_MASK_WCTRL) {
        if (ssi_check_wcpu_is_working()) {
            ret = ssi_read_reg_info(&hi1103_w_ctrl_full, NULL, 0, ssi_is_logfile);
            if (ret) {
                goto ssi_fail;
            }

            ret = ssi_read_reg_info(&hi1103_rf_w_c0_full, NULL, 0, ssi_is_logfile);
            if (ret) {
                goto ssi_fail;
            }

            ret = ssi_read_reg_info(&hi1103_rf_w_c1_full, NULL, 0, ssi_is_logfile);
            if (ret) {
                goto ssi_fail;
            }

        } else {
            PS_PRINT_INFO("wctrl can't dump, wcpu down\n");
        }
    }

    return 0;
ssi_fail:
    return ret;
}

static int ssi_dump_device_pcie_regs(unsigned long long module_set)
{
    int ret;

    if (module_set & SSI_MODULE_MASK_PCIE_CFG) {
        if (ssi_check_wcpu_is_working()) {
            ret = ssi_read_reg_info_arry(hi1103_pcie_cfg_reg_full,
                                         sizeof(hi1103_pcie_cfg_reg_full) / sizeof(ssi_reg_info *),
                                         ssi_is_logfile);
            if (ret) {
                goto ssi_fail;
            }
        } else {
            PS_PRINT_INFO("pcie cfg can't dump, wcpu down\n");
        }
    }

    if (module_set & SSI_MODULE_MASK_PCIE_CUT) {
        if (ssi_check_wcpu_is_working()) {
            ret = ssi_read_reg_info_arry(hi1103_pcie_cfg_reg_cut,
                                         sizeof(hi1103_pcie_cfg_reg_cut) / sizeof(ssi_reg_info *),
                                         ssi_is_logfile);
            if (ret) {
                goto ssi_fail;
            }
        } else {
            PS_PRINT_INFO("pcie cfg cut can't dump, wcpu down\n");
        }
    }

    if (module_set & SSI_MODULE_MASK_PCIE_DBI) {
        if (ssi_check_wcpu_is_working()) {
            oal_uint32 reg_nums;
            ssi_reg_info **pst_pcie_dbi_reg;
            reg_nums = sizeof(hi1103_pcie_dbi_pilot_reg_full) / sizeof(ssi_reg_info *);
            pst_pcie_dbi_reg = hi1103_pcie_dbi_pilot_reg_full;

            ret = ssi_read_reg_info_arry(pst_pcie_dbi_reg, reg_nums, ssi_is_logfile);
            if (ret) {
                goto ssi_fail;
            }
        } else {
            PS_PRINT_INFO("pcie dbi can't dump, wcpu down\n");
        }
    }

    return 0;
ssi_fail:
    return ret;
}

static int ssi_dump_device_sdio_regs(unsigned long long module_set)
{
    int ret;

    if (module_set & SSI_MODULE_MASK_SDIO) {
        if (ssi_check_wcpu_is_working()) {
            ret = ssi_read_reg_info(&hi1103_pcie_sdio_ctrl_full, NULL, 0, ssi_is_logfile);
            if (ret) {
                goto ssi_fail;
            }
        } else {
            PS_PRINT_INFO("sdio can't dump, wcpu down\n");
        }
    }

    return 0;
ssi_fail:
    return ret;
}

static int ssi_dump_device_bctl_and_exception_regs(unsigned long long module_set)
{
    int ret;

    if (module_set & SSI_MODULE_MASK_BCTRL) {
        if (ssi_check_bcpu_is_working()) {
            ret = ssi_read_reg_info(&hi1103_b_ctrl_full, NULL, 0, ssi_is_logfile);
            if (ret) {
                goto ssi_fail;
            }

            ret = ssi_read_reg_info(&hi1103_rf_bfg_full, NULL, 0, ssi_is_logfile);
            if (ret) {
                goto ssi_fail;
            }
        }
    }

    if (module_set & SSI_MODULE_MASK_BCPU_EXCEPT_MEM) {
        if (ssi_check_bcpu_is_working()) {
            ret = ssi_read_reg_info(&hi1103_b_except_mem, NULL, 0, ssi_is_logfile);
            if (ret) {
                PS_PRINT_INFO("bcpu key mem read failed\n");
                goto ssi_fail;
            }
        }
    }

    return 0;
ssi_fail:
    return ret;
}

static int ssi_dump_device_arm_regs(unsigned long long module_set, int trace_en)
{
    int ret;

    if (module_set & SSI_MODULE_MASK_ARM_REG) {
        ret = ssi_read_device_arm_register(trace_en);
        if (ret) {
            goto ssi_fail;
        }
    }

    return 0;
ssi_fail:
    return ret;
}

static int ssi_dump_device_tcxo_regs(unsigned long long module_set)
{
    int ret;

    if (module_set & (SSI_MODULE_MASK_AON | SSI_MODULE_MASK_AON_CUT)) {
        ret = ssi_detect_tcxo_is_normal();
        if (ret) {
            PS_PRINT_INFO("tcxo detect failed, continue dump\n");
        }
    }

    return 0;
}

int ssi_device_regs_dump(unsigned long long module_set)
{
    int ret;

    halt_det_cnt = 0;
    memset_s(&st_ssi_cpu_infos, sizeof(st_ssi_cpu_infos), 0, sizeof(st_ssi_cpu_infos));

    ssi_read16_etc(GPIO_SSI_REG(SSI_SSI_CTRL));
    ssi_read16_etc(GPIO_SSI_REG(SSI_SEL_CTRL));

    ssi_switch_clk(SSI_AON_CLKSEL_SSI);

    ret = ssi_check_device_isalive();
    if (ret) {
        /* try to reset aon */
        ssi_force_reset_reg();
        ssi_switch_clk(SSI_AON_CLKSEL_SSI);
        if (ssi_check_device_isalive()) {
            PS_PRINT_INFO("after reset aon, ssi still can't work\n");
            goto ssi_fail;
        } else {
            PS_PRINT_INFO("after reset aon, ssi ok, dump acp/ocp reg\n");
            ssi_check_buck_scp_ocp_status();
            module_set = SSI_MODULE_MASK_COMM;
        }
    } else {
        ssi_check_buck_scp_ocp_status();
    }

    ret = ssi_dump_device_arm_regs(module_set, 0);
    if (ret) {
        goto ssi_fail;
    }

    ret = ssi_check_device_isalive();
    if (ret) {
        goto ssi_fail;
    }

    ret = ssi_dump_device_aon_regs(module_set);
    if (ret != OAL_SUCC) {
        goto ssi_fail;
    }

    ret = ssi_dump_device_arm_regs(module_set, 1);
    if (ret) {
        goto ssi_fail;
    }

    /* detect tcxo clock is normal, trigger */
    ssi_dump_device_tcxo_regs(module_set);

    ret = ssi_dump_device_wcpu_key_mem(module_set);
    if (ret) {
        goto ssi_fail;
    }

    ret = ssi_dump_device_wctrl_regs(module_set);
    if (ret) {
        goto ssi_fail;
    }

    ret = ssi_dump_device_pcie_regs(module_set);
    if (ret) {
        goto ssi_fail;
    }

    ret = ssi_dump_device_sdio_regs(module_set);
    if (ret) {
        goto ssi_fail;
    }

    ret = ssi_dump_device_bctl_and_exception_regs(module_set);
    if (ret) {
        goto ssi_fail;
    }

    ssi_switch_clk(SSI_AON_CLKSEL_TCXO);
    dsm_cpu_info_dump();

    return 0;

ssi_fail:
    ssi_switch_clk(SSI_AON_CLKSEL_TCXO);
    dsm_cpu_info_dump();
    return ret;
}

/*
 * SSI寄存器打印，根据发生错误的业务场景，
 * 区分SSI打印的寄存器大小和打印频率，尽量减少对业务的影响
 * 区分Android user/eng ，区分user beta 和 user release版本
 * 针对user release应严格限制SSI DUMP操作
 **/
OAL_STATIC unsigned long long ssi_get_module_comm(ssi_err_type etype)
{
    OAL_REFERENCE(etype);
    return SSI_MODULE_MASK_COMM;
}

OAL_STATIC int32 ssi_private_comm(ssi_err_type etype)
{
    OAL_REFERENCE(etype);
    return 0;
}

OAL_STATIC int ssi_get_err_dump_level(void)
{
    int auto_switch = ssi_clk_auto_switch_is_support();
    hi110x_release_vtype vtype = hi110x_get_release_type();
    if (vtype == HI110X_VTYPE_RELEASE) {
        return SSI_ERR_LOG_LEVEL_CLOSE;
    } else if (vtype == HI110X_VTYPE_RELEASE_DEBUG) {
        if (auto_switch == OAL_TRUE) {
            return SSI_ERR_LOG_LEVEL_FULL;
        } else {
            return SSI_ERR_LOG_LEVEL_CUT;
        }
    } else if (vtype == HI110X_VTYPE_DEBUG) {
        return SSI_ERR_LOG_LEVEL_FULL;
    } else {
        return SSI_ERR_LOG_LEVEL_CLOSE;
    }
}

/* SSI_ERR_HCC_EXCP_SDIO */
OAL_STATIC unsigned long long ssi_get_module_hcc_excp_sdio(ssi_err_type etype)
{
    unsigned long long flag = SSI_MODULE_NON_FLAG;

    int err_level = ssi_get_err_dump_level();
    OAL_REFERENCE(etype);
    if (err_level == SSI_ERR_LOG_LEVEL_CLOSE) {
        if (oal_print_rate_limit(24 * PRINT_RATE_HOUR)) { /* 24小时打印一次 */
            flag = (bfgx_is_shutdown_etc() == true) ? (SSI_MODULE_MASK_ARM_REG | SSI_MODULE_MASK_AON_CUT) : (0);
        } else {
            flag = SSI_MODULE_NON_FLAG;
        }
    } else if (err_level == SSI_ERR_LOG_LEVEL_CUT) {
        flag = (bfgx_is_shutdown_etc() == true) ? (SSI_MODULE_MASK_COMM) : (SSI_MODULE_MASK_ARM_REG);
    } else if (err_level == SSI_ERR_LOG_LEVEL_FULL) {
        flag = SSI_MODULE_MASK_COMM;
    } else {
        flag = SSI_MODULE_NON_FLAG;
    }

    return flag;
}

/* SSI_ERR_HCC_EXCP_PCIE */
OAL_STATIC unsigned long long ssi_get_module_hcc_excp_pcie(ssi_err_type etype)
{
    int err_level;
    unsigned long long cut_flag, full_flag;
    unsigned long long flag = SSI_MODULE_NON_FLAG;
    OAL_REFERENCE(etype);

    cut_flag = (SSI_MODULE_MASK_ARM_REG | SSI_MODULE_MASK_AON_CUT | SSI_MODULE_MASK_PCIE_CUT);
    full_flag = (SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG | SSI_MODULE_MASK_PCIE_DBI);

    err_level = ssi_get_err_dump_level();
    if (err_level == SSI_ERR_LOG_LEVEL_CLOSE) {
        if (oal_print_rate_limit(24 * PRINT_RATE_HOUR)) { /* 24小时打印一次 */
            flag = (bfgx_is_shutdown_etc() == true) ? cut_flag : 0;
        } else {
            flag = SSI_MODULE_NON_FLAG;
        }
    } else if (err_level == SSI_ERR_LOG_LEVEL_CUT) {
        flag = (bfgx_is_shutdown_etc() == true) ? cut_flag : (SSI_MODULE_MASK_ARM_REG);
    } else if (err_level == SSI_ERR_LOG_LEVEL_FULL) {
        flag = full_flag;
    } else {
        flag = SSI_MODULE_NON_FLAG;
    }

    return flag;
}

OAL_STATIC unsigned long long ssi_get_module_kirin_noc(ssi_err_type etype)
{
    int err_level;
    unsigned long long cut_flag, full_flag;
    unsigned long long flag = SSI_MODULE_NON_FLAG;

    OAL_REFERENCE(etype);

    cut_flag = (SSI_MODULE_MASK_ARM_REG);
    full_flag = (SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG);

    err_level = ssi_get_err_dump_level();
    if (err_level == SSI_ERR_LOG_LEVEL_CLOSE) {
        flag = cut_flag;
    } else {
        flag = full_flag;
    }

    return flag;
}

OAL_STATIC unsigned long long ssi_get_module_pcie_check_linkfail(ssi_err_type etype)
{
    int err_level;
    unsigned long long cut_flag, full_flag;
    unsigned long long flag = SSI_MODULE_NON_FLAG;

    OAL_REFERENCE(etype);

    cut_flag = SSI_MODULE_NON_FLAG;
    full_flag = (SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG);

    err_level = ssi_get_err_dump_level();
    if (err_level == SSI_ERR_LOG_LEVEL_CLOSE) {
        flag = cut_flag;
    } else if (err_level == SSI_ERR_LOG_LEVEL_CUT) {
        flag = (bfgx_is_shutdown_etc() == true) ? full_flag : SSI_MODULE_MASK_ARM_REG;
    } else {
        flag = full_flag;
    }

    return flag;
}

OAL_STATIC unsigned long long ssi_get_module_pcie_wait_boot_fail(ssi_err_type etype)
{
    int err_level;
    unsigned long long cut_flag, full_flag;
    unsigned long long flag = SSI_MODULE_NON_FLAG;

    OAL_REFERENCE(etype);

    cut_flag = (SSI_MODULE_MASK_ARM_REG);
    full_flag = (SSI_MODULE_MASK_COMM);

    err_level = ssi_get_err_dump_level();
    if (err_level == SSI_ERR_LOG_LEVEL_CLOSE) {
        if (oal_print_rate_limit(1 * PRINT_RATE_HOUR)) { /* 1小时打印一次 */
            flag = cut_flag;
        } else {
            flag = SSI_MODULE_NON_FLAG;
        }
    } else {
        flag = full_flag;
    }

    return flag;
}

OAL_STATIC unsigned long long ssi_get_module_pcie_sr_wakeup_fail(ssi_err_type etype)
{
    unsigned long long flag = SSI_MODULE_NON_FLAG;
    hi110x_release_vtype vtype;
    OAL_REFERENCE(etype);

    vtype = hi110x_get_release_type();
    if (vtype == HI110X_VTYPE_DEBUG) {
        flag = (SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG);
    }

    return flag;
}

OAL_STATIC unsigned long long ssi_get_module_pcie_sr_wakeup_retry_fail(ssi_err_type etype)
{
    int err_level;
    unsigned long long cut_flag, full_flag;
    unsigned long long flag = SSI_MODULE_NON_FLAG;
    OAL_REFERENCE(etype);

    cut_flag = (SSI_MODULE_MASK_ARM_REG | SSI_MODULE_MASK_AON_CUT | SSI_MODULE_MASK_PCIE_CUT);
    full_flag = (SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG);

    err_level = ssi_get_err_dump_level();
    if (err_level == SSI_ERR_LOG_LEVEL_CLOSE) {
        if (oal_print_rate_limit(1 * PRINT_RATE_HOUR)) { /* 1小时打印一次 */
            flag = (bfgx_is_shutdown_etc() == true) ? cut_flag : 0;
        } else {
            flag = SSI_MODULE_MASK_ARM_REG;
        }
    } else if (err_level == SSI_ERR_LOG_LEVEL_CUT) {
        flag = (bfgx_is_shutdown_etc() == true) ? cut_flag : (SSI_MODULE_MASK_ARM_REG);
    } else if (err_level == SSI_ERR_LOG_LEVEL_FULL) {
        flag = full_flag;
    } else {
        flag = SSI_MODULE_NON_FLAG;
    }

    return flag;
}

OAL_STATIC unsigned long long ssi_get_module_pcie_pwrup_fail(ssi_err_type etype)
{
    int err_level;
    unsigned long long cut_flag, full_flag;
    unsigned long long flag = SSI_MODULE_NON_FLAG;
    OAL_REFERENCE(etype);

    cut_flag = SSI_MODULE_MASK_PCIE_CUT_SET;
    full_flag = SSI_MODULE_MASK_PCIE_FULL;

    err_level = ssi_get_err_dump_level();
    if (err_level == SSI_ERR_LOG_LEVEL_CLOSE) {
        if (oal_print_rate_limit(24 * PRINT_RATE_HOUR)) { /* 24小时打印一次 */
            flag = (bfgx_is_shutdown_etc() == true) ? cut_flag : 0;
            PS_PRINT_ERR("bfgx is shutdown\n");
        } else {
            flag = SSI_MODULE_MASK_ARM_REG;
            PS_PRINT_ERR("bfgx is working\n");
        }
    } else if (err_level == SSI_ERR_LOG_LEVEL_CUT) {
        flag = (bfgx_is_shutdown_etc() == true) ? cut_flag : (SSI_MODULE_MASK_ARM_REG);
    } else if (err_level == SSI_ERR_LOG_LEVEL_FULL) {
        flag = full_flag;
    } else {
        flag = SSI_MODULE_NON_FLAG;
    }

    return flag;
}

/* PCIE 发生一次的错误，比如枚举错误和第一次WiFi关机错误 */
OAL_STATIC unsigned long long ssi_get_module_pcie_proc_once_fail(ssi_err_type etype)
{
    int err_level;
    unsigned long long cut_flag, full_flag;
    unsigned long long flag = SSI_MODULE_NON_FLAG;

    OAL_REFERENCE(etype);

    cut_flag = (SSI_MODULE_MASK_PCIE_CUT_SET);
    full_flag = (SSI_MODULE_MASK_PCIE_FULL);

    err_level = ssi_get_err_dump_level();
    if (err_level == SSI_ERR_LOG_LEVEL_CLOSE) {
        if (oal_print_rate_limit(1 * PRINT_RATE_MINUTE)) { /* 1分钟打印一次 */
            flag = cut_flag;
        } else {
            flag = SSI_MODULE_NON_FLAG;
        }
    } else {
        flag = full_flag;
    }

    return flag;
}

OAL_STATIC unsigned long long ssi_get_module_sdio_reenum_fail(ssi_err_type etype)
{
    int err_level;
    unsigned long long cut_flag, full_flag;
    unsigned long long flag = SSI_MODULE_NON_FLAG;

    OAL_REFERENCE(etype);

    cut_flag = (SSI_MODULE_MASK_ARM_REG | SSI_MODULE_MASK_AON_CUT | SSI_MODULE_MASK_SDIO);
    full_flag = (SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_SDIO);

    err_level = ssi_get_err_dump_level();
    if (err_level == SSI_ERR_LOG_LEVEL_CLOSE) {
        if (oal_print_rate_limit(1 * PRINT_RATE_HOUR)) { /* 1小时打印一次 */
            flag = (bfgx_is_shutdown_etc() == true) ? SSI_MODULE_MASK_ARM_REG : 0;
        } else {
            flag = SSI_MODULE_NON_FLAG;
        }
    } else if (err_level == SSI_ERR_LOG_LEVEL_CUT) {
        flag = (bfgx_is_shutdown_etc() == true) ? cut_flag : (SSI_MODULE_MASK_ARM_REG);
    } else if (err_level == SSI_ERR_LOG_LEVEL_FULL) {
        flag = full_flag;
    } else {
        flag = SSI_MODULE_NON_FLAG;
    }

    return flag;
}

OAL_STATIC unsigned long long ssi_get_module_sdio_probeinit_fail(ssi_err_type etype)
{
    int err_level;
    unsigned long long flag = SSI_MODULE_NON_FLAG;

    OAL_REFERENCE(etype);

    err_level = ssi_get_err_dump_level();
    if (err_level == SSI_ERR_LOG_LEVEL_CLOSE) {
        flag = SSI_MODULE_NON_FLAG;
    } else {
        flag = SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_SDIO;
    }

    return flag;
}

OAL_STATIC unsigned long long ssi_get_module_bfg_wkup_fail(ssi_err_type etype)
{
    int err_level;
    unsigned long long full_flag;
    unsigned long long flag = SSI_MODULE_NON_FLAG;

    OAL_REFERENCE(etype);

    /* bfg异常打印SSI 有可能导致PCIE异常，
       甚至PCIE NOC，所以只在root和beta 版本打印 */
    full_flag = (SSI_MODULE_MASK_ARM_REG | SSI_MODULE_MASK_AON | SSI_MODULE_MASK_COEX_CTL | SSI_MODULE_MASK_BCTRL);

    err_level = ssi_get_err_dump_level();
    if (err_level == SSI_ERR_LOG_LEVEL_CLOSE) {
        if (oal_print_rate_limit(1 * PRINT_RATE_HOUR)) { /* 1小时打印一次 */
            flag = ((wlan_is_shutdown_etc() == true) && (ssi_dump_en != 0)) ? SSI_MODULE_MASK_ARM_REG : 0;
        } else {
            flag = SSI_MODULE_NON_FLAG;
        }
    } else if (err_level == SSI_ERR_LOG_LEVEL_CUT) {
        flag = ((wlan_is_shutdown_etc() == true) && (ssi_dump_en != 0)) ? full_flag : 0;
    } else if (err_level == SSI_ERR_LOG_LEVEL_FULL) {
        flag = full_flag;
    } else {
        flag = SSI_MODULE_NON_FLAG;
    }

    return flag;
}

OAL_STATIC unsigned long long ssi_proc_fw_down_fail(ssi_err_type etype, int32 is_sdio)
{
    hi110x_release_vtype vtype;
    unsigned long long flag = SSI_MODULE_NON_FLAG;

    OAL_REFERENCE(etype);

    vtype = hi110x_get_release_type();
    if (vtype == HI110X_VTYPE_DEBUG) {
        flag = SSI_MODULE_MASK_COMM;
        if (is_sdio == OAL_TRUE) {
            flag |= SSI_MODULE_MASK_SDIO;
        }
    }

    return flag;
}

OAL_STATIC unsigned long long ssi_get_module_fw_down_fail(ssi_err_type etype)
{
    return ssi_proc_fw_down_fail(etype, 0);
}

OAL_STATIC unsigned long long ssi_get_module_fw_down_sdio_fail(ssi_err_type etype)
{
    return ssi_proc_fw_down_fail(etype, 1);
}

OAL_STATIC unsigned long long ssi_get_module_pcie_gpio_wkup_fail(ssi_err_type etype)
{
    int err_level;
    unsigned long long flag = SSI_MODULE_NON_FLAG;

    OAL_REFERENCE(etype);

    err_level = ssi_get_err_dump_level();
    if (err_level == SSI_ERR_LOG_LEVEL_CLOSE) {
        PS_PRINT_ERR("dump when dfr\n");
    } else if (err_level == SSI_ERR_LOG_LEVEL_CUT) {
        flag = (bfgx_is_shutdown_etc() == true) ? (SSI_MODULE_MASK_COMM) : (SSI_MODULE_MASK_ARM_REG);
    } else if (err_level == SSI_ERR_LOG_LEVEL_FULL) {
        flag = SSI_MODULE_MASK_COMM;
    } else {
        flag = SSI_MODULE_NON_FLAG;
    }

    return flag;
}

OAL_STATIC unsigned long long ssi_get_module_sdio_wkup_fail(ssi_err_type etype)
{
    hi110x_release_vtype vtype;
    unsigned long long flag = SSI_MODULE_NON_FLAG;

    OAL_REFERENCE(etype);

    vtype = hi110x_get_release_type();
    if (vtype == HI110X_VTYPE_DEBUG) {
        if (oal_print_rate_limit(30 * PRINT_RATE_SECOND)) { /* 30s打印一次 */
            flag = SSI_MODULE_MASK_COMM;
        }
    }

    if (flag == 0) {
        PS_PRINT_ERR("dump when dfr\n");
    }

    return flag;
}

OAL_STATIC unsigned long long ssi_get_module_wlan_pwr_off_fail(ssi_err_type etype)
{
    int err_level;
    unsigned long long flag = SSI_MODULE_NON_FLAG;

    OAL_REFERENCE(etype);

    err_level = ssi_get_err_dump_level();
    if (err_level == SSI_ERR_LOG_LEVEL_CLOSE) {
        if (oal_print_rate_limit(24 * PRINT_RATE_HOUR)) { /* 24小时打印一次 */
            flag = (bfgx_is_shutdown_etc() == true) ? SSI_MODULE_MASK_ARM_REG : 0;
        } else {
            flag = SSI_MODULE_NON_FLAG;
        }
    } else if (err_level == SSI_ERR_LOG_LEVEL_CUT) {
        if (oal_print_rate_limit(1 * PRINT_RATE_HOUR)) { /* 1小时打印一次 */
            flag = (bfgx_is_shutdown_etc() == true) ? (SSI_MODULE_MASK_COMM) : (SSI_MODULE_MASK_ARM_REG);
        } else {
            flag = (bfgx_is_shutdown_etc() == true) ? (SSI_MODULE_MASK_ARM_REG) : (0);
        }
    } else if (err_level == SSI_ERR_LOG_LEVEL_FULL) {
        if (oal_print_rate_limit(1 * PRINT_RATE_HOUR)) { /* 1小时打印一次 */
            flag = SSI_MODULE_MASK_COMM;
        } else {
            flag = SSI_MODULE_MASK_ARM_REG;
        }
    } else {
        flag = SSI_MODULE_NON_FLAG;
    }

    return flag;
}

OAL_STATIC unsigned long long ssi_get_module_bfgx_dump_fail(ssi_err_type etype, unsigned long long flag_t)
{
    unsigned long long flag = SSI_MODULE_NON_FLAG;

    int auto_switch = ssi_clk_auto_switch_is_support();
    hi110x_release_vtype vtype = hi110x_get_release_type();

    OAL_REFERENCE(etype);

    if (auto_switch == OAL_TRUE) {
        if (oal_print_rate_limit(30 * PRINT_RATE_SECOND)) {
            if (vtype != HI110X_VTYPE_RELEASE) {
                PS_PRINT_INFO("bfgx exception ssi dump starts\n");
                flag = flag_t;
            }
        }
    } else {
        PS_PRINT_WARNING("subchip type not support bfgx exception ssi dump\n");
    }
    return flag;
}

OAL_STATIC unsigned long long ssi_get_module_bfgx_open_fail(ssi_err_type etype)
{
    return ssi_get_module_bfgx_dump_fail(etype, (SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_BCTRL));
}

OAL_STATIC unsigned long long ssi_get_module_bfgx_hb_timeout_fail(ssi_err_type etype)
{
    return ssi_get_module_bfgx_dump_fail(etype, (SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_BCTRL | SSI_MODULE_MASK_WCTRL));
}

OAL_STATIC ssi_err_stru g_ssi_err_infos[] = {
    DECLARE_SSI_EINFO(SSI_ERR_COMM, ssi_get_module_comm, ssi_private_comm), /* 1st fixed */
    DECLARE_SSI_EINFO(SSI_ERR_HCC_EXCP_SDIO, ssi_get_module_hcc_excp_sdio, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_HCC_EXCP_PCIE, ssi_get_module_hcc_excp_pcie, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_PCIE_KIRIN_NOC, ssi_get_module_kirin_noc, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_PCIE_CHECK_LINK_FAIL, ssi_get_module_pcie_check_linkfail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_PCIE_WAIT_BOOT_TIMEOUT, ssi_get_module_pcie_wait_boot_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_PCIE_SR_WAKEUP_FAIL, ssi_get_module_pcie_sr_wakeup_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_PCIE_SR_WAKEUP_RETRY_FAIL, ssi_get_module_pcie_sr_wakeup_retry_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_PCIE_POWER_UP_FAIL, ssi_get_module_pcie_pwrup_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_PCIE_FST_POWER_OFF_FAIL, ssi_get_module_pcie_proc_once_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_PCIE_ENUM_FAIL, ssi_get_module_pcie_proc_once_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_SDIO_REENUM_FAIL, ssi_get_module_sdio_reenum_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_SDIO_REINIT_FAIL, ssi_get_module_sdio_reenum_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_SDIO_PROBE_INIT_FAIL, ssi_get_module_sdio_probeinit_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_SDIO_PROBE_FAIL, ssi_get_module_sdio_probeinit_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_BFG_WAKE_UP_FAIL, ssi_get_module_bfg_wkup_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_FIRMWARE_DOWN_FAIL, ssi_get_module_fw_down_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_FIRMWARE_DOWN_SDIO_FAIL, ssi_get_module_fw_down_sdio_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_PCIE_GPIO_WAKE_FAIL, ssi_get_module_pcie_gpio_wkup_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_SDIO_CMD_WAKE_FAIL, ssi_get_module_sdio_wkup_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_SDIO_GPIO_WAKE_FAIL, ssi_get_module_sdio_wkup_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_WLAN_POWEROFF_FAIL, ssi_get_module_wlan_pwr_off_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_BFGX_OPEN_FAIL, ssi_get_module_bfgx_open_fail, NULL),
    DECLARE_SSI_EINFO(SSI_ERR_BFGX_HEART_TIMEOUT, ssi_get_module_bfgx_hb_timeout_fail, NULL),
};

OAL_STATIC ssi_err_stru* ssi_find_idx_by_etype(ssi_err_type etype)
{
    int idx;
    int err_size, i;

    err_size = OAL_ARRAY_SIZE(g_ssi_err_infos);

    if (OAL_WARN_ON((uint32)etype >= (uint32)SSI_ERR_BUT)) {
        PS_PRINT_WARNING("unkown ssi etype=%d\n", etype);
        etype = SSI_ERR_COMM;
    }

    for (i = 0; i < err_size; i++) {
        if (g_ssi_err_infos[i].etype == etype) {
            break;
        }
    }

    if (OAL_WARN_ON(i == err_size)) {
        /* can't found */
        PS_PRINT_WARNING("can't found ssi etype=%d\n", etype);
        idx = 0; /* comm err type */
    } else {
        /* found */
        idx = i;
    }

    return &g_ssi_err_infos[idx];
}

int ssi_dump_err_reg(ssi_err_type etype)
{
    int ret;
    unsigned long long module_set = 0;
    ssi_err_stru* pst_err_info;

    pst_err_info = ssi_find_idx_by_etype(etype);
    if (pst_err_info == NULL) {
        return -OAL_ENODEV;
    }

    /* print err info */
    PS_PRINT_ERR("etype=%d , name=%s\n", pst_err_info->etype, pst_err_info->type_name);

    if (pst_err_info->ssi_get_module_flag != NULL) {
        module_set = pst_err_info->ssi_get_module_flag(pst_err_info->etype);
    } else {
        PS_PRINT_ERR("ssi_get_module_flag is null, bypass dump\n");
    }

    PS_PRINT_ERR("ssi_err_module_flag = 0x%llx\n", module_set);

    /* dump callback before dump ssi err regs */
    OAL_WARN_ON(module_set != 0);

    ret = ssi_dump_device_regs(module_set);

    return ret;
}

static int32 ssi_file_test_pre(void)
{
    // waring: fpga version should set 300801c0 1 to let host control ssi
    /* first set ssi clk ctl */
    if (ssi_switch_clk(SSI_AON_CLKSEL_SSI) != BOARD_SUCC) {
        PS_PRINT_ERR("set ssi clk fail\n");
        return BOARD_FAIL;
    }
    // env init
#ifdef BFGX_UART_DOWNLOAD_SUPPORT
    /* set bootloader deadbeaf */
    if (ssi_write32_etc(0x8010010c, 0xbeaf) != BOARD_SUCC) {
        PS_PRINT_ERR("set flag:beaf fail\n");
        return BOARD_FAIL;
    }
    if (ssi_write32_etc(0x8010010e, 0xdead) != BOARD_SUCC) {
        PS_PRINT_ERR("set flag:dead fail\n");
        return BOARD_FAIL;
    }
#else
    /* set wcpu wait */
    if (ssi_write32_etc(HI1103_GLB_CTL_WCPU_WAIT_CTL_REG, 0x1) != BOARD_SUCC) {
        PS_PRINT_ERR("set wcpu wait fail\n");
        return BOARD_FAIL;
    }

    /* reset wcpu */
    if (ssi_write32_etc(HI1103_W_CTL_WTOPCRG_SOFT_RESET_REG, 0xfe5e) != BOARD_SUCC) {
        // 脉冲复位
    }
    /* boot flag */
    if (ssi_write32_etc(HI1103_GLB_CTL_PWR_ON_LABLE_REG, 0xbeaf) != BOARD_SUCC) {
        PS_PRINT_ERR("set boot flag fail\n");
        return BOARD_FAIL;
    }
    /* dereset bcpu */
    if (ssi_write32_etc(HI1103_GLB_CTL_SOFT_RST_BCPU_REG, 1) != BOARD_SUCC) {
        PS_PRINT_ERR("dereset bcpu\n");
        return BOARD_FAIL;
    }
#endif

    return BOARD_SUCC;
}

static int32 ssi_file_test_post(void)
{
    /* let cpu go */
#ifdef BFGX_UART_DOWNLOAD_SUPPORT
    /* reset bcpu */
    if (ssi_write32_etc(HI1103_GLB_CTL_SOFT_RST_BCPU_REG, 0) != BOARD_SUCC) {
        PS_PRINT_ERR("reset bcpu set 0 fail\n");
        return BOARD_FAIL;
    }
    if (ssi_write32_etc(HI1103_GLB_CTL_SOFT_RST_BCPU_REG, 1) != BOARD_SUCC) {
        PS_PRINT_ERR("reset bcpu set 1 fail\n");
        return BOARD_FAIL;
    }
#else
    /* clear b wait */
    if (ssi_write32_etc(HI1103_GLB_CTL_BCPU_WAIT_CTL_REG, 0x0) != BOARD_SUCC) {
        PS_PRINT_ERR("clear b wait\n");
        return BOARD_FAIL;
    }
    /* clear w wait */
    if (ssi_write32_etc(HI1103_GLB_CTL_WCPU_WAIT_CTL_REG, 0x0) != BOARD_SUCC) {
        PS_PRINT_ERR("clear w wait\n");
        return BOARD_FAIL;
    }
#endif
    /* reset clk */
    if (ssi_switch_clk(SSI_AON_CLKSEL_TCXO) != BOARD_SUCC) {
        PS_PRINT_ERR("set ssi clk fail\n");
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
}

int32 ssi_file_test(ssi_trans_test_st *pst_ssi_test)
{
    int32 i = 0;
    int32 ret;

    if (pst_ssi_test == NULL) {
        return BOARD_FAIL;
    }
    pst_ssi_test->trans_len = 0;

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    hi1103_chip_power_on();
    hi1103_bfgx_enable();
    if (hi1103_wifi_enable()) {
        PS_PRINT_ERR("hi1103_wifi_enable failed!\n");
        return BOARD_FAIL;
    }
#endif

    ret = ssi_file_test_pre();
    if (ret != BOARD_SUCC) {
        return ret;
    }

    /* file download */
    for (i = 0; i < sizeof(aSsiFile) / sizeof(ssi_file_st); i++) {
        if (do_ssi_file_test(&aSsiFile[i], pst_ssi_test) != BOARD_SUCC) {
            PS_PRINT_ERR("%s write %d error\n", aSsiFile[i].file_name, aSsiFile[i].write_addr);
            return BOARD_FAIL;
        }
    }

    ret = ssi_file_test_post();
    if (ret != BOARD_SUCC) {
        return ret;
    }

    return BOARD_SUCC;
}

static int32 do_ssi_mem_test(ssi_trans_test_st *pst_ssi_test)
{
    uint32 i = 0;
    uint32 ul_write_base = 0x0;
    uint32 ul_addr;
    int32 l_ret = BOARD_FAIL;
    if (pst_ssi_test == NULL) {
        return BOARD_FAIL;
    }

    for (i = 0; i < pst_ssi_test->trans_len; i++) {
        ul_addr = ul_write_base + 2 * i;  // 按2字节读写
        l_ret = ssi_write32_etc(ul_addr, SSI_WRITE_DATA);
        if (l_ret != BOARD_SUCC) {
            PS_PRINT_ERR(" write data error, ul_addr=0x%x, l_ret=%d\n", ul_addr, l_ret);
            return l_ret;
        }
        l_ret = ssi_read32_etc(ul_addr);
        if (l_ret != SSI_WRITE_DATA) {
            PS_PRINT_ERR("read write 0x%x error, expect:0x5a5a,actual:0x%x\n", ul_addr, l_ret);
            return l_ret;
        }
    }
    return BOARD_SUCC;
}

int32 ssi_download_test(ssi_trans_test_st *pst_ssi_test)
{
    int32 l_ret = BOARD_FAIL;
    const uint32 ul_test_trans_len = 1024;

    struct timeval stime, etime;

    if (pst_ssi_test == NULL) {
        return BOARD_FAIL;
    }
    pst_ssi_test->trans_len = ul_test_trans_len;
    if (ssi_request_gpio_etc(SSI_CLK_GPIO, SSI_DATA_GPIO) != BOARD_SUCC) {
        PS_PRINT_ERR("ssi_request_gpio_etc fail\n");
        goto fail_process;
    }

    do_gettimeofday(&stime);
    switch (pst_ssi_test->test_type) {
        case SSI_MEM_TEST:
            l_ret = do_ssi_mem_test(pst_ssi_test);
            break;
        case SSI_FILE_TEST:
            l_ret = ssi_file_test(pst_ssi_test);
            break;
        default:
            PS_PRINT_ERR("error type=%d\n", pst_ssi_test->test_type);
            break;
    }
    do_gettimeofday(&etime);
    ssi_free_gpio_etc();
    if (l_ret != BOARD_SUCC) {
        goto fail_process;
    }
    pst_ssi_test->used_time = (etime.tv_sec - stime.tv_sec) * 1000 + (etime.tv_usec - stime.tv_usec) / 1000;
    pst_ssi_test->send_status = 0;
    return BOARD_SUCC;
fail_process:
    pst_ssi_test->used_time = 0;
    pst_ssi_test->send_status = -1;
    return BOARD_FAIL;
}

#endif /* #ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG */

