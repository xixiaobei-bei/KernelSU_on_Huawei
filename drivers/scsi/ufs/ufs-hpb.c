/*
 * ufs-hpb.c
 *
 * ufs hpb configuration
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/blktrace_api.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/sysfs.h>
#include <asm/unaligned.h>
#include <linux/async.h>
#include <linux/atomic.h>
#include <linux/blkdev.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/devfreq.h>
#include <linux/bitops.h>
#include <linux/fs.h>
#include <linux/radix-tree.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/uaccess.h>
#include <linux/list_sort.h>
#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_eh.h>
#include <scsi/scsi_host.h>

#include "ufs.h"
#include "ufs-hpb.h"
#include "ufs_test.h"
#include "ufshcd.h"
#include "ufshci.h"

#define MODULE_NAME "ufs_turbo_table"

static struct completion uftt_completion;
static struct scsi_device ufstt_scsi_device;
struct radix_tree_root node_tree;

static inline void ufshpb_done(struct scsi_cmnd *cmd)
{
	complete(&uftt_completion);
}

static inline struct ufshpb_rsp_sense_data *ufshpb_get_from_rsp(const struct ufshcd_lrb *lrbp)
{
	return (struct ufshpb_rsp_sense_data *)&lrbp->ucd_rsp_ptr->sr
			.sense_data_len;
}

static inline struct utp_cmd_rsp *ufshpb_get_from_rsp_host(const struct ufshcd_lrb *lrbp)
{
	return (struct utp_cmd_rsp *)&lrbp->ucd_rsp_ptr->sr;
}

static inline void init_hpb_radix_tree(void)
{
	node_tree.gfp_mask = GFP_ATOMIC | __GFP_NOWARN;
	node_tree.rnode = NULL;
}

#ifdef HPB_HUFS_DEBUG_PRINT
static inline struct subregion_node *node_addr_regionid_debug(struct ufshcd_lrb *lrbp, u32 id, u8 type)
{
	ktime_t time1, time2;
	struct subregion_node *node = NULL;

	time1 = ktime_get();
	node = radix_tree_lookup(&node_tree, id);
	time2 = ktime_get();

	lrbp->find_time += ktime_to_ns(ktime_sub(time2, time1));

	return node;
}
#endif

static inline struct subregion_node *node_addr_regionid(u32 id)
{
	return radix_tree_lookup(&node_tree, id);
}

static inline void idn_node_insert(unsigned long index, void *item)
{
	radix_tree_insert(&node_tree, index, item);
}

static inline void idn_node_delete(unsigned long index, void *item)
{
	radix_tree_delete_item(&node_tree, index, item);
}

static inline void ufshpb_sg_init_multi(struct scatterlist *sg, const void *buf,
					unsigned int buflen, int n)
{
	int i;
	char *p = (char *)buf;

	sg_init_table(sg, n);
	for (i = 0; i < n; i++)
		sg_set_buf(sg + i, p + buflen * i, buflen);
}

static inline void ufshpb_init_comm_scsi_cmd(struct scsi_cmnd *cmd,
					    struct ufs_hba *hba,
					    unsigned char *cdb,
					    unsigned char *sense_buffer)
{
	ufstt_scsi_device.host = hba->host;
	cmd->device = &ufstt_scsi_device;
	cmd->cmd_len = COMMAND_SIZE(cdb[0]);
	cmd->cmnd = cdb;
	cmd->sense_buffer = sense_buffer;
	cmd->scsi_done = ufshpb_done;
}

static inline void ufshpb_init_scsi_cmd(
	struct scsi_cmnd *cmd, struct ufs_hba *hba, unsigned char *cdb,
	unsigned char *sense_buffer, enum dma_data_direction sc_data_direction,
	struct scatterlist *sglist, unsigned int nseg, unsigned int sg_len)
{
	ufshpb_init_comm_scsi_cmd(cmd, hba, cdb, sense_buffer);

	cmd->sc_data_direction = sc_data_direction;
	cmd->sdb.table.sgl = sglist;
	cmd->sdb.table.nents = nseg;
	cmd->sdb.length = sg_len;

	cmd->serial_number = 0;
	cmd->prot_op = SCSI_PROT_NORMAL;
	cmd->sdb.resid = 0;
	cmd->transfersize = TRAN_LEN;
}

static inline void fill_rb_cdb_info(struct ufs_hba *hba, u8 *cdb, int region_id)
{
	cdb[RB_OPCODE] = UFSHPB_READ_BUFFER;
	cdb[RB_BUFFER_ID] = RB_BUFFER_ID;
	cdb[RB_REGION_ID_HIGH] = (unsigned char)GET_BYTE_1(region_id);
	cdb[RB_REGION_ID_LOW] = (unsigned char)GET_BYTE_0(region_id);
	cdb[RB_SUBREGION_ID_HIGH] = (unsigned char)GET_BYTE_1(0);
	cdb[RB_SUBREGION_ID_LOW] = (unsigned char)GET_BYTE_0(0);

	cdb[RB_AL_LEN3] =
		(unsigned char)GET_BYTE_2(hba->ufs_hpb->one_subregion_ppn_size);
	cdb[RB_AL_LEN2] =
		(unsigned char)GET_BYTE_1(hba->ufs_hpb->one_subregion_ppn_size);
	cdb[RB_AL_LEN1] =
		(unsigned char)GET_BYTE_0(hba->ufs_hpb->one_subregion_ppn_size);
	cdb[RB_CONTROL] = 0;
}

static int ufshpb_read_buffer(struct ufs_hba *hba, int region_id,
			      int subregion_id, char *subregion_hpb_buffer,
			      int lun)
{
	struct scsi_device *sdev = NULL;
	int err;
	unsigned char cdb[RB_CDB_SIZE] = {0};
	struct scsi_sense_hdr sshdr;

	if (!subregion_hpb_buffer)
		return -ENOMEM;

	fill_rb_cdb_info(hba, cdb, region_id);

	memset(subregion_hpb_buffer, '\0', hba->ufs_hpb->one_subregion_ppn_size);


	__shost_for_each_device(sdev, hba->host) {
		if (sdev->lun == lun)
			break;
	}

	err = scsi_execute_req(sdev, cdb, DMA_FROM_DEVICE,
			       subregion_hpb_buffer,
			       TRAN_LEN * hba->ufs_hpb->region_num_4k, &sshdr,
			       5 * HZ, /* retries */ 1, NULL);
	if (err)
		hba->ufshpb_read_buffer_fail_count++;

	return err;
}

static void run_active_worker(struct work_struct *work)
{
	struct ndlist_data *ndlist_data = NULL;
	struct subregion_node *node = NULL;
	int err;

	ndlist_data = container_of(work, struct ndlist_data, run_active_work);

	if (!mutex_trylock(&ndlist_data->hba->update_lock))
		return;
	while ((node = list_first_entry_or_null(&ndlist_data->active_list,
						struct subregion_node,
						active_list))) {
		if (ndlist_data->hba->in_suspend || ndlist_data->hba->in_shutdown) {
			mutex_unlock(&ndlist_data->hba->update_lock);
			return;
		}
		list_del_init(&node->active_list);
		err = ufshpb_read_buffer(ndlist_data->hba, node->subregion_id,
					 node->subregion_id,
					 (u8 *)node->node_addr, HPB_UNIT);

		ndlist_data->hba->ufshpb_read_buffer_count++;
		node->status = NODE_NEW;
		ndlist_data->hba->ufshpb_work_count--;

#ifdef HPB_HUFS_DEBUG_PRINT
		pr_err("%s, node id %x updated\n", __func__,
		       node->subregion_id);
		pr_err("%s, read buffer %d\n", __func__, err);
#endif

		if (err)
			pr_info("%s, read buffer processing\n", __func__);
	}
	mutex_unlock(&ndlist_data->hba->update_lock);
}

static int ufshpb_write_buffer_lru(struct ufs_hba *hba, u32 subregion_id,
				   int lun)
{
	unsigned char cmd[HPB_WRITE_BUFFER_CMD_LEN];
	int err;
	int timeout;
	struct scsi_device *sdev = NULL;

	__shost_for_each_device(sdev, hba->host) {
		if (sdev->lun == lun)
			break;
	}

	timeout = sdev->request_queue->rq_timeout;

	memset(cmd, '\0', HPB_WRITE_BUFFER_CMD_LEN);
	cmd[OPCODE] = HPB_WRITE_BUFFER;
	cmd[BUFFER_ID] = HPB_BUFFER_ID_INACTIVE;
	cmd[ID_HIGH] = GET_BYTE_1(subregion_id);
	cmd[ID_LOW] = GET_BYTE_0(subregion_id);
	cmd[RESERVD4] = 0;
	cmd[RESERVD5] = 0;
	cmd[RESERVD6] = 0;
	cmd[RESERVD7] = 0;
	cmd[RESERVD8] = 0;
	cmd[RESERVD9] = 0;

	err = scsi_execute_req(sdev, cmd, DMA_NONE, NULL, 0, NULL, timeout,
			       HPB_WRITE_BUFFER_RETRY, NULL);

	return err;
}

static void run_inactive_worker(struct work_struct *work)
{
	struct ndlist_data *ndlist_data = NULL;
	struct subregion_node *node = NULL;
	int ret;

	ndlist_data = container_of(work, struct ndlist_data, run_inactive_work);
	while ((node = list_first_entry_or_null(&ndlist_data->inactive_list,
						struct subregion_node,
						inactive_list))) {
		list_del_init(&node->inactive_list);

		ret = ufshpb_write_buffer_lru(ndlist_data->hba,
					      node->subregion_id_old, HPB_UNIT);
		ndlist_data->hba->ufshpb_inactive_count++;

		if (ret)
			pr_info("%s, write buffer processing %d\n", __func__,
			       ret);

		node->status = NODE_UPDATING;

		list_add_tail(&node->active_list, &ndlist_data->active_list);

		queue_work(system_freezable_wq, &ndlist_data->run_active_work);
	}
}

static inline void init_ndlist_data(struct ndlist_data *ndlist_data)
{
	INIT_WORK(&ndlist_data->run_active_work, run_active_worker);
	INIT_WORK(&ndlist_data->run_inactive_work, run_inactive_worker);
	spin_lock_init(&ndlist_data->lock);
}

static inline void init_subregion_node_list(struct subregion_node *subregion_node)
{
	INIT_LIST_HEAD(&subregion_node->active_list);
	INIT_LIST_HEAD(&subregion_node->inactive_list);
	INIT_LIST_HEAD(&subregion_node->lru_list);
}

static int alloc_subregion_node_mem(struct ufshpb_lu_control *ufshpb_lu_control)
{
	int i, j, k;
	u8 *base_addr = NULL;
	struct subregion_node *subregion_node = NULL;
	struct ndlist_data *ndlist_data = NULL;
	struct subregion_node *subregion_node_entry = NULL;
	u8 *temp_addr[HPB_ALLOC_MEM_COUNT];
	int temp_count = 0;
	struct subregion_node *subregion_node_temp = NULL;

	k = 0;
	ndlist_data = ufshpb_lu_control->ndlist_data;
	ndlist_data->hba = ufshpb_lu_control->hba;
	init_ndlist_data(ndlist_data);
	for (i = 0; i < HPB_ALLOC_MEM_COUNT; i++) {
		base_addr = kzalloc(HPB_MEMSIZE_PER_LOOP, GFP_KERNEL);
		if (!base_addr)
			goto out_mem;
		temp_addr[i] = base_addr;
		temp_count++;
		for (j = 0; j < HPB_SUBREGION_NUM_PER_MEMSIZE; j++) {
			subregion_node = kzalloc(sizeof(struct subregion_node), GFP_KERNEL);
			if (!subregion_node) {
				pr_err("%s, alloc subregion fail\n", __func__);
				goto out_mem;
			}
			ufshpb_lu_control->subregion_array[k] = subregion_node;
			k++;
			subregion_node->node_addr =
				(u64 *)(base_addr + j * HPB_SUBREGION_PPN_SIZE);
			init_subregion_node_list(subregion_node);
			/* add to lru list, default status node empty */
			list_add(&subregion_node->lru_list,
				 &ndlist_data->lru_list);
		}
	}
	return 0;
out_mem:
	for (k = 0; k < temp_count; k++)
		kfree(temp_addr[k]);
	list_for_each_entry_safe(subregion_node_entry, subregion_node_temp, &ndlist_data->lru_list,
			     lru_list) {
		list_del_init(&subregion_node_entry->lru_list);
		kfree(subregion_node_entry);
	}
	return -ENOMEM;
}

static inline void add_to_active_list(struct subregion_node *subregion_node,
				      struct ndlist_data *ndlist_data,
				      u32 region_id)
{
	list_del_init(&subregion_node->lru_list);
	subregion_node->subregion_id = region_id;
	subregion_node->status = NODE_UPDATING;
	list_add(&subregion_node->lru_list, &ndlist_data->lru_list);

	list_add_tail(&subregion_node->active_list, &ndlist_data->active_list);
}

static int ufshpb_rsp_is_invalid(struct ufshpb_rsp_sense_data *rsp_data)
{
	if (be16_to_cpu(rsp_data->sense_data_len) != HPB_SENSE_DATA_LEN ||
	    rsp_data->desc_type != HPB_DESC_TYPE ||
	    rsp_data->additional_len != HPB_ADDITIONAL_LEN ||
	    rsp_data->hpb_opt == HPB_REGION_NONE ||
	    rsp_data->act_cnt > MAX_ACTIVE_NUM ||
	    rsp_data->inact_cnt > MAX_INACTIVE_NUM ||
	    (!rsp_data->act_cnt && !rsp_data->inact_cnt) ||
	    rsp_data->lun > UFS_UPIU_MAX_GENERAL_LUN)
		return -EINVAL;

	return 0;
}

static inline struct subregion_node *ufshpb_get_active_list(struct ufs_hba *hba, struct ndlist_data *ndlist_data)
{
	unsigned long flags;
	struct subregion_node *node = NULL;

	spin_lock_irqsave(hba->host->host_lock, flags);
	node = list_first_entry_or_null(&ndlist_data->active_list,
					struct subregion_node, active_list);
	if (node)
		list_del_init(&node->active_list);
	spin_unlock_irqrestore(hba->host->host_lock, flags);

	return node;
}

static inline void ufshpb_active_hpb_entry(struct ufs_hba *hba,
					   struct ndlist_data *ndlist_data)
{
	struct subregion_node *node = NULL;
	int err;
	unsigned long flags;

	while ((node = ufshpb_get_active_list(hba, ndlist_data))) {
		if (hba->in_suspend || hba->in_shutdown)
			return;

		err = ufshpb_read_buffer(hba, node->subregion_id,
					 0, (u8 *)node->node_addr, HPB_UNIT);
		hba->ufshpb_read_buffer_count++;
		if (err) {
			pr_info("%s, read buffer processing %d\n", __func__,
			       err);
		} else {
			spin_lock_irqsave(hba->host->host_lock, flags);
			node->status = NODE_NEW;
			spin_unlock_irqrestore(hba->host->host_lock, flags);
#ifdef HPB_HUFS_DEBUG_PRINT
			pr_info("%s, node id %x updated\n", __func__,
			       node->subregion_id);
#endif
		}
	}
}

static void ufshpb_update_entry_hanler(struct work_struct *work)
{
	struct ufs_hba *hba = NULL;
	int i;

	hba = container_of(work, struct ufs_hba, hpb_update_work);
	if (hba->ufs_hpb->ufshpb_state != UFSHPB_PRESENT)
		return;

	for (i = 0; i < MAX_HPB_LUN_NUM; i++)
		if (hba->ufs_hpb->ufshpb_lu_control[i])
			ufshpb_active_hpb_entry(
				hba,
				hba->ufs_hpb->ufshpb_lu_control[i]->ndlist_data);
}

static void check_and_add_inactive_list(struct ufs_hba *hba,
					struct ufshpb_rsp_sense_data *rsp_data,
					struct ufshcd_lrb *lrbp)
{
	int i;
	struct ndlist_data *ndlist_data = NULL;
	struct ufshpb_lu_control *hpb_lu_control = NULL;
	struct subregion_node *subregion_node = NULL;

	hpb_lu_control = hba->ufs_hpb->ufshpb_lu_control[HPB_UNIT];
	if (!hpb_lu_control)
		return;

	ndlist_data = hpb_lu_control->ndlist_data;

	if (hba->in_suspend || hba->in_shutdown)
		return;

	for (i = 0; i < rsp_data->inact_cnt; i++) {
#ifdef HPB_HUFS_DEBUG_PRINT
		subregion_node = node_addr_regionid_debug(
			lrbp, i == 0 ? (be16_to_cpu(rsp_data->inact_region_0)) :
				       (be16_to_cpu(rsp_data->inact_region_1)),
			0);
#else
		subregion_node = node_addr_regionid(
			i == 0 ? (be16_to_cpu(rsp_data->inact_region_0)) :
				 (be16_to_cpu(rsp_data->inact_region_1)));
#endif
		if (subregion_node) {
			subregion_node->status = NODE_EMPTY;
			list_del_init(&subregion_node->active_list);
			idn_node_delete(subregion_node->subregion_id,
					subregion_node);
			list_add(&subregion_node->lru_list,
				 &ndlist_data->lru_list);

			hba->ufshpb_delete_node_count++;
		}
	}
}

static void check_and_add_active_list(struct ufs_hba *hba,
				      struct ufshpb_rsp_sense_data *rsp_data,
				      struct ufshcd_lrb *lrbp)
{
	int i;
	struct ndlist_data *ndlist_data = NULL;
	struct ufshpb_lu_control *hpb_lu_control = NULL;
	struct subregion_node *subregion_node = NULL;

	hpb_lu_control = hba->ufs_hpb->ufshpb_lu_control[HPB_UNIT];
	if (!hpb_lu_control)
		return;
	ndlist_data = hpb_lu_control->ndlist_data;

	if (hba->in_suspend || hba->in_shutdown)
		return;

	for (i = 0; i < rsp_data->act_cnt; i++) {
#ifdef HPB_HUFS_DEBUG_PRINT
		subregion_node = node_addr_regionid_debug(
			lrbp, i == 0 ? (be16_to_cpu(rsp_data->act_region_0)) :
				       (be16_to_cpu(rsp_data->act_region_1)),
			1);
#else
		subregion_node = node_addr_regionid(
			i == 0 ? (be16_to_cpu(rsp_data->act_region_0)) :
				 (be16_to_cpu(rsp_data->act_region_1)));
#endif
		if (subregion_node)
			continue;
		if (!list_empty(&ndlist_data->lru_list)) {
			subregion_node =
				container_of((&(ndlist_data->lru_list))->prev,
					     struct subregion_node, lru_list);
			subregion_node->subregion_id =
				(i == 0 ?
					 (be16_to_cpu(rsp_data->act_region_0)) :
					 (be16_to_cpu(rsp_data->act_region_1)));
			subregion_node->status = NODE_UPDATING;
			idn_node_insert(subregion_node->subregion_id,
					subregion_node);
			list_del_init(&subregion_node->lru_list);
			list_add_tail(&subregion_node->active_list,
				      &ndlist_data->active_list);
			hba->ufshpb_add_node_count++;

		}
	}
}

static void ufshpb_region_update(struct ufs_hba *hba,
				 struct ufshpb_rsp_sense_data *rsp_data,
				 struct ufshcd_lrb *lrbp)
{
	check_and_add_inactive_list(hba, rsp_data, lrbp);
	check_and_add_active_list(hba, rsp_data, lrbp);
	queue_work(system_freezable_wq, &hba->hpb_update_work);
}

void ufshpb_check_rsp_upiu(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	struct utp_cmd_rsp *rsp_info = NULL;
	struct subregion_node *subregion_node = NULL;
	struct ndlist_data *ndlist_data = NULL;
	struct ufshpb_lu_control *hpb_lu_control = NULL;
	int region_id_one;
	int region_id_two;
	u8 *rsp_info_byte = NULL;

	if (hba->in_suspend || hba->in_shutdown)
		return;

	if ((lrbp->cmd->request == NULL) || (lrbp->lun != HPB_UNIT))
		return;

	hpb_lu_control = hba->ufs_hpb->ufshpb_lu_control[HPB_UNIT];
	rsp_info = ufshpb_get_from_rsp_host(lrbp);
	rsp_info_byte = (u8 *)rsp_info;
	ndlist_data = hpb_lu_control->ndlist_data;

	if (rsp_info_byte[HPB_REGION_OPREQ_OFFSET] != HPB_REGION_UPDATE_REQ)
		return;

	if (rsp_info_byte[HPB_REGION_VALID_OFFSET] == HPB_ONE_VALID)
		goto one_valid_region;

	/* use two byte compose word according to ufs3.1 */
	region_id_two = rsp_info_byte[HPB_ACTIVE_REGION_TWO_OFFSET]
				<< ONE_BYTE_SHIFT |
			rsp_info_byte[HPB_ACTIVE_REGION_TWO_OFFSET + 1];
	subregion_node = node_addr_regionid(region_id_two);
	if (subregion_node) {
		add_to_active_list(subregion_node, ndlist_data,
				   region_id_two);
		hba->ufshpb_work_count++;
	}
one_valid_region:
	region_id_one = rsp_info_byte[HPB_ACTIVE_REGION_ONE_OFFSET]
				<< ONE_BYTE_SHIFT |
			rsp_info_byte[HPB_ACTIVE_REGION_ONE_OFFSET + 1];
	subregion_node = node_addr_regionid(region_id_one);

	if (subregion_node) {
		add_to_active_list(subregion_node, ndlist_data,
				   region_id_one);
		hba->ufshpb_work_count++;
	}

	queue_work(system_freezable_wq, &ndlist_data->run_active_work);
}

void ufshpb_check_rsp_upiu_device_mode(struct ufs_hba *hba,
				       struct ufshcd_lrb *lrbp)
{
	struct ufshpb_rsp_sense_data *rsp_data = NULL;

	if (hba->in_suspend || hba->in_shutdown)
		return;

	if (hba->ufs_hpb == NULL)
		return;

	if (hba->ufs_hpb->ufshpb_state != UFSHPB_PRESENT)
		return;

	if (hba->manufacturer_id != UFS_VENDOR_SAMSUNG)
		return;

	rsp_data = ufshpb_get_from_rsp(lrbp);
	if (ufshpb_rsp_is_invalid(rsp_data))
		return;

#ifdef HPB_HUFS_DEBUG_PRINT
	pr_err("%s, hpb operation %x,lun %x,lrbp->lun %x\n", __func__,
	       rsp_data->hpb_opt, rsp_data->lun, lrbp->lun);
	pr_err("%s,act_cnt %x, region_0 %x,subregion_0 %x,region_1 "
	       "%x,subregion_1 %x\n",
	       __func__, rsp_data->act_cnt,
	       (be16_to_cpu(rsp_data->act_region_0)),
	       (be16_to_cpu(rsp_data->act_subregion_0)),
	       (be16_to_cpu(rsp_data->act_region_1)),
	       (be16_to_cpu(rsp_data->act_subregion_1)));
	pr_err("%s,inact_cnt %x,region_0 %x, region_1 %x\n", __func__,
	       rsp_data->inact_cnt, (be16_to_cpu(rsp_data->inact_region_0)),
	       (be16_to_cpu(rsp_data->inact_region_1)));
#endif
	if (lrbp->lun != HPB_UNIT)
		return;

	switch (rsp_data->hpb_opt) {
	case HPB_REGION_UPDATE_REQ:
		ufshpb_region_update(hba, rsp_data, lrbp);
		break;
	case HPB_REGION_RESET_REQ:
		break;
	default:
		pr_err("hpb_type is not available : %d", rsp_data->hpb_opt);
		return;
	}
}

static inline bool ufshpb_is_read_lrbp(struct ufshcd_lrb *lrbp)
{
	if (lrbp->cmd->cmnd[0] == READ_10 || lrbp->cmd->cmnd[0] == READ_16)
		return true;

	return false;
}

static inline bool ufshpb_is_write_discard_lrbp(struct ufshcd_lrb *lrbp)
{
	if (lrbp->cmd->cmnd[0] == WRITE_10 || lrbp->cmd->cmnd[0] == WRITE_16 ||
	    lrbp->cmd->cmnd[0] == UNMAP)
		return true;

	return false;
}

static void ufshpb_ppn_prep(struct ufs_hba *hba, struct ufshcd_lrb *lrbp,
			    u64 ppn)
{
	unsigned char cmd[HPB_READ16_CMDLEN] = {0};
	unsigned int transfer_len;

	transfer_len = SHIFT_BYTE_1(lrbp->cmd->cmnd[HPB_READ16_TRANSLEN7]) |
		       SHIFT_BYTE_0((lrbp->cmd->cmnd[HPB_READ16_TRANSLEN8] &
				     ONE_BYTE_MASK));

#ifdef HPB_HUFS_DEBUG_PRINT
	pr_err("%s, ppn %llx", __func__, ppn);
	pr_err("%s, transfer_len %llx", __func__, transfer_len);
#endif
	if ((hba->manufacturer_id == UFS_VENDOR_SAMSUNG) ||
	    (hba->manufacturer_id == UFS_VENDOR_TOSHIBA) ||
	    (hba->manufacturer_id == UFS_VENDOR_SKHYNIX)) {
		cmd[HPB_READ16_OP] = READ_16;
		cmd[HPB_READ16_LBA1] = lrbp->cmd->cmnd[HPB_READ16_LBA1];
		cmd[HPB_READ16_LBA2] = lrbp->cmd->cmnd[HPB_READ16_LBA2];
		cmd[HPB_READ16_LBA3] = lrbp->cmd->cmnd[HPB_READ16_LBA3];
		cmd[HPB_READ16_LBA4] = lrbp->cmd->cmnd[HPB_READ16_LBA4];
		cmd[HPB_READ16_PPN1] = GET_BYTE_7(ppn);
		cmd[HPB_READ16_PPN2] = GET_BYTE_6(ppn);
		cmd[HPB_READ16_PPN3] = GET_BYTE_5(ppn);
		cmd[HPB_READ16_PPN4] = GET_BYTE_4(ppn);
		cmd[HPB_READ16_PPN5] = GET_BYTE_3(ppn);
		cmd[HPB_READ16_PPN6] = GET_BYTE_2(ppn);
		cmd[HPB_READ16_PPN7] = GET_BYTE_1(ppn);
		cmd[HPB_READ16_PPN8] = GET_BYTE_0(ppn);

		cmd[HPB_READ16_TRANS_LEN] = transfer_len & ONE_BYTE_MASK;
		cmd[HPB_READ16_CONTROL] = HPB_READ16_CONTROL_VALUE;
	}

	if (hba->manufacturer_id == UFS_VENDOR_SAMSUNG) {
		cmd[HPB_RESERVED0] = lrbp->cmd->cmnd[HPB_RESERVED0];
		cmd[HPB_READ16_TRANS_LEN] = HPB_SAMSUNG_BUFID;
		cmd[HPB_READ16_CONTROL] = transfer_len & ONE_BYTE_MASK;
		lrbp->cmd->cmd_len = MAX_CDB_SIZE;
	}

	memcpy(lrbp->cmd->cmnd, cmd, MAX_CDB_SIZE);
	memcpy(lrbp->ucd_req_ptr->sc.cdb, cmd, MAX_CDB_SIZE);
}

void ufshpb_init_handler(struct work_struct *work)
{
	struct ufs_hba *hba = NULL;
	int err;
	struct delayed_work *dwork = to_delayed_work(work);

	hba = ((struct ufs_hpb_info *)(container_of(dwork, struct ufs_hpb_info,
						    ufshpb_init_work)))
		      ->hba;
	if (hba->ufs_hpb && (hba->ufs_hpb->ufshpb_state == HPB_NEED_INIT)) {
		pr_err("ufshpb init start\n");
		err = hba->ufs_hpb->ufs_hpb_init_lru(hba);
		if (err)
			pr_err("ufshpb init no need\n");
	}
}

static struct ufshpb_lu_control *ufshpb_lu_control_init(void)
{
	struct ufshpb_lu_control *ufshpb_lu_control = NULL;

#ifdef HPB_HUFS_DEBUG_PRINT
	pr_err("%s: hpb lun control init\n", __func__);
#endif
	ufshpb_lu_control =
		kzalloc(sizeof(struct ufshpb_lu_control), GFP_KERNEL);
	if (!ufshpb_lu_control) {
		pr_err("%s: hpb kzalloc lun control error\n", __func__);
		return NULL;
	}

	spin_lock_init(&ufshpb_lu_control->hpb_control_lock);

	return ufshpb_lu_control;
}

static inline int init_lru_list(struct ufshpb_lu_control *hpb_lu_control)
{
	struct ndlist_data *ndlist_data = NULL;

	ndlist_data = kzalloc(sizeof(struct ndlist_data), GFP_KERNEL);
	if (!ndlist_data)
		return -ENOMEM;
	hpb_lu_control->ndlist_data = ndlist_data;

	INIT_LIST_HEAD(&ndlist_data->active_list);
	INIT_LIST_HEAD(&ndlist_data->inactive_list);
	INIT_LIST_HEAD(&ndlist_data->lru_list);

	return 0;
}

static inline void ppn_size_select(struct ufs_hba *hba)
{
	hba->ufs_hpb->ppn_size = HPB_ONE_ENTRY_PPN_SIZE_JDEC;
}

static inline void fill_hpb_unit_desc(struct ufs_hba *hba, u8 *unit_desc_buf)
{
	hba->ufs_hpb->hpb_lu_enable[HPB_UNIT] =
		unit_desc_buf[HPB_UNIT_DESC_LU_ENBALE];
	hba->ufs_hpb->hpb_logical_block_size =
		(1 << unit_desc_buf[HPB_UNIT_DESC_LOGICAL_BLOCK_SIZE]);
	hba->ufs_hpb->hpb_logical_block_count = cpu_to_be64(
		*((u64 *)(&unit_desc_buf[HPB_UNIT_DESC_LOGICAL_BLOCK_COUNT])));

	hba->ufs_hpb->hpb_lu_max_active_regions[HPB_UNIT] = cpu_to_be16(*((
		u16 *)(&unit_desc_buf[HPB_UNIT_DESC_LUMAX_ACTIVE_HPB_REGIONS])));

	hba->ufs_hpb->hpb_lu_pinned_region_startidx[HPB_UNIT] = cpu_to_be16(*(
		(u16 *)(&unit_desc_buf[HPB_UNIT_DESC_PINNED_REGION_STARTIDX])));

	hba->ufs_hpb->hpb_lu_pinned_region_num[HPB_UNIT] = cpu_to_be16(
		*((u16 *)(&unit_desc_buf[HPB_UNIT_DESC_NUM_PINNED_REGIONS])));
}

static inline void fill_hpb_info(struct ufs_hba *hba, u8 *device_desc_buf,
				 u8 *geo_desc_buf, u8 *unit_desc_buf)
{
	if (hba->manufacturer_id == UFS_VENDOR_SAMSUNG) {
		hba->ufs_hpb->one_region_physical_size =
			(1 << geo_desc_buf[HPB_REGION_SIZE_OFFSET]) *
			HPB_SECTOR_SIZE;
		hba->ufs_hpb->one_subregion_physical_size =
			(1 << geo_desc_buf[HPB_SUBREGION_SIZE_OFFSET]) *
			HPB_SECTOR_SIZE;
	} else if ((hba->manufacturer_id == UFS_VENDOR_TOSHIBA) ||
		   (hba->manufacturer_id == UFS_VENDOR_SKHYNIX)) {
		hba->ufs_hpb->one_region_physical_size =
			(1 << geo_desc_buf[HPB_REGION_SIZE_OFFSET]) * SECTOR;
		hba->ufs_hpb->one_subregion_physical_size =
			(1 << geo_desc_buf[HPB_SUBREGION_SIZE_OFFSET]) * SECTOR;
	}
	hba->ufs_hpb->hpb_lu_number = geo_desc_buf[HPB_LU_NUMBER_OFFSET];
	hba->ufs_hpb->hpb_device_max_active_regions =
		cpu_to_be16(*((u16 *)(&geo_desc_buf[HPB_MAX_ACTIVE_REGIONS])));
	hba->ufs_hpb->bud0_base_offset = device_desc_buf[BUDO_BASE_OFFSET];
	hba->ufs_hpb->bud_config_plength = device_desc_buf[BUD_CONFIG_PLENGTH];
	hba->ufs_hpb->config_desc_length =
		hba->ufs_hpb->bud_config_plength * HPB_UNIT_DESCS_COUNT +
		hba->ufs_hpb->bud0_base_offset;
	hba->ufs_hpb->hpb_control_mode =
		device_desc_buf[UFSHPB_CONTROL_MODE_OFFSET];
	fill_hpb_unit_desc(hba, unit_desc_buf);

	hba->ufs_hpb->one_node_ppn_size =
		(hba->ufs_hpb->one_subregion_physical_size / UFS_BLOCK_SIZE) *
		ONE_BYTE_SHIFT;
	hba->ufs_hpb->one_node_ppn_number =
		hba->ufs_hpb->one_node_ppn_size / hba->ufs_hpb->ppn_size;
	hba->ufs_hpb->one_region_ppn_size =
		(hba->ufs_hpb->one_region_physical_size / UFS_BLOCK_SIZE) *
		hba->ufs_hpb->ppn_size;
	hba->ufs_hpb->one_region_ppn_number =
		hba->ufs_hpb->one_region_physical_size / UFS_BLOCK_SIZE;
	hba->ufs_hpb->one_subregion_ppn_number =
		hba->ufs_hpb->one_subregion_physical_size / UFS_BLOCK_SIZE;
	hba->ufs_hpb->one_subregion_ppn_size =
		(hba->ufs_hpb->one_subregion_physical_size / UFS_BLOCK_SIZE) *
		hba->ufs_hpb->ppn_size;
	hba->ufs_hpb->one_region_subregion_number =
		hba->ufs_hpb->one_region_ppn_size /
		hba->ufs_hpb->one_subregion_ppn_size;
	hba->ufs_hpb->region_num_4k =
		hba->ufs_hpb->one_region_ppn_size / UFS_BLOCK_SIZE;
}

static int get_desc_samsung(struct ufs_hba *hba, u8 *device_desc_buf,
			    u8 *geo_desc_buf, u8 *unit_desc_buf)
{
	int size;
	int err;

	pr_err("%s, enter samsung hpb desc info\n", __func__);
	size = HPB_DESC_DEVICE_MAX_SIZE_SAMSUNG;
	err = ufshcd_query_descriptor_retry(hba, UPIU_QUERY_OPCODE_READ_DESC,
					    QUERY_DESC_IDN_DEVICE, 0,
					    HPB_DEVICE_DESC_SELECTOR_SAMSUNG,
					    device_desc_buf, &size);
	if (err)
		return err;

	size = HPB_GEOMETRY_MAX_SIZE_SAMSUNG;
	err = ufshcd_query_descriptor_retry(hba, UPIU_QUERY_OPCODE_READ_DESC,
					    QUERY_DESC_IDN_GEOMETRY, 0,
					    HPB_GEOMETRY_DESC_SELECTOR_SAMSUNG,
					    geo_desc_buf, &size);
	if (err)
		return err;

	size = HPB_UNIT_DESC_MAX_SIZE;
	err = ufshcd_query_descriptor_retry(hba, UPIU_QUERY_OPCODE_READ_DESC,
					    QUERY_DESC_IDN_UNIT, HPB_UNIT,
					    HPB_UNIT_DESC_SELECTOR_SAMSUNG,
					    unit_desc_buf, &size);
	if (err)
		return err;

	return err;
}

static int get_desc_jdec(struct ufs_hba *hba, u8 *device_desc_buf,
			 u8 *geo_desc_buf, u8 *unit_desc_buf)
{
	int size;
	int err;

	size = HPB_DESC_DEVICE_MAX_SIZE;
	err = ufshcd_query_descriptor_retry(hba, UPIU_QUERY_OPCODE_READ_DESC,
					    QUERY_DESC_IDN_DEVICE, 0,
					    HPB_DEVICE_DESC_SELECTOR_PROTOCOL,
					    device_desc_buf, &size);
	if (err)
		return err;

	size = HPB_GEOMETRY_MAX_SIZE;
	err = ufshcd_query_descriptor_retry(hba, UPIU_QUERY_OPCODE_READ_DESC,
					    QUERY_DESC_IDN_GEOMETRY, 0,
					    HPB_GEOMETRY_DESC_SELECTOR_PROTOCOL,
					    geo_desc_buf, &size);
	if (err)
		return err;

	size = HPB_MAX_UNIT_DESC_SIZE;
	err = ufshcd_query_descriptor_retry(hba, UPIU_QUERY_OPCODE_READ_DESC,
					    QUERY_DESC_IDN_UNIT, HPB_UNIT,
					    HPB_UNIT_DESC_SELECTOR_PROTOCOL,
					    unit_desc_buf, &size);
	if (err)
		return err;

	return 0;
}

static void fill_support_hpb_version_info(struct ufs_hba *hba,
					  u8 *device_desc_buf)
{
	const u64 support_offset = HPB_DEVICE_DESC_DEXTENDED_UFSFEATURE_SUPPORT;
	const u64 hpb_version_offset = HPB_VERSION;

	hba->ufs_hpb->hpb_version =
		cpu_to_be16(*((u16 *)(&device_desc_buf[hpb_version_offset])));

	hba->ufs_hpb->hpb_extended_ufsfeature_support =
		cpu_to_be32(*((u32 *)(&device_desc_buf[support_offset])));

	pr_err("%s: hpb version %x device desc feature support %x\n",
	       __func__, hba->ufs_hpb->hpb_version,
	       hba->ufs_hpb->hpb_extended_ufsfeature_support);
}

static int get_manufacturer_desc(struct ufs_hba *hba, u8 *device_desc_buf,
				 u8 *geo_desc_buf, u8 *unit_desc_buf)
{
	int err;

	switch (hba->manufacturer_id) {
	case UFS_VENDOR_SAMSUNG:
		err = get_desc_samsung(hba, device_desc_buf, geo_desc_buf,
				       unit_desc_buf);
		break;
	case UFS_VENDOR_TOSHIBA:
	case UFS_VENDOR_SKHYNIX:
		err = get_desc_jdec(hba, device_desc_buf, geo_desc_buf,
				    unit_desc_buf);
		break;
	default:
		pr_err("%s: no such product\n", __func__);
		err = -EINVAL;
	}

	return err;
}

int is_hpb_opened_card(struct ufs_hba *hba)
{
	int err;
	u8 *device_desc_buf = NULL;
	u8 *geo_desc_buf = NULL;
	u8 *unit_desc_buf = NULL;

	device_desc_buf = kzalloc(HPB_DESC_DEVICE_MAX_SIZE_1862_SUPPORT,
				  GFP_KERNEL);
	if (!device_desc_buf)
		return 0;

	geo_desc_buf = kzalloc(HPB_GEOMETRY_MAX_SIZE_SAMSUNG, GFP_KERNEL);
	if (!geo_desc_buf) {
		err = -ENOMEM;
		goto out;
	}

	unit_desc_buf = kzalloc(HPB_UNIT_DESC_MAX_SIZE, GFP_KERNEL);
	if (!unit_desc_buf) {
		err = -ENOMEM;
		goto out;
	}

	err = get_manufacturer_desc(hba, device_desc_buf, geo_desc_buf,
				    unit_desc_buf);
	if (err)
		goto out;

	if (unit_desc_buf[HPB_UNIT_DESC_LU_ENBALE] != HPB_FUNC_ENABLE)
		err = -EINVAL;

out:
	kfree(device_desc_buf);
	kfree(geo_desc_buf);
	kfree(unit_desc_buf);

	if (err)
		return 0;
	return 1;
}

static int get_device_hpb_desc_info(struct ufs_hba *hba)
{
	int err;
	u8 *device_desc_buf = NULL;
	u8 *geo_desc_buf = NULL;
	u8 *unit_desc_buf = NULL;

	device_desc_buf = kzalloc(HPB_DESC_DEVICE_MAX_SIZE_1862_SUPPORT,
				  GFP_KERNEL);
	if (!device_desc_buf)
		return -ENOMEM;

	geo_desc_buf = kzalloc(HPB_GEOMETRY_MAX_SIZE_SAMSUNG, GFP_KERNEL);
	if (!geo_desc_buf) {
		err = -ENOMEM;
		goto out;
	}

	unit_desc_buf = kzalloc(HPB_UNIT_DESC_MAX_SIZE, GFP_KERNEL);
	if (!unit_desc_buf) {
		err = -ENOMEM;
		goto out;
	}

	ppn_size_select(hba);
	err = get_manufacturer_desc(hba, device_desc_buf, geo_desc_buf,
				    unit_desc_buf);
	if (err)
		goto out;

	hba->ufs_hpb->hpb_lu_enable[HPB_UNIT] =
		unit_desc_buf[HPB_UNIT_DESC_LU_ENBALE];

	pr_err("%s: lu enable %x\n", __func__,
	       hba->ufs_hpb->hpb_lu_enable[HPB_UNIT]);

	fill_support_hpb_version_info(hba, device_desc_buf);

	fill_hpb_info(hba, device_desc_buf, geo_desc_buf, unit_desc_buf);

out:
	kfree(device_desc_buf);
	kfree(geo_desc_buf);
	kfree(unit_desc_buf);

	return err;
}

static inline int control_init_per_lun(struct ufs_hba *hba, int lun)
{
	struct ufshpb_lu_control *hpb_lu_control = NULL;
	int ret;

	ret = get_device_hpb_desc_info(hba);
	if (ret)
		return ret;
	hba->ufs_hpb->ufshpb_lu_control[lun] = ufshpb_lu_control_init();
	if (!hba->ufs_hpb->ufshpb_lu_control[lun]) {
		pr_err("%s: lun %d ufshpb control struct no need alloc\n",
		       __func__, lun);
		goto alloc_lu_fail;
	}
	hpb_lu_control = hba->ufs_hpb->ufshpb_lu_control[lun];
	hpb_lu_control->hba = hba;
	hpb_lu_control->lun = lun;
	ret = init_lru_list(hpb_lu_control);
	if (ret)
		goto lru_fail;
	ret = alloc_subregion_node_mem(hpb_lu_control);
	if (ret)
		goto out;
	return ret;
out:
	kfree(hpb_lu_control->ndlist_data);
lru_fail:
	kfree(hba->ufs_hpb->ufshpb_lu_control[lun]);

alloc_lu_fail:
	return -ENOMEM;
}

static void pre_load_ppn(struct ufs_hba *hba)
{
	int err;
	int i = 0;
	struct ndlist_data *ndlist_data = NULL;
	struct ufshpb_lu_control *ufshpb_lu_control = NULL;
	struct subregion_node *subregion_node_entry = NULL;

	ufshpb_lu_control = hba->ufs_hpb->ufshpb_lu_control[HPB_UNIT];
	ndlist_data = ufshpb_lu_control->ndlist_data;

	list_for_each_entry(subregion_node_entry, &ndlist_data->lru_list,
			     lru_list) {
		subregion_node_entry->subregion_id = i;
		idn_node_insert(subregion_node_entry->subregion_id,
				subregion_node_entry);
		i++;
	}

	list_for_each_entry(subregion_node_entry, &ndlist_data->lru_list,
			     lru_list) {

		err = ufshpb_read_buffer(hba,
					 subregion_node_entry->subregion_id,
					 subregion_node_entry->subregion_id,
					 (u8 *)subregion_node_entry->node_addr,
					 HPB_UNIT);
		hba->ufshpb_read_buffer_count++;

		subregion_node_entry->status = NODE_NEW;
		if (err)
			pr_err("read buffer processing\n");
	}
}

int hpb_init(struct ufs_hba *hba)
{
	unsigned long flags;

	spin_lock_irqsave(hba->host->host_lock, flags);
	if (ufshcd_eh_in_progress(hba) || hba->pm_op_in_progress) {
		spin_unlock_irqrestore(hba->host->host_lock, flags);
		return 0;
	}
	spin_unlock_irqrestore(hba->host->host_lock, flags);
	hba->ufs_hpb = kzalloc(sizeof(struct ufs_hpb_info), GFP_KERNEL);
	if (!hba->ufs_hpb)
		return -EINVAL;
	pr_err("%s: support ufs hpb\n", __func__);
#ifdef CONFIG_HISI_DEBUG_FS
	ufshpb_debug_init(hba);
#endif
	hba->ufs_hpb->ufshpb_state = HPB_NEED_INIT;
	hba->ufs_hpb->ufs_hpb_init_lru = ufs_hpb_init_lru;
	hba->ufs_hpb->hba = hba;
	hba->ufs_hpb->ufs_hpb_support = 1;
	INIT_DELAYED_WORK(&hba->ufs_hpb->ufshpb_init_work, ufshpb_init_handler);
	pr_err("%s: start init work schedule\n", __func__);
	schedule_delayed_work(&hba->ufs_hpb->ufshpb_init_work,
			      msecs_to_jiffies(0));
	return 0;
}

int ufs_hpb_init_lru(struct ufs_hba *hba)
{
	unsigned int lun;
	unsigned int ret = 0;
	bool hpb_reset_status = true;

	pm_runtime_get_sync(hba->dev);

	if (hba->ufs_hpb->ufshpb_state != HPB_NEED_INIT) {
		pr_err("ufs hpb has been inited\n");
		goto out;
	}

	init_hpb_radix_tree();
	mutex_init(&hba->update_lock);

	if (hba->manufacturer_id != UFS_VENDOR_SAMSUNG) {
		ret = hpb_reset_process(hba, &hpb_reset_status);
		if (ret)
			goto out;
	}

	pr_err("ufs hpb reset status %x\n", hpb_reset_status);
	for (lun = 0; lun < MAX_HPB_LUN_NUM; lun++) {
		hba->ufs_hpb->ufshpb_lu_control[lun] = NULL;
		if (lun == HPB_LUN_SELECT) {
			ret = control_init_per_lun(hba, lun);
			if (ret)
				goto out;
		}
	}

	if (hba->manufacturer_id != UFS_VENDOR_SAMSUNG)
		pre_load_ppn(hba);
	INIT_WORK(&hba->hpb_update_work, ufshpb_update_entry_hanler);
	hba->ufs_hpb->ufshpb_state = UFSHPB_PRESENT;

	pr_err("now ufshpb status is %d\n", hba->ufs_hpb->ufshpb_state);
	pm_runtime_put_sync(hba->dev);
	return ret;
out:
	kfree(hba->ufs_hpb);
	hba->ufs_hpb = NULL;
	pm_runtime_put_sync(hba->dev);
	return ret;
}

#ifdef CONFIG_HISI_DEBUG_FS
static void ufs_hpb_basic_print(struct ufs_hba *hba)
{
	pr_err("hpb one region ppn size 0x%x\n",
	       hba->ufs_hpb->one_region_ppn_size);
	pr_err("hpb region num 4k 0x%x\n", hba->ufs_hpb->region_num_4k);

	pr_err("bud0 base offset 0x%x bud config plength 0x%x\n",
	       hba->ufs_hpb->bud0_base_offset,
	       hba->ufs_hpb->bud_config_plength);
	pr_err("hpb version 0x%x\n", hba->ufs_hpb->hpb_version);
	pr_err("hpb lu number 0x%x hpb region size 0x%x\n",
	       hba->ufs_hpb->hpb_lu_number,
	       hba->ufs_hpb->one_region_physical_size);
	pr_err("hpb subregion size 0x%x hpb device max active region 0x%x\n",
	       hba->ufs_hpb->one_subregion_physical_size,
	       hba->ufs_hpb->hpb_device_max_active_regions);
	pr_err("hpb lun enable 0x%x, hpb logical block size 0x%x, hpb logical block count 0x%x\n",
	       hba->ufs_hpb->hpb_lu_enable[HPB_UNIT],
	       hba->ufs_hpb->hpb_logical_block_size,
	       hba->ufs_hpb->hpb_logical_block_count);
	pr_err("hpb lu max active regions 0x%x, hpb pinned region startidx 0x%x, hpb pinned region num 0x%x\n",
	       hba->ufs_hpb->hpb_lu_max_active_regions[HPB_UNIT],
	       hba->ufs_hpb->hpb_lu_pinned_region_startidx[HPB_UNIT],
	       hba->ufs_hpb->hpb_lu_pinned_region_num[HPB_UNIT]);

	pr_err("%s, mode contrl %d\n", __func__,
	       hba->ufs_hpb->hpb_control_mode);

	pr_err("%s, one subregion ppn number %x\n", __func__,
	       hba->ufs_hpb->one_subregion_ppn_number);

	pr_err("%s, one node ppn size %x, one node ppn number %x\n",
	       __func__, hba->ufs_hpb->one_node_ppn_size,
	       hba->ufs_hpb->one_node_ppn_number);
}

static ssize_t hufs_hpb_debug_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct ufs_hba *hba = dev_get_drvdata(dev);

	if (hba->ufs_hpb->ufshpb_debug_flag == UFSHPB_TURN_OFF)
		return snprintf(buf, PAGE_SIZE, "%s\n", "ufshpb_turn_off");
	else if (hba->ufs_hpb->ufshpb_debug_flag == UFSHPB_TURN_ON)
		return snprintf(buf, PAGE_SIZE, "%s\n", "ufshpb_turn_on");
	else if (hba->ufs_hpb->ufshpb_debug_flag == UFSHPB_PRINT_ALL_INFO)
		return snprintf(buf, PAGE_SIZE, "%s\n", "ufshpb_print_info");
	else
		return snprintf(buf, PAGE_SIZE, "%s\n", "ufshpb_default");
}

static ssize_t hufs_hpb_debug_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct ufs_hba *hba = dev_get_drvdata(dev);

	if (sysfs_streq(buf, "ufshpb_default")) {
		hba->ufs_hpb->ufshpb_debug_flag = 0;
	} else if (sysfs_streq(buf, "ufshpb_turn_off")) {
		hba->ufs_hpb->ufshpb_debug_flag = UFSHPB_TURN_OFF;
		hba->ufs_hpb->ufshpb_state = UFSHPB_NOT_SUPPORTED;
	} else if (sysfs_streq(buf, "ufshpb_turn_on")) {
		hba->ufs_hpb->ufshpb_debug_flag = UFSHPB_TURN_ON;
		hba->ufs_hpb->ufshpb_state = UFSHPB_PRESENT;
	} else if (sysfs_streq(buf, "ufshpb_print_info")) {
		hba->ufs_hpb->ufshpb_debug_flag = UFSHPB_PRINT_ALL_INFO;
		ufs_hpb_basic_print(hba);
	} else {
		dev_err(hba->dev, "%s: invalid input debug parameter.\n",
			__func__);
		return -EINVAL;
	}

	return count;
}

void ufshpb_debug_init(struct ufs_hba *hba)
{
	hba->ufs_hpb->ufshpb_debug_flag = 0;

	hba->ufs_hpb->ufshpb_debug_state.ufshpb_attr.show =
		hufs_hpb_debug_show;
	hba->ufs_hpb->ufshpb_debug_state.ufshpb_attr.store =
		hufs_hpb_debug_store;
	sysfs_attr_init(&hba->ufs_hpb->ufshpb_debug_state.ufshpb_attr.attr);
	hba->ufs_hpb->ufshpb_debug_state.ufshpb_attr.attr.name = "ufshpb_debug";
	hba->ufs_hpb->ufshpb_debug_state.ufshpb_attr.attr.mode =
		0640; /* 0640 node attribute mode */
	if (device_create_file(hba->dev,
			       &hba->ufs_hpb->ufshpb_debug_state.ufshpb_attr))
		dev_err(hba->dev, "Failed to create sysfs for ufshpb\n");
}
#endif

static int read_device_desc(struct ufs_hba *hba, u8 *device_desc_buf)
{
	int ret;
	int size = HPB_DESC_DEVICE_MAX_SIZE;

	if (hba->manufacturer_id == UFS_VENDOR_SAMSUNG)
		size = HPB_DESC_DEVICE_MAX_SIZE_SAMSUNG;

	switch (hba->manufacturer_id) {
	case UFS_VENDOR_SAMSUNG:
		ret = ufshcd_query_descriptor_retry(
			hba, UPIU_QUERY_OPCODE_READ_DESC, QUERY_DESC_IDN_DEVICE,
			0, HPB_DEVICE_DESC_SELECTOR_SAMSUNG, device_desc_buf,
			&size);
		if (ret)
			goto out;
		break;
	case UFS_VENDOR_TOSHIBA:
	case UFS_VENDOR_SKHYNIX:
		ret = ufshcd_query_descriptor_retry(
			hba, UPIU_QUERY_OPCODE_READ_DESC, QUERY_DESC_IDN_DEVICE,
			0, HPB_DEVICE_DESC_SELECTOR_PROTOCOL, device_desc_buf,
			&size);
		if (ret)
			goto out;
		break;
	default:
		pr_err("%s: no such product\n", __func__);
	}

	return 0;
out:
	pr_err("%s: read device desc error\n", __func__);
	return -EINVAL;
}

int feature_open(struct ufs_hba *hba)
{
	if ((hba->manufacturer_id == UFS_VENDOR_TOSHIBA) &&
	    (hba->caps & UFSHCD_CAP_HPB_HOST_CONTROL))
		return 1;

	if ((hba->manufacturer_id == UFS_VENDOR_SKHYNIX) &&
	    (hba->caps & UFSHCD_CAP_HPB_HOST_CONTROL_HYNIX))
		return 1;

	if ((hba->manufacturer_id == UFS_VENDOR_SAMSUNG) &&
	    (hba->caps & UFSHCD_CAP_HPB_DEVICE_CONTROL))
		return 1;

	return 0;
}

int ufshpb_support(struct ufs_hba *hba)
{
	int ret;
	u64 ufs_hpb_support;
	u8 *device_desc_buf = NULL;
	u64 support_offset;

	device_desc_buf = kzalloc(HPB_DESC_DEVICE_MAX_SIZE_1862_SUPPORT,
				  GFP_KERNEL);
	if (!device_desc_buf)
		return 0;

	ret = read_device_desc(hba, device_desc_buf);

	if (ret) {
		pr_err("%s: read ufs hpb support err, don't use hpb\n",
		       __func__);
		goto out;
	}

	support_offset = HPB_DEVICE_DESC_DEXTENDED_UFSFEATURE_SUPPORT;

	ufs_hpb_support =
		cpu_to_be32(*((u32 *)(&device_desc_buf[support_offset])));

	pr_err("%s: hpb support %llx\n", __func__, ufs_hpb_support);

out:
	kfree(device_desc_buf);
	if (ret)
		return 0;
	return ufs_hpb_support & HPB_SUPPORT_BIT;
}

static inline void
empty_node_update_process(struct subregion_node *subregion_node,
			  struct ndlist_data *ndlist_data, u64 subregion)
{
	subregion_node->subregion_id = subregion;
	subregion_node->status = NODE_UPDATING;
	idn_node_insert(subregion_node->subregion_id, subregion_node);

	list_add(&subregion_node->lru_list, &ndlist_data->lru_list);
	list_add_tail(&subregion_node->active_list, &ndlist_data->active_list);
#ifdef HPB_HUFS_DEBUG_PRINT
	pr_err("%s: hit miss node id %x node status %x\n", __func__,
	       subregion_node->subregion_id, subregion_node->status);
#endif
	queue_work(system_freezable_wq, &ndlist_data->run_active_work);
}

static inline void
new_node_update_process(struct subregion_node *subregion_node,
			struct ndlist_data *ndlist_data, u64 subregion)
{
	subregion_node->subregion_id_old = subregion_node->subregion_id;
	subregion_node->subregion_id = subregion;
	subregion_node->status = NODE_INACTIVATING;
	idn_node_delete(subregion_node->subregion_id_old, subregion_node);

	idn_node_insert(subregion_node->subregion_id, subregion_node);

	list_add(&subregion_node->lru_list, &ndlist_data->lru_list);
	list_add_tail(&subregion_node->inactive_list,
		      &ndlist_data->inactive_list);

#ifdef HPB_HUFS_DEBUG_PRINT
	pr_err("%s: hit miss node id %x node status %x\n", __func__,
	       subregion_node->subregion_id, subregion_node->status);
#endif
	queue_work(system_freezable_wq, &ndlist_data->run_inactive_work);
}

static int valid_4k_read_check(struct ufs_hba *hba, struct ufshcd_lrb *lrbp,
			       struct ufshpb_lu_control **hpb_lu_control)
{
	u64 sector;
	u64 sector_cnt;

	if ((lrbp->cmd->request == NULL) || (lrbp->lun != HPB_UNIT))
		goto out;

	if (!ufshpb_is_read_lrbp(lrbp))
		goto out;

	sector = blk_rq_pos(lrbp->cmd->request);
	sector_cnt = blk_rq_sectors(lrbp->cmd->request);
	if (sector_cnt != ONE_BLOCK_SECTORS) {
#ifdef HPB_HUFS_DEBUG_PRINT
		pr_err("%s: hpb don't support read exceed 4k\n",
		       __func__);
#endif
		goto out;
	}
	*hpb_lu_control = hba->ufs_hpb->ufshpb_lu_control[lrbp->lun];

	if (!(*hpb_lu_control)) {
		pr_err("%s: lu don't have lu control struct\n",
		       __func__);
		goto out;
	}
#ifdef HPB_HUFS_DEBUG_PRINT
	pr_err("%s: current sector %lx sector_cnt %lx current lun %x\n",
	       __func__, sector, sector_cnt, lrbp->lun);
#endif
	return 0;
out:
	return -EINVAL;
}

static int invalid_4k_read_check_dev_mode(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	if (!hba->ufs_hpb)
		goto out;

	if (hba->ufs_hpb->ufshpb_state != UFSHPB_PRESENT)
		goto out;

	if ((lrbp->cmd->request == NULL) || (lrbp->lun != HPB_UNIT))
		goto out;

	if (!ufshpb_is_read_lrbp(lrbp))
		goto out;

	return 0;
out:
	return 1;
}

void ufshpb_prep_for_dev_mode(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	struct ufshpb_lu_control *hpb_lu_control = NULL;
	u64 lba;
	u64 ppn;
	u64 ppn_offset, subregion;
	unsigned long long sector;
	unsigned int sector_cnt;
	struct subregion_node *subregion_node = NULL;
	unsigned long flags;

	if (invalid_4k_read_check_dev_mode(hba, lrbp))
		return;
#ifdef HPB_HISI_DEBUG_STATIC
	hba->rd_cnt++;
#endif
	sector = blk_rq_pos(lrbp->cmd->request);
	sector_cnt = blk_rq_sectors(lrbp->cmd->request);

	if (sector_cnt < MIN_SECTOR_SAMSUNG || sector_cnt > MAX_SECTOR_SAMSUNG) {
#ifdef HPB_HUFS_DEBUG_PRINT
		pr_err("%s: hpb don't support read exceedi 4-32k\n", __func__);
#endif
		return;
	}
	hba->ufshpb_4k_total++;
	lba = sector / ONE_BLOCK_SECTORS;
#ifdef HPB_HUFS_DEBUG_PRINT
	pr_err("%s: current lba %llx, sector %llx, sector cnt %llx, lu %d\n",
	       __func__, lba, sector, sector_cnt, lrbp->lun);
#endif
	hpb_lu_control = hba->ufs_hpb->ufshpb_lu_control[lrbp->lun];
	if (!hpb_lu_control)
		return;
	subregion = lba / hba->ufs_hpb->one_subregion_ppn_number;
	ppn_offset = lba % hba->ufs_hpb->one_subregion_ppn_number;

	spin_lock_irqsave(hba->host->host_lock, flags);
	subregion_node = node_addr_regionid(subregion);
	spin_unlock_irqrestore(hba->host->host_lock, flags);

	if (subregion_node) {
		if (subregion_node->status != NODE_NEW)
			return;
		ppn = *(subregion_node->node_addr + ppn_offset);
		ufshpb_ppn_prep(hba, lrbp, ppn);
		hba->ufshpb_hit_count++;
		lrbp->regionid = subregion;
#ifdef HPB_HISI_DEBUG_STATIC
		hba->hit_nd_cnt++;
#endif
	}
}

void ufshpb_prep_fn_lru(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	struct ufshpb_lu_control *hpb_lu_control = NULL;
	unsigned int lba;
	u64 ppn;
	u64 ppn_offset, subregion;
	struct subregion_node *subregion_node = NULL;
	struct ndlist_data *ndlist_data = NULL;

	if (valid_4k_read_check(hba, lrbp, &hpb_lu_control))
		return;
	hba->ufshpb_4k_total++;
	ndlist_data = hpb_lu_control->ndlist_data;
	lba = blk_rq_pos(lrbp->cmd->request) / ONE_BLOCK_SECTORS;
	subregion = lba / hba->ufs_hpb->one_subregion_ppn_number;
	ppn_offset = lba % hba->ufs_hpb->one_subregion_ppn_number;
	subregion_node = node_addr_regionid(subregion);
#ifdef HPB_HUFS_DEBUG_PRINT
	pr_err("%s, node id %x, offset %x\n", __func__, subregion,
	       ppn_offset);
#endif
	if (subregion_node) {
		if (subregion_node->status != NODE_NEW)
			return;
		ppn = *(subregion_node->node_addr + ppn_offset);
		ufshpb_ppn_prep(hba, lrbp, ppn);
		hba->ufshpb_hit_count++;
#ifdef HPB_HUFS_DEBUG_PRINT
		pr_err("%s: hit node %x\n", __func__,
		       subregion_node->subregion_id);
#endif
	} else {
		spin_lock(&ndlist_data->lock);
		subregion_node = container_of((&(ndlist_data->lru_list))->prev,
					      struct subregion_node, lru_list);
		spin_unlock(&ndlist_data->lock);
		if (subregion_node->status == NODE_EMPTY) {
			list_del_init(&subregion_node->lru_list);

			empty_node_update_process(subregion_node, ndlist_data,
						  subregion);
		} else if (subregion_node->status == NODE_NEW) {
			list_del_init(&subregion_node->lru_list);
			new_node_update_process(subregion_node, ndlist_data,
						subregion);
		}
	}
}

void ufshpb_prep_fn_condense(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	struct ufshpb_lu_control *hpb_lu_control = NULL;
	unsigned int lba;
	u64 ppn;
	u64 ppn_offset, subregion;
	struct subregion_node *subregion_node = NULL;

	if (valid_4k_read_check(hba, lrbp, &hpb_lu_control))
		return;
	hba->ufshpb_4k_total++;
	lba = blk_rq_pos(lrbp->cmd->request) / ONE_BLOCK_SECTORS;
	subregion = lba / hba->ufs_hpb->one_subregion_ppn_number;

	if (subregion >= (HPB_ALLOC_MEM_COUNT * HPB_MEMSIZE_PER_LOOP /
			  HPB_SUBREGION_PPN_SIZE))
		return;

	if (!mutex_trylock(&hba->update_lock))
		return;
	ppn_offset = lba % hba->ufs_hpb->one_subregion_ppn_number;
	subregion_node = hpb_lu_control->subregion_array[subregion];
#ifdef HPB_HUFS_DEBUG_PRINT
	pr_err("%s, node id %x, offset %x\n", __func__, subregion,
	       ppn_offset);
	if (subregion_node->status == NODE_EMPTY)
		pr_err("%s: node id  %x, node empty\n", __func__,
		       subregion_node->subregion_id);
	if (subregion_node->status == NODE_UPDATING)
		pr_err("%s: node id  %x, node updating\n", __func__,
		       subregion_node->subregion_id);
#endif
	if (subregion_node->status != NODE_NEW) {
		mutex_unlock(&hba->update_lock);
		return;
	}
	ppn = *(subregion_node->node_addr + ppn_offset);
	ufshpb_ppn_prep(hba, lrbp, ppn);
	mutex_unlock(&hba->update_lock);
	hba->ufshpb_hit_count++;
}

void parse_hpb_dts(struct ufs_hba *hba, struct device_node *np)
{
	if (of_find_property(np, "ufs-use-hpb-host-control-hynix", NULL))
		hba->caps |= UFSHCD_CAP_HPB_HOST_CONTROL_HYNIX;

	if (of_find_property(np, "ufs-use-hpb-host-control", NULL))
		hba->caps |= UFSHCD_CAP_HPB_HOST_CONTROL;

	if (of_find_property(np, "ufs-use-hpb-device-control", NULL))
		hba->caps |= UFSHCD_CAP_HPB_DEVICE_CONTROL;

	if (of_find_property(np, "ufs-use-turbo-table-host-control", NULL))
		hba->caps |= UFSHCD_CAP_HPB_TURBO_TABLE;
}

void suspend_wait_hpb(struct ufs_hba *hba)
{
	struct ndlist_data *ndlist_data = NULL;
	struct ufshpb_lu_control *hpb_lu_control = NULL;

	if (hba->ufs_hpb && (hba->ufs_hpb->ufshpb_state == UFSHPB_PRESENT)) {
		hpb_lu_control = hba->ufs_hpb->ufshpb_lu_control[HPB_UNIT];
		ndlist_data = hpb_lu_control->ndlist_data;

		if (hba->manufacturer_id == UFS_VENDOR_SAMSUNG)
			cancel_work_sync(&hba->hpb_update_work);
		else
			cancel_work_sync(&ndlist_data->run_active_work);
	}
}

MODULE_AUTHOR("HUFS");
MODULE_DESCRIPTION("HPB FEATURE");
MODULE_LICENSE("GPL");
MODULE_VERSION("V1.0");
