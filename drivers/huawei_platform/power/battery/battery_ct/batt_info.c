/*
 * batt_info.c
 *
 * battery authenticate information
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

#define BATTCT_KERNEL_SRV_SHARE_VAR
#include "batt_info.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG batt_info
HWLOG_REGIST();

#define DMD_CONTENT_BASE_LEN 24
#define DMD_REPORT_CONTENT_LEN 256

static LIST_HEAD(batt_checkers_head);
static unsigned int valid_checkers;
static unsigned int shield_ct_sign;
static struct dmd_record_list g_dmd_list;
#ifdef BATTERY_LIMIT_DEBUG
static struct timespec64 finish_time;
#endif

static int prepare_dmd_no(struct batt_chk_rslt *result);
static void dmd_record_reporter(struct work_struct *work);
static int get_dmd_no(unsigned int index);
static const char *dmd_no_to_str(unsigned int dmd_no);

void add_to_batt_checkers_lists(struct batt_checker_entry *entry)
{
	list_add(&entry->node, &batt_checkers_head);
	valid_checkers++;
}

/* A function to implement bubble sort */
void bubbleSort(char arr[], int n)
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
	if (strstr(saved_command_line, "enter_erecovery=1")) {
		work_mode = ERECOVERY_MODE;
		goto print_work_mode;
	}
	if (strstr(saved_command_line, "enter_recovery=1")) {
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
		if (dev_info->id_in_grp == checker_data->id_in_grp &&
		    dev_info->id_of_grp == checker_data->id_of_grp &&
		    dev_info->ic_type == checker_data->ic_type)
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
	struct batt_chk_data *checker_data = NULL;
	struct completion *comp = NULL;
	struct batt_res *res = NULL;
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
	default:
		hwlog_err("unkown subcmd for checker(NO:%d Group:%d)\n",
			  checker_data->id_in_grp, checker_data->id_of_grp);
		return;
	}
	/* try to free last res first */
	kfree(res->data);
	res->data = NULL;
	res->len = 0;
	if (len <= MAX_DATA_LEN && len > 0) {
		mesg_data = kmalloc(len, GFP_KERNEL);
		if (mesg_data == NULL)
			return;
		memcpy(mesg_data, data, len);
		res->data = mesg_data;
		res->len = len;
		complete(comp);
	} else {
		hwlog_err("subcmd get too large data length(%d)\n", len);
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

	dev_attr = info->attrs[BATT_INFO_DEVICE_ATTR];
	if (!dev_attr) {
		hwlog_err("dev_attr is null found in %s\n", __func__);
		return -1;
	}
	data_attr = info->attrs[BATT_INFO_DATA_ATTR];
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

	batt_info_mesg_data_processor(pdev, dev_info->subcmd,
				      nla_data(data_attr), nla_len(data_attr));

	return 0;
}

static char new_board;
static struct completion board_info_ready;

static int board_info_cb(unsigned char version, void *data, int len)
{
	hwlog_info("board information going to process\n");
	if (len != strlen(board_info_cb_mesg[NEW_BOARD_MESG_INDEX])) {
		hwlog_err("board info callback mesg len(%d) illegal\n", len);
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
			count += snprintf(dmd_content + count,
					  DMD_REPORT_CONTENT_LEN - count,
					  "NO:%d Group:%d ",
					  value[0], value[1]);
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
		hwlog_err("dmd info call back mesg length(%d) illegal\n", len);
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
		hwlog_err("dmd info call back mesg length(%d:%d) illegal\n",
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

static const struct power_mesg_cbs batt_cbs[BATT_INFO_NL_CBS_NUM] = {
	{
		.cmd = BATT_INFO_CMD,
		.doit = batt_info_cb,
	},
};

#define BOARD_INFO_NL_OPS_NUM    3
static const struct power_mesg_easy_cbs batt_ops[BOARD_INFO_NL_OPS_NUM] = {
	{
		.cmd = BOARD_INFO_CMD,
		.doit = board_info_cb,
	},
	{
		.cmd = BATT_FINAL_RESULT_CMD,
		.doit = nop_cb,
	},
	{
		.cmd = BATT_DMD_CMD,
		.doit = batt_dmd_cb,
	},
};

/* BATT_SERVICE_ON handler */
static int batt_srv_on_cb(void)
{
	complete(&ct_srv_ready);

	return 0;
}

static struct power_mesg_node batt_info_genl_node = {
	.target = POWERCT_PORT,
	.name = "BATT_INFO",
	.ops = batt_ops,
	.n_ops = BOARD_INFO_NL_OPS_NUM,
	.cbs = batt_cbs,
	.n_cbs = BATT_INFO_NL_CBS_NUM,
	.srv_on_cb = batt_srv_on_cb,
};

static int batt_mesg_init(void)
{
	int ret;

	ret = power_genl_node_register(&batt_info_genl_node);
	if (ret)
		hwlog_err("power_genl_add_op failed(%d)!\n", ret);

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

static inline void prepare_dmd_record(int dmd_index, struct dmd_record *record)
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

static inline int copy_record_to_buf(char *buf, struct dmd_record *record)
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

	if (power_genl_easy_send(&batt_info_genl_node, BATT_FINAL_RESULT_CMD,
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
	int dmd_index = DMD_INVALID;

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

	if (power_genl_easy_send(&batt_info_genl_node, BOARD_INFO_CMD, 0,
				 (void *)mesg, strlen(mesg))) {
		hwlog_err("send board information message failed\n");
		return -1;
	}

	return 0;
}

int send_batt_info_mesg(struct nl_dev_info *info, void *data, unsigned int len)
{
	struct batt_res res[TOTAL_POWER_GENL_ATTR];

	res[0].data = (const unsigned char *)info;
	res[0].len = sizeof(*info);
	res[0].type = BATT_INFO_DEVICE_ATTR;

	res[1].data = (const unsigned char *)data;
	res[1].len = len;
	res[1].type = BATT_INFO_DATA_ATTR;

	if (power_genl_send_attrs(&batt_info_genl_node, BATT_INFO_CMD, 0, res,
				  ARRAY_SIZE(res))) {
		hwlog_err("Send attributes failed in %s\n", __func__);
		return -1;
	}

	return 0;
}

#define LAST_RESULT_NV_NUMBER                       388
#define LAST_RESULT_NV_NAME                         "BLIMSW"
#define BBINFO_NV_NUMBER                            389
#define BBINFO_NV_NAME                              "BBINFO"
static char new_battery;

static int write_nv(uint32_t nv_number, const char *nv_name, const void *data,
	uint32_t data_len)
{
	struct hisi_nve_info_user nvinfo;
	int ret;

	if (data_len > NVE_NV_DATA_SIZE) {
		hwlog_err("data length(%u) is too big to write into NV\n",
			  data_len);
		return -1;
	}
	nvinfo.nv_operation = NV_WRITE;
	nvinfo.nv_number = nv_number;
	strlcpy(nvinfo.nv_name, nv_name, NV_NAME_LENGTH);
	nvinfo.valid_size = data_len;
	memcpy(&nvinfo.nv_data, data, data_len);
	ret = hisi_nve_direct_access(&nvinfo);
	if (ret) {
		hwlog_err("Write NV(%s) failed(%d).\n", nv_name, ret);
		return -1;
	}

	return 0;
}

static int read_nv(uint32_t nv_number, const char *nv_name, void *data,
		   uint32_t data_len)
{
	struct hisi_nve_info_user nvinfo;
	int ret;

	if (data_len > NVE_NV_DATA_SIZE) {
		hwlog_err("data length(%u) is too big to read from NV\n",
			  data_len);
		return -1;
	}
	nvinfo.nv_operation = NV_READ;
	nvinfo.nv_number = nv_number;
	strncpy(nvinfo.nv_name, nv_name, NV_NAME_LENGTH - 1);
	nvinfo.valid_size = data_len;
	ret = hisi_nve_direct_access(&nvinfo);
	if (ret) {
		hwlog_err("Read NV(%s) failed(%d).\n", nv_name, ret);
		return -1;
	}
	memcpy(data, &nvinfo.nv_data, data_len);

	return 0;
}

static int get_last_check_result(struct batt_chk_rslt *result)
{
	if (read_nv(LAST_RESULT_NV_NUMBER, LAST_RESULT_NV_NAME, result,
		    sizeof(*result))) {
		hwlog_err("get last check result failed\n");
		return -1;
	}

	return 0;
}

static void record_final_check_result(struct batt_chk_rslt *result)
{
	if (write_nv(LAST_RESULT_NV_NUMBER, LAST_RESULT_NV_NAME, result,
		     sizeof(*result)))
		hwlog_err("record final check result failed\n");
}

static void get_sn_from_nv(struct batt_info *drv_data)
{
	struct binded_info bbinfo;
	struct batt_chk_rslt result;

	drv_data->sn = drv_data->sn_buff;
	if (read_nv(BBINFO_NV_NUMBER, BBINFO_NV_NAME, &bbinfo,
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
	if (write_nv(BBINFO_NV_NUMBER, BBINFO_NV_NAME, bbinfo,
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
	if (result->key_status == KEY_FAIL_TIMEOUT ||
	    result->sn_status == SN_FAIL_TIMEOUT ||
	    result->sn_status == SN_FAIL_NV)
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

	if (read_nv(BBINFO_NV_NUMBER, BBINFO_NV_NAME, &bbinfo,
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
		hwlog_err("Unkown BBINFO NV version(%u) found in %s\n",
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
	if (drv_data->result.sn_status == SN_OBT_REMATCH ||
	    drv_data->result.sn_status == SN_OBD_REMATCH ||
	    drv_data->result.sn_status == SN_NN_REMATCH) {
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

/* 0 is match, -1 isn't match */
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
		hwlog_err("too much(%u) valid battery checkers\n",
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
			return -1;
		}
		if (!sn && sn_len < 0) {
			sn = checker_data->sn;
			sn_len = checker_data->sn_len;
			buf[i++] = sn[sn_len - 1];
			continue;
		}
		if (memcmp(sn, checker_data->sn, sn_len - 1) ||
		    sn_len != checker_data->sn_len) {
			drv_data->result.sn_status = SN_SNS_UNMATCH;
			kfree(buf);
			hwlog_err("sn body match fail\n");
			return -1;
		}
		buf[i++] = checker_data->sn[sn_len - 1];
	}
	bubbleSort(buf, i);
	for (j = 0; j < i - 1; j++) {
		if (buf[j] == buf[j + 1]) {
			drv_data->result.sn_status = SN_SNS_UNMATCH;
			kfree(buf);
			hwlog_err("sn no match fail\n");
			return -1;
		}
	}
	kfree(buf);

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

static void check_func(struct work_struct *work)
{
	int ret;
	enum result_stat result_status = FINAL_RESULT_FAIL;
	struct batt_chk_rslt last_result;
	struct batt_chk_rslt dummy_result;
	struct batt_chk_rslt *final_result = NULL;
	struct batt_chk_data *checker_data = NULL;
	struct batt_info *drv_data = NULL;
	struct batt_checker_entry *temp = NULL;

	hwlog_info("battery checking started in %s\n", __func__);
	drv_data = container_of(work, struct batt_info, check_work);
	final_result = &drv_data->result;
	/* wait for service powerct ready */
	wait_for_completion(&ct_srv_ready);

	/* check last result */
	ret = get_last_check_result(&last_result);
	if (ret)
		hwlog_err("get last check result failed\n");
	else
		hwlog_info("last result(ic(%d) key(%d) sn(%d) mode(%d)).\n",
			   last_result.ic_status, last_result.key_status,
			   last_result.sn_status, last_result.check_mode);

	if (shield_ct_sign) {
		dummy_result.check_mode = 0;
		dummy_result.ic_status = IC_PASS;
		dummy_result.key_status = KEY_PASS;
		dummy_result.sn_status = SN_PASS;
		*final_result = dummy_result;
		result_status = FINAL_RESULT_PASS;
		hwlog_info("battery ct shielding to dummy result\n");
		goto process_result_status;
	}
	/* Follow conditions will trigger real battery check
	 * 1.get last check result failed
	 * 2.last check result is illegal
	 * 3.kernel is factory version
	 * 4.battery is removed before boot
	 */
	if (!(ret || !is_legal_result(&last_result) ||
	      last_result.check_mode == FACTORY_CHECK_MODE ||
	      hisi_battery_removed_before_boot())) {
		*final_result = last_result;
		result_status = FINAL_RESULT_PASS;
		if (drv_data->sn_checker != final_sn_checker_nop)
			get_sn_from_nv(drv_data);
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

	hwlog_info("getting borad status started in %s\n", __func__);
	/* board on boot new or old? */
	if (send_board_info()) {
		hwlog_err("Checking stopped by send get board info failed\n");
		return;
	}
	wait_for_completion(&board_info_ready);

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
		if (!ret) {
			final_result->sn_status =
			    checker_data->result.sn_status;
			hwlog_info("batt(NO:%d Group:%d) sn status illegal\n",
				   checker_data->id_in_grp,
				   checker_data->id_of_grp);
			goto process_result_status;
		}
	}

	/* sn final check */
	if (drv_data->sn_checker(drv_data)) {
		hwlog_err("final sn check failed\n");
		return;
	}

process_result_status:
	if (memcmp(&last_result, final_result, sizeof(struct batt_chk_rslt)))
		record_final_check_result(final_result);
	schedule_delayed_work(&drv_data->dmd_report_dw, 5 * HZ);
	result_status = chk_rs_to_rs_stat(final_result);
	send_result_status(final_result, result_status);

skip_result_status:
#ifdef BATTERY_LIMIT_DEBUG
	finish_time = ktime_to_timespec(ktime_get_boottime());
#endif
	if (result_status != FINAL_RESULT_FAIL)
		power_down_ics();
}

static ssize_t batt_id_show(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	struct batt_info *drv_data = NULL;
	struct batt_checker_entry *entry = NULL;
	ssize_t len = 0;
	char i = '1';

	DEV_GET_DRVDATA(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");
	if (!drv_data->sn) {
		len = strlen("IC ERROR");
		memcpy(buf, "IC ERROR", len);
		return len;
	}
	list_for_each_entry(entry, &batt_checkers_head, node) {
		memcpy(buf + len, drv_data->sn, drv_data->sn_len);
		buf[drv_data->sn_len + len] = i++;
		buf[drv_data->sn_len + len + 1] = '\n';
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

	DEV_GET_DRVDATA(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");

	return snprintf(buf, PAGE_SIZE, "%d", drv_data->sn_version);
}

static ssize_t batt_num_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct batt_info *drv_data = NULL;

	DEV_GET_DRVDATA(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");

	return snprintf(buf, PAGE_SIZE, "%d", drv_data->total_checkers);
}

static ssize_t ic_status_show(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	struct batt_info *drv_data = NULL;

	DEV_GET_DRVDATA(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");
	if (ic_status_legal(drv_data->result.ic_status))
		return snprintf(buf, PAGE_SIZE, "PASS");
	else
		return snprintf(buf, PAGE_SIZE, "FAIL(%u)",
				drv_data->result.ic_status);
}

static ssize_t key_status_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct batt_info *drv_data = NULL;

	DEV_GET_DRVDATA(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");
	if (key_status_legal(drv_data->result.key_status))
		return snprintf(buf, PAGE_SIZE, "PASS");
	else
		return snprintf(buf, PAGE_SIZE, "FAIL(%u)",
				drv_data->result.key_status);
}

static ssize_t sn_status_show(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	struct batt_info *drv_data = NULL;

	DEV_GET_DRVDATA(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error driver data");
	if (sn_status_legal(drv_data->result.sn_status))
		return snprintf(buf, PAGE_SIZE, "PASS");
	else
		return snprintf(buf, PAGE_SIZE, "FAIL(%u)",
				drv_data->result.sn_status);
}

static ssize_t check_mode_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct batt_info *drv_data = NULL;

	DEV_GET_DRVDATA(drv_data, dev);
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

	DEV_GET_DRVDATA(drv_data, dev);
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
	struct batt_chk_data *checker_data = NULL;
	struct batt_checker_entry *temp = NULL;

	if (new_battery < 0) {
		list_for_each_entry(temp, &batt_checkers_head, node) {
			checker_data = platform_get_drvdata(temp->pdev);
			if (checker_data->batt_rematch_onboot ==
			    BATTERY_UNREMATCHABLE) {
				new_battery = BATTERY_UNREMATCHABLE;
				break;
			}
		}
	}

	return snprintf(buf, PAGE_SIZE, "%s", new_battery ? "New" : "Old");
}

#ifdef BATTERY_LIMIT_DEBUG
static ssize_t ftime_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%llu.%06llu",
			(unsigned long long)finish_time.tv_sec,
			(unsigned long long)finish_time.tv_nsec / 1000);
}

static ssize_t ctime_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	struct timespec ts;

	ts = ktime_to_timespec(ktime_get_boottime());

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

	if (read_nv(BBINFO_NV_NUMBER, BBINFO_NV_NAME, &bbinfo, sizeof(bbinfo)))
		return snprintf(buf, PAGE_SIZE, "Error:Read NV Fail");
	for (i = 0; i < MAX_SN_BUFF_LENGTH; i++) {
		memcpy(buf + count, bbinfo.info[i], MAX_SN_LEN);
		count += MAX_SN_LEN;
		buf[count++] = '\n';
	}

	return count;
}

static ssize_t check_store(struct device *dev, struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct batt_info *drv_data = NULL;

	DEV_GET_DRVDATA(drv_data, dev);
	if (!drv_data)
		return -1;
	if (count >= 5 && !memcmp(buf, "check", 5))
		schedule_work(&drv_data->check_work);

	return count;
}

static ssize_t final_store(struct device *dev, struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct batt_info *drv_data = NULL;
	int temp;

	DEV_GET_DRVDATA(drv_data, dev);
	if (!drv_data)
		return -1;
	if (count >= 6 && !memcmp(buf, "final", 5)) {
		temp = buf[6] - '0';
		if (temp >= 0 && temp < __FINAL_RESULT_MAX)
			send_result_status(NULL, (enum result_stat)temp);
	}

	return count;
}

static ssize_t sn_checker_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct batt_info *drv_data = NULL;

	DEV_GET_DRVDATA(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");

	return snprintf(buf, PAGE_SIZE, "%pf", drv_data->sn_checker);
}

static ssize_t sn_checker_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct batt_info *drv_data = NULL;
	int temp;

	DEV_GET_DRVDATA(drv_data, dev);
	if (!drv_data)
		return -1;
	if (count >= 5 && !memcmp(buf, "type", 4)) {
		temp = buf[4] - '0';
		if (temp >= 0 && temp < ARRAY_SIZE(final_sn_checkers))
			drv_data->sn_checker = final_sn_checkers[temp];
	}

	return count;
}

static ssize_t check_result_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct batt_info *drv_data = NULL;

	DEV_GET_DRVDATA(drv_data, dev);
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
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	enum result_stat result_status;
	int temp[4];
	char str[CHECK_RESUT_STR_SIZE] = { 0 };
	size_t len;
	struct batt_info *drv_data = NULL;
	char *sub = NULL;
	char *cur = NULL;

	DEV_GET_DRVDATA(drv_data, dev);
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
#endif

static const DEVICE_ATTR_RO(ic_status);
static const DEVICE_ATTR_RO(key_status);
static const DEVICE_ATTR_RO(sn_status);
static const DEVICE_ATTR_RO(batt_id);
static const DEVICE_ATTR_RO(batt_id_v);
static const DEVICE_ATTR_RO(batt_num);
static const DEVICE_ATTR_RO(check_mode);
static const DEVICE_ATTR_RO(official);
static const DEVICE_ATTR_RO(board);
static const DEVICE_ATTR_RO(battery);
#ifdef BATTERY_LIMIT_DEBUG
static const DEVICE_ATTR_RO(ftime);
static const DEVICE_ATTR_RO(ctime);
static const DEVICE_ATTR_RO(bind_info);
static const DEVICE_ATTR_WO(check);
static const DEVICE_ATTR_WO(final);
static const DEVICE_ATTR_RW(sn_checker);
static const DEVICE_ATTR_RW(check_result);
#endif

static const struct attribute *batt_info_attrs[] = {
	&dev_attr_ic_status.attr,
	&dev_attr_key_status.attr,
	&dev_attr_sn_status.attr,
	&dev_attr_batt_id.attr,
	&dev_attr_batt_id_v.attr,
	&dev_attr_batt_num.attr,
	&dev_attr_check_mode.attr,
	&dev_attr_official.attr,
	&dev_attr_board.attr,
	&dev_attr_battery.attr,
#ifdef BATTERY_LIMIT_DEBUG
	&dev_attr_ftime.attr,
	&dev_attr_ctime.attr,
	&dev_attr_bind_info.attr,
	&dev_attr_check.attr,
	&dev_attr_final.attr,
	&dev_attr_sn_checker.attr,
	&dev_attr_check_result.attr,
#endif
	NULL,			/* sysfs_create_files need last one be NULL */
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
	switch (result->ic_status) {
	case IC_FAIL_UNMATCH:
	case IC_FAIL_UNKOWN:
		return DMD_ROM_ID_ERROR;
	case IC_FAIL_MEM_STATUS:
		return DMD_IC_STATE_ERROR;
	case IC_PASS:
		break;
	default:
		hwlog_err("illegal IC checking result(%d)\n",
			  result->ic_status);
		break;
	}

	switch (result->key_status) {
	case KEY_FAIL_TIMEOUT:
		return DMD_SERVICE_ERROR;
	case KEY_FAIL_UNMATCH:
		return DMD_IC_KEY_ERROR;
	case KEY_PASS:
		break;
	default:
		hwlog_err("illegal KEY checking result(%d)\n",
			  result->key_status);
		break;
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
		hwlog_err("illegal SN checking result(%d)\n",
			  result->sn_status);
		break;
	}

	return DMD_INVALID;
}

#define DMD_BUF_SIZE    1023
static const char *is_sn_read(struct batt_chk_data *checker_data)
{
	if (checker_data->sn != NULL && checker_data->sn_len != 0)
		return "Y";

	return "N";
}

static char *prepare_dmd_mesg(struct batt_chk_rslt *result, char *buff,
			      int *dmd_no)
{
	struct batt_checker_entry *temp = NULL;
	struct batt_chk_data *checker_data = NULL;
	struct platform_device *pdev = NULL;
	int count;

	if (buff)
		return buff;
	hwlog_info("final result(ic:%02x, key:%02x, sn:%02x).\n",
		   result->ic_status, result->key_status, result->sn_status);

	*dmd_no = prepare_dmd_no(result);

	if (*dmd_no) {
		buff = kzalloc(DMD_BUF_SIZE + 1, GFP_KERNEL);
		if (!buff)
			return NULL;
		count = 0;
		count += snprintf(buff + count, DMD_BUF_SIZE,
				  battery_detect_err_str[*dmd_no]);
		list_for_each_entry(temp, &batt_checkers_head, node) {
			pdev = temp->pdev;
			checker_data = platform_get_drvdata(pdev);
			count += snprintf(buff + count, DMD_BUF_SIZE - count,
					  "NO:%d Group:%d ",
					  checker_data->id_in_grp,
					  checker_data->id_of_grp);
			count += snprintf(buff + count, DMD_BUF_SIZE - count,
					  "rlt:ic(%d) key(%d) sn(%d Read:%s)\n",
					  checker_data->result.ic_status,
					  checker_data->result.key_status,
					  checker_data->result.sn_status,
					  is_sn_read(checker_data));
		}
	} else
		return NULL;

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
	struct batt_chk_rslt *result = &drv_data->result;
	int *dmd_no = &drv_data->dmd_no;
	static char *dmd_buf;

	dmd_buf = prepare_dmd_mesg(result, dmd_buf, dmd_no);
	if (dmd_buf) {
		if (power_dsm_dmd_report(POWER_DSM_BATTERY_DETECT,
					 batt_info_dmd_no[*dmd_no], dmd_buf)) {
			hwlog_err("dmd report failed in %s\n", __func__);
			if (drv_data->dmd_retry++ < 30) {
				schedule_delayed_work(&drv_data->dmd_report_dw,
						      (drv_data->dmd_retry / 2 +
						       3) * HZ);
			}
			return;
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
		if (report_sign ||
		    power_dsm_dmd_report(pos->dmd_type, pos->dmd_no,
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

static struct batt_info *batt_info_data_init(struct platform_device *pdev)
{
	struct batt_info *drv_data = NULL;
	unsigned int sn_check_type;
	struct device_node *prev = NULL;
	struct device_node *next = NULL;
	int ret;

	/*set up device's driver data now */
	drv_data = devm_kzalloc(&pdev->dev, sizeof(*drv_data), GFP_KERNEL);
	if (!drv_data)
		return NULL;
	platform_set_drvdata(pdev, drv_data);

	ret = of_property_read_u32(pdev->dev.of_node, "sn-check-type",
				   &sn_check_type);
	if (ret || sn_check_type >= ARRAY_SIZE(final_sn_checkers)) {
		hwlog_err("%s read sn_check_type failed\n", pdev->name);
		return NULL;
	}
	ret = of_property_read_u32(pdev->dev.of_node, "sn-version",
				   &drv_data->sn_version);
	if (ret || drv_data->sn_version == ILLEGAL_BIND_VERSION ||
	    drv_data->sn_version >= LEFT_UNUSED_VERSION) {
		hwlog_err("%s read sn_version failed\n", pdev->name);
		return NULL;
	}
	drv_data->sn_checker = final_sn_checkers[sn_check_type];
	drv_data->dmd_retry = 0;

	init_shield_ct_sign(pdev);
	INIT_DELAYED_WORK(&drv_data->dmd_report_dw, dmd_report_func);
	INIT_WORK(&drv_data->check_work, check_func);
	init_battery_check_result(&drv_data->result);

	while ((next = of_get_next_available_child(pdev->dev.of_node, prev))) {
		prev = next;
		drv_data->total_checkers++;
	}
	if (drv_data->total_checkers == 0) {
		hwlog_err("no valid checker found under huawei_batt_info\n");
		return NULL;
	}

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
	if (work_mode == RECOVERY_MODE || work_mode == ERECOVERY_MODE) {
		hwlog_info("Recovery mode not support now\n");
		return BATTERY_DRIVER_SUCCESS;
	}

	hwlog_info("Battery information driver is going to probing...\n");
	drv_data = batt_info_data_init(pdev);
	if (!drv_data) {
		hwlog_err("battery information driver data init failed\n");
		return BATTERY_DRIVER_FAIL;
	}
	set_up_static_vars();
	/* init mesg interface(used to communicate with native server) */
	if (batt_mesg_init()) {
		hwlog_err("%s general netlink initialize failed\n", pdev->name);
		return BATTERY_DRIVER_FAIL;
	}
	/* battery node initialization */
	if (batt_info_node_create(pdev)) {
		hwlog_err("%s battery information nodes create failed\n",
			  pdev->name);
		return BATTERY_DRIVER_FAIL;
	}

	/* start checking work */
	schedule_work(&drv_data->check_work);

	/*
	 * for batt_info compatible not contain "simple-bus"
	 * reason: if batt_info not valid, no battery checker should be valid
	 */
	create_battery_checker_devices(pdev);
	hwlog_info("Battery information driver was probed successfully\n");

	return BATTERY_DRIVER_SUCCESS;
}

static int battery_info_remove(struct platform_device *pdev)
{
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
