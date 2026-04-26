/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: analog headset common head file
 * Author: lijinkui
 * Create: 2019-7-16
 */

#ifndef ANA_HS_COMMON_H
#define ANA_HS_COMMON_H

/* IO controls for user */
#define IOCTL_ANA_HS_GET_MIC_SWITCH_STATE               _IOR('U', 0x01, __u32)
#define IOCTL_ANA_HS_GND_FB_CONNECT                     _IO('U', 0x02)
#define IOCTL_ANA_HS_GND_FB_DISCONNECT                  _IO('U', 0x03)
#define IOCTL_ANA_HS_GET_CONNECT_LINEIN_R_STATE         _IOR('U', 0x04, __u32)

#define IOCTL_ANA_HS_CHARGE_ENABLE_CMD           _IO('A', 0x01)
#define IOCTL_ANA_HS_CHARGE_DISABLE_CMD          _IO('A', 0x02)
#define IOCTL_ANA_HS_GET_CHARGE_STATUS_CMD       _IOR('A', 0xFF, __u32)
#define IOCTL_ANA_HS_GET_HEADSET_CMD             _IOR('A', 0xFE, __u32)
#define IOCTL_ANA_HS_GET_VBST_5VOLTAGE_CMD       _IOR('A', 0xFD, __u32)
#define IOCTL_ANA_HS_GET_VDD_BUCK_VOLTAGE_CMD    _IOR('A', 0xFC, __u32)
#define IOCTL_ANA_HS_GET_HEADSET_RESISTANCE_CMD  _IOR('A', 0xFB, __u32)

enum {
	ANA_HS_NORMAL_4POLE = 0,
	ANA_HS_NORMAL_3POLE,
	ANA_HS_HEADSET,
	ANA_HS_REVERT_4POLE,
	ANA_HS_NONE,
};

enum ana_hs_state {
	ANA_HS_PLUG_OUT = 0,
	ANA_HS_PLUG_IN,
	/* for aux switch in display port */
	DP_PLUG_IN,
	DP_PLUG_IN_CROSS,
	DP_PLUG_OUT,
	DIRECT_CHARGE_IN,
	DIRECT_CHARGE_OUT,
	POGOPIN_PLUG_IN,
	POGOPIN_PLUG_OUT
};

/* codec operate used by anc_hs driver */
struct ana_hs_codec_ops {
	bool (*check_headset_in)(void *);
	void (*plug_in_detect)(void *);
	void (*plug_out_detect)(void *);
	int (*get_headset_type)(void *);
	void (*hs_high_resistence_enable)(void *, bool);
};

struct ana_hs_codec_dev {
	char *name;
	struct ana_hs_codec_ops ops;
};

#endif /* ANA_HS_COMMON_H */
