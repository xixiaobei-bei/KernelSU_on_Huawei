/*
 * hkip_atkinfo.c
 *
 * Huawei HKIP driver to upload attack process information.
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#include "hkip_atkinfo.h"
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/arm-smccc.h>
#include <linux/compiler.h>
#include <linux/io.h>
#include <chipset_common/security/saudit.h>

static struct hkip_atkinfo *g_atkinfo;
unsigned int disable_upload;

#define MODULE_NAME          "hkip-atkinfo"
#define MAX_UPLOAD_TIMES     3
#define HKIP_ATKINFO_ENABLE  1
#define HKIP_ATKINFO_DISABLE 0

static int msg_channel_callback(unsigned int len, void *buf)
{
	(void)len;
	(void)buf;

	if (!g_atkinfo)
		return -EINVAL;

	pr_err("hkip called by irq triiger.\n");
	if (!disable_upload)
		queue_delayed_work(g_atkinfo->wq_atkinfo,
				   &g_atkinfo->atkinfo_work, 0);
	return 0;
}

static int verify_event_hdr(struct hhee_event_header *hdr)
{
	if (!hdr)
		return -EINVAL;

	if (hdr->magic != HHEE_EVENT_MAGIC)
		return -EINVAL;

	if (hdr->buffer_size != (hdr->footer_offset + PAGE_SIZE))
		return -EINVAL;
	return 0;
}

static int upload_events_kill_atkprocess(struct hkip_atkinfo *atkinfo)
{
	struct hhee_event_header *hdr = atkinfo->header;
	struct hhee_event_footer *ftr = atkinfo->footer;
	uint64_t start;
	uint64_t end;
	uint64_t i;
	uint32_t j;
	int ret = 0;

	if (!ftr || !hdr) {
		pr_err("kill atkprocess parameter null\n");
		return -EINVAL;
	}
	start = ftr->read_offset;
	end = hdr->write_offset;

	if (start >= end) {
		pr_err("%s: Stop to upload, Wrong offset %llu:%llu\n",
		       MODULE_NAME, start, end);
		/* if write_offset overflow, will reset read_offset */
		ftr->read_offset = hdr->write_offset;
		return -EINVAL;
	}

	if (end > (start + MAX_UPLOAD_TIMES)) {
		/*
		 * if the number of uploaded events is more than 3,
		 * we just upload 3 events.
		 */
		end = start + MAX_UPLOAD_TIMES;
		pr_warn("%s: upload events is more than 3\n", MODULE_NAME);
	}

	pr_err("%s: uploading the attack events %llu:%llu\n",
		MODULE_NAME, start, end);

	for (i = start; i < end; i++) {
		if (!hdr->buffer_capacity)
			return -EINVAL;
		j = i % hdr->buffer_capacity;

		saudit_log(HKIP, STP_RISK, 0, "event=%d,type=%d,ctxid=%d,",
			j, hdr->events[j].type, hdr->events[j].ctx_id);

		/* modify read_offset once finished to upload events */
		ftr->read_offset++;
		pr_info("%s: event[%d] type:%d/ts[j].typpid:%d succ\n",
			MODULE_NAME, j,
			hdr->events[j].type, hdr->events[j].ctx_id);
	}

	return ret;
}

/* A worker to upload the events reported from HKIP */
static void upload_atkinfo_worker(struct work_struct *work)
{
	int ret;
	struct hkip_atkinfo *atkinfo = container_of(work, struct hkip_atkinfo,
						    atkinfo_work.work);

	if (verify_event_hdr(atkinfo->header)) {
		pr_err("%s: it's a wrong atk event from hkip\n", MODULE_NAME);
		return;
	}
	pr_err("%s: got a attack event from HKIP\n", MODULE_NAME);
	mutex_lock(&atkinfo->atkinfo_mtx);
	dsb(ish);

	ret = upload_events_kill_atkprocess(atkinfo);
	if (ret) {
		pr_err("%s: fail to upload the events\n", MODULE_NAME);
		mutex_unlock(&atkinfo->atkinfo_mtx);
		return;
	}

	mutex_unlock(&atkinfo->atkinfo_mtx);
}

/* This function will reset hkip counter each 24hours */
static inline void clear_hkip_monitorlog_reset_counters(void)
{
	hkip_clean_counters();
}

static void clear_hkip_counter_timer(uintptr_t data)
{
	struct hkip_atkinfo *atkinfo = (struct hkip_atkinfo *)data;

	if (!atkinfo)
		return;

	pr_info("%s: reset hkip counter.\n", MODULE_NAME);
	clear_hkip_monitorlog_reset_counters();

	mod_timer(&atkinfo->timer, jiffies +
		msecs_to_jiffies((const unsigned int)atkinfo->cycle_time));
}

/* Check if has unprocessed events when boot up device */
static void upload_unread_events(struct hkip_atkinfo *atkinfo)
{
	struct hhee_event_header *hdr = atkinfo->header;
	struct hhee_event_footer *ftr = atkinfo->footer;

	if (!ftr || !hdr) {
		pr_err("unread events parameter null\n");
		return;
	}

	if (verify_event_hdr(hdr)) {
		pr_err("%s: it's a wrong atk event from hkip\n", MODULE_NAME);
		return;
	}

	mutex_lock(&atkinfo->atkinfo_mtx);

	if (hdr->write_offset > ftr->read_offset) {
		pr_err("%s: Found unread hkip events %llu:%llu\n",
			MODULE_NAME, hdr->write_offset, ftr->read_offset);
		/* Time out setting, 8 seconds */
		queue_delayed_work(atkinfo->wq_atkinfo,
			&atkinfo->atkinfo_work, msecs_to_jiffies(8000));
	}

	mutex_unlock(&atkinfo->atkinfo_mtx);
}

int hkip_atkinfo_check_enable(void)
{
	unsigned int dts_enable = 0;
	int ret;
	struct device_node *np = of_find_compatible_node(NULL, NULL, ATKINFO_DTS);

	if (!np && !of_find_property(np, "hkip_atkinfo_enable", NULL)) {
		pr_err("%s: no hkip-atkinfo node found\n", MODULE_NAME);
		return HKIP_ATKINFO_DISABLE;
	}

	ret = of_property_read_u32(np, "hkip_atkinfo_enable", &dts_enable);
	if (ret) {
		pr_err("%s: failed to find node", MODULE_NAME);
		return HKIP_ATKINFO_DISABLE;
	}

	if (dts_enable != HKIP_ATKINFO_ENABLE) {
		pr_err("%s: dts_enable is %d\n", MODULE_NAME, dts_enable);
		return HKIP_ATKINFO_DISABLE;
	}

	return HKIP_ATKINFO_ENABLE;
}

static inline void hkip_hhee_monitorlog_info(struct arm_smccc_res *res)
{
	arm_smccc_hvc((unsigned long)HHEE_MONITORLOG_INFO,
		      0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, res);
}

static int set_atkinfo_data(struct platform_device *pdev,
			    struct hkip_atkinfo *atkinfo)
{
	int ret;

	if (!pdev || !atkinfo) {
		pr_err("atkinfo_int parameter null\n");
		return -EINVAL;
	}
	atkinfo->wq_atkinfo = create_singlethread_workqueue(MODULE_NAME);
	if (!atkinfo->wq_atkinfo) {
		pr_err("%s: create workqueue fail\n", MODULE_NAME);
		return -EINVAL;
	}

	mutex_init(&atkinfo->atkinfo_mtx);
	INIT_DELAYED_WORK(&atkinfo->atkinfo_work, upload_atkinfo_worker);

	ret = hhee_msg_register_handler(HHEE_MSG_ID_HKIP, msg_channel_callback);
	if (ret) {
		pr_err("%s: open HKIP message channel error\n", MODULE_NAME);
		return -EINVAL;
	}

	upload_unread_events(atkinfo);

	ret = of_property_read_u32_index(pdev->dev.of_node,
		"cycle-time", 0, &atkinfo->cycle_time);
	if (ret) {
		pr_err("%s: can't get cycle-time from dts\n", MODULE_NAME);
		return -EINVAL;
	}

	init_timer(&atkinfo->timer);
	atkinfo->timer.function = clear_hkip_counter_timer;
	atkinfo->timer.expires =
		msecs_to_jiffies((const unsigned int)atkinfo->cycle_time);
	atkinfo->timer.data = (uintptr_t)atkinfo;
	add_timer(&atkinfo->timer);

	platform_set_drvdata(pdev, atkinfo);
	g_atkinfo = atkinfo;

	ret = atkinfo_create_debugfs(atkinfo);
	if (ret) {
		pr_err("%s: fail to create debugfs\n", MODULE_NAME);
		return -EINVAL;
	}
	return 0;
}

static int hkip_atkinfo_probe(struct platform_device *pdev)
{
	int ret;
	struct hkip_atkinfo *atkinfo = NULL;
	struct arm_smccc_res res;
	void __iomem *base = NULL;

	if (hkip_atkinfo_check_enable() == HKIP_ATKINFO_DISABLE ||
	    hhee_check_enable() == HHEE_DISABLE)
		return 0;

	if (!pdev)
		return -EINVAL;

	atkinfo = devm_kzalloc(&pdev->dev, sizeof(*atkinfo), GFP_KERNEL);
	if (!atkinfo)
		return -ENOMEM;

	hkip_hhee_monitorlog_info(&res);
	if (!res.a0 || !res.a1 || (res.a1 & (PAGE_SIZE - 1))) {
		pr_err("%s: get a wrong event buffer parameter\n", MODULE_NAME);
		ret = -EINVAL;
		goto error;
	}

	base = ioremap_cache(res.a0, res.a1);
	if (!base) {
		pr_err("%s: failed to ioremap event buffer\n", MODULE_NAME);
		ret = -ENXIO;
		goto error;
	}

	pr_info("%s: ioremap base 0x%lx, size 0x%lx\n",
		MODULE_NAME, res.a0, res.a1);

	atkinfo->header = (struct hhee_event_header *)base;
	/* footer is at the beginning of the last full page */
	atkinfo->header->footer_offset = res.a1 - PAGE_SIZE;
	atkinfo->footer = (struct hhee_event_footer *)(base +
		atkinfo->header->footer_offset);

	ret = set_atkinfo_data(pdev, atkinfo);
	if (ret != 0) {
		ret = -EINVAL;
		goto err;
	}

	return 0;
err:
	iounmap(base);
error:
	devm_kfree(&pdev->dev, atkinfo);
	return ret;
}

static int hkip_atkinfo_remove(struct platform_device *pdev)
{
	struct hkip_atkinfo *atkinfo = NULL;

	if (!pdev)
		return -EINVAL;

	atkinfo = platform_get_drvdata(pdev);
	if (!atkinfo)
		return -ENODEV;

	cancel_delayed_work_sync(&atkinfo->atkinfo_work);
	(void)del_timer(&atkinfo->timer);
	iounmap((void __iomem *)atkinfo->header);
	devm_kfree(&pdev->dev, atkinfo);

	return 0;
}

static const struct of_device_id hkip_atkinfo_of_table[] = {
	{ .compatible = ATKINFO_DTS },
	{ },
};

static struct platform_driver hkip_atkinfo_driver = {
	.probe  = hkip_atkinfo_probe,
	.remove = hkip_atkinfo_remove,
	.driver = {
		.name           = MODULE_NAME,
		.of_match_table = of_match_ptr(hkip_atkinfo_of_table),
	},
};

static int __init hkip_atkinfo_init(void)
{
	int ret;

	ret = platform_driver_register(&hkip_atkinfo_driver);
	if (ret)
		pr_err("%s: register hkip atkinfo driver fail\n", MODULE_NAME);

	return ret;
}

static void __exit hkip_atkinfo_exit(void)
{
	platform_driver_unregister(&hkip_atkinfo_driver);
}

late_initcall(hkip_atkinfo_init);
module_exit(hkip_atkinfo_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HKIP ATKINFO UPLOAD Driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
