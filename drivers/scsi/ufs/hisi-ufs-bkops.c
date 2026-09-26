/*
 * hisi-ufs-bkops.c
 *
 * ufs bkops hynix
 *
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

#include "hisi-ufs-bkops.h"

#include <linux/mas_bkops_core.h>
#include <linux/version.h>

#include "hisi_ufs_bkops_interface.h"
#include <scsi/scsi_host.h>
#include "ufs.h"
#include "ufshcd.h"
#include "ufs-quirks.h"

#ifdef CONFIG_MAS_UNISTORE_PRESERVE
#include <linux/blkdev.h>
#endif

#define QUERY_REQ_RETRIES 3
#define QUERY_REQ_TIMEOUT 1300 /* msec */

#define UFS_BKOPS_1861_FLAG                                                    \
		(BKOPS_CHK_TIME_INTERVAL |                          \
		BKOPS_CHK_ACCU_WRITE |                              \
		BKOPS_CHK_ACCU_DISCARD)

#define UFS_BKOPS_HYNIX_FLAG                                                   \
		(BKOPS_CHK_TIME_INTERVAL |                          \
		BKOPS_CHK_ACCU_WRITE |                              \
		BKOPS_CHK_ACCU_DISCARD)

#ifdef CONFIG_MAS_DEBUG_FS
static int mas_bkops_send_stop;
void mas_bkops_send_stop_enable(void)
{
	mas_bkops_send_stop = 1;
}

void mas_bkops_send_stop_disable(void)
{
	mas_bkops_send_stop = 0;
}
#endif /* CONFIG_MAS_DEBUG_FS */

static int mas_bkops_upiu_query_opcode(struct ufs_query_req *request,
					struct ufs_hba *hba,
					enum query_opcode opcode,
					u32 *bkops_status)
{
	int err;

	switch (opcode) {
	case UPIU_QUERY_OPCODE_M_GC_START:
		/* fall-through */
	case UPIU_QUERY_OPCODE_M_GC_STOP:
		request->query_func = UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST;
		break;
	case UPIU_QUERY_OPCODE_M_GC_CHECK:
		request->query_func = UPIU_QUERY_FUNC_STANDARD_READ_REQUEST;
		if (!bkops_status) {
			dev_err(hba->dev,
			    "%s: Invalid argument for read request\n",
			    __func__);
			err = -EINVAL;
			return err;
		}
		break;
	default:
		dev_err(hba->dev,
		    "%s: Expected query bkops opcode but got = %d\n",
		    __func__, opcode);
		err = -EINVAL;
		return err;
	}

	return 0;
}

static int ufshcd_query_hi1861_bkops(struct ufs_hba *hba,
	enum query_opcode opcode, u32 *bkops_status)
{
	struct ufs_query_req *request = NULL;
	struct ufs_query_res *response = NULL;
	int err;
	unsigned long flags;

	BUG_ON(!hba);

	spin_lock_irqsave(hba->host->host_lock, flags);
	if (hba->ufshcd_state != UFSHCD_STATE_OPERATIONAL) {
		spin_unlock_irqrestore(hba->host->host_lock, flags);
		return -EBUSY;
	}
	spin_unlock_irqrestore(hba->host->host_lock, flags);

	/* hold error handler, reset may cause send command a noc error */
	mutex_lock(&hba->eh_mutex);
	mutex_lock(&hba->dev_cmd.lock);

	ufshcd_init_query(hba, &request, &response, opcode, 0, 0, 0);

	err = mas_bkops_upiu_query_opcode(request, hba, opcode, bkops_status);
	if (err)
		goto out_unlock;

	err = ufshcd_exec_dev_cmd(hba, DEV_CMD_TYPE_QUERY, QUERY_REQ_TIMEOUT);
	if (err) {
		dev_err(hba->dev, "%s: Sending bkops query failed, err = %d\n",
		    __func__, err);
		goto out_unlock;
	}

	if (bkops_status)
		*bkops_status =
			be32_to_cpu(response->upiu_res.reserved[0]) & 0xFF;

out_unlock:
	mutex_unlock(&hba->dev_cmd.lock);
	mutex_unlock(&hba->eh_mutex);
	return err;
}

static int ufshcd_1861_bkops_query(struct ufs_hba *hba, u32 *status)
{
	if (!status)
		return -EINVAL;

	return ufshcd_query_hi1861_bkops(
		hba, UPIU_QUERY_OPCODE_M_GC_CHECK, status);
}

static int ufshcd_1861_bkops_start(struct ufs_hba *hba)
{
	return ufshcd_query_hi1861_bkops(
		hba, UPIU_QUERY_OPCODE_M_GC_START, NULL);
}

static int ufshcd_1861_bkops_stop(struct ufs_hba *hba)
{
	return ufshcd_query_hi1861_bkops(
		hba, UPIU_QUERY_OPCODE_M_GC_STOP, NULL);
}

static struct ufs_dev_bkops_ops hi1861_bkops_ops = {
	.ufs_bkops_query = ufshcd_1861_bkops_query,
	.ufs_bkops_start = ufshcd_1861_bkops_start,
	.ufs_bkops_stop = ufshcd_1861_bkops_stop,
};

static int ufshcd_hynix_bkops_query(struct ufs_hba *hba, u32 *status)
{
	if (!status)
		return -EINVAL;

	return ufshcd_query_attr_safe(hba, UPIU_QUERY_OPCODE_READ_ATTR,
	    QUERY_ATTR_IDN_M_GC_STATUS, 0, 0, status);
}

static int ufshcd_hynix_bkops_start(struct ufs_hba *hba)
{
	u32 val = 1;

	return ufshcd_query_attr_safe(hba, UPIU_QUERY_OPCODE_WRITE_ATTR,
	    QUERY_ATTR_IDN_M_GC_START_STOP, 0, 0, &val);
}

static int ufshcd_hynix_bkops_stop(struct ufs_hba *hba)
{
	u32 val = 0;

	return ufshcd_query_attr_safe(hba, UPIU_QUERY_OPCODE_WRITE_ATTR,
	    QUERY_ATTR_IDN_M_GC_START_STOP, 0, 0, &val);
}

static struct ufs_dev_bkops_ops hynix_bkops_ops = {
	.ufs_bkops_query = ufshcd_hynix_bkops_query,
	.ufs_bkops_start = ufshcd_hynix_bkops_start,
	.ufs_bkops_stop = ufshcd_hynix_bkops_stop,
};

int ufshcd_bkops_status_query(void *bkops_data, u32 *status)
{
	int err;
	struct ufs_hba *hba = NULL;
#ifdef CONFIG_MAS_DEBUG_FS
	struct mas_bkops *bkops = NULL;
#endif

	if ((!bkops_data) || (!status))
		return -EINVAL;

	hba = (struct ufs_hba *)bkops_data;
#ifdef CONFIG_MAS_DEBUG_FS
	bkops = hba->ufs_bkops;
#endif

	pm_runtime_get_sync(hba->dev);
	err = hba->ufs_dev_bkops_ops->ufs_bkops_query(hba, status);
	pm_runtime_put_sync(hba->dev);
#ifdef CONFIG_MAS_DEBUG_FS
	if (bkops->bkops_debug_ops.sim_critical_bkops) {
		*status = 1;
		pr_err("sim_critical_bkops\n");
		bkops->bkops_debug_ops.sim_critical_bkops = 0;
	}
	if (bkops->bkops_debug_ops.sim_bkops_query_fail) {
		*status = 0;
		pr_err("simulate bkops query failure!\n");
		bkops->bkops_debug_ops.sim_bkops_query_fail = false;
		err = -1;
	}
#endif
	if (err)
		pr_err("UFS bkops status query failed!\n");

	return err;
}

static int ufshcd_bkops_start(struct ufs_hba *hba)
{
	int ret;

	pm_runtime_get_sync(hba->dev);
	ret = hba->ufs_dev_bkops_ops->ufs_bkops_start(hba);
	pm_runtime_put_sync(hba->dev);
	if (ret)
		pr_err("UFS bkops start failed!\n");

	return ret;
}

static int ufshcd_bkops_stop(struct ufs_hba *hba)
{
#ifdef CONFIG_MAS_DEBUG_FS
	int ret = 0;
	struct mas_bkops *bkops = hba->ufs_bkops;
	struct bkops_stats *bkops_stats_p = &(bkops->bkops_stats);
	u64 start_time, stop_time, time_interval;
	start_time = ktime_get_ns();

	if (mas_bkops_send_stop) {
		pm_runtime_get_sync(hba->dev);
		ret = hba->ufs_dev_bkops_ops->ufs_bkops_stop(hba);
		pm_runtime_put_sync(hba->dev);
	}

	bkops_stats_p->bkops_stop_count++;
	mas_bkops_update_dur(bkops_stats_p);

	stop_time = ktime_get_ns();
	time_interval = stop_time - start_time;
	if (time_interval > bkops_stats_p->bkops_max_stop_time)
		bkops_stats_p->bkops_max_stop_time = time_interval;

	bkops_stats_p->bkops_avrg_stop_time =
		((bkops_stats_p->bkops_avrg_stop_time *
			 (bkops_stats_p->bkops_stop_count - 1)) +
			time_interval) /
		bkops_stats_p->bkops_stop_count;

	if (ret)
		pr_err("UFS bkops stop failed!\n");
	return ret;
#else
	return 0;
#endif
}

int ufshcd_bkops_start_stop(void *bkops_data, int start)
{
	struct ufs_hba *hba = NULL;
	int ret;

	if (!bkops_data)
		return -EINVAL;

	hba = bkops_data;

	if (start)
		ret = ufshcd_bkops_start(hba);
	else
		ret = ufshcd_bkops_stop(hba);

	return ret;
}

static struct bkops_ops ufs_bkops_ops = {
	.bkops_start_stop = __ufshcd_bkops_start_stop,
	.bkops_status_query = __ufshcd_bkops_status_query,
};

static bool hufs_is_bkops_id_match(struct hufs_bkops_id *bkops_id,
	uint16_t man_id, const char *ufs_model, const char *ufs_rev)
{
	if (man_id != bkops_id->manufacturer_id)
		return false;

	if (strncmp(ufs_model, bkops_id->ufs_model,
		    strlen(bkops_id->ufs_model)) &&
	    strcmp(HUFS_MODEL_ANY, bkops_id->ufs_model))
		return false;

	if (strcmp(ufs_rev, bkops_id->ufs_rev) &&
	    strcmp(HUFS_REV_ANY, bkops_id->ufs_rev))
		return false;

	return true;
}

static bool hufs_is_bkops_supported(struct scsi_device *sdev)
{
	struct ufs_hba *hba = shost_priv(sdev->host);
	struct list_head *node = NULL;
	struct hufs_bkops_id *bkops_id = NULL;

	/* blacklist first */
	list_for_each(node, &hba->bkops_blacklist)
	{
		bkops_id = list_entry(node, struct hufs_bkops_id, p);
		if (hufs_is_bkops_id_match(bkops_id, hba->manufacturer_id,
		    sdev->model, sdev->rev))
			return false;
	}

	/* then whitelist */
	list_for_each(node, &hba->bkops_whitelist)
	{
		bkops_id = list_entry(node, struct hufs_bkops_id, p);
		if (hufs_is_bkops_id_match(bkops_id, hba->manufacturer_id,
		    sdev->model, sdev->rev))
			return true;
	}

	return false;
}

#ifdef CONFIG_MAS_DEBUG_FS
#define BKOPS_STATUS_MAX_HI1861 6
static const char *bkops_status_str_1861[BKOPS_STATUS_MAX_HI1861] = {
	"bkops none", "bkops normal", "bkops urgent count", "bkops wl",
	"bkops not care", "bkops slc cache",
};

#define BKOPS_STATUS_MAX_HYNIX 2
static const char *bkops_status_str_hynix[BKOPS_STATUS_MAX_HYNIX] = {
	"bkops none", "bkops needed",
};
#endif
static void mas_bkops_status_info(struct ufs_hba *hba,
				  struct mas_bkops *ufs_bkops)
{
	if (hba->manufacturer_id == UFS_VENDOR_HI1861) {
		ufs_bkops->dev_type = BKOPS_DEV_UFS_1861;
		ufs_bkops->bkops_flag |= UFS_BKOPS_1861_FLAG;
		hba->ufs_dev_bkops_ops = &hi1861_bkops_ops;
#ifdef CONFIG_MAS_DEBUG_FS
		mas_bkops_set_status_str(ufs_bkops,
		    BKOPS_STATUS_MAX_HI1861, bkops_status_str_1861);
#endif
	} else if (hba->manufacturer_id == UFS_VENDOR_SKHYNIX) {
		ufs_bkops->dev_type = BKOPS_DEV_UFS_HYNIX;
		ufs_bkops->bkops_flag |= UFS_BKOPS_HYNIX_FLAG;
		hba->ufs_dev_bkops_ops = &hynix_bkops_ops;
#ifdef CONFIG_MAS_DEBUG_FS
		mas_bkops_set_status_str(ufs_bkops,
		    BKOPS_STATUS_MAX_HYNIX, bkops_status_str_hynix);
#endif
	}
}

int hufs_manual_bkops_config(struct scsi_device *sdev)
{
	struct ufs_hba *hba = NULL;
	struct mas_bkops *ufs_bkops = NULL;
	struct request_queue *q = NULL;

	if (!sdev)
		return -EINVAL;

	hba = shost_priv(sdev->host);
	q = sdev->request_queue;

	if (!(sdev->host->hufs_dev_quirk_flag &
		    SHOST_HUFS_DEV_QUIRK(SHOST_QUIRK_BKOPS_ENABLE)))
		return 0;

	if (!hufs_is_bkops_supported(sdev) || hba->ufs_bkops_enabled) {
		pr_err("bkops not supported or has been enabled already!\n");
		return 0;
	}

	ufs_bkops = mas_bkops_alloc();
	if (!ufs_bkops)
		return -ENOMEM;

	mas_bkops_status_info(hba, ufs_bkops);

	hba->ufs_bkops = ufs_bkops;
	ufs_bkops->bkops_data = hba;
	ufs_bkops->bkops_ops = &ufs_bkops_ops;
	ufs_bkops->en_bkops_retry = 1;
	ufs_bkops->bkops_check_interval = BKOPS_DEF_CHECK_INTERVAL;

	if (mas_bkops_enable(q, ufs_bkops, hba->debugfs_files.debugfs_root))
		goto free_ufs_bkops;

	hba->ufs_bkops_enabled = 1;
#ifdef CONFIG_MAS_UNISTORE_PRESERVE
	if (hba->host->unistore_enable)
		mas_blk_mq_tagset_set_bkops(q, ufs_bkops);
#endif
	return 0;

free_ufs_bkops:
	kfree(ufs_bkops);
	return -EAGAIN;
}
