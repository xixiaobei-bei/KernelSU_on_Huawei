/*
 * NoC. (NoC Mntn Module.)
 *
 * Copyright (c) 2016 Huawei Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/syscore_ops.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/debugfs.h>
#include <linux/io.h>
#include <linux/string.h>
#include <linux/hisi/util.h>
#include <securec.h>
#include <linux/cpu.h>

#include "hisi_noc.h"
#include "hisi_noc_err_probe.h"
#include "hisi_noc_packet.h"
#include "hisi_noc_info.h"
#include "hisi_noc_transcation.h"
#include "hisi_noc_dump.h"

void free_noc_bus_source(void);

char *opc_array[] = {
	"RD:  INCR READ",
	"RDW: WRAP READ",
	"RDL: EXCLUSIVE READ",
	"RDX: LOCK READ",
	"WR:  INCR WRITE",
	"WRW: WRAP WRITE",
	"WRC: EXCLUSIVE WRITE",
	"Reversed",
	"PRE: FIXED BURST",
	"URG:urgency packet"
};

/*
 * errcode
 */
char *err_code_array[] = {
	"Slave:  returns Error response",
	"Master: access reserved memory space",
	"Master: send the illigel type burst to slave",
	"Master: access the powerdown area",
	"Master: Permission error",
	"Master: received Hide Error Response from Firewall",
	"Master: accessed slave timeout and returned Error reponse",
	"None"
};

struct noc_bus_info *noc_get_tmp_bus_info(void)
{
	return g_noc_bus_info;
}

struct noc_busid_initflow *noc_get_busid_initflow(void)
{
	return g_busid_initflow;
}

struct noc_dump_reg *noc_get_dump_reg_offset(void)
{
	return g_dump_reg_offset;
}

/***************************************************************************
 * Function:       hisi_noc_get_array_size
 * Description:    get static array size
 * Input:          bus info array size
 *                 dump list array size
 * Output:         NA
 * Return:         NA
 ***************************************************************************/
void hisi_noc_get_array_size(unsigned int *bus_info_size, unsigned int *dump_list_size)
{
	if ((bus_info_size == NULL) || (dump_list_size == NULL))
		return;

	*bus_info_size = ARRAY_SIZE_NOC(g_noc_bus_info);
	*dump_list_size = ARRAY_SIZE_NOC(g_dump_reg_offset);
}

/***************************************************************************
 * Function:    hisi_noc_get_initflow_list
 * Description: get static initflow dts info
 * Input:       busid
 *              initflow_array
 * Output:       NA
 * Return:      success or fail
 ***************************************************************************/
static int hisi_noc_get_initflow_list(unsigned int busid, struct device_node *np)
{
	int ret = 0;
	unsigned int i, init_num = 0;
	unsigned long *initflow_array = NULL;
	char *name[MAX_INITFLOW_ARRAY_SIZE];

	if (of_property_read_u32(np, "init_nums", &init_num) != 0) {
		pr_err("[%s] Get init_nums from DTS error.\n", __func__);
		return FAIL;
	}

	if (init_num > MAX_INITFLOW_ARRAY_SIZE) {
		init_num = MAX_INITFLOW_ARRAY_SIZE;
		pr_err("initflow nums is too big, busid = [%d]\n", busid);
	}

	initflow_array = kzalloc(sizeof(char *) * init_num, GFP_KERNEL);
	if (initflow_array == NULL)
		return FAIL;

	ret = of_property_read_string_array(np, "init_list", (const char **)&name, init_num);
	if (ret < 0) {
		pr_err("[%s], get initflow_array fail in dts,%d!\n", __func__, ret);
		kfree(initflow_array);
		initflow_array = NULL;
		return FAIL;
	}

	for (i = 0; i < init_num; i++)
		initflow_array[i] = (unsigned long)name[i];

	g_noc_bus_info[busid].initflow_array = (char **)initflow_array;
	g_noc_bus_info[busid].initflow_array_size = init_num;

	return SUCCESS;

}

/***************************************************************************
 * Function:    hisi_noc_get_targetflow_list
 * Description: get static targetflow dts info
 * Input:       np
 *              busid
 * Output:      NA
 * Return:      success or fail
 ***************************************************************************/
static int hisi_noc_get_targetflow_list(unsigned int busid, struct device_node *np)
{
	int ret = 0;
	unsigned int i, target_num = 0;
	unsigned long *targetflow_array = NULL;
	char *name[MAX_TARGETFLOW_ARRAY_SIZE];

	/*get targetflow from dts */
	if (of_property_read_u32(np, "target_nums", &target_num) != 0) {
		pr_err("[%s] Get target_nums from DTS error.\n", __func__);
		return FAIL;
	}
	if (target_num > MAX_TARGETFLOW_ARRAY_SIZE) {
		target_num = MAX_TARGETFLOW_ARRAY_SIZE;
		pr_err("targetflow nums is too big, busid = [%d]\n", busid);
	}
	targetflow_array = kzalloc(sizeof(char *) * target_num, GFP_KERNEL);
	if (targetflow_array == NULL)
		return FAIL;

	ret = of_property_read_string_array(np, "target_list", (const char **)&name[0], target_num);
	if (ret < 0) {
		pr_err("[%s], get targetflow_array fail in dts,0x%x!\n", __func__, ret);
		kfree(targetflow_array);
		targetflow_array = NULL;
		return FAIL;
	}

	for (i = 0; i < target_num; i++)
		targetflow_array[i] = (unsigned long)name[i];

	g_noc_bus_info[busid].targetflow_array = (char **)targetflow_array;
	g_noc_bus_info[busid].targetflow_array_size = target_num;

	return SUCCESS;

}

/***************************************************************************
 * Function:       hisi_noc_get_routeid_addr_tbl
 * Description:    get static routeid dts info
 * Input:          busid
 *                 np
 * Output:         NA
 * Return:         success or fail
 ****************************************************************************/
static int hisi_noc_get_routeid_addr_tbl(unsigned int busid, struct device_node *np)
{
	int i, nums = 0;
	struct datapath_routid_addr *routeid_addr_tbl = NULL;
	const struct property *prop = NULL;
	const __be32 *val = NULL;

	if (of_property_read_u32(np, "routeid_nums", &nums) != 0) {
		pr_err("[%s] Get routeid_nums from DTS error.\n", __func__);
		return FAIL;
	}

	if (nums > MAX_ROUTEID_TBL_SIZE) {
		nums = MAX_ROUTEID_TBL_SIZE;
		pr_err("routeid_addr_tbl nums is too big, busid = [%d]\n", busid);
	}

	routeid_addr_tbl = kzalloc(sizeof(struct datapath_routid_addr) * nums, GFP_KERNEL);
	if (routeid_addr_tbl == NULL)
		return FAIL;

	prop = of_find_property(np, "routeid_list", NULL);
	if (!prop || !prop->value) {
		pr_err("No routeid_addr_tbl\n");
		goto error;
	}

	for (i = 0, val = prop->value; i < nums; i++) {
		routeid_addr_tbl[i].init_flow = (u32) (be32_to_cpup(val++));
		routeid_addr_tbl[i].targ_flow = (u32) (be32_to_cpup(val++));
		routeid_addr_tbl[i].targ_subrange = (u32) (be32_to_cpup(val++));
		routeid_addr_tbl[i].init_localaddr = (u64) (be32_to_cpup(val++));
		if ((routeid_addr_tbl[i].init_localaddr & NOC_ROUTEID_TBL_FLAG) == NOC_ROUTEID_TBL_FLAG)
			routeid_addr_tbl[i].init_localaddr = (routeid_addr_tbl[i].init_localaddr
				- NOC_ROUTEID_TBL_FLAG) << 1;
	}

	g_noc_bus_info[busid].routeid_tbl = routeid_addr_tbl;
	g_noc_bus_info[busid].routeid_tbl_size = nums;

	return SUCCESS;

error:

	kfree(routeid_addr_tbl);
	routeid_addr_tbl = NULL;

	return FAIL;
}

/***************************************************************************
 * Function:       hisi_noc_get_init_target_routeid_list
 * Description:    get initflow targetflow routeid_addr from dts
 * Input:          NA
 * Output:         NA
 * Return:         success or fail
 ***************************************************************************/
static int hisi_noc_get_init_target_routeid_list(void)
{
	unsigned int busid = 0;
	int ret = 0;
	struct device_node *np = NULL;

	for_each_compatible_node(np, NULL, "hisilicon,init-target-routeid-array") {

		if (of_property_read_u32(np, "bus_id", &busid) != 0) {
			pr_err("[%s] Get bus_id from DTS error.\n", __func__);
			continue;
		}
		if (busid >= MAX_BUSID_VALE)
			continue;

		ret = hisi_noc_get_routeid_addr_tbl(busid, np);
		if (ret < 0)
			pr_err("[%s], get routeid_addr_tbl fail in dts,0x%x!\n", __func__, ret);

		ret = hisi_noc_get_initflow_list(busid, np);
		if (ret < 0)
			pr_err("[%s], get initflow_array fail in dts,0x%x!\n", __func__, ret);

		ret = hisi_noc_get_targetflow_list(busid, np);
		if (ret < 0)
			pr_err("[%s], get targetflow_array fail in dts,0x%x!\n", __func__, ret);

	}

	return SUCCESS;

}

static int hisi_noc_get_mid_init(unsigned int *nums, struct property **prop)
{
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,noc-mid-info");
	if (!np) {
		pr_err("[%s], cannot find noc_mid_info node in dts!\n", __func__);
		return FAIL;
	}
	/* Get mid nums from DTS. */
	if (of_property_read_u32(np, "target_nums", nums) != 0) {
		pr_err("[%s] Get target_name_list_nums from DTS error.\n", __func__);
		return FAIL;
	}
	if (*nums > MAX_MID_NUM) {
		*nums = MAX_MID_NUM;
		pr_err("[%s] Get mid nums is too big from DTS.\n", __func__);
	}
	/* Get mid list from DTS. */
	*prop = of_find_property(np, "target_list", NULL);
	if (!*prop || !(*prop)->value) {
		pr_err("No npu_routeid_addr_tbl\n");
		return FAIL;
	}

	return SUCCESS;
}

/***************************************************************************
 * Function:       hisi_noc_get_mid_info
 * Description:    get mid info from dts
 * Input:          platform_device
 * Output:         NA
 * Return:         success or fail
 ***************************************************************************/
static int hisi_noc_get_mid_info(void)
{
	unsigned int i, nums = 0;
	struct noc_mid_info *mid_info = NULL;
	struct property *prop = NULL;
	const __be32 *val = NULL;
	unsigned int cnt = 0;
	int ret;

	ret = hisi_noc_get_mid_init(&nums, &prop);
	if (ret == FAIL) {
		pr_err("[%s] hisi_noc_get_mid_init error.\n", __func__);
		return FAIL;
	}

	if (!prop || !prop->value) {
		pr_err("mid prop is null\n");
		return FAIL;
	}

	mid_info = kzalloc(sizeof(struct noc_mid_info) * nums, GFP_KERNEL);
	if (mid_info == NULL)
		return FAIL;

	for (i = 0; i < MAX_BUSID_VALE; i++) {
		g_noc_bus_info[i].p_noc_mid_info = mid_info;
		g_noc_bus_info[i].noc_mid_info_size = nums;
	}

	for (i = 0, val = prop->value; i < nums; i++) {
		mid_info[i].idx = (u32) (be32_to_cpup(val++));
		mid_info[i].init_flow = (int)(be32_to_cpup(val++));
		mid_info[i].mask = (u32) (be32_to_cpup(val++));
		mid_info[i].mid_val = (u32) (be32_to_cpup(val++));
		mid_info[i].mid_name = kzalloc(MAX_NAME_LEN, GFP_KERNEL);

		if (mid_info[i].mid_name == NULL)
			return FAIL;

		ret = strncpy_s(mid_info[i].mid_name, MAX_NAME_LEN, (char *)val, (MAX_NAME_LEN - 1));
		if (ret != EOK) {
			pr_err("[%s:%d]strncpy_s ret : %d\n", __func__, __LINE__, ret);
			return FAIL;
		}

		cnt = strlen((const char *)val);
		val = (__be32 *)((char *)val + cnt + 1);
	}

	return SUCCESS;
}

/***************************************************************************
 * Function:       hisi_noc_get_dump_reg_offset
 * Description:    get dump reg offset from dts
 * Input:          platform_device
 * Output:         NA
 * Return:         success or fail
 ***************************************************************************/
static int hisi_noc_get_dump_reg_offset(void)
{
	unsigned int nums, i;
	int ret = 0;
	struct device_node *np = NULL;
	char *name[MAX_DUMP_REG];
	u32 data[MAX_DUMP_REG];

	np = of_find_compatible_node(NULL, NULL, "hisilicon,noc-dump-reg-offset");
	if (!np) {
		pr_err("[%s], cannot find noc-dump-list node in dts!\n", __func__);
		return FAIL;
	}

	/* Get reg offset nums from DTS. */
	if (of_property_read_u32(np, "reg_num", &nums) != 0) {
		pr_err("[%s] Get reg_list_nums from DTS error.\n", __func__);
		return FAIL;
	}
	if (nums > MAX_DUMP_REG) {
		nums = MAX_DUMP_REG;
		pr_err("noc dump reg offset nums is too big\n");
	}

	/* Get reg offset Form DTS. */
	if (of_property_read_u32_array(np, "reg_list", (u32 *) data, nums) != 0) {
		pr_err("[%s] Get get_dump_reg_offset list from DTS error.,num %d,%x\n", __func__, nums, data[0]);
		return FAIL;
	}

	/* Get reg names Form DTS. */
	ret = of_property_read_string_array(np, "reg_names", (const char **)&name[0], nums);
	if (ret < 0) {
		pr_err("[%s], get dump reg names fail in dts\n", __func__);
		return FAIL;
	}

	for (i = 0; i < nums; i++) {
		g_dump_reg_offset[i].name = kzalloc(MAX_NAME_LEN, GFP_KERNEL);
		if (g_dump_reg_offset[i].name == NULL)
			return FAIL;

		g_dump_reg_offset[i].offset = data[i];
		ret = strncpy_s(g_dump_reg_offset[i].name, MAX_NAME_LEN, name[i], (MAX_NAME_LEN - 1));
		if (ret != EOK) {
			pr_err("[%s:%d]strncpy_s ret : %d\n", __func__, __LINE__, ret);
			return FAIL;
		}
	}

	return SUCCESS;
}

static int hisi_noc_get_sec_init(unsigned int *nums, struct property **prop)
{
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,noc-sec-info");
	if (!np) {
		pr_err("[%s], cannot find noc_sec_info node in dts!\n", __func__);
		return FAIL;
	}

	/* Get sec nums from DTS. */
	if (of_property_read_u32(np, "target_nums", nums) != 0) {
		pr_err("[%s] Get sec info target nums from DTS error.\n", __func__);
		return FAIL;
	}
	if (*nums > MAX_SEC_MODE) {
		*nums = MAX_SEC_MODE;
		pr_err("noc sec info is too big\n");
	}

	/* Get sec lists from DTS. */
	*prop = of_find_property(np, "target_list", NULL);
	if (!*prop || !(*prop)->value) {
		pr_err("No sec info target lists\n");
		return FAIL;
	}

	return SUCCESS;
}

/***************************************************************************
 * Function:       hisi_noc_get_sec_info
 * Description:    get security operation property from dts
 * Input:          platform_device
 * Output:         NA
 * Return:         success or fail
 ***************************************************************************/
static int hisi_noc_get_sec_info(void)
{
	unsigned int nums, i;
	struct noc_sec_info *sec_info = NULL;
	struct property *prop = NULL;
	const __be32 *val = NULL;
	unsigned int cnt = 0;
	int ret;

	ret = hisi_noc_get_sec_init(&nums, &prop);
	if (ret == FAIL) {
		pr_err("[%s] hisi_noc_get_sec_init error.\n", __func__);
		return FAIL;
	}

	if (!prop || !prop->value) {
		pr_err("sec prop is null\n");
		return FAIL;
	}

	sec_info = kzalloc(sizeof(struct noc_sec_info) * nums, GFP_KERNEL);
	if (sec_info == NULL)
		return FAIL;

	for (i = 0; i < MAX_BUSID_VALE; i++) {
		g_noc_bus_info[i].p_noc_sec_info = sec_info;
		g_noc_bus_info[i].noc_sec_info_size = nums;
	}

	for (i = 0, val = prop->value; i < nums; i++) {
		sec_info[i].mask = (u32) (be32_to_cpup(val++));
		sec_info[i].sec_val = (u32) (be32_to_cpup(val++));

		sec_info[i].sec_array = kzalloc(MAX_NAME_LEN, GFP_KERNEL);
		if (sec_info[i].sec_array == NULL)
			return FAIL;

		ret = strncpy_s(sec_info[i].sec_array, MAX_NAME_LEN, (char *)val, (MAX_NAME_LEN - 1));
		if (ret != EOK) {
			pr_err("[%s:%d]strncpy_s ret : %d\n", __func__, __LINE__, ret);
			return FAIL;
		}

		cnt = strlen((const char *)val);
		val = (__be32 *)((char *)val + cnt + 1);

		sec_info[i].sec_mode = kzalloc(MAX_NAME_LEN, GFP_KERNEL);
		if (sec_info[i].sec_mode == NULL)
			return FAIL;

		ret = strncpy_s(sec_info[i].sec_mode, MAX_NAME_LEN, (char *)val, (MAX_NAME_LEN - 1));
		if (ret != EOK) {
			pr_err("[%s:%d]strncpy_s ret : %d\n", __func__, __LINE__, ret);
			return FAIL;
		}

		cnt = strlen((const char *)val);
		val = (__be32 *)((char *)val + cnt + 1);
	}

	return SUCCESS;
}

/***************************************************************************
 * Function:       hisi_noc_get_busid_initflow
 * Description:    get busid and initflow from dts
 * Input:          platform_device
 * Output:         NA
 * Return:         success or fail
 ***************************************************************************/
static int hisi_noc_get_busid_initflow(void)
{
	unsigned int nums, i;
	struct device_node *np = NULL;
	struct property *prop = NULL;
	const __be32 *val = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,noc-busid-initflow");
	if (!np) {
		pr_err("[%s], cannot find noc_busid_initflow node in dts!\n", __func__);
		return FAIL;
	}

	/* Get nums from DTS. */
	if (of_property_read_u32(np, "target_nums", &nums) != 0) {
		pr_err("[%s] Get noc_busid_initflow target nums from DTS error.\n", __func__);
		return FAIL;
	}

	prop = of_find_property(np, "target_list", NULL);
	if (!prop || !prop->value) {
		pr_err("No noc_busid_initflow target lists\n");
		return FAIL;
	}

	for (i = 0, val = prop->value; i < nums; i++) {
		g_busid_initflow[i].bus_id = (u32) (be32_to_cpup(val++));
		g_busid_initflow[i].init_flow = (int)(be32_to_cpup(val++));
		g_busid_initflow[i].coreid = (int)(be32_to_cpup(val++));
	}

	return SUCCESS;
}

static int __hisi_noc_get_bus_init(struct device_node *np, unsigned int *bus_id, char **name)
{
	int ret = 0;

	ret = of_property_read_string(np, "name", (const char **)name);
	if (ret < 0) {
		pr_debug("the node doesnot have name!\n");
		return FAIL;
	}

	ret = of_property_read_u32(np, "bus_id", bus_id);
	if (ret < 0) {
		pr_debug("the node doesnot have bus_id!\n");
		return FAIL;
	}
	if (*bus_id >= MAX_BUSID_VALE) {
		pr_debug("the busid is too big, busid = [%d]!\n", *bus_id);
		return FAIL;
	}

	ret = of_property_read_u32(np, "initflow_mask", &g_noc_bus_info[*bus_id].initflow_mask);
	if (ret < 0) {
		pr_debug("the node doesnot have initflow_mask!\n");
		return FAIL;
	}

	ret = of_property_read_u32(np, "targetflow_mask", &g_noc_bus_info[*bus_id].targetflow_mask);
	if (ret < 0) {
		pr_debug("the node doesnot have targetflow_mask!\n");
		return FAIL;
	}

	return SUCCESS;
}

/***************************************************************************
 * Function:       _hisi_noc_get_bus_info
 * Description:    get static businfo dts info
 * Input:          np
 * Output:         NA
 * Return:         success or fail
 **************************************************************************/
static int __hisi_noc_get_bus_info(struct device_node *np)
{
	int ret = 0;
	unsigned int bus_id = 0;
	char *name = NULL;

	ret = __hisi_noc_get_bus_init(np, &bus_id, &name);
	if (ret < 0) {
		pr_debug("_hisi_noc_get_bus_init fail!\n");
		return FAIL;
	}

	if (!name) {
		pr_debug("_hisi_noc_get_bus_name fail!\n");
		return FAIL;
	}

	ret = of_property_read_u32(np, "targ_subrange_mask", &g_noc_bus_info[bus_id].targ_subrange_mask);
	if (ret < 0) {
		pr_debug("the node doesnot have targ_subrange_mask!\n");
		return FAIL;
	}

	ret = of_property_read_u32(np, "seq_id_mask", &g_noc_bus_info[bus_id].seq_id_mask);
	if (ret < 0) {
		pr_debug("the node doesnot have seq_id_mask!\n");
		return FAIL;
	}

	ret = of_property_read_u32(np, "opc_mask", &g_noc_bus_info[bus_id].opc_mask);
	if (ret < 0) {
		pr_debug("the node doesnot have opc_mask!\n");
		return FAIL;
	}

	ret = of_property_read_u32(np, "err_code_mask", &g_noc_bus_info[bus_id].err_code_mask);
	if (ret < 0) {
		pr_debug("the node doesnot have err_code_mask!\n");
		return FAIL;
	}

	g_noc_bus_info[bus_id].name = kzalloc(MAX_NAME_LEN, GFP_KERNEL);
	if (g_noc_bus_info[bus_id].name == NULL)
		return FAIL;

	if (memcpy_s((void *)g_noc_bus_info[bus_id].name, MAX_NAME_LEN - 1, (void *)name, strlen(name)) != EOK) {
		pr_err("[%s] memcpy_s error.\n", __func__);
		return FAIL;
	}

	g_noc_bus_info[bus_id].opc_array = opc_array;
	g_noc_bus_info[bus_id].opc_array_size = OPC_NR;
	g_noc_bus_info[bus_id].err_code_array = err_code_array;
	g_noc_bus_info[bus_id].err_code_array_size = ERR_CODE_NR;

	return SUCCESS;
}

/***************************************************************************
 * Function:       hisi_noc_set_platform_info
 * Description:    get static platform dts info
 * Input:          np
 * Output:         NA
 * Return:         success or fail
 ***************************************************************************/
static int hisi_noc_set_platform_info(unsigned int platform_id)
{
	int ret = 0;

	ret = noc_get_platform_info_index(platform_id);
	if (ret < 0) {
		pr_err("[%s]: Error!! platform_id[%d], No platform id matched!!\n", __func__, platform_id);
		return FAIL;
	}

	g_noc_platform_info[ret].p_noc_info_bus = noc_get_tmp_bus_info();
	g_noc_platform_info[ret].p_noc_info_dump = noc_get_dump_reg_offset();
	g_noc_platform_info[ret].p_noc_info_filter_initflow = noc_get_busid_initflow();
	g_noc_platform_info[ret].pfun_get_size = hisi_noc_get_array_size;

	return SUCCESS;
}

/***************************************************************************
 * Function:       hisi_noc_get_bus_info
 * Description:    get static bus dts info
 * Input:          pdev
 * Output:         NA
 * Return:         success or fail
 ***************************************************************************/
static int hisi_noc_get_bus_info(const struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *np = NULL;
	const struct device *dev = &pdev->dev;

	np = dev->of_node;
	/* Get noc bus info from DTS. */
	for_each_compatible_node(np, NULL, "hisilicon,noc-bus-info") {
		ret = __hisi_noc_get_bus_info(np);
		if (ret != 0) {
			pr_err("[%s]hisi get bus info err\n", __func__);
			return FAIL;
		}
	}

	ret = hisi_noc_set_platform_info(noc_property_dt.platform_id);
	if (ret < 0) {
		pr_err("[%s]hisi_noc_set_platform_info return fail\n", __func__);
		return FAIL;
	}

	return SUCCESS;
}

/***************************************************************************
 * Function:       hisi_noc_get_data_from_dts
 * Description:    get static noc bus dts info
 * Input:          pdev
 * Output:         NA
 * Return:         success or fail
 ***************************************************************************/
int hisi_noc_get_data_from_dts(const struct platform_device *pdev)
{
	int ret = 0;

	ret = hisi_noc_get_init_target_routeid_list();
	if (ret != 0) {
		pr_err("hisi_noc_init_target_routeid_list err\n");
		goto err;
	}

	ret = hisi_noc_get_mid_info();
	if (ret != 0) {
		pr_err("hisi_mid_info_list err\n");
		goto err;
	}

	ret = hisi_noc_get_dump_reg_offset();
	if (ret != 0) {
		pr_err("hisi_noc_get_dump_reg_offset err\n");
		goto err;
	}

	ret = hisi_noc_get_sec_info();
	if (ret != 0) {
		pr_err("hisi_noc_get_sec_info err\n");
		goto err;
	}

	ret = hisi_noc_get_busid_initflow();
	if (ret != 0) {
		pr_err("hisi_noc_get_busid_initflow err\n");
		goto err;
	}

	ret = hisi_noc_get_bus_info(pdev);
	if (ret != 0) {
		pr_err("hisi get bus info err\n");
		goto err;
	}

	return SUCCESS;
err:
	free_noc_bus_source();

	return FAIL;
}

/*
 * if noc init fail ,free malloc source
 */
void free_noc_bus_source(void)
{
	int i ,sec_num, mid_num;
	struct noc_sec_info *sec_info = NULL;
	struct noc_mid_info *mid_info = NULL;

	sec_info = g_noc_bus_info[0].p_noc_sec_info;
	sec_num = g_noc_bus_info[0].noc_sec_info_size;
	mid_info = g_noc_bus_info[0].p_noc_mid_info;
	mid_num = g_noc_bus_info[0].noc_mid_info_size;

	if (sec_info) {
		for (i = 0; i < sec_num; i++) {
			kfree(sec_info[i].sec_array);
			kfree(sec_info[i].sec_mode);
		}
		kfree(sec_info);
	}

	if (mid_info) {
		for (i = 0; i < mid_num; i++)
			kfree(mid_info[i].mid_name);
		kfree(mid_info);
	}

	for (i = 0; i < MAX_DUMP_REG; i++) {
		kfree(g_dump_reg_offset[i].name);
		g_dump_reg_offset[i].name = NULL;
	}

	for (i = 0; i < MAX_BUSID_VALE; i++) {
		kfree(g_noc_bus_info[i].name);
		g_noc_bus_info[i].name = NULL;

		kfree(g_noc_bus_info[i].routeid_tbl);
		g_noc_bus_info[i].routeid_tbl = NULL;

		kfree(g_noc_bus_info[i].initflow_array);
		g_noc_bus_info[i].initflow_array = NULL;

		kfree(g_noc_bus_info[i].targetflow_array);
		g_noc_bus_info[i].targetflow_array = NULL;

		g_noc_bus_info[i].p_noc_sec_info = NULL;
		g_noc_bus_info[i].p_noc_mid_info = NULL;
	}
}
