
#include "public.h"
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
	(((end) >= (begin)) ? ((end)-(begin)) : (0xffffffff - (begin) + (end)))

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
SINT32 check_frm_buf_addr(UADDR src_frm_addr, mem_buffer_s *p_vdh_mem_map)
{
	UINT32 index;
	hi_s32 is_mapped = 0;

	if (!src_frm_addr) {
		dprint(PRN_FATAL, "%s src_frm_addr is NULL", __func__);
		return VDMHAL_ERR;
	}

	for (index = VDH_SHAREFD_FRM_BUF; index < VDH_SHAREFD_MAX; index++) {
		if (p_vdh_mem_map[index].is_mapped == 0)
			break;

		if ((src_frm_addr >= p_vdh_mem_map[index].start_phy_addr)
			&& (src_frm_addr <= (p_vdh_mem_map[index].start_phy_addr
			+ p_vdh_mem_map[index].size))) {
			is_mapped = 1;
			break;
		}
	}

	if (!is_mapped) {
		dprint(PRN_FATAL, "%s src_frm_addr is out of range (%d)\n", __func__, index);
		return VDMHAL_ERR;
	}
	return VDMHAL_OK;
}

SINT32 check_pmv_buf_addr(UADDR src_pmv_addr, mem_buffer_s *p_vdh_mem_map)
{
	if ((src_pmv_addr < p_vdh_mem_map[VDH_SHAREFD_PMV_BUF].start_phy_addr)
		|| (src_pmv_addr > (p_vdh_mem_map[VDH_SHAREFD_PMV_BUF].start_phy_addr
		+ p_vdh_mem_map[VDH_SHAREFD_PMV_BUF].size))) {
		return VDMHAL_ERR;
	}
	return VDMHAL_OK;
}

SINT32 vctrl_vdh_unmap_message_pool(mem_buffer_s *p_mem_map)
{
	if (p_mem_map->is_mapped == 1) {
		p_mem_map->is_map_virtual = 1;
	}
	return VCTRL_OK;
}

SINT32 vctrl_vdh_map_message_pool(
	mem_buffer_s *p_mem_map, hi_s32 share_fd, hi_bool is_vdh_all_buf_remap)
{
	hi_s32 ret;

	vdec_scene scene = p_mem_map->scene;

	if ((p_mem_map->is_mapped == 1) && (is_vdh_all_buf_remap == HI_TRUE)) {
		p_mem_map->is_map_virtual = 1;
		ret = memset_s(p_mem_map, sizeof(*p_mem_map), 0, sizeof(*p_mem_map));
		if (ret != EOK) {
			dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
			return VCTRL_ERR;
		}
		p_mem_map->scene = scene;
	}

	if (p_mem_map->is_mapped == 0) {
		p_mem_map->is_map_virtual = 1;
		ret = vdec_mem_get_map_info(share_fd, p_mem_map);
		vctrl_assert_ret((ret == HI_SUCCESS), "share fd map failed");
		p_mem_map->is_mapped = 1;
		p_mem_map->share_fd = share_fd;

		if (p_mem_map->scene == SCENE_VIDEO)
			ret = vdmhal_imp_open_hal(p_mem_map);
		else if (p_mem_map->scene == SCENE_HEIF)
			ret = vdmhal_imp_open_heif_hal(p_mem_map);

		if (ret) {
			vdec_mem_put_map_info(p_mem_map);
			ret = memset_s(p_mem_map, sizeof(*p_mem_map), 0, sizeof(*p_mem_map));
			if (ret != EOK) {
				dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
			}
			dprint(PRN_FATAL, "%s vdmhal_imp_open_hal is failed\n", __func__);
			return VCTRL_ERR;
		}
	}

	return VCTRL_OK;
}
#endif

#ifdef HI_TVP_SUPPORT
#define notify_and_wait_tvp_process(notify_flag) \
do { \
	notify_flag = true; \
	wake_up_interruptible(&vdec_tvp.waitq); \
	sleep_count = 0; \
	do { \
		VFMW_OSAL_M_SLEEP(WAIT_TASK_EXIT_INER); \
		sleep_count++; \
	} while (notify_flag && \
	(sleep_count < (WAIT_TASK_EXIT_TIMEOUT / WAIT_TASK_EXIT_INER))); \
	if (notify_flag) \
		dprint(PRN_ALWS, "task don't process\n"); \
	dprint(PRN_ALWS, "notify flag %d, sleep count %d\n", notify_flag, sleep_count); \
} while (0)
#endif

VOID vctrl_suspend(void)
{
	UINT8 is_scd_sleep = 0;
	UINT8 is_vdm_sleep = 0;
	UINT32 sleep_count = 0;
	UINT32 begin_time, entr_time, cur_time;

	entr_time = VFMW_OSAL_GET_TIME_IN_MS();

	scd_drv_prepare_sleep();

	vdmhal_prepare_sleep();

	begin_time = VFMW_OSAL_GET_TIME_IN_MS();
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
	notify_and_wait_tvp_process(vdec_tvp.suspend);
#endif

	cur_time = VFMW_OSAL_GET_TIME_IN_MS();
	dprint(PRN_ALWS, "Vfmw suspend totally take %d ms\n", time_period(entr_time, cur_time));

}

VOID vctrl_resume(void)
{
	UINT32 entr_time, cur_time;
#ifdef HI_TVP_SUPPORT
	UINT32 sleep_count = 0;
#endif

	entr_time = VFMW_OSAL_GET_TIME_IN_MS();

	smmu_init_global_reg();

	scd_drv_exit_sleep();

	vdmhal_exit_sleep();

#ifdef HI_TVP_SUPPORT
	notify_and_wait_tvp_process(vdec_tvp.resume);
#endif

	cur_time = VFMW_OSAL_GET_TIME_IN_MS();
	dprint(PRN_ALWS, "Vfmw resume totally take %d ms\n", time_period(entr_time, cur_time));
}

static irqreturn_t vctrl_isr(int irq, VOID *dev_id)
{
	UINT32 d_32;

	d_32 = rd_scd_reg(REG_SCD_INI_CLR) & 0x1;
	if (d_32 == 1)
		scd_drv_isr();

	rd_vreg(VREG_INT_STATE, d_32, 0);
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
		(OSAL_IRQ_HANDLER_T)vctrl_isr, IRQF_DISABLED,
		"vdec_norm_irq", NULL) != 0) {
		dprint(PRN_FATAL, "Request vdec norm irq %d failed\n", irq_num_norm);
		return VCTRL_ERR;
	}
#endif

#if !defined(SMMU_BUSY_WAITTING)
#ifdef ENV_SOS_KERNEL
	// for 2.6.24 after
	if (VFMW_OSAL_REQUEST_IRQ(irq_num_prot,
		(OSAL_IRQ_HANDLER_T)vctrl_isr, IRQF_DISABLED,
		"vdec_prot_smmu_irq", NULL) != 0) {
		dprint(PRN_FATAL, "Request vdec prot irq %d failed\n", irq_num_prot);
		return VCTRL_ERR;
	}
#endif
#endif

	return VCTRL_OK;
}

static VOID vctrl_free_irq(
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

	return VCTRL_OK;
}

static VOID vctrl_hal_deinit(void)
{
#ifdef HIVDEC_SMMU_SUPPORT
	smmu_deinit();
#endif
	vdmhal_imp_deinit();
	scd_drv_deinit();
}

static SINT32 vctrl_scd_get_addr_info(
	mem_buffer_s *p_mem_map, scd_config_reg_s *ctrl_reg)
{
	hi_s32 ret;
	hi_u32 index;

	vctrl_assert_ret((p_mem_map != HI_NULL), "p_mem_map parameter error");
	vctrl_assert_ret((ctrl_reg != HI_NULL), "ctrl_reg parameter error");
	vctrl_assert_ret((ctrl_reg->scd_output_buf_num <= SCD_OUTPUT_BUF_CNT),
					 "scd output buffer num is out of range");

	for (index = 0;
			index < (SCD_SHAREFD_OUTPUT_BUF +
			ctrl_reg->scd_output_buf_num); index++) {
		if ((p_mem_map[index].is_mapped == 0) ||
				(ctrl_reg->is_scd_all_buf_remap)) {
			p_mem_map[index].is_map_virtual = 0;
			ret = vdec_mem_get_map_info(
				ctrl_reg->scd_share_fd[index], &p_mem_map[index]);
			vctrl_assert_ret((ret == HI_SUCCESS),
				"share fd map is failed");
			p_mem_map[index].is_mapped = 1;
			p_mem_map[index].share_fd =
				ctrl_reg->scd_share_fd[index];
			vdec_mem_put_map_info(&p_mem_map[index]);
		}
	}
	return VCTRL_OK;
}

/* New Func added for judging current scenario is Video or HEIF */
static SINT32 vdec_mem_check_scene(mem_buffer_s *p_mem_map, hi_s32 share_fd)
{
	hi_s32 ret;
	vdec_scene scene;

	p_mem_map->is_map_virtual = 0;
	ret = vdec_mem_get_map_info(share_fd, p_mem_map);
	vctrl_assert_ret((ret == VCTRL_OK), "msg sharefd map failed");

	if (p_mem_map->size < MIN_VIDEO_MSG_POOL_SIZE)
		scene = SCENE_HEIF;
	else
		scene = SCENE_VIDEO;

	vdec_mem_put_map_info(p_mem_map);
	ret = memset_s(p_mem_map, sizeof(*p_mem_map), 0, sizeof(*p_mem_map));
	if (ret != EOK) {
		dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
		return VCTRL_ERR;
	}
	p_mem_map->scene = scene;

	return VCTRL_OK;
}

static SINT32 vctrl_get_msg_pool_addr(
	mem_buffer_s *p_mem_map, mem_buffer_s *p_com_msg_map,
	hi_s32 share_fd, hi_bool is_vdh_all_buf_remap)
{
	hi_s32 ret;

	if (p_mem_map->is_mapped != 1) {
		ret = vdec_mem_check_scene(p_mem_map, share_fd);
		if (ret) {
			dprint(PRN_FATAL, "%s %d %s\n", __func__, __LINE__, "msg sharefd map failed");
			return ret;
		}
	}

	if (p_mem_map->scene == SCENE_VIDEO) {
		ret = vctrl_vdh_map_message_pool(p_com_msg_map,
			share_fd, is_vdh_all_buf_remap);
		vctrl_assert_ret((ret == VCTRL_OK),
			"video scenario, msg sharefd map failed");
		ret = memcpy_s(p_mem_map, sizeof(*p_mem_map), p_com_msg_map,
			sizeof(*p_com_msg_map));
		if (ret != EOK) {
			dprint(PRN_FATAL, " %s %d memcpy_s err in function\n", __func__, __LINE__);
			return VCTRL_ERR;
		}
	} else {
		ret = vctrl_vdh_map_message_pool(p_mem_map,
			share_fd, is_vdh_all_buf_remap);
		vctrl_assert_ret((ret == VCTRL_OK),
			"heif scenario, msg sharefd map failed");
	}

	return VCTRL_OK;
}

static SINT32 vctrl_vdh_get_addr_info(
	mem_buffer_s *p_mem_map, mem_buffer_s *p_com_msg_map,
	omxvdh_reg_cfg_s *p_vdm_reg_cfg)
{
	hi_s32 ret;
	hi_u32 index;
	hi_s32 *share_fd = &(p_vdm_reg_cfg->vdh_share_fd[0]);
	hi_bool is_vdh_frm_buf_remap = p_vdm_reg_cfg->is_frm_buf_remap;
	hi_bool is_vdh_pmv_buf_remap = p_vdm_reg_cfg->is_pmv_buf_remap;
	hi_bool is_vdh_all_buf_remap = p_vdm_reg_cfg->is_all_buf_remap;

	vctrl_assert_ret((p_mem_map != HI_NULL), "p_mem_map parameter error");
	vctrl_assert_ret((p_vdm_reg_cfg->vdh_frm_buf_num
		<= MAX_FRAME_NUM), "vdh_frm_buf_num error");
	vctrl_assert_ret((p_vdm_reg_cfg->vdh_stream_buf_num
		<= VDH_STREAM_BUF_CNT), "vdh_stream_buf_num error");

	for (index = 0; index < (VDH_SHAREFD_FRM_BUF
			+ p_vdm_reg_cfg->vdh_frm_buf_num); index++) {
		/* do not get addr info for addtional stream buffer share fd */
		if ((index >= (VDH_SHAREFD_STREAM_BUF +
				p_vdm_reg_cfg->vdh_stream_buf_num)) &&
				(index < VDH_SHAREFD_PMV_BUF)) {
			continue;
		}
#ifdef MSG_POOL_ADDR_CHECK
		if (index == VDH_SHAREFD_MESSAGE_POOL) {
			ret = vctrl_get_msg_pool_addr(&p_mem_map[index],
				p_com_msg_map, share_fd[index],
				is_vdh_all_buf_remap);
			vctrl_assert_ret((ret == VCTRL_OK),
				"msg sharefd map failed");
			continue;
		}
#endif
		if ((p_mem_map[index].is_mapped == 1) && (is_vdh_all_buf_remap)) {
			ret = memset_s(&p_mem_map[index], sizeof(p_mem_map[index]),
				0, sizeof(p_mem_map[index]));
			if (ret != EOK) {
				dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
				return VCTRL_ERR;
			}
		}

		/* pmv or fmv remap */
		if ((p_mem_map[index].is_mapped == 1) &&
				(((index == VDH_SHAREFD_PMV_BUF) &&
				(is_vdh_pmv_buf_remap)) ||
				((index >= VDH_SHAREFD_FRM_BUF) &&
				(is_vdh_frm_buf_remap)))) {
				ret = memset_s(&p_mem_map[index], sizeof(p_mem_map[index]),
					0, sizeof(p_mem_map[index]));
				if (ret != EOK) {
					dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
					return VCTRL_ERR;
				}
		}

		if (p_mem_map[index].is_mapped == 0) {
			p_mem_map[index].is_map_virtual = 0;
			ret = vdec_mem_get_map_info(share_fd[index],
				&p_mem_map[index]);
			if (ret != HI_SUCCESS) {
				dprint(PRN_FATAL, "share fd map failed, index is %d", index);
				return VCTRL_ERR;
			}
			p_mem_map[index].is_mapped = 1;
			p_mem_map[index].share_fd = share_fd[index];
			vdec_mem_put_map_info(&p_mem_map[index]);
		}
	}
	return VCTRL_OK;
}

#ifdef HI_TVP_SUPPORT
static int vdec_tvp_task(void *data)
{
	int ret;

	dprint(PRN_ALWS, "enter vdec tvp task\n");

	if (tvp_vdec_secure_init() != VDEC_OK)
		dprint(PRN_FATAL, "%s, tvp vdec init failed\n", __func__);

	while (!kthread_should_stop()) {
		dprint(PRN_ALWS, "before wait event interruptible\n");
		ret = wait_event_interruptible(
				  vdec_tvp.waitq,
				  (vdec_tvp.resume ||
				   vdec_tvp.suspend ||
				   (kthread_should_stop()))); /* lint !e666 */
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
				pst_mem->vir_addr, pst_mem->length)
				!= MEM_MAN_OK) {
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
			 // needn't return fail for normal video play
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
		if (ret != EOK) {
			dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
			return VCTRL_ERR;
		}
	}

	vctrl_free_irq(g_vdec_irq_num_norm, g_vdec_irq_num_prot, g_vdec_irq_num_safe);

#ifdef HI_TVP_SUPPORT
	if (vdec_tvp.task != NULL) {
		kthread_stop(vdec_tvp.task);
		ret = memset_s(&vdec_tvp, sizeof(vdec_tvp), 0, sizeof(vdec_tvp));
		if (ret != EOK) {
			dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
			return VCTRL_ERR;
		}
	}
#endif
	return VCTRL_OK;
}

SINT32 vctrl_vdm_hal_process(omxvdh_reg_cfg_s *p_vdm_reg_cfg, vdmhal_backup_s *p_vdm_reg_state,
							 mem_buffer_s *p_vdh_mem_map, mem_buffer_s *p_com_msg_map)
{
	hi_s32 ret;
	vdmdrv_sleep_stage_e sleep_state;

	sleep_state = vdmhal_get_sleep_stage();
	if (sleep_state == VDMDRV_SLEEP_STAGE_SLEEP) {
		dprint(PRN_ALWS, "vdm sleep state\n");
		return HI_FAILURE;
	}

	if (p_vdm_reg_cfg->vdh_reset_flag)
		vdmhal_imp_reset_vdm(0);

	ret = vctrl_vdh_get_addr_info(&p_vdh_mem_map[0], p_com_msg_map, p_vdm_reg_cfg);
	if (ret) {
		dprint(PRN_FATAL, "vctrl_vdh_get_addr_info is failed\n");
		return HI_FAILURE;
	}

	VFMW_OSAL_INIT_EVENT(G_VDMHWDONEEVENT, 0);
	ret = vdmhal_hw_dec_proc(p_vdm_reg_cfg, p_vdh_mem_map);

	if (ret) {
		dprint(PRN_FATAL, "%s config error\n", __func__);
	} else {
		ret = VFMW_OSAL_WAIT_EVENT(G_VDMHWDONEEVENT,
			((g_is_fpga == 0) ? VDM_TIMEOUT : VDM_FPGA_TIMEOUT));
		if (ret == HI_SUCCESS) {
			vdmhal_getregstate(p_vdm_reg_state);
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
	omx_scd_reg_cfg_s *p_scd_reg_cfg, scd_state_reg_s *p_scd_state_reg,
	mem_buffer_s *p_scd_mem_map)
{
	hi_s32 ret;
	scd_drv_sleep_stage_e sleep_state;
	config_scd_cmd cmd = p_scd_reg_cfg->cmd;

	sleep_state = scd_drv_get_sleep_stage();
	if (sleep_state == SCDDRV_SLEEP_STAGE_SLEEP) {
		dprint(PRN_ALWS, "SCD sleep state\n");
		return HI_FAILURE;
	}

	if (p_scd_reg_cfg->s_reset_flag) {
		if (scd_drv_reset_scd() != HI_SUCCESS) {
			dprint(PRN_FATAL, "VDEC_IOCTL_SCD_WAIT_HW_DONE  Reset SCD failed\n");
			return HI_FAILURE;
		}
	}

	ret = vctrl_scd_get_addr_info(&p_scd_mem_map[0], &(p_scd_reg_cfg->sm_ctrl_reg));
	if (ret != VCTRL_OK) {
		dprint(PRN_FATAL, "memory map failure\n");
		return HI_FAILURE;
	}

	switch (cmd) {
	case CONFIG_SCD_REG_CMD:
		VFMW_OSAL_INIT_EVENT(G_SCDHWDONEEVENT, 0);
		ret = scd_drv_write_reg(&p_scd_reg_cfg->sm_ctrl_reg, p_scd_mem_map);
		if (ret != HI_SUCCESS) {
			dprint(PRN_FATAL, "SCD busy\n");
			return HI_FAILURE;
		}

		ret = VFMW_OSAL_WAIT_EVENT(G_SCDHWDONEEVENT,
			((g_is_fpga == 0) ? SCD_TIMEOUT : SCD_FPGA_TIMEOUT));
		if (ret == HI_SUCCESS) {
			scd_drv_get_reg_state(p_scd_state_reg);
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
	rd_vreg(SCEN_IDENT, value, 0);
#else
	value = rd_scd_reg(SCEN_IDENT);
#endif
#ifdef OMXVDEC_TVP_CONFLICT
	if ((value != 0) && (value != current->tgid)
			&& (cmd != VDEC_IOCTL_SET_CLK_RATE)) {
#else
	if (value == current->tgid) {
#endif
		return HI_TRUE;
	}

	return HI_FALSE;
}

hi_s32 vfmw_drv_mod_init(hi_void)
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
	hi_s32 ret;
	ret = memset_s(&vdec_tvp, sizeof(vdec_tvp), 0, sizeof(vdec_tvp));
	if (ret != EOK) {
		dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
		return VCTRL_ERR;
	}

#endif

	return 0;
}

hi_void vfmw_drv_mod_exit(hi_void)
{
#ifdef MODULE
	dprint(PRN_ALWS, "%s : Unload hi_vfmw.ko (%d) success\n", __func__, VFMW_VERSION_NUM);
#endif
}

module_init(vfmw_drv_mod_init);
module_exit(vfmw_drv_mod_exit);

MODULE_AUTHOR("gaoyajun");
MODULE_LICENSE("GPL");
