/*
 * subpmu.c
 *
 * subpmu interface for dieid
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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

#include "subpmu.h"
#include <asm/compiler.h>
#include <linux/compiler.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/cpumask.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <asm/memory.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/delay.h>
#include <securec.h>

#ifdef CONFIG_HISI_DIEID

#define PMUSSI0_BASE_ADDR	0xFFF34000
#define PMUSSI_ADDR(ch, offset)	((char *)PMUSSI0_BASE_ADDR + (ch) * 0x2000 + ((offset) << 2))

#define PMU_ERROR	(-1)
#define PMU_OK	0

static int hisi_subpmu_read(const struct pmu_para *pmu_info,
			    unsigned short addr, unsigned char *value)
{
	unsigned char slave_id = pmu_info->slave_id;

	if (strncmp(pmu_info->bus_type, "spmi", strlen("spmi")) == 0)
		*value = pmic_read_sub_pmu(slave_id, addr);
	else if (strncmp(pmu_info->bus_type, "ssi", strlen("ssi")) == 0)
		*value = *(volatile unsigned char*)(PMUSSI_ADDR(slave_id, addr));
	else
		return PMU_ERROR;

	return PMU_OK;
}

static int hisi_subpmu_write(const struct pmu_para *pmu_info,
			     unsigned short addr, unsigned char value)
{
	unsigned char slave_id = pmu_info->slave_id;

	if (strncmp(pmu_info->bus_type, "spmi", strlen("spmi")) == 0)
		pmic_write_sub_pmu(slave_id, addr, value);
	else if (strncmp(pmu_info->bus_type, "ssi", strlen("ssi")) == 0)
		*(volatile unsigned char*)(PMUSSI_ADDR(slave_id, addr)) = value;
	else
		return PMU_ERROR;

	return PMU_OK;
}

static int get_hi6422_dieid(const struct pmu_para *pmu_info,
			    unsigned char *die_id, unsigned int dieid_size)
{
	int i, ret;
	unsigned char val = 0;
	unsigned char otp_reg = OTP_REG_BEGIN;
	unsigned char id_reg = OTP_119_112;
	unsigned char rst_reg = OTP_CLK_CTRL;

	if (dieid_size < HI6422_OTP_REG_NUM) {
		pr_err("%s: invalid para:%d\n", __func__, dieid_size);
		return -EINVAL;
	}

	/* step1: set register-controlled read mode */
	ret = hisi_subpmu_read(pmu_info, OTP_CTRL1, &val);
	val = (val & AND_VALUE4) | BIT(OTP_INCTRL_SEL_BIT);
	ret += hisi_subpmu_write(pmu_info, OTP_CTRL1, val);

	/* step2: open clock */
	ret += hisi_subpmu_write(pmu_info, OTP_CLK_CTRL, OPEN_CLK_VALUE);

	/* step3: set Normal Read mode */
	ret += hisi_subpmu_read(pmu_info, OTP_CTRL2, &val);
	val = val & AND_VALUE5;
	ret += hisi_subpmu_write(pmu_info, OTP_CTRL2, val);

	/* step4: set address in Normal Read mode from bit[119:112] to [175:168] */
	for (i = 0; i < HI6422_OTP_REG_NUM; i++) {
		ret += hisi_subpmu_read(pmu_info, OTP_CTRL2, &val);
		val = (val & AND_VALUE6) | otp_reg;
		ret += hisi_subpmu_write(pmu_info, OTP_CTRL2, val);
		otp_reg = otp_reg + OTP_REG_STEP;

		/* step5: enable read */
		ret += hisi_subpmu_read(pmu_info, OTP_CTRL0, &val);
		val = (val & AND_VALUE2) | BIT(OTP_POR_INT_BIT);
		ret += hisi_subpmu_write(pmu_info, OTP_CTRL0, val);

		/* step6: rescind read_enable signal */
		udelay(DELAY_TIME);
		ret += hisi_subpmu_read(pmu_info, OTP_CTRL0, &val);
		val = val & AND_VALUE2;
		ret += hisi_subpmu_write(pmu_info, OTP_CTRL0, val);
	}

	/* step7: check out data */
	for (i = 0; i < HI6422_OTP_REG_NUM; i++) {
		ret += hisi_subpmu_read(pmu_info, id_reg, die_id);
		id_reg++;
		die_id++;
	}

	/* step8: reset registers to default value */
	for (i = 0; i < RST_REG_NUM; i++) {
		ret += hisi_subpmu_write(pmu_info, rst_reg, RESET_VALUE);
		rst_reg++;
	}

	return ret;
}

static int get_hi6422v5_dieid(const struct pmu_para *pmu_info,
			      unsigned char *die_id, unsigned int dieid_size)
{
	int i, ret;

	if (dieid_size < (OTP_END_V5 - OTP_START_V5 + 1)) {
		pr_err("%s: invalid para:%d\n", __func__, dieid_size);
		return -EINVAL;
	}

	/* set Normal Read mode */
	ret = hisi_subpmu_write(pmu_info, OTP_CTRL1_V5, BIT(OTP_INCTRL_SEL_BIT));
	ret += hisi_subpmu_write(pmu_info, OTP_CTRL2_V5, RESET_VALUE);

	for (i = OTP_START_V5; i < OTP_END_V5; i += 2) {
		ret += hisi_subpmu_write(pmu_info, OTP_CTRL2_V5, i);
		ret += hisi_subpmu_write(pmu_info, OTP_CTRL0_V5, BIT(OTP_POR_INT_BIT));
		udelay(45);

		ret += hisi_subpmu_write(pmu_info, OTP_CTRL0_V5, RESET_VALUE);

		ret += hisi_subpmu_read(pmu_info, OTP0_V5, die_id);
		die_id++;
		ret += hisi_subpmu_read(pmu_info, OTP1_V5, die_id);
		die_id++;
		udelay(62);
	}

	ret = hisi_subpmu_write(pmu_info, OTP_CTRL2_V5, RESET_VALUE);
	ret += hisi_subpmu_write(pmu_info, OTP_CTRL1_V5, RESET_VALUE);

	return ret;
}

static int get_hi6423_dieid(const struct pmu_para *pmu_info,
			    unsigned char *die_id, unsigned int dieid_size)
{
	int i, ret;
	unsigned char val = 0;
	unsigned char id_reg = ID_OTP_0;

	if (dieid_size < HI6423_READ_TIMES) {
		pr_err("%s: invalid para:%d\n", __func__, dieid_size);
		return -EINVAL;
	}

	/* clock open */
	ret = hisi_subpmu_write(pmu_info, ID_OTP_CLK_CTRL, OPEN_CLK_VALUE);

	/* step1: set id_otp_ptm = 2'b00, id_otp_pprog = 1'b0, enter in read mode */
	ret += hisi_subpmu_read(pmu_info, ID_OTP_CTRL1, &val);
	val = val & AND_VALUE1;
	ret += hisi_subpmu_write(pmu_info, ID_OTP_CTRL1, val);

	/* step2: set id_otp_pa = 2'b00, address writed to register */
	ret += hisi_subpmu_read(pmu_info, ID_OTP_CTRL1, &val);
	val = val & AND_VALUE3;
	ret += hisi_subpmu_write(pmu_info, ID_OTP_CTRL1, val);

	/* step3: set id_por_int = 1'b1, enable read mode */
	ret += hisi_subpmu_read(pmu_info, ID_OTP_CTRL0, &val);
	val = (val & AND_VALUE2) | BIT(OTP_POR_INT_BIT);
	ret += hisi_subpmu_write(pmu_info, ID_OTP_CTRL0, val);

	/*
	 * step4: delay 30us, then set id_otp_por_int = 1'b0
	 * to cancel the read mode enable signal
	 */
	udelay(DELAY_TIME);
	ret += hisi_subpmu_read(pmu_info, ID_OTP_CTRL0, &val);
	val = val & AND_VALUE2;
	ret += hisi_subpmu_write(pmu_info, ID_OTP_CTRL0, val);

	/* step5: read from registers ID_OTP_0~3 */
	for (i = 0; i < HI6423_READ_TIMES; i++) {
		ret += hisi_subpmu_read(pmu_info, id_reg, die_id);
		id_reg++;
		die_id++;
	}

	/* clock close */
	ret += hisi_subpmu_write(pmu_info, ID_OTP_CLK_CTRL, RESET_VALUE);

	return ret;
}

static int get_subpmu_tree_data(struct device_node *root,
				struct hisi_subpmu_dieid *subpmu)
{
	int ret;
	int i = 0;
	struct device_node *child = NULL;

	while ((child = of_get_next_child(root, child)) != NULL) {
		ret = of_property_read_string(child, "pmu-type",
					      &subpmu->pmu_info[i].pmu_type);
		if (ret != 0) {
			pr_err("%s:no %s pmu-type!\n", __func__, child->name);
			return -ENODEV;
		}

		ret = of_property_read_string(child, "bus-type",
					      &subpmu->pmu_info[i].bus_type);
		if (ret != 0) {
			pr_err("%s:no %s bus-type!\n", __func__, child->name);
			return -ENODEV;
		}

		ret = of_property_read_u32(child, "slave-id",
					   &subpmu->pmu_info[i].slave_id);
		if (ret != 0) {
			pr_err("%s:no %s slave-id!\n", __func__, child->name);
			return -ENODEV;
		}

		i++;
	}

	return PMU_OK;
}

struct pmu_dieid_func_def g_pmu_dieid_func[] = {
	{ "Hi6422v300", (pmu_func_t)get_hi6422_dieid },
	{ "Hi6423v100", (pmu_func_t)get_hi6423_dieid },
	{ "Hi6422v500", (pmu_func_t)get_hi6422v5_dieid },
};

static int get_dieid(const struct pmu_para *pmu_info,
		     unsigned char *ip, unsigned int ip_size)
{
	int ret;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_pmu_dieid_func); i++) {
		ret = strncmp(pmu_info->pmu_type,
			      g_pmu_dieid_func[i].pmu_type,
			      strlen(pmu_info->pmu_type));
		if (ret == 0) {
			ret = (*g_pmu_dieid_func[i].pmu_func)(pmu_info, ip, ip_size);
			if (ret != PMU_OK)
				pr_err("%s:get_%s_dieid_fail!\n",
				       __func__, pmu_info->pmu_type);

			return ret;
		}
	}

	pr_err("%s: %s no pmu_type matched!\n", __func__, pmu_info->pmu_type);
	return PMU_ERROR;
}

static int dieid_package(const struct pmu_para *pmu_info, int idx, char *pmu_buf)
{
	int i, ret;
	char tmp[HISI_PMU_TMP_BUF] = {0};
	unsigned char ip[HISI_PMU_IP_SIZE] = {0};

	ret = snprintf_s(pmu_buf, HISI_PMU_TMP_BUF, HISI_PMU_TMP_BUF - 1,
			 "\r\n%s_PMU%d:0x", pmu_info->pmu_type, idx);
	if (ret < 0) {
		pr_err("%s:snprintf_s_1 error!\n", __func__);
		return ret;
	}

	ret = get_dieid(pmu_info, ip, HISI_PMU_IP_SIZE);
	if (ret != PMU_OK)
		return ret;

	for (i = HISI_PMU_IP_SIZE - 1; i >= 0; i--) {
		ret = snprintf_s(tmp, HISI_PMU_TMP_BUF, HISI_PMU_TMP_BUF - 1,
				 "%02x", ip[i]);
		if (ret < 0) {
			pr_err("%s:snprintf_s_2 error!\n", __func__);
			return ret;
		}
		ret = strncat_s(pmu_buf, HISI_PMU_TMP_BUF, tmp, strlen(tmp));
		if (ret != EOK) {
			pr_err("%s:strncat_s_1 error:%d!\n", __func__, ret);
			return ret;
		}
		(void)memset_s(tmp, HISI_PMU_TMP_BUF, 0, strlen(tmp));
	}

	ret = strncat_s(pmu_buf, HISI_PMU_TMP_BUF, "\r\n", strlen("\r\n"));
	if (ret != EOK) {
		pr_err("%s:strncat_s_2 error:%d!\n", __func__, ret);
		return ret;
	}
	return PMU_OK;
}

int hisi_subpmu_get_dieid(char *dieid, unsigned int len)
{
	int ret;
	int pmu_idx = PMU_IDX_START;
	unsigned int i;
	char pmu_buf[HISI_PMU_TMP_BUF] = {0};
	char dieid_buf[HISI_PMU_BUF_SIZE] = {0};
	struct device_node *root = NULL;
	struct hisi_subpmu_dieid subpmu;

	if (dieid == NULL) {
		pr_err("%s:dieid is NULL!\n", __func__);
		return -ENOMEM;
	}

	root = of_find_compatible_node(NULL, NULL, "hisilicon,subpmu-dieid");
	if (root == NULL) {
		pr_err("%s:no hisi-subpmu-dieid root node!\n", __func__);
		return -ENODEV;
	}

	if (!of_device_is_available(root)) {
		of_node_put(root);
		pr_err("%s:no dieid!\n", __func__);
		return -ENODEV;
	}

	subpmu.dieid_num = of_get_child_count(root);
	subpmu.pmu_info = kzalloc(sizeof(struct pmu_para) * subpmu.dieid_num,
				  GFP_KERNEL);
	if (subpmu.pmu_info == NULL) {
		of_node_put(root);
		pr_err("%s: alloc subpmu->pmu_info fail\n", __func__);
		return -ENOMEM;
	}

	ret = get_subpmu_tree_data(root, &subpmu);
	if (ret != PMU_OK) {
		pr_err("%s:Error reading subpmu_dieid dts!\n", __func__);
		goto out;
	}

	for (i = 0; i < subpmu.dieid_num; i++) {
		ret = dieid_package(&subpmu.pmu_info[i], pmu_idx, pmu_buf);
		if (ret < 0) {
			ret = -EINVAL;
			goto out;
		}

		ret = strncat_s(dieid_buf, HISI_PMU_BUF_SIZE,
				pmu_buf, strlen(pmu_buf));
		if (ret != EOK) {
			pr_err("%s %d:strncat_s error:%d!\n",
			       __func__, __LINE__, ret);
			goto out;
		}
		(void)memset_s(pmu_buf, HISI_PMU_TMP_BUF, 0, HISI_PMU_TMP_BUF);
		pmu_idx++;
	}

	if (len >= strlen(dieid_buf)) {
		ret = strncpy_s(dieid, len, dieid_buf, strlen(dieid_buf));
		if (ret != EOK)
			pr_err("%s:strncpy_s error!\n", __func__);
	} else {
		pr_err("%s:dieid length is not enough!\n", __func__);
		ret = strlen(dieid_buf);
	}

out:
	of_node_put(root);
	kfree(subpmu.pmu_info);
	return ret;
}
#endif
