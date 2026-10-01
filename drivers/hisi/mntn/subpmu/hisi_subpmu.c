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
#include "hisi_subpmu.h"


#define PMUSSI0_BASE_ADDR			(0xFFF34000)
#define PMUSSI_ADDR(ch, offset)     ((char *)PMUSSI0_BASE_ADDR + (ch) * 0x2000 + ((offset) << 2))

static int hisi_subpmu_read(struct pmu_para *pmu_info, u16 addr, unsigned char *value)
{
	unsigned char slave_id = pmu_info->slave_id;

	if (0 == strncmp(pmu_info->bus_type, "spmi", 4)) {
		*value = hisi_pmic_read_sub_pmu(slave_id, addr);
	} else if (0 == strncmp(pmu_info->bus_type, "ssi", 3)) {
		*value = *(volatile unsigned char*)(PMUSSI_ADDR(slave_id, addr));
	} else {
		return -1;
	}

	return 0;
}

static int hisi_subpmu_write(struct pmu_para *pmu_info, u16 addr, unsigned char value)
{
	unsigned char slave_id = pmu_info->slave_id;

	if (0 == strncmp(pmu_info->bus_type, "spmi", 4)) {
		hisi_pmic_write_sub_pmu(slave_id, addr, value);
	} else if (0 == strncmp(pmu_info->bus_type, "ssi", 3)) {
		*(volatile unsigned char*)(PMUSSI_ADDR(slave_id, addr)) = value;
	} else {
		return -1;
	}

	return 0;
}

static int get_hi6422_dieid(struct pmu_para *pmu_info, unsigned char *die_id)
{
	int i, ret;
	unsigned char val = 0, otp_reg = OTP_REG_BEGIN, id_reg = OTP_119_112, rst_reg = OTP_CLK_CTRL;

	/*step1 : set register-controlled read mode*/
	ret = hisi_subpmu_read(pmu_info, OTP_CTRL1, &val);
	val = (val & AND_VALUE4) | 0x02;
	ret += hisi_subpmu_write(pmu_info, OTP_CTRL1, val);

	/*step2 : open clock*/
	ret += hisi_subpmu_write(pmu_info, OTP_CLK_CTRL, OPEN_CLK_VALUE);

	/*step3 : set Normal Read mode*/
	ret += hisi_subpmu_read(pmu_info, OTP_CTRL2, &val);
	val = val & AND_VALUE5;
	ret += hisi_subpmu_write(pmu_info, OTP_CTRL2, val);

	/*step4 : set address in Normal Read mode from bit[119:112] to [175:168]*/
	for (i = 0; i < HI6422_OTP_REG_NUM; i++) {
		ret += hisi_subpmu_read(pmu_info, OTP_CTRL2, &val);
		val = (val & AND_VALUE6) | otp_reg;
		ret += hisi_subpmu_write(pmu_info, OTP_CTRL2, val);
		otp_reg = otp_reg + 2;

		/*step5 : enable read*/
		ret += hisi_subpmu_read(pmu_info, OTP_CTRL0, &val);
		val = (val & AND_VALUE2) | 0x04;
		ret += hisi_subpmu_write(pmu_info, OTP_CTRL0, val);

		/*step6 : rescind read_enable signal*/
		udelay(DELAY_TIME);
		ret += hisi_subpmu_read(pmu_info, OTP_CTRL0, &val);
		val = val & AND_VALUE2;
		ret += hisi_subpmu_write(pmu_info, OTP_CTRL0, val);
	}

	/*step7 : check out data*/
	for (i = 0; i < HI6422_OTP_REG_NUM; i++) {
		ret += hisi_subpmu_read(pmu_info, id_reg, die_id);
		id_reg++;
		die_id++;
	}

	/*step8 : reset registers to default value*/
	for (i = 0; i < RST_REG_NUM; i++) {
		ret += hisi_subpmu_write(pmu_info, rst_reg, RESET_VALUE);
		rst_reg++;
	}

	return ret;
}

static int get_hi6423_dieid(struct pmu_para *pmu_info, unsigned char *die_id)
{
	int i, ret;
	unsigned char val = 0, id_reg = ID_OTP_0;

	/*clock open */
	ret = hisi_subpmu_write(pmu_info, ID_OTP_CLK_CTRL, OPEN_CLK_VALUE);

	/* step1 : set id_otp_ptm = 2'b00, id_otp_pprog = 1'b0, enter in read mode*/
	ret += hisi_subpmu_read(pmu_info, ID_OTP_CTRL1, &val);
	val = val & AND_VALUE1;
	ret += hisi_subpmu_write(pmu_info, ID_OTP_CTRL1, val);

	/* step2 : set id_otp_pa = 2'b00, address writed to register*/
	ret += hisi_subpmu_read(pmu_info, ID_OTP_CTRL1, &val);
	val = val & AND_VALUE3;
	ret += hisi_subpmu_write(pmu_info, ID_OTP_CTRL1, val);

	/* step3 : set id_por_int = 1'b1, enable read mode*/
	ret += hisi_subpmu_read(pmu_info, ID_OTP_CTRL0, &val);
	val = (val & AND_VALUE2) | 0x4;
	ret += hisi_subpmu_write(pmu_info, ID_OTP_CTRL0, val);

	/* step4 : delay 30us, then set id_otp_por_int = 1'b0 to cancel the read mode enable signal */
	udelay(DELAY_TIME);
	ret += hisi_subpmu_read(pmu_info, ID_OTP_CTRL0, &val);
	val = val & AND_VALUE2;
	ret += hisi_subpmu_write(pmu_info, ID_OTP_CTRL0, val);

	/* step5 : read from registers ID_OTP_0~3*/
	for (i = 0; i < HI6423_READ_TIMES; i++) {
		ret += hisi_subpmu_read(pmu_info, id_reg, die_id);
		id_reg++;
		die_id++;
	}

	/*clock close */
	ret += hisi_subpmu_write(pmu_info, ID_OTP_CLK_CTRL, RESET_VALUE);

	return ret;
}


static int get_subpmu_tree_data(struct device_node *root, struct hisi_subpmu_dieid *subpmu)
{
	int ret = 0, i = 0;
	struct device_node *child = NULL;

	subpmu->dieid_num = of_get_child_count(root);
	subpmu->pmu_info = kzalloc(sizeof(struct pmu_para) * (subpmu->dieid_num), GFP_KERNEL);
	if (!subpmu->pmu_info) {
		pr_err("%s: alloc subpmu->pmu_info fail\n", __func__);
		return -ENOMEM;
	}

	while (NULL != (child = of_get_next_child(root, child))) {
		ret = of_property_read_string(child, "pmu-type", &(subpmu->pmu_info[i].pmu_type));
		if (ret) {
			pr_err("%s:no %s pmu-type property set!\n", __func__, child->name);
			ret = -ENODEV;
			goto err_out;
		}

		ret = of_property_read_string(child, "bus-type", &(subpmu->pmu_info[i].bus_type));
		if (ret) {
			pr_err("%s:no %s bus-type property set!\n", __func__, child->name);
			ret = -ENODEV;
			goto err_out;
		}

		ret = of_property_read_u32(child, "slave-id", &(subpmu->pmu_info[i].slave_id));
		if (ret) {
			pr_err("%s:no %s slave-id property set!\n", __func__, child->name);
			ret = -ENODEV;
			goto err_out;
		}

		i++;
	}

	return 0;

err_out:
	kfree(subpmu->pmu_info);
	subpmu->pmu_info = NULL;
	return ret;
}

struct pmu_dieid_func_def pmu_dieid_func[] = {
	{"Hi6422v300", (pmu_func_t)get_hi6422_dieid},
	{"Hi6423v100", (pmu_func_t)get_hi6423_dieid},
};

static int get_dieid(struct pmu_para *pmu_info, unsigned char *ip)
{
	int ret = 0;
	unsigned int i;

	for (i = 0; i < sizeof(pmu_dieid_func)/sizeof(pmu_dieid_func[0]); i++) {
		if (0 == strncmp(pmu_info->pmu_type, pmu_dieid_func[i].pmu_type, strlen(pmu_info->pmu_type))) {
			ret = (*(pmu_dieid_func[i].pmu_func))(pmu_info, ip);
			if (ret)
				pr_err("%s:get_%s_dieid_fail!\n", __func__, pmu_info->pmu_type);

			return ret;
		}
	}

	pr_err("%s: %s no pmu_type matched!\n", __func__, pmu_info->pmu_type);
	return -1;
}

static int dieid_package(struct pmu_para *pmu_info, int idx, char *pmu_buf)
{
	int i, ret;
	char tmp[HISI_PMU_TMP_BUF] = {0};
	unsigned char ip[HISI_PMU_IP_SIZE] = {0};

	ret = snprintf(pmu_buf, HISI_PMU_BUF_SIZE, "\r\n%s_PMU%d:0x", pmu_info->pmu_type, idx);/* [false alarm]:snprintf is safe  *//* unsafe_function_ignore: snprintf */
	if (ret < 0) {
		pr_err("%s:snprintf error!\n", __func__);
		return ret;
	}

	ret = get_dieid(pmu_info, ip);
	if (ret)
		return -1;

	for (i = HISI_PMU_IP_SIZE - 1; i >= 0; i--) {
		ret = snprintf(tmp, sizeof(tmp), "%02x", ip[i]);/* unsafe_function_ignore: snprintf */
		if (ret < 0) {
			pr_err("%s:snprintf error!\n", __func__);
			return ret;
		}
		strncat(pmu_buf, tmp, strlen(tmp));/* unsafe_function_ignore: strncat */
		memset(tmp, 0, strlen(tmp));/* unsafe_function_ignore: memset */
	}

	strncat(pmu_buf, "\r\n", strlen("\r\n"));/* unsafe_function_ignore: strncat */
	return 0;
}

int hisi_subpmu_get_dieid(char *dieid, unsigned int len)
{
	int ret, pmu_idx = 2;
	unsigned int i;
	char pmu_buf[HISI_PMU_TMP_BUF]={0};
	char dieid_buf[HISI_PMU_BUF_SIZE] = {0};
	struct device_node *root = NULL;
	struct hisi_subpmu_dieid subpmu;

	if (NULL == dieid) {
		pr_err("%s:dieid is NULL!\n", __func__);
		return -ENOMEM;
	}

	root = of_find_compatible_node(NULL, NULL, "hisilicon,subpmu-dieid");
	if (!root) {
		pr_err("%s:no hisi-subpmu-dieid root node!\n", __func__);
		return -ENODEV;
	}

	if (!of_device_is_available(root)) {
		pr_err("%s:no dieid!\n", __func__);
		return -1;
	}

	ret = get_subpmu_tree_data(root, &subpmu);
	if (ret) {
		pr_err("%s:Error reading subpmu_dieid dts!\n", __func__);
		return ret;
	}

	for (i = 0; i < subpmu.dieid_num; i++) {
		ret = dieid_package(&(subpmu.pmu_info[i]), pmu_idx, pmu_buf);
		if (ret < 0) {
			ret = -EINVAL;
			goto out;
		}

		strncat(dieid_buf, pmu_buf, strlen(pmu_buf));/* unsafe_function_ignore: strncat */
		memset(pmu_buf, 0, HISI_PMU_TMP_BUF);/* unsafe_function_ignore: memset */
		pmu_idx++;
	}

	if(len >= strlen(dieid_buf)) {
		strncpy(dieid, dieid_buf, strlen(dieid_buf));/* unsafe_function_ignore: strncpy */
		ret = 0;
	}else{
		pr_err("%s:dieid length is not enough!\n", __func__);
		ret = strlen(dieid_buf);
	}

out:
	kfree(subpmu.pmu_info);
	subpmu.pmu_info = NULL;
	return ret;
}
