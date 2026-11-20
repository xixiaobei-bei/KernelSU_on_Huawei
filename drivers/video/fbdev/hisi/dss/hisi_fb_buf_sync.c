/* Copyright (c) 2008-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "hisi_fb.h"
#define HISI_DSS_LAYERBUF_FREE "dpufb%u-layerbuf-free"

/* layerbuffer handle, for online compose */
static bool hisi_check_parameter_valid(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, struct list_head *plock_list)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL!\n");
		return false;
	}
	if (!pov_req) {
		DPU_FB_ERR("pov_req is NULL!\n");
		return false;
	}
	if (!plock_list) {
		DPU_FB_ERR("plock_list is NULL!\n");
		return false;
	}

	return true;
}

static int dpufb_layerbuf_block_lock(dss_overlay_t *pov_req, struct list_head *plock_list,
	dss_layer_t *layer)
{
	struct dpufb_layerbuf *node = NULL;
	struct dma_buf *buf_handle = NULL;

	buf_handle = hisi_dss_get_dmabuf(layer->img.shared_fd);
	if (!buf_handle)
		return -EINVAL;

	node = (struct dpufb_layerbuf *)kzalloc(sizeof(struct dpufb_layerbuf), GFP_KERNEL);
	if (!node) {
		DPU_FB_ERR("layer_idx%d, failed to kzalloc!\n", layer->layer_idx);

		hisi_dss_put_dmabuf(buf_handle);
		buf_handle = NULL;
		return -ENOMEM;
	}

	node->shared_fd = layer->img.shared_fd;
	node->buffer_handle = buf_handle;
	node->frame_no = pov_req->frame_no;
	node->timeline = 0;

	/* mmbuf */
	node->mmbuf.addr = layer->img.mmbuf_base;
	node->mmbuf.size = layer->img.mmbuf_size;

	node->vir_addr = layer->img.vir_addr;
	node->chn_idx = layer->chn_idx;

	list_add_tail(&node->list_node, plock_list);

	if (g_debug_layerbuf_sync)
		DPU_FB_INFO("frame_no=%u, shared_fd=%d, timeline=%u, mmbuf(0x%x, %u).\n",
			node->frame_no, node->shared_fd, node->timeline, node->mmbuf.addr, node->mmbuf.size);

	return 0;
}

int dpufb_layerbuf_lock(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, struct list_head *plock_list)
{
	dss_overlay_block_t *pov_h_block_infos = NULL;
	dss_overlay_block_t *pov_h_block = NULL;
	dss_layer_t *layer = NULL;
	bool parameter_valid;
	uint32_t i;
	uint32_t m;

	parameter_valid = hisi_check_parameter_valid(dpufd, pov_req, plock_list);
	if (!parameter_valid)
		return -EINVAL;

	pov_h_block_infos = (dss_overlay_block_t *)(uintptr_t)(pov_req->ov_block_infos_ptr);
	for (m = 0; m < pov_req->ov_block_nums; m++) {
		pov_h_block = &(pov_h_block_infos[m]);
		for (i = 0; i < pov_h_block->layer_nums; i++) {
			layer = &(pov_h_block->layer_infos[i]);

			if (layer->dst_rect.y < pov_h_block->ov_block_rect.y)
				continue;

			if (layer->need_cap & (CAP_DIM | CAP_BASE | CAP_PURE_COLOR))
				continue;

			if (dpufb_layerbuf_block_lock(pov_req, plock_list, layer)) {
				if (g_debug_layerbuf_sync)
					DPU_FB_INFO("fb%u, layer_idx = %d.\n", dpufd->index, i);
			}
		}
	}

	return 0;
}

void dpufb_layerbuf_flush(struct dpu_fb_data_type *dpufd,
	struct list_head *plock_list)
{
	struct dpufb_layerbuf *node = NULL;
	struct dpufb_layerbuf *_node_ = NULL;
	unsigned long flags = 0;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is nullptr!\n");
	dpu_check_and_no_retval(!plock_list, ERR, "plock_list is nullptr!\n");

	spin_lock_irqsave(&(dpufd->buf_sync_ctrl.layerbuf_spinlock), flags);
	dpufd->buf_sync_ctrl.layerbuf_flushed = true;
	list_for_each_entry_safe(node, _node_, plock_list, list_node) {
		list_del(&node->list_node);
		list_add_tail(&node->list_node, &(dpufd->buf_sync_ctrl.layerbuf_list));
	}
	spin_unlock_irqrestore(&(dpufd->buf_sync_ctrl.layerbuf_spinlock), flags);
}

void dpufb_layerbuf_unlock(struct dpu_fb_data_type *dpufd,
	struct list_head *pfree_list)
{
	struct dpufb_layerbuf *node = NULL;
	struct dpufb_layerbuf *_node_ = NULL;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is nullptr!\n");
	dpu_check_and_no_retval(!pfree_list, ERR, "pfree_list is nullptr!\n");

	list_for_each_entry_safe(node, _node_, pfree_list, list_node) {
		list_del(&node->list_node);

		if (g_debug_layerbuf_sync)
			DPU_FB_INFO("fb%u, frame_no=%u, share_fd=%d, buffer_handle=%pK, "
				"timeline=%u, mmbuf(0x%x, %u), vir_addr = 0x%llx, chn_idx = %d\n",
				dpufd->index, node->frame_no, node->shared_fd, node->buffer_handle,
				node->timeline, node->mmbuf.addr, node->mmbuf.size, node->vir_addr, node->chn_idx);

		node->timeline = 0;
		if (node->buffer_handle) {
			hisi_dss_put_dmabuf(node->buffer_handle);
			node->buffer_handle = NULL;
		}
		kfree(node);
	}
}

void dpufb_layerbuf_lock_exception(struct dpu_fb_data_type *dpufd,
	struct list_head *plock_list)
{
	unsigned long flags = 0;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is nullptr!\n");
	dpu_check_and_no_retval(!plock_list, ERR, "plock_list is nullptr!\n");

	spin_lock_irqsave(&(dpufd->buf_sync_ctrl.layerbuf_spinlock), flags);
	dpufd->buf_sync_ctrl.layerbuf_flushed = false;
	spin_unlock_irqrestore(&(dpufd->buf_sync_ctrl.layerbuf_spinlock), flags);

	dpufb_layerbuf_unlock(dpufd, plock_list);
}

static void dpufb_layerbuf_unlock_work(struct work_struct *work)
{
	struct dpufb_buf_sync *pbuf_sync = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpufb_layerbuf *node = NULL;
	struct dpufb_layerbuf *_node_ = NULL;
	struct list_head free_list;
	unsigned long flags = 0;

	pbuf_sync = container_of(work, struct dpufb_buf_sync, free_layerbuf_work);
	dpu_check_and_no_retval(!pbuf_sync, ERR, "pbuf_sync is nullptr!\n");

	dpufd = container_of(pbuf_sync, struct dpu_fb_data_type, buf_sync_ctrl);
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is nullptr!\n");

	INIT_LIST_HEAD(&free_list);
	down(&pbuf_sync->layerbuf_sem);
	spin_lock_irqsave(&pbuf_sync->layerbuf_spinlock, flags);
	list_for_each_entry_safe(node, _node_, &pbuf_sync->layerbuf_list, list_node) {
		if (node->timeline >= 2) {
			list_del(&node->list_node);
			list_add_tail(&node->list_node, &free_list);
		}
	}
	spin_unlock_irqrestore(&pbuf_sync->layerbuf_spinlock, flags);
	up(&pbuf_sync->layerbuf_sem);
	dpufb_layerbuf_unlock(dpufd, &free_list);
}

static int dpufb_wb_layer_lock(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	struct list_head *plock_list, dss_wb_layer_t *wb_layer4block)
{
	struct dpufb_layerbuf *node = NULL;
	struct dma_buf *buf_handle = NULL;

	buf_handle = hisi_dss_get_dmabuf(wb_layer4block->dst.shared_fd);
	if (buf_handle) {
		node = kzalloc(sizeof(struct dpufb_layerbuf), GFP_KERNEL);
		if (!node) {
			hisi_dss_put_dmabuf(buf_handle);
			buf_handle = NULL;

			DPU_FB_ERR("fb%u, wb_layer4block failed to kzalloc!\n", dpufd->index);
			return -ENOMEM;
		}

		node->shared_fd = wb_layer4block->dst.shared_fd;
		node->buffer_handle = buf_handle;
		node->timeline = 2;  /* timeline to create the fence on */
		node->mmbuf.addr = wb_layer4block->dst.mmbuf_base;
		node->mmbuf.size = wb_layer4block->dst.mmbuf_size;

		node->vir_addr = wb_layer4block->dst.vir_addr;
		node->chn_idx = wb_layer4block->chn_idx;

		list_add_tail(&node->list_node, plock_list);
		if (g_debug_layerbuf_sync)
			DPU_FB_INFO("fb%u, shared_fd=%d, buffer_handle=%pK, mmbuf(0x%x, %u)\n",
				dpufd->index, node->shared_fd, node->buffer_handle,
				node->mmbuf.addr, node->mmbuf.size);
	}
	return 0;
}

static void dpufb_offline_layerbuf_block_lock(struct dpu_fb_data_type *dpufd,
	struct list_head *plock_list, dss_layer_t *layer)
{
	struct dma_buf *buf_handle = NULL;
	struct dpufb_layerbuf *node = NULL;

	buf_handle = hisi_dss_get_dmabuf(layer->img.shared_fd);
	if (!buf_handle)
		return;

	node = kzalloc(sizeof(struct dpufb_layerbuf), GFP_KERNEL);
	if (!node) {
		DPU_FB_ERR("fb%u, layer_idx%d, failed to kzalloc!\n", dpufd->index, layer->layer_idx);
		hisi_dss_put_dmabuf(buf_handle);
		buf_handle = NULL;
		return;
	}

	node->fence = (dpufd->index == MEDIACOMMON_PANEL_IDX) ?
		hisi_dss_get_fd_sync_fence(layer->acquire_fence) : NULL;
	node->shared_fd = layer->img.shared_fd;
	node->buffer_handle = buf_handle;
	node->timeline = 2;  /* timeline to create the fence on */

	node->mmbuf.addr = layer->img.mmbuf_base;
	node->mmbuf.size = layer->img.mmbuf_size;

	node->vir_addr = layer->img.vir_addr;
	node->chn_idx = layer->chn_idx;

	list_add_tail(&node->list_node, plock_list);

	if (g_debug_layerbuf_sync)
		DPU_FB_INFO("fb%u, acquire_fence=%d fence=%pK, shared_fd=%d, buffer_handle=%pK, mmbuf0x[%x, %d]\n",
			dpufd->index, layer->acquire_fence, node->fence, node->shared_fd,
			node->buffer_handle, node->mmbuf.addr, node->mmbuf.size);
}

int dpufb_offline_layerbuf_lock(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, struct list_head *plock_list)
{
	dss_overlay_block_t *pov_h_block_infos = NULL;
	dss_overlay_block_t *pov_h_block = NULL;
	dss_layer_t *layer = NULL;
	int ret;
	uint32_t i;
	uint32_t m;
	bool parameter_valid;

	parameter_valid = hisi_check_parameter_valid(dpufd, pov_req, plock_list);
	if (parameter_valid == false)
		return -EINVAL;


	ret = dpufb_wb_layer_lock(dpufd, pov_req, plock_list, &(pov_req->wb_layer_infos[0]));
	if (ret != 0)
		return ret;

	pov_h_block_infos = (dss_overlay_block_t *)(uintptr_t)(pov_req->ov_block_infos_ptr);
	for (m = 0; m < pov_req->ov_block_nums; m++) {
		pov_h_block = &(pov_h_block_infos[m]);
		for (i = 0; i < pov_h_block->layer_nums; i++) {
			layer = &(pov_h_block->layer_infos[i]);

			if (layer->dst_rect.y < pov_h_block->ov_block_rect.y)
				continue;

			if (layer->need_cap & (CAP_DIM | CAP_BASE | CAP_PURE_COLOR))
				continue;

			dpufb_offline_layerbuf_block_lock(dpufd, plock_list, layer);
		}
	}

	return 0;
}

/* buf sync fence */
#define BUF_SYNC_TIMEOUT_MSEC (4 * MSEC_PER_SEC)

/**
 * hisi_dss_fb_sync_get_fence() - get fence from timeline
 * @timeline: Timeline to create the fence on
 * @fence_name:	Name of the fence that will be created for debugging
 * @val: Timeline value at which the fence will be signaled
 *
 * Function returns a fence on the timeline given with the name provided.
 * The fence created will be signaled when the timeline is advanced.
 */
static struct hisi_dss_fence *hisi_dss_fb_sync_get_fence(struct hisi_dss_timeline *timeline,
		const char *fence_name, int val)
{
	struct hisi_dss_fence *fence = NULL;

	fence = hisi_dss_get_sync_fence(timeline, fence_name, NULL, val);
	if (IS_ERR_OR_NULL(fence)) {
		DPU_FB_ERR("%s: cannot create fence\n", fence_name);
		return NULL;
	}

	return fence;
}

static struct hisi_dss_fence *dpufb_buf_create_fence(struct dpu_fb_data_type *dpufb,
	struct dpufb_buf_sync *buf_sync_ctrl, u32 fence_type,
	int *fence_fd, int value)
{
	struct hisi_dss_fence *sync_fence = NULL;
	char fence_name[32];  /* fence name length */

	if (fence_type == HISI_DSS_RETIRE_FENCE) {
		snprintf(fence_name, sizeof(fence_name), "fb%u_retire", dpufb->index);
		sync_fence = hisi_dss_fb_sync_get_fence(
					buf_sync_ctrl->timeline_retire,
					fence_name, value);
	} else {
		snprintf(fence_name, sizeof(fence_name), "fb%u_release", dpufb->index);
		sync_fence = hisi_dss_fb_sync_get_fence(
					buf_sync_ctrl->timeline,
					fence_name, value);
	}

	if (IS_ERR_OR_NULL(sync_fence)) {
		DPU_FB_ERR("%s: unable to retrieve release fence\n", fence_name);
		goto end;
	}

	/* get fence fd */
	*fence_fd = hisi_dss_get_sync_fence_fd(sync_fence);
	if (*fence_fd < 0) {
		DPU_FB_ERR("%s: get_unused_fd_flags failed error:0x%x\n", fence_name, *fence_fd);
		hisi_dss_put_sync_fence(sync_fence);
		sync_fence = NULL;
		goto end;
	}

end:
	return sync_fence;
}

int dpufb_offline_create_fence(struct dpu_fb_data_type *dpufd)
{
	int value;
	int ret = 0;
	struct hisi_dss_fence *release_fence = NULL;
	struct dpufb_buf_sync *buf_sync_ctrl = NULL;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is nullptr!\n");
	if (dpufd->index != MEDIACOMMON_PANEL_IDX) {
		DPU_FB_INFO("fb%u only for mdc offline compose create fence!\n", dpufd->index);
		return ret;
	}

	buf_sync_ctrl = &dpufd->buf_sync_ctrl;
	value = buf_sync_ctrl->timeline_max + buf_sync_ctrl->threshold;
	release_fence = dpufb_buf_create_fence(dpufd, buf_sync_ctrl,
		HISI_DSS_RELEASE_FENCE, &(dpufd->ov_req.release_fence), value);
	if (IS_ERR_OR_NULL(release_fence)) {
		DPU_FB_ERR("unable to retrieve release fence\n");
		ret = PTR_ERR(release_fence);
		dpufd->ov_req.release_fence = -1;
	}
	dpufd->ov_req.retire_fence = -1;

	return ret;
}

int dpufb_buf_sync_create_fence(struct dpu_fb_data_type *dpufd,
	int32_t *release_fence_fd, int32_t *retire_fence_fd)
{
	int ret = 0;
	int value;
	struct hisi_dss_fence *release_fence = NULL;
	struct hisi_dss_fence *retire_fence = NULL;
	struct dpufb_buf_sync *buf_sync_ctrl = NULL;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is nullptr!\n");

	buf_sync_ctrl = &dpufd->buf_sync_ctrl;
	value = buf_sync_ctrl->timeline_max + buf_sync_ctrl->threshold + 1;
	release_fence = dpufb_buf_create_fence(dpufd, buf_sync_ctrl,
		HISI_DSS_RELEASE_FENCE, release_fence_fd, value);
	if (IS_ERR_OR_NULL(release_fence)) {
		DPU_FB_ERR("unable to retrieve release fence\n");
		ret = PTR_ERR(release_fence);
		goto release_fence_err;
	}

	value += buf_sync_ctrl->retire_threshold;
	retire_fence = dpufb_buf_create_fence(dpufd, buf_sync_ctrl,
		HISI_DSS_RETIRE_FENCE, retire_fence_fd, value);
	if (IS_ERR_OR_NULL(retire_fence)) {
		DPU_FB_ERR("unable to retrieve retire fence\n");
		ret = PTR_ERR(retire_fence);
		goto retire_fence_err;
	}

	if (g_debug_fence_timeline)
		DPU_FB_INFO("dpufb%u frame_no[%u] create fence timeline_max[%d], %s[%d],"
			"%s[%d], timeline[%u], timeline_retire[%u], release_fence_fd[%d], retire_fence_fd[%d]!\n",
			dpufd->index, dpufd->ov_req.frame_no,
			buf_sync_ctrl->timeline_max,
			release_fence->name, release_fence->base.seqno,
			retire_fence->name, retire_fence->base.seqno,
			buf_sync_ctrl->timeline->value,
			buf_sync_ctrl->timeline_retire->value, *release_fence_fd, *retire_fence_fd);

	return ret;

retire_fence_err:
	put_unused_fd(*release_fence_fd);
	hisi_dss_put_sync_fence(release_fence);
release_fence_err:
	*retire_fence_fd = -1;
	*release_fence_fd = -1;

	return ret;
}

static int dpufb_buf_sync_wait_fence(struct hisi_dss_fence *fence)
{
	int ret;

	ret = hisi_dss_wait_sync_fence(fence, BUF_SYNC_TIMEOUT_MSEC);
	if (ret < 0)
		DPU_FB_ERR("Waiting on fence=%pK failed, ret=%d\n", fence, ret);

	hisi_dss_put_sync_fence(fence);

	return ret;
}

void dpufb_buf_sync_wait_async(struct dpufb_buf_sync *pbuf_sync)
{
	struct dpufb_layerbuf *node = NULL;
	struct dpufb_layerbuf *_node_ = NULL;

	dpu_check_and_no_retval(!pbuf_sync, ERR, "pbuf_sync is nullptr!\n");

	down(&pbuf_sync->layerbuf_sem);
	list_for_each_entry_safe(node, _node_, &pbuf_sync->layerbuf_list, list_node) {
		if (g_debug_layerbuf_sync)
			DPU_FB_INFO("fence=%pK shared_fd=%d, buffer_handle=%pK, mmbuf(0x%x, %d)\n",
				node->fence, node->shared_fd, node->buffer_handle,
				node->mmbuf.addr, node->mmbuf.size);

		if (node->fence) {
			dpufb_buf_sync_wait_fence(node->fence);
			node->fence = NULL;
		}
	}
	up(&pbuf_sync->layerbuf_sem);
}

int dpufb_buf_sync_wait(int fence_fd)
{
	struct hisi_dss_fence *fence = NULL;

	fence = hisi_dss_get_fd_sync_fence(fence_fd);
	if (!fence) {
		DPU_FB_ERR("fence_fd=%d, sync_fence_fdget failed!\n", fence_fd);
		return -EINVAL;
	}

	return dpufb_buf_sync_wait_fence(fence);
}

void dpufb_buf_sync_suspend(struct dpu_fb_data_type *dpufd)
{
	struct dpufb_buf_sync *buf_sync_ctrl = NULL;
	unsigned long flags = 0;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is nullptr!\n");
	buf_sync_ctrl = &dpufd->buf_sync_ctrl;
	dpu_check_and_no_retval(!buf_sync_ctrl->timeline, ERR, "dpufd is nullptr!\n");

	spin_lock_irqsave(&buf_sync_ctrl->refresh_lock, flags);

	hisi_dss_resync_timeline(buf_sync_ctrl->timeline);
	hisi_dss_resync_timeline(buf_sync_ctrl->timeline_retire);

	buf_sync_ctrl->timeline_max = buf_sync_ctrl->timeline->next_value + 1;
	buf_sync_ctrl->refresh = 0;

	spin_unlock_irqrestore(&buf_sync_ctrl->refresh_lock, flags);
	if (g_debug_fence_timeline)
		DPU_FB_INFO("fb%u frame_no[%u] timeline_max[%d], TL[Nxt %u , Crnt %u]!\n",
			dpufd->index, dpufd->ov_req.frame_no, buf_sync_ctrl->timeline_max,
			buf_sync_ctrl->timeline->next_value, buf_sync_ctrl->timeline->value);
}

void dpufb_buf_sync_close_fence(int32_t *release_fence, int32_t *retire_fence)
{
	if (*release_fence >= 0) {
		sys_close(*release_fence);
		*release_fence = -1;
	}

	if (*retire_fence >= 0) {
		sys_close(*retire_fence);
		*retire_fence = -1;
	}
}

int dpufb_buf_sync_handle(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req)
{
	dss_overlay_block_t *pov_h_block_infos = NULL;
	dss_overlay_block_t *pov_h_block = NULL;
	dss_layer_t *layer = NULL;
	dss_wb_layer_t *wb_layer = NULL;
	uint32_t i;
	uint32_t m;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is nullptr!\n");
	dpu_check_and_return(!pov_req, -EINVAL, ERR, "pov_req is nullptr!\n");

	if (pov_req->wb_layer_nums > MAX_DSS_DST_NUM) {
		DPU_FB_ERR("pov_req->wb_layer_nums %u Exceeded array limit\n", pov_req->wb_layer_nums);
		return -EINVAL;
	}

	if (pov_req->wb_enable) {
		for (i = 0; i < pov_req->wb_layer_nums; i++) {
			wb_layer = &(pov_req->wb_layer_infos[i]);

			if (wb_layer->acquire_fence >= 0)
				dpufb_buf_sync_wait(wb_layer->acquire_fence);
		}
	}

	pov_h_block_infos = (dss_overlay_block_t *)(uintptr_t)(pov_req->ov_block_infos_ptr);
	for (m = 0; m < pov_req->ov_block_nums; m++) {
		pov_h_block = &(pov_h_block_infos[m]);

		for (i = 0; i < pov_h_block->layer_nums; i++) {
			layer = &(pov_h_block->layer_infos[i]);

			if (layer->dst_rect.y < pov_h_block->ov_block_rect.y)
				continue;

			if (layer->acquire_fence >= 0)
				dpufb_buf_sync_wait(layer->acquire_fence);
		}
	}

	return 0;
}

void dpufb_buf_sync_signal(struct dpu_fb_data_type *dpufd)
{
	struct dpufb_layerbuf *node = NULL;
	struct dpufb_layerbuf *_node_ = NULL;
	struct dpufb_buf_sync *buf_sync_ctrl = NULL;
	int val = 0;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is nullptr!\n");
	buf_sync_ctrl = &dpufd->buf_sync_ctrl;
	if (dpufd->index == AUXILIARY_PANEL_IDX) {
		queue_work(buf_sync_ctrl->free_layerbuf_queue, &(buf_sync_ctrl->free_layerbuf_work));
		return;
	}

	spin_lock(&buf_sync_ctrl->refresh_lock);
	if (buf_sync_ctrl->refresh) {
		val = buf_sync_ctrl->refresh;
		hisi_dss_inc_timeline(buf_sync_ctrl->timeline, val);
		hisi_dss_inc_timeline(buf_sync_ctrl->timeline_retire, val);

		buf_sync_ctrl->timeline_max += val;
		buf_sync_ctrl->refresh = 0;

		if (g_debug_fence_timeline)
			DPU_FB_INFO("dpufb%u frame_no=%u timeline_max=%d, timeline=%d, timeline_retire=%d!\n",
				dpufd->index, dpufd->ov_req.frame_no,
				buf_sync_ctrl->timeline_max,
				buf_sync_ctrl->timeline->value,
				buf_sync_ctrl->timeline_retire->value);
	}
	spin_unlock(&buf_sync_ctrl->refresh_lock);

	spin_lock(&(buf_sync_ctrl->layerbuf_spinlock));
	list_for_each_entry_safe(node, _node_, &(buf_sync_ctrl->layerbuf_list), list_node) {
		if (buf_sync_ctrl->layerbuf_flushed)
			node->timeline++;
	}
	buf_sync_ctrl->layerbuf_flushed = false;
	spin_unlock(&(buf_sync_ctrl->layerbuf_spinlock));

	queue_work(buf_sync_ctrl->free_layerbuf_queue, &(buf_sync_ctrl->free_layerbuf_work));
}

void dpufb_buf_sync_register(struct platform_device *pdev)
{
	char tmp_name[256] = {0};  /* release fence time line name */
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpufb_buf_sync *buf_sync_ctrl = NULL;

	dpu_check_and_no_retval(!pdev, ERR, "pdev is nullptr!\n");
	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is nullptr!\n");

	DPU_FB_DEBUG("fb%u, +\n", dpufd->index);
	buf_sync_ctrl = &dpufd->buf_sync_ctrl;
	buf_sync_ctrl->timeline = NULL;
	buf_sync_ctrl->timeline_retire = NULL;
	buf_sync_ctrl->fence_name = "dss-fence";

	buf_sync_ctrl->threshold = 0;
	buf_sync_ctrl->retire_threshold = 0;

	buf_sync_ctrl->timeline_max = 1;
	spin_lock_init(&buf_sync_ctrl->refresh_lock);
	spin_lock(&buf_sync_ctrl->refresh_lock);
	buf_sync_ctrl->refresh = 0;
	spin_unlock(&buf_sync_ctrl->refresh_lock);

	if (dpufd->index != AUXILIARY_PANEL_IDX) {
		snprintf(tmp_name, sizeof(tmp_name), "hisi_dss_fb%u", dpufd->index);
		buf_sync_ctrl->timeline = hisi_dss_create_timeline(tmp_name);
		dpu_check_and_no_retval(!buf_sync_ctrl->timeline, ERR, "cannot create release fence time line\n");

		snprintf(tmp_name, sizeof(tmp_name), "hisi_dss_fb%u_retire", dpufd->index);
		buf_sync_ctrl->timeline_retire = hisi_dss_create_timeline(tmp_name);
		dpu_check_and_no_retval(!buf_sync_ctrl->timeline_retire, ERR,
			"cannot create retire fence time line\n");
	}

	spin_lock_init(&(buf_sync_ctrl->layerbuf_spinlock));
	INIT_LIST_HEAD(&(buf_sync_ctrl->layerbuf_list));
	spin_lock(&(buf_sync_ctrl->layerbuf_spinlock));
	buf_sync_ctrl->layerbuf_flushed = false;
	spin_unlock(&(buf_sync_ctrl->layerbuf_spinlock));
	sema_init(&(buf_sync_ctrl->layerbuf_sem), 1);

	snprintf(tmp_name, sizeof(tmp_name), HISI_DSS_LAYERBUF_FREE, dpufd->index);
	INIT_WORK(&(buf_sync_ctrl->free_layerbuf_work), dpufb_layerbuf_unlock_work);
	buf_sync_ctrl->free_layerbuf_queue = create_singlethread_workqueue(tmp_name);
	if (!buf_sync_ctrl->free_layerbuf_queue) {
		dev_err(&pdev->dev, "failed to create free_layerbuf_queue!\n");
		return;
	}

	DPU_FB_DEBUG("fb%u, -\n", dpufd->index);
}

void dpufb_buf_sync_unregister(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpufb_buf_sync *buf_sync_ctrl = NULL;

	dpu_check_and_no_retval(!pdev, ERR, "pdev is nullptr!\n");
	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is nullptr!\n");

	DPU_FB_DEBUG("fb%u, +\n", dpufd->index);
	buf_sync_ctrl = &dpufd->buf_sync_ctrl;

	if (buf_sync_ctrl->timeline) {
		hisi_dss_destroy_timeline(buf_sync_ctrl->timeline);
		buf_sync_ctrl->timeline = NULL;
	}

	if (buf_sync_ctrl->timeline_retire) {
		hisi_dss_destroy_timeline(buf_sync_ctrl->timeline_retire);
		buf_sync_ctrl->timeline_retire = NULL;
	}

	if (buf_sync_ctrl->free_layerbuf_queue) {
		destroy_workqueue(buf_sync_ctrl->free_layerbuf_queue);
		buf_sync_ctrl->free_layerbuf_queue = NULL;
	}

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);
}

