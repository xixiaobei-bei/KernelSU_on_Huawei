/*
 * rtc module, Provide rtc read and write interface for pmu&soc
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <asm/compiler.h>
#include <linux/compiler.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/reboot.h>
#include <linux/syscalls.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/of.h>
#include <linux/rtc.h>
#include <linux/sizes.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/hisi/hw_cmdline_parse.h>
#include <pmic_interface.h>
#include <linux/version.h>
#include <linux/of_irq.h>
#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>
#include <securec.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#ifdef CONFIG_HISI_RTC_SECURE_FEATURE
#include <linux/mtd/hisi_nve_interface.h>
#define NVE_RTC_NUM 272
#define NVE_NV_RTC_OFFSET_SIZE 4
#endif
#define IRQ_ENABLE 0x1
#define IRQ_DISABLE 0x0
#define RTC_NVE_INIT 0x0
#define RTC_PMU_BULK_BYTE 8
#define RTC_PMU_BULK_INT_LEN 4
#define RTC_PMU_BULK_INIT 0xff
#define RTC_YEAR_LINUX_INIT 1900
#define RTC_MONTH_INIT 1
#define RTC_YEAR_VALID 75
#define PMU_RTC_SET_DELAY 200

#define  SHOW_TIME(time)   \
	pr_err("[%s]: %d-%d-%d %d:%d:%d\n", __func__,\
		(time)->tm_year + RTC_YEAR_LINUX_INIT,\
		(time)->tm_mon + RTC_MONTH_INIT,\
		(time)->tm_mday, (time)->tm_hour,\
		(time)->tm_min, (time)->tm_sec)

/* Register definitions */
#define SOC_RTC_DR 0x00   /* Data read register */
#define SOC_RTC_MR 0x04   /* Match register */
#define SOC_RTC_LR 0x08   /* Data load register */
#define SOC_RTC_CR 0x0c   /* Control register */
#define SOC_RTC_IMSC 0x10 /* Interrupt mask and set register */
#define SOC_RTC_RIS 0x14  /* Raw interrupt status register */
#define SOC_RTC_MIS 0x18  /* Masked interrupt status register */
#define SOC_RTC_ICR 0x1c  /* Interrupt clear register */

#define PMU_RTC_DR 0x00 /* Data read register */
#define PMU_RTC_MR 0x04 /* Match register */
#define PMU_RTC_LR 0x08 /* Data load register */
#define PMU_RTC_CR 0x0c /* Control register */

#define SOC_RTCALARM_INT 0x1
#ifdef CONFIG_HUAWEI_DSM
#define DSM_RTC_BUF_SIZE 256
#define DSM_RTC_PMU_READCOUNT_ERROR_NUM 925005000
#define DSM_RTC_SET_RTC_TMIE_WARNING_NUM 925005001
#endif

struct hisi_rtc_dev {
	struct rtc_device *rtc_dev;

	struct resource *soc_rtc_res;
	void __iomem *soc_rtc_baseaddr;
	unsigned int soc_rtc_irq;

	int pmu_rtc_irq;
	unsigned int pmu_rtc_baseaddr;
	unsigned int pmu_rtc_imr; /* IRQ Mask register */
	unsigned int pmu_rtc_icr; /* IRQ & IRQ Clear register */
	unsigned int pmu_rtc_ib;  /* IRQ Bit in register */

	unsigned int pmu_rtc_xo_thrshd_pwroff_val;
	/* work to notify here's a rtc alram */
	struct work_struct rtc_alarm_work;
#ifdef CONFIG_HISI_RTC_SECURE_FEATURE
	struct work_struct rtc_nv_work; /* work to write offset to nv */
	struct workqueue_struct *nv_work_queue;
	int offset; /* offset of PMU RTC and SOC RTC */
#endif
};

#ifdef CONFIG_HUAWEI_DSM
struct dsm_client *get_rtc_dsm_client(void);
unsigned long lastTime;

#define rtc_dsm_report(err_no, fmt, args...)\
do {\
	if (!get_rtc_dsm_client()) {\
		pr_err("[RTC DSM]rtc dsm client is null\n");\
	} else if (!dsm_client_ocuppy(get_rtc_dsm_client())) {\
		dsm_client_record(get_rtc_dsm_client(), (fmt), ##args);\
		dsm_client_notify(get_rtc_dsm_client(), (err_no));\
		pr_err("[RTC DSM]rtc dsm report err = %d\n", (err_no));\
	} else {\
		pr_err("[RTC DSM]rtc dsm is busy err = %d\n", (err_no));\
	} \
} while (0)

struct dsm_dev dsm_rtc = {
	.name = "dsm_kirin_rtc",
	.device_name = "rtc",
	.fops = NULL,
	.buff_size = DSM_RTC_BUF_SIZE,
};

static struct dsm_client *rtc_dsm_client;

struct dsm_client *get_rtc_dsm_client(void)
{
	return rtc_dsm_client;
}

static void rtc_register_dsm_client(void)
{
	if (rtc_dsm_client)
		return;

	rtc_dsm_client = dsm_register_client(&dsm_rtc);
	if (!rtc_dsm_client)
		pr_err("[RTC DSM]Rtc dsm register failed\n");
}

static int dsm_rtc_valid_tm(struct rtc_time *tm)
{
	if (tm->tm_year < RTC_YEAR_VALID)
		return -EINVAL;

	return 0;
}
#endif

#ifdef CONFIG_HISI_RTC_SECURE_FEATURE
enum pmu_rtc_ops {
	PMU_RTC_ENABLE = 0,
	PMU_RTC_XO_THRESHOLD_SET,
#ifdef CONFIG_HISI_RTC_TEST
	PMU_RTC_TEST,
#endif
};

#define RTC_REGISTER_FN_ID 0xc500ddd0
noinline int atfd_hisi_service_rtc_smc(
	u64 _function_id, u64 _arg0, u64 _arg1, u64 _arg2)
{
	register u64 function_id asm("x0") = _function_id;
	register u64 arg0 asm("x1") = _arg0;
	register u64 arg1 asm("x2") = _arg1;
	register u64 arg2 asm("x3") = _arg2;
	asm volatile(__asmeq("%0", "x0") __asmeq("%1", "x1") __asmeq("%2", "x2")
		__asmeq("%3", "x3") "smc    #0\n" : "+r"(function_id)
		: "r"(arg0), "r"(arg1), "r"(arg2));

	return (int)function_id;
}
#endif

/* extern from kernel source code to avoid intrusion */
extern struct rtc_wkalrm poweroff_rtc_alarm;
static struct hisi_rtc_dev *ldata;

static unsigned int get_pd_charge_flag(void);
static unsigned int pd_charge_flag;

#ifdef CONFIG_HISI_RTC_TEST
static bool rtc_poweroff_alarm_test;
#endif

/*
 * parse powerdown charge cmdline which is passed from fastoot
 * Format : pd_charge=0 or 1
 */
static int __init early_parse_pdcharge_cmdline(char *p)
{
	if (p) {
		if (!strncmp(p, "charger", strlen("charger")))
			pd_charge_flag = 1;
		else
			pd_charge_flag = 0;

		pr_err("power down charge p:%s, pd_charge_flag :%u\n", p,
			pd_charge_flag);
	}

	return 0;
}

early_param("androidboot.mode", early_parse_pdcharge_cmdline);

static unsigned int get_pd_charge_flag(void)
{
	return pd_charge_flag;
}

static inline struct hisi_rtc_dev *hisi_rtc_ldata_get(void)
{
	return (struct hisi_rtc_dev *)ldata;
}

#ifdef CONFIG_HISI_RTC_SECURE_FEATURE
static int hisi_rtc_nve_write(unsigned int index,
	const int *data, unsigned int len)
{
	int ret;
	struct hisi_nve_info_user pinfo;

	pinfo.nv_operation = NV_READ;
	pinfo.nv_number = NVE_RTC_NUM;
	pinfo.valid_size = NVE_NV_RTC_OFFSET_SIZE;

	/*
	 * The dest buffer size is always greater than
	 * the length of the character set to the dest buffer
	 */
	ret = memset_s(pinfo.nv_data, NVE_NV_DATA_SIZE, IRQ_DISABLE,
		(unsigned long)pinfo.valid_size);
	if (ret) {
		pr_err("write hisi rtc nve memset failed!\n");
		return -1;
	}

	ret = hisi_nve_direct_access(&pinfo);
	if (ret) {
		pr_err("rtc Read nve failed!\n");
		return -1;
	}

	pinfo.nv_operation = NV_WRITE;
	ret = memcpy_s(pinfo.nv_data + index, NVE_NV_DATA_SIZE - index,
		data, (unsigned long)len);
	if (ret) {
		pr_err("hisi rtc nve memcpy failed!\n");
		return -1;
	}
	ret = hisi_nve_direct_access(&pinfo);
	if (ret) {
		pr_err("hisi rtc nve write failed!\n");
		return -1;
	}

	return 0;
}

static int hisi_rtc_nve_read(unsigned int index, int *buf,
	unsigned long buffsize, unsigned int len)
{
	int ret;
	struct hisi_nve_info_user pinfo;

	pinfo.nv_operation = NV_READ;
	pinfo.nv_number = NVE_RTC_NUM;
	pinfo.valid_size = NVE_NV_RTC_OFFSET_SIZE;

	/*
	 * The dest buffer size is always greater than
	 * the length of the character set to the dest buffer
	 */
	ret = memset_s(pinfo.nv_data, NVE_NV_DATA_SIZE, IRQ_DISABLE,
		(unsigned long)pinfo.valid_size);
	if (ret) {
		pr_err("read hisi rtc nve memset failed!\n");
		return -1;
	}

	ret = hisi_nve_direct_access(&pinfo);
	if (ret) {
		pr_err("hisi rtc nve read failed!\n");
		return -1;
	}

	ret = memcpy_s(buf, buffsize, pinfo.nv_data + index,
		(unsigned long)len);
	if (ret) {
		pr_err("hisi rtc memcpy nve failed!\n");
		return -1;
	}

	return 0;
}

static void hisi_rtc_nv_write_work(struct work_struct *work)
{
	int ret;
	struct hisi_rtc_dev *s_ldata = hisi_rtc_ldata_get();

	ret = hisi_rtc_nve_write(0, &s_ldata->offset, NVE_NV_RTC_OFFSET_SIZE);
	if (ret)
		pr_err("[%s]write offset to nv failed!\n", __func__);
}
#endif

static void hisi_pmu_rtc_write_bulk(unsigned int base, unsigned long data)
{
	unsigned int value, i;

	for (i = 0; i < RTC_PMU_BULK_INT_LEN; i++) {
		value = (data >> (i * RTC_PMU_BULK_BYTE)) & RTC_PMU_BULK_INIT;
		hisi_pmic_reg_write((base + i), value);
	}
}

static unsigned long hisi_pmu_rtc_read_bulk(unsigned int base)
{
	unsigned long data = 0;
	unsigned int value, i;

	for (i = 0; i < RTC_PMU_BULK_INT_LEN; i++) {
		value = hisi_pmic_reg_read(base + i);
		data |= (value & RTC_PMU_BULK_INIT) << (i * RTC_PMU_BULK_BYTE);
	}

	return data;
}

static void hisi_pmu_rtc_irq_enable(unsigned int enable)
{
	struct irq_desc *desc = NULL;
	struct hisi_rtc_dev *ldata = NULL;

	ldata = hisi_rtc_ldata_get();
	if (!ldata) {
		pr_err("[%s]get rtc ldata failed\n", __func__);
		return;
	}

	desc = irq_to_desc(ldata->pmu_rtc_irq);
	if (!desc) {
		pr_err("%s:desc is NULL!\n", __func__);
		return;
	}

	if (enable == IRQ_ENABLE) {
		if (desc->irq_data.chip->irq_unmask)
			desc->irq_data.chip->irq_unmask(&desc->irq_data);
	} else {
		if (desc->irq_data.chip->irq_mask)
			desc->irq_data.chip->irq_mask(&desc->irq_data);
	}
}

void hisi_pmu_rtc_readtime(struct rtc_time *tm)
{
	unsigned long time;
	struct hisi_rtc_dev *ldata = NULL;
#ifdef CONFIG_HISI_RTC_SECURE_FEATURE
	int ret;
	int offset;
#endif
	if (!tm)
		return;

	ldata = hisi_rtc_ldata_get();
	if (!ldata) {
		pr_err("[%s]get rtc ldata failed\n", __func__);
		return;
	}

	time = hisi_pmu_rtc_read_bulk(ldata->pmu_rtc_baseaddr + PMU_RTC_DR);

#ifdef CONFIG_HISI_RTC_SECURE_FEATURE
	ret = hisi_rtc_nve_read(0, &offset, (unsigned long)sizeof(int),
		NVE_NV_RTC_OFFSET_SIZE);
	if (ret)
		pr_err("[%s]read offset from nv failed\n", __func__);
	time += offset;
#endif

	rtc_time_to_tm(time, tm);
}

unsigned long hisi_pmu_rtc_readcount(void)
{
#ifdef CONFIG_HUAWEI_DSM
	unsigned long time;
#endif
	struct hisi_rtc_dev *data = hisi_rtc_ldata_get();

	if (!data) {
		pr_err("[%s]data is NULL!\n", __func__);
		return 0;
	}
#ifdef CONFIG_HUAWEI_DSM
	time = hisi_pmu_rtc_read_bulk(data->pmu_rtc_baseaddr + PMU_RTC_DR);
	if (lastTime > time) {
		/* DMD RTC PRINT INFO */
		rtc_dsm_report(DSM_RTC_PMU_READCOUNT_ERROR_NUM,
			"Pmu rtc readcount error, time : %lu, lasttime : %lu\n",
			time, lastTime);
	}
	lastTime = time;

	return time;
#else
	return hisi_pmu_rtc_read_bulk(data->pmu_rtc_baseaddr + PMU_RTC_DR);
#endif
}

#ifdef CONFIG_HISI_PMU_RTC_READCOUNT
EXPORT_SYMBOL_GPL(hisi_pmu_rtc_readcount);
#endif

void hisi_pmu_rtc_setalarmtime(unsigned long time)
{
#ifdef CONFIG_HISI_RTC_SECURE_FEATURE
	struct hisi_rtc_dev *ldata = NULL;
	struct rtc_time tm;
	int ret;
	int offset;

	ret = hisi_rtc_nve_read(0, &offset, (unsigned long)sizeof(int),
		NVE_NV_RTC_OFFSET_SIZE);
	if (ret)
		pr_err("[%s]read offset from nv failed\n", __func__);

	ldata = hisi_rtc_ldata_get();
	if (!ldata) {
		pr_err("[%s]get rtc ldata failed\n", __func__);
		return;
	}

	hisi_pmu_rtc_write_bulk(ldata->pmu_rtc_baseaddr + PMU_RTC_MR,
		time - offset);
	atfd_hisi_service_rtc_smc(RTC_REGISTER_FN_ID, PMU_RTC_XO_THRESHOLD_SET,
		ldata->pmu_rtc_xo_thrshd_pwroff_val, 0);
	hisi_pmu_rtc_irq_enable(IRQ_ENABLE);

	rtc_time_to_tm(time, &tm);
	SHOW_TIME(&tm);
#endif
}

int hisi_pmu_rtc_settime(struct rtc_time *tm)
{
	int err;
	unsigned long time;
	struct hisi_rtc_dev *ldata = NULL;

	if (!tm)
		return -1;

	SHOW_TIME(tm);
	err = rtc_valid_tm(tm);
	if (err) {
		pr_err("[%s]Time is invalid\n", __func__);
		return err;
	}
	rtc_tm_to_time(tm, &time);

	ldata = hisi_rtc_ldata_get();
	if (!ldata) {
		pr_err("[%s]get rtc ldata failed\n", __func__);
		return -1;
	}

	hisi_pmu_rtc_write_bulk(ldata->pmu_rtc_baseaddr + PMU_RTC_LR, time);

	return 0;
}

unsigned long hisi_pmu_rtc_readalarmtime(void)
{
	unsigned long time;
	struct hisi_rtc_dev *ldata = NULL;
#ifdef CONFIG_HISI_RTC_SECURE_FEATURE
	int ret;
	int offset;
#endif
	ldata = hisi_rtc_ldata_get();
	if (!ldata) {
		pr_err("[%s]get ldata failed\n", __func__);
		return 0;
	}
	time = hisi_pmu_rtc_read_bulk(ldata->pmu_rtc_baseaddr + PMU_RTC_MR);

#ifdef CONFIG_HISI_RTC_SECURE_FEATURE
	ret = hisi_rtc_nve_read(0, &offset, (unsigned long)sizeof(int),
		NVE_NV_RTC_OFFSET_SIZE);
	if (ret)
		pr_err("[%s]read offset from nv failed\n", __func__);
	time += offset;
#endif
	return time;
}

static int hisi_soc_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	struct hisi_rtc_dev *ldata = NULL;

	if (!dev || !tm)
		return -1;

	ldata = dev_get_drvdata(dev);
	rtc_time_to_tm(readl(ldata->soc_rtc_baseaddr + SOC_RTC_DR), tm);

	return 0;
}

static int hisi_soc_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	int err;
	unsigned long time;
#ifdef CONFIG_HISI_RTC_SECURE_FEATURE
	long pmu_time;
	struct hisi_rtc_dev *s_ldata = hisi_rtc_ldata_get();
#endif
	struct hisi_rtc_dev *ldata = NULL;

	if (!dev || !tm)
		return -1;

	ldata = dev_get_drvdata(dev);

	SHOW_TIME(tm);
	err = rtc_valid_tm(tm);
	if (err) {
		pr_err("[%s]Time is invalid\n", __func__);
		return err;
	}
#ifdef CONFIG_HUAWEI_DSM
	err = dsm_rtc_valid_tm(tm);
	if (err) {
		/* DSM RTC SET TIME ERROR */
		rtc_dsm_report(DSM_RTC_SET_RTC_TMIE_WARNING_NUM,
			"Soc rtc set time error, time : %d-%d-%d-%d-%d-%d\n",
			(tm->tm_year + RTC_YEAR_LINUX_INIT), (tm->tm_mon +
			RTC_MONTH_INIT), tm->tm_mday, tm->tm_hour,
			tm->tm_min, tm->tm_sec);
	}
#endif
	rtc_tm_to_time(tm, &time);

	writel(time, ldata->soc_rtc_baseaddr + SOC_RTC_LR);
/* set pmu rtc */
#ifdef CONFIG_HISI_RTC_SECURE_FEATURE
	pmu_time = hisi_pmu_rtc_read_bulk(ldata->pmu_rtc_baseaddr);
	s_ldata->offset = (int)time - (int)pmu_time;
	queue_work(s_ldata->nv_work_queue, &s_ldata->rtc_nv_work);
#else
	hisi_pmu_rtc_write_bulk(ldata->pmu_rtc_baseaddr + PMU_RTC_LR, time);
#endif
	return 0;
}

static int hisi_soc_rtc_read_alarmtime(struct device *dev,
	struct rtc_wkalrm *alarm)
{
	struct hisi_rtc_dev *ldata = NULL;
	unsigned long time;

	if (!dev || !alarm)
		return -1;

	ldata = dev_get_drvdata(dev);
	time = readl(ldata->soc_rtc_baseaddr + SOC_RTC_MR);

	rtc_time_to_tm(time, &alarm->time);

	alarm->pending = (unsigned int)readl(
		ldata->soc_rtc_baseaddr + SOC_RTC_RIS) & SOC_RTCALARM_INT;
	alarm->enabled = (unsigned int)readl(
		ldata->soc_rtc_baseaddr + SOC_RTC_IMSC) & SOC_RTCALARM_INT;

	return 0;
}

static int hisi_soc_rtc_alarm_irq_enable(struct device *dev,
	unsigned int enabled)
{
	unsigned int imsc;
	struct hisi_rtc_dev *ldata = NULL;

	if (!dev)
		return -1;

	ldata = dev_get_drvdata(dev);

	/* Clear any pending alarm interrupts. */
	writel(SOC_RTCALARM_INT, ldata->soc_rtc_baseaddr + SOC_RTC_ICR);

	imsc = readl(ldata->soc_rtc_baseaddr + SOC_RTC_IMSC);
	if (enabled == 1)
		writel(imsc | SOC_RTCALARM_INT,
			ldata->soc_rtc_baseaddr + SOC_RTC_IMSC);
	else
		writel(imsc & ~SOC_RTCALARM_INT,
			ldata->soc_rtc_baseaddr + SOC_RTC_IMSC);

	return 0;
}

static int hisi_soc_rtc_set_alarmtime(struct device *dev,
	struct rtc_wkalrm *alarm)
{
	struct hisi_rtc_dev *ldata = NULL;
	unsigned long time;
	int ret;

	if (!dev || !alarm)
		return -1;

	ldata = dev_get_drvdata(dev);

	SHOW_TIME(&alarm->time);
	ret = rtc_valid_tm(&alarm->time);
	if (!ret) {
		rtc_tm_to_time(&alarm->time, &time);
		writel(time, ldata->soc_rtc_baseaddr + SOC_RTC_MR);
		hisi_soc_rtc_alarm_irq_enable(dev, alarm->enabled);
	} else {
		pr_err("[%s]Alarm time is invalid\n", __func__);
	}

	return ret;
}

static const struct rtc_class_ops hisi_rtc_ops = {
	.read_time = hisi_soc_rtc_read_time,
	.set_time = hisi_soc_rtc_set_time,
	.read_alarm = hisi_soc_rtc_read_alarmtime,
	.set_alarm = hisi_soc_rtc_set_alarmtime,
	.alarm_irq_enable = hisi_soc_rtc_alarm_irq_enable,
};

static void rtc_alarm_notify_work(struct work_struct *work)
{
	struct hisi_rtc_dev *ldata =
		container_of(work, struct hisi_rtc_dev, rtc_alarm_work);

	pr_err("Entering rtc alarm notify work!\n");
	if ((ldata) && (ldata->rtc_dev) && (&(ldata->rtc_dev->dev)))
		kobject_uevent(&(ldata->rtc_dev->dev.kobj), KOBJ_CHANGE);
}

static irqreturn_t hisi_pmu_rtc_interrupt(int irq, void *dev_id)
{
	pr_err("[%s]RTC Interrupt Comes\n", __func__);

	if (unlikely(get_pd_charge_flag())) {
		schedule_work(&ldata->rtc_alarm_work);
		return IRQ_HANDLED;
	}

	return IRQ_HANDLED;
}

static irqreturn_t hisi_soc_rtc_interrupt(int irq, void *dev_id)
{
	struct hisi_rtc_dev *ldata = dev_id;
	unsigned int rtcmis;
	unsigned long events = 0;

	pr_err("[%s]RTC Interrupt Comes\n", __func__);

	/* read interrupt */
	rtcmis = readl(ldata->soc_rtc_baseaddr + SOC_RTC_MIS);
	if (rtcmis) {
		/* clear interrupt */
		writel(rtcmis, ldata->soc_rtc_baseaddr + SOC_RTC_ICR);
		if (rtcmis & SOC_RTCALARM_INT)
			events |= (RTC_AF | RTC_IRQF);
		rtc_update_irq(ldata->rtc_dev, 1, events);

		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

static void hisi_rtc_shutdown(struct spmi_device *pdev)
{
	unsigned long time;
	struct hisi_rtc_dev *ldata = NULL;

	ldata = dev_get_drvdata(&pdev->dev);
	if (!ldata)
		return;

	if (unlikely(get_pd_charge_flag()))
		return;

#ifdef CONFIG_HISI_RTC_TEST
	if (rtc_poweroff_alarm_test)
		return;
#endif

	if (poweroff_rtc_alarm.enabled) {
		rtc_tm_to_time(&poweroff_rtc_alarm.time, &time);
		hisi_pmu_rtc_setalarmtime(time);
	} else {
		hisi_pmu_rtc_write_bulk(
			ldata->pmu_rtc_baseaddr + PMU_RTC_MR, 0);
	}
}

static int hisi_rtc_remove(struct spmi_device *pdev)
{
	struct hisi_rtc_dev *ldata = NULL;
#ifdef CONFIG_HISI_RTC_SECURE_FEATURE
	struct hisi_rtc_dev *s_ldata = NULL;
#endif

	ldata = dev_get_drvdata(&pdev->dev);
	if (!ldata)
		return -1;

#ifdef CONFIG_HISI_RTC_SECURE_FEATURE
	s_ldata = hisi_rtc_ldata_get();
	if (!s_ldata)
		return -1;

	cancel_work_sync(&s_ldata->rtc_nv_work);
	flush_workqueue(s_ldata->nv_work_queue);
	destroy_workqueue(s_ldata->nv_work_queue);
#endif
	free_irq(ldata->pmu_rtc_irq, ldata->rtc_dev);
	free_irq(ldata->soc_rtc_irq, ldata->rtc_dev);

	dev_set_drvdata(&pdev->dev, NULL);

	rtc_device_unregister(ldata->rtc_dev);
	iounmap(ldata->soc_rtc_baseaddr);

	return 0;
}

static int get_hisi_rtc_irq(struct hisi_rtc_dev *prtc,
	struct device_node *root, struct spmi_device *pdev)
{
	int ret = 0;

	prtc->soc_rtc_irq = irq_of_parse_and_map(root, 0);
	if (!prtc->soc_rtc_irq) {
		pr_err("[%s]get soc_rtc_irq err\n", __func__);
		ret = -ENOENT;
	}

	prtc->pmu_rtc_irq = spmi_get_irq_byname(pdev, NULL, "hisi-pmic-rtc");
	if (prtc->pmu_rtc_irq <= 0) {
		pr_err("[%s]get pmu_rtc_irq err\n", __func__);
		ret = -ENOENT;
	}
	return ret;
}

static int hisi_rtc_get_property(
	struct device_node *np, struct hisi_rtc_dev *prtc)
{
	int ret;

	ret = of_property_read_u32(np, "hisilicon,pmic-rtc-base",
		&prtc->pmu_rtc_baseaddr);
	if (ret) {
		pr_err("[%s]Get hisilicon,pmic-rtc-base failed\n",
			__func__);
		return -ENODEV;
	}

	ret = of_property_read_u32(
		np, "hisilicon,pmic-rtc-imr", &prtc->pmu_rtc_imr);
	if (ret) {
		pr_err("[%s]Get hisilicon,pmic-rtc-imr failed\n",
			__func__);
		return -ENODEV;
	}

	ret = of_property_read_u32(np, "hisilicon,pmic-rtc-icr",
		&prtc->pmu_rtc_icr);
	if (ret) {
		pr_err("[%s]Get hisilicon,pmic-rtc-icr failed\n",
			__func__);
		return -ENODEV;
	}

	ret = of_property_read_u32(np, "hisilicon,pmic-rtc-ib",
		&prtc->pmu_rtc_ib);
	if (ret) {
		pr_err("[%s]Get hisilicon,pmic-rtc-ib failed\n",
			__func__);
		return -ENODEV;
	}

	ret = of_property_read_u32(np,
		"hisilicon,pmic-rtc-xo-thrshd-pwroff-val",
		&prtc->pmu_rtc_xo_thrshd_pwroff_val);
	if (ret) {
		pr_err(
		"[%s]Get hisilicon,pmic-rtc-xo-thrshd-pwroff-val failed\n",
			__func__);
		return -ENODEV;
	}

	return 0;
}

static int hisi_rtc_init_cfg(struct hisi_rtc_dev *prtc,
	struct device *dev, struct spmi_device *pdev)
{
	int ret;
	unsigned int soc_baseaddr;
	struct device_node *root = NULL;
	struct device_node *np = dev->of_node;

	root = of_find_compatible_node(
		pdev->dev.of_node, NULL, "hisilicon,soc-rtc");
	ret = of_property_read_u32(root, "soc-rtc-baseaddr", &soc_baseaddr);
	if (ret) {
		pr_err("[%s]get soc_rtc_baseaddr err\n", __func__);
		return ret;
	}

	prtc->soc_rtc_baseaddr = ioremap(soc_baseaddr, PAGE_SIZE);
	if (!prtc->soc_rtc_baseaddr) {
		pr_err("[%s]base address ioremap fail\n", __func__);
		return -EIO;
	}

	ret = get_hisi_rtc_irq(prtc, root, pdev);
	if (ret) {
		pr_err("[%s]get pmu soc and soc rtc irq failed\n", __func__);
		iounmap(prtc->soc_rtc_baseaddr);
		return ret;
	}

	ret = hisi_rtc_get_property(np, prtc);
	if (ret) {
		pr_err("[%s]rtc_get_property error\n", __func__);
		iounmap(prtc->soc_rtc_baseaddr);
		return ret;
	}

	return 0;
}

static int request_hisi_rtc_irq(struct device *dev, struct hisi_rtc_dev *prtc)
{
	int ret = 0;

	if (devm_request_irq(dev, prtc->pmu_rtc_irq, hisi_pmu_rtc_interrupt,
		IRQF_NO_SUSPEND, "hisi-pmu-rtc", prtc)) {
		pr_err("[%s]request_pmu_rtc_irq error\n", __func__);
		ret = -EIO;
	}

	if (devm_request_irq(dev, prtc->soc_rtc_irq, hisi_soc_rtc_interrupt,
		IRQF_NO_SUSPEND, "hisi-soc-rtc", prtc)) {
		pr_err("[%s]request_soc_rtc_irq error\n", __func__);
		ret = -EIO;
	}
	return ret;
}

static void hisi_rtc_enable(struct hisi_rtc_dev *prtc)
{
#ifndef CONFIG_HISI_RTC_SECURE_FEATURE
	unsigned long time;
	unsigned int reg_value;
#endif

#ifdef CONFIG_HISI_RTC_SECURE_FEATURE
	ldata->nv_work_queue = create_workqueue("rtc_nv_work");
	INIT_WORK(&ldata->rtc_nv_work, hisi_rtc_nv_write_work);
	atfd_hisi_service_rtc_smc(RTC_REGISTER_FN_ID, PMU_RTC_ENABLE, 0, 0);
#else
	reg_value = hisi_pmic_reg_read(prtc->pmu_rtc_baseaddr + PMU_RTC_CR);
	if (!(reg_value & IRQ_ENABLE)) {
		reg_value |= IRQ_ENABLE;
		hisi_pmic_reg_write(
			prtc->pmu_rtc_baseaddr + PMU_RTC_CR, reg_value);
		mdelay(PMU_RTC_SET_DELAY);
	}

	/* enable soc rtc */
	writel(IRQ_ENABLE, prtc->soc_rtc_baseaddr + SOC_RTC_CR);

	/* sync pmu rtc to soc rtc */
	time = hisi_pmu_rtc_read_bulk(prtc->pmu_rtc_baseaddr + PMU_RTC_DR);
	writel(time, prtc->soc_rtc_baseaddr + SOC_RTC_LR);
#endif
}

static int hisi_rtc_probe(struct spmi_device *pdev)
{
	int ret;
	struct hisi_rtc_dev *prtc = NULL;
	struct device *dev = &pdev->dev;

	pr_err("HISI PMU RTC Init Start----------------\n");

	prtc = devm_kzalloc(dev, sizeof(struct hisi_rtc_dev), GFP_KERNEL);
	if (!prtc)
		return -ENOMEM;

	ldata = prtc;
	dev_set_drvdata(dev, prtc);

	ret = hisi_rtc_init_cfg(prtc, dev, pdev);
	if (ret) {
		pr_err("[%s]hisi rtc init failed\n", __func__);
		goto init_cfg_err;
	}

	device_init_wakeup(&pdev->dev, 1);

	prtc->rtc_dev = rtc_device_register(
		"hisi-rtc", &pdev->dev, &hisi_rtc_ops, THIS_MODULE);
	if (IS_ERR(prtc->rtc_dev)) {
		pr_err("[%s]rtc_device_register error\n", __func__);
		ret = PTR_ERR(prtc->rtc_dev);
		goto out_no_rtc;
	}
	ret = request_hisi_rtc_irq(dev, prtc);
	if (ret) {
		pr_err("[%s]request rtc irq failed\n", __func__);
		goto out_no_irq;
	}

/* enable pmu rtc */
	hisi_rtc_enable(prtc);

	INIT_WORK(&ldata->rtc_alarm_work, rtc_alarm_notify_work);

#ifdef CONFIG_HUAWEI_DSM
	rtc_register_dsm_client();
#endif

	pr_err("HISI PMU RTC Init End----------------\n");

	return 0;

out_no_irq:
	rtc_device_unregister(prtc->rtc_dev);

out_no_rtc:
	iounmap(prtc->soc_rtc_baseaddr);

init_cfg_err:
	dev_set_drvdata(dev, NULL);
	return ret;
}

#ifdef CONFIG_PM
static int hisi_rtc_suspend(struct device *dev)
{
	struct hisi_rtc_dev *ldata = NULL;

	ldata = dev_get_drvdata(dev);
	if (!ldata)
		return -1;

	dev_info(&ldata->rtc_dev->dev, "%s: suspend +\n", __func__);

	if (device_may_wakeup(dev) && enable_irq_wake(ldata->soc_rtc_irq))
		dev_err(&ldata->rtc_dev->dev, "%s: failed to enable irq wake\n",
			__func__);

	dev_info(&ldata->rtc_dev->dev, "%s: suspend -\n", __func__);
	return 0;
}

static int hisi_rtc_resume(struct device *dev)
{
	struct hisi_rtc_dev *ldata = NULL;

	ldata = dev_get_drvdata(dev);
	if (!ldata)
		return -1;

	dev_info(&ldata->rtc_dev->dev, "%s: resume +\n", __func__);

	if (device_may_wakeup(dev) && disable_irq_wake(ldata->soc_rtc_irq))
		dev_err(&ldata->rtc_dev->dev,
			"%s: failed to disable irq wake\n", __func__);

	dev_info(&ldata->rtc_dev->dev, "%s: resume -\n", __func__);
	return 0;
}

static SIMPLE_DEV_PM_OPS(hisi_rtc_pm_ops, hisi_rtc_suspend, hisi_rtc_resume);
#define HISI_RTC_PM (&hisi_rtc_pm_ops)
#else
#define HISI_RTC_PM NULL
#endif

static const struct of_device_id hisi_rtc_of_match[] = {
	{ .compatible = "hisilicon-hisi-rtc-spmi" },
	{ }
};

MODULE_DEVICE_TABLE(of, hisi_rtc_of_match);

static const struct spmi_device_id rtc_spmi_id[] = {
	{ "hisilicon-hisi-rtc-spmi", 0 },
	{ }
};

static struct spmi_driver hisi_rtc_driver = {
	.probe = hisi_rtc_probe,
	.remove = hisi_rtc_remove,
	.shutdown = hisi_rtc_shutdown,
	.driver = {
		.name = "hisi-rtc",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_rtc_of_match),
		.pm = HISI_RTC_PM,
	},
	.id_table = rtc_spmi_id,
};

static int __init hisi_rtc_init(void)
{
	return spmi_driver_register(&hisi_rtc_driver);
}

static void __exit hisi_rtc_exit(void)
{
	spmi_driver_unregister(&hisi_rtc_driver);
}

module_init(hisi_rtc_init);
module_exit(hisi_rtc_exit);

#ifdef CONFIG_HISI_RTC_TEST
/* For Test */
#ifndef CONFIG_HISI_RTC_SECURE_FEATURE
#define RTC_TEST_WRITE_PMURTC_TIME 0
#endif
#define RTC_TEST_READ_PMURTC_TIME 1
#define RTC_TEST_ENABLE_PMURTC_ALARM 2
#define RTC_TEST_CANCEL_PMURTC_ALARM 3
#define RTC_TEST_WRITE_SOCRTC_TIME 4
#define RTC_TEST_READ_SOCRTC_TIME 5
#define RTC_TEST_ENABLE_SOCRTC_ALARM 6
#define RTC_TEST_CANCEL_SOCRTC_ALARM 7
#define RTC_TEST_POWEROFF_ALARM 8
#ifdef CONFIG_HISI_RTC_SECURE_FEATURE
#define RTC_TEST_ATF_READ_WRITE 9
#endif

static long hisi_rtc_test_enable_pmurtc_alarm(
	struct hisi_rtc_dev *data, unsigned long wtime)
{
	unsigned long rtime;
	unsigned int reg_val;
	unsigned long now_time;

	rtime = hisi_pmu_rtc_read_bulk(
		data->pmu_rtc_baseaddr + PMU_RTC_DR);
	now_time = rtime + wtime;
	hisi_pmu_rtc_write_bulk(
		data->pmu_rtc_baseaddr + PMU_RTC_MR, now_time);
	reg_val = hisi_pmic_reg_read(data->pmu_rtc_imr);
	reg_val &= ~(IRQ_ENABLE << (data->pmu_rtc_ib));
	hisi_pmic_reg_write(data->pmu_rtc_imr, reg_val);
	return 0;
}

static long hisi_rtc_test_enable_socrtc_alarm(
	struct hisi_rtc_dev *data, unsigned long wtime)
{
	unsigned long rtime;
	unsigned long now_time;

	rtime = readl(data->soc_rtc_baseaddr + SOC_RTC_DR);
	now_time = rtime + wtime;
	writel(now_time, data->soc_rtc_baseaddr + SOC_RTC_MR);
	writel(IRQ_ENABLE, data->soc_rtc_baseaddr + SOC_RTC_IMSC);
	return 0;
}

unsigned long hisi_rtc_test(int cmd, unsigned long wtime)
{
	struct hisi_rtc_dev *data = hisi_rtc_ldata_get();
	unsigned long rtime;
	unsigned int reg_val;

	switch (cmd) {
#ifndef CONFIG_HISI_RTC_SECURE_FEATURE
	case RTC_TEST_WRITE_PMURTC_TIME:
		hisi_pmu_rtc_write_bulk(
			data->pmu_rtc_baseaddr + PMU_RTC_LR, wtime);
		return 0;
#endif
	case RTC_TEST_READ_PMURTC_TIME:
		rtime = hisi_pmu_rtc_read_bulk(
			data->pmu_rtc_baseaddr + PMU_RTC_DR);
		return rtime;
	case RTC_TEST_ENABLE_PMURTC_ALARM:
		return hisi_rtc_test_enable_pmurtc_alarm(data, wtime);
	case RTC_TEST_CANCEL_PMURTC_ALARM:
		reg_val = hisi_pmic_reg_read(data->pmu_rtc_imr);
		reg_val |= (IRQ_ENABLE << (data->pmu_rtc_ib));
		hisi_pmic_reg_write(data->pmu_rtc_imr, reg_val);
		return 0;
	case RTC_TEST_WRITE_SOCRTC_TIME:
		writel(wtime, data->soc_rtc_baseaddr + SOC_RTC_LR);
		return 0;
	case RTC_TEST_READ_SOCRTC_TIME:
		rtime = readl(data->soc_rtc_baseaddr + SOC_RTC_DR);
		return rtime;
	case RTC_TEST_ENABLE_SOCRTC_ALARM:
		return hisi_rtc_test_enable_socrtc_alarm(data, wtime);
	case RTC_TEST_CANCEL_SOCRTC_ALARM:
		writel(RTC_NVE_INIT, data->soc_rtc_baseaddr + SOC_RTC_IMSC);
		return 0;
	case RTC_TEST_POWEROFF_ALARM:
		rtc_poweroff_alarm_test = true;
		/* set rtc_poweroff_alarm_test true to change shutdown flow */
		return 0;
#ifdef CONFIG_HISI_RTC_SECURE_FEATURE
	case RTC_TEST_ATF_READ_WRITE:
		atfd_hisi_service_rtc_smc(
			RTC_REGISTER_FN_ID, PMU_RTC_TEST, 0, 0);
		return 0;
#endif
	default:
		return 0;
	}
}

#endif

MODULE_AUTHOR("Hisilicon Co. Ltd");
MODULE_DESCRIPTION("HISI RTC Driver");
MODULE_LICENSE("GPL");
