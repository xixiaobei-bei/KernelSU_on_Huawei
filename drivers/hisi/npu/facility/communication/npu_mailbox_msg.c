/*
 * npu_mailbox_msg.c
 *
 * about npu mailbox msg
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
 *
 */
#include "npu_mailbox_msg.h"
#include <linux/hisi/hisi_svm.h>
#include <linux/sched.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
#include <linux/sched/mm.h>
#endif

#include "npu_shm.h"
#include "npu_stream.h"
#include "npu_calc_sq.h"
#include "npu_calc_cq.h"
#include "npu_mailbox.h"
#include "npu_calc_cq.h"
#include "npu_svm.h"

static int hisi_svm_get_ttbr(u64 *ttbr, u64 *tcr)
{
	unsigned long asid;
	struct mm_struct *mm = NULL;

	if ((ttbr == NULL) || (tcr == NULL)) {
		npu_drv_err("invalid ttbr or tcr\n");
		return -EINVAL;
	}
	mm = get_task_mm(current);
	if (mm == NULL) {
		npu_drv_err("get mm is null\n");
		return -EINVAL;
	}
	// flush cache? ion_flush_all_cpus_caches
	asid = ASID(mm);
	*ttbr = virt_to_phys(mm->pgd) | (asid << 48);
	*tcr  = read_sysreg(tcr_el1);

	npu_drv_debug("pgdaddr:0x:%pK, context:0x%pK, pa:0x%pK\n",
		mm->pgd, (u64 *)(mm->pgd), (void *)(uintptr_t)virt_to_phys(mm->pgd));

	mmput(mm);

	npu_drv_debug("asid:%lu ,ttbr:0x%pK, tcr:0x%pK\n",
		asid, (void *)(uintptr_t)*ttbr, (void *)(uintptr_t)*tcr);

	return 0;
}

static int __npu_create_alloc_stream_msg(u8 dev_id, u32 stream_id,
	struct npu_stream_msg *stream_msg)
{
	phys_addr_t sq_phy_addr = 0;
	phys_addr_t cq_phy_addr = 0;
	int ret;
	u32 sq_index;
	u32 cq_index;
	u32 cq_stream_num;
	struct npu_ts_sq_info *sq_info = NULL;
	struct npu_ts_cq_info *cq_info = NULL;
	struct npu_stream_info *stream_info = NULL;
	struct npu_dev_ctx *dev_ctx = NULL;
	struct npu_sync_cq_info *sync_cq_info = NULL;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_error(plat_info == NULL, -1,
		"npu_plat_get_info failed dev id %d\n", dev_id);
	cond_return_error(dev_id >= NPU_DEV_NUM, -1, "illegal npu dev id %d\n",
		dev_id);
	cond_return_error(stream_id >= NPU_MAX_STREAM_ID, -1,
		"illegal npu stream id %d\n", stream_id);
	cond_return_error(stream_msg == NULL, -1,
		"illegal para ,stream_msg is null\n");

	stream_info = npu_calc_stream_info(dev_id, stream_id);
	cond_return_error(stream_info == NULL, -1, "npu dev id %d stream_id = %d "
		"stream_info ,stream_info is null\n", dev_id, stream_id);

	dev_ctx = get_dev_ctx_by_id(dev_id);
	cond_return_error(dev_ctx == NULL, -1,
		"get_dev_ctx_by_id error, dev_id = %d\n", dev_id);

	ret = npu_get_ssid_bypid(dev_ctx->devid, current->tgid, current->tgid,
		&(stream_msg->ssid), &(stream_msg->ttbr), &(stream_msg->tcr));
	cond_return_error(ret != 0, ret, "fail to get ssid, ret = %d\n", ret);

	ret = hisi_svm_get_ttbr(&(stream_msg->ttbr), &(stream_msg->tcr));
	cond_return_error(ret != 0, ret, "fail to get process info, ret = %d\n",
		ret);

	sq_index = stream_info->sq_index;
	cq_index = stream_info->cq_index;
	sq_info = npu_calc_sq_info(dev_id, sq_index);
	cq_info = npu_calc_cq_info(dev_id, cq_index);
	cond_return_error(cq_info == NULL || sq_info == NULL, -1,
		"npu_calc_cq_info or sq_info error, cq_index = %d, sq_index = %d\n",
		cq_index, sq_index);
	// get sq and cq phy addr of cur stream
	(void)npu_get_sq_phy_addr(dev_id, sq_index, &sq_phy_addr);
	(void)npu_get_cq_phy_addr(dev_id, cq_index, &cq_phy_addr);

	/* add message header */
	stream_msg->valid = NPU_MAILBOX_MESSAGE_VALID;
	stream_msg->cmd_type = NPU_MAILBOX_CREATE_CQSQ_CALC;
	stream_msg->result = 0;

	/* add payload */
	stream_msg->sq_index = sq_index;
	stream_msg->sq_addr = sq_phy_addr;
	stream_msg->cq0_addr = cq_phy_addr;

	sync_cq_info = (struct npu_sync_cq_info *)(dev_ctx->sync_cq);
	stream_msg->cq1_index = stream_info->sync_cq_index;
	stream_msg->cq1_addr = sync_cq_info->sync_cq_paddr;
	cq_stream_num = npu_get_cq_ref_by_communication_stream(dev_id, cq_index);
	if (cq_stream_num > 1) {
		stream_msg->cq0_addr = 0;
		npu_drv_debug("cur cq %d has been reference by %d streams and cq_addr should be zero to "
			"inform ts when alloc stream id %d\n", cq_info->index,
			cq_stream_num, stream_id);
	}

	npu_drv_debug("cur cq %d has been reference by %d streams inform ts stream_id = %d\n",
		cq_info->index, cq_stream_num, stream_info->id);

	stream_msg->cq0_index = cq_index;
	stream_msg->stream_id = stream_id;
	stream_msg->plat_type = plat_info->plat_type;
	stream_msg->cq_slot_size = cq_info->slot_size;
	npu_drv_debug("create alloc stream msg :"
		"stream_msg->valid = %d  stream_msg->cmd_type = %d  "
		"stream_msg->result = %d  stream_msg->sq_index = %d "
		"stream_msg->sq_addr = %pK  stream_msg->cq0_index = %d "
		"stream_msg->cq0_addr = %pK  stream_msg->cq1_index = %d "
		"stream_msg->cq1_addr = %pK  stream_msg->stream_id = %d "
		"stream_msg->plat_type = %d  stream_msg->cq_slot_size = %d\n",
		stream_msg->valid, stream_msg->cmd_type,
		stream_msg->result, stream_msg->sq_index,
		(void *)(uintptr_t) stream_msg->sq_addr, stream_msg->cq0_index,
		(void *)(uintptr_t) stream_msg->cq0_addr, stream_msg->cq1_index,
		(void *)(uintptr_t) stream_msg->cq1_addr, stream_msg->stream_id,
		stream_msg->plat_type, stream_msg->cq_slot_size);

	return 0;
}

static int __npu_create_free_stream_msg(u8 dev_id, u32 stream_id,
	struct npu_stream_msg *stream_msg)
{
	struct npu_stream_info *stream_info = NULL;
	struct npu_ts_sq_info *sq_info = NULL;
	struct npu_ts_cq_info *cq_info = NULL;
	u32 sq_index;
	u32 cq_index;
	u32 cq_stream_num;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_error(plat_info == NULL, -1,
		"npu_plat_get_info failed dev id %d\n", dev_id);
	cond_return_error(dev_id >= NPU_DEV_NUM, -1, "illegal npu dev id %d\n",
		dev_id);
	cond_return_error(stream_id >= NPU_MAX_STREAM_ID, -1,
		"illegal npu stream id %d\n", stream_id);
	cond_return_error(stream_msg == NULL, -1,
		"illegal para ,stream_msg is null\n");

	stream_info = npu_calc_stream_info(dev_id, stream_id);
	cond_return_error(stream_info == NULL, -1,
		"npu dev id %d stream_id = %d `s "
		"stream_info ,stream_info is null\n", dev_id, stream_id);

	sq_index = stream_info->sq_index;
	cq_index = stream_info->cq_index;
	sq_info = npu_calc_sq_info(dev_id, sq_index);
	cq_info = npu_calc_cq_info(dev_id, cq_index);
	cond_return_error(cq_info == NULL || sq_info == NULL, -1,
		"npu_calc_cq_info or sq_info error, cq_index = %d, sq_index = %d\n",
		cq_index, sq_index);
	/* add message header */
	stream_msg->valid = NPU_MAILBOX_MESSAGE_VALID;
	stream_msg->cmd_type = NPU_MAILBOX_RELEASE_CQSQ_CALC;
	stream_msg->result = 0;

	/* add payload */
	// no need carry address info when free stream
	stream_msg->sq_addr = 0;
	stream_msg->cq0_addr = 0;
	stream_msg->sq_index = sq_index;
	stream_msg->cq0_index = cq_index;

	cq_stream_num = npu_get_cq_ref_by_stream(dev_id, cq_index);
	if (cq_stream_num > 1) {
		stream_msg->cq0_index = NPU_MAILBOX_INVALID_SQCQ_INDEX;
		npu_drv_debug("no need free stream`s cq now because "
			"it is not the last stream reference of cq %d\n", cq_index);
	} else {
		npu_drv_debug("should free stream`s cq now because it is the last "
			"stream reference of cq %d cq_info->stream_num = %d\n",
			cq_index, cq_stream_num);
	}

	stream_msg->stream_id = stream_id;
	stream_msg->plat_type = plat_info->plat_type;
	stream_msg->cq_slot_size = cq_info->slot_size;

	npu_drv_debug("create free stream msg : stream_msg->valid = %d "
		"stream_msg->cmd_type = %d stream_msg->result = %d stream_msg->sq_index = %d\n"
		"stream_msg->sq_addr = %pK stream_msg->cq0_index = %d "
		"stream_msg->cq0_addr = %pK stream_msg->stream_id = %d\n"
		"stream_msg->plat_type = %d stream_msg->cq_slot_size = %d\n",
		stream_msg->valid, stream_msg->cmd_type,
		stream_msg->result, stream_msg->sq_index,
		(void *)(uintptr_t) stream_msg->sq_addr, stream_msg->cq0_index,
		(void *)(uintptr_t) stream_msg->cq0_addr, stream_msg->stream_id,
		stream_msg->plat_type, stream_msg->cq_slot_size);
	return 0;
}

// build stream_msg  about stream to inform ts
// stream_msg is output para
// called after bind stream with sq and cq
int npu_create_alloc_stream_msg(u8 dev_id, u32 stream_id,
	struct npu_stream_msg *stream_msg)
{
	int ret = __npu_create_alloc_stream_msg(dev_id, stream_id, stream_msg);

	return ret;
}

int npu_create_free_stream_msg(u8 dev_id, u32 stream_id,
	struct npu_stream_msg *stream_msg)
{
	int ret = __npu_create_free_stream_msg(dev_id, stream_id, stream_msg);

	return ret;
}
