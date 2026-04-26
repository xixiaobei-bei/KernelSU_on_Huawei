/*
 * npu_dfx_black_box.c
 *
 * about npu dfx black box
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
#include "npu_dfx_black_box.h"

#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/idr.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/stat.h>
#include <linux/io.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>
#include <linux/notifier.h>
#include <linux/hisi/hisi_svm.h>
#include <linux/hisi/hisi_noc_modid_para.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <securec.h>

#include "hisi_svm.h"
#include "npu_shm.h"
#include "npu_manager_common.h"
#include "npu_platform.h"
#include "mntn_public_interface.h"
#include "mntn_subtype_exception.h"
#include "soc_mid.h"
#include "npu_platform_register.h"
#include "npu_pm_framework.h"
#include "npu_adapter.h"
#include "npu_dfx_black_box_adapter.h"

static vir_addr_t g_bbox_addr_vir;
static vir_addr_t g_bbox_tslog_addr_vir;
static vir_addr_t g_bbox_aicpulog_addr_vir;

struct npu_mntn_private_s *g_npu_mntn_private;

struct rdr_exception_info g_rdr_exception_info[RDR_EXCEPTION_TYPES_SUM];

/*
 * Description: npu_mntn_copy_aicpulog_to_bbox
 * input: const char *src_addr, unsigned int* offset, unsigned int len
 * output: NA
 * return: void
 */
static int npu_mntn_copy_aicpulog_to_bbox(const char *src_addr,
	unsigned int len)
{
	unsigned int temp_offset;
	int ret;

	if ((src_addr == NULL) || (len == 0) ||
		(g_npu_mntn_private->mntn_info.rdr_addr == NULL)) {
		npu_drv_err("input parameter is error!\n");
		return -EINVAL;
	}

	temp_offset = g_npu_mntn_private->mntn_info.aicpu_add_offset + len;

	/* aicpu alloc size 64K */
	if (temp_offset >= NPU_AICPU_BBOX_MEM_MAX || temp_offset <= 0) {
		npu_drv_warn("bbox aicpu buf is full ,copy log to bbox is error! temp_offset=0x%x\n",
			temp_offset);
		g_npu_mntn_private->mntn_info.aicpu_add_offset = 0;
		return -ENOMEM;
	}

	ret = memcpy_s(((char *)g_npu_mntn_private->mntn_info.rdr_addr +
		NPU_BBOX_MEM_MAX +
		g_npu_mntn_private->mntn_info.aicpu_add_offset), len, src_addr, len);
	cond_return_error(ret != 0, -EINVAL, "memcpy_s failed!, ret=%d\n", ret);

	g_npu_mntn_private->mntn_info.aicpu_add_offset = temp_offset;

	return 0;
}

/*
 * Description: npu_mntn_copy_reg_to_bbox
 * input: const char *src_addr, unsigned int* offset, unsigned int len
 * output: NA
 * return: void
 */
int npu_mntn_copy_reg_to_bbox(const char *src_addr,
	unsigned int len)
{
	unsigned int left_len;
	unsigned int rdr_left_len;
	int ret;

	if ((src_addr == NULL) || (len == 0) ||
		(g_npu_mntn_private->mntn_info.rdr_addr == NULL)) {
		npu_drv_err("input parameter is error!\n");
		return -EINVAL;
	}

	if (g_npu_mntn_private->mntn_info.bbox_addr_offset >
		g_npu_mntn_private->mntn_info.npu_ret_info.log_len) {
		npu_drv_err("bbox_addr_offset is error!\n");
		return -EINVAL;
	}

	if (g_npu_mntn_private->mntn_info.bbox_addr_offset >
		NPU_BBOX_MEM_MAX - sizeof(exce_module_info_t)) {
		npu_drv_err("bbox_addr_offset is error!\n");
		return -EINVAL;
	}

	left_len = NPU_BBOX_MEM_MAX - sizeof(exce_module_info_t) -
		g_npu_mntn_private->mntn_info.bbox_addr_offset;
	rdr_left_len = g_npu_mntn_private->mntn_info.npu_ret_info.log_len -
		g_npu_mntn_private->mntn_info.bbox_addr_offset;

	if (len > left_len || len > rdr_left_len) {
		npu_drv_warn("bbox buf is full, turn to the start to reuse, "
			"len=%u, left_len=%u, rdr_left_len=%u\n",
			len, left_len, rdr_left_len);
		g_npu_mntn_private->mntn_info.bbox_addr_offset = 0;
		rdr_left_len = g_npu_mntn_private->mntn_info.npu_ret_info.log_len;
	}

	if (len > rdr_left_len) {
		npu_drv_err("message is too long to write into bbox buffer, "
			"len=%u, rdr_left_len=%u\n", len, rdr_left_len);
		len = rdr_left_len;
	}

	ret = memcpy_s(((char *)g_npu_mntn_private->mntn_info.rdr_addr +
		g_npu_mntn_private->mntn_info.bbox_addr_offset), len, src_addr, len);
	cond_return_error(ret != 0, -EINVAL, "memcpy_s failed, ret=%d\n", ret);

	g_npu_mntn_private->mntn_info.bbox_addr_offset += len;

	return 0;
}

/*
 * Description: npu_mntn_write_peri_reg_info
 * input:	char *file_path
 * output:	NA
 * return:	void
 */
static void npu_mntn_write_peri_reg_info(unsigned int core_id)
{
	char log_buf[NPU_BUF_LEN_MAX + 1] = {0};
	struct npu_peri_reg_s *peri_reg =
		&g_npu_mntn_private->reg_info[core_id].peri_reg;
	int ret;

	ret = snprintf_s(log_buf, NPU_BUF_LEN_MAX + 1, NPU_BUF_LEN_MAX,
		"NPU exception info : peri_stat=%x, ppll_select=%x, power_stat=%x,"
		"power_ack=%x, reset_stat=%x, perclken=%x, perstat=%x\r\n",
		peri_reg->peri_stat, peri_reg->ppll_select, peri_reg->power_stat,
		peri_reg->power_ack, peri_reg->reset_stat,
		peri_reg->perclken0, peri_reg->perstat0);
	cond_return_void(ret < 0, "snprintf_s is fail, ret = %d\n", ret);

	npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));
}

/*
 * Description: npu_mntn_write_mstr_reg_info
 * input:	char *file_path
 * output:	NA
 * return:	void
 */
static void npu_mntn_write_mstr_reg_info(unsigned int core_id)
{
	char log_buf[NPU_BUF_LEN_MAX + 1] = {0};
	struct npu_mstr_reg_s *mstr_reg =
		&g_npu_mntn_private->reg_info[core_id].mstr_reg;
	int ret;

	ret = snprintf_s(log_buf, NPU_BUF_LEN_MAX + 1, NPU_BUF_LEN_MAX,
		"NPU exception info : RD_BITMAP=%x, WR_BITMAP=%x, rd_cmd_total_cnt[0-3]={%x, %x, %x},"
		" wr_cmd_total_cnt=%x\n", mstr_reg->rd_bitmap, mstr_reg->wr_bitmap,
		mstr_reg->rd_cmd_total_cnt0, mstr_reg->rd_cmd_total_cnt0,
		mstr_reg->rd_cmd_total_cnt2, mstr_reg->wr_cmd_total_cnt);
	cond_return_void(ret < 0, "snprintf_s is fail, ret = %d\n", ret);

	npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));
}

void npu_mntn_log_add_exception_type(unsigned int modid,
	char log_buf[], unsigned int buf_size)
{
	switch (modid) {
	case (unsigned int)EXC_TYPE_TS_AICORE_EXCEPTION:
		(void)snprintf_s(log_buf, buf_size, buf_size - 1, "%s",
			"NPU exception info : aicore exception\r\n");
		break;
	case (unsigned int)EXC_TYPE_TS_AICORE_TIMEOUT:
		(void)snprintf_s(log_buf, buf_size, buf_size - 1, "%s",
			"NPU exception info : aicore timeout\r\n");
		break;
	case (unsigned int)EXC_TYPE_TS_SDMA_EXCEPTION:
		(void)snprintf_s(log_buf, buf_size, buf_size - 1, "%s",
			"NPU exception info : sdma exception\r\n");
		break;
	case (unsigned int)EXC_TYPE_TS_SDMA_TIMEOUT:
		(void)snprintf_s(log_buf, buf_size, buf_size - 1, "%s",
			"NPU exception info : sdma timeout\r\n");
		break;
	case (unsigned int)RDR_EXC_TYPE_TS_RUNNING_EXCEPTION:
		(void)snprintf_s(log_buf, buf_size, buf_size - 1, "%s",
			"NPU exception info : ts running exception\r\n");
		break;
	case (unsigned int)RDR_EXC_TYPE_TS_RUNNING_TIMEOUT:
		(void)snprintf_s(log_buf, buf_size, buf_size - 1, "%s",
			"NPU exception info : ts running timeout\r\n");
		break;
	case (unsigned int)RDR_EXC_TYPE_OS_EXCEPTION:
		(void)snprintf_s(log_buf, buf_size, buf_size - 1, "%s",
			"NPU exception info : OS running exception\r\n");
		break;
	case (unsigned int)RDR_EXC_TYPE_TS_INIT_EXCEPTION:
		(void)snprintf_s(log_buf, buf_size, buf_size - 1, "%s",
			"NPU exception info : ts init exception\r\n");
		break;
	case (unsigned int)RDR_EXC_TYPE_AICPU_INIT_EXCEPTION:
		(void)snprintf_s(log_buf, buf_size, buf_size - 1, "%s",
			"NPU exception info : aicpu init exception\r\n");
		break;
	case (unsigned int)RDR_EXC_TYPE_AICPU_HEART_BEAT_EXCEPTION:
		(void)snprintf_s(log_buf, buf_size, buf_size - 1, "%s",
			"NPU exception info : aicpu heart beat exception\r\n");
		break;
	case (unsigned int)RDR_EXC_TYPE_NPU_POWERUP_FAIL:
		(void)snprintf_s(log_buf, buf_size, buf_size - 1, "%s",
			"NPU exception info : npu powerup fail\r\n");
		break;
	case (unsigned int)RDR_EXC_TYPE_NPU_POWERDOWN_FAIL:
		(void)snprintf_s(log_buf, buf_size, buf_size - 1, "%s",
			"NPU exception info : npu powerdown fail\r\n");
		break;
	case (unsigned int)RDR_EXC_TYPE_NPU_SMMU_EXCEPTION:
		(void)snprintf_s(log_buf, buf_size, buf_size - 1, "%s",
			"NPU exception info : npu smmu exception\r\n");
		break;
	case (unsigned int)RDR_TYPE_HWTS_BUS_ERROR:
		(void)snprintf_s(log_buf, buf_size, buf_size - 1, "%s",
			"NPU exception info : hwts bus error\r\n");
		break;
	default:
		(void)snprintf_s(log_buf, buf_size, buf_size - 1, "%s",
			"NPU exception info : no exception\r\n");
		break;
	}
}

/*
 * Description: npu_get_peri_reg_info
 * input:	struct device *dev, char *buf, int len
 * output:	NA
 * return:	0: succes,  -1: error
 */
static int npu_get_peri_reg_info(struct device *dev, char *buf, int len)
{
	char *ptr = buf;
	unsigned int value, i, j;
	int remain = len, ret = 0;
	void __iomem  *vaddr = NULL;
	struct npu_dump_reg *cur_reg = NULL;

	buf[0] = 0;
	if (ARRAY_SIZE(peri_regs) == 0)
		return -1;

	for (i = 0; i < ARRAY_SIZE(peri_regs); ++i) {
		cur_reg = peri_regs + i;
		vaddr = devm_ioremap_nocache(dev, cur_reg->base, cur_reg->range);
		if (vaddr == NULL) {
			npu_drv_err("ioremap failed\n");
			continue;
		}

		for (j = 0; j < cur_reg->reg_num; ++j) {
			value = ioread32(vaddr + cur_reg->regs[j].offset);
			ret = snprintf_s(ptr, remain, remain - 1, "%s = 0x%x\n", cur_reg->regs[j].desc, value);
			if (ret < 0) {
				npu_drv_err("snprintf_s failed, remain=%d\n", remain);
				devm_iounmap(dev, vaddr);
				return -1;
			}
			ptr += ret;

			if (remain <= ret)
				break;

			remain -= ret;
		}

		devm_iounmap(dev, vaddr);

		if (remain <= ret)
			break;
	}

	return 0;
}

/*
 * Description: npu_mntn_dump_by_modid
 * input:	unsigned int modid
 * output:	NA
 * return:	void
 */
void npu_mntn_dump_by_modid(unsigned int modid)
{
	int ret;
	char log_buf[SMMU_EXCP_INFO_BUF_LEN];
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_void(plat_info == NULL, "npu_plat_get_info is NULL\n");
	cond_return_void(plat_info->pdev == NULL, "plat_info->pdev is NULL\n");

	if ((modid == RDR_EXC_TYPE_NOC_NPU7) || (modid == RDR_EXC_TYPE_NPU_SMMU_EXCEPTION)) {
		if (hisi_get_smmu_info(plat_info->pdev, log_buf, sizeof(log_buf)))
			npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));

		ret = npu_get_peri_reg_info(plat_info->pdev, log_buf, sizeof(log_buf));
		if (ret == 0)
			npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));
	}
}

/*
 * Description: npu_mntn_write_reg_log
 * input:	void
 * output:	NA
 * return:	void
 */
static void npu_mntn_write_reg_log(unsigned int core_id)
{
	char log_buf[NPU_BUF_LEN_MAX + 1] = {0};
	unsigned int modid;
	int ret = 0;

	ret = snprintf_s(log_buf, NPU_BUF_LEN_MAX + 1, NPU_BUF_LEN_MAX,
		"\r\nNPU exception info : core_id = 0x%x\r\n", core_id);
	cond_return_void(ret < 0, "snprintf_s is fail, ret = %d\n", ret);
	npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));

	ret = snprintf_s(log_buf, NPU_BUF_LEN_MAX + 1, NPU_BUF_LEN_MAX,
		"NPU exception info : exception_code =0x%x\r\n",
		g_npu_mntn_private->mntn_info.dump_info.modid);
	cond_return_void(ret < 0, "snprintf_s is fail, ret = %d\n", ret);
	npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));

	ret = snprintf_s(log_buf, NPU_BUF_LEN_MAX + 1, NPU_BUF_LEN_MAX,
		"NPU exception info : interrupt_status=0x%x\r\n",
		g_npu_mntn_private->exc_info[core_id].interrupt_status);
	cond_return_void(ret < 0, "snprintf_s is fail, ret = %d\n", ret);
	npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));

	ret = snprintf_s(log_buf, NPU_BUF_LEN_MAX + 1, NPU_BUF_LEN_MAX,
		"NPU exception info : ip=0x%x, ret=0x%x\r\n",
		g_npu_mntn_private->exc_info[core_id].target_ip,
		g_npu_mntn_private->exc_info[core_id].result);
	cond_return_void(ret < 0, "snprintf_s is fail, ret = %d\n", ret);
	npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));

	modid = g_npu_mntn_private->mntn_info.dump_info.modid;
	npu_mntn_log_add_exception_type(modid, log_buf, sizeof(log_buf));
	npu_mntn_copy_reg_to_bbox(log_buf, strlen(log_buf));

	npu_mntn_write_peri_reg_info(core_id);
	npu_mntn_write_mstr_reg_info(core_id);
	npu_mntn_dump_by_modid(modid);
}

static void npu_mntn_dump_aicpulog_work(void)
{
	uint32_t log_len;
	struct exc_module_info_s *exception_info = NULL;
	char log_buf[NPU_BUF_LEN_MAX + 1] = {0};
	vir_addr_t temp_addr_vir = g_bbox_aicpulog_addr_vir;
	int ret;

	exception_info = (struct exc_module_info_s *)(uintptr_t)(temp_addr_vir);
	log_len = exception_info->e_info_offset + exception_info->e_info_len;

	if (log_len > (uint32_t)AICPU_BUFF_LEN || log_len <= 0) {
		npu_drv_err("log_len is more than the max len, log_len = %x\n",
			log_len);
		return;
	}

	if (log_len > (uint32_t)NPU_BUF_LEN_MAX) {
		do {
			ret = memcpy_s((void *)log_buf, NPU_BUF_LEN_MAX, (void *)((uintptr_t)temp_addr_vir),
				NPU_BUF_LEN_MAX);
			cond_return_void(ret != 0, "memcpy_s failed!, ret is %d\n", ret);

			npu_mntn_copy_aicpulog_to_bbox(log_buf,
				(unsigned int)NPU_BUF_LEN_MAX);
			log_len -= NPU_BUF_LEN_MAX;
			temp_addr_vir += NPU_BUF_LEN_MAX;
		} while (log_len > NPU_BUF_LEN_MAX);
	}
	ret = memcpy_s((void *)log_buf, log_len, (void *)((uintptr_t)temp_addr_vir), log_len);
	cond_return_void(ret != 0, "memcpy_s log_buf failed!, ret is %d\n", ret);

	npu_mntn_copy_aicpulog_to_bbox(log_buf, log_len);
}

static void npu_rdr_regs_dump(void *dest, const void *src, size_t len)
{
	size_t remain;
	size_t mult;
	size_t i;
	u64 *u64_dst = NULL;
	u64 *u64_src = NULL;

	remain = len % sizeof(u64);
	mult = len / sizeof(u64);
	u64_dst = (u64 *)dest;
	u64_src = (u64 *)src;

	for (i = 0; i < mult; i++) {
		*u64_dst = *u64_src;
		npu_drv_debug("index:%llu u64_dst:0x%016lx= 0x%016lx  u64_src:0x%016lx = 0x%016lx",
			i, (u64)u64_dst, *u64_dst, (u64)(u64_src), *u64_src);
		u64_dst++;
		u64_src++;
	}

	for (i = 0; i < remain; i++)
		*((u8 *)u64_dst + i) = *((u8 *)u64_src + i);
}

static void npu_mntn_dump_npucrg(
	const struct npu_mem_desc *dump_region, vir_addr_t dest_addr)
{
	u8 *source_addr = NULL;
	uint32_t pericrg_stat;
	u64 pericrg_base;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_irq failed\n");
		return;
	}

	pericrg_base = (u64) (uintptr_t)plat_info->dts_info.reg_vaddr[NPU_REG_PERICRG_BASE];
	if (pericrg_base == 0) {
		npu_drv_err("pericrg_base is 0\n");
		return;
	}
	npu_drv_warn("dump region: base=0x%x len=0x%x",
		dump_region->base, dump_region->len);

	pericrg_stat = readl_relaxed((const volatile void *)(
		uintptr_t)soc_crgperiph_npucrg_stat_addr(pericrg_base));
	if (bitmap_get(pericrg_stat,
		SOC_CRGPERIPH_NPUCRG_IP_RST_BIT_OFFSET_START) == 0x1) {
		npu_drv_err("NPU CRG has not been unreset!\n");
		return;
	}

	source_addr = ioremap_nocache(dump_region->base, dump_region->len);
	if (source_addr == NULL) {
		npu_drv_err("ioremap_nocache failed.base=0x%x len=0x%x\n",
			dump_region->base, dump_region->len);
		return;
	}
	npu_rdr_regs_dump((void *)(uintptr_t)dest_addr, source_addr,
		dump_region->len);
	iounmap(source_addr);
}

static void npu_mntn_dump_region(void)
{
	u32 i;
	vir_addr_t target_addr = g_bbox_addr_vir + BBOX_ADDR_LEN;
	struct npu_mem_desc *dump_region = NULL;
	struct npu_platform_info *plat_info = npu_plat_get_info();
	struct npu_dev_ctx *dev_ctx = NULL;

	for (i = 0; i < NPU_DUMP_REGION_MAX; i++) {
		dump_region = &plat_info->dts_info.dump_region_desc[i];
		if (dump_region == NULL) {
			npu_drv_err("dump_region %d is NULL", i);
			break;
		}
		if (dump_region->base == 0 || dump_region->len == 0) {
			npu_drv_err("dump_region %d ' base=0x%x len=0x%x",
				i, dump_region->base, dump_region->len);
			break;
		}
		/* from the last area of npu_bbox */
		target_addr = target_addr - dump_region->len;

		npu_drv_warn("dump region %d: base=0x%x len=0x%x, bbox offset=0x%lx",
			i, dump_region->base, dump_region->len,
			target_addr - g_bbox_addr_vir);

		if ((target_addr < g_bbox_addr_vir) ||
			(target_addr > (g_bbox_addr_vir + BBOX_ADDR_LEN))) {
			npu_drv_err("dump_region %d ' out of range; len:0x%x is error",
				i, dump_region->len);
			return;
		}

		if (i == NPU_DUMP_REGION_NPU_CRG) {
			dev_ctx = get_dev_ctx_by_id(0);
			if (dev_ctx == NULL) {
				npu_drv_err("dev_ctx 0 is null\n");
				return;
			}

			mutex_lock(&dev_ctx->npu_power_up_off_mutex);

			if (dev_ctx->power_stage != NPU_PM_UP)
				npu_drv_warn("can not dump npu_crg because npu is powered off\n");
			else if (bitmap_get(dev_ctx->pm.work_mode, NPU_SEC))
				npu_drv_warn("can not dump npu_crg because npu is in secure mode\n");
			else
				npu_mntn_dump_npucrg(dump_region, target_addr);

			mutex_unlock(&dev_ctx->npu_power_up_off_mutex);
		}
	}
}

/*
 * Description: npu_mntn_dump_work
 * input: struct work_struct *work
 * output: NA
 * return: NA
 */
static void npu_mntn_dump_work(
	struct work_struct *work __attribute__((__unused__)))
{
	exce_module_info_t *module_info = NULL;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_info null\n");
		return;
	}

	npu_mntn_write_reg_log(g_npu_mntn_private->core_id);

	/* copy aicpu log to rdr address */
	if (plat_info->dts_info.feature_switch[NPU_FEATURE_AICPU] == 1)
		npu_mntn_dump_aicpulog_work();

	module_info = (exce_module_info_t *)(uintptr_t)(g_bbox_addr_vir);
	if (module_info->exp_message_len > module_info->buffer_len ||
		module_info->exp_message_len == 0) {
			npu_drv_warn("exp_message_len = 0x%x is more than the max len or is 0\n",
				module_info->exp_message_len);
		return;
	}

	npu_mntn_copy_reg_to_bbox((void *)((uintptr_t)g_bbox_tslog_addr_vir),
		module_info->exp_message_len);

	if (g_npu_mntn_private->mntn_info.dump_info.cb != NULL)
		g_npu_mntn_private->mntn_info.dump_info.cb(
			g_npu_mntn_private->mntn_info.dump_info.modid,
			g_npu_mntn_private->mntn_info.dump_info.coreid);

	npu_mntn_dump_region();
}

/*
 * Description: npu_mntn_reset_work
 * input: struct work_struct *work
 * output: NA
 * return: NA
 */
static void npu_mntn_reset_work(
	struct work_struct *work __attribute__((__unused__)))
{
	npu_plat_mntn_reset();
}

/*
 * Description : npu_rdr_resource_init
 * input : void
 * output : NA
 * return : int:  0 is OK. other value is Error No.
 */
int npu_rdr_resource_init(void)
{
	g_npu_mntn_private = kzalloc(
		sizeof(struct npu_mntn_private_s), GFP_KERNEL);
	if (g_npu_mntn_private == NULL) {
		npu_drv_err("failed to allocate memory!");
		return -ENOMEM;
	}

	g_npu_mntn_private->rdr_wq =
		create_singlethread_workqueue("npu_mntn_rdr_wq");
	if (g_npu_mntn_private->rdr_wq == NULL) {
		npu_drv_err("create_singlethread_workqueue is failed!");
		kfree(g_npu_mntn_private);
		g_npu_mntn_private = NULL;
		return -EINTR;
	}

	INIT_WORK(&g_npu_mntn_private->dump_work, npu_mntn_dump_work);
	INIT_WORK(&g_npu_mntn_private->reset_work, npu_mntn_reset_work);
	return 0;
}

/* when smmu is exception, platform  callback this func report */
static int npu_smmu_excp_callback(struct notifier_block *nb,
	unsigned long value, void *data)
{
	u8 smmu_event_id;
	(void)nb;
	(void)value;

	if (data == NULL)
		return -ENOMEM;

	// data pointer to u64 evt[EVTQ_ENT_DWORDS]
	smmu_event_id = ((u64 *)data)[0] & 0xFF;
	npu_drv_warn("evt_info[0] : 0x%x !", smmu_event_id);
	// fiter arm_smmu_ai event 0x10
	if (smmu_event_id != SMMU_ADDR_TRANSLATION_FAULT &&
		smmu_event_id != SMMU_BAD_CD_FAULT)
		if (npu_rdr_exception_is_count_exceeding(
			RDR_EXC_TYPE_NPU_SMMU_EXCEPTION) == 0)
			rdr_system_error((u32)RDR_EXC_TYPE_NPU_SMMU_EXCEPTION, 0, 0);
	return 0;
}

static struct notifier_block smmu_excp_notifier = {
	.notifier_call = npu_smmu_excp_callback,
};

/*
 * Description : npu_rdr_register_exception
 * input : void
 * output : NA
 * rdr_register_exception : ret == 0 is fail. ret > 0 is success.
 */
int npu_rdr_register_exception(void)
{
	int ret;
	unsigned int  size;
	unsigned long index;

	size = sizeof(npu_rdr_excetption_info) /
		sizeof(npu_rdr_excetption_info[0]);
	for (index = 0; index < size; index++) {
		/* error return 0, ok return modid */
		ret = rdr_register_exception(&npu_rdr_excetption_info[index]);
		if (ret == 0) {
			npu_drv_err("rdr_register_exception is failed! index = %ld, ret = %d",
				index, ret);
			return -EINTR;
		}
	}
	return 0;
}

/*
 * Description : npu_rdr_unregister_exception
 * input : void
 * output : NA
 * rdr_unregister_exception : ret < 0 is fail. ret >= 0 success.
 */
int npu_rdr_unregister_exception(void)
{
	int ret;
	unsigned int  size;
	unsigned long index;

	size = sizeof(npu_rdr_excetption_info) /
		sizeof(npu_rdr_excetption_info[0]);
	for (index = 0; index < size; index++) {
		/* ret < 0 is fail, ret >= 0 is success */
		ret = rdr_unregister_exception(
			npu_rdr_excetption_info[index].e_modid);
		if (ret < 0) {
			npu_drv_err("rdr_unregister_exception is failed! index = %ld, ret = %d",
				index, ret);
			return -EINTR;
		}
	}
	return 0;
}

/*
 * Description: npu_mntn_rdr_dump
 * input: modid: module id
 *        etype:exception type
 *        coreid: core id
 *        pathname: log path
 *        pfn_cb: callback function
 * output: NA
 * return: NA
 */
static void npu_mntn_rdr_dump(u32 modid, u32 etype, u64 coreid,
	char *pathname, pfn_cb_dump_done pfn_cb)
{
	npu_drv_warn("modid=0x%x, etype=0x%x, coreid=0x%llx",
		modid, etype, coreid);
	if (pathname == NULL) {
		npu_drv_err("pathname is empty!");
		return;
	}
	g_npu_mntn_private->mntn_info.dump_info.modid = modid;
	g_npu_mntn_private->mntn_info.dump_info.coreid = coreid;
	g_npu_mntn_private->mntn_info.dump_info.pathname = pathname;
	g_npu_mntn_private->mntn_info.dump_info.cb = pfn_cb;
	queue_work(g_npu_mntn_private->rdr_wq,
		&g_npu_mntn_private->dump_work);

	if (pfn_cb)
		pfn_cb(modid, coreid);
}

/*
 * Description: npu_mntn_rdr_reset
 * input: modid:module id
 *        etype:exception type
 *        coreid:core id
 * output: NA
 * return: NA
 */
static void npu_mntn_rdr_reset(u32 modid, u32 etype, u64 coreid)
{
	npu_drv_warn("modid=0x%x, etype=0x%x, coreid=0x%llx",
		modid, etype, coreid);
	if ((modid >= RDR_EXC_TYPE_NOC_NPU2) && (modid <= RDR_EXC_TYPE_NOC_NPU7))
		queue_work(g_npu_mntn_private->rdr_wq,
			&g_npu_mntn_private->reset_work);
}

/*
 * Description : npu_rdr_register_core
 * input : void
 * output : NA
 * rdr_register_module_ops : ret <0 is fail. ret >= 0 is success.
 */
int npu_rdr_register_core(void)
{
	int ret;
	struct rdr_module_ops_pub s_soc_ops;

	s_soc_ops.ops_dump = npu_mntn_rdr_dump;
	s_soc_ops.ops_reset = npu_mntn_rdr_reset;
	/* register npu core dump and reset function */
	ret = rdr_register_module_ops((u64)RDR_NPU, &s_soc_ops,
		&g_npu_mntn_private->mntn_info.npu_ret_info);
	if (ret < 0)
		npu_drv_err("rdr_register_module_ops is failed! ret = 0x%08x", ret);

	return ret;
}

/*
 * Description : npu_rdr_unregister_core
 * input : void
 * output : NA
 * rdr_unregister_module_ops : ret < 0 is fail. ret >= 0 is success.
 */
int npu_rdr_unregister_core(void)
{
	int ret;

	/* unregister npu core dump and reset function */
	ret = rdr_unregister_module_ops((u64)RDR_NPU);
	if (ret < 0)
		npu_drv_err("rdr_unregister_module_ops is failed! ret = 0x%08x", ret);

	return ret;
}

/*
 * Description : npu_rdr_addr_map
 * input : void
 * output : NA
 * return : int:  0 is OK. other value is Error No.
 */
int npu_rdr_addr_map(void)
{
	int ret;
	if (g_npu_mntn_private == NULL) {
		npu_drv_err("invalid null pointer");
		return -EINVAL;
	}
	g_npu_mntn_private->mntn_info.rdr_addr =
		hisi_bbox_map((
		phys_addr_t)g_npu_mntn_private->mntn_info.npu_ret_info.log_addr,
		g_npu_mntn_private->mntn_info.npu_ret_info.log_len);
	if (g_npu_mntn_private->mntn_info.rdr_addr == NULL) {
		npu_drv_err("hisi_bbox_map is failed!");
		return -EFAULT;
	}
	ret = memset_s((char *)g_npu_mntn_private->mntn_info.rdr_addr, 
		g_npu_mntn_private->mntn_info.npu_ret_info.log_len, 0,
		g_npu_mntn_private->mntn_info.npu_ret_info.log_len);
	cond_return_error(ret != 0, -EINVAL,  "memset_s failed. ret=%d\n", ret);

	return 0;
}

/*
 * Description : npu_rdr_addr_unmap
 * input : void
 * output : NA
 * return : int:  0 is OK. other value is Error No.
 */
int npu_rdr_addr_unmap(void)
{
	hisi_bbox_unmap(g_npu_mntn_private->mntn_info.rdr_addr);
	g_npu_mntn_private->mntn_info.rdr_addr = NULL;
	return 0;
}

/*
 * Description : npu_blackbox_addr_init
 * input : void
 * output : NA
 * return : int:  0 is OK. other value is Error No.
 */
int npu_blackbox_addr_init(void)
{
	struct npu_platform_info *plat_info = NULL;
	struct npu_dfx_desc *dfx_desc = NULL;
	struct npu_mem_desc *aicpu_mem_desc = NULL;
	int ret;

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("npu_plat_get_info error\n");
		return -1;
	}

	dfx_desc = &plat_info->resmem_info.dfx_desc[NPU_DFX_DEV_BLACKBOX];
	if (dfx_desc == NULL) {
		npu_drv_err("get dtsi failed\n");
		return -1;
	}
	aicpu_mem_desc = &plat_info->resmem_info.resmem_desc[AICPU_EL1];
	if (aicpu_mem_desc == NULL) {
		npu_drv_err("get dtsi failed\n");
		return -1;
	}

	g_bbox_addr_vir = (unsigned long long)(uintptr_t)ioremap_wc(
		dfx_desc->bufs->base, BBOX_ADDR_LEN);
	if (g_bbox_addr_vir == 0) {
		npu_drv_err("ioremap_wc failed\n");
		return -1;
	}
	g_bbox_tslog_addr_vir = g_bbox_addr_vir + sizeof(exce_module_info_t);
	ret = memset_s((void *)(uintptr_t)g_bbox_addr_vir, BBOX_ADDR_LEN, 0, BBOX_ADDR_LEN);
	cond_goto_error(ret != 0, bbox_release, ret, -1,  "memset_s failed. ret=%d\n", ret);

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_AICPU] == 1) {
		g_bbox_aicpulog_addr_vir = (unsigned long long)(uintptr_t)ioremap_wc(
			aicpu_mem_desc->base + AICPU_BUFF_OFFSET, AICPU_BUFF_LEN);
		cond_goto_error(g_bbox_aicpulog_addr_vir == 0, bbox_release, ret, -1, "ioremap_wc failed\n");
		ret = memset_s((void *)(uintptr_t)g_bbox_aicpulog_addr_vir, AICPU_BUFF_LEN,
			0, AICPU_BUFF_LEN);
		cond_goto_error(ret != 0, bbox_aicpulog_release, ret, -1,  "memset_s failed. ret=%d\n", ret);
	}

	return 0;
bbox_aicpulog_release:
	iounmap((void *)((uintptr_t)g_bbox_aicpulog_addr_vir));
	g_bbox_aicpulog_addr_vir = 0;
bbox_release:
	iounmap((void *)((uintptr_t)g_bbox_addr_vir));
	g_bbox_addr_vir = 0;
	return -1;
}

int npu_blackbox_addr_release(void)
{
	if (g_bbox_addr_vir != 0) {
		iounmap((void *)((uintptr_t)g_bbox_addr_vir));
		g_bbox_addr_vir = 0;
	}

	if (g_bbox_aicpulog_addr_vir != 0) {
		iounmap((void *)((uintptr_t)g_bbox_aicpulog_addr_vir));
		g_bbox_aicpulog_addr_vir = 0;
	}

	g_bbox_tslog_addr_vir = 0;

	return 0;
}

void npu_rdr_exception_init(void)
{
	const struct rdr_exception_info tmp[RDR_EXCEPTION_TYPES_SUM] = {
		/* OS exception code 0xc0000000-0xc00000ff */
		{EXC_TYPE_OS_DATA_ABORT, 1, 0},
		{EXC_TYPE_OS_INSTRUCTION_ABORT, 1, 0},
		{EXC_TYPE_OS_PC_ALIGN_FAULT, 1, 0},
		{EXC_TYPE_OS_SP_ALIGN_FAULT, 1, 0},
		{EXC_TYPE_OS_INFINITE_LOOP, 1, 0},
		{EXC_TYPE_OS_UNKNOWN_EXCEPTION, 1, 0},
		{RDR_EXC_TYPE_OS_EXCEPTION, 1, 0},
		/* AICPU exception code 0xc0000100-0xc00001ff,
		 * only for AICPU use, empty now
		 */
		/* AICORE exception code 0xc0000200-0xc00002ff */
		{EXC_TYPE_TS_AICORE_EXCEPTION, 1, 0},
		{EXC_TYPE_TS_AICORE_TIMEOUT, 1, 0},
		/* SDMA exception code 0xc0000300-0xc00003ff */
		{EXC_TYPE_TS_SDMA_EXCEPTION, 1, 0},
		{EXC_TYPE_TS_SDMA_TIMEOUT, 1, 0},
		/* TS exception code 0xc0000400-0xc00004ff */
		{RDR_EXC_TYPE_TS_RUNNING_EXCEPTION, 1, 0},
		{RDR_EXC_TYPE_TS_RUNNING_TIMEOUT, 1, 0},
		{RDR_EXC_TYPE_TS_INIT_EXCEPTION, 1, 0},
		/* AICPU exception code 0xc0000500-0xc00005ff */
		{RDR_EXC_TYPE_AICPU_INIT_EXCEPTION, 1, 0},
		{RDR_EXC_TYPE_AICPU_HEART_BEAT_EXCEPTION, 1, 0},
		/* driver exception code 0xc0000600-0xc00006ff, only for driver */
		{RDR_EXC_TYPE_NPU_POWERUP_FAIL, 1, 0},
		{RDR_EXC_TYPE_NPU_POWERDOWN_FAIL, 1, 0},
		{RDR_EXC_TYPE_NPU_SMMU_EXCEPTION, 1, 0},
		/* NPU NOC exception code 0xc0000700-0xc00007ff */
		{RDR_EXC_TYPE_NOC_NPU0, 1, 0},
		{RDR_EXC_TYPE_NOC_NPU1, 1, 0},
		/* HWTS exception code 0xc0000800-0xc00008ff */
		{RDR_TYPE_HWTS_BUS_ERROR, 1, 0},
		{RDR_TYPE_HWTS_EXCEPTION_ERROR, 1, 0}
	};
	int i = 0;

	for (; i < RDR_EXCEPTION_TYPES_SUM; i++) {
		g_rdr_exception_info[i].error_code = tmp[i].error_code;
		g_rdr_exception_info[i].limitation = tmp[i].limitation;
		g_rdr_exception_info[i].count = tmp[i].count;
	}
}

/*
 * Description : npu_rdr_exception_is_count_exceeding
 * input : u32
 * output : NA
 * return : u32: 0 means not exceeding limitation and allowed to report,
 *          otherwise not allowed.
 */
u32 npu_rdr_exception_is_count_exceeding(u32 error_code)
{
	u32 i;

	for (i = 0; i < RDR_EXCEPTION_TYPES_SUM; i++)
		if (g_rdr_exception_info[i].error_code == error_code) {
			if (g_rdr_exception_info[i].count >=
				g_rdr_exception_info[i].limitation) {
				npu_drv_warn("error code %u exceeds the limitation %u",
					error_code, g_rdr_exception_info[i].limitation);
				return 1;
			}
			g_rdr_exception_info[i].count++;
			npu_drv_debug("error code %u occurs %u time(s)",
				error_code, g_rdr_exception_info[i].count);
			return 0;
		}

	npu_drv_warn("error code %u is undefined", error_code);

	return 1;
}

/*
 * Description : npu_black_box_init
 * input : void
 * output : NA
 * return : int:  0 is OK. other value is Error No.
 */
int npu_black_box_init(void)
{
	int ret;

	npu_drv_info("Enter\n");

	ret = npu_rdr_resource_init();
	cond_return_error(ret < 0, ret, "npu_rdr_resource_init is faild ! ret = %d",
		ret);

	/* register NPU exception */
	ret = npu_rdr_register_exception();
	cond_goto_error(ret < 0, error, ret, ret,
		"npu_rdr_register_exception is faild! ret = %d", ret);

	/* register NPU dump and reset function */
	ret = npu_rdr_register_core();
	if (ret < 0) {
		npu_drv_err("npu_rdr_register_core is failed! ret = %d", ret);
		npu_rdr_unregister_exception();
		goto error;
	}

	ret = npu_rdr_addr_map();
	if (ret < 0) {
		npu_drv_err("npu_rdr_addr_map is failed! ret = %d", ret);
		npu_rdr_unregister_core();
		npu_rdr_unregister_exception();
		goto error;
	}

	ret = npu_blackbox_addr_init();
	if (ret < 0) {
		npu_rdr_addr_unmap();
		npu_rdr_unregister_core();
		npu_rdr_unregister_exception();
		npu_drv_err("npu_blackbox_addr_init is failed ! ret = %d", ret);
		goto error;
	}

	ret = npu_smmu_evt_register_notify(&smmu_excp_notifier);
	if (ret < 0) {
		npu_blackbox_addr_release();
		npu_rdr_addr_unmap();
		npu_rdr_unregister_core();
		npu_rdr_unregister_exception();
		npu_drv_err("hisi_smmu_evt_register_notify is failed ! ret = %d", ret);
		goto error;
	}

	return 0;

error:
	if (g_npu_mntn_private != NULL) {
		destroy_workqueue(g_npu_mntn_private->rdr_wq);
		kfree(g_npu_mntn_private);
		g_npu_mntn_private = NULL;
	}
	return ret;
}

/*
 * Description : npu_black_box_exit
 * input : void
 * output : NA
 * return : int:  0 is OK. other value is Error No.
 */
int npu_black_box_exit(void)
{
	int ret;

	(void)npu_smmu_evt_unregister_notify(&smmu_excp_notifier);
	npu_blackbox_addr_release();
	(void)npu_rdr_addr_unmap();

	ret = npu_rdr_unregister_core();
	if (ret < 0)
		npu_drv_err("npu_rdr_unregister_core is faild! ret = %d", ret);

	ret = npu_rdr_unregister_exception();
	if (ret < 0)
		npu_drv_err("npu_rdr_unregister_exception is faild! ret = %d", ret);

	if (g_npu_mntn_private != NULL) {
		destroy_workqueue(g_npu_mntn_private->rdr_wq);
		kfree(g_npu_mntn_private);
		g_npu_mntn_private = NULL;
	}

	return 0;
}

int npu_noc_register(void)
{
#ifdef CONFIG_NPU_NOC
	unsigned int i;

	if (array_size(npu_noc_para) != array_size(modid_array)) {
		npu_drv_err("array size not match, para size = %lu, modid size = %lu",
			array_size(npu_noc_para), array_size(modid_array));
		return -1;
	}

	for (i = 0; i < array_size(npu_noc_para); i++) {
		npu_drv_info("register noc err, type:%x", modid_array[i]);
		noc_modid_register(npu_noc_para[i], modid_array[i]);
	}
#endif
	return 0;
}
