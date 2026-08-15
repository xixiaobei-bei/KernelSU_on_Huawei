/*
 * ds28el15.c
 *
 * this is for ic ds28el15
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

#include <huawei_platform/power/batt_info_pub.h>

#include "ds28el15.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG ds28el15
HWLOG_REGIST();

#define INFO_BLOCK0                        MAXIM_BLOCK0
#define INFO_BLOCK1                        MAXIM_BLOCK1
#define BATT_ID_BLOCK                      MAXIM_BLOCK2
#define BATT_PCB_BLOCK                     MAXIM_BLOCK3

enum debug_com_t {
	DEBUG_ROM_ID = 0,
	DEBUG_PERSONALITY,
	DEBUG_GET_EEPROM,
	DEBUG_SET_EEPROM,
	DEBUG_GET_SRAM,
	DEBUG_SET_SRAM,
	DEBUG_GET_MAC,
	DEBUG_EEPROM_STATUS,
	DEBUG_MAX_NUM,
};

static struct ds28el15_des ds28el15;

/* random 32 bytes for sram */
static unsigned char *random_bytes;

static unsigned char mac_datum[MAX_MAC_SOURCE_SIZE];

static struct batt_ct_ops_list ds28el15_ct_node;

/* number of all kinds of communication errors */
static unsigned int err_num[__MAX_COM_ERR_NUM];

/* lock for read & write battery safety information */
static DEFINE_MUTEX(batt_safe_info_lock);

/* battery constraints */
static struct battery_constraint batt_cons;

static char sn_printable[SN_CHAR_PRINT_SIZE + SN_HEX_PRINT_SIZE + 1];

static char batt_type[BATTERY_TYPE_BUFF_SIZE];

#ifdef ONEWIRE_STABILITY_DEBUG
static unsigned int com_err[DEBUG_MAX_NUM];
static unsigned int total_com[DEBUG_MAX_NUM];
#endif

static void set_sched_affinity_to_current(void)
{
	long retval;
	int current_cpu;

	preempt_disable();
	current_cpu = smp_processor_id();
	preempt_enable();
	retval = sched_setaffinity(CURRENT_DS28EL15_TASK,
				   cpumask_of(current_cpu));
	if (retval)
		hwlog_info("Set cpu af to current cpu failed(%ld) in %s\n",
			   retval, __func__);
	else
		hwlog_info("Set cpu af to current cpu(%d) in %s\n", current_cpu,
			   __func__);
}

static void set_sched_affinity_to_all(void)
{
	long retval;
	cpumask_t dstp;

	cpumask_setall(&dstp);
	retval = sched_setaffinity(CURRENT_DS28EL15_TASK, &dstp);
	if (retval)
		hwlog_info("Set cpu af to all valid cpus failed(%ld) in %s\n",
			   retval, __func__);
	else
		hwlog_info("Set cpu af to all valid cpus in %s\n", __func__);
}

static int ds28el15_check_ic_status(struct platform_device *pdev)
{
	int ret;
	int i;
	struct maxim_ow_mem *mom = &ds28el15.ic_des.memory;
	struct maxim_mem_ops *mops = &ds28el15.ic_des.mem_ops;
	char submem_cr[DS28EL15_INFO_BLOCK_NUM] = { 0 };

	i = 0;
	while (!(mom->validity_rom_id)) {
		hwlog_info("ds28el15 read rom id communication start\n");
		ret = mops->get_rom_id(&ds28el15.ic_des);
		DS28EL15_COMMUNICATION_INFO(ret, "get_rom_id");
		i++;
		if (ret)
			err_num[GET_ROM_ID_INDEX]++;
		if (i >= GET_ROM_ID_RETRY)
			break;
	}
	if (!(mom->validity_rom_id)) {
		hwlog_err("get rom id failed\n");
		return DS28EL15_FAIL;
	}

	for (i = 0; i < mom->rom_id_len; i++)
		batt_cons.id_chk[i] = batt_cons.id_example[i] ^ mom->rom_id[i];
	for (i = 0; i < mom->rom_id_len; i++)
		batt_cons.id_chk[i] &= batt_cons.id_mask[i];
	for (i = 0; i < mom->rom_id_len; i++) {
		if (batt_cons.id_chk[i]) {
			hwlog_err("IC id was unmatched at %dth byte\n", i);
			return DS28EL15_FAIL;
		}
	}

	for (i = 0; i < GET_BLOCK_STATUS_RETRY;) {
		if (!(mom->validity_status)) {
			hwlog_info("read block status communication start\n");
			i++;
			ret = mops->get_status(&ds28el15.ic_des);
			DS28EL15_COMMUNICATION_INFO(ret, "get_status");
		}
		if (mom->validity_status)
			break;
		err_num[GET_BLOCK_STATUS_INDEX]++;
	}
	if (!(mom->validity_status))
		return DS28EL15_FAIL;

	mom->block_status[INFO_BLOCK0] &= DS28EL15_PROT_MASK;
	mom->block_status[INFO_BLOCK1] &= DS28EL15_PROT_MASK;
	mom->block_status[BATT_ID_BLOCK] &= DS28EL15_PROT_MASK;
	mom->block_status[BATT_PCB_BLOCK] &= DS28EL15_PROT_MASK;

	if (mom->block_status[INFO_BLOCK0] != DS28EL15_AUT_PROT &&
	    mom->block_status[INFO_BLOCK0] != DS28EL15_EMPTY_PROT) {
		hwlog_err("Information block0 status was wrong(%02X)\n",
			  mom->block_status[INFO_BLOCK0]);
		submem_cr[0] = DS28EL15_FAIL;
	} else {
		hwlog_info("Information block0 status was %02X\n",
			   mom->block_status[INFO_BLOCK0]);
	}
	if (mom->block_status[INFO_BLOCK1] != DS28EL15_AUT_PROT &&
	    mom->block_status[INFO_BLOCK1] != DS28EL15_EMPTY_PROT) {
		hwlog_err("Information block1 status was wrong(%02X)\n",
			  mom->block_status[INFO_BLOCK1]);
		submem_cr[1] = DS28EL15_FAIL;
	} else {
		hwlog_info("Information block1 status was %02X\n",
			   mom->block_status[INFO_BLOCK1]);
	}
	if (mom->block_status[BATT_ID_BLOCK] !=
	    DS28EL15_EMPTY_PROT &&
	    mom->block_status[BATT_ID_BLOCK] != DS28EL15_WRT_PROT) {
		hwlog_err("Battery vendor block status was wrong(%02X)\n",
			  mom->block_status[BATT_ID_BLOCK]);
		submem_cr[2] = DS28EL15_FAIL;
	} else {
		hwlog_info("Battery vendor block status was %02X\n",
			   mom->block_status[BATT_ID_BLOCK]);
	}
	if (mom->block_status[BATT_PCB_BLOCK] != DS28EL15_EMPTY_PROT &&
	    mom->block_status[BATT_PCB_BLOCK] != DS28EL15_WRT_PROT) {
		hwlog_err("pcb vendor block status was %02X\n",
			  mom->block_status[BATT_PCB_BLOCK]);
		submem_cr[3] = DS28EL15_FAIL;
	} else {
		hwlog_info("pcb vendor block status was %02X\n",
			   mom->block_status[BATT_PCB_BLOCK]);
	}

	for (i = 0; i < DS28EL15_INFO_BLOCK_NUM; i++) {
		if (submem_cr[i] == DS28EL15_FAIL)
			return DS28EL15_FAIL;
	}

	return DS28EL15_SUCCESS;
}

static int ds28el15_certification(struct platform_device *pdev,
				  struct batt_res *res, enum key_cr *result)
{
	int ret;
	int i, j;
	int page = MAXIM_PAGE1;
	const unsigned char *data = NULL;
	unsigned int data_len;
	struct maxim_mem_ops *mops = &ds28el15.ic_des.mem_ops;
	struct maxim_ow_mem *mom = &ds28el15.ic_des.memory;

	if (!result || !res)
		return DS28EL15_FAIL;
	data = res->data;
	data_len = res->len;
	if (!data) {
		hwlog_err("NULL point(data) found in %s\n", __func__);
		return DS28EL15_FAIL;
	}
	if (data_len != mom->mac_length) {
		hwlog_err("cert data length(%d) not correct\n", data_len);
		return DS28EL15_FAIL;
	}

	set_sched_affinity_to_current();
	for (j = 0; j < GET_MAC_RETRY; j++) {
		if (!(mom->validity_mac)) {
			for (i = 0; i < SET_SRAM_RETRY; i++) {
				if (!(mom->validity_sram)) {
					ret = mops->set_sram(&ds28el15.ic_des,
							     random_bytes);
					DS28EL15_COMMUNICATION_INFO(ret,
								    "set_sram");
				}
				if (mom->validity_sram)
					break;
				err_num[SET_SRAM_INDEX]++;
			}

			if (!(mom->validity_sram)) {
				hwlog_info("Set random failed(%d@%d) in %s\n",
					   j, GET_MAC_RETRY, __func__);
				msleep(200); /* wait ic 200ms for next op */
				continue;
			}

			ret = mops->get_mac(&ds28el15.ic_des, !ANONYMOUS_MODE,
					    page);
			DS28EL15_COMMUNICATION_INFO(ret, "get_mac");
		}

		if (mom->validity_mac) {
			if (memcmp(mom->mac, data, data_len)) {
				*result = KEY_FAIL_UNMATCH;
				hwlog_err("mac unmatch\n");
			} else
				*result = KEY_PASS;
			set_sched_affinity_to_all();
			return DS28EL15_SUCCESS;
		}
		err_num[GET_MAC_INDEX]++;
	}
	set_sched_affinity_to_all();
	*result = KEY_FAIL_IC;
	return DS28EL15_SUCCESS;
}

static int ds28el15_get_sn(struct platform_device *pdev, struct batt_res *res,
			   const unsigned char **sn, unsigned int *sn_size)
{
	int ret;
	int i;
	char hex_print;
	char *sn_to_print = NULL;
	static char sn_change_done;
	struct maxim_mem_ops *mops = &ds28el15.ic_des.mem_ops;
	struct maxim_ow_mem *mom = &ds28el15.ic_des.memory;

	if (sn) {
		if (mom->sn_page >= mom->page_number)
			return DS28EL15_FAIL;
		set_sched_affinity_to_current();
		for (i = 0; i < GET_USER_MEMORY_RETRY; i++) {
			if (!mom->validity_page[mom->sn_page]) {
				ret = mops->get_user_memory(&ds28el15.ic_des,
							    mom->sn_page, 0,
							    MAXIM_SEGMENT_NUM);
				DS28EL15_COMMUNICATION_INFO(ret,
							    "get_user_memory");
			}
			if (mom->validity_page[mom->sn_page])
				break;

			else
				err_num[GET_USER_MEMORY_INDEX]++;
		}
		if (!mom->validity_page[mom->sn_page]) {
			hwlog_err("Get page0 failed so get mac stop in %s\n",
				  __func__);
			goto get_sn_err;
		}
		set_sched_affinity_to_all();

		if (!sn_change_done) {
			sn_to_print = mom->user_memory;
			sn_to_print += mom->sn_offset_bits / BYTEBITS;
			sn_to_print += mom->sn_page * MAXIM_PAGE_SIZE;
			for (i = 0; i < SN_CHAR_PRINT_SIZE; i++)
				sn_printable[i] = sn_to_print[i];
			sn_to_print += SN_CHAR_PRINT_SIZE;
			for (i = 0; i < SN_HEX_PRINT_SIZE; i++) {
				if (IS_ODD(i))
					hex_print = (sn_to_print[i / 2] & 0x0f);
				else
					hex_print =
					    ((sn_to_print[i / 2] & 0xf0) >> 4) &
					    0x0f;
				sprintf(sn_printable + i + SN_CHAR_PRINT_SIZE,
					"%X", hex_print);
			}
			sn_change_done = !sn_change_done;
		}
		*sn = sn_printable;
	}
	if (sn_size)
		*sn_size = SN_CHAR_PRINT_SIZE + SN_HEX_PRINT_SIZE;
	return DS28EL15_SUCCESS;

get_sn_err:
	set_sched_affinity_to_all();
	return DS28EL15_FAIL;
}

static int ds28el15_get_batt_type(struct platform_device *pdev,
				  const unsigned char **type,
				  unsigned int *type_len)
{
	const unsigned char *sn = NULL;
	unsigned int sn_size;

	if (!type || !type_len || !pdev) {
		hwlog_err("NULL found in %s\n", __func__);
		return DS28EL15_FAIL;
	}

	if (ds28el15_get_sn(pdev, NULL, &sn, &sn_size)) {
		hwlog_err("get battery sn failed in %s\n", __func__);
		return DS28EL15_FAIL;
	}
	batt_type[0] = sn_printable[BATTERY_PACK_FACTORY];
	batt_type[1] = sn_printable[BATTERY_CELL_FACTORY];
	*type = batt_type;
	*type_len = 2;

	return DS28EL15_SUCCESS;
}

static int ds28el15_prepare(struct platform_device *pdev, enum res_type type,
			    struct batt_res *res)
{
	int ret;
	int i;
	int page;
	int page_offset;
	struct maxim_mem_ops *mops = &ds28el15.ic_des.mem_ops;
	struct maxim_ow_mem *mom = &ds28el15.ic_des.memory;

	if (!res) {
		hwlog_err("NULL found in %s\n", __func__);
		return DS28EL15_FAIL;
	}

	switch (type) {
	case RES_CT:
		page = MAXIM_PAGE1;
		page_offset = page * mom->page_size;
		set_sched_affinity_to_current();
		for (i = 0; i < GET_ROM_ID_RETRY; i++) {
			if (!(mom->validity_rom_id)) {
				ret = mops->get_rom_id(&ds28el15.ic_des);
				DS28EL15_COMMUNICATION_INFO(ret, "get_rom_id");
			}
			if (mom->validity_rom_id)
				break;

			else
				err_num[GET_ROM_ID_INDEX]++;
		}
		if (!(mom->validity_rom_id)) {
			hwlog_err("Stop(get rom id failed) in %s\n", __func__);
			goto get_ct_src_fatal_err;
		}

		for (i = 0; i < GET_PERSONALITY_RETRY; i++) {
			if (!(mom->validity_personality)) {
				ret = mops->get_personality(&ds28el15.ic_des);
				DS28EL15_COMMUNICATION_INFO(ret,
							    "get_personality");
			}
			if (mom->validity_personality)
				break;

			else
				err_num[GET_PERSONALITY_INDEX]++;
		}
		if (!mom->validity_personality) {
			hwlog_err("Stop(get personality failed) in %s\n",
				  __func__);
			goto get_ct_src_fatal_err;
		}

		for (i = 0; i < GET_USER_MEMORY_RETRY; i++) {
			if (!mom->validity_page[page]) {
				ret = mops->get_user_memory(&ds28el15.ic_des,
							    page, 0,
							    MAXIM_SEGMENT_NUM);
				DS28EL15_COMMUNICATION_INFO(ret,
							    "get_user_memory");
			}
			if (mom->validity_page[page])
				break;
			err_num[GET_USER_MEMORY_INDEX]++;
		}
		if (!mom->validity_page[page]) {
			hwlog_err("Stop(get user memory page0 failed) in %s\n",
				  __func__);
			goto get_ct_src_fatal_err;
		}
		set_sched_affinity_to_all();

		memset(mac_datum, 0, MAX_MAC_SOURCE_SIZE);
		memcpy(mac_datum + AUTH_MAC_ROM_ID_OFFSET, mom->rom_id,
		       mom->rom_id_len);
		memcpy(mac_datum + AUTH_MAC_PAGE_OFFSET,
		       mom->user_memory + page_offset, mom->page_size);
		memcpy(mac_datum + AUTH_MAC_SRAM_OFFSET, random_bytes,
		       mom->sram_length);
		mac_datum[AUTH_MAC_PAGE_NUM_OFFSET] = page;
		memcpy(mac_datum + AUTH_MAC_MAN_ID_OFFSET,
		       mom->personality + MAXIM_MAN_ID_OFFSET,
		       MAXIM_MAN_ID_SIZE);
		res->data = mac_datum;
		res->len = DS28EL15_CT_MAC_SIZE;
		return DS28EL15_SUCCESS;
	case RES_SN:
		res->data = NULL;
		res->len = 0;
		return DS28EL15_SUCCESS;
	default:
		hwlog_err("Wrong mac resource type(%ud) requred in %s\n",
			  type, __func__);
		break;
	}

	return DS28EL15_FAIL;

get_ct_src_fatal_err:
	set_sched_affinity_to_all();
	return DS28EL15_FAIL;
}

static int ds28el15_set_batt_safe_info(struct platform_device *pdev,
				       enum batt_safe_info_t type, void *value)
{
	int i;
	struct maxim_ow_mem *mom = &ds28el15.ic_des.memory;
	struct maxim_mem_ops *mops = &ds28el15.ic_des.mem_ops;
	int ret_val = DS28EL15_FAIL;
	int ret;

	mutex_lock(&batt_safe_info_lock);
	wake_lock_timeout(&ds28el15.write_lock, 5 * HZ);
	switch (type) {
	case BATT_MATCH_ABILITY:
		if (!value)
			break;
		if (*(enum batt_match_type *)value == BATTERY_REMATCHABLE)
			break;
		if ((mom->validity_status) &&
		    (mom->block_status[BATT_ID_BLOCK] & DS28EL15_WRT_PROT)) {
			ret_val = DS28EL15_SUCCESS;
			hwlog_info("already write protection\n");
			break;
		}
		hwlog_info("going to set battery to write protection\n");
		set_sched_affinity_to_current();
		for (i = 0; i < SET_BLOCK_STATUS_RETRY; i++) {
			ret = mops->set_status(&ds28el15.ic_des, BATT_ID_BLOCK,
					       DS28EL15_WRT_PROT);
			DS28EL15_COMMUNICATION_INFO(ret, "set_status");
			if (mom->validity_status) {
				mom->block_status[BATT_ID_BLOCK] |=
				    DS28EL15_WRT_PROT;
				ret_val = DS28EL15_SUCCESS;
				break;
			}
			err_num[SET_BLOCK_STATUS_INDEX]++;
		}
		set_sched_affinity_to_all();
		break;
	default:
		hwlog_err("unsupported safety info type in %s\n", __func__);
		break;
	}
	wake_unlock(&ds28el15.write_lock);
	mutex_unlock(&batt_safe_info_lock);

	return ret_val;
}

static int ds28el15_get_batt_safe_info(struct platform_device *pdev,
				       enum batt_safe_info_t type, void *value)
{
	struct maxim_ow_mem *mom = &ds28el15.ic_des.memory;
	struct maxim_mem_ops *mops = &ds28el15.ic_des.mem_ops;
	int retval = DS28EL15_FAIL;
	int i, ret;

	if (!value)
		return DS28EL15_FAIL;
	mutex_lock(&batt_safe_info_lock);
	switch (type) {
	case BATT_MATCH_ABILITY:
		for (i = 0; i < GET_BLOCK_STATUS_RETRY;) {
			if (!(mom->validity_status)) {
				if (!i)
					set_sched_affinity_to_current();
				hwlog_info("read block status start\n");
				i++;
				ret = mops->get_status(&ds28el15.ic_des);
				DS28EL15_COMMUNICATION_INFO(ret, "get_status");
			}
			if (mom->validity_status)
				break;

			else
				err_num[GET_BLOCK_STATUS_INDEX]++;
		}
		if (i)
			set_sched_affinity_to_all();
		if (mom->block_status[BATT_ID_BLOCK] & DS28EL15_WRT_PROT)
			*(enum batt_match_type *)value = BATTERY_UNREMATCHABLE;
		else
			*(enum batt_match_type *)value = BATTERY_REMATCHABLE;
		retval = DS28EL15_SUCCESS;
		break;
	default:
		hwlog_err("unsupported safety info type in %s\n", __func__);
		break;
	}
	mutex_unlock(&batt_safe_info_lock);

	return retval;
}

static enum batt_ic_type ds28el15_get_ic_type(void)
{
	return MAXIM_DS28EL15_TYPE;
}

#ifdef ONEWIRE_STABILITY_DEBUG

static ssize_t get_rom_id_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct maxim_mem_ops *mops = &ds28el15.ic_des.mem_ops;

	if (mops->get_rom_id(&ds28el15.ic_des))
		com_err[DEBUG_ROM_ID]++;
	total_com[DEBUG_ROM_ID]++;

	return snprintf(buf, PAGE_SIZE, "%d@%d", com_err[DEBUG_ROM_ID],
			total_com[DEBUG_ROM_ID]);
}

static ssize_t get_personality_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct maxim_mem_ops *mops = &ds28el15.ic_des.mem_ops;

	if (mops->get_personality(&ds28el15.ic_des))
		com_err[DEBUG_PERSONALITY]++;
	total_com[DEBUG_PERSONALITY]++;

	return snprintf(buf, PAGE_SIZE, "%d@%d", com_err[DEBUG_PERSONALITY],
			total_com[DEBUG_PERSONALITY]);
}

static ssize_t get_eeprom_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct maxim_mem_ops *mops = &ds28el15.ic_des.mem_ops;

	if (mops->get_user_memory(&ds28el15.ic_des, MAXIM_PAGE0,
				  0, MAXIM_SEGMENT_NUM))
		com_err[DEBUG_GET_EEPROM]++;
	total_com[DEBUG_GET_EEPROM]++;

	return snprintf(buf, PAGE_SIZE, "%d@%d", com_err[DEBUG_GET_EEPROM],
			total_com[DEBUG_GET_EEPROM]);
}

static ssize_t set_eeprom_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct maxim_mem_ops *mops = &ds28el15.ic_des.mem_ops;

	if (mops->set_user_memory(&ds28el15.ic_des, "1234", MAXIM_PAGE0,
				  MAXIM_SEGMENT0))
		com_err[DEBUG_SET_EEPROM]++;
	total_com[DEBUG_SET_EEPROM]++;

	return snprintf(buf, PAGE_SIZE, "%d@%d", com_err[DEBUG_SET_EEPROM],
			total_com[DEBUG_SET_EEPROM]);
}

static ssize_t get_sram_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct maxim_mem_ops *mops = &ds28el15.ic_des.mem_ops;

	if (mops->get_sram(&ds28el15.ic_des))
		com_err[DEBUG_GET_SRAM]++;
	total_com[DEBUG_GET_SRAM]++;

	return snprintf(buf, PAGE_SIZE, "%d@%d", com_err[DEBUG_GET_SRAM],
			total_com[DEBUG_GET_SRAM]);
}

static ssize_t set_sram_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct maxim_mem_ops *mops = &ds28el15.ic_des.mem_ops;

	if (mops->set_sram(&ds28el15.ic_des, random_bytes))
		com_err[DEBUG_SET_SRAM]++;
	total_com[DEBUG_SET_SRAM]++;

	return snprintf(buf, PAGE_SIZE, "%d@%d", com_err[DEBUG_SET_SRAM],
			total_com[DEBUG_SET_SRAM]);
}

static ssize_t get_mac_show(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	struct maxim_mem_ops *mops = &ds28el15.ic_des.mem_ops;

	if (mops->get_mac(&ds28el15.ic_des, !ANONYMOUS_MODE, MAXIM_PAGE0))
		com_err[DEBUG_GET_MAC]++;
	total_com[DEBUG_GET_MAC]++;

	return snprintf(buf, PAGE_SIZE, "%d@%d", com_err[DEBUG_GET_MAC],
			total_com[DEBUG_GET_MAC]);
}

static ssize_t eeprom_status_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct maxim_mem_ops *mops = &ds28el15.ic_des.mem_ops;

	if (mops->get_status(&ds28el15.ic_des))
		com_err[DEBUG_EEPROM_STATUS]++;
	total_com[DEBUG_EEPROM_STATUS]++;

	return snprintf(buf, PAGE_SIZE, "%d@%d", com_err[DEBUG_EEPROM_STATUS],
			total_com[DEBUG_EEPROM_STATUS]);
}

static DEVICE_ATTR_RO(get_rom_id);
static DEVICE_ATTR_RO(get_personality);
static DEVICE_ATTR_RO(get_eeprom);
static DEVICE_ATTR_RO(set_eeprom);
static DEVICE_ATTR_RO(get_sram);
static DEVICE_ATTR_RO(set_sram);
static DEVICE_ATTR_RO(get_mac);
static DEVICE_ATTR_RO(eeprom_status);
#endif

static const char * const err_str[] = {
	[SET_SRAM_INDEX] = "SET_SRAM",
	[GET_USER_MEMORY_INDEX] = "GET_USER_MEMORY",
	[GET_BLOCK_STATUS_INDEX] = "GET_BLOCK_STATUS",
	[SET_BLOCK_STATUS_INDEX] = "SET_BLOCK_STATUS",
	[GET_PERSONALITY_INDEX] = "GET_PERSONALITY",
	[GET_ROM_ID_INDEX] = "GET_ROM_ID",
	[GET_MAC_INDEX] = "GET_MAC",
};

static ssize_t err_num_show(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	int val = 0;
	int i;

	for (i = 0; i < __MAX_COM_ERR_NUM; i++)
		val += snprintf(buf + val, PAGE_SIZE, "%s failed:%d times\n",
				err_str[i], err_num[i]);

	return val;
}

static DEVICE_ATTR_RO(err_num);

static struct attribute *ds28el15_attrs[] = {
#ifdef ONEWIRE_STABILITY_DEBUG
	&dev_attr_get_rom_id.attr,
	&dev_attr_get_personality.attr,
	&dev_attr_get_eeprom.attr,
	&dev_attr_set_eeprom.attr,
	&dev_attr_get_sram.attr,
	&dev_attr_set_sram.attr,
	&dev_attr_get_mac.attr,
	&dev_attr_eeprom_status.attr,
#endif
	&dev_attr_err_num.attr,
	NULL,			/* sysfs_create_files need last one be NULL */
};

int ds28el15_ct_ops_register(struct platform_device *pdev,
			     struct batt_ct_ops *bco)
{
	struct maxim_mem_ops *mops = &ds28el15.ic_des.mem_ops;

	if (!mops->valid_mem_ops) {
		hwlog_err("%s can't find ops\n", ds28el15.pdev->name);
		return DS28EL15_FAIL;
	}
	if (mops->valid_mem_ops(&ds28el15.ic_des, ds28el15.pdev)) {
		hwlog_err("%s can't find phy ctl\n", ds28el15.pdev->name);
		return DS28EL15_FAIL;
	}
	if (ds28el15_check_ic_status(ds28el15.pdev)) {
		hwlog_err("%s ic status was not fine\n", ds28el15.pdev->name);
		return DS28EL15_FAIL;
	}
	get_random_bytes(random_bytes, ds28el15.ic_des.memory.sram_length);
	bco->get_ic_type = ds28el15_get_ic_type;
	bco->get_batt_type = ds28el15_get_batt_type;
	bco->get_batt_sn = ds28el15_get_sn;
	bco->certification = ds28el15_certification;
	bco->prepare = ds28el15_prepare;
	bco->set_batt_safe_info = ds28el15_set_batt_safe_info;
	bco->get_batt_safe_info = ds28el15_get_batt_safe_info;
	bco->power_down = NULL;

	return DS28EL15_SUCCESS;
}

/* Battery constraints initialization */
static int batt_cons_init(struct platform_device *pdev)
{
	int ret;

	/* Allocate memory for battery constraint infomation */
	batt_cons.id_mask = devm_kzalloc(&pdev->dev,
					 ds28el15.ic_des.memory.rom_id_len,
					 GFP_KERNEL);
	if (!batt_cons.id_mask)
		return DS28EL15_FAIL;
	batt_cons.id_example = devm_kzalloc(&pdev->dev,
					    ds28el15.ic_des.memory.rom_id_len,
					    GFP_KERNEL);
	if (!batt_cons.id_example)
		return DS28EL15_FAIL;
	batt_cons.id_chk = devm_kzalloc(&pdev->dev,
					ds28el15.ic_des.memory.rom_id_len,
					GFP_KERNEL);
	if (!batt_cons.id_chk)
		return DS28EL15_FAIL;

	/* Get battery id mask & id example */
	ret = of_property_read_u8_array(pdev->dev.of_node, "id-mask",
					batt_cons.id_mask,
					ds28el15.ic_des.memory.rom_id_len);
	if (ret) {
		hwlog_err("DTS:id-mask needed in %s\n", __func__);
		return DS28EL15_FAIL;
	}
	ret = of_property_read_u8_array(pdev->dev.of_node, "id-example",
					batt_cons.id_example,
					ds28el15.ic_des.memory.rom_id_len);
	if (ret) {
		hwlog_err("DTS:id-example needed in %s\n", __func__);
		return DS28EL15_FAIL;
	}
	return DS28EL15_SUCCESS;
}

static int dev_sys_node_init(struct platform_device *pdev)
{
	ds28el15.attr_group = devm_kzalloc(&pdev->dev,
					   sizeof(*ds28el15.attr_group),
					   GFP_KERNEL);
	if (!ds28el15.attr_group)
		return DS28EL15_FAIL;
	ds28el15.attr_groups = devm_kzalloc(&pdev->dev,
					    2 * sizeof(ds28el15.attr_group),
					    GFP_KERNEL);
	if (!ds28el15.attr_groups)
		return DS28EL15_FAIL;
	ds28el15.attr_group->attrs = &ds28el15_attrs[0];
	ds28el15.attr_groups[0] = ds28el15.attr_group;
	ds28el15.attr_groups[1] = NULL;
	if (sysfs_create_groups(&pdev->dev.kobj, ds28el15.attr_groups))
		return -1;

	return 0;
}

static int ds28el15_driver_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *batt_ic_np = NULL;
	struct maxim_ow_mem *mom = NULL;

	if (!pdev)
		return DS28EL15_FAIL;
	hwlog_info("ds28el15: probing\n");
	/* record ds28el15 platform_device */
	ds28el15.pdev = pdev;
	batt_ic_np = pdev->dev.of_node;
	wake_lock_init(&ds28el15.write_lock, WAKE_LOCK_SUSPEND, pdev->name);
	/* maxim onewire ic and use gpio as communication controller */
	if (maxim_onewire_register(&ds28el15.ic_des, ds28el15.pdev)) {
		hwlog_err("ds28el15 registeration failed in %s\n", __func__);
		goto ds28el15_probe_fail;
	}
	if (batt_cons_init(pdev)) {
		hwlog_err("ds28el15 constraint init failed in %s\n", __func__);
		goto ds28el15_probe_fail;
	}
	if (dev_sys_node_init(pdev)) {
		hwlog_err("dev_sys_node_init failed in %s\n", __func__);
		goto ds28el15_probe_fail;
	}
	mom = &ds28el15.ic_des.memory;
	/* get random 32 bytes */
	random_bytes = devm_kmalloc(&pdev->dev, mom->sram_length, GFP_KERNEL);
	if (!random_bytes)
		goto ds28el15_probe_fail;
	mom->sn_length_bits = SN_LENGTH_BITS;
	ret = of_property_read_u32(batt_ic_np, "sn-offset-bits",
				   &(mom->sn_offset_bits));
	if (ret) {
		hwlog_err("Read SN offset in bits failed in %s\n", __func__);
		goto ds28el15_probe_fail;
	} else if (NOT_MUTI8(mom->sn_offset_bits)) {
		hwlog_err("Illegal SN offset(%u) found in %s\n",
			  mom->sn_offset_bits, __func__);
		goto ds28el15_probe_fail;
	}
	if (mom->sn_length_bits > BYTES2BITS(mom->page_size) ||
	    mom->sn_offset_bits > BYTES2BITS(mom->page_size) ||
	    mom->sn_length_bits + mom->sn_offset_bits >
	    BYTES2BITS(mom->page_size)) {
		hwlog_err("sn len(%u) or off(%u) is illegal found in %s\n",
			  mom->sn_length_bits, mom->sn_offset_bits, __func__);
	}
	ret = of_property_read_u32(batt_ic_np, "sn-page", &(mom->sn_page));
	if (ret || mom->sn_page >= mom->page_number)
		hwlog_err("Illegal SN page(%u) readed by %s\n", mom->sn_page,
			  __func__);

	/* add ds28el15_ct_ops_register to struct batt_ct_ops_list */
	ds28el15_ct_node.ic_memory_release = NULL;
	ds28el15_ct_node.ct_ops_register = ds28el15_ct_ops_register;
	ds28el15_ct_node.ic_dev = pdev;
	add_to_aut_ic_list(&ds28el15_ct_node);
	hwlog_info("ds28el15: probing success\n");

	return DS28EL15_SUCCESS;

ds28el15_probe_fail:
	wake_lock_destroy(&ds28el15.write_lock);
	return DS28EL15_FAIL;
}

static int ds28el15_driver_remove(struct platform_device *pdev)
{
	ds28el15.pdev = NULL;
	wake_lock_destroy(&ds28el15.write_lock);

	return DS28EL15_SUCCESS;
}

static const struct of_device_id ds28el15_match_table[] = {
	{ .compatible = "maxim,onewire-sha", },
	{ /* end */ },
};

static struct platform_driver ds28el15_driver = {
	.probe = ds28el15_driver_probe,
	.remove = ds28el15_driver_remove,
	.driver = {
		.name = "Ds28el15",
		.owner = THIS_MODULE,
		.of_match_table = ds28el15_match_table,
	},
};

int __init ds28el15_driver_init(void)
{
	return platform_driver_register(&ds28el15_driver);
}

void __exit ds28el15_driver_exit(void)
{
	platform_driver_unregister(&ds28el15_driver);
}

subsys_initcall_sync(ds28el15_driver_init);
module_exit(ds28el15_driver_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ds28el15 ic");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
