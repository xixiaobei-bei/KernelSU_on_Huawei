#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/pm_runtime.h>
#include <linux/wakelock.h>

#include "soc_ics_interface.h"
#include "soc_media2_crg_interface.h"
#include "soc_media1_crg_interface.h"
#include "soc_pmctrl_interface.h"
#include "soc_pctrl_interface.h"
#include "soc_crgperiph_interface.h"
#include "soc_acpu_baseaddr_interface.h"
#include "soc_dmss_interface.h"

#include "npu_public.h"
#include "npu_main.h"
#include "npu_task.h"
#include "npu_power.h"
#include "npu_clock.h"
#include "npu_smmu.h"
#include "npu_mntn.h"

#define NPU_DRV_NAME "hisilicon,kirin-npu"   /*dtsi need modify*/

#define NPU_DCQ_DMSS_MSTS (0x9)
#define NPU_TO_START      (0x1)
#define NPU_START_REG     (0x18)
#define NPU_STATUS_REG    (0x20)
#define NPU_BASE_ADDR_REG (0x28)
#define NPU_WAIT_THRESHOLD_US (500)

#define NPU_SUSPEND_DELAY  (20) /* set latency(1ms) before PowerDown */

#define MAGIC_NUM (100)
#define IN_TASKQUEUE               _IOW(MAGIC_NUM, 14, unsigned int[11])
#define GETCONFIG_CHIP_TYPE        _IOR(MAGIC_NUM, 15, unsigned int[2])
#define SETCONFIG_RESET_STATISTIC  _IOW(MAGIC_NUM, 30, unsigned int)
#define SETCONFIG_REPORT_STATISTIC _IOW(MAGIC_NUM, 31, SMMU_STAT_S)
#define SETCONFIG_UPDATE_PTE       _IOW(MAGIC_NUM, 32, unsigned int)
#define SETCONFIG_RESET_VIRT_ADDR  _IOW(MAGIC_NUM, 33, struct map_data)
#define SETCONFIG_NPU_PROFILE      _IOW(MAGIC_NUM, 34, int[2])
#define SET_SECURE_MODE            _IOR(MAGIC_NUM, 35, int[2])
#define RELEASE_SECURE_MODE        _IOR(MAGIC_NUM, 36, int[2])
#define SETCONFIG_MAP              _IOW(MAGIC_NUM, 37, struct map_data)
#define SETCONFIG_UNMAP            _IOW(MAGIC_NUM, 38, struct map_data)
#define NPU_IOC_MAXNR   (64)

typedef long (*ioctl_cb)(unsigned long);

/* IOCTL map */
struct npu_ioctl_map {
    unsigned int cmd;
    ioctl_cb func;
};

int hard_version = 0;
NPU_MAIN_PRIVATE_S *adapter = NULL;
NPU_PM_RUNTIME_S *adapter_pm_core0 = NULL;
NPU_PM_RUNTIME_S *adapter_pm_core1 = NULL;

/* open performance monitor */
void performance_monitor_open(unsigned int coreid)
{
    unsigned long irq_io_addr = 0;

    if (!adapter->common.feature.performance_monitor) {
        NPU_ERR("unsupport op in this platform!\n");
        return;
    }

    if (coreid > adapter->common.feature.core_num - 1) {
        NPU_ERR("coreid [%d] err\n", coreid);
        return;
    }

    irq_io_addr = (unsigned long)adapter->core[coreid].npu_irq.io_addr;

    /* PERF_CNT_CLK_GT.perf_cnt_clk_gt = 0x1, to OPEN performance monitor */
    iowrite32(PERF_CNT_CLK_GT_ENABLE, (void *)SOC_ICS_PERF_CNT_CLK_GT_ADDR(irq_io_addr));


    /* PERF_CNT_CLEAR.perf_cnt_clear = 0x1, FRAME_CNT_CLEAR = 0x11, CLEAR old values */
    iowrite32(PERF_CNT_CLEAR, (void *)SOC_ICS_PERF_CNT_CLEAR_ADDR(irq_io_addr));
    iowrite32(FRAME_START_CNT_CLEAR | FRAME_FINISH_CNT_CLEAR, (void *)SOC_ICS_FRAME_CNT_CLEAR_ADDR(irq_io_addr));

    return;
}

/*stat*/
static void performance_monitor_get_stat(unsigned int coreid)
{
    unsigned int frame_cycle_cnt = 0;
    unsigned int fu_idle_cnt = 0;
    unsigned int io_idle_cnt = 0;
    unsigned int all_idle_cnt = 0;
    unsigned int all_busy_cnt = 0;
    unsigned int frame_cnt = 0;
    unsigned int vcodecbus_clk = 0;
    unsigned long irq_io_addr = (unsigned long)adapter->core[coreid].npu_irq.io_addr;

    if (!adapter->common.feature.performance_monitor) {
        NPU_ERR("unsupport op in this platform!\n");
        return;
    }

    /* after ICS finish, record value of FRAME_CYC_CNT, FU_IDLE_CNT, IO_IDLE_CNT, ALL_IDLE_CNT, ALL_BUSY_CNT, ICS_FRAME_CNT  */
    frame_cycle_cnt = ioread32((void *)SOC_ICS_FRAME_CYC_CNT_ADDR(irq_io_addr));
    fu_idle_cnt     = ioread32((void *)SOC_ICS_FU_IDLE_CNT_ADDR(irq_io_addr));
    io_idle_cnt     = ioread32((void *)SOC_ICS_IO_IDLE_CNT_ADDR(irq_io_addr));
    all_idle_cnt    = ioread32((void *)SOC_ICS_ALL_IDLE_CNT_ADDR(irq_io_addr));
    all_busy_cnt    = ioread32((void *)SOC_ICS_ALL_BUSY_CNT_ADDR(irq_io_addr));
    frame_cnt       = ioread32((void *)SOC_ICS_FRAME_CNT_ADDR(irq_io_addr));

    // todo: here you can config PERF_CNT_CLEAR.perf_cnt_clear = 0x1 if want to clear current value

    /* get vcodecbus_clk*/
    vcodecbus_clk = npu_get_vcodecbus_rate();
    if (0 == vcodecbus_clk) {
        NPU_ERR("vcodecbus_clk=0\n");
        return;
    }

    vcodecbus_clk = vcodecbus_clk / NPU_RATE_HZ_TO_MHZ; /* convert vcodecbus clk working rate from bps to Mbps */

    if (0 != vcodecbus_clk) {
        NPU_PRINT("coreid: %d fu/io/all idle: %d/%d/%d | all_busy: %d | frame_cnt: %d; hw time: %d @ %dM",
            coreid, fu_idle_cnt, io_idle_cnt, all_idle_cnt, all_busy_cnt, frame_cnt & 0xffff,
            frame_cycle_cnt / vcodecbus_clk, vcodecbus_clk);
    } else {
        NPU_PRINT("coreid: %d frame_cycle: %d | fu/io/all idle: %d/%d/%d | all_busy: %d | ics_frame_cnt: %d",
            coreid, frame_cycle_cnt, fu_idle_cnt, io_idle_cnt, all_idle_cnt, all_busy_cnt, frame_cnt & 0xffff);
    }

    return;
}

/*close performance monitor*/
static void performance_monitor_close(unsigned int coreid)
{
    unsigned long irq_io_addr = (unsigned long)adapter->core[coreid].npu_irq.io_addr;

    if (!adapter->common.feature.performance_monitor) {
        NPU_ERR("unsupport op in this platform!\n");
        return;
    }

	performance_monitor_get_stat(coreid);

    /* PERF_CNT_CLEAR.perf_cnt_clear = 0x1, FRAME_CNT_CLEAR = 0x11, to clear current value */
    iowrite32(PERF_CNT_CLEAR, (void *)SOC_ICS_PERF_CNT_CLEAR_ADDR(irq_io_addr));
    iowrite32(FRAME_START_CNT_CLEAR | FRAME_FINISH_CNT_CLEAR, (void *)SOC_ICS_FRAME_CNT_CLEAR_ADDR(irq_io_addr));

    /* PERF_CNT_CLK_GT.perf_cnt_clk_gt = 0x0, to CLOSE performance monitor */
    iowrite32(PERF_CNT_CLK_GT_CLOSE, (void *)SOC_ICS_PERF_CNT_CLK_GT_ADDR(irq_io_addr));
    return;
}

/*interrupt init*/
static void npu_interrupt_init(unsigned int coreid)
{
    unsigned long irq_io_addr = (unsigned long)adapter->core[coreid].npu_irq.io_addr;

    /* clear npu status to unfinished */
    npu_task_finish_interrupt_clear(coreid);

    /* clear ns interrupt */
    iowrite32(NPU_IRQ_CLEAR_IRQ_NS, (void *)SOC_ICS_IRQ_CLR_NS_ADDR(irq_io_addr));

    /* unmask interrupt */
    iowrite32(NPU_IRQ_UNMASK_NO_SECURITY, (void *)SOC_ICS_IRQ_MASK_NS_ADDR(irq_io_addr));

    return;
}

/* to mask npu interrupt and will not receive it */
static void npu_interrupt_deinit(unsigned int coreid)
{
    unsigned long irq_io_addr = (unsigned long)adapter->core[coreid].npu_irq.io_addr;

    /* clear npu status to unfinished */
    npu_task_finish_interrupt_clear(coreid);

    /* clear ns interrupt */
    iowrite32(NPU_IRQ_CLEAR_IRQ_NS, (void *)SOC_ICS_IRQ_CLR_NS_ADDR(irq_io_addr));

    /* mask interrupt */
    iowrite32(NPU_IRQ_MASK_NO_SECURITY, (void *)SOC_ICS_IRQ_MASK_NS_ADDR(irq_io_addr));

    return;
}

/*irq handle*/
static void npu_core_irq_handler_func(unsigned int coreid)
{
    unsigned long irq_io_addr = (unsigned long)adapter->core[coreid].npu_irq.io_addr;

    if (adapter->common.feature.level1_irq) {
        npu_core_irq_handler(coreid, irq_io_addr);
    } else {
    	 /* clear npu finished status */
    	npu_task_finish_interrupt_clear(coreid);
        iowrite32(NPU_IRQ_CLEAR_IRQ_NS, (void *)SOC_ICS_IRQ_CLR_NS_ADDR(irq_io_addr));
    }

    return;
}

/* npu char device ops function implementation, inode:node of file, filp: pointer of file */
static int npu_open(struct inode *inode, struct file *filp)
{
    NPU_MAIN_PRIVATE_COMMON_S *common_dev;
    int coreid = 0;

    if (!inode || !filp){
        NPU_ERR("invalid input parameter !\n");
        return -EINVAL;
    }

    if (!adapter) {
        NPU_ERR("FATAL: adapter is NULL\n");
        return -EFAULT;
    }

    for (coreid = 0; coreid < (int)(adapter->common.feature.core_num); coreid++) {
        CTRL_LOCK(coreid);  //lint !e454
    }

    if (adapter->common.npu_opened) {

        NPU_ERR("NPU device has already opened !\n");
        for (coreid = (int)(adapter->common.feature.core_num - 1); coreid >= 0; coreid--) {
            CTRL_UNLOCK(coreid); //lint !e455
        }

        return -EBUSY; /*lint !e454*/
    }

    /* find offset of "cdev" in "struct NPU_MAIN_PRIVATE_S" */
    common_dev = container_of(inode->i_cdev, NPU_MAIN_PRIVATE_COMMON_S, cdev);/*lint !e826*/

    /* save dev info to "filp->private_data", it will be used later */
    filp->private_data = common_dev;

    npu_smmu_set_stat_en(0);
    npu_set_default_profile();

    adapter->common.npu_opened = true;

    for (coreid = (int)(adapter->common.feature.core_num - 1); coreid >= 0; coreid--) {
        CTRL_UNLOCK(coreid); //lint !e455
    }

    NPU_INFO("NPU device open success!\n");

    return 0; /*lint !e454*/
}

/* npu device release and power down */
static int npu_release(struct inode *inode, struct file *filp)
{
    int coreid = 0;
    NPU_MAIN_PRIVATE_COMMON_S *common_dev;

    if ((!inode) || (!filp)) {
        NPU_ERR("input parameter inode or filp is invalid !\n");
        return -EINVAL;
    }

    if (!adapter) {
        NPU_ERR("FATAL: adapter is NULL\n");
        return -EFAULT;
    }

    common_dev = filp->private_data;

    if (&adapter->common != common_dev) {
        NPU_ERR("No NPU device! input dev is %pK\n", (void*)common_dev);
        return -EINVAL;
    }

    for (coreid = 0; coreid < (int)(adapter->common.feature.core_num); coreid++) {
        CTRL_LOCK(coreid); //lint !e454
    }

    if (!adapter->common.npu_opened) {

        NPU_ERR("npu device is already closed\n");
        for (coreid = (int)(adapter->common.feature.core_num - 1); coreid >= 0; coreid--) {
            CTRL_UNLOCK(coreid); //lint !e455
        }

        return -EBUSY; /*lint !e454*/
    }

    adapter->common.npu_opened = false;
    if (false == npu_power_status(0) && false == npu_power_status(1)) {
        npu_mem_mngr_exit();
    } else {
        NPU_INFO("npu is still working, core0=%d, core1=%d\n", npu_power_status(0), npu_power_status(1));
	}

    for (coreid = (int)(adapter->common.feature.core_num - 1); coreid >= 0; coreid--) {
        CTRL_UNLOCK(coreid); //lint !e455
    }

    NPU_INFO("NPU device release success!\n");

    return 0;  /*lint !e454*/
}

/* task for callback*/
int npu_work_get(unsigned int coreid)
{
    static unsigned int npu_work_get_cnt0 = 0;
    static unsigned int npu_work_get_cnt1 = 0;

    if (coreid > adapter->common.feature.core_num - 1) {
        NPU_ERR("coreid [%d] err\n", coreid);
        return -EINVAL;
    }

    if (0 == coreid) {
        pm_runtime_get_sync(adapter_pm_core0->dev);
        npu_work_get_cnt0++;
        NPU_DEBUG("npu_work_get_cnt0=%d,coreid=%d", npu_work_get_cnt0, coreid);
    } else {
        pm_runtime_get_sync(adapter_pm_core1->dev);
        npu_work_get_cnt1++;
        NPU_DEBUG("npu_work_get_cnt1=%d, coreid=%d", npu_work_get_cnt1, coreid);
    }

    return 0;
}

/* task for callback*/
int npu_work_put(unsigned int coreid)
{
    static unsigned int npu_work_put_cnt0 = 0;
    static unsigned int npu_work_put_cnt1 = 0;

    if (coreid > adapter->common.feature.core_num - 1) {
        NPU_ERR("coreid [%d] err\n", coreid);
        return -EINVAL;
    }

    if (0 == coreid) {
        pm_runtime_put_sync(adapter_pm_core0->dev);
        npu_work_put_cnt0++;
        NPU_DEBUG("npu_work_put_cnt0=%d,coreid=%d", npu_work_put_cnt0, coreid);
    } else {
        pm_runtime_put_sync(adapter_pm_core1->dev);
        npu_work_put_cnt1++;
        NPU_DEBUG("npu_work_put_cnt1=%d,coreid=%d", npu_work_put_cnt1, coreid);
    }

    return 0;
}

static bool npu_chip_can_direct_access(unsigned int coreid)
{
    if (NON_SECURETY == adapter->core[coreid].security_mode) {
        return true;
    }

    return false;
}

/* CPU write inst to NPU SRAM */
static ssize_t npu_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    return count;
}

/*get chip num*/
static long npu_get_chip_type(unsigned long arg)
{
    int param[2];

    if (!arg) {
        NPU_ERR("input parameter arg is NULL, FATAL arg and ignore\n");
        return -EINVAL;
    }

    memset(param, 0x0, sizeof(param));

    param[0] = hard_version;
    param[1] = PLATFORM_VERSION;

    NPU_DEBUG("param[0][0x%x], param[1][0x%x]", param[0], param[1]);

    if (copy_to_user((void __user *)arg, param, sizeof(param))) {
        NPU_ERR("copy_to_user failed!\n");
        return -EFAULT;
    }

    return 0;
}

void npu_common_lock(unsigned int coreid)
{
    if (coreid > adapter->common.feature.core_num - 1) {
        NPU_ERR("param err coreid[%d]!", coreid);
        return;
    }

    COMMON_LOCK(coreid); //lint !e454

    return; /*lint !e454*/
}

void npu_common_unlock(unsigned int coreid)
{
    if (coreid > adapter->common.feature.core_num - 1) {
        NPU_ERR("param err coreid[%d]!", coreid);
        return;
    }

    COMMON_UNLOCK(coreid); //lint !e455
    return;
}

/*this func is special common lock put task module*/
static long npu_process_workqueue(unsigned long arg)
{
    int ret = 0;
    unsigned int coreid;
    unsigned int param[11] = {0};

    if (!arg) {
        NPU_ERR("Input parameter arg is NULL, FATAL arg and ignore!");
        return -EINVAL;
    }

    if (!adapter) {
        NPU_ERR("FATAL: adapter is NULL\n");
        return -EFAULT;
    }

    ret = copy_from_user(param, (void __user *)arg, sizeof(param));
    if (ret) {
        NPU_ERR("Copy_from_user failed!");
        return -EINVAL;
    }

    coreid = param[0];

    if (coreid > adapter->common.feature.core_num - 1) {
        NPU_ERR("param err coreid[%d]!", coreid);
        return -EFAULT;
    }

    /*this func is special common lock put task module*/
    ret = npu_push_task((void *)param, adapter->core[coreid].irq);
    if (ret) {
        NPU_ERR("Call npu_push_task failed! ret=%d\n", ret);
        return ret;
    }

    return 0;
}

/*soft reset*/
static void npu_soft_reset(unsigned int coreid)
{
    int loop = 0;
    unsigned long irq_io_addr = (unsigned long)adapter->core[coreid].npu_irq.io_addr;

    /*config ICS_SOFT_RST_REQ  = 0x1*/
    iowrite32(1, (void *)SOC_ICS_SOFT_RST_REQ_ADDR(irq_io_addr));

    /* loop wait ICS_SOFT_RST_ACK == 0x1 */
    for (loop = 0; loop < NPU_WAIT_THRESHOLD_US; loop++) {
        if (SOFT_RST_ACK == ioread32((void *)SOC_ICS_SOFT_RST_ACK_ADDR(irq_io_addr))) {
            break;
        }

        udelay(1);
    }

	if (NPU_WAIT_THRESHOLD_US == loop) {
		NPU_ERR("FATAL error: no response of SOFT reset\n");
	} else {
		NPU_DEBUG("loop wait ICS_SOFT_RST_ACK ok\n");
	}

    return;
}

/*reset func*/
static int npu_reset_proc(unsigned int coreid)
{
    int ret;
    struct timeval tv1, tv2;

    NPU_MAIN_PRIVATE_COMMON_S* p_common = &adapter->common;

    if (coreid > adapter->common.feature.core_num - 1) {
        NPU_ERR("param err coreid[%d]!", coreid);
        return -EINVAL;
    }

    if ( (p_common->feature.npu_reset_when_in_error != RESET_BY_CONFIG_NOC_BUS)
        && (p_common->feature.npu_reset_when_in_error != SOFT_RESET) ) {
        NPU_ERR("unsupported npu reset trategy=%u!\n",
                p_common->feature.npu_reset_when_in_error);
        return -EINVAL;
    }

    do_gettimeofday(&tv1);

    /*1. soft reset*/
    if (SOFT_RESET == p_common->feature.npu_reset_when_in_error) {

        npu_soft_reset(coreid);

        NPU_DEBUG("core[%d] wait SOFT_RST_ACK ok\n", coreid);
    } else {

        /* config NOC register, to enter stream limited mode, stream is limited to 0; SOC suggest here use NOC-BUS trategy for bandwidth lmt */
        npu_bandwidth_lmt_reset(coreid);
        npu_smmu_override_prefetch_addr(coreid);
        udelay(10);
    }

    /* 4. regulator disable then regulator enable */
    npu_power_off(coreid);
    ret = npu_power_on(coreid);
    if (ret) {
        NPU_ERR("FATAL error, npu power on fail");
        return ret;
    }

    /* smmu init*/
    npu_smmu_init(coreid);

    /*config bandwidth*/
    npu_bandwidth_lmt_config(coreid, (void*)adapter->core[coreid].npu_irq.io_addr, npu_get_npu_rate(coreid), npu_get_vcodecbus_rate());

    /* interrupt init*/
    npu_interrupt_init(coreid);

    do_gettimeofday(&tv2);

    NPU_INFO("npu reset time elapse is %ld usec\n",
            (tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec));

    return 0;
}

static long npu_set_secure_mode(unsigned long arg)
{
    int param[2];
    NPU_PM_RUNTIME_S pm_runtime;
    unsigned int coreid = 0;
    int profile = 0;

    memset(param, 0x0, sizeof(param));

    if (!arg) {
        NPU_ERR("input parameter arg is NULL, FATAL arg and ignore\n");
        return -EINVAL;
    }

    if (!adapter) {
        NPU_ERR("FATAL: adapter is NULL\n");
        return -EFAULT;
    }

    if (copy_from_user(param, (void __user *)arg, sizeof(param))) {
        NPU_ERR("copy arg failed!\n");
        return -EFAULT;
    }

    coreid = param[0];
    profile = param[1];

    if (coreid > adapter->common.feature.core_num - 1) {
        NPU_ERR("coreid[%d] err\n", coreid);
        return -EFAULT;
    }

    if (profile > DEV_HIGH_PROFILE) {
        NPU_ERR("profile[%u] err\n", profile);
        return -EFAULT;
    }

    memset(&pm_runtime, 0x0, sizeof(NPU_PM_RUNTIME_S));
    if (0 == coreid) {
        pm_runtime.dev = adapter_pm_core0->dev;
    } else {
        pm_runtime.dev = adapter_pm_core1->dev;
    }

    disable_irq(adapter->core[coreid].irq);

    /*1. get mode lock*/
    MODE_LOCK(coreid); //lint !e454
    CTRL_LOCK(coreid); //lint !e454

    if (pm_runtime_active(pm_runtime.dev)) {
        npu_work_get(coreid);
        npu_reset_proc(coreid);
        npu_work_put(coreid);
    }

    adapter->core[coreid].security_mode = SECURETY;

    npu_work_get(coreid);

    npu_interrupt_deinit(coreid);

    npu_smmu_exit(coreid);

    /* add param set secure clk profile*/
    npu_set_profile(coreid, profile);

    npu_update_profile(coreid);

    CTRL_UNLOCK(coreid);  //lint !e455

    return 0; /*lint !e454*/
}

static long npu_release_secure_mode(unsigned long arg)
{
    int param[2];

    unsigned int coreid = 0;
    int profile = 0;

    memset(param, 0x0, sizeof(param));

    if (!arg) {
        NPU_ERR("input parameter arg is NULL, FATAL arg and ignore\n");
        return -EINVAL;
    }

    if (!adapter) {
        NPU_ERR("FATAL: adapter is NULL\n");
        return -EFAULT;
    }

    if (copy_from_user(param, (void __user *)arg, sizeof(param))) {
        NPU_ERR("copy arg failed!\n");
        return -EFAULT;
    }

    coreid = param[0];
    profile = param[1];

    if (coreid > adapter->common.feature.core_num - 1) {
        NPU_ERR("coreid[%d] err\n", coreid);
        return -EFAULT;
    }

    if (profile > DEV_HIGH_PROFILE) {
        NPU_ERR("profile[%u] err\n", profile);
        return -EFAULT;
    }

    if (adapter->core[coreid].security_mode != SECURETY) {
        NPU_ERR("security[%u] err\n", adapter->core[coreid].security_mode);
        return -EBUSY;
    }

    enable_irq(adapter->core[coreid].irq);

    CTRL_LOCK(coreid);  //lint !e454

    /* recover non-secure clk profile */
    npu_set_profile(coreid, profile);

    npu_smmu_init(coreid);

    npu_interrupt_init(coreid);

    npu_work_put(coreid);

    adapter->core[coreid].security_mode = NON_SECURETY;

    npu_task_restore(coreid, TASK_SEC_CANCEL);

    CTRL_UNLOCK(coreid); //lint !e455

    MODE_UNLOCK(coreid); //lint !e455

    NPU_DEBUG(" Release secmode coreid[%d]\n", coreid);

    return 0;
}

static long npu_set_reset_statistic(unsigned long arg)
{
    int coreid = 0;
    int enable = 1;

    UNUSED_PARAMETER(arg);

    if (!adapter) {
        NPU_ERR("FATAL: adapter is NULL\n");
        return -EFAULT;
    }

    for (coreid = 0; coreid < (int)(adapter->common.feature.core_num); coreid++) {
        CTRL_LOCK(coreid); //lint !e454
    }
    npu_smmu_set_stat_en(enable);

    for (coreid = (int)(adapter->common.feature.core_num - 1); coreid >= 0; coreid--) {
        CTRL_UNLOCK(coreid); //lint !e455
    }

    return 0; /*lint !e454*/
}

static long npu_set_report_statistic(unsigned long arg)
{
    unsigned int coreid = 0;
    SMMU_STAT_S smmu_stat;

    if (!arg) {
        NPU_ERR("nPU_ERROR:input parameter arg is NULL, FATAL arg and ignore\n");
        return -EINVAL;
    }

    if (!adapter) {
        NPU_ERR("FATAL: adapter is NULL\n");
        return -EFAULT;
    }

    if (copy_from_user(&smmu_stat, (void __user *)arg, sizeof(smmu_stat))) {
        NPU_ERR("Copy data from user failed!");
        return -EFAULT;
    }

    /* only smmu_stat.coreID is an input param, other members in smmu_stat are output params, no need check */
    if (smmu_stat.coreID > adapter->common.feature.core_num - 1) {
        NPU_ERR("invalid input core number=%u", smmu_stat.coreID);
        return -EFAULT;
    }

    coreid = smmu_stat.coreID;

    CTRL_LOCK(coreid);  //lint !e454

    npu_smmu_report_statistic(&smmu_stat);

    CTRL_UNLOCK(coreid);  //lint !e455

    if (copy_to_user((void __user *)arg, &smmu_stat, sizeof(SMMU_STAT_S))) {
        NPU_ERR("copy_to_user failed core number=%d!\n", coreid);
        return -EFAULT;
    }

    return 0;
}

static long npu_set_update_pte(unsigned long arg)
{
    unsigned int coreid = 0;

    if (!arg) {
        NPU_ERR("input parameter arg is NULL, FATAL arg and ignore\n");
        return -EINVAL;
    }

    if (!adapter) {
        NPU_ERR("FATAL: adapter is NULL\n");
        return -EFAULT;
    }

    if (copy_from_user(&coreid, (void __user *)arg, sizeof(int))) {
        NPU_ERR("copy arg failed!\n");
        return -EFAULT;
    }

    if (coreid > adapter->common.feature.core_num - 1) {
        NPU_ERR("coreid[%d] err\n", coreid);
        return -EFAULT;
    }

    CTRL_LOCK(coreid);  //lint !e454

    if (!npu_chip_can_direct_access(coreid)) {
        CTRL_UNLOCK(coreid); //lint !e455
        NPU_ERR("this work is not allowed when NPU[%d] is power-down, FATAL and ignore\n", coreid);
        return -EBUSY;
    }

    npu_work_get(coreid);
    npu_smmu_pte_update(coreid);
    npu_work_put(coreid);

    CTRL_UNLOCK(coreid);  //lint !e455

    return 0;
}

static long npu_set_reset_virt_addr(unsigned long arg)
{
    long ret = 0;
    int coreid = 0;
    struct map_data map_data;
    memset(&map_data, 0x0, sizeof(struct map_data));

    if (!arg) {
        NPU_ERR("input parameter arg is NULL, FATAL arg and ignore\n");
        return -EINVAL;
    }

    if (!adapter) {
        NPU_ERR("FATAL: adapter is NULL\n");
        return -EFAULT;
    }

    if (copy_from_user(&map_data, (void __user *)arg, sizeof(map_data))) {
        NPU_ERR("copy arg failed!\n");
        return -EFAULT;
    }

    /* for map_data:
        (1) map_data.share_fd is a input param.
            dma_buf_get in kernel/linux-4.9/drivers/dma-buf/dma-buf.c checks it.
        (2) map_data.format.iova_size and map_data.format.iova_start are output params. no need check */
    if (map_data.format.prot) {
        NPU_ERR("FATAL: map_data.format.prot=%d is invalid\n", map_data.format.prot);
        return -EFAULT;
    }

    for (coreid = 0; coreid < (int)(adapter->common.feature.core_num); coreid++) {
        CTRL_LOCK(coreid); //lint !e454
    }

    ret = npu_smmu_reset_virt(&map_data);
    if (ret) {
        NPU_ERR("npu_smmu_map failed!\n");
    }

	if (copy_to_user((void __user *)arg, &map_data, sizeof(map_data))) {
        NPU_ERR("copy_to_user failed!\n");
        ret = -EFAULT;
    }
    for (coreid = (int)(adapter->common.feature.core_num - 1); coreid >= 0; coreid--) {
        CTRL_UNLOCK(coreid); //lint !e455
    }

    return ret; /*lint !e454*/
}

long npu_set_workprofile(unsigned long arg)
{
    int ret;
    unsigned int param[2];
    unsigned int coreid = 0;
    unsigned int profile = 0;

    if (!arg) {
        NPU_ERR("input parameter arg is NULL, FATAL arg and ignore\n");
        return -EINVAL;
    }

    if (!adapter) {
        NPU_ERR("FATAL: adapter is NULL\n");
        return -EFAULT;
    }

    memset(param, 0x0, sizeof(param));
    if (copy_from_user(param, (void __user *)arg, sizeof(param))) {
        NPU_ERR("copy arg failed!\n");
        return -EFAULT;
    }

    coreid  = param[0];
    profile = param[1];   /*profile clock module check*/

    if (coreid > adapter->common.feature.core_num - 1) {
        NPU_ERR("coreid[%u] err\n", coreid);
        return -EFAULT;
    }

    if (profile > DEV_HIGH_PROFILE) {
        NPU_ERR("profile[%u] err\n", profile);
        return -EFAULT;
    }

    CTRL_LOCK(coreid);  //lint !e454

    ret = npu_set_profile(coreid, profile);
    if (ret) {
        CTRL_UNLOCK(coreid);  //lint !e455
        NPU_ERR("npu_clock_set_profile coreid[%d] err\n", coreid);
        return ret;
    }

    CTRL_UNLOCK(coreid);  //lint !e455

    return 0;
}

static long npu_set_map(unsigned long arg)
{
    long ret = 0;
    int coreid = 0;
    struct map_data map_data;
    memset(&map_data, 0x0, sizeof(struct map_data));

    if (!arg) {
        NPU_ERR("input parameter arg is NULL, FATAL arg and ignore\n");
        return -EINVAL;
    }

    if (!adapter) {
        NPU_ERR("FATAL: adapter is NULL\n");
        return -EFAULT;
    }

    if (copy_from_user(&map_data, (void __user *)arg, sizeof(map_data))) {
        NPU_ERR("copy arg failed!\n");
        return -EFAULT;
    }

    /* for map_data:
        (1) map_data.share_fd is a input param.
            dma_buf_get in kernel/linux-4.9/drivers/dma-buf/dma-buf.c checks it.
        (2) map_data.format.iova_size and map_data.format.iova_start are output params. no need check */
    if (map_data.format.prot) {
        NPU_ERR("FATAL: map_data.format.prot=%d is invalid\n", map_data.format.prot);
        return -EFAULT;
    }

    for (coreid = 0; coreid < (int)(adapter->common.feature.core_num); coreid++) {
        CTRL_LOCK(coreid); //lint !e454
    }

    ret = npu_smmu_map(&map_data);
    if (ret) {

        for (coreid = (int)(adapter->common.feature.core_num - 1); coreid >= 0; coreid--) {
            CTRL_UNLOCK(coreid); //lint !e455
        }

        NPU_ERR("npu_smmu_map failed!\n");
        return ret; /*lint !e454*/
    }

    for (coreid = (int)(adapter->common.feature.core_num - 1); coreid >= 0; coreid--) {
        CTRL_UNLOCK(coreid); //lint !e455
    }

    if (copy_to_user((void __user *)arg, &map_data, sizeof(map_data))) {
        NPU_ERR("copy_to_user failed!\n");
        ret = -EFAULT;
    }

    return ret; /*lint !e454*/
}

static long npu_set_unmap(unsigned long arg)
{
    long ret = 0;
    int coreid = 0;
    struct map_data map_data;
    memset(&map_data, 0x0, sizeof(struct map_data));

    if (!arg) {
        NPU_ERR("input parameter arg is NULL, FATAL arg and ignore\n");
        return -EINVAL;
    }

    if (!adapter) {
        NPU_ERR("FATAL: adapter is NULL\n");
        return -EFAULT;
    }

    if (copy_from_user(&map_data, (void __user *)arg, sizeof(map_data))) {
        NPU_ERR("copy_from_user failed!\n");
        return -EFAULT;
    }

    for (coreid = 0; coreid < (int)(adapter->common.feature.core_num); coreid++) {
        CTRL_LOCK(coreid); //lint !e454
    }

    /* the validation of map_data is checked in npu_mem_mngr_check_repeat */
    if (npu_smmu_unmap(&map_data)) {
        NPU_ERR("unmap failed\n");
        ret = -EINVAL;
    }

    for (coreid = (int)(adapter->common.feature.core_num - 1); coreid >= 0; coreid--) {
        CTRL_UNLOCK(coreid);  //lint !e455
    }

    return ret; /*lint !e454*/
}

static const struct npu_ioctl_map npu_ioctl_maps[] = {
    {GETCONFIG_CHIP_TYPE,       npu_get_chip_type},
    {IN_TASKQUEUE,              npu_process_workqueue},
    {SETCONFIG_RESET_STATISTIC, npu_set_reset_statistic},
    {SETCONFIG_REPORT_STATISTIC,npu_set_report_statistic},
    {SETCONFIG_UPDATE_PTE,      npu_set_update_pte},
    {SETCONFIG_RESET_VIRT_ADDR, npu_set_reset_virt_addr},
    {SETCONFIG_NPU_PROFILE,     npu_set_workprofile},
    {SET_SECURE_MODE,           npu_set_secure_mode},
    {RELEASE_SECURE_MODE,       npu_release_secure_mode},
    {SETCONFIG_MAP,             npu_set_map},
    {SETCONFIG_UNMAP,           npu_set_unmap},
};

static int input_filp_check(struct file *filp, unsigned int cmd)
{
    NPU_MAIN_PRIVATE_COMMON_S *common_dev;
    if (!filp){
        NPU_ERR("input parameter filp is invalid !\n");
        return -EINVAL;
    }
    common_dev = filp->private_data;
    if (!common_dev || common_dev != &adapter->common) {
        NPU_ERR("No NPU device! input dev is %pK\n", (void *)common_dev);
        return -EINVAL;
    }

    /* check whether cmd is valid */
    if (_IOC_TYPE(cmd) != MAGIC_NUM) {
        NPU_ERR("cmd is invalid!(not a MAGIC_NUM)\n");
        return -EINVAL;
    }

    if (_IOC_NR(cmd) > NPU_IOC_MAXNR) {
        NPU_ERR("cmd is invalid!(%d > IPU_IOC_MAXNR)\n", _IOC_NR(cmd));
        return -EINVAL;
    }

    NPU_DEBUG("cmd is (%d)\n", _IOC_NR(cmd));

    return 0;
}

static ioctl_cb npu_obtain_ioctl_callback(unsigned int cmd)
{
    unsigned int cnt;
    unsigned int size = ARRAY_SIZE(npu_ioctl_maps);

    for (cnt = 0; cnt < size; cnt++) {
        if (npu_ioctl_maps[cnt].cmd == cmd) {
            return npu_ioctl_maps[cnt].func;
        }
    }

    NPU_ERR("error cmd=0x%x\n", cmd);

    return NULL;
}

static long npu_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    ioctl_cb ioctl_callback;
    long ret = -EINVAL;

    if (!adapter) {
        NPU_ERR("FATAL: adapter is NULL\n");
        return -EFAULT;
    }

    if (input_filp_check(filp, cmd)) {
        NPU_ERR(" input parameter filp is invalid !\n");
        return -EINVAL;
    }

    ioctl_callback = npu_obtain_ioctl_callback(cmd);

    if (ioctl_callback) {
        ret = ioctl_callback(arg);
    } else {
        NPU_ERR(" unknown cmd=0x%x\n", cmd);
    }

    return ret;
}

static long npu_ioctl32(struct file *filp, unsigned int cmd, unsigned long arg)
{
    void *ptr_user = compat_ptr(arg);

    return npu_ioctl(filp, cmd, (unsigned long)ptr_user);
}

static loff_t npu_llseek(struct file *filp, loff_t off, int whence)
{

    return off;
}

/* global variable declaration */
static const struct file_operations npu_fops = {
    .owner          = THIS_MODULE,/*lint !e64*/
    .open           = npu_open,
    .release        = npu_release,
    .write          = npu_write,
    .unlocked_ioctl = npu_ioctl,
    .compat_ioctl   = npu_ioctl32,
    .llseek         = npu_llseek,
};/*lint !e785*/


/* ipu platform device */
static struct platform_device npu_device_core0 = {
    .name   = "npu-core0",
    .id = -1,
};/*lint !e785*/

/* ipu platform device */
static struct platform_device npu_device_core1 = {
    .name   = "npu-core1",
    .id = -1,
};/*lint !e785*/

/* platform device */
static struct platform_device npu_device = {
    .name   = "kirin-npu",
    .id = -1,
};

static const struct of_device_id npu_match_table[] = {
    { .compatible = NPU_DRV_NAME, },
    {},
};

/* reset common feature*/
static void npu_common_release_features(void)
{
    NPU_MAIN_PRIVATE_COMMON_S* p_common = &adapter->common;

    memset(&p_common->feature, 0x0, sizeof(FEATURE_S));

    hard_version = 0;

    return;
}

/* get common feature by dtsi */
static int npu_common_get_features(struct device *dev)
{
    int property_rd = 0;
    FEATURE_S* p_feature = &adapter->common.feature;

    struct device_node *node = of_find_node_by_name(dev->of_node, "platform_features");
    if (!node) {
        NPU_ERR(" find common_features node error.\n");
        return -EFAULT;
    }

    property_rd  = of_property_read_u32(node, "core_num", &p_feature->core_num);
    property_rd |= of_property_read_u32(node, "is_sswq",  &p_feature->is_selfstart);
    if (property_rd) {
        NPU_ERR("read property of core-num and sswq error. p_feature->core_num=%u, p_feature->core_num=%u\n",
			p_feature->core_num, p_feature->is_selfstart);
        goto err;
    }
    NPU_DEBUG("core num[%d], sswq[%d]\n", p_feature->core_num, p_feature->is_selfstart);
    if ((p_feature->core_num > MAX_SUPPORT_CORE_NUM) || (0 == p_feature->core_num)) {
        NPU_ERR("read property of core-num[%d].\n", p_feature->core_num);
        goto err;
    }

    if (MAX_SUPPORT_CORE_NUM == p_feature->core_num) {
        hard_version = NPU_VERSION_V200;
    } else if ((1 == p_feature->core_num) && (0 == p_feature->is_selfstart)) {
        hard_version = NPU_VERSION_V150;
    } else {
        NPU_ERR(" not support core num[%d], sswq[%d]\n", p_feature->core_num, p_feature->is_selfstart);
        goto err;
    }

    property_rd = of_property_read_u32(node, "level1_irq",               &p_feature->level1_irq);
    if (property_rd) {
        NPU_DEBUG("not support level1_irq\n");
        p_feature->level1_irq = 0;
    }
    property_rd = of_property_read_u32(node, "performance_monitor",      &p_feature->performance_monitor);
    if (property_rd) {
        NPU_DEBUG("not support performance_monitor\n");
        p_feature->performance_monitor = 0;
    }
    property_rd = of_property_read_u32(node, "lpm3_set_vcodecbus",       &p_feature->lpm3_set_vcodecbus);
    if (property_rd) {
        NPU_DEBUG("not support lpm3_set_vcodecbus\n");
        p_feature->lpm3_set_vcodecbus = 0;
    }
    property_rd = of_property_read_u32(node, "npu_reset_when_in_error",  &p_feature->npu_reset_when_in_error);
    if (property_rd) {
        NPU_DEBUG("not support npu_reset_when_in_error\n");
        p_feature->npu_reset_when_in_error = 0;
    }
    property_rd = of_property_read_u32(node, "npu_bandwidth_lmt",        &p_feature->npu_bandwidth_lmt);
    if (property_rd) {
        NPU_DEBUG("not support npu_bandwidth_lmt\n");
        p_feature->npu_bandwidth_lmt = 0;
    }

    return 0;

err:
    npu_common_release_features();
    return -EFAULT;
}

/* Allocate npu chrdev region ("dev/npu") */
static int npu_common_chrdev_region(dev_t *chrdev)
{
    int err = -1;
    NPU_MAIN_PRIVATE_COMMON_S* p_common = &adapter->common;

    /* Allocate char device region */
    if (p_common->npu_major) {
        *chrdev = MKDEV(p_common->npu_major, p_common->npu_minor);
        err = register_chrdev_region(*chrdev, 1, NPU_NAME);
    } else {
        err = alloc_chrdev_region(chrdev, 0, 1, NPU_NAME);
    }

    if (err < 0) {
        NPU_ERR("alloc_chrdev_region fail!\n");
        return err;
    }

    p_common->npu_major = MAJOR(*chrdev);
    p_common->npu_minor = MINOR(*chrdev);

    return err;
}

/* free npu chrdev region ("dev/npu") */
static void npu_common_chrdev_unregion(void)
{
    dev_t chrdev;
    NPU_MAIN_PRIVATE_COMMON_S* p_common = &adapter->common;

    /* device has two dev number, i.e. npu_major, npu_minor */
    chrdev = MKDEV(p_common->npu_major, p_common->npu_minor);

    /* Unregister chrdev region */
    unregister_chrdev_region(chrdev, 1);

    p_common->npu_major = 0;
    p_common->npu_minor = 0;

    return;
}

/*device init*/
static int npu_common_device_init(void)
{
    int err = -1;
    dev_t chrdev = 0;
    NPU_MAIN_PRIVATE_COMMON_S* p_common = &adapter->common;

    /* Allocate char device region */
    err = npu_common_chrdev_region(&chrdev);
    if (err < 0) {
        NPU_ERR("Failed to allocate device ID for NPU!\n");
        return -EFAULT;
    }

    /* Add npu char device to system, udev can auto detect */
    cdev_init(&(p_common->cdev), &npu_fops);
    p_common->cdev.owner = THIS_MODULE;/*lint !e64*/
    p_common->cdev.ops = &npu_fops;
    err = cdev_add(&(p_common->cdev), chrdev, 1);
    if (err) {
        NPU_ERR("Failed to Add NPU to system!\n");
        goto chrdev_unregion;
    }

    /* Create npu class under /sys/class */
    p_common->dev_class = class_create(THIS_MODULE, NPU_NAME);/*lint !e64*/
    if (IS_ERR(p_common->dev_class)) {
        err = (int)PTR_ERR(p_common->dev_class);
        NPU_ERR("Failed to create Npu class!\n");
        goto cdev_delete;
    }

    /* Register npu device in sysfs, and this will cause udev to create corresponding device node */
    p_common->device = device_create(p_common->dev_class, NULL, chrdev, NULL, "%s", NPU_NAME);
    if (IS_ERR(p_common->device)) {
        err = (int)PTR_ERR(p_common->device);
        NPU_ERR("Failed to mount NPU to /dev/%s!\n", NPU_NAME);
        goto destroy_class;
    }

    return 0;

destroy_class:
    if (p_common->dev_class) {
        class_destroy(p_common->dev_class);
        p_common->dev_class = NULL;
    }

cdev_delete:
    cdev_del(&p_common->cdev);

chrdev_unregion:
    npu_common_chrdev_unregion();

    return err;
}

static void npu_common_device_exit(void)
{
    dev_t chrdev = 0;
    NPU_MAIN_PRIVATE_COMMON_S* p_common = &adapter->common;

    chrdev = MKDEV(p_common->npu_major, p_common->npu_minor);

    if (p_common->dev_class) {
        device_destroy(p_common->dev_class, chrdev);
        class_destroy(p_common->dev_class);
        p_common->dev_class = NULL;
    }

    cdev_del(&p_common->cdev);

    npu_common_chrdev_unregion();

    return;
}

/*reset exit*/
static void npu_common_reset_exit(void)
{
    NPU_MAIN_PRIVATE_COMMON_S* p_common = &adapter->common;

    if (RESET_UNSUPPORT == p_common->feature.npu_reset_when_in_error) {
        NPU_DEBUG("feature reset is not support\n");
        return;
    }

    if (p_common->dmss.io_addr) {
        iounmap(p_common->dmss.io_addr);
        p_common->dmss.io_addr = NULL;
    }

    if (p_common->peri_reg.io_addr) {
        iounmap(p_common->peri_reg.io_addr);
        p_common->peri_reg.io_addr = NULL;
    }

    if (p_common->media1_reg.io_addr) {
        iounmap(p_common->media1_reg.io_addr);
        p_common->media1_reg.io_addr = NULL;
    }

    if (p_common->media2_reg.io_addr) {
        iounmap(p_common->media2_reg.io_addr);
        p_common->media2_reg.io_addr = NULL;
    }

    if (p_common->pctrl_reg.io_addr) {
        iounmap(p_common->pctrl_reg.io_addr);
        p_common->pctrl_reg.io_addr = NULL;
    }
    if (p_common->pmctrl_reg.io_addr) {
        iounmap(p_common->pmctrl_reg.io_addr);
        p_common->pmctrl_reg.io_addr = NULL;
    }

    return;
}

/*reset reg init*/
static int npu_common_reset_init(struct device *dev)
{
    NPU_MAIN_PRIVATE_COMMON_S* p_common = &adapter->common;
    PMCTRL_REG_S* pmctrl = &p_common->pmctrl_reg;
    PCTRL_REG_S* pctrl   = &p_common->pctrl_reg;
    MEDIA2_REG_S* media2 = &p_common->media2_reg;
    PERI_REG_S* peri     = &p_common->peri_reg;
    MEDIA1_REG_S* media1 = &p_common->media1_reg;
    DMSS_REG_S*   dmss   = &p_common->dmss;

    if (RESET_UNSUPPORT == p_common->feature.npu_reset_when_in_error) {
        NPU_DEBUG("feature reset is not support\n");
        return 0;
    }

    pmctrl->io_addr = ioremap((unsigned long)SOC_ACPU_PMC_BASE_ADDR, (unsigned long)0xfff);
    if (!pmctrl->io_addr) {
        NPU_ERR("pmctrl_reg ioremap fail\n");
        goto err;
    }

    pctrl->io_addr = ioremap((unsigned long)SOC_ACPU_PCTRL_BASE_ADDR, (unsigned long)0xff);
    if (!pctrl->io_addr) {
        NPU_ERR("pctrl_reg ioremap fail\n");
        goto err;
    }

    media2->io_addr = ioremap((unsigned long)SOC_ACPU_MEDIA2_CRG_BASE_ADDR, (unsigned long)0xff);
    if (!media2->io_addr) {
        NPU_ERR("media2_reg ioremap fail\n");
        goto err;
    }

    media1->io_addr = ioremap((unsigned long)SOC_ACPU_MEDIA1_CRG_BASE_ADDR, (unsigned long)0xff);
    if (!media1->io_addr) {
        NPU_ERR("media1_io_addr ioremap fail\n");
        goto err;
    }

    peri->io_addr = ioremap((unsigned long)SOC_ACPU_PERI_CRG_BASE_ADDR, (unsigned long)0xfff);
    if (!peri->io_addr) {
        NPU_ERR("peri_reg ioremap fail\n");
        goto err;
    }

    dmss->io_addr = ioremap((unsigned long)SOC_DMSS_GLB_MST_FLUX_ADDR(SOC_ACPU_DMSS_BASE_ADDR, NPU_DCQ_DMSS_MSTS), (unsigned long)0xf);
    if (!dmss->io_addr) {
        NPU_ERR("dmss_glb_mst_flux_addr ioremap fail\n");
        goto err;
    }

    return 0;

err:
    npu_common_reset_exit();

    return -EFAULT;
}

/*common_init*/
static int npu_common_init(struct device *dev)
{
    int err = -1;

    /* get common_features */
    err = npu_common_get_features(dev);
    if (err < 0) {
        NPU_ERR("fatal err, npu_common_get_features\n");
        return err;
    }

    err = npu_common_device_init();
    if (err < 0) {
        NPU_ERR("Failed npu_common_device_init!\n");
        goto reset_features;
    }

    err = npu_common_reset_init(dev);
    if (err < 0) {
        NPU_ERR("Failed npu_common_reset_init!\n");
        goto device_exit;
    }

    NPU_DEBUG("Succeeded to npu_common_init.\n");

    return 0;

device_exit:
    npu_common_device_exit();

reset_features:
    npu_common_release_features();

    return err;
}

/*common exit*/
static void npu_common_exit(void)
{
    npu_common_reset_exit();

    npu_common_device_exit();

    npu_common_release_features();

    return;
}

/*get device features*/
static int npu_core_get_features(unsigned int coreid, struct platform_device *pdev)
{
    char* dma_irq_name[2] = {"npu_dma_irq0", "npu_dma_irq1"};

    if (coreid > adapter->common.feature.core_num - 1) {
        NPU_ERR("coreid[%d] err\n", coreid);
        return -1;
    }

    adapter->core[coreid].irq = (unsigned int)platform_get_irq_byname(pdev, dma_irq_name[coreid]);

    NPU_DEBUG("platform_get_irq_byname:irq[%d]=%d\n", coreid, adapter->core[coreid].irq);

    return 0;
}

/*reset device features*/
static void npu_core_release_features(unsigned int coreid)
{
    if (coreid > adapter->common.feature.core_num - 1) {
        NPU_ERR("coreid[%d] err\n", coreid);
        return;
    }

    adapter->core[coreid].irq = 0;

    return;
}

/* reset device feature*/
static void npu_core_reg_irq_exit(unsigned int coreid)
{
    NPU_MAIN_PRIVATE_CORE_S* p_core = &adapter->core[coreid];

    if (p_core->npu_irq.io_addr){
        iounmap(p_core->npu_irq.io_addr);
        p_core->npu_irq.io_addr = NULL;
    }

    memset(&p_core->npu_irq, 0, sizeof(NPU_IRQ_REG_S));

    return;
}

/* get device feature by dtsi*/
static int npu_core_reg_irq_init(unsigned int coreid, struct device *dev)
{
    int property_rd = 0;
    char* dts_irq_name[2] = {"npu_irq0", "npu_irq1"};
    struct device_node *node = NULL;
    NPU_MAIN_PRIVATE_CORE_S* p_core;

    if (coreid > adapter->common.feature.core_num - 1) {
        NPU_ERR("coreid[%d] err\n", coreid);
        return -1;
    }
    p_core = &adapter->core[coreid];

    node = of_find_node_by_name(dev->of_node, dts_irq_name[coreid]);
    if (!node) {
        NPU_ERR("find npu_irq node error\n");
        return -EFAULT;
    }

    property_rd  = of_property_read_u32(node, "base-addr", &p_core->npu_irq.base_addr);
    if (property_rd) {
        NPU_ERR("read property of irq offset error\n");
        goto err;
    }

    p_core->npu_irq.io_addr = ioremap(p_core->npu_irq.base_addr, (unsigned long)0xff);
    if (!p_core->npu_irq.io_addr) {
        NPU_ERR("io_addr[%d] ioremap fail!\n", coreid);
        goto err;
    }

    return 0;

err:
    npu_core_reg_irq_exit(coreid);

    return -EFAULT;
}

void npu_reset(unsigned int coreid)
{
	struct npu_peri_reg_s peri_reg;
    unsigned long peri_io_addr   = (unsigned long)adapter->common.peri_reg.io_addr;
    unsigned long media2_io_addr = (unsigned long)adapter->common.media2_reg.io_addr;

    if (coreid > adapter->common.feature.core_num - 1) {
        NPU_ERR("param err coreid[%d]!", coreid);
        return;
    }

    if (!npu_chip_can_direct_access(coreid)) {
        NPU_ERR(" npu_chip_can_direct_access is false\n");
        return;
    }

    npu_work_get(coreid);

    /* get clock and power status in register */
    peri_reg.peri_stat   = ioread32((void *)SOC_CRGPERIPH_PERI_STAT7_ADDR(peri_io_addr));
    peri_reg.ppll_select = ioread32((void *)SOC_CRGPERIPH_CLKDIV8_ADDR(peri_io_addr));
    peri_reg.power_stat  = ioread32((void *)SOC_CRGPERIPH_PERPWRSTAT_ADDR(peri_io_addr));
    peri_reg.power_ack   = ioread32((void *)SOC_CRGPERIPH_PERPWRACK_ADDR(peri_io_addr));

    peri_reg.reset_stat  = ioread32((void *)SOC_MEDIA2_CRG_PERRSTSTAT0_ADDR(media2_io_addr));
    peri_reg.perclken0   = ioread32((void *)SOC_MEDIA2_CRG_PERCLKEN0_ADDR(media2_io_addr));
    peri_reg.perstat0    = ioread32((void *)SOC_MEDIA2_CRG_PERSTAT0_ADDR(media2_io_addr));

    NPU_DEBUG("peri_stat=%x, ppll_select=%x, power_stat=%x, power_ack=%x, reset_stat=%x, perclken=%x, perstat=%x\n",
        peri_reg.peri_stat, peri_reg.ppll_select, peri_reg.power_stat, peri_reg.power_ack, peri_reg.reset_stat, peri_reg.perclken0, peri_reg.perstat0);

    npu_mntn_peri_reg_init(coreid, &peri_reg);

    npu_smmu_dump_strm(coreid);

    rdr_system_error((unsigned int)MODID_NPU_EXC_DEAD, coreid, 0);

    /* reset ipu */
    npu_reset_proc(coreid);

    npu_work_put(coreid);

    return; /*lint !e527*/
}

/*interrupt handler*/
static irqreturn_t npu_interrupt_handler(int irq, void *dev)
{
    unsigned int coreid = 0;
    bool smmu_err_isr = false;
    NPU_MAIN_PRIVATE_COMMON_S *common_dev;

    if (dev == NULL) {
        NPU_ERR("no device\n");
        return IRQ_HANDLED;
    }

    common_dev = container_of((struct cdev *)dev, NPU_MAIN_PRIVATE_COMMON_S, cdev);
    if (!common_dev || common_dev != &adapter->common) {
        NPU_ERR("IPU_ERROR:error dev\n");
        return IRQ_HANDLED;
    }

    for (coreid = 0; coreid < MAX_SUPPORT_CORE_NUM; coreid++) {
        if (adapter->core[coreid].irq == irq) {
            break;
        }
    }

    if (coreid == MAX_SUPPORT_CORE_NUM) {
        NPU_ERR("invalid irq = %d\n", irq);
        return IRQ_HANDLED;
    }

    CTRL_LOCK(coreid);  //lint !e454

    if (!npu_chip_can_direct_access(coreid)) {
        CTRL_UNLOCK(coreid);  //lint !e455

        NPU_ERR("npu is work off, ignore\n");

        return IRQ_HANDLED;
    }

    npu_work_get(coreid);

    performance_monitor_close(coreid);

    /*smmu interrupt handler*/
    smmu_err_isr = npu_smmu_interrupt_handler(coreid);
    if (smmu_err_isr) {
        npu_mntn_set_core_id(coreid);
        rdr_system_error((unsigned int)MODID_NPU_EXC_INTERRUPT_ABNORMAL, coreid, 0);
        npu_reset_proc(coreid);
        npu_task_restore(coreid, TASK_INTERRUPT_ERR);
    } else {
        npu_core_irq_handler_func(coreid);
    }

    npu_work_put(coreid);

    CTRL_UNLOCK(coreid);  //lint !e455

    return IRQ_HANDLED;
}

/*core module device init*/
static int npu_core_init(unsigned int coreid, struct platform_device *pdev)
{
    int err = -EFAULT;
    NPU_MAIN_PRIVATE_COMMON_S* p_common = &adapter->common;
    NPU_MAIN_PRIVATE_CORE_S* p_core = &adapter->core[coreid];

    mutex_init(&p_core->ctrl_mutex);
    mutex_init(&p_core->mode_mutex);

    p_core->security_mode = NON_SECURETY;

    err = npu_core_get_features(coreid, pdev);
    if (err < 0){
        NPU_ERR("coreid[%d], Failed to get device features!\n", coreid);
        return -EFAULT;
    }

    err = npu_core_reg_irq_init(coreid, &pdev->dev);
    if (err){
        NPU_ERR("coreid[%d], failed npu_core_reg_irq_init\n", coreid);
        goto core_reset_features;
    }

    /* request npu irq */
    if (request_threaded_irq(p_core->irq, NULL, npu_interrupt_handler,
            (unsigned long)(IRQF_ONESHOT | IRQF_TRIGGER_HIGH), NPU_NAME, &p_common->cdev)) {
        NPU_ERR("NPU%d Require IRQ failed!\n", coreid);
        err = -EIO;
        goto core_reg_irq_exit;
    }

    wake_lock_init(&p_core->wakelock, WAKE_LOCK_SUSPEND, "npu_process");
    NPU_DEBUG("Succeeded to npu_core_init coreid[%d].\n", coreid);

    return 0;

core_reg_irq_exit:
    npu_core_reg_irq_exit(coreid);

core_reset_features:
    npu_core_release_features(coreid);

    return err;
}

/*core module device exit*/
static void npu_core_exit(unsigned int coreid)
{
    NPU_MAIN_PRIVATE_COMMON_S* p_common = &adapter->common;
    NPU_MAIN_PRIVATE_CORE_S* p_core = &adapter->core[coreid];

    free_irq(p_core->irq, &p_common->cdev);

    npu_core_reg_irq_exit(coreid);

    npu_core_release_features(coreid);

    p_core->security_mode = NON_SECURETY;

    wake_lock_destroy(&p_core->wakelock);
    mutex_destroy(&p_core->mode_mutex);
    mutex_destroy(&p_core->ctrl_mutex);

    return;
}

static int npu_main_init(struct platform_device *pdev)
{
    int err = -1;
    int coreid = 0;

    /* Allocate npu_private struct and asigned to global variable adapter, devm_kzalloc will memset adapter to zeros */
    adapter = devm_kzalloc(&pdev->dev, sizeof(NPU_MAIN_PRIVATE_S), GFP_KERNEL);
    if (!adapter) {
        NPU_ERR("Failed to allocate memory for struct npu!\n");
        return -ENOMEM;
    }

    err = npu_common_init(&pdev->dev);
    if (err < 0) {
        NPU_ERR("Failed to init common!\n");
        goto cleanup;
    }

    for (coreid = 0; coreid < (int)(adapter->common.feature.core_num); coreid++) {
        err = npu_core_init(coreid, pdev);
        if (err < 0) {
            NPU_ERR("Failed npu_core_init coreid[%d]!\n", coreid);
            goto device_exit;
        }
    }

    platform_set_drvdata(pdev, adapter);

    return 0;

device_exit:
    for (; coreid > 0; coreid--) {
        npu_core_exit(coreid - 1);
    }

    npu_common_exit();

cleanup:
    if (adapter) {
        devm_kfree(&pdev->dev, adapter);
        adapter = NULL;
    }

    return err;
}

static void npu_main_exit(struct platform_device *pdev)
{
    unsigned int coreid = 0;

    if (!adapter) {
        return;
    }

    for (coreid = 0; coreid < adapter->common.feature.core_num; coreid++) {
        npu_core_exit(coreid);
    }

    npu_common_exit();

    devm_kfree(&pdev->dev, adapter);
    adapter = NULL;

    return;
}

static int npu_pm_runtime_suspend(struct device *dev)
{
    unsigned int coreid = 0;

    /* Call NPU PowerDown Interface */
    if (dev == adapter_pm_core0->dev) {
        coreid = 0;
    } else if (dev == adapter_pm_core1->dev) {
        coreid = 1;
    } else {
        NPU_ERR("ERR[%s][%d]!\n", __func__, __LINE__);
        return -EFAULT;
    }

    if ((NPU_VERSION_V200 != hard_version) && (1 == coreid)) {
        NPU_ERR("not support this op.\n");
        return -EFAULT;
    }

    /* Check the status before PowerDown */
    if (npu_smmu_get_stat_en()){
        npu_smmu_record_statistic(coreid);
    }

    npu_interrupt_deinit(coreid);

    npu_smmu_exit(coreid);

    npu_power_off(coreid);
    NPU_DEBUG("call npu_power_off success! coreid=%d", coreid);

    if ((false == npu_power_status(0) && false == npu_power_status(1)) && (false == adapter->common.npu_opened)) {
        NPU_INFO("npu_mem_mngr_exit after release! coreid=%d", coreid);
        npu_mem_mngr_exit();
    }

    wake_unlock(&adapter->core[coreid].wakelock); //lint !e454  !e455

    return 0;
}

static int npu_pm_runtime_resume(struct device *dev)
{
    int ret = 0;
    unsigned int coreid = 0;

    /* Call NPU PowerDown Interface */
    if (dev == adapter_pm_core0->dev) {
        coreid = 0;
    } else if (dev == adapter_pm_core1->dev) {
        coreid = 1;
    } else {
        NPU_ERR(" [%s][%d]!\n", __func__, __LINE__);
        return -EFAULT;
    }

     if ((NPU_VERSION_V200 != hard_version) && (1 == coreid)) {
        NPU_ERR("not support this op.\n");
        return -EFAULT;
    }

    wake_lock(&adapter->core[coreid].wakelock); //lint !e454  !e455
    ret = npu_power_on(coreid);
    if (ret) {
        NPU_ERR("Call npu_power_on failed! coreid[%d]\n", coreid);
        /* IPU power up fail, report to DSM */
        DSM_AI_KERN_ERROR_REPORT(DSM_AI_KERN_POWER_UP_ERR_NO, "NPU power up fail! core_id=%d, ret=%d\n", coreid, ret);
        wake_unlock(&adapter->core[coreid].wakelock); //lint !e454  !e455
        return ret;
    }
    npu_smmu_init(coreid);
    NPU_DEBUG("call npu_power_on success! coreid=%d", coreid);
    npu_bandwidth_lmt_config(coreid, (void*)adapter->core[coreid].npu_irq.io_addr, npu_get_npu_rate(coreid), npu_get_vcodecbus_rate());
    npu_interrupt_init(coreid);
    if (npu_smmu_get_stat_en()){
        npu_smmu_reset_statistic(coreid);
    }

    return 0; //lint !e454
}

static int npu_pm_runtime_idle(struct device *dev)
{
    /* Check the status before PowerDown */

    if (dev == adapter_pm_core0->dev) {
        pm_schedule_suspend(dev, NPU_SUSPEND_DELAY);

    } else if (dev == adapter_pm_core1->dev) {
        pm_schedule_suspend(dev, NPU_SUSPEND_DELAY);

    } else {
        NPU_ERR("ERR[%s][%d]!\n", __func__, __LINE__);
        return 0;
    }

    return -EBUSY;
}

static int pm_runtime_probe_core0(struct platform_device *pdev)
{
    adapter_pm_core0 = devm_kzalloc(&pdev->dev, sizeof(NPU_PM_RUNTIME_S), GFP_KERNEL);
    if (!adapter_pm_core0) {
        NPU_ERR("Failed to allocate memory for struct pm core!\n");
        return -ENOMEM;
    }

    pm_runtime_enable(&pdev->dev);

    adapter_pm_core0->dev = &pdev->dev;

    platform_set_drvdata(pdev, adapter_pm_core0);
    NPU_DEBUG("pm_runtime_probe_core0 ok\n");

    return 0;
}

static int pm_runtime_remove_core0(struct platform_device *pdev)
{
    NPU_PM_RUNTIME_S *priv = (NPU_PM_RUNTIME_S *)platform_get_drvdata(pdev);

    NPU_DEBUG("pm_runtime_remove_core0!\n");
    pm_runtime_disable(priv->dev);

    if (adapter_pm_core0) {
        devm_kfree(&pdev->dev, adapter_pm_core0);
        adapter_pm_core0 = NULL;
    }

    return 0;
}

static int pm_runtime_probe_core1(struct platform_device *pdev)
{
    adapter_pm_core1 = devm_kzalloc(&pdev->dev, sizeof(NPU_PM_RUNTIME_S), GFP_KERNEL);
    if (!adapter_pm_core1) {
        NPU_ERR("Failed to allocate memory for struct pm core!\n");
        return -ENOMEM;
    }

    pm_runtime_enable(&pdev->dev);

    adapter_pm_core1->dev = &pdev->dev;

    platform_set_drvdata(pdev, adapter_pm_core1);
    NPU_DEBUG("pm_runtime_probe_core1 ok\n");

    return 0;
}

static int pm_runtime_remove_core1(struct platform_device *pdev)
{
    NPU_PM_RUNTIME_S *priv = (NPU_PM_RUNTIME_S *)platform_get_drvdata(pdev);

    NPU_DEBUG("pm_runtime_remove_core1!\n");
    pm_runtime_disable(priv->dev);

    if (adapter_pm_core1) {
        devm_kfree(&pdev->dev, adapter_pm_core1);
        adapter_pm_core1 = NULL;
    }

    return 0;
}

/* probe() function for platform driver */
static int npu_probe(struct platform_device *pdev)
{
    int err = -1;
    NPU_CALLBACK_FUNC_S callback;

    if (!pdev){
        NPU_ERR("FATAL: pdev is NULL\n");
        return -EFAULT;
    }

    memset(&callback, 0x0, sizeof(callback));
    callback.pf_npu_pm_put            = (NPU_PM_PUT_CALLBACK_FUNC)npu_work_put;
    callback.pf_npu_pm_get            = (NPU_PM_GET_CALLBACK_FUNC)npu_work_get;
    callback.pf_npu_reset             = (NPU_RESET_CALLBACK_FUNC)npu_reset;
    callback.pf_npu_update_profile    = (NPU_UPDATE_CALLBACK_FUNC)npu_update_profile;
    callback.pf_npu_perf_monitor_open = (NPU_PERF_MONITOR_EN_CALLBACK_FUNC)performance_monitor_open;
    callback.pf_npu_smmu_pte_update   = (NPU_SMMU_PTE_UPDATE_CALLBACK_FUNC)npu_smmu_pte_update;
    callback.pf_npu_mem_mngr_check    = (NPU_MEM_MNGR_CHECK_CALLBACK_FUNC)npu_mem_mngr_check_valid;
    callback.pf_npu_mem_mngr_dump     = (NPU_MEM_MNGR_DUMP_CALLBACK_FUNC)npu_mem_mngr_dump;
    callback.pf_npu_common_lock       = (NPU_COMMON_LOCK_FUNC)npu_common_lock;
    callback.pf_npu_common_unlock     = (NPU_COMMON_LOCK_FUNC)npu_common_unlock;

    err = npu_main_init(pdev);
    if (err < 0) {
        NPU_ERR("Failed npu_main_init!\n");
        return err;
    }

    err = npu_power_init(&pdev->dev);
    if (err < 0) {
        NPU_ERR("Failed npu_power_init!\n");
        goto main_exit;
    }

    err = npu_smmu_mngr_init(&pdev->dev, adapter->common.feature.npu_bandwidth_lmt);
    if (err < 0) {
        NPU_ERR("Failed npu_smmu_mngr_init!\n");
        goto power_exit;
    }

    err = npu_task_init(pdev, adapter->common.feature.core_num, &callback);
    if (err < 0) {
        NPU_ERR("Failed npu_task_init!\n");
        goto smmu_exit;
    }

    err = npu_mntn_rdr_init();
	if (err) {
		NPU_ERR("Failed npu_mntn_rdr_init!ret=%d\n", err);
		goto task_exit;
	}

    npu_mem_mngr_init();
    NPU_DEBUG("Succeeded to initialize npu device.\n");

    return 0;

task_exit:
    npu_task_exit();

smmu_exit:
    npu_smmu_mngr_exit(&pdev->dev);

power_exit:
    npu_power_exit();

main_exit:
    npu_main_exit(pdev);

    return err;
}

/* remove() function for platform driver */
static int __exit npu_remove(struct platform_device *pdev)
{
    npu_task_exit();

    npu_smmu_mngr_exit(&pdev->dev);

    npu_power_exit();

    npu_main_exit(pdev);

    npu_mntn_rdr_exit();


    NPU_DEBUG("Succeeded to destroying NPU device.\n");
    return 0;
}

static const struct dev_pm_ops npu_pm_ops0 = {
    SET_RUNTIME_PM_OPS(npu_pm_runtime_suspend,
                    npu_pm_runtime_resume,
                    npu_pm_runtime_idle)
};


static const struct dev_pm_ops npu_pm_ops1 = {
    SET_RUNTIME_PM_OPS(npu_pm_runtime_suspend,
                    npu_pm_runtime_resume,
                    npu_pm_runtime_idle)
};

static struct platform_driver npu_driver_core0 = {
    .driver = {
        .name = "npu-core0",
        .owner = THIS_MODULE,/*lint !e64*/
        .pm = &npu_pm_ops0,/* add Set PM ops */
    },
    .probe  = pm_runtime_probe_core0,
    .remove = pm_runtime_remove_core0,
};

static struct platform_driver npu_driver_core1 = {
    .driver = {
        .name = "npu-core1",
        .owner = THIS_MODULE,/*lint !e64*/
        .pm = &npu_pm_ops1,/* add Set PM ops */
    },
    .probe  = pm_runtime_probe_core1,
    .remove = pm_runtime_remove_core1,
};

/* to find key word "kirin-npu" in dts, if failed, not load driver */
MODULE_DEVICE_TABLE(of, npu_match_table);
/* npu platform drive */
static struct platform_driver npu_driver = {
    .driver = {
        .name = "kirin-npu",
        .owner = THIS_MODULE,/*lint !e64*/
        .of_match_table = of_match_ptr(npu_match_table),
    },
    .probe  = npu_probe,
    .remove = npu_remove,
};

/* npu platform device and driver register */
static int __init npu_module_init(void)
{
    int ret;

    NPU_DEBUG("platform device and driver register!\n");
    ret = platform_driver_register(&npu_driver);

    /* No need to "platform_device_register(&npu_device);",
           because it did when DTS initializating, no need to register device again
         */

    ret |= platform_device_register(&npu_device_core0);/*lint !e64*/
    ret |= platform_driver_register(&npu_driver_core0);/*lint !e64*/
    ret |= platform_device_register(&npu_device_core1);/*lint !e64*/
    ret |= platform_driver_register(&npu_driver_core1);/*lint !e64*/

    return ret;
}

/* npu platform device and driver unregister */
static void __exit npu_module_exit(void)
{
    NPU_DEBUG("platform device and driver unregister!\n");

    platform_device_unregister(&npu_device_core1);
    platform_driver_unregister(&npu_driver_core1);

    platform_device_unregister(&npu_device_core0);
    platform_driver_unregister(&npu_driver_core0);

    platform_device_unregister(&npu_device);
    platform_driver_unregister(&npu_driver);
}

module_init(npu_module_init);
module_exit(npu_module_exit);

MODULE_AUTHOR("HISI Limited");
MODULE_LICENSE("GPL");

