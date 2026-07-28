#ifndef __HISI_SUBPMU_H__
#define __HISI_SUBPMU_H__

#ifdef CONFIG_HISI_DIEID
#define ID_OTP_CLK_CTRL	0x54
#define ID_OTP_CTRL0	0x51
#define ID_OTP_CTRL1	0x52
#define ID_OTP_0	0x4d
#define HI6423_READ_TIMES	4

/*define hi6423v100 & hi6422v300 commen para*/
#define OPEN_CLK_VALUE	0x5a
#define RESET_VALUE	0x00
#define DELAY_TIME	30
#define AND_VALUE1	0xe3
#define AND_VALUE2	0xfb
#define AND_VALUE3	0xfc
#define AND_VALUE4	0xfd
#define AND_VALUE5	0x3f
#define AND_VALUE6	0xc0

#define HISI_PMU_TMP_BUF	50
#define HISI_PMU_IP_SIZE	8
#define HISI_PMU_BUF_SIZE	100

/*define hi6422v300 register*/
#define OTP_119_112		0xf6
#define OTP_REG_BEGIN	0x0c
#define HI6422_OTP_REG_NUM	8
#define RST_REG_NUM		4

enum hi6422v300_reg {
	OTP_CLK_CTRL = 0xC0,
	OTP_CTRL0,
	OTP_CTRL1,
	OTP_CTRL2,
};

struct pmu_para {
	const char *pmu_type;
	const char *bus_type;
	unsigned int slave_id;
};

struct hisi_subpmu_dieid {
	unsigned int dieid_num;
	struct pmu_para *pmu_info;
};

typedef int (*pmu_func_t)(struct pmu_para *pmu_info, unsigned char *die_id);
struct pmu_dieid_func_def {
	const char *pmu_type;
	pmu_func_t pmu_func;
};

int hisi_subpmu_get_dieid(char *dieid, unsigned int len);
#endif
#endif
