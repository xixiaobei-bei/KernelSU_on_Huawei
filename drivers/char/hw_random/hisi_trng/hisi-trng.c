/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: get trng datas from cc engine and push them to random pool.
 * Author: security-ap
 * Create: 2019/09/20
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/uaccess.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/hw_random.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/random.h>
#include <linux/slab.h>
#include "teek_client_api.h"
#include "teek_client_id.h"
#include "load_image_external.h"

/* max length for one time */
#define TRNG_BUF_SIZE 4096
/*
 * when current_quality set to 128, 'rc * current_quality * 8 >> 10'
 * is equal to 'rc' in file core.c.
 */
#define CURRENT_QUALITY 128

struct hisi_trng_buf {
	void *start_locate;
	void *current_locate;
	size_t max_len;
	size_t remaining_len;
};

static int hisi_trng_init(struct hwrng *rng)
{
	struct hisi_trng_buf *trng_buf =
		(struct hisi_trng_buf *)(uintptr_t)rng->priv;
	if (!trng_buf || !trng_buf->start_locate) {
		pr_err("%s: init failed! rng->priv error!\n", __func__);
		return -1;
	}

	trng_buf->current_locate = trng_buf->start_locate;
	trng_buf->max_len = TRNG_BUF_SIZE;
	trng_buf->remaining_len = 0;

	return 0;
}

static void hisi_trng_cleanup(struct hwrng *rng)
{
	struct hisi_trng_buf *trng_buf =
		(struct hisi_trng_buf *)(uintptr_t)rng->priv;

	if (trng_buf && trng_buf->start_locate)
		kfree(trng_buf->start_locate);
	trng_buf->start_locate = NULL;

	if (!trng_buf)
		kfree(trng_buf);
	trng_buf = NULL;

	rng->priv = 0;
}

static int get_rng_data(void *random_buf, size_t random_size)
{
	int ret;
	TEEC_Operation operation = {0};
	u32 origin = 0;
	TEEC_Result result;
	TEEC_Session g_session;
	TEEC_Context g_context;

	if (random_size < 0) {
		pr_err("error! random_size is a invalid param.\n");
		return -1;
	}

	ret = teek_init(&g_session, &g_context);
	if (ret != 0) {
		pr_err("%s: teek_init failed!\n", __func__);
		return -1;
	}

	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(
		TEEC_MEMREF_TEMP_INOUT,
		TEEC_NONE,
		TEEC_NONE,
		TEEC_NONE);
	operation.params[0].tmpref.buffer = random_buf;
	operation.params[0].tmpref.size = random_size;

	result = TEEK_InvokeCommand(
		&g_session,
		SECBOOT_CMD_ID_GET_RNG_NUM,
		&operation,
		&origin);
	if (result != TEEC_SUCCESS) {
		pr_err("%s: TEEK_InvokeCommand failed!\n", __func__);
		return -1;
	}

	TEEK_CloseSession(&g_session);
	TEEK_FinalizeContext(&g_context);

	return 0;
}

static int hisi_trng_read(struct hwrng *rng, void *buf, size_t size, bool wait)
{
	int ret;
	u32 *data = buf;
	struct hisi_trng_buf *trng_buf =
		(struct hisi_trng_buf *)(uintptr_t)rng->priv;

	if (!trng_buf || !trng_buf->start_locate || !trng_buf->current_locate) {
		pr_err("error! the given rng param is wrong!\n");
		return -1;
	}

	if (size > TRNG_BUF_SIZE) {
		pr_err("error! random buf size is too large.\n");
		return -1;
	}

	if (trng_buf->remaining_len < size) {
		ret = get_rng_data(trng_buf->start_locate, trng_buf->max_len);
		if (ret != 0) {
			pr_err("error! get random fail or no data available!\n");
			return -1;
		}
		trng_buf->current_locate = trng_buf->start_locate;
		trng_buf->remaining_len = trng_buf->max_len;
	}

	(void)memcpy(data, trng_buf->current_locate, size);

	trng_buf->current_locate += size;
	trng_buf->remaining_len -= size;

	/* return the random quantity obtained */
	return size;
}

static int hisi_trng_probe(struct platform_device *pdev)
{
	struct hwrng *rng;
	struct hisi_trng_buf *trng_buf = NULL;
	int ret;

	rng = devm_kzalloc(&pdev->dev, sizeof(struct hwrng), GFP_KERNEL);
	if (!rng)
		return -ENOMEM;

	trng_buf = (struct hisi_trng_buf *)kzalloc(sizeof(struct hisi_trng_buf),
			GFP_KERNEL);
	if (!trng_buf) {
		pr_err("%s: fail to alloc memory for trng_buf!\n", __func__);
		return -1;
	}
	trng_buf->start_locate = kzalloc(TRNG_BUF_SIZE, GFP_KERNEL);
	if (!trng_buf->start_locate) {
		pr_err("%s: fail to alloc memory for start_locate!\n", __func__);
		kfree(trng_buf);
		return -1;
	}

	platform_set_drvdata(pdev, rng);

	rng->priv = (unsigned long)(uintptr_t)trng_buf;
	rng->quality = CURRENT_QUALITY;
	rng->name = pdev->name;
	rng->init = hisi_trng_init;
	rng->cleanup = hisi_trng_cleanup;
	rng->read = hisi_trng_read;

	ret = devm_hwrng_register(&pdev->dev, rng);
	if (ret) {
		dev_err(&pdev->dev, "failed to register hw_trng\n");
		kfree(trng_buf);
		kfree(trng_buf->start_locate);
		return ret;
	}

	return 0;
}

static const struct of_device_id hisi_trng_dt_ids[] = {
	{ .compatible = "hisilicon,tee-trng" },
	{ }
};
MODULE_DEVICE_TABLE(of, hisi_trng_dt_ids);

static struct platform_driver hisi_trng_driver = {
	.probe		= hisi_trng_probe,
	.driver		= {
		.name	= "hisi-trng",
		.of_match_table = of_match_ptr(hisi_trng_dt_ids),
	},
};

module_platform_driver(hisi_trng_driver);

MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_AUTHOR("security-ap");
MODULE_DESCRIPTION("Hisilicon hardware random number generator driver");
