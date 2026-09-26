
/*
 * Watchdog driver for ARM SP805 watchdog module
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
#include <asm/sysreg.h>
#include <linux/../../kernel/sched/sched.h>
#include <linux/../../kernel/workqueue_internal.h>
#include <linux/amba/bus.h>
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <pr_log.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/hisi/rdr_hisi_platform.h>
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
#include <linux/resource.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/syscore_ops.h>
#include <linux/version.h>
#ifdef CONFIG_HISI_RPROC
#include <linux/hisi/ipc_msg.h>
#endif
#include <securec.h>
#include <soc_rtctimerwdt_interface.h>

#define PR_LOG_TAG SP805_WDT_TAG

/* default timeout in seconds */
#define DEFAULT_TIMEOUT     60
/*
 * it is used to adjust the time of feeding wdg,
 * defined 3,  So TIMEOUT/2/3 to feed
 */
#define WDG_TIMEOUT_ADJUST      2
#define WDG_FEED_MOMENT_ADJUST  3
#define MODULE_NAME             "sp805-wdt"

#define DFX_WDT_AMBA_ID        0x00141805
#define DFX_WDT_AMBA_MASK      0x00ffffff

/* watchdog register offsets and masks */
#define WDTLOAD          SOC_RTCTIMERWDT_WDLOAD_ADDR(0)
#define LOAD_MIN         0x00000001
#define LOAD_MAX         0xFFFFFFFF
#define WDTVALUE         SOC_RTCTIMERWDT_WDVALUE_ADDR(0)
#define WDTCONTROL       SOC_RTCTIMERWDT_WDCONTROL_ADDR(0)
#define WDT_TIMEOUT_KICK (32768 * 3)

/* control register masks */
#define INT_ENABLE        (1 << SOC_RTCTIMERWDT_WDCONTROL_inten_START)
#define RESET_ENABLE      (1 << SOC_RTCTIMERWDT_WDCONTROL_resen_START)
#define WDTINTCLR         SOC_RTCTIMERWDT_WDINTCLR_ADDR(0)
#define WDTRIS            SOC_RTCTIMERWDT_WDRIS_ADDR(0)
#define INT_MASK          (1 << SOC_RTCTIMERWDT_WDRIS_wdogris_START)
#define WDTLOCK           SOC_RTCTIMERWDT_WDLOCK_ADDR(0)
#define UNLOCK            0x1ACCE551
#define LOCK              0x00000001
#define INT_RESET_DISABLE 0

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
static struct sp805_wdt *g_wdt = NULL;
static struct syscore_ops sp805_wdt_syscore_ops;
void __iomem *wdt_base = NULL;

/*
 * sp805 wdt device structure
 * @wdd: instance of struct watchdog_device
 * @lock: spin lock protecting dev structure and io access
 * @base: base address of wdt
 * @clk: clock structure of wdt
 * @adev: amba device structure of wdt
 * @kick_time: kick dog time
 * @dfx_wdt_delayed_work: kick dog work
 * @status: current status of wdt
 * @load_val: load value to be set for current timeout
 * @timeout: current programmed timeout
 */
struct sp805_wdt {
	struct watchdog_device wdd;
	spinlock_t lock;
	void __iomem *base;
	struct clk *clk;
	struct amba_device *adev;
	unsigned int load_val;
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
	struct sp805_wdt *wdt = NULL;
	u64 load;
	u64 rate;

	if (wdd == NULL)
		return -1;

	wdt = watchdog_get_drvdata(wdd);
	if (wdt == NULL)
		return -1;

	rate = clk_get_rate(wdt->clk);
	if (rate == 0)
		return -1;

	/*
	 * sp805 runs counter with given value twice, after the end of first
	 * counter it gives an interrupt and then starts counter again. If
	 * interrupt already occurred then it resets the system. This is why
	 * load is half of what should be required.
	 */
	wdt->wdd.timeout = timeout;

	load = div_u64(rate, WDG_TIMEOUT_ADJUST) * timeout - 1;

	load = (load > LOAD_MAX) ? LOAD_MAX : load;
	load = (load < LOAD_MIN) ? LOAD_MIN : load;

	spin_lock(&wdt->lock);
	wdt->load_val = load;

	/* roundup timeout to closest positive integer value */
	wdt->timeout = div_u64((load + 1) * WDG_TIMEOUT_ADJUST + (rate / WDG_TIMEOUT_ADJUST), rate);
	spin_unlock(&wdt->lock);
	dev_info(&wdt->adev->dev, "default-timeout=%u, load_val=0x%x\n", timeout, wdt->load_val);

	return 0;
}

/* returns number of seconds left for reset to occur */
static unsigned int wdt_timeleft(struct watchdog_device *wdd)
{
	struct sp805_wdt *wdt = NULL;
	u64 load;
	u64 rate;

	if (wdd == NULL)
		return DEFAULT_TIMEOUT;

	wdt = watchdog_get_drvdata(wdd);
	if (wdt == NULL)
		return DEFAULT_TIMEOUT;

	rate = clk_get_rate(wdt->clk);
	if (rate == 0)
		return DEFAULT_TIMEOUT;

	spin_lock(&wdt->lock);
	load = readl_relaxed(wdt->base + WDTVALUE);

	/* If the interrupt is inactive then time left is WDTValue + WDTLoad. */
	if (((u32)readl_relaxed(wdt->base + WDTRIS) & INT_MASK) == 0)
		load += wdt->load_val + 1;
	spin_unlock(&wdt->lock);

	return div_u64(load, rate);
}

static int wdt_config(struct watchdog_device *wdd, bool ping)
{
	struct sp805_wdt *wdt = NULL;
	int ret;

	if (wdd == NULL)
		return -1;

	wdt = watchdog_get_drvdata(wdd);
	if (wdt == NULL)
		return -1;

	if (ping == false) {
		ret = clk_prepare_enable(wdt->clk);
		if (ret) {
			dev_err(&wdt->adev->dev, "clock enable fail");
			return ret;
		}
	}

	spin_lock(&wdt->lock);

	writel_relaxed(UNLOCK, wdt->base + WDTLOCK);
	writel_relaxed(wdt->load_val, wdt->base + WDTLOAD);

	if (ping == false) {
		writel_relaxed(INT_MASK, wdt->base + WDTINTCLR);
		writel_relaxed(INT_ENABLE | RESET_ENABLE, wdt->base + WDTCONTROL);
		wdt->active = true;
	}

	writel_relaxed(LOCK, wdt->base + WDTLOCK);

	/* Flush posted writes. */
	readl_relaxed(wdt->base + WDTLOCK);
	spin_unlock(&wdt->lock);

	return 0;
}

static int wdt_ping(struct watchdog_device *wdd)
{
	return wdt_config(wdd, true);
}

/* enables watchdog timers reset */
static int wdt_enable(struct watchdog_device *wdd)
{
	return wdt_config(wdd, false);
}

/* disables watchdog timers reset */
static int wdt_disable(struct watchdog_device *wdd)
{
	struct sp805_wdt *wdt = NULL;

	if (wdd == NULL)
		return -1;

	wdt = watchdog_get_drvdata(wdd);
	if (wdt == NULL)
		return -1;

	spin_lock(&wdt->lock);

	writel_relaxed(UNLOCK, wdt->base + WDTLOCK);
	writel_relaxed(INT_RESET_DISABLE, wdt->base + WDTCONTROL);
	writel_relaxed(LOCK, wdt->base + WDTLOCK);

	/* Flush posted writes. */
	readl_relaxed(wdt->base + WDTLOCK);
	wdt->active = false;
	spin_unlock(&wdt->lock);

	clk_disable_unprepare(wdt->clk);

	return 0;
}

static const struct watchdog_info wdt_info = {
	.options = WDIOF_MAGICCLOSE | WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING,
	.identity = MODULE_NAME,
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
	struct sp805_wdt *wdt = g_wdt;
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

static int sp805_wdt_rdr_init(void)
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
	struct sp805_wdt *wdt = g_wdt;
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
	struct sp805_wdt *wdt = NULL;
	u64 kickslice = get_32k_abs_timer_value();
	struct rq *run_queue = NULL;
	int cpu;

	if (work == NULL)
		return;
	wdt = container_of(work, struct sp805_wdt, dfx_wdt_delayed_work.work);
#ifdef CONFIG_HISI_AGING_WDT
	if (disable_wdt_flag == true) {
#if (KERNEL_VERSION(4, 9, 76) > LINUX_VERSION_CODE)
		dev_err(wdt->wdd.dev, "disable wdt ok!!!!!\n");
#else
		dev_err(&wdt->adev->dev, "disable wdt ok!!!!!\n");
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
	dev_info(&wdt->adev->dev, "watchdog kick now 32K %llu rqclock %llu\n",
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

static int sp805_wdt_get_timeout(struct amba_device *adev, unsigned int *default_timeout)
{
	struct device_node *np = NULL;
	int ret;

	np = of_find_compatible_node(NULL, NULL, "arm,sp805");
	if (IS_ERR(np)) {
		dev_err(&adev->dev, "Can not find sp805 node\n");
		return (-ENOENT);
	}

	ret = of_property_read_u32_index(np, "default-timeout", 0, default_timeout);
	if (ret) {
		dev_warn(&adev->dev,
			"find default-timeout property fail, Use the default value: %us\n", DEFAULT_TIMEOUT);
		*default_timeout = DEFAULT_TIMEOUT;
	}
	return 0;
}

static int sp805_wdt_dev_init(struct amba_device *adev, struct sp805_wdt **wdt)
{
	int ret = 0;

	if (adev == NULL)
		return -ENODEV;

	dev_info(&adev->dev, "[%s] enter\n", __func__);
	*wdt = devm_kzalloc(&adev->dev, sizeof(**wdt), GFP_KERNEL);
	if ((*wdt) == NULL) {
		dev_warn(&adev->dev, "Kzalloc failed\n");
		return -ENOMEM;
	}

	(*wdt)->base = devm_ioremap_resource(&adev->dev, &adev->res);
	if (IS_ERR((*wdt)->base)) {
		dev_warn(&adev->dev, "base is error\n");
		ret = -ENOMEM;
		goto wdt_free;
	}

	wdt_base = (*wdt)->base;
	(*wdt)->clk = devm_clk_get(&adev->dev, NULL);
	if (IS_ERR((*wdt)->clk)) {
		dev_warn(&adev->dev, "Clock not found\n");
		ret = -ENOMEM;
		goto wdt_free;
	}

	(*wdt)->adev = adev;
	(*wdt)->wdd.info = &wdt_info;
	(*wdt)->wdd.ops = &wdt_ops;

	spin_lock_init(&(*wdt)->lock);
	watchdog_set_nowayout(&(*wdt)->wdd, nowayout);
	watchdog_set_drvdata(&(*wdt)->wdd, *wdt);

	dev_info(&adev->dev, "[%s] device init successful\n", __func__);
	return 0;
wdt_free:
	devm_kfree(&adev->dev, *wdt);
	dev_err(&adev->dev, "[%s] device init Failed!!!\n", __func__);
	return ret;
}

static int sp805_wdt_probe(struct amba_device *adev, const struct amba_id *id)
{
	struct sp805_wdt *wdt = NULL;
	int ret;
	unsigned int default_timeout;

#ifdef CONFIG_HISI_MNTN_PC
	if (check_himntn(HIMNTN_AP_WDT) == 0) {
		dev_err(&adev->dev, "ap watchdog is closed in nv!!!\n");
		return 0;
	}
#endif
	ret = sp805_wdt_dev_init(adev, &wdt);
	if (ret)
		goto probe_fail;

	ret = sp805_wdt_get_timeout(adev, &default_timeout);
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
		dev_err(&adev->dev, "alloc workqueue failed\n");
		ret = -ENOMEM;
		goto probe_fail;
	}

	ret = watchdog_register_device(&wdt->wdd);
	if (ret) {
		dev_err(&adev->dev, "watchdog_register_device() failed: %d\n", ret);
		goto workqueue_destroy;
	}

	wdt_enable(&wdt->wdd);
	if (cpu_online(0))
		queue_delayed_work_on(0, wdt->dfx_wdt_wq, &wdt->dfx_wdt_delayed_work, 0);
	else
		queue_delayed_work(wdt->dfx_wdt_wq, &wdt->dfx_wdt_delayed_work, 0);

	g_wdt = wdt;
	register_syscore_ops(&sp805_wdt_syscore_ops);

	amba_set_drvdata(adev, wdt);

	if (sp805_wdt_rdr_init())
		dev_err(&adev->dev, " register sp805_wdt_rdr_init failed\n");

	watchdog_set_thresh((int)(default_timeout >> 1));

	dev_info(&adev->dev, "registration successful\n");
	return 0;

workqueue_destroy:
	if (wdt->dfx_wdt_wq != NULL) {
		destroy_workqueue(wdt->dfx_wdt_wq);
		wdt->dfx_wdt_wq = NULL;
	}
probe_fail:
	dev_err(&adev->dev, "Probe Failed!!!\n");
	return ret;
}

static int sp805_wdt_remove(struct amba_device *adev)
{
	struct sp805_wdt *wdt = NULL;

	if (adev == NULL)
		return -1;

	wdt = amba_get_drvdata(adev);
	if (wdt == NULL)
		return -1;
	cancel_delayed_work(&wdt->dfx_wdt_delayed_work);
	destroy_workqueue(wdt->dfx_wdt_wq);
	wdt->dfx_wdt_wq = NULL;
	watchdog_unregister_device(&wdt->wdd);
	watchdog_set_drvdata(&wdt->wdd, NULL);

	return 0;
}


static int sp805_wdt_suspend(void)
{
	struct sp805_wdt *wdt = g_wdt;
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

static void sp805_wdt_resume(void)
{
	struct sp805_wdt *wdt = g_wdt;
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

static struct syscore_ops sp805_wdt_syscore_ops = {
	.suspend = sp805_wdt_suspend,
	.resume = sp805_wdt_resume
};


static struct amba_id sp805_wdt_ids[] = {
	{
		.id = DFX_WDT_AMBA_ID,
		.mask = DFX_WDT_AMBA_MASK,
	}, {
		0, 0
	},
};
MODULE_DEVICE_TABLE(amba, sp805_wdt_ids);

static struct amba_driver sp805_wdt_driver = {
	.drv = {
		.name = MODULE_NAME,
		.pm = NULL,
	},
	.id_table = sp805_wdt_ids,
	.probe = sp805_wdt_probe,
	.remove = sp805_wdt_remove,
};
module_amba_driver(sp805_wdt_driver);

MODULE_LICENSE("GPL v2");
