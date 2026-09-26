/*
 * vfmw_intf.c
 *
 * This is vfmw interface.
 *
 * Copyright (c) 2017-2020 Huawei Technologies CO., Ltd.
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

#include "vfmw_intf.h"

#include <linux/module.h>
#include <linux/kern_levels.h>
#include <linux/sched.h>
#include <linux/kthread.h>

#include "basedef.h"

#ifdef HI_TVP_SUPPORT
#include "tvp_adapter.h"
#endif

#ifdef HIVDEC_SMMU_SUPPORT
#include "smmu.h"
#include "smmu_regs.h"
#endif
#include "vdm_hal_api.h"
#include "drv_omxvdec.h"

#ifndef IRQF_DISABLED
#define IRQF_DISABLED              0x00000020
#endif
#define VDM_TIMEOUT                800 // ms
#define VDM_FPGA_TIMEOUT           5000 // ms
#define SCD_TIMEOUT                800 // ms
#define SCD_FPGA_TIMEOUT           200000 // ms
#ifdef SECURE_VS_NOR_SECURE
#define SCEN_IDENT                 0x078
#else
#define SCEN_IDENT                 0x828
#endif
#define MAP_SIZE                  (256 * 1024)
#define MIN_VIDEO_MSG_POOL_SIZE   (10 * 1024 * 1024)

#define time_period(begin, end) \
	(((end) >= (begin)) ? ((end) - (begin)) : (0xffffffff - (begin) + (end)))

// cppcheck-suppress *
#define  vctrl_assert_ret(cond, else_print) \
	do { \
		if (!(cond)) { \
			dprint(PRN_FATAL, "%s %d %s\n", __func__, __LINE__, else_print); \
			return VCTRL_ERR; \
		} \
	} while (0)

static drv_mem_s g_regs_base_addr;

#ifdef HI_TVP_SUPPORT
#define WAIT_TASK_EXIT_TIMEOUT  20 // ms
#define WAIT_TASK_EXIT_INER     1  // ms

typedef struct {
	struct task_struct *task;
	wait_queue_head_t waitq;
	bool suspend;
	bool resume;
} vdec_tvp_info;

static vdec_tvp_info vdec_tvp;
#endif

vfmw_osal_func_ptr g_vfmw_osal_fun_ptr;

#ifdef MSG_POOL_ADDR_CHECK
SINT32 check_frm_buf_addr(UADDR src_frm_addr, mem_buffer_s *vdh_mem_map)
{
	UINT32 index;
	hi_s32 is_mapped = MEM_BUF_UNMAPPED;

	if (vdh_mem_map == NULL) {
		dprint(PRN_FATAL, "%s vdh_mem_map is NULL", __func__);
		return VDMHAL_ERR;
	}
	if (!src_frm_addr) {
		dprint(PRN_FATAL, "%s src_frm_addr is NULL", __func__);
		return VDMHAL_ERR;
	}

	for (index = VDH_SHAREFD_FRM_BUF; index < VDH_SHAREFD_MAX; index++) {
		if (vdh_mem_map[index].is_mapped == MEM_BUF_UNMAPPED)
			break;

		if ((src_frm_addr >= vdh_mem_map[index].start_phy_addr) &&
			(src_frm_addr <= (vdh_mem_map[index].start_phy_addr +
			vdh_mem_map[index].size))) {
			is_mapped = MEM_BUF_MAPPED;
			break;
		}
	}

	if (!is_mapped) {
		dprint(PRN_FATAL, "%s src_frm_addr is out of range (%d)\n", __func__, index);
		return VDMHAL_ERR;
	}
	return VDMHAL_OK;
}

SINT32 check_pmv_buf_addr(UADDR src_pmv_addr, mem_buffer_s *vdh_mem_map)
{
	if (vdh_mem_map == NULL) {
		dprint(PRN_FATAL, "%s vdh_mem_map is NULL", __func__);
		return VDMHAL_ERR;
	}
	if ((src_pmv_addr < vdh_mem_map[VDH_SHAREFD_PMV_BUF].start_phy_addr) ||
		(src_pmv_addr > (vdh_mem_map[VDH_SHAREFD_PMV_BUF].start_phy_addr +
		vdh_mem_map[VDH_SHAREFD_PMV_BUF].size))) {
		return VDMHAL_ERR;
	}
	return VDMHAL_OK;
}

/*lint -e578*/
SINT32 vctrl_vdh_unmap_message_pool(mem_buffer_s *mem_map)
{
	if (mem_map == NULL) {
		dprint(PRN_FATAL, "%s mem_map is NULL", __func__);
		return VCTRL_ERR;
	}
	if (mem_map->is_mapped == MEM_BUF_MAPPED)
		mem_map->is_map_virtual = MEM_BUF_MAPPED;
	return VCTRL_OK;
}
/*lint +e578*/

/*lint -e578*/
SINT32 vctrl_vdh_map_message_pool(
	mem_buffer_s *mem_map, hi_s32 share_fd, hi_bool is_vdh_all_buf_remap)
{
	hi_s32 ret;

	vdec_scene scene = mem_map->scene;

	if ((mem_map->is_mapped == MEM_BUF_MAPPED) && (is_vdh_all_buf_remap == HI_TRUE)) {
		mem_map->is_map_virtual = MEM_BUF_MAPPED;
		ret = memset_s(mem_map, sizeof(*mem_map), 0, sizeof(*mem_map));
		if (ret != EOK) {
			dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
			return VCTRL_ERR;
		}
		mem_map->scene = scene;
	}

	if (mem_map->is_mapped == MEM_BUF_UNMAPPED) {
		mem_map->is_map_virtual = MEM_BUF_MAPPED;
		ret = vdec_mem_get_map_info(share_fd, mem_map);
		vctrl_assert_ret((ret == HI_SUCCESS), "share fd map failed");
		mem_map->is_mapped = MEM_BUF_MAPPED;
		mem_map->share_fd = share_fd;
	}

	if (mem_map->scene == SCENE_VIDEO)
		ret = vdmhal_imp_open_hal(mem_map);
	else if (mem_map->scene == SCENE_HEIF)
		ret = vdmhal_imp_open_heif_hal(mem_map);

	if (ret) {
		vdec_mem_put_map_info(mem_map);
		ret = memset_s(mem_map, sizeof(*mem_map), 0, sizeof(*mem_map));
		if (ret != EOK)
			dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
		dprint(PRN_FATAL, "%s vdmhal_imp_open_hal is failed\n", __func__);
		return VCTRL_ERR;
	}

	return VCTRL_OK;
}
/*lint +e578*/
#endif

#ifdef HI_TVP_SUPPORT
inline void notify_and_wait_tvp_process(bool *notify_flag)
{
	SINT32 sleep_count = 0;
	*notify_flag = true;
	wake_up_interruptible(&vdec_tvp.waitq);
	do {
		VFMW_OSAL_M_SLEEP(WAIT_TASK_EXIT_INER);
		sleep_count++;
	} while (sleep_count < (WAIT_TASK_EXIT_TIMEOUT / WAIT_TASK_EXIT_INER));

	dprint(PRN_ALWS, "notify flag %d, sleep count %d\n", *notify_flag, sleep_count);
}
#endif

void vctrl_suspend(void)
{
	UINT8 is_scd_sleep = 0;
	UINT8 is_vdm_sleep = 0;
	UINT32 sleep_count = 0;
	UINT64 entr_time;
	UINT64 cur_time;

	entr_time = VFMW_OSAL_GET_TIME_IN_MS();

	scd_drv_prepare_sleep();

	vdmhal_prepare_sleep();

	do {
		if (scd_drv_get_sleep_stage() == SCDDRV_SLEEP_STAGE_SLEEP)
			is_scd_sleep = 1;

		if (vdmhal_get_sleep_stage() == VDMDRV_SLEEP_STAGE_SLEEP)
			is_vdm_sleep = 1;

		if ((is_scd_sleep == 1) && (is_vdm_sleep == 1))
			break;

		if (sleep_count > VFMW_OSAL_SLEEP_COUNT) {
			if (is_scd_sleep != 1) {
				dprint(PRN_FATAL, "Force scd sleep\n");
				scd_drv_force_sleep();
			}
			if (is_vdm_sleep != 1) {
				dprint(PRN_FATAL, "Force vdm sleep\n");
				vdmhal_force_sleep();
			}
			break;
		}

		VFMW_OSAL_M_SLEEP(VFMW_OSAL_SLEEP_TIME);
		sleep_count++;
	} while ((is_scd_sleep != 1) || (is_vdm_sleep != 1));

#ifdef HI_TVP_SUPPORT
	notify_and_wait_tvp_process(&vdec_tvp.suspend);
#endif

	cur_time = VFMW_OSAL_GET_TIME_IN_MS();
	dprint(PRN_ALWS, "Vfmw suspend totally take %llu ms\n", time_period(entr_time, cur_time));
}

void vctrl_resume(void)
{
	UINT64 entr_time, cur_time;

	entr_time = VFMW_OSAL_GET_TIME_IN_MS();

	smmu_init_global_reg();

	scd_drv_exit_sleep();

	vdmhal_exit_sleep();

#ifdef HI_TVP_SUPPORT
	notify_and_wait_tvp_process(&vdec_tvp.resume);
#endif

	cur_time = VFMW_OSAL_GET_TIME_IN_MS();
	dprint(PRN_ALWS, "Vfmw resume totally take %llu ms\n", time_period(entr_time, cur_time));
}

static irqreturn_t vctrl_isr(int irq, void *dev_id)
{
	UINT32 d_32;

	d_32 = rd_scd_reg(REG_SCD_INI_CLR) & 0x1;
	if (d_32 == 1)
		scd_drv_isr();

	read_vreg(VREG_INT_STATE, d_32, 0);
	if (d_32 == 1)
		vdmhal_isr(0);

	return IRQ_HANDLED;
}

static SINT32 vctrl_request_irq(
	UINT32 irq_num_norm, UINT32 irq_num_prot, UINT32 irq_num_safe)
{
#if !defined(VDM_BUSY_WAITTING)
	// for 2.6.24 after
	if (VFMW_OSAL_REQUEST_IRQ(irq_num_norm,
		(osal_irq_handler_t)vctrl_isr, IRQF_DISABLED,
		"vdec_norm_irq", NULL) != 0) {
		dprint(PRN_FATAL, "Request vdec norm irq %d failed\n", irq_num_norm);
		return VCTRL_ERR;
	}
#endif

#if !defined(SMMU_BUSY_WAITTING)
#ifdef ENV_SOS_KERNEL
	// for 2.6.24 after
	if (VFMW_OSAL_REQUEST_IRQ(irq_num_prot,
		(osal_irq_handler_t)vctrl_isr, IRQF_DISABLED,
		"vdec_prot_smmu_irq", NULL) != 0) {
		dprint(PRN_FATAL, "Request vdec prot irq %d failed\n", irq_num_prot);
		return VCTRL_ERR;
	}
#endif
#endif

	return VCTRL_OK;
}

static void vctrl_free_irq(
	UINT32 irq_num_norm, UINT32 irq_num_prot, UINT32 irq_num_safe)
{
#if !defined(VDM_BUSY_WAITTING)
	VFMW_OSAL_FREE_IRQ(irq_num_norm, NULL);
#endif

#if !defined(SMMU_BUSY_WAITTING)
#ifdef ENV_SOS_KERNEL
	VFMW_OSAL_FREE_IRQ(irq_num_prot, NULL);
#endif
#endif
}

static SINT32 vctrl_hal_init(void)
{
#ifdef HIVDEC_SMMU_SUPPORT
	if (smmu_init() != SMMU_OK) {
		dprint(PRN_FATAL, "smmu_init failed\n");
		return VCTRL_ERR;
	}
#endif

	scd_drv_init();
	vdmhal_imp_init();
	smmu_init_global_reg();

#ifdef HISMMUV300
	if (smmu_v3_init() != SMMU_OK) {
		dprint(PRN_FATAL, "smmu_init failed\n");
		return VCTRL_ERR;
	}
#endif
	return VCTRL_OK;
}

static void vctrl_hal_deinit(void)
{
#ifdef HIVDEC_SMMU_SUPPORT
	smmu_deinit();
#endif
	vdmhal_imp_deinit();
	scd_drv_deinit();
}

/*lint -e578*/
static SINT32 vctrl_scd_get_addr_info(
	mem_buffer_s *mem_map, scd_config_reg_s *ctrl_reg)
{
	hi_s32 ret;
	hi_u32 index;

	vctrl_assert_ret((mem_map != HI_NULL), "mem_map parameter error");
	vctrl_assert_ret((ctrl_reg != HI_NULL), "ctrl_reg parameter error");
	vctrl_assert_ret((ctrl_reg->scd_output_buf_num <= SCD_OUTPUT_BUF_CNT),
					 "scd output buffer num is out of range");

	for (index = 0;
			index < (SCD_SHAREFD_OUTPUT_BUF +
			ctrl_reg->scd_output_buf_num); index++) {
		if ((mem_map[index].is_mapped == MEM_BUF_UNMAPPED) ||
				(ctrl_reg->is_scd_all_buf_remap)) {
			mem_map[index].is_map_virtual = MEM_BUF_UNMAPPED;
			ret = vdec_mem_get_map_info(
				ctrl_reg->scd_share_fd[index], &mem_map[index]);
			vctrl_assert_ret((ret == HI_SUCCESS),
				"share fd map is failed");
			mem_map[index].is_mapped = MEM_BUF_MAPPED;
			mem_map[index].share_fd =
				ctrl_reg->scd_share_fd[index];
			vdec_mem_put_map_info(&mem_map[index]);
		}
	}
	return VCTRL_OK;
}
/*lint +e578*/

/*lint -e578*/
/* New Func added for judging current scenario is Video or HEIF */
static SINT32 vdec_mem_check_scene(mem_buffer_s *mem_map, hi_s32 share_fd)
{
	hi_s32 ret;
	vdec_scene scene;

	mem_map->is_map_virtual = MEM_BUF_UNMAPPED;
	ret = vdec_mem_get_map_info(share_fd, mem_map);
	vctrl_assert_ret((ret == VCTRL_OK), "msg sharefd map failed");

	if (mem_map->size < MIN_VIDEO_MSG_POOL_SIZE)
		scene = SCENE_HEIF;
	else
		scene = SCENE_VIDEO;

	vdec_mem_put_map_info(mem_map);
	ret = memset_s(mem_map, sizeof(*mem_map), 0, sizeof(*mem_map));
	if (ret != EOK) {
		dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
		return VCTRL_ERR;
	}
	mem_map->scene = scene;

	return VCTRL_OK;
}
/*lint +e578*/

/*lint -e578*/
static SINT32 vctrl_get_msg_pool_addr(
	mem_buffer_s *mem_map, hi_s32 share_fd, hi_bool is_vdh_all_buf_remap)
{
	hi_s32 ret;

	if (mem_map->is_mapped != MEM_BUF_MAPPED) {
		ret = vdec_mem_check_scene(mem_map, share_fd);
		if (ret) {
			dprint(PRN_FATAL, "%s %d %s\n", __func__, __LINE__, "msg sharefd map failed");
			return ret;
		}
	}

	ret = vctrl_vdh_map_message_pool(mem_map,
		share_fd, is_vdh_all_buf_remap);
	vctrl_assert_ret((ret == VCTRL_OK),
		"msg sharefd map failed");

	return VCTRL_OK;
}
/*lint +e578*/

/*lint -e578*/
static SINT32 vctrl_vdh_get_addr_info(
	mem_buffer_s *mem_map, omxvdh_reg_cfg_s *vdm_reg_cfg)
{
	hi_s32 ret;
	hi_u32 index;
	hi_s32 *share_fd = &(vdm_reg_cfg->vdh_share_fd[0]);
	hi_bool is_vdh_frm_buf_remap = vdm_reg_cfg->is_frm_buf_remap;
	hi_bool is_vdh_pmv_buf_remap = vdm_reg_cfg->is_pmv_buf_remap;
	hi_bool is_vdh_all_buf_remap = vdm_reg_cfg->is_all_buf_remap;

	vctrl_assert_ret((mem_map != HI_NULL), "mem_map parameter error");
	vctrl_assert_ret((vdm_reg_cfg->vdh_frm_buf_num <=
		MAX_FRAME_NUM), "vdh_frm_buf_num error");
	vctrl_assert_ret((vdm_reg_cfg->vdh_stream_buf_num <=
		VDH_STREAM_BUF_CNT), "vdh_stream_buf_num error");

	for (index = 0; index < (VDH_SHAREFD_FRM_BUF
			+ vdm_reg_cfg->vdh_frm_buf_num); index++) {
		/* do not get addr info for addtional stream buffer share fd */
		if ((index >= (VDH_SHAREFD_STREAM_BUF +
				vdm_reg_cfg->vdh_stream_buf_num)) &&
				(index < VDH_SHAREFD_PMV_BUF)) {
			continue;
		}
#ifdef MSG_POOL_ADDR_CHECK
		if (index == VDH_SHAREFD_MESSAGE_POOL) {
			ret = vctrl_get_msg_pool_addr(&mem_map[index],
				share_fd[index], is_vdh_all_buf_remap);
			vctrl_assert_ret((ret == VCTRL_OK),
				"msg sharefd map failed");
			continue;
		}
#endif
		if ((mem_map[index].is_mapped == MEM_BUF_MAPPED) && (is_vdh_all_buf_remap)) {
			ret = memset_s(&mem_map[index], sizeof(mem_map[index]),
				0, sizeof(mem_map[index]));
			if (ret != EOK) {
				dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
				return VCTRL_ERR;
			}
		}

		/* pmv or fmv remap */
		if ((mem_map[index].is_mapped == MEM_BUF_MAPPED) &&
				(((index == VDH_SHAREFD_PMV_BUF) &&
				(is_vdh_pmv_buf_remap)) ||
				((index >= VDH_SHAREFD_FRM_BUF) &&
				(is_vdh_frm_buf_remap)))) {
				ret = memset_s(&mem_map[index], sizeof(mem_map[index]),
					0, sizeof(mem_map[index]));
				if (ret != EOK) {
					dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
					return VCTRL_ERR;
				}
		}

		if (mem_map[index].is_mapped == MEM_BUF_UNMAPPED) {
			mem_map[index].is_map_virtual = MEM_BUF_UNMAPPED;
			ret = vdec_mem_get_map_info(share_fd[index],
				&mem_map[index]);
			if (ret != HI_SUCCESS) {
				dprint(PRN_FATAL, "share fd map failed, index is %d", index);
				return VCTRL_ERR;
			}
			mem_map[index].is_mapped = MEM_BUF_MAPPED;
			mem_map[index].share_fd = share_fd[index];
			vdec_mem_put_map_info(&mem_map[index]);
		}
	}
	return VCTRL_OK;
}
/*lint +e578*/

#ifdef HI_TVP_SUPPORT
static int vdec_tvp_task(void *data)
{
	int ret;

	dprint(PRN_ALWS, "enter vdec tvp task\n");

	if (tvp_vdec_secure_init() != VDEC_OK)
		dprint(PRN_FATAL, "%s, tvp vdec init failed\n", __func__);

	while (!kthread_should_stop()) {
		dprint(PRN_ALWS, "before wait event interruptible\n");
		/*lint -e666 -e578*/
		ret = wait_event_interruptible(
				  vdec_tvp.waitq,
				  (vdec_tvp.resume ||
				   vdec_tvp.suspend ||
				   (kthread_should_stop())));
		/*lint +e666 +e578*/
		if (ret) {
			dprint(PRN_FATAL, "wait event interruptible failed\n");
			continue;
		}
		dprint(PRN_ALWS, "after wait event interruptible\n");

		if (vdec_tvp.resume) {
			dprint(PRN_ALWS, "tvp vdec resume\n");
			if (tvp_vdec_resume() != VDEC_OK)
				dprint(PRN_FATAL, "tvp vdec resume failed\n");
			vdec_tvp.resume = false;
		}

		if (vdec_tvp.suspend) {
			dprint(PRN_ALWS, "tvp vdec supend\n");
			if (tvp_vdec_suspend() != VDEC_OK)
				dprint(PRN_FATAL, "tvp vdec suspend failed\n");
			vdec_tvp.suspend = false;
		}
	}

	tvp_vdec_secure_exit();

	dprint(PRN_ALWS, "exit vdec tvp task\n");

	return HI_SUCCESS;
}
#endif

SINT32 vctrl_open_drivers(void)
{
	mem_record_s *pst_mem;
	SINT32 ret;

	pst_mem = &g_regs_base_addr.st_vdh_reg;
	if (mem_map_register_addr(g_vdh_reg_base_addr,
			MAP_SIZE, pst_mem) == MEM_MAN_OK) {
		if (mem_add_mem_record(pst_mem->phy_addr,
				pst_mem->vir_addr, pst_mem->length) != MEM_MAN_OK) {
			dprint(PRN_ERROR, "%s %d mem_add_mem_record failed\n", __func__, __LINE__);
			goto exit;
		}
	} else {
		dprint(PRN_FATAL, "Map vdh register failed! g_vdh_reg_base_addr : %pK, g_vdh_reg_range : %d\n",
			(void *)(uintptr_t)g_vdh_reg_base_addr, g_vdh_reg_range);
		goto exit;
	}

	ret = vctrl_request_irq(g_vdec_irq_num_norm,
		g_vdec_irq_num_prot, g_vdec_irq_num_safe);
	if (ret != VCTRL_OK) {
		dprint(PRN_FATAL, "vctrl_request_irq failed\n");
		goto exit;
	}

	if (vctrl_hal_init() != VCTRL_OK) {
		dprint(PRN_FATAL, "vctrl_hal_init failed\n");
		goto exit;
	}

	VFMW_OSAL_INIT_EVENT(G_SCDHWDONEEVENT, 0);
	VFMW_OSAL_INIT_EVENT(G_VDMHWDONEEVENT, 0);

	return VCTRL_OK;

exit:
	vctrl_close_vfmw();
	return VCTRL_ERR;
}

SINT32 vctrl_open_vfmw(void)
{
	hi_s32 ret;

	ret = memset_s(&g_regs_base_addr, sizeof(g_regs_base_addr), 0,
		sizeof(g_regs_base_addr));
	if (ret != EOK) {
		dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
		return VCTRL_ERR;
	}
	mem_init_mem_manager();
	if (vctrl_open_drivers() != VCTRL_OK) {
		dprint(PRN_FATAL, "OpenDrivers fail\n");
		return VCTRL_ERR;
	}

#ifdef HI_TVP_SUPPORT
	if (vdec_tvp.task == NULL) {
		ret = memset_s(&vdec_tvp, sizeof(vdec_tvp), 0, sizeof(vdec_tvp));
		if (ret != EOK) {
			dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
			return VCTRL_ERR;
		}
		init_waitqueue_head(&vdec_tvp.waitq);

		vdec_tvp.task = kthread_run(vdec_tvp_task,
			NULL, "vdec tvp task");
		if (IS_ERR(vdec_tvp.task)) {
			/* needn't return fail for normal video play */
			dprint(PRN_FATAL, "creat vdec tvp task failed\n");
			ret = memset_s(&vdec_tvp, sizeof(vdec_tvp), 0, sizeof(vdec_tvp));
			if (ret != EOK) {
				dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
				return VCTRL_ERR;
			}
		}
	}
#endif

	return VCTRL_OK;
}

SINT32 vctrl_close_vfmw(void)
{
	mem_record_s *pst_mem = NULL;
	hi_s32 ret;

	vctrl_hal_deinit();

	pst_mem = &g_regs_base_addr.st_vdh_reg;
	if (pst_mem->length != 0) {
		mem_unmap_register_addr(pst_mem->phy_addr,
			pst_mem->vir_addr, pst_mem->length);
		mem_del_mem_record(pst_mem->phy_addr,
			pst_mem->vir_addr, pst_mem->length);
		ret = memset_s(&g_regs_base_addr.st_vdh_reg, sizeof(g_regs_base_addr.st_vdh_reg),
					   0, sizeof(g_regs_base_addr.st_vdh_reg));
		if (ret != EOK)
			dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
	}

	vctrl_free_irq(g_vdec_irq_num_norm, g_vdec_irq_num_prot, g_vdec_irq_num_safe);

#ifdef HI_TVP_SUPPORT
	if (vdec_tvp.task != NULL) {
		kthread_stop(vdec_tvp.task);
		ret = memset_s(&vdec_tvp, sizeof(vdec_tvp), 0, sizeof(vdec_tvp));
		if (ret != EOK)
			dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
	}
#endif
	return VCTRL_OK;
}

SINT32 vctrl_vdm_hal_process(omxvdh_reg_cfg_s *vdm_reg_cfg,
							 vdmhal_backup_s *vdm_reg_state,
							 mem_buffer_s *vdh_mem_map)
{
	hi_s32 ret;
	vdmdrv_sleep_stage_e sleep_state;

	if (vdm_reg_cfg == NULL || vdm_reg_state == NULL || vdh_mem_map == NULL) {
		dprint(PRN_FATAL, "input params is null\n");
		return HI_FAILURE;
	}
	sleep_state = vdmhal_get_sleep_stage();
	if (sleep_state == VDMDRV_SLEEP_STAGE_SLEEP) {
		dprint(PRN_ALWS, "vdm sleep state\n");
		return HI_FAILURE;
	}

	if (vdm_reg_cfg->vdh_reset_flag)
		vdmhal_imp_reset_vdm(0);

	ret = vctrl_vdh_get_addr_info(&vdh_mem_map[0], vdm_reg_cfg);
	if (ret) {
		dprint(PRN_FATAL, "vctrl_vdh_get_addr_info is failed\n");
		return HI_FAILURE;
	}

	VFMW_OSAL_INIT_EVENT(G_VDMHWDONEEVENT, 0);
	ret = vdmhal_hw_dec_proc(vdm_reg_cfg, vdh_mem_map);
	if (ret) {
		dprint(PRN_FATAL, "%s config error\n", __func__);
	} else {
		ret = VFMW_OSAL_WAIT_EVENT(G_VDMHWDONEEVENT,
			((g_is_fpga == 0) ? VDM_TIMEOUT : VDM_FPGA_TIMEOUT));
		if (ret == HI_SUCCESS) {
			vdmhal_getregstate(vdm_reg_state);
		} else {
			dprint(PRN_FATAL, "VFMW_OSAL_WAIT_EVENT wait time out\n");
			vdmhal_imp_reset_vdm(0);
		}
	}

	sleep_state = vdmhal_get_sleep_stage();
	if (sleep_state == VDMDRV_SLEEP_STAGE_PREPARE)
		vdmhal_set_sleep_stage(VDMDRV_SLEEP_STAGE_SLEEP);

	return ret;
}

SINT32 vctrl_scd_hal_process(
	omx_scd_reg_cfg_s *scd_reg_cfg, scd_state_reg_s *scd_state_reg,
	mem_buffer_s *scd_mem_map)
{
	hi_s32 ret;
	scd_drv_sleep_stage_e sleep_state;
	config_scd_cmd cmd;

	if (scd_reg_cfg == NULL || scd_state_reg == NULL || scd_mem_map == NULL) {
		dprint(PRN_FATAL, "input params is null\n");
		return HI_FAILURE;
	}

	cmd = scd_reg_cfg->cmd;

	sleep_state = scd_drv_get_sleep_stage();
	if (sleep_state == SCDDRV_SLEEP_STAGE_SLEEP) {
		dprint(PRN_ALWS, "SCD sleep state\n");
		return HI_FAILURE;
	}

	if (scd_reg_cfg->s_reset_flag) {
		if (scd_drv_reset_scd() != HI_SUCCESS) {
			dprint(PRN_FATAL, "VDEC_IOCTL_SCD_WAIT_HW_DONE  Reset SCD failed\n");
			return HI_FAILURE;
		}
	}

	ret = vctrl_scd_get_addr_info(&scd_mem_map[0], &(scd_reg_cfg->sm_ctrl_reg));
	if (ret != VCTRL_OK) {
		dprint(PRN_FATAL, "memory map failure\n");
		return HI_FAILURE;
	}

	switch (cmd) {
	case CONFIG_SCD_REG_CMD:
		VFMW_OSAL_INIT_EVENT(G_SCDHWDONEEVENT, 0);
		ret = scd_drv_write_reg(&scd_reg_cfg->sm_ctrl_reg, scd_mem_map);
		if (ret != HI_SUCCESS) {
			dprint(PRN_FATAL, "SCD busy\n");
			return HI_FAILURE;
		}

		ret = VFMW_OSAL_WAIT_EVENT(G_SCDHWDONEEVENT,
			((g_is_fpga == 0) ? SCD_TIMEOUT : SCD_FPGA_TIMEOUT));
		if (ret == HI_SUCCESS) {
			scd_drv_get_reg_state(scd_state_reg);
		} else {
			dprint(PRN_ALWS, "VDEC_IOCTL_SCD_WAIT_HW_DONE  wait time out\n");
			scd_drv_reset_scd();
		}
		wr_scd_reg(REG_SCD_INT_MASK, 1);
		sleep_state = scd_drv_get_sleep_stage();
		if (sleep_state == SCDDRV_SLEEP_STAGE_PREPARE)
			scd_drv_set_sleep_stage(SCDDRV_SLEEP_STAGE_SLEEP);
		break;

	default:
		dprint(PRN_FATAL, " cmd type unknown:%d\n", cmd);
		return HI_FAILURE;
	}

	return ret;
}

SINT32 vctrl_vdm_hal_is_run(void)
{
	return vdmhal_is_vdm_run(0);
}

hi_bool vctrl_scen_ident(hi_u32 cmd)
{
	hi_u32 value;
#ifdef SECURE_VS_NOR_SECURE
	read_vreg(SCEN_IDENT, value, 0);
#else
	value = rd_scd_reg(SCEN_IDENT);
#endif
	if (value == current->tgid) {
		return HI_TRUE;
	}

	return HI_FALSE;
}

hi_s32 vfmw_drv_mod_init(void)
{
	osal_init_interface();
	VFMW_OSAL_SEMA_INIT(G_SCD_SEM);
	VFMW_OSAL_SEMA_INIT(G_VDH_SEM);
	VFMW_OSAL_SEMA_INIT(G_BPD_SEM);

	VFMW_OSAL_SPIN_LOCK_INIT(G_SPINLOCK_SCD);
	VFMW_OSAL_SPIN_LOCK_INIT(G_SPINLOCK_VDH);
	VFMW_OSAL_SPIN_LOCK_INIT(G_SPINLOCK_RECORD);
	VFMW_OSAL_INIT_EVENT(G_SCDHWDONEEVENT, 0);
	VFMW_OSAL_INIT_EVENT(G_VDMHWDONEEVENT, 0);

#ifdef MODULE
	dprint(PRN_ALWS, "%s : Load hi_vfmw.ko (%d) success\n", __func__, VFMW_VERSION_NUM);
#endif

#ifdef HI_TVP_SUPPORT
	(void)memset_s(&vdec_tvp, sizeof(vdec_tvp), 0, sizeof(vdec_tvp));
#endif

	return 0;
}

void vfmw_drv_mod_exit(void)
{
#ifdef MODULE
	dprint(PRN_ALWS, "%s : Unload hi_vfmw.ko (%d) success\n", __func__, VFMW_VERSION_NUM);
#endif
}

module_init(vfmw_drv_mod_init);
module_exit(vfmw_drv_mod_exit);

MODULE_AUTHOR("gaoyajun");
MODULE_LICENSE("GPL");
