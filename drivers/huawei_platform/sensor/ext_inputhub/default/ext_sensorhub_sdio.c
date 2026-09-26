/*
 * ext_sensorhub_sdio.c
 *
 * code for external sensorhub sdio driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "ext_sensorhub_sdio.h"

#include <linux/mmc/host.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sdio_func.h>
#include <linux/module.h>
#include <asm/memory.h>
#include <linux/slab.h>

#include "securec.h"
#include "ext_sensorhub_commu.h"

#define HISDIO_FUNC1_INT_DREADY (1 << 0) /* data ready interrupt */
#define HISDIO_FUNC1_INT_RERROR (1 << 1) /* data read error interrupt */
#define HISDIO_FUNC1_INT_MFARM  (1 << 2) /* ARM Msg interrupt */
#define HISDIO_FUNC1_INT_ACK	(1 << 3) /* ACK interrupt */
#define HISDIO_REG_FUNC1_FIFO   0x00 /* Read Write FIFO */
#define HISDIO_FUNC1_INT_MASK \
(HISDIO_FUNC1_INT_DREADY | HISDIO_FUNC1_INT_RERROR | HISDIO_FUNC1_INT_MFARM)
#define HISDIO_REG_FUNC1_INT_STATUS	0x08 /* interrupt mask and clear reg */
#define HISDIO_REG_FUNC1_INT_ENABLE	0x09 /* interrupt */
#define HISDIO_EXTEND_BASE_ADDR		0x30
#define HISDIO_ENABLE_TIMEOUT		1000
#define HISDIO_CARD_DETECT_TIMEOUT	15
#define HISDIO_REG_FUNC1_XFER_COUNT	0x0c /* notify number of bytes */

struct sdio_extend_func {
	u32 int_stat;
	u32 msg_stat;
	u32 xfer_count;
};

static struct mmc_host *sdio_mmc_host;
static  struct completion  sdio_driver_complete;
static bool g_sdio_initial;
static bool g_sdio_resume;

int ext_sensorhub_mmc_sdio_dev_init(void)
{
	struct sdio_func *func = NULL;
	int ret;

	pr_info("%s start ++\n", __func__);
	if (!sdio_mmc_host) {
		pr_err("%s, mmc host is null\n", __func__);
		return -1;
	}

	mmc_claim_host(sdio_mmc_host);
	if (!sdio_mmc_host->card) {
		pr_err("%s host->card is null\n", __func__);
		goto err;
	}
	func = sdio_mmc_host->card->sdio_func[0];
	func->enable_timeout = HISDIO_ENABLE_TIMEOUT;
	ret = sdio_enable_func(func);
	if (ret < 0) {
		pr_err("mmc sdio_enable_func err ret = %d\n", ret);
		goto err;
	} else {
		pr_info("mmc sdio_enable_func success\n");
	}

	ret = sdio_set_block_size(func, HISDIO_BLOCK_SIZE);
	if (ret) {
		pr_err("sdio_set_block_size error ret = %d\n", ret);
		goto err;
	}

	sdio_writeb(func, HISDIO_FUNC1_INT_MASK,
		    HISDIO_REG_FUNC1_INT_STATUS, &ret);
	if (ret) {
		pr_err("failed to clear sdio interrupt!  ret = %d\n", ret);
		goto err;
	} else {
		pr_info("clear sdio interrupt\n");
	}

	sdio_writeb(func, HISDIO_FUNC1_INT_MASK,
		    HISDIO_REG_FUNC1_INT_ENABLE, &ret);
	if (ret) {
		pr_err("failed to enable sdio interrupt!  ret = %d\n", ret);
		goto err;
	}
	g_sdio_initial = true;
err:
	mmc_release_host(sdio_mmc_host);
	pr_info("%s end --\n", __func__);
	return ret;
}

static void ext_sensorhub_sdio_reinit(void)
{
	int ret;

	pr_info("%s start ++\n", __func__);
	if (!sdio_mmc_host) {
		pr_err("%s, mmc host is null\n", __func__);
		return;
	}
	mmc_claim_host(sdio_mmc_host);

	ret = mmc_power_save_host(sdio_mmc_host);
	sdio_mmc_host->pm_flags &= ~MMC_PM_KEEP_POWER;
	ret = mmc_power_restore_host(sdio_mmc_host);
	sdio_mmc_host->pm_flags |= MMC_PM_KEEP_POWER;
	if (ret >= 0) {
		pr_info("%s success, begin dev init", __func__);
		ext_sensorhub_mmc_sdio_dev_init();
	} else {
		pr_err("%s failed", __func__);
	}

	mmc_release_host(sdio_mmc_host);
	pr_info("%s end --\n", __func__);
}

void ext_sensorhub_detect_sdio_card(void)
{
	if (sdio_mmc_host) {
		ext_sensorhub_sdio_reinit();
		pr_info("%s, mmc host has already detect", __func__);
		return;
	}
	/* detect card */
	dw_mci_1135_card_detect_change();

	if (wait_for_completion_timeout(&sdio_driver_complete,
					HISDIO_CARD_DETECT_TIMEOUT * HZ)) {
		pr_info("ext_sensorhub sdio load sucuess, sdio enum done.\n");
		ext_sensorhub_mmc_sdio_dev_init();
	} else {
		pr_err("ext_sensorhub sdio enum timeout, reason[probe timeout]\n");
	}
}

int ext_sensorhub_mmc_sdio_dev_write(u8 *buf, u32 len)
{
	struct sdio_func *func = NULL;
	int ret;

	if (!sdio_mmc_host) {
		pr_err("%s, mmc host is null\n", __func__);
		return -1;
	}

	if (g_sdio_initial) {
		pr_debug("ext_sdio has already init");
	} else {
		pr_debug("ext_sdio need to init");
		ext_sensorhub_mmc_sdio_dev_init();
	}

	if (!g_sdio_resume) {
		pr_err("%s sdio is in suspend\n", __func__);
		return -EACCES;
	}
	mmc_claim_host(sdio_mmc_host);
	if (!sdio_mmc_host->card) {
		pr_err("%s host->card is null\n", __func__);
		goto err;
	}
	func = sdio_mmc_host->card->sdio_func[0];

	ret = sdio_writesb(func, HISDIO_REG_FUNC1_FIFO, buf, len);
	if (ret < 0)
		pr_err("sdio_writesb error ret = %d\n", ret);

err:
	mmc_release_host(sdio_mmc_host);
	return ret;
}

int ext_sensorhub_mmc_sdio_dev_read(u8 *buf, u32 len)
{
	struct sdio_func *func = NULL;
	int ret;

	if (!sdio_mmc_host) {
		pr_err("%s, mmc host is null\n", __func__);
		return -1;
	}

	if (g_sdio_initial) {
		pr_debug("ext_sdio has already init");
	} else {
		pr_debug("ext_sdio need to init");
		ext_sensorhub_mmc_sdio_dev_init();
	}

	if (!g_sdio_resume) {
		pr_err("%s sdio is in suspend\n", __func__);
		return -EACCES;
	}
	mmc_claim_host(sdio_mmc_host);
	if (!sdio_mmc_host->card) {
		pr_err("%s host->card is null\n", __func__);
		goto err;
	}
	func = sdio_mmc_host->card->sdio_func[0];
	memset_s(buf, len, 0, len);
	ret = sdio_readsb(func, buf, HISDIO_REG_FUNC1_FIFO, len);
	if (ret < 0) {
		pr_err("sdio_readsb ret = %d\n", ret);
		goto err;
	}

err:
	mmc_release_host(sdio_mmc_host);
	return ret;
}

int ext_sensorhub_mmc_sdio_get_xfercount(u32 *xfercount)
{
	struct sdio_func *func = NULL;
	struct sdio_extend_func *extend_func = NULL;
	int ret;

	if (!sdio_mmc_host) {
		pr_err("%s, mmc host is null\n", __func__);
		return -1;
	}
	if (g_sdio_initial) {
		pr_debug("ext_sdio has already init");
	} else {
		pr_debug("ext_sdio need to init");
		ext_sensorhub_mmc_sdio_dev_init();
	}
	if (!g_sdio_resume) {
		pr_err("%s sdio is in suspend\n", __func__);
		return -EACCES;
	}
	mmc_claim_host(sdio_mmc_host);
	if (!sdio_mmc_host->card) {
		pr_err("%s host->card is null\n", __func__);
		ret = -EINVAL;
		goto err;
	}
	extend_func = kmalloc(sizeof(*extend_func), GFP_KERNEL);
	if (!extend_func) {
		ret = -ENOMEM;
		goto err;
	}
	func = sdio_mmc_host->card->sdio_func[0];
	ret = sdio_memcpy_fromio(func, extend_func, HISDIO_EXTEND_BASE_ADDR,
				 sizeof(struct sdio_extend_func));
	if (ret != 0) {
		pr_err("%s read int mask error: %d", __func__, ret);
		ret = -EFAULT;
		goto err;
	}

	if (extend_func->int_stat & HISDIO_FUNC1_INT_DREADY) {
		ret = 0;
		*xfercount = extend_func->xfer_count;
	} else {
		ret = -EINVAL;
		pr_err("%s read int mask data is not ready", __func__);
	}
err:
	kfree(extend_func);
	mmc_release_host(sdio_mmc_host);
	return ret;
}

/* driver func */
static int ext_sensorhub_sdio_probe(struct sdio_func *func,
				    const struct sdio_device_id *ids)
{
	pr_info("%s start ++ ext_sdio, begin to get host.\n", __func__);

	if (!func || !func->card) {
		/* param error */
		pr_info("%s param invalid\n", __func__);
		return -EFAULT;
	}

	/* get host from sdio func */
	sdio_mmc_host = func->card->host;
	if (!sdio_mmc_host) {
		pr_err("%s, sdio_mmc_host is null.\n", __func__);
		return -EFAULT;
	}

	complete(&sdio_driver_complete);
	pr_info("%s ext_sdio, sdio driver probec complete\n", __func__);
		return 0;
}

/* do nothing in remove function */
static void ext_sensorhub_sdio_remove(struct sdio_func *func)
{
}

/* do nothing in shutdown function */
void ext_sensorhub_sdio_dev_shutdown(struct device *dev)
{
}

/* do nothing in suspend function  */
static int ext_sensorhub_sdio_suspend(struct device *dev)
{
	pr_info("%s ext_sdio, get in suspend\n", __func__);
	g_sdio_resume = false;
	commu_suspend(SDIO_DRIVER);

	return 0;
}

/* do nothing in resume function  */
static int ext_sensorhub_sdio_resume(struct device *dev)
{
	pr_info("%s ext_sdio, get in resume\n", __func__);
	g_sdio_resume = true;
	commu_resume(SDIO_DRIVER);

	return 0;
}

static struct sdio_device_id const ext_sensorhub_sdio_ids[] = {
	{ SDIO_DEVICE(HISDIO_VENDOR_ID_HISI, HISDIO_PRODUCT_ID_HISI) },
	{},
};
MODULE_DEVICE_TABLE(sdio, ext_sensorhub_sdio_ids);

static const struct dev_pm_ops ext_sensorhub_sdio_pm_ops = {
	.suspend = ext_sensorhub_sdio_suspend,
	.resume = ext_sensorhub_sdio_resume,
};

static  struct sdio_driver ext_sensorhub_sdio_driver = {
	.name = "ext_sensorhub_sdio",
	.id_table = ext_sensorhub_sdio_ids,
	.probe = ext_sensorhub_sdio_probe,
	.remove	= ext_sensorhub_sdio_remove,
	.drv = {
		.owner = THIS_MODULE,
		.pm = &ext_sensorhub_sdio_pm_ops,
		.shutdown = ext_sensorhub_sdio_dev_shutdown,
	}
};

int ext_sensorhub_sdio_func_probe(void)
{
	int ret;

	pr_info("%s start ++, begin register sdio driver.\n", __func__);
	init_completion(&sdio_driver_complete);
	g_sdio_resume = true;
	ret = sdio_register_driver(&ext_sensorhub_sdio_driver);

	if (ret) {
		pr_err("register sdio driver Failed ret=%d\n", ret);
		return ret;
	}
	pr_info("register sdio driver Success ret=%d\n", ret);

	return ret;
}

void ext_sensorhub_sdio_exit(void)
{
	sdio_unregister_driver(&ext_sensorhub_sdio_driver);
}
