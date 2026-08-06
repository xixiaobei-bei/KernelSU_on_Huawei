/*
 * batt_info.c
 *
 * battery authenticate information
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

#define BATTCT_KERNEL_SRV_SHARE_VAR
#include "batt_info.h"
#include "batt_uuid_binder.h"
#include "batt_early_param.h"
#include <linux/delay.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG batt_info
HWLOG_REGIST();

#define DMD_CONTENT_BASE_LEN 24
#define DMD_REPORT_CONTENT_LEN 256
#define CHECKING_WAKELOCK_TIME_IN_SECOND 10
#define RETRY_TIMES 5

static LIST_HEAD(batt_checkers_head);
static unsigned int valid_checkers;
static unsigned int g_total_checkers;
static unsigned int shield_ct_sign;
static struct dmd_record_list g_dmd_list;
#ifdef BATTERY_LIMIT_DEBUG
static struct timespec64 finish_time;
#endif

static enum powerct_error_code g_error_code;
static bool g_is_sn_empty;
static int prepare_dmd_no(struct batt_chk_rslt *result);
static void dmd_record_reporter(struct work_struct *work);
static int get_dmd_no(unsigned int index);
static const char *dmd_no_to_str(unsigned int dmd_no);
static int g_sn_change_onboot = -EBUSY;

void add_to_batt_checkers_lists(struct batt_checker_entry *entry)
{
	list_add(&entry->node, &batt_checkers_head);
	valid_checkers++;
}

/* A function to implement bubble sort */
void bubble_sort(char arr[], int n)
{
	int i, j;
	char temp;

	for (i = 0; i < n - 1; i++) {
		for (j = 0; j < n - i - 1; j++) {
			if (arr[j] > arr[j + 1]) {
				temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;
			}
		}
	}
}

int get_battery_type(unsigned char *name, unsigned int name_size)
{
	struct batt_checker_entry *temp = NULL;
	struct platform_device *pdev = NULL;
	struct batt_chk_data *checker_data = NULL;
	const unsigned char *batt_type = NULL;
	unsigned int len = 0;

	list_for_each_entry(temp, &batt_checkers_head, node) {
		pdev = temp->pdev;
		checker_data = platform_get_drvdata(pdev);
		if (checker_data->bco.get_batt_type) {
			if (checker_data->bco.get_batt_type(checker_data,
				&batt_type, &len)) {
				hwlog_err("Get battery type error in %s\n",
					__func__);
				return -1;
			}
			if (len >= name_size) {
				hwlog_err("buffer is too small found in %s\n",
					__func__);
				return -1;
			}
			memcpy(name, batt_type, len);
			name[len] = 0;
			hwlog_info("battery type is %s\n", name);
			return 0;
		}
		return -1;
	}

	return -1;
}

static int compare_battery_sn(struct batt_chk_data *checker_data)
{
	const unsigned char *sn = NULL;
	unsigned char *nv_sn = (unsigned char *)batt_get_nv_sn();
	struct binded_info bbinfo;
	unsigned int sn_len;

	if (!nv_sn) {
		if (power_nv_read(POWER_NV_BBINFO, &bbinfo,
			sizeof(struct binded_info))) {
			hwlog_err("[%s]: bbinfo read failed\n", __func__);
			return -1;
		}
		nv_sn = (unsigned char *)&bbinfo.info[0];
	}

	if (checker_data->bco.get_batt_sn(checker_data, 0, &sn, &sn_len)) {
		hwlog_err("get battery sn failed in %s\n",
			__func__);
		return -1;
	}

	if (sn_len < MAX_RAW_SN_LEN) {
		hwlog_err("battery sn len %u error in %s\n",
			sn_len, __func__);
		return -1;
	}

	if (memcmp(nv_sn, sn, MAX_RAW_SN_LEN)) {
		hwlog_err("battery sn is not same with nv in %s\n",
			__func__);
		return 1;
	}

	return 0;
}

int check_battery_sn_changed(void)
{
	struct batt_checker_entry *temp = NULL;
	struct platform_device *pdev = NULL;
	struct batt_chk_data *checker_data = NULL;
	int same_nums = 0;
	int ret;

	if (g_total_checkers == 0)
		return -1;

	list_for_each_entry(temp, &batt_checkers_head, node) {
		pdev = temp->pdev;
		checker_data = platform_get_drvdata(pdev);
		if (!checker_data ||
			!checker_data->bco.get_ic_type ||
			!checker_data->bco.get_batt_sn)
			continue;

		ret = compare_battery_sn(checker_data);
		if (ret)
			return ret;

		same_nums++;
	}

	if (same_nums != g_total_checkers) {
		hwlog_err("battery checkers number is wrong %d:%d\n",
			same_nums, g_total_checkers);
		return -1;
	}

	return 0;
}

int get_batt_changed_on_boot(void)
{
	if (g_sn_change_onboot != -EBUSY)
		return g_sn_change_onboot;
	if (valid_checkers == g_total_checkers)
		g_sn_change_onboot = check_battery_sn_changed();
	return g_sn_change_onboot;
}

enum phone_work_mode_t {
	NORMAL_MODE = 0,
	CHARGER_MODE,
	RECOVERY_MODE,
	ERECOVERY_MODE,
	MODE_UNKOWN,
};

static const char * const work_mode_str[] = {
	[NORMAL_MODE] = "NORMAL",
	[CHARGER_MODE] = "CHARGER",
	[RECOVERY_MODE] = "RECOVERY",
	[ERECOVERY_MODE] = "ERECOVERY",
};

static enum phone_work_mode_t work_mode = MODE_UNKOWN;

static void update_work_mode(void)
{
	if (work_mode != MODE_UNKOWN)
		return;
	if (power_cmdline_is_powerdown_charging_mode()) {
		work_mode = CHARGER_MODE;
		goto print_work_mode;
	}
	if (power_cmdline_is_erecovery_mode()) {
		work_mode = ERECOVERY_MODE;
		goto print_work_mode;
	}
	if (power_cmdline_is_recovery_mode()) {
		work_mode = RECOVERY_MODE;
		goto print_work_mode;
	}
	work_mode = NORMAL_MODE;

print_work_mode:
	hwlog_info("work mode is %s\n", work_mode_str[work_mode]);
}

static struct completion ct_srv_ready;

static struct platform_device *get_checker_pdev(struct nl_dev_info *dev_info)
{
	struct batt_checker_entry *temp = NULL;
	struct platform_device *pdev = NULL;
	struct batt_chk_data *checker_data = NULL;

	list_for_each_entry(temp, &batt_checkers_head, node) {
		pdev = temp->pdev;
		checker_data = platform_get_drvdata(pdev);
		if ((dev_info->id_in_grp == checker_data->id_in_grp) &&
			(dev_info->id_of_grp == checker_data->id_of_grp) &&
			(dev_info->ic_type == checker_data->ic_type))
			return pdev;
	}

	return NULL;
}

/*
 * netlink max support data 2^16-1-4 Bytes in one attribute,
 * we limit it one page size here
 */
#define MAX_DATA_LEN    PAGE_SIZE
static void batt_info_mesg_data_processor(struct platform_device *pdev,
	unsigned char subcmd, const void *data, int len)
{
	int act_result = SIGN_FAIL;
	struct batt_chk_data *checker_data = NULL;
	struct completion *comp = NULL;
	struct power_genl_attr *res = NULL;
	unsigned char *mesg_data = NULL;

	checker_data = platform_get_drvdata(pdev);
	switch (subcmd) {
	case CT_PREPARE:
		comp = &checker_data->key_prepare_ready;
		res = &checker_data->key_res;
		break;
	case SN_PREPARE:
		comp = &checker_data->sn_prepare_ready;
		res = &checker_data->sn_res;
		break;
	case ACT_SIGN:
		if (len != sizeof(int))
			return;
		memcpy(&act_result, data, len);
		checker_data->act_state = (enum batt_act_state)act_result;
		hwlog_info("mesg %c result %d for battery checker\n",
			subcmd, act_result);
		complete(&checker_data->act_sign_complete);
		return;
	default:
		hwlog_err("unkown subcmd for checker(NO:%d Group:%d)\n",
			checker_data->id_in_grp, checker_data->id_of_grp);
		return;
	}
	/* try to free last res first */
	kfree(res->data);
	res->data = NULL;
	res->len = 0;
	if ((len <= MAX_DATA_LEN) && (len > 0)) {
		mesg_data = kmalloc(len, GFP_KERNEL);
		if (mesg_data == NULL)
			return;
		memcpy(mesg_data, data, len);
		res->data = mesg_data;
		res->len = len;
		complete(comp);
	} else if (len == 0) {
		/*
		 * If subcmd is SN_PREPARE, it's just for nxp A1005. That ic
		 * will be discarded, and no more maintenance is required.
		 * This does not affect original logic.
		 * If subcmd is ACT_SIGN, the program will not run to here.
		 *
		 * This is for prohibited id, after setting (data=NULL, len=0)
		 * in powerct and complete here, the program will think it is
		 * fail_ic error, and take action.
		 *
		 * In other words, processing prohibited id uses the way
		 * that belongs to other fail, but they are all failed,
		 * have same action like notification and limiting charge.
		 */
		complete(comp);
	} else {
		hwlog_err("subcmd get too large data length %d\n", len);
	}
}

static int batt_info_cb(struct sk_buff *skb_in, struct genl_info *info)
{
	struct nl_dev_info *dev_info = NULL;
	struct platform_device *pdev = NULL;
	int len;
	struct nlattr *dev_attr = NULL;
	struct nlattr *data_attr = NULL;

	if (!info) {
		hwlog_err("info is null found in %s\n", __func__);
		return -1;
	}

	if (!info->attrs) {
		hwlog_err("info attrs is null found in %s\n", __func__);
		return -1;
	}

	dev_attr = info->attrs[POWER_GENL_BATT_INFO_DEV_ATTR];
	if (!dev_attr) {
		hwlog_err("dev_attr is null found in %s\n", __func__);
		return -1;
	}
	data_attr = info->attrs[POWER_GENL_BATT_INFO_DAT_ATTR];
	if (!data_attr) {
		hwlog_err("data_attr is null found in %s\n", __func__);
		return -1;
	}

	len = nla_len(dev_attr);
	dev_info = nla_data(dev_attr);
	if (len != sizeof(*dev_info)) {
		hwlog_err("dev attr in batt info mesg len uncorrect\n");
		return -1;
	}

	pdev = get_checker_pdev(dev_info);
	if (!pdev) {
		hwlog_err("Can't find battery checker(NO.:%d in Group:%d)\n",
			dev_info->id_in_grp, dev_info->id_of_grp);
		return -1;
	}

	hwlog_info("mesg %d for battery checker no.:%d in group:%d\n",
		dev_info->subcmd, dev_info->id_in_grp, dev_info->id_of_grp);
	batt_info_mesg_data_processor(pdev, dev_info->subcmd,
		nla_data(data_attr), nla_len(data_attr));

	return 0;
}

static signed char new_board;
static struct completion board_info_ready;

static int board_info_cb(unsigned char version, void *data, int len)
{
	hwlog_info("board information going to process\n");
	if (len != strlen(board_info_cb_mesg[NEW_BOARD_MESG_INDEX])) {
		hwlog_err("board info callback mesg len %d illegal\n", len);
		new_board = 0;
		return 0;
	}
	if (memcmp(board_info_cb_mesg[NEW_BOARD_MESG_INDEX], data, len))
		new_board = 0;

	else
		new_board = 1;
	complete(&board_info_ready);
	return 0;
}

static int nop_cb(unsigned char version, void *data, int len)
{
	return 0;
}

static char *generate_dmd_content(char *buf, int len, int dmd_no, int times)
{
	int offset = 0;
	int count = 0;
	int i;
	char *dmd_content = NULL;
	int *value = NULL;
	const char *str = dmd_no_to_str(dmd_no);

	dmd_content = kzalloc(DMD_REPORT_CONTENT_LEN + 1, GFP_KERNEL);
	if (!dmd_content)
		return NULL;

	if ((len % DMD_CONTENT_BASE_LEN) == 0) {
		if (str)
			count += snprintf(dmd_content + count,
				DMD_REPORT_CONTENT_LEN, str);
		for (i = 0; i < (len / DMD_CONTENT_BASE_LEN); i++) {
			value = (int *)buf + offset;
			/* value[0] : NO, value[1] : Group */
			count += snprintf(dmd_content + count,
				DMD_REPORT_CONTENT_LEN - count,
				"NO:%d Group:%d ", value[0], value[1]);
			/*
			 * value[2] : ic, value[3] : key, value[4] : sn
			 * value[5] : Y or N
			 */
			count += snprintf(dmd_content + count,
				DMD_REPORT_CONTENT_LEN - count,
				"rlt: ic(%d) key(%d) sn(%d Read:%s)",
				value[2], value[3], value[4],
				(value[5] > 0) ? "Y" : "N");
			offset += DMD_CONTENT_BASE_LEN;
		}
	}
	snprintf(dmd_content + count, DMD_REPORT_CONTENT_LEN - count,
		"Times:%d\n", times);
	return dmd_content;
}

static int batt_dmd_cb(unsigned char version, void *data, int len)
{
	struct dmd_record *record = NULL;
	char *ptr = (char *)data;
	int times;
	int head = (sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int));

	if (len < head) {
		hwlog_err("dmd info call back mesg length %d illegal\n", len);
		return 0;
	}
	record = kzalloc(sizeof(struct dmd_record), GFP_KERNEL);
	if (!record)
		return 0;

	memcpy(&record->dmd_type, ptr, sizeof(int));
	ptr += sizeof(int);
	memcpy(&record->dmd_no, ptr, sizeof(int));
	ptr += sizeof(int);
	memcpy(&times, ptr, sizeof(int));
	ptr += sizeof(int);
	memcpy(&record->content_len, ptr, sizeof(int));
	ptr += sizeof(int);

	if (len < (head + record->content_len)) {
		hwlog_err("dmd info call back mesg length %d:%d illegal\n",
			len, head + record->content_len);
		kfree(record);
		return 0;
	}
	record->content = generate_dmd_content(ptr, record->content_len,
		record->dmd_no, times);
	INIT_LIST_HEAD(&record->node);

	mutex_lock(&g_dmd_list.lock);
	list_add_tail(&record->node, &g_dmd_list.dmd_head);
	mutex_unlock(&g_dmd_list.lock);
	/* 3s */
	schedule_delayed_work(&g_dmd_list.dmd_record_report, 3 * HZ);

	return 0;
}

#define BATT_INFO_NL_CBS_NUM    1

static const struct power_genl_normal_ops batt_cbs[BATT_INFO_NL_CBS_NUM] = {
	{
		.cmd = POWER_GENL_CMD_BATT_INFO,
		.doit = batt_info_cb,
	},
};

#define BOARD_INFO_NL_OPS_NUM    5
static const struct power_genl_easy_ops batt_ops[BOARD_INFO_NL_OPS_NUM] = {
	{
		.cmd = POWER_GENL_CMD_BOARD_INFO,
		.doit = board_info_cb,
	},
	{
		.cmd = POWER_GENL_CMD_BATT_FINAL_RESULT,
		.doit = nop_cb,
	},
	{
		.cmd = POWER_GENL_CMD_BATT_DMD,
		.doit = batt_dmd_cb,
	},
	{
		.cmd = POWER_GENL_CMD_BATT_BIND_RD,
		.doit = batt_uuid_read_cb,
	},
	{
		.cmd = POWER_GENL_CMD_BATT_BIND_WR,
		.doit = batt_uuid_write_cb,
	},
};

/* BATT_SERVICE_ON handler */
static int batt_srv_on_cb(void)
{
	complete(&ct_srv_ready);

	return 0;
}

static struct power_genl_node batt_info_genl_node = {
	.target = POWER_GENL_TP_POWERCT,
	.name = "BATT_INFO",
	.easy_ops = batt_ops,
	.n_easy_ops = BOARD_INFO_NL_OPS_NUM,
	.normal_ops = batt_cbs,
	.n_normal_ops = BATT_INFO_NL_CBS_NUM,
	.srv_on_cb = batt_srv_on_cb,
};

static int batt_mesg_init(void)
{
	int ret;

	ret = power_genl_node_register(&batt_info_genl_node);
	if (ret)
		hwlog_err("power_genl_add_op failed %d\n", ret);

	return ret;
}

static inline void copy_int_to_buf(char **buf, int value)
{
	memcpy(*buf, &value, sizeof(int));
	*buf += sizeof(int);
}

static int prepare_dmd_content(int dmd_index, char **content)
{
	int content_len = 0;
	char *ptr = NULL;
	struct batt_checker_entry *temp = NULL;
	struct batt_chk_data *checker_data = NULL;

	if (!dmd_index)
		return 0;

	if (valid_checkers == 0)
		return 0;

	*content = kzalloc(valid_checkers * DMD_CONTENT_BASE_LEN, GFP_KERNEL);
	if (!(*content))
		return 0;
	ptr = *content;

	list_for_each_entry(temp, &batt_checkers_head, node) {
		if (temp->pdev) {
			checker_data = platform_get_drvdata(temp->pdev);
			if (!checker_data)
				return content_len;
			copy_int_to_buf(&ptr, checker_data->id_in_grp);
			copy_int_to_buf(&ptr, checker_data->id_of_grp);
			copy_int_to_buf(&ptr, checker_data->result.ic_status);
			copy_int_to_buf(&ptr, checker_data->result.key_status);
			copy_int_to_buf(&ptr, checker_data->result.sn_status);
			if (checker_data->sn && (checker_data->sn_len != 0))
				copy_int_to_buf(&ptr, 1);
			else
				copy_int_to_buf(&ptr, 0);
			content_len += DMD_CONTENT_BASE_LEN;
		}
	}

	return content_len;
}

static int get_records_num(const int *dmd_index, int dmd_num)
{
	int i;
	int record_num = 0;

	for (i = 0; i < dmd_num; i++) {
		if (!dmd_index || (dmd_index[i] <= DMD_INVALID))
			continue;
		record_num++;
	}

	return record_num;
}

static void prepare_dmd_record(int dmd_index, struct dmd_record *record)
{
	record->dmd_type = POWER_DSM_BATTERY_DETECT;
	record->dmd_no = get_dmd_no(dmd_index);
	record->content_len = prepare_dmd_content(dmd_index,
		&(record->content));
}

static int prepare_dmd_records(const int *dmd_index, int dmd_num,
	struct dmd_record **records)
{
	int i;
	int j = 0;
	int record_num;
	int record_size;

	record_num = get_records_num(dmd_index, dmd_num);
	if (record_num == 0)
		return 0;
	record_size = record_num * sizeof(struct dmd_record);

	*records = kzalloc(record_size, GFP_KERNEL);
	if (!(*records))
		return 0;

	for (i = 0; i < dmd_num; i++) {
		if (!dmd_index || (dmd_index[i] <= DMD_INVALID))
			continue;
		prepare_dmd_record(dmd_index[i], (*records) + j);
		j++;
		if (j >= record_num)
			break;
	}

	return record_num;
}

static int get_records_length(struct dmd_record *records, int num)
{
	int i;
	int length = 0;

	for (i = 0; i < num; i++) {
		length += sizeof(int);
		length += sizeof(int);
		length += sizeof(int);
		if (records[i].content_len > 0)
			length += records[i].content_len;
	}

	return length;
}

static int copy_record_to_buf(char *buf, struct dmd_record *record)
{
	char *tmp_ptr = buf;

	copy_int_to_buf(&tmp_ptr, record->dmd_type);
	copy_int_to_buf(&tmp_ptr, record->dmd_no);
	copy_int_to_buf(&tmp_ptr, record->content_len);
	if ((record->content_len > 0) && record->content)
		memcpy(tmp_ptr, record->content, record->content_len);

	return sizeof(int) + sizeof(int) + sizeof(int) + record->content_len;
}

static void send_result_msg(enum result_stat result_status,
	const int *dmd_index, int dmd_num)
{
	char *msg_buf = NULL;
	char *msg_ptr = NULL;
	struct dmd_record *records = NULL;
	int result_len = strlen(result_status_mesg[result_status]);
	int msg_len = (result_len + sizeof(int));
	int records_num = 0;
	int i;

	if (dmd_index && (dmd_num != 0)) {
		records_num = prepare_dmd_records(dmd_index, dmd_num, &records);
		msg_len += get_records_length(records, records_num);
	}

	msg_buf = kzalloc(msg_len, GFP_KERNEL);
	if (!msg_buf)
		goto FREE_DMD_RECORDS;
	msg_ptr = msg_buf;

	copy_int_to_buf(&msg_ptr, result_len);
	memcpy(msg_ptr, result_status_mesg[result_status], result_len);
	msg_ptr += result_len;
	if (records) {
		for (i = 0; i < records_num; i++)
			msg_ptr += copy_record_to_buf(msg_ptr, records + i);
	}

	if (power_genl_easy_send(POWER_GENL_TP_POWERCT, POWER_GENL_CMD_BATT_FINAL_RESULT,
		0, (void *)msg_buf, msg_len))
		hwlog_err("send battery final result status failed\n");
	kfree(msg_buf);

FREE_DMD_RECORDS:
	if (records) {
		for (i = 0; i < records_num; i++) {
			kfree(records[i].content);
			records[i].content = NULL;
		}
		kfree(records);
	}
}

/*
 * RESULT_STATUS is an existing definition, which has a wide
 * range of applications and will be uniformly rectified
 * in the future.
 */
static void send_result_status(struct batt_chk_rslt *result,
	enum result_stat result_status)
{
	int dmd_index;

	if (result) {
		dmd_index = prepare_dmd_no(result);
		send_result_msg(result_status, &dmd_index, 1);
		return;
	}

	send_result_msg(result_status, NULL, 0);
}

static int send_board_info(void)
{
	const char *mesg = board_info_mesg;

	if (power_genl_easy_send(POWER_GENL_TP_POWERCT, POWER_GENL_CMD_BOARD_INFO, 0,
		(void *)mesg, strlen(mesg))) {
		hwlog_err("send board information message failed\n");
		return -1;
	}

	return 0;
}

int send_power_genl_mesg(unsigned char cmd, void *data, unsigned int len)
{
	if (power_genl_easy_send(POWER_GENL_TP_POWERCT, cmd, 0, data, len)) {
		hwlog_err("send board information message failed\n");
		return -1;
	}

	return 0;
}

int send_batt_info_mesg(struct nl_dev_info *info, void *data, unsigned int len)
{
	struct power_genl_attr res[POWER_GENL_TOTAL_ATTR];

	res[0].data = (const unsigned char *)info;
	res[0].len = sizeof(*info);
	res[0].type = POWER_GENL_BATT_INFO_DEV_ATTR;

	res[1].data = (const unsigned char *)data;
	res[1].len = len;
	res[1].type = POWER_GENL_BATT_INFO_DAT_ATTR;

	if (power_genl_send(POWER_GENL_TP_POWERCT, POWER_GENL_CMD_BATT_INFO, 0, res,
		ARRAY_SIZE(res))) {
		hwlog_err("Send attributes failed in %s\n", __func__);
		return -1;
	}

	return 0;
}

int check_sn_includes_null(const unsigned char *sn, unsigned int sn_len)
{
	if (sn_len > MAX_SN_LEN)
		return -1;

	if (!sn)
		g_is_sn_empty = true;
	else
		g_is_sn_empty = strnlen(sn, sn_len) < sn_len;

	hwlog_info("[%s] g_is_sn_empty = %d\n", __func__, g_is_sn_empty);
	return 0;
}

static signed char new_battery;

static int get_last_check_result(struct batt_info *drv_data,
	struct batt_chk_rslt *result)
{
	hwlog_info("[%s] is_first_check_done = %d\n",
		__func__, drv_data->is_first_check_done);
	if (drv_data->is_first_check_done) {
		memcpy(result, &drv_data->result, sizeof(*result));
		return 0;
	}
	if (power_nv_read(POWER_NV_BLIMSW, result,
		sizeof(*result))) {
		hwlog_err("get last check result failed\n");
		return -1;
	}

	return 0;
}

static void record_final_check_result(struct batt_chk_rslt *result)
{
	if (power_nv_write(POWER_NV_BLIMSW, result,
		sizeof(*result)))
		hwlog_err("record final check result failed\n");
}

static void get_sn_from_nv(struct batt_info *drv_data)
{
	struct binded_info bbinfo;
	struct batt_chk_rslt result;

	drv_data->sn = drv_data->sn_buff;
	if (power_nv_read(POWER_NV_BBINFO, &bbinfo,
		sizeof(struct binded_info))) {
		hwlog_err("%s failed\n", __func__);
		drv_data->sn_len = strlen("NV ERROR");
		memcpy(drv_data->sn_buff, "NV ERROR", drv_data->sn_len);
		return;
	}
	if (bbinfo.version != drv_data->sn_version) {
		hwlog_info("sn binding nv info version different from dts\n");
		/* force next reboot real check battery information */
		memset(&result, 0, sizeof(result));
		record_final_check_result(&result);
		drv_data->sn_len = strlen("REBOOT PLEASE");
		memcpy(drv_data->sn_buff, "REBOOT PLEASE", drv_data->sn_len);
		return;
	}
	switch (drv_data->sn_version) {
	case RAW_BIND_VERSION:
		/* init normal battery sn length(16) */
		drv_data->sn_len = 16;
		break;
	case PAIR_BIND_VERSION:
		/* init paired battery sn length(17) */
		drv_data->sn_len = 17;
		break;
	default:
		/* never enter here */
		break;
	}
	memcpy(drv_data->sn_buff, bbinfo.info[0], drv_data->sn_len);
}

static int record_sn_to_nv(struct binded_info *bbinfo)
{
	if (power_nv_write(POWER_NV_BBINFO, bbinfo,
		sizeof(struct binded_info))) {
		hwlog_err("%s failed\n", __func__);
		return -1;
	}

	return 0;
}

static int ic_status_legal(enum ic_cr ic_status)
{
	return ic_status == IC_PASS;
}

static int key_status_legal(enum key_cr key_status)
{
	return key_status == KEY_PASS;
}

static int sn_status_legal(enum sn_cr sn_status)
{
	return sn_status <= SN_NN_REMATCH;
}

static int is_legal_result(const struct batt_chk_rslt *result)
{
	return ic_status_legal(result->ic_status) &&
	    key_status_legal(result->key_status) &&
	    sn_status_legal(result->sn_status);
}

static enum result_stat chk_rs_to_rs_stat(const struct batt_chk_rslt *result)
{
	if ((result->key_status == KEY_FAIL_TIMEOUT) ||
		(result->sn_status == SN_FAIL_TIMEOUT) ||
		(result->sn_status == SN_FAIL_NV))
		return FINAL_RESULT_CRASH;
	if (is_legal_result(result))
		return FINAL_RESULT_PASS;

	return FINAL_RESULT_FAIL;
}

/* use the sn result of the first battery checker in batt_checkers */
static int final_sn_checker_nop(struct batt_info *drv_data)
{
	drv_data->result.sn_status = SN_PASS;

	return 0;
}

static void move_match_sn_to_head(int i, struct binded_info *bbinfo)
{
	char temp[MAX_SN_LEN] = { 0 };

	if (i <= 0)
		return;
	memcpy(temp, bbinfo->info[i], MAX_SN_LEN);
	for (; i > 0; i--)
		memcpy(bbinfo->info[i], bbinfo->info[i - 1], MAX_SN_LEN);
	memcpy(bbinfo->info[0], temp, MAX_SN_LEN);
}

static inline void set_nv_fail(struct batt_info *drv_data)
{
	drv_data->result.sn_status = SN_FAIL_NV;
}

static int binding_check(struct batt_info *drv_data)
{
	int i;
	const unsigned char *sn = drv_data->sn;
	unsigned int sn_len = drv_data->sn_len;
	struct binded_info bbinfo;

	if (power_nv_read(POWER_NV_BBINFO, &bbinfo,
		sizeof(bbinfo))) {
		drv_data->result.sn_status = SN_FAIL_NV;
		return -1;
	}

	switch (bbinfo.version) {
	case RAW_BIND_VERSION:
	case PAIR_BIND_VERSION:
		for (i = 0; i < MAX_SN_BUFF_LENGTH; i++) {
			if (memcmp(bbinfo.info[i], sn, sn_len))
				continue;
			hwlog_info("board and battery well matched\n");
			if ((i > 0) ||
				(bbinfo.version != drv_data->sn_version)) {
				move_match_sn_to_head(i, &bbinfo);
				bbinfo.version = drv_data->sn_version;
				if (record_sn_to_nv(&bbinfo)) {
					set_nv_fail(drv_data);
					return -1;
				}
			}
			drv_data->result.sn_status = SN_PASS;
			return 0;
		}
		break;
	default:
		hwlog_err("Unkown BBINFO NV version %u found in %s\n",
			bbinfo.version, __func__);
		break;
	}

	if (!new_battery && new_board)
		drv_data->result.sn_status = SN_OBT_REMATCH;
	else if (!new_board && new_battery)
		drv_data->result.sn_status = SN_OBD_REMATCH;
	else if (new_battery && new_board)
		drv_data->result.sn_status = SN_NN_REMATCH;
	else
		drv_data->result.sn_status = SN_OO_UNMATCH;

	bbinfo.version = drv_data->sn_version;
#ifdef BATTBD_FORCE_MATCH
	for (i = 0; i < MAX_SN_BUFF_LENGTH; i++)
		memcpy(bbinfo.info[i], sn, sn_len);
	if (record_sn_to_nv(&bbinfo)) {
		drv_data->result.sn_status = SN_FAIL_NV;
		return -1;
	}
#else
	if ((drv_data->result.sn_status == SN_OBT_REMATCH) ||
		(drv_data->result.sn_status == SN_OBD_REMATCH) ||
		(drv_data->result.sn_status == SN_NN_REMATCH)) {
		for (i = MAX_SN_BUFF_LENGTH - 1; i > 0; i--)
			memcpy(bbinfo.info[i], bbinfo.info[i - 1], MAX_SN_LEN);
		memcpy(bbinfo.info[0], sn, sn_len);
		if (record_sn_to_nv(&bbinfo)) {
			drv_data->result.sn_status = SN_FAIL_NV;
			return -1;
		}
	}
#endif /* BATTBD_FORCE_MATCH */

	return 0;
}

#define MAX_VALID_CHECKERS      200

static int check_devs_match(struct batt_info *drv_data)
{
	struct batt_checker_entry *temp = NULL;
	struct platform_device *pdev = NULL;
	struct batt_chk_data *checker_data = NULL;
	int i, j;
	int sn_len = -1;
	const unsigned char *sn = NULL;
	char *buf = NULL;

	if (valid_checkers > MAX_VALID_CHECKERS) {
		hwlog_err("too much %u valid battery checkers\n",
			valid_checkers);
		return -1;
	}
	buf = kmalloc(valid_checkers, GFP_KERNEL);
	if (!buf)
		return -1;
	memset(buf, -1, valid_checkers);
	i = 0;
	list_for_each_entry(temp, &batt_checkers_head, node) {
		pdev = temp->pdev;
		checker_data = platform_get_drvdata(pdev);
		if (!checker_data->sn) {
			hwlog_err("%s sn is null\n", checker_data->ic->name);
			kfree(buf);
			drv_data->result.sn_status = SN_SNS_UNMATCH;
			return 0;
		}
		if (!sn && (sn_len < 0)) {
			sn = checker_data->sn;
			sn_len = checker_data->sn_len;
			buf[i++] = sn[sn_len - 1];
			continue;
		}
		if (memcmp(sn, checker_data->sn, sn_len - 1) ||
			(sn_len != checker_data->sn_len)) {
			drv_data->result.sn_status = SN_SNS_UNMATCH;
			kfree(buf);
			hwlog_err("sn body match fail\n");
			return 0;
		}
		buf[i++] = checker_data->sn[sn_len - 1];
	}
	bubble_sort(buf, i);
	for (j = 0; j < i - 1; j++) {
		if (buf[j] == buf[j + 1]) {
			drv_data->result.sn_status = SN_SNS_UNMATCH;
			kfree(buf);
			hwlog_err("sn no match fail\n");
			return 0;
		}
	}
	kfree(buf);

	return 0;
}

static enum batt_aut_util_type check_util_type(void)
{
	struct batt_checker_entry *temp = NULL;
	struct batt_chk_data *checker_data = NULL;
	enum batt_aut_util_type util_type;

	list_for_each_entry(temp, &batt_checkers_head, node) {
		checker_data = platform_get_drvdata(temp->pdev);
		util_type = checker_util_type(checker_data->ic_type);
		if (util_type == UTIL_BYC_TYPE)
			return UTIL_BYC_TYPE;
	}

	return UTIL_BSC_TYPE;
}

static bool update_binded_info(struct binded_info *bbinfo, const char *sn,
	unsigned int sn_len)
{
	int i;

	if (!bbinfo)
		return false;

	if (!sn || (sn_len == 0) || (sn_len >= MAX_SN_LEN))
		return false;

	for (i = 0; i < MAX_SN_BUFF_LENGTH; i++) {
		if (memcmp(bbinfo->info[i], sn, sn_len))
			continue;

		if (i > 0) {
			move_match_sn_to_head(i, bbinfo);
			return true;
		}
		return false;
	}

	for (i = MAX_SN_BUFF_LENGTH - 1; i > 0; i--)
		memcpy(bbinfo->info[i], bbinfo->info[i - 1], MAX_SN_LEN);
	memcpy(bbinfo->info[0], sn, sn_len);
	return true;
}

static int record_batt_sn(struct batt_info *drv_data)
{
	int i = 0;
	bool update_nv = false;
	struct batt_checker_entry *temp = NULL;
	struct batt_chk_data *checker_data = NULL;
	struct binded_info bbinfo;

	memset(&bbinfo, 0, sizeof(bbinfo));
	if (power_nv_read(POWER_NV_BBINFO, &bbinfo, sizeof(bbinfo)))
		return -1;

	bbinfo.version = drv_data->sn_version;
	list_for_each_entry(temp, &batt_checkers_head, node) {
		checker_data = platform_get_drvdata(temp->pdev);
		if (!checker_data || !checker_data->bco.get_batt_sn)
			continue;

		if (!checker_data->sn) {
			if (checker_data->bco.get_batt_sn(checker_data, 0,
				&checker_data->sn, &checker_data->sn_len))
				hwlog_err("get battery sn failed in %s\n",
					__func__);
		}

		update_nv |= update_binded_info(&bbinfo, checker_data->sn,
			checker_data->sn_len);

		i++;
		if (i >= MAX_SN_BUFF_LENGTH)
			break;
	}

	if (!update_nv)
		return 0;

	if (record_sn_to_nv(&bbinfo))
		hwlog_err("write sn to nv failed in %s\n", __func__);
	else
		hwlog_info("write sn to nv success in %s\n", __func__);

	return 0;
}

/* for paired sns(only last byte of sn is different) */
static int final_sn_checker_1(struct batt_info *drv_data)
{
	struct batt_checker_entry *temp = NULL;
	struct platform_device *pdev = NULL;
	struct batt_chk_data *checker_data = NULL;

	/* sn matchs with each other? */
	if (valid_checkers > 1) {
		if (check_devs_match(drv_data)) {
			hwlog_info("checking battery match failed\n");
			return -1;
		}
		/* if sn in batterys unmatch */
		if (drv_data->result.sn_status == SN_SNS_UNMATCH)
			return 0;
	}

	/* get final sn and sn_len here */
	list_for_each_entry(temp, &batt_checkers_head, node) {
		pdev = temp->pdev;
		checker_data = platform_get_drvdata(pdev);
		drv_data->sn = checker_data->sn;
		if (valid_checkers > 1) {
			drv_data->sn_len = checker_data->sn_len - 1;
			if (drv_data->sn_version != PAIR_BIND_VERSION) {
				hwlog_err("PAIR_BIND_VERSION unmatch sn len\n");
				return -1;
			}
		} else {
			drv_data->sn_len = checker_data->sn_len;
			if (drv_data->sn_version != RAW_BIND_VERSION) {
				hwlog_err("RAW_BIND_VERSION unmatch sn len\n");
				return -1;
			}
		}
	}

	if (check_util_type() == UTIL_BYC_TYPE)
		return record_batt_sn(drv_data);

	/* sn checking last step final sn binding check */
	if (binding_check(drv_data)) {
		hwlog_err("checking battery binded with board failed\n");
		return -1;
	}
	return 0;
}

static const final_sn_checker_t final_sn_checkers[] = {
	final_sn_checker_nop,
	final_sn_checker_1,
};

static void power_down_ics(void)
{
	struct batt_checker_entry *temp = NULL;
	struct batt_chk_data *checker_data = NULL;

	list_for_each_entry(temp, &batt_checkers_head, node) {
		checker_data = platform_get_drvdata(temp->pdev);
		if (checker_data->bco.power_down)
			checker_data->bco.power_down(checker_data);
	}
}

static int prepare_bind_info(void)
{
	if (check_util_type() == UTIL_BYC_TYPE)
		return batt_read_bind_mesg();

	/* board on boot new or old? */
	if (send_board_info()) {
		hwlog_err("checking stopped by send get board info failed\n");
		return -1;
	}
	wait_for_completion(&board_info_ready);
	return 0;
}

static void run_check_func(struct batt_info *drv_data, int check_strategy_no)
{
	unsigned long flags;

	if (!drv_data || (check_strategy_no < CHECK_STRATEGY_INVALID) ||
		(check_strategy_no > CHECK_STRATEGY_MAX_NO))
		return;

	hwlog_info("[%s] request_lock locking, is_checking = %d, no = %d\n",
		__func__, drv_data->is_checking, check_strategy_no);

	spin_lock_irqsave(&drv_data->request_lock, flags);
	if (drv_data->is_checking) {
		spin_unlock_irqrestore(&drv_data->request_lock, flags);
		return;
	}
	drv_data->is_checking = true;
	spin_unlock_irqrestore(&drv_data->request_lock, flags);

	drv_data->check_strategy_no = check_strategy_no;
	/* start checking work */
	schedule_work(&drv_data->check_work);
}

static void check_nv_sn(struct batt_info *info)
{
	if (check_battery_sn_changed() != 1)
		return;
	info->nv_sn_old = 1;
	info->dmd_no = DMD_OLD_NV_SN_ERROR;
	schedule_delayed_work(&info->dmd_report_dw, 5 * HZ);
}

enum batt_match_type get_batt_match_type()
{
	struct batt_chk_data *checker_data = NULL;
	struct batt_checker_entry *temp = NULL;
	int check_cnt = 0;
	enum batt_match_type match_type = BATTERY_REMATCHABLE;

	list_for_each_entry(temp, &batt_checkers_head, node) {
		check_cnt++;
		checker_data = platform_get_drvdata(temp->pdev);
		if (checker_data->batt_rematch_onboot ==
				BATTERY_UNREMATCHABLE) {
			match_type = BATTERY_UNREMATCHABLE;
			break;
		}
	}
	if ((match_type == BATTERY_REMATCHABLE) && (check_cnt != g_total_checkers))
		match_type = BATTERY_UNREMATCHABLE;
	hwlog_info("%s:batt_match_type %d, check_cnt %d\n", __func__, match_type, check_cnt);
	return match_type;
}

static bool check_moved_recheck(struct batt_info *info)
{
	int removed_flag = 0;
	int sn_change_flag = 0;
	bool need_recheck = false;

	sn_change_flag = check_battery_sn_changed();
	removed_flag = power_platform_is_battery_removed();
	if (info->moved_recheck_logic) {
		need_recheck = (sn_change_flag != 0) && (removed_flag == 1);
	} else {
		need_recheck = (sn_change_flag != 0) || (removed_flag == 1);
	}
	hwlog_info("%s:moved_recheck_logic %d, removed_flag %d, sn_change_flag %d\n",
		__func__, info->moved_recheck_logic, removed_flag, sn_change_flag);
	return need_recheck;
}

static void check_func(struct work_struct *work)
{
	int ret;
	int i;
	enum result_stat result_status;
	struct batt_chk_rslt dummy_result;
	struct batt_chk_rslt *last_result = NULL;
	struct batt_chk_rslt *final_result = NULL;
	struct batt_chk_data *checker_data = NULL;
	struct batt_info *drv_data = NULL;
	struct batt_checker_entry *temp = NULL;

	hwlog_info("battery checking started in %s\n", __func__);
	drv_data = container_of(work, struct batt_info, check_work);

	__pm_wakeup_event(&drv_data->checking_wakelock,
		jiffies_to_msecs(CHECKING_WAKELOCK_TIME_IN_SECOND * HZ));
	/*
	 * wait for service powerct ready when first checking.
	 * It also can use check_strategy as a flag, but for avoiding
	 * introducing new problems due to someone does't know, here uses a
	 * new variable.
	 *
	 * is_first_check_done: 0: false, means checking in booting;
	 *                      1: true, means checking in others
	 */
	if (!drv_data->is_first_check_done)
		wait_for_completion(&ct_srv_ready);

	if (drv_data->check_strategy_no == CHECK_STRATEGY_BOOTING &&
		g_sn_change_onboot < 0)
		g_sn_change_onboot = check_battery_sn_changed();

	/* check last result */
	last_result = &drv_data->last_result;
	for (i = 0; i < RETRY_TIMES; i++) {
		ret = get_last_check_result(drv_data, last_result);
		if (!ret) {
			hwlog_info("last result: ic %d, key %d, sn %d, mode %d\n",
				last_result->ic_status, last_result->key_status,
				last_result->sn_status, last_result->check_mode);
			break;
		}

		/* retry after 1s when reading nv fail */
		msleep(1000);
		hwlog_err("get last result failed, retry times: %d\n", i);
	}

	init_battery_check_result(&drv_data->result);
	final_result = &drv_data->result;

	if (ret != 0) {
		hwlog_err("last_result NV still failed, skip check\n");
		dummy_result.check_mode = 0;
		dummy_result.ic_status = IC_PASS;
		dummy_result.key_status = KEY_PASS;
		dummy_result.sn_status = SN_FAIL_NV;
		*final_result = dummy_result;
		schedule_delayed_work(&drv_data->dmd_report_dw, 0);
		goto exit_check_func;
	}

	if (shield_ct_sign) {
		dummy_result.check_mode = 0;
		dummy_result.ic_status = IC_PASS;
		dummy_result.key_status = KEY_PASS;
		dummy_result.sn_status = SN_PASS;
		*final_result = dummy_result;
		result_status = FINAL_RESULT_PASS;
		if (record_batt_sn(drv_data))
			hwlog_err("record battery sn failed in %s\n", __func__);
		hwlog_info("battery ct shielding to dummy result\n");
		goto process_result_status;
	}

	/* Follow conditions will trigger real battery check
	 * 1.get last check result failed
	 * 2.last check result is illegal
	 * 3.kernel is factory version
	 * 4.battery is removed before boot
	 */
	if (!(ret || !is_legal_result(last_result) ||
		(last_result->check_mode == FACTORY_CHECK_MODE) ||
		check_moved_recheck(drv_data) ||
		(get_batt_match_type() == BATTERY_REMATCHABLE))) {
		*final_result = *last_result;
		result_status = FINAL_RESULT_PASS;
		check_nv_sn(drv_data);
		if (drv_data->sn_checker != final_sn_checker_nop)
			get_sn_from_nv(drv_data);
		hwlog_info("battery ct will skip checking in %s\n", __func__);
		goto skip_result_status;
	}

	hwlog_info("real checking started in %s\n", __func__);
	/* check if all battery checkers probe successfully */
	if (drv_data->total_checkers != valid_checkers) {
		final_result->ic_status = IC_FAIL_UNKOWN;
		hwlog_err("some checkers failed to probe\n");
		goto process_result_status;
	}
	/* checkers' ic status all good? */
	list_for_each_entry(temp, &batt_checkers_head, node) {
		checker_data = platform_get_drvdata(temp->pdev);
		ret = ic_status_legal(checker_data->result.ic_status);
		if (!ret) {
			final_result->ic_status =
				checker_data->result.ic_status;
			hwlog_info("checker(NO:%d Group:%d) status illegal\n",
				checker_data->id_in_grp,
				checker_data->id_of_grp);
			goto process_result_status;
		}
	}
	final_result->ic_status = IC_PASS;

	hwlog_info("checking key status started in %s\n", __func__);
	/* authenticate checker keys */
	list_for_each_entry(temp, &batt_checkers_head, node) {
		checker_data = platform_get_drvdata(temp->pdev);
		schedule_work(&checker_data->check_key_work);
	}
	list_for_each_entry(temp, &batt_checkers_head, node) {
		checker_data = platform_get_drvdata(temp->pdev);
		flush_work(&checker_data->check_key_work);
	}
	list_for_each_entry(temp, &batt_checkers_head, node) {
		checker_data = platform_get_drvdata(temp->pdev);
		ret = key_status_legal(checker_data->result.key_status);
		if (!ret) {
			*final_result = checker_data->result;
			hwlog_info("batt(NO:%d Group:%d) key status illegal\n",
				checker_data->id_in_grp,
				checker_data->id_of_grp);
			goto process_result_status;
		}
	}
	final_result->key_status = KEY_PASS;

	hwlog_info("getting board status started in %s\n", __func__);
	prepare_bind_info();

	hwlog_info("getting battery status started in %s\n", __func__);
	/* battery on boot new or old? */
	list_for_each_entry(temp, &batt_checkers_head, node) {
		checker_data = platform_get_drvdata(temp->pdev);
		if (checker_data->batt_rematch_onboot ==
			BATTERY_UNREMATCHABLE) {
			new_battery = BATTERY_UNREMATCHABLE;
			break;
		}
	}

	hwlog_info("checking all battery checkers's sn started in %s\n",
		__func__);
	/* all battery checkers check its own sn(maybe just get sn) */
	list_for_each_entry(temp, &batt_checkers_head, node) {
		checker_data = platform_get_drvdata(temp->pdev);
		schedule_work(&checker_data->check_sn_work);
	}
	list_for_each_entry(temp, &batt_checkers_head, node) {
		checker_data = platform_get_drvdata(temp->pdev);
		flush_work(&checker_data->check_sn_work);
	}
	list_for_each_entry(temp, &batt_checkers_head, node) {
		checker_data = platform_get_drvdata(temp->pdev);
		ret = sn_status_legal(checker_data->result.sn_status);
		final_result->sn_status = checker_data->result.sn_status;
		if (!ret) {
			hwlog_info("batt(NO:%d Group:%d) sn status illegal\n",
				checker_data->id_in_grp,
				checker_data->id_of_grp);
			goto process_result_status;
		}
	}

	/* sn final check */
	if (drv_data->sn_checker(drv_data)) {
		hwlog_err("final sn check failed\n");
		goto exit_check_func;
	}
	batt_write_bind_mesg();

process_result_status:
	if (memcmp(last_result, final_result, sizeof(struct batt_chk_rslt)))
		record_final_check_result(final_result);
	schedule_delayed_work(&drv_data->dmd_report_dw, 0);
skip_result_status:
	result_status = chk_rs_to_rs_stat(final_result);
	send_result_status(final_result, result_status);
#ifdef BATTERY_LIMIT_DEBUG
	finish_time = ktime_to_timespec(ktime_get_boottime());
#endif
	if (result_status != FINAL_RESULT_FAIL)
		power_down_ics();

exit_check_func:
	if (drv_data->checking_wakelock.active)
		__pm_relax(&drv_data->checking_wakelock);
	drv_data->is_first_check_done = true;
	drv_data->is_checking = false;
}

static int batt_id_fill(struct batt_chk_data *checker_data,
	char *buf, int buf_len, char index)
{
	const unsigned char *sn = NULL;
	unsigned int sn_len;
	int cur_len = strlen(buf);

	if (!checker_data->bco.get_batt_sn)
		return 0;

	if (checker_data->bco.get_batt_sn(checker_data,
		&checker_data->sn_res, &sn, &sn_len) ||
		(sn_len == 0)) {
		hwlog_err("get battery sn failed in %s\n", __func__);
		return 0;
	}

	/* 3 is size of '1' + '\n' + '0' */
	if ((cur_len + sn_len + 3) >= buf_len) {
		hwlog_err("battery sn lenght error %d %u %d\n",
			cur_len, sn_len, buf_len);
		return -1;
	}

	memcpy(buf + cur_len, sn, sn_len);
	buf[sn_len + cur_len] = index;
	buf[sn_len + cur_len + 1] = '\n';
	return 0;
}

static ssize_t batt_id_online(char *buf, int buf_len)
{
	struct batt_checker_entry *temp = NULL;
	struct platform_device *pdev = NULL;
	struct batt_chk_data *checker_data = NULL;
	int len;
	char index = '1';

	memset(buf, 0, buf_len);
	list_for_each_entry(temp, &batt_checkers_head, node) {
		pdev = temp->pdev;
		checker_data = platform_get_drvdata(pdev);
		if (batt_id_fill(checker_data, buf, buf_len, index))
			goto get_sn_fail;

		index++;
	}

	len = strlen(buf);
	if (len > 0)
		return len;

get_sn_fail:
	memset(buf, 0, buf_len);
	len = strlen("IC ERROR");
	memcpy(buf, "IC ERROR", len);
	return len;
}

static ssize_t batt_id_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct batt_info *drv_data = NULL;
	struct batt_checker_entry *entry = NULL;
	ssize_t len = 0;
	char i = '1';

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");
	if (!drv_data->sn)
		return batt_id_online(buf, PAGE_SIZE);

	list_for_each_entry(entry, &batt_checkers_head, node) {
		memcpy(buf + len, drv_data->sn, drv_data->sn_len);
		buf[drv_data->sn_len + len] = i++;
		buf[drv_data->sn_len + len + 1] = '\n';
		/* 2: buf size */
		len += drv_data->sn_len + 2;
	}
	if (len)
		len -= 1;

	return len;
}

static ssize_t batt_id_v_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct batt_info *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");

	return snprintf(buf, PAGE_SIZE, "%u", drv_data->sn_version);
}

static ssize_t nv_sn_good_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_info *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (drv_data && drv_data->nv_sn_old == 1)
		return snprintf(buf, PAGE_SIZE, "0");
	return snprintf(buf, PAGE_SIZE, "1");
}

static ssize_t batt_num_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct batt_info *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");

	return snprintf(buf, PAGE_SIZE, "%d", drv_data->total_checkers);
}

static ssize_t ic_status_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct batt_info *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");
	if (ic_status_legal(drv_data->result.ic_status))
		return snprintf(buf, PAGE_SIZE, "PASS");
	else
		return snprintf(buf, PAGE_SIZE, "FAIL %u",
			drv_data->result.ic_status);
}

static ssize_t key_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_info *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");
	if (key_status_legal(drv_data->result.key_status))
		return snprintf(buf, PAGE_SIZE, "PASS");
	else
		return snprintf(buf, PAGE_SIZE, "FAIL %u",
			drv_data->result.key_status);
}

static ssize_t sn_status_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct batt_info *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error driver data");
	if (sn_status_legal(drv_data->result.sn_status))
		return snprintf(buf, PAGE_SIZE, "PASS");
	else
		return snprintf(buf, PAGE_SIZE, "FAIL %u",
			drv_data->result.sn_status);
}

static ssize_t check_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_info *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");
	if (drv_data->result.check_mode == FACTORY_CHECK_MODE)
		return snprintf(buf, PAGE_SIZE, "Factory");
	else if (drv_data->result.check_mode == COMMERCIAL_CHECK_MODE)
		return snprintf(buf, PAGE_SIZE, "Normal");
	else
		return snprintf(buf, PAGE_SIZE, "Unkown");
}

static ssize_t official_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct batt_info *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");
	if (is_legal_result(&drv_data->result))
		return snprintf(buf, PAGE_SIZE, "1");
	else
		return snprintf(buf, PAGE_SIZE, "0");
}

static ssize_t board_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	if (new_board < 0) {
		if (send_board_info())
			return snprintf(buf, PAGE_SIZE, "unknown");
		else if (wait_for_completion_interruptible(&board_info_ready))
			return snprintf(buf, PAGE_SIZE, "%s", "Unknown");
	}

	return snprintf(buf, PAGE_SIZE, "%s", new_board ? "New" : "Old");
}

static ssize_t battery_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	if (new_battery < 0)
		new_battery = get_batt_match_type();
	return snprintf(buf, PAGE_SIZE, "%s", new_battery ? "New" : "Old");
}

static ssize_t board_runnable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d", (shield_ct_sign > 0) ? 0 : 1);
}

static ssize_t battery_ct_shield_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d", shield_ct_sign);
}

static ssize_t check_request_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int check_strategy_no = 0;
	struct batt_info *drv_data = NULL;

	hwlog_info("[%s] buf is %s\n", __func__, buf);
	if (kstrtoint(buf, POWER_BASE_DEC, &check_strategy_no))
		return -EINVAL;

	hwlog_info("[%s] strategy_no is %d\n", __func__, check_strategy_no);
	dev_get_drv_data(drv_data, dev);
	/*
	 * if some products don't support checking in running, the function
	 * would send last result that is getting in booting to powerCt.
	 */
	if (!drv_data->can_check_in_running) {
		hwlog_info("[%s] cannot check in running, using last result\n",
			__func__);
		send_result_status(&drv_data->result,
			chk_rs_to_rs_stat(&drv_data->result));
		return count;
	}

	run_check_func(drv_data, check_strategy_no);
	return count;
}

static ssize_t check_execute_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_info *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	return snprintf(buf, PAGE_SIZE, "%d", drv_data->is_checking);
}

static ssize_t uevent_data_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct batt_info *drv_data = NULL;

	hwlog_info("[%s] buf is %s\n", __func__, buf);
	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return -EINVAL;

	bsoh_uevent_rcv(BSOH_EVT_BATT_INFO_UPDATE, buf);
	return count;
}

static ssize_t can_check_in_running_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_info *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	return snprintf(buf, PAGE_SIZE, "%u", drv_data->can_check_in_running);
}

/*
 * if the ic-chip id is in the prohibited id list
 * powerct will do the follow two things:
 * 1. return flag data (res.data = NULL, res.len = 0) to kernel by netlink
 * 2. write error code to here(powerct_error_code_store), but this feature
 *    just uses for dmd module.
 *    Because the program use the flag data at the first point above,
 *    then kernel will occur an error in original logic like paramters invalid.
 *    The error will make powerct take action.
 *
 *    Why program doesn't use the second feature? because it can prevent
 *    mistakes leading by modifying original logic.
 */
static ssize_t powerct_error_code_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int error_code = 0;

	if (kstrtoint(buf, POWER_BASE_DEC, &error_code))
		return -EINVAL;
	hwlog_info("[%s] error_code = %d\n", __func__, error_code);
	g_error_code = (enum powerct_error_code)error_code;
	return count;
}

#ifdef BATTERY_LIMIT_DEBUG
static ssize_t ftime_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	/* 1000 : Convert to nanoseconds */
	return snprintf(buf, PAGE_SIZE, "%llu.%06llu",
		(unsigned long long)finish_time.tv_sec,
		(unsigned long long)finish_time.tv_nsec / 1000);
}

static ssize_t ctime_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct timespec ts;

	ts = ktime_to_timespec(ktime_get_boottime());

	/* 1000 : Convert to nanoseconds */
	return snprintf(buf, PAGE_SIZE, "%llu.%06llu",
		(unsigned long long)ts.tv_sec,
		(unsigned long long)ts.tv_nsec / 1000);
}

static ssize_t bind_info_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct binded_info bbinfo;
	int i;
	int count = 0;

	if (power_nv_read(POWER_NV_BBINFO, &bbinfo, sizeof(bbinfo)))
		return snprintf(buf, PAGE_SIZE, "Error:Read NV Fail");
	for (i = 0; i < MAX_SN_BUFF_LENGTH; i++) {
		memcpy(buf + count, bbinfo.info[i], MAX_SN_LEN);
		count += MAX_SN_LEN;
		buf[count++] = '\n';
	}

	return count;
}

static ssize_t bind_info_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int indx = 0;
	char sn_buf[MAX_SN_LEN];
	struct binded_info bbinfo;

	sscanf(buf, "%d,%s", &indx, sn_buf);
	power_nv_read(POWER_NV_BBINFO, &bbinfo, sizeof(bbinfo));
	if ((indx < 0) || (indx >= MAX_BATT_BIND_NUM))
		return -1;
	memcpy(bbinfo.info[indx], sn_buf, MAX_SN_LEN);
	if (record_sn_to_nv(&bbinfo))
		return -1;

	return count;
}

static ssize_t bind_uuid_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_uuid_bind info;
	int i;
	int j;
	int count;

	if (batt_get_bind_uuid(&info))
		return snprintf(buf, PAGE_SIZE, "error\n");

	count = snprintf(buf, PAGE_SIZE, "%s\n",
		(info.new_board == OLD_BOARD) ? "old board" : "new board");
	for (i = 0; i < MAX_BATT_BIND_NUM; i++) {
		for (j = 0; j < MAX_BATT_UUID_LEN; j++) {
			count += snprintf(buf + count, PAGE_SIZE - count,
				"%02X", info.record.uuid[i][j]);
			/* 4 = sizeof("xx\n") */
			if ((count + 4) > PAGE_SIZE)
				return snprintf(buf, PAGE_SIZE, "error\n");
		}
		count += snprintf(buf + count, PAGE_SIZE - count, "\n");
	}

	return count;
}

static ssize_t bind_uuid_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	if (!sysfs_streq(buf, "clear"))
		return -1;

	if (batt_clear_bind_uuid())
		return -1;

	return count;
}

static ssize_t check_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct batt_info *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return -1;

	/* 5: buf size */
	if ((count >= 5) && !memcmp(buf, "check", 5))
		run_check_func(drv_data, CHECK_STRATEGY_DEBUG);

	return count;
}

static ssize_t final_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct batt_info *drv_data = NULL;
	int temp;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return -1;
	/* 6: buf size  5: final length */
	if ((count >= 6) && !memcmp(buf, "final", 5)) {
		temp = buf[6] - '0';
		if ((temp >= 0) && (temp < __FINAL_RESULT_MAX))
			send_result_status(NULL, (enum result_stat)temp);
	}

	return count;
}

static ssize_t sn_checker_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_info *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");

	return snprintf(buf, PAGE_SIZE, "%pf", drv_data->sn_checker);
}

static ssize_t sn_checker_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct batt_info *drv_data = NULL;
	int temp;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return -1;
	/* 5: buf size  4: type length */
	if ((count >= 5) && !memcmp(buf, "type", 4)) {
		temp = buf[4] - '0';
		if ((temp >= 0) && (temp < ARRAY_SIZE(final_sn_checkers)))
			drv_data->sn_checker = final_sn_checkers[temp];
	}

	return count;
}

static ssize_t check_result_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_info *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");

	return snprintf(buf, PAGE_SIZE, "%d %d %d %d",
		drv_data->result.ic_status,
		drv_data->result.key_status,
		drv_data->result.sn_status,
		drv_data->result.check_mode);
}

#define CHECK_RESUT_STR_SIZE 64

static ssize_t check_result_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	enum result_stat result_status;
	/* 4: temp size */
	int temp[4];
	char str[CHECK_RESUT_STR_SIZE] = { 0 };
	size_t len;
	struct batt_info *drv_data = NULL;
	char *sub = NULL;
	char *cur = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return -1;
	len = min_t(size_t, sizeof(str) - 1, count);
	memcpy(str, buf, len);
	cur = &str[0];
	sub = strsep(&cur, " ");
	if (!sub || kstrtoint(sub, 0, &temp[0]))
		return -1;
	sub = strsep(&cur, " ");
	if (!sub || kstrtoint(sub, 0, &temp[1]))
		return -1;
	sub = strsep(&cur, " ");
	if (!sub || kstrtoint(sub, 0, &temp[2]))
		return -1;
	if (!cur || kstrtoint(cur, 0, &temp[3]))
		return -1;
	drv_data->result.ic_status = temp[0];
	drv_data->result.key_status = temp[1];
	drv_data->result.sn_status = temp[2];
	drv_data->result.check_mode = temp[3];
	record_final_check_result(&drv_data->result);
	schedule_delayed_work(&drv_data->dmd_report_dw, 5 * HZ);
	result_status = chk_rs_to_rs_stat(&drv_data->result);
	send_result_status(&drv_data->result, result_status);

	return count;
}

static int batt_snprintf_hex_array(char *buf, char *hex, int size)
{
	int i;
	int count = 0;

	for (i = 0; i < size; i++)
		count += snprintf(buf + count, PAGE_SIZE, "%02x", hex[i]);

	return count;
}

static ssize_t batt_param_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int len = 0;
	struct batt_ic_para *ic_para = batt_get_ic_para();
	struct batt_info_para *info = batt_get_info_para();
	struct batt_cert_para *cert = batt_get_cert_para();
	char *nv_sn = batt_get_nv_sn();

	len += snprintf(buf + len, PAGE_SIZE - len, "ic debug\n");
	if (!ic_para) {
		len += snprintf(buf + len, PAGE_SIZE - len, "ic is null\n");
	} else {
		len += snprintf(buf + len, PAGE_SIZE - len,
			"ic type: %d, uid: ", ic_para->ic_type);
		len += batt_snprintf_hex_array(buf + len, ic_para->uid,
			ic_para->uid_len);
		len += snprintf(buf + len, PAGE_SIZE - len, "\n");
	}

	if (!info)
		len += snprintf(buf + len, PAGE_SIZE - len,
			"battery info is null\n");
	else
		len += snprintf(buf + len, PAGE_SIZE - len,
			"battery source: %d, type: %s, sn: %s\n",
			info->source, info->type, info->sn);

	if (!cert) {
		len += snprintf(buf + len, PAGE_SIZE - len, "cert is null\n");
	} else {
		len += snprintf(buf + len, PAGE_SIZE - len,
			"ecce result: %d, sign: ", cert->key_result);
		len += batt_snprintf_hex_array(buf + len, cert->signature,
			cert->sign_len);
		len += snprintf(buf + len, PAGE_SIZE - len, "\n");
	}

	if (!nv_sn)
		len += snprintf(buf + len, PAGE_SIZE - len, "nv_sn is null\n");
	else
		len += snprintf(buf + len, PAGE_SIZE - len, "nv_sn: %s\n",
			nv_sn);

	return len;
}
#endif

static const DEVICE_ATTR_RO(ic_status);
static const DEVICE_ATTR_RO(key_status);
static const DEVICE_ATTR_RO(sn_status);
static const DEVICE_ATTR_RO(batt_id);
static const DEVICE_ATTR_RO(nv_sn_good);
static const DEVICE_ATTR_RO(batt_id_v);
static const DEVICE_ATTR_RO(batt_num);
static const DEVICE_ATTR_RO(check_mode);
static const DEVICE_ATTR_RO(official);
static const DEVICE_ATTR_RO(board);
static const DEVICE_ATTR_RO(battery);
static const DEVICE_ATTR_RO(board_runnable);
static const DEVICE_ATTR_RO(battery_ct_shield);
static const DEVICE_ATTR_WO(check_request);
static const DEVICE_ATTR_RO(check_execute_state);
static const DEVICE_ATTR_WO(uevent_data);
static const DEVICE_ATTR_RO(can_check_in_running);
static const DEVICE_ATTR_WO(powerct_error_code);
#ifdef BATTERY_LIMIT_DEBUG
static const DEVICE_ATTR_RO(ftime);
static const DEVICE_ATTR_RO(ctime);
static const DEVICE_ATTR_RW(bind_info);
static const DEVICE_ATTR_RW(bind_uuid);
static const DEVICE_ATTR_WO(check);
static const DEVICE_ATTR_WO(final);
static const DEVICE_ATTR_RW(sn_checker);
static const DEVICE_ATTR_RW(check_result);
static const DEVICE_ATTR_RO(batt_param);
#endif

static const struct attribute *batt_info_attrs[] = {
	&dev_attr_ic_status.attr,
	&dev_attr_key_status.attr,
	&dev_attr_sn_status.attr,
	&dev_attr_batt_id.attr,
	&dev_attr_nv_sn_good.attr,
	&dev_attr_batt_id_v.attr,
	&dev_attr_batt_num.attr,
	&dev_attr_check_mode.attr,
	&dev_attr_official.attr,
	&dev_attr_board.attr,
	&dev_attr_battery.attr,
	&dev_attr_board_runnable.attr,
	&dev_attr_battery_ct_shield.attr,
	&dev_attr_check_request.attr,
	&dev_attr_check_execute_state.attr,
	&dev_attr_uevent_data.attr,
	&dev_attr_can_check_in_running.attr,
	&dev_attr_powerct_error_code.attr,
#ifdef BATTERY_LIMIT_DEBUG
	&dev_attr_ftime.attr,
	&dev_attr_ctime.attr,
	&dev_attr_bind_info.attr,
	&dev_attr_bind_uuid.attr,
	&dev_attr_check.attr,
	&dev_attr_final.attr,
	&dev_attr_sn_checker.attr,
	&dev_attr_check_result.attr,
	&dev_attr_batt_param.attr,
#endif
	NULL, /* sysfs_create_files need last one be NULL */
};

static int batt_info_node_create(struct platform_device *pdev)
{
	if (sysfs_create_files(&pdev->dev.kobj, batt_info_attrs)) {
		hwlog_err("Can't create all expected nodes under %s in %s\n",
			pdev->dev.kobj.name, __func__);
		return -1;
	}

	return 0;
}

#ifdef CONFIG_HUAWEI_DSM
static const int batt_info_dmd_no[] = {
	[DMD_INVALID] = 0,
	[DMD_ROM_ID_ERROR] = DSM_BATTERY_ROM_ID_CERTIFICATION_FAIL,
	[DMD_IC_STATE_ERROR] = DSM_BATTERY_IC_EEPROM_STATE_ERROR,
	[DMD_IC_KEY_ERROR] = DSM_BATTERY_IC_KEY_CERTIFICATION_FAIL,
	[DMD_OO_UNMATCH] = DSM_OLD_BOARD_AND_OLD_BATTERY_UNMATCH,
	[DMD_OBD_UNMATCH] = DSM_OLD_BOARD_AND_NEW_BATTERY_UNMATCH,
	[DMD_OBT_UNMATCH] = DSM_NEW_BOARD_AND_OLD_BATTERY_UNMATCH,
	[DMD_NV_ERROR] = DSM_BATTERY_NV_DATA_READ_FAIL,
	[DMD_SERVICE_ERROR] = DSM_CERTIFICATION_SERVICE_IS_NOT_RESPONDING,
	[DMD_UNMATCH_BATTS] = DSM_UNMATCH_BATTERYS,
	[DMD_OLD_NV_SN_ERROR] = DSM_NEW_BOARD_AND_OLD_BATTERY_UNMATCH,
	[DMD_CHECK_PASS] = DSM_NEW_BOARD_AND_OLD_BATTERY_UNMATCH,
};

static const char * const battery_detect_err_str[] = {
	[DMD_INVALID] = "",
	[DMD_ROM_ID_ERROR] = "DSM_BATTERY_IC_STATE_ERROR:\n",
	[DMD_IC_STATE_ERROR] = "DSM_BATTERY_IC_EEPROM_STATE_ERROR:\n",
	[DMD_IC_KEY_ERROR] = "DSM_BATTERY_IC_KEY_CERTIFICATION_FAIL:\n",
	[DMD_OO_UNMATCH] = "DSM_OLD_BOARD_AND_OLD_BATTERY_UNMATCH:\n",
	[DMD_OBD_UNMATCH] = "DSM_OLD_BOARD_AND_NEW_BATTERY_UNMATCH:\n",
	[DMD_OBT_UNMATCH] = "DSM_NEW_BOARD_AND_OLD_BATTERY_UNMATCH:\n",
	[DMD_NV_ERROR] = "DSM_BATTERY_NV_DATA_FAIL:\n",
	[DMD_SERVICE_ERROR] = "DSM_CERTIFICATION_SERVICE_NOT_RESPOND:\n",
	[DMD_UNMATCH_BATTS] = "DSM_MULTI_BATTERY_UMATCH:\n",
	[DMD_OLD_NV_SN_ERROR] = "DMD_OLD_NV_SN_ERROR:\n",
	[DMD_CHECK_PASS] = "DMD_CHECK_PASS:\n",
};

static const char * const check_strategy_str[] = {
	[CHECK_STRATEGY_INVALID] = "",
	[CHECK_STRATEGY_DEBUG] = "DEBUG:",
	[CHECK_STRATEGY_BOOTING] = "BOOT_CHECK:",
	[CHECK_STRATEGY_PERIOD] = "WEEKLY_CHECK:",
};
#endif

int get_dmd_no(unsigned int index)
{
	if (index < ARRAY_SIZE(batt_info_dmd_no))
		return batt_info_dmd_no[index];

	return -1;
}

const char *dmd_no_to_str(unsigned int dmd_no)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(batt_info_dmd_no); i++) {
		if (dmd_no == batt_info_dmd_no[i])
			return battery_detect_err_str[i];
	}

	return NULL;
}

int prepare_dmd_no(struct batt_chk_rslt *result)
{
	hwlog_info("[%s] g_error_code = %d\n", __func__, g_error_code);
	switch (g_error_code) {
	case ERROR_CODE_IS_PROHIBITED_ID:
		return DMD_UNMATCH_BATTS;
	default:
		break;
	}

	switch (result->ic_status) {
	case IC_FAIL_UNMATCH:
	case IC_FAIL_UNKOWN:
		return DMD_ROM_ID_ERROR;
	case IC_FAIL_MEM_STATUS:
		return DMD_IC_STATE_ERROR;
	case IC_PASS:
		break;
	default:
		hwlog_err("illegal IC checking result %d\n", result->ic_status);
		break;
	}

	switch (result->key_status) {
	case KEY_FAIL_TIMEOUT:
		return DMD_SERVICE_ERROR;
	case KEY_FAIL_UNMATCH:
		return DMD_IC_KEY_ERROR;
	case KEY_FAIL_ACT:
		return DMD_IC_STATE_ERROR;
	case KEY_PASS:
		break;
	default:
		hwlog_err("illegal KEY checking result %d\n",
			result->key_status);
		break;
	}

	if (g_is_sn_empty) {
		hwlog_info("[%s] sn is empty\n", __func__);
		return DMD_NV_ERROR;
	}

	switch (result->sn_status) {
	case SN_FAIL_NV:
		return DMD_NV_ERROR;
	case SN_FAIL_TIMEOUT:
		return DMD_SERVICE_ERROR;
	case SN_FAIL_IC:
		return DMD_ROM_ID_ERROR;
	case SN_OO_UNMATCH:
		return DMD_OO_UNMATCH;
	case SN_OBD_REMATCH:
		return DMD_OBD_UNMATCH;
	case SN_OBT_REMATCH:
		return DMD_OBT_UNMATCH;
	case SN_SNS_UNMATCH:
		return DMD_UNMATCH_BATTS;
	case SN_NN_REMATCH:
	case SN_PASS:
		break;
	default:
		hwlog_err("illegal SN checking result %d\n", result->sn_status);
		break;
	}
	if (is_legal_result(result))
		return DMD_CHECK_PASS;
	return DMD_INVALID;
}

#define DMD_BUF_SIZE    1023
static const char *is_sn_read(struct batt_chk_data *checker_data)
{
	if ((checker_data->sn != NULL) && (checker_data->sn_len != 0))
		return "Y";

	return "N";
}

static char *prepare_dmd_mesg(struct batt_info *info, char *buff,
	int *dmd_no, int check_strategy_no)
{
	struct batt_chk_rslt *result = &info->result;
	struct batt_chk_rslt *last_result = &info->last_result;
	struct batt_checker_entry *temp = NULL;
	struct batt_chk_data *checker_data = NULL;
	struct platform_device *pdev = NULL;
	int count;

	if (buff)
		return buff;
	hwlog_info("final result(ic:%02x, key:%02x, sn:%02x).\n",
		result->ic_status, result->key_status, result->sn_status);
	if (*dmd_no == 0)
		*dmd_no = prepare_dmd_no(result);

	if (*dmd_no) {
		buff = kzalloc(DMD_BUF_SIZE + 1, GFP_KERNEL);
		if (!buff)
			return NULL;
		count = 0;
		count += snprintf(buff + count, DMD_BUF_SIZE - count,
			check_strategy_str[check_strategy_no]);
		count += snprintf(buff + count, DMD_BUF_SIZE - count,
			"dmd_index: %d ", *dmd_no);
		count += snprintf(buff + count, DMD_BUF_SIZE - count,
			battery_detect_err_str[*dmd_no]);
		list_for_each_entry(temp, &batt_checkers_head, node) {
			pdev = temp->pdev;
			checker_data = platform_get_drvdata(pdev);
			count += snprintf(buff + count, DMD_BUF_SIZE - count,
				"NO:%d Group:%d ",
				checker_data->id_in_grp,
				checker_data->id_of_grp);
			count += snprintf(buff + count, DMD_BUF_SIZE - count,
				"rlt:ic %d, key %d, sn %d Read:%s\n",
				checker_data->result.ic_status,
				checker_data->result.key_status,
				checker_data->result.sn_status,
				is_sn_read(checker_data));
		}

		count += snprintf(buff + count, DMD_BUF_SIZE - count,
			"last_rslt:ic %d, key %d, sn %d, mode %d, batt_removed %d\n",
			last_result->ic_status,
			last_result->key_status,
			last_result->sn_status,
			last_result->check_mode,
			power_platform_is_battery_removed());
		hwlog_info("dmd_buf:%s\n", buff);
	} else {
		return NULL;
	}
	return buff;
}

/*
 * This is the function checking battery information result
 * and acting safe strategies
 */
static void dmd_report_func(struct work_struct *work)
{
	struct delayed_work *dw = container_of(work, struct delayed_work, work);
	struct batt_info *drv_data =
		container_of(dw, struct batt_info, dmd_report_dw);
	int *dmd_no = &drv_data->dmd_no;
	static char *dmd_buf;

	dmd_buf = prepare_dmd_mesg(drv_data, dmd_buf, dmd_no,
		 drv_data->check_strategy_no);
	if (dmd_buf) {
		if (power_dsm_report_dmd(POWER_DSM_BATTERY_DETECT,
			batt_info_dmd_no[*dmd_no], dmd_buf)) {
			hwlog_err("dmd report failed in %s\n", __func__);
			/* 30: dmd report retry time */
			if (drv_data->dmd_retry++ < 30) {
				schedule_delayed_work(&drv_data->dmd_report_dw,
					/* 2s, 3s */
					(drv_data->dmd_retry / 2 + 3) * HZ);
				return;
			}
		}
		kfree(dmd_buf);
		dmd_buf = NULL;
		return;
	} else if (*dmd_no) {
		hwlog_err("prepare dmd mesg failed in %s\n", __func__);
	}
}

void dmd_record_reporter(struct work_struct *work)
{
	int report_sign = 0;
	struct dmd_record *pos = NULL;
	struct dmd_record *tmp = NULL;

	mutex_lock(&g_dmd_list.lock);
	list_for_each_entry_safe(pos, tmp, &g_dmd_list.dmd_head, node) {
		if (report_sign || power_dsm_report_dmd(pos->dmd_type, pos->dmd_no,
			pos->content)) {
			if (!report_sign)
				hwlog_err("dmd failed in %s\n", __func__);
			mutex_unlock(&g_dmd_list.lock);
			/* 3s */
			schedule_delayed_work(&g_dmd_list.dmd_record_report,
				3 * HZ);
			return;
		}
		hwlog_info("report dmd record %d %d\n", pos->dmd_no,
			pos->content_len);

		list_del_init(&pos->node);
		kfree(pos->content);
		kfree(pos);
		report_sign = 1;
	}
	mutex_unlock(&g_dmd_list.lock);
}

/*
 * dts shield_ct_sign bit 0 is reserved for closing anti-counterfeiting
 * function by patch; bit 1 is used to distinguish whether to close
 * anti-counterfeiting by board ID.
 */
static void init_shield_ct_sign(struct platform_device *pdev)
{
	unsigned int sign = 0;

	if (of_property_read_u32(pdev->dev.of_node, "shield_ct_sign", &sign))
		hwlog_info("%s read shield_ct_sign failed\n", pdev->name);
	else
		hwlog_info("shield_ct_sign is %d\n", sign);

	if (sign & SCT_SIGN_BOARD_BIT_MASK)
		shield_ct_sign = 1;
	else
		shield_ct_sign = 0;
}

static void init_can_check_in_running(struct platform_device *pdev)
{
	struct batt_info *drv_data = platform_get_drvdata(pdev);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), pdev->dev.of_node,
		"can_check_in_running",
		(u32 *)&drv_data->can_check_in_running, 0);
}

static struct batt_info *batt_info_data_init(struct platform_device *pdev)
{
	struct batt_info *drv_data = NULL;
	unsigned int sn_check_type;
	struct device_node *prev = NULL;
	struct device_node *next = NULL;
	int ret;

	/* set up device's driver data now */
	drv_data = devm_kzalloc(&pdev->dev, sizeof(*drv_data), GFP_KERNEL);
	if (!drv_data)
		return NULL;
	platform_set_drvdata(pdev, drv_data);
	if (of_property_read_u32(pdev->dev.of_node, "moved_recheck_logic",
		&drv_data->moved_recheck_logic)) {
		/* 0 change logic or, 1 moved logic */
		drv_data->moved_recheck_logic = 1;
		hwlog_err("set default batt_change_logic and\n");
	}
	ret = of_property_read_u32(pdev->dev.of_node, "sn-check-type",
		&sn_check_type);
	if (ret || (sn_check_type >= ARRAY_SIZE(final_sn_checkers))) {
		hwlog_err("%s read sn_check_type failed\n", pdev->name);
		return NULL;
	}
	ret = of_property_read_u32(pdev->dev.of_node, "sn-version",
		&drv_data->sn_version);
	if (ret || (drv_data->sn_version == ILLEGAL_BIND_VERSION) ||
		(drv_data->sn_version >= LEFT_UNUSED_VERSION)) {
		hwlog_err("%s read sn_version failed\n", pdev->name);
		return NULL;
	}
	drv_data->sn_checker = final_sn_checkers[sn_check_type];
	drv_data->dmd_retry = 0;

	init_shield_ct_sign(pdev);
	INIT_DELAYED_WORK(&drv_data->dmd_report_dw, dmd_report_func);
	INIT_WORK(&drv_data->check_work, check_func);

	spin_lock_init(&drv_data->request_lock);
	init_can_check_in_running(pdev);

	while ((next = of_get_next_available_child(pdev->dev.of_node, prev))) {
		prev = next;
		drv_data->total_checkers++;
	}
	if (drv_data->total_checkers == 0) {
		hwlog_err("no valid checker found under huawei_batt_info\n");
		return NULL;
	}

	g_total_checkers = drv_data->total_checkers;
	return drv_data;
}

static void set_up_static_vars(void)
{
	init_completion(&ct_srv_ready);
	init_completion(&board_info_ready);
	new_board = -1;
	new_battery = -1;

	INIT_DELAYED_WORK(&g_dmd_list.dmd_record_report, dmd_record_reporter);
	INIT_LIST_HEAD(&g_dmd_list.dmd_head);
	mutex_init(&g_dmd_list.lock);
}

static void create_battery_checker_devices(struct platform_device *pdev)
{
	const struct of_device_id *match_tbl = NULL;

	match_tbl = get_battery_checkers_match_table();
	of_platform_populate(pdev->dev.of_node, match_tbl, NULL, &pdev->dev);
}

static int battery_info_probe(struct platform_device *pdev)
{
	struct batt_info *drv_data = NULL;

	if (!pdev)
		return BATTERY_DRIVER_FAIL;
	/* find mode need battery checking */
	update_work_mode();
	/* under recovery mode no further checking */
	if ((work_mode == RECOVERY_MODE) || (work_mode == ERECOVERY_MODE)) {
		hwlog_info("Recovery mode not support now\n");
		return BATTERY_DRIVER_SUCCESS;
	}

	hwlog_info("Battery information driver is going to probing...\n");
	drv_data = batt_info_data_init(pdev);
	if (!drv_data) {
		hwlog_err("battery information driver data init failed\n");
		return BATTERY_DRIVER_FAIL;
	}
	wakeup_source_init(&drv_data->checking_wakelock, pdev->name);
	set_up_static_vars();
	/* init mesg interface(used to communicate with native server) */
	if (batt_mesg_init()) {
		hwlog_err("%s general netlink initialize failed\n", pdev->name);
		goto trash_wakelock;
	}
	/* battery node initialization */
	if (batt_info_node_create(pdev)) {
		hwlog_err("%s battery information nodes create failed\n",
			pdev->name);
		goto trash_wakelock;
	}

	/*
	 * for batt_info compatible not contain "simple-bus"
	 * reason: if batt_info not valid, no battery checker should be valid
	 */
	create_battery_checker_devices(pdev);
	hwlog_info("Battery information driver was probed successfully\n");

	/* start checking work */
	run_check_func(drv_data, CHECK_STRATEGY_BOOTING);

	return BATTERY_DRIVER_SUCCESS;

trash_wakelock:
	wakeup_source_trash(&drv_data->checking_wakelock);
	return BATTERY_DRIVER_FAIL;
}

static int battery_info_remove(struct platform_device *pdev)
{
	struct batt_info *drv_data = platform_get_drvdata(pdev);

	wakeup_source_trash(&drv_data->checking_wakelock);
	return BATTERY_DRIVER_SUCCESS;
}

static const struct of_device_id battery_info_match_table[] = {
	{ .compatible = "huawei,battery-information", },
	{ /* end */ },
};

static struct platform_driver battery_info_driver = {
	.probe = battery_info_probe,
	.remove = battery_info_remove,
	.driver = {
		.name = "Battery_Info",
		.owner = THIS_MODULE,
		.of_match_table = battery_info_match_table,
	},
};

int __init battery_info_init(void)
{
	return platform_driver_register(&battery_info_driver);
}

void __exit battery_info_exit(void)
{
	platform_driver_unregister(&battery_info_driver);
}

subsys_initcall_sync(battery_info_init);
module_exit(battery_info_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery information");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
