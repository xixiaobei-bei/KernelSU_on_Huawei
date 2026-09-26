#include <linux/err.h>
#include <linux/printk.h>
#include "memory.h"

#define MEM_HISPCPE_SIZE   (0x00300000) /*  the cpe iova size */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

static unsigned int       g_mem_used[MEM_ID_MAX]  = {0};
static unsigned long long g_mem_va[MEM_ID_MAX] = {0};
static unsigned int       g_mem_da[MEM_ID_MAX] = {0};

unsigned int g_mem_offset[MEM_ID_MAX] = {
	[MEM_ID_CMDLST_BUF_MM         ] = ipp_align_up(CMDLST_BUFFER_SIZE * MAX_CPE_STRIPE_NUM, CVDR_ALIGN_BYTES),//MEM_CMDLST_BUF_MM_OFFSET
	[MEM_ID_CMDLST_ENTRY_MM       ] = ipp_align_up(sizeof(schedule_cmdlst_link_t) * MAX_CPE_STRIPE_NUM, CVDR_ALIGN_BYTES),//MEM_CMDLST_ENTRY_MM_OFFSET
	[MEM_ID_CMDLST_PARA_MM        ] = ipp_align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),//MEM_CMDLST_PARA_MM_OFFSET
	[MEM_ID_MCF_CFG_TAB           ] = ipp_align_up(sizeof(mcf_config_table_t) * MAX_CPE_STRIPE_NUM, CVDR_ALIGN_BYTES),//MEM_MCF_CFG_TAB_OFFSET
	[MEM_ID_CPETOP_CFG_TAB_MM     ] = ipp_align_up(sizeof(cpe_top_config_table_t) * MAX_CPE_STRIPE_NUM, CVDR_ALIGN_BYTES),//MEM_CPETOP_CFG_TAB_MM_OFFSET
	[MEM_ID_CVDR_CFG_TAB_MM       ] = ipp_align_up(sizeof(cfg_tab_cvdr_t) * MAX_CPE_STRIPE_NUM, CVDR_ALIGN_BYTES),//MEM_CVDR_CFG_TAB_MM_OFFSET
	[MEM_ID_SLAM_CFG_TAB          ] = ipp_align_up(sizeof(cfg_tab_slam_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES), //MEM_SLAM_CFG_TAB_OFFSET
	[MEM_ID_CMDLST_BUF_SLAM       ] = ipp_align_up(CMDLST_BUFFER_SIZE * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES), //MEM_CMDLST_BUF_SLAM_OFFSET
	[MEM_ID_CMDLST_ENTRY_SLAM     ] = ipp_align_up(sizeof(schedule_cmdlst_link_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES), //MEM_CMDLST_ENTRY_SLAM_OFFSET
	[MEM_ID_CMDLST_PARA_SLAM      ] = ipp_align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),//MEM_CMDLST_PARA_SLAM_OFFSET
	[MEM_ID_CPETOP_CFG_TAB_SLAM   ] = ipp_align_up(sizeof(cpe_top_config_table_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES), //MEM_CPETOP_CFG_TAB_SLAM_OFFSET
	[MEM_ID_CVDR_CFG_TAB_SLAM     ] = ipp_align_up(sizeof(cfg_tab_cvdr_t) * CMDLST_STRIPE_MAX_NUM, CVDR_ALIGN_BYTES), //MEM_CVDR_CFG_TAB_SLAM_OFFSET
	[MEM_ID_SLAM_STAT             ] = ipp_align_up(SLAM_SCOREHIST_NUM * SLAM_LAYER_MAX, CVDR_ALIGN_BYTES),//MEM_SLAM_STAT_OFFSET
	[MEM_ID_VBK_CFG_TAB           ] = ipp_align_up(sizeof(vbk_config_table_t), CVDR_ALIGN_BYTES),//MEM_VBK_CFG_TAB_OFFSET
	[MEM_ID_REORDER_CFG_TAB       ] = ipp_align_up(sizeof(cfg_tab_reorder_t) * SLAM_LAYER_MAX, CVDR_ALIGN_BYTES),//MEM_REORDER_CFG_TAB_OFFSET
	[MEM_ID_CMDLST_BUF_REORDER    ] = ipp_align_up(CMDLST_BUFFER_SIZE * SLAM_LAYER_MAX, CVDR_ALIGN_BYTES),//MEM_CMDLST_BUF_REORDER_OFFSET
	[MEM_ID_CMDLST_ENTRY_REORDER  ] = ipp_align_up(sizeof(schedule_cmdlst_link_t) * SLAM_LAYER_MAX, CVDR_ALIGN_BYTES),//MEM_CMDLST_ENTRY_REORDER_OFFSET
	[MEM_ID_CMDLST_PARA_REORDER   ] = ipp_align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),//MEM_CMDLST_PARA_REORDER_OFFSET
	[MEM_ID_CPETOP_CFG_TAB_REORDER] = ipp_align_up(sizeof(cpe_top_config_table_t), CVDR_ALIGN_BYTES),//MEM_CPETOP_CFG_TAB_REORDER_OFFSET
	[MEM_ID_CVDR_CFG_TAB_REORDER  ] = ipp_align_up(sizeof(cfg_tab_cvdr_t) * SLAM_LAYER_MAX, CVDR_ALIGN_BYTES),//MEM_CVDR_CFG_TAB_REORDER_OFFSET
	[MEM_ID_COMPARE_CFG_TAB       ] = ipp_align_up(sizeof(cfg_tab_compare_t) * SLAM_LAYER_MAX, CVDR_ALIGN_BYTES),//MEM_COMPARE_CFG_TAB_OFFSET
	[MEM_ID_CMDLST_BUF_COMPARE    ] = ipp_align_up(CMDLST_BUFFER_SIZE *SLAM_LAYER_MAX * 3, CVDR_ALIGN_BYTES), //MEM_CMDLST_BUF_COMPARE_OFFSET
	[MEM_ID_CMDLST_ENTRY_COMPARE  ] = ipp_align_up(sizeof(schedule_cmdlst_link_t) * SLAM_LAYER_MAX * 3, CVDR_ALIGN_BYTES), //MEM_CMDLST_ENTRY_COMPARE_OFFSET
	[MEM_ID_CMDLST_PARA_COMPARE   ] = ipp_align_up(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),//MEM_CMDLST_PARA_COMPARE_OFFSET
	[MEM_ID_CPETOP_CFG_TAB_COMPARE] = ipp_align_up(sizeof(cpe_top_config_table_t), CVDR_ALIGN_BYTES),//MEM_CPETOP_CFG_TAB_COMPARE_OFFSET
	[MEM_ID_CVDR_CFG_TAB_COMPARE  ] = ipp_align_up(sizeof(cfg_tab_cvdr_t) * SLAM_LAYER_MAX * 3, CVDR_ALIGN_BYTES), //MEM_CVDR_CFG_TAB_COMPARE_OFFSET
	[MEM_ID_READ_COMPARE_INDEX    ] = ipp_align_up(CPMPARE_INDEX_NUM * SLAM_LAYER_MAX, CVDR_ALIGN_BYTES),//MEM_ID_READ_COMPARE_INDEX_OFFSET
	[MEM_ID_COMPARE_MATCHED_KPT   ] = ipp_align_up(4 * SLAM_LAYER_MAX, CVDR_ALIGN_BYTES),//MEM_ID_READ_COMPARE_INDEX_OFFSET
};

int cpe_mem_init(unsigned long long va, unsigned int da, unsigned int size)
{
	int i = 0;

	for (i = 0; i < MEM_ID_MAX; i++) {
		g_mem_used[i]  = 0;

		if (i == 0) {
			g_mem_va[i] = va;
			g_mem_da[i] = da;
		} else {
			g_mem_va[i] = g_mem_va[i - 1] + g_mem_offset[i - 1];
			g_mem_da[i] = g_mem_da[i - 1] + g_mem_offset[i - 1];
		}
	}

	if (g_mem_va[MEM_ID_MAX - 1] - va > size) {
		e("[%s] Failed : vaddr overflow %d\n", __func__, size);
		return -ENOMEM;
	}

	if (g_mem_da[MEM_ID_MAX - 1] - da > size) {
		e("[%s] Failed : daddr overflow %d\n", __func__, size);
		return -ENOMEM;
	}

	return 0;
}

int cpe_init_memory(unsigned int ipp_buff_size)
{
	unsigned int da = 0;
	unsigned long long va = 0;
	int ret = 0;

	if(ipp_buff_size > MEM_HISPCPE_SIZE){
		pr_err("[%s] Failed : ipp_buff_size overflow 0x%08x\n", __func__, ipp_buff_size);
		return -ENOMEM;
	}

	if ((da = get_cpe_addr_da()) == 0) {
		e("[%s] Failed : CPE Device da false.\n", __func__);
		return -ENOMEM;
	}

	if ((va = (unsigned long long)(uintptr_t)(get_cpe_addr_va())) == 0) {
		e("[%s] Failed : CPE Device va false.\n", __func__);
		return -ENOMEM;
	}

	if ((ret = cpe_mem_init(va, da, ipp_buff_size)) != 0) {
		e("[%s] Failed : cpe_mem_init 0x%x %d\n", __func__, ipp_buff_size, ret);
		return -ENOMEM;
	}

	return 0;
}

int cpe_mem_get(cpe_mem_id mem_id, unsigned long long *va, unsigned int *da)
{
	if (va == NULL || da == NULL) {
		e("[%s] Failed : va.%pK, da.%pK\n", __func__, va, da);
		return -ENOMEM;
	}

	if (mem_id >= MEM_ID_MAX) {
		e("[%s] Failed : mem_id.(%u >= %u)\n", __func__, mem_id, MEM_ID_MAX);
		return -ENOMEM;
	}

	if (g_mem_used[mem_id] == 1) {
		e("[%s] Failed : g_mem_used[%u].%u\n", __func__, mem_id, g_mem_used[mem_id]);
		return -ENOMEM;
	}

	if (!(*da = (unsigned int)g_mem_da[mem_id]) || !(*va = (unsigned long long)g_mem_va[mem_id])) {
		e("[%s] Failed : mem_get[%u]\n", __func__, mem_id);
		return -ENOMEM;
	}

	g_mem_used[mem_id] = 1;
	return 0;
}

void cpe_mem_free(cpe_mem_id mem_id)
{
	if (mem_id >= MEM_ID_MAX) {
		e("Failed : Invalid parameter! mem_id = %u\n", mem_id);
		return;
	}

	if (g_mem_used[mem_id] == 0) {
		e("Failed : Unable to free, g_mem_used[%u].%u", mem_id, g_mem_used[mem_id]);
		return;
	}

	g_mem_used[mem_id] = 0;
	return;
}

#pragma GCC diagnostic pop

