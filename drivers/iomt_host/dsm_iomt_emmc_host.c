/*
 * file_name
 * dsm_iomt_host_emmc.c
 * description
 * stat io latency scatter at driver level,
 * show it in kernel log and host attr node.
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include <linux/mmc/host.h>
#ifdef CONFIG_HUAWEI_EMMC_DSM
#include <linux/mmc/dsm_emmc.h>
#endif
#include <linux/iomt_host/dsm_iomt_host.h>

#define iomt_cls_dev_to_mmc_host(d)	\
	container_of(d, struct mmc_host, class_dev)

static void iomt_mmc_io_timeout_dsm_action(
		struct mmc_host *host)
{
	unsigned int op_arg;
	unsigned int block_ticks;
	struct iomt_host_info *iomt_host_info;

#ifdef CONFIG_HUAWEI_EMMC_DSM
	if (host != NULL && host->card != NULL &&
		host->iomt_host_info != NULL &&
		!mmc_card_is_removable(host)) {
		iomt_host_info = iomt_info_from_host(host);
		op_arg = iomt_host_info->io_timeout_dsm.op_arg;
		block_ticks = iomt_host_info->io_timeout_dsm.block_ticks;
		DSM_EMMC_LOG(host->card, DSM_EMMC_IO_TIMEOUT,
			"%s:%s(manfid:0x%x) emmc io timeout, judge slot = %u, "
			"opcode = %u, arg = 0x%x, blocks = %u, ticks = %u, dir = %u",
			__func__, mmc_hostname(host),
			host->card->cid.manfid,
			iomt_host_info->io_timeout_dsm.judge_slot,
			op_arg >> IOMT_OP_BLOCK_SHIFT,
			(unsigned int)((unsigned short)op_arg),
			block_ticks >> IOMT_OP_BLOCK_SHIFT,
			(unsigned int)((unsigned short)block_ticks),
			iomt_host_info->io_timeout_dsm.data_dir);

	}
#endif
}

static void *mmc_host_to_iomt(void *mmc_host)
{
	return (void *)(((struct mmc_host *)mmc_host)->iomt_host_info);
}

static ssize_t io_latency_mmc_show_checker(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mmc_host *host = iomt_cls_dev_to_mmc_host(dev);

	if (host->iomt_host_info != NULL)
		return io_latency_scatter_show(
			(struct iomt_host_info *)host->iomt_host_info,
			buf);
	return 0;
}

static ssize_t io_latency_mmc_store_checker(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct mmc_host *host = iomt_cls_dev_to_mmc_host(dev);

	if (host->iomt_host_info != NULL)
		return io_latency_scatter_store(
			(struct iomt_host_info *)host->iomt_host_info,
			buf, count);
	return -EINVAL;
}

static DEVICE_ATTR(io_latency_scatter, 0644,
			io_latency_mmc_show_checker,
			io_latency_mmc_store_checker);

static void mmc_host_create_latency_attr(void *iomt_host_info)
{
	struct mmc_host *host = NULL;

	struct iomt_host_latency_stat *latency_scatter =
		&(((struct iomt_host_info *)iomt_host_info)->latency_scatter);

	host = (struct mmc_host *)latency_scatter->host;

	if (host == NULL) {
		pr_err("%s:Invalid host info\n", __func__);
		return;
	}

	if (device_create_file(&host->class_dev, &dev_attr_io_latency_scatter))
		dev_err(&host->class_dev, "Failed to create io_latency_scatter sysfs entry\n");
}

static void mmc_host_delete_latency_attr(void *iomt_host_info)
{
	struct mmc_host *host = NULL;

	struct iomt_host_latency_stat *latency_scatter =
		&(((struct iomt_host_info *)iomt_host_info)->latency_scatter);

	host = (struct mmc_host *)latency_scatter->host;

	if (host == NULL) {
		pr_err("%s:Invalid host info\n", __func__);
		return;
	}

	device_remove_file(&host->class_dev, &dev_attr_io_latency_scatter);
}

static void mmc_host_io_timeout_dsm(void *iomt_host_info)
{
	struct mmc_host *host = NULL;

	struct iomt_host_latency_stat *latency_scatter =
		&(((struct iomt_host_info *)iomt_host_info)->latency_scatter);

	host = (struct mmc_host *)latency_scatter->host;

	if (host == NULL) {
		pr_err("%s:Invalid host info\n", __func__);
		return;
	}

	iomt_mmc_io_timeout_dsm_action(host);
}

static char *mmc_host_name(void *host)
{
	return (char*)mmc_hostname((struct mmc_host *)host);
}

static unsigned int mmc_host_blk_size(void *host)
{
	return  MMC_CARD_CMDQ_BLK_SIZE;
}

static ssize_t rw_size_mmc_show_checker(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mmc_host *host = iomt_cls_dev_to_mmc_host(dev);

	if (host->iomt_host_info != NULL)
		return rw_size_scatter_show(
			(struct iomt_host_info *)host->iomt_host_info,
			buf);
	return 0;
}

static DEVICE_ATTR(rw_size_scatter, 0644,
			rw_size_mmc_show_checker,
			NULL);

static void mmc_host_create_rw_size_attr(void *iomt_host_info)
{
	struct mmc_host *host = NULL;

	struct iomt_host_rw_size_stat *rw_size_scatter =
		&(((struct iomt_host_info *)iomt_host_info)->rw_size_scatter);

	host = (struct mmc_host *)rw_size_scatter->host;

	if (host == NULL) {
		pr_err("%s:Invalid host info\n", __func__);
		return;
	}

	if (device_create_file(&host->class_dev, &dev_attr_rw_size_scatter))
		dev_err(&host->class_dev, "Failed to create rw_size_scatter sysfs entry\n");
}

static void mmc_host_delete_rw_size_attr(void *iomt_host_info)
{
	struct mmc_host *host = NULL;

	struct iomt_host_rw_size_stat *rw_size_scatter =
		&(((struct iomt_host_info *)iomt_host_info)->rw_size_scatter);

	host = (struct mmc_host *)rw_size_scatter->host;

	if (host == NULL) {
		pr_err("%s:Invalid host info\n", __func__);
		return;
	}

	device_remove_file(&host->class_dev, &dev_attr_rw_size_scatter);
}

static const struct iomt_host_ops iomt_emmc_host_ops = {
	.host_to_iomt_func = mmc_host_to_iomt,
	.host_name_func = mmc_host_name,
	.create_latency_attr_func = mmc_host_create_latency_attr,
	.delete_latency_attr_func = mmc_host_delete_latency_attr,
	.dsm_func = mmc_host_io_timeout_dsm,
	.host_blk_size_fun = mmc_host_blk_size,
	.create_rw_size_attr_func = mmc_host_create_rw_size_attr,
	.delete_rw_size_attr_func = mmc_host_delete_rw_size_attr,
};

void dsm_iomt_mmc_host_init(struct mmc_host *host)
{
	dsm_iomt_host_init((void *)host, &iomt_emmc_host_ops);
}

void dsm_iomt_mmc_host_exit(struct mmc_host *host)
{
	dsm_iomt_host_exit((void *)host, &iomt_emmc_host_ops);
}

