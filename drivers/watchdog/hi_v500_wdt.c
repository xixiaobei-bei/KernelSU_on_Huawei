
 /*
 * hi_v500_wdt.c
 *
 * Watchdog driver for HISI V500 watchdog module
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/hisi/dfx_universal_wdt.h>
#include <linux/watchdog.h>
#include <asm/arch_timer.h>
#include <linux/../../kernel/sched/sched.h>
#include <linux/../../kernel/workqueue_internal.h>
#include <linux/amba/bus.h>
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <pr_log.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/platform_device.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/util.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/math64.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/pm.h>
#include <linux/printk.h>
#include <linux/resource.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/syscore_ops.h>
#include <linux/version.h>
#include <securec.h>
#include <soc_hi_watchdog_v500_interface.h>
#ifdef CONFIG_HISI_RPROC
#include <linux/hisi/ipc_msg.h>
#endif

#define PR_LOG_TAG HI_V500_WDT_TAG

/* default timeout in seconds */
#define DEFAULT_TIMEOUT     60
/*
 * it is used to adjust the time of feeding wdg,
 * defined 3,  So TIMEOUT/2/3 to feed
 */
#define WDG_TIMEOUT_ADJUST      2
#define WDG_FEED_MOMENT_ADJUST  3

/* watchdog register offsets and masks */
#define WDINTRTIME SOC_HI_WatchDog_V500_CNT_INTR_TIME_ADDR(0)
#define WDRSTTIME SOC_HI_WatchDog_V500_CNT_RST_TIME_ADDR(0)
#define WDKICKEN SOC_HI_WatchDog_V500_WD_KICK_ADDR(0)
#define KICKEN 0x00000001
#define LOAD_MIN         0x00000001
#define LOAD_MAX         0xFFFFFFFF
#define WDKICK SOC_HI_WatchDog_V500_WD_KICK_ADDR(0)
#define KICK 0x00000001
#define WDVALUE SOC_HI_WatchDog_V500_WD_CNT_ADDR(0)
#define WDINTMASK SOC_HI_WatchDog_V500_INTR_WD_MASK_ADDR(0)
#define WDRSTMASK SOC_HI_WatchDog_V500_WD_RST_MASK_ADDR(0)
#define WDCNTEN SOC_HI_WatchDog_V500_WD_EN_ADDR(0)
#define WDT_TIMEOUT_KICK (32768 * 3)

/* control register masks */
#define WDINT_ENABLE 0x0
#define WDCNT_ENABLE 0x1
#define WDRST_ENABLE 0x0
#define WDINTCLR SOC_HI_WatchDog_V500_INTR_WD_CLR_ADDR(0)
#define WDRIS SOC_HI_WatchDog_V500_INTR_WD_ADDR(0)
#define INT_MASK 0x1
#define WDLOCKEN SOC_HI_WatchDog_V500_LOCKEN_ADDR(0)
#define WDLOCKDIS SOC_HI_WatchDog_V500_LOCKDIS_ADDR(0)
#define LOCKEN 0x1AAEE533
#define LOCKDIS 0x00000001
#define WDINT_DISABLE 0x1
#define WDRST_DISABLE 0x1
#define WDCNT_DISABLE 0x0

/* wdt aging test, default 120s to be reset */
#define AGING_WDT_TIMEOUT 240

/* jiffies delay mseconds */
#define MSECS_TO_JIFFIES 1000

/* lpm3 message length */
#define M3_MSG_LEN 2
#undef CONFIG_HISI_AGING_WDT
#ifdef CONFIG_HISI_AGING_WDT
static bool disable_wdt_flag = false;
#endif

static struct hiv500_wdt *g_wdt = NULL;
static struct syscore_ops hiv500_wdt_syscore_ops;
void __iomem *wdt_base = NULL;

/*
 * hiv500 wdt device structure
 * @wdd: instance of struct watchdog_device
 * @lock: spin lock protecting dev structure and io access
 * @base: base address of wdt
 * @clk: clock structure of wdt
 * @pdev: platform device structure of wdt
 * @kick_time: kick dog time
 * @dfx_wdt_delayed_work: kick dog work
 * @status: current status of wdt
 * @intr_val: intr value to be set for interupt time
 * @rst_val: rst value to be set for reset time
 * @timeout: current programmed timeout
*/
struct hiv500_wdt {
	struct watchdog_device wdd;
	spinlock_t lock;
	void __iomem *base;
	struct clk *clk;
	struct platform_device *pdev;
	unsigned int intr_val;
	unsigned int rst_val;
	unsigned int kick_time;
	struct delayed_work dfx_wdt_delayed_work;
	struct workqueue_struct *dfx_wdt_wq;
	bool active;
	unsigned int timeout;
};

struct rdr_arctimer_s g_rdr_arctimer_record;

static bool nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, bool, 0);
MODULE_PARM_DESC(nowayout,
		"Set to 1 to keep watchdog running after device release");

/* This routine finds load value that will reset system in required timout */
static int wdt_setload(struct watchdog_device *wdd, unsigned int timeout)
{
	struct hiv500_wdt *wdt = NULL;
	u64 loadintr, loadrst;
	u64 rate;

	if (wdd == NULL)
		return -1;

	wdt = watchdog_get_drvdata(wdd);
	if (wdt == NULL)
		return -1;

	rate = clk_get_rate(wdt->clk);
	if (rate == 0)
		return -1;

	wdt->wdd.timeout = timeout;

	loadintr = div_u64(rate, WDG_TIMEOUT_ADJUST) * timeout - 1;
	loadrst = div_u64(rate, WDG_TIMEOUT_ADJUST) * timeout - 1;

	loadintr = (loadintr > LOAD_MAX) ? LOAD_MAX : loadintr;
	loadrst = (loadrst > LOAD_MAX) ? LOAD_MAX : loadrst;
	loadintr = (loadintr < LOAD_MIN) ? LOAD_MIN : loadintr;
	loadrst = (loadrst < LOAD_MIN) ? LOAD_MIN : loadrst;

	spin_lock(&wdt->lock);
	wdt->intr_val = loadintr;
	wdt->rst_val = loadrst;

	/* roundup timeout to closest positive integer value */
	wdt->timeout = div_u64((loadintr + 1) * WDG_TIMEOUT_ADJUST + (rate / WDG_TIMEOUT_ADJUST), rate);
	spin_unlock(&wdt->lock);
	dev_info(&wdt->pdev->dev, "default-timeout=%u, intr_val=0x%x\n", timeout, wdt->intr_val);

	return 0;
}

/* returns number of seconds left for reset to occur */
static unsigned int wdt_timeleft(struct watchdog_device *wdd)
{
	struct hiv500_wdt *wdt = NULL;
	u64 load;
	u64 rate;
	u64 curvalue;

	if (wdd == NULL)
		return DEFAULT_TIMEOUT;

	wdt = watchdog_get_drvdata(wdd);
	if (wdt == NULL)
		return DEFAULT_TIMEOUT;

	rate = clk_get_rate(wdt->clk);
	if (rate == 0)
		return DEFAULT_TIMEOUT;

	spin_lock(&wdt->lock);
	curvalue = readl_relaxed(wdt->base + WDVALUE);
	load = wdt->intr_val - curvalue;

	/* If the interrupt is inactive then time left is intr_val - curvalue + rst_val. */
	if ((u32)readl_relaxed(wdt->base + WDRIS) == 0)
		load += wdt->rst_val + 1;
	spin_unlock(&wdt->lock);

	return div_u64(load, rate);
}

static int wdt_config(struct watchdog_device *wdd, bool ping)
{
	struct hiv500_wdt *wdt = NULL;
	int ret;

	if (wdd == NULL)
		return -1;

	wdt = watchdog_get_drvdata(wdd);
	if (wdt == NULL)
		return -1;

	if (ping == false) {
		ret = clk_prepare_enable(wdt->clk);
		if (ret) {
			dev_err(&wdt->pdev->dev, "clock enable fail");
			return ret;
		}
	}

	spin_lock(&wdt->lock);

	writel_relaxed(LOCKEN, wdt->base + WDLOCKEN);
	writel_relaxed(wdt->intr_val, wdt->base + WDINTRTIME);
	writel_relaxed(wdt->rst_val, wdt->base + WDRSTTIME);
	writel_relaxed(KICKEN, wdt->base + WDKICKEN);

	if (ping == false) {
		writel_relaxed(INT_MASK, wdt->base + WDINTCLR);
		writel_relaxed(WDINT_ENABLE, wdt->base + WDINTMASK);
		writel_relaxed(WDRST_ENABLE, wdt->base + WDRSTMASK);
		writel_relaxed(WDCNT_ENABLE, wdt->base + WDCNTEN);
		wdt->active = true;
	}

	writel_relaxed(LOCKDIS, wdt->base + WDLOCKDIS);

	/* Flush posted writes. */
	readl_relaxed(wdt->base + WDLOCKEN);
	readl_relaxed(wdt->base + WDLOCKDIS);
	spin_unlock(&wdt->lock);

	return 0;
}

static int wdt_ping(struct watchdog_device *wdd)
{
	struct hiv500_wdt *wdt = NULL;
	int ret;
	if (wdd == NULL)
		return -1;

	wdt = watchdog_get_drvdata(wdd);
	if (wdt == NULL)
		return -1;

	spin_lock(&wdt->lock);

	writel_relaxed(LOCKEN, wdt->base + WDLOCKEN);
	writel_relaxed(KICK, wdt->base + WDKICK);
	writel_relaxed(LOCKDIS, wdt->base + WDLOCKDIS);

	readl_relaxed(wdt->base + WDLOCKEN);
	readl_relaxed(wdt->base + WDLOCKDIS);
	spin_unlock(&wdt->lock);

	return 0;
}

/* enables watchdog timers reset */
static int wdt_enable(struct watchdog_device *wdd)
{
	return wdt_config(wdd, false);
}

/* disables watchdog timers reset */
static int wdt_disable(struct watchdog_device *wdd)
{
	struct hiv500_wdt *wdt = NULL;

	if (wdd == NULL)
		return -1;

	wdt = watchdog_get_drvdata(wdd);
	if (wdt == NULL)
		return -1;

	spin_lock(&wdt->lock);

	writel_relaxed(LOCKEN, wdt->base + WDLOCKEN);
	writel_relaxed(WDINT_DISABLE, wdt->base + WDINTMASK);
	writel_relaxed(WDRST_DISABLE, wdt->base + WDRSTMASK);
	writel_relaxed(WDCNT_DISABLE, wdt->base + WDCNTEN);
	writel_relaxed(LOCKDIS, wdt->base + WDLOCKDIS);

	/* Flush posted writes. */
	readl_relaxed(wdt->base + WDLOCKEN);
	readl_relaxed(wdt->base + WDLOCKDIS);
	wdt->active = false;
	spin_unlock(&wdt->lock);

	clk_disable_unprepare(wdt->clk);

	return 0;
}

static const struct watchdog_info wdt_info = {
	.options = WDIOF_MAGICCLOSE | WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING,
	.identity = "hiv500wdt",
};

static const struct watchdog_ops wdt_ops = {
	.owner          = THIS_MODULE,
	.start          = wdt_enable,
	.stop           = wdt_disable,
	.ping           = wdt_ping,
	.set_timeout    = wdt_setload,
	.get_timeleft   = wdt_timeleft,
};

void watchdog_shutdown_oneshot(unsigned int timeout)
{
	struct hiv500_wdt *wdt = g_wdt;
	unsigned int timeout_rst;
	int ret;

	if (wdt == NULL) {
		pr_err("[%s]wdt device not init\n", __func__);
		return;
	}
	if ((timeout < wdt->kick_time) || (timeout > DEFAULT_TIMEOUT)) {
		pr_err("[%s]timeout invalid[%u, %u]\n", __func__, timeout, wdt->kick_time);
		return;
	}
	cancel_delayed_work_sync(&wdt->dfx_wdt_delayed_work);
	wdt_disable(&wdt->wdd);

	timeout_rst = timeout * WDG_TIMEOUT_ADJUST;
	wdt_setload(&wdt->wdd, timeout_rst);
	ret = wdt_enable(&wdt->wdd);
	if (ret < 0) {
		pr_err("[%s]enable failed\n", __func__);
		return;
	}
	pr_err("[%s]set %u seconds\n", __func__, timeout);
}

unsigned long get_wdt_expires_time(void)
{
	if (g_wdt == NULL)
		return 0;

	return g_wdt->dfx_wdt_delayed_work.timer.expires;
}

unsigned int get_wdt_kick_time(void)
{
	if (g_wdt == NULL)
		return 0;

	return g_wdt->kick_time;
}

#ifdef CONFIG_HISI_AGING_WDT
void set_wdt_disable_flag(bool iflag)
{
	disable_wdt_flag = iflag;
}
#endif

static int hiv500_wdt_rdr_init(void)
{
	struct rdr_exception_info_s einfo;
	unsigned int ret;
	errno_t errno;

	(void)memset_s(&einfo, sizeof(einfo), 0, sizeof(einfo));
	einfo.e_modid = MODID_AP_S_WDT;
	einfo.e_modid_end = MODID_AP_S_WDT;
	einfo.e_process_priority = RDR_ERR;
	einfo.e_reboot_priority = RDR_REBOOT_NOW;
	einfo.e_notify_core_mask = RDR_AP;
	einfo.e_reset_core_mask = RDR_AP;
	einfo.e_from_core = RDR_AP;
	einfo.e_reentrant = (u32)RDR_REENTRANT_DISALLOW;
	einfo.e_exce_type = AP_S_AWDT;
	einfo.e_upload_flag = (u32)RDR_UPLOAD_YES;
	errno = memcpy_s(einfo.e_from_module, sizeof(einfo.e_from_module),
					"ap wdt", sizeof("ap wdt"));
	if (errno != EOK) {
		pr_err("[%s]exception info module memcpy_s failed\n", __func__);
		return -1;
	}
	errno = memcpy_s(einfo.e_desc, sizeof(einfo.e_desc), "ap wdt", sizeof("ap wdt"));
	if (errno != EOK) {
		pr_err("[%s]exception info desc memcpy_s failed\n", __func__);
		return -1;
	}
	ret = rdr_register_exception(&einfo);
	if (ret != MODID_AP_S_WDT)
		return -1;

	return 0;
}

void rdr_arctimer_register_read(struct rdr_arctimer_s *arctimer)
{
	if (arctimer == NULL)
		return;

	arctimer->cntv_ctl_el0 = (u32)read_sysreg(cntv_ctl_el0);
	arctimer->cntv_tval_el0 = (u32)read_sysreg(cntv_tval_el0);
	arctimer->cntp_ctl_el0 = (u32)read_sysreg(cntp_ctl_el0);
	arctimer->cntp_tval_el0 = (u32)read_sysreg(cntp_tval_el0);
	arctimer->cntfrq_el0 = (u32)read_sysreg(cntfrq_el0);

	arctimer->cntv_cval_el0 = read_sysreg(cntv_cval_el0);
	arctimer->cntp_cval_el0 = read_sysreg(cntp_cval_el0);
	arctimer->cntvct_el0 = read_sysreg(cntvct_el0);
	arctimer->cntpct_el0 = read_sysreg(cntpct_el0);
}

void rdr_archtime_register_print(struct rdr_arctimer_s *arctimer, bool after)
{
	if (unlikely(arctimer == NULL))
		return;

	pr_err("<%s>cntv_ctl_el0 0x%x cntv_tval_el0 0x%x cntv_cval_el0 0x%llx cntvct_el0 0x%llx, cntp_ctl_el0 0x%x ",
		(after == true) ? "after" : "before", arctimer->cntv_ctl_el0, arctimer->cntv_tval_el0,
		arctimer->cntv_cval_el0, arctimer->cntvct_el0, arctimer->cntp_ctl_el0);
	pr_err("cntp_tval_el0 0x%x cntp_cval_el0 0x%llx cntpct_el0 0x%llx cntfrq_el0 0x%x\n",
		arctimer->cntp_tval_el0, arctimer->cntp_cval_el0, arctimer->cntpct_el0, arctimer->cntfrq_el0);
}

void dfx_wdt_dump(void)
{
	struct rdr_arctimer_s rdr_arctimer;
	struct hiv500_wdt *wdt = g_wdt;
	struct timer_list *timer = NULL;

	rdr_arctimer_register_read(&rdr_arctimer);

#ifdef CONFIG_SCHED_INFO
	if (current != NULL) {
		pr_crit("current process last_arrival clock %llu last_queued clock %llu, ",
			current->sched_info.last_arrival, current->sched_info.last_queued);
		pr_crit("printk_time is %llu, 32k_abs_timer_value is %llu\n", hisi_getcurtime(), get_32k_abs_timer_value());
	}
#endif

	rdr_archtime_register_print(&g_rdr_arctimer_record, false);
	rdr_archtime_register_print(&rdr_arctimer, true);

	if (wdt == NULL)
		return;

	pr_crit("work_busy 0x%x, latest kick slice %llu\n",
		work_busy(&(wdt->dfx_wdt_delayed_work.work)),
		rdr_get_last_wdt_kick_slice());

	/* check if the watchdog work timer in running state */
	timer = &(wdt->dfx_wdt_delayed_work.timer);
#if (KERNEL_VERSION(4, 9, 76) > LINUX_VERSION_CODE)
	pr_crit("timer 0x%pK: next 0x%pK pprev 0x%pK expires %lu jiffies %lu sec_to_jiffies %lu flags 0x%x slacks %d\n",
		timer, timer->entry.next, timer->entry.pprev, timer->expires,
		jiffies, msecs_to_jiffies(MSECS_TO_JIFFIES), timer->flags, timer->slack);
#else
	pr_crit("timer 0x%pK: next 0x%pK pprev 0x%pK expires %lu jiffies %lu sec_to_jiffies %lu flags 0x%x\n",
		timer, timer->entry.next, timer->entry.pprev, timer->expires,
		jiffies, msecs_to_jiffies(MSECS_TO_JIFFIES), timer->flags);
#endif
}

static void dfx_wdt_mond(struct work_struct *work)
{
	struct hiv500_wdt *wdt = NULL;
	u64 kickslice = get_32k_abs_timer_value();
	struct rq *run_queue = NULL;
	int cpu;

	if (work == NULL)
		return;
	wdt = container_of(work, struct hiv500_wdt, dfx_wdt_delayed_work.work);
#ifdef CONFIG_HISI_AGING_WDT
	if (disable_wdt_flag == true) {
#if (KERNEL_VERSION(4, 9, 76) > LINUX_VERSION_CODE)
		dev_err(wdt->wdd.dev, "disable wdt ok!!!!!\n");
#else
		dev_err(&wdt->pdev->dev, "disable wdt ok!!!!!\n");
#endif
		return;
	}
#endif
	wdt_ping(&wdt->wdd);

	rdr_set_wdt_kick_slice(kickslice);

	cpu = get_cpu();
	run_queue = cpu_rq(cpu);
	put_cpu();

	if (cpu_online(0))
		queue_delayed_work_on(0, wdt->dfx_wdt_wq,
					&wdt->dfx_wdt_delayed_work,
					msecs_to_jiffies(wdt->kick_time * MSECS_TO_JIFFIES));
	else
		queue_delayed_work(wdt->dfx_wdt_wq,
					&wdt->dfx_wdt_delayed_work,
					msecs_to_jiffies(wdt->kick_time * MSECS_TO_JIFFIES));

#if (KERNEL_VERSION(4, 9, 76) > LINUX_VERSION_CODE)
	dev_info(wdt->wdd.dev, "watchdog kick now 32K %llu rqclock %llu\n",
		kickslice, (run_queue ? run_queue->clock : 0));
#else
	dev_info(&wdt->pdev->dev, "watchdog kick now 32K %llu rqclock %llu\n",
		kickslice, (run_queue ? run_queue->clock : 0));
#endif

	watchdog_check_hardlockup_hiwdt();
}

#ifdef CONFIG_HISI_RPROC
static void m3_wdt_timeout_notify(unsigned int default_timeout)
{
	int msg[M3_MSG_LEN] = {0};
	int ret;

/*
 * ask mcu reset its wdt timeout value start
 *
 * Macro definitions of PSCI_MSG_TYPE_M3_WDTTIMEOUT etc. should be same in these files below:
 *
 * kernel\drivers\watchdog\hiv500_wdt.c
 * kernel\drivers\hisi\mntn\blackbox\platform_lpm3\rdr_lpm3.c
 * vendor\hisi\confidential\lpmcu\include\psci.h
 */
#define PSCI_MSG_TYPE_M3_WDTTIMEOUT IPC_CMD(OBJ_AP, OBJ_LPM3, CMD_INQUIRY, 1)
	msg[0] = PSCI_MSG_TYPE_M3_WDTTIMEOUT;
	msg[1] = default_timeout;
	ret = RPROC_ASYNC_SEND(HISI_ACPU_LPM3_MBX_5, (mbox_msg_t *)msg, M3_MSG_LEN);
	if (ret != 0)
		pr_err("RPROC_ASYNC_SEND failed! return 0x%x, msg:[0x%x 0x%x]\n",
			ret, msg[0], msg[1]);
/* ask mcu reset its wdt timeout value end */
}
#else
static inline void m3_wdt_timeout_notify(unsigned int default_timeout)
{
}
#endif

static int hiv500_wdt_get_timeout(struct platform_device *pdev, unsigned int *default_timeout)
{
	struct device_node *np = NULL;
	int ret;

	np = of_find_compatible_node(NULL, NULL, "hisi,wdtv500");
	if (IS_ERR(np)) {
		dev_err(&pdev->dev, "Can not find hiv500 node\n");
		return (-ENOENT);
	}

	ret = of_property_read_u32_index(np, "default-timeout", 0, default_timeout);
	if (ret) {
		dev_warn(&pdev->dev,
			"find default-timeout property fail, Use the default value: %us\n", DEFAULT_TIMEOUT);
		*default_timeout = DEFAULT_TIMEOUT;
	}
	return 0;
}

static int hiv500_wdt_dev_init(struct platform_device *pdev, struct hiv500_wdt **wdt)
{
	int ret = 0;

	if (pdev == NULL)
		return -ENODEV;

	dev_info(&pdev->dev, "[%s] enter\n", __func__);
	*wdt = devm_kzalloc(&pdev->dev, sizeof(**wdt), GFP_KERNEL);
	if ((*wdt) == NULL) {
		dev_warn(&pdev->dev, "Kzalloc failed\n");
		return -ENOMEM;
	}

	(*wdt)->base = devm_ioremap_resource(&pdev->dev, pdev->resource);
	if (IS_ERR((*wdt)->base)) {
		dev_warn(&pdev->dev, "base is error\n");
		ret = -ENOMEM;
		goto wdt_free;
	}

	wdt_base = (*wdt)->base;
	(*wdt)->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR((*wdt)->clk)) {
		dev_warn(&pdev->dev, "Clock not found\n");
		ret = -ENOMEM;
		goto wdt_free;
	}

	(*wdt)->pdev = pdev;
	(*wdt)->wdd.info = &wdt_info;
	(*wdt)->wdd.ops = &wdt_ops;

	spin_lock_init(&(*wdt)->lock);
	watchdog_set_nowayout(&(*wdt)->wdd, nowayout);
	watchdog_set_drvdata(&(*wdt)->wdd, *wdt);

	dev_info(&pdev->dev, "[%s] device init successful\n", __func__);
	return 0;
wdt_free:
	devm_kfree(&pdev->dev, *wdt);
	dev_err(&pdev->dev, "[%s] device init Failed!!!\n", __func__);
	return ret;
}

static int hiv500_wdt_probe(struct platform_device *pdev)
{
	struct hiv500_wdt *wdt = NULL;
	int ret;
	unsigned int default_timeout;

#ifdef CONFIG_HISI_MNTN_PC
	if (check_himntn(HIMNTN_AP_WDT) == 0) {
		dev_err(&pdev->dev, "ap watchdog is closed in nv!!!\n");
		return 0;
	}
#endif
	ret = hiv500_wdt_dev_init(pdev, &wdt);
	if (ret)
		goto probe_fail;

	ret = hiv500_wdt_get_timeout(pdev, &default_timeout);
	if (ret)
		goto probe_fail;

#ifdef CONFIG_HISI_AGING_WDT
	if (check_himntn(HIMNTN_AGING_WDT))
		wdt_setload(&wdt->wdd, AGING_WDT_TIMEOUT);
	else
		wdt_setload(&wdt->wdd, default_timeout);
#else
	wdt_setload(&wdt->wdd, default_timeout);
#endif

	m3_wdt_timeout_notify(default_timeout);

	if ((default_timeout >> 1) < WDG_FEED_MOMENT_ADJUST)
		wdt->kick_time = (default_timeout >> 1) - 1;
	else
		wdt->kick_time = ((default_timeout >> 1) - 1) / WDG_FEED_MOMENT_ADJUST; /* minus 1 from the total */

	wdt_ping(&wdt->wdd);

#ifdef CONFIG_HISI_AGING_WDT
	if (check_himntn(HIMNTN_AGING_WDT))
		set_wdt_disable_flag(true);
#endif

	INIT_DELAYED_WORK(&wdt->dfx_wdt_delayed_work, dfx_wdt_mond);

	wdt->dfx_wdt_wq = alloc_workqueue("wdt_wq", WQ_MEM_RECLAIM | WQ_HIGHPRI, 1);
	if (wdt->dfx_wdt_wq == NULL) {
		dev_err(&pdev->dev, "alloc workqueue failed\n");
		ret = -ENOMEM;
		goto probe_fail;
	}

	ret = watchdog_register_device(&wdt->wdd);
	if (ret) {
		dev_err(&pdev->dev, "watchdog_register_device() failed: %d\n", ret);
		goto workqueue_destroy;
	}

	wdt_enable(&wdt->wdd);
	if (cpu_online(0))
		queue_delayed_work_on(0, wdt->dfx_wdt_wq, &wdt->dfx_wdt_delayed_work, 0);
	else
		queue_delayed_work(wdt->dfx_wdt_wq, &wdt->dfx_wdt_delayed_work, 0);

	g_wdt = wdt;
	register_syscore_ops(&hiv500_wdt_syscore_ops);

	platform_set_drvdata(pdev, wdt);

	if (hiv500_wdt_rdr_init())
		dev_err(&pdev->dev, " register hiv500_wdt_rdr_init failed\n");

	watchdog_set_thresh((int)(default_timeout >> 1));

	dev_info(&pdev->dev, "registration successful\n");
	return 0;

workqueue_destroy:
	if (wdt->dfx_wdt_wq != NULL) {
		destroy_workqueue(wdt->dfx_wdt_wq);
		wdt->dfx_wdt_wq = NULL;
	}
probe_fail:
	dev_err(&pdev->dev, "Probe Failed!!!\n");
	return ret;
}

static int hiv500_wdt_remove(struct platform_device *pdev)
{
	struct hiv500_wdt *wdt = NULL;

	if (pdev == NULL)
		return -1;

	wdt = platform_get_drvdata(pdev);
	if (wdt == NULL)
		return -1;
	cancel_delayed_work(&wdt->dfx_wdt_delayed_work);
	destroy_workqueue(wdt->dfx_wdt_wq);
	wdt->dfx_wdt_wq = NULL;
	watchdog_unregister_device(&wdt->wdd);
	watchdog_set_drvdata(&wdt->wdd, NULL);

	return 0;
}

static int hiv500_wdt_suspend(void)
{
	struct hiv500_wdt *wdt = g_wdt;
	int ret = -1;

	if (wdt == NULL) {
		pr_err("[%s]wdt device not init\n", __func__);
		return ret;
	}
	pr_info("%s+\n", __func__);

	if (watchdog_active(&wdt->wdd) || wdt->active) {
		cancel_delayed_work(&wdt->dfx_wdt_delayed_work);
#ifdef CONFIG_HISI_SR_AP_WATCHDOG
/*
 * delay the disable operation to the lpm3 for the case
 * of system failure in the suspend&resume flow
 */
		spin_lock(&wdt->lock);
		wdt->active = false;
		spin_unlock(&wdt->lock);

		ret = wdt_ping(&wdt->wdd);
#else
		ret = wdt_disable(&wdt->wdd);
#endif
	}

	pr_info("%s-, ret=%d\n", __func__, ret);

	return ret;
}

static void hiv500_wdt_resume(void)
{
	struct hiv500_wdt *wdt = g_wdt;
	int ret = -1;

	if (wdt == NULL) {
		pr_err("[%s]wdt device not init\n", __func__);
		return;
	}
	pr_info("%s+\n", __func__);

	if (watchdog_active(&wdt->wdd) || (wdt->active == false)) {
		ret = wdt_enable(&wdt->wdd);
		if (cpu_online(0))
			queue_delayed_work_on(0, wdt->dfx_wdt_wq, &wdt->dfx_wdt_delayed_work, 0);
		else
			queue_delayed_work(wdt->dfx_wdt_wq, &wdt->dfx_wdt_delayed_work, 0);
	}
	pr_info("%s-, ret=%d\n", __func__, ret);
}

static struct syscore_ops hiv500_wdt_syscore_ops = {
	.suspend = hiv500_wdt_suspend,
	.resume = hiv500_wdt_resume
};

static struct of_device_id of_wdt_match_tab[] = {
	{
		.compatible = "hisi,wdtv500",
	},

	/* end */
	{ }
};

static struct platform_driver platwdt_driver = {
	.driver = {
		   .name = "hiv500wdt",
		   .owner = THIS_MODULE,
		   .of_match_table = of_wdt_match_tab,
		   },
	.probe = hiv500_wdt_probe,
	.remove = hiv500_wdt_remove,
};

static __init int platwdt_init(void)
{
	int ret = 0;
	ret = platform_driver_register(&platwdt_driver);
	if (ret) {
		pr_err("[%s] platform_driver_register failed %d\n",
		       __func__, ret);
	}
	return ret;
}

static void __exit platwdt_exit(void)
{
	platform_driver_unregister(&platwdt_driver);
}

module_init(platwdt_init);
module_exit(platwdt_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
