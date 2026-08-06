/*
 * charger_protocol_fcp.h
 *
 * fcp protocol driver
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#ifndef _CHARGER_PROTOCOL_FCP_H_
#define _CHARGER_PROTOCOL_FCP_H_

#ifndef BIT
#define BIT(x)                               (1 << (x))
#endif

#define FCP_PROTOCOL_BYTE_BITS               8
#define FCP_PROTOCOL_BYTE_MASK               0xff

/* adapter type information register */
#define FCP_PROTOCOL_ADP_TYPE0               0x7e
#define FCP_PROTOCOL_ADP_TYPE0_AB_MASK       (BIT(7) | BIT(5) | BIT(4))
#define FCP_PROTOCOL_ADP_TYPE0_B_MASK        BIT(4)
#define FCP_PROTOCOL_ADP_TYPE0_B_SC_MASK     BIT(3)
#define FCP_PROTOCOL_ADP_TYPE0_B_LVC_MASK    BIT(2)

#define FCP_PROTOCOL_ADP_TYPE1               0x80
#define FCP_PROTOCOL_ADP_TYPE1_B_MASK        BIT(4)

#define FCP_PROTOCOL_COMPILEDVER_HBYTE       0x7c /* xx */
#define FCP_PROTOCOL_COMPILEDVER_LBYTE       0x7d /* yy.zz */
#define FCP_PROTOCOL_COMPILEDVER_XX_MASK     (BIT(7) | BIT(6) | BIT(5) | \
	BIT(4) | BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define FCP_PROTOCOL_COMPILEDVER_XX_SHIFT    0
#define FCP_PROTOCOL_COMPILEDVER_YY_MASK     (BIT(7) | BIT(6) | BIT(5) | BIT(4))
#define FCP_PROTOCOL_COMPILEDVER_YY_SHIFT    4
#define FCP_PROTOCOL_COMPILEDVER_ZZ_MASK     (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define FCP_PROTOCOL_COMPILEDVER_ZZ_SHIFT    0

#define FCP_PROTOCOL_FWVER_HBYTE             0x8a /* xx */
#define FCP_PROTOCOL_FWVER_LBYTE             0x8b /* yy.zz */
#define FCP_PROTOCOL_FWVER_XX_MASK           (BIT(7) | BIT(6) | BIT(5) | \
	BIT(4) | BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define FCP_PROTOCOL_FWVER_XX_SHIFT          0
#define FCP_PROTOCOL_FWVER_YY_MASK           (BIT(7) | BIT(6) | BIT(5) | BIT(4))
#define FCP_PROTOCOL_FWVER_YY_SHIFT          4
#define FCP_PROTOCOL_FWVER_ZZ_MASK           (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define FCP_PROTOCOL_FWVER_ZZ_SHIFT          0

#define FCP_PROTOCOL_RESERVED_0X70           0x70
#define FCP_PROTOCOL_RESERVED_0X71           0x71
#define FCP_PROTOCOL_RESERVED_0X72           0x72
#define FCP_PROTOCOL_RESERVED_0X73           0x73
#define FCP_PROTOCOL_RESERVED_0X74           0x74
#define FCP_PROTOCOL_RESERVED_0X75           0x75
#define FCP_PROTOCOL_RESERVED_0X76           0x76
#define FCP_PROTOCOL_RESERVED_0X77           0x77
#define FCP_PROTOCOL_RESERVED_0X78           0x78
#define FCP_PROTOCOL_RESERVED_0X79           0x79
#define FCP_PROTOCOL_RESERVED_0X7A           0x7a
#define FCP_PROTOCOL_RESERVED_0X7B           0x7b
#define FCP_PROTOCOL_RESERVED_0X7F           0x7f

#define FCP_PROTOCOL_DVCTYPE                 0x00

#define FCP_PROTOCOL_SPEC_VER                0x01 /* xx.yy.zz */
#define FCP_PROTOCOL_SPEC_XX_MASK            (BIT(7) | BIT(6) | BIT(5))
#define FCP_PROTOCOL_SPEC_XX_SHIFT           5
#define FCP_PROTOCOL_SPEC_YY_MASK            (BIT(4) | BIT(3) | BIT(2))
#define FCP_PROTOCOL_SPEC_YY_SHIFT           2
#define FCP_PROTOCOL_SPEC_ZZ_MASK            (BIT(1) | BIT(0))
#define FCP_PROTOCOL_SPEC_ZZ_SHIFT           0

#define FCP_PROTOCOL_SCNTL                   0x02

#define FCP_PROTOCOL_SSTAT                   0x03
#define FCP_PROTOCOL_CRCRX_MASK              BIT(1)
#define FCP_PROTOCOL_CRCRX_SHIFT             1
#define FCP_PROTOCOL_PARRX_MASK              BIT(0)
#define FCP_PROTOCOL_PARRX_SHIFT             0

#define FCP_PROTOCOL_CRCRX_ERR               1
#define FCP_PROTOCOL_CRCRX_NOERR             0
#define FCP_PROTOCOL_PARRX_ERR               1
#define FCP_PROTOCOL_PARRX_NOERR             0

#define FCP_PROTOCOL_ID_OUT0                 0x04
#define FCP_PROTOCOL_ID_OUT1                 0x05

/* adapter control information register */
#define FCP_PROTOCOL_CAPABILOTIES            0x20

#define FCP_PROTOCOL_DISCRETE_CAPABILOTIES0  0x21

#define FCP_PROTOCOL_CAPABILOTIES_5V_9V      0x1
#define FCP_PROTOCOL_CAPABILOTIES_5V_9V_12V  0x2

#define FCP_PROTOCOL_MAX_PWR                 0x22
#define FCP_PROTOCOL_MAX_PWR_STEP            500 /* step: 500mw */

#define FCP_PROTOCOL_ADAPTER_STATUS          0x28
#define FCP_PROTOCOL_UVP_MASK                BIT(3)
#define FCP_PROTOCOL_UVP_SHIFT               3
#define FCP_PROTOCOL_OVP_MASK                BIT(2)
#define FCP_PROTOCOL_OVP_SHIFT               2
#define FCP_PROTOCOL_OCP_MASK                BIT(1)
#define FCP_PROTOCOL_OCP_SHIFT               1
#define FCP_PROTOCOL_OTP_MASK                BIT(0)
#define FCP_PROTOCOL_OTP_SHIFT               0

#define FCP_PROTOCOL_VOUT_STATUS             0x29

#define FCP_PROTOCOL_OUTPUT_CONTROL          0x2b

#define FCP_PROTOCOL_VOUT_CONFIG_ENABLE      1
#define FCP_PROTOCOL_IOUT_CONFIG_ENABLE      1

#define FCP_PROTOCOL_VOUT_CONFIG             0x2c
#define FCP_PROTOCOL_VOUT_STEP               100 /* step: 100mv */

#define FCP_PROTOCOL_IOUT_CONFIG             0x2d
#define FCP_PROTOCOL_IOUT_STEP               100 /* step: 100ma */

#define FCP_PROTOCOL_DISCRETE_CAPABILOTIES1  0x2f

/* output voltage configure information register */
#define FCP_PROTOCOL_OUTPUT_V0               0x30
#define FCP_PROTOCOL_OUTPUT_V1               0x31
#define FCP_PROTOCOL_OUTPUT_V2               0x32
#define FCP_PROTOCOL_OUTPUT_V3               0x33
#define FCP_PROTOCOL_OUTPUT_V4               0x34
#define FCP_PROTOCOL_OUTPUT_V5               0x35
#define FCP_PROTOCOL_OUTPUT_V6               0x36
#define FCP_PROTOCOL_OUTPUT_V7               0x37
#define FCP_PROTOCOL_OUTPUT_V8               0x38
#define FCP_PROTOCOL_OUTPUT_V9               0x39
#define FCP_PROTOCOL_OUTPUT_VA               0x3a
#define FCP_PROTOCOL_OUTPUT_VB               0x3b
#define FCP_PROTOCOL_OUTPUT_VC               0x3c
#define FCP_PROTOCOL_OUTPUT_VD               0x3d
#define FCP_PROTOCOL_OUTPUT_VE               0x3e
#define FCP_PROTOCOL_OUTPUT_VF               0x3f
#define FCP_PROTOCOL_OUTPUT_V_REG(n)         (FCP_PROTOCOL_OUTPUT_V0 + n)
#define FCP_PROTOCOL_OUTPUT_V_STEP           100 /* step: 100mv */

#define FCP_PROTOCOL_DISCRETE_UVP0           0x40
#define FCP_PROTOCOL_DISCRETE_UVP1           0x41
#define FCP_PROTOCOL_DISCRETE_UVP2           0x42
#define FCP_PROTOCOL_DISCRETE_UVP3           0x43
#define FCP_PROTOCOL_DISCRETE_UVP4           0x44
#define FCP_PROTOCOL_DISCRETE_UVP5           0x45
#define FCP_PROTOCOL_DISCRETE_UVP6           0x46
#define FCP_PROTOCOL_DISCRETE_UVP7           0x47
#define FCP_PROTOCOL_DISCRETE_UVP8           0x48
#define FCP_PROTOCOL_DISCRETE_UVP9           0x49
#define FCP_PROTOCOL_DISCRETE_UVPA           0x4a
#define FCP_PROTOCOL_DISCRETE_UVPB           0x4b
#define FCP_PROTOCOL_DISCRETE_UVPC           0x4c
#define FCP_PROTOCOL_DISCRETE_UVPD           0x4d
#define FCP_PROTOCOL_DISCRETE_UVPE           0x4e
#define FCP_PROTOCOL_DISCRETE_UVPF           0x4f
#define FCP_PROTOCOL_DISCRETE_UVP_REG(n)     (FCP_PROTOCOL_DISCRETE_UVP0 + n)
#define FCP_PROTOCOL_DISCRETE_UVP_STEP       100 /* step: 100mv */

/* output current configure information register */
#define FCP_PROTOCOL_OUTPUT_I0               0x50
#define FCP_PROTOCOL_OUTPUT_I1               0x51
#define FCP_PROTOCOL_OUTPUT_I2               0x52
#define FCP_PROTOCOL_OUTPUT_I3               0x53
#define FCP_PROTOCOL_OUTPUT_I4               0x54
#define FCP_PROTOCOL_OUTPUT_I5               0x55
#define FCP_PROTOCOL_OUTPUT_I6               0x56
#define FCP_PROTOCOL_OUTPUT_I7               0x57
#define FCP_PROTOCOL_OUTPUT_I8               0x58
#define FCP_PROTOCOL_OUTPUT_I9               0x59
#define FCP_PROTOCOL_OUTPUT_IA               0x5a
#define FCP_PROTOCOL_OUTPUT_IB               0x5b
#define FCP_PROTOCOL_OUTPUT_IC               0x5c
#define FCP_PROTOCOL_OUTPUT_ID               0x5d
#define FCP_PROTOCOL_OUTPUT_IE               0x5e
#define FCP_PROTOCOL_OUTPUT_IF               0x5f
#define FCP_PROTOCOL_OUTPUT_I_REG(n)         (FCP_PROTOCOL_OUTPUT_I0 + n)
#define FCP_PROTOCOL_OUTPUT_I_STEP           100 /* step: 100ma */

enum fcp_protocol_error_code {
	FCP_DETECT_OTHER = -1,
	FCP_DETECT_SUCC = 0,
	FCP_DETECT_FAIL = 1,
};

struct fcp_protocol_device_info {
	int support_mode; /* adapter support mode */
	unsigned int vid_h; /* vendor id */
	unsigned int vid_l;
	unsigned int compiledver_h; /* compiled version */
	unsigned int compiledver_l;
	unsigned int fwver_h; /* firmware version */
	unsigned int fwver_l;
	unsigned int specver; /* spec version */
	int volt_cap;
	int max_volt;
	int max_pwr;
	int vid_rd_flag;
	int compiledver_rd_flag;
	int fwver_rd_flag;
	int specver_rd_flag;
	int volt_cap_rd_flag;
	int max_volt_rd_flag;
	int max_pwr_rd_flag;
	int rw_error_flag;
};

struct fcp_protocol_ops {
	const char *chip_name;
	int (*reg_read)(int reg, int *val, int num);
	int (*reg_write)(int reg, int *val, int num);
	int (*detect_adapter)(void);
	int (*soft_reset_master)(void);
	int (*soft_reset_slave)(void);
	int (*get_master_status)(void);
	int (*stop_charging_config)(void);
	int (*pre_init)(void); /* process non protocol flow */
	int (*post_init)(void);  /* process non protocol flow */
	int (*pre_exit)(void); /* process non protocol flow */
	int (*post_exit)(void);  /* process non protocol flow */
};

struct fcp_protocol_dev {
	struct device *dev;
	struct fcp_protocol_device_info info;

	int dev_id;
	struct fcp_protocol_ops *p_ops;
};

#ifdef CONFIG_CHARGER_PROTOCOL_FCP
int fcp_protocol_ops_register(struct fcp_protocol_ops *ops);

#else

static inline int fcp_protocol_ops_register(struct fcp_protocol_ops *ops)
{
	return -1;
}
#endif /* CONFIG_CHARGER_PROTOCOL_FCP */

#endif /* _CHARGER_PROTOCOL_FCP_H_ */
