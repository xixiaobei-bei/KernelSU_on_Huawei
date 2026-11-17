/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: stat io latency scatter at driver level, show it in kernel log
 * and host attr node.
 * Author: chenqi
 * Create: 2020-09-04
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

#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_host.h>
#include <linux/iomt_host/dsm_iomt_host.h>

#define UFS_SECTOR_TO_KB		4
#define IOMT_UFS_OPCODE_BIT		0
#define IOMT_UFS_READ_CMD		0x28
#define IOMT_UFS_WRITE_CMD		0x2A
#define IOMT_UFS_SYNC_CMD		0x35
#define IOMT_UFS_UNMAP_CMD		0x42
#define IOMT_UFS_GET_CHUNKSIZE_OFFSIZE	12
#define IOMT_UFS_HOST_BLK_SIZE		512

static unsigned char iomt_host_cmd_dir(struct scsi_cmnd *cmd)
{
	unsigned char cmd_type;

	switch ((int)cmd->cmnd[IOMT_UFS_OPCODE_BIT]) {
	case IOMT_UFS_READ_CMD:
		cmd_type = IOMT_DIR_READ;
		break;
	case IOMT_UFS_WRITE_CMD:
		cmd_type = IOMT_DIR_WRITE;
		break;
	case IOMT_UFS_UNMAP_CMD:
		cmd_type = IOMT_DIR_UNMAP;
		break;
	case IOMT_UFS_SYNC_CMD:
		cmd_type = IOMT_DIR_SYNC;
		break;
	default:
		cmd_type = IOMT_DIR_OTHER;
		break;
	}
	return cmd_type;
}

void iomt_host_latency_cmd_start(struct scsi_cmnd *cmd, ktime_t s_time)
{
	if (cmd == NULL || s_time == 0)
		return;
	cmd->iomt_start_time.ktime = s_time;
}

void iomt_host_latency_cmd_end(struct Scsi_Host *scsi_host,
	struct scsi_cmnd *cmd)
{
	struct iomt_host_info *iomt_host_info = NULL;
	unsigned char cmd_type;
	unsigned int chunk_size;

	if ((scsi_host->iomt_host_info == NULL) || (cmd == NULL))
		return;

	/* Caculate cmd operation data length, unit is KB */
	chunk_size  = (cmd->request->__data_len >> IOMT_UFS_GET_CHUNKSIZE_OFFSIZE)
		* UFS_SECTOR_TO_KB;

	iomt_host_info = iomt_info_from_host(scsi_host);
	cmd_type = iomt_host_cmd_dir(cmd);

	iomt_host_io_latency(iomt_host_info, &(cmd->iomt_start_time), cmd_type, chunk_size);
}

static void iomt_ufs_io_timeout_dsm_action(
	struct Scsi_Host *host)
{
	return;
}

static void *ufs_host_to_iomt(const void *scsi_host)
{
	return (void *)(((struct Scsi_Host *)scsi_host)->iomt_host_info);
}

static ssize_t io_latency_ufs_show_checker(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct Scsi_Host *host = class_to_shost(dev);

	if (host->iomt_host_info != NULL)
		return io_latency_scatter_show(
			(struct iomt_host_info *)host->iomt_host_info,
			buf);
	return 0;
}

static ssize_t io_latency_ufs_store_checker(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct Scsi_Host *host = class_to_shost(dev);

	if (host->iomt_host_info != NULL)
		return io_latency_scatter_store(
			(struct iomt_host_info *)host->iomt_host_info,
			buf, count);
	return -EINVAL;
}

static DEVICE_ATTR(io_latency_scatter, 0644,
	io_latency_ufs_show_checker,
	io_latency_ufs_store_checker);

static void ufs_host_create_latency_attr(void *iomt_host_info)
{
	struct Scsi_Host * host = NULL;

	struct iomt_host_latency_stat *latency_scatter =
		&(((struct iomt_host_info *)iomt_host_info)->latency_scatter);

	host = (struct Scsi_Host *)latency_scatter->host;
	if (host == NULL) {
		pr_err("%s:Invalid host info\n", __func__);
		return;
	}

	if (device_create_file(&host->shost_dev, &dev_attr_io_latency_scatter))
		dev_err(&host->shost_dev, "Failed to create io_latency_scatter sysfs entry\n");

	/* Creat iomt/io_latency_scatter under /proc directory */
	io_latency_creat_proc_entry(iomt_host_info);
}
static void ufs_host_delete_latency_attr(void *iomt_host_info)
{
	struct Scsi_Host * host = NULL;

	struct iomt_host_latency_stat *latency_scatter =
		&(((struct iomt_host_info *)iomt_host_info)->latency_scatter);

	host = (struct Scsi_Host *)latency_scatter->host;

	if (host == NULL) {
		pr_err("%s:Invalid host info\n", __func__);
		return;
	}

	device_remove_file(&host->shost_dev, &dev_attr_io_latency_scatter);
	/* Remove /proc/iomt/io_latency_scatter */
	io_latency_proc_remove();
}

static void ufs_host_io_timeout_dsm(void *iomt_host_info)
{
	struct Scsi_Host * host = NULL;

	struct iomt_host_latency_stat *latency_scatter =
		&(((struct iomt_host_info *)iomt_host_info)->latency_scatter);

	host = (struct Scsi_Host *)latency_scatter->host;

	if (host == NULL) {
		pr_err("%s:Invalid host info\n", __func__);
		return;
	}

	iomt_ufs_io_timeout_dsm_action(host);
}

static char *ufs_host_name(const void *host)
{
	return (char *)dev_name(&((struct Scsi_Host *)host)->shost_dev);
}

static unsigned int ufs_host_blk_size(void *host)
{
	return IOMT_UFS_HOST_BLK_SIZE;
}

static const struct iomt_host_ops iomt_ufs_host_ops = {
	.host_to_iomt_func = ufs_host_to_iomt,
	.host_name_func = ufs_host_name,
	.create_latency_attr_func = ufs_host_create_latency_attr,
	.delete_latency_attr_func = ufs_host_delete_latency_attr,
	.dsm_func = ufs_host_io_timeout_dsm,
	.host_blk_size_fun = ufs_host_blk_size,
};

void dsm_iomt_ufs_host_init(struct Scsi_Host *host)
{
	dsm_iomt_host_init((void *)host, &iomt_ufs_host_ops);
}

void dsm_iomt_ufs_host_exit(const struct Scsi_Host *host)
{
	dsm_iomt_host_exit((const void *)host, &iomt_ufs_host_ops);
}

