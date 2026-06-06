/*
* NoC. (NoC Mntn Module.)
*
* Copyright (c) 2016 Huawei Technologies CO., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*/

#include <linux/io.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/bug.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include <linux/jiffies.h>
#include <linux/nmi.h>
#include <linux/kthread.h>

#include "hisi_noc_err_probe.h"
#include "hisi_noc.h"
#include "hisi_noc_info.h"
#include <linux/hisi/util.h>
#include <linux/hisi/reset.h>
#include <linux/hisi/hisi_hw_diag.h>
#include "securec.h"
#include <soc_mid.h>
#include <hisi_noc_modid_para.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/hisi/rdr_pub.h>

static char str_out_of_range[] = { "out of range!" };
static struct noc_errorprobe_backup_info noc_info_backup = {0, 0, NULL};
static struct workqueue_struct *noc_err_handle_wq;
static struct work_struct noc_err_handle_w;
static struct work_struct noc_timeout_handle_w;
static int g_noc_err_coreid;
static struct hisi_noc_device *g_noc_dev;
static struct err_probe_msg g_err_msg;

extern int sensorhub_noc_notify(int value);

static struct noc_err_para_s err_info;


int __weak sensorhub_noc_notify(int value)
{
    return 0;
}

static void NOC_opti_init(void)
{
	err_info.masterid = ERR_INFO_INIT;
	err_info.targetflow = ERR_INFO_INIT;
	err_info.bus = ERR_INFO_INIT;
}
static void NOC_opti_set_masterid(u32 mid)
{
	err_info.masterid = mid;
}
static void NOC_opti_set_targetflow(u32 tarflow)
{
	err_info.targetflow= tarflow;
}
static void NOC_opti_set_busid(unsigned int busid)
{
	err_info.bus= busid;
}
static struct noc_err_para_s hisi_noc_get_err_info(void)
{
	return err_info;
}
static void hisi_noc_err_handle_work_func(struct work_struct *work)
{
		pr_info("hisi_noc_err_handle_work_func 0x%lx\n", (uintptr_t)work);

	switch(g_noc_err_coreid) {
	case RDR_CP:
		pr_err("RDR_CP noc_error happen\n");
		rdr_system_error(RDR_MODEM_NOC_MOD_ID, 0, 0);
		break;
	case RDR_IOM3:
		pr_err("sensorhub noc_error happen\n");
		sensorhub_noc_notify(g_err_msg.opc.val);
		break;
	case RDR_HIFI:
		pr_err("RDR_HIFI noc_error happen\n");
		rdr_system_error(RDR_AUDIO_NOC_MODID, 0, 0);
		break;
	default:
		pr_err("g_noc_err_coreid is unknown[%d]\n", g_noc_err_coreid);
		break;
	}
}


static void hisi_noc_timeout_handle_work_func(struct work_struct *work)
{
    pr_err("hisi_noc_timeout_handle_work_func 0x%lx\n", (uintptr_t)work);
    rdr_syserr_process_for_ap((u32)MODID_AP_S_NOC, (u64)0, (u64)1);
}

static LIST_HEAD(__noc_modid_list);
static DEFINE_SPINLOCK(__noc_modid_list_lock);

static s32 noc_opti_sec_check(s32 mod_cnt)
{
if (mod_cnt > NOC_OPTI_LIST_MAX_LENGTH)
	   return NOC_OPTI_FAIL;
else
	   return NOC_OPTI_SUCCESS;
}
static u32 is_para_registerd(struct noc_err_para_s *noc_err_info_check)
{
	struct noc_mid_modid_trans_s *p_modid_e = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;
	unsigned long lock_flag;

	spin_lock_irqsave(&__noc_modid_list_lock, lock_flag);

	list_for_each_safe(cur, next, &__noc_modid_list) {
		p_modid_e = list_entry(cur, struct noc_mid_modid_trans_s, s_list);
		if (p_modid_e == NULL) {
			pr_err("It might be better to look around here. %s:%d\n",
				 __func__, __LINE__);
			continue;
		}
		if ((p_modid_e->err_info_para.masterid== noc_err_info_check->masterid)&&(p_modid_e->err_info_para.targetflow == noc_err_info_check->targetflow)&&(p_modid_e->err_info_para.bus == noc_err_info_check->bus)) {
			spin_unlock_irqrestore(&__noc_modid_list_lock,
						   lock_flag);
			return MODID_EXIST;
		}
	}

	spin_unlock_irqrestore(&__noc_modid_list_lock, lock_flag);

	return NOC_OPTI_SUCCESS;
}


static u32 __noc_find_modid(struct noc_err_para_s *noc_err_info)
{
	struct noc_mid_modid_trans_s *p_modid_e = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;
	unsigned long lock_flag;
	spin_lock_irqsave(&__noc_modid_list_lock, lock_flag);

	list_for_each_safe(cur, next, &__noc_modid_list) {
		p_modid_e = list_entry(cur, struct noc_mid_modid_trans_s, s_list);
		if (p_modid_e == NULL) {
			pr_err("It might be better to look around here. %s:%d\n",
				 __func__, __LINE__);
			continue;
		}
		if ((noc_err_info->masterid == p_modid_e->err_info_para.masterid)
			&&((noc_err_info->targetflow == p_modid_e->err_info_para.targetflow) ||
				(p_modid_e->err_info_para.targetflow == 0xFF))
			&&(noc_err_info->bus == p_modid_e->err_info_para.bus)) {
			spin_unlock_irqrestore(&__noc_modid_list_lock,
						   lock_flag);
			pr_err("=================================");
			pr_err("[%s]: modid register masterid is equel,value = 0x%x !\n", __func__,p_modid_e->err_info_para.masterid);
			pr_err("[%s]: modid register targerflow  is equel,value = 0x%x !\n", __func__,p_modid_e->err_info_para.targetflow);
			pr_err("[%s]:modid register busid is equel,value = 0x%x !\n", __func__,p_modid_e->err_info_para.bus);
			pr_err("[%s]: mod id match is ok, modid = 0x%x !\n", __func__,p_modid_e->modid);
			pr_err("=================================");
			return p_modid_e->modid;
		}
	}
	spin_unlock_irqrestore(&__noc_modid_list_lock, lock_flag);

	return MODID_NEGATIVE;
}

static s32 noc_para_check(struct noc_err_para_s noc_err_info)
{
	if (noc_err_info.masterid > MID_MAX) {
		pr_err("%s masterid over !\n", __func__);
		return NOC_OPTI_FAIL;
	}

	if (noc_err_info.bus >= NOC_ERRBUS_BOTTOM) {
		pr_err("%s bus id over !\n", __func__);
		return NOC_OPTI_FAIL;
	}

	if (noc_err_info.targetflow > TARGETFLOW_MAX) {
		pr_err("%s targetflow id over !\n", __func__);
		return NOC_OPTI_FAIL;
	}

	return NOC_OPTI_SUCCESS;

}

static void __noc_modid_register(struct noc_mid_modid_trans_s *node)
{
	struct noc_mid_modid_trans_s *p_info = NULL;
	struct list_head *cur = NULL;
	struct list_head *next = NULL;
	unsigned long lock_flag;

	if (NULL == node) {
		pr_err("invalid node:%pK\n", node);
		return;
	}

	spin_lock_irqsave(&__noc_modid_list_lock, lock_flag);

	if (list_empty(&__noc_modid_list)) {
		pr_err("list_add_tail masterid is [0x%x]\n",
				 node->err_info_para.masterid);
		list_add_tail(&(node->s_list), &__noc_modid_list);
		goto out;
	}

	list_for_each_safe(cur, next, &__noc_modid_list) {
		p_info = list_entry(cur, struct noc_mid_modid_trans_s, s_list);
		if (node->err_info_para.masterid < p_info->err_info_para.masterid) {
			pr_err("list_add2 masterid is [0x%x]\n",
					 node->err_info_para.masterid);
			list_add(&(node->s_list), cur);
			goto out;
		}
	}
	pr_err("list_add_tail2 masterid is [0x%x]\n", node->err_info_para.masterid);
	list_add_tail(&(node->s_list), &__noc_modid_list);

out:
	spin_unlock_irqrestore(&__noc_modid_list_lock, lock_flag);
}

static u32 noc_modid_find(struct noc_err_para_s noc_err_info)
{
	s32 pret = FAIL;
	u32 ret = MODID_NEGATIVE;

	pret = noc_para_check(noc_err_info);
	if (NOC_OPTI_FAIL == pret) return MODID_NEGATIVE;

	ret = __noc_find_modid(&noc_err_info);

	return ret;

}

/*
* noc modid registe API,for use to registe own process, and before this,
* noc err use the AP_S_NOC process, after adapt,user can define his own process.
*/
void noc_modid_register(struct noc_err_para_s noc_err_info,u32 modid)
{
	struct noc_mid_modid_trans_s *node = NULL;
	s32 pret = FAIL;
	u32 ret = MODID_EXIST;
	static s32 mod_cnt = NOC_OPTI_LIST_INIT;

	pret = noc_opti_sec_check(mod_cnt);
	if (NOC_OPTI_FAIL == pret){
		pr_err("%s node is full!\n", __func__);
		return;
	}
	noc_err_info.masterid &= 0x3F;
	pret = noc_para_check(noc_err_info);
	if (NOC_OPTI_FAIL == pret) return;

	/*
	* before registering modid,we have to check that has modid been registered
	* berore,double check.
	*/
	ret = is_para_registerd(&noc_err_info);
	if (MODID_EXIST == ret) {
		pr_err("%s node is exist!\n", __func__);
		return;
	}

	node = kmalloc(sizeof(struct noc_mid_modid_trans_s), GFP_ATOMIC);
	if (node == NULL) {
		pr_err("%s mem alloc fail!\n", __func__);
		return;
	}

	ret = memset_s(node, sizeof(struct noc_mid_modid_trans_s), ERR_INFO_INIT, sizeof(struct noc_mid_modid_trans_s));
	if (NOC_OPTI_SUCCESS != ret)
		return; //lint !e429
	node->err_info_para.masterid  = noc_err_info.masterid;
	node->err_info_para.targetflow = noc_err_info.targetflow;
	node->err_info_para.bus = noc_err_info.bus;
	node->modid = modid;
	pr_err("=================================");
	pr_err("[%s]: modid register master id = 0x%x !\n", __func__,node->err_info_para.masterid);
	pr_err("[%s]: modid register targetflow = 0x%x !\n", __func__,node->err_info_para.targetflow);
	pr_err("[%s]: modid register bus id = %d !\n", __func__,node->err_info_para.bus);
	pr_err("[%s]: modid register node is ok !\n", __func__);
	pr_err("=================================");
	mod_cnt++;
	/*
	this func is the real func to registe the user's modid and
	* user's err judge
	*/
	__noc_modid_register(node);
}

EXPORT_SYMBOL(noc_modid_register);

/*输出base+0x14的值的含义*/
static void print_errlog0(unsigned int idx, unsigned int val)
{
	unsigned int value;
	unsigned int shift;
	const struct noc_bus_info *noc_bus;

	pr_err("[ERR_LOG0 = 0x%x]:\n", val);

	noc_bus = noc_get_bus_info(idx);
	if (NULL == noc_bus) {
		pr_err("[%s] noc_bus get error!\n", __func__);
		return;
	}

	shift = ffs((int)noc_bus->err_code_mask) - 1;
	value = (val & (noc_bus->err_code_mask)) >> shift;

	if (value < noc_bus->err_code_array_size)
		pr_err("\t[err_code=%d] %s\n", value, noc_bus->err_code_array[value]);
	else
		pr_err("\t[err_code=%d] out of range!\n", value);

	shift = ffs((int)noc_bus->opc_mask) - 1;
	value = (val & (noc_bus->opc_mask)) >> shift;

	if (value < noc_bus->opc_array_size)
		pr_err("\t[opc=%d] %s\n", value, noc_bus->opc_array[value]);
	else
		pr_err("\t[opc=%d] out of range!\n", value);
	g_err_msg.opc.val = value;
}

/*输出输出base+0x18的值的含义*/
u64 print_errlog1(unsigned int val, unsigned int idx, int *pinitflow)

{
	const struct noc_bus_info *noc_bus;
	unsigned int shift;
	int initflow;
	int targetflow;
	int targetsubrange;
	hisi_hw_diag_info hwdiag_noc_info;

	hwdiag_noc_info.noc_info.init_flow = "null";
	hwdiag_noc_info.noc_info.target_flow = "null";

	noc_bus = noc_get_bus_info(idx);
	if (NULL == noc_bus) {
		pr_err("[%s] noc_bus get error!\n", __func__);
		return 0;
	}

	pr_err("[ERR_LOG1 = 0x%x]\n", val);

	shift = ffs((int)noc_bus->initflow_mask) - 1;
	initflow = (int)((val & (noc_bus->initflow_mask)) >> shift);
	if ((unsigned int)initflow < noc_bus->initflow_array_size) {
		*pinitflow = initflow;
		noc_info_backup.init_flow = initflow;
		pr_err("\t[init_flow=%d]: %s\n", initflow,
		       noc_bus->initflow_array[initflow]);
		hwdiag_noc_info.noc_info.init_flow = noc_bus->initflow_array[initflow];
	} else {
		*pinitflow = -1;
		pr_err("\t[init_flow=%d]: %s\n", initflow,
		       "index is out of range!");
	}

	shift = ffs((int)noc_bus->targetflow_mask) - 1;
	targetflow = (int)((val & (noc_bus->targetflow_mask)) >> shift);
	NOC_opti_set_targetflow(targetflow);

	if ((unsigned int)targetflow < noc_bus->targetflow_array_size) {
		pr_err("\t[target_flow=%d]: %s\n", targetflow,
		       noc_bus->targetflow_array[targetflow]);
		hwdiag_noc_info.noc_info.target_flow = noc_bus->targetflow_array[targetflow];
	} else {
		pr_err("\t[target_flow=%d]: %s\n", targetflow,
		       "index is out of range!");
	}

	hisi_hw_diaginfo_trace(NOC_FAULT_INFO, &hwdiag_noc_info);

	shift = ffs((int)noc_bus->targ_subrange_mask) - 1;
	targetsubrange = (int)((val & (noc_bus->targ_subrange_mask)) >> shift);
	pr_err("\t[target_subrange]: %d\n", targetsubrange);

	return noc_find_addr_from_routeid(idx, initflow, targetflow,
					  targetsubrange);

}

/*输出base+0x28的值的含义 ERRLOG5*/
void print_errlog5(unsigned int val, unsigned int idx, int initflow)
{
	struct noc_mid_info *noc_mid = NULL;
	char *mid_name = NULL;
	unsigned int i,noc_mid_size;

	noc_get_mid_info(idx, &noc_mid, &noc_mid_size);
	for (i = 0; (noc_mid != NULL) && (i < noc_mid_size); noc_mid++, i++) {
		if ((noc_mid->idx == idx)
			&& ((noc_mid->init_flow == initflow) || (0xFF == noc_mid->init_flow))
			&& (noc_mid->mid_val == (val & (unsigned int)noc_mid->mask))) {/*lint !e737*/
			mid_name = noc_mid->mid_name;
	NOC_opti_set_masterid(noc_mid->mid_val);
			break;
		}
	}

	if (mid_name != NULL)
		pr_err("[ERR_LOG5]: USER_SIGNAL = 0x%x (MID: %s )\n", val,
		       mid_name);
	else
		pr_err("[ERR_LOG5]: USER_SIGNAL = 0x%x\n", val);

	return;
}

static void print_target_name(struct hisi_noc_device *noc_dev, u64 addr)
{
	unsigned int i;

	if ((NULL != noc_dev) && (NULL != noc_dev->noc_property)) {
		for (i = 0; i < noc_dev->noc_property->noc_list_target_nums; i++) {
			if ((addr >= noc_dev->ptarget_list[i].base)
				&& (addr < (noc_dev->ptarget_list[i].end))) {
				pr_err("            target:%s\n",  noc_dev->ptarget_list[i].name);
				break;
			}
		}
	}
}

void print_errlog7(unsigned int val, unsigned int idx)
{
	struct noc_sec_info *noc_sec = NULL;
	unsigned int i, noc_sec_size;

	noc_get_sec_info(idx, &noc_sec, &noc_sec_size);
	pr_err("[ERR_LOG7]: sec_val = 0x%x\n", val);
	for (i = 0; (noc_sec != NULL) &&  (i < noc_sec_size); noc_sec++, i++) {
		if ((val&noc_sec->mask) == noc_sec->sec_val)
			pr_err("            [%s] mode is %s\n", noc_sec->sec_mode,
					noc_sec->sec_array);
	}

}
/*Output Error log buffer, one-track code for both Hi6xxx and Hi3xxx */
static void print_errlog(const void __iomem *base, unsigned int idx)
{
	unsigned int val, val_errlog3, val_errlog5;
	u64 base_addr, adjust_addr;
	int initflow = -1;
	unsigned int fama_offset;

	/* dump all the err_log register */
	val = readl_relaxed((u8 __iomem *)base +
				g_noc_dev->perr_probe_reg->err_probe_errlog0_offset);
	print_errlog0(idx, val);

	val = readl_relaxed((u8 __iomem *)base +
				g_noc_dev->perr_probe_reg->err_probe_errlog1_offset);
	base_addr = print_errlog1(val, idx, &initflow);

	val_errlog5 = readl_relaxed((u8 __iomem *)base +
				g_noc_dev->perr_probe_reg->err_probe_errlog5_offset);

	if (g_noc_dev->noc_property->noc_fama_enable)
		fama_offset = (val_errlog5 >> 6) & g_noc_dev->noc_property->noc_fama_mask;
	else
		fama_offset = 0;

	val_errlog3 = readl_relaxed((u8 __iomem *)base +
				g_noc_dev->perr_probe_reg->err_probe_errlog3_offset);
	pr_err("[ERR_LOG3]: ADDRESS_LOW = 0x%x,\n", val_errlog3);

	val = readl_relaxed((u8 __iomem *)base +
				g_noc_dev->perr_probe_reg->err_probe_errlog4_offset);
	pr_err("[ERR_LOG4]: ADDRESS_HIGH = 0x%x\n", val);

	adjust_addr = val_errlog3 + ((u64) val << 32) + base_addr + ((u64) fama_offset << 34);
	pr_err("            adjusted ADDRESS = 0x%llx\n", adjust_addr);
	print_target_name(g_noc_dev, adjust_addr);

	print_errlog5(val_errlog5, idx, initflow);

	val = readl_relaxed((u8 __iomem *)base +
				g_noc_dev->perr_probe_reg->err_probe_errlog7_offset);
	print_errlog7(val, idx);
}

void noc_err_get_msg_log0(uint *reg_val_buf, uint idx,
			  struct err_probe_msg *pt_err_msg)
{
	uint val_tmp;
	uint val;
	unsigned int shift = 0;
	const struct noc_bus_info *noc_bus = 0;

	noc_bus = noc_get_bus_info(idx);
	if (NULL == noc_bus) {
		pr_err("[%s] noc_bus get error!\n", __func__);
		return;
	}

	val = reg_val_buf[0];

	shift = ffs(noc_bus->err_code_mask) - 1;
	val_tmp = (val & (noc_bus->err_code_mask)) >> shift;

	if (val_tmp < noc_bus->err_code_array_size) {
		pt_err_msg->err_code.val = val_tmp;
		pt_err_msg->err_code.pt_str = noc_bus->err_code_array[val_tmp];
	} else
		pt_err_msg->err_code.pt_str = str_out_of_range;

	shift = ffs(noc_bus->opc_mask) - 1;
	val_tmp = (val & (noc_bus->opc_mask)) >> shift;

	if (val_tmp < noc_bus->opc_array_size) {
		pt_err_msg->opc.val = val_tmp;
		pt_err_msg->opc.pt_str = noc_bus->opc_array[val_tmp];
	} else {
		pt_err_msg->opc.pt_str = str_out_of_range;
	}
}

void noc_err_get_msg_log1(uint *reg_val_buf, uint idx,
			  struct err_probe_msg *pt_err_msg)
{
	const struct noc_bus_info *noc_bus = 0;
	unsigned int shift = 0;
	uint val;
	int initflow;
	int targetflow;

	noc_bus = noc_get_bus_info(idx);
	if (NULL == noc_bus) {
		pr_err("[%s] noc_bus get error!\n", __func__);
		return;
	}
	val = reg_val_buf[1];
	shift = ffs(noc_bus->initflow_mask) - 1;
	initflow = (val & (noc_bus->initflow_mask)) >> shift;
	pt_err_msg->init_flow.val = initflow;
	if ((unsigned int)initflow < noc_bus->initflow_array_size)
		pt_err_msg->init_flow.pt_str =
		    noc_bus->initflow_array[initflow];
	else
		pt_err_msg->init_flow.pt_str = str_out_of_range;

	shift = ffs(noc_bus->targetflow_mask) - 1;
	targetflow = (val & (noc_bus->targetflow_mask)) >> shift;
	pt_err_msg->target_flow.val = targetflow;
	if ((unsigned int)targetflow < noc_bus->targetflow_array_size)
		pt_err_msg->target_flow.pt_str =
		    noc_bus->targetflow_array[targetflow];
	else
		pt_err_msg->target_flow.pt_str = str_out_of_range;

	shift = ffs(noc_bus->targ_subrange_mask) - 1;
	pt_err_msg->targetsubrange =
	    (val & (noc_bus->targ_subrange_mask)) >> shift;

	pt_err_msg->base_addr = noc_find_addr_from_routeid(idx, initflow,
							   targetflow, pt_err_msg->targetsubrange);
}

void noc_err_get_msg_log5(uint *reg_val_buf, uint idx,
			  struct err_probe_msg *pt_err_msg)
{
	struct noc_mid_info *noc_mid = NULL;
	unsigned int i,noc_mid_size;
	int mid_val;
	char *mid_name = NULL;

	noc_get_mid_info(idx, &noc_mid, &noc_mid_size);
	pt_err_msg->user_signal = reg_val_buf[5];

	mid_val = pt_err_msg->user_signal;
	for (i = 0; (noc_mid != NULL) && (i < noc_mid_size); noc_mid++, i++) {
		if ((noc_mid->idx == idx)
		    && (noc_mid->init_flow == pt_err_msg->init_flow.val)
		    && (noc_mid->mid_val == ((unsigned int)mid_val & (unsigned int)noc_mid->mask))) {/*lint !e737*/
			mid_name = noc_mid->mid_name;
			break;
		}
	}

	pt_err_msg->mid_name = mid_name;
}

void noc_err_get_msg(uint *reg_val_buf, uint idx,
		     struct err_probe_msg *pt_err_msg)
{
	/* ERR LOG 0 */
	noc_err_get_msg_log0(reg_val_buf, idx, pt_err_msg);
	/* ERR LOG 1 */
	noc_err_get_msg_log1(reg_val_buf, idx, pt_err_msg);

	/* ERR LOG 3,4 */
	pt_err_msg->addr_low = reg_val_buf[3];
	pt_err_msg->addr_high = reg_val_buf[4];

	/* ERR LOG 5 */
	noc_err_get_msg_log5(reg_val_buf, idx, pt_err_msg);

	/* ERR LOG 7 */
	pt_err_msg->SECURITY = reg_val_buf[7];
}

void noc_err_prt_msg(struct err_probe_msg *pt_err_msg)
{
	if (NULL == pt_err_msg) {
		pr_err("[%s]:arg is NULL in\n", __func__);
		return;
	}

	/* ERR LOG 0 */
	pr_err("[err_code=%d] %s\n", pt_err_msg->err_code.val,
	       pt_err_msg->err_code.pt_str);
	pr_err("[opc=%d] %s\n", pt_err_msg->opc.val, pt_err_msg->opc.pt_str);

	/* ERR LOG 1 */
	pr_err("[init_flow=%d]: %s\n", pt_err_msg->init_flow.val,
	       pt_err_msg->init_flow.pt_str);
	pr_err("[target_flow=%d]: %s\n", pt_err_msg->target_flow.val,
	       pt_err_msg->target_flow.pt_str);
	pr_err("[target_subrange]: %d\n", pt_err_msg->targetsubrange);

	/* ERR LOG 3,4 */
	pr_err("ADDRESS_LOW = 0x%llx  (0x%x)\n",
	       (u64) (pt_err_msg->addr_low + pt_err_msg->base_addr),
	       pt_err_msg->addr_low);
	pr_err("ADDRESS_HIGH = 0x%x\n", pt_err_msg->addr_high);
	pr_err("adjusted ADDRESS = 0x%llx\n",
	       (u64) (pt_err_msg->addr_low + pt_err_msg->base_addr) +
	       ((u64) pt_err_msg->addr_high << 32) +
	       ((u64) (pt_err_msg->user_signal & 0x7c0) << 34));
	/* ERR LOG 5 */
	pr_err("[USER_SIGNAL = 0x%x MID: %s\n", pt_err_msg->user_signal,
	       pt_err_msg->mid_name);

	/* ERR LOG 7 */
	pr_err("SECURITY = %d\n", pt_err_msg->SECURITY);
}

void noc_err_probe_prt(const void __iomem *base, uint idx)
{
	uint reg_val[10];
	uint i;

	if (EOK != memset_s(&g_err_msg, sizeof(struct err_probe_msg), 0, sizeof(struct err_probe_msg))) {
		pr_err("[%s:%d]: memset_s err \n]", __func__, __LINE__);
	}

	for (i = 0; i < 8; i++) {
		reg_val[i] =
		    readl_relaxed((u8 __iomem *)base +
		    		g_noc_dev->perr_probe_reg->err_probe_errlog0_offset + 4 * i);
	}
	noc_err_get_msg(reg_val, idx, &g_err_msg);
	noc_err_prt_msg(&g_err_msg);
}

/*******************************************************************************
Function:       get_noc_err_coreid
Description:    get the coreid of noc_error
Input:          NA
Output:         NA
Return:         coreid
********************************************************************************/
static int get_noc_err_coreid(void)
{
	int i, ret;
	const struct noc_busid_initflow *filter_initflow = NULL;
	char **modemnoc_nodename = NULL;

	ret = RDR_AP;
	hisi_get_noc_initflow(&filter_initflow);
	hisi_get_modemnoc_nodename(&modemnoc_nodename);

	if (NULL == filter_initflow || NULL == modemnoc_nodename) {
		pr_err("hisi_get_noc_initflow_nodename fail\n");
		goto out;
	}

	for (i = 0; modemnoc_nodename[i] != NULL; i++) {
		if (0 == strncmp(noc_info_backup.nodename,
			modemnoc_nodename[i],
			strlen(modemnoc_nodename[i]))) {
			ret = RDR_CP;
			goto out;
		}
	}

	for (i = 0; filter_initflow[i].bus_id != ARRAY_END_FLAG; i++) {
		if (noc_info_backup.bus_id == filter_initflow[i].bus_id &&
			noc_info_backup.init_flow == filter_initflow[i].init_flow) {
			ret = filter_initflow[i].coreid;
			goto out;
		}
	}
	pr_err("%s:end\n", __func__);

out:
	return ret;
}

void noc_err_probe_hanlder(void __iomem *base, struct noc_node *node)
{
	u32 ret = MODID_NEGATIVE;
	struct noc_err_para_s err_info_status;
	NOC_opti_init();
	noc_info_backup.bus_id = node->bus_id;
	noc_info_backup.nodename = node->name;
	/* output error log buffer */
	print_errlog(base, node->bus_id);

	NOC_opti_set_busid(node->bus_id);
	/*将noc的寄存器信息打印出来 */
	noc_record_log_pstorememory(base, NOC_PTYPE_UART);

	/* disable corresponding bus node interrupt by clear FAULTEN bit. */

	/* clear interrupt */
	writel_relaxed(ERR_PROBE_CLEAR_BIT, (u8 __iomem *)base +
			g_noc_dev->perr_probe_reg->err_probe_errclr_offset);


	if (check_himntn(HIMNTN_NOC_ERROR_REBOOT)) {
		pr_err("noc happen,node->name is [%s], bus_id is [%d], initflow is [%d]\n",
			noc_info_backup.nodename,
			noc_info_backup.bus_id,
			noc_info_backup.init_flow);
		g_noc_err_coreid = get_noc_err_coreid();
		pr_err("g_noc_err_coreid is %d\n", g_noc_err_coreid);
		if (RDR_AP == g_noc_err_coreid){
	err_info_status = hisi_noc_get_err_info();
	ret = noc_modid_find(err_info_status);
	pr_err("[%s] find_noc_modid = %x!\n", __func__, ret);
	node->noc_modid = ret;
		}
		else
			queue_work(noc_err_handle_wq, &noc_err_handle_w);
	}
}

/* noc_err_save_noc_dev save noc device handle from hisi noc probe
 * @noc_dev: noc device pointer
 */
int noc_err_save_noc_dev(struct hisi_noc_device *noc_dev)
{
	if (NULL == noc_dev){
		pr_err("noc_err_save_noc_dev param = NULL!\n");
		return -1;
	}

	g_noc_dev = noc_dev;

	return 0;
}
/***************************************************************************
Function:       noc_err_probe_init
Description:    when hisi_noc is probed, this is called.
Input:          NA
Output:         NA
Return:         NA
****************************************************************************/
void noc_err_probe_init(void)
{
	noc_err_handle_wq = create_singlethread_workqueue("noc_err_handle_wq");
	if (NULL == noc_err_handle_wq) {
		pr_err("noc_err_handle_wq create fail\n");
		return;
	}
	INIT_WORK(&noc_err_handle_w, hisi_noc_err_handle_work_func);
	INIT_WORK(&noc_timeout_handle_w, hisi_noc_timeout_handle_work_func);
}

/***************************************************************************
Function:       noc_err_probe_exit
Description:    when hisi_noc is removed, this is called.
Input:          NA
Output:         NA
Return:         NA
****************************************************************************/
void noc_err_probe_exit(void)
{
	int ret;

	ret = cancel_delayed_work((struct delayed_work *)&noc_err_handle_w);
	pr_err("cancel_delayed_work return %d\n", ret);

	flush_workqueue(noc_err_handle_wq);
	destroy_workqueue(noc_err_handle_wq);
}

void enable_err_probe(void __iomem *base)
{
	unsigned int val;
	int bus_id = 0;
	unsigned int noc_bus_info_num = 0;

	val = readl_relaxed((u8 __iomem *)base +
				    g_noc_dev->perr_probe_reg->err_probe_errvld_offset);
	if (val & ERR_PORBE_ERRVLD_BIT) {

		pr_err("NoC ErrProbe happened before enabling interrupt\n");
		pr_err("NoC Error Probe:\n");

		bus_id = get_bus_id_by_base(base);

		noc_bus_info_num = hisi_noc_get_bus_info_num();

		if (bus_id >= 0 && (unsigned int)bus_id < noc_bus_info_num) {
			print_errlog(base, bus_id);
		} else {
			val = readl_relaxed((u8 __iomem *)base +
				    g_noc_dev->perr_probe_reg->err_probe_errlog0_offset);
			print_errlog0(bus_id, val);

			val = readl_relaxed((u8 __iomem *)base +
				    g_noc_dev->perr_probe_reg->err_probe_errlog1_offset);
			pr_err("[ERR_LOG1 = 0x%x]\n", val);

			val = readl_relaxed((u8 __iomem *)base +
				    g_noc_dev->perr_probe_reg->err_probe_errlog3_offset);
			pr_err("[ERR_LOG3] = 0x%x\n", val);

			val = readl_relaxed((u8 __iomem *)base +
				    g_noc_dev->perr_probe_reg->err_probe_errlog4_offset);
			pr_err("[ERR_LOG4] = 0x%x\n", val);

			val = readl_relaxed((u8 __iomem *)base +
				    g_noc_dev->perr_probe_reg->err_probe_errlog5_offset);
			pr_err("[ERR_LOG5] = 0x%x\n", val);

			val = readl_relaxed((u8 __iomem *)base +
				    g_noc_dev->perr_probe_reg->err_probe_errlog7_offset);
			pr_err("[ERR_LOG7] = %d\n", val);
		}
		/* clear errvld */
		writel_relaxed(ERR_PROBE_CLEAR_BIT,
			       base + g_noc_dev->perr_probe_reg->err_probe_errclr_offset);
		wmb();
	}

	/* enable err probe intrrupt */
	writel_relaxed(ERR_PORBE_ENABLE_BIT, (u8 __iomem *)base +
				    g_noc_dev->perr_probe_reg->err_probe_faulten_offset);
}

void disable_err_probe(void __iomem *base)
{
	writel_relaxed(~ERR_PORBE_ENABLE_BIT, (u8 __iomem *)base +
				    g_noc_dev->perr_probe_reg->err_probe_faulten_offset);
}

/* Need check if noc init is finished,
	as this interface would be called by other module */
void enable_err_probe_by_name(const char *name)
{
	void __iomem *base;

	if (!name) {
		pr_err("%s Name is NULL\n", __func__);
		return;
	}

	if (!is_noc_init()) {
		pr_err("%s: NOC not init\n", __func__);
		return;
	}

	base = get_errprobe_base(name);
	if (base == NULL) {
		pr_err("%s cannot get the node!\n", __func__);
		return;
	}

	enable_err_probe(base);
}
EXPORT_SYMBOL(enable_err_probe_by_name);

/* Need check if noc init is finished,
	as this interface would be called by other module */
void disable_err_probe_by_name(const char *name)
{
	void __iomem *base;
	if (!name) {
		pr_err("%s Name is NULL\n", __func__);
		return;
	}

	if (!is_noc_init()) {
		pr_err("%s: NOC not init\n", __func__);
		return;
	}

	base = get_errprobe_base(name);
	if (base == NULL) {
		pr_err("%s cannot get the node!\n", __func__);
		return;
	}

	disable_err_probe(base);
}
EXPORT_SYMBOL(disable_err_probe_by_name);

void noc_timeout_handler_wq(void)
{
	pr_err("noc_timeout_handler_wq enter\n");
	queue_work(noc_err_handle_wq, &noc_timeout_handle_w);
}
