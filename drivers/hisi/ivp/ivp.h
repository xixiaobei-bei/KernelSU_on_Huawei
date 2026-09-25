/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description:  This file is IVP driver related operations.
 * Create: 2017-2-9
 */
#ifndef _IVP_H_
#define _IVP_H_
// This is used to compat  mmap for Android 32Bit to Kernel 64Bit.
// Must compat with IVP DTS

#define SIZE_1MB                         (1 * 1024 * 1024)
#define SIZE_2MB                         (2 * 1024 * 1024)
#define MASK_1MB                         (SIZE_1MB - 1)
#define SIZE_4K                          (4 * 1024)
#define IVP_MMAP_SHIFT                   4
#define SHIFT_1MB                        20
#define IVP_STATUS_CHECK_DELAY_US        100

#define IVP_MODULE_NAME                  "hisi-ivp"
#define IVP1_MODULE_NAME                 "hisi-ivp1"
#define IVP_REGULATOR                    "hisi-ivp"
#define IVP_MEDIA_REGULATOR              "hisi-ivp-media"

// dts info
#define OF_IVP_SECTION_NAME              "section_mem"
#define OF_IVP_SECTION_NODE_NAME         "ivp_mem_section"
#define OF_IVP_DDR_MEM_NAME              "ddr_mem"
#define OF_IVP_SHARE_MEM_NAME            "share_mem"
#define OF_IVP_LOG_MEM_NAME              "log_mem"

#define OF_IVP_CLK_NAME                  "hisi-ivp-clk"
#define OF_IVP_CLK_RATE_NAME             "ivp-clk-rate"
#define OF_IVP_MIDDLE_CLK_RATE_NAME      "ivp-middle-clk-rate"
#define OF_IVP_LOW_CLK_RATE_NAME         "ivp-low-clk-rate"
#define OF_IVP_ULTRA_LOW_CLK_RATE_NAME   "ivp-ultra-low-clk-rate"
#define OF_IVP_LOW_CLK_PU_RATE_NAME      "ivp-lowfrq-pu-clk-rate"
#define OF_IVP_LOW_TEMP_RATE_NAME        "ivp-lowtemp-clk-rate"
#define OF_IVP_LOWFREQ_CLK_RATE_NAME     "lowfrq-pd-clk-rate"

#define OF_IVP_DYNAMIC_MEM               "ivp-dynamic-mem"
#define OF_IVP_DYNAMIC_MEM_SEC_SIZE      "ivp-dynamic-mem-section-size"
#define OF_IVP_SEC_SUPPORT               "ivp-sec-support-flag"
#define OF_IVP_PRIVATE_MEM               "ivp-private-mem"

#define IVP_IOCTL_SECTCOUNT              _IOR('v', 0x70, unsigned int)
#define IVP_IOCTL_SECTINFO               _IOWR('v', 0x71, struct ivp_sect_info)
#define IVP_IOCTL_DSP_RUN                _IOW('v', 0x72, unsigned int)
#define IVP_IOCTL_DSP_SUSPEND            _IOW('v', 0x73, unsigned int)
#define IVP_IOCTL_DSP_RESUME             _IOW('v', 0x74, unsigned int)
#define IVP_IOCTL_DSP_STOP               _IOW('v', 0x75, unsigned int)
#define IVP_IOCTL_QUERY_RUNSTALL         _IOR('v', 0x76, unsigned int)
#define IVP_IOCTL_QUERY_WAITI            _IOR('v', 0x77, unsigned int)
#define IVP_IOCTL_TRIGGER_NMI            _IOW('v', 0x78, unsigned int)
#define IVP_IOCTL_WATCHDOG               _IOR('v', 0x79, unsigned int)
#define IVP_IOCTL_WATCHDOG_SLEEP         _IOR('v', 0x7a, unsigned int)

#define IVP_IOCTL_SMMU_INVALIDATE_TLB    _IOW('v', 0x7b, unsigned int)
#define IVP_IOCTL_BM_INIT                _IOW('v', 0x7c, unsigned int)
#define IVP_IOCTL_CLK_LEVEL              _IOW('v', 0x7d, unsigned int)
#define IVP_IOCTL_POWER_UP               _IOW('v', 0x7e, struct ivp_power_up_info)
#define IVP_IOCTL_DUMP_DSP_STATUS        _IOW('v', 0x7f, unsigned int)
#define IVP_IOCTL_QUERY_CHIP_TYPE        _IOW('v', 0x80, unsigned int)
#define IVP_IOCTL_LOAD_FIRMWARE          _IOW('v', 0x8A, struct ivp_image_info)

#define IVP_IOCTL_IPC_FLUSH_ENABLE       _IOWR('v', 0x60, unsigned int)
#define IVP_IOCTL_IPC_IVP_SECMODE        _IOWR('v', 0x61, unsigned int)

enum ivp_core_id {
	IVP_CORE0_ID,
	IVP_CORE1_ID,
	IVP_CORE_MAX_NUM
};
#define MEDIA2_IVP_SID_VALUE             8
#define MEDIA2_IVP0_SSID_VALUE           11
#define MEDIA2_IVP1_SSID_VALUE           12

#define MAX_INDEX                        64
#define SECT_START_NUM                   3
enum SEC_MODE {
	NOSEC_MODE = 0,
	SECURE_MODE = 1
};
enum IVP_SECTION_INDEX {
    DRAM0_SECTION_INDEX = 0,
    DRAM1_SECTION_INDEX,
    IRAM_SECTION_INDEX,
    DDR_SECTION_INDEX,
    SHARE_SECTION_INDEX,
    LOG_SECTION_INDEX,
    IVP_SECTION_MAX
};

struct ivp_sect_info {
	char name[64];
	unsigned int index;
	unsigned int len;
	unsigned int ivp_addr;
	unsigned int reserved;
	union {
		unsigned long acpu_addr;
		char compat32[8];
	};
};

struct ivp_image_info {
	char name[64];
	unsigned int length;
};

struct ivp_power_up_info {
	int sec_mode;
	int sec_buff_fd;
};

#endif /* IVP_H_ */
