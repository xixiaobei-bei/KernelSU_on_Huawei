/*
 * wireless_test_hw.h
 *
 * common interface, variables, definition etc for wireless hardware test
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _WIRELESS_TEST_HW_H_
#define _WIRELESS_TEST_HW_H_

/* test module */
#define WIRELESS_HW_TEST_PWR_GOOD_GPIO    BIT(0)
#define WIRELESS_HW_TEST_ALIGN_CIRCUIT    BIT(1)
#define WIRELESS_HW_TEST_REVERSE_CP       BIT(2)

/* test reverse cp type */
#define CHK_RVS_CP_PREV_PREPARE           0
#define CHK_RVS_CP_POST_PREPARE           1

enum wireless_hw_test_sysfs {
	WIRELESS_HW_TEST,
	WIRELESS_COIL_TEST,
};

struct wireless_hw_test_ops {
	int (*chk_pwr_good_gpio)(void);
	int (*chk_alignment_circuit)(void);
	int (*chk_reverse_cp)(void);
	int (*chk_reverse_cp_prepare)(int);
};

struct wireless_hw_dev_info {
	struct device_node *np;
	struct work_struct chk_hw_work;
	struct wireless_hw_test_ops *ops;
	u16 hw_support_module;
	u16 hw_test_result;
	u16 hw_test_module;
	bool test_busy;
};

#ifdef CONFIG_WIRELESS_CHARGER
int wireless_hw_test_ops_register(const struct wireless_hw_test_ops *ops);
int wireless_hw_test_reverse_cp_prepare(int type);
void wireless_coil_test_start(void);
int wirless_coil_test_result(void);
#else
static inline int wireless_hw_test_ops_register(
	const struct wireless_hw_test_ops *ops)
{
	return 0;
}

static inline int wireless_hw_test_reverse_cp_prepare(int type)
{
	return 0;
}

static inline void wireless_coil_test_start(void)
{
}

static inline int wirless_coil_test_result(void);
{
	return -ENOTSUPP;
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_TEST_HW_H_ */
