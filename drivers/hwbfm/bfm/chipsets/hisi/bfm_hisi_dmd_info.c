/*
 * bfm_hisi_dmd_info.c
 *
 * get dmd info on hisi platform
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <log/log_usertype.h>
#include <chipset_common/bfmr/common/bfmr_common.h>

#define ERROR_NO "[Error no]:"

#define MAX_SIZE       256
#define ERR_NO_SIZE    32
#define ERR_OCCUR_TIME 3
#define ERR_NUM_SIZE   12
#define DECIMAL_SYSTEM 10

#define BETA_DMD_PATH_NEW "data/log/dmd_log/dmd_record_0.txt"
#define BETA_DMD_PATH_OLD "data/log/dmd_log/dmd_record_1.txt"
#define DMD_PATH_NEW      "splash2/dmd_log/dmd_record_0.txt"
#define DMD_PATH_OLD      "splash2/dmd_log/dmd_record_1.txt"

struct dmd_err_num {
	char mod_name[ERR_NO_SIZE];
	char err_num[ERR_NUM_SIZE];
};

struct dmd_err_info {
	struct dmd_err_num mod_info;
	uint32_t err_num;
	uint32_t err_count;
	struct list_head list;
};

static struct dmd_err_num dmd_info_table[] = {
	{ "BATTERY", "920001003" },
	{ "BATTERY", "920001004" },
	{ "BATTERY", "920001010" },
	{ "BATTERY", "920001016" },
	{ "BATTERY", "920001017" },
	{ "BATTERY", "920001018" },
	{ "BATTERY", "920001019" },
	{ "SMPL", "920003000" },
	{ "USCP", "920004000" },
	{ "USCP", "920004001" },
	{ "PMU_IRQ", "920005000" },
	{ "PMU_IRQ", "920005001" },
	{ "PMU_IRQ", "920005002" },
	{ "PMU_IRQ", "920005003" },
	{ "PMU_IRQ", "920005004" },
	{ "PMU_IRQ", "920005005" },
	{ "PMU_IRQ", "920005006" },
	{ "PMU_IRQ", "920005007" },
	{ "PMU_IRQ", "920005008" },
	{ "PMU_IRQ", "920005011" },
	{ "PMU_IRQ", "920005012" },
	{ "PMU_IRQ", "920005013" },
	{ "PMU_COUL", "920006000" },
	{ "PMU_OCP", "920007000" },
	{ "PMU_OCP", "920007001" },
	{ "PMU_OCP", "920007002" },
	{ "PMU_OCP", "920007003" },
	{ "PMU_OCP", "920007004" },
	{ "PMU_OCP", "920007005" },
	{ "PMU_OCP", "920007006" },
	{ "PMU_OCP", "920007007" },
	{ "PMU_OCP", "920007008" },
	{ "PMU_OCP", "920007009" },
	{ "PMU_OCP", "920007010" },
	{ "PMU_OCP", "920007011" },
	{ "PMU_OCP", "920007012" },
	{ "PMU_OCP", "920007013" },
	{ "PMU_OCP", "920007014" },
	{ "PMU_OCP", "920007015" },
	{ "PMU_OCP", "920007016" },
	{ "PMU_OCP", "920007017" },
	{ "PMU_OCP", "920007018" },
	{ "PMU_OCP", "920007019" },
	{ "PMU_OCP", "920007020" },
	{ "PMU_OCP", "920007021" },
	{ "PMU_OCP", "920007022" },
	{ "PMU_OCP", "920007023" },
	{ "PMU_OCP", "920007024" },
	{ "PMU_OCP", "920007025" },
	{ "PMU_OCP", "920007026" },
	{ "PMU_OCP", "920007027" },
	{ "PMU_OCP", "920007028" },
	{ "PMU_OCP", "920007029" },
	{ "PMU_OCP", "920007030" },
	{ "PMU_OCP", "920007031" },
	{ "PMU_OCP", "920007032" },
	{ "PMU_OCP", "920007033" },
	{ "PMU_OCP", "920007034" },
	{ "PMU_OCP", "920007035" },
	{ "PMU_OCP", "920007036" },
	{ "PMU_OCP", "920007037" },
	{ "PMU_OCP", "920007038" },
	{ "PMU_OCP", "920007039" },
	{ "PMU_OCP", "920007040" },
	{ "PMU_OCP", "920007041" },
	{ "PMU_OCP", "920007042" },
	{ "PMU_OCP", "920007043" },
	{ "PMU_OCP", "920007044" },
	{ "PMU_OCP", "920007045" },
	{ "PMU_OCP", "920007046" },
	{ "PMU_OCP", "920007047" },
	{ "PMU_OCP", "920007048" },
	{ "PMU_OCP", "920007049" },
	{ "CHARGE_MONITOR", "920008000" },
	{ "CHARGE_MONITOR", "920008008" },
	{ "LCD", "922001001" },
	{ "LCD", "922001002" },
	{ "LCD", "922001003" },
	{ "LCD", "922001005" },
	{ "LCD", "922001007" },
	{ "LCD", "922001008" },
	{ "KEY", "926003000" },
	{ "KEY", "926003001" },
	{ "EMMC", "928002001" },
	{ "EMMC", "928002002" },
	{ "EMMC", "928002004" },
	{ "EMMC", "928002005" },
	{ "EMMC", "928002006" },
	{ "EMMC", "928002007" },
	{ "EMMC", "928002008" },
	{ "EMMC", "928002009" },
	{ "EMMC", "928002010" },
	{ "EMMC", "928002011" },
	{ "EMMC", "928002012" },
	{ "EMMC", "928002013" },
	{ "EMMC", "928002014" },
	{ "EMMC", "928002015" },
	{ "EMMC", "928002020" },
	{ "EMMC", "928002021" },
	{ "EMMC", "928002022" },
	{ "EMMC", "928002023" },
	{ "EMMC", "928002025" }
};

static int find_index_by_err_no(const char *err_num, int max_len)
{
	int i;
	int max_lenth = sizeof(dmd_info_table) / sizeof(struct dmd_err_num);

	if (!err_num || (max_lenth > max_len))
		return -1;

	for (i = 0; i < max_lenth; i++) {
		if (strncmp(dmd_info_table[i].err_num, err_num,
			    strlen(dmd_info_table[i].err_num)) == 0)
			return i;
	}
	return -1;
}

static int get_the_max_err_info(struct list_head *list_head,
		struct dmd_err_info *max_err_info)
{
	struct dmd_err_info *cur_info = NULL;
	struct dmd_err_info *next_info = NULL;
	uint32_t maxCount = 0;

	/* find error node in the list, with the max count. */
	if (!max_err_info || !list_head)
		return -1;

	if (list_head->next == list_head)
		return -1;

	list_for_each_entry_safe(cur_info, next_info, list_head, list) {
		int index;
		int max_lenth = sizeof(dmd_info_table) /
			sizeof(struct dmd_err_num);

		if (!cur_info)
			break;

		index = find_index_by_err_no(cur_info->mod_info.err_num,
				max_lenth);
		if (index != -1) {
			if (cur_info->err_count > maxCount) {
				strncpy(max_err_info->mod_info.mod_name,
					dmd_info_table[index].mod_name,
					ERR_NO_SIZE - 1);
				max_err_info->err_num = cur_info->err_num;
				max_err_info->err_count = cur_info->err_count;
				maxCount = cur_info->err_count;
			}
		}
		list_del(&cur_info->list);
		kfree(cur_info);
	}
	return 0;
}

static void create_new_list_node(const char *err_num,
		struct list_head *list_head)
{
	struct dmd_err_info *dmd_info = NULL;
	int rnum;
	unsigned long tmp = 0;

	if (!err_num || !list_head)
		return;

	dmd_info = kmalloc(sizeof(struct dmd_err_info), GFP_KERNEL);
	if (!dmd_info)
		return;

	memset(dmd_info, 0, sizeof(struct dmd_err_info));
	strncpy(dmd_info->mod_info.err_num, err_num, ERR_NUM_SIZE - 1);
	rnum = kstrtoul(dmd_info->mod_info.err_num, DECIMAL_SYSTEM, &tmp);
	if (rnum != 0)
		printk("the str to unsinged long is err");
	dmd_info->err_num = (uint32_t)tmp;
	dmd_info->err_count = 1;
	list_add(&dmd_info->list, list_head);
}

static void check_list_for_each(const char *err_num, int char_len,
				struct list_head *list_head)
{
	int is_exist = 0;

	if (!err_num || !list_head || (char_len < ERR_NO_SIZE))
		return;

	if (list_empty(list_head)) {
		create_new_list_node(err_num, list_head);
	} else {
		struct dmd_err_info *cur_err_info = NULL;

		list_for_each_entry(cur_err_info, list_head, list) {
			if (!strncmp(cur_err_info->mod_info.err_num, err_num,
				strlen(cur_err_info->mod_info.err_num))) {
				cur_err_info->err_count++;
				is_exist = 1;
				break;
			}
		}

		if (!is_exist)
			create_new_list_node(err_num, list_head);
	}
}

static int read_line(struct file *fp, char *buffer, int buf_len, loff_t *ppos)
{
	ssize_t ret;
	char buff[MAX_SIZE] = { 0 };
	loff_t pos = *ppos;
	int read_lenth;
	char *pstr = NULL;

	if (!fp || !buffer || !ppos)
		return -1;

	ret = vfs_read(fp, buff, MAX_SIZE - 1, &pos);
	if (ret <= 0)
		return -1;

	read_lenth = (int)(pos - *ppos);
	pstr = strchr(buff, '\n');
	if (!pstr) {
		*ppos = pos;
		return 1;
	}

	*pstr = 0;
	strncpy(buffer, buff, MAX_SIZE - 1);
	*ppos = pos - (read_lenth - strlen(buffer) - 1);
	return 0;
}

static void count_err_num(struct file *fp, struct list_head *list_head)
{
	mm_segment_t fs;
	loff_t pos = 0;
	char buff[MAX_SIZE] = { 0 };
	char *p = NULL;
	int ret;

	if(!fp || !list_head)
		return;

	fs = get_fs();
	set_fs(KERNEL_DS);

	while (1) {
		ret = read_line(fp, buff, MAX_SIZE, &pos);

		if (ret == -1)
			break;
		else if (ret == 1)
			continue;

		p = strstr(buff, ERROR_NO);
		if (p) {
			char err_num[ERR_NO_SIZE] = { 0 };

			strncpy(err_num, p + strlen(ERROR_NO), ERR_NO_SIZE - 1);
			check_list_for_each(err_num, ERR_NO_SIZE, list_head);
		}
	}

	set_fs(fs);
}

static int count_dmd_file(struct list_head *list_head)
{
	struct file *fp_new = NULL;
	struct file *fp_old = NULL;
	char *path_new = NULL;
	char *path_old = NULL;
	unsigned int type = get_logusertype_flag();

	if(!list_head)
		return -1;

	if (type == BETA_USER) {
		path_new = BETA_DMD_PATH_NEW;
		path_old = BETA_DMD_PATH_OLD;
	} else {
		path_new = DMD_PATH_NEW;
		path_old = DMD_PATH_OLD;
	}

	fp_new = filp_open(path_new, O_RDONLY, 0);
	if (IS_ERR(fp_new))
		return -1;

	count_err_num(fp_new, list_head);
	filp_close(fp_new, NULL);

	fp_old = filp_open(path_old, O_RDONLY, 0);
	if (!IS_ERR(fp_old)) {
		count_err_num(fp_old, list_head);
		filp_close(fp_old, NULL);
	}
	return 0;
}

/*
 * Function:       get_dmd_err_num
 * Description:    get the most err number from DMD file about boot fail
 * Input:           NA
 * Output:         errNumber     the dmd err number
 *                 count            the times of err occur
 *                 errName       the mod name of this err
 * Return:         0:   find adapt err number
 *                -1: not find
 */
int get_dmd_err_num(unsigned int *err_num,
		unsigned int *p_count, char *err_name)
{
	struct list_head list_head;
	struct dmd_err_info err_info;

	if (!err_num || !p_count || !err_name) {
		BFMR_PRINT_ERR("%s: param is invalid\n", __func__);
		return -1;
	}

	INIT_LIST_HEAD(&list_head);

	if (count_dmd_file(&list_head) == -1) {
		BFMR_PRINT_ERR("%s: count_dmd_file failed\n", __func__);
		return -1;
	}

	memset(&err_info, 0, sizeof(err_info));
	if (get_the_max_err_info(&list_head, &err_info) == -1) {
		BFMR_PRINT_ERR("%s: get_the_max_err_info failed\n", __func__);
		return -1;
	}

	if (err_info.err_count < ERR_OCCUR_TIME) {
		BFMR_PRINT_ERR("%s: err_info.err_count  failed\n", __func__);
		return -1;
	}
	*err_num = err_info.err_num;
	*p_count = err_info.err_count;
	strncpy(err_name, err_info.mod_info.mod_name, ERR_NO_SIZE - 1);
	BFMR_PRINT_ERR("%s: err_num:%u, err_count:%u, err_name:%s\n",
		       __func__, err_num, p_count, err_name);

	return 0;
}
