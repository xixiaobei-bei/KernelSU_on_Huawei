/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * pmic_dcxo_cali.c
 *
 * driver for saving dcxo calibration result
 *
 * This software is licensed under the terms of the GNU General Public
 * either version 2 of that License or (at your option) any later version.
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "pmic_dcxo_cali.h"
#include <linux/mfd/hisi_pmic_dcxo.h>
#include <securec.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/notifier.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/hisi/ipc_msg.h>
#include <linux/hisi/hisi_rproc.h>

struct pmic_dcxo_cali_dev {
	struct device *dev;
	u32 support_proc_ipcmsg;
	void *dcxo_para_msg;
	struct notifier_block pmu_dcxo_nb;
	struct work_struct dcxo_callback_work;
};

static int pmu_dcxo_resp_to_modem(void *msg, int result)
{
	int ret;
	struct cfg_dcxo_para_resp msg_resp;

	ret = memcpy_s(&msg_resp, sizeof(struct cfg_dcxo_para_resp), msg,
		sizeof(struct ipcmsg_header_t));
	if (ret)
		return ret;

	msg_resp.result = result;
	ret = RPROC_ASYNC_SEND(HISI_ACPU_MODEM_MBX_1, (mbox_msg_t *)(&msg_resp),
		sizeof(struct cfg_dcxo_para_resp) / sizeof(u32));
	pr_info("[%s]resp to modem result %d\n", __func__, result);
	return ret;
}

static int pmu_dcxo_proc_c1c2_data(void *msg)
{
	int ret;

	struct cfg_dcxo_para_req *msg_req = (struct cfg_dcxo_para_req *)msg;

	pr_info("type %d, c1 %d, c2 %d\n", msg_req->msg_head.msg_type,
		msg_req->dcxo_c1, msg_req->dcxo_c2);

	ret = pmu_dcxo_set(msg_req->dcxo_c1, msg_req->dcxo_c2);

	return pmu_dcxo_resp_to_modem(msg, ret);
}

static void pmic_dcxo_cp_work(struct work_struct *work)
{
	int ret;
	int idx;
	struct pmic_dcxo_cali_dev *di =
		container_of(work, struct pmic_dcxo_cali_dev, dcxo_callback_work);
	struct ipcmsg_header_t *msg_header =
		(struct ipcmsg_header_t *)di->dcxo_para_msg;
	struct dcxo_msg_proc ipcmsg_func_list[] = {
		{ IPC_MSG_CFG_DCXO_PARA, pmu_dcxo_proc_c1c2_data },
	};

	for (idx = 0; idx < ARRAY_SIZE(ipcmsg_func_list); idx++) {
		if (ipcmsg_func_list[idx].msg_type == msg_header->msg_type) {
			ret = ipcmsg_func_list[idx].msg_proc(di->dcxo_para_msg);
			pr_info("proc msg %d result %d\n", idx, ret);
			return;
		}
	}

	pr_info("[%s] can not proc mess type : %d\n", __func__, msg_header->msg_type);
}

static int pmu_dcxo_recv_notifier(struct notifier_block *nb, unsigned long len, void *msg)
{
	int ret = -1;
	struct pmic_dcxo_cali_dev *di =
		container_of(nb, struct pmic_dcxo_cali_dev, pmu_dcxo_nb);

	if (len == 0 || msg == NULL) {
		pr_err("[%s]no msg\n", __func__);
		return ret;
	}

	di->dcxo_para_msg = msg;
	schedule_work(&di->dcxo_callback_work);

	return ret;
}

static void pmu_dcxo_ipc_recv_register(struct pmic_dcxo_cali_dev *di)
{
	int ret;

	ret = of_property_read_u32(di->dev->of_node, "support-proc-ipcmsg",
		&di->support_proc_ipcmsg);
	if (ret) {
		pr_err("doesn't have support-proc-ipcmsg property!\n");
		return;
	}
	di->pmu_dcxo_nb.next = NULL;
	di->pmu_dcxo_nb.notifier_call = pmu_dcxo_recv_notifier;

	INIT_WORK(&di->dcxo_callback_work, pmic_dcxo_cp_work);

	ret = RPROC_MONITOR_REGISTER(HISI_MODEM_ACPU_MBX_1, &di->pmu_dcxo_nb);
	if (ret)
		pr_err("%s: register callback failed\n", __func__);
	else
		pr_err("%s: register callback succ\n", __func__);
}

static int __init pmic_dcxo_cali_probe(struct platform_device *pdev)
{
	struct pmic_dcxo_cali_dev *di = NULL;

	di = (struct pmic_dcxo_cali_dev *)devm_kzalloc(&pdev->dev,
		sizeof(struct pmic_dcxo_cali_dev), GFP_KERNEL);

	di->dev = &pdev->dev;

	pmu_dcxo_ipc_recv_register(di);
	platform_set_drvdata(pdev, di);
	return 0;
}

static int pmic_dcxo_cali_remove(struct platform_device *pdev)
{
	struct pmic_dcxo_cali_dev *di = platform_get_drvdata(pdev);

	if (!di) {
		pr_err("[%s]di is NULL!\n", __func__);
		return -ENODEV;
	}

	dev_set_drvdata(&pdev->dev, NULL);
	return 0;
}

const static struct of_device_id pmic_dcxo_cali_match_table[] = {
	{
		.compatible = "hisilicon,pmic_dcxo_cali",
	},
	{ },
};

static struct platform_driver pmic_dcxo_cali_driver = {
	.probe = pmic_dcxo_cali_probe,
	.remove = pmic_dcxo_cali_remove,
	.driver = {
		.name = "pmic_dcxo_cali",
		.owner = THIS_MODULE,
		.of_match_table = pmic_dcxo_cali_match_table,
	},
};

int __init pmic_dcxo_cali_init(void)
{
	return platform_driver_register(&pmic_dcxo_cali_driver);
}

void __exit pmic_dcxo_cali_exit(void)
{
	platform_driver_unregister(&pmic_dcxo_cali_driver);
}

late_initcall(pmic_dcxo_cali_init);
module_exit(pmic_dcxo_cali_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("pmic dcxo calibration module");
