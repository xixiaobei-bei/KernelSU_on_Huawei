/*
 * rdr_field_core.c
 *
 * blackbox. (kernel run data recorder.)
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

#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/io.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/of.h>

#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/hisi_log.h>

#include "rdr_inner.h"
#include "rdr_print.h"
#include "rdr_field.h"
#include <securec.h>

#define HISI_LOG_TAG HISI_BLACKBOX_TAG

static struct rdr_struct_s *g_pbb = NULL;
static struct rdr_struct_s *g_tmp_pbb = NULL;

u64 rdr_area_mem_addr[RDR_AREA_MAXIMUM + 1];
u32 rdr_area_mem_size[RDR_AREA_MAXIMUM] = {
	0, /* 6, the num of core to allocate memory */
	0, /* 0x180000, CP */
	0, /* 0x20000, TEE */
	0, /* 0x20000, HIFI */
	0, /* 0x20000, LPM3 */
	0, /* 0x20000, IOM3 */
};

void rdr_set_area_info(int index, u32 size)
{
	rdr_area_mem_size[index] = size;
}

struct rdr_struct_s *rdr_get_pbb(void)
{
	return g_pbb;
}

struct rdr_struct_s *rdr_get_tmppbb(void)
{
	return g_tmp_pbb;
}

void rdr_clear_tmppbb(void)
{
	if (g_tmp_pbb != NULL) {
		vfree(g_tmp_pbb);
		g_tmp_pbb = NULL;
	}
}

u64 rdr_get_pbb_size(void)
{
	return rdr_reserved_phymem_size();
}

int rdr_get_areainfo(enum RDR_AREA_LIST area,
				struct rdr_register_module_result *retinfo)
{
	if ((area >= RDR_AREA_MAXIMUM) || (retinfo == NULL))
		return -1;

	retinfo->log_addr = g_pbb->area_info[area].offset;
	retinfo->log_len = g_pbb->area_info[area].length;

	return 0;
}

static void rdr_field_baseinfo_init(void)
{
	BB_PRINT_START();
	g_pbb->base_info.modid = 0;
	g_pbb->base_info.arg1 = 0;
	g_pbb->base_info.arg2 = 0;
	g_pbb->base_info.e_core = 0;
	g_pbb->base_info.e_type = 0;
	g_pbb->base_info.e_subtype = 0;
	g_pbb->base_info.start_flag = 0;
	g_pbb->base_info.savefile_flag = 0;
	g_pbb->base_info.reboot_flag = 0;
	(void)memset_s(g_pbb->base_info.e_module, MODULE_NAME_LEN, 0, MODULE_NAME_LEN);

	memset(g_pbb->base_info.e_desc, 0, STR_EXCEPTIONDESC_MAXLEN);
	memset(g_pbb->base_info.datetime, 0, DATATIME_MAXLEN);

	g_pbb->cleartext_info.savefile_flag = 0;

	BB_PRINT_END();
}

void rdr_field_baseinfo_reinit(void)
{
	BB_PRINT_START();
	g_pbb->base_info.modid = 0;
	g_pbb->base_info.arg1 = 0;
	g_pbb->base_info.arg2 = 0;
	g_pbb->base_info.e_core = 0;
	g_pbb->base_info.e_type = 0;
	g_pbb->base_info.e_subtype = 0;
	g_pbb->base_info.start_flag = RDR_PROC_EXEC_START;
	g_pbb->base_info.savefile_flag = RDR_DUMP_LOG_START;

	(void)memset_s(g_pbb->base_info.datetime, DATATIME_MAXLEN, 0, DATATIME_MAXLEN);

	g_pbb->cleartext_info.savefile_flag = 0;

	BB_PRINT_END();
}

static void rdr_field_areainfo_init(void)
{
	int index;

	for (index = 0; index < RDR_AREA_MAXIMUM; index++) {
		g_pbb->area_info[index].offset = rdr_area_mem_addr[index];
		g_pbb->area_info[index].length = rdr_area_mem_size[index];
	}
}

char *rdr_field_get_datetime(void)
{
	return (char *)(g_pbb->base_info.datetime);
}

void rdr_cleartext_dumplog_done(void)
{
	g_pbb->cleartext_info.savefile_flag = 1;
}

void rdr_field_dumplog_done(void)
{
	g_pbb->base_info.savefile_flag = RDR_DUMP_LOG_DONE;
}

void rdr_field_procexec_done(void)
{
	g_pbb->base_info.start_flag = RDR_PROC_EXEC_DONE;
}

void rdr_field_reboot_done(void)
{
	g_pbb->base_info.reboot_flag = RDR_REBOOT_DONE;
}

static void rdr_field_top_init(void)
{
	int length;

	BB_PRINT_START();

	g_pbb->top_head.magic = FILE_MAGIC;
	g_pbb->top_head.version = RDR_VERSION;
	g_pbb->top_head.area_number = RDR_AREA_MAXIMUM;

	rdr_get_builddatetime(g_pbb->top_head.build_time, RDR_BUILD_DATE_TIME_LEN);
	length = strlen(RDR_PRODUCT) > RDR_PRODUCT_MAXLEN ? RDR_PRODUCT_MAXLEN : strlen(RDR_PRODUCT);
	if (memcpy_s(g_pbb->top_head.product_name, RDR_PRODUCT_MAXLEN, RDR_PRODUCT, length) != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
		BB_PRINT_END();
		return;
	}

	length = strlen(RDR_PRODUCT_VERSION) >
		RDR_PRODUCT_MAXLEN ? RDR_PRODUCT_MAXLEN : strlen(RDR_PRODUCT_VERSION);
	if (memcpy_s(g_pbb->top_head.product_version, RDR_PRODUCT_MAXLEN,
		RDR_PRODUCT_VERSION, length) != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
		BB_PRINT_END();
		return;
	}

	BB_PRINT_END();
}

int rdr_field_init(void)
{
	int ret;
	int index;
	u32 last;
	unsigned int fpga_flag = 0;
	struct device_node *np = NULL;

	BB_PRINT_START();

	g_pbb = hisi_bbox_map(rdr_reserved_phymem_addr(),
			rdr_reserved_phymem_size());
	if (g_pbb == NULL) {
		BB_PRINT_ERR("hisi_bbox_map g_pbb faild\n");
		ret = -1;
		goto out;
	}

	g_tmp_pbb = vmalloc(rdr_reserved_phymem_size());
	if (g_tmp_pbb == NULL) {
		BB_PRINT_ERR("vmalloc g_tmp_pbb faild\n");
		ret = -1;
		hisi_bbox_unmap(g_pbb);
		g_pbb = NULL;
		goto out;
	}

	rdr_show_base_info(1); /* show g_pbb */
	if (memcpy_s(g_tmp_pbb, rdr_reserved_phymem_size(), g_pbb, rdr_reserved_phymem_size()) != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
		ret = -1;
		goto free_pbb;
	}

	rdr_show_base_info(0); /* show g_tmp_pbb */

	np = of_find_compatible_node(NULL, NULL, "hisilicon,hisifb");
	if (np == NULL) {
		BB_PRINT_ERR("NOT FOUND device node 'hisilicon,hisifb'!\n");
		ret = -ENXIO;
		goto free_pbb;
	}
	ret = of_property_read_u32(np, "fpga_flag", &fpga_flag);
	if (ret) {
		BB_PRINT_ERR("failed to get fpga_flag resource\n");
		ret = -ENXIO;
		goto free_pbb;
	}
	/*
	 * if the power_up of phone is the first time,
	 * need clear bbox memory.
	 */
	if ((rdr_get_reboot_type() == AP_S_COLDBOOT) && (fpga_flag != 1)) {
		if (memset_s(g_pbb, rdr_reserved_phymem_size(), 0, rdr_reserved_phymem_size()) != EOK) {
			BB_PRINT_ERR("%s():%d:memset_s fail!\n", __func__, __LINE__);
			ret = -1;
			goto free_pbb;
		}
	} else {
		(void)memset_s(g_pbb, RDR_BASEINFO_SIZE, 0, RDR_BASEINFO_SIZE);
	}

	last = rdr_area_mem_size[0];
	rdr_area_mem_addr[last] = rdr_reserved_phymem_addr() + rdr_reserved_phymem_size();
	for (index = last - 1; index > 0; index--)
		rdr_area_mem_addr[index] = rdr_area_mem_addr[index + 1] - rdr_area_mem_size[index];

	rdr_area_mem_addr[0] = rdr_reserved_phymem_addr() + RDR_BASEINFO_SIZE;
	rdr_area_mem_size[0] = rdr_area_mem_addr[1] - RDR_BASEINFO_SIZE - rdr_reserved_phymem_addr();

	rdr_field_top_init();
	rdr_field_baseinfo_init();
	rdr_field_areainfo_init();
	BB_PRINT_END();
out:
	return ret;
free_pbb:
	hisi_bbox_unmap(g_pbb);
	g_pbb = NULL;
	vfree(g_tmp_pbb);
	g_tmp_pbb = NULL;
	return ret;
}

void rdr_field_exit(void)
{
}

void rdr_save_args(u32 modid, u32 arg1, u32 arg2)
{
	BB_PRINT_START();
	g_pbb->base_info.modid = modid;
	g_pbb->base_info.arg1 = arg1;
	g_pbb->base_info.arg2 = arg2;

	BB_PRINT_END();
}

void rdr_fill_edata(struct rdr_exception_info_s *e, const char *date)
{
	BB_PRINT_START();
	if ((e == NULL) || (date == NULL)) {
		BB_PRINT_ERR("%s():%d:invalid  parameter!\n", __func__, __LINE__);
		BB_PRINT_END();
		return;
	}

	g_pbb->base_info.e_core = e->e_from_core;
	g_pbb->base_info.e_type = e->e_exce_type;
	g_pbb->base_info.e_subtype = e->e_exce_subtype;
	if (memcpy_s(g_pbb->base_info.datetime, DATATIME_MAXLEN, date, DATATIME_MAXLEN) != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
		return;
	}

	memcpy(g_pbb->base_info.e_module, e->e_from_module, MODULE_NAME_LEN);
	memcpy(g_pbb->base_info.e_desc, e->e_desc, STR_EXCEPTIONDESC_MAXLEN);
	BB_PRINT_END();
}

void rdr_show_base_info(int flag)
{
	struct rdr_struct_s *p = NULL;
	int index;

	if (flag == 1)
		p = rdr_get_pbb();
	else
		p = rdr_get_tmppbb();

	if (p == NULL)
		return;

	if (p->top_head.magic != FILE_MAGIC) {
		BB_PRINT_PN("[%s]: rdr_struct_s information is not initialized, no need to print it's content!\n",
			__func__);
		return;
	}

	p->base_info.datetime[DATATIME_MAXLEN - 1] = '\0';
	p->base_info.e_module[MODULE_NAME_LEN - 1] = '\0';
	p->base_info.e_desc[STR_EXCEPTIONDESC_MAXLEN - 1] = '\0';
	p->top_head.build_time[RDR_BUILD_DATE_TIME_LEN - 1] = '\0';

	BB_PRINT_PN("========= print baseinfo start =========\n");
	BB_PRINT_PN("modid        :[0x%x]\n", p->base_info.modid);
	BB_PRINT_PN("arg1         :[0x%x]\n", p->base_info.arg1);
	BB_PRINT_PN("arg2         :[0x%x]\n", p->base_info.arg2);
	BB_PRINT_PN("coreid       :[0x%x]\n", p->base_info.e_core);
	BB_PRINT_PN("reason       :[0x%x]\n", p->base_info.e_type);
	BB_PRINT_PN("subtype      :[0x%x]\n", p->base_info.e_subtype);
	BB_PRINT_PN("e data       :[%s]\n", p->base_info.datetime);
	BB_PRINT_PN("e module     :[%s]\n", p->base_info.e_module);
	BB_PRINT_PN("e desc       :[%s]\n", p->base_info.e_desc);
	BB_PRINT_PN("e start_flag :[%u]\n", p->base_info.start_flag);
	BB_PRINT_PN("e save_flag  :[%u]\n", p->base_info.savefile_flag);
	BB_PRINT_PN("========= print baseinfo e n d =========\n");

	BB_PRINT_PN("========= print top head start =========\n");
	BB_PRINT_PN("maigc        :[0x%x]\n", p->top_head.magic);
	BB_PRINT_PN("version      :[0x%x]\n", p->top_head.version);
	BB_PRINT_PN("area num     :[0x%x]\n", p->top_head.area_number);
	BB_PRINT_PN("reserve      :[0x%x]\n", p->top_head.reserve);
	BB_PRINT_PN("buildtime    :[%s]\n", p->top_head.build_time);
	BB_PRINT_PN("========= print top head e n d =========\n");

	BB_PRINT_PN("========= print areainfo start =========\n");
	for (index = 0; index < RDR_AREA_MAXIMUM; index++)
		BB_PRINT_PN("area[%d] addr[0x%llx] size[0x%x]\n",
			index, g_pbb->area_info[index].offset, g_pbb->area_info[index].length);

	BB_PRINT_PN("========= print areainfo e n d =========\n");

	BB_PRINT_PN("========= print clear text start =========\n");
	BB_PRINT_PN("savefile_flag:[0x%x]\n", p->cleartext_info.savefile_flag);
	BB_PRINT_PN("========= print clear text e n d =========\n");
}
