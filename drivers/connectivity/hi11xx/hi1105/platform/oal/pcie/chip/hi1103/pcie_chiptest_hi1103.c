

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#define HI11XX_LOG_MODULE_NAME "[PCIE_H]"
#define HISI_LOG_TAG           "[PCIE]"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include "oal_thread.h"
#include "oam_ext_if.h"
#include "pcie_linux.h"
#include "pcie_reg.h"
#include "oal_hcc_host_if.h"
#include "oal_kernel_file.h"
#include "plat_firmware.h"
#include "plat_pm_wlan.h"
#include "board.h"
#include "securec.h"
#define PCIE_MEM_SCAN_VERFIY_VAL_1 0x5a
#define PCIE_MEM_SCAN_VERFIY_VAL_2 0xa5

/* tcxo->640M */
#define HI1103_PA_GLB_CTL_BASE_ADDR 0x50000000
#define HI1103_PA_PMU_CMU_CTL_BASE  0x50002000
#define HI1103_PA_PMU2_CMU_IR_BASE  0x50003000
#define HI1103_PA_W_CTL_BASE        0x40000000

#define HI1103_GLB_CTL_BASE_ADDR pst_glb_ctrl

#define HI1103_PMU_CMU_BASE pst_pmu_cmu_ctrl

#define HI1103_PMU2_CMU_IR_BASE pst_pmu2_cmu_ir_ctrl

#define HI1103_PMU2_CMU_IR_PMU_LNALDO0_EN_DELAY_STS_OFFSET 1
#define HI1103_PMU2_CMU_IR_PMU_LNALDO0_EN_STS_OFFSET       0
#define HI1103_PMU2_CMU_IR_PMU_LNALDO0_EN_DELAY_STS_MSK ((0x1UL) << HI1103_PMU2_CMU_IR_PMU_LNALDO0_EN_DELAY_STS_OFFSET)
#define HI1103_PMU2_CMU_IR_PMU_LNALDO0_EN_STS_MSK       ((0x1UL) << HI1103_PMU2_CMU_IR_PMU_LNALDO0_EN_STS_OFFSET)

#define HI1103_PMU2_CMU_IR_FBDIV_FRAC_SLP_STS_LEN    1
#define HI1103_PMU2_CMU_IR_FBDIV_FRAC_SLP_STS_OFFSET 0
#define HI1103_PMU2_CMU_IR_FBDIV_FRAC_WKUP_MSK       ((0x1UL) << HI1103_PMU2_CMU_IR_FBDIV_FRAC_WKUP_OFFSET)
#define HI1103_PMU2_CMU_IR_FBDIV_FRAC_SLP_STS_MSK    ((0x1UL) << HI1103_PMU2_CMU_IR_FBDIV_FRAC_SLP_STS_OFFSET)

#define HI1103_PMU2_CMU_IR_PMU2_CMU_ABB_MAN_SEL_W_2_REG   (HI1103_PMU2_CMU_IR_BASE + 0x148) /* PMU2_CMU_ABB 手动选择 */
#define HI1103_PMU2_CMU_IR_PMU_RFLDO3_EN_MAN_W_SEL_OFFSET 6

#define HI1103_PMU2_CMU_IR_PMU2_CMU_ABB_STS_2_REG (HI1103_PMU2_CMU_IR_BASE + 0x15C) /* PMU2_CMU_ABB 实际状态 */

#define HI1103_PMU2_CMU_IR_PMU2_CMU_ABB_MAN_SEL_W_3_REG   (HI1103_PMU2_CMU_IR_BASE + 0x168) /* PMU2_CMU_ABB 手动选择 */
#define HI1103_PMU2_CMU_IR_PMU_RFLDO6_EN_MAN_W_SEL_OFFSET 4

#define HI1103_PMU2_CMU_IR_PMU2_CMU_ABB_STS_3_REG (HI1103_PMU2_CMU_IR_BASE + 0x17C) /* PMU2_CMU_ABB 实际状态 */

#define HI1103_PMU2_CMU_IR_PMU2_CMU_ABB_MAN_SEL_W_6_REG (HI1103_PMU2_CMU_IR_BASE + 0x1C8) /* PMU2_CMU_ABB 手动选择 */
#define HI1103_PMU2_CMU_IR_PMU_XLDO_EN_MAN_W_SEL_OFFSET 0

#define HI1103_PMU2_CMU_IR_PMU2_CMU_ABB_STS_6_REG (HI1103_PMU2_CMU_IR_BASE + 0x1DC) /* PMU2_CMU_ABB 实际状态 */

#define HI1103_PMU2_CMU_IR_PMU2_CMU_ABB_MAN_SEL_W_8_REG (HI1103_PMU2_CMU_IR_BASE + 0x208) /* PMU2_CMU_ABB 手动选择 */
#define HI1103_PMU2_CMU_IR_PMU2_CMU_ABB_MAN_STS_8_REG   (HI1103_PMU2_CMU_IR_BASE + 0x210) /* PMU2_CMU_ABB 手动状态 */
#define HI1103_PMU2_CMU_IR_PMU2_CMU_ABB_DBG_SEL_8_REG   (HI1103_PMU2_CMU_IR_BASE + 0x218) /* PMU2_CMU_ABB 调试选择 */

#define HI1103_PMU2_CMU_IR_REFDIV_REG    (HI1103_PMU2_CMU_IR_BASE + 0x300) /* CMU 分频比配置 */
#define HI1103_PMU2_CMU_IR_FBDIV_REG     (HI1103_PMU2_CMU_IR_BASE + 0x304) /* CMU 分频比配置 */
#define HI1103_PMU2_CMU_IR_FRAC_L_REG    (HI1103_PMU2_CMU_IR_BASE + 0x308) /* CMU 分频比配置 */
#define HI1103_PMU2_CMU_IR_FRAC_H_REG    (HI1103_PMU2_CMU_IR_BASE + 0x30C) /* CMU 分频比配置 */
#define HI1103_PMU2_CMU_IR_POSTDIV_REG   (HI1103_PMU2_CMU_IR_BASE + 0x310) /* CMU 分频比配置 */
#define HI1103_PMU2_CMU_IR_CMU_PD_REG    (HI1103_PMU2_CMU_IR_BASE + 0x314) /* CMU PD 控制 */
#define HI1103_PMU2_CMU_IR_CMU_GT_W_REG  (HI1103_PMU2_CMU_IR_BASE + 0x318) /* CMU GT 控制 */
#define HI1103_PMU2_CMU_IR_TCXO_GT_W_REG (HI1103_PMU2_CMU_IR_BASE + 0x31C) /* TCXO GT 控制 */

#define HI1103_PMU2_CMU_IR_AON_DIV_1_REG    (HI1103_PMU2_CMU_IR_BASE + 0x418)
#define HI1103_PMU2_CMU_IR_AON_CRG_CKEN_REG (HI1103_PMU2_CMU_IR_BASE + 0x420)
#define HI1103_PMU2_CMU_IR_CLK_SEL_REG      (HI1103_PMU2_CMU_IR_BASE + 0x430)

#define HI1103_PMU2_CMU_IR_FBDIV_FRAC_SLP_REG     (HI1103_PMU2_CMU_IR_BASE + 0x370) /* CMU 分频比控制 */
#define HI1103_PMU2_CMU_IR_FBDIV_FRAC_WKUP_OFFSET 1

#define HI1103_PMU2_CMU_IR_CMU_STATUS_GRM_REG (HI1103_PMU2_CMU_IR_BASE + 0x890) /* CMU状态查询 */

#define HI1103_W_CTL_BASE                   pst_wctrl
#define HI1103_W_CTL_WTOPCRG_SOFT_CLKEN_REG (HI1103_W_CTL_BASE + 0x40)
#define HI1103_W_CTL_W_TCXO_SEL_REG         (HI1103_W_CTL_BASE + 0x80) /* WLAN TCXO/PLL时钟模式选择寄存器 */
#define HI1103_W_CTL_CLKMUX_STS_REG         (HI1103_W_CTL_BASE + 0x88)

int32_t g_ft_pcie_wcpu_max_freq_bypass = 0;
oal_debug_module_param(g_ft_pcie_wcpu_max_freq_bypass, int, S_IRUGO | S_IWUSR);

int32_t g_ft_pcie_link_err_bypass = 0;
oal_debug_module_param(g_ft_pcie_link_err_bypass, int, S_IRUGO | S_IWUSR);

int32_t g_ft_pcie_wcpu_mem_check_times = 1;
oal_debug_module_param(g_ft_pcie_wcpu_mem_check_times, int, S_IRUGO | S_IWUSR);

int32_t g_ft_pcie_perf_runtime = 200;
oal_debug_module_param(g_ft_pcie_perf_runtime, int, S_IRUGO | S_IWUSR);

int32_t g_ft_pcie_perf_wr_bypass = 0;
oal_debug_module_param(g_ft_pcie_perf_wr_bypass, int, S_IRUGO | S_IWUSR);

int32_t g_ft_pcie_perf_rd_bypass = 1;
oal_debug_module_param(g_ft_pcie_perf_rd_bypass, int, S_IRUGO | S_IWUSR);

int32_t g_ft_pcie_wcpu_mem_check_burst_check = 0;
oal_debug_module_param(g_ft_pcie_wcpu_mem_check_burst_check, int, S_IRUGO | S_IWUSR);

int32_t g_ft_pcie_wcpu_mem_check_byword_bypass = 1;
oal_debug_module_param(g_ft_pcie_wcpu_mem_check_byword_bypass, int, S_IRUGO | S_IWUSR);

int32_t g_ft_pcie_write_address_bypass = 0;
oal_debug_module_param(g_ft_pcie_write_address_bypass, int, S_IRUGO | S_IWUSR);

/* factory test */
/* mpw2 */
OAL_STATIC uintptr_t g_mpw2_wmem_scan_array[][PCIE_MEM_MSG_SIZE] = {
    { 0x00004000, 0x0008BFFF }, /* itcm ram */
    { 0x20000000, 0x20067FFF }, /* dtcm */
    { 0x60000000, 0x6008FFFF }, /* share mem */
};

/* pilot */
OAL_STATIC uintptr_t g_pilot_wmem_scan_array[][PCIE_MEM_MSG_SIZE] = {
    { 0x00040000, 0x000A7FFF }, /* itcm ram */
    { 0x20020000, 0x2007FFFF }, /* dtcm */
    { 0x60000000, 0x6007FFFF }, /* share mem */
};

OAL_STATIC uintptr_t g_mpw2_bmem_scan_array[][PCIE_MEM_MSG_SIZE] = {
    { 0x80000000, 0x800FFFFF }, /* itcm */
    { 0x80100000, 0x801DFFFF }, /* dtcm */
};

/* pilot */
OAL_STATIC uintptr_t g_pilot_bmem_scan_array[][PCIE_MEM_MSG_SIZE] = {
    { 0x80040000, 0x8010FFFF }, /* itcm */
    { 0x80200000, 0x8030FFFF }, /* dtcm */
};

/* 1105 */
OAL_STATIC uintptr_t g_hi1105_asic_wmem_scan_array[][PCIE_MEM_MSG_SIZE] = {
    { 0x00040000, 0x000BFFFF }, /* itcm ram */
    { 0x20020000, 0x2007FFFF }, /* dtcm */
    { 0x60000000, 0x600FFFFF }, /* share mem */
};

OAL_STATIC uintptr_t g_hi1105_fpga_wmem_scan_array[][PCIE_MEM_MSG_SIZE] = {
    { 0x00010000, 0x00037FFF }, /* itcm rom */
    { 0x00040000, 0x000BFFFF }, /* itcm ram */
    { 0x20020000, 0x2007FFFF }, /* dtcm */
    { 0x60000000, 0x6007FFFF }, /* share mem */
};

OAL_STATIC uintptr_t g_hi1105_asic_bmem_scan_array[][PCIE_MEM_MSG_SIZE] = {
    { 0x80040000, 0x8014FFFF }, /* itcm */
    { 0x80200000, 0x8030FFFF }, /* dtcm */
};

oal_reg_bits_stru g_pcie_phy_0v9_bits[] = {
    { 700, 0x0, "0.7v" },
    { 750, 0x1, "0.75v" },
    { 775, 0x2, "0.775v" },
    { 800, 0x3, "0.8v" },
    { 825, 0x4, "0.825v" },
    { 850, 0x5, "0.85v"},
    { 875, 0x6, "0.875v"},
    { 900,  0x7, "0.9v" },
    { 925,  0x8, "0.925v" },
    { 950,  0x9, "0.950v" },
    { 975,  0xa, "0.975v" },
    { 1000, 0xb, "1v" },
    { 1025, 0xc, "1.025v" },
    { 1050, 0xd, "1.050v" },
    { 1075, 0xe, "1.075v" },
    { 1100, 0xf, "1.1v" }
};

oal_reg_bits_stru g_pcie_phy_1v8_bits[] = {
    { 1600, 0x0, "1.6v" },
    { 1625, 0x1, "1.625v" },
    { 1650, 0x2, "1.650v" },
    { 1675, 0x3, "1.675v" },
    { 1700, 0x4, "1.700v" },
    { 1725, 0x5, "1.725v" },
    { 1750, 0x6, "1.750v" },
    { 1775, 0x7, "1.775v" },
    { 1800, 0x8, "1.800v" }
};

OAL_STATIC int32_t oal_pcie_device_read32_timeout(void *pst_va, uint32_t target,
                                                  uint32_t mask, unsigned long timeout)
{
    uint32_t value;
    unsigned long timeout_t;
    timeout_t = jiffies + oal_msecs_to_jiffies(hi110x_get_emu_timeout(timeout));
    forever_loop() {
        value = oal_readl(pst_va);
        if ((value & mask) == target) {
            return OAL_SUCC;
        }

        if (timeout == 0) {
            return -OAL_EFAIL;
        }

        if (oal_time_after(jiffies, timeout_t)) {
            break;
        }

        cpu_relax();
    }

    oal_print_hi11xx_log(HI11XX_LOG_WARN, "%p expect 0x%x , read 0x%x, mask 0x%x, timeout=%lu ms",
                         pst_va, target, value, mask, timeout);
    return -OAL_ETIMEDOUT;
}

#define PCIE_DEV_FREQ_CTIMEOUT  100
static void oal_pcie_change_highfreq_regconfig(void *pst_glb_ctrl, void *pst_pmu_cmu_ctrl,
                                               void *pst_pmu2_cmu_ir_ctrl, void *pst_wctrl)
{
    uint32_t value;
    /* sysldo power chagne from vddio to buck bit8 */
    oal_setl_bit(pst_pmu_cmu_ctrl + 0xc8, 8);

    oal_setl_bit(pst_pmu_cmu_ctrl + 0x88, 6); // bit6

    oal_setl_bit(pst_pmu_cmu_ctrl + 0x88, 12); // bit12
    oal_readl(pst_pmu_cmu_ctrl + 0x88);
    oal_udelay(30); // delay 30us

    oal_setl_bit(pst_pmu_cmu_ctrl + 0x88, 2);  // bit2
    oal_readl(pst_pmu_cmu_ctrl + 0x88);
    oal_udelay(60);  // delay 60us

    oal_setl_bit(pst_pmu_cmu_ctrl + 0x88, 1); // bit1
    oal_readl(pst_pmu_cmu_ctrl + 0x88);
    oal_udelay(30); // delay 30us

    oal_clearl_bit(pst_pmu_cmu_ctrl + 0x88, 12); // bit12

    oal_setl_bit(pst_pmu_cmu_ctrl + 0x88, 13); // bit13

    // clk_960m_sel :2->3, frq=640M
    oal_writel(0x31b, pst_pmu2_cmu_ir_ctrl + 0x328);

    // buck_clamp_sel: 1>3>2
    oal_setl_bit(pst_pmu_cmu_ctrl + 0x170, 0);

    /* open intldo1 bit2 */
    oal_setl_bit(pst_pmu2_cmu_ir_ctrl + 0x108, 2);

    oal_pcie_device_read32_timeout((pst_pmu2_cmu_ir_ctrl + 0x11c), 0xc, 0xc, PCIE_DEV_FREQ_CTIMEOUT);

    /* open xldo bit14 */
    oal_setl_bit(pst_pmu2_cmu_ir_ctrl + 0x148, 14);

    oal_pcie_device_read32_timeout((pst_pmu2_cmu_ir_ctrl + 0x15c), 0xc000, 0xc000, PCIE_DEV_FREQ_CTIMEOUT);

    oal_setl_bits(HI1103_PMU2_CMU_IR_REFDIV_REG, 0, 6, 0x1); // 6bits [0-5] to 0x1
    oal_writel(0x19, HI1103_PMU2_CMU_IR_FBDIV_REG);
    oal_writel(0x0, HI1103_PMU2_CMU_IR_FRAC_H_REG);
    oal_writel(0x0, HI1103_PMU2_CMU_IR_FRAC_L_REG);
    oal_writel(0x11, HI1103_PMU2_CMU_IR_POSTDIV_REG);

    oal_value_mask(pst_pmu2_cmu_ir_ctrl + 0x168, BIT8, BIT9 | BIT8);
    oal_pcie_device_read32_timeout((pst_pmu2_cmu_ir_ctrl + 0x17c), 0x200, 0x200, PCIE_DEV_FREQ_CTIMEOUT);

    oal_clearl_mask(HI1103_PMU2_CMU_IR_CMU_PD_REG, (BIT4 | BIT3 | BIT1));
    oal_readl(HI1103_PMU2_CMU_IR_CMU_PD_REG);
    oal_msleep(1); // sched sleep 1ms

    oal_pcie_device_read32_timeout(pst_pmu2_cmu_ir_ctrl + 0x880, 0x0, 0x1, PCIE_DEV_FREQ_CTIMEOUT);

    oal_clearl_mask(HI1103_PMU2_CMU_IR_CMU_GT_W_REG, (BIT2 | BIT5));

    oal_clearl_mask(HI1103_PMU2_CMU_IR_CMU_GT_W_REG, (BIT3 | BIT0));

    oal_clearl_bit(HI1103_PMU2_CMU_IR_TCXO_GT_W_REG, 4); // bit4

    oal_setl_mask(HI1103_W_CTL_WTOPCRG_SOFT_CLKEN_REG, (BIT8));

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "change 640M set");
    oal_setl_bit(HI1103_W_CTL_W_TCXO_SEL_REG, 0); // bit0

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "change 640M wait start");
    oal_msleep(10); /* 防止这里高频切出问题，下面只回读一次, delay 10ms */
    value = oal_readl(HI1103_W_CTL_CLKMUX_STS_REG);
    if ((value & 0x2) != 0x2) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "change 640M wait failed, clkmux=0x%x", value);
        ssi_dump_device_regs(SSI_MODULE_MASK_COMM);
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "change 640M wait ok, clkmux=0x%x", value);
    }
}

static int32_t oal_pcie_changefreq_get_soc_regs(oal_pcie_res *pst_pci_res, uintptr_t *pst_glb_ctrl,
                                                uintptr_t *pst_pmu_cmu_ctrl, uintptr_t *pst_pmu2_cmu_ir_ctrl,
                                                uintptr_t *pst_wctrl)
{
    int32_t ret;
    pci_addr_map addr_map;

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, HI1103_PA_GLB_CTL_BASE_ADDR, &addr_map);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "can not found mem map for dev cpu address 0x%x\n",
                             HI1103_PA_GLB_CTL_BASE_ADDR);
        return -OAL_EFAIL;
    }

    *pst_glb_ctrl = addr_map.va;

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, HI1103_PA_PMU_CMU_CTL_BASE, &addr_map);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "can not found mem map for dev cpu address 0x%x\n",
                             HI1103_PA_PMU_CMU_CTL_BASE);
        return -OAL_EFAIL;
    }

    *pst_pmu_cmu_ctrl = addr_map.va;

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, HI1103_PA_PMU2_CMU_IR_BASE, &addr_map);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "can not found mem map for dev cpu address 0x%x\n",
                             HI1103_PA_PMU2_CMU_IR_BASE);
        return -OAL_EFAIL;
    }

    *pst_pmu2_cmu_ir_ctrl = addr_map.va;

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, HI1103_PA_W_CTL_BASE, &addr_map);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "can not found mem map for dev cpu address 0x%x\n", HI1103_PA_W_CTL_BASE);
        return -OAL_EFAIL;
    }

    *pst_wctrl = addr_map.va;

    return OAL_SUCC;
}

int32_t oal_pcie_device_changeto_high_cpufreq(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    void *pst_glb_ctrl = NULL;         /* 0x50000000 */
    void *pst_pmu_cmu_ctrl = NULL;     /* 0x50002000 */
    void *pst_pmu2_cmu_ir_ctrl = NULL; /* 0x50003000 */
    void *pst_wctrl = NULL;

    if (g_ft_pcie_wcpu_max_freq_bypass) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie_wcpu_max_freq_bypass");
        return OAL_SUCC;
    }

    ret = oal_pcie_changefreq_get_soc_regs(pst_pci_res, (uintptr_t*)&pst_glb_ctrl, (uintptr_t*)&pst_pmu_cmu_ctrl,
                                           (uintptr_t*)&pst_pmu2_cmu_ir_ctrl, (uintptr_t*)&pst_wctrl);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "get soc regs failed, ret=%d", ret);
        return -OAL_EFAIL;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "pst_pci_res->revision %u", pst_pci_res->revision);

    if (pst_pci_res->revision == PCIE_REVISION_4_70A) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie 4.70a mpw2 freq");
        return -OAL_ENODEV;
    } else if (pst_pci_res->revision == PCIE_REVISION_5_00A) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie 5.00a pilot freq from 76.8M");
        /* tcxo to 38.4M */
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "tcxo div=0x%x", oal_readl(pst_pmu2_cmu_ir_ctrl + 0x338));
        oal_pcie_change_highfreq_regconfig(pst_glb_ctrl, pst_pmu_cmu_ctrl, pst_pmu2_cmu_ir_ctrl, pst_wctrl);
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "unsupport revision: %u", pst_pci_res->revision);
        return -OAL_ENODEV;
    }

    ret = oal_pcie_device_check_alive(pst_pci_res);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "change 640M failed");
        ssi_dump_device_regs(SSI_MODULE_MASK_ARM_REG);
        return ret;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "change 640M done");
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_device_scan_reg(oal_pcie_res *pst_pci_res)
{
    return OAL_SUCC;
}

int32_t oal_pcie_get_gen_mode(oal_pcie_res *pst_pci_res)
{
    uint32_t value = (uint32_t)oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_STATUS2_OFF);
    value = ((value >> 9) & 0x1);
    if (value == 0) {
        return PCIE_GEN1;
    } else {
        return PCIE_GEN2;
    }
}

int32_t g_pcie_ldo_phy_0v9_param = 0;
oal_debug_module_param(g_pcie_ldo_phy_0v9_param, int, S_IRUGO | S_IWUSR);

int32_t g_pcie_ldo_phy_1v8_param = 0;
oal_debug_module_param(g_pcie_ldo_phy_1v8_param, int, S_IRUGO | S_IWUSR);

void oal_pcie_set_voltage_bias_param(uint32_t phy_0v9_bias, uint32_t phy_1v8_bias)
{
    g_pcie_ldo_phy_0v9_param = phy_0v9_bias;
    g_pcie_ldo_phy_1v8_param = phy_1v8_bias;
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "param 0v9=%u 1v8=%u", phy_0v9_bias, phy_1v8_bias);
}

void oal_pcie_voltage_bias_param_init(void)
{
    char *token = NULL;
    char bias_param[100]; /* pcie vol bias 关键字长度最大为100 */
    uint32_t param[10];   /* pcie vol bias 参数个数最大为10 */
    uint32_t param_nums;
    char *bias_buf = bias_param;

    memset_s(bias_param, sizeof(bias_param), 0, sizeof(bias_param));
    memset_s(param, sizeof(param), 0, sizeof(param));

    if (get_cust_conf_string(INI_MODU_PLAT, "pcie_vol_bias_param", bias_param, sizeof(bias_param) - 1) == INI_FAILED) {
        return;
    }

    param_nums = 0;
    forever_loop() {
        token = strsep(&bias_buf, ",");
        if (token == NULL) {
            break;
        }
        if (param_nums >= sizeof(param) / sizeof(uint32_t)) {
            pci_print_log(PCI_LOG_ERR, "too many bias param:%u", param_nums);
            return;
        }

        param[param_nums++] = oal_strtol(token, NULL, 10); /* 10进制 */
        pci_print_log(PCI_LOG_INFO, "bias param %u , value is %u", param_nums, param[param_nums - 1]);
    }

    oal_pcie_set_voltage_bias_param(param[0], param[1]);
}

OAL_STATIC int32_t oal_pcie_get_vol_reg_bits_value(uint32_t target_value, oal_reg_bits_stru *pst_reg_bits,
                                                   uint32_t nums, uint32_t *pst_value)
{
    int32_t i;
    oal_reg_bits_stru *pst_tmp = NULL;
    for (i = 0; i < nums; i++) {
        pst_tmp = pst_reg_bits + i;
        if (target_value == pst_tmp->flag) {
            *pst_value = pst_tmp->value;
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "flag:%u matched value:0x%x , [%s]",
                                 target_value, *pst_value, pst_tmp->name);
            return OAL_SUCC;
        }
    }

    return -OAL_ENODEV;
}

int32_t oal_pcie_get_vol_reg_1v8_value(int32_t request_vol, uint32_t *pst_value)
{
    int32_t ret;
    ret = oal_pcie_get_vol_reg_bits_value(request_vol, g_pcie_phy_1v8_bits,
                                          oal_array_size(g_pcie_phy_1v8_bits), pst_value);
    if (ret != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "invalid pcie ldo bias phy 1v8 param =%d mV", request_vol);
        return -OAL_EINVAL;
    }
    return OAL_SUCC;
}
int32_t oal_pcie_get_vol_reg_0v9_value(int32_t request_vol, uint32_t *pst_value)
{
    int32_t ret;
    ret = oal_pcie_get_vol_reg_bits_value(request_vol, g_pcie_phy_0v9_bits,
                                          oal_array_size(g_pcie_phy_0v9_bits), pst_value);
    if (ret != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "invalid pcie ldo bias phy 0v9 param =%d mV", request_vol);
        return -OAL_EINVAL;
    }
    return OAL_SUCC;
}

/* 电压拉偏初始化 */
int32_t oal_pcie_voltage_bias_init_hi1103(oal_pcie_res *pst_pci_res)
{
    /* vp,vptx,vph 降压 5% */
    int32_t ret;
    uint32_t value, phy_0v9_bias, phy_1v8_bias;
    pci_addr_map addr_map;
    void *pst_pmu_cmu_ctrl = NULL;     /* 0x50002000 */
    void *pst_pmu2_cmu_ir_ctrl = NULL; /* 0x50003000 */

    if (oal_warn_on(pst_pci_res == NULL)) {
        return -OAL_ENODEV;
    }

    if (g_pcie_ldo_phy_0v9_param == 0 ||
        g_pcie_ldo_phy_1v8_param == 0) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "phy 0v9=%u mV, 1v8=%u mV invaild, pcie vol bias bypass",
                             g_pcie_ldo_phy_0v9_param, g_pcie_ldo_phy_1v8_param);
        return OAL_SUCC;
    }

    ret = oal_pcie_get_vol_reg_bits_value(g_pcie_ldo_phy_0v9_param, g_pcie_phy_0v9_bits,
                                          oal_array_size(g_pcie_phy_0v9_bits), &phy_0v9_bias);
    if (ret != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "invalid pcie ldo bias phy 0v9 param =%d mV", g_pcie_ldo_phy_0v9_param);
        return -OAL_EINVAL;
    }

    ret = oal_pcie_get_vol_reg_bits_value(g_pcie_ldo_phy_1v8_param, g_pcie_phy_1v8_bits,
                                          oal_array_size(g_pcie_phy_1v8_bits), &phy_1v8_bias);
    if (ret != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "invalid pcie ldo bias phy 1v8 param =%d mV", g_pcie_ldo_phy_1v8_param);
        return -OAL_EINVAL;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, HI1103_PA_PMU_CMU_CTL_BASE, &addr_map);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "can not found mem map for dev cpu address 0x%x\n",
                             HI1103_PA_PMU_CMU_CTL_BASE);
        return -OAL_EFAIL;
    }

    pst_pmu_cmu_ctrl = (void *)addr_map.va;

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, HI1103_PA_PMU2_CMU_IR_BASE, &addr_map);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "can not found mem map for dev cpu address 0x%x\n",
                             HI1103_PA_PMU2_CMU_IR_BASE);
        return -OAL_EFAIL;
    }

    pst_pmu2_cmu_ir_ctrl = (void *)addr_map.va;

    /* PCIELDO0V9_CFG0 */
    value = oal_readl(pst_pmu_cmu_ctrl + 0x0128);
    value &= ~(0xf << 8); // 8, bit pos
    value &= ~(0xf << 12); // 12, bit pos
    value |= (phy_0v9_bias << 8);
    value |= (phy_0v9_bias << 12);
    oal_writel(value, (pst_pmu_cmu_ctrl + 0x0128));

    /* PCIELDO1V8_CFG0 */
    value = oal_readl(pst_pmu2_cmu_ir_ctrl + 0x0268);
    value &= ~(0xf << 8);
    value |= (phy_1v8_bias << 8);
    oal_writel(value, (pst_pmu2_cmu_ir_ctrl + 0x0268));

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcieldo0v9 reg=0x%x, pcieldo1v8 reg=0x%x",
                         oal_readl(pst_pmu_cmu_ctrl + 0x0128), oal_readl(pst_pmu2_cmu_ir_ctrl + 0x0268));

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_print_device_aer_cap_reg(oal_pcie_res *pst_pci_res)
{
    int32_t  pos_cap_aer;
    uint32_t uncor = 0;
    uint32_t cor   = 0;
    oal_pci_dev_stru *pst_pci_dev;
    pst_pci_dev = pcie_res_to_dev(pst_pci_res);

    pos_cap_aer = oal_pci_find_ext_capability(pst_pci_dev, PCI_EXT_CAP_ID_ERR);
    if (pos_cap_aer == 0) {
        return -OAL_EFAIL;
    }

    /* 状态寄存器读清 */
    if (oal_pci_read_config_dword(pst_pci_dev, pos_cap_aer + PCI_ERR_UNCOR_STATUS, &uncor)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "PCI_ERR_UNCOR_STATUS: read fail");
        return -OAL_EFAIL;
    }
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "PCI_ERR_UNCOR_STATUS: 0x%x", uncor);

    if (oal_pci_read_config_dword(pst_pci_dev, pos_cap_aer + PCI_ERR_COR_STATUS, &cor)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "PCI_ERR_COR_STATUS: read fail");
        return -OAL_EFAIL;
    }
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "PCI_ERR_COR_STATUS: 0x%x", cor);
#ifdef _PRE_COMMENT_CODE_
    oal_pci_write_config_dword(pst_pci_dev, pos_cap_aer + PCI_ERR_UNCOR_STATUS, uncor);
    oal_pci_write_config_dword(pst_pci_dev, pos_cap_aer + PCI_ERR_COR_STATUS, cor);
#endif
    return OAL_SUCC;
}

int32_t oal_pcie_unmask_device_link_erros(oal_pcie_res *pst_pci_res)
{
    uint32_t ucor_mask = 0;
    int32_t pos_cap_aer;
    oal_pci_dev_stru *pst_pci_dev;
    pst_pci_dev = pcie_res_to_dev(pst_pci_res);

    pos_cap_aer = oal_pci_find_ext_capability(pst_pci_dev, PCI_EXT_CAP_ID_ERR);
    if (pos_cap_aer == 0) {
        return -OAL_EFAIL;
    }

    if (oal_pci_read_config_dword(pst_pci_dev, pos_cap_aer + PCI_ERR_UNCOR_MASK, &ucor_mask)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "PCI_ERR_UNCOR_MASK: read fail");
        return -OAL_EFAIL;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "PCI_ERR_UNCOR_MASK: 0x%x", ucor_mask);

    /* bit 22, 26 unmask , vendor suggest */
    ucor_mask = 0x0;

    oal_pci_write_config_dword(pst_pci_dev, pos_cap_aer + PCI_ERR_UNCOR_MASK, ucor_mask);

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_device_mem_check_burst(oal_pcie_res *pst_pci_res, uint32_t burst_size,
                                                   uintptr_t start, unsigned long length, uint8_t test_value)
{
    uint32_t i;
    int32_t ret;
    void *pst_ddr_buf = NULL;
    uint32_t verify, value;
    unsigned long remain_size, offset, copy_size, total_size;

    pci_addr_map addr_map;

    if ((length & 0x7) || (start & 0x7)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "length %lu, cpu address 0x%lx is invalid", length, start);
        return -OAL_EFAUL;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, start + length - 1, &addr_map);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "cpu address start:0x%lx + length:0x%lx -1 invalid", start, length);
        return ret;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, start, &addr_map);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "cpu address start:0x%lx", start);
        return ret;
    }

    pst_ddr_buf = oal_memalloc(burst_size);
    if (pst_ddr_buf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc mem %u bytes failed", burst_size);
        return -OAL_ENOMEM;
    }

    memset_s(pst_ddr_buf, burst_size, test_value, burst_size);

    /* 先连续写再连续读，连续写性能最优 */
    remain_size = length;
    offset = 0;
    total_size = 0;
    verify = (test_value | (test_value << 8) | (test_value << 16) | (test_value << 24));

    forever_loop() {
        if (remain_size == 0) {
            break;
        }

        copy_size = oal_min(burst_size, remain_size);

        /* h2d write */
        oal_pcie_io_trans(addr_map.va + offset, (uintptr_t)pst_ddr_buf, copy_size);

        /* d2h read */
        oal_pcie_io_trans((uintptr_t)pst_ddr_buf, addr_map.va + offset, copy_size);

        /* memcheck */
        for (i = 0; i < copy_size; i += sizeof(uint32_t)) {
            value = *(uint32_t *)(pst_ddr_buf + i);
            if (value != verify) {
                oal_print_hi11xx_log(HI11XX_LOG_ERR,
                                     "mem check failed, cpu address : 0x%lx, \
                                     write 0x%x, read 0x%x, offset =%lu, i = %u",
                                     start + offset + i, verify, value, offset, i);
                oal_print_hex_dump((uint8_t *)pst_ddr_buf, copy_size, HEX_DUMP_GROUP_SIZE, "burst check ");
                oal_print_hex_dump((uint8_t *)(uintptr_t)(addr_map.va + offset),
                                   copy_size, HEX_DUMP_GROUP_SIZE, "bcpu check ");
                oal_free(pst_ddr_buf);
                return -OAL_EFAIL;
            }
        }

        offset += copy_size;
        total_size += copy_size;
        remain_size -= copy_size;
    }

    if (total_size != length) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "wrong total_size=%lu ,length=%lu", total_size, length);
        oal_free(pst_ddr_buf);
        return -OAL_EFAIL;
    }

    oal_free(pst_ddr_buf);
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_device_mem_check_word(oal_pcie_res *pst_pci_res,
                                                  uintptr_t start, unsigned long length, uint8_t test_value)
{
    uint32_t i;
    int32_t ret;
    uint32_t verify, value;
    declare_time_cost_stru(cost);

    pci_addr_map addr_map;

    oal_get_time_cost_start(cost);

    if ((length & 0x7) || (start & 0x7)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "length %lu, cpu address 0x%lx is invalid", length, start);
        return -OAL_EFAUL;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, start + length - 1, &addr_map);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "cpu address start:0x%lx + length:0x%lx -1 invalid", start, length);
        return ret;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, start, &addr_map);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "cpu address start:0x%lx", start);
        return ret;
    }

    verify = (test_value | (test_value << 8) | (test_value << 16) | (test_value << 24));

    for (i = 0; i < (int32_t)(uint32_t)length; i += sizeof(uint32_t)) { /* 每次偏移4字节 */
        oal_writel(verify, (void *)(addr_map.va + i));
        value = oal_readl((void *)(addr_map.va + i));
        if (oal_unlikely(verify != value)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "cpu addr:0x%lx check failed, read=0x%x write=0x%x",
                                 start + i, value, verify);
            return -OAL_EFAIL;
        }
    }

    oal_get_time_cost_end(cost);
    oal_calc_time_cost_sub(cost);
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "memcheck_byword 0x%lx length %lu, cost %llu us",
                         start, length, time_cost_var_sub(cost));

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_device_mem_check(oal_pcie_res *pst_pci_res, uintptr_t start, unsigned long length)
{
    int32_t ret;
    int32_t i;
    const uint32_t ul_burst_size = 4096;

    if (g_ft_pcie_wcpu_mem_check_burst_check) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie_wcpu_mem_check_burst_check_bypass");
    } else {
        for (i = 0; i < g_ft_pcie_wcpu_mem_check_times; i++) {
            ret = oal_pcie_device_mem_check_burst(pst_pci_res, ul_burst_size, start,
                                                  length, PCIE_MEM_SCAN_VERFIY_VAL_1);
            if (ret) {
                return ret;
            }

            ret = oal_pcie_device_mem_check_burst(pst_pci_res, ul_burst_size, start,
                                                  length, PCIE_MEM_SCAN_VERFIY_VAL_2);
            if (ret) {
                return ret;
            }

            oal_schedule();
        }
    }

    if (g_ft_pcie_wcpu_mem_check_byword_bypass) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie_wcpu_mem_check_byword_bypass");
    } else {
        for (i = 0; i < g_ft_pcie_wcpu_mem_check_times; i++) {
            ret = oal_pcie_device_mem_check_word(pst_pci_res, start, length, PCIE_MEM_SCAN_VERFIY_VAL_1);
            if (ret) {
                return ret;
            }

            ret = oal_pcie_device_mem_check_word(pst_pci_res, start, length, PCIE_MEM_SCAN_VERFIY_VAL_2);
            if (ret) {
                return ret;
            }

            oal_schedule();
        }
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "mem check address: 0x%lx , length: 0x%lx  pass!", start, length);

    return OAL_SUCC;
}

/* 一次性写入全部的地址值 */
OAL_STATIC int32_t oal_pcie_device_mem_write_address_onetime(oal_pcie_res *pst_pci_res, uintptr_t start,
                                                             unsigned long length)
{
    int32_t ret;
    int32_t i;
    unsigned long remain_size, offset, copy_size;
    void *pst_ddr_buf = NULL;

    pci_addr_map addr_map;

    if ((length & 0x7) || (start & 0x7)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "length %lu, cpu address 0x%lx is invalid", length, start);
        return -OAL_EFAUL;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, start + length - 1, &addr_map);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "cpu address start:0x%lx + length:0x%lx -1 invalid", start, length);
        return ret;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, start, &addr_map);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "cpu address start:0x%lx", start);
        return ret;
    }

    pst_ddr_buf = oal_memalloc(PAGE_SIZE);
    if (pst_ddr_buf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc mem %lu failed", PAGE_SIZE);
        return -OAL_ENOMEM;
    }

    remain_size = length;
    offset = 0;

    /* 4字节写 */
    forever_loop() {
        if (remain_size == 0) {
            break;
        }

        copy_size = oal_min(remain_size, PAGE_SIZE);

        for (i = 0; i < copy_size; i += sizeof(uint32_t)) {      /* 每次偏移4字节 */
            *(uint32_t *)(pst_ddr_buf + i) = start + offset + i; /* CPU地址 */
        }

        oal_pcie_io_trans(addr_map.va + offset, (uintptr_t)pst_ddr_buf, copy_size);

        offset += copy_size;

        remain_size -= copy_size;
    }

    oal_free(pst_ddr_buf);

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "device_mem_write_address_onetime start:0x%lx length:%lu", start, length);

    oal_schedule();

    return OAL_SUCC;
}

/* 一次性读出全部的地址值并且校验 */
OAL_STATIC int32_t oal_pcie_device_mem_read_address_onetime(oal_pcie_res *pst_pci_res, uintptr_t start,
                                                            unsigned long length)
{
    int32_t ret;
    int32_t i;
    unsigned long remain_size, offset, copy_size;
    void *pst_ddr_buf = NULL;

    pci_addr_map addr_map;

    if ((length & 0x7) || (start & 0x7)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "length %lu, cpu address 0x%lx is invalid", length, start);
        return -OAL_EFAUL;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, start + length - 1, &addr_map);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "cpu address start:0x%lx + length:0x%lx -1 invalid", start, length);
        return ret;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, start, &addr_map);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "cpu address start:0x%lx", start);
        return ret;
    }

    pst_ddr_buf = oal_memalloc(PAGE_SIZE);
    if (pst_ddr_buf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc mem %lu failed", PAGE_SIZE);
        return -OAL_ENOMEM;
    }

    remain_size = length;
    offset = 0;

    /* 4字节写 */
    forever_loop() {
        if (remain_size == 0) {
            break;
        }

        copy_size = oal_min(remain_size, PAGE_SIZE);

        oal_pcie_io_trans((uintptr_t)pst_ddr_buf, addr_map.va + offset, copy_size);

        for (i = 0; i < copy_size; i += sizeof(uint32_t)) { /* 每次偏移4字节 */
            uint32_t value = *((uint32_t *)(pst_ddr_buf + i));
            uint32_t cpu_address = start + offset + i; /* CPU地址 */
            if (oal_unlikely(value != cpu_address)) {
                oal_print_hi11xx_log(HI11XX_LOG_ERR,
                                     "mem check address verify failed, [0x%lx--0x%lx] at 0x%x, write 0x%x read 0x%x",
                                     start, start + length - 1, cpu_address, cpu_address, value);
                oal_print_hex_dump((uint8_t *)pst_ddr_buf, copy_size, HEX_DUMP_GROUP_SIZE, "address check ");
                oal_print_hex_dump((uint8_t *)(uintptr_t)(addr_map.va + offset),
                                   copy_size, HEX_DUMP_GROUP_SIZE, "pcie mem ");
                oal_free(pst_ddr_buf);
                return -OAL_EFAIL;
            }
        }

        offset += copy_size;

        remain_size -= copy_size;
    }

    oal_free(pst_ddr_buf);

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "device_mem_read_address_onetime start:0x%lx length:%lu", start, length);

    oal_schedule();

    return OAL_SUCC;
}

OAL_STATIC void oal_pcie_device_mem_write_perf(pci_addr_map *addr_map,
                                               uintptr_t    pst_buf,
                                               unsigned long length,
                                               unsigned long burst_size,
                                               uintptr_t start,
                                               unsigned long runtime)
{
    unsigned long timeout;
    unsigned long offset = 0;
    uint64_t trans_size, us_to_s;
    declare_time_cost_stru(cost);
    unsigned long remain_size, copy_size, total_size;

    if (g_ft_pcie_perf_wr_bypass) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie_perf_wr_bypass");
    } else {
        oal_get_time_cost_start(cost);

        /* 写性能, 写可以覆盖读 */
        remain_size = 0;
        total_size = 0;

        timeout = jiffies + oal_msecs_to_jiffies(runtime);
        forever_loop() {
            if (oal_time_after(jiffies, timeout)) {
                break;
            }

            if (remain_size == 0) {
                /* repeat */
                remain_size = length;
                offset = 0;
            }

            copy_size = oal_min(burst_size, remain_size);

            oal_pcie_io_trans(addr_map->va + offset, (uintptr_t)pst_buf, copy_size);

            offset += copy_size;
            total_size += copy_size;

            remain_size -= copy_size;

            cpu_relax();
        }
        oal_get_time_cost_end(cost);
        oal_calc_time_cost_sub(cost);

        us_to_s = time_cost_var_sub(cost);
        trans_size = ((total_size * 1000u * 1000u) >> PCIE_TRANS_US_OFFSET_BITS);
        trans_size = div_u64(trans_size, us_to_s);

        oal_print_hi11xx_log(HI11XX_LOG_INFO,
                             "Write(H2D) Thoughtput: %llu Mbps, trans_time:%llu us, tran_size:%lu, address:0x%lx",
                             trans_size, us_to_s, total_size, start);
    }
}

OAL_STATIC void oal_pcie_device_mem_read_perf(pci_addr_map *addr_map,
                                              uintptr_t    pst_buf,
                                              unsigned long length,
                                              unsigned long burst_size,
                                              uintptr_t start,
                                              unsigned long runtime)
{
    unsigned long timeout;
    unsigned long offset = 0;
    uint64_t trans_size, us_to_s;
    declare_time_cost_stru(cost);
    unsigned long remain_size, copy_size, total_size;

    if (g_ft_pcie_perf_rd_bypass) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie_perf_wr_bypass");
    } else {
        oal_get_time_cost_start(cost);

        /* 读性能, 写可以覆盖读 */
        remain_size = 0;
        total_size = 0;

        timeout = jiffies + oal_msecs_to_jiffies(runtime);
        forever_loop() {
            if (oal_time_after(jiffies, timeout)) {
                break;
            }

            if (remain_size == 0) {
                /* repeat */
                remain_size = length;
                offset = 0;
            }

            copy_size = oal_min(burst_size, remain_size);

            oal_pcie_io_trans((uintptr_t)pst_buf, addr_map->va + offset, copy_size);

            offset += copy_size;
            total_size += copy_size;

            remain_size -= copy_size;

            cpu_relax();
        }
        oal_get_time_cost_end(cost);
        oal_calc_time_cost_sub(cost);

        us_to_s = time_cost_var_sub(cost);
        trans_size = ((total_size * 1000u * 1000u) >> PCIE_TRANS_US_OFFSET_BITS);
        trans_size = div_u64(trans_size, us_to_s);

        oal_print_hi11xx_log(HI11XX_LOG_INFO,
                             "Read(D2H) Thoughtput: %llu Mbps, trans_time:%llu us, tran_size:%lu, address:0x%lx",
                             trans_size, us_to_s, total_size, start);
    }
}

OAL_STATIC int32_t oal_pcie_device_mem_performance(oal_pcie_res *pst_pci_res, uintptr_t start,
                                                   unsigned long length, unsigned long runtime)
{
    int32_t ret;
    unsigned long burst_size = 4096;

    pci_addr_map addr_map;

    void *pst_buf = NULL;

    if ((length & 0x7) || (start & 0x7)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "start: %lu , length %lu is invalid", start, length);
        return -OAL_EFAUL;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, start + length - 1, &addr_map);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "cpu address start:0x%lx + length:0x%lx -1 invalid", start, length);
        return ret;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, start, &addr_map);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "cpu address start:0x%lx", start);
        return ret;
    }

    pst_buf = oal_memalloc(burst_size);
    if (pst_buf == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc perf mem %lu bytes failed", burst_size);
        return -OAL_ENOMEM;
    }

    oal_pcie_device_mem_write_perf(&addr_map, (uintptr_t)pst_buf, length, burst_size, start, runtime);
    oal_pcie_device_mem_read_perf(&addr_map, (uintptr_t)pst_buf, length, burst_size, start, runtime);

    ret = oal_pcie_device_check_alive(pst_pci_res);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "performance write failed!");
        oal_free(pst_buf);
        return ret;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "mem performance done");

    oal_free(pst_buf);

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_device_get_scan_wmem_info(oal_pcie_res *pst_pci_res, uintptr_t (**pst_scan_base)[],
                                                      uint32_t *pst_scan_nums)
{
    *pst_scan_base = NULL;
    *pst_scan_nums = 0;
    if (get_hi110x_subchip_type() == BOARD_VERSION_HI1105) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "1105 %s wmem scan", hi110x_is_asic() ? "asic" : "fpga");
        if (hi110x_is_asic()) {
            *pst_scan_nums = sizeof(g_hi1105_asic_wmem_scan_array) /
                             (PCIE_MEM_MSG_SIZE * sizeof(g_hi1105_asic_wmem_scan_array[0][0]));
            *pst_scan_base = g_hi1105_asic_wmem_scan_array;
        } else {
            *pst_scan_nums = sizeof(g_hi1105_fpga_wmem_scan_array) /
                             (PCIE_MEM_MSG_SIZE * sizeof(g_hi1105_fpga_wmem_scan_array[0][0]));
            *pst_scan_base = g_hi1105_fpga_wmem_scan_array;
        }
    } else {
        if (pst_pci_res->revision == PCIE_REVISION_4_70A) {
            *pst_scan_nums = sizeof(g_mpw2_wmem_scan_array) /
                             (PCIE_MEM_MSG_SIZE * sizeof(g_mpw2_wmem_scan_array[0][0]));
            *pst_scan_base = g_mpw2_wmem_scan_array;
        } else if (pst_pci_res->revision == PCIE_REVISION_5_00A) {
            *pst_scan_nums = sizeof(g_pilot_wmem_scan_array) /
                             (PCIE_MEM_MSG_SIZE * sizeof(g_pilot_wmem_scan_array[0][0]));
            *pst_scan_base = g_pilot_wmem_scan_array;
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "unkown pcie ip revision :0x%x", pst_pci_res->revision);
            return -OAL_ENODEV;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_device_memcheck_default(oal_pcie_res *pst_pci_res)
{
    uint32_t i;
    int32_t ret;
    unsigned long cpu_start, cpu_end, mem_len;
    uint32_t scan_nums;
    uintptr_t(*pst_scan_base)[PCIE_MEM_MSG_SIZE];

    declare_time_cost_stru(cost);

    ret = oal_pcie_device_get_scan_wmem_info(pst_pci_res, &pst_scan_base, &scan_nums);
    if (ret != OAL_SUCC) {
        /* bypass */
        return OAL_SUCC;
    }

    oal_get_time_cost_start(cost);

    for (i = 0; i < scan_nums; i++) {
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "i:%u, %pK", i, pst_scan_base + i);
        cpu_start = pst_scan_base[i][0];
        cpu_end = pst_scan_base[i][1];
        if (cpu_end <= cpu_start) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "invalid mem region, start=0x%lx, end=0x%lx, i:%d",
                                 cpu_start, cpu_end, i);
            return -OAL_EFAUL;
        }

        mem_len = cpu_end - cpu_start + 1;

        ret = oal_pcie_device_mem_check(pst_pci_res, cpu_start, mem_len);
        if (ret) {
            return ret;
        }
    }

    oal_get_time_cost_end(cost);
    oal_calc_time_cost_sub(cost);
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "device wmem done cost %llu us, regions :%u",
                         time_cost_var_sub(cost), scan_nums);

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_device_memcheck_by_address(oal_pcie_res *pst_pci_res)
{
    uint32_t i;
    int32_t ret;
    unsigned long cpu_start, cpu_end, mem_len;
    uint32_t scan_nums;
    uintptr_t(*pst_scan_base)[PCIE_MEM_MSG_SIZE];

    declare_time_cost_stru(cost);

    ret = oal_pcie_device_get_scan_wmem_info(pst_pci_res, &pst_scan_base, &scan_nums);
    if (ret != OAL_SUCC) {
        /* bypass */
        return OAL_SUCC;
    }

    if (g_ft_pcie_write_address_bypass) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "wmem write_address_bypass");
    } else {
        oal_get_time_cost_start(cost);
        /* 连续写，连续读，Soc要求写入当前地址的值 */
        for (i = 0; i < scan_nums; i++) {
            cpu_start = pst_scan_base[i][0];
            cpu_end = pst_scan_base[i][1];

            mem_len = cpu_end - cpu_start + 1;

            ret = oal_pcie_device_mem_write_address_onetime(pst_pci_res, cpu_start, mem_len);
            if (ret) {
                return ret;
            }
        }

        for (i = 0; i < scan_nums; i++) {
            cpu_start = pst_scan_base[i][0];
            cpu_end = pst_scan_base[i][1];

            mem_len = cpu_end - cpu_start + 1;

            ret = oal_pcie_device_mem_read_address_onetime(pst_pci_res, cpu_start, mem_len);
            if (ret) {
                return ret;
            }
        }

        oal_get_time_cost_end(cost);
        oal_calc_time_cost_sub(cost);
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "wmem address check done cost %llu us", time_cost_var_sub(cost));
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_device_memcheck_perf(oal_pcie_res *pst_pci_res)
{
    uint32_t i;
    int32_t ret;
    unsigned long cpu_start, cpu_end, mem_len;
    uint32_t scan_nums;
    uintptr_t(*pst_scan_base)[PCIE_MEM_MSG_SIZE];

    declare_time_cost_stru(cost);

    ret = oal_pcie_device_get_scan_wmem_info(pst_pci_res, &pst_scan_base, &scan_nums);
    if (ret != OAL_SUCC) {
        /* bypass */
        return OAL_SUCC;
    }

    if (g_ft_pcie_perf_wr_bypass) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie_perf_wr_bypass");
    } else {
        oal_get_time_cost_start(cost);
        for (i = 0; i < scan_nums; i++) {
            cpu_start = pst_scan_base[i][0];
            cpu_end = pst_scan_base[i][1];
            if (cpu_end <= cpu_start) {
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "invalid mem region, start=0x%lx, end=0x%lx, i:%d",
                                     cpu_start, cpu_end, i);
                return -OAL_EFAUL;
            }

            mem_len = cpu_end - cpu_start + 1;

            ret = oal_pcie_device_mem_performance(pst_pci_res, cpu_start, mem_len, g_ft_pcie_perf_runtime);
            if (ret) {
                return ret;
            }
        }
        oal_get_time_cost_end(cost);
        oal_calc_time_cost_sub(cost);
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "mem performance done cost %llu us", time_cost_var_sub(cost));
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_device_scan_wmem(oal_pcie_res *pst_pci_res)
{
    int32_t ret;

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "start scan wmem..");

    ret = oal_pcie_device_memcheck_default(pst_pci_res);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = oal_pcie_device_memcheck_by_address(pst_pci_res);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = oal_pcie_device_memcheck_perf(pst_pci_res);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_device_dereset_bcpu(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    pci_addr_map addr_map;
    void *pst_glb_ctrl = NULL; /* 0x50000000 */

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, HI1103_PA_GLB_CTL_BASE_ADDR, &addr_map);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "can not found mem map for dev cpu address 0x%x\n",
                             HI1103_PA_GLB_CTL_BASE_ADDR);
        return -OAL_EFAIL;
    }

    pst_glb_ctrl = (void *)addr_map.va;

    oal_writel(0x1, pst_glb_ctrl + 0x94);

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "bcpu dereset, reg=0x%x", oal_readl(pst_glb_ctrl + 0x94));

    /* bcpu mem解复位需要时间, 1103 32K计数, 230us */
    oal_msleep(1);

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_device_get_scan_bmem_info(oal_pcie_res *pst_pci_res, uintptr_t (**pst_scan_base)[],
                                                      uint32_t *pst_scan_nums)
{
    *pst_scan_base = NULL;
    *pst_scan_nums = 0;
    if (get_hi110x_subchip_type() == BOARD_VERSION_HI1105) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "1105 %s bmem scan", hi110x_is_asic() ? "asic" : "fpga");
        if (hi110x_is_asic()) {
            *pst_scan_nums = sizeof(g_hi1105_asic_bmem_scan_array) /
                             (PCIE_MEM_MSG_SIZE * sizeof(g_hi1105_asic_bmem_scan_array[0][0]));
            *pst_scan_base = g_hi1105_asic_bmem_scan_array;
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "skip 1105 fpga bmem scan");
            return -OAL_ENOMEM;
        }
    } else {
        if (pst_pci_res->revision == PCIE_REVISION_4_70A) {
            *pst_scan_nums = sizeof(g_mpw2_bmem_scan_array) /
                             (PCIE_MEM_MSG_SIZE * sizeof(g_mpw2_bmem_scan_array[0][0]));
            *pst_scan_base = g_mpw2_bmem_scan_array;
        } else if (pst_pci_res->revision == PCIE_REVISION_5_00A) {
            *pst_scan_nums = sizeof(g_pilot_bmem_scan_array) /
                             (PCIE_MEM_MSG_SIZE * sizeof(g_pilot_bmem_scan_array[0][0]));
            *pst_scan_base = g_pilot_bmem_scan_array;
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "unkown pcie ip revision :0x%x", pst_pci_res->revision);
            return -OAL_ENODEV;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_device_scan_bmem(oal_pcie_res *pst_pci_res)
{
    uint32_t i;
    int32_t ret;
    unsigned long cpu_start, cpu_end, mem_len;
    uint32_t scan_nums;
    uintptr_t(*pst_scan_base)[PCIE_MEM_MSG_SIZE];

    declare_time_cost_stru(cost);

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "start scan bmem..");

    ret = oal_pcie_device_get_scan_bmem_info(pst_pci_res, &pst_scan_base, &scan_nums);
    if (ret != OAL_SUCC) {
        /* bypass */
        return OAL_SUCC;
    }

    oal_get_time_cost_start(cost);

    ret = oal_pcie_device_dereset_bcpu(pst_pci_res);
    if (ret != OAL_SUCC) {
        return ret;
    }

    for (i = 0; i < scan_nums; i++) {
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "i:%u, %pK", i, pst_scan_base + i);
        cpu_start = pst_scan_base[i][0];
        cpu_end = pst_scan_base[i][1];
        if (cpu_end <= cpu_start) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "invalid mem region, start=0x%lx, end=0x%lx, i:%d",
                                 cpu_start, cpu_end, i);
            return -OAL_EFAUL;
        }

        mem_len = cpu_end - cpu_start + 1;

        ret = oal_pcie_device_mem_check(pst_pci_res, cpu_start, mem_len);
        if (ret) {
            return ret;
        }
    }

    if (g_ft_pcie_write_address_bypass) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "bmem write_address_bypass");
    } else {
        /* 连续写，连续读，Soc要求写入当前地址的值 */
        for (i = 0; i < scan_nums; i++) {
            cpu_start = pst_scan_base[i][0];
            cpu_end = pst_scan_base[i][1];

            mem_len = cpu_end - cpu_start + 1;

            ret = oal_pcie_device_mem_write_address_onetime(pst_pci_res, cpu_start, mem_len);
            if (ret) {
                return ret;
            }
        }

        for (i = 0; i < scan_nums; i++) {
            cpu_start = pst_scan_base[i][0];
            cpu_end = pst_scan_base[i][1];

            mem_len = cpu_end - cpu_start + 1;

            ret = oal_pcie_device_mem_read_address_onetime(pst_pci_res, cpu_start, mem_len);
            if (ret) {
                return ret;
            }
        }
    }

    oal_get_time_cost_end(cost);
    oal_calc_time_cost_sub(cost);
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "device bmem done cost %llu us, regions :%u",
                         time_cost_var_sub(cost), scan_nums);

    return OAL_SUCC;
}

/* scan the mem */
int32_t oal_pcie_device_mem_scanall(oal_pcie_res *pst_pci_res)
{
    int32_t ret;

    ret = oal_pcie_device_scan_reg(pst_pci_res);
    if (ret) {
        return ret;
    }

    ret = oal_pcie_device_scan_wmem(pst_pci_res);
    if (ret) {
        return ret;
    }

    ret = oal_pcie_device_scan_bmem(pst_pci_res);
    if (ret) {
        return ret;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "device_mem_scanall pass");
    return OAL_SUCC;
}

int32_t oal_pcie_copy_from_device_by_dword(oal_pcie_res *pst_pci_res,
                                           void *ddr_address,
                                           unsigned long start,
                                           uint32_t data_size)
{
    uint32_t i;
    int32_t ret;
    uint32_t value;
    unsigned long length;
    pci_addr_map addr_map;

    length = (unsigned long)data_size;

    if (oal_unlikely(((uintptr_t)ddr_address & 0x3) || (data_size & 0x3))) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "ddr address %lu, length 0x%lx is invalid",
                             (uintptr_t)ddr_address, length);
        return -OAL_EINVAL;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, start + length - 1, &addr_map);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "cpu address start:0x%lx + length:0x%lx -1 invalid", start, length);
        return ret;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, start, &addr_map);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "cpu address start:0x%lx", start);
        return ret;
    }

    for (i = 0; i < (uint32_t)length; i += sizeof(uint32_t)) { /* 每次偏移4字节 */
        value = oal_readl((void *)(addr_map.va + i));
        oal_writel(value, ddr_address + i);
    }

    return (int32_t)data_size;
}

int32_t oal_pcie_ip_l1pm_check(oal_pcie_linux_res *pst_pci_lres)
{
    oal_pcie_print_chip_info(pst_pci_lres, OAL_TRUE);

    oal_msleep(20); /* wait pcie enter L1.2 */

    if (g_ft_pcie_aspm_check_bypass) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "l1pm_check bypass");
        return OAL_SUCC;
    } else {
#ifdef CONFIG_ARCH_KIRIN_PCIE
#if defined(CONFIG_KIRIN_PCIE_HI3660)
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie960 leacy");
        return -OAL_ENODEV;

#elif defined(CONFIG_KIRIN_PCIE_KIRIN970)
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie970 not implemented");
        return -OAL_ENODEV;
#else
        uint32_t value = show_link_state(g_kirin_rc_idx);
        if (value == 0xC000) {
            return OAL_SUCC;
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "L1.2 check failed");
            return -OAL_EFAIL;
        }
#endif
#else
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "to be implemented");
        return -OAL_ENODEV;
#endif
    }
}

int32_t oal_pcie_gen_mode_check(oal_pcie_linux_res *pst_pci_lres)
{
    /* 根据实际产品来判断当前链路状态是否正常 */
    int32_t gen_select = oal_pcie_get_gen_mode(pst_pci_lres->pst_pci_res);

    if (g_ft_pcie_gen_check_bypass) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie_gen_check_bypass");
    } else {
        if (g_hipci_gen_select != gen_select) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "expect link mode is %d, but current is %d",
                                 g_hipci_gen_select, gen_select);
            return -OAL_EFAIL;
        }
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "current link is %s", (gen_select == PCIE_GEN2) ? "GEN2" : "GEN1");

    return OAL_SUCC;
}

static int32_t oal_pcie_check_device_link_errors(oal_pcie_res *pst_pci_res)
{
    pcie_msg_intr_status msg_intr_status, msg_mask;

    msg_mask.as_dword = 0x0;
    msg_intr_status.bits.soc_pcie_send_nf_err_status = 1;
    msg_intr_status.bits.soc_pcie_send_f_err_status = 1;
    msg_intr_status.bits.soc_pcie_send_cor_err_status = 1;

    msg_intr_status.as_dword = oal_readl(pst_pci_res->pst_pci_ctrl_base + PCIE_MSG_INTR_STATUS_OFF);
    msg_intr_status.as_dword &= msg_intr_status.as_dword;

    if (msg_intr_status.bits.soc_pcie_send_f_err_status) {
        /* 链路信号极差 */
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "f_err found, intr_status=0x%8x", msg_intr_status.as_dword);
        oal_pcie_print_device_aer_cap_reg(pst_pci_res);
        /* Clear the int */
        oal_writel(msg_intr_status.as_dword, (pst_pci_res->pst_pci_ctrl_base + PCIE_MSG_INTR_CLR_OFF));
        declare_dft_trace_key_info("soc_pcie_send_f_err", OAL_DFT_TRACE_SUCC);
        if (g_ft_pcie_link_err_bypass) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie_link_err_bypass");
            return OAL_SUCC;
        } else {
            return -OAL_EFAIL;
        }
    }

    if (msg_intr_status.bits.soc_pcie_send_nf_err_status) {
        /* 链路信号差 */
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "nf_err found, intr_status=0x%8x", msg_intr_status.as_dword);
        oal_pcie_print_device_aer_cap_reg(pst_pci_res);
        /* Clear the int */
        oal_writel(msg_intr_status.as_dword, (pst_pci_res->pst_pci_ctrl_base + PCIE_MSG_INTR_CLR_OFF));
        declare_dft_trace_key_info("soc_pcie_send_nf_err", OAL_DFT_TRACE_SUCC);
        if (g_ft_pcie_link_err_bypass) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie_link_err_bypass");
            return OAL_SUCC;
        } else {
            return -OAL_EFAIL;
        }
    }

    if (msg_intr_status.bits.soc_pcie_send_cor_err_status) {
        /* 可忽略的错误 */
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "cor_err found, intr_status=0x%8x", msg_intr_status.as_dword);
        oal_pcie_print_device_aer_cap_reg(pst_pci_res);
        declare_dft_trace_key_info("soc_pcie_send_cor_err", OAL_DFT_TRACE_SUCC);
    }

    /* Clear the int */
    oal_writel(msg_intr_status.as_dword, (pst_pci_res->pst_pci_ctrl_base + PCIE_MSG_INTR_CLR_OFF));

    /* pass */
    return OAL_SUCC;
}

static int32_t oal_pcie_ip_factory_test_init(hcc_bus *pst_bus, oal_pcie_linux_res *pst_pci_lres)
{
    int32_t ret = oal_pcie_ip_init(pst_bus);

    oal_msleep(5); /* 5ms wait pcie enter L1.2 */

    ret = oal_pcie_ip_l1pm_check(pst_pci_lres);
    if (ret) {
        return ret;
    }

    ret = oal_pcie_voltage_bias_init(pst_pci_lres->pst_pci_res);
    if (ret) {
        return ret;
    }

    ret = oal_pcie_device_changeto_high_cpufreq(pst_pci_lres->pst_pci_res);
    if (ret) {
        return ret;
    }

    return OAL_SUCC;
}

int32_t oal_pcie_ip_factory_test_hi1103(hcc_bus *pst_bus, int32_t test_count)
{
    int32_t i;
    int32_t ret;
    oal_pcie_linux_res *pst_pci_lres;

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_ENODEV;
    }

    ret = oal_pcie_ip_factory_test_init(pst_bus, pst_pci_lres);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = oal_pcie_gen_mode_check(pst_pci_lres);
    if (ret) {
        return ret;
    }

    ret = oal_pcie_unmask_device_link_erros(pst_pci_lres->pst_pci_res);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "unmask device link err failed = %d", ret);
        return ret;
    }

    for (i = 0; i < test_count; i++) {
        /* memcheck */
        ret = oal_pcie_device_mem_scanall(pst_pci_lres->pst_pci_res);
        if (ret) {
            return ret;
        }

        oal_print_hi11xx_log(HI11XX_LOG_INFO, "scan all mem done , test %d times", i + 1);

        ret = oal_pcie_check_device_link_errors(pst_pci_lres->pst_pci_res);
        if (ret) {
            return ret;
        }

        ret = oal_pcie_gen_mode_check(pst_pci_lres);
        if (ret) {
            return ret;
        }
    }

    oal_msleep(5); /* wait pcie enter L1.2 */

    ret = oal_pcie_ip_l1pm_check(pst_pci_lres);
    if (ret) {
        return ret;
    }

    return OAL_SUCC;
}

int32_t oal_pcie_chiptest_init_hi1103(oal_pcie_res *pst_pci_res, int32_t device_id)
{
    pst_pci_res->chip_info.cb.pcie_ip_factory_test = oal_pcie_ip_factory_test_hi1103;
    return OAL_SUCC;
}

#endif
