

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
#include "ssi_hi1106.h"

#include "plat_debug.h"
#include "plat_pm.h"
#include "ssi_common.h"
#include "plat_parse_changid.h"

static hi1106_ssi_cpu_infos g_ssi_cpu_infos;

/* dump 寄存器定义 */
static ssi_reg_info g_hi1106_glb_ctrl_full = { 0x40000000, 0x1000, SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_glb_ctrl_extend1 = { 0x40001400, 0x30,   SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_coex_ctrl_full = { 0x4000D000, 0x354,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_pmu_cmu_ctrl_full = { 0x40002000, 0xb00,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_pmu2_cmu_ctrl_full = { 0x4000E000, 0xa20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_rf_tcxo_pll_ctrl_full = { 0x40010000, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_xoadc_ctrl_full = { 0x40013000, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_g_ctrl_full = { 0x40300000, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_g_diag_ctrl_full = { 0x40305000, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_g_rf_abb_ctrl_full = { 0x40307000, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_g_patch_ctrl_full = { 0x40301000, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_gcpu_trace_ctrl_full = { 0x40312000, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_b_ctrl_full = { 0x40200000, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_bf_diag_ctrl_full = { 0x40207000, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_bf_rf_abb_ctrl_full = { 0x40209000, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_b_patch_ctrl_full = { 0x40217000, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_fm_ctrl_full = { 0x40231000, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_bcpu_trace_ctrl_full = { 0x4021B000, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_w_ctrl_full = { 0x40105100, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_diag_ctrl_full = { 0x40124100, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_w_patch_ctrl_full = { 0x40126000, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_w_cpu_ctrl_full = { 0x40100100, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_wl_rf_abb_ch_full = { 0x40114000, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_wl_rf_abb_com_ctrl_full = { 0x40123000, 0x20,  SSI_RW_WORD_MOD };
static ssi_reg_info g_hi1106_pcie_ctrl_full = { 0x40108000, 0x4c8,  SSI_RW_WORD_MOD };

/* 1106 默认dump 所有寄存器 */
static ssi_reg_info *g_hi1106_aon_reg_full[] = {
    &g_hi1106_glb_ctrl_full,
    &g_hi1106_glb_ctrl_extend1,
    &g_hi1106_coex_ctrl_full,
    &g_hi1106_pmu_cmu_ctrl_full,
    &g_hi1106_pmu2_cmu_ctrl_full,
    &g_hi1106_rf_tcxo_pll_ctrl_full,
    &g_hi1106_xoadc_ctrl_full,
    &g_hi1106_g_ctrl_full,
    &g_hi1106_g_diag_ctrl_full,
    &g_hi1106_g_rf_abb_ctrl_full,
    &g_hi1106_g_patch_ctrl_full,
    &g_hi1106_gcpu_trace_ctrl_full,
    &g_hi1106_b_ctrl_full,
    &g_hi1106_bf_diag_ctrl_full,
    &g_hi1106_bf_rf_abb_ctrl_full,
    &g_hi1106_b_patch_ctrl_full,
    &g_hi1106_fm_ctrl_full,
    &g_hi1106_bcpu_trace_ctrl_full,
    &g_hi1106_w_ctrl_full,
    &g_hi1106_diag_ctrl_full,
    &g_hi1106_w_patch_ctrl_full,
    &g_hi1106_w_cpu_ctrl_full,
    &g_hi1106_wl_rf_abb_ch_full,
    &g_hi1106_wl_rf_abb_com_ctrl_full,
    &g_hi1106_pcie_ctrl_full
};

static void hi1106_dsm_cpu_info_dump(void)
{
    int32_t i;
    int32_t ret;
    int32_t count = 0;
    char buf[DSM_CPU_INFO_SIZE];
    /* dsm cpu信息上报 */
    if (g_halt_det_cnt || g_hi11xx_kernel_crash) {
        ps_print_info("g_halt_det_cnt=%u g_hi11xx_kernel_crash=%d dsm_cpu_info_dump return\n",
                      g_halt_det_cnt, g_hi11xx_kernel_crash);
        return;
    }

    /* 没有检测到异常，上报记录的CPU信息 */
    memset_s((void *)buf, sizeof(buf), 0, sizeof(buf));
    ret = snprintf_s(buf + count, sizeof(buf) - count, sizeof(buf) - count - 1,
                     "wcpu_state=0x%x %s, bcpu_state=0x%x %s, gcpu_state=0x%x %s ",
                     g_ssi_cpu_infos.wcpu0_info.cpu_state,
                     (g_ssi_cpu_st_str[g_ssi_cpu_infos.wcpu0_info.cpu_state & 0x7]),
                     g_ssi_cpu_infos.bcpu_info.cpu_state,
                     (g_ssi_cpu_st_str[g_ssi_cpu_infos.bcpu_info.cpu_state & 0x7]),
                     g_ssi_cpu_infos.gcpu_info.cpu_state,
                     (g_ssi_cpu_st_str[g_ssi_cpu_infos.gcpu_info.cpu_state & 0x7]));
    if (ret < 0) {
        goto done;
    }
    count += ret;

    for (i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++) {
        if (g_ssi_cpu_infos.wcpu0_info.reg_flag[i] == 0) {
            continue;
        }
        ret = snprintf_s(buf + count, sizeof(buf) - count, sizeof(buf) - count - 1,
                         "wcpu0[%d] pc:0x%x lr:0x%x sp:0x%x ", i, g_ssi_cpu_infos.wcpu0_info.pc[i],
                         g_ssi_cpu_infos.wcpu0_info.lr[i], g_ssi_cpu_infos.wcpu0_info.sp[i]);
        if (ret < 0) {
            goto done;
        }
        count += ret;
    }

done:
#ifdef CONFIG_HUAWEI_DSM
    hw_110x_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_1103_HALT, "%s\n", buf);
#else
    oal_io_print("log str format err [non-dsm]%s\n", buf);
#endif
}

static void hi1106_ssi_check_buck_scp_ocp_status(void)
{
    ps_print_info("hi1106_ssi_check_buck_scp_ocp_status bypass");
}

static int hi1106_ssi_check_wcpu_is_working(void)
{
    uint32_t reg, mask;

    /* pilot */
    reg = (uint32_t)ssi_read32(HI1106_PMU_CMU_CTL_SYS_STATUS_0_REG);
    mask = reg & 0x7;
    ps_print_info("cpu state=0x%8x, wcpu0 is %s\n", reg, g_ssi_cpu_st_str[mask]);
    g_ssi_cpu_infos.wcpu0_info.cpu_state = mask;
    if (mask == 0x5) {
        hi1106_ssi_check_buck_scp_ocp_status();
    }
    return (mask == 0x3);
}

static int hi1106_ssi_check_bcpu_is_working(void)
{
    uint32_t reg, mask;

    /* pilot */
    reg = (uint32_t)ssi_read32(HI1106_PMU_CMU_CTL_SYS_STATUS_0_REG);
    mask = (reg >> 3) & 0x7; /* reg shift right by 3 bits and 0x7 bits */
    ps_print_info("cpu state=0x%8x, bcpu is %s\n", reg, g_ssi_cpu_st_str[mask]);
    g_ssi_cpu_infos.bcpu_info.cpu_state = mask;
    if (mask == 0x5) {
        hi1106_ssi_check_buck_scp_ocp_status();
    }
    return (mask == 0x3);
}

static int hi1106_ssi_check_gcpu_is_working(void)
{
    uint32_t reg, mask;

    /* pilot */
    reg = (uint32_t)ssi_read32(HI1106_PMU_CMU_CTL_SYS_STATUS_0_REG);
    mask = (reg >> 6) & 0x7; /* reg shift right by 3 bits and 0x7 bits */
    ps_print_info("cpu state=0x%8x, gcpu is %s\n", reg, g_ssi_cpu_st_str[mask]);
    g_ssi_cpu_infos.gcpu_info.cpu_state = mask;
    if (mask == 0x5) {
        hi1106_ssi_check_buck_scp_ocp_status();
    }
    return (mask == 0x3);
}

int hi1106_ssi_read_wcpu_pc_lr_sp(void)
{
    int i;
    uint32_t reg_low, reg_high, pc_core0, pc_core1;

    /* read pc twice check whether cpu is runing */
    for (i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++) {
        // pc锁存 使能位:bit0
        ssi_write32(HI1106_GLB_CTL_WCPU0_LOAD_REG, 0x1);
        ssi_write32(HI1106_GLB_CTL_WCPU1_LOAD_REG, 0x1);
        oal_mdelay(1);

        reg_low = (uint32_t)ssi_read32(HI1106_GLB_CTL_WCPU0_PC_L_REG);
        reg_high = (uint32_t)ssi_read32(HI1106_GLB_CTL_WCPU0_PC_H_REG);
        pc_core0 = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        reg_low = (uint32_t)ssi_read32(HI1106_GLB_CTL_WCPU1_PC_L_REG);
        reg_high = (uint32_t)ssi_read32(HI1106_GLB_CTL_WCPU1_PC_H_REG);
        pc_core1 = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        // pc锁存 清除使能位:bit1
        ssi_write32(HI1106_GLB_CTL_WCPU0_LOAD_REG, 0x2);
        ssi_write32(HI1106_GLB_CTL_WCPU1_LOAD_REG, 0x2);
        oal_mdelay(1);

        ps_print_info("gpio-ssi:read wcpu0[%i], pc:0x%x; wcpu1[%i], pc:0x%x \n", i, pc_core0, i, pc_core1);

        if (!pc_core0 && !pc_core1) {
            ps_print_info("wcpu0 & wcpu1 pc all zero\n");
        } else {
            if (g_ssi_cpu_infos.wcpu0_info.reg_flag[i] == 0 && g_ssi_cpu_infos.wcpu1_info.reg_flag[i] == 0) {
                g_ssi_cpu_infos.wcpu0_info.reg_flag[i] = 1;
                g_ssi_cpu_infos.wcpu0_info.pc[i] = pc_core0;
                g_ssi_cpu_infos.wcpu1_info.reg_flag[i] = 1;
                g_ssi_cpu_infos.wcpu1_info.pc[i] = pc_core1;
            }
        }
        oal_mdelay(10); /* delay 10 ms */
    }

    return 0;
}

int hi1106_ssi_read_bpcu_pc_lr_sp(void)
{
    int i;
    uint32_t reg_low, reg_high, pc, lr, sp;

    /* read pc twice check whether cpu is runing */
    for (i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++) {
        ssi_write32(HI1106_GLB_CTL_BCPU_LOAD_REG, 0x1);
        oal_mdelay(1);

        reg_low = (uint32_t)ssi_read32(HI1106_GLB_CTL_BCPU_PC_L_REG);
        reg_high = (uint32_t)ssi_read32(HI1106_GLB_CTL_BCPU_PC_H_REG);
        pc = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        reg_low = (uint32_t)ssi_read32(HI1106_GLB_CTL_BCPU_LR_L_REG);
        reg_high = (uint32_t)ssi_read32(HI1106_GLB_CTL_BCPU_LR_H_REG);
        lr = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        reg_low = (uint32_t)ssi_read32(HI1106_GLB_CTL_BCPU_SP_L_REG);
        reg_high = (uint32_t)ssi_read32(HI1106_GLB_CTL_BCPU_SP_H_REG);
        sp = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        ps_print_info("gpio-ssi:read bcpu[%i], pc:0x%x, lr:0x%x, sp:0x%x \n", i, pc, lr, sp);
        if (!pc && !lr && !sp) {
            ps_print_info("bcpu pc lr sp all zero\n");
        } else {
            if (g_ssi_cpu_infos.bcpu_info.reg_flag[i] == 0) {
                g_ssi_cpu_infos.bcpu_info.reg_flag[i] = 1;
                g_ssi_cpu_infos.bcpu_info.pc[i] = pc;
                g_ssi_cpu_infos.bcpu_info.lr[i] = lr;
                g_ssi_cpu_infos.bcpu_info.sp[i] = sp;
            }
        }
        oal_mdelay(10); /* delay 10 ms */
    }

    return 0;
}

int hi1106_ssi_read_gpcu_pc_lr_sp(void)
{
    int i;
    uint32_t reg_low, reg_high, pc, lr, sp;

    /* read pc twice check whether cpu is runing */
    for (i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++) {
        ssi_write32(HI1106_GLB_CTL_GCPU_LOAD_REG, 0x1);
        oal_mdelay(1);

        reg_low = (uint32_t)ssi_read32(HI1106_GLB_CTL_GCPU_PC_L_REG);
        reg_high = (uint32_t)ssi_read32(HI1106_GLB_CTL_GCPU_PC_H_REG);
        pc = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        reg_low = (uint32_t)ssi_read32(HI1106_GLB_CTL_GCPU_LR_L_REG);
        reg_high = (uint32_t)ssi_read32(HI1106_GLB_CTL_GCPU_LR_H_REG);
        lr = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        reg_low = (uint32_t)ssi_read32(HI1106_GLB_CTL_GCPU_SP_L_REG);
        reg_high = (uint32_t)ssi_read32(HI1106_GLB_CTL_GCPU_SP_H_REG);
        sp = reg_low | (reg_high << 16); /* Shift leftwards by 16 bits */

        ps_print_info("gpio-ssi:read bcpu[%i], pc:0x%x, lr:0x%x, sp:0x%x \n", i, pc, lr, sp);
        if (!pc && !lr && !sp) {
            ps_print_info("gcpu pc lr sp all zero\n");
        } else {
            if (g_ssi_cpu_infos.gcpu_info.reg_flag[i] == 0) {
                g_ssi_cpu_infos.gcpu_info.reg_flag[i] = 1;
                g_ssi_cpu_infos.gcpu_info.pc[i] = pc;
                g_ssi_cpu_infos.gcpu_info.lr[i] = lr;
                g_ssi_cpu_infos.gcpu_info.sp[i] = sp;
            }
        }
        oal_mdelay(10); /* delay 10 ms */
    }

    return 0;
}

int hi1106_ssi_read_device_arm_register(void)
{
    int32_t ret;

    ret = hi1106_ssi_check_wcpu_is_working();
    if (ret < 0) {
        return ret;
    }
    if (ret) {
        hi1106_ssi_read_wcpu_pc_lr_sp();
    }
    bfgx_print_subsys_state();
    ret = hi1106_ssi_check_bcpu_is_working();
    if (ret < 0) {
        return ret;
    }
    if (ret) {
        hi1106_ssi_read_bpcu_pc_lr_sp();
    }
    ret = hi1106_ssi_check_gcpu_is_working();
    if (ret < 0) {
        return ret;
    }
    if (ret) {
        hi1106_ssi_read_gpcu_pc_lr_sp();
    }

    // 打印device版本
    plat_changid_print();

    return 0;
}

static int hi1106_ssi_dump_device_aon_regs(unsigned long long module_set)
{
    int ret;
    if (module_set & SSI_MODULE_MASK_AON) {
        ret = ssi_read_reg_info_arry(g_hi1106_aon_reg_full, sizeof(g_hi1106_aon_reg_full) / sizeof(ssi_reg_info *),
                                     g_ssi_is_logfile);
        if (ret) {
            return -OAL_EFAIL;
        }
    }

    return OAL_SUCC;
}

static int hi1106_ssi_dump_device_arm_regs(unsigned long long module_set)
{
    int ret;

    if (module_set & SSI_MODULE_MASK_ARM_REG) {
        ret = hi1106_ssi_read_device_arm_register();
        if (ret) {
            goto ssi_fail;
        }
    }

    return 0;
ssi_fail:
    return ret;
}

int hi1106_ssi_device_regs_dump(unsigned long long module_set)
{
    int ret;
    g_halt_det_cnt = 0;

    memset_s(&g_ssi_cpu_infos, sizeof(g_ssi_cpu_infos), 0, sizeof(g_ssi_cpu_infos));

    ssi_read16(gpio_ssi_reg(SSI_SSI_CTRL));
    ssi_read16(gpio_ssi_reg(SSI_SEL_CTRL));

    ssi_switch_clk(SSI_AON_CLKSEL_SSI);

    ret = ssi_check_device_isalive();
    if (ret) {
        /* try to reset aon */
        ssi_force_reset_reg();
        ssi_switch_clk(SSI_AON_CLKSEL_SSI);
        if (ssi_check_device_isalive()) {
            ps_print_info("after reset aon, ssi still can't work\n");
            goto ssi_fail;
        } else {
            ps_print_info("after reset aon, ssi ok, dump acp/ocp reg\n");
            hi1106_ssi_check_buck_scp_ocp_status();
            module_set = SSI_MODULE_MASK_COMM;
        }
    } else {
        hi1106_ssi_check_buck_scp_ocp_status();
    }

    /* try to read pc&lr&sp regs */
    ret = hi1106_ssi_dump_device_arm_regs(module_set);
    if (ret) {
        goto ssi_fail;
    }

    ret = ssi_check_device_isalive();
    if (ret) {
        goto ssi_fail;
    }

    /* try to read all aon regs */
    ret = hi1106_ssi_dump_device_aon_regs(module_set);
    if (ret != OAL_SUCC) {
        goto ssi_fail;
    }

    ret = hi1106_ssi_dump_device_arm_regs(module_set);
    if (ret) {
        goto ssi_fail;
    }

    ssi_switch_clk(SSI_AON_CLKSEL_TCXO);
    hi1106_dsm_cpu_info_dump();

    return 0;

ssi_fail:
    ssi_switch_clk(SSI_AON_CLKSEL_TCXO);
    hi1106_dsm_cpu_info_dump();
    return ret;
}

#endif /* #ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG */