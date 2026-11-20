/*
 * smartpakit_defs.h
 *
 * smartpakit driver
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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
#ifndef __SMARTPAKIT_DEFS_H__
#define __SMARTPAKIT_DEFS_H__

#define SMARTPAKIT_NAME_MAX       64
#define SMARTPAKIT_NAME_INVALID   "none"
#define SMARTPAKIT_VENDR_INVALID  0xff
#define RESERVED_NUM              200

/* platform type */
enum smartpakit_soc_platform {
	SMARTPAKIT_SOC_PLATFORM_HISI = 0, /* hisi chip */
	SMARTPAKIT_SOC_PLATFORM_QCOM,     /* qcom chip */
	SMARTPAKIT_SOC_PLATFORM_MTK,

	SMARTPAKIT_SOC_PLATFORM_MAX,
};

/*
 * pa type
 * 1. smartpa with dsp
 * 2. smartpa without dsp
 *    2.1 use soc dsp
 *    2.2 use codec dsp
 * 3. simple pa, not smart pa
 *
 * algo running on: 0 codec_dsp | 1 soc_dsp | 2 smartpa_dsp
 */
enum smartpakit_algo_in {
	SMARTPAKIT_ALGO_IN_CODEC_DSP = 0,
	SMARTPAKIT_ALGO_IN_SOC_DSP,
	SMARTPAKIT_ALGO_IN_WITH_DSP,
	SMARTPAKIT_ALGO_IN_SIMPLE,
	SMARTPAKIT_ALGO_IN_SIMPLE_WITH_I2C,
	SMARTPAKIT_ALGO_IN_WITH_DSP_PLUGIN, /* only use gpio and getinfo */

	SMARTPAKIT_ALGO_IN_MAX,
	SMARTPAKIT_ALGO_IN_DSP_MAX = SMARTPAKIT_ALGO_IN_WITH_DSP + 1,
};

/* different device(rec or spk) use different algo params */
enum smartpakit_out_device {
	SMARTPAKIT_OUT_DEVICE_SPEAKER = 0,
	SMARTPAKIT_OUT_DEVICE_RECEIVER,

	SMARTPAKIT_OUT_DEVICE_MAX,
};

/* out device mask */
#define SMARTPAKIT_PA_OUT_DEVICE_MASK      0xF
#define SMARTPAKIT_PA_OUT_DEVICE_SHIFT     4

/* Now, up to only support four pa */
enum smartpakit_pa_id {
	SMARTPAKIT_PA_ID_BEGIN = 0,
	SMARTPAKIT_PA_ID_PRIL  = SMARTPAKIT_PA_ID_BEGIN,
	SMARTPAKIT_PA_ID_PRIR,
	SMARTPAKIT_PA_ID_SECL,
	SMARTPAKIT_PA_ID_SECR,
	SMARTPAKIT_PA_ID_THIRDL,
	SMARTPAKIT_PA_ID_THIRDR,
	SMARTPAKIT_PA_ID_FOURTHL,
	SMARTPAKIT_PA_ID_FOURTHR,
	SMARTPAKIT_PA_ID_MAX,
	SMARTPAKIT_PA_ID_ALL   = 0xFF,
};

/* which chip provider */
enum smartpakit_chip_vendor {
	SMARTPAKIT_CHIP_VENDOR_MAXIM = 0,    // max98925
	SMARTPAKIT_CHIP_VENDOR_NXP,          // tfa9872, tfa9895
	SMARTPAKIT_CHIP_VENDOR_TI,           // tas2560
	SMARTPAKIT_CHIP_VENDOR_CS,           // cs
	SMARTPAKIT_CHIP_VENDOR_OTHER = SMARTPAKIT_CHIP_VENDOR_CS,  // other
	SMARTPAKIT_CHIP_VENDOR_CUSTOMIZE,    // huawei customize
	SMARTPAKIT_CHIP_VENDOR_RT,           // richtek
	SMARTPAKIT_CHIP_VENDOR_AWINIC,       // awinic
	SMARTPAKIT_CHIP_VENDOR_FOURSEMI,     // foursemi

	SMARTPAKIT_CHIP_VENDOR_MAX,
};

/* For hismartpa */
struct hismartpa_coeff {
	unsigned short kv1;
	unsigned short kv2;
	unsigned short kr1;
	unsigned short kr2;
};

enum gpio_state {
	GPIO_LOW = 0,
	GPIO_HIGH,
};

struct smartpakit_info {
	/* common info */
	unsigned int soc_platform;
	unsigned int algo_in;
	unsigned int out_device;
	unsigned int pa_num;
	unsigned int param_version;
	char special_name_config[SMARTPAKIT_NAME_MAX];

	/* smartpa chip info */
	unsigned int algo_delay_time;
	unsigned int chip_vendor;
	char chip_model[SMARTPAKIT_NAME_MAX];
	char chip_model_list[SMARTPAKIT_PA_ID_MAX][SMARTPAKIT_NAME_MAX];
	char cust[SMARTPAKIT_NAME_MAX];
	char product_name[SMARTPAKIT_NAME_MAX];
	struct hismartpa_coeff otp[SMARTPAKIT_PA_ID_MAX];
	unsigned int cali_data_update_mode;

	/* reserved field */
	char reserved[RESERVED_NUM];
};

struct smartpakit_get_param {
	/* index : reg addr for smartpa, or gpio index for simple pa */
	unsigned int index;
	unsigned int value;
};

/* for ioctl cmd SMARTPAKIT_W_ALL */
#define SMARTPAKIT_PA_CTL_MASK        0x1
#define SMARTPAKIT_PA_CTL_OFFSET      4
#define SMARTPAKIT_NEED_RESUME_MASK   0x80000000
#define SMARTPAKIT_PA_MUTE_MASK       0x40000000

/* for system/lib64/x.so(64 bits) */
struct smartpakit_set_param {
	unsigned int pa_ctl_mask;
	unsigned int param_num;
	unsigned int *params;
};

/* ioctl params, 8k unsigned int */
#define SMARTPAKIT_IO_PARAMS_NUM_MAX   (8 * 1024)
/* rw params(i2c_transfer), 32k bytes */
#define SMARTPAKIT_RW_PARAMS_NUM_MAX   (32 * 1024)

/* for system/lib/x.so(32 bits) */
struct smartpakit_set_param_compat {
	unsigned int pa_ctl_mask;
	unsigned int param_num;
	unsigned int params_ptr;
};

/* IO controls for smart pa */
#define SMARTPAKIT_GET_INFO       _IOR('M', 0x01, struct smartpakit_info)

#define SMARTPAKIT_HW_RESET       _IO('M', 0x02) /* reset chip      */
#define SMARTPAKIT_HW_PREPARE     _IO('M', 0x03) /* prepare chip    */
#define SMARTPAKIT_HW_UNPREPARE   _IO('M', 0x04) /* un-prepare chip */
#define SMARTPAKIT_REGS_DUMP      _IO('M', 0x05) /* dump regs       */

/* read reg cmd */
#define SMARTPAKIT_R_PRIL    _IOR('M', 0x11, struct smartpakit_get_param)
#define SMARTPAKIT_R_PRIR    _IOR('M', 0x12, struct smartpakit_get_param)
#define SMARTPAKIT_R_SECL    _IOR('M', 0x13, struct smartpakit_get_param)
#define SMARTPAKIT_R_SECR    _IOR('M', 0x14, struct smartpakit_get_param)

/* write regs cmd */
#define SMARTPAKIT_INIT      _IOW('M', 0x21, struct smartpakit_set_param)
#define SMARTPAKIT_W_ALL     _IOW('M', 0x22, struct smartpakit_set_param)
#define SMARTPAKIT_W_PRIL    _IOW('M', 0x23, struct smartpakit_set_param)
#define SMARTPAKIT_W_PRIR    _IOW('M', 0x24, struct smartpakit_set_param)
#define SMARTPAKIT_W_SECL    _IOW('M', 0x25, struct smartpakit_set_param)
#define SMARTPAKIT_W_SECR    _IOW('M', 0x26, struct smartpakit_set_param)

/* write regs cmd(compat_ioctl) */
#define SMARTPAKIT_INIT_COMPAT \
	_IOW('M', 0x21, struct smartpakit_set_param_compat)
#define SMARTPAKIT_W_ALL_COMPAT \
	_IOW('M', 0x22, struct smartpakit_set_param_compat)
#define SMARTPAKIT_W_PRIL_COMPAT \
	_IOW('M', 0x23, struct smartpakit_set_param_compat)
#define SMARTPAKIT_W_PRIR_COMPAT \
	_IOW('M', 0x24, struct smartpakit_set_param_compat)
#define SMARTPAKIT_W_SECL_COMPAT \
	_IOW('M', 0x25, struct smartpakit_set_param_compat)
#define SMARTPAKIT_W_SECR_COMPAT \
	_IOW('M', 0x26, struct smartpakit_set_param_compat)

// pa check loop
#define SMARTPAKIT_CHECK    _IOW('M', 0x27, int)

#endif /* __SMARTPAKIT_DEFS_H__ */

